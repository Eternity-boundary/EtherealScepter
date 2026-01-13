//Created by: EternityBoundary on Jan 7, 2026
#include "pch.h"
#include "AddPortRuleDialog.xaml.h"
#include "Views.Dialogs.AddPortRuleDialog.g.cpp"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::EtherealScepter::Views::Dialogs::implementation
{
    static uint16_t ClampPort(double v)
    {
		constexpr double MAX_PORT = 65535.0;
		constexpr double MIN_PORT = 1.0;
        if (!(v >= MIN_PORT)) return 0;     // NaN 或 < 1
        if (v > MAX_PORT) return 65535;
        return static_cast<uint16_t>(v);
    }

    AddPortRuleDialog::AddPortRuleDialog()
    {
        InitializeComponent();
    }

    hstring AddPortRuleDialog::Description()
    {
        return DescriptionBox().Text();
    }

    uint16_t AddPortRuleDialog::ExternalPort()
    {
        return ClampPort(ExternalPortBox().Value());
    }

    uint16_t AddPortRuleDialog::InternalPort()
    {
        return ClampPort(InternalPortBox().Value());
    }

    hstring AddPortRuleDialog::Protocol()
    {
        // 0=TCP, 1=UDP
        return (ProtocolBox().SelectedIndex() == 1) ? L"UDP" : L"TCP";
    }

    hstring AddPortRuleDialog::InternalClient()
    {
        return InternalClientBox().Text();
    }
}
