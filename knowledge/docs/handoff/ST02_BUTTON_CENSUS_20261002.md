# ST02-C12 按鈕普查：「看得到、按了沒反應」（靜態＋點擊實測）

> **golden 基準：906_0625_Steven（`D:/HT9045/HT9011UC_Code_V3.33.906.0_20260625_Steven`）；0618 沒有對照**（STEVEN-NB3 讀不到 0618，NIGHT_REPORT §0 #79／#80）。
> 量測樹：main `7238673d 2026-10-04 18:35:26 +0800`。只量、只分類，**沒有改任何程式**（TO_STEVEN.md §3 ST02-C12）。
> 產生：`HT9011UC_Cpp_V3.33.906.0/tools/webprobe/c12_button_census.py`（靜態）＋ `c12_click_probe.py`（無頭 Edge＋假伺服器，**已合併實測結果**）。每顆的完整欄位在同名 `.tsv`。

## 0. 白話摘要

- 範圍：外框 `web/background.html` WINDOWS 表開得到的頁＋Alert 覆蓋頁，共 64 頁、1859 顆按鈕（golden 的 TButton／TBitBtn／TSpeedButton，加上網頁自己的 `<button>`）。
- 看得到又沒有變灰的：**430 顆**（其餘：靜態藏起來 1312、變灰 105、開發用頁 12）。
- 實測按了**什麼都沒發生**（沒送 WS、沒有 POST、沒有視窗動作、畫面沒變）：**110 顆**，依 golden／移植樹分成 A～E（§1）。

## 1. 分類與建議誰做

| 分類 | 意思 | 顆數 | 建議誰做 |
|---|---|---:|---|
| `dead/?` | 實測沒反應，不是 golden 元件、也沒有處理器線索 | 110 |  |
| `OK-cmd` | 實測送了 C++ 認得的命令 | 67 |  |
| `OK-http` | 實測送了 POST／PUT | 2 |  |
| `OK-ui` | 實測只有畫面／視窗動作（沒送 C++） | 43 |  |
| `s:unbound/?` | 靜態：沒有 script 提到這顆 id，不是 golden 元件、也沒有處理器線索 | 186 |  |
| `s:unbound/?+E?` | 靜態：沒有 script 提到這顆 id，不是 golden 元件、也沒有處理器線索；WORKLOG_MACHINE §4 有提到（id 或頁） | 22 | 機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| `greyed(static)` | 網頁 disabled | 1 |  |
| `greyed(probe)` | 實測變灰 | 104 |  |
| `hidden(static)` | 網頁 display:none／visibility:hidden（不算「看得到」） | 254 |  |
| `hidden(probe)` | 實測看不到（含 C++／FormShow 藏的） | 1058 |  |
| `dev-page` | IDE.*／ScreenShots 開發用頁，不算 | 12 |  |

分類規則（`c12_button_census.py` classify／dead_cat）：C0 > C > D > A > B；D 只看處理器**本體**（呼叫下去的函式不追），讀 `MOT[i]` 位置不算 D，`MOT[i].Gali_MotMove(...)`、`ServoOnOff(...)`、`SW[..].On()`、`SetOutput*`、`ADAM_Write*`、`fMain->Start()` 算。E 只是「§4 文字裡出現這個 id 或頁名」，要人看。

## 2. 每頁一列

