> 保存來源：`.claude/skills/ht9045-motionview-html-ui/references/parameter-contract.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 參數契約（Parameter Contract）

模板 `assets/motionview-template.html` 的參數區（`參數區 BEGIN` ~ `參數區 END` 之間）
是**唯一**需要換機台時修改的地方。以下每個參數都標明來源檔與推導規則。

> ## ⚠ 參數來源是**機台本機的資料夾**，不是 StateRecord
>
> | 資料夾 | 檔案 |
> |---|---|
> | `D:\HT9045\system\` | `Gerneral.ini`、`teach.ini` |
> | `D:\HT9045\IniData\Data\<Device>\` | `Tray.Data`、`HotPlate.Data`、`TestMode.Data`、`ArmCondition.Data` |
>
> 每台機都有這兩個資料夾，**不需要別人的 StateRecord**。
> StateRecord 只是剛好包了一份副本，所以拖它進去參數也會對接；
> 但它真正的用途是 LIVE 模式（`MainFormSnapshot.txt` / `Motor.xls` / `HP*_*.xls`）——
> 見 [live-mode.md](live-mode.md)。
>
> 頁面 runtime 自己 parse 這些檔（`parseIni` / `parseBiff2`），
> `applyParams()` 是唯一入口，`applyTeachGeometry()` 連幾何都從 `teach.ini` 推。
> **不要把值產生出來貼進 HTML** —— 那是踩過的架構錯誤。

## 目錄

- [1. 直接讀檔就有的參數](parameter-contract.md#1-直接讀檔就有的參數)
- [2. 必須推導的參數](parameter-contract.md#2-必須推導的參數)
- [3. 機台空間座標（teach → mm）](parameter-contract.md#3-機台空間座標teach--mm)
- [4. 選配資料](parameter-contract.md#4-選配資料)
- [5. 抽參數指令](parameter-contract.md#5-抽參數指令)
- [6. 換機台檢查清單](parameter-contract.md#6-換機台檢查清單)

---

## 1. 直接讀檔就有的參數

全部 `.ini` / `.Data` / `.csv` 一律 **cp950** 讀取。專案強制 Big5，勿存成 UTF-8。

| JS 變數 | 欄位 | 來源檔（StateRecord 相對路徑） |
|---|---|---|
| `MACHINE.model` / `.id` | `Model` / `Machine ID` | `HT9045\system\Gerneral.ini [Version]` |
| `MACHINE.device` | 目錄名 | `HT9045\IniData\Data\<DeviceName>\` |
| `MACHINE.cc` | `CUSTOMER_CODE` | `DecisionVariables.csv` |
| `TRAY.xp/.yp/.cols/.rows` | `X Pitch` / `Y Pitch` / `X Division` / `Y Division` | `IniData\Data\<Dev>\Tray.Data [Type0]` |
| `HPF.xp/.yp/.cols/.rows` | 同上 | `IniData\Data\<Dev>\HotPlate.Data [Hotplate Form]` |
| `ARM.rows` / `.cols` | `iPickRow` / `iPickCol` | `MainFormSnapshot.txt`（runtime 實況，優先） |
| — | `USE_PICKER_COUNT` | `Gerneral.ini [System]`　`0=ep4Picker 1=ep8 2=ep2 4=ep1` |
| `PIT.minX3` / `.maxX3` | `IN_OUT_ARM_X_PITCH_MIN` / `_MAX` | `Gerneral.ini [System]` |
| `YP.min` / `.max` | `IN_OUT_ARM_Y_PITCH_MIN` / `_MAX` | `Gerneral.ini [System]` |
| `YP.installed` | `USE_IN_OUT_ARM_Y_PITCH != 0` | `Gerneral.ini [System]`　**0 = Y 變距軸未安裝** |
| `PIT.t40` / `.t120` | `setEditInXPitch40` / `120` | `system\teach.ini [MInArmPitch]` |
| `PMODE.inArm` / `.outArm` | `One by one` | `IniData\Data\<Dev>\ArmCondition.Data [Input Arm]` / `[Output Arm]` |
| — | `Temperature Mode`（1=Hot） | `IniData\Data\<Dev>\TestMode.Data [TestMode]` |
| — | `Dut Aa`…（=1 的即啟用 site） | `TestMode.Data [DutOnOff]` |

> `PMODE` 對應 `rgInArmPitch->ItemIndex`（`cSpeed.cpp:1867` 註解 `0:Open/Close, 1:Fixed`），
> 旗標是 `ArmSpeed_File[InArm].bVariModeFIX`（`cSpeed.cpp:497` 讀 / `:1582` 寫）。
>
> - **`0` = Open/Close**：每次下針可重算間距、**中間允許空格**，一次能吸多支。
> - **`1` = Fixed**：ini key 名稱就是 **`One by one`** —— **一次只吸一顆**。
>   `bVariModeFIX==true` → `bCanPick2ICAtOnceTime=false` →
>   `GetInArmToLoaderPosition_Single()` 把其餘吸嘴（**含另一排**）全部關掉。
>   詳見 [placement-algorithms.md §1.4](placement-algorithms.md)。

---

## 2. 必須推導的參數

### 2.1 `PIT.min1` / `PIT.max1`（單一間距上下限）

`cmydef.cpp` / `database.cpp` 在 `USE_IN_OUT_ARM_X_PITCH == iXPitch40mm` 時固定：

```
iXpitchMin = 1333   iXpitchMinX2 = 2666   iXpitchMinX3 = 4000
iXpitchMax = 4000   iXpitchMaxX2 = 8000   iXpitchMaxX3 = 12000
iPitch_Max_minus_Min = iXpitchMaxX3 - iXpitchMinX3 = 8000
```

單位是 **0.01 mm**。`X3` 是「四支吸嘴的總跨距（3 個間距）」，非 X3 的是單一間距。
所以 `min1 = minX3/3`、`max1 = maxX3/3`（→ 13.33 ~ 40.00 mm）。
`iXPitch50mm` 模式改成 `max=5000 / maxX3=15000`；`iXPitch16Pick` 另一組值，見 `database.cpp`。

### 2.2 變距軸 pulse 換算

公式取自 `ainarm9045.cpp` `GetInArmPitchX_9045()`：

```
m     = (iInArmX120Pitch - iInArmX40Pitch) / iPitch_Max_minus_Min
pulse = iInArmX40Pitch + m * (跨距(0.01mm) - iXpitchMinX3)
```

模板的 `pitchPulse(gapMM)` 就是這條式子。OutArm 用 `MOutArmPitch` 的教點同式。

### 2.3 合法倍數 k / m（可跳幾欄／幾列）

```
X：k 使 round(TrayXPitch*100*k) 落在 [min1, max1]
Y：m 使 round(TrayYPitch*100*m) 落在 [YP.min, YP.max]（需 Y 變距軸）
```

- **Fixed 模式**：只取第一個合法 k（＝程式碼 `iInArmXStep` 由 k=1 往上 first-fit 的行為）
- **Open/Close 模式**：整個合法集合都可用，每次下針各自挑

### 2.4 Site 節距與 `iYHalf`（列步進）

**權威來源是碼**：`ainarm_SearchPlacePlate.cpp` `GetHotPlateYHalfPos()` 第 167 行

```c
iYHalf = TestIF.iARM_HP_Y_PITCH / HotPlateForm.YPitch;
```

`iARM_HP_Y_PITCH` 是吸嘴兩排在 HotPlate 上的 Y 間距（無 Y 變距時＝機構固定值）。
若抓不到該值，可由 **HotPlate 逐格表反推**（`extract_params.py` 用這招）：

- **欄步進**：同一個 Site 連續佔幾欄 → 通常 2（＝`spacX`）
- **列步進**：統計「同一欄裡 A 排 site + d = B 排 site」出現最多次的 d → 即 `iYHalf`
  - A 排 = Site 1~`ARM.cols`；B 排 = Site `ARM.cols+1`~
  - ⚠ **不可**只取「第一個 A 列」與「第一個 B 列」之差 —— 盤上會有混排的列，會算錯

推得：`SITE.xp = HPF.xp * 欄步進`、`SITE.yp = HPF.yp * iYHalf`、`ARM.yGapMM = SITE.yp`

### 2.5 Tray 端能不能兩排同時取

```
ARM.yGapMM / TRAY.yp 是整數 → 兩排可同時取（一次 2*cols 顆）
                    非整數 → Tray 端一次只能下一排（cols 顆）
