# Rotate.Data — 旋轉臂設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\Rotate.Data`
**模組：** [cRotate.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cRotate.cpp)
**結構體：** 旋轉相關全域結構
**讀：** `ReadRotateFile()` ｜ **寫：** `WriteRotateFile()`

---

## 欄位定義

| 節點 | 欄位 | 型態 | 值域/說明 |
|------|------|------|----------|
| `[SETTING]` | ActiveRotate | int | 0=禁用旋轉, 1=啟用 |
| | Dut Num | int | 旋轉圓盤上的 DUT 數 (1～8) |
| | FromTrayAngle | int | 從 Tray 取到旋轉器的起始角度 (°) |
| | DutAngle | int[8] | 8 個 DUT 的角度位置，逗號分隔 (°) |
| | ColCount | int | Kit 橫向（列）分割數 |
| | RowCount | int | Kit 縱向（行）分割數 |
| | KitPitchX | float | Kit DUT 橫向間距 (mm) |
| | KitPitchY | float | Kit DUT 縱向間距 (mm) |

---

## 範例

```ini
[SETTING]
ActiveRotate=0
Dut Num=1
FromTrayAngle=0
DutAngle=90,90,90,90,90,90,90,90
ColCount=4
RowCount=2
KitPitchX=40.0000
KitPitchY=60.0000
```

---

## 注意事項

- `ActiveRotate=0`：此工作檔不使用旋轉功能（大部分機台不用）
- `DutAngle` 8 個值對應旋轉盤上 8 個位置的角度
- `FromTrayAngle`：InArm 取到旋轉器的角度，與旋轉器的 0° 基準對應
- `ColCount`/`RowCount`：當 Kit 有多個 DUT 時（如 4×2 Kit）需配置

---

## 關聯程式碼

- 讀寫實作：[cRotate.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cRotate.cpp) — `ReadRotateFile()`, `WriteRotateFile()`
- 表單：[cRotate.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cRotate.h)
