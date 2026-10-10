# 機台 patch 收件紀錄 2026-10-09

> 接 `MACHINE_PATCHES_20261008.md`（§19 記到 cpp 0339 PKG207，C++ 鏈尾 `7251baf1`）。

## 1. 01:2x～01:3x：cpp 0321／0322／0331 的模擬組態紅，量出原因都在測試 ⇒ 第 125 批收進 main（第 208 包）

- **怎麼量**：b19 從 main `4270eedf`（第 124 批）疊 cpp 0331（`5fe1bc7e`）、0321（`adb3a720`）、0322（`dc3d4247`），只重編 `test_flow9050_tray`／`test_mainproc_guard`，用 ctest 跑兩組態：模擬組態 [BC] 2 項（第 1102／1107 行）＋[PC] 3 項（第 1126／1131／1137 行）紅，跟 gate b110a／b120a 一樣；出貨組態全綠。
- **[BC]（cpp 0321，W-173）**：在 Yes 那項檢查前印狀態：模擬組態 `TrayY=0 L=-1 E=-1 A=0/0/0 Auto1=1 type[Auto1]=0 under=[]`，出貨組態 `Auto1=0`。每段測試開頭印 Auto1：翻成 1 的是 **[H2120]**（筆電 1006 F9050-FIX2 那段）——60 次 DoCatchTray（LoaderToEmptyColor＝2）在模擬組態走到 golden DoPlaceToBuffer 「Loader 盤放 Auto1」那一支（`acatchtray.cpp`:6403；模擬組態真空恆成功），盤一直留在 Auto1；InitialMachine 只清盤型是 tTrayAuto 的站（這裡是 0），所以 Yes 之後還是「有料」、又問一次。**1008 §8 寫的原因「InitialMachine 沒清 Tray Z／吸嘴／Shuttle／Index 上的 IC」是錯的**（`under=[]`）——那是讀碼推的，沒有量。
- **[PC]（cpp 0331，W-182）**：模擬組態根本不跑探層本體——`W906_ProbeEmpty9050` case 1 在 `#ifdef SOFT_SIMULTE` 回原點後直接設 5 層——那三項只能在出貨組態檢查。
- **修法（只動測試，`80012432`）**：[H2120] 開頭存、結尾還原 `MOT[MMAuto1].fHasTray`；[PC] 那三項放進 `#else`（出貨組態照舊跑），模擬組態改查捷徑（5 層、Z 在 Home 0）；[BC] → [FT] 的串接照機台 cpp 0324 的原字。結果：模擬組態 140／0（原 137／5）、出貨組態 142／0；`mainproc_guard`（0322 的 MainProc 檢查）兩組態都過。反向：拿掉這顆，同一個建置模擬組態 137 過／5 紅。（那顆 commit 訊息寫「01:3x-02:0x」量的，實際是 01:2x～01:3x。）
- **衝突**：0331 與 0321 在 `tests/test_flow9050_tray.cpp` 檔尾互撞，兩段都留，順序照機台 [TT]→[BC]→[FT]→[PC]；三顆的 `WORKLOG_MACHINE.md` 都取 main 那份（main 的已經是機台鏈尾的整份）。
- **第 125 批**：上面四顆＋ES02 HTDESIGNER 0.313～0.315，gate b125a：兩組態 34 分鐘：只有固定失敗（兩組態都是 4 支，492 支測試）；模擬組態的 W7_F2_SckArtState 一次 Not Run（新 exe 第一次開檔被防毒鎖住），單獨重跑通過；main `d379516c`＝第 208 包 GitHub `bcd54543`。W-173、W-182 關（機台不用再修）。
- **鏈尾**照舊 C++ `7251baf1`（cpp 0339）；0321／0322／0331 早就接過鏈（`adb3a720`／`dc3d4247`／`5fe1bc7e`）。下一次從 **cpp 0340／web 0142／tools 0182** 開始。

## 2. 02:2x：cpp 0340 PKG208、tools 0182；快照 02:28

- **cpp 0340 PKG208**：機台 02:2x 套了第 208 包（02:13 推，約 15 分鐘）——包本身，不 cherry-pick。tools 0182＝機台的 HTDESIGNER 合第 208 包，不收。
- **快照 02:28**（main `9d50397e`）：只有兩個 README 的時間戳變，機台參數與工單沒變（不叮嚀全體）。
- **鏈尾**：C++ `7251baf1`（0339）→ **`39d04020`**（0340）。下一次從 **cpp 0341／web 0142／tools 0183** 開始。
- 第 126 批（St02 MR !353）＝第 209 包，GitLab `2fb30fac`／GitHub `282accc8`。

## 3. 03:00 交件（RULINGS_20261008 第 5 條）：只有 cpp 0341 WIP；04:00 目標＝第 210 包

- 機台 `README.txt` 03:00：「03:00 交件完成：cpp 0335～0341（…0337～0340 PKG205～PKG208＝機台已套到第 208 包、0341 WIP＝EastSun 未 commit 的 acatchtray／asendic_Loader／cinitial，只保存不整合；PadInterface_St02.cpp 的 SafeLock 那行不交）；web 0141（PKG204 網頁半）。04:00 只需要套第 209 包以後。」
- **cpp 0341 WIP**（`acatchtray.cpp`／`asendic_Loader.cpp`／`cinitial.cpp`，3 檔 +29／−11）：照講好的**只保存**（就在 GitHub `machine/integ-ioweb`），不 cherry-pick、不接鏈；EastSun commit 之後會以新編號再交。
- GitHub README 03:0x 寫「04:00 目標＝第 209 包」（`73c010a2`）；第 210 包（第 127 批：St02 !354＋HTDESIGNER 0.317）03:4x 推出，GitLab `55f8238b`／GitHub `14df90a4`，03:48 推出後 README 改成「04:00 目標＝第 210 包」（`dbdce38a`）。
- **鏈尾**照舊 C++ `39d04020`（0340）。下一次從 **cpp 0342／web 0142／tools 0183** 開始（0341 是 WIP，不接鏈）。

