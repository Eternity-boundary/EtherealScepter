//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "PortPage.xaml.h"
#include "PortPage.g.cpp"

#include <winrt/EtherealScepter.Models.h>
#include <winrt/EtherealScepter.Views.Dialogs.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::EtherealScepter::implementation
{
    PortPage::PortPage()
    {
        InitializeComponent();
        m_viewModel = winrt::EtherealScepter::ViewModels::PortPageViewModel{};
    }

    EtherealScepter::ViewModels::PortPageViewModel PortPage::ViewModel() const
    {
        return m_viewModel;
    }

    winrt::fire_and_forget PortPage::OnAddRuleClicked(IInspectable const&, RoutedEventArgs const&)
    {
        auto lifetime = get_strong();

        EtherealScepter::Views::Dialogs::AddPortRuleDialog dialog;
        dialog.XamlRoot(this->XamlRoot());

        auto r = co_await dialog.ShowAsync();
        if (r != ContentDialogResult::Primary)
            co_return;

        EtherealScepter::Models::PortMappingInfo m{
            dialog.Description(),
            dialog.ExternalPort(),
            dialog.InternalPort(),
            dialog.Protocol(),
            dialog.InternalClient(),
        };

        m_viewModel.AddMapping(m);
    }

    void PortPage::OnDeleteClicked(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto fe = sender.try_as<FrameworkElement>();
        if (!fe) return;

        auto dc = fe.DataContext();
        auto m = winrt::unbox_value<EtherealScepter::Models::PortMappingInfo>(dc);
        m_viewModel.RemoveMapping(m);
    }
}
