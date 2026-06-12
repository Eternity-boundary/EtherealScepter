// Created by: EternityBoundary on Jan 16, 2026
#include "pch.h"

#include "include/Services/StunClient.h"

#include <winsock2.h>
#include <WS2tcpip.h>
#include <algorithm>
#include <array>
#include <charconv>
#include <memory>
#include <random>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.Http.h>

#pragma comment(lib, "ws2_32.lib")

namespace EtherealScepter::Services::Stun {

namespace {
constexpr wchar_t STUN_SERVER_LIST_URL[] =
    L"https://raw.githubusercontent.com/pradt2/always-online-stun/master/"
    L"valid_hosts.txt";
constexpr size_t MAX_STUN_SERVERS_TO_PROBE = 16;

struct AddrInfoDeleter {
  void operator()(addrinfo *value) const noexcept {
    if (value) {
      freeaddrinfo(value);
    }
  }
};

std::string ToNarrowIp(std::wstring const &value) {
  std::string result;
  result.reserve(value.size());
  for (wchar_t ch : value) {
    if (ch > 0x7F) {
      return {};
    }
    result.push_back(static_cast<char>(ch));
  }
  return result;
}

bool HasMappedEndpoint(StunResult const &result) {
  return !result.mappedAddress.empty() && result.mappedPort != 0;
}

bool HasAlternateEndpoint(StunResult const &result) {
  return !result.changedAddress.empty() && result.changedPort != 0;
}

bool SameMappedEndpoint(StunResult const &lhs, StunResult const &rhs) {
  return lhs.mappedAddress == rhs.mappedAddress &&
         lhs.mappedPort == rhs.mappedPort;
}

bool IsPublicIPv4(std::wstring const &address) {
  in_addr parsed{};
  if (InetPtonW(AF_INET, address.c_str(), &parsed) != 1) {
    return false;
  }

  auto const ip = ntohl(parsed.S_un.S_addr);
  auto const first = (ip >> 24) & 0xFF;
  auto const second = (ip >> 16) & 0xFF;

  if (first == 0 || first == 10 || first == 127 || first >= 224) {
    return false;
  }
  if (first == 100 && second >= 64 && second <= 127) {
    return false;
  }
  if (first == 169 && second == 254) {
    return false;
  }
  if (first == 172 && second >= 16 && second <= 31) {
    return false;
  }
  if (first == 192 && second == 168) {
    return false;
  }

  return true;
}

bool HasUsableAlternateEndpoint(StunResult const &result) {
  return HasAlternateEndpoint(result) && IsPublicIPv4(result.changedAddress);
}

std::optional<StunServerInfo> ParseStunServerToken(std::string_view token) {
  if (token.empty() || token.front() == '#') {
    return std::nullopt;
  }

  std::string_view host;
  std::string_view portText;

  if (token.front() == '[') {
    auto const endBracket = token.find(']');
    if (endBracket == std::string_view::npos ||
        endBracket + 2 > token.size() || token[endBracket + 1] != ':') {
      return std::nullopt;
    }

    host = token.substr(1, endBracket - 1);
    portText = token.substr(endBracket + 2);
  } else {
    auto const colon = token.rfind(':');
    if (colon == std::string_view::npos || colon == 0 ||
        colon + 1 >= token.size()) {
      return std::nullopt;
    }

    host = token.substr(0, colon);
    portText = token.substr(colon + 1);
  }

  unsigned int port{};
  auto const *first = portText.data();
  auto const *last = first + portText.size();
  auto [ptr, ec] = std::from_chars(first, last, port);
  if (ec != std::errc{} || ptr != last || port == 0 || port > 65535) {
    return std::nullopt;
  }

  return StunServerInfo{std::string(host), static_cast<uint16_t>(port)};
}

std::vector<StunServerInfo> ParseStunServerList(std::string const &content) {
  std::vector<StunServerInfo> servers;
  std::unordered_set<std::string> seen;
  std::istringstream lines(content);
  std::string line;

  while (std::getline(lines, line)) {
    auto const comment = line.find('#');
    if (comment != std::string::npos) {
      line.resize(comment);
    }

    std::istringstream tokens(line);
    std::string token;
    while (tokens >> token) {
      auto parsed = ParseStunServerToken(token);
      if (!parsed) {
        continue;
      }

      auto key = parsed->host + ":" + std::to_string(parsed->port);
      if (seen.insert(key).second) {
        servers.push_back(std::move(*parsed));
      }
    }
  }

  return servers;
}

std::optional<std::string> DownloadStunServerList() {
  try {
    winrt::Windows::Web::Http::HttpClient client;
    auto content = client.GetStringAsync(
                             winrt::Windows::Foundation::Uri{
                                 winrt::hstring{STUN_SERVER_LIST_URL}})
                       .get();
    if (content.empty()) {
      return std::nullopt;
    }

    return winrt::to_string(content);
  } catch (...) {
    return std::nullopt;
  }
}
} // namespace

class StunClient::WinsockSession {
public:
  WinsockSession() {
    WSADATA data{};
    initialized_ = WSAStartup(MAKEWORD(2, 2), &data) == 0;
  }

