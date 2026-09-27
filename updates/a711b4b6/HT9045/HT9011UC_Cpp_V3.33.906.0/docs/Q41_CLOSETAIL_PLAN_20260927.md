# 關窗尾段 9 頁：查證與實作方案（Q41 盤點第一節第 3 項）

- 讀者：ST01-E（整理後轉 Steven）；第七節是給 Steven 看的決策題，寫成不必看程式也讀得懂。
- 性質：唯讀查證＋方案，這一趟沒改任何程式、沒 build、沒跑 wb_serve。
- 基準：D:\HT9045 分支 v906/steven-cbridge-review6，HEAD `99ec7b7b`（2026-09-27 下午）。開工時是 `1433ed1b`，中途進來 Q40 的 `76058840`（S157 form.event C++ 分派）、Q41 的 `a8eca460`（YM-1＋CC-L2～L4）、`834fcc78`（S126 第一型 /api/form 退役）等；**下面所有移植樹行號都用 `git show 99ec7b7b:<路徑>` 重量過**。
- 三棵樹：
  - 移植樹＝D:\HT9045\HT9011UC_Cpp_V3.33.906.0（下文「移植樹」路徑都在這裡）
  - golden V912＝D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950 讀，唯讀）
  - 網頁＝D:\HT9045\web\page
- 盤點原表：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md（LU-2、TF-5、HP-3、TA-6、SP-6、YM-5、CT-4、BS-2、CC-E15）

---

## 零、結論（先看這段）

1. **S88 Q1 已經結案**：Steven 20260926 17:5x 的 S107-1 裁決「溫度頁尾段維持存檔後就跑，不改成等 Exit」。盤點表寫「仍是 S88 的待決 Q1」是過期說法（出處見第一節）。S107-1 字面上只講溫度頁；SetUp 頁（S100）已沿用同一做法。9 頁照同一做法，建議當成 S107-1 的延伸，列 R 題給 Steven 點頭（第七節 R-新2），不必另開 Q。
2. **9 頁的尾段函式在移植樹全部有本體，而且都是活的**（DoStructUnitConvert、SetWorkParameter、ShowBinSel、SortCT UpForm、GetHotPlateYHalfPos、LoadAutoCleanData、SetStartModeData、UpdateMainOperateMode、YieldMonitoring ReadFile、SCKART AccessFile）。只有三支是空殼或純畫面：主畫面 Tray／IC 方向圖 `ShowTrayDeviceDir`（移植樹沒有，網頁主畫面也沒有這張圖）、`ShowTestHeadComp`（空殼，golden 本體歸 Jimmy，S88）、主畫面 `sbQAMode->Visible`（AMKOR_Korea 客戶專屬，S25 跳過）。**不需要先翻任何新函式**。
3. **最自然的掛法＝照現成兩頁**：尾段本體放 移植樹 FileRW\MainClick.cpp 檔尾（跟 SetUp 的 `W906_Main_sbSetupClickTail` 同一處），各頁手寫的入口 cpp 把 PageDesc 的 saveFlow 換成一個包一層的 SaveFlow：golden 存檔鈕跑完，`closed`（A02 關窗）或 savedMark（真的寫了檔）就跑尾段。C 路共用層 FileRW\_EditPage.cpp **沒有**「存檔成功後」的共用掛點，也不建議加（理由第四節）。**7 頁不用改描述檔、不用重產**；只有 HotPlate（A 形狀、產生檔不可手改）要改 tools\formbridge\TfHotPlate.py＋tools\gen_formbridge.py 並重產。
4. **兩頁的時機跟盤點表寫的不一樣**：
   - **Contact（CT-4）不是關窗尾段**：golden V912 main.cpp:28314 `fContact->Show()`（Jimmychiu 20240731 把 ShowModal 改成 Show，V899 同），所以 :28315-28316 的換算＋重載參數在**打開 Contact 視窗的當下**就跑完，不等關窗。照 golden 要掛在開頁（editlist.get）。網頁存完一定自動重新開頁（引擎「寫完一定重讀」規則），所以實際效果是存完也會跑一次（第七節 R-新1）。
   - **Configuration（CC-E15）**：網頁的「存檔」就是 golden 的 FormClose（FileRW\IniConfig.cpp:346），golden 關窗不論存不存都跑尾段，所以掛在 FormClose 之後、不看有沒有存成，**跟 golden 完全一樣**，沒有「沒存就關」的差異。它還要在開頁時先記下 6 個設定的舊值（golden :28615-28622），比對之後才決定要不要重讀 Auto Clean、重建起動模式清單。
5. **一個安全缺口**：HotPlate 頁存檔走 WS `form.save`，**運轉中沒有擋**（移植樹 tools\wb_serve.cpp:5340 起沒有 SystemStart 檢查；RULINGS_20260927 第 7 條只擋了 editlist.save，當時註明 form.save「另列給 Steven 決定」，但還沒列題）。HotPlate 尾段會重算入料臂放 HotPlate 的位置，運轉中跑等於途中換參數。建議每支尾段開頭都再查一次「機台停著」（golden 運轉中根本打不開這些視窗，等於照 golden 的前提），form.save 本身要不要擋列 Q 題（第七節 Q-新1）。
6. **能不能現在做**：現在就能做 5 頁＋共用本體（LU-2、SP-6、TA-6、BS-2、CT-4；檔案沒人登記）。TF-5、HP-3 碰到 Q40 剛推的檔（`76058840`，工作樹已乾淨），照「看區段不看檔名」規則其實不同段，但建議先跟 Q40 同事確認沒有後續 C++ 再動。YM-5、CC-E15 等 Q41 後續同事（Yield／Configuration C 路）交件。

---

## 一、S88 與它的 Q1：已結案

| 項目 | 內容 | 出處 |
|---|---|---|
| S88 是什麼 | `SaveTestMode`（TestMode.Data）golden 16 個呼叫點逐一對照；這次補上溫度頁尾段 `MainTempOffsetTail`（golden `TfMain::sbTempOffsetClick` 尾段），溫度頁存檔後跑。commit `295bc768` | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:239（S88 列） |
| S88 的 Q1 | 「尾段在『存檔後』跑（A，目前）或等網頁 Exit 送伺服器（B）」 | 同上 :239 |
| 裁決 | **S107-1**（Steven 20260926 17:5x，經 github-02 轉達）：「照目前做法『存檔後就啟用』，不改成等 Exit」→ 維持 `295bc768` 的 A；**S88 Q1 結案** | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:284（S107-1 列） |
| 決策檔 | 「S88 Q1（溫度頁尾段何時跑）——已由 S107-1 裁決：維持現行『存檔後就啟用』，S88 Q1 結案。」 | D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md:918-919 |
| 進度表 | S88 ✅ 已完成；S107 ✅ 生效中（「S88 Q1 隨 S107-1 結案」） | D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md:119、:138 |
| decisions-pending | 沒有 S88／Q1 相關的待決題（20260927 下午 grep「尾段」「關窗」「SetWorkParameter」「DoStructUnitConvert」皆無） | D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md |
| 相關裁決 | RULINGS_20260927 第 2 條第 7 題 A：editlist.save 一律擋 `SystemStart\|\|SoftStart`（form.save、recipe.doc.put、system.file.put 另列給 Steven） | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md:22；落實 移植樹 tools\wb_serve.cpp:5303 |
| 客戶專屬 | S25「先暫時跳過，註記就好」 | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260925.md:153 |

