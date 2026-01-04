//Created by: EternityBoundary on Jan 4, 2025
#include "pch.h"

#include "include/Services/NetworkStatusService.h"
#include "include/Services/UpnpDiscoveryService.h"

using namespace winrt;
using namespace Windows::Networking;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Networking::Connectivity;

static bool IsPrivateIPv4(hstring const& ip)
{
    std::wstring s = ip.c_str();
    return
        s.starts_with(L"10.") ||
        s.starts_with(L"192.168.") ||
        (s.starts_with(L"172.") &&
            std::stoi(s.substr(4, 2)) >= 16 &&
            std::stoi(s.substr(4, 2)) <= 31);
}

static hstring GetLocalIPv4()
{
    auto profiles = NetworkInformation::GetConnectionProfiles();

    for (auto const& profile : profiles)
    {
        auto hostNames = NetworkInformation::GetHostNames();

        for (auto const& host : hostNames)
        {
            if (host.Type() == HostNameType::Ipv4)
            {
                auto ip = host.CanonicalName();

                if (!ip.starts_with(L"169.254"))
                    return ip;
            }
        }
    }

    return L"-";
}

static hstring QueryWanIp()
{
    try
    {
        HttpClient client;
        auto uri = Uri(L"https://api.ipify.org");
        auto result = client.GetStringAsync(uri).get();
        return result;
    }
    catch (...)
    {
        return L"-";
    }
}

static hstring DetectCGNAT(hstring const& wanIp)
{
    if (wanIp == L"-")
        return L"Unknown";

    if (IsPrivateIPv4(wanIp))
        return L"CGNAT";

    return L"Public";
}


namespace EtherealScepter::Services
{
    NetworkSnapshot NetworkStatusService::Query()
    {
        NetworkSnapshot snapshot{};

		auto profile = NetworkInformation::GetInternetConnectionProfile();

        if (profile != nullptr &&
			profile.GetNetworkConnectivityLevel() != NetworkConnectivityLevel::None)
        {
			auto level = profile.GetNetworkConnectivityLevel();
            switch (level)
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

        //TODO: Implement actual network status querying logic here.
        snapshot.upnpStatus = L"Enabled";
        snapshot.natType = L"Open";
        snapshot.summary = L"Active Port Mappings: 3";

		snapshot.localIp = GetLocalIPv4();
		snapshot.wanIp = QueryWanIp();
		snapshot.cgnatStatus = DetectCGNAT(snapshot.wanIp);

        snapshot.upnpDeviceCount = L"2 Devices Found";
        snapshot.portForwardingStatus = L"Port Forwarding Available";

        return snapshot;
    }
}