| 頁 | golden dfm | 全部 | 藏 | 灰 | 看得到 | 主要分類（顆數） |
|---|---|---:|---:|---:|---:|---|
| Main.html | — | 2 | 1 | 0 | 1 | s:unbound/?+E? 1 |
| Data.SortCT.html | — | 1 | 0 | 0 | 1 | OK-cmd 1 |
| Data.ContactCT.html | — | 2 | 0 | 0 | 2 | OK-cmd 2 |
| Data.LotInfo.html | — | 15 | 13 | 2 | 0 |  |
| Status.TemperFrom.html | — | 2 | 1 | 0 | 1 | dead/? 1 |
| Data.Observer.html | — | 35 | 33 | 0 | 2 | OK-cmd 2 |
| Status.ShowMessage.html | — | 1 | 1 | 0 | 0 |  |
| Status.ShowBinSelect.html | — | 4 | 3 | 0 | 1 | dead/? 1 |
| Setup.OffSet.html | — | 89 | 73 | 0 | 16 | dead/? 15, OK-ui 1 |
| Setup.Speed.html | — | 6 | 0 | 0 | 6 | dead/? 6 |
| HW.IoSetView.html | — | 17 | 17 | 0 | 0 |  |
| Config.Configuration.html | — | 49 | 48 | 0 | 1 | dead/? 1 |
| Status.CounterSel.html | — | 1 | 0 | 0 | 1 | dead/? 1 |
| Data.CounterClear.html | — | 2 | 0 | 0 | 2 | dead/? 1, OK-cmd 1 |
| Data.Builder.html | — | 5 | 0 | 4 | 1 | OK-cmd 1 |
| Config.DIOInterFaceCFG.html | — | 4 | 0 | 1 | 3 | dead/? 3 |
| Status.LtcSensor.html | — | 16 | 1 | 14 | 1 | OK-ui 1 |
| Status.TowerLight.html | — | 1 | 0 | 0 | 1 | dead/? 1 |
| HW.OmronEJ1N.html | — | 15 | 11 | 4 | 0 |  |
| Setup.QAMode.html | — | 2 | 0 | 0 | 2 | dead/? 2 |
| Setup.BarCode.html | — | 87 | 82 | 0 | 5 | dead/? 5 |
| HW.MyCCLinkSensor.html | — | 80 | 54 | 25 | 1 | dead/? 1 |
| Setup.Cleaning.html | — | 8 | 2 | 0 | 6 | dead/? 6 |
| Setup.Contact.html | — | 20 | 15 | 2 | 3 | dead/? 3 |
| Setup.TesterIF.html | — | 3 | 1 | 0 | 2 | dead/? 2 |
| Status.GroundMan.html | — | 5 | 0 | 3 | 2 | dead/? 2 |
| Setup.Ld_ULd.html | — | 3 | 0 | 0 | 3 | dead/? 3 |
| Status.Security.html | — | 187 | 6 | 0 | 181 | s:unbound/? 180, OK-cmd 1 |
| Setup.TrayForm.html | — | 4 | 1 | 0 | 3 | dead/? 3 |
| Setup.SCK_ART.html | — | 8 | 2 | 0 | 6 | dead/? 6 |
| Setup.YieldMonitoring.html | — | 3 | 1 | 0 | 2 | dead/? 2 |
| Setup.HotPlate.html | — | 2 | 0 | 0 | 2 | OK-cmd 1, dead/? 1 |
| Setup.SetUp.html | — | 9 | 0 | 0 | 9 | dead/? 9 |
| Data.SmartDiagnostic.html | — | 6 | 1 | 2 | 3 | OK-ui 2, dead/? 1 |
| Data.StartCondition.html | — | 88 | 84 | 0 | 4 | dead/? 4 |
| Setup.Temp_Set.html | — | 10 | 7 | 0 | 3 | dead/? 3 |
| Setup.BinSel.html | — | 8 | 0 | 0 | 8 | dead/? 8 |
| HW.teach.html | — | 865 | 782 | 23 | 60 | OK-cmd 32, OK-ui 28 |
| HW.MotorTest.html | — | 52 | 24 | 1 | 27 | OK-cmd 18, OK-ui 9 |
| HW.home.html | — | 2 | 1 | 0 | 1 | OK-cmd 1 |
| HW.ShuttleMove.html | — | 26 | 7 | 17 | 2 | dead/? 2 |
| Setup.TrayAssignment.html | — | 2 | 0 | 0 | 2 | dead/? 2 |
| Setup.ContactForce.html | — | 3 | 1 | 0 | 2 | dead/? 2 |
| HW.VacuumUnit.html | — | 7 | 1 | 4 | 2 | dead/? 2 |
| Setup.AGV.html | — | 4 | 0 | 0 | 4 | dead/? 4 |
| HW.HandlerSys.html | — | 5 | 2 | 0 | 3 | dead/? 2, OK-cmd 1 |
| Main.gbControlBtn.html | — | 1 | 0 | 0 | 1 | OK-cmd 1 |
| Main.MotionView.html | — | 1 | 0 | 0 | 1 | OK-ui 1 |
| Main.CommView.html | — | 9 | 5 | 0 | 4 | OK-cmd 4 |
| Main.Record.html | — | 2 | 1 | 0 | 1 | OK-cmd 1 |
| Main.AOAInfo.html | — | 1 | 0 | 0 | 1 | dead/? 1 |
| eventlog.html | — | 8 | 1 | 0 | 7 | s:unbound/? 4, OK-http 2, OK-ui 1 |
| testercomm.html | — | 23 | 2 | 0 | 21 | s:unbound/?+E? 21 |
| IDE.StyleGuide.html | — | 2 | 0 | 0 | 0 |  |
| IDE.I18nEditor.html | — | 4 | 0 | 0 | 0 |  |
| IDE.WidgetTemplates.html | — | 6 | 0 | 0 | 0 |  |
| HW.TrayEdit.html | — | 4 | 1 | 3 | 0 |  |
| Data.FTPClient.html | — | 8 | 8 | 0 | 0 |  |
| Alert.MotionView.html | — | 1 | 1 | 0 | 0 |  |
| Alert.MotionView9050.html | — | 5 | 3 | 0 | 2 | s:unbound/? 2 |
| Alert.MyMessageBox.NonStop.html | — | 1 | 0 | 0 | 1 | dead/? 1 |
| Alert.Note.NonStop.html | — | 1 | 0 | 0 | 1 | dead/? 1 |
| Alert.Note.html | — | 8 | 8 | 0 | 0 |  |
| Alert.Password.html | — | 8 | 6 | 0 | 2 | dead/? 2 |

