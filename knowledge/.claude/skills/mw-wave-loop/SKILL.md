---
name: mw-wave-loop
description: "HT9045 V906 MON1203-WEB 戰役（讓 1203 的 IO 與 Motor 在 web 上真的可監控）的自動波次政策。定義最終目標、MW-1~MW-6 佇列與各自的前提、驗收 gate、三個 repo 的邊界、真機硬邊界、停止條件。Use when：執行 /mw-wave、規劃 1203 監控波次、判斷某個 1203 波次的前提成立了沒、要引用 1203 現況數字。關鍵字：MW-W, MON1203, pci1203, 1203 監控, EtherCAT 從站, ENI, Acm_DevLoadConfig, exe_startup_gate, x64 heap corruption, byIdMode, slavesFound"
---

# MON1203-WEB 戰役政策

權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/MON1203_WEB_CAMPAIGN_PLAN.md`。
本檔只放**政策**（怎麼選、怎麼判、什麼不做）；量到的數字與交付明細在計畫書。

## 最終目標

使用者 20260908 原話的目標：「這台是機台端PC，我的目的是要先測試 1203 的監控，
包含 IO 和 Motor」。翻成可驗收的條件：

1. web 首頁有入口，點進去看得到 1203 的**卡片／Motor／EtherCAT 環網／IO** 四層狀態。
2. 顯示的每一個值都**可以被追溯到一次真實的唯讀讀取**，而讀不到就誠實顯示讀不到。
3. **監控永遠不能變成控制** —— 唯讀是機械強制的，不是承諾。
4. 開關走 `MachineType.h` 的 `#define`，不走命令列旗標（使用者 20260908 裁決）。

★ **第 1 與第 4 已完成（MW-1／MW-2）。第 2 的「有值可顯示」卡在硬體，見下。**

## 佇列與各自的前提 —— 這是本戰役最重要的一段

⚠ **本戰役大部分剩餘工作的前提不在程式裡，在硬體上。**
「前提不成立就跳過」不是偷懶，是因為**硬做會產出看起來權威而實際錯誤的畫面**。

| 波次 | 狀態 | 前提 | 前提怎麼量 |
|---|---|---|---|
| MW-1 旗標→#define | ✅ 20260908 | — | — |
| MW-2 web 監控頁 | ✅ 20260908 | — | — |
| **MW-6 軌道 A** startup gate | **可做，最有價值** | 無 | — |
| MW-6 軌道 B 找越界寫入 | 等軌道 A | 要先有紅燈才判得出「修好了沒」 | — |
| MW-3 馬達名上 wire | ⏸ 等硬體 | `axScan.byIdMode==true` 且 `axesOpened>0` | 跑一次 publisher 看它印的那行 |
| MW-4 IO 位元命名 | ⏸ 等硬體 | `scan.found>0` | 同上 |
| MW-5 `wb_serve` 旗標 | 可做 | 無；或併入 FUSE（計畫書 D6） | — |

**20260908 實測基線**（要引用就重量，不要抄）：
`subDevices=0`、`axesOpened=0`、`slavesFound=0`、`axByIdMode=NO`、
`mode=owned`、`disabled=true`（連續 10 次全失敗後自我停用，**那是設計上的安全閥**）。

### 為什麼 `byIdMode=false` 時不准顯示馬達名

只有 `Acm_AxOpenbyID` 開出來的軸，其 `(boardId, port)` 才對得上
`Mot_Table.csv` 的列。實體索引開出來的軸配上馬達名，是**拿一個真實的編碼器讀值
配上錯的名字** —— 「InArm Z 在 12.5」而其實是別的軸。
**那比空白危險**，因為空白會讓人去查，錯的名字不會。

## 1203 現在缺什麼：三層，各自獨立

計畫書 §3.5 有完整版。政策上只要記住**不要把三層混成一句「還沒設定好」**：

1. **硬體**：1203 是 EtherCAT **主站**，軸與 IO 在從卡的 EtherCAT 埠串出去的**從站**裡。
   卡插著本身永遠不會有軸。⇒ **需要外接模組。**