## 4. 04:00 夜間更新完成：tools 0183、cpp 0342 PKG210（第 209～210 包）；快照 04:15

- **03:59 tools 0183**：機台的 HTDESIGNER 合第 210 包（0.315 → 0.317），不收。
- **04:15 cpp 0342 PKG210**：「integrate laptop packages 209-210（GitLab 2fb30fac, 55f8238b）-- the 04:00 night update」——照 GitHub README 的 04:00 目標（第 210 包）套完。包本身，不 cherry-pick。
- **快照 04:15**（main `08a11607`）：只有兩個 README 的時間戳變。
- **鏈尾**：C++ `39d04020`（0340）→ **`706de251`**（0342；0341 是 WIP，沒接鏈）。下一次從 **cpp 0343／web 0142／tools 0184** 開始。
- 第 128 批（St02 POOL-9 !355／!356）＝第 211 包，GitLab `cadc0934`／GitHub `571535cd`（04:27 推，機台下一輪再套）。

## 5. 07:27 tools 0184（機台的 HTDESIGNER 合第 214 包）；機台的 C++ 還停在第 210 包

- **07:27 tools 0184**：機台 AI 的定時檢查（EastSun 1008「請一直持續偵測有沒有新版」）把第 214 包的 HTDESIGNER 接上（0.317 → 0.325；第 211～213 包沒帶設計工具），沒有衝突。是合我們的包，不收。
- **C++ 還在第 210 包**：第 211 包 04:27 推，07:3x 滿 3 小時（`pkg_uptake.py` 印 REMIND）。機台 AI 07:27 有在跑，C++ 那半（第 211～214 包）還沒回來——照機台的規矩編好要請 EastSun 按 F5，可能等他上班。筆電沒有管道叫醒機台，已在 NIGHT_REPORT 告訴 Jimmy；不推第二份。
- **鏈尾**照舊 C++ `706de251`（0342）。下一次從 **cpp 0343／web 0142／tools 0185** 開始。

## 6. 07:33 cpp 0343 PKG214：機台套了第 211～214 包；快照 07:33

- **cpp 0343 PKG214**（07:33）：「integrate laptop packages 211-214（GitLab cadc0934, 7f107cef, 1607b33a, 786bc76f）... Auto-accepted」——15 個檔：7 個整檔換（逐檔掃過機台專屬的 `AI(W906-*)` 標記／`W906_` 識別字，沒有掉）、6 支新測試、2 個無衝突合併（`csystem.cpp`：MainProc 的 pitch 同步／開機清除詢問都還在；`tests/CMakeLists.txt`）；Debug wb_serve＋6 支新測試編過、20 支相關 ctest 過（原本在機台上因為缺 SECS 檔而紅的 uHGemEquipment，靠第 213 包的沙盒轉綠）、Release wb_serve 編過；備份分支 `backup/pre-pkg214-20261009`。包本身，不 cherry-pick。第 5 節的 REMIND 因此解除。
- **快照 07:33**（main `1661c466`）：只有兩個 README 的時間戳變，機台參數與工單沒變 ⇒ 不用叮嚀全體。
- **鏈尾**：C++ `706de251`（0342）→ **`90e965bf`**（0343）。下一次從 **cpp 0344／web 0142／tools 0185** 開始。

## 7. 08:50 cpp 0344 SOFTKEY-ST02P2（機台自己的）、09:04 cpp 0345 PKG215；快照 08:50／09:04

- **cpp 0344 SOFTKEY-ST02P2**（08:50，EastSun 1009「174所以你能修正嗎？」）：機台的 `WebMainScanKey.cpp` 從自己的按鍵佇列改用 St02 的 ST02-P2 面板鍵（golden `bAse*` 旗標），第 174 包的 `tests/test_scankey_golden.cpp` [19] 照原樣跑；`tests/test_main_scankey.cpp` 跟著改。收進第 134 批（b18 `3cdf7fc0`，作者照 patch）：這些行在 main 上**本來就一樣**；只有兩段衝突——機台的畫面「緊急停止」`W906_SoftEmergencyStop` 與它的測試 [13]——照 **RULINGS_20261005 第 17 條**（軟體急停只留在機台、不收進 main）取 main（不收）。⇒ 對 main 的淨效果只有 `docs/WORKLOG_MACHINE.md` 多一列。（機台的急停註記寫的是 `AI(W906-SOFTKEY)`；第 17 條請機台改標 `AI(W906-TEMP-ESTOP)`，還沒改。）
- **cpp 0345 PKG215**（09:04）：機台套了第 215 包（St02 W-159），5 個檔、相關 10 支 ctest 過；Release 沒重編（EastSun 08:57 起在跑 `build_integ_ship_x86\wb_serve.exe`，等他關了再編）。包本身，不 cherry-pick。
- **快照 08:50／09:04**（09:04 那份＝main `5a642fa9`）：`Gerneral.ini` 只有 `Program Close=1→0`（wb_serve 開著的狀態旗標）、`machinerecord.dat` 是執行紀錄；設定與工單沒變 ⇒ 不用叮嚀全體。
- **鏈尾**：C++ `90e965bf`（0343）→ `ac4df547`（0344）→ **`413485ba`**（0345）。下一次從 **cpp 0346／web 0142／tools 0185** 開始。

