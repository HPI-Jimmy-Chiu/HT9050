---
name: ht9050-hw
description: >
  HT9050（HP-9050，專案代號 PQMB056，客戶 台積電-龍潭）機台硬體規格表。
  當要查 HT9050 有哪些 IO 點位／馬達軸／溫控通道／離子設備／安全門／IPC 與 Comport 配置，
  或要把硬體資料表的欄位對回 HT9045 程式碼的常數（cmydef.cpp 的 Sn/C_/Sw/M 常數、
  MachineType.h 的 eTempControll / eIOType / eHeaterType）時使用。
  重點是 **溫控器對照表**：HT9050 用台達 DTM（DTME08×2 + DTMN08×1，3 站 24 通道），
  與 HT9045 的 eTempControll 之間需要轉換表，且 HT9045 既有的 iTempCode[] 不能直接沿用。
  觸發關鍵字：HT9050, HP-9050, PQMB056, 硬體規格, 元件代碼, 開發機資料,
  IO_Table_9050.csv, Mot_Table, 溫控器站號, DTM, DTME08, DTMN08, eTempControll,
  iTempCode, SLK, Direct Heater, 站別 TA/UL/ST/DH/HA/CB/TS/HP/PA/MS/RC/ES/BF/AL/CD/AO/US/CL,
  軸號 M0~M153, 安全門 SnSafeDoor, 離子槍, 離子Bar, PCIe-1203, MIC-7700。
applyTo: "**/IO_Table_9050.csv, **/Mot_Table*.csv, **/HP-9050*.xls, **/HP-9050*.xlsx"
---

# HT9050（HP-9050）硬體規格表

> ★ **機台現況（實際在跑的 IO_Table／Mot_Table／Gerneral.ini／teach.ini／config.ini／工單）不在這裡**：
> 在 GitHub `machine/integ-ioweb` 的機台快照（GitLab 鏡像 `machines/HT9050/snapshot/`）。懷疑是機台設定或工單問題時，
> 先跑 `python tools/machine_sync/machine_sync.py check`，沒同步就先同步再看（RULINGS_20261005 第 4 條；`AGENTS.md` 同名一節）。
>
> ⚠ **本 skill 是「硬體給的資料」的權威索引，不是機台現況**。
> 三份來源工作簿由硬體／電控填寫，程式端尚未全部接上。
> 每一節都分「**表上寫什麼**」與「**程式端目前是什麼**」兩欄，
> 兩者不一致的地方一律標成 ⚠ 缺口，不要當成已完成。

## 機台一句話

| 項目 | 內容 |
|------|------|
| 機台名稱 | HP-9050 |
| 系統模組 | Handler / AOI |
| 入電源 | 3Φ220V，19 KVA，主開關 50A |
| IPC | MIC-7700 + 4 Slot；Slot1 = PCIe-1203-32A（Ring0 軸卡／Ring1 IO 卡），Slot2 = PCI-GPIB |
| 視覺 | RTC×2 + AOI×3 + 2D（**沒有 tray map**） |
| 伺服馬達 | 20 軸 |
| 步進馬達 | 5 軸 |
| 溫控 | 台達 DTM，3 站 24 通道（DTME08×2 + DTMN08×1），全部 Ethernet |

## 來源資料（唯一真相）

五份原件都在本 skill 的 `docs/` 下，**不要改它們**——它們是硬體／電控交來的原件，
改了就對不回版本。要更新就整份換掉，並更新檔名日期。

| 檔案 | 誰填的 | 內容 | 詳見 |
|------|--------|------|------|
| `docs/HP-9050開發機資料-20260717.xlsx` | 硬體設計 | 9 個分頁：總覽／機構資訊（298 列）／馬達驅動器／加熱與感溫／**溫控器站號**／離子／入電源／IPC&Comport／選單設定 | [references/source-workbooks.md](references/source-workbooks.md) |
| `docs/HP-9050機構類元件代碼-20260922-2-軟體.xls` | 軟體視角 | 166 列，機構元件 ＋ **輸入／輸出線號** ＋ I/O 點位命名 ＋「命名來源」欄（軟體既有／建議新增） | 同上 |
| `docs/HP-9050機構類元件代碼-20260923-1-電控.xls` | 電控視角 | 同上，**只有 29 列的輸出線號不同**（`01xxx` → `0Bxxx`） | 同上 |
| `docs/IO_Table_9050.csv` | EastSun 20260923 | 1062 列，機台實際要載入的 IO 表；第 895 列 `#NEW_FROM_9050_DRAWING_20260923` 之後是 9050 新增段 | [references/io-table-9050.md](references/io-table-9050.md) |
| `docs/Mot_Table_9050.csv` | EastSun 20260924 | 48 列馬達表，`Enable=1` 的 19 軸全走 `PCI1203`；欄位順序與 9045 版不同 | [references/motors-9050.md](references/motors-9050.md) |

