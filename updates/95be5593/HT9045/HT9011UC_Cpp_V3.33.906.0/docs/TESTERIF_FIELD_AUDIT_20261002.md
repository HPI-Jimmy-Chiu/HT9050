# Setup.TesterIF（golden TFTestIF）欄位稽核 20261002

> census 129 C3-119 備註那一句（「TesterIF 28 個 PENDING 已過時（UNDECIDED）」）的查核。
> 唯讀稽核：只讀檔、沒有建置、沒有執行（STEVEN-NB3 只編譯不執行）。St02-E 的 helper 寫，給 Steven／St02-M／St01 看。
> 基準：worktree `D:\AI_TempFile\st02-s14`（`v906/st02-board-1002`＝origin/main `8b8209cc`＋一個現況板 commit）。
> golden：`906_0625_Steven`＝`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven`（cp950）。本文 golden 行號一律是 906 的 `cTesterIF.cpp`（寫成 `G:行`）；
> 移植的產生檔是從 **912** 轉的（`tools/editlist/TestIF_File_TesterIF.py:28`，RULINGS_20260925 第 37 條「一律照 V912」），所以 `.gen.inc` 註解裡的「golden cTesterIF.cpp:NNN」是 912 行號；兩版差異見 §8。
> 移植檔路徑都相對 `HT9011UC_Cpp_V3.33.906.0/`；`N:行`＝`FileRW/TestIF_File_TesterIF.gen.inc` 的行號。

---

## 摘要（先看這裡）

**結論一句話：C 路（St01 `8af13c07`）的讀檔與存檔跟 golden 逐行相同，88 個存設定的元件沒有錯鍵、錯區段、錯預設、錯型別；舊的 28 個 PENDING 全部是死的。真正的問題在頁面端的小鍵盤：22 個欄位打進去的值會被改掉。**

### 數字

| 項目 | 數量 | 說明 |
|---|---|---|
| golden 表單上存設定的元件（edit／combo／check／radio group） | **105** | DFM 110 個輸入類元件，扣掉 lstTTL（顯示）、btTesterTCPShow、spbSave、sbtExit、rbTemp（只拿焦點用） |
| 其中 golden 有讀有寫（`<配方>\Tester.Data`） | **88** | 讀＝ReadTestIFFile G:562＋DoIniDataToForm G:979；寫＝SaveSetupFile G:336 |
| 其中 golden 自己就不讀不寫 | **17** | SLT 分頁 15 個＋cb_NeedSendVSOT＋rgBaudRate（§4），移植也不讀不寫＝一致 |
| C 路讀檔一致 | **88／88** | 區段、鍵、預設值、struct 欄位型別逐字相同（§3、§附錄） |
| C 路存檔一致 | **88／88** | WriteIniData 的區段、鍵、格式（FormatFloat／Text／ItemIndex／Checked）逐字相同 |
| 讀檔漏掉 | **0** | |
| 存檔漏掉 | **0** | |
| 頁面上沒有 | **0** | 110 個 DFM 元件的 id 全在 `web/page/Setup.TesterIF.html:56`；存檔必送（mustSend）88 個全在 |
| 頁面端不一致（不是 C++ 讀寫） | **22 個欄位會被改值**＋4 項較小 | §6 P1～P8 |
| 舊 PENDING（`web/page/ht9045_testerif_wire.js:101-130`） | **28：26 已由 C 路讀寫、2 條本來就寫錯；真缺口 0** | §5；而且這支 js **沒有任何頁面載入** |

### 不一致清單（照嚴重度排）

C++ 端「寫錯鍵／錯預設／錯型別」：**0 筆**。以下都在頁面端或旁路：

1. **P1（高）小鍵盤範圍抄錯分支，6 個「最長測試時間」欄位**：edMaxTestTime、edInitialMaxTest 與兩者的 `_RT`／`_EQC`。網頁是 `INTEGER 60～9999`（`web/page/ht9045_wire_setuptesterif.js:100-102`、`:104-106`），那是 golden `edInitialMaxTestClick` 的 **CC_ASE_M 分支**（G:1382-1385）；一般客戶走 G:1399-1402＝`N_DOUBLE 2 位、0～15000`（`IniConfig.bVTESTFunction` 時 0～36000，G:1395-1398）。網頁小鍵盤按 OK 會夾限（`web/page/qwerty.js:68-71`）：打 10 或 12.5 都變 60、打 20000 變 9999、範圍內的小數也被捨成整數 —— **寫進 Tester.Data 的不是操作員打的值**。
2. **P2（高）Initial Start Delay 三個欄位下限 30**：edtInitStartDelay（＋`_RT`／`_EQC`）網頁是 `DOUBLE 2、30～3000`（`ht9045_wire_setuptesterif.js:115,119,120`），那是 `CosFunction.bHiSiliconFunction && CC_ASE_KaohSiung` 分支（G:1412-1413）；golden 一般是 `N_DOUBLE 2、不檢查範圍`（G:1415）。打 0～29.99 會被改成 30。
3. **P3（中，共用檔）小鍵盤把小數吃掉，13 個欄位**：golden 給 `N_DOUBLE, 0` 的欄位（edDummyTestTime、edStartDelayTime 與兩者 `_RT`／`_EQC`、edtAfterTestedDelay、edtInitWaitTime、edtTestingWaitTime、edtInitialDec1～4），網頁 `qwerty.js:71` 用 `toFixed(dp)`＝`toFixed(0)` 四捨五入成整數；golden 的 dp 只決定 ±鍵的級距（`myQwertyKeyBoard.cpp:379-418`），OK 後是 `AnsiString(CheckRange(d,min,max))` 不改小數（`myQwertyKeyBoard.cpp:290`）。Start Delay 打 0.5 秒會被四捨五入成整數秒。
4. **P6（中低）遠端 SETMAXTEST／SETINITIALMAXTEST 沒更新頁面替身**：`Command.cpp:3691`、`:3718` 寫的是 facade `FTestIF`（`forms/fTesterIF.cpp`）的 TEdit，不是 C 路替身 `EL<TEdit>("TFTestIF","edMaxTestTime")`。頁面開著時收到指令、操作員再按存檔，會把舊值寫回去；golden 改的是同一個表單元件（golden `Command.cpp:12474`、`:12495`），存檔會帶新值。
5. **P4（中，已知，不寫錯檔）RS232 那組在 golden 停用時網頁仍可點**：Ifor01 的探針（`docs/handoff/TO_STEVEN.md:347`，`tools/webprobe/s12c_page_probe.py` 3／3 FAIL）。伺服器存檔時照 ELEditable 丟掉，不會寫錯，但操作員會以為改到了。
6. **P5（低）下拉不能自己打字**：golden 五個 TComboBox 都沒設 Style＝預設 csDropDown（可打字）；網頁是 `<select>`。實際有影響的只有 cbbBaudRate（golden 存 `->Text`，G:397，可以打 38400／57600；網頁只能選 6 個值，檔案本來就有的非清單值會照原樣帶回）。
7. **P8（低）Exit 鈕少一個檢查**：golden sbtExitClick（G:1358-1366）看「畫面上選的」rgInterfaceType==0 就先 CheckTTLBoardBitMode；網頁關頁掛勾 `FileRW/TestIF_File_TesterIF.cpp:590-604` 只跑 FormClose 那段（看檔案值 TTL_MODE）。
8. 已裁決的偏離（不算錯，列出來免得被當成缺口）：W6（GPIB 模式也能改 RS232 四個，Steven 20260927 W6＝A，`FileRW/TestIF_File_TesterIF.cpp:330-334`）、5B（RS232 多 5/6 Bits、1.5 Bits、Mark/Space 選項，`FileRW/TestIF_File_TesterIF.cpp:258-265`，註記仍是「暫照建議，待使用者確認」）、數字預檢與 DIO 檔遺失拒存（`FileRW/TestIF_File_TesterIF.cpp:33-37`、`:76-91`）。

### PENDING 結論

**全部死掉，真缺口 0。** 26 條現在由 C 路 `editlist.get／editlist.save tag=TestIF_File_TesterIF` 讀寫（key 與 golden 相同）；另外 2 條（rgBaudRate、cb_NeedSendVSOT）是當初抽錯，golden 本來就不存這兩個元件。舊檔 FIELD_MAP／PENDING 還漏了 AntiSignalCBox（`[DIO] Anti Signal`）與 cbRs232Type（`[RS-232C] Type`），C 路也都有。
更根本的是：`ht9045_testerif_wire.js` **沒有任何頁面載入**（`Setup.TesterIF.html:118-128` 載的是 `ht9045_wire_testerif.js`、`ht9045_wire_setuptesterif.js`、`ht9045_testerif_c_wire.js`），只靠筆電的 `tools/websync/sync_web.py:173` OURS 清單留在部署裡。

