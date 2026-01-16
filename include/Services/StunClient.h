// Created by: EternityBoundary on Jan 16, 2026
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace EtherealScepter::Services::Stun {

// RFC 5780 NAT Behavior Types
enum class NatMappingBehavior {
  Unknown,
  EndpointIndependent,   // Full Cone / Best for P2P
  AddressDependent,      // Restricted Cone
  AddressAndPortDependent // Symmetric / Worst for P2P
};

enum class NatFilteringBehavior {
  Unknown,
  EndpointIndependent,   // Full Cone
  AddressDependent,      // Restricted Cone
  AddressAndPortDependent // Port Restricted / Symmetric
};

// Simplified NAT Type classification (for UI display)
enum class NatType {
  Unknown,
  Open,           // No NAT or Full Cone
  Moderate,       // Restricted Cone or Port Restricted Cone
  Strict,         // Symmetric NAT
  UdpBlocked      // UDP traffic blocked
};

struct StunResult {
  bool success{false};
  std::wstring mappedAddress;
  uint16_t mappedPort{0};
  std::wstring sourceAddress;
  uint16_t sourcePort{0};
  std::wstring changedAddress;
  uint16_t changedPort{0};
};

struct NatAnalysisResult {
  NatType natType{NatType::Unknown};
  NatMappingBehavior mappingBehavior{NatMappingBehavior::Unknown};
  NatFilteringBehavior filteringBehavior{NatFilteringBehavior::Unknown};
  std::wstring externalIp;
  uint16_t externalPort{0};
  std::wstring stunServer;
  std::wstring natTypeDescription;
  std::wstring mappingDescription;
  std::wstring filteringDescription;
};

struct StunServerInfo {
  std::string host;
  uint16_t port;
  uint16_t altPort; // Alternate port for RFC 5780 tests
};

class StunClient {
public:
  StunClient();
  ~StunClient();

  // Perform full RFC 5780 NAT analysis
  NatAnalysisResult AnalyzeNat();

  // Simple binding request to get external IP
  std::optional<StunResult> BindingRequest(const std::string &server,
                                           uint16_t port);

  // Get list of public STUN servers
  static std::vector<StunServerInfo> GetPublicStunServers();

private:
  // STUN message types
  static constexpr uint16_t STUN_BINDING_REQUEST = 0x0001;
  static constexpr uint16_t STUN_BINDING_RESPONSE = 0x0101;
  static constexpr uint16_t STUN_BINDING_ERROR = 0x0111;

  // STUN attribute types
  static constexpr uint16_t ATTR_MAPPED_ADDRESS = 0x0001;
  static constexpr uint16_t ATTR_RESPONSE_ADDRESS = 0x0002;
  static constexpr uint16_t ATTR_CHANGE_REQUEST = 0x0003;
  static constexpr uint16_t ATTR_SOURCE_ADDRESS = 0x0004;
  static constexpr uint16_t ATTR_CHANGED_ADDRESS = 0x0005;
  static constexpr uint16_t ATTR_XOR_MAPPED_ADDRESS = 0x0020;
  static constexpr uint16_t ATTR_OTHER_ADDRESS = 0x802C;

  // STUN magic cookie (RFC 5389)
  static constexpr uint32_t STUN_MAGIC_COOKIE = 0x2112A442;

  // Change request flags
  static constexpr uint32_t CHANGE_IP = 0x04;
  static constexpr uint32_t CHANGE_PORT = 0x02;

  std::vector<uint8_t> BuildBindingRequest(bool changeIp = false,
                                           bool changePort = false);
  std::optional<StunResult>
  ParseBindingResponse(const std::vector<uint8_t> &response,
                       const uint8_t *transactionId);
  std::optional<StunResult> SendStunRequest(const std::string &server,
                                            uint16_t port, bool changeIp = false,
                                            bool changePort = false,
                                            int localPort = 0);

  NatMappingBehavior DetermineMappingBehavior(const StunServerInfo &server1,
                                              const StunServerInfo &server2);
  NatFilteringBehavior DetermineFilteringBehavior(const StunServerInfo &server);

  std::wstring GetNatTypeDescription(NatType type);
  std::wstring GetMappingDescription(NatMappingBehavior behavior);
  std::wstring GetFilteringDescription(NatFilteringBehavior behavior);

  uint8_t m_transactionId[12];
  void GenerateTransactionId();
};

} // namespace EtherealScepter::Services::Stun
