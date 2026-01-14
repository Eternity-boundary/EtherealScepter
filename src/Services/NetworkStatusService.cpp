// Created by: EternityBoundary on Jan 4, 2026
#include "pch.h"

#include "include/Services/IgdDescriptionParser.h"
#include "include/Services/NetworkStatusService.h"
#include "include/Services/UpnpDiscoveryService.h"
#include "include/Services/UpnpSoapClient.h"
#include <include/Services/UpnpIgdServiceInfo.h>
#include <optional>
#include <string>
#include <utility>
#include <winrt/base.h>
#include <winrt/impl/EtherealScepter.Models.2.h>
#include <winrt/impl/Microsoft.UI.Xaml.0.h>
#include <winrt/impl/Windows.Foundation.2.h>
#include <winrt/impl/Windows.Networking.0.h>
#include <winrt/impl/Windows.Networking.Connectivity.0.h>
#include <winrt/impl/Windows.Networking.Connectivity.2.h>
#include <winrt/impl/Windows.Web.Http.2.h>

using namespace winrt;
using namespace Windows::Networking;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Networking::Connectivity;
namespace ES = ::EtherealScepter;

namespace {
bool IsPrivateIPv4(hstring const &ip) {
  std::wstring s = ip.c_str();
  return s.starts_with(L"10.") || s.starts_with(L"192.168.") ||
         (s.starts_with(L"172.") && std::stoi(s.substr(4, 2)) >= 16 &&
          std::stoi(s.substr(4, 2)) <= 31);
}

hstring GetLocalIPv4() {
  for (auto const &host : NetworkInformation::GetHostNames()) {
    if (host.Type() == HostNameType::Ipv4) {
      auto ip = host.CanonicalName();
      if (!ip.starts_with(L"169.254"))
        return ip;
    }
  }
  return L"-";
}

hstring QueryWanIpHttpFallback() {
  try {
    HttpClient client;
    auto uri = Uri(L"https://api.ipify.org");
    auto asyncOp = client.GetStringAsync(uri);
    auto result = asyncOp.get();
    if (result.empty()) {
      return L"-";
    }
    return result;
  } catch (...) {
    return L"-";
  }
}

hstring DetectCGNAT(hstring const &wanIp,bool const &isIpFallback) {
  if (wanIp == L"-" || wanIp.empty() || isIpFallback)
    return L"Unknown";
  return IsPrivateIPv4(wanIp) ? L"CGNAT" : L"Open";
}
} // namespace

