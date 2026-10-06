> 保存來源：`.claude/skills/ht9045-json-bridge/references/measured-facts.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# §三、量到的事實（改設計前先讀） —— 全文

> 從 `SKILL.md` **§三** 拆出（拆出日期 2026-09-23，skill 維護第二輪續拆）。
> **章節編號不變**：§3.1～§3.6 同名同號；**物理順序照原檔是 3.1→3.2→3.3→3.6→3.4→3.5**，不是錯，不要重排。原文**逐字**保留（含已被更正段推翻的原句 —— 那是稽核軌跡），
> `SKILL.md` 原處留 stub（一句話結論 ＋ 指到本檔）。
> 原本被誰引用：SKILL.md §〇（速查表多列的權威出處）、§十二（審查員逐條重量的就是本節的「實測值」）、§九 陷阱 1～3e；全樹 `.cpp`／`.h`／`.py` 無直接引用 §3.x（2026-09-23 grep 實測）。
> 本檔內的 `§X`／`§4.x` 交叉引用一律指 `SKILL.md` 的章節編號，`SKILL.md` 那裡搜得到、再一跳就到對應 reference；
> `references/<檔>.md` 這種路徑是以 `SKILL.md` 所在目錄為基準寫的（原文未改），在本檔內即同目錄的 `<檔>.md`。

---

## 三、量到的事實（改設計前先讀）

### 3.1 golden 的讀寫檔是三段式，新架構只留一段半

```
golden（BCB6，顯示層是 VCL）
  ① ReadFile()          檔案 ──→ 全域結構      ReadIniData(path,"區段","鍵",預設值)
  ② DoIniDataToForm()   全域結構 ──→ VCL 控制項
  ③ SaveSetupFile()     VCL 控制項 ──→ 檔案    WriteIniData(path,"區段","鍵",值)

移植樹（顯示層是 HTML，使用者 20260923 裁決）
  ① ReadFile()          檔案 ──→ 全域結構      JerryYang 逐字翻譯（不變）
  ②' ToJson()           全域結構 ──→ JSON ──→ HTML     取代 ②，Steven
  ③' FromJson+Persist   HTML ──→ JSON ──→ 全域結構 ──→ 檔案   取代 ③，Steven
  然後回到 ①：存檔後再讀檔，再 ②' 回 HTML（#6）
