#include "pch.h"
#include "UPnPPage.xaml.h"
#if __has_include("UPnPPage.g.cpp")
#include "UPnPPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::EtherealScepter::implementation
{
    int32_t UPnPPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void UPnPPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
