//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "UPnPPage.xaml.h"
#if __has_include("UPnPPage.g.cpp")
#include "UPnPPage.g.cpp"
#endif

#include "App.xaml.h"

#include "include/ViewModels/UpnpViewModel.h" 

namespace winrt::EtherealScepter::implementation
{
    UPnPPage::UPnPPage()
    {
        InitializeComponent();

        auto dashboardVm = App::Current().DashboardViewModel();
        m_viewModel =
            winrt::make<winrt::EtherealScepter::ViewModels::implementation::UpnpViewModel>(dashboardVm);
    }

    winrt::EtherealScepter::ViewModels::UpnpViewModel UPnPPage::ViewModel() noexcept
    {
        return m_viewModel;
    }

    winrt::Microsoft::UI::Xaml::Visibility UPnPPage::BoolToVisibility(bool value) noexcept
    {
        return value
            ? winrt::Microsoft::UI::Xaml::Visibility::Visible
            : winrt::Microsoft::UI::Xaml::Visibility::Collapsed;
    }

}
