---
name: hpi-build
description: "Handler建置、兩組態gate與加速的共同入口；按BCB6 bpr2mak／make或V906 CMake／build.bat分流，涵蓋增量、wb_serve、Ninja、ccache／PCH、toolchain、PE／caller完成標準與歷史量測。HT9050與其他Handler同題，保留bcb_build／cpp_build舊名與工具。"
---

# Handler建置與驗證

同題管理HT9050與其他Handler；先依來源樹／工具鏈分版本，再選增量、目標建置或交付gate。

## 先確認

- [共同流程](references/common.md)：commit、來源樹、工具鏈、組態與目標需對得上，編譯成功與機台驗證分開。
- [機型／版本](references/machines/index.md)：HT9050不等於WinLibs，其他Handler也可能使用V906；BCB6與C++移植樹分流。
- [客戶／runtime](references/customers.md)：機型、CUSTOMER_CODE與建置define分開，按實際配置核對。
- 活定位使用版本／檔名＋batch label、CMake option、function／變數；原文行號僅作歷史定位。

## 按問題選路

| 問題 | Reference |
|---|---|
| BCB6、.bpr／.mak、缺lib／Obj／PCH與原工具 | [BCB6路由](references/versions/bcb6.md) |
| V906、sim／ship、wb_serve、完整交付gate | [C++路由](references/versions/cpp.md) |
| Ninja、ccache／PCH、增量速度與標頭／Motor-IO評估 | [加速與量測](references/methods/index.md)，保留原裁決與日期 |
| 本次build入口／組態選項來源及查證界線 | [來源核對](references/runtime/current-source.md) |
| 同事指定的代編／代跑與結果回報 | [既有代編Skill](../ops-ht9045-proxy-build/SKILL.md)，角色與版本以該流程及當次授權為準 |

保留兩個舊入口、原metadata、script與原量測。文件整理不執行build／ctest、安裝、clean／prune或關閉其他session；模擬／真機依建置期SOFT_SIMULTE，不恢復已退場--dry。
