---
name: ht9045-html-version
description: >
  HT9045/HT9011UC Handler 主程式（BCB6 VCL）畫面的 HTML 互動模擬專案（HTML Version）。
  涵蓋 background.html 視窗管理器、page/*.html 各 dfm 轉換頁、hwidgets 元件模板、theme.css 佈景、
  _gen_dfm_abs 產生器、ScreenShots.html 截圖索引、版面拖曳模式、i18n。
  觸發關鍵字：HTML version, HTML 畫面, 畫面模擬, background.html, release.html, debug.html,
  hwidgets, WidgetTemplates, ComponentMap, dfm 轉 HTML, _gen_dfm_abs, theme.css, SHOT_MAP,
  layoutEdit, htPropPanel, HTLAYOUT_BASE, htLoader, HT9045_Release.cmd, HT9045_Debug.cmd
---

> **//Steven 20260922** — 新增「**盤點一頁接了沒**」一節：`PAGE_WIRE_STATUS`（表⑤）是
> 現成的權威表，任何新盤點都要先對它；`tag` 級對顯示頁是**完成態**不是缺口。
> 另記 dfm 事件盤點工具 `audit_dfm_events.py`（九桶、兩句必講的話、五個會讓數字錯一倍的陷阱）、
> 不停機告警 NonStop（定義＝C++ 不呼叫 `StopAllMotor()`）、以及 AccessLevel 的缺口。
> 當日完整變更紀錄：`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260922_Steven.md`

> **//Steven 20260921** — 新增**第三種形狀：行為翻譯層**（golden 的 VCL handler 搬到瀏覽器）。
> 兩支落地：`ht9045_setup_sitemap.js`（Setup.SetUp 的 Site Mode）與
> `ht9045_contact_slk.js`（Setup.Contact 的 SLK 捲軸與 Contact Force）。
> 一併記下 dfm `TScrollBar` 產生成空 `<div>`、VCL `OnChange` 在 web 沒有對應、
> 以及存檔擴充點 `HT9045Contact.addCollector()` 三件事，見「行為翻譯層」一節。
> 當日完整變更紀錄：`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260921_Steven.md`

> **//Steven 20260919** — 新增三個 dfm 轉換頁（`Setup.ContactForce` / `HW.VacuumUnit` / `Setup.AGV`）、
> 產生器三個新機制（`JOB_BASE` 指定別棵 golden 樹、`HT9045_DFM_OUT` 輸出目錄覆寫、
> 宣告式 `widget_host()` ＋新檔 `page-widgets.js`），以及兩個元件模板
> （`makeVacuumPanel` 新增、`makeContactForceGroup` 依 dfm 原型重寫）。
> 當日完整變更紀錄：`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260919_Steven.md`

> **//Steven 20260916** — 更新接線分級（16/14/37 → 21/1/9/36）、補上執行期 tag 訂閱層，
> 並訂正 `Status.Security` 的後端檔（`levelset.dat`，不是 `Security_new.def`）。
> 當日完整變更紀錄：`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260916_Steven.md`

> **//Steven 20260915** — 本檔於 2026-09-15 更新。
> 當日完整變更紀錄：`D:\HT9045\CHANGES_20260915_Steven.md`


<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-html-version，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->
<!-- AI(W906-BA-SKILL) 20260915：本檔提到 D:\AI_TempFile，那些路徑在 D:\HT9045 這台機器上**不存在**（或已凍結）。
     產生器請改用本 skill 自帶的 scripts\；引用那些路徑的段落只當史料讀。 -->

# HT9045 HTML Version（BCB6 GUI 網頁化模擬）

HT9045/HT9011UC Handler 主程式（BCB6 VCL）畫面的 HTML 互動模擬專案。
用於客戶手冊、教育訓練、畫面對照，不需開啟實機即可瀏覽操作介面。

## 觸發關鍵字

HTML version, HTML 畫面, 畫面模擬, background.html, hwidgets, WidgetTemplates,
主畫面模擬桌面, ComponentMap, 元件對照表, dfm 轉 HTML, _gen_dfm_abs,
TALed 模板, TBtnPanel 模板, TTMyTray 模板, HW.IoSetView.html, Setup.OffSet.html, Setup.Speed.html,
theme.css, 佈景主題, SHOT_MAP, 截圖頁, 版面拖曳模式, layoutEdit,
屬性盤, htPropPanel, 元件盤, 群組, 對齊, 均分, alignSel, distSel,
基底版面, ht9xxx-layout-base, HTLAYOUT_BASE, 匯出 JSON, 縮放把手,
開站載入進度, htLoader, 品牌 Banner, Handler_Banner, Poseidon_Banner, HPI_1091

## 產出位置

```
D:\HT9045\                    ← 2026-09-02 由 <入口網站 repo>\public\Docs\manual\HT9xxx_Manual\ 搬移至此
├── background.html          ← 視窗管理器桌面＋開站品牌 loader；WINDOWS 陣列定義各視窗，?mode= 往下傳；?machine= 決定機種 profile
├── release.html / debug.html / debug9050.html ← 入口（debug9050 帶 machine=HT9050）
├── HT9045_Release.cmd    ← **Edge --kiosk 全螢幕**啟動器（release：無瀏覽器 UI；頁內封鎖右鍵/快速鍵）
├── HT9045_Debug.cmd      ← Edge 一般視窗（debug）
├── HT9050_Debug.cmd      ← HT9050（HP-9050）debug 啟動器；獨立 Edge profile，可與 HT9045 並存
├── IMG\ScreenShot\       ← loader 本機媒體（HPI_1091、Handler/Poseidon MP4、主視覺 JPG）
└── page\
    ├── ht9xxx.css           ← 共用樣式（XP 風格、tabs、submenu、win 樣式）
    ├── theme.css / theme.js ← 設計 token 與主題套用器（classic/dark/steel/contrast）
    ├── i18n.js              ← 多語系字典（HTI18N）；IDE.I18nEditor.html 編輯器
    ├── tabs.js              ← 簡單分頁切換（.tabs/.tab/.tabPane）
    ├── hwidgets.js          ← 自訂 VCL 元件模板庫（18 個 maker；20260919 加 makeVacuumPanel、makeContactForceGroup 依 dfm 原型重寫）
    ├── page-widgets.js      ← 宣告式執行期面板展開器（把產生器放的 `.htWidgetHost` 交給 hwidgets maker）；**必須在 hwidgets.js 之後載入**，由產生器 PAGE_EXTRA 掛上
    ├── qwerty.js            ← 彈出式小鍵盤（HTQwerty，模擬 TfQwertyKey；依 N_ 旗標切換數字/QWERTY/密碼）
    ├── motor-access.js      ← 馬達動作指令通道（HTMotorAccess：互斥、motion 鎖定只留 btnStop、ack 解鎖）
    ├── motionview-sim.js    ← Motion View 模擬層（HTMotionSim：Sim-scale.json＋Motor/IO-runtime.json → 位置/LED）
    ├── state-record.js      ← 程式快照（HTStateRecord：DoStateRecord request/ack；離線自動 ack＋下載快照包）
    ├── settings.js          ← 設定檔載入器（HTSettings：General-config / Config / View-rules；decide / applyBlocks）
    ├── settings-bind.js     ← JSON 設定值 → dfm 元件（HTSettingsBind：Config.Configuration / HandlerSys）
    ├── json-writer.js       ← C++ 離線模式 JSON 寫回 D:\HT9045\JSON（HTJsonWriter：File System Access，background 持 handle，子頁 postMessage）
    ├── dialog-bridge.js     ← Alarm/Message 雙向 modal bridge（background 載入：輪詢 *-dialog-request / Dialog-close-request，iframe overlay 開 Alert.* 頁，寫 response）
    ├── dialog-page.js       ← Alert.Note / Alert.MyMessageBox 頁內綁定（HT_DIALOG_REQUEST 填值、kCode 按鍵可見性、HT_DIALOG_ACTION）
    ├── IDE.WidgetTemplates.html ← 模板示範頁（Debug ▾「🧩 元件模板」；含小鍵盤展示區）
    ├── IDE.ComponentMap.html    ← 全部畫面的元件名稱對照表
   ├── IDE.define-to-json.html  ← BCB6 define / extern / struct JSON 化範圍與快速搜尋
    ├── IDE.StyleGuide.html      ← CSS 規範設定畫面（字體/字級/尺寸/色票）
    ├── IDE.I18nEditor.html      ← 多語系字典編輯器（HTI18N）
    ├── ScreenShots.html + shot\*.html ← 尚未 dfm 轉換表單的實機截圖頁（產生器生成）
    ├── Main.html            ← 主視窗（手工，含 Tools/Config/Debug 子選單、SHOT_MAP、標色）
    ├── Main.gbControlBtn.html ← tsMotionView 控制按鈕欄（操作鈕 debug／跳轉鈕兩版；視窗 noClose+fixed）
    ├── Main.MotorView.html    ← tsMotorView（StringGrid1 位置＋StringGrid3 11 顆狀態 LED，列＝enable 馬達，資料驅動動態生成）
    ├── Main.MotionView.html   ← tsActionView（dfm 轉換完成，SUBTREE_JOBS 產生；掛 motionview-sim.js）
    ├── Main.MotionView9050.html ← **HT9050（HP-9050）手工頁**（NO_OVERWRITE）；SVG 版面讀 `MotionView9050-layout.json`，含流程播放與版面編輯；調整見 skill `ht9050-motionview-layout`
    ├── Main.Logs.html         ← tsLogs（ScrollBox2 六段 Log）＋tsMNetLog → TPageControl 分頁各一 Memo
    ├── Main.ShuttleSensor.html ← tsShuttleSensor（Out Shuttle 1/2 Memo＋三 CheckBox）
    ├── Main.TaskList.html     ← tsTaskList（sgTaskList 110×62 StringGrid，State Record）
    ├── Main.CommView.html     ← tsCommView（8 子分頁巢狀 PageControl：Torque/InArm/TrayMap/Tj/TCPIP Log/Tray Step/Vibration/IndexArmYPos）
    ├── Main.HeaterView.html   ← tsHeaterView（HOTPLATE 1/2 StringGrid＋Message RadioGroup）
    ├── Main.Record.html       ← tsRecord（sgDebugRecord＋memoAutoClean/AseRecordMemo＋CLEAR/Save Log）
    ├── Main.UnloaderInfo.html ← tsUnloaderInfo（sgUnloaderInfo 4×12＋ComboBox＋Info RadioGroup）
    ├── Main.AOAInfo.html      ← ts1（AOA IN/OUT Memo＋Panel14 各站 X/Y 位置表）
   ├── Data.SortCT.html / Data.ContactCT.html / Data.LotInfo.html / Status.TemperFrom.html
   ├── Data.Observer.html / Data.TestCategory.html / Status.ShowBinSelect.html   ← 手工頁；SortCT／ShowBinSelect／Setup.BinSel 共用 bin-mode.js，以 Production context.activeBinSelectIndex（fallback 為 Run/Start Mode）選 Binasgn variant，Tray Pass/Fail（0=綠、>0=紅）與 bin 指派一致；TestCategory／Observer 的 Tester Category、Contact Count、Yield 依 Production runtime 同步測試資料
   ├── Setup.OffSet.html / Setup.Speed.html / HW.IoSetView.html / Config.Configuration.html ← 產生器自動生成（分類前綴見原則 0）
   ├── HW.HandlerSys.html      ← Handler System（HandlerSys.dfm；入口 cTemperFrom Panel71 密碼 27025312；settings-bind general；TabSheet2 Track Matrix）
   ├── Setup.ContactForce.html ← Contact Force Setting（ContactForce.dfm，862×716）；入口在 Setup.Contact 的 btContactForce（cContact.cpp:15099 → fContactForce->Show()），**不在主選單**；四個 ScrollBox 走 widget_host → makeContactForceGroup
   ├── HW.VacuumUnit.html      ← Vacuum Unit（VacuumUnit\VacuumUnit.dfm，682×989）；主畫面 sbVacuumUnit（main.cpp:34660）；尺寸與四個 GroupBox 由 CLIENT_OVERRIDE＋RUNTIME_PROPS 還原**執行期**（dfm 設計期 655×711 不是任何機台看得到的樣子）；四個 ScrollBox 走 widget_host → makeVacuumPanel
   ├── Setup.AGV.html          ← AMR Setting（Automation\AGV.dfm，895×683）；主畫面 spbAGV（main.cpp:34879）；**來源樹走 V912**（JOB_BASE），V910 那份的 E84 燈號還是 Label33 這類 IDE 自動命名
    ├── Alert.Note.html         ← fNote（note.dfm，ShowErrorMessage Alarm 对话框；972×761，RUNTIME_PROPS 依 FormShow 擺位）
    ├── Alert.MyMessageBox.html ← MyMessageBox（mymessbox.dfm，ShowMyMessage 对话框；472×219）；兩頁皆由 dialog-bridge.js 以 overlay iframe 開啟，不在 WINDOWS 清單
    ├── Alert.Password.html     ← fPassword（Password.dfm，609×305）登入頁；掛 login-page.js，由 dialog-bridge #dialogAuth（z 21000）iframe 開啟，帳密經 Dialog-auth-verify.json 交 C++ 裁決
    ├── login-page.js        ← Alert.Password 頁內綁定（HT_DIALOG_AUTH_REQUEST/INPUT/AUTH_SUBMIT/AUTH_CANCEL/AUTH_RESULT；不驗證密碼）
    └── img\                 ← HonPrec.png（去背 logo）、dfm 底圖 png、shot\ 實機截圖

JSON（`D:\HT9045\JSON\`，全部由產生器輸出；file:// 墊片在 `JSON\js\`）：
`IO-config/IO-runtime`、`Motor-config/Motor-runtime`、`Teach-config`、`motor-access/-ack`、`teach-access`、`Sim-scale`、
`state-record/-ack`、`Task-runtime`、`System-runtime`、`General-config`（Gerneral.ini）、`Config`（config.ini）、`View-rules`、
`Setup-index`（工作檔索引）、`Setup-current`（目前 Recipe 11 組資料）、
`Machine-profile`（**HTML-only 機種能力集**，runtimeSupported:false）、`MotionView9050-layout`（HT9050 版面）。
各檔資料項、四大分類（硬體設定檔／Config 檔／Setup 檔(Recipe)／生產記錄檔）與開站載入順序見 skill `ht9045-html-json`。
```

