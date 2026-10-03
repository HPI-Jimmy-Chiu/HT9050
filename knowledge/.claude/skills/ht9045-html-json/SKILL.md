---
name: ht9045-html-json
description: >
  HT9045 HTML Version（BCB6 GUI 網頁化模擬）開站時所需 JSON 資料規範。定義
  background.html 啟動時要載入哪些 JSON、各檔資料項為何。
  JSON 依來源分四大類：硬體設定檔（Gerneral.ini／IO_Table.csv／Mot_Table.csv／
  teach.ini 等，D:\HT9045\system\）、Config 檔（config.ini 功能開關）、
  Setup 檔／Recipe（D:\HT9045\IniData\Data\<Recipe>\ ＋ SetUp.inf ＋
  cbSetupFileName 清單）、生產記錄檔（Task/System/IO/Motor runtime、程式快照）。
  觸發關鍵字：JSON 啟動載入, 開站資料, background.html 預載, General-config.json,
  Config.json, View-rules.json, Setup-index.json, cbSetupFileName, edSetupFileName,
  labSetupFile, SetUp.inf, GetLastOpenFN, 硬體設定檔, config檔, Setup檔, Recipe,
  生產記錄檔, IO-config.json, Motor-config.json, Teach-config.json, Sim-scale.json,
  Task-runtime.json, System-runtime.json, 載入順序, JSON-only, file:// 墊片,
  HTSettings, HTJsonWriter, decideAll,
  wb_serve, /api/recipe, /api/system, /api/text, /api/system/levelset,
  system.levels.put, levelset.dat, Status.Security, 執行期 tag, tag 串流,
  HT9045Tags, snapshot patch, 接線分級, PAGE_WIRE_STATUS, TAG_WIRE_STATUS,
  C 路, golden 表單橋, editlist.get, editlist.save, CRouteOwner, kOwned, WebCmdGuard, busy:, W906_CMDGUARD_MS, HT9045Busy,
  wbserve-conventions, 同一行插入, 行號錨點, 檔尾附加, 函式指標安裝座, W906_XxxBody, W906_FRW_Install, 測試縫, getenv W906_, W906_LEVELSET_PATH, W906_LOGINDAT_PATH, W906_SOCKETIDLOG_ROOT, W906_PrintDataRedirects, 語法檢查, SIM／SHIP, 警告基準, W906IoClickGuardScope, motor.access action 級, editlist.save 運轉中, SystemStart||SoftStart, BeforeApply, Q14, Q15, CheckIniData, CRouteOwnerDio, 動態 DIO 檔名, Q3, form.event, Q40, gen_editlist --only, _expect, REPLACE, GATE, 冪等, form.event 已做, PageEventsRegistrar, g_evreg, k<P>_Events, RunPageEvent, ELOperable, 清單過期, TA_RadioIndex, TA_EvOnTab, activePageIndex, ELSetPageOrder, 開窗閘, kOpenGates, OpenGateRefused, no-gate, not-authorized, 關窗尾段, MainClickTail.h, CloseTailRunning, saveFlowAfter, S107-1, form.save 運轉中, IC_PasswordGuard, SU_DoPassword, ht9045_yieldmonitoring_c.js, InitialOK, 唯讀輪詢, 免權杖, contactct.get, observer.get, READ_ACTS, WebCmdGuard::Exempt, ui.windows.put, WebWindowRegistry, FShowConservative, 瀏覽器全關停產, W906_SocketIDLogBody, golden_root, W906_GOLDEN_ROOT, 產生器根目錄, golden 0618 切換, golden_methods, KEEP_V912, _hand_kept.py, keep-list, 手寫 bridge 留存表, E031_FormBridgeFullRun, formbridge_fullrun_check, 過期產生檔
applyTo: "**/*"
---

> **//Steven 團隊 20260930（St01 ST01-E3）** — 新增 [ws-link-hub.md](references/ws-link-hub.md)：一個瀏覽器分頁一條 WebSocket（外框 hub，`web\page\ht9045_link.js`）——接線、權杖（acquire 就地／takeover 一律送伺服器）、**hub 只會放開按著的 jog、從來不送 STOP**（M1）、relay 探測依網址、自測與對照組、踩過的坑。

> **//Steven 團隊 20260927 晚間（St01，HEAD db1b7638）** — `references/route-c-golden-bridge.md`：§3.0g `form.event` 改成現況（C++ 已做，
> 怎麼加一個事件、Tray Assignment 的三個寫法、分頁值 `activePageIndex`）、新增 §3.0h（開窗閘 `kOpenGates`）、§3.0i（關窗尾段兩批＋`form.save` 運轉中拒絕）、
> §3.0j（Yield 頁補件、要密碼的勾選框 fail-closed、R74 更正、`SU_DoPassword`），並標出位移的行號；`references/wbserve-conventions.md`：§3 `W906_SocketIDLogBody` 已裝、
> §5 測試縫補讀者，新增 §9（免權杖的唯讀輪詢）、§10（視窗總表與「瀏覽器全關停產」風險）。

