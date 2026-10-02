# Exit（關閉程式）時的停機：現在怎麼走、哪些關不掉、1203 輸出怎麼清、還在等什麼裁決

> 移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（C++，UTF-8）；golden＝V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，cp950）；網頁＝`D:\HT9045\web\page\`。只寫 `:行號` 的，是同一段前面那個檔。
> 行號以 `git -C D:\HT9045 show HEAD:<路徑>`（HEAD `89ccb4cc`，分支 `v906/steven-cbridge-review6`，20260927 14:xx）為準。
> 出處：S121 的實作 `a684f171`（20260926 23:37；FileRW\MainClose.cpp、web\page\ht9045_main_close.js、tools\wb_serve.cpp）與 Q44 方案（`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 的「### Q44.」，`fd215f72`），逐條回程式核對過。
> ⚠ 本 skill 的 `SKILL.md`「IO 基底類型」表把 `ePLCbase`／`ePCI1203` 的值寫反了。正確是 `ePCI1203=3`、`ePLCbase=4`（golden `MachineType.h:781-782`、移植樹 `MachineType.h:841-842`）。IO_Table 的 ISABase 欄 3＝1203。

## 1. 現在 Exit 怎麼走（事實）

| 步 | 頁面（`D:\HT9045\web\page\ht9045_main_close.js:138-211`） | C++（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:1180-1383` `W906_Main_CloseProgramOp`） | 對 golden |
|---|---|---|---|
| 0 | 按 Exit → 送 `act.main.closeProgram {step:0}` | 每一步都先過 `CloseGuards`（`:368-414`）：運轉中不理、KYEC 條碼、權限低於 `LevelSet.AccessLevel[6]` 時馬達要在原點、實跑（REALLY）時 In／Out Arm、Shuttle、Index 上不能有 IC（MES1645）。過了回 needConfirm "Sure close??" | `main.cpp:29051-29107` `sbCloseProgramClick` |
| 1 | 第一框答「是」→ step 1 | `ReadWriteBinCountMode(false)`，進 FormClose：`WriteLastDataFile(true)` → `SaveMachineRecord` → `SaveRunMode` → `LotSummary.WriteFile`，回 needConfirm "Sure To Exit?" | `:29108-29131` → `FormClose :11852`、`:11861-11864`、`:11903` |
| 2 | 第二框答「是」→ step 2 | SECS DoExit（Sim 計數器）→ `W906_ProdCloseSave_Tail`（JamRate、`TimerRecordLoaderDate`、`WriteCTInfo`）→ **設結束旗標** `W906_ServeQuitRequested`（`:1356-1357`）→ 回的是關站段的**預估** `ShutdownSequence(false)`（`:1358`，phase=plan） | `:11911-11919`、`:12090`、`:12195` |
| 3 | 蓋層「機台停機中…」，列 `notStopped`（`ht9045_main_close.js:94-100`，標明「預估不是結果」） | wb_serve 主迴圈**晚一圈**離開（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` `W906_ServeQuitDue`，`MainClose.cpp:1066-1072`）→ 卸訊息 hook（`wb_serve.cpp:5957-5963`）→ `W906_ProdCloseSave` ＋ `W906_ProdCloseShutdown`（`:5964`，本體 `MainClose.cpp:1082-1096` → `ShutdownSequence(true)` `:675-969`）→ `Program Close=1`（`:5965`）→ `server.Stop()`（`:5967`） | FormClose `:11881-12474` 的關站順序 |

