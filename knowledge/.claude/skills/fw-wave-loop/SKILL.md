---
name: fw-wave-loop
description: HT9045 V906 DFM→WEB 戰役（FW CAMPAIGN）的自動波次政策。定義最終目標、三件套交付、選標的規則、驗收 gate、夜間 loop 與額度中斷協議、安全邊界。Use when：執行 /fw-wave、規劃表單→web 波次、tag 接線波、dfm2web 產生器、判斷某表單能不能夜間自動做。關鍵字：FW-W, DFM→WEB, 表單波次, facade, tag 接線, formview, emit_web, WebBridgeTags, 夜間 loop
---

> **//Steven 20260915** — 本檔於 2026-09-15 更新。
> 當日完整變更紀錄：`D:\HT9045\CHANGES_20260915_Steven.md`


# FW 波次迴圈政策（HT9045 V906 DFM→WEB）

**權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/DFM2WEB_CAMPAIGN_PLAN.md`**（§0 定案鏈、
§4 波次佇列、§5 gate、§6 loop 協議、§7 停止條件）。本 skill 是執行摘要；細節以計畫書為準，
**兩邊不一致時以計畫書為準並回頭修這裡**。

## 先讀 pt-wave-loop（單一出處，不複寫）

`pt-wave-loop` skill 的以下段落**全部適用**，動工前載入它：
六個已付代價的陷阱／硬邊界（Big5、EOL、唯讀樹）／「不准自行停下」三規則／
冷啟動協議／額度中斷實測表／「agent 論證比程式碼更常錯」。

## ⛔ CPP 模式（使用者 20260915 裁決）

**資料一律走 `wb_serve` 的 API，不啟用 C# JSON Simulator。**

```
/api/recipe/     16 個配方文件
/api/system/     40 個機台設定檔
/api/text/       6 個記錄來源（唯讀）
```

模擬器寫的是模擬值，而且那些檔是 `sync_web.py` 的 `runtime_owned`——跑它等於用假
資料蓋掉真實執行期資料。PowerShell 產生的靜態 config JSON 同樣是過期快照。
模擬器的 JSON **格式**仍可參考，但不作為資料來源。

⚠ `HT9045_Debug.cmd` / `HT9050_Debug.cmd` 會啟動模擬器 —— 只適合看 UI 外觀。
驗資料用 `HT9045_Web.cmd`（http://127.0.0.1:8045）。

---

## FW 專屬要點

1. **目標**：dfm→web。`.dfm` 走 dfm2rc 既有 IR（解析層 133 表單已完成），
   新 emitter `emit_web.py` 產 `web/forms/<form>.layout.json`；表單業務邏輯忠實翻 C++，
   widget 讀寫改 form-core UI-state 欄位；狀態經 WebBridgeTags（liveness 必備）進瀏覽器。
2. **三件套可分波**：(a) 邏輯翻譯 (b) tag 匯出 (c) web 渲染，各自有 gate（計畫書 §3）。
3. **唯讀是硬邊界**：write path（指令通道）是安全關鍵，夜間永不做，撞到就佇列換標的。
   events handler 一律不接線。
   ⚠ **這一條限制的是「無人值守的夜間自動波次」，不是任何特定頁面。**
   20260916 訂正：`gen_wire.py` 在 20260914 自行寫下「HW.teach 在 FW 戰役政策裡標為
   永不自動接線」，隔天被日誌當成依據引用，於是 `HW.teach` / `HW.HandlerSys` 被擋了兩天。
   本檔從來沒有那句話。使用者在線上明確要求時，本條不適用——已於 20260916 完成接線
   （296 個欄位，`wire_probe.py` 逐鍵驗證 notFound=0、changed=0）。
   **通則：引用一條限制前，要指得出它在哪個治理檔的哪一行；自己產生的註解不是治理檔。**
4. **表單 ctor 高危**：form-core ctor 只塞欄位，初始化搬顯式 Init()——陷阱 #4 的
   18 個 NULL 全域在表單區最容易踩。
5. **web 專屬 gate**：e2e 探針（`tools/webprobe`）不與 ctest 並行（WB_TcpLink 撞埠）；
   ⚠ **20260908 更正（MW-1）**：原文是「`wb_publish`/`wb_serve` 一律 `--dry`」——
   **對 `wb_publish` 已無法執行**，它不再接受任何命令列參數（給了就 exit 2），
   而 `--dry` 想保證的事現在是結構保證：它只能載入 `%TEMP%` 副本，碰不到真 ini。
   **`wb_serve` 仍然要 `--dry`**，它不在 MW-1 範圍內。
   收工比對 `D:\HT9045\system` MD5+mtime；
   JS 無編譯器把關→防禦式渲染＋誠實回報「渲染回歸靠人工 F5」。
5b. **驗收 gate 一律用 `tools/dualgate.sh <tag>`（序列版），不要用 `dualgate2.sh`。**
   `dualgate2` 已於 20260827 停用：它把 Debug ctest 疊在 Release build 上，實測把
   `dfm2rc_fidelity` 從單獨跑 565.94s（Debug，通過）／355.37s（Release，通過）
   推過 600s 逾時，**兩側都假紅**，而換到的加速只有 2m25s／37m43（6%）。
   保留該檔只為留紀錄。**「多出 dfm2rc_fidelity 就重跑」不是解法，那是在替這個根因打補丁。**
   ⚠ 這條原本只活在會漂移的心跳提示文字裡（提示說 dualgate2、腳本檔頭說別用它），
   20260828 才落到這份 skill。判定一律由 `tools/gateverdict.sh` 給：
   **`ctest` 對任何失敗數都回 exit 8，exit code 零鑑別力**，只看
   `*_VERDICT` 與 `*_EXTRA`／`*_ABSENT`，並比對**失敗集合逐項**等於常駐五項。
5c. **gate 執行期間不准改任何進入建置的檔**（原始碼與 CMakeLists）。
   20260828 實測代價：波次 agent 在 gate 起跑 26 分鐘後回頭修正自己的行號引用，
   當時 Release 正在編譯 → 物件是改動前後的混合，**整次量測作廢重跑**。
   波次 agent 是在「它的檔案停止變動」時才算結束，**不是在它說結束時**。
   啟動 gate 前一刻記下交付檔 mtime，收工用 `build_<tag>g/cfg.log`（**起跑**錨點，
   不是 sentinel 寫入時刻）比對；任何一個較新就作廢。
5d. **⚠ 先問「golden 樹在不在」，再談耗時。**
   20260911 實測：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0` 的 `dfm2rc_fidelity`
   與 `dfm2rc_idempotent` 分別在 **0.77 / 0.66 秒**就失敗。
   **那不是逾時，是立刻死**，而下面整套探針規則的前提是「它們會跑很久然後可能逾時」——
   照那個前提去解讀 0.7 秒，方向會完全相反（我當時就先假設成 CPU/IO 競爭）。
   根因：`dfm_parse.py` 的 `_find_golden_root()` 回傳一個不存在的路徑，
   `os.walk()` 對不存在的路徑**靜默回空**，133 個表單無聲變成 0 個，
   然後死在兩個 frame 之外一個看起來毫不相干的 `FileNotFoundError`。
   golden 樹當時只在 `D:\HT9050`，不在 `D:\HT9045`。
   ⚠ **20260915 再次實測：`HT9011UC_Code_V3.33.906.0_20260618` 在 `D:\HT9045` 與
   `D:\HT9050` 兩處都不存在**，所以 `dfm_parse.py` docstring 裡「it is under D:/HT9050」
   那句已經過時。當時的處置是把 `GOLDEN_DIR_NAME` 改指
   `HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（134 個 `.dfm`，原 golden 為 133）。
   **後果要知道**：`dfm2rc_fidelity` 轉綠，但 `dfm2rc_idempotent` 改以
   `G7_BYTE_MISMATCH`（`cBinSel`／`cConfiguration`）失敗——那是**換了不同版本語料的
   直接後果，不是程式回歸**。要回到原語意，取回 906_20260618 那棵樹，或用環境變數
   `HT9045_GOLDEN_ROOT` 指過去（`dfm_parse.py` 本來就支援，不必改碼）。
   **已修**（`_find_golden_root()` 找不到就 raise + A 自己有一份副本），
   所以下面的規則前提重新成立。但判讀順序永遠是：
   **先看耗時是不是「異常地短」——短到不可能做完事，那是「根本沒跑」，不是慢。**

   **開 gate 前先花 20 秒量環境：單獨跑一次 `dfm2rc_idempotent`。**
   它是純 Python、`do_compile=False`（`run_b1d.py:505`），**零編譯、零 spawn**，
   所以它量到的就是 **build 目錄的檔案 I/O 成本**。
   ```
   build_<tag>r/tests/test_dfm2rc_pipeline.exe idempotent <build_dir>/dfm2rc_regen_idempotent
   ```
   **基線 17-25 秒。它是單向的：綠燈可靠，紅燈不可靠。**
   - **≤40 秒 → 直接開 gate**。128 次中有 105 次落在這裏，**其中只有 1 次 `dfm2rc_fidelity` 失敗**。
   - **>40 秒 → 意思是「未知」，不是「不要開」。**
     實測：>40 秒的 23 次裡 **fidelity 通過 16 次、失敗 7 次（偽陰性 70%）**；
     兩邊的 idempotent 區間幾乎完全重疊（通過 7.3-289.9、失敗 22.7-335.5）。
     此時**改直接量 `dfm2rc_fidelity` 本體**（單獨、無上限，2.5-11 分鐘）：
     ```
     build_<tag>r/tests/test_dfm2rc_pipeline.exe fidelity <build_dir>/dfm2rc_regen_fidelity
     ```
     **<450 秒 → 開 gate；>450 秒 → 等。** 直接量比代理量準，而且仍然比 80 分鐘的 gate 便宜。
   ⚠⚠ **這個門檻 20260910 從 550 收到 450，因為 550 的餘裕不夠。** 實測那天：
     單獨量 **539 秒**（只比 550 低 2%）照規則開了 gate，
     然後 Debug 腿的 `dfm2rc_fidelity` **在 gate 內破 600 逾時**。
     而同一場 gate 的 `dfm2rc_rc_compiles`，Debug 腿 **455.62s**、
     Release 腿 **187.47s** —— **差 2.4 倍**。
     **gate 內的負載波動遠大於 2%，所以門檻必須留餘裕，不能貼著預算走。**
     450 留的是 25%。⚠ 這不是說規則錯了 —— 它比舊版（「>40 秒就不要開」，
     照那個寫法歷史上會白白延後 16 次本來會綠的 gate）正確得多；
     錯的是**它缺一個餘裕概念**。
   ⚠ **這條規則第一版寫成「>40 秒就不要開 gate」，那是錯的**（寫完 90 分鐘後自己驗出來）：
     照那個寫法，歷史上會有 **16 次本來會綠的 gate 被白白延後**。
     **一個門檻在一個方向可靠，不代表它在另一個方向也可靠。**
   20260828 實測：92 秒（「未知」）-> 直接量 fidelity **363 秒通過** -> 開 gate。
   若照舊規則就會在這裏錯過一次可用的 gate 窗口。
   ⚠ **`dfm2rc_fidelity` 的基線是 ~150 秒，600 秒預算是它的 4 倍。**
   它偶發的紅燈**不是「預算太緊」**，不要去放寬 `tests/CMakeLists.txt:3246`；
   那只是把儀表關掉。實測波中真值 635 秒（`RC=0`、133/133、零問題），
   **只差 35 秒**；而同型的波會**自己過去**（08-27 六連逾時後 08:13 自行恢復）。
   `idempotent` 跟 `rc_compiles` 合看還能**分辨兩種不同成因的事件**：
   `rc_compiles` 偏高而 `idempotent` 正常 = **CPU 競爭**（重疊排程、失控行程）；
   `rc_compiles` 正常而 `idempotent` 爆掉 = **檔案 I/O 風暴**（端點防護 on-access 掃描）。
   詳見 `docs/DEVLOG.md` 20260828 VII。
   ⚠ **通則：連續更正四次通常不是運氣差，是基準沒有建立。**
   沒有基線時，每一個新樣本都會長得像一個新發現。
6. **選標的**：照計畫書 §4 佇列順序（FW-0 基建 → FW-1 tag 接線 → FW-2 產生器 →
   FW-3+ 表單波），每波開工以「使用頻率 × 唯讀可完成度」重評；單波 golden ≤15k 行，
   大表單切塊；`Command.cpp`（TfMain）記表單帳。
8. **頁面接線（Setup.*.html ↔ 配方）的抽取方法 —— 20260914/15 實證定案。**
   這是 FW-3+ 表單波每一頁都會用到的儀器，工具在
   工作副本在 `HT9011UC_Cpp_V3.33.906.0/scratchpad/`，**保存版（唯一權威）在**
   `D:\HT9045\.github\skills\ht9045-html-version\scripts\`：
   `extract_page_v2.py`（抽取）／`gen_wire.py`（產生）／`deploy_wire.py`（部署）／
   `kb_audit.py`（小鍵盤稽核）／`dfm_rename_diff.py`（V910↔V912 改名比對）。
   ⚠ scratchpad 未進版控也不在鏡射清單裡 —— v1 的 `extract_page_map.py` 就是這樣消失的。
   改工具請改保存版，再複製回 scratchpad 執行。

   8a. **⚠ 絕對不要「全樹依 widget id 反查」。** v1 這樣做，把 `Setup.TrayForm` 的
      `XCT1/XST1/XPitch1/YCT1/YPitch1/YST1` 註冊安到 `Setup.HotPlate` 頭上——兩頁有
      同名 id。照抽出來的結果接線，會把 HotPlate 的值寫進 `Tray.Data`。
      **這種錯不會報錯，只會寫錯檔案。**

   8b. **先鎖定表單**：用 `.dfm` 的 `object <id>:` 與頁面 id 的重疊度挑出「這一頁屬於
      哪個表單」，之後只從那個表單自己的 `.cpp` 抽。HotPlate 鎖到 `cHotPlate.dfm`
      （29 個 id 全重疊）。**校準證據**：v2 對 `Setup.Contact` 抽出 **47 個欄位，
      正是人工驗證的真值 47**（v1 只有 28）。

   8c. **對照表的最佳來源是 `WriteIniData` 那一側**，它把 (區段, 鍵) 與 widget
      寫在同一行，比一跳（HTEditList）與兩跳（ReadIniData→成員→widget）都可靠：
      `cHotPlate.cpp:598  WriteIniData(szDir, "Hotplate Form", "Name", HotPlateName->Text);`
      讀取端 `:162-170` 用同一組 (區段, 鍵)，兩側可互相佐證。
      `HTEditList` 一跳法仍要保留——`cTrayForm`／`cConfiguration`／`cLd_ULd` 只有它，
      而且它多帶 C++ 型別（`ECPosInt`／`ECDouble`）。
      `cContact.cpp` 兩者皆無（`el*->Add(` 是 0 個）且區段名是執行期變數 `S`，
      **純正則解不了**，所以 Contact 是最壞情況，不適合當召回率的標準。

   8d. **只接「所有配方都有」的鍵。** 排序主鍵是鍵的存在性，不是欄位數量——
      欄位數量的是工作量，不是可用性。分母只算**擁有該文件**的配方
      （`HotPlate.Data` 在 216 個配方裡有 215 個，用 216 當分母永遠不會滿）。
      不安全的鍵一律進 PENDING 附 n/denom：送出去就會讓 preview 回 `notFound`，
      依規則 2 整頁拒寫。**寧可少接，不要讓整頁壞掉。**

   8e. **小鍵盤旗標也不要猜**：抽 `.cpp` 的
      `ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)` 配上 `.dfm` 的
      `OnMouseDown = <handler>`，與 `HTQwerty.show(target, flags, {dp,checkRange,min,max})`
      參數一一對應。⚠ **min/max 是 C++ 執行期變數時（`InputLimit.dContactHigh` 之類）
      一律關掉夾限，不要填猜的數字**——猜錯的夾限會擋掉合法輸入或放過非法輸入，
      比沒有夾限更危險。Contact 42 個鍵盤裡只有 5 個有硬編上下限。
      機台原本就有 `web/page/qwerty.js`，**不要另造一套**。

   8f. **共用引擎，不要每頁複製**：`web/page/ht9045_wire_engine.js` 提供 load/save/鍵盤，
      每頁只提供資料（`ht9045_wire_<slug>.js`）。理由很實際——「狀態列蓋住 Save 鈕」
      這個 bug 在複製式寫法下踩了兩次。
      ⚠ **狀態列絕對不要用 `position:fixed; bottom:0`**：這些頁的 Save/Exit 多半在底部
      Panel（`cHotPlate` 的 Panel2 在 `top:475px`），橫跨底部的固定列會把它們整個蓋掉，
      而且是看不到也按不到。表單寬 606px，停靠 `left:614px` 的右側空白區。

   8g. **20260915 成果**：Speed 65 / Temp_Set 56 / TrayForm 29 / Contact 35 / TesterIF 15
      / HotPlate 7 個欄位可讀寫；16 頁共 825 組小鍵盤設定。
      `Config.Configuration` **不是配方頁**（1011 個欄位裡 1008 個在任何配方檔都不存在，
      它讀的是機台層 `config.ini`），只給鍵盤。

7. **完成度三軸不可換算**（census 行數／活 tag 數／layout.json 覆蓋數），
   引用必附分母與單位。

## `/api/system/` —— 五個機台設定檔的即時讀寫（20260915 新增）

`wb_serve.exe` 新增一條路由與一個指令，取代 PowerShell 一次性產生的 JSON 快照。

```
GET  /api/system/          五個檔的索引（name / path / kind / available / bytes）
GET  /api/system/<name>    ini  -> 與配方文件完全同形狀 {sections:{sec:{key:{value,type,raw}}}}
                           csv  -> {columns:[...], keyColumn, rows:[{col:val}]}
