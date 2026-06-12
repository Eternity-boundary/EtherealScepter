// Created by: EternityBoundary on Jun 12, 2026
#pragma once

#include <winrt/base.h>

namespace EtherealScepter::Services {

struct ConnectivityTestResults {
  winrt::hstring pingGateway;
  winrt::hstring pingGoogle;
  winrt::hstring httpReachability;
  winrt::hstring systemDns;
  winrt::hstring ispDns;
  winrt::hstring publicDns;
};

struct NetworkConnectivityTestService {
  static ConnectivityTestResults Run();
};

} // namespace EtherealScepter::Services
