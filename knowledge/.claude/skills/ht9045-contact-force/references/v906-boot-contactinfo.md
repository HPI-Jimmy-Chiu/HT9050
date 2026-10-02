# 移植樹 ContactForce：開機時序、開機寫檔、golden 怪處（V906，20260927）

> 給之後要改 ContactForce、`ContactInfo.ini` 或 EP 鍵的 Claude session 與 Steven。
> 核對基準：`git -C D:\HT9045 show HEAD:<路徑>`，HEAD＝`89ccb4cc`（分支 `v906/steven-cbridge-review6`），2026-09-27 14:3x 核對。
> 兩棵樹：**移植樹**＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`；**golden V912**＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（cp950）。
> 下文「V912 `:行號`」沒寫檔名時，指 golden V912 的 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ContactForce.cpp`。
>
> 相關 commit：
> - `21d37f2b`（S57 讀寫檔翻譯）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.gen.inc`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce_Panels.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp`
> - `725038a6`（R15＝B，建構子開機就跑）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4062`（只補註解）
> - `f1ad780c`（S90，EP 四鍵兩頁都寫）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp`

## 1. 規則（先記這四條）

1. **開機就跑 golden `TfContactForce` 建構子**（R15＝B，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` S162）。「開機不碰 ContactInfo.ini」「建構子延到第一次開頁」都是 `725038a6` 之前的說法。
2. **機台用的值來自 `ContactForceTables()`，不是頁面面板。** 開機由 `LoadContactForceTables()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp:43`）讀；讀者是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\adam6024.cpp`（TransformFuntion）與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp`（Setup.Contact）。頁面存檔後 `RefreshShared()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp:145`）重跑同一支載入器。
3. **`D:\HT9045\system\ContactInfo.ini` 路徑寫死，`--dry` 擋不住。** golden V912 `:428` 字面值，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp:26` 與 gen.inc 照抄；`--dry` 只重導 `asGeneralPath`（Gerneral.ini）。任何一次 wb_serve 開機都會碰這個檔。
4. **只有 wb_serve 連 `FileRW\ContactForce.cpp`，ctest 不連**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp:401`）⇒ ctest 綠證明不了開機行為對。

## 2. 開機順序（移植樹 ↔ golden）

| 順序 | 移植樹 | golden |
|---|---|---|
| 1 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4062` `FileRW_HSys_Boot()` | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HT9045.cpp:210` `CreateForm(THandlerSystem)` |
| 2 | 同一行 `FileRW_ContactForce_Boot()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp:419`）：建替身 → `EnsureCreated("boot …")`（:198）→ `CF_TfContactForce()`＝V912 建構子本體 `:421-653` | `HT9045.cpp:220` `CreateForm(TfContactForce)` |
| 3 | 同一行 `FileRW_ACTForm_Boot()` | `HT9045.cpp:221` `CreateForm(TACTForm)` |
| 4 | `wb_serve.cpp:4065` `InitialHandler()` | TfMain::FormShow 的 InitialHandler |
| 5 | `wb_serve.cpp:4095` `LoadContactForceTables()` → 機台用的表 | 沒有這一步（golden 的表就是建構子建的那一份） |

- **前提**：在 `LoadMachineConfig()` 之後。建構子讀 `CosFunction.bUseDynamicKitDiameter`、`CUSTOMER_CODE`、`EP_Install`、`INSTALL_DOUBLE_EP`、`iIndEPCnt`、`IniConfig.bSPILFunction`。`TestIF_File.iTestMode`（`iIndEPCnt==8` 那一支用）這時還沒讀配方——golden 同（DoReadLastData 在所有 CreateForm 之後）。
- **`EnsureCreated(when)` 冪等**，開機之後的呼叫點都只是保底：開頁 `FormShowFlow`（`FileRW\ContactForce.cpp:214`）、存檔 `SaveFlow`（:233）、`FileRW_ContactForce_ReadFile()`（:463，留給 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\iosetview.cpp:3451` `Label120Click`，**目前沒有呼叫者**）。log 的 `ran at …` 後面出現 `fallback` ＝ 開機沒跑到，要查。
- **換配方不讀**：ContactInfo.ini 不跟配方（golden DoReadLastData／ChangeSetUpFile 沒有 fContactForce）。

