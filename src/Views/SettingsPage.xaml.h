// Created by: EternityBoundary
#pragma once

#include "SettingsPage.g.h"
#include <winrt/EtherealScepter.ViewModels.h>
#include "include/Services/ThemeService.h"

namespace winrt::EtherealScepter::implementation {

struct SettingsPage : SettingsPageT<SettingsPage> {
  SettingsPage();
  ~SettingsPage();

  winrt::EtherealScepter::ViewModels::SettingsPageViewModel ViewModel() noexcept;

  // Helper for formatting percentage
  winrt::hstring FormatPercent(double value);

  // Event handlers
  void OnPageLoaded(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

  void OnPickBackgroundImageClicked(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

  void OnPickNavPaneBackgroundImageClicked(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

  void OnClearBackgroundImageClicked(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

  void OnClearNavPaneBackgroundImageClicked(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

  void OnSaveSettingsClicked(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

private:
  void OnThemeChanged(::EtherealScepter::Services::ThemeType theme);
  void ApplyThemeToCards();

  winrt::EtherealScepter::ViewModels::SettingsPageViewModel m_viewModel{nullptr};
  uint32_t m_themeSubscriptionId{ 0 };
};

} // namespace winrt::EtherealScepter::implementation

namespace winrt::EtherealScepter::factory_implementation {

struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage> {};

} // namespace winrt::EtherealScepter::factory_implementation