執行期路徑：`D:\HT9045\system\IO_Table_9050.csv`、`D:\HT9045\system\Mot_Table_9050.csv`（與 `docs/` 下的副本目前**內容相同**；repo 的 `core.autocrlf=true`，
clone 出來的 CSV 換行會變 CRLF，所以比對用內容不要用 hash）。

## 八份 reference

| 主題 | 檔案 | 什麼時候看 |
|------|------|-----------|
| **`Type_HT9050=800` 進入條件 ＋ JSON 介面** | [references/machine-type-entry.md](references/machine-type-entry.md) | 問「怎麼切到 HT9050」「C++ 要實作什麼」「機種 tag」 |
| 總覽、電源、IPC/Comport、離子、安全門數量 | [references/hardware-overview.md](references/hardware-overview.md) | 問「HT9050 有幾個 EMG／幾道門／哪個 COM 接什麼」 |
| IO 表結構與 9050 新增點位 | [references/io-table-9050.md](references/io-table-9050.md) | 改 IO、查點位、判斷某個 `Sn*`／`C_*` 存不存在 |
| 27 軸馬達與驅動器 | [references/motors-9050.md](references/motors-9050.md) | 查軸號、驅動器型號、煞車輸出、Mot_Table 缺哪幾軸 |
| **溫控器 DTM ↔ eTempControll 對照表** | [references/temp-dtm-map.md](references/temp-dtm-map.md) | 做溫控、看到 `iTempCode[]`、要加溫區 |
| 三份工作簿的分頁盤點與差異 | [references/source-workbooks.md](references/source-workbooks.md) | 硬體送新版來、要比對改了什麼 |
| **開發知識與極限規則**（Ifor01 1002：機型架構與通訊、溫控逾時／閾值／保護、開發與驗證規範；原本在 CLAUDE.md） | [references/dev-knowledge-and-limits.md](references/dev-knowledge-and-limits.md) | 做溫控、序列埠、1203、ctest 沙盒、寫測試之前 |
| **HT9050 與 HT9045 的差異（活紀錄）**：身分／軸卡／Index／飛梭／軌道料盤／手臂氣缸／安全／溫控／IO／畫面，每條附出處與狀態 | [references/ht9050-vs-ht9045.md](references/ht9050-vs-ht9045.md) | 問「HT9050 跟 HT9045 哪裡不一樣」、要寫「只有 HT9050 才改」的程式、Steven 又說了一條新差異要記下來（照該檔 §0 加一列） |

機器可讀的溫控對照表：[data/HT9050-TempMap.json](data/HT9050-TempMap.json)。

> ⚠ repo 的 `.gitignore:76` 有一條全域 `data/`，會把這個資料夾整個擋掉。
> 這個檔是用 `git add -f` 進版控的；**在 `data/` 底下新增檔案記得也要 `-f`**，
> 否則 `git status` 看不到、`git add -A` 也加不進去。

## Scripts

| 腳本 | 用法 |
|------|------|
| [scripts/dump_hw_workbooks.py](scripts/dump_hw_workbooks.py) | 把三份工作簿全部分頁 dump 成 UTF-8 純文字（丟到 scratchpad）。硬體送新版來、或要 diff 兩版時先跑這支 |
| [scripts/check_io_table.py](scripts/check_io_table.py) | 檢查 `IO_Table_9050.csv`：欄位數、Alias 重複、Enable 統計、ISABase／ModuleType 分佈、與 HT9045 `IO_Table.csv` 的差集、與 `cmydef.cpp` 常數名的對不上清單 |

