> 保存來源：`.claude/skills/ht9045-json-bridge/references/unit-convert-layer.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# `cUnitConvert.h`：`_File` → 執行中 的轉換層（全部）

> 量測日 20260923。BCB 對照樹 `HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（`cUnitConvert.h` 10 行、
> `cUnitConvert.cpp` 272 行）；移植樹 `HT9011UC_Cpp_V3.33.906.0/cUnitConvert.cpp`。
>
> 使用者 20260923：「`TestIF_File` 跟 `TestIF` 其實是類似的東西，只是部分參數會經過型態轉換，
> 可以在 BCB 的 `cUnitConvert.h` 找到；`DeviceForm` 跟 `HotPlateForm` 都有類似的動作。」
>
> 這份修正 `SKILL.md` §3.3 原本寫的「執行中那份從未被整包指派」—— 那是錯的，grep `TestIF = TestIF_File`
> 漏掉了 `memcpy`。**整包指派就在這裡，而且是 golden 唯一的套用點。**

---

## 一、它在做什麼

每個 `_File` 結構是「檔案裡的值」，單位是**畫面單位**（mm、秒）；對應的執行中結構是「機台在用的值」，
單位是**馬達單位**（0.01 mm 的 int）。轉換層做三件事：

1. `memcpy(&Live.第一欄, &File.第一欄, sizeof(File))` —— **先整包複製**，所以沒被逐欄轉換的欄位兩邊完全相同。
2. 對特定欄位 `Live.x = iUnitMultiply100(File.x)` —— double mm → int（×100，經 `FormatFloat("0.00")` 消 IEEE 浮點雜訊再 `atoi`）。
3. 少數欄位帶**選擇邏輯**（AutoClean 走 Tray 還是 Kit、HotPlate 26.67 修成 26.66、`iPickUp` 越界回 65）。

```cpp
int iUnitMultiply100(double Data)          // :13
{
    asString=FormatFloat("0.00", Data*100.00);   // 0.03*100 = 2.9999… → "3.00"
    return atoi(asString.c_str());               // → 3（直接 (int) 會得到 2）
}
int iUnitMultiply1000(double Data) { return Data*1000.0; }   // :22，沒有消雜訊
```

---

## 二、七個函式逐一

### `DoTestIFConvert()`（`:27-60`）`TestIF_File` → `TestIF`

```cpp
memcpy(&TestIF.iTestMode, &TestIF_File.iTestMode, sizeof(TestIF_File));   // 整包
// ×100 → int 的欄位（10 個）
dSiteXPitch  dSiteYPitch  dSiteYOffset  dShiftXPitch  dSiteXCenterPitch
dMulti2DXPitch  dMulti2DSH1Ofs_L  dMulti2DSH1Ofs_R  dMulti2DSH2Ofs_L  dMulti2DSH2Ofs_R
// 選擇邏輯：AutoClean 六個欄位從 _Tray 或 _Kit 那組挑
if(TestIF.bAutoClean_UseTray && !IniConfig.bE43AutoCleanUseHotplate)
    TestIF.dAutoClean_{XPitch,YPitch,XStart,YStart} / iAutoClean_{X,Y}Division  = TestIF_File.*_Tray
else                                                                              = TestIF_File.*_Kit
```

⚠ 注意 `TestIF_File` 裡 **`_Tray` 與 `_Kit` 各一組（12 欄）**，`TestIF` 裡只有一組（6 欄）。所以 `TestIF` 的欄位集合**不是** `TestIF_File` 的子集——JSON 要開 `testIF.live` 時，`FieldDesc` 不能直接沿用 `testIF.file` 的那張。

### `DoDeviceConvert()`（`:62-75`）`DeviceForm_File` → `DeviceForm`

```cpp
memcpy(&DeviceForm.IndexArmPick[0], &DeviceForm_File.IndexArmPick[0], sizeof(DeviceForm_File));
for i in 0..1:  IndexArmPick[i] IndexPlace[i] IndexDrop[i] IndexContact[i] IndexUp[i]   ×100
XDimension  YDimension                                                                  ×100
```