## 3. 每顆一列（看得到又沒有變灰的 430 顆；藏起來／變灰的只在 .tsv）

欄位：頁、id、caption、分類、送出的命令（實測）、C++ 認不認得、golden 處理器（906_0625_Steven 檔名:行）、會動機台、建議誰做。

| 頁 | id | caption | 分類 | 送出（實測） | C++ | golden 處理器 | 動機台 | 建議誰做 |
|---|---|---|---|---|---|---|---|---|
| Main.html | （沒有 id ×1） | 🔄 Set … | `s:unbound/?+E?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Status.Security.html | （沒有 id ×180） | 🔑[00] Main - Tools … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| eventlog.html | （沒有 id ×4） | Save … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| testercomm.html | （沒有 id ×21） | Send to Handler … | `s:unbound/?+E?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Alert.MotionView9050.html | （沒有 id ×2） | P1 … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Alert.MyMessageBox.NonStop.html | nsOk | 確認 OK | `dead/?` | clicked | — | — |  |  |
| Alert.Note.NonStop.html | nsOk | 確認 OK | `dead/?` | clicked | — | — |  |  |
| Alert.Password.html | btnOK | OK | `dead/?` | clicked | — | — |  |  |
| Alert.Password.html | spbCancel | Cancel | `dead/?` | clicked | — | — |  |  |
| Config.Configuration.html | sbExit | Exit | `dead/?` | clicked | — | — |  |  |
| Config.DIOInterFaceCFG.html | spbDelete | Delete | `dead/?` | clicked | — | — |  |  |
| Config.DIOInterFaceCFG.html | spbExit | Exit | `dead/?` | clicked | — | — |  |  |
| Config.DIOInterFaceCFG.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Data.CounterClear.html | spbExit | Exit | `dead/?` | clicked | — | — |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Exit | Exit | `dead/?` | clicked | — | — |  |  |
| Data.StartCondition.html | sbClearCount | Clear | `dead/?` | clicked | — | — |  |  |
| Data.StartCondition.html | sbSameAsHead1 | Same As Head 1 | `dead/?` | clicked | — | — |  |  |
| Data.StartCondition.html | sbSave | Save | `dead/?` | clicked | — | — |  |  |
| Data.StartCondition.html | spbExit | Exit | `dead/?` | clicked | — | — |  |  |
| HW.HandlerSys.html | ExitBtn | Close | `dead/?` | clicked | — | — |  |  |
| HW.HandlerSys.html | SaveBtn | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| HW.MyCCLinkSensor.html | sbExit | Exit | `dead/?` | clicked | — | — |  |  |
| HW.ShuttleMove.html | sbUpdate | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| HW.ShuttleMove.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| HW.VacuumUnit.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| HW.VacuumUnit.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Main.AOAInfo.html | OffsetSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.AGV.html | btInitalLoad | Inital Load | `dead/?` | clicked | form.event=yes | — |  |  |
| Setup.AGV.html | btInitalUnLoad | Inital Unload | `dead/?` | clicked | form.event=yes | — |  |  |
| Setup.AGV.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.AGV.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BarCode.html | bt2DIDOffset | 2DID Offset | `dead/?` | clicked | — | — |  |  |
| Setup.BarCode.html | btStart2DIDCheckSh1 | Cheack 2DID SH1 | `dead/?` | clicked | — | — |  |  |
| Setup.BarCode.html | btStart2DIDCheckSh2 | Cheack 2DID SH2 | `dead/?` | clicked | — | — |  |  |
| Setup.BarCode.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.BarCode.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BinSel.html | btnAutoHide | 自動隱藏停用列 | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BinSel.html | btnSetAll2NotUse | Set all to not use | `dead/?` | clicked ; dom:2 | — | — |  |  |
| Setup.BinSel.html | btnSettingSpecificBin | Specific Bin | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BinSel.html | sbtExit | ✕Exit | `dead/?` | clicked | — | — |  |  |
| Setup.BinSel.html | spbAOIBin | AOI Bin | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BinSel.html | spbNormal | Normal | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BinSel.html | spbPrime | Prime | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BinSel.html | spbSave | 💾Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.Cleaning.html | btFocusOnly | Only Focus | `dead/?` | clicked | — | — |  |  |
| Setup.Cleaning.html | btnResetCleanCount | Reset | `dead/?` | clicked | form.event=yes | — |  |  |
| Setup.Cleaning.html | btnStartAutoClean | Clean | `dead/?` | clicked | form.event=yes | — |  |  |
| Setup.Cleaning.html | sbCleanExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.Cleaning.html | sbCleanSave | Save | `dead/?` | clicked | — | — |  |  |
| Setup.Cleaning.html | sbTrayAssign | Tray Assign | `dead/?` | clicked | form.event=yes | — |  |  |
| Setup.Contact.html | btnTempSet | Temperature Setting | `dead/?` | clicked | — | — |  |  |
| Setup.Contact.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.Contact.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.ContactForce.html | btExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.ContactForce.html | btSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.HotPlate.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.Ld_ULd.html | btnDefaultValue | Default Value | `dead/?` | clicked | — | — |  |  |
| Setup.Ld_ULd.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.Ld_ULd.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.OffSet.html | btnOffsetList | To Offset List | `dead/?` | clicked ; dom:2 | — | — |  |  |
| Setup.OffSet.html | sbFix5 |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbFix6 |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbInSh1LB_AutoClean |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbInSh1RA_AutoClean |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbInSh1RB_AutoClean |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbInSh1_AutoClean |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbInSh2LB_AutoClean |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbInSh2RA_AutoClean |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbInSh2RB_AutoClean |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbInSh2_AutoClean |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbScanAOI |  | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbZcalibration | Go To In/Out Arm Z Calib | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.OffSet.html | spbSave | Save | `dead/?` | clicked ; dom:2 | — | — |  |  |
| Setup.QAMode.html | btnApply | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.QAMode.html | btnOk | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.SCK_ART.html | btnApplyCount | Apply Count | `dead/?` | clicked | — | — |  |  |
| Setup.SCK_ART.html | btnApplyLotInfo | Apply Lot | `dead/?` | clicked | — | — |  |  |
| Setup.SCK_ART.html | btnApplyQty | Apply QTY | `dead/?` | clicked | — | — |  |  |
| Setup.SCK_ART.html | btnApplySetting | Apply Setting | `dead/?` | clicked | — | — |  |  |
| Setup.SCK_ART.html | btnExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.SCK_ART.html | btnExit1 | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.SetUp.html | btAutoShuttlePitch | Auto Shuttle Pitch | `dead/?` | clicked | — | — |  |  |
| Setup.SetUp.html | btnLDownToRUpZ |  | `dead/?` | clicked | — | — |  |  |
| Setup.SetUp.html | btnLUpToRDownN |  | `dead/?` | clicked | — | — |  |  |
| Setup.SetUp.html | btnLUpToRDownZ |  | `dead/?` | clicked | — | — |  |  |
| Setup.SetUp.html | btnRDownToLUpZ |  | `dead/?` | clicked | — | — |  |  |
| Setup.SetUp.html | btnRUpToLDownN |  | `dead/?` | clicked | — | — |  |  |
| Setup.SetUp.html | btnRUpToLDownZ |  | `dead/?` | clicked | — | — |  |  |
| Setup.SetUp.html | sbUpdate | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.SetUp.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.Speed.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.Speed.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.Speed.html | spbSelectAll | Select All | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.Speed.html | spbSetToDef | Set to define | `dead/?` | clicked | — | — |  |  |
| Setup.Speed.html | spbSpeedAdd | Speed + | `dead/?` | clicked | — | — |  |  |
| Setup.Speed.html | spbSpeedDec | Speed - | `dead/?` | clicked | — | — |  |  |
| Setup.Temp_Set.html | btClearAll | Clear All | `dead/?` | clicked | — | — |  |  |
| Setup.Temp_Set.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.Temp_Set.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.TesterIF.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.TesterIF.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.TrayAssignment.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.TrayAssignment.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.TrayForm.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Setup.TrayForm.html | spbCopy | Copy From | `dead/?` | clicked | — | — |  |  |
| Setup.TrayForm.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.YieldMonitoring.html | btnApply | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.YieldMonitoring.html | btnOk | Exit | `dead/?` | clicked | — | — |  |  |
| Status.CounterSel.html | spbExit | Exit | `dead/?` | clicked | — | — |  |  |
| Status.GroundMan.html | sbtExit | Exit | `dead/?` | clicked | — | — |  |  |
| Status.GroundMan.html | spbSave | Save | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Status.ShowBinSelect.html | btReturn | return | `dead/?` | clicked | — | — |  |  |
| Status.TemperFrom.html | tzExpand | Expand all ▸ | `dead/?` | clicked ; dom:2 | — | — |  |  |
| Status.TowerLight.html | spbExit | Exit | `dead/?` | clicked | — | — |  |  |
| Data.Builder.html | spbExit | Exit | `OK-cmd` | clicked ; control.acquire ; builder.op | — | — |  |  |
| Data.ContactCT.html | btClearCount | Count Clear | `OK-cmd` | clicked ; control.acquire ; act.contactCT.clearCount ; contactct.get ; dom:8 | — | — |  |  |
| Data.ContactCT.html | btYieldChart | Yield Chart | `OK-cmd` | clicked ; control.acquire ; act.contactCT.yieldChart ; contactct.get ; dom:8 | — | — |  |  |
| Data.CounterClear.html | spbExe | Execute | `OK-cmd` | clicked ; counterclear.exe ; dom:6 | counterclear.get=yes counterclear.click=yes | — |  |  |
| Data.Observer.html | btExit | Exit | `OK-cmd` | clicked ; control.acquire ; act.observer.exit ; observer.get ; dom:7 | — | — |  |  |
| Data.Observer.html | btnClearTime | Clear Time Data | `OK-cmd` | clicked ; control.acquire ; act.observer.clearTime ; observer.get ; dom:7 | — | — |  |  |
| Data.SortCT.html | btnClearCount | 🗒 Clear Count | `OK-cmd` | clicked ; control.acquire ; act.sortCT.clearCount ; dom:3 | act.trayEdit=yes | — |  |  |
| HW.HandlerSys.html | LoadBtn | Load | `OK-cmd` | clicked ; control.acquire ; dom:4 | — | — |  |  |
| HW.MotorTest.html | btResetMNet | Reset MNet | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | — | — |  |  |
| HW.MotorTest.html | btnGo | Go | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:932 | — | — |  |  |
| HW.MotorTest.html | btnGoSoftN | Go Soft N Pos | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | — | — |  |  |
| HW.MotorTest.html | btnGoSoftP | Go Soft P Pos | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | — | — |  |  |
| HW.MotorTest.html | btnHighSpeed | Jog High | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:114 | — | — |  |  |
| HW.MotorTest.html | btnHome | Home Reset | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | — | — |  |  |
| HW.MotorTest.html | btnHomeLow | Home Low | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:112 | — | — |  |  |
| HW.MotorTest.html | btnMotorPower | Motor Power | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:160 | — | — |  |  |
| HW.MotorTest.html | btnReloadMotorData | Reload Motor Data | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET /api/struct/motor/config ; http:GET / | — | — |  |  |
| HW.MotorTest.html | btnServoOff | Servo Off | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:158 | — | — |  |  |
| HW.MotorTest.html | btnSetPosN | Set Position 2 | `OK-cmd` | clicked ; control.acquire ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline. | — | — |  |  |
| HW.MotorTest.html | btnSetRange | Test Range | `OK-cmd` | clicked ; control.acquire ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline. | — | — |  |  |
| HW.MotorTest.html | btnStop | Stop | `OK-cmd` | clicked ; motor.stop ; dom:114 | — | — |  |  |
| HW.MotorTest.html | palExit | Exit | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:112 | — | — |  |  |
| HW.MotorTest.html | sbMotorTest_JogN | - | `OK-cmd` | clicked ; control.takeover ; motor.access ; motor.stop ; dom:963 | — | — |  |  |
| HW.MotorTest.html | sbMotorTest_JogP | + | `OK-cmd` | clicked ; control.takeover ; motor.access ; motor.stop ; dom:965 | — | — |  |  |
| HW.MotorTest.html | sbMotorTest_MoveN | - | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | — | — |  |  |
| HW.MotorTest.html | sbMotorTest_MoveP | + | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | — | — |  |  |
| HW.home.html | sbAbortHome | Abort Home | `OK-cmd` | clicked ; act.home.abort ; dom:6 | act.home.abort=yes | — |  |  |
| HW.teach.html | MotorInArmPitchX | X Pitch | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:47 | — | — |  |  |
| HW.teach.html | MotorInArmX | X | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:68 | — | — |  |  |
| HW.teach.html | MotorInArmZA | ZA | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZA) ; dom:27 | — | — |  |  |
| HW.teach.html | MotorInArmZAe | Z Ae | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZAe) ; dom:29 | — | — |  |  |
| HW.teach.html | MotorInArmZAh | Z Ah | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | — |  |  |
| HW.teach.html | MotorInArmZB | ZB | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZB) ; dom:27 | — | — |  |  |
| HW.teach.html | MotorInArmZD | ZD | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZD) ; dom:48 | — | — |  |  |
| HW.teach.html | MotorInArmZF | ZF | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | — |  |  |
| HW.teach.html | MotorInArmZG | ZG | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.acquire ; motor.access(MInArmZG) ; dom: | — | — |  |  |
| HW.teach.html | MotorInSh2 | X | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.acquire ; motor.access(MInShuttle2) ; d | — | — |  |  |
| HW.teach.html | MotorIndexArm1Y | Y | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:68 | — | — |  |  |
| HW.teach.html | MotorIndexArm1Z | Z | `OK-cmd` | clicked ; control.acquire ; motor.access(MTestZ1) ; dom:47 | — | — |  |  |
| HW.teach.html | MotorOutArmX | X | `OK-cmd` | clicked ; control.acquire ; motor.access(MOutArmX) ; dom:26 | — | — |  |  |
| HW.teach.html | MotorOutArmY | Y | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:68 | — | — |  |  |
| HW.teach.html | MotorOutArmZB | ZB | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | — |  |  |
| HW.teach.html | MotorOutArmZH | ZH | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | — |  |  |
| HW.teach.html | btnAlarmReset | Alarm Reset | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:48 | — | — |  |  |
| HW.teach.html | btnHome | HOME | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | — |  |  |
| HW.teach.html | btnInZAllUp | In Z All Up | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZH) ; dom:5890 | — | — |  |  |
| HW.teach.html | btnJogN | JOG N | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | — |  |  |
| HW.teach.html | btnJogP | JOG P | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; motor.stop(MInArmX) ; dom:5885 | — | — |  |  |
| HW.teach.html | btnLoaderY | Loader Y | `OK-cmd` | clicked ; control.acquire ; motor.access(MLoaderY) ; dom:27 | — | — |  |  |
| HW.teach.html | btnMotorTest | Motor Tools | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:21 | — | — |  |  |
| HW.teach.html | btnMoveN | Move - | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:5888 | — | — |  |  |
| HW.teach.html | btnMoveP | Move + | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | — |  |  |
| HW.teach.html | btnMoveTo | Move | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:5865 | — | — |  |  |
| HW.teach.html | btnOutZAllUp | Out Z All Up | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchX4) ; dom:5867 | — | — |  |  |
| HW.teach.html | btnSave | SAVE | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:44 | — | — |  |  |
| HW.teach.html | btnServo | Servo | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | — |  |  |
| HW.teach.html | btnSetTo | SET TO | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:24 | — | — |  |  |
| HW.teach.html | btnStop | STOP | `OK-cmd` | clicked ; motor.stop(MInArmX) ; dom:29 | — | — |  |  |
| HW.teach.html | pnlExit | EXIT | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:42 | — | — |  |  |
| Main.CommView.html | btnReadZ1 | Read Z1 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:4 | — | — |  |  |
| Main.CommView.html | btnReadZ2 | Read Z2 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | — |  |  |
| Main.CommView.html | btnSetZ1 | Set Z1 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | — |  |  |
| Main.CommView.html | btnSetZ2 | Set Z2 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | — |  |  |
| Main.Record.html | spbClearRecord | CLEAR | `OK-cmd` | clicked ; control.acquire ; act.main.clearRecord ; dom:3 | act.main.clearRecord=yes act.main.meShuttle2Dbl=yes | — |  |  |
| Main.gbControlBtn.html | sbStateRecord | State Record | `OK-cmd` | clicked ; control.takeover ; act.main.stateRecord ; dom:6 | control.takeover=yes act.main.stateRecord=yes | — |  |  |
| Setup.HotPlate.html | spbSave | Save | `OK-cmd` | clicked ; control.acquire ; dom:4 ; err:rejection c12 probe stub | — | — |  |  |
| Status.Security.html | SecurityExit | Exit | `OK-cmd` | clicked ; control.acquire ; dom:4 | — | — |  |  |
| eventlog.html | q | Query | `OK-http` | clicked ; http:POST /api/ela/query ; http:GET /api/ela ; dom:4 | — | — |  |  |
| eventlog.html | saveSum | Save Summary | `OK-http` | clicked ; http:POST /api/ela/summary ; http:GET /api/ela ; dom:4 | — | — |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Setup | Setup | `OK-ui` | clicked ; dom:5 | — | — |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Summary | Summary | `OK-ui` | clicked ; dom:5 | — | — |  |  |
| HW.MotorTest.html | BitBtn1 | Copy From | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | — | — |  |  |
| HW.MotorTest.html | btnAlarmReset | Alarm Reset | `OK-ui` | clicked ; dom:114 | — | — |  |  |
| HW.MotorTest.html | btnHomeHigh | Home High | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | — | — |  |  |
| HW.MotorTest.html | btnLoopMove | Loop Move | `OK-ui` | clicked ; dom:111 | — | — |  |  |
| HW.MotorTest.html | btnLowSpeed | Jog Low | `OK-ui` | clicked ; dom:112 | — | — |  |  |
| HW.MotorTest.html | btnRange | Range | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | — | — |  |  |
| HW.MotorTest.html | btnSetPosP | Set Position 1 | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | — | — |  |  |
| HW.MotorTest.html | btnSoftNPos | Soft Neg | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | — | — |  |  |
| HW.MotorTest.html | btnSoftPPos | Soft Pos | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | — | — |  |  |
| HW.teach.html | MotorAuto1YCW | Auto 1 CW | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorAuto2YCW | Auto 2 CW | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorInArmPitchX2 | X Pitch 2 | `OK-ui` | clicked ; dom:48 | — | — |  |  |
| HW.teach.html | MotorInArmPitchX3 | X Pitch 3 | `OK-ui` | clicked ; dom:48 | — | — |  |  |
| HW.teach.html | MotorInArmPitchX4 | X Pitch 4 | `OK-ui` | clicked ; dom:48 | — | — |  |  |
| HW.teach.html | MotorInArmY | Y | `OK-ui` | clicked ; dom:26 | — | — |  |  |
| HW.teach.html | MotorInArmZAf | Z Af | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorInArmZAg | Z Ag | `OK-ui` | clicked ; dom:48 | — | — |  |  |
| HW.teach.html | MotorInArmZC | ZC | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorInArmZE | ZE | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorInArmZH | ZH | `OK-ui` | clicked ; dom:48 | — | — |  |  |
| HW.teach.html | MotorInSh1 | X | `OK-ui` | clicked ; dom:47 | — | — |  |  |
| HW.teach.html | MotorIndexArm2Y | Y | `OK-ui` | clicked ; dom:26 | — | — |  |  |
| HW.teach.html | MotorIndexArm2Z | Z | `OK-ui` | clicked ; dom:26 | — | — |  |  |
| HW.teach.html | MotorOutArmPitchX | X Pitch | `OK-ui` | clicked ; dom:47 | — | — |  |  |
| HW.teach.html | MotorOutArmPitchX2 | X Pitch 2 | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorOutArmPitchX3 | X Pitch 3 | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorOutArmPitchX4 | X Pitch 4 | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorOutArmZA | ZA | `OK-ui` | clicked ; dom:26 | — | — |  |  |
| HW.teach.html | MotorOutArmZC | ZC | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorOutArmZD | ZD | `OK-ui` | clicked ; dom:48 | — | — |  |  |
| HW.teach.html | MotorOutArmZE | ZE | `OK-ui` | clicked ; dom:48 | — | — |  |  |
| HW.teach.html | MotorOutArmZF | ZF | `OK-ui` | clicked ; dom:48 | — | — |  |  |
| HW.teach.html | MotorOutArmZG | ZG | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorOutSh1 | X | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorOutSh2 | X | `OK-ui` | clicked ; dom:27 | — | — |  |  |
| HW.teach.html | MotorTrayX | X | `OK-ui` | clicked ; dom:26 | — | — |  |  |
| HW.teach.html | btnSetToOffset | SET TO | `OK-ui` | clicked ; dom:22 | — | — |  |  |
| Main.MotionView.html | axisToggle | 顯示未啟用軸 | `OK-ui` | clicked ; dom:7 | — | — |  |  |
| Setup.OffSet.html | btnToIndexOffset | Go To Index & Tray Arm O | `OK-ui` | clicked ; dom:10 | — | — |  |  |
| Status.LtcSensor.html | btnClose | Exit | `OK-ui` | clicked ; dom:63 | — | — |  |  |
| eventlog.html | jamSetting | Jam Code Setting | `OK-ui` | clicked ; http:GET /api/ela ; frame:open Status.Security.html ; dom:4 | — | — |  |  |

