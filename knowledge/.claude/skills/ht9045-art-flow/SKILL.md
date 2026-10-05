---
name: ht9045-art-flow
description: >
  （已併入 hpi-gpib；本空殼一週後刪除）
  HT9045 / HT9046LS / HT9011UC Handler 端 ART（Auto Retest 自動重測）流程知識庫。
  涵蓋 [A10-1] Enable ART 設定、SCK_ART 模組（fSCKART）、iCurrent93KARTStep 狀態機、
  DoART_AfterCleanOut() FT/RT lot-end 三岔分支、SetLotState() 與 GPIB 通訊、
  bAutoRetestGPIBmode / bUseSCKART / iTesterType 旗標關係、
  FT Lot Start / FT Lot End / RT / Final Lot End 與 GPIB SRQKIND 對應、
  「請結批報表 / Please Print Summary」對話框來源、ContinuStart_ART 執行模式。
  關鍵字：ART, Auto Retest, A10, Enable ART, SCK_ART, fSCKART, iCurrent93KARTStep,
  DoART_AfterCleanOut, SetLotState, bAutoRetestGPIBmode, bUseSCKART, iTesterType,
  bFirstTestAutoRetestGPIB, bEndLotAutoRetestGPIB, bWaitStartLotAutoRetestGPIB,
  iLotStatus, SRQKIND, SRQ 0xC0, FT Lot End, RT Lot Start, Final Lot End,
  ContinuStart_ART, rsmContinuRetest_ART, Move Tray to Loader, Please Print Summary,
  請結批報表, CheckNeedRT, MSG_CMD_LotStatus, MSG_CMD_SCKART_SRQMASK, TESNA, TeraTech,
  bART_SECSGEM_93K, iAutoRetestTCPmode, bSCKART_RunARTWithoutCmd, bDummyART,
  DoInitialStart, MSG_CMD_SCKART_RunDummy, Dummy ART Running Status
---

本 skill 已併入 `hpi-gpib`：先讀 `.claude/skills/hpi-gpib/SKILL.md`（路由表與安全事項）。
原內容整份搬到 `.claude/skills/hpi-gpib/references/ht9045-art-flow/ht9045-art-flow.md`（只有指向舊位置的連結改成新位置；舊的 `SKILL.md:<行號>` 引用，行號在新檔照舊有效；舊的 `references/...` 相對路徑改接在 `.claude/skills/hpi-gpib/references/ht9045-art-flow/` 底下）。
