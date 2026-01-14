//Created by: EternityBoundary on Jan 4, 2026
#include "pch.h"
#include "include/Services/UpnpSoapClient.h"

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include <vector>

namespace EtherealScepter::Services::Upnp
{

    std::optional<std::wstring>
        UpnpSoapClient::GetExternalIPAddressViaStatus(
            const UpnpIgdServiceInfo& igd)
    {
        // 組合 control URL
        const std::wstring url =
            CombineUrl(igd.baseUrl, igd.controlUrl);

        // SOAPAction: "<serviceType>#GetStatusInfo"
        const std::wstring soapAction =
            igd.serviceType + L"#GetStatusInfo";

        // SOAP Body
        std::wstring body =
            L"<?xml version=\"1.0\"?>"
            L"<s:Envelope "
            L"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
            L"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
            L"<s:Body>"
            L"<u:GetStatusInfo xmlns:u=\"" + igd.serviceType + L"\"/>"
            L"</s:Body>"
            L"</s:Envelope>";

        // 轉 UTF-8
        std::string bodyUtf8;
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, body.c_str(), (int)body.size(), nullptr, 0, nullptr, nullptr);
        if (size_needed > 0) {
            bodyUtf8.resize(size_needed);
            WideCharToMultiByte(CP_UTF8, 0, body.c_str(), (int)body.size(), bodyUtf8.data(), size_needed, nullptr, nullptr);
        }

        // 發送 SOAP
        auto respBytesOpt =
            WinHttpPostSoap(url, soapAction, bodyUtf8);

        if (!respBytesOpt)
            return std::nullopt;

        // 回應 XML（通常 ASCII / UTF-8）
        const std::string& respBytes = *respBytesOpt;
        
        // 正確處理 UTF-8 轉換
        std::wstring xml;
        int needed = MultiByteToWideChar(CP_UTF8, 0, respBytes.data(), (int)respBytes.size(), nullptr, 0);
        if (needed > 0) {
            xml.resize(needed);
            MultiByteToWideChar(CP_UTF8, 0, respBytes.data(), (int)respBytes.size(), xml.data(), needed);
        }

        // 嘗試解析 <NewExternalIPAddress>
        auto ip =
            ExtractXmlValue(xml, L"NewExternalIPAddress");

        if (!ip || ip->empty())
            return std::nullopt;

