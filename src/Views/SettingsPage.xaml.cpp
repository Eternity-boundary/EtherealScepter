// Created by: EternityBoundary
#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "include/ViewModels/SettingsPageViewModel.h"

#include <format>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
namespace Services = ::EtherealScepter::Services;

namespace winrt::EtherealScepter::implementation {

SettingsPage::SettingsPage() {
  InitializeComponent();
  m_viewModel = winrt::make<ViewModels::implementation::SettingsPageViewModel>();

  // 訂閱主題變更
  m_themeSubscriptionId = Services::ThemeService::Instance().Subscribe(
      [weak_this = get_weak()](Services::ThemeType theme) {
        if (auto strong_this = weak_this.get()) {
          strong_this->OnThemeChanged(theme);
        }
      });
}

SettingsPage::~SettingsPage() {
  if (m_themeSubscriptionId != 0) {
    Services::ThemeService::Instance().Unsubscribe(m_themeSubscriptionId);
    m_themeSubscriptionId = 0;
  }
}

winrt::EtherealScepter::ViewModels::SettingsPageViewModel
SettingsPage::ViewModel() noexcept {
  return m_viewModel;
}

void SettingsPage::OnPageLoaded(IInspectable const& /*sender*/, RoutedEventArgs const& /*args*/) {
  ApplyThemeToCards();
}

void SettingsPage::OnThemeChanged(Services::ThemeType /*theme*/) {
  ApplyThemeToCards();
}

void SettingsPage::ApplyThemeToCards() {
  auto& themeService = Services::ThemeService::Instance();
  bool isCustomTheme = themeService.IsCustomThemeActive();

  auto themeCard = ThemeCard();
  auto customThemeCard = CustomThemeCard();
  auto aboutCard = AboutCard();

  if (isCustomTheme) {
    auto brush = themeService.GetCardBackgroundBrush();
    if (brush) {
      if (themeCard) themeCard.Background(brush);
      if (customThemeCard) customThemeCard.Background(brush);
      if (aboutCard) aboutCard.Background(brush);
    }
  } else {
    if (themeCard) themeCard.Background(nullptr);
    if (customThemeCard) customThemeCard.Background(nullptr);
    if (aboutCard) aboutCard.Background(nullptr);
  }
}

hstring SettingsPage::FormatPercent(double value) {
  int percent = static_cast<int>(value * 100);
  return to_hstring(percent) + L"%";
}

void SettingsPage::OnPickBackgroundImageClicked(IInspectable const &,
                                                 RoutedEventArgs const &) {
  if (m_viewModel) {
    m_viewModel.PickBackgroundImageAsync();
  }
}

void SettingsPage::OnPickNavPaneBackgroundImageClicked(IInspectable const &,
                                                        RoutedEventArgs const &) {
  if (m_viewModel) {
    m_viewModel.PickNavPaneBackgroundImageAsync();
  }
}

void SettingsPage::OnClearBackgroundImageClicked(IInspectable const &,
                                                  RoutedEventArgs const &) {
  if (m_viewModel) {
    m_viewModel.ClearBackgroundImage();
  }
}

void SettingsPage::OnClearNavPaneBackgroundImageClicked(IInspectable const &,
                                                         RoutedEventArgs const &) {
  if (m_viewModel) {
    m_viewModel.ClearNavPaneBackgroundImage();
  }
}

void SettingsPage::OnSaveSettingsClicked(IInspectable const &,
                                          RoutedEventArgs const &) {
  if (m_viewModel) {
    m_viewModel.SaveSettingsAsync();
  }
}

} // namespace winrt::EtherealScepter::implementation
