# EtherealScepter

![Language](https://img.shields.io/badge/language-C++-red)  [![MSBuildTest](https://github.com/Eternity-boundary/EtherealScepter/actions/workflows/msbuild.yml/badge.svg)](https://github.com/Eternity-boundary/EtherealScepter/actions/workflows/msbuild.yml)  
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

### Network
- 連接埠/DNS測試
- WAN / CGNAT 判斷邏輯整合

### UPnP
- IGD 裝置探索（SSDP）

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
## 🖼️預覽截圖
<img width="1622" height="891" alt="1768266063871" src="https://github.com/user-attachments/assets/cbffbdc7-4106-44d9-8423-0f52583805b5" />
<img width="1622" height="891" alt="2d2c00555c89e9565c356705b8b9cf5f" src="https://github.com/user-attachments/assets/311d88a6-a77c-4b12-97e7-44a0ba8cb474" />
<img width="1622" height="891" alt="1768266084237" src="https://github.com/user-attachments/assets/07d54499-2b5d-4aed-8ef9-99844077047e" />
<img width="1622" height="891" alt="1768266106514" src="https://github.com/user-attachments/assets/8c68ce9f-c8cd-476c-ae97-80af32427d9a" />
<img width="1622" height="891" alt="1768266166568" src="https://github.com/user-attachments/assets/1d79e055-0682-432b-9bfc-c3118ddcda89" />


---
⚠ 注意事項

本專案大量使用 C++/WinRT 工具鏈

IDL 修改後請務必：

確認檔案實際儲存至磁碟

Clean + Rebuild

必要時刪除 Generated Files / Unmerged / Merged

強烈建議搭配 Git 使用，以避免編輯器快取造成的錯誤判斷

---
## 📜 授權  
GNU GENERAL PUBLIC LICENSE Version 3
