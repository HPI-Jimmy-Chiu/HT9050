# 機台 patch 收件紀錄 20261007

> 接 `MACHINE_PATCHES_20261006.md`（第 16 節）。筆電迴圈 10/06 22:16 斷掉、10/07 08:2x 接回；這段時間機台推了 cpp 0247～0258、web 0126～0131，tools 沒有新的（最大仍是 0169）。

## 1. 08:3x～09:0x：cpp 0247～0258、web 0126～0131；快照 08:10（全部收進第 83 批 `v906/jimmy-b83`）

| 編號 | 內容 | 處置 |
|---|---|---|
| cpp 0247 ERRPART | 告警說明那一行照 golden 帶 `" : "+errPart`（例：哪一個 1203 模組失敗） | 收（`d082b4e2`） |
| cpp 0248 PKG-162、0249 PKG-163 | 機台套筆電第 162／163 包 | 包本身，不 cherry-pick；WORKLOG 的兩列照鏈尾收 |
| cpp 0250 TRAYZ-PITCH | HT9050 疊盤 Z＝基準層 − 層數×pitch，Teach 頁新增「9050 Tray Z」區塊；全機 HOME 1310 步 Loader Z 改走教導的 Home（`W906_TrayZ9050Home(0)`） | 收（`d30c7601`）；`uhome.cpp` 一處衝突以機台為準（main 原本是 `Prod.TrayZ_Home`，HT9050 上恆為 0） |
| cpp 0251 PLCDOOR-2 | 安全 PLC 的門真的會告警（IO 稽核找到的兩個缺陷） | 收（`f2c812dd`） |
| cpp 0252 REVIEW-1 | 安全 PLC 斷線告警、Tray Z 打字列沒有 Set／GO、IO 執行緒卡住不拖住主迴圈 | 收（`9ae34477`）；`EtherCAT/Pci1203Monitor.cpp` 的改動全是 IO 執行緒（W906-IOFIX-2）⇒ **不收**（§0 #115＝C）；`WebMotorAccess.cpp` 檔尾兩段（main 的 Teach Home All＋機台的 `W906_TeachTypedOnly`）都留 |
| cpp 0253 REVIEW-2 | REVIEW-1 的三個缺陷；告警號改成 WAR16156 | 收（`199613b8`）；`Pci1203Monitor.cpp`（IOFIX-3）與 `WebMotorAccessLive.cpp` 那一行（看 IO 執行緒心跳才有意義）不收 |
| cpp 0254～0258 1203REOPEN（1～4、-2、-3） | 開卡異常或模組沒偵測到時的「重新開卡」：環網重置（`Acm_MasResetRing`）→ 重掃站 → 重開軸，不行就退回整卡 Rescan；1203 頁加按鈕；重置後重建 IO 寬度與 IO 對照表 | 收（`784a8bb9`～`500a5944`）；IO 執行緒的 4 處不收：`TPci1203Control::Execute()` 開頭轉交 IO 執行緒的代理、`RescanRing()` 交給 IO 執行緒的那行、重開後重啟 IO 執行緒（`W906_IoThreadBoot`）、Impl 的 `ioPub`／`ioStaged` 欄位 |
| web 0126 | 跟 web 0125 同一顆（1203MOTNAME）重推 | 已在第 82 批 |
| web 0127 PKG-163 | 包本身 | 不收 |
| web 0128～0131 | TRAYZ-PITCH 網頁、REVIEW-1／2 告警表（WAR16155 → WAR16156）、1203 頁「⟲ 重新開卡（再試 3 次）」 | 收（`1f9180f1`～`f3803304`）；JSON／JS 語法檢查全過 |

- **oracle 編譯器的兩處修正**（機台的編譯器有、MinGW.org 6.3 沒有）：①`tools/wb_serve.cpp` 的 `_putenv`（1203REOPEN 用）沒有宣告 ⇒ 照 `tests/test_agv_e84.cpp:158-163` 加三行條件宣告（`abf5ceef`，語法預檢抓到）；②`tests/test_myplc_modbus.cpp` 用 `std::this_thread::sleep_for` ⇒ 同行改成 `::Sleep(1100)`（`b43186db`；gate b83a 第一次 08:50 就停在這裡，08:57 重開）。**請機台下次收包時照收這兩處**，兩邊才一致。
- **WORKLOG_MACHINE.md**：照機台鏈尾整份收（多 PKG-162／163 兩列，`e8f0dd6b`）。
- **快照 08:10** 鏡像到 main `a23e7ea6`：`Gerneral.ini` `SafePlcIO=0→1`；`IO_Table.csv` 安全門 1～8 改走安全 PLC 位址（301／302），新增 `SnAllSafeDoor`（310）⇒ 參數真的變了，CHAT_JIMMY 已叮嚀全體同步後再測。
- **鏈尾**：C++ `ce1b26c5` → **`ce0edb9f`**（cpp 0258）。下一次從 **cpp 0259／web 0132／tools 0170** 開始。

