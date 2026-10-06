> 保存來源：`.claude/skills/ht9045-html-version/references/motion-view.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Motion View 模擬層（Main.MotionView.html ← main.dfm `tsActionView`）

> 原始碼一律為 **BCB6**（Borland C++ Builder 6）：
> `D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2`（cp950/Big5）。
> 後續會改用 **VC++** 重寫，兩套來源不可混寫。

## 1. 頁面產生方式

`tsActionView` 位於 `main.dfm`（357 個物件：TALed×218、TTMyTray×53、TPanel×32…），
不是獨立表單，因此由 `_gen_dfm_abs.py` 的 **`SUBTREE_JOBS`** 機制輸出：

```python
SUBTREE_JOBS = [
    ('main.dfm', 'tsActionView', 'Main.MotionView.html',
     'Motion View（main.dfm tsActionView）', ['motionview-sim.js']),
]
```

- `parse_dfm_block(path, objname)`：**只切出該 object 區塊再解析**。
  必要原因：`parse_dfm()` 對超大 dfm 會在 stack 提前清空時把 `root` 覆寫掉，
  直接找 `tsActionView` 會 miss。
- 容器的 `Width/Height` 當成頁面 client 尺寸（1008×731）。
- `extra_js` 會在 `</body>` 前插入 `<script src="...">`。

### 連帶注意（同一支產生器）

- **`NO_OVERWRITE`**：手工維護頁（IoSetView／uteach／uMotorTest／cObserver／cBinSel）
  仍留在 `JOBS`（ComponentMap 要取元件樹）但**不覆寫 HTML**。
  之前把它們從 `JOBS` 註解掉，導致 ComponentMap 章節用位置 `zip` 配 anchor 時**整段錯位**並少 2 節。
- **`ANCHORS`**：改成 `zip(JOBS, _ANCHOR_SEQ)` 後以 dfm 為 key 查表，增刪 JOBS 不再錯位。
- `MotionView` 已在 `_gen_screenshot_pages.py` 的 `SKIP` 內，不會被截圖卡覆蓋。

## 2. 資料來源：`Sim-scale.json`

由 `_gen_simscale.py` 從 BCB6 `cinitial.cpp` 的 `void SetSimuScreenPara()` 抽出
（61 顆馬達、12 個 LED 群組）。同一顆馬達被多個機種分支重複設定時**取最後一次**（等同 cpp 順序執行結果）。

| cpp 呼叫 | 意義 | JSON 欄位 |
|---|---|---|
| `MOT[x].SetScreenScale(screenA, screenB, machineA, machineB)` | 機構座標 ↔ 畫面像素線性對應 | `screen:[a,b]`、`machine:[a,b]`、`machineExpr:[...]` |
| `MOT[x].SetPanel(fMain->元件, vertical)` | 該馬達要移動的畫面元件 | `target`、`vertical` |
| `<群組>.SetMyLed(row, col, fMain->led)` | 吸嘴/Kit/Socket LED 矩陣 | `ledGroups[群組] = [{row,col,led}]` |

`machineExpr` 保留 BCB6 原式（多為 `Prod.*` Recipe 值），HTML 端無法求值時 `machine` 為 `null`。

節錄：

| motorId | screen | target | vertical |
|---|---|---|---|
| MInArmX | 0 → 109 | pnlInArmX | false |
| MInArmY | 208 → 20 | Panel61 | true |
| MInShuttle1 | 128 → 284 | palShuttle1 | false |
| MOutShuttle1 | 296 → 433 | pnlOutSht1 | false |
| MOutArmY | 8 → 208 | pnlOutArmY | true |
| MTrayX | 130 → 702 | ALed89 | false |

> `screenA > screenB`（例：MInArmY 208→20）代表座標增加時往上/左移動，公式已自然涵蓋。

## 3. 換算規則（`motionview-sim.js` / `HTMotionSim`）

```
t  = clamp((pos - machineA) / (machineB - machineA), 0, 1)
px = round(screenA + (screenB - screenA) * t)
vertical ? style.top = px : style.left = px
```

- `pos` 取 `Motor-runtime.json` 的 `position.cmdPos`（無則 `encPos`）。
- **機構座標區間退回鏈**（Recipe 值未進 JSON 前的過渡）：
  1. `Sim-scale.json` 的 `machine`（cpp 內為常數時才有值）
  2. `Motor-config.json` 的 `params.softLimitN / softLimitP`
  3. `params.range` → `[0, range]`
  4. 都取不到 → **跳過該馬達**（不亂移）
- `target` 為 `null`（cpp 未 `SetPanel`）的馬達一律跳過。
- 首次套用前記錄元件原始 `left/top` 至 `base`，並在 `position:static` 時補 `absolute`。
- LED：以 `IO-runtime.json` 的 `points[].isOn` 切 `.led-on` 與底色。
- 輪詢 1 秒；右下角狀態徽章 `#mvSimInfo` 顯示 `Sim: motors=N leds=N`。

