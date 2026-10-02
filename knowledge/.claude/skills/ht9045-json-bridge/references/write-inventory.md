# 寫檔盤點 —— 以結構為主軸（S12 第二型的工作清單）

> Steven 20260924 建立；20260925 更新逐結構狀態（C 形狀第二波五個結構）並補兩條通用規則
> （一之四、一之五）。來源：golden `HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（cp950）逐支 grep，
> 移植樹 `HT9011UC_Cpp_V3.33.906.0` 對照（`#if 0` 內的呼叫點不算 live）。
> 使用者 20260924：「全部不同結構都要做」「其餘 12 個是自由函式，但是應該有寫檔相關的行為吧？由 BCB 找出來並修正」
> 「HTEditList（config.ini 的 elConfig 等）和二進位檔（lastdata.dat、levelset.dat）也要設計寫檔的 cpp」。
>
> **為什麼要這份而不只看表⑧**：表⑧（`table8-fileio-bridge-status.md`）以「寫檔單位」列、只算 `WriteIniData` 那一套，
> 而且產生於 ca4e903 之前。它不列 HTEditList（最大的 `elConfig` 就在這一套）、不列二進位，也看不出一個結構被幾個單位寫。
>
> 落點：`HT9011UC_Cpp_V3.33.906.0/FileRW/<結構>.cpp`（一個結構一支，**讀與寫在同一支**，BCB 表單分 function；`FileRW/README.md` 是產生器出的索引，**只反映整合跑之後的狀態**）。
> 兩個產生器（`gen_formbridge.py`＝A 形狀、`gen_editlist.py`＝C 形狀）怎麼用、怎麼加一個新表單、平行分工規則，見 `generators.md`——本檔只列「哪個結構做到哪」，不重複產生器操作細節。
> 資料夾 20260924 由 `WriteFile/` 改名 `FileRW/`（使用者：「把相同結構的 Read Write 整合到同一個 cpp」「WriteFile 改成 FileRW」）。
> 狀態：✅ 完成　⚠ 完成但有缺口　⏳ 未做　— 不需要做（見理由）

### 〇、頁面讀寫總表（20260925 `008db55e` 時點；每次整合或測完都要更新這張）

「通過」＝無頭瀏覽器從頁面讀＋真存檔，備份→測→SHA256 還原（判準見 `generators.md`）。
⚠ 本表只記**讀寫驗收**。C 路每個結構的接線（tag、頁面、PageDesc 入口、開機建替身、開機／換配方讀檔、`CRouteOwner`
擁有的檔）單一出處是 skill `ht9045-html-json` 的 `references/route-c-golden-bridge.md` §6（32 個結構＋HotPlate，
20260926 HEAD 8fad1522 核對；⛔ 20260927 HEAD 227b79db 起 35 個，#33～#35 已補），這裡不重列。⚠ 0926 之後的 commit 多數只做語法檢查、未 build（S51），「過」欄以
各列寫的 commit 當下實測為準。
備份範圍：D:\HT9045\IniData、system、config，加 D:\RS232Standard\System\Setup.ini、D:\GPIB9045\system\general.ini（HSys 寫死路徑）、D:\HT9045_Log\QtyData（CounterClear 真清除寫 QtyLog；測前清單、測後刪新檔）。
測試腳本 scratchpad `run_page.sh` 有互斥鎖（平行工作時同時只能一個 e2e）；檔名含空格要加引號。

| 狀態 | 頁面（結構） | 讀 | 寫 | 還差什麼 |
|---|---|---|---|---|
| ✅ | Configuration、Ld_ULd、TrayForm、YieldMonitoring | 過 | 過 | — |
| ✅ | Speed（ArmSpeed_File） | 過 | 過 | 第一次存原值 `Input Arm/Enable Relase Delay` 0→1 是 golden 正規化：非 CC_TSMC_TAINAN 一律 iEnableReleaseDelay=1（cSpeed ReadFile else 分支）→ DoIniDataToForm :2102 → 存 :2545；探針 `--allow '^Input Arm/Enable Relase Delay$'` |
| ✅ | TrayAssignment（TrayForm） | 過 | 過 | 20260925 開機／reload／SetWorkParameter 讀檔全部改走 golden 912 版（`g_W906_TrayAssignmentReadFileHook`，Steven 裁決） |
| ✅ | Contact（DeviceForm_File） | 過 | 過 | 改 Force Per Pin Kg 時 4 個衍生鍵照 golden `edDieForcePerPinGChange` 連動（探針 `--derived`） |
| ✅ | DIOInterFaceCFG（TTLCfg） | 過 | 過 | ⛔ 20260926 更正（舊句「`InitDIOStstus` 未移植、存檔 ack 列 ELTodo」移到 archive）：Jimmy `0ca03ee6` 已移植 `InitDIOStstus`（`cDIOStatus.cpp`），存檔後接上（`FileRW/TTLCfg.cpp:267-270`）；開機那一處仍閘著（`cDIOStatus.cpp:91`），見四、`TTLCfg` 列 |
| ✅ | BinSel（BinSelect） | 過 | 過 | 第一次存原值 golden 會重寫（`--allow "^Bin Func RT/|^Category6/Bin$"`）；`bin.rules` 27 項＋modeItems 由後端輸出真值（頁面推定 0 項，`11df5b42`）；後端驗證托盤 Link／Retest／CateR |
| ✅ | HandlerSys（HSys） | 過 | 過 | `cf530be8`：rgTrayArmType 補進 Steven 本機版頁面（Steven 核准 (b)）；9050GPIB 改善 A＋B；asHandlerVersion 由 exe 版本資源（3.33.906.0）設。第一次存 `UNLOADER_AUTO4~6_ART` 1→0 是 golden（AUTO_EMPTY_COLOR=1 設未安裝）|
| ✅ | Temp_Set（Temperature＋ATC.ini） | 過 | 過 | `bc935659`；存檔只送改過的面板欄位（WS 單則 64 KB 上限）；存檔會補寫 config\config.ini 缺鍵（golden SaveLastSetIni）；SOFT_SIMULTE InitialDelay=5 Steven 接受 |
| ✅ | OffSet（Offset_File） | 過 | 過 | `e086de38`；專用探針 `s12c_offset_probe.py`／scratchpad `run_offset.sh`；切組不先存（Steven 設計） |
| ✅ | SetUp（TestIF_File_SetUp） | 過 | 過 | `cf530be8`：根因是 golden InitialMemory 的 SiteData[] 在移植樹全 0，開機補填；ScrollBar1／CoSocketCombo／排序鈕存檔前照 golden 重播（_EditPage beforeApply）；換 Test Mode 也驗過 |
| ✅ | StartCondition（Data.StartCondition） | 過 | 過 | `e7e8f5f0`：golden TfStartCondition 19 支方法；測試欄位 edtInOutArmPickerAlmCnt（edtKitNo1 在 golden 開頁是停用的） |
| ✅ 🆕 | BarCode（TestIF_File_BarCode，golden TfBarCode 17 個方法） | 過 | 過 | `0609a14f`：W 帶 13 個 allow 才過（12 個 golden 補鍵、1 個 golden 改值：非 `bKoreaFunction` 機台 Check duplicate code by shuttle 強制 1）；開機／換配方接在 `W906_DoReadLastData` golden `:9361`／`:9405`；owner gate 加 `barcode.ini`（CHANGES_20260926 §3.1） |
| ✅ 🆕 | TowerLight（Status.TowerLight，WS towerlight.op） | 過 | 過 | `0609a14f`：S45 `LastSet.h` 補 V912 檔尾 `iBinBaseRT[256]`／`bO25_RTBaselined`（sizeof 178896→179928，389 欄與 golden 相同）；S46 音樂選項選了立刻寫檔；probe 28/30，另 2 項是 golden 開機遷移欄位一起寫出、非缺陷（CHANGES_20260926 §3.2） |
| ✅ 🆕 | ShuttleMove（HW.ShuttleMove） | 過 | 過 | `0609a14f`：S47 只有 latch 機台讀寫 `InSH?SenICAddPos`（本機非 latch）；`teach.ini` ALL PASS；換配方 golden `:25814` `ReadData` 已接；`cinitial.cpp` N3-G5 交給 Jimmy（CHANGES_20260926 §3.3） |
| ✅ 🆕 | Cleaning（Setup.Cleaning，golden TfCleaning） | 過 | 過 | `8af13c07`：開機依 golden `CreateForm`／`MainFormShow` 順序呼叫 `LoadAutoCleanData`；`autoclean.data`／`autocleancount.data` 上 owner gate；`edAlarmCount`／`edDevicePices` 唯讀＋小鍵盤可改已驗；`btnResetCleanCount`（歸零＋SECS 事件）未接線、先停用（ChangeLog 20260925 §14.1） |
| ⚠ 🆕 | QAMode（Setup.QAMode，TestIF_File_QAMode） | 過 | 部分 | `8af13c07`：存檔鈕**不**寫 `config.ini [Index]`（golden `QABackupStatus` 屬底層，交給 Jimmy）；golden `cBinSel.cpp:1415` GATE(G1) `fQAMode->ReadFile()` 待 Jimmy 解閘（ChangeLog 20260925 §14.2） |
| ⚠ 🆕 | VacuumUnit（HW.VacuumUnit，TestIF_File_VacuumUnit） | 過（不重讀） | 硬體鈕停用 | `8af13c07`：golden 開機與換配方都不呼叫 `ReadFile`，照做維持不讀；硬體鈕（Set／Set All／吸破真空／Reset）網頁端先停用（S48→Jimmy）；`iVaccumThrd*` 有無讀者待確認（ChangeLog 20260925 §14.2） |
| ✅ 🆕 | TesterIF（Setup.TesterIF，golden TFTestIF 14 個方法） | 過 | 過 | `8af13c07`：開機讀檔順序目前 bin 排在 tester 之前（與 golden 相反），已交給換配方工程師依 §14.4 順序重排；`cprod.h` 缺 golden 912 `[AMR]` 四欄交給 Jimmy；owner gate `tester.data` 四頁共用（ChangeLog 20260925 §14.3） |
| ✅ 🆕 | Status.Security Jam 分頁（WS security.jam） | 過 | 過 | `0609a14f`：open／select／save／import／export／stats，不走 editlist（section／鍵動態）；缺陷修正：開機補跑 golden `cSecurity.cpp:217-239`（建構子 static init 期 `FileNameJam000` 曾是空字串）；alarm 視窗待 `note.cpp` 14 個讀點翻完（交給 Jimmy）；匯出佔用主執行緒約 97 秒，要不要加「運轉中不能匯出」防護待 Steven 決定；密碼未實作（設計建議待 Steven 決定）（CHANGES_20260926 §10.3，本日第 5 次 e2e 通過） |
| ✅ 🆕 | Status.GroundMan（golden TfGroundMan 讀寫半邊） | 過 | 過 | `7a84e018`：`ReadGroundOffset`（system\GroundMan.ini）；`FileRW_GroundMan_Boot()` 接在 `FileRW_BarCode_Boot` 同一行（golden CreateForm 順序 HT9045.cpp:262）；owner gate `groundman.ini`；s12c_page_probe ALL PASS（16 項：R7／W7／L1／login1，改一筆只差 `Alarm_Continuous_Time` 15→20，G1 510→510 bytes）；執行期顯示與 RS232 起停鈕未做（S48→Jimmy）（CHANGES_20260926 §11.2） |
| ⚠ 🆕 | AOA offset（Main.AOAInfo.html，Gerneral.ini 38 鍵） | 過 | 送了但不生效 | `611edb7e`：golden 38 個輸入框全部 Visible=False，網頁照 golden 一樣改不到（頁面送的值進 `ack.ignored`）；只做語法檢查，未 build。兩題待 Steven 決定：①要不要開放網頁修改（偏離 golden）②開機沒開 AOA 時按 Save 會把 26 鍵寫成 0 怎麼防（CHANGES_20260926 §11.11） |
| ⏳ | 其他（TZteach、AOI、ProductionInfo、ContactForce…） | — | — | ⛔ 更正（20260926）：讀寫檔本體已翻好（`58bd9425` TZteach／ProductionInfo、`9ec84450` AOI、`21d37f2b` ContactForce），原「移植樹整檔不在」已過期；**都還沒有網頁**，見「二、逐結構清單」對應列與 `pending-pages.md`。⛔ 20260927 補：`CfgTrayPlate`（S98）、`ACTForm`（S108）、`Winway`（S109）、`Monitor`（S110）的讀寫本體（`217e7e5e`）也都沒有網頁（`pending-pages.md` 十六～十九；要不要現在建頁待 Steven，skill `ht9050-construction` `references/todo.md` ★ Q41） |

