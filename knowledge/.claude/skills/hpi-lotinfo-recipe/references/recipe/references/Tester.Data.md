> 保存來源：`.claude/skills/ht9045-recipe/references/Tester.Data.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Tester.Data — 測試介面與報警設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\Tester.Data`
**模組：** [uLotInfo.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uLotInfo.cpp)
**結構體：** `SYSTEM_TEST_IF TestIF_File` (global)
**讀/寫：** `ReadIniData()` / `WriteIniData()`

---

## 欄位定義

| 節點 | 欄位 | 型態 | 值域/說明 |
|------|------|------|----------|
| `[Mode]` | Tester Type | int | 0=未安裝, 1=安裝 |
| `[Time]` | MAX Time | float | 測試最長時間 (秒)；超時 = SocketError |
| | Dummy Time | float | 虛擬測試時間 (秒)；Dummy Mode 時使用 |
| | Stary Delay | float | 啟動延遲 (秒)；首次下壓前等待 |
| | Initial MAX Time | float | LOT 第一次（初始）測試最長時間 (秒) |
| | Init Wait Time | float | 初始化等待 (秒) |
| | Testing Wait Time | float | 測試中等待 (秒) |
| | After Tested Delay | float | 測試完成後延遲 (秒) |
| | bUseSocketHeating | int | 1=Socket 加熱功能啟用 |
| `[Alarm]` | SocketEnable | int | 1=Socket 連續失敗報警 |
| | SocketCT | int | Socket 連續失敗計數閾值 |
| | HeadEnable | int | 1=測試頭連續失敗報警 |
| | HeadCT | int | 測試頭連續失敗計數閾值 |
| | RateEnable | int | 1=Fail 比例報警 |
| | Continuous Pass | int | 1=連續通過計數報警 |
| | Continuous Pass Bin | int | 連續通過的 Bin 編號 |
| | Continuous Pass Bin Count | int | 連續通過計數閾值 |
| | Count Action | int | 0=停機, 1=暫停, ... |
| | Continuous Contact | int | 1=連續接觸失敗報警 |
| | Continuous Contact Count | int | 連續接觸失敗計數閾值 |
| `[DIO]` | Type | int | DIO 介面類型 |
| | Anti Signal | int | 防抖動訊號設定 |
| | Version | string | DIO 版本（如 "V2.06"） |
| | TypeName | string | DIO 型態描述 |
| `[GP-IB]` | Type | int | 0=未安裝, 1以上=安裝 |
| | Address | int | GP-IB 位址 (0～30)；通常為 7 |
| | 2DIDFormat | int | 2D Barcode 格式 |
| `[RS-232C]` | Type | int | 0=未安裝, 1以上=安裝 |
| | Baud Rate | int | 0=9600, 1=19200, 2=4800, ... |
| | Bin Count | int | RS-232 可回報的 Bin 數 |
| `[Site Yield Alarm]` | Site Yield Different | int | 1=啟用 Site 產率差異報警 |
| | Site Yield | int | 最低產率閾值 (%) |
| | Site Yield Different Count | int | 報警觸發計數 |

---

## 範例

```ini
[Mode]
   Tester Type=1
[Time]
   MAX Time=6000.00
   Dummy Time=0.00
   Stary Delay=0.00
   Initial MAX Time=6000.00
   Init Wait Time=0.00
   Testing Wait Time=0.00
   After Tested Delay=0.00
   bUseSocketHeating=0
[Alarm]
   SocketEnable=1
   SocketCT=5
   HeadEnable=0
   HeadCT=5
   Continuous Pass=1
   Continuous Pass Bin=1
   Continuous Pass Bin Count=400
   Count Action=0
[GP-IB]
   Type=0
   Address=7
   2DIDFormat=0
[RS-232C]
   Type=0
   Baud Rate=0
   Bin Count=32
```

---

## 注意事項

- `MAX Time` 超時會觸發 SocketError，計為接觸失敗
- `RT` 字首欄位（如 `RT MAX Time`）對應 Retest 模式的獨立時間
- `EQC` 字首欄位對應 EQC 模式的獨立時間
- `Continuous Pass Bin Count=400` 意指同一 Bin 連續 Pass 400 次觸發警報（可能測試條件異常）

---

## 關聯程式碼

- 結構體定義：[MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h) — `SYSTEM_TEST_IF`
- 讀寫實作：[uLotInfo.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uLotInfo.cpp) — `ReadIniData()`, `WriteIniData()`
- Yield Alarm 流程：ht9045-yield-flow Skill
- GPIB 通訊：ht9045-secs-sem Skill / GPIB9045 工作區

<!-- preserved-content:end -->
