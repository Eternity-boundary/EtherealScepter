//Created by: EternityBoundary on Jan 3, 2025
#pragma once

#include "UPnPPage.g.h"
#include <winrt/EtherealScepter.ViewModels.h>

namespace winrt::EtherealScepter::implementation::XAML {
    struct UPnPPage : UPnPPageT<UPnPPage>
    {
        UPnPPage();

        winrt::EtherealScepter::ViewModels::UpnpViewModel ViewModel() noexcept;
        winrt::Microsoft::UI::Xaml::Visibility BoolToVisibility(bool value) noexcept;


    private:
        winrt::EtherealScepter::ViewModels::UpnpViewModel m_viewModel{ nullptr };
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct UPnPPage : UPnPPageT<UPnPPage, implementation::UPnPPage> {};
}