## 3. 同一個檔、兩份表

| | 頁面面板（四個 vector，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce_Panels.h`） | 機台用的表（`ContactForceTables()`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForce.h`） |
|---|---|---|
| 誰建 | V912 建構子（§2 第 2 步） | `LoadContactForceTables()`（§2 第 5 步；頁面存檔後再跑） |
| Ind 表型號清單 | 跟 **`[SLK Type]`**（V912 `:453` `edtCurrentTypeInd->Text=edtCurrentType->Text`，使用者 20260909 裁決） | 跟 **`[SLK Type Ind]`**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp:164-169`、`:186-194`） |
| 誰讀 | 只有 Setup.ContactForce（`editlist.get` 的 `extra.panels`） | adam6024、Setup.Contact（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp:199-205`） |

- 兩份經同一個檔同步。`SLKClass`／`DieForce` 兩份內容相同（同一個 `[SLK Type]` CSV），只有 Ind 表不同。
- Ind 表不同只影響 `INSTALL_DOUBLE_EP` 是 2 或 3、且 `bIndEPSLK` 開著的機台（S57 交件報告）。
- **例（Steven01，20260927 讀 `D:\HT9045\system\ContactInfo.ini`、`D:\HT9045\system\Gerneral.ini`）**：`[SLK Type] Type=30,40,60,56,80`、`[SLK Type Ind] Type=20,30`、`EP_Install=3`、`INSTALL_DOUBLE_EP=1`、`CUSTOMER_CODE=791`。頁面面板 SLK 5／Ind 80／DieForce 5／OneByOne 0（S57 交件報告）；機台用的表 Ind＝2 型 × 16＝32（推算，未實跑）。這台看不到逐站頁籤。

## 4. 開機會寫的檔（sysguard 預期變動）

只在 `CosFunction.bUseDynamicKitDiameter==true` 時（V912 `:430`；V910 起預設 true、全樹沒有設回 false，見 SKILL.md「`rgKitDiameter` 的選項」一節）。

### 4.1 `D:\HT9045\system\ContactInfo.ini`（`--dry` 擋不住）

- **有檔**（`bHasFile`）→ 建構子本身（`:439-449`、`:580-587`）與建構子 `:641-642` 跑的 `ReadFile`（`:966-1181`）用 `CheckAndReadIniData` 補缺鍵：缺才寫、有就讀；double 寫成 `"%0.4f"`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:677-693`）。會補的是：
  - `[SLK Type]` Type／Visible（`INSTALL_DOUBLE_EP>0` 另加 DieForceType／DieForceVisible）
  - `[Diameter_<d>.000mm]` 五鍵（`EP_Install==5` 與 `CC_ASE_SG` 的段名不同，見 `:1041-1063`）、`[DieForceDiameter_<d>.000mm]` 兩鍵（`INSTALL_DOUBLE_EP>0`）
  - **Ind 段**：`[SLK Type]` 的每個口徑 `[Diameter_<d>.000mm_0]`～`[Diameter_<d>.000mm_15]`（`:1167-1179`）；MULTI（`INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI`）是 `[Diameter_<d>.000mm_Arm1_1..4]`／`[..._Arm2_1..4]` 共 8 段（`:1146-1164`）。每段 `LoadRate=1.0000`、`ContactOffset=0.0000`。
  - ⇒ 實際新增的是「`[SLK Type]` 有、`[SLK Type Ind]` 沒有」的口徑；`[SLK Type Ind]` 那幾個口徑 906 載入器本來就會補。
- **沒檔** → 建構子 `:643-644` 跑 `WriteFile()`（`:1183-1376`）建整份檔，另外寫 Gerneral.ini EP 8 鍵（見 §5）。
- **CC_KYEC_LEE** 且 `[SLK Type] Type` 以 30 或 60 開頭 → ReadFile `:1112-1143` 把 `[SLK Type]` 改寫成 `28,40,58,56`／`1,1,1,1` 並 `WriteFile()`。R15＝B 之後這件事**開機就發生**（原本要開頁）。

**例（Steven01，客戶 791）**：`[SLK Type]` 有 30,40,60,56,80、`[SLK Type Ind]` 只有 20,30 ⇒ 開機補 40／56／60／80 各 16 段＝**64 段、128 把鍵**，其他段不缺鍵。
依據：唯讀模擬腳本 `D:\HT9045\.claude\skills\ht9045-contact-force\scripts\predict_contactinfo_boot_keys.py`（只讀 D:\HT9045\system\ContactInfo.ini，不寫任何檔；假設客戶碼 791、EP_Install=3、INSTALL_DOUBLE_EP=1，別台要改檔頭假設），20260927 14:2x 輸出 `missing keys: 128 in 64 sections`、`{'40.000mm': 16, '56.000mm': 16, '60.000mm': 16, '80.000mm': 16}`、非 Ind 段缺鍵 0。當時檔案 mtime 仍是 2026-07-28——`725038a6` 之後還沒 build、還沒開機（Steven：「暫時先不要 build」），第一次開機後才會真的長出來。

- 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\realfile_guard.py:51` 已把這個檔列在 TARGETS。`check` 應該報「多了幾個鍵」（新段附加在檔尾），不應該是 `CHANGED`。

