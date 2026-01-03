//Created by: EternityBoundary on Jan 3, 2025
#pragma once
#include "ViewModels.DashboardViewModel.g.h"
#include <winrt/Microsoft.UI.Xaml.Data.h>

namespace winrt::EtherealScepter::ViewModels::implementation
{
    struct DashboardViewModel : DashboardViewModelT<DashboardViewModel>
    {
        DashboardViewModel();

        winrt::hstring NetworkStatus();
        winrt::hstring UpnpStatus();
        winrt::hstring NatType();
        winrt::hstring SummaryText();

        void Refresh();

        winrt::event_token PropertyChanged(
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token);

    private:
        void RaisePropertyChanged(winrt::hstring const& name);

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        winrt::hstring m_networkStatus;
        winrt::hstring m_upnpStatus;
        winrt::hstring m_natType;
        winrt::hstring m_summary;
    };
}

namespace winrt::EtherealScepter::ViewModels::factory_implementation
{
    struct DashboardViewModel
        : DashboardViewModelT<DashboardViewModel, implementation::DashboardViewModel>
    {
    };
}