**還寫著「待 Steven」的過期地方**（實作那顆 commit 順手改註解，同一行改寫）：
- 移植樹 FileRW\Temperature.cpp:64（「觸發時機是待 Steven 決定的題目」）、:206（「觸發時機待 Steven 決定」）
- 移植樹 FileRW\TestIF_File_SetUp.cpp:189（「觸發時機待 Steven（交件報告 S100）」）
- 移植樹 FileRW\MainClick.cpp:321（「時機待 Steven」）
- 移植樹 docs\Q41_INVENTORY_20260927.md:32、:65、:328
- D:\HT9045\.claude\skills\ht9045-json-bridge\references\generators.md:594（十九節「待 Steven 決定 Q1」）

---

## 二、現成做法：Temp_Set、SetUp（逐行對照 golden V912 main.cpp）

### 2.1 共同的掛法

C 路存檔流程（移植樹 FileRW\_EditPage.cpp `PageSave` :84 起）：必送檢查 → beforeApply（可選）→ 丟掉不可改的 → 套值 → **:169 `d.saveFlow()`** → :170 `saved = ELMarked(d.savedMark)` → :172 沒寫檔就 `d.reload()` → :173 `closed` 就把「開過頁」標記清掉。

尾段不是 PageSave 呼叫的，是**每頁自己的 saveFlow 函式在 golden 存檔鈕跑完之後呼叫**：
- `ELMarked("closed")`：golden 存檔鈕開頭的 A02 權限守衛 `Close()`（產生器把 `Close()` 換成 `filerw::ELMark("closed")`，例 移植樹 FileRW\Ld_UldDelayTime.gen.inc:304）→ golden 是 modal 視窗關掉 → ShowModal 回傳 → 尾段。**與 golden 相同**。
- `ELMarked(savedMark)`（通常 `"SaveSetupFile"`）：真的寫了檔。golden 存完視窗還開著，要等 Exit 才跑尾段；網頁 Exit 不送伺服器 → 存完就跑（S107-1）。
- 唯一差異：golden「開頁、沒存就關」也會跑尾段，網頁不跑。

### 2.2 Temp_Set：移植樹 FileRW\Temperature.cpp

呼叫點：`SaveFlow()` :167-214；:203 `TS_spbSaveClick()`（golden 存檔鈕）之後
- :207-209 `if (ELMarked("closed")) { TS_FormClose(); MainTempOffsetTail(); }`（先跑 golden FormClose uTemp_Set.cpp:4196 再跑尾段，順序同 golden）
- :210-211 `else if (ELMarked("SaveSetupFile")) MainTempOffsetTail();`

本體 `MainTempOffsetTail()` :70-135 ↔ golden V912 main.cpp `TfMain::sbTempOffsetClick` :28347-28402（:28351 NewRecordProcess MES21109、:28352 `fTemp_Set->ShowModal()`，尾段 :28353-28401）。⚠ Temperature.cpp 註解引的是已退場的 D:\HT9045_ref 行號（:28351-28399），比現在這份 V912 少 2（generators.md 二十節已記）。

| 移植樹 Temperature.cpp | golden V912 main.cpp | 內容 | 狀態 |
|---|---|---|---|
| :74-86 | :28353-28365 | iMachineTempMode 0／3 → LastSet.iTemperature＝Hot／AmbientHot；bLastSetInSetUpFile 時 SaveTestMode | 照翻 |
| :87-96 | :28366-28375 | iMachineTempMode 1 → Ambient；SaveTestMode | 照翻 |
| :98 | :28377 | `fTemp_Set->ReadTempFile(true)` → `TS_ReadTempFile(true)` | 照翻（本 TU 的 golden 本體） |
| :99-108 | :28378-28384 | Hot 時 fHeaterOK=false、bHeatOKBellowError=false、iThermoTask=1（重啟溫控任務） | `#if 0 GATE(W906-FRW-S88-THERMO)`＋ELTodo，交 Jimmy |
| :113-120 | :28386-28393 | NewRecordProcess MES2153／2151／2152／2154 | 照翻；:110-112 註明 golden 代碼看起來對調了，不順手修 |
| :122 | :28395 | UpdateMainOperateMode | 照呼叫 `fMain->UpdateMainOperateMode()`。⚠ :122 行尾註解「移植樹門面計數 stub」已過期：wb_serve 開機 tools\wb_serve.cpp:4065 裝了真本體（forms\fMain_OperateMode.cpp:139），會切加熱器繼電器、送 ATC7、寫 lastdata.dat（forms\fMain.cpp:507 註解） |
| :123 | :28396 | DoStructUnitConvert | 照翻 |
| :124 | :28397 | SetWorkParameter | 照翻 |
| :125-129 | :28398 | fAutomation->AmkorSendMessage(1) | `#if 0 GATE(W906-FRW-S88-AMKOR)`；只有 bAmkorFunction／CC_QUALCOMM 記 ELTodo |
| :131 | :28400 | ShowTestHeadComp(false) | 呼叫移植樹空殼 forms\fMain.cpp:247 |
| :132-134 | :28401 | ATKRecipeInfo->SaveFile() | 加 NULL 保護（移植樹沒建 ATKRecipeInfo，database.cpp:149） |

### 2.3 SetUp：移植樹 FileRW\TestIF_File_SetUp.cpp ＋ FileRW\MainClick.cpp

呼叫點：TestIF_File_SetUp.cpp `SaveFlow()` :179-192；:182 `SU_sbUpdateClick()` 之後 :190-191 `if (ELMarked("closed") || ELMarked("SaveSetupFile")) W906_Main_sbSetupClickTail();`。宣告在 :39（全域範圍）。

本體 MainClick.cpp `W906_Main_sbSetupClickTail()` :335-369 ↔ golden V912 main.cpp `TfMain::sbSetupClick` :28457-28498（:28461-28468 ASE K15 密碼＝客戶專屬、:28469 NewRecordProcess MES2177、:28471 `fSetup->ShowModal()`，尾段 :28472-28497）：

| 移植樹 MainClick.cpp | golden V912 main.cpp | 內容 | 狀態 |
|---|---|---|---|
| :337 | :28472 | DoStructUnitConvert | 照翻 |
| :339 | :28474 | fTestCategory->AdjFormData | 照呼叫（移植樹門面，純版面） |
| :340 | :28475 | fContactCT->ShowFormComp | 同上 |
| :341 | :28476 | SetWorkParameter | 照翻 |
| :342-346 | :28477 | fAutomation->AmkorSendMessage(0) | GATE＋條件式 ELTodo（同 S88） |
| :347 | :28478 | UpdateMainOperateMode | 照呼叫（真本體，同上） |
| :350 | :28481 | ShowTestHeadComp(false) | 空殼 |
| :351-355 | :28482-28486 | bA09_ByArmCloseSite 時 ShowTestCategory(0)／(1) | 照翻 |
| :357-368 | :28488-28497 | _8Site1X4 時寫配方 Contact.Data 4 鍵再重讀 Contact | 照翻（golden 怪行為：Drop 選到 bool 多載寫成 1，:322-328 已註明） |

