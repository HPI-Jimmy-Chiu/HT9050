# 給 Jimmy：京元電 HT9046LS 810_B06 的 type4 TFT（Bin 顯示器）程式碼，請合進 V912

來源：`D:\HT9045\工作原始碼\京元電\HT9046LS_Code_V3.32.810_B06_20261006KeyPro_02_AMR`（ES02，EastSun 20261006 要求整理）
比對對象：`HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（GitLab main 上的 V912）

type4 TFT ＝ `NUMBER_PANEL_TYPE == 4`（`General.ini [System] NUMBER_PANEL_TYPE`，database.cpp 讀；3＝彩色七段、4＝TFT），
另有 Magazine 的 `MAGAZINE_BIN_DISP_TYPE == eTFT`。

## ⚠ 不能整檔覆蓋

兩邊**各有對方沒有的改動**，整檔換會把一邊的修正蓋掉：

| 只有 810_B06 有（要合進 V912） | 只有 V912 有（要保留） |
|---|---|
| Eastsun 20260825：Mag Bin RGB（含新檔 BinRGBSetting.*、BinRGBTable.Data.sample） | Ifor 20260810：未初始化修正 |
| Eastsun 20260911：預設色改由 `GetBinRGB` 統一提供 | Ifor 20260819：initial bin font、CC_MAXIM_THAILAND 字樣 |
| Eastsun 20260917：修正 FIX123 數值不會變化 | Ifor 20260827：AOI fail bin 位置（藍色，code 7 / 4~6） |
| Eastsun 20260513／0514：閃爍功能（兩邊寫法不同，見 diff） | RogerYang 20250729 |
| Eastsun 20260929 | |

## 這個資料夾裡有什麼

| 路徑 | 內容 |
|---|---|
| `src/` | TFT 專屬模組的**原檔（Big5，一個位元組都沒改；`.gitattributes` 設 `-text` 不轉換換行）**：BinDisplay 整個資料夾、BinRGBSetting.cpp/.h/.dfm、BinRGBTable.Data.sample |
| `diff/` | 上面跟 V912 不一樣的檔，**V912 → 810_B06** 的 diff（已轉成 UTF-8 方便看；`-` 是 V912 才有、`+` 是 810_B06 才有） |
| `SNIPPETS.md` | 共用大檔（main.cpp、database.cpp、cmydef、cShowBinSelect、cSortCT、cBinSel、csystem、HandlerSys、myMN200motor、acatchtray、MachineType.h）裡 **type4 相關的那幾行**（前後 2 行），810_B06 和 V912 並排。這些檔兩個產品差很多，**只合 TFT 那幾行，整檔不要動** |
| `TABLE.txt` | 每個模組檔跟 V912 一樣／不一樣／新檔的清單 |

模組檔對照：

| 檔 | 大小（byte） | 跟 V912 |
|---|---|---|
| `BinDisplay/BinDisp.cpp` | 2418 | 一樣 |
| `BinDisplay/BinDisp.h` | 1552 | 一樣 |
| `BinDisplay/BinDisp.dfm` | 5097 | 一樣 |
| `BinDisplay/BinDispTester.cpp` | 800 | 一樣 |
| `BinDisplay/BinDispTester.bpr` | 3474 | 一樣 |
| `BinDisplay/MyBinDisp.cpp` | 129597 | **不一樣**：+154／-236 行 |
| `BinDisplay/MyBinDisp.h` | 12850 | **不一樣**：+9／-20 行 |
| `BinDisplay/MyBinDisp.dfm` | 1660 | **不一樣**：+2／-2 行 |
| `BinDisplay/MyBinDisp.dti` | 508 | 一樣 |
| `BinRGBSetting.cpp` / `.h` / `.dfm` | 8003 / 1737 / 2670 | **V912 沒有（新檔）**，要加進專案 .bpr |
| `BinRGBTable.Data.sample` | 318 | **V912 沒有（新檔）**，Bin 顏色表範例 |

## 建議合法

1. `BinRGBSetting.*`、`BinRGBTable.Data.sample` 直接加進 V912，`.bpr` 加上 `BinRGBSetting.cpp`。
2. `MyBinDisp.cpp` / `.h` / `.dfm`：照 `diff/` 一段一段看，`+` 的 Eastsun 0825～0929 段落合進來，`-` 的 Ifor 0810～0827 段落留著。
3. 共用大檔：照 `SNIPPETS.md` 對每個 type4 位置，V912 缺的才補。
4. BCB6 編譯（Big5，存檔不要轉成 UTF-8）。

## 沒有放進來的

- main.cpp、database.cpp、cmydef.cpp 等共用大檔的**整檔**：兩個產品其他部分差很多，整檔沒有合的意義；而且 main.cpp 有寫死的解鎖密碼，不放進 commit。要看全文請直接開上面的來源資料夾。
