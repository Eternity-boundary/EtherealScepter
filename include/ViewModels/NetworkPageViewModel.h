// Created by: EternityBoundary on Jan 12, 2026
#pragma once

#include <string_view>
#include <winrt/base.h>

#include "ViewModels.NetworkPageViewModel.g.h"
#include "include/Services/NetworkStatusService.h"
#include <winrt/impl/Microsoft.UI.Dispatching.2.h>
#include <winrt/impl/Microsoft.UI.Xaml.Data.2.h>
#include <winrt/impl/Microsoft.UI.Xaml.Media.2.h>
#include <winrt/impl/Windows.Foundation.1.h>

namespace winrt::EtherealScepter::ViewModels::implementation {
struct NetworkPageViewModel : NetworkPageViewModelT<NetworkPageViewModel> {
  NetworkPageViewModel();
  ~NetworkPageViewModel();

  // --- External IP Verification ---
  winrt::hstring HttpExternalIp() const;
  void HttpExternalIp(winrt::hstring const &value);
  winrt::hstring UpnpExternalIp() const;
  void UpnpExternalIp(winrt::hstring const &value);
  winrt::hstring StunExternalIp() const;
  void StunExternalIp(winrt::hstring const &value);
  winrt::hstring VerificationStatus() const;
  void VerificationStatus(winrt::hstring const &value);
  winrt::Microsoft::UI::Xaml::Media::Brush VerificationStatusBrush();

  // --- NAT Analysis (RFC 5780) ---
  winrt::hstring NatType() const;
  void NatType(winrt::hstring const &value);
  winrt::hstring NatTypeDescription() const;
  void NatTypeDescription(winrt::hstring const &value);
  winrt::hstring StunServer() const;
  void StunServer(winrt::hstring const &value);
  winrt::hstring MappingBehavior() const;
  void MappingBehavior(winrt::hstring const &value);
  winrt::hstring FilteringBehavior() const;
  void FilteringBehavior(winrt::hstring const &value);
  winrt::Microsoft::UI::Xaml::Media::Brush NatTypeBrush();

  // --- Connectivity Tests ---
  winrt::hstring PingGatewayResult() const;
  void PingGatewayResult(winrt::hstring const &value);
  winrt::Microsoft::UI::Xaml::Media::Brush PingGatewayBrush();
  winrt::hstring PingGoogleResult() const;
  void PingGoogleResult(winrt::hstring const &value);
  winrt::Microsoft::UI::Xaml::Media::Brush PingGoogleBrush();
  winrt::hstring HttpReachabilityResult() const;
  void HttpReachabilityResult(winrt::hstring const &value);
  winrt::Microsoft::UI::Xaml::Media::Brush HttpReachabilityBrush();
  bool IsRunningTests() const;
  void IsRunningTests(bool value);

  // --- DNS Diagnostics ---
  winrt::hstring SystemDnsResult() const;
  void SystemDnsResult(winrt::hstring const &value);
  winrt::hstring IspDnsResult() const;
  void IspDnsResult(winrt::hstring const &value);
  winrt::Microsoft::UI::Xaml::Media::Brush IspDnsBrush();
  winrt::hstring PublicDnsResult() const;
  void PublicDnsResult(winrt::hstring const &value);

  // --- Port Reachability ---
  winrt::hstring PortTestResult() const;
  void PortTestResult(winrt::hstring const &value);
  winrt::Microsoft::UI::Xaml::Media::Brush PortTestBrush();
  winrt::hstring ExternalPortTestResult() const;
  void ExternalPortTestResult(winrt::hstring const &value);
  winrt::Microsoft::UI::Xaml::Media::Brush ExternalPortTestBrush();
  bool IsTestingPort() const;
  void IsTestingPort(bool value);

  // --- Actions ---
  winrt::Windows::Foundation::IAsyncAction RefreshAsync();
  winrt::Windows::Foundation::IAsyncAction RunConnectivityTestsAsync();
  winrt::Windows::Foundation::IAsyncAction TestPortAsync(int32_t port);
  winrt::Windows::Foundation::IAsyncAction TestExternalPortAsync(int32_t port);

  // --- INotifyPropertyChanged ---
  winrt::event_token PropertyChanged(
      winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler);
  void PropertyChanged(winrt::event_token const &token) noexcept;

private:
  void RaisePropertyChanged(std::wstring_view name);
  void RaiseAll();
  void ApplySnapshot(::EtherealScepter::Services::NetworkSnapshot const &snapshot);
  winrt::Microsoft::UI::Xaml::Media::Brush GetResultBrush(winrt::hstring const &result);

  winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

  // External IP
  winrt::hstring m_httpIp{L"-"};
  winrt::hstring m_upnpIp{L"(Not available)"};
  winrt::hstring m_stunIp{L"(Not available)"};
  winrt::hstring m_status{L"Not checked"};

  // NAT Analysis (RFC 5780)
  winrt::hstring m_natType{L"Unknown"};
  winrt::hstring m_natTypeDescription{L"Not analyzed"};
  winrt::hstring m_stunServer{L"-"};
  winrt::hstring m_mappingBehavior{L"Unknown"};
  winrt::hstring m_filteringBehavior{L"Unknown"};

  // Connectivity Tests
  winrt::hstring m_pingGateway{L"Not tested"};
  winrt::hstring m_pingGoogle{L"Not tested"};
  winrt::hstring m_httpReachability{L"Not tested"};
  bool m_isRunningTests{false};

  // DNS Diagnostics
  winrt::hstring m_systemDns{L"Not tested"};
  winrt::hstring m_ispDns{L"Not tested"};
  winrt::hstring m_publicDns{L"Not tested"};

  // Port Test
  winrt::hstring m_portTestResult{L"Enter a port to test"};
  winrt::hstring m_externalPortTestResult{L"Test if port is open from internet"};
  bool m_isTestingPort{false};

  uint64_t m_subscriptionId{0};
};
} // namespace winrt::EtherealScepter::ViewModels::implementation

namespace winrt::EtherealScepter::ViewModels::factory_implementation {
struct NetworkPageViewModel
    : NetworkPageViewModelT<NetworkPageViewModel,
                            implementation::NetworkPageViewModel> {};
} // namespace winrt::EtherealScepter::ViewModels::factory_implementation