```

無 Y 變距軸時 `ARM.yGapMM` 固定，這個判斷就決定了上料節拍。
碼上的佐證：`GetInArmToLoaderPosition()` 的 `iYPosition = iRow` 是**純量**（只有一個列），
且 `IniConfig.bE46_LoaderUse2Offset` 讓前後排在 Loader 各有獨立 Offset。

---

## 3. 機台空間座標（teach → mm）

模板的 `G{}` 是機台佈局座標（mm）。換機台時用該機的 `teach.ini` 重算：

| 座標 | 換算 | 說明 |
|---|---|---|
| 尺度 | **1 pulse ≈ 0.01 mm**（SMC 軸） | 由 Shuttle 行程與 InArm/OutArm 教點差交叉驗證得出 |
| InArm X | `x_mm = pulse/100 + 偏移` | 偏移可自訂，習慣把 Loader 取料位定為 300 mm |
| InArm Y / OutArm Y | `y_mm = pulse/100` | Y 軸 home 在最後側，前為負 |
| Shuttle 站 A | `MInArmX` 的 Shuttle 教點 | InArm↔Shuttle 交料位 |
| Shuttle 站 B | 站 A + 行程 | 行程 = `iRight - iLeft`；Index/Socket 位置 |
| Shuttle 站 C | 站 B + 行程 | OutArm↔Shuttle 交料位 |
| Socket Y | Index 教點反推 | `SHT1_Y + (ToSocketY - ToSHT1Y)/100`，應等於 SHT2 側反推值 |

**必做的四項交叉驗證**（任一項差 > 1 mm 就是尺度或偏移錯了）：

1. SHT1 的 Y：InArm 教點 vs OutArm 教點
2. SHT2 的 Y：InArm 教點 vs OutArm 教點
3. Socket 的 Y：Index1 反推 vs Index2 反推
4. 料盤軌的 Y：Loader（InArm）vs Auto1-3（OutArm）

> 已知殘差：`MTrayX` 推出的 Loader→Auto1 距離會比軸教點少約 40 mm，
> 因為 TrayX 是「夾盤基準」、另兩者是「吸料基準」，差一個固定夾持偏置
> （另含 GearRatio 0.235 vs 0.2363 的 0.55%）。不要為了對齊它去改前四項。

---

## 4. 選配資料

| JS 變數 | 來源 | 用途 |
|---|---|---|
| `HP_SITE[][]` | `HP2_Site.xls`（或 `HP1_`） | HotPlate 逐格 Site，可當「實機重播」起始狀態 |
| `HP_SHT[][]` | `HP2_WhichShuttle.xls` | 逐格「預定送哪一條 Shuttle」 |
| `PICKREC[]` | `system\PickHPRec.json` | 實機取料群組記錄（含孤兒單吸嘴群與上下排分拆群） |

⚠ 檔名 `HP1_` / `HP2_` 與 `P0` / `P1` **不是同一套編號**：實測 `HP2_*.xls` 才是
`DecisionVariables.csv` 裡 `HP RealIC P0` 那一片。抽出後一定要跟 `DecisionVariables` 的顆數對帳。

⚠ `PickHPRec.json` 是**記錄**不是未來計畫，內含重複 pattern；重播時只執行來源格確實有料的 team。

`HP*_*.xls` 是 BIFF 二進位，需 `xlrd>=2.0`（`xlrd 2.x` 只支援 .xls，正好）。

---

## 5. 抽參數指令

```bat
python scripts\extract_params.py "<StateRecord 目錄>" --report    :: 人可讀核對表
python scripts\extract_params.py "<StateRecord 目錄>"             :: 核對表 + JS 片段
python scripts\extract_params.py "<StateRecord 目錄>" --json      :: 給其他工具
```

把 JS 片段貼進模板的參數區，覆蓋原有的 `MACHINE / TRAY / HPF / ARM / PIT / YP / SITE / PMODE`
與（若有）`HP_SITE / HP_SHT / PICKREC`。`G{}` 空間座標仍需依 §3 手動重算。

---

## 6. 換機台檢查清單

- [ ] `--report` 的四項推導值合理（k 集合非空、iYHalf ≥ 1、Site 節距為 HP pitch 整數倍）
- [ ] `USE_PICKER_COUNT` 與 `iPickRow × iPickCol` 相符（`1 = ep8Picker` → 2×4）
- [ ] `ARM.yGapMM / TRAY.yp` 是否整數 → 決定 Tray 端能否一次取滿
- [ ] §3 的四項座標交叉驗證都 < 1 mm
- [ ] `HP_SITE` 顆數與 `DecisionVariables.csv` 的 `HP RealIC` 一致
- [ ] 跑 `assets/smoke-test.js`，全部不變量通過（見 `verification.md`）

<!-- preserved-content:end -->