合計：有頁面的 C 路 13 頁＋StartCondition 全部讀寫都過（20260925 完整回歸 21/21、全量 build＋ctest 與 0924 基準相同）。
回歸套件：scratchpad `regress.sh`（`run_full.sh`＝鎖內全量 build＋ctest＋回歸）。已知 golden 行為差異的 `--allow` 寫在套件裡。

**🆕 更新（20260926）**：0925 晚間～0926 上午再接 9 頁（BarCode、TowerLight、ShuttleMove、Cleaning、QAMode、VacuumUnit、TesterIF、Status.Security Jam、Status.GroundMan），`tools/editlist/_integrated.txt` 目前共 24 個結構。回歸套件 20260925 收尾時擴大到 31 項全部通過（ChangeLog 20260925 §17／§20.3），20260926 又加 GroundMan 到 35 項（CHANGES_20260926 §11.2）；QAMode／VacuumUnit 仍各有一塊功能交給 Jimmy（見上表）。
**下拉 VCL 連動**（`cf530be8`）：gen_editlist 把 golden 方法本體的 `->Text=`／`->ItemIndex=` 改寫成 filerw::ELComboText／ELComboIndex（vclcompat TComboBox 只有欄位；超出範圍 -1＝Win32 CB_SETCURSEL）。新結構自動套用。

**引擎**：頁面載入兩份產生的接線檔時，以前每次 attach 都再掛一個存檔攔截器（按一次存跑兩次 golden 存檔），`d4d3214b` 起一頁只掛一個。

**客戶專屬條件先跳過、只註記（Steven 20260925）**：例如 BinSel 的 CC_SCC Off-Line 密碼（cBinSel.cpp:3016）、CC_ASE_CL Double Contact 權限 132、CC_Greatek 列名與 Control Bin 疊圖、labWarning 的 CC_JCET／TERAPOWER／SIGURD_PeiXing 例外。各頁遇到同類條件照此處理：不模擬，在交件報告與本表註記。

### 〇之二、Data.*.html 顯示頁（20260925 `008db55e` 時點）

Steven 20260925：讀寫檔工作完成後把全部頁面接上，遇問題依 BCB 自行決斷、在報告列出決斷。

| 頁面 | 狀態 | 做法／還差什麼 |
|---|---|---|
| ContactCT | ✅ `267425bc` | WS contactct.get，golden sgYieldDrawCell 逐格擷取；開機補 ReadCTInfo（golden main.cpp:10596）；Count Clear 未接 |
| CounterClear | ✅ `267425bc` | WS counterclear.get／click／exe，golden spbExeClick 一對 Clarn_Data |
| LotInfo | ⚠ `c8eb63a7` | Lot 分頁接 lot.* tag；其餘分頁顯示 `---`（要先用 dfm 重產頁面） |
| Observer | ✅ `f259d400` | WS observer.get（Timer1 1000 ms，acquire→get→release）；System Message＝golden EventLog CSV |
| SortCT | ✅ `008db55e` | 179 個 sort.* tag；Clear＝WS act.sortCT.clearCount |
| TestCategory | ✅ `008db55e` | tcat.* tag（golden sgArm1DrawCell 擷取）；測試流程 setter 解閘 |
| SmartDiagnostic | ✅ `008db55e` | WS smartdiag.op；Save／Reset 照 golden |
| Builder | ✅ `008db55e` | WS builder.op；建立／刪除／匯入／匯出（Steven 同意），另加移植樹守衛 P1–P5 |
| StartCondition | ✅ `e7e8f5f0` | C 路（見〇） |


---

## 一、寫檔的四種形狀（決定 bridge 怎麼做）

| 形狀 | 例 | 資料從哪來 | bridge 做法 |
|---|---|---|---|
| **A 表單存檔**（讀 widget → `WriteIniData`） | `TFTestIF::SaveSetupFile` | 畫面 | 產生器改寫 golden 原檔：widget → `FormState`；頁面送 JSON → `form.save` |
| **B 自由函式**（讀全域結構 → `WriteIniData`） | `SaveTestMode()` | 結構 | 本體**移植樹已翻、與 golden 一致**，不必改寫；要補的是 **golden 的觸發者**（多半是 `TfMain` 的按鈕處理器 → `act.*`） |
| **C HTEditList**（`el->SaveEditTextToFile`） | `elConfig` → config.ini | 畫面（`Add()` 綁的控制項） | `SaveEditTextToFile` 本體移植樹已翻（`Public/HTEditList.cpp`）；它讀控制項，要把 `Add()` 的註冊表接到 `FormState`（下一步設計） |
| **D 二進位**（`WriteData`／`WriteLastDataFile`／`SetLevelSet`） | `lastdata.dat` | 結構整塊 | 本體移植樹已翻；bridge 只負責「JSON → 結構欄位」再呼叫 golden 落地器；觸發者同 B |
| **E Gerneral.ini**（`WriteIniDataGeneral`，20260924 補：使用者「`WriteIniDataGeneral` 也加入設計 C++」） | `THandlerSystem::SaveSystemSet` | 畫面 | 同 A（多半是表單存檔）。golden 22 個函式／383 處，表⑧ 與本檔初版都漏算 |

### 一之二、C 類 HTEditList 的寫檔設計（Steven 20260924，含高級審查員第二輪意見）

使用者：「在 add 的時候都有決定鍵值，`SaveEditTextToFile(Path, FileName)` 只需要路徑就可以直接存檔」。所以**不重寫** golden 那 429 行：

1. **對照用執行期 `FEditList`，不用靜態抽的 Add 清單**。審查員實測 golden `elConfig` 1,593 筆中 **573 筆在 if／else／#if 內**（客戶碼）、**447 組 (區段,鍵) 重複**；移植樹 1,559 筆、**缺 12 個鍵**（`A77_EnableHanArmsInterlock`、`bA76AfterHomeNeedAlm`、`bA78EnableQAMode`、`bE79_InArmWaitShtLeftBeforePick`、`bD83NNAutoCleanTwoArmSimultaneous`、`bO21_1`／`bO25`、SECS GEM Alarm、`bF37NN`／`iF37NN`… → 請 JerryYang 補 Add）。
   每筆 `THTEdit` 的 `SourceControl` 指標 → 頁面 id：用產生器從**移植樹表單 header** 出的「成員名 → 指標」表（`gen_formjson.py` 那一套）反查。port 的 `TControl` 沒有 `Name`。
2. **存檔**：JSON 值填進各筆 `SourceControl`（TEdit→Text、CheckBox→Checked、Combo／RadioGroup→ItemIndex）→ 走 golden 的**存檔入口**（例 `TfConfiguration::SaveConfiguration`，含 `CheckConfigurationBeforeSave`、Lock by File、`SaveLastSetIni` 帶出的 `cbLastSet`／`elConfig_byRecipe`／`WriteLastDataFile`），不只呼叫 `el->SaveEditTextToFile`。
3. **拒寫要在呼叫之前**：golden `SaveEditTextToFile` **不是 all-or-nothing**（`bResult=false` 仍 `UpdateFile()`，且邊存邊寫回 `*iParameter`）。所以缺值／型別／Min-Max 的檢查在呼叫前用 `FEditList` 的 metadata 做；若要空跑到 %TEMP%，之後要 `ReadEditTextFromFile(真檔)` 把被改的參數還原。
4. `iTransformType≠0`（×100／×1000，elConfig 4 筆）頁面送**顯示單位**；重複 (區段,鍵) 只有第一筆生效（golden 語意），其餘填同值。

> **20260924 定案取代上面第 1 點的反查做法**（使用者：「CPP 端沒有元件，但是元件在 html 端」「參考 BCB 的，元件的部分改用名稱即可」「要針對有定義的結構變數來設計存檔與讀檔的功能」）：
>
> * **具名替身**：`tools/gen_editlist.py` 直接讀 golden 表單原檔（cp950），把 golden header 宣告的元件 `cbA01` 改寫成 `EL<TCheckBox>("TfConfiguration","cbA01")`（`FileRW/_EditList.h`）。替身是 vclcompat 物件，只存值；鍵是 (golden 表單類別, 名稱) —— golden 不同表單有同名元件（`TfConfiguration::cbP13` 寫 config.ini、`TfLd_ULd::cbP13` 寫 UdUld.Data），只用名稱會併成一個（審查 M3）。
> * `THTEdit::ControlName`（`Public/HTEdit.h`）在 `Add()` 時由名稱登記表填入＝HTML 元件 id。golden `ReadEditTextFromFile`／`SaveEditTextToFile`／`InitialDataToEdit` 本體**一行不改**。
> * **開機**照 golden：`main.cpp:1532-1543` new 各清單 → CreateForm 順序 `TfLd_ULd`（:191）先於 `TfConfiguration`（:207）→ 建構子 `cConfiguration.cpp:112-115`：`ReadLockByFile`→`InitConfigEdtList`→`ReadConfigStandard`（待辦）→**`ReadLastSetIni`**。最後這次是 golden 第一次有效讀 config.ini，必須在 `InitialHandler` 之前（審查 H1；Isaac 20191105 就是為此把它搬進建構子）。`cprod.cpp` 讀／寫三份清單的 GATE GA1-B2 已退役（經 `Public/HTEditList.cpp` 轉接函式，`cprod.cpp` 不能 include `HTEditList.h`）。
> * **讀**：`GET /api/editlist/<elConfig|cbLastSet|elConfig_byRecipe|elUdUld>` → `{id, form, group, key, content, readFromFile, visible, enabled, min, max, text|checked|itemIndex}`。實測 elConfig 748 筆（443 讀檔）、cbLastSet 32、elConfig_byRecipe 0（golden 20 筆全在客戶開關下，本機沒開）、elUdUld 12（只有 2 筆有名稱，其餘是移植樹 `fLd_ULd->Init()` 用真元件登記 → 待 elUdUld 自己的 cpp）。
> * **寫**：WS `editlist.save`，tag=`IniConfig`，value=`{"widgets":{名稱:{text|checked|itemIndex|position|dateTime|cells}},"answers":{"Config data save to define?":1}}`。走 golden `TfConfiguration::FormClose`（:5816）→`CheckConfigurationBeforeSave`→`SaveConfiguration`（`WriteLastDataFile`／`SaveLastSetIni`／`SaveTasterInfo`／`SetA73`）→`LoadConfiguration`，全由產生器從 golden 轉。
>   - 訊息框：`ShowMyMessage`→ack `messages`；`ShowMyMessageBox_YES_NO`→查 `answers`（1＝YES），沒答＝2＝NO 並列在 `asked`；`MyMessageBox->DoPassword_MBox()`→一律 false（不假裝驗過，列 todo）。
>   - 伺服器端接不上的 golden 段落（主畫面 UI、未移植門面）由產生器 `blocks` 包進 `#if 0 // GATE (S12-C save)`，執行時進 ack `todo`，共 15 段。
>   - **必送規則**：產生器掃出存檔流程「讀」的替身（`kIC_SaveReads`，102 個）。在某個 HTEditList 裡的，頁面沒送就沿用（值＝開機／上次存檔從檔案讀進的值，ack `kept` 列出）；**不在任何清單裡的**（值來自 golden `FormShow`，例 `tbD25_*->Position`、`dtO06_LastDate->Date`、`strngrdAutoSaveLog->Cells`）頁面沒送就 400 拒存 —— 否則會把 0 寫進 `LastSet.dIndexLoadRate` 等。實測空送 → 拒存列出 94 個，檔案 0 變動。
>   - ⏳ 下一步：轉 golden `TfConfiguration::FormShow`（顯示端，:4614）讓 GET 帶出那 94 個值；再做 G1（原值存檔 config.ini 位元組不變）與改一筆測試。
> * **`kIC_EnableAll` 的範圍（20260924 第六輪審查 M1）**：golden `FormShow :4619-4623` 的
>   `pcConfig->Pages[i]->ChangeCompomentEnabled(頁,true,true)` 迴圈明確限定在 `pcConfig`
>   底下、且排除 `tsSearchFunction`（`if(Pages[i]!=tsSearchFunction)`）。`tools/gen_editlist.py`
>   因此新增兩個設定：`enable_all_root='pcConfig'`、`enable_all_skip=['tsSearchFunction']`，
>   產生 `kIC_EnableAll[]` 時只收 `pcConfig` 子樹、跳過 `tsSearchFunction` 整棵。修正前
>   `kIC_EnableAll` 是全樹符合型別的元件都收，**1,131** 個；修正後 **1,109** 個，少掉的是
>   `pcConfig` 本身、外層 `PageControl1` 的其他頁（`tsConfig`／`tsHPData`／`tsSoftSimu`／
>   `tsTempComm`／`tsTrayData` 等）與整棵 `tsSearchFunction`。

