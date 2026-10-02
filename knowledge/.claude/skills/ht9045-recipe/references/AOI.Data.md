# AOI.Data — AOI／Scanner AOI／OCR 設定；golden 兩個鍵名錯誤與 906 的單邊修法

> 給之後要碰 AOI.Data、做 AOI／OCR 頁面的 Claude session 與 Steven。
> 核對基準：`git -C D:\HT9045 show HEAD:<路徑>`，HEAD＝`89ccb4cc`（分支 `v906/steven-cbridge-review6`），2026-09-27 14:3x 核對。
> 三棵樹：**移植樹**＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`；**golden V912**＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（cp950）；**V899**＝`D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422`（唯讀）。
> 下文「V912 `:行號`」沒寫檔名時，指 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\fAOI.cpp`。
>
> 相關 commit：
> - `9ec84450`（S69，golden TFrmAOI 讀寫）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.gen.inc`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\AOISetup.py`；開機／換配方接線 `e6a8e0fc`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:3454`）；`4e8c93af`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.h:141` 補 `bEnabledPositionByAOI`）
> - `973f2540`（S86，golden TfOCR `[OCR SETTING]`）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig_OCR.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig_OCR.gen.inc`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\IniConfig_OCR.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:3416`
> - `725038a6`（Q31＝A'，修兩個鍵名錯誤）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\AOISetup.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.gen.inc:1195-1202`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.cpp`（註解）

## 1. 基本事實

- **位置**：`D:\HT9045\IniData\Data\<配方>\AOI.Data`（V912 `:3139`、`:3376` `szDir.sprintf("%s%s\\AOI.Data", DataPath, S)`）。cp950，同其他 .Data。
- **不是每個配方都有。** Steven01 20260927：`D:\HT9045\IniData\Data` 216 個配方夾只有 4 個有 AOI.Data，其中 2 個只有 `[OCR SETTING]`；目前配方 `QPM5577_8`（`D:\HT9045\setup.inf`）沒有。
- **讀檔不補鍵、不建檔。** 兩支讀檔器都用 `ReadIniData` ⇒ 沒檔＝全部預設值。唯一例外是 OCR 的 `D:\HT9045\system\Gerneral.ini [OCR SETTING] "OCR Port"`（`CheckAndReadIniData`，缺鍵照 golden 補寫 24；V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\OCR.cpp:2212`）。
- **兩個寫者、分段不重疊。** 都是逐鍵 `WriteIniData`（不整檔回寫），互不覆蓋：

| 段 | golden 表單（V912） | 移植樹 | 讀進哪裡 |
|---|---|---|---|
| `[SETTING]` `[DutOnOff_BGAView]` `[DutOnOff_PADView]` `[RS232]` `[AOITRAY]` | `TFrmAOI`：`fAOI_ReadFile` `:3369`、`spbSaveClick` `:3120` | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.cpp`（S69） | `tAOISetup`、`ScannerAOIIF`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.h`）、`bVitroxBGA/PADViewUse/Map`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fAOI.cpp`）、`MOT[MMScanAOI].Tray.Data` |
| `[OCR SETTING]`（19 鍵） | `TfOCR`：`fOCR_ReadFile` `OCR.cpp:2197`、`spbSaveClick` `OCR.cpp:2151` | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig_OCR.cpp`（S86） | `IniConfig` 的 OCR 欄位 |
| `[TopBottomInspect]` `[ZPickOffset]` `[Function Setting]`（70 多鍵，只有 Top&Bottom 機台） | `TTopBottomInspect` 的 `elParameter` | **沒有移植**（Jimmy） | 存檔時保持檔案原值 |

## 2. 讀取時機

- golden `TfMain::DoReadLastData`（開機與換配方）：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:9367` `fOCR->fOCR_ReadFile()`、`:9376` `FrmAOI->fAOI_ReadFile()`。
- 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 的 `W906_DoReadLastData`：`:3416` `FileRW_IniConfig_OCR_ReadFile()`、`:3454` `FileRW_AOISetup_ReadFile()`。
- ⚠ 行號：移植樹註解寫的 `main.cpp:9365`／`:9374` 是舊副本 `D:\HT9045_ref`（20260926 退場）的行號。主 repo 的 V912 `main.cpp` 之後被 Jimmy 改過，現在是 `:9367`／`:9376`。`fAOI.cpp`、`OCR.cpp`、`ContactForce.cpp`、`HT9045.cpp` 的行號沒有位移。

## 3. golden 的兩個既有錯誤（V912 與 V899 都有；V912 不改）

| 設定 | 存檔寫的鍵 | 讀檔讀的鍵 | 結果 |
|---|---|---|---|
| Bottom Start Delay | `StartDelayTimeScanAOIView`（`:3236`） | `StartDelayTimeScanAOIView`（`:3471`） | 被下一列蓋掉 ⇒ 讀到的是 **Top 欄位的值** |
| Top Start Delay | `StartDelayTimeScanAOIView`（`:3300`，**寫到 Bottom 的鍵**） | `StartDelayTimeTopScanAOIView`（`:3494`） | 沒人寫這個鍵 ⇒ 永遠預設 10 |
| Bottom Timeout | `TimeOutScanAOIView`（`:3237`） | `TimeOutScanAOIView`（`:3472`） | 正常 |
| Top Timeout | `TimeOutTopScanAOIView`（`:3301`） | `TimeOutScanTopAOIView`（`:3495`，**字序不同**） | 畫面打的值從來沒被讀到 ⇒ 永遠預設 3 |

