# HT9050 總覽：電源、IPC、通訊、離子、安全

來源：`HP-9050開發機資料-20260717.xlsx` 分頁 `專案資訊`／`總覽`／`05_離子相關`／
`06_入電源`／`07_IPC&Comport`。

## 專案

| 欄位 | 值 |
|------|-----|
| 機台名稱 | HP-9050 |
| 專案代號 | PQMB056 |
| 客戶／廠別 | 台積電-龍潭 |
| 系統模組 | Handler / AOI |
| 填表日期／版次／審核人 | **表上空白** |

## 系統與主要規格

| # | 項目 | 內容 | 備註 |
|:-:|------|------|------|
| 1 | 入電源 | 3Φ220V | 19 KVA，主開關 50A |
| 2 | IPC 電腦 | MIC-7700 | ＋4 Slot |
| 3 | 視覺電腦系統 | RTC×2 + AOI×3 + 2D | **沒有 tray map** |

## 安全／設備數量

| # | 項目 | 數量 | 備註 |
|:-:|------|:----:|------|
| 1 | 緊急停止按鈕 EMG | 4 | |
| 2 | 安全門檢 | 12 | |
| 3 | 三色燈 | 1 | **特規** |
| 4 | 蜂鳴器 | 1 | |
| 5 | 伺服馬達 | 20 | |
| 6 | 步進馬達 | 5 | |
| 7 | 離子風扇 | 0 | |
| 8 | 離子槍 | 6 | |
| 9 | 離子 Bar | 4 | |
| 10 | 4 吋散熱風扇 | 11 | |
| 11 | 大散熱風扇（220V） | 1 | |
| 12 | EP | 1 | **有 ATC 時為 5 EP** |
| 13 | 真空泵浦 | 0 | 預留 |
| 14 | 螢幕 | 2 | |
| 15 | 鍵盤 | 1 | |
| 16 | 滑鼠 | 2 | |
| — | 日光燈 | — | **特規** |

### RD 需確認事項（硬體在表上留給 RD 的問題，未結案）

1. 問 INDEX 管排長度
2. 問 CHAMBER 加熱的 U 型管保險絲
3. 問 6B040S-2N 馬達 + SBD040-2 是否有認證

## 安全門

`01_機構資訊` 站別 `MS`／`TS`，感測器 `P030050228`：

| 點位 | 位置 | 型號 |
|------|------|------|
| `SnSafeDoor1` | 前方左拉門 | GS-ML51P |
| `SnSafeDoor2` | 前方右拉門 | GS-ML51P |
| `SnSafeDoor3` | 後方左拉門 | GS-ML51P |
| `SnSafeDoor4` | 後方右拉門 | GS-ML51P |
| `SnSafeDoor5` | 後方右掀門 | GS-ML51P |
| `SnSafeDoor6` | 左方掀門 | GS-ML51P |
| `SnSafeDoor7` | 右方掀門 | GS-ML51P |
| `SnSafeDoor8` | Index 開檢 | GS-ML5**1N** |

⚠ `SnSafeDoor8` 用 `GS-ML51N`（NPN），其餘 7 顆是 `GS-ML51P`（PNP）。

另有 5 個抽屜門檢用 **RFID 唯一編碼安全開關** `P030011575`（RSAFE BASIC C M U 1）：
`SnLoaderDrawerDoor`／`SnAuto1DrawerDoor`／`SnAuto2DrawerDoor`／`SnAuto3DrawerDoor`／
`SnEmptyDrawerDoor`。7 + 1 + 5 = 13，與總覽的「安全門檢 12」對不上一個，
**要問硬體是哪一顆不計入**。

門栓輸出統一一點：`SwSafeDoorLock`（`00122`）。

## 離子設備（`05_離子相關`）

