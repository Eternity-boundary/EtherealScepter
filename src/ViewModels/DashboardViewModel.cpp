//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"

#include "include/ViewModels/DashboardViewModel.h"
#include "ViewModels.DashboardViewModel.g.cpp"
#include "include/Services/NetworkStatusService.h"

using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace EtherealScepter::Services;
using EtherealScepter::Services::NetworkStatusService;

namespace winrt::EtherealScepter::ViewModels::implementation
{
    Brush DashboardViewModel::NetworkStatusBrush()
    {
        if (m_networkStatus == L"Internet")
            return SolidColorBrush(Colors::Green());
        if (m_networkStatus == L"Local Network")
            return SolidColorBrush(Colors::Yellow());
        if (m_networkStatus == L"Disconnected")
            return SolidColorBrush(Colors::Red());

        return SolidColorBrush(Colors::Gray());
    }

    Brush DashboardViewModel::CgnatStatusBrush()
    {
        if (m_cgnatStatus == L"CGNAT")
            return SolidColorBrush(Colors::Red());
        if (m_cgnatStatus == L"Open")
            return SolidColorBrush(Colors::Green());
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

    Brush DashboardViewModel::StatusBrush()
    {
        if (m_hasError)
            return SolidColorBrush(Colors::Red());

        return SolidColorBrush(Colors::LightGreen());
    }

	// Constructor
    DashboardViewModel::DashboardViewModel()
		: m_ui{}
    {
		RefreshAsync();
        auto dispatcherQueue =
            winrt::Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

        m_refreshTimer = dispatcherQueue.CreateTimer();
        m_refreshTimer.Interval(std::chrono::seconds(30));
        m_refreshTimer.IsRepeating(true);

        m_refreshTimer.Tick([this](auto&&, auto&&)
            {
                RefreshAsync();
            });

        m_refreshTimer.Start();
    }

    DashboardViewModel::~DashboardViewModel()
    {
        if (m_refreshTimer)
        {
            m_refreshTimer.Stop();
            m_refreshTimer = nullptr;
        }
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

    winrt::Windows::Foundation::IAsyncAction
        DashboardViewModel::RefreshAsync()
    {
        if (m_refreshing)
            co_return;

        m_refreshing = true;

        NetworkSnapshot snapshot{};
        bool hasError = false;

        // ===== 1. 先在 UI thread 顯示 loading =====
        {
            m_summary = L"Refreshing…";
			m_hasError = false;
            RaisePropertyChanged(L"SummaryText");
			RaisePropertyChanged(L"StatusBrush");
        }

        // ===== 2. 背景執行=====
        try
        {
            co_await winrt::resume_background();
            snapshot = NetworkStatusService::Query();
        }
        catch (...)
        {
            hasError = true;
        }

        // ===== 3. 回 UI thread=====
        co_await m_ui;

        if (hasError)
        {
            // UI-safe error handling
            m_summary = L"Network status unavailable";
            m_statusMessage = L"Error occurred";
            m_hasError = true;
            RaisePropertyChanged(L"SummaryText");
			RaisePropertyChanged(L"StatusMessage");

            m_refreshing = false;
            co_return;
        }

        // ===== 4. 正常套用 snapshot =====
        m_networkStatus = snapshot.networkStatus;
        m_upnpStatus = snapshot.upnpStatus;
        m_natType = snapshot.natType;
        m_summary = snapshot.summary;

        m_localIp = snapshot.localIp;
        m_wanIp = snapshot.wanIp;
        m_cgnatStatus = snapshot.cgnatStatus;

        m_numberOfUPnPDevice = snapshot.upnpDeviceCount;
        m_isPortForwardingAvailable = snapshot.portForwardingStatus;

        m_upnpDevices.Clear();
        for (auto const& d : snapshot.upnpDevices)
        {
            m_upnpDevices.Append(d);
        }
        

        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);

        std::tm tmLocal{};
        localtime_s(&tmLocal, &t);

        wchar_t buf[64]{};
        wcsftime(buf, 64, L"%H:%M:%S", &tmLocal);

        m_lastRefreshText =
            winrt::hstring(L"Last refresh: ") + buf;

        m_statusMessage = L"No errors detected";
        m_hasError = false;


        RaisePropertyChanged(L"NetworkStatus");
        RaisePropertyChanged(L"NetworkStatusBrush");

        RaisePropertyChanged(L"UpnpStatus");
        RaisePropertyChanged(L"UpnpStatusBrush");

        RaisePropertyChanged(L"NatType");
        RaisePropertyChanged(L"NatTypeBrush");

        RaisePropertyChanged(L"SummaryText");

        RaisePropertyChanged(L"LocalIp");
        RaisePropertyChanged(L"WanIp");
        RaisePropertyChanged(L"CgnatStatus");
        RaisePropertyChanged(L"CgnatStatusBrush");

        RaisePropertyChanged(L"NumberOfUPnPDevice");
        RaisePropertyChanged(L"IsPortForwardingAvailable");

        RaisePropertyChanged(L"LastRefreshText");
        RaisePropertyChanged(L"StatusMessage");
        RaisePropertyChanged(L"StatusBrush");

        RaisePropertyChanged(L"UpnpDevices");

        m_refreshing = false;
    }

    winrt::hstring DashboardViewModel::NetworkStatus() { return m_networkStatus; }
    winrt::hstring DashboardViewModel::UpnpStatus() { return m_upnpStatus; }
    winrt::hstring DashboardViewModel::NatType() { return m_natType; }
    winrt::hstring DashboardViewModel::SummaryText() { return m_summary; }
	winrt::hstring DashboardViewModel::NumberOfUPnPDevice() { return m_numberOfUPnPDevice; }
	winrt::hstring DashboardViewModel::IsPortForwardingAvailable() { return m_isPortForwardingAvailable; }
	winrt::hstring DashboardViewModel::LocalIp() { return m_localIp; }
	winrt::hstring DashboardViewModel::WanIp() { return m_wanIp; }
	winrt::hstring DashboardViewModel::CgnatStatus() { return m_cgnatStatus; }
    winrt::hstring DashboardViewModel::LastRefreshText() {return m_lastRefreshText;}
    winrt::hstring DashboardViewModel::StatusMessage(){return m_statusMessage;}

}