### 一之三、D 類的做法（使用者 20260924：「可以用一個暫存的結構去接 JSON 的值」）

golden 觸發者（例 `TfStartCondition::sbSaveClick`）由產生器轉：它的「widget → `LastSet.xxx`」那幾行改成寫進**暫存結構**（`LAST_GENERAL_SET` 複本），全部成功後才覆蓋真 `LastSet`，再呼叫移植樹已完整翻好的 `WriteLastDataFile()`／`SetLevelSet()`。
⚠ lastdata.dat 前提未驗：`sizeof(LAST_GENERAL_SET)` 與欄位對齊要與 BCB6 相同（`file-io-mechanisms.md` §C.3）。

### 一之四、公式衍生欄位的存檔：移植公式＋存檔前重算，不要因為缺公式就整組拒存（20260925）

有些結構的部分欄位不是操作員直接輸入，是 golden 事件處理器依主輸入算出來、隨主輸入一起存檔的
（例：`DeviceForm_File`／`Contact.Data` 的 `Torque`、`Force Per Pin N/G`、`Double Force`）。
正確做法是**移植這些公式本身**，存檔前依 golden 事件相依順序重算一次衍生欄位，只在真的缺
公式所需來源表時才拒存——不是「看到算出來的值就整組拒存」，也不是信任頁面送上來的舊衍生值。
完整案例（`CalculateTotalAirForce`／`GetMaxIndexForceLimit`／`GetMinForce`／`CountDieForceKg`、
`DF_DeriveBeforeSave`）見 `generators.md` 十四；這是**通用規則**，加新結構時先問「golden 這個
表單的存檔鈕/`OnChange`有沒有在寫完主輸入後接著算別的欄位」，有就套這個模式。

### 一之五、golden `TfMain` 建構子裡「移植樹沒做的初始化」——轉表單前要查（20260925）

golden `TfMain` 建構子除了 `CreateForm` 之外，還會直接初始化不屬於任何單一表單的全域變數／
結構陣列（例：`Prod.iTrayType[]`、`GetMainAuth()`／`GetObserAuth()`、`EP_Install` 等）。這些
初始化被移植樹漏掉時，症狀常常出現在**別的地方**（開機、或另一個表單），不會在被漏掉的那個
表單本身報錯。加新表單時，先在 `main.cpp` 的 `TfMain` 建構子搜尋這個表單會用到的全域變數，
確認開機序列有沒有對應呼叫，**再**翻表單本體。已知案例與完整說明見 `generators.md` 十三。

---

## 二、逐結構清單

### `TestIF_File`（SYSTEM_TEST_IF，788 欄）

| 寫檔者 | 形狀 | golden | 寫哪 | 讀檔端（移植樹） | 狀態 |
|---|---|---|---|---|---|
| `TFTestIF::SaveSetupFile`＋`spbSaveClick` | ~~A~~ **C**（`8af13c07`；A 形狀 `f89be4ce` 退役） | `cTesterIF.cpp:337`／`:1320` | Tester.Data（RS232 模式另寫 `D:\RS232Standard\System\Setup.ini`） | ✅ C 路 `FileRW/TestIF_File_TesterIF.cpp` 直接轉 golden `ReadTestIFFile`（`cTesterIF.cpp:563`）：開機 `FileRW_TesterIF_BootReadTestIFFile`（`TestIF_File_TesterIF.cpp:268`）、換配方 `FileRW_TesterIF_ReadTestIFFile`（`:253`），呼叫點 `tools/wb_serve.cpp:3231`（移植樹 `forms/fTesterIF.cpp:800-1223` 那份仍在 GATE (F-5)，已不需要） | ✅ 讀寫通過（〇表 TesterIF 列）；`porting-gaps.md` 十四結案；A 形狀 bridge、`kBridge_TFTestIF`、ctest `FormBridgeTesterIF` 已退役（`f89be4ce`）；舊狀態移到 `archive/write-inventory_superseded.md` |
| `TfSetup::SaveSetupFile` | ~~A~~ **C**（20260925 改走，`16f463b8`） | `cSetUp.cpp:3655`（133 鍵） | HandlerCondition.Data（另寫 Contact／Temperature／configByRecipe） | ✅ `ReadFile` | ✅ 已完成（`cf530be8`）：讀、寫、換 Test Mode 三組 ALL PASS，見〇表 `SetUp` 列；舊狀態（頁面未接）已移到 `archive/write-inventory_superseded.md` |
| `TfYieldMonitoring::SaveSetupFile` | ~~A~~ **C**（20260925 改走，`16f463b8`） | `uYieldMonitoring.cpp:2033`（217 鍵） | Tester.Data、config.ini | ✅ | ✅ 完成並實測（`s12c_page_probe.py --write`）：改 `edLowYieldIg_FT` 只差 `Count` 一行、`G1` 位元組不變；可改 106／不可改 117／`mustSend` 170。探針鍵名前後空白正規化（`a9636d9c`）後 ALL PASS。舊 A 形狀 `tools/formbridge/TfYieldMonitoring.py` 已退役 |
| `TfAutoAlignment` 存檔（`spbSaveClick`） | C（`FileRW/TestIF_File_AutoAlignment.*`，前綴 AA） | `AutoAlignment/AutoAlignment.cpp:1573`（22 鍵；⛔ 舊句誤植 `:426`／11 鍵，已移到 `archive/write-inventory_superseded.md`；編譯進去的是 `AutoAlignment.cpp`，0925 稽核指的 `cAutoAlignment.cpp` 是死碼，不在 `HT9045.bpr`） | HandlerCondition.Data `[AutoAlignmrnt]` | ✅（開機讀＋`DoIniDataToForm` 已接，`c675594d`） | ⏳ 存檔函式已翻譯（`FileRW_AutoAlignment_spbSaveClick()`），**還沒有頁面／WS 指令觸發**（沒有 CCD 時 golden 在 `CheckAutoAlignmentEvent` 後就 return 不寫檔）；CCD 對位的 TCP 通訊（`:4664-5325`）移植樹沒有，交給 Jimmy |
| `elVacuumUnit->SaveEditTextToFile` | C | `VacuumUnit/VacuumUnit.cpp:369` | HandlerCondition.Data | ✅ | ⚠ 頁面已接（`8af13c07`）：golden 開機與換配方都不呼叫 `ReadFile`，照做維持不讀；硬體鈕（Set／Set All／吸破真空／Reset）網頁端先停用（S48→Jimmy）；`iVaccumThrd*` 有無讀者待確認 |
| `elLaser->SaveEditTextToFile` | C | `OmronLaser/LaserSensor.cpp:1116` | HandlerCondition.Data | ✅ | ⚠ 讀已接：頁面讀已於 `8af13c07` 接上，`c675594d` 補開機呼叫 golden `TfMain::FormShow :11189` 的 `ReadLaserFile`；寫（`SaveEditTextToFile`）已翻譯但沒有頁面觸發點；靜態初始化時看不到 `USE_LASER_DISTANCE`（那時 `Gerneral.ini` 還沒讀）——有雷射的機台永遠不開 COM，交給 Jimmy（`porting-gaps.md` 十九僅記筆電 IO 限制，這條另記在 ChangeLog §12.5） |

### `DeviceForm_File`（SYSTEM_DEVICE_FORM）

| `TfContact::SaveSetupFile`（含 `elContact`） | ~~A~~ **C**（20260925 起，`16f463b8`／`a9636d9c`） | `cContact.cpp:14313`（104 鍵）、`:14205` | Contact.Data、Position Offset.Data | ✅ | ✅ 讀寫通過（〇表 Contact 列：改 Force Per Pin Kg 時 4 個衍生鍵照 golden `edDieForcePerPinGChange` 連動，探針 `--derived`）。做法：`a9636d9c` 移植力量公式（`CalculateTotalAirForce`／`GetMaxIndexForceLimit`／`GetMinForce`／`CountDieForceKg` 與 DFM 事件處理器），存檔前 `DF_DeriveBeforeSave` 依 golden 事件順序重算衍生欄位，只在缺 `ContactForce` 表時才拒存（`generators.md` 十四）。原 A 形狀 `tools/formbridge/TfContact.py` 已退役（`b2aea32f`）；舊狀態（「寫入測試尚未跑完」）移到 `archive/write-inventory_superseded.md` |
|---|---|---|---|---|---|

### `Temperature`（SYSTEM_TEMPERATURE）

| 寫檔者 | 形狀 | golden | 寫哪 | 讀檔端 | 狀態 |
|---|---|---|---|---|---|
| `TfTemp_Set::SaveSetupFile`＋`spbSaveClick` | ~~A~~ **C**（`322d68a3`） | `uTemp_Set.cpp`（249 鍵） | Temperature.Data、Tester.Data | ✅ `ReadTempFile`；ATC.ini 已讀（`W906_ReadATCIni`，`tools/wb_serve.cpp:3123`，`porting-gaps.md` 十三已結案） | ✅ 讀寫通過（〇表 Temp_Set 列，`bc935659`）。做法：固定元件用具名替身、執行期動態面板 `myTempPal[tcTotalCount]` JSON 直接交換（`generators.md` 十五）；開機接法見 skill `ht9045-html-json` `route-c-golden-bridge.md` §6 #13。之後補的：S88 關窗尾段 `MainTempOffsetTail`（`FileRW/Temperature.cpp:70`，`generators.md` 十九）、`ATKRecipeInfo` NULL 保護（`cfb5735a`，`generators.md` 廿二）。舊狀態（「進行中、未整合」與整合步驟）移到 `archive/write-inventory_superseded.md` |
| `SaveTempMode` | B | `cprod.cpp:3645` | Temperature.Data `[Mode] WorkTempMode`（LastSet.iTemperature） | — | ✅ 本體一致、觸發者 `TfMain::ChangeTempMode` 已接 |
| `SaveTempModeByDLL` | B | `cprod.cpp:3657` | Temperature.Data `[Mode] Mode` | — | ✅ 同上 |

