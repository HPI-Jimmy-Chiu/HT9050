# SKILL.md（ht9045-html-json）—— 已失效內容封存

> 這份檔案只收「已經失效、不再需要追蹤」的舊內容，從本 skill 的 `SKILL.md` 移出來的
> （規則照 skill `ht9045-json-bridge` 的 `references/archive/` 慣例）。
> 每段開頭寫：移出日期、原本在哪一節、為什麼失效。**原檔對應位置已換成現況，不留這裡的舊句。**
> 不要刪這份檔案裡的任何段落——這是稽核用的封存，不是垃圾桶。

---

## 移出日期 20260926｜原本在「⛔ 先讀這一節：有三條路」→ C 路那一條的最後一句

**為什麼失效**：那是 20260924 的狀態（C 路只有三個結構）。20260926 HEAD 8fad1522：
`tools/editlist/_integrated.txt` 32 個結構、`web/page/ht9045_wire_engine.js:1038-1061` 的 `GOLDEN_BRIDGE`
24 頁；`TestIF_File` 的 A 形狀 `TFTestIF` bridge 已於 commit `f89be4ce` 退役（`FileRW/_registry.cpp`
只剩 `kBridge_TfHotPlate`），`Tester.Data` 改由 C 路 `FileRW/TestIF_File_TesterIF.cpp` 接手。
現況總表在 `references/route-c-golden-bridge.md` §6。

**原句**：

> 目前負責 `IniConfig`／`HotPlateForm_File`／
> `TestIF_File`。完整規格見 [route-c-golden-bridge.md](references/route-c-golden-bridge.md)。

---

## 移出日期 20260926｜原本在「四大類 JSON」→「4. 生產記錄檔」的最後一句

**為什麼失效**：`io.*`／`motor.*` 兩個 prefix 自 skill `ht9045-json-bridge` 的 S9（`JsonBridge/ChanIo.cpp`／
`ChanMotor.cpp`，commit `8deffd1`）起就存在（`io.di`／`io.do`／`io.ver`／`motor.axes`／`motor.count`／
`motor.ver`，20260926 grep 實測）；只有 `task.*`／`system.*` 仍不存在。

**原句**：

> `io.*`／`motor.*`／`task.*`／`system.*` 四個 prefix **目前不存在**。

---

## 移出日期 20260927｜原本在「⛔ 先讀這一節：有三條路」→ C 路那一條的「現況」開頭兩行

**為什麼失效**：20260927 `tools/editlist/_integrated.txt` 加了 `ACTForm`／`Winway`／`Monitor`（commit `217e7e5e`），HEAD 227b79db
是 35 個結構；`GOLDEN_BRIDGE` 仍是 24 頁（`web/page/ht9045_wire_engine.js:1038-1061`，63bc008f 之後這個檔沒有 commit）。
現況總表在 `references/route-c-golden-bridge.md` §6（#33～#35 已補）。

**原句**：

> 現況（Steven 團隊 20260926，HEAD 8fad1522）：
>   `tools/editlist/_integrated.txt` 32 個結構、24 頁在 `ht9045_wire_engine.js` 的 `GOLDEN_BRIDGE`
