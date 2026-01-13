//Created by: EternityBoundary on Jan 6, 2026
#include "pch.h"
#include "include/ViewModels/UpnpViewModel.h"
#include "ViewModels.UpnpViewModel.g.cpp"

using namespace winrt;
using namespace Microsoft::UI::Xaml::Data;

namespace winrt::EtherealScepter::ViewModels::implementation
{
    UpnpViewModel::UpnpViewModel(winrt::EtherealScepter::ViewModels::DashboardViewModel const& dashboard)
        : m_dashboard(dashboard)
    {
        // 把 Dashboard 的變化轉發給 UPnP Page
        m_dashboardRevoker = m_dashboard.PropertyChanged(auto_revoke,
            [this](auto&&, PropertyChangedEventArgs const& e)
            {
                auto name = e.PropertyName();

                // 這些屬性變更時，UPnP Page刷新對應 x:Bind
                if (name == L"UpnpDevices")
                {
                    RaisePropertyChanged(L"Devices");
                    RaisePropertyChanged(L"NumberOfUPnPDevice");
                }
                else if (name == L"UpnpStatus")
                {
                    RaisePropertyChanged(L"UpnpStatus");
                }
                else if (name == L"UpnpStatusBrush")
                {
                    RaisePropertyChanged(L"UpnpStatusBrush");
                }
                else if (name == L"NumberOfUPnPDevice")
                {
                    RaisePropertyChanged(L"NumberOfUPnPDevice");
                }
            });
    }

    event_token UpnpViewModel::PropertyChanged(PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void UpnpViewModel::PropertyChanged(event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void UpnpViewModel::RaisePropertyChanged(hstring const& name)
    {
        m_propertyChanged(*this, PropertyChangedEventArgs{ name });
    }

    Windows::Foundation::Collections::IObservableVector<EtherealScepter::Models::UpnpDeviceInfo>
        UpnpViewModel::Devices() const
    {
        return m_dashboard.UpnpDevices();
    }

    hstring UpnpViewModel::UpnpStatus() const
    {
        return m_dashboard.UpnpStatus();
    }

    Microsoft::UI::Xaml::Media::Brush UpnpViewModel::UpnpStatusBrush() const
    {
        return m_dashboard.UpnpStatusBrush();
    }

    hstring UpnpViewModel::NumberOfUPnPDevice() const
    {
        return m_dashboard.NumberOfUPnPDevice();
    }

    winrt::Microsoft::UI::Xaml::Visibility UpnpViewModel::BoolToVisibility(bool value)
    {
        return value
            ? winrt::Microsoft::UI::Xaml::Visibility::Visible
            : winrt::Microsoft::UI::Xaml::Visibility::Collapsed;
    }
}
