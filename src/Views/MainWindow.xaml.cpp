// Created by: EternityBoundary on Jan 3, 2026
#include "pch.h"

#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <microsoft.ui.xaml.window.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/base.h>

#include "DashboardPage.xaml.h"
#include "NetworkPage.xaml.h"
#include "PortPage.xaml.h"
#include "SettingsPage.xaml.h"
#include "UPnPPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Interop;
using namespace winrt::Microsoft::UI::Windowing;
namespace Services = ::EtherealScepter::Services;

static HWND GetHwnd(winrt::Microsoft::UI::Xaml::Window const &window) {
  HWND hwnd{};
  auto native = window.as<IWindowNative>();
  check_hresult(native->get_WindowHandle(&hwnd));
  return hwnd;
}

void SetTitleBarIcon(winrt::Microsoft::UI::Xaml::Window const &window) {
  HWND hwnd = GetHwnd(window);

  auto windowId = Microsoft::UI::GetWindowIdFromWindow(hwnd);
  auto appWindow =
      Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

  auto root =
      Windows::ApplicationModel::Package::Current().InstalledLocation().Path();
  winrt::hstring iconPath = root + L"\\Assets\\flash.ico";

  appWindow.SetIcon(iconPath);
}

static void SetFixedWindowSize(winrt::Microsoft::UI::Xaml::Window const &window,
                               int width, int height) {
  HWND hwnd = GetHwnd(window);

  auto windowId = Microsoft::UI::GetWindowIdFromWindow(hwnd);
  auto appWindow =
      Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

  // 設置固定視窗大小
  Windows::Graphics::SizeInt32 size{width, height};
  appWindow.Resize(size);

  // 獲取 OverlappedPresenter 並禁用調整大小和最大化
  auto presenter =
      appWindow.Presenter().as<Microsoft::UI::Windowing::OverlappedPresenter>();
  presenter.IsResizable(false);
  presenter.IsMaximizable(false);
}