> **//Steven 團隊 20260927 下午（St01，HEAD 89ccb4cc）** — `references/route-c-golden-bridge.md` 補 §3.0d～§3.0g
> （`editlist.save` 運轉中拒絕、BeforeApply 重播＋Q14、Q15 開頁不寫檔的讀法、`form.event`）、§4.1（產生器改 golden 行為的流程）、
> §6 Q3 動態 DIO 檔名，並更正過期行號；在 `tools\wb_serve.cpp` 與周邊動手的規矩（同一行插入、檔尾附加、函式指標安裝座、
> 防連點、測試縫、驗證、註解、誰的檔）新寫在 `references/wbserve-conventions.md`。

> **//Steven 團隊 20260927（St01）** — C 路對齊程式（HEAD 227b79db）：`tools/editlist/_integrated.txt` 35 個結構
> （新增 `ACTForm`／`Winway`／`Monitor`，`217e7e5e`，都沒有頁面），`references/route-c-golden-bridge.md` §6 總表補三列、
> `kOwned` 補 5 個檔、表註的手寫 FileRW 改成現況；伺服器防連點（`busy:` 回覆）寫進 `references/web-bridge-json-contract.md`
> §1.3 補註。被取代的舊句在 `references/archive/`。

> **//Steven 團隊 20260926** — C 路對齊程式（HEAD 8fad1522）：`references/route-c-golden-bridge.md`
> §3 改寫成 JSON→HTML（`editlist.get`）／HTML→JSON（`editlist.save`）兩段並附程式行號，§6 改成
> **全部 32 個 C 路結構的總表**（單一出處）；A 形狀 `TFTestIF` 退役（`f89be4ce`）、`D:\HT9045_ref`
> 退場（`3e0ebb92`，golden 改指主 repo V912）。被取代的舊句在 `references/archive/`。
> 當日完整變更紀錄：`D:\docs\ChangeLog\CHANGES_20260926_Steven.md`

> **//Steven 20260921** — 接線分級之外多了一層：**行為翻譯層**（golden 的 VCL
> handler 搬到瀏覽器）。它不改變本檔的 `data`/`kb`/`tag`/`none` 分級 ——
> 那張表問的是「欄位有沒有接到 `/api/recipe`、`/api/system`」，
> 行為層問的是「動這個元件之後畫面該怎麼變」，兩件事。
> 20260921 落地兩支：`ht9045_setup_sitemap.js`、`ht9045_contact_slk.js`。
> 規格與三個坑寫在 skill `ht9045-html-version` 的「第三種形狀：行為翻譯層」。
> 當日完整變更紀錄：`D:\docs\ChangeLog\CHANGES_20260921_Steven.md`

> **//Steven 20260916** — 實測重查並修正：四大類表的 ✅ 只屬於已廢棄的靜態 JSON 那條路；
> 新增 wb_serve 對照表、執行期 tag 實況與 `/api/system/levelset`；重寫「待辦」。
> 當日完整變更紀錄：`D:\docs\ChangeLog\CHANGES_20260916_Steven.md`

> **//Steven 20260915** — 本檔於 2026-09-15 更新。
> 當日完整變更紀錄：`D:\HT9045\CHANGES_20260915_Steven.md`


# HT9045 HTML Version — 開站 JSON 資料規範

