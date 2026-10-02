# GPIB Command Manual（GPIB 通訊指令手冊）

## 適用時機
- 使用者要求產生 HT-9xxx GPIB Communication Command Manual
- 更新或修訂既有 GPIB 手冊（EN / ZH ）
- 新增指令章節或修正 Site Map 布局描述

---

## 開始前的詢問清單

在生成手冊前，請先向使用者確認以下問題：

| # | 問題 | 預設值 | 說明 |
|---|---|---|---|
| 1 | **版本類型**：標準版（Standard）或完整版（Full/Customer-Specific）？ | Standard | 完整版含客製指令（ART、Hana、Qorvo 等）|
| 2 | **指定客戶**？ | 無 | 影響是否加入客製章節（Qorvo CONFIGURE、Hana TDATA 等）|
| 3 | **指定功能**？（可複選）| 無 | ART/Auto-Retest、溫度控制/Temperature、SECS/GEM、2D Barcode 等 |
| 4 | **手冊語言**？ | 英文（EN） | 可選：English / 繁體中文 |
| 5 | **GPIB 程式版本**？ | 最新版 | 例：V12.13.905.0，路徑：`d:\GPIB9045\GPIB_Code_32Site_Vxx.xx.xxx.x_YYYYMMDD` |

---

## 輸出路徑與命名

### 命名規則
```
{YYYYMMDD}_GPIB_Command_Manual_{LANG}_{Edition}_V{版本號}.md
{YYYYMMDD}_GPIB_Command_Manual_{LANG}_{Edition}_V{版本號}.html
```

- `{YYYYMMDD}`：產生日期，例 `20260624`
- `{LANG}`：語言代碼 `EN` 或 `ZH`
- `{Edition}`：
  - `Standard`：標準通用版
  - `Full`：完整版（含全部客製章節，非針對特定客戶）
  - `{客戶名稱}`：客戶專版，直接使用客戶名稱，例 `Qorvo`、`Hana`、`TSMC`
- `{版本號}`：對應 GPIB 程式版本，例 `V12.13.905.0`

**範例：**
```
20260624_GPIB_Command_Manual_EN_Standard_V12.13.905.0.md
20260624_GPIB_Command_Manual_EN_Standard_V12.13.905.0.html
20260624_GPIB_Command_Manual_ZH_Full_V12.13.905.0.html
20260624_GPIB_Command_Manual_EN_Qorvo_V12.13.905.0.html
20260624_GPIB_Command_Manual_EN_Hana_V12.13.905.0.html
```

### 輸出目錄
```
<repo>\public\Docs\manual\GPIB_Manual\
```

### HTML 轉換指令
```powershell
python "d:\.github\skills\make-report-skill\scripts\md_to_html.py" `
    "<repo>\public\Docs\manual\GPIB_Manual\{YYYYMMDD}_GPIB_Command_Manual_EN_{Edition}_V{版本號}.md" --template blue
python "d:\.github\skills\make-report-skill\scripts\md_to_html.py" `
    "<repo>\public\Docs\manual\GPIB_Manual\{YYYYMMDD}_GPIB_Command_Manual_ZH_{Edition}_V{版本號}.md" --template blue
```

---

## 標準章節結構（18 章）

| 章節 | 英文標題 | 中文標題 |
|---|---|---|
| 1 | Introduction | 簡介 |
| 2 | Hardware Connection & Address | 硬體連接與位址 |
| 3 | Communication Protocol Overview | 通訊協定概述 |
| 4 | System Control Commands | 系統控制指令 |
| 5 | Lot Control Commands | 批次控制指令 |
| 6 | SRQ (Service Request) Flow | SRQ 服務請求流程 |
| 7 | BINON / BINOFF — Bin Assignment | BINON / BINOFF — Bin 分配 |
| 8 | FULLSITES — Active Site Mask | FULLSITES — 啟用站點遮罩 |
| 9 | ECHO / ECHOOK / ECHONG — Echo Confirm | ECHO / ECHOOK / ECHONG — 回音確認 |
| 10 | Site Map Commands | Site 對應指令 |
| 11 | Bin Map & Yield Control | Bin 對應與良率控制 |
| 12 | Temperature Control Commands | 溫度控制指令 |
| 13 | 2D Barcode Commands | 2D 條碼指令 |
| 14 | ART (Auto Retest) Flow | ART 自動重測流程 |
| 15 | ATC (Thermal Controller) Interface | ATC 溫控介面 |
| 16 | Customer-Specific Extensions | 客製擴充指令 |
| 17 | Error Handling & Timeout | 錯誤處理與逾時 |
| 18 | Appendix: Quick Reference | 附錄：快速參考 |

