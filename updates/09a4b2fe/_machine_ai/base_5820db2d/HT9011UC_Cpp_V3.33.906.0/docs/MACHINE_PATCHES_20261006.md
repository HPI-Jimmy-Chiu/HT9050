# 機台 patch 收件紀錄 20261006

> 接 `MACHINE_PATCHES_20261005.md`（第 25 節）。機台 10/05 23:08 之後停了一整晚，10/06 08:0x 開起來。

## 1. 08:0x～08:4x：cpp 0229（PKG-152）、cpp 0230（IOTUNE）、web 0116（PKG-150 web）；快照 08:23／08:43（08:4x）

- **cpp 0229＝PKG-152**（機台 08:06～08:17 套第 152 包，定時檢查發現）：EastSun 1006 裁決——①FR-DR14 空跑改成 EastSun 的 14 步「收」（⚠ teach.ini `[MTestZ1] setEditIndex1ToOutSht1Z` 仍是 0，In Sht Z −4357 已教）；②F9050-FIX1「收」。`WebMotorAccess.cpp` 與 `HW.teach.html` 留機台版：第 152 包對 `WebMotorAccess.cpp` 的修改就是機台自己 10/03 的 HOME-PERAXIS（main `835e2e0d` 收的），`HW.teach.html` 機台多了自己的 Home All／小鍵盤腳本——**沒有漏掉的東西**。tests/CMakeLists 與 test_web_motor_access 的衝突取機台側。機台 o2 全建 0 錯誤、ctest 374／394（失敗＝已知清單＋FastClk_Jobs 同時跑偶發）。PKG 不 cherry-pick；夾帶的 WORKLOG 照收。
- **cpp 0230＝IOTUNE**（機台自己的；EastSun 1006「你先自己啟動分析」，WORKLOG 第 129 列）：IO 執行緒每次採用新樣本都重建 api 快取並多發布一次 → 改成只在 IO 時鐘到點時才重建；間隔預設 10 → 30 ms。**上機實測（release、沒開 HMI、45 秒、60 次 sys.ping）**：單執行緒 CPU 77%、ping p50 12／p90 77／最慢 1,231 ms；間隔 30 ms：CPU 110%（IO 執行緒 74%）、p50 1.8／p90 6.6／最慢 22 ms；間隔 10：126%、p90 12.6；間隔 50：100%、p90 10.1；間隔 100：85%、p90 19.3、最慢 374。一次 Poll 約 80～90 ms 的 CPU（下一步查耗在哪）。10/05 21:01 那次（Debug）反而變差：CPU p50 34%→162%、按鍵 p90 14.8→31.5 ms。只改 `tools/wb_serve.cpp` 的 IOTHREAD 部分 ⇒ 照 #115＝C **留機台、不收進 main**；WORKLOG 照收。這組數字就是 **W-89 的答案**（機台自己量的）。
- **web 0116＝PKG-150 web**：第 150 包本來就有的 State Record 逾時文字，機台套上而已，不收。
- **快照**：08:23、08:43 兩次（設定／工單／op log，無程式變更）；08:43 那份鏡像到 GitLab main `9e812117`（general.ini 的 LastFile、lastdata*.dat、README；6 檔）。
- **鏈尾**：C++ `98c94aad` → **`10791142`**（mach_chain.py 的 REPO 改指 b18：原本的 mach1002 worktree 07:2x 清掉了）。下一次從 **cpp 0231／web 0117／tools 0165** 開始。
- **WORKLOG_MACHINE.md**：照機台鏈尾整份收（main 少 6 列：第 128 列 IOTHREAD、第 129 列 IOTUNE、套第 150／151／152 包三列、設計外掛 tools 0164；只有新增）。

## 2. 09:00：cpp 0231（PKG-153-157）、web 0117（PKG-150 web 第二段）、web 0118（PKG-153-157 web）；快照 09:00（09:2x）

