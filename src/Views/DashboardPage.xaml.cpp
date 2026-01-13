//Created by: EternityBoundary on Jan 3, 2026
#include "pch.h"
#include "DashboardPage.xaml.h"
#if __has_include("DashboardPage.g.cpp")
#include "DashboardPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
namespace Services = ::EtherealScepter::Services;

namespace winrt::EtherealScepter::implementation
{
    DashboardPage::DashboardPage()
    {
        InitializeComponent();

        m_viewModel =
            winrt::make<winrt::EtherealScepter::ViewModels::implementation::DashboardViewModel>();

        // 訂閱主題變更
        m_themeSubscriptionId = Services::ThemeService::Instance().Subscribe(
            [weak_this = get_weak()](Services::ThemeType theme) {
                if (auto strong_this = weak_this.get()) {
                    strong_this->OnThemeChanged(theme);
                }
            });
    }

    DashboardPage::~DashboardPage()
    {
        if (m_themeSubscriptionId != 0) {
            Services::ThemeService::Instance().Unsubscribe(m_themeSubscriptionId);
            m_themeSubscriptionId = 0;
        }
    }

    winrt::EtherealScepter::ViewModels::DashboardViewModel DashboardPage::ViewModel() noexcept
    {
        return m_viewModel;
    }

    void DashboardPage::OnPageLoaded(IInspectable const& /*sender*/, RoutedEventArgs const& /*args*/)
    {
        // 頁面載入時套用主題
        ApplyThemeToCards();
    }

    void DashboardPage::OnThemeChanged(Services::ThemeType /*theme*/)
    {
        ApplyThemeToCards();
    }

    void DashboardPage::ApplyThemeToCards()
    {
        auto& themeService = Services::ThemeService::Instance();
        bool isCustomTheme = themeService.IsCustomThemeActive();

        // 獲取所有卡片元素並設定背景
        auto networkCard = NetworkCard();
        auto upnpCard = UpnpCard();
        auto natCard = NatCard();
        auto summaryCard = SummaryCard();

        if (isCustomTheme) {
            auto brush = themeService.GetCardBackgroundBrush();
            if (brush) {
                if (networkCard) networkCard.Background(brush);
                if (upnpCard) upnpCard.Background(brush);
                if (natCard) natCard.Background(brush);
                if (summaryCard) summaryCard.Background(brush);
            }
        } else {
            // 恢復為系統預設（設為 nullptr 讓系統使用 ThemeResource）
            if (networkCard) networkCard.Background(nullptr);
            if (upnpCard) upnpCard.Background(nullptr);
            if (natCard) natCard.Background(nullptr);
            if (summaryCard) summaryCard.Background(nullptr);
        }
    }
}