| 類型 | 廠商 | 型號 | Close loop | 通訊 | 位置 |
|------|------|------|:----------:|------|------|
| 離子槍 | SMC | IZN10E-1106 | **Y** | Ethernet | Chamber type |
| 離子槍 | SMC | IZN10E-1106 | N | — | Chamber/ATC type ×2 |
| 離子槍 | SMC | IZN10E-1106 | N | — | Hot plate |
| 離子槍 | SMC | IZN10E-1106 | N | — | air blow |
| 離子槍 | SMC | IZN10E-1106 | N | — | Die clean |
| 離子 Bar | SSD | HPE-B05C-1070 | Y | RS-485 | Loader |
| 離子 Bar | SSD | HPE-B05C-734 | Y | RS-485 | Unloader |
| 離子 Bar | SSD | HPE-B05C-734 | Y | RS-485 | In Shuttle |
| 離子 Bar | SSD | HPE-B05C-734 | Y | RS-485 | Out Shuttle |
| 離子中和器 | KASUGUA | ND-503T-HT3 | N | — | Chamber |

對應點位 `SnIonFanPower01`–`09`、`SnLoadIonGun`、`SnIonBarrierAlarm`、
`SnIonBar1`–`SnIonBar8`（輸入 `11130`–`11137` = ION ALARM1–8）。
離子 Bar 控制器經 COM6 → 4520 轉接。

## IPC 與擴充槽（`07_IPC&Comport`）

電腦本體 **MIC-7700 + 4 Slot**。

| 介面 | 功能 | COM |
|------|------|-----|
| 內建 485-1 | BIN | COM1 |
| 內建 485-2 | Multi Bin | COM2 |
| 內建 232-1 | 板金 RS232 孔 | COM3 |
| 內建 232-2 | 通訊面板（ELC-001） | COM4 |
| 內建 232-3 | RTC | COM5 |
| 內建 232-4 | SSD 離子 Bar 控制器轉 4520 | COM6 |
| 內建 LAN1 | 板金 LAN 孔 | — |
| 內建 LAN2 | 16 port Hub | — |

| Slot | 卡 | 型號 | 備註 |
|:----:|----|------|------|
| 1 | 軸卡 | **PCIe-1203-32A** | Ring 0（右邊）；Ring 1 走 IO 卡（左邊） |
| 2 | GPIB | PCI-GPIB #778032-01 | |
| 3 | 預留 1 | | |
| 4 | 預留 2 | | |

### Ethernet Hub（DGS-1016D，16 port）

| Port | 接什麼 | Port | 接什麼 |
|:----:|--------|:----:|--------|
| 1 | Handler IPC 內建 LAN2 | 9 | ION-Simco 3362 #2（預留） |
| 2 | **DTM #1** | 10 | — |
| 3 | **DTM #2** | 11 | — |
| 4 | — | 12 | — |
| 5 | EP | 13 | 2DID PC 內建 LAN1 |
| 6 | — | 14 | ATC IPC 內建 LAN1 |
| 7 | Safety PLC | 15 | RTC IPC 內建 LAN1 |
| 8 | ION-Simco 3362 #1（預留） | 16 | AOI IPC 內建 LAN1 |

⚠ Hub 上**只規劃了 DTM #1 與 DTM #2 兩個 port**，但
[溫控器對照表](temp-dtm-map.md) 是 **3 站**（DTME08×2 + DTMN08×1）。
第 3 站要接哪個 port 表上沒寫——**要問硬體**（空的 port 4／6／10–12 都可用，
或 3 站之間串接）。

## AOI 與光源

| 元件 | 品號 | 位置 | 通道 |
|------|------|------|:----:|
| 定電流倍頻 RS232 電源控制器 ×2 | E010040078 | 上罩 | 8CH，Top Bottom AOI |
| 定電流倍頻 RS232 電源控制器 ×2 | E010040078 | 上罩 | 8CH，Bottom AOI |
| 定電流倍頻 RS232 電源控制器 | E010040076 | 上罩 | 4CH，4S AOI |
| 定電流倍頻 RS232 電源控制器 | E010040076 | 上罩 | 4CH，2D |

## 入電源

| 系統／區域 | 規格 | 容量 | 主開關 |
|------------|------|------|--------|
| Handler | 3Φ220V | 19 KVA | 50A |
