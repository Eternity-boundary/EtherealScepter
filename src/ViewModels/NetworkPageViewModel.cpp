// Created by: EternityBoundary on Jan 12, 2026
#include "pch.h"

#include "include/ViewModels/NetworkPageViewModel.h"
#if __has_include("ViewModels.NetworkPageViewModel.g.cpp")
#include "ViewModels.NetworkPageViewModel.g.cpp"
#endif

#include "include/Services/NetworkConnectivityTestService.h"
#include "include/Services/NetworkStatusProvider.h"
#include "include/Services/NetworkStatusService.h"
#include "include/Services/PortReachabilityService.h"
#include <string_view>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/base.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml::Data;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI;
namespace Services = ::EtherealScepter::Services;

namespace winrt::EtherealScepter::ViewModels::implementation {

// -------------------------
// Constructor
// -------------------------
NetworkPageViewModel::NetworkPageViewModel() {
  auto &provider = Services::NetworkStatusProvider::Instance();
  provider.InitializeDispatcher();

  m_subscriptionId = provider.Subscribe(
      [weak_this = get_weak()](Services::NetworkSnapshot const &snapshot) {
        if (auto strong_this = weak_this.get()) {
          strong_this->ApplySnapshot(snapshot);
        }
      });

  auto cached = provider.GetCachedSnapshot();
  ApplySnapshot(cached);
}

// -------------------------
// Destructor
// -------------------------
NetworkPageViewModel::~NetworkPageViewModel() {
  if (m_subscriptionId != 0) {
    Services::NetworkStatusProvider::Instance().Unsubscribe(m_subscriptionId);
    m_subscriptionId = 0;
  }
}

// -------------------------
// Helper: Get Brush based on result
// -------------------------
Brush NetworkPageViewModel::GetResultBrush(winrt::hstring const &result) {
  if (result.starts_with(L"✔")) {
    return SolidColorBrush(Colors::LightGreen());
  }
  if (result.starts_with(L"✖") || result.starts_with(L"⚠")) {
    return SolidColorBrush(Colors::Orange());
  }
  return SolidColorBrush(Colors::Gray());
}

// -------------------------
// External IP Properties
// -------------------------
winrt::hstring NetworkPageViewModel::HttpExternalIp() const { return m_httpIp; }
void NetworkPageViewModel::HttpExternalIp(winrt::hstring const &value) {
  if (m_httpIp != value) { m_httpIp = value; RaisePropertyChanged(L"HttpExternalIp"); }
}

winrt::hstring NetworkPageViewModel::UpnpExternalIp() const { return m_upnpIp; }
void NetworkPageViewModel::UpnpExternalIp(winrt::hstring const &value) {
  if (m_upnpIp != value) { m_upnpIp = value; RaisePropertyChanged(L"UpnpExternalIp"); }
}

winrt::hstring NetworkPageViewModel::StunExternalIp() const { return m_stunIp; }
void NetworkPageViewModel::StunExternalIp(winrt::hstring const &value) {
  if (m_stunIp != value) { m_stunIp = value; RaisePropertyChanged(L"StunExternalIp"); }
}

winrt::hstring NetworkPageViewModel::VerificationStatus() const { return m_status; }
void NetworkPageViewModel::VerificationStatus(winrt::hstring const &value) {
  if (m_status != value) {
    m_status = value;
    RaisePropertyChanged(L"VerificationStatus");
    RaisePropertyChanged(L"VerificationStatusBrush");
  }
}

Brush NetworkPageViewModel::VerificationStatusBrush() {
  if (m_status.starts_with(L"⚠") || m_status == L"CGNAT") {
    return SolidColorBrush(Colors::Orange());
  }
  if (m_status == L"Open") {
    return SolidColorBrush(Colors::Green());
  }
  return SolidColorBrush(Colors::Gray());
}

// -------------------------
// NAT Analysis Properties (RFC 5780)
// -------------------------
winrt::hstring NetworkPageViewModel::NatType() const { return m_natType; }
void NetworkPageViewModel::NatType(winrt::hstring const &value) {
  if (m_natType != value) {
    m_natType = value;
    RaisePropertyChanged(L"NatType");
    RaisePropertyChanged(L"NatTypeBrush");
  }
}

winrt::hstring NetworkPageViewModel::NatTypeDescription() const { return m_natTypeDescription; }
void NetworkPageViewModel::NatTypeDescription(winrt::hstring const &value) {
  if (m_natTypeDescription != value) {
    m_natTypeDescription = value;
    RaisePropertyChanged(L"NatTypeDescription");
  }
}

winrt::hstring NetworkPageViewModel::StunServer() const { return m_stunServer; }
void NetworkPageViewModel::StunServer(winrt::hstring const &value) {
  if (m_stunServer != value) {
    m_stunServer = value;
    RaisePropertyChanged(L"StunServer");
  }
}

winrt::hstring NetworkPageViewModel::MappingBehavior() const { return m_mappingBehavior; }
void NetworkPageViewModel::MappingBehavior(winrt::hstring const &value) {
  if (m_mappingBehavior != value) {
    m_mappingBehavior = value;
    RaisePropertyChanged(L"MappingBehavior");
  }
}

winrt::hstring NetworkPageViewModel::FilteringBehavior() const { return m_filteringBehavior; }
void NetworkPageViewModel::FilteringBehavior(winrt::hstring const &value) {
  if (m_filteringBehavior != value) {
    m_filteringBehavior = value;
    RaisePropertyChanged(L"FilteringBehavior");
  }
}

Brush NetworkPageViewModel::NatTypeBrush() {
  if (m_natType == L"Open") {
    return SolidColorBrush(Colors::Green());
  }
  if (m_natType == L"Moderate") {
    return SolidColorBrush(Colors::Orange());
  }
  if (m_natType == L"Strict" || m_natType == L"UDP Blocked") {
    return SolidColorBrush(Colors::Red());
  }
  return SolidColorBrush(Colors::Gray());
}

// -------------------------
// Connectivity Tests Properties
// -------------------------
winrt::hstring NetworkPageViewModel::PingGatewayResult() const { return m_pingGateway; }
void NetworkPageViewModel::PingGatewayResult(winrt::hstring const &value) {
  if (m_pingGateway != value) {
    m_pingGateway = value;
    RaisePropertyChanged(L"PingGatewayResult");
    RaisePropertyChanged(L"PingGatewayBrush");
  }
}
Brush NetworkPageViewModel::PingGatewayBrush() { return GetResultBrush(m_pingGateway); }

winrt::hstring NetworkPageViewModel::PingGoogleResult() const { return m_pingGoogle; }
void NetworkPageViewModel::PingGoogleResult(winrt::hstring const &value) {
  if (m_pingGoogle != value) {
    m_pingGoogle = value;
    RaisePropertyChanged(L"PingGoogleResult");
    RaisePropertyChanged(L"PingGoogleBrush");
  }
}
Brush NetworkPageViewModel::PingGoogleBrush() { return GetResultBrush(m_pingGoogle); }

winrt::hstring NetworkPageViewModel::HttpReachabilityResult() const { return m_httpReachability; }
void NetworkPageViewModel::HttpReachabilityResult(winrt::hstring const &value) {
  if (m_httpReachability != value) {
    m_httpReachability = value;
    RaisePropertyChanged(L"HttpReachabilityResult");
    RaisePropertyChanged(L"HttpReachabilityBrush");
  }
}
Brush NetworkPageViewModel::HttpReachabilityBrush() { return GetResultBrush(m_httpReachability); }

bool NetworkPageViewModel::IsRunningTests() const { return m_isRunningTests; }
void NetworkPageViewModel::IsRunningTests(bool value) {
  if (m_isRunningTests != value) {
    m_isRunningTests = value;
    RaisePropertyChanged(L"IsRunningTests");
  }
}

// -------------------------
// DNS Diagnostics Properties
// -------------------------
winrt::hstring NetworkPageViewModel::SystemDnsResult() const { return m_systemDns; }
void NetworkPageViewModel::SystemDnsResult(winrt::hstring const &value) {
  if (m_systemDns != value) { m_systemDns = value; RaisePropertyChanged(L"SystemDnsResult"); }
}

winrt::hstring NetworkPageViewModel::IspDnsResult() const { return m_ispDns; }
void NetworkPageViewModel::IspDnsResult(winrt::hstring const &value) {
  if (m_ispDns != value) {
    m_ispDns = value;
    RaisePropertyChanged(L"IspDnsResult");
    RaisePropertyChanged(L"IspDnsBrush");
  }
}
Brush NetworkPageViewModel::IspDnsBrush() { return GetResultBrush(m_ispDns); }

winrt::hstring NetworkPageViewModel::PublicDnsResult() const { return m_publicDns; }
void NetworkPageViewModel::PublicDnsResult(winrt::hstring const &value) {
  if (m_publicDns != value) { m_publicDns = value; RaisePropertyChanged(L"PublicDnsResult"); }
}

// -------------------------
// Port Test Properties
// -------------------------
winrt::hstring NetworkPageViewModel::PortTestResult() const { return m_portTestResult; }
void NetworkPageViewModel::PortTestResult(winrt::hstring const &value) {
  if (m_portTestResult != value) {
    m_portTestResult = value;
    RaisePropertyChanged(L"PortTestResult");
    RaisePropertyChanged(L"PortTestBrush");
  }
}
Brush NetworkPageViewModel::PortTestBrush() { return GetResultBrush(m_portTestResult); }

winrt::hstring NetworkPageViewModel::ExternalPortTestResult() const { return m_externalPortTestResult; }
void NetworkPageViewModel::ExternalPortTestResult(winrt::hstring const &value) {
  if (m_externalPortTestResult != value) {
    m_externalPortTestResult = value;
    RaisePropertyChanged(L"ExternalPortTestResult");
    RaisePropertyChanged(L"ExternalPortTestBrush");
  }
}
Brush NetworkPageViewModel::ExternalPortTestBrush() { return GetResultBrush(m_externalPortTestResult); }

bool NetworkPageViewModel::IsTestingPort() const { return m_isTestingPort; }
void NetworkPageViewModel::IsTestingPort(bool value) {
  if (m_isTestingPort != value) {
    m_isTestingPort = value;
    RaisePropertyChanged(L"IsTestingPort");
  }
}

// -------------------------
// Apply Snapshot from Provider
// -------------------------
void NetworkPageViewModel::ApplySnapshot(Services::NetworkSnapshot const &snapshot) {
  m_httpIp = (snapshot.httpWanIp.empty() || snapshot.httpWanIp == L"-") ? L"-" : snapshot.httpWanIp;
  m_upnpIp = snapshot.upnpWanIp.empty() ? L"(Not available)" : snapshot.upnpWanIp;
  m_stunIp = snapshot.stunWanIp.empty() ? L"(Not available)" : snapshot.stunWanIp;

  if (m_httpIp != L"-" && m_upnpIp != L"(Not available)" && m_httpIp != m_upnpIp) {
    m_status = L"⚠ Possible CGNAT detected";
  } else if (snapshot.cgnatStatus.empty() || snapshot.isIpFallback) {
    m_status = L"Unknown";
  } else {
    m_status = snapshot.cgnatStatus;
  }

  // NAT Analysis (RFC 5780)
  m_natType = snapshot.natType.empty() ? L"Unknown" : snapshot.natType;
  m_natTypeDescription = snapshot.natTypeDescription.empty() ? L"Not analyzed" : snapshot.natTypeDescription;
  m_stunServer = snapshot.stunServer.empty() ? L"-" : snapshot.stunServer;
  m_mappingBehavior = snapshot.mappingBehavior.empty() ? L"Unknown" : snapshot.mappingBehavior;
  m_filteringBehavior = snapshot.filteringBehavior.empty() ? L"Unknown" : snapshot.filteringBehavior;

  RaiseAll();
}

// -------------------------
// Async Actions
// -------------------------
IAsyncAction NetworkPageViewModel::RefreshAsync() {
  co_await Services::NetworkStatusProvider::Instance().RefreshAsync();
}

IAsyncAction NetworkPageViewModel::RunConnectivityTestsAsync() {
  auto strong_this = get_strong();
  apartment_context ui_thread;

  IsRunningTests(true);
  PingGatewayResult(L"Testing...");
  PingGoogleResult(L"Testing...");
  HttpReachabilityResult(L"Testing...");
  SystemDnsResult(L"Testing...");
  IspDnsResult(L"Testing...");
  PublicDnsResult(L"Testing...");

  co_await resume_background();

  auto results = Services::NetworkConnectivityTestService::Run();

  co_await ui_thread;
  PingGatewayResult(results.pingGateway);
  PingGoogleResult(results.pingGoogle);
  HttpReachabilityResult(results.httpReachability);
  SystemDnsResult(results.systemDns);
  IspDnsResult(results.ispDns);
  PublicDnsResult(results.publicDns);
  IsRunningTests(false);
}

IAsyncAction NetworkPageViewModel::TestPortAsync(int32_t port) {
  auto strong_this = get_strong();
  apartment_context ui_thread;

  if (port <= 0 || port > 65535) {
    PortTestResult(L"✖ Invalid port number");
    co_return;
  }

  IsTestingPort(true);
  PortTestResult(L"Testing port " + to_hstring(port) + L" locally...");

  co_await resume_background();

  auto result = Services::PortReachabilityService::TestLocalPort(port);

  co_await ui_thread;
  PortTestResult(result);
  IsTestingPort(false);
}

IAsyncAction NetworkPageViewModel::TestExternalPortAsync(int32_t port) {
  auto strong_this = get_strong();
  apartment_context ui_thread;

  if (port <= 0 || port > 65535) {
    ExternalPortTestResult(L"✖ Invalid port number");
    co_return;
  }

  IsTestingPort(true);
  ExternalPortTestResult(L"Checking port " + to_hstring(port) + L" from internet...");

  auto externalIp = m_httpIp;
  co_await resume_background();

  auto result =
      Services::PortReachabilityService::TestExternalPort(port, externalIp);

  co_await ui_thread;
  ExternalPortTestResult(result);
  IsTestingPort(false);
}

// -------------------------
// INotifyPropertyChanged
// -------------------------
event_token NetworkPageViewModel::PropertyChanged(PropertyChangedEventHandler const &handler) {
  return m_propertyChanged.add(handler);
}

void NetworkPageViewModel::PropertyChanged(event_token const &token) noexcept {
  m_propertyChanged.remove(token);
}

void NetworkPageViewModel::RaisePropertyChanged(std::wstring_view name) {
  m_propertyChanged(*this, PropertyChangedEventArgs{hstring{name}});
}

void NetworkPageViewModel::RaiseAll() {
  RaisePropertyChanged(L"HttpExternalIp");
  RaisePropertyChanged(L"UpnpExternalIp");
  RaisePropertyChanged(L"StunExternalIp");
  RaisePropertyChanged(L"VerificationStatus");
  RaisePropertyChanged(L"VerificationStatusBrush");
  RaisePropertyChanged(L"NatType");
  RaisePropertyChanged(L"NatTypeBrush");
  RaisePropertyChanged(L"NatTypeDescription");
  RaisePropertyChanged(L"StunServer");
  RaisePropertyChanged(L"MappingBehavior");
  RaisePropertyChanged(L"FilteringBehavior");
}

} // namespace winrt::EtherealScepter::ViewModels::implementation