- **cpp 0231＝PKG-153-157**（機台 09:00 一次套第 153～157 包，`PLAN_157.md`；疊包比對 OLD 46／NEW 11／SAME 1／兩邊都改 13）：照舊保留機台自己的 `WebMainScanKey.cpp`（軟體急停 `W906_SoftEmergencyStop`／`MotorAccessOnAlarm` 與它的軟鍵佇列）與 `WebBridge/WebBridgeServer.cpp`（TOKEN-OFF、`panel.estop` 免權杖）。PKG 不 cherry-pick。
- **筆電逐檔對過**（main `445275d1` 對機台鏈尾 `a36d04e8`，第 153～157 包改到的 57 個程式檔）：45 個相同；12 個不同，全是已知留在機台的——`WebMainScanKey.cpp`（main 多 68 行＝St02 !226 ST02-P2 把機台的軟鍵整合進 main 的版本；機台 74 行＝它自己的原版＋軟體急停，10/05 稽核判定等價）、`WebBridgeServer.cpp`（TOKEN-OFF）、`tools/wb_serve.cpp`（IOTHREAD／IOTUNE）、`WebMotorAccess*.cpp／.h`（機台 HOME-PERAXIS 版）、`cinitial.cpp`、`FileRW/MainClose.cpp`、兩個 CMakeLists 與三支測試。**沒有漏掉的筆電改動、也沒有新的機台自己的改動。**
- **web 0117**（10/05 22:24）＝第 150 包 web 的第二段（`Main.gbControlBtn.html`）；**web 0118**＝第 153～157 包的 web（`HW.teach.html` Set To Offset 的防呆）。都是包本身，不收。
- **快照 09:00** 鏡像到 GitLab main `48503fdc`（6 檔）。`pkg_uptake.py` 這輪先誤報「機台在第 153 包」——它把 `PKG-153-157` 讀成 153，已改成取範圍的尾端（重跑：OK，機台已套第 157 包）。
- **鏈尾**：C++ `10791142` → **`a36d04e8`**（cpp 0231）。下一次從 **cpp 0232／web 0119／tools 0165** 開始。

## 3. 09:14：cpp 0232（IOTUNE-D）、web 0119（第 153～157 包的 web 重做）（09:3x）

- **cpp 0232＝IOTUNE-D**（機台自己的；EastSun「先更新 再跑最佳化執行緒」）：`tools/wb_serve.cpp` 的 io 網頁快取（約 148 KB）只在最近 3 秒有頁面讀過才重建（原本每 100 ms 一拍都重建、每次 18 ms＝主迴圈 18%，而沒有人讀）；motor 那份照舊每次重建；環境變數 `W906_APICACHE_IO_ALWAYS=1` 回到舊行為。跟 IO 執行緒無關 ⇒ **收進 main**：b19 `v906/jimmy-b76` `1fe677ed`（作者照機台），第 76 批 gate `b76a` 09:31 起。已知取捨：閒置 3 秒後第一次讀 io/runtime 拿到上一份（下一次輪詢就新）；沒有 ctest 讀線上快取。
- **web 0119**＝第 153～157 包的 web 重做：收了 `HW.teach.html` 的 Set To Offset 防呆（ST02-C23 #103）、`ht9045_contact_slk.js`／`ht9045_showbinselect_ev.js`（St01 第 154 包）；`main.html`／軟鍵／Home All／`ht9045_wire_engine.js`／motor-access.js 留機台的——**ST02-C23 #100（Teach 136 欄照 golden 夾範圍，`HT9045PageKbRange`）沒收**，問 EastSun 是不是刻意的（W-98，同時給 St01）。
- **WORKLOG_MACHINE.md**：照鏈尾收進第 76 批（機台改寫了自己的第 129 列，加第 153～157 包那一列）。
- **鏈尾**：C++ `a36d04e8` → **`f306ea95`**（cpp 0232）。下一次從 **cpp 0233／web 0120／tools 0165** 開始。

## 4. cpp 0232（IOTUNE-D）→ main `bc2891cc` → 第 158 包 GitHub `31e2b21`（11:1x）

- 第 76 批：b19 `v906/jimmy-b76`（`1fe677ed` 作者照機台＋合 origin/main 文件／工具），gate `b76a` 兩組態綠（出貨 439＝基準 4；模擬＝基準 19＋3 個負載逾時單獨重跑過）。包裡另有 ES02 HTDESIGNER 0.176／0.177。
- 機台這邊沒有新 patch：下一次從 **cpp 0233／web 0120／tools 0165** 開始；C++ 鏈尾 `f306ea95`。

## 5. 11:07：tools 0165（HTDESIGNER：工具列的 ▶ 按鈕＝F5）（11:2x）

