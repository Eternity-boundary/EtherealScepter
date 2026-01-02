#include "pch.h"
#include "PortPage.xaml.h"
#if __has_include("PortPage.g.cpp")
#include "PortPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::EtherealScepter::implementation
{
    int32_t PortPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void PortPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