## 4. 抽 10 顆給筆電複驗

| # | 頁 | id | 分類 | 怎麼複驗 |
|---|---|---|---|---|
| 1 | Alert.MyMessageBox.NonStop.html | nsOk | `dead/?` | 開這一頁按 `nsOk`，看 wb_serve oplog 有沒有收到命令；golden — |
| 2 | Alert.Note.NonStop.html | nsOk | `dead/?` | 開這一頁按 `nsOk`，看 wb_serve oplog 有沒有收到命令；golden — |
| 3 | Alert.Password.html | btnOK | `dead/?` | 開這一頁按 `btnOK`，看 wb_serve oplog 有沒有收到命令；golden — |
| 4 | Config.Configuration.html | sbExit | `dead/?` | 開這一頁按 `sbExit`，看 wb_serve oplog 有沒有收到命令；golden — |
| 5 | Config.DIOInterFaceCFG.html | spbDelete | `dead/?` | 開這一頁按 `spbDelete`，看 wb_serve oplog 有沒有收到命令；golden — |
| 6 | Data.CounterClear.html | spbExit | `dead/?` | 開這一頁按 `spbExit`，看 wb_serve oplog 有沒有收到命令；golden — |
| 7 | Data.SmartDiagnostic.html | sb_SmartDiagnostic_Exit | `dead/?` | 開這一頁按 `sb_SmartDiagnostic_Exit`，看 wb_serve oplog 有沒有收到命令；golden — |
| 8 | Data.StartCondition.html | sbClearCount | `dead/?` | 開這一頁按 `sbClearCount`，看 wb_serve oplog 有沒有收到命令；golden — |
| 9 | HW.HandlerSys.html | ExitBtn | `dead/?` | 開這一頁按 `ExitBtn`，看 wb_serve oplog 有沒有收到命令；golden — |
| 10 | HW.MyCCLinkSensor.html | sbExit | `dead/?` | 開這一頁按 `sbExit`，看 wb_serve oplog 有沒有收到命令；golden — |