- 機台自己的（EastSun 1006「我剛剛用方案總管那邊的 啟動按鈕 建置時 和我按F5 時好像不太一樣」「我需要跟F5一樣」）：`tools/vscode-htdesigner` 的 ▶ 原本自己建 `build_nonoracle`（模擬）；改成停掉正在跑的再執行 `workbench.action.debug.start`（＝F5，照「執行與偵錯」選的組態與它的 preLaunchTask），設定 `ht9045Designer.run.likeF5`（預設 true，false＝舊的 ▶）。只動 extension.js／package.json／CHANGELOG／HANDOVER／smoke 測試。
- 照 tools 0164 的做法 **交 ES02 併進下一版 htdesigner**（ES02 是那條線的主人，main 上是 0.178；直接收機台這顆會跟 ES02 的版本岔開）。TO_ES02 §4 11:2x。
- 下一次從 **cpp 0233／web 0120／tools 0166** 開始；C++ 鏈尾仍是 `f306ea95`（tools 不接鏈）。

## 6. 11:37：cpp 0233（IDX1203：HT9050 的 Index Z1 改走 1203 類別）、web 0120（第 153～157 包 web 的同一顆）；快照 11:37 `fb0981f3`（11:5x）

- **cpp 0233＝IDX1203**（機台自己的；EastSun 1006：Q130「INDEX_MOTION_CARD 改成非 0（用 MyMotor）」「gali 是不是有單獨class ? 可以用1203 class 分支?」「你可以統一 index 寫成一個移動函式 裡面再分支呼叫?」——推翻 0929 D1）：`INDEX_MOTION_CARD≠0` 時 golden 照馬達表建 Index 軸（M14 → `TMyEtherCatMotor`），Galil 轉送不安裝；約 20 個 `TMyMotor::Gali_*` 在開頭依類別分流——Index 軸是 1203 就走**唯一的 Index 移動 `W906_IndexMove`**（golden 的 Index 互鎖 `IndexZCanMove`／`CheckTestZ1`／`ScanIndexMotorCanMove`、速度直送 1203 類別、`MotorMove`、golden 的編碼器範圍檢查→JAM）；Galil 命令字串→`W906_Idx1203Command`（停止、伺服、DP／DE、查詢）；`ScanMotStatus`／`TIMO`／警報→類別的燈號＋golden 的警報鎖存；`SingalHome`→golden 的 task 編號＋`MotorInitial`＋`MotorHome`；`FindZPhase` 拒絕。開關：`MachineType.h` `#define W906_IDX1203_GALI_BRANCH`；**`INDEX_MOTION_CARD=0`（其他機台）＝golden 不變**。附 ctest `test_idx1203_gali_branch.cpp`。動到 `Motor/myGALILmotor.cpp`（+274）、`Motor/mymotor.cpp`、`Motor/EcatMotorRoute.cpp`、`Motor/GaliRoute.h`、`EtherCAT/Pci1203GaliRoute.cpp`、`MachineType.h`、tests。文件部分是第 158 包帶過去的 main 文件（＋機台 WORKLOG 兩列）＝機台套了第 158 包。
- **收進 main**：第 78 批（跟 Ifor01 MR !238 一起；第 77 批 gate `b77a` 跑完就開，在 b18 準備）。⚠ **跟 St01 的 ST01-C 第 1／1b 片（Index 那一側、`Gali_*` 路由）重疊**——照「機台已經做的不要再做一份」，已請 St01 Index 側的程式先停、等 IDX1203 進 main 再接（TO_STEVEN §4 11:5x）。
- **web 0120**：跟 web 0118／0119 同名（第 153～157 包 web 的 Set To Offset 防呆），包本身，不收。
- **鏈尾**：C++ `f306ea95` → **`81b64a21`**（cpp 0233）。下一次從 **cpp 0234／web 0121／tools 0166** 開始。

## 7. 11:51：cpp 0234（IOFIX：IO 執行緒審查 #1～#6 在機台修好）、web 0121（包本身）、`dispatch/20261006_iothread_tick/REPORT.md`（給 Jimmy 的節拍數據）（12:0x）