## 核心原則

0. **頁面檔名規則**（2026-09-02）：去掉 dfm 前置字母（`fMain.X`→`Main.X`、`cBinSel`→`BinSel`、`uhome`→`home`）；
   分類前置：**`Setup.`** 工作檔／機台設定（21；20260919 加 ContactForce／AGV）、**`Data.`** 生產數據／紀錄（StartCondition/LotInfo/Observer/SmartDiagnostic/SortCT/ContactCT/TestCategory/CounterClear/Builder，9）、
   **`Status.`** 狀態顯示（ShowMessage/LtcSensor/ShowBinSelect/GroundMan/TowerLight/TemperFrom/CounterSel/Security，8）、**`HW.`** 硬體直接操作（MotorTest/home/teach/IoSetView/OmronEJ1N/MyCCLinkSensor/HandlerSys/VacuumUnit，8；20260919 加 VacuumUnit）、
   **`IDE.`** 開發輔助工具頁（ComponentMap/I18nEditor/StyleGuide/WidgetTemplates，4）；
   **`Alert.`** modal 对话框（Note/MyMessageBox/Password，3；由 dialog-bridge.js overlay 開啟，非桌面視窗）；
   `Main.*`／`main`／`ScreenShots` 不分類。分類集合定義在 `_rename_pages.py`（可重跑，含引用替換）——詳 [references/naming-release-writer.md](references/naming-release-writer.md)。
   `HW.teach.html` 預設分頁＝`tsAxleCtrl`。
1. **元件名稱必須與 .dfm 完全一致**（id 與 title 都要），並同步登錄到 IDE.ComponentMap.html。
2. **畫面以 HTML 元件重建，不用截圖貼圖**（使用者已明確否決貼圖方案）；
   尚未轉換的表單以「實機截圖頁＋⚠ badge」過渡（選單項標色區分狀態）。
3. 位置以 dfm 的 `Left/Top/Width/Height` 絕對定位還原；底圖（TImage）為執行期
   `LoadFromFile` 載入，來源在 `D:\HT9045\IMG\BMP`、`D:\HT9045\IMG\Graphic`。
4. **dfm/cpp 原始碼＝BCB6 版本**（Borland C++ Builder 6）：
   `D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2`
   （cp950/Big5 編碼，讀取需 `encoding='cp950', errors='replace'`）。
   後續會改用 **VC++** 重寫，兩套原始碼必須明確區隔：
   - 本 skill、references、產生器與生成的 JSON 中，凡引用此路徑的 dfm/cpp 一律標注 **BCB6**。
   - 產生器輸出的 JSON `source` 區塊需帶 `"toolchain": "BCB6"`。
   - 日後新增 VC++ 來源時另立標注（`VC++`），不可與 BCB6 混寫在同一段落。
5. 主題只換「介面鉻件」；dfm 內容色（LED TrueColor、Panel Color、Tray colorMap）不隨主題改變。
6. **顯示環境固定為機台 FullHD 1920×1080**；固定視窗不得超出此範圍（扣工作列後可用高 ≤ ~1032px）。
   本專案不支援手機、平板、直向或 responsive mobile layout，也不得以 390×844 等 mobile viewport 作為驗收或據此修改版面。
   唯一產品 UI 驗收 viewport 為 1920×1080；權威規格見 [`html-display-environment.md`](../../specs/html-display-environment.md)。
   - `#htLoader` 固定 1320×890、向上位移 36px；公司圖文 Banner 與媒體間保留 12px 白帶。
   - 媒體依序為 Handler MP4 → 主視覺 JPG → Poseidon MP4，於進度 30%／60% 順序切換；防跳級、每頁至少 700ms，快速開站至少 7s。
   - 載入提示置於進度條下方，以 `#e5f1f8` 置中顯示；載入中輸入封鎖與完成後暫停影片不可弱化。完整規格見 [references/desktop-theme.md](references/desktop-theme.md)。
7. **dfm 完成轉換後必須同步更新 ScreenShots.html**：將 dfm base 加入 `_scan_dfm_shot.py` 的 `converted_dfm`
   （若有截圖再將 Form 名加入 `_gen_screenshot_pages.py` 的 `SKIP`）、必要時更新 `DYNAMIC_CLASSES`，
   重跑兩支產生器並同步保存版——詳見 [references/screenshot-index.md](references/screenshot-index.md)。
8. **HTML 端資料來源一律只用 JSON**（放在 `D:\HT9045\JSON\`，離線快照放 `JSON\offline\`）。
   BCB 程式若讀取 **非 JSON 檔**（`.ini`／`.dat`／`.csv`／`.Data` 等），HTML **不得直接解析或讀取**，
   必須**先通知工程師完成檔案轉換（→ JSON）後才能使用**。
   **哪些 JSON 要載入、資料項為何、四大分類（硬體設定檔／Config 檔／Setup 檔(Recipe)／生產記錄檔）
   與開站載入順序 → 已獨立為 skill [`ht9045-html-json`](../ht9045-html-json/SKILL.md)，本檔不再重複。**
   - 實作規範：載入失敗或檔案未轉換時，狀態列顯示明確訊息（例：`需工程師轉換 Mot_Table.csv → JSON`），
     不可 fallback 去讀原始 ini/dat/csv。
   - **file:// 傳輸墊片**：Edge/Chromium 在 `file://` 下**封鎖 XHR/fetch**（回 status 0＋空字串，
     頁面顯示 `Empty response`）。故各頁載入器在 `location.protocol==='file:'` 時**優先用 `<script>` 標籤**
     讀 `JSON\js\<name>.js`（內容為 `window.__HT9045_DATA__["<name>"]={...}`），失敗才退回 XHR/fetch；
     http(s) 下則相反。`JSON\js\*.js` 由 `_gen_json_shim.py` 從 JSON **自動生成**（JSON 仍是唯一資料來源，
     .js 只是傳輸層，勿手改）。**改完 JSON 必須重跑產生器**：
     `& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py`
9. **動態 TPanel Caption 不可覆寫容器**：Observer、Teach、MotorTest 等頁一律更新 Panel 的直接子層
    `.pnlCap.textContent`；產生器永遠輸出 `.pnlCap`（無設計期 Caption 時為空），並完整套用 DFM
    `Font.Name/Height/Color/Style`。禁止 `panel.textContent=value`，否則會刪除子元件並退回頁面預設字形。

10. **Main 事件與狀態分離**：`main-control.js` 只在 Debug 且 JSON 資料夾已授權時，將可操作的 `TfMain` 事件寫入 `Main-command-request.json`；`palTesterMode`、`imgTempOnOff`、`palNormal`、`palPrime` 和 `palFunc[]` 是純 runtime 顯示。實際 C++ 串接時，所有 Main 控制項的最終 caption、visible、enabled、狀態色與 `imgTester.connection` 均由 `Production-update.json.state.context.mainControls` 發佈；`activeBinSelectIndex` 是 Bin 顯示權威值。僅 `mode=debug` 的 C++ 模擬可將 Tester、FT、RT 點擊視為成功並廣播暫時 Bin 預覽，下一筆 C++ 快照即覆蓋。完整事件與 `palFunc` key 契約見 [references/main-control-commands.md](references/main-control-commands.md)。
11. **BinSel 模式與密碼**：`Setup.BinSel.html #cbTestMode` 的項目及單一可見 TabSheet 必須由 BCB6 `TfBinSel::cbTestModeChange()` 對應的 `production.context.binSelectControls` 發佈；不可固定成 Normal/ART/MRT。Save 以 `BinSel-command-request.json` 請求 C++ 執行原始 `spbSaveClick` 守衛。SCC Off-Line 首次編輯的密碼是 `mtBinSelectMouseDown` 專用條件，HTML 不保存或傳送密碼，必須由 C++ 驗證並回填 `offlineEditAuthorized`。細節見 [references/bin-select-form.md](references/bin-select-form.md)。
   Normal、RT、Off-Line 的可重複驗證資料位於 [JSON/BinSel-mode-validation.json](../../../JSON/BinSel-mode-validation.json)，只可作 `HT_SETTINGS` 測試注入；Debug 的 `#cbTestMode` 選擇對應模式時必須顯示各案例不同 tray/bin/pass-fail 差異。Main `#cbRunStartMode` 廣播後須清除 fixture，並同步 BinSel、SortCT、ShowBinSelect 的 resolved mode。
