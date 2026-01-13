// Created by: EternityBoundary on Jan 12, 2026
#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>

namespace EtherealScepter::Services {

// Color with alpha configuration
struct ColorConfig {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  double opacity = 1.0; // 0.0 - 1.0

  bool IsDefault() const {
    return r == 0 && g == 0 && b == 0 && opacity == 1.0;
  }
};

/// <summary>
/// Custom theme configuration for user-defined themes
/// </summary>
struct CustomThemeConfig {
  // Background image path (empty = no image)
  std::wstring backgroundImagePath;

  // Navigation pane background image path (empty = use main background or
  // default)
  std::wstring navPaneBackgroundImagePath;

  // Panel/Card opacity (0.0 - 1.0) - for backward compatibility
  double panelOpacity = 0.8;

  // Navigation pane opacity (0.0 - 1.0) - for backward compatibility
  double navPaneOpacity = 0.8;

  // Base theme to use (Light/Dark)
  winrt::Microsoft::UI::Xaml::ElementTheme baseTheme =
      winrt::Microsoft::UI::Xaml::ElementTheme::Dark;

  // Accent color override (empty = use system accent)
  std::wstring accentColorHex;

  // === Extended color configurations ===

  // Navigation pane (sidebar) background color
  ColorConfig navPaneBackgroundColor{32, 32, 32,
                                     0.8}; // Dark gray with 80% opacity

  // Navigation pane button colors
  ColorConfig navButtonNormalColor{0, 0, 0, 0.0};      // Transparent by default
  ColorConfig navButtonHoverColor{255, 255, 255, 0.1}; // Slight white highlight
  ColorConfig navButtonSelectedColor{0, 120, 215, 0.3}; // Accent blue

  // Main content area card/panel colors
  ColorConfig cardBackgroundColor{45, 45, 45, 0.8}; // Slightly lighter gray

  // Custom theme name for identification
  std::wstring themeName = L"Custom Theme";
};

// Theme types supported by the application
enum class ThemeType { System, Light, Dark, Custom1, Custom2 };

// Singleton service for managing application themes
class ThemeService {
public:
  using ThemeChangedCallback = std::function<void(ThemeType)>;

  static ThemeService &Instance();

  // Prevent copying
  ThemeService(ThemeService const &) = delete;
  ThemeService &operator=(ThemeService const &) = delete;

  // Initialize the service with the main window's root element
  void
  Initialize(winrt::Microsoft::UI::Xaml::FrameworkElement const &rootElement);

  // Get the current theme type
  ThemeType GetCurrentTheme() const;

  // Set the application theme
  void SetTheme(ThemeType theme);

  // Get the custom theme configuration for a specific slot (1 or 2)
  CustomThemeConfig const &GetCustomThemeConfig(int slot = 1) const;

  // Get the current active custom theme config (if custom theme is active)
  CustomThemeConfig const &GetActiveCustomThemeConfig() const;

  // Set the custom theme configuration for a specific slot (1 or 2)
  void SetCustomThemeConfig(CustomThemeConfig const &config, int slot = 1);

  // Get the background image brush (if custom theme with background is active)
  winrt::Microsoft::UI::Xaml::Media::Brush GetBackgroundBrush() const;

  // Get the navigation pane background brush
  winrt::Microsoft::UI::Xaml::Media::Brush GetNavPaneBackgroundBrush() const;

  // Get the navigation pane background color brush (solid color)
  winrt::Microsoft::UI::Xaml::Media::Brush GetNavPaneColorBrush() const;

  // Get the navigation button normal state brush
  winrt::Microsoft::UI::Xaml::Media::Brush GetNavButtonNormalBrush() const;

  // Get the navigation button hover state brush
  winrt::Microsoft::UI::Xaml::Media::Brush GetNavButtonHoverBrush() const;

  // Get the navigation button selected state brush
  winrt::Microsoft::UI::Xaml::Media::Brush GetNavButtonSelectedBrush() const;

  // Get the card/panel background brush
  winrt::Microsoft::UI::Xaml::Media::Brush GetCardBackgroundBrush() const;

  // Get the panel/card opacity
  double GetPanelOpacity() const;

  // Get the navigation pane opacity
  double GetNavPaneOpacity() const;

  // Check if custom theme is currently active
  bool IsCustomThemeActive() const;

  // Subscribe to theme changes
  uint32_t Subscribe(ThemeChangedCallback callback);

  // Unsubscribe from theme changes
  void Unsubscribe(uint32_t subscriptionId);

  // Save theme settings to local storage
  winrt::Windows::Foundation::IAsyncAction SaveSettingsAsync();

  // Load theme settings from local storage
  winrt::Windows::Foundation::IAsyncAction LoadSettingsAsync();

private:
  ThemeService();
  ~ThemeService() = default;

  void ApplyTheme();
  void NotifySubscribers();
  winrt::Microsoft::UI::Xaml::Media::Brush
  CreateImageBrush(std::wstring const &imagePath) const;
  winrt::Microsoft::UI::Xaml::Media::Brush
  CreateColorBrush(ColorConfig const &config) const;
  void ClearCachedBrushes();
  bool IsCustomThemeActiveInternal() const;

  mutable std::mutex m_mutex;
  ThemeType m_currentTheme = ThemeType::System;
  CustomThemeConfig m_customConfig1; // Custom theme slot 1
  CustomThemeConfig m_customConfig2; // Custom theme slot 2
  winrt::Microsoft::UI::Xaml::FrameworkElement m_rootElement{nullptr};

  std::map<uint32_t, ThemeChangedCallback> m_subscribers;
  uint32_t m_nextSubscriptionId = 1;

  // Cached brushes
  mutable winrt::Microsoft::UI::Xaml::Media::Brush m_backgroundBrush{nullptr};
  mutable winrt::Microsoft::UI::Xaml::Media::Brush m_navPaneImageBrush{nullptr};
  mutable winrt::Microsoft::UI::Xaml::Media::Brush m_navPaneColorBrush{nullptr};
  mutable winrt::Microsoft::UI::Xaml::Media::Brush m_navButtonNormalBrush{
      nullptr};
  mutable winrt::Microsoft::UI::Xaml::Media::Brush m_navButtonHoverBrush{
      nullptr};
  mutable winrt::Microsoft::UI::Xaml::Media::Brush m_navButtonSelectedBrush{
      nullptr};
  mutable winrt::Microsoft::UI::Xaml::Media::Brush m_cardBackgroundBrush{
      nullptr};
};

} // namespace EtherealScepter::Services
