# 還沒有網頁的表單 —— 建頁備忘

> Steven 20260926 13:4x：「還沒有網頁的，可以在 reference 裡面先註記該網頁的一些事項，方便後續建立
> 網頁的時候可以使用」。
>
> 這份檔案收「讀寫檔（C 路）已經做了、但還沒有對應網頁頁面」的表單，每個表單一節，欄位固定，
> 方便之後要建頁的人直接查。**資料來源**：各 `FileRW/*.cpp` 與 `tools/editlist/*.py` 的檔頭
> 註解、下面列的 commit 本文（`git show <hash>`）。**找不到的欄位寫「待確認」，不是用猜的。**
>
> 固定欄位：建議頁面檔名｜golden 檔｜移植入口｜PageDesc 有沒有登記｜讀檔時機｜存檔函式與 golden
> 觸發鈕｜必送欄位｜要建的元件 id｜頁面事件要 JS 做的｜已知陷阱｜待 Steven 決定｜要等 Jimmy 的
> 前提｜commit。
>
> 「建議頁面檔名」是**建議**，沒有人裁決過，照現有頁面的命名慣例（`Setup.*`／`HW.*`／`Status.*`／
> `Data.*`）猜一個方便之後討論，不是定案。
>
> **20260927（St01，HEAD 227b79db）**：加十六～十九（Configuration 頁 Tray／HP 分頁 S98、`TACTForm` S108、`TfWinway` S109、
> `TfMonitor` S110）。十六不是 C 路結構（手寫 `FileRW/CfgTrayPlate.cpp`），因為一樣「讀寫做了、沒有頁」所以放在這裡。
> 要不要現在建這些頁待 Steven（skill `ht9050-construction` `references/todo.md` ★ Q41）。
>
> **20261001（St01，todo E-003 ①）**：十六（Configuration 頁 Tray／HP 分頁，S98）**已接上網頁**——S169「沒有移植的, 我們直接實作」蓋過 Q41 S158「新頁面先不做」；
> 見該節開頭的 ⛔ 更新（原文保留）。

---

## 1. Rotate（旋轉機構）