要記住的三件事：
- 停機是在「已經決定結束、連線快斷」之後才做；結果只印在主控台（`MainClose.cpp:1085-1095`，"MainClose:" 開頭、最後一行完整 JSON），頁面拿不到。**不管停沒停下來都會結束**。
- 頁面的「未停」清單（`MainClose.cpp:995-999` `notStopped`）只是 step 2 當下的預估。
- `--seconds` 到期（R40）走同一條：主迴圈 break 後一樣跑 `:5964`（存檔＋停機）。主控台 Ctrl-C／按 X／登出／關機走 `W906_ConsoleCtrl`（`wb_serve.cpp:6153-6159`）：**只寫 `Program Close=1`、回 FALSE 讓系統結束**，什麼都沒停（Q39 已裁決要改，見 §6，HEAD 與 origin/main `56039beb` 都還沒改）。　⛔ 20260929 更新（D-012，commit `83de77fe`）：主控台 Ctrl-C／Ctrl-Break／按 X 現在會停機——處理函式只設旗標，tick 執行緒（主迴圈底、W906_ModalWaitTick）送跟 Exit 一樣的 Q44Issue（StopAllMotor、Stop1203All、MotorAllBtnUp、SwHeaterRelay Off，不等讀回）；第一次 Ctrl-C 送完停機再走正常關站，第二次立刻結束；按 X 最多等 3.5 秒送出停機。登出／關機事件在載入 user32 的程式大概收不到（要隱藏視窗接 WM_QUERYENDSESSION，待辦）。關站中（Exit 第二框之後、或主控台關）只收關、停、讀、登入指令，清單見 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md` A2。
- ⛔ 20260930 更新（D-012 A8，St01 工程師；ST01-E 核對後 commit）：**golden 關站的事件紀錄每一條路都有了**。golden `TfMain::FormClose` 只寫一筆 `NewRecordProcess("MES2109", "Program Close", asHandlerVersion+"."+SVNRevision)`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:12443`；`bHandlerModel=false` 時 FormClose 不跑、不寫），golden 沒有「強制關閉」的字。移植樹：正常關站路（Exit、［強制關閉］、`--seconds`、第一次 Ctrl-C）本來就由 `ShutdownSequence(true)` 在 golden 的位置寫（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:1086-1101`）；第二次 Ctrl-C／Break、按主控台 X、登出、關機（行程在 `W906_ConsoleQuit` 裡結束）原本沒寫，現在處理函式請 tick 執行緒（`Q44ConsoleTick`，`NewRecordProcess` 不是執行緒安全）送完停機命令緊接著寫同一筆、同樣的字，寫完才結束（X 併在 3.5 秒裡；第二次 Ctrl-C 最多等 0.5 秒）；一個行程只寫一筆（先認領的寫）。「強制」「哪一個主控台事件」照舊只在主控台與 `D:\HT9045\Error\BootLog.txt`。被直接砍掉（F5 停止、taskkill /F）仍然什麼都寫不了（golden 同）。細節 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md` A 第 8 條；ctest MainCloseStop [18]。
- ⛔ 20260930 晚更新（D-012 A3W，St01 工程師；ST01-E 核對後 commit）：**登出／關機做了，而且改成照 golden 不寫**。
  - golden 事實（BCB6 VCL 6 原始碼 `D:\ProgramFiles\Borland\CBuilder6\Source\vcl\forms.pas`）：WM_QUERYENDSESSION → `TCustomForm.WMQueryEndSession`＝「CloseQuery and CallTerminateProcs」（`:4104-4107`；TfMain 沒有 OnCloseQuery，訊息表 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.h:1206-1209` 只接 WM_COPYDATA／WM_HOTKEY）⇒ 回 TRUE、**不呼叫 FormClose**；WM_ENDSESSION → `TApplication.WndProc` 只設 FTerminate（`:6420`），之後拆表單走 `main.cpp:12480` FormDestroy（不停馬達）。⇒ golden 登出／關機不停馬達、不關加熱器繼電器、不寫 MES2109、不寫 Program Close=1（留 0，下次開機提示異常關機）。上一點說「登出、關機原本沒寫、現在寫同一筆」與 A8 裡「golden 程式結束一定走 FormClose（VCL 登出／關機也是）」**都不對**，以這一點為準。
  - 移植樹規則（ST01-E 20260930 裁定：BCB 清楚的照 BCB；停機是 Q44-6 選 a「停輸出」，golden 沒有的安全補強）：登出／關機＝只送停機命令（tick 執行緒，同 Exit 的 Q44Issue），**不寫** MES2109、**不寫** Program Close=1；按主控台 X、第二次 Ctrl-C 照舊（停機＋MES2109＋Program Close=1，golden 沒有對應，是移植樹自己的選擇）。
  - 收得到登出／關機的方法：微軟 SetConsoleCtrlHandler 備註——載入 user32／gdi32 的主控台程式收不到 CTRL_LOGOFF／SHUTDOWN_EVENT，要自己建隱藏視窗接 WM_QUERYENDSESSION／WM_ENDSESSION（HWND_MESSAGE 的訊息視窗收不到）。`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp` 檔尾 `W906_SessionEndWatchStart`（`:2752-2877`）開一條自己的執行緒、建一個看不見的頂層視窗＋訊息迴圈；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4536` 在 SetConsoleCtrlHandler 旁同一行呼叫。WM_QUERYENDSESSION 立刻回 TRUE（golden 同）；WM_ENDSESSION(TRUE) 走主控台登出／關機同一支（`Q44ConsoleQuit`，事件 5／6 → 新的 `kConSessionEnd`），等 tick 執行緒送出停機命令（最多 3.5 秒；WM_ENDSESSION 回去後系統隨時可以結束行程）後 ExitProcess；WM_ENDSESSION(FALSE) 什麼都不做。視窗那條執行緒只設旗標、等，不呼叫廠商 API。
  - 兩條路都到（主控台 CTRL_CLOSE＋WM_ENDSESSION）：「寫 Program Close=1、結束行程」只有先到的 owner 做（`s_conExitOwner`）；後到的只補停機旗標、不寫、不結束行程——避免兩條執行緒同時寫 `D:\HT9045\system\Gerneral.ini`、一條把另一條寫到一半的檔砍斷。CTRL_CLOSE 先到＝照 X 寫；登出／關機先到＝不寫。
  - 規則是純判斷（(甲) A3W 段 `MainClose.cpp:1928-2007`：`ConsoleDecide`、`SessionEndConsoleEvent`、`QueryEndSessionAnswer`、`ConsolePlanFor`、`ConsoleExitRoleFor`、`SessionEndWindowExits`），ctest MainCloseStop [19]（90 項，全檔 390 項通過）。真的登出／關機訊息 ctest 測不到，機邊要看的清單在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md` A 第 3 條最後一點。

## 2. 關站段做了什麼（`ShutdownSequence`，照 golden 順序）

每一步回報一個狀態（`MainClose.cpp:473-491`）：`done` 做了而且確定到了機台（1203 ISSUED ret=0）或本身是記憶體旗標；`noop` 這台沒有這個東西（golden 也不做事）；`stub` 移植樹是替身、空殼、只到模擬後端或 DRY RUN；`missing` 沒翻或這一輪不接；`failed` 送了但被 1203 路由／命令面拒絕或廠商回錯；`unverified` 呼叫了但看不出有沒有到機台。停馬達／關加熱／關輸出三類裡，只有 done 叫「已停」、noop 叫「這台沒有」，其餘一律「未停」。

真的會動到輸出或馬達的步驟（golden `main.cpp:12151-12171`；移植樹 `MainClose.cpp:875-933`）：

| golden | 動作 | 移植樹 |
|---|---|---|
| `:12041-12044` | `SW[SwMusic1+0..3].Off()`（蜂鳴器） | `SwOffItem` `:797-800` |
| `:12151` | `EndHeaterThread()` | noop：移植樹沒有溫控執行緒（`:877-879`）。golden 這一步也只是停輪詢，不是關加熱器 |
| `:12153` | `StopAllMotor()` | `:882-898`；輸出有送到卡也只標 unverified（馬達那一半看不到，`:896`） |
| （移植樹專用） | 1203 監看器開的軸 `Acm_AxStopDec`＋`Acm_AxSetExtDrive(0)` | `Stop1203Item` `:631-662`（經 `W906_Stop1203AllHook`，`:125`） |
| `:12156` | `IndexMotorBreakerOFF()`（煞車鎖住） | `:902-916` |
| `:12157` | `SW[SwHeaterRelay].Off()` | `:917` |
| `:12163-12170` | 加熱風扇、Index 離子風扇、DUT 冷卻風扇、Shuttle／HotPlate 冷卻、行車紀錄、`SwAirOff`（有 Enable 才關） | `:922-931` |
| `:12171` | `SoftStop=true` | `:932-933` |

照 R38：**不做伺服 OFF、不切 `SwMotorRelay`**（golden 沒有）。

## 3. SIM 建置接真卡時，為什麼關不掉 1203 頁打開的 DO

- SIM 建置（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MachineType.h:63-65`，沒定義 `W906_NO_SOFT_SIMULTE` 就開 `SOFT_SIMULTE`）裡，引擎 IO 走模擬後端，1203 路由不裝（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203IoRoute.cpp:345-348`）。golden 的 `SW[].Off()` 只寫到模擬值，回報 stub（`MainClose.cpp:549-551` 的說明文字、`:584`）。
- 但 1203 **命令面**不看 SIM：`WB_PUMP_1203_CONTROL`、`WB_PUMP_1203_CONTROL_LIVE`、`INSTALL_1203_MONITOR` 無條件定義（`MachineType.h:105-106`、`:121`）。只要這顆二進位有 SDK（`HAVE_PCI1203`）又接著卡，1203 頁的 `pci1203.do.setBit`／`setByte`（`wb_serve.cpp:5791` → `:6236-6300` `W906_Dispatch1203Ex` → `Pci1203Control::Execute`）就真的打到卡（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.cpp:1671-1694`，`Acm_DaqDoSetBitEx`／`Acm_DaqDoSetByteEx`）。
- 結果：筆電用 SIM 建置接 HT9050、從 1203 頁打開 `SwHeaterRelay`，按 Exit → golden 的 `SW[SwHeaterRelay].Off()` 只關到模擬值 → 繼電器仍通電 → 程式照樣結束。