## 8. 10:37 cpp 0346 PKG217、10:46 cpp 0347 PKG218、10:47 cpp 0348 BOOT-Z1RESET（機台自己的）；快照 10:37／10:46／10:47

- **cpp 0346 PKG217**（10:37）／**cpp 0347 PKG218**（10:46）：機台套了第 216～218 包（12＋14 個檔，逐檔掃過機台標記沒有掉；相關 ctest 過；EastSun 開著 release wb_serve，release 等他關了再編）。包本身，不 cherry-pick。0347 註明：EastSun 沒 commit 的 `cinitial.cpp` 改動留在機台工作樹，沒進 commit。
- **cpp 0348 BOOT-Z1RESET**（10:47，EastSun 1009「M14 現在跳什麼異常」→「請持續處理」）：機台的 Index Z1（M14，1203 第 14 站，走 Galil 路線）開機時會帶著驅動器警報 A.A12（EtherCAT 輸出資料同步錯誤）起來，以前要 EastSun 在 1203 頁按 Reset 才清。現在開機過了馬達電源 1 秒的閘門之後，**如果 MTestZ1 在警報就送一次 `Acm_AxResetError`**（跟 Motor Test／1203 頁的 Alarm Reset 同一個命令）；不上伺服、不寫設定、不動作，只寫一行 BRAKE 紀錄。沒有 `AI(W906-TEMP-*)` 標記、不是暫時繞過 ⇒ 收進**第 137 批**（b19 `e2a4638c`，作者照 patch）；沒有 ctest 會編 `WebMotorAccessLive.cpp`，驗證看下次開機的 op log「MTestZ1 boot alarm reset」。
- **快照 10:47**（main `0ebbf6b2`）：**`IO_Table.csv` 的 `SnRKCoverOpen` 對應改了**（`1,2,32,1,0,1,0,0` → `1,1,2,29,5,1,3,1`）——真的參數變更 ⇒ 同一輪在 CHAT_JIMMY 叮嚀全體同步；其餘是執行紀錄（lastdata、machinerecord、Program Close、GPIB LastFile）。
- **鏈尾**：C++ `413485ba`（0345）→ `1cfa956a`（0346）→ `e94e8ef6`（0347）→ **`4cfaf8b2`**（0348）。下一次從 **cpp 0349／web 0142／tools 0185** 開始。
- （11:5x 補記）第 137 批在第 136 批進 main 之後重開：cpp 0348 現在是 b19 `543f055d` 的前一顆（`8402f9b2`），同批另有 St02 的 HANA H1～H3（!372～!374）。
- （12:3x 補記）**cpp 0348 進 main**：第 137 批 `9290fe38`＝第 220 包（GitHub `5bea18ae`），gate b137a 兩組態只有固定 4 支。

## 9. 11:49 cpp 0349 TO-JIMMY（機台自己的，只有文件）；快照 11:49

- **cpp 0349**（11:49，EastSun 1009「請派工給 jimmy 需要測試 在沒有AI 的環境下 安裝與更新 需要怎麼做」）：只有文件——`docs/TO_JIMMY_20261009_NO_AI_INSTALL_UPDATE.md`（安裝手冊、更新手冊＋腳本、沒有 Claude 的電腦實測三個情境）＋WORKLOG 216（當天 IO 表依 EastSun 接線修正）。排第 138 批（跟 St02 H5 !375 同批）。派工：手冊＋腳本 → Ifor01（W-197，TO_IFOR §4 12:3x）；實測由誰做 → NIGHT_REPORT §0 #152。
- **快照 11:49**（main `de141e58`）：`IO_Table.csv` 依 EastSun 現場接線改了（風刀、Die 清潔、熱風槍換位或新增，撞位址的 `SwIndEpArm1` 停用，`SnNegativePressureAir`／`Air2` 停用）——真的參數變更 ⇒ 同一輪在 CHAT_JIMMY 叮嚀全體；其餘（`Program Close`、lastdata、machinerecord、GPIB LastFile）是執行紀錄。
- **鏈尾**：C++ `4cfaf8b2`（0348）→ **`ff19f3b2`**（0349）。下一次從 **cpp 0350／web 0142／tools 0186** 開始。

## 10. 12:2x cpp 0350 PKG219（機台合筆電第 219 包，只接鏈）；快照 12:28

- **cpp 0350 PKG219**：機台把筆電第 219 包（GitLab `51f954bf`）合進去——機台合筆電的包，照規則不 cherry-pick，只接鏈。`pkg_uptake.py`（13:1x）：機台已經套到第 220 包。
- **快照 12:28**（main `c434dff3`）：只有執行紀錄（`Program Close`、lastdata、machinerecord、GPIB LastFile），沒有參數或工單變更 ⇒ 不用叮嚀全體。
- **鏈尾**：C++ `ff19f3b2`（0349）→ **`243458f7`**（0350）。下一次從 **cpp 0351／web 0142／tools 0186** 開始。
- （13:2x 補記）**cpp 0349 進 main**：第 138 批 `be3e7d75`＝第 221 包（GitHub `741e482f`）；WORKLOG_MACHINE.md 的衝突（§3 表：機台有、main 沒有的包／設計外掛列）照第 11 條取機台那一邊。

## 11. 12:5x cpp 0351 PKG220（機台合筆電第 220 包，只接鏈）；快照 12:57

- **cpp 0351 PKG220**：機台把筆電第 220 包（GitLab `9290fe38`）合進去——照規則不 cherry-pick，只接鏈。`pkg_uptake.py`（13:4x）：機台已經套到第 221 包。
- **快照 12:57**（main `e873ba6b`）：只有兩個說明檔的拍照時間變了，沒有參數或工單變更 ⇒ 不用叮嚀全體。
- **鏈尾**：C++ `243458f7`（0350）→ **`f0e1b55a`**（0351）。下一次從 **cpp 0352／web 0142／tools 0186** 開始。

## 12. 13:2x cpp 0352 PKG221（機台合筆電第 221 包，只接鏈）；快照 13:27

- **cpp 0352 PKG221**：機台把筆電第 221 包（GitLab `be3e7d75`）合進去——只接鏈。`pkg_uptake.py`（13:5x）：機台套到第 221 包（當時最新）。
- **快照 13:27**：`snap_push.py` 回 up to date（跟 main 那份一樣）。
- **鏈尾**：C++ `f0e1b55a`（0351）→ **`913cfdec`**（0352）。下一次從 **cpp 0353／web 0142／tools 0186** 開始。第 222 包（第 139 批 `73069916`，GitHub `69c776fa`）14:0x 推出。

## 13. 15:2x～15:4x cpp 0353 PKG223（只接鏈）、cpp 0354 WORKLOG（照收）、tools 0187（設計外掛，不收）；快照 15:40

- **cpp 0353 PKG223**：機台把筆電第 222～223 包合進去——只接鏈。`pkg_uptake.py`（16:0x）：機台已經套到第 224 包（當時最新）。
- **cpp 0354 WORKLOG**：機台 WORKLOG 第 3 節補設計外掛 tools 0186／0187 的列——照收，排第 143 批（第 142 批 gate 中）。
- **tools 0186／0187**：機台的設計外掛工作階段合筆電包（0.330 → 0.341、→ 第 222～223 包的版本）——機台自己的設計外掛線，筆電不收。
- **快照 15:40**（main `f1dcb125`）：只有兩個說明檔的拍照時間變了，沒有參數或工單變更 ⇒ 不用叮嚀全體。
- **鏈尾**：C++ `913cfdec`（0352）→ `ca2c67fe`（0353）→ **`7a402f24`**（0354）。下一次從 **cpp 0355／web 0142／tools 0188** 開始。

## 14. 16:0x cpp 0355 PKG224（只接鏈）、cpp 0356 WORKLOG（照收）、tools 0188（設計外掛，不收）；快照 16:02

- **cpp 0355 PKG224**：機台把筆電第 224 包合進去——只接鏈；機台已套到第 224 包。
- **cpp 0354＋0356 WORKLOG**：照收，進第 143 批（b19 `784b76c9`，cherry-pick 都沒有衝突）。
- **tools 0188**：機台的設計外掛線合第 224 包（0.345 → 0.347），筆電不收。
- **快照 16:02**（main `e72694be`）：只有兩個說明檔的拍照時間變了，沒有參數或工單變更。
- **鏈尾**：C++ `7a402f24`（0354）→ `110eb661`（0355）→ **`66b67a82`**（0356）。下一次從 **cpp 0357／web 0142／tools 0189** 開始。
- （17:1x 補記）**cpp 0354／0356 進 main**：第 143 批 `1dd66de4`＝第 226 包（GitHub `762b3fb5`）。

## 15. 17:0x cpp 0357 PKG225（只接鏈）、web 0142 IOFIT（機台自己的網頁改動，收）；快照 17:01／17:02

- **cpp 0357 PKG225**：機台把筆電第 225 包合進去——只接鏈；`pkg_uptake.py`：機台套到第 225 包。
- **web 0142 IOFIT**（17:00，EastSun 1009「這個畫面右邊的空位太大可否變成第二張圖這樣」）：IO 檢查／驗證的全螢幕層只跟頁面一樣寬（`MODAL_POLICY` io 加 fitWidth），只改 `web/background.html` 19 行——排第 145 批，照 web 0140 的慣例 `git am --directory=web`。
- **快照 17:01／17:02**（main `72a594fd`）：`Gerneral.ini` [IndexDriver] 多了 **`HT9050_INDEXZ_TORQUE_CONFIRMED=1`**（EastSun E-10 量測後確認 1203 扭力來源）——真的參數變更 ⇒ 同一輪 CHAT_JIMMY 叮嚀全體；其餘是執行紀錄。
- **鏈尾**：C++ `66b67a82`（0356）→ **`74457544`**（0357）。下一次從 **cpp 0358／web 0143／tools 0189** 開始。
- （18:0x 補記）**web 0142 IOFIT** 收進第 145 批（b19 `951960ae`，`git am --directory=web`，作者／日期照 patch）；gate b145a 跑中。

## 16. 18:0x cpp 0358 PKG226（只接鏈）；快照 17:36

- **cpp 0358 PKG226**：機台把筆電第 226 包合進去——只接鏈；`pkg_uptake.py`：機台套到第 226 包（第 227 包 18:04 推出）。
- **快照 17:36**（main `714a303b`）：只有 `Gerneral.ini` 的 `Program Close=1→0`（程式自己寫的執行狀態，開著＝0）、`machinerecord.dat`（運轉紀錄）與兩份 README 的時間——不是參數或工單變更，這輪不叮嚀全體。
- **鏈尾**：C++ `74457544`（0357）→ **`990775c4`**（0358）。下一次從 **cpp 0359／web 0143／tools 0189** 開始。
- （18:3x 補記）**web 0142 進 main**：第 145 批 `af18d6ab`＝第 228 包（GitHub `e3e54dae`）。

## 17. 18:2x cpp 0359 PKG227（只接鏈）、cpp 0360 WORKLOG（排第 146 批）；快照 18:26

- **cpp 0359 PKG227**：機台把筆電第 227 包合進去——只接鏈；`pkg_uptake.py`：機台套到第 227 包（第 228 包 18:33 推出）。
- **cpp 0360 WORKLOG**：機台 WORKLOG §3 補設計工具 tools 0189 一列（只有文件）——照慣例 cherry-pick，排**第 146 批**（跟下一張程式 MR 一起，不單獨出包）。tools 0189 HTDESIGNER 照舊不收。
- **快照 18:26**（main `21e9d7ee`）：只有兩份 README 的時間——不是參數或工單變更，不叮嚀。
- **鏈尾**：C++ `990775c4`（0358）→ **`93dfc1bb`**（0360）。下一次從 **cpp 0361／web 0143／tools 0190** 開始。
- （19:3x 補記）**cpp 0360 WORKLOG** 收進第 146 批（b18 `b36d1d01`，`cherry-pick -x 93dfc1bb`，乾淨）；gate b146a 跑中。

## 18. 19:5x 快照：IO_Table.csv 兩個空盤感測器對調（真的參數變更）

- **快照 19:53**（main `a0acdb37`，781 檔）：`IO_Table.csv` `SenEmptySelectHasTray` 84 第 9 位 → 第 8 位（極性 1 → 0）、`SnEmptyDrawerHasTray` 84 第 8 位 → 第 9 位；另有 EastSun 改表時留的三份 `.bak`（19:46／19:47／19:52）、`lastdata*.dat`、GPIB 的 LastFile。
- 真的參數變更 ⇒ 同一輪 CHAT_JIMMY 叮嚀全體；TO_ES02 §4 請 EastSun 回一行原因，同時給 St01。
- 機台 patch 沒有新的（cpp 0360／web 0142／tools 0189）；機台套到第 227 包，第 228 包 18:33 推出。
- （20:3x 補記）**快照 20:25**（main `d211caca`，782 檔）：IO_Table.csv 再改一處——`SnEmptyDrawerHasTray` 84,9,**0 → 1**（EastSun 20:09 留了 `.bak`）。跟 19:53 合起來，兩個空盤感測器的整組設定對調：`SenEmptySelectHasTray` 84,9,1 → 84,8,0；`SnEmptyDrawerHasTray` 84,8,0 → 84,9,1。再叮嚀一次全體。
- （21:1x 補記）**快照 20:52**（main `1dc8b097`，783 檔）：IO_Table.csv 第三次修改（EastSun 20:43 留 `.bak`）。欄位是 `IP, Port, Bit, InType`——前兩則把 84（IP）寫成「埠」、把 Port 寫成「位」，更正。跟 17:02 的快照比，淨變化只有：`SenEmptySelectHasTray` IP 84／Port 9／Bit 1 的 **InType 0 → 1**；`SnEmptyDrawerHasTray` IP 84／Port 8 的 **Bit 0 → 1**。19:53／20:25 的 Port 對調已改回。
- （21:3x 補記）**cpp 0360 WORKLOG 進 main**：第 146 批 `40b3b2b4`＝第 229 包（GitHub `5068e928`）。
- （21:3x 補記）**快照 21:22**（main `576fa7b9`，790 檔）：IO_Table.csv 第四次修改（EastSun 20:57～21:17 留了 7 份 `.bak`），跟 17:02 比：`SnLoaderTrayHasTray` 80,8,0,0 → **80,9,1,0**；`SnLoaderDrawerHasTray` 80,9,1,1 → **80,8,0,0**；`SnEmptyDrawerHasTray` 84,8,0,1 → **84,8,0,0**；`SenEmptySelectHasTray` 回到 84,9,1,0（跟 17:02 一樣）。叮嚀全體（前三則作廢）。`pkg_uptake` REMIND：機台套到第 227 包，第 228 包推出 3 小時、第 229 包也推了——已在 TO_ES02 §4 請 EastSun 有空叫機台端整合。

## 19. 22:5x cpp 0361～0363 PKG228／PKG229／PKG230（只接鏈）、web 0143 PKG230 網頁那一半（不收）；快照 22:43

- **cpp 0361／0362／0363**：機台把筆電第 228、229、230 包依序合進去——三顆都只是整合，沒有機台自己的改動，只接鏈；`pkg_uptake.py`：機台套到**第 230 包**（第 231 包 22:49 推出），18:33 起「第 228 包超過 3 小時沒套」的提醒解除。
- **web 0143 PKG230**：第 230 包網頁那一半（KYEC FTP 上傳按鈕）在機台的 web repo 套上——照慣例不收。
- **機台推送時的掃描**：機台自己的 push_stage 密碼掃描命中 2 處，EastSun 看過後手動推：0361:790 `tests/test_st02_w202_ftpseams.cpp` 的 `127.0.0.1`／`st02-test`（第 228 包的假測試帳號）、0363:646 測試還原變數（沒有字面值）——都不是真密碼。
- **快照 22:43**（main `2b994a91`，790 檔）：只有 GPIB 的 `LastFile`、`lastdata*.dat`（執行紀錄）與兩份 README 的時間——不是參數或工單變更，不叮嚀。
- **鏈尾**：C++ `93dfc1bb`（0360）→ **`e2e49a5e`**（0363）。下一次從 **cpp 0364／web 0144／tools 0191** 開始。

## 20. 23:2x cpp 0364 PKG231（只接鏈）、cpp 0365 WORKLOG（排第 150 批）；快照 22:58

- **cpp 0364 PKG231**：機台把筆電第 231 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 231 包**（第 232 包 23:2x 推出）。
- **cpp 0365 WORKLOG**（機台 WORKLOG §3 補設計工具 tools 0191 一列，只有文件）：cherry-pick 進**第 150 批**（b19 `855111f8`）。衝突：機台在 PKG commit 裡加的列（第 219～231 包各一列＋tools 0190）我們沒收，0365 的上下文對不上——`WORKLOG_MACHINE.md` 只有機台在寫，照 RULINGS_20260930 第 11 條**採用機台整份**（只多 14 列、沒有刪任何一列；cherry-pick 後跟機台 `327c6240` 那份逐位元組相同）。
- **快照 22:58**（main `cb78a48e`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `e2e49a5e`（0363）→ **`327c6240`**（0365）。下一次從 **cpp 0366／web 0144／tools 0192** 開始。

## 21. （20261010 00:0x）cpp 0366 PKG232（只接鏈）、cpp 0367 WORKLOG（等下一張程式 MR）；快照 23:42

- **cpp 0366 PKG232**：機台把筆電第 232 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 232 包**。
- **cpp 0367 WORKLOG**（機台 WORKLOG §3 補設計工具 tools 0192 一列，只有文件）：第 150 批 gate 已在跑，不插隊——照慣例等下一張程式 MR 一起 cherry-pick（不單獨出包）。
- **cpp 0365 WORKLOG 進 main**：第 150 批 `934e5da8`＝第 233 包（GitHub `ba13553f`）。
- **快照 23:42**（main `c5f19588`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `327c6240`（0365）→ **`276ae779`**（0367）。下一次從 **cpp 0368／web 0144／tools 0193** 開始。

## 22. （20261010 00:4x）cpp 0368 PKG233（只接鏈）；快照 00:24

- **cpp 0368 PKG233**：機台把筆電第 233 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 233 包**（＝GitHub main 最新）。
- **cpp 0367 WORKLOG** 收進第 151 批（b18 `5c2cee5e`，`cherry-pick -x 276ae779` 乾淨；另一顆把 `WORKLOG_MACHINE.md` 換成機台整份——補上機台在 PKG232 裡加的第 232 包那列，RULINGS_20260930 第 11 條）；gate b151a 跑中。
- **快照 00:24**（main `bd7d4c81`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `276ae779`（0367）→ **`61ecf384`**（0368）。下一次從 **cpp 0369／web 0144／tools 0193** 開始。
- （20261010 00:5x 補記）**cpp 0367 WORKLOG 進 main**：第 151 批 `c8057031`＝第 234 包（GitHub `84c06f33`）。

## 23. （20261010 01:3x）cpp 0369 PKG234（只接鏈）、cpp 0370 WORKLOG（等下一張程式 MR）；快照 01:02／01:25

- **cpp 0369 PKG234**：機台把筆電第 234 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 234 包**（第 235 包 01:33 推出）。
- **cpp 0370 WORKLOG**（機台 WORKLOG §3 補設計工具 tools 0193 一列，只有文件）：等下一張程式 MR 一起 cherry-pick（不單獨出包）。
- **快照 01:02**（main `df1acd6f`）：`Gerneral.ini` 的 `Program Close=0→1`（程式自己寫的執行狀態＝EastSun 把程式關了）、GPIB `LastFile`、`lastdata*.dat`、兩份 README——不是參數或工單變更，不叮嚀。**快照 01:25**（main `948d83f4`）：只有兩份 README 的時間。
- **鏈尾**：C++ `61ecf384`（0368）→ `5c952466`（0369）→ **`2f897064`**（0370）。下一次從 **cpp 0371／web 0144／tools 0194** 開始。

## 24. （20261010 02:1x）cpp 0371 PKG235（只接鏈）；cpp 0370 WORKLOG 收進第 153 批；快照 01:56

- **cpp 0371 PKG235**：機台把筆電第 235 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 235 包**（＝GitHub main 最新）。
- **cpp 0370 WORKLOG** 收進**第 153 批**（b18 `6654587a`，`cherry-pick -x 2f897064` 乾淨；另一顆把 `WORKLOG_MACHINE.md` 換成機台 0371（`8684708b`）那份整份——補上機台在 PKG233／234／235 裡寫的三列，RULINGS_20260930 第 11 條）；gate b153a 跑中。
- **快照 01:56**（main `f321e3e6`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `2f897064`（0370）→ **`8684708b`**（0371）。下一次從 **cpp 0372／web 0144／tools 0194** 開始。

## 25. （20261010 03:3x）cpp 0372 PKG236（只接鏈）、cpp 0373 WORKLOG（等下一張程式 MR）；快照 03:25

- **cpp 0372 PKG236**：機台把筆電第 236 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 236 包**（＝GitHub main 最新）。
- **cpp 0373 WORKLOG**（設計工具 tools 0194 一列，只有文件）：第 154 批 gate 在跑，不插隊——等下一張程式 MR 一起 cherry-pick，WORKLOG 照機台整份收。
- **快照 03:25**（main `15af13d0`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `8684708b`（0371）→ **`a70208a2`**（0373）。下一次從 **cpp 0374／web 0144／tools 0195** 開始。

## 26. （20261010 04:3x）cpp 0374 PKG237（只接鏈）；快照 03:54；第 155 批收 cpp 0373 WORKLOG

- **cpp 0374 PKG237**：機台把筆電第 237 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 237 包**（＝GitHub main 最新）。
- **快照 03:54**（main `1d22b6d1`）：只有兩份 README 的時間——不叮嚀。
- **cpp 0373 WORKLOG**：cherry-pick 進第 155 批（b18）；之後 `WORKLOG_MACHINE.md` 照機台 cpp 0374（`fb698a35`）整份收（補 0372／0374 PKG commit 寫的第 236／237 包兩列），跟機台那份逐位元組相同（diff 0 行）。
- **鏈尾**：C++ `a70208a2`（0373）→ **`fb698a35`**（0374）。下一次從 **cpp 0375／web 0144／tools 0195** 開始。

## 27. （20261010 05:4x）cpp 0375 PKG238（只接鏈）；快照 05:24

- **cpp 0375 PKG238**：機台把筆電第 238 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 238 包**（第 239 包剛推）。PKG commit 裡機台寫的 WORKLOG 列等下一張程式批次整份收。
- **快照 05:24**（main `c99117e0`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `fb698a35`（0374）→ **`0672acca`**（0375）。下一次從 **cpp 0376／web 0144／tools 0195** 開始。

## 28. （20261010 05:5x）cpp 0376 WORKLOG（第 157 批收）；tools 0195 不收；快照 05:30

- **cpp 0376 WORKLOG**（設計工具 tools 0195 一列，只有文件）：cherry-pick 進第 157 批（b18 `750eb1fc`）；之後 `WORKLOG_MACHINE.md` 照機台 0376（`2b6e5e59`）整份收（補 0375 PKG238 寫的第 238 包那列），跟機台那份逐位元組相同。
- **tools 0195 HTDESIGNER**：設計工具，照慣例不收（ES02 的 `v906/es02-htdesigner` 帶著，0.389～0.399 已合進 main）。
- **快照 05:30**（main `d95fc5fc`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `0672acca`（0375）→ **`2b6e5e59`**（0376）。下一次從 **cpp 0377／web 0144／tools 0196** 開始。

## 29. （20261010 06:2x）cpp 0377 PKG239（只接鏈）；快照 05:55

- **cpp 0377 PKG239**：機台把筆電第 239 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 239 包**（＝GitHub main 最新）。
- **快照 05:55**（main `abb7e7e2`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `2b6e5e59`（0376）→ **`15213870`**（0377）。下一次從 **cpp 0378／web 0144／tools 0196** 開始。PKG239 寫的 WORKLOG 列等下一張程式批次整份收。

## 30. （20261010 06:4x）cpp 0378 WORKLOG（等下一張程式批次）；tools 0196 不收；快照 06:20

- **cpp 0378 WORKLOG**（設計工具 tools 0196 一列，只有文件）：第 157 批 gate 已在跑，不插隊——等下一張程式批次 cherry-pick，WORKLOG 照機台整份收（含 0377 PKG239 寫的那列）。
- **tools 0196 HTDESIGNER**：照慣例不收。
- **快照 06:20**（main `1368d8d7`）：只有兩份 README 的時間——不叮嚀。機台在**第 239 包**；第 240 包剛推。
- **鏈尾**：C++ `15213870`（0377）→ **`b7b9e9b9`**（0378）。下一次從 **cpp 0379／web 0144／tools 0197** 開始。

## 31. （20261010 07:1x）cpp 0379 PKG240（只接鏈）；快照 06:54

- **cpp 0379 PKG240**：機台把筆電第 240 包合進去——只接鏈；`pkg_uptake.py`：機台套到**第 240 包**（＝GitHub main 最新）。
- **快照 06:54**（main `3d90db22`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `b7b9e9b9`（0378）→ **`2f517b9d`**（0379）。下一次從 **cpp 0380／web 0144／tools 0197** 開始。0378 WORKLOG 與 PKG239／240 寫的 WORKLOG 列等下一張程式批次整份收。

## 32. （20261010 07:5x）第 158 批收 cpp 0378 WORKLOG

- **cpp 0378 WORKLOG**：cherry-pick 進第 158 批（b18 `ac2aa6f4`，同批 Ifor01 的 !413）；之後 `WORKLOG_MACHINE.md` 照機台 cpp 0379（`2f517b9d`）整份收（補 0377／0379 PKG commit 寫的第 239／240 包兩列），跟機台那份逐位元組相同（diff 0 行）。
- **鏈尾**仍是 C++ `2f517b9d`（0379）。下一次從 **cpp 0380／web 0144／tools 0197** 開始。

## 33. （20261010 09:1x）cpp 0380 PKG241（只接鏈）；tools 0197 不收；快照 08:54

- **cpp 0380 PKG241**：機台把筆電第 241 包（Ifor01 W-208 ①，氣缸壽命讀檔）合進去——只接鏈；`pkg_uptake.py`：機台套到**第 241 包**（＝GitHub main 最新）。
- **tools 0197 HTDESIGNER**：照慣例不收。
- **快照 08:54**（main `35cbda50`）：只有兩份 README 的時間——不叮嚀；`MachineLife.ini` 在快照裡沒有變動。
- **鏈尾**：C++ `2f517b9d`（0379）→ **`ffefd4ab`**（0380）。下一次從 **cpp 0381／web 0144／tools 0198** 開始。PKG241 寫的 WORKLOG 列等下一張程式批次整份收。

## 34. （20261010 10:3x）cpp 0381 PKG242（只接鏈）；快照 10:28

- **cpp 0381 PKG242**：機台把筆電第 242 包（Ifor01 W-208 ②，氣缸壽命存檔＋清零）合進去——只接鏈；`pkg_uptake.py`：機台套到**第 242 包**（＝GitHub main 最新）。
- **快照 10:28**（main `aca2a991`）：只有兩份 README 的時間——不叮嚀。
- **鏈尾**：C++ `ffefd4ab`（0380）→ **`5f19513b`**（0381）。下一次從 **cpp 0382／web 0144／tools 0198** 開始。PKG242 寫的 WORKLOG 列等下一張程式批次整份收。

## 35. （20261010 11:5x）cpp 0382 PKG243（只接鏈）、cpp 0383 BOOTREOPEN0AX（收進第 162 批）；快照 11:36

- **cpp 0382 PKG243**：機台 11:27 把筆電第 243 包（NB2-1 W-188 #7）合進去——只接鏈。第 243 包 11:05 推出，22 分鐘就套了。
- **cpp 0383 BOOTREOPEN0AX**（機台自己的修正，EastSun 1010「現在還是一樣，幫我弄好他」）：11:11:55 開機時 `Acm_DevOpen` 成功，但 EtherCAT 從站還沒準備好，每一軸都是「沒開」，而且沒有任何東西判定失敗（不算開卡失敗；連線沒起來，模組檢查也沒跑），只能靠人按重新開卡。修法是 `tools/wb_serve.cpp`:4463 同一行：卡開了但 `axesOpened==0` 就呼叫既有的 `W906_Pci1203ReopenOpenFailed`（:9410，跟 :4492「沒開成」那條同一個入口），交給停機時自動重開的流程（WAR16150）。筆電核對：`axesOpened` 在 `Pci1203CardSample` 裡（`EtherCAT/Pci1203Monitor.cpp` 會累加），函式簽章一致。**收進第 162 批**（b18，cherry-pick `529272e8`）。
- **WORKLOG**：第 162 批整份對齊到鏈尾 `dc6ae221`（PKG242／PKG243／BOOTREOPEN0AX 三列）。
- **快照 11:36**（main `856ad9d7`）：`D_HT9045_system/MachineLife.ini` 多 42 鍵（21 支氣缸的 `*_OnOffCountAlarm`／`*_TimeOutCountAlarm`，值都是 1）——就是 W-215 講的 golden 913 行為（缺鍵讀成 1 並寫回），套第 241 包之後第一次開 wb_serve 寫的，不是 EastSun 改設定；另有 GPIB `general.ini` 的 LastFile、`lastdata*.dat`（執行期資料）。照第 6 條叮嚀全體（CHAT_JIMMY 同時間）。
- **鏈尾**：C++ `5f19513b`（0381）→ **`dc6ae221`**（0383）。下一次從 **cpp 0384／web 0144／tools 0198** 開始。

## 36. （20261010 12:3x）cpp 0384 PKG244（只接鏈）；快照 12:26

- **cpp 0384 PKG244**：機台 12:26 把筆電第 244 包（Ifor-GPT IG-8 ②）合進去——只接鏈；`pkg_uptake.py`：機台套到**第 244 包**（＝GitHub main 最新）。
- **快照 12:26**（main `206a5111`）：`D_HT9045_system/Gerneral.ini` 的 `Program Close` 1→0（程式自己寫的執行期旗標）、`lastdata*.dat`／`machinerecord.dat`、GPIB `general.ini` LastFile——沒有參數或工單的變更，不叮嚀。
- **WORKLOG**：第 163 批整份對齊到鏈尾 `423bfa36`（PKG244 那列）。
- **鏈尾**：C++ `dc6ae221`（0383）→ **`423bfa36`**（0384）。下一次從 **cpp 0385／web 0144／tools 0198** 開始。

## 37. （20261010 13:1x）cpp 0385 PKG245（只接鏈）；快照 13:04

- **cpp 0385 PKG245**：機台把筆電第 245 包（St02 W-214＋它自己的 0383 BOOTREOPEN0AX）合進去——只接鏈。
- **快照 13:04**（main `78d5bc03`）：只有兩份 README 的時間——不叮嚀。
- **WORKLOG**：第 164 批整份對齊到鏈尾 `e8509123`（PKG245 那列）。
- **鏈尾**：C++ `423bfa36`（0384）→ **`e8509123`**（0385）。下一次從 **cpp 0386／web 0144／tools 0198** 開始。

## 38. （20261010 13:3x）cpp 0386 WORKLOG（只接鏈）、tools 0198 HTDESIGNER（不收）；快照 13:28

- **cpp 0386**：只有 `WORKLOG_MACHINE.md` 一列（設計外掛 tools 0198，0.405.0→0.419.0）——只接鏈；第 164 批已在 gate，這份 WORKLOG 等**下一個程式批次**整份收。
- **tools 0198 HTDESIGNER**：機台合筆電第 244／245 包的設計外掛——照規矩不收（HTDESIGNER 走 ES02 的 `v906/es02-htdesigner`）。
- **快照 13:28**（main `07e95150`）：只有 `lastdata.dat`／`lastdata_backup.dat`（執行期資料）與兩份 README——不叮嚀。
- **鏈尾**：C++ `e8509123`（0385）→ **`6609b2a1`**（0386）。下一次從 **cpp 0387／web 0144／tools 0199** 開始。

## 39. （20261010 14:3x）cpp 0387 PKG246（只接鏈）；快照 14:08

- **cpp 0387 PKG246**：機台把筆電第 246 包（St02 W-213 ②④＋W-216、Ifor01 L07 資料模型）合進去——只接鏈；`pkg_uptake.py`：機台在**第 246 包**，第 247 包推出 0.5 小時。
- **快照 14:08**（main `a4c39521`）：只有兩份 README 的時間——不叮嚀。
- **WORKLOG**：0386＋0387 兩列等下一個程式批次整份收（鏈尾 `7b1ec790`）。
- **鏈尾**：C++ `6609b2a1`（0386）→ **`7b1ec790`**（0387）。下一次從 **cpp 0388／web 0144／tools 0199** 開始。
