# HT9045 BCB6 的三套讀寫檔機制 —— 實作層對照

> 量測日 20260923。BCB 對照樹 `HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（Big5），
> 移植樹 `HT9011UC_Cpp_V3.33.906.0`（UTF-8）。行號屬於這兩棵樹；各人手上的 BCB
> 版本略有不同，行號會漂，函式名與行為不會。
>
> 這份是給 JSON 橋接層（`ht9045-json-bridge`）用的：每套機制各自的**值格式、預設值
> 行為、副作用、移植樹現況**，決定 `FieldDesc`／`Binding` 要怎麼填、`Persist()` 要走哪條。

---

## 總表

| # | 機制 | 讀 | 寫 | 檔案格式 | 用在哪 | 移植樹 |
|---|---|---|---|---|---|---|
| A | **ini 文字**（`TIniFile`） | `ReadIniData` ×4 多載 | `WriteIniData` ×6 多載 | `[區段]`／`鍵=值` | 大多數 `.Data`、`config.ini`、`system\*.ini` | **已翻譯**（`common.cpp:785-1178`） |
| A' | ini 文字（記憶體整檔） | `ReadIniDataMem` ×4 | — | 同上 | RogerYang 20260214 為 >32 KB 區段加的 | 已翻譯（`common.cpp:852-897`） |
| B | **`HTEditList`**（`TMemIniFile`＋控制項綁定） | `ReadEditTextFromFile` | `SaveEditTextToFile` | 同 A，但鍵值由控制項決定 | `UdUld.Data`、`Contact.Data`、`Tray.Data`、`HandlerCondition.Data`、`config.ini`、`LastSet.ini`、`AOI.Data` | **已翻譯**（`Public/HTEditList.cpp:594` / `:1024`，本體 429 / 306 行） |
| C | **二進位 blob**（Win32 `ReadFile`／`WriteFile` 整塊） | `ReadData`／`ReadLastDataFile`／`GetLevelSet` | `WriteData`／`WriteLastDataFile`／`SetLevelSet` | `sizeof(struct)` 位元組原樣 | `lastdata.dat`（`LAST_GENERAL_SET`）、`levelset.dat`（`LAST_LEVEL_SET`）、`login.dat`、`machinerecoder.dat` | 已翻譯（`cprod.cpp:1691` / `:2001`；`cSecurity.cpp:271/533` 有呼叫點） |

全樹用量（V912，剝註解後）：`ReadIniData` 3,704 次、`WriteIniData` 3,609 次、
`ReadEditTextFromFile` 15 次、`SaveEditTextToFile` 16 次、`InitialDataToEdit` 18 次。

---

## A. ini 文字：`ReadIniData` / `WriteIniData`（`common.cpp`）

### A.1 快取：一次只開一個檔

```cpp
// common.cpp:323
bool OpenIniFile(AnsiString FileName)
{
    if(FileName=="") return false;
    if(INIFile==NULL || INIFile->FileName!=FileName)
    {
        CloseIniFile();                 // :336  UpdateFile() + delete
        INIFile=new TIniFile(FileName);
    }
    return true;
}
```

- 全域單一 `INIFile`。**換檔就 flush 上一個**。同一個檔連續讀寫不重開，這是 Steven 20141120 加的速度優化。
- ⇒ 橋接層 `Persist()` 若要「dryRun 先算、再一次寫入」，不能依賴這個快取，要自己收集後再逐鍵 `WriteIniData`。
- `TIniFile` 走 Win32 `GetPrivateProfileString`，**單一區段 32 KB 上限**。`ReadIniDataMem`／`OpenIniFileMem`（`:345`）改用 `TMemIniFile` 整檔載入避開它；目前只有讀，沒有 `WriteIniDataMem`。

### A.2 讀：四個多載，找不到就回預設值，**不寫回**

```cpp
bool       ReadIniData(FileName, Group, Name, bool   bValue);   // :510  INIFile->ReadBool
double     ReadIniData(FileName, Group, Name, double Value);    // :524  INIFile->ReadFloat
int        ReadIniData(FileName, Group, Name, int    Value);    // :538  INIFile->ReadInteger
AnsiString ReadIniData(FileName, Group, Name, AnsiString Value);// :552  INIFile->ReadString
```

- 開檔失敗 → `RecordProcess("Read NULL INI on [%s] %s")` 然後**回傳預設值**。呼叫端分不出「檔沒開」和「鍵不存在」。
- `TDateTime` 那個多載（`:503`）例外：鍵不存在會 `WriteDateTime` 寫回預設值。
- 多載靠**第四個參數的型別**選。`ReadIniData(szDir,"X","Y", 0.1)` 走 double；`0` 走 int；`false` 走 bool；`AnsiString("…")` 走字串。**產生器決定 `FieldDesc.type` 就是看這個參數的字面型別**，不是看被賦值的欄位型別（兩者不一致時 golden 有隱式轉換，要照 golden）。

### A.3 寫：六個多載，帶 Change Log 副作用

```cpp
void WriteIniData(FileName, Group, Name, bool   bValue);   // :622
void WriteIniData(FileName, Group, Name, int    Value);    // :691
void WriteIniData(FileName, Group, Name, double Value);    // :875
void WriteIniData(FileName, Group, Name, unsigned long);   // :974
void WriteIniData(FileName, Group, Name, AnsiString Value);// :1021
void WriteIniData(FileName, Group, Name, TDateTime Value); // :1101
```

每個多載的流程一樣：

1. `OpenIniFile`，失敗 → `RecordProcess("Write NULL INI …")` 返回。
2. **先讀舊值** `ret=INIFile->ReadXxx(Group,Name,Value)`。
3. 舊值≠新值且 `InitialOK==true` → 組 Change Log：`"%s_%s change Value"`（檔名含 `Offset` 時是 `"… Offset change Value"`）＋ `"%d==>%d"`，呼叫 `RecordChangeLogProcess`；`HandlerCondition.Data [Configuration]`／`Tester.Data [Mode]／[GP-IB]` 幾個鍵有客製的文字（`:649-760`，拿 radio group 的 caption 當文字）。
4. `INIFile->WriteXxx`，包在 `try/catch(...)`。
5. `CosFunction.bUseChangeLogByLot && bSysLotStart && bHasChange` → `FormHS->RecordChangeLogByLot`。

**值格式**（寫進檔的字串）：

| 多載 | 格式 | 備註 |
|---|---|---|
| bool | `TIniFile::WriteBool` → `1`／`0` | |
| int | 十進位整數 | |
| double | **`%0.4f`**（`:888`，Steven 20150723「double 資料存檔前都補成 4 個 0」） | 讀回用 `ReadFloat`，所以 `26.66` 存成 `26.6600` |
| AnsiString | 原字串 | Change Log 比對時先 `TryStrToFloat` 兩邊，都是數字就用 `atof` 比（`:1053-1058`），避免 `"1.0"` vs `"1"` 被記成變更 |

⚠ 表單自己的 `SaveSetupFile` 常常**先格式化再丟進 AnsiString 多載**（`cHotPlate.cpp:599` `FormatFloat("0.000", …)`），所以同一個 double 欄位在不同表單可能是 3 位或 4 位小數。**產生器要記錄 golden 實際用的格式字串**，不能一律 `%0.4f`；G1 gate「位元組 diff 只有那一行」就是抓這個。

### A.4 對橋接層的意義

- `FieldDesc` 四元組 `[欄位, 區段, 鍵, 預設值]` 直接從 `Struct.field = ReadIniData(path, "區段", "鍵", 預設值)` 抽；字串欄位另認 `strncpy(Struct.field, asString.c_str(), …)`（先 `ReadIniData` 到暫存再 `strncpy`，如 `cHotPlate.cpp:162-163`）。
- 同一結構同一欄位可能在**兩個檔**被讀（`TestIF_File` 同時來自 `Tester.Data` 與 `HandlerCondition.Data [Configuration]`），`FieldDesc` 要帶檔名，不能只帶區段。
- 直寫路徑（沒表單的綁定）沿用 `WriteIniData`，**Change Log 副作用會自動帶到**——這是好事，網頁改值也會進 Change Log。

---

## B. `HTEditList`：控制項驅動的 ini（`Public/HTEditList.cpp`）

### B.1 設計意圖（檔頭註解 `:2-25`，Steven 20170629）

> 為了節省存檔與讀檔時間，每個 INI 檔案帶一個 `HTEditList`。
> `HTEditList->Add(Edit元件, 參數指標, 參數型態, IniGroupName, IniKeyName, DefaultValue, DisableEventOverlap, MinValue, MaxValue)`
> 讀檔後會**同時**把資料轉給元件與參數；客戶限制處理完要再 `InitialDataToEdit()` 把參數指回元件。

也就是：**鍵值表不在程式碼裡，在表單 `FormCreate` 的一串 `Add()` 呼叫裡**，每筆綁一個 VCL 控制項＋一個結構欄位指標。

### B.2 型別（`Public/HTEdit.h`，`TEditContent`）

```
ECText=0 純文字      ECInteger=1      ECDouble=2      ECPosInt=3    ECPosDouble=4
ECNegInt=5           ECNegDouble=6    ECFileName=7    ECPassword=8  ECPercent=9
ECPort=10            ECIPAddr=11      ECBool=12（Steven 20230224 加）
```

### B.3 寫：`SaveEditTextToFile(Path, FileName)`（`:574-1007`，429 行）

- 用 `TMemIniFile`（整檔記憶體），**不是** A 的 `INIFile` 快取。
- 逐一走 `FEditList`，每筆：
  - `IniGroupName`／`IniKeyName` 任一為空 → 警告、`bResult=false`、**跳過這筆**。
  - 以 `"%s_%s"`（區段_鍵）去重，**同一鍵只寫第一個綁到它的控制項**（`:627-635`）。
  - `bReadFromFile==false` 的不寫。
  - `CheckRange` 開著就做範圍檢查：
    - `ECBool` → `Chb->Checked` 必須 0/1。
    - 整數族 → `TEdit` 用 `atof(Text)`；`TComboBox`／`TRadioGroup` 用 `ItemIndex`；比 `MinValue`／`MaxValue`。
    - 浮點族 → `atof(Text)` 比 `dMin`／`dMax`。
  - **單位轉換**（`iTransformType`，Steven 20230905）：`EUuMToMM` 顯示 mm、存檔 ×100（μm 的 0.01）；`EUMSToSec` 顯示秒、存檔 ×1000（ms）。**畫面值 ≠ 檔案值。**
  - 浮點小數位由 `iDecimalPoint` 決定（`:747-760`）：`<=1` 原字串、`1..6` 對應 `%0.1f`…`%0.6f`。
  - 超出範圍 → `bResult=false`，`bAlarmLimitation` 時彈 `MessageBoxA`。**整體回傳 false 但其他筆仍會寫**——不是 all-or-nothing。

### B.4 讀：`ReadEditTextFromFile(Path, FileName)`（`:1009-1321`，306 行）

- 同樣 `TMemIniFile`＋`"%s_%s"` 去重。
- `bReadFromFile==false` → 控制項填 `DefaultValue`（`TEdit` 填 Text、`TComboBox` 填 `atoi(DefaultValue)` 當 ItemIndex）。
- 讀到的值**同時**寫進控制項與 `Add()` 時給的參數指標；之後表單可能再 `SetCustomerLimitationForConfig()` 改參數，再 `InitialDataToEdit()` 推回控制項。

### B.5 對橋接層的意義

- **鍵值表要從 `Add()` 呼叫抽**，不是從 `ReadIniData`。產生器要多認一種來源：`elXxx->Add(控制項, &Struct.field, EC型別, "區段", "鍵", "預設", …, "min", "max")`。`FieldDesc` 順便就有了 min／max 與 `TEditContent` 型別，比 A 更完整。
- **單位轉換與小數位要進 `FieldDesc`**（`iTransformType`、`iDecimalPoint`），否則 JSON 送 mm、檔案期待 μm，G1 就會失敗。
- golden 的寫方向讀的是控制項（`Chb->Checked`／`CEd->Text`／`ItemIndex`），不是參數指標。**使用者 20260923 裁決：新架構沒有控制項**，③' 改從結構欄位寫；`SaveEditTextToFile` 裡跟著控制項走的三件事要搬進 `FieldDesc`／`Clamp*`：
  1. min／max 範圍檢查 → `FieldDesc.min/max`，在 `FromJson(dryRun)` 就拒；
  2. `iTransformType` 單位轉換（畫面 mm→檔案 ×100；畫面 s→檔案 ×1000）→ `FieldDesc.scale`；JSON 送**畫面單位**，寫檔時乘回去；
  3. `iDecimalPoint` 小數位 → `FieldDesc.fmt`。
- 去重規則（同鍵只寫第一個控制項）在結構寫檔下自然消失（一個欄位一個鍵）；但 golden 檔裡若真有「兩個控制項綁同一鍵、只有第一個生效」的情況，產生器要在抽 `Add()` 時偵測並警告，那代表 golden 有一個欄位實際上從未被存過。

---

## C. 二進位 blob：整塊 `sizeof(struct)`

### C.1 原語（`cprod.cpp:1320` / `:1340`）

```cpp
bool WriteData(char *cFName, char *ptr, int size)   // CreateFile(CREATE_ALWAYS) + WriteFile(ptr,size)；失敗 ShowErrorMessage("WAR1682")
bool ReadData (char *cFName, char *ptr, int size)   // CreateFile(OPEN_EXISTING) + ReadFile(ptr,size)；失敗 ShowErrorMessage("WAR1681")
```

沒有表頭、沒有版本號、沒有校驗；**檔案位元組 = 結構在記憶體的位元組**。

### C.2 `LAST_LEVEL_SET` ↔ `system\levelset.dat`（`cSecurity.cpp`）

```cpp
typedef struct { int AccessLevel[256]; } LAST_LEVEL_SET;   // cprod.h:1149-1152，1 個成員

