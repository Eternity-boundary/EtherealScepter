//Created by: EternityBoundary on Jan 3, 2026
#pragma once
#include "DashboardPage.g.h"
#include "include/ViewModels/DashboardViewModel.h"
#include "include/Services/ThemeService.h"

namespace winrt::EtherealScepter::implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage>
    {
        DashboardPage();
        ~DashboardPage();

        winrt::EtherealScepter::ViewModels::DashboardViewModel ViewModel() noexcept;

        void OnPageLoaded(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

    private:
        void OnThemeChanged(::EtherealScepter::Services::ThemeType theme);
        void ApplyThemeToCards();

        winrt::EtherealScepter::ViewModels::DashboardViewModel m_viewModel{ nullptr };
        uint32_t m_themeSubscriptionId{ 0 };
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage, implementation::DashboardPage> {};
}
