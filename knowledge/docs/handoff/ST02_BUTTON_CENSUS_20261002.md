# ST02-C12 按鈕普查：「看得到、按了沒反應」（靜態＋點擊實測）

> **golden 基準：`D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618`**（RULINGS_20261003 第 2 條：0618 為準，906_0625_Steven 只對照；對照結果見文末）。
> 量測樹：main ``。只量、只分類，**沒有改任何程式**（TO_STEVEN.md §3 ST02-C12）。
> 產生：`HT9011UC_Cpp_V3.33.906.0/tools/webprobe/c12_button_census.py`（靜態）＋ `c12_click_probe.py`（無頭 Edge＋假伺服器，**已合併實測結果**）。每顆的完整欄位在同名 `.tsv`。

## 0. 白話摘要

- 範圍：外框 `web/background.html` WINDOWS 表開得到的頁＋Alert 覆蓋頁，共 65 頁、1905 顆按鈕（golden 的 TButton／TBitBtn／TSpeedButton，加上網頁自己的 `<button>`）。
- 看得到又沒有變灰的：**1358 顆**（其餘：靜態藏起來 341、變灰 194、開發用頁 12）。
- 實測按了**什麼都沒發生**（沒送 WS、沒有 POST、沒有視窗動作、畫面沒變）：**378 顆**，依 golden／移植樹分成 A～E（§1）。

## 1. 分類與建議誰做

| 分類 | 意思 | 顆數 | 建議誰做 |
|---|---|---:|---|
| `dead/D` | 實測沒反應，golden 處理器本體會動馬達／寫輸出 | 5 | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| `dead/B` | 實測沒反應，C++ 沒有處理器 | 99 | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| `dead/A` | 實測沒反應，C++ 有處理器（form.event 表列或同名函式） | 194 | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| `dead/A+E?` | 實測沒反應，C++ 有處理器（form.event 表列或同名函式）；WORKLOG_MACHINE §4 有提到（id 或頁） | 1 | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送；機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| `dead/C` | 實測沒反應，golden dfm Visible=False、程式也不打開 | 2 | 照 golden 藏（golden 在這台本來就看不到）——網頁那一側 |
| `dead/C0` | 實測沒反應，golden 沒有 OnClick 或處理器是空的 | 72 | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| `dead/?` | 實測沒反應，不是 golden 元件、也沒有處理器線索 | 5 |  |
| `OK-cmd` | 實測送了 C++ 認得的命令 | 688 |  |
| `OK-http` | 實測送了 POST／PUT | 2 |  |
| `OK-ui` | 實測只有畫面／視窗動作（沒送 C++；探針 v2 起也算只改輸入框值的） | 72 |  |
| `s:unbound/?` | 靜態：沒有 script 提到這顆 id，不是 golden 元件、也沒有處理器線索 | 186 |  |
| `s:unbound/?+E?` | 靜態：沒有 script 提到這顆 id，不是 golden 元件、也沒有處理器線索；WORKLOG_MACHINE §4 有提到（id 或頁） | 32 | 機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| `greyed(static)` | 網頁 disabled | 1 |  |
| `greyed(probe)` | 實測變灰 | 193 |  |
| `hidden(static)` | 網頁 display:none／visibility:hidden（不算「看得到」） | 286 |  |
| `hidden(probe)` | 實測看不到（含 C++／FormShow 藏的） | 41 |  |
| `hidden(tab)` | 所在的分頁本身藏起來（多半是 golden dfm 的 TabVisible=False；探針 v3 不點） | 14 |  |
| `dev-page` | IDE.*／ScreenShots 開發用頁，不算 | 12 |  |

分類規則（`c12_button_census.py` classify／dead_cat）：C0 > C > D > A > B；D 只看處理器**本體**（呼叫下去的函式不追），讀 `MOT[i]` 位置不算 D，`MOT[i].Gali_MotMove(...)`、`ServoOnOff(...)`、`SW[..].On()`、`SetOutput*`、`ADAM_Write*`、`fMain->Start()` 算。E 只是「§4 文字裡出現這個 id 或頁名」，要人看。

## 2. 每頁一列

| 頁 | golden dfm | 全部 | 藏 | 灰 | 看得到 | 主要分類（顆數） |
|---|---|---:|---:|---:|---:|---|
| Main.html | main.dfm | 12 | 1 | 0 | 11 | s:unbound/?+E? 11 |
| Data.SortCT.html | cSortCT.dfm | 1 | 0 | 0 | 1 | OK-cmd 1 |
| Data.ContactCT.html | cContactCT.dfm | 2 | 0 | 0 | 2 | OK-cmd 2 |
| Data.LotInfo.html | uLotInfo.dfm | 15 | 13 | 2 | 0 |  |
| Status.TemperFrom.html | — | 2 | 1 | 0 | 1 | dead/? 1 |
| Data.Observer.html | cObserver.dfm | 35 | 0 | 0 | 35 | OK-cmd 35 |
| Status.ShowMessage.html | uShowMessage.dfm | 1 | 1 | 0 | 0 |  |
| Status.ShowBinSelect.html | cShowBinSelect.dfm | 4 | 3 | 0 | 1 | dead/A 1 |
| Setup.OffSet.html | cOffSet.dfm | 89 | 21 | 0 | 68 | dead/C0 62, OK-ui 3, dead/A 2, dead/D 1 |
| Setup.Speed.html | cSpeed.dfm | 6 | 0 | 0 | 6 | OK-ui 4, dead/A 2 |
| HW.IoSetView.html | iosetview.dfm | 17 | 2 | 5 | 10 | dead/A 9, OK-ui 1 |
| Config.Configuration.html | cConfiguration.dfm | 49 | 1 | 3 | 45 | dead/A 25, dead/B 16, dead/C 2, dead/C0 2 |
| Status.CounterSel.html | cCounterSel.dfm | 1 | 0 | 0 | 1 | dead/A 1 |
| Data.CounterClear.html | cCounterClear.dfm | 2 | 0 | 0 | 2 | dead/A 1, OK-cmd 1 |
| Data.Builder.html | cBuilder.dfm | 5 | 0 | 4 | 1 | OK-cmd 1 |
| Config.DIOInterFaceCFG.html | DIOInterFaceCFG.dfm | 4 | 0 | 1 | 3 | dead/A 2, OK-cmd 1 |
| Status.LtcSensor.html | LtcSensor.dfm | 16 | 1 | 14 | 1 | OK-ui 1 |
| Status.TowerLight.html | cTowerLight.dfm | 1 | 0 | 0 | 1 | dead/A 1 |
| HW.OmronEJ1N.html | EJ1N/OmronEJ1N.dfm | 15 | 0 | 15 | 0 |  |
| Setup.QAMode.html | QAMode.dfm | 2 | 0 | 0 | 2 | dead/A 2 |
| Setup.BarCode.html | BarCode/BarCode.dfm | 87 | 4 | 0 | 83 | dead/B 74, dead/C0 5, dead/A 3, OK-ui 1 |
| HW.MyCCLinkSensor.html | CCLink/MyCCLinkSensor.dfm | 80 | 6 | 73 | 1 | dead/B 1 |
| Setup.Cleaning.html | AutoClean/uCleaning.dfm | 8 | 0 | 0 | 8 | dead/A 7, dead/C0 1 |
| Setup.Contact.html | cContact.dfm | 20 | 15 | 2 | 3 | dead/A 2, dead/D 1 |
| Setup.TesterIF.html | cTesterIF.dfm | 3 | 0 | 0 | 3 | dead/A 3 |
| Status.GroundMan.html | GroundMan/GroundMan.dfm | 5 | 0 | 3 | 2 | dead/A 2 |
| Setup.Ld_ULd.html | cLd_ULd.dfm | 3 | 0 | 0 | 3 | dead/A 2, OK-ui 1 |
| Status.Security.html | cSecurity.dfm | 187 | 4 | 0 | 183 | s:unbound/? 180, OK-cmd 3 |
| Setup.TrayForm.html | cTrayForm.dfm | 4 | 0 | 0 | 4 | dead/A 3, dead/B 1 |
| Setup.SCK_ART.html | Automation/SCK_ART.dfm | 8 | 2 | 0 | 6 | dead/B 4, dead/A 2 |
| Setup.YieldMonitoring.html | uYieldMonitoring.dfm | 3 | 1 | 0 | 2 | dead/A 2 |
| Setup.HotPlate.html | cHotPlate.dfm | 2 | 0 | 0 | 2 | OK-cmd 1, dead/A 1 |
| Setup.SetUp.html | cSetUp.dfm | 9 | 0 | 0 | 9 | OK-ui 5, dead/A 2, dead/B 1, dead/D 1 |
| Data.SmartDiagnostic.html | SmartDiagnostic.dfm | 6 | 0 | 3 | 3 | OK-ui 2, dead/C0 1 |
| Data.StartCondition.html | cStartCondition.dfm | 88 | 0 | 0 | 88 | dead/A 85, dead/B 2, dead/C0 1 |
| Setup.Temp_Set.html | uTemp_Set.dfm | 10 | 2 | 0 | 8 | dead/A 8 |
| Setup.BinSel.html | cBinSel.dfm | 8 | 0 | 0 | 8 | dead/A 6, dead/? 2 |
| HW.teach.html | uteach.dfm | 898 | 205 | 44 | 649 | OK-cmd 612, OK-ui 37 |
| HW.MotorTest.html | uMotorTest.dfm | 52 | 16 | 1 | 35 | OK-cmd 20, OK-ui 15 |
| HW.home.html | uhome.dfm | 2 | 1 | 0 | 1 | OK-cmd 1 |
| HW.ShuttleMove.html | ShuttleMove.dfm | 26 | 7 | 17 | 2 | dead/A 2 |
| Setup.TrayAssignment.html | cTrayAssignment.dfm | 2 | 0 | 0 | 2 | dead/A 2 |
| Setup.ContactForce.html | ContactForce.dfm | 3 | 0 | 0 | 3 | dead/D 2, dead/A 1 |
| HW.VacuumUnit.html | VacuumUnit/VacuumUnit.dfm | 7 | 1 | 4 | 2 | dead/A 2 |
| HW.PadInterface.html | uPadInterface.dfm | 3 | 0 | 0 | 3 | OK-cmd 3 |
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
| Alert.Password.html | Password.dfm | 8 | 1 | 0 | 7 | dead/A 7 |

## 3. 每顆一列（看得到又沒有變灰的 1358 顆；藏起來／變灰的只在 .tsv）

欄位：頁、id、caption、分類、送出的命令（實測）、C++ 認不認得、golden 處理器（golden 906 檔名:行）、會動機台、建議誰做。