2. **設定**：`system\` 沒有任何 1203 的 ENI／`.cfg`，`Gerneral.ini` 沒有對應 key，
   而 **`Acm_DevLoadConfig` 全樹沒有呼叫點** —— 放了檔也不會被讀。
   ⚠ **但 ENI 不是「掃不到從站」的原因**：主站沒有 ENI 也能掃匯流排，那正是產生 ENI 的方式。
   所以 `slavesFound=0` 指向第 1 層。
3. **資料表**：`Mot_Table.csv` 的 `CardModel` 45/45 全 SMC、零筆 PCI1203；
   `IO_Table.csv` 的 `ISABase` 零筆是 3。⇒ 就算從站接好，這棵樹也不會把軸交給 1203。

**判別硬體 vs 軟體的唯一乾淨方法**（工具已裝在這台）：
`C:\Program Files (x86)\Advantech\Common Motion\Utility\Common Motion Utility.exe`。
它也看不到從站 ⇒ 第 1 層，與我們軟體無關。它看得到而我們看不到 ⇒ 才是軟體問題。

## 三個 repo，分開 commit

| 路徑 | 版控 |
|---|---|
| `HT9011UC_Cpp_V3.33.906.0` | ✅ 自己一個 repo（C++、`docs/`、`tools/`） |
| `D:\HT9045\web` | ✅ **自己一個 repo**（頁面、`js/`、`tools/jsprobe/`） |
| `D:\BCB6_1203_UI` | ❌ 不是 repo |
| `D:\HT9045`（根） | ❌ 不是 repo（`CLAUDE.md`／`.claude\`／`installer\` 都不受版控） |

⚠ **一次 `git status` 看不到全部。** 每波收工逐 repo 檢查並分別 commit；
不受版控的改動要在 DEVLOG 裡寫明改了什麼。
⚠ `installer\ai\` 是「複製回專案根」的交付快照，**改了 `CLAUDE.md`／`.claude\`
就要同步它**，否則將來有人照 `installer\docs\06` 的指示還原，會把安全更正覆蓋掉。

## 驗收 gate（每波必跑）

`pci1203_readonly_gate` / `macro_order_gate` / `pe_truncation_check` /
`production_audit`（**跑前先 `-Snapshot`**）/ `devlog_resume_gate` /
`web\tools\jsprobe\run.ps1 -All`。
⚠ **20260909（FW-1h）起是 `-All`，不是 `-Probe <相關的>`** ——「相關」沒有定義，
所以那個寫法**不可能變紅**。`-All` 自己發現全部 7 支、印總計、
產出可貼進 DEVLOG 的一行，且「0 支被發現」算失敗。

**三條容易被跳過而付過代價的**：

* **動到 `#ifdef` 就把每個組合都編一次。** 預設關閉的臂從沒被型別檢查過。
* **`exit 0` 不是產物完整的證據** —— 被 kill 的連結器留下的截斷 exe 照樣跑、
  照樣印 ALL PASS。`pe_truncation_check` 才是。
* **產物新鮮度用內容證明**（新字串在、舊字串消失），不要只看 mtime。

⚠ **e2e 一律用 32-bit 線**（`build_nonoracle`）：x64 的 `wb_publish.exe` 有
`main()` 之前的 heap 破壞，觸發條件是環境區塊多幾個位元組。在 x64 上跑 e2e 會得到
「gateway 起來、HTTP 200、`feed=down`」，那不是這一波弄壞的。
⚠ **煙霧測試的判準是「持有埠的 PID 等於你剛啟動的那一支」**，不是 HTTP 回 200。

## web 波的品質規則

* 邏輯是**純函式**（tag Map → DOM），接線留在頁面檔。
  判準：**探針要補幾個 stub 才碰得到邏輯？>0 就是黏在一起了。**
* fixture 用**真實擷取**（從 8046 抓），不要手寫。附帶好處：可以做
  「頁面讀的每個 tag 名稱都必須存在於 wire 上」這道拼字 gate ——
  拼錯的 tag 渲染成 `n/a`，看起來像 publisher 的問題。
* **三種顯示狀態不是兩種**：`n/a`（不在 feed 上）／`---`（在、但 null）／值。
* **null 永不畫成 0**，⚠ 而且**真正的 0 仍要顯示 0** —— 會藏掉真 0 的規則更糟。
* **對照組要驗它真的會綠**：一個永遠警告的橫幅，其他斷言照樣全過。
* **零控制元件要機械斷言**（走過 DOM 數 button/input/listener），不是寫在註解裡。

## 硬邊界（不做、不問，跳過並記錄）

* `GATE G9`（`csystem.cpp:16913`，馬達煞車）與 `MachineType.h:48` 的 `SOFT_SIMULTE`。
* 不為了讓畫面有數字而改 `Mot_Table.csv` 的 `CardModel` 或 `IO_CARD_TYPE`。
* 不打開 `INSTALL_ETHETCAT()`（同時武裝 shuttle sensor 與 vacuum 的**輸出寫入**）。
* 唯讀白名單不得加入任何會下命令的進入點；`Acm_AxResetError` 看起來像讀取而不是。
* 不自行改 F5 預設線（x64 取代是使用者 20260902 的裁決）。
* 不用「把環境變數加長」當 x64 缺陷的修法。
* 不在 `build\`／`build_x64`／`build_nonoracle` 裡做實驗。
* golden 樹唯讀（Big5）；`D:\HT9045\EXE` 永不寫入。

## 停止條件

* 所有前提成立的波次都做完，只剩等硬體的 → **停**，報告在等什麼、
  以及使用者要做的那一個動作（掃匯流排）。
* 同一根因連紅兩次 → 停、換標的、記錄。
* 需要碰硬邊界才能推進 → 停，把裁決點寫清楚交還使用者。
* **絕不把「監控頁面有數字」講成「1203 可以用了」**，也絕不把
  「模擬跑起來」講成「機台跑過了」。
