# 派工給 Jimmy／Steven：HT9050 Motion View 接上機台（2026-10-07）

EastSun 1007：「這個畫面沒有跟機台動作同步，是哪邊的問題」→ 決定派工給 Jimmy／Steven。

## 現況（機台實測）

- 頁面：`web/page/Main.MotionView9050.html`（`Alert.MotionView9050.html` 同一份）。畫面停在 SIM 起始圖，狀態列寫「等待 C++ Runtime 在 Production-update.json.state.motionView 發布 machine:'HT9050' 的逐格快照……（runtimeSupported:false）」。
- 頁面在 `applyRuntimeState`（Main.MotionView9050.html:1262-1279）只接受 `Production-update.json.state.motionView` 且 `machine === "HT9050"`；其他（含 HT9045 runtime 的 motionView）一律忽略。
- C++ 端**沒有任何地方發布 HT9050 的 motionView**。現有的 `JsonBridge/ChanMvTrays.cpp`（`W906_StageMotionViewTrays`）是 HT9045 版契約（trays／ledGroups／trayLocations），本頁不吃。
- `web/JSON/Machine-profile.json` 的 HT9050 仍是 `runtimeSupported:false`，note 寫「全樹沒有任何 if(MachineTypeChoice==Type_HT9050) 分派」——**已過時**：機台現在 MachineTypeChoice＝Type_HT9050（database.cpp:520-524），正式流程跑 Frank 的單 Index Z 流程（csystem.cpp DoAllProcess，2026-10-07 起機台版也開）。

## 要做的

1. C++ 每拍（或每 publish）在 `Production-update.json.state.motionView` 發布 `machine:"HT9050"` 的快照，欄位照頁面 `applyLive`（Main.MotionView9050.html 的 `function applyLive`）讀的：
   - `paused`（機台暫停中）
   - `loader`／`auto[0..2]`／`empty`：`{ cells:[0|1…], sensorOn:true|false }`（每格有沒有料、定位感測器）
   - `hotPlate`：`{ cells, soakDone }`
   - `inPP`：`{ at, z:"up"|"down", hold, vacuum }`（In P&P 在哪一站、Z、有沒有吸著料、真空）
   - `inShuttle`：`{ kit:0|1, inTest:bool }`（In Shuttle1 有沒有料、是否在 Index 下）
   - `index`：`{ z:"safe"|"kit"|"down", hold:bool }`（Index Z 高度、有沒有吸著料）
   - `dut`：`{ hasIC:bool }`
   - `outShuttle`：`{ kit:0|1, inTest:bool, yAtPick:bool }`（Out Shuttle1 有料／在 Index；Out Shuttle2(Y) 在 Out Arm 吸料點）
   - `outPP`：`{ at, z, hold, vacuum, cylinder:"front"|"back" }`（含 C_OutArmSmallY）
   - 「位置」用實際讀回（1203 encoder／ReadPos）對照教導點判站，不要用命令值。資料來源：MOT[]（MInArmX/Y/ZA、MInShuttle1、MTestZ1、MOutShuttle1/2、MOutArmX/Y/ZA、MTrayX、Loader/Auto/Empty Z）、FLCarryKit／FTestSuck／FRCarryKit／InArmSuck／OutArmSuck、MOT[MMTrayY]／MMAuto*、Cylinder[C_OutArmSmallY]、SystemStart／SoftStop。
   - 只讀，不寫任何 .ini，不動機台。
2. `Machine-profile.json` HT9050 改 `runtimeSupported:true`，更新過時的 note。
3. `state.motionView.log`（頁面寫「尚未定義」）：如果要動作記錄，一併定義。
4. 測試：headless 頁面在收到一份假的 HT9050 motionView 時切到 LIVE、各機構擺到正確站點；另一份 machine 不是 HT9050 時維持忽略。

## 參考

- 機台 HT9050 的教導點：teach.ini（runcfg\system），Index Z 吸取 -7580／放置 -7480、安全位置 0（2026-10-07 EastSun 確認）。
- 頁面版面：`web/JSON/MotionView9050-layout.json`、`web/page/ht9045_mv9050_axes.js`。
