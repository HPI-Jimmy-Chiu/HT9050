---
name: ht9045-atc-interface
description: >
  （已併入 hpi-gpib；本空殼一週後刪除）
  HT9045 Handler ↔ ATC 溫控設備 TCP 通訊協定知識庫。涵蓋 137 個 ATC 命令（1001-1137）、封包格式、Handler Side 類別、溫度/Offset 傳送流程、Site Mapping、Recipe、Self-Test、TJ/FFC/PFC、MTK ASIF、TSMC Hulk 等。Use when: 分析 ATC 通訊 log、追 @1002/@1003/@1004 等封包內容、除錯 ATC 不回應、新增/修改 ATC 命令、解析 ATC hangup 資料夾 TXT 檔。關鍵字：ATC, ATC Interface, ATC_SET_TEMP, ATC_SET_TOFS, ATC_SITE_ENABLED, ATC_Handler_Side, SendCommand, GetData, @1001, @1002, @1003, @1004, @1016, @1105, ATC_Multi_Temperature_Control, ATC_SET_T2OFS, HANDLER_2DID, ATC Recipe, Self-Test, TJ Offset, FFC, PFC, MTK ASIF, TSMC Hulk。
  （20261001：V906 現況與 ATC.ini 位置 → ht9045-temperature；本 skill 與 ht9045-atc 重疊，見 ht9045-temperature references/temperature-facts-index.md §1）
---

本 skill 已併入 `hpi-gpib`：先讀 `.claude/skills/hpi-gpib/SKILL.md`（路由表與安全事項）。
原內容整份搬到 `.claude/skills/hpi-gpib/references/ht9045-atc-interface/ht9045-atc-interface.md`（只有指向舊位置的連結改成新位置；舊的 `SKILL.md:<行號>` 引用，行號在新檔照舊有效；舊的 `references/...` 相對路徑改接在 `.claude/skills/hpi-gpib/references/ht9045-atc-interface/` 底下）。
