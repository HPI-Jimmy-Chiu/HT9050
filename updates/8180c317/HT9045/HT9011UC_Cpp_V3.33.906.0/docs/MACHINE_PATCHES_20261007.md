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

## 5. 收進 main（10:3x～10:4x）

- 第 83 批 `185ae2ab`（第 1 節的機台 cpp／web＋NB2-GPT 7 顆＋!285／!287／!288／!289／!290／!293＋oracle 兩處修正＋兩支測試修正 `d74e577e`）；**GitHub 第 165 包 `0ff4584c`（10:39）**。
- 第 84 批（第 2～4 節：cpp 0259／0260／0262～0266、web 0132＋Ifor01 !292＋St02 !295）b18 `36474cbd`，gate b84a 10:40 起。

## 6. 10:4x～11:0x：cpp 0267～0268、web 0133～0134；快照 10:57／10:59（收進第 85 批 `v906/jimmy-b85`）

- **cpp 0267 PARAMSYNC**：馬達參數在開機與打開 Motor Test／Teach 頁時寫進 1203 卡（開機 InitMotor 的 MaxAcc／MaxDec 用 Mot_Table 的 100%，照 golden cinitial.cpp:3616／:3643）。收（第 85 批 `aff11fa6`）；`tests/test_web_motor_access.cpp` 檔尾兩邊各加一段函式（St02 !285 的 POOL-5 檢查＋機台的 PARAMSYNC 測試），git 把共同的結尾 `}` 抽到衝突外 ⇒ 兩段都留、中間補一個 `}`。
- **cpp 0268 PKG-165、web 0133**：**機台 10:55 已套第 165 包**（含 NB2-GPT 的物流模擬修正）。包本身，不收；WORKLOG 照鏈尾。
- **web 0134 TRAYZ-LAYOUT**：Teach 頁「9050 Tray Z」區塊不再蓋住 Tray Arm 頁。收（`5bf492e5`）。
- 快照 10:57／10:59 鏡像到 main（`272d1c22`）：只有 Program Close 與說明檔 ⇒ 參數沒變，不叮嚀。
- **鏈尾**：C++ `3a1089bc` → **`bab01064`**（cpp 0268）。下一次從 **cpp 0269／web 0135／tools 0170** 開始。

## 7. 11:1x～11:2x：cpp 0269～0270；快照 11:22（收進第 85 批 `v906/jimmy-b85`）

- **cpp 0269 WORKLOG**：機台工作紀錄第 165 列（套第 165 包＋Teach Tray Z 版面）；PARAMSYNC 10:54 實機驗過（開機／Teach／Motor Test 寫卡 0 失敗、讀回 133 項一致）。照收（`bdff8b53`）。
- **cpp 0270 PARAMSYNC-2／NOTICE-DEFER-5**：機台自己每小時的檢查抓到兩個真錯——①開機 InitMotor 一次做完 18 軸，主迴圈卡 16.5 秒（10:53 RESET 16 秒才回）⇒ 每輪只做一軸、20 秒沒有伺服回報的軸略過、開 Motor Test／Teach 頁不再等開機 InitMotor；②通知框「授權已過」的記號原本記通知的 tag，被別的等待吃掉時會留下來，下一次同一則通知的 PAUSE 就跳過密碼 ⇒ 改記指令編號（只用一次）。D026 的釘字串機台一起改了。收（`4c786641`），兩顆都乾淨套上。
- ⚠ 第 84 批 gate 的 `D026_NoteAuth` 失敗＝cpp 0265 NOTICE-DEFER-4 改了它釘的那一行：第 84 批先照 0265 改測試；第 85 批合 main 時 `tests/test_note_auth.cpp` 那一行以機台（0270）為準。
- 快照 11:22 鏡像到 main（`b421e6f6`）：**EastSun 暫時停用安全門**（IO_Table 9 個門感測器 Enable 1→0、Gerneral.ini `SafePlcIO` 1→0；機台留了紀錄檔 `IO_Table_safedoor_off_20261007_111239.txt` 與整檔備份，之後要設回來）——已叮嚀全體。
- **鏈尾**：C++ `bab01064` → **`3e8282d5`**（cpp 0270）。下一次從 **cpp 0271／web 0135／tools 0170** 開始。
- **cpp 0271 WORKLOG**（11:22）：§2 第 157～160 列（PARAMSYNC 實機驗過；PARAMSYNC-2／NOTICE-DEFER-5；Teach Tray Z 版面；安全門在 IO_Table 停用並留紀錄；`SafePlcIO` 機台的 Claude 改設定被權限檢查擋下、留給 EastSun 自己改）＋11:21 建好 Release wb_serve。照收（第 85 批 `46cdfe5e`）。**鏈尾 `3e8282d5` → `7477383b`**，下一次 cpp 0272／web 0135／tools 0170。

## 8. 11:4x～12:2x：cpp 0272～0277；快照 12:26（收進第 85 批 `v906/jimmy-b85`）

- **cpp 0272 JOGDEC**（EastSun 1007「我有的時候JOG 放開後 沒有馬上停 一直過去」；機台的裁決「JOG 一律用 Mot_Table 加減速 100%」）：11:41 量到 MOutArmZA 以 33 % 寸動、加減速 80（＝Mot_Table 8000 的 1 %），停止指令 30 ms 就到卡，但軸滑了 2.6 秒、約 7700 counts ⇒ 寸動家族的 Acc／Dec 改用 Mot_Table 值，放開時用 Acm_AxStopDecEx 以該 Dec 停。收（`40b5d9ae`）；`tests/test_web_motor_access.cpp` 檔尾又是兩邊各加一段函式（第 85 批已有的 PARAMSYNC 測試＋機台的 JOGDEC 測試），git 把共同的結尾 `}` 與分隔線抽到衝突外 ⇒ 兩段都留、中間補 `}` 與分隔線；main() 兩支都有呼叫；兩組態 `-fsyntax-only` 過。
- **cpp 0273 PARAMSYNC-3**：沒歸原點的機台（iHome＝1）參數同步一直延後、永遠不寫 ⇒ 忙碌判斷改看 Home Monitor 是否開著。收（`edaad2cf`）。
- **cpp 0275 JOGDEC-2**（機台每小時檢查抓到的嚴重錯）：Acm_AxStopDecEx 的 NewDec 超過軸的 CFG_AxMaxDec 時會被卡拒絕、而且後面沒有補救 ⇒ 任何路徑都可能「完全沒停」；現在失敗就立刻再下一般的 Acm_AxStopDec。另外只有寸動放開那一次用 Mot_Table Dec（用一次就清掉）。收（`1e25b3b5`）。
- **cpp 0276 PARAMSYNC-4**：手動頁面工作（寸動、單軸歸原點、Motor Test 迴圈、Arm Cell、手動教導）進行中時，參數同步要等，不然後段會用錯速度。收（`713c22c0`）。
- **cpp 0274／0277 WORKLOG**：照收（`0cff7973`、`bb06342d`）。產品檔（Pci1203Control.cpp、WebMotorAccess.cpp、WebMotorAccessLive.cpp）出貨組態 `-fsyntax-only` 過。
- 快照 12:26 鏡像到 main（`41a34470`）：**Mot_Table 改了 M18 MOutShuttle2（5000→8000）與 M19 MOutArmX（80000→20000）**，另有兩個 `.bak` 備份檔 ⇒ 已叮嚀全體。
- **鏈尾**：C++ `7477383b` → **`323e169c`**（cpp 0277）。下一次從 **cpp 0278／web 0135／tools 0170** 開始。
