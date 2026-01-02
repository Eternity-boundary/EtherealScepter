//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "DashboardPage.xaml.h"
#if __has_include("DashboardPage.g.cpp")
#include "DashboardPage.g.cpp"
#endif


using namespace winrt;

namespace winrt::EtherealScepter::implementation
{
    DashboardPage::DashboardPage()
    {
        InitializeComponent();
        m_viewModel = winrt::make_self<::EtherealScepter::ViewModels::DashboardViewModel>();
        m_viewModel->Refresh();
    }

    ::EtherealScepter::ViewModels::DashboardViewModel& DashboardPage::ViewModel() noexcept
    {
        return *m_viewModel;
    }
}