## 2. 09:5x～10:0x：cpp 0259～0262、web 0132；快照 09:38（收進第 84 批 `v906/jimmy-b84`）

| 編號 | 內容 | 處置 |
|---|---|---|
| cpp 0259 1203REOPEN-4 | 重新開卡流程審查找到的兩個 bug | 收（第 84 批 `c3ca0d09`，乾淨套上；只有一行註解提到 IO 執行緒，程式沒依賴它） |
| cpp 0260 NOTICE-DEFER-3 | 馬達卡料通知畫面按 PAUSE／RESET 立刻反應（EastSun 1007「按 pause 10 秒後才有反應」） | 收（`fff84403`） |
| cpp 0261 PKG-164 | **機台 09:35 套好第 164 包** | 包本身，不收；WORKLOG 照鏈尾收（`debdf5e1`） |
| cpp 0262 PLCDOOR-DEB | 安全 PLC 的門訊號要連續開 0.5 秒才算開（防彈跳） | 收（`d55a5542`）；測試新加的 3 處 `std::this_thread::sleep_for` 照 b43186db 同行改成 `::Sleep`（`b55b7934`，oracle 沒有 std::thread） |
| web 0132 RTFDESC | 告警視窗的說明顯示文字，不是 RTF 原始碼 | 收（`5d157b2b`，JS 語法檢查過） |

- oracle 語法預檢（模擬＋出貨旗標）：`Pci1203Monitor.cpp`、`wb_serve.cpp`、`MyPLC_IO_Modbus.cpp`、`uHGemHT9045.cpp` 全過。
- **快照 09:38** 鏡像到 main `3a8f8d3d`：`Mot_Table.csv` M14 MTestZ1（200→20000）、M30 MTrayX（150000→200000）兩個 1203 軸參數；`runcfg/system/teach.ini` 新增 TRAYZ-PITCH 的 27 個「9050 Tray Z」教點，**全是 0（還沒教）**；`Gerneral.ini` 只有 Program Close ⇒ 參數真的變了，CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `ce0edb9f` → **`252e8c23`**（cpp 0262）。下一次從 **cpp 0263／web 0133／tools 0170** 開始。

## 3. 10:0x：cpp 0263；快照 10:02（收進第 84 批）

- **cpp 0263 FLUSH-100**：主畫面狀態字與塔燈／面板燈回到 100 ms 節拍更新（`WebBridgeTags.cpp`、`tests/test_wb_simpump.cpp`）。收（第 84 批 `f4d01ae3`，乾淨套上；oracle 語法預檢過）。
- **快照 10:02** 鏡像到 main `ce599461`：`IO_Table.csv` **拿掉入／出料取放的 Drop 氣缸**（`C_InPnPDrop1～4`、`C_OutPnPDrop1～4` 與它們的 On／Off 列；機台備份 `IO_Table.csv.bak_dropcyl_20261007_094218`）⇒ 參數真的變了，CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `252e8c23` → **`71668ec3`**（cpp 0263）。下一次從 **cpp 0264／web 0133／tools 0170** 開始。

## 4. 10:2x：cpp 0264～0266；快照 10:20（收進第 84 批）

- **cpp 0264 DOCS**：EastSun 給 Jimmy 的請求「所有警報都要顯示說明」（`docs/REQUEST_JIMMY_ALARM_DESC_20261007.md`＋`docs/alarm_audit_20261007.csv`，機台只讀盤點、沒改程式）。收（`373e18e6`）⇒ 轉成工作卡 **W-142 給 St02-E**（TO_STEVEN §4 10:3x）。
- **cpp 0265 NOTICE-DEFER-4**：MyMessageBox 等待中提早的 PAUSE 也先過同一道權限檢查（`tools/wb_serve.cpp`）。收（`dd45900a`；oracle 語法預檢過）。
- **cpp 0266 WORKLOG**：第 147～156 列。照鏈尾收（`f3c882ed`，blob 與鏈尾相同）。
- **快照 10:20** 鏡像到 main `4ce777bf`：只有 README_PARAMS／README_WORKORDER 兩個說明檔 ⇒ 參數沒變，不叮嚀。
- **鏈尾**：C++ `71668ec3` → **`3a1089bc`**（cpp 0266）。下一次從 **cpp 0267／web 0133／tools 0170** 開始。
