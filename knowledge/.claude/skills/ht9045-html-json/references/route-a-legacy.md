# A 路（靜態 JSON）— 已棄用路線的舊規格

**這是什麼**：`ht9045-html-json` skill 早期規範的「A 路」——PowerShell／`_gen_*.py`
一次性把 BCB6 讀的 `.ini`／`.csv`／`.dat`／`.Data` 轉成靜態 JSON（`D:\HT9045\JSON\`），
搭配 `JSON\js\*.js` file:// 墊片與 C# JSON Simulator，讓 `D:\HT9045\page\` 舊樹離線跑。

**為什麼搬到這裡**：Steven 20260924——A 路已於使用者 20260915 裁決棄用（見 SKILL.md
「⛔ 資料來源裁決」一節，該節仍留在 SKILL.md 本文）；現行資料一律走 `wb_serve` 的
`/api/recipe`、`/api/system`、`/api/text`（B 路）。這些 A 路細節在日常工作中已用不到，
搬過來是為了不占 SKILL.md 版面，但保留內容以備查考。

**什麼時候還用得到**：
- 需要讀懂或清理 `D:\HT9045\page\`（file:// ＋ Simulator 那棵舊樹，交付包是 `D:\HT9045\web\page\`，兩者不同）。
- 需要追溯某個 JSON 欄位形狀／`*-ack` 協定當初怎麼設計的（欄位形狀仍可參考）。
- 需要理解「哪些資料該一次讀、哪些該事件更新、哪些該輪詢」這個分類邏輯的原始版本
  （分類概念仍對，只是 A 路的「JSON 檔」載體已換成 B 路的 API／tag 串流）。

以下內容為原 SKILL.md 對應章節之逐字搬移，未作內容修改（相對連結因搬移深度改變而調整，
內容本身不變）。

---

## 四大類 JSON（A 路原始表）

Steven 20260924：原 SKILL.md「## 四大類 JSON」四張表含 A 路 ✅ 已轉換等狀態欄，
以及 Simulator 專用投影、`View-rules.json`／`Define-index.json` 等 B 路沒有端點的項目。
SKILL.md 已改寫成「B 路實際在用」的精簡版，本節保留原始四張表逐字全文備查。

### 1. 硬體設定檔（Hardware Config）— 機台實體選配與硬體表，來源 `D:\HT9045\system\`

這類描述「這台機器實際裝了什麼」：機型／驅動卡／馬達清單／IO 清單／教導位置，變動頻率低，
通常只在安裝/維修時才改。

| BCB6 來源檔 | JSON | 狀態 | 主要資料項 | BCB6 對照 |
|---|---|---|---|---|
| `system\Gerneral.ini` | `General-config.json` | ✅ 已轉換 | `sections`（43 節/623 鍵）＋`quick`（customerCode/model/subModel/serialNo/machineId/factory/version/useATC/secsGemSystem/motionCardType/indexMotionCard/ioCardType/indexDriverType/trayArmMode/…）＋`uiMap`（HandlerSys 246 元件對照）＋`handlerTrackMatrix`（HTML 匯出延伸） | `HandlerSys.cpp` `WriteIniDataGeneral()`／`CheckAndReadIniDataGeneral()` |
| `system\IO_Table.csv` | `IO-config.json` | ✅ 已轉換 | `points[]`：`alias`/`ioType`/`direction`/`address`(lane/ip/port/bit/ioCode)/`hw`(moduleType/isaBase/inType/enable)/`timing`；`index.byId/byAlias/byIOCode` | `iosetview.cpp` |
| `system\Mot_Table.csv` | `Motor-config.json` | ✅ 已轉換（初版由畫面端建立，非 csv 逐列自動轉換器） | `motors[]`：`motorId`/`motorIndex`/`alias`/`group`/`axis`/`driverType`/`limits`/`params`（10 項，對應 `strngrdMotor` 設定表）/`enableCondition` | `uMotorTest.cpp UpdateMotorParameter()` |
| `system\teach.ini` | `Teach-config.json` | ✅ 已轉換 | `motorAxle`（121 筆軸控鈕→motorId）＋`techPoints`（`TECH_PARA` 267＋`TECH_TWOPARA` 52＝319 筆 Set/Go 對照） | `uteach.cpp` |
| `cinitial.cpp SetSimuScreenPara()`（非讀檔，程式常數抽出） | `Sim-scale.json` | ✅ 已轉換 | `motors[]`：`screen`/`machine`/`target`/`vertical`（機構座標↔畫面像素換算）；`ledGroups[]` | Motion View 換算用 |
| `system\MotorTest.ini` | `Production-update.json.state.context.simulatorStartup.motorTest` | ✅ Simulator startup 投影 | 以 CP950/Big5 解析為 section/key `{value,type,raw}` JSON；HTML 僅讀 Simulator 發布快照 | `uMotorTest.cpp` |
| `system\tech.dat`（`TECH` 結構，二進位） | `Production-update.json.state.context.simulatorStartup.legacyTech` | ✅ Simulator legacy 投影 | raw little-endian `int32` 陣列、實際長度、預期 3,808 bytes 與 `legacyPartial`；具名欄位仍以 `teach.ini`／`Teach-config.json` 為準 | `uteach.cpp`／`cinitial.cpp` |

附屬（非機台設定值，是**馬達操作指令目錄／請求/回報協定檔**，與硬體馬達強相關，故放在本類）：
`motor-access.json`／`motor-access-ack.json`／`teach-access.json`——詳
[motor-access.md](../../ht9045-html-version/references/motor-access.md)（skill `ht9045-html-version`）。

> 欄位細節：[io-dual-json.md](../../ht9045-html-version/references/io-dual-json.md)、
> [motor-dual-json.md](../../ht9045-html-version/references/motor-dual-json.md)、
> [motion-view.md](../../ht9045-html-version/references/motion-view.md)（皆屬 skill `ht9045-html-version`）。
> `Gerneral.ini` Section/Key 權威定義：skill `ht9045-general-ini`
> （`d:\HT9045\.github\specs\gerneral-ini-schema.md`）。

`handlerTrackMatrix` 是 `HW.HandlerSys.html` 的 JSON 匯出延伸，不是 `Gerneral.ini` 來源鍵：
它將分散的 Loader/Unloader 選配投影成 `tracks[]`，包括 T3 `e3TrayName`／T6 `e6TrayName`
enum 對照、Auto1-6 Y Motor 與 Fix 安裝選擇。既有 DFM 控制項仍由 `sections`／`uiMap`
權威化；沒有 BCB6 對應 Key 的 Fix1/2/4/5/6 和 Auto4/5/6 Y Motor 必須標示
`runtimeSupported:false`，僅能供 HTML JSON 傳送與後續 C++ 導入，不能直接改變機台行為。

### 2. Config 檔 — 機台功能開關，來源 `D:\HT9045\config\config.ini`

這類描述「這台機器的軟體功能是否啟用」（QA Mode／Auto Clean／SECS GEM／各種客製選項），
與硬體無關，是純軟體行為開關。

| BCB6 來源檔 | JSON | 狀態 | 主要資料項 |
|---|---|---|---|
| `config\config.ini` | `Config.json` | ✅ 已轉換 | `sections`（70 節/1184 鍵）＋`uiMap`（`cConfiguration.cpp` `elConfig->Add()` 988 元件對照） |
| BCB6 `cmydef.h`／`Config.h`／`cprod.h`（靜態宣告） | `Define-index.json` | ✅ 已轉換 | `#define`／extern／struct/class 欄位索引與 JSON 化分類；不含執行期值 |
| `system\levelset.dat`＋`cSecurity.cpp` | `Security-access.json`／`Security-access-update.json` | ✅ 已轉換 | `AccessLevel[256]`、179 個權限項目、C++ 可見性規則來源；HTML 僅 Debug 寫 request，C++ 驗證後回寫權威資料；實際可見性由 `Security-visibility-runtime.json` 的 C++ 快照決定 |
| （衍生，非讀檔） | `View-rules.json` | ✅ 已轉換 | `windows{}`／`blocks{}`：依 `General-config.json`＋`Config.json` 的值判斷各視窗/頁內區塊 `hide`／`disable`／`dim` |