12. **Speed 與 ShowMessage 同步**：`Setup.Speed.html` 與 `Status.ShowMessage.html` 共用 `speed-view.js`。前者輸入異動以 `HT_SPEED_VIEW` 提供 debug 桌面預覽，background 轉送；最終狀態由 `Production-update.json.state.productionStreams.speedView` 覆蓋。必須保留 BCB6 `ShowSpeed()` 的六列/`Motor Speed` 單列條件和 SCS 才顯示 Index Accel 的條件，完整 schema 見 [references/speed-show-message-sync.md](references/speed-show-message-sync.md)。
13. **Setup 表單 JSON 與離線 Debug**：除 `Setup.BinSel.html` 的專用 Binasgn resolver 外，所有 `Setup.*.html` 均由 `theme.js` 自動載入 `setup-runtime.js`。每頁載入 `Setup-current.json`，並接受 `Production-update.json.state.context.setupForms["Setup.<頁名>"].fields["<元件 id>"]` 的 C++ 權威回傳；`mode=debug` 且尚無該頁 C++ runtime 時，套用 [JSON/Setup-offline-debug.json](../../../JSON/Setup-offline-debug.json) 的同名頁面 fixture。fixture 僅供 HTML C++ 離線模擬，絕不可寫回或取代 Production runtime。新增或修改此 JSON 後必須重跑 `_gen_json_shim.py`。
   `Setup.SetUp.html #ScrollBar1` 是 BCB6 `TfSetup::ScrollBar1Change()` 的 Test Mode 選擇器。Debug fixture 提供 Single/2x2/2x4/4x4/32-site NN 模式；切換時同步更新 `Panel1`、對應 Setup 欄位與 Main `SitePanel`。實際 C++ 串接時由 `context.setupForms["Setup.SetUp"].sitePanel` 回傳完整 panel 覆寫，HTML 不得自行推定實機 Site Map。

## scripts/（產生器與工具）

