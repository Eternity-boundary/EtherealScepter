//Created by: EternityBoundary on Jan 3, 2026
#pragma once
#include "MainWindow.g.h"
#include "include/Services/ThemeService.h"

namespace winrt::EtherealScepter::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        ~MainWindow();

        void OnNavigationChanged(
            Microsoft::UI::Xaml::Controls::NavigationView const& sender,
            Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);

        void OnPaneOpening(
            Microsoft::UI::Xaml::Controls::NavigationView const& sender,
            Windows::Foundation::IInspectable const& args);

        void OnPaneClosing(
            Microsoft::UI::Xaml::Controls::NavigationView const& sender,
            Microsoft::UI::Xaml::Controls::NavigationViewPaneClosingEventArgs const& args);

    private:
        void InitializeTheme();
        void OnThemeChanged(::EtherealScepter::Services::ThemeType theme);
        void ApplyThemeVisuals();
        void UpdateNavPaneBackgroundWidth(bool isPaneOpen);

        uint32_t m_themeSubscriptionId = 0;
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
