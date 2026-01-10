#pragma once

#include "App.xaml.g.h"
#include <winrt/EtherealScepter.ViewModels.h>

namespace winrt::EtherealScepter::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

        static App& Current()
        {
            WINRT_ASSERT(s_current);
            return *s_current;
        }

        winrt::EtherealScepter::ViewModels::DashboardViewModel DashboardViewModel() const
        {
            return m_dashboardVm;
        }

    private:
        winrt::Microsoft::UI::Xaml::Window window{ nullptr };

        winrt::EtherealScepter::ViewModels::DashboardViewModel m_dashboardVm{ nullptr };

        inline static App* s_current = nullptr;
    };
}