## 5. 點擊實測（請 St01 代跑；STEVEN-NB3 只編譯、不執行）

```
cd <repo>/HT9011UC_Cpp_V3.33.906.0/tools/webprobe
python c12_button_census.py --out-tsv c12_static.tsv
python c12_click_probe.py --census c12_static.tsv --out c12_click.tsv
python c12_button_census.py --merge c12_click.tsv --out-tsv <repo>/docs/handoff/ST02_BUTTON_CENSUS_20261002.tsv --out-md <repo>/docs/handoff/ST02_BUTTON_CENSUS_20261002.md
```

- 探針只連自己的假伺服器：頁面腳本跑之前先把**所有** WebSocket 網址（有頁寫死 `ws://127.0.0.1:9045/...`）和 fetch／XHR 網址改到假伺服器的埠；假伺服器每個命令都回 ok:false、`/api/*` 回 404。碰不到 wb_serve、碰不到機台、不讀寫機台檔案。需要 Edge（不是 ctest）。
- 每頁單獨開（不在外框裡）；有分頁的先點那一頁的頁籤；confirm／prompt 一律回「確定」，才看得到確認之後送的命令。
- 已知限制：①C++ 資料到了才綁的鈕（例 Offset 部位鈕）在假伺服器下看起來沒反應，會落在 dead/；②FormShow bridge（Teach／IO）藏的元件在假伺服器下不會藏；③不在外框裡，`HT9045Link` 的 hub 路徑沒走到。這三種在 dead/ 裡要人看。

