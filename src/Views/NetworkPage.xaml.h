// Created by: EternityBoundary on Jan 3, 2026
#pragma once

#include "NetworkPage.g.h"
#include <NetworkPage.xaml.g.h>
#include <winrt/base.h>
#include <winrt/impl/EtherealScepter.ViewModels.2.h>
#include <winrt/impl/Microsoft.UI.Xaml.2.h>
#include "include/Services/ThemeService.h"

namespace winrt::EtherealScepter::implementation {
struct NetworkPage : NetworkPageT<NetworkPage> {
  NetworkPage();
  ~NetworkPage();

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

  void OnPageLoaded(
      winrt::Windows::Foundation::IInspectable const& sender,
      winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

private:
  void OnThemeChanged(::EtherealScepter::Services::ThemeType theme);
  void ApplyThemeToCards();

  winrt::EtherealScepter::ViewModels::NetworkPageViewModel m_viewModel{nullptr};
  uint32_t m_themeSubscriptionId{ 0 };
};
} // namespace winrt::EtherealScepter::implementation

namespace winrt::EtherealScepter::factory_implementation {
struct NetworkPage : NetworkPageT<NetworkPage, implementation::NetworkPage> {};
} // namespace winrt::EtherealScepter::factory_implementation
