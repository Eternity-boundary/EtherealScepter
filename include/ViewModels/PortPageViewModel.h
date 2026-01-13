// Created by: EternityBoundary on Jan 6, 2026
#pragma once

#include "ViewModels.PortPageViewModel.g.h"
#include "include/Services/UpnpNatPortMappingService.h"

#include <winrt/EtherealScepter.Models.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::EtherealScepter::ViewModels::implementation {
struct PortPageViewModel : PortPageViewModelT<PortPageViewModel> {
  PortPageViewModel();

  Windows::Foundation::Collections::IObservableVector<
      EtherealScepter::Models::PortMappingInfo>
  PortMappings();

  void AddMapping(EtherealScepter::Models::PortMappingInfo const &mapping);
  void RemoveMapping(EtherealScepter::Models::PortMappingInfo const &mapping);

  // INotifyPropertyChanged
  winrt::event_token PropertyChanged(
      Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler);
  void PropertyChanged(winrt::event_token const &token) noexcept;

private:
  winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler>
      m_propertyChanged;
  Windows::Foundation::Collections::IObservableVector<
      EtherealScepter::Models::PortMappingInfo>
      m_portMappings{nullptr};

  winrt::apartment_context m_ui{};
  ::EtherealScepter::Services::UpnpNatPortMappingService m_service{};
};
} // namespace winrt::EtherealScepter::ViewModels::implementation

namespace winrt::EtherealScepter::ViewModels::factory_implementation {
struct PortPageViewModel
    : PortPageViewModelT<PortPageViewModel, implementation::PortPageViewModel> {
};
} // namespace winrt::EtherealScepter::ViewModels::factory_implementation
