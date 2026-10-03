# ST02-C12 按鈕普查：「看得到、按了沒反應」（靜態一半；點擊實測欄 UNVERIFIED）

> **golden 基準：906_0625_Steven（`D:/HT9045/HT9011UC_Code_V3.33.906.0_20260625_Steven`）；0618 沒有對照**（STEVEN-NB3 讀不到 0618，NIGHT_REPORT §0 #79／#80）。
> 量測樹：main `5967db16 2026-10-03 09:57:43 +0800`。只量、只分類，**沒有改任何程式**（TO_STEVEN.md §3 ST02-C12）。
> 產生：`HT9011UC_Cpp_V3.33.906.0/tools/webprobe/c12_button_census.py`（靜態）＋ `c12_click_probe.py`（無頭 Edge＋假伺服器，**還沒跑：請 St01 代跑，見 §5**）。每顆的完整欄位在同名 `.tsv`。

## 0. 白話摘要

- 範圍：外框 `web/background.html` WINDOWS 表開得到的頁＋Alert 覆蓋頁，共 64 頁、1859 顆按鈕（golden 的 TButton／TBitBtn／TSpeedButton，加上網頁自己的 `<button>`）。
- 看得到又沒有變灰的：**1461 顆**（其餘：靜態藏起來 254、變灰 132、開發用頁 12）。
- **靜態只能確定兩件事**：①golden 那一側（有沒有處理器、處理器在哪一行、會不會動機台、golden 在這台藏不藏）；②移植樹 C++ 有沒有同名的處理器／form.event 表列。「按下去送了什麼」靜態只能找到「這顆 id 被哪支 script 用到」，命令多半是引擎依資料表組出來的，所以每一顆的分類前面都加 `s:`（暫定），要等 §5 的點擊實測合併才算數。
- 靜態就看得出「**網頁沒有任何一支 script 提到這顆 id**」的：**462 顆**（最可能是真的沒反應，§1 的 s:unbound 列）。

## 1. 分類與建議誰做

| 分類 | 意思 | 顆數 | 建議誰做 |
|---|---|---:|---|
| `s:unbound/D` | 靜態：沒有 script 提到這顆 id，golden 處理器本體會動馬達／寫輸出 | 36 | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| `s:unbound/B` | 靜態：沒有 script 提到這顆 id，C++ 沒有處理器 | 92 | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| `s:unbound/A` | 靜態：沒有 script 提到這顆 id，C++ 有處理器（form.event 表列或同名函式） | 121 | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| `s:unbound/?` | 靜態：沒有 script 提到這顆 id，不是 golden 元件、也沒有處理器線索 | 186 |  |
| `s:unbound/?+E?` | 靜態：沒有 script 提到這顆 id，不是 golden 元件、也沒有處理器線索；WORKLOG_MACHINE §4 有提到（id 或頁） | 27 | 機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| `s:B(web-sends-unknown-cmd)` |  | 1 | 網頁送了 C++ 不認得的命令（對一下命令名；誰的頁誰改） |
| `s:referenced` | 靜態：id 有被 script 用到，按了做什麼要實測 | 234 |  |
| `s:bound-data` | 靜態：id 在資料表裡（例 teach-access），命令由引擎組，要實測 | 573 |  |
| `s:bound-ui` | 靜態：附近只有視窗動作（openWin／close／postMessage），應該是純畫面動作 | 72 |  |
| `s:bound-cmd` | 靜態：附近找得到命令字串、C++ 認得（實測確認） | 28 |  |
| `s:C` | golden dfm Visible=False、程式也不打開 | 3 |  |
| `s:C0` | golden 沒有 OnClick 或處理器是空的 | 88 |  |
| `greyed(runtime-list)` | 網頁執行時變灰並寫原因（例 TEACH_UNWIRED_B38） | 125 |  |
| `greyed(static)` | 網頁 disabled | 7 |  |
| `hidden(static)` | 網頁 display:none／visibility:hidden（不算「看得到」） | 254 |  |
| `dev-page` | IDE.*／ScreenShots 開發用頁，不算 | 12 |  |

分類規則（`c12_button_census.py` classify／dead_cat）：C0 > C > D > A > B；D 只看處理器**本體**（呼叫下去的函式不追），讀 `MOT[i]` 位置不算 D，`MOT[i].Gali_MotMove(...)`、`ServoOnOff(...)`、`SW[..].On()`、`SetOutput*`、`ADAM_Write*`、`fMain->Start()` 算。E 只是「§4 文字裡出現這個 id 或頁名」，要人看。

## 2. 每頁一列

| 頁 | golden dfm | 全部 | 藏 | 灰 | 看得到 | 主要分類（顆數） |
|---|---|---:|---:|---:|---:|---|
| Main.html | main.dfm | 2 | 1 | 0 | 1 | s:unbound/?+E? 1 |
| Data.SortCT.html | cSortCT.dfm | 1 | 0 | 0 | 1 | s:bound-cmd 1 |
| Data.ContactCT.html | cContactCT.dfm | 2 | 0 | 0 | 2 | s:bound-ui 1, s:referenced 1 |
| Data.LotInfo.html | uLotInfo.dfm | 15 | 3 | 3 | 9 | s:bound-cmd 7, s:B(web-sends-unknown-cmd) 1, s:referenced 1 |
| Status.TemperFrom.html | — | 2 | 1 | 0 | 1 | s:referenced 1 |
| Data.Observer.html | cObserver.dfm | 35 | 0 | 0 | 35 | s:referenced 24, s:unbound/A 10, s:bound-ui 1 |
| Status.ShowMessage.html | uShowMessage.dfm | 1 | 0 | 0 | 1 | s:C0 1 |
| Status.ShowBinSelect.html | cShowBinSelect.dfm | 4 | 0 | 0 | 4 | s:bound-cmd 3, s:unbound/A 1 |
| Setup.OffSet.html | cOffSet.dfm | 89 | 21 | 0 | 68 | s:C0 62, s:referenced 5, s:bound-ui 1 |
| Setup.Speed.html | cSpeed.dfm | 6 | 0 | 0 | 6 | s:referenced 5, s:bound-ui 1 |
| HW.IoSetView.html | iosetview.dfm | 17 | 2 | 5 | 10 | s:unbound/A 5, s:referenced 5 |
| Config.Configuration.html | cConfiguration.dfm | 49 | 0 | 3 | 46 | s:unbound/B 15, s:referenced 14, s:unbound/A 10, s:C 3, s:C0 2, s:bound-cmd 1, s:bound-ui 1 |
| Status.CounterSel.html | cCounterSel.dfm | 1 | 0 | 0 | 1 | s:bound-ui 1 |
| Data.CounterClear.html | cCounterClear.dfm | 2 | 0 | 0 | 2 | s:bound-ui 1, s:bound-cmd 1 |
| Data.Builder.html | cBuilder.dfm | 5 | 0 | 0 | 5 | s:referenced 4, s:bound-ui 1 |
| Config.DIOInterFaceCFG.html | DIOInterFaceCFG.dfm | 4 | 0 | 1 | 3 | s:bound-ui 2, s:referenced 1 |
| Status.LtcSensor.html | LtcSensor.dfm | 16 | 1 | 0 | 15 | s:referenced 14, s:bound-ui 1 |
| Status.TowerLight.html | cTowerLight.dfm | 1 | 0 | 0 | 1 | s:bound-ui 1 |
| HW.OmronEJ1N.html | EJ1N/OmronEJ1N.dfm | 15 | 0 | 14 | 1 | s:referenced 1 |
| Setup.QAMode.html | QAMode.dfm | 2 | 0 | 0 | 2 | s:referenced 1, s:bound-ui 1 |
| Setup.BarCode.html | BarCode/BarCode.dfm | 87 | 4 | 0 | 83 | s:unbound/B 72, s:bound-ui 5, s:C0 5, s:unbound/A 1 |
| HW.MyCCLinkSensor.html | CCLink/MyCCLinkSensor.dfm | 80 | 6 | 69 | 5 | s:referenced 2, s:C0 2, s:bound-ui 1 |
| Setup.Cleaning.html | AutoClean/uCleaning.dfm | 8 | 0 | 0 | 8 | s:bound-cmd 4, s:bound-ui 2, s:referenced 1, s:C0 1 |
| Setup.Contact.html | cContact.dfm | 20 | 15 | 1 | 4 | s:bound-ui 3, s:referenced 1 |
| Setup.TesterIF.html | cTesterIF.dfm | 3 | 0 | 0 | 3 | s:bound-ui 2, s:referenced 1 |
| Status.GroundMan.html | GroundMan/GroundMan.dfm | 5 | 0 | 0 | 5 | s:referenced 3, s:bound-ui 1, s:C0 1 |
| Setup.Ld_ULd.html | cLd_ULd.dfm | 3 | 0 | 0 | 3 | s:referenced 2, s:bound-ui 1 |
| Status.Security.html | cSecurity.dfm | 187 | 0 | 0 | 187 | s:unbound/? 180, s:bound-cmd 6, s:bound-ui 1 |
| Setup.TrayForm.html | cTrayForm.dfm | 4 | 0 | 0 | 4 | s:referenced 3, s:bound-ui 1 |
| Setup.SCK_ART.html | Automation/SCK_ART.dfm | 8 | 2 | 0 | 6 | s:unbound/B 4, s:bound-ui 2 |
| Setup.YieldMonitoring.html | uYieldMonitoring.dfm | 3 | 1 | 0 | 2 | s:referenced 1, s:bound-ui 1 |
| Setup.HotPlate.html | cHotPlate.dfm | 2 | 0 | 0 | 2 | s:referenced 1, s:bound-ui 1 |
| Setup.SetUp.html | cSetUp.dfm | 9 | 0 | 0 | 9 | s:referenced 7, s:unbound/B 1, s:bound-ui 1 |
| Data.SmartDiagnostic.html | SmartDiagnostic.dfm | 6 | 0 | 0 | 6 | s:C0 4, s:referenced 2 |
| Data.StartCondition.html | cStartCondition.dfm | 88 | 0 | 0 | 88 | s:unbound/A 79, s:referenced 5, s:bound-ui 3, s:C0 1 |
| Setup.Temp_Set.html | uTemp_Set.dfm | 10 | 2 | 0 | 8 | s:unbound/A 4, s:referenced 3, s:bound-ui 1 |
| Setup.BinSel.html | cBinSel.dfm | 8 | 0 | 0 | 8 | s:bound-ui 8 |
| HW.teach.html | uteach.dfm | 865 | 169 | 29 | 667 | s:bound-data 573, s:referenced 55, s:unbound/D 32, s:bound-ui 5, s:C0 2 |
| HW.MotorTest.html | uMotorTest.dfm | 52 | 2 | 1 | 49 | s:referenced 25, s:bound-ui 8, s:C0 6, s:unbound/A 6, s:unbound/D 4 |
| HW.home.html | uhome.dfm | 2 | 1 | 0 | 1 | s:bound-cmd 1 |
| HW.ShuttleMove.html | ShuttleMove.dfm | 26 | 7 | 0 | 19 | s:referenced 17, s:C0 1, s:bound-ui 1 |
| Setup.TrayAssignment.html | cTrayAssignment.dfm | 2 | 0 | 0 | 2 | s:bound-ui 1, s:referenced 1 |
| Setup.ContactForce.html | ContactForce.dfm | 3 | 0 | 0 | 3 | s:referenced 2, s:bound-ui 1 |
| HW.VacuumUnit.html | VacuumUnit/VacuumUnit.dfm | 7 | 1 | 0 | 6 | s:referenced 5, s:bound-ui 1 |
| Setup.AGV.html | Automation/AGV.dfm | 4 | 0 | 0 | 4 | s:bound-cmd 2, s:referenced 1, s:bound-ui 1 |
| HW.HandlerSys.html | HandlerSys.dfm | 5 | 1 | 0 | 4 | s:referenced 3, s:bound-ui 1 |
| Main.gbControlBtn.html | main.dfm | 1 | 0 | 0 | 1 | s:bound-cmd 1 |
| Main.MotionView.html | — | 1 | 0 | 0 | 1 | s:bound-ui 1 |
| Main.CommView.html | main.dfm | 9 | 0 | 3 | 6 | s:unbound/?+E? 5, s:referenced 1 |
| Main.Record.html | main.dfm | 2 | 1 | 0 | 1 | s:bound-cmd 1 |
| Main.AOAInfo.html | main.dfm | 1 | 0 | 0 | 1 | s:referenced 1 |
| eventlog.html | — | 8 | 0 | 0 | 8 | s:unbound/? 4, s:referenced 3, s:bound-ui 1 |
| testercomm.html | — | 23 | 1 | 0 | 22 | s:unbound/?+E? 21, s:referenced 1 |
| IDE.StyleGuide.html | — | 2 | 0 | 0 | 0 |  |
| IDE.I18nEditor.html | — | 4 | 0 | 0 | 0 |  |
| IDE.WidgetTemplates.html | — | 6 | 0 | 0 | 0 |  |
| HW.TrayEdit.html | uTrayEditForm.dfm | 4 | 1 | 0 | 3 | s:bound-ui 3 |
| Data.FTPClient.html | KYECFTP/FTPClient.dfm | 8 | 2 | 3 | 3 | s:referenced 3 |
| Alert.MotionView.html | — | 1 | 0 | 0 | 1 | s:bound-ui 1 |
| Alert.MotionView9050.html | — | 5 | 0 | 0 | 5 | s:referenced 3, s:unbound/? 2 |
| Alert.MyMessageBox.NonStop.html | — | 1 | 0 | 0 | 1 | s:referenced 1 |
| Alert.Note.NonStop.html | — | 1 | 0 | 0 | 1 | s:referenced 1 |
| Alert.Note.html | note.dfm | 8 | 8 | 0 | 0 |  |
| Alert.Password.html | Password.dfm | 8 | 1 | 0 | 7 | s:unbound/A 5, s:referenced 2 |

## 3. 每顆一列（看得到又沒有變灰的 1461 顆；藏起來／變灰的只在 .tsv）

欄位：頁、id、caption、分類、送出的命令（靜態猜測，UNVERIFIED）、C++ 認不認得、golden 處理器（906_0625_Steven 檔名:行）、會動機台、建議誰做。

