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
