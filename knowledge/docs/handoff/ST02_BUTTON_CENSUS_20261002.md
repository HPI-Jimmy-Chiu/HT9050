# ST02-C12 按鈕普查：「看得到、按了沒反應」（靜態＋點擊實測）

> **golden 基準：`D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618`**（RULINGS_20261003 第 2 條：0618 為準，906_0625_Steven 只對照；對照結果見文末）。
> 量測樹：靜態在 main `e0e6b70d`（St02-E 1004 在 STEVEN-NB3 重跑，golden 0618）；點擊實測是 St01 1004 19:3x 在 main `7238673d` 量的（兩者程式相同，`e0e6b70d` 只多了本文件），從 St01 合併版 tsv 的 sent_probe 欄還原後重新合併。只量、只分類，**沒有改任何程式**（TO_STEVEN.md §3 ST02-C12）。
> 產生：`HT9011UC_Cpp_V3.33.906.0/tools/webprobe/c12_button_census.py`（靜態）＋ `c12_click_probe.py`（無頭 Edge＋假伺服器，**已合併實測結果**）。每顆的完整欄位在同名 `.tsv`。

## 0. 白話摘要

- 範圍：外框 `web/background.html` WINDOWS 表開得到的頁＋Alert 覆蓋頁，共 64 頁、1859 顆按鈕（golden 的 TButton／TBitBtn／TSpeedButton，加上網頁自己的 `<button>`）。
- 看得到又沒有變灰的：**430 顆**（其餘：靜態藏起來 1312、變灰 105、開發用頁 12）。
- 實測按了**什麼都沒發生**（沒送 WS、沒有 POST、沒有視窗動作、畫面沒變）：**110 顆**，依 golden／移植樹分成 A～E（§1）。

## 1. 分類與建議誰做

| 分類 | 意思 | 顆數 | 建議誰做 |
|---|---|---:|---|
| `dead/D` | 實測沒反應，golden 處理器本體會動馬達／寫輸出 | 5 | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| `dead/B` | 實測沒反應，C++ 沒有處理器 | 11 | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| `dead/A` | 實測沒反應，C++ 有處理器（form.event 表列或同名函式） | 74 | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| `dead/A+E?` | 實測沒反應，C++ 有處理器（form.event 表列或同名函式）；WORKLOG_MACHINE §4 有提到（id 或頁） | 1 | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送；機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| `dead/C0` | 實測沒反應，golden 沒有 OnClick 或處理器是空的 | 14 | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| `dead/?` | 實測沒反應，不是 golden 元件、也沒有處理器線索 | 5 |  |
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
| Main.html | main.dfm | 2 | 1 | 0 | 1 | s:unbound/?+E? 1 |
| Data.SortCT.html | cSortCT.dfm | 1 | 0 | 0 | 1 | OK-cmd 1 |
| Data.ContactCT.html | cContactCT.dfm | 2 | 0 | 0 | 2 | OK-cmd 2 |
| Data.LotInfo.html | uLotInfo.dfm | 15 | 13 | 2 | 0 |  |
| Status.TemperFrom.html | — | 2 | 1 | 0 | 1 | dead/? 1 |
| Data.Observer.html | cObserver.dfm | 35 | 33 | 0 | 2 | OK-cmd 2 |
| Status.ShowMessage.html | uShowMessage.dfm | 1 | 1 | 0 | 0 |  |
| Status.ShowBinSelect.html | cShowBinSelect.dfm | 4 | 3 | 0 | 1 | dead/A 1 |
| Setup.OffSet.html | cOffSet.dfm | 89 | 73 | 0 | 16 | dead/C0 12, dead/A 2, OK-ui 1, dead/D 1 |
| Setup.Speed.html | cSpeed.dfm | 6 | 0 | 0 | 6 | dead/A 6 |
| HW.IoSetView.html | iosetview.dfm | 17 | 17 | 0 | 0 |  |
| Config.Configuration.html | cConfiguration.dfm | 49 | 48 | 0 | 1 | dead/B 1 |
| Status.CounterSel.html | cCounterSel.dfm | 1 | 0 | 0 | 1 | dead/A 1 |
| Data.CounterClear.html | cCounterClear.dfm | 2 | 0 | 0 | 2 | dead/A 1, OK-cmd 1 |
| Data.Builder.html | cBuilder.dfm | 5 | 0 | 4 | 1 | OK-cmd 1 |
| Config.DIOInterFaceCFG.html | DIOInterFaceCFG.dfm | 4 | 0 | 1 | 3 | dead/A 3 |
| Status.LtcSensor.html | LtcSensor.dfm | 16 | 1 | 14 | 1 | OK-ui 1 |
| Status.TowerLight.html | cTowerLight.dfm | 1 | 0 | 0 | 1 | dead/A 1 |
| HW.OmronEJ1N.html | EJ1N/OmronEJ1N.dfm | 15 | 11 | 4 | 0 |  |
| Setup.QAMode.html | QAMode.dfm | 2 | 0 | 0 | 2 | dead/A 2 |
| Setup.BarCode.html | BarCode/BarCode.dfm | 87 | 82 | 0 | 5 | dead/B 3, dead/A 2 |
| HW.MyCCLinkSensor.html | CCLink/MyCCLinkSensor.dfm | 80 | 54 | 25 | 1 | dead/B 1 |
| Setup.Cleaning.html | AutoClean/uCleaning.dfm | 8 | 2 | 0 | 6 | dead/A 5, dead/C0 1 |
| Setup.Contact.html | cContact.dfm | 20 | 15 | 2 | 3 | dead/A 2, dead/D 1 |
| Setup.TesterIF.html | cTesterIF.dfm | 3 | 1 | 0 | 2 | dead/A 2 |
| Status.GroundMan.html | GroundMan/GroundMan.dfm | 5 | 0 | 3 | 2 | dead/A 2 |
| Setup.Ld_ULd.html | cLd_ULd.dfm | 3 | 0 | 0 | 3 | dead/A 3 |
| Status.Security.html | cSecurity.dfm | 187 | 6 | 0 | 181 | s:unbound/? 180, OK-cmd 1 |
| Setup.TrayForm.html | cTrayForm.dfm | 4 | 1 | 0 | 3 | dead/A 2, dead/B 1 |
| Setup.SCK_ART.html | Automation/SCK_ART.dfm | 8 | 2 | 0 | 6 | dead/B 4, dead/A 2 |
| Setup.YieldMonitoring.html | uYieldMonitoring.dfm | 3 | 1 | 0 | 2 | dead/A 2 |
| Setup.HotPlate.html | cHotPlate.dfm | 2 | 0 | 0 | 2 | OK-cmd 1, dead/A 1 |
| Setup.SetUp.html | cSetUp.dfm | 9 | 0 | 0 | 9 | dead/A 7, dead/B 1, dead/D 1 |
| Data.SmartDiagnostic.html | SmartDiagnostic.dfm | 6 | 1 | 2 | 3 | OK-ui 2, dead/C0 1 |
| Data.StartCondition.html | cStartCondition.dfm | 88 | 84 | 0 | 4 | dead/A 4 |
| Setup.Temp_Set.html | uTemp_Set.dfm | 10 | 7 | 0 | 3 | dead/A 3 |
| Setup.BinSel.html | cBinSel.dfm | 8 | 0 | 0 | 8 | dead/A 6, dead/? 2 |
| HW.teach.html | uteach.dfm | 865 | 782 | 23 | 60 | OK-cmd 32, OK-ui 28 |
| HW.MotorTest.html | uMotorTest.dfm | 52 | 24 | 1 | 27 | OK-cmd 18, OK-ui 9 |
| HW.home.html | uhome.dfm | 2 | 1 | 0 | 1 | OK-cmd 1 |
| HW.ShuttleMove.html | ShuttleMove.dfm | 26 | 7 | 17 | 2 | dead/A 2 |
| Setup.TrayAssignment.html | cTrayAssignment.dfm | 2 | 0 | 0 | 2 | dead/A 2 |
| Setup.ContactForce.html | ContactForce.dfm | 3 | 1 | 0 | 2 | dead/D 2 |
| HW.VacuumUnit.html | VacuumUnit/VacuumUnit.dfm | 7 | 1 | 4 | 2 | dead/A 2 |
| Setup.AGV.html | Automation/AGV.dfm | 4 | 0 | 0 | 4 | dead/A 4 |
| HW.HandlerSys.html | HandlerSys.dfm | 5 | 2 | 0 | 3 | dead/A 2, OK-cmd 1 |
| Main.gbControlBtn.html | main.dfm | 1 | 0 | 0 | 1 | OK-cmd 1 |
| Main.MotionView.html | — | 1 | 0 | 0 | 1 | OK-ui 1 |
| Main.CommView.html | main.dfm | 9 | 5 | 0 | 4 | OK-cmd 4 |
| Main.Record.html | main.dfm | 2 | 1 | 0 | 1 | OK-cmd 1 |
| Main.AOAInfo.html | main.dfm | 1 | 0 | 0 | 1 | dead/A+E? 1 |
| eventlog.html | — | 8 | 1 | 0 | 7 | s:unbound/? 4, OK-http 2, OK-ui 1 |
| testercomm.html | — | 23 | 2 | 0 | 21 | s:unbound/?+E? 21 |
| IDE.StyleGuide.html | — | 2 | 0 | 0 | 0 |  |
| IDE.I18nEditor.html | — | 4 | 0 | 0 | 0 |  |
| IDE.WidgetTemplates.html | — | 6 | 0 | 0 | 0 |  |
| HW.TrayEdit.html | uTrayEditForm.dfm | 4 | 1 | 3 | 0 |  |
| Data.FTPClient.html | KYECFTP/FTPClient.dfm | 8 | 8 | 0 | 0 |  |
| Alert.MotionView.html | — | 1 | 1 | 0 | 0 |  |
| Alert.MotionView9050.html | — | 5 | 3 | 0 | 2 | s:unbound/? 2 |
| Alert.MyMessageBox.NonStop.html | — | 1 | 0 | 0 | 1 | dead/? 1 |
| Alert.Note.NonStop.html | — | 1 | 0 | 0 | 1 | dead/? 1 |
| Alert.Note.html | note.dfm | 8 | 8 | 0 | 0 |  |
| Alert.Password.html | Password.dfm | 8 | 6 | 0 | 2 | dead/A 2 |