| 頁 | id | caption | 分類 | 送出（靜態） | C++ | golden 處理器 | 動機台 | 建議誰做 |
|---|---|---|---|---|---|---|---|---|
| Main.html | （沒有 id ×1） | 🔄 Set … | `s:unbound/?+E?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Status.Security.html | （沒有 id ×180） | 🔑[00] Main - Tools … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| eventlog.html | （沒有 id ×4） | Save … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| testercomm.html | （沒有 id ×21） | Send to Handler … | `s:unbound/?+E?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| Alert.MotionView9050.html | （沒有 id ×2） | P1 … | `s:unbound/?` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |
| HW.MotorTest.html | SpeedButton44 | - | `s:unbound/D` | — | forms/fMotorTest.h | SpeedButton44Click uMotorTest.cpp:1515-1519 (2 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.MotorTest.html | SpeedButton45 | + | `s:unbound/D` | — | forms/fMotorTest.h | SpeedButton45Click uMotorTest.cpp:1521-1525 (2 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.MotorTest.html | SpeedButton50 | - | `s:unbound/D` | — | forms/fMotorTest.h | SpeedButton50Click uMotorTest.cpp:1527-1531 (2 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.MotorTest.html | SpeedButton51 | + | `s:unbound/D` | — | forms/fMotorTest.h | SpeedButton51Click uMotorTest.cpp:1533-1537 (2 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90InRA | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90InRB | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90InRC | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90InRD | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90InRE | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90InRF | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90InRG | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90InRH | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90OutRA | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90OutRB | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90OutRC | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90OutRD | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90OutRE | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90OutRF | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90OutRG | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnNeg90OutRH | -90 | `s:unbound/D` | — | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90InRA | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90InRB | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90InRC | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90InRD | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90InRE | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90InRF | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90InRG | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90InRH | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90OutRA | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90OutRB | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90OutRC | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90OutRD | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90OutRE | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90OutRF | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90OutRG | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| HW.teach.html | btnPos90OutRH | +90 | `s:unbound/D` | — | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove | 列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派 |
| Config.Configuration.html | btnMesSystem | Mes System | `s:unbound/B` | — | — | btnMesSystemClick cConfiguration.cpp:7625-7628 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN14_22Export | Export | `s:unbound/B` | — | — | btnN14_22ExportClick cConfiguration.cpp:7713-7719 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN14_22Import | Import | `s:unbound/B` | — | — | btnN14_22ImportClick cConfiguration.cpp:7721-7727 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN15ESDForm | ESD Form | `s:unbound/B` | — | — | btnN15ESDFormClick cConfiguration.cpp:6703-6706 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN25_3_Manual | Manual | `s:unbound/B` | — | — | btnN25_3_ManualClick cConfiguration.cpp:7740-7743 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN25_4_Manual | Manual | `s:unbound/B` | — | — | btnN25_4_ManualClick cConfiguration.cpp:7745-7748 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN25_5_Manual | Manual | `s:unbound/B` | — | — | btnN25_5_ManualClick cConfiguration.cpp:7750-7753 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN31_Manual | Manual | `s:unbound/B` | — | — | btnN31_ManualClick cConfiguration.cpp:7619-7623 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnN32 | Check Manual | `s:unbound/B` | — | — | btnN32Click cConfiguration.cpp:7630-7640 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnOpenEP | OpenEP | `s:unbound/B` | — | — | btnOpenEPClick cConfiguration.cpp:6641-6670 (22 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnSetIPSCCleaarQty | Set Clear | `s:unbound/B` | — | — | btnSetIPSCCleaarQtyClick cConfiguration.cpp:7157-7169 (6 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | btnSetToTech | Set Offset To Tech | `s:unbound/B` | — | — | btnSetToTechClick cConfiguration.cpp:5981-5984 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | sbReadTemp | Read Temp | `s:unbound/B` | — | — | sbSendTempClick cConfiguration.cpp:5658-5683 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | sbSendTemp | Send Temp Set | `s:unbound/B` | — | — | sbSendTempClick cConfiguration.cpp:5658-5683 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Config.Configuration.html | spbA27 | [A27] Save Standard Conf | `s:unbound/B` | — | — | spbA27Click cConfiguration.cpp:6820-6840 (12 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_1_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_1_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_2_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_2_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_3_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_3_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_4_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_4_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_5_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_5_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_6_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_6_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_7_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_7_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_8_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtBottom_8_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_1A_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_1A_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_1B_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_1B_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_2A_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_2A_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_2B_Connect | Connect | `s:unbound/B` | — | — | BtShuttle_1A_ConnectClick BarCode/BarCode.cpp:2753-2762 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | BtShuttle_2B_Disconnect | Disconnect | `s:unbound/B` | — | — | BtShuttle_1A_DisconnectClick BarCode/BarCode.cpp:2764-2824 (34 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button1 | Get Barcode | `s:unbound/B` | — | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button10 | Initial | `s:unbound/B` | — | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button2 | Initial | `s:unbound/B` | — | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button3 | Get Barcode | `s:unbound/B` | — | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button4 | Initial | `s:unbound/B` | — | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button5 | Get Barcode | `s:unbound/B` | — | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button6 | Initial | `s:unbound/B` | — | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button7 | Get Barcode | `s:unbound/B` | — | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button8 | Initial | `s:unbound/B` | — | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | Button9 | Get Barcode | `s:unbound/B` | — | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBarcodeChangeFileConnect | Connect | `s:unbound/B` | — | — | btBarcodeChangeFileConnectClick BarCode/BarCode.cpp:6119-6127 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBarcodeChangeFileDisConnect | DisConnect | `s:unbound/B` | — | — | btBarcodeChangeFileDisConnectClick BarCode/BarCode.cpp:6129-6139 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBarcodeChangeFileSend | send | `s:unbound/B` | — | — | btBarcodeChangeFileSendClick BarCode/BarCode.cpp:6141-6149 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom1_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom2_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom3_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom4_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom5_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom6_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom7_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btBottom8_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btReader1On | Reader 1 On | `s:unbound/B` | — | — | btReader1OnClick BarCode/BarCode.cpp:2430-2460 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btReader2On | Reader 2 On | `s:unbound/B` | — | — | btReader1OnClick BarCode/BarCode.cpp:2430-2460 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btReader3On | Reader 3 On | `s:unbound/B` | — | — | btReader1OnClick BarCode/BarCode.cpp:2430-2460 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btReader4On | Reader 4 On | `s:unbound/B` | — | — | btReader1OnClick BarCode/BarCode.cpp:2430-2460 (14 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btShuttle_1A_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btShuttle_1B_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btShuttle_2A_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btShuttle_2B_Trigger | Send CMD | `s:unbound/B` | — | — | btShuttle_1A_TriggerClick BarCode/BarCode.cpp:2827-2902 (48 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btStart2DIDCheckSh1 | Cheack 2DID SH1 | `s:unbound/B` | — | — | btStart2DIDCheckSh1Click BarCode/BarCode.cpp:6806-6810 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btStart2DIDCheckSh2 | Cheack 2DID SH2 | `s:unbound/B` | — | — | btStart2DIDCheckSh2Click BarCode/BarCode.cpp:6812-6816 (2 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Auto1 | Connect | `s:unbound/B` | — | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Auto2 | Connect | `s:unbound/B` | — | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Auto3 | Connect | `s:unbound/B` | — | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Fix1 | Connect | `s:unbound/B` | — | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Fix2 | Connect | `s:unbound/B` | — | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Connect_Fix3 | Connect | `s:unbound/B` | — | — | btnClip_Connect_Auto1Click BarCode/BarCode.cpp:11721-11727 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Auto1 | Disconnect | `s:unbound/B` | — | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Auto2 | Disconnect | `s:unbound/B` | — | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Auto3 | Disconnect | `s:unbound/B` | — | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Fix1 | Disconnect | `s:unbound/B` | — | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Fix2 | Disconnect | `s:unbound/B` | — | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_Disconnect_Fix3 | Disconnect | `s:unbound/B` | — | — | btnClip_Disconnect_Auto1Click BarCode/BarCode.cpp:11729-11735 (4 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnClip_SendCmd_Auto1 | Send CMD | `s:unbound/B` | — | — | btnClip_SendCmd_Auto1Click BarCode/BarCode.cpp:11737-11768 (17 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnGPIBReader4 | GPIB Rearder 4 | `s:unbound/B` | — | — | btnGPIBReader4Click BarCode/BarCode.cpp:2462-2472 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnGetBarcode | Get Barcode | `s:unbound/B` | — | — | btnGetBarcodeClick BarCode/BarCode.cpp:11770-11782 (8 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | btnInitialGetBarcode | Initial | `s:unbound/B` | — | — | btnInitialGetBarcodeClick BarCode/BarCode.cpp:11784-11791 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.BarCode.html | spbResetCom | Stop COM | `s:unbound/B` | — | — | spbResetComClick BarCode/BarCode.cpp:2419-2428 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyCount | Apply Count | `s:unbound/B` | — | — | btnApplyCountClick Automation/SCK_ART.cpp:723-798 (30 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyLotInfo | Apply Lot | `s:unbound/B` | — | — | btnApplyLotInfoClick Automation/SCK_ART.cpp:1596-1606 (6 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplyQty | Apply QTY | `s:unbound/B` | — | — | btnApplyQtyClick Automation/SCK_ART.cpp:1521-1530 (5 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SCK_ART.html | btnApplySetting | Apply Setting | `s:unbound/B` | — | — | btnApplySettingClick Automation/SCK_ART.cpp:1532-1594 (40 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Setup.SetUp.html | btAutoShuttlePitch | Auto Shuttle Pitch | `s:unbound/B` | — | — | btAutoShuttlePitchClick cSetUp.cpp:4732-4736 (1 句) |  | C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領） |
| Alert.Password.html | sbPasswordCancel | Cancel | `s:unbound/A` | — | forms/fPassword.cpp forms/fPassword.h | sbPasswordCancelClick Password.cpp:273-276 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordModify | Modify | `s:unbound/A` | — | forms/fPassword.cpp forms/fPassword.h | sbPasswordModifyClick Password.cpp:299-312 (7 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordModifyCancel | Cancel | `s:unbound/A` | — | forms/fPassword.cpp forms/fPassword.h | sbPasswordCancelClick Password.cpp:273-276 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordModifyOK | Save | `s:unbound/A` | — | Password.cpp forms/fPassword.h | sbPasswordModifyOKClick Password.cpp:284-297 (6 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Alert.Password.html | sbPasswordOK | OK | `s:unbound/A` | — | forms/fPassword.cpp forms/fPassword.h | sbPasswordOKClick Password.cpp:278-282 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btHeaterClearSelect | Clear | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btHeaterClearSelectClick cConfiguration.cpp:5480-5487 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btHeaterSelectAll | All | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btHeaterSelectAllClick cConfiguration.cpp:5471-5478 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btN06_TesterList | ... | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btN06_TesterListClick cConfiguration.cpp:6160-6166 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btN06_TesterMap | ... | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btN06_TesterMapClick cConfiguration.cpp:6342-6348 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnAdd1000 | +1000 | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btnAdd1000Click cConfiguration.cpp:5432-5435 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnAdd10000 | +10000 | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btnAdd10000Click cConfiguration.cpp:5976-5979 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnDec1000 | -1000 | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btnDec1000Click cConfiguration.cpp:5437-5440 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnSetIPSCQty | Set Qty | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btnSetIPSCQtyClick cConfiguration.cpp:7151-7155 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | btnSetTo1000 | =1000 | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | btnSetTo1000Click cConfiguration.cpp:5442-5445 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Config.Configuration.html | sbN15UserLevelByTxtReadFilePath |  | `s:unbound/A` | — | cConfiguration.cpp forms/fConfiguration.h | sbN15UserLevelByTxtReadFilePathClick cConfiguration.cpp:6690-6701 (5 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | btOpenLoadLog | Open loader log | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | btOpenLoadLogClick cObserver.cpp:3638-3681 (33 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | btnLot1 | LOT 1 | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | btnLot2 | LOT 2 | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | btnLot3 | LOT 3 | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | btnLot4 | LOT 4 | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | btnLot5 | LOT 5 | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | btnLot1Click cObserver.cpp:5401-5424 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | btnSG_QueryNow | Query Now | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | btnSG_QueryNowClick cObserver.cpp:5361-5364 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | btnSG_QueryYesterday | Query Yesterday | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | btnSG_QueryYesterdayClick cObserver.cpp:5366-5369 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | sbScreenkeyboard | Keyboard | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | sbScreenkeyboardClick cObserver.cpp:4431-4450 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.Observer.html | sbScreenkeyboard2 | Keyboard | `s:unbound/A` | — | cObserver.cpp forms/fObserver.h | sbScreenkeyboardClick cObserver.cpp:4431-4450 (12 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Aa | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ab | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ac | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ad | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ae | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Af | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ag | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ah | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Ba | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bb | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bc | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bd | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Be | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bf | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bg | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm1Bh | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm1AaClick cStartCondition.cpp:1153-1158 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Aa | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ab | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ac | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ad | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ae | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Af | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ag | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ah | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Ba | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bb | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bc | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bd | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Be | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bf | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bg | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnArm2Bh | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnArm2AaClick cStartCondition.cpp:1146-1151 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAa | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAb | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAc | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAd | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAe | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAf | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAg | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearAh | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBa | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBb | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBc | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBd | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBe | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBf | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBg | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearBh | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCa | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCb | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCc | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCd | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCe | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCf | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCg | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearCh | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDa | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDb | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDc | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDd | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDe | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDf | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnClearDg | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnClearAaClick cStartCondition.cpp:1216-1232 (11 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmA | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmB | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmC | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmD | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmE | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmF | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmG | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnInArmH | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnInArmAClick cStartCondition.cpp:1132-1137 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmA | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmB | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmC | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmD | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmE | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmF | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmG | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Data.StartCondition.html | btnOutArmH | clear | `s:unbound/A` | — | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | btnOutArmAClick cStartCondition.cpp:1139-1144 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btComPoet | COM Port Setting | `s:unbound/A` | — | forms/fIoSetView.h | btComPoetClick iosetview.cpp:2831-2836 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btIPSetting | IP Setting | `s:unbound/A` | — | forms/fIoSetView.h | btIPSettingClick iosetview.cpp:2824-2829 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btKVM | KVM Setting | `s:unbound/A` | — | forms/fIoSetView.h | btKVMClick iosetview.cpp:2838-2859 (8 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | btnLatchCheck | Sensor latch check | `s:unbound/A` | — | forms/fIoSetView.h | btnLatchCheckClick iosetview.cpp:2025-2028 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.IoSetView.html | sb_IO_CommunicationPad | Pad | `s:unbound/A` | — | forms/fIoSetView.h | sb_IO_CommunicationPadClick iosetview.cpp:3882-3885 (1 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.MotorTest.html | SpeedButton4 | Set Ref 1 | `s:unbound/A` | — | forms/fATCHandlerSide.h forms/fMotorTest.cpp | SpeedButton4Click uMotorTest.cpp:1465-1469 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.MotorTest.html | SpeedButton5 | Set Ref 2 | `s:unbound/A` | — | forms/fMotorTest.cpp forms/fMotorTest.h | SpeedButton5Click uMotorTest.cpp:1471-1475 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.MotorTest.html | SpeedButton6 | Set Ref 1 | `s:unbound/A` | — | forms/fMotorTest.cpp forms/fMotorTest.h | SpeedButton6Click uMotorTest.cpp:1483-1487 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.MotorTest.html | SpeedButton7 | Set Ref 2 | `s:unbound/A` | — | forms/fMotorTest.cpp forms/fMotorTest.h | SpeedButton7Click uMotorTest.cpp:1489-1493 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.MotorTest.html | SpeedButton8 | Set To Max | `s:unbound/A` | — | forms/fMotorTest.cpp forms/fMotorTest.h | SpeedButton8Click uMotorTest.cpp:1477-1481 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| HW.MotorTest.html | SpeedButton9 | Set To Max | `s:unbound/A` | — | forms/fMotorTest.cpp forms/fMotorTest.h | SpeedButton9Click uMotorTest.cpp:1495-1499 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.BarCode.html | spbStartCom | Start COM | `s:unbound/A` | — | FileRW/TestIF_File_BarCode.gen.inc WebStart.cpp | spbStartComClick BarCode/BarCode.cpp:2504-2510 (2 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btnDefrostEnd | End | `s:unbound/A` | — | forms/fTemp_Set.h uTemp_Set.cpp | btnDefrostEndClick uTemp_Set.cpp:6770-6776 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btnDefrostStart | Start | `s:unbound/A` | — | forms/fTemp_Set.h uTemp_Set.cpp | btnDefrostStartClick uTemp_Set.cpp:6729-6768 (20 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btn_DefrostAllUseEnd | All Use End | `s:unbound/A` | — | forms/fTemp_Set.h uTemp_Set.cpp | btn_DefrostAllUseEndClick uTemp_Set.cpp:6951-6956 (3 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Setup.Temp_Set.html | btn_DefrostAllUseStart | All Use Start | `s:unbound/A` | — | forms/fTemp_Set.h uTemp_Set.cpp | btn_DefrostAllUseStartClick uTemp_Set.cpp:6936-6943 (4 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Status.ShowBinSelect.html | btReturn | return | `s:unbound/A` | — | cShowBinSelect.cpp forms/fShowBinSelect.h | btReturnClick cShowBinSelect.cpp:2244-2251 (5 句) |  | 網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送 |
| Main.CommView.html | btnClearIndexPosLog | Clear | `s:unbound/?+E?` | — | — | btnClearIndexPosLogClick main.cpp:33688-33691 (1 句) |  | 機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| Main.CommView.html | btnRecordSHT | RecordSHT | `s:unbound/?+E?` | — | — | btnRecordSHTClick main.cpp:33676-33680 (0 句) |  | 機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| Main.CommView.html | btnRecordSocket | RecordSocket | `s:unbound/?+E?` | — | — | btnRecordSocketClick main.cpp:33682-33686 (0 句) |  | 機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| Main.CommView.html | btnSaveIndexPosLog | Save | `s:unbound/?+E?` | — | — | btnSaveIndexPosLogClick main.cpp:33660-33674 (6 句) |  | 機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| Main.CommView.html | btnSaveMaxMin | Save Max/Min | `s:unbound/?+E?` | — | — | btnSaveMaxMinClick main.cpp:33693-33710 (10 句) |  | 機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工 |
| Data.LotInfo.html | btChangeFile | Change File | `s:B(web-sends-unknown-cmd)` | lot.barcode.checkByLot lot.barcode.checkByLot.color lot.barcode.changeFile.visible lot.bar | lot.barcode.checkByLot=yes lot.barcode.checkByLot.color=yes  | btChangeFileClick uLotInfo.cpp:10213-10237 (12 句) |  | 網頁送了 C++ 不認得的命令（對一下命令名；誰的頁誰改） |
| Alert.MotionView9050.html | btnPlay | ▶ 播放 | `s:referenced` | refs 3 | — | — |  |  |
| Alert.MotionView9050.html | btnReset | ⟲ 重置 | `s:referenced` | refs 1 | — | — |  |  |
| Alert.MotionView9050.html | btnStep | ⏭ 下一步 | `s:referenced` | refs 1 | — | — |  |  |
| Alert.MyMessageBox.NonStop.html | nsOk | 確認 OK | `s:referenced` | refs 1 | — | — |  |  |
| Alert.Note.NonStop.html | nsOk | 確認 OK | `s:referenced` | refs 1 | — | — |  |  |
| Alert.Password.html | btnOK | OK | `s:referenced` | refs 2 | forms/fPassword.cpp forms/fPassword.h | btnOKClick Password.cpp:385-388 (1 句) |  |  |
| Alert.Password.html | spbCancel | Cancel | `s:referenced` | refs 2 | forms/fPassword.cpp forms/fPassword.h | spbCancelClick Password.cpp:123-128 (3 句) |  |  |
| Config.Configuration.html | btResume | Resume | `s:referenced` | refs 2 | form.event 表列 | btResumeClick cConfiguration.cpp:6154-6158 (2 句) |  |  |
| Config.Configuration.html | btnAddHP | Add HP | `s:referenced` | refs 1 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnAddHPClick cConfiguration.cpp:7017-7027 (8 句) |  |  |
| Config.Configuration.html | btnAddTray | Add Tray | `s:referenced` | refs 1 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnAddTrayClick cConfiguration.cpp:6881-6891 (8 句) |  |  |
| Config.Configuration.html | btnAutoSaveSetAll | Set All | `s:referenced` | refs 1 | cConfiguration.cpp forms/fConfiguration.h | btnAutoSaveSetAllClick cConfiguration.cpp:6334-6340 (4 句) |  |  |
| Config.Configuration.html | btnDeleteHP | Delete HP | `s:referenced` | refs 1 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnDeleteHPClick cConfiguration.cpp:7029-7043 (11 句) |  |  |
| Config.Configuration.html | btnDeleteTray | Delete Tray | `s:referenced` | refs 1 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnDeleteTrayClick cConfiguration.cpp:6893-6907 (11 句) |  |  |
| Config.Configuration.html | btnModifyHP | Modify Data | `s:referenced` | refs 2 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnModifyHPClick cConfiguration.cpp:6992-7015 (10 句) |  |  |
| Config.Configuration.html | btnModifyTray | Modify Data | `s:referenced` | refs 2 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | btnModifyTrayClick cConfiguration.cpp:6854-6879 (10 句) |  |  |
| Config.Configuration.html | btnRecordJamRateByTimeClear | Set All | `s:referenced` | refs 2 | form.event 表列 | btnRecordJamRateByTimeClearClick cConfiguration.cpp:6588-6594 (3 句) |  |  |
| Config.Configuration.html | btnSave | Save Config. | `s:referenced` | refs 2 | FileRW/Teach.cpp Interface/TesterTCP.cpp | btnSaveClick cConfiguration.cpp:7779-7783 (2 句) |  |  |
| Config.Configuration.html | sbUpdateHP | Save | `s:referenced` | refs 1 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | sbUpdateHPClick cConfiguration.cpp:7091-7114 (18 句) |  |  |
| Config.Configuration.html | sbUpdateTray | Save | `s:referenced` | refs 1 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | sbUpdateTrayClick cConfiguration.cpp:6909-6932 (18 句) |  |  |
| Config.Configuration.html | sbtReloadHP | Load Data | `s:referenced` | refs 1 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | sbtReloadHPClick cConfiguration.cpp:7045-7089 (34 句) |  |  |
| Config.Configuration.html | sbtReloadTray | Load Data | `s:referenced` | refs 1 | FileRW/CfgTrayPlate.cpp FileRW/CfgTrayPlate.h | sbtReloadTrayClick cConfiguration.cpp:6934-6978 (34 句) |  |  |
| Config.DIOInterFaceCFG.html | spbSave | Save | `s:referenced` | refs 1 | Automation/auto9045.cpp Command.cpp | spbSaveClick DIOInterFaceCFG.cpp:191-235 (28 句) |  |  |
| Data.Builder.html | btCreateSetupFile | Create | `s:referenced` | refs 3 | WebBuilder.cpp forms/fBuilder.cpp | btCreateSetupFileClick cBuilder.cpp:72-159 (60 句) |  |  |
| Data.Builder.html | btDeleteSetupFile | Delete | `s:referenced` | refs 3 | WebBuilder.cpp forms/fBuilder.cpp | btDeleteSetupFileClick cBuilder.cpp:202-230 (16 句) |  |  |
| Data.Builder.html | spbExport | Export | `s:referenced` | refs 2 | WebBuilder.cpp cSecurity.cpp | spbExportClick cBuilder.cpp:396-473 (50 句) |  |  |
| Data.Builder.html | spbImport | Import | `s:referenced` | refs 2 | WebBuilder.cpp WebSecurityJam.cpp | spbImportClick cBuilder.cpp:475-482 (4 句) |  |  |
| Data.ContactCT.html | btYieldChart | Yield Chart | `s:referenced` | refs 3 | cContactCT.cpp forms/fContactCT.h | btYieldChartClick cContactCT.cpp:1071-1075 (2 句) |  |  |
| Data.FTPClient.html | Button3 | Exit | `s:referenced` | refs 2 | KYECFTP/FTPClientForm_St02.cpp KYECFTP/FTPClientForm_St02.h | Button3Click KYECFTP/FTPClient.cpp:1703-1706 (1 句) |  |  |
| Data.FTPClient.html | btSafeTasterName | Save | `s:referenced` | refs 2 | KYECFTP/FTPClientForm_St02.cpp KYECFTP/FTPClientForm_St02.h | btSafeTasterNameClick KYECFTP/FTPClient.cpp:1868-1952 (48 句) |  |  |
| Data.FTPClient.html | plLoad | Load from HD | `s:referenced` | refs 2 | KYECFTP/FTPClientForm_St02.cpp KYECFTP/FTPClientForm_St02.h | plLoadClick KYECFTP/FTPClient.cpp:1575-1653 (40 句) |  |  |
| Data.LotInfo.html | btTesterTCPShow | Tester TCP Show | `s:referenced` | refs 1 | forms/fLotInfo.cpp forms/fLotInfo.h | btTesterTCPShowClick uLotInfo.cpp:13841-13844 (1 句) |  |  |
| Data.Observer.html | SpeedButton1 | Clear | `s:referenced` | refs 3 | cObserver.cpp forms/fATCHandlerSide.h | SpeedButton1Click cObserver.cpp:865-871 (7 句) |  |  |
| Data.Observer.html | btnBackupLogYear | Backup Log | `s:referenced` | refs 2 | cObserver.cpp forms/fObserver.h | btnBackupLogYearClick cObserver.cpp:5371-5391 (16 句) |  |  |
| Data.Observer.html | btnClearTime | Clear Time Data | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | btnClearTimeClick cObserver.cpp:5393-5399 (4 句) |  |  |
| Data.Observer.html | btnQueryEventLogTxt | Query | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | btnQueryEventLogTxtClick cObserver.cpp:4423-4426 (1 句) |  |  |
| Data.Observer.html | btnTimeData | Query | `s:referenced` | refs 3 | cObserver.cpp forms/fObserver.h | btnQueryEventLogTxtClick cObserver.cpp:4423-4426 (1 句) |  |  |
| Data.Observer.html | cobNoteContentsSet | Add | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | cobNoteContentsSetClick cObserver.cpp:4452-4458 (2 句) |  |  |
| Data.Observer.html | sbCountermeasure | Add | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbCountermeasureClick cObserver.cpp:4552-4556 (1 句) |  |  |
| Data.Observer.html | sbCountermeasureClear | Clear | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbCountermeasureClearClick cObserver.cpp:4564-4567 (1 句) |  |  |
| Data.Observer.html | sbHandlerPrecautionFormShow | Show | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbHandlerPrecautionFormShowClick cObserver.cpp:4473-4492 (10 句) |  |  |
| Data.Observer.html | sbHandlerPrecautionRecordClear | Clear | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbHandlerPrecautionRecordClearClick cObserver.cpp:4467-4471 (1 句) |  |  |
| Data.Observer.html | sbHandlerPrecautionRecordSet | Add | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbHandlerPrecautionRecordSetClick cObserver.cpp:4460-4465 (1 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceDate | 日期: | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceDateClick cObserver.cpp:4535-4538 (1 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceEndTime | 結束時間: | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceEndTimeClick cObserver.cpp:4569-4572 (1 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceSave | Save | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceSaveClick cObserver.cpp:4574-4594 (15 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceSearch | Search | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceSearchClick cObserver.cpp:4596-4678 (45 句) |  |  |
| Data.Observer.html | sbMajorMaintenanceStartTime | 開始時間: | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbMajorMaintenanceStartTimeClick cObserver.cpp:4540-4544 (1 句) |  |  |
| Data.Observer.html | sbPRFinishDate | 結案日期： | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbPRFinishDateClick cObserver.cpp:4523-4527 (2 句) |  |  |
| Data.Observer.html | sbPRStartDate | 開始日期 : | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbPRStartDateClick cObserver.cpp:4529-4533 (2 句) |  |  |
| Data.Observer.html | sbPrecautionSave | Save | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbPrecautionSaveClick cObserver.cpp:4494-4521 (21 句) |  |  |
| Data.Observer.html | sbSearchPrecautionLog | Search | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbSearchPrecautionLogClick cObserver.cpp:4680-4753 (39 句) |  |  |
| Data.Observer.html | sbUndesirablePhenomenon | Add | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbUndesirablePhenomenonClick cObserver.cpp:4546-4550 (1 句) |  |  |
| Data.Observer.html | sbUndesirablePhenomenonClear | Clear | `s:referenced` | refs 1 | cObserver.cpp forms/fObserver.h | sbUndesirablePhenomenonClearClick cObserver.cpp:4558-4562 (1 句) |  |  |
| Data.Observer.html | txtReload | 🔄 重讀 | `s:referenced` | refs 1 | — | — |  |  |
| Data.Observer.html | txtUp | ⬆ 上一層 | `s:referenced` | refs 1 | — | — |  |  |
| Data.SmartDiagnostic.html | sb_SmarDiagnostic_CreateCyliderName | Create Initial Cylider N | `s:referenced` | refs 2 | SmartDiagnostic.cpp WebSmartDiag.cpp | sb_SmarDiagnostic_CreateCyliderNameClick SmartDiagnostic.cpp:594-623 (20 句) |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_ResetRecordCount | Reset Cylider Record | `s:referenced` | refs 2 | WebSmartDiag.cpp forms/fSmartDiagnostic.cpp | sb_SmartDiagnostic_ResetRecordCountClick SmartDiagnostic.cpp:692-724 (24 句) |  |  |
| Data.StartCondition.html | sbClearCount | Clear | `s:referenced` | refs 1 | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbClearCountClick cStartCondition.cpp:607-625 (12 句) |  |  |
| Data.StartCondition.html | sbHeadCondition1Clear | Clear | `s:referenced` | refs 1 | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbHeadCondition1ClearClick cStartCondition.cpp:691-758 (41 句) |  |  |
| Data.StartCondition.html | sbHeadCondition1Save | Save | `s:referenced` | refs 1 | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbHeadCondition1SaveClick cStartCondition.cpp:926-974 (31 句) |  |  |
| Data.StartCondition.html | sbSameAsHead1 | Same As Head 1 | `s:referenced` | refs 1 | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbSameAsHead1Click cStartCondition.cpp:590-605 (10 句) |  |  |
| Data.StartCondition.html | sbSave | Save | `s:referenced` | refs 1 | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | sbSaveClick cStartCondition.cpp:627-658 (20 句) |  |  |
| HW.HandlerSys.html | LoadBtn | Load | `s:referenced` | refs 1 | — | LoadBtnClick HandlerSys.cpp:985-988 (1 句) |  |  |
| HW.HandlerSys.html | SaveBtn | Save | `s:referenced` | refs 1 | FileRW/HSys.cpp FileRW/HSys.gen.inc | SaveBtnClick HandlerSys.cpp:979-983 (2 句) |  |  |
| HW.HandlerSys.html | btnSetATCCom | Set Default | `s:referenced` | refs 2 | — | btnSetATCComClick HandlerSys.cpp:1081-1098 (15 句) |  |  |
| HW.IoSetView.html | btnAddIO | Add IO | `s:referenced` | refs 2 | forms/fIoSetView.h | btnAddIOClick iosetview.cpp:3155-3164 (7 句) |  |  |
| HW.IoSetView.html | btnDeleteIO | Delete IO | `s:referenced` | refs 2 | forms/fIoSetView.h | btnDeleteIOClick iosetview.cpp:3166-3181 (12 句) |  |  |
| HW.IoSetView.html | btnModify | Modify Data | `s:referenced` | refs 2 | forms/fIoSetView.h forms/fMotorTest.cpp | btnModifyClick iosetview.cpp:3123-3141 (8 句) |  |  |
| HW.IoSetView.html | sbUpdate | Save | `s:referenced` | refs 2 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick iosetview.cpp:3183-3321 (86 句) |  |  |
| HW.IoSetView.html | sbtReload | Load Data | `s:referenced` | refs 2 | forms/fDTME08.cpp forms/fDTME08.h | btnReloadClick iosetview.cpp:1252-1266 (9 句) |  |  |
| HW.MotorTest.html | BitBtn1 | Copy From | `s:referenced` | refs 3 | ATC/ATCInterface.cpp ATC/ATCInterface.h | BitBtn1Click uMotorTest.cpp:1384-1401 (14 句) |  |  |
| HW.MotorTest.html | btnAddMotor | Add Motor | `s:referenced` | refs 2 | forms/fMotorTest.cpp forms/fMotorTest.h | btnAddMotorClick uMotorTest.cpp:2188-2197 (7 句) |  |  |
| HW.MotorTest.html | btnDeleteMotor | Delete Motor | `s:referenced` | refs 2 | forms/fMotorTest.cpp forms/fMotorTest.h | btnDeleteMotorClick uMotorTest.cpp:2199-2212 (10 句) |  |  |
| HW.MotorTest.html | btnGo | Go | `s:referenced` | refs 3 | forms/fMotorTest.h | btnGoClick uMotorTest.cpp:1605-1619 (6 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnGoSoftN | Go Soft N Pos | `s:referenced` | refs 3 | forms/fMotorTest.h | btnGoSoftNClick uMotorTest.cpp:1105-1111 (2 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnGoSoftP | Go Soft P Pos | `s:referenced` | refs 3 | forms/fMotorTest.h | btnGoSoftPClick uMotorTest.cpp:1097-1103 (2 句) | MOT[].MotorMove |  |
| HW.MotorTest.html | btnHighSpeed | Jog High | `s:referenced` | refs 3 | forms/fMotorTest.h | btnHighSpeedClick uMotorTest.cpp:1403-1409 (3 句) |  |  |
| HW.MotorTest.html | btnHome | Home Reset | `s:referenced` | refs 15 | forms/fMotorTest.h | btnHomeClick uMotorTest.cpp:1113-1174 (37 句) | MOT[].PCIL132_StopMotor |  |
| HW.MotorTest.html | btnHomeHigh | Home High | `s:referenced` | refs 3 | forms/fMotorTest.h | btnHomeHighClick uMotorTest.cpp:1419-1425 (3 句) |  |  |
| HW.MotorTest.html | btnHomeLow | Home Low | `s:referenced` | refs 3 | forms/fMotorTest.h | btnHomeLowClick uMotorTest.cpp:1427-1433 (3 句) |  |  |
| HW.MotorTest.html | btnLoopMove | Loop Move | `s:referenced` | refs 16 | forms/fMotorTest.h | btnLoopMoveClick uMotorTest.cpp:1300-1345 (26 句) | MOT[].PCIL132_StopMotor |  |
| HW.MotorTest.html | btnLowSpeed | Jog Low | `s:referenced` | refs 3 | forms/fMotorTest.h | btnLowSpeedClick uMotorTest.cpp:1411-1417 (3 句) |  |  |
| HW.MotorTest.html | btnModify | Modify Data | `s:referenced` | refs 2 | forms/fIoSetView.h forms/fMotorTest.cpp | btnModifyClick uMotorTest.cpp:2214-2238 (10 句) |  |  |
| HW.MotorTest.html | btnRange | Range | `s:referenced` | refs 3 | forms/fMotorTest.cpp forms/fMotorTest.h | btnRangeClick uMotorTest.cpp:1451-1456 (2 句) |  |  |
| HW.MotorTest.html | btnReloadMotorData | Reload Motor Data | `s:referenced` | refs 4 | forms/fMotorTest.h | btnReloadMotorDataClick uMotorTest.cpp:1695-1711 (8 句) |  |  |
| HW.MotorTest.html | btnServoOff | Servo Off | `s:referenced` | refs 3 | forms/fMotorTest.h | btnServoOffClick uMotorTest.cpp:1661-1671 (6 句) | MOT[].ServoOnOff |  |
| HW.MotorTest.html | btnSetPosN | Set Position 2 | `s:referenced` | refs 3 | forms/fMotorTest.cpp forms/fMotorTest.h | btnSetPosNClick uMotorTest.cpp:1088-1095 (2 句) |  |  |
| HW.MotorTest.html | btnSetPosP | Set Position 1 | `s:referenced` | refs 3 | forms/fMotorTest.cpp forms/fMotorTest.h | btnSetPosPClick uMotorTest.cpp:1079-1086 (2 句) |  |  |
| HW.MotorTest.html | btnSetRange | Test Range | `s:referenced` | refs 3 | forms/fMotorTest.h | btnSetRangeClick uMotorTest.cpp:1374-1382 (5 句) |  |  |
| HW.MotorTest.html | btnSoftNPos | Soft Neg | `s:referenced` | refs 3 | forms/fMotorTest.h | btnSoftNPosClick uMotorTest.cpp:1443-1449 (3 句) |  |  |
| HW.MotorTest.html | btnSoftPPos | Soft Pos | `s:referenced` | refs 3 | forms/fMotorTest.h | btnSoftPPosClick uMotorTest.cpp:1435-1441 (3 句) |  |  |
| HW.MotorTest.html | sbMotorTest_MoveN | - | `s:referenced` | refs 4 | forms/fMotorTest.h | sbMotorTest_MoveNClick uMotorTest.cpp:1224-1250 (13 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.MotorTest.html | sbMotorTest_MoveP | + | `s:referenced` | refs 4 | forms/fMotorTest.h | sbMotorTest_MovePClick uMotorTest.cpp:1252-1278 (13 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.MotorTest.html | sbUpdate | Save | `s:referenced` | refs 2 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick uMotorTest.cpp:2240-2268 (20 句) |  |  |
| HW.MotorTest.html | sbtReload | Load Data | `s:referenced` | refs 2 | forms/fMotorTest.cpp forms/fMotorTest.h | sbtReloadClick uMotorTest.cpp:2135-2179 (31 句) |  |  |
| HW.MyCCLinkSensor.html | btSave | Save | `s:referenced` | refs 1 | ATC/ATCInterface.cpp ATC/ATCInterface.h | btSaveClick CCLink/MyCCLinkSensor.cpp:2305-2309 (2 句) |  |  |
| HW.MyCCLinkSensor.html | spbSave | Save | `s:referenced` | refs 1 | Automation/auto9045.cpp Command.cpp | spbSaveClick CCLink/MyCCLinkSensor.cpp:1689-1693 (2 句) |  |  |
| HW.OmronEJ1N.html | btSaveData | Save Data | `s:referenced` | refs 1 | — | btSaveDataClick EJ1N/OmronEJ1N.cpp:2028-2031 (1 句) |  |  |
| HW.ShuttleMove.html | btInSH1BarCodePos | In Shuttle 1 Bar Code Po | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btInSH2BarCodePos | In Shuttle 2 Bar Code Po | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btOutSH1OneRowDetectPos | OutShuttle1 One Row kit  | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btOutSH1ZDetectPos | OutShuttle1 8 site kit p | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btOutSH2OneRowDetectPos | OutShuttle2 One Row kit  | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btOutSH2ZDetectPos | OutShuttle2 8 site kit p | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btScanOutShu1 | Out Shuttle 1 | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btScanOutShu2 | Out Shuttle 2 | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btShu1Left | Shuttle1 Left | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btShu1Right | Shuttle1 Right | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btShu2Left | Shuttle2 Left | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btShu2Right | Shuttle2 Right | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btnInSH1Sen7DetectPos | In Shuttle1 8 site kit p | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | btnInSH2Sen7DetectPos | In Shuttle2 8 site kit p | `s:referenced` | refs 1 | — | ShuttleMoveClick ShuttleMove.cpp:1410-1482 (46 句) | fMain->Start |  |
| HW.ShuttleMove.html | sbSensorLatch | Sen. Latch | `s:referenced` | refs 1 | — | sbSensorLatchClick ShuttleMove.cpp:2144-2148 (2 句) |  |  |
| HW.ShuttleMove.html | sbShuttleSensor | Sensor Adj. | `s:referenced` | refs 1 | — | sbShuttleSensorClick ShuttleMove.cpp:1769-1782 (6 句) |  |  |
| HW.ShuttleMove.html | sbUpdate | Save | `s:referenced` | refs 2 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick ShuttleMove.cpp:1965-1994 (20 句) |  |  |
| HW.VacuumUnit.html | btnSetInArm | Set | `s:referenced` | refs 4 | VacuumUnit/VacuumUnit.cpp VacuumUnit/VacuumUnit.h | btnSetInArmClick VacuumUnit/VacuumUnit.cpp:550-595 (35 句) |  |  |
| HW.VacuumUnit.html | btnSetIndexArm | Set | `s:referenced` | refs 4 | VacuumUnit/VacuumUnit.cpp VacuumUnit/VacuumUnit.h | btnSetInArmClick VacuumUnit/VacuumUnit.cpp:550-595 (35 句) |  |  |
| HW.VacuumUnit.html | btnSetOutArm | Set | `s:referenced` | refs 4 | VacuumUnit/VacuumUnit.cpp VacuumUnit/VacuumUnit.h | btnSetInArmClick VacuumUnit/VacuumUnit.cpp:550-595 (35 句) |  |  |
| HW.VacuumUnit.html | sbReset | Reset | `s:referenced` | refs 4 | VacuumUnit/VacuumUnit.cpp VacuumUnit/VacuumUnit.h | sbResetClick VacuumUnit/VacuumUnit.cpp:408-429 (16 句) |  |  |
| HW.VacuumUnit.html | spbSave | Save | `s:referenced` | refs 3 | Automation/auto9045.cpp Command.cpp | spbSaveClick VacuumUnit/VacuumUnit.cpp:378-400 (12 句) |  |  |
| HW.teach.html | MotorAuto1YCCW | Auto 1 CCW | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorAuto1YCW | Auto 1 CW | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorAuto2YCCW | Auto 2 CCW | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorAuto2YCW | Auto 2 CW | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorFix3 | Fix3 | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX | X Pitch | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX2 | X Pitch 2 | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX3 | X Pitch 3 | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmPitchX4 | X Pitch 4 | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInSh1 | X | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInSh2 | X | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm1Y | Y | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm1Z | Z | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm2Y | Y | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorIndexArm2Z | Z | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorLoaderYCCW | Loader CCW | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX | X Pitch | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX2 | X Pitch 2 | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX3 | X Pitch 3 | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmPitchX4 | X Pitch 4 | `s:referenced` | refs 2 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutSh1 | X | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutSh2 | X | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | SetComputeInSh2 | ComputeInSh2 | `s:referenced` | refs 2 | forms/fTeach.cpp forms/fTeach.h | SetComputeInSh2Click uteach.cpp:5147-5157 (8 句) |  |  |
| HW.teach.html | SpeedButtonInRotateNeg90 | -90 | `s:referenced` | refs 1 | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | SpeedButtonInRotatePos90 | +90 | `s:referenced` | refs 1 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | SpeedButtonOutRotatepNeg90 | -90 | `s:referenced` | refs 1 | — | SpeedButtonInRotateNeg90Click uteach.cpp:4657-4707 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | SpeedButtonOutRotatepPos90 | +90 | `s:referenced` | refs 1 | — | SpeedButtonInRotatePos90Click uteach.cpp:4605-4655 (23 句) | MOT[].MotorMove |  |
| HW.teach.html | btnAlarmReset | Alarm Reset | `s:referenced` | refs 5 | — | — |  |  |
| HW.teach.html | btnArm1YServo | Server ON | `s:referenced` | refs 5 | — | btnArm1YServoClick uteach.cpp:2919-2940 (13 句) |  |  |
| HW.teach.html | btnArm2YServo | Server ON | `s:referenced` | refs 5 | — | btnArm1YServoClick uteach.cpp:2919-2940 (13 句) |  |  |
| HW.teach.html | btnInXPitch1 | In X Pitch | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInXPitch2 | In X Pitch 2 | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInXPitch3 | In X Pitch 3 | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInXPitch4 | In X Pitch 4 | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInYPitch | In Y Pitch | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnInZAllUp | In Z All Up | `s:referenced` | refs 4 | — | btnInZAllUpClick uteach.cpp:4466-4478 (8 句) |  |  |
| HW.teach.html | btnLoaderRotZ | Loader Rot Z | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnLoaderY | Loader Y | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnMoveN | Move - | `s:referenced` | refs 4 | — | btnMoveNClick uteach.cpp:2205-2232 (15 句) | MOT[].Gali_MovePR MOT[].MotorMove |  |
| HW.teach.html | btnMoveTo | Move | `s:referenced` | refs 4 | — | btnMoveToClick uteach.cpp:2391-2407 (8 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnOutXPitch1 | Out X Pitch | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutXPitch2 | Out X Pitch 2 | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutXPitch3 | Out X Pitch 3 | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutXPitch4 | Out X Pitch 4 | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutYPitch | Out Y Pitch | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnOutZAllUp | Out Z All Up | `s:referenced` | refs 4 | — | btnOutZAllUpClick uteach.cpp:4480-4492 (8 句) |  |  |
| HW.teach.html | btnSave | SAVE | `s:referenced` | refs 1 | FileRW/Teach.cpp Interface/TesterTCP.cpp | btnSaveClick uteach.cpp:2261-2389 (67 句) |  |  |
| HW.teach.html | btnServo | Servo | `s:referenced` | refs 3 | — | btnServoClick uteach.cpp:4287-4293 (3 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetAllInArmZ_Move | Move | `s:referenced` | refs 1 | — | btnSetAllInArmZ_MoveClick uteach.cpp:5948-5971 (14 句) | MOT[].MotorMove |  |
| HW.teach.html | btnSetAllOutArmZ_Move | Move | `s:referenced` | refs 1 | — | btnSetAllOutArmZ_MoveClick uteach.cpp:5973-5996 (14 句) | MOT[].MotorMove |  |
| HW.teach.html | btnSetTo | SET TO | `s:referenced` | refs 3 | forms/fTeach.cpp forms/fTeach.h | btnSetToClick uteach.cpp:2098-2102 (2 句) |  |  |
| HW.teach.html | btnSetToOffset | SET TO | `s:referenced` | refs 3 | forms/fTeach.cpp forms/fTeach.h | btnSetToOffsetClick uteach.cpp:2200-2203 (1 句) |  |  |
| HW.teach.html | btnSortXPitch | Sort X Pitch | `s:referenced` | refs 3 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | btnZ1Servo | Server ON | `s:referenced` | refs 4 | — | btnZ1ServoClick uteach.cpp:4253-4268 (9 句) |  |  |
| HW.teach.html | btnZ2Servo | Server ON | `s:referenced` | refs 4 | — | btnZ2ServoClick uteach.cpp:4270-4285 (9 句) |  |  |
| Main.AOAInfo.html | OffsetSave | Save | `s:referenced` | refs 3 | FileRW/AOAOffset.cpp FileRW/AOAOffset.gen.inc | OffsetSaveClick main.cpp:33900-33993 (78 句) |  |  |
| Main.CommView.html | btnSetZ1 | Set Z1 | `s:referenced` | refs 1 | — | btnSetZ1Click main.cpp:22005-22012 (3 句) |  |  |
| Setup.AGV.html | spbSave | Save | `s:referenced` | refs 3 | Automation/auto9045.cpp Command.cpp | spbSaveClick Automation/AGV.cpp:928-969 (30 句) |  |  |
| Setup.Cleaning.html | btInclude | Include Tray Data | `s:referenced` | refs 1 | form.event 表列 | btIncludeClick AutoClean/uCleaning.cpp:2248-2262 (10 句) |  |  |
| Setup.Contact.html | spbSave | Save | `s:referenced` | refs 2 | Automation/auto9045.cpp Command.cpp | spbSaveClick cContact.cpp:14072-14201 (71 句) | ADAM_WriteVoltage |  |
| Setup.ContactForce.html | Button1 | Convert | `s:referenced` | refs 1 | ATC/ATCInterface.cpp ATC/ATCInterface.h | Button1Click ContactForce.cpp:1588-1599 (5 句) |  |  |
| Setup.ContactForce.html | btSave | Save | `s:referenced` | refs 3 | ATC/ATCInterface.cpp ATC/ATCInterface.h | btSaveClick ContactForce.cpp:935-961 (19 句) | ADAM_WriteVoltage |  |
| Setup.HotPlate.html | spbSave | Save | `s:referenced` | refs 2 | Automation/auto9045.cpp Command.cpp | spbSaveClick cHotPlate.cpp:440-474 (21 句) |  |  |
| Setup.Ld_ULd.html | btnDefaultValue | Default Value | `s:referenced` | refs 2 | forms/fLd_ULd.cpp forms/fLd_ULd.h | btnDefaultValueClick cLd_ULd.cpp:224-237 (8 句) |  |  |
| Setup.Ld_ULd.html | spbSave | Save | `s:referenced` | refs 1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cLd_ULd.cpp:179-202 (12 句) |  |  |
| Setup.OffSet.html | btnBack | Back | `s:referenced` | refs 1 | FileRW/Offset_File.gen.inc forms/fOffSet.cpp | btnBackClick cOffSet.cpp:3363-3367 (2 句) |  |  |
| Setup.OffSet.html | btnOffsetList | To Offset List | `s:referenced` | refs 1 | forms/fOffSet.h | btnOffsetListClick cOffSet.cpp:3369-3374 (3 句) |  |  |
| Setup.OffSet.html | btnToArmOffset | Go To Index & Tray Arm O | `s:referenced` | refs 1 | forms/fOffSet.cpp forms/fOffSet.h | btnToArmOffsetClick cOffSet.cpp:3382-3386 (2 句) |  |  |
| Setup.OffSet.html | btnToIndexOffset | Go To Index & Tray Arm O | `s:referenced` | refs 2 | forms/fOffSet.cpp forms/fOffSet.h | btnToIndexOffsetClick cOffSet.cpp:3376-3380 (2 句) |  |  |
| Setup.OffSet.html | spbSave | Save | `s:referenced` | refs 2 | Automation/auto9045.cpp Command.cpp | spbSaveClick cOffSet.cpp:2803-2879 (47 句) | fMain->Pause |  |
| Setup.QAMode.html | btnApply | Save | `s:referenced` | refs 2 | Command.cpp FileRW/TestIF_File_QAMode.cpp | btnApplyClick QAMode.cpp:73-110 (26 句) |  |  |
| Setup.SetUp.html | btnLDownToRUpZ |  | `s:referenced` | refs 2 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnLUpToRDownN |  | `s:referenced` | refs 2 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnLUpToRDownZ |  | `s:referenced` | refs 2 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnRDownToLUpZ |  | `s:referenced` | refs 2 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnRUpToLDownN |  | `s:referenced` | refs 2 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | btnRUpToLDownZ |  | `s:referenced` | refs 2 | FileRW/TestIF_File_SetUp.cpp FileRW/TestIF_File_SetUp.gen.in | btnLUpToRDownNClick cSetUp.cpp:4167-4290 (64 句) |  |  |
| Setup.SetUp.html | sbUpdate | Save | `s:referenced` | refs 2 | Command.cpp FileRW/ShuttleMove.cpp | sbUpdateClick cSetUp.cpp:3468-3626 (77 句) | ADAM_WriteVoltage |  |
| Setup.Speed.html | spbSave | Save | `s:referenced` | refs 1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cSpeed.cpp:1433-1786 (200 句) |  |  |
| Setup.Speed.html | spbSelectAll | Select All | `s:referenced` | refs 2 | form.event 表列 | spbSelectAllClick cSpeed.cpp:1795-1810 (13 句) |  |  |
| Setup.Speed.html | spbSetToDef | Set to define | `s:referenced` | refs 2 | form.event 表列 | spbSetToDefClick cSpeed.cpp:1812-1933 (63 句) |  |  |
| Setup.Speed.html | spbSpeedAdd | Speed + | `s:referenced` | refs 3 | form.event 表列 | spbSpeedAddClick cSpeed.cpp:1414-1419 (3 句) |  |  |
| Setup.Speed.html | spbSpeedDec | Speed - | `s:referenced` | refs 3 | form.event 表列 | spbSpeedDecClick cSpeed.cpp:1421-1424 (1 句) |  |  |
| Setup.Temp_Set.html | btClearAll | Clear All | `s:referenced` | refs 2 | forms/fTemp_Set.h uTemp_Set.cpp | btClearAllClick uTemp_Set.cpp:5126-5135 (7 句) |  |  |
| Setup.Temp_Set.html | btnSameAsArm1 | Same as Arm1 | `s:referenced` | refs 1 | forms/fTemp_Set.h uTemp_Set.cpp | btnSameAsArm1Click uTemp_Set.cpp:6177-6186 (7 句) |  |  |
| Setup.Temp_Set.html | spbSave | Save | `s:referenced` | refs 1 | Automation/auto9045.cpp Command.cpp | spbSaveClick uTemp_Set.cpp:4201-4528 (167 句) |  |  |
| Setup.TesterIF.html | spbSave | Save | `s:referenced` | refs 1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTesterIF.cpp:1302-1356 (32 句) |  |  |
| Setup.TrayAssignment.html | spbSave | Save | `s:referenced` | refs 1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTrayAssignment.cpp:1254-1316 (30 句) |  |  |
| Setup.TrayForm.html | btnBinBoxReset | Reset | `s:referenced` | refs 1 | form.event 表列 | btnBinBoxResetClick cTrayForm.cpp:728-733 (3 句) |  |  |
| Setup.TrayForm.html | spbCopy | Copy From | `s:referenced` | refs 2 | — | spbCopyClick cTrayForm.cpp:692-713 (17 句) |  |  |
| Setup.TrayForm.html | spbSave | Save | `s:referenced` | refs 1 | Automation/auto9045.cpp Command.cpp | spbSaveClick cTrayForm.cpp:610-638 (15 句) |  |  |
| Setup.YieldMonitoring.html | btnApply | Save | `s:referenced` | refs 3 | Command.cpp FileRW/TestIF_File_QAMode.cpp | btnApplyClick uYieldMonitoring.cpp:3064-3138 (39 句) |  |  |
| Status.GroundMan.html | spbSave | Save | `s:referenced` | refs 3 | Automation/auto9045.cpp Command.cpp | spbSaveClick GroundMan/GroundMan.cpp:1418-1468 (24 句) |  |  |
| Status.GroundMan.html | spbStartCom | Start COM | `s:referenced` | refs 1 | FileRW/TestIF_File_BarCode.gen.inc WebStart.cpp | spbStartComClick GroundMan/GroundMan.cpp:247-251 (2 句) |  |  |
| Status.GroundMan.html | spbStopCom | Stop COM | `s:referenced` | refs 1 | — | spbStopComClick GroundMan/GroundMan.cpp:253-257 (2 句) |  |  |
| Status.LtcSensor.html | Button5 | Get | `s:referenced` | refs 1 | — | btGetSh1LtcClick LtcSensor.cpp:482-554 (46 句) |  |  |
| Status.LtcSensor.html | Button6 | Get | `s:referenced` | refs 1 | — | btGetSh2LtcClick LtcSensor.cpp:556-628 (46 句) |  |  |
| Status.LtcSensor.html | btGetInSh1YLtc | Get | `s:referenced` | refs 1 | — | btGetSh1LtcClick LtcSensor.cpp:482-554 (46 句) |  |  |
| Status.LtcSensor.html | btGetInSh2YLtc | Get | `s:referenced` | refs 1 | — | btGetSh2LtcClick LtcSensor.cpp:556-628 (46 句) |  |  |
| Status.LtcSensor.html | btGetSh1Ltc | Get | `s:referenced` | refs 1 | — | btGetSh1LtcClick LtcSensor.cpp:482-554 (46 句) |  |  |
| Status.LtcSensor.html | btGetSh1YLtc | Get | `s:referenced` | refs 1 | — | btGetSh1LtcClick LtcSensor.cpp:482-554 (46 句) |  |  |
| Status.LtcSensor.html | btGetSh2Ltc | Get | `s:referenced` | refs 1 | — | btGetSh2LtcClick LtcSensor.cpp:556-628 (46 句) |  |  |
| Status.LtcSensor.html | btGetSh2YLtc | Get | `s:referenced` | refs 1 | — | btGetSh2LtcClick LtcSensor.cpp:556-628 (46 句) |  |  |
| Status.LtcSensor.html | btGetSh3Ltc | Get | `s:referenced` | refs 1 | — | btGetSh3LtcClick LtcSensor.cpp:630-676 (33 句) |  |  |
| Status.LtcSensor.html | btGetSh3YLtc | Get | `s:referenced` | refs 1 | — | btGetSh3LtcClick LtcSensor.cpp:630-676 (33 句) |  |  |
| Status.LtcSensor.html | btSetLtc | Set | `s:referenced` | refs 1 | — | btSetLtcClick LtcSensor.cpp:123-128 (3 句) |  |  |
| Status.LtcSensor.html | btSh1Servo | Shuttle1 Servo | `s:referenced` | refs 1 | — | btSh1ServoClick LtcSensor.cpp:105-109 (2 句) | MOT[].ServoOnOff |  |
| Status.LtcSensor.html | btSh2Servo | Shuttle2 Servo | `s:referenced` | refs 1 | — | btSh2ServoClick LtcSensor.cpp:111-115 (2 句) | MOT[].ServoOnOff |  |
| Status.LtcSensor.html | btSh3Servo | Shuttle3 Servo | `s:referenced` | refs 2 | — | btSh3ServoClick LtcSensor.cpp:117-121 (2 句) | MOT[].ServoOnOff |  |
| Status.TemperFrom.html | tzExpand | Expand all ▸ | `s:referenced` | refs 2 | — | — |  |  |
| eventlog.html | oeeMonth | 本月 | `s:referenced` | refs 1 | — | — |  |  |
| eventlog.html | q | Query | `s:referenced` | refs 2 | — | — |  |  |
| eventlog.html | saveSum | Save Summary | `s:referenced` | refs 1 | — | — |  |  |
| testercomm.html | g-spbAutoRetest | Dummy ART | `s:referenced` | refs 2 | — | — |  |  |
| HW.teach.html | GoAuto1CassetteFront | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto1CassetteFrontBack | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto1CassetteRear | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto1CassetteRearBack | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto1CassetteZStart | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteFront | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteFrontBack | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteRear | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteRearBack | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoAuto2CassetteZStart | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnAuto4 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnAuto5 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnAuto6 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnAutoCleanPick2 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBGAView | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBGAView_Z | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer10X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer10Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer1X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer1Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer2X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer2Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer3X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer3Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer4X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer4Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer5X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer5Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer6X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer6Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer7X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer7Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer8X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer8Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer9X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnBuffer9Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnFix3L | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnFix3R | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnHP1Laser | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnHP2Laser | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPort1Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPort2Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPort3Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPort4Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPortBufferZ | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadPortZ | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadSafeZ | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnLoadTemporaryZ | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnMagZTray1 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnMagZTrayStandby | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPADView | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPADView_Z | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPlacePreciser | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPreciserClose | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnPreciserOpen | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnSafePos | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnScannerAOI | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto1X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto1Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto2X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto2Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto3X | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedAuto3Z | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedConversionX | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedConversionZ | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedEmptyX | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedEmptyZ | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedLoaderX | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnStackedLoaderZ | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTopView | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTopViewKit_Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTopView_Place | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTrayBracketConversionZ | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnTrayBracketSaftZ | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPort1Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPort2Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPort3Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPort4Z | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnUnloadPortBufferZ | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnYCarPos | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnYOCRPos | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoBtnYSurePos | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton003 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton004 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton005 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton006 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton014 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton020 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton024 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton026 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton030 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton040 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton042 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton060 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton061 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton062 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton063 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton064 | GO | `s:bound-data` | refs 2 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton065 | GO | `s:bound-data` | refs 2 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton066 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton067 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton068 | Go | `s:bound-data` | refs 5 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton069 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton077 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton078 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton080 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton082 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton100 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton102 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton104 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton120 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton122 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton124 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton140 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton141 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton142 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton143 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton144 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton145 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton146 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton147 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton153 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton154 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton155 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton156 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton160 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton161 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton162 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton163 | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton200 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton201 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton206 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButton207 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRA | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRB | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRC | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRD | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRE | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRF | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRG | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInRH | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX2120 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX240 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX3120 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX340 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX4120 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInX440 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInY15 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonInY60 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonNGBinBox | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRA | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRB | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRC | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRD | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRE | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRF | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRG | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutRH | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX2120 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX240 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX3120 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX340 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX4120 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutX440 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutY15 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonOutY60 | Go | `s:bound-data` | refs 2 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick1 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick2 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick3 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick4 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPick5 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace1 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace2 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace3 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace4 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace5 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlace6 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPlaceNGBinBox | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonPreciser | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonRotate | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonRotateA | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonRotateOut | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoButtonRotateOutA | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoDecayInButton | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoDecayOutButton | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoInarmPlacementXY | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteFront | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteFrontBack | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteRear | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteRearBack | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | GoLoaderCassetteZStart | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | MotorInArmX | X | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmY | Y | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZA | ZA | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAe | Z Ae | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAf | Z Af | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAg | Z Ag | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZAh | Z Ah | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZB | ZB | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZBe | Z Be | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZBf | Z Bf | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZBg | Z Bg | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZBh | Z Bh | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZC | ZC | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZD | ZD | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZE | ZE | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZF | ZF | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZG | ZG | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorInArmZH | ZH | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmX | X | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmY | Y | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZA | ZA | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZAe | Z Ae | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZAf | Z Af | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZAg | Z Ag | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZAh | Z Ah | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZB | ZB | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZBe | Z Be | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZBf | Z Bf | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZBg | Z Bg | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZBh | Z Bh | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZC | ZC | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZD | ZD | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZE | ZE | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZF | ZF | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZG | ZG | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorOutArmZH | ZH | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorTopAOIArmR | Seat R | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorTopAOIArmX | Seat X | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorTopAOIArmY | Seat Y | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorTopAOICCDZ | Seat Z | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | MotorTrayX | X | `s:bound-data` | refs 1 | — | MotorTrayXClick uteach.cpp:3630-3648 (10 句) |  |  |
| HW.teach.html | SetAuto1CassetteFront | Auto1 Front | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto1CassetteFrontBack | Auto1 Front Back | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto1CassetteRear | Auto1 Rear | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto1CassetteRearBack | Auto1 Rear Back | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto1CassetteZStart | Auto1 Cass Z Start | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteFront | Auto2 Front | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteFrontBack | Auto2 Front Back | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteRear | Auto2 Rear | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteRearBack | Auto2 Rear Back | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetAuto2CassetteZStart | Auto2 Cass Z Start | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnAutoCleanPick2 | AutoClean | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBGAView | BGA View | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBGAView_Z | BGA View Z | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer10X | Buffer10 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer10Z | Buffer10 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer1X | Buffer1 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer1Z | Buffer1 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer2X | Buffer2 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer2Z | Buffer2 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer3X | Buffer3 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer3Z | Buffer3 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer4X | Buffer4 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer4Z | Buffer4 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer5X | Buffer5 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer5Z | Buffer5 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer6X | Buffer6 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer6Z | Buffer6 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer7X | Buffer7 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer7Z | Buffer7 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer8X | Buffer8 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer8Z | Buffer8 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer9X | Buffer9 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnBuffer9Z | Buffer9 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnHP1Laser | HP 1 Laser | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnHP2Laser | HP 2 Laser | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPort1Z | Port1 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPort2Z | Port2 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPort3Z | Port3 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPort4Z | Port4 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPortBufferZ | Buffer1 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadPortZ | Load Port | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadSafeZ | Safe | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnLoadTemporaryZ | Temporary | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnPADView | Pad View | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnPADView_Z | Pad View Z | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnPlacePreciser | Preciser | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnPreciserClose | Preciser | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | SetBtnPreciserOpen | Preciser | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | SetBtnSafePos | Safe Pos | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnScannerAOI | Scanner AOI | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto1X | Auto1 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto1Z | Auto1 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto2X | Auto2 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto2Z | Auto2 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto3X | Auto3 X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedAuto3Z | Auto3 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedConversionX | Conversion X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedConversionZ | Conversion Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedEmptyX | Empty X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedEmptyZ | Empty Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedLoaderX | Loader X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnStackedLoaderZ | Loader Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTopView | Top View | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTopViewKit_Z | AOI WD (kit Down) | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTopView_Place | Top View Place | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTrayBracketConversionZ | Conversion | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnTrayBracketSaftZ | Saft | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPort1Z | Port1 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPort2Z | Port2 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPort3Z | Port3 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPort4Z | Port4 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetBtnUnloadPortBufferZ | Buffer10 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton014 | Auto Clean | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton020 | Loader | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton024 | Hot Plate 1 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton026 | Hot Plate 2 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton040 | Shuttle 1 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton042 | Shuttle 2 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton060 | Shuttle1 L | `s:bound-data` | refs 1 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton061 | Shuttle1 R | `s:bound-data` | refs 1 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton062 | Shuttle2 L | `s:bound-data` | refs 1 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton063 | Shuttle2 R | `s:bound-data` | refs 1 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton064 | Arm1 Y | `s:bound-data` | refs 2 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton065 | Arm2 Y | `s:bound-data` | refs 2 | — | SetButton064Click uteach.cpp:3650-3695 (29 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton066 | Arm1 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton067 | Arm2 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton068 | Wait Test Z Down | `s:bound-data` | refs 5 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton069 | Test Z Safe Poision | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton077 | Arm1 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton078 | Arm2 Z | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton080 | Shuttle 1 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton082 | Shuttle 2 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton100 | Auto 1 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton102 | Auto 2 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton104 | Auto 3 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton120 | Fix 1 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton122 | Fix 2 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton124 | Fix 3 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton140 | Loader | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton141 | Empty | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton142 | Color | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton143 | Auto 1 | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton144 | Auto 2 | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton145 | Auto 3 | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton146 | Clean | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton147 | OCR | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton153 | 120mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton154 | 180mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton155 | 120mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton156 | 180mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton160 | Mapping | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton161 | ID | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton162 | Rotate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton163 | Rotate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton200 | OutShuttle1 8 site kit p | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton201 | OutShuttle2 8 site kit p | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton206 | OutShuttle1 One Row kit  | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButton207 | OutShuttle2 One Row kit  | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRA | RA | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRB | RB | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRC | RC | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRD | RD | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRE | RE | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRF | RF | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRG | RG | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInRH | RH | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX1120 | 120mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX140 | 40mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX2120 | 120mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX240 | 40mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX3120 | 120mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX340 | 40mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX4120 | 120mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInX440 | 40mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInY15 | 15mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonInY60 | 60mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonNGBinBox | NG Bin Box | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRA | RA | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRB | RB | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRC | RC | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRD | RD | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRE | RE | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRF | RF | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRG | RG | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutRH | RH | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX1120 | 120mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX140 | 40mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX2120 | 120mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX240 | 40mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX3120 | 120mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX340 | 40mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX4120 | 120mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutX440 | 40mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutY15 | 15mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonOutY60 | 60mm | `s:bound-data` | refs 2 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick1 | Loader | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick2 | Hot Plate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick3 | Rotate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick4 | Shuttle | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPick5 | Rotate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace1 | Fix | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace2 | Auto | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace3 | Shuttle | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace4 | Rotate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace5 | Rotate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlace6 | Fix2 | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPlaceNGBinBox | NG Bin Box | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonPreciser | Preciser | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonRotate | Rotate Kit | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonRotateA | In Rotate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonRotateOut | Rotate Kit | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetButtonRotateOutA | Out Rotate | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetDecayInButton | Decay | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetDecayOutButton | Decay | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetInarmPlacementXY | Placement | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteFront | LD Front | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteFrontBack | LD Front Back | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteRear | LD Rear | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteRearBack | LD Rear Back | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | SetLoaderCassetteZStart | Cassette Z Start | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnAuto4 | Auto 4 | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnAuto5 | Auto 5 | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnAuto6 | Auto 6 | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnBottom2DGo | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnBottom2DSet | Bottom 2DID | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnGoAutoPlace | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBinBoxZ | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnAuto4 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnAuto5 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnAuto6 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnBinBox | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnFix4 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnFix5 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoBtnFix6 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoButton204 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoButton205 | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoInSht1BarCode | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoInSht2BarCode | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoLoadXGabage | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoLoadYGabage | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoOutArmToSHT | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoOutArmToSortShtPlace | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoOutSht1BarCode | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoOutSht2BarCode | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht1Laser | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht1XGabage | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht1YGabage | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht2Laser | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht2XGabage | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSht2YGabage | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmToAuto4 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmToAuto5 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmToAuto6 | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmToSHT | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmXMax | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortArmXMin | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortShtL | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortShtPick | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortShtR | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoSortZSafeHeight | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoXGabage | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnGoYGabage | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | btnSetAutoPlace | Auto | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBinBoxZ | Bin Box | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnAuto4 | Auto 4 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnAuto5 | Auto 5 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnAuto6 | Auto 6 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnBinBox | Bin Box | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnFix4 | Fix 4 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnFix5 | Fix 5 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetBtnFix6 | Fix 6 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetButton204 | In Shuttle1 8 site kit p | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetButton205 | In Shuttle2 8 site kit p | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetInSht1BarCode | In Shuttle 1 Bar Code Po | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetInSht2BarCode | In Shuttle 2 Bar Code Po | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetLoadXGabage | In Arm X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetLoadYGabage | In Arm Y | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetOutArmToSHT | Sort SHT | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetOutArmToSortShtPlace | Auto | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetOutSht1BarCode | Out Shuttle 1 Bar Code P | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetOutSht2BarCode | Out Shuttle 2 Bar Code P | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht1Laser | In Shuttle 1 Laser Pos | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht1XGabage | In Arm X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht1YGabage | In Arm Y | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht2Laser | In Shuttle 2 Laser Pos | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht2XGabage | In Arm X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSht2YGabage | In Arm Y | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmToAuto4 | Auto 4 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmToAuto5 | Auto 5 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmToAuto6 | Auto 6 | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmToSHT | Sort SHT | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmXMax | 40mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortArmXMin | 13.33mm | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortShtL | Sort Shuttle L | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortShtPick | Sort Shuttle | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetSortShtR | Sort Shuttle R | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetXGabage | In Arm X | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | btnSetYGabage | In Arm Y | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAa | Aa | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAb | Ab | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAc | Ac | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAd | Ad | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAe | Ae | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAf | Af | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAg | Ag | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZAh | Ah | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBa | Ba | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBb | Bb | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBc | Bc | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBd | Bd | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBe | Be | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBf | Bf | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBg | Bg | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignInZBh | Bh | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAa | A | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAb | C | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAc | E | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAd | G | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAe | Ae | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAf | Af | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAg | Ag | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZAh | Ah | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBa | B | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBb | D | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBc | F | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBd | H | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBe | Be | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBf | Bf | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBg | Bg | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbAlignOutZBh | Bh | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbCatchMagFront | Catch Mag Front | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbCatchMagRear | Catch Mag Rear | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbGoAlignInZAa | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAb | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAc | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAd | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAe | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAf | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAg | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZAh | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBa | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBb | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBc | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBd | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBe | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBf | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBg | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignInZBh | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAa | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAb | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAc | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAd | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAe | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAf | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAg | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZAh | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBa | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBb | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBc | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBd | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBe | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBf | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBg | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoAlignOutZBh | Go | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoCatchMagFront | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoCatchMagRear | GO | `s:bound-data` | refs 1 | — | GoButton140Click uteach.cpp:3401-3472 (40 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoInArmBasePickerPos | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoInArmCCDPos | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoOutArmBasePickerPos | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbGoOutArmCCDPos | Go | `s:bound-data` | refs 1 | — | GoButton020Click uteach.cpp:3559-3623 (37 句) | MOT[].Gali_MotMove MOT[].MotorMove |  |
| HW.teach.html | sbInArmBasePickerPos | Base Pick | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbInArmCCDPos | CCD | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbLoaderYCarPos | Car | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbLoaderYOCRPos | OCR | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbLoaderYSurePos | Sure | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbOutArmBasePickerPos | Base Pick | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | sbOutArmCCDPos | CCD | `s:bound-data` | refs 1 | — | SetButton020Click uteach.cpp:3517-3557 (28 句) | MOT[].ServoOnOff |  |
| HW.teach.html | setBtnFix3L | Left | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | setBtnFix3R | Right | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | setBtnMagZTray1 | Magazine 1 | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| HW.teach.html | setBtnMagZTrayStandby | Standby Pos | `s:bound-data` | refs 1 | — | SetButton140Click uteach.cpp:3360-3399 (23 句) | MOT[].ServoOnOff |  |
| Alert.MotionView.html | axisToggle | 顯示未啟用軸 | `s:bound-ui` | ui:toggle | — | — |  |  |
| Config.Configuration.html | sbExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle ui:closeWin | — | sbExitClick cConfiguration.cpp:6280-6287 (4 句) |  |  |
| Config.DIOInterFaceCFG.html | spbDelete | Delete | `s:bound-ui` | ui:HT_WIN | FileRW/TTLCfg.cpp FileRW/TTLCfg.gen.inc | spbDeleteClick DIOInterFaceCFG.cpp:248-256 (4 句) |  |  |
| Config.DIOInterFaceCFG.html | spbExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick DIOInterFaceCFG.cpp:258-261 (1 句) |  |  |
| Data.Builder.html | spbExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cBuilder.cpp:484-487 (1 句) |  |  |
| Data.ContactCT.html | btClearCount | Count Clear | `s:bound-ui` | ui:postMessage | JsonBridge/actions/MainClarnData.cpp cContactCT.cpp | btClearCountClick cContactCT.cpp:944-1069 (76 句) |  |  |
| Data.CounterClear.html | spbExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cCounterClear.cpp:452-456 (2 句) |  |  |
| Data.Observer.html | btExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle ui:closeWin | cObserver.cpp forms/fObserver.h | BtnExitClick cObserver.cpp:697-706 (4 句) |  |  |
| Data.StartCondition.html | btnSetOffsetLimit | set limit to all | `s:bound-ui` | ui:postMessage | — | btnSetOffsetLimitClick cStartCondition.cpp:1234-1249 (11 句) |  |  |
| Data.StartCondition.html | sb_Maintenance_SmartDiagnosticFunction | Setup | `s:bound-ui` | ui:postMessage | — | sb_Maintenance_SmartDiagnosticFunctionClick cStartCondition.cpp:1080-1084 (1 句) |  |  |
| Data.StartCondition.html | spbExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cStartCondition.cpp:670-689 (16 句) |  |  |
| HW.HandlerSys.html | ExitBtn | Close | `s:bound-ui` | ui:postMessage ui:toggle ui:selectTab | FileRW/HSys.cpp FileRW/HSys.gen.inc | ExitBtnClick HandlerSys.cpp:1037-1043 (4 句) |  |  |
| HW.MotorTest.html | BitBtn2 | 開始 | `s:bound-ui` | ui:HT_WIN | ATC/ATCInterface.cpp ATC/ATCInterface.h | BitBtn2Click uMotorTest.cpp:2096-2100 (2 句) |  |  |
| HW.MotorTest.html | BitBtn3 | 存檔 | `s:bound-ui` | ui:HT_WIN | ATC/ATCInterface.cpp ATC/ATCInterface.h | BitBtn3Click uMotorTest.cpp:2102-2119 (10 句) |  |  |
| HW.MotorTest.html | btResetMNet | Reset MNet | `s:bound-ui` | ui:HT_WIN | forms/fMotorTest.h | btResetMNetClick uMotorTest.cpp:1718-1725 (3 句) |  |  |
| HW.MotorTest.html | btnAlarmReset | Alarm Reset | `s:bound-ui` | ui:HT_WIN | — | — |  |  |
| HW.MotorTest.html | btnMotorPower | Motor Power | `s:bound-ui` | ui:toggle | forms/fMotorTest.h | btnMotorPowerClick uMotorTest.cpp:1621-1645 (14 句) | SW[].On |  |
| HW.MotorTest.html | btnSaveLogLightScaleData | Save | `s:bound-ui` | ui:HT_WIN | forms/fMotorTest.h | btnSaveLogLightScaleDataClick uMotorTest.cpp:2051-2094 (27 句) |  |  |
| HW.MotorTest.html | btnStop | Stop | `s:bound-ui` | ui:HT_WIN | forms/fMotorTest.h | btnStopClick uMotorTest.cpp:1647-1659 (6 句) | MOT[].PCIL132_StopMotor StopAllMotor |  |
| HW.MotorTest.html | palExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | forms/fMotorTest.cpp forms/fMotorTest.h | palExitClick uMotorTest.cpp:1727-1730 (1 句) |  |  |
| HW.MyCCLinkSensor.html | sbExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | — | sbExitClick CCLink/MyCCLinkSensor.cpp:491-521 (13 句) |  |  |
| HW.ShuttleMove.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick ShuttleMove.cpp:1956-1961 (3 句) |  |  |
| HW.TrayEdit.html | Button1 | Manual Input | `s:bound-ui` | ui:HT_WIN | ATC/ATCInterface.cpp ATC/ATCInterface.h | Button1Click uTrayEditForm.cpp:619-662 (25 句) |  |  |
| HW.TrayEdit.html | SpeedButton2 | Abort | `s:bound-ui` | ui:HT_WIN | forms/fATCHandlerSide.h | SpeedButton2Click uTrayEditForm.cpp:614-617 (1 句) |  |  |
| HW.TrayEdit.html | spbUpdate | Update | `s:bound-ui` | ui:HT_WIN | — | spbUpdateClick uTrayEditForm.cpp:522-612 (38 句) |  |  |
| HW.VacuumUnit.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick VacuumUnit/VacuumUnit.cpp:402-406 (2 句) |  |  |
| HW.teach.html | btnHome | HOME | `s:bound-ui` | ui:toggle | forms/fMotorTest.h | btnHomeClick uteach.cpp:2133-2198 (37 句) | MOT[].ServoOnOff StopAllMotor |  |
| HW.teach.html | btnMotorTest | Motor Tools | `s:bound-ui` | ui:postMessage | — | btnMotorTestClick uteach.cpp:2436-2440 (2 句) |  |  |
| HW.teach.html | btnMoveP | Move + | `s:bound-ui` | ui:HT_WIN | — | btnMovePClick uteach.cpp:2104-2131 (15 句) | MOT[].Gali_MovePR MOT[].MotorMove |  |
| HW.teach.html | btnStop | STOP | `s:bound-ui` | ui:HT_WIN | forms/fMotorTest.h | btnStopClick uteach.cpp:2948-2954 (4 句) | StopAllMotor |  |
| HW.teach.html | pnlExit | EXIT | `s:bound-ui` | ui:postMessage ui:toggle | — | pnlExitClick uteach.cpp:5026-5032 (3 句) |  |  |
| Main.MotionView.html | axisToggle | 顯示未啟用軸 | `s:bound-ui` | ui:toggle | — | — |  |  |
| Setup.AGV.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick Automation/AGV.cpp:1036-1041 (3 句) |  |  |
| Setup.BarCode.html | bt2DIDOffset | 2DID Offset | `s:bound-ui` | ui:postMessage | — | bt2DIDOffsetClick BarCode/BarCode.cpp:6799-6804 (3 句) |  |  |
| Setup.BarCode.html | btnGetLotID | Get | `s:bound-ui` | ui:postMessage | — | btnGetLotIDClick BarCode/BarCode.cpp:11489-11495 (4 句) |  |  |
| Setup.BarCode.html | btnTestLotID | Test | `s:bound-ui` | ui:postMessage | — | btnTestLotIDClick BarCode/BarCode.cpp:11497-11507 (5 句) |  |  |
| Setup.BarCode.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | form.event 表列 | sbtExitClick BarCode/BarCode.cpp:2392-2401 (7 句) |  |  |
| Setup.BarCode.html | spbSave | Save | `s:bound-ui` | ui:postMessage | Automation/auto9045.cpp Command.cpp | spbSaveClick BarCode/BarCode.cpp:1254-1457 (129 句) |  |  |
| Setup.BinSel.html | btnAutoHide | 自動隱藏停用列 | `s:bound-ui` | ui:toggle | — | — |  |  |
| Setup.BinSel.html | btnSetAll2NotUse | Set all to not use | `s:bound-ui` | ui:postMessage | cBinSel.cpp forms/fBinSel.h | btnSetAll2NotUseClick cBinSel.cpp:6372-6378 (4 句) |  |  |
| Setup.BinSel.html | btnSettingSpecificBin | Specific Bin | `s:bound-ui` | ui:postMessage | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | btnSettingSpecificBinClick cBinSel.cpp:6118-6131 (7 句) |  |  |
| Setup.BinSel.html | sbtExit | ✕Exit | `s:bound-ui` | ui:postMessage | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cBinSel.cpp:2755-2759 (2 句) |  |  |
| Setup.BinSel.html | spbAOIBin | AOI Bin | `s:bound-ui` | ui:postMessage | — | 沒有 OnClick |  |  |
| Setup.BinSel.html | spbNormal | Normal | `s:bound-ui` | ui:postMessage | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | spbNormalClick cBinSel.cpp:2761-2781 (11 句) |  |  |
| Setup.BinSel.html | spbPrime | Prime | `s:bound-ui` | ui:postMessage | FileRW/BinSelect.cpp FileRW/BinSelect.gen.inc | spbPrimeClick cBinSel.cpp:2783-2803 (11 句) |  |  |
| Setup.BinSel.html | spbSave | 💾Save | `s:bound-ui` | ui:postMessage | Automation/auto9045.cpp Command.cpp | spbSaveClick cBinSel.cpp:2214-2272 (30 句) |  |  |
| Setup.Cleaning.html | sbCleanExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle ui:HT_WIN | FileRW/TestIF_File_Cleaning.cpp FileRW/TestIF_File_Cleaning. | sbCleanExitClick AutoClean/uCleaning.cpp:1868-1872 (2 句) |  |  |
| Setup.Cleaning.html | sbCleanSave | Save | `s:bound-ui` | ui:postMessage | FileRW/TestIF_File_Cleaning.cpp FileRW/TestIF_File_Cleaning. | sbCleanSaveClick AutoClean/uCleaning.cpp:1761-1866 (49 句) |  |  |
| Setup.Contact.html | btnTempSet | Temperature Setting | `s:bound-ui` | ui:postMessage | forms/fContact.h | btnTempSetClick cContact.cpp:17184-17187 (1 句) |  |  |
| Setup.Contact.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cContact.cpp:14478-14487 (5 句) |  |  |
| Setup.Contact.html | spbOneCycle | One Cycle | `s:bound-ui` | ui:toggle | form.event 表列 | spbOneCycleClick cContact.cpp:16861-16865 (2 句) |  |  |
| Setup.ContactForce.html | btExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | DynamicTemp.cpp forms/fDynamicTemp.h | btExitClick ContactForce.cpp:929-933 (2 句) | ADAM_WriteVoltage |  |
| Setup.HotPlate.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cHotPlate.cpp:627-631 (2 句) |  |  |
| Setup.Ld_ULd.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cLd_ULd.cpp:218-222 (2 句) |  |  |
| Setup.OffSet.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cOffSet.cpp:2797-2801 (2 句) |  |  |
| Setup.QAMode.html | btnOk | Exit | `s:bound-ui` | ui:postMessage ui:toggle | forms/fQAMode.cpp forms/fQAMode.h | btnOkClick QAMode.cpp:27-30 (1 句) |  |  |
| Setup.SCK_ART.html | btnExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | ATC/ATCInterface.cpp ATC/ATCInterface.h | btnExitClick Automation/SCK_ART.cpp:808-820 (7 句) |  |  |
| Setup.SCK_ART.html | btnExit1 | Exit | `s:bound-ui` | ui:postMessage ui:toggle | ATC/ATCInterface.cpp ATC/ATCInterface.h | btnExitClick Automation/SCK_ART.cpp:808-820 (7 句) |  |  |
| Setup.SetUp.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | form.event 表列 | sbtExitClick cSetUp.cpp:3452-3466 (8 句) |  |  |
| Setup.Speed.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cSpeed.cpp:1788-1793 (3 句) |  |  |
| Setup.Temp_Set.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | form.event 表列 | sbtExitClick uTemp_Set.cpp:5101-5124 (15 句) |  |  |
| Setup.TesterIF.html | btTesterTCPShow | Tester TCP | `s:bound-ui` | ui:postMessage | forms/fLotInfo.cpp forms/fLotInfo.h | btTesterTCPShowClick cTesterIF.cpp:1516-1519 (1 句) |  |  |
| Setup.TesterIF.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTesterIF.cpp:1358-1366 (4 句) |  |  |
| Setup.TrayAssignment.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTrayAssignment.cpp:1544-1548 (2 句) |  |  |
| Setup.TrayForm.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick cTrayForm.cpp:604-608 (2 句) |  |  |
| Setup.YieldMonitoring.html | btnOk | Exit | `s:bound-ui` | ui:postMessage ui:toggle | forms/fQAMode.cpp forms/fQAMode.h | btnOkClick uYieldMonitoring.cpp:3140-3145 (3 句) |  |  |
| Status.CounterSel.html | spbExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cCounterSel.cpp:145-149 (2 句) |  |  |
| Status.GroundMan.html | sbtExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/Temperature.gen.inc FileRW/TestIF_File_BarCode.gen.in | sbtExitClick GroundMan/GroundMan.cpp:1471-1476 (3 句) |  |  |
| Status.LtcSensor.html | btnClose | Exit | `s:bound-ui` | ui:postMessage ui:toggle | forms/fIoSetView.h | btnCloseClick LtcSensor.cpp:678-681 (1 句) |  |  |
| Status.Security.html | SecurityExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle ui:exportCsv | WebLevelSet.cpp cSecurity.cpp | SecurityExitClick cSecurity.cpp:813-816 (1 句) |  |  |
| Status.TowerLight.html | spbExit | Exit | `s:bound-ui` | ui:postMessage ui:toggle | FileRW/StartCondition.cpp FileRW/StartCondition.gen.inc | spbExitClick cTowerLight.cpp:155-158 (1 句) |  |  |
| eventlog.html | jamSetting | Jam Code Setting | `s:bound-ui` | ui:postMessage | — | — |  |  |
| Config.Configuration.html | btD47 | Clear | `s:bound-cmd` | form.event | form.event=yes | btD47Click cConfiguration.cpp:5959-5963 (2 句) |  |  |
| Data.CounterClear.html | spbExe | Execute | `s:bound-cmd` | counterclear.get counterclear.click | counterclear.get=yes counterclear.click=yes | spbExeClick cCounterClear.cpp:394-450 (28 句) |  |  |
| Data.LotInfo.html | btClearBarcodeCount | Clear Count | `s:bound-cmd` | lot.testerLog.tail | lot.testerLog.tail=yes | btClearBarcodeCountClick uLotInfo.cpp:9987-10003 (12 句) |  |  |
| Data.LotInfo.html | btClearBarcodeList | Clear List | `s:bound-cmd` | lot.testerLog.tail | lot.testerLog.tail=yes | btClearBarcodeListClick uLotInfo.cpp:10005-10014 (7 句) |  |  |
| Data.LotInfo.html | btTesterLogAll | 全部 | `s:bound-cmd` | lot.testerLog.tail | lot.testerLog.tail=yes | — |  |  |
| Data.LotInfo.html | btnFtpHD | HD | `s:bound-cmd` | act.lotInfoFtp.open act.lotInfoFtp.state | act.lotInfoFtp.open=prefix(act.lotInfoFtp.) act.lotInfoFtp.s | btnFtpServerClick uLotInfo.cpp:5001-5126 (62 句) |  |  |
| Data.LotInfo.html | btnFtpServer | Server | `s:bound-cmd` | act.lotInfoFtp.open act.lotInfoFtp.state | act.lotInfoFtp.open=prefix(act.lotInfoFtp.) act.lotInfoFtp.s | btnFtpServerClick uLotInfo.cpp:5001-5126 (62 句) |  |  |
| Data.LotInfo.html | btnFtpTester | Tester Name | `s:bound-cmd` | act.lotInfoFtp.open act.lotInfoFtp.state | act.lotInfoFtp.open=prefix(act.lotInfoFtp.) act.lotInfoFtp.s | btnFtpServerClick uLotInfo.cpp:5001-5126 (62 句) |  |  |
| Data.LotInfo.html | sbSECSLotEnd | Lot End | `s:bound-cmd` | lot.testerLog.tail | lot.testerLog.tail=yes | sbSECSLotEndClick uLotInfo.cpp:1346-1402 (33 句) |  |  |
| Data.SortCT.html | btnClearCount | 🗒 Clear Count | `s:bound-cmd` | act.trayEdit | act.trayEdit=yes | btnClearCountClick cSortCT.cpp:577-669 (40 句) |  |  |
| HW.home.html | sbAbortHome | Abort Home | `s:bound-cmd` | act.home.abort | act.home.abort=yes | sbAbortHomeClick uhome.cpp:4980-4986 (4 句) |  |  |
| Main.Record.html | spbClearRecord | CLEAR | `s:bound-cmd` | act.main.clearRecord act.main.meShuttle2Dbl | act.main.clearRecord=yes act.main.meShuttle2Dbl=yes | spbClearRecordClick main.cpp:30140-30169 (7 句) |  |  |
| Main.gbControlBtn.html | sbStateRecord | State Record | `s:bound-cmd` | control.takeover act.main.stateRecord ui:postMessage ui:download | control.takeover=yes act.main.stateRecord=yes | sbStateRecordClick main.cpp:26287-26290 (1 句) |  |  |
| Setup.AGV.html | btInitalLoad | Inital Load | `s:bound-cmd` | form.event | form.event=yes | btInitalLoadClick Automation/AGV.cpp:1055-1059 (2 句) |  |  |
| Setup.AGV.html | btInitalUnLoad | Inital Unload | `s:bound-cmd` | form.event | form.event=yes | btInitalUnLoadClick Automation/AGV.cpp:1061-1065 (2 句) |  |  |
| Setup.Cleaning.html | btnResetCleanCount | Reset | `s:bound-cmd` | form.event ui:postMessage | form.event=yes | btnResetCleanCountClick AutoClean/uCleaning.cpp:2095-2117 (15 句) |  |  |
| Setup.Cleaning.html | btnResetInterval | Reset Interval | `s:bound-cmd` | form.event ui:postMessage | form.event=yes | btnResetIntervalClick AutoClean/uCleaning.cpp:2810-2815 (2 句) |  |  |
| Setup.Cleaning.html | btnStartAutoClean | Clean | `s:bound-cmd` | form.event | form.event=yes | btnStartAutoCleanClick AutoClean/uCleaning.cpp:2869-2872 (1 句) |  |  |
| Setup.Cleaning.html | sbTrayAssign | Tray Assign | `s:bound-cmd` | form.event ui:postMessage | form.event=yes | sbTrayAssignClick AutoClean/uCleaning.cpp:2290-2294 (2 句) |  |  |
| Status.Security.html | btnHonPrec | HonPrec | `s:bound-cmd` | control.acquire security.passwd | control.acquire=yes security.passwd=yes | btnHonPrecClick cSecurity.cpp:927-933 (2 句) |  |  |
| Status.Security.html | btnOperator | Operator | `s:bound-cmd` | control.acquire security.passwd | control.acquire=yes security.passwd=yes | btnOperatorClick cSecurity.cpp:1659-1662 (1 句) |  |  |
| Status.Security.html | sbEngineer | Engineer | `s:bound-cmd` | control.acquire security.passwd | control.acquire=yes security.passwd=yes | sbEngineerClick cSecurity.cpp:943-949 (2 句) |  |  |
| Status.Security.html | sbSupervisor | Supervisor | `s:bound-cmd` | control.acquire security.passwd | control.acquire=yes security.passwd=yes | sbSupervisorClick cSecurity.cpp:935-941 (2 句) |  |  |
| Status.Security.html | spbExport | Export | `s:bound-cmd` | control.acquire security.passwd ui:exportCsv | control.acquire=yes security.passwd=yes | spbExportClick cSecurity.cpp:1579-1657 (42 句) |  |  |
| Status.Security.html | spbImport | Import | `s:bound-cmd` | control.acquire security.passwd ui:exportCsv | control.acquire=yes security.passwd=yes | spbImportClick cSecurity.cpp:1509-1577 (50 句) |  |  |
| Status.ShowBinSelect.html | btnAutoClean | Auto Clean | `s:bound-cmd` | act.showBinSelect.state act.showBinSelect.autoClean act.showBinSelect.uphDblClick | act.showBinSelect.state=yes act.showBinSelect.autoClean=yes  | btnAutoCleanClick cShowBinSelect.cpp:2101-2166 (32 句) |  |  |
| Status.ShowBinSelect.html | btnClearCount | CLEAR | `s:bound-cmd` | act.showBinSelect.clearCount | act.showBinSelect.clearCount=yes | btnClearCountClick cShowBinSelect.cpp:2262-2277 (6 句) |  |  |
| Status.ShowBinSelect.html | sbCopyRecipe | Copy Recipe | `s:bound-cmd` | act.showBinSelect.copyRecipe | act.showBinSelect.copyRecipe=yes | sbCopyRecipeClick cShowBinSelect.cpp:2845-2850 (3 句) |  |  |
| Config.Configuration.html | BitBtn1 | Set Vender Password | `s:C` | refs 1 | ATC/ATCInterface.cpp ATC/ATCInterface.h | BitBtn1Click cConfiguration.cpp:5949-5957 (6 句) |  |  |
| Config.Configuration.html | spbA25_RunExecutFilePathChoice |  | `s:C` | — | cConfiguration.cpp forms/fConfiguration.h | spbA25_RunExecutFilePathChoiceClick cConfiguration.cpp:6708-6719 (5 句) |  |  |
| Config.Configuration.html | spbA28_2 | OPEN | `s:C` | — | — | spbA28_2Click cConfiguration.cpp:6803-6818 (9 句) |  |  |
| Config.Configuration.html | btN06_UpdateTesterList | Update Tester List | `s:C0` | — | — | 沒有 OnClick |  |  |
| Config.Configuration.html | btnAutoCalSuckZ | Set Auto Calibrate Suck  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Exit | Exit | `s:C0` | ui:postMessage ui:toggle | — | 沒有 OnClick |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Save | Save | `s:C0` | refs 2 | — | 沒有 OnClick |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Setup | Setup | `s:C0` | refs 1 | — | 沒有 OnClick |  |  |
| Data.SmartDiagnostic.html | sb_SmartDiagnostic_Summary | Summary | `s:C0` | refs 1 | — | 沒有 OnClick |  |  |
| Data.StartCondition.html | btnClearDh | clear | `s:C0` | — | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | SpeedButton46 | - | `s:C0` | — | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | SpeedButton47 | + | `s:C0` | — | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | SpeedButton52 | - | `s:C0` | — | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | SpeedButton53 | + | `s:C0` | — | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | sbMotorTest_JogN | - | `s:C0` | refs 2 | — | 沒有 OnClick |  |  |
| HW.MotorTest.html | sbMotorTest_JogP | + | `s:C0` | refs 2 | — | 沒有 OnClick |  |  |
| HW.MyCCLinkSensor.html | Button10 | Set Socket Base | `s:C0` | — | — | 沒有 OnClick |  |  |
| HW.MyCCLinkSensor.html | setSocket9_24 | Set Socket Base | `s:C0` | — | — | 沒有 OnClick |  |  |
| HW.ShuttleMove.html | sbBarCode | Bar Code | `s:C0` | refs 1 | — | 沒有 OnClick |  |  |
| HW.teach.html | btnJogN | JOG N | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| HW.teach.html | btnJogP | JOG P | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.BarCode.html | btnClip_SendCmd_Auto2 | Send CMD | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.BarCode.html | btnClip_SendCmd_Auto3 | Send CMD | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.BarCode.html | btnClip_SendCmd_Fix1 | Send CMD | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.BarCode.html | btnClip_SendCmd_Fix2 | Send CMD | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.BarCode.html | btnClip_SendCmd_Fix3 | Send CMD | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.Cleaning.html | btFocusOnly | Only Focus | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | IndexOffSetBT1 |  | `s:C0` | refs 2 | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | IndexOffSetBT2 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnBottom2D |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsAuto1 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsAuto2 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsAuto3 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsAuto4 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsAuto5 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsAuto6 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsColor |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsEmpty |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | btnTrayOfsLoader |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAuto1 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAuto2 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAuto3 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAuto4 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAuto5 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAuto6 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAutoSh1 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbAutoSh2 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbFix1 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbFix2 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbFix3 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbFix4 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbFix5 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbFix6 |  | `s:C0` | ui:HT_WIN | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbHp1 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbHp2 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInPlacement |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInRotate |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh1 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh1LB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh1LB_AutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh1RA |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh1RA_AutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh1RB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh1RB_AutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh1_AutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh2 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh2LB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh2LB_AutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh2RA |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh2RA_AutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh2RB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh2RB_AutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbInSh2_AutoClean |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbLoader |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbLoaderB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOCR |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutRotate |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutSh1 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutSh1LB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutSh1RA |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutSh1RB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutSh2 |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutSh2LB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutSh2RA |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbOutSh2RB |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbPreciser |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbScanAOI |  | `s:C0` | — | — | 沒有 OnClick |  |  |
| Setup.OffSet.html | sbZcalibration | Go To In/Out Arm Z Calib | `s:C0` | — | — | 沒有 OnClick |  |  |
| Status.GroundMan.html | btnMaintenanceMode | Maintenance Mode | `s:C0` | refs 1 | — | 沒有 OnClick |  |  |
| Status.ShowMessage.html | btnBackToMain | Return to Main | `s:C0` | — | — | 沒有 OnClick |  |  |

## 4. 抽 10 顆給筆電複驗

| # | 頁 | id | 分類 | 怎麼複驗 |
|---|---|---|---|---|
| 1 | HW.MotorTest.html | SpeedButton44 | `s:unbound/D` | 開這一頁按 `SpeedButton44`，看 wb_serve oplog 有沒有收到命令；golden uMotorTest.cpp:1515-1519 |
| 2 | HW.teach.html | btnNeg90InRA | `s:unbound/D` | 開這一頁按 `btnNeg90InRA`，看 wb_serve oplog 有沒有收到命令；golden uteach.cpp:4657-4707 |
| 3 | Config.Configuration.html | btnMesSystem | `s:unbound/B` | 開這一頁按 `btnMesSystem`，看 wb_serve oplog 有沒有收到命令；golden cConfiguration.cpp:7625-7628 |
| 4 | Setup.BarCode.html | BtBottom_1_Connect | `s:unbound/B` | 開這一頁按 `BtBottom_1_Connect`，看 wb_serve oplog 有沒有收到命令；golden BarCode/BarCode.cpp:2753-2762 |
| 5 | Setup.SCK_ART.html | btnApplyCount | `s:unbound/B` | 開這一頁按 `btnApplyCount`，看 wb_serve oplog 有沒有收到命令；golden Automation/SCK_ART.cpp:723-798 |
| 6 | Setup.SetUp.html | btAutoShuttlePitch | `s:unbound/B` | 開這一頁按 `btAutoShuttlePitch`，看 wb_serve oplog 有沒有收到命令；golden cSetUp.cpp:4732-4736 |
| 7 | Alert.Password.html | sbPasswordCancel | `s:unbound/A` | 開這一頁按 `sbPasswordCancel`，看 wb_serve oplog 有沒有收到命令；golden Password.cpp:273-276 |
| 8 | Config.Configuration.html | btHeaterClearSelect | `s:unbound/A` | 開這一頁按 `btHeaterClearSelect`，看 wb_serve oplog 有沒有收到命令；golden cConfiguration.cpp:5480-5487 |
| 9 | Data.Observer.html | btOpenLoadLog | `s:unbound/A` | 開這一頁按 `btOpenLoadLog`，看 wb_serve oplog 有沒有收到命令；golden cObserver.cpp:3638-3681 |
| 10 | Data.StartCondition.html | btnArm1Aa | `s:unbound/A` | 開這一頁按 `btnArm1Aa`，看 wb_serve oplog 有沒有收到命令；golden cStartCondition.cpp:1153-1158 |

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