### `DoHotPlateConvert()`（`:77-91`）`HotPlateForm_File` → `HotPlateForm`

```cpp
memcpy(&HotPlateForm.XPitch, &HotPlateForm_File.XPitch, sizeof(TRAY_TYPE_PARA));
XPitch = (File.XPitch==26.67) ? iUnitMultiply100(26.66) : iUnitMultiply100(File.XPitch);   // 26.67×3=8001 的修正
YPitch  XStart  YStart                                                                        ×100
```

⚠ `26.67 → 26.66` 是**執行中才改**，檔案保留 26.67。JSON 若顯示 `hotPlate.live.XPitch` 會是 2666，`hotPlate.file.XPitch` 是 26.67 —— 兩者都對，是不同實例。

### `DoArmOffsetConvert()`（`:93-201`）`*ArmOffSet_File[i]` → `*ArmOffSet[i]`、`Offset_File` → `Offset`

- `InArmOffSet[i]`（`i<InOfsTotal`）、`OutArmOffSet[i]`（`<OutOfsTotal`）、`SortArmOffSet[i]`（`<SortOfsTotal`，僅 `USE_OUT_SORT_ARM!=eartUninstall`）三組同型：
  `SetOneByOne(GetOneByOne())`（不轉）＋ `X Y PickUp Place Variable VariableY Variable2 Variable3 Variable4` ×100
  ＋ `SingleOffSet->dPosOffSetX/Y[j][k]`、`dPickUpOffSet[j][k]`、`dPlaceOffSet[j][k]` 逐格 ×100（`j<iMotRow, k<iMotCol`，In/Out 用 `InArmSuck`，Sort 用 `OutArm2Suck`）。
- `Offset`：`iTrayArmX[i]`、`iTrayArmX_ART[i]`（`i<MAX_TRACK`）、`iIndexArmPickUp/Place/Contact[i]`、`iSHHalft/iSHRightPod/iSHLeftPod/iSHLeft2D[i]`（`i<2`）、`iPreciserOpen`、`iPreciserClose`、`dTrayZseparate[i]` 全部 ×100。
- **沒有 memcpy**：這一段是逐欄，`Offset` 裡沒被列到的欄位**不會**從 `Offset_File` 過來。
- **來源也是讀寫檔的一員**（使用者 20260923）：`*ArmOffSet_File[i]` 與 `Offset_File` 由 `TfOffSet::ReadFile()`（`cOffSet.cpp:1998`）從 `Position Offset.Data` 讀進來、`TfOffSet::SaveFile()`（`:1406`）寫回去，區段是 `CapStrInput[i]`、目標是 `->SetXxx()`。細節見 `file-io-mechanisms.md` §F。⚠ 這三個檔（`cOffSet.cpp`／`InOutArmZteach.cpp`／`ArmOffsetData.cpp`）**在移植樹不存在**，所以移植樹的 `DoArmOffsetConvert()` 即使被叫到，來源結構是建構子初值。

### `DoArmSpeedConvert()`（`:203-216`）`ArmSpeed_File[i]` → `ArmSpeed[i]`、`SHSpeed_File` → `SHSpeed`、`MGSpeed_File` → `MGSpeed`

```cpp
for i<SpeedPartTotal: memcpy(ArmSpeed[i] ← ArmSpeed_File[i], sizeof(ARM_CONDITION)); dRetryDown ×100
memcpy(SHSpeed ← SHSpeed_File, sizeof(SHUTTLE_SPEED));     // 沒有欄位轉換
memcpy(MGSpeed ← MGSpeed_File, sizeof(MAGAZINE_SPEED));    // 沒有欄位轉換
fShowMessage->ShowSpeed(IniConfig.bG05ShowSpeedMessage);   // ⚠ 副作用：碰 VCL 表單
```

