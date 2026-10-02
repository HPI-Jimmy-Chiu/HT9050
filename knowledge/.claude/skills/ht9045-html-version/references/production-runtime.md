# Production Runtime JSON

`JSON/Production-runtime.json` 是 HTML 模擬版開站載入的生產資料 baseline。schema 1.3.0 中只有
`machineRecord` 是 startup-only；其他區塊由 `Production-update.json` 事件快照覆蓋。HTML 不直接讀取
BCB6 使用的 `.dat`、`.csv` 或 `config.ini`；`D:\AI_TempFile\_gen_production_runtime.py`
先將來源轉為 UTF-8 JSON，再由 `_gen_json_shim.py` 產生 file:// 傳輸墊片。

## BCB6 來源

| JSON 區塊 | 來源 | BCB6 對照 |
|---|---|---|
| `lotInfo` | `config/config.ini [Lot Info]`、選用 `[RFID]` | `TfLotInfo::ReadWriteLotInfo()`、`SetLotID()` |
| `machineRecord.normal/spare` | `system/machinerecord.dat`／`machinerecordRealCCD.dat` | `LoadMachineRecord(bool bSpare)` |
| `observerRecord` | `system/lastdata.dat`（必要時採 `_backup.dat`） | `ReadLastDataFile()`、`TfObserver::GetMachineData()` |
| `observerPanels` | LastSet＋`General-config.json [Version]` | `GetMachineData()`、`ProcessRunInfo()`、`ShowVer()` |
| `arms.current/history/byLot/autoClean` | `system/Arm*.dat` 與 `_backup.dat` | `TArm::ReadFile()`、`WriteFile()` |
| `lotSummary` | `system/LotSummary.csv` | `TLotSummary::ReadFile()`、`WriteFile()` |
| `testCategory.current/byLot` | 由 Arm 快照重算 | `TEST_CATEGORY::UpdataCount()`、`UpdataYield()` |
| `productionStreams.socketCounters` | 生產期 TMySocket 記憶體 | `SetTesterBin()`、`ClearALLCT()` |
| `productionStreams.testStatus` | TestSocket 與顯示狀態 | `ProcessShowTestStatus()` |
| `productionStreams.continuousFail` | 生產期連續失敗陣列 | `CheckContinuoussFail()` |
| `productionStreams.yieldHistory` | 生產期記憶體，無持久化來源 | `HistroyBin`／`HistroyPassFail`、`UpdateBin()` |
| `productionStreams.testTiming` | 生產期記憶體，無持久化來源 | `ProcessRunInfo()`、`TimeInfoGrid` |
| `productionStreams.temperature` | 溫控器通訊值，無關站持久化來源 | `UN150Read[]`、`ShowThermo()` |

`source.toolchain` 固定標記 `BCB6`。來源文字以 CP950 讀取；JSON、shim 與 HTML 一律 UTF-8 無 BOM。

## MachineRecord

目前 BCB6 `struct MachineRecord` 經 4-byte 對齊後固定為 876,908 bytes。產生器以
`ctypes.sizeof(MachineRecord)` 與實檔尺寸完全相同才解碼，否則回傳 `available:false`，不猜測舊版 layout。
`normal` 對應 `bSpare=false`，`spare` 對應 `bSpare=true`。目前 `machinerecordRealCCD.dat`
為 2011 年舊格式 249,552 bytes，因此明確標記不相容。

輸出涵蓋 Tray/Loader、Carry Kit、Test Socket、In/Out Arm、HotPlate、Auto Site Map、Magazine、
Tray ID 與流程 routing。大型 30×70、2×50×50 矩陣採 `[row,col,value]` 或
`[plate,row,col,...values]` 稀疏格式，未列位置視為 0。`restoreEligible` 直接對應
`bInitialStart`；BCB6 normal 流程只有該值為 true 才進入實際還原。

## Observer Machine Data

`observerRecord` 從 LastSet 穩定前綴讀取 `SendCT[4]`、`SystemAccSecond[4][8]`、`iJamCount[3]`。
`observerPanels.operating` 使用 BCB6 相同公式與格式產生 Power ON Time、Running Time、Product Time、
Loading Count、MUBA、MTBA、MTBF。`observerPanels.version` 的 Model、Serial No、Machine ID、Factory、
Handler Version 來自 `General-config.json [Version]`。

GPIB／ESD／ATC／TTL-RS232 Version 只在通訊成功後寫入 `RunInfo`，關站資料沒有持久化；JSON 以
`persisted:false` 與空值表示，HTML 不再顯示 DFM 示範字串。

## TArm 格式

格式依 `TArm::ReadFile()` 明列的檔案大小辨識：

| Bytes | 每站 uint32 數 | Bin 解碼規則 |
|---:|---:|---|
| 2,432 | 19 | 固定 15 Bin |
| 12,800 | 100 | 依目前 Recipe `iTestBinCount` |
| 13,312 | 104 | 依目前 Recipe `iTestBinCount` |
| 33,280 | 260 | 依目前 Recipe `iTestBinCount`，容量上限 256 |