  ~WinsockSession() {
    if (initialized_) {
      WSACleanup();
    }
  }

  WinsockSession(WinsockSession const &) = delete;
  WinsockSession &operator=(WinsockSession const &) = delete;

  bool IsInitialized() const { return initialized_; }

private:
  bool initialized_{false};
};

class StunClient::SocketHandle {
public:
  SocketHandle() = default;
  ~SocketHandle() { Reset(); }

  SocketHandle(SocketHandle const &) = delete;
  SocketHandle &operator=(SocketHandle const &) = delete;

  bool EnsureOpen() {
    if (handle_ != INVALID_SOCKET) {
      return true;
    }

    handle_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (handle_ == INVALID_SOCKET) {
      return false;
    }

    DWORD timeout = 3000;
    if (setsockopt(handle_, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char *>(&timeout),
                   sizeof(timeout)) == SOCKET_ERROR) {
      Reset();
      return false;
    }

    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(0);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(handle_, reinterpret_cast<sockaddr *>(&localAddr),
             sizeof(localAddr)) == SOCKET_ERROR) {
      Reset();
      return false;
    }

    return true;
  }

  SOCKET Get() const { return handle_; }

private:
  void Reset() {
    if (handle_ != INVALID_SOCKET) {
      closesocket(handle_);
      handle_ = INVALID_SOCKET;
    }
  }

  SOCKET handle_{INVALID_SOCKET};
};

StunClient::StunClient()
    : winsock_(std::make_unique<WinsockSession>()),
      socket_(std::make_unique<SocketHandle>()) {
  GenerateTransactionId();
}

StunClient::~StunClient() = default;

std::vector<StunServerInfo> StunClient::GetPublicStunServers() {
  auto content = DownloadStunServerList();
  if (!content) {
    return {};
  }

  return ParseStunServerList(*content);
}

void StunClient::GenerateTransactionId() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 255);

  for (int i = 0; i < 12; i++) {
    m_transactionId[i] = static_cast<uint8_t>(dist(gen));
  }
}

bool StunClient::EnsureSocket() {
  return winsock_ && winsock_->IsInitialized() && socket_ &&
         socket_->EnsureOpen();
}

std::vector<uint8_t> StunClient::BuildBindingRequest(bool changeIp,
                                                     bool changePort) {
  std::vector<uint8_t> message;

  // Message Type: Binding Request (0x0001)
  message.push_back(0x00);
  message.push_back(0x01);

  // Message Length (will be updated)
  uint16_t attrLength = (changeIp || changePort) ? 8 : 0;
  message.push_back(static_cast<uint8_t>(attrLength >> 8));
  message.push_back(static_cast<uint8_t>(attrLength & 0xFF));

  // Magic Cookie (RFC 5389)
  message.push_back(0x21);
  message.push_back(0x12);
  message.push_back(0xA4);
  message.push_back(0x42);

  // Transaction ID (12 bytes)
  GenerateTransactionId();
  for (int i = 0; i < 12; i++) {
    message.push_back(m_transactionId[i]);
  }

  // Add CHANGE-REQUEST attribute if needed (RFC 5780)
  if (changeIp || changePort) {
    // Attribute Type: CHANGE-REQUEST (0x0003)
    message.push_back(0x00);
    message.push_back(0x03);
    // Attribute Length: 4
    message.push_back(0x00);
    message.push_back(0x04);
    // Flags
    uint32_t flags = 0;
    if (changeIp)
      flags |= CHANGE_IP;
    if (changePort)
      flags |= CHANGE_PORT;
    message.push_back(static_cast<uint8_t>((flags >> 24) & 0xFF));
    message.push_back(static_cast<uint8_t>((flags >> 16) & 0xFF));
    message.push_back(static_cast<uint8_t>((flags >> 8) & 0xFF));
    message.push_back(static_cast<uint8_t>(flags & 0xFF));
  }

  return message;
}