### 下一步

- **St01（FileRW，認領行）**：TestIF_File_TesterIF.* 本身**不用改**。兩個「別的頁存檔後 golden 會重讀 TesterIF」的子集／閘，現在可以改呼叫現成的 `FileRW_TesterIF_ReadTestIFFile()`（`FileRW/TestIF_File_TesterIF.cpp:276-280`，檔頭 :20-21 已預留）：
  1. `FileRW/IniConfig.gen.inc:7974-7977`（改 `tools/editlist/IniConfig.py` 的 replace）：golden `cConfiguration.cpp:7287`（912 `:7400`）`FTestIF->ReadTestIFFile()` 目前只記 ELTodo。
  2. `FileRW/Temperature.gen.inc:4224`、`:6205`（改 `tools/editlist/Temperature.py`）：golden `uTemp_Set.cpp:3149`、`:5088` 是完整 ReadTestIFFile，移植只重讀 `[InitialMode]` 子集（census C2-057／C2-058）。
- **St02（頁面／js）**：
  1. P1＋P2：在手寫補件 `web/page/ht9045_testerif_c_wire.js`（St01 建、St02 Q41 改過）覆寫 9 個欄位的小鍵盤設定成 golden 一般分支（產生檔 `ht9045_wire_*.js` 不能手改）；要連 `IniConfig.bVTESTFunction`（不是客戶碼）一起照，就由 `FileRW/TestIF_File_TesterIF.cpp` ExtraJson（:217-239）多帶一個 `extra.kb`。CC_ASE_M／AMKOR_Korea／SCK／GIGAS／HiSilicon+ASE_KaohSiung 是客戶專屬，S25 只註記。
  2. P3：`web/page/qwerty.js:68-71` 的 `toFixed(dp)` 改成照 golden（夾限後不改小數位）—— 共用檔，所有頁都受影響，先認領。
  3. P4：`ht9045_testerif_c_wire.js:135-145`（applyEditable）／`:187-214`（W6）讓 golden 停用的 RS232 元件在網頁也停用（照 Ifor01 的探針重測）。
  4. P6：`Command.cpp:3691`、`:3718`（St02 `329b6f18` 解的閘）同時更新 C 路替身（FileRW 只編進 wb_serve，比照 `FileRW/TestIF_File_TesterIF.cpp:292-293` 的掛勾做法）。
  5. P5、P8 低優先，可等。
- **筆電（jimmychiu）**：
  1. 退役 `web/page/ht9045_testerif_wire.js`（產生器 `tools/pagewire/make_wire.py`，唯一 commit `55302732`），同時拿掉 `tools/websync/sync_web.py:173` 那一行；census 129 C3-119 備註裡「TesterIF 28」可以關。
  2. 順手：`Setup.TesterIF.html:121` 與 `:125` 兩支產生檔內容幾乎一樣，後註冊的蓋掉前一支（`ht9045_wire_engine.js:2154-2155` `CFG = cfg`），留一支即可；產生器（`scratchpad/gen_wire.py`）抽小鍵盤時拿「處理函式裡第一個 ShowQwertyKey」，遇到客戶碼分支就抽錯（P1／P2 的來源），其他頁可能也有。
- **只有 Steven 能答（一題）**：5B（RS232 加 golden 沒有的 5/6 Bits、1.5 Bits、Mark/Space）程式註記仍是「暫照建議，待使用者確認 decision #5」，要正式確認嗎？在哪看：`D:\AI_TempFile\st02-s14\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp:258-265`、`D:\AI_TempFile\st02-s14\web\page\Setup.TesterIF.html:56`（rgBitLength／rgStopBit／rgParity 後面的選項）。其餘都照 RULINGS_20261001 第 0 條「照 golden 翻」自己定。

---

## 1. golden 的讀寫路徑（906 `cTesterIF.cpp`）

- 檔案：`DataPath + GetLastOpenFN() + "\\Tester.Data"`（讀 G:566-569，寫 G:1311-1316＋G:338-339）。RS232 模式另外同步 `D:\RS232Standard\System\Setup.ini`（CheckRs232StandardIni G:496-558）。
- **開頁**：FormShow G:108 → InitcbDIOType G:117（列 `DIOCFGPath\*.ini`）→ ReadTestIFFile G:118 → DoIniDataToForm G:119（ReadTestIFFile 尾端 G:976 也呼叫一次）→ 顯示／權限（G:124-331）。
- **讀檔器** ReadTestIFFile G:562-977：讀進 `TestIF_File`（cprod.h `SYSTEM_TEST_IF`）。**讀的時候也會寫**：Tester Type 超出範圍改 GPIB 並回寫（G:574-578）、缺 TypeName 時由 Type 反查回寫（G:797-827）、缺 BaudRate 時由舊鍵 Baud Rate 換算回寫（G:883-893）、RS232 模式同步 Setup.ini（G:945-948）。
- **表單填值** DoIniDataToForm G:979-1124：`TestIF_File` → 元件。
- **存檔** spbSaveClick G:1302-1356 → SaveSetupFile G:336-492（全部寫 Tester.Data）→ SECS SaveRecipe → ReadTestIFFile → Recipe Parameter Default（CosFunction.bRecipeParameterDefault，客戶 Greatek 用）→ BackupSetupFile → SetWorkParameter。
- **關頁** FormClose G:1197-1217：ReadTestIFFile（丟掉沒存的改動）、oldLastiTestMode、TTL 收尾；呼叫端主畫面 sbTesterClick 尾段（golden `main.cpp:27425-27432`）。
- **其他 golden 呼叫點**：開機／換配方 `main.cpp:8904`（ReadTestIFFile）、`:8962`（DoIniDataToForm）；設定頁存檔後 `cConfiguration.cpp:7287`；溫度頁 `uTemp_Set.cpp:3149`、`:5088`；整份另存 `cBuilder.cpp:517-563`、`csystem.cpp:22562-22563`。

## 2. 移植的讀寫路徑（C 路）

- 產生器 `tools/editlist/TestIF_File_TesterIF.py`（golden 912，METHODS :32-34，replace 清單 :95-197）→ `FileRW/TestIF_File_TesterIF.gen.inc`（替身 EL<T>("TFTestIF", 名稱)，名稱＝HTML id）。
- 頁面登記 `FileRW/TestIF_File_TesterIF.cpp:241-248`：開頁＝TIF_FormShow（N:629），存檔＝SaveFlow（`:76-118`）→ TIF_spbSaveClick（N:1873）→ TIF_SaveSetupFile（N:869），重讀＝TIF_ReadTestIFFile（N:1107）→ TIF_DoIniDataToForm（N:1565）。
- 存檔前重播 golden 畫面事件 BeforeApply（`:126-184`：rgInterfaceTypeClick、cbRs232TypeChange、cbDIOTypeChange、AntiSignalCBoxClick×2，W6）。
- 網頁版多的兩道檢查（golden 沒有，已寫在檔頭 `:33-37`）：DIO 檔遺失拒存、33 個 ToDouble 欄位先試轉（全有或全無）。
- 開機／換配方：`tools/wb_serve.cpp:4059`（Boot）、`:2995`（ReadTestIFFile）、`:3460`（DoIniDataToForm）。
- 關頁：`FileRW/TestIF_File_TesterIF.cpp:590-604`（St01 視窗邊緣 API，Q41 (d)）。
- 頁面：`web/page/Setup.TesterIF.html`；引擎 `ht9045_wire_engine.js:1066` 把這頁登記成 C 路（GOLDEN_BRIDGE），load＝gbLoad（`:1313-1314`）、save＝gbSave（`:1790-1791` → `:1264`）；補件 `ht9045_testerif_c_wire.js`。
- `forms/fTesterIF.cpp` 的 TFTestIF 是另一份 facade，讀寫本體全部 `#if 0`（GATE F-1～F-17，`:236` 起），**不是**活的讀寫者。

逐行比對結果（腳本見附錄）：

| 函式 | golden 906 | 移植 | 差別 |
|---|---|---|---|
| ReadTestIFFile | G:562-977 | N:1107-1562 | 只有：TCP/IP 計時器 3 處改 ELTodo／空敘述（門面沒有 TTimer，census C2-085／086）、912 才有的 [AMR] 四鍵（沒有元件）、MyList 越界保護、AMD 判斷照 912、KYEC_LEE AOI 照 912、1Arm/2Arm 訊息照 912、ATKRecipeInfo 空指標保護 |
| DoIniDataToForm | G:979-1124 | N:1565-1715 | 119／119 行相同（只有函式頭） |
| SaveSetupFile | G:336-492 | N:869-1028 | 116 行相同，多一行 trace（N:871） |
| spbSaveClick | G:1302-1356 | N:1873-1937 | A02 分支多 `return`（照 912）、Recipe Parameter Default 改 ELTodo（N:1906，census C2-056） |
| FormShow | G:108-334 | N:629-866 | Caption／Left／Top／ActivePage 交給頁面；rgTime2 頁籤條件照 912 |
| CheckRs232StandardIni | G:496-558 | N:1031-1104 | Bit/Stop/Parity 換算改 `W906_Rs232*Code`（5B，原有值對應不變） |