```

量到的、決定 ③' 怎麼寫的事實：

- golden ③ 讀的是**控制項**：`cHotPlate.cpp:598-605` 寫出去的是 `HotPlateName->Text`／`XCT1->Text`，不是 `HotPlateForm_File.*`。`SaveAllFile()`（`csystem.cpp:23542`）對 12 個表單一律先 ② 再 ③。**新架構沒有控制項，所以 ③ 不能逐字翻譯**，`WriteIniData` 的來源改成結構欄位。
- golden ③ 裡有「存檔時才做的鉗制」（`cHotPlate.cpp:495-510`：`i8PickerHPMode==iHPWideHP` 時 `XCT1` 只能是 12 或 ≤8；`HTEditList::SaveEditTextToFile` 的 min／max 範圍檢查）。**這是丟掉控制項後唯一會遺失的邏輯**，必須逐表單搬進 ③' 的 `Clamp` 鉤子，並在 `FromJson` 的 `dryRun` 就回報，不等寫檔才發現。
- golden ③ 的**值格式**跟著控制項走（`FormatFloat("0.000", XST1->Text.ToDouble())`、`HTEditList` 的 `iDecimalPoint`／`iTransformType` 單位轉換）。③' 要把這些格式帶進 `FieldDesc`，否則 G1 的「位元組 diff 只有那一行」過不了。

### 3.2 讀寫檔有三套持久化機制

| 機制 | 讀 | 寫 | 例 |
|---|---|---|---|
| ini 文字 | `ReadIniData`（全樹 3,704 次） | `WriteIniData`（3,609 次） | `HotPlate.Data`、`Tester.Data`、`Temperature.Data` |
| `HTEditList` 文字 | `ReadEditTextFromFile`（15） | `SaveEditTextToFile`（16） | `UdUld.Data`、`Contact.Data`、`HandlerCondition.Data`、`config.ini` |
| 二進位 blob | `ReadData`／`ReadLastDataFile` | `WriteData`／`WriteLastDataFile` | `lastdata.dat`（`LAST_GENERAL_SET`）、`levelset.dat`（`LAST_LEVEL_SET`） |

`TfLd_ULd` 整個檔一個 `ReadIniData` 都沒有，完全靠第二套；`TfContact` 兩套都用。**只認第一套會讓核心配方檔整個消失**（表⑧第一版就漏了 3 個單位）。

### 3.3 每個結構有三個實例；`_File` → 執行中 的套用點是 `cUnitConvert.h`

```
TestIF_File    11,797 次   檔案裡那份，畫面單位（mm／秒）。ReadFile 灌它，網頁編它
TestIF          5,598 次   執行中那份，馬達單位（0.01 mm 的 int）。由 DoTestIFConvert() 從 _File 整包 memcpy 後對 10 個欄位 ×100
TestIF_NET         32 次   FTP 快照，TestIF_NET = TestIF_File 整包複製   ← 不開
```

⚠ **更正**（20260923）：第一版寫「執行中那份從未被整包指派」是錯的——grep `TestIF = TestIF_File` 漏掉了 `memcpy(&TestIF.iTestMode, &TestIF_File.iTestMode, sizeof(TestIF_File))`（`cUnitConvert.cpp:29`）。`DeviceForm`／`HotPlateForm`／`ArmSpeed`／`UserDefForm`／`*ArmOffSet`／`Offset` 都有同型的轉換函式，總指揮是 `DoStructUnitConvert()`，golden 在**每次讀檔後、每次 Save 後**叫一次（24 個呼叫點）。全文見 `references/unit-convert-layer.md`。

⇒ 三個推論：
1. `*.live` 綁定**不另外讀檔、不另張 `FieldDesc`**：`live = Convert(file)`，`ToJson(live)` 就是叫 golden 的 `Do*Convert()` 再讀執行中結構。
2. **`struct.put` → `Persist` → `Reload` 之內要叫一次、且只叫一次 `DoStructUnitConvert()`**，這是照 golden，不是發明。不叫＝「檔案改了、機台沒變」；叫兩次 `ArmSpeed[OutArm].dWaitOnSH` 會累加（`:263-270`）。
3. **三層單位（使用者 20260923）：HTML 用 mm；機台從檔案讀進來也是 mm（`_File`）；只有給馬達的時候才是 0.01 mm（執行中結構，`Do*Convert()` ×100）。** 橋接層只碰前兩層，兩層值相同、不轉換；×100 是馬達層用值時的事，不進線上、不進 `FieldDesc`。`*.live` 唯讀綁定回的是第三層的值，僅供顯示。

⚠ **移植樹現況**：`cUnitConvert.cpp:675`（本體，全檔 693 行）已翻譯，但 `cinitial.cpp:7175` 用 `#if 0` 擋住開機呼叫、`Automation/auto9045.cpp:153` 與 `ckernel_shims.cpp:113` 用 `#define` 把它換成空 stub。**這很可能就是「JerryYang 讀進 `TestIF_File` 的值到不了 `TestIF`」的直接原因**，在「沒有 JSON」的更上游。P0 之前先釐清 `wb_serve` 開機有沒有叫到真的那個。

### 3.6 `IniConfig` ↔ `config.ini` 族與 `LAST_GENERAL_SET`（使用者 20260923 點名 `ReadLastSetIni`／`SaveLastSetIni`、`Config.Configuration.html`、`Config.DIOInterFaceCFG.html`）

`ReadLastSetIni()`／`SaveLastSetIni()`（`cprod.cpp:2977`／`:3092`）**名字誤導：主體是 `IniConfig` ↔ `config.ini`，不是 `LAST_GENERAL_SET`**。量到的：

