//Created by: EternityBoundary on Jan 8, 2025
#include "pch.h"
#include "include/ViewModels/PortPageViewModel.h"
#include "ViewModels.PortPageViewModel.g.cpp"

#include "include/Services/UpnpNatPortMappingService.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace
{
    ::EtherealScepter::Services::UpnpNatPortMappingService g_upnpService;

    inline void LoadMappingsFromRouter(
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::EtherealScepter::Models::PortMappingInfo> const& vec)
    {
        vec.Clear();

        // 可能丟例外：路由器不支援 UPnP / IGD、UPnP 關閉、無權限、或路由器不回應
        auto list = g_upnpService.Enumerate();
        for (auto const& m : list)
        {
            vec.Append(m);
        }
    }
}

namespace winrt::EtherealScepter::ViewModels::implementation
{
    PortPageViewModel::PortPageViewModel()
    {
        m_portMappings = single_threaded_observable_vector<EtherealScepter::Models::PortMappingInfo>();

        try
        {
            LoadMappingsFromRouter(m_portMappings);
        }
        catch (...)
        {
			//TODO: 顯示錯誤訊息？
        }
    }

    IObservableVector<EtherealScepter::Models::PortMappingInfo> PortPageViewModel::PortMappings()
    {
        return m_portMappings;
    }

    void PortPageViewModel::AddMapping(EtherealScepter::Models::PortMappingInfo const& mapping)
    {
        g_upnpService.Add(mapping);

        m_portMappings.Append(mapping);
    }

    void PortPageViewModel::RemoveMapping(EtherealScepter::Models::PortMappingInfo const& mapping)
    {
        g_upnpService.Remove(mapping);

        //簡單比對
        uint32_t i = 0;
        for (auto const& m : m_portMappings)
        {
            if (m.Description == mapping.Description &&
                m.ExternalPort == mapping.ExternalPort &&
                m.InternalPort == mapping.InternalPort &&
                m.Protocol == mapping.Protocol &&
                m.InternalClient == mapping.InternalClient)
            {
                m_portMappings.RemoveAt(i);
                return;
            }
            ++i;
        }
    }

    winrt::event_token PortPageViewModel::PropertyChanged(
        Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void PortPageViewModel::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }
}
