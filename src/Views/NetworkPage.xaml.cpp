// Created by: EternityBoundary on Jan 3, 2026
#include "pch.h"
#include "NetworkPage.xaml.h"
#if __has_include("NetworkPage.g.cpp")
#include "NetworkPage.g.cpp"
#endif

#include "include/ViewModels/NetworkPageViewModel.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
namespace Services = ::EtherealScepter::Services;

namespace winrt::EtherealScepter::implementation {
NetworkPage::NetworkPage() {
  InitializeComponent();

  m_viewModel = winrt::make<ViewModels::implementation::NetworkPageViewModel>();

  // 訂閱主題變更
  m_themeSubscriptionId = Services::ThemeService::Instance().Subscribe(
      [weak_this = get_weak()](Services::ThemeType theme) {
        if (auto strong_this = weak_this.get()) {
          strong_this->OnThemeChanged(theme);
        }
      });
}

NetworkPage::~NetworkPage() {
  if (m_themeSubscriptionId != 0) {
    Services::ThemeService::Instance().Unsubscribe(m_themeSubscriptionId);
    m_themeSubscriptionId = 0;
  }
}

winrt::EtherealScepter::ViewModels::NetworkPageViewModel
NetworkPage::ViewModel() noexcept {
  return m_viewModel;
}

void NetworkPage::OnPageLoaded(IInspectable const& /*sender*/, RoutedEventArgs const& /*args*/) {
  ApplyThemeToCards();
}

void NetworkPage::OnThemeChanged(Services::ThemeType /*theme*/) {
  ApplyThemeToCards();
}

void NetworkPage::ApplyThemeToCards() {
  auto& themeService = Services::ThemeService::Instance();
  bool isCustomTheme = themeService.IsCustomThemeActive();

  auto externalIpCard = ExternalIpCard();
  auto connectivityCard = ConnectivityCard();
  auto dnsCard = DnsCard();
  auto portCard = PortCard();
  auto eventsCard = EventsCard();

  if (isCustomTheme) {
    auto brush = themeService.GetCardBackgroundBrush();
    if (brush) {
      if (externalIpCard) externalIpCard.Background(brush);
      if (connectivityCard) connectivityCard.Background(brush);
      if (dnsCard) dnsCard.Background(brush);
      if (portCard) portCard.Background(brush);
      if (eventsCard) eventsCard.Background(brush);
    }
  } else {
    if (externalIpCard) externalIpCard.Background(nullptr);
    if (connectivityCard) connectivityCard.Background(nullptr);
    if (dnsCard) dnsCard.Background(nullptr);
    if (portCard) portCard.Background(nullptr);
    if (eventsCard) eventsCard.Background(nullptr);
  }
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