> 欄位細節：[settings-json.md](../../ht9045-html-version/references/settings-json.md)（skill `ht9045-html-version`）。
> `config.ini`／`IniConfig` 全域結構權威定義：skill `ht9045-config`。
> BCB6 class／結構要整合到既有 JSON 的資料所有權與投影規則：
> [class-json-projection.md](class-json-projection.md)。

### 3. Setup 檔（又稱 Recipe）— 來源 `D:\HT9045\IniData\Data\<RecipeName>\` ＋ `D:\HT9045\SetUp.inf`

目前已建立工作檔索引與目前 Recipe 聚合 JSON。`Main.html` 的 `edSetupFileName`、工作溫度、Soak Time 與
Run Mode 由 JSON 更新，不再使用寫死的示範值；頁面直接開啟時也必須在 `HTSettings.load()` 完成後套用，
不能只依賴 background 的 `HT_SETTINGS` 廣播。SitePanel 的 BCB6 對照、table 列欄標題、離線快照與
runtime 欄位邊界見 [main-sitepanel-json.md](../../ht9045-html-version/references/main-sitepanel-json.md)。

| BCB6 來源 | 元件／函式 | 行為 | 對應 JSON（已建立） |
|---|---|---|---|
| `D:\HT9045\setup.inf`（純文字，**第一行＝目前工作檔資料夾名稱**） | `GetLastOpenFN()`（`common.cpp`）→ `edSetupFileName`、`cbSetupFileName->Text` | 開站與存檔後皆會讀寫；Simulator 也會在 bridge start 讀取 | ✅ `Setup-index.json.current`＋`Production-update.json.state.context.simulatorStartup.setup` |
| `D:\HT9045\IniData\Data\` 目錄清單（`FindFirst`/`FindNext` 掃資料夾名，`main.cpp` ~9100~9134） | `cbSetupFileName`（`TComboBox.Items`） | 開站時掃描全部子資料夾名稱加入清單 | ✅ `Setup-index.json.list[]` |
| `system\Gerneral.ini [System] SetupFileCheckList` 指向的名單檔 | `CheckSetupNamelist_Hisi()` | 檢查工作檔名稱是否在客戶允許名單；**不是必要檔清單** | ✅ `Setup-index.json.nameCheckList` |
| 各 Recipe 資料夾內 **11 個 `.Data` 檔**：`TestMode.Data`／`HotPlate.Data`／`HandlerCondition.Data`／`Contact.Data`／`Temperature.Data`／`Binasgn.Data`（7 變體）／`ArmCondition.Data`／`Tray.Data`／`Tester.Data`／`Rotate.Data`／`UdUld.Data` | 各功能頁（`cSetUp`/`cContact`/`uTemp_Set`/`cBinSel`/…） | `ReadTestMode()`／`ReadContactFile()`／`ReadTrayFile()`／`ReadArmCondition()`／… | ✅ `Setup-current.json`（聚合；Binasgn 依 `variants` 分組） |

**`Setup-index.json` 已落地格式**（完整 schema 見 `setup-recipe-json.md`）：

```json
{ "schemaVersion": "1.0.0",
  "source": { "toolchain": "BCB6",
    "files": ["D:\\HT9045\\SetUp.inf", "D:\\HT9045\\IniData\\Data\\"] },
  "current": "HT9046LS-HIDRA-8-FT2_85C",
  "list": ["HT9046LS-HIDRA-8-FT2_85C", "..."],
  "nameCheckList": {"configuredFile": "...dat", "entries": [], "currentAllowed": null},
  "integrity": {"requiredFiles": ["TestMode.Data", "HotPlate.Data", "..."], "missingFiles": [], "complete": true},
  "currentData": "Setup-current.json"
}
```

> Recipe 11 個 `.Data` 檔完整格式、7 個 `Binasgn` 變體、`ChangeRecipe()`/`DataPath` 切換流程
> 權威定義：skill `ht9045-recipe`。本 skill 只描述「HTML 開站要用哪份 JSON 顯示/切換目前工作檔」，
> 不重複 `.Data` 檔案本身的欄位定義。

### 4. 生產記錄檔（Production Records）— 機台運轉期間產生／即時狀態

| JSON | 狀態 | 主要資料項 | 更新頻率 |
|---|---|---|---|
| `IO-runtime.json` | ✅ 已轉換 | `runtime{connected,lastPollAt,pollIntervalMs,seq}`；`points[]`：`ioId`/`isOn`/`isOff`/`state`/`quality` | 高頻（輪詢） |
| `Motor-runtime.json` | ✅ 已轉換 | `motors[]`：`position`(cmdPos/encPos/targetPos)／`motion`(speed/busy/…)／`state`(servoOn/alarm/…)／`diag` | 高頻（輪詢） |
| `Task-runtime.json` | ⛔ 骨架（C++ 端未輸出真實資料） | `mainProcMonitor`（Alive/CallCount/LastEnter/SilentSec/SaveTime，對應 `Task_ListWithTime.csv`）、`tasks[]` | 中頻 |
| `System-runtime.json` | ⛔ 骨架 | `version`／`lot`／`state`（systemStart/pause/alarm）／`flags`（DumpMainFormSnapshot）／`decisionVariables`／`counters` | 中頻 |
| `Production-runtime.json` | ✅ schema 1.3.0 | 基礎資料；只有 `machineRecord`（normal/spare）固定為開站一次，其餘八區塊可被事件快照覆蓋 | 開站載入＋事件合併 |
| `Production-update.json` | ✅ C++ 交付骨架 | `event` envelope＋八個 mutable sections 的完整快照；明確不含 `machineRecord` | 每 250 ms 檢查 `seq`，內容由事件更新 |
| `Production-update.json.state.motionView` | ✅ HTML consumer / C++ producer contract | Tray／LED 逐格快照；`trayLocations` 每位置發布獨立的 `{motorHasTray, sensorOn}`：Loader、Auto1~6、Fix1~6 使用已確認的 `MOT[].fHasTray` + `Sen[].IsOn()`，Empty/Color Sensor 對照未確認時固定 `null`；`outArm.phase` + `targetTrack` 驅動 Motion View 退料 tab；`autoClean.active` 表示 fCleaning 執行中 | 事件快照；缺值一律顯示未知，禁止前端推導 |
| `MotionView-i18n.json` | ✅ Motion View 多國語系字典 | `en`／`zh`／`ja`／`ko` 的操作員標題與 Tray 位置狀態；`Main.MotionView.html` 以 `HTSettings.loadJson()` 載入，並接收主畫面的 `HT_LANG` 廣播 | 開站時載入；字典新增或修改後必重跑 `_gen_json_shim.py` |
| `Production-update-ack.json` | ✅ 可選回報骨架 | HTML 套用結果：`seq/eventId/state/appliedAt/message/errors` | debug writer 已授權時回報 |
| `Production-sync-contract.json`／`production-sync/*.json` | ✅ 二層傳輸規格與情境範例 | `full-snapshot` checkpoint、`test-complete-delta`（affected Site＋Arm counters）、resync request/ack | 開站、Lot/Recipe/SiteMap 邊界、每 100 delta 或 5 分鐘 checkpoint；測試完成只傳受影響 Site |
| `Main-command-ack.json` | ✅ Simulator/C++ command acknowledgement | 啟用 `simulator=on` 的 Main 以 WebSocket 將操作意圖交給 Simulator；Simulator/C++ 驗證後寫 ack 並發布 runtime。HTML 不建立 `Main-command-request.json`。 | 每次 Simulator Main 操作 |
| `Debug-json-log-config.json` / `Debug-json-log-flush-request.json` / `Debug-json-log-flush-ack.json` | ✅ Debug timeline archive | `IDE.JsonCommandLog.html` 以 localStorage 累積 Main/Site request 與 Main ack；按「儲存並清空」才送 flush，C++ 寫入固定 daily 目錄並以同一 `flushId` ack 後允許清空 | checkbox／使用者存檔 |
| `Site-toggle-request.json` | ✅ HTML→C++ request mailbox | `Main.html` SitePanel mouse-up 的 target（row/col/position/siteNumber）、mouse button/shift 與 Recipe/seq 基準；HTML 不直接改 Site 狀態 | 每次 mapped Site mouse-up |
| `Runtime-bridge-contract.json` | ✅ 工程交付規格 | 檔案責任、原子寫檔步驟、八種事件、必填欄位、clear scope、各事件更新區塊 | schema 變更時更新 |
| `Dialog-bridge-contract.json` | ✅ 工程交付規格 | `ShowErrorMessage`／`ShowMyMessage` 的 request→modal→close response、K code 與回傳關聯 | schema 變更時更新 |
| `Alarm-dialog-request/response.json` | ✅ C++ 交付骨架 | Alarm 顯示內容、KCode bitmask、精確整數 action 回傳與關閉原因 | 每次 Alarm 觸發／關閉 |
| `Message-dialog-request/response.json` | ✅ C++ 交付骨架 | Message 文字、OK/Pause、Servo/SECS 狀態與關閉後副作用確認 | 每次 Message 觸發／關閉 |
| `Dialog-close-request/response.json` | ✅ C++↔HTML 關閉握手 | C++ 偵測實體 IO 後指定 target/action 主動關閉；HTML 關閉完成後回報。HTML 不判讀 IO | 每次外部或畫面關閉 || `Dialog-auth-verify/result.json` | ✅ 權限密碼握手 | Alert 需密碼時 HTML 寫 verify（憑證＋待執行 action），C++ 比對後寫 result；通過才寫 normal response。HTML 不驗證密碼 | 每次需密碼的關閉動作 || `state-record.json` / `state-record-ack.json` | ✅ 協定已生效 | 程式快照（`DoStateRecord`）request／ack | 事件觸發（非輪詢） |

> 欄位細節：[motor-dual-json.md](../../ht9045-html-version/references/motor-dual-json.md)、
> [io-dual-json.md](../../ht9045-html-version/references/io-dual-json.md)、
> [state-record.md](../../ht9045-html-version/references/state-record.md)（皆屬 skill `ht9045-html-version`）。
> `Task-runtime.json` 分析方法對應：skill `ht9045-state-record-analysis`（`Task_ListWithTime.csv`／`MainProcMonitor`）。
> `Production-runtime.json` schema、TArm/TLotSummary 尺寸與一致性規則：
> [production-runtime.md](../../ht9045-html-version/references/production-runtime.md)。
> IO、Motor、Tray、Sucker 與 LastSet 的 class 投影邊界：
> [class-json-projection.md](class-json-projection.md)。

---

## JSON-only 原則（強制，不可違反）

⛔ **這一整節是 A 路（靜態 JSON）的規則，20260916 起只適用於 `D:\HT9045\page\` 那棵舊樹。**
交付包（`HT9045\web\page\`）走 B 路：資料**不經過 JSON 檔**，直接打
`wb_serve` 的 `GET /api/recipe/*`、`GET /api/system/*`、`GET /api/text/*` 與 WS tag 串流。
所以「一律只用 JSON」「不可 fallback 去讀原始檔」「需工程師轉換 XXX → JSON」這三條
**在 B 路上已不成立**——B 路就是由 C++ 直接讀那些 `.ini`／`.csv`／`.dat`／`.Data`，
然後以 JSON **形狀**（不是 JSON **檔案**）回給瀏覽器。
本節保留為 A 路的歷史規格與 file:// 墊片機制說明。


- HTML 端資料來源**一律只用 JSON**（`D:\HT9045\JSON\`；離線快照在 `JSON\offline\`）。
- BCB6 程式讀取的非 JSON 檔（`.ini`／`.dat`／`.csv`／`.Data`）**不得由 HTML 直接解析**，
  必須先請工程師轉換成 JSON 才能用。
- 尚未轉換前：畫面只做 UI、資料留空並標示「需工程師轉換 XXX → JSON」；**不可** fallback 去讀原始檔。
- file:// 下 Edge/Chromium 封鎖 XHR/fetch → 每個 JSON 都有 `JSON\js\<name>.js` 墊片
  （`_gen_json_shim.py` 自動生成，內容為 `window.__HT9045_DATA__["<name>"]={...}`，勿手改；
  改完 JSON 必跑一次 `_gen_json_shim.py`）。
- 原始碼一律標注 **BCB6**（Borland C++ Builder 6，
  `D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2`，cp950/Big5）；
  後續 VC++ 版需另行標注，兩者不可混寫在同一段落。JSON `source` 區塊需帶 `"toolchain":"BCB6"`。
- **編碼**：JSON 檔本身、`JSON\js\*.js` 墊片一律 **UTF-8**（無 BOM），與 BCB6 原始碼的
  Big5/CP950 是不同編碼領域；產生器讀 BCB6 來源用 `encoding='cp950'`，寫出 JSON 用
  `encoding='utf-8'`，同一支腳本內方向不可搞混——詳見 AGENTS.md 與
  `ht9045-html-json-encoding.instructions.md`。
- **Simulator IPC**：`release.html` 與 `debug.html` 先將 `{ mode, simulator: true }` 寫入同分頁
  `sessionStorage`，再以無參數的 `background.html` 開站，故網址列不顯示 JS 操作旗標。background
  僅在內部 iframe URL 傳遞 `mode=<release|debug>&simulator=on`；`simulator=on` 表示已啟用 VC#
  Simulator 的 HTML 上線資料與命令通道，並非連線至實機 Handler。兩種模式的 `Main.html` 都以
  `page/simulator-bridge.js` 訂閱 `ws://127.0.0.1:9045/ht9045-json/`。HTML 開站會收到
  `HT_SIMULATOR_READY` 與啟動摘要，再讀取 Simulator 原子寫出的 runtime JSON；HTML 不得寫入任何 JSON
  或要求資料夾權限。

## 更新觸發與頻率分類（⚠ 這是 **A 路（靜態 JSON）** 的規格）

⚠ 下面這張表定義的是**已廢棄的 A 路**怎麼決定「什麼時候重讀哪一份 JSON」。
`wb_serve` 一支都不產生這些 JSON，所以這張表**不能直接套到 B 路**。
留著是因為「哪些資料該一次讀、哪些該事件更新、哪些該輪詢」這個分類本身仍然對，
B 路的對應在 SKILL.md 本文「同五個分類在 B 路（wb_serve）的對應」一節。

| 分類 | 觸發 | JSON／資料 | 頻率規則 |
|---|---|---|---|
| `startup-once` | HTML/機台開站 | General/Config/View-rules 與 `machineRecord` | `machineRecord` 只讀一次；其他 Production 區塊不是 startup-only |
| `operator-event` | 操作員切換/儲存 Setup、Lot Start/End 或修改 Lot Info | Setup-index/current、Production `context`/`lotInfo` | 每次操作完成後更新，不輪詢 |
| `production-event` | `SetTesterBin`／`ProcessShowTestStatus`／`CheckContinuoussFail`／`ClearALLCT`／Test Complete | Arms/LotSummary/TEST_CATEGORY 與六個 production streams | 狀態邊界發 `full-snapshot`；Test Complete 發 affected Site delta；`seq` 單調遞增，未生產時 stream `available:false` |
| `forced-poll` | HTML runtime bridge 強制讀硬體狀態 | IO-runtime、Motor-runtime | 依各檔 `pollIntervalMs`；不得以開站快照冒充即時值 |
| `controller-poll` | 溫控器通訊完成 | Production `productionStreams.temperature` | 由 bridge 填 `configuredIntervalMs`；建議 250 ms，斷線即 stale/unavailable |

A 路規定每個 runtime stream 必須帶 `available`、`updateClass`、`trigger`、`updatedAt`、`seq`；
沒有真實資料時使用空陣列/空物件，禁止填 DFM demo 值。

## Simulator 開站讀取與 HTML 查詢

⛔ **A 路（已廢棄），不作為資料來源。** C# JSON Simulator 寫的是模擬值，使用者 20260916
裁決全面退場。本節保留，是因為它記著「開站要讀哪些實體來源、摘要成什麼形狀」——
那份清單本身仍有參考價值（`wb_serve` 的 `/api/system/` 表列就是同一批檔）。
**但不要照它開 Simulator，也不要拿它的 JSON 當資料。**


`HT9045 JSON Simulator` 啟動 bridge 時是 Debug 模式的唯一磁碟 I/O owner，必須讀取下列來源並
將摘要放在 `Production-update.json.state.context.simulatorStartup`：

| 實體來源 | Simulator 摘要 | 用途 |
|---|---|---|
| `D:\HT9045\setup.inf` | `setup.currentRecipe`／`availableRecipes` | 決定 Main 的目前 Setup File；以第一行為準 |
| `D:\HT9045\IniData\Data\` | `setup.availableRecipes` | Setup 工作檔目錄數 |
| `D:\HT9045\system\IO_Table.csv` | `ioTable.path`／`columns`／`rows` | 驗證 IO 表可讀與資料列數 |
| `D:\HT9045\system\Mot_Table.csv` | `motorTable.path`／`columns`／`rows` | 驗證馬達表可讀與資料列數 |
| `D:\HT9045\system\MotorTest.ini` | `motorTest` | CP950 INI section/key `{value,type,raw}` 投影 |
| `D:\HT9045\system\tech.dat` | `legacyTech` | raw int32 little-endian 投影；必帶 `fileBytes`／`expectedBytes`／`legacyPartial`，不可臆測短檔尾端欄位 |

HTML 的 `simulator-bridge.js` 訂閱時會送出 `HT_SIMULATOR_SUBSCRIBE`，Simulator 回覆
`HT_SIMULATOR_READY`（包含 `startup`），每次發布再送 `HT_SIMULATOR_PUBLISHED`。HTML 收到任一訊息
後以 `HTSettings.refreshProduction()` 載入 C# 已寫好的 snapshot；不得直接讀取上述非 JSON 原始檔。
訂閱連線開啟後，HTML 每秒傳送 `HT_SIMULATOR_HEARTBEAT {clientTime}`；Simulator 僅記錄並以
`HT_SIMULATOR_HEARTBEAT_ACK {clientTime,simulatorTime}` 回應，不應為 heartbeat 重寫大型 Production snapshot。

## C++ 工程師交付檔

⛔ **A 路（已廢棄）的交付規格。** 這裡講的 `Runtime-bridge-contract.json`／
`Production-sync-contract.json`／`full-snapshot`／`test-complete-delta`／原子寫檔，
全部是靜態 JSON 那條路的東西，`wb_serve` 一個都不產生。
保留是因為 **兩層傳輸（checkpoint ＋ affected-Site delta）的設計仍然是對的**，
B 路的 tag 串流目前沒有這一層（整份 snapshot 重送），日後要補時這節是現成的起點。


1. 先讀 `Runtime-bridge-contract.json` 與 `Production-sync-contract.json`；既有 eight-section snapshot contract 仍適用於 full snapshot。
2. 開站/重連、Lot/Recipe/SiteMap 邊界、清除與 HTML resync 時寫 `full-snapshot`；至少每 100 個 delta 或 5 分鐘再寫一份 checkpoint。
3. `ProcessCount(int Index, bool bHasIC)` 完成所有 count、history、continuous-fail、test-category 與 yield 處理後，寫 `test-complete-delta`；只含 affected Site、Arm `pass/fail/total/ifError/binCounts` 與必要 stream change。
4. delta 禁止傳 `sum`、`passYield` 及 `current/history/byLot/autoClean` 重複 view；HTML `HTSettings` 以 Arm counters 動態建立 `arms.canonical[].display`。若 delta `baseSeq` 不等於 HTML 的 last sequence，HTML 拒收並發出 `HT_PRODUCTION_RESYNC_REQUIRED`，C++ 應回傳新的 full snapshot。
5. `seq` 單調遞增、`eventId` 唯一、`occurredAt` 使用含時區 ISO-8601；先寫 `.tmp`、flush/close，再原子 replace。file:// 模式需同步原子更新 `JSON/js/Production-update.js`。
6. 六個 stream：`socketCounters`、`testStatus`、`continuousFail`、`yieldHistory`、`testTiming`、`temperature`。

Modal dialog 不併入 Production update。C++ 依 `Dialog-bridge-contract.json` 寫入兩組獨立 mailbox；
HTML 以 `requestId + requestSeq` 回應。Alarm 的 `selectedAction.code` 必須原值回傳為 `fNote->ReturnCode`。
C++ 偵測 `SnFKPause`／`SnFKStart`／`SnRKPause`／`SnRKStart` 等實體 IO 後，另寫
`Dialog-close-request.json`；HTML 只比對 target、關閉畫面並寫 `Dialog-close-response.json`，不得讀取或推論 IO 狀態。对话框畫面本體為 dfm 生成的 `page/Alert.Note.html`（fNote）與 `page/Alert.MyMessageBox.html`（MyMessageBox），由
`dialog-bridge.js` 以 overlay iframe 開啟、`dialog-page.js` 依 request 填值；Alarm response 含 `pressedButton`（BtnStart/BtnPause）。

## 開站載入順序（background.html）

⛔ **A 路（已廢棄）。** `background.html` ＋ `JSON/js/*.js` 墊片是 file:// ＋ Simulator
那條路的開站流程。B 路沒有「預載一堆 JSON 再開站」這件事：頁面各自 `GET` 自己要的
`/api/recipe/<doc>`／`/api/system/<name>`，執行期資料走 WS tag 串流。
保留是為了讀懂 `D:\HT9045\page\` 那棵舊樹；交付包 `D:\HT9045\web\page\`
走的不是這條。


JSON 之間有相依關係——**類別間必須依序載入**，後面的類別可能需要前面類別的值才能正確判斷：

| 順序 | 類別 | JSON | 為什麼要這個順序 |
|---|---|---|---|
| 1 | 硬體設定檔（機台基本資料） | `General-config.json` | **最優先**：決定機型/選配/驅動卡類型，其餘規則與畫面是否顯示都要先看它 |
| 2 | 硬體設定檔（個別頁面用，可延後） | `IO-config.json`／`Motor-config.json`／`Teach-config.json`／`Sim-scale.json` | 各自頁面用到才載入；不影響開站時哪些視窗要顯示 |
| 3 | Config 檔 | `Config.json` | 功能開關，需排在 `General-config.json` 之後（部分規則要同時看兩者） |
| 4 | Config 檔（衍生規則） | `View-rules.json` | 必須排在 1、3 之後：規則同時讀 `General-config`與`Config`的值才能算出 |
| 5 | Setup 檔（Recipe） | `Setup-index.json`／`Setup-current.json` | 排在規則之後：目前工作檔名稱與內容供 Main/Setup 頁顯示，不影響視窗 hide/disable |
| 6 | 生產記錄檔 | `Production-runtime.json`／`Production-update.json`／`Task-runtime.json`／`System-runtime.json`／`IO-runtime.json`／`Motor-runtime.json` | 最後；先載入 baseline，再依事件或輪詢更新，未就緒不可阻塞開站 |

### 目前 `background.html` 的實際載入（2026-09-02）

```html
<script src="JSON/js/General-config.js"></script>
<script src="JSON/js/Config.js"></script>
<script src="JSON/js/View-rules.js"></script>
<script src="JSON/js/Setup-index.js"></script>
<script src="JSON/js/Setup-current.js"></script>
<script src="JSON/js/Production-runtime.js"></script>
<script src="JSON/js/Production-update.js"></script>
<script src="page/settings.js"></script>      <!-- HTSettings.loadSync() → decideAll() -->
<script src="page/json-writer.js"></script>   <!-- 僅其他 legacy Debug flow；Main 不得使用 -->
```

已完成順序 1、3、4、5、6 的 Production baseline 與 update（7 個 JSON `<script>` 同步預載）；
background 每 250 ms 檢查 update `seq`，只覆蓋八個 mutable sections，保留第一次載入的 `machineRecord`。
`IO-config`／`Motor-config`／`Teach-config`／`Sim-scale`／各 `runtime` JSON 目前由**各自頁面**
自行 `loadJson()`／`loadDual()`，不在 background 開站流程內；Setup 的兩份 JSON 則由 background 預載。

若日後要在開站畫面（例如 `Main.html` 狀態列顯示目前工作檔名稱、機台序號等）用到更多分類，
需把對應 `<script src="JSON/js/...">` 依上表順序加進 `background.html`，並在
`HTSettings`（`page/settings.js`）內新增對應的 `get()` 路徑。

## A 路（靜態 JSON）的舊待辦——已隨裁決失效，保留只為追溯

⛔ 下面這些是 A 路的工作項。A 路已廢棄，**不要再去做**，也不要當成缺口報告。

- [x] `Setup-index.json`／`Setup-current.json`（Recipe 11 個 `.Data`，Binasgn 變體分組）
- [x] `Runtime-bridge-contract.json`、`Production-update.json`、`Production-update-ack.json`
- [x] HTML `HTSettings` 支援 `full-snapshot`／`test-complete-delta`
- [x] JSON Simulator 的 `MotorTest.ini`／`tech.dat` → `simulatorStartup` 投影
- [x] 其他讀寫檔（表③）30 餘項全數處理
- [~] C++ 端依 contract 原子發佈 Production update 與 shim —— **A 路，不做**
- [~] `Task-runtime.json`／`System-runtime.json` 由 C++ 輸出真實資料 —— **A 路，不做**；
      B 路對應的是 tag 串流，而 `task.*`／`system.*` 這兩個 prefix 目前根本不存在
- [~] `LastSet-projection-contract.json` 逐欄位補齊 —— **A 路，不做**；
      B 路已有 `lastset.*` 4 個 tag ＋ 33 個 LastSet blob tag
- [~] `ARMS.ini`／`ColorSensorType.ini`／`MVData.ini`／`RPDefault.ini`／SECS `Gerneral.ini`／
      `NSKit.txt` 待實機重跑 —— **A 路**；B 路這 5 支已在 `/api/system/` 表列中，
      上機台後檔案在即可讀，不需要重跑產生器

## 產生器（A 路，`ht9045-html-version` 工作副本）

⛔ **這整張表都是 A 路的產生器。** 它們產出的 JSON 已不作為資料來源。
B 路的產生器是另外兩支：`HT9011UC_Cpp_V3.33.906.0\scratchpad\gen_wire.py`
（從 golden 機械產生每頁的 `ht9045_wire_<slug>.js` 接線資料）與
`scratchpad\gen_page_status.py`（量出表⑤／表⑥ 的接線分級）。


| 腳本 | 產出 | 對應類別 |
|---|---|---|
| `_gen_ini_json.py` | `General-config.json`、`Config.json` | 1、2 |
| `_gen_define_index.py` | `Define-index.json`（標頭宣告索引） | 2（靜態參考） |
| `_gen_security_access_json.py` | `Security-access.json`（LevelSet 權限與項目索引） | 2（存取控制） |
| `_gen_setup_json.py` | `Setup-index.json`、`Setup-current.json` | 3 |
| `_gen_production_runtime.py` | `Production-runtime.json`、`Production-update.json`、`Production-update-ack.json`、`Runtime-bridge-contract.json` | 4 |
| `_gen_view_rules.py` | `View-rules.json` | 2（衍生） |
| `_gen_json_shim.py` | 全部 `JSON\js\*.js` 墊片（**任何 JSON 改動後必跑**） | 全部 |
| `_gen_motor_access.py` | `motor-access.json`／`motor-access-ack.json`／`teach-access.json` | 1（附屬） |
| `_gen_simscale.py` | `Sim-scale.json` | 1 |
| `_gen_state_record.py` | `state-record*.json`、`Task-runtime.json`、`System-runtime.json` 骨架 | 4 |
| `_gen_dialog_bridge.py` | `Dialog-bridge-contract.json`、Alarm/Message request/response 骨架 | 4（雙向 modal） |
| `_gen_misc_config_json.py` | 其他讀寫檔 28 個 JSON（`ARMS-config`／`ContactInfo-config`／`AutoTemperature-config`／`ATC-config`／`Barcode-config`／`PadInterfacePara-config`／`EventLogLevel-config`／`TrayStepSpeed-config`／`SpecialErrNote-config`／`DioCfg-index`／`Security-def`／`CriticalParaControl-config`／`ESDconfig-config`／`RPDefault-config`／`SocketCount-config`／`Offset-index`／`SaveByMachine-index`／`DeviceCorrespond-config`／`AlarmCodeList-index`／`AlarmDescription-config`／`AlarmDescriptionOverride-index`／`SecsGem-config`／`PMAlarm-config`／`ProductionInfo-config`／`NSKit-flag`／`SitMap-config`／`LastSet-projection-contract`／`TrayForm-config`／`PlateForm-config`） | 5（其他讀寫檔） |
| `_merge_language_csv_i18n.py` | `Language.csv` → `page/i18n.js`（`HTI18N.dfm`，不建 JSON） | 5（i18n 併入） |
| `_embed_release_note.py` | `ReleaseNote.txt` → `Data.Observer.html #Memo1`（靜態內嵌，不建 JSON） | 5（HTML 內嵌） |

工作副本在 `D:\AI_TempFile\`；保存版在 `d:\HT9045\.github\skills\ht9045-html-version\scripts\`。