### 2.4 另外兩個已做的（不在 9 頁內，給對照）
- Cleaning：移植樹 FileRW\TestIF_File_Cleaning.cpp:188-205，golden V912 main.cpp:29674-29675（`fCleaning->ShowModal(); DoStructUnitConvert();`）。**用的是 S88 Q1 的 B 做法**：頁面在 editlist.save 的 widgets 多帶 `W906_clButton {"text":"sbCleanSave"|"sbCleanExit"}`，Exit 才跑尾段（:197-200）；A02 關窗 :193-196 先 FormClose 再尾段。
- DIO：移植樹 FileRW\TTLCfg.cpp:273-283（golden V912 main.cpp:28669-28674 sbDioSetClick 尾段）。

---

## 三、9 頁逐頁表

golden 行號都是 golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp（另註明者除外）。移植樹入口 cpp 在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\。「頭段」＝ShowModal 之前（NewRecordProcess「Enter …」、等級檢查），不在本項。

| 盤點編號／頁 | golden 尾段（ShowModal 之後） | 移植樹本體狀態 | 會寫哪些檔 | 掛點（移植樹） | 風險 | 現在能做嗎 |
|---|---|---|---|---|---|---|
| **LU-2** Setup.Ld_ULd | `sbLdUldClick` :28448-28455；:28453 ShowModal；:28454 DoStructUnitConvert（只有這一行） | 活（cUnitConvert.cpp:675） | 不寫 | Ld_UldDelayTime.cpp:32 saveFlow＝`&LU_spbSaveClick` 換成包一層；closed（gen.inc:304）或 SaveSetupFile（:331） | 最低。Ld_UldDelayTime 本身不在換算範圍（DoStructUnitConvert 只換 TestIF／Device／HotPlate／ArmOffset／ArmSpeed／DefForm），效果只是把其他結構重換一次，跟 golden 一樣 | ✅ 能 |
| **TF-5** Setup.TrayForm（TfTrayForm） | `sbTrayFormClick` :28404-28414；:28409 ShowModal；:28411 DoStructUnitConvert、:28412 SetWorkParameter、:28413 fShowMessage->ShowTrayDeviceDir() | 前兩支活；ShowTrayDeviceDir 移植樹沒有（golden uShowMessage.cpp:183，純主畫面方向圖；網頁 main.html 沒有這張圖、也沒有 tag 送 UserDefForm[0].iTrayDirection） | SetWorkParameter 以讀為主（見 3a） | UserDefForm_File.cpp:34 `&TF_spbSaveClick` 換掉；closed（gen.inc:753）或 SaveSetupFile（:785；Q40 推進後的行號） | SetWorkParameter 會重算 Prod 位置（停機時才跑，golden 同） | ⏳ 檔案是 Q40 剛改的（`76058840` 加了 :37-40 事件表）；本項改 :29／:34 與檔尾，不同段。先跟 Q40 同事確認沒有後續 C++ |
| **HP-3** Setup.HotPlate（A 形狀） | `sbPlateFormClick` :28416-28426；:28421 ShowModal；:28422 DoStructUnitConvert、:28423 SetWorkParameter、:28424 fCleaning->LoadAutoCleanData()、:28425 GetHotPlateYHalfPos() | 全部活：LoadAutoCleanData＝TestIF_File_Cleaning.cpp:399 `FileRW_Cleaning_LoadAutoCleanData()`；GetHotPlateYHalfPos＝ainarm_SearchPlacePlate.cpp:132（InitialOK==false 時直接 return） | **LoadAutoCleanData 會寫**：<配方>\HandlerCondition.Data 的 iIndexArmAutoCleanCnt 每次都寫，另有條件式寫入（TestIF_File_Cleaning.cpp:19-21） | 產生檔 HotPlateForm_File.cpp:455-458 `SaveFlow(FormState& J)`，**不可手改**；要在 tools\formbridge\TfHotPlate.py 加設定、tools\gen_formbridge.py:330 支援、重產。條件：`J.M("closed")`（:231）或 `J.M("saved")`（:266） | **form.save 運轉中沒擋**（第零節第 5 點）；尾段改入料臂放 HotPlate 的路徑參數（iYHalf、b4x11HP…），必須在尾段開頭自己查機台停著；另列 Q-新1 | ⏳ 等 Q40 同事確認（這三個檔他剛改過）；Q-新1 可並行問 |
| **TA-6** Setup.TrayAssignment（檔名 TrayForm.cpp，⚠ 不是 Q40 的 TrayForm 頁） | `sbTrayAssignClick` :28428-28438；:28433 ShowModal；:28434 DoStructUnitConvert、:28435 SetWorkParameter、:28436 fShowBinSelect->ShowBinSel()、:28437 fSortCT->UpForm() | 全部活：ShowBinSel＝cShowBinSelect.cpp:1187；UpForm＝forms\fSortCT.cpp:488 | 不寫（ShowBinSel／UpForm 都只改記憶體） | TrayForm.cpp:259 `&TA_spbSaveClick` 換掉；closed（gen.inc:1994）或 SaveSetupFile（:2071） | **ShowBinSel 不是純畫面**：它重算 iBinTray／bUnloadHasBin／iTrayLastBin／BinAssign，出料分 bin 在用（移植樹 acatchtray.cpp:8698、csystem.cpp:14074 讀 iBinTray）。改了 Tray 用途卻沒跑，停機期間的狀態就是舊的（START 時 golden 也不會重跑 ShowBinSel，這條比其他頁重要） | ✅ 能 |
| **SP-6** Setup.Speed | `sbSpeedClick` :28677-28700；:28695 ShowModal；:28696 DoStructUnitConvert；:28698-28699 bEnable_SECS_GEM 時 EventReport(SECS_EVENT.EnterSpeed) | 活；EventReport（SECSGEM\SecsEventReport.cpp:15）目前只是模擬計數，不會送 MES | 不寫 | ArmSpeed_File.cpp:28 `&SP_spbSaveClick` 換掉；golden Speed 存檔鈕沒有 A02 守衛、沒有 Close()，savedMark `"spbSaveClick"` 一進來就成立 → 每次存檔都跑 | 盤點表說 EnterSpeed「進頁時送」不對：程式順序是 ShowModal 回來之後才送（關窗時）。網頁每存一次就送一次（golden 一次開關送一次）；目前 EventReport 只是計數，等 SECS 真的接上再看 | ✅ 能 |
| **YM-5** Setup.YieldMonitoring | `sbYieldClick` :28500-28513；:28509 ShowModal；:28510 DoStructUnitConvert、:28511 SetWorkParameter、:28512 SetStartModeData() | 全部活：SetStartModeData＝RunStartMode.cpp:1298（W906-SSMD 20260927） | **SetStartModeData 會寫**：它最後呼叫 SetRunStartMode → :883 fMain->SaveRunMode（RunMode.txt，本體有裝才寫）、:922-923 NewRecordProcess MES2107／ChangeLog（事件紀錄）、A37 且 Initial 類模式時 :761／:764 SaveToFile 兩份 Bundle ID 清單；:768 UpdateMainOperateMode（真本體，見 3a） | TestIF_File_YieldMonitoring.cpp:31 `&YM_btnApplyClick` 換掉；closed（gen.inc:3050）或 SaveSetupFile（:3463） | 每存一次就多兩筆事件紀錄（golden 一次開關一筆）；UpdateMainOperateMode 在 HT9050 機台上會動加熱器繼電器 | ⏳ 等 Q41 後續同事（Yield／Configuration C 路；`a8eca460` 的 YM-1 是網頁 JS，C++ 檔沒動過，但他可能還要改） |
| **CT-4** Setup.Contact | `sbContactClick` :28302-28317；:28306 NewRecordProcess；:28308-28312 RTC vision 收尾（頭段，機台通訊）；**:28314 `fContact->Show()`（非 modal，Jimmychiu 20240731）**；:28315 DoStructUnitConvert、:28316 SetWorkParameter → **開窗當下就跑** | 活 | SetWorkParameter 以讀為主 | DeviceForm_File.cpp:208-214 `FormShowAndSnap()`（＝editlist.get 的 golden FormShow）的 :213 TakeSnapshot 之後；不是 SaveFlow | golden 自己的怪處：該行註解「存檔後要重新 load 參數」，改成 Show 之後變成「開窗前」跑，存檔後反而不跑（Contact 的 spbSaveClick cContact.cpp:14179 起沒有 SetWorkParameter）。照 golden 翻、寫 //AI 註解；網頁存完會自動重讀＝重開頁，所以存完也會跑（R-新1）。editlist.get 運轉中也會被呼叫（只看不存），尾段一定要自己查機台停著 | ✅ 能（先做 R-新1 的 A） |
| **BS-2** Setup.BinSel | `sbBinClick` :28319-28328；:28324 ShowModal；:28325 DoStructUnitConvert、:28326 fShowBinSelect->ShowBinSel()、:28327 SetWorkParameter。關窗 golden cBinSel.cpp:2137-2145 `FormClose`：bShow=false、ReadFile(false,false,"")、fShowBinSelect->InitShowBinDigital() | 全部活：InitShowBinDigital＝cShowBinSelect.cpp:316（兩個主畫面旗標）；移植樹 fBinSel->ReadFile 本身就會跑 SetWorkParameter（cBinSel.cpp:1796，golden cBinSel.cpp:1557 同） | 不寫 | BinSelect.cpp `SaveFlow()` :601-640 已是手寫包裝，:627 `BS_spbSaveClick()` 之後接；closed（gen.inc:1070）或 SaveFunctionData（savedMark :649） | 同 TA-6，ShowBinSel 影響分 bin。A02 關窗時要**先還原（Reload :643＝golden FormClose 的 ReadFile）再跑尾段**，否則 ShowBinSel 可能用到頁面套上但沒存的 bin 資料（PageSave 的 reload 在 saveFlow 之後才跑，_EditPage.cpp:172） | ✅ 能（BinSelect 走自己的入口 FileRW_BinSelect_Save :774，最後一樣進 PageSave :838） |
| **CC-E15** Config.Configuration | `sbConfigurationClick` :28599-28658。頭段：:28603-28607 CC_SCS 等級 30（客戶專屬）、**:28615 記 bE43AutoCleanUseHotplate、:28618-28622 記 A10／I37／A51／I21／A78 五個舊值**、:28624 ShowModal。尾段：:28626-28632 CC_AMKOR_Korea（bQAMode、sbQAMode 可見、A78 變了重建清單）、:28634 SetWorkParameter、:28635 UpdateMainOperateMode、:28637 fYieldMonitoring->ReadFile()、:28638 fSCKART->AccessFile(true)、:28641 ShowTestHeadComp(false)、:28642-28646 A09 ShowTestCategory、:28648-28651 E43 變了 LoadAutoCleanData、:28653-28657 四個條件任一成立 SetStartModeData | 全部有：fYieldMonitoring->ReadFile＝forms\fYieldMonitoring.h:382（開機也用這支）；fSCKART->AccessFile＝forms\fSCKART.cpp:151（St02 W1 只做 [AutoRetest] iTesterType 一組，bUseSCKART 關時直接 return）；其餘同上 | UpdateMainOperateMode 寫 lastdata.dat；AccessFile 在 bUseSCKART 且缺鍵時寫 <配方>\Tester.Data [AutoRetest] iTesterType=1；E43 變了時 LoadAutoCleanData 會寫（同 HP-3）；SetStartModeData 會寫（同 YM-5） | IniConfig.cpp：開頁 `IniConfigPageJson` :400-403 的 :402 `IC_FormShow()` 之前記舊值；存檔 `FileRW_IniConfig_Save` :346 `IC_FormClose()`（＝golden FormClose）之後、:351 `if (!saved) ReadLastSetIni();` 之後跑尾段，**不看有沒有存成**（golden 關窗一律跑）；:306-314 密碼守衛整次拒存時不跑（golden 視窗還開著） | UpdateMainOperateMode 會動加熱器繼電器／ATC7（HT9050 機台驗證要機台端在場，RULINGS_20260927 第 16 條）；AMKOR_Korea 三行照 S25 先 GATE＋註記 | ⏳ 等 Q41 後續同事（IniConfig.cpp 是 `a8eca460` 剛加 107 行的檔；Q45／Q46 還沒裁，這個檔可能再改） |

