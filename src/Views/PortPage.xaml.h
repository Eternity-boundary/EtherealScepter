//Created by: EternityBoundary on Jan 3, 2026
#pragma once

#include "PortPage.g.h"
#include "include/ViewModels/PortPageViewModel.h"
#include "include/Services/ThemeService.h"

namespace winrt::EtherealScepter::implementation
{
    struct PortPage : PortPageT<PortPage>
    {
        PortPage();
        ~PortPage();

        EtherealScepter::ViewModels::PortPageViewModel ViewModel() const;

        winrt::fire_and_forget OnAddRuleClicked(
            winrt::Windows::Foundation::IInspectable const& sender,
            Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void OnDeleteClicked(
            winrt::Windows::Foundation::IInspectable const& sender,
            Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void OnPageLoaded(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

    private:
        void OnThemeChanged(::EtherealScepter::Services::ThemeType theme);
        void ApplyThemeToCards();

        EtherealScepter::ViewModels::PortPageViewModel m_viewModel{ nullptr };
        uint32_t m_themeSubscriptionId{ 0 };
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct PortPage : PortPageT<PortPage, implementation::PortPage> {};
}
