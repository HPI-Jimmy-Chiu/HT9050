> 保存來源：`.claude/skills/ht9045-index-flow/references/Z1UpZ2Down_MotionSequence.md`，main `e184ef205`。以下保留原文；原文中的機型／版本與「裁決、提案、已實作」仍依原標註。當前實作狀態先看 [共同與差異](../common.md)。

<!-- preserved-content:start -->
# Z1UpZ2Down1 / Z1DownZ2Up1 向量運動序列

> 原始碼：`Motor/myGALILmotor.cpp`
> 參數來源：`cinitial.cpp` `GetIndexParm()`

---

## 1. Galil 軸對應

| Galil 軸 | 馬達索引 | 物理軸 | 備註 |
|----------|----------|--------|------|
| X | MTestY1 | Y1 (Front Arm 水平) | `_BGx` |
| Y | MTestZ1 | Z1 (Front Arm 垂直) | `_BGy` |
| Z | MTestZ2 | Z2 (Rear Arm 垂直) | `_BGz` |
| W | MTestY2 | Y2 (Rear Arm 水平) | `_BGw` (4軸模式) |

LI 引數順序：`LI X_inc, Y_inc, Z_inc, W_inc` = `LI Y1, Z1, Z2, Y2`

---

## 2. 安全高度定義

```
Tech.iTestZDown (Teach 頁面 "Index to Socket Height")
    ↓
Prod.All_TestZ_Test_Safe = Tech.iTestZDown    // cinitial.cpp L9246
```

此值為 Z 軸的「安全分界線」：
- Z 在此高度以上 → Y 軸可以安全水平移動，不會撞到 Shuttle/Socket
- Z 在此高度以下 → Y 軸必須已經在 Socket 正上方（Middle 位置），否則 Z 下壓會撞機

---

## 3. Z1UpZ2Down1 動作邏輯

### 安全規則

1. **Z1 必須先上升到安全高度** → Y1 才能水平移動（否則 Z1 在 Socket 裡面橫移會撞到）
2. **Y2 必須先抵達 Middle 位置** → Z2 才能下壓到 Socket（否則 Z2 在錯誤位置下壓會撞到 Shuttle）

### LI 3 段式向量指令

```
LMXYZW;                                    // 設定線性補間模式 (X=Y1, Y=Z1, Z=Z2, W=Y2)

Step 1: LI 0, -Z1Safe, 0, 0;              // 只有 Z1 上升到安全高度
        → Z1: 從 Test 位 → All_TestZ_Test_Safe
        → Y1, Z2, Y2: 不動
        → 確保規則①: Z1 先上來

Step 2: LI Y1_delta, -Z1Up, -Z2DownSafe, Y2_delta;
        → Z1: 繼續上升到 TestZ1_Safe (最高)
        → Z2: 下降到 TestZ1_Safe (安全高度，還沒到 Socket)
        → Y1: 從 Middle → Front (移往 Shuttle 方向)
        → Y2: 從 Rear → Middle (移往 Socket 方向)
        → 4 軸線性插補同步移動，比例抵達

Step 3: LI 0, 0, -Z2Down, 0;              // 只有 Z2 最後下壓到 Test 位
        → Z2: 從安全高度 → TestZ2_Test (Socket 壓合位)
        → Y1, Z1, Y2: 不動
        → 確保規則②: 此時 Y2 已在 Step 2 抵達 Middle
```

### GetIndexParm 參數計算 (Normal Mode)

```
Z1Safe     = All_TestZ_Test_Safe - MOT[MTestZ1].Gali_ReadPos()  // Z1 到安全高度的增量
Z1Up       = TestZ1_Safe - All_TestZ_Test_Safe                  // Z1 從安全到最高的增量
Z2DownSafe = All_TestZ_Test_Safe - TestZ1_Safe                  // Z2 下降到安全高度的增量
Z2Down     = TestZ2_Test - All_TestZ_Test_Safe                  // Z2 最後下壓的增量

XShiftF    = TestY1_Front - TestY1_Middle                       // Y1 水平移動量
XShiftR    = TestY2_Middle - TestY2_Rear                        // Y2 水平移動量

TestYBuffer  = XShiftF                                          // Y1 增量 (用於 LI)
TestY2Buffer = -XShiftR                                         // Y2 增量 (用於 LI)
```

---

## 4. Z1DownZ2Up1 動作邏輯（對稱）

### 安全規則

1. **Z2 必須先上升到安全高度** → Y2 才能水平移動
2. **Y1 必須先抵達 Middle 位置** → Z1 才能下壓到 Socket

### LI 3 段式向量指令

```
LMXYZW;

Step 1: LI 0, 0, -Z2Safe, 0;              // 只有 Z2 上升到安全高度
Step 2: LI -Y1_delta, -Z1DownSafe, -Z2Up, Y2_delta;
                                            // 4 軸同動：Z2 繼續上、Z1 下到安全、Y1→Middle、Y2→Rear
Step 3: LI 0, -Z1Down, 0, 0;              // 只有 Z1 最後下壓到 Test 位
```

---

## 5. 運動中保護檢查 (JerryYang 20251124~20260114)

### Z1UpZ2Down1 中的檢查 (L1592-L1616)

```cpp
// 在 MovFlag==true (運動進行中) 的 else 分支，每次 polling 讀取 encoder
iEncoderZ2 = MOT[MTestZ2].Gali_ReadEncoderPos();   // TP command → actual encoder
iEncoderY2 = MOT[MTestY2].Gali_ReadEncoderPos();

// 檢查 1: Z2 已下降到安全線以下 → Y2 應該在 Middle
if(iEncoderZ2 < Prod.All_TestZ_Test_Safe - iCheckZ)          // iCheckZ=4000 (default)
{
    if(CheckArmPosArrival(iEncoderY2, Prod.TestY2_Middle, IniConfig.GaliPosRange) == false)
        ShowIndexMotorError("Z1UpZ2Down1");  // ← GaliPosRange=50 太嚴格，易誤報
}

// 檢查 2/3: 硬編碼絕對值判斷（與 Recipe 無關）
if(iEncoderZ2 < -4000 && iEncoderY2 > -9000)       // TEST: Z2 深、Y2 不在 Middle
if(iEncoderZ1 < -4000 && iEncoderY1 < 9000)        // TEST2: Z1 深、Y1 不在 Middle
```

### Z1DownZ2Up1 中的對稱檢查 (L1932-L1960)

```cpp
if(iEncoderZ1 < Prod.All_TestZ_Test_Safe - iCheckZ)
{
    if(CheckArmPosArrival(iEncoderY1, Prod.TestY1_Middle, IniConfig.GaliPosRange) == false)
        ShowIndexMotorError("Z1DownZ2Up1");
}
```

### 已知問題（#P260504-SCK-H9L-01）

- `GaliPosRange=50` 對於運動中 encoder following error (~100 counts) 來說太嚴格
- 向量插補 Step 2→Step 3 轉換時，Z2 command 已開始 Step 3 下壓，
  Y2 encoder 仍有 ~100 counts 殘差未收斂 → 觸發誤報
- 檢查 2/3 的硬編碼 `-4000 / -9000` 與 Recipe Teach 點無關，對不同 Recipe 可能不適用

### 目前未使用的 Galil 原生保護

| 機制 | 說明 | 狀態 |
|------|------|------|
| `ER n` | Position Error Limit (following error 上限) | **未使用** |
| `OE n` | Off-on-Error (超限時自動關馬達) | **未使用** |
| `TE` | Tell Error (讀取 following error) | **未使用** |
| `#POSERR` | Galil firmware 自動中斷副程式 | **未使用** |

所有位置保護均由 Host 端 C++ 程式以 `Gali_ReadEncoderPos()` (TP) polling 實現。

---

## 6. 運動完成後的到位檢查

```
MovFlag==true → polling Gali_ScanMotStatus() → Led[iInposLed]==false (4 軸皆停)
    → GaliSofDelayCount >= DelayCount
        → CheckPos(true/false)     // 檢查 Y1/Y2 encoder 是否在 Front/Middle 或 Middle/Rear
        → ISZ1Up_Z2Down() 或 ISZ1Down_Z2Up()  // 檢查 Z1/Z2 encoder 是否在預期位置
            → return true (動作完成) 或 ShowMotorErrorMessage (到位失敗)
```

此處使用 `GaliPosOffSet`（而非 `GaliPosRange`）作為容差。

---

## 7. GaliProtectMode 三段式保護方案 (Steven 20260504)

> config.ini [Index] 區段：`GaliProtectMode=0` (預設)

### 方案概覽

| Mode | 名稱 | 說明 | 優點 | 缺點 |
|------|------|------|------|------|
| 0 | Original | JerryYang 原始 encoder polling 檢查 | 向後相容 | GaliPosRange=50 太嚴格易誤報 |
| 1 | ER+OE | Galil firmware 自動保護 | 零 polling 延遲、硬體即時反應 | 超限時直接斷馬達，需 Home 重啟 |
| 2 | TE | Host 端讀 Following Error | 直接量測偏差，不受段間轉換影響 | 仍為 polling，有取樣延遲 |

### Mode 0 — Original (預設)

與 V3.32.751/V3.33.899 行為相同：
- 讀取 `Gali_ReadEncoderPos()` (TP 指令)
- 用 `CheckArmPosArrival(encoder, target, GaliPosRange)` 比較
- 已知 Bug: Following error ~100 counts + GaliPosRange=50 → 誤報

### Mode 1 — ER+OE (Galil Firmware Protection)

初始化時 (`Open_GaliCard()`)：
```cpp
DMCCommand(hDmc, "ER 2000,2000,3000,2000", ...);  // X=Y1, Y=Z1, Z=Z2, W=Y2
DMCCommand(hDmc, "OE 1,1,1,1", ...);              // Off-on-Error enabled (all axes)
```

- ER 值：Y1=2000, Z1=2000, Z2=3000 (較寬，因為 Z2 下壓距離長), Y2=2000
- 運動中 Host 端不做 encoder polling 檢查
- 若 Galil firmware 偵測 |TD| > ER → 自動停止馬達，SC 回報 8
- Host 端 SC check: `if(lSC==100 || lSC==8)` 觸發 alarm

### Mode 2 — TE (Following Error Check)

運動中 Host polling：
```cpp
int iFollowErrY1 = Gali_Command("TEx");   // Tell Error for axis X (=Y1)
int iFollowErrZ1 = Gali_Command("TEy");   // Tell Error for axis Y (=Z1)
int iFollowErrZ2 = Gali_Command("TEz");   // Tell Error for axis Z (=Z2)
int iFollowErrY2 = Gali_Command("TEw");   // Tell Error for axis W (=Y2)

if(abs(Y1)>2000 || abs(Z1)>2000 || abs(Z2)>3000 || abs(Y2)>2000)
    → ShowIndexMotorError("Z1UpZ2Down1_FE" / "Z1DownZ2Up1_FE");
```

- TE = Command Position - Actual Position（跟隨誤差）
- 正常運動中 TE 在 50~200 counts 之間
- 閾值 2000~3000 遠大於正常值，只有真正碰撞/阻礙/脫步才會觸發
- 優於 Mode 0: 不需與 Middle 目標位置比較，避免段間轉換的暫態誤判

### SC 停止碼擴展

```cpp
long lSC = Gali_Command("SC");
if(lSC==100 || lSC==8)  // 100=normal stop; 8=OE stop (firmware detected error)
```

Mode 1 使用時，SC=8 表示 Galil firmware 偵測到 ER 超限並自動停止。

<!-- preserved-content:end -->
