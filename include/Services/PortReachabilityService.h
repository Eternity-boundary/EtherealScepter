// Created by: EternityBoundary on Jun 12, 2026
#pragma once

#include <cstdint>
#include <winrt/base.h>

namespace EtherealScepter::Services {

struct PortReachabilityService {
  static winrt::hstring TestLocalPort(int32_t port);
  static winrt::hstring TestExternalPort(int32_t port,
                                         winrt::hstring const &externalIp);
};

} // namespace EtherealScepter::Services
