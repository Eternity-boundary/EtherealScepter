// Created by: EternityBoundary on Jan 16, 2026
#include "pch.h"

#include "include/Services/StunClient.h"

#include <winsock2.h>
#include <WS2tcpip.h>
#include <algorithm>
#include <array>
#include <random>

#pragma comment(lib, "ws2_32.lib")

namespace EtherealScepter::Services::Stun {

StunClient::StunClient() { GenerateTransactionId(); }

StunClient::~StunClient() = default;

std::vector<StunServerInfo> StunClient::GetPublicStunServers() {
  return {
      // Google STUN servers
      {"stun.l.google.com", 19302, 19305},
      {"stun1.l.google.com", 19302, 19305},
      {"stun2.l.google.com", 19302, 19305},
      // Cloudflare
      {"stun.cloudflare.com", 3478, 3479},
      // Mozilla
      {"stun.services.mozilla.com", 3478, 3479},
      // Twilio
      {"global.stun.twilio.com", 3478, 3479},
  };
}

void StunClient::GenerateTransactionId() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 255);

  for (int i = 0; i < 12; i++) {
    m_transactionId[i] = static_cast<uint8_t>(dist(gen));
  }
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
                                                      bool changePort,
                                                      int localPort) {
  SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == INVALID_SOCKET) {
    return std::nullopt;
  }

  // Set socket timeout
  DWORD timeout = 3000; // 3 seconds
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
             sizeof(timeout));

  // Bind to specific local port if requested
  if (localPort > 0) {
    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(static_cast<u_short>(localPort));
    localAddr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, reinterpret_cast<sockaddr *>(&localAddr), sizeof(localAddr));
  }

  // Resolve server address
  addrinfo hints{};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  addrinfo *result = nullptr;

  if (getaddrinfo(server.c_str(), std::to_string(port).c_str(), &hints,
                  &result) != 0) {
    closesocket(sock);
    return std::nullopt;
  }

  auto request = BuildBindingRequest(changeIp, changePort);
  uint8_t savedTransactionId[12];
  memcpy(savedTransactionId, m_transactionId, 12);

  // Send request
  int sent = sendto(sock, reinterpret_cast<const char *>(request.data()),
                    static_cast<int>(request.size()), 0, result->ai_addr,
                    static_cast<int>(result->ai_addrlen));

  freeaddrinfo(result);

  if (sent == SOCKET_ERROR) {
    closesocket(sock);
    return std::nullopt;
  }

  // Receive response
  std::vector<uint8_t> responseBuffer(1024);
  int received =
      recv(sock, reinterpret_cast<char *>(responseBuffer.data()),
           static_cast<int>(responseBuffer.size()), 0);

  closesocket(sock);

  if (received <= 0) {
    return std::nullopt;
  }

  responseBuffer.resize(received);
  return ParseBindingResponse(responseBuffer, savedTransactionId);
}

std::optional<StunResult> StunClient::BindingRequest(const std::string &server,
                                                     uint16_t port) {
  return SendStunRequest(server, port, false, false, 0);
}

NatMappingBehavior
StunClient::DetermineMappingBehavior(const StunServerInfo &server1,
                                     const StunServerInfo &server2) {
  // Test 1: Send to server1
  auto result1 = SendStunRequest(server1.host, server1.port, false, false, 0);
  if (!result1 || result1->mappedAddress.empty()) {
    return NatMappingBehavior::Unknown;
  }

  auto mappedIp1 = result1->mappedAddress;
  auto mappedPort1 = result1->mappedPort;

  // Test 2: Send to server2 (different IP) from same local port
  // Since we can't easily reuse the same port, we use a heuristic:
  // Send to server1 on alternate port
  auto result2 =
      SendStunRequest(server1.host, server1.altPort, false, false, 0);
  if (!result2 || result2->mappedAddress.empty()) {
    // Try with server2
    result2 = SendStunRequest(server2.host, server2.port, false, false, 0);
    if (!result2 || result2->mappedAddress.empty()) {
      return NatMappingBehavior::Unknown;
    }
  }

  auto mappedIp2 = result2->mappedAddress;
  auto mappedPort2 = result2->mappedPort;

  // Compare results
  if (mappedIp1 == mappedIp2 && mappedPort1 == mappedPort2) {
    // Same mapping regardless of destination = Endpoint Independent
    return NatMappingBehavior::EndpointIndependent;
  } else if (mappedIp1 == mappedIp2) {
    // Same IP but different port = Address Dependent
    return NatMappingBehavior::AddressDependent;
  } else {
    // Different IP and port = Address and Port Dependent (Symmetric)
    return NatMappingBehavior::AddressAndPortDependent;
  }
}