ReadIniData／WriteIniData／CheckIniData 呼叫逐條比：golden 205 條、移植 206 條；只有 912 [AMR] 四條讀（N:1184-1187）多、RPDefault 三條（G:1439、:1468、:1499，Greatek 專用）少。struct 欄位型別 89 個全部與 golden `cprod.h` 相同。

## 3. 逐元件對照表（golden 有讀寫的 88 個）

欄位說明：
- **golden 讀**：`R:`＝ReadTestIFFile 讀 ini 的行、`F:`＝DoIniDataToForm 填元件的行，括號是預設值，分號後是 `TestIF_File` 欄位與型別。
- **golden 寫**：區段／鍵、SaveSetupFile 行、寫入格式。
- **移植讀／寫**：`N:` 行號（R／F／W 同上）。
- 檔案一律是 `<配方>\Tester.Data`。
- **頁面**：`Setup.TesterIF.html:56` 的 id（全部存在），再加 golden FormShow 決定的顯示／可改條件（移植由後端照同一段 golden 算，頁面照套）。
- **PENDING**：是否在舊 `ht9045_testerif_wire.js` 的 PENDING。

| 元件 | golden 讀 | golden 寫 | 移植讀 | 移植寫 | 一致？ | 頁面 | 舊 PENDING？ |
|---|---|---|---|---|---|---|---|
| rgInterfaceType | R:572 F:981（預設 0；超出範圍改 1 並回寫 G:574-578）；iTestType int | [Mode] Tester Type W:341（int 索引） | R:1118 F:1568 | W:876 | 是 | id 在 :56；grpIFType 可改條件 G:130-142／:243-259／:316；SCC/SCK RMS G:205 | 是 |
| cbDIOType | R:684,830 F:1073,1075,1077,1086（預設 0／""；缺 TypeName 時 G:797-827 反查回寫）；iDioMode int、sDioName AnsiString | [DIO] Type／[DIO] TypeName W:371,372（Type＝ItemIndex、TypeName＝Text） | R:1264,1413 F:1660,1662,1664,1673 | W:906,907 | 是 | id 在 :56；tsDio（只在 DIO 模式 G:1174-1183） | 是 |
| AntiSignalCBox | R:834（預設 0）；畫面值＝FormShow G:125 的執行期 TestIF.bAntiSignal（不是 TestIF_File）；bAntiSignal bool | [DIO] Anti Signal W:373（bool 0/1） | R:1417；FormShow N:650 同 golden | W:908 | 是 | id 在 :56；tsDio；Visible=LastSet.bUseNewTTLBoard G:124 | 否（兩邊都沒有） |
| cbASEJPMode | R:781 F:1070（預設 false）；bTTLUseASEJPMode bool | [InitialMode] bTTLUseASEJPMode W:482（bool 0/1） | R:1361 F:1657 | W:1017 | 是 | id 在 :56；tsDio | 是 |
| cbGPIBType | R:835 F:1090（預設 0；ATK ART 時改用 Config 值 G:838-844）；iGpibMode int | [GP-IB] Type W:385（int 索引；ATK ART 條件 G:376-386 不寫） | R:1418 F:1677 | W:920 | 是 | id 在 :56；tsGpib（只在 GPIB 模式） | 是 |
| edGPIBAddress | R:850 F:1091（預設 0）；iGpibAddress int | [GP-IB] Address W:390（原字串） | R:1433 F:1678 | W:925 | 是 | id 在 :56；tsGpib（只在 GPIB 模式） | 否（FIELD_MAP） |
| cbSpiroxTesterLotEnd | R:771 F:1104（預設 false；非 bI39 固定 false G:770-773）；bSpiroxTesterLotEnd bool | [InitialMode] bSpiroxTesterLotEnd W:458（bool 0/1；非 bI39 時寫 false G:457-460） | R:1351 F:1691 | W:993 | 是 | id 在 :56；tsGpib；Visible=IniConfig.bI39SpiroxTesterLotEnd G:291-294 | 是 |
| rg2DID_Format | R:872,876 F:1092（預設 1／int(bAMDFunction)）；i2DIDFormat int | [GP-IB] 2DIDFormat W:391（int 索引） | R:1446,1450 F:1679 | W:926 | 是（照 912；906 的預設條件是 CC_AMD_M，§8，S25） | id 在 :56；tsGpib（只在 GPIB 模式） | 是 |
| cbAutoOnecycleHomStart | R:788 F:1118（預設 false；非 GIGAS 固定 false G:791-795）；bAutoOnecycleHomStart bool | [Time] bAutoOnecycleHomStart W:485（bool 0/1；只 CC_GIGAS G:483-487） | R:1368 F:1705 | W:1020 | 是 | id 在 :56；tsGpib；DFM 隱藏，只 CC_GIGAS 顯示 G:318-323（S25） | 是 |
| edtAutoOnecycleHomStartTime | R:789 F:1119（預設 0）；iAutoOnecycleHomStartTime int | [Time] iAutoOnecycleHomStartTime W:486（原字串；只 CC_GIGAS G:483-487） | R:1369 F:1706 | W:1021 | 是 | id 在 :56；tsGpib；DFM 隱藏，只 CC_GIGAS 顯示 G:318-323（S25） | 否（FIELD_MAP） |
| cbRs232Type | R:879 F:1094（預設 0）；iRs232Mode eRs232Mode | [RS-232C] Type W:394（int 索引） | R:1453 F:1681 | W:929 | 是 | id 在 :56；tsRs232（只在 RS232 模式） | 否（兩邊都沒有） |
| cbForEgistec | R:904 F:1103（預設 false；非 bArm2ForFingerPrintTest 固定 false G:903-906）；bForEgisTecTest bool | [RS-232C] ForEgisTecTest W:404（bool 0/1；非 bArm2ForFingerPrintTest 時寫 false G:403-406） | R:1478 F:1690 | W:939 | 是 | id 在 :56；tsRs232；非 SOFT_SIMULTE 一律隱藏 G:187-191 | 是 |
| rgParity | R:900（預設 0）F:1100；Rs232_Data.Parity int | [RS-232C] Parity W:400（int 索引） | R:1474 F:1687 | W:935 | 是（另有 5B 補選項＋W6，裁決偏離，§6 P7） | id 在 :56；tsRs232（只在 RS232 模式）；網頁 W6：GPIB 模式也可改 | 是 |
| rgStopBit | R:899（預設 0）F:1099；Rs232_Data.Stop_Bit int | [RS-232C] Stop Bit W:399（int 索引） | R:1473 F:1686 | W:934 | 是（另有 5B 補選項＋W6，裁決偏離，§6 P7） | id 在 :56；tsRs232（只在 RS232 模式）；網頁 W6：GPIB 模式也可改 | 是 |
| rgBitLength | R:898（預設 0）F:1098；Rs232_Data.Bit_Length int | [RS-232C] Bit Length W:398（int 索引） | R:1472 F:1685 | W:933 | 是（另有 5B 補選項＋W6，裁決偏離，§6 P7） | id 在 :56；tsRs232（只在 RS232 模式）；網頁 W6：GPIB 模式也可改 | 是 |
| cbbBaudRate | R:883-897（缺 BaudRate 時由舊鍵 Baud Rate 索引 0/1/2→19200/9600/4800 換算並回寫）F:1097；Rs232_Data.Baud_Rate int | [RS-232C] BaudRate W:397（原字串＝Text） | R:1457-1471 F:1684 | W:932 | 是（W6 裁決偏離；golden 可打字，網頁不行 §6 P5） | id 在 :56；tsRs232（只在 RS232 模式）；網頁 W6：GPIB 模式也可改 | 是 |
| edTCPIP_Address | R:778 F:1067（預設 "172.16.8.150"）；asTester_Address AnsiString | [Tester TCPIP] Address W:480（原字串） | R:1358 F:1654 | W:1015 | 是 | id 在 :56；tsTCPIP（只在 TCP/IP 模式） | 否（FIELD_MAP） |
| edTCPIP_Port | R:779 F:1068（預設 6000）；iTester_Port int | [Tester TCPIP] Port W:481（原字串） | R:1359 F:1655 | W:1016 | 是 | id 在 :56；tsTCPIP（只在 TCP/IP 模式） | 否（FIELD_MAP） |
| edtTestingWaitTime | R:679 F:1018（預設 3.0）；dTestingWaitTime double | [Time] Testing Wait Time W:366（"0.00"，ToDouble） | R:1259 F:1605 | W:901 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；grpTestingStopTime Visible=bEnableTestingNeedStopAllMotor&&bI24 G:237 | 否（FIELD_MAP） |
| edtInitWaitTime | R:678 F:1017（預設 1.0）；dInitWaitTime double | [Time] Init Wait Time W:365（"0.00"，ToDouble） | R:1258 F:1604 | W:900 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；grpTestingStopTime Visible=bEnableTestingNeedStopAllMotor&&bI24 G:237 | 否（FIELD_MAP） |
| edMaxBinCount | R:881 F:1095（預設 32）；iRs232MaxBinCount int | [RS-232C] Bin Count W:395（原字串） | R:1455 F:1682 | W:930 | 是 | id 在 :56；gbRs232BinCount 只在 cbRs232Type≠0 顯示 G:1294-1300 | 否（FIELD_MAP） |
| cbEveryFirstDeviceUseInitialDelay | R:690 F:1021（預設 false）；bool | [InitialMode] bEveryFirstDeviceUseInitialDelay W:409（bool 0/1） | R:1270 F:1608 | W:944 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| edtInitialDelay_1 | R:713 F:1028（預設 1.00；註 B）；iInitialDelay double | [InitialMode] iInitialDelay W:419（"0.00"，ToDouble） | R:1293 F:1615 | W:954 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_1_RT | R:747 F:1054（預設 0.0）；dInitialDelay_1_RT double | [InitialMode] dInitialDelay_1_RT W:467（"0.00"，ToDouble） | R:1327 F:1641 | W:1002 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=IniConfig.bI13InitStartDelayHasFTandRT G:299-311 | 否（FIELD_MAP） |
| cbWhenPressStopOverUseInitialDelay | R:735 F:1035（預設 false）；bool | [InitialMode] bWhenPressStopOverUseInitialDelay W:426（bool 0/1） | R:1315 F:1622 | W:961 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| edtInitialDelay_6 | R:718 F:1037（預設 1.00；註 B）；iInitialDelay_6 double | [InitialMode] iInitialDelay_6 W:424（"0.00"，ToDouble） | R:1298 F:1624 | W:959 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edOverSec | R:736 F:1036（預設 1.00）；iWhenPressStopOver double | [InitialMode] iWhenPressStopOver W:427（原字串） | R:1316 F:1623 | W:962 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_6_RT | R:752 F:1059（預設 0.0）；double | [InitialMode] dInitialDelay_6_RT W:472（"0.00"，ToDouble） | R:1332 F:1646 | W:1007 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| cbWhenHappenTestedTimeBelowUseInitialDelay | R:698 F:1024（預設 false）；bool | [InitialMode] bWhenHappenTestedTimeBelowUseInitialDelay W:417（bool 0/1） | R:1278 F:1611 | W:952 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| edtInitialDelay_4 | R:716 F:1031（預設 1.00；註 B）；double | [InitialMode] iInitialDelay_4 W:422（"0.00"，ToDouble） | R:1296 F:1618 | W:957 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edBelowSec | R:699 F:1027（預設 1.00）；iEveryFirstDeviceUseInitialDelay double | [InitialMode] iEveryFirstDeviceUseInitialDelay W:418（原字串） | R:1279 F:1614 | W:953 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_4_RT | R:750 F:1057（預設 0.0）；double | [InitialMode] dInitialDelay_4_RT W:470（"0.00"，ToDouble） | R:1330 F:1644 | W:1005 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| cbAfterOpenHeatDoorUseInitialDelay | R:741 F:1026（預設 false）；bool | [InitialMode] bAfterOpenHeatDoorUseInitialDelay W:435（bool 0/1） | R:1321 F:1613 | W:970 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| edtInitialDelay_5 | R:717 F:1032（預設 1.00；註 B）；double | [InitialMode] iInitialDelay_5 W:423（"0.00"，ToDouble） | R:1297 F:1619 | W:958 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_5_RT | R:751 F:1058（預設 0.0）；double | [InitialMode] dInitialDelay_5_RT W:471（"0.00"，ToDouble） | R:1331 F:1645 | W:1006 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| edtInitialDelay_3 | R:715 F:1030（預設 1.00；註 B）；double | [InitialMode] iInitialDelay_3 W:421（"0.00"，ToDouble） | R:1295 F:1617 | W:956 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| cbAfterAutoCleanFunctionUseInitialDelay | R:738 F:1025（預設 false）；bool | [InitialMode] bAfterAutoCleanFunctionUseInitialDelay W:433（bool 0/1） | R:1318 F:1612 | W:968 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| edtInitialDelay_3_RT | R:749 F:1056（預設 0.0）；double | [InitialMode] dInitialDelay_3_RT W:469（"0.00"，ToDouble） | R:1329 F:1643 | W:1004 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| cbAfterShowAlarmMessageUseInitialDelay | R:697 F:1023（預設 false）；bool | [InitialMode] bAfterShowAlarmMessageUseInitialDelay W:416（bool 0/1） | R:1277 F:1610 | W:951 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| edtInitialDelay_2 | R:714 F:1029（預設 1.00；註 B）；double | [InitialMode] iInitialDelay_2 W:420（"0.00"，ToDouble） | R:1294 F:1616 | W:955 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_2_RT | R:748 F:1055（預設 0.0）；double | [InitialMode] dInitialDelay_2_RT W:468（"0.00"，ToDouble） | R:1328 F:1642 | W:1003 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| cbOTDUnlockDelay | R:734 F:1046（預設 false）；bool | [InitialMode] bOTDUnlockDelay W:441（bool 0/1） | R:1314 F:1633 | W:976 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| edtInitialDelay_9 | R:721 F:1047（預設 1.00；註 B）；double | [InitialMode] iInitialDelay_9 W:442（"0.00"，ToDouble） | R:1301 F:1634 | W:977 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_9_RT | R:755 F:1062（預設 0.0）；double | [InitialMode] dInitialDelay_9_RT W:475（"0.00"，ToDouble） | R:1335 F:1649 | W:1010 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| cbTestFinishToNextTestOver | R:739 F:1042（預設 false）；bool | [InitialMode] bTestFinishToNextTestOver W:437（bool 0/1） | R:1319 F:1629 | W:972 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| iTestFinishToNextTestOver | R:740 F:1043（預設 1.00）；double | [InitialMode] iTestFinishToNextTestOver W:438（"0.00"，ToDouble） | R:1320 F:1630 | W:973 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_8 | R:720 F:1044（預設 1.00；註 B）；double | [InitialMode] iInitialDelay_8 W:439（"0.00"，ToDouble） | R:1300 F:1631 | W:974 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_8_RT | R:754 F:1061（預設 0.0）；double | [InitialMode] dInitialDelay_8_RT W:474（"0.00"，ToDouble） | R:1334 F:1648 | W:1009 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| cbWhenNoFullSiteUseInitialDelay | R:737 F:1039（預設 false）；bool | [InitialMode] bWhenNoFullSiteUseInitialDelay W:429（bool 0/1） | R:1317 F:1626 | W:964 | 是 | id 在 :56；gbInitialDelay（註 A）；pnlNoFullsite 只 CC_TSMC_TAINAN G:281-284（S25） | 是 |
| edtInitialDelay_7 | R:719 F:1040（預設 1.00；註 B）；double | [InitialMode] iInitialDelay_7 W:430（原字串） | R:1299 F:1627 | W:965 | 是 | id 在 :56；gbInitialDelay（註 A）；pnlNoFullsite 只 CC_TSMC_TAINAN G:281-284（S25） | 否（FIELD_MAP） |
| edtInitialDelay_7_RT | R:753 F:1060（預設 0.0）；double | [InitialMode] dInitialDelay_7_RT W:473（"0.00"，ToDouble） | R:1333 F:1647 | W:1008 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| cbUseOtherArmToTestAfterInitialDelay | R:693 F:1022（預設 false；非 bAfterInitialDelayUseOtherArm 固定 false G:692-695；單 ARM 再清 G:964-967）；bool | [InitialMode] bUseOtherArmToTestAfterInitialDelay W:412（bool 0/1；非 bAfterInitialDelayUseOtherArm 時寫 false G:411-414） | R:1273 F:1609 | W:947 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=CosFunction.bAfterInitialDelayUseOtherArm G:286-289 | 是 |
| cbTestStartToNextTestStart | R:742 F:1048（預設 false）；bool | [InitialMode] bTestStartToNextTestStart W:443（bool 0/1） | R:1322 F:1635 | W:978 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| iTeststartToNextTestStart | R:743 F:1049（預設 1.00）；dTeststartToNextTestStart double | [InitialMode] dTeststartToNextTestStart W:444（"0.00"，ToDouble） | R:1323 F:1636 | W:979 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_10 | R:722 F:1050（預設 0.0；註 B）；dInitialDelay_10 double | [InitialMode] dInitialDelay_10 W:445（"0.00"，ToDouble） | R:1302 F:1637 | W:980 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edtInitialDelay_10_RT | R:756 F:1063（預設 0.0）；double | [InitialMode] dInitialDelay_10_RT W:476（"0.00"，ToDouble） | R:1336 F:1650 | W:1011 | 是 | id 在 :56；gbInitialDelay（註 A）；Visible=bI13 G:299-311 | 否（FIELD_MAP） |
| cbUseSocketHeating | R:974 F:1121（預設＝目前值）；bUseSocketHeating bool | [Time] bUseSocketHeating W:489（bool 0/1） | R:1558 F:1708 | W:1024 | 是 | id 在 :56；gbInitialDelay（註 A） | 是 |
| edt_UseSocketHeating | R:975 F:1122（預設 0）；iUseSocketHeating int | [Time] iUseSocketHeating W:490（原字串） | R:1559 F:1709 | W:1025 | 是 | id 在 :56；gbInitialDelay（註 A） | 否（FIELD_MAP） |
| edMaxTestTime | R:620（預設 0.0）＋R:629 FT 份 F:995／986（註 C）；iMaxTime、iFTMaxTime double | [Time] MAX Time W:344（"0.00"，ToDouble） | R:1178,1209 F:1582,1573 | W:879 | 讀寫是；小鍵盤範圍錯 §6 P1；遠端 SETMAXTEST §6 P6 | id 在 :56；rgTime1；PageControl1 權限 [91] G:227 | 否（FIELD_MAP） |
| edInitialMaxTest | R:621（預設＝iMaxTime）＋R:630 F:994／985（註 C）；double | [Time] Initial MAX Time W:343（"0.00"，ToDouble） | R:1179,1210 F:1581,1572 | W:878 | 讀寫是；小鍵盤範圍錯 §6 P1；遠端 SETINITIALMAXTEST §6 P6 | id 在 :56；rgTime1；PageControl1 權限 [91] G:227 | 否（FIELD_MAP） |
| edDummyTestTime | R:622（預設 0.0）＋R:631 F:996／987（註 C）；double | [Time] Dummy Time W:345（"0.00"，ToDouble） | R:1180,1211 F:1583,1574 | W:880 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；rgTime1；PageControl1 權限 [91] G:227 | 否（FIELD_MAP） |
| edStartDelayTime | R:623（預設 0.0）＋R:632 F:997／988（註 C）；double | [Time] Stary Delay W:346（"0.00"，ToDouble） | R:1181,1212 F:1584,1575 | W:881 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；rgTime1；PageControl1 權限 [91] G:227 | 否（FIELD_MAP） |
| edtInitStartDelay | R:625（預設 0.0；HiSilicon+ASE_KaohSiung 下限 30 G:725-732，S25）＋R:633 F:999／989（註 C）；double | [Time] Initial Stary Delay W:348（"0.00"，ToDouble） | R:1205,1213 F:1586,1576 | W:883 | 讀寫是；小鍵盤下限 30 錯 §6 P2 | id 在 :56；rgTime1；Visible=IniConfig.bInitialStartDelayCount G:230-235 | 否（FIELD_MAP） |
| edtInitStartDelayCT | R:626（預設 0）＋R:634 F:1000／990（註 C）；int | [Time] Initial Stary Delay CT W:349（原字串） | R:1206,1214 F:1587,1577 | W:884 | 是 | id 在 :56；rgTime1；Visible=IniConfig.bInitialStartDelayCount G:230-235 | 否（FIELD_MAP） |
| edtAfterTestedDelay | R:682 F:1033（預設 0.0）；dAfterTestedDelay double | [Time] After Tested Delay W:369（"0.00"，ToDouble） | R:1262 F:1620 | W:904 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；rgTime1；Visible=CosFunction.bEnableAfterTestedDelay G:239-241 | 否（FIELD_MAP） |
| iStartDelayCount | R:760 F:1106（預設 0）；iEnStartDelayCount int | [InitialMode] iEnStartDelayCount W:447（int 索引） | R:1340 F:1693 | W:982 | 是 | id 在 :56；rgTime2（註 D） | 是 |
| edtInitialDec1 | R:761 F:1107（預設 0.00）；dInitialStartDelayDec[0] double | [InitialMode] dInitialStartDelayDec1 W:448（"0.00"，ToDouble） | R:1341 F:1694 | W:983 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；rgTime2（註 D） | 否（FIELD_MAP） |
| edtInitialDecCount1 | R:762 F:1108（預設 0）；iStartDelayCount[0] int | [InitialMode] iStartDelayCount1 W:449（原字串） | R:1342 F:1695 | W:984 | 是 | id 在 :56；rgTime2（註 D） | 否（FIELD_MAP） |
| edtInitialDec2 | R:763 F:1109（預設 0.00）；dInitialStartDelayDec[1] double | [InitialMode] dInitialStartDelayDec2 W:450（"0.00"，ToDouble） | R:1343 F:1696 | W:985 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；rgTime2（註 D） | 否（FIELD_MAP） |
| edtInitialDecCount2 | R:764 F:1110（預設 0）；iStartDelayCount[1] int | [InitialMode] iStartDelayCount2 W:451（原字串） | R:1344 F:1697 | W:986 | 是 | id 在 :56；rgTime2（註 D） | 否（FIELD_MAP） |
| edtInitialDec3 | R:765 F:1111（預設 0.00）；dInitialStartDelayDec[2] double | [InitialMode] dInitialStartDelayDec3 W:452（"0.00"，ToDouble） | R:1345 F:1698 | W:987 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；rgTime2（註 D） | 否（FIELD_MAP） |
| edtInitialDecCount3 | R:766 F:1112（預設 0）；iStartDelayCount[2] int | [InitialMode] iStartDelayCount3 W:453（原字串） | R:1346 F:1699 | W:988 | 是 | id 在 :56；rgTime2（註 D） | 否（FIELD_MAP） |
| edtInitialDec4 | R:767 F:1113（預設 0.00）；dInitialStartDelayDec[3] double | [InitialMode] dInitialStartDelayDec4 W:454（"0.00"，ToDouble） | R:1347 F:1700 | W:989 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；rgTime2（註 D） | 否（FIELD_MAP） |
| edtInitialDecCount4 | R:768 F:1114（預設 0）；iStartDelayCount[3] int | [InitialMode] iStartDelayCount4 W:455（原字串） | R:1348 F:1701 | W:990 | 是 | id 在 :56；rgTime2（註 D） | 否（FIELD_MAP） |
| cbPurgeAir | R:775 F:1116（預設 false）；bPurgeAirAfterContract bool | [InitialMode] bPurgeAir W:462（bool 0/1） | R:1355 F:1703 | W:997 | 是 | id 在 :56；rgPurgeAir 頁籤只 ASE_KaohSiung 或 bSPILFunction G:276-277，且 SW[SwPurgeAir] G:325-328 | 是 |
| edPurgeAir | R:776 F:1117（預設 0）；iPurgeAirContract int | [InitialMode] iPurgeAir W:463（原字串） | R:1356 F:1704 | W:998 | 是 | id 在 :56；rgPurgeAir 頁籤只 ASE_KaohSiung 或 bSPILFunction G:276-277，且 SW[SwPurgeAir] G:325-328 | 否（FIELD_MAP） |
| edMaxTestTime_RT | R:636（預設＝iMaxTime；註 E）F:1004；iRTMaxTime double | [Time] RT MAX Time W:352（原字串） | R:1216 F:1591 | W:887 | 讀寫是；小鍵盤範圍錯 §6 P1 | id 在 :56；tsRT 頁籤 Visible=IniConfig.bSPILFunction G:160-169 | 否（FIELD_MAP） |
| edInitialMaxTest_RT | R:637（預設＝iInitialMaxTime；註 E）F:1003；double | [Time] RT Initial MAX Time W:351（原字串） | R:1217 F:1590 | W:886 | 讀寫是；小鍵盤範圍錯 §6 P1 | id 在 :56；tsRT（同上） | 否（FIELD_MAP） |
| edDummyTestTime_RT | R:638（預設＝iDummyTime；註 E）F:1005；double | [Time] RT Dummy Time W:353（原字串） | R:1218 F:1592 | W:888 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；tsRT（同上） | 否（FIELD_MAP） |
| edStartDelayTime_RT | R:639（預設＝dStartDelayTime；註 E）F:1006；double | [Time] RT Stary Delay W:354（原字串） | R:1219 F:1593 | W:889 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；tsRT（同上） | 否（FIELD_MAP） |
| edtInitStartDelay_RT | R:640（預設＝dInitStartDelayTime；註 E）F:1007；double | [Time] RT Initial Stary Delay W:355（原字串） | R:1220 F:1594 | W:890 | 讀寫是；小鍵盤下限 30 錯 §6 P2 | id 在 :56；tsRT（同上） | 否（FIELD_MAP） |
| edtInitStartDelayCT_RT | R:641（預設＝iInitStartDelayTimeCT；註 E）F:1008；int | [Time] RT Initial Stary Delay CT W:356（原字串） | R:1221 F:1595 | W:891 | 是 | id 在 :56；tsRT（同上） | 否（FIELD_MAP） |
| edMaxTestTime_EQC | R:643（預設＝iMaxTime；註 E）F:1011；iEQCMaxTime double | [Time] EQC MAX Time W:359（原字串） | R:1223 F:1598 | W:894 | 讀寫是；小鍵盤範圍錯 §6 P1 | id 在 :56；tsEQC 頁籤 Visible=IniConfig.bSPILFunction G:160-169 | 否（FIELD_MAP） |
| edInitialMaxTest_EQC | R:644（預設＝iInitialMaxTime；註 E）F:1010；double | [Time] EQC Initial MAX Time W:358（原字串） | R:1224 F:1597 | W:893 | 讀寫是；小鍵盤範圍錯 §6 P1 | id 在 :56；tsEQC（同上） | 否（FIELD_MAP） |
| edDummyTestTime_EQC | R:645（預設＝iDummyTime；註 E）F:1012；double | [Time] EQC Dummy Time W:360（原字串） | R:1225 F:1599 | W:895 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；tsEQC（同上） | 否（FIELD_MAP） |
| edStartDelayTime_EQC | R:646（預設＝dStartDelayTime；註 E）F:1013；double | [Time] EQC Stary Delay W:361（原字串） | R:1226 F:1600 | W:896 | 讀寫是；小鍵盤吃掉小數 §6 P3 | id 在 :56；tsEQC（同上） | 否（FIELD_MAP） |
| edtInitStartDelay_EQC | R:647（預設＝dInitStartDelayTime；註 E）F:1014；double | [Time] EQC Initial Stary Delay W:362（原字串） | R:1227 F:1601 | W:897 | 讀寫是；小鍵盤下限 30 錯 §6 P2 | id 在 :56；tsEQC（同上） | 否（FIELD_MAP） |
| edtInitStartDelayCT_EQC | R:648（預設＝iInitStartDelayTimeCT；註 E）F:1015；int | [Time] EQC Initial Stary Delay CT W:363（原字串） | R:1228 F:1602 | W:898 | 是 | id 在 :56；tsEQC（同上） | 否（FIELD_MAP） |

註：
- **A** gbInitialDelay：Visible＝熱或恆溫模式或 CC_KYEC_LEE（G:195-197）；可改＝AccessLevel>=LevelSet.AccessLevel[123]（G:279）。Boost 功能開時 10 個勾選在讀檔後被清成 false（G:950-962），移植同（N:1531-1543）。
- **B** SOFT_SIMULTE 編譯時 iInitialDelay～dInitialDelay_10 讀檔改成固定 5（G:701-712），移植同（N:1281-1291）。
- **C** CosFunction.bFTRTDiffInitStartDelayTime 時，畫面顯示的是 FT 那一份（G:983-991），執行值依 RT／QC／FT 換（G:649-675）；移植同（N:1207-1256、N:1570-1578）。
- **D** rgTime2 頁籤：906 是 CC_ASE_KaohSiung 或 CC_AMD_M（G:275），912／移植是 CC_ASE_KaohSiung 或 IniConfig.bAMDFunction（N:806）；客戶專屬（S25）。
- **E** RT／EQC 這 12 個只在 CosFunction.bFTRTDiffInitStartDelayTime 時才從檔案讀（G:627），但存檔一律寫（G:351-363，而且寫原字串不是 FormatFloat）—— golden 本身的怪處，移植照搬（§9）。

## 4. golden 表單上有、但 golden 自己不讀不寫的元件（17 個）＋按鈕

| 元件 | DFM | golden 讀寫 | 頁面 | 結論 |
|---|---|---|---|---|
| cb_NeedSendVSOT（TCheckBox） | `cTesterIF.dfm:340`，Visible=False | 讀 G:902、寫 G:402、填 G:1101 全部註解掉 | id 在 :56，display:none | 一致（不讀不寫）；舊 PENDING 把它列成 `[RS-232C] NeedVSOT` 是錯的 |
| rgBaudRate（TRadioGroup） | `:425`，Visible=False | 寫 G:396、填 G:1096 註解掉；G:885 只在缺 BaudRate 時讀舊鍵 "Baud Rate" 做換算，不碰元件 | id 在 :56，display:none | 一致；舊 PENDING 把它對到 `[RS-232C] BaudRate`（其實是 cbbBaudRate 的鍵）是錯的 |
| SLT 分頁 tsSLTSetting 的 15 個：cbRunCheckProgramUse、edtRunCheckProgramName、edtRunCheckProgramNameErrCode、cbMaxBIOSWaitTimeAlm、cbMinTestTimeAlm、cbMaxTestTimeAlm、edMinTestTimeErrCode、edBIOSErrCode、edMaxTestTimeErrCode、edTestOKWaitTime、edSendNEXTDelay、edMaxBIOSWaitTime、edMinTestTime、edPowerSwitchDelay、edSLTMaxTestTime | `:490-936` | cTesterIF.cpp 沒有任何讀寫（只有小鍵盤 G:1535-1548）；`SendSLTState` 只在 `cTesterIF.h:306` 宣告、906 沒有本體；全樹沒有 `FTestIF->` 參照；`cTesterIF.h:244-248` 自己註「沒用到的edit」 | id 都在 :56，分頁看得到，改了不會存 | 一致（golden 是死畫面） |

按鈕（只列會寫檔的）：
- **spbSave**（spbSaveClick G:1302-1356）：寫 Tester.Data（SaveSetupFile）、`#ifdef ASE_KaohSiung` 另存 JOBFILE（G:1317-1319，移植 N:1894-1896 原樣保留）、SECS SaveRecipe、ReadTestIFFile（會寫，見 §1）、Recipe Parameter Default（Greatek，移植 ELTodo N:1906）、BackupSetupFile、SetWorkParameter。移植＝`FileRW/TestIF_File_TesterIF.cpp:76-118`＋N:1873-1937，存檔成功後再跑 FormClose＋sbTesterClick 尾段（`:540-569`，Q41 TI-4／TI-5，時機 S88 Q1 待決）。
- **sbtExit**（sbtExitClick G:1358-1366）：TTL 時 CheckTTLBoardBitMode，Close → FormClose（G:1197-1217，ReadTestIFFile 會寫）。移植＝關頁掛勾 `FileRW/TestIF_File_TesterIF.cpp:590-604`（§6 P8 的小差別）。
- **btTesterTCPShow**（G:1516-1519）：只開 fTesterTCP，不寫檔；移植 `ht9045_testerif_c_wire.js:176-182`。
- DoSetRPDefault（G:1418-1442，寫 `D:\HT9045\IniData\RPDefault.ini`）不是這頁的按鈕（由 RPDefault 表單呼叫），Greatek 專用，未移植。

## 5. 舊 PENDING 逐條（`web/page/ht9045_testerif_wire.js:101-130`）

這支檔的 FIELD_MAP（`:32-98`，60 個純文字欄位）與 PENDING（28 個）是 `make_wire.py` 從 golden 機械抽的 B 路接線，**現在完全不用**：頁面不載入它（`Setup.TesterIF.html:118-128`），而且引擎對這頁一律走 C 路（`ht9045_wire_engine.js:1066`、`:1314`、`:1791`）。

| 行 | PENDING 寫的 | golden 實際 | C 路有沒有 | 結論 |
|---|---|---|---|---|
| :102 | cbDIOType [DIO] Type select | [DIO] Type＋TypeName（G:371-372） | 有（N:906-907） | 死 |
| :103 | rg2DID_Format [GP-IB] 2DIDFormat | 同（G:391） | 有（N:926） | 死 |
| :104 | cbGPIBType [GP-IB] Type | 同（G:385） | 有（N:920） | 死 |
| :105-118 | 14 個 InitialMode 勾選（cbAfterAutoClean…、cbAfterOpenHeatDoor…、cbAfterShowAlarm…、cbEveryFirstDevice…、cbOTDUnlockDelay、cbPurgeAir、cbSpiroxTesterLotEnd、cbASEJPMode、cbTestFinishToNextTestOver、cbTestStartToNextTestStart、cbUseOtherArm…、cbWhenHappenTestedTimeBelow…、cbWhenNoFullSite…、cbWhenPressStopOver…） | 鍵名都對（G:409-482） | 都有（N:944-1017） | 死 |
| :119 | iStartDelayCount [InitialMode] iEnStartDelayCount | 同（G:447） | 有（N:982） | 死 |
| :120 | rgInterfaceType [Mode] Tester Type | 同（G:341） | 有（N:876） | 死 |
| :121 | rgBaudRate [RS-232C] BaudRate | **錯**：golden 不存 rgBaudRate（§4） | 不需要 | 死（抽錯） |
| :122 | cbbBaudRate [RS-232C] BaudRate | 同（G:397，存 Text） | 有（N:932） | 死 |
| :123 | rgBitLength [RS-232C] Bit Length | 同（G:398） | 有（N:933） | 死 |
| :124 | cbForEgistec [RS-232C] ForEgisTecTest | 同（G:403-406） | 有（N:938-941） | 死 |
| :125 | cb_NeedSendVSOT [RS-232C] NeedVSOT | **錯**：golden 讀寫都註解掉（§4） | 不需要 | 死（抽錯） |
| :126 | rgParity [RS-232C] Parity | 同（G:400） | 有（N:935） | 死 |
| :127 | rgStopBit [RS-232C] Stop Bit | 同（G:399） | 有（N:934） | 死 |
| :128 | cbAutoOnecycleHomStart [Time] bAutoOnecycleHomStart | 同（G:485，只 GIGAS） | 有（N:1020） | 死 |
| :129 | cbUseSocketHeating [Time] bUseSocketHeating | 同（G:489） | 有（N:1024） | 死 |

舊檔兩邊都沒列、C 路有的：AntiSignalCBox（G:373／N:908）、cbRs232Type（G:394／N:929）。
census C3-228（AntiSignalCBoxClick「沒接」）也可以關：存檔前 BeforeApply 照 golden 重播（`FileRW/TestIF_File_TesterIF.cpp:173-182`），cbASEJPMode 照 DFM 綁同一支。

## 6. 頁面端的問題（C++ 讀寫以外）

頁面 `Setup.TesterIF.html` 載入順序（`:118-128`）：qwerty.js → recipe_client → wire_engine → `ht9045_wire_testerif.js`（:121）→ `ht9045_wire_setuptesterif.js`（:125，後註冊者生效）→ `ht9045_testerif_c_wire.js`（:128）。值與可見／可改由後端 golden FormShow 給（gbApply），**小鍵盤設定（kb）則來自產生的 `ht9045_wire_*.js`**，而且機台上小鍵盤是唯一輸入方式（`ht9045_wire_engine.js:2027-2047` 設 readonly）。

**P1 最長測試時間 6 欄（edMaxTestTime、edInitialMaxTest，＋_RT、_EQC）**
- golden：OnClick＝edInitialMaxTestClick（`cTesterIF.dfm:2348-2366`、`:3100-3118`、`:3380-3398`），G:1380-1403：CC_ASE_M `N_INTEGER 60～9999`（:1384）；CC_AMKOR_Korea／CC_SCK `N_INTEGER` 不限（:1389）；CC_GIGAS `N_DOUBLE 2、0～50000`（:1393）；IniConfig.bVTESTFunction `N_DOUBLE 2、0～36000`（:1397）；其他 `N_DOUBLE 2、0～15000`（:1401）。
- 網頁：`['INTEGER', 0, true, 60, 9999]`（`ht9045_wire_setuptesterif.js:100-102`、`:104-106`；`ht9045_wire_testerif.js:97-99`、`:101-103` 同）＝抄到 CC_ASE_M 那支。
- 後果：`qwerty.js:68-71` OK 時夾限＋整數化 → 寫入值≠輸入值。
- 修法：一般分支 `['DOUBLE', 2, true, 0, 15000]`；bVTESTFunction 要後端帶旗標（ExtraJson）。四個客戶分支 S25 只註記。

**P2 Initial Start Delay 3 欄（edtInitStartDelay，＋_RT、_EQC）**
- golden：OnClick＝edtInitStartDelayClick，G:1410-1416：HiSilicon 且 CC_ASE_KaohSiung `30～3000`（:1413），其他 `N_DOUBLE 2、不檢查`（:1415）。
- 網頁：`['DOUBLE', 2, true, 30.0, 3000.0]`（`ht9045_wire_setuptesterif.js:115`、`:119`、`:120`）。
- 修法：`['DOUBLE', 2, false, 0, 0]`（客戶分支 S25 註記）。

**P3 N_DOUBLE、dp＝0 的 13 欄（edMaxTestTimeMouseDown G:1185-1189：`N_DOUBLE, 0, true, 0.0, 5000.0`）**
- 欄位：edDummyTestTime、edDummyTestTime_RT、edDummyTestTime_EQC、edStartDelayTime、edStartDelayTime_RT、edStartDelayTime_EQC、edtAfterTestedDelay、edtInitWaitTime、edtTestingWaitTime、edtInitialDec1～4。網頁 kb 照抄 `['DOUBLE', 0, true, 0.0, 5000.0]`（對），問題在 `qwerty.js:71` `toFixed(opt.dp || 0)`。
- golden：dp 只設定 ±鍵是 ±10／±1／±0.1…（`myQwertyKeyBoard.cpp:368-418`），OK 後 `AnsiString(CheckRange(d, min, max))`（`:290`）不動小數。
- 修法（共用檔，影響所有頁）：夾限後照 golden 輸出數字字串，不用 dp 去 toFixed。

**P4 RS232 那組該停用沒停用（已知）**：`docs/handoff/TO_STEVEN.md:347`（Ifor01 → Steven，20261001 11:2x）：golden 依條件停用 tsRs232／grpIFType、後端也回 not editable，但頁面 8 個元件還能點；存檔時伺服器丟值（ELEditable），不寫錯檔。相關碼：`ht9045_testerif_c_wire.js:135-145`（applyEditable）、`:187-214`（W6 的 w6Rs232InGpib／w6ApplyEditable）。本稽核沒有重跑探針。

**P5 下拉不能打字**：DFM 五個 TComboBox 都沒有 Style（`cTesterIF.dfm:85`、`:181`、`:318`、`:459`、`:2776`）＝csDropDown；頁面是 `<select>`。只有 cbbBaudRate 有實質影響（存 Text）；引擎讀到清單外的值會補一個選項原樣帶回（`ht9045_wire_engine.js:1155-1157`），所以不會改壞檔案，只是新的非清單值打不進去。

**P6 遠端設定最長時間沒更新頁面**：`Command.cpp:3675-3696`（SetMaxTest）、`:3707-3723`（SetMaxInitialTest）寫 `TestIF_File`、Tester.Data，以及 `FTestIF->edMaxTestTime->Text`／`->edInitialMaxTest->Text`——這個 FTestIF 是 `forms/fTesterIF.cpp:52` 的 facade，它的兩個 TEdit 是自己 `new` 的（`forms/fTesterIF.h:428-429`），不是頁面用的 C 路替身。golden（`Command.cpp:12474`、`:12495`）改的是表單自己的 edit。頁面開著時：畫面舊值 → 存檔 → 舊值蓋回去。另外 `Automation/auto9045.cpp:1285`、`:1297`、`:1310`、`:1322` 也遠端寫同幾個鍵，但 golden 那邊本來就不更新表單，移植一致。

**P7 已裁決的偏離**（列給審查參考，不用修）：W6（`FileRW/TestIF_File_TesterIF.cpp:330-388`、`ht9045_testerif_c_wire.js:184-214`）；5B 新選項（`FileRW/TestIF_File_TesterIF.cpp:258-265`、`Setup.TesterIF.html:56`、`TesterComm/Rs232SetupCodes.h`）；RS232 不合法組合拒存（`:92-106`）；數字預檢、DIO 遺失拒存（`:61-91`）。

**P8 Exit 少一個檢查**：golden sbtExitClick（G:1358-1366）在畫面選 DIO（rgInterfaceType==0，可能還沒存）時先 CheckTTLBoardBitMode；移植關頁只跑 FormClose 那段（檢查檔案值）。census C3-260 已列 sbtExitClick。

## 7. 其他會讀寫同一批鍵的地方

| 位置 | 讀／寫 | golden 對應 | 狀態 |
|---|---|---|---|
| `FileRW/Temperature.gen.inc:6146-6186` | 寫 Tester.Data [InitialMode] 一部分（TfTemp_Set 自己那份元件） | `uTemp_Set.cpp:5029-5069` | 一致（golden 也是兩個寫者） |
| `FileRW/Temperature.gen.inc:89-146`、`:4224`、`:6205` | 存檔後只重讀 [InitialMode] 子集 | `uTemp_Set.cpp:3149`、`:5088` 完整 ReadTestIFFile | 子集（census C2-057／058）→ St01 可改呼叫 FileRW_TesterIF_ReadTestIFFile |
| `FileRW/IniConfig.gen.inc:7974-7977` | 設定頁存檔後應重讀，現為 ELTodo | `cConfiguration.cpp:7287` | 閘 → St01 同上 |
| `FileRW/TTLCfg.cpp:181`、`:236` | 讀 [DIO] TypeName | golden G:797／:830 | 一致（[DIO] 子集，開機已改用完整版 `tools/wb_serve.cpp:2995`） |
| `Command.cpp:3693`、`:3720` | 寫 [Time] MAX Time／Initial MAX Time | `Command.cpp:12475`、`:12496` | 寫檔一致；畫面替身沒更新（§6 P6） |
| `Automation/auto9045.cpp:1285-1322` | 寫 [Time] 四鍵 | golden `Automation/auto9045.cpp` | 一致 |
| `TesterComm/Handler/HandlerGpibAux.cpp:150-154` | GPIB 模式配方第一次由 Setup.ini 種 [RS-232C] BaudRate／Bit Length／Stop Bit／Parity，另寫標記鍵 `W906_SeededFromSetupIni`（`HandlerGpibAux.h:27`，golden 沒有的鍵）；之後叫 `FileRW_TesterIF_DoIniDataToForm` 刷新替身（`FileRW/TestIF_File_TesterIF.cpp:292-293`） | 無（P6 裁決 2A／Q2 (a)） | 裁決過的移植專屬寫者 |
| `csystem.cpp:27297-27298`（GATE H3-6）、`forms/fBuilder.cpp:859-905`（GATE B-9）、`ProductionInfo/uPAT_Function.cpp:2063-2073`（GATE G12） | 整份另存時 DoIniDataToForm＋SaveSetupFile | `csystem.cpp:22562-22563`、`cBuilder.cpp:517-563` | 全部 `#if 0`；之後誰解閘，要對 C 路替身提供一個公開的 SaveSetupFile 入口（現在 TIF_SaveSetupFile 是 static） |
| `forms/fTesterIF.cpp` | 整份 golden 讀寫本體 | cTesterIF.cpp | 全部 `#if 0`（GATE F-1～F-17），不是活的 |

## 8. 906 與 912 的差異（產生器用 912）

`cTesterIF.h`、`cTesterIF.dfm` 906／912 逐位元相同；`cTesterIF.cpp` 只差 81 行 diff：
1. rgTime2 頁籤條件 CC_AMD_M → IniConfig.bAMDFunction（906 G:275／912 :276）——客戶專屬（S25）。
2. 「AMD Function」與 2DIDFormat 的預設條件 CC_AMD_M → IniConfig.bAMDFunction（906 G:861、G:870／912 :875、:884）——rg2DID_Format 的**預設值**在「不是 CC_AMD_M 但 bAMDFunction 開著」的機台上兩版不同；移植照 912（第 37 條）。
3. 912 多讀 [AMR] 四鍵（912 :626-646），表單沒有對應元件（Lot 頁用）；移植照 912（RULINGS_20260926 第 10 條補了 cprod.h 欄位）。
4. 912 iTestBinCount 多 KYEC_LEE AOI（:937）、bD58 時多送 1Arm／2Arm 訊息（:956-959）。
5. 912 spbSaveClick 在 A02 權限不足時 Close 後 `return`（:1327）；906 會繼續存檔。移植照 912（不寫）。

## 9. golden 本身的怪處（照搬，不算移植錯）

- RT／EQC 12 欄只有 bFTRTDiffInitStartDelayTime 時才讀，但每次存檔都寫（註 E）：bSPILFunction 開、bFTRTDiff… 關的機台，存一次就會把 RT／EQC 鍵寫成 struct 現值（通常 0.00）。
- ReadTestIFFile 名字是讀，實際會寫 Tester.Data 三處與 Setup.ini（§1）；開頁、關頁、存檔後都會跑。
- AntiSignalCBox 顯示的是執行期 TestIF.bAntiSignal，不是檔案值（G:125）。
- edt_UseSocketHeating 用 N_DOUBLE 小鍵盤（edtInitialDelay_1Click），存原字串，讀回是 int（G:975）。
- SLT 分頁 15 個元件不讀不寫（§4）。

## 10. 檔案擁有者

`git log --format='%h %an %s' -- <file>`（作者欄都是 "Steven" 的，用 commit 標題判斷是 St01 還是 St02）：

| 檔案 | 最近 commit | 擁有者 |
|---|---|---|
| `FileRW/TestIF_File_TesterIF.cpp` | `64738609`、`f88b9ffd`、`102ed625`、`38971aab`、`e6e90401`（St02-E helper，Q41／ELA），`7aa13ca3`（St02 P6），建立 `8af13c07`（St01） | St01 建立；20260926 起 St01 同意 St02 直接改（`docs/handoff/FROM_STEVEN.md:151`） |
| `FileRW/TestIF_File_TesterIF.gen.inc`、`tools/editlist/TestIF_File_TesterIF.py` | `29554f87`（合併）、`7aa13ca3`（St02 P6）、`3e0ebb92`（產生器改 912）、`551c7398`（jimmychiu，第 10 條 [AMR]）、`8af13c07`（St01） | 產生檔；改 .py 要一起重產並 commit（`docs/handoff/CHAT_ST01.md:21`） |
| `web/page/Setup.TesterIF.html` | `5d87abcd`（NOOVERLAP-C，作者 HT9045 Machine，jimmychiu commit）、`e6e90401`（St02 W6 選項）、`8af13c07`（St01）、`a016aa01`（Steven 0923 匯入） | 共用 |
| `web/page/ht9045_testerif_c_wire.js` | `e6e90401`（St02 Q41）、`8af13c07`（St01 建立，手寫） | St01 建立，St02 改過 |
| `web/page/ht9045_testerif_wire.js` | `55302732`（jimmychiu，0923 web 進版控的三方合併） | **筆電合併的產生檔**：檔頭 `:3`「機器產生（scratchpad/make_wire.py）」，產生器 `tools/pagewire/make_wire.py`（jimmychiu `59444a0c`／`119a51d0`），部署清單 `tools/websync/sync_web.py:173`；沒有頁面載入 |
| `web/page/ht9045_wire_testerif.js`、`ht9045_wire_setuptesterif.js` | `a016aa01`（Steven 0923 匯入） | 產生檔（檔頭「這個檔是產生的；標記由 gen_wire.py 的樣板寫入」，`scratchpad/gen_wire.py`） |
| `forms/fTesterIF.cpp`／`.h` | `18a49487`、`b51f6ab5`（jimmychiu，0828 FW3-TIF1） | 筆電；全部閘住 |
| `web/page/qwerty.js` | `a016aa01`（Steven 0923 匯入，之後沒人改） | 共用，改之前先認領 |

## 附錄：方法與重跑

腳本都在 `C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\tif\`，用 `D:\HT9045\.venv\Scripts\python.exe` 跑：
- `dec.py`／`dec912.py`：golden cp950 轉 UTF-8、906 對 912 的 diff。
- `dfm_scan.py`：DFM 110 個輸入類元件與事件、Visible／Enabled。
- `cmp_calls.py`：ReadIniData／WriteIniData／CheckIniData 逐條比（替身寫法正規化後）。
- `cmp_body.py`：ReadTestIFFile／DoIniDataToForm／SaveSetupFile／spbSaveClick／FormShow／CheckRs232StandardIni 逐行比。
- `fieldtypes.py`：`SYSTEM_TEST_IF` 欄位型別 golden 對移植。
- `rows.py`／`mdrows.py`／`finalize.py`：§3 的行號。
- `presence.py`、`html_scan.py`：頁面 id、mustSend、舊 FIELD_MAP／PENDING 對照。
- `keyscan.py`、`goldscan.py`、`ftestif_refs.py`、`gatecheck.py`：同一批鍵的其他讀寫者與 `#if 0` 判斷。

限制：純靜態。P4 引用 Ifor01 的探針結果，本次沒重跑；qwerty／kb 的行為是讀碼推論，沒有在瀏覽器實測。
