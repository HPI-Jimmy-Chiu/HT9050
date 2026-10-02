# Shuttle ↔ InArm/OutArm Safety Interlock

> Source: `acarry.cpp`, `aoutarm9045_2x8_8.cpp`, `csystem.cpp`
> Project: HT9011UC_Code_V3.33.904.4

---

## 1. 互鎖總覽（三層保護）

Shuttle 移動與 InArm/OutArm 取放料之間有三層安全機制：

| 層次 | 機制 | 方向 | 位置 |
|------|------|------|------|
| **L1** | `fCanMoveR` / `fCanMoveM` 旗標鎖定 | OutArm → Shuttle（阻止 shuttle 移動） | `aoutarm9045_2x8_8.cpp` |
| **L2** | `DoInOutARM_SHT_MoveSafe()` 位置安全檢查 | Shuttle 移動時自檢（偵測 arm Z 在 shuttle 區） | `acarry.cpp` L7456 |
| **L3** | `OutSHT1InRT()` / `OutSHT2InRT()` 到位確認 | OutArm → Shuttle（確認 shuttle 已到位才吸料） | `csystem.cpp` L456/697 |

---

## 2. L1: fCanMoveR 旗標鎖定

### 機制

OutArm 決定從 Shuttle 吸料時，先將對應 shuttle 馬達的 `fCanMoveR` 設為 `false`，
shuttle 狀態機 (`Do_Auto_SHT1/SHT2`) 多個關鍵 case 會檢查此旗標，為 `false` 時不執行移動。

### 時序

```
OutArm case 1140/1100:
  OutSHT1InRT() && FRCarryKit.UseSiteHasIC()
    → MOT[MInShuttle1].fCanMoveR = false     ← 鎖定 shuttle
    → Task = 1200 (進入 DoPickFromShuttle)

OutArm case 1200:
  DoPickFromShuttle_9045_2x8_8(0)            ← 吸料（shuttle 不能動）
    → 完成後進入 case 3000+

OutArm case 50 (reset):
  MOT[MInShuttle1].fCanMoveR = true          ← 解鎖 shuttle
  MOT[MInShuttle2].fCanMoveR = true
```

### 關鍵程式碼位置（aoutarm9045_2x8_8.cpp）

| 動作 | 行號 | 說明 |
|------|------|------|
| SHT1 鎖定 | L2245, L2299 | `MOT[MInShuttle1].fCanMoveR=false` |
| SHT2 鎖定 | L2500, L2546 | `MOT[MInShuttle2].fCanMoveR=false` |
| 解鎖 | L2005-2006, L2660-2673 | `fCanMoveR=true` |

### Shuttle 端受 fCanMoveR 控制的 case

`Do_Auto_SHT1()` 在以下 case 檢查 `MOT[MInShuttle1].fCanMoveR`（為 false 則不進入移動步驟）：
- case ~120（左移前檢查殘料）
- case ~200（右移前檢查）
- case ~300（右移到位後等待）

---

## 3. L2: DoInOutARM_SHT_MoveSafe()

### 呼叫階層

```
DoInOutARM_SHT_MoveSafe(iShuttle)              acarry.cpp L7456
  ├─ DoINARM_SHT_MoveSafe(iShuttle)            L7482   ← InArm Z 是否在 shuttle 區
  │    └─ 若觸發 → StopMotor + MoveInArmZToPlateSafe(2222)
  └─ DoOutARM_SHT_MoveSafe(iShuttle)           L7521   ← OutArm Z 是否在 shuttle 區
       └─ 若觸發 → StopMotor + MoveOutArmToAutoSafe_9045()
```

### 觸發條件（Gate）

**⚠ 關鍵：所有呼叫點都被 `IniConfig.bF21InOutArmZMotorPrivate` 門控：**

```cpp
if(IniConfig.bF21InOutArmZMotorPrivate)      // Config F21 必須開啟
{
    if(DoInOutARM_SHT_MoveSafe(0))
        return;
}
```

`bF21InOutArmZMotorPrivate=false` 時此安全檢查完全不生效。

### 呼叫點統計（acarry.cpp 904.4 版）

| 函式 | shuttle 參數 | 呼叫次數 |
|------|-------------|---------|
| `Do_Auto_SHT1()` | 0 | 7 處 |
| `Do_Auto_SHT2()` | 1 | 8 處 |
| Sort Shuttle (9046AU) | 2 | 2 處 |

### DoOutARM_SHT_MoveSafe 判斷邏輯（acarry.cpp L7521）

```
Step 1: 掃描所有 OutArm Z 軸
  → 任何一軸 Enable 且 Led[iHomeLed]==false（Z 不在 Home）→ bAnyZDown=true
  → 全部 Z 在 Home → return false（安全）

Step 2: 位置範圍判斷
  SHT1: iXPos < XShtTeach+6000 && iYPos ∈ [YShtTeach-1000, YShtTeach+2000]
  SHT2: iXPos < XShtTeach+6000 && iYPos > YShtTeach-3000
  若在範圍內且 bAnyZDown → return true（不安全，阻擋 shuttle）
```

### DoINARM_SHT_MoveSafe 判斷邏輯（acarry.cpp L7482）

與 OutArm 對稱，但 X 方向相反（InArm X 正方向 = 右，取 `-6000`）：

```
SHT1: iXPos > XShtTeach-6000 && iYPos ∈ [YShtTeach, YShtTeach+2000]
SHT2: iXPos > XShtTeach-6000 && iYPos > YShtTeach-3000
```

### 已知問題（V3.33.904.4 現狀）

| 問題 | 說明 |
|------|------|
| SHT1 Y 範圍過窄 | 僅 3mm（ShtY-1000 ~ ShtY+2000），picker 足跡遠大於此 |
| SHT2 Y 無上界 | `iYPos > ShtY-3000` 開放式上界 |
| Pre-filter 提前排除 | `iXPos>XSafe1 \|\| iYPos<YSafe1-6000` 以 SHT1 為基準，可能跳過 SHT2 檢查 |

---

## 4. L3: OutSHT1InRT() / OutSHT2InRT() 到位確認

### 判斷邏輯（csystem.cpp L456/697）

```
OutSHT1InRT() = InSHT1InRT():
  1. MOT[MInShuttle1].ScanMotorStatus()
  2. InPos LED 必須亮（馬達靜止到位）
  3. Encoder 位置 = Prod.InSHT[0].iRight（教導的 Right 位置，容許誤差 ≤ 9 pulse）
  → 三項全滿足才 return true
```

### OutArm 在吸料前的使用（aoutarm9045_2x8_8.cpp）

```cpp
// case 1140
if(OutSHT1InRT() && bCheckShuttle1Flag==false)
{
    if(FRCarryKit.UseSiteHasIC())
    {
        MOT[MInShuttle1].fCanMoveR=false;   // L1: 鎖定
        Task=1200;                          // 進入吸料
    }
}
```

---

## 5. 互鎖時序圖

```
                 OutArm                    Shuttle (Do_Auto_SHT1)
                   │                              │
                   │ ← OutSHT1InRT() ────────────── 到位確認
                   │                              │
                   │── fCanMoveR=false ──────────→ │ (被鎖定)
                   │                              │
                   │── MoveOutArmToShuttle ──→     │
                   │── Z Down (pick) ──→           │ [fCanMoveR=false, 不能動]
                   │── Suck() × N ──→              │
                   │── Z Up (home) ──→             │
                   │                              │
                   │── fCanMoveR=true ───────────→ │ (解鎖)
                   │                              │ ← 可以移動
                   ↓                              ↓

During shuttle movement (fCanMoveR=true):
  └─ 每個 case 檢查 DoInOutARM_SHT_MoveSafe()
     └─ 若 Arm Z 在 shuttle 區 → 強制停 shuttle + 移 Arm 到安全位
```
