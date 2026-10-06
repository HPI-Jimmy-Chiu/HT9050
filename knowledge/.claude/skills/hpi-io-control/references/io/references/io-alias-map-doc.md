> 保存來源：`.claude/skills/ht9045-io-control/references/io-alias-map-doc.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# IO 畫面 Alias 對照文件生成（iosetview Alias Map）

從 `iosetview.dfm` 解析全部 `TMyLedLane` / `TBtnPanelLane` 元件，
在 `D:\HT9045\IMG\IO\` 的 1:1 截圖上疊加紅框標註，
生成自含式 HTML 對照文件（截圖以 base64 內嵌，可單檔攜帶）。

## 產出物

| 項目 | 路徑 |
|------|------|
| 生成腳本 | `scripts/gen_io_alias_doc.py`（本 skill 內） |
| 輸出文件 | `<入口網站 repo>\public\Docs\manual\HT9011UC_IOSetView_Alias_Map.html` |
| 截圖來源 | `D:\HT9045\IMG\IO\<tabsheet名稱>.png`（938x938、1:1 未縮放） |
| DFM 來源 | `D:\HT9045\HT9011UC_Code_V3.33.908.0_20260702\iosetview.dfm` |

## 執行方式

```powershell
py d:\HT9045\.github\skills\ht9045-io-control\scripts\gen_io_alias_doc.py
```

輸出 `pages: {...}` 各頁元件數與 `total:` 合計（V3.33.908.0 為 1846 個）。

## 運作原理

1. 解析 DFM 樹狀結構（cp950 編碼），跳過二進位/括號區塊。
2. 遞迴累加各層容器偏移計算元件絕對座標：
   - `TGroupBox`：`(2, Font.Height*1.2+3)`
   - `TPanel`：`BorderWidth + Bevel` 數
   - `TPageControl` client 區：`PGC_ADJ` 表（頂層 `PC_IOSET` 為 `(4,51)` 兩列頁籤；其餘 `(4,25)`）
3. 全域偏移：`BORDER_X=8`（視窗左框）、`TITLE_Y=31`（標題列）。
4. 依「最深且有截圖的 TTabSheet」分組；`pgcTrayArm` 子頁映射到 `tsOtherVacuum`（截圖僅顯示 RT Arm，其他子頁元件以灰字標示「截圖未顯示」）。

## 座標微調（四個層級）

腳本開頭有四組校正常數，由粗到細，**同一元件命中多層時效果疊加**：

```python
# 1. 全域（所有頁一起動）
BORDER_X = 8
TITLE_Y  = 31

# 2. 頁級：該頁全部標註框整體平移 (dx, dy)
PAGE_ADJ = {
    "tsShuttle": (-3, -13),
}

# 3. 編號區間級：依頁內表格編號 # 範圍平移 (no_from, no_to, dx, dy)
#    注意：編號是依 (y, x) 排序自動編的，位移後編號不會重排
#    （編號來自 DFM 原始座標，校正量不影響排序）
PAGE_NO_ADJ = {
    "tsInArmVacuum": [
        (1, 73,  0, 4),     # #1~#73 下移 4px
        (74, 75, -1, 5),
    ],
}

# 4. 元件級：單一元件 (dx, dy) 或 (dx, dy, w, h) 連框大小一起改
COMP_ADJ = {
    "myldln1": (0, -10),          # 以元件名稱為 key（非 Alias）
}
```

改完重跑腳本即可，瀏覽器重新整理驗證。

### 微調工作流程（重要經驗）

1. **使用者回報的位移量是「相對於目前顯示」的增量**，修改表時要與現值**累加**，
   並在註解中記錄歷程（例：`# prev -1,-10, this time +5`）。
2. 「向下/向右」= 正值，「向上/向左」= 負值。
3. 若回報「某頁其餘元件向 X、指定幾個元件向 Y」：把共同量收進 PAGE_ADJ，
   指定元件用 PAGE_NO_ADJ 補回差額（淨效果 = 頁級 + 區段級）。
