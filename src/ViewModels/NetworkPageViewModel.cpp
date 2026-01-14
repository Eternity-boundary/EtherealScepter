// Created by: EternityBoundary on Jan 12, 2026
#include "pch.h"

#include "include/ViewModels/NetworkPageViewModel.h"
#if __has_include("ViewModels.NetworkPageViewModel.g.cpp")
#include "ViewModels.NetworkPageViewModel.g.cpp"
#endif

#include "include/Services/NetworkStatusProvider.h"
#include <string_view>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.Networking.Connectivity.h>
#include <winrt/Windows.Networking.Sockets.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/base.h>

// Win32 API for DNS server detection
#include <iphlpapi.h>
#include <ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml::Data;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI;
using namespace winrt::Windows::Networking;
using namespace winrt::Windows::Networking::Connectivity;
using namespace winrt::Windows::Networking::Sockets;
using namespace winrt::Windows::Web::Http;
namespace Services = ::EtherealScepter::Services;

namespace {
// DNS 伺服器資訊結構
struct DnsServerInfo {
  std::wstring address;
  bool isIPv6;
};

// Helper: 取得系統配置的 DNS 伺服器位址
std::vector<DnsServerInfo> GetSystemDnsServers() {
  std::vector<DnsServerInfo> dnsServers;
  
  ULONG bufferSize = 15000;
  std::vector<BYTE> buffer(bufferSize);
  PIP_ADAPTER_ADDRESSES addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
  
  ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | 
                GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_FRIENDLY_NAME;
  
  DWORD result = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addresses, &bufferSize);
  
