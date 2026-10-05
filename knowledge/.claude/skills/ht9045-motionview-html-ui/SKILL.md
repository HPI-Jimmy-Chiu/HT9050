---
name: ht9045-motionview-html-ui
description: 用 HTML 做出可取代實機 Motion View 的頁面（HT9045 / HT9046 Handler）。參數不寫死：頁面 runtime 直接讀機台的 Gerneral.ini / teach.ini / Tray.Data / HotPlate.Data / TestMode.Data / ArmCondition.Data，連站點與行程幾何都從 teach.ini 推導，再依真實程式碼的配位與互鎖規則跑模擬（SIM）；另有 LIVE 模式讀 StateRecord 的 MainFormSnapshot.txt / Motor.xls / HP*_*.xls，直接顯示實機當下的各軸位置、各 Task 與各處 IC 在籍。當使用者要求「取代 Motion View / 機台動作模擬畫面 / 整機跑料動畫 / 吸嘴取放料順序 / HotPlate 配位視覺化 / 看實機當下狀態 / 給客戶解釋機台動作的 HTML」，或要換機台換 recipe、加模組、排查配位與互鎖缺陷時使用。
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-motionview-html-ui，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 Motion View HTML UI

## Overview

一個**參數化的單檔 HTML 模板**，把實機 recipe 讀成參數後，用 SVG 畫出整機三維動作、
每一格料件的在籍狀態、吸嘴變距開合，並附一條可拖曳的甘特時間軸。
目標是**取代實機 Motion View**：現場看得懂、工程能拿去解釋，且參數換掉就換機型。

## HT9045 HTML 部署規則（2026-09-08）

`D:\HT9045\page\Main.MotionView.html` 是目前的整合頁，以下規則優先於本 skill 中保留的獨立模板／SIM／StateRecord 範例：

- HTML 僅透過 `HTSettings` 讀取 `JSON/`；不得直接解析 `.ini`、`.Data`、StateRecord 或 Excel。
- 退料 Tray（Auto1~6、Fix1~6）使用單一 tab 容器。僅當 `Production-update.json.state.motionView.outArm.phase === "placing"` 且 `targetTrack` 是已安裝 tray key 時，才自動切換對應 tab；不可由 OutArm 座標推論目標。
- `Setup-current.json.documents.hotPlate.sections["Hotplate Form"]["Using Flag"]` 投影 fHotPlate：bit `0x01` 為 `cbEnableHP1`／Plate1，bit `0x02` 為 `cbEnableHP2`／Plate2。畫面僅顯示啟用 Plate 的教導位置。
- Auto Clean 的設定來自 `Setup-current.json.documents.handlerCondition.sections.Configuration` 的 `iAutoClean_Function`、`iAutoClean_Mode`、`iAutoClean_Tray`、`iAutoClean_IntervalContact`、`iAutoClean_CleanCount`；執行中狀態只認 `state.motionView.autoClean.active === true`。
- 未發布 Tray、OutArm 或 Auto Clean runtime 值時必須顯示未知／等待，不得建立模擬 occupancy 或推導機構狀態。

完整 producer 欄位以 `JSON/Runtime-bridge-contract.json` 的 `motionView` 為準；變更 JSON 後必須重新執行 `_gen_json_shim.py`。

模板不是靜態示意圖 —— 它內建**排程器 + 配位演算法**，動作順序是算出來的，
所以任何配位缺陷（漏料、吸嘴沒吸滿、Shuttle 停位不符）都能被機器驗出來。

## 快速開始

**參數不寫進頁面。** 複製模板、用瀏覽器打開、把機台檔案載進去就換好機台了：

```bat
copy assets\motionview-template.html "<輸出路徑>\<檔名>.html"
rem 用瀏覽器打開 → 按「📂 載入機台檔案」或直接把資料夾拖進頁面
```

要載的是**機台本機的兩個資料夾**（每台機都有，不需要 StateRecord）：

