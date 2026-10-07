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

## 9. 12:3x～13:2x：cpp 0278～0281；快照 13:28（收進第 86 批 `v906/jimmy-b86`，b18）

- **cpp 0278 JOGDEC-3／PARAMSYNC-5**（機台每小時檢查）：停止指令改用備援時，操作紀錄寫清楚兩個呼叫；golden 從不寫寸動家族的軸（Index 軸等）不套一次性減速；手動頁面工作多算 Light Scale。收（`f190b92c`）。
- **cpp 0279 PKG-166**：**機台已套第 166 包**（GitHub `def246a6`，自動收）。包本身，不收。
- **cpp 0280 WORKLOG**：照收（`16eb437b`）。
- **cpp 0281 TEACHGO-SPD**（EastSun 1007「沒阿 我按GO 他自動條1%的」「其他的也要修 不是只有這頁」）：Teach 頁每個 GO 都照頁面速度走，速度條不再被拉回 1%。收（`a9e8598a`）。
- 第 86 批另收 ES02 HTDESIGNER 0.229～0.234（`70fc4e1d`、`f99e990b`；只動外掛，node 測試 209／0——同時有 gate 在跑時第一次是 208／1，單獨重跑全過）。第 86 批頂端 `9e838cf1`，等第 85 批進 main 後合 main、開 gate。
- 快照 13:28 鏡像到 main（`242bd1fc`）：**Mot_Table M18 MOutShuttle2 那一欄 8000→10000**（12:26 才從 5000 改成 8000）＋一個 `.bak` ⇒ 已叮嚀全體。
- **鏈尾**：C++ `323e169c` → **`83230283`**（cpp 0281）。下一次從 **cpp 0282／web 0135／tools 0170** 開始。

## 10. 13:5x～14:2x：cpp 0282；快照 14:11／14:24（收進第 87 批 `v906/jimmy-b87`，b19）

- **cpp 0282 WORKLOG**（§2 第 165～167 列＋13:58／13:59 重新連結 Debug／Release）：照收（第 87 批 `a4a909f0`）。
- 第 86 批（cpp 0278／0280／0281＋HTDESIGNER 0.234）已上 main `463f57a1`＝**第 168 包**（GitHub `dda0814c`）。
- 快照 14:24 鏡像到 main（`938ebc37`）：只有 `lastdata.dat`（＋兩份備份）與說明檔 ⇒ 參數、工單沒變，不叮嚀。14:11 那份在 14:24 之前，第一次鏡像跑到逾時沒推出去、背景重跑時已是 14:24 那份。
- **鏈尾**：C++ `83230283` → **`609f9579`**（cpp 0282）。下一次從 **cpp 0283／web 0135／tools 0170** 開始。

## 11. 14:2x～15:1x：web 0135、cpp 0283；快照 15:11（收進第 88 批 `v906/jimmy-b88`，b18）

- **web 0135 TEACH-SPDMEM**（EastSun 1007「speed 可以不要我每go 一次 就1%嗎 可以記憶住?」）：Teach 頁按 GO／換馬達／按 Set 之後，速度欄與速度條保留操作員設的值（空白或不合法才給 1）。照收（第 88 批 `80b3389d`）。
- **cpp 0283 HT9050-TRAYX-ARMY**（EastSun 1007「這圖片上用9050分支 幫我取消掉」，截圖是「Please let InArm Y at home position first!!」）：`forms/fTeach.cpp` `TfTeach::CheckCanMove`，Teach 頁移動 Tray X 時，**只有 HT9050** 跳過「InArm Y／OutArm Y 要在原點」兩個拒絕（golden `uteach.cpp:1156-1167`；機種判斷照 cinitial／TeachKb 的 `W906_GpibModel == "9050GPIB" || Type_HT9050`）；氣缸 `C_TrayX_UpDown` 檢查與其他機種照 golden。照收（第 88 批 `e64ac4cf`）。⚠ 這是放寬一道手動畫面的互鎖——EastSun 本人在機台旁要的（機構事實歸他），NIGHT_REPORT 記給 Jimmy 知道，不是決策題。
- gate b88a（兩組態 32 分）：基準＋`Stream2E_PagePolls`——那支測試只切 Teach 頁 `var teachMotorConfig=null`～`teachLoadMotors();` 一段到 node vm 跑；0135 讓那段呼叫 `teachNum`，它定義在段落後面（瀏覽器裡會提升，vm 裡沒有）。筆電只改測試（`14849f0d`：測試也跑頁面自己的 `teachNum`，不是替身），新舊頁面都 27／0，單獨重跑兩組態通過。
- 第 88 批上 main `c55a7c97`＝**第 170 包**（GitHub `e8c11069`；第一次推送 HTTP 408，重推成功）。
- 快照 15:11 鏡像到 main（`f6691973`）：**`teach.ini` 改了（18 行）**＋`lastdata.dat` ⇒ 教導值變了，CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `609f9579` → **`84a6aac6`**（cpp 0283）。下一次從 **cpp 0284／web 0136／tools 0170** 開始。