### 4.2 `D:\HT9045\system\Gerneral.ini`（`--dry` 會重導）

- 有檔那一支：ReadFile 補 `[System]` EP 12 鍵（`EP_Install!=0`，`:983-1012`）與 `[Test Arm] dIndexZOffset[0..1][0..14]` 30 鍵（`:1014-1036`）的缺鍵。之後的 `LoadContactForceTables()` 本來就補同一批，不是新增。
- 沒檔那一支：WriteFile 直接寫 EP 8 鍵（§5 的怪處）。

### 4.3 連帶效果（推論，未實跑）

`bUseDynamicKitDiameter` 開著、沒有 ContactInfo.ini 的機台：
- `725038a6` 之前：開機後 `ContactForceTables().SLKClass.bLoaded==false`（載入器沒檔就提早 return，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp:247-249`），Setup.Contact 存檔被 `ContactForceInputsMissing()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp:199-205`）拒絕，要先開一次 Setup.ContactForce。
- 之後：建構子在載入器之前先建好檔，載入器讀到檔、`bLoaded=true`，Setup.Contact 一開機就能存。

## 5. golden 怪處 R72：沒有 ContactInfo.ini 的機台，開機把 `EP_*_1032` 寫成 0（照 golden 保留）

**事實**
- V912 建構子 `:572-574` 用全域 `EP_MAXKPA_1032`／`EP_MAXAFB_1032`／`EP_MINMPA_1032` 填三格畫面。golden 全樹只有 ReadFile `:994-996` 讀這三個全域（grep V912 `*.cpp`／`*.h`），沒檔時 ReadFile 要等 WriteFile 尾端 `:1375` 才跑 ⇒ 填進去的是 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cmydef.cpp:3063-3067` 的初值 **0.0**。
- WriteFile `:1357-1365` 把畫面值寫進 `D:\HT9045\system\Gerneral.ini [System]`：

| 鍵 | 寫入值 | 畫面值從哪來 |
|---|---|---|
| `EP_MAXKPA`／`EP_MAXA`／`EP_MINMPA` | 建構子 `:542-544` 剛讀到的值（數值不變） | `:568-570` |
| `EP_MINA_FeedBack` | `0.968` | DFM `edtMinMpaFB.Text`（建構子沒填） |
| `EP_MAXKPA_1032`／`EP_MAXA_1032`／`EP_MINMPA_1032` | **`0`**（原本有值也蓋掉） | 全域初值 |
| `EP_MINA_FeedBack_1032` | `0.968` | DFM `edtMinMpaFB_1032.Text` |

**例**：新機台 Gerneral.ini 原本 `EP_MAXKPA_1032=499`、`EP_MINA_FeedBack_1032=0.908`，第一次開機後變成 `0`、`0.968`。Steven01 這台有 ContactInfo.ini，不會碰到。