每個 Socket 依序為：

```text
pass, fail, storedTotal, binCounts[iTestBinCount], ifError, reserved...
```

`ifError` 的位置是 `iTestBinCount+3`，不是結構末端。輸出會把未持久化的 Bin 補 0 至 256，
每站及整體都提供 `pass`、`fail`、`total`、`passYield`。檔案完整性的硬性條件為：

- `storedTotal == pass + fail`

符合時標記 `totalConsistent:true` 與 `consistent:true`。另提供診斷欄位
`binAccountingComplete = (sum(binCounts) + ifError == pass + fail)`；它不作為檔案無效條件，
因為 History/ByLot 的 Bin 累計可能跨越 Recipe Bin 定義或不同清除週期，真實檔案可大於 Lifetime total。

主檔缺少時使用 `_backup.dat`；主檔與備份內容不同時選備份，並在 `source.selected` 標記
`backup-differs`，避免載入可能尚未完整寫入的主檔。未知檔案大小不猜測結構，回傳
`available:false` 與錯誤訊息。

## TLotSummary 格式

`LotSummary.csv` 必須是 33 列 × 256 欄：前 32 列對應 Aa-Dh，最後一列是儲存的
`totalCategory`。產生器另重算 `computedTotalCategory`，兩者相同才標記 `consistent:true`。

## TEST_CATEGORY

`TEST_CATEGORY` 不直接持久化。`testCategory.current` 由 current Arm0/Arm1 重算，
`testCategory.byLot` 由 byLot Arm0/Arm1 重算；`binCount` 取目前 `Setup-current.json` 的
`Tester.Data [RS-232C] Bin Count`，但 Arm 原始 256 Bin 仍完整保留。

`sites[]` 同時提供 Socket 聚合與 `arms[0..1]` 明細，供 `Data.Observer.html` 切換 Head/Socket
數字或百分比。`siteMap` 與 Recipe 名稱保存在 `context`，無法確認 NN 執行期旗標時，
`strategy` 明確標記 `physical-socket`，不得把它誤稱為 BCB6 當下的完整記憶體鏡像。

## Production Streams（schema 1.3.0）

以下六組資料只在生產期間存在。開站 JSON 僅宣告 contract，初值一律 `available:false`；C++/VC++ bridge
取得真實資料後才填值，並遞增 `seq`、設定 `updatedAt`。不可從 DFM Caption 或示範格資料補值。

- `socketCounters.sites[]`：TMySocket lifetime、BySite window、Bin counts 與三種 ByBin pass counter；`SetTesterBin()` 或 `ClearALLCT()` 後更新。
- `testStatus.arms[]`：目前 item/bin/pass/hasDevice/color；`ProcessShowTestStatus(Index)` 後更新。
- `continuousFail`：socket/arm/AutoClean/special-bin 的 count、threshold 與觸發結果；`CheckContinuoussFail(Index)` 後更新。
- `yieldHistory.rows[]`：`{target:"mtRowA",sites:[{name:"Aa",history:[{bin,pass}]}]}`；Test Complete 後 `HistroyBin/HistroyPassFail` 搬移即更新。
- `testTiming.rows[]`：六欄 `site/startTime/endTime/testTime/indexCycleTime/indexTime`，對應 Now、Last 1..9、Average；`ProcessRunInfo()` 完成後更新。
- `temperature.channels`：鍵為 `hotPlate/shuttle/aa1/ac1/ae1/ag1/aa2/ac2/ae2/ag2/ccd`；值可為純顯示值或 `{value,status}`，`status=normal|warning|alarm`。`configuredIntervalMs` 由 bridge 提供，建議 250 ms。
- `speedView`：`TfShowMessage::sgdSpeedView` 的六組 `cSpeed` 設定投影：`{available,visible,autoSpeed,showTotal,indexAccelVisible,rows:[{name,speed,accel}]}`，rows 順序固定為 Index Arm、Input Arm、Output Arm、Tray Arm、Shuttle 1、Shuttle 2。`TfSpeed::spbSaveClick()` 完成 `ReadFile()/DoIniDataToForm()` 後及 `sbtExitClick()` 均應更新。`autoSpeed:false` 且所有 Speed/Accel 相同時 `showTotal:true`，ShowMessage 僅顯示 `Motor Speed` 一列；`indexAccelVisible` 只在 BCB6 的 `CUSTOMER_CODE==CC_SCS` 條件成立時為 true。

`Data.Observer.html` 與 `Status.TemperFrom.html` 在 unavailable 時分別顯示空表與 `---`，不顯示 `false` 或 DFM demo 溫度。

### TestCategory 與 Yield 顯示頁

