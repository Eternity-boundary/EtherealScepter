// Created by: EternityBoundary on Jan 4, 2025
#include "pch.h"

#include "include/Services/NetworkStatusService.h"
#include "include/Services/UpnpDiscoveryService.h"
#include "include/Services/IgdDescriptionParser.h"
#include "include/Services/UpnpSoapClient.h"

using namespace winrt;
using namespace Windows::Networking;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Networking::Connectivity;

namespace
{
    bool IsPrivateIPv4(hstring const& ip)
    {
        std::wstring s = ip.c_str();
        return
            s.starts_with(L"10.") ||
            s.starts_with(L"192.168.") ||
            (s.starts_with(L"172.") &&
                std::stoi(s.substr(4, 2)) >= 16 &&
                std::stoi(s.substr(4, 2)) <= 31);
    }

    hstring GetLocalIPv4()
    {
        for (auto const& host : NetworkInformation::GetHostNames())
        {
            if (host.Type() == HostNameType::Ipv4)
            {
                auto ip = host.CanonicalName();
                if (!ip.starts_with(L"169.254"))
                    return ip;
            }
        }
        return L"-";
    }

    hstring QueryWanIpHttpFallback()
    {
        try
        {
            HttpClient client;
            auto uri = Uri(L"https://api.ipify.org");
            return client.GetStringAsync(uri).get();
        }
        catch (...)
        {
            return L"-";
        }
    }

    hstring DetectCGNAT(hstring const& wanIp)
    {
        if (wanIp == L"-")
            return L"Unknown";
        return IsPrivateIPv4(wanIp) ? L"CGNAT" : L"Open";
    }
}

namespace EtherealScepter::Services
{
    NetworkSnapshot NetworkStatusService::Query()
    {
        NetworkSnapshot snapshot{};

        // =====================================================
        // 1. OS-authoritative network connectivity
        // =====================================================
        auto profile = NetworkInformation::GetInternetConnectionProfile();
        if (profile &&
            profile.GetNetworkConnectivityLevel() != NetworkConnectivityLevel::None)
        {
            switch (profile.GetNetworkConnectivityLevel())
            {
            case NetworkConnectivityLevel::InternetAccess:
                snapshot.networkStatus = L"Internet";
                break;
            case NetworkConnectivityLevel::LocalAccess:
                snapshot.networkStatus = L"Local Network";
                break;
            default:
                snapshot.networkStatus = L"Disconnected";
                break;
            }
        }
        else
        {
            snapshot.networkStatus = L"Disconnected";
        }

        // =====================================================
        // 2. Baseline IP info (always available)
        // =====================================================
        snapshot.localIp = GetLocalIPv4();
        snapshot.wanIp = QueryWanIpHttpFallback();
        snapshot.cgnatStatus = DetectCGNAT(snapshot.wanIp);

        // Default safe values
        snapshot.upnpStatus = L"Unavailable";
        snapshot.upnpDeviceCount = L"0 Devices";
        snapshot.natType = L"Unknown";
        snapshot.portForwardingStatus = L"Unavailable";
        snapshot.summary = L"WAN IP via HTTP";

        // =====================================================
        // 3. UPnP Discovery
        // =====================================================
        EtherealScepter::Services::Upnp::UpnpDiscoveryService discovery;
        auto devices = discovery.Discover();
        if (devices.empty())
            return snapshot;

        snapshot.upnpStatus = L"Enabled";
        snapshot.upnpDeviceCount =
            winrt::to_hstring(devices.size()) + L" Devices Found";

        // =====================================================
        // 4. Pick first valid IGD service (WANIP / WANPPP)
        // =====================================================
        std::optional<EtherealScepter::Services::Upnp::UpnpIgdServiceInfo> igdService;

        for (auto const& device : devices)
        {
            igdService =
                EtherealScepter::Services::Upnp::IgdDescriptionParser
                ::ParseFromLocation(device.location);

            if (igdService)
                break;
        }

        if (!igdService)
        {
            snapshot.portForwardingStatus = L"UPnP IGD Not Found";
            return snapshot;
        }

        // =====================================================
        // 5. SOAP: GetExternalIPAddress
        // =====================================================
        EtherealScepter::Services::Upnp::UpnpSoapClient soap;
        auto externalIpOpt = soap.GetExternalIPAddress(*igdService);

        if (externalIpOpt && !externalIpOpt->empty())
        {
            snapshot.wanIp = winrt::hstring{ externalIpOpt->c_str() };
            snapshot.cgnatStatus = DetectCGNAT(snapshot.wanIp);
            snapshot.summary = L"WAN IP via UPnP";
        }

        // =====================================================
        // 6. Interpretation (best-effort)
        // =====================================================
        snapshot.natType = L"Open";                 // conservative default
        snapshot.portForwardingStatus = L"Supported";

        return snapshot;
    }
}
