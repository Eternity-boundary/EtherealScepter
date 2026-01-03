//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "include/ViewModels/DashboardViewModel.h"
#include "ViewModels.DashboardViewModel.g.cpp"

namespace winrt::EtherealScepter::ViewModels::implementation
{
    DashboardViewModel::DashboardViewModel()
    {
        Refresh();
    }

    winrt::event_token DashboardViewModel::PropertyChanged(
        winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void DashboardViewModel::PropertyChanged(winrt::event_token const& token)
    {
        m_propertyChanged.remove(token);
    }

    void DashboardViewModel::RaisePropertyChanged(winrt::hstring const& name)
    {
        m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ name });
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

    winrt::hstring DashboardViewModel::NetworkStatus() { return m_networkStatus; }
    winrt::hstring DashboardViewModel::UpnpStatus() { return m_upnpStatus; }
    winrt::hstring DashboardViewModel::NatType() { return m_natType; }
    winrt::hstring DashboardViewModel::SummaryText() { return m_summary; }
}
