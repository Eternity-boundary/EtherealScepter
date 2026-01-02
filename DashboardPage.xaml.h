//Created by: EternityBoundary on Jan 3, 2025
#pragma once

#pragma once
#include "DashboardPage.g.h"
#include "include/ViewModels/DashboardViewModel.h"

namespace winrt::EtherealScepter::implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage>
    {
        DashboardPage();

        ::EtherealScepter::ViewModels::DashboardViewModel& ViewModel() noexcept;

    private:
        winrt::com_ptr<::EtherealScepter::ViewModels::DashboardViewModel> m_viewModel;
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage, implementation::DashboardPage> {};
}
