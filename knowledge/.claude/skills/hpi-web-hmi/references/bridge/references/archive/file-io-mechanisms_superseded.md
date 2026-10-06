> 保存來源：`.claude/skills/ht9045-json-bridge/references/archive/file-io-mechanisms_superseded.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# file-io-mechanisms.md —— 已失效內容封存

> 這份檔案只收「已經失效、不再需要追蹤」的舊內容，從 `references/file-io-mechanisms.md` 移出來的。
> 每段開頭寫：移出日期、原本在哪一節、為什麼失效。**原檔對應位置留著標題／表格框架，不留這裡的舊句。**
> 不要刪這份檔案裡的任何段落——這是稽核用的封存，不是垃圾桶。

---

## 移出日期 20260926｜原本在「C.2 `LAST_LEVEL_SET` ↔ `system\levelset.dat`」的兩個項目符號

**為什麼失效**：這兩點描述的是 20260923 量測當下（S64 之前）的狀態——`system.levels.put` 還是
舊的、鉗制還沒搬進來。S64（commit `8c5ea501`）已經把 `system.levels.put` 換成呼叫新檔
`WebLevelSet.cpp` 的 `W906_LevelSetPut`，照 golden `TfSecurity::FormClose` 全流程（含三條
鉗制、備份、`SaveJamLevel`、整塊重讀比對），`cSecurity.cpp` 的 SEC-W1／SEC-W2 兩個 GATE 都已
解開。現況見 `references/file-io-mechanisms.md` §C.2 現版、`write-inventory.md`「二、逐結構
清單」`LevelSet` 列、ChangeLog `CHANGES_20260926_Steven.md` §11.27。

**原句**：

> 目前 `wb_serve` 已有 `GET /api/system/levelset`＋`WS system.levels.put`（`wb_serve.cpp:1359`），
> 索引定址、`min 0`／`max 4`（`:1400`）。缺的是 **index → 權限名稱**那層。

> `SetLevelSet()` 之前還有三條鉗制要搬進 `ClampLevelSet`（`:448-457`）：`[87]`（Teaching）不得
> 低於 `iDefSupervisorLevel`；`[129]` 不得高於 `[130]`（SCK 93K ART）；`[86]`（Motion View）
> 不得低於 `iDefEngineerLevel`。加上 `GetLevelSet()` 的客戶碼與 `CheckRange`。

<!-- preserved-content:end -->
