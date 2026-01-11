# EtherealScepter

![Language](https://img.shields.io/badge/language-C++-red)  ![Language](https://img.shields.io/badge/build-passing-blue)  
一個以 **WinUI 3 + C++/WinRT** 打造的UPnP管理工具，用於快速建立網路連接埠轉發

---
## ⚠開發中，功能尚未完成
## ✨ 專案目標

EtherealScepter 目前是一個「UPnP 開關工具」，向下列目標發展：

- 🔍 **即時可視化的網路狀態儀表板**
- 🔁 **可安全管理 UPnP Port Mapping**
---

## 🖥 功能概覽（持續擴充中）

### Dashboard（狀態總覽）
- Network Status（Connected / Disconnected）
- UPnP Status（Enabled / Disabled）
- NAT Type（Open / Moderate / Strict）
- Local IP
- WAN IP
- CGNAT 判斷
- Port Mapping 摘要

### Network(未完成)
- 連接埠/DNS測試
- WAN / CGNAT 判斷邏輯整合

### UPnP
- IGD 裝置探索（SSDP）
- External IP 查詢
- Port Mapping 管理

### Port
- 顯示目前 Port Mapping
- 新增 / 移除對應規則

---

## 🧱 技術架構

- **UI Framework**：WinUI 3
- **語言**：C++23
- **WinRT 投影**：C++/WinRT
- **架構模式**：MVVM
- **資料綁定**：`x:Bind`
- **非同步模型**：C++ coroutine (`IAsyncAction`)
- **UI Thread 回切**：`winrt::apartment_context`

---
⚠ 注意事項

本專案大量使用 C++/WinRT 工具鏈

IDL 修改後請務必：

確認檔案實際儲存至磁碟

Clean + Rebuild

必要時刪除 Generated Files / Unmerged / Merged

強烈建議搭配 Git 使用，以避免編輯器快取造成的錯誤判斷

---
📜 授權
GNU GENERAL PUBLIC LICENSE Version 3
