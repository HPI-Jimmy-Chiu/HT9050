# 頁面命名規則、Release 模式與離線 JSON 寫入

> 原始碼皆為 **BCB6**（VC++ 版需另行標注）。

## 1. 頁面檔名規則（2026-09-02 重新命名 48 檔；同日下午兩次重新分類共 15+9 檔）

| 規則 | 說明 | 例 |
|---|---|---|
| ① 去掉 dfm 表單前綴字母 | `c*/u*/f*` 一律去掉；`fMain.X` → `Main.X`；`uhome`/`uteach` → `home`/`teach` | `cBinSel`→`BinSel`、`fMain.MotorView.html`→`Main.MotorView.html` |
| ② `Setup.` | 工作檔範疇（cpp 存取 `D:\HT9045\IniData\Data`） | `Setup.BinSel`、`Setup.BarCode`、`Setup.Contact` |
| ⑥ `Config.` | 機台功能或介面設定畫面 | `Config.Configuration`、`Config.DIOInterFaceCFG` |
| ③ `Data.` | 生產數據／紀錄顯示 | `Data.SortCT`、`Data.ContactCT`、`Data.StartCondition`、`Data.CounterClear`、`Data.Builder` |
| ④ `Status.` | 狀態顯示 | `Status.ShowMessage`、`Status.GroundMan`、`Status.CounterSel`、`Status.Security` |
| ⑤ `HW.` | 硬體直接操作（馬達／IO／溫控／感測器／機台系統設定） | `HW.MotorTest`、`HW.home`、`HW.teach`、`HW.HandlerSys` |
| ⑥ `IDE.` | 開發輔助工具頁（非實機畫面） | `IDE.ComponentMap`、`IDE.I18nEditor`、`IDE.StyleGuide`、`IDE.WidgetTemplates` |
| 不分類 | `Main.*`、`Main.html`、`ScreenShots.html`／shot/* | — |

**最終分類（2026-09-02，共 44 檔有分類前綴）**：

| 類別 | 頁面 |
|---|---|
| Setup.（21） | OffSet、Speed、Configuration、QAMode、BarCode、Cleaning、Contact、TesterIF、Ld_ULd、TrayForm、SCK_ART、YieldMonitoring、HotPlate、SetUp、Temp_Set、BinSel、BinSelNormal、TrayAssignment、DIOInterFaceCFG、**ContactForce**、**AGV** |
| Data.（9） | SortCT、ContactCT、TestCategory、StartCondition、LotInfo、Observer、SmartDiagnostic、CounterClear、Builder |
| Status.（8） | ShowMessage、LtcSensor、ShowBinSelect、GroundMan、TowerLight、TemperFrom、CounterSel、Security |
| HW.（8） | OmronEJ1N、MyCCLinkSensor、MotorTest、home、IoSetView、teach、HandlerSys、**VacuumUnit** |
| IDE.（4） | ComponentMap、I18nEditor、StyleGuide、WidgetTemplates |
| 無前綴 | Main.×11、main、ScreenShots |

> **//Steven 20260919** — Setup. 加 `ContactForce`、`AGV`（19→**21**）；HW. 加 `VacuumUnit`（7→**8**）。
> Data.（9）／Status.（8）／IDE.（4）未變。當日完整變更紀錄：`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260919_Steven.md`
>
> ⚠ **這張表是「分類規則的登錄表」，不等於 `page\` 目錄的檔案數**，對不起來是正常的，原因有三：
> ① `Setup.Configuration` / `Setup.DIOInterFaceCFG` 已於 20260915 改名進 `Config.*`（本表仍列在 Setup. 列），
>    所以磁碟上的 `Setup.*.html` 會比表中少 2 個；
> ② `HW.ShuttleMove`（20260918）與 3 個 `IDE.MotionView9050-*` 頁尚未回填本表；
> ③ `Alert.`（3）只列在 SKILL.md 原則 0，不計入本表。
>
> 要以磁碟為準時請直接數 `D:\HT9045\web\page\`，**不要把這張表的數字當量測結果引用**。

判定工作檔範疇的輔助掃描：`D:\AI_TempFile\_scan_datapath.py`（cpp 內 `DataPath` / `GetLastOpenFN` / `*.Data` 命中數）；最終分類以使用者指定為準。

### 重新命名工具 `D:\AI_TempFile\_rename_pages.py`（可重跑）
- 以**基底名**（先去 `Setup./Data./Status./HW./IDE.` 再去 dfm 前綴字母）查 `SETUP / DATA / STATUS / HW / IDE / KEEP` 集合 → 決定新名；改分類只需改集合再跑一次。
- dry-run 列出 map 與引用數，`--apply` 才改檔名並替換引用。
- 引用替換範圍：`D:\HT9045\*.html`、`page\*.html|*.js`、`page\shot\*.html`、`D:\AI_TempFile\_gen_*|_scan_*.py`、skill SKILL.md／references／scripts、`docs\ops\daily`、`<入口網站 repo>\public\Docs\Daily\Steven\20260902.md`。
- 以一次性 regex alternation（長鍵優先，`(?<![\w.])` 前綴保護）避免鏈式替換（例：`cSetUp`→`Setup.SetUp` 不會再被吃）。
- **產生器 `JOBS` / `NO_OVERWRITE` / `PAGE_EXTRA` / `SUBTREE_JOBS` 的輸出檔名同時被替換**，重跑 `_gen_dfm_abs.py` 即輸出新檔名；ComponentMap 章節 anchor 不變。
- background `WINDOWS[].id` 不變（`binsel`、`motortest`…），只有 `src` 改名 → Main.html `DFM_MAP`／`View-rules.json` 不受影響。

### Teaching 預設分頁
`HW.teach.html`（手工頁）最外層 `PageControl2` 預設進 **`tsAxleCtrl`**（Axle Control）：`_teach_default_tab.py` 把 `tsIndex` 頁籤 `act`／pane `display:block` 移到 `tsAxleCtrl`；產生器 `DEFAULT_ACTIVE['uteach.dfm']={'PageControl2':'tsAxleCtrl'}` 同步登錄供重生一致。

## 2. Release 模式（只顯示 HTML 本體）

| 層 | 做法 |
|---|---|
| 頁內（`page/theme.js`，release 分支） | `contextmenu` / `auxclick` / `dragover` / `drop` / Ctrl+wheel 一律 `preventDefault`；`keydown` 封鎖 F1~F12、Ctrl／Alt／Meta 組合、Escape（編輯框內保留 Ctrl+C/V/X/A/Z）。每個 iframe 各自掛（theme.js 每頁都載） |
| 桌面（`background.html`） | `html[data-mode="release"] #taskbar{display:none}`；`#desktopWrap` 藏捲軸（仍可滾輪捲動） |
| 瀏覽器（固定 **Microsoft Edge**） | `D:\HT9045\HT9045_Release.cmd`：`msedge --kiosk <url> --edge-kiosk-type=fullscreen`，獨立 `--user-data-dir=%LOCALAPPDATA%\HT9045_Edge_Release`，`--disable-pinch --overscroll-history-navigation=0 --noerrdialogs --disable-session-crashed-bubble`。Kiosk 全螢幕無網址列／分頁列，並停用大部分瀏覽器快速鍵（Ctrl+T/N/W、F11 等）。Alt+F4／Win 鍵屬 OS 層，需另以 Windows 指派存取（Assigned Access）或群組原則鎖定 |
| Debug | `HT9045_Debug.cmd`：一般 Edge 視窗（保留 F12／右鍵／工作列），`--user-data-dir=...HT9045_Edge_Debug` |

