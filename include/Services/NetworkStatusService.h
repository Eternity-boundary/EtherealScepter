//Created by: EternityBoundary on Jan 4, 2025
#pragma once
#include <winrt/base.h>

#include "include/Services/UpnpDiscoveryService.h"
#include "include/Services/UpnpSoapClient.h"
#include "include/Services/IgdDescriptionParser.h"

namespace EtherealScepter::Services
{
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

        Upnp::UpnpDiscoveryService   m_upnpDiscovery;
        Upnp::UpnpSoapClient         m_upnpSoap;
        Upnp::IgdDescriptionParser  m_igdParser;
    };

    struct NetworkStatusService
    {
        static NetworkSnapshot Query();
    };

    
}
