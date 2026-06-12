// Created by: EternityBoundary on Jun 12, 2026
//AI注解壞了，所以這是這個編譯單元最後一條注解，祝你好運
#include "pch.h"

#include "include/Services/NetworkConnectivityTestService.h"

#include <iphlpapi.h>
#include <vector>
#include <winrt/Windows.Networking.Connectivity.h>
#include <winrt/Windows.Networking.Sockets.h>
#include <winrt/Windows.Web.Http.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Networking;
using namespace winrt::Windows::Networking::Connectivity;
using namespace winrt::Windows::Networking::Sockets;
using namespace winrt::Windows::Web::Http;

namespace {
struct DnsServerInfo {
  std::wstring address;
  bool isIPv6;
};

std::vector<DnsServerInfo> GetSystemDnsServers() {
  std::vector<DnsServerInfo> dnsServers;

  ULONG bufferSize = 15000;
  std::vector<BYTE> buffer(bufferSize);
  auto addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

  ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST |
                GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_FRIENDLY_NAME;

  DWORD result = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addresses,
                                      &bufferSize);

  if (result == ERROR_BUFFER_OVERFLOW) {
    buffer.resize(bufferSize);
    addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    result = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addresses,
                                  &bufferSize);
  }

  if (result != NO_ERROR) {
    return dnsServers;
  }

  for (auto adapter = addresses; adapter != nullptr; adapter = adapter->Next) {
    if (adapter->OperStatus != IfOperStatusUp) {
      continue;
    }

    for (auto dnsServer = adapter->FirstDnsServerAddress; dnsServer != nullptr;
         dnsServer = dnsServer->Next) {
      wchar_t ipStr[INET6_ADDRSTRLEN] = {};
      bool isIPv6 = false;

      if (dnsServer->Address.lpSockaddr->sa_family == AF_INET) {
        auto addr =
            reinterpret_cast<sockaddr_in *>(dnsServer->Address.lpSockaddr);
        InetNtopW(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN);
      } else if (dnsServer->Address.lpSockaddr->sa_family == AF_INET6) {
        auto addr =
            reinterpret_cast<sockaddr_in6 *>(dnsServer->Address.lpSockaddr);
        InetNtopW(AF_INET6, &addr->sin6_addr, ipStr, INET6_ADDRSTRLEN);
        isIPv6 = true;
      }

      if (wcslen(ipStr) == 0) {
        continue;
      }

      std::wstring ip(ipStr);
      if (ip == L"127.0.0.1" || ip == L"::1" || ip.starts_with(L"fe80")) {
        continue;
      }

      bool found = false;
      for (auto const &existing : dnsServers) {
        if (existing.address == ip) {
          found = true;
          break;
        }
      }

      if (!found) {
        dnsServers.push_back({ip, isIPv6});
      }
    }
  }

  return dnsServers;
}

hstring FormatDnsAddress(DnsServerInfo const &dns) {
  return hstring(dns.address);
}

hstring FormatDnsDisplayAddress(DnsServerInfo const &dns) {
  if (dns.isIPv6) {
    return L"[" + hstring(dns.address) + L"]";
  }
  return hstring(dns.address);
}

hstring TestGatewayReachability() {
  hstring result = L"✖ Gateway unreachable";
  try {
    auto profile = NetworkInformation::GetInternetConnectionProfile();
    if (profile &&
        profile.GetNetworkConnectivityLevel() != NetworkConnectivityLevel::None) {
      result = L"✔ Gateway reachable";
    }
  } catch (...) {
  }
  return result;
}

hstring TestGoogleReachability() {
  hstring result = L"✖ 8.8.8.8 unreachable";
  try {
    StreamSocket socket;
    auto start = std::chrono::steady_clock::now();
    socket.ConnectAsync(HostName(L"8.8.8.8"), L"443").get();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    result = L"✔ 8.8.8.8 (" + to_hstring(elapsed.count()) + L" ms)";
    socket.Close();
  } catch (...) {
  }
  return result;
}