驗證：release 下 `#taskbar display:none`、top 與 main iframe 的 `contextmenu`/`F12`/`Ctrl+C(body)` 皆 `defaultPrevented`。
⚠ 修改 theme.js 後瀏覽器可能吃快取 → 測試時加 `?v=` 或強制重新整理。

## 3. 主畫面 State Record 按鈕

`sbStateRecord2`（dfm `palSetting/Panel2` 圖示鈕）已從 `Main.html` 移除；程式快照只由 `Main.gbControlBtn.html` 的 `sbStateRecord` 提供（controller 面板兩版皆顯示）。

## 4. C++ 離線模式 JSON 寫入（`page/json-writer.js`，HTJsonWriter）

目的：`cppoffline=1` 時把 HTML 端產生的 request / ack / 設定值**真正寫回 `D:\HT9045\JSON\`**，方便比對除錯（原本只能下載）。

### 架構
- **File System Access API**（Edge/Chromium；`file://` 亦為 secure context）。
- **頂層 background.html 持有 `dirHandle`**，存 IndexedDB（`ht9045-json-writer/kv/jsonDir`）；子頁不直接碰檔案。
- 子頁 `HTJsonWriter.write(name, obj)` → `postMessage({writeJson:{id,name,text}})` → background 寫檔 → 回 `HT_JSON_WRITTEN`。
- **每次同時寫 `<name>.json` 與 `js/<name>.js` 墊片**（file:// 各頁先讀墊片，否則讀到舊值）。
- 狀態 `none | prompt | granted | unsupported`，background 以 `HT_JSON_WRITER_STATE` 廣播（iframe load 時亦補送）。

### 操作
1. Debug 版 + C++ 離線 ON → 工作列出現 `JSON 寫入：選擇資料夾…` 徽章（或 main Debug ▾「📁 JSON 寫入資料夾…」）。
2. 點一下 → `showDirectoryPicker` 選 `D:\HT9045\JSON` → 徽章 `JSON 寫入：JSON ✓`。
3. 下次啟動徽章顯示 `點此授權`（handle 已存，需一次手勢 `requestPermission`）。已授權時再點＝換資料夾。
4. 寫入成功徽章短暫顯示 `已寫入 xxx.json`。

### 接入點
| 模組 | 寫入 |
|---|---|
| `motor-access.js publish()` | `motor-access.json`（catalog+request）；state 為 done/error/aborted 時另寫 `motor-access-ack.json`（`source.toolchain="HTML-offline"`） |
| `state-record.js publish()` | `state-record.json`；done/error 時 `state-record-ack.json`（steps 全 done、`zipFile=newPath.zip`） |
| `HW.MotorTest.html dbSave()` | `HTJsonWriter.save('Motor-config', …)` |
| `HW.IoSetView.html saveIO()` | `HTJsonWriter.save('IO-config', …)` |
| `settings-bind.js exportJson()` | `HTJsonWriter.save('Config' \| 'General-config', …)` |

`HTJsonWriter.save(name,obj)`：已授權→寫檔（回 `'written'`），否則退回瀏覽器下載（`'downloaded'`）。模組（motor-access / state-record / settings-bind）在 `HTJsonWriter` 不存在時會自動插入 `<script src="json-writer.js">`。

驗證（Playwright，以假 handle 取代 `showDirectoryPicker`）：MotorTest `btnHome` 離線一趟 → 寫入 `motor-access.json`×2、`js/motor-access.js`×2、`motor-access-ack.json`、`js/motor-access-ack.js`。

### 限制
- release 不載入 writer（僅 debug）。
- 假 handle 無法存 IndexedDB（DataCloneError）→ `pick()` 已容錯，只影響下次啟動需重選。
- 寫回的是 HTML 端模擬值；接上 C++ 後請由 C++ 端輸出，避免互相覆寫。
