// Created by: EternityBoundary on Jan 8, 2026
#include "pch.h"
#include "include/ViewModels/PortPageViewModel.h"
#include "ViewModels.PortPageViewModel.g.cpp"

#include "include/Services/UpnpNatPortMappingService.h"
#include <cstdint>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/base.h>
#include <winrt/impl/EtherealScepter.Models.2.h>
#include <winrt/impl/Microsoft.UI.Xaml.Data.2.h>
#include <winrt/impl/Windows.Foundation.Collections.1.h>

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace {
::EtherealScepter::Services::UpnpNatPortMappingService g_upnpService;

inline void LoadMappingsFromRouter(
    winrt::Windows::Foundation::Collections::IObservableVector<
        winrt::EtherealScepter::Models::PortMappingInfo> const &vec) {
  vec.Clear();

  // 可能丟例外：路由器不支援 UPnP / IGD、UPnP 關閉、無權限、或路由器不回應
  auto list = g_upnpService.Enumerate();
  for (auto const &m : list) {
    vec.Append(m);
  }
}
} // namespace

namespace winrt::EtherealScepter::ViewModels::implementation {
PortPageViewModel::PortPageViewModel() {
  m_portMappings = single_threaded_observable_vector<
      EtherealScepter::Models::PortMappingInfo>();

  try {
    LoadMappingsFromRouter(m_portMappings);
  } catch (...) {
    // TODO: 顯示錯誤訊息？
  }
}

IObservableVector<EtherealScepter::Models::PortMappingInfo>
PortPageViewModel::PortMappings() {
  return m_portMappings;
}

void PortPageViewModel::AddMapping(
    EtherealScepter::Models::PortMappingInfo const &mapping) {
  try {
    g_upnpService.Add(mapping);
    m_portMappings.Append(mapping);
  } catch (winrt::hresult_error const &ex) {
    // UPnP 裝置不可用或操作失敗時拋出例外
    throw winrt::hresult_error(ex.code(), L"Failed to add port mapping: UPnP device unavailable or operation failed.");
  } catch (...) {
    throw winrt::hresult_error(E_FAIL, L"Failed to add port mapping: Unknown error.");
  }
}

void PortPageViewModel::RemoveMapping(
    EtherealScepter::Models::PortMappingInfo const &mapping) {
  try {
    g_upnpService.Remove(mapping);
  } catch (winrt::hresult_error const &ex) {
    // UPnP 裝置不可用或操作失敗時拋出例外
    throw winrt::hresult_error(ex.code(), L"Failed to remove port mapping: UPnP device unavailable or operation failed.");
  } catch (...) {
    throw winrt::hresult_error(E_FAIL, L"Failed to remove port mapping: Unknown error.");
  }

  // 簡單比對
  uint32_t i = 0;
  for (auto const &m : m_portMappings) {
    if (m.Description == mapping.Description &&
        m.ExternalPort == mapping.ExternalPort &&
        m.InternalPort == mapping.InternalPort &&
        m.Protocol == mapping.Protocol &&
        m.InternalClient == mapping.InternalClient) {
      m_portMappings.RemoveAt(i);
      return;
    }
    ++i;
  }
}

winrt::event_token PortPageViewModel::PropertyChanged(
    Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler) {
  return m_propertyChanged.add(handler);
}

void PortPageViewModel::PropertyChanged(
    winrt::event_token const &token) noexcept {
  m_propertyChanged.remove(token);
}
} // namespace winrt::EtherealScepter::ViewModels::implementation