| 成分 | 機制 | 規模 | 目標 |
|---|---|---|---|
| `elConfig->Add(...)` | `HTEditList` → `config.ini` | **1,581 筆**（`cConfiguration.cpp` 的 `InitConfigEdtList_ItemA..P`） | `IniConfig.*` 1,578、`TrayForm.*` 3 |
| `cbLastSet->Add(...)` | `HTEditList` → `LastSet.ini` | 91 筆 | `IniConfig.*` 90、`LastSet.*` **1** |
| `elConfig_byRecipe->Add` | `HTEditList` → `configByRecipe.ini` | 20 筆 | `IniConfig.*` |
| 16 個 `ProcessLastSetIni_*(bRead)` | **第五種呼叫形式 `ReadWriteIni(path, 區段, 鍵, 現值, 預設, bRead[, min, max])`**（`common.cpp:1489-1561`，5 個多載，同一行依旗標讀或寫，含 `CheckRange`） | 產生器第二版**不認**，表⑧ 因此把它們量成 0 | `IniConfig.*`（含 `ContactSet[3][2][16]` 等三維計數陣列） |
| `ReadLastDataFile()` | 二進位 blob | 在 `ReadLastSetIni` 開頭被呼叫 | `LastSet` 整包 |
| `CheckAndReadIniDataGeneral` | 對 `Gerneral.ini` 讀不到就**寫回預設**（`common.cpp:1423-1457`） | 4 個多載 | `IniConfig.sMachineType` 等 |

頁面對應：`Config.Configuration.html` ↔ `TfConfiguration`（wire 163 筆全綁 `config`）；`Config.DIOInterFaceCFG.html` ↔ `TfDIOFrom`（`DIOInterFaceCFG.cpp:71 LoadData`／`:132 DoIniDataToForm`，11 讀／11 寫，`iniData\DioCfg\*.ini`，wire 4 筆綁 `dio`）。兩者都**不是** `ReadFile()`／`SaveSetupFile()` 命名，所以表⑧ 沒列——是產生器的盲區，不是它們沒有讀寫檔。

`LAST_GENERAL_SET` 本身（387 欄、`lastdata.dat`）的寫入點：1,165 處賦值散在 67 個 `.cpp`；有畫面的是 `Config.Configuration`（68 處／31 欄）、`Main.*`（176／41）、`Data.CounterClear`（33／12）、`Setup.StartCondition`（30／5）、`Data.LotInfo`（34／15）。**`HW.HandlerSys.html` 是 0 處**——它的 wire 214 筆全綁 `gerneral`（`Gerneral.ini`／`HSys`），BCB `HandlerSys.cpp` 也不碰 `LastSet`。

### 3.4 結構大小（**剝註解後**做大括號配對；量法見下方 ⚠，20260923 全部重驗過）

| 結構 | 成員 | 陣列 | 持久化 | 對應檔 |
|---|---|---|---|---|
| `LAST_LEVEL_SET` | **1**（`int AccessLevel[256]`） | 1 | blob | `system\levelset.dat`，256 個小端 int32 |
| `SYSTEM_DEVICE_FORM` | 66 | 10 | ini＋editlist | `Contact.Data` |
| `SYSTEM_TEST_IF` | **788**（golden 796） | 55 | ini | `Tester.Data`／`HandlerCondition.Data [Configuration]` |
| `SYSTEM_TRAY_FORM` | 44 | 11 | ini＋editlist | `Tray.Data` |
| `SYSTEM_TEMPERATURE` | **223** | 43 | ini | `Temperature.Data` |
| `SYSTEM_TEST_MODE` | 5 | 2 | ini | `TestMode.Data` |
| `LAST_GENERAL_SET` | 387 | 139 | blob | `system\lastdata.dat` |

⚠ **量法（20260923 重訂，前一版有缺陷）**：從 `} NAME;` 往回做大括號配對，
但**配對前一定要先剝掉 `//` 與 `/* */`**，而且剝的時候要認得字串字面值。
剝完之後頂層分號數就是成員數（這七個結構都沒有巢狀大括號）。
巢狀型別宣告（`enum` / `struct` / `union`）不是欄位，要排除。

前一版的腳本沒有剝註解就配對，結果：

