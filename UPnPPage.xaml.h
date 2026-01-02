#pragma once

#include "UPnPPage.g.h"

namespace winrt::EtherealScepter::implementation
{
    struct UPnPPage : UPnPPageT<UPnPPage>
    {
        UPnPPage()
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
    struct UPnPPage : UPnPPageT<UPnPPage, implementation::UPnPPage>
    {
    };
}