| 頁 | id | caption | 分類 | 送出（實測） | C++ | golden 處理器 | 動機台 | 建議誰做 |
|---|---|---|---|---|---|---|---|---|
| Main.html | （沒有 id ×11） | 🔄 Set … | `s:unbound/?+E?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Status.Security.html | （沒有 id ×180） | 🔑[00] Main - Tools … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| eventlog.html | （沒有 id ×4） | Save … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| testercomm.html | （沒有 id ×21） | Send to Handler … | `s:unbound/?+E?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Alert.MotionView9050.html | （沒有 id ×2） | P1 … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Setup.Contact.html | spbSave | Save | `dead/D` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cContact.cpp:14072-14201 (71 句) | ADAM_WriteVoltage | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Setup.ContactForce.html | btExit | Exit | `dead/D` | clicked | DynamicTemp.cpp forms/fDynamicTemp.h | btExitClick ContactForce.cpp:929-933 (2 句) | ADAM_WriteVoltage | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Setup.ContactForce.html | btSave | Save | `dead/D` | clicked ; dom:1 | ATC/ATCInterface.cpp ATC/ATCInterface.h | btSaveClick ContactForce.cpp:935-961 (19 句) | ADAM_WriteVoltage | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Setup.OffSet.html | spbSave | Save | `dead/D` | clicked ; dom:2 | Automation/auto9045.cpp Command.cpp | spbSaveClick cOffSet.cpp:2803-2879 (47 句) | fMain->Pause | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Setup.SetUp.html | sbUpdate | Save | `dead/D` | clicked ; dom:1 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick cSetUp.cpp:3468-3626 (77 句) | ADAM_WriteVoltage | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Config.Configuration.html | btnMesSystem | Mes System | `dead/B` | clicked | — | btnMesSystemClick cConfiguration.cpp:7622-7625 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN14_22Export | Export | `dead/B` | clicked | — | btnN14_22ExportClick cConfiguration.cpp:7710-7716 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN14_22Import | Import | `dead/B` | clicked | — | btnN14_22ImportClick cConfiguration.cpp:7718-7724 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN15ESDForm | ESD Form | `dead/B` | clicked | — | btnN15ESDFormClick cConfiguration.cpp:6700-6703 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN25_3_Manual | Manual | `dead/B` | clicked | — | btnN25_3_ManualClick cConfiguration.cpp:7737-7740 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN25_4_Manual | Manual | `dead/B` | clicked | — | btnN25_4_ManualClick cConfiguration.cpp:7742-7745 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN25_5_Manual | Manual | `dead/B` | clicked | — | btnN25_5_ManualClick cConfiguration.cpp:7747-7750 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN31_Manual | Manual | `dead/B` | clicked | — | btnN31_ManualClick cConfiguration.cpp:7616-7620 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN32 | Check Manual | `dead/B` | clicked | — | btnN32Click cConfiguration.cpp:7627-7637 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnOpenEP | OpenEP | `dead/B` | clicked | — | btnOpenEPClick cConfiguration.cpp:6638-6667 (22 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnSetIPSCCleaarQty | Set Clear | `dead/B` | clicked | — | btnSetIPSCCleaarQtyClick cConfiguration.cpp:7154-7166 (6 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnSetToTech | Set Offset To Tech | `dead/B` | clicked | — | btnSetToTechClick cConfiguration.cpp:5978-5981 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | sbExit | Exit | `dead/B` | clicked | — | sbExitClick cConfiguration.cpp:6277-6284 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | sbReadTemp | Read Temp | `dead/B` | clicked | — | sbSendTempClick cConfiguration.cpp:5655-5680 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | sbSendTemp | Send Temp Set | `dead/B` | clicked | — | sbSendTempClick cConfiguration.cpp:5655-5680 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | spbA27 | [A27] Save Standard Conf | `dead/B` | clicked | — | spbA27Click cConfiguration.cpp:6817-6837 (12 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Data.StartCondition.html | btnSetOffsetLimit | set limit to all | `dead/B` | clicked | — | btnSetOffsetLimitClick cStartCondition.cpp:1234-1249 (11 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Data.StartCondition.html | sb_Maintenance_SmartDiagnosticFunction | Setup | `dead/B` | clicked | — | sb_Maintenance_SmartDiagnosticFunctionClick cStartCondition.cpp:1080-1084 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| HW.MyCCLinkSensor.html | sbExit | Exit | `dead/B` | clicked | — | sbExitClick CCLink/MyCCLinkSensor.cpp:491-521 (13 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_1_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_1_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_2_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_2_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_3_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_3_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_4_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_4_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_5_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_5_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_6_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_6_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_7_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_7_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_8_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_8_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_1A_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_1A_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_1B_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_1B_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_2A_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_2A_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_2B_Connect | Connect | `dead/B` | clicked | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_2B_Disconnect | Disconnect | `dead/B` | clicked | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button1 | Get Barcode | `dead/B` | clicked | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button10 | Initial | `dead/B` | clicked | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button2 | Initial | `dead/B` | clicked | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button3 | Get Barcode | `dead/B` | clicked | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button4 | Initial | `dead/B` | clicked | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button5 | Get Barcode | `dead/B` | clicked | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button6 | Initial | `dead/B` | clicked | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button7 | Get Barcode | `dead/B` | clicked | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button8 | Initial | `dead/B` | clicked | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button9 | Get Barcode | `dead/B` | clicked | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | bt2DIDOffset | 2DID Offset | `dead/B` | clicked | — | bt2DIDOffsetClick BarCode/BarCode.cpp:6799-6804 (3 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBarcodeChangeFileConnect | Connect | `dead/B` | clicked | — | btBarcodeChangeFileConnectClick BarCode/BarCode.cpp:6119-6127 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBarcodeChangeFileDisConnect | DisConnect | `dead/B` | clicked | — | btBarcodeChangeFileDisConnectClick BarCode/BarCode.cpp:6129-6139 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBarcodeChangeFileSend | send | `dead/B` | clicked | — | btBarcodeChangeFileSendClick BarCode/BarCode.cpp:6141-6149 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom1_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom2_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom3_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom4_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom5_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom6_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom7_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom8_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btReader1On | Reader 1 On | `dead/B` | clicked | — | btReader1OnClick BarCode/BarCode.cpp:2430-2460 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btReader2On | Reader 2 On | `dead/B` | clicked | — | btReader1OnClick BarCode/BarCode.cpp:2430-2460 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btReader3On | Reader 3 On | `dead/B` | clicked | — | btReader1OnClick BarCode/BarCode.cpp:2430-2460 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btReader4On | Reader 4 On | `dead/B` | clicked | — | btReader1OnClick BarCode/BarCode.cpp:2430-2460 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btShuttle_1A_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btShuttle_1B_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btShuttle_2A_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btShuttle_2B_Trigger | Send CMD | `dead/B` | clicked | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btStart2DIDCheckSh1 | Cheack 2DID SH1 | `dead/B` | clicked | — | btStart2DIDCheckSh1Click BarCode/BarCode.cpp:6806-6810 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btStart2DIDCheckSh2 | Cheack 2DID SH2 | `dead/B` | clicked | — | btStart2DIDCheckSh2Click BarCode/BarCode.cpp:6812-6816 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Auto1 | Connect | `dead/B` | clicked | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Auto2 | Connect | `dead/B` | clicked | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Auto3 | Connect | `dead/B` | clicked | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Fix1 | Connect | `dead/B` | clicked | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Fix2 | Connect | `dead/B` | clicked | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Fix3 | Connect | `dead/B` | clicked | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Auto1 | Disconnect | `dead/B` | clicked | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Auto2 | Disconnect | `dead/B` | clicked | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Auto3 | Disconnect | `dead/B` | clicked | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Fix1 | Disconnect | `dead/B` | clicked | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Fix2 | Disconnect | `dead/B` | clicked | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Fix3 | Disconnect | `dead/B` | clicked | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_SendCmd_Auto1 | Send CMD | `dead/B` | clicked | — | btnClip_SendCmd_Auto1Click BarCode/BarCode.cpp:11737-11768 (17 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnGPIBReader4 | GPIB Rearder 4 | `dead/B` | clicked | — | btnGPIBReader4Click BarCode/BarCode.cpp:2462-2472 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnGetBarcode | Get Barcode | `dead/B` | clicked | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnGetLotID | Get | `dead/B` | clicked | — | btnGetLotIDClick BarCode/BarCode.cpp:11489-11495 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnInitialGetBarcode | Initial | `dead/B` | clicked | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | spbResetCom | Stop COM | `dead/B` | clicked | — | spbResetComClick BarCode/BarCode.cpp:2419-2428 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyCount | Apply Count | `dead/B` | clicked | — | btnApplyCountClick Automation/SCK_ART.cpp:723-798 (30 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyLotInfo | Apply Lot | `dead/B` | clicked | — | btnApplyLotInfoClick Automation/SCK_ART.cpp:1596-1606 (6 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyQty | Apply QTY | `dead/B` | clicked | — | btnApplyQtyClick Automation/SCK_ART.cpp:1521-1530 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplySetting | Apply Setting | `dead/B` | clicked | — | btnApplySettingClick Automation/SCK_ART.cpp:1532-1594 (40 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SetUp.html | btAutoShuttlePitch | Auto Shuttle Pitch | `dead/B` | clicked | — | btAutoShuttlePitchClick cSetUp.cpp:4732-4736 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.TrayForm.html | spbCopy | Copy From | `dead/B` | clicked | — | spbCopyClick cTrayForm.cpp:692-713 (17 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Alert.Password.html | btnOK | OK | `dead/A` | clicked | forms/fPassword.cpp forms/fPassword.h | btnOKClick Password.cpp:385-388 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordCancel | Cancel | `dead/A` | clicked | forms/fPassword.cpp forms/fPassword.h | sbPasswordCancelClick Password.cpp:273-276 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordModify | Modify | `dead/A` | clicked | forms/fPassword.cpp forms/fPassword.h | sbPasswordModifyClick Password.cpp:299-312 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordModifyCancel | Cancel | `dead/A` | clicked | forms/fPassword.cpp forms/fPassword.h | sbPasswordCancelClick Password.cpp:273-276 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordModifyOK | Save | `dead/A` | clicked | Password.cpp forms/fPassword.h | sbPasswordModifyOKClick Password.cpp:284-297 (6 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordOK | OK | `dead/A` | clicked | forms/fPassword.cpp forms/fPassword.h | sbPasswordOKClick Password.cpp:278-282 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | spbCancel | Cancel | `dead/A` | clicked | forms/fPassword.cpp forms/fPassword.h | spbCancelClick Password.cpp:123-128 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btD47 | Clear | `dead/A` | clicked | form.event=yes | btD47Click cConfiguration.cpp:5956-5960 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btHeaterClearSelect | Clear | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btHeaterClearSelectClick cConfiguration.cpp:5477-5484 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btHeaterSelectAll | All | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btHeaterSelectAllClick cConfiguration.cpp:5468-5475 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btN06_TesterList | ... | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btN06_TesterListClick cConfiguration.cpp:6157-6163 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btN06_TesterMap | ... | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btN06_TesterMapClick cConfiguration.cpp:6339-6345 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btResume | Resume | `dead/A` | clicked | form.event 表列 | btResumeClick cConfiguration.cpp:6151-6155 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnAdd1000 | +1000 | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btnAdd1000Click cConfiguration.cpp:5429-5432 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnAdd10000 | +10000 | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btnAdd10000Click cConfiguration.cpp:5973-5976 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnAddHP | Add HP | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnAddHPClick cConfiguration.cpp:7014-7024 (8 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnAddTray | Add Tray | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnAddTrayClick cConfiguration.cpp:6878-6888 (8 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnAutoSaveSetAll | Set All | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btnAutoSaveSetAllClick cConfiguration.cpp:6331-6337 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnDec1000 | -1000 | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btnDec1000Click cConfiguration.cpp:5434-5437 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnDeleteHP | Delete HP | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnDeleteHPClick cConfiguration.cpp:7026-7040 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnDeleteTray | Delete Tray | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnDeleteTrayClick cConfiguration.cpp:6890-6904 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnModifyHP | Modify Data | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnModifyHPClick cConfiguration.cpp:6989-7012 (10 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnModifyTray | Modify Data | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnModifyTrayClick cConfiguration.cpp:6851-6876 (10 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnRecordJamRateByTimeClear | Set All | `dead/A` | clicked | form.event 表列 | btnRecordJamRateByTimeClearClick cConfiguration.cpp:6585-6591 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnSave | Save Config. | `dead/A` | clicked ; dom:1 | FileRW/Teach.cpp Interface/TesterTCP.cpp | btnSaveClick cConfiguration.cpp:7776-7780 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnSetIPSCQty | Set Qty | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btnSetIPSCQtyClick cConfiguration.cpp:7148-7152 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnSetTo1000 | =1000 | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | btnSetTo1000Click cConfiguration.cpp:5439-5442 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | sbN15UserLevelByTxtReadFilePath |  | `dead/A` | clicked | cConfiguration.cpp forms/fConfiguration.h | sbN15UserLevelByTxtReadFilePathClick cConfiguration.cpp:6687-6698 (5 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | sbUpdateHP | Save | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | sbUpdateHPClick cConfiguration.cpp:7088-7111 (18 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | sbUpdateTray | Save | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | sbUpdateTrayClick cConfiguration.cpp:6906-6929 (18 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | sbtReloadHP | Load Data | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | sbtReloadHPClick cConfiguration.cpp:7042-7086 (34 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | sbtReloadTray | Load Data | `dead/A` | clicked | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | sbtReloadTrayClick cConfiguration.cpp:6931-6975 (34 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.DIOInterFaceCFG.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick DIOInterFaceCFG.cpp:258-261 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.DIOInterFaceCFG.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick DIOInterFaceCFG.cpp:191-235 (28 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.CounterClear.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cCounterClear.cpp:452-456 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Aa | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ab | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ac | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ad | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ae | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Af | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ag | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ah | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ba | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bb | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bc | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bd | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Be | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bf | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bg | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bh | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Aa | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ab | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ac | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ad | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ae | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Af | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ag | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ah | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ba | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bb | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bc | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bd | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Be | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bf | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bg | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bh | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAa | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAb | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAc | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAd | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAe | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAf | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAg | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAh | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBa | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBb | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBc | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBd | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBe | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBf | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBg | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBh | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCa | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCb | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCc | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCd | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCe | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCf | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCg | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCh | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDa | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDb | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDc | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDd | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDe | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDf | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDg | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmA | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmB | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmC | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmD | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmE | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmF | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmG | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmH | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmA | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmB | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmC | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmD | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmE | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmF | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmG | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmH | clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | sbClearCount | Clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbClearCountClick cStartCondition.cpp:607-625 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | sbHeadCondition1Clear | Clear | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbHeadCondition1ClearClick cStartCondition.cpp:691-758 (41 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | sbHeadCondition1Save | Save | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbHeadCondition1SaveClick cStartCondition.cpp:926-974 (31 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | sbSameAsHead1 | Same As Head 1 | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbSameAsHead1Click cStartCondition.cpp:590-605 (10 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | sbSave | Save | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbSaveClick cStartCondition.cpp:627-658 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cStartCondition.cpp:670-689 (16 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.HandlerSys.html | ExitBtn | Close | `dead/A` | clicked | FileRW/HSys.cpp FileRW/HSys.gen.inc | ExitBtnClick HandlerSys.cpp:1037-1043 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.HandlerSys.html | SaveBtn | Save | `dead/A` | clicked ; dom:1 | FileRW/HSys.cpp FileRW/HSys.gen.inc | SaveBtnClick HandlerSys.cpp:979-983 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btComPoet | COM Port Setting | `dead/A` | clicked | forms/fIoSetView.h | btComPoetClick iosetview.cpp:2831-2836 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btIPSetting | IP Setting | `dead/A` | clicked | forms/fIoSetView.h | btIPSettingClick iosetview.cpp:2824-2829 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btKVM | KVM Setting | `dead/A` | clicked | forms/fIoSetView.h | btKVMClick iosetview.cpp:2838-2859 (8 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btnAddIO | Add IO | `dead/A` | clicked | forms/fIoSetView.h | btnAddIOClick iosetview.cpp:3155-3164 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btnDeleteIO | Delete IO | `dead/A` | clicked | forms/fIoSetView.h | btnDeleteIOClick iosetview.cpp:3166-3181 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btnLatchCheck | Sensor latch check | `dead/A` | clicked | forms/fIoSetView.h | btnLatchCheckClick iosetview.cpp:2025-2028 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btnModify | Modify Data | `dead/A` | clicked ; dom:2 | forms/fIoSetView.h forms/fMotorTest.cpp | btnModifyClick iosetview.cpp:3123-3141 (8 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | sb_IO_CommunicationPad | Pad | `dead/A` | clicked | forms/fIoSetView.h | sb_IO_CommunicationPadClick iosetview.cpp:3882-3885 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | sbtReload | Load Data | `dead/A` | clicked ; http:GET /api/system/ioTable ; dom:1 ; err:rejection GET /api/system/ioTable ->  | forms/fDTME08.cpp forms/fDTME08.h | btnReloadClick iosetview.cpp:1252-1266 (9 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.ShuttleMove.html | sbUpdate | Save | `dead/A` | clicked ; dom:1 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick ShuttleMove.cpp:1965-1994 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.ShuttleMove.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick ShuttleMove.cpp:1956-1961 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.VacuumUnit.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick VacuumUnit/VacuumUnit.cpp:402-406 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.VacuumUnit.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick VacuumUnit/VacuumUnit.cpp:378-400 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Main.AOAInfo.html | OffsetSave | Save | `dead/A+E?` | clicked ; dom:1 | FileRW/AOAOffset.cpp FileRW/AOAOffset.gen.inc | OffsetSaveClick main.cpp:33822-33915 (78 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送；機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| Setup.AGV.html | btInitalLoad | Inital Load | `dead/A` | clicked ; dom:1 | form.event=yes | btInitalLoadClick Automation/AGV.cpp:1055-1059 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.AGV.html | btInitalUnLoad | Inital Unload | `dead/A` | clicked ; dom:1 | form.event=yes | btInitalUnLoadClick Automation/AGV.cpp:1061-1065 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.AGV.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick Automation/AGV.cpp:1036-1041 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.AGV.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick Automation/AGV.cpp:928-969 (30 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BarCode.html | sbtExit | Exit | `dead/A` | clicked | form.event 表列 | sbtExitClick BarCode/BarCode.cpp:2392-2401 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BarCode.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick BarCode/BarCode.cpp:1254-1457 (129 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BarCode.html | spbStartCom | Start COM | `dead/A` | clicked | FileRW/TestIF_File_BarCode.gen.inc WebStart.cpp | spbStartComClick BarCode/BarCode.cpp:2504-2510 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | btnSetAll2NotUse | Set all to not use | `dead/A` | clicked ; dom:2 | cBinSel.cpp forms/fBinSel.h | btnSetAll2NotUseClick cBinSel.cpp:6372-6378 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | btnSettingSpecificBin | Specific Bin | `dead/A` | clicked ; dom:1 | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | btnSettingSpecificBinClick cBinSel.cpp:6118-6131 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | sbtExit | ✕Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cBinSel.cpp:2755-2759 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | spbNormal | Normal | `dead/A` | clicked ; dom:1 | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | spbNormalClick cBinSel.cpp:2761-2781 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | spbPrime | Prime | `dead/A` | clicked ; dom:1 | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | spbPrimeClick cBinSel.cpp:2783-2803 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BinSel.html | spbSave | 💾Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cBinSel.cpp:2214-2272 (30 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | btInclude | Include Tray Data | `dead/A` | clicked | form.event 表列 | btIncludeClick AutoClean/uCleaning.cpp:2248-2262 (10 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | btnResetCleanCount | Reset | `dead/A` | clicked | form.event=yes | btnResetCleanCountClick AutoClean/uCleaning.cpp:2095-2117 (15 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | btnResetInterval | Reset Interval | `dead/A` | clicked | form.event=yes | btnResetIntervalClick AutoClean/uCleaning.cpp:2810-2815 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | btnStartAutoClean | Clean | `dead/A` | clicked | form.event=yes | btnStartAutoCleanClick AutoClean/uCleaning.cpp:2869-2872 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | sbCleanExit | Exit | `dead/A` | clicked | FileRW/TestIF_File_Cleaning.cpp FileRW/TestIF_File_Cleaning. | sbCleanExitClick AutoClean/uCleaning.cpp:1868-1872 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | sbCleanSave | Save | `dead/A` | clicked | FileRW/TestIF_File_Cleaning.cpp FileRW/TestIF_File_Cleaning. | sbCleanSaveClick AutoClean/uCleaning.cpp:1761-1866 (49 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Cleaning.html | sbTrayAssign | Tray Assign | `dead/A` | clicked | form.event=yes | sbTrayAssignClick AutoClean/uCleaning.cpp:2290-2294 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Contact.html | btnTempSet | Temperature Setting | `dead/A` | clicked | forms/fContact.h | btnTempSetClick cContact.cpp:17184-17187 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Contact.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cContact.cpp:14478-14487 (5 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.ContactForce.html | Button1 | Convert | `dead/A` | clicked ; dom:1 | ATC/ATCInterface.cpp ATC/ATCInterface.h | Button1Click ContactForce.cpp:1588-1599 (5 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.HotPlate.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cHotPlate.cpp:627-631 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Ld_ULd.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cLd_ULd.cpp:218-222 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Ld_ULd.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cLd_ULd.cpp:179-202 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.OffSet.html | btnOffsetList | To Offset List | `dead/A` | clicked ; dom:2 | forms/fOffSet.h | btnOffsetListClick cOffSet.cpp:3369-3374 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.OffSet.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cOffSet.cpp:2797-2801 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.QAMode.html | btnApply | Save | `dead/A` | clicked ; dom:1 | Command.cpp FileRW/TestIF_File_QAMode.cpp | btnApplyClick QAMode.cpp:73-110 (26 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.QAMode.html | btnOk | Exit | `dead/A` | clicked | forms/fQAMode.cpp forms/fQAMode.h | btnOkClick QAMode.cpp:27-30 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SCK_ART.html | btnExit | Exit | `dead/A` | clicked | ATC/ATCInterface.cpp ATC/ATCInterface.h | btnExitClick Automation/SCK_ART.cpp:808-820 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SCK_ART.html | btnExit1 | Exit | `dead/A` | clicked | ATC/ATCInterface.cpp ATC/ATCInterface.h | btnExitClick Automation/SCK_ART.cpp:808-820 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | btnRUpToLDownN |  | `dead/A` | clicked | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.SetUp.html | sbtExit | Exit | `dead/A` | clicked | form.event 表列 | sbtExitClick cSetUp.cpp:3452-3466 (8 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Speed.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cSpeed.cpp:1788-1793 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Speed.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cSpeed.cpp:1433-1786 (200 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btClearAll | Clear All | `dead/A` | clicked | forms/fTemp_Set.h uTemp_Set.cpp | btClearAllClick uTemp_Set.cpp:5126-5135 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btnDefrostEnd | End | `dead/A` | clicked | forms/fTemp_Set.h uTemp_Set.cpp | btnDefrostEndClick uTemp_Set.cpp:6770-6776 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btnDefrostStart | Start | `dead/A` | clicked | forms/fTemp_Set.h uTemp_Set.cpp | btnDefrostStartClick uTemp_Set.cpp:6729-6768 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btnSameAsArm1 | Same as Arm1 | `dead/A` | clicked | forms/fTemp_Set.h uTemp_Set.cpp | btnSameAsArm1Click uTemp_Set.cpp:6177-6186 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btn_DefrostAllUseEnd | All Use End | `dead/A` | clicked | forms/fTemp_Set.h uTemp_Set.cpp | btn_DefrostAllUseEndClick uTemp_Set.cpp:6951-6956 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btn_DefrostAllUseStart | All Use Start | `dead/A` | clicked | forms/fTemp_Set.h uTemp_Set.cpp | btn_DefrostAllUseStartClick uTemp_Set.cpp:6936-6943 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | sbtExit | Exit | `dead/A` | clicked | form.event 表列 | sbtExitClick uTemp_Set.cpp:5101-5124 (15 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick uTemp_Set.cpp:4201-4528 (167 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TesterIF.html | btTesterTCPShow | Tester TCP | `dead/A` | clicked | forms/fLotInfo.cpp forms/fLotInfo.h | btTesterTCPShowClick cTesterIF.cpp:1516-1519 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TesterIF.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTesterIF.cpp:1358-1366 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TesterIF.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTesterIF.cpp:1302-1356 (32 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayAssignment.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTrayAssignment.cpp:1544-1548 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayAssignment.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTrayAssignment.cpp:1254-1316 (30 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayForm.html | btnBinBoxReset | Reset | `dead/A` | clicked | form.event 表列 | btnBinBoxResetClick cTrayForm.cpp:728-733 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayForm.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTrayForm.cpp:604-608 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.TrayForm.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTrayForm.cpp:610-638 (15 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.YieldMonitoring.html | btnApply | Save | `dead/A` | clicked ; dom:1 | Command.cpp FileRW/TestIF_File_QAMode.cpp | btnApplyClick uYieldMonitoring.cpp:3064-3138 (39 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.YieldMonitoring.html | btnOk | Exit | `dead/A` | clicked | forms/fQAMode.cpp forms/fQAMode.h | btnOkClick uYieldMonitoring.cpp:3140-3145 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.CounterSel.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cCounterSel.cpp:145-149 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.GroundMan.html | sbtExit | Exit | `dead/A` | clicked | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick GroundMan/GroundMan.cpp:1471-1476 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.GroundMan.html | spbSave | Save | `dead/A` | clicked ; dom:1 | Automation/auto9045.cpp Command.cpp | spbSaveClick GroundMan/GroundMan.cpp:1418-1468 (24 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.ShowBinSelect.html | btReturn | return | `dead/A` | clicked | cShowBinSelect.cpp forms/fShowBinSelect.h | btReturnClick cShowBinSelect.cpp:2244-2251 (5 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.TowerLight.html | spbExit | Exit | `dead/A` | clicked | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cTowerLight.cpp:155-158 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | spbA25_RunExecutFilePathChoice |  | `dead/C` | clicked | cConfiguration.cpp forms/fConfiguration.h | spbA25_RunExecutFilePathChoiceClick cConfiguration.cpp:6705-6716 (5 句) |  | 照 golden 藏（golden 在這台本來就看不到）——網頁那一側 |
| Config.Configuration.html | spbA28_2 | OPEN | `dead/C` | clicked | — | spbA28_2Click cConfiguration.cpp:6800-6815 (9 句) |  | 照 golden 藏（golden 在這台本來就看不到）——網頁那一側 |
| Config.Configuration.html | btN06_UpdateTesterList | Update Tester List | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Config.Configuration.html | btnAutoCalSuckZ | Set Auto Calibrate Suck  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Exit | Exit | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Data.StartCondition.html | btnClearDh | clear | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.BarCode.html | btnClip_SendCmd_Auto2 | Send CMD | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.BarCode.html | btnClip_SendCmd_Auto3 | Send CMD | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.BarCode.html | btnClip_SendCmd_Fix1 | Send CMD | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.BarCode.html | btnClip_SendCmd_Fix2 | Send CMD | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.BarCode.html | btnClip_SendCmd_Fix3 | Send CMD | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.Cleaning.html | btFocusOnly | Only Focus | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | IndexOffSetBT1 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | IndexOffSetBT2 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnBottom2D |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsAuto1 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsAuto2 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsAuto3 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsAuto4 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsAuto5 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsAuto6 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsColor |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsEmpty |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | btnTrayOfsLoader |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAuto1 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAuto2 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAuto3 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAuto4 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAuto5 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAuto6 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAutoSh1 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbAutoSh2 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbFix1 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbFix2 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbFix3 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbFix4 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbFix5 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbFix6 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbHp1 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbHp2 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInPlacement |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInRotate |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1LB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1LB_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1RA |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1RA_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1RB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1RB_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh1_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2LB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2LB_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2RA |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2RA_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2RB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2RB_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbInSh2_AutoClean |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbLoader |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbLoaderB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOCR |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutRotate |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutSh1 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutSh1LB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutSh1RA |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutSh1RB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutSh2 |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutSh2LB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutSh2RA |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbOutSh2RB |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbPreciser |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbScanAOI |  | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Setup.OffSet.html | sbZcalibration | Go To In/Out Arm Z Calib | `dead/C0` | clicked | — | 沒有 OnClick |  | 不用做：golden 這顆本來就沒有 OnClick／處理器是空的 |
| Alert.MyMessageBox.NonStop.html | nsOk | 確認 OK | `dead/?` | clicked | — | — |  |  |
| Alert.Note.NonStop.html | nsOk | 確認 OK | `dead/?` | clicked | — | — |  |  |
| Setup.BinSel.html | btnAutoHide | 自動隱藏停用列 | `dead/?` | clicked ; dom:1 | — | — |  |  |
| Setup.BinSel.html | spbAOIBin | AOI Bin | `dead/?` | clicked ; dom:1 | — | 沒有 OnClick |  |  |
| Status.TemperFrom.html | tzExpand | Expand all ▸ | `dead/?` | clicked ; dom:2 | — | — |  |  |
| Config.DIOInterFaceCFG.html | spbDelete | Delete | `OK-cmd` | clicked ; control.acquire ; ttlcfg.op ; dom:1 | FileRW/TTLCfg.cpp FileRW/TTLCfg.gen.inc | spbDeleteClick DIOInterFaceCFG.cpp:248-256 (4 句) |  |  |
| Data.Builder.html | spbExit | Exit | `OK-cmd` | clicked ; control.acquire ; builder.op | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cBuilder.cpp:484-487 (1 句) |  |  |
| Data.ContactCT.html | btClearCount | Count Clear | `OK-cmd` | clicked ; control.acquire ; act.contactCT.clearCount ; contactct.get ; dom:8 | JsonBridge/actions/MainClarnData.cpp cContactCT.cpp | btClearCountClick cContactCT.cpp:944-1069 (76 句) |  |  |
| Data.ContactCT.html | btYieldChart | Yield Chart | `OK-cmd` | clicked ; control.acquire ; act.contactCT.yieldChart ; contactct.get ; dom:8 | cContactCT.cpp forms/fContactCT.h | btYieldChartClick cContactCT.cpp:1071-1075 (2 句) |  |  |
| Data.CounterClear.html | spbExe | Execute | `OK-cmd` | clicked ; counterclear.exe ; dom:6 | counterclear.get=yes counterclear.click=yes | spbExeClick cCounterClear.cpp:394-450 (28 句) |  |  |
| Data.Observer.html | SpeedButton1 | Clear | `OK-cmd` | clicked ; observer.get | cObserver.cpp forms/fATCHandlerSide.h | SpeedButton1Click cObserver.cpp:865-871 (7 句) |  |  |
| Data.Observer.html | btExit | Exit | `OK-cmd` | clicked ; control.acquire ; act.observer.exit ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | BtnExitClick cObserver.cpp:697-706 (4 句) |  |  |
| Data.Observer.html | btOpenLoadLog | Open loader log | `OK-cmd` | clicked ; observer.get ; dom:1 | cObserver.cpp forms/fObserver.h | btOpenLoadLogClick cObserver.cpp:3638-3681 (33 句) |  |  |
| Data.Observer.html | btnBackupLogYear | Backup Log | `OK-cmd` | clicked ; observer.get ; control.acquire ; act.observer.backupLogYear ; dom:5 | cObserver.cpp forms/fObserver.h | btnBackupLogYearClick cObserver.cpp:5371-5391 (16 句) |  |  |
| Data.Observer.html | btnClearTime | Clear Time Data | `OK-cmd` | clicked ; control.acquire ; act.observer.clearTime ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | btnClearTimeClick cObserver.cpp:5393-5399 (4 句) |  |  |
| Data.Observer.html | btnLot1 | LOT 1 | `OK-cmd` | clicked ; observer.get ; dom:1 | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  |  |
| Data.Observer.html | btnLot2 | LOT 2 | `OK-cmd` | clicked ; observer.get | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  |  |
| Data.Observer.html | btnLot3 | LOT 3 | `OK-cmd` | clicked ; observer.get | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  |  |
| Data.Observer.html | btnLot4 | LOT 4 | `OK-cmd` | clicked ; observer.get ; dom:1 | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  |  |
| Data.Observer.html | btnLot5 | LOT 5 | `OK-cmd` | clicked ; observer.get ; dom:1 | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  |  |
| Data.Observer.html | btnQueryEventLogTxt | Query | `OK-cmd` | clicked ; observer.get ; observer.get ; dom:4 | cObserver.cpp forms/fObserver.h | btnQueryEventLogTxtClick cObserver.cpp:4423-4426 (1 句) |  |  |
| Data.Observer.html | btnSG_QueryNow | Query Now | `OK-cmd` | clicked ; control.acquire ; act.observerSG.state ; control.acquire ; act.observerSG.queryN | act.observerSG.=yes | btnSG_QueryNowClick cObserver.cpp:5361-5364 (1 句) |  |  |
| Data.Observer.html | btnSG_QueryYesterday | Query Yesterday | `OK-cmd` | clicked ; control.acquire ; act.observerSG.queryYesterday ; observer.get ; dom:7 | act.observerSG.=yes | btnSG_QueryYesterdayClick cObserver.cpp:5366-5369 (1 句) |  |  |
| Data.Observer.html | btnTimeData | Query | `OK-cmd` | clicked ; observer.get ; dom:1 | cObserver.cpp forms/fObserver.h | btnQueryEventLogTxtClick cObserver.cpp:4423-4426 (1 句) |  |  |
| Data.Observer.html | cobNoteContentsSet | Add | `OK-cmd` | clicked ; control.acquire ; act.observer.prNoteSet ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | cobNoteContentsSetClick cObserver.cpp:4452-4458 (2 句) |  |  |
| Data.Observer.html | sbCountermeasure | Add | `OK-cmd` | clicked ; observer.get ; control.acquire ; act.observer.mmCmAdd ; dom:5 | cObserver.cpp forms/fObserver.h | sbCountermeasureClick cObserver.cpp:4552-4556 (1 句) |  |  |
| Data.Observer.html | sbCountermeasureClear | Clear | `OK-cmd` | clicked ; control.acquire ; act.observer.mmCmClear ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbCountermeasureClearClick cObserver.cpp:4564-4567 (1 句) |  |  |
| Data.Observer.html | sbHandlerPrecautionFormShow | Show | `OK-cmd` | clicked ; observer.get ; control.acquire ; act.observer.prFormShow ; dom:5 | cObserver.cpp forms/fObserver.h | sbHandlerPrecautionFormShowClick cObserver.cpp:4473-4492 (10 句) |  |  |
| Data.Observer.html | sbHandlerPrecautionRecordClear | Clear | `OK-cmd` | clicked ; observer.get ; control.acquire ; act.observer.prRecordClear ; dom:5 | cObserver.cpp forms/fObserver.h | sbHandlerPrecautionRecordClearClick cObserver.cpp:4467-4471 (1 句) |  |  |
| Data.Observer.html | sbHandlerPrecautionRecordSet | Add | `OK-cmd` | clicked ; control.acquire ; act.observer.prRecordSet ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbHandlerPrecautionRecordSetClick cObserver.cpp:4460-4465 (1 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceDate | 日期: | `OK-cmd` | clicked ; observer.get ; control.acquire ; act.observer.mmDate ; dom:5 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceDateClick cObserver.cpp:4535-4538 (1 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceEndTime | 結束時間: | `OK-cmd` | clicked ; control.acquire ; act.observer.mmEnd ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceEndTimeClick cObserver.cpp:4569-4572 (1 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceSave | Save | `OK-cmd` | clicked ; control.acquire ; act.observer.mmSave ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceSaveClick cObserver.cpp:4574-4594 (15 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceSearch | Search | `OK-cmd` | clicked ; control.acquire ; act.observer.mmSearch ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceSearchClick cObserver.cpp:4596-4678 (45 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceStartTime | 開始時間: | `OK-cmd` | clicked ; control.acquire ; act.observer.mmStart ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceStartTimeClick cObserver.cpp:4540-4544 (1 句) |  |  |
| Data.Observer.html | sbPRFinishDate | 結案日期： | `OK-cmd` | clicked ; control.acquire ; act.observer.prFinishDate ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbPRFinishDateClick cObserver.cpp:4523-4527 (2 句) |  |  |
| Data.Observer.html | sbPRStartDate | 開始日期 : | `OK-cmd` | clicked ; control.acquire ; act.observer.prStartDate ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbPRStartDateClick cObserver.cpp:4529-4533 (2 句) |  |  |
| Data.Observer.html | sbPrecautionSave | Save | `OK-cmd` | clicked ; control.acquire ; act.observer.prSave ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbPrecautionSaveClick cObserver.cpp:4494-4521 (21 句) |  |  |
| Data.Observer.html | sbScreenkeyboard | Keyboard | `OK-cmd` | clicked ; observer.get ; dom:1 | cObserver.cpp forms/fObserver.h | sbScreenkeyboardClick cObserver.cpp:4431-4450 (12 句) |  |  |
| Data.Observer.html | sbScreenkeyboard2 | Keyboard | `OK-cmd` | clicked ; observer.get ; dom:1 | cObserver.cpp forms/fObserver.h | sbScreenkeyboardClick cObserver.cpp:4431-4450 (12 句) |  |  |
| Data.Observer.html | sbSearchPrecautionLog | Search | `OK-cmd` | clicked ; control.acquire ; act.observer.prLogSearch ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbSearchPrecautionLogClick cObserver.cpp:4680-4753 (39 句) |  |  |
| Data.Observer.html | sbUndesirablePhenomenon | Add | `OK-cmd` | clicked ; observer.get ; control.acquire ; act.observer.mmPhenAdd ; dom:5 | cObserver.cpp forms/fObserver.h | sbUndesirablePhenomenonClick cObserver.cpp:4546-4550 (1 句) |  |  |
| Data.Observer.html | sbUndesirablePhenomenonClear | Clear | `OK-cmd` | clicked ; control.acquire ; act.observer.mmPhenClear ; observer.get ; dom:7 | cObserver.cpp forms/fObserver.h | sbUndesirablePhenomenonClearClick cObserver.cpp:4558-4562 (1 句) |  |  |
| Data.Observer.html | txtReload | 🔄 重讀 | `OK-cmd` | clicked ; observer.get ; dom:1 | — | — |  |  |
| Data.Observer.html | txtUp | ⬆ 上一層 | `OK-cmd` | clicked ; observer.get ; dom:3 | — | — |  |  |
| Data.SortCT.html | btnClearCount | 🗒 Clear Count | `OK-cmd` | clicked ; control.acquire ; act.sortCT.clearCount ; dom:3 | act.trayEdit=yes | btnClearCountClick cSortCT.cpp:577-669 (40 句) |  |  |
| HW.HandlerSys.html | LoadBtn | Load | `OK-cmd` | clicked ; control.acquire ; dom:4 | — | LoadBtnClick HandlerSys.cpp:985-988 (1 句) |  |  |
| HW.MotorTest.html | BitBtn2 | 開始 | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | ATC/ATCInterface.cpp ATC/ATCInterface.h | BitBtn2Click uMotorTest.cpp:2096-2100 (2 句) |  |  |
| HW.MotorTest.html | BitBtn3 | 存檔 | `OK-cmd` | clicked ; control.takeover ; motor.access ; dlg:alert c12 probe stub ; dom:157 | ATC/ATCInterface.cpp ATC/ATCInterface.h | BitBtn3Click uMotorTest.cpp:2102-2119 (10 句) |  |  |
| HW.MotorTest.html | btResetMNet | Reset MNet | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | forms/fMotorTest.h | btResetMNetClick uMotorTest.cpp:1718-1725 (3 句) |  |  |
| HW.MotorTest.html | btnGo | Go | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | forms/fMotorTest.h | btnGoClick uMotorTest.cpp:1605-1619 (6 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnGoSoftN | Go Soft N Pos | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | forms/fMotorTest.h | btnGoSoftNClick uMotorTest.cpp:1105-1111 (2 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnGoSoftP | Go Soft P Pos | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | forms/fMotorTest.h | btnGoSoftPClick uMotorTest.cpp:1097-1103 (2 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnHome | Home Reset | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:969 | forms/fMotorTest.h | btnHomeClick uMotorTest.cpp:1113-1174 (37 句) | MOT[].PCIL132_StopMotor |  |
| HW.MotorTest.html | btnHomeLow | Home Low | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:112 | forms/fMotorTest.h | btnHomeLowClick uMotorTest.cpp:1427-1433 (3 句) |  |  |
| HW.MotorTest.html | btnModify | Modify Data | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:111 | forms/fIoSetView.h forms/fMotorTest.cpp | btnModifyClick uMotorTest.cpp:2214-2238 (10 句) |  |  |
| HW.MotorTest.html | btnMotorPower | Motor Power | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:160 | forms/fMotorTest.h | btnMotorPowerClick uMotorTest.cpp:1621-1645 (14 句) | SW[].On |  |
| HW.MotorTest.html | btnReloadMotorData | Reload Motor Data | `OK-cmd` | clicked ; control.takeover ; motor.access ; control.acquire ; motor.access ; http:GET /api | forms/fMotorTest.h | btnReloadMotorDataClick uMotorTest.cpp:1695-1711 (8 句) |  |  |
| HW.MotorTest.html | btnSaveLogLightScaleData | Save | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | forms/fMotorTest.h | btnSaveLogLightScaleDataClick uMotorTest.cpp:2051-2094 (27 句) |  |  |
| HW.MotorTest.html | btnServoOff | Servo Off | `OK-cmd` | clicked ; control.takeover ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline | forms/fMotorTest.h | btnServoOffClick uMotorTest.cpp:1661-1671 (6 句) | MOT[].ServoOnOff |  |
| HW.MotorTest.html | btnSetPosP | Set Position 1 | `OK-cmd` | clicked ; control.acquire ; motor.access ; http:GET ../JSON/offline/Motor-runtime.offline. | forms/fMotorTest.cpp forms/fMotorTest.h | btnSetPosPClick uMotorTest.cpp:1079-1086 (2 句) |  |  |
| HW.MotorTest.html | btnStop | Stop | `OK-cmd` | clicked ; motor.stop ; control.acquire ; motor.access ; dom:114 | forms/fMotorTest.h | btnStopClick uMotorTest.cpp:1647-1659 (6 句) | MOT[].PCIL132_StopMotor StopAllMotor |  |
| HW.MotorTest.html | palExit | Exit | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:76 | forms/fMotorTest.cpp forms/fMotorTest.h | palExitClick uMotorTest.cpp:1727-1730 (1 句) |  |  |
| HW.MotorTest.html | sbMotorTest_JogN | - | `OK-cmd` | clicked ; control.takeover ; motor.access ; motor.stop ; dom:927 | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | sbMotorTest_JogP | + | `OK-cmd` | clicked ; control.takeover ; motor.access ; motor.stop ; http:GET ../JSON/offline/Motor-ru | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | sbMotorTest_MoveN | - | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:968 | forms/fMotorTest.h | sbMotorTest_MoveNClick uMotorTest.cpp:1224-1250 (13 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.MotorTest.html | sbMotorTest_MoveP | + | `OK-cmd` | clicked ; control.takeover ; motor.access ; dom:932 | forms/fMotorTest.h | sbMotorTest_MovePClick uMotorTest.cpp:1252-1278 (13 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.PadInterface.html | btnResetCom | Reset Com | `OK-cmd` | clicked ; pad.get ; dom:2 | — | 沒有 OnClick |  |  |
| HW.PadInterface.html | sb_PadInterface_Exit | Exit | `OK-cmd` | clicked ; pad.get | — | sb_PadInterface_ExitClick uPadInterface.cpp:946-953 (6 句) |  |  |
| HW.PadInterface.html | sb_PadInterface_ManualSend | Send | `OK-cmd` | clicked ; pad.get ; dom:1 | — | sb_PadInterface_ManualSendClick uPadInterface.cpp:385-389 (1 句) |  |  |
| HW.home.html | sbAbortHome | Abort Home | `OK-cmd` | clicked ; act.home.abort ; dom:6 | act.home.abort=yes | sbAbortHomeClick uhome.cpp:4980-4986 (4 句) |  |  |
| HW.teach.html | GoAuto1CassetteFront | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto1CassetteFrontBack | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto1CassetteRear | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto1CassetteRearBack | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto1CassetteZStart | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteFront | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6137 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteFrontBack | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6116 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteRear | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteRearBack | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteZStart | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnAuto4 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnAuto5 | GO | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnAuto6 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnAutoCleanPick2 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBGAView | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:6142 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBGAView_Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer10X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer10Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer1X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer1Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer2X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer2Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer3X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer3Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer4X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer4Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer5X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer5Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer6X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer6Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer7X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer7Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer8X | GO | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.acquire ; motor.access ; contr | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer8Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer9X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer9Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnFix3L | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnFix3R | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnHP1Laser | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6114 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnHP2Laser | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPort1Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPort2Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPort3Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPort4Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPortBufferZ | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPortZ | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadSafeZ | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadTemporaryZ | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnMagZTray1 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; cont | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnMagZTrayStandby | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPADView | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:6142 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPADView_Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPlacePreciser | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPreciserClose | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; cont | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPreciserOpen | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnSafePos | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:6142 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnScannerAOI | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; control.acquire ; motor.access ; do | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto1X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto1Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto2X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto2Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto3X | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto3Z | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedConversionX | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedConversionZ | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedEmptyX | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedEmptyZ | GO | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedLoaderX | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedLoaderZ | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTopView | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:6142 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTopViewKit_Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTopView_Place | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6134 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTrayBracketConversionZ | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTrayBracketSaftZ | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPort1Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPort2Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPort3Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPort4Z | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPortBufferZ | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnYCarPos | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnYOCRPos | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnYSurePos | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton003 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton004 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; cont | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton005 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton006 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6137 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton014 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6121 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton020 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6140 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton024 | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton026 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6140 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton030 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton040 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton042 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6119 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton060 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6116 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton061 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6137 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton062 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; control.acquire ; motor.access  | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton063 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton064 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6118 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton065 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6140 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton066 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton067 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6116 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton068 | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton069 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6146 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton077 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton078 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton080 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6119 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton082 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6119 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton100 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6121 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton102 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton104 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6140 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton120 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton122 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6121 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton124 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6119 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton140 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton141 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton142 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton143 | GO | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton144 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton145 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton146 | GO | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton147 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton153 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6137 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton154 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton155 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6116 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton156 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton160 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton161 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton162 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton163 | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton200 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton201 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton206 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton207 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRA | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRB | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRC | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRD | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRE | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRF | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRG | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6132 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRH | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6111 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX2120 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX240 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX3120 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX340 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX4120 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX440 | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInY15 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInY60 | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonNGBinBox | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRA | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRB | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRC | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRD | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRE | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRF | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRG | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRH | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX2120 | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX240 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX3120 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX340 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX4120 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX440 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutY15 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutY60 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick1 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick2 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick3 | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick4 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick5 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace1 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace2 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace3 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace4 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace5 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace6 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlaceNGBinBox | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; cont | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPreciser | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonRotate | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6121 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonRotateA | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonRotateOut | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonRotateOutA | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6111 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoDecayInButton | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoDecayOutButton | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoInarmPlacementXY | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteFront | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteFrontBack | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteRear | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteRearBack | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteZStart | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | MotorAuto2YCW | Auto 2 CW | `OK-cmd` | clicked ; control.acquire ; motor.access(MAuto1Y) ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX2 | X Pitch 2 | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmPitchX2) ; control.acquire ; motor.access ; | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX3 | X Pitch 3 | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmPitchX3) ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmX | X | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZA | ZA | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmPitch) ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAf | Z Af | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZC | ZC | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZE | ZE | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmZE) ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZG | ZG | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:69 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInSh1 | X | `OK-cmd` | clicked ; control.acquire ; motor.access(MInShuttle1) ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm1Y | Y | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.acquire ; motor.access(MTestY1) ; dom:4 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm2Y | Y | `OK-cmd` | clicked ; control.acquire ; motor.access(MTestY2) ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX4 | X Pitch 4 | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmX | X | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZA | ZA | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZF | ZF | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:69 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorTrayX | X | `OK-cmd` | clicked ; control.acquire ; motor.access(MTrayX) ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | SetAuto1CassetteFront | Auto1 Front | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:27 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto1CassetteFrontBack | Auto1 Front Back | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto1CassetteRear | Auto1 Rear | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto1CassetteRearBack | Auto1 Rear Back | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto1CassetteZStart | Auto1 Cass Z Start | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteFront | Auto2 Front | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteFrontBack | Auto2 Front Back | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteRear | Auto2 Rear | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteRearBack | Auto2 Rear Back | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteZStart | Auto2 Cass Z Start | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnAutoCleanPick2 | AutoClean | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBGAView | BGA View | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:37 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBGAView_Z | BGA View Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer10X | Buffer10 X | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer10Z | Buffer10 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer1X | Buffer1 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer1Z | Buffer1 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer2X | Buffer2 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer2Z | Buffer2 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer3X | Buffer3 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer3Z | Buffer3 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer4X | Buffer4 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer4Z | Buffer4 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer5X | Buffer5 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer5Z | Buffer5 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer6X | Buffer6 X | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer6Z | Buffer6 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer7X | Buffer7 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer7Z | Buffer7 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer8X | Buffer8 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer8Z | Buffer8 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer9X | Buffer9 X | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer9Z | Buffer9 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnHP1Laser | HP 1 Laser | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:51 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnHP2Laser | HP 2 Laser | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPort1Z | Port1 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPort2Z | Port2 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPort3Z | Port3 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPort4Z | Port4 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPortBufferZ | Buffer1 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPortZ | Load Port | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadSafeZ | Safe | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadTemporaryZ | Temporary | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnPADView | Pad View | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:37 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnPADView_Z | Pad View Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnPlacePreciser | Preciser | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnPreciserClose | Preciser | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | SetBtnPreciserOpen | Preciser | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | SetBtnSafePos | Safe Pos | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnScannerAOI | Scanner AOI | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:52 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto1X | Auto1 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; cont | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto1Z | Auto1 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto2X | Auto2 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto2Z | Auto2 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto3X | Auto3 X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto3Z | Auto3 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedConversionX | Conversion X | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedConversionZ | Conversion Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedEmptyX | Empty X | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedEmptyZ | Empty Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedLoaderX | Loader X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedLoaderZ | Loader Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTopView | Top View | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:56 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTopViewKit_Z | AOI WD (kit Down) | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmZAg) ; do | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTopView_Place | Top View Place | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZAg) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTrayBracketConversionZ | Conversion | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTrayBracketSaftZ | Saft | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPort1Z | Port1 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPort2Z | Port2 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPort3Z | Port3 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:50 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPort4Z | Port4 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPortBufferZ | Buffer10 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton014 | Auto Clean | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:30 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton020 | Loader | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton024 | Hot Plate 1 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:56 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton026 | Hot Plate 2 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton040 | Shuttle 1 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:56 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton042 | Shuttle 2 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:37 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton060 | Shuttle1 L | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:32 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton061 | Shuttle1 R | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:29 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton062 | Shuttle2 L | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; control.acquire ; motor.access  | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton063 | Shuttle2 R | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:28 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton064 | Arm1 Y | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:56 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton065 | Arm2 Y | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton066 | In Sht Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton067 | Arm2 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton068 | Wait Test Z Down | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:60 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton069 | Test Z Safe Poision | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton077 | Arm1 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton078 | Arm2 Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton080 | Shuttle 1 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:58 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton082 | Shuttle 2 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:56 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton100 | Auto 1 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton102 | Auto 2 | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton104 | Auto 3 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:56 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton120 | Fix 1 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton122 | Fix 2 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton124 | Fix 3 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton140 | Loader | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton141 | Empty | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton142 | Color | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton143 | Auto 1 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton144 | Auto 2 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton145 | Auto 3 | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton146 | Clean | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton147 | OCR | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton153 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton154 | 180mm | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MOutArmPitchY)  | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton155 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton156 | 180mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; control.acquire ; motor.access  | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton160 | Mapping | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton161 | ID | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton162 | Rotate | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton163 | Rotate | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton200 | OutShuttle1 8 site kit p | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton201 | OutShuttle2 8 site kit p | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton206 | OutShuttle1 One Row kit  | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton207 | OutShuttle2 One Row kit  | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRA | RA | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRB | RB | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRC | RC | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRD | RD | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRE | RE | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRF | RF | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRG | RG | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRH | RH | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX1120 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX140 | 40mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX2120 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX240 | 40mm | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX3120 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX340 | 40mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX4120 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX440 | 40mm | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInY15 | 15mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInY60 | 60mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonNGBinBox | NG Bin Box | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:51 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRA | RA | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRB | RB | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRC | RC | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRD | RD | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRE | RE | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRF | RF | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRG | RG | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRH | RH | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX1120 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX140 | 40mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX2120 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX240 | 40mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX3120 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX340 | 40mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX4120 | 120mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX440 | 40mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutY15 | 15mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutY60 | 60mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick1 | Loader | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick2 | Hot Plate | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick3 | Rotate | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick4 | Shuttle | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick5 | Rotate | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace1 | Fix | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace2 | Auto | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace3 | Shuttle | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace4 | Rotate | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace5 | Rotate | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace6 | Fix2 | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlaceNGBinBox | NG Bin Box | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPreciser | Preciser | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonRotate | Rotate Kit | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:56 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonRotateA | In Rotate | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonRotateOut | Rotate Kit | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonRotateOutA | Out Rotate | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetComputeInSh2 | ComputeInSh2 | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:71 | forms/fTeach.cpp forms/fTeach.h | SetComputeInSh2Click uteach.cpp:5147-5157 (8 句) |  |  |
| HW.teach.html | SetDecayInButton | Decay | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetDecayOutButton | Decay | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:56 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetInarmPlacementXY | Placement | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:58 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteFront | LD Front | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteFrontBack | LD Front Back | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteRear | LD Rear | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteRearBack | LD Rear Back | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteZStart | Cassette Z Start | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SpeedButtonInRotateNeg90 | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6111 | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | SpeedButtonInRotatePos90 | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | SpeedButtonOutRotatepNeg90 | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6111 | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | SpeedButtonOutRotatepPos90 | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnAlarmReset | Alarm Reset | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | — |  |  |
| HW.teach.html | btnArm1YServo | Server ON | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | btnArm1YServoClick uteach.cpp:2919-2940 (13 句) |  |  |
| HW.teach.html | btnArm2YServo | Server ON | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | btnArm1YServoClick uteach.cpp:2919-2940 (13 句) |  |  |
| HW.teach.html | btnAuto4 | Auto 4 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnAuto5 | Auto 5 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnAuto6 | Auto 6 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnBottom2DGo | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6142 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnBottom2DSet | Bottom 2DID | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnGoAutoPlace | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBinBoxZ | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6138 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnAuto4 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6114 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnAuto5 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnAuto6 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnBinBox | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6142 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnFix4 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6119 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnFix5 | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnFix6 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoButton204 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoButton205 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoInSht1BarCode | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoInSht2BarCode | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoLoadXGabage | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoLoadYGabage | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoOutArmToSHT | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6121 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoOutArmToSortShtPlace | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoOutSht1BarCode | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoOutSht2BarCode | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht1Laser | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6115 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht1XGabage | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6135 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht1YGabage | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; control.acquire ; motor.access  | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht2Laser | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht2XGabage | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:6137 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht2YGabage | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmToAuto4 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmToAuto5 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmToAuto6 | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmToSHT | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmXMax | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmXMin | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6137 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortShtL | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortShtPick | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortShtR | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6117 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortZSafeHeight | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoXGabage | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6137 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoYGabage | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnHome | HOME | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | forms/fMotorTest.h | btnHomeClick uteach.cpp:2133-2198 (37 句) | MOT[].ServoOnOff StopAllMotor |  |
| HW.teach.html | btnHomeAll | Home All | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6111 | — | — |  |  |
| HW.teach.html | btnInXPitch2 | In X Pitch 2 | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:29 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInXPitch3 | In X Pitch 3 | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmPitchX3) ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInYPitch | In Y Pitch | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInZAllUp | In Z All Up | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmZH) ; dom:6112 | — | btnInZAllUpClick uteach.cpp:4466-4478 (8 句) |  |  |
| HW.teach.html | btnJogN | JOG N | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; moto | — | 沒有 OnClick |  |  |
| HW.teach.html | btnJogP | JOG P | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | 沒有 OnClick |  |  |
| HW.teach.html | btnMoveN | Move - | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | btnMoveNClick uteach.cpp:2205-2232 (15 句) | MOT[].Gali_MovePR MOT[].MotorMove |  |
| HW.teach.html | btnMoveP | Move + | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | btnMovePClick uteach.cpp:2104-2131 (15 句) | MOT[].Gali_MovePR MOT[].MotorMove |  |
| HW.teach.html | btnMoveTo | Move | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | btnMoveToClick uteach.cpp:2391-2407 (8 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnNeg90InRA | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6132 | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90InRB | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90InRC | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6111 | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90InRD | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90InRE | -90 | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90InRF | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90InRG | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90InRH | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90OutRA | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6109 | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90OutRB | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90OutRC | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90OutRD | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90OutRE | -90 | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90OutRF | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90OutRG | -90 | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.acquire ; motor.access(MInArmX) ; contr | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnNeg90OutRH | -90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnOutXPitch2 | Out X Pitch 2 | `OK-cmd` | clicked ; control.acquire ; motor.access ; dom:69 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutZAllDown | All Down | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | btnOutZAllDownClick uteach.cpp:4542-4562 (16 句) |  |  |
| HW.teach.html | btnOutZAllUp | Out Z All Up | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchX4) ; dom:6111 | — | btnOutZAllUpClick uteach.cpp:4480-4492 (8 句) |  |  |
| HW.teach.html | btnPos90InRA | +90 | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90InRB | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90InRC | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90InRD | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90InRE | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90InRF | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90InRG | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90InRH | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90OutRA | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90OutRB | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6134 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90OutRC | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90OutRD | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6113 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90OutRE | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90OutRF | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6113 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90OutRG | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnPos90OutRH | +90 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6134 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnServo | Servo | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:48 | — | btnServoClick uteach.cpp:4287-4293 (3 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetAllInArmZ | Set InArm Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | btnSetAllInArmZClick uteach.cpp:4300-4365 (42 句) |  |  |
| HW.teach.html | btnSetAllInArmZ_Move | Move | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; cont | — | btnSetAllInArmZ_MoveClick uteach.cpp:5948-5971 (14 句) | MOT[].MotorMove |  |
| HW.teach.html | btnSetAllOutArmZ | Set OutArm Z | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:48 | — | btnSetAllOutArmZClick uteach.cpp:4367-4435 (42 句) |  |  |
| HW.teach.html | btnSetAllOutArmZ_Move | Move | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6111 | — | btnSetAllOutArmZ_MoveClick uteach.cpp:5973-5996 (14 句) | MOT[].MotorMove |  |
| HW.teach.html | btnSetAutoPlace | Auto | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBinBoxZ | Bin Box | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnAuto4 | Auto 4 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; cont | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnAuto5 | Auto 5 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:51 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnAuto6 | Auto 6 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:51 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnBinBox | Bin Box | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnFix4 | Fix 4 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:56 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnFix5 | Fix 5 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:30 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnFix6 | Fix 6 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:30 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetButton204 | In Shuttle1 8 site kit p | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetButton205 | In Shuttle2 8 site kit p | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetInSht1BarCode | In Shuttle 1 Bar Code Po | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetInSht2BarCode | In Shuttle 2 Bar Code Po | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetLoadXGabage | In Arm X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetLoadYGabage | In Arm Y | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetOutArmToSHT | Sort SHT | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetOutArmToSortShtPlace | Auto | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetOutSht1BarCode | Out Shuttle 1 Bar Code P | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetOutSht2BarCode | Out Shuttle 2 Bar Code P | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht1Laser | In Shuttle 1 Laser Pos | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht1XGabage | In Arm X | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht1YGabage | In Arm Y | `OK-cmd` | clicked ; control.takeover ; motor.access(MOutArmPitchY) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht2Laser | In Shuttle 2 Laser Pos | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht2XGabage | In Arm X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht2YGabage | In Arm Y | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmToAuto4 | Auto 4 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmToAuto5 | Auto 5 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmToAuto6 | Auto 6 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmToSHT | Sort SHT | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:34 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmXMax | 40mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmXMin | 13.33mm | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortShtL | Sort Shuttle L | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortShtPick | Sort Shuttle | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortShtR | Sort Shuttle R | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetTo | SET TO | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; dom:24 | forms/fTeach.cpp forms/fTeach.h | btnSetToClick uteach.cpp:2098-2102 (2 句) |  |  |
| HW.teach.html | btnSetXGabage | In Arm X | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetYGabage | In Arm Y | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnStop | STOP | `OK-cmd` | clicked ; motor.stop(MInArmX) ; dom:27 | forms/fMotorTest.h | btnStopClick uteach.cpp:2948-2954 (4 句) | StopAllMotor |  |
| HW.teach.html | btnZ1Servo | Server ON | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:48 | — | btnZ1ServoClick uteach.cpp:4253-4268 (9 句) |  |  |
| HW.teach.html | btnZ2Servo | Server ON | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:27 | — | btnZ2ServoClick uteach.cpp:4270-4285 (9 句) |  |  |
| HW.teach.html | sbAlignInZAa | Aa | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAb | Ab | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAc | Ac | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAd | Ad | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAe | Ae | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAf | Af | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAg | Ag | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAh | Ah | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBa | Ba | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBb | Bb | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBc | Bc | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBd | Bd | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBe | Be | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBf | Bf | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBg | Bg | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBh | Bh | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAa | A | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAb | C | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAc | E | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAd | G | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAe | Ae | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAf | Af | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAg | Ag | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAh | Ah | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBa | B | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBb | D | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:31 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBc | F | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBd | H | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBe | Be | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBf | Bf | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:53 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBg | Bg | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBh | Bh | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:49 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbCatchMagFront | Catch Mag Front | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbCatchMagRear | Catch Mag Rear | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbGoAlignInZAa | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAb | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAc | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAd | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAe | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAf | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAg | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAh | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBa | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6136 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBb | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBc | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBd | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBe | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6116 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBf | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBg | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBh | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAa | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAb | Go | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.acquire ; motor.access ; contr | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAc | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAd | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAe | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAf | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAg | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAh | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBa | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBb | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBc | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBd | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBe | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6137 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBf | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBg | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBh | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6112 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoCatchMagFront | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access ; dom: | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoCatchMagRear | GO | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6133 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoInArmBasePickerPos | Go | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoInArmCCDPos | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoOutArmBasePickerPos | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:6135 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoOutArmCCDPos | Go | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; control.acquire ; motor.access(MInArm | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbInArmBasePickerPos | Base Pick | `OK-cmd` | clicked ; control.acquire ; motor.access(MInArmX) ; control.takeover ; motor.access(MInArm | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbInArmCCDPos | CCD | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:32 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbLoaderYCarPos | Car | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:33 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbLoaderYOCRPos | OCR | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbLoaderYSurePos | Sure | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:52 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbOutArmBasePickerPos | Base Pick | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:35 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbOutArmCCDPos | CCD | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:34 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | setBtnFix3L | Left | `OK-cmd` | clicked ; control.acquire ; motor.access ; control.takeover ; motor.access(MInArmX) ; dom: | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | setBtnFix3R | Right | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:28 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | setBtnMagZTray1 | Magazine 1 | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | setBtnMagZTrayStandby | Standby Pos | `OK-cmd` | clicked ; control.takeover ; motor.access(MInArmX) ; dom:54 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| Main.CommView.html | btnReadZ1 | Read Z1 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:4 | — | btnReadZ1Click main.cpp:21945-21948 (1 句) |  |  |
| Main.CommView.html | btnReadZ2 | Read Z2 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | btnReadZ2Click main.cpp:21950-21953 (1 句) |  |  |
| Main.CommView.html | btnSetZ1 | Set Z1 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | btnSetZ1Click main.cpp:21927-21934 (3 句) |  |  |
| Main.CommView.html | btnSetZ2 | Set Z2 | `OK-cmd` | clicked ; control.acquire ; act.main.indexTorque ; dom:1 | — | btnSetZ2Click main.cpp:21936-21943 (3 句) |  |  |
| Main.Record.html | spbClearRecord | CLEAR | `OK-cmd` | clicked ; control.acquire ; act.main.clearRecord ; dom:3 | act.main.clearRecord=yes act.main.meShuttle2Dbl=yes | spbClearRecordClick main.cpp:30062-30091 (7 句) |  |  |
| Main.gbControlBtn.html | sbStateRecord | State Record | `OK-cmd` | clicked ; control.takeover ; act.main.stateRecord ; dom:6 | control.takeover=yes act.main.stateRecord=yes | sbStateRecordClick main.cpp:26209-26212 (1 句) |  |  |
| Setup.HotPlate.html | spbSave | Save | `OK-cmd` | clicked ; control.acquire ; dom:4 ; err:rejection c12 probe stub | Automation/auto9045.cpp Command.cpp | spbSaveClick cHotPlate.cpp:440-474 (21 句) |  |  |
| Status.Security.html | SecurityExit | Exit | `OK-cmd` | clicked ; control.acquire ; dom:4 | WebLevelSet.cpp cSecurity.cpp | SecurityExitClick cSecurity.cpp:813-816 (1 句) |  |  |
| Status.Security.html | spbExport | Export | `OK-cmd` | clicked ; control.acquire ; security.jam ; dom:4 | control.acquire=yes security.passwd=yes | spbExportClick cSecurity.cpp:1579-1657 (42 句) |  |  |
| Status.Security.html | spbImport | Import | `OK-cmd` | clicked ; control.acquire ; security.jam ; dom:4 | control.acquire=yes security.passwd=yes | spbImportClick cSecurity.cpp:1509-1577 (50 句) |  |  |
| eventlog.html | q | Query | `OK-http` | clicked ; http:POST /api/ela/query ; http:GET /api/ela ; dom:4 | — | — |  |  |
| eventlog.html | saveSum | Save Summary | `OK-http` | clicked ; http:POST /api/ela/summary ; http:GET /api/ela ; dom:4 | — | — |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Setup | Setup | `OK-ui` | clicked ; dom:3 | — | 沒有 OnClick |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Summary | Summary | `OK-ui` | clicked ; dom:5 | — | 沒有 OnClick |  |  |
| HW.IoSetView.html | sbUpdate | Save | `OK-ui` | clicked ; dom:4 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick iosetview.cpp:3183-3321 (86 句) |  |  |
| HW.MotorTest.html | BitBtn1 | Copy From | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | ATC/ATCInterface.cpp ATC/ATCInterface.h | BitBtn1Click uMotorTest.cpp:1384-1401 (14 句) |  |  |
| HW.MotorTest.html | btnAddMotor | Add Motor | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.cpp forms/fMotorTest.h | btnAddMotorClick uMotorTest.cpp:2188-2197 (7 句) |  |  |
| HW.MotorTest.html | btnAlarmReset | Alarm Reset | `OK-ui` | clicked ; dom:114 | — | — |  |  |
| HW.MotorTest.html | btnDeleteMotor | Delete Motor | `OK-ui` | clicked ; dom:111 | forms/fMotorTest.cpp forms/fMotorTest.h | btnDeleteMotorClick uMotorTest.cpp:2199-2212 (10 句) |  |  |
| HW.MotorTest.html | btnHighSpeed | Jog High | `OK-ui` | clicked ; dom:114 | forms/fMotorTest.h | btnHighSpeedClick uMotorTest.cpp:1403-1409 (3 句) |  |  |
| HW.MotorTest.html | btnHomeHigh | Home High | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.h | btnHomeHighClick uMotorTest.cpp:1419-1425 (3 句) |  |  |
| HW.MotorTest.html | btnLoopMove | Loop Move | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.h | btnLoopMoveClick uMotorTest.cpp:1300-1345 (26 句) | MOT[].PCIL132_StopMotor |  |
| HW.MotorTest.html | btnLowSpeed | Jog Low | `OK-ui` | clicked ; dom:112 | forms/fMotorTest.h | btnLowSpeedClick uMotorTest.cpp:1411-1417 (3 句) |  |  |
| HW.MotorTest.html | btnRange | Range | `OK-ui` | clicked ; dom:75 | forms/fMotorTest.cpp forms/fMotorTest.h | btnRangeClick uMotorTest.cpp:1451-1456 (2 句) |  |  |
| HW.MotorTest.html | btnSetPosN | Set Position 2 | `OK-ui` | clicked ; dom:111 | forms/fMotorTest.cpp forms/fMotorTest.h | btnSetPosNClick uMotorTest.cpp:1088-1095 (2 句) |  |  |
| HW.MotorTest.html | btnSetRange | Test Range | `OK-ui` | clicked ; dom:112 | forms/fMotorTest.h | btnSetRangeClick uMotorTest.cpp:1374-1382 (5 句) |  |  |
| HW.MotorTest.html | btnSoftNPos | Soft Neg | `OK-ui` | clicked ; dom:115 | forms/fMotorTest.h | btnSoftNPosClick uMotorTest.cpp:1443-1449 (3 句) |  |  |
| HW.MotorTest.html | btnSoftPPos | Soft Pos | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | forms/fMotorTest.h | btnSoftPPosClick uMotorTest.cpp:1435-1441 (3 句) |  |  |
| HW.MotorTest.html | sbUpdate | Save | `OK-ui` | clicked ; http:GET ../JSON/offline/Motor-runtime.offline.json ; http:GET ../JSON/offline/M | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick uMotorTest.cpp:2240-2268 (20 句) |  |  |
| HW.MotorTest.html | sbtReload | Load Data | `OK-ui` | clicked ; http:GET /api/system/motTable ; dom:112 ; err:rejection GET /api/system/motTable | forms/fMotorTest.cpp forms/fMotorTest.h | sbtReloadClick uMotorTest.cpp:2135-2179 (31 句) |  |  |
| HW.teach.html | MotorAuto1YCW | Auto 1 CW | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX | X Pitch | `OK-ui` | clicked ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX4 | X Pitch 4 | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmY | Y | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAe | Z Ae | `OK-ui` | clicked ; dom:50 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAg | Z Ag | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAh | Z Ah | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZB | ZB | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZD | ZD | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZF | ZF | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZH | ZH | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInSh2 | X | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm1Z | Z | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm2Z | Z | `OK-ui` | clicked ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX | X Pitch | `OK-ui` | clicked ; dom:47 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX2 | X Pitch 2 | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX3 | X Pitch 3 | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmY | Y | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZB | ZB | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZC | ZC | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZD | ZD | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZE | ZE | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZG | ZG | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZH | ZH | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutSh1 | X | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutSh2 | X | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInXPitch1 | In X Pitch | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInXPitch4 | In X Pitch 4 | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnLoaderY | Loader Y | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnMotorTest | Motor Tools | `OK-ui` | clicked ; dom:21 | — | btnMotorTestClick uteach.cpp:2436-2440 (2 句) |  |  |
| HW.teach.html | btnOutXPitch1 | Out X Pitch | `OK-ui` | clicked ; dom:26 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutXPitch3 | Out X Pitch 3 | `OK-ui` | clicked ; dom:27 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutXPitch4 | Out X Pitch 4 | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutYPitch | Out Y Pitch | `OK-ui` | clicked ; dom:48 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnSave | SAVE | `OK-ui` | clicked ; dom:44 | FileRW/Teach.cpp Interface/TesterTCP.cpp | btnSaveClick uteach.cpp:2261-2389 (67 句) |  |  |
| HW.teach.html | btnSetToOffset | SET TO | `OK-ui` | clicked ; dom:43 | forms/fTeach.cpp forms/fTeach.h | btnSetToOffsetClick uteach.cpp:2200-2203 (1 句) |  |  |
| HW.teach.html | pnlExit | EXIT | `OK-ui` | clicked ; dom:21 | — | pnlExitClick uteach.cpp:5026-5032 (3 句) |  |  |
| Main.MotionView.html | axisToggle | 顯示未啟用軸 | `OK-ui` | clicked ; dom:7 | — | — |  |  |
| Setup.BarCode.html | btnTestLotID | Test | `OK-ui` | clicked ; inp:4 ; val:2 | — | btnTestLotIDClick BarCode/BarCode.cpp:11497-11507 (5 句) |  |  |
| Setup.Ld_ULd.html | btnDefaultValue | Default Value | `OK-ui` | clicked ; inp:16 ; val:8 | forms/fLd_ULd.cpp forms/fLd_ULd.h | btnDefaultValueClick cLd_ULd.cpp:224-237 (8 句) |  |  |
| Setup.OffSet.html | btnBack | Back | `OK-ui` | clicked ; dom:10 | FileRW/Offset_File.gen.inc forms/fOffSet.cpp | btnBackClick cOffSet.cpp:3363-3367 (2 句) |  |  |
| Setup.OffSet.html | btnToArmOffset | Go To Index & Tray Arm O | `OK-ui` | clicked ; dom:10 | forms/fOffSet.cpp forms/fOffSet.h | btnToArmOffsetClick cOffSet.cpp:3382-3386 (2 句) |  |  |
| Setup.OffSet.html | btnToIndexOffset | Go To Index & Tray Arm O | `OK-ui` | clicked ; dom:10 | forms/fOffSet.cpp forms/fOffSet.h | btnToIndexOffsetClick cOffSet.cpp:3376-3380 (2 句) |  |  |
| Setup.SetUp.html | btnLDownToRUpZ |  | `OK-ui` | clicked ; val:4 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnLUpToRDownN |  | `OK-ui` | clicked ; val:32 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnLUpToRDownZ |  | `OK-ui` | clicked ; val:4 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnRDownToLUpZ |  | `OK-ui` | clicked ; val:4 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnRUpToLDownZ |  | `OK-ui` | clicked ; val:4 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.Speed.html | spbSelectAll | Select All | `OK-ui` | clicked ; dom:1 ; inp:9 ; val:9 | form.event 表列 | spbSelectAllClick cSpeed.cpp:1795-1810 (13 句) |  |  |
| Setup.Speed.html | spbSetToDef | Set to define | `OK-ui` | clicked ; inp:165 ; val:61 | form.event 表列 | spbSetToDefClick cSpeed.cpp:1812-1933 (63 句) |  |  |
| Setup.Speed.html | spbSpeedAdd | Speed + | `OK-ui` | clicked ; inp:9 ; val:2 | form.event 表列 | spbSpeedAddClick cSpeed.cpp:1414-1419 (3 句) |  |  |
| Setup.Speed.html | spbSpeedDec | Speed - | `OK-ui` | clicked ; inp:3 ; val:2 | form.event 表列 | spbSpeedDecClick cSpeed.cpp:1421-1424 (1 句) |  |  |
| Status.LtcSensor.html | btnClose | Exit | `OK-ui` | clicked ; dom:72 | forms/fIoSetView.h | btnCloseClick LtcSensor.cpp:678-681 (1 句) |  |  |
| eventlog.html | jamSetting | Jam Code Setting | `OK-ui` | clicked ; http:GET /api/ela ; frame:open Status.Security.html ; dom:4 | — | — |  |  |

## 4. 抽 10 顆給筆電複驗

| # | 頁 | id | 分類 | 怎麼複驗 |
|---|---|---|---|---|
| 1 | Setup.Contact.html | spbSave | `dead/D` | 開這一頁按 `spbSave`，看 wb_serve oplog 有沒有收到命令；golden cContact.cpp:14072-14201 |
| 2 | Setup.ContactForce.html | btExit | `dead/D` | 開這一頁按 `btExit`，看 wb_serve oplog 有沒有收到命令；golden ContactForce.cpp:929-933 |
| 3 | Setup.OffSet.html | spbSave | `dead/D` | 開這一頁按 `spbSave`，看 wb_serve oplog 有沒有收到命令；golden cOffSet.cpp:2803-2879 |
| 4 | Setup.SetUp.html | sbUpdate | `dead/D` | 開這一頁按 `sbUpdate`，看 wb_serve oplog 有沒有收到命令；golden cSetUp.cpp:3468-3626 |
| 5 | Config.Configuration.html | btnMesSystem | `dead/B` | 開這一頁按 `btnMesSystem`，看 wb_serve oplog 有沒有收到命令；golden cConfiguration.cpp:7622-7625 |
| 6 | Data.StartCondition.html | btnSetOffsetLimit | `dead/B` | 開這一頁按 `btnSetOffsetLimit`，看 wb_serve oplog 有沒有收到命令；golden cStartCondition.cpp:1234-1249 |
| 7 | HW.MyCCLinkSensor.html | sbExit | `dead/B` | 開這一頁按 `sbExit`，看 wb_serve oplog 有沒有收到命令；golden CCLink/MyCCLinkSensor.cpp:491-521 |
| 8 | Setup.BarCode.html | BtBottom_1_Connect | `dead/B` | 開這一頁按 `BtBottom_1_Connect`，看 wb_serve oplog 有沒有收到命令；golden BarCode/BarCode.cpp:2753-2762 |
| 9 | Setup.SCK_ART.html | btnApplyCount | `dead/B` | 開這一頁按 `btnApplyCount`，看 wb_serve oplog 有沒有收到命令；golden Automation/SCK_ART.cpp:723-798 |
| 10 | Setup.SetUp.html | btAutoShuttlePitch | `dead/B` | 開這一頁按 `btAutoShuttlePitch`，看 wb_serve oplog 有沒有收到命令；golden cSetUp.cpp:4732-4736 |

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

## 11. 探針 v2 實測結果（St01 1005 06:0x～06:13 代跑，St02-E 合併；產生器不寫這節，重跑 `--out-md` 會蓋掉）

- 樹：St01 本機合併 `66c9f29c` ＝ main `bb065426`（第 65 批，含 !187）＋ !193 `1bd7e1b7`；指令同 §10；**自我測試 6 項全過**（ws／dom／dead／trusted／value／covered）；1384 列，0 列「被蓋住」。原始輸出在 `docs/handoff/c12_raw_v2/`（`c12_click_v2.tsv`、`console_notes.txt`）。上面 §0～§8 已是 v2 的數字（golden 0618）。
- 看得到又沒變灰的 430 顆裡，「按了沒反應」**110 → 99 顆**；**dead/A 74 → 63**。
- 從 dead/A 變活的 11 顆（都是 v1 探針讀錯，網頁早就接好）：

| 頁 | 鈕 | v2 量到 | 原因 |
|---|---|---|---|
| Config.DIOInterFaceCFG | spbDelete | `OK-cmd`：`ttlcfg.op` | ④ 只收真的點擊 |
| Setup.Speed | spbSpeedAdd／spbSpeedDec／spbSelectAll／spbSetToDef | `OK-ui`：inp／val | ⑤ 只改輸入框值 |
| Setup.SetUp | btnLUpToRDownN／btnLUpToRDownZ／btnLDownToRUpZ／btnRUpToLDownZ／btnRDownToLUpZ | `OK-ui`：val | ⑤（§10 的候選，確認） |
| Setup.Ld_ULd | btnDefaultValue | `OK-ui`：inp 16／val 8 | ⑤ |

- §10 列過、v2 仍是 dead 的：Temp_Set **btClearAll**（① 要等 `editlistGet('Temperature')` 回資料才掛，假伺服器不回——已知接好）；AGV btInitalLoad／btInitalUnLoad／spbSave（只量到 dom:1，處理器要的資料沒到）；Cleaning btnStartAutoClean／sbTrayAssign／btnResetCleanCount；BarCode／SetUp／Temp_Set 的 sbtExit；SetUp sbUpdate（dead/D）；SetUp **btnRUpToLDownN**——**有接上、不是缺口**（St02-E 1005 07:0x 唯讀查過、St02-M 同意）：六顆排序鈕在 `web/page/ht9045_setup_sitemap.js:1398-1402` 用同一個迴圈綁同一支 `sortSiteMap`（golden cSetUp.cpp:4176-4299 btnLUpToRDownNClick，方向取 dfm Tag，它是 Tag 5）；探針在同一次開頁裡連點六顆、它排第六，它的方向在假伺服器的格子配置下排出的號碼跟前一顆 btnRDownToLUpZ 留下的一樣（golden 原樣的怪處：Visible 看目標格、Enabled 看 [i][j]），所以 val:0。
- **31 顆在 OK-cmd／OK-ui 之間互換**（v1→v2 17 顆、反向 14 顆）：全是 HW.teach／HW.MotorTest 的馬達鈕，差別只在 `control.acquire ; motor.access` 有沒有剛好落在 0.7 秒觀察窗——那是頁面定時續約馬達存取權，不是這顆鈕送的。兩次都算「有反應」，誰該做什麼不受影響。
- **結論**：探針能分辨的 ④⑤ 已排除；剩下的 dead/ 99 顆（dead/A 63）主要是 ①（等 C++ 資料才綁）、②、③——**要在真的 wb_serve 上確認才算缺口**，在那之前這不是待辦清單。

## 12. 探針 v3：非預設分頁上的按鈕第一次被點到（St02-E 1008；產生器不寫這節，重跑 `--out-md` 會蓋掉）

- **為什麼重跑**（INBOX 155，St02-E 1008 13:3x 發現、St02-M 13:4x 核准）：v1／v2 的 1058 列「not-visible」**都是不在預設分頁上、從來沒點過**的按鈕。兩邊都切不到分頁：
  普查器找的是 title 含「: TTabSheet」的上層元素並取它的 id，但頁面產生器的窗格是 `<div class="pcPane" data-p=N title="TabSheet7">`（沒有 id；「: TTabSheet」寫在頁籤上，不是上層）⇒ sheet 欄全空；
  探針找 `.tab[data-tab=...]`，頁面用的是 `.tab[data-t=N]`。
- **工具修正**（同一張 MR，`AI(W906-ST02-C12-TAB)`）：普查器的 sheet＝由外到內的窗格鏈（1905 列裡 1609 列有值）；探針遇到藏起來的 `.pcPane[data-p]` 就由外到內點同一個 `.pcWrap` 裡對應的 `.tab[data-t]`，分頁本身藏起來（多半是 golden dfm TabVisible=False）時記成 `tab-hidden`（不點）→ 新分類 `hidden(tab)`；自我測試多兩顆（c12Tab、c12TabHid），**8／8 全過**。
- **跑法**：STEVEN-NB3（1008 起這台可以跑）、無頭 Edge、只連探針自己的假伺服器；樹＝main `0b55e181`（1008 13:36）匯出的副本；1388 列、exit 0。原始輸出 `docs/handoff/c12_raw_v3/`。合併用 golden 0618。
- **全表**：v2 1859 列 → v3 1905 列（main 新加的按鈕）。看得到又沒變灰的 430 → **1358 顆**；not-visible 1058 → **41**；hidden(tab) **14**；OK 123 → **762**；dead 99 → **378**（dead/A 63→194、dead/B 11→99、dead/C0 14→72、dead/C 0→2、dead/D 5→5、dead/? 5→5）。
- **有變化的頁**（OK／dead／not-visible 的 v2→v3，tab-hidden 是 v3 的新分類）：

| page | OK v2→v3 | dead v2→v3 | not-visible v2→v3 | tab-hidden v3 | greyed v2→v3 |
|---|---|---|---|---|---|
| Alert.Password.html | 0→0 | 2→7 | 5→0 | 0 | 0→0 |
| Config.Configuration.html | 0→0 | 1→45 | 48→1 | 0 | 0→3 |
| Data.Observer.html | 2→35 | 0→0 | 33→0 | 0 | 0→0 |
| Data.SmartDiagnostic.html | 2→2 | 1→1 | 1→0 | 0 | 2→3 |
| Data.StartCondition.html | 0→0 | 4→88 | 84→0 | 0 | 0→0 |
| HW.IoSetView.html | 0→1 | 0→9 | 15→0 | 0 | 0→5 |
| HW.MotorTest.html | 27→35 | 0→0 | 22→0 | 14 | 1→1 |
| HW.MyCCLinkSensor.html | 0→0 | 1→1 | 48→0 | 0 | 25→73 |
| HW.OmronEJ1N.html | 0→0 | 0→0 | 11→0 | 0 | 4→15 |
| HW.PadInterface.html | 0→3 | 0→0 | 0→0 | 0 | 0→0 |
| HW.teach.html | 60→649 | 0→0 | 613→4 | 0 | 23→44 |
| Setup.BarCode.html | 0→1 | 5→82 | 78→0 | 0 | 0→0 |
| Setup.Cleaning.html | 0→0 | 6→8 | 2→0 | 0 | 0→0 |
| Setup.ContactForce.html | 0→0 | 2→3 | 1→0 | 0 | 0→0 |
| Setup.OffSet.html | 1→3 | 15→65 | 52→0 | 0 | 0→0 |
| Setup.Temp_Set.html | 0→0 | 3→8 | 5→0 | 0 | 0→0 |
| Setup.TesterIF.html | 0→0 | 2→3 | 1→0 | 0 | 0→0 |
| Setup.TrayForm.html | 0→0 | 3→4 | 1→0 | 0 | 0→0 |
| Status.Security.html | 1→3 | 0→0 | 6→4 | 0 | 0→0 |

- **D 類**：只剩 §9.1 那 5 顆 Save／Exit（OffSet spbSave、Contact spbSave、SetUp sbUpdate、ContactForce btSave／btExit），一樣要在真的 wb_serve 上確認。原本靜態的 36 顆：HW.teach ±90 的 32 顆現在量到 `OK-cmd`（`motor.access` teachSt02 kind=motion，St02 C9-G2；INBOX 155 1008，`docs/handoff/ST02_INBOX155_D36_20261008.md`）；HW.MotorTest SpeedButton44／45／50／51 落在 `hidden(tab)`——Preasure 分頁照 golden 藏起來（uMotorTest.dfm:1935-1938 TabVisible=False，使用者裁決 R6）。
- **hidden(tab) 14 列**：全是 HW.MotorTest 的 TabSheet7（Preasure）：SpeedButton4～9、44～47、50～53。
- **新量到的 dead/**（主要在 Data.StartCondition 88、Setup.BarCode 82、Setup.OffSet 65、Config.Configuration 45）：§10／§11 的限制照舊——①等 C++ 資料才綁的鈕在假伺服器下看起來沒反應、②FormShow bridge 藏的不會藏、③不在外框裡；所以 **dead/ 要在真的 wb_serve 上確認才算缺口**，這張表不是待辦清單。
- §5 的「請 St01 代跑；STEVEN-NB3 只編譯、不執行」是產生器的舊字，1008 起 STEVEN-NB3 可以跑。