| 結構 | 舊值 | 正確 | 錯在哪 |
|---|---|---|---|
| `SYSTEM_TEST_IF` | 351 / 45 陣列 | **788 / 55** | `cprod.h:2156` 行尾註解裡有一個 `{`，反向配對咬住它，量到的只是尾巴（漏 437 欄） |
| `SYSTEM_TEMPERATURE` | 224 | **223** | 把結構內的 `enum BoostFunction` 算成一個成員 |
| 其餘五個 | 1 / 66 / 44 / 5 / 387 | 相同 | 碰巧正確 —— 它們前面剛好沒有註解裡的 `{`。**是運氣不是方法對。** |

現在的權威來源是 `tools/gen_sjson.py`（它的 `strip_comments` 就是為這件事存在的），
重跑一次就會印出全部七個的成員數與陣列數。

### 3.5 目前橋接層的真實狀態

- 表⑧（20260923 **第三版**產生器：括號／引號感知的參數切分，認變數區段、`Obj->Set*()` 目標、`ReadWriteIni`、`CheckAndReadIniData`、`ReadIniDataMem`、`ProcessLastSetIni_*`）：**57 個讀寫檔單位、BCB 讀 1,159 鍵／寫 1,162 鍵、移植樹讀 532／寫 404；讀檔欄位 553 個，只有 3 個出得了 JSON（0.5%）**：`TestIF_File.dSiteXPitch`／`dSiteYPitch`／`TrayForm.bEnableAMR`。
  ⚠ 早上 11:05 寄給研五軟體的信與 commit `a332397` 用的是第一版數字（30 單位／661／951／514／0.6%）；第二版 35／898／1,158；差異全部來自產生器補認呼叫形式（§九 3b／3d），不是程式碼變了。
  ⚠ `IniConfig` 的 `elConfig->Add()` 1,581 筆仍**不在**表⑧（那是註冊式 IO，不是函式呼叫），所以 553 這個分母仍低估；第四版要補（§十 #8）。
- `GET /api/recipe/<doc>` 回的是**檔案的通用鏡像**（每鍵 `{value,type,raw}` 三份），不是 `ReadFile()` 算完的結構。兩者會不一樣：`Use Wide Hotplate` 只在 `IniConfig.bHotPlateMove1CM` 為真時讀檔，否則一律 `true`（`cHotPlate.cpp:193-200`）；`ReadFile()` 另有 4 條這類修正規則。**踩到任何一條，畫面顯示的就不是機台在用的值，且沒有徵兆。**
- `GET /api/system/levelset` 已存在（`wb_serve.cpp:1359`），但**索引定址、沒有欄位名**。
- 執行期 tag（20260919 實測）：4,608 個，其中 4,481 個是 `pci1203.*`；`io.*`／`motor.*`／`task.*`／`system.*`／`alarm.*` 五個 prefix 在 `WebBridgeTags.cpp` 裡 **0 個**。
- `WebBridge/WsFrame.cpp:39` 支援 64 位元長度欄位（>65536）。`ht9045-html-json` 寫的「64 KB WS 上限」**尚未在現行程式碼裡找到對應常數**，設計時不要依賴一個沒量到的上限 —— 先量。
- DI／DO 窗口：`kPci1203MaxDiPorts = 320`、`kPci1203MaxDoPorts = 192`（~~`Pci1203Monitor.h:383`~~ → **`:403-405`**，Q34-2 第四次放大）。
  ⚠ 更正（20260923）：`:383` 是那段註解的開頭，不是 enum 的位置。tag 層另有一對
  `kPci1203TagDiPorts` / `kPci1203TagDoPorts` 在 **`:1637-1638`**（標著 `== kPci1203MaxDiPorts`）——
  **`ChanIo.cpp` 用的是 tag 層那一對**，這樣下一次窗口放大時兩邊會一起被改到。
  ⚠ 一個 port 是**一個位元組**不是一個位元（`Pci1203DiSample::byteData` 是 `unsigned char`，`:1099`）。見 §六 的更正段。


<!-- preserved-content:end -->