- **cpp 0234＝IOFIX**（機台自己的；EastSun 1006 回 Jimmy #128＝A：「我現在修 #1～#6，修好繼續開」）：#1 重掃失敗／卡沒開也在 IO 執行緒發布 ⇒ WAR16152 照常；#2 代理等待 500 ms 分段＋心跳，5 秒沒進展就撤回並拒絕；#3 發布帶時間，3 秒沒新發布就標「卡沒開」；#4 IO 執行緒結束前自己清旗標、Stop 逾時就棄置（代理一律拒絕、不 Close／delete）；#5 重掃後主執行緒立刻採用新對照表；#6 `armPoll` 經 `Pci1203IoFreshMark`；低風險三處不再繞回。機台 o2 全建 0 錯、ctest 381／399（已知清單）。**程式只留機台**（依賴 IOTHREAD，#115＝C）；WORKLOG 第 131 列收進第 78 批。IO 執行緒路徑沒有卡的 ctest 起不來，上機驗由 EastSun（拔一顆驅動器電源後按 Refresh 看 WAR16152、Refresh 後按 DO、EXIT）。
- **REPORT.md（給 Jimmy）**：EastSun 看了 #127＝B／#128＝A 之後——**IO 執行緒不關、改成當天修好**；**機台節拍維持 100**（「維持 100，把今天數據回報 Jimmy」）。待機實測（release、不開 HMI、45 秒）：現行（IO 執行緒 30 ms＋IOTUNE-D）每圈平均 2.6 ms、最慢 24.7 ms、溫控 20 ms 漏跑 7 次／10 秒、ping p90 9.8 ms、CPU 96%；照專案規則（tick ≥ 3 × 最慢一圈）＝≥75 ms，100 成立；單執行緒最慢 125～141 ms 才需要 500。**負載下（HOME、空跑、Motor Test、頁面開著）還沒量**，機台端說之後補。⇒ NIGHT_REPORT §0 #131 給 Jimmy。
- web 0121：跟 0118～0120 同一顆（包本身），不收。
- **鏈尾**：C++ `81b64a21` → **`2559dc4a`**（cpp 0234）。下一次從 **cpp 0235／web 0122／tools 0166** 開始。

## 8. 13:38：cpp 0235（PASSPROF＋COMM-STOPFAST）、13:52：cpp 0236（PKG-159：機台套了第 159 包）；快照 13:52（14:4x）

- **cpp 0235＝PASSPROF＋COMM-STOPFAST**（機台自己的；EastSun 1006「請最佳化下去」；機台備份分支 `backup/pre-passprof-20261006`）：`tools/wb_serve.cpp` 在既有的 WdMark 點上做每圈分段計時（QPC，一圈 ≥ `W906_SLOWPASS_MS`（預設 100）就在 op log 寫一行 SLOW：牆鐘時間、tick 執行緒 CPU 時間、最慢 10 段）；`FastClockJobs.cpp` 的 `g_W906PhaseMark`（ctest 裡是 0）讓 `bthermo.cpp`（加熱拍、DoThermo、DoThermoReal case 100、每次 COM2 StopComm／StartComm）與 `WebMotorAccessLive.cpp` 打子段；全部打在既有行上（不移行）。`vclcompat/Comm.cpp` COMM-STOPFAST：讀取端閒置的 `Sleep(1)` 改等手動重設事件（StopComm 設它 ⇒ 讀取端 join 0.05～0.13 ms），StopComm 把埠的 handle 交給短命的關閉執行緒，StartComm／下一次 StopComm／解構等它（上限 5 秒）；golden 的重設順序（StopComm、0.5 秒、StartComm）不變。`csystem.cpp` W906-TEMPLOG-DIR（溫度紀錄的 `MyForceDirectories` 每個路徑 60 秒最多查一次）；`Pci1203ModuleCheck.h` 沒有待做的就立刻回。機台量測：COM2 StopComm 不再出現在 SLOW 行；開機視窗過後加熱 20 ms 拍每 10 秒漏 0～1 次；ping p50 1.8 ms。o2 全建 0 錯、ctest 381／399（已知清單）。
- **收進 main：第 79 批**（b19，`67372aed`，作者照機台）。`tools/wb_serve.cpp` 兩處衝突：①1203 Poll 那段——前兩行收機台的（同樣的內容＋計時點 `MotorAccessPollTick`／`LinkWatch`／`ModuleCheck`），第三行留 main 的：機台的 `if (w906IoDue || s_w906CacheEachPoll)` 屬於它的 IOTUNE（cpp 0230），跟 IOTHREAD 一樣只留機台（#115＝C）；②檔尾——main 的 W906-EXIT-ORDER 一段，接著機台的 `W906_PassProfEnd`。其他 hunk 都乾淨套上。用 oracle MinGW 6.3 探針確認：匿名命名空間函式裡的區塊 `extern unsigned long long g_ppCyc0, g_ppTsc0;` 綁到檔尾匿名命名空間的定義（nm：兩個 local 符號、沒有未定義的外部參照）。
- **cpp 0236＝PKG-159**：機台 13:45～13:52 套第 159 包（`backup/pre-pkg159-20261006`）。逐檔比對：它對每個檔的改動跟第 159 包本身（`bc2891cc..22603f5f`）完全一樣，**沒有夾帶**；跟 main 仍不同的只有已知的機台本地項目（`mymotor.cpp` 的 TRAYSAFE 歷史註解、`csystem.cpp` 的 BYPASS-WAR1603）。WORKLOG：機台改寫了自己的第 132 列並加了第 159 包的套用列 ⇒ 第 79 批照機台的版本整份收（blob `40ba1068` 相同）。
- **快照 13:52** 鏡像到 main `84233b64`（snap_push）；CHAT_JIMMY 已叮嚀。
- **給 EastSun 的機台問題**（機台 WORKLOG 132）：COM2 一直被重設＝有幾個 DTK4848 位址（看到 1／2／4／5／8）從不回應——接線或位址設定要上機看。
- **鏈尾**：C++ `2559dc4a` → **`5d30b065`**（cpp 0236）。下一次從 **cpp 0237／web 0122／tools 0166** 開始。