void TfSecurity::GetLevelSet()   // :1474
{
    FileName="d:\\HT9045\\system\\levelset.dat";
    if(!FileExists) WriteData(…, (char*)&LevelSet.AccessLevel[0], sizeof(LevelSet));  // 不存在就用記憶體現值建檔
    ReadData(…);
    for(i=0;i<256;i++)
        // CC_KYEC_LEE：i==35/114/128 強制 2，i==104 強制 3
        // i==163 一律 3（Life Time Edit Permission 只有 Hontech 能改）
        // 其他：CheckRange(x, 0, bSecurityHave5Level ? 4 : 3)
}
void TfSecurity::SetLevelSet()   // :1511
{   WriteData(…, (char*)&LevelSet.AccessLevel[0], sizeof(LevelSet));   }
```

- 檔案 = 256 × 小端 int32 = 1,024 bytes。**路徑寫死** `d:\HT9045\system\`。
- 讀完有**客戶碼鉗制**與範圍鉗制——所以 JSON 拿到的 `LevelSet.AccessLevel[i]` 是鉗制後的值，不是檔案值（跟 A 的 `ReadFile()` 修正規則同一類問題）。
- ✅ **已完成（20260926，commit `8c5ea501`，S64）**：`system.levels.put` 已改呼叫新檔
  `WebLevelSet.cpp` 的 `W906_LevelSetPut`，照 golden `TfSecurity::FormClose` 全流程走
  （`Insufficient(29)`→`GetLevelSet`→驗證→套值→三條鉗制→備份→`SaveJamLevel`→`SetLevelSet`
  整塊 1024 bytes→重讀逐位元組比對）；`cSecurity.cpp` 的 SEC-W1（`GetLevelSet` 建檔）／
  SEC-W2（`SetLevelSet` 寫入）兩個 GATE 都已解開。**index → 權限名稱**那層（180 筆名稱）仍待
  確認是否已接（見下一點，該點本身未受本次更正影響）。SECS S125F4（GATE [L1]）交給 Jimmy。
  舊句（20260923 量測當下的狀態）移到 `archive/file-io-mechanisms_superseded.md`。
- **名稱來源就在 golden 裡**（使用者 20260923 指出）：`cSecurity.cpp:28-207` 的 180 筆
  `mySecurityPal.push_back(new TMySecurity("[NN] 名字", Glyph, 群組))`。20260923 驗證：`[00]`～`[179]` 連號、無重複、無缺號；vector 索引＝`[NN]` 標號（0 筆不一致）；**沒有任何一筆被 `if`／`CUSTOMER_CODE` 包住**，所以順序不隨機台變；`:279` `mySecurityPal[i]->SetLevel(LevelSet.AccessLevel[i])` 與 `:443` 反向證明 `i` 就是 `AccessLevel` 索引。移植樹 `cSecurity.cpp:74` 起同樣 180 筆。`AccessLevel[256]` 只用 0～179。
- ✅ **已完成（同上，commit `8c5ea501`）**：三條鉗制（`[87]`／`[129]`／`[86]`）與 `GetLevelSet()`
  的客戶碼、`CheckRange` 都已經在 `W906_LevelSetPut` 裡照 golden 順序做了，不用再搬。舊句移到
  `archive/file-io-mechanisms_superseded.md`。
- **線上縮減**：名字是靜態的，`/schema` 送一次 `names[180]`；`GET` 回 `values[180]`（不送 180～255）；`put` 收稀疏索引物件。名字全部是 ASCII 英文（如 `"[177] [I49] - Offline clean out all ic"`），沒有編碎問題。

### C.3 `LAST_GENERAL_SET` ↔ `system\lastdata.dat`（`cprod.cpp`）

```cpp
bool ReadLastDataFile()                 // :1603（移植樹 :1691）
    ReadTestMode();                     // 先讀 TestMode.Data
    LastSet.bD41TestSocketICCheckSkip=false;
    // 主檔與 backup 都是 0 bytes → 讀 lastdata_backup2.dat
    // 否則 ReadFile(lastdata.dat, &LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET))
    // 之後比對 lastdata_backup.dat …（省略）

