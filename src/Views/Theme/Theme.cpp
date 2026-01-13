// Created by: EternityBoundary on Jan 12, 2025
#include "pch.h"
#include "include/Services/ThemeService.h"

using namespace EtherealScepter::Services;

namespace EtherealScepter::Views::Theme {

/// <summary>
/// 註冊預設的自定義主題
/// </summary>
void RegisterDefaultCustomThemes() {
    auto& themeService = ThemeService::Instance();

    // ========== Custom Theme 1: 深色磨砂玻璃 ==========
    CustomThemeConfig theme1;
    theme1.themeName = L"Dark Frosted Glass";
    theme1.baseTheme = winrt::Microsoft::UI::Xaml::ElementTheme::Dark;
    
    // 背景圖片（空 = 不使用）
    theme1.backgroundImagePath = L"";
    theme1.navPaneBackgroundImagePath = L"";
    
    // 側邊欄背景：深灰色，75% 透明度
    theme1.navPaneBackgroundColor = {25, 25, 25, 0.75};
    
    // 導航按鈕 - 正常狀態：完全透明
    theme1.navButtonNormalColor = {0, 0, 0, 0.0};
    
    // 導航按鈕 - 懸停狀態：白色微光
    theme1.navButtonHoverColor = {255, 255, 255, 0.1};
    
    // 導航按鈕 - 選中狀態：藍色強調
    theme1.navButtonSelectedColor = {0, 120, 215, 0.35};
    
    // 卡片背景：深灰色，70% 透明度（磨砂玻璃效果）
    theme1.cardBackgroundColor = {40, 40, 40, 0.7};
    
    // 舊版透明度
    theme1.panelOpacity = 0.75;
    theme1.navPaneOpacity = 0.75;
    
    // 註冊到 Slot 1
    themeService.SetCustomThemeConfig(theme1, 1);

    // ========== Custom Theme 2: 淺色清新 ==========
    CustomThemeConfig theme2;
    theme2.themeName = L"Light Breeze";
    theme2.baseTheme = winrt::Microsoft::UI::Xaml::ElementTheme::Light;
    
    // 背景圖片（空 = 不使用）
    theme2.backgroundImagePath = L"";
    theme2.navPaneBackgroundImagePath = L"";
    
    // 側邊欄背景：淺灰色，85% 透明度
    theme2.navPaneBackgroundColor = {245, 245, 245, 0.85};
    
    // 導航按鈕 - 正常狀態：完全透明
    theme2.navButtonNormalColor = {0, 0, 0, 0.0};
    
    // 導航按鈕 - 懸停狀態：黑色微光
    theme2.navButtonHoverColor = {0, 0, 0, 0.08};
    
    // 導航按鈕 - 選中狀態：藍色強調
    theme2.navButtonSelectedColor = {0, 120, 215, 0.2};
    
    // 卡片背景：白色，90% 透明度
    theme2.cardBackgroundColor = {255, 255, 255, 0.9};
    
    // 舊版透明度
    theme2.panelOpacity = 0.9;
    theme2.navPaneOpacity = 0.85;
    
    // 註冊到 Slot 2
    themeService.SetCustomThemeConfig(theme2, 2);
}

/// <summary>
/// 切換到自定義主題 1
/// </summary>
void ApplyCustomTheme1() {
    ThemeService::Instance().SetTheme(ThemeType::Custom1);
}

/// <summary>
/// 切換到自定義主題 2
/// </summary>
void ApplyCustomTheme2() {
    ThemeService::Instance().SetTheme(ThemeType::Custom2);
}

} // namespace EtherealScepter::Views::Theme