### `DoDefFormConvert()`（`:218-252`）`UserDefForm_File[i]` → `UserDefForm[i]`（`i<4`）

```cpp
memcpy(UserDefForm[i] ← UserDefForm_File[i], sizeof(TRAY_TYPE_PARA));
XPitch YPitch XStart YStart ZDepth  iPickUp  BlockXStart BlockYStart BlockPitchX BlockPitchY BlockTraySize   ×100
// ⚠ 鉗制寫回 _File：
if(IniConfig.bC03UseCatchTray) { if(File.iPickUp<60 || >73)  File.iPickUp=65; }
else                           { if(File.iPickUp<30 || >110) File.iPickUp=65; }
if(USE_LdUldCassetteMode==1) UserDefForm[0].dCassetteZPitch/dCassetteZStart ×100
```

⚠ 這是七個裡**唯一會改 `_File`** 的：`iPickUp` 越界時把 `_File` 改成 65，之後存檔就是 65。橋接層的 `Clamp*` 要把這條搬進 `userDefForm.file`。

### `DoStructUnitConvert()`（`:254-271`）總指揮

```cpp
DoTestIFConvert(); DoDeviceConvert(); DoHotPlateConvert();
DoArmOffsetConvert(); DoArmSpeedConvert(); DoDefFormConvert();
if(Temperature.bATCActiveCooling && LastSet.iTemperature==Tempture_Ambient &&
   (ATC_InterfaceForm->IS_ATC33() || iATC_MODE_TYPE==ATC_TYPE_61) &&
   Temperature.bActiveHeatGun && Temperature.dATC_HotGunTime!=0)
    ArmSpeed[OutArm].dWaitOnSH += Temperature.dATC_HotGunTime;     // ⚠ 累加，不是賦值
```

⚠ 最後那條是**累加**：每叫一次 `DoStructUnitConvert()` 就再加一次 `dATC_HotGunTime`。golden 一次 Save 流程只叫一次所以沒事；橋接層 `Reload` 若多叫，`dWaitOnSH` 會漂。`Reload` 必須確保只叫一次。

---

## 三、golden 什麼時候叫它（V912，24 個呼叫點）

| 位置 | 情境 |
|---|---|
| `cinitial.cpp:13509` | **開機**讀完全部檔之後 |
| `main.cpp:28313-28508`（10 處）、`:25755`、`:28694`、`:29673` | 各設定表單 **Save／關閉**之後、換配方之後 |
| `Command.cpp:7844 / 8511 / 8994` | 遠端指令（GPIB／SECS）改設定之後 |
| `AutoClean/AutoClean.cpp:637`（只叫 `DoTestIFConvert`）、`:5189` | AutoClean 切 Tray／Kit 之後 |
| `AutoAlignment/SmartSetup.cpp:1692`、`Automation/auto9045.cpp:503`、`Automation/mainAT.cpp:347`、`ProductionInfo/ProductionInfo.cpp:2331` | 各自動化流程改完設定之後 |

規律：**「檔案 → `_File` 結構」之後、「機台開始用」之前**，一定叫一次。它就是 ①（`ReadFile`）與執行之間的那一步。

---

## 四、移植樹現況（20260923）

| 項目 | 狀態 | 位置 |
|---|---|---|
| `iUnitMultiply100`／`1000` | 已翻譯，含浮點雜訊註解 | `cUnitConvert.cpp:42` |
| `DoTestIFConvert`／`DoDeviceConvert`／`DoHotPlateConvert`／`DoDefFormConvert`／`DoStructUnitConvert` | **本體已翻譯** | `cUnitConvert.cpp:370 / 413 / 442 / 630 / 675` |
| `cUnitConvert.cpp` 檔頭 | ⚠ **過時**，仍寫 `SKIPPED: DoTestIFConvert …`；以本體為準 | `:1-20` |
| 開機呼叫點 `cinitial.cpp` | ⚠ **`#if 0` 擋住**（`:7175`，註 `N3-G8: blocked by DoStructUnitConvert()`） | `cinitial.cpp:7175-7176` |
| `Automation/auto9045.cpp` | ⚠ `#define DoStructUnitConvert W5FA_DoStructUnitConvert` → **空 stub** | `:152-153` |
| `ckernel_shims.cpp` | ⚠ `#define DoStructUnitConvert W7L2_DoStructUnitConvert` → **stub** | `:113` |
| `AutoClean/AutoClean.cpp` | `:207` 有 `static void DoStructUnitConvert() {}` stub；`:3717` 直接叫 `DoTestIFConvert()`；`:8329` 叫 stub | — |
| `Command.cpp` | `:8659 / 9168 / 15839` 直接叫（要確認連到本體還是 shim） | — |

