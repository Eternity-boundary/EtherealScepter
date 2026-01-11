// Created by: EternityBoundary on Jan 3, 2025
#include "pch.h"
#include "NetworkPage.xaml.h"
#if __has_include("NetworkPage.g.cpp")
#include "NetworkPage.g.cpp"
#endif

#include "include/ViewModels/NetworkPageViewModel.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::EtherealScepter::implementation {
NetworkPage::NetworkPage() {
  InitializeComponent();

  m_viewModel = winrt::make<ViewModels::implementation::NetworkPageViewModel>();
}

winrt::EtherealScepter::ViewModels::NetworkPageViewModel
NetworkPage::ViewModel() noexcept {
  return m_viewModel;
}

void NetworkPage::OnRunTestsClicked(IInspectable const &,
                                    RoutedEventArgs const &) {
  if (m_viewModel) {
    m_viewModel.RunConnectivityTestsAsync();
  }
}

void NetworkPage::OnTestPortClicked(IInspectable const &,
                                    RoutedEventArgs const &) {
  if (!m_viewModel) return;

  auto portTextBox = PortTextBox();
  if (!portTextBox) return;

  auto portText = portTextBox.Text();
  if (portText.empty()) {
    m_viewModel.PortTestResult(L"✖ Please enter a port number");
    return;
  }

  try {
    int port = std::stoi(portText.c_str());
    m_viewModel.TestPortAsync(port);
  } catch (...) {
    m_viewModel.PortTestResult(L"✖ Invalid port number");
  }
}

void NetworkPage::OnTestExternalPortClicked(IInspectable const &,
                                            RoutedEventArgs const &) {
  if (!m_viewModel) return;

  auto portTextBox = PortTextBox();
  if (!portTextBox) return;

  auto portText = portTextBox.Text();
  if (portText.empty()) {
    m_viewModel.ExternalPortTestResult(L"✖ Please enter a port number");
    return;
  }

  try {
    int port = std::stoi(portText.c_str());
    m_viewModel.TestExternalPortAsync(port);
  } catch (...) {
    m_viewModel.ExternalPortTestResult(L"✖ Invalid port number");
  }
}
} // namespace winrt::EtherealScepter::implementation