## 12. 16:1x：web 0136；快照 16:11（收進第 90 批 `v906/jimmy-b90`，b18）

- **web 0136 TEACH-9050HIDE**（EastSun 1007「這些用不到 用9050分支 隱藏」「你整個畫面可以像 dockpanel 嗎? 我隱藏一個方塊 下面往上移」）：`page/ht9045_teach_3axes_c.js`，只有 HT9050 隱藏 Teach 頁用不到的 Tray Arm 控制項（OCR／Clean／Color 的 Set、GO、欄位，Tray X U/D，Suck On／Off，Tray Mapping），其餘方塊往上補位。照收（第 90 批 `a7d8950c`，直接接在批次 HEAD 上；`node --check` 過）。
- 第 89 批（Jimmy 的 !301＋St02 !303）已上 main `09154f16`＝**第 171 包**（GitHub `437d425f`）。
- 快照 16:11 鏡像到 main（`a88c5079`）：`Mot_Table.csv` M35 MLoaderZ 兩個值 40000→60000、`teach.ini` 的 Tray X 教點整批重教 ⇒ CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ 仍是 **`84a6aac6`**（cpp 0283）。下一次從 **cpp 0284／web 0137／tools 0170** 開始。

## 13. 16:2x～16:5x：cpp 0284～0286、web 0137～0138；快照 16:58