### `TestMode`（SYSTEM_TEST_MODE）

| `SaveTestMode` | B | `cprod.cpp:3314`（27 鍵） | TestMode.Data | — | ✅ 本體一致；S88（commit `295bc768`，⛔ 更正取代下面「只接 2 個」的舊敘述，原句移到 `archive/write-inventory_superseded.md`）逐一對照 golden 16 個呼叫點：已接 9 個——`W906_DoReadLastData`（`wb_serve.cpp:3138`／`:3147`／`:3156`）、`ChangeSetUpFile`（`WebRecipeChange.cpp:320`／`:330`）、`ChangeTempMode`（`MainTempMode.cpp:172`）、`LastSetUseTestSocketToTestModeDutOnOff`（`cprod.cpp:1682`）、`sbTempOffsetClick` 尾段（`FileRW/Temperature.cpp:84`／`:94`，S88 這次補上，觸發時機見待 Steven 決定 Q1）；`TfBuilder::bSaveAllFillOrFile` golden 本身走不到（唯一呼叫者固定傳 "Temperature"），不用做；⛔ 20260926 更新：`ChangeTesterConnect` 已由 Steven02 移植（`forms/fMain.cpp:1096`，`SaveTestMode()` 在 `:1233`；`979eac6b`，併入 main `7f332938`）⇒ 已接 **10** 個；剩 `ShowTestHeadComp1`、`RunICModeChange`、`DoTrayFeedProcess`×3（S79）交 Jimmy，詳見 `scratchpad\frw_s88\report.md` 第 1 節（舊句移到 `archive/write-inventory_superseded.md`） |
|---|---|---|---|---|---|

### `IniConfig`（config.ini）

| 寫檔者 | 形狀 | golden | 寫哪 | 狀態 |
|---|---|---|---|---|
| `SaveLastSetIni` → `elConfig`（1,593 筆 `Add()`）、`cbLastSet`（91 筆）、`elConfig_byRecipe`（20 筆） | C | `cprod.cpp:3122`／`:3139`／`:3146` | config.ini、LastSet.ini、configByRecipe.ini | ✅ 讀寫通過（〇表 Configuration 列）：`FileRW/IniConfig.cpp`＋`IniConfig.gen.inc`；`editlist.get`＝golden `FormShow`（`IniConfig.cpp:226-257` `IniConfigPageJson` 先跑 `IC_FormShow`）、`editlist.save`＝golden `FormClose` 全流程；頁面 `Config.Configuration.html` 在 `GOLDEN_BRIDGE`（`web/page/ht9045_wire_engine.js:1039`），B 路 `system.file.put` 寫這三個檔一律 409（`CRouteOwner`）。舊狀態（缺 FormShow、頁面未改接）移到 `archive/write-inventory_superseded.md` |
| `TfMain::cbSetupFileNameChange` → `elConfig` | C | `main.cpp:25351`（函式，主 repo V912）；寫 `elConfig` 的是 `:25361-25375` | config.ini | — 客戶專屬：那一段只在 `CUSTOMER_CODE==CC_SCK` 才跑（勾 `[Monitor]` 再 `SaveEditTextToFile`），`WebRecipeChange.cpp:398-399` 已標 `Cust(...)`，依 S25 不做（舊句「⏳ `main.cpp` 整支不在移植樹」移到 archive） |
| `SaveTasterInfo` | B | `cprod.cpp:3229`（7 鍵，只在 `CosFunction.bFTPFunction` 時寫） | config.ini `[Taster]` | ✅ 本體一致；S89（commit `295bc768`，⛔ 更正取代「0 處接上」的舊敘述，原句移到 `archive/write-inventory_superseded.md`）：`TfConfiguration::SaveConfiguration` 已接（`FileRW/IniConfig.gen.inc:8253`）；`TfFTPClient::btSafeTasterNameClick` 客戶專屬（`bFTPFunction`），整支 `TfFTPClient` 表單不在移植樹，依 S25／S84 不做 |
| `SaveEventLogAutoSaveInfo` | B | `cprod.cpp:3177`（只在 `bEventLogAutoSaveFunction` 時寫 2 鍵） | config.ini `[Event Log]` | ✅ 本體一致（`FormatString`→`FormatDateTime` 等價）；S89（commit `295bc768`）：`TfConfiguration::FormClose` 已接（`FileRW/IniConfig.gen.inc:7672`）；`TfObserver::bAutoSaveEventLog` 移植樹 4 處仍在 `#if 0`（C-log-5a～d），但 golden 本身也沒有活的呼叫者——唯一呼叫點 `btAutoSaveClick :3175` 早被 JerryYang 20161117 註解掉，golden 也是死碼，不用解閘 |
| `SaveRmsInfo` | B | `cprod.cpp:3150`（寫 `[Server]`，CC_SCC／CC_SCK 另寫 `[RMS]`，鍵是 Product Name／Product Temp；⛔ 更正：舊句「`[Product*]`」寫錯，原句移到 `archive/write-inventory_superseded.md`） | config.ini `[Server]`／`[RMS]` | ⏳ 本體一致；觸發者 `TfMain::FormClose`（條件 `bShowLotInfo && HasICUnderMachine()`）未接——S89（commit `295bc768`）待 Steven 決定（Q2）：建議與 S65 Q2 關站收尾一起做，且要連同開機讀回 `fLotInfo->ResetLotInfo()`（golden `main.cpp:10989`，目前在 `forms/fLotInfo.cpp:3108`／`:3208` 的 WC-1 `#if 0` 裡）一起接，只接寫沒有意義。⛔ 20260927 補：觸發者已接——`FileRW/MainClose.cpp` 關站段照 golden `FormClose :12051-12057` 呼叫（S95R，`bd40ffcb`）；開機讀回 `ResetLotInfo`（golden `main.cpp:10991`）也接了（GATE WC-1 退役，`61c96910`：`FileRW/MainBoot.cpp` `W906_FRWBoot_ResetLotInfo`，`tools/wb_serve.cpp:4162`，另有 Lot Info 開頁兩處），所以 `bd40ffcb` 那道「兩個值都空就不寫」的守衛已關掉（`s_bRmsBlankGuard=false`，todo ★ R49）。未 build、未實跑 |

### `LastSet`（LAST_GENERAL_SET，lastdata.dat）

| `WriteLastDataFile` | D | `cprod.cpp:1910` | lastdata.dat＋backup＋backup2 | ✅ 本體已翻（`cprod.cpp:2001`）；golden 20+ 個觸發者已逐一對照完成，見 **S65**（commit `1ba00a68`／`c317ca30`，23 個活呼叫點逐一對照、10 處已接；⛔ 更正取代「⏳ 逐一對 live 狀態」的舊敘述，原句移到 `archive/write-inventory_superseded.md`）；細節不在本列重複，見 ChangeLog `CHANGES_20260926_Steven.md` §11.25 與 `scratchpad\frw_s65\audit.md` |
|---|---|---|---|---|

### `LevelSet`（LAST_LEVEL_SET，levelset.dat）

| `SetLevelSet` | D | `cSecurity.cpp:1511` | levelset.dat | ✅ 已完成（commit `8c5ea501`，S64）：`system.levels.put` 改呼叫新檔 `WebLevelSet.cpp` 的 `W906_LevelSetPut`，照 golden `TfSecurity::FormClose` 全流程（`Insufficient(29)`→`GetLevelSet`→驗證→套值→三條鉗制`[87]`／`[129]`／`[86]`→備份→`SaveJamLevel`→`SetLevelSet` 整塊 1024 bytes→重讀逐位元組比對）；`cSecurity.cpp` SEC-W1／SEC-W2 兩個 GATE 已解開；未 build。SECS S125F4（GATE [L1]）交給 Jimmy |
|---|---|---|---|---|

### 其餘結構

