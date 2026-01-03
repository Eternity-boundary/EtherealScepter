//Created by: EternityBoundary on Jan 3, 2025
#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

#include "ViewModels.DashboardViewModel.g.h"

namespace winrt::EtherealScepter::ViewModels::implementation
{
    struct DashboardViewModel : DashboardViewModelT<DashboardViewModel>
    {
        DashboardViewModel();

        winrt::hstring NetworkStatus();
        winrt::hstring UpnpStatus();
        winrt::hstring NatType();
        winrt::hstring SummaryText();
		winrt::hstring NumberOfUPnPDevice();
		winrt::hstring IsPortForwardingAvailable();
		winrt::hstring LocalIp();
		winrt::hstring WanIp();
		winrt::hstring CgnatStatus();

        winrt::Windows::Foundation::IAsyncAction RefreshAsync();

        winrt::event_token PropertyChanged(
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);

        void PropertyChanged(winrt::event_token const& token);

        winrt::Microsoft::UI::Xaml::Media::Brush NetworkStatusBrush();
        winrt::Microsoft::UI::Xaml::Media::Brush UpnpStatusBrush();
        winrt::Microsoft::UI::Xaml::Media::Brush NatTypeBrush();

    private:
        void RaisePropertyChanged(winrt::hstring const& name);

        winrt::event<
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler
        > m_propertyChanged;

        winrt::Microsoft::UI::Dispatching::DispatcherQueue m_dispatcher{ nullptr };

        winrt::apartment_context m_ui;
        bool m_refreshing{ false };

        winrt::hstring m_networkStatus;
        winrt::hstring m_upnpStatus;
        winrt::hstring m_natType;
        winrt::hstring m_summary;
        winrt::hstring m_localIp;
		winrt::hstring m_wanIp;
		winrt::hstring m_cgnatStatus;
        winrt::hstring m_numberOfUPnPDevice;
        winrt::hstring m_isPortForwardingAvailable;

    };
}

namespace winrt::EtherealScepter::ViewModels::factory_implementation
{
    struct DashboardViewModel
        : DashboardViewModelT<
        DashboardViewModel,
        implementation::DashboardViewModel
        >
    {
    };
}
