//Created by: EternityBoundary on Jan 3, 2026
#pragma once

#include "UPnPPage.g.h"
#include <winrt/EtherealScepter.ViewModels.h>
#include "include/Services/ThemeService.h"

namespace winrt::EtherealScepter::implementation
{
    struct UPnPPage : UPnPPageT<UPnPPage>
    {
        UPnPPage();
        ~UPnPPage();

        winrt::EtherealScepter::ViewModels::UpnpViewModel ViewModel() noexcept;
        winrt::Microsoft::UI::Xaml::Visibility BoolToVisibility(bool value) noexcept;

        // Event handlers
        void OnPageLoaded(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

        void OnDeviceCardLoaded(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

    private:
        void OnThemeChanged(::EtherealScepter::Services::ThemeType theme);
        void ApplyThemeToCards();
        void ApplyThemeToDeviceCard(winrt::Microsoft::UI::Xaml::Controls::Border const& border);

        winrt::EtherealScepter::ViewModels::UpnpViewModel m_viewModel{ nullptr };
        uint32_t m_themeSubscriptionId{ 0 };
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct UPnPPage : UPnPPageT<UPnPPage, implementation::UPnPPage> {};
}