| 結構 | 寫檔者 | 形狀 | golden | 寫哪 | 狀態 |
|---|---|---|---|---|---|
| ArmSpeed 族（ArmCondition.Data） | `TfSpeed::SaveSetupFile` | ~~A~~ **C**（20260924 深夜改走） | `cSpeed.cpp:2250`（110 鍵）；golden 存檔鈕 `spbSaveClick` **不**呼叫 `SaveSetupFile`，自己逐鍵寫 139 處 | ArmCondition.Data | ✅ 完成並實測（`s12c_page_probe.py --write`）：可改 117／不可改 109、`mustSend` 130；`G1` 位元組不變；A 形狀 `tools/formbridge/TfSpeed.py` 退役到 `_retired/`；`InArmSuck`／`OutArmSuck`（`TList` 重複定義）經新檔 `FileRW/_KitSuck.cpp` 轉接；詳見 `generators.md` §62、十 |
| `UserDefForm_File`（Tray.Data） | `TfTrayForm::SaveSetupFile`（含 `elTrayForm`） | A＋C | `cTrayForm.cpp:649` | Tray.Data | ✅ 讀寫通過（〇表 TrayForm 列，頁面 `Setup.TrayForm.html`）：`tools/editlist/UserDefForm_File.py` → `FileRW/UserDefForm_File.cpp`／`.gen.inc`；開機與換配方讀 `FileRW_TrayForm_Boot()`／`FileRW_TrayForm_ReadFile()`（`tools/wb_serve.cpp:3184-3185`）。舊狀態（「驗收待確認」）移到 `archive/write-inventory_superseded.md` |
| Tray Assignment（Tray.Data） | `TfTrayAssignment::SaveSetupFile` | ~~A~~ **C**（20260925 完成整合，`16f463b8`） | `cTrayAssignment.cpp:1409`（86 鍵） | Tray.Data | ⚠ `tools/editlist/TrayForm.py`、`FileRW/TrayForm.cpp`／`.gen.inc` 已整合。整合時發現既有缺陷：golden `TfMain` 建構子 `main.cpp:1414-1497` 的 `Prod.iTrayType[]` 預設值移植樹從沒做過（全落 `tNotUse`，golden `ReadFile` 因此跳過 Auto／Fix 區段——**開機與每次 `SetWorkParameter` 都受影響**，不只影響本頁），已補 `FileRW_TrayAssignment_InitProdTrayType`（見 `generators.md` 十三）。⛔ 20260926 更正：已讀寫通過（〇表 TrayAssignment 列——開機／reload／`SetWorkParameter` 讀檔全部改走 golden 912 版，`tools/wb_serve.cpp:4057-4058` 的 `g_W906_TrayAssignmentReadFileHook`）；舊句「仍剩 `Fix3/Direction` 1→0 查證中，不要標成完全通過」移到 `archive/write-inventory_superseded.md`。原 A 形狀 bridge（`tools/formbridge/TfTrayAssignment.py`、舊版 `FileRW/TrayForm.cpp`）已退役（commit `b2aea32f`，第八輪審查 H-2） |
| `HotPlateForm_File` | `TfHotPlate`：`FormShow`＋`DoIniDataToForm`＋`spbSaveClick`＋`SaveSetupFile` | A | `cHotPlate.cpp:35`／`:310`／`:440`／`:480`（11 鍵） | HotPlate.Data | ✅ 20260924 `WriteFile/HotPlateForm_File.cpp`：G1（原值存檔 SHA256 不變）、改 XST1 只差一行、缺值空跑拒寫、兩盤都不勾 `saved:false`；頁面 probe 過；Todo：`fMain->ChangeATCSiteUse()` 無本體、`BackupSetupFile()` 空函式、Plate CSV 選項 |
| `Ld_UldDelayTime`（UdUld.Data） | `elUdUld->SaveEditTextToFile` | C | `cLd_ULd.cpp:213` | UdUld.Data | ✅ 完成（20260924 深夜）：`tools/gen_editlist.py` 第二個 C 形狀結構，共用 `FileRW/_EditPage.h`；開機經 `W906_LdUldInitOnce()`（`cSpeed.cpp`）接到 `FileRW_IniConfig_Boot()` 之前（golden `CreateForm` 順序 `TfLd_ULd` 先於 `TfConfiguration`）；GATE (SEC1) 查表半解閘（見 `generators.md` 四）後 `editable` 才不會整頁 false；四頁回歸（含本結構）`--write` **ALL PASS**（`generators.md` §66）。詳見 `generators.md` |
| Offset 族（`*ArmOffSet_File`、`Offset_File`） | `TfOffSet::SaveSetupFile` | ~~A~~ **C**（`322d68a3`） | `cOffSet.cpp:1470`（48 鍵） | Position Offset.Data（Hot／cool 變體） | ✅ 讀寫通過（〇表 OffSet 列，`e086de38`）：`tools/editlist/Offset_File.py` → `FileRW/Offset_File.cpp`，專用入口 `FileRW_Offset_Page`／`FileRW_Offset_Save`（`tools/wb_serve.cpp:5212`／`:5323`，`widgets={offsets,common}`）；做法「開頁對每個選取跑一次 golden 按鈕事件收成整包、存檔逐組跑 `spbSaveClick` 的 SaveFile 段」（`generators.md` 十五）；A 形狀 `TfOffSet.py` 已退役。舊狀態（「暫留 A 形狀、未整合」）移到 `archive/write-inventory_superseded.md` |
| 同上 | `TZteach::SaveSetupFile` | B（讀結構） | `AutoTeach/InOutArmZteach.cpp:4455`（11 鍵） | Position Offset.Data | ✅ 已完成（S68，commit `58bd9425`，未 build）：只有自動教導 Z 機台流程會叫，待 Jimmy 解 GATE（見 `pending-pages.md` 十二） |
| `Tech`（tech.dat） | `TfTeach::SaveFile`（`WriteData` 整塊＋`elTeach`） | D＋C | `uteach.cpp:4482`／`:4501` | tech.dat、teach.ini | ⚠ C 路 `FileRW/Teach.cpp`（`ad7561d4`，產生器 `gen_teach_editlist.py` 讀 golden **V906** 樹，`Teach.cpp` 註解引 V906 `uteach.cpp:4924-4949`）：`IC_SaveFile`（`Teach.cpp:147`）寫 teach.ini（各 `TECH_*`＋`elTeach`＋背隙）；**tech.dat 刻意不寫**、回報 `ELTodo`——`TECH` 版面依版本不同（V899 3792／906 3872／V912 兩欄搬到結尾，`Teach.cpp:149-153`），待使用者裁決（S75 交 Jimmy）。頁面 `HW.teach.html` 在 `GOLDEN_BRIDGE`。舊句「⏳ 移植樹無此函式」移到 `archive/write-inventory_superseded.md` |
| 同上 | `TfSmartSetup::SaveZCalibrationPosition`／`SavePosition` | D | `AutoAlignment/SmartSetup.cpp:1468`／`:1501` | tech.dat | ⏳ |
| `ESD_GENERAL` | `WriteESDDataFile` | B | `csystem.cpp:23482` | ESD 設定檔 | ⚠ 本體一致（移植樹 `csystem.cpp:27145`）；觸發者 `HT9045Gem::S2F15_UpdateNewEquipmentConstant` **本身是活的**（`SECSGEM/uHGemHT9045.cpp:3249`，由 `uHGemClass.cpp:1670` 呼叫），閘住的是它裡面呼叫 `WriteESDDataFile()` 那一段（`#if 0` GATE [E8]，註解 `:3700` 起、呼叫在 `:3715`）；閘上寫的理由「defined NOWHERE」（`:3702`）已過期——解閘屬 SECS 範圍（Jimmy）。舊句「觸發者在 `#if 0`」移到 `archive/write-inventory_superseded.md` |
| ContactForce（SLK 表） | `TfContactForce::WriteFile` | ~~A~~ **C**（S57，20260926） | `ContactForce.cpp:1183`（22 呼叫） | ContactForce 檔 | ✅ 已完成（commit `21d37f2b`，未 build）：`SLKClass` 容器缺口已補（`ContactForce.h`／`ContactForceLoad.cpp`），`FileRW/ContactForce.cpp`／`ContactForce_Panels.h`；沒有網頁，見 `pending-pages.md` 十五 |
| CCLink | `TfCCLink::SaveSetupFile` | B（讀值） | `CCLink/MyCCLinkSensor.cpp:648` | CCLink.Data | ⏳ |
| `MachRec` | `SaveMachineRecord` | D | `cinitial.cpp:7975`／`:7980` | machinerecord*.dat | ✅ 本體已翻（`cinitial.cpp:16965`）；`cinitial.cpp` 的呼叫點 n2-15／n2-16／n2-22 已解閘（`cinitial.cpp:7632-7639` 註記 `LIFTED 20260925 (W3-12c)`）。golden 關程式 `TfMain::FormClose` 那一處沒接（關站收尾另案，S65 Q2 待 Steven）。舊句「⏳ 對照」移到 `archive/write-inventory_superseded.md` |
| AOI 參數 | `TFrmAOI::spbSaveClick` → `elParameter` | C | `fAOI.cpp:3335` | AOI.Data | ✅ 已完成（S69，commit `9ec84450`／`e6a8e0fc`，未 build）：`FileRW/AOISetup.cpp`；`[OCR SETTING]` 另見下一列 S86 |
| AOI `[OCR SETTING]` | `TfOCR::spbSaveClick` | C | `OCR.cpp:2151`（19 鍵） | AOI.Data | ✅ 已完成（S86，commit `973f2540`，未 build）：`FileRW/IniConfig_OCR.cpp`，讀檔已接開機／換配方鏈；沒有網頁，見 `pending-pages.md` 十四 |
| AutoCalSuckZ | `TfProductionInfo::SaveAutoCalSuckZ` → `elData` | C | `ProductionInfo/ProductionInfo.cpp:6060` | AutoCalSuckZ.Data | ✅ 已完成（S70，commit `58bd9425`，未 build）：golden 本身也叫不到，沒有網頁 |
| Unloader Clip | `TfBarCode::WriteUnloaderClipIni` → `elUnloaderClip` | C | `BarCode/BarCode.cpp:9334` | UnloaderClip ini | ⏳ |
| Configuration 頁 Tray／HP 表（S98） | `TfConfiguration::sbUpdateTrayClick`／`sbUpdateHPClick`（讀：`sbtReloadTrayClick`／`sbtReloadHPClick`） | A（讀格子 → `TStringList::SaveToFile`，CSV） | `cConfiguration.cpp:7012`／`:7194`（讀 `:7037`／`:7148`） | `System\TrayForm.csv`、`System\PlateForm.csv` | ⚠ 讀寫本體已完成（`217e7e5e`，手寫 `FileRW/CfgTrayPlate.cpp`，未 build）：讀者 TrayForm／Cleaning（C 路，`217e7e5e` 解閘）、HotPlate（A 形狀，`7d490f7c`）開頁照 golden 讀；寫（兩顆 Update）沒有呼叫者——`Config.Configuration.html` 的兩個分頁只是 DFM 產生的外殼，`editlist.get IniConfig` 不帶表格（`FileRW/IniConfig.gen.inc` 沒有 `strngrdTray`／`strngrdHP` 替身）。golden 怪處照翻：HP 存完重讀的是 Tray 表（主 repo V912 `cConfiguration.cpp:7215-7216`，todo ★ R33）、存一次再讀第 16 欄會掉（R34）；空欄位照 BCB6 `CommaText` 不加引號（`W906_Bcb6CommaText`，R35）。建頁備忘 `pending-pages.md` 十六 |
| `ACTForm`（S108） | `TACTForm::btnUpdateClick` → `SaveACTData`／`SaveCommData` | A（用 C 形狀產生器，`tools/editlist/ACTForm.py`） | `AutoTemperature.cpp:692`（`SaveACTData` `:834` 起） | `System\AutoTemperature.ini` `[ACT]`／`[Display]`／`[COMPort]` | ⚠ 讀寫本體已完成（`217e7e5e`，`FileRW/ACTForm.*`，未 build）：開機照 golden **不讀**（建構子讀檔時 `FilePath` 還是空的，todo ★ R37）；開頁讀、存檔寫的函式都沒有呼叫者（沒有頁面）；自動 K 溫、COM、TCP 交 Jimmy。建頁備忘 `pending-pages.md` 十七 |
| `Winway`（S109） | `TfWinway` 建構子（每次開機）／`btnUpdateClick` → `SaveCommData` | A（用 C 形狀產生器，`tools/editlist/Winway.py`） | `ATC/WinWaySetting.cpp:12`／`:78`（`LoadCommData` `:116`、`SaveCommData` `:135`） | `config\ATCWinWay.ini` `[COMPort1..4]` | ⚠ 讀寫本體已完成（`217e7e5e`，`FileRW/Winway.*`，未 build）：**每次開機讀並原樣寫回**（golden 同，todo ★ R36）；Update 鈕沒有呼叫者；COM／Modbus 交 Jimmy。建頁備忘 `pending-pages.md` 十八 |
| `Monitor`（S110） | `TfMonitor::sbMVUpdateClick` → `SaveTCPIPParament` | A（用 C 形狀產生器，`tools/editlist/Monitor.py`） | `Monitor/MonitorInterface.cpp:123`／`:98`（讀 `LoadTCPIPParament` `:79`） | `system\MVData.ini` `[Setup]`／`[Specific]` 7 鍵 | ⚠ 讀寫本體已完成（`217e7e5e`，`FileRW/Monitor.*`，未 build）：開機照 golden 建構子 `:32` 只讀（檔不在不建）；存檔沒有呼叫者；MVCtrl（TCP）、HD 容量警報、錄影交 Jimmy。建頁備忘 `pending-pages.md` 十九 |

### Gerneral.ini（形狀 E，`WriteIniDataGeneral`；20260924 補）