## 9. 14:3x：cpp 0237（CFGRETRY）；快照 14:35（15:1x）

- **cpp 0237＝CFGRETRY**（機台自己的；EastSun 1006「參數重試一定要一直重試 只是秒數可以拉長」→「1 分鐘」，另說「馬達不用每個參數都讀 讀有使用的」、站狀態「維持每次全讀」；機台計畫 `D:\HT9045\_iothread_1006\PLAN_CFGRETRY.md`，備份分支 `backup/pre-cfgretry-20261006`）。`EtherCAT/Pci1203Monitor.cpp` 6 行同行修改：沒有重試次數上限（以前 30 秒窗內 `cfgTries < 3`、所有失敗的軸同一次 Poll 一起重試）；每 60 秒開一輪、每次 Poll 只重試一軸（`cfgRetryAxis`／`cfgRoundOn`／`cfgRoundNext`／`cfgRoundTick`）；純重試（不是開卡／重掃／寫入要做的，`cfgDueNow`）時齒輪組第一個失敗就停、驅動組 Pn21D 失敗就略過其餘。603Fh 重試、扭力退避、站狀態迴圈不變。
- 機台量測（release、IO 執行緒 30 ms、閒置）：IO 執行緒用掉 74% 的一顆核；一次 Poll 85 ms＝站狀態 84 ms（線上 40 站，環 0 13 站／環 1 27 站，`Acm_DevGetSlaveStates` 每次 2.1 ms）＋DI 0.3＋軸 0.75＋DO 0.15。改之前每 30 秒一次 Poll 1.26 秒：ax14～18（站 35／36／38／39／40，不是安川）的 kCfgGear＋kCfgDrive 每次都失敗，每軸 29 次 SDO 讀約 252 ms，五軸擠在同一次 Poll。改之後最慢一次 Poll 1350 → 約 110 ms（85＋一軸重試約 24），150 秒內沒有超過一秒的 Poll。o2 全建 0 錯；ctest 382／400（已知清單）。WORKLOG 133。
- **收進 main：第 79 批**（b19 `bbee5953`，作者照機台）。一處衝突（Impl 的 `maxAxes` 那一行）：main 的那一行＋機台在那行新加的 4 個 CFGRETRY 欄位（含註解）；同一行的 `ioPub`／`ioStaged` 是機台本地的 IOTHREAD（#115＝C），照舊不收。oracle MinGW 6.3 對這支檔 `-fsyntax-only` 乾淨。WORKLOG blob 跟機台相同（`cec11bb5`）。
- ⚠ 站狀態「維持每次全讀」跟 Jimmy 13:1x UA4 ②（量完改輪流讀）不一樣 ⇒ NIGHT_REPORT §0 #137。
- **快照 14:35** 鏡像到 main `4f41dffb`；CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `5d30b065` → **`1db33362`**（cpp 0237）。下一次從 **cpp 0238／web 0122／tools 0166** 開始。

## 10. 第 79 批整合補充：oracle 編譯器的相容層、被鎖住的 ABI 探針（15:3x）