- `Data.TestCategory.html` 顯示的是每個 Arm/Site 的**當次**測試結果，資料只能取自
	`productionStreams.testStatus`，不可誤用累計的 `testCategory.current`。每筆使用
	`row`／`col`／`bin`／`hasDevice` 定位；`color` 為 `green` 或 `red` 時優先套用，否則由
	`pass` 判斷。Bin `0` 是有效測試結果，必須顯示。`hasDevice:false` 或缺少 bin 時保持空白。
- `Data.Observer.html` 的 Tester Category 表格顯示**目前累計**的 Category count，資料取
	`testCategory.current`；Contact Count (Kinds) 使用 `arms.current`，Contact Count (History)
	使用 `arms.history`。兩者均以 Arm、`row`、`col` 的 Site `total`／`pass` 欄位重建，並依
	radio 選項顯示 Total、Kinds（Pass）或 Kinds(%)。
- `Data.Observer.html` 的 Yield tab 讀取 `yieldHistory.rows[].sites[].history[]`，順序固定為
	`Now`、`Last 1` 至 `Last 9`，最多 10 筆。每筆 `{bin,pass}` 依 `pass=true/'pass'` 顯示綠色，
	`pass=false/'fail'` 顯示紅色；stream 不可用時清空資料格。

## C++ Runtime Bridge


- `context.startModeOptions`／`context.activeRunStartMode`：由 BCB6 `TfMain::SetStartModeData()` 匯出 `cbRunStartMode` 的實際可選 `StartModeName[]` 與目前值。選項必須已套用 `LastSet.iStartMode`、SCK ART、ASM、QA、ART、FIFO、MRT 等執行期條件，HTML 不得以固定 DFM Items 取代。`Main.html` 在尚未取得此欄位時只顯示基本四個 BCB6 模式。
- `context.activeBinSelectIndex`：目前生效的 BCB6 `iTestRunMode` / `BinSelect[]` index（0=RT，1=FT，2=OffLine，3=RT_ART，4=FT_ART，5=RT_MRT，6=FT_MRT）。每次 Start Mode 或 FT/RT/ART/MRT phase 切換時更新。`Data.SortCT.html`、`Status.ShowBinSelect.html`、`Setup.BinSel.html` 以此選擇相同 Recipe Binasgn variant；這是唯一可正確反映 `bFTBin2RTBin` / Prime RT 共用邏輯的 runtime 值。
- `context.mainControls`：BCB6 Main 畫面的權威 UI 投影，供 `Main.html` 同步 `palTesterMode`、`imgTester`、`imgTempOnOff`、`imgRunMode`、`runModeValue`、三個 fan/light 按鈕、FT/RT/EQC、Normal/Prime 與 `palFunc[]`。每個一般控制項用 `{caption,visible,enabled,state,status,color}`；`palFunc` 是 `{key,caption,visible,enabled,status,color}` 陣列，key 固定採 `TfMain::ePalFunc` 的 enum 名稱，而非數字 index。事件命令對照與完整 key 清單見 [main-control-commands.md](main-control-commands.md)。HTML 不可從 request intent 推測或修改最終顯示狀態。
- `productionStreams.speedView`：C++ 對 `Status.ShowMessage.html` 的權威速度顯示值。`Setup.Speed.html` 在 Debug 編輯中會廣播 `HT_SPEED_VIEW` 作跨 iframe 預覽；C++ snapshot 到達後必須覆蓋此預覽。完整列規則與 BCB6 函式對照見 [speed-show-message-sync.md](speed-show-message-sync.md)。
- `Runtime-bridge-contract.json`：工程師入口，定義 producer/consumer、八種事件、必填 envelope、clear scope 與原子寫檔步驟。
- `Production-update.json`：C++ 每次事件輸出的完整 mutable snapshot。包含 `context/lotInfo/observerRecord/observerPanels/arms/lotSummary/testCategory/productionStreams`，禁止包含 `machineRecord`。
- `Production-update-ack.json`：HTML 套用回報骨架；只在 debug File System Access 已授權時保證可寫，release 不依賴 ack。

background 每 250 ms 重新載入 `Production-update.js`，只有新 `seq` 才合併並廣播。C++ 必須同時原子覆寫 JSON 和 shim：寫 `.tmp`、flush/close、replace target，避免瀏覽器讀到半份內容。採 full snapshot 而非 delta，是為了 polling 跨過中間事件時仍能得到最終一致狀態。

## 開站與頁面

載入順序：Setup shims → `Production-runtime.js` → `Production-update.js` → `settings.js`。`HTSettings` 以
`settings.production` 廣播給 iframe：

- `Data.LotInfo.html`：Lot No、Device、Run Mode、Operator、Start Time、Loading。
- `Data.Observer.html`：Device 標題與 Tester Category 表格。
- `Data.Observer.html`：另載入 Counter 頁 GroupBox1 Operating Information 與 GroupBox2 Version 的 TPanel。

更新來源後依序執行：

```powershell
py -3 D:\AI_TempFile\_gen_production_runtime.py
py -3 D:\AI_TempFile\_gen_json_shim.py
```