* **建議頁面檔名**：`Setup.Rotate.html`（建議，未定案）。
* **golden 檔**：`RotateKit\fRotate.cpp`（V912，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\RotateKit\fRotate.cpp`），表單 `TFrmRotate`。
* **移植入口**：`FileRW/Rotate.cpp`／`FileRW/Rotate.gen.inc`；`tools/editlist/Rotate.py`；前綴 `RT`；結構名 `Rotate`（golden 全域 `tRotate`，型別 `TRotate`，定義在 `forms/fRotate.h`）；commit `c675594d`。
* **PageDesc 有沒有登記**：**沒有**。沒有頁面、沒有 WS 指令、沒有登記 `PageDesc`。
* **讀檔時機**：開機與換配方都走（`W906_DoReadLastData`）——`FileRW_Rotate_ReadFile()`（golden `DoReadLastData main.cpp:9361`）→ `FileRW_Rotate_DoIniDataToForm()`（golden `:9407`）。開頁（`FormShow :131-132`）沒有頁面所以沒接。
* **存檔函式與 golden 觸發鈕**：`spbSaveClick`（golden `:1036`）→ `FileRW_Rotate_spbSaveClick()`。流程：A02 權限 →「與入料角度相同不存檔」保護 → Active Rotate 流程保護 → ART RT 檢查 → `WriteIniData` 寫 `Rotate.Data` → 重讀 → `bHasSaveSet` → `fMain->BackupSetupFile()`。
* **必送欄位**：見 `Rotate.gen.inc` 的 `kRT_SaveReads`（未逐欄列出，待確認詳細清單；含 `tRotate` 的欄位與 `Rotate_Offset[2]`／`Pre_Rotate[2]`）。
* **要建的元件 id**：`ColCount`／`RowCount`（golden `sgRotateIn` 的格子大小，開頁時 `SetGridDraw` 才會設，見下面陷阱）、`rgDutNum`、`sgRotateIn`／`sgFTA` 兩個格子、`cbOutRotateDifferentAngle`、`chkRotateUseRTmode` 等（golden `fRotate.h` 元件，逐一清單待確認——本檔只翻了讀寫段，畫面元件沒有整理成表）。
* **頁面事件要 JS 做的**：`sgRotateInSelectCell`（點格子改角度 → `iRotateDutDate`）、`rgDutNumClick`、`sgFTADrawCell`／`SelectCell`、`rgSelectClick`、`cbOutRotateDifferentAngleClick`、`chkRotateUseRTmodeClick`、小鍵盤（`XPitchKeyPress`／`YPitchMouseDown`／`edIn*MouseDown`）——這些都還沒轉，見 `tools/editlist/Rotate.py` 的「不轉（頁面／顯示）」清單。
* **已知陷阱**：
  1. `spbSaveClick` 有兩段 GATE：ART RT 格子檢查用 `sgRotateIn` 的 `RowCount` 當 `iRotateDutDate` 索引（格子大小不對會越界）；`ColCount`／`RowCount` 兩鍵要等頁面開頁時 `SetGridDraw` 設定過——建頁時要把 golden `FormShow`、`SetGridDraw` 加進 `.py` 的 `METHODS` 才能打開這兩段。
  2. `FrmRotDegree->SetDegree`（64 個動態 `TfrmRot` 表單，`aRotateDegreeClass.cpp`）是純畫面，移植樹沒有，不影響讀寫檔。
  3. `DoIniDataToForm` 結尾 golden 自己呼叫 `SetWorkParameter()`——開機／換配方時會多跑一次（`ChangeSite`／`ReadTechData`／`DoStructUnitConvert`…，沒有馬達動作），是 golden 原本的行為，照翻。
* **待 Steven 決定**：無（這節目前沒有交件報告列出的決策題；如果之後有，補在這裡）。
* **要等 Jimmy 的前提**：`SetIn/OutRotateSpeed`（golden `:1297`／`:1309`，馬達速度）；`InitialIn/OutRotateHome`、`DoIn/OutRotateHome`（機台動作，已在移植樹，不是這裡要做的）。
* **commit**：`c675594d`。

---

## 2. AutoAlignment（CCD 對位）

* **建議頁面檔名**：`Setup.AutoAlignment.html`（建議，未定案）。
* **golden 檔**：**編譯進去的是 `AutoAlignment\AutoAlignment.cpp`**（V912，22 鍵）。⚠ 0925 稽核指的
  `cAutoAlignment.cpp` 是死碼，不在 `HT9045.bpr` 裡，不要照那份查。表單 `TfAutoAlignment`。
* **移植入口**：`FileRW/TestIF_File_AutoAlignment.cpp`／`.gen.inc`；`tools/editlist/TestIF_File_AutoAlignment.py`；前綴 `AA`；結構 `TestIF_File_AutoAlignment`；commit `c675594d`。
* **PageDesc 有沒有登記**：**沒有**。沒有頁面、沒有 WS 指令。
* **讀檔時機**：開機與換配方（`DoReadLastData :9432` `fAutoAlignment->ReadFile()`，已接——這支是移植樹
  `AutoAlignment/AutoAlignment.cpp` 自己的 `ReadFile`，不在本檔）→ `:9434` `FileRW_AutoAlignment_DoIniDataToForm()`。
  建構子（`:47`）在開機讀 `Gerneral.ini [Auto_Alignment]` 的 CCD 位址／埠（缺鍵補 golden 預設值）。
* **存檔函式與 golden 觸發鈕**：`spbSaveClick`（golden `:1573`，22 鍵）→ `FileRW_AutoAlignment_spbSaveClick()`。
  沒有 CCD（`MACHINE_HAS_AUTO_ALIGNMENT_CCD==0`，本機 `Gerneral.ini:158` 就是這樣）時 golden 在
  `CheckAutoAlignmentEvent` 之後就 `return`，不寫檔。
* **必送欄位**：`TestIF_File_AutoAlignment.gen.inc` 的 `kAA_SaveReads`（22 鍵；逐欄清單待確認）。
* **要建的元件 id**：四個 CCD 位址／埠輸入框（建構子填的那組）＋ `spbSaveClick` 讀的 22 個元件（逐一清單待確認）；`cbAutoAlignmentTray_AfterHome` 勾選框（見下面頁面事件）。
* **頁面事件要 JS 做的**：`cbAutoAlignmentTray_AfterHomeClick`（勾選連動）是頁面事件，還沒轉。
* **已知陷阱**：
  1. golden 的區段名字拼錯：`[AutoAlignmrnt]`（少一個 e），不是 `[AutoAlignment]`——照抄，不要「修正」。
  2. `DoIniDataToForm` 不只是顯示：golden 在這裡把 `TestIF_File.iAutoAlignmentTrayEvent |=
     AutoAlignmentTray_AfterHome`、`iAutoAlignmentShuttleHotplateEvent |= AutoAlignmentCK_AfterHome`——
     每次開機／換配方都會強制帶「回 Home 後」這個位元，這是 golden 的既有行為，不是移植樹加的。
  3. Tray 與 CK 四個 Z offset 共用同一組鍵名，**後寫的 CK 會蓋掉 Tray**——這是 golden 既有的鍵名
     衝突，照翻，不要在移植樹「順手修好」。
  4. `FormClose` 會呼叫 `BtnOneCycleClick`（機台動作）——網頁版**關頁不要照做**這一步，只做存檔。
* **待 Steven 決定**：無獨立決策題（AOA offset 相關的兩題見「9. AOA offset」節；那是另一個表單
  `TfMain` 的 `Ed_*Offset_*`，不是這裡的 `TfAutoAlignment` 22 鍵）。
* **要等 Jimmy 的前提**：CCD 對位的 TCP 通訊（`AutoAlignment.cpp:4664-5325`，讀
  `fMain->Ed_*Offset_*->Text`）——移植樹沒有這段；`ClientSocket` 相關指令一併屬 Jimmy。
* **commit**：`c675594d`。

---

## 3. LaserSensor（雷射感測器）

* **建議頁面檔名**：`HW.LaserSensor.html`（建議，未定案）。
* **golden 檔**：`OmronLaser\LaserSensor.cpp`（V912），表單 `TfLaserSensor`；相關檔
  `LaserSensorInArm.cpp`／`LaserSensorShuttle.cpp`（只設任務游標，本身不讀寫檔）。
* **移植入口**：**不走 C 形狀替身**——這支是用真的 `vclcompat` 元件直接翻的（流程碼也共用），
  本體在 `OmronLaser/LaserSensor.cpp`（移植樹既有檔案，不是本輪新增）。讀檔已於 `8af13c07`
  全部接上；`c675594d` 這次補開機呼叫 golden `TfMain::FormShow :11189` 的 `ReadLaserFile`。
* **PageDesc 有沒有登記**：**沒有**。
* **讀檔時機**：`8af13c07` 已經全接；`c675594d` 再補一個開機呼叫點（`:11189` `ReadLaserFile`）。
* **存檔函式與 golden 觸發鈕**：`sbUpdateClick`（golden `:1109-1127`）——**golden 只有表單上的
  Update 鈕**，這支沒有觸發點（沒有頁面）。存檔會用到 `fMain->BackupSetupFile()`（同 S63 的
  `FixAICCD`，移植樹目前是 offline no-op）。
* **必送欄位**：待確認（這支不是 C 形狀替身，沒有 `k*_SaveReads` 這種產生器清單可查；要看
  `LaserSensor.cpp` 的 `sbUpdateClick` 本體逐行列）。
* **要建的元件 id**：待確認（同上，這支的元件是真的 `vclcompat` 物件，不是具名替身表，要另外
  盤點 `LaserSensor.h`）。
* **頁面事件要 JS 做的**：待確認。
* **已知陷阱**：**建頁要另外設計**——因為不是 C 形狀替身模式（沒有 `tools/editlist/*.py` 可以
  照抄），跟這份清單裡其他表單的做法不一樣，動手前要先想清楚要不要沿用既有的 `vclcompat` 元件
  還是另外包一層。
* **待 Steven 決定**：無獨立決策題。
* **要等 Jimmy 的前提**：`TfLaserSensor` 靜態初始化時看不到 `USE_LASER_DISTANCE`（那時
  `Gerneral.ini` 還沒讀）——有雷射的機台永遠不開 COM，這條交給 Jimmy。
* **commit**：`8af13c07`（讀）、`c675594d`（開機呼叫點）。

---

## 4. TrayMapping ScanLine（S74）

* **建議頁面檔名**：待確認——`ScanLine` 可能是既有 `TrayMapping` 頁面的一個分頁，不是獨立頁面
  （本檔沒有查到現有 `HW.TrayMapping.html` 或類似頁面是否存在，待確認）。
* **golden 檔**：`cTrayMapping.cpp`（V912），表單 `TfTrayMapping`。
* **移植入口**：`forms/fTrayMapping.cpp`／`.h`（既有檔案，非本輪新增）；`W906_TrayMapping_BootFlags()`
  （`c675594d` 新增，開機重算 `bUseTrayMap` 等旗標，golden `cTrayMapping.cpp:92-104`——這些旗標
  原本在**靜態初始化時**算、那時 `Gerneral.ini` 還沒讀，永遠 `false`，現在改成開機讀完之後重算）。
* **PageDesc 有沒有登記**：**沒有**。
* **讀檔時機**：`ReadFile_ScanLine`（golden `:5635`／`:5661`）——**仍在 GATE**，等 Jimmy 的
  G-7／G-8（AOI 物件 `mapAOI`＋log 回呼）做完才能解。
* **存檔函式與 golden 觸發鈕**：`SaveFile_ScanLine`（golden `:5701`／`:5729`）——同樣在 GATE；
  唯一呼叫者 `spbSaveClick` 也在 G-2。
* **必送欄位**：待確認（GATE 還沒解開，產生器沒有針對這兩支方法產生清單）。
* **要建的元件 id**：缺 6 個元件（`forms/fTrayMapping.h` 的 GATE 註記提到，逐一名稱待確認）。
* **頁面事件要 JS 做的**：待確認。
* **已知陷阱**：
  1. `ReadFile_ScanLine`／`SaveFile_ScanLine` **各有兩個多載**，C 形狀產生器只抓得到第一個，
     要手翻（不能單靠 `tools/gen_editlist.py` 自動產生）。
  2. 唯一呼叫者 `spbSaveClick` 也在 `GATE (G-2)`，就算 `ReadFile_ScanLine`／`SaveFile_ScanLine`
     解了，沒有 `spbSaveClick` 這條鏈也叫不到。
* **待 Steven 決定**：無獨立決策題（這節本身就是「等 Jimmy」，見下）。
* **要等 Jimmy 的前提**：**整節都卡在這裡**——G-7／G-8（AOI 物件 `mapAOI` 與 log 回呼）、
  `spbSaveClick` 的 G-2，三個 GATE 都要 Jimmy 解開才能繼續。
* **commit**：無（這節目前沒有讀寫檔 commit，仍是 S74，卡在 GATE）。

---

## 5. Magazine（料匣）

* **建議頁面檔名**：`HW.Magazine.html`（建議，未定案）。
* **golden 檔**：`Magazine.cpp`（V912），表單 `TfMagazine`。
* **移植入口**：`FileRW/TestIF_File_Magazine.cpp`／`.gen.inc`；`tools/editlist/TestIF_File_Magazine.py`；
  前綴 `MG`；commit `c79ee4e9`（S62）。
* **PageDesc 有沒有登記**：**沒有**。
* **讀檔時機**：`fMagazine->ReadFile()`（移植樹既有的 `Magazine.cpp`，開機鏈 golden `:9351`／
  `:9357` 兩次都已接，非本輪新增）。**golden `DoReadLastData` 不會呼叫它的 `DoIniDataToForm`**
  ——只在 `FormShow :88`、`FormClose :354`、`spbSaveClick :3609` 呼叫，所以開機鏈沒有再加東西，
  `FileRW_Magazine_DoIniDataToForm()` 目前**沒有呼叫者**。
* **存檔函式與 golden 觸發鈕**：`spbSaveClick`（golden `:3547-3611`）→
  `FileRW_Magazine_spbSaveClick()`。流程：Tray Magazine Source、Mag Fix tray Type（三道檢查，
  不過只跳訊息、這一鍵不寫）、寫 `config.ini [Magazine Z Offset]` 32 鍵→顯示順序有變就
  `fShowBinSelect->InitShowBinDigital()`→`fMagazine->ReadFile()`→`DoIniDataToForm`。**golden 這支
  沒有 `BackupSetupFile()`**（跟 `FixAICCD`／`Rotate` 不一樣，這是 golden 本來的行為，不是漏翻）。
* **必送欄位**：`kMG_SaveReads`（`TestIF_File_Magazine.gen.inc`）只列到 3 個 `TRadioGroup`——
  **32 個 `Z offset` 輸入框沒有收進去**，因為它們是經 `EditInMag[i]`／`EditOutAuto[i]` 指標表讀的，
  產生器掃 `EL<>` 字面值掃不到。**登記 PageDesc 時 `mustSend` 要自己補**：
  `edInMagOfs01..16`、`edOutAuto3Ofs01..16`（共 32 個）。
* **要建的元件 id**：`EditInMag[16]`／`EditOutAuto[16]`（32 個 Z offset 輸入框）＋ 3 個
  `TRadioGroup`（Tray Magazine Source、Mag Fix tray Type、顯示順序，逐一 DFM 名稱待確認）。
* **頁面事件要 JS 做的**：待確認（`FormShow`／`FormClose` 的畫面連動細節本檔沒有列出）。
* **已知陷阱**：
  1. **替身沒灌值就呼叫存檔會把 32 個 0 寫進 `config.ini`**——golden 的存檔鈕只有 `FormShow`
     之後才按得到（`FormShow :87-88` 先 `ReadFile`＋`DoIniDataToForm`），本檔的替身開機只有
     DFM 設計期值（32 個輸入框都是 `"0"`；`ReadFile` 的預設值是 `-3`／`3`）。**登記 PageDesc 時
     `editlist.get` 要走 FormShow 那條鏈**（至少 `fMagazine->ReadFile()`＋
     `FileRW_Magazine_DoIniDataToForm()`），`editlist.save` 才套頁面值 → `MG_spbSaveClick`。
  2. `FormShow` 在 `iMagFixTrayType==1` 時會改寫 MOT 手動盤盤面資料（不是讀寫檔，是機台狀態）
     ——**待 Steven 決定 Q3**（做頁面時要不要照做），做之前先問 Jimmy。
  3. golden 怪處（照翻，不修）：兩個選項事件的 `if(CanChangeData(false)) return;` 其實什麼都
     沒擋（條件恆假或恆真，細節見 commit `c79ee4e9` 本文）；Fix 設 Bin 判斷寫死 `temp<=9`；
     Tray Magazine Source 存了也會被 `ReadFile` 固定成 0（存檔即被讀檔覆蓋，是 golden 既有行為）。
* **待 Steven 決定**：
  - Q2（與 S63 共用）：`Magazine`／`FixAICCD` 要不要做網頁——性質同 S73，建議先不做。
  - Q3：`Magazine` 開頁要不要照 golden 改寫 MOT 手動盤盤面資料——做頁面時再定，先問 Jimmy。
  - Q4：`Magazine` 沒有呼叫者，還要不要編進 `wb_serve` 建置——已照 A（兩個都編）做。
* **要等 Jimmy 的前提**：`Button10Click`／`btnMagazineTrayOutClick`（出盤動作，機台動作）。
* **commit**：`c79ee4e9`。

---

## 6. FixAICCD（AI CCD 固定架）

* **建議頁面檔名**：`HW.FixAICCD.html`（建議，未定案）。
* **golden 檔**：`FixAICCD.cpp`（V912），表單 `TfFixAICCD`。
* **移植入口**：`FileRW/TestIF_File_FixAICCD.cpp`／`.gen.inc`；`tools/editlist/TestIF_File_FixAICCD.py`；
  前綴 `FX`；commit `c79ee4e9`（S63）。
* **PageDesc 有沒有登記**：**沒有**。
* **讀檔時機**：`fFixAICCD->ReadFile()`（移植樹 `forms/fFixAICCD.cpp`，golden `:9370` 已接，非本輪
  新增）→ `DoIniDataToForm`（golden `:77-93`，這次接到 golden `DoReadLastData main.cpp:9410`，
  接在 `:9407 FrmRotate->DoIniDataToForm` 那一行之後；golden `:9409 fTrayMapping->DoIniDataToForm`
  移植樹還沒有，見 S74）。**golden 建構子（`:42`）沒有轉**——它沒有讀寫檔。
* **存檔函式與 golden 觸發鈕**：`spbSaveClick`（golden `:669`）→ `FileRW_FixAICCD_spbSaveClick()`。
  寫 `HandlerCondition.Data` 10 鍵 → `fFixAICCD->ReadFile()` → `fMain->BackupSetupFile()`
  （**移植樹目前是 offline no-op**，`forms/fMain.h:270`——golden 會真的備份配方檔，移植樹沒有
  備份，這是既有缺口，不是本輪造成的）。
* **必送欄位**：`kFX_SaveReads`（`TestIF_File_FixAICCD.gen.inc`）——含 `HSys.asFix2BGAAICCDIP`／
  `Port`＋`TestIF_File` 9 欄；**特別注意 `lblBGALightValue`**（見下面陷阱）。
* **要建的元件 id**：`HSys.asFix2BGAAICCDIP`／`Port` 對應的輸入框、`TestIF_File` 9 欄對應元件、
  `lblBGALightValue`（光源值標籤，捲軸連動，見陷阱）——逐一 DFM 名稱待確認。
* **頁面事件要 JS 做的**：待確認（捲軸 `scrBGALightValueChange` 的頁面端行為要看陷阱怎麼解）。
* **已知陷阱**：
  1. **開機後直接存檔會把光源值寫成 0**（golden 既有問題，照翻不修，交件報告列為決策題 Q1）：
     `lblBGALightValue->Caption` 只在 `scrBGALightValueChange`（`InitialOK` 之後）→
     `LightDataReflesh` 才更新；golden 開機 `DoReadLastData`（`main.cpp:9993`）早於
     `InitialOK=true`（`:10898`），捲軸 `OnChange` 第一行就 `return`；`:11328` 再設同值的
     `Position` 也不會觸發 `OnChange`——開機後沒動過捲軸就按存檔，「Fix2 AI CCD BGA Light
     Value」會寫成 DFM 的 `'0'`（`FixAICCD.dfm:352`）。移植樹沒有捲軸（`GATE (G-FX-BGALIGHT)`），
     替身 `Caption` 照 golden DFM 種成 `"0"`（產生器的 `DfmState` 不收 `TLabel Caption`，是在
     `Boot` 這裡手動補的）。**建頁時頁面要送捲軸值，或照 Q1 的裁決處理**。
  2. `TfMain::FormShow main.cpp:11325-11329`（`USE_Fix_AI_CCD` 時 `TimerDownFixAICCDConnect`
     開 CCD 連線＋`ChangeFix2AICCDSetupFile` 設捲軸→`OnChange`→COM2 光源控制）——外部設備動作，
     不在這裡做，交給 Jimmy（S48）。
* **待 Steven 決定**：
  - Q1：`FixAICCD` 光源存檔寫 0 的 golden 問題要不要修——建議 A：照 golden（先不修），請
    Jimmy 判斷 V912 是否也要修。
  - Q2（與 S62 共用）：要不要做網頁——建議先不做。
* **要等 Jimmy 的前提**：CCD 連線與光源控制（`main.cpp:11325-11329`、COM2 燈控）；
  `fMain->BackupSetupFile()` 補成真的備份（不只是這個表單卡，是共用缺口）。
* **commit**：`c79ee4e9`。

---

## 7. PE 模式鈕（主畫面按鈕，S58）

* **建議頁面檔名**：不是新頁面——`main.html` 上加一顆客戶專屬鈕（KYEC／ASE-K＋`[C12]`）。
* **golden 檔**：`main.cpp`（V912），`TfMain::sbPEModelClick`（`:33724-33778`）。
* **移植入口**：新檔 `FileRW/MainClick.cpp`，函式 `W906_Main_PEModelOp`；commit `c913d5e5`。
* **PageDesc 有沒有登記**：不適用（這是 `act.*` 動作通道，不是 editlist 頁面）——**還沒有分派**：
  `act.main.peModel` 這一支要接在 `tools/wb_serve.cpp` 的 WS 指令分派鏈上，Jimmy 正在改那條鏈
  （第 8 條 `hw.access`，`TO_STEVEN.md` §1），所以先不插，片段已經寫好放在
  `D:\docs\ops\registers\HT9045_待插入片段.md`（片段 1）。
* **讀檔時機**：讀檔端已 live——`database.cpp:1520`（`SYSTEM_MODULAR::ReadGeneralIni`，開機
  `LoadMachineConfig` 跑）。
* **存檔函式與 golden 觸發鈕**：`W906_Main_PEModelOp(payloadJson, &ok)`——`{"op":"get"}` 回目前
  狀態，`{"op":"click"}` 跑 golden 本體，寫 `Gerneral.ini [System] bHasEnteredPEModel=1`。
* **必送欄位**：三個輸入框的 `Enabled` 狀態放在回應的 `ui` 欄（細節見 commit 本文）。
* **要建的元件 id**：`main.html` 上目前沒有這顆鈕，要新增；元件 id 待確認（頁面還沒建）。
* **頁面事件要 JS 做的**：按鈕點擊送 `act.main.peModel {"op":"click"}`；頁面開啟／輪詢時可送
  `{"op":"get"}` 讀狀態。
* **已知陷阱**：golden `main.cpp:33726` 的守衛疑似寫反——這是交件報告列出的決策題，不是本檔
  自己發現後修掉的，**照翻，不要自己「修正」**。
* **待 Steven 決定**（P8 五題裡與這個按鈕直接相關的）：
  1. 主畫面要不要放 PE 鈕（目前沒有加進 `main.html`）。
  2. golden `:33726` 守衛疑似寫反，要不要處理。
* **要等 Jimmy 的前提**：`tools/wb_serve.cpp` 的 `hw.access` 分派線落地（Jimmy 正在改）。另外
  golden `KYECFTP/FTPClient.cpp:1017-1021`（FTP 下載 Setup File 後清旗標）與 `:1142`
  （上傳前擋）也用到 `bHasEnteredPEModel`，移植樹 `KYECFTP/` 沒翻這幾行——KYEC 客戶專屬的 FTP
  表單，不在這節範圍內。
* **commit**：`c913d5e5`。

---

## 8. DUT on/off（SitePanel 點格，S59）

* **建議頁面檔名**：不是新頁面——是 `main.html`（或既有的 Site Panel 頁面）上點格子的既有互動。
* **golden 檔**：`main.cpp`（V912），`TfMain::mtDutOnOffMouseUp`（`:29930-30615`，本次翻的是
  `:30329-30374` 那一段）。
* **移植入口**：`FileRW/MainClick.cpp`，函式 `W906_Main_DutOnOff_SaveATC7Channels`；commit
  `c913d5e5`。
* **PageDesc 有沒有登記**：不適用（動作通道，非 editlist）。
* **讀檔時機**：不適用（這支只寫，不讀）。
* **存檔函式與 golden 觸發鈕**：`W906_Main_DutOnOff_SaveATC7Channels`——寫配方
  `Temperature.Data [ATC] ATC7CH1..4Enabled`，再讀回 `Temperature.bATC7ChannelEnabled[]`。
* **必送欄位**：待確認（本體只翻了寫檔那一小段，完整輸入形狀要等 Jimmy 翻完整支
  `mtDutOnOffMouseUp` 才能定）。
* **要建的元件 id**：待確認——網頁 `SitePanel` 點格目前是關掉的，要等 Jimmy 那邊翻完才能接。
* **頁面事件要 JS 做的**：待確認。
* **已知陷阱**：**沒有呼叫端**——`SendCommToATC7`（golden `:30375`，對 ATC 溫控器的通訊，外部
  設備）不在這支裡，回傳兩個字串給呼叫端送；整個 `mtDutOnOffMouseUp`（守衛、
  `LastSet.bUseTestSocket`、`ChangeATCSiteUse`、Auto Site Mapping、site log、送 ATC7）是底層
  流程＋外部設備，要等 Jimmy 翻。Jimmy 翻那支時要在 golden `:30329` 的位置呼叫
  `W906_Main_DutOnOff_SaveATC7Channels()`，回 `true` 就照 golden `:30375` 送 ATC7。
* **待 Steven 決定**：無獨立決策題（這節本身就是「等 Jimmy」）。
* **要等 Jimmy 的前提**：**整個 `mtDutOnOffMouseUp` 都要 Jimmy 翻**，這支只是預先做好的一小塊
  存檔本體，等 Jimmy 翻完主體才會被呼叫。
* **commit**：`c913d5e5`。

---

## 9. AOA offset（頁面已建但功能受限）

* **頁面**：`Main.AOAInfo.html`（**已經有頁面**，不是待建；這節放在這裡是因為功能受限，跟其他
  「還沒有頁面」的表單性質不同，先在此註記）。
* **golden 檔**：`main.cpp`（V912），`TfMain::OffsetSaveClick`（`:34956-35049`）、`FormShow
  :11679-11724`＋`:11736`。
* **移植入口**：`FileRW/AOAOffset.cpp`／`.gen.inc`；`tools/editlist/AOAOffset.py`；commit
  `611edb7e`。
* **PageDesc 有沒有登記**：C 路已接（`GOLDEN_BRIDGE` 有 `Main.AOAInfo.html` 這一項）。
* **讀檔時機**：開機（`FileRW_AOAOffset_Boot()`，接在 `ShuttleMove` 讀檔那一行，golden `:11740`
  順序），只在 `MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF_File.bEnableAutoAlignment` 成立時才
  真的填值。
* **存檔函式與 golden 觸發鈕**：`OffsetSaveClick`（`:34956-35049`）→ 38 個 `Ed_*Offset_X/Y` →
  `iAOA_*`（`atoi`）→ `WriteIniDataGeneral` ×26（`AUTO_EMPTY_COLOR>=3` 時 ×38）。golden 沒有
  權限檢查、沒有確認框、沒有 `return`，寫完不重讀。
* **必送欄位**：38 個 `TEdit`（`mustSend`，見 `AOAOffset.gen.inc`）。
* **要建的元件 id**：頁面已建（38 個輸入框＋`pnlAOAForHT9011`＋`OffsetSave`，帶 DFM id）。
* **頁面事件要 JS 做的**：頁面已接，不需要新事件（除非 AOA①的裁決是「開放」，那就要加輸入
  驗證與送出邏輯）。
* **已知陷阱**：
  1. **golden 38 個輸入框全部 `Visible=False`**（`main.dfm`），golden 全樹沒有地方打開它們——
     操作員在 golden 畫面上本來就看不到、改不到；網頁替身照 DFM 設 `editable=false`，頁面送的
     值會進 `ack.ignored`，存檔寫的是開機時填進元件的值。
  2. 開機時條件不成立（`MACHINE_HAS_AUTO_ALIGNMENT_CCD=0` 或配方 `bEnableAutoAlignment=false`）
     時元件留在 DFM 的 `0`——按 `OffsetSave` 會把 `Gerneral.ini AOA_*` 全部寫成 0（本機目前
     `MACHINE_HAS_AUTO_ALIGNMENT_CCD=0`、26 鍵全 0，所以寫 0 不改值，還沒實際發生過）。
  3.（不在本檔範圍）golden `AutoAlignment.cpp:4971` `iYPos+atoi(Ed_Auto3Offset_X)`（Auto6 同型）
     疑似把 X 打進 Y——golden 既有寫法，翻譯 CCD 對位流程時要照翻並註記。
* **待 Steven 決定**：
  - AOA①：網頁要不要能改 AOA offset——A 照 golden 改不到（現狀）／B 開放但偏離 golden；
    工程師建議 A。
  - AOA②：開機時 AOA 沒開、之後按 Save 會把 26 鍵寫成 0（golden 既有地雷）——A 頁面警告／
    B 伺服器拒存／C 每次開頁重填；工程師建議 A。
* **要等 Jimmy 的前提**：無（讀寫檔本身沒有卡 Jimmy；CCD 對位流程本身屬 Jimmy，見「2.
  AutoAlignment」節）。
* **commit**：`611edb7e`。

---

## 10. Alert.Password Event Log 兩分頁（S71）

* **建議頁面檔名**：待確認（`Alert.Password` 可能是既有頁面的一部分，或是新頁面，本檔沒有查證）。
* **golden 檔**：待確認（讀事件記錄檔那一支的 golden 來源本檔沒有查到明確行號）。
* **移植入口**：**尚未移植**。
* **PageDesc 有沒有登記**：沒有（尚未移植，無從登記）。
* **讀檔時機**：待確認。
* **存檔函式與 golden 觸發鈕**：不適用（這是讀事件記錄檔，WS 跑 golden，非存檔）。
* **必送欄位**：待確認。
* **要建的元件 id**：待確認。
* **頁面事件要 JS 做的**：待確認。
* **已知陷阱**：無（尚未開工，沒有陷阱紀錄）。
* **待 Steven 決定**：無獨立決策題。
* **要等 Jimmy 的前提**：不是卡 Jimmy——**暫不派**是因為與 Steven 另一個 session（STEVEN-NB3，
  分支 `v906/steven-gpib-widget`）的 cMyDB CSV 版移植 P1（S72，`slEventLog` 等事件記錄基礎建設）
  範圍重疊，要等那邊的計畫確定範圍後再排。
* **commit**：無（尚未開工）。

---

## 11. AutoCalSuckZ（S70，golden `TfProductionInfo` 的 `tsCalibrateSuckZHeight` 分頁）

* **建議頁面檔名**：`Setup.AutoCalSuckZ.html`（建議，未定案；tag＝`AutoCalSuckZ`；入口要等 D2
  決定要不要做）。
* **golden 檔**：`ProductionInfo\ProductionInfo.cpp`（V912），只取 `tsCalibrateSuckZHeight`
  這一個分頁（不是整張 `TfProductionInfo` 表單）：`ShowAutoCalSuckZForm :6105`、
  `btnAutoCalSuckZSaveClick :6135`、`btnCancelClick :5931`、`FormClose :4684`、
  `InitAutoCalSuckZ :6019`；`ProductionInfo.dfm`。
* **移植入口**：`FileRW/AutoCalSuckZ.cpp`／`.gen.inc`；`tools/editlist/AutoCalSuckZ.py`；
  結構 `AutoCalSuckZ`；前綴 `ACZ`；commit `58bd9425`。
* **PageDesc 有沒有登記**：**沒有**。建頁時：把 `ShowAutoCalSuckZForm` 加進 `METHODS`，並
  `replace` 掉 `Left`／`Top`／`ShowSheet` 三行當 `formShow`；`kLists={&elData}`、
  `kListNames={"elData"}`；`saveFlow=ACZ_btnAutoCalSuckZSaveClick`、
  `savedMark="ACZ_SaveEditTextToFile"`、`reload=ACZ_LoadAutoCalSuckZ`；`_integrated.txt`
  已經有 `AutoCalSuckZ`（`58bd9425`）。
* **讀檔時機**：建構子在開機（golden `CreateForm HT9045.cpp:257`，22 個 `elData` 項目，不碰
  檔案）；`DoIniDataToForm` 接到 golden `DoReadLastData :9430`——golden 一進去就 `return`，
  除非 `CosFunction.bOEEFunction`（只有 `CC_Greatek` 開），其他機台維持建構子預設值，照
  golden；OEE 本體仍閘著（RULINGS_20260926 第 25 條）。
* **存檔函式與 golden 觸發鈕**：`SaveAutoCalSuckZ`／`btnAutoCalSuckZSaveClick`（`:6135`）
  已翻譯，**沒有呼叫者**——golden V912 本身也沒有任何入口能打開這一頁（`cConfiguration.dfm:
  17154` 的 `btnAutoCalSuckZ` 沒有 `OnClick`，見陷阱⑤／D2）。
* **必送欄位**：`kACZ_SaveReads` 是空的（存檔走 `elData`、不直接讀元件）⇒ 必送＝`elData` 的
  22 筆全部（同 `Ld_UldDelayTime` 的做法）。
* **要建的元件 id**：
  - 入料臂分頁 `tsInArmSuckZ`：`pcInOutArmSuckZ` > `tsInArmSuckZ`；`Panel12`：
    `cbInarmSuckZAutoEnable`；`Panel13` > `GroupBox12`「InArm Z」：`Label90..97`＝A..H、
    `setEditZ1A`..`H`；`gbTestAreaInarm`：`edInarmTestAreaX`／`Y`；`gbInarmSearchStartZ`：
    `edInarmSearchStartZ`。
  - 出料臂分頁 `tsOutArmSuckZ`：`Panel10` > `GroupBox13`：`setEditZ2A`..`H`（DFM 宣告順序
    G,E,C,A,H,F,D,B，位置照 DFM 的 Left/Top 排）；`gbTestAreaOutarm`：
    `edOutarmTestAreaX`／`Y`；`gbOutarmSearchStartZ`：`edOutarmSearchStartZ`；`Panel11`：
    `cbOutarmSuckZAutoEnable`。
  - 底部 `plBottomArmAutoSuckZ`：`SpeedButton1`「Abort」（`btnCancelClick`）、
    `btnAutoCalSuckZSave`「Save」。
  - 容器父子關係已在 `.gen.inc` 的 `kACZ_ParentOf` 產生好。
  - 欄位對照：`setEditZ1A`→`iInArmZHeightDiff[0][0]`、Z1B→`[1][0]`、Z1C→`[0][1]`……
    （字母＝`'A'+col*2+row`）；ini 鍵名 `iInArmZHeightDiff<row>_<col>`。只有高度差有範圍
    `-500..500`。
* **頁面事件要 JS 做的**：Abort＝關頁（golden `FormClose` 在 OEE 機台會重讀檔，非 OEE 不動）；
  Save 之後 golden 會 `Close`（`ack.trace` 記 `closed`），頁面自己關；這些 edit 在 DFM 沒有
  小鍵盤事件。
* **已知陷阱**：
  1. 開頁就會把檔案值讀進機台正在用的全域，而且不看 `bOEEFunction`。
  2. 系統檔 `D:\HT9045\system\ProductionInfo\AutoCalSuckZ.Data`，**不跟配方**——存一次所有
     配方都受影響。
  3. 讀檔也會重寫檔、建資料夾（golden `UpdateFile`／`MyForceDirectories`）。
  4. golden 存檔沒有任何權限檢查（沒有 A02、沒有 `Insufficient`）、沒有確認框。
  5. **golden V912 沒有任何入口能打開這一頁**（設定頁 `[N14_23] btnAutoCalSuckZ` 在
     `cConfiguration.dfm:17154` 沒有 `OnClick`）——這是 D2 的背景。
  6. 兩個 `uPoint2D` 定義放在 `FileRW/AutoCalSuckZ.cpp`（避免對 `ht9045_sm` 產生連結相依，
     見 D4）。
* **待 Steven 決定**：
  - D1：`AutoCalSuckZ` 只在 OEE 機台讀——照 golden，這題是確認不是問題。
  - D2：要不要做網頁——建議不做（golden 本身也沒有入口能開這頁）。
  - D4：兩個 `uPoint2D` 定義要放哪——目前放在 `FileRW/AutoCalSuckZ.cpp`。
* **要等 Jimmy 的前提**：
  - J1：`cinitial.cpp` GATE n5-G17／G18 解開（`:16413-16418`、`:16439-16444`）並 include
    `forms/fProductionInfo.h`——`Enable=1` 時吸嘴 Z 高度會加 `iIn/OutArmZHeightDiff`（±500）。
  - J2：量測流程 `DoInArmAutoCalSuckZ`（`ainarm9045.cpp:3105` GATE；golden `:9075-9230`）；
    存檔前寫替身 `EL<TEdit>("TfProductionInfo","setEditZ1A".."H")`；需要函式指標跳板；順手把
    `uPoint2D` 定義搬到 `Public/HTEditList.cpp`。
* **commit**：`58bd9425`。

---

## 12. TZteach（S68，Auto Teach 表單）

* **建議頁面檔名**：`HW.AutoTeachZ.html`（建議，未定案）。這是**機台動作頁**；寫檔只是量測
  完的副產品，頁面上沒有可以直接編輯的欄位。
* **golden 檔**：`AutoTeach\InOutArmZteach.cpp`／`.dfm`（V912，5171 行）；`SaveFile :4653`、
  `SaveSetupFile :4674`。
* **移植入口**：新檔 `FileRW/Zteach.cpp`（手寫，`FileRW_Zteach_SaveFile(int,int,bool)`；
  **不是 C 形狀、沒有 HTEditList**）；commit `58bd9425`。
* **PageDesc 有沒有登記**：不適用（`SaveSetupFile` 不讀任何元件，見下）。
* **讀檔時機**：不適用／待確認（本檔只翻了 `SaveFile`／`SaveSetupFile` 這兩支寫檔函式，讀檔
  端的接法本檔沒有記錄，待確認）。
* **存檔函式與 golden 觸發鈕**：`SaveFile :4653`／`SaveSetupFile :4674`（`InArm` 20 鍵、
  `OutShuttle` 19 鍵）。**golden 既有的筆誤照翻並註記**：`:4741`／`:4743` 寫的是
  `dPick[sel][j][j]` 而不是 `[j][i]`，16 格裡有 12 格會拿到過期值（見 D3）。唯一呼叫者是
  自動教導 Z 的機台流程（Jimmy，J3）。
* **必送欄位**：無（`SaveSetupFile` 不讀任何元件）。
* **要建的元件 id**：
  - `GroupBox8`「InArm Z」：`AEdit1`..`8`（`OnClick`＝`AEdit1Click`）、`SendInarm`。
  - `GroupBox9`「OutArm Z」：`AOutEdit1`..`8`、`SendOutarm`。
  - 結果與設定：`StringGrid1`（`Cells[6]` 教導 Z、`[7]` Z offset；`OnMouseDown`）、
    `sgThreePointTeach`、`Memo1`。
  - 選擇：`AutoTeachSingle`、`rgTeachMode`、`gbInarm`（`sbLoader`、`sbHotPlate1/2`、
    `sbInSht1/2`、`sbInZ_Calibration`、`sbInRotate`）、`gbOutArm`（`sbOutSht1/2`、
    `sbAuto1-3`、`sbFix1-3`、`sbOutZ_Calibration`、`sbOutRotate`，全部 `OnClick`＝
    `sbLoaderClick`）。
  - 勾選與移動：`cbEnableUseAreaZCalbration`、`CbEnRecordVacuum`、`Panel2`
    （`MotorInArmX/Y`、`SpeedButton12/13`「Move ±」、`SpeedButton1`「Start Auto Z
    offset」、`ComboBox1`、`edtPosition`）。
  - 其他：`Button1`「Vacuum Check」、`SpeedButton11`「Start」、`SpeedButton14`「STOP」、
    `SendData`、`BitBtn1`「Exit」、`AutoTimer`、`FormShow`／`FormClose`。
* **頁面事件要 JS 做的**：幾乎全部是機台動作鈕，要走 `hw.access`（RULINGS_20260926 第 8 條，
  Jimmy）；頁面只負責顯示 `StringGrid1` 的結果。
* **已知陷阱**：
  1. golden `:4741`／`:4743` 的 `[j][j]` 筆誤會把錯的值寫進 `Position Offset.Data`（16 格
     只有 4 格寫對）（見 D3）。
  2. 這個檔和 Offset 頁（`Offset_File`，C 路）是**同一個檔，兩個寫者**——頁面開著時 `TZteach`
     寫檔會和 Offset 頁的暫存值互蓋。
  3. 寫 Hot 還是一般檔，由存檔當下的 `LastSet.iTemperature` 決定。
  4. 入料臂寫 `InArmOffSet`（執行期）、出料寫 `OutArmOffSet_File`（檔案那份），**不對稱**。
* **待 Steven 決定**：D3——`[j][j]` 筆誤要不要修，建議 **A：照翻並通報 Jimmy／V912 維護者**。
* **要等 Jimmy 的前提**：J3——`TZteach` 本體＋`hw.access`（`AutoTeachZ`、`AutoTeachPos`、
  `DoZHome`、`InOutArmZHome`、真空檢查、各按鈕）；翻的時候把 `FileRW/Zteach.cpp` 兩支函式
  搬進那個 TU、`FileRW/Zteach.cpp` 退役。另外 J4——通報 V912 維護者
  `InOutArmZteach.cpp:4741`／`:4743` 的 `[j][j]` 筆誤。
* **commit**：`58bd9425`。

---

## 13. `TFrmAOI`（S69，AOI.Data）

* **建議頁面檔名**：`web/page/Setup.AOI.html`（建議，未定案）。
* **golden 入口**：主畫面 `sbCCDClick`（`main.cpp:29616-29627`）：先過權限
  `Insufficient(37)`，再 `SetWorkParameter()`，只有 `USE_AOI_Inspection`、
  `USE_Scanner_AOI_Inspection>Uninstall` 或 `USE_Top_Scanner_AOI_Inspection` 成立時才
  `FrmAOI->Show()`。
* **golden 檔**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 的 `fAOI.cpp`、
  `fAOI.dfm`、`fAOI.h`（類別 `TFrmAOI`）。
* **移植入口**：`FileRW/AOISetup.cpp`／`.gen.inc`；`tools/editlist/AOISetup.py`；結構
  `AOISetup`；前綴 `AO`；`FileRW_AOISetup_Boot`／`_ReadFile`／`_spbSaveClick`；commit
  `9ec84450`。
* **PageDesc 有沒有登記**：**沒有**。建頁時：①把 `FormShow :3769` 加進 `METHODS`（它會呼叫
  `fAOI_ReadFile`、依安裝型態切換分頁顯示；`AOIFailCountRefresh` 與 `KYEC` 段要擋）
  ②`tag=AOISetup`、`form=TFrmAOI`、`saveFlow=AO_spbSaveClick`、
  `savedMark=AO_WriteIniData`、`reload=AO_fAOI_ReadFile`
  ③`s12c_page_probe` 通過後才加進 `_integrated.txt`（⚠ `9ec84450` 已經先加進
  `_integrated.txt` 讓它編進 `wb_serve`，登記頁面時要注意這點）。
* **讀檔時機**：**已接開機／換配方讀檔**（commit `e6a8e0fc`：`tools/wb_serve.cpp:3441`
  `W906_DoReadLastData` 呼叫 `FileRW_AOISetup_ReadFile()`，golden `main.cpp:9374`）。只讀
  `AOI.Data`，不補寫缺鍵；目前配方 `QPM5577_8` 沒有 `AOI.Data`，所以會用預設值、不建檔。
* **存檔函式與 golden 觸發鈕**：`spbSaveClick`（golden `:3120-3367`），**還沒有呼叫者**
  （沒有頁面、沒有 WS 指令）。
* **必送欄位**：86 個（`kAO_SaveReads`），依分頁：
  - TabSheet1 `Normal_setting`（20）：`cbEnabledTopView`、`cbEnabledPADView`、
    `cbEnabledBGAView`、`edt_FailTopView`、`edt_TimeoutTopView`、`edt_SDelayTopView`、
    `edt_FailPADView`、`edt_TimeoutPADView`、`edt_SDelayPADView`、`edt_FailBGAView`、
    `edt_TimeoutBGAView`、`edt_SDelayBGAView`、`edt_AlarmCount`、`cbBGAFailRun`、
    `cbAutoSkipBGA`、`cbAutoSkipPadPkg`、`cbAOINoSort`、`edt_PassFailBGAPADView`、
    `edt_FailFailBGAPADView`、`cbTesterFailBin`。
  - TabSheet2（7）：`cb_VitContiuneAlarm`、`cb_AlarmBySite`、`cb_AlarmByArm`、
    `cb_AlarmBySite1`、`cb_AlarmByArm1`、`edt_ConAlarmConut`、`edt_ConAlarmConut1`。
  - TabSheet3 `Scanner Mode`（33）：`rgScannerMode`、`edtScannerReadTimeout`、
    `edtAOIIntervalCounter`、`edtAOIRetryCounter`、`rgAOIFailBinType`、`cbAOIFial`、
    `cbAOIFialAndTestPass`、`cbBDByTotal`、`cbBDByTotalConti`、`cbBDBySite`、
    `cbBDBySiteConti`、`cbDBAlarmAutoReset`、`edtAOIBDCounterTotal`、
    `edtAOIBDCounterTotalConti`、`edtAOIBDCounterSite`、`edtAOIBDCounterSiteConti`、
    `pnlAOIBDTotalLast`、`pnlAOIBDTotalContiLast`、`pnlAOIBDSite1Last`、
    `pnlAOIBDSite2Last`、`pnlAOIBDSite1ContiLast`、`pnlAOIBDSite2ContiLast`（這幾個存的是
    `Caption`）、`edt_SDelayScanAOI`、`edt_TimeoutScanAOI`、`edtScannerICGain`、
    `cb_ScanAOIUseLGAMode`、`edtScannerLGAWaitTime`、`cb_LGAModeDirection`、
    `cb_ScanAOIAlarmBySite`、`cb_ScanAOIAlarmByArm`、`edt_ScanAOIAlarmCountBySite`、
    `edt_ScanAOIAlarmCountByArm`、`cb_ScanAOIUnUseFailBin`（`cb_EnabledPositionByAOI` 在
    同一頁但目前被擋，不在必送清單）。
  - TabSheet4（5）：`cbDevice`、`cbBaudRate`、`cbByteSize`、`cbStopBit`、`cbParity`。
  - TabSheet5 `TOP Scanner`（12）：`rgTopScannerMode`、`edtTopAOIIntervalCounter`、
    `edtTopAOIRetryCounter`、`cbTopAOIFial`、`cb_TopScanAOIAlarmBySite`、
    `edt_TopScanAOIAlarmCountBySite`、`cb_TopScanAOIAlarmByArm`、
    `edt_TopScanAOIAlarmCountByArm`、`edt_SDelayTopScanAOI`、`edt_TimeoutTopScanAOI`、
    `edtTopScannerReadTimeout`、`cb_TopScanAOIUnUseFailBin`。
  - TabSheet6（5）：`cbTopDevice`、`cbTopBaudRate`、`cbTopByteSize`、`cbTopStopBit`、
    `cbTopParity`。
  - 只顯示、不必送：`lblAOIBinSel1`、`lblAOIBinSel2`。
* **要建的元件 id**：門面既有的 14 個元件已經是替身，**不要另建一套**；不能用具名替身的
  `TTMyTray` 格子：`mtDutOnOff_Vitrox`、`mtDutOnOff_Vitrox1`（四張 Vitrox 表）、
  `mtAOIBuffer`（`MOT[MMScanAOI]`）——建議比照 `BinSel` 直接用 JSON 交換陣列；Top&Bottom 的
  `tsTopBtnModSet`、`tsTopBtnCommSet` 兩頁要等 Jimmy。
* **頁面事件要 JS 做的**：`rgAOIFailBinTypeClick`（`lblAOIBinSel1` 換文字、`lblAOIBinSel2`
  與 `cbAOIFialAndTestPass` 顯示／隱藏）；`cbTesterFailBinMouseUp`（`:4022`）與
  `FormShow :3845-3860`（`Label11`、`edt_FailBGAView`、`Label6`、`edt_FailPADView`、
  `Panel3` 顯示／隱藏）；小鍵盤範圍 `edt_FailTopView` 1..15、`edtAOIRetryCounter` 0..15、
  `edt_SDelay*` 0..5000、`edt_AlarmCount` 1..10000、`edtScannerReadTimeout` 小數
  1..120；要做成 WS 動作：點格子切 Dut on/off、`sbBDTotalResetClick`（`:4348`，計數歸零）、
  `mtAOIBufferMouseDown` → `EditTray(MMScanAOI)`（`HasICUnderMachine()==false` 才行，否則
  `MES1646`）。
* **已知陷阱**：
  1. 替身沒灌值就存檔會把 DFM 設計期值（`"cbCOM"`、`"cbBaudRate"`、`0`）寫進檔，存檔前一定
     要先跑 `FormShow`／`ReadFile`。
  2. 第一次存檔有正規化差異（新增 `iAOIFailBinType`、`iAOIFialAndTestPass` 兩鍵；
     `StartDelayTimeScanAOIView` 變成 Top 欄位的值＝golden 錯誤 1；`bEnabledPositionByAOI`
     不寫），探針要 `--allow`。
  3. `vclcompat` 的 `TRadioGroup` 不像 VCL 會把 `ItemIndex` 夾在 `Count-1` 以內。
  4. 讀 `[AOITRAY]` 字串清單索引超出時 `vclcompat` 回空字串、VCL 丟例外。
  5. `AOI.Data` 另有 `[OCR SETTING]` 屬 `TfOCR`（不在移植樹），B 路整檔鏡像要替 `aoi.data`
     加 owner gate。
  6. 門面的 14 個元件已經是替身，不要另建一套（同「要建的元件 id」提醒）。
  7. golden 錯誤照翻：Top 延遲寫進 Bottom 鍵（`:3300` vs 讀 `:3494`）；`TimeOut` 鍵名讀寫
     不一致（寫 `TimeOutTopScanAOIView :3301`、讀 `TimeOutScanTopAOIView :3495`）；Top
     Scan AOI 5 個警報設定只寫不讀；`iBallDamageType` 只讀不寫；`fScannerICGain` 讀兩次。
* **待 Steven 決定**：
  1. `cprod.h` 要不要補 `bEnabledPositionByAOI`——建議 Steven 團隊直接補一行。
  2. golden 錯誤 1、2（見陷阱⑦）要不要修——建議請 Jimmy 在 V912 修，移植樹跟進。
  3. Top&Bottom 70 多個參數要不要先做成純資料結構——建議等 Jimmy。
  4. 要不要做 AOI 頁面——建議與 S73 一起決定。
* **要等 Jimmy 的前提**：`TTopBottomInspect`（Top&Bottom 兩頁）、Scanner Comm 兩頁的
  Start／Stop（RS232）、AOI Tray 格子編輯（`EditTray`）、`IniConfig` `A74` 開啟時的
  `AOIFailCountRefresh`；另外：`AOI.Data` 的 `[OCR SETTING]` 由 `TfOCR` 讀寫（golden
  `OCR.cpp:2165`／`:2204`，`:9366` 未接），**建議另開一個 S 編號**，不算進 S69。
* **commit**：`9ec84450`（讀檔／存檔本體）、`e6a8e0fc`（開機／換配方讀檔鏈接上）。

---

## 14. `TfOCR`（S86，`AOI.Data` `[OCR SETTING]`）

* **建議頁面檔名**：`web/page/Setup.OCR.html`（建議，未定案）。`web/page/main.html:70` 已經有
  `sbOCR` 選單項。
* **golden 入口**：`main.cpp:29871` `sbOCRClick`：`Insufficient(48)` →
  `NewRecordProcess("MES2198")` → `fOCR->Show()`。
* **golden 檔**：V912 的 `OCR.cpp`／`OCR.dfm`／`OCR.h`（類別 `TfOCR`）。
* **移植入口**：`FileRW/IniConfig_OCR.cpp`／`.gen.inc`；`tools/editlist/IniConfig_OCR.py`；
  結構 `IniConfig_OCR`；前綴 `OC`；commit `973f2540`。
* **PageDesc 有沒有登記**：沒有。建頁時建議值：
  - `tag=IniConfig_OCR`、`form=TfOCR`
  - `formShow=OC_FormShow`、`saveFlow=OC_spbSaveClick`、`savedMark=OC_WriteIniData`
  - `reload=OC_FormShow`：golden 存完只重讀檔，沒有灌畫面，所以重讀要用 `FormShow`
  - 若片段（`_integrated.txt` 加一行 `IniConfig_OCR`）先進去，同 AOISetup 的注意事項（見 §13）
* **讀檔時機**：已接開機／換配方讀檔鏈（commit `973f2540`：`tools/wb_serve.cpp:3416`
  `W906_DoReadLastData` 呼叫 `FileRW_IniConfig_OCR_ReadFile()`，golden `main.cpp:9365`
  `fOCR->fOCR_ReadFile()`）。讀 `<配方>\AOI.Data [OCR SETTING]` 19 鍵（不補寫）＋
  `system\Gerneral.ini [OCR SETTING] "OCR Port"`（`CheckAndReadIniData`，缺鍵照 golden 補寫
  24）。目前配方 `QPM5577_8` 沒有 `AOI.Data`，會拿到 golden 預設值（Skip=3、Retry=10、
  WordCount=10，其餘 0／false／空字串）。
* **存檔函式與 golden 觸發鈕**：`FileRW_IniConfig_OCR_spbSaveClick()`（golden `spbSaveClick`
  `:2151-2189`），目前沒有呼叫者（沒有頁面、沒有 WS 指令）。
* **必送 19 個欄位（`kOC_SaveReads`）**：
  - 勾選框（11）：`cbCheckBarCodeMap`、`ckCompareOCRData`、`ckDisabledKeyin`、
    `ckOCRAndBinLog`、`ckOCRBinLogAddMark`、`ckOCRCheckHasIC`、`ckOCRLightChange`、
    `ckOCRLightNoDown`、`ckOCRMoveSRead`、`ckStartposshift`、`ckWordCount`
  - 輸入框（7）：`edOCRRetry`、`edOCRSkip`、`edOCRWordCount`、`edOCRWordType`、
    `edSetBlueLight`、`edSetRedLight`、`edStartposshift`
  - 單選（1）：`rgOCRTriggerMode`
  - 只顯示、不存：`IPPort2`；`TabSheet1`／`tsOCR_Cognex`／`tsOCR_Cognex_Setting` 三分頁的顯示
    與否由 `INSTALL_OCR` 與 `CosFunction.bTrayOCR` 決定。
* **頁面事件要 JS 做的**：小鍵盤範圍 `edOCRRetry`／`edOCRSkip`／`edOCRWordCount` 1..300；
  `edStartposshift` -50000..50000；`edSetBlueLight` golden 參數寫成 (255, 0)，上下限順序看起來
  反了；輸入框與捲軸 `sbSet*Light`（0..255）互相同步（golden 送光源那幾行已註解掉）；
  `edSetRedLight` 的 `OnClick` 在 golden DFM 接到 `edSetBlueLightChange`，看起來接錯；
  `ckWordCount` 在 DFM 掛在表單根，golden `:311-313` 把它移到設定分頁，頁面上應放在設定分頁。
* **DFM 設計期狀態**：`ckOCRCheckHasIC`、`ckOCRMoveSRead` 設計期隱藏，但存檔仍會寫；輸入框
  初值 `edOCRRetry`／`edOCRSkip`／`edOCRWordCount` 為 "2"、`edOCRWordType` 為 "AANN\*\*"、
  `IPPort2` 為 "23"。
* **已知陷阱**：
  1. 沒先跑 `FormShow` 就存檔，會把 DFM 設計期值寫進檔案。
  2. `rgOCRTriggerMode` 在 V912 的 `OCR.dfm` 不存在（見待 Steven 決定 Q1／要等 Jimmy J1，
     Steven 裁示不急，記 `todo.md` G-006）——量產機開 OCR 表單會 Access violation。
  3. `vclcompat` 的 `TRadioGroup` 不會夾 `ItemIndex`。
  4. `INSTALL_OCR≠0` 時每次開頁都會送 4 個 `MSG_CMD`（`fMain->SendMSG_CMD` 目前是 no-op；
     GB 工作把 GPIB 接成真的通訊後才會實際送出，要通知 Steven02）。
  5. 第一次存檔會補上 `iOCRTriggerMode` 鍵，驗收探針要帶 `--allow`。
  6. `AOI.Data` 和 AOI 頁（§13）共用同一個檔，owner gate 登記時兩個擁有者都要列。
* **golden 錯誤照翻清單**：這次翻譯 `fOCR_ReadFile`／`spbSaveClick` 沒有發現既有缺陷；J3
  （`iOCRPort` 只在缺鍵時補寫 24，畫面上改了不會存）待確認是不是刻意設計。
* **待 Steven 決定**：
  1. Q1（＝J1）：V912 `OCR.dfm` 缺 `rgOCRTriggerMode`——建議 A 請 Jimmy 在 V912 從 V899 補回
     DFM（已降級不急，記 `todo.md` G-006）。
  2. Q2：網頁 B 路（`recipe.doc.put`）直改 `AOI.Data` 之後記憶體不重讀、`CRouteOwner` 沒登記
     `aoi.data`——建議現在選 A 維持現狀，建 AOI／OCR 頁時改做 C（`CRouteOwner` 加 `aoi.data`
     擋掉 B 路直改）。
  3. Q3：存檔目前沒有觸發點——依 S73 裁決（建頁往後排）先不做頁面。
  4. 要不要做 OCR 頁面——建議與 S73／AOI 頁一起決定。
* **要等 Jimmy 的前提**：
  - J1（不急，`todo.md` G-006）：V912 `OCR.dfm` 補回 `rgOCRTriggerMode`（`OCR.h:81` 宣告、
    `OCR.cpp:2181`／`:2278` 使用，V899 `OCR.dfm:479-490` 有；量產機開 OCR 表單會 Access
    violation，存檔只存前 14 鍵）。已記進 `docs/handoff/FROM_STEVEN.md`（`23dd3492`）。
  - J2：`OCRInsp.cpp` 的替身類別 `W906OCR_TfOCRSeam`（`:347`／`:351`／`:400`）把
    `IsOCRCommandTrigger`／`CheckOCRWordType` 寫死，S86 讀進 `IniConfig` 的值目前那條 OCR
    流程還看不到，要等 OCR 執行期工作把替身拿掉才會生效。
  - J3：golden `iOCRPort` 只在缺鍵時補寫 24（`OCR.cpp:2212`），畫面上 `IPPort2` 改了不會存，
    請確認是不是刻意設計。
  - J4：OCR 相機、光源、COM、socket、模擬分頁都沒接。
  - 另請 Steven02 注意：`fMain->SendMSG_CMD` 目前是 no-op，GB 工作把 GPIB 接成真的通訊後，
    `INSTALL_OCR≠0` 的機台在開機、換配方、開 OCR 頁時都會送出 4 個指令（golden 行為）。
* **commit**：`973f2540`（讀檔／存檔本體＋開機／換配方讀檔鏈已接，均未 build／未 e2e）。

---

## 15. `TfContactForce`（S57，`Setup.ContactForce`）

* **建議頁面檔名**：`web/page/Setup.ContactForce.html`（**已存在**，目前是純靜態頁）。
* **golden 入口**：`iosetview.cpp` `Label120Click`（待確認精確行號）→ 開 `TfContactForce`；
  同一入口要接 `FileRW_ContactForce_ReadFile()`（golden `iosetview.cpp:3451` 之後，見要等
  Jimmy 項）。
* **golden 檔**：V912 的 `ContactForce.cpp`／`.dfm`／`.h`（類別 `TfContactForce`）。
* **移植入口**：`FileRW/ContactForce.cpp`／`.gen.inc`、`FileRW/ContactForce_Panels.h`（golden
  四個動態面板類別手寫轉錄）；`tools/editlist/ContactForce.py`；結構 `ContactForce`；前綴
  `CF`；commit `21d37f2b`。
* **PageDesc**：已登記（`FileRW_ContactForce_Boot()` 開機建替身；`FileRW_ContactForce_ReadFile()`
  供 golden `iosetview.cpp:3451` 之後接用）；`CRouteOwner` 已加 `contactinfo.ini`。
* **讀檔時機**：開機只建替身（DFM 設計期狀態＋容器父子），golden 建構子本體延到第一次
  `editlist.get ContactForce` 才跑（決策題①，建議先維持延後，等恢復 build、e2e 通過後再改
  照 golden 開機跑）；`ContactForceLoad.cpp` 的 `LoadContactForceTables()` 開機讀
  `dIndexZOffset`（P4，`[0..1][0..14]` 讀 `Gerneral.ini [Test Arm]`，`[2][*]` 是 golden 算的
  固定階梯不讀檔）＋ EP 3 鍵（golden 建構子 `:542-544`，原本漏了不看 `EP_Install`）。
* **存檔函式**：`CF_btSaveClick`（golden `:936-964`），目前沒有網頁呼叫者（
  `Setup.ContactForce.html` 是靜態頁，缺 JS）。
* **必送清單**：產生器掃出的 24 個＋30 個 `edtArm*Offset_*`＋面板 trackbar／offset 欄（這台
  約 250 個；面板筆數依 `EP_Install` 決定，這台 `INSTALL_DOUBLE_EP=1`：SLK 5、Ind 80、
  DieForce 5、OneByOne 0）。
* **要建的元件 id**：四個動態面板類別（`THTSLKClass`、`THTSLKIndClass`、
  `THTDieForceSLKClass`、`THTDieForceOneByOneSLKClass`）用 golden 自己的 `Name` 當具名替身
  （例如 `trckbrDiameter_30`、`edtContactOffset_30_5`），開頁時依 `extra.panels.*.items` 在
  四個 ScrollBox 裡建動態面板，元件 id 用 `items[].ids` 的值。頁籤只有 `title`、沒有 `id`，
  要用 `title` 去對應 `proxies` 的 `tabVisible`／`editable`。
* **頁面事件要 JS 做的**：存檔送完整必送清單（trackbar 送 `{position}`，edit 送 `{text}`）；
  小鍵盤範圍和 `Button1` 的算式寫在 `ContactForce.cpp` 檔尾；缺補件
  `web/page/ht9045_contactforce_c.js`，照 GroundMan 的做法寫；`ht9045_wire_engine.js` 的
  `GOLDEN_BRIDGE` 對應行要等 Jimmy 建頁時一起加（片段見 `HT9045_待插入片段.md` 第 5 項）。
* **多寫者**：`Gerneral.ini [System]` 的 `EP_MAXKPA`／`EP_MAXA`／`EP_MINMPA`／
  `EP_MINA_FeedBack` 四鍵與 HSys 的 C 路存檔（`HSys.gen.inc:3841-3844`）重疊，本頁採「沒動過
  的欄位不把舊值蓋回去」（`kCF_KeepNewerOverlap=true`）；**S90（commit `f1ad780c`）已裁決
  HSys 比照同一規則**（`kHS_KeepNewerOverlap=true`），並修正本頁 reload 會重拍快照的小漏洞
  （只在開頁與真的寫檔之後拍，不在 reload）；`ContactInfo.ini` 各 Diameter 段本頁是唯一寫者
  （B 路 `sysfile contactInfo` 只讀）；`SaveLastSetIni`（`config.ini` 含 `[Contact Force]`
  8 鍵、`LastSet.ini`、`configByRecipe.ini`）擁有者是 `IniConfig`，做法同 Temperature 頁前例。
* **會讀寫的真實檔**：`system\ContactInfo.ini`（golden 寫死路徑，`--dry` 擋不到；這台第一次
  開頁會補 64 段 Ind 預設鍵，golden 912 既有行為）、`system\Gerneral.ini`（走
  `asGeneralPath`，`--dry` 會轉到 scratch；這台 EP 12 鍵與 `[Test Arm]` 30 鍵原本齊全，開機
  不會改檔）、`config\config.ini`／`config\LastSet.ini`／`<配方>\configByRecipe.ini`（只在
  `SaveLastSetIni` 時寫）。
* **已知陷阱**：
  1. `WriteFile` 固定取 `SLKClass[0..3]`（`EP_Install==5` 時 `[0..7]`），口徑不足 golden 會當，
     移植樹整支不做、頁面存檔在寫任何檔之前就拒絕。
  2. `IndexZOffsetEdit` 在「沒有檔、建構子直接 `WriteFile`」時是 NULL（golden 在這裡 AV），
     這時 `[Test Arm]` 30 鍵不寫、回報 todo。
  3. `TTrackBar` `Min`／`Max` 是照推論的 VCL 守衛設（決策題⑤，待 Jimmy BCB6 核對）。
  4. 開本頁前一定要先跑過 golden 建構子（第一次 `editlist.get`），否則存檔會把 DFM 設計期值
     寫進檔案。
  5. KYEC 機台上，開機的 906 載入器不做 30→28 轉換（既有缺口），要開過本頁一次才會轉。
* **待 Steven 決定（4 題）**：①golden 建構子要不要開機就跑（建議 A 維持延後）；②Ind 表型號
  清單 906 vs 912（建議 B 底層改跟 `[SLK Type]`，和 V899、V912 出貨行為一致）；③必送清單
  嚴格度（建議 A 嚴格）；④`TTrackBar` `SetMin` 守衛（建議 A，請 Jimmy BCB6 核對）（原第③題
  「EP 四鍵舊值蓋回」已於 **S90**／commit `f1ad780c` 解決：同意 HSys 比照 ContactForce；
  原第⑥題「`_integrated.txt` 何時加」已解決：照 `c79ee4e9` 慣例現在就加）。
* **要等 Jimmy 的前提**：EP 硬體輸出（存檔／關頁／Exit 的 `ADAM_WriteVoltage`；`tb*mm_*kg` 的
  `ADAM_DirectWriteData`）；golden `FormClose` 效果（`fContact->ShowArmAndDeviceForce`、
  `DeviceForm.dPress`／`DeviceForm_File.dPress`，網頁沒有關頁事件）；`iosetview.cpp`
  `Label120Click` 接 `FileRW_ContactForce_ReadFile()`；`ht9045_wire_engine.js` 的
  `GOLDEN_BRIDGE` 片段；決策題⑤ BCB6 核對；KYEC 機台 30→28 轉換。已記進
  `docs/handoff/FROM_STEVEN.md`（`6837bb19`，標不急）。
* **commit**：`21d37f2b`（讀寫檔本體，未 build／未 e2e）。

---

## 16. Configuration 頁 Tray／HP 分頁（S98，`System\TrayForm.csv`／`PlateForm.csv`）

> **⛔ 20261001 更新：已做（St01，todo E-003 ①；RULINGS_20260926 S98＋S169，蓋過 Q41 S158）。** 下面的原文是 20260927 的狀態，
> 「沒有呼叫者」「沒有 WS 指令」「JSON 形狀待確認」「頁面事件待確認」都已過期。現況：
> * **頁面**：就在既有的 `D:\HT9045\web\page\Config.Configuration.html` 兩個分頁（沒有新頁）；St01 新檔 `D:\HT9045\web\page\ht9045_config_trayplate.js`（`:138` 同一行載入）
>   把 `strngrdTray`／`strngrdHP` 畫成真的格子、接上五顆鈕。開窗那一次引擎 editlist.get IniConfig 回來才讀（stage F）。
> * **C++**：WS `cfgtrayplate.op`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4826` 同一行）→ `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp` 檔尾
>   `W906_CfgTrayPlateOp`：get／select／modify（小鍵盤兩步）／add／delete／reload／save，golden `cConfiguration.cpp:6945-7217` 全部照翻（其餘六支處理器與 iSel*、edtTemp 補進 `TfConfigurationTrayPlate`）。
>   伺服器持表、頁面只送動作（不送整張表）；守衛與回覆格式見 skill `ht9045-html-json` `references/route-c-golden-bridge.md` §3.0n。
> * **寫檔**：只有 op=save（Tray 的 Save 寫 `D:\HT9045\System\TrayForm.csv`、HP 的 Save 寫 `D:\HT9045\System\PlateForm.csv`，整檔覆寫；開機、開頁都不寫）。
>   測試縫 `W906_TRAYFORMCSV_PATH`／`W906_PLATEFORMCSV_PATH`。
> * **ctest**：`S98_CfgTrayPlate`（C++）、`S98_TrayPlatePage`（node）。
> * **新的 R 題候選（照 golden 留著，Steven 可推翻）**：權限不夠（pnlTray 停用）還是能點格＋連點兩下改畫面上的格子（存不了檔）；Load Data 之後 iSel 停在表外時
>   按 Delete 刪的是最後一列、不是反白那一列；Modify Data 的文字欄收的字＝golden 鍵盤打得出來的（英數字、空白、- _ = + [ ] { } ( ) .），非 ASCII 不收。

* **建議頁面檔名**：不用新頁——在既有 `web/page/Config.Configuration.html` 的 `tsTrayData`／`tsHPData` 分頁接上（建議，未定案）。
  那兩個分頁與 `strngrdTray`／`strngrdHP` 目前只是 DFM 產生的靜態外殼：頁面有這兩個 id，但 `editlist.get IniConfig` 不帶表格內容
  （`FileRW/IniConfig.gen.inc` 沒有 `strngrdTray`／`strngrdHP` 替身，20260927 grep）。
* **golden 檔**：`cConfiguration.cpp`（V912）`TfConfiguration::sbUpdateTrayClick :7012`、`sbtReloadTrayClick :7037`、`sbtReloadHPClick :7148`、
  `sbUpdateHPClick :7194`；建構子尾段 `:226-234`；兩個旗標在 `TfMain::FormShow main.cpp:9987-9988`。
* **移植入口**：`FileRW/CfgTrayPlate.h`／`.cpp`（類別 `TfConfigurationTrayPlate`，單例 `W906_CfgTrayPlate()`）；手寫、不走產生器、
  不在 `_integrated.txt`（`CMakeLists.txt:3404`）；commit `217e7e5e`。
* **PageDesc 有沒有登記**：沒有（它不是 C 路結構）。兩顆 Load 鈕登記成 `"TfConfiguration"` 具名替身（`W906_CfgTrayPlate_CreateForm()`，
  `tools/wb_serve.cpp:4052`），所以 C 路 `IniConfig` 開頁（golden `FormShow :4676-4684`，`FileRW/IniConfig.gen.inc:8419`／`:8424`）會照 golden
  重讀兩張表——只是沒有畫面接收。
* **讀檔時機**：開機不讀（golden 同：`CreateForm` 那一刻 `bHasTrayCSV`／`bHasPlateCSV` 還是 false，`tools/wb_serve.cpp:4066` 才設）；
  開 Configuration／TrayForm／Cleaning／HotPlate 四頁時讀（`FileRW/CfgTrayPlate.cpp` 檔頭 ③）。
* **存檔函式與 golden 觸發鈕**：兩個分頁各自的 Save（`sbUpdateTray`／`sbUpdateHP`）→ `TfConfigurationTrayPlate::sbUpdateTrayClick`／
  `sbUpdateHPClick`（整檔覆寫 CSV）；目前沒有呼叫者，也沒有 WS 指令。
* **必送欄位**：整張格子（`Cells[c][r]`、`RowCount`／`ColCount`）；JSON 形狀待確認（還沒設計）。
* **要建的元件 id**：`strngrdTray`、`strngrdHP`、`sbUpdateTray`、`sbtReloadTray`、`sbUpdateHP`、`sbtReloadHP`（golden `cConfiguration.h:1040-1054`，
  `FileRW/CfgTrayPlate.h` 註解）。
* **頁面事件要 JS 做的**：格子編輯、Load（重讀）、Save（送整張表）；待確認。
* **已知陷阱**（照 golden 留著，`FileRW/CfgTrayPlate.cpp` 檔頭 (a)～(c)）：
  1. HP 分頁 Save 完呼叫的是 **Tray** 表的重讀、清的是 Tray 那顆的 `Down`（主 repo V912 `cConfiguration.cpp:7215-7216`；`CfgTrayPlate.cpp`
     檔頭與 todo R33 寫 `:7212-7213`，20260927 `sed -n '7194,7217p'` 實測差 3 行）——HP 畫面不會重讀（todo ★ R33）。
  2. reload 只收「後面有逗號」的欄位；Save 用 `CommaText` 寫、最後一欄後面沒有逗號 ⇒ 存一次再讀，第 16 欄（表頭 BlockPitchY）會掉（R34）。
  3. reload 先拿掉所有引號再切逗號：欄位本身含逗號時會切錯。
  4. 空欄位照 BCB6 `CommaText` 不加引號（`W906_Bcb6CommaText`，`FileRW/CfgTrayPlate.cpp:83`；vclcompat 會寫成 `""`）；依據是 Delphi 6/7
     原始碼，沒有在 BCB6 實機量過（R35）。
  5. 移植樹 `cConfiguration.cpp:350`／`:468` 另有一份門面翻譯、沒有實例——不要接（會變成兩張表；`generators.md` 廿六）。
* **待 Steven 決定**：要不要現在做這兩個分頁（todo ★ Q41）；R33～R35 可推翻；「從資料庫選一筆就自動填欄位」三頁共用的事件入口（Q40）。
* **要等 Jimmy 的前提**：Q40 若選 A 要動 `web/page/ht9045_wire_engine.js`（Jimmy 的檔；Jimmy 20260927 00:2x 的意見記在 todo Q40）。
* **commit**：`217e7e5e`（本體＋TrayForm／Cleaning 讀者解閘）、`7d490f7c`（HotPlate 讀者）。

---

## 17. `TACTForm`（S108，自動 K 溫設定，`System\AutoTemperature.ini`）

* **建議頁面檔名**：`Setup.AutoTemperature.html`（建議，未定案）。
* **golden 檔**：`AutoTemperature.cpp`／`.h`（V912），表單 `TACTForm`；主畫面 `sbAutoTempClick`（`main.cpp:29883-29890`，`Insufficient(49)` →
  `ACTForm->Show()`；`FileRW/ACTForm.cpp` 檔頭：`CosFunction.bAutoKTemp` 才顯示這顆鈕）。
* **移植入口**：`FileRW/ACTForm.cpp`／`.gen.inc`；`tools/editlist/ACTForm.py`；前綴 `ACT`；結構 `ACTForm`；狀態全是本 TU 的 static
  （`ACTData`、`FilePath`…，golden 表單外沒有讀者）；commit `217e7e5e`。
* **PageDesc 有沒有登記**：沒有。
* **讀檔時機**：開機建構子（`FileRW_ACTForm_Boot`，`tools/wb_serve.cpp:4062`）照 golden 讀空檔名 ⇒ 不讀不寫、全是預設值（todo ★ R37）；
  開頁 `FileRW_ACTForm_FormShow`（golden `FormShow :419`）才真的讀 `[ACT]`／`[Display]`／`[COMPort]`，目前沒有呼叫者；換配方不讀。
* **存檔函式與 golden 觸發鈕**：`btnUpdateClick`（`:692`）→ `FileRW_ACTForm_btnUpdateClick`：`SaveACTData`（`:834` 起）＋`SaveCommData`；
  `palSaveLogClick`（`:1355`）→ `FileRW_ACTForm_palSaveLogClick`（`MemoOffset` → `D:\HT9045_Log\AutoTempCalibration\<時間>.txt`）。都沒有呼叫者。
* **必送欄位**：`kACT_SaveReads` 16 個——`cbBaudRate`、`cbByteSize`、`cbDevice`、`cbParity`、`cbStopBit`、`cbbThermoCtrlType`、`edColumns`、
  `edSingleOffset`、`edtCalibrationRange`、`edtCheckIntervalTime`、`edtOffsetLimit`、`edtRange1`～`edtRange3`、`edtReadScanTime`、`rgOffsetMethod`
  （`FileRW/ACTForm.gen.inc`）。
* **要建的元件 id**：上面 16 個＋`MemoOffset`；量測點面板 `myATPal`（GUI）沒有翻。完整清單待確認。
* **頁面事件要 JS 做的**：待確認（`FileRW/ACTForm.cpp` 檔頭「不在本檔」列的按鈕多半牽涉機台動作）。
* **已知陷阱**：
  1. 開機讀空檔名是 golden 的時序（`:297` 在 `:398` 之前），照 golden 留著；開頁前 `ACTData` 是預設值（例 `iThermoCtrlType=1`＝DeltaDTB4824），
     即使檔裡寫的是 0（todo R37 的例子）。
  2. `palSaveLogClick` golden 建的資料夾是 `OffsetPath`、不是 `AutoTempCalibration`（`:1358`）；那個資料夾不在時 golden `SaveToFile` 丟例外，
     vclcompat 不寫也不報（`FileRW/ACTForm.cpp` 註解）。
  3. `TimerACTTimer`（`:887`）會改 `Temperature.fTempOffSet`（溫控參數）——沒翻，SAFETY 相關。
* **待 Steven 決定**：要不要現在建頁（todo ★ Q41）；R37 可推翻。
* **要等 Jimmy 的前提**：`OpenCommPort`／`CloseCommPort`／`SendCommand`／`ACTComReceiveData`、`TimerACTTimer` 自動 K 溫、`btnAutoStartClick`、
  TCP 那組、`DTB4824_ReadPV`、`FormClose`（`FileRW/ACTForm.cpp` 檔頭「不在本檔」）。
* **commit**：`217e7e5e`（未 build）。

---

## 18. `TfWinway`（S109，WinWay 4 站 ATC 通訊設定，`config\ATCWinWay.ini`）

* **建議頁面檔名**：`HW.ATCWinway.html`（建議，未定案）。
* **golden 檔**：`ATC/WinWaySetting.cpp`／`.h`（V912）；主畫面 `sbATCClick`（`main.cpp:29598`）在 `ATC_SYSTEM==eWinWay` 時 `fWinway->ShowModal()`
  （`:29607-29609`）。
* **移植入口**：`FileRW/Winway.cpp`／`.gen.inc`；`tools/editlist/Winway.py`；前綴 `WW`；`arrATC_Site[4]`／`iWinwayATCIndex` 用 `#define`
  接到門面 `forms/fWinway.h` 的 `fWinway`（`Winway.py:108-109`）；`edtSetTemp`／`cbbWinwayATCIndex` 收養門面元件（`WW_AdoptPortWidgets`）；commit `217e7e5e`。
* **PageDesc 有沒有登記**：沒有。
* **讀檔時機**：**每次開機**（`FileRW_Winway_Boot`，`tools/wb_serve.cpp:4064`；golden `CreateForm` `HT9045.cpp:272` → 建構子 `:12`）讀
  `[COMPort1..4]` 並原樣寫回；檔不在就用預設值建出來；不看 `ATC_SYSTEM`（todo ★ R36）。開頁（`FormShow :34`／`ShowCommData :43`）與切換站別
  （`cbbWinwayATCIndexChange :150`）只把記憶體放到元件、不讀檔。換配方不讀。
* **存檔函式與 golden 觸發鈕**：`btnUpdateClick`（`:78`）→ `FileRW_Winway_btnUpdateClick`：寫目前那一站；結尾重開 COM（`:111-112`）記 `ELTodo`。
  沒有呼叫者。
* **必送欄位**：`kWW_SaveReads` 5 個——`cbbBaudRate`、`cbbByteSize`、`cbbDevice`、`cbbParity`、`cbbStopBit`（`FileRW/Winway.gen.inc`）。
* **要建的元件 id**：上面 5 個＋`edtSetTemp`、`cbbWinwayATCIndex`；`lblShowPT`（`GetPT()`，連線時才有值）。完整清單待確認。
* **頁面事件要 JS 做的**：換站別要先把 `cbbWinwayATCIndex` 的值套到替身，再跑 `FileRW_Winway_cbbWinwayATCIndexChange`（目前沒有 WS 指令）。
* **已知陷阱**：
  1. 開機一定會寫檔（golden 同）；這台 `D:\HT9045\config\ATCWinWay.ini` 在版控裡、內容就是預設值，寫回的位元組不變（`FileRW/Winway.cpp` 檔頭）；
     新機台第一次開機會建出這個檔。
  2. 門面 `forms/fWinway.cpp` 在 static init 把 `arrATC_Site` 設成 NULL，開機（`FileRW_Winway_Boot`）之後才填上 4 個物件——門面檔頭「permanently
     NULL」從 S109 起只到開機為止。golden 讀者（`bthermo.cpp:4609` 起、`forms/fLotInfo.cpp:4272-4313`）目前都在 `#if 0`。
  3. `.gen.inc` 之後 `arrATC_Site`／`iWinwayATCIndex` 是巨集，手寫碼直接寫名字（`generators.md` 廿五）。
* **待 Steven 決定**：要不要現在建頁（todo ★ Q41）；R36 可推翻。
* **要等 Jimmy 的前提**：`OpenCommPort`／`CloseCommPort`、`GetPT`、`SetST`、`btnSendTempClick`／`SetTemprature*`（Modbus 寫設定溫度）、
  `btnGetPVClick`、`WinWayATCComm1ReceiveData`（`FileRW/Winway.cpp` 檔頭「不在本檔」）。
* **commit**：`217e7e5e`（未 build）。

---

## 19. `TfMonitor`（S110，錄影監控，`system\MVData.ini`）

* **建議頁面檔名**：`HW.MonitorView.html`（建議，未定案）。
* **golden 檔**：`Monitor/MonitorInterface.cpp`／`.h`（V912）；主畫面 `sbMonitorViewClick`（`main.cpp:33471`，`Insufficient(127)` → `fMonitor->Show()`；
  `IniConfig.bC11UseMonitorView` 才顯示這顆鈕，`FileRW/Monitor.cpp` 檔頭）。
* **移植入口**：`FileRW/Monitor.cpp`／`.gen.inc`；`tools/editlist/Monitor.py`；前綴 `MN`；7 個元件收養門面 `forms/fMonitor.h`（`MN_AdoptPortWidgets`）；
  `sADDRESS`／`iPORT` 用 `#define` 接門面（`Monitor.py:93-94`）；commit `217e7e5e`。
* **PageDesc 有沒有登記**：沒有。
* **讀檔時機**：開機（`FileRW_Monitor_Boot`，`tools/wb_serve.cpp:4064`；golden `CreateForm` `HT9045.cpp:247` → 建構子 `:32` `LoadTCPIPParament`（`:79`））
  只讀 `[Setup]` IP／Port、`[Specific]` 5 鍵，檔不在＝預設值、不建檔；開頁 `FormShow`（`:47`）再讀一次（`FileRW_Monitor_FormShow`，沒有呼叫者）；
  換配方不讀。
* **存檔函式與 golden 觸發鈕**：`sbMVUpdateClick`（`:123`）→ `SaveTCPIPParament`（`:98`，7 鍵，每一鍵立即落地，檔不在會建）→
  `FileRW_Monitor_sbMVUpdateClick`；沒有呼叫者。
* **必送欄位**：`kMN_SaveReads` 7 個——`cbAfterHandlerTrayFeedMonitor1ClosedVideo`、`cbWhenHDFullAlarm`、`edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime`、
  `edLowHDSpace`、`edMVAddress`、`edMVPort`、`edWhenHDFullPrompt`（`FileRW/Monitor.gen.inc`）。
* **要建的元件 id**：上面 7 個；錄影鈕、Connect／Disconnect 待確認。
* **頁面事件要 JS 做的**：待確認。
* **已知陷阱**：
  1. golden 每台機台開機都讀（不看 `bC11UseMonitorView`），照做。
  2. `.gen.inc` 之後 `sADDRESS`／`iPORT` 是巨集（`generators.md` 廿五）。
  3. golden `MonitorTimerTimer`（`:143`，HD 容量警報、錄影關閉等待）讀的就是這 7 個元件——之後翻它時讀到的是本檔讀進來的值。
* **待 Steven 決定**：要不要現在建頁（todo ★ Q41）。
* **要等 Jimmy 的前提**：MVCtrl（`MonitorTCPIP`，建構子 `:30-31`／`:33`；Load／Save 結尾 `MVCtrl->InitialSocket`，`:94`／`:119`）、
  Connect／Disconnect、`MonitorTimerTimer`、錄影開關、`GetMonitorHDSpec`（`FileRW/Monitor.cpp` 檔頭「不在本檔」）。
* **commit**：`217e7e5e`（未 build）。

---