bool WriteLastDataFile(bool BackUp2, bool bNotContact)   // :1910（移植樹 :2001）
    LastSet.bBinData32=true;
    if(BackUp2) WriteFile(lastdata_backup2.dat, …, FILE_FLAG_WRITE_THROUGH)
    WriteFile(lastdata.dat, …)          // try/catch，失敗 WAR1682
    WriteFile(lastdata_backup.dat, …)
```

- 387 個成員、139 個陣列（大括號配對量的）。`sizeof` 跟編譯器對齊有關——**移植樹（MinGW/MSVC）與 BCB6 的 `sizeof(LAST_GENERAL_SET)` 必須先量到一致，才能共用同一個 `lastdata.dat`**。這一條不在本 skill 範圍，但橋接 `LastSet` 前要先過。
- 三份檔（主、backup、backup2）＋「主檔與 backup 皆 0 bytes 才讀 backup2」的救援邏輯，橋接層寫方向**只能呼叫 `WriteLastDataFile()`**，不要自己碰檔。
- `FILE_IO_STATUS`（表③）已註：`LastSet` 僅可做 C++ projection，Python 不可臆測 binary layout。

### C.4 對橋接層的意義

- `FieldDesc` 沒有區段／鍵可填，改由**解析結構宣告**產生，`offsetof` 由 C++ 出。
- 寫方向不逐欄寫檔，而是 `FromJson` 改記憶體 → 呼叫 golden 的 `SetLevelSet()`／`WriteLastDataFile()` 整塊落地 → `Reload`。
- 二進位沒有「只改一行」可比，G1 改成：**檔案位元組 diff 只在該欄位的 `offsetof..+sizeof` 範圍內**。

---

## D. 自由函式與總指揮（不屬於任何表單）

| 函式 | 位置（V912） | 機制 | 檔 | 備註 |
|---|---|---|---|---|
| `ReadTestMode()` | `cprod.cpp:3464` | A | `TestMode.Data` | `bLastSetInSetUpFile==false` 時**不讀檔**，改從 `LastSet.bUseTestSocket` 複製；`bProgramStartOnLine` 開機時會**寫回** OnLine/Real |
| `SaveTestMode()` | `cprod.cpp:3314` | A | `TestMode.Data` | `InitialOK==false` 或 `bLastSetInSetUpFile==false` 直接 return |
| `TFTestIF::ReadTestIFFile()` | `cTesterIF.cpp:563`（移植樹 `forms/fTesterIF.cpp:807`） | A | `Tester.Data` | 是 `TFTestIF` 的讀檔器（名字不叫 `ReadFile`，表⑧ 因此把它列成 n/a）；`iTestType` 越界會**當場寫回** 1。⛔ 20260926：移植樹那份在 `#if 0 // GATE (F-5)`（`forms/fTesterIF.cpp:800-1223`）；**真正在跑的是 C 路 `FileRW/TestIF_File_TesterIF.cpp` 從 golden 轉出的那份**，開機／換配方 `tools/wb_serve.cpp:3231`（`porting-gaps.md` 十四結案） |
| `SaveAllFile(SaveFileNm)` | `csystem.cpp:23542` | — | — | **寫檔總指揮**：對 12 個表單依序 `DoIniDataToForm()` → `SaveSetupFile()`；`fOffSet` 兩個 offset 各存一次；`fYieldMonitoring` 只 Save 不 ToForm |
| 讀檔總指揮 | `main.cpp:9321-9350` | — | — | 順序有依賴：`fTrayForm` → `fSetup`（Contact 要用 TestIF）→ `fContact` → `FTestIF->ReadTestIFFile` → `fYieldMonitoring` → `fLaserSensor` → **`fSetup` 再讀一次** → `fTemp_Set->ReadTempFile`（得在 fSetup 後）→ `fHotPlate`（得在 fSetup 後）→ `fLd_ULd` → `fSpeed`（得在 fTrayForm 後）→ `fTrayAssignment`（得在 fSpeed 後）→ `fQAMode`（必須在 Bin 之前） |