4. 驗證方式：重跑腳本 → 瀏覽器 reload → 對 `#w_<頁名>` 元素截圖比對。

### 使用者回報偏移的建議格式

- 頁級：「`tsIndex` 整頁往右偏 5px」→ `PAGE_ADJ` 累加 `(-5, 0)`（框偏右就往左修）
- 區段級：「`tsInArmVacuum` 的 #74~#97 向上 10px」→ `PAGE_NO_ADJ` 累加 `(0, -10)`
- 元件級：用 Alias（如 `SnMCUSensor1`）回報時，先在 DFM 反查元件名稱再填 `COMP_ADJ`
- 也可直接說「Index 頁 EP Switch 1 按鈕的框」用 Caption/Alias 描述，由表格反查元件名稱

## IO_Table.csv 整合（2026-07-06 新增）

HTML 內建 CSV 檢視引擎，將 IO 點位資訊顯示於三處：
表格「IO 點位」「時序(Output)」欄、標註框 tooltip、各區段統計列。

### IO 點位 5 碼規則

| 碼 | 來源 | 規則 |
|----|------|------|
| 1 | IOType | **I** = Sensor / Cylinder_On / Cylinder_Off / **Sucker**；**O** = Switch / Cylinder / **Sucker_On / Sucker_Off** |
| 2 | Lane | 0~3 |
| 3 | IP | 0~9 直接顯示，10=A、11=B…（Base36 大寫，32=W） |
| 4 | Port | 0~3 |
| 5 | Bit | 0~7 |

注意：Sucker（真空感測）是 input，Sucker_On/Off（電磁閥）是 output——與 Cylinder 方向相反。

### 資料載入機制（方案 A）

- 預設使用**生成時內嵌**的 `D:\HT9045\system\IO_Table.csv`（cp950 讀取）
- 使用者可按「載入 IO_Table.csv」或**拖放 CSV** 到頁面 → 存入 localStorage，重新整理仍沿用
- 「還原內建資料」清除外部載入
- 腳本會複製 CSV 到 `<入口網站 repo>\public\Docs\manual\IO_Table.csv`（僅在不存在時，不覆蓋使用者編輯）
- 重複 Alias 全部列出；Enable=0 顯示刪除線；Lane/IP 空白顯示「未配置」；查無 Alias 顯示「無資料」
- 時序欄（警報 On/Off ｜ 延遲 On/Off）僅 Output 類型顯示

## 已知限制

- 截圖必須是 1:1（DPI 100%）擷取；整窗截圖 938x938，區域截圖尺寸須等於錨點容器尺寸。
- `tsSafe`（Safe PLC 頁）的 64 顆燈是 `TALed`（aLedSPLC000~057），非目標型別故無標註；
  其感測器對應由 `InitPairInfo_SafePLCIOLed()` 在程式端配對 PLC 位址。
- 動態建立/搬移的元件（如 `FormShow` 內調整 Top/Left 者）標註位置以 DFM 設計值為準。
- **Runtime reflow 頁面**：`tsSystem` 最嚴重——頂/底 EMG 列（`Align=alTop/alBottom`）、
  左右安全門直條、右側狀態欄的 runtime 位置與 DFM 差 20~33px，
  須靠 `PAGE_NO_ADJ` 分區校正（現行腳本已內建校正值，勿隨意清除）。
- `btnSwEpArm2`（舊 tsIndex #116）已依需求排除（腳本 `EXCLUDE_COMPS`）。
- 換版時更新腳本內 `DFM` 路徑常數即可重新生成；**既有校正表可沿用**
  （除非 UI 佈局改版，否則各頁相對位置不變）。
- 校正現況（2026-07-06）：全部 26 頁（含區域截圖頁 grpManual / TrayArm / RTArm / UnderArm /
  Cassette x2 與 TTL）已人工逐頁校正，校正值全部存於 `gen_io_alias_doc.py` 的
  PAGE_ADJ / PAGE_NO_ADJ / COMP_ADJ。

<!-- preserved-content:end -->