### 離線與 file://

- `?offline=1` / `?cppoffline=1` → 讀 `JSON/offline/*.offline.json`。
- `file://` 下優先用 `<script src="../JSON/js/<name>.js">` 墊片（Edge 封鎖 file:// XHR），
  失敗才退回 XHR；http(s) 相反。**`Sim-scale.json` 已加入 `_gen_json_shim.py` 的 `FILES`。**

### `window.HTMotionSim`

| API | 說明 |
|---|---|
| `start()` / `tick()` / `stop()` | 啟動、單次更新、停止輪詢 |
| `apply()` | 立即重套用，回 `{motors, leds}` |
| `setPosition(motorId, pos)` | **測試用**：不改 JSON 直接餵位置驗證換算 |
| `debug()` | 取 `scale / motorCfg / motorRt / ioRt / base` |

## 4. Tray 位置與多國語系

- 立體視圖下方的 `Tray Location Diagram` 只讀 `Production-update.json.state.motionView.trayLocations`。每個 Tray 以一張狀態卡呈現：Sensor LED 只讀 `inside.sensorOn`，九格盤面只讀 `inside.motorHasTray`；兩者不得互相推論，也不得由馬達座標推論。
- Loader `upper`／`lower`／`inside` 的 Motor 分別是 `MOT[MMTrayZ]`／`MOT[MMTrayY_Car]`／`MOT[MMTrayY]` 的 `fHasTray`；Sensor 分別是 `Sen[SnLoaderTrayHasTray]`／`Sen[SnLoaderCarHasTray]`／`Sen[SnLoaderSureTray]` 的 `IsOn()`。
- Auto1~6 以 index `0..5` 發布：Motor 為 `iAutoZMot`／`iMMAuto_Car`／`iMMAuto` 的 `fHasTray`，Sensor 為 `SnAutoTrayHasTray`／`SnAutoTrayCar`／`SnAutoTrayDetect` 的 `IsOn()`。
- Fix1~6 只有 `inside`：Motor 為 `MOT[iMFixTray[index]].fHasTray`，Sensor 為 `Sen[SnFixedTrayDetect[index]].IsOn()`；`iMFixTray[0..5]` 指向 `MManualTray1..6`，沒有 Auto/Loader 的外側暫存位置。
- 整機 SVG 的 Fix tray 僅在已有非零 X/Y teach 座標時繪製；目前 Fix1~3 有位置，Fix4~6 的教導值是 `0`，因此仍只在下方位置示意呈現，不可自行補畫虛構座標。
- SVG Tray 的 `fHasTray:true` 只顯示淺灰格子，格數一律取當前 Tray Form 的 `X Division × Y Division`；`false` 隱藏格子，`null` 亦不得當作空盤。名稱與方形 Sensor LED 必須位於盤框上方，盤內只能顯示格子。Fix 使用 `SnFixedTrayDetect[i].IsOn()`；Loader/Empty/Color 使用 inside 的 Sensor；Auto1~3 使用 `SnAutoTrayDetect[i].IsOn()`：ON 為綠色、OFF 為灰色、未知保持中性。兩種提示絕不可互相推導。
- 立體 SVG 的 Sensor LED 必須位於每個盤框的左上方，不得落入盤內格子；適用於目前有幾何的 Loader、Empty、Color、Auto1~3、Fix1~3。
- 下方區域保留 `Loader`、`Empty`、`Color`、`Auto1~6`、`Fix1~6` 共 15 張 Tray Location 狀態卡，不採用跨欄矩陣表。每張卡的九格盤面以 `inside.motorHasTray=true` 顯示滿格、`false` 顯示空格、`null` 顯示未知格；左側 LED 以 `inside.sensorOn` 顯示 ON/OFF/unknown。
- 最新的下方 Tray 狀態呈現改為矩陣表，不再顯示 Tray Location 小卡或 Fix。欄位固定為 `Loader`、`Empty`、`Color`、`Auto1~6`；列固定為 `Up Sensor has tray`、`Up has tray`、`Car sensor has tray`、`Car has tray`。Sensor 列顯示 `upper/lower.sensorOn` LED，HasTray 列直接顯示 `upper/lower.motorHasTray` 的 `true`／`false`／`—` 原始值。此表只變更整機圖下方 DOM，不得改動 SVG。
- Tray 名稱應緊鄰盤框左上方的 LED，不得回到盤內。Shuttle 的站點輔助線可保留，但不得顯示 `站A`、`站B`、`站C` 文字；Socket 標籤固定為 `Test Socket` 並置於 Socket 方框外側。
- Shuttle Kit／Index／Socket 的格數必須依 **BCB6** `TfMain::MainFormChange()` 顯示規則，由 `Production-update.json.state.context.setupForms["Setup.SetUp"].testModeIndex` 決定，不可直接共用 ArmCondition 的 rows/cols：1xN 僅顯示一列；2x3 顯示 4 欄；2x5／2x6／2x8／4x4／32-site 顯示 8 欄；`_32Site4X8N`（index 15，`NN_2Row`）的 Socket 額外顯示 C/D 兩列，故 Shuttle/Index 為 $2\times8$、Socket 為 $4\times8$。
- Tray 狀態表的 Auto 欄位只顯示 `OUTPUT_PANELS` 中存在的 Auto key，因此未啟用的 Auto4~6 自動隱藏；啟用後立即出現，並非寫死三個 Auto 欄。
- 立體底板的入料側與測試側均為 463 單位寬：入料為 X=`7..470`，測試為 X=`470..933`，出料自 X=`933` 起。Shuttle 的原始 Teach 行程保留為 `G.TRAVEL_RAW`，顯示行程 `G.TRAVEL` 上限為 400，使 Station B/Test Socket、Out-Kit 和 Shuttle 長度一併在測試側內縮短。
- AutoClean 以 Teach 座標為基準向左 100 單位，避免與 Loader Tray 投影重疊。In Rotator 位於入料側、兩條 Shuttle 的 Y 中線；Out Rotator 位於 Fix Tray 前側深度之外，避免與 Fix Tray 重疊。
- 所有立體 Tray 的名稱與 Sensor LED 仍以同一基準向右 5px；LED 的 Y 再上移 2px，並保持在盤內格子上方。
- `HTSettings.load()` 回傳完整設定 wrapper 時，Motion View 必須從 `settings.recipe.documents` 讀取 Recipe JSON。Renderer 對已移除的 `cntBin`、活動摘要與 scrub 元件必須採可選更新；16-picker 的 Z 軸摘要須為第 9 支以上提供遞補名稱，否則不得阻斷 Runtime snapshot 顯示。
- 測試側內的 Test Socket、Index Y 軌道、Station guide line、Index badge 和其文字一律以縮短後的 `G.B` 為基準，禁止保留原始 469.4 mm 行程位置。Empty 與 Color Tray 必須完整落在測試側 X=`470..933`；Color 的中心上限為 X=`850`，避免盤寬跨過右界。
- 出料側的 Y 向列序固定為：上方 `SHT2`，中線 `Out Rotator | Fix1~6`，下方 `SHT1`，最下方 `Auto1~6`。Fix4~6 仍遵守非零 Teach 座標才繪製的規則。Shuttle 軌道中心為 `G.SHT_RAIL_X`、長度為 `G.SHT_RAIL_W=2*G.TRAVEL+140`，不再使用固定 1080 的長度。
- 測試側中心固定為 `G.TEST_X=(470+933)/2`。Test Socket、Index Y 軌道、Index 動態頭、badge 與 Shuttle/Kit 視覺位置均以 `G.SHT_DRAW_OFFSET=G.TEST_X-G.B` 左移到中心；實機 Teach 行程仍保留，不可修改。Tray Arm X 軌與動態 Tray Car 使用 `G.TRAY_RAIL_Y=-1040`，位於 Tray 前側下方。OutArm 沒有 Runtime 馬達座標時，預設停在 Out Rotator 的 X 與 SHT2 列，避免出現無來源的中間位置。
- InArm X 軌使用 `G.IN_ARM_RAIL={x:240,width:440}`（X=`20..460`），對齊入料側；OutArm X 軌使用 `G.OUT_ARM_RAIL={x:1390,width:720}`（X=`1030..1750`），覆蓋出料側操作範圍。OutArm 投影下方的矩形為 Shuttle 的 `Out-Kit`，承接 Index 已測 IC，並非另一個 Tray。
- `lanes` 只能建立 Loader、Empty、Color 的前側 Tray；Auto1~3 必須另以 `G.AUTO[]` 與 `LANE.auto1.y0` 在出料側建立盤框和 presence grid。將 Auto 混入 `lanes` 會先在 Loader Y 建立錯位 Auto grid，再畫一次實際 Auto Bin Tray，形成看似多出一盤的重複 Tray。
- Empty 與 Color 的 X 各由 Teach 投影基準左移 60，Color 中心上限為 X=`790`。Tray Arm 動態投影的方形 LED 只讀 `motionView.trayArm.motorHasTray`，其權威來源是 `MOT[MTrayX].fHasTray`：true 綠、false 灰、null 中性；絕不可由 MTrayX 位置、盤面格點或其他 Tray 狀態推論。
- SVG 畫面座標 Y=`364` 為 Tray 區上方的橘色水平分隔線。Loader、Empty、Color、以及已具 Teach 座標的 Auto Tray 盤框頂緣必須大於該值；Auto Tray 使用與 Loader 相同的下方列。Tray Arm LED 固定於測試側中心，範圍 Y=`346..354`，即分隔線上方 10px，不隨 Tray Arm 車體移動。
- Empty／Color 的 Sensor 三位置硬體對照尚未確認，必須發佈 `sensorOn:null`；未安裝、尚不可用或未知的任一旗標一律用 `null`，HTML 顯示未知且不自行補值。
- `JSON/MotionView-i18n.json` 是 Motion View 的唯一翻譯字典。頁面透過 `HTSettings.loadJson()` 載入，收到主畫面 `HT_LANG` 訊息或讀取 `ht9xxx-lang` 後切換 `en`／`zh`／`ja`／`ko`；修改 JSON 後需更新 `JSON/js/MotionView-i18n.js` shim。