⚠ 讀檔順序不是可交換的。橋接層的 `Reload(binding)` 若只重讀單一表單，要確認它的前置（例如 `fHotPlate` 依賴 `TestIF_File.iTestMode`）已經是新值；保守做法是 `Reload` 走總指揮同一段順序。

---

## F. Offset 族：物件 setter ＋ 變數區段 ＋ 第四種格式（使用者 20260923 點名）

`DoArmOffsetConvert()` 的來源 `InArmOffSet_File[i]`／`OutArmOffSet_File[i]`／`SortArmOffSet_File[i]`／`Offset_File` 自己也是讀寫檔的一員，但寫法跟 A 不同，產生器第一版整個漏掉（`TfOffSet` 被量成 2 讀／2 寫，實際 `cOffSet.cpp` 有 **135 讀／97 寫**）。

### F.1 資料型別

| 型別 | 宣告 | 內容 |
|---|---|---|
| `class ARM_OFFSET` | `cprod.h:251` 起，86 行 | 11 個資料成員（`bOneByOne`、`dArmX/Y`、`dArmVariable`、`dXPitch[4]`、`dPickUp`、`dPlaceUp`、`dArmVariableY/2/3/4`）＋ 12 個 `Set*`／對應 `Get*`＋ `ARM_SINGLE_PARAM *SingleOffSet`（逐吸嘴 `dPosOffSetX/Y[j][k]`、`dPickUpOffSet`、`dPlaceOffSet`）＋ 3 個 `TStringList*`（SECS/GEM 用） |
| `RUN_OFFSET` | `cprod.h:275` `extern RUN_OFFSET Offset_File;` | 15 個成員（`iTrayArmX[]`、`iIndexArmPickUp/Place/Contact[2]`、`iSH*[2]`、`iPreciserOpen/Close`、`dTrayZseparate[]`…） |
| 實例 | `InArmOffSet_File[InOfsTotal]`、`OutArmOffSet_File[OutOfsTotal]`、`SortArmOffSet_File[SortOfsTotal]`（各有不帶 `_File` 的執行中版本） | 指標陣列，建構子配置 |

