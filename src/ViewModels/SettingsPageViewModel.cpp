// Created by: EternityBoundary
#include "pch.h"

#include "include/ViewModels/SettingsPageViewModel.h"
#if __has_include("ViewModels.SettingsPageViewModel.g.cpp")
#include "ViewModels.SettingsPageViewModel.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <Shobjidl.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Data;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Windows::Storage::Pickers;
namespace Services = ::EtherealScepter::Services;

namespace winrt::EtherealScepter::ViewModels::implementation {

SettingsPageViewModel::SettingsPageViewModel()
    : m_themeService(Services::ThemeService::Instance()) {
  LoadFromService();
}

SettingsPageViewModel::~SettingsPageViewModel() {}

void SettingsPageViewModel::LoadFromService() {
  auto currentTheme = m_themeService.GetCurrentTheme();
  m_selectedThemeIndex = static_cast<int32_t>(currentTheme);

  // Determine which slot to load
  if (currentTheme == Services::ThemeType::Custom2) {
    m_currentCustomSlot = 2;
  } else {
    m_currentCustomSlot = 1;
  }

  auto const &config = m_themeService.GetCustomThemeConfig(m_currentCustomSlot);
  LoadFromConfig(config);
}

void SettingsPageViewModel::LoadFromConfig(Services::CustomThemeConfig const &config) {
  m_customThemeName = config.themeName;
  m_backgroundImagePath = config.backgroundImagePath;
  m_navPaneBackgroundImagePath = config.navPaneBackgroundImagePath;
  m_panelOpacity = config.panelOpacity;
  m_navPaneOpacity = config.navPaneOpacity;
  m_baseThemeIndex = (config.baseTheme == ElementTheme::Light) ? 0 : 1;

  // Navigation pane background
  m_navPaneBgR = config.navPaneBackgroundColor.r;
  m_navPaneBgG = config.navPaneBackgroundColor.g;
  m_navPaneBgB = config.navPaneBackgroundColor.b;
  m_navPaneBgOpacity = config.navPaneBackgroundColor.opacity;

  // Navigation button normal
  m_navBtnNormalR = config.navButtonNormalColor.r;
  m_navBtnNormalG = config.navButtonNormalColor.g;
  m_navBtnNormalB = config.navButtonNormalColor.b;
  m_navBtnNormalOpacity = config.navButtonNormalColor.opacity;

  // Navigation button hover
  m_navBtnHoverR = config.navButtonHoverColor.r;
  m_navBtnHoverG = config.navButtonHoverColor.g;
  m_navBtnHoverB = config.navButtonHoverColor.b;
  m_navBtnHoverOpacity = config.navButtonHoverColor.opacity;

  // Navigation button selected
  m_navBtnSelectedR = config.navButtonSelectedColor.r;
  m_navBtnSelectedG = config.navButtonSelectedColor.g;
  m_navBtnSelectedB = config.navButtonSelectedColor.b;
  m_navBtnSelectedOpacity = config.navButtonSelectedColor.opacity;

  // Card background
  m_cardBgR = config.cardBackgroundColor.r;
  m_cardBgG = config.cardBackgroundColor.g;
  m_cardBgB = config.cardBackgroundColor.b;
  m_cardBgOpacity = config.cardBackgroundColor.opacity;
}

void SettingsPageViewModel::RaiseAllPropertiesChanged() {
  RaisePropertyChanged(L"CustomThemeName");
  RaisePropertyChanged(L"BackgroundImagePath");
  RaisePropertyChanged(L"NavPaneBackgroundImagePath");
  RaisePropertyChanged(L"PanelOpacity");
  RaisePropertyChanged(L"NavPaneOpacity");
  RaisePropertyChanged(L"BaseThemeIndex");
  RaisePropertyChanged(L"NavPaneBgR");
  RaisePropertyChanged(L"NavPaneBgG");
  RaisePropertyChanged(L"NavPaneBgB");
  RaisePropertyChanged(L"NavPaneBgOpacity");
  RaisePropertyChanged(L"NavBtnNormalR");
  RaisePropertyChanged(L"NavBtnNormalG");
  RaisePropertyChanged(L"NavBtnNormalB");
  RaisePropertyChanged(L"NavBtnNormalOpacity");
  RaisePropertyChanged(L"NavBtnHoverR");
  RaisePropertyChanged(L"NavBtnHoverG");
  RaisePropertyChanged(L"NavBtnHoverB");
  RaisePropertyChanged(L"NavBtnHoverOpacity");
  RaisePropertyChanged(L"NavBtnSelectedR");
  RaisePropertyChanged(L"NavBtnSelectedG");
  RaisePropertyChanged(L"NavBtnSelectedB");
  RaisePropertyChanged(L"NavBtnSelectedOpacity");
  RaisePropertyChanged(L"CardBgR");
  RaisePropertyChanged(L"CardBgG");
  RaisePropertyChanged(L"CardBgB");
  RaisePropertyChanged(L"CardBgOpacity");
  RaisePropertyChanged(L"CurrentSlotDisplayText");
  RaisePropertyChanged(L"DefaultThemePreviewVisibility");
  RaisePropertyChanged(L"PreviewThemeName");
  RaisePropertyChanged(L"PreviewBaseTheme");
  RaisePropertyChanged(L"PreviewNavPaneBgBrush");
  RaisePropertyChanged(L"PreviewCardBgBrush");
  RaisePropertyChanged(L"PreviewBtnHoverBrush");
  RaisePropertyChanged(L"PreviewBtnSelectedBrush");
}

int32_t SettingsPageViewModel::SelectedThemeIndex() const {
  return m_selectedThemeIndex;
}

void SettingsPageViewModel::SelectedThemeIndex(int32_t value) {
  if (m_selectedThemeIndex != value) {
    m_selectedThemeIndex = value;
    m_themeService.SetTheme(static_cast<Services::ThemeType>(value));
    RaisePropertyChanged(L"SelectedThemeIndex");
    RaisePropertyChanged(L"IsCustomThemeEnabled");
    RaisePropertyChanged(L"CustomThemeVisibility");
    RaisePropertyChanged(L"DefaultThemePreviewVisibility");
    RaisePropertyChanged(L"SaveButtonVisibility");
    RaisePropertyChanged(L"PreviewThemeName");
    RaisePropertyChanged(L"PreviewBaseTheme");
    RaisePropertyChanged(L"PreviewNavPaneBgBrush");
    RaisePropertyChanged(L"PreviewCardBgBrush");
    RaisePropertyChanged(L"PreviewBtnHoverBrush");
    RaisePropertyChanged(L"PreviewBtnSelectedBrush");

    // Update custom slot when selecting custom themes
    if (value == 3) { // Custom1
      SwitchToCustomSlot(1);
    } else if (value == 4) { // Custom2
      SwitchToCustomSlot(2);
    }
  }
}

int32_t SettingsPageViewModel::CurrentCustomSlot() const {
  return m_currentCustomSlot;
}

void SettingsPageViewModel::CurrentCustomSlot(int32_t value) {
  SwitchToCustomSlot(value);
}

hstring SettingsPageViewModel::CurrentSlotDisplayText() const {
  return L"(Editing Slot " + to_hstring(m_currentCustomSlot) + L")";
}

// Default theme names 
hstring SettingsPageViewModel::DefaultTheme1Name() const {
  return L"Starry Sky";
}

hstring SettingsPageViewModel::DefaultTheme2Name() const {
  return L"Black Souls Leaf";
}

// Preview properties for default themes
hstring SettingsPageViewModel::PreviewThemeName() const {
  if (m_selectedThemeIndex == 3) {
    return L"Starry Sky";
  } else if (m_selectedThemeIndex == 4) {
    return L"Black Souls Leaf";
  }
  return L"";
}

hstring SettingsPageViewModel::PreviewBaseTheme() const {
  Services::CustomThemeConfig const* config = nullptr;
  if (m_selectedThemeIndex == 3) {
    config = &m_themeService.GetCustomThemeConfig(1);
  } else if (m_selectedThemeIndex == 4) {
    config = &m_themeService.GetCustomThemeConfig(2);
  }
  
  if (config) {
    return config->baseTheme == ElementTheme::Light ? L"Light" : L"Dark";
  }
  return L"";
}

SolidColorBrush SettingsPageViewModel::CreateBrushFromColor(Services::ColorConfig const &color) const {
  SolidColorBrush brush;
  winrt::Windows::UI::Color c;
  c.A = static_cast<uint8_t>(color.opacity * 255);
  c.R = color.r;
  c.G = color.g;
  c.B = color.b;
  brush.Color(c);
  return brush;
}

Brush SettingsPageViewModel::PreviewNavPaneBgBrush() const {
  if (m_selectedThemeIndex == 3) {
    return CreateBrushFromColor(m_themeService.GetCustomThemeConfig(1).navPaneBackgroundColor);
  } else if (m_selectedThemeIndex == 4) {
    return CreateBrushFromColor(m_themeService.GetCustomThemeConfig(2).navPaneBackgroundColor);
  }
  return nullptr;
}

Brush SettingsPageViewModel::PreviewCardBgBrush() const {
  if (m_selectedThemeIndex == 3) {
    return CreateBrushFromColor(m_themeService.GetCustomThemeConfig(1).cardBackgroundColor);
  } else if (m_selectedThemeIndex == 4) {
    return CreateBrushFromColor(m_themeService.GetCustomThemeConfig(2).cardBackgroundColor);
  }
  return nullptr;
}

Brush SettingsPageViewModel::PreviewBtnHoverBrush() const {
  if (m_selectedThemeIndex == 3) {
    return CreateBrushFromColor(m_themeService.GetCustomThemeConfig(1).navButtonHoverColor);
  } else if (m_selectedThemeIndex == 4) {
    return CreateBrushFromColor(m_themeService.GetCustomThemeConfig(2).navButtonHoverColor);
  }
  return nullptr;
}

Brush SettingsPageViewModel::PreviewBtnSelectedBrush() const {
  if (m_selectedThemeIndex == 3) {
    return CreateBrushFromColor(m_themeService.GetCustomThemeConfig(1).navButtonSelectedColor);
  } else if (m_selectedThemeIndex == 4) {
    return CreateBrushFromColor(m_themeService.GetCustomThemeConfig(2).navButtonSelectedColor);
  }
  return nullptr;
}

// Visibility for default theme preview (only show for Custom1/Custom2, which are now default themes)
Visibility SettingsPageViewModel::DefaultThemePreviewVisibility() const {
  // 顯示預設主題預覽（Custom1 和 Custom2 現在是預設主題，只讀）
  return (m_selectedThemeIndex == 3 || m_selectedThemeIndex == 4) 
         ? Visibility::Visible : Visibility::Collapsed;
}

Visibility SettingsPageViewModel::SaveButtonVisibility() const {
  // 只在非預設主題時顯示儲存按鈕（目前沒有可編輯的自定義主題）
  // 如果將來添加可編輯的自定義主題槽位，這裡需要修改
  return Visibility::Collapsed;
}

void SettingsPageViewModel::SwitchToCustomSlot(int32_t slot) {
  if (slot < 1) slot = 1;
  if (slot > 2) slot = 2;

  if (m_currentCustomSlot != slot) {
    m_currentCustomSlot = slot;
    auto const &config = m_themeService.GetCustomThemeConfig(slot);
    LoadFromConfig(config);
    RaisePropertyChanged(L"CurrentCustomSlot");
    RaiseAllPropertiesChanged();
  }
}

hstring SettingsPageViewModel::CustomThemeName() const {
  return m_customThemeName;
}

void SettingsPageViewModel::CustomThemeName(hstring const &value) {
  if (m_customThemeName != value) {
    m_customThemeName = value;
    UpdateCustomConfig();
    RaisePropertyChanged(L"CustomThemeName");
  }
}

hstring SettingsPageViewModel::BackgroundImagePath() const {
  return m_backgroundImagePath;
}

void SettingsPageViewModel::BackgroundImagePath(hstring const &value) {
  if (m_backgroundImagePath != value) {
    m_backgroundImagePath = value;
    UpdateCustomConfig();
    RaisePropertyChanged(L"BackgroundImagePath");
  }
}

hstring SettingsPageViewModel::NavPaneBackgroundImagePath() const {
  return m_navPaneBackgroundImagePath;
}

void SettingsPageViewModel::NavPaneBackgroundImagePath(hstring const &value) {
  if (m_navPaneBackgroundImagePath != value) {
    m_navPaneBackgroundImagePath = value;
    UpdateCustomConfig();
    RaisePropertyChanged(L"NavPaneBackgroundImagePath");
  }
}

double SettingsPageViewModel::PanelOpacity() const { return m_panelOpacity; }

void SettingsPageViewModel::PanelOpacity(double value) {
  if (m_panelOpacity != value) {
    m_panelOpacity = value;
    UpdateCustomConfig();
    RaisePropertyChanged(L"PanelOpacity");
  }
}

double SettingsPageViewModel::NavPaneOpacity() const { return m_navPaneOpacity; }

void SettingsPageViewModel::NavPaneOpacity(double value) {
  if (m_navPaneOpacity != value) {
    m_navPaneOpacity = value;
    UpdateCustomConfig();
    RaisePropertyChanged(L"NavPaneOpacity");
  }
}

int32_t SettingsPageViewModel::BaseThemeIndex() const {
  return m_baseThemeIndex;
}

void SettingsPageViewModel::BaseThemeIndex(int32_t value) {
  if (m_baseThemeIndex != value) {
    m_baseThemeIndex = value;
    UpdateCustomConfig();
    RaisePropertyChanged(L"BaseThemeIndex");
  }
}

// Navigation pane background color
uint8_t SettingsPageViewModel::NavPaneBgR() const { return m_navPaneBgR; }
void SettingsPageViewModel::NavPaneBgR(uint8_t value) {
  if (m_navPaneBgR != value) { m_navPaneBgR = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavPaneBgR"); }
}

uint8_t SettingsPageViewModel::NavPaneBgG() const { return m_navPaneBgG; }
void SettingsPageViewModel::NavPaneBgG(uint8_t value) {
  if (m_navPaneBgG != value) { m_navPaneBgG = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavPaneBgG"); }
}

uint8_t SettingsPageViewModel::NavPaneBgB() const { return m_navPaneBgB; }
void SettingsPageViewModel::NavPaneBgB(uint8_t value) {
  if (m_navPaneBgB != value) { m_navPaneBgB = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavPaneBgB"); }
}

double SettingsPageViewModel::NavPaneBgOpacity() const { return m_navPaneBgOpacity; }
void SettingsPageViewModel::NavPaneBgOpacity(double value) {
  if (m_navPaneBgOpacity != value) { m_navPaneBgOpacity = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavPaneBgOpacity"); }
}

// Navigation button normal color
uint8_t SettingsPageViewModel::NavBtnNormalR() const { return m_navBtnNormalR; }
void SettingsPageViewModel::NavBtnNormalR(uint8_t value) {
  if (m_navBtnNormalR != value) { m_navBtnNormalR = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnNormalR"); }
}

uint8_t SettingsPageViewModel::NavBtnNormalG() const { return m_navBtnNormalG; }
void SettingsPageViewModel::NavBtnNormalG(uint8_t value) {
  if (m_navBtnNormalG != value) { m_navBtnNormalG = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnNormalG"); }
}

uint8_t SettingsPageViewModel::NavBtnNormalB() const { return m_navBtnNormalB; }
void SettingsPageViewModel::NavBtnNormalB(uint8_t value) {
  if (m_navBtnNormalB != value) { m_navBtnNormalB = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnNormalB"); }
}

double SettingsPageViewModel::NavBtnNormalOpacity() const { return m_navBtnNormalOpacity; }
void SettingsPageViewModel::NavBtnNormalOpacity(double value) {
  if (m_navBtnNormalOpacity != value) { m_navBtnNormalOpacity = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnNormalOpacity"); }
}

// Navigation button hover color
uint8_t SettingsPageViewModel::NavBtnHoverR() const { return m_navBtnHoverR; }
void SettingsPageViewModel::NavBtnHoverR(uint8_t value) {
  if (m_navBtnHoverR != value) { m_navBtnHoverR = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnHoverR"); }
}

uint8_t SettingsPageViewModel::NavBtnHoverG() const { return m_navBtnHoverG; }
void SettingsPageViewModel::NavBtnHoverG(uint8_t value) {
  if (m_navBtnHoverG != value) { m_navBtnHoverG = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnHoverG"); }
}

uint8_t SettingsPageViewModel::NavBtnHoverB() const { return m_navBtnHoverB; }
void SettingsPageViewModel::NavBtnHoverB(uint8_t value) {
  if (m_navBtnHoverB != value) { m_navBtnHoverB = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnHoverB"); }
}

double SettingsPageViewModel::NavBtnHoverOpacity() const { return m_navBtnHoverOpacity; }
void SettingsPageViewModel::NavBtnHoverOpacity(double value) {
  if (m_navBtnHoverOpacity != value) { m_navBtnHoverOpacity = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnHoverOpacity"); }
}

// Navigation button selected color
uint8_t SettingsPageViewModel::NavBtnSelectedR() const { return m_navBtnSelectedR; }
void SettingsPageViewModel::NavBtnSelectedR(uint8_t value) {
  if (m_navBtnSelectedR != value) { m_navBtnSelectedR = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnSelectedR"); }
}

uint8_t SettingsPageViewModel::NavBtnSelectedG() const { return m_navBtnSelectedG; }
void SettingsPageViewModel::NavBtnSelectedG(uint8_t value) {
  if (m_navBtnSelectedG != value) { m_navBtnSelectedG = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnSelectedG"); }
}

uint8_t SettingsPageViewModel::NavBtnSelectedB() const { return m_navBtnSelectedB; }
void SettingsPageViewModel::NavBtnSelectedB(uint8_t value) {
  if (m_navBtnSelectedB != value) { m_navBtnSelectedB = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnSelectedB"); }
}

double SettingsPageViewModel::NavBtnSelectedOpacity() const { return m_navBtnSelectedOpacity; }
void SettingsPageViewModel::NavBtnSelectedOpacity(double value) {
  if (m_navBtnSelectedOpacity != value) { m_navBtnSelectedOpacity = value; UpdateCustomConfig(); RaisePropertyChanged(L"NavBtnSelectedOpacity"); }
}

// Card background color
uint8_t SettingsPageViewModel::CardBgR() const { return m_cardBgR; }
void SettingsPageViewModel::CardBgR(uint8_t value) {
  if (m_cardBgR != value) { m_cardBgR = value; UpdateCustomConfig(); RaisePropertyChanged(L"CardBgR"); }
}

uint8_t SettingsPageViewModel::CardBgG() const { return m_cardBgG; }
void SettingsPageViewModel::CardBgG(uint8_t value) {
  if (m_cardBgG != value) { m_cardBgG = value; UpdateCustomConfig(); RaisePropertyChanged(L"CardBgG"); }
}

uint8_t SettingsPageViewModel::CardBgB() const { return m_cardBgB; }
void SettingsPageViewModel::CardBgB(uint8_t value) {
  if (m_cardBgB != value) { m_cardBgB = value; UpdateCustomConfig(); RaisePropertyChanged(L"CardBgB"); }
}

double SettingsPageViewModel::CardBgOpacity() const { return m_cardBgOpacity; }
void SettingsPageViewModel::CardBgOpacity(double value) {
  if (m_cardBgOpacity != value) { m_cardBgOpacity = value; UpdateCustomConfig(); RaisePropertyChanged(L"CardBgOpacity"); }
}

bool SettingsPageViewModel::IsCustomThemeEnabled() const {
  // 現在 Custom1 和 Custom2 是預設主題（不可編輯），所以這裡返回 false
  return false;
}

Visibility SettingsPageViewModel::CustomThemeVisibility() const {
  // 可編輯的自定義主題設定面板 - 目前隱藏，因為沒有可編輯的槽位
  return Visibility::Collapsed;
}

void SettingsPageViewModel::UpdateCustomConfig() {
  // 預設主題不允許修改，所以這個函數不會有實際效果
  // 如果將來添加可編輯的自定義主題槽位，這裡需要修改
}

IAsyncAction SettingsPageViewModel::PickBackgroundImageAsync() {
  auto strong_this = get_strong();

  FileOpenPicker picker;
  
  // Get the active window handle
  HWND hwnd = GetActiveWindow();
  auto initializeWithWindow = picker.as<::IInitializeWithWindow>();
  initializeWithWindow->Initialize(hwnd);
  
  picker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
  picker.FileTypeFilter().Append(L".png");
  picker.FileTypeFilter().Append(L".jpg");
  picker.FileTypeFilter().Append(L".jpeg");
  picker.FileTypeFilter().Append(L".bmp");
  picker.FileTypeFilter().Append(L".gif");

  auto file = co_await picker.PickSingleFileAsync();
  if (file) {
    BackgroundImagePath(file.Path());
  }
}

IAsyncAction SettingsPageViewModel::PickNavPaneBackgroundImageAsync() {
  auto strong_this = get_strong();

  FileOpenPicker picker;
  
  // Get the active window handle
  HWND hwnd = GetActiveWindow();
  auto initializeWithWindow = picker.as<::IInitializeWithWindow>();
  initializeWithWindow->Initialize(hwnd);
  
  picker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
  picker.FileTypeFilter().Append(L".png");
  picker.FileTypeFilter().Append(L".jpg");
  picker.FileTypeFilter().Append(L".jpeg");
  picker.FileTypeFilter().Append(L".bmp");
  picker.FileTypeFilter().Append(L".gif");

  auto file = co_await picker.PickSingleFileAsync();
  if (file) {
    NavPaneBackgroundImagePath(file.Path());
  }
}

void SettingsPageViewModel::ClearBackgroundImage() {
  BackgroundImagePath(L"");
}

void SettingsPageViewModel::ClearNavPaneBackgroundImage() {
  NavPaneBackgroundImagePath(L"");
}

IAsyncAction SettingsPageViewModel::SaveSettingsAsync() {
  co_await m_themeService.SaveSettingsAsync();
}

event_token SettingsPageViewModel::PropertyChanged(
    PropertyChangedEventHandler const &handler) {
  return m_propertyChanged.add(handler);
}

void SettingsPageViewModel::PropertyChanged(event_token const &token) noexcept {
  m_propertyChanged.remove(token);
}

void SettingsPageViewModel::RaisePropertyChanged(std::wstring_view name) {
  m_propertyChanged(*this, PropertyChangedEventArgs{hstring{name}});
}

} // namespace winrt::EtherealScepter::ViewModels::implementation