## 4. 出貨建置也有漏網的

路由裝上後（出貨建置，`Pci1203IoRoute.cpp:349-356`）`SW[].Off()` 會真的到卡，但：
- **沒有引擎物件的 1203 輸出**（`MachineType.h:1762-1764` 列的 54 個：`C_*Off` 第二線圈、`C_*EdgeClip`、`C_*DrawerLock`、`C_InPnPDrop1-4`…）golden 順序裡沒有任何一步會碰它們。
- 1203 頁直接打開、但不在 golden 關站清單上的點，同樣沒人關。

## 5. 固定「未停」的項目（為什麼每台都有）

這些是移植樹沒翻，不是真的有東西開著；如果「有未停就擋關閉」，每一台都會關不掉：

| 項目 | 位置 | 為什麼 |
|---|---|---|
| EP 電壓歸零 `ADAM_WriteVoltage(0)`、`ADAM_DirectWriteData(0,0)` | `MainClose.cpp:746-748` | ADAM 是空殼（`atester_shims.cpp:326-327`），不分機台一律 stub |
| Index kit 吸嘴 `CheckKitSuck.Suck[0][0..1].Normal()` | `:801-802` | 一律 missing：`CheckKitSuck` 是 `mykitsuck.h` 那個 `TMyKitSuck`（兩個佈局的陷阱），本檔不能 include |
| `StopAllMotor()` 的非 1203 馬達卡 | `:896` | 一律 unverified：看不到 MN200／MNet／ISA 後端有沒有真的寫到卡 |
| ESD 停止 | `:759-775` | 有 ESD 時：找得到 ESD_Monitor 視窗＝unverified，找不到＝stub |
| HonPrec ATC、ATC6.0／3.0／New ATC 的 STOP | `:787-794`、`:936-942` | 這一輪不接（ATC 區同事在改） |

