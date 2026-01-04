//Created by: EternityBoundary on Jan 4, 2025
#pragma once
#include <string>
#include <optional>

#include "UpnpIgdServiceInfo.h"

namespace EtherealScepter::Services::Upnp
{
    class UpnpSoapClient
    {
    public:
        // Returns public WAN IP, e.g. "203.0.113.45"
        static std::optional<std::wstring>
            GetExternalIPAddress(const UpnpIgdServiceInfo& igd);

        //extend later:
        // AddPortMapping / DeletePortMapping / GetGenericPortMappingEntry...
    };
}