### F.2 讀（`TfOffSet::ReadFile()`，`cOffSet.cpp:1998`）

```cpp
szDir = (bSaveOffsetByMachine && bA57_3SaveOffsetByMachine) ? sSaveByMachine : GetOffsetPath();
szDir += (Tri_Temp_Machine==1) ? Tri_Position_Offset() : "\\Position Offset.Data";
for(i<InOfsTotal)
    InArmOffSet_File[i]->SetOneByOne (ReadIniData(szDir, CapStrInput[i], "One By One", 0));
    InArmOffSet_File[i]->SetVariableY(ReadIniData(szDir, CapStrInput[i], "VariableY",  0.0));
    ...                                              ^^^^^^^^^^^^^^ 區段是變數
```

- **區段是 `CapStrInput[i]`**（`cOffSet.cpp` 定義：`"Loader"`, `"Hot Plate1"`, `"Hot Plate2"`, `"Input Shuttle1"`, `"Input Shuttle2"`, `"Auto Clean"`, `"OCR"`, `"Input Rotate"`, `"Loader Row B"`, …），Out／Sort 各有自己的一組。
- **目標是 setter**：`Obj[i]->SetXxx(ReadIniData(...))`，全樹 38 個這種寫法全在這裡。
- 路徑三選一：`GetOffsetPath()`（`:1430`）＝ `E45_AllSetupFileUseOneFile` 或 `CC_ASE_CL` → `DefaultPath\DefineOffset`；`E59GroupOffsetFile` → 依配方名 `]` 前的群組；否則 `OffsetPath\<配方名>`。`A57_3` 開時改 `sSaveByMachine`。
- 三溫機（`Tri_Temp_Machine==1`）檔名由 `Tri_Position_Offset()` 決定（`Position Offset Hot.Data` 等）。
- 另有 `ReadInvisibleFile(szDir2)`（`:1859`，266 處 `Invisible`）讀隱藏偏移。

### F.3 寫（`TfOffSet::SaveFile(iSelPartData, SpecialMode, bReset)` → `SaveSetupFile(...)`，`:1406`／`:1470`）

- **參數化**：一次只存一個部位 `iSelPartData`（`OfsLoader`…`OfsAuto1-6`／`OfsFix1-6`），`SpecialMode` 有 `iSaveStander`／`iSaveSpecial` 兩種，`SaveAllFile()` 各叫一次。
- **鉗制在寫檔器裡**：`InputLimit.iOffsetZHigh/Low`（Unloader 部位用 `iOffsetUnloaderZHigh/Low`），超限設 `bOverLimitation`／`bUnderLimitation`。這是 `Clamp*` 的來源之一。
- `ASE_KaohSiung` 另存 `JOBFILE`。
- 其他寫檔器：`TZteach::SaveSetupFile`（`AutoTeach/InOutArmZteach.cpp:4674`，11 寫，自帶一份 `CapStr[OfsTotal]`，**跟 `CapStrInput` 不同一份**，只有 7 個名字）、`TfArmOffsetData::SaveOffSet`（`ArmOffsetData.cpp:148`，13 寫）。

### F.4 第四種格式：Tab 分隔文字（`TStringList`）

```cpp
bool TfOffSet::ReadHotTempShiftOffsetData()    // :4047
    Filename="D:\\HT9045\\IniData\\DefineOffset\\HotTempShiftOffsetData.txt";   // 寫死
    memoPtr->LoadFromFile(Filename); 去尾空行;
    PasteStringGridAsTabFormat(sg_Offset_HotTempShiftOffset, memoPtr);           // 直接貼進 TStringGrid
void TfOffSet::WriteHotTempShiftOffsetData()   // :4006  反向
```