hstring TestHttpReachability() {
  hstring result = L"✖ HTTP Timeout";
  try {
    HttpClient client;
    auto start = std::chrono::steady_clock::now();
    auto response =
        client.GetAsync(Uri(L"http://www.msftconnecttest.com/connecttest.txt"))
            .get();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    if (response.IsSuccessStatusCode()) {
      result = L"✔ HTTP OK (" + to_hstring(elapsed.count()) + L" ms)";
    } else {
      result =
          L"✖ HTTP Error: " + to_hstring(static_cast<int>(response.StatusCode()));
    }
  } catch (...) {
  }
  return result;
}

hstring TestSystemDns() {
  hstring result = L"✖ System DNS failed";
  try {
    auto start = std::chrono::steady_clock::now();
    auto hosts =
        DatagramSocket::GetEndpointPairsAsync(HostName(L"www.microsoft.com"),
                                              L"80")
            .get();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    if (hosts.Size() > 0) {
      result = L"✔ System DNS OK (" + to_hstring(elapsed.count()) + L" ms)";
    }
  } catch (...) {
  }
  return result;
}

hstring TestConfiguredDns(std::vector<DnsServerInfo> const &dnsServers,
                          hstring const &systemDnsResult) {
  if (dnsServers.empty()) {
    return L"⚠ No DNS servers configured";
  }

  auto const &firstDns = dnsServers[0];
  hstring displayAddr = FormatDnsDisplayAddress(firstDns);
  hstring hostAddr = FormatDnsAddress(firstDns);

  bool connected = false;
  int64_t elapsedMs = 0;
  std::vector<hstring> ports = {L"53", L"853", L"443"};

  for (auto const &port : ports) {
    if (connected) {
      break;
    }

    try {
      StreamSocket socket;
      auto start = std::chrono::steady_clock::now();
      socket.ConnectAsync(HostName(hostAddr), port).get();
      elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - start)
                      .count();
      connected = true;
      socket.Close();
    } catch (...) {
    }
  }

  if (connected) {
    return L"✔ DNS " + displayAddr + L" (" + to_hstring(elapsedMs) + L" ms)";
  }
  if (systemDnsResult.starts_with(L"✔")) {
    return L"✔ DNS " + displayAddr + L" (via system resolver)";
  }
  return L"✖ DNS " + displayAddr + L" unreachable";
}

hstring TestPublicDns() {
  hstring result = L"✖ Public DNS unreachable";

  bool ipv4Success = false;
  int64_t ipv4Elapsed = 0;
  try {
    StreamSocket socket;
    auto start = std::chrono::steady_clock::now();
    socket.ConnectAsync(HostName(L"1.1.1.1"), L"443").get();
    ipv4Elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - start)
                      .count();
    ipv4Success = true;
    socket.Close();
  } catch (...) {
  }

  bool ipv6Success = false;
  int64_t ipv6Elapsed = 0;
  try {
    StreamSocket socket;
    auto start = std::chrono::steady_clock::now();
    socket.ConnectAsync(HostName(L"2606:4700:4700::1111"), L"443").get();
    ipv6Elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - start)
                      .count();
    ipv6Success = true;
    socket.Close();
  } catch (...) {
  }

  if (ipv4Success && ipv6Success) {
    result = L"✔ 1.1.1.1 (" + to_hstring(ipv4Elapsed) + L" ms) / IPv6 (" +
             to_hstring(ipv6Elapsed) + L" ms)";
  } else if (ipv4Success) {
    result = L"✔ 1.1.1.1 (" + to_hstring(ipv4Elapsed) + L" ms) / ⚠ IPv6 N/A";
  } else if (ipv6Success) {
    result = L"⚠ IPv4 N/A / ✔ IPv6 (" + to_hstring(ipv6Elapsed) + L" ms)";
  }

  return result;
}
} // namespace

namespace EtherealScepter::Services {

ConnectivityTestResults NetworkConnectivityTestService::Run() {
  ConnectivityTestResults results{};
  auto dnsServers = GetSystemDnsServers();

  results.pingGateway = TestGatewayReachability();
  results.pingGoogle = TestGoogleReachability();
  results.httpReachability = TestHttpReachability();
  results.systemDns = TestSystemDns();
  results.ispDns = TestConfiguredDns(dnsServers, results.systemDns);
  results.publicDns = TestPublicDns();

  return results;
}

} // namespace EtherealScepter::Services
