// Created by: EternityBoundary on Jun 12, 2026
#include "pch.h"

#include "include/Services/PortReachabilityService.h"

#include <string>
#include <winrt/Windows.Networking.Sockets.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Web.Http.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Networking;
using namespace winrt::Windows::Networking::Sockets;
using namespace winrt::Windows::Web::Http;

namespace {
hstring QueryExternalIp(HttpClient const &client) {
  try {
    auto ipResponse = client.GetStringAsync(Uri(L"https://api.ipify.org")).get();
    if (!ipResponse.empty()) {
      return ipResponse;
    }
  } catch (...) {
  }
  return L"-";
}

hstring TryReadCheckHostResult(HttpClient const &client,
                               std::wstring const &requestId, int32_t port,
                               hstring const &externalIp) {
  Sleep(2000);

  auto resultUri =
      Uri(L"https://check-host.net/check-result/" + hstring(requestId));
  auto resultResponse = client.GetAsync(resultUri).get();

  if (!resultResponse.IsSuccessStatusCode()) {
    return L"⚠ Port " + to_hstring(port) + L" check pending (" + externalIp +
           L")";
  }

  auto resultContent = resultResponse.Content().ReadAsStringAsync().get();
  std::wstring resultStr(resultContent.c_str());

  bool hasSuccess = resultStr.find(L"\"address\"") != std::wstring::npos &&
                    resultStr.find(L"\"time\"") != std::wstring::npos;

  if (hasSuccess) {
    return L"✔ Port " + to_hstring(port) + L" is OPEN (" + externalIp + L")";
  }
  return L"✖ Port " + to_hstring(port) + L" is CLOSED (" + externalIp + L")";
}

hstring ParseCheckHostResponse(HttpClient const &client,
                               std::wstring const &contentStr, int32_t port,
                               hstring const &externalIp) {
  if (contentStr.find(L"\"request_id\"") == std::wstring::npos &&
      contentStr.find(L"\"ok\"") == std::wstring::npos) {
    return L"⚠ Port " + to_hstring(port) + L" status unclear (" + externalIp +
           L")";
  }

  auto requestIdPos = contentStr.find(L"\"request_id\":\"");
  if (requestIdPos == std::wstring::npos) {
    return L"⚠ Port " + to_hstring(port) + L" status unclear (" + externalIp +
           L")";
  }

  auto idStart = requestIdPos + 14;
  auto idEnd = contentStr.find(L"\"", idStart);
  if (idEnd == std::wstring::npos) {
    return L"⚠ Port " + to_hstring(port) + L" status unclear (" + externalIp +
           L")";
  }

  auto requestId = contentStr.substr(idStart, idEnd - idStart);
  return TryReadCheckHostResult(client, requestId, port, externalIp);
}
} // namespace

namespace EtherealScepter::Services {

hstring PortReachabilityService::TestLocalPort(int32_t port) {
  hstring result = L"✖ Port " + to_hstring(port) + L" unavailable";
  try {
    StreamSocketListener listener;
    listener.BindServiceNameAsync(to_hstring(port)).get();
    listener.Close();
    result = L"✔ Port " + to_hstring(port) + L" available locally";
  } catch (hresult_error const &ex) {
    if (ex.code() == HRESULT_FROM_WIN32(WSAEADDRINUSE)) {
      result = L"⚠ Port " + to_hstring(port) + L" in use";
    }
  } catch (...) {
  }

  return result;
}

hstring PortReachabilityService::TestExternalPort(int32_t port,
                                                  hstring const &externalIp) {
  hstring result = L"✖ Unable to check external port";

  try {
    HttpClient client;
    hstring detectedExternalIp = externalIp;
    if (detectedExternalIp.empty() || detectedExternalIp == L"-") {
      detectedExternalIp = QueryExternalIp(client);
    }

    if (detectedExternalIp.empty() || detectedExternalIp == L"-") {
      return L"⚠ Cannot determine external IPv4 address";
    }

    auto uri = Uri(L"https://check-host.net/check-tcp?host=" +
                   detectedExternalIp + L":" + to_hstring(port));

    client.DefaultRequestHeaders().Accept().Clear();
    client.DefaultRequestHeaders().Accept().Append(
        Headers::HttpMediaTypeWithQualityHeaderValue(L"application/json"));

    auto response = client.GetAsync(uri).get();

    if (response.IsSuccessStatusCode()) {
      auto content = response.Content().ReadAsStringAsync().get();
      std::wstring contentStr(content.c_str());
      return ParseCheckHostResponse(client, contentStr, port, detectedExternalIp);
    }

    if (response.StatusCode() == HttpStatusCode::TooManyRequests) {
      result = L"⚠ Rate limited, please try again later";
    } else {
      result =
          L"⚠ API error (HTTP " +
          to_hstring(static_cast<int>(response.StatusCode())) + L")";
    }
  } catch (hresult_error const &) {
    result = L"⚠ Check manually: canyouseeme.org or portchecker.co";
  } catch (...) {
    result = L"⚠ External check failed";
  }

  return result;
}

} // namespace EtherealScepter::Services
