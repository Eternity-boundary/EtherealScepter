// Created by: EternityBoundary on Jan 12, 2026
#include "pch.h"
#include "DefaultThemes.h"
#include "include/Services/ThemeService.h"


using namespace EtherealScepter::Services;

namespace EtherealScepter::Views::Theme {

void RegisterSystemDefaultThemes() {
  auto &themeService = ThemeService::Instance();

  // 預設主題 1: StarrySky

  CustomThemeConfig StarrySky;

  // 基本資訊
  StarrySky.themeName = L"Starry Sky";
  StarrySky.baseTheme = winrt::Microsoft::UI::Xaml::ElementTheme::Dark;

  // 背景圖片設定
  // 使用 ms-appx:///Assets/Background.png 格式的應用程式資源
  StarrySky.backgroundImagePath = L"ms-appx:///Assets/StarrySkyMain.png";
  StarrySky.navPaneBackgroundImagePath = L"ms-appx:///Assets/StarrySkySide.png";

  // 側邊欄背景顏色
  StarrySky.navPaneBackgroundColor = {
      .r = 15, .g = 25, .b = 45, .opacity = 0.5};

  // 導航按鈕 - 正常狀態
  StarrySky.navButtonNormalColor = {.r = 0, .g = 0, .b = 0, .opacity = 0.8};

  // 導航按鈕 - 滑鼠懸停狀態
  StarrySky.navButtonHoverColor = {
      .r = 100, .g = 150, .b = 200, .opacity = 0.4};

  // 導航按鈕 - 選中狀態
  StarrySky.navButtonSelectedColor = {
      .r = 0, .g = 120, .b = 215, .opacity = 0.4};

  // 卡片/面板背景顏色
  StarrySky.cardBackgroundColor = {
      .r = 20, .g = 35, .b = 60, .opacity = 0.75};

  // 舊版透明度設定（向後兼容）
  StarrySky.panelOpacity = 0.85;
  StarrySky.navPaneOpacity = 0.9;

  // 強調色（十六進位格式）
  StarrySky.accentColorHex = L"#0078D7";

  // 預設主題 2:

  CustomThemeConfig Leaf;

  // 基本資訊
  Leaf.themeName = L"Black Souls Leaf";
  Leaf.baseTheme = winrt::Microsoft::UI::Xaml::ElementTheme::Dark;

  // 背景圖片設定
  Leaf.backgroundImagePath = L"ms-appx:///Assets/LeafMain.png";
  Leaf.navPaneBackgroundImagePath = L"ms-appx:///Assets/LeafSide.png";

  // 側邊欄背景顏色
  Leaf.navPaneBackgroundColor = {.r = 15, .g = 25, .b = 45, .opacity = 0.5};

  // 導航按鈕 - 正常狀態
  Leaf.navButtonNormalColor = {.r = 0, .g = 0, .b = 0, .opacity = 0.0};

  // 導航按鈕 - 滑鼠懸停狀態
  Leaf.navButtonHoverColor = {
      .r = 152, .g = 238, .b = 144, .opacity = 0.4};

  // 導航按鈕 - 選中狀態
  Leaf.navButtonSelectedColor = {
      .r = 152, .g = 251, .b = 152, .opacity = 0.45};

  // 卡片/面板背景顏色
  Leaf.cardBackgroundColor = {
      .r = 193, .g = 255, .b = 193, .opacity = 0.55};

  // 舊版透明度設定
  Leaf.panelOpacity = 0.9;
  Leaf.navPaneOpacity = 0.92;

  // 強調色
  Leaf.accentColorHex = L"#98FB98";


  // 將 StarrySky 註冊到 Slot 1
  themeService.SetCustomThemeConfig(StarrySky, 1);

  // 將 Black Souls Leaf 註冊到 Slot 2
  themeService.SetCustomThemeConfig(Leaf, 2);
}

void ApplyDefaultDarkTheme() {
  ThemeService::Instance().SetTheme(ThemeType::Custom1);
}

void ApplyDefaultLightTheme() {
  ThemeService::Instance().SetTheme(ThemeType::Custom2);
}

} // namespace EtherealScepter::Views::Theme