| 寫檔者 | golden | 處數 | 頁面 | 狀態 |
|---|---|---|---|---|
| `THandlerSystem::SaveSystemSet` | `HandlerSys.cpp:579` | 277 | HW.HandlerSys | ✅ 已完成（`cf530be8`）：讀寫 ALL PASS，見〇表 `HandlerSys` 列；舊狀態（進行中、未整合）已移到 `archive/write-inventory_superseded.md` |
| `TfMain::OffsetSaveClick` | `main.cpp:34961` | 38 | Main.AOAInfo.html | ✅ 已完成（`611edb7e`，C 形狀 `FileRW/AOAOffset.*`）：golden 38 個輸入框全部 `Visible=False`，網頁照 golden 一樣改不到（頁面送值進 `ack.ignored`）；只做語法檢查、未 build。開機沒開 AOA 時按 Save 會把鍵寫成 0（golden 既有地雷）——兩題都待 Steven 決定，見 ChangeLog `CHANGES_20260926_Steven.md` §11.11／§12.4 |
| `SYSTEM_MODULAR::ReadGeneralIni`（讀時補鍵） | `database.cpp:233` | 16 | — | 讀檔端，已翻 |
| `TfMain::TfMain`（建構子 12 鍵＋`ZSafePos`） | `main.cpp:1743-2156` 一帶（V912 行號，906 內容相同） | 12 | — | ✅ 已完成（Jimmy `212c8e1d`／`c55e2954`；`FileRW/HSys.cpp` 20260926 加 11 行註解逐句對過 V912，見 ChangeLog §11.8）。`INDEX_SUCKER_TYPE` 仍閘著（半解，見 `porting-gaps.md` 十七） |
| `TfMain::FormShow`（開機三小塊，不屬於任何單一表單） | `main.cpp:9609`／`:9779-9782`／`:11576-11580` | 3 | — | ✅ 已完成（`4dee7107`，新檔 `FileRW/MainBoot.cpp`）：`BinCount.txt` 讀回（寫點仍在 SEAM S2 空替身，見 `porting-gaps.md` 十八）、`[Version] Ver` 戳記每次開機都寫、JamRawData `ReadLoaderCount`／`StatisticalJamCount`（後者在 `slEventLog` 為 NULL 時跳過，見 `porting-gaps.md` 十五）。只做語法檢查、未 build。⛔ 20260927 補：同檔又接了 S91（DailyJamRate／`CriticalParaControl.ini`／`GetLimitAuth`，`1d68d518`）、S94（`LotData.txt`／`TrayIDByLot.txt` 讀回，`26d0b3f8`）、WC-1（`ResetLotInfo`，`61c96910`），見下方「`TfMain` 開機／關程式／計時器的手寫 FileRW」 |
| `TfMain::FormShow`／`FormClose`／`Start`／`stOperatorClick`／`sbPEModelClick`（其餘） | `main.cpp` | 2＋1＋1＋2＋1 | — | ⏳（上一行的 3 小塊已完成，這裡是剩下還沒動的部分；`main.cpp` 整支不在移植樹，見 `porting-gaps.md` 一）。⛔ 20260926 補：`sbPEModelClick` 已做（`FileRW/MainClick.cpp` `W906_Main_PEModelOp`，`c913d5e5`；WS `act.main.peModel` 分派 `tools/wb_serve.cpp:4826`，`a83d7f22`）；`porting-gaps.md` 一「`main.cpp` 整支不存在」已部分過期（見該節）。⛔ 20260927 補：`FormClose` 的生產資料存檔與關站順序、Exit 鈕 `sbCloseProgramClick` 已做（`FileRW/MainClose.cpp`，S95 `6905f8eb`／S121 `a684f171`／S95R `bd40ffcb`），見下方同名小節 |
| `TCOM2` 兩鍵 | `main.cpp` 附近（golden `TCOM2` 相關讀檔） | 2 | — | ✅ 已完成（Jimmy `c55e2954`；比對 V912 逐行相同，見 ChangeLog §11.8） |
| `TfContactForce::WriteFile` | `ContactForce.cpp:1357` | 9 | — | ✅ 已完成（S57，commit `21d37f2b`；EP 四鍵與 HSys 重疊已於 S90／`f1ad780c` 解決，見 `pending-pages.md` 十五） |
| `SaveLastSetIni`／`ReadLastSetIni` | `cprod.cpp:3094`／`:3056` | 4＋1 | — | 已翻（C 類一起做） |
| `TfTrayMapping::SaveFile_ScanLine` | `cTrayMapping.cpp:1946` | 4 | — | ⏳ |
| `THandlerSystem::rgHeaterTypeClick`、`TfTeach::btnSaveClick`、`ChangePassword`、`DoTrayFeedProcess`、`TFormHS::CheckMTKFileName`、`TfFTPClient::plSLoadClick`、`InitialGaliDelayCount`、`TASESendMessage::*` | 各處 | 1～3 | — | 部分完成（20260926 核對；舊句「⏳」移到 archive）：`rgHeaterTypeClick` 在 `FileRW/HSys.cpp` `BeforeApply` 存檔前重播（`:792-815`，P8 `c913d5e5`）；`TfTeach::btnSaveClick` 在 `FileRW/Teach.cpp`（`editlist.save tag=Teach`）；`InitialGaliDelayCount` 本體在 `Motor/myGALILmotor.cpp:1042` 但沒有呼叫者（golden 由 `TfMain::FormShow` 主 repo `main.cpp:9884` 呼叫，Jimmy）；`DoTrayFeedProcess`＝S79（Jimmy）；`ChangePassword` 移植樹是只累加計數的空殼（`forms/fMain.cpp:512`，Jimmy）；`CheckMTKFileName`／`plSLoadClick`／`TASESendMessage::*` 客戶專屬（S25） |

### `TfMain` 開機／關程式／計時器的手寫 FileRW（20260927 補，St01；全部只編進 `wb_serve`，`CMakeLists.txt:3404`）

不屬於任何一個 golden 表單、也不走產生器的 `TfMain` 小段，逐支手寫在 `FileRW/Main*.cpp`（golden 是主 repo V912 `main.cpp`；
呼叫點是 `tools/wb_serve.cpp` HEAD 227b79db 的行號；**全部只做語法檢查、沒有 build、沒有實跑**）。「台帳」是 skill `ht9050-construction`
`references/todo.md` 的 A～J 編號，「★」是同檔「待 Steven 決定」的題號。forms 門面怎麼叫到這些只編進 `wb_serve` 的本體：`generators.md` 廿七。

