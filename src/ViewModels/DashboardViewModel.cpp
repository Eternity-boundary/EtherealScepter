//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "include/ViewModels/DashboardViewModel.h"

using namespace winrt;

namespace EtherealScepter::ViewModels
{
    DashboardViewModel::DashboardViewModel()
    {
        Refresh();
    }

    event_token DashboardViewModel::PropertyChanged(
        Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value) noexcept
    {
        return m_propertyChanged.add(value);
    }

	void DashboardViewModel::PropertyChanged(event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void DashboardViewModel::RaisePropertyChanged(hstring const& name)
    {
        m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ name });
    }

    void DashboardViewModel::Refresh()
    {
        m_networkStatus = L"Connected";
        m_upnpStatus = L"Enabled";
        m_natType = L"Open";
        m_summary = L"Active Port Mappings: 3";

        RaisePropertyChanged(L"NetworkStatus");
        RaisePropertyChanged(L"UpnpStatus");
        RaisePropertyChanged(L"NatType");
        RaisePropertyChanged(L"SummaryText");
    }

    hstring DashboardViewModel::NetworkStatus() const { return m_networkStatus; }
    hstring DashboardViewModel::UpnpStatus() const { return m_upnpStatus; }
    hstring DashboardViewModel::NatType() const { return m_natType; }
    hstring DashboardViewModel::SummaryText() const { return m_summary; }
}
