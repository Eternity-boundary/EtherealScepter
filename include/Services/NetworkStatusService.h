//Created by: EternityBoundary on Jan 4, 2025
#pragma once
#pragma once
#include <winrt/base.h>

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
    };

    struct NetworkStatusService
    {
        static NetworkSnapshot Query();
    };
}