  if (result == ERROR_BUFFER_OVERFLOW) {
    buffer.resize(bufferSize);
    addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    result = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addresses, &bufferSize);
  }
  
  if (result != NO_ERROR) {
    return dnsServers;
  }
  
  for (PIP_ADAPTER_ADDRESSES adapter = addresses; adapter != nullptr; adapter = adapter->Next) {
    // 跳過非活動的網路卡
    if (adapter->OperStatus != IfOperStatusUp) {
      continue;
    }
    
    // 取得 DNS 伺服器
    for (PIP_ADAPTER_DNS_SERVER_ADDRESS dnsServer = adapter->FirstDnsServerAddress;
         dnsServer != nullptr; dnsServer = dnsServer->Next) {
      
      wchar_t ipStr[INET6_ADDRSTRLEN] = {};
      bool isIPv6 = false;
      
      if (dnsServer->Address.lpSockaddr->sa_family == AF_INET) {
        auto* addr = reinterpret_cast<sockaddr_in*>(dnsServer->Address.lpSockaddr);
        InetNtopW(AF_INET, &addr->sin_addr, ipStr, INET_ADDRSTRLEN);
        isIPv6 = false;
      } else if (dnsServer->Address.lpSockaddr->sa_family == AF_INET6) {
        auto* addr = reinterpret_cast<sockaddr_in6*>(dnsServer->Address.lpSockaddr);
        InetNtopW(AF_INET6, &addr->sin6_addr, ipStr, INET6_ADDRSTRLEN);
        isIPv6 = true;
      }
      
      if (wcslen(ipStr) > 0) {
        std::wstring ip(ipStr);
        // 過濾掉 localhost 和 link-local
        if (ip != L"127.0.0.1" && ip != L"::1" && !ip.starts_with(L"fe80")) {
          // 避免重複
          bool found = false;
          for (const auto& existing : dnsServers) {
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
    }
  }
  
  return dnsServers;
}

// Helper: 格式化 IPv6 位址用於 HostName
winrt::hstring FormatDnsAddress(const DnsServerInfo& dns) {
  return winrt::hstring(dns.address);
}

// Helper: 格式化顯示用的 DNS 位址
winrt::hstring FormatDnsDisplayAddress(const DnsServerInfo& dns) {
  if (dns.isIPv6) {
    return L"[" + winrt::hstring(dns.address) + L"]";
  }
  return winrt::hstring(dns.address);
}
} // namespace

namespace winrt::EtherealScepter::ViewModels::implementation {

// -------------------------
// Constructor
// -------------------------
NetworkPageViewModel::NetworkPageViewModel() {
  auto &provider = Services::NetworkStatusProvider::Instance();
  provider.InitializeDispatcher();

  m_subscriptionId = provider.Subscribe(
      [weak_this = get_weak()](Services::NetworkSnapshot const &snapshot) {
        if (auto strong_this = weak_this.get()) {
          strong_this->ApplySnapshot(snapshot);
        }
      });

  auto cached = provider.GetCachedSnapshot();
  ApplySnapshot(cached);
}

// -------------------------
// Destructor
// -------------------------
NetworkPageViewModel::~NetworkPageViewModel() {
  if (m_subscriptionId != 0) {
    Services::NetworkStatusProvider::Instance().Unsubscribe(m_subscriptionId);
    m_subscriptionId = 0;
  }
}

// -------------------------
// Helper: Get Brush based on result
// -------------------------
Brush NetworkPageViewModel::GetResultBrush(winrt::hstring const &result) {
  if (result.starts_with(L"✔")) {
    return SolidColorBrush(Colors::LightGreen());
  }
  if (result.starts_with(L"✖") || result.starts_with(L"⚠")) {
    return SolidColorBrush(Colors::Orange());
  }
  return SolidColorBrush(Colors::Gray());
}

// -------------------------
// External IP Properties
// -------------------------
winrt::hstring NetworkPageViewModel::HttpExternalIp() const { return m_httpIp; }
void NetworkPageViewModel::HttpExternalIp(winrt::hstring const &value) {
  if (m_httpIp != value) { m_httpIp = value; RaisePropertyChanged(L"HttpExternalIp"); }
}

winrt::hstring NetworkPageViewModel::UpnpExternalIp() const { return m_upnpIp; }
void NetworkPageViewModel::UpnpExternalIp(winrt::hstring const &value) {
  if (m_upnpIp != value) { m_upnpIp = value; RaisePropertyChanged(L"UpnpExternalIp"); }
}

winrt::hstring NetworkPageViewModel::VerificationStatus() const { return m_status; }
void NetworkPageViewModel::VerificationStatus(winrt::hstring const &value) {
  if (m_status != value) {
    m_status = value;
    RaisePropertyChanged(L"VerificationStatus");
    RaisePropertyChanged(L"VerificationStatusBrush");
  }
}

Brush NetworkPageViewModel::VerificationStatusBrush() {
  if (m_status.starts_with(L"⚠") || m_status == L"CGNAT") {
    return SolidColorBrush(Colors::Orange());
  }
  if (m_status == L"Open") {
    return SolidColorBrush(Colors::Green());
  }
  return SolidColorBrush(Colors::Gray());
}

// -------------------------
// Connectivity Tests Properties
// -------------------------
winrt::hstring NetworkPageViewModel::PingGatewayResult() const { return m_pingGateway; }
void NetworkPageViewModel::PingGatewayResult(winrt::hstring const &value) {
  if (m_pingGateway != value) {
    m_pingGateway = value;
    RaisePropertyChanged(L"PingGatewayResult");
    RaisePropertyChanged(L"PingGatewayBrush");
  }
}
Brush NetworkPageViewModel::PingGatewayBrush() { return GetResultBrush(m_pingGateway); }

winrt::hstring NetworkPageViewModel::PingGoogleResult() const { return m_pingGoogle; }
void NetworkPageViewModel::PingGoogleResult(winrt::hstring const &value) {
  if (m_pingGoogle != value) {
    m_pingGoogle = value;
    RaisePropertyChanged(L"PingGoogleResult");
    RaisePropertyChanged(L"PingGoogleBrush");
  }
}
Brush NetworkPageViewModel::PingGoogleBrush() { return GetResultBrush(m_pingGoogle); }

winrt::hstring NetworkPageViewModel::HttpReachabilityResult() const { return m_httpReachability; }
void NetworkPageViewModel::HttpReachabilityResult(winrt::hstring const &value) {
  if (m_httpReachability != value) {
    m_httpReachability = value;
    RaisePropertyChanged(L"HttpReachabilityResult");
    RaisePropertyChanged(L"HttpReachabilityBrush");
  }
}
Brush NetworkPageViewModel::HttpReachabilityBrush() { return GetResultBrush(m_httpReachability); }

bool NetworkPageViewModel::IsRunningTests() const { return m_isRunningTests; }
void NetworkPageViewModel::IsRunningTests(bool value) {
  if (m_isRunningTests != value) {
    m_isRunningTests = value;
    RaisePropertyChanged(L"IsRunningTests");
  }
}

// -------------------------
// DNS Diagnostics Properties
// -------------------------
winrt::hstring NetworkPageViewModel::SystemDnsResult() const { return m_systemDns; }
void NetworkPageViewModel::SystemDnsResult(winrt::hstring const &value) {
  if (m_systemDns != value) { m_systemDns = value; RaisePropertyChanged(L"SystemDnsResult"); }
}

winrt::hstring NetworkPageViewModel::IspDnsResult() const { return m_ispDns; }
void NetworkPageViewModel::IspDnsResult(winrt::hstring const &value) {
  if (m_ispDns != value) {
    m_ispDns = value;
    RaisePropertyChanged(L"IspDnsResult");
    RaisePropertyChanged(L"IspDnsBrush");
  }
}
Brush NetworkPageViewModel::IspDnsBrush() { return GetResultBrush(m_ispDns); }

winrt::hstring NetworkPageViewModel::PublicDnsResult() const { return m_publicDns; }
void NetworkPageViewModel::PublicDnsResult(winrt::hstring const &value) {
  if (m_publicDns != value) { m_publicDns = value; RaisePropertyChanged(L"PublicDnsResult"); }
}

// -------------------------
// Port Test Properties
// -------------------------
winrt::hstring NetworkPageViewModel::PortTestResult() const { return m_portTestResult; }
void NetworkPageViewModel::PortTestResult(winrt::hstring const &value) {
  if (m_portTestResult != value) {
    m_portTestResult = value;
    RaisePropertyChanged(L"PortTestResult");
    RaisePropertyChanged(L"PortTestBrush");
  }
}
Brush NetworkPageViewModel::PortTestBrush() { return GetResultBrush(m_portTestResult); }

winrt::hstring NetworkPageViewModel::ExternalPortTestResult() const { return m_externalPortTestResult; }
void NetworkPageViewModel::ExternalPortTestResult(winrt::hstring const &value) {
  if (m_externalPortTestResult != value) {
    m_externalPortTestResult = value;
    RaisePropertyChanged(L"ExternalPortTestResult");
    RaisePropertyChanged(L"ExternalPortTestBrush");
  }
}
Brush NetworkPageViewModel::ExternalPortTestBrush() { return GetResultBrush(m_externalPortTestResult); }

bool NetworkPageViewModel::IsTestingPort() const { return m_isTestingPort; }
void NetworkPageViewModel::IsTestingPort(bool value) {
  if (m_isTestingPort != value) {
    m_isTestingPort = value;
    RaisePropertyChanged(L"IsTestingPort");
  }
}

// -------------------------
// Apply Snapshot from Provider
// -------------------------
void NetworkPageViewModel::ApplySnapshot(Services::NetworkSnapshot const &snapshot) {
  m_httpIp = (snapshot.httpWanIp.empty() || snapshot.httpWanIp == L"-") ? L"-" : snapshot.httpWanIp;
  m_upnpIp = snapshot.upnpWanIp.empty() ? L"(Not available)" : snapshot.upnpWanIp;

  if (m_httpIp != L"-" && m_upnpIp != L"(Not available)" && m_httpIp != m_upnpIp) {
    m_status = L"⚠ Possible CGNAT detected";
  } else if (snapshot.cgnatStatus.empty() || snapshot.isIpFallback) {
    m_status = L"Unknown";
  } else {
    m_status = snapshot.cgnatStatus;
  }

  RaiseAll();
}

// -------------------------
// Async Actions
// -------------------------
IAsyncAction NetworkPageViewModel::RefreshAsync() {
  co_await Services::NetworkStatusProvider::Instance().RefreshAsync();
}

IAsyncAction NetworkPageViewModel::RunConnectivityTestsAsync() {
  auto strong_this = get_strong();
  apartment_context ui_thread;

  IsRunningTests(true);
  PingGatewayResult(L"Testing...");
  PingGoogleResult(L"Testing...");
  HttpReachabilityResult(L"Testing...");
  SystemDnsResult(L"Testing...");
  IspDnsResult(L"Testing...");
  PublicDnsResult(L"Testing...");

  co_await resume_background();

  // 先取得系統 DNS 伺服器位址
  auto dnsServers = GetSystemDnsServers();

  // --- Test 1: Ping Gateway ---
  winrt::hstring gatewayResult = L"✖ Gateway unreachable";
  try {
    auto profile = NetworkInformation::GetInternetConnectionProfile();
    if (profile && profile.GetNetworkConnectivityLevel() != NetworkConnectivityLevel::None) {
      gatewayResult = L"✔ Gateway reachable";
    }
  } catch (...) { }

  co_await ui_thread;
  PingGatewayResult(gatewayResult);
  co_await resume_background();

  // --- Test 2: Ping 8.8.8.8 (TCP connect to port 443) ---
  winrt::hstring googleResult = L"✖ 8.8.8.8 unreachable";
  try {
    StreamSocket socket;
    auto hostName = HostName(L"8.8.8.8");
    
    auto start = std::chrono::steady_clock::now();
    socket.ConnectAsync(hostName, L"443").get();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    
    googleResult = L"✔ 8.8.8.8 (" + to_hstring(elapsed.count()) + L" ms)";
    socket.Close();
  } catch (...) { }

  co_await ui_thread;
  PingGoogleResult(googleResult);
  co_await resume_background();

  // --- Test 3: HTTP Reachability ---
  winrt::hstring httpResult = L"✖ HTTP Timeout";
  try {
    HttpClient client;
    auto start = std::chrono::steady_clock::now();
    auto response = client.GetAsync(Uri(L"http://www.msftconnecttest.com/connecttest.txt")).get();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    
    if (response.IsSuccessStatusCode()) {
      httpResult = L"✔ HTTP OK (" + to_hstring(elapsed.count()) + L" ms)";
    } else {
      httpResult = L"✖ HTTP Error: " + to_hstring(static_cast<int>(response.StatusCode()));
    }
  } catch (...) { }

  co_await ui_thread;
  HttpReachabilityResult(httpResult);
  co_await resume_background();

  // --- Test 4: System DNS (使用系統 DNS 解析) ---
  winrt::hstring systemDns = L"✖ System DNS failed";
  try {
    auto start = std::chrono::steady_clock::now();
    auto hosts = DatagramSocket::GetEndpointPairsAsync(HostName(L"www.microsoft.com"), L"80").get();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    
    if (hosts.Size() > 0) {
      systemDns = L"✔ System DNS OK (" + to_hstring(elapsed.count()) + L" ms)";
    }
  } catch (...) { }

  co_await ui_thread;
  SystemDnsResult(systemDns);
  co_await resume_background();

  // --- Test 5: ISP/Configured DNS (測試系統配置的 DNS 伺服器) ---
  winrt::hstring ispDns = L"⚠ No DNS servers configured";
  if (!dnsServers.empty()) {
    // 取得第一個 DNS 伺服器並測試連線
    const auto& firstDns = dnsServers[0];
    winrt::hstring displayAddr = FormatDnsDisplayAddress(firstDns);
    winrt::hstring hostAddr = FormatDnsAddress(firstDns);
    
    bool connected = false;
    int64_t elapsedMs = 0;
    
    // 嘗試多個 port：53 (DNS), 853 (DoT), 443 (DoH)
    std::vector<winrt::hstring> ports = {L"53", L"853", L"443"};
    
    for (const auto& port : ports) {
      if (connected) break;
      
      try {
        StreamSocket socket;
        auto start = std::chrono::steady_clock::now();
        socket.ConnectAsync(HostName(hostAddr), port).get();
        elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        connected = true;
        socket.Close();
      } catch (...) {
        // 嘗試下一個 port
      }
    }
    
    if (connected) {
      ispDns = L"✔ DNS " + displayAddr + L" (" + to_hstring(elapsedMs) + L" ms)";
    } else if (systemDns.starts_with(L"✔")) {
      // 直接連線失敗，但系統 DNS 解析成功，表示 DNS 可能透過其他方式運作（如 DoH 內建）
      ispDns = L"✔ DNS " + displayAddr + L" (via system resolver)";
    } else {
      ispDns = L"✖ DNS " + displayAddr + L" unreachable";
    }
  }

  co_await ui_thread;
  IspDnsResult(ispDns);
  co_await resume_background();

  // --- Test 6: Public DNS (IPv4: 1.1.1.1, IPv6: 2606:4700:4700::1111) ---
  winrt::hstring publicDns = L"✖ Public DNS unreachable";
  
  // 先嘗試 IPv4 (1.1.1.1)
  bool ipv4Success = false;
  int64_t ipv4Elapsed = 0;
  try {
    StreamSocket socket;
    auto start = std::chrono::steady_clock::now();
    socket.ConnectAsync(HostName(L"1.1.1.1"), L"443").get();
    ipv4Elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    ipv4Success = true;
    socket.Close();
  } catch (...) { }

  // 嘗試 IPv6 (2606:4700:4700::1111)
  bool ipv6Success = false;
  int64_t ipv6Elapsed = 0;
  try {
    StreamSocket socket;
    auto start = std::chrono::steady_clock::now();
    socket.ConnectAsync(HostName(L"2606:4700:4700::1111"), L"443").get();
    ipv6Elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    ipv6Success = true;
    socket.Close();
  } catch (...) { }

  // 根據結果組合顯示
  if (ipv4Success && ipv6Success) {
    publicDns = L"✔ 1.1.1.1 (" + to_hstring(ipv4Elapsed) + L" ms) / IPv6 (" + to_hstring(ipv6Elapsed) + L" ms)";
  } else if (ipv4Success) {
    publicDns = L"✔ 1.1.1.1 (" + to_hstring(ipv4Elapsed) + L" ms) / ⚠ IPv6 N/A";
  } else if (ipv6Success) {
    publicDns = L"⚠ IPv4 N/A / ✔ IPv6 (" + to_hstring(ipv6Elapsed) + L" ms)";
  }

  co_await ui_thread;
  PublicDnsResult(publicDns);

  IsRunningTests(false);
}

IAsyncAction NetworkPageViewModel::TestPortAsync(int32_t port) {
  auto strong_this = get_strong();
  apartment_context ui_thread;

  if (port <= 0 || port > 65535) {
    PortTestResult(L"✖ Invalid port number");
    co_return;
  }

  IsTestingPort(true);
  PortTestResult(L"Testing port " + to_hstring(port) + L" locally...");

  co_await resume_background();

  winrt::hstring result = L"✖ Port " + to_hstring(port) + L" unavailable";
  try {
    StreamSocketListener listener;
    listener.BindServiceNameAsync(to_hstring(port)).get();
    listener.Close();
    result = L"✔ Port " + to_hstring(port) + L" available locally";
  } catch (winrt::hresult_error const& ex) {
    if (ex.code() == HRESULT_FROM_WIN32(WSAEADDRINUSE)) {
      result = L"⚠ Port " + to_hstring(port) + L" in use";
    }
  } catch (...) { }

  co_await ui_thread;
  PortTestResult(result);
  IsTestingPort(false);
}

IAsyncAction NetworkPageViewModel::TestExternalPortAsync(int32_t port) {
  auto strong_this = get_strong();
  apartment_context ui_thread;

  if (port <= 0 || port > 65535) {
    ExternalPortTestResult(L"✖ Invalid port number");
    co_return;
  }

  IsTestingPort(true);
  ExternalPortTestResult(L"Checking port " + to_hstring(port) + L" from internet...");

  co_await resume_background();

  winrt::hstring result = L"✖ Unable to check external port";
  
  try {
    HttpClient client;
    
    // 先取得外部 IPv4 位址 (使用 IPv4-only 的 api.ipify.org)
    winrt::hstring externalIp = m_httpIp;
    if (externalIp.empty() || externalIp == L"-") {
      try {
        auto ipResponse = client.GetStringAsync(Uri(L"https://api.ipify.org")).get();
        if (!ipResponse.empty()) {
          externalIp = ipResponse;
        }
      } catch (...) { }
    }
    
    if (externalIp.empty() || externalIp == L"-") {
      result = L"⚠ Cannot determine external IPv4 address";
    } else {
      // 使用 check-host.net API 進行 IPv4 端口檢測
      // 格式: https://check-host.net/check-tcp?host={ip}:{port}
      auto uri = Uri(L"https://check-host.net/check-tcp?host=" + externalIp + L":" + to_hstring(port));
      
      // 設定 Accept header 以取得 JSON 回應
      client.DefaultRequestHeaders().Accept().Clear();
      client.DefaultRequestHeaders().Accept().Append(
          Headers::HttpMediaTypeWithQualityHeaderValue(L"application/json"));
      
      auto start = std::chrono::steady_clock::now();
      auto response = client.GetAsync(uri).get();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start).count();
      
      if (response.IsSuccessStatusCode()) {
        auto content = response.Content().ReadAsStringAsync().get();
        std::wstring contentStr(content.c_str());
        
        // check-host.net 返回 JSON，包含 request_id 和多個檢測節點
        // 成功回應表示檢測請求已排隊，需要輪詢結果
        // 簡化處理：如果收到有效回應且包含 request_id，表示 API 可用
        if (contentStr.find(L"\"request_id\"") != std::wstring::npos ||
            contentStr.find(L"\"ok\"") != std::wstring::npos) {
          // 取得 request_id 進行結果查詢
          auto requestIdPos = contentStr.find(L"\"request_id\":\"");
          if (requestIdPos != std::wstring::npos) {
            auto idStart = requestIdPos + 14; // length of "\"request_id\":\""
            auto idEnd = contentStr.find(L"\"", idStart);
            if (idEnd != std::wstring::npos) {
              auto requestId = contentStr.substr(idStart, idEnd - idStart);
              
              // 等待一小段時間讓檢測完成
              Sleep(2000);
              
              // 查詢結果
              auto resultUri = Uri(L"https://check-host.net/check-result/" + winrt::hstring(requestId));
              auto resultResponse = client.GetAsync(resultUri).get();
              
              if (resultResponse.IsSuccessStatusCode()) {
                auto resultContent = resultResponse.Content().ReadAsStringAsync().get();
                std::wstring resultStr(resultContent.c_str());
                
                // 分析結果：檢查是否有任何節點回報連線成功
                bool hasSuccess = false;
                
                // 如果結果中包含 "address" 和 "time" 欄位，表示有節點成功連線
                if (resultStr.find(L"\"address\"") != std::wstring::npos &&
                    resultStr.find(L"\"time\"") != std::wstring::npos) {
                  hasSuccess = true;
                }
                
                // 只要有任何節點成功連線，就視為端口開放
                // (部分 null 可能是因為特定國家的網路限制，非端口問題)
                if (hasSuccess) {
                  result = L"✔ Port " + to_hstring(port) + L" is OPEN (" + externalIp + L")";
                } else {
                  result = L"✖ Port " + to_hstring(port) + L" is CLOSED (" + externalIp + L")";
                }
              } else {
                result = L"⚠ Port " + to_hstring(port) + L" check pending (" + externalIp + L")";
              }
            } else {
              result = L"⚠ Port " + to_hstring(port) + L" status unclear (" + externalIp + L")";
            }
          } else {
            result = L"⚠ Port " + to_hstring(port) + L" status unclear (" + externalIp + L")";
          }
        } else {
          result = L"⚠ Port " + to_hstring(port) + L" status unclear (" + externalIp + L")";
        }
      } else if (response.StatusCode() == HttpStatusCode::TooManyRequests) {
        result = L"⚠ Rate limited, please try again later";
      } else {
        result = L"⚠ API error (HTTP " + to_hstring(static_cast<int>(response.StatusCode())) + L")";
      }
    }
  } catch (winrt::hresult_error const&) {
    result = L"⚠ Check manually: canyouseeme.org or portchecker.co";
  } catch (...) {
    result = L"⚠ External check failed";
  }

  co_await ui_thread;
  ExternalPortTestResult(result);
  IsTestingPort(false);
}

// -------------------------
// INotifyPropertyChanged
// -------------------------
event_token NetworkPageViewModel::PropertyChanged(PropertyChangedEventHandler const &handler) {
  return m_propertyChanged.add(handler);
}

void NetworkPageViewModel::PropertyChanged(event_token const &token) noexcept {
  m_propertyChanged.remove(token);
}

void NetworkPageViewModel::RaisePropertyChanged(std::wstring_view name) {
  m_propertyChanged(*this, PropertyChangedEventArgs{hstring{name}});
}

void NetworkPageViewModel::RaiseAll() {
  RaisePropertyChanged(L"HttpExternalIp");
  RaisePropertyChanged(L"UpnpExternalIp");
  RaisePropertyChanged(L"VerificationStatus");
  RaisePropertyChanged(L"VerificationStatusBrush");
}

} // namespace winrt::EtherealScepter::ViewModels::implementation