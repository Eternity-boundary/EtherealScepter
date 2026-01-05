//Created by: EternityBoundary on Jan 3, 2025
#pragma once

#include "NetworkPage.g.h"

namespace winrt::EtherealScepter::implementation
{
    struct NetworkPage : NetworkPageT<NetworkPage>
    {
        NetworkPage()
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
    struct NetworkPage : NetworkPageT<NetworkPage, implementation::NetworkPage>
    {
    };
}