        return ip;
    }


    std::wstring EtherealScepter::Services::Upnp::UpnpSoapClient::CombineUrl(const std::wstring& baseUrl, const std::wstring& controlUrl)
    {
        // controlUrl could be:
        //  - "/upnp/control/..." (relative)
        //  - "http://host:port/..." (absolute)
        if (controlUrl.rfind(L"http://", 0) == 0 || controlUrl.rfind(L"https://", 0) == 0)
            return controlUrl;

        if (!controlUrl.empty() && controlUrl.front() == L'/')
            return baseUrl + controlUrl;

        return baseUrl + L"/" + controlUrl;
    }

    std::optional<std::wstring> EtherealScepter::Services::Upnp::UpnpSoapClient::ExtractXmlValue(
        const std::wstring& xml, 
        const std::wstring& tag)
    {
        const std::wstring open = L"<" + tag + L">";
        const std::wstring close = L"</" + tag + L">";

        auto s = xml.find(open);
        if (s == std::wstring::npos) return std::nullopt;
        s += open.size();

        auto e = xml.find(close, s);
        if (e == std::wstring::npos) return std::nullopt;

        return xml.substr(s, e - s);
    }

    // WinHTTP: send SOAP POST and return response body (UTF-8/ASCII XML usually)
     std::optional<std::string> EtherealScepter::Services::Upnp::UpnpSoapClient::WinHttpPostSoap(
         const std::wstring& url, 
         const std::wstring& soapAction, 
         const std::string& bodyUtf8)
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
            return std::nullopt;

        const bool isHttps = (uc.nScheme == INTERNET_SCHEME_HTTPS);
        const std::wstring hostName(uc.lpszHostName, uc.dwHostNameLength);
        const std::wstring urlPath(uc.lpszUrlPath, uc.dwUrlPathLength);

        HINTERNET session = WinHttpOpen(
            L"EtherealScepter/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

        if (!session) return std::nullopt;

        // Optional: timeouts
        WinHttpSetTimeouts(session, 3000, 3000, 3000, 5000);

        HINTERNET connect = WinHttpConnect(session, hostName.c_str(), uc.nPort, 0);
        if (!connect)
        {
            WinHttpCloseHandle(session);
            return std::nullopt;
        }

        HINTERNET request = WinHttpOpenRequest(
            connect,
            L"POST",
            urlPath.c_str(),
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            isHttps ? WINHTTP_FLAG_SECURE : 0);

        if (!request)
        {
            WinHttpCloseHandle(connect);
            WinHttpCloseHandle(session);
            return std::nullopt;
        }

        // Build headers
        std::wstring headers;
        headers += L"Content-Type: text/xml; charset=\"utf-8\"\r\n";
        headers += L"SOAPAction: \"" + soapAction + L"\"\r\n";

        const BOOL sent = WinHttpSendRequest(
            request,
            headers.c_str(),
            (DWORD)headers.size(),
            (LPVOID)bodyUtf8.data(),
            (DWORD)bodyUtf8.size(),
            (DWORD)bodyUtf8.size(),
            0);

        if (!sent || !WinHttpReceiveResponse(request, nullptr))
        {
            WinHttpCloseHandle(request);
            WinHttpCloseHandle(connect);
            WinHttpCloseHandle(session);
            return std::nullopt;
        }

        // Read response bytes
        std::string resp;
        while (true)
        {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(request, &avail) || avail == 0)
                break;

            std::vector<char> buf(avail);
            DWORD read = 0;
            if (!WinHttpReadData(request, buf.data(), avail, &read) || read == 0)
                break;

            resp.append(buf.data(), buf.data() + read);
        }

        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);

        return resp;
    }

    static std::string BuildGetExternalIpSoapBodyUtf8(const std::wstring& serviceType)
    {
        // serviceType: urn:schemas-upnp-org:service:WANIPConnection:1
        // SOAP body must use serviceType as xmlns:u
        std::wstring body =
            L"<?xml version=\"1.0\"?>"
            L"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
            L"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
            L"<s:Body>"
            L"<u:GetExternalIPAddress xmlns:u=\"" + serviceType + L"\"/>"
            L"</s:Body>"
            L"</s:Envelope>";

        // Most routers accept UTF-8.
        std::string result;
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, body.c_str(), (int)body.size(), nullptr, 0, nullptr, nullptr);
        if (size_needed > 0) {
            result.resize(size_needed);
            WideCharToMultiByte(CP_UTF8, 0, body.c_str(), (int)body.size(), result.data(), size_needed, nullptr, nullptr);
        }
        return result;
    }

    // -------- public API --------

    std::optional<std::wstring>
        UpnpSoapClient::GetExternalIPAddress(const UpnpIgdServiceInfo& igd)
    {
        // Compose target URL
        const std::wstring url = CombineUrl(igd.baseUrl, igd.controlUrl);

        // SOAPAction: "<serviceType>#GetExternalIPAddress"
        const std::wstring soapAction = igd.serviceType + L"#GetExternalIPAddress";

        const std::string body = BuildGetExternalIpSoapBodyUtf8(igd.serviceType);

        auto respBytesOpt = WinHttpPostSoap(url, soapAction, body);
        if (!respBytesOpt)
            return std::nullopt;

        // Response is XML. Convert bytes -> wstring (UTF-8).
        const std::string& respBytes = *respBytesOpt;
        std::wstring xml;
        int needed = MultiByteToWideChar(CP_UTF8, 0, respBytes.data(), (int)respBytes.size(), nullptr, 0);
        if (needed > 0) {
            xml.resize(needed);
            MultiByteToWideChar(CP_UTF8, 0, respBytes.data(), (int)respBytes.size(), xml.data(), needed);
        }

        // Parse <NewExternalIPAddress>...</NewExternalIPAddress>
        auto ip = ExtractXmlValue(xml, L"NewExternalIPAddress");
        if (!ip || ip->empty())
            return std::nullopt;

        return ip;
    }
}
