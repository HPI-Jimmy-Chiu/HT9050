---
name: gpib-ht9045-sync
description: >
  （已併入 hpi-gpib；本空殼一週後刪除）
  GPIB9045 / RS232Standard ↔ HT9045 跨專案定義同步技能。
  將 HT9045 `MachineType.h` 的 Customer Code（CC_xxx）、
  `MessageDef.h/.cpp` 的 MSG_CMD_ 指令碼，以及 `enum eTestMode`
  同步至 GPIB9045 與 RS232Standard 對應的 `cmydef.h` 與 `MessageDef.h/.cpp`。
  使用時會詢問目前使用的 HT9045、GPIB9045 與 RS232Standard 版本資料夾路徑。
  觸發關鍵字：sync, 同步, Customer Code, CC_, MSG_CMD, eTestMode,
  cmydef, MessageDef, GPIB 同步, HT9045 同步, RS232 同步, RS232Standard, 跨專案同步。
---

本 skill 已併入 `hpi-gpib`：先讀 `.claude/skills/hpi-gpib/SKILL.md`（路由表與安全事項）。
原內容整份搬到 `.claude/skills/hpi-gpib/references/gpib-ht9045-sync/gpib-ht9045-sync.md`（只有指向舊位置的連結改成新位置；舊的 `SKILL.md:<行號>` 引用，行號在新檔照舊有效；舊的 `references/...` 相對路徑改接在 `.claude/skills/hpi-gpib/references/gpib-ht9045-sync/` 底下）。
