# §五、寫方向（#3 → #6） —— 全文

> 從 `SKILL.md` **§五** 拆出（拆出日期 2026-09-23，skill 維護第二輪續拆）。
> **章節編號不變**：§五 同名同號，8 條規則的序號不變。原文**逐字**保留（含已被更正段推翻的原句 —— 那是稽核軌跡），
> `SKILL.md` 原處留 stub（一句話結論 ＋ 指到本檔）。
> 原本被誰引用：SKILL.md §一 #5（「見 §五」）、§二 `IniConfig` 那列（「§五 第 8 條」）、§十 #8；全樹 `.cpp`／`.h`／`.py` 無直接引用（2026-09-23 grep 實測）。
> 本檔內的 `§X`／`§4.x` 交叉引用一律指 `SKILL.md` 的章節編號，`SKILL.md` 那裡搜得到、再一跳就到對應 reference；
> `references/<檔>.md` 這種路徑是以 `SKILL.md` 所在目錄為基準寫的（原文未改），在本檔內即同目錄的 `<檔>.md`。
>
> ⛔ **現況補註（Steven 團隊 20260926，HEAD 8fad1522）**：下面這條「`FromJson`→`Clamp*`→`Persist`→`Reload`」管線**沒有實作**
> （`struct.put` 只到 dryRun，`JsonBridge/StructApply.cpp:252`；`phases.md` S6 更正）。20260924 下午的裁決（`decisions.md` 二之三）
> 改走 **C 路**：頁面 `editlist.save` → 伺服器跑 **golden 自己的存檔流程**（從 golden 原檔轉出來的 `saveFlow`，golden 的鉗制、
> 權限、確認框都在裡面）→ golden 自己的 `SaveEditTextToFile`／`WriteIniData` 寫檔 → 頁面一律重讀（skill `ht9045-html-json`
> `route-c-golden-bridge.md` §3.0b）。本檔規則仍成立的部分：第 3 條「寫完必重讀」（頁面 `gbSave()` 寫完一定 `gbLoad()`）、
> 第 8 條 editlist 的格式（因為 C 形狀直接呼叫 golden 那 429 行 `SaveEditTextToFile`，格式自然一致；G1 判準改成
> `generators.md` 五 的兩段式）。其餘（`Clamp*` 純函式、`FieldDesc.fmt`）只是當時的設計。

---

## 五、寫方向（#3 → #6）

**沒有控制項。** 使用者 20260923 裁決：顯示層是 HTML，golden 的「元件→寫檔→讀檔→填元件」變成：

```
  JSON ──FromJson(dryRun)──→ 暫存結構副本
       ──Clamp*()──→ 套 golden ③ 的存檔鉗制，回報哪些欄位被改／被拒        ← 在 dryRun 就回
       ──（dryRun=false 才繼續）
       ──Persist──→ 依 Binding.persist 三選一：
            ini      依 FieldDesc 逐欄 WriteIniData（格式字串照 golden）
            editlist 依 FieldDesc 逐欄寫 TMemIniFile（iDecimalPoint／iTransformType 照 golden）
            blob     改記憶體後呼叫 golden 的 SetLevelSet()／WriteLastDataFile() 整塊落地
       ──ReadFile()──→ _File 結構                                            ← #6，JerryYang 的翻譯
       ──ToJson──→ 回頁面
```

規則：
1. **dryRun 先跑**，`Clamp*` 在 dryRun 就執行並回報 `clamped:{欄位: 新值}`，頁面看得到「你填 15，會存成 12」。
2. 再備份，再原子替換（沿用 `system.file.put` 契約）。
3. **`Persist` 之後必跑 `Reload`**（#6）。回頁面的是重讀後的值，不是頁面送來的值 —— 兩者不同就是 bug 被抓到了。
4. 字串欄位：**不轉碼**。JSON 與檔案都是 UTF-8，直通。寫檔前只驗證位元組序列是合法 UTF-8（JSON 解析器已保證，但 `char[]` 定長欄位截斷時可能切在多位元組中間——`strncpy` 到 `sizeof` 邊界後要退到最後一個完整字元，否則檔案裡會出現半個字）。
5. `*.live` 綁定一律唯讀；它的值**不是**由 `struct.put` 直接寫，而是 `Reload` 裡照 golden 叫一次 `DoStructUnitConvert()` 之後產生（見 §3.3 與 `references/unit-convert-layer.md`）。
6. `Clamp*` 必須是**純函式**（吃結構、回結構），不碰檔、不碰全域；這樣才能在 dryRun 跑，也才能單元測試。
7. `vclcompat/` 的控制項 shim **不在這條路上**。它們留給還沒翻譯完的 golden 邏輯用，不當寫檔中介。
8. **`editlist` 持久化的三條格式硬規則**（使用者 20260923：「cpp 存檔要按照原本的格式」）：
   - **順序**：`FieldDesc` 的排列順序＝golden `InitConfigEdtList_ItemA..P` 裡 `elConfig->Add()` 的**呼叫順序**，產生器抽的時候要保留原始出現序，不得排序；區段第一次出現的位置也由它決定。
   - **通道**：一律經 `vclcompat/IniFiles.h` 的 `TMemIniFile`（它保留插入順序、原始位元組、`UpdateFile()` 才落盤），不自己拼字串；這樣 CRLF／`Key=Value`／無空行這些由同一個 shim 保證。
   - **值字串**：照 `SaveEditTextToFile` 的規則（量到的 `ini->Write*` 用法：`WriteInteger` 3 處、`WriteString` 4 處）——`ECBool`／`TCheckBox` → `WriteInteger(Checked?1:0)`；`TComboBox`／`TRadioGroup` → `WriteInteger(ItemIndex)`；整數族 `TEdit` → 先做 `iTransformType` 縱放（mm→×100、s→×1000）再 `WriteString(AnsiString(iCurr))`；浮點依 `iDecimalPoint` 用 `%0.Nf` 後 `WriteString`；文字 `WriteString(CEd->Text)` 原樣；日期 `WriteString(sNowStr)`。每一條都進 `FieldDesc.fmt`／`.scale`，由產生器從 `Add()` 的 `EC型別`＋控制項型別推出，不手填。
   G1 對 `iniConfig` 的判定：拿同一份 `IniConfig` 讓 golden 的 `SaveEditTextToFile` 與新的 `Persist` 各存一份，`cmp` 必須零差異；改一欄後再比，diff 只能是那一行。