不是 ini、不是 `HTEditList`、不是 blob：**整檔是 Tab 分隔的格子**，讀進 `TStringGrid`。`vclcompat/StringGrid.h` 有 shim，但橋接層對它的 `FieldDesc` 要用「二維表」形狀，不是欄位表。`Invisible`／`HotTempShift` 共 138＋266 處引用，不小。

### F.5 移植樹現況

**三個檔都不存在**：`cOffSet.cpp`、`AutoTeach/InOutArmZteach.cpp`、`ArmOffsetData.cpp`。`class ARM_OFFSET` 在移植樹 `cprod.h` 有沒有宣告要另查（`grep -n "class ARM_OFFSET" cprod.h` 沒命中）。⇒ `DoArmOffsetConvert()` 在移植樹即使被叫到，來源也是空的。**Offset 族是目前唯一整族未翻譯的讀寫檔單位**，JerryYang 翻譯排程要加上它。

### F.6 對橋接層的意義

- `FieldDesc` 不能從 `<CapStrInput[i]>` 這種表達式直接產生；產生器要**展開陣列**：讀到 `CapStrInput[InOfsTotal]={...}` 的定義後把 `i` 展開成 N 筆，每筆區段一個常數。`TZteach` 那份 `CapStr[]` 只有 7 個、`CapStrInput` 更多，**兩份要分開展開**，不能混。
- setter 目標 `Obj.SetXxx` 要對回資料成員（`SetVariableY`→`dArmVariableY`），`FieldDesc.offset` 才算得出來；對照表手寫一次（12 筆）。
- 這一族有**四個持久化路徑決策**（by machine／one file／group／per recipe）＋三溫檔名切換，`Binding.persist` 要是**函式**不是常數，跟 golden `GetOffsetPath()` 同邏輯。
- `HotTempShiftOffsetData.txt` 另開一種 `Persist` 種類 `tabgrid`。

---

## G. `IniConfig` ↔ `config.ini` 族：`ReadLastSetIni()`／`SaveLastSetIni()`（使用者 20260923 點名）

**名字誤導**：這一對的主體是 `IniConfig`（全樹最大的設定結構）↔ `config.ini`，`LastSet` 只沾到 1 個欄位。畫面是 `Config.Configuration.html`（`TfConfiguration`，`cConfiguration.cpp` 7,939 行）。

### G.1 `ReadLastSetIni()`（`cprod.cpp:2977-3090`）

```
sPath = AuthPath + "config.ini"
fConfiguration->ChangeCBListProperty()          // 先改權限才改顯示
CustomerFunctionSelect()                        // 客戶功能選擇
ReadLastDataFile()                              // ← 二進位 LastSet 在這裡讀
IniConfig.sMachineType / sGPIBMachineID / RMSTesterID = CheckAndReadIniDataGeneral(...)   // Gerneral.ini，讀不到就寫回預設
HotPlate.Data [Hotplate Form] Using Flag 的 E43 修正（讀→改→寫回）
16 × ProcessLastSetIni_*(bReadFile)             // 見 G.3
SetCustomerLimitationForConfig()
USE_SOCKET_SENSOR==999 的舊版相容（寫回 Gerneral.ini）
氣缸沒裝就關 F14；HANA ART 初始化
```

### G.2 `SaveLastSetIni()`（`cprod.cpp:3092-3148`）

```
WriteIniDataGeneral("Version", …)  ×4          // Gerneral.ini
16 × ProcessLastSetIni_*(bWriteFile)
cbLastSet->SaveEditTextToFile(AuthPath, "LastSet.ini")          // HTEditList，91 筆：IniConfig×90、LastSet×1
elConfig ->SaveEditTextToFile(AuthPath, "config.ini")           // HTEditList，1,581 筆：IniConfig×1,578、TrayForm×3
elConfig_byRecipe->SaveEditTextToFile(szDir, configByRecipe.ini) // HTEditList，20 筆
SPIL 的 Run-check 關閉警告（WAR16132）；SendCommand_EventLog(EL_UPDATE_PARAMETER)
```

`elConfig` 的 1,581 筆 `Add()` 分在 `TfConfiguration::InitConfigEdtList_ItemA..P`（`cConfiguration.cpp:374-4594`），區段分佈：`Function`×247、`Index`×210、`Tester`×102、`Tempture`×101、`Handler_OEE`×81、`In/Out Arm`×72、`Tray`×65、`Shuttle`×62…；`cbLastSet` 的 91 筆幾乎全在 `In/Out Arm`×85。

### G.3 第五種呼叫形式：`ReadWriteIni(path, 區段, 鍵, 現值, 預設, bRead[, bCheckRange, max, min])`

```cpp
// common.cpp:1489 bool / :1505 AnsiString / :1520 TDateTime / :1540 int / :1561 double
T tValue=Value;
if(bIsRead) { Value=CheckAndReadIniData(FileName,Group,Name,DefaultValue); tValue = bCheckRange ? CheckRange(Value,Max,Min) : Value; }
else        { WriteIniData(FileName,Group,Name,tValue); }
return tValue;
```

