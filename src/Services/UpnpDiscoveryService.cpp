//Created by: EternityBoundary on Jan 4, 2025
#include "pch.h"
#include "include/Services/UpnpDiscoveryService.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")


namespace EtherealScepter::Services::Upnp
{
    // -------------------------
    // Case-insensitive header extraction
    // -------------------------
    static std::wstring ExtractHeaderCI(const std::string& response, const char* headerName)
    {
        std::string lower = response;
        for (char& c : lower)
            c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));

        std::string key = headerName;
        for (char& c : key)
            c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));

        const std::string needle = key + ":";

        size_t pos = 0;
        while (true)
        {
            pos = lower.find(needle, pos);
            if (pos == std::string::npos)
                return L"";

            // Must be at start of line (or start of buffer)
            if (pos == 0 || (pos >= 2 && lower[pos - 2] == '\r' && lower[pos - 1] == '\n'))
                break;

            pos += 1;
        }

        const size_t colon = lower.find(':', pos);
        if (colon == std::string::npos)
            return L"";

        size_t end = lower.find("\r\n", colon);
        if (end == std::string::npos)
            end = lower.size();

        std::string value = response.substr(colon + 1, end - colon - 1);

        while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
            value.erase(value.begin());
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t'))
            value.pop_back();

        return std::wstring(value.begin(), value.end());
    }

    // -------------------------
    // Choose a "best" IPv4 interface using IP Helper:
    // prefer adapter that is Up and has an IPv4 gateway.
    // Return interface IPv4 as in_addr + also return string for debug.
    // -------------------------
    static bool GetBestInterfaceIPv4(in_addr& outIface, std::wstring& outIpString)
    {
        outIpString.clear();
        outIface.S_un.S_addr = INADDR_ANY;

        ULONG flags = GAA_FLAG_INCLUDE_GATEWAYS;
        ULONG family = AF_INET;

        ULONG size = 0;
        if (GetAdaptersAddresses(family, flags, nullptr, nullptr, &size) != ERROR_BUFFER_OVERFLOW)
            return false;

        std::vector<unsigned char> buf(size);
        auto* addrs = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buf.data());

        if (GetAdaptersAddresses(family, flags, nullptr, addrs, &size) != NO_ERROR)
            return false;

        // First pass: prefer adapters with an IPv4 gateway
        for (auto* a = addrs; a; a = a->Next)
        {
            if (a->OperStatus != IfOperStatusUp)
                continue;
            if (a->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
                continue;

            bool hasV4Gateway = false;
            for (auto* g = a->FirstGatewayAddress; g; g = g->Next)
            {
                if (!g->Address.lpSockaddr) continue;
                if (g->Address.lpSockaddr->sa_family == AF_INET)
                {
                    hasV4Gateway = true;
                    break;
                }
            }
            if (!hasV4Gateway)
                continue;

            for (auto* u = a->FirstUnicastAddress; u; u = u->Next)
            {
                if (!u->Address.lpSockaddr) continue;
                if (u->Address.lpSockaddr->sa_family != AF_INET) continue;

                auto* sin = reinterpret_cast<sockaddr_in*>(u->Address.lpSockaddr);
                // skip APIPA
                unsigned long ip = ntohl(sin->sin_addr.S_un.S_addr);
                if ((ip & 0xFFFF0000u) == 0xA9FE0000u) // 169.254.x.x
                    continue;

                outIface = sin->sin_addr;

                wchar_t ipStr[INET_ADDRSTRLEN]{};
                InetNtopW(AF_INET, &outIface, ipStr, INET_ADDRSTRLEN);
                outIpString = ipStr;
                return true;
            }
        }

        // Second pass fallback: any Up non-loopback IPv4
        for (auto* a = addrs; a; a = a->Next)
        {
            if (a->OperStatus != IfOperStatusUp)
                continue;
            if (a->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
                continue;

            for (auto* u = a->FirstUnicastAddress; u; u = u->Next)
            {
                if (!u->Address.lpSockaddr) continue;
                if (u->Address.lpSockaddr->sa_family != AF_INET) continue;

                auto* sin = reinterpret_cast<sockaddr_in*>(u->Address.lpSockaddr);

                unsigned long ip = ntohl(sin->sin_addr.S_un.S_addr);
                if ((ip & 0xFFFF0000u) == 0xA9FE0000u) // 169.254.x.x
                    continue;

                outIface = sin->sin_addr;

                wchar_t ipStr[INET_ADDRSTRLEN]{};
                InetNtopW(AF_INET, &outIface, ipStr, INET_ADDRSTRLEN);
                outIpString = ipStr;
                return true;
            }
        }

        return false;
    }

    std::vector<UpnpDeviceLocation> UpnpDiscoveryService::Discover()
    {
        std::vector<UpnpDeviceLocation> results;
        std::unordered_set<std::wstring> seenLocations;

        WSADATA wsa{};
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            return results;

        // Pick interface
        in_addr iface{};
        std::wstring ifaceIp;
        if (!GetBestInterfaceIPv4(iface, ifaceIp))
        {
            WSACleanup();
            return results;
        }

        // -----------------------------------------------------
        // Socket A: notifySock (bind 1900 + join multicast) to receive NOTIFY
        // -----------------------------------------------------
        SOCKET notifySock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (notifySock == INVALID_SOCKET)
        {
            WSACleanup();
            return results;
        }

        BOOL reuse = TRUE;
        (void)setsockopt(notifySock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

        sockaddr_in localNotify{};
        localNotify.sin_family = AF_INET;
        localNotify.sin_port = htons(1900);
        localNotify.sin_addr.s_addr = INADDR_ANY;

        // bind may fail if SSDP service owns 1900 exclusively; we still proceed with searchSock
        bool notifyReady = (bind(notifySock, (sockaddr*)&localNotify, sizeof(localNotify)) != SOCKET_ERROR);

        ip_mreq mreq{};
        InetPtonW(AF_INET, L"239.255.255.250", &mreq.imr_multiaddr);
        mreq.imr_interface = iface;

        if (notifyReady)
        {
            if (setsockopt(notifySock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR)
            {
                notifyReady = false;
            }
        }

        // -----------------------------------------------------
        // Socket B: searchSock (bind 0) to send M-SEARCH and receive responses
        // -----------------------------------------------------
        SOCKET searchSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (searchSock == INVALID_SOCKET)
        {
            if (notifySock != INVALID_SOCKET) closesocket(notifySock);
            WSACleanup();
            return results;
        }

        sockaddr_in localSearch{};
        localSearch.sin_family = AF_INET;
        localSearch.sin_port = 0; // ephemeral
        localSearch.sin_addr.s_addr = INADDR_ANY;

        if (bind(searchSock, (sockaddr*)&localSearch, sizeof(localSearch)) == SOCKET_ERROR)
        {
            closesocket(searchSock);
            if (notifySock != INVALID_SOCKET) closesocket(notifySock);
            WSACleanup();
            return results;
        }

        // ensure outbound multicast uses selected interface
        (void)setsockopt(searchSock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&iface, sizeof(iface));

        int ttl = 2;
        (void)setsockopt(searchSock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));

        // Send M-SEARCH
        sockaddr_in mcast{};
        mcast.sin_family = AF_INET;
        mcast.sin_port = htons(1900);
        inet_pton(AF_INET, "239.255.255.250", &mcast.sin_addr);

        // IGD search is usually better than ssdp:all (some routers respond less to ssdp:all)
        const char* request =
            "M-SEARCH * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "MAN: \"ssdp:discover\"\r\n"
            "MX: 2\r\n"
            "ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
            "\r\n";

        (void)sendto(
            searchSock,
            request,
            (int)strlen(request),
            0,
            (sockaddr*)&mcast,
            sizeof(mcast)
        );

        // -----------------------------------------------------
        // Wait up to 3 seconds for both NOTIFY + M-SEARCH responses
        // -----------------------------------------------------
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);

        while (std::chrono::steady_clock::now() < deadline)
        {
            fd_set readfds;
            FD_ZERO(&readfds);

            FD_SET(searchSock, &readfds);
            SOCKET maxfd = searchSock;

            if (notifyReady)
            {
                FD_SET(notifySock, &readfds);
                if (notifySock > maxfd) maxfd = notifySock;
            }

            TIMEVAL tv{};
            tv.tv_sec = 0;
            tv.tv_usec = 250 * 1000; // 250ms tick

            int ready = select((int)maxfd + 1, &readfds, nullptr, nullptr, &tv);
            if (ready == SOCKET_ERROR)
            {
                // select error; break
                break;
            }
            if (ready == 0)
            {
                // timeout tick; keep looping until deadline
                continue;
            }

            auto handlePacket = [&](SOCKET s)
                {
                    char buffer[8192]{};
                    sockaddr_in from{};
                    int fromLen = sizeof(from);

                    int received = recvfrom(s, buffer, (int)sizeof(buffer) - 1, 0, (sockaddr*)&from, &fromLen);
                    if (received <= 0)
                        return;

                    buffer[received] = '\0';
                    std::string resp(buffer);

                    UpnpDeviceLocation dev;
                    dev.location = ExtractHeaderCI(resp, "LOCATION");
                    dev.server = ExtractHeaderCI(resp, "SERVER");

                    // M-SEARCH response uses ST; NOTIFY uses NT
                    dev.st = ExtractHeaderCI(resp, "ST");
                    if (dev.st.empty())
                        dev.st = ExtractHeaderCI(resp, "NT");

                    if (!dev.location.empty())
                    {
                        if (seenLocations.insert(dev.location).second)
                            results.push_back(std::move(dev));
                    }
                };

            if (FD_ISSET(searchSock, &readfds))
                handlePacket(searchSock);

            if (notifyReady && FD_ISSET(notifySock, &readfds))
                handlePacket(notifySock);
        }

        // cleanup
        if (notifyReady)
        {
            (void)setsockopt(notifySock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
        }

        closesocket(searchSock);
        closesocket(notifySock);
        WSACleanup();

        return results;
    }
}
