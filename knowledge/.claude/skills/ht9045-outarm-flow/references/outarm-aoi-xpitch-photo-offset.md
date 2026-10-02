# Scanner AOI 拍照位置偏移 — X-Pitch 出界 + 吸嘴欄位映射

> 案例：偉測 HHT-532（HT-9046AT+ / HT-9046LS 機型顯示 HT-9046LS），V3.33.908.15，2026-08/09
> 關鍵字：AOI 第二顆拍不到、AOI 拍照位置偏移、`DoMoveXY_ScannerAOI`、`iScannerAOI_X`、
> `OutArm XPitch out of range`、`ScannerAOI pitch probe`、`iMovePitchX=21000`、
> `GetOutArmPitchX_9045` clamp、`MOutArmPitch` 開迴路失步、`iMyCol`、`CopyInitSuck`、
> `iPickStep`、`_13` / `_14` 吸嘴交換、`Position Offset.Data`、`Tech.M_ScannerAOI_X`

---

## 1. 症狀分層（兩個不同的問題，別混在一起）

| # | 症狀 | 狀態 |
|---|------|------|
| **P1** | 過 AOI 之後 OutArm pitch「縮到最小」，之後取放全部失準，要 Home 才恢復 | **已修**（`GetOutArmPitchX_9045` clamp，RogerYang 20260825） |
| **P2** | 第一顆在環形光源正中、**第二顆落在光源左緣**（拍不到料） | **未修**（本文重點） |

P1 是馬達失步，P2 是座標算錯。**clamp 修好 P1 之後 P2 才浮出來**——因為 pitch 不再亂跑，位置誤差變成穩定可複現的固定量。

> ⚠ 步進馬達失步**不會報警**。不能用「沒有 alarm」推論「沒失步」；要看
> `Motor.xls` 的 `MOutArmPitch` 命令值與實際開度是否對得上，或直接看 clamp log 有沒有觸發。

---

## 2. P1：X-Pitch 出界撞止檔失步（已修，背景）

`MOutArmPitch` 是**開迴路步進**（`EncodeType=0`），軟體極限 ±999999 形同不擋。
呼叫端算出超過機構行程的 pitch → 馬達被驅去撞機械止檔 → 掉步 → 脈波計數器與實體永久錯開 →
下一次「回到全開」的指令變成把機構往內縮 → pitch 一直縮著。

修正：`GetOutArmPitchX_9045()`（`aoutarm9045.cpp`，函式起點約 L238）在**換算成馬達位置之前**
把 `w` 夾在 `[iXpitchMinX3, iXpitchMaxX3]`（16-picker 用 `X7` 那組）。

```cpp
int iPitchLo=(USE_PICKER_COUNT==ep16Picker)?iXpitchMinX7:iXpitchMinX3;
int iPitchHi=(USE_PICKER_COUNT==ep16Picker)?iXpitchMaxX7:iXpitchMaxX3;
if(w>iPitchHi || w<iPitchLo) { ...log...; w=(w>iPitchHi)?iPitchHi:iPitchLo; }
// 之後才是 dX_40[iX] + m*(w - iXpitchMinX3) 的線性內插
```

範圍來自 Gerneral.ini（`database.cpp`：`iXpitchMinX3=IN_OUT_ARM_X_PITCH_MIN`、
`iXpitchMaxX3=IN_OUT_ARM_X_PITCH_MAX`）。HHT-532 = `4000~12000`，與 log 印的
`range=4000~12000` 完全吻合。

### clamp 未涵蓋的孿生函式（已知缺口）

`GetOutArmPitch_9045(int w)` / `GetOutArmPitch2_9045(int w)`（`aoutarm9045.cpp` 約 L157 / L165）
用**同一條內插公式但沒有任何範圍檢查**。風險呼叫點：

| 呼叫點 | 傳入值 | 備註 |
|---|---|---|
| `AutoAlignment.cpp` (~L6899/6901) | **`iPitch*3`** | 乘法放大，與 AOI 同型風險，**最需注意** |
| `aoutarm9045.cpp` `MoveOutArmXYAndSuck()` (~L4402/4432) | 呼叫端帶入 | 只在 "Fill The Tray After Out Arm Place" 啟用時 |
| `CAlignmentB` / `AutoTeach` / `InOutArmZteach` | 4000 / 9000 / `iXpitchMaxX3` 固定值 | 範圍內，安全 |
| `LaserSensorShuttle.cpp` (~L896) | `iXPitch` | Omron laser 選配 |

> `uhome.cpp` 對 `MOutArmPitch` 下 `PSoftLimitP-10`（≈999989）是**步進歸 home 的正常做法**
> （驅動到底直到 home sensor），不是缺陷，掃描時勿誤判。

---

## 3. P2：拍照位置算錯（本文重點）

### 3.1 出問題的算式

`fAOI.cpp` → `DoMoveXY_ScannerAOI(int &iRow, int &iCol)`（函式起點約 L1722）：

```cpp
if(TestIF_File.iTestMode==SingleSite)
    iMovePitchX=8000;
else if(XPitchIsStand())  iMovePitchX=TestIF.dSiteXPitch/2*3;      // 1.5 倍
else                      iMovePitchX=TestIF.dSiteXPitch+TestIF.dSiteXPitch/2;  // 同樣 1.5 倍

for(int i=0; i<X_PITCH_COUNT; i++)
    iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);  // ← 這裡會被 clamp

iXPos=Prod.iScannerAOI_X+(iMovePitchX/3)*2-(iMovePitchX/3)*iCol;     // ← 這裡沒被 clamp
```

**兩個獨立的錯誤**：

| # | 錯誤 | 說明 |
|---|------|------|
| **A** | 位置用**未 clamp** 的 `iMovePitchX` 算單格距 | 送馬達的 pitch 被夾成 12000（單格 4000），位置卻用 21000/3 = **7000** |
| **B** | 拿 **logical `iCol`**（Item 陣列索引）當步數 | `_14` 模式兩支吸嘴實體差 **3 格**，不是 1 格 |

### 3.2 吸嘴欄位映射（B 的機制，容易看漏）

`cinitial.cpp` 初始化時 `OutArmSuck.Suck[i][j].iMyCol = j`（恆等），
**但那是 `OutArmSuckBackup` 的狀態**。`ainarm9045_1x2_2.cpp` 的 `SetInOutArmParameter_1x2_2()`
會對 `_14` 做吸嘴交換：

```cpp
if(iInArmType==e9045_1x2_2_14)
{
    CopyInitSuck(&OutArmSuckBackup, &OutArmSuck, 0, 3,  0, 1);   // 實體 col3 (Ad/G) → 邏輯 slot[0][1]
    CopyInitSuck(&OutArmSuckBackup, &OutArmSuck, 0, 1,  0, 3);   // 互換
    OutArmSuck.SetPickerCount(1, 2, 1, 2, 3, 0, 0);              // 末三碼 iPickStep=3
}
else  // _13
{
    CopyInitSuck(&OutArmSuckBackup, &OutArmSuck, 0, 2, 0, 1);    // 實體 col2 (Ac/E) → slot[0][1]
    CopyInitSuck(&OutArmSuckBackup, &OutArmSuck, 0, 1, 0, 2);
    OutArmSuck.SetPickerCount(1, 2, 1, 2, 2, 0, 0);              // iPickStep=2
}
```

`CopyInitSuck` → `CopySuck` 會**一併複製 `iMyCol`**（`mykitsuck.cpp` 約 L2856）。
所以 live 物件是 **`OutArmSuck.Suck[0][1].iMyCol == 3`**。

> `SetPickerCount` 第 5 個參數 `iPickStep` 原始註解就寫明
> 「**Pitch倍數, 13吸嘴就寫2, 14吸嘴就寫3**」（`mykitsuck.cpp` 約 L213）。
> 可當 `iMyCol` 的交叉檢查來源：`iPickStep` == 兩支吸嘴相隔幾格。

### 3.3 數字（HHT-532）

| 量 | 值 |
|---|---|
| `dSiteXPitch` | 14000（140mm） |
| `iMovePitchX`（AOI 算出） | 21000（`XPitchIsStand()`=0 → `14000+7000`） |
| clamp 後實際 pitch | **12000**（全開）→ 單格 **4000** |
| 兩顆料位置 | `Item[0][0]`（`iMyCol=0`）與 `Item[0][1]`（**`iMyCol=3`**） |
| **實體需要移動** | 3 格 × 4000 = **12000（120mm）** |
| **程式實際移動** | 1 格 × 7000 = **7000（70mm）** |
| **偏差** | **短 5000 = 50mm** ← 與照片「第二顆在光源左緣」吻合 |

### 3.4 交叉驗證（取料端是對的）

`CheckOutArmXYPitch_1x2_2()` 預設分支（`aoutarm9045_1x2_2.cpp` 約 L207-230），
`dMovePitchX = 12000/3 = 4000`、`dSiteXPitch = 14000`：

| iModeCol | 註解 | 算式 | 結果 |
|---|---|---|---|
| 100（左站） | `Aa --> Aa` | `+4000*2 − 7000` | **+1000** |
| 110（右站，_14） | `Ad --> Ab` | `−4000*1 + 7000` | **+3000** |

幾何推導（base 吸嘴 col 2，teach 錨在 shuttle 中心 `S_L + siteP/2`）：

```
把吸嘴 col c 放到左站：armX = S_L − (c−2)*gap
  iModeCol=100 → armX = S_L + 2*gap  →  c = 0   ✅ 左站用 col 0 (Aa)
把吸嘴 col c 放到右站：armX = S_R − (c−2)*gap
  iModeCol=110 → armX = S_R − gap    →  c = 3   ✅ 右站用 col 3 (Ad)
  （_13 對應算出 c = 2，符合註解 Ac --> Ab）
```

兩個 arm 位置相差 2000，與程式的 `+1000` / `+3000` 完全一致 →
**gap=4000、兩吸嘴差 3 格確認無誤**，取料端沒有問題，只有 AOI 這條路沒處理。

### 3.5 ⚠ 兩個錯必須一起修（只修一個會更糟）

| 修法 | 移動量 | 對 12000 的誤差 |
|---|---|---|
| 現況（A 錯 + B 錯） | 1 × 7000 = 7000 | 短 5000（50mm） |
| 只修 A（gap→4000，仍用 logical col） | 1 × 4000 = 4000 | 短 8000（80mm，**更糟**） |
| 只修 B（用實體 col，仍用 7000） | 3 × 7000 = 21000 | 過衝 9000（90mm，**更糟**） |
| **A + B 一起修** | 3 × 4000 = **12000** | **0** ✅ |

---

## 4. 修法

在 `DoMoveXY_ScannerAOI()` 內，用「clamp 後的實際 pitch」+「`Suck[][].iMyCol`」重算：

```cpp
int iRealPitchX =iMovePitchX;                                       // clamp 後才是實際做得到的 pitch
int iPitchLo    =(USE_PICKER_COUNT==ep16Picker)?iXpitchMinX7:iXpitchMinX3;
int iPitchHi    =(USE_PICKER_COUNT==ep16Picker)?iXpitchMaxX7:iXpitchMaxX3;
int iSuckGapX   =0;
int iMyColX     =iCol;
if(iRealPitchX>iPitchHi) iRealPitchX=iPitchHi;
if(iRealPitchX<iPitchLo) iRealPitchX=iPitchLo;
iSuckGapX=(USE_PICKER_COUNT==ep16Picker)?(iRealPitchX/7):(iRealPitchX/3);
if(iRow>=0 && iRow<MAX_ARM_Row && iCol>=0 && iCol<MAX_ARM_Col)
    iMyColX=OutArmSuck.Suck[iRow][iCol].iMyCol;

for(int i=0; i<X_PITCH_COUNT; i++)
    iXVariable[i]=GetOutArmPitchX_9045(iRealPitchX, i, iOffsetPos); // 傳已夾好的值,不再觸發 clamp log

iXPos=Prod.iScannerAOI_X+iSuckGapX*(iOutArmXBase-iMyColX);          // base = 基準軸
iYPos=Prod.iScannerAOI_Y+iMovePitchY*iRow;
```

設計要點：

1. **`base` 用 `iOutArmXBase`，不硬寫 `2`**（★RogerYang 20260902 設計裁決）。
   理由是**回歸 teach 精神：teach 點的定義就是「基準軸在相機正上方時的手臂 X」，
   其餘吸嘴一律由 `gap*(base − iMyCol)` 計算**——不為了規避重 teach 而保留舊錨點。
   `iOutArmXBase` 由 `database.cpp`（約 L803-952）依 `USE_IN_OUT_ARM_Y_PITCH` 設定：
   `ep1Picker`=2、`iXYPitchVariable`=1、`iXYPitchRowA`=1、`16Picker`=3、`Bb`/`In_Bb_Out_Bc`=2，
   最後 `USE_OUT_ARM_Y_PITCH==iXPitch60||iXPitchManual635` 會覆寫為 2（本案機台走這條）。
2. **傳 `iRealPitchX` 給 `GetOutArmPitchX_9045`**：馬達目標完全相同（函式內部本來就會夾），
   但避免 clamp log 在這台上每個循環都印——這台 site pitch 140mm > 機構上限 120mm，
   **clamp 是永久且正常的狀態，不是故障**。
3. **16-picker 的 `/7`**：與 `GetOutArmPitchX_9045` 的 `X7` 基準一致。若要對 16-picker 機型
   零更動，把該行固定成 `/3` 即可（但那樣就與 clamp 基準不一致，屬已知不一致）。

### 4.1 🚨 升版影響面：**單顆拍照也會位移，全部要重 teach**

> 早期草稿曾判斷「單顆是 no-op」，**那是錯的**，已於 20260902 更正。

錯在假設 1x1 模式 `iMyCol==iCol==0`。實際 `ainarm9045_1x1_1.cpp`（約 L85-96）的預設分支
（非 `ep16Picker`、非 `ep1Picker`）就有交換：

```cpp
CopyInitSuck(&OutArmSuckBackup, &OutArmSuck, 0, 0, 0, 3);   // 實體 col0 -> slot[0][3]
CopyInitSuck(&OutArmSuckBackup, &OutArmSuck, 0, 3, 0, 0);   // 實體 col3 -> slot[0][0]  ★
OutArmSuck.SetPickerCount(1, 1, 1, 1, 1, 0, 0);             // 只用 slot[0][0]
```

→ **單顆那一顆料掛在實體 col 3，`Suck[0][0].iMyCol == 3`**，
舊式算 `2666*2=+5333`、新式算 `2666*(2−3)=−2666`，**位移 8000 counts（80mm）**。

**為什麼單顆「原本沒問題」**：單顆只有一個落點，誤差是**常數**，被 teach 完全吸收、看不出來；
多顆的誤差**隨 col 變化**，teach 只能吸收一顆，第二顆就露餡。
**兩者是同一個 bug，單顆只是被藏起來了。**

**唯一不受影響的**：`USE_PICKER_COUNT==ep1Picker`（1x1 走 `ep1Picker` 分支不交換、
`iOutArmXBase`=2、pitch 8000 不被夾）→ 輸出不變。

### 4.2 重 teach 換算式（免現場試誤）

設 ref = 目前對得準的那一顆（多顆時通常是掃描到的第一顆）：

```
dT = (iMovePitchX/3)*(2 - iCol_ref)  -  iSuckGapX*(iOutArmXBase - iMyCol_ref)
新 Tech.M_ScannerAOI_X = 舊值 + dT
```

### 4.3 單顆（SingleSite）機台的 dT 全表 — **不是單一數字，共 5 種**

單顆固定 `iMovePitchX=8000`、`iCol_ref=0`，故 `dT = (8000/3)*2 − gap*(base − iMyCol) = 5332 − gap*(base−iMyCol)`
（整數截斷：`8000/3=2666`，所以是 5332 不是 5333）。

**關鍵：單顆有兩種 `iInArmType`，走不同的參數設定函式，吸嘴映射完全不同**：

| `iInArmType` | 分派（`ainarm9045.cpp` L1519 / L1550） | 條件 | `Suck[0][0].iMyCol` | base | gap | **dT** |
|---|---|---|---|---|---|---|
| `0 = e9045_1x1_1` | `SetInOutArmParameter_1x1_1()` | 預設（非 16/1picker、無 OtherSuck 旗標）<br>`CopyInitSuck(...,0,3, 0,0)` | **3** | 2 | 2666 | **+7998** |
| `0 = e9045_1x1_1` | 同上 | `TestIF_File.bSingleUseOtherSuck`（非 16picker）<br>`CopyInitSuck(...,0,2, 0,0)` | **2** | 2 | 2666 | **+5332** |
| `0 = e9045_1x1_1` | 同上 | `USE_PICKER_COUNT==ep1Picker` → 不交換 | 0 | 2 | 2666 | **0**（免疫） |
| `0 = e9045_1x1_1` | 同上 | `USE_PICKER_COUNT==ep16Picker` → 不交換 | 0 | **3** | **1142** | **+1906** |
| `1 = e9045_1x4_1_Ac` | **`SetInOutArmParameter_1x4_4()`**（L1550，**不是** 1x1_1）<br>`CopyInitSuck(...,0,2, 0,0)` | 無條件 | **2**（＝基準軸本身） | 2 | 2666 | **+5332** |

> `e9045_1x4_1_Ac` 那條容易看漏：它雖然也是單顆（`SetPickerCount(1,1,...)`），
> 卻是在 `ainarm9045_1x4_4.cpp` L108-117 設定，不在 1x1_1 檔裡。

### 4.4 多顆 / 其他

| 機台 / 模式 | 代入 | dT |
|---|---|---|
| 偉測 HHT-532（`_14`，gap_old=7000、gap=4000、base=2、ref: iCol=0/iMyCol=0） | `7000*2 − 4000*2` | **+6000**（−21750 → **−15750**） |

> ⛔ **結論：不要查表決定現場數值**——組合太多（`iInArmType` × `USE_PICKER_COUNT` ×
> `iOutArmXBase`(由 `USE_IN_OUT_ARM_Y_PITCH`/`USE_OUT_ARM_Y_PITCH` 決定) × OtherSuck 旗標）。
> 上表僅供理解與交叉驗證，實際請用 §6 的探針在該機上量測 `dT`（乾跑即可，見 §6.2）。

---

## 5. ⚠ teach / offset：改哪一層

修正後這台第一顆會位移，因為**現有 teach 值本身就是操作員的補償值**：

```
正解      X(col0) = C + 2格×4000 = C + 8000       （C = 相機中心）
現況(正確) X(col0) = T + 14000
→ T = C − 6000    ← teach 被往下拉了 60mm 去吸收 A+B 的誤差
```

### 5.1 兩層補償的層級與**容量**

```cpp
// cinitial.cpp 約 L11434
Prod.iScannerAOI_X = Tech.M_ScannerAOI_X + OutArmOffSet[OutOfsScannerAOI]->GetX();
//                   ^機台級(tech.dat)      ^per-recipe(Position Offset.Data)
```

| 層 | 範圍 | 單位 | **可輸入量級** |
|---|---|---|---|
| `Tech.M_ScannerAOI_X` | **機台級**，全工作檔共用（`LastSet.h` Tech 結構 → tech.dat；teach 介面 `uteach.cpp` `setEditScannerAOIX`） | 0.01mm counts | 不受 offset 限制 |
| `OutArmOffSet[OutOfsScannerAOI]->GetX()` | **per-recipe**（工作檔 `Position Offset.Data`，section `[Top Bottom AOI]`、key `Hand X`） | **mm**（`cUnitConvert.cpp` 約 L119 `iUnitMultiply100()` 才 ×100） | **只有 `InputLimit.iOffsetXYHigh/Low`，預設 ±5mm、實機 Security_new.def 也是 ±5** |

> ⚠⚠ **60mm 的補償塞不進 per-recipe offset**。
> `Security_new.def` `[Input Limit] Offset XY High/Low` 預設 `+5 / -5`
> （`cAuthority.cpp` `GetLimitAuth()`；`CC_ASE_CL` 另有 `CheckRange(...,0,10)` 硬上限 10mm）。
> 偉測 HHT-532 實機值就是 ±5。**所以「把 60mm 放進 per-recipe offset」行不通**——
> 這一階只能改 `Tech.M_ScannerAOI_X`。

### 5.2 為什麼沒有「不動 teach」的公式解

現行 teach 滿足 `T + 14000 = C + 8000` → `T = C − 6000`。
修正後 `X(col0) = T' + gap*(base−0) = T' + 4000*base`。
要讓 col0 維持不動需要 `4000*base = 14000` → `base = 3.5`，**非整數**。
代表現行 teach 補的是「錯的 gap」，不是「錯的 base」，任何合理公式都無法重現它
→ **teach 必須改，沒有繞道**。

### 5.3 升版程序：teach 是**機台級一次性**動作

`Tech.M_ScannerAOI_X` 是機台級、全工作檔共用，而**位移量對該機台是固定的**
（同一台的 `iOutArmXBase`、`iMyCol` 映射、`iMovePitchX` 由機型與模式決定）。
所以流程是：

1. 升版後，用 §4.2 的 `dT` 算出新值，直接改 `Tech.M_ScannerAOI_X` 的**數值**（不是 jog 對中，見 §5.4）
2. 開相機跑實料，微調數值收尾
### 5.3.1 ★關鍵：修正後 teach 是「一台一個常數」，切模式**不需要**再動

基準軸的物理意義就是**pitch 機構的固定參考點**——其他吸嘴在 col `m` 的位置是
`armX + (m − base)*gap`，代入 `m == base` 得 `armX`，**與 pitch、與 site 模式完全無關**。
所以

```
C ≡ 「基準軸落在相機正上方時的手臂 X」= 機台物理常數
```

`C` 不隨 site 別、吸取方式、pitch 大小改變。修正後 `Tech.M_ScannerAOI_X = C`，
**一次 teach、所有模式共用、換工作檔不用重 teach**。這正是 teach 該有的語意。

### 5.3.2 那為什麼「不同模式的 dT 不一樣」？

因為 `dT` 是**舊值與真值的差**，而**舊公式的錯誤量本身就是 mode-dependent**：

```
err(mode) = (iMovePitchX/3)*(2 − iCol_ref) − gap*(base − iMyCol_ref)
  _14 :  7000*2 − 4000*(2−0) = +6000
  單顆:  2666*2 − 2666*(2−3) = +8000
```

→ 現行的 teach 值 = `C + err(該機當初 teach 時所用的模式)`。
**`dT` 的差異是舊 bug 留下的歷史債，屬於升版當下的一次性清算，不是往後的常態負擔。**

### 5.3.3 由此得到一個可驗證的預測

現行版本下，**同一台若真的同時跑 `_14` 與單顆 AOI，今天就已經有一個是錯的**
（需要的 teach 差 2000 counts = 20mm，而 per-recipe offset 只有 ±5mm 補不到）。

所以若客戶回報「兩種模式都正常」，只可能是：
(a) 該機實際只跑其中一種、(b) 換模式時有人重調過 teach、(c) 另一種其實一直偏但沒人發現
（單顆偏移不會有「第二顆對不到」的顯性症狀，容易被當成 AOI 誤判吸收掉）。

**升版前值得順手向現場確認這一點——它同時是本分析正確性的交叉驗證。**

### 5.4 ⚠ AOI teach 點的語意＝**基準軸位置**，不是「第一顆的位置」

`Tech.M_ScannerAOI_X` 在 teach 頁是 `TECH_TWOPARA(&Tech.M_ScannerAOI_X, &Tech.M_ScannerAOI_Y,
MOutArmX, MOutArmY, ...)`（`uteach.cpp` 約 L940）。
`GoButton020Click` / `SetButton020Click`（約 L3521 / L3563）操作的是
**`MOutArmX` 的原始命令座標**——Go = 手臂開到該值，Set = 把 jog 後的原始座標寫回去。
**teach 機制本身完全不知道「哪一支吸嘴在相機下」**，語意是由消費端公式賦予的：

```cpp
iXPos = Prod.iScannerAOI_X + gap*(2 - iMyCol);   // iMyCol==2 時偏移為 0
```

→ 公式的約定是「**teach 點 = 第 2 欄吸嘴（基準軸 col2 / E / Ac）在相機正上方時的手臂 X**」。

#### 客戶證言（偉測，20260902）— 完全對上

> 問：你們 AOI 的 teach 是怎麼建的，依據一樣是基準軸嗎？
> 答：**「对，但一般会偏，然后根据实际位置调整数值」**

三句話對應本案三個環節：

| 客戶說法 | 對應 |
|---|---|
| 「依據是基準軸」 | teach 語意確實是基準軸（對準的是**空吸嘴本體**，不是料——所以基準軸上沒 IC 也對得了）→ `T` 初值 = `C`（幾何正確） |
| 「**一般会偏**」 | 幾何正確的 `T=C` 送進**錯的公式**，拍照位置就是會偏 `err` → 這句話本身就是舊 bug 的直接證詞 |
| 「根据实际位置**调整数值**」 | 手動把 `T` 調成 `C + err(mode)` → 現行 teach 值是補償值，`err` 隨模式而異（`_14`=+6000、單顆=+7998…） |

> ⚠ 早期草稿曾判斷「現場對不了基準軸（col2 沒有 IC）」，**那是錯的**——
> 對的是吸嘴本體，不需要有料。已於 20260902 依客戶證言更正。

#### ★ 因此升版後的現場程序極簡化（不需要算 dT）

修正後公式為 `iXPos = T + gap*(base − iMyCol)`，代入幾何真值 `T = C` 即完全正確。
所以**只要照他們原本的方法做一次基準軸 teach 就好**：

1. Teach 頁 → Scanner AOI X/Y → jog 到**基準軸吸嘴本體對準相機中心** → Set
2. **不要再「根据实际位置调整数值」** ← 這一步以後不需要了
3. 跑一次確認；若仍需大幅調整，代表另有問題（不該再有 `err`）

這對所有機台、所有 site 模式**一體適用**，不必查 dT 表、不必用探針。
`dT` 表（§4.3/§4.4）改為備查與交叉驗證用：
`新值 − 舊值` 應該接近表上的 `dT`，若差很多表示該機當初的手調另有別的成分。

**副產品：「一般会偏」這個長期困擾被根治**——這是本次修正對客戶最直接的價值。

per-recipe 隔離的前提：`bE45_AllSetupFileUseOneFile==0` 且 `bE59GroupOffsetFile==0`，
> 否則 `GetOffsetPath()`（`cOffSet.cpp` 約 L1426）改指向共用 `DefineOffset` 資料夾，
> offset 也變全機共用。HHT-532 兩者皆為 0，已確認；`CC_ASE_CL` 強制走共用路徑。

---

## 6. 診斷用探針（驗證完可移除）

兩支臨時 log 在本案發揮決定性作用：

| 位置 | 內容 |
|---|---|
| `fAOI.cpp` `DoMoveXY_ScannerAOI()` 算完 `iMovePitchX` 後 | `ScannerAOI pitch probe : iMovePitchX=%d, dSiteXPitch=%.0f, Stand=%d, iRow=%d, iCol=%d` |
| `aoutarm9045.cpp` `GetOutArmPitchX_9045()` clamp 區塊 | `OutArm XPitch out of range : req=%d, clamp=%d, range=%d~%d, iX=%d, OffsetPos=%d` |

實際擷取（20260831 14:13:07）：

```
ScannerAOI pitch probe : iMovePitchX=21000, dSiteXPitch=14000, Stand=0, iRow=0, iCol=0
OutArm XPitch out of range : req=21000, clamp=12000, range=4000~12000, iX=0, OffsetPos=-1
```

### 6.1 log 節流的坑

- probe 用 `iAOIProbeCT<20` 上限 → `DoMoveXY_ScannerAOI` 是 **polling 式**，單次定位就把 20 筆燒光。
- clamp 用 `w!=iLastClampReq`（同值只印一次）→ 整份 log 只留下 1 筆，看不出發生頻率。

**建議樣式**：改成「前 N 筆全記 + 之後每 M 次記一筆並帶累計數」，
既保留發生頻率又不洗版（`GetOutArmPitchX_9045` 每次定位被呼叫 `X_PITCH_COUNT`=4 次，
再乘上 polling 次數，累積很快）。

---

## 7. 教訓（LL）

1. **開迴路步進軸（`EncodeType=0`）的位置命令一律要有範圍 clamp**，軟體極限 ±999999 等於沒有保護，
   而且**失步不報警**。
2. **「pitch = f(site pitch)」的公式，遇到 `iXStep==2`（分兩趟取料）的機型都要重新檢查**——
   取料端有 `iXStep` 分支，AOI / Rotator / 其他附加功能常常沒有。
3. **`Suck[][].iMyCol` 在 `Backup` 是恆等、在 live 物件可能被 `CopyInitSuck` 換過**。
   任何「用陣列索引當實體距離」的算式在 `_13`/`_14` 這類交換模式都會錯。
   交叉檢查用 `SetPickerCount` 的 `iPickStep`（13→2、14→3）。
4. **兩個誤差互相抵消時，只修一個會比不修更糟**。改之前先把「只修 A」「只修 B」「AB 都修」
   三種結果都算出來。
5. **Dummy run 驗得了馬達命令，驗不了相機**：`DoScanAOIFunction` case 1000 在
   `LastSet.iRealDummy!=REALLY` 時直接跳到 case 3000、強制 `bAOIPassFail=true`，
   整段 `DoScanAOIFunction_Inspection()` 不執行。位置偏移只有實料 + 開相機才看得出來。
6. **操作員「調 teach 讓它能跑」會把軟體缺陷藏起來**。看到「第一顆對、第二顆錯」這種
   pattern，要先懷疑 teach 已經吸收了某個固定誤差，修程式時必須連帶算出 teach 要回補多少。

---

## 7b. ⏸ 已知未修：Y 方向的同型缺陷（RogerYang 20260902 裁決暫緩）

`DoMoveXY_ScannerAOI()` 修好 X 之後，**下一行的 Y 有一模一樣的兩個問題**：

```cpp
iYPos = Prod.iScannerAOI_Y + iMovePitchY*iRow;      // 未修
```

| # | 問題 | 證據 |
|---|---|---|
| Y-a | 用 **logical `iRow`**，不是 `Suck[iRow][iCol].iMyRow` | `ainarm9045_1x4_4.cpp` 的 `e9045_1x4_4_Back` 分支整排對調 → `Suck[0][j].iMyRow = 1`（全 14 個模式檔中**只有這一支**會換排） |
| Y-b | **基準排硬當 0**（`iRow=0` 時偏移為 0），沒用 `iOutArmYBase` | `database.cpp` L825/870/893/916/939 設 `iOutArmYBase=1`（Variable、16Picker、Bb…）；L808/846/953 為 0 |

對照組：取料端 `CheckOutArmXYPitch_1x2_2()` **有**處理基準排
（用 `USE_OUT_Y_IS_AUTO_PITCH` 分 Row A / Row B 給 `−iMovePitchY` 或 `+iMovePitchY`），
只有 AOI 這條沒有——與 X 的情況完全平行。

對稱寫法：

```cpp
int iMyRowX=OutArmSuck.Suck[iRow][iCol].iMyRow;
iYPos = Prod.iScannerAOI_Y + iMovePitchY*(iMyRowX - iOutArmYBase);
// iOutArmYBase==0 的機台輸出完全不變；只有 base row=1 或 e9045_1x4_4_Back 會位移
```

**為何暫不修**：無客訴、且 `iOutArmYBase=0` 的機台（含偉測 HHT-532，單排）本來就不受影響；
修了會讓 base row=1 的機台也被迫重 teach Y。
**裁決原則：同型缺陷若無客訴則不動，優先壓低客戶進版成本**，改以註解在碼上站崗
（`fAOI.cpp` `DoMoveXY_ScannerAOI()` 註解區「已知未修」段）。有客訴時照上式一次補齊。

---

## 8. 相關

- 同檔 Y 方向的姊妹案：[outarm-ypitch-softlimit-shrink.md](outarm-ypitch-softlimit-shrink.md)
  （`ShrinkOutArmYPitchForSoftLimit`，同屬「縮 pitch 類」缺陷家族）
- AOI 四種型態與 Fail Bin 對應：[outarm-aoi.md](outarm-aoi.md)
- 吸嘴基準軸 / Y-Pitch 模組定義：`ht9045-sucker-architecture` §4.3
- 附加功能執行順序（**Rotator → AOI → Fix AI CCD**，`DoOutArmAdditionalFunction()` case 100
  依序分派 10000 / 20000 / 30000，每項做完回 100 挑下一項）：本檔 §3、`aoutarm9045.cpp` 約 L2364
- StateRecord 判讀：`ht9045-staterecord-analysis`
