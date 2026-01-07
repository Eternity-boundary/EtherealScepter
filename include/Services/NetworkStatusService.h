//Created by: EternityBoundary on Jan 4, 2025
#pragma once
#include <winrt/base.h>

#include "include/Services/UpnpDiscoveryService.h"
#include "include/Services/UpnpSoapClient.h"
#include "include/Services/IgdDescriptionParser.h"

#include "winrt/EtherealScepter.Models.h"

namespace EtherealScepter::Services
{
    namespace ES = ::EtherealScepter;

    struct NetworkSnapshot
    {
        winrt::hstring networkStatus;
        winrt::hstring upnpStatus;
        winrt::hstring natType;
        winrt::hstring summary;
        winrt::hstring upnpDeviceCount;
        winrt::hstring portForwardingStatus;

        winrt::hstring localIp;          
        winrt::hstring wanIp;            
        winrt::hstring cgnatStatus;   

        std::optional<ES::Services::Upnp::UpnpIgdServiceInfo> igdService;

        std::vector<winrt::EtherealScepter::Models::UpnpDeviceInfo> upnpDevices;

        Upnp::UpnpDiscoveryService   m_upnpDiscovery;
        Upnp::UpnpSoapClient         m_upnpSoap;
        Upnp::IgdDescriptionParser  m_igdParser;
    };

    struct NetworkStatusService
    {
        static NetworkSnapshot Query();
    };

    
}
