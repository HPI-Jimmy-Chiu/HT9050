# 給機台端 Claude：更新包 11（GitLab main `10936285`，相對更新包 10 `4c067b5f`）

> 筆電端 Claude 20260926 17:4x 產生。**先套更新包 3～10，再套這一包。要不要套由 Jimmy 決定。**
> 8 檔，底稿 `base_4c067b5f\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：你們 IOWEB-P17 回報的「OutPortData 越界、14 對輸出共用快取位元」修掉了

Jimmy 20260925 裁決「要修」（RULINGS_20260925 第 18／29 條，授權偏離 golden）。

| 檔 | 改了什麼 |
|---|---|
| `MyLaneIo.h:60`／`:76`／`:78` | 輸出命令快取 `OutPortData`／`BackOutPortData` 從 golden 的 `[4][64][4]` 放大到 `[4][256][32]`（`LANEIO_CACHE_IP`／`LANEIO_CACHE_PORT`） |
| `MyLaneIo.cpp:154` | `InitialMyOutIOData` 開頭整塊清快取 |
| `MyLaneIo.cpp:654-655` | `BackUpOutputData` 備份整個快取（`RestoreOutputData` 只還原 MotionNet 的點，照 golden 不變） |
| `MyLaneIo.cpp:735` | `CheckPortRangeErr`：**1203 的輸出**（`DO_Type==true`）超出新快取就回 2（跟 MotionNet 越界同一個錯誤碼與處理）。**輸入不檢查**（照 golden）——你們表上真空的 `Sucker` 列 Port 是 128～135 |
| `MachineType.h:1766` | 那段警告只加註「已修」（註解） |
| `tests/test_lane_io_route.cpp` [5]、`test_sim_io.cpp:230-231`、`test_lane_io_sim.cpp:140-141` | [5]＝14 組逐一驗證互不影響；另兩支原本斷言「1203 一律不檢查」，改成輸入照舊、輸出超出回 2 |

**MotionNet 的規則一字未動**（範圍仍是 golden 的 64／4，打錯位置的警告照 golden）。

### 為什麼是 14 組

用 `machines/HT9050/IO_Table.csv` 的 206 列 1203 輸出（Switch／Cylinder_On／_Off／Sucker_On／_Off）算：
golden 快取的格子是 `Lane*256 + IP*4 + Port`（同一個 Bit），有 14 組不同點位落在同一格，例如 **(IP 80, Port 17, Bit 1) 與 (IP 84, Port 1, Bit 1)**。
修正前開其中一個，另一個的「是否已開」（`SW[].Status()`、氣缸 `GetOutBit()`）也會讀成開；卡片本身的讀寫一直是對的。
⚠ 筆電這份表是 Lane 0 的舊版，你們現場表（Lane 1）請用現場表再算一次，應該同樣是這 14 組（格子公式跟 Lane 無關）。

## 在機台上要看的（輕量）

1. **全量重編**（`MyLaneIo.h` 改了 TLaneIO 的大小）。開機、IO 頁照常。
2. ctest `LaneIORoute` 通過（[5] 那 14 組）。⚠ **IO 頁看不出這個差別**：IO 頁顯示的是卡片回讀（`MachineType.h:1766` 下兩行），
   快取只影響引擎自己的判斷（`SW[].Status()`、氣缸 `GetOutBit()`），所以不要用「IO 頁燈號沒跟著亮」當驗證。
3. log 沒有新的「Maybe input wrong IO position!」（表上所有 1203 輸出都在新快取裡；真的出現代表那一列的 IP≥256 或 Port≥32）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證（兩組態全量 gate）：出貨＝基準 3＋5 Disabled、模擬＝基準 18＋5 Disabled；`LaneIORoute` [5] 修正前紅 16 條（對照組）、修正後 96／96；
子檢查只有重新校準的那兩條不同，其他 190 支 0 差異；system\ config\ 586 檔 0 變動。
