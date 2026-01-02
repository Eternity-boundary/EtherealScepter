#pragma once

#include "DashboardPage.g.h"

namespace winrt::EtherealScepter::implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage>
    {
        DashboardPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::EtherealScepter::factory_implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage, implementation::DashboardPage>
    {
    };
}
