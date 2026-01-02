//Created by: EternityBoundary on Jan 3, 2025
#pragma once
#include <winrt/Microsoft.UI.Xaml.Data.h>

namespace winrt::EtherealScepter::ViewModels
{
    struct DashboardViewModel :
        winrt::implements<
        DashboardViewModel,
        Microsoft::UI::Xaml::Data::INotifyPropertyChanged>
    {
        DashboardViewModel();

        // Properties
        winrt::hstring NetworkStatus();
        winrt::hstring UpnpStatus();
        winrt::hstring NatType();
        winrt::hstring SummaryText();

        void Refresh();

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler>
            PropertyChanged;

    private:
        void RaisePropertyChanged(hstring const& name);

        winrt::hstring m_networkStatus;
        winrt::hstring m_upnpStatus;
        winrt::hstring m_natType;
        winrt::hstring m_summary;
    };
}
