---
name: ht9045-uph-model
description: >
  HT9045／HT9046／HT9011UC 系列 IC Test Handler 離線 UPH 解析式計算與馬達速度校正表（UPH_MotorProfiler）知識庫。
  Use when: 估算客戶條件（Tray X×Y、Soak、Test Time、2D on/off、Speed %）下的 UPH、分析實機 UPH 低於規格、
  HotPlate Soak／Index 平行／Tray Arm 攤提、用 Mot_Table 速度反推每段 cycle time、UPH_MotorProfiler 用法與記錄檔欄位。
  關鍵字：UPH, 產能, Throughput, Cycle Time, Soak Time, Test Time, Tray Form, Motor Speed, MotorProfiler, Mot_Table, Speed 校正。
---

# HT9045 UPH Model

HT9045 / HT9046 / HT9011UC 系列 IC Test Handler **離線 UPH 解析式計算** 與
**馬達速度校正表（UPH_MotorProfiler）** 知識庫。

當使用者詢問下列問題時，應載入此 Skill：

- 「客戶要 QFP/QFN 在 Tray X×Y、Soak N 秒、Test M 秒、2D on/off 條件下的 UPH」
- 「換成 Speed 80% 後 UPH 會差多少」
- 「為什麼實機 UPH 低於規格」
- 「HotPlate Soak 是否被吃掉」「TestTime 多長以上 Tray Arm 不影響 UPH」
- 「如何用 Mot_Table 速度反推每段 cycle time」
- 「UPH_MotorProfiler 工具怎麼用 / 記錄檔欄位定義」

## 觸發關鍵字

UPH, 產能, Throughput, Cycle Time, Soak Time, Test Time, Tray Form, Motor Speed,
模擬 UPH, UPH 表格, UPH 估算, UPH 公式, MotorProfiler, Mot_Table, Speed 校正,
HotPlate 平行, Index 平行, Tray Arm 攤提, ProcessMotorHome, Teach 座標, Auto Home

## 適用版本

HT9011UC V3.33.905.0 以後（含 Mot_Table 表格驅動馬達架構）。

---

## 1. 機台既有 UPH 計算（Per-Tray Real-Time）

機台目前的 UPH 是 **後驗式每盤計算**：

1. `asendic_Loader.cpp` — 新盤入 Loader 完成時 → `bRecordUPH = true`
2. `ainarm9045.cpp` — 每次 Loader 吸取一排 → `iUPH_LoaderCount++`
3. `ainarm9045.cpp` — InArm 到 Loader 吸**下一盤第一顆** (iCol≤0, iRow==0) → 觸發 `CalculateUPH(false)`
4. 公式：$\text{UPH} = \frac{3600}{(\text{End} - \text{Start} - \text{Pause})_{\text{sec}}} \times \text{iUPH\_LoaderCount}$

特徵：包含所有真實因素（JAM、retry、idle、Auto Clean），但**只有跑完一盤後才能得知**。

詳細程式碼對照 → [`references/uph-existing-realtime-method.md`](references/uph-existing-realtime-method.md)。

---

## 2. 三大平行模型（解析式預測 — 核心觀念）

UPH 不是各段時間相加，而是 **「最慢的那條平行路徑」決定** 一個 cycle 的長度。

### 規則 1：HotPlate Soak 與 InArm 平行
若 InArm 把 HotPlate 排滿並回到第一顆所花時間 ≥ Soak Time，Soak 完全被吸收：

$$T_{IA\_eff} = \max(T_{IA},\ T_{HP}/N_{HP})$$

### 規則 2：InArm vs Index+Test vs OutArm 三段平行（主瓶頸）

$$T_{cycle} = \max(T_{IA\_eff},\ T_{IDX} + T_{SD} + T_{TT},\ T_{OA})$$

當 $T_{TT}$ 很大時 Index 段主導，InArm/OutArm 完全藏起來。

### 規則 3：Tray Arm 補退盤攤提
$T_{LT}$（loader 補盤）、$T_{ULT}$（unloader 退盤）攤提到 Tray 內 IC 數 $N_{Tray}$：

$$T_{cycle}' = \max(T_{cycle},\ T_{LT}/N_{Tray},\ T_{ULT}/N_{Tray})$$

長測試秒數時三項都遠小於 $T_{cycle}$ → 完全隱藏，**不影響 UPH**。

### 最終 UPH

$$\text{UPH} = \frac{3600}{T_{cycle}'} \times N_{site} \times \frac{Y}{100}$$

詳細推導與每段時間構成 → [`references/uph-parallel-model.md`](references/uph-parallel-model.md)。

---

## 3. 11 大時間項定義

| 編號 | 名稱 | 包含動作 | 落在哪條平行路徑 |
|------|------|----------|----------------|
| 1 | InArm 吸放料 | Loader Suck + Vacuum + Place | InArm |
| 2 | Tray Form 多次擺放 | 同顆 row 內 X/Y pitch 連續放料 | InArm |
| 3 | HotPlate Soak | IC 入加熱盤後等待 | HotPlate（與 InArm 平行） |
| 4 | Rotate / Bottom 2D | 旋轉、底部 2D 讀取 | InArm |
| 5 | Shuttle 移動 + 2D | Shuttle 進/出 + 2DID scan | Shuttle（並入 InArm 或 Index 路徑取大者） |
| 6 | Index 吸放 | Sht pick / release | Index |
| 7 | Index Contact | Z 下壓、Drop Contact | Index |
| 8 | Start Delay + Test Time | Initial Delay + Tester EOT | Index |
| 9 | OutArm 吸放 + Rotate + 4S | OutArm cycle | OutArm |
| 10 | Loader Tray 補入 | Tray Arm 取空 + 補新 tray | Tray Arm（攤提） |
| 11 | Unloader Tray 退出 | Tray Arm 取滿 + 補空 tray | Tray Arm（攤提） |

對應全域變數 / 函式來源 → [`references/uph-stage-breakdown.md`](references/uph-stage-breakdown.md)。

---

## 4. 馬達速度校正表（UPH_MotorProfiler — 三方混合架構）

### 目的
產出離線查表：給定（馬達、距離、Speed%、Acc%），輸出實測平均時間，讓 UPH 公式
不需實機就能模擬任意客戶條件。

### 架構：埋點 + uMotorTest 補位 + Excel 計算

| 元件 | 角色 | 風險 |
|------|------|------|
| **Phase 1A 流程埋點** | 量產被動收集真實 cycle time（`IniConfig.bP11_1_RecordFlowProcessTime` 開關控制） | 極低（read-only） |
| **Phase 1B uMotorTest UPH Tab** | 4 子分頁：Logger 開關 / Coverage Map 熱圖 / UPH 即時試算 / Smart 補位掃描 | 補位掃描沿用 ContinuousMove，撞機風險可控 |
| **Phase 2 Excel UPH_Calculator** | 灌入 CSV 做擬合與全條件矩陣展開 | 零 |

**三方共用同一份 CSV**：`D:\HT9045_Log\UPH_Profile\YYYY\MM\UPH_<Model>_<SN>_<YYYYMMDD>.csv`

### 實作優先順序（建議）

1. Phase 1A 流程埋點（~1~2 天） — 量產即收資料
2. Phase 1B-1 Coverage Map（~1 天） — 看資料密度
3. Phase 1B-3 UPH 即時試算（~1 天） — 客戶問即時答
4. Phase 2 Excel 計算器（~0.5 天）
5. Phase 1B-2 Smart 補位掃描（~2~3 天） — 跑 1~2 週後再評估必要性

### 校正策略：Teach 座標 + 機台固定尺寸導出距離（適用 Phase 1B 補位）
每顆馬達利用**實際 Teach 座標 + 機台固定尺寸**導出 1~4 段移動距離，
量測結果直接對應真實生產行程（非固定 10mm/100mm/全行程）。

關鍵設計：
- **InArm** 使用 `InArmContinuousMove_9045()` 量測完整移動時間（Z-Safe→Pitch→XY同動→ZDown→Cylinder Delay），非個別 `MotorMove()`；含 Loader/HotPlate/Shuttle 多路徑段
- **OutArm** 使用 `OutArmContinuousMove_9045()` 同上鏡像；含 Shuttle/AutoTray/FixTray/Magazine 多路徑段
- **Index 8 步交替 cycle**（a~h）完整量測：a/c/e/g 分軸 Y+Z、b/f Shuttle、**d `Z1DownZ2Up` + h `Z1UpZ2Down` 雙軸聯動**（atester.cpp:11184/11233）；$T_{IDX}$ = 8 步加總
- **Z 高度極限以 cContact `IndexContact[i]`/`IndexPlace[i]` 為準**，Y/Shuttle 移動前強制 Z-Safe 互鎖防撞機
- **TrayArm X** 起終點自動加 6.5cm 補償偏移
- 可解出 $T_{acc}$ 與 $v_{max}$，套 $T(d) = 2 T_{acc} + (d - d_{acc})/v$ 回推任意距離

### Phase 1A 埋點位置（~15~25 個呼叫點）

| 模組 | 呼叫點 | tag 前綴 |
|------|--------|---------|
| InArm | `ainarm9045.cpp` 所有 `InArmContinuousMove_9045()` | `IA_*` |
| OutArm | `aoutarm9045.cpp` 所有 `OutArmContinuousMove_9045()` | `OA_*` |
| Index | `atester.cpp:11184/11233/11381/11430` Z1DownZ2Up / Z1UpZ2Down | `IDX_*` |
| Shuttle | `acarry.cpp` MOT[MInShuttle*].MotorMove | `SHT*` |
| TrayArm | `acatchtray.cpp` MOT[MTrayX].MotorMove | `TRAY_X_*` |

埋點包裝為 macro，IniConfig OFF 時編譯成 noop，零量產性能影響。

### Speed + Acc 掃描（Phase 1B 補位用）
- Speed 預設 5%~100%，步進 5%
- Acc 預設 100%~100%（固定），改起始/步進即跑 Acc 維度，**不需改 code**
- 每組重複 3 次取平均
- Smart 模式：只跑 coverage < MinSamples 的格子（通常 30 分內完成）

### 馬達清單（43 顆，依 Mot_Table.csv）

| 群組 | 馬達 | 數量 | UPH 影響 |
|------|------|------|---------|
| InArm | M00~M10 (X/Y/Pitch/ZA~ZH)、M31 (PitchY)、M32 (PitchX2) | 13 | $T_{IA}$ |
| Shuttle | M11/M12 (InShuttle1/2)、M17/M18 (OutShuttle1/2) | 4 | $T_{IDX}$ 內含 |
| Index | M13/M14 (TestY1/Z1)、M15/M16 (TestZ2/Y2) | 4 | $T_{IDX}$ |
| OutArm | M19~M29 (X/Y/Pitch/ZA~ZH)、M33 (PitchY)、M34 (PitchX2) | 13 | $T_{OA}$ |
| Tray Arm | M30 (TrayX) | 1 | $T_{LT}/T_{ULT}$ |
| Loader/Unloader Z | M35 (LoaderZ)、M36 (EmptyZ)、M37 (ColorZ)、M38~M40 (Auto1~3 Z) | 6 | $T_{LT}/T_{ULT}$ |
| Rotate / AOI | M41 (InRotate)、M42 (OutRotate)、M43 (AOIKit) | 3 | InArm/OutArm 附加 |

Asendic 系列（`asendic_Auto.cpp`、`asendic_Loader.cpp` 等）本身不持有獨立馬達，
共用 M30 + M35~M40，所以列在 Tray Arm / Loader Z 群組內，毋須額外計時。

詳細欄位、CSV 格式、埋點 API、UI 元件 → [`references/uph-motor-time-table.md`](references/uph-motor-time-table.md)。

---

## 5. 編譯開關與自動歸零

### 不需要 `#define`

UPH Motor Profiler 直接整合在 `uMotorTest` 的新 Tab `tsUPHProfiler`。
`uMotorTest` 本身只在 Service mode 下開啟，天然隔離量產流程。

### 自動歸零（Auto Home）

啟動 Scan 時**自動呼叫 `ProcessMotorHome(0)` (uhome.cpp)**，不需手動先 Home。
此函式內建 Z 軸優先、EMG 檢查、多馬達卡分流。

### 安全門中斷 → 重新初始化

安全門被開啟 → 立即停止馬達 + Flush CSV + `UPHSCAN_INTERRUPTED` 狀態。
Resume 時**重新歸零** (`ProcessMotorHome(0)`) 再從中斷點接續。
→ 技術人員可能已手動移動機構，必須 re-Home 確保 encoder 位置正確。

---

## 6. 使用流程（FAQ）

### Q1：客戶要算 UPH，從哪裡開始？
1. 收集條件：Package、Tray X×Y、Sites/Index、2D、Soak、TT、各馬達 Speed%、Yield
2. 查 motor time table CSV 得到各段 $T_{IA}, T_{IDX}, T_{OA}, T_{LT}, T_{ULT}$
3. 套規則 1→2→3 算 $T_{cycle}'$
4. UPH = 3600 / $T_{cycle}'$ × $N_{site}$ × $Y$/100

Worked example → [`references/uph-formula-worked-example.md`](references/uph-formula-worked-example.md)。

### Q2：實機跑出來與公式差很多怎辦？
先排除以下因素再質疑公式：
- Vacuum delay、Destroy 吹氣
- Site Off / Retest 比率
- Auto Clean、Pick Error 重試
- Tray 換盤等待（loader empty / unloader full）
- ATC 升降溫等候

驗證對照 → [`references/uph-validation-cases.md`](references/uph-validation-cases.md)。

### Q3：MotorProfiler 怎麼啟用？
1. 進入 Service mode → 開啟 Motor Test 畫面
2. 切換到「UPH Profiler」Tab
3. 勾選馬達 → 設定 Speed/Acc 範圍 → 按「Start Scan」
4. 程式自動歸零 → 掃描 → 輸出 CSV 到 `D:\HT9045_Log\UPH_MotorTime\YYYY\MM\`

---

## 7. 安全注意事項

- Profiler 只在 **Service Mode / 無 IC 狀態** 下執行，避免吸料/放料中段切入造成撞機
- 每組移動前自動回 home → 移到起點 → 計時跑到終點 → 回 home，**單筆獨立**
- 跑掃描期間禁止 Auto Start（程式內鎖 `bUPHProfilerRunning` 旗標）
- CSV 寫入失敗 / 磁碟滿時，立即停止掃描並 Alarm

---

## References

| 檔案 | 內容 |
|------|------|
| [`references/uph-existing-realtime-method.md`](references/uph-existing-realtime-method.md) | **機台既有 Per-Tray UPH 計算方式**：CalculateUPH 函式、bRecordUPH 觸發、CSV/SECS 記錄、UI 顯示 |
| [`references/uph-parallel-model.md`](references/uph-parallel-model.md) | 三大平行模型完整推導、邊界條件、特殊場景 |
| [`references/uph-stage-breakdown.md`](references/uph-stage-breakdown.md) | 11 大時間項對應的 cpp 函式、全域變數、Recipe 欄位 |
| [`references/uph-motor-time-table.md`](references/uph-motor-time-table.md) | MotorProfiler CSV 欄位、檔名規則、存放路徑、Excel 樞紐建議 |
| [`references/uph-formula-worked-example.md`](references/uph-formula-worked-example.md) | QFP 7×7、10×25 tray、2D off、TT=10s 完整算式 |
| [`references/uph-validation-cases.md`](references/uph-validation-cases.md) | 對照 UPH 詳解.xlsx 既有實測值，誤差 <±3% 驗證 |
