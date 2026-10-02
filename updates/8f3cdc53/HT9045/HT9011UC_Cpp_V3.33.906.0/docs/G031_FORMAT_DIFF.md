# G-031 舊版 → 新版格式差異清單（第一步：差異清單）

> **寫給**：St02（做轉換器）、Jimmy。**作者**：Ifor01（TO_IFOR.md I-02，20261001）。**範圍**：唯讀分析，不動任何人的檔。
> **工具**：`tools/g031_format_diff.py`（唯讀、可重跑）。**工具原始輸出**：`docs/G031_FORMAT_DIFF_DETAIL.md`（逐鍵明細）。
> 本檔是**人工核過**的清單：工具抓到的每一類差異都回原始碼確認過，假象（寫法不同、路徑沒解出來）另外標明，不算差異。
> 「舊版」＝量產 BCB6 程式：V912（`HT9011UC_Code_V3.33.912.0_20260908_Jimmy`，目前量產）與 V899（前一版）；
> 「移植樹」＝本樹 `HT9011UC_Cpp_V3.33.906.0`；golden 906＝`HT9011UC_Code_V3.33.906.0_20260618`（翻譯原文）。

## 0. 結論（先看這段）

1. **INI 文字檔的格式本身不用轉。** 移植樹的 `TIniFile`／`TMemIniFile`（`vclcompat/IniFiles.*`）照 Win32／Borland 的行為重寫，
   區段、鍵、值的寫法逐位元組相同（`docs/KNOWLEDGE.md:470-480` 對 kernel32 量過）；讀寫函式 `common.cpp` 是 1:1 翻譯。舊檔可以直接讀。
   已知殘差三點見 §7。
2. **轉換器一定要處理的只有四類：**
   - **二進位檔版面**（§1）：`tech.dat`（V912 跟 906／移植樹同大小、不同版面）、`machinerecord.dat`（V899 不相容）；`lastdata.dat` 只是尾端加長，可直接讀。
   - **同一個鍵、預設值不同**（§2）：只有兩個。缺這個鍵的舊檔，新版會用新預設值並寫回，行為因此改變。
   - **舊版讀檔時順手做的遷移**（§3）：改鍵名、改區段名、tech.dat → teach.ini。移植樹都有對應，只有 teach.ini 的 shuttle 區段規則不同。
     20261002 補一項：V899 的 ATC 機台升級時，常溫用的溫度校正表要從 `Temperature_ATC.Data` 複製成 `Temperature_ATC_Cold.Data`（V910 起分冷熱）。
   - **配方資料夾的 `.MD5` 檔要重算**（§5）。
3. **其他差異都不用轉：**
   - 舊版有、移植樹沒翻的模組留下的鍵，原封保留即可（§4.1）；
   - 移植樹新加的鍵，第一次執行時會自己補上（§4.2）；
   - 每份配方裡都有一大批**歷代遺留、沒有任何一版程式在讀的舊鍵**，保留無害（§6）。

## 1. 二進位檔（`system\*.dat`，整個結構直接寫進檔案）

量法：`tools/nb2_assist/struct_layout_across_trees.py`（NB2 R24，BCB6 `-a8` 對齊模型；它算出的 V899 `TECH` 大小 3,792 正好等於機台上實際的檔案大小）。
明細在 `docs/G031_FORMAT_DIFF_DETAIL.md` 最後一節。

| 檔案 | 結構（宣告處） | V899 | V912 | golden 906＝移植樹 | 轉換器要做的事 |
|---|---|---|---|---|---|
| `tech.dat` | `TECH`（`LastSet.h`） | 408 欄、3,792 bytes | 427 欄、3,872 bytes，**但 `M_In_iRotateA_Backlash`／`M_Out_iRotateA_Backlash` 從 byte 724（第 161 欄）搬到結尾** | 427 欄、3,872 bytes＝V899＋結尾 19 個 SortArm 欄位（對 V899 是前綴相容） | 見下方說明 |
| `lastdata.dat`（＋`_backup`、`_backup2`） | `LAST_GENERAL_SET`（`LastSet.h`） | 382 欄、178,096 | 389 欄、179,928 | 906：387 欄、178,896；**移植樹＝V912**（179,928） | 不用轉：新增欄位全在結尾，`ReadData`（`cprod.cpp:1438`）讀短檔只填到檔案長度，其餘保留初值 |
| `machinerecord.dat`（＋`machinerecordRealCCD.dat`） | `MachRec`（`cinitial.cpp` 內） | **不相容**：少了中段的 `SortCarryKitItem[4][8]`（接在 `BRCarryKitItem` 後面，之後每個 byte 位移 128）與結尾的 `cTrayID[20][2048]` | ＝906 | 58 欄 | **V899 → 新版要重排**：插入點之前照抄、插入 128 bytes（`SortCarryKitItem` 填 0）、之後照抄、結尾補 `cTrayID` 0。V899 實檔 835,820 bytes（`docs/reports/RD5軟體_HT9046LS導入V899可行性評估_20260812_101500.md:23`），新版 876,908（這台 `system\machinerecord.dat` 實際大小） |
| `login.dat` | `PASS_WORD`（`cprod.h`） | 相同 | 相同 | 相同 | 不用轉 |
| `levelset.dat` | `LAST_LEVEL_SET`（`cprod.h`） | 相同 | 相同 | 相同 | 不用轉 |
| `AutoTeach_*.dat` | `AUTOTEACH_POINT`（`cprod.h`） | 相同 | 相同 | 相同 | 不用轉 |

**`tech.dat` 的處理**：

- 移植樹**只讀不寫**：`FileRW/Teach.cpp:149-155`，等使用者裁決（NIGHT_REPORT §0 第 6 件）。
- 移植樹只在 teach.ini 缺 `[Teach INI] Update2`（或缺 `[MInShuttle1/2]`）時讀 tech.dat 做遷移：`forms/fTeachPara.cpp:407`／`:453`，對應 V912 `uteach.cpp:4811`／`:4849`。讀到的值會永久寫進 teach.ini。
- 所以轉換器的重點是**確認 teach.ini 已經有 `[Teach INI] Update2`**。有的話 tech.dat 不會再被讀到。
- 沒有的話：
  - 來源是 **V912**：要先把兩個 backlash 欄位從結尾搬回 byte 724，再交給移植樹讀。否則第 161 欄之後全部錯位 8 bytes。
  - 來源是 **V899**：直接讀即可，因為新版只是在結尾多了 SortArm 欄位。
- ⚠ 大小相同（V912 與移植樹都是 3,872）**不代表**版面相同，不能用檔案大小判斷。

## 2. 同一個鍵、預設值不同（缺鍵時行為會改變）

工具一共標出 4 個（V912 對移植樹）和 3 個（V899 對移植樹）預設值衝突。回原始碼核對後，真的只有下面兩個。
其餘幾個是同一個值的不同寫法：
- `iLOTSTATUS_W`＝1（V912 `Automation/SCK_ART.cpp:44`）；
- `eSafePLCIOType_Uninstall`＝0（`cmydef.h:5660`）；
- MachineLife 的時間字串兩種寫法，結果相同。

| 檔案 | 區段／鍵 | 舊版預設 | 移植樹預設 | 影響 | 建議 |
|---|---|---|---|---|---|
| 配方 `Tester.Data` | `[AutoRetest] iTesterType` | V899／V912：**0**（Flex） | **1**（93K） | Steven W1 的刻意偏離（`docs/TESTERCOMM_PORT_LEDGER.md:330-338`）。配方沒有這個鍵時，移植樹開機或換配方會把 `iTesterType=1` 寫回 Tester.Data | 舊配方沒有這個鍵時，要保留舊行為就寫 0；要照偏離就不動。**由 Steven／St02 定** |
| `system\Gerneral.ini` | `[OutSortArm] USE_OUT_SORT_X_PITCH_MIN` | V899：**1500** | **1333**（V912 也是 1333） | 只影響從 V899 升級、而且 ini 沒有這個鍵的機台：實際值會從 1500 變成 1333 | 從 V899 升級時，缺這個鍵就寫入 1500 |

另外，有一批鍵在舊版有**兩處**讀取、各自用不同的預設值，移植樹只翻了其中一處。工具把這類列成「default sets differ (subset)」。
例子：HandlerCondition.Data `[Configuration] Tray Mapping Start Delay`、`Exposure Time Out`、`Get Result Time Out`。
- V912 的 `cTrayMapping.cpp:560-562` 讀取時用 5900／1500／10000；
- V912 的 `uLotInfo.cpp:3264-3266` 用 100；
- 移植樹只有前者（`forms/fTrayMapping.cpp:990-992`）。

這類只有在檔案缺鍵、而且走到舊版另一條路徑時才有差別。明細在 DETAIL 檔各檔案的「Default sets differ only by an extra registration」小節。

## 3. 舊版讀檔時順手做的遷移

| 檔案 | 舊版做什麼 | 出處 | 移植樹 | 轉換器 |
|---|---|---|---|---|
| 配方 `HandlerCondition.Data` | `[Configuration] Test Mode`（文字）是空的時候，改讀舊的 `Handling Mode`（整數 0～10）換算成文字 | V912 `cSetUp.cpp:2207-2226`（「向舊版的相容，不要動!!」） | 有同一段（`cSetUp.cpp:437`） | 不用處理 |
| 配方 `Contact.Data` | `CosFunction.bFixNameOfForcePerPinG` 時，沒有 `[Torque Control] Force Per Pin N` 就把舊的 `Force Per Pin` 值複製過去 | V912 `cContact.cpp:563-571` | 有（`forms/fContact.cpp:1697-1703`、`FileRW/DeviceForm_File.gen.inc:1805-1811`） | 不用處理；據 `docs/LEAVE_TASK_PLAN.md:152`，63 份配方裡 0 份已改名 |
| `system\teach.ini` | Shuttle 教點的區段名：正確拼法 `[MInShuttleN]` 與舊拼法 `[MInShutteN]`。開機時 `DecideShuttleTeachSection` 判斷：正確那一節的檢查鍵非 0 就讀它，否則讀舊的；判斷結果寫進 `[Teach INI] ShtSecRead1/2`；**存檔兩節都寫**（鏡像） | V912 `uteach.cpp:88-108`、`:126`／`:144`（RogerYang 20260902） | **規則不同**：照 golden 906，先讀馬達別名那一節（移植樹的別名是舊拼法），沒有才讀另一節；不寫 `ShtSecRead`、不鏡像（`forms/fTeachPara.cpp:96-125`） | V912 存檔時兩節都會寫，所以一般兩節值相同，讀哪一節都一樣。**兩節值不同時**（有人手改過）兩版會讀到不同的教點。建議轉換器發現兩節不同時，照 V912 的判斷把結果寫回兩節 |
| `system\tech.dat` → `teach.ini` | teach.ini 缺 `[Teach INI] Update2` 時讀 tech.dat 遷移 | V912 `uteach.cpp:4811`／`:4849` | 有（`forms/fTeachPara.cpp:407`／`:453`） | 見 §1 的 tech.dat |
| `IniData\DefineTemp\Temperature_ATC_Cold.Data`（ATC 機台常溫用的溫度校正表；20261002 Ifor01 補） | V910 起（V912 量產）ATC 常溫——`eNewATCSystem`＋`bATCActiveCooling`＋`CosFunction.bATCUseTempAdjustment`，而且溫度模式不是 Hot／AmbientHot——讀寫這個新檔；**第一次找不到時從 `Temperature.Data`（非 ATC 的基準表）複製**。V899 與 golden 906 只有 `Temperature_ATC.Data` 一份，冷熱共用 | V912 `uTemp_Set.cpp:3001-3020`（讀）／`:4463-4482`（寫）；golden 906 `uTemp_Set.cpp:2976`／`:4428`；V899 `_ATC_Cold` 0 筆 | **兩份翻譯不一致**：網頁溫度頁（C 路，`FileRW/Temperature.gen.inc:4036-4040`／`:5531-5535`）照 912 分冷熱；開機、換配方、SetTemp（`uTemp_Set.cpp:3179`／`:4743`）照 906 只用一份。整棵樹要不要統一照 912，等 Jimmy 定（FROM_IFOR §3 1002 08:4x；§9 第 5 題） | **來源是 V899 的 ATC 機台**：`Temperature_ATC_Cold.Data` 不存在時，把 `Temperature_ATC.Data` 複製過去（已存在就不動），冷測的補償才會跟升級前一樣；不轉的話新版會從 `Temperature.Data` 複製，冷測補償換成基準表，而且不會提示。來源是 V912：不用處理 |

## 4. 鍵層級的差異（不用轉換，只要知道）

工具以靜態方式抓出每一棵樹裡所有 INI 讀寫（V912 9,702 處、V899 9,098 處、移植樹 11,585 處），
逐檔比對（區段, 鍵）組合。各檔數字見 DETAIL 檔的兩張摘要表。V899 → V912 的變化**只有新增，沒有刪除**
（工具量的字面鍵：HandlerCondition +15、Tester +10、Tray +9（`[AMR]`）、ArmCondition +5、Contact +1、config.ini +54、
Gerneral.ini +9；每個檔都是 −0）。

### 4.1 舊版有、移植樹沒有：幾乎都是沒翻的模組

| 來源模組（V912） | 讀寫的檔案 | 鍵數 |
|---|---|---:|
| `fAOI.cpp`（AOI Top&Bottom／Function Setting；移植樹 `tools/editlist/AOISetup.py:41-54`、`:157-181` 記錄刻意沒翻，`[AOI]` 的 top delay／timeout 鍵名移植樹另改過，裁決 S152） | 配方 `AOI.Data` | 53 |
| `ASE_K Socket/aseTest.cpp` | 配方 HandlerCondition.Data（12）、Gerneral.ini（3） | 15 |
| `AutoAlignment/*` | 配方 HandlerCondition.Data（11）、Gerneral.ini（1） | 12 |
| `MR/*`（RFID） | 配方 HandlerCondition.Data | 10 |
| `cTrayMapping.cpp`（Laser 掃描 AOI 連線） | 配方 HandlerCondition.Data（6）、Gerneral.ini（4） | 10 |
| `Mes/fVATMesFileSys.cpp`（`[Server]*`） | `config\config.ini` | 12 |
| `uLotInfo.cpp`（`[FTPAutomation]*`、`[ARTReset]` 等） | `config\config.ini` | 11 |
| `CCLink/*` | `config\config.ini`（4）、配方 CCLink.Data（2） | 6 |
| `ProductionInfo/*` | `PI_Setting.ini`（13）、`ESDconfig.ini`（4）、`config_Standard.ini`（4） | 21 |
| `uLotInfo.cpp`（每批的 Contact 記錄） | `system\Contact.ini` | 24 |
| `cCounterSel.cpp` 等（視窗位置） | `config\FormPos.def` | 24 |
| `cSpeed.cpp` | `TrayStepSpeed.ini` | 10 |
| `Password.cpp`／`HS_Function.cpp`／`note.cpp` 等 | `EvenLogLevel.ini`、`LogFileName_Record.ini`、`SpecialErrNote.ini`… | 十來個 |
| `cShowBinSelect.cpp` | 配方 Binasgn 系列 `[Category%d] 3722 BinTraySetName／BinTraySetting` | 2 |
| `Automation/SCK_ART.cpp:243` | 配方 Tester.Data `[AutoRetest] iLotICQty` | 1 |

轉換器：**原封保留**。移植樹不讀這些鍵；INI 寫入只改它認得的鍵，不會刪掉其他鍵。

⚠ 工具的「只有舊版有」**不全是**真的少。下列幾項其實是移植樹也讀寫，只是路徑變數沒解出來，被歸到「?」，比對時看起來像「只有舊版有」：
- `AGV.ini`（26 鍵）：移植樹 `FileRW/TestIF_File_AGV.gen.inc` 讀寫同一批 E84 逾時與 Auto Tray Count 鍵。
- `config.ini` 的 `[Function] bAutoSwitchToOperatorMode`：移植樹 `FileRW/Main_A01AutoLogout.cpp:253`。
- Contact Force 的 `LoadRate`／`ContactOffset` 等：V912 `ContactForce.cpp:1064-1157` 與移植樹 `ContactForceLoad.cpp:33-37` 都有，區段名都是執行期組的，兩邊寫法不同。

### 4.2 移植樹新加的鍵（第一次執行時自己補，不用轉）

| 檔案 | 鍵 | 出處 |
|---|---|---|
| `system\Gerneral.ini` | `[TempCtrl] HeaterInsMode`、`HeaterInsIndexOpt`、`HeaterInsOtherOpt`、`HeaterInsAddr_<通道>`（Q34 方案 D）；`HeaterInsOpt_<通道>` 改寫實際廠牌而不是 -9999，開頁不再補寫缺鍵 | `FileRW/HSys_Heater.h:135-160`、`FileRW/HSys.cpp:57-69`、`:960-971` |
| 配方 `Tester.Data` | `[RS-232C]` 從 `D:\RS232Standard\System\Setup.ini` 帶入一次，並留下標記 `W906_SeededFromSetupIni=1`；RS-232 的新選項值**附加在舊索引值之後**，舊值意義不變（超出舊範圍的回到 8 bits／2 stop／None），不合法的組合存檔時拒絕 | `docs/TESTERCOMM_PORT_LEDGER.md:306`、`:316`、`:361`；`TesterComm/Rs232SetupCodes.h:1-12` |
| `config\config.ini` | `[FTPUpLoad] dN10_3_1_SpecifiedTime` | `FileRW/IniConfig.gen.inc:6365` |
| `D:\GPIB9045\system\general.ini`、`D:\RS232Standard\System\Setup.ini` | 原本是另外兩支程式（GPIB9045、RS232Standard）自己的設定檔，現在由移植樹 `TesterComm` 在程序內直接讀寫。格式沿用原程式，不需要轉換。工具把它們跟機台的 Gerneral.ini 分開列 | `TesterComm/Gpib/GpibBridge.h:230`、`TesterComm/Rs232/Rs232Log.cpp` |

## 5. 配方資料夾的 `.MD5` 檔（轉換後要重算）

- 檔名是所有 `*.Data` 內容的 MD5，內容是 `*.Data` 的路徑清單：V912 `cpublic.cpp:808`（`CompareMD5ByFolder`）／`:846`（`SetMD5ByFolder`），移植樹 `cpublic.cpp:955`／`:993`。
- 轉換器改過任何 `*.Data` 之後，都要對那個資料夾重算一次（照 `SetMD5ByFolder`）。不重算的話，用到 `CompareMD5ByFolder` 的流程會判定配方被改過。V912 的呼叫端是 `uLotInfo.cpp:4645`。
- 移植樹的網頁寫入**刻意不更新** MD5，跟 golden `TfContact::SaveSetupFile` 一致（`WebBridgeRecipeDoc.h:191`）。

## 6. 真實配方資料的觀察（188 個配方資料夾）

資料：`D:\HT9045\IniData\Data`（13）、`Data_20260622`（92）、`Data_20250225`（36）、`Data_20240802`（13）、`Data_20140617`（34）。
用 Win32 的讀法：不分大小寫、重複的區段以第一個為準、鍵和值去頭尾空白。

- **每一棵樹都不認得的舊鍵很多，而且幾乎每份配方都有。** 原因是配方一代代用「另存新檔」複製下來，INI 寫入不會刪掉不認得的鍵，所以舊鍵會一直留著。
  - 例：Tester.Data 531 個（`[Alarm] Ignored IC` 在 187 份全都有）；HandlerCondition.Data 896 個；Temperature.Data 213 個；ArmCondition.Data 21 個。
  - ArmCondition.Data 的例子是 `[Input Arm] Auto Y Pitch Accel`，有 168 份；已確認 V912 原始碼裡完全沒有 `Auto Y Pitch`、`Bottom Blower`、`Rotate Wait Time`、`After Offset`、`Ignored IC` 這些字。
  - 轉換器**保留**即可，無害。
- **移植樹會讀、但很多配方沒有的鍵**：例如 ArmCondition.Data `[Index Arm] Y Accel`，153 份沒有。
  - 移植樹第一次載入時會用預設值，`CheckAndRead*` 系列還會**把預設值寫回配方檔**。
  - 不希望第一次開機就改到配方的話，轉換器可以先把這些鍵補上預設值。清單在 DETAIL 檔「Real recipe folders vs the port」各檔的第三行。
- **只有舊版認得、移植樹不認得的資料鍵**：例如 AOI.Data 的 242 個 Function Setting 鍵、HandlerCondition.Data 的 61 個 `[ATC]` 鍵（只有 1 份配方有）。這些對應 §4.1 沒翻的模組。

## 7. INI 位元組格式的已知殘差（不影響讀，影響寫回去的長相）

- `TMemIniFile` 解構時**一律寫回檔案**，Borland 只在有改動時才寫：`docs/KNOWLEDGE.md` 第 4 點，未結。
- Win32 每個區段 32 KB 的上限沒有重現：`vclcompat/IniFiles.h:30-35`。
- 日期時間固定寫成 `yyyy/mm/dd hh:nn:ss`，讀的時候只看數字；BCB6 是跟著電腦的地區格式：`vclcompat/TDateTime.cpp:262-287`。

## 8. 給 St02 的轉換器清單（照順序）

1. **二進位**
   - `machinerecord.dat`：來源是 V899 就重排（§1）。
   - `tech.dat`：teach.ini 沒有 `[Teach INI] Update2`、來源又是 V912 時，把 backlash 兩欄搬回 byte 724（§1）。
   - `lastdata.dat`：不用轉。
2. **預設值會變的兩個鍵**（§2）
   - `Tester.Data [AutoRetest] iTesterType`：缺鍵時寫 0 或不動，等 Steven 決定。
   - V899 機台的 `Gerneral.ini [OutSortArm] USE_OUT_SORT_X_PITCH_MIN`：缺鍵時寫 1500。
3. **teach.ini**：`[MInShuttleN]` 與 `[MInShutteN]` 兩節值不同時，照 V912 的判斷統一（§3）。
4. **ATC 常溫校正表**（§3，20261002 補）：來源是 V899 的 ATC 機台，`DefineTemp\Temperature_ATC_Cold.Data` 不存在時從 `Temperature_ATC.Data` 複製。
5. **（選擇性）** 補上移植樹會讀但配方缺的鍵（§6），避免第一次開機寫配方。
6. **每個改過的配方資料夾重算 `.MD5`**（§5）。
7. 其餘鍵（§4.1、§6 的舊鍵）**一律保留不動**。

## 9. 待確認

| # | 題目 | 給誰 |
|---|---|---|
| 1 | tech.dat 要不要寫（NIGHT_REPORT §0 第 6 件）；轉換器要不要產生 906 版面的 tech.dat | Jimmy |
| 2 | 舊配方缺 `iTesterType` 時寫 0（保持舊行為）還是照偏離寫 1 | Steven／St02 |
| 3 | teach.ini 兩節不一致時以誰為準（建議照 V912） | St02 |
| 4 | 轉換器是獨立工具，還是移植樹開機時自動轉（TO_IFOR I-02 說清單出來再定） | Jimmy／Steven |
| 5 | 移植樹的 ATC 常溫校正表要不要整棵照 912 分冷熱（FROM_IFOR §3 1002 08:4x，Ifor 建議照 912）；定了之後 §3 那一列的「移植樹」欄跟著改 | Jimmy |

## 10. 怎麼重跑

```
python tools/g031_format_diff.py --md docs/G031_FORMAT_DIFF_DETAIL.md [--json out.json]
```

預設會比 V912 與 V899 兩棵量產樹（`D:\HT9045` 底下有的才比）、掃 `D:\HT9045\IniData\Data*` 全部配方資料夾，並跑二進位版面比對。這台大約 70 秒。
參數：
- `--old 名稱=路徑`：換一棵舊樹，可以重複；
- `--recipes 資料夾`：換配方來源，可以重複；
- `--no-binary`：不跑二進位版面。

全程唯讀，只寫你指定的 `--md`／`--json`。

**準確度**：
- 歸不到檔案的讀寫：V912 1.7%、V899 1.4%、移植樹 1.6%，都列在 DETAIL 檔「?」那一節，附位置，不猜。
- 同一個鍵在不同客戶或功能條件下各註冊一次時，會列出每一個預設值。
- 執行期組出來、轉不成樣式的鍵名，在資料比對時一律當成「認得」，所以「不認得」只會少算、不會多算。
