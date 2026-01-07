//Created by: EternityBoundary on Jan 8, 2025
#include "pch.h"
#include "include/ViewModels/PortPageViewModel.h"
#include "ViewModels.PortPageViewModel.g.cpp"

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::EtherealScepter::ViewModels::implementation
{
    PortPageViewModel::PortPageViewModel()
    {
        m_portMappings = single_threaded_observable_vector<EtherealScepter::Models::PortMappingInfo>();

        // fake data
        m_portMappings.Append({ L"HTTP Server", 80, 8080, L"TCP", L"192.168.1.10" });
        m_portMappings.Append({ L"Game Server", 25565, 25565, L"UDP", L"192.168.1.20" });
    }

    IObservableVector<EtherealScepter::Models::PortMappingInfo> PortPageViewModel::PortMappings()
    {
        return m_portMappings;
    }

    void PortPageViewModel::AddMapping(EtherealScepter::Models::PortMappingInfo const& mapping)
    {
        m_portMappings.Append(mapping);
    }

    void PortPageViewModel::RemoveMapping(EtherealScepter::Models::PortMappingInfo const& mapping)
    {
        // 簡單比對
        //TODO
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

    winrt::event_token PortPageViewModel::PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void PortPageViewModel::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }
}
