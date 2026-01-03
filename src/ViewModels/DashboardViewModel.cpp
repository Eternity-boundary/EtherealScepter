//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"

#include <winrt/base.h>

#include "include/ViewModels/DashboardViewModel.h"
#include "ViewModels.DashboardViewModel.g.cpp"

using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Xaml::Media;

namespace winrt::EtherealScepter::ViewModels::implementation
{
    Brush DashboardViewModel::NetworkStatusBrush()
    {
        if (m_networkStatus == L"Connected")
            return SolidColorBrush(Colors::Green());
        if (m_networkStatus == L"Disconnected")
            return SolidColorBrush(Colors::Red());

        return SolidColorBrush(Colors::Gray());
    }

    Brush DashboardViewModel::UpnpStatusBrush()
    {
        if (m_upnpStatus == L"Enabled")
            return SolidColorBrush(Colors::Green());
        if (m_upnpStatus == L"Disabled")
            return SolidColorBrush(Colors::Orange());

        return SolidColorBrush(Colors::Gray());
    }

    Brush DashboardViewModel::NatTypeBrush()
    {
        if (m_natType == L"Open")
            return SolidColorBrush(Colors::Green());
        if (m_natType == L"Moderate")
            return SolidColorBrush(Colors::Orange());
        if (m_natType == L"Strict")
            return SolidColorBrush(Colors::Red());

        return SolidColorBrush(Colors::Gray());
    }

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
            winrt::hstring IPAddress = L"10.0.0.0";
			winrt::hstring numberOfUPnPDevice = L"2 Devices Found";
			winrt::hstring isPortForwardingAvailable = L"Port Forwarding Available";

            co_await winrt::resume_after(std::chrono::seconds(1));

            co_await m_ui;

            m_networkStatus = network;
            m_upnpStatus = upnp;
            m_natType = nat;
            m_summary = summary;
			m_IPAddress = IPAddress;
			m_numberOfUPnPDevice = numberOfUPnPDevice;
			m_isPortForwardingAvailable = isPortForwardingAvailable;

            RaisePropertyChanged(L"NetworkStatus");
            RaisePropertyChanged(L"NetworkStatusBrush");

            RaisePropertyChanged(L"UpnpStatus");
            RaisePropertyChanged(L"UpnpStatusBrush");

            RaisePropertyChanged(L"NatType");
            RaisePropertyChanged(L"NatTypeBrush");

            RaisePropertyChanged(L"SummaryText");
			RaisePropertyChanged(L"IPAddress");
			RaisePropertyChanged(L"NumberOfUPnPDevice");
			RaisePropertyChanged(L"IsPortForwardingAvailable");
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
	winrt::hstring DashboardViewModel::IPAddress() { return m_IPAddress; }
	winrt::hstring DashboardViewModel::NumberOfUPnPDevice() { return m_numberOfUPnPDevice; }
	winrt::hstring DashboardViewModel::IsPortForwardingAvailable() { return m_isPortForwardingAvailable; }
}