**結論**：轉換層已翻好，但**開機不跑、大部分呼叫點被 `#define` 換成 stub**。這是「JerryYang 讀進 `TestIF_File` 的值到不了 `TestIF`」最可能的直接原因，比「沒有 JSON」更上游。橋接層 P0 之前要先把這件事釐清：`wb_serve` 開機序列有沒有叫真的 `DoStructUnitConvert()`。

---

## 五、對橋接層的意義

1. **`*.live` 綁定不需要另一張 `FieldDesc`，也不需要另外讀檔**：`live = Convert(file)`。`ToJson(testIF.live)` 的正確做法是**呼叫 golden 的 `DoTestIFConvert()` 之後讀 `TestIF`**，不是自己算 ×100。
   例外：`TestIF` 的 AutoClean 六欄在 `_File` 是 12 欄（`_Tray`／`_Kit`），`FieldDesc` 要分開兩張，或 `live` 那張用「投影」把 12→6。
2. **`struct.put` 之後、`Reload` 之內，要叫一次且只叫一次 `DoStructUnitConvert()`**——這是 golden 每次 Save 後都做的事，不叫等於「檔案改了、機台沒變」；叫兩次 `dWaitOnSH` 會累加。這條**取代** `SKILL.md` 原本「`struct.put` 不套用到執行中那份」的說法：golden 有套用點，照 golden 叫。
3. **單位（使用者 20260923 定案）**：三層——HTML 用 mm；機台從檔案讀進來也是 mm（`_File`）；給馬達時才是 0.01 mm（執行中結構）。JSON 一律送前兩層那個 mm 值；×100 是馬達層用值時的事，不進線上。`hotPlate.live.XPitch=2666` 這種只給 Motion View 等需要「機台在用的值」的頁面看，且標成唯讀。
   ⇒ 本檔 §四「開機不叫 `DoStructUnitConvert()`」**不是橋接層的門檻**（P0 讀 `_File`→JSON 不需要它），是引擎側能不能拿到第三層值的問題，歸 JerryYang 翻譯排程；橋接層 `Reload` 對它的態度是「解閘了就照 golden 叫一次，沒解閘就標 `liveNotApplied`」。
4. **`Clamp*` 來源多一處**：除了各表單 `SaveSetupFile` 與 `GetLevelSet`，`DoDefFormConvert` 的 `iPickUp→65` 也是鉗制，且它寫回 `_File`。
5. **副作用要拆**：`DoArmSpeedConvert` 末尾 `fShowMessage->ShowSpeed(...)` 碰 VCL 表單；`DoStructUnitConvert` 末尾的 `ArmSpeed[OutArm].dWaitOnSH +=` 是累加。橋接層在無介面 `wb_serve` 裡叫它們時，前者要有 no-op shim，後者要保證單次。
6. 表⑧的「已出 JSON 3 個欄位」裡 `TestIF_File.dSiteXPitch/dSiteYPitch` 正好是這層 ×100 的欄位——JerryYang 20260922 信裡量到的 `X Pitch=40.000` 是 `_File`（mm）；機台實際用的 `TestIF.dSiteXPitch` 應為 4000。驗收 G2 要兩邊都量。

<!-- preserved-content:end -->
