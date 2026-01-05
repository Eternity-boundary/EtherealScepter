//Created by: EternityBoundary on Jan 4, 2025
#pragma once

#include <string>
#include <optional>
#include "include/Services/UpnpIgdServiceInfo.h"

namespace EtherealScepter::Services::Upnp
{
    class IgdDescriptionParser
    {
    public:
        // location = SSDP LOCATION (igd.xml URL)
        static std::optional<UpnpIgdServiceInfo> ParseFromLocation(std::wstring const& location);
    };

}
