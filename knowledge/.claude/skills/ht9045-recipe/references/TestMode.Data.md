# TestMode.Data — 測試位置與連接模式設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\TestMode.Data`
**模組：** [cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp#L1696)
**結構體：** `SYSTEM_TEST_MODE TestMode` (global)
**讀：** `ReadTestMode()` ｜ **寫：** `SaveTestMode()`

---

## 欄位定義

| 節點 | 欄位 | 型態 | 值域/說明 | 結構體成員 |
|------|------|------|----------|-----------|
| `[TestMode]` | Tester Connection | int | 0=None, 1=DIO, 2=GPIB, 3=RS-232 | `TestMode.iTestConnection` |
| | Temperature Mode | int | 0=加熱, 1=常溫(Ambient) | `TestMode.iTemperatureMode` |
| | Running Mode | int | 0=線上FT, 1=線下/Dummy | `TestMode.iRunMode` |
| `[DutOnOff]` | Dut Aa～Hh | int[4][8] | 0=關, 1=開；臂1各位置 | `TestMode.iDutOnOff[0][row][col]` |
| | Dut Aa2～Hh2 | int[4][8] | 0=關, 1=開；臂2各位置 | `TestMode.iDutOnOff[1][row][col]` |
| `[DutOnOff_RT]` | Dut Aa～Hh | int[4][8] | 與 DutOnOff 相同，但用於 ReTest 模式 | `TestMode.iDutOnOff[arm][row][col]`（RT 寫入） |
| `[DutOnOffEE]` | Dut Aa～Hh | int[4][8] | Extended/EE Site 的 DUT On/Off | `TestMode.iDutOnOffEE[0][row][col]` |
| `[DutOnOffEE_RT]` | Dut Aa～Hh | int[4][8] | EE Site 的 ReTest DUT On/Off | `TestMode.iDutOnOffEE[1][row][col]` |

---

## 範例

```ini
[TestMode]
Tester Connection=0   ; 0=None
Temperature Mode=1    ; 常溫
Running Mode=0        ; 線上測試
[DutOnOff]
Dut  Aa=1    ; Row0,Col0 開
Dut  Ab=1    ; Row0,Col1 開
Dut  Ac=0    ; Row0,Col2 關
[DutOnOff_RT]
Dut  Aa=1    ; RT 模式下同樣開啟
Dut  Ab=0    ; RT 模式下關閉
[DutOnOffEE]
Dut  Aa=1    ; EE Site Row0,Col0 開
[DutOnOffEE_RT]
Dut  Aa=1    ; EE Site RT Row0,Col0 開
```

---

## 注意事項

- `Dut On/Off` 與 `LastSet.bUseTestSocket[][][]` 雙向同步
- 同步函數：`TestModeDutOnOffToLastSetUseTestSocket()` / `LastSetUseTestSocketToTestModeDutOnOff()`
- `Running Mode=0` = 線上測試（FT）; `=1` = Dummy/線下
- `iTestConnection` 值決定程式從哪個介面取得測試結果（GPIB/DIO/RS232）
- `[DutOnOff_RT]`：ReTest（RT）模式單獨的 DUT On/Off 設定，覆蓋 `[DutOnOff]`
- `[DutOnOffEE]`：EE（Extended Edge）Site 的 DUT On/Off，用於擴充 Socket 配置
- `[DutOnOffEE_RT]`：EE Site 的 RT 模式設定

---

## 關聯程式碼

- 結構體定義：[MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h) — `SYSTEM_TEST_MODE`
- 讀寫實作：[cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp) — `ReadTestMode()`, `SaveTestMode()`
- DutOnOff 同步：搜尋 `TestModeDutOnOffToLastSetUseTestSocket` in [cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp)