## 6. 裁決狀態

| 題 | 狀態 | 內容 |
|---|---|---|
| S121（Q35，20260926 22:xx） | 生效，已做 `a684f171` | 「是的話，要通知 c++ 完全停工，馬達跟加熱都要關掉，安全第一」：Exit 第二框確認後照 golden FormClose 順序停機；替身沒停的標「未停」，不假裝已停 |
| S122 | 交 Jimmy | 關 Teach／Motor Test 分頁要重新 find home；重新整理先停馬達（不在本檔範圍） |
| R38 | 照建議先做，可推翻 | 不加 golden 沒有的伺服 OFF／切 `SwMotorRelay` |
| R39 → **S163**（20260927 11:xx） | 裁決方向，做法待定 | 「必須先停下才能關閉」：推翻「頁面標未停就照樣關」。Exit 要確定輸出（含 SIM 建置接真卡時 1203 頁打開的真 DO）都停了才關——關閉流程真的關掉，或有未停時擋關閉、要操作員先停。另「SIM 建置不在真機上通電加熱」這條規定還要 Steven 點頭 |
| R40 | 照建議先做，可推翻 | `--seconds` 到期也照 golden 停機 |
| R41 | 照建議先做，可推翻 | 沒讀到機型（`bHandlerModel=false`）照樣停機、只是不存檔 |
| Q39（RULINGS_20260927 第 2 條 #17） | 已裁決，**已做**（Exit：`9bc19493`／`5789eeea`；Ctrl-C／X：D-012 `83de77fe`，停機先於存檔，是安全的方向） | Exit 與 Ctrl-C 都照 golden FormClose 順序停；Ctrl-C 那條由筆電補 |
| **Q44** | **已裁決（最小版已做）** | Steven 20260928：最小版＝停全部馬達＋關加熱器繼電器，其餘列待辦（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md`）；20260929 細化 「SOFT_SIMULTE因為馬達不會真的動作, 所以關閉時沒有限制, 機台上只要c++有回復 StopAllMotor() 是已經發送, 且加熱io也有off, 就可以當成已停機」 ⇒ SIM 建置不設限；機台上停止命令送到、加熱器 Off 寫出去就算停，**不等讀回**，只有送不到（卡失聯、沒武裝、被拒、DRY RUN、廠商回錯）才擋、給［重試停機］／［強制關閉］。程式：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp` 檔尾 w906q44::Evaluate（commit `9bc19493`，20260929 放寬）；ctest MainCloseStop。原本的 A／B／C 與 Q44-1～6 細節只剩參考價值。 |

