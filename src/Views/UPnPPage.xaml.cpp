//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "UPnPPage.xaml.h"
#include "UPnPPage.g.cpp"
#include "../App/App.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::EtherealScepter::implementation
{
    UPnPPage::UPnPPage()
    {
        InitializeComponent();

        auto dashboardVm = ::winrt::EtherealScepter::implementation::App::Current().DashboardViewModel();
        m_viewModel = winrt::EtherealScepter::ViewModels::UpnpViewModel{ dashboardVm };
    }

    winrt::EtherealScepter::ViewModels::UpnpViewModel UPnPPage::ViewModel() noexcept
    {
        return m_viewModel;
    }

    winrt::Microsoft::UI::Xaml::Visibility UPnPPage::BoolToVisibility(bool value) noexcept
    {
        return value ? Visibility::Visible : Visibility::Collapsed;
    }
}
