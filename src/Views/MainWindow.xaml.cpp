// Created by: EternityBoundary on Jan 3, 2026
#include "pch.h"

#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <algorithm>
#include <cmath>

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

static UINT GetWindowDpi(HWND hwnd) {
  UINT dpi = GetDpiForWindow(hwnd);
  return dpi == 0 ? 96 : dpi;
}

static int ScaleDipToPixel(double value, UINT dpi) {
  return std::max(1, static_cast<int>(std::lround(value * dpi / 96.0)));
}

static double ScalePixelToDip(int value, UINT dpi) {
  return static_cast<double>(value) * 96.0 / dpi;
}

static Windows::Graphics::SizeInt32 GetStartupWindowSize(HWND hwnd) {
  constexpr double kPreferredWidthDip = 1200.0;
  constexpr double kPreferredHeightDip = 760.0;
  constexpr double kMinimumStartupWidthDip = 900.0;
  constexpr double kMinimumStartupHeightDip = 560.0;
  constexpr double kMinimumSavedStartupWidthDip = 640.0;
  constexpr double kMinimumSavedStartupHeightDip = 420.0;

  UINT dpi = GetWindowDpi(hwnd);
  double preferredWidthDip = kPreferredWidthDip;
  double preferredHeightDip = kPreferredHeightDip;
  bool hasSavedWindowSize = Services::ThemeService::Instance()
                                .TryGetSavedWindowSize(preferredWidthDip,
                                                       preferredHeightDip);

  int preferredWidth = ScaleDipToPixel(preferredWidthDip, dpi);
  int preferredHeight = ScaleDipToPixel(preferredHeightDip, dpi);

  HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitorInfo{sizeof(MONITORINFO)};
  if (GetMonitorInfo(monitor, &monitorInfo)) {
    int workWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
    int workHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

    // 保留一點桌面邊界，同時避免小螢幕或高 DPI 環境中視窗超出工作區。
    int maxWidth = std::max(1, MulDiv(workWidth, 92, 100));
    int maxHeight = std::max(1, MulDiv(workHeight, 92, 100));
    double minimumWidthDip = hasSavedWindowSize ? kMinimumSavedStartupWidthDip
                                                : kMinimumStartupWidthDip;
    double minimumHeightDip = hasSavedWindowSize ? kMinimumSavedStartupHeightDip
                                                 : kMinimumStartupHeightDip;
    int minWidth = std::min(maxWidth, ScaleDipToPixel(minimumWidthDip, dpi));
    int minHeight = std::min(maxHeight, ScaleDipToPixel(minimumHeightDip, dpi));

    preferredWidth = std::clamp(preferredWidth, minWidth, maxWidth);
    preferredHeight = std::clamp(preferredHeight, minHeight, maxHeight);
  }

  return Windows::Graphics::SizeInt32{preferredWidth, preferredHeight};
}

static void ConfigureStartupWindow(
    winrt::Microsoft::UI::Xaml::Window const &window) {
  HWND hwnd = GetHwnd(window);

  auto windowId = Microsoft::UI::GetWindowIdFromWindow(hwnd);
  auto appWindow =
      Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

  auto size = GetStartupWindowSize(hwnd);
  HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitorInfo{sizeof(MONITORINFO)};
  if (GetMonitorInfo(monitor, &monitorInfo)) {
    int workWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
    int workHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
    int x = monitorInfo.rcWork.left + (workWidth - size.Width) / 2;
    int y = monitorInfo.rcWork.top + (workHeight - size.Height) / 2;
    appWindow.MoveAndResize(Windows::Graphics::RectInt32{x, y, size.Width,
                                                         size.Height});
  } else {
    appWindow.Resize(size);
  }

  if (auto presenter = appWindow.Presenter().try_as<OverlappedPresenter>()) {
    presenter.IsResizable(true);
    presenter.IsMaximizable(true);
  }
}

namespace winrt::EtherealScepter::implementation {

MainWindow::MainWindow() {
  InitializeComponent();
  Services::ThemeService::Instance().LoadSettings();
  SetTitleBarIcon(*this);
  ConfigureStartupWindow(*this);
  InitializeWindowSizePersistence();

  // Initialize theme service
  InitializeTheme();

  // 預設顯示 Dashboard
  ContentFrame().Navigate(xaml_typename<EtherealScepter::DashboardPage>());
}

MainWindow::~MainWindow() {
  if (m_saveWindowSizeTimer) {
    m_saveWindowSizeTimer.Stop();
  }

  SaveCurrentWindowSize();

  if (m_hasWindowChangedSubscription && m_appWindow) {
    m_appWindow.Changed(m_windowChangedToken);
    m_hasWindowChangedSubscription = false;
  }

  if (m_themeSubscriptionId != 0) {
    Services::ThemeService::Instance().Unsubscribe(m_themeSubscriptionId);
    m_themeSubscriptionId = 0;
  }
}

void MainWindow::InitializeWindowSizePersistence() {
  HWND hwnd = GetHwnd(*this);
  auto windowId = Microsoft::UI::GetWindowIdFromWindow(hwnd);
  m_appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

  auto dispatcherQueue = DispatcherQueue();
  if (dispatcherQueue) {
    m_saveWindowSizeTimer = dispatcherQueue.CreateTimer();
    m_saveWindowSizeTimer.Interval(std::chrono::milliseconds(500));
    m_saveWindowSizeTimer.Tick(
        [weak_this = get_weak()](auto const &, auto const &) {
          if (auto strong_this = weak_this.get()) {
            strong_this->m_saveWindowSizeTimer.Stop();
            strong_this->SaveCurrentWindowSize();
          }
        });
  }

  m_windowChangedToken = m_appWindow.Changed(
      {this, &MainWindow::OnWindowChanged});
  m_hasWindowChangedSubscription = true;
}

void MainWindow::OnWindowChanged(
    Microsoft::UI::Windowing::AppWindow const &sender,
    Microsoft::UI::Windowing::AppWindowChangedEventArgs const &args) {
  if (!args.DidSizeChange()) {
    return;
  }

  if (auto presenter = sender.Presenter().try_as<OverlappedPresenter>()) {
    if (presenter.State() != OverlappedPresenterState::Restored) {
      return;
    }
  }

  if (m_saveWindowSizeTimer) {
    m_saveWindowSizeTimer.Stop();
    m_saveWindowSizeTimer.Start();
  } else {
    SaveCurrentWindowSize();
  }
}

void MainWindow::SaveCurrentWindowSize() {
  if (!m_appWindow) {
    return;
  }

  if (auto presenter = m_appWindow.Presenter().try_as<OverlappedPresenter>()) {
    if (presenter.State() != OverlappedPresenterState::Restored) {
      return;
    }
  }

  auto size = m_appWindow.Size();
  if (size.Width <= 0 || size.Height <= 0) {
    return;
  }

  UINT dpi = GetWindowDpi(GetHwnd(*this));
  auto &themeService = Services::ThemeService::Instance();
  themeService.SetSavedWindowSize(ScalePixelToDip(size.Width, dpi),
                                  ScalePixelToDip(size.Height, dpi));
  themeService.SaveSettings();
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
  case Services::ThemeType::StarrySky:
  case Services::ThemeType::BlackSoulsLeaf:
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
    NavigationView const & /*sender*/,
    NavigationViewSelectionChangedEventArgs const &args) {

  // Handle Settings navigation
  if (args.IsSettingsSelected()) {
    ContentFrame().Navigate(xaml_typename<EtherealScepter::SettingsPage>());
    return;
  }

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