### 3a. 尾段用到的函式：移植樹現況

| golden 呼叫 | 移植樹本體 | 閘 | 寫檔 | 碰到的執行中資料 |
|---|---|---|---|---|
| DoStructUnitConvert() | cUnitConvert.cpp:675（golden cUnitConvert.cpp:243-260） | 無 | 不寫 | *_File → 執行值：TestIF、DeviceForm、HotPlateForm、ArmOffset、ArmSpeed（memcpy 整份，所以 :690 ATC HotGun 加時不會累加）、UserDefForm |
| SetWorkParameter() | cinitial.cpp:7084-7322（golden cinitial.cpp:13505-13594） | 只剩 :7309 `fMain->ShowFunctions()`（主畫面功能列表重畫）在 `#if 0` | 以讀為主：ReadTechData 缺鍵補寫 D:\HT9045\system\Gerneral.ini [Shuttle] CHECK_RANGE／iInShtZRange（cinitial.cpp:16137-16138）；fTeach->ReadFile 第一次把舊 tech 轉 teach.ini 時會寫（forms\fTeachPara.cpp:370 起） | ChangeSite（site map、iInArmType）、Tech 重讀、SetTechDataToProd／DoSetupSystemToProd（Prod 位置；:9352 還會 fTrayAssignment->ReadFile）、SThreadPara、吸嘴 retry。rotate shuttle 不支援的模式會跳 ShowMyMessage（:7270） |
| fShowMessage->ShowTrayDeviceDir() | 沒有 | — | — | 純畫面（主畫面方向圖） |
| fCleaning->LoadAutoCleanData() | FileRW\TestIF_File_Cleaning.cpp:399 FileRW_Cleaning_LoadAutoCleanData（沒開機完會印一行並跳過） | 無 | 會寫（見 HP-3） | Auto Clean 結構＋Cleaning 頁替身 |
| GetHotPlateYHalfPos() | ainarm_SearchPlacePlate.cpp:132 | 無（InitialOK==false 時 return；InitialOK 在 WebBridgeTags.cpp:563 PumpInit 成功後才是 true） | 不寫 | iYHalf、b6x20HP、b4x11HP、b4x10HP_2x2、b8x16HP_2x2 |
| fShowBinSelect->ShowBinSel() | cShowBinSelect.cpp:1187 | GATE B10／B11（ART 兩個子函式是 no-op） | 不寫 | iBinTray、bUnloadHasBin、iTrayLastBin、BinAssign（分 bin 在用） |
| fShowBinSelect->InitShowBinDigital() | cShowBinSelect.cpp:316 | 無 | 不寫 | 主畫面顯示旗標 |
| fSortCT->UpForm() | forms\fSortCT.cpp:488 | 無 | 不寫 | 只改 SortCT 面板可見／位置 |
| fMain->SetStartModeData() | RunStartMode.cpp:1298 | 無 | 會寫（見 YM-5） | 起動模式清單、LastSet、UpdateMainOperateMode、SetWorkParameter |
| UpdateMainOperateMode() | forms\fMain.cpp:507 → forms\fMain_OperateMode.cpp:139，wb_serve 開機裝（tools\wb_serve.cpp:4065） | 本體裡 web-display 的幾行 | 寫 lastdata.dat | 加熱器繼電器、ATC7 指令（使用者 20260922 裁決「全部動作都要執行」） |
| fYieldMonitoring->ReadFile() | forms\fYieldMonitoring.h:382（uYieldMonitoring.cpp） | 只擋 UI | 以讀為主 | TestIF_File 的 Yield 欄位 |
| fSCKART->AccessFile(true) | forms\fSCKART.cpp:151（W1，部分） | 只做 iTesterType 一組 | bUseSCKART 且缺鍵時寫 Tester.Data | CosFunction.bAutoRetestGPIBmode |
| ShowTestHeadComp(false) | forms\fMain.cpp:247 空殼 | — | — | golden ShowTestHeadComp1 本體歸 Jimmy（S88） |
| fTestCategory->ShowTestCategory() | forms\fTestCategory.cpp:204 | 無 | — | 顯示（SetUp 尾段已在叫） |
| EventReport(SECS_EVENT.EnterSpeed) | SECSGEM\SecsEventReport.cpp:15 | 模擬計數 | — | — |