- **cpp 0284 NOTE-JIMMY-TEACHDATA**（EastSun 1007「幫我把點位資料全部推上去 並且跟jimmy說」）：新文件 `docs/NOTE_JIMMY_TEACHDATA_20261007.md`——點位在 GitHub `machine/integ-ioweb` 的 `machine_params\`（主要看 `runcfg\system\teach.ini`）、重點值、機台上的暫時設定。**cpp 0285**＝WORKLOG 第 168 列；**cpp 0286**＝那份說明的 Out Shuttle Z 一行更正＋WORKLOG 第 169 列。三顆都只有文件，**直接 cherry-pick 上 main**（`79f4b921`／`dda9e59d`／`21c12031`，不用 gate）。
- **web 0137 TEACH-OUTSHTZ-POS**（把 Index 頁 Out Shuttle Z 的教導鈕搬到 EastSun 找得到的位置）與 **web 0138**（把 0137 撤回：EastSun 說 Index Z 高度在別頁就能設）＝改了又撤回，淨變化 0 ⇒ **不收**。
- 第 90 批（web 0136＋St02 !304 W-140）已上 main `6ab8edd5`＝**第 172 包**（GitHub `36621358`）。
- **鏈尾**：C++ `84a6aac6` → **`44d84921`**（cpp 0286）。下一次從 **cpp 0287／web 0139／tools 0170** 開始。

## 14. 17:0x：cpp 0287；快照 17:06

- **cpp 0287 WORKLOG** 第 170 列（EastSun「我實體按鈕案home都沒反應」）：機台只查不改——面板按鍵封包只到 09:15:57，最後一筆 HOME＝`t050400002020`（HOME 0x20＋ALARM RESET 0x2000 卡住，golden ScanPannelKey 的 ALARM RESET 判斷蓋掉 HOME）；程式跟 golden 相同，請查面板 ALARM RESET 鈕／按鍵線。只有文件，直接 cherry-pick 上 main（`a399d0ca`）。
- 快照 17:06 鏡像到 main（`23153562`）：`teach.ini` 26 行（已叮嚀）；兩支 general.ini 只有 GPIB 記錄檔路徑與 Program Close 旗標。
- **鏈尾**：C++ `44d84921` → **`fec6c4bc`**（cpp 0287）。下一次從 **cpp 0288／web 0139／tools 0170** 開始。

## 15. 17:2x～17:5x：cpp 0288、tools 0170；快照 17:31

- **cpp 0288 RSTHELD**（EastSun 1007「就算reset卡住其他按鈕不是也要可以用?」，機台的 Claude 問過他、他選「改」）：`ckernel.cpp` ScanPannelKey 前後面板四個 ALARM RESET 判斷——golden 放在最後、會蓋掉 ret，按住（卡住）時 bK 鎖存讓每一拍都回 -1，其他鍵全被吃掉（09:15:57 的 `t050400002020`＝HOME＋卡住的 RESET）。改成：只有沒有別的鍵（ret==-1）或 RESET 鎖存還沒設時才由 RESET 接手——新按的 RESET 照 golden 優先，按住的讓出來。**跟 golden 不同，是 EastSun 本人的決定**（RULINGS_20261006 第 21 條）。`tests/test_scankey_golden.cpp` 加一段（新按→RESET、按住→-1、按住 RESET 時按 HOME→HOME、同時新按→RESET）。收進第 92 批（`245e707a`）：那支測試跟 main 上 St02 後加的 [18]／[19] 兩段衝突（機台在第 166 包，沒有那兩段）⇒ 兩邊都留；gate b92a 17:5x 重開（原本 17:51 開的那次停掉，建置程序清掉、資料夾重建）。
- **tools 0170 HTDESIGNER**＝機台把筆電第 162／163／167／168 包的 HTDESIGNER 合進它自己那份（「機台合筆電的包」）⇒ **不收**。下一次 tools 從 **0171** 開始。
- 快照 17:31 鏡像到 main（`4aadf9aa`）：只有兩份說明檔 ⇒ 參數、工單沒變，不叮嚀。
- **鏈尾**：C++ `fec6c4bc` → **`29180f7d`**（cpp 0288）。下一次從 **cpp 0289／web 0139／tools 0171** 開始。

## 16. 17:5x～18:1x：cpp 0289～0291；第 92 批上 main

- 第 92 批（cpp 0288＋St02 !308／!309＋realfile_guard 保護 Offset 資料夾）上 main `6690e980`＝**第 174 包**（GitHub `417e3cde`）。gate b92a 兩組態多一支 **ScanKeyGolden**：機台加在 `tests/test_scankey_golden.cpp` 的 RSTHELD 那段，在 main 上排在 St02 後加的 [18]／[19] 後面，而 [19] 留下 `InitialOK=false` 等狀態（機台樹在第 166 包，沒有那兩段）⇒ 筆電把那段移到 [18] 前面（只動測試順序，`b0ba0894`），兩組態單獨重跑通過。
- **cpp 0289 PKG-167-172**＝機台 17:57 套第 167～172 包的合併 ⇒ 不收，但**要接進重建鏈**（後面的 patch 以合完的樹為底；這一輪先只接 0290 失敗，補上 0289 才套得上）。
- **cpp 0290 ALARMTIME**（EastSun「出現異常的速度慢」）：`tools/wb_serve.cpp` ForwardShowErrorMessage 量每個擋機警報從進來到送進網頁的時間（停馬達／記錄／權限＋送出），寫一行 `ALARMTIME <碼> stop N ms, record N ms, auth+post N ms, total N ms` 進操作記錄；行為不變（實測 0.5～3.6 秒）。
- **cpp 0291 PADNOTE**（EastSun「實體按鈕home還是沒反應」）：`PadInterface_St02.cpp` ProcessReceiceData 收到不是燈號回音（類型 "90"）的封包就立刻寫一行 `PADKEY <封包>` 進操作記錄（原本的面板記錄要滿 1000 行才落檔）；行為不變。⚠ 動到 St02 的檔（同一行＋檔尾），已在 TO_STEVEN 告知。
- 兩顆收進**第 93 批**（`v906/jimmy-b93` `24df1daa`；WORKLOG 衝突＝機台的第 172、173 列照收），gate b93a 18:33 起。
- **鏈尾**：C++ `29180f7d` → **`e1ed9db3`**（cpp 0291）。下一次從 **cpp 0292／web 0140／tools 0171** 開始。

## 17. 18:3x～20:2x：cpp 0292～0299；快照 18:36／19:18／19:25／20:19；第 94 批上 main

- **cpp 0292 REQUEST-JIMMY-IOPANEL**（EastSun「請派工給jimmy 把這個頁面程式碼補齊」）：`docs/REQUEST_JIMMY_IOPANEL_20261007.md` ⇒ 卡 W-155 給 St02，**St02 !315（W-155 A）已在第 94 批**。
- **cpp 0293 WORKLOG**（第 176 列：實體 HOME 被後面板 Safe Lock 擋住，未改）。
- **cpp 0294 Z1GUARD＋PRODREFRESH**（EastSun：Pick up 12.99＝入料飛梭 Index Z 取點、Release 14.99＝出料飛梭放點，照 golden 加在教點上）：空跑 Step5／Step9 的目標在原點或以上就擋；`wb_serve.cpp` ReloadRecipeDocAfterSave 存檔後立即 `SetTechDataToProd_Index()`。
- **cpp 0295 SUCK9050**（EastSun「HT9050 不換嘴」，推翻 RULINGS_20260925 第 5 條的 HT9050 部分——本人決定）：`cinitial.cpp` ChangeSite 在 9050GPIB／Type_HT9050 不做 Single Site 的 Aa 換嘴。
- **cpp 0296 VU9050**（EastSun「有4組就顯示4組」）：真空頁在 HT9050 依 IO_Table 的 Enable 顯示所有 Index 吸嘴。
- **cpp 0297 REQUEST-JIMMY-OVERLAP**（EastSun「請派工jimmy 偵測所有畫面是否有擋到」）：`docs/REQUEST_JIMMY_OVERLAP_20261007.md` ⇒ 卡 W-161 給 ES02。
- **cpp 0298 BRAKE-LEFTOVER**（EastSun 選「接受留下來的 Servo ON」，推翻 RULINGS_20261003 第 24 條——本人決定）：`WebMotorAccessLive.cpp` 重開後沿用上次的 Servo ON 也放開煞車。
- **cpp 0299 PRODSWITCH**（EastSun「我現在要測試 9050 正常流程 請切換流程」）：`Ht9050DryRun.cpp` W906_Ht9050DryRunOn 除了 MachineType.h 的開關，還要 Gerneral.ini `[System] W906_HT9050DryRun=1`；沒有／0＝HT9050 正式流程。⚠ 20:19 快照的 M00／M01／M19／M20 軟體極限仍是 ±999999 ⇒ TO_ES02 ⛔ 警告（W-162，St01 W-62 B2）。
- 全部收進**第 94 批**（`v906/jimmy-b94`，b19），衝突：合 main 時 `tests/CMakeLists.txt` 兩處「兩邊都接在 St02_W140Command 後面」（W-150 與 W-152 的測試區塊、再來 W-155）⇒ 兩塊都留、各自逐字等於自己那一邊（`resolve_append_conflict.py`）；機台 patch 本身沒有衝突。
- 快照：18:36／19:18（`37cef908`，Index Z 教點改 0，已叮嚀）／19:25（`6069004d`，只有記錄檔）／20:19（`d5bf700c`：工單 Contact.Data Pick Up −75.8、Place −74.8、AutoKSHTOfs 2.00、Torque；Mot_Table M108 回原點方向；teach.ini OutSht2Left——已叮嚀）。
- 第 94 批上 main `56b065d5`＝**第 177 包**（GitHub `3308172f`）；gate b94a：兩組態 32 分鐘，只有固定失敗（出貨 4、模擬 7）。
- **cpp 0300 WORKLOG**（第 183 列，EastSun「照這個是對的」：teach.ini [MTestZ1] In Sht Z／Out Sht Z／WaitTestZDown／Safe 刻意存成 0，Index Z 的高度全由工單 Contact.Data [Test Arm1] 給：取料 −75.8 mm＝−7580、放料 −74.8 mm＝−7480）：接進鏈（`f9efab08`），只有 WORKLOG，跟第 95 批走。
- **鏈尾**：C++ `e1ed9db3` → `1d39a31e`（cpp 0299，第 94 批）→ **`f9efab08`**（cpp 0300，第 95 批）。下一次從 **cpp 0301／web 0140／tools 0171** 開始。

## 18. 20:4x：cpp 0300；快照 20:44；第 95 批上 main

- **cpp 0300 WORKLOG**（第 183 列：Index Z 教點刻意 0、高度全由工單 Contact.Data 給）收進**第 95 批**（`v906/jimmy-b95`，b18），連同 St02 !316（W-155 B 網頁版 Pad 介面視窗）。
- 快照 20:44（`3a679bc0`）：只有記錄檔 ⇒ 參數、工單沒變，不叮嚀。
- 合 !316 時 `tests/CMakeLists.txt` 又是「兩邊都接在同一行後面」，而且 !316 的分支本身帶著 main 已有的 W-152／W-152 (B)／W-155 A 三個測試區塊（它的基底順序不同）⇒ 第一次解完有重複的同名測試目標（會讓 CMake 設定失敗），用 `dedupe_cmake_blocks.py` 逐字比對後刪掉第二份；`resolve_append_conflict.py` 補上「比 origin/main 多出來的同名目標」檢查（同一個坑不再靠眼睛）。
- 第 95 批上 main `f63a4099`＝**第 178 包**（GitHub `7e7b5bd4`）；gate b95a：兩組態 33 分鐘：固定失敗之外只有 WebPageTable（已補 C++ 那一列，相關 9 支兩組態全過）。
- **cpp 0301 FP9050-MACHINE＋Z1SAFE0**（EastSun「用正常的流程模擬跑一次」→ 機台上選「機台也跑 Frank 的流程」「用原點 0」——本人決定）：`csystem.cpp` DoAllProcess 的 Index 那一格 `#ifdef SOFT_SIMULTE`→`#if 1`（HT9050 在出貨組態也跑 Frank 的 DoTestHeadMotorFP，其他機種不變）；`cinitial.cpp` HT9050 的 TestZ1_Safe＝TestZ2_Safe＝0（原本被拉到 golden 的最小值 +200，在 HT9050 是原點上方，飛梭等「Z1 在安全高度」永遠等不到）。會動流程 ⇒ **第 96 批**全量 gate。也回答了 Frank01 F-04 的第①題（關空跑後 HT9050 正式 Index 用 FP）。
- **鏈尾**：C++ `f9efab08`（cpp 0300，第 95 批）→ **`fc9fa3bf`**（cpp 0301，第 96 批）。下一次從 **cpp 0302／web 0140／tools 0171** 開始。

