//Created by: EternityBoundary on Jan 3, 2026
#include "pch.h"
#include "UPnPPage.xaml.h"
#include "UPnPPage.g.cpp"
#include "../App/App.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
namespace Services = ::EtherealScepter::Services;

namespace winrt::EtherealScepter::implementation
{
    UPnPPage::UPnPPage()
    {
        InitializeComponent();

        auto dashboardVm = ::winrt::EtherealScepter::implementation::App::Current().DashboardViewModel();
        m_viewModel = winrt::EtherealScepter::ViewModels::UpnpViewModel{ dashboardVm };

        // 訂閱主題變更
        m_themeSubscriptionId = Services::ThemeService::Instance().Subscribe(
            [weak_this = get_weak()](Services::ThemeType theme) {
                if (auto strong_this = weak_this.get()) {
                    strong_this->OnThemeChanged(theme);
                }
            });
    }

    UPnPPage::~UPnPPage()
    {
        if (m_themeSubscriptionId != 0) {
            Services::ThemeService::Instance().Unsubscribe(m_themeSubscriptionId);
            m_themeSubscriptionId = 0;
        }
    }

    winrt::EtherealScepter::ViewModels::UpnpViewModel UPnPPage::ViewModel() noexcept
    {
        return m_viewModel;
    }

    winrt::Microsoft::UI::Xaml::Visibility UPnPPage::BoolToVisibility(bool value) noexcept
    {
        return value ? Visibility::Visible : Visibility::Collapsed;
    }

    void UPnPPage::OnPageLoaded(IInspectable const& /*sender*/, RoutedEventArgs const& /*args*/)
    {
        ApplyThemeToCards();
    }

    void UPnPPage::OnDeviceCardLoaded(IInspectable const& sender, RoutedEventArgs const& /*args*/)
    {
        // 當 Device 卡片載入時，套用主題
        if (auto border = sender.try_as<Border>()) {
            ApplyThemeToDeviceCard(border);
        }
    }

    void UPnPPage::OnThemeChanged(Services::ThemeType /*theme*/)
    {
        ApplyThemeToCards();
    }

    void UPnPPage::ApplyThemeToDeviceCard(Border const& border)
    {
        auto& themeService = Services::ThemeService::Instance();
        bool isCustomTheme = themeService.IsCustomThemeActive();

        if (isCustomTheme) {
            auto brush = themeService.GetCardBackgroundBrush();
            if (brush) {
                border.Background(brush);
            }
        } else {
            // 恢復為系統預設
            border.Background(nullptr);
        }
    }

    void UPnPPage::ApplyThemeToCards()
    {
        auto& themeService = Services::ThemeService::Instance();
        bool isCustomTheme = themeService.IsCustomThemeActive();

        // 套用到 Status Card
        auto statusCard = StatusCard();
        if (statusCard) {
            if (isCustomTheme) {
                auto brush = themeService.GetCardBackgroundBrush();
                if (brush) {
                    statusCard.Background(brush);
                }
            } else {
                statusCard.Background(nullptr);
            }
        }

        // 套用到所有 Device Cards
        auto devicesControl = DevicesItemsControl();
        if (devicesControl) {
            // 遍歷 ItemsControl 的所有項目容器
            auto itemCount = devicesControl.Items().Size();
            for (uint32_t i = 0; i < itemCount; ++i) {
                auto container = devicesControl.ContainerFromIndex(i);
                if (container) {
                    // 在 ItemsControl 中，容器通常是 ContentPresenter
                    if (auto contentPresenter = container.try_as<ContentPresenter>()) {
                        // 嘗試獲取 ContentPresenter 的內容（Border）
                        if (auto border = contentPresenter.Content().try_as<Border>()) {
                            ApplyThemeToDeviceCard(border);
                        }
                    }
                    // 也嘗試直接作為 Border
                    else if (auto border = container.try_as<Border>()) {
                        ApplyThemeToDeviceCard(border);
                    }
                }
            }
        }
    }
}
