---
name: ht9045-catchtray-flow
description: HT9045 IC Test Handler CatchTray（Tray Arm）流程知識庫。當使用者詢問 DoCatchTray、DoCatchFromLoader、CatchNewTrayFromBuffer、DoPlaceTrayToAuto、DoPlaceToBuffer、DoSlapTray、DoCatchUnderTray、DoSupportUnderTray、DoPlaceBufferTray、Loader/Empty/Color 補盤、Auto1/2/3 放盤、Tray Mapping、CatchTray Fix 氣缸、RFID/TrayID 相關問題時，應先載入此技能以理解 CatchTray 完整狀態機流程。關鍵字：DoCatchTray, CatchTrayTask, iCatchFromLoaderTask, iCatchNewTrayFromBufferTask, iPlaceTrayToAutoTask, iPlaceToBufferTask, iSlapTrayTask, DoCatchUnderTray, DoSupportUnderTray, DoPlaceBufferTray, C_CatchTray_Fix_Puch, C_CatchTray_Fix_Pop。另含 Auto 出料盤「收料 vs 退盤」區分（§7）：DoReceiveAutoTray（clean-out/tray-feed 批次退盤到 Car，iReceiveAutoTrayTask，DoTrayFeed case 200 呼叫）vs DoAutoReceiveBinTray（生產中出料盤收料+滿盤換盤，TrayMoveOut/TrayMoveIn 雙向）；JAM1101/JAM1201/JAM1301 兩種來源（退盤 arrival time up vs DoAutoReceiveBinTray case 1 的 Car 軟硬不一致 fHasTray&&Sen.IsOff）。另含 DoLoad() 進料限制地圖（LoadTask 狀態機、呼叫層/入口/case 1→300→600→1000/800 三層閘門、SupplyNewIC_From_LoaderCar、進料前置 gate 插入點）見 references/DoLoad_LoaderSupply_Map.md。另含分盤機構（§9）：通用機型 Loader/Empty/Color 分張＝兩段式舉升 C_Load_Up/C_Load_Middle（CylinderUp/Middle/Lower 三高度、汽缸或馬達 LOAD_Z_USE_MOTOR）＋ 分離爪 C_TrayZ_Selector（Push 咬上疊/Pop 放）；Double-Belt 四角爪 SeparateRL/FL/RR/FR 為 DOUBLE_BELT_MODE 特例非通用。關鍵字：分盤, Tray Separate, C_Load_Up, C_Load_Middle, C_TrayZ_Selector, Selector, 舉升, CylinderUp, CylinderMiddle, CylinderLower, LOAD_Z_USE_MOTOR, DOUBLE_BELT_MODE。
---

<!-- AI(W906-BA-SKILL) 20260915：正文換成網頁同事 20260915 那版（較新／較完整）。
     frontmatter 的路由描述保留我們的。
     我們原有但他沒有的段落，另存 references/ours-kept-20260915.md。 -->


# HT9045 CatchTray Flow Knowledge

## 適用場景

當使用者詢問以下主題時，載入此技能：
- CatchTray / TrayArm 的動作流程、狀態機、case 數值意義
- Loader 吸空 Tray 流程（DoCatchFromLoader）
- Empty/Color/Auto2 補盤流程（CatchNewTrayFromBuffer）
- Auto1/Auto2/Auto3 放盤流程（DoPlaceTrayToAuto）
- Buffer 放盤流程（DoPlaceToBuffer / DoPlaceBufferTray）
- 下方 Conveyor 模式（DoCatchUnderTray / DoSupportUnderTray）
- 拍盤（DoSlapTray）與 CatchTray Fix 氣缸 push/pop
- Tray Mapping / RFID 讀取相關流程

## 專案資訊

- **專案**: HT9045 IC Test Handler
- **原始碼根路徑（可自定義）**: `${HT9045_SOURCE_ROOT}`
- **預設值（範例）**: `d:\HT9045\HT9011UC_Code_V3.33.897.0_20260306\`
- **語言**: C++ (Borland C++ Builder 6, VCL framework, AnsiString)
- **架構**: State Machine pattern — `switch(Task)` 搭配 `int &Task` 參考變數
## 參考文件

- [DoCatchTray_ProcessFlow.md](references/DoCatchTray_ProcessFlow.md) — CatchTray 完整狀態機 Case 說明
- [DoLoad_LoaderSupply_Map.md](references/DoLoad_LoaderSupply_Map.md) — **DoLoad() 進料限制地圖**：呼叫層/入口/狀態機三層閘門、case 1→300→600→1000/800 供料決策、進料前置 gate 插入點（908.2 行號）。查「什麼條件才進料 / 要下退盤未完成不進料的閘門」時看此檔

## 關鍵原始檔

| 檔案 | 主要函式 | Task 變數 |
|---|---|---|
| `acatchtray.cpp` | `DoCatchTray()` — CatchTray 主狀態機 | `CatchTrayTask` |
| `acatchtray.cpp` | `DoCatchFromLoader()` — 從 Loader 吸空 Tray | `iCatchFromLoaderTask` |
| `acatchtray.cpp` | `CatchNewTrayFromBuffer()` — 從 Empty/Color/Auto2 補盤 | `iCatchNewTrayFromBufferTask` |
| `acatchtray.cpp` | `DoPlaceTrayToAuto()` — 放 Tray 到 Auto | `iPlaceTrayToAutoTask` |
| `acatchtray.cpp` | `DoPlaceToBuffer()` — 放 Tray 到 Buffer | `iPlaceToBufferTask` |
| `acatchtray.cpp` | `DoSlapTray()` — 拍盤流程 | `iSlapTrayTask` |
| `acatchtray.cpp` | `DoCatchUnderTray()` — 下方夾盤流程 | `iCatchUnderTrayTask` |
| `acatchtray.cpp` | `DoSupportUnderTray()` — 下方補盤流程 | `iSupportUnderTrayTask` |
| `acatchtray.cpp` | `DoPlaceBufferTray()` — Buffer 放盤流程 | `iPlaceBufferTrayTask` |
| `acatchtray.cpp` | `C_CatchTray_Fix_Puch()` — Fix Push | `iCatchTray_Fix_Puch` |
| `acatchtray.cpp` | `C_CatchTray_Fix_Pop()` — Fix Pop | `iCatchTray_Fix_Pop` |
| `acatchtray.cpp` | `DoLoadCarRotArmReadRFID()` — LoaderCar RFID 讀取 | `iCoverTrayIDTask[iKeyenceCoverTrayID_LoaderCar]` |

---

## 1. 呼叫階層總覽

```text
DoCatchTray()                               [acatchtray.cpp]
  |- DoCatchFromLoader()                    [switch Task]
  |   |- C_CatchTray_Fix_Pop()              [switch iTask]
  |   `- C_CatchTray_Fix_Puch()             [switch iTask]
  |
  |- DoCatchUnderTray()                     [switch Task]
  |- CatchNewTrayFromBuffer()               [switch Task]
  |- DoPlaceTrayToAuto()                    [switch Task]
  |   `- DoSupportUnderTray()               [switch Task]
  |
  |- DoPlaceToBuffer()                      [switch Task]
  |   `- DoPlaceBufferTray()                [switch Task]
  |
  |- DoSlapTray()                           [switch Task]
  `- DoLoadCarRotArmReadRFID()              [switch Task]