**運轉中**：editlist.save 在 移植樹 tools\wb_serve.cpp:5303 擋 `SystemStart||SoftStart`（R0927-7），所以 C 路 7 頁的尾段只會在停機時跑。沒擋的兩個入口：HotPlate 的 form.save（:5340）與 Contact 的 editlist.get（:5199，開頁本來就不擋）。golden 運轉中打不開這些視窗（golden V912 main.cpp:3970-3978 DoMainPadProcess 把 palSetup／palConfig 藏起來；:29030 sbSettingClick 開頭 `SystemStart` 就 return）。

**START 時會不會補上**：會一部分。golden ckernel.cpp:366-372（移植樹 ckernel.cpp:810 起、:830）START 的 ScanSystemSensor 在 SoftStart 時跑 SetWorkParameter（裡面含 DoStructUnitConvert）。所以「沒跑尾段」的後果主要在**存檔到下一次 START 之間**（手動動作、Teach、Contact 模式、主畫面顯示），以及 START 不會重跑的那幾支：ShowBinSel（TA-6、BS-2）、SortCT UpForm、LoadAutoCleanData（HP-3、CC）、SetStartModeData（YM-5、CC）。盤點表第七節第 2 點的推論可以這樣收斂。

---

## 四、掛點：C 路存檔流程有沒有「存檔成功後」的共用掛點

- **沒有**。移植樹 FileRW\_EditPage.h:29 `struct PageDesc` 只有 formShow、saveFlow、savedMark、reload、booted，外加可選的 beforeApply（:49）、extraJson（:51）；PageSave（_EditPage.cpp:84 起）在 :169 呼叫 saveFlow 之後只做 reload／closed 標記。Q40（`76058840`）的事件表刻意另外註冊（_EditPage.h:79-80 PageEventsRegistrar），註解寫明原因：「PageDesc 是逐欄位初始化的彙總型別，加欄位會讓每個結構的 cpp 多一個 -Wmissing-field-initializers 警告」。
- A 形狀（HotPlate）：JsonBridge\FormJson.cpp:115 `FormSave` 在 :172 呼叫 `b->saveFlow(J)` 之後直接回 ack；BridgeDesc（JsonBridge\FormBridge.h:155）也沒有尾段欄位。
- tools\editlist\*.py、tools\gen_editlist.py：描述檔只管 gen.inc（golden 方法轉換），**不產生**各結構的入口 cpp（_editlist_sources.cmake 檔頭：「新增結構：在 STRUCTS 加一筆、寫 FileRW/<struct>.cpp、重跑本產生器」）。入口 cpp 是手寫的，Temp_Set／SetUp 的尾段就是在手寫入口 cpp 裡接的。

**建議的掛法（照現成兩頁，不加共用掛點）**
- 尾段本體：全部放 移植樹 FileRW\MainClick.cpp 檔尾（golden 本體都是 TfMain 的按鈕處理器，MainClick.cpp 檔頭就是「golden TfMain 主畫面按鈕處理器裡讀寫檔的那一半」；SetUp 的尾段已在這裡）。
- 宣告：新檔 移植樹 FileRW\MainClickTail.h（全域範圍宣告 9 支＋Configuration 的開頁記錄函式）。HotPlate 的產生碼在 `namespace ht9045::formbridge` 裡，區塊內 `extern` 會宣告成錯的命名空間，所以一定要有全域標頭（產生器 'includes' 帶進去）。
- 呼叫：各頁手寫入口 cpp 的 saveFlow 包一層（或 BinSelect 在既有 SaveFlow 裡接），Contact 接在開頁，Configuration 接在 FormClose 之後。
- 不加共用掛點的理由：9 頁裡只有 LU／TF／TA／SP／YM 5 頁長得一樣；Contact（開頁）、Configuration（自己的存檔入口，不走 PageSave）、HotPlate（A 形狀）、BinSelect（已有手寫 SaveFlow）都是例外，共用掛點只省 5 個三行函式，卻要動 Q40 剛改完的 _EditPage.*；也照 Q40 的前例不再加 PageDesc 欄位。

**每支尾段本體的共同規矩**
1. 開頭再查一次機台停著：`if (SystemStart || SoftStart)` → 不跑、回一句原因（呼叫端放進 ELTodo 或 J.Todo）。C 路 7 頁是多一道保險，HotPlate、Contact 是真的需要。註解標 [W906]：golden 運轉中打不開視窗，這是把 golden 的前提在伺服器端重查一次，不是改行為。
2. 回傳 `const char*`：nullptr＝跑了；非 nullptr＝沒跑的原因。這樣 C 路（filerw::ELTodo）和 A 形狀（J.Todo）都接得上，不必在 MainClick.cpp 裡碰兩套訊息收集。
3. 每支開頭 `filerw::ELMark("W906_Main_sbXxxClickTail")`（同 MainTempOffsetTail :72），printf 一行，存檔 ack 與 wb_serve 主控台看得出有跑。
4. 純畫面的行（ShowTrayDeviceDir、sbQAMode->Visible）寫成註解保留 golden 原文；客戶專屬（AMKOR_Korea）`#if 0 // GATE ... S25` 並在 CUSTOMER_CODE 相符時 ELTodo（同 S88／S100 對 AmkorSendMessage 的寫法）。
5. A02 關窗（closed）時：先跑該頁的 Reload（＝golden FormClose 的 ReadFile＋DoIniDataToForm）再跑尾段，順序同 golden（Temperature.cpp:208-209 先 TS_FormClose 的前例）。PageSave 之後會再 reload 一次，重讀兩次無害。
6. 行號規矩（D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md §1、§2）：既有行只做同一行改寫；前置宣告佔用原本的空行；新函式放檔尾。這些 cpp 的行號被盤點表、ChangeLog 引用。

---

## 五、逐頁實作步驟

行號都是 `99ec7b7b`。「同一行」＝整行改寫、行數不變。

