#include "pch.h"
#include "include/Services/SsdpParser.h"

#include <algorithm>

namespace EtherealScepter::Services::Upnp
{
    static std::string TrimLeft(std::string s)
    {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
            s.erase(s.begin());
        return s;
    }

    static std::string ExtractHeaderValue(std::string const& response, char const* headerName)
    {
        // SSDP header 可能是 "Location:" / "LOCATION:" / "location:"
        // 這裡做一個簡單 case-insensitive 搜尋（不追求極致效率）
        std::string lower = response;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });

        std::string key = headerName;
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return (char)std::tolower(c); });

        auto pos = lower.find(key);
        if (pos == std::string::npos) return {};

        auto colon = response.find(':', pos);
        if (colon == std::string::npos) return {};

        auto end = response.find("\r\n", colon);
        if (end == std::string::npos) return {};

        std::string value = response.substr(colon + 1, end - colon - 1);
        return TrimLeft(value);
    }

    SsdpHeaders ParseSsdpHeaders(std::string const& response)
    {
        SsdpHeaders h{};
        h.location = ExtractHeaderValue(response, "location");
        h.server = ExtractHeaderValue(response, "server");
        h.st = ExtractHeaderValue(response, "st");
        h.nt = ExtractHeaderValue(response, "nt");
        h.usn = ExtractHeaderValue(response, "usn");
        h.nts = ExtractHeaderValue(response, "nts");
        return h;
    }

    std::optional<std::string> TryGetLocation(std::string const& response)
    {
        auto h = ParseSsdpHeaders(response);
        if (h.location.empty()) return std::nullopt;
        return h.location;
    }
}
