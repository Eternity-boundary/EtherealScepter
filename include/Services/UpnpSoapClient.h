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

        std::optional<std::wstring>
            GetExternalIPAddressViaStatus(const UpnpIgdServiceInfo& igd);

        //extend later:
        // AddPortMapping / DeletePortMapping / GetGenericPortMappingEntry...
	private:
        static std::wstring CombineUrl(
            const std::wstring& baseUrl,
            const std::wstring& controlUrl);

        static std::optional<std::string> WinHttpPostSoap(
            const std::wstring& url,
            const std::wstring& soapAction,
            const std::string& bodyUtf8);

        static std::optional<std::wstring> ExtractXmlValue(
            const std::wstring& xml,
            const std::wstring& tag);
    };
}