```

---

## 2. DoCatchTray() 主流程

> Task variable: `int &Task = CatchTrayTask`

### Case 清單

`1, 10, 15, 20, 30, 40, 50, 100, 140, 145, 147, 150, 160, 170, 200, 250, 260, 300, 350, 400, 500, 600, 1000, 1100, 1150, 1200, 2000, 2100, 2120, 2130, 2140, 2160, 3000, 3010, 3100, 4000, 4100, 4200, 5000, 5100, 5150, 5200, 6000, 6010, 6030, 6100`

### 主流程段落

1. 初始化與安全檢查：`1~50`
2. Loader 吸空 Tray：`100~260`
3. 判斷哪個 Auto 需要 Tray：`300~400`
4. 從 Buffer 補新 Tray：`500/600`（呼叫 `CatchNewTrayFromBuffer`）
5. 放 Tray 到 Auto：`1000~1200`（呼叫 `DoPlaceTrayToAuto`）
6. 放 Tray 到 Buffer：`2000~2160`（呼叫 `DoPlaceToBuffer`）
7. 手動/映射/收尾：`3000~6100`

---

## 3. 下一層函式 Case 摘要

### 3.1 DoCatchFromLoader
- Task: `iCatchFromLoaderTask`
- Cases: `1, 10, 50, 90, 95, 100, 110, 120, 145, 150, 155, 156, 160, 161, 162, 163, 165, 170, 230, 240, 250, 300, 301, 305, 310, 400, 410, 500, 501, 502, 503, 550, 560, 570, 600`

### 3.2 CatchNewTrayFromBuffer
- Task: `iCatchNewTrayFromBufferTask`
- Cases: `1, 2100, 2101, 2102, 2103, 2149, 2150, 2160, 2165, 2180, 2185, 2190, 2191, 2199, 2200, 2250, 2260, 2300, 2310, 2340, 2350, 2351, 2352, 2353, 2355, 2360, 2400, 2450, 2500, 2550, 2600, 2700, 4000`

### 3.3 DoPlaceTrayToAuto
- Task: `iPlaceTrayToAutoTask`
- Cases: `1, 100, 150, 200, 210, 220, 240, 250, 260, 300, 400, 401, 500, 510, 520, 530, 540, 600, 900, 1000, 1030, 1040, 1050, 1060, 1070, 1100, 1130, 1140, 1150, 1160, 1200, 1300`

### 3.4 DoPlaceToBuffer
- Task: `iPlaceToBufferTask`
- Cases: `1, 2100, 2150, 2160, 2250, 2251, 2260, 2270, 2280, 2300, 2350, 2351, 2352, 2500, 2600, 2900, 3000`

### 3.5 DoSlapTray
- Task: `iSlapTrayTask`
- Cases: `1, 5, 10, 20, 50, 100, 200, 300, 400, 450, 500`

### 3.6 DoCatchUnderTray
- Task: `iCatchUnderTrayTask`
- Cases: `1, 50, 100, 200, 300, 400, 500, 600, 700`

### 3.7 DoSupportUnderTray
- Task: `iSupportUnderTrayTask`
- Cases: `1, 50, 100, 110, 200, 300, 400, 600`

### 3.8 DoPlaceBufferTray
- Task: `iPlaceBufferTrayTask`
- Cases: `1, 50, 100, 200, 300, 400, 600, 1000`

---

## 4. 子函式 switch（下一層內）

### 4.1 C_CatchTray_Fix_Puch
- Task: `iCatchTray_Fix_Puch`
- Cases: `1, 100`

### 4.2 C_CatchTray_Fix_Pop
- Task: `iCatchTray_Fix_Pop`
- Cases: `1, 100`

### 4.3 DoLoadCarRotArmReadRFID
- Task: `iCoverTrayIDTask[iKeyenceCoverTrayID_LoaderCar]`
- Cases: `1, 2, 3, 100, 500, 1000, 2000, 2500, 3000, 3100, 4000, 4100, 5000, 6000`
- ⛔ 20261003 V906 移植樹補（AI(W906-E034) 20261003，todo E-034＝筆電卡 S-21，St01；Steven 1003 14:5x「Q82. A」＝#20 例外，Steven 1003 常設規則）：golden 0618 `acatchtray.cpp:8403`／`:8444`／`:8512` 三處 `asTrayIDDataCorverLoader=="NOREAD";` 沒作用（讀不到／按 Skip 時 ID 停在 case 3 清的 `""`）；0625 同行號、V912 改寫過的函式 `:9088`／`:9166`／`:9245` 是 `="NOREAD"`。移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\acatchtray.cpp:8568`／`:8609`／`:8677` 已改 `=`；下游 `asendic_Loader.cpp:1836-1838` 會把 "NOREAD" 當讀取失敗處理。移植樹目前**沒有呼叫者**（golden 呼叫端 `cTrayMapping.cpp:5828` 沒移植），所以今天沒有執行期差異。測試：ctest `E034_NoopEq` [A]（離線：`MOT[MLdCarRotArm].Motor=NULL`、氣缸 Enable=false、Sen 強制、`W906_ShowErrorMessage_SimReturn`）。

---

## 使用指引

回答 CatchTray 相關問題時：

1. 先讀 `CatchTrayTask`，定位主流程段落（吸盤、補盤、放盤、收尾）。
2. 再看對應子流程 Task（`iCatchFromLoaderTask`、`iCatchNewTrayFromBufferTask`、`iPlaceTrayToAutoTask`）。
3. 若涉及夾爪異常，優先檢查 `C_CatchTray_Fix_Puch/Pop` case 與 sensor 狀態。
4. 若涉及 RFID/TrayID，再切到 `DoLoadCarRotArmReadRFID()`。

---

> **以下為 Roger 維護的額外章節（原始版本保留）**

## Local Preserved Notes (from previous local agent)

## 5. TrayX 座標排列（物理位置）

TrayArm X 軸的座標值由左到右（screen 130 → 702）遞增，對應的料盤站位順序為：

```
Loader(最小) < Empty < Color < Auto1 < Auto2 < Auto3(最大)
```

### 驗證依據

**cinitial.cpp `SetScreenScale()`**（約 line 5938）：
```cpp
MOT[MTrayX].SetScreenScale(130, 702, Prod.iXTrayLoad, Prod.iXTrayAuto[2]);
//  螢幕左邊界=130 → Loader X    螢幕右邊界=702 → Auto3 X
```

**uteach.cpp Teaching 頁順序**（line 400-408）：
```
Loader → Empty → Color → Auto1 → Auto2 → Auto3 → Auto4 → Auto5 → Auto6
```

**安全位置公式**（mymotor.cpp line 5724）：
```cpp
iSafePos = (Prod.iXTrayEmpty + Prod.iXTrayColor) / 2 + 6500;
// 位於 Empty 與 Color 之間偏向 Color 一側
```

### 全域變數對應

| 站位 | 全域座標變數 | 說明 |
|------|-------------|------|
| Loader | `Prod.iXTrayLoad` | Loader 料盤區中心 X |
| Empty | `Prod.iXTrayEmpty` | 空盤回收區中心 X |
| Color | `Prod.iXTrayColor` | 色盤（Color Tray）區中心 X |
| Auto1 | `Prod.iXTrayAuto[0]` | Auto1 出料區中心 X |
| Auto2 | `Prod.iXTrayAuto[1]` | Auto2 出料區中心 X |
| Auto3 | `Prod.iXTrayAuto[2]` | Auto3 出料區中心 X |

### 常見易錯提醒

- **`MOT[MTrayX].ReadPos() > Prod.iXTrayColor`** 為 true 時，代表 TrayArm 在 **Auto 區域**（Color 右邊），而非 Loader/Empty 區域。
- CatchTray idle 停靠在 **Empty 位置** 時（`bP39=0, bP56=0, OCR=0`），其 X 值 **小於** `Prod.iXTrayColor`。

---

## 6. CatchTray 補盤狀態判斷（State Record 診斷用）

在 OutArm 卡住時（如 case 3010 迴圈），可透過以下指標判斷 CatchTray 是否「正在補盤」：

### 6.1 關鍵指標

| 指標 | 位置 / 取得方式 | 空閒值 | 忙碌（補盤中）值 |
|------|---------------|--------|----------------|
| `CatchTrayTask` | State Record `CatchTrayTask` | `100`、`300`、`400` 循環 | `≥500`（補盤）、`≥1000`（放盤到 Auto） |
| `iCatchTrayControlManual` | State Record 或 `cmydef.cpp` | `0` | `≥2`（OutArm 已通知 CatchTray 需要補新盤） |
| `WhichAutoNeedTray()` | 由 `acatchtray.cpp` case 400 呼叫 | `0`（所有 Auto 皆有盤且未滿） | `1/2/3`（eAuto1/2/3 需要盤） |
| `MOT[MTrayX].ReadPos()` | State Record `MTrayX CMD/Enc` | 在 Empty/Color 區（≤ iXTrayColor） | 在 Auto 區域移動中（> iXTrayColor） |

### 6.2 CatchTray 空閒循環（Idle Loop）特徵

```
case 100 → case 300（判斷是否有盤 → 決定停靠位置）
         → case 400（WhichAutoNeedTray()==0 → iCatchTrayControlManual=0 → 回 case 100）
```

- 空閒循環週期很短（約 2ms），在 State Record 中會看到 `CatchTrayTask` 快速在 `100`/`300`/`400` 之間交替。
- 此時 `iCatchTrayControlManual == 0`，TrayArm 停在 Empty 或 Color 位置不動。

### 6.3 OutArm case 3010 中的 CatchTray 忙碌判斷

OutArm 在 `aoutarm9045_*.cpp case 3010` 中有兩道安全檢查防止與 TrayArm 碰撞：

```cpp
// 檢查 1：TrayArm 已「越過 Color 進入 Auto 區」時暫停（避免 X 軸碰撞）
//   ⚠ 門檻是 Color，不是 Empty —— TrayArm 停在 Empty 或 Empty~Color 之間時這道完全不擋
if (TRAY_ARM_MODE==eAboveCoveyor &&
    MOT[MTrayX].ReadPos() > Prod.iXTrayColor) {
    MoveOutArmXY_ToFix_Tray_Full();
    break;  // 暫停放料，等 TrayArm 離開
}

// 檢查 2：CatchTray 補盤流程進行中，讓位等待
if (iCatchTrayControlManual >= 2 || WhichAutoNeedTray() != 0) {
    MoveOutArmXY_ToFix_Tray_Full();
    break;  // 暫停放料，等補盤完成
}
```

> 908.5+（`IsTrayXSafeForOutArm()`，20260723）把檢查 1 換成「命令＋encoder 雙判」，
> **但沿用同一個 Color 門檻** —— 只補「命令已回 Color 以內、實體卻飄過去」這一種失效，
> 沒有縮小下面 §6.3.1 的灰帶。

#### 6.3.1 TrayArm↔OutArm 干涉區在哪，以及唯一真正的撞機失效模式

TrayX 站位由小到大固定為：`Loader < Empty < Color < Auto1 < Auto2 < Auto3`
（實例 HLY642：`-4410 / 27970 / 49640 / 78740 / 97180 / 115690`）。

> **⚠ 干涉區的正確認定（RogerYang 20260907 裁示）**
> **大部分機型，TrayArm 停在 Loader / Empty / Color 都是安全的，OutArm 撞不到；
> 只要 TrayArm 進到 Auto 區（> Color）才有風險。**
> 所以 case 3010 用 `ReadPos() > Prod.iXTrayColor` 當門檻**是正確的**，不要去改小。

**唯一真正會撞的失效模式**：

> 軟體看到的 **TrayArm 馬達命令位置已到達（＝判定已離開 Auto 區）**，
> 但**實體還在 unloader(Auto) 區**沒退出去 → OutArm 過來放料就撞上。

→ 所以正解是**同時看命令位置與 encoder**，不是動門檻。這就是 `IsTrayXSafeForOutArm()`
（20260723，**908.5+**）在做的事：safe 必須 `ReadPos() <= iXTrayColor` **且**
`ReadEncoderPos() <= iXTrayColor + 500` 同時成立。
（`+500` 相對 Color→Auto1 的 29100 counts 只佔 1.7%，門檻很保守、不會誤擋。）
**在 908.5 以前的版本，這道保護只看命令位置 → 撞機防護等於沒有；客戶端解法＝升版或 backport。**

#### 6.3.2 ⚠ 尚未修的對稱缺口：TrayArm 側只看命令位置

互鎖本來是雙邊的，但兩邊品質不對稱：

| 方向 | 守門 | 判什麼 | 狀態 |
|------|------|--------|------|
| OutArm 要動 → 看 TrayArm | case 3010 → `IsTrayXSafeForOutArm()` | 命令 **＋ encoder** | ✅ 20260723 已修（908.5+）|
| TrayArm 要動 → 看 OutArm | `AvoidOutArm()` → `IsOutArmSafe()`（`acatchtray.cpp`）| **只有 `MOT[MOutArmY].ReadPos()`，無 encoder** | ❌ 未修（至 910.0）|

`IsOutArmSafe()` 三個分支（`INSTALL_OCR` / `C_TrayCover|USE_TRAY_MAPPING` / else）都是
`iPosY >= 門檻 || CompareCommandPos(Prod.iOutArmSafeY, 2)==1`，**全部只讀命令位置**。
⇒ 「OutArm 命令說已退到安全位、實體還在 Auto 盤上方」時，TrayArm 會照樣開進 Auto 區撞上去
——**與 §6.3.1 同型的病，只是主客互換**。補法＝比照 `IsTrayXSafeForOutArm()`，
每個 OR 條件各加一個 encoder 同意項（餘裕 500）。

`AvoidOutArm(S, Task)` 本身的逾時保護是完整的：`IsOutArmSafe()==false` → 停 TrayArm → 等 10 秒
（`AvoidOutArmDelay`）→ 仍不安全就凍結 `MOT[MOutArmX/Y].fCanMove` ＋ 自動 `DoStateRecord`
＋ `WAR0630 Tray Arm Position Error` ＋ 強制 home。
**判讀技巧：EventLog 若全天 0 筆 `WAR0630`，代表 `IsOutArmSafe()` 從未回 false
＝ TrayArm 側守門一路都判「OutArm 安全」**（JSCC HLY642 20260720 即是此況）。

補充：`IsTrayArmMoveAvoidOutArmCrash()`（`ReadPos() >= iXTrayEmpty && Led[iInposLed]`）門檻
比實際干涉區**保守一站**（Empty/Color 其實安全），且在多支 mode 檔（如 `aoutarm9045_2x8_8.cpp`）
只掛在 `DoMoveOutArmXYToPlaceForAutoTeachOffset_*()`（CCD Auto Alignment 專用，
`bC02InstallCCD=0` 永不呼叫）——**它不是主保護，別拿它當干涉區的依據**。

實例：JSCC HLY642（2026-07-20），見
[ht9045-motor-control → 參考案例](../ht9045-motor-control/SKILL.md)。

> **補盤退讓位置（與干涉問題相關）**：上述兩處皆呼叫 `MoveOutArmXY_ToFix_Tray_Full()`（`bMoveY=false`），
> OutArm 退到 **X 最左、Y = `iOutArmSafeY` = Shuttle1 Y（Shuttle1 上方）**。`[E90]` 的 -150mm 額外退避
> **只在 `bMoveY=true` 生效，不套用到補盤退讓**。若 Shuttle1 上方仍會被 TrayArm Z 管掃到 → 須改程式。
> 詳見 [ht9045-outarm-flow §Fix「補盤退讓位置」](../ht9045-outarm-flow/SKILL.md) 與單位換算 [ht9045-motor-control](../ht9045-motor-control/SKILL.md)。

### 6.4 如何從 State Record 快速判斷

1. **確認 CatchTrayTask 值**：
   - `100/300/400` → CatchTray 空閒（未補盤）
   - `500~999` → CatchTray 正在取新盤
   - `1000~1300` → CatchTray 正在放盤到 Auto
   - `2000~2160` → CatchTray 正在放盤到 Buffer

2. **確認 iCatchTrayControlManual 值**：
   - `0` → 無補盤需求
   - `≥2` → OutArm 已通知需求，CatchTray 準備或正在執行

3. **確認 MTrayX 位置**：
   - 停在 Empty 附近（≈ `Prod.iXTrayEmpty`）→ 空閒停靠
   - 正在移向 Loader / Auto → 補盤流程中

---

5. 判斷 CatchTray 是否正在補盤，參見 §6 的 State Record 診斷指標。
6. TrayX 座標相關計算或碰撞檢查，參見 §5 的座標排列說明。

---

## 7. Auto Tray（出料盤）「收料 vs 退盤」：DoAutoReceiveBinTray ≠ DoReceiveAutoTray

> ⚠ 兩個名字很像、都會動到 Auto 出料盤與其下方 Car(stack)，但**是不同函式、不同觸發時機、不同檔案**。分析退盤/JAM1101 前務必分清。（實碼驗證版本 906.5 / 908.1，行號可能漂移，函式名為錨點。）

### 7.1 功能對照

| | `DoAutoReceiveBinTray(Pos)` | `DoReceiveAutoTray(Pos)` |
|---|---|---|
| 檔案 | `asendic_Auto.cpp`（函式起點約 :71） | `csystem.cpp`（函式起點約 :6755） |
| 觸發時機 | **生產中**（unload 迴圈）：出料盤在收 IC，盤滿要換 | **收工**：clean-out/tray-feed，由 `DoTrayFeed` case 200 `f[i]=DoReceiveAutoTray(i)` 呼叫 |
| 動作方向 | **雙向循環**：滿盤 `TrayMoveOut`(退到 Car) ＋ 新盤 `TrayMoveIn`(送進來，約 :2345) | **只往外/退**：把 Auto 出料盤整批退到 Car |
| Task 變數 | 生產 unload 側 | `iReceiveAutoTrayTask[Pos]`（Pos=0/1/2 = Auto1/2/3）|
| 一句話 | 生產時「逐盤填滿→退出+補新」的常態管理 | 收工時「一次把現有盤全退到 Car」 |

**易錯**：不要把 `DoAutoReceiveBinTray` 說成「只送盤進來」，也不要說成「只退盤」——它**兩個方向都做**。真正的「批次退盤」是 `DoReceiveAutoTray`。

### 7.2 DoReceiveAutoTray 退盤狀態序列（csystem.cpp）

```
case 1        檢查 Car 有無空間收退盤(Car已有盤→JAM1101 或等)
case 5~10/20  啟動退出、floodgate、(震動/edge-push)
case 30/31/32 SwAutoCCW off → AutoCylinderLower(降氣缸放盤) → 起到位計時
case 250/260  TrayMoveOut；MOT[iMMAuto_Car].InitNewTray+MoveTrayAllItem(auto→car) ← 軟體把盤資料移到 Car、標 Car 有盤
case 300/310  等 Car sensor 檢到盤(IsOff()==false)；逾時→JAM1101("DoReceiveAutoTray_310")
case 400      TrayMoveOut + 等 DetectAutoReceTime.Off() → 410
case 410      最後 TrayMoveOut(false) + 選盤/補下一盤(依 Auto2/ART/motor-Z 分支)→ 500
case 500      完成、Car ClearTray → 600(收尾) → return true
```
> **關鍵**：case 260/310 就先「軟體標 Car 有盤」，但**盤實體要到 410/500 才真正落定**。若退盤在 400 卡住、沒跑完 410/500（被 PAUSE 打斷、或 Car 頂不進），會留下**「Car 軟體=有盤、Car sensor=空」的軟硬不一致**。

### 7.3 JAM1101/JAM1201/JAM1301 兩種來源（同碼、不同條件）

`sJAM1101[Pos]`（Pos=0/1/2）是同一組 alarm code，**被多處復用**，畫面固定顯示 alarm DB 文字「Auto N tray goes inside arrival time up error!」，**但實際觸發條件不一定是字面上的 arrival timeout**：

| 觸發點 | 條件 | 真正意義 |
|---|---|---|
| `DoReceiveAutoTray` case 250/310 | `DetectAutoReceTime.Off()`（退盤到位計時逾時）| 退盤時盤沒到 Car（真 arrival timeout）|
| **`DoAutoReceiveBinTray` case 1**（asendic_Auto.cpp:149，tag `DoAutoReceiveBinTray_1`）| **`MOT[iMMAuto_Car].fHasTray && Sen[SnAutoTrayCar].IsOff()`** | **Car 軟硬不一致**：軟體以為 Car 有盤、Car sensor 讀空（**非** arrival timeout；別被訊息文字誤導）|

### 7.4 JSCC clean-out 死雞關聯（見記憶 [[jscc-cleanout-deadchicken-judgeempty]]）

事故序列印證上面機制：退盤(`DoReceiveAutoTray`)時 **Auto1/Auto2 卡在 case 400、沒跑完 410/500**（Auto3 到 500 正常）→ 留下「Car 軟體有盤/sensor 空」→ 16:26:41 PAUSE → 16:28 ONE CYCLE+START 恢復生產 → `DoAutoReceiveBinTray` case 1 一檢查 Car 就撞 **JAM1101(Auto1)/JAM1201(Auto2)**；Auto3 退乾淨無 JAM。**所以「JAM1101 在恢復生產後才出現」不是退盤當下發的，是退盤沒收乾淨留下的 Car 不一致、等生產供盤流程才被檢出。**

### 7.5 呼叫來源 ＋「兩支共用 `iMMAuto_Car`」（為何 JAM 出自生產函式、根卻在退盤）

> 常見誤解：「JAM1101 的 log tag 是 `DoAutoReceiveBinTray_1`（生產函式），怎麼會跟收工退盤(`DoReceiveAutoTray`)扯上關係？」——因為**兩支操作的是同一組 `MOT[iMMAuto_Car[Pos]]` tray 狀態，一支寫、一支檢查**。

**呼叫來源（906.5 實碼驗證）**：

| 函式 | 角色 | 呼叫點 |
|---|---|---|
| `DoReceiveAutoTray`（退盤，**寫** Car） | 收工把出料盤整批退到 Car | **`DoTrayFeed`**：csystem.cpp:7893 `DoReceiveAutoTray(0)`、7987 `f[i]=DoReceiveAutoTray(i)`；另 CatchTray 前置 acatchtray.cpp:6875/6904 |
| `DoAutoReceiveBinTray`（生產收料，**檢查** Car） | 跑料中逐盤收料/換盤 | **主生產迴圈** csystem.cpp:10218 `for(auto i) DoAutoReceiveBinTray(i)`（在 `Do_Auto_SHT2/DoTestHeadMotor/DoCatchTray/DoOutArm` 之後、受 `SystemStart && fAllMotorHome` 把關；非-AMR 走 10233）；CatchTray acatchtray.cpp:8957/9003。**（ATK-AMR 專屬 csystem.cpp:18175/18202/18230；RT asendic_Auto_RT.cpp:483；Magazine Magazine.cpp:4138 — 一般客戶不走）** |

**共用狀態關係**：
- 退盤 `DoReceiveAutoTray` case 260/310 **寫**：`MOT[iMMAuto_Car[Pos]].InitNewTray + MoveTrayAllItem(auto→car)`（csystem.cpp:7113-7114 / 7176-7177）。
- 生產 `DoAutoReceiveBinTray` case 1 **檢查/讀**：`MOT[iMMAuto_Car[Pos]].fHasTray && Sen[SnAutoTrayCar].IsOff()`（asendic_Auto.cpp:149）→ 不一致就 JAM1101。

→ **結論**：JAM 由生產函式報出 ≠ 與退盤無關。**退盤是「寫入方」、生產是「檢查方」**；退盤沒收乾淨在 `iMMAuto_Car` 留下「軟體=有盤/sensor=空」，要等**操作員按 START、主生產迴圈恢復**、下一輪 `DoAutoReceiveBinTray` 檢查到，JAM 才在**恢復生產後**浮現。這也解釋為何 JAM 不在退盤當下(16:26)發、而在恢復後(16:28)發。

> ⚠ 誠實邊界：「究竟退盤哪個 case 留下不一致」（case 260 搬移**無** car-sensor 閘門 vs case 400 沒收尾）無法從 post-home 污染的 state record 逐拍分辨，需 in-hang log；但「兩支共用 `iMMAuto_Car`、退盤寫/生產檢查」與 JAM 條件是實碼可證的。

---

## 8. DoLoad auto-clean-out 觸發條件 ＋「收工後補料」的坑（JSCC 死雞 code-confirmed 根因）

### 8.1 DoLoad auto-clean-out 只在「no any tray」才觸發（含 Loader Car）

`asendic_Loader.cpp` case 800 的 auto-clean-out 區塊（含 `DoLoad 7`）外層閘門（906.5:2697-2700）：
```cpp
if(flag ||
   (fMain->ALed1->Value==false &&        // loader 盤感測: 無盤
    MOT[MMTrayY_Car].fHasTray==false &&  // ← Loader Car(下方盤庫): 無盤
    MOT[MMTrayY].fHasTray==false))       // ← loader 位: 無盤   // 註解 "no any tray"
```
- 三者皆空（loader 位 + **Loader Car** + ALed1）才進此區塊。
- `DoLoad 7`（2911）在 2900 `else ret=K_CLEAN_OUT`：`bNoTrayAutoCleanOut==true` 自動清；`==false` 則 2894 跳 `MES0920` 問操作員(RETRY/CLEAN_OUT)。
- ⇒ **DoLoad clean-out 一觸發，代表當下 Loader Car 確實是空的、機台真的到料尾——不是誤觸。**（`bNoTrayAutoCleanOut` 由 CosFunction 依客戶碼設，多數 true。）

### 8.2 SupplyNewIC_From_LoaderCar = loader 常態補盤

loader 盤吃完 → `SupplyNewIC_From_LoaderCar`(task) 從 Loader Car 載下一盤，全天每 ~4~6 分一次（一盤的節拍）。**Car 有盤就載、繼續跑；Car 也空，才會走 §8.1 的 DoLoad clean-out。**

### 8.3 JSCC「clean-out 死雞」根因（翻案版，取代先前「判空 desync」推測）

1. 尾盤吃到底（prod log In X6Y16 / 7x17），loader + Loader Car **皆空** → `DoLoad 7` clean-out **正確**觸發。
2. clean-out finish → 退出料盤（`DoReceiveAutoTray`，正常收工；Auto1/2 退到 case 400 沒收乾淨、留 Car 軟硬不一致）。
3. operator 在 PAUSE 期間**手動補一盤進 Loader Car**（16:25:47 Car 空、16:28:52 SupplyNewIC 又載到盤，中間只有人補料）。
4. ONE CYCLE + START（**非完整 Initial Start，未重建/供出料盤**）。
5. SupplyNewIC 載入新盤 → InArm 取料 `MES0101`(A/E/G pick-up error) → 出料盤已退、Car 不一致(`JAM1101/1201`) → 新料無處放 → OutArm 卡 case 3010 → HOME 倒料(≈15)。

→ **不是誤觸 clean-out、也不是判空 desync；坑在「機台正確收工(含退出料盤)之後，operator 補料 + ONE CYCLE 硬接，機台沒重建出料盤就把新料載進來跑」。**

### 8.4 防範方向

| 層級 | 作法 | 備註 |
|------|------|------|
| **操作 SOP（最直接、免改碼）** | end-of-lot clean-out 後若要再跑料，做**完整 Initial Start**（會重建/供出料盤＋清計數），**不要用 ONE CYCLE+START 硬接**；若真沒料就別補盤 | 立即可行；本案就是 ONE CYCLE 接料釀成 |
| **機台守衛（程式，可選）** | clean-out finish 後立「lot-ended」旗標；此態下 `SupplyNewIC_From_LoaderCar` / InArm 從新盤取料**之前**，先確認**出料 auto 盤已就位**，否則 alarm「請先 Initial Start 重建出料盤」 | 擋在「載新料前」，比 OutArm 卡 3010（已太晚）早攔 |
| **告警收緊** | 此態下 `MES0101`/`JAM1101`/`JAM1201` 的 SKIP 選項收緊或改強制處理 | 本案 operator 一路 SKIP 就靜默 strand |

⚠ **caveat**：實作守衛前要先確認「收工後補料再跑」是否為支援情境；守衛不可擋到正常 Initial Start 與正常補盤。要 100% 坐實「operator 補料」時序，仍建議重現時擷 `SnLoaderCarHasTray`/`MMTrayY_Car.fHasTray` 與操作按鍵時間軸。
---

## 9. 分盤機構（緩衝站 Tray Separate：兩段舉升 + Selector 分離爪）

> 通用 9045/9011UC 機型 **Loader / Empty / Color 從一疊料盤分出「最底單張」** 的機構。實碼驗證版本 908.5。
> ⚠ 別把 **Double-Belt Mode 的四角分離爪**（`SeparateRL/FL/RR/FR`、走 `DoCatchUnderTray`）當通用機構——那是**非通用特例模式**，`DOUBLE_BELT_MODE==1` 才成立。查通用分盤請看本節，勿抓 `Separate` 字串就跳進四角爪那條。

### 9.1 硬體兩件套

| 元件 | 符號（每軌一組） | 作用 |
|---|---|---|
| **兩段式舉升（Lifter）** | `C_Load_Up` + `C_Load_Middle`（Empty=`C_Empty_Up/Middle`、Color=`C_Color_Up/Middle`；陣列 `iC_Up[MAX_TRACK]`/`iC_Middle[MAX_TRACK]`，cmydef.h:348-353） | 兩顆氣缸疊出**三個高度**，把整疊頂上去／降到盤間／降到底 |
| **分離爪（Separate / Selector）** | `C_TrayZ_Selector`（Load2=`C_Tray2Z_Selector`、Auto=`C_Auto1/2/3_Selector`，cmydef.h:326） | `.Push()`＝爪伸出插進盤間咬住上疊；`.Pop()`＝縮回釋放 |

### 9.2 舉升三高度（汽缸 or 馬達雙版本）

三顆函式於 `asendic.cpp`（傳入 `C_Load_Up`，內部自動配對 `C_Load_Middle`）：

| 函式 | 高度 | 雙氣缸組合 | 馬達版（`LOAD_Z_USE_MOTOR[Part]`）|
|---|---|---|---|
| `CylinderUp(C_Load_Up)` | 最高（頂整疊、卸掉爪上重量） | Up On + Middle On | `MOT[iTrayZMotor[Part]].MotorMove(Prod.TrayZ_Up[Part])` |
| `CylinderMiddle(C_Load_Up)` | 中位（爪面落在第 1、2 盤之間） | Up On + Middle Off | `…MotorMove(Prod.TrayZ_Mid[Part])` |
| `CylinderLower(C_Load_Up)` | 最低（底盤落到車上） | 兩顆 Off | `…MotorMove(0)` |

- 版本切換就在每顆函式開頭 `if(LOAD_Z_USE_MOTOR[Part]) {走馬達} else {走雙氣缸}`（asendic.cpp:177 / 312 / 444）。
- `Part`：0=Loader、1=Empty、2=Color（`CylinderUp` 依 `CylinderName` 判定）。

### 9.3 分盤序列（以 Loader 入料側為例，asendic_Loader.cpp）

```
CylinderUp(C_Load_Up)            舉升頂整疊上去、卸掉爪上重量
Cylinder[C_TrayZ_Selector].Push()  分離爪伸出，插進盤間        (:1714)
CylinderMiddle(C_Load_Up)        舉升降到中位，讓上疊坐到爪上
── 安全閘：舉升須還撐著才准後續動作 ──
Cylinder[C_TrayZ_Selector].Pop() / 接續動作 接住上疊、放底盤   (:1787)
CylinderLower(C_Load_Up)         舉升降到底，底盤單獨落到車上
```

一句話：**舉升頂整疊 → 爪插盤間 → 降中位交棒給爪 → 舉升到底、底盤單獨落車**。整疊重量在「舉升 ↔ 爪」之間 hand-over，全程靠中位這一段完成單張分離。

### 9.4 Auto 側略有不同

Auto 出料盤走 `AutoCylinderUp/Middle/Lower(Part, C_AutoX_Up, C_AutoX_Selector)`（asendic.cpp:562/767/937），把 **Selector 當中位鎖** 一起帶進舉升函式，與 Loader/Empty/Color 的「Up+Middle 兩氣缸、Selector 獨立」略有差異；分析 Auto 補盤/退盤時以這組 helper 為錨點。

### 9.5 常見陷阱

- grep 到 `Separate` 就跳進 **Double-Belt 四角爪 + `DoCatchUnderTray`**：那是 `DOUBLE_BELT_MODE==1` 特例，**非通用**。通用分盤看 §9.1-9.3。
- 找機構前先去 `cmydef.h` / IO 定義核對真實符號（`C_Load_Up`/`C_Load_Middle`/`C_TrayZ_Selector`）再比較，勿憑字串假設路徑（見記憶 [[catchtray-separate-lifter-selector]]、[[regression-hunt-diff-all-not-assumed-path]]）。

---

## 10. 「Loader 空盤放哪一軌 / Auto 空盤從哪一軌來」的三個判斷點（P04 陷阱）⚠

分析任何「放不掉盤 / 取不到盤 / auto 不去抓空盤」之前，先確認這條路由，**它由 recipe + 一個 Config 旗標共同決定，而且碼裡有三個獨立判斷點、其中一個漏認旗標**。

### 10.1 路由來源

| 來源 | 鍵 | 意義 |
|---|---|---|
| recipe `Tray.Data` `[Flag]` | `Loader Type` | 0=same（用 `LoaderToEmptyColor`）／1=different（用 `AutoFromEmptyColor[mode][auto]`）|
| recipe `Tray.Data` `[Loader]` | `ToBuffer` / `ToBuffer_RT` | Loader 空盤放到哪：0=**Empty**、1=**Color** |
| recipe `Tray.Data` `[Auto1/2/3]` | `FromBuffer` / `FromBuffer_RT` | Auto 空盤從哪來：0=**Empty**、1=**Color**、2=**Auto2** |
| `config.ini` | **`bP04ColorIsEmptyUnloader`** | 1＝**Color 軌改當「收空盤的 unloader」**（供盤段整段短路）|

### 10.2 三個判斷點（P04 只有兩處認得）

| 判斷點（908.7）| 認 P04？ | 決定什麼 |
|---|---|---|
| `IsPlaceToColor()` `acatchtray.cpp:8045` | ✅ `return true` | `DoCatchTray` case 2100 走 Color 分支、收盤觸發設 `iReceiveColorTray` |
| `DoPlaceToBuffer` case 2100 的 `pos` `:5223` | ✅ `pos=iXTrayColor` | 手臂實體移到哪一軌 |
| **`CatchTraySetItemData()` `:8335-8388`** | ❌ **完全沒有 P04 分支** | 設 `CatchTraySuck.Item[0][0]`（1=Empty／2=Color／3=Auto1），**而 case 2260 的 ready-guard 就是吃這個值**（`:5294-5296`）|

→ **P04=1 且 `[Loader] ToBuffer=0` 時：動作在 Color 軌、ready 檢查在 Empty 軌**，`MMEmpty.fHasTray` 結構性永久 true，`iPlaceToBufferTask` 永遠卡 2260（每 300 s 只記一行 `DoPlaceToBuffer Time Out 2260`、不報警）。

### 10.3 連帶：Auto 也取不到盤

`DoAutoColor()`（`asendic_Color.cpp:848`）在 P04=1 時**供盤段直接 `return`** → `MMColor.fHasTray` 永遠 false。若 recipe 仍是 `[Auto1/2/3] FromBuffer=1`（從 Color 取），則 `CatchNewTrayFromBuffer` case 2200（`acatchtray.cpp:2925-2934`）走
```cpp
if(MOT[MMColor].fHasTray==false) { Task=2100; return 2; }
```
→ `2100→2149→2199→2200→2100` 每約 10 ms 一圈**永久空轉、零提示**，Auto1/2/3 永遠沒盤，下游 `OutArmTask=3010`。
`USE_COLOR_TRAY_SENSOR=0` 時 `MMColor.fHasTray` 無 sensor 可校正 → 現場看到「Color 實體有盤、畫面顯示沒盤」。

### 10.4 自洽 vs 必死組合

| P04 | `[Loader] ToBuffer` | `[Auto1/2/3] FromBuffer` | 結果 |
|---|---|---|---|
| 0 | 0（Empty）| 1（Color）| ✅ 常規組合（Empty 收、Color 供）|
| 1 | 1（Color）| 0（Empty）| ✅ 自洽（`SetItemData` 自然給 2，三處剛好對齊）|
| **1** | **0（Empty）** | **1（Color）** | ❌ **雙靜默死結**，見 staterecord Pattern #15 |

> 判斷順序：**先看 `config.ini` 的 `bP04ColorIsEmptyUnloader`，再看 recipe `Tray.Data`**，最後才看 sensor / 機構。
> 現場鐵證：EventLog `ChangeLog` 的 `Tray_bP04ColorIsEmptyUnloader change Value 0==>1`。
> 案例：2026-07-31 偉測 HHT-477（V3.33.908.2，908.2/908.7 逐行相同，升版無效）。
> AI(ht9045-staterecord-analysis) 20260731 (RogerYang)


---

## 11. Color / Empty 軌「退盤序列」不可中途 Init ⚠（JSCC `CC_SCC` 專屬防護的回歸）

> 案例：2026-09-14 JSCC 江陰 JLD653（HT-9045WA，`CUSTOMER_CODE=943`，V3.33.908.16）
> 客訴「Color 軌道反轉、Auto1 第一次換盤 TrayArm 夾不到盤」。
> **根因是 20260225 為了修 20260214「ColorTray 亂跑」而加的那道防護本身**（`RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中`）。
> 見 staterecord [Pattern #29](../ht9045-staterecord-analysis/references/deadlock-patterns.md)。

### 11.1 退盤（把 Color/Empty 待用新盤收回料疊）是**四段交棒**，帳只在最後一段清

| 段 | 函式 / case | 動作 | 帳本變化 |
|---|---|---|---|
| ① | `DoColorTrayToFront()` case 100 | `Cylinder[C_Color_Fix].Off()` + `TrayMoveOut(true,2)`：盤由後方取料位往前送到 car | case 300：`MMColor_Car.SetTray()` + `MMColor.ClearTray()` → **回 true** |
| ② | `DoAutoColorReceive()` case 100 | 收到 ① 的 true | **`iReceiveColorTray=2`** ← 「退盤已開跑」的旗標 |
| ③ | `DoUnLoadNewColorToStack()` | 分離汽缸 + `C_Color_Up` 升降，把盤壓回料疊 | case 400：`MMColorZ.ClearTray()`；**case 500（延時＋`CylinderLower`）才 return true** |
| ④ | `DoAutoColorReceive()` case 200 → 300 | 收到 ③ 的 true | **`MMColor_Car.ClearTray()`** → `iReceiveColorTray=0` |

> **`MMColor_Car` 的帳只有第 ④ 段會清。** 序列在 ③ 完成前被 `InitAutoColorReceiveTask()` 打回 case 1 ⇒
> ④ 永遠不執行 ⇒ **`MMColor_Car.fHasTray` 永久殘留（車上實體已空、帳說有盤）**，
> 而 `iUnLoadNewColorTrayTask` 也永遠擱淺在 500（沒人再呼叫它；`InitAutoColorReceiveTask()` 管不到它）。
> Empty 軌 (`asendic_Empty.cpp`) 結構完全對稱，同一組病。

### 11.2 一被打斷就是永久壞掉（四方互鎖，零 alarm）

```
帳本殘留 MMColor_Car.fHasTray=true
   └─► csystem.cpp DoTrayFeed 每輪重新點火 iReceiveColorTray=1
          └─► DoAutoColor() 第一段就 return（iReceiveColorTray!=0）
                 └─► Color「補盤」狀態機永遠不執行 → MMColor 永遠沒盤
                        └─► CatchNewTrayFromBuffer case 2200 每輪回 2 → bIsCatchingFromBuffer 反覆點起
                               └─► 防護每輪把退盤 Init 回 case 1
                                      └─► DoColorTrayToFront case 1→100 每 ~15 ms 重下
                                          Cylinder[C_Color_Fix].Off() + TrayMoveOut(true,2)
                                          ＝ 皮帶持續往外 + 固定鉤鬆開 ⇒ 現場看到「軌道反轉、夾不到盤」
```

三個讓它「安靜」的細節：

- `DoColorTrayToFront` case 1 每次重進都 `hColorTrayToFront.SetSecAndOn(20)` ⇒ **JAM1412 永遠不可達**
- Home 不清 `iReceiveColorTray`（`uhome.cpp` 只清 `bIsPlacingToBuffer` / `bIsCatchingFromBuffer`）
- RESET 也不清 —— 只有 `IniConfig.bO05ResetNeedRemoveAllTray==1` 時 `TfMain::Reset()` 才會 `MMColor_Car.ClearTray()`

⇒ **現場止血：重開程式；或暫時勾 [O05] → 按 RESET → 再取消勾選。**

### 11.3 所有 `InitAutoXxxReceiveTask()` 呼叫點的保護（為何只有 JSCC 中招）

| 呼叫點 | 是否擋「退盤進行中」 |
|---|---|
| `acatchtray.cpp` DoCatchTray case 2100（×2） | `if(iReceiveColorTray!=2)` ✔（`Steven 20131025 : 三片Tray會夾斷`）|
| `asendic_Color.cpp` DoAutoColor case 100 | `if(iReceiveColorTray==0)` ✔ |
| `csystem.cpp` DoTrayFeed else 分支 | 需 `iReceiveColorTray==0`（或 LoadNewColorTrayToCar 非 idle）△ |
| `acatchtray.cpp` DoCatchTray case 2120 | 無，但需 `bP25` / `iNeedManualRemoved` △ |
| **`asendic_Color.cpp:840` / `asendic_Empty.cpp:642`（20260225 防護）** | **完全沒有** ✖ |

**`iReceiveColorTray==2` 就是「不要打斷我」的既有約定**；20260225 那道防護跳過了它，而且它寫死 `CUSTOMER_CODE==CC_SCC`，所以**其他客戶那行永遠 false、碰不到**。
存在版本：**894.25 沒有、896.3 起全部都有**（含 908.x / 912.x）。

### 11.4 觸發情境：Clean Out 之後 Auto1 的第一次換盤（race，非必然）

- Clean Out → Tray Feed 會把 Color 軌待用新盤退回料疊（`DoTrayFeed` case 200 的 flag3 區段）
- 同一個 Clean Out 也讓 Auto1/2/3 清空、TrayArm 立刻要去 buffer 抓新盤
- Auto1 `Tray.Data [Auto1] FromBuffer=1` ⇒ **只能從 Color 拿**（見 §10.1 路由）
- **競態窗口 ≈ 6~8 秒**（② 的 `iReceiveColorTray=2` 到 ③ 回 true）。命中機率高但不是 100%，**命中後永不自癒**

### 11.5 現場 30 秒辨識

| 證據 | 值 |
|---|---|
| `Task.xls`（＝`StringGrid2` 快照）`UnLoadNewColorTrayTask` | **500**（擱淺在最後一步）|
| 同表 `AutoColorReceiveTask` / `ColorTrayToFrontTask` | 1 / 100（15 ms 振盪）|
| 同表 `fColorCanSupplyNewTray`（`[38,3]`）| `-1`＝true ⇒ 排除 `fColorCanSupplyNewTray==false` 早退，證明卡在 `iReceiveColorTray!=0` |
| `AutoColorTask` / `AutoEmptyTask` | 停在 1 不再變（DoAutoColor 每輪早退）|
| `MNetLog` | 最後一筆是 `Clear tray [MMColorZ] by function DoUnLoadNewColorToStack`，之後**再無任何 Color 紀錄** |
| EventLog | **沒有任何 alarm**（JAM1412 不可達）|

### 11.6 修正（V3.33.912.1_20260908_RogerYang_AI，2026-09-14）

抽出 `IsColorReceiveHoldByCatching()` / `IsEmptyReceiveHoldByCatching()`（`asendic_Color.cpp` / `asendic_Empty.cpp`，`extern` 於同名 .h）：

```cpp
if(CUSTOMER_CODE!=CC_SCC)        return false;   // 非 JSCC 恆 false ⇒ 其他客戶零行為變更
if(bIsCatchingFromBuffer==false) return false;
if(iReceiveColorTray==2)         return false;   // ToFront 已完成＝退盤進行中, 不可中斷
return (iAutoColorReceiveTask==1);               // 只暫緩「尚未開始」的退盤
```

語意由「**中途 Init 打斷**」改成「**尚未開始才暫緩**」，並**移除 `InitAutoColorReceiveTask()` 呼叫**；
`csystem.cpp` DoTrayFeed 兩個原本無防護的 `DoAutoColorReceive()` / `DoAutoEmptyReceive()` 呼叫點補上同一判斷。

> **不會餓死的理由**：`bIsCatchingFromBuffer` 在 `DoCatchTray` case 500 拿到 `ret==2` 就同 scan 清掉，
> 而 `CatchNewTrayFromBuffer` case 2200 在 MMColor 無盤時**每輪都回 2** ⇒ 旗標逐 scan 抖動，
> 退盤總有 scan 可以啟動；**一旦啟動暫緩就不再成立**，必定跑完。

刻意未改（評估後判定不必要／屬通用風險）：`hColorTrayToFront` 計時器重設、`bIsCatchingFromBuffer` 生命週期、
`csystem.cpp` else 分支把 `iReceiveColorTray` 由 2 降級回 1 的通用漏洞。

> AI(ht9045-catchtray-flow) 20260914 (RogerYang)

---

## 合併補充：repo 既有參考（20261001）

- [references/colleague-skill-body-20260915.md](references/colleague-skill-body-20260915.md)：repo 版 SKILL.md 正文原樣保存（合併前）
