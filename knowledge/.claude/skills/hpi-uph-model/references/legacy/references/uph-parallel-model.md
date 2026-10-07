> 保存來源：`.claude/skills/ht9045-uph-model/references/uph-parallel-model.md`，main `76dd45f37`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# UPH 三大平行模型完整推導

## 符號定義

| 符號 | 意義 | 單位 |
|------|------|------|
| $T_{IA}$ | InArm 一個 cycle 時間（Loader → HotPlate → Sht 全程） | sec |
| $T_{HP}$ | HotPlate Soak Time | sec |
| $N_{HP}$ | HotPlate 一次可容納 IC 數 | pcs |
| $T_{IDX}$ | Index cycle time（Sht pick → release → 退回 Sht） | sec |
| $T_{SD}$ | Start Delay（Initial Delay） | sec |
| $T_{TT}$ | Test Time（Tester EOT） | sec |
| $T_{OA}$ | OutArm cycle time | sec |
| $T_{LT}$ | Loader 補空盤 + 入新盤一次時間 | sec |
| $T_{ULT}$ | Unloader 退滿盤 + 補空盤一次時間 | sec |
| $N_{site}$ | 一次 Index 同時測試 IC 數 | pcs |
| $N_{Tray}$ | 一盤 IC 數（TrayX × TrayY） | pcs |
| $Y$ | Yield | % |

---

## 規則 1：HotPlate 平行（Soak 吸收條件）

HotPlate 為「容器型平行」：IC 放入後自行計時 Soak，不需 InArm 等候。
只要 InArm 把 HotPlate 排滿並回到第一顆所花時間 ≥ Soak Time，Soak 不影響 UPH。

$$T_{IA\_eff} = \max\left(T_{IA},\ \frac{T_{HP}}{N_{HP}}\right)$$

**Worked**：
- 若 $T_{IA}$ = 4 sec、$T_{HP}$ = 8 sec、$N_{HP}$ = 4 → $T_{HP}/N_{HP}$ = 2 sec → **$T_{IA\_eff}$ = 4 sec**（Soak 完全吸收）
- 若 $T_{IA}$ = 1 sec、$T_{HP}$ = 30 sec、$N_{HP}$ = 4 → $T_{HP}/N_{HP}$ = 7.5 sec → **$T_{IA\_eff}$ = 7.5 sec**（Soak 主導 InArm）

**邊界條件**：
- 無 HotPlate 模式 → $T_{HP}$ = 0
- HotPlate 不滿盤運行（Lot 末尾） → 暫態 Soak 主導，本公式只算穩態

---

## 規則 2：三段平行（主瓶頸）

InArm 把 IC 放到 Sht → Sht 把 IC 送到 Index → Index 測完放回 Sht → OutArm 取走。
**三段同時運作**，cycle time 由最慢者決定：

$$T_{cycle} = \max(T_{IA\_eff},\ T_{IDX} + T_{SD} + T_{TT},\ T_{OA})$$

### 場景分析

| 場景 | 主導段 | 改善方向 |
|------|--------|---------|
| Short TT（< 2 sec） | InArm 或 OutArm | 提高 InArm/OutArm Speed%、優化 Pitch |
| Mid TT（2~10 sec） | Index + TT | 縮 Drop Contact、縮 Start Delay |
| Long TT（> 10 sec） | Index + TT | 增加 site 數、改 2-Arm 32-Site |

### Shuttle 並入

$T_{Sht}$ 與 $T_{IA\_eff}$、$T_{IDX}$ 同時運作。實務上：
- Sht 進場時間 → 並入 Index 路徑（Index 等 Sht 到位才能下壓）
- Sht 出場時間 → 並入 InArm/OutArm 路徑（Arm 等 Sht 到位才能放/取）

簡化做法：把 $T_{Sht}$ 拆成 in/out 兩半，分別加進 Index 與 InArm/OutArm。

---

## 規則 3：Tray Arm 攤提

Tray Arm 不在每顆 IC 的流程上，而是每盤 IC 觸發一次。攤提到單顆 IC：

$$T_{cycle}' = \max\left(T_{cycle},\ \frac{T_{LT}}{N_{Tray}},\ \frac{T_{ULT}}{N_{Tray}}\right)$$

**Worked**（TrayX×Y = 10×25 = 250 pcs）：
- $T_{LT}$ = 30 sec → 30/250 = 0.12 sec/pc
- 若 $T_{cycle}$ = 2 sec → **完全藏起來**
- 若 $T_{cycle}$ = 0.1 sec（極短 TT + 8 site） → Tray Arm 變主瓶頸

**何時可能 Tray Arm 主導**：
- 非常短 TT（< 1 sec）+ 大 site 數
- 小 tray（< 50 pcs）+ 短 TT

---

## 最終 UPH 公式

$$\boxed{\text{UPH} = \frac{3600}{T_{cycle}'} \times N_{site} \times \frac{Y}{100}}$$

### 完整代換式

$$\text{UPH} = \frac{3600 \cdot N_{site} \cdot Y / 100}{\max\left(\max(T_{IA}, T_{HP}/N_{HP}),\ T_{IDX} + T_{SD} + T_{TT},\ T_{OA},\ T_{LT}/N_{Tray},\ T_{ULT}/N_{Tray}\right)}$$

---

## 特殊修正項（可選）

| 項目 | 修正方式 | 何時加入 |
|------|---------|---------|
| Retest 比率 $r$ | $\text{UPH}_{net} = \text{UPH} \times (1 - r)$ | 客戶要 Net UPH（已扣 retest） |
| Auto Clean 中斷 | 每 N 顆 IC 加 $T_{clean}$ 攤提 | 有強制 Auto Clean policy |
| ATC 升降溫 | 換溫一次加 $T_{ramp}$ | 多溫程式 |
| Tray 換盤 idle | Loader empty 時加 $T_{wait}$ | 補盤不及時 |

預設 UPH 公式 **不包含** 這些修正項，由分析者依場景決定。

---

## 為什麼這樣切？

| 切法 | 理由 |
|------|------|
| HotPlate 為獨立平行 | 容器型機構，Soak 與 InArm 物理上不互鎖 |
| InArm/Index/OutArm 三段平行 | Sht 為兩端 buffer，三段可並行作業 |
| Tray Arm 為攤提 | 不在主流程，只在 tray 切換時觸發 |
| Shuttle 並入而非獨立 | Sht 與 Arm 有互鎖（等 Sht 到位才能放/取），不能視為純平行 |

<!-- preserved-content:end -->
