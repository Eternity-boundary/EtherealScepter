// Created by: EternityBoundary on Jan 3, 2025
#pragma once

#include "NetworkPage.g.h"
#include <NetworkPage.xaml.g.h>
#include <winrt/base.h>
#include <winrt/impl/EtherealScepter.ViewModels.2.h>
#include <winrt/impl/Microsoft.UI.Xaml.2.h>

namespace winrt::EtherealScepter::implementation {
struct NetworkPage : NetworkPageT<NetworkPage> {
  NetworkPage();

  winrt::EtherealScepter::ViewModels::NetworkPageViewModel ViewModel() noexcept;

  void
  OnRunTestsClicked(winrt::Windows::Foundation::IInspectable const &sender,
                    winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

  void
  OnTestPortClicked(winrt::Windows::Foundation::IInspectable const &sender,
                    winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

  void
  OnTestExternalPortClicked(winrt::Windows::Foundation::IInspectable const &sender,
                            winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

private:
  winrt::EtherealScepter::ViewModels::NetworkPageViewModel m_viewModel{nullptr};
};
} // namespace winrt::EtherealScepter::implementation

namespace winrt::EtherealScepter::factory_implementation {
struct NetworkPage : NetworkPageT<NetworkPage, implementation::NetworkPage> {};
} // namespace winrt::Ether