- **同一行程式碼依旗標讀或寫**，所以 `ProcessLastSetIni_Count`（`cprod.cpp:2462`）這種函式被 `bReadFile`／`bWriteFile` 各叫一次。
- 讀的那條走 `CheckAndReadIniData`：**鍵不存在就寫回預設值**（`common.cpp:409`）——讀檔有寫檔副作用，`dryRun` 要知道。
- 鍵常是 `sprintf` 出來的（`"ContactSet%d_%d"`、`"O_14"+str1`），區段是常數但鍵不是；三維陣列 `IniConfig.ContactSet[3][2][16]`、`HeadContactCount*`、`SocketContactSet[4][8]`。
- **產生器第二版不認這個形式**（只認 `ReadIniData`／`WriteIniData`），16 個函式合計被量成 Read 0／Write 13。第三版要把它當「讀＋寫各一」計，並把 `bCheckRange/max/min` 進 `FieldDesc`。

### G.4 對橋接層的意義

- `Config.Configuration.html` 的 `FieldDesc` **從 `elConfig->Add(控制項, &IniConfig.x, EC型別, "區段", "鍵", "預設", …, "min", "max")` 抽**，一筆就有型別／預設／範圍，比 `ReadIniData` 族完整；`ProcessLastSetIni_*` 那些不經控制項的欄位另從 `ReadWriteIni` 抽。
- `SaveLastSetIni()` 在新架構**不逐字翻譯**（它的 `SaveEditTextToFile` 讀控制項）；③' 改成依 `FieldDesc` 寫 `TMemIniFile`，但 G.2 列的**非 HTEditList 副作用**（`WriteIniDataGeneral`、`ProcessLastSetIni_*(bWriteFile)`、`EL_UPDATE_PARAMETER`、SPIL 警告）要保留在 `Persist(iniConfig)` 裡，照順序叫。
- 現有 `GET /api/system/config`（文件鏡像）保留唯讀；新增 `iniConfig` 綁定走 `/api/struct/iniConfig`。
- **使用者 20260923 裁決：走結構，但存檔格式要跟原本一樣。** 「原本」量到的樣子：`D:\HT9045\config\config.ini` 68 區段、1,144 鍵、1,212 行全 CRLF、ASCII、`Key=Value`（等號旁 0 個空白、0 個空行）、bool 寫 `0`/`1`、日期 `1899/12/30` 這種 `yyyy/mm/dd`、陣列鍵直接帶索引 `bAutoSaveLogWeek[3]`。順序＝`elConfig->Add()` 呼叫順序。vclcompat 的 `TMemIniFile`（`IniFiles.h:94`）已保證「保留區段／鍵插入順序、原始位元組、double 以 `%0.4f` 文字」，所以 `Persist(iniConfig)` 只要**照 `Add()` 順序經同一個 shim 寫**，格式就會一致；細則見 `SKILL.md` §五 第 8 條。

## H. `TfDIOFrom` ↔ `iniData\DioCfg\*.ini`（`Config.DIOInterFaceCFG.html`）

- `DIOInterFaceCFG.cpp:71 LoadData()`（讀，11 個 `ReadIniData`）／`:132 DoIniDataToForm()`；寫 11 個 `WriteIniData`。檔案依 `cbDIOType` 動態選 `iniData\DioCfg\<型態>.ini`，另有 `DIO.CFG` 型態順序清單。
- 命名不是 `ReadFile`／`SaveSetupFile`，表⑧ 因此沒列；`FILE_IO_STATUS`（表③）已有它的檔案鏡像 `GET /api/system/dio`，頁面 wire 4 筆綁 `dio`。
- 橋接層：`dioCfg` 綁定，`Binding.persist` 是**函式**（檔名由 `cbDIOType` 決定），同 Offset 族的做法。

## I. 留痕（event log）：`RecordProcess` 族 → `MyDBIProcess`（使用者 20260923 點名）

不是設定檔，但 HTML 的每個操作都要落到它，所以列在這裡。全部在 `cMyDB.cpp`（V912 行號）。

| 入口 | 位置 | 呼叫數 | 做什麼 |
|---|---|---|---|
| `RecordProcess(S, S2)` | `:1564` | 1,121 | 去重後 `MyDBIProcess("Process", S, S2)` |
| `NewRecordProcess(AlarmCode, S, Debug)` | `:1545` | 342 | 去重後 `MyDBIProcessNew("Process", AlarmCode, S, Debug)`；`Debug==""` 補 `" "`；Greatek＋OEE 另 `SaveMessageHistroy` |
| `RecordChangeLogProcess(S, S2)` | `:1575` | 21 | `MyDBIProcess("ChangeLog", S, S2)`；`WriteIniData` 改值時自動叫 |
| `MyDBIProcess(asTable, S1, S2)` | `:789`，66 行 | 614 | 落地器（下） |

去重規則（三個入口相同）：`S==ExString && S.Pos(" pressed")!=0 → return`，ASE 高雄與 SPIL 例外。**所以訊息文字的 `" pressed"` 後綴是語意的一部分**，HTML 送來的 `msg` 要沿用這個慣例。

`MyDBIProcess` 的四個落地：