## 19. 21:0x～21:5x：cpp 0301～0304；快照 21:14；第 96 批上 main

- **cpp 0301 FP9050-MACHINE＋Z1SAFE0**（見 §18；EastSun 在機台上選的）、**cpp 0302 TESTERMODE**（EastSun：主畫面 Tester 框要顯示 On-Line／Off-Line——`WebBridgeTags.cpp` 照 golden labTesterMode 寫 `tester.name`）、**cpp 0303 OFFLINE-NOBRIDGE**（EastSun「GPIB的部分先Pass掉，用Off-Line去進行」「Index Z等待是在哪邊?」：Frank 的 HT9050 接觸步驟進 GetTesterResult 時，OFF_LINE 也要找 GPIB 橋接視窗，找不到就每 3 秒回 case 1、Index Z 一直壓著；現在 OFF_LINE 兩處檢查都放行、走 dummy 測試時間，ON_LINE 照 golden）、**cpp 0304 WORKLOG**（第 187 列：機台把 MInArmY／MOutShuttle1／MOutShuttle2／MOutArmX 的 InitSpeed 調到不超過 JogHigh，EastSun 同意，有備份）——四顆乾淨 cherry-pick，收進**第 96 批**（`v906/jimmy-b96`，b19）。
- 快照 21:14（`e7f1ff3a`）：工單 Contact 下壓高度 −50.00→−100.00，已叮嚀全體（`f5e8fa3c`）。
- 第 96 批上 main `6da66c25`＝**第 179 包**（GitHub `cfbb6b15`）；gate b96a：兩組態 33 分鐘，只有固定失敗（出貨 4＋FastClk_Jobs 負載逾時、單獨重跑過；模擬 7）。
- **鏈尾**：C++ `fc9fa3bf` → **`594b50ee`**（cpp 0304）。下一次從 **cpp 0305／web 0140／tools 0171** 開始。