namespace winrt::EtherealScepter::implementation {

MainWindow::MainWindow() {
  InitializeComponent();
  SetTitleBarIcon(*this);
  SetFixedWindowSize(*this, 1640, 900);

  // Initialize theme service
  InitializeTheme();

  // 預設顯示 Dashboard
  ContentFrame().Navigate(xaml_typename<EtherealScepter::DashboardPage>());
}

MainWindow::~MainWindow() {
  if (m_themeSubscriptionId != 0) {
    Services::ThemeService::Instance().Unsubscribe(m_themeSubscriptionId);
    m_themeSubscriptionId = 0;
  }
}

void MainWindow::InitializeTheme() {
  auto &themeService = Services::ThemeService::Instance();

  // Initialize with root element
  auto rootGrid = RootGrid();
  if (rootGrid) {
    themeService.Initialize(rootGrid);
  }

  // Subscribe to theme changes
  m_themeSubscriptionId = themeService.Subscribe(
      [weak_this = get_weak()](Services::ThemeType theme) {
        if (auto strong_this = weak_this.get()) {
          strong_this->OnThemeChanged(theme);
        }
      });

  // Load saved settings and apply
  themeService.LoadSettingsAsync();
  ApplyThemeVisuals();
}

void MainWindow::OnThemeChanged(Services::ThemeType /*theme*/) {
  ApplyThemeVisuals();
}

void MainWindow::UpdateNavPaneBackgroundWidth(bool isPaneOpen) {
  auto navPaneBorder = NavPaneBackgroundBorder();
  if (navPaneBorder) {
    // 根據 Pane 狀態設定寬度：展開時 320px，收合時 48px
    navPaneBorder.Width(isPaneOpen ? 320.0 : 48.0);
  }
}

void MainWindow::OnPaneOpening(NavigationView const& /*sender*/,
                                Windows::Foundation::IInspectable const& /*args*/) {
  UpdateNavPaneBackgroundWidth(true);
}

void MainWindow::OnPaneClosing(NavigationView const& /*sender*/,
                                NavigationViewPaneClosingEventArgs const& /*args*/) {
  UpdateNavPaneBackgroundWidth(false);
}

void MainWindow::ApplyThemeVisuals() {
  auto &themeService = Services::ThemeService::Instance();
  auto currentTheme = themeService.GetCurrentTheme();
  bool isCustomTheme = themeService.IsCustomThemeActive();

  // Determine the ElementTheme to apply
  ElementTheme elementTheme = ElementTheme::Default;
  switch (currentTheme) {
  case Services::ThemeType::System:
    elementTheme = ElementTheme::Default;
    break;
  case Services::ThemeType::Light:
    elementTheme = ElementTheme::Light;
    break;
  case Services::ThemeType::Dark:
    elementTheme = ElementTheme::Dark;
    break;
  case Services::ThemeType::Custom1:
  case Services::ThemeType::Custom2:
    elementTheme = themeService.GetActiveCustomThemeConfig().baseTheme;
    break;
  }

  // Apply theme to RootGrid
  auto rootGrid = RootGrid();
  if (rootGrid) {
    rootGrid.RequestedTheme(elementTheme);
  }

  // Apply theme to NavigationView explicitly
  auto navView = NavView();
  if (navView) {
    navView.RequestedTheme(elementTheme);
  }

  // Apply navigation pane background (側邊欄背景)
  auto navPaneBorder = NavPaneBackgroundBorder();
  if (navPaneBorder) {
    if (isCustomTheme) {
      // 優先使用圖片背景
      auto imageBrush = themeService.GetNavPaneBackgroundBrush();
      if (imageBrush) {
        navPaneBorder.Background(imageBrush);
        navPaneBorder.Visibility(Visibility::Visible);
      } else {
        // 沒有圖片時使用顏色
        auto colorBrush = themeService.GetNavPaneColorBrush();
        if (colorBrush) {
          navPaneBorder.Background(colorBrush);
          navPaneBorder.Visibility(Visibility::Visible);
        } else {
          navPaneBorder.Background(nullptr);
          navPaneBorder.Visibility(Visibility::Collapsed);
        }
      }
      // 根據當前 Pane 狀態設定寬度
      if (navView) {
        UpdateNavPaneBackgroundWidth(navView.IsPaneOpen());
      }
    } else {
      navPaneBorder.Background(nullptr);
      navPaneBorder.Visibility(Visibility::Collapsed);
    }
  }

  // Apply content area background image (右側內容區背景)
  auto backgroundBorder = BackgroundImageBorder();
  if (backgroundBorder) {
    if (isCustomTheme) {
      auto brush = themeService.GetBackgroundBrush();
      if (brush) {
        backgroundBorder.Background(brush);
        backgroundBorder.Visibility(Visibility::Visible);
      } else {
        backgroundBorder.Background(nullptr);
        backgroundBorder.Visibility(Visibility::Collapsed);
      }
    } else {
      backgroundBorder.Background(nullptr);
      backgroundBorder.Visibility(Visibility::Collapsed);
    }
  }
}

void MainWindow::OnNavigationChanged(
    NavigationView const &sender,
    NavigationViewSelectionChangedEventArgs const &args) {

  // Handle Settings navigation
  if (args.IsSettingsSelected()) {
    ContentFrame().Navigate(xaml_typename<EtherealScepter::SettingsPage>());
    return;
  }

void MainWindow::OnNavigationChanged(
    NavigationView const & /*sender*/,
    NavigationViewSelectionChangedEventArgs const &args) {
  if (!args.SelectedItem())
    return;

  auto item = args.SelectedItem().as<NavigationViewItem>();
  auto tag = unbox_value<hstring>(item.Tag());

  if (tag == L"dashboard") {
    ContentFrame().Navigate(xaml_typename<EtherealScepter::DashboardPage>());
  } else if (tag == L"upnp") {
    ContentFrame().Navigate(xaml_typename<EtherealScepter::UPnPPage>());
  } else if (tag == L"ports") {
    ContentFrame().Navigate(xaml_typename<EtherealScepter::PortPage>());
  } else if (tag == L"network") {
    ContentFrame().Navigate(xaml_typename<EtherealScepter::NetworkPage>());
  }
}
} // namespace winrt::EtherealScepter::implementation
