#include "pch.h"

#include "../Views/MainWindow.xaml.h"
#include "App.xaml.h"
#include "src/Views/Theme/DefaultThemes.h"

#if __has_include("App.g.cpp")
#include "App.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::EtherealScepter::implementation {
/// <summary>
/// Initializes the singleton application object.  This is the first line of
/// authored code executed, and as such is the logical equivalent of main() or
/// WinMain().
/// </summary>
App::App() {
  // Xaml objects should not call InitializeComponent during construction.
  // See
  // https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent

#if defined _DEBUG &&                                                          \
    !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
  UnhandledException(
      [](IInspectable const &, UnhandledExceptionEventArgs const &e) {
        if (IsDebuggerPresent()) {
          auto errorMessage = e.Message();
          __debugbreak();
        }
      });
#endif
}

/// <summary>
/// Invoked when the application is launched.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const &e) {
  s_current = this;

  // 先註冊系統預設主題（這會設定 themeName 等配置）
  ::EtherealScepter::Views::Theme::RegisterSystemDefaultThemes();

  window = make<MainWindow>();
  window.Activate();

  // MainWindow::InitializeTheme() 會呼叫 LoadSettingsAsync()，
  // 這會從儲存的設定檔中載入使用者的設定（包括選擇的主題類型）。
  // 但是 LoadSettingsAsync() 也會覆蓋主題配置，所以我們需要再次註冊預設主題。
  // 重新註冊預設主題，確保 themeName 等配置是正確的
  ::EtherealScepter::Views::Theme::RegisterSystemDefaultThemes();

  m_dashboardVm = winrt::EtherealScepter::ViewModels::DashboardViewModel{};
}
} // namespace winrt::EtherealScepter::implementation
