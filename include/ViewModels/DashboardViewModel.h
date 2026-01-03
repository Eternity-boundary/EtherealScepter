//Created by: EternityBoundary on Jan 3, 2025
#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/base.h>

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

        winrt::Windows::Foundation::IAsyncAction RefreshAsync();

        winrt::event_token PropertyChanged(
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);

        void PropertyChanged(winrt::event_token const& token);



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
