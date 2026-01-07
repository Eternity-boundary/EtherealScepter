//Created by: EternityBoundary on Jan 6, 2025
#pragma once
#include "ViewModels.UpnpViewModel.g.h"

#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <winrt/EtherealScepter.Models.h>

namespace winrt::EtherealScepter::ViewModels::implementation
{
    struct UpnpViewModel : UpnpViewModelT<UpnpViewModel>
    {
        UpnpViewModel(winrt::EtherealScepter::ViewModels::DashboardViewModel const& dashboard);

        // INotifyPropertyChanged
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        // Properties for x:Bind
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::EtherealScepter::Models::UpnpDeviceInfo>
            Devices() const;

        winrt::hstring UpnpStatus() const;
        winrt::Microsoft::UI::Xaml::Media::Brush UpnpStatusBrush() const;

        winrt::hstring NumberOfUPnPDevice() const;

        winrt::Microsoft::UI::Xaml::Visibility BoolToVisibility(bool value);

    private:
        void RaisePropertyChanged(winrt::hstring const& name);

        winrt::EtherealScepter::ViewModels::DashboardViewModel m_dashboard{ nullptr };
        winrt::EtherealScepter::ViewModels::DashboardViewModel::PropertyChanged_revoker m_dashboardRevoker;

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::EtherealScepter::ViewModels::factory_implementation
{
    struct UpnpViewModel : UpnpViewModelT<UpnpViewModel, implementation::UpnpViewModel>
    {
    };
}