兩支都要用 `d:\HT9045\.venv\Scripts\python.exe` 跑（PATH 上的 `python` 是 Store 殼，靜默 exit 49）：

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\HT9045\.claude\skills\ht9050-hw\scripts\check_io_table.py
```

## ⚠ 目前已知的六個程式端缺口

這四項是「表上有、程式端沒有」，動 HT9050 之前先確認狀態有沒有變。

1. **表的路徑沒有依機種切換**。`IoTablePath`（`common.cpp:232`、`database.cpp:1698`）
   與 `MotTablePath`（`database.cpp:1775`）都硬寫 9045 的檔名。
   `MotTablePath` 有 `W906_MOTTABLE_PATH` env seam 可當過渡，`IoTablePath` 沒有。
   **不要用複製覆蓋的方式**——那會讓 9045 的表消失且無警告。詳見
   [machine-type-entry.md](references/machine-type-entry.md)。
2. **`Mot_Table_9050.csv` 還缺 5 軸**。M99／M100／M101／M142／M151 沒有資料列
   （M108／M140／M141／M153 已補上）。常數都在，缺的是表。
   另有 3 個 alias 對不上硬體表：`MInRotate`／`MOutRotate`／`MInShutte1`。
3. **溫控沒有 HT9050 的通道表**。現有 `iTempCode[INDEX_HEAT_COUNT=32]`（`cmydef.cpp:111`）
   只涵蓋 Index 的 32 組加熱器，且排列是前後排交錯（Aa1,Ba1,Ab1,Bb1…）；
   HT9050 的 3 站 24 通道含 Hotplate／Shuttle／DUT／Chamber／Hot Air，
   **結構不同，不能沿用**。詳見 [temp-dtm-map.md](references/temp-dtm-map.md)。
4. **SLK-1~8 沒有專屬列舉**。建議映射到 `tcAa1+(n-1)`（見對照表），
   但這是推導值，**要 Jimmy／硬體確認 site 編號方向**再定案。
5. **93 個點位名字程式端沒有常數**。`IO_Table_9050.csv` 裡有 93 個 Alias
   在 `cmydef.cpp` 找不到對應的 `const int`（已扣掉 HT9045 既有的 13 個雜訊）。
   讀進來也接不到 `Sen[]`／`CY[]`／`SW[]`。跑 `scripts/check_io_table.py` 看當下清單。
6. **`Type_HT9050=800` 認得了，但沒有任何分派**。`MachineTypeChoice` 全樹 334 次，
   **0 處**比對 `Type_HT9050`。JSON 介面（`Machine-type-index.json` 的三個 tag）已定死並部署，
   C++ 端待實作。詳見 [machine-type-entry.md](references/machine-type-entry.md)。

## 站別代號（機構資料的第一欄）

| 代號 | 站別 | 代號 | 站別 |
|------|------|------|------|
| TA | Tray arm | ST | Shuttle（In／Out） |
| UL | Unloader（Loader/Auto1-3/Empty 抽屜） | DH | DUT Heater |
| HA | Hot Air gun | CB | Chamber |
| TS | Test Station（Index） | HP | Hot plate |
| PA / PA01 / PA03 | Pick&Place（臂／軌／吸嘴組） | MS | Machine Safety（門檢、支撐腳、氣源） |
| RC | Die clean / 負壓 | ES | 靜電消除 |
| BF | Buffer arm | AL | OTD 夾爪 |
| CD | RTC Real time check | AO / AO02 | AOI |
| US | Multi bin | CL | 2D 光源 |

## 相關 skill

- [ht9050-motionview-layout](../ht9050-motionview-layout/SKILL.md) — HT9050 版面／流程／軸綁定（HTML-only，`runtimeSupported:false`）
- [ht9050-uph-model](../ht9050-uph-model/SKILL.md) — HT9050 動作秒數與 UPH 模型
- [ht9045-io-control](../ht9045-io-control/SKILL.md) — `TMyCylinder`／`TMySensor`／`TMySucker` 控制層（IO 表怎麼被吃進去）
- [ht9045-motor-control](../ht9045-motor-control/SKILL.md) — 馬達控制層與 `Mot_Table.csv` 欄位
- [ht9045-html-json](../ht9045-html-json/SKILL.md) — 硬體設定檔怎麼送到 web HMI