### 5.0 共用（先做，一顆 commit，沒有行為變化）
1. 新檔 移植樹 FileRW\MainClickTail.h：宣告
   `const char* W906_Main_sbLdUldClickTail();`、`…sbTrayFormClickTail();`、`…sbPlateFormClickTail();`、`…sbTrayAssignClickTail();`、`…sbSpeedClickTail();`、`…sbYieldClickTail();`、`…sbContactClickOpen();`、`…sbBinClickTail();`、`void W906_Main_sbConfigurationClickHead();`、`const char* W906_Main_sbConfigurationClickTail();`（檔頭寫 golden 出處與「只給 wb_serve」）。
2. 移植樹 FileRW\MainClick.cpp 檔尾（:544 之後）加一段 `AI(W906-FRW-Q41-CT)`：需要的 include（forms\fShowBinSelect.h、forms\fSortCT.h、forms\fYieldMonitoring.h、forms\fSCKART.h、SECSGEM\SecsEventType.h、SECSGEM\SecsEventReport.h、FileRW\MainClickTail.h；都只帶 vclcompat／MachineType，不碰兩個 TMyKitSuck，陷阱 #3）；前置宣告 `void GetHotPlateYHalfPos();`、`void FileRW_Cleaning_LoadAutoCleanData();`、`namespace filerw { void ELMark(const char*); }`（:74 已有 ELTodo）；再放 9 支本體，逐行對照第三節的 golden 行號。Configuration 的開頁舊值（golden :28609-28615 的 static＋bBackupState）放同一段的匿名 namespace。
3. 本體寫好但沒有呼叫端 ⇒ 不影響建置結果；陷阱 #2 的檢查在接呼叫端那一步做。

### 5.1 LU-2（移植樹 FileRW\Ld_UldDelayTime.cpp）— 現在可做
- :18（空行）→ `#include "FileRW/MainClickTail.h"`
- :24（匿名 namespace 內的空行）→ `void SaveFlow();   // AI(W906-FRW-Q41-CT) … 定義在檔尾`
- :32 同一行：`&LU_spbSaveClick` → `&SaveFlow`
- 檔尾（:56 之後）：重開 `namespace {`，`void SaveFlow() { LU_spbSaveClick(); if (filerw::ELMarked("closed")) { LU_ReadFile(); …Tail } else if (filerw::ELMarked("SaveSetupFile")) …Tail }`（golden FormClose cLd_ULd.cpp:170 只有 ReadFile＋fShow=false）
- 描述檔 tools\editlist\Ld_UldDelayTime.py：不改；不重產。

### 5.2 SP-6（移植樹 FileRW\ArmSpeed_File.cpp）— 現在可做
- :17（空行）→ include；:23（空行）→ `void SaveFlow();`；:28 同一行 `&SP_spbSaveClick` → `&SaveFlow`；檔尾加 SaveFlow（`SP_spbSaveClick();` 之後無條件跑尾段——golden Speed 存檔沒有 A02 關窗）。
- 尾段本體：DoStructUnitConvert；`if (IniConfig.bEnable_SECS_GEM) EventReport(SECS_EVENT.EnterSpeed);`（照 golden 放在關窗後；註明每存一次送一次、目前 EventReport 是模擬計數）。
- 不改描述檔、不重產。

### 5.3 TA-6（移植樹 FileRW\TrayForm.cpp，Tray Assignment 頁）— 現在可做
- :30（空行）→ include；:246（匿名 namespace 內空行）→ `void SaveFlow();`；:259 同一行 `&TA_spbSaveClick` → `&SaveFlow`；檔尾加 SaveFlow（closed → `Reload()` :249 再尾段；SaveSetupFile → 尾段）。
- 尾段本體：DoStructUnitConvert、SetWorkParameter、`fShowBinSelect->ShowBinSel()`、`fSortCT->UpForm()`。
- 不改描述檔、不重產。

### 5.4 BS-2（移植樹 FileRW\BinSelect.cpp）— 現在可做
- :32（gen.inc 之後的空行）→ include（MainClickTail.h 只有 W906_Main_* 宣告，不會撞到 gen.inc 的 #define）；:599（空行）→ `void BinCloseTail();`（匿名 namespace :87-680，:599 與 :601 SaveFlow 都在裡面）
- :627 同一行：`BS_spbSaveClick();` → `BS_spbSaveClick(); BinCloseTail();   // AI(…) golden FormClose cBinSel.cpp:2137＋main.cpp:28325-28327`
- 檔尾：`BinCloseTail()`：closed → `Reload()`（:643，golden FormClose 的 ReadFile）；closed 或 SaveFunctionData → `fShowBinSelect->InitShowBinDigital()`（golden FormClose :2141）＋尾段本體（DoStructUnitConvert、ShowBinSel、SetWorkParameter）。
- 放在 :627 而不是 :639：:629-639 的清單比對讀的是 ReadFile 重讀的 TStringList，尾段不動它們，前後都可以；放 :627 才不用加行。
- 不改描述檔、不重產。

### 5.5 CT-4（移植樹 FileRW\DeviceForm_File.cpp）— 現在可做（照 R-新1 的 A）
- :37（空行）→ include
- :213 同一行：`TakeSnapshot();` → `TakeSnapshot(); if (const char* w = W906_Main_sbContactClickOpen()) filerw::ELTodo(w);   // AI(…) golden main.cpp:28314 fContact->Show()（非 modal）之後 :28315-28316`
- 本體（MainClick.cpp）：運轉中不跑；DoStructUnitConvert、SetWorkParameter；//AI 註解寫 golden 的怪處（:28316 註解說存檔後、實際在開窗前）與「網頁存完自動重開頁，所以存完也會跑」。頭段 :28308-28312（RTC vision rtInspEnd）是機台通訊，不在本項（交 Jimmy，另列）。
- 不改描述檔、不重產。

### 5.6 TF-5（移植樹 FileRW\UserDefForm_File.cpp）— 等 Q40 同事確認
- :20（空行）→ include；:29（空行）→ `void SaveFlow();`；:34 同一行 `&TF_spbSaveClick` → `&SaveFlow`；檔尾（:64 之後）加 SaveFlow（closed → `TF_ReadFile(); TF_DoIniDataToForm();`＝golden FormClose cTrayForm.cpp:559 → 尾段）。
- 尾段本體：DoStructUnitConvert、SetWorkParameter；`fShowMessage->ShowTrayDeviceDir()` 寫成註解（純畫面、網頁主畫面沒有，另列給畫面那邊）。
- Q40 登記的是 :37-40（事件表）與 UserDefForm_File.gen.inc／tools\editlist\UserDefForm_File.py；本項不碰那幾處。不改描述檔、不重產。