| 檔／函式 | golden | 讀／寫哪些真實檔 | 呼叫點 | commit | 台帳／★ |
|---|---|---|---|---|---|
| `MainBoot.cpp` `W906_FRWBoot_JamRateByDayRead` | `TfMain::FormShow :9607` → `RUN_INFO::ReadJamRateByDay`（`cprod.cpp:1112-1169`，逐行抄） | 讀 `D:\HT9045_Log\JamRate_Daily\` 當天那一份 | `:3864` | `1d68d518` | D-006 |
| `MainBoot.cpp` `W906_FRWBoot_CriticalParaAuth`／`W906_FRWBoot_LimitAuth` | `:9787` `GetCriticalParaAuth`／`:9786` `GetLimitAuth` | 讀 `config\CriticalParaControl.ini`（缺鍵才補寫；`1d68d518` 本文：這台 26 鍵齊）；`InputLimit`（以前開機是 0，溫度 offset 在記憶體被鉗成 0，直到開溫度頁） | `:3886` | `1d68d518` | D-006 |
| `MainBoot.cpp` `W906_FRWBoot_LotListsRead` | `:11344-11365` | 讀 `LotData.txt`（`BAR_CODE_INSTALL` 2～5）、`TrayIDByLot.txt`（`bCheckTrayIDBylot`）；只開機，換配方不重讀 | `:4163` | `26d0b3f8` | G-011／R10 |
| `MainBoot.cpp` `W906_FRWBoot_ResetLotInfo` | `:10991` `fLotInfo->ResetLotInfo()`（本體 `uLotInfo.cpp:14820-14834`） | 有 IC 且 `bShowLotInfo`：讀 `config.ini [Server]`／`[RMS]` Product Name／Temp（缺鍵補寫空字串，golden 同）；`LastSet.bHasDownloadFile=true`（只改記憶體） | `:4162` | `61c96910`（GATE WC-1 退役） | R49 |
| `MainBackup.cpp` `W906_TfMain_BackupSetupFile` | `TfMain::BackupSetupFile`（`main.cpp:34053-34127`） | 每次 C 路存檔後：刪目前配方夾全部 `*.MD5`、寫一個新的 `<32 hex>.MD5`；`bUseAutoBackUpSetupFile` 時複製到 `D:\HT9045_Backup\SetupFile`；CC_ASE_SG 整個刪除重建 `D:\SetupFile\`（只在客戶碼 930） | 安裝 `:4111`（`forms/fMain.cpp:458` 安裝座） | `4c5d7a26` | G-009／R13、R14 |
| `MainClose.cpp` `W906_Main_CloseProgramOp` | `sbCloseProgramClick :29051-29131`（golden 兩個確認框，網頁分三步） | 每一步寫哪些檔列在 `FileRW/MainClose.cpp` 檔頭 `:28-55`（`BinCount.txt` 只有這條路寫，golden `:29129`） | WS `act.main.closeProgram` `:4826` | `6905f8eb`／`bd40ffcb` | D-007／R26、R27 |
| `MainClose.cpp` `W906_ProdCloseSave`／`W906_ProdCloseShutdown` | `FormClose :11852` 起（存檔 `:11861`／`:11862`／`:11863`／`:11864`／`:11919`／`:12195`；關站 `:11881-12474`） | lastdata*、`config.ini`（`WriteLastDataFile` 尾段）、machinerecord、`RunMode.txt`、`LotSummary.csv`、DailyJamRate、`Arm*.dat` 等 18 檔、有 IC 時 `config.ini [Server]`（`SaveRmsInfo`）；停馬達／鎖煞車／關加熱器繼電器、風扇、蜂鳴器 | `:5964`（主迴圈結束後、`Program Close=1` 之前）；結束旗標 `:5953` | `6905f8eb`／`a684f171`／`bd40ffcb` | D-007／R25、R38～R41、R49～R51 |
| `MainClose.cpp` `W906_TfMain_SaveRunMode` | `TfMain::SaveRunMode`（`main.cpp:33648-33658`） | `d:\HT9045\system\RunMode.txt` 一行 `RunMode=%d` | 安裝 `:4111`（`forms/fMain.cpp:458`）；裝上之後 `RunStartMode.cpp:883` 每次 `SetRunStartMode` 都寫 | `bd40ffcb` | D-007／R50、R51 |
| `MainRecord.cpp` `W906_MainRecordTimer1Tick`／`W906_MainRunInfoTimer2Tick` | `UpdateRecordScreen`（`main.cpp:8584-8697`）／`UpdateRunInfo`（`:22740-22805`） | 只改記憶體（`LastSet.SystemAccSecond`、`RunInfo.iYieldChart`／MTBA／MUBA）；值由既有的 `WriteLastDataFile` 寫出 | 每拍 `:5953`；開機起點 `W906_MainRecordBootLatch` `:3864` | `0b38b6b5`／`6db687d4` | D-009／R42～R47 |
| `CfgTrayPlate.cpp` | 見上表「Configuration 頁 Tray／HP 表」列 | `System\TrayForm.csv`／`PlateForm.csv` | `:4052`、`:4066` | `217e7e5e`／`7d490f7c` | G-014／R33～R35、Q40 |

**不列入**：`TempCtrl/MyTempture_*.cpp` 的 `WriteData(Addr, reg, v)` 是溫控器 Modbus 暫存器寫入，不是檔案；`ASE_K Socket/aseTest.cpp` 的 `WriteData()` 是 ASE 客製通訊。

---

## 三、順序（Steven 20260924 暫定，依「有頁面＋讀檔端已在」優先）

⛔ **20260924 深夜更正**：下面這條順序寫的時候還把 Speed／TrayAssignment／Contact／Setup／
YieldMonitoring 全部算成形狀 A；後來 Speed 已改走 C 形狀完成（§62），TrayAssignment／Contact／
Setup／YieldMonitoring 也在改走 C 形狀（十一、下一波，見 `generators.md`）。原始清單保留供對照，
不要照字面當成現況。

1. 形狀 A 且讀檔端已在：Contact → Setup → YieldMonitoring → Speed → TrayAssignment → HotPlate → TrayForm → OffSet（Temp_Set 等 `porting-gaps.md` 十三修好）
2. 形狀 C：`elConfig`（config.ini）先設計 `Add()` 註冊表 → `FormState` 的接法，再套到 UdUld／Contact／TrayForm／Laser／VacuumUnit
3. 形狀 B／D 的觸發者：`SaveTestMode`、`WriteLastDataFile`、`SetLevelSet` 等，golden 觸發者逐一對 live 狀態，缺的由產生器從 golden 改寫成 `act.*`——⛔ 20260926 更正：這句原是待辦事項，現已做完盤點：`SetLevelSet`＝S64（`8c5ea501`）、`WriteLastDataFile`＝S65（`1ba00a68`／`c317ca30`）、`SaveTestMode`＝S88（`295bc768`）、`config.ini [Taster]／[Event Log]／[Server]`＝S89（`295bc768`）都已逐一對照完成，見「二、逐結構清單」各對應列
4. 移植樹整檔不在的（TZteach、TfTeach::SaveFile、AOI、ProductionInfo、ContactForce）列待辦，同時產生 bridge（形狀 A／B 不依賴移植樹的表單檔）

---

## 四、golden `cprod.h` 全域變數的讀寫盤點（20260924；使用者：「根據定義的變數與結構，如果有被存檔與寫檔的，都要根據他的讀寫檔方式，生成到 FileRW；如果有對應的元件，那就是要給 html 用的，需要跟著 JSON 發送」）

形狀：A＝TIniFile 逐鍵；B＝二進位整塊；C＝HTEditList；D＝結構成員 SaveToFile；E＝其他（文字檔）。
「顯示」＝golden `DoIniDataToForm`／`FormShow` 缺或 GATE；「寫」＝golden `SaveSetupFile`／存檔鈕缺或 GATE。

| 變數（型別） | 檔案 | 形狀 | golden 讀 | golden 寫 | 表單（→ HTML JSON） | 移植樹現況 → FileRW |
|---|---|---|---|---|---|---|
| `TestIF_File`（SYSTEM_TEST_IF） | `<recipe>\Tester.Data`、`HandlerCondition.Data`、`Tray.Data`（2 鍵）、`DefineAutoClean\AutoClean.Data`、`config\AGV.ini`、`system\Barcode.ini` | A＋C（elLaser、elVacuumUnit→HandlerCondition.Data；elTrayForm→Tray.Data） | `TFTestIF::ReadTestIFFile` cTesterIF.cpp:563；`TfSetup::ReadFile` cSetUp.cpp:2199；YieldMonitoring :900；QAMode :179；LaserSensor :1218；BarCode :673；`LoadAutoCleanData` uCleaning.cpp:65；AutoAlignment :1373 等 | `TFTestIF::SaveSetupFile` :337；`TfSetup::SaveSetupFile` cSetUp.cpp:3655；YM :2033；Laser :1116；`SaveAutoCleanData` :836 | TFTestIF、TfSetup、TfYieldMonitoring、TfLaserSensor | 讀：`ReadTestIFFile` 改由 C 路 `FileRW/TestIF_File_TesterIF.cpp` 轉 golden 本體（開機／換配方 `tools/wb_serve.cpp:3231`；移植樹 `forms/fTesterIF.cpp` 那份仍 GATE F-5、已不需要）；`fSetup`／YM 移植樹 `ReadFile` live；AutoClean 讀在 `FileRW_Cleaning_Boot`（`wb_serve.cpp:3200`）。寫：C 形狀分成 `TestIF_File_TesterIF`／`_SetUp`／`_YieldMonitoring`／`_Cleaning`／`_QAMode`／`_VacuumUnit`／`_BarCode`／`_AutoAlignment`／`_Magazine`／`_FixAICCD` 十個結構（接線見 skill `ht9045-html-json` `route-c-golden-bridge.md` §6）。A 形狀 `FileRW/TestIF_File.cpp` 已刪（`f89be4ce`）；舊句移到 `archive/write-inventory_superseded.md` |
| `DeviceForm_File` | `<recipe>\Contact.Data`（選配 `system\Contact.ini`） | A（elContact 在 golden **沒有任何 Add()**） | `TfContact::ReadFile` cContact.cpp:365 | `TfContact::SaveSetupFile` :14313 | TfContact（:728） | ReadFile live；A 形狀 bridge 已退役（commit `b2aea32f`）→ ✅ C 形狀 `FileRW/DeviceForm_File.cpp` 讀寫通過（〇表 Contact 列；`a9636d9c` 力量公式＋存檔前重算衍生欄位，`generators.md` 十四）。舊句「寫入測試未跑完」移到 archive |
| `HotPlateForm_File` | `<recipe>\HotPlate.Data` | A | cHotPlate.cpp:154 | :480 | TfHotPlate | ✅ `FileRW/HotPlateForm_File.cpp` |
| `UserDefForm_File[4]` | `<recipe>\Tray.Data` | C（elTrayForm 48 筆）＋A | cTrayForm.cpp:364 | :643 | TfTrayForm（:318） | ✅ C 形狀 `FileRW/UserDefForm_File.cpp`：讀 `FileRW_TrayForm_ReadFile`（`tools/wb_serve.cpp:3185`）、顯示＋寫讀寫通過（〇表 TrayForm 列）。舊句「顯示＋寫未移植」移到 archive |
| `TrayForm`（SYSTEM_TRAY_FORM） | `<recipe>\Tray.Data`；config.ini 3 鍵（elConfig） | A＋C | cTrayAssignment.cpp:163 | :1409 | TfTrayAssignment（:548） | 讀＋顯示 live；A 形狀 bridge（`TrayForm.cpp`）已退役（commit `b2aea32f`，第八輪審查 H-2）→ **C 形狀已整合**（`16f463b8`，config.ini 那 3 鍵已在 IniConfig.gen.inc），整合時補上 `Prod.iTrayType[]` 缺口（見二、Tray Assignment 行）；✅ 讀寫通過（〇表 TrayAssignment 列，讀檔全部改走 golden 912 版）。舊句「仍剩 `Fix3/Direction` 查證中」移到 archive |
| `Temperature` | `<recipe>\Temperature.Data`、`DefineTemp\*`、`Config\ATC.ini` | A | uTemp_Set.cpp:1986 | :4606；`SaveTempMode` cprod.cpp:3645 | TfTemp_Set | 讀寫全 live（移植樹真碼）；`porting-gaps.md` 十三已結案（`322d68a3`，`W906_ReadATCIni` 在 `tools/wb_serve.cpp:3123`）；✅ C 形狀 `FileRW/Temperature.cpp` 讀寫通過（〇表 Temp_Set 列，`bc935659`）。舊句「porting-gaps 十三先修」移到 archive |
| `TestMode` | `<recipe>\TestMode.Data`（bLastSetInSetUpFile） | A | `ReadTestMode` cprod.cpp:3464 | `SaveTestMode` :3314 | 無表單（主畫面模式鈕） | live；Bindings.cpp 已綁 |
| `BinSelect[8]` | `<recipe>\Binasgn*.Data`（7 種） | A | `TfBinSel::ReadFile` cBinSel.cpp:1121 | `SaveOther` :2378 | TfBinSel | 讀 live；**SaveOther 的寫全在 GATE** → **C 形狀已整合並推送（`fe4f4d1d`）**：`tools/editlist/BinSelect.py`→`FileRW/BinSelect.cpp`，wb_serve 開機 `FileRW_BinSelect_Boot`、editlist 分派、binasgn*.data owner gate；網頁 `GB_EXTRA_SAVE` 附 bin 資料。A 形狀 `TfBinSel.py` 已退役。✅ 讀寫通過（〇表 BinSel 列；`bin.rules` 由後端輸出真值 `11df5b42`；專用入口 `FileRW_BinSelect_Page`／`_Save`，`tools/wb_serve.cpp:5212`／`:5322`）——舊句「讀頁探針 FAIL，未完成」移到 archive。20260925 Steven 設計指示：資料在 `vector<TMyBinPanel*> MyBinPanel` 與 8 組 `TStringList`，**改成 JSON 直接交換字串清單，不模擬面板 edit**（不建面板元件的具名替身），詳見 `generators.md` 十五 |
| `ArmSpeed_File[]`／`SHSpeed_File`／`MGSpeed_File` | `<recipe>\ArmCondition.Data` | ~~A~~ **C**（20260924 深夜改走，見二、逐結構清單） | cSpeed.cpp:340 | :2250（golden 存檔鈕不呼叫 `SaveSetupFile`，自己逐鍵寫 139 處） | TfSpeed（:1008） | ✅ 完成並實測 → **`FileRW/ArmSpeed_File.cpp`／`.gen.inc`** |
| `Ld_UldDelayTime` | `<recipe>\UdUld.Data` | C（elUdUld 15 筆＋IniConfig 14 鍵） | cLd_ULd.cpp:143 | :207 | TfLd_ULd | ✅ 完成（20260924 深夜）：**`FileRW/Ld_UldDelayTime.cpp`** 已產生（具名替身，移植樹 `fLd_ULd->Init()` 的真元件 Add 不再使用，程式仍在但不再被呼叫）；四頁回歸（含本結構）`--write` **ALL PASS**（`generators.md` §66） |
| `Offset_File`＋`In/Out/SortArmOffSet_File[]` | `<offset dir>\Position Offset.Data`（Hot／cool 變體） | A | cOffSet.cpp:1998 | `SaveFile` :1406 → `SaveSetupFile` :1470 | TfOffSet（:2316） | 讀 live；✅ C 形狀 `FileRW/Offset_File.cpp` 顯示＋寫讀寫通過（〇表 OffSet 列，`e086de38`）。舊句「顯示＋寫未移植」移到 archive |
| `InvisibleOffset` | `DefineOffset\STD_*.Data`、`data\<recipe>.ini` | A（讀或建） | cOffSet.cpp:1859 | 同左 | 無 | live，不需 FileRW |
| `LevelSet` | `system\levelset.dat` | B | `GetLevelSet` cSecurity.cpp:1474 | `SetLevelSet` :1511（FormClose :465） | TfSecurity | ✅ 已完成（commit `8c5ea501`，S64）：`system.levels.put` 已改走 `WebLevelSet.cpp`／`W906_LevelSetPut`（見二、`LevelSet` 列細節）；`Bindings.cpp:85` 已綁 |
| `USER`（PASS_WORD） | `system\login.dat` | B | `ReadPassword` cprod.cpp:1374 | `SavePassword` :1360 | TfSecurity／TfLogin | live；**不經 GET 暴露**，只走 auth.*／act.* |
| `TTLCfg` | `<recipe>\<DIO>.ini` | A | `TfDIOFrom::LoadData` DIOInterFaceCFG.cpp:72 | `spbSaveClick` :191 | TfDIOFrom | LoadData／存檔 GATE → 進行中（20260924 夜）：`tools/formbridge/TfDIOFrom.py` 已建、**`FileRW/TTLCfg.cpp`** 已產生（五工程師平行分工「BinSel＋DIOFrom」那一組）。20260925 Steven 設計指示：「預設顯示 `TFTestIF->cbDIOType->Text`」，來源是 golden `TFTestIF::InitcbDIOType(bool bAlarm)`／`cbDIOTypeChange`，改走這條，**不必等** `ReadTestIFFile`（原本卡在 GATE F-5）。**C 形狀已整合並推送（`fe4f4d1d`）**：`tools/editlist/TTLCfg.py`→`FileRW/TTLCfg.cpp`，開機與 reload 在 `fContactForm->ReadFile()` 後呼叫 `FileRW_TTLCfg_ReadDIOSection()`（只讀 Tester.Data `[DIO]`）；A 形狀 `TfDIOFrom.py` 已退役。⛔ 20260926 更正（舊句「探針未跑起來、未驗」「`InitDIOStstus` 移植樹沒有」移到 archive）：✅ 讀寫通過（〇表 DIOInterFaceCFG 列）；開機讀 `FileRW_TTLCfg_DoReadLastDataLoad`（`tools/wb_serve.cpp:3470`）；`InitDIOStstus` 已由 Jimmy 移植（`0ca03ee6`，`cDIOStatus.cpp`），存檔後 `FileRW/TTLCfg.cpp:267-270` 呼叫 `fMain->InitDIOStstus(true)`；開機那一處仍閘著（`cDIOStatus.cpp:91` AI(W906-DIO-BOOTGATE)，理由「開機沒讀 DIO 檔」——現在 `:3470` 已讀，閘的前提是否已過期待 Jimmy 確認） |
| `tAOISetup`、`ScannerAOIIF` | `<recipe>\AOI.Data` | A | fAOI.cpp:3369／:3446 | :3120 | TFrmAOI | ✅ 讀寫檔已翻（S69，`9ec84450`／`e6a8e0fc`；`FileRW/AOISetup.cpp`，開機／換配方讀 `tools/wb_serve.cpp:3454`）；沒有網頁（`pending-pages.md` 十三）。舊句「未移植」移到 archive |
| `Teach` | `System\teach.ini` | D（TECH_* SaveToFile）＋C（elTeach 5 筆） | `TfTeach::ReadFile` uteach.cpp:4785 | `TfTeach::SaveFile` :4939 | TfTeach | ReadFile／SaveToFile live；✅ C 路 `FileRW/Teach.cpp`（`ad7561d4`）：`btnSaveClick`＋`IC_SaveFile`（`Teach.cpp:147`）寫 teach.ini，tech.dat 刻意不寫（`ELTodo`，S75）；見二、`Tech` 列。舊句「SaveFile 未移植」移到 archive |
| `AutoTeach` 表 | `System\AutoTeach_*.dat` | B | cprod.cpp:3842／:3887 | :3921 | TfAutoAlignment | live |
| `ATKRecipeInfo` | `<recipe>\Information.txt` | E（只寫） | — | cprod.cpp:381 | 無 | live |
| `RunInfo` | DailyJamRate txt | E | cprod.cpp:1112 | :995／:1171 | 無 | live；⛔ 20260927 補：移植樹 `cprod.cpp` 的 `ReadJamRateByDay`（`:1195`）／`SaveJamRateByDay`（`:1070`）本體仍在 `#if 0 // TODO(GA1-B2)`，所以 golden 開機讀（`main.cpp:9607`）與關程式寫（`FormClose :11919`）另外照 golden 逐行抄在 `FileRW/MainBoot.cpp` `W906_FRWBoot_JamRateByDayRead`（`1d68d518`）／`FileRW/MainClose.cpp` `W906_RunInfo_SaveJamRateByDay`（`6905f8eb`；一個行程只存一次，todo ★ R25） |
| `Prod`（2 鍵） | `HandlerCondition.Data [Shuttle]` | A | ShuttleMove.cpp:2252 | :2749 | TfShuttleMove | 呼叫點 GATE（cinitial.cpp N3-G5）。⛔更正（20260926）：C 形狀已整合並推送（commit `0609a14f`）——S47 裁決「只有 latch 機台才讀寫 `InSH?SenICAddPos`」，`teach.ini` ALL PASS；`cinitial.cpp` N3-G5 那道 GATE 仍交給 Jimmy（見〇、ShuttleMove 列，CHANGES_20260926 §3.3） |
| （LastSet／IniConfig／CosFunction） | lastdata.dat（B）＋LastSet.ini（C）；config.ini＋configByRecipe.ini（C） | B／C | cprod.cpp:1603／:2977 | :1910／:3092 | TfConfiguration | `FileRW/IniConfig.cpp`（見一之二） |

