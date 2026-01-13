//Created by: EternityBoundary on Jan 8, 2026
#pragma once
#include <vector>
#include <winrt/base.h>
#include <winrt/EtherealScepter.Models.h>

namespace EtherealScepter::Services
{
    struct UpnpNatPortMappingService
    {
        std::vector<winrt::EtherealScepter::Models::PortMappingInfo> Enumerate();
        void Add(winrt::EtherealScepter::Models::PortMappingInfo const& m);
        void Remove(winrt::EtherealScepter::Models::PortMappingInfo const& m);
    };
}