## 3. 每顆一列（看得到又沒有變灰的 430 顆；藏起來／變灰的只在 .tsv）

欄位：頁、id、caption、分類、送出的命令（實測）、C++ 認不認得、golden 處理器（golden 906 檔名:行）、會動機台、建議誰做。

| 頁 | id | caption | 分類 | 送出（實測） | C++ | golden 處理器 | 動機台 | 建議誰做 |
|---|---|---|---|---|---|---|---|---|
| Main.html | （沒有 id ×1） | 🔄 Set … | `s:unbound/?+E?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Status.Security.html | （沒有 id ×180） | 🔑[00] Main - Tools … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| eventlog.html | （沒有 id ×4） | Save … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| testercomm.html | （沒有 id ×21） | Send to Handler … | `s:unbound/?+E?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Alert.MotionView9050.html | （沒有 id ×2） | P1 … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Setup.Contact.html | spbSave | Save | `dead/D` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cContact.cpp:14072-14201 (71 句) | ADAM_WriteVoltage | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Setup.ContactForce.html | btExit | Exit | `dead/D` | clicked | DynamicTemp.cpp forms/fDynamicTemp.h | btExitClick ContactForce.cpp:929-933 (2 句) | ADAM_WriteVoltage | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Setup.ContactForce.html | btSave | Save | `dead/D` | clicked ; dom:1 | ATC/ATCInterface.cpp ATC/ATCInterface.h | btSaveClick ContactForce.cpp:935-961 (19 句) | ADAM_WriteVoltage | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Setup.OffSet.html | spbSave | Save | `dead/D` | clicked ; dom:2 | Automation/auto9045.cpp Command.cpp | spbSaveClick cOffSet.cpp:2803-2879 (47 句) | fMain->Pause | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Setup.SetUp.html | sbUpdate | Save | `dead/D` | clicked ; dom:1 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick cSetUp.cpp:3468-3626 (77 句) | ADAM_WriteVoltage | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Config.Configuration.html | sbExit | Exit | `dead/B` | clicked | — | sbExitClick cConfiguration.cpp:6277-6284 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| HW.MyCCLinkSensor.html | sbExit | Exit | `dead/B` | clicked | — | sbExitClick CCLink/MyCCLinkSensor.cpp:491-521 (13 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | bt2DIDOffset | 2DID Offset | `dead/B` | clicked | — | bt2DIDOffsetClick BarCode/BarCode.cpp:6799-6804 (3 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btStart2DIDCheckSh1 | Cheack 2DID SH1 | `dead/B` | clicked | — | btStart2DIDCheckSh1Click BarCode/BarCode.cpp:6806-6810 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btStart2DIDCheckSh2 | Cheack 2DID SH2 | `dead/B` | clicked | — | btStart2DIDCheckSh2Click BarCode/BarCode.cpp:6812-6816 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyCount | Apply Count | `dead/B` | clicked | — | btnApplyCountClick Automation/SCK_ART.cpp:723-798 (30 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyLotInfo | Apply Lot | `dead/B` | clicked | — | btnApplyLotInfoClick Automation/SCK_ART.cpp:1596-1606 (6 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyQty | Apply QTY | `dead/B` | clicked | — | btnApplyQtyClick Automation/SCK_ART.cpp:1521-1530 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplySetting | Apply Setting | `dead/B` | clicked | — | btnApplySettingClick Automation/SCK_ART.cpp:1532-1594 (40 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SetUp.html | btAutoShuttlePitch | Auto Shuttle Pitch | `dead/B` | clicked | — | btAutoShuttlePitchClick cSetUp.cpp:4732-4736 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.TrayForm.html | spbCopy | Copy From | `dead/B` | clicked | — | spbCopyClick cTrayForm.cpp:692-713 (17 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Alert.Password.html | btnOK | OK | `dead/A` | clicked | forms/fPassword.cpp forms/fPassword.h | btnOKClick Password.cpp:385-388 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | spbCancel | Cancel | `dead/A` | clicked | forms/fPassword.cpp forms/fPassword.h | spbCancelClick Password.cpp:123-128 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.DIOInterFaceCFG.html | spbDelete | Delete | `dead/A` | clicked | FileRW/TTLCfg.cpp FileRW/TTLCfg.gen.inc | spbDeleteClick DIOInterFaceCFG.cpp:248-256 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.DIOInterFaceCFG.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick DIOInterFaceCFG.cpp:258-261 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.DIOInterFaceCFG.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick DIOInterFaceCFG.cpp:191-235 (28 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.CounterClear.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cCounterClear.cpp:452-456 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | sbClearCount | Clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbClearCountClick cStartCondition.cpp:607-625 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | sbSameAsHead1 | Same As Head 1 | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbSameAsHead1Click cStartCondition.cpp:590-605 (10 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | sbSave | Save | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbSaveClick cStartCondition.cpp:627-658 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cStartCondition.cpp:670-689 (16 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.HandlerSys.html | ExitBtn | Close | `dead/A` | clicked | FileRW/HSys.cpp FileRW/HSys.gen.inc | ExitBtnClick HandlerSys.cpp:1037-1043 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.HandlerSys.html | SaveBtn | Save | `dead/A` | clicked ; dom:1 | FileRW/HSys.cpp FileRW/HSys.gen.inc | SaveBtnClick HandlerSys.cpp:979-983 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.ShuttleMove.html | sbUpdate | Save | `dead/A` | clicked ; dom:1 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick ShuttleMove.cpp:1965-1994 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.ShuttleMove.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick ShuttleMove.cpp:1956-1961 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.VacuumUnit.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick VacuumUnit/VacuumUnit.cpp:402-406 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.VacuumUnit.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick VacuumUnit/VacuumUnit.cpp:378-400 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Main.AOAInfo.html | OffsetSave | Save | `dead/A+E?` | clicked ; dom:1 | FileRW/AOAOffset.cpp FileRW/AOAOffset.gen.inc | OffsetSaveClick main.cpp:33822-33915 (78 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送；機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| Setup.AGV.html | btInitalLoad | Inital Load | `dead/A` | clicked | form.event=yes | btInitalLoadClick Automation/AGV.cpp:1055-1059 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.AGV.html | btInitalUnLoad | Inital Unload | `dead/A` | clicked | form.event=yes | btInitalUnLoadClick Automation/AGV.cpp:1061-1065 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.AGV.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick Automation/AGV.cpp:1036-1041 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.AGV.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick Automation/AGV.cpp:928-969 (30 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BarCode.html | sbtExit | Exit | `dead/A` | clicked | form.event 表列 | sbtExitClick BarCode/BarCode.cpp:2392-2401 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BarCode.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick BarCode/BarCode.cpp:1254-1457 (129 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | btnSetAll2NotUse | Set all to not use | `dead/A` | clicked ; dom:2 | cBinSel.cpp forms/fBinSel.h | btnSetAll2NotUseClick cBinSel.cpp:6372-6378 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | btnSettingSpecificBin | Specific Bin | `dead/A` | clicked ; dom:1 | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | btnSettingSpecificBinClick cBinSel.cpp:6118-6131 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | sbtExit | ✕Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cBinSel.cpp:2755-2759 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | spbNormal | Normal | `dead/A` | clicked ; dom:1 | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | spbNormalClick cBinSel.cpp:2761-2781 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | spbPrime | Prime | `dead/A` | clicked ; dom:1 | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | spbPrimeClick cBinSel.cpp:2783-2803 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | spbSave | 💾Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cBinSel.cpp:2214-2272 (30 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | btnResetCleanCount | Reset | `dead/A` | clicked | form.event=yes | btnResetCleanCountClick AutoClean/uCleaning.cpp:2095-2117 (15 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | btnStartAutoClean | Clean | `dead/A` | clicked | form.event=yes | btnStartAutoCleanClick AutoClean/uCleaning.cpp:2869-2872 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | sbCleanExit | Exit | `dead/A` | clicked | FileRW/TestIF_File_Cleaning.cpp FileRW/TestIF_File_Cleaning. | sbCleanExitClick AutoClean/uCleaning.cpp:1868-1872 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | sbCleanSave | Save | `dead/A` | clicked | FileRW/TestIF_File_Cleaning.cpp FileRW/TestIF_File_Cleaning. | sbCleanSaveClick AutoClean/uCleaning.cpp:1761-1866 (49 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | sbTrayAssign | Tray Assign | `dead/A` | clicked | form.event=yes | sbTrayAssignClick AutoClean/uCleaning.cpp:2290-2294 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Contact.html | btnTempSet | Temperature Setting | `dead/A` | clicked | forms/fContact.h | btnTempSetClick cContact.cpp:17184-17187 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Contact.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cContact.cpp:14478-14487 (5 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.HotPlate.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cHotPlate.cpp:627-631 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Ld_ULd.html | btnDefaultValue | Default Value | `dead/A` | clicked | forms/fLd_ULd.cpp forms/fLd_ULd.h | btnDefaultValueClick cLd_ULd.cpp:224-237 (8 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Ld_ULd.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cLd_ULd.cpp:218-222 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Ld_ULd.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cLd_ULd.cpp:179-202 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.OffSet.html | btnOffsetList | To Offset List | `dead/A` | clicked ; dom:2 | forms/fOffSet.h | btnOffsetListClick cOffSet.cpp:3369-3374 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.OffSet.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cOffSet.cpp:2797-2801 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.QAMode.html | btnApply | Save | `dead/A` | clicked ; dom:1 | Command.cpp FileRW/TestIF_File_QAMode.cpp | btnApplyClick QAMode.cpp:73-110 (26 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.QAMode.html | btnOk | Exit | `dead/A` | clicked | forms/fQAMode.cpp forms/fQAMode.h | btnOkClick QAMode.cpp:27-30 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SCK_ART.html | btnExit | Exit | `dead/A` | clicked | ATC/ATCInterface.cpp ATC/ATCInterface.h | btnExitClick Automation/SCK_ART.cpp:808-820 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SCK_ART.html | btnExit1 | Exit | `dead/A` | clicked | ATC/ATCInterface.cpp ATC/ATCInterface.h | btnExitClick Automation/SCK_ART.cpp:808-820 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | btnLDownToRUpZ |  | `dead/A` | clicked | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | btnLUpToRDownN |  | `dead/A` | clicked | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | btnLUpToRDownZ |  | `dead/A` | clicked | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | btnRDownToLUpZ |  | `dead/A` | clicked | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | btnRUpToLDownN |  | `dead/A` | clicked | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | btnRUpToLDownZ |  | `dead/A` | clicked | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | sbtExit | Exit | `dead/A` | clicked | form.event 表列 | sbtExitClick cSetUp.cpp:3452-3466 (8 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Speed.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cSpeed.cpp:1788-1793 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Speed.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cSpeed.cpp:1433-1786 (200 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Speed.html | spbSelectAll | Select All | `dead/A` | clicked ; dom:1 | form.event 表列 | spbSelectAllClick cSpeed.cpp:1795-1810 (13 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Speed.html | spbSetToDef | Set to define | `dead/A` | clicked | form.event 表列 | spbSetToDefClick cSpeed.cpp:1812-1933 (63 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Speed.html | spbSpeedAdd | Speed + | `dead/A` | clicked | form.event 表列 | spbSpeedAddClick cSpeed.cpp:1414-1419 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Speed.html | spbSpeedDec | Speed - | `dead/A` | clicked | form.event 表列 | spbSpeedDecClick cSpeed.cpp:1421-1424 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btClearAll | Clear All | `dead/A` | clicked | forms/fTemp_Set.h uTemp_Set.cpp | btClearAllClick uTemp_Set.cpp:5126-5135 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | sbtExit | Exit | `dead/A` | clicked | form.event 表列 | sbtExitClick uTemp_Set.cpp:5101-5124 (15 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick uTemp_Set.cpp:4201-4528 (167 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TesterIF.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTesterIF.cpp:1358-1366 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TesterIF.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTesterIF.cpp:1302-1356 (32 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayAssignment.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTrayAssignment.cpp:1544-1548 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayAssignment.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTrayAssignment.cpp:1254-1316 (30 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayForm.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTrayForm.cpp:604-608 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayForm.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTrayForm.cpp:610-638 (15 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.YieldMonitoring.html | btnApply | Save | `dead/A` | clicked ; dom:1 | Command.cpp FileRW/TestIF_File_QAMode.cpp | btnApplyClick uYieldMonitoring.cpp:3064-3138 (39 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.YieldMonitoring.html | btnOk | Exit | `dead/A` | clicked | forms/fQAMode.cpp forms/fQAMode.h | btnOkClick uYieldMonitoring.cpp:3140-3145 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.CounterSel.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cCounterSel.cpp:145-149 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.GroundMan.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick GroundMan/GroundMan.cpp:1471-1476 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.GroundMan.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick GroundMan/GroundMan.cpp:1418-1468 (24 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.ShowBinSelect.html | btReturn | return | `dead/A` | clicked | cShowBinSelect.cpp forms/fShowBinSelect.h | btReturnClick cShowBinSelect.cpp:2244-2251 (5 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.TowerLight.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cTowerLight.cpp:155-158 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Exit | Exit | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.Cleaning.html | btFocusOnly | Only Focus | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbFix5 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbFix6 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1LB_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1RA_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1RB_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2LB_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2RA_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2RB_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbScanAOI |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbZcalibration | Go To In/Out Arm Z Calib | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Alert.MyMessageBox.NonStop.html | nsOk | 確認 OK | `dead/?` | clicked | — | — |  |  |
| Alert.Note.NonStop.html | nsOk | 確認 OK | `dead/?` | clicked | — | — |  |  |
| Setup.BinSel.html | btnAutoHide | 自動隱藏停用列 | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BinSel.html | spbAOIBin | AOI Bin | `dead/?` | clicked ; dom:1 | — | 沒有 OnClick |  |  |
| Status.TemperFrom.html | tzExpand | Expand all ▸ | `dead/?` | clicked ; dom:2 | — | — |  |  |
| Data.Builder.html | spbExit | Exit | `OK-cmd` | clicked ; control.acquire ; builder.op | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cBuilder.cpp:484-487 (1 句) |  |  |
| Data.ContactCT.html | btClearCount | Count Clear | `OK-cmd` | clicked ; control.acquire ; act.contactCT.clearCount ; contactct.get ; dom:8 | JsonBridge/actions/MainClarnData.cpp cContactCT.cpp | btClearCountClick cContactCT.cpp:944-1069 (76 句) |  |  |
| Data.ContactCT.html | btYieldChart | Yield Chart | `OK-cmd` | clicked ; control.acquire ; act.contactCT.yieldChart ; contactct.get ; dom:8 | cContactCT.cpp forms/fContactCT.h | btYieldChartClick cContactCT.cpp:1071-1075 (2 句) |  |  |
| Data.CounterClear.html | spbExe | Execute | `OK-cmd` | clicked ; counterclear.exe ; dom:6 | counterclear.get=yes counterclear.click=yes | spbExeClick cCounterClear.cpp:394-450 (28 句) |  |  |
| Data.Observer.html | btExit | Exit | `OK-cmd` | clicked ; control.acquire ; act.observer.exit ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | BtnExitClick cObserver.cpp:697-706 (4 句) |  |  |
| Data.Observer.html | btnClearTime | Clear Time Data | `OK-cmd` | clicked ; control.acquire ; act.observer.clearTime ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | btnClearTimeClick cObserver.cpp:5393-5399 (4 句) |  |  |
| Data.SortCT.html | btnClearCount | 🗒 Clear Count | `OK-cmd` | clicked ; control.acquire ; act.sortCT.clearCount ; dom:3 | act.trayEdit=yes | btnClearCountClick cSortCT.cpp:577-669 (40 句) |  |  |
| HW.HandlerSys.html | LoadBtn | Load | `OK-cmd` | clicked ; control.acquire ; dom:4 | — | LoadBtnClick HandlerSys.cpp:985-988 (1 句) |  |  |
| HW.MotorTest.html | btResetMNet | Reset MNet | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | forms/fMotorTest.h | btResetMNetClick uMotorTest.cpp:1718-1725 (3 句) |  |  |
| HW.MotorTest.html | btnGo | Go | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:932 | forms/fMotorTest.h | btnGoClick uMotorTest.cpp:1605-1619 (6 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnGoSoftN | Go Soft N Pos | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | forms/fMotorTest.h | btnGoSoftNClick uMotorTest.cpp:1105-1111 (2 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnGoSoftP | Go Soft P Pos | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | forms/fMotorTest.h | btnGoSoftPClick uMotorTest.cpp:1097-1103 (2 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnHighSpeed | Jog High | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:114 | forms/fMotorTest.h | btnHighSpeedClick uMotorTest.cpp:1403-1409 (3 句) |  |  |
| HW.MotorTest.html | btnHome | Home Reset | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | forms/fMotorTest.h | btnHomeClick uMotorTest.cpp:1113-1174 (37 句) | MOT[].PCIL132_StopMotor |  |
| HW.MotorTest.html | btnHomeLow | Home Low | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:112 | forms/fMotorTest.h | btnHomeLowClick uMotorTest.cpp:1427-1433 (3 句) |  |  |
| HW.MotorTest.html | btnMotorPower | Motor Power | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:160 | forms/fMotorTest.h | btnMotorPowerClick uMotorTest.cpp:1621-1645 (14 句) | SW[].On |  |
| HW.MotorTest.html | btnReloadMotorData | Reload Motor Data | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET /api/struct/motor/config ; http:GET / | forms/fMotorTest.h | btnReloadMotorDataClick uMotorTest.cpp:1695-1711 (8 句) |  |  |
| HW.MotorTest.html | btnServoOff | Servo Off | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:158 | forms/fMotorTest.h | btnServoOffClick uMotorTest.cpp:1661-1671 (6 句) | MOT[].ServoOnOff |  |
| HW.MotorTest.html | btnSetPosN | Set Position 2 | `OK-cmd` | clicked ; control.acquire ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline. | forms/fMotorTest.cpp forms/fMotorTest.h | btnSetPosNClick uMotorTest.cpp:1088-1095 (2 句) |  |  |
| HW.MotorTest.html | btnSetRange | Test Range | `OK-cmd` | clicked ; control.acquire ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline. | forms/fMotorTest.h | btnSetRangeClick uMotorTest.cpp:1374-1382 (5 句) |  |  |
| HW.MotorTest.html | btnStop | Stop | `OK-cmd` | clicked ; motor.stop ; dom:114 | forms/fMotorTest.h | btnStopClick uMotorTest.cpp:1647-1659 (6 句) | MOT[].PCIL132_StopMotor StopAllMotor |  |
| HW.MotorTest.html | palExit | Exit | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:112 | forms/fMotorTest.cpp forms/fMotorTest.h | palExitClick uMotorTest.cpp:1727-1730 (1 句) |  |  |
| HW.MotorTest.html | sbMotorTest_JogN | - | `OK-cmd` | clicked ; control.takeover ; motor.access ; motor.stop ; dom:963 | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | sbMotorTest_JogP | + | `OK-cmd` | clicked ; control.takeover ; motor.access ; motor.stop ; dom:965 | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | sbMotorTest_MoveN | - | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | forms/fMotorTest.h | sbMotorTest_MoveNClick uMotorTest.cpp:1224-1250 (13 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.MotorTest.html | sbMotorTest_MoveP | + | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | forms/fMotorTest.h | sbMotorTest_MovePClick uMotorTest.cpp:1252-1278 (13 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.home.html | sbAbortHome | Abort Home | `OK-cmd` | clicked ; act.home.abort ; dom:6 | act.home.abort=yes | sbAbortHomeClick uhome.cpp:4980-4986 (4 句) |  |  |
| HW.teach.html | MotorInArmPitchX | X Pitch | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmX | X | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:68 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZA | ZA | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZA) ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAe | Z Ae | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZAe) ; dom:29 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAh | Z Ah | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZB | ZB | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZB) ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZD | ZD | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZD) ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZF | ZF | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZG | ZG | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.acquire ; motor.access(MInArmZG) ; dom: | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInSh2 | X | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.acquire ; motor.access(MInShuttle2) ; d | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm1Y | Y | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:68 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm1Z | Z | `OK-cmd` | clicked ; control.acquire ; motor.access(MTestZ1) ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmX | X | `OK-cmd` | clicked ; control.acquire ; motor.access(MOutArmX) ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmY | Y | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:68 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZB | ZB | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZH | ZH | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnAlarmReset | Alarm Reset | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:48 | — | — |  |  |
| HW.teach.html | btnHome | HOME | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | forms/fMotorTest.h | btnHomeClick uteach.cpp:2133-2198 (37 句) | MOT[].ServoOnOff StopAllMotor |  |
| HW.teach.html | btnInZAllUp | In Z All Up | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZH) ; dom:5890 | — | btnInZAllUpClick uteach.cpp:4466-4478 (8 句) |  |  |
| HW.teach.html | btnJogN | JOG N | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | 沒有 OnClick |  |  |
| HW.teach.html | btnJogP | JOG P | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; motor.stop(MInArmX) ; dom:5885 | — | 沒有 OnClick |  |  |
| HW.teach.html | btnLoaderY | Loader Y | `OK-cmd` | clicked ; control.acquire ; motor.access(MLoaderY) ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnMotorTest | Motor Tools | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:21 | — | btnMotorTestClick uteach.cpp:2436-2440 (2 句) |  |  |
| HW.teach.html | btnMoveN | Move - | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:5888 | — | btnMoveNClick uteach.cpp:2205-2232 (15 句) | MOT[].Gali_MovePR MOT[].MotorMove |  |
| HW.teach.html | btnMoveP | Move + | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | btnMovePClick uteach.cpp:2104-2131 (15 句) | MOT[].Gali_MovePR MOT[].MotorMove |  |
| HW.teach.html | btnMoveTo | Move | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:5865 | — | btnMoveToClick uteach.cpp:2391-2407 (8 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnOutZAllUp | Out Z All Up | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchX4) ; dom:5867 | — | btnOutZAllUpClick uteach.cpp:4480-4492 (8 句) |  |  |
| HW.teach.html | btnSave | SAVE | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:44 | FileRW/Teach.cpp Interface/TesterTCP.cpp | btnSaveClick uteach.cpp:2261-2389 (67 句) |  |  |
| HW.teach.html | btnServo | Servo | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | btnServoClick uteach.cpp:4287-4293 (3 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetTo | SET TO | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:24 | forms/fTeach.cpp forms/fTeach.h | btnSetToClick uteach.cpp:2098-2102 (2 句) |  |  |
| HW.teach.html | btnStop | STOP | `OK-cmd` | clicked ; motor.stop(MInArmX) ; dom:29 | forms/fMotorTest.h | btnStopClick uteach.cpp:2948-2954 (4 句) | StopAllMotor |  |
| HW.teach.html | pnlExit | EXIT | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:42 | — | pnlExitClick uteach.cpp:5026-5032 (3 句) |  |  |
| Main.CommView.html | btnReadZ1 | Read Z1 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:4 | — | btnReadZ1Click main.cpp:21945-21948 (1 句) |  |  |
| Main.CommView.html | btnReadZ2 | Read Z2 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | btnReadZ2Click main.cpp:21950-21953 (1 句) |  |  |
| Main.CommView.html | btnSetZ1 | Set Z1 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | btnSetZ1Click main.cpp:21927-21934 (3 句) |  |  |
| Main.CommView.html | btnSetZ2 | Set Z2 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | btnSetZ2Click main.cpp:21936-21943 (3 句) |  |  |
| Main.Record.html | spbClearRecord | CLEAR | `OK-cmd` | clicked ; control.acquire ; act.main.clearRecord ; dom:3 | act.main.clearRecord=yes act.main.meShuttle2Dbl=yes | spbClearRecordClick main.cpp:30062-30091 (7 句) |  |  |
| Main.gbControlBtn.html | sbStateRecord | State Record | `OK-cmd` | clicked ; control.takeover ; act.main.stateRecord ; dom:6 | control.takeover=yes act.main.stateRecord=yes | sbStateRecordClick main.cpp:26209-26212 (1 句) |  |  |
| Setup.HotPlate.html | spbSave | Save | `OK-cmd` | clicked ; control.acquire ; dom:4 ; err:rejection c12 probe stub | Automation/auto9045.cpp Command.cpp | spbSaveClick cHotPlate.cpp:440-474 (21 句) |  |  |
| Status.Security.html | SecurityExit | Exit | `OK-cmd` | clicked ; control.acquire ; dom:4 | WebLevelSet.cpp cSecurity.cpp | SecurityExitClick cSecurity.cpp:813-816 (1 句) |  |  |
| eventlog.html | q | Query | `OK-http` | clicked ; http:POST /api/ela/query ; http:GET /api/ela ; dom:4 | — | — |  |  |
| eventlog.html | saveSum | Save Summary | `OK-http` | clicked ; http:POST /api/ela/summary ; http:GET /api/ela ; dom:4 | — | — |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Setup | Setup | `OK-ui` | clicked ; dom:5 | — | 沒有 OnClick |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Summary | Summary | `OK-ui` | clicked ; dom:5 | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | BitBtn1 | Copy From | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | ATC/ATCInterface.cpp ATC/ATCInterface.h | BitBtn1Click uMotorTest.cpp:1384-1401 (14 句) |  |  |
| HW.MotorTest.html | btnAlarmReset | Alarm Reset | `OK-ui` | clicked ; dom:114 | — | — |  |  |
| HW.MotorTest.html | btnHomeHigh | Home High | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.h | btnHomeHighClick uMotorTest.cpp:1419-1425 (3 句) |  |  |
| HW.MotorTest.html | btnLoopMove | Loop Move | `OK-ui` | clicked ; dom:111 | forms/fMotorTest.h | btnLoopMoveClick uMotorTest.cpp:1300-1345 (26 句) | MOT[].PCIL132_StopMotor |  |
| HW.MotorTest.html | btnLowSpeed | Jog Low | `OK-ui` | clicked ; dom:112 | forms/fMotorTest.h | btnLowSpeedClick uMotorTest.cpp:1411-1417 (3 句) |  |  |
| HW.MotorTest.html | btnRange | Range | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.cpp forms/fMotorTest.h | btnRangeClick uMotorTest.cpp:1451-1456 (2 句) |  |  |
| HW.MotorTest.html | btnSetPosP | Set Position 1 | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.cpp forms/fMotorTest.h | btnSetPosPClick uMotorTest.cpp:1079-1086 (2 句) |  |  |
| HW.MotorTest.html | btnSoftNPos | Soft Neg | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.h | btnSoftNPosClick uMotorTest.cpp:1443-1449 (3 句) |  |  |
| HW.MotorTest.html | btnSoftPPos | Soft Pos | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.h | btnSoftPPosClick uMotorTest.cpp:1435-1441 (3 句) |  |  |
| HW.teach.html | MotorAuto1YCW | Auto 1 CW | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorAuto2YCW | Auto 2 CW | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX2 | X Pitch 2 | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX3 | X Pitch 3 | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX4 | X Pitch 4 | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmY | Y | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAf | Z Af | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAg | Z Ag | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZC | ZC | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZE | ZE | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZH | ZH | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInSh1 | X | `OK-ui` | clicked ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm2Y | Y | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm2Z | Z | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX | X Pitch | `OK-ui` | clicked ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX2 | X Pitch 2 | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX3 | X Pitch 3 | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX4 | X Pitch 4 | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZA | ZA | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZC | ZC | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZD | ZD | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZE | ZE | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZF | ZF | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZG | ZG | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutSh1 | X | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutSh2 | X | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorTrayX | X | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnSetToOffset | SET TO | `OK-ui` | clicked ; dom:22 | forms/fTeach.cpp forms/fTeach.h | btnSetToOffsetClick uteach.cpp:2200-2203 (1 句) |  |  |
| Main.MotionView.html | axisToggle | 顯示未啟用軸 | `OK-ui` | clicked ; dom:7 | — | — |  |  |
| Setup.OffSet.html | btnToIndexOffset | Go To Index & Tray Arm O | `OK-ui` | clicked ; dom:10 | forms/fOffSet.cpp forms/fOffSet.h | btnToIndexOffsetClick cOffSet.cpp:3376-3380 (2 句) |  |  |
| Status.LtcSensor.html | btnClose | Exit | `OK-ui` | clicked ; dom:63 | forms/fIoSetView.h | btnCloseClick LtcSensor.cpp:678-681 (1 句) |  |  |
| eventlog.html | jamSetting | Jam Code Setting | `OK-ui` | clicked ; http:GET /api/ela ; frame:open Status.Security.html ; dom:4 | — | — |  |  |

## 4. 抽 10 顆給筆電複驗

| # | 頁 | id | 分類 | 怎麼複驗 |
|---|---|---|---|---|
| 1 | Setup.Contact.html | spbSave | `dead/D` | 開這一頁按 `spbSave`，看 wb_serve oplog 有沒有收到命令；golden cContact.cpp:14072-14201 |
| 2 | Setup.ContactForce.html | btExit | `dead/D` | 開這一頁按 `btExit`，看 wb_serve oplog 有沒有收到命令；golden ContactForce.cpp:929-933 |
| 3 | Setup.OffSet.html | spbSave | `dead/D` | 開這一頁按 `spbSave`，看 wb_serve oplog 有沒有收到命令；golden cOffSet.cpp:2803-2879 |
| 4 | Setup.SetUp.html | sbUpdate | `dead/D` | 開這一頁按 `sbUpdate`，看 wb_serve oplog 有沒有收到命令；golden cSetUp.cpp:3468-3626 |
| 5 | Config.Configuration.html | sbExit | `dead/B` | 開這一頁按 `sbExit`，看 wb_serve oplog 有沒有收到命令；golden cConfiguration.cpp:6277-6284 |
| 6 | HW.MyCCLinkSensor.html | sbExit | `dead/B` | 開這一頁按 `sbExit`，看 wb_serve oplog 有沒有收到命令；golden CCLink/MyCCLinkSensor.cpp:491-521 |
| 7 | Setup.BarCode.html | bt2DIDOffset | `dead/B` | 開這一頁按 `bt2DIDOffset`，看 wb_serve oplog 有沒有收到命令；golden BarCode/BarCode.cpp:6799-6804 |
| 8 | Setup.SCK_ART.html | btnApplyCount | `dead/B` | 開這一頁按 `btnApplyCount`，看 wb_serve oplog 有沒有收到命令；golden Automation/SCK_ART.cpp:723-798 |
| 9 | Setup.SetUp.html | btAutoShuttlePitch | `dead/B` | 開這一頁按 `btAutoShuttlePitch`，看 wb_serve oplog 有沒有收到命令；golden cSetUp.cpp:4732-4736 |
| 10 | Setup.TrayForm.html | spbCopy | `dead/B` | 開這一頁按 `spbCopy`，看 wb_serve oplog 有沒有收到命令；golden cTrayForm.cpp:692-713 |

## 5. 點擊實測（請 St01 代跑；STEVEN-NB3 只編譯、不執行）

```
cd <repo>/HT9011UC_Cpp_V3.33.906.0/tools/webprobe
python c12_button_census.py --out-tsv c12_static.tsv
python c12_click_probe.py --census c12_static.tsv --out c12_click.tsv
python c12_button_census.py --merge c12_click.tsv --out-tsv <repo>/docs/handoff/ST02_BUTTON_CENSUS_20261002.tsv --out-md <repo>/docs/handoff/ST02_BUTTON_CENSUS_20261002.md
```

- 探針只連自己的假伺服器：頁面腳本跑之前先把**所有** WebSocket 網址（有頁寫死 `ws://127.0.0.1:9045/...`）和 fetch／XHR 網址改到假伺服器的埠；假伺服器每個命令都回 ok:false、`/api/*` 回 404。碰不到 wb_serve、碰不到機台、不讀寫機台檔案。需要 Edge（不是 ctest）。
- 每頁單獨開（不在外框裡）；有分頁的先點那一頁的頁籤；confirm／prompt 一律回「確定」，才看得到確認之後送的命令。
- 已知限制：①C++ 資料到了才綁的鈕（例 Offset 部位鈕、Temp_Set 的 btClearAll——editlist.get 回來才掛）在假伺服器下看起來沒反應，會落在 dead/；②FormShow bridge（Teach／IO）藏的元件在假伺服器下不會藏；③不在外框裡，`HT9045Link` 的 hub 路徑沒走到；④只收使用者真的點擊（`e.isTrusted`）的處理器（例 ht9045_dio_delete.js 的 spbDelete）——探針 v1 用 `el.click()`（isTrusted＝false）會讀成 dead/；⑤只改輸入框值的處理器（不送命令、不改 DOM，例 btClearAll）——v1 只數 DOM 異動，讀成 dead/。④⑤ 由探針 v2 解決（AI(W906-ST02-C12T) 20261005：先捲到看得見、找沒被蓋住的點，用 DevTools 真的滑鼠點擊；另數 input／change 事件與值有變的控制項，輸出多 inp／val 兩欄；被蓋住的記 covered(probe)、不點）；①②③ 仍在。dead/ 在真的 wb_serve 上確認之前不是待辦清單。

## 9. INBOX 155 重跑與 D 類分派（St02-E 1004；產生器不寫這節，重跑 `--out-md` 會蓋掉）

- **為什麼重跑**：St01 1004 19:33 的合併版（main `e0e6b70d`）在它那台讀不到 golden 樹，tsv 的 golden_dfm 從 1603 筆變成 0、§1 的 A／B／C／D 全不見、110 顆實測沒反應的都成了 `dead/?`。點擊那一欄（sent_probe）是好的。
- **做法**：在 STEVEN-NB3 用 golden **0618** 重跑靜態普查（RULINGS_20261003 第 2 條）；St01 的點擊結果從 sent_probe 欄還原成 c12_click_probe.py 的輸出格式（1384 列，跟 St01 回報的列數相同）再用 `--merge` 併回。golden_dfm 回到 1603 筆，跟 St01 合併前一致。
- **0625_Steven 對照**：同樣的合併跑一次 `--golden 906_0625_Steven`：**分類 0 列不同**；golden 細節欄（語句數、危險呼叫等）有 25 列不同，都不影響分類。
- **實測沒反應的 110 顆（A～E）**：A 75（含 1 顆 A+E?，Main.AOAInfo）、B 11、C0 14、D 5、? 5（網頁自己的按鈕，不是 golden 元件）。各列在 §1／§2 與 tsv。

### 9.1 D 類（golden 處理器會動馬達或寫輸出）分派建議

擁有者是依頁面事件／接線 JS 裡 AI 註記的角色推的**建議**，請 St02-M／筆電確認。St02 的檔案這次沒有 D 類。

| 頁 | 按鈕 | 實測 | golden 處理器（0618） | 會動什麼 | 建議誰做 | 註記 |
|---|---|---|---|---|---|---|
| Setup.OffSet | spbSave（Save） | 沒反應 | cOffSet.cpp:2803-2879 | fMain->Pause | St01（ht9045_offset_ev.js） | 先在真的 wb_serve 上確認：點擊探針用假伺服器，Save 沒反應可能是環境造成 |
| Setup.Contact | spbSave（Save） | 沒反應 | cContact.cpp:14072-14201 | ADAM_WriteVoltage | St01（ht9045_contact_ev.js） | 同上 |
| Setup.SetUp | sbUpdate（Save） | 沒反應 | cSetUp.cpp:3468-3626 | ADAM_WriteVoltage | St01（ht9045_setup_c_wire.js） | 同上 |
| Setup.ContactForce | btSave（Save） | 沒反應 | ContactForce.cpp:935-961 | ADAM_WriteVoltage | 筆電／機台端（ht9045_contactforce_c.js） | 同上 |
| Setup.ContactForce | btExit（Exit） | 沒反應 | ContactForce.cpp:929-933 | ADAM_WriteVoltage | 筆電／機台端（ht9045_contactforce_c.js） | 同上 |
| HW.teach | btnPos90／btnNeg90 × InRA～InRH、OutRA～OutRH（32 顆，±90） | 量測時看不見（分頁沒切到） | uteach.cpp:4605-4655／4657-4707 | MOT[].MotorMove | 筆電／機台端 | **先過 EastSun**；**第 63 批（KB-GOLDEN 認領了 HW.teach.html／ht9045_wire_hwteach.js）進 main 後重量** |
| HW.MotorTest | SpeedButton44／45／50／51（－／＋） | 量測時看不見 | uMotorTest.cpp:1515-1537 | MOT[].MotorMove | 筆電／機台端 | **先過 EastSun** |

- 靜態普查原本的 36 顆 D（St01 合併前的 `s:unbound/D`）就是上表的 HW.teach 32＋HW.MotorTest 4；點擊探針量的時候都在沒切到的分頁上，所以實測欄是「看不見」，不是「有反應」。
- **跟小鍵盤有關的列**：第 63 批（KB-GOLDEN）改了 qwerty.js 與引擎的按鍵攔截，所有「按了會開小鍵盤」的列（多在 `OK-ui`）與 HW.teach 頁，第 63 批進 main 後要重量。

## 10. 探針 v1 讀錯的列與 v2 重跑（St02-E 1005；產生器不寫這節，重跑 `--out-md` 會蓋掉）

St02-M 派 C12 A 類兩顆（DIOInterFaceCFG spbDelete、Temp_Set btClearAll）去接，讀了才發現**兩顆早就接好了**（都是 St02 做的），dead/A 是探針讀錯：

| 頁 | 鈕 | 已經接在 | 為什麼 v1 讀成 dead |
|---|---|---|---|
| Config.DIOInterFaceCFG | spbDelete | `web/page/ht9045_dio_delete.js`（AI(W906-DIO-DEL) 20261001；WS `ttlcfg.op` → FileRW/TTLCfg.cpp FileRW_TTLCfg_DeleteOp；golden 0618 DIOInterFaceCFG.cpp:248-256） | ④ `onDelete` 第一行 `if (ev && ev.isTrusted === false) return;` |
| Setup.Temp_Set | btClearAll | `web/page/Setup.Temp_Set.html:385`（AI(W906-Q41) 20260927 TS-3 `btClearAllClick`，清 golden 0618 uTemp_Set.cpp:5126-5135 的 7 格） | ① 在 `editlistGet('Temperature')` 回來後才由 `q41Hook()` 掛上；⑤ 只改 `.value` |

同樣原因、**已確認**的還有 Setup.AGV 的 btInitalLoad／btInitalUnLoad（`ht9045_agv_c.js` `onClick` 第一行就擋 isTrusted）。
用「該頁載入的腳本裡有 isTrusted 檢查、又引用這顆 id」掃出來的**候選**（沒逐一確認擋的是不是這顆鈕的 click）：
Setup.BarCode sbtExit；Setup.Cleaning btnStartAutoClean／sbTrayAssign／btnResetCleanCount；Setup.SetUp sbUpdate（dead/D）／sbtExit／btnLUpToRDownN／btnLUpToRDownZ／btnLDownToRUpZ／btnRUpToLDownZ／btnRDownToLUpZ／btnRUpToLDownN；
Setup.Temp_Set sbtExit；Setup.AGV spbSave。另外 dead/A 74 顆裡 73 顆在 .tsv 的 refs 欄已經有網頁程式引用——**dead/ 的「網頁沒送」多半是探針環境（①～⑤）造成的**，St02-M 1004 20:0x 列的「27 個真的缺口」要等真的 wb_serve 確認。

**v2 重跑（請 St01 代跑；這台只編譯、不執行）**——等第 65 批（含 !187 主畫面 Logo）進 main 之後在 main 上跑：

```
cd <repo>/HT9011UC_Cpp_V3.33.906.0/tools/webprobe
python c12_click_probe.py --census <repo>/docs/handoff/ST02_BUTTON_CENSUS_20261002.tsv --out c12_click_v2.tsv [--dbg-port 9362]
```

- 先跑自我測試（6 顆：ws、dom、dead、trusted（要捲 2400 px、只收真的點擊）、value（只改值）、covered（被蓋住、不點））；任一顆讀錯就 exit 2、不量。
- `c12_click_v2.tsv` 交回（放 handoff 或傳給 St02-M）；STEVEN-NB3 用 `python c12_button_census.py --merge c12_click_v2.tsv --out-tsv ... --out-md ...`（golden 0618）合併重分類，§9／§10 手寫節照舊補回。