- **`QueryThreadCycleTime`**：cpp 0235 的 `W906_PassProfBegin`／`W906_PassProfEnd` 呼叫 `::QueryThreadCycleTime`；機台的 WinLibs（mingw-w64）有宣告，oracle MinGW.org GCC 6.3 的 w32api 沒有 ⇒ gate b79a（15:11）出貨組態編譯失敗。筆電加 `W906_QTCT`（`9bd972bc`，AI(W906-PASSPROF-ORACLE)）：執行時從 kernel32 取得同一個函式，拿不到就回 FALSE＝它原本的「CPU 時間未知」路徑；同行改 2 行、不移行。機台套第 161 包時這兩行會跟它的版本衝突，取 main 的即可（README 已寫）。教訓：摘機台的 C++ 之前，先用 oracle 對它動到的每個 .cpp 做 `-fsyntax-only`（0237 做了、0235 只探了 `extern` 寫法）。
- **ABI 探針**：15:17、15:18 兩次重開，cmake 設定階段四次都報 `CMakeDetermineCompilerABI_CXX.bin cannot be read`——C 的那個在，C++ 的那個一產生就不見（try_compile 本身 exitCode 0）；Defender 操作紀錄 30 分鐘內沒有事件；同資料夾手編的 C++ exe 放 45 秒仍在。換建置資料夾名（`GATE_DIR_SUFFIX=_b` ⇒ `build_ship_b`／`build_sim_b`）15:23 第一次就過。原因不明（猜是某個掃描器對那個路徑的快取判定），先記錄；再發生就同樣換名。

## 11. 15:4x：cpp 0238（SAFEPLC）；快照 15:46（15:5x）

- **cpp 0238＝SAFEPLC**（機台自己的；EastSun 1006「安全PLC 連線模式」→「都幫我補上」、「面板走 RS-232」（COM4／115200，「實體按鈕已接上」）、「好像沒有把訊息寫到視窗」；備份分支 `backup/pre-safeplc-20261006`）。安全 PLC（golden Jason 20230619：`Gerneral.ini` [System] `SafePlcIO`，Modbus TCP 172.16.8.120:502，輸入暫存器 30001～30022）：以前 `TPLCIOThread::Resume()` 是空的、`PlcComm` 是模擬 socket。新檔 `MyPLC/SafePlcClock_St02.cpp`：`SafePlcIO=1` 時一個 FastClock 1 ms 的工作（＝golden 的 `Synchronize(PLCIOProcess)`＋`MySleepEx(1)`）先跑 `PlcComm.W906_Poll()` 再照 golden 跑 `PLCIOProcess`；`ModbusTCPClient` 改成可以輪詢的真 socket；`FastClockWbServe.cpp` 同一行登記、`CMakeLists.txt` 同一行加檔、`cinitial.cpp` 閘的註解同一行更新。這台機台 `SafePlcIO` 照舊 0（PLC 有回應、22 個位元組全 0、20 分鐘沒變；IO 表沒有 SnAllEMG／SnAllSafeDoor 列 ⇒ 打開會一直跳 WAR16140）。機台本地資料（不在 repo）：`Gerneral.ini` ControlPanelMode 0→1、[TrayY] COM18→COM4、[NUMBER_PANEL2] COM4→COM14；`Error\AlarmCodeList.txt` 加 WAR16154。o2 全建 0 錯；ctest 380／400（兩支並行失敗、單獨過）。WORKLOG 134。
- **收進 main：第 80 批**（b18 `9f73eb0d`，作者照機台，乾淨套上）。動到的 `MyPLC/SafePlcClock_St02.cpp`、`MyPLC/ModbusTCPClient.cpp`、`FastClockWbServe.cpp` 先用 oracle MinGW 6.3 `-fsyntax-only` 檢查都乾淨（§10 的教訓）。WORKLOG blob 跟機台相同（`fec26702`）。
- **快照 15:46** 鏡像到 main `75c0f6c6`；CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `1db33362` → **`579076de`**（cpp 0238）。下一次從 **cpp 0239／web 0122／tools 0166** 開始。

## 12. 17:1x～17:5x：cpp 0239（PKG-160）、0240（PLCMODEL）、0241（IDX1203-LOG）、0242（TFT9050）；web 0122（PKG-160）；tools 0166；快照 17:59（18:1x）