WS   system.file.put       tag=<name>  value=JSON 字串
                           ini -> {"sections":{sec:{key:{"raw":"..."}}}, "dryRun":bool}
                           csv -> {"rows":{rowKey:{col:"raw"}}, "dryRun":bool}
                           ack -> {changed, identical, notFound, backup}
```

| name | 檔案 | 格式 | 路徑來源（golden 全域） |
|---|---|---|---|
| `gerneral` | `system\Gerneral.ini` | ini | `asGeneralPath`（common.cpp:89） |
| `teach` | `system	each.ini` | ini | `asTeachPath`（:239） |
| `motTable` | `system\Mot_Table.csv` | csv | `MotTablePath`（:167） |
| `ioTable` | `system\IO_Table.csv` | csv | `IoTablePath`（:166） |
| `config` | `config\config.ini` | ini | `AuthPath`（:102）＋ `"config.ini"` |

**四條設計約束，改這段程式前先讀**：

1. **固定五筆表，不做目錄掃描。** `system\` 是共用量產設定；掃描會在有人丟新 ini 進去的當下自動暴露它。五筆寫死，路徑穿越在結構上不可能。
2. **路徑一律取 golden 全域，不寫死字串。** 好處是 `--dry` 對 `asGeneralPath` 的重導向自動生效。
3. **寫入要 `--allow-system-write`，`--allow-cmd` 不夠。** AGENTS.md：「共用量產執行期參數。預設唯讀。要寫必須先備份，且要人明確同意」——這個旗標就是那個明確同意。
4. **逐位元組保留 + 備份 + 原子置換。** ini 沿用 `ht9045::RecipeDocApplyEdits`（它吃任意路徑）；csv 是新寫的 `CsvApplyEdits`，只換指名那一格，行尾與其他欄位原樣複製。備份為 `<path>.bak_<時間戳>`。

⚠ **`--dry` 只保護 `Gerneral.ini`**，因為只有 `asGeneralPath` 被重導向到暫存副本。`teach.ini`、兩個 csv、`config.ini` 帶 `--allow-system-write` 寫下去就是**寫真檔**（有備份，但是真檔）。

⚠ **`teach.ini` 特別小心**：`forms/fTeach.h` 記著這個專案已經因為一條**無聲失敗**的 teach 寫入路徑遺失過教導資料一次。所以 ack 一定帶 `changed/identical/notFound`，寫不中任何鍵會回報 `notFound` 而不是默默成功。

⚠ **`config.ini` 的寫入是新開的路徑**，不是移植既有的。golden 自己在 `cConfiguration.cpp` 的 `#if 0 // GATE (CFG4-seed)` 把寫入關掉了——那個閘門針對的是**啟動時無人值守補鍵**，不是操作員按存檔，但仍要知道這件事。

## 夜間 loop 與額度

- 每波收工＝commit + DEVLOG + 🔖RESUME，然後**立即開下一波，不准閒著結束回合**。
- ScheduleWakeup 當保險 + cron 心跳（20 分鐘守衛式）雙保險；
  額度中斷後 cron 自己回來續跑，**前提 session 沒關**。
- session 死了：磁碟上的 commit + RESUME 就是全部狀態，使用者重開後
  `/loop /fw-wave` 冷啟動即無損接續。