---

## 關鍵技術細節

### 實體座標系（Row / Column）

HT-9xxx 使用 **排（Row） × 欄（Column）** 座標系：
- 排：A（第 1 排）、B（第 2 排）、C（第 3 排）、D（第 4 排）
- 欄：a（最左）、b、c、d、e、f、g、h（最右）

### 通道分配規則（Column-Major，按欄）

**重要**：通道分配為「按欄（Column-Major）」，**不是按排（Row-Major）**。

- 每欄依序分配 N 個通道（N = 排數），由排 A 往下
- 欄 a → 最小編號通道群，欄 h → 最大編號通道群
- 驗證來源：`d:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\Command.cpp`，函式 `WriteSiteMapData(bool bGPIB)`

**各模式通道分配：**

| 模式 | 欄 a | 欄 b | 欄 c | 欄 d | 欄 e–h |
|---|---|---|---|---|---|
| 1×4 | Ch1（排A）| Ch2（排A）| Ch3（排A）| Ch4（排A）| — |
| 2×2 | Ch1/Ch2 | Ch3/Ch4 | — | — | — |
| 2×4 | Ch1/Ch2 | Ch3/Ch4 | Ch5/Ch6 | Ch7/Ch8 | — |
| 2×8 | Ch1/Ch2 | Ch3/Ch4 | Ch5/Ch6 | Ch7/Ch8 | Ch9/Ch10...Ch15/Ch16 |
| 4×8 | Ch1–Ch4 | Ch5–Ch8 | Ch9–Ch12 | Ch13–Ch16 | Ch17–Ch32（各欄 4 通道）|

### BINON 字串格式

```
BINON:XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXXX
       第1組       第2組      第3組      第4組
      (Ch32-Ch25) (Ch24-Ch17) (Ch16-Ch9) (Ch8-Ch1)
                                              ↑ Ch1 永遠在最右一個字元
```

---

## 參考技能（Skills）

生成手冊時應優先載入以下技能：

| 技能名稱 | 用途 |
|---|---|
| `gpib-command-list` | 完整指令清單、BIN Mapping、SRQ 流程 |
| `gpib-program-manual` | GPIB 軟體架構、初始化流程 |
| `gpib-93k-art` | Advantest 93K ART 自動重測流程 |
| `gpib-hana` | Hana Micron 客製 ART 協定 |
| `gpib-qrovo` | Qorvo 客製 GPIB 指令（CONFIGURE、FULLSITES? 等）|
| `gpib-rs232-merge` | TTL/GPIB/RS232 三介面整合設計 |

---

## 版本類型選擇指引

### 標準版（Standard）
- 適用：大多數客戶通用
- 章節 16（Customer-Specific）留白或僅含基本擴充
- 不含 ART 流程、Hana/Qorvo 特殊指令

### 完整版（Full）
- 含客製擴充指令（由問題 2、3 判斷）
- ART 章節：引用 `gpib-93k-art` / `gpib-hana`
- Qorvo 章節：引用 `gpib-qrovo`
- 溫度控制：引用 `gpib-command-list` 中 SETTEMP / SETTESTTEMP

---

## 製作歷程

| 日期 | 說明 |
|---|---|
| 2026-06 | 初版建立（18 章 EN + ZH HTML）|
| 2026-06 | 修正 Section 10 Site Map：通道分配由錯誤的 Row-Major 修正為正確的 Column-Major |
| | 驗證來源：`Command.cpp` `WriteSiteMapData()`，`8SITE2X4-%d-%d-%d-%d-%d-%d-%d-%d_` 格式確認欄優先順序 |