- **cpp 0239＝PKG-160**＋**web 0122**：機台套第 160 包。逐檔比對 14 個檔：每一行改動都在第 160 包裡（`22603f5f..2afc1946`），只多 `WORKLOG_MACHINE.md` 一列（照收）——**沒有夾帶**。
- **cpp 0240＝PLCMODEL**（機台自己的；EastSun 1006「agent 分析 安全PLC.pdf 看看跟軟體是否不一樣」→「記得PLC 要用型號分支出去喔」；備份分支 `backup/pre-plcmodel-20261006`）：`Gerneral.ini` [System] `SafePlcModel=1` 時照廠商手冊（Reer MOSAIC M1S COM，p.28）讀 System I/O 區塊（FC4 0x400 起 17 個暫存器），填進 golden 的 `InPortData_Byte`；0＝golden 原本的讀法。`SafePlcIO` 仍是 0（等 E-STOP／門的位元對好）。動到 `MyPLC/MyPLC_IO_Modbus.cpp`、`cmydef.cpp`／`cmydef.h`、`database.cpp`，`tests/test_myplc_modbus.cpp` 加測項。
- **cpp 0241＝IDX1203-LOG**（機台自己的；EastSun 1006「為什麼一直送圖片上的訊號?」）：馬達斷電（E-STOP）時 golden 每一圈都對 Index 送「ST」「VS0;SP0,0,0,0;」，1203 分支每次印一行、洗版；現在照舊每次停，訊息不再洗版。`Motor/myGALILmotor.cpp`。
- **cpp 0242＝TFT9050**（機台自己的；EastSun 1006「我需要TFT 更改9050 分支 第二個到第4個 TFT 是AUTO1~AUTO3 第五個是empty COLOR 先掠過」；備份分支 `backup/pre-tft9050-20261006`）：HT9050 的 5 台 TFT 照實際接線對位址（Loader／Auto1～Auto3／Empty，Color 跳過）。`BinDisplay/BinDispBringUp_St02.cpp`。
- **收進第 81 批**（b19 `v906/jimmy-b81`，接在第 80 批 `6eaa5a16` 後面：`bc3a0423`／`d60a0a8a`／`fa63b0c5`，作者照機台；WORKLOG 列 `1c98183b`）。動到的 6 支 .cpp 先用 oracle MinGW 6.3 `-fsyntax-only` 檢查都乾淨（§10 的教訓）。同一批另併 Jerry !266、NB2-1 !272；第 80 批的 gate 跑完才開。
- **tools 0166**：機台的 HTDESIGNER 從筆電包同步（ES02 那條線），不收。
- **快照 17:59** 鏡像到 main `49dcebe2`：`Gerneral.ini` 多一行 `SafePlcModel=1`（就是 0240）＋`general.ini` 的 log 指標——真的參數變了 ⇒ CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `579076de` → **`c7c2704b`**（cpp 0242）。下一次從 **cpp 0243／web 0123／tools 0167** 開始。

## 13. 18:2x～18:3x：cpp 0243（PKG-161）；tools 0167；快照 18:37（18:4x）

- **cpp 0243＝PKG-161**：機台套第 161 包。逐檔比對 15 個檔，包外只有：`tools/wb_serve.cpp` 3 行＝機台把自己那版的 :239／:9365 換成第 161 包的 `W906_QTCT` 相容層（另一行只差行尾）——就是第 161 包的內容，**沒有夾帶**；`WORKLOG_MACHINE.md` 5 行＝機台更新自己那列「還沒完成」＋新增 161／tools 0166／0167 三列，照「機台為準」整份收進第 81 批（`928b469e`，blob 跟機台一致）。
- **tools 0167**：HTDESIGNER（ES02 那條線），不收。
- **快照 18:37** 鏡像到 main `5be11fa3`：`IO_Table.csv` 多一列 `Sensor,SnAllEMG,…`（急停感測，對應 cpp 0240 PLCMODEL 還沒對好的位元）、工單 `IOWEB_TEST_R003` 的 `Binasgn.Data` 改了 Bin／Tray 對應、`Gerneral.ini` 的 Program Close——真的參數與工單變了 ⇒ CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `c7c2704b` → **`fd219ea1`**（cpp 0243）。下一次從 **cpp 0244／web 0123／tools 0168** 開始。

## 14. 18:5x～19:0x：cpp 0244（IDX1203-LOG2）；tools 0168；快照 18:59（19:0x）

