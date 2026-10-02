# B 路（wb_serve）— 現行資料路線的詳細規格

**這是什麼**：`ht9045-html-json` skill 的「B 路」——現行方向，資料一律由 `wb_serve.exe`
直接讀機台實體檔案（`.ini`／`.csv`／`.dat`／`.Data`），以 HTTP `GET`／WebSocket 回給
`D:\HT9045\web\page\` 交付包，不經過任何靜態 JSON 檔。對照的「A 路」（已棄用）見
[route-a-legacy.md](route-a-legacy.md)。

**為什麼搬到這裡**：Steven 20260924——B 路是現行方向，內容天天在用，但端點對照表、
tag 實況數字、`--dry`／`--allow-system-write` 等旗標細節、待辦清單這些「操作級」細節
占了 SKILL.md 大半版面。搬過來只是為了不占 SKILL.md 版面，SKILL.md 留 2–3 行導覽
＋指標；細節（含哪個端點、哪個 tag、目前卡在哪）都在這裡查。

**什麼時候還用得到**：
- 要接一個新頁面到 `wb_serve`，需要查某類資料對應哪個端點。
- 要除錯「畫面上這個值是不是機台在用的值」，需要查檔案鏡像 vs C++ 結構層 vs tag 串流的現況。
- 要接手「待辦」裡任何一項（tag 協定缺 metadata、`guard.*` 訊號未交付等）。

以下內容為原 SKILL.md 對應章節之逐字搬移，未作內容修改。

---

> `GET /api/form/<Page>`／`WS form.save`／`WS editlist.get`／`editlist.save`／
> `GET /api/editlist/<list>` 這條「golden 表單橋」路線 20260924 已獨立正名為 **C 路**，
> 完整規格搬到 [route-c-golden-bridge.md](route-c-golden-bridge.md)。

> ⛔ **狀態更新（Steven 團隊 20260926，HEAD 8fad1522；下文原句不改寫，照本檔「逐字搬移」的性質只在這裡更正）**：
> 1. 下面「⛔ 20260924：`web/page/` 零個頁面呼叫 `/api/struct`」與「S12 第一波（同日，未 commit）已接 4 頁」
>    **都已過期**：(a) `/api/struct` 的**結構綁定**（`deviceForm`／`testIF`…）仍沒有頁面用，但 IO／馬達頁用
>    `/api/struct/io/*`、`/api/struct/motor/*`（`web/page/HW.IoSetView.html:433`、`HW.MotorTest.html:227`、
>    `HW.teach.html:386`、`IoLive.html:14`）；(b) 「畫面上的值來自 C++」現在走 **C 路**：24 頁在
>    `ht9045_wire_engine.js:1038-1061` 的 `GOLDEN_BRIDGE`、A 形狀只剩 `Setup.HotPlate.html`；S12 第一型
>    `/api/form`（`formOverlay()`）的 4 頁全被 C 路／A 形狀蓋過，沒有頁面用得到（`route-c-golden-bridge.md` §3.3）。
>    逐結構總表見 `route-c-golden-bridge.md` §6。
> 2. 「`io.*`／`motor.*`／`task.*`／`system.*` 四個 prefix 在 tag 表裡根本不存在」**只剩 `task.*`／`system.*` 成立**；
>    `io.di`／`io.do`／`io.ver`／`motor.axes`／`motor.count`／`motor.ver` 自 json-bridge S9 起就有（`JsonBridge/ChanIo.cpp`／
>    `ChanMotor.cpp`；本檔下面「結構層」小表那一列本來就寫對了）。
> 3. C 路接手的檔（`CRouteOwner`，`tools/wb_serve.cpp:1289-1318`）在 B 路的 `system.file.put`／`recipe.doc.put`
>    一律 409（`dryRun` 放行）；清單見 `route-c-golden-bridge.md` §6。

## 兩條路對照與 wb_serve 端點現況

| 路 | 誰在餵 | 現況 |
|---|---|---|
| **A. 靜態 JSON**（`D:\HT9045\JSON\` ＋ `JSON\js\*.js` 墊片） | PowerShell／`_gen_*.py` 一次性產生；C# JSON Simulator 寫 runtime 那幾支 | ⛔ **已廢棄，不作為資料來源**。格式（欄位形狀、`*-ack` 協定）仍可參考 |
| **B. `wb_serve` HTTP／WebSocket** | `wb_serve.exe` 直接讀機台實體檔，外加執行期 tag 串流 | ✅ **唯一真值來源** |

`wb_serve` **一個 A 路 JSON 檔都不產生**。20260916 實測：對
`HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`（2,461 行）grep
`General-config|Config\.json|View-rules|Setup-index|Setup-current|Production-|IO-runtime|Motor-runtime|Task-runtime|System-runtime|Security-access|Sim-scale|Teach-config`
命中 **0**。兩套不共用任何檔案，要分開讀。

### 四大類在 B 路（wb_serve）的實際狀態與端點

| 類別 | A 路檔名（下面表格的 ✅ 指這個） | B 路端點 | B 路可用性（20260916 實測） |
|---|---|---|---|
| **1. 硬體設定檔** | `General-config` `IO-config` `Motor-config` `Teach-config` `Sim-scale` | `GET /api/system/{gerneral,ioTable,motTable,teach}`；`WS system.file.put`／`system.csv.rows` | ✅ 讀寫皆通，HTML 側四頁已接（20260916）。`Sim-scale` 是畫面像素換算、不是機台檔，**B 路沒有也不需要端點** |
| **2. Config 檔** | `Config.json` `Define-index` `Security-access` `View-rules` | `GET /api/system/config`；權限改走 `GET /api/system/levelset` ＋ `WS system.levels.put` | ✅ config 讀寫通；權限端點 20260916 才補上。`View-rules`／`Define-index` 是衍生規則與靜態宣告索引，**B 路沒有端點**，畫面規則仍走 `HTSettings.applyBlocks` |
| **3. Setup 檔（Recipe）** | `Setup-index` `Setup-current` | `GET /api/recipe/`（16 文件）＋`WS recipe.doc.put`；目前配方名另由 tag `recipe.current` 提供 | ✅ 讀寫皆通 |
| **4. 生產記錄檔** | `IO-runtime` `Motor-runtime` `Task-runtime` `System-runtime` `Production-*`／各 `*-dialog-*` | **執行期 tag 串流**（WS `/ht9045`，snapshot ＋ 500 ms patch，**20260919 實測 4608 個 tag**，其中 4481 個是 HMI 用不到的 `pci1203.*`）；`GET /api/text/`（6 個唯讀記錄來源） | ⚠ **管道通了、內容幾乎全空**（見下節）。**`io.*`／`motor.*`／`task.*`／`system.*` 四個 prefix 在 tag 表裡根本不存在（各 0 個）**，所以 A 路那四支 runtime JSON 在 B 路**沒有任何對應** |
| **5. 其他讀寫檔** | 28 個 `<Name>-config.json` | `GET /api/system/<name>`（固定 40 支表列） | ✅ 35 支本機有實檔、5 支 `available:false` |

#### ⚠ 上表的端點都是「檔案鏡像」；C++ 結構層另算（Steven 20260924 實測）

上表 1～3 類的端點（`/api/system`、`/api/recipe`）讀寫的是**檔案內容**，不是 C++ `ReadFile()` 之後的結構值。
結構層由 skill `ht9045-json-bridge` 提供，四大類的對應是：

| 類別 | 檔案鏡像（上表） | 結構層（`/api/struct`、tag 通道） |
|---|---|---|
| 1. 硬體設定檔 | ✅ | ❌ 沒有。`MOT[]`／`Tech` 等沒有綁定，只有 `machine.defines`（編譯期設定） |
| 2. Config 檔 | ✅ | ⚠ 只有 `levelSet`（唯讀）；`IniConfig`、`TfDIOFrom`（json-bridge S5）**沒做** |
| 3. Setup 檔（Recipe） | ✅ | ⚠ 5 個唯讀綁定：`deviceForm`／`testIF`／`temperature`／`trayForm`／`testMode`（`.file` 與 `.live`）。HotPlate／ArmCondition／Binasgn／Rotate／UdUld／Offset 沒有綁定。`struct.put` 只到 dryRun |
| 4. 生產記錄檔 | `/api/text/` 唯讀 | ✅ `prod.*`／`alarm.*`／`act.*`／`log.*`／`io.*`／`motor.*`（後兩者在沒編 PCI1203 的 exe 上是 null）。`LastSet` 沒有結構綁定；Task-runtime／System-runtime 沒有對應 |

⛔ **20260924：`web/page/` 零個頁面呼叫 `/api/struct`**，畫面顯示的一律是檔案。使用者裁決改由 C++ 執行
`DoIniDataToForm()` 出 JSON 給頁面（json-bridge `decisions.md` 二之二、`phases.md` S12）。
**S12 第一波（同日，未 commit）已接 4 頁**：Setup.HotPlate／Setup.Speed／Setup.TrayAssignment／Config.DIOInterFaceCFG
走 `GET /api/form/<Page>`，引擎 `formOverlay()` 在讀檔後覆蓋；畫面上值來自 C++ 的欄位帶 `data-src="cpp"`。
**其他頁面仍只讀檔案**，不要假設畫面上的配方值就是機台在用的值（例：`Chiller Temp` 檔案被開機改寫成 5，
見 json-bridge `porting-gaps.md` 十三）。

### 執行期 tag 的實況（20260916 實測，不是推估）

```
> build\wb_publish.exe --pump --seconds 2 --port 8149
published 117 tags on the wire; of the 96 MACHINE-sourced ones, 0 carry a loaded value

> build\wb_serve.exe --dry --seconds 5 --port 8152
bin.*  chain loaded: BinSelect -> iT6CatData -> MyBinSel captions
temp.* chain loaded: ReadTempFile -> fWorkTemperBase/fSoakTime/iMachineTempMode
published 117 tags, 9 of 96 carry a loaded value
```

- **117** ＝ 真正在線上的 tag 數（`snap.stagedTagCount()`）。
- **96** ＝ 其中被算成「機台資料來源」的（`HandlerTagCoverage().total`）。
  另外 21 個是 publisher 自己的東西，**刻意不計入分母**：`pump.*` 15、`clock.text`、
  `machine.state`／`machine.stateSource`、`lot.auto1..3.trayCount`。
- **9** ＝ 唯一有值的那幾個，全部來自**配方檔**：
  `bin.auto1..3`／`bin.fix1..3`（6，來源 Binasgn）＋
  `temp.sv`／`temp.soak`／`temp.mode`（3，來源 `Temperature.Data`）。
  **一個都不是從硬體讀來的。**

117 個 tag 的 prefix 分佈：
`site` 32、`pump` 15、`status` 11、`sort` 8、`zone` 8、`bin` 6、`machine` 6、`lot` 5、
`lastset` 4、`temp` 4、`run` 3、`runmode` 3、`tower` 3，以及
`auth`／`clock`／`control`／`fan`／`light`／`recipe`／`startmode`／`tester`／`user` 各 1。

> **教訓**：`wb_serve` 行程裡**讀不到真實 IO**。它從不呼叫 `TLaneIO::SetBackend()`
> （`MyLaneIo.cpp:145` 定義，全樹唯一的呼叫點都在 `tests/`），所以 `MyLaneIO` 一直用建構式
> 給的預設值 `new TSimIOBackend()`（`MyLaneIo.cpp:139`）——一塊全零的記憶體位元陣列
> （`IOBackend.cpp:24-30`）。其他四個 backend（MN200／PCI1203／MNET／RawPort）的函式體都在
> `#if HAVE_xxx` 裡，預設全關。**任何「畫面上的 IO 燈是機台狀態」的假設都是錯的。**

### 為什麼 96 個裡有 87 個是 null —— 不是「producer 缺」，是一個 early return

這是 20260916 查證出來的，**和先前「C++ 端還沒有 producer」的說法相反**：
producer 都寫好了，是整條資料層載入在第一個判斷就折返。

`SYSTEM_MODULAR::ReadGeneralIni()`（V906 `database.cpp:313`）開頭做的第一件事
**不是**讀 `D:\HT9045\system\Gerneral.ini`，而是讀**另一個專案**的檔：

```
database.cpp:316   MyForceDirectories("D:\\GPIB9045\\system");
database.cpp:317   str = "D:\\GPIB9045\\system\\general.ini";
database.cpp:318   MachName = CheckAndReadIniData(str, "Version", "Model", "ModelNG");
database.cpp:319-326  只認 9045GPIB / 9046GPIB / 9046_32GPIB / 9045GPIB_12Site /
                      502GPIB / 1032GPIB / 7080GPIB
database.cpp:332-333  else { bHandlerModel=false; return; }      ← 折返點
```

這台開發機的 `D:\GPIB9045\system\general.ini:2` 是 `Model=9050GPIB`（HT-9050 的 GPIB
橋接設定），**不在那七個之列**，所以 `:333` 直接 return，後面全部沒跑到：

| 沒跑到的 | 行 | 連帶讓誰變 null |
|---|---|---|
| `CUSTOMER_CODE = CheckAndReadIniDataGeneral("System","CUSTOMER_CODE",0)` | `:339` | `machine.customerCode`、`auth.level`、`user.level`、`control.owner`、`recipe.current`、`lot.loaderLastBundleId`、`lot.loadercarBundleId`（liveness 全 key 在 `CustomerCodeLoaded()` ＝ `CUSTOMER_CODE != 0`） |
| `CustomerFunctionSelect()`／`ReadLastSetIni()`／`ReadEventLogAutoSaveInfo()` | `:395-397` | `IniConfig`（→ `machine.id.type/gpib/tester`）、`LastSet` 整塊 blob（→ `lastset.*` 4、`startmode.value`、32 個 `site.*`、`sort.*` 8） |

**這不是移植 bug**：golden V899 `database.cpp:306`／`:321-322` 一模一樣，是忠實翻譯。
上了真機、`D:\GPIB9045\system\general.ini` 的 `Model` 屬於 `9045GPIB` 家族時，這條鏈會整條跑完。

> **教訓**：在這台開發機上把 tag 接到畫面，量出來一定是 `---`，**那不代表接線壞掉**。
> `main.html` 的 `recipe.current → edSetupFileName` 就是活例子：`wb_serve` 開機時明明印出
> `recipe.current = HT9046LS-HIDRA-8-FT2_85C`，但 `WebBridgeTags.cpp:449` 把這個 tag 的
> liveness 綁在 `cust` 上，所以畫面收到的是 null。要在開發機驗 tag，得先確認
> `D:\GPIB9045\system\general.ini` 的 `Model`。

⚠ `database.cpp:316` 的 `MyForceDirectories("D:\\GPIB9045\\system")` 是**硬編路徑**，
`--dry` 重導不到（`--dry` 只保護 `asGeneralPath` 與配方資料夾）。

⚠ **`wb_serve` 的預設 document root 是 `D:\HT9045\web`**（`tools/wb_serve.cpp:1705`），
而這台機器上**沒有這個目錄**——不帶 `--root` 時靜態檔案是服務不到的（API 仍然正常）。
交付包的 HTML 在 `D:\HT9045\web\page\`；`D:\HT9045\page\` 是 file:// ＋
Simulator 那條舊路的樹，44 個檔已與交付包分歧（見 SKILL.md「⛔ 資料來源裁決」）。**改 HTML 前先確認在哪一棵。**

---

## 同五個分類在 B 路（wb_serve）的對應（20260916 實測）

| A 路分類 | B 路對應 | 現況 |
|---|---|---|
| `startup-once` | **HTTP GET 一次**：`GET /api/system/gerneral`、`/api/system/config`、`/api/system/levelset` | ✅ 已有。畫面規則（View-rules 那半邊）**B 路沒有端點**，仍由 `HTSettings.applyBlocks` 在前端算 |
| `operator-event` | **存檔後重讀**：`WS recipe.doc.put`／`system.file.put`／`system.csv.rows`／`system.levels.put` → ack `{changed,identical,notFound,backup}` → 前端重新 `GET` 同一份 | ✅ 已有。ack 帶 `notFound` 是刻意的：寫不中任何鍵要回報，不可默默成功 |
| `production-event` | **tag snapshot ＋ patch**（WS `/ht9045`，500 ms tick） | ⚠ 管道通了，但沒有 `full-snapshot`／`delta`／`baseSeq` 的兩層協定，也沒有 affected-Site 的概念。整份 snapshot 重送，patch 只帶變動的 key |
| `forced-poll`（IO／Motor） | **尚無** | ⛔ tag 表裡沒有 `io.*`／`motor.*`，而且 `wb_serve` 行程用的是全零的 `TSimIOBackend`。**要做的是真實 IO 後端，不是前端輪詢** |
| `controller-poll`（溫控） | **尚無** | ⛔ `temp.sv`／`temp.soak`／`temp.mode` 是從配方 `Temperature.Data` 讀的**設定值**，不是溫控器回來的 PV。`temp.pv` 目前恆為 null |

⚠ **B 路的 tag 協定目前一個 metadata 欄位都沒有**：`available`／`updateClass`／`trigger`／
`updatedAt`／`seq` 五個 A 路必填欄位，tag 訊框（`{"type":"snapshot","gen":N,"data":{...}}`／
`{"type":"patch","gen":N,"data":{...},"removed":[]}`）**一個都沒有提供**。
唯一有的是 `gen`（單調遞增的世代號）。
所以前端**無法區分**「這個值是 null 因為沒接上」與「這個值是 null 因為機台真的沒有」——
現行約定是一律顯示 `---`（不可知），不可顯示 0。要做到 A 路那種「stale/unavailable」語意，
必須先擴 tag 協定，這是還沒設計的工作。

Motion View 補充：HotPlate 啟用位置不是 runtime 推論值，必須讀 `Setup-current.json.documents.hotPlate.sections["Hotplate Form"]["Using Flag"]`（bit `0x01`=Plate1，`0x02`=Plate2）。Auto Clean 的設定讀 `documents.handlerCondition.sections.Configuration`；運轉指示僅讀 `state.motionView.autoClean.active`。OutArm 退料 tab 僅讀 `state.motionView.outArm.phase === "placing"` 與已安裝的 lowercase `targetTrack`，不可依馬達座標或 Tray occupancy 猜測。Tray 位置卡僅讀 `state.motionView.trayLocations` 的 `{motorHasTray, sensorOn}`，並將兩者獨立顯示：`loader.upper/lower/inside` 的 Motor 是 `MOT[MMTrayZ]`／`MOT[MMTrayY_Car]`／`MOT[MMTrayY]` 的 `fHasTray`，Sensor 是 `Sen[SnLoaderTrayHasTray]`／`Sen[SnLoaderCarHasTray]`／`Sen[SnLoaderSureTray]` 的 `IsOn()`；`auto1..auto6` 以 index `0..5` 對應 `iAutoZMot`／`iMMAuto_Car`／`iMMAuto` 與 `SnAutoTrayHasTray`／`SnAutoTrayCar`／`SnAutoTrayDetect`；`fix1..fix6` 的 `inside` 對應 `iMFixTray` 與 `SnFixedTrayDetect`。Empty/Color 三位置的 Sensor 對照尚未確認，必須發布 `sensorOn:null`。任一欄位只能是 `true`、`false` 或 `null`（未安裝／尚不可用）；HTML 不可由另一旗標、馬達位置或 Tray cells 推論。

---

## ⚠ 值域（min／max）：兩邊都要有（使用者裁決 20260916）

> 「手動輸入的時候會需要知道上下限，讀檔的時候也需要做上下限的保護。」

⚠ 這與「變動門檻（deadband）」是兩件事：**值域**是參數的合法範圍（安全問題），
**門檻**是變化多少才值得送一幀（頻寬問題）。不要混談。

現況（實測 20260916）：

| 用途 | 現況 |
|---|---|
| 手動輸入的上下限 | ✅ **已有** —— 接線檔 `kb` 區塊的 `[FLAG, dp, checkRange, min, max]`，來源是 golden 的 `ShowQwertyKey` |
| 寫入時的值域保護 | ⚠ **只有 `levelset`**（`system.levels.put` 收 0..4，超出整批拒寫）。`recipe.doc.put`／`system.file.put`／`system.csv.rows` **完全沒有** |
| 讀取時的值域資訊 | ❌ **沒有** —— `{value,type,raw}` 不帶 `min`／`max` |

**為什麼瀏覽器擋了伺服器還要再擋**：小鍵盤的 min/max 只在操作員用畫面小鍵盤時生效，
**WS 直連完全繞得過**。這與 20260916 §7c 對 `motTable` 鍵值格式做過的決定是同一條理由
（「WS 直連可繞過瀏覽器檢查，所以伺服器端也要擋」）。瀏覽器那道的價值是即時回饋，不是安全。

**三條要求**（詳見契約 §2.9）：讀取端點在有值域可講的欄位補 `min`／`max`；
寫入時伺服器端檢查、超出**整批拒寫不夾回**（clamp 會讓操作員以為存了 15000 其實存了 10000）；
讀到檔案裡既有的壞值**照實送不要修**（那是現場的事實，夾回去會讓問題永遠不被發現）。

---

## 20260916 升級：衝突一律走 wb_serve

> 使用者：「把 JSON Simulator 忘記吧, 後面都用不到了。如果有遇到衝突都使用 web serv 模式處理。」

同一個畫面欄位若同時有模擬器路徑與 wb_serve 路徑在餵，一律以 wb_serve 為準，
模擬器那段**直接拆掉，不並存**。20260916 稽核交付包 67 頁後實際衝突只有兩處，都已拆除：

| 檔案 | 原本 | 處置 |
|---|---|---|
| `main.html` | `HT_SETTINGS` 廣播把 `edSetupFileName`／`edWorkTemperBase`／`edSoakTime` 三格從靜態快照填進去，與 tag（`recipe.current`／`temp.sv`／`temp.soak`）互相覆蓋 | 刪掉那三格的填值段。`decisions`（選單標灰）保留——那是 View-rules 的**畫面規則**，不是機台資料，B 路沒有對應端點 |
| `HW.IoSetView.html` | `saveIO()` 經 `HTJsonWriter` 寫 `IO-config.json` | 移除 `json-writer.js`，`saveIO()` 改成空殼 |

⚠ **兩棵 HTML 樹，別改錯**（20260916 實測）：

| 樹 | 角色 | 狀態 |
|---|---|---|
| `D:\HT9045\web\page\` | **交付包，wb_serve 服務的就是它** | ✅ 權威，改這裡 |
| `D:\HT9045\page\` | `HT9045_Debug.cmd` 的 file:// ＋ C# Simulator 堆疊 | ⛔ 被裁決退場的那條路本身 |

兩棵同名檔 89 個裡已有 **44 個分歧**（另有 31 個只存在於交付包）。
例：`HW.IoSetView.html` 交付包是 20260916 14:02（已拆掉 `HTJsonWriter`），
`D:\HT9045\page\` 那份停在 20260902，**還留著寫 `IO-config.json` 的舊碼**。
在舊樹上 grep 會得到錯誤結論。（`ScreenShots.html`／`screenshot_meta.js`
依使用者指示兩份同步，那是例外。）

⚠ 本文件下面「開站載入順序」與「更新觸發與頻率分類」兩節描述的是**靜態 JSON 那條
路徑**的規格。`wb_serve` 一個都不產生那些 JSON（實測 grep 全為 0）。兩套要分開讀。

---

## `/api/system/` 與 `/api/text/` —— 機台檔案的即時讀寫（20260915 實測）

`wb_serve.exe` 提供三條 API，取代 PowerShell 一次性產生的 JSON 快照。
以下數字是 2026-09-15 對跑著的 `wb_serve`（`--dry --allow-cmd`）實測回報，不是推估。

```
GET  /api/recipe/          作用中配方的 16 份文件
GET  /api/recipe/<doc>     {sections:{sec:{key:{value,type,raw}}}}
WS   recipe.doc.put        tag=<doc>  value={"sections":{...},"dryRun":bool}

GET  /api/system/          40 支設定檔的索引（name/path/kind/available/bytes）
                           另含二進位投影 levelset（kind:"i32"）
GET  /api/system/<name>    ini -> 與配方文件完全同形狀
                           csv -> {columns:[...], keyColumn, rows:[{col:val}]}
WS   system.file.put       tag=<name>  value={"sections":{...},"dryRun":bool}
                           ack -> {changed, identical, notFound, backup}

GET  /api/system/levelset  二進位定長陣列投影（20260916 新增）
                           -> {kind:"i32", count:256, min, max, values:[...]}
                           來源 system\levelset.dat（LAST_LEVEL_SET，
                           cprod.h:1148 `int AccessLevel[256]`，1024 bytes 小端）
WS   system.levels.put     tag=levelset
                           value={"values":{"<idx>":<int>,...},"dryRun":bool}
                           ack -> {changed, identical, notFound, backup}
                           表列在 wb_serve.cpp:1131；分流在 :2148

GET  /api/text/            6 個純文字記錄來源（唯讀）
GET  /api/text/<root>      該來源下的檔案（單層）
GET  /api/text/<root>/<f>  整檔內容 {path, bytes, text}
```

⚠ **`levelset` 是唯一的二進位投影，而且它是 `Status.Security` 的真正後端。**
那 179 組 radio 存的是 `system\levelset.dat`，**不是** `config\Security_new.def`
（後者也在 `/api/system/securityNew` 服務中，但那是**另一個檔**，接錯不會報錯，
只會把權限寫到沒人讀的地方）。`levelset.dat` 沒有鍵名、沒有表頭、沒有分隔字元，
只有 256 個小端 int32，所以 `IniGet()` 那套解析器對它完全無效，必須走這條路。

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

---

## 待辦（本 skill 的落地缺口，20260916 重新盤點）

範圍已由使用者收斂：**真實 IO 後端由其他工程師實作，不歸這裡。
這邊的目標只有一個——讓 HTML 接上 `wb_serve`。**
逐頁狀態見 `page/ScreenShots.html` 表⑤（`screenshot_meta.js` 的 `PAGE_WIRE_STATUS`），
逐 tag 狀態見表⑥（`TAG_WIRE_STATUS`）。

### B 路（wb_serve）——目前的接線分級（**20260919 實測 68 頁**）

| 級別 | 頁數 | 意思 |
|---|---|---|
| `data` | **21** | 已接 `/api/recipe` 或 `/api/system`，按存檔真的會寫回檔案 |
| `kb` | **10** | **只有小鍵盤**：打得了字，按存檔不寫回任何檔案 |
| `none` | **33** | 完全未接 |

另有 **4** 頁是 `tag` 級（只接執行期唯讀顯示）。合計 21+10+33+4 = 68。

欄位合計：配方 **637**、系統檔 **947**、鍵盤 **1466**、
系統檔唯讀顯示（`sysText`，不進 `save()`）**3**、已查證無來源（顯示 `---`）**14**。

⚠ **20260916 的舊數字（67 頁／`tag` 1／`none` 36）已作廢。** 這段期間發生了：
新增 `HW.ShuttleMove.html`（+1 頁）、A 路 6 頁全面退場、`Data.Observer` 改走 wb_serve。

### ⛔ A 路（JSON Simulator）已於 20260918 全面退場

**`web\page\*.html` 的 `HTSettings.` 呼叫數實測 = 0**，
真正的 `<script src="settings.js">` 標籤 = 0。

⚠ `grep` 仍會命中 2 處，但那是**註解文字**
（`IDE.MotionView9050-LayoutEditor.html:336`、`Status.TemperFrom.html:81`）。
數這個數字時要排除註解 —— 這裡踩過一次。

最後退場的 6 頁：`HW.IoSetView`／`HW.teach`（`applyBlocks` 改讀 `/api/system/gerneral`
即時值）、`Main.MotionView`／`Main.MotionView9050`／`Status.TemperFrom`／
`IDE.MotionView9050-LayoutEditor`。共用模組 `client\ht9045_wire_livesettings.js`。

⚠ **`MotionView-i18n.json` 刻意保留** —— 它是語系字典，不是機台資料。
「退場 A 路」指的是**機台資料**不再走靜態 JSON，不是把所有 JSON 都拿掉。

⚠ **`web\JSON\` 與根目錄 `JSON\` 兩份快照不同步**（實測差 7 天、`Model` 差一個機種：
`HT-9046AT` vs 實機 `HT-9050`）。6 頁已脫離，但 `background.html` 與其他頁還在讀 `web\JSON\`。
要不要重產或整個退場，**未決**。

### tag → widget 對照：**61 / 4608**（20260919 實測）

```
共 4608 個 tag：已接 61、無 HTML 目標 4547、對照表有但伺服器沒送 0
其中這一刻有值（非 null）的有 62 個 —— 與接線無關，是來源載入與否
```

⚠ **4608 不是 117。** 舊版本寫的 117 是 20260916 的數字；20260917 之後
`pci1203.*` 家族（4481 個馬達卡診斷 tag）上線，**佔 snapshot 的 97%**，
但 HMI 一個都用不到。要談「HMI 能用的 tag」時指的是非 `pci1203` 的那 **127** 個。

對照表的唯一權威來源是 `scratchpad\gen_wire.py` 的 `TAG_PAGES`。
**不要手改 `client\ht9045_wire_*.js`** —— 那是產生物。

⚠ `gen_wire.py` 有 `verify_citations()`，每次產生會回頭讀 `WebBridgeTags.cpp`、
`tagmap.js` **與 golden 樹**，確認被引用的行真的提到該 tag。
現況：**141 條全部命中**。同名檔在本樹與 golden912 **行號不同**，引用要講明哪一棵。

### 引擎支援的 tag 綁定模式（`ht9045_wire_engine.js` 的 `tagApply`）

| 模式 | 用途 |
|---|---|
| `text`（預設） | `textContent`，`null` → `---` |
| `chip` | 狀態小方塊：true→`.on-g`／false→`.off`／**null→`.unknown`** |
| `led` | `.aled`：true→`.on`／false→熄／**null→`.unknown`** |
| `select` | `<select>` 選中對應 option；值不在選單裡就補一個 |
| `map` | 第 4 個元素給查表（例：`{"0":"Hot","1":"Ambient"}`） |
| `site` | SitePanel 格子，引擎自己從 tag 名算座標 |

⚠ **每個非文字模式都必須有第三種狀態（`unknown`）。**
布林 tag 的 `null` 是「不可知」不是「關」。只用 `toggle('on', !!v)` 的話，
斷線的燈和機台真的關著的燈**長得一模一樣** —— 沒有紅字、沒有 `---`，
操作員看著一顆熄掉的燈以為安全。

### 還沒做的

- [ ] **其餘 4547 個 tag 沒有 HTML 目標**，但其中 4481 個是 `pci1203.*`（HMI 用不到）。
      真正值得接的是非 `pci1203` 的 127 個裡還沒接的 66 個。
      ⚠ 其中 `status.*`(11)／`tower.*`(3)／`temp.pv`＋`zone.*`(8)／`pump.*`(15)／`clock.text`
      **刻意不接**（契約未定，或來源在 `wb_serve` 之下結構上不會有值）。
- [ ] **tag 協定缺 metadata**：`seq`／`at`／`trigger`／`gen` 現行訊框**一個都沒有**
      （實測 `meta=[(none)]`）。⚠ 本檔舊版寫「唯一有的是 `gen`」是**錯的**。
      客戶端 `seqCheck()` 因此永遠早退，整段防護是死碼。
- [ ] **沒有訂閱過濾**：snapshot 130,370 bytes 單一訊框，97% 是 `pci1203.*`。
- [ ] **指令通道**：目前 11 個指令，**沒有任何一個會讓機台動**。
      要讓瀏覽器操作機台，互鎖與安全判斷永遠留在 C++ 端。
- [ ] **`guard.*` 機台狀態訊號尚未交付** —— 這是目前最高優先的阻擋項，
      見 `docs\web-client\SYSTEMSTART_SIGNAL_CONTRACT.md` 與 `CPP_REQUESTS_20260918.md` §0。
- [ ] 尚未接的設定頁：`Status.CounterSel`(19)、`Data.CounterClear`(9)、`Data.ContactCT`(8)。
- [ ] 尚未決定 `--dry` 是否涵蓋全部 40 支系統檔。
      ⚠ 20260918 查證：`--dry` **只隔離 C++ loader**，A1 裁決之後 web API 一律解析到真實檔，
      所以 `recipe.doc.put` 打的是**真實配方夾**，唯一閘門是 `--allow-cmd`（啟動器預設值）。

### 相關契約與規範（20260918 新增）

| 文件 | 內容 |
|---|---|
| `docs\web-client\CPP_REQUESTS_20260918.md` | 要 C++ 端處理的 9 項，**§0 最優先** |
| `docs\web-client\SYSTEMSTART_SIGNAL_CONTRACT.md` | `guard.systemStart` 等機台狀態訊號 |
| `docs\web-client\WINDOW_REGISTRY_CONTRACT.md` | 視窗狀態總表（取代 golden 的 `fShow`） |
| `.github\specs\page-access-policy.md` | 畫面存取政策表（互斥／SystemStart 守衛） |

⚠ 兩條**方向相反**的保守規則，實作時最容易寫反：
`guard.systemStart` 不可知 → 當成 **`true`**（正在運轉）；
視窗狀態總表不可知 → 當成**該頁還開著**。兩者都往「比較擋得住」的那一側倒，但值相反。
