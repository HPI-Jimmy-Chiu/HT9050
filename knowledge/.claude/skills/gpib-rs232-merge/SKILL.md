---
name: gpib-rs232-merge
description: >
  （已併入 hpi-gpib；本空殼一週後刪除）
  GPIB_RS232 三介面整合設計知識庫。將 GPIB_RS232 程式從純 GPIB
  擴展為支援 TTL/GPIB/RS232 三種介面的設計與實作。
  涵蓋 g_iTestType 架構、COM port 開關、startup ini 還原、
  iTesterMode 職責分離、DIO TypeName log、Qorvo guard。
  關鍵字：g_iTestType, TTL_MODE, GPIB_MODE, RS232_MODE,
  MSG_CMD_ChangeGpib, iTestType, OpenTesterComm, CloseTesterComm,
  iLotStatus, SendMessageToGpibProg, ProcessHVisionConnect, eRs232Mode
---

本 skill 已併入 `hpi-gpib`：先讀 `.claude/skills/hpi-gpib/SKILL.md`（路由表與安全事項）。
原內容整份搬到 `.claude/skills/hpi-gpib/references/gpib-rs232-merge/gpib-rs232-merge.md`（只有指向舊位置的連結改成新位置；舊的 `SKILL.md:<行號>` 引用，行號在新檔照舊有效；舊的 `references/...` 相對路徑改接在 `.claude/skills/hpi-gpib/references/gpib-rs232-merge/` 底下）。
