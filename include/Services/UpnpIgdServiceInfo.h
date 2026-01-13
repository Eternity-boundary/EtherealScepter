//Created by: EternityBoundary on Jan 4, 2026
#pragma once
#include <string>

namespace EtherealScepter::Services::Upnp
{
    struct UpnpIgdServiceInfo
    {
        std::wstring baseUrl;     // e.g. http://192.168.1.1:1900
        std::wstring controlUrl;  // e.g. /upnp/control/WANIPConn1
        std::wstring serviceType; // WANIPConnection / WANPPPConnection
    };
}