裁決原文：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`（S121 `:327-331`、S122 `:333-340`、S163 `:421`）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md:31`（#17）；帳本 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`（R38～R41、Q35、Q39）與 `decisions-pending.md`（Q44）。**在 Steven 選之前，Exit 維持 `a684f171` 的行為（標未停、照樣結束）。**

## 7. 1203 命令面清零：正確做法與陷阱（Q44 (b) 的技術前提，做法本身待裁決）

用現有 API 就做得到，不用改 EastSun 的檔（`EtherCAT\Pci1203Monitor.*`、`Pci1203Control.*` 只讀、只呼叫）：
1. **讀**：`Pci1203Monitor()->doCount()`／`do_(k)`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.h:1436-1439`）；每個 DO byte 帶 `valid`、目前值 `byteData`、`station`、`stationChan`、`ring`（`:1148-1178`，-1＝對應表沒說）。
2. **寫前一定先過** `Pci1203RouteCanWriteBit`（`Pci1203IoRoute.cpp:332-341` → `CheckWrite_` `:119-179`）：命令面沒武裝、卡沒開或監看器停了、**ring 0**（`:144-149`）、對應表沒有這個 byte、byte 讀不回來（`valid=false`）、**驅動器站**（`:171-176`）一律不寫。
3. **寫**：`Pci1203Control()->Execute(kCmdDoSetByte, port=k, value)`（`Pci1203Control.cpp:666-672` 驗證、`:1683-1694` 用 ring／站號送 `Acm_DaqDoSetByteEx`，成功後自己 `ForceDoReread`）。
4. **讀回**：清零與停軸之後跑一次監看器 `Poll`（只讀；Poll 前後把「輸出優先」掛鉤拿掉，做法同 `wb_serve.cpp:7627`），再判定。1203 軸看 `Acm_AxGetState`，`STA_AX_READY=1`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\vendor\AdvMotDrv.h:792-800`）。