**裁決**：BCB6 機台也是這樣。`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` R72，St01 建議 A 照 golden，已照做、可推翻（說明在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp:413-416` 的註解）。

**同一條路的第二個怪處（存取違規）**
- WriteFile `:1372` 寫 `[Test Arm]` 30 鍵時 `IndexZOffsetEdit[i][j]` 還是 NULL（檔案層級全域 `:19`，只有 ReadFile `:972-980` 指派）⇒ golden 在這裡存取違規。
- 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.gen.inc:1440` 加了守衛：不寫那 30 鍵（Gerneral.ini 原值不動）、記 todo。
- 推論（沒在 BCB6 驗）：golden 的例外發生在 CreateForm 裡，被 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HT9045.cpp:147-298` 的 try/catch 接住（記 boot log；VCL Exception 另 ShowException），接著 `:299` `return 0` 結束程式、不會進 `Application->Run()`。ContactInfo.ini 與 EP 8 鍵在例外之前已寫好，所以第二次開機走有檔那一支就正常。

## 6. 過期註解（只記錄，不要照著判斷）

| 位置（移植樹） | 註解寫的 | HEAD 實際 |
|---|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp:237-246` | 「檔案不存在時 … ContactInfo.ini 不會被建立」 | 載入器本身仍不建檔，但開機時建構子（`wb_serve.cpp:4062`）先建好了 ⇒ 載入器讀到的都是有檔 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4074-4076` | 「移植樹沒有那個表單，所以載入時機要由整合者決定」 | 表單讀寫已翻（S57）、建構子開機就跑（R15＝B）。Jimmy 的區段，只記錄 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4090-4093`（同一段 P2a 註解） | 「只讀不寫 … 這一行不會動到 system\ContactInfo.ini（… 實測 same）」 | 載入器這一行仍不 WriteFile，但同一條開機鏈的 `:4062` 會寫這個檔；realfile_guard 不會再是 `same` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4062` 同一行的 S57 註解 | 「只建替身 … 不讀不寫檔 —— golden 建構子本體延到第一次 editlist.get」 | 同一行後面的 S162 註解已更正（`725038a6`） |

## 7. 既有裁決（`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md` R15～R18，另加 S90）

- **R15＝B**（Steven 20260927，S162）：建構子開機就跑。已做 `725038a6`。
- **R16**（Ind 表型號清單跟 906 還是 912）：decisions-decided 記「St01 建議 B（底層改跟 912 `[SLK Type]`），已照做」，**但程式不是這樣**。HEAD 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp:164-194` 仍讀 `[SLK Type Ind]`，這支檔 `21d37f2b` 之後沒再改；S57 交件報告也寫「現況：機台用 906 的 [SLK Type Ind]…建議 B」。⇒ 程式現況是 A（底層 906、畫面 912，RULINGS 第 26 條字面）。要動之前先請 ST01-M 確認以哪個為準；照 B 做的話，只改 `ContactForceLoad.cpp:164-169` 的 CSV 來源一處。
- **R17＝A**（已照做）：必送清單嚴格，頁面漏送回 400、不存（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp:162-194` `RebuildMustSend`）。
- **R18＝A**（已照做）：`TTrackBar` 的 SetMin 守衛照推論（面板 trackbar 建好是 Min=0、Position=10），等 Jimmy 在 BCB6 核對。
- **S90**：EP 四鍵（HSys 也寫）「本頁沒改的欄位不把舊值蓋回」，`f1ad780c`（SKILL.md 20260926 那一段已記）。

## 8. 開機後怎麼確認（wb_serve stdout）

依序應該看到（字串照 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp:441`、`:204-207`、`:447-454` 與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4097-4102`）：

```
FileRW ContactForce: TfContactForce proxies ready (N save reads before panels)
FileRW ContactForce: golden TfContactForce ctor (ContactForce.cpp:421) ran at boot (golden CreateForm HT9045.cpp:220) -- bUseDynamicKitDiameter=1 bHasFile=1 SLK=5 Ind=80 DieForce=5 DieForce1by1=0 (mustSend N)
FileRW ContactForce: boot ctor ran ReadFile on (missing keys seeded) system\ContactInfo.ini (file existed); ...
ContactForce tables loaded: N entries (SLK=5 Ind=32 DieForce=5 DieForce1by1=0)
```

- 數字是 Steven01 的推算值（未實跑）。
- `ran at` 後面不是 `boot` ＝ 開機沒跑到。
- 出現 `ran WriteFile on` ＝ 沒檔建檔，或 KYEC 30→28／60→58 轉換；看同一行印出的 session todo。