NatFilteringBehavior
StunClient::DetermineFilteringBehavior(const StunServerInfo &server) {
  // Test 1: Normal binding request
  auto result1 = SendStunRequest(server.host, server.port, false, false, 0);
  if (!result1 || result1->mappedAddress.empty()) {
    return NatFilteringBehavior::Unknown;
  }

  // Test 2: Request response from different IP (CHANGE_IP flag)
  auto result2 = SendStunRequest(server.host, server.port, true, false, 0);
  if (result2 && result2->success) {
    // Received response from different IP = Endpoint Independent Filtering
    return NatFilteringBehavior::EndpointIndependent;
  }

  // Test 3: Request response from different port (CHANGE_PORT flag)
  auto result3 = SendStunRequest(server.host, server.port, false, true, 0);
  if (result3 && result3->success) {
    // Received response from different port = Address Dependent Filtering
    return NatFilteringBehavior::AddressDependent;
  }

  // No response from different IP or port = Address and Port Dependent
  return NatFilteringBehavior::AddressAndPortDependent;
}

NatAnalysisResult StunClient::AnalyzeNat() {
  NatAnalysisResult result;

  auto servers = GetPublicStunServers();
  if (servers.empty()) {
    result.natType = NatType::Unknown;
    result.natTypeDescription = L"No STUN servers available";
    return result;
  }

  // Try multiple servers until one works
  std::optional<StunResult> initialResult;
  StunServerInfo workingServer;

  for (const auto &server : servers) {
    initialResult = BindingRequest(server.host, server.port);
    if (initialResult && initialResult->success) {
      workingServer = server;
      result.stunServer =
          std::wstring(server.host.begin(), server.host.end()) + L":" +
          std::to_wstring(server.port);
      break;
    }
  }

  if (!initialResult || !initialResult->success) {
    result.natType = NatType::UdpBlocked;
    result.natTypeDescription = L"UDP traffic appears to be blocked";
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

  // Determine mapping behavior
  StunServerInfo secondServer =
      servers.size() > 1 ? servers[1] : workingServer;
  result.mappingBehavior =
      DetermineMappingBehavior(workingServer, secondServer);
  result.mappingDescription = GetMappingDescription(result.mappingBehavior);

  // Determine filtering behavior
  result.filteringBehavior = DetermineFilteringBehavior(workingServer);
  result.filteringDescription = GetFilteringDescription(result.filteringBehavior);

  // Classify NAT type based on behaviors
  if (result.mappingBehavior == NatMappingBehavior::EndpointIndependent) {
    if (result.filteringBehavior == NatFilteringBehavior::EndpointIndependent) {
      result.natType = NatType::Open; // Full Cone NAT
    } else if (result.filteringBehavior ==
               NatFilteringBehavior::AddressDependent) {
      result.natType = NatType::Moderate; // Restricted Cone NAT
    } else {
      result.natType = NatType::Moderate; // Port Restricted Cone NAT
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
    return L"Unknown";
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
    return L"Unknown";
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
    return L"Unknown";
  }
}

} // namespace EtherealScepter::Services::Stun