## 5. 驗證紀錄

Playwright（`http://localhost:8899`，Playwright 不能開 `file://`）：

- 載入後 `Sim: motors=31 leds=211`（61 顆中 31 顆有 `target`＋可用區間）。
- `setPosition('MInArmX', 5000)` → `pnlInArmX.left` `0px` → `55px`（screen 0→109 之中點）✅
- `setPosition('MOutArmY', 10000)` → `pnlOutArmY.top` `8px` → `208px`（上限）✅ 垂直軸正確

## 6. 重跑順序

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_simscale.py    # cpp → Sim-scale.json
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py   # JSON → js 墊片
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_dfm_abs.py     # dfm → HTML
```

## 7. Simulator Tray Matrix 驗證

- JSON Simulator 的 `simulate-tray-states` action 會以原子 snapshot 寫入 Auto1~6 與 Fix1~6 的 `inside`：`{true,true}`、`{true,false}`、`{false,true}`、`{false,false}`、`{true,null}`、`{null,null}`。
- Simulator 啟動前須執行 `Encoding.RegisterProvider(CodePagesEncodingProvider.Instance)`，否則 .NET 無法讀取 CP950 的機台 INI 並在 WebSocket `9045` 建立前失敗。
- 2026-09-08 Browser 驗證：Motion View 收到 `Simulator tray state matrix`，顯示 4 個 Sensor ON、2 個 OFF、3 個 unknown LED 與 160 個可見 Tray cell；頁面沒有 console error。
- 2026-09-08 區域重排驗證：Empty X=`650.11`（盤範圍 `580..720`）與 Color X=`850`（盤範圍 `780..920`）皆在測試側；Socket/Index guide X=`822.29`，Shuttle 軌道長度為 `940`，列序符合 SHT2 / Out Rotator+Fix / SHT1 / Auto。
- 2026-09-08 置中驗證：Socket/Index guide X=`701.5`、Shuttle offset=`-120.79`、Tray Arm rail Y=`-1040`；OutArm fallback=`{x:1159.49,y:-505.6}`，對應 Out Rotator 與 SHT2。
- 2026-09-08 重複盤修正驗證：SVG `D.trayViews` 僅保留出料側 `auto1/auto2/auto3`，每盤各有當前 Tray Form 的 40 個 presence cell；不再建立 Loader Y 的 Auto grid。
- 2026-09-08 Tray Arm LED 驗證：Simulator 發布 `trayArm.motorHasTray=true` 後，Motion View LED class=`fix-sensor-led on`、aria label=`MTrayX HasTray ON`；Empty/Color 中心分別為 X=`590.11` / `790`。
- 2026-09-08 Tray 分隔線驗證：線 Y=`364`；Loader、Empty、Color、Auto1~3 盤框頂緣均為 Y=`364.8`，全數在線下；Tray Arm LED=`Y=346..354` 且維持綠色。

<!-- preserved-content:end -->