**純執行期（不存檔）**：所有 `*_NET` 快照、cUnitConvert 轉出的 live 鏡像（TestIF、DeviceForm、UserDefForm、ArmSpeed、Offset…）、`ReserverEmptyPoint*`、`AutoArmSpeed`、`MRSpeed*`（golden 只有 extern）、`bDoRTCLearning`、`SThreadPara` 等；`InputLimit`／`DummyVacuum`（Security_new.def，唯讀）。

### 四之二、FileRW 與 JsonBridge／HTML 要對齊的事（工程師檢查 20260924）

1. **一個檔一個寫者**。現在頁面走檔案鏡像（`system.file.put`／`recipe.doc.put`）直接改檔，記憶體不會重讀；下一次 golden 存檔（例 `SaveLastSetIni`）會把舊值寫回去（lost update）。重疊的檔：config.ini、LastSet.ini、configByRecipe.ini、UdUld.Data、Tray.Data、HandlerCondition.Data、teach.ini、Gerneral.ini、levelset.dat。→ wb_serve 加「檔案 → 擁有者」表，FileRW 接手的檔鏡像寫一律 409。
2. `Config.Configuration.html`：1,191 個替身名稱中 1,181 個對得上頁面 id（99.2%），型別全對；**V912 才有的 8 個元件頁面沒有**（`cbA77 cbA78 cbD83 cbE79 cbF37 cbO21_1 chkO25 edF37_Delay`）→ 由 V912 dfm 重產頁面。頁面現在只從 `/api/system/config` 載入 163 個文字欄，628 個勾選／選項沒載。
3. JS：`ht9045_recipe_client.js` 加 `editlist(name)`／`editlistSave(struct, widgets, answers)`；引擎加 `editLists` wire 鍵與 `editlistOverlay()`（套值、visible／enabled、proxies 的 tabVisible），`save()` 有 `editLists` 就走 `editlist.save`。
4. **重複註冊**：`ht9045_wire_lduld.js`／`ht9045_wire_setuplduld.js`、`ht9045_wire_trayform.js`／`ht9045_wire_setuptrayform.js` 各兩支都 `HT9045Wire.register` → 按一次存檔存兩次。
   - `lduld` 那組：**已處理（20260924 夜）**——`Setup.Ld_ULd.html` 內 `ht9045_wire_setuplduld.js` 那個 `<script>` 已標記「與上面 `ht9045_wire_lduld.js` 內容相同」並移除（頁面走 C 路 `editlist.get`／`editlist.save` 之後，舊的 B 路 wire 本來就該退場，見 `generators.md` 三之三）。
   - `trayform` 那組：**仍待辦**——`ht9045_wire_trayform.js`／`ht9045_wire_setuptrayform.js` 尚未處理。⛔ 20260924 深夜更正：`TrayAssignment` 的 A 形狀 bridge 已產生又退役（commit `b2aea32f`），`UserDefForm_File` 已有 `tools/editlist/UserDefForm_File.py`／`FileRW/UserDefForm_File.cpp` 在做（進行中），不是「還沒轉到 C 路或 A 形狀 bridge」——但頁面這組重複註冊本身還沒清，等這兩個結構的 C 路驗收過再一併處理。
5. JsonBridge：editlist 用自己的登記表（`{name, HTEditList**, struct, files, page, owner cpp}`），不要在 `Bindings.cpp` 另建 FieldDesc；`testIF.file` 的 `sourcePorted:true` 要改成部分（ReadTestIFFile GATE F-5）。（⛔ 20260926：後半句作廢——開機已由 C 路讀完整的 golden `ReadTestIFFile`（`tools/wb_serve.cpp:3231`），`JsonBridge/Bindings.cpp:99-100` 的 `sourcePorted:true` 現在是對的。）A＋C 頁面（TrayForm／Teach）的 `FormSave` 要先 `ELApplyProxies` 再跑 saveFlow。
6. D 類：`struct.put{binding, values}` 在 FormLock 下**只把改的欄位**蓋到 live 結構（tick／Clarn_Data 一直在改 LastSet 計數），再呼叫 golden 落地器。⛔ 20260924 深夜更正：`system.levels.put` **不是**改成別名——第八輪審查 M-1 已讓它自己在寫檔前套 golden `TfSecurity::FormClose` 三條鉗制（`W906_SecurityClampLevels`），檔案與記憶體一致，不必再解 GATE (SEC-W2)，見 `generators.md` 十。

---

## 🆕 進行中（20260926 15:30，更正舊敘述）

> 與 ChangeLog `CHANGES_20260926_Steven.md` §12.2 同步，每次記錄員更新 §12 時一併更新這裡。
> 下面兩行原寫「⏳ 進行中」已過期：S88／S89 已完成（commit `295bc768`，15:30），§12.2 目前
> 沒有工程師在做。

* S88 `TestMode.Data` 的 `SaveTestMode` 觸發者對照——✅ 已完成（commit `295bc768`）：golden 16 個呼叫點已接 9 個，其餘 7 個交 Jimmy／Steven02，或 golden 本身走不到；見「二、逐結構清單」`TestMode` 列。
* S89 `config.ini [Taster]`／`[Event Log]`／`[Server]` 觸發者對照——✅ 已完成（commit `295bc768`）：golden 8 個呼叫點已接 2 個，4 個 golden 本身是死碼，1 個客戶專屬不做，1 個（`SaveRmsInfo`／`TfMain::FormClose`）待 Steven 決定（Q2）；見「二、逐結構清單」`IniConfig` 列。

## 🆕 待派佇列（20260926）

> 來源：0925 `cmydef_io_audit` 盤點（第四節，另一個 session 的 scratchpad，結論摘要於此，不是活路徑）；
> 與 ChangeLog `CHANGES_20260926_Steven.md` §12.3 同步。

**⛔ 更新（20260926 稍晚）**：本節原本列的項目全部做完了（P8 四小項 `c913d5e5`；`TfMagazine`／
`TfFixAICCD` `c79ee4e9`；`SetLevelSet` 觸發者盤點 `8c5ea501`；`WriteLastDataFile` 觸發者盤點
`1ba00a68`；`TZteach`／`TfProductionInfo` `58bd9425`；`IniConfig` 的 `SetTestRunMode` GATE
`5bbbb31f`），原句移到 `archive/write-inventory_superseded.md`。現在只剩：

* S69 `TFrmAOI`（AOI.Data）——✅ 已完成（`9ec84450`／`e6a8e0fc`）。
* S86 `AOI.Data [OCR SETTING]`（`TfOCR`）——✅ 已完成（`973f2540`）。
* S57 `Setup.ContactForce`——✅ 已完成（`21d37f2b`；429 中斷後 Steven 手動停掉，13:5x「要做」
  重派）。
* S74 `TrayMapping` 的 `ScanLine`——卡 Jimmy 的 G-7／G-8（AOI 物件），見「二、逐結構清單」
  對應列與 `porting-gaps.md`。
* S67 tech.dat（`TfSmartSetup`）——與 S75（`TfTeach::SaveFile`）同一個檔；⛔ 20260926 更新：Steven 13:5x 已裁示「不重要，往後排」（`RULINGS_20260926.md` S86～S87 表 S67 列），記在 skill `ht9050-construction` `references/todo.md`「往後排」（舊句「暫不派，待分工」移到 archive）。
* S71 `Alert.Password` Event Log 兩分頁——與 Steven 另一個 session 的 cMyDB P1（S72）範圍
  重疊，暫不派。

**不做**：P7（Jimmy 底層）、P9／`Setup.SCK_ART`／`Setup.AGV`／`HW.MyCCLinkSensor`（客戶專屬，S25 裁決）。