namespace EtherealScepter::Services {
NetworkSnapshot NetworkStatusService::Query() {
  NetworkSnapshot snapshot{};
  std::optional<ES::Services::Upnp::UpnpIgdServiceInfo> igdService;
  winrt::hstring igdLocation;

  // 1. OS network status
  snapshot.networkStatus = L"Disconnected";

  if (auto profile = NetworkInformation::GetInternetConnectionProfile()) {
    switch (profile.GetNetworkConnectivityLevel()) {
    case NetworkConnectivityLevel::InternetAccess:
      snapshot.networkStatus = L"Internet";
      break;
    case NetworkConnectivityLevel::LocalAccess:
      snapshot.networkStatus = L"Local Network";
      break;
    default:
      break;
    }
  }

  // 2. Baseline IP (HTTP fallback) - 只呼叫一次
  snapshot.localIp = GetLocalIPv4();
  snapshot.httpWanIp = QueryWanIpHttpFallback();
  snapshot.wanIp = snapshot.httpWanIp;
  snapshot.cgnatStatus = DetectCGNAT(snapshot.wanIp, true);
  snapshot.summary = L"WAN IP via HTTP";

  // 初始化 upnpWanIp 為空（稍後如果 UPnP 成功會覆蓋）
  snapshot.upnpWanIp = L"";

  // Default UPnP values
  snapshot.upnpStatus = L"Unavailable";
  snapshot.upnpDeviceCount = L"0 Devices";
  snapshot.portForwardingStatus = L"Unavailable";
  snapshot.natType = L"Unknown";

  // 3. UPnP Discovery
  ES::Services::Upnp::UpnpDiscoveryService discovery;
  auto devices = discovery.Discover();

  snapshot.upnpDevices.clear();
  snapshot.upnpDevices.reserve(devices.size());

  for (auto const &d : devices) {
    winrt::EtherealScepter::Models::UpnpDeviceInfo info{};

    info.Location = winrt::hstring{d.location.c_str()};

    // TODO:沒有 St/Server 欄位：先暫用現有欄位讓 UI 能顯示
    // finished by: EternityBoundary on Jan 14, 2026
    info.DeviceType = winrt::hstring{d.st.c_str()};
    info.Manufacturer = winrt::hstring{d.server.c_str()};

    if (!d.st.empty())
      info.FriendlyName = winrt::hstring{d.st.c_str()};
    else if (!d.server.empty())
      info.FriendlyName = winrt::hstring{d.server.c_str()};
    else
      info.FriendlyName = L"(UPnP Device)";

    info.IsIgd = false;

    snapshot.upnpDevices.push_back(std::move(info));
  }

  if (!devices.empty()) {
    snapshot.upnpStatus = L"Enabled";
    snapshot.upnpDeviceCount =
        winrt::to_hstring(devices.size()) + L" Devices Found";
  }

  // 4. Parse IGD (WANIP / WANPPP)
  for (auto const &device : devices) {
    auto parsed = ES::Services::Upnp::IgdDescriptionParser ::ParseFromLocation(
        device.location);

    if (parsed) {
      snapshot.igdService = std::move(parsed);
      igdLocation = winrt::hstring{device.location.c_str()};
      break;
    }
  }

  if (snapshot.igdService && !igdLocation.empty()) {
    for (auto &info : snapshot.upnpDevices) {
      if (info.Location == igdLocation) {
        info.IsIgd = true;
        info.IgdBadgeVisibility =
            winrt::Microsoft::UI::Xaml::Visibility::Visible;
        info.DeviceType =
            winrt::hstring{snapshot.igdService->serviceType.c_str()};
        info.PresentationUrl =
            winrt::hstring{snapshot.igdService->presentationUrl.c_str()};

        // Populate device info from parsed IGD description
        if (!snapshot.igdService->friendlyName.empty())
          info.FriendlyName =
              winrt::hstring{snapshot.igdService->friendlyName.c_str()};
        if (!snapshot.igdService->manufacturer.empty())
          info.Manufacturer =
              winrt::hstring{snapshot.igdService->manufacturer.c_str()};
        if (!snapshot.igdService->modelName.empty())
          info.ModelName =
              winrt::hstring{snapshot.igdService->modelName.c_str()};

        break;
      }
    }
  }

  // 5. SOAP WAN IP (primary + fallback)
  if (snapshot.igdService) {
    ES::Services::Upnp::UpnpSoapClient soap;

    std::optional<std::wstring> wanIp;

    wanIp = soap.GetExternalIPAddress(*snapshot.igdService);

    // fallback
    if (!wanIp) {
      wanIp = soap.GetExternalIPAddressViaStatus(*snapshot.igdService);
      snapshot.isIpFallback = true;
    }

    if (wanIp && !wanIp->empty()) {
      snapshot.upnpWanIp = winrt::hstring{wanIp->c_str()};
      snapshot.wanIp = winrt::hstring{wanIp->c_str()};
      snapshot.cgnatStatus = DetectCGNAT(snapshot.wanIp, snapshot.isIpFallback);
      snapshot.summary = L"WAN IP via UPnP";
      snapshot.portForwardingStatus = L"Forwarding Supported";
      snapshot.natType = L"Open";
    } else {
      snapshot.summary = L"UPnP IGD found, but WAN IP unavailable";
    }
  }

  return snapshot;
}

} // namespace EtherealScepter::Services
