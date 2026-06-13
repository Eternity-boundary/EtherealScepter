//Created by: EternityBoundary on Jan 3, 2026
#pragma once
#include "MainWindow.g.h"
#include "include/Services/ThemeService.h"
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Windowing.h>

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
        void InitializeWindowSizePersistence();
        void OnWindowChanged(
            Microsoft::UI::Windowing::AppWindow const& sender,
            Microsoft::UI::Windowing::AppWindowChangedEventArgs const& args);
        void SaveCurrentWindowSize();
        void OnThemeChanged(::EtherealScepter::Services::ThemeType theme);
        void ApplyThemeVisuals();
        void UpdateNavPaneBackgroundWidth(bool isPaneOpen);

        uint32_t m_themeSubscriptionId = 0;
        Microsoft::UI::Windowing::AppWindow m_appWindow{ nullptr };
        winrt::event_token m_windowChangedToken{};
        bool m_hasWindowChangedSubscription = false;
        Microsoft::UI::Dispatching::DispatcherQueueTimer m_saveWindowSizeTimer{ nullptr };
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
