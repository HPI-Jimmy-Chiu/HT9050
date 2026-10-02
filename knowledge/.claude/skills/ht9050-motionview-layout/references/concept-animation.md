# 概念頁：純模擬動畫版（`page/IDE.MotionView9050-Concept.html`）

> 目的：**不載入任何 JSON、不依賴 theme.js／settings.js**，雙擊即開，用來確認版面、
> 流程與互鎖是否符合硬體人員的認知。所有數值都是示意，不可拿去當教點或工程座標。

## 1. 檔案結構（單檔，由上而下）

| 區塊 | 內容 |
|------|------|
| `<style>` | 自帶完整色票（`--bg/--panel/--ink/--dim/--line/...`），**不可**再引 `theme.css` |
| `MODS` | 各模組方塊 `[x, y, w, h]`（mm，rect 原點左上、y 向下） |
| `ST` | 站點與行程常數（動畫用），見 §3 |
| `TRAY` / `HPF` | Loader 盤形 / Hot plate 盤形（示意，預設 2×1 / 2×2） |
| `drawIso()` | 靜態機構 + 動態元件（`D.*`）建立 |
| `drawHead()` | 吸嘴 inset（含 `D.headIC` / `D.headState` 持料指示） |
| `buildPanels()` | 下方欄位骨架（Loader Tray、HotPlate、Bin Tray 分頁、KIT、軸表、動作記錄、chips） |
| `STEP_DEF` / `buildOps(dur)` | 12 步的預設秒數與排程：主鏈（從 `SHT_IN` 起）順序執行；`IN_KIT` 從 `SHT_BACK` 結束起、`IN_PICK`/`IN_HP` 接續，全部與主鏈平行；產生 `OPS`（依 a 排序）、`K`（key→op）、`MAIN`（主鏈）、`CYCLE`、`PRIME_LEN`、`KIT0_LEN` |
| `hashParams()` | 讀 URL hash：`#IN_KIT=3&TEST=10&hpc=2&hpr=2&trc=2&trr=1` 覆寫秒數與盤形（UPH 頁用） |
| `drawTL()` | 時序軌（Hot plate 軌另畫一條「soak 進行中」跨 cycle 底條） |
| 位置函式 | `inArmAt / ishtXAt / oshtXAt / idxZAt / outArmAt / oshtYAt / icAt / icBAt / icCAt`，全部用 `K.<key>.a/.b/.d` 查表，**不用數字索引** |
| `applyPanels(idx,inPh)` | 每換相位才呼叫；`idx`＝主鏈索引（非正式 cycle -1），`inPh`＝In P&P 子相位（0 IN_KIT/1 IN_PICK/2 IN_HP/3 待命） |
| `render()` | 每幀；只搬動 SVG 元件位置 |
| `loop()` / 按鈕 | 播放引擎、單步、重置、TOP AOI 勾選 |

## 2. 12 步時序（`STEP_DEF` → `buildOps()`）

```
主鏈（測區） SHT_IN → IDX_PICK → SHT_BACK → TEST → OSHT_IN → IDX_PLACE → OSHT_OUT → OUT_PICK → OUT_PLACE
In P&P 軌                            └─ IN_KIT → IN_PICK → IN_HP（In Shuttle 回到入料側就開始，與 TEST… 平行）
```

> **cycle 起點（SHT_IN）時 In Shuttle KIT 上已經有料**（上一輪 IN_KIT 放的）。In P&P 不等下一輪，
> In Shuttle 一回家（`SHT_BACK.b`）就把 Hot plate 最舊一顆放上 KIT，再去 Loader 補料。
> `CYCLE = max(OUT_PLACE.b, IN_HP.b)`。

| key | 機構 | 意義 |
|-----|------|------|
| `IN_KIT` | In P&P | 從 Hot plate 取 **最舊（已達 soak）** 那顆 → In Shuttle KIT |
| `IN_PICK` | In P&P | → Loader → Z 下降 → **82% 處吸起** |
| `IN_HP` | In P&P | → Hot plate → Z 下降 → **步尾放下**（補回剛空出的位） |
| `SHT_IN` | In Shuttle | X 進測區 |
| `IDX_PICK` | Index | Z 下降吸取 KIT 上的 IC |
| `SHT_BACK` | In Shuttle | X 出測區＋Line sensor |
| `TEST` | Index | Z 下壓 DUT |
| `OSHT_IN` | Out Shuttle | X 進測區（Z sensor 確認空 KIT） |
| `IDX_PLACE` | Index | Z 下降放到 Out Shuttle KIT |
| `OSHT_OUT` | Out Shuttle | X 出測區 → **Y 往前微降到取料位** |
| `OUT_PICK` | Out P&P | Y 懸臂縮回、Z 下降取料 |
| `OUT_PLACE` | Out P&P | Y 懸臂伸出、X 移到 Auto{n}、Z 下降放料 |