1. **sqlite**：`INSERT INTO <asTable>(<asTable>, Debug, OccurDateTime) VALUES('S1','S2', datetime(CURRENT_TIMESTAMP,'localtime'))` → `MyDBExecSQL`。表名就是欄名（`Process.Process`、`ChangeLog.ChangeLog`）。
2. **文字**：`ProductionLog(S1+S2, true)`；`slEventLog->AddTextWithDateTime(SL->CommaText)`（SPIL 用 `AddTextWithLineNo`）→ `SaveEventLog()`。`CommaText` 8 欄：`UnitName, AlarmCode, OccurDateTime, Recovery, StopedTime, Duplicate, Message, ErrPart`（非 SPIL 時中間四欄是 `\t` 佔位）。檔案在 `<root>\YYYY\MM\` 底下（`wb_serve` 的 `/api/text/` 已能讀）。
3. **事件層**：`SaveEventLogInfo("220000000", S1, 22, " ")`（`:1723`，70 行）—— `iType` 主旨：`0 ERROR／10 START／11 END／12 INPUT／13 PASS／14 FAIL`；帶 Recipe／LotID／TesterID／HandlerID；非 `220000000` 的另進 `SaveEventTracker`。
4. **ASE 高雄**：`RespondASECom("@e02002"+S1+S2)`（`S1` 含 `ATC` 除外）。

相關：`SendCommand_EventLog(EL_*, Data)`（`Interface/InterfaceSYS.cpp:479`）是**FTP 上傳排程**，7 個 `EL_*`（`InterfaceSYS.h:206`），不是留痕本身。

### 移植樹現況

- `cMyDB.cpp` 已翻譯且連進 `wb_serve`（`CMakeLists.txt:1233`）：`MyDBIProcess :1000`、`NewRecordProcess :1803`、`RecordProcess :1831`、`RecordChangeLogProcess :1852`。
- ⚠ 檔頭（`:19-75`）明講：`SaveEventLogInfo`「file I/O gated」；`slEventLog` 是 incomplete type（`cmydef.h:117`），`slEventLog!=NULL` 那條永遠走不到。
- ⚠ `wb_serve` **不開 sqlite**（`wb_serve.cpp:3316`）：`MyDBExecSQL` 對 null `dbReadWrite` 是 crash。
- ⚠ 兩個空 shim 會讓呼叫端誤以為有留痕：`canary_support.cpp:116 RecordProcess`、`acatchtray_shims.cpp:152 NewRecordProcess(){}`。
- ⇒ 橋接層 P8 的第一步是**開 DB、解 gate、退場 shim**，之後 `log.event` 才有東西可寫；之前一律誠實回 `db not open`。

## E. 三套機制在移植樹的現況（20260923）

| 項目 | 狀態 | 位置 |
|---|---|---|
| `OpenIniFile`／`ReadIniData`×4／`ReadIniDataMem`×4 | 已翻譯 | `common.cpp:525-897` |
| `WriteIniData` bool／int／double／AnsiString | 已翻譯 | `common.cpp:1008-1178` |
| `TIniFile`／`TMemIniFile` shim | 有 | `vclcompat/IniFiles.h:136` / `:196` |
| `HTEditList::SaveEditTextToFile`／`ReadEditTextFromFile` | 已翻譯（429／306 行） | `Public/HTEditList.cpp:594` / `:1024` |
| `ReadLastDataFile`／`WriteLastDataFile` | 已翻譯 | `cprod.cpp:1691` / `:2001` |
| `GetLevelSet`／`SetLevelSet` | 有呼叫點 | `cSecurity.cpp:271` / `:533` |
| `TFTestIF::ReadTestIFFile` | 已翻譯（⛔ 20260926：移植樹這份在 GATE (F-5) 的 `#if 0` 裡；在跑的是 C 路 `FileRW/TestIF_File_TesterIF.cpp` 那份） | `forms/fTesterIF.cpp:807` |
| 控制項 shim | 8 個：`TEdit` `TComboBox` `TCheckBox` `TRadioGroup` `TMemo` `TLabel` `TStringGrid` `TSpeedButton` | `vclcompat/` |
| `cUnitConvert`（`_File`→執行中 轉換層） | 本體已翻譯，**開機呼叫被 `#if 0`／`#define` stub 擋住** | `cUnitConvert.cpp:370-693`；`cinitial.cpp:7175`；見 `unit-convert-layer.md` |
| **沒有的** | `TDateTimePicker` shim；`WriteIniDataMem`；`sizeof(LAST_GENERAL_SET)` 對齊驗證 | — |

結論：**三套機制的讀寫原語移植樹都有了**，橋接層不需要重寫任何一套，只需要在它們上面加 `FieldDesc`／`Binding`／`ToJson`／`FromJson`。缺的是「哪個欄位對哪個鍵」那張表——那正是產生器要出的東西。

> ⛔ **20260924 更正（使用者兩次再裁，見 `decisions.md` 二之二、二之三）**：② 與 ③ **不翻譯，但也不廢除**——
> 改由 `tools/gen_formbridge.py` **直接讀 golden 原檔**，把「控制項」機械改寫成 JSON（`FormState`），
> 產出 `WriteFile/<結構>.cpp`（20260924 深夜資料夾改名 `FileRW/`，設定也拆成 `tools/formbridge/<Class>.py`，見 `generators.md`）。
> 所以 ③ 的 `WriteIniData` 呼叫、格式字串（本檔 §A.3 那張表）、存檔鉗制**原樣保留在產生檔裡**，
> 不必搬進 `Clamp*`／`FieldDesc.fmt`。下面這段原文保留作為歷史。

⚠ 但 **golden 的 ②（`DoIniDataToForm`）與 ③（`SaveSetupFile`／`SaveEditTextToFile` 的控制項讀值部分）在新架構不翻譯**（使用者 20260923 裁決：顯示層是 HTML）。移植樹裡已翻譯的 `SaveEditTextToFile`（429 行）與各表單的 `SaveSetupFile` 因此只剩兩個用途：(a) 當 `Clamp*` 與 `FieldDesc.fmt` 的**對照原文**，(b) 給還沒接上 HTML 的舊路徑暫用。不要在它們上面繼續加功能。
