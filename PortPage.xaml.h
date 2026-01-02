//Created by: EternityBoundary on Jan 3, 2025
#pragma once

#include "PortPage.g.h"

namespace winrt::EtherealScepter::implementation
{
    struct PortPage : PortPageT<PortPage>
    {
        PortPage()
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
    struct PortPage : PortPageT<PortPage, implementation::PortPage>
    {
    };
}
