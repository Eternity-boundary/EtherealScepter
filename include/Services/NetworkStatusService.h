// Created by: EternityBoundary on Jan 4, 2025
#pragma once
#include <winrt/base.h>

#include "UpnpIgdServiceInfo.h"
#include "include/Services/IgdDescriptionParser.h"
#include "include/Services/UpnpDiscoveryService.h"
#include "include/Services/UpnpSoapClient.h"

#include <optional>
#include <vector>
#include <winrt/impl/EtherealScepter.Models.2.h>

namespace EtherealScepter::Services {
namespace ES = ::EtherealScepter;

struct NetworkSnapshot {
  winrt::hstring networkStatus;
  winrt::hstring upnpStatus;
  winrt::hstring natType;
  winrt::hstring summary;
  winrt::hstring upnpDeviceCount;
  winrt::hstring portForwardingStatus;

  winrt::hstring localIp;
  winrt::hstring wanIp;
  winrt::hstring cgnatStatus;

  winrt::hstring httpWanIp;
  winrt::hstring upnpWanIp;

  bool isStale;

  std::optional<ES::Services::Upnp::UpnpIgdServiceInfo> igdService;

  std::vector<winrt::EtherealScepter::Models::UpnpDeviceInfo> upnpDevices;

  Upnp::UpnpDiscoveryService m_upnpDiscovery;
  Upnp::UpnpSoapClient m_upnpSoap;
  Upnp::IgdDescriptionParser m_igdParser;
};

struct NetworkStatusService {
  static NetworkSnapshot Query();
};

} // namespace EtherealScepter::Services
