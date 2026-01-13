// Created by: EternityBoundary on Jan 12, 2026
#include "pch.h"

#include "include/Services/NetworkStatusProvider.h"
#include <algorithm>
#include <winrt/Microsoft.UI.Dispatching.h>

namespace EtherealScepter::Services {

NetworkStatusProvider &NetworkStatusProvider::Instance() {
  static NetworkStatusProvider instance;
  return instance;
}

void NetworkStatusProvider::InitializeDispatcher() {
  if (!m_dispatcherQueue) {
    m_dispatcherQueue = winrt::Microsoft::UI::Dispatching::DispatcherQueue::
        GetForCurrentThread();
  }
}

NetworkSnapshot NetworkStatusProvider::GetCachedSnapshot() {
  {
    std::shared_lock lock(m_mutex);
    if (m_hasValidCache && IsCacheValid()) {
      return m_cachedSnapshot;
    }
  }

  // 快取無效，觸發重新整理（但不等待）
  if (!IsRefreshing()) {
    RefreshAsync();
  }

  // 返回舊的快取（可能是空的）
  std::shared_lock lock(m_mutex);
  return m_cachedSnapshot;
}

winrt::Windows::Foundation::IAsyncAction NetworkStatusProvider::RefreshAsync() {
  // 檢查是否已在重新整理中
  {
    std::unique_lock lock(m_mutex);
    if (m_isRefreshing) {
      co_return;
    }
    m_isRefreshing = true;
  }

  NetworkSnapshot snapshot{};
  bool hasError = false;

  // 背景執行查詢
  co_await winrt::resume_background();

  try {
    snapshot = NetworkStatusService::Query();
  } catch (winrt::hresult_error const& e) {
    snapshot.summary = L"Error: " + winrt::hstring(e.message());
    hasError = true;
  }catch (...) {
    snapshot.summary = L"Error: Unknown exception";
    hasError = true;
  }

  // 更新快取
  {
    std::unique_lock lock(m_mutex);
    if (!hasError) {
      m_cachedSnapshot = snapshot;
      m_lastRefreshTime = std::chrono::steady_clock::now();
      m_hasValidCache = true;
    }
    m_isRefreshing = false;
  }

  // 通知訂閱者（在 UI 執行緒）
  if (!hasError) {
    NotifySubscribers(snapshot);
  }

  co_return;
}

uint64_t
NetworkStatusProvider::Subscribe(NetworkStatusChangedCallback callback) {
  std::lock_guard lock(m_subscribersMutex);
  uint64_t id = m_nextSubscriptionId++;
  m_subscribers.emplace_back(id, std::move(callback));
  return id;
}

void NetworkStatusProvider::Unsubscribe(uint64_t subscriptionId) {
  std::lock_guard lock(m_subscribersMutex);
  m_subscribers.erase(std::remove_if(m_subscribers.begin(), m_subscribers.end(),
                                     [subscriptionId](auto const &pair) {
                                       return pair.first == subscriptionId;
                                     }),
                      m_subscribers.end());
}

void NetworkStatusProvider::SetCacheDuration(std::chrono::seconds duration) {
  std::unique_lock lock(m_mutex);
  m_cacheDuration = duration;
}

bool NetworkStatusProvider::IsCacheValid() const {
  auto now = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::seconds>(now - m_lastRefreshTime);
  return elapsed < m_cacheDuration;
}

bool NetworkStatusProvider::IsRefreshing() const {
  std::shared_lock lock(m_mutex);
  return m_isRefreshing;
}

void NetworkStatusProvider::NotifySubscribers(NetworkSnapshot const &snapshot) {
  std::vector<NetworkStatusChangedCallback> callbacks;

  {
    std::lock_guard lock(m_subscribersMutex);
    callbacks.reserve(m_subscribers.size());
    for (auto const &[id, callback] : m_subscribers) {
      callbacks.push_back(callback);
    }
  }

  // 如果有 DispatcherQueue，在 UI 執行緒通知
  if (m_dispatcherQueue) {
    m_dispatcherQueue.TryEnqueue(
        [callbacks = std::move(callbacks), snapshot]() {
          for (auto const &callback : callbacks) {
            if (callback) {
              callback(snapshot);
            }
          }
        });
  } else {
    // 直接在當前執行緒通知（可能不安全，但作為 fallback）
    for (auto const &callback : callbacks) {
      if (callback) {
        callback(snapshot);
      }
    }
  }
}

} // namespace EtherealScepter::Services
