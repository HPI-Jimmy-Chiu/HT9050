---
name: ht9045-atc
description: >
  （已併入 hpi-gpib；本空殼一週後刪除）
  HT9045 IC Test Handler ATC（溫度控制系統）通訊模組知識庫。
  當使用者詢問 ATC 溫控流程、ATC 通訊指令（code 1001~1137）、
  ATC_Handler_Side.cpp 指令實作、Handler↔ATC TCP 封包格式、
  ATC 版本型號差異（ATC_3.5 / ATC_Rogers / ATC_3.1 / 5.0 / 6.0 / 7.0）、
  iATC_MODE_TYPE 判斷邏輯、新增或修改 ATC 指令 case、
  ProcessHandlerCommand / SendCommandToHandler 空實作 bug、
  ATC Self-Test 流程、Lot Start/End 通知、FFC/PFC 功能、
  TJ Slope/Offset 校正、Dynamic PID（TSMC）、HulkMode、
  多區 Tc Offset、Recipe 傳輸、MTK ASIF 指令、2DID 設定、
  水閥/水流/水警告、Chiller 控制等問題時，應先載入此技能。
  觸發關鍵字：ATC, ATCInterface, ATC_Handler_Side, iATC_MODE_TYPE,
  SendCommand, ProcessHandlerCommand, ProcessATC_BufferCommand,
  ATC_RECIPE_FILE, ATC_RUN_STOP, ATC_READ_TEMP, ATC_TEST_START,
  ATC_LOT_START, ATC_SELFTEST, FFC, PFC, TJ Offset, Dynamic PID,
  HulkMode, ASIF, 2DID, Chiller, 水閥, 溫控, ATC指令, ATC通訊。
---

本 skill 已併入 `hpi-gpib`：先讀 `.claude/skills/hpi-gpib/SKILL.md`（路由表與安全事項）。
原內容整份搬到 `.claude/skills/hpi-gpib/references/ht9045-atc/ht9045-atc.md`（只有指向舊位置的連結改成新位置；舊的 `SKILL.md:<行號>` 引用，行號在新檔照舊有效；舊的 `references/...` 相對路徑改接在 `.claude/skills/hpi-gpib/references/ht9045-atc/` 底下）。
