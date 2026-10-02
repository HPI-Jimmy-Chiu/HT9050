---
name: ht9045-tray-group-mechanism
description: HT9045 料盤升降機構（Tray Group GoUp/GoDown）通用知識庫。Loader、Color、Empty 三組料盤站的升降機構原理完全相同，僅機台定義名稱不同。當使用者詢問 Loader GoUp、Loader GoDown、Color GoUp、Color GoDown、Empty GoUp、Empty GoDown、料盤升降、Tray Group 升降原理、C_Color_Up、C_Empty_Up、C_Load_Up、C_LoaderUpPress、CylinderUp、CylinderMiddle、CylinderLower、DoLoadNewColorTrayToCar、DoLoadNewEmptyTrayToCar、DoSupplyNewICTray、asendic_Loader、asendic_Color、asendic_Empty、SnLoaderUpSafedetect、SnEmptyUpSafedetect、SnColorUpSafedetect、料盤站機構差異、三組機構對比、*_Z_USE_MOTOR（[TrayZ] 料盤 Z 用馬達還是氣缸）、HT9050 的 Loader／Empty／Auto Z（MLoaderZ／MEmptyZ／MAuto1Z～3Z、分盤、Tray Arm 交接） 等相關問題時，應先載入此技能。關鍵字：Tray Group, GoUp, GoDown, Loader, Color, Empty, C_Color_Up, C_Empty_Up, C_Load_Up, CylinderUp, CylinderMiddle, CylinderLower, asendic_Loader, asendic_Color, asendic_Empty, 料盤升降, 升降機構, LOAD_Z_USE_MOTOR, TrayZ_Up, TrayZ_Mid, MLoaderZ, MEmptyZ, HT9050, Type_HT9050, 分盤, C_MobileTrayTableSelect。
---

# HT9045 Tray Group GoUp/GoDown Mechanism

## 核心知識

**Loader、Color、Empty 三組料盤站的升降機構結構完全相同，僅名稱不同。**

三者共用：
- **同一個 `TMyCylinder` 類別**（定義於 `mycylin.h`）
- **相同的三階位置控制**：`CylinderUp()` → `CylinderMiddle()` → `CylinderLower()`
- **相同的狀態機流程**（`switch(Task)` 結構，case 編號對應一致）
- **相同的安全感測邏輯**（UpSafedetect sensor）

差異僅在於：變數名稱、氣缸/感測器常數 ID、JAM 報警碼、檔案名稱。

## 原始檔位置

| 機構 | 主程式 | 關鍵函式 |
|------|--------|----------|
| Loader | `asendic_Loader.cpp` | `DoSupplyNewICTray()` |
| Color | `asendic_Color.cpp` | `DoLoadNewColorTrayToCar()` |
| Empty | `asendic_Empty.cpp` | `DoLoadNewEmptyTrayToCar()` |

> 三個檔案結構一致，include 相同模組（MyMotor, mycylin, myswitch, cmydef 等）。

## 共用狀態機流程

以下是三組機構共用的升降 GoUp/GoDown 狀態機骨架：

```
case 1:   初始化 → 檢查是否已有 Tray
case 20:  Middle 汽缸防護（timeout 保護）
case 50:  ASE Report / TrayID 讀取前處理
case 60:  ★ CylinderUp(C_XXX_Up) → 料盤升到最高位
case 100: Z_Select 汽缸推出（分離爪）
case 200: ★ CylinderMiddle(C_XXX_Up) → 中層停止（含 pause 檢查）
case 300: Z_Select 汽缸復位
case 400: 延遲等待（5 秒）
case 410: ★ CylinderLower(C_XXX_Up) → 下降到最低位
case 420: 感測器到位驗證（SnXXXCCWDete / CarHasTray sensor），失敗報 JAM
```

其中 `C_XXX_Up` 依機構替換為 `C_Color_Up` / `C_Empty_Up` / `C_Load_Up`。

## 命名對照表

詳見 [references/naming-map.md](references/naming-map.md)，包含：
- 氣缸常數對照
- 感測器常數對照
- 函式名稱對照
- JAM 報警碼對照

