//Created by: EternityBoundary on Jan 3, 2026
#include "pch.h"
#include "PortPage.xaml.h"
#include "PortPage.g.cpp"

#include <winrt/EtherealScepter.Models.h>
#include <winrt/EtherealScepter.Views.Dialogs.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
namespace Services = ::EtherealScepter::Services;

namespace winrt::EtherealScepter::implementation
{
    PortPage::PortPage()
    {
        InitializeComponent();
        m_viewModel = winrt::EtherealScepter::ViewModels::PortPageViewModel{};

        // 訂閱主題變更
        m_themeSubscriptionId = Services::ThemeService::Instance().Subscribe(
            [weak_this = get_weak()](Services::ThemeType theme) {
                if (auto strong_this = weak_this.get()) {
                    strong_this->OnThemeChanged(theme);
                }
            });
    }

    PortPage::~PortPage()
    {
        if (m_themeSubscriptionId != 0) {
            Services::ThemeService::Instance().Unsubscribe(m_themeSubscriptionId);
            m_themeSubscriptionId = 0;
        }
    }

    EtherealScepter::ViewModels::PortPageViewModel PortPage::ViewModel() const
    {
        return m_viewModel;
    }

    void PortPage::OnPageLoaded(IInspectable const& /*sender*/, RoutedEventArgs const& /*args*/)
    {
        ApplyThemeToCards();
    }

    void PortPage::OnThemeChanged(Services::ThemeType /*theme*/)
    {
        ApplyThemeToCards();
    }

    void PortPage::ApplyThemeToCards()
    {
        auto& themeService = Services::ThemeService::Instance();
        bool isCustomTheme = themeService.IsCustomThemeActive();

        auto tableCard = TableCard();

        if (isCustomTheme) {
            auto brush = themeService.GetCardBackgroundBrush();
            if (brush) {
                if (tableCard) tableCard.Background(brush);
            }
        } else {
            if (tableCard) tableCard.Background(nullptr);
        }
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

        hstring errorMessage;
        try {
            m_viewModel.AddMapping(m);
        } catch (winrt::hresult_error const& ex) {
            errorMessage = ex.message();
        }

        if (!errorMessage.empty()) {
            ContentDialog errorDialog;
            errorDialog.XamlRoot(this->XamlRoot());
            errorDialog.Title(box_value(L"Error"));
            errorDialog.Content(box_value(errorMessage));
            errorDialog.CloseButtonText(L"OK");
            co_await errorDialog.ShowAsync();
        }
    }

    winrt::fire_and_forget PortPage::OnDeleteClicked(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto lifetime = get_strong();

        auto fe = sender.try_as<FrameworkElement>();
        if (!fe) co_return;

        auto dc = fe.DataContext();
        auto m = winrt::unbox_value<EtherealScepter::Models::PortMappingInfo>(dc);

        hstring errorMessage;
        try {
            m_viewModel.RemoveMapping(m);
        } catch (winrt::hresult_error const& ex) {
            errorMessage = ex.message();
        }

        if (!errorMessage.empty()) {
            ContentDialog errorDialog;
            errorDialog.XamlRoot(this->XamlRoot());
            errorDialog.Title(box_value(L"Error"));
            errorDialog.Content(box_value(errorMessage));
            errorDialog.CloseButtonText(L"OK");
            co_await errorDialog.ShowAsync();
        }
    }
}