### 5.7 HP-3（A 形狀，要改產生器）— 等 Q40 同事確認；Q-新1 並行問
- tools\gen_formbridge.py:330：SaveFlow 的產生加一個可選鍵，例 `FORM['saveFlowAfter']`（字串清單，照原樣接在 `%s(J);` 之後）。不給這個鍵的表單輸出一字不變。
- tools\formbridge\TfHotPlate.py：'includes' 加 `'FileRW/MainClickTail.h'`；加 `'saveFlowAfter': ['if (J.M("closed") || J.M("saved")) { if (const char* w = ::W906_Main_sbPlateFormClickTail()) J.Todo(w); }']`（J.M("saved") 在 B_SaveSetupFile 一進來就設，:266；closed 在 :231）。
- 重產：`C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_formbridge.py --only TfHotPlate`，確認 HotPlateForm_File.cpp 的 diff 只有 include 一行與 SaveFlow 內兩行；其他 A 形狀表單（如果有）不變。
- 尾段本體：運轉中不跑；DoStructUnitConvert、SetWorkParameter、FileRW_Cleaning_LoadAutoCleanData()、GetHotPlateYHalfPos()。closed 的情況不必另外還原：A 形狀每次請求一份 FormState，沒有伺服器端替身；golden FormClose 的 fHotPlate->ReadFile 讀的是沒改過的檔。
- 另一條路（不建議）：在 JsonBridge\FormJson.cpp:172 之後查一個「頁面→尾段」登記表，不動產生器。缺點是尾段離頁面的 SaveFlow 很遠，且 FormJson.cpp 剛被 `834fcc78` 大改。

### 5.8 YM-5（移植樹 FileRW\TestIF_File_YieldMonitoring.cpp）— 等 Q41 後續同事
- :20（空行）→ include；:26（空行）→ `void SaveFlow();`；:31 同一行 `&YM_btnApplyClick` → `&SaveFlow`；檔尾加 SaveFlow（closed → `YM_ReadFile()`；closed 或 SaveSetupFile → 尾段）。golden FormClose uYieldMonitoring.cpp:3288 只有 fShow=false，不必另外還原。
- 尾段本體：DoStructUnitConvert、SetWorkParameter、`fMain->SetStartModeData()`。
- 不改描述檔、不重產。golden btnOkClick（uYieldMonitoring.cpp:3190，ReadFile＋主畫面 Yield 狀態＋Close）是盤點 YM-3，另案。

### 5.9 CC-E15（移植樹 FileRW\IniConfig.cpp）— 等 Q41 後續同事
- include：找檔頭的空行（或同一行接在既有 include 後面）。
- :402 同一行：`IC_FormShow();` → `W906_Main_sbConfigurationClickHead(); IC_FormShow();`（golden 在 ShowModal 之前記舊值，也就是在 FormShow 讀檔之前）
- :351 同一行：`if (!saved) ReadLastSetIni();` → 後面接 `if (const char* w = W906_Main_sbConfigurationClickTail()) filerw::ELTodo(w);`（golden :28626 起；不看 saved——golden 關窗一律跑）。
- 尾段本體：AMKOR_Korea 三行 GATE（S25）＋CUSTOMER_CODE 相符時 ELTodo；SetWorkParameter；fMain->UpdateMainOperateMode；fYieldMonitoring->ReadFile；fSCKART->AccessFile(true)；fMain->ShowTestHeadComp(false)；A09 → ShowTestCategory(0)／(1)；E43 變了 → FileRW_Cleaning_LoadAutoCleanData；`(USE_AUTO_RETEST==eartInstall && A10 變了) || (CosFunction.bHaveFIFOMode && I37 變了) || (CosFunction.bCanDisableQAMode && A51 變了) || (IniConfig.bUseAutoSiteMapping && I21 變了)` → fMain->SetStartModeData。符號都在（Config.h、CosFunction.h、MachineType.h、cmydef.h 已確認）。
- 同一個函式的相關發現（不在 9 頁範圍，給 Q41 後續同事）：移植樹 FileRW\IniConfig.gen.inc:8169-8173 `CheckConfigurationBeforeSave` 裡 golden cConfiguration.cpp:7645-7648 的 `fCleaning->LoadAutoCleanData()` 還是 GATE（tools\editlist\IniConfig.py:109 的 blocks），理由「移植樹仍是 GATE」已過期——現在有 FileRW_Cleaning_LoadAutoCleanData。照 generators.md 廿九節：拿掉那條、改成 replace 接過去、`gen_editlist.py --only IniConfig` 重產。

---

## 六、建議順序與驗收

**順序**
1. 5.0 共用本體＋標頭（沒有呼叫端，零行為變化）。
2. SP-6、LU-2（只有換算，最單純）。
3. TA-6、BS-2（牽涉分 bin 的 ShowBinSel，驗收要看 iBinTray）。
4. CT-4（照 R-新1 的 A 先做，可推翻）。
5. TF-5（Q40 同事點頭後）。
6. HP-3（Q40 同事點頭後；尾段自帶運轉中檢查，Q-新1 另外決定 form.save 本身）。
7. YM-5、CC-E15（Q41 後續同事交件後）。
每一步一顆 commit，本文寫「same-line, no line moved（除了檔尾與空行）」。

**驗收（照 wbserve-conventions.md §6：這台不跑 wb_serve）**
- 兩組態語法檢查（sim／ship）＋MinGW 建 wb_serve；比對警告基準。
- 陷阱 #2：`nm --undefined-only` 各頁入口 cpp 的 .obj 要看到 `W906_Main_sb…Tail`；`nm --defined-only` MainClick.cpp.obj 要有它們。
- `build.bat gate` 全新 build dir，ctest **比對失敗清單**（FileRW 只編進 wb_serve、不在任何 ctest，但 MainClick.cpp 多了 include，要確認沒有別的失敗）。
- 行號自檢：`git -C D:\HT9045 diff -U0 99ec7b7b..HEAD -- <檔>` 的 hunk 除了檔尾與空行都是 `-N +N`。
- 端對端（能跑 wb_serve 的那台，照 Jimmy 0918 規則「備份→驗證→還原」）：每頁存一次，看 ack.session.todo 與主控台的尾段那一行；TA-6／BS-2 另外看 iBinTray（主畫面 Bin 顯示）有沒有跟著換；HP-3 在運轉中送 form.save，尾段要回「運轉中不跑」。可以照 移植樹 tools\webprobe\formevent_probe.py 的樣子寫一支 closetail_probe.py。

---

## 七、要 Steven 決定的

### Q-新1. HotPlate 頁運轉中存檔要不要擋（RULINGS_20260927 第 7 條的延伸）
**背景**：Steven 已決定「機台運轉中，網頁一律不能存設定」（RULINGS_20260927 第 2 條第 7 題 A），但當時只擋了其他設定頁用的存檔指令。HotPlate 頁用的是另一條存檔指令，當時註明「另列給 Steven 決定」，一直沒列出來。BCB 版運轉中根本打不開 HotPlate 視窗。這次要補的「存完重新載入參數」會重算入料臂把 IC 放到加熱盤上的位置。
**選項**：
- A：HotPlate 頁運轉中也一律不能存（跟其他設定頁一樣）。
- B：可以存進檔案，但「重新載入參數」運轉中不跑，停機後按 START 才生效。
- C：維持現狀（存檔不擋，重新載入也照跑）——不建議。
**St01建議**：A。不管選哪個，「重新載入參數」本身都會先檢查機台停著（這是 BCB 版本來就有的前提）。
**例子**：生產中有人在網頁 HotPlate 頁把 Y Pitch 從 12.70 改成 13.00 按存檔。A：畫面顯示「機台運轉中不能存設定」，檔案不動。B：HotPlate.Data 變成 13.00，這一批繼續用 12.70，停機後按 START 才用 13.00。C：存完馬上重算，入料臂下一次放料就可能用 13.00，跟正在加熱盤上的 IC 位置對不上。

