// Created by: EternityBoundary
#pragma once

#include "ViewModels.SettingsPageViewModel.g.h"
#include "include/Services/ThemeService.h"

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.Foundation.h>

namespace winrt::EtherealScepter::ViewModels::implementation {

struct SettingsPageViewModel : SettingsPageViewModelT<SettingsPageViewModel> {
  SettingsPageViewModel();
  ~SettingsPageViewModel();

  // Theme selection
  int32_t SelectedThemeIndex() const;
  void SelectedThemeIndex(int32_t value);

  // Current custom slot being edited
  int32_t CurrentCustomSlot() const;
  void CurrentCustomSlot(int32_t value);

  // Display text for current slot
  winrt::hstring CurrentSlotDisplayText() const;

  // Default theme names (read-only)
  winrt::hstring DefaultTheme1Name() const;
  winrt::hstring DefaultTheme2Name() const;

  // Custom theme name
  winrt::hstring CustomThemeName() const;
  void CustomThemeName(winrt::hstring const &value);

  // Background images
  winrt::hstring BackgroundImagePath() const;
  void BackgroundImagePath(winrt::hstring const &value);

  winrt::hstring NavPaneBackgroundImagePath() const;
  void NavPaneBackgroundImagePath(winrt::hstring const &value);

  // Legacy opacity settings
  double PanelOpacity() const;
  void PanelOpacity(double value);

  double NavPaneOpacity() const;
  void NavPaneOpacity(double value);

  int32_t BaseThemeIndex() const;
  void BaseThemeIndex(int32_t value);

  // Navigation pane background color
  uint8_t NavPaneBgR() const;
  void NavPaneBgR(uint8_t value);
  uint8_t NavPaneBgG() const;
  void NavPaneBgG(uint8_t value);
  uint8_t NavPaneBgB() const;
  void NavPaneBgB(uint8_t value);
  double NavPaneBgOpacity() const;
  void NavPaneBgOpacity(double value);

  // Navigation button normal color
  uint8_t NavBtnNormalR() const;
  void NavBtnNormalR(uint8_t value);
  uint8_t NavBtnNormalG() const;
  void NavBtnNormalG(uint8_t value);
  uint8_t NavBtnNormalB() const;
  void NavBtnNormalB(uint8_t value);
  double NavBtnNormalOpacity() const;
  void NavBtnNormalOpacity(double value);

  // Navigation button hover color
  uint8_t NavBtnHoverR() const;
  void NavBtnHoverR(uint8_t value);
  uint8_t NavBtnHoverG() const;
  void NavBtnHoverG(uint8_t value);
  uint8_t NavBtnHoverB() const;
  void NavBtnHoverB(uint8_t value);
  double NavBtnHoverOpacity() const;
  void NavBtnHoverOpacity(double value);

  // Navigation button selected color
  uint8_t NavBtnSelectedR() const;
  void NavBtnSelectedR(uint8_t value);
  uint8_t NavBtnSelectedG() const;
  void NavBtnSelectedG(uint8_t value);
  uint8_t NavBtnSelectedB() const;
  void NavBtnSelectedB(uint8_t value);
  double NavBtnSelectedOpacity() const;
  void NavBtnSelectedOpacity(double value);

  // Card/panel background color
  uint8_t CardBgR() const;
  void CardBgR(uint8_t value);
  uint8_t CardBgG() const;
  void CardBgG(uint8_t value);
  uint8_t CardBgB() const;
  void CardBgB(uint8_t value);
  double CardBgOpacity() const;
  void CardBgOpacity(double value);

  // UI state
  bool IsCustomThemeEnabled() const;
  Microsoft::UI::Xaml::Visibility CustomThemeVisibility() const;
  Microsoft::UI::Xaml::Visibility DefaultThemePreviewVisibility() const;
  Microsoft::UI::Xaml::Visibility SaveButtonVisibility() const;

  // Preview properties for default themes (read-only)
  winrt::hstring PreviewThemeName() const;
  winrt::hstring PreviewBaseTheme() const;
  Microsoft::UI::Xaml::Media::Brush PreviewNavPaneBgBrush() const;
  Microsoft::UI::Xaml::Media::Brush PreviewCardBgBrush() const;
  Microsoft::UI::Xaml::Media::Brush PreviewBtnHoverBrush() const;
  Microsoft::UI::Xaml::Media::Brush PreviewBtnSelectedBrush() const;

  // Actions
  winrt::Windows::Foundation::IAsyncAction PickBackgroundImageAsync();
  winrt::Windows::Foundation::IAsyncAction PickNavPaneBackgroundImageAsync();
  void ClearBackgroundImage();
  void ClearNavPaneBackgroundImage();
  void SwitchToCustomSlot(int32_t slot);
  winrt::Windows::Foundation::IAsyncAction SaveSettingsAsync();

  // INotifyPropertyChanged
  winrt::event_token PropertyChanged(
      Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const &handler);
  void PropertyChanged(winrt::event_token const &token) noexcept;

private:
  void RaisePropertyChanged(std::wstring_view name);
  void UpdateCustomConfig();
  void LoadFromService();
  void LoadFromConfig(::EtherealScepter::Services::CustomThemeConfig const &config);
  void RaiseAllPropertiesChanged();
  Microsoft::UI::Xaml::Media::SolidColorBrush CreateBrushFromColor(
      ::EtherealScepter::Services::ColorConfig const &color) const;

  winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler>
      m_propertyChanged;

  int32_t m_selectedThemeIndex = 0;
  int32_t m_currentCustomSlot = 1;
  
  // Custom theme settings
  winrt::hstring m_customThemeName;
  winrt::hstring m_backgroundImagePath;
  winrt::hstring m_navPaneBackgroundImagePath;
  double m_panelOpacity = 0.8;
  double m_navPaneOpacity = 0.8;
  int32_t m_baseThemeIndex = 1; // Dark by default

  // Navigation pane background
  uint8_t m_navPaneBgR = 32;
  uint8_t m_navPaneBgG = 32;
  uint8_t m_navPaneBgB = 32;
  double m_navPaneBgOpacity = 0.8;

  // Navigation button normal
  uint8_t m_navBtnNormalR = 0;
  uint8_t m_navBtnNormalG = 0;
  uint8_t m_navBtnNormalB = 0;
  double m_navBtnNormalOpacity = 0.0;

  // Navigation button hover
  uint8_t m_navBtnHoverR = 255;
  uint8_t m_navBtnHoverG = 255;
  uint8_t m_navBtnHoverB = 255;
  double m_navBtnHoverOpacity = 0.1;

  // Navigation button selected
  uint8_t m_navBtnSelectedR = 0;
  uint8_t m_navBtnSelectedG = 120;
  uint8_t m_navBtnSelectedB = 215;
  double m_navBtnSelectedOpacity = 0.3;

  // Card background
  uint8_t m_cardBgR = 45;
  uint8_t m_cardBgG = 45;
  uint8_t m_cardBgB = 45;
  double m_cardBgOpacity = 0.8;

  ::EtherealScepter::Services::ThemeService &m_themeService;
};

} // namespace winrt::EtherealScepter::ViewModels::implementation

namespace winrt::EtherealScepter::ViewModels::factory_implementation {

struct SettingsPageViewModel
    : SettingsPageViewModelT<SettingsPageViewModel,
                              implementation::SettingsPageViewModel> {};

} // namespace winrt::EtherealScepter::ViewModels::factory_implementation