## 實務應用

1. **修改一組升降邏輯時**，務必確認另外兩組是否需同步修改
2. **新增 Tray 站（如 Auto4~6 升降）**，可直接複製任一組狀態機，替換名稱常數即可
3. **除錯時**，若 Color 升降異常，可參照 Empty/Loader 的正常行為做比對
4. **Loader 特殊性**：Loader 使用額外的 `C_LoaderUpPress`（ID=59）氣缸做壓盤，Color/Empty 無此機構

## 料盤 Z 改用馬達：`[TrayZ] *_Z_USE_MOTOR`（golden 906，20261001 補）

上面的三階氣缸是 `*_Z_USE_MOTOR=0` 的那條路。`Gerneral.ini` `[TrayZ]` 九個鍵讀進 `LOAD_Z_USE_MOTOR[0..8]`（0＝Loader、1＝Empty、2＝Color、3～8＝Auto1～6；缺鍵預設 false，golden `database.cpp:760-768`），對到 `iTrayZMotor[]`＝`{MLoaderZ, MEmptyZ, MColorZ, MAuto1Z…MAuto6Z}`（`cmydef.cpp:2656`）。

| 開關 | `CylinderUp` | `CylinderMiddle` | `CylinderLower` |
|---|---|---|---|
| 0（氣缸） | `C_XXX_Up` 加中段氣缸 On，等 on-sensor，逾時報 Lifter Up error | `C_XXX_Up` On、中段 Off | 兩支都 Off |
| 1（馬達） | `MotorMove(Prod.TrayZ_Up[軌])`（`asendic.cpp:177-182`） | `MotorMove(Prod.TrayZ_Mid[軌])`（`:312-318`） | `MotorMove(0)`（`:444-452`） |

- 上表是 Loader／Empty／Color（`CylinderUp`／`Middle`／`Lower`，`iMot=iTrayZMotor[Part]`）；Auto 疊走 `AutoCylinderUp`／`Middle`／`Lower`（`asendic.cpp:578`／`:792`／`:996` 看 `LOAD_Z_USE_MOTOR[iAuto]`），馬達取 `iAutoZMot[]`。
- 馬達模式只是用**每軌三個固定點**（上／中／0）取代三段氣缸；`TrayZ_Up`／`TrayZ_Mid` 是 `[MAX_TRACK]` 陣列，**按軌道排**，開機由 `Tech.iTrayLoaderZ[軌]` 算（Up＝教導值＋200，Mid＝教導值－ZDepth＋separate 偏移；`cinitial.cpp:10266-10267`，只到 Auto3；Auto4～6 用 `Tech.iTrayAuto4Z[]`，`:10270-10275`）。
- HOME 要不要回原點**只看 `Mot_Table` 的 Enable**，不看這個開關；開關只管 HOME 畫面顯不顯示該軸（`uhome.cpp:166-171`）、以及要不要復歸 `C_Load_Up` 這類氣缸（`uhome.cpp:1297-1305`、`:730-735`）。
- ⚠ 軸 `Enable=0` 時 `MotorMove` 會直接把 Position 設成目標並回成功（真機建置，`#ifndef SOFT_SIMULTE`；`Motor/mymotor.cpp:565-566`、`:821-823`）：開關設 1 但軸沒啟用 ⇒ 流程照走、Z 沒動、不報警。

## HT9050 不一樣（20261001，RULINGS_20261001 第 22 條）

