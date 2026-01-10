//Created by: EternityBoundary on Jan 7, 2025
#pragma once

#include "Views.Dialogs.AddPortRuleDialog.g.h"

namespace winrt::EtherealScepter::Views::Dialogs::implementation
{
    struct AddPortRuleDialog : AddPortRuleDialogT<AddPortRuleDialog>
    {
        AddPortRuleDialog();

        winrt::hstring Description();
        uint16_t ExternalPort();
        uint16_t InternalPort();
        winrt::hstring Protocol();
        winrt::hstring InternalClient();
    };
}

namespace winrt::EtherealScepter::Views::Dialogs::factory_implementation
{
    struct AddPortRuleDialog
        : AddPortRuleDialogT<AddPortRuleDialog, implementation::AddPortRuleDialog>
    {
    };
}