工作副本在 `D:\AI_TempFile\`（實際執行入口），本 skill `scripts/` 為保存版；重大改版後同步。

| 腳本 | 用途 |
|---|---|
| _gen_dfm_abs.py | dfm→HTML 主產生器（絕對座標）；dfm 更新後重跑即全部同步：`& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_dfm_abs.py`。<br>`NO_OVERWRITE` 集合＝**手工維護頁**（IoSetView／uteach／uMotorTest／cObserver／cBinSel）：仍留在 `JOBS`（供 ComponentMap 取元件樹）但不覆寫 HTML。<br>`SUBTREE_JOBS` 集合＝**把大型 dfm 內單一容器輸出成獨立頁**（目前 `main.dfm` 的 `tsActionView` → `Main.MotionView.html`），由 `parse_dfm_block()` 只切該 object 區塊解析。<br>`RUNTIME_PROPS` ＝**執行期 FormShow/Reset 擺位覆寫表**（dfm→{元件名:{屬性:值}}，render 前直接改節點；mymessbox/note 依 BCB6 FormShow 預設分支）；`HIDE_TABS` ＝執行期 `TabVisible=false` 的 TPageControl（頁籤列不輸出、內容貼頂）。<br>**20260919 三個新機制**：`JOB_BASE` ＝**單一 job 指定別棵 golden 樹**（目前只有 `Automation\AGV.dfm`→V912，因為 V910 的 E84 燈號還是 `LabelNN` 自動命名，違反核心原則 1）；`OUT` 可由環境變數 **`HT9045_DFM_OUT`** 覆寫（只想取幾頁時指到 scratch，**但仍要跑完整 JOBS**，否則 `IDE.ComponentMap.html` 會被砍到只剩那幾段）；`widget_host()` ＝**宣告式執行期面板**（產生器只宣告要幾個／叫什麼，座標留在 `hwidgets.js` 一份，由 `page-widgets.js` 展開）——詳 [references/dfm-generator.md](references/dfm-generator.md) |
| _gen_screenshot_pages.py | 掛 `D:\HT9045\IMG\ScreenShot` 生成 shot/*.html + shot_windows.js（`SKIP` 集合控制哪些已轉表單不出截圖卡） |
| _scan_dfm_shot.py | 掛全部 *.dfm 生成 screenshot_meta.js（ALL_DFM 狀態表 + DYNAMIC_CLASSES）；`converted_dfm` 集合＝html 狀態唯一開關。dfm 轉換完必重跑 |
| _scan_cfg_overlap.py | cConfiguration Visible=False 元件重疊掃描 |
| _apply_theme_links.py | 批次為靜態頁掛 theme.css/theme.js |
| _gen_json_shim.py | 由 `D:\HT9045\JSON\*.json` 生成 `JSON\js\*.js` 傳輸墊片（file:// 下 Edge 封鎖 XHR 用）；**JSON 改動後必重跑** |
| _gen_motor_access.py | 由 **BCB6** `uteach.cpp` 抽出 TECH_PARA/TECH_TWOPARA/TECH_MotorAxle，生成 `motor-access.json`／`motor-access-ack.json`／`teach-access.json`；**跑完要再跑 _gen_json_shim.py** |
| _gen_simscale.py | 由 **BCB6** `cinitial.cpp` 的 `SetSimuScreenPara()` 抽出 SetScreenScale／SetPanel／SetMyLed，生成 `Sim-scale.json`（Motion View 模擬對照）；**跑完要再跑 _gen_json_shim.py** |
| _gen_state_record.py | 依 **BCB6** `main.cpp DoStateRecord()` 生成 `state-record.json`（catalog+steps+request）、`state-record-ack.json`、`Task-runtime.json`、`System-runtime.json` 骨架 |
| _gen_sitepanel_offline.py | 從 `Setup-current.json`（不讀 `.Data`）產生 `offline/SitePanel-runtime.offline.json`；依 **BCB6** `TfMain::DrawTestSitePanel()`／`ShowTestHeadComp1()`／`CheckSiteMapIsStander()` 輸出 $8\times4$ Site Map、Lime/Aqua/Silver 色碼與雙臂啟用狀態；完成後必跑 `_gen_json_shim.py` |
| _gen_ini_json.py | `Gerneral.ini` → `General-config.json`（＋**BCB6** `HandlerSys.cpp` uiMap）、`config.ini` → `Config.json`（＋`cConfiguration.cpp elConfig->Add` uiMap）；需 HW.HandlerSys.html/Config.Configuration.html 已存在才算 inHtml |
| _gen_config_help_json.py | Config YAML + 七語 i18n → `Config-help.json`（Configuration MemoA..MemoP 說明與 ECID）；完成後必跑 `_gen_json_shim.py` |
| _gen_define_index.py | **BCB6** `cmydef.h`／`Config.h`／`cprod.h` → `Define-index.json`（靜態宣告與 JSON 化分類）；完成後必跑 `_gen_json_shim.py` |
| _gen_security_access_json.py | **BCB6** `system\levelset.dat`（`LAST_LEVEL_SET.AccessLevel[256]`）＋`cSecurity.cpp` → `Security-access.json`；HTML Debug 請求寫入 `Security-access-update.json`，C++ 驗證後回寫權威 JSON；完成後必跑 `_gen_json_shim.py` |
| _gen_setup_json.py | `SetUp.inf`＋`IniData\Data\<Recipe>` → `Setup-index.json`／`Setup-current.json`（11 個核心 `.Data`、Binasgn 變體、configByRecipe）；**跑完要再跑 _gen_json_shim.py** |
| _gen_view_rules.py | 產生 `View-rules.json`（視窗／頁內區塊顯示規則，對照 USE_xxx / IniConfig）；改規則請改產生器 |
| _rename_pages.py | 頁面批次改名（Setup./Data./Status./HW./IDE./去前置；依基底名查集合，可重跑）＋全工作區引用替換；dry-run 預設，`--apply` 才執行 |
| _teach_default_tab.py | HW.teach.html（手工頁）預設分頁切到 tsAxleCtrl |
| _scan_datapath.py | 掃各表單 cpp 是否存取 `IniData\Data`（判定 Setup. 前置） |

**CPP 模式接線工具鏈**（20260914/15 新增，工作副本在 `HT9011UC_Cpp_V3.33.906.0/scratchpad/`）：

| 腳本 | 用途 |
|---|---|
| extract_page_v2.py | 從 golden 抽每頁的 widget→(檔,區段,鍵) 對照。**v2 的關鍵是表單範圍鎖定**：先用 `.dfm` 的 `object <id>:` 重疊度鎖住表單，再只從該表單自己的 `.cpp` 抽，不做全樹 id 反查。v1 沒鎖範圍，把 `Setup.HotPlate` 的欄位指到 `Tray.Data`（`XCT1` 等 id 兩頁都有），會把值寫進錯的檔還不報錯。輸出 `v2all.json` |
| gen_wire.py | 由 `v2all.json` 產生每頁的 `ht9045_wire_<slug>.js`。`SYS_PAGES` 決定哪些頁走 `/api/system/`（機台設定檔）而非配方；`KB_ONLY` 決定哪些頁只掛鍵盤。**slug 由頁名推導**，所以 JSON 裡的 `"page"` 值與 `SYS_PAGES` 的 key 必須字字相符 |
| deploy_wire.py | 把產生的 wire js 與 `<script>` 行佈署到頁面 |
| kb_audit.py | 掃 `web/page/` 產生 `docs/KEYBOARD_AUDIT.md`（A/B/C 三級覆蓋率）。**該文件標明「不要手改」，數字有變就重跑** |
| gen_page_status.py | 從實體接線檔量出每頁的接線程度，寫入 `screenshot_meta.js` 的 `PAGE_WIRE_STATUS`（`page/ScreenShots.html` 表⑤）。分 data／kb／none 三級 |
| add_liveapi.py | 依「跑著的 wb_serve 回報的 `/api/system`＋`/api/text` 索引」為 `FILE_IO_STATUS` 補 `liveApi`／`liveOk` 欄（表③）。**要先啟動 wb_serve** |
| gen_sysfile_table.py | 產生 `wb_serve.cpp` 的 40 筆 `SysFileEntry` 初始化列（路徑一律取 golden 全域） |
| ws_probe.py | 手工 RFC6455 WebSocket 客戶端，比對指令前後的檔案 SHA256，驗證 `dryRun` 是否真的不寫。曾用它證出 `dryRun` 被靜默丟棄、`preview()` 其實在真寫 |
| dfm_rename_diff.py | 比對兩版 `.dfm` 找出只改名沒改程式碼的元件 |

## references/（詳細規則）

| 文件 | 內容 |
|---|---|
| [references/dfm-generator.md](references/dfm-generator.md) | 產生器解析規則、IMG_MAP、hwidgets API、VCL→HTML 定位法則、Tab/Panel 樣式、--gbi 內縮、cConfiguration Visible=False 策略、POS_OVERRIDE、DEFAULT_ACTIVE、LED＋Label 整合 |
| [references/dfm-html-proxy-mapping.md](references/dfm-html-proxy-mapping.md) | **DFM authority ↔ HTML Proxy 對照**：保留 uiMap/INI binding 的表格化與分類重整、雙向同步、JSON-only extension、HandlerSys Track Matrix/Com Port 實例與驗證清單 |
| [references/screenshot-index.md](references/screenshot-index.md) | ScreenShots.html 索引頁資料來源（shot_windows.js／screenshot_meta.js）、兩支產生器（_gen_screenshot_pages.py／_scan_dfm_shot.py）、**dfm 轉換後的更新檢查清單**（converted_dfm／SKIP／DYNAMIC_CLASSES）、重跑指令、繪製欄位 |
| [references/vcl-widgets.md](references/vcl-widgets.md) | 自訂 VCL 元件轉 HTML（hwidgets.js／HTWidgets）：18 個 maker（TALed/TMyLed/TBtnPanel/TSpeedButton/TTMyTray/Labeled 系列＋8 個執行期動態 class，含 20260919 的 `makeVacuumPanel` 與重寫的 `makeContactForceGroup`）、opt 參數、VCL 原始碼對照、LED 樣式、TTMyTray colorMap/cells、CSS 類別、`page-widgets.js` 宣告式展開、與產生器平行實作同步 |
| [references/exit-save-unified.md](references/exit-save-unified.md) | Exit/Save 按鈕全域統一規則（標準 glyph/字型、EXIT_CAPS/SAVE_CAPS、按鈕型 TPanel 改造條件、豁免清單）＋57 顆命中元件全清單 |
| [references/desktop-theme.md](references/desktop-theme.md) | background 視窗管理器（品牌 loader、{open} scrollIntoView、主視窗鎖定、預設版面）、佈景主題/i18n/字體/--fsx、RTC 主題化、StyleGuide、截圖頁 v2＋SHOT_MAP、選單標色、版面拖曳模式、瀏覽器截圖限制 |
| [references/bin-select-form.md](references/bin-select-form.md) | Bin 設定畫面表單重構版（Setup.BinSel.html，取代 TMyTray）：版面結構、拖拉 paint、狀態欄條件、每 bin 單列、隱藏列、7 個 tabsheet 分頁（TAB_DATA 獨立狀態）、release/debug 兩版本（html[data-mode]）、底部操作、資料模型，以及 SortCT／ShowBinSelect 的 Recipe pass/fail 顯色契約 |
| [references/observer-tables.md](references/observer-tables.md) | fObserver 內 TTMyTray/TStringGrid 拼盤改 HTML table：共通手法（Python 佔位替換、div 配對移除、分頁增刪）、Tester Category、Contact Count、最近 10 次 Site Yield 歷史、Event Log、移除 tsScanner/tsMDB/SavePictureDialog1 |
| [references/qwerty-keyboard.md](references/qwerty-keyboard.md) | 彈出式小鍵盤（qwerty.js／HTQwerty，模擬 TfQwertyKey）：N_ 旗標表、ShowQwertyKey 模式邏輯、版面（數字/QWERTY/Value Limit）、HTQwerty.show API、接線範例（main edSoakTime/btLogin、cBinSel Sete* 事件對應）、CSS 類別 |
| [references/main-split.md](references/main-split.md) | main.dfm/main.cpp 拆分為獨立 HTML：pgMain→tsMain(Main.html)+tsMotionView；tsMotionView 拆成 gbControlBtn(debug only, gbControlBtn.html)＋pgMotionView 11 個 TabSheet 各一頁（Motor/Motion/Comm/Heater View、Logs、Shuttle Sensor、Record、Task List、Unloader Info、MNetLog、AOA Info），含按鈕/元件↔main.cpp 事件對照、debugOnly 視窗掛載 |
| [references/main-sitepanel-json.md](references/main-sitepanel-json.md) | Main.html 的 Setup JSON 綁定盤點與 SitePanel table 資料契約：BCB6 `DrawTestSitePanel()` 色碼、A-D/a-h 列欄標題、標準 Site Map 規則、C++ offline 快照與 runtime-only 欄位邊界 |
| [references/main-control-commands.md](references/main-control-commands.md) | Main.html 的 13 個 BCB6 控制事件代理：溫度、Tester、Run/Start Mode、工作檔、FT/RT/EQC、燈光與風扇的 JSON request/ack schema、payload 範例與 C++ guard 邊界 |
| [references/debug-json-command-log.md](references/debug-json-command-log.md) | Debug 專用 JSON Command Log 獨立視窗：Main/Site request、BCB6 ack 的即時檢視、固定 `D:\HT9045_Log\Debug_JSON_Log` 記錄設定與 C++ 實體寫檔責任 |
| [references/io-dual-json.md](references/io-dual-json.md) | IoSetView 的 IO 雙檔架構（IO-config.json + IO-runtime.json）：靜態/動態分流、`ioId` 合併規則、`unknown/stale/no-data` 狀態顯示與 legacy fallback |
| [references/motor-dual-json.md](references/motor-dual-json.md) | Motor 雙檔架構（Motor-config.json + Motor-runtime.json）：position/motion/state 與 limits/driverType 分流，對應 uMotorTest/uhome/uteach/fMain.MotorView |
| [references/motor-access.md](references/motor-access.md) | **馬達動作指令通道**：`motor-access.json`（commands 目錄＋request）、`motor-access-ack.json`（完成回報）、`teach-access.json`（Set/Go 對照表）；互斥規則、motion 鎖定只留 btnStop、ack 解鎖、離線自動 ack、jog 放開要監聽 document |
| [references/motion-view.md](references/motion-view.md) | **Motion View 模擬層**（`Main.MotionView.html` ← main.dfm `tsActionView`）：BCB6 `SetSimuScreenPara()` 對照、`Sim-scale.json` 結構、機構座標→畫面像素線性換算與退回規則、LED 群組綁定、`HTMotionSim` API、產生器 `SUBTREE_JOBS`／`NO_OVERWRITE` |
| [references/state-record.md](references/state-record.md) | **程式快照**（BCB6 `DoStateRecord(iShowAlarm,bManual)`）：步驟摘要、HTML 限制與對策（request/ack/Task-runtime/System-runtime）、`HTStateRecord` API、main `sbStateRecord2`／gbControlBtn `sbStateRecord` 接線、C++ 端实作要點 |
| [references/settings-json.md](references/settings-json.md) | **設定檔 JSON**：`General-config.json`（Gerneral.ini）／`Config.json`（config.ini）結構、uiMap（cConfiguration `elConfig->Add`／HandlerSys `WriteIniDataGeneral`）、`settings-bind.js` 綁定規則、`HW.HandlerSys.html` 入口、`View-rules.json`＋`HTSettings`（background 啟動預載、視窗 hide/disable、頁內 applyBlocks） |
| [references/setup-recipe-json.md](references/setup-recipe-json.md) | **Setup/Recipe JSON**：`Setup-index.json`／`Setup-current.json` schema、11 個核心 `.Data`、Binasgn 變體、Main 綁定與重跑流程 |
| [references/naming-release-writer.md](references/naming-release-writer.md) | **頁面命名規則**（Setup./Data./Status./HW./IDE./Alert.，`_rename_pages.py`）、**Release 模式**（theme.js 封鎖右鍵/快速鍵、工作列隱藏、Edge `--kiosk` 啟動器）、**離線 JSON 寫入**（json-writer.js：File System Access、徽章授權、接入點）、主畫面 sbStateRecord2 移除 |
| [references/dialog-bridge.md](references/dialog-bridge.md) | **Alarm/Message modal bridge**：`ShowErrorMessage`/`ShowMyMessage` → Alarm/Message-dialog-request/response、C++ IO 主動關閉（Dialog-close-request/response）、**權限密碼握手**（Dialog-auth-verify/result，HTML 不驗證）、K code 12 bit、`pressedButton`（Start/Pause）；`Alert.Note.html`/`Alert.MyMessageBox.html` 由 dfm 生成（RUNTIME_PROPS 依 FormShow 擺位），`dialog-page.js` 填值与按鍵邏輯，`dialog-bridge.js` iframe overlay；**置頂層級** 20000/21000/99999；多 Alert 同時出現提案；固定 FullHD |
| [references/header-json-index.md](references/header-json-index.md) | **BCB6 header JSON index**：`cmydef.h`／`Config.h`／`cprod.h` 的 JSON 化分類、統計、runtime bridge 設計與重生流程 |
| [references/security-access-json.md](references/security-access-json.md) | **Security 權限 JSON**：`levelset.dat`／`AccessLevel[256]`、`SecurityPalVisible()` runtime 快照、`Status.Security.html` 套用規則與 C++ 交付 schema |

> **開站 JSON 清單與載入順序**（硬體設定檔／Config 檔／Setup 檔(Recipe)／生產記錄檔四大分類）已独立為 skill
> [`ht9045-html-json`](../ht9045-html-json/SKILL.md)，不在本檔重複。

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

## Setup 頁的配方接線與小鍵盤（20260914/15 落地）

頁面要能讀寫機台配方，走的是 `wb_serve.exe`（不是 JSON Simulator）。
**載入順序固定，不可顛倒**，放在 `</body>` 之前：

```html
<script src="qwerty.js"></script>               <!-- 機台原有，不要另造 -->
<script src="ht9045_recipe_client.js"></script> <!-- 通訊層＋HT9045Tags，全站共用 -->
<script src="ht9045_wire_engine.js"></script>   <!-- 共用引擎：load/save/鍵盤/tag -->
<script src="ht9045_wire_<slug>.js"></script>   <!-- 只有資料，機械產生 -->
```

### 執行期 tag（唯讀顯示）——20260916 新增的第五種形狀

`ht9045_recipe_client.js` 的 `HT9045Tags` 是瀏覽器端的 tag 訂閱層
（`connect()` / `on(tag,fn)` / `subscribe(fn)` / `get(tag)` / `status()`）。
引擎的 `tags` 形狀讓接線檔直接寫對照：

```js
tags: { '<tag 名>': ['<元素 id>', '<形狀>', <小數位>] }
// 例（ht9045_wire_main.js）
'temp.sv':        ['edWorkTemperBase', 'text', 1],
'recipe.current': ['edSetupFileName',  'text'],
```

⚠ **`onmessage` 原本只有 `ack` 一個分支、沒有 else**，伺服器每 500 ms 送的
snapshot／patch 與 alarm／modal／query 五種訊框全被靜默丟棄。C++ 端一直是完整的，
缺口只在這一段，20260916 修掉。

⚠ **目前 117 個 tag 只有 4 個接到 HTML 目標**（都在 `main.html`），其餘 113 個
沒有 HTML 目標。tag → widget 對照是現在最大的落差，而且**是純前端工作**。

⚠ **null 一律顯示 `---`，不可顯示 0。** tag 訊框沒有 `available`／`updateClass`／
`trigger`／`updatedAt`／`seq` 任何一個 metadata 欄位，前端分不出「沒接上」與
「機台真的沒有」。在開發機上量到的一定是 `---`——原因與那不是接線壞掉的證據，
見 skill `ht9045-html-json` 的「為什麼 96 個裡有 87 個是 null」。

- **共用引擎，不是每頁複製**。每頁資料檔只呼叫
  `HT9045Wire.register({page, slug, fields, pending, kb})`。
  `fields` 只放「所有擁有該文件的配方都有」的鍵；不安全的進 `pending`。
- 資料檔由 `HT9011UC_Cpp_V3.33.906.0/scratchpad/gen_wire.py` 從 golden 機械產生，
  **手改會在下次重跑時被覆蓋**。抽取方法見 `fw-wave-loop` skill 第 8 條。
- 例外：`Setup.Contact.html`（`ht9045_contact_wire.js`）與 `Setup.HotPlate.html`
  （`ht9045_hotplate_wire.js`）保留手寫版，它們在真機驗證過且 PENDING 註解較完整。
- 目前狀態（**20260916 實測**，`scratchpad/gen_page_status.py` 量出）：
  交付包 67 頁，**已接資料 21 頁**、執行期 tag 1 頁、只有小鍵盤 9 頁、完全未接 36 頁。
  完整逐頁表見 `page/ScreenShots.html` 表⑤（資料在 `screenshot_meta.js` 的 `PAGE_WIRE_STATUS`），
  逐 tag 表見表⑥（`TAG_WIRE_STATUS`）。

  已接資料的 21 頁：`Config.Configuration`、`Setup.Temp_Set`、`Setup.Speed`、
  `Setup.YieldMonitoring`、`Setup.Contact`、`Setup.Cleaning`、`Setup.TesterIF`、
  `Setup.TrayForm`、`Setup.Ld_ULd`、`Setup.SetUp`、`Setup.BarCode`、
  `Data.StartCondition`、`Setup.TrayAssignment`、`Config.DIOInterFaceCFG`、
  `Setup.HotPlate`、`Setup.QAMode`，**以及 20260916 新接的五頁**：
  `HW.teach`、`HW.HandlerSys`、`HW.MotorTest`、`HW.IoSetView`、`Status.Security`。

  ✅ **20260916：先前「最大落差」那四頁已補上。** `HW.teach` / `HW.HandlerSys` /
  `HW.MotorTest` / `HW.IoSetView` 的 `sysFields`／`sysGrid` 已填，按存檔會經
  `system.file.put`／`system.csv.rows` 寫回真檔（仍需 `--allow-system-write`）。

  ⚠ **`Status.Security` 的後端不是 `config\Security_new.def`。** 那 179 組 radio 存的是
  `system\levelset.dat`（`LAST_LEVEL_SET`，`int AccessLevel[256]`，1024 bytes 二進位）。
  `Security_new.def` 是另一個檔，雖然也在 `/api/system/securityNew` 服務中，
  **接錯不會報錯，只會把權限寫到沒人讀的地方**。正確端點是 20260916 新增的
  `GET /api/system/levelset`（`kind:"i32"`）＋ `WS system.levels.put`，
  引擎側的形狀叫 `sysLevels`，接線檔是 `ht9045_wire_statussecurity.js`。

  ⚠ **「只有小鍵盤」不等於完成**：那 9 頁欄位打得了字，但按存檔不會寫回任何檔案。
  剩下 45 頁（`kb` ＋ `none`）抽取器找到的安全欄位與 props **都是 0**——
  不是抽取器壞了，是**那些多半是執行期顯示頁不是設定頁**，它們要的不是 `fields`，
  是 tag → widget 對照（見下節）。

  ⚠ 產生器對這幾頁產出的檔頭註解是從 `Config.Configuration` 複製來的
  （寫著「抽取到 1011 個欄位」，那是 Configuration 的數字，不是該頁的），
  看到別被誤導。

- ⚠ **頁名以 `Config.` 開頭的才是對的**（使用者 20260915 裁決）。
  曾經同時存在 `Setup.Configuration.html` / `Setup.DIOInterFaceCFG.html` 與
  `Config.*` 兩套，各自接不同的 wire js。`Setup.*` 那套已於 20260915 刪除，
  連帶修掉四處會靜默壞掉的引用：`ht9xxx-layout-base.js` 的版面位移 key、
  `sync_web.py` 的 `OURS` 清單、`KEYBOARD_AUDIT.md` 的重複列、
  以及抽取 JSON（`all.json` / `v2all.json`）裡的 `"page"` 值。
  **最後一項最要命**：`gen_wire.py` 的 `SYS_PAGES` 用 `Config.*` 當 key，
  而 JSON 存 `Setup.*`，兩邊對不上時重跑會產生錯名檔，且查表落空讓
  1015 個欄位全掉進 `pending` 變成死接線。改頁名時這五處要一起改。

- 新增的 js 都要進 `tools/sync_web.py` 的 `OURS`，否則同步的刪除階段會把它們移走。
  ⚠ `OURS` **不保護 HTML 裡的 `<script>` 行**：那些檔在鏡像來源裡存在，
  `--apply` 會整檔覆蓋而讓接線靜默失效（頁面照開、不填值、Save 沒反應）。

  ⚠ **20260921 實測**：`sync_web.py` 在樹上有**兩份**，內容不同 ——
  `tools\web-client\sync_web.py`（現行，`OURS` 有 60 筆）與
  `HT9011UC_Cpp_V3.33.906.0\tools\websync\sync_web.py`（**停在較早版本，少 15 筆**）。
  改 `OURS` 只改了現行那一份。另外鏡像來源 `D:\HT9045\incoming\web_html`
  目前**不存在**，所以這支腳本現在跑不起來 —— `OURS` 是預防性維護，不是當下的阻塞。

### 盤點「一頁到底接了沒」—— 先對現成的表，不要自己另算（20260922）

**`PAGE_WIRE_STATUS`（`screenshot_meta.js`，`page/ScreenShots.html` 表⑤）是權威表。**
68 頁、四級，由 `scratchpad/gen_page_status.py` 從**實體接線檔**量出來，不要手改。

| 級別 | 頁數 | 意思 |
|---|---|---|
| `data` | 21 | 接了 `/api/recipe` 或 `/api/system`，按存檔真的會寫回檔案 |
| `kb` | 10 | 只有小鍵盤：打得了字，按存檔不寫回任何檔案 |
| **`tag`** | **4** | **只接執行期唯讀顯示**（表⑥ `TAG_WIRE_STATUS`） |
| `none` | 33 | 完全未接 |

⚠ **`tag` 對顯示頁是完成態，不是缺口。** `Status.ShowBinSelect`（6 個 tag）與
`Data.SortCT`（8 個）的 `fields`／`optional` 是空的，接的全是 `tags:` ——
那是對的：它們是顯示頁不是設定頁，本來就該接 tag 而不是配方欄位。
接線檔的註解逐條對得回 `WebBridgeTags.cpp` 與 golden 的
`cShowBinSelect.cpp:49-50`／`cSortCT.cpp:92-94,213,399,412`。

⚠ **20260922 踩過的坑：做了一整天的事件盤點，卻沒先對這張表。**
於是把 `Data.LotInfo`／`Status.ShowBinSelect`／`Data.SortCT`／`Setup.BinSel`
歸成同一類「手寫示意頁、還沒翻」，被使用者質疑後查表才發現**兩頁有接、兩頁沒接**。
**任何關於「接了沒」的結論，先查表⑤。**

**三個軸要分開，混在一起就會得出上面那種錯結論：**

| 軸 | 問的問題 | 看哪裡 |
|---|---|---|
| 資料接線 | 值有沒有接上？（再分配方 `fields` / 執行期 `tags`） | 表⑤ `PAGE_WIRE_STATUS` |
| 事件行為 | 元件被操作之後有沒有人做事？ | `audit_dfm_events.py` |
| id 體系 | html 的 id 是不是來自 dfm？ | 上者的 `missing-el` |

`missing-el` 命中只代表「這一頁換了設計、id 對不上 dfm」，**與有沒有接資料無關**。

#### dfm 事件盤點工具 `scratchpad/audit_dfm_events.py`

回答「dfm 的 `OnChange`／`OnClick`／`OnMouse*` 有沒有人實作」。49 頁、5020 筆繫結，
**九桶**：`bound-real` / `keypad` / `cosmetic` / `exit-generic` / `data-only` / `none` /
`title-only` / `missing-el` / `menu-item`。判準**以 `classify()` 為準，不是 docstring**
（那兩處同步失敗過三次 —— v2 少一種、v3 少一種、skill 這裡也慢了一版）。

⚠ `title-only`＝**元件畫了，但名字寫在 `title=` 不是 `id=`，所以任何接線都接不上去**。
它既不是「沒畫」也不是「事件沒接」，**修法是改 HTML 加 id 不是寫 JS**，所以不能混算。
筆數兩個口徑不一致（複驗數 65、腳本量 51，**差 14 未查明**），引用時兩個都要講。

**報這個數字時，兩句話一定要一起講**（否則會被讀成完成度）：
1. `bound-real` 只證明「掛了監聽器」，**不證明行為與 golden 相同**。
   例：`HW.IoSetView` 686 顆 IO 鈕，golden 是 `iosetview.cpp:1070` 的 219 行
   （含 `IndexHasIC()` 互鎖與真空控制），web 只有 `classList.toggle('down')`。
2. `bound-real` 有 **721/825（87.4%）集中在 `HW.teach.html` 一頁**，全部來自兩條非字面綁定規則。
   **拿掉那一頁，其餘 48 頁只有 104/3642 = 2.9%**；49 頁裡只有 30 頁有任何一筆。

**三個會讓數字錯一倍的陷阱（兩輪對抗性複驗抓出來的）：**

* 🔴 **剝註解不能用正則。** 先剝區塊註解、後剝行註解 → 行註解裡的 `/*` 是活的，
  會開啟假的區塊註解。實測 `HW.teach.html:182` 的 `// ... JSON/js/*.js ...` 一路吃到
  `:418`，三頁誤吃 **55,438 字元** —— 而那正好是唯一會驅動馬達的三頁。要用詞法狀態機。
* 🔴 **「id 出現在 js 裡」不等於「掛了監聽器」。** 第一版這樣判，245 筆裡 57 筆只落在
  資料表列、5 筆只落在註解。要看 id 有沒有落在綁定敘述附近，**而且事件種類要對得上**
  （`watchValue` 綁的是 `change`，不該算成 `OnMouseDown` 有接）。
* 🔴 **計數／查證時不要用 `head -N` 截斷 grep，然後把結果當成全部。**
  20260922 一天踩三次：`grep "iUnLoaderCount=8" | head -3` 讓 [P32] 的四個進入點
  少報一個；`grep -o "Q7HK21A0339\|85\.0\|..." | head -8` 因為 `85.0` 剛好 8 次把
  緩衝佔滿，於是回報「那個字串不存在」（它在 `Data.LotInfo.html:57`）。
  要數就數完（`| wc -l`）或先 `sort -u`。
* 🔴 **`grep -o 'src="..."'` 會撈到註解裡的字串。**
  同日：以此「發現」`Status.TemperFrom.html` 還在載入已退場的 `settings.js`，
  差點去改一個正確的 skill —— 那三處全是註解，而 `ht9045-html-json` 早就寫了
  「grep 仍會命中 2 處，但那是註解文字…這裡踩過一次」。**查證前先看該主題的 skill。**
* 🟡 **非字面綁定認不出來。** `HW.teach.html` 用 title 正規式
  `/^(Motor[A-Za-z0-9]+) : TSpeedButton/` 與 `JSON/teach-access.json` 的 `techPoints[]`
  驅動綁定，逐字比對一筆都看不到 —— 漏掉 660 筆。加新頁時要檢查有沒有這類機制。

### 第三種形狀：行為翻譯層（golden handler → 瀏覽器）—— 20260921 新增

前面兩種形狀是**資料**（哪個欄位對哪個配方鍵）。第三種是**行為**：
golden 的 VCL 事件處理函式本身搬到瀏覽器。產生器做不出來，一律手寫。

| 檔 | 頁 | 搬了哪些 golden handler |
|---|---|---|
| `web\page\ht9045_setup_sitemap.js` | `Setup.SetUp.html` | `cSetUp.cpp` 的 `ScrollBar1Change` / `CompChange` / `chkOffCenterkitClick` / `btnLUpToRDownNClick` |
| `web\page\ht9045_contact_slk.js` | `Setup.Contact.html` | `cContact.cpp` 的 `scrbSLKChange` / `DutCount` / `ShowArmAndDeviceForce` / `CalculateTotalAirForce` / `GetMinForce` / `GetMaxIndexForceLimit` / `CalcDeviceForce` / `edAirForceChange` / `edPinCountChange` / `edForcePerPinNChange` / `edDieForcePerPinGChange` / `CountDieForceKg` |

載入順序：**放在該頁所有接線檔的最後**。兩支資料接線都會填欄位，
行為層算完會蓋回去 —— 這正是 golden 的順序
（`cContact.cpp:1166` `ReadFile` → `:1175` `fShow=true` → `:1178` `scrbSLKChange`）。
所以行為層要**明確再呼叫一次** `HT9045Contact.load()` / `HT9045Page.load()`
並等兩個都 resolve，不要賭誰先回來。

寫這一類檔之前，先知道這三個坑：

**(1) dfm 的 `TScrollBar` 產生出來是一個空的 `<div>`。**
`_gen_dfm_abs.py` 只畫一條靜態槽，點不動。全樹有 6 頁有 `TScrollBar`
（`HW.MotorTest` / `HW.teach` / `Setup.Contact` / `Setup.SetUp` /
`Setup.TrayAssignment` / `Status.LtcSensor`），每一頁都要自己升級成
可拖／可點槽／滾輪／鍵盤的真捲軸。上面兩支各有一份實作，形狀一樣，
差別只在 `Min`/`Max` 與 `OnChange` 接到誰 —— 抄其中一份即可。

**(2) VCL 的 `OnChange` 在 web **沒有**對應，要自己做。**
* VCL：`TEdit->Text = x` 觸發 `OnChange`，但 `TControl::SetText` 是
  `if GetText <> Value then …` —— **指派相同的字串不會觸發**。
  互相換算的兩個欄位（例如 N ↔ gf）能收斂，靠的就是這一道守衛。
* web：欄位是 `readonly`，只有 `qwerty.js` 的 `commit()` 會寫值，
  而它就是 `tgt.value = val`（`qwerty.js:74`），**不發任何事件**；
  DOM 也不會因為程式指派 `.value` 就送 `input` / `change`。

做法（`ht9045_contact_slk.js` 的 `watchValue()`）：把該欄位的 `value`
換成自己的 accessor，指派時比對舊值、**不同才**呼叫 handler。
語意與 VCL 一致，且 qwerty、接線的 `load()`、Console 手動指派三種來源都涵蓋。

⚠ **不要改 `qwerty.js` 讓它發事件** —— 那支全站共用，改它等於幫所有頁面的
每一個輸入框都加事件，影響面遠大於單一頁。
⚠ 建議加一道 golden 沒有的**遞迴深度上限**。golden 靠「相同文字不觸發」
＋四位小數來回換算剛好是不動點來收斂；浮點來回極少數情況差一個 ulp，
在瀏覽器裡就是整頁卡死。踩到要寫 `console.error`，不可靜默。

**(3) 不是純文字欄位的鍵，用存檔擴充點，不要另開側門。**
捲軸的 `Position`、radio 的 `ItemIndex` 進不了 `FIELD_MAP`（那張表是
「id → 一個文字框」）。`ht9045_contact_wire.js` 20260921 加了擴充點：

```js
HT9045Contact.addCollector('<名字>', function () {
  return { '<區段>': { '<鍵>': '<字串>' } };   // 併進 collect() 的 edits
});
```

仍走原本的 `preview`(dryRun) → 攤給人看 → 確認 → 寫入 → 重讀。
⚠ 回傳的值要對齊 golden `WriteIniData` 的格式（`common.cpp:887` double 是
`"%0.4f"`、`common.cpp:691` int 就是整數），否則 preview 每次都報 changed。

#### 不停機告警 NonStop（20260922）

**定義只有一件事：NonStop 模式下 C++ 那邊不呼叫 `StopAllMotor()`。**（使用者 20260922 裁定）

| | golden | 停機？ |
|---|---|---|
| `ShowErrorMessage()` | `note.cpp:805-808`：`if(Code!="WAR1635") StopAllMotor(); else StopAllMotor(false);` —— **兩條路都停**（`StopAllMotor(false)` 只跳過 Galil MTestY1 的 `VS0;SP0`，`myGALILmotor.cpp:4713-4719`） | 停 |
| `MyMessageBox` | `mymessbox.cpp:303` `if(!iUnLoaderCount){ ... StopAllMotor(); }` | `iUnLoaderCount != 0` 就**不停** |

[P32] Empty/Color Tray Pre Alarm 就是後者：`acatchtray.cpp` 四個進入點
（V910 `5467/5519/5606/5677`、**V912 `5655/5707/5794/5865`** —— Jimmy 用 V912 樹）
都設 `iUnLoaderCount=8`，註解寫「必須不為 0 Handler 才不停機」。

* ✅ **不需要新欄位。** 既有契約 `web\JSON\Message-dialog-request.json` 就有
  `requestedSideEffects: { pauseHandler, stopAllMotor, servoOffInArmXY }`，
  `stopAllMotor` 講的正是同一件事。路由第一順位就看它。
  （`show-error-message` 那條**沒有**這個區塊，因為停機不是可選項。）
* 檔：`web\config\AlarmNonStop.json`（對照表）、`Alert.Note.NonStop.html`、
  `Alert.MyMessageBox.NonStop.html`、`web\page\ht9045_nonstop_{alarm,page}.js`、
  `dialog-bridge.js`（多兩個顯示 kind ＋ `routeAndRender()` ＋ `raiseNonStop()`）
* ⚠ **把 code 加進對照表不會讓機台不停。** `StopAllMotor()` 在 request 送到瀏覽器
  之前就跑完了；加錯只會讓操作員看到寫著「機台未停機」的小視窗而馬達其實已停。
* 唯一真能保證不停的是 **web 端自己擋、完全不進 C++** —— 權限不足就走這條。

#### 權限：levelset 有管道，目前登入者的等級沒有（20260922）

| | 狀態 |
|---|---|
| **levelset（需求等級表）** | ✅ `GET /api/system/levelset`（i32，`levelset.dat` 256 個小端 int32）＋ `system.levels.put`；`ht9045_wire_statussecurity.js` 已在用 |
| **目前登入者 `AccessLevel`** | ❌ golden 是 `cmydef.h:3527` 的執行期全域，登入後才有值 —— 不在 ini、沒有 tag、wb_serve 沒端點。**要請 Jimmy 推成 `security.accessLevel`** |

語意（使用者 20260922 確認，與 golden 一致）：**0 是最低等級，數字越大權限越高**
（`cSecurity.cpp:589` `if(AccessLevel < LevelSet.AccessLevel[iType]) 擋`）。
⚠ 「**預設權限**」指的是**需求等級**（levelset 那一格的值），不是操作者的等級：
**debug 預設需求 0**（任何等級都過得去 → 操作者全部能操作）、**normal 預設需求 2**。
這個預設只在 levelset 讀不到或該 iType 沒有格子時才用得上。

⚠ **`Setup.Contact.html` 有兩支接線同時掛在 `spbSave`**
（`ht9045_contact_wire.js` 與 `ht9045_wire_engine.js` 都用 document 捕獲階段
認同一顆鈕）。20260921 量到 `click` 監聽器 2 個。`stopPropagation()` 依規範
不擋同一節點上的另一個監聽器，所以按一次 Save 應會跑兩次
「preview → confirm → write」。**未在真瀏覽器驗證**，待決。

---

## `/api/system/` 與 `/api/text/` —— 機台檔案的即時讀寫（20260915 實測）

`wb_serve.exe` 提供三條 API，取代 PowerShell 一次性產生的 JSON 快照。
以下數字是 2026-09-15 對跑著的 `wb_serve`（`--dry --allow-cmd`）實測回報，不是推估。

```
GET  /api/recipe/          作用中配方的 16 份文件
GET  /api/recipe/<doc>     {sections:{sec:{key:{value,type,raw}}}}
WS   recipe.doc.put        tag=<doc>  value={"sections":{...},"dryRun":bool}

GET  /api/system/          40 支設定檔的索引（name/path/kind/available/bytes）
GET  /api/system/<name>    ini -> 與配方文件完全同形狀
                           csv -> {columns:[...], keyColumn, rows:[{col:val}]}
WS   system.file.put       tag=<name>  value={"sections":{...},"dryRun":bool}
                           ack -> {changed, identical, notFound, backup}

GET  /api/text/            6 個純文字記錄來源（唯讀）
GET  /api/text/<root>      該來源下的檔案（單層）
GET  /api/text/<root>/<f>  整檔內容 {path, bytes, text}
```

### `/api/system/` 的 40 支（35 支本機實檔存在，5 支此環境未部署）

| 群組 | name | 路徑來源（golden 全域） |
|---|---|---|
| 核心四檔 | `gerneral` `teach` `motTable` `ioTable` | `asGeneralPath` / `asTeachPath` / `MotTablePath` / `IoTablePath` |
| config\ | `config` `lastSet` `description` `securityNew` `criticalPara` `esdConfig` `atcConfig` | `AuthPath` / `ConfigMemoPath` ＋ 固定檔名 |
| 動態解析 | `dio` | 依 config 開關＋`Tester.Data` 的 TypeName 算出，見下 |
| system\ 專屬全域 | `errNote` `setupInf` `trayForm` `plateForm` `trayStepSpeed` `machineLife` `arms` `secsGem` | 各有專屬全域 |
| system\ 固定檔名 | `contactInfo` `autoTemp` `atcSystem` `barcode` `padInterface` `eventLogLevel` `socketCount` `motorTest` `colorSensor` `mvData` `rpDefault` | `asSystemPath` ＋ 檔名 |
| Error\ | `alarmDesc` `alarmCodeList` | 本地常數（golden 該處寫死路徑，無全域） |
| PMAlarm\ | `pmMonth` `pmQuarter` `pmYear` `pmTemperature` `pmEsd` `pmIonFan` `pmSetting` | `sPMList_*` / `sPMSetting` |

此開發環境不存在（`available:false`，契約仍完整，上機台後檔案在即可讀）：
`arms`、`secsGem`、`colorSensor`、`mvData`、`rpDefault`。

**`dio` 是唯一動態解析的**：`ResolveDio()` 複製 golden 的 `GetDIOFileName()` 邏輯，
但**刻意不做** golden 那個 `CopyFile`——讀取不該有寫入副作用。
20260915 實測解析到 `D:\HT9045\iniData\DioCfg\5 Bit Binary(4ch).ini`。
使用者裁決：**DIO 只做讀目前生效的那一支，不做切換**。

### `/api/text/` 的 6 個來源（唯讀）

| root | 路徑 | 20260915 實測 |
|---|---|---|
| `releaseNote` | `config\ReleaseNote.txt` | 可用（單一檔） |
| `eventLogTxt` | `D:\HT9045_Log\EventLogTxt\` | 可用 |
| `jamCount` | `D:\HT9045_Log\EventLogTxt\SGJamCount\` | 此環境不存在 |
| `timeData` | `D:\HT9045_Log\TimeData\` | 可用 |
| `indexCycleTime` | `D:\HT9045_Log\IndexCycleTimeRecord\` | 此環境不存在 |
| `precaution` | `D:\PrecautionRecord\system\` | 可用 |

為什麼另開一條而不是塞進 `/api/system/`：設定檔是「固定路徑、鍵值結構、要能寫回」，
這些是「動態路徑、整檔純文字、唯讀」。日誌檔名依日期產生，固定表列不完。
**沒有對應的寫入指令**——這些是機台產生的記錄，不是人設定的東西。

### 五條設計約束，改這段程式前先讀

1. **固定 40 筆表，不做目錄掃描。** `system\` 是共用量產設定；掃描會在有人丟新 ini 進去的當下自動暴露它。寫死表列，路徑穿越在結構上不可能。`/api/text/` 的 root 同理是固定表，檔名另過 `SafeDocName`（擋 `..` 與路徑分隔字元），只列單層。
2. **路徑一律取 golden 全域，不寫死字串。** 好處是 `--dry` 對 `asGeneralPath` 的重導向自動生效。少數 golden 自己就寫死路徑、沒有全域的（`Error\`），才用本地常數，並在該處註明原因。
3. **`IniGet()` 是自寫的解析器，不是 golden 的 `ReadIniData`。** 後者會在鍵不存在時把鍵補進檔案——那是「讀取帶寫入副作用」，對唯讀 API 不可接受。
4. **寫入要 `--allow-system-write`，`--allow-cmd` 不夠。** AGENTS.md：「共用量產執行期參數。預設唯讀。要寫必須先備份，且要人明確同意」——這個旗標就是那個明確同意。20260915 實測：只帶 `--allow-cmd` 時 `system.file.put` 回 `{"ok":false,"error":"system writes need --allow-system-write ..."}`，閘門有效。
5. **逐位元組保留 + 備份 + 原子置換。** ini 沿用 `ht9045::RecipeDocApplyEdits`（它吃任意路徑）；csv 是新寫的 `CsvApplyEdits`，只換指名那一格，行尾與其他欄位原樣複製。備份為 `<path>.bak_<時間戳>_webwrite`。

⚠ **`--dry` 只保護 `Gerneral.ini` 與配方資料夾**，因為只有 `asGeneralPath` 與 DataPath 被重導向。
`teach.ini`、兩個 csv、`config.ini` 帶 `--allow-system-write` 寫下去就是**寫真檔**（有備份，但是真檔）。
這是已知缺口，尚未決定是否讓 `--dry` 涵蓋全部 40 支。

⚠ **`teach.ini` 特別小心**：`forms/fTeach.h` 記著這個專案已經因為一條**無聲失敗**的 teach 寫入路徑遺失過教導資料一次。所以 ack 一定帶 `changed/identical/notFound`，寫不中任何鍵會回報 `notFound` 而不是默默成功。

⚠ **`config.ini` 的寫入是新開的路徑**，不是移植既有的。golden 自己在 `cConfiguration.cpp` 的 `#if 0 // GATE (CFG4-seed)` 把寫入關掉了——那個閘門針對的是**啟動時無人值守補鍵**，不是操作員按存檔，但仍要知道這件事。

⚠ **`SetHttpRoute` 只存得下一條路由**（`WebBridgeServer.cpp:1590` 直接覆寫）。
所以 `/api/recipe`、`/api/system`、`/api/text` 必須共用一個進入點，由 `ApiRoute()` 內部分流。
曾經因為註冊 `/api/system` 把 `/api/recipe` 整條打死（全部 404）。

## 輸入途徑規格（20260915 定案，不可違反）

**機台上沒有實體鍵盤。**

1. **HTML 畫面本身不可以使用實體鍵盤** —— 所有文字框設 `readonly`，單擊叫出小鍵盤（與 golden 的 `OnMouseDown` 一致）。
   golden 點了不會開小鍵盤的欄位（`.dfm` 沒有 `OnClick`／`OnMouseDown`、也不在 `HTEditList`；或 golden `ReadOnly`），接線檔 `kb` 那一列寫 `null`：
   引擎不掛小鍵盤、欄位照樣 `readonly`、title 寫原因（20261004 KB-GOLDEN 2/2，`ht9045_wire_hwteach.js` 50 格）。沒有列＝照舊通用 QWERTY。
2. **只有小鍵盤出現時**才可以用「對應到小鍵盤按鍵」的實體鍵：可見字元（大小寫容錯；數字鍵盤的 `-`、`%`、`.` 也是）、
   Backspace→`⌫`／數字鍵盤 `BS`、Delete→`Delete`／`Del`、Enter→`Enter`／`OK`、Escape→`Abort`、Space→空白鍵（畫面上有才算）。
   小鍵盤上沒有的鍵一律不接受——小鍵盤開著時直接吃掉，不給後面的畫面（golden `ShowModal`；只放 F1～F12、Ctrl／Alt／Meta 組合鍵、單獨修飾鍵）；
   小鍵盤沒開時實體鍵完全無效。

實作在 `web/page/ht9045_wire_engine.js` 的 `physicalKeys()`（對照表 `PHYS_MAP`）：偵測 `.qkOv` 覆蓋層存在時把 keydown 轉成對應按鈕的 `click()`，用**按鈕文字**比對，不依賴 `qwerty.js` 內部結構。
實體鍵＝按畫面上那顆鍵；畫面上的鍵 20261004 起照 golden `TfQwertyKey`（KB-GOLDEN 1/2）：開窗整段反白、第一個鍵取代舊值、`-` 切換正負、
`%` 只輪換步進鍵刻度、`dp` 決定步進鍵的字、Abort＝放回原值再照範圍夾、沒有 ✕、點遮罩不關。細節見 `references/qwerty-keyboard.md`。

⚠ **沒有做在 `qwerty.js`**：那是網頁作者的檔案、存在鏡像來源，`sync_web.py --apply` 會整檔覆蓋。長久解法是網頁作者把它收進 `qwerty.js`。

覆蓋率紀錄：`HT9045/docs/KEYBOARD_AUDIT.md`（由 `scratchpad/kb_audit.py` 產生，可重跑）。
20260915 現況：A（有 golden 旗標＋夾限）1461／B（通用 QWERTY 無夾限）1233／C（無鍵盤）53。

## 已知陷阱（速查）

- 🔴 **ack 在成功時不回結果 → 整條存檔通路是啞的**（20260916 修，`WebBridgeServer.cpp:1204`）。
  `AckJson()` 的第三個參數叫 `error`，原本**只有失敗時才輸出**，所以 `wb_serve.cpp` 組好的
  `{"changed":N,"identical":N,"notFound":N}` 被整個丟掉，瀏覽器只收到 `{"ok":true}`。
  後果是兩層無聲失效：引擎的**規則 2（notFound 就拒寫）從未觸發過**，而 `changed` 永遠是 0
  讓 `save()` 停在「沒有任何值改變，不寫入」——操作員按存檔會看到綠色成功訊息，檔案卻沒動。
  `recipe.doc.put` 更是連結果物件都沒組（只 printf 到主控台），一併補上。
  ⚠ **推論**：20260916 之前「規則 2 沒擋下東西」不能當成對照表正確的證據，已接的頁要各驗一次。
- 🔴 **一律用 `cell.raw`，不要用 `cell.value`**。GET 回來是
  `{"value":0,"type":"float","raw":"0.000000"}`——`raw` 是檔案字面值，`value` 是解析後的數字，
  而**寫入只認 `raw`**。引擎原本填 `value`，於是讀 `"0.000000"` 填成 `"0"`，存檔寫回去就把檔案改了。
  實測：把剛讀出的值原樣送回，伺服器仍回報 `changed=9`。**操作員什麼都沒改、只按存檔，
  就會改掉 9 個機台設定值。** 驗收一定要有「讀進來原樣送出，changed 必須是 0」這一項。
- ⚠ **斷言「沒問題」之前先證明檢查有能力發現問題**。`wire_probe.py` 第一版用
  `ack.get('notFound') or []`，而當時伺服器根本不回這個欄位 → 一律空的 → 印 PASS；
  且未帶 `--allow-system-write` 時整個指令被閘門擋在 preview 之前，**也**印 PASS。
  連兩次假通過。現在探針會先檢查 ack 裡有沒有那些欄位，沒有就印「這次驗證無效」。
- ⚠ **抽取器要認齊 golden 的五種 ini 呼叫**（20260916 補）。只認
  `WriteIniData(` / `ReadIniData(` 會漏掉 `CheckAndReadIniDataGeneral`(730)、
  `WriteIniDataGeneral`(381)、`WriteIniDataNoLog`(38)——`*General` 是 3 參數、無檔名，
  固定寫 `Gerneral.ini`。`HandlerSys.cpp` 有 506 個 ini 呼叫卻只抽到 3 個欄位就是這個原因。
- ⚠ **不要假設「一頁對一個檔」**。`uteach.cpp` 同一個表單寫三個目標（`asTeachPath` 11 處、
  `asGeneralPath` 4 處、區域變數 `szDir` 1 處）。必須解析 `WriteIniData` 的**第 1 個參數**，
  否則那 4 個 `Gerneral.ini` 的鍵會被寫進 `teach.ini`——兩邊都有同名區段時**不會報錯**。
  檔名還會轉手兩層：`szDir.sprintf("%s", asTeachPath)`、
  `ExtractFileName/FilePath(asTeachPath)` 再餵給 `elTeach->ReadEditTextFromFile()`。
- ⚠ **同名變數在不同函式指向不同檔**。`HandlerSys.cpp` 的 `Str` 分別是
  `D:\GPIB9045\system\general.ini`、`D:\RS232Standard\System\Setup.ini`、一個目錄、
  甚至字串 `"HonPrec"`。所以檔名解析**只能認已查證過的全域別名**
  （`asTeachPath` / `asGeneralPath` / `asConfigPath`），認不得就回 `None` 不接，不要猜。
- ⚠ **非文字控制項的 id 掛在外框上**。`TRadioGroup` → `<fieldset id=X>` 內含
  `input[type=radio]`；`TCheckBox` → `<label id=X><input type="checkbox">`。
  直接對 `el` 取 `.value` / `.checked` 會拿到 `undefined` **而且不拋錯**，
  存檔就寫出一整排 `"undefined"`。一律走引擎的 `ctlGet()` / `ctlSet()`。
  另：`ItemIndex = -1` 是 VCL 的「未選取」，是合法值不是缺陷。
- 🔴 **`--dry` 曾把 web 的 `Gerneral.ini`／配方寫入導進 `%TEMP%` 暫存、結束時刪掉**（20260916 審查 A1，已修）。
  交付包 launcher 寫死 `--dry --allow-cmd`，所以 HandlerSys 214 欄的存檔會 ack 成功、重讀「正常」、伺服器一關就消失，
  而同一頁的 teach.ini 欄位卻寫真檔。現在 web API 一律指向真檔（`gRealGeneralPath`），`--dry` 只隔離 C++ 載入器。
  **通則：任何「驗證寫入成功」都要拿 `path` 欄位或實體檔 SHA 對，不能只看 ack 與重讀**——重讀跟寫入若指同一個假目標，永遠一致。
  另兩個相關落差：`--real`（不帶 `--dry`）時載入器結束 flush 會蓋掉 web 改的鍵（啟動有 WARNING）；`--dry` 下 live tags 顯示的是暫存副本，不反映 web 寫入。
- **審查代理抓到的另外三類（皆已修）**：`ctlSet` 填不進去仍會被 Save 送出 `-1`（→ `UNFILLABLE` 拒寫）；
  csv/ini 寫入值不驗 `, " \r \n`（→ `notFound`）；apply 不是 all-or-nothing（→ `notFound>0` 不寫）。
  **驗收要有「引擎級」探針**：wire_probe 直接把 raw 送回是繞過引擎的，看不到 ctlGet/ctlSet 的漂移；
  `window.HT9045Page.collectSysIni/collectSysCsv/unfillable()` 已露出給 DOM 級探針用。
- 🔴 **`build.bat` 編譯失敗時仍可能回 exit 0。** 20260916 缺 `<set>` 的 build 明明有 4 個 `error:`，
  背景任務卻報「exit code 0」。判定一律 `grep -q "Build OK" <log>`，不要信 exit code；
  `build.bat gate` 的判定也只看 `tools/gateverdict.sh`。
- **teach.ini 的鍵名 == widget id，對照表是靜態的**（20260916）。golden 用
  `new TECH_PARA(&par, <MotorEnum>, <TEdit>, "<Key>", ...)`（269）與 `TECH_TWOPARA`（59）綁定，
  區段 = 該馬達在 `Mot_Table.csv` 的 `Alias`。不要被 `WriteIniData(asTeachPath, MOT[MotorSelect].Alias, Key, ...)`
  那一行騙成「執行期算的」——往上追建構式。`extract_page_v2.py` 的 `via='TECH_PARA'`；
  同一 TEdit 綁兩個馬達（`setEditTestZSafePos`）記 `row['ambiguous']` 不接。
- **整列新增／刪除走 `system.csv.rows`**（20260916）：`{add:[{col:raw}], delete:[key], dryRun}` →
  `{added, deleted, notFound}`。識別欄必填不重複、格內不可含 `,` `"` 換行、刪除命中須剛好一列。
  引擎 `sysGrid.add / .del` 接管 `btnAdd*` / `btnDelete*`。
- ⚠ **csv 的寫入以哪一欄定位列，每檔不同**（20260916 修）。`CsvApplyEdits` 原本一律拿第 0 欄比對，
  `IO_Table.csv` 第 0 欄 `IOType` 有 8 組重複（Sensor×255…），改 A 會寫到同型別的第一列 B，回 `changed=1`
  不報錯。現在 `CsvKeyColumn()`：ioTable→`Alias`、motTable→`Motorname`；空 rowKey 與命中多列都回 `notFound`。
  **新增 csv 系統檔時先確認識別欄唯一，不唯一就不要接寫入。**
- ⚠ **`SplitCsv` 要連 `'\n'` 一起跳過**（20260916 修）。`CsvApplyEdits` 的 `lines[]` 帶行尾，表頭最後一欄會被解成
  `"In1Logic\n"`，最後一欄永遠 `notFound`。GET 看得到、PUT 寫不到——**驗收要有「整張表原樣回送 changed=0」**，
  只測幾格會漏掉邊界欄。
- ⚠ **WS 單則訊息上限 64 KB（`WebBridgeServer.cpp` `kMaxWsMessage`），超過是直接斷線不是回錯。**
  操作員一次改幾十格遠不到；探針整張回送（ioTable 9,002 格 ≈ 300 KB）就會 `WinError 10053`。
  探針分批（`wire_probe.py` 每批 ≤ 40 KB），引擎 > 56 KB 拒送並提示分批。
- **表格頁走 `sysGrid`，不是 `sysRows`**（20260916）。`HW.MotorTest` / `HW.IoSetView` 在 golden 是 `TStringGrid`
  「整張 csv 就是檔案」；引擎直接把 `/api/system/<csv>` 畫成表格、雙擊改、只送改過的格子、鍵欄唯讀。
  legacy 的 JSON 快照 grid 會非同步畫進同一個 host——**不要跟它搶**，把原容器 `display:none`、旁邊插自己的。
  Save/Load 鈕上還掛著 legacy handler，用 `document` 捕獲階段攔。設定在 `gen_wire.py` 的 `GRID_PAGES`。
- ⚠ **`wb_serve` 的預設 web root 是 `D:\HT9045\web`，不是交付包的 `HT9045\web`。** 在 `server\` 起
  `wb_serve.exe` 而沒給 `--root`，API 全部正常但每個靜態頁都 404——接線探針全綠、瀏覽器卻打不開頁面。
  用 `run_wb_serve.cmd <webroot>` 或 `--root D:\HT9045\web`。
- **沒有 Playwright 也能做瀏覽器級驗證**：`msedge --headless=new --dump-dom --virtual-time-budget=8000 <url>`
  會吐出跑完 JS 的 DOM（`scratchpad/grid_smoke.py`）。數 `<th>` 時用 `<th[\s>]`，不然 `<thead>` 也算進去。
- ⚠ **交付包的 `server/wb_serve.exe` 會過期**。20260915 發現它停在 09-14 的建置，
  只有 `/api/recipe`，沒有 `/api/system`、`/api/text`、`system.file.put`——
  照那個包部署，`Config.Configuration` 的 163 個欄位會全部 404。
  改完 `tools/wb_serve.cpp` 要記得把 `build/wb_serve.exe` 複製到
  `HT9045/server/`。快速判斷：`grep -ac 'api/system' <exe>`，0 就是舊的。
- ⚠ **重建前一定要先停掉跑著的 wb_serve**，否則連結器報
  `cannot open output file wb_serve.exe: Permission denied`（執行中的 exe 被鎖）。
- ⚠ **機台的 `D:\HT9045\page\` 與交付包的 `HT9045\web\page\` 會分岔**。
  20260915 實測：機台那份停在最早期版本（71 頁裡只有 `Setup.Contact` 有接線、
  只有 3 頁有 `qwerty.js`），而 30 頁接線成果全在交付包。
  兩邊都要改的檔（`ScreenShots.html` / `screenshot_meta.js`）改完務必雙向同步。

- **接線狀態列不要用 `position:fixed; bottom:0`**：Setup 頁的 Save/Exit 多半在底部
  Panel（`cHotPlate` 的 Panel2 在 `top:475px`），橫跨底部的固定列會把它們整個蓋住——
  看不到也按不到。表單寬 606px，停靠 `left:614px` 的右側空白區。20260914 踩過兩次。
- **小鍵盤的 min/max 若在 golden 是 C++ 執行期變數**（`InputLimit.dContactHigh` 之類），
  一律 `checkRange:false`，不要填猜的數字——猜錯的夾限會擋掉合法輸入或放過非法輸入，
  比沒有夾限更危險。`cContact` 42 個鍵盤裡只有 5 個有硬編上下限。
- **同名 widget id 跨頁存在**：`XCT1`/`XST1`/`XPitch1`/`YCT1`/`YPitch1`/`YST1` 同時出現在
  `Setup.HotPlate` 與 `Setup.TrayForm`。任何「依 id 反查 golden」的工具都必須先鎖定表單
  （用 `.dfm` 的 `object <id>:` 重疊度），否則會把對照表指到錯誤的 `.Data` 檔。
- grep_search 對大型 .dfm 會逾時 → 用 Python（d:\HT9045\.venv\Scripts\python.exe，含 PIL）。
- Alert overlay 必須在 background 頂層文件（z 20000）；登入層 `#dialogAuth` 21000、HTQwerty 99999 是唯二可出現在其上的元件。子 iframe（Alert 頁）內彈的鍵盤會被自己的 iframe 裁切（MyMessageBox 472×219），密碼/鍵盤一律由 dialog-bridge 在頂層處理。
- `HTQwerty.show(target, flags, {onCommit, onAbort})`：2026-09-02 新增回呼（原只寫回 target）；舊呼叫不受影響。
- Playwright 對 Alert overlay 的 `locator.click()`/`screenshot()` 會因桌面時鐘與 `.dbFlush` 閃紅動畫判定不 stable 而 timeout → 用 `evaluate(el=>el.click())` 與 DOM 量測。
- **PowerShell 5.1 `Get-Content | Set-Content -Encoding UTF8` 會把 UTF-8 中文讀成 cp950 再寫回 → 中文不可逆損毀**（2026-09-02 `_scan_dfm_shot.py` 中招，靠已產出的 screenshot_meta.js 重建）。改 .py/.js/.json 一律用 Python `encoding='utf-8'` 或編輯工具，禁用 PowerShell 文字取代。
- PowerShell 終端偶發損壞（Get-ChildItem 無法辨識）→ 換 Python one-liner。
- iosetview.dfm 元件數 ~3900、TMyLedLane×1182、TBtnPanelLane×664；產生的 HTML 很大屬正常。
- VCL Caption `&&` 顯示單一 `&`；dfm 字串 `#NNN` 為 Unicode 碼位。
- Playwright 點擊會被重疊 .win 攔截 → 先 page.evaluate 隱藏/提升 zIndex，或 postMessage 模擬。- **release 版 `title` 被 theme.js 移到 `data-htitle`** → 依元件名選取時兩者都要比對（teach 頁籤規則曾因此 count=0）。
- `_gen_dfm_abs.py` 会覆寫手工頁 → 新手工頁必加入 `NO_OVERWRITE`（目前：HW.IoSetView / HW.teach / HW.MotorTest / Data.Observer / Setup.BinSel / HW.HandlerSys / **Main.MotionView9050**）；跑前備份 `page\`。
- `HW.HandlerSys.html` 已列入 `NO_OVERWRITE`，因 TabSheet2 的 `handler-track-table.js` 將分散
   的 Loader/Unloader DFM 控制項整合為 Track Matrix。維持原元件作為 INI binding adapter；
   新增的 Fix1/2/4/5/6 安裝與 Auto4/5/6 Y Motor 僅寫入 `General-config.json.handlerTrackMatrix`
   （`runtimeSupported:false`），不可宣稱已由 BCB6 實機支援。Matrix 的 T3/T6 欄分別對照
   `e3TrayName`／`e6TrayName`。
- ComponentMap 章節 anchor 依 dfm 查表（`ANCHORS`），增刪 `JOBS` 要同步 `_ANCHOR_SEQ`。
- 瀏覽器快取：改 theme.js / *.js 後用 http 驗證可能吃舊檔 → URL 加 `?v=` 或 route 加 `Cache-Control: no-cache`。
- `showDirectoryPicker` 需使用者手勢且只在頂層執行；假 handle 存不進 IndexedDB（DataCloneError）已容錯。- logo 去背：PIL 將 RGB>235 像素 alpha=0（page/img/HonPrec.png 已處理）。
- file:// 各獨立分頁 localStorage 不同源 → 獨立開頁請用 `?theme=` 參數。
- postMessage 非同步：廣播後驗證需延遲 ~300ms。
- 視窗在可視區外（如底列 y=860）點選單會「看似沒反應」→ {open} handler 已加 scrollIntoView。
- **Exit/Close 按鈕關窗**：產生器對 Caption=Exit/Close 的按鈕型元件（TSpeedButton/TButton/TBitBtn/TPanel）
  自動加 `exitbtn` class，頁尾 JS 統一綁 `postMessage({closeMe:1})` 通知 background 關視窗；
  不可用 id 白名單（各表單命名不一：spbExit/sbtExit/sbExit/btnClose/btnOk/sbCleanExit）。
- **alClient 蓋住 alRight/alBottom 兄弟**：alClient 展成 right:0;bottom:0 會蓋掉 dfm 中排在前面的
  alRight/alBottom 元件（DIO Panel1 案例：整個右欄+Name+Exit 消失）→ `mark_alclient()` 預算
  兄弟佔用空間輸出 `right:{R}px;bottom:{B}px`。
- **release/debug 兩版本**：入口 `release.html`/`debug.html` → `background.html?mode=`；`theme.js` 讀 `?mode=` 設 `<html data-mode>`（**無參數預設 release**）；子 iframe 靠 `background.html` `withMode()` 附 `?mode=` 取得。新增 debug 專用元件請用 `html[data-mode="release"]{display:none}` 隱藏（已用於 Main.html #sbDebug、cBinSel .hint/.colHide、.srcnote）。**Release 另隱 cpp/dfm 內部資訊**：theme.js 將 `title` **移到 `data-htitle`**（靠 title 分派的程式需改讀 title||data-htitle，否則 release 事件失效——Main.html `comp()` 已修）、background.html `dispTitle()` 去標題括號。**footer(.srcnote) 隱藏後** background `winHeight()` 依 `cfg.footerH` 扣高度。
- **視窗框旗標**：`locked`＝無縮小/關閉鈕**且**不可拖曳（主視窗）；`noClose`＝僅隱藏縮小/關閉鈕但仍可拖曳；`fixed`＝無右下角縮放把手（不可調整大小）。gbControlBtn 用 `noClose:true`＋`fixed:true`。按鈕產生於 `(cfg.locked||cfg.noClose)?'':'…'`。
