//Created by: EternityBoundary on Jan 4, 2025
#pragma once

#include <string>
#include <vector>

namespace EtherealScepter::Services::Upnp
{
    struct UpnpDeviceLocation
    {
        std::wstring location;   // e.g. http://10.0.0.1:1900/igd.xml
        std::wstring server;     
        std::wstring st;         
    };

    class UpnpDiscoveryService
    {
    public:
        // Blocking call
        // Must be called from background thread
        static std::vector<UpnpDeviceLocation> Discover();
    };
}