> HTML 模擬專案（`D:\HT9045\`，對應 **BCB6** GUI）需要哪些 JSON 才能正確反映
> 「目前機台的相關設定與狀態」。本 skill 定義**資料**（哪些 JSON、資料項、載入順序）；
> **畫面**／元件轉換規則在姊妹 skill `ht9045-html-version`（本文內容原為其原則 8，
> 2026-09-02 搬移並擴充為獨立 skill）。

## ⛔ 先讀這一節：有三條路（20260916 裁決 A／B，20260924 新增 C，同日精簡）

- **A 路（靜態 JSON，已棄用）**：PowerShell／`_gen_*.py` 一次性產生的 JSON，服務已棄用的
  `D:\HT9045\page\` 舊樹。⛔ 不作資料來源，格式仍可參考。
  詳見 [route-a-legacy.md](references/route-a-legacy.md)。
- **B 路（`wb_serve` 檔案鏡像）**：`wb_serve.exe` 直接讀寫機台實體檔案（ini/csv 原始
  鍵值）＋執行期 tag 串流，服務 `D:\HT9045\web\page\` 交付包。端點對照、tag 實況、
  已知坑全部細節見 [route-b-wbserve.md](references/route-b-wbserve.md)。
- **C 路（golden 表單橋）**：與 B 路共用 `wb_serve` 傳輸，但 HTML 拿到的是 golden 表單
  開頁後的畫面狀態（值／`Visible`／`Enabled`／權限），存檔跑 golden 存檔流程（檢查／
  鉗制／權限／Lock by File），不是直接改檔。現況（Steven 團隊 20260927，HEAD 227b79db）：
  `tools/editlist/_integrated.txt` 35 個結構（24 個有頁面、`ContactForce` 有 `PageDesc` 但頁面未接、10 個沒有頁面）、
  24 頁在 `ht9045_wire_engine.js` 的 `GOLDEN_BRIDGE`
  （C 形狀 `WS editlist.get`／`editlist.save`），A 形狀（`/api/form`＋`form.save`）只剩
  `Setup.HotPlate.html`。**JSON→HTML**＝`editlist.get` 跑 golden `FormShow` 回 `lists`／`proxies`／
  `mustSend`，頁面 `gbLoad()` 套值並「先開後關」；**HTML→JSON**＝頁面 `gbSave()` 送全部套過值的
  `widgets`＋`answers`，伺服器檢查（開頁同權限／`mustSend`／不可改的丟掉）後跑 golden 存檔流程，
  ack `{saved, applied, ignored, kept, unknown, session{messages, asked, todo, trace}}`，頁面一律重讀。
  逐結構總表（tag／頁面／開機／讀檔鏈／檔案擁有者）與完整規格見
  [route-c-golden-bridge.md](references/route-c-golden-bridge.md) §3、§6。
  要在 `tools\wb_serve.cpp`（或 `FileRW\`、`WebCmdGuard.*`、`forms\fMain.cpp`）動手，先讀 [wbserve-conventions.md](references/wbserve-conventions.md)。
  golden 控制項事件（`form.event`）、開窗閘、關窗尾段見 route-c §3.0g～§3.0j；會一直更新的唯讀頁與視窗總表見 wbserve-conventions §9、§10。
  主畫面 Tools ▾／Config ▾ 選單鈕「按下去」（D-025，20261001）：頁面 `D:\HT9045\web\page\ht9045_main_st01_ev.js` initMenuOpen 在 window capture 攔下、送 WS `act.main.menuOpen`，C++ 照 golden sbSettingClick／sbConfigClick 擋（運轉中／按鈕灰／等級）並跑尾段（Config：SetWorkParameter＋UpdateMainOperateMode；兩個都送 SECS），放行才讓 main.html 原本的 toggleMenu 開選單；「看得見」照舊是 D-015 的 `act.main.menuVisible`——見 route-c §3.0l 末條。

「四大類 JSON」四張表裡每一個 **✅ 已轉換**，意思都只是「A 路曾經產出過這個檔案」，
**不代表畫面接得到資料，也不代表機台在餵它**（見本檔「⛔ 資料來源裁決」）。

## 四大類 JSON

只列 **B 路（`wb_serve`）實際在用**的端點、檔案、資料項與 BCB6 讀寫函式。
A 路原始四張表（含 ✅ 已轉換等狀態欄、Simulator 專用投影、View-rules／Define-index 等
B 路沒有端點的項目）逐字搬到[route-a-legacy.md](references/route-a-legacy.md)「四大類 JSON（A 路原始表）」。

### 1. 硬體設定檔 — 來源 `D:\HT9045\system\`

端點：`GET/WS /api/system/{gerneral,ioTable,motTable,teach}`（`WS system.file.put`／`system.csv.rows`）。

| 檔案 | 主要資料項 | BCB6 讀寫函式 |
|---|---|---|
| `Gerneral.ini` | `sections`（43 節/623 鍵）＋`quick`（customerCode/model/subModel/…）＋`uiMap`（HandlerSys 246 元件對照） | `HandlerSys.cpp` `WriteIniDataGeneral()`／`CheckAndReadIniDataGeneral()` |
| `IO_Table.csv` | `points[]`：`alias`/`ioType`/`direction`/`address`/`hw`/`timing` | `iosetview.cpp` |
| `Mot_Table.csv` | `motors[]`：`motorId`/`motorIndex`/`alias`/`group`/`axis`/`driverType`/`limits`/`params`（10 項） | `uMotorTest.cpp` `UpdateMotorParameter()` |
| `teach.ini` | `motorAxle`（121 筆）＋`techPoints`（`TECH_PARA` 267＋`TECH_TWOPARA` 52＝319 筆） | `uteach.cpp` |

> `Gerneral.ini` Section/Key 權威定義：skill `ht9045-general-ini`。

### 2. Config 檔 — 來源 `D:\HT9045\config\config.ini`

端點：`GET /api/system/config`；權限另走 `GET /api/system/levelset` ＋ `WS system.levels.put`。

| 檔案 | 主要資料項 | BCB6 讀寫函式 |
|---|---|---|
| `config.ini` | `sections`（70 節/1184 鍵）＋`uiMap`（988 元件對照） | `cConfiguration.cpp` `elConfig->Add()` |
| `system\levelset.dat` | `AccessLevel[256]`、179 個權限項目 | `cSecurity.cpp` |

> `config.ini`／`IniConfig` 全域結構權威定義：skill `ht9045-config`。

### 3. Setup 檔（Recipe）— 來源 `D:\HT9045\IniData\Data\<RecipeName>\` ＋ `D:\HT9045\SetUp.inf`

端點：`GET /api/recipe/`（16 文件）＋`WS recipe.doc.put`；目前配方名另由 tag `recipe.current` 提供。

| 檔案 | 主要資料項 | BCB6 讀寫函式 |
|---|---|---|
| `SetUp.inf`（第一行＝目前工作檔資料夾名稱） | 目前工作檔名稱 | `GetLastOpenFN()`（`common.cpp`） |
| `IniData\Data\` 目錄清單 | 子資料夾名稱清單 | `main.cpp` `FindFirst`/`FindNext`（~9100-9134） |
| 各 Recipe 11 個 `.Data` 檔（TestMode／HotPlate／HandlerCondition／Contact／Temperature／Binasgn／ArmCondition／Tray／Tester／Rotate／UdUld） | 各功能頁資料 | `ReadTestMode()`／`ReadContactFile()`／`ReadTrayFile()`／`ReadArmCondition()`／… |

> Recipe 11 個 `.Data` 檔完整格式、7 個 `Binasgn` 變體、`ChangeRecipe()`/`DataPath` 切換流程
> 權威定義：skill `ht9045-recipe`。

### 4. 生產記錄檔 — 機台運轉期間產生／即時狀態

端點：**執行期 tag 串流**（WS `/ht9045`，snapshot ＋ 500 ms patch）＋ `GET /api/text/`（6 個唯讀記錄來源）。
這類原本以 30 餘個 `Production-*.json`／`*-dialog-*.json` 個別檔案契約表達（A 路設計），
B 路沒有逐檔對應，改走 tag 串流既有 prefix（`site`／`status`／`bin`／`temp` 等）；
`task.*`／`system.*` 兩個 prefix **目前不存在**；`io.*`（`io.di`／`io.do`／`io.ver`…，
`JsonBridge/ChanIo.cpp`）與 `motor.*`（`motor.axes`／`motor.count`／`motor.ver`，`ChanMotor.cpp`）
自 json-bridge S9 起已有（沒編 PCI1203 的 exe 上值是 null）；IO／馬達頁另走
`GET /api/struct/io/*`、`/api/struct/motor/*`（例 `web/page/HW.IoSetView.html:433`、`HW.MotorTest.html:227`）。
> **20260930 第 2 階段 E**：這兩條輪詢照 golden `if(fShow==false) return;`（V912 `iosetview.cpp:162`、`uteach.cpp:1366`）**視窗關著不拿**——`HW.IoSetView.html`（200 ms）、`HW.teach.html`（1 s，連 `teachSyncHome` HOME 彈起；關窗那一下照 golden FormClose→AllBtnUp `uteach.cpp:2092`／`:5346-5358` 當場彈起 `#btnHome`，不送指令）、`HW.MotorTest.html`、`Main.MotorView.html` 都聽外框的 `HT_WIN`，`pollAllowed()＝!winHosted || winShown`，縮小算開、開窗立刻拿一次、沒收過 `HT_WIN` 照舊一直拿；新頁要輪詢 runtime 也照這個寫。ctest `Stream2E_PagePolls`；細節 `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\stream-by-open-page.md` §4.5。
tag 實況數字、prefix 分佈、已知坑見 [route-b-wbserve.md](references/route-b-wbserve.md)。

### 5. 其他讀寫檔（2026-09-09 補完轉換，人工稽核清單見 `page/ScreenShots.html` 表③）

原四大類之外，BCB6 原始碼仍有 30 餘個非 `D:\HT9045_Log` 的讀寫檔（系統級 `.ini`／`.csv`／`.def`／
`.dat`／`.txt`），2026-09-09 已依下列規則全數處理，**不再視為待辦**：

| 處理方式 | 檔案 | 對應輸出 |
|---|---|---|
| 併入既有 i18n（不建 JSON） | `system\Language.csv` | `page/i18n.js` 新增 `HTI18N.dfm`（190 筆 Form.Component→{en,zh}）＋ `HTI18N.td(form,name,lang)` |
| 直接內嵌對應 HTML（不建 JSON） | `config\ReleaseNote.txt` | `Data.Observer.html` `#Memo1`（`readonly` textarea，靜態文字＝cObserver.cpp 硬編歷史＋檔案全文） |
| 使用者指示忽略 | `MDB\Handler.db3` | 不產生 JSON；歸 MDB Updater 專案管轄 |
| 確認已由既有 JSON 涵蓋 | `cbSetupFileName` 掃描 `IniData\Data` 資料夾名稱 | `Setup-index.json.list[]`（既有，無需新工作） |
| 產生標準 JSON（`_gen_misc_config_json.py`） | `ContactInfo.ini`／`AutoTemperature.ini`／`ATC.ini`(x2)／`Barcode.ini`／`PadInterfacePara.ini`／`EvenLogLevel.ini`／`TrayStepSpeed.ini`／`SpecialErrNote.ini`／`DioCfg\*.ini`／`Security_new.def`／`CriticalParaControl.ini`／`ESDconfig.ini`／`SocketCount.ini`／`IniData\Offset\*`／`IniData\SaveByMachine\*`／`AlarmCodeList.txt`／`AlarmDescription.ini`／`Error\<Lang>\*.dat` 索引／`PM_*.ini`(7 檔)／`ProductionInfo\PI_Setting.ini`／`TrayForm.csv`／`PlateForm.csv` | 對應 `<Name>-config.json`／`-index.json`（見下表，`state:"ready"`） |
| schema-only（此開發環境未部署來源檔，或為二進位不可臆測） | `ARMS.ini`／`ColorSensorType.ini`／`MVData.ini`／`RPDefault.ini`／`SECS\SECS\SYSTEM\Gerneral.ini`／`NSKit.txt`／`C:\Windows\SitMap.ini`／`IniData\Data\<Recipe>\DeviceCorrespond.ini`／`lastdata.dat`＋`login.dat` | 對應 JSON 仍會產生，但 `state:"missing"` 或 `"pending-cpp-projection"`，待實機或 C++ 端補值 |

`lastdata.dat`／`lastdata_backup*.dat`／`login.dat`（`LAST_GENERAL_SET LastSet`）**僅產生
`LastSet-projection-contract.json` 契約骨架**，不做二進位解析——依 skill `ht9045-array-audit`，
此結構欄位、對齊與版本相容性只能由 C++ 端依 `cprod.h` 定義逐欄位輸出，Python 端臆測有資料錯置風險。

完整 31 個新 JSON 檔名、來源、狀態列表見 `page/screenshot_meta.js` 的 `window.FILE_IO_STATUS`（`page/ScreenShots.html` 表③即時渲染，含 `done`/`i18n`/`embedded`/`ignored`/`pending` 五種狀態徽章）。

### 更新觸發與頻率分類
B 路對應見 [route-b-wbserve.md](references/route-b-wbserve.md)「同五個分類在 B 路
（wb_serve）的對應」；完整舊表見 [route-a-legacy.md](references/route-a-legacy.md)。

### ⛳ B 路的正式契約另有專文（20260916 新增）

`D:\HT9045\.github\specs\web-bridge-json-contract.md` —— **要交給實作 wb_serve／底層 C++ 的人看的就是那一份**。
本 skill 記的是「有哪些資料、分幾類」；契約記的是「線上長什麼樣、誰負責產生哪個欄位」。

契約裡與本節直接相關的三個決定：

1. **五個 metadata 欄位不掛在每個 tag 上**，改成訊框層 `seq`／`at`／`trigger` 三個。
   照字面每 tag 掛五個 → 117 tag × 5 欄 × 每 500ms，metadata 比資料還大，而且會撞
   `WebBridgeServer.cpp:258` 的 **64 KiB 單則訊息上限（超過是直接斷線，不是回錯誤）**。
2. **`available` 不做成欄位** —— 現行協定已用 `null` 表達不可知。
3. **`updateClass` 留在 HTML 端**（接線檔的 `tags:` 區塊），它是設計時決定的靜態屬性，不必上線傳。

### 值域（min／max）與同五個分類在 B 路的對應——B 路細節

⚠ 值域保護現況（哪些端點有 min/max 檢查）與 A 路五個更新分類在 B 路的實際對應
（`startup-once`／`operator-event`／`production-event`／`forced-poll`／`controller-poll`），
含 tag 協定缺 metadata 的細節與 Motion View 欄位對照，見
[route-b-wbserve.md](references/route-b-wbserve.md)。

## ⛔ 新的 tag 串流／頁面輪詢：一律照筆電的規則（RULINGS_20260930 第 12 條，20260930 起）

- **誰做**：網頁更新頻率（C++ 的 tag 整理／發布、hub、各頁輪詢）由筆電全部接手（Jimmy 20260930 20:4x「你能夠全部接手做嗎？後面通知st01按照我們做法」「有開的網頁才能更新資料」）。St01／St02 **不做**串流／輪詢／發布的改動；新頁面、新 tag 照下面的規則寫。St01 自己寫過的第 1 階段／2E 已在 review6 revert（`c43cee9e`／`b56e1165`），筆電的版本在 main（`40ab8374`，TO_STEVEN 20260930 22:0x 那一列）。
- **規則（筆電 TO_STEVEN 20260930 22:0x）**：
  - 只有**一個視窗**在讀的 tag 家族：在整理（stage）那裡問 `W906_PageStreamWanted("<background.html 的視窗 id>")`，回「不要」就不整理（例：1203 區塊問 `pci1203`、Motion View 的 `motionView.trays.*`／`screenScale*` 問 `motionview`，後者是在 `tools\wb_serve.cpp` 的 `PublishExtraTags` 呼叫端問，不改 St02 的 `ChanMvTrays.cpp`）。
  - 頁面自己的輪詢：照 `D:\HT9045\web\page\HW.MotorTest.html:848-851` 的 HT_WIN 寫法——`pollAllowed = !winHosted || winShown`，開窗那一下（關→開的邊緣）立刻拿一次；從沒收過 HT_WIN（單獨開頁）照舊一直拿。
  - 不確定就照送；最小化算開著；一條 WebSocket 都沒有＝沒有收件人。不看網頁的使用者（事件記錄分析、SECS SV、REST API、TCP 7016／7017）讀的一律照整理。
- **量測**：`[STREAM]` 每 10 秒一行（筆電的 `WebStreamStats.cpp`，主控台＋操作紀錄 kind STREAM）。檔名跟 St01 revert 掉的那份一樣，St01 淨變動是 0，合 main 時確認一次。
- **stage F（筆電，main `e977284a`，20260930 23:5x 已上）**：hub（`D:\HT9045\web\page\ht9045_link.js`，約 :600-660）＋外框 `background.html` postWinState——
  - 視窗**關著／從沒開過**的 iframe 頁：**收不到任何 tag snapshot／patch**；只在 socket 開的時候收一份小的 `BOOT_TAGS` 快照（`/^(auth\.level|machine\.gpibModel|site\.arm[12]\.s\d+)$/`）；**開窗那一刻**收一份整張的合成快照（synthetic snapshot）。最小化算開著。
  - 外框自己（background.html／main.html）、dialog-bridge 蓋層、單獨開的頁：照舊全送。alarm／modal／query／link.*／ack 照舊每個 client 都收。
  - **St01／St02 頁面的規則**：視窗關著時**不要靠 tag**。頁面載入時只看一次某個 tag 來決定事情的，二選一：①把 tag 名告訴筆電加進 `BOOT_TAGS`；②在 HT_WIN 開窗邊緣（關→開）重讀一次。視窗關著時因為 tag 變化而去做事的（送指令、彈按鈕、續約權杖…）現在關著就不會發生——要照 golden 判斷是不是本來就該停（golden 表單的 Timer 有看 fShow 才停）。
  - 主畫面（main.html）與外框的腳本不受影響；受影響的是跑在 `.win` iframe 裡的頁。
  - ⚠ **開窗邊緣重讀要用 `on()`／`subscribe`，不要在 HT_WIN 處理裡直接 `get()`**：外框 `background.html:697`（origin/main）先送 HT_WIN、再叫 `HT9045Link.windowState`，合成快照走 MessagePort（`ht9045_link.js:522`），兩條路沒有保證順序 ⇒ HT_WIN 裡 `get()` 多半拿到舊值（ST01-E 派的唯讀稽核 20261001 00:3x，沒實測）。另：頁面在 iframe load 回報「關」之前就 hello 的話，開機那一次仍可能收到整張快照（`ht9045_link.js:196`），所以關著的頁載入時讀 BOOT_TAGS 以外的 tag 結果不固定。
  - St01 頁面稽核（20261001 00:3x，唯讀）：St01 的 iframe 頁**沒有**要加 BOOT_TAGS 或要改的；Status.Security（`auth.level`）、Data.LotInfo 的幾個「tag 變了就送指令」路徑關著時暫停、開窗後值有變才補跑（golden TfSecurity 也只在 FormShow 排版）；Setup.Contact 的 `site.arm*`、HT9045Live 的 `machine.gpibModel` 都在 BOOT_TAGS 裡。
    - D-019（20261001，ST01-E，ctest `D019_D020_PageSelftest`）：Setup.Contact 在 HT_WIN 關→開邊緣照 golden TfContact::FormShow（V912 cContact.cpp:1186 scrbSLKChange）立刻重算 Contact Force，再用 `HT9045Tags.subscribe` 等邊緣後 2 秒內的 `site.arm*` 變動補算一次（stage F 的開窗快照比 HT_WIN 晚到）；最小化不算邊緣、從沒收過 HT_WIN（單獨開頁）照舊（`D:\HT9045\web\page\ht9045_contact_slk.js` §18）。
    - D-020（20261001，ST01-E，同一支 ctest）：Data.LotInfo 的 `lot.tab.*` 把「選中的頁」藏掉時只換畫面、不再 `click()`（以前會送 lotinfo.op selection.get／lotEnd.state，selection.get 缺鍵時會寫 Security_new.def）；golden pgLotinfoChange（V912 uLotInfo.cpp:7340）只掛在 uLotInfo.dfm:98 OnChange＝使用者點頁籤，全樹沒人直接呼叫（`D:\HT9045\web\page\ht9045_lotinfo_wire.js` showTabOnly）。
    - D-021（20261001，ST01-E，ctest `D021_ContactScrollSync`）：Setup.Contact 的 scrbSLK 捲軸位置只有一份——`#scrbSLK.value` 改成 SB.pos 的 accessor（`D:\HT9045\web\page\ht9045_contact_slk.js` §19）：捲動後引擎存檔（`ht9045_wire_engine.js` gbValue :1250）與 form.event 的 state（`ht9045_contact_ev.js` :146）讀到的是現在的位置（以前一直送開頁那個值，改了 Head Device Mode 存不進去）；引擎每次 editlist.get（gbApply :1163，C++ 每次都跑一次 golden FormShow）與 ack（contact_ev :237）給的 Position 照 golden 906 cContact.cpp:1169（V912 :1185）採用，該批套完再照 :1170（V912 :1186）scrbSLKChange 重算一次（重新開窗放回檔案值，同 golden FormClose 906 :1820-1822（V912 :1847-1849））；頁面自己捲動不回寫 `value`（沒有迴圈）。引擎沒改、沒有新的串流／輪詢。
    - D-022（20261001，ST01-E，同一支 ctest `D021_ContactScrollSync`）：Setup.Contact 開頁時視窗關著就**不再**叫 `HT9045Page.load()`（以前開站那一刻就一次 editlist.get＝C++ 跑 golden FormShow＋記 Enter，繞過引擎 H4）；照引擎 H4 同一套判斷（同一個 HT_WIN、同一個 3 秒）：關著 ⇒ 等第一次開窗引擎自己讀的那一次回來（包 `HT9045Recipe.editlistGet`，同 contact_ev）才跑 §17 的 fShow／Position／scrbSLKChange＝golden 906 cContact.cpp:1078（V912 :1094）FormShow（只在 Show 時跑，906 main.cpp:27324（V912 :28314），E-032 20261003）；開著（含最小化）、3 秒沒收到 HT_WIN、單獨開頁照舊各讀一次（`D:\HT9045\web\page\ht9045_contact_slk.js` §20；引擎沒改）。
    - D-023（20261001，ST01-E，同一支 ctest）：Setup.Contact 的 rgKitDiameter 開頁時採用 C++ 給的 ItemIndex（`HT9045Page.golden().page`），不再拿配方檔的 [Mode] Kit Diameter 重選；選項也照 golden FormShow（906 cContact.cpp:1108-1128（V912 :1124-1144））只列 `[SLK Type] Visible=1` 的口徑（這台 Type=30,40,60,56,80／Visible=1,1,1,0,1 ⇒ 30,40,60,80；以前多列 56，80mm 存成序號 4、超出 C++ 的 4 顆清單）；頁面自己的開頁讀取排在讀檔之後，C++ 的序號一定套在重建後的選項上（否則引擎可能判「填不進」整頁拒存）；C++ 沒給才用配方檔對應（`D:\HT9045\web\page\ht9045_contact_slk.js` §5／§15／§17）。
- 評估與現況調查：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\stream-by-open-page.md`（檔頭註明已改由筆電實作）。

## `/api/system/` 與 `/api/text/` —— 機台檔案的即時讀寫

`wb_serve.exe` 提供三條 API（`/api/recipe`、`/api/system`、`/api/text`），取代
PowerShell 一次性產生的 JSON 快照。端點清單、`/api/system/` 40 支表、`/api/text/`
6 個來源表、五條設計約束（固定表不掃目錄、`--dry`／`--allow-system-write` 邊界等），
完整內容見 [route-b-wbserve.md](references/route-b-wbserve.md)。

## ⛔ 「四大類」只存在於本文件，程式裡沒有（20261002 實測）

線上格式是**完全扁平**的 `{"type":"snapshot","gen":N,"data":{tag:value,...}}`
（`WebBridge/TcpTagLink.cpp:122`），狀態是一張 `std::map<std::string,TagValue>`
（`WebBridge/TagSnapshot.h:76`）—— **沒有分類欄位、沒有分組**。這是刻意的（見上面「⛳ B 路的
正式契約」那三個決定），所以**分類的唯一載體是 tag 名字的點號前綴，沒有程式在強制它**。

程式裡實際有的是三套互不相通的局部分類：`StreamFamilies`（4 桶，只為了印 `[STREAM]`）、
`W906_PageStreamWanted`（按消費頁面，**全樹只接了 3 個家族**）、`IsBuildFactTag`（測試白名單）。
**沒有 tag registry。**

20261002 對跑著的 `wb_serve` 普查（`tools/webprobe/channel_census.py`）：
**1790 個 tag 裡 1089 個（60.8%／43.1% 位元組）沒有任何網頁在讀**，其中 `secs.*` 772 個
在整個 `web/` 一次都沒出現（它的來源是 `PLAN_START_TO_RUN.md:2591` 的驗收條件，不是畫面需求）。
20 秒觀測窗內真的變過的只有 **6 個** tag。

⇒ 數字、量法、三個會讓人白做工的坑（**不要自己寫 WS client**、**op log 會汙染 apiCache 計時**、
**這台要用 `py` 不是 `python`**）、無磁碟量測檢查單、以及能省多少，全部在
**[stream-census-20261002.md](references/stream-census-20261002.md)**。

⛔ 誰可以動：`RULINGS_20260930` 第 12 條 —— 串流／輪詢／發布的改動**由筆電接手**，St01／St02 不做。

## 待辦（本 skill 的落地缺口）

範圍已由使用者收斂：真實 IO 後端由其他工程師實作，不歸這裡；這邊的目標只有一個——
讓 HTML 接上 `wb_serve`。B 路接線分級（20260919 實測 68 頁）、tag→widget 對照
（61/4608）、引擎綁定模式、還沒做的項目、相關契約，完整內容見
[route-b-wbserve.md](references/route-b-wbserve.md)。


## 與其他 Skill 的關係

| Skill | 關係 |
|---|---|
| `ht9045-html-version` | 頁面／元件轉 HTML 規則、產生器、release 模式、離線 JSON 寫入；本 skill 的姊妹篇，聚焦「資料」而非「畫面」 |
| `ht9045-recipe` | Setup 檔／Recipe 11 個 `.Data` 檔權威格式定義 |
| `ht9045-general-ini` | `Gerneral.ini` Section/Key 權威定義（`specs\gerneral-ini-schema.md`） |
| `ht9045-config` | `config.ini`／`IniConfig` 全域配置結構權威定義 |
| `ht9045-state-record-analysis` | `Task_ListWithTime.csv`／`MainProcMonitor` 分析方法，對應 `Task-runtime.json` 設計來源 |
| `ht9045-json-bridge` | C 路的 C++ 端：產生器操作與陷阱（`references/generators.md`）、逐頁讀寫驗收（`write-inventory.md` 〇）、沒有頁面的表單建頁備忘（`pending-pages.md`）、值是假的原因（`porting-gaps.md`）；C 路結構總表本身在本 skill 的 `route-c-golden-bridge.md` §6 |

## 產生器（沿用 `ht9045-html-version` 的工作副本）

B 路的產生器是另外兩支：`HT9011UC_Cpp_V3.33.906.0\scratchpad\gen_wire.py`
（從 golden 機械產生每頁的 `ht9045_wire_<slug>.js` 接線資料）與
`scratchpad\gen_page_status.py`（量出表⑤／表⑥ 的接線分級）。


完整 A 路產生器清單（`_gen_ini_json.py` 等 16 支腳本與對應輸出）見
[route-a-legacy.md](references/route-a-legacy.md)；工作副本在 `D:\AI_TempFile\`，
保存版在 `d:\HT9045\.github\skills\ht9045-html-version\scripts\`。