陷阱：
- ⚠ **第 2 步不能省**：命令面自己的 DO 路徑只驗 port／bit 範圍（`Pci1203Control.cpp:653-672` `ValidateDo`），`W906_Dispatch1203Ex` 也沒有另外檢查（`wb_serve.cpp:6259-6280`）——**不擋驅動器站**。ring 0 的 DO byte 是伺服的 RxPDO，寫 0 可能清掉伺服的控制字。
- 廠商 API 只能在 tick 執行緒（就是 Poll 監看器那一條）呼叫（`Pci1203Control.h:799-804`：not thread-safe、同一條執行緒）。Ctrl-C 處理函式跑在系統另開的執行緒，只能設旗標（`W906_ServeQuitRequested` 是 `std::atomic`，`MainClose.cpp:1059-1062`），由主迴圈做。
- 只有命令面 LIVE 才真的到卡（`MachineType.h:106`）；DRY RUN 或沒武裝時那些點算「未停」。
- 保留清單：照 R38 不切 `SwMotorRelay`——HT9050 表 `D:\HT9045\machines\HT9050\IO_Table.csv:86` 這一點 ISABase=3（1203 輸出），全清會把它也清掉。全清的副作用要先讓 Steven／EastSun 知道：真空關掉吸嘴上的 IC 會掉（CloseGuards 只在實跑時擋機台內有 IC，`MainClose.cpp:399-411`）、單線圈氣缸回彈簧位置會動、門鎖會放開。
- 要 EastSun 確認（repo 沒有量測紀錄）：ring 1 各 IO 站清零有沒有副作用、保留清單放哪些點、從站看門狗在程式結束時會不會自己清輸出。

## 8. 驗證限制

- Steven01 這台沒有 1203 卡：監看器沒開過 ⇒ 只能驗「沒有卡＝不擋」與蓋層流程。
- 真卡清零、讀回、擋關閉要在 HT9050 機台旁驗；先用無害的點（例如 `SwHeaterFan`）做低能量量測：開 DO → 按 Exit／Ctrl-C → 看 DO 燈 10 秒內會不會熄（Q39 當初的建議）。
- RULINGS_20260927 第 4 條：這兩天模擬組態驗過就算完成；上機要看什麼寫進交件報告。
- ⛔ 20260930 補（A3W）：登出／關機只能真的登出／關機才驗得到（ctest 送不出 WM_QUERYENDSESSION／WM_ENDSESSION）。要看：開站印「登出／關機的隱藏視窗已建立」；登出後 `D:\HT9045\Error\BootLog.txt` 最後有 `W906 Q44 console logoff: stop commands sent …`；下次開機 `D:\HT9045\system\Gerneral.ini` 的 `[Record] Program Close` 是 0、事件紀錄最後沒有 MES2109「Program Close」；HT9050 用 `SwHeaterFan` 之類的低能量點看 DO 有沒有熄。清單全文在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md` A 第 3 條。

## 9. 例子

- **SIM 建置接 HT9050（今天）**：1203 頁打開 `SwHeaterRelay`、`SwHeaterFan` → Exit 兩框都答是 → 頁面列「未停：SW[SwHeaterRelay].Off()（stub）…」→ 程式結束，繼電器仍通電。
- **Q44＝A 之後（方案）**：同樣操作 → 第一框就列出「還開著：SwHeaterRelay、SwHeaterFan（1203 卡讀回＝1），關閉時會關掉」→ 清零寫 0 → 讀回 0 → 結束。卡片途中失聯時清零被拒 → 紅字「無法確認輸出已關」→ 操作員 EMG 或斷電後按［強制關閉］（Q44-3）。
- **HT9045 有 EP（Q44＝A、Q44-2＝a 之後）**：沒有「擋」類，只有「EP 電壓沒有歸零（ADAM 沒翻，看不到）」→ 勾「我已確認」→ 結束。
