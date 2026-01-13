// Created by: EternityBoundary
#include "pch.h"
#include "include/Services/ThemeService.h"

#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Data.Json.h>

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Data::Json;

namespace EtherealScepter::Services {

ThemeService &ThemeService::Instance() {
  static ThemeService instance;
  return instance;
}

ThemeService::ThemeService() {
  // Initialize default custom theme names
  m_customConfig1.themeName = L"Custom Theme 1";
  m_customConfig2.themeName = L"Custom Theme 2";
}

void ThemeService::Initialize(FrameworkElement const &rootElement) {
  std::lock_guard lock(m_mutex);
  m_rootElement = rootElement;
  ApplyTheme();
}

ThemeType ThemeService::GetCurrentTheme() const {
  std::lock_guard lock(m_mutex);
  return m_currentTheme;
}

void ThemeService::SetTheme(ThemeType theme) {
  {
    std::lock_guard lock(m_mutex);
    if (m_currentTheme == theme)
      return;
    m_currentTheme = theme;
    ClearCachedBrushes();
  }

  ApplyTheme();
  NotifySubscribers();
  
  // 自動儲存使用者選擇的主題
  SaveSettingsAsync();
}

CustomThemeConfig const &ThemeService::GetCustomThemeConfig(int slot) const {
  std::lock_guard lock(m_mutex);
  return (slot == 2) ? m_customConfig2 : m_customConfig1;
}

CustomThemeConfig const &ThemeService::GetActiveCustomThemeConfig() const {
  std::lock_guard lock(m_mutex);
  if (m_currentTheme == ThemeType::Custom2) {
    return m_customConfig2;
  }
  return m_customConfig1;
}

void ThemeService::SetCustomThemeConfig(CustomThemeConfig const &config, int slot) {
  ThemeType currentTheme;
  {
    std::lock_guard lock(m_mutex);
    if (slot == 2) {
      m_customConfig2 = config;
    } else {
      m_customConfig1 = config;
    }
    ClearCachedBrushes();
    currentTheme = m_currentTheme;
  }

  // If the corresponding custom theme is active, reapply
  if ((slot == 1 && currentTheme == ThemeType::Custom1) ||
      (slot == 2 && currentTheme == ThemeType::Custom2)) {
    ApplyTheme();
    NotifySubscribers();
  }
}

bool ThemeService::IsCustomThemeActive() const {
  std::lock_guard lock(m_mutex);
  return m_currentTheme == ThemeType::Custom1 || m_currentTheme == ThemeType::Custom2;
}

// Private helper - must be called with lock already held
bool ThemeService::IsCustomThemeActiveInternal() const {
  return m_currentTheme == ThemeType::Custom1 || m_currentTheme == ThemeType::Custom2;
}

Brush ThemeService::GetBackgroundBrush() const {
  std::lock_guard lock(m_mutex);

  if (!IsCustomThemeActiveInternal()) {
    return nullptr;
  }

  auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;

  if (config.backgroundImagePath.empty()) {
    return nullptr;
  }

  if (!m_backgroundBrush) {
    m_backgroundBrush = CreateImageBrush(config.backgroundImagePath);
  }

  return m_backgroundBrush;
}

Brush ThemeService::GetNavPaneBackgroundBrush() const {
  std::lock_guard lock(m_mutex);

  if (!IsCustomThemeActiveInternal()) {
    return nullptr;
  }

  auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;

  if (!config.navPaneBackgroundImagePath.empty()) {
    if (!m_navPaneImageBrush) {
      m_navPaneImageBrush = CreateImageBrush(config.navPaneBackgroundImagePath);
    }
    return m_navPaneImageBrush;
  }

  return nullptr;
}

Brush ThemeService::GetNavPaneColorBrush() const {
  std::lock_guard lock(m_mutex);

  if (!IsCustomThemeActiveInternal()) {
    return nullptr;
  }

  auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;

  if (!m_navPaneColorBrush) {
    m_navPaneColorBrush = CreateColorBrush(config.navPaneBackgroundColor);
  }

  return m_navPaneColorBrush;
}

Brush ThemeService::GetNavButtonNormalBrush() const {
  std::lock_guard lock(m_mutex);

  if (!IsCustomThemeActiveInternal()) {
    return nullptr;
  }

  auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;

  if (!m_navButtonNormalBrush) {
    m_navButtonNormalBrush = CreateColorBrush(config.navButtonNormalColor);
  }

  return m_navButtonNormalBrush;
}

Brush ThemeService::GetNavButtonHoverBrush() const {
  std::lock_guard lock(m_mutex);

  if (!IsCustomThemeActiveInternal()) {
    return nullptr;
  }

  auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;

  if (!m_navButtonHoverBrush) {
    m_navButtonHoverBrush = CreateColorBrush(config.navButtonHoverColor);
  }

  return m_navButtonHoverBrush;
}

Brush ThemeService::GetNavButtonSelectedBrush() const {
  std::lock_guard lock(m_mutex);

  if (!IsCustomThemeActiveInternal()) {
    return nullptr;
  }

  auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;

  if (!m_navButtonSelectedBrush) {
    m_navButtonSelectedBrush = CreateColorBrush(config.navButtonSelectedColor);
  }

  return m_navButtonSelectedBrush;
}

Brush ThemeService::GetCardBackgroundBrush() const {
  std::lock_guard lock(m_mutex);

  if (!IsCustomThemeActiveInternal()) {
    return nullptr;
  }

  auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;

  if (!m_cardBackgroundBrush) {
    m_cardBackgroundBrush = CreateColorBrush(config.cardBackgroundColor);
  }

  return m_cardBackgroundBrush;
}

double ThemeService::GetPanelOpacity() const {
  std::lock_guard lock(m_mutex);
  if (IsCustomThemeActiveInternal()) {
    auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;
    return config.panelOpacity;
  }
  return 1.0;
}

double ThemeService::GetNavPaneOpacity() const {
  std::lock_guard lock(m_mutex);
  if (IsCustomThemeActiveInternal()) {
    auto const &config = (m_currentTheme == ThemeType::Custom2) ? m_customConfig2 : m_customConfig1;
    return config.navPaneOpacity;
  }
  return 1.0;
}

uint32_t ThemeService::Subscribe(ThemeChangedCallback callback) {
  std::lock_guard lock(m_mutex);
  uint32_t id = m_nextSubscriptionId++;
  m_subscribers[id] = std::move(callback);
  return id;
}

void ThemeService::Unsubscribe(uint32_t subscriptionId) {
  std::lock_guard lock(m_mutex);
  m_subscribers.erase(subscriptionId);
}

void ThemeService::ApplyTheme() {
  if (!m_rootElement)
    return;

  ElementTheme theme = ElementTheme::Default;

  switch (m_currentTheme) {
  case ThemeType::System:
    theme = ElementTheme::Default;
    break;
  case ThemeType::Light:
    theme = ElementTheme::Light;
    break;
  case ThemeType::Dark:
    theme = ElementTheme::Dark;
    break;
  case ThemeType::Custom1:
    theme = m_customConfig1.baseTheme;
    break;
  case ThemeType::Custom2:
    theme = m_customConfig2.baseTheme;
    break;
  }

  m_rootElement.RequestedTheme(theme);
}

void ThemeService::NotifySubscribers() {
  std::map<uint32_t, ThemeChangedCallback> subscribers;
  ThemeType currentTheme;
  {
    std::lock_guard lock(m_mutex);
    subscribers = m_subscribers;
    currentTheme = m_currentTheme;
  }

  for (auto const &[id, callback] : subscribers) {
    if (callback) {
      callback(currentTheme);
    }
  }
}

void ThemeService::ClearCachedBrushes() {
  m_backgroundBrush = nullptr;
  m_navPaneImageBrush = nullptr;
  m_navPaneColorBrush = nullptr;
  m_navButtonNormalBrush = nullptr;
  m_navButtonHoverBrush = nullptr;
  m_navButtonSelectedBrush = nullptr;
  m_cardBackgroundBrush = nullptr;
}

Brush ThemeService::CreateImageBrush(std::wstring const &imagePath) const {
  if (imagePath.empty())
    return nullptr;

  try {
    ImageBrush brush;
    BitmapImage bitmap;
    bitmap.UriSource(winrt::Windows::Foundation::Uri(imagePath));
    brush.ImageSource(bitmap);
    brush.Stretch(Stretch::UniformToFill);
    return brush;
  } catch (...) {
    return nullptr;
  }
}

Brush ThemeService::CreateColorBrush(ColorConfig const &config) const {
  SolidColorBrush brush;
  winrt::Windows::UI::Color color;
  color.A = static_cast<uint8_t>(config.opacity * 255);
  color.R = config.r;
  color.G = config.g;
  color.B = config.b;
  brush.Color(color);
  return brush;
}

// Helper to save ColorConfig to JSON
static void SaveColorConfigToJson(JsonObject &parent, std::wstring const &prefix, ColorConfig const &config) {
  parent.SetNamedValue(prefix + L"_r", JsonValue::CreateNumberValue(config.r));
  parent.SetNamedValue(prefix + L"_g", JsonValue::CreateNumberValue(config.g));
  parent.SetNamedValue(prefix + L"_b", JsonValue::CreateNumberValue(config.b));
  parent.SetNamedValue(prefix + L"_opacity", JsonValue::CreateNumberValue(config.opacity));
}

// Helper to load ColorConfig from JSON
static void LoadColorConfigFromJson(JsonObject const &parent, std::wstring const &prefix, ColorConfig &config) {
  if (parent.HasKey(prefix + L"_r")) {
    config.r = static_cast<uint8_t>(parent.GetNamedNumber(prefix + L"_r"));
  }
  if (parent.HasKey(prefix + L"_g")) {
    config.g = static_cast<uint8_t>(parent.GetNamedNumber(prefix + L"_g"));
  }
  if (parent.HasKey(prefix + L"_b")) {
    config.b = static_cast<uint8_t>(parent.GetNamedNumber(prefix + L"_b"));
  }
  if (parent.HasKey(prefix + L"_opacity")) {
    config.opacity = parent.GetNamedNumber(prefix + L"_opacity");
  }
}

// Helper to save CustomThemeConfig to JSON
static void SaveCustomConfigToJson(JsonObject &parent, std::wstring const &prefix, CustomThemeConfig const &config) {
  parent.SetNamedValue(prefix + L"_themeName", JsonValue::CreateStringValue(config.themeName));
  parent.SetNamedValue(prefix + L"_backgroundImagePath", JsonValue::CreateStringValue(config.backgroundImagePath));
  parent.SetNamedValue(prefix + L"_navPaneBackgroundImagePath", JsonValue::CreateStringValue(config.navPaneBackgroundImagePath));
  parent.SetNamedValue(prefix + L"_panelOpacity", JsonValue::CreateNumberValue(config.panelOpacity));
  parent.SetNamedValue(prefix + L"_navPaneOpacity", JsonValue::CreateNumberValue(config.navPaneOpacity));
  parent.SetNamedValue(prefix + L"_baseTheme", JsonValue::CreateNumberValue(static_cast<int>(config.baseTheme)));
  parent.SetNamedValue(prefix + L"_accentColorHex", JsonValue::CreateStringValue(config.accentColorHex));

  SaveColorConfigToJson(parent, prefix + L"_navPaneBg", config.navPaneBackgroundColor);
  SaveColorConfigToJson(parent, prefix + L"_navBtnNormal", config.navButtonNormalColor);
  SaveColorConfigToJson(parent, prefix + L"_navBtnHover", config.navButtonHoverColor);
  SaveColorConfigToJson(parent, prefix + L"_navBtnSelected", config.navButtonSelectedColor);
  SaveColorConfigToJson(parent, prefix + L"_cardBg", config.cardBackgroundColor);
}

// Helper to load CustomThemeConfig from JSON
static void LoadCustomConfigFromJson(JsonObject const &parent, std::wstring const &prefix, CustomThemeConfig &config) {
  if (parent.HasKey(prefix + L"_themeName")) {
    config.themeName = parent.GetNamedString(prefix + L"_themeName").c_str();
  }
  if (parent.HasKey(prefix + L"_backgroundImagePath")) {
    config.backgroundImagePath = parent.GetNamedString(prefix + L"_backgroundImagePath").c_str();
  }
  if (parent.HasKey(prefix + L"_navPaneBackgroundImagePath")) {
    config.navPaneBackgroundImagePath = parent.GetNamedString(prefix + L"_navPaneBackgroundImagePath").c_str();
  }
  if (parent.HasKey(prefix + L"_panelOpacity")) {
    config.panelOpacity = parent.GetNamedNumber(prefix + L"_panelOpacity");
  }
  if (parent.HasKey(prefix + L"_navPaneOpacity")) {
    config.navPaneOpacity = parent.GetNamedNumber(prefix + L"_navPaneOpacity");
  }
  if (parent.HasKey(prefix + L"_baseTheme")) {
    config.baseTheme = static_cast<ElementTheme>(static_cast<int>(parent.GetNamedNumber(prefix + L"_baseTheme")));
  }
  if (parent.HasKey(prefix + L"_accentColorHex")) {
    config.accentColorHex = parent.GetNamedString(prefix + L"_accentColorHex").c_str();
  }

  LoadColorConfigFromJson(parent, prefix + L"_navPaneBg", config.navPaneBackgroundColor);
  LoadColorConfigFromJson(parent, prefix + L"_navBtnNormal", config.navButtonNormalColor);
  LoadColorConfigFromJson(parent, prefix + L"_navBtnHover", config.navButtonHoverColor);
  LoadColorConfigFromJson(parent, prefix + L"_navBtnSelected", config.navButtonSelectedColor);
  LoadColorConfigFromJson(parent, prefix + L"_cardBg", config.cardBackgroundColor);
}

winrt::Windows::Foundation::IAsyncAction ThemeService::SaveSettingsAsync() {
  auto localFolder = ApplicationData::Current().LocalFolder();

  JsonObject json;

  {
    std::lock_guard lock(m_mutex);
    json.SetNamedValue(L"themeType", JsonValue::CreateNumberValue(static_cast<int>(m_currentTheme)));
  }

  auto file = co_await localFolder.CreateFileAsync(L"theme_settings.json",
      CreationCollisionOption::ReplaceExisting);
  co_await FileIO::WriteTextAsync(file, json.Stringify());
}

winrt::Windows::Foundation::IAsyncAction ThemeService::LoadSettingsAsync() {
  auto localFolder = ApplicationData::Current().LocalFolder();

  try {
    auto file = co_await localFolder.GetFileAsync(L"theme_settings.json");
    auto content = co_await FileIO::ReadTextAsync(file);

    JsonObject json;
    if (JsonObject::TryParse(content, json)) {
      {
        std::lock_guard lock(m_mutex);

        // 只載入使用者選擇的主題類型，不載入自定義主題配置
        // 預設主題的配置由 RegisterSystemDefaultThemes() 設定
        if (json.HasKey(L"themeType")) {
          m_currentTheme = static_cast<ThemeType>(
              static_cast<int>(json.GetNamedNumber(L"themeType")));
        }

        ClearCachedBrushes();
      }
      
      // 在鎖外面通知訂閱者
      ApplyTheme();
      NotifySubscribers();
    }
  } catch (...) {
    // File doesn't exist or is invalid, use defaults
    ApplyTheme();
  }
}

} // namespace EtherealScepter::Services
