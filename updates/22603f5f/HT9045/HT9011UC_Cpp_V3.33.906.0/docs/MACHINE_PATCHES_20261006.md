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

- **cpp 0232＝IOTUNE-D**（機台自己的；EastSun「先更新 再跑最佳化執行緒」）：`tools/wb_serve.cpp` 的 io 網頁快取（約 148 KB）只在最近 3 秒有頁面讀過才重建（原本每 100 ms 一拍都重建、每次 18 ms＝主迴圈 18%%，而沒有人讀）；motor 那份照舊每次重建；環境變數 `W906_APICACHE_IO_ALWAYS=1` 回到舊行為。跟 IO 執行緒無關 ⇒ **收進 main**：b19 `v906/jimmy-b76` `1fe677ed`（作者照機台），第 76 批 gate `b76a` 09:31 起。已知取捨：閒置 3 秒後第一次讀 io/runtime 拿到上一份（下一次輪詢就新）；沒有 ctest 讀線上快取。
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
- **REPORT.md（給 Jimmy）**：EastSun 看了 #127＝B／#128＝A 之後——**IO 執行緒不關、改成當天修好**；**機台節拍維持 100**（「維持 100，把今天數據回報 Jimmy」）。待機實測（release、不開 HMI、45 秒）：現行（IO 執行緒 30 ms＋IOTUNE-D）每圈平均 2.6 ms、最慢 24.7 ms、溫控 20 ms 漏跑 7 次／10 秒、ping p90 9.8 ms、CPU 96%%；照專案規則（tick ≥ 3 × 最慢一圈）＝≥75 ms，100 成立；單執行緒最慢 125～141 ms 才需要 500。**負載下（HOME、空跑、Motor Test、頁面開著）還沒量**，機台端說之後補。⇒ NIGHT_REPORT §0 #131 給 Jimmy。
- web 0121：跟 0118～0120 同一顆（包本身），不收。
- **鏈尾**：C++ `81b64a21` → **`2559dc4a`**（cpp 0234）。下一次從 **cpp 0235／web 0122／tools 0166** 開始。
