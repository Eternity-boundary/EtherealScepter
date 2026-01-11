// Created by: EternityBoundary on Jan 3, 2025
#pragma once

#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include "ViewModels.DashboardViewModel.g.h"
#include "include/Services/NetworkStatusService.h"
#include "winrt/EtherealScepter.Models.h"

namespace winrt::EtherealScepter::ViewModels::implementation {
struct DashboardViewModel : DashboardViewModelT<DashboardViewModel> {
  DashboardViewModel();
  ~DashboardViewModel();

  winrt::hstring NetworkStatus();
  winrt::hstring UpnpStatus();
  winrt::hstring NatType();
  winrt::hstring SummaryText();
  winrt::hstring NumberOfUPnPDevice();
  winrt::hstring IsPortForwardingAvailable();
  winrt::hstring LocalIp();
  winrt::hstring WanIp();
  winrt::hstring CgnatStatus();
  winrt::hstring LastRefreshText();
  winrt::hstring StatusMessage();
  winrt::Microsoft::UI::Xaml::Media::Brush StatusBrush();

  winrt::Windows::Foundation::Collections::IObservableVector<
      winrt::EtherealScepter::Models::UpnpDeviceInfo>
  UpnpDevices() const {
    return m_upnpDevices;
  }

  winrt::Windows::Foundation::IAsyncAction RefreshAsync();

  winrt::event_token PropertyChanged(
      winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const
          &handler);

  void PropertyChanged(winrt::event_token const &token);

  winrt::Microsoft::UI::Xaml::Media::Brush NetworkStatusBrush();
  winrt::Microsoft::UI::Xaml::Media::Brush UpnpStatusBrush();
  winrt::Microsoft::UI::Xaml::Media::Brush NatTypeBrush();
  winrt::Microsoft::UI::Xaml::Media::Brush CgnatStatusBrush();

private:
  void RaisePropertyChanged(winrt::hstring const &name);
  void RaiseAll();
  void
  ApplySnapshot(::EtherealScepter::Services::NetworkSnapshot const &snapshot);

  winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler>
      m_propertyChanged;

  winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer m_refreshTimer{
      nullptr};

  bool m_refreshing{false};

  winrt::hstring m_networkStatus;
  winrt::hstring m_upnpStatus;
  winrt::hstring m_natType;
  winrt::hstring m_summary;
  winrt::hstring m_localIp;
  winrt::hstring m_wanIp;
  winrt::hstring m_cgnatStatus;
  winrt::hstring m_numberOfUPnPDevice;
  winrt::hstring m_isPortForwardingAvailable;

  winrt::hstring m_lastRefreshText{L"-"};
  winrt::hstring m_statusMessage{L"Idle"};
  bool m_hasError{false};

  winrt::Windows::Foundation::Collections::IObservableVector<
      winrt::EtherealScepter::Models::UpnpDeviceInfo>
      m_upnpDevices{winrt::single_threaded_observable_vector<
          winrt::EtherealScepter::Models::UpnpDeviceInfo>()};

  uint64_t m_subscriptionId{0};
};
} // namespace winrt::EtherealScepter::ViewModels::implementation

namespace winrt::EtherealScepter::ViewModels::factory_implementation {
struct DashboardViewModel
    : DashboardViewModelT<DashboardViewModel,
                          implementation::DashboardViewModel> {};
} // namespace winrt::EtherealScepter::ViewModels::factory_implementation