std::optional<StunResult>
StunClient::ParseBindingResponse(const std::vector<uint8_t> &response,
                                 const uint8_t *transactionId) {
  if (response.size() < 20) {
    return std::nullopt;
  }

  // Check message type (Binding Response = 0x0101)
  uint16_t msgType = (response[0] << 8) | response[1];
  if (msgType != STUN_BINDING_RESPONSE) {
    return std::nullopt;
  }

  // Verify magic cookie
  uint32_t cookie = (response[4] << 24) | (response[5] << 16) |
                    (response[6] << 8) | response[7];
  if (cookie != STUN_MAGIC_COOKIE) {
    return std::nullopt;
  }

  // Verify transaction ID
  for (int i = 0; i < 12; i++) {
    if (response[8 + i] != transactionId[i]) {
      return std::nullopt;
    }
  }

  StunResult result;
  result.success = true;

  // Parse attributes
  uint16_t msgLen = (response[2] << 8) | response[3];
  size_t pos = 20;

  while (pos + 4 <= 20 + msgLen && pos + 4 <= response.size()) {
    uint16_t attrType = (response[pos] << 8) | response[pos + 1];
    uint16_t attrLen = (response[pos + 2] << 8) | response[pos + 3];
    pos += 4;

    if (pos + attrLen > response.size())
      break;

    auto parseAddress = [&](size_t offset, std::wstring &outAddr,
                            uint16_t &outPort, bool xor_mapped = false) {
      if (offset + 8 > response.size())
        return;

      uint8_t family = response[offset + 1];
      uint16_t port = (response[offset + 2] << 8) | response[offset + 3];

      if (xor_mapped) {
        port ^= (STUN_MAGIC_COOKIE >> 16);
      }
      outPort = port;

      if (family == 0x01) { // IPv4
        uint32_t ip = (response[offset + 4] << 24) |
                      (response[offset + 5] << 16) |
                      (response[offset + 6] << 8) | response[offset + 7];

        if (xor_mapped) {
          ip ^= STUN_MAGIC_COOKIE;
        }

        wchar_t ipStr[32];
        swprintf_s(ipStr, L"%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                   (ip >> 8) & 0xFF, ip & 0xFF);
        outAddr = ipStr;
      }
    };

    switch (attrType) {
    case ATTR_MAPPED_ADDRESS:
      parseAddress(pos, result.mappedAddress, result.mappedPort, false);
      break;
    case ATTR_XOR_MAPPED_ADDRESS:
      parseAddress(pos, result.mappedAddress, result.mappedPort, true);
      break;
    case ATTR_SOURCE_ADDRESS:
      parseAddress(pos, result.sourceAddress, result.sourcePort, false);
      break;
    case ATTR_CHANGED_ADDRESS:
    case ATTR_OTHER_ADDRESS:
      parseAddress(pos, result.changedAddress, result.changedPort, false);
      break;
    }

    // Move to next attribute (with padding to 4-byte boundary)
    pos += attrLen;
    if (attrLen % 4 != 0) {
      pos += 4 - (attrLen % 4);
    }
  }

  return result;
}

std::optional<StunResult> StunClient::SendStunRequest(const std::string &server,
                                                      uint16_t port,
                                                      bool changeIp,
                                                      bool changePort) {
  if (!EnsureSocket()) {
    return std::nullopt;
  }

  // Resolve server address
  addrinfo hints{};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  addrinfo *rawResult = nullptr;
  std::string portString = std::to_string(port);

  if (getaddrinfo(server.c_str(), portString.c_str(), &hints, &rawResult) !=
      0) {
    return std::nullopt;
  }
  std::unique_ptr<addrinfo, AddrInfoDeleter> result(rawResult);

  auto request = BuildBindingRequest(changeIp, changePort);
  uint8_t savedTransactionId[12];
  memcpy(savedTransactionId, m_transactionId, 12);

  // Send request
  int sent = sendto(socket_->Get(), reinterpret_cast<const char *>(request.data()),
                    static_cast<int>(request.size()), 0, result->ai_addr,
                    static_cast<int>(result->ai_addrlen));

  if (sent == SOCKET_ERROR) {
    return std::nullopt;
  }

  // Receive response
  for (int attempt = 0; attempt < 3; ++attempt) {
    std::vector<uint8_t> responseBuffer(1024);
    sockaddr_storage from{};
    int fromLen = sizeof(from);
    int received = recvfrom(socket_->Get(),
                            reinterpret_cast<char *>(responseBuffer.data()),
                            static_cast<int>(responseBuffer.size()), 0,
                            reinterpret_cast<sockaddr *>(&from), &fromLen);

    if (received <= 0) {
      return std::nullopt;
    }

    responseBuffer.resize(received);
    if (auto parsed = ParseBindingResponse(responseBuffer, savedTransactionId)) {
      return parsed;
    }
  }

  return std::nullopt;
}

std::optional<StunResult> StunClient::BindingRequest(const std::string &server,
                                                     uint16_t port) {
  return SendStunRequest(server, port, false, false);
}

NatMappingBehavior
StunClient::DetermineMappingBehavior(const StunServerInfo &server,
                                     const StunResult &initialResult) {
  if (!HasMappedEndpoint(initialResult) ||
      !HasUsableAlternateEndpoint(initialResult)) {
    return NatMappingBehavior::Unknown;
  }

  // RFC 5780 mapping Test II: use only the alternate endpoint advertised by
  // the STUN response, while keeping the same local socket/port.
  auto alternateHost = ToNarrowIp(initialResult.changedAddress);
  if (alternateHost.empty()) {
    return NatMappingBehavior::Unknown;
  }
  auto result2 =
      SendStunRequest(alternateHost, initialResult.changedPort, false, false);
  if (!result2 || !HasMappedEndpoint(*result2)) {
    return NatMappingBehavior::Unknown;
  }

  if (SameMappedEndpoint(initialResult, *result2)) {
    return NatMappingBehavior::EndpointIndependent;
  }

  // RFC 5780 mapping Test III: compare the alternate address on the original
  // server port when the server exposes enough data to distinguish behaviors.
  if (initialResult.changedPort == server.port) {
    return NatMappingBehavior::Unknown;
  }

  auto result3 = SendStunRequest(alternateHost, server.port, false, false);
  if (!result3 || !HasMappedEndpoint(*result3)) {
    return NatMappingBehavior::Unknown;
  }

  if (SameMappedEndpoint(*result2, *result3)) {
    return NatMappingBehavior::AddressDependent;
  }

  return NatMappingBehavior::AddressAndPortDependent;
}

NatFilteringBehavior
StunClient::DetermineFilteringBehavior(const StunServerInfo &server,
                                       const StunResult &initialResult,
                                       bool alternateEndpointVerified) {
  if (!HasMappedEndpoint(initialResult) ||
      !HasUsableAlternateEndpoint(initialResult) ||
      !alternateEndpointVerified) {
    return NatFilteringBehavior::Unknown;
  }

  // RFC 5780 filtering Test II: ask the server to reply from alternate IP and
  // port. A successful response proves endpoint-independent filtering.
  auto result2 = SendStunRequest(server.host, server.port, true, true);
  if (result2 && result2->success) {
    return NatFilteringBehavior::EndpointIndependent;
  }

  // RFC 5780 filtering Test III: a response from the same IP but alternate
  // port proves address-dependent filtering.
  auto result3 = SendStunRequest(server.host, server.port, false, true);
  if (result3 && result3->success) {
    return NatFilteringBehavior::AddressDependent;
  }

  // The advertised alternate endpoint was already verified on a separate
  // socket, so dual CHANGE-REQUEST timeouts can be attributed to filtering.
  return NatFilteringBehavior::AddressAndPortDependent;
}

NatAnalysisResult StunClient::AnalyzeNat() {
  NatAnalysisResult result;

  auto servers = GetPublicStunServers();
  if (servers.empty()) {
    result.natType = NatType::Unknown;
    result.natTypeDescription = L"No STUN servers available from remote list";
    return result;
  }

  // Prefer a server that advertises an RFC 5780 alternate endpoint. Keep the
  // first basic binding result as a fallback so external IP display remains
  // useful even when no full RFC 5780 server is found.
  std::optional<StunResult> initialResult;
  StunServerInfo workingServer{};
  std::optional<StunResult> firstSuccessfulResult;
  StunServerInfo firstSuccessfulServer{};
  bool alternateEndpointVerified = false;

  auto const probeCount = std::min(servers.size(), MAX_STUN_SERVERS_TO_PROBE);
  for (size_t index = 0; index < probeCount; ++index) {
    auto const &server = servers[index];
    auto bindingResult = BindingRequest(server.host, server.port);
    if (bindingResult && bindingResult->success) {
      if (!firstSuccessfulResult) {
        firstSuccessfulResult = bindingResult;
        firstSuccessfulServer = server;
      }

      if (!HasUsableAlternateEndpoint(*bindingResult)) {
        continue;
      }

      auto alternateHost = ToNarrowIp(bindingResult->changedAddress);
      if (alternateHost.empty()) {
        continue;
      }

      StunClient alternateProbeClient;
      auto alternateResult = alternateProbeClient.BindingRequest(
          alternateHost, bindingResult->changedPort);
      if (!alternateResult || !HasMappedEndpoint(*alternateResult)) {
        continue;
      }

      initialResult = bindingResult;
      workingServer = server;
      alternateEndpointVerified = true;
      result.stunServer =
          std::wstring(server.host.begin(), server.host.end()) + L":" +
          std::to_wstring(server.port);
      break;
    }
  }

  if (!initialResult && firstSuccessfulResult) {
    initialResult = firstSuccessfulResult;
    workingServer = firstSuccessfulServer;
    result.stunServer =
        std::wstring(workingServer.host.begin(), workingServer.host.end()) +
        L":" + std::to_wstring(workingServer.port);
  }

  if (!initialResult || !initialResult->success) {
    result.natType = NatType::UdpBlocked;
    result.natTypeDescription =
        L"UDP traffic appears to be blocked or STUN hosts are unreachable";
    return result;
  }

  result.externalIp = initialResult->mappedAddress;
  result.externalPort = initialResult->mappedPort;

  // Check if we're behind NAT at all
  // (This is a simplified check - in production you'd compare with local IP)
  if (result.externalIp.empty()) {
    result.natType = NatType::Unknown;
    result.natTypeDescription = L"Could not determine external IP";
    return result;
  }

  // Determine filtering behavior before mapping tests contact the alternate
  // endpoint on this socket.
  result.filteringBehavior =
      DetermineFilteringBehavior(workingServer, *initialResult,
                                 alternateEndpointVerified);
  result.filteringDescription = GetFilteringDescription(result.filteringBehavior);

  // Determine mapping behavior
  result.mappingBehavior =
      DetermineMappingBehavior(workingServer, *initialResult);
  result.mappingDescription = GetMappingDescription(result.mappingBehavior);

  // Classify NAT type based on behaviors
  if (result.mappingBehavior == NatMappingBehavior::EndpointIndependent) {
    if (result.filteringBehavior == NatFilteringBehavior::EndpointIndependent) {
      result.natType = NatType::Open; // Full Cone NAT
    } else if (result.filteringBehavior ==
               NatFilteringBehavior::AddressDependent) {
      result.natType = NatType::Moderate; // Restricted Cone NAT
    } else if (result.filteringBehavior ==
               NatFilteringBehavior::AddressAndPortDependent) {
      result.natType = NatType::Moderate; // Port Restricted Cone NAT
    } else {
      result.natType = NatType::Unknown;
    }
  } else if (result.mappingBehavior == NatMappingBehavior::AddressDependent) {
    result.natType = NatType::Moderate;
  } else if (result.mappingBehavior ==
             NatMappingBehavior::AddressAndPortDependent) {
    result.natType = NatType::Strict; // Symmetric NAT
  } else {
    result.natType = NatType::Unknown;
  }

  result.natTypeDescription = GetNatTypeDescription(result.natType);

  return result;
}

std::wstring StunClient::GetNatTypeDescription(NatType type) {
  switch (type) {
  case NatType::Open:
    return L"Open (Full Cone) - Best for P2P";
  case NatType::Moderate:
    return L"Moderate (Restricted) - Good for most games";
  case NatType::Strict:
    return L"Strict (Symmetric) - May have connectivity issues";
  case NatType::UdpBlocked:
    return L"UDP Blocked - Cannot connect to P2P services";
  default:
    return L"Unknown - RFC 5780 analysis incomplete";
  }
}

std::wstring StunClient::GetMappingDescription(NatMappingBehavior behavior) {
  switch (behavior) {
  case NatMappingBehavior::EndpointIndependent:
    return L"Endpoint Independent";
  case NatMappingBehavior::AddressDependent:
    return L"Address Dependent";
  case NatMappingBehavior::AddressAndPortDependent:
    return L"Address & Port Dependent";
  default:
    return L"Unknown (insufficient RFC 5780 alternate endpoint data)";
  }
}

std::wstring StunClient::GetFilteringDescription(NatFilteringBehavior behavior) {
  switch (behavior) {
  case NatFilteringBehavior::EndpointIndependent:
    return L"Endpoint Independent";
  case NatFilteringBehavior::AddressDependent:
    return L"Address Dependent";
  case NatFilteringBehavior::AddressAndPortDependent:
    return L"Address & Port Dependent";
  default:
    return L"Unknown (CHANGE-REQUEST test inconclusive)";
  }
}

} // namespace EtherealScepter::Services::Stun
