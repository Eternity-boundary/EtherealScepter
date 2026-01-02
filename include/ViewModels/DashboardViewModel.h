//Created by: EternityBoundary on Jan 3, 2025
#pragma once
#include <Generated Files/winrt/Microsoft.UI.Xaml.Data.h>

namespace EtherealScepter::ViewModels
{
    struct DashboardViewModel :
        winrt::implements<DashboardViewModel, winrt::Microsoft::UI::Xaml::Data::INotifyPropertyChanged>
    {
        DashboardViewModel();

        winrt::hstring NetworkStatus() const;
        winrt::hstring UpnpStatus() const;
        winrt::hstring NatType() const;
        winrt::hstring SummaryText() const;

        void Refresh();

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value) noexcept;
        void PropertyChanged(winrt::event_token const& token) noexcept;

    private:
        void RaisePropertyChanged(winrt::hstring const& name);

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        winrt::hstring m_networkStatus;
        winrt::hstring m_upnpStatus;
        winrt::hstring m_natType;
        winrt::hstring m_summary;
    };
}
