//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "DashboardPage.xaml.h"
#if __has_include("DashboardPage.g.cpp")
#include "DashboardPage.g.cpp"
#endif


namespace winrt::EtherealScepter::implementation::XAML {
    DashboardPage::DashboardPage()
    {
        InitializeComponent();

        m_viewModel =
            winrt::make<winrt::EtherealScepter::ViewModels::implementation::DashboardViewModel>();
    }

    winrt::EtherealScepter::ViewModels::DashboardViewModel DashboardPage::ViewModel() noexcept
    {
        return m_viewModel;
    }
}