### 三階段：暖機（prime）→ kit0 → 正式

```js
HP_CAP  = HPF.rows*HPF.cols          // Hot plate 盤位數
PRIME_C = HP_CAP                     // 暖機 cycle 數：補到滿
phase()     → 0 暖機 (CYC<PRIME_C) / 1 kit0 (CYC===PRIME_C) / 2 正式
cycleLen()  → PRIME_LEN / KIT0_LEN / CYCLE
opAt(t)     → 非正式回 -1；否則回 MAIN 索引
inT(t)      → 暖機 t+K.IN_PICK.a；kit0 t+K.IN_KIT.a；正式 t
inPhaseAt(t)→ 0 IN_KIT / 1 IN_PICK / 2 IN_HP / 3 待命
kitDone(t)  → inT(t) >= K.IN_KIT.b（C 已在 In Shuttle KIT 上）
delivered() → max(0, CYC-PRIME_C-1)     // 已進 Auto 盤的顆數
kittedBefore() → max(0, CYC-PRIME_C)   // 本 cycle 開始前已從 Hot plate 取走的顆數
```

- **暖機**：只跑 IN_PICK / IN_HP，`icAt()`/`icCAt()` 回 `null`，測區全部待命。
- **kit0**：Hot plate 已滿，做第一次 IN_KIT（取最舊一顆放 KIT）再 IN_PICK/IN_HP 補滿；測區仍待命。
- **正式**：起點 KIT 上有料、Hot plate 滿；SHT_BACK 結束後 In P&P 立即 IN_KIT 下一顆。

## 3. 站點與行程常數（`ST`）

| 鍵 | 用途 |
|----|------|
| `SHT_Y` | 兩台 Shuttle 的基準 Y |
| `RAIL_Y_IN` / `RAIL_Y_OUT` | In / Out P&P X 長軌的 Y |
| `Z_GANTRY / Z_ARM_SAFE / Z_KIT / Z_DUT` | 各高度 |
| `Z_IDX_RAIL / Z_IDX_SAFE` | Index 導軌高度／安全高度 |
| `OSHT_Y_PICK` | Out Shuttle 出測區後 **往前微降** 量（取料位） |
| `OSHT_Y_STROKE` | TOP AOI（Option）往後掃描行程 |
| `OUT_CYL {back, front}` | Out P&P Y 懸臂氣壓缸的兩段停點（縮回＝Out Shuttle 側／伸出＝Tray 側） |
| `LOADER / HP / ISHT / OSHT / TEST / AUTO[]` | 各站 XY |

## 4. 料件表示規則（畫面 ↔ 盤點必須一致）

**同一顆料在畫面上只能出現一次**：

| 位置 | 畫面表示 | 盤點欄位 |
|------|----------|----------|
| Loader 盤內 | Tray Status Unit 格點（藍） | `Loader` chip、`盤內 n / N` |
| In P&P 吸嘴 | 大 IC 圓（`D.ic2` 或 `D.ic3`）跟著臂走 ＋ inset 持料方塊 | `In P&P` chip、`真空 ON（持料）` |
| Hot plate | `D.dHP[i]` 格點亮（`--s0`），最舊一顆紅圈 `sk` | `Hot plate` chip、`料 n / HP_CAP`、`已達 soak` |
| In Shuttle KIT | 大 IC 圓在 KIT 上 | `In Sht KIT` chip、KIT 卡 `has` |
| Index 持料 / DUT | 大 IC 圓跟 Index Z | `Index` / `DUT` chip |
| Out Shuttle KIT | 大 IC 圓（跟 Out Shuttle X／Y） | `Out Sht KIT` chip |
| Out P&P 吸嘴 | 大 IC 圓（綠＝已測）＋ inset 持料方塊 | `Out P&P` chip |
| Auto 盤 | Tray Status Unit 格點（綠） | `Auto1..3` chip、`盤內 n` |

⚠ **Hot plate 上的料由格點表示，`icBAt()` 在料放上 Hot plate 後、`icCAt()` 在 IN_KIT 開始前必須回 `null`**，
否則會出現「格點亮了又多一顆大 IC」的雙重顯示（已踩過）。

### A / B / C 三顆料

- **A**（`D.ic`）：本輪送下線的那顆。cycle 起點就在 In Shuttle KIT 上 → Index → DUT → Out Shuttle → Out P&P → Auto。
- **B**（`D.ic2`）：本輪剛從 Loader 取起、要補進 Hot plate 的那顆。格位＝`(taken-1) % HP_CAP`。
- **C**（`D.ic3`）：下一顆。IN_KIT 從 Hot plate 最舊格 `cCell() = kittedBefore() % HP_CAP` 取走 → 吸嘴 → In Shuttle KIT 上等到 cycle 結束（下一輪就是 A）。
- `applyPanels()` 的 `first = kitted + 1`，`kitted = kittedBefore() + (C 在吸嘴或已在 KIT ? 1 : 0)`；`first-1` 必須等於下一個 `cCell()`。

### 取放時序（不可提早／延後）

| 事件 | 觸發點 | 效果 |
|------|--------|------|
| C 離開 Hot plate | `IN_KIT` 開始（＝`SHT_BACK.b`，`inPh===0`） | `first = kittedBefore()+2`；`icCAt()` 回格位→臂位置 |
| C 放上 In Shuttle KIT | `IN_KIT` 結束（`kitDone()`） | `v[3]++`；KIT 卡 `has`；`icCAt()` 回 KIT 位置 |
| Loader 扣 1 | `inT(T) >= K.IN_PICK.b`（IN_PICK **完成**；不可用 `inPh>=2`，因 IN_KIT 之前也是待命 3） | `taken = CYC + 1` |
| B 出現在吸嘴 | `IN_PICK` 82%（Z 到底） | `icBAt()` 回臂位置 |
| B 併入 Hot plate | `IN_HP` 結束（`inPh === 3`） | `icBAt()` 回 `null`；`last = taken` |

## 5. 盤點守恆（`applyPanels`）

```
LOT = PRIME_C + 1 + AUTO_CAP      // Hot plate 滿 + KIT 上 1 顆 + 3 個 Auto 盤（AUTO_CAP = TRAY 格數 × 3）
v = [loaderLeft, inArm, hpCount, inSht, index, dut, outSht, outArm, auto1, auto2, auto3, 0]
Σv 必須恆等於 LOT；最後一個 cycle = LOT-1
```

- `loaderLeft = LOT - taken`
- `hpCount = last - first + 1`
- A 用 `MAIN[idx].posA` 定位（`v[posA]++`）；B 在吸嘴（`inPh===2`）時 `v[1]++`；C 在吸嘴（`inPh===0`）`v[1]++`，已在 KIT `v[3]++`
- Loader 盤格：`trayLeft = min(loaderLeft, n - ((taken-1) % n) - 1)`，取滿 n 顆由 Tray Arm 換盤

## 6. 驗證腳本

在 DevTools console 貼上 `scripts/verify-concept.js`（或用 Playwright `page.evaluate`）：

- 每 cycle 取 60 個時間點（In P&P 軌與測區軌平行，不能只看主鏈索引）
- chips 合計 = `LOT / LOT`
- Hot plate 亮格數 = `cntHP` 卡片數 = `Hot plate` chip
- 大 IC 不得落在已亮的 Hot plate 格點上
- 暖機最後一輪末端 Hot plate = `HP_CAP-1`（B 仍在吸嘴）；kit0 開始 Hot plate = `HP_CAP-1`（C 立刻被取走）
- 每個正式 cycle 起點：Hot plate = `HP_CAP`（滿）、In Shuttle KIT `has`
- `pageerror` = 0

## 7. 調整常見需求

| 需求 | 改哪裡 |
|------|--------|
| 換 Tray / Hot plate 盤形 | `TRAY` / `HPF`，或 hash `trc/trr/hpc/hpr`（LOT、PRIME_C 自動跟著變） |
| 換某模組位置／大小 | `MODS` |
| 改某步秒數 | `STEP_DEF[].d`，或 hash `#KEY=秒` |
| 加一步流程 | `STEP_DEF` 新增（含 `posA`）→ `buildOps()` 的 `chain` 插入 → 位置函式用 `K.<key>` 接上 |
| 改 Out Shuttle Y 微降量 | `ST.OSHT_Y_PICK`（同時改 `ST.OUT_CYL.back`） |
| 開關 TOP AOI 掃描 | 工具列勾選 → `OPT_TOP_AOI` |
