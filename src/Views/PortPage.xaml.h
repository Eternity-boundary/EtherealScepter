//Created by: EternityBoundary on Jan 3, 2025
#pragma once

#include "PortPage.g.h"
#include "include/ViewModels/PortPageViewModel.h"

namespace winrt::EtherealScepter::implementation
{
    struct PortPage : PortPageT<PortPage>
    {
        PortPage();

        EtherealScepter::ViewModels::PortPageViewModel ViewModel() const;

        winrt::fire_and_forget OnAddRuleClicked(
            winrt::Windows::Foundation::IInspectable const& sender,
            Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void OnDeleteClicked(
            winrt::Windows::Foundation::IInspectable const& sender,
            Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        EtherealScepter::ViewModels::PortPageViewModel m_viewModel{ nullptr };
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct PortPage : PortPageT<PortPage, implementation::PortPage> {};
}