### R-新1. Contact 頁的「單位換算＋重新載入參數」照 BCB 版在開頁時跑（CT-4）
**背景**：BCB 版（V912、V899 都一樣）在 2024-07-31 把 Contact 視窗改成「開著也能操作主畫面」的視窗，結果主畫面的「單位換算＋重新載入參數」變成在**打開 Contact 視窗的當下**就跑完，不再等關窗；那一行原作者的註解是「存檔後要重新載入參數」，看起來原本的用意在改版後沒有了。照翻譯規則，照 BCB 版做、在程式裡註明這個怪處。
**選項**：A 照 BCB 版：網頁每次打開 Contact 頁時跑／B 改成存檔後跑（照註解的原意，跟 BCB 版不同）／C 兩個都跑。
**St01建議**：A。網頁存完一定會自動重新打開頁面讀回新值，所以 A 實際上存完也會跑一次，而且沒有偏離 BCB 版。
**例子**：操作員改 Contact 高度按存檔 → 網頁自動重讀 → 伺服器這時跑「重新載入參數」→ 機台用的是新高度。BCB 版在同樣操作下要關掉再開一次 Contact 視窗、或按 START 才會換成新高度。

### R-新2. 其餘 8 頁照溫度頁的做法（S107-1）：存檔成功就跑
**背景**：Steven 在 S107-1 決定溫度頁的收尾「存檔後就跑，不等按 Exit」，SetUp 頁也是這樣做的。這次 Ld_ULd、TrayForm、HotPlate、Tray Assignment、Speed、Yield、BinSel 照同一做法；Configuration 比較特別：網頁按存檔就等於 BCB 版的關窗，所以不管按存檔時選「是」或「否」都會跑，跟 BCB 版完全一樣。
**選項**：A 照 S107-1 延伸到這幾頁／B 改成網頁按 Exit 才跑（Cleaning 頁現成的做法：存檔指令多帶「按了哪一顆鈕」）。
**St01建議**：A，跟溫度頁、SetUp 頁一致。跟 BCB 版唯一的差別是「打開頁面、沒存就關掉」BCB 版會跑一次、網頁不跑——那種情況值沒有變，結果一樣。
**例子**：在 Speed 頁把 Index 速度從 80% 改成 60% 按存檔 → 馬上換算，停機時手動動作就用 60%（不必等按 START）。只開 Speed 頁看看就關 → BCB 版會跑一次換算（值沒變、結果相同），網頁不跑。

### R-新3. 收尾一律先檢查機台停著（知會）
**背景**：BCB 版運轉中打不開這些設定視窗，所以收尾永遠只會在停機時跑。網頁的 Contact 頁開頁、HotPlate 頁存檔在運轉中都送得進來，伺服器要自己再查一次。
**選項**：A 每支收尾開頭都查，運轉中不跑、回一句原因／B 只在 Contact、HotPlate 兩頁查。
**St01建議**：A（多查一次沒有壞處，也符合「指令進 C++ 後要重新過互鎖」的原則）。
**例子**：生產中有人打開網頁 Contact 頁看數值 → 伺服器照常回畫面，但不跑「重新載入參數」，回一句「機台運轉中，重新載入參數這次沒跑」。

（不需要 Steven 決定、照既有裁決處理的：Configuration 收尾裡 AMKOR_Korea 的三行＝客戶專屬，照 S25 跳過並註記；Speed 的 SECS「Enter Speed Page」事件照 BCB 版在收尾送，目前 SECS 還沒真的接上（只計數），等接上再看要不要改成一次開頁只送一次。）

---

## 八、順手發現（要不要改盤點表由 ST01-E 決定）

1. 盤點 CT-4 寫「關窗尾段」不對，是開窗當下（golden V912 main.cpp:28314 Show()）。
2. 盤點 SP-6 寫「進頁時送 SECS EnterSpeed」不對，程式順序是 ShowModal 回來之後（關窗時）送（:28695→:28698-28699）。
3. 盤點 CL-5 寫 golden `TfCleaning::sbTrayAssignClick` 會開 Tray Assignment 頁，實際 golden V912 AutoClean\uCleaning.cpp:2311-2315 是 `fMain->MainFormSizeToEpson(false); fMain->AutoCleanStringGrid->Visible=true;`（主畫面縮放＋顯示 Auto Clean 格子），不開 Tray Assignment。
4. 「頭段」也有缺：9 頁 golden 的 sbXxxClick 在 ShowModal 之前都有 `NewRecordProcess("MES21xx","Enter …")`（例 main.cpp:28408 MES2173、:28452 MES2176、:28694 MES2187），移植樹除了 AlarmCodeCatalog.cpp 的代碼表，沒有任何地方在網頁開頁時記這筆（2026-09-27 `git grep MES2173|MES2176|MES2170|MES2187|MES2185|MES21109|MES2177`）。盤點表沒列。另 Contact 頭段 :28308-28312（RTC vision rtInspEnd）是機台通訊，歸 Jimmy。
5. golden 主畫面「設定」選單開／關（main.cpp:29009-29025 sbConfigClick、:28440-28446 sbExitClick）各跑一次 SetWorkParameter＋UpdateMainOperateMode，網頁選單開關不送伺服器。跟盤點 C-2 同一個按鈕，可以一起記。
6. 過期註解與文件（第一節末尾）；另 移植樹 FileRW\Temperature.cpp:122 「移植樹門面計數 stub」已過期（UpdateMainOperateMode 真本體已裝）；移植樹 forms\fSCKART.h:99「offline no-op」已過期（W1 已有部分本體）；移植樹 FileRW\IniConfig.cpp:159 說「wb_serve 沒有把 InitialOK 設成 true」，但 移植樹 WebBridgeTags.cpp:563 在 PumpInit 成功時會設 true——兩處說法不一致，影響 GetHotPlateYHalfPos、LoadAutoCleanData 是否真的做事，實作 HP-3 時以 wb_serve 主控台實際印出的狀態為準。

---

## 九、沒做的與不確定的

- 這一趟沒改任何程式、沒 build、沒跑 wb_serve；上面的「活」是讀程式與符號定義判斷的，不是實跑。
- 沒有逐行驗 SetWorkParameter 裡每一支子函式會不會寫檔，只驗了 ReadTechData、fTeach->ReadFile、DoSetupSystemToProd 裡的 fTrayAssignment->ReadFile（TA_ReadFile）；「以讀為主」的說法沿用 移植樹 FileRW\MainClick.cpp:331 既有註解。
- 尾段裡的 golden 函式可能跳 ShowMyMessage（例 移植樹 cinitial.cpp:7270 SetWorkParameter、RunStartMode.cpp:526 SetRunStartMode「No Run ART Mode」；同檔 :1202／:1223 在 W906_CbRunStartModeChange :1184，SetStartModeData 走不到），在 editlist.save 持 FormLock 期間會變成網頁的阻塞框。溫度頁、SetUp 頁尾段與換配方已經是同樣情況，但沒有實測過阻塞框在存檔途中的行為。
- YM-5、CC-E15 的步驟是照 `99ec7b7b` 寫的；Q41 後續同事交件後行號要重量。
- TF-5、HP-3 碰到的檔在本查證期間由 Q40 推進 `76058840`（15:23），工作樹目前乾淨；是否還有後續（R77～R79 的回饋）要問 Q40 同事。
- 盤點表列的其他尾段不在本項：TI-5（TesterIF，Steven02）、TS-10／SU-9（離開頁面的機台動作）、ContactForce FormClose（ChangeLog §11.33 已交 Jimmy）。
