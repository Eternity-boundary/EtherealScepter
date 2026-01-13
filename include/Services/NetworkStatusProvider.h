// Created by: EternityBoundary on Jan 12, 2026
#pragma once

#include "include/Services/NetworkStatusService.h"
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>
#include <winrt/impl/Microsoft.UI.Dispatching.2.h>
#include <winrt/impl/Windows.Foundation.1.h>

namespace EtherealScepter::Services {
// 訂閱者回呼類型
using NetworkStatusChangedCallback =
    std::function<void(NetworkSnapshot const &)>;

// 共用的網路狀態提供者（單例）
// 快取查詢結果，避免重複網路請求
// 提供訂閱/通知機制，讓多個 ViewModel 共享資料
// 執行緒安全
class NetworkStatusProvider {
public:
  static NetworkStatusProvider &Instance();

  // 禁止複製和移動
  NetworkStatusProvider(NetworkStatusProvider const &) = delete;
  NetworkStatusProvider &operator=(NetworkStatusProvider const &) = delete;

  // 取得快取的 Snapshot（如果有效），否則觸發重新查詢
  NetworkSnapshot GetCachedSnapshot();

  // 強制重新查詢（背景執行），完成後通知所有訂閱者
  winrt::Windows::Foundation::IAsyncAction RefreshAsync();

  // 訂閱狀態變更通知
  // 訂閱 ID，用於取消訂閱
  uint64_t Subscribe(NetworkStatusChangedCallback callback);

  // 取消訂閱
  void Unsubscribe(uint64_t subscriptionId);

  // 設定快取有效期（預設 30 秒）
  void SetCacheDuration(std::chrono::seconds duration);

  // 檢查快取是否有效
  bool IsCacheValid() const;

  // 檢查是否正在重新整理中
  bool IsRefreshing() const;

  // 初始化 DispatcherQueue（必須在 UI 執行緒呼叫）
  void InitializeDispatcher();

private:
  NetworkStatusProvider() = default;
  ~NetworkStatusProvider() = default;

  void NotifySubscribers(NetworkSnapshot const &snapshot);

  mutable std::shared_mutex m_mutex;
  NetworkSnapshot m_cachedSnapshot{};
  std::chrono::steady_clock::time_point m_lastRefreshTime{};
  std::chrono::seconds m_cacheDuration{30};
  bool m_hasValidCache{false};
  bool m_isRefreshing{false};

  std::mutex m_subscribersMutex;
  std::vector<std::pair<uint64_t, NetworkStatusChangedCallback>> m_subscribers;
  uint64_t m_nextSubscriptionId{1};

  winrt::Microsoft::UI::Dispatching::DispatcherQueue m_dispatcherQueue{nullptr};
};

} // namespace EtherealScepter::Services
