//Created by: EternityBoundary on Jan 4, 2025
#pragma once
#include <string>
#include <optional>

namespace EtherealScepter::Services::Upnp
{
    struct SsdpHeaders
    {
        std::string location;
        std::string server;
        std::string st;
        std::string nt;
        std::string usn;
        std::string nts;
    };

    // 解析一段 SSDP 封包文字（NOTIFY 或 M-SEARCH response）
    SsdpHeaders ParseSsdpHeaders(std::string const& response);

    std::optional<std::string> TryGetLocation(std::string const& response);
}

