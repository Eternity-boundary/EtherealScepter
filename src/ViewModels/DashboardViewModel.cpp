//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"

#include <winrt/base.h>

#include "include/ViewModels/DashboardViewModel.h"
#include "ViewModels.DashboardViewModel.g.cpp"

namespace winrt::EtherealScepter::ViewModels::implementation
{
    DashboardViewModel::DashboardViewModel()
		: m_ui{}
    {
        RefreshAsync();
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
        m_propertyChanged(
            *this,
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ name }
        );
    }

    winrt::Windows::Foundation::IAsyncAction DashboardViewModel::RefreshAsync()
    {
        if (m_refreshing)
            co_return;

        m_refreshing = true;

        try {
            co_await winrt::resume_background();
            //TODO: Replace with actual network status retrieval logic
            winrt::hstring network = L"Connected";
            winrt::hstring upnp = L"Enabled";
            winrt::hstring nat = L"Open";
            winrt::hstring summary = L"Active Port Mappings: 3";

            co_await winrt::resume_after(std::chrono::seconds(1));

            co_await m_ui;

            m_networkStatus = network;
            m_upnpStatus = upnp;
            m_natType = nat;
            m_summary = summary;

            RaisePropertyChanged(L"NetworkStatus");
            RaisePropertyChanged(L"UpnpStatus");
            RaisePropertyChanged(L"NatType");
            RaisePropertyChanged(L"SummaryText");
        }
        catch (...)
        {
            // Handle exceptions as needed
		}

        m_refreshing = false;
    }

    winrt::hstring DashboardViewModel::NetworkStatus() { return m_networkStatus; }
    winrt::hstring DashboardViewModel::UpnpStatus() { return m_upnpStatus; }
    winrt::hstring DashboardViewModel::NatType() { return m_natType; }
    winrt::hstring DashboardViewModel::SummaryText() { return m_summary; }
}
