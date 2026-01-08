//Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <winrt/base.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <microsoft.ui.xaml.window.h>     
#include <winrt/Microsoft.UI.Interop.h>   
#include <winrt/Microsoft.UI.Windowing.h> 
#include <winrt/Windows.ApplicationModel.h> 
#include <winrt/Windows.Storage.h>

#include "DashboardPage.xaml.h"
#include "UPnPPage.xaml.h"
#include "PortPage.xaml.h"
#include "NetworkPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Interop;
using namespace winrt::Microsoft::UI::Windowing;

static HWND GetHwnd(winrt::Microsoft::UI::Xaml::Window const& window)
{
    HWND hwnd{};
    auto native = window.as<IWindowNative>();
    check_hresult(native->get_WindowHandle(&hwnd));
    return hwnd;
}

void SetTitleBarIcon(winrt::Microsoft::UI::Xaml::Window const& window)
{
    HWND hwnd = GetHwnd(window);

    auto windowId = Microsoft::UI::GetWindowIdFromWindow(hwnd);
    auto appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

    auto root = Windows::ApplicationModel::Package::Current().InstalledLocation().Path();
    winrt::hstring iconPath = root + L"\\Assets\\flash.ico";

    appWindow.SetIcon(iconPath);
}

namespace winrt::EtherealScepter::implementation
{


    MainWindow::MainWindow()
    {
        InitializeComponent();
        SetTitleBarIcon(*this);
        // 預設顯示 Dashboard
        ContentFrame().Navigate(xaml_typename<EtherealScepter::DashboardPage>());
    }

    void MainWindow::OnNavigationChanged(
        NavigationView const& /*sender*/,
        NavigationViewSelectionChangedEventArgs const& args)
    {
        if (!args.SelectedItem()) return;

        auto item = args.SelectedItem().as<NavigationViewItem>();
        auto tag = unbox_value<hstring>(item.Tag());

        if (tag == L"dashboard")
        {
            ContentFrame().Navigate(xaml_typename<EtherealScepter::DashboardPage>());
        }
        else if (tag == L"upnp")
        {
            ContentFrame().Navigate(xaml_typename<EtherealScepter::UPnPPage>());
        }
        else if (tag == L"ports")
        {
            ContentFrame().Navigate(xaml_typename<EtherealScepter::PortPage>());
        }
        else if (tag == L"network")
        {
            ContentFrame().Navigate(xaml_typename<EtherealScepter::NetworkPage>());
        }
    }
}