- **機構**（Frank 20261001）：「`MLoaderZ` 這個是 Loader 的馬達」「`MEmptyZ` 這個是 Empty 的馬達」；Auto1～3 的 Z「目前是有汽缸以及馬達雙複合的機構」；「目前 Loader Empty Auto 1~3 的分盤是 Z 軸馬達，但在與 Tray Arm 交接的地方會觸發汽缸的動作」。RULINGS_20260930 第 2、3 條（「Z軸是氣缸」、M35／M36／M38～M40 `Enable=0`）**對 Loader／Empty 不再成立、對 Auto1～3 只對一半**；`machines/HT9050/Mot_Table.csv` 的 Enable 等 ES02 上機確認後才改（TO_ES02 E-03），確認前不在機台上打開這幾軸跑 HOME。佐證：硬體表 M35／M36／M38～M40（5 軸）是步進 EEDO-06-80U、EtherCAT ring 0 有 5 台 SW3D-680、這 5 軸有煞車輸出（硬體表煞車欄卻填 N）——詳見 `ht9050-hw/references/motors-9050.md` §5、`v906/frank-handoff` 的 `FROM_FRANK.md` §3（20261001 14:38 那列 ①～④）。
- **910 的 `_9050` 函式（Eastsun 0818～0820，F-01b 照它翻）不走上面那張表**：以 `MachineTypeChoice==Type_HT9050` 分流（例：`DoLoad()` 一開頭就轉 `DoLoad_9050()`），**這些 `_9050` 函式不看 `*_Z_USE_MOTOR`**（⚠ 但 golden 的 `DoAutoEmpty` 在 9050 上照跑——910 `csystem.cpp:10480-10494` 沒有機型守衛——它經 `CylinderUp(C_Empty_Up)` 看 `LOAD_Z_USE_MOTOR[1]`，所以 `[TrayZ]` 在 9050 上不是完全沒作用；同一個 Empty 區兩種模型），直接對 `MLoaderZ`／`MEmptyZ`／`MAuto1Z`～`MAuto3Z` 下馬達命令，而且是**逐層模型**：`TrayZ_Up[層]`、`TrayZ_Down[層]`、`TrayZ_Home`、`EmptyZ_*`、`Auto1～3Z_*`、層數探測、`i*CatchExtraLift_9050`。函式：`DoLoadNewICTray_9050`／`DoLoad_9050`（`asendic_Loader.cpp`）、`DoCatchFromLoader_9050`／`DoPlaceTrayToEmpty_9050`／`DoPlaceTrayToAuto_9050`／`DoPlaceToBuffer_9050`（`acatchtray.cpp`）、`DoReceiveAllToBottom_9050`（`csystem.cpp`）。
- **交接時的氣缸**：910 的 9050 流程在 Tray Arm 交接動的是 `C_MobileTrayTableSelect`——**取盤**（`DoCatchFromLoader_9050`）：Push → Z 抬 ExtraLift → Off（料盤歸 `MTrayX`）→ Pop；**放盤**（`DoPlaceTrayToEmpty_9050`／`DoPlaceTrayToAuto_9050`）：Z 抬 ExtraLift → On（`MTrayX` ClearTray）→ Push → Z 降 → Pop——以及各區 `EdgeClip`／`EdgePush`、分離氣缸 `C_TrayZ_Selector`／`C_*LoaderZ_Select`。每一疊另有「托盤升降」氣缸 `C_Load_Up`／`C_Empty_Up`／`C_Auto1～3_Up`（HT9050 IO 表 Enable=1、有實際 DO），**910 的 9050 流程一次都沒驅動它們**；它們跟 Z 步進各管什麼，待 EastSun 確認。
- ⚠ **同名陣列兩種意義**：golden 的 `TrayZ_Up[]` 是「軌」，910 的 9050 拿它當「層」——而且它只有 `MAX_TRACK`＝9 格，9050 用到第 19 層、起始 −1 ⇒ 越界（Empty／Auto 的 9050 陣列 `[20]` 第一次放盤也讀到 `[-1]`）。F-01b 要不要修待 Jimmy 裁。
- ⚠ 9050 新增的教導欄位（`TrayZ_Home`、`EmptyZ_Up[]`…）在 910 **沒有任何一處設值**，恆為 0。
- 移植樹現況與更多細節：`HT9011UC_Cpp_V3.33.906.0/docs/FLOW9050_PORT_LEDGER.md`（§0 第 2 點；第 9 點「機構」在 MR !50 `v906/frank-flow9050-ledger-2`，合入後才在 main）、`v906/frank-handoff` 的 `docs/handoff/FROM_FRANK.md` §3、910 原檔在 orphan 分支 `ref/frank-910-9050`。