- V899（`D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\fAOI.cpp`）同樣：寫 `:2938-2939`、讀 `:3102-3103`。
- **例**：Steven01 的 `D:\HT9045\IniData\Data\buyoffuse\AOI.Data` 與 `D:\HT9045\IniData\Data\ZX222021H8-FT7-V0\AOI.Data` 都只有 BCB6 寫的 `TimeOutTopScanAOIView=3`，沒有 `TimeOutScanTopAOIView`，也沒有 `StartDelayTimeTopScanAOIView`。

## 4. 906 的修法（Q31＝A'，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` S152；鍵名 R71＝A）

**規則：只改寫檔那兩行，讀檔照 golden。**

- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.gen.inc:1195`：Top 延遲改寫 `StartDelayTimeTopScanAOIView`（`:3494` 讀的鍵）；golden 原行留在 `:1196-1198` 的 `#if 0`。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.gen.inc:1199`：Top Timeout 改寫 `TimeOutScanTopAOIView`（`:3495` 讀的鍵）；golden 原行留在 `:1200-1202` 的 `#if 0`。
- 兩行都是產生器設定 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\AOISetup.py` 的 REPLACE（`_SV_TOPDLY` `:171-176`、`_SV_TOPTO` `:177-183`；理由在檔頭 `:41-54`）。重產用 `python tools/gen_editlist.py --only AOISetup`。**不要手改 gen.inc**，重產會蓋掉。

**後果**
- 同一份 AOI.Data 放在 BCB6 或 906 上，**讀出來一樣**（讀檔沒動）。
- 906 存出來的檔跟 BCB6 存的差三點：
  1. Bottom 的 `StartDelayTimeScanAOIView` 是 Bottom 自己的值，不再被 Top 蓋；
  2. 多兩把鍵 `StartDelayTimeTopScanAOIView`、`TimeOutScanTopAOIView`（檔裡原本沒有的話）；
  3. 不再寫 `TimeOutTopScanAOIView`。檔裡的舊值留著、沒人讀（`WriteIniData` 不刪鍵）。
- **例**（decisions-pending R71）：BCB6 機台 Top Timeout 打 5 並存檔 → 檔裡 `TimeOutTopScanAOIView=5`，BCB6 實際跑 3。這份配方拿到 906 一樣跑 3、頁面顯示 3；在 906 改成 5 存檔 → 寫 `TimeOutScanTopAOIView=5`，拿回 BCB6 也跑 5。
- V912 不改（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md` 第 1 條：HT9050 專案只改 906 C++），只記給 Jimmy。
- 裁決紀錄：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` R71（St01 建議 A，已照做、可推翻）。

## 5. 現況與限制

- **修法還沒生效。** `FileRW_AOISetup_spbSaveClick()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.cpp:106`）沒有呼叫者：`D:\HT9045\web\page` 沒有 AOI 頁、沒有 WS 指令、沒登記 PageDesc（grep HEAD，20260927 14:29）。OCR 的 `FileRW_IniConfig_OCR_FormShow()`／`FileRW_IniConfig_OCR_spbSaveClick()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig_OCR.cpp:88`／`:97`）也一樣。開機／換配方的**讀檔**已接（§2）。
- **接頁時要先灌值再存。** 呼叫存檔前替身必須已是頁面值（開頁＝`fAOI_ReadFile`／OCR 的 `FormShow`）；沒灌值就存，會把 DFM 設計期值（多半 0 或空字串）寫進 AOI.Data（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.cpp:104-105`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig_OCR.cpp:42-43`）。
- **存檔尾巴的 `fMain->BackupSetupFile()` 在 wb_serve 裡是真的。** `FileRW\AOISetup.cpp:18` 與 `FileRW\IniConfig_OCR.cpp:19` 寫「offline no-op」已過期：wb_serve 開機已裝 golden 本體（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4111` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBackup.cpp:170`，S92；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:458` 經函式指標轉）⇒ 在 wb_serve 裡存 AOI.Data 會重寫 `<配方>\*.MD5`。只有 ctest 沒裝、才是 no-op。
- **B 路沒擋。** `aoi.data` 不在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:1289-1319` 的 `CRouteOwner` kOwned 表 ⇒ `/api/recipe` 的 `recipe.doc.put` 可以直接改 AOI.Data，改了記憶體不會重讀。裁決：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md` R19＝A（維持現狀，等做 OCR／AOI 頁時改走 C 路擋掉）。
- **另一個過期註解**：`FileRW\AOISetup.cpp:33` 說「`ScannerAOIIF.bEnabledPositionByAOI` 移植樹 cprod.h 沒有這一欄 → 讀／顯示／寫三處擋」。`4e8c93af` 已補欄位並解開三處（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.gen.inc:638`、`:887`、`:1146`）。
- **其他 golden 怪處照翻、沒修**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\AOISetup.py:55-58`）：Top Scan AOI 的 5 個警報設定有寫沒讀（記憶體恆 0，再存就寫 0）；`iBallDamageType` 有讀沒寫；`fScannerICGain` 讀兩次（無害）。
- **OCR 的兩個已知缺口**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig_OCR.cpp` 檔頭）：
  - V912 `OCR.dfm` 少 `rgOCRTriggerMode`（V899 有）⇒ V912 開 OCR 表單或存檔會存取違規，存檔只寫到前 14 鍵。V912 不改（RULINGS_20260927 第 1 條，夜間報告第 6 題）。移植樹替身存在，不會出錯。
  - 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\OCRInsp.cpp` 的 TU 內替身把 `IsOCRCommandTrigger` 寫死 false、`CheckOCRWordType` 寫死 true ⇒ 讀進來的 `iOCRTriggerMode`／`asOCRWordType` 對 OCR 流程暫時沒作用，要等 OCR 執行期（Jimmy）拿掉替身。
