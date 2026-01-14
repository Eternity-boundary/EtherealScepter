//Created by: EternityBoundary on Jan 4, 2026
#include "pch.h"
#include "include/Services/IgdDescriptionParser.h"

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include <string>
#include <vector>
#include <optional>

using namespace winrt;
using namespace Windows::Data::Xml::Dom;

namespace EtherealScepter::Services::Upnp
{
    // ---- helpers ------------------------------------------------------------

    static std::wstring BytesToWideUtf8OrAcp(const std::string& bytes)
    {
        if (bytes.empty())
            return L"";

        // Try UTF-8 first (most IGD XML is UTF-8)
        int needed = MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            bytes.data(),
            (int)bytes.size(),
            nullptr,
            0);

        if (needed > 0)
        {
            std::wstring w;
            w.resize(needed);
            MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                bytes.data(),
                (int)bytes.size(),
                w.data(),
                needed);
            return w;
        }

        // Fallback to system ANSI codepage (rare)
        needed = MultiByteToWideChar(
            CP_ACP,
            0,
            bytes.data(),
            (int)bytes.size(),
            nullptr,
            0);

        if (needed <= 0)
            return L"";

        std::wstring w;
        w.resize(needed);
        MultiByteToWideChar(
            CP_ACP,
            0,
            bytes.data(),
            (int)bytes.size(),
            w.data(),
            needed);
        return w;
    }

    static void SanitizeXmlInPlace(std::wstring& xml)
    {
        if (xml.empty())
            return;

        // Remove UTF-16 BOM if present
        if (!xml.empty() && xml.front() == 0xFEFF)
            xml.erase(xml.begin());

        // Remove any leading garbage before first '<' (whitespace, stray chars, etc.)
        auto pos = xml.find(L'<');
        if (pos != std::wstring::npos && pos > 0)
            xml.erase(0, pos);

        // Remove embedded NULs that can appear if bytes were mishandled somewhere upstream
        xml.erase(std::remove(xml.begin(), xml.end(), L'\0'), xml.end());
    }

    static std::wstring HttpGet(const std::wstring& url)
    {
        URL_COMPONENTS uc{};
        uc.dwStructSize = sizeof(uc);

        wchar_t host[256]{};
        wchar_t path[2048]{};

        uc.lpszHostName = host;
        uc.dwHostNameLength = _countof(host);
        uc.lpszUrlPath = path;
        uc.dwUrlPathLength = _countof(path);

        if (!WinHttpCrackUrl(url.c_str(), 0, 0, &uc))
            return L"";

        const bool isHttps = (uc.nScheme == INTERNET_SCHEME_HTTPS);
        std::wstring hostName(uc.lpszHostName, uc.dwHostNameLength);
        std::wstring urlPath(uc.lpszUrlPath, uc.dwUrlPathLength);

        HINTERNET session = WinHttpOpen(
            L"EtherealScepter/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

        if (!session)
            return L"";

        // Optional: timeouts (ms)
        WinHttpSetTimeouts(session, 3000, 3000, 3000, 5000);

        HINTERNET connect = WinHttpConnect(session, hostName.c_str(), uc.nPort, 0);
        if (!connect)
        {
            WinHttpCloseHandle(session);
            return L"";
        }

        HINTERNET request = WinHttpOpenRequest(
            connect,
            L"GET",
            urlPath.c_str(),
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            isHttps ? WINHTTP_FLAG_SECURE : 0);

        if (!request)
        {
            WinHttpCloseHandle(connect);
            WinHttpCloseHandle(session);
            return L"";
        }

        std::string bytes;

        if (WinHttpSendRequest(
            request,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            WINHTTP_NO_REQUEST_DATA,
            0,
            0,
            0) &&
            WinHttpReceiveResponse(request, nullptr))
        {
            while (true)
            {
                DWORD avail = 0;
                if (!WinHttpQueryDataAvailable(request, &avail) || avail == 0)
                    break;

                std::vector<char> buf(avail);
                DWORD read = 0;
                if (!WinHttpReadData(request, buf.data(), avail, &read) || read == 0)
                    break;

                bytes.append(buf.data(), buf.data() + read);
            }
        }

        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);

        // Handle UTF-8 BOM at byte level (EF BB BF)
        if (bytes.size() >= 3 &&
            (unsigned char)bytes[0] == 0xEF &&
            (unsigned char)bytes[1] == 0xBB &&
            (unsigned char)bytes[2] == 0xBF)
        {
            bytes.erase(0, 3);
        }

        std::wstring result = BytesToWideUtf8OrAcp(bytes);
        SanitizeXmlInPlace(result);
        return result;
    }

    // (FindTag can stay; not used in current DOM approach, but keep harmless)
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
        IgdDescriptionParser::ParseFromLocation(std::wstring const& location)
    {
        // 1. 下載 XML
        std::wstring xml = HttpGet(location);
        if (xml.empty())
            return std::nullopt;

        try
        {
            // 2. 載入 XML
            XmlDocument doc;
            doc.LoadXml(winrt::hstring{ xml });

            // 3. 找所有 <service>（忽略 namespace）
            auto services = doc.SelectNodes(L"//*[local-name()='service']");

            static const std::wstring targets[] = {
                L"urn:schemas-upnp-org:service:WANIPConnection:1",
                L"urn:schemas-upnp-org:service:WANPPPConnection:1",
            };

            for (uint32_t i = 0; i < services.Length(); ++i)
            {
                auto service = services.Item(i);

                auto typeNode =
                    service.SelectSingleNode(L"./*[local-name()='serviceType']");
                auto controlNode =
                    service.SelectSingleNode(L"./*[local-name()='controlURL']");

                if (!typeNode || !controlNode)
                    continue;

                std::wstring serviceType = typeNode.InnerText().c_str();
                std::wstring controlUrl = controlNode.InnerText().c_str();

                for (auto const& target : targets)
                {
                    if (serviceType == target)
                    {
                        UpnpIgdServiceInfo info{};
                        info.serviceType = serviceType;
                        info.controlUrl = controlUrl;

                        // baseUrl = scheme + host + port
                        auto slash =
                            location.find(L'/', location.find(L"//") + 2);
                        info.baseUrl =
                            (slash == std::wstring::npos)
                            ? location
                            : location.substr(0, slash);

                        // Parse device info from root device element
                        if (auto deviceNode = doc.SelectSingleNode(L"//*[local-name()='device']"))
                        {
                            if (auto node = deviceNode.SelectSingleNode(L"./*[local-name()='friendlyName']"))
                                info.friendlyName = node.InnerText().c_str();
                            if (auto node = deviceNode.SelectSingleNode(L"./*[local-name()='manufacturer']"))
                                info.manufacturer = node.InnerText().c_str();
                            if (auto node = deviceNode.SelectSingleNode(L"./*[local-name()='modelName']"))
                                info.modelName = node.InnerText().c_str();
                            if (auto node = deviceNode.SelectSingleNode(L"./*[local-name()='presentationURL']"))
                                info.presentationUrl = node.InnerText().c_str();
                        }

                        return info;
                    }
                }
            }
        }
        catch (winrt::hresult_error const& e)
        {
            OutputDebugStringW(L"[IgdDescriptionParser] XML parse failed: ");
            OutputDebugStringW(e.message().c_str());
            OutputDebugStringW(L"\n");
            return std::nullopt;
        }
        catch (...)
        {
            OutputDebugStringW(L"[IGD Parser] unknown exception\n");
            return std::nullopt;
        }

        return std::nullopt;
    }

}
