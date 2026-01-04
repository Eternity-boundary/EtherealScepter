//Created by: EternityBoundary on Jan 4, 2025
#include "pch.h"
#include "include/Services/IgdDescriptionParser.h"

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

namespace EtherealScepter::Services::Upnp
{
    static std::wstring HttpGet(const std::wstring& url)
    {
        URL_COMPONENTS uc{};
        uc.dwStructSize = sizeof(uc);

        wchar_t host[256]{};
        wchar_t path[1024]{};

        uc.lpszHostName = host;
        uc.dwHostNameLength = _countof(host);
        uc.lpszUrlPath = path;
        uc.dwUrlPathLength = _countof(path);

        WinHttpCrackUrl(url.c_str(), 0, 0, &uc);

        HINTERNET session = WinHttpOpen(
            L"EtherealScepter/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            nullptr, nullptr, 0);

        if (!session)
            return L"";

        HINTERNET connect = WinHttpConnect(
            session,
            std::wstring(uc.lpszHostName, uc.dwHostNameLength).c_str(),
            uc.nPort, 0);

        if (!connect)
        {
            WinHttpCloseHandle(session);
            return L"";
        }

        HINTERNET request = WinHttpOpenRequest(
            connect,
            L"GET",
            std::wstring(uc.lpszUrlPath, uc.dwUrlPathLength).c_str(),
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            uc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);

        std::wstring result;

        if (WinHttpSendRequest(request,
            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(request, nullptr))
        {
            DWORD size = 0;
            while (WinHttpQueryDataAvailable(request, &size) && size > 0)
            {
                std::wstring buffer(size / sizeof(wchar_t), L'\0');
                DWORD read = 0;
                WinHttpReadData(
                    request,
                    buffer.data(),
                    size,
                    &read);

                result.append(buffer, 0, read / sizeof(wchar_t));
            }
        }

        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);

        return result;
    }

    static std::optional<std::wstring>
        FindTag(const std::wstring& xml, const std::wstring& tag)
    {
        auto start = xml.find(L"<" + tag + L">");
        auto end = xml.find(L"</" + tag + L">");

        if (start == std::wstring::npos || end == std::wstring::npos)
            return std::nullopt;

        start += tag.length() + 2;
        return xml.substr(start, end - start);
    }

    std::optional<UpnpIgdServiceInfo>
        IgdDescriptionParser::ParseFromLocation(const std::wstring& location)
    {
        std::wstring xml = HttpGet(location);
        if (xml.empty())
            return std::nullopt;

        const std::wstring targets[] = {
            L"urn:schemas-upnp-org:service:WANIPConnection:1",
            L"urn:schemas-upnp-org:service:WANPPPConnection:1"
        };

        for (auto& serviceType : targets)
        {
            auto pos = xml.find(serviceType);
            if (pos == std::wstring::npos)
                continue;

            auto control = FindTag(xml.substr(pos), L"controlURL");
            if (!control)
                continue;

            UpnpIgdServiceInfo info;
            info.serviceType = serviceType;
            info.controlUrl = *control;

            // base URL = scheme + host + port
            auto slash = location.find(L'/', location.find(L"//") + 2);
            info.baseUrl = location.substr(0, slash);

            return info;
        }

        return std::nullopt;
    }
}
