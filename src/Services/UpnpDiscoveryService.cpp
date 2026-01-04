//Created by: EternityBoundary on Jan 4, 2025
#include "pch.h"
#include "include/Services/UpnpDiscoveryService.h"

#pragma comment(lib, "Ws2_32.lib")

namespace EtherealScepter::Services::Upnp
{
    static std::wstring ExtractHeader(
        const std::string& response,
        const char* headerName)
    {
        auto pos = response.find(headerName);
        if (pos == std::string::npos)
            return L"";

        auto colon = response.find(':', pos);
        auto end = response.find("\r\n", colon);
        if (colon == std::string::npos || end == std::string::npos)
            return L"";

        std::string value = response.substr(colon + 1, end - colon - 1);

        // trim leading spaces
        while (!value.empty() && value.front() == ' ')
            value.erase(value.begin());

        return std::wstring(value.begin(), value.end());
    }

    std::vector<UpnpDeviceLocation> UpnpDiscoveryService::Discover()
    {
        std::vector<UpnpDeviceLocation> results;

        WSADATA wsa{};
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            return results;

        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET)
        {
            WSACleanup();
            return results;
        }

        // Receive timeout: 2 seconds
        DWORD timeoutMs = 2000;
        setsockopt(
            sock,
            SOL_SOCKET,
            SO_RCVTIMEO,
            reinterpret_cast<char*>(&timeoutMs),
            sizeof(timeoutMs)
        );

        sockaddr_in multicastAddr{};
        multicastAddr.sin_family = AF_INET;
        multicastAddr.sin_port = htons(1900);
        inet_pton(AF_INET, "239.255.255.250", &multicastAddr.sin_addr);

        const char* request =
            "M-SEARCH * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "MAN: \"ssdp:discover\"\r\n"
            "MX: 2\r\n"
            "ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
            "\r\n";

        sendto(
            sock,
            request,
            static_cast<int>(strlen(request)),
            0,
            reinterpret_cast<sockaddr*>(&multicastAddr),
            sizeof(multicastAddr)
        );

        char buffer[4096]{};
        sockaddr_in from{};
        int fromLen = sizeof(from);

        while (true)
        {
            int received = recvfrom(
                sock,
                buffer,
                sizeof(buffer) - 1,
                0,
                reinterpret_cast<sockaddr*>(&from),
                &fromLen
            );

            if (received <= 0)
                break;

            buffer[received] = '\0';
            std::string response(buffer);

            UpnpDeviceLocation device;
            device.location = ExtractHeader(response, "LOCATION");
            device.server = ExtractHeader(response, "SERVER");
            device.st = ExtractHeader(response, "ST");

            if (!device.location.empty())
                results.push_back(std::move(device));
        }

        closesocket(sock);
        WSACleanup();

        return results;
    }
}