- **cpp 0244＝IDX1203-LOG2**（機台自己的；EastSun 1006「這個你開已開agent修嗎 他一直跳 剛剛講了很多次也都沒修」）：0241 用「軸在動」擋那行訊息，但 1203 沒起來（安全迴路沒通、EtherCAT 從站離線）時 `MotionDone()` 一直是 false，每圈照印；現在 `W906IdxStop` 不管動不動，30 秒最多印一次。`Motor/myGALILmotor.cpp`。**收進第 81 批**（b19 `9484cb7a`，作者照機台；oracle `-fsyntax-only` 乾淨；WORKLOG blob 跟機台一致）。
- **tools 0168**：HTDESIGNER（ES02 那條線），不收。
- **快照 18:59** 鏡像到 main `efc1e1bb`：**`IO_Table.csv` 的安全門 `SnSafeDoor2`～`8` 從 1203 IO 卡改成讀安全 PLC 的區塊（埠 301／302，類型 4）**——就是 cpp 0240 PLCMODEL 等著對的門位元；另 `Gerneral.ini` 的 Program Close、六個 `IO_Table.csv.bak_*`（機台自己的備份）。真的參數變了 ⇒ CHAT_JIMMY 已叮嚀，並另外提醒 St02-E（W-127 冷溫門讀的就是這些門）。
- **鏈尾**：C++ `fd219ea1` → **`f89efdca`**（cpp 0244）。下一次從 **cpp 0245／web 0123／tools 0169** 開始。

## 15. 19:1x：cpp 0245（PLCDOOR）；快照 19:14（19:3x）

- **cpp 0245＝PLCDOOR**（機台自己的；EastSun 1006「門警報不用看其他地方 看PLC通訊就好 請把IO_TABLE 做修正」；備份分支 `backup/pre-plcdoor-20261006`）：安全 PLC 機型 1 時，golden 的「全部安全門關」輸入（IO_Table Port 310／Bit 0）改由 PLC 的門位元給；`MyPLC/MyPLC_IO_Modbus.cpp`＋`tests/test_myplc_modbus.cpp` [5]。IO_Table 改號與 `SafePlcIO=1` 是機台資料、不在這顆。**收進第 81 批**（b19 `83a2a53f`，作者照機台；oracle `-fsyntax-only` 乾淨；WORKLOG blob 跟機台一致）。
- **快照 19:14** 鏡像到 main `5df77362`：只有 `.bak` 備份與兩份 README，參數與工單沒變 ⇒ 照規則不叮嚀。
- **鏈尾**：C++ `f89efdca` → **`ce1b26c5`**（cpp 0245）。下一次從 **cpp 0246／web 0123／tools 0169** 開始。

## 16. 19:3x～20:3x：cpp 0246（0245 重推）、web 0122～0124（PKG-160 重推）、web 0125（1203MOTNAME＋WAR16154）、tools 0169；快照 20:15（21:0x）

- **cpp 0246**：跟 0245 同一個 blob（`f72d8e4b`），機台重推，不收。
- **web 0122／0123／0124**：同一顆 PKG-160（`bc979c69`，三個 patch 檔 blob 都是 `29c25929`），包本身，不收。
- **web 0125＝1203MOTNAME＋WAR16154 文字**（機台自己的；EastSun 1006「圖片這邊我希望對應的IP有 MOT_TABLE 文字描述 和站號」、WAR16154 截圖「沒有文字描述異常」）：1203 頁「馬達」AXES 欄照 Mot_Table 標出每軸名稱與站號；網頁告警表加 WAR16154（C++ 端 `EtherCAT/Pci1203ModuleCheck.h` 早就會報，只是網頁沒有說明文字）。`web/js/pci1203/view.js`、`web/js/pci1203.js`、`web/JSON/Alarm-description.json`、`web/JSON/AlarmCodeList-index.json` 與兩支 `web/JSON/js/` 殼。**收進第 82 批** b18 `b416d701`（作者照機台）。⚠ 機台 patch 裡四支 js 的段落是 CRLF、main 的 blob 是 LF，直接 `git apply --cached` 套不上；整份轉 LF 後套（84 行，只差行尾）；`node --check` 四支 js、兩支 JSON 解析都過。（量 blob 行尾時又踩到一次 `$'\r'` 包在 `$()` 裡變空樣式、`grep -c` 回總行數的坑，改用 Python 量。）
- **tools 0169**：HTDESIGNER（ES02 那條線），不收。
- **快照 20:15** 鏡像到 main `843212f1`：`Gerneral.ini` 只有 Program Close、`general.ini` 只有 log 指標、`lastdata*.dat`、`machinerecord.dat`（執行期狀態）——參數與工單沒變 ⇒ 照規則不叮嚀。
- **鏈尾**：C++ 仍是 `ce1b26c5`（cpp 0245）。下一次從 **cpp 0247／web 0126／tools 0170** 開始。