| 資料夾 | 裡面用到的檔 | 供什麼 |
|---|---|---|
| `D:\HT9045\system\` | `Gerneral.ini`、`teach.ini` | 變距行程／Y 變距有無／**全部站點與行程幾何** |
| `D:\HT9045\IniData\Data\<Device>\` | `Tray.Data`、`HotPlate.Data`、`TestMode.Data`、`ArmCondition.Data` | 盤形、吸嘴排欄、Pitch 模式 |

**StateRecord 只在要看「某台機當下狀態」（LIVE 模式）時才需要** ——
`MainFormSnapshot.txt` / `Motor.xls` / `HP*_*.xls` 只有 StateRecord 才有。
什麼都不載時，頁面用內建預設值（HHT-139）照樣能跑。

改完模板本身（不是換參數）才要跑驗證：

```bat
node assets\smoke-test.js "<你的 html>"
python D:\HT9045\.claude\skills\make-report-skill\scripts\verify_animation_html.py "<你的 html>"
```

> 20261005 St01 查：`verify_animation_html.py` 在 `D:\HT9045\.claude\skills\make-report-skill\scripts\`、舊的 skill 位置都**不存在**，這一行目前跑不起來；先只跑 `smoke-test.js`。

輸出路徑依 `Make-Report-Skill` 的動畫類路由：
`D:\00_ReleaseNote\{代理商或廠內}\{客戶}\{proposals|bug-reports}\{YYYY}\{YYYYMMDD}_{作者}_{機型}_{主題}_{受眾}.html`
**單一獨立 HTML，不產 .md、不呼叫 `md_to_html.py`。**

## 工作流程

### 步驟 1 — 參數層：頁面自己讀，不要把值烤進頁面

⚠ **這是踩過的架構錯誤**：一開始我做成「用腳本從機台檔案產出一段 JS，人工貼進 HTML」——
那是把參數烤進畫面裡，換一台機就要改一次程式碼，不叫模擬。

正確做法：**頁面 runtime 直接讀機台檔案，依讀到的參數跑模擬**。
模板已內建 parser（`parseIni` / `parseBiff2` / `parseSnapshot`），
`applyParams()` 是唯一的參數入口，`applyTeachGeometry()` 連**幾何**都從 `teach.ini` 推。
`scripts/extract_params.py` 現在只是**離線核對工具**（`--report` 給人看推導依據），不參與正常流程。

必須釘死的六件事（頁面會自己讀，但你要知道它讀哪裡、讀不到會怎樣），
全部見 [references/parameter-contract.md](references/parameter-contract.md)：

| 參數 | 為什麼一定要先確定 |
|---|---|
| `TRAY` 欄列數 / Pitch | 決定合法變距倍數 k，直接決定下針次數 |
| `HPF` 欄列數 / Pitch | 決定 `spacX`、`iYHalf`，決定配位游標與**可用容量** |
| `ARM` 排欄數 / 兩排 Y 間距 | 決定「一次能不能兩排一起放」 |
| `PIT` 變距教點 | 決定 mm ↔ pulse 讀數與機構上下限 |
| `YP` 有無 Y 變距 | 有／無會讓總趟數差一倍 |
| `SITE_MAP` | 吸嘴 ↔ Site 對應；錯了會漏料（Kit 槽位撞號） |

### 步驟 2 — 決定要畫哪些模組

模板已內建：Loader / HotPlate ×2 / Precisor / AutoClean / Shuttle ×2（三站雙 Kit）/
Index ×2 + Socket / OutArm / Fix ×6 / Auto ×6 / Tray Arm。
機型沒有的模組**刪掉不要留空框**，但**互鎖區（Zone A~H）要留**，
那是使用者判斷「為什麼卡住」的依據，見 `ht9045-motor-spatial-layout`。

### 步驟 3 — 配位與取放料規則

不要自己推導順序，照 [references/placement-algorithms.md](references/placement-algorithms.md)。
權威來源是程式碼，**模擬畫面／影片只是佐證**（誤讀顏色會讓整個水位模型歪掉）。

三條最容易做錯、且錯了看不出來的規則：

1. **吸滿才放** —— InArm 一定把吸嘴吸滿才去 HotPlate；只有 Tray 已空的尾批才允許部分放。
2. **滿手一定兩排一起放** —— 分拆放下去會生出「單排 4 顆」的 team，
   而吸嘴兩排固定差 `iYHalf` 列 → 之後永遠湊不回 8 顆一起取。
   代價是配不到對的那幾列用不到（可用容量 < 盤面格數），這是**幾何必然**，不是缺陷。
3. **HotPlate 取料是帳本不是幾何搜尋** —— 實機 `SearchPlateToPick()` 走
   `PickFromHPList->GetHPFirstTeamPlate()`，**取料群組 == 放料群組**。
   所以「放料時怎麼分組」就決定了「取料時能不能取滿」。

### 步驟 4 — 顯示語意要與實機一致

見 [references/display-semantics.md](references/display-semantics.md)。最關鍵三點：

- `HAS_NULL_IC` **是白色**，和「從未放過料」同色，不要另外配顯眼顏色
- 「已達 soak time」用**原色 + 紅色外圈**表示，不要換色系
- 不透明盤面必須畫在逐格點**之前**：`gS → gDot → gD → gT`

### 步驟 5 — 驗證（不可省）

見 [references/verification.md](references/verification.md)。
這類頁面的 bug **看不出來** —— 動畫照樣會跑，只是料件悄悄消失或機構做出不可能的動作。
`assets/smoke-test.js` 用 DOM stub 在 node 裡把每個機型組合的整條時間軸跑完，
驗守恆、容量、動作者互斥、Socket 互斥、依賴滿足、吸滿才放、列優先、
幾何一致、Shuttle 停位互鎖、以及「盤上足量卻未取滿」。

改參數後 op 數會變，**斷言若是為舊模型寫的就要一起改** ——
但先確認是斷言過時，不是實作退步。

## 架構約束（改模板時務必遵守）

| 約束 | 原因 |
|---|---|
| 狀態只能透過 `move` 物件 `{fk,fi,tk,ti,v,sht,at}` 經 `doMove()` 變更 | 料件守恆變成結構性保證，不必到處補檢查 |
| 快照是依 `tau`（`a + 0.55*dur`）排序 replay 出來的 | **build 順序 ≠ 時間順序**；「先清空再填」必須寫成 `deps` |
| 需要機構在特定停位的動作要標 `needPh:[軌號,停位]` | 否則會出現「Shuttle 還沒到左邊，InArm 就放料上去」 |
| 同一停位能做完的事要批次化 | 否則 Shuttle 每個動作各要求一次停位，左右狂跑，實機不可能 |
| 吸嘴頭用 `drawHead(H,...)` 傳 store，不要寫死 id | InArm（左下）與 OutArm（右上）共用同一段繪製 |
| 參數區集中在檔頭 `0-B`，其餘程式不得再出現硬編數字 | 換機型只改一處 |

## 兩種模式

| 模式 | 資料來源 | 做什麼 | 用在哪 |
|---|---|---|---|
| **SIM** | 機台的 recipe ＋ `teach.ini` | 依參數與程式碼規則**推算**整條動作序列，可播放／拖時間軸 | 解釋動作、比較設定改動、找排程互鎖問題 |
| **LIVE** | StateRecord 的 `MainFormSnapshot.txt` ＋ `Motor.xls` ＋ `HP*_*.xls` | **不推算**，直接把讀到的當下狀態畫出來 | 取代 Motion View：看實機此刻在哪、各處有沒有料 |

LIVE 的資料契約、已對接／未對接的項目，以及要補齊需在 handler 端加什麼，
見 [references/live-mode.md](references/live-mode.md)。

## 常見需求對應

| 使用者說 | 做什麼 |
|---|---|
| 「換這台機器的設定」 | 頁面上載入該機的 `system\` ＋ `IniData\Data\<Device>\`（**不改程式碼**）|
| 「看實機現在的狀態」 | 載入該機的 StateRecord → 切 LIVE |
| 「加上 Y 變距看看」 | 參數 `YP.teachOK=true`；模板已有 `cfg.ypitch` 切換鈕 |
| 「吸嘴順序不對 / 只取一半」 | 先跑 smoke-test 看是哪條不變量掛，再查 placement-algorithms §2、§3 |
| 「盤上料一直卡在後半段」 | HotPlate 取料改 FIFO（`hpAt` 最小），不可用列號升序 |
| 「Shuttle 左右移動太頻繁」 | 批次化：把同一停位能做的事集中，OutArm 取料延後到同一次右移 |
| 「某個模組被遮住」 | 圖層順序：不透明盤面搬到 `gS`，逐格點在 `gDot` |
| 「要給客戶看」 | 語言以既有同主題對外文件為準；時間軸標「示意，非實測」，不要放 UPH |

## Resources

- `assets/motionview-template.html` —— 完整可跑的模板（HT9045W / 2x4 8-site / 8×16 HotPlate 實例）
- `assets/smoke-test.js` —— DOM stub 全時間軸驗證器
- `scripts/extract_params.py` —— **離線核對工具**（`--report` 列推導依據）；正常流程不需要，頁面自己會讀
- `references/parameter-contract.md` —— 參數來源、推導、合法值域
- `references/placement-algorithms.md` —— 取放料配位（對照程式碼）
- `references/display-semantics.md` —— 顏色與標籤語意
- `references/live-mode.md` —— LIVE 模式資料契約、缺口與 handler 端補 dump 提案
- `references/verification.md` —— 不變量清單與已知缺陷
