# HotPlate.Data — 加熱盤配置設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\HotPlate.Data`
**模組：** [cHotPlate.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cHotPlate.cpp#L160)
**結構體：** `TRAY_TYPE_PARA HotPlateForm_File` (global)
**讀：** `TfHotPlate::ReadFile()` ｜ **寫：** `TfHotPlate::SaveFile()`

---

## 欄位定義

| 節點 | 欄位 | 型態 | 值域/說明 | 結構體成員 |
|------|------|------|----------|-----------|
| `[Hotplate Form]` | Name | char[] | 加熱盤型號（Polaris, BGA XE, PHOENIX...） | `HotPlateForm_File.Alias` |
| | X Start | float | X 起始位置 (mm) | `HotPlateForm_File.XStart` |
| | Y Start | float | Y 起始位置 (mm) | `HotPlateForm_File.YStart` |
| | X Pitch | float | X 間距 (mm)；若 XDivision=1 則程式強制設為 0 | `HotPlateForm_File.XPitch` |
| | Y Pitch | float | Y 間距 (mm) | `HotPlateForm_File.YPitch` |
| | X Division | int | X 分割數（列）；≤0 則預設 6 | `HotPlateForm_File.XDivision` |
| | Y Division | int | Y 分割數（行）；≤0 則預設 11 | `HotPlateForm_File.YDivision` |
| | Using Flag | int | 0/1/2/3 選擇加熱盤面；預設讀值為 2 | `HotPlateForm_File.iPlateSelect` |
| | Use Wide Hotplate | bool | 0=標準, 1=寬盤；受 `IniConfig.bHotPlateMove1CM` 控制 | `HotPlateForm_File.bUseWideHotplate` |
| `[System]` | bTrayHotplateCheck | bool | true=每次補盤前檢查加熱盤有無 IC 殘留 | `HotPlateForm_File.bTrayHotplateCheck` |

---

## 範例（Polaris 3×5）

```ini
[Hotplate Form]
   Name=Polaris
   X Start=45.000
   Y Start=50.000
   X Pitch=65.000
   Y Pitch=70.000
   X Division=3
   Y Division=5
   Using Flag=3
Use Wide Hotplate=1
```

---

## 注意事項

- 當 `TestIF_File.iTestMode >= _6Site2X3` 且 `XDivision==4` 且 `YDivision%2==1` 時，程式強制將 `YDivision` 改為偶數（×10）
- `XDivision=1` 時程式自動強制 `XPitch=0`，無需手動填入
- `Using Flag` 用於多塊加熱盤切換（0～3）
- `bHotPlateMove1CM` (IniConfig) 開啟時自動切換 Wide Hotplate 模式

---

## 關聯程式碼

- 結構體定義：[MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h) — `TRAY_TYPE_PARA`
- 讀寫實作：[cHotPlate.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cHotPlate.cpp) — `TfHotPlate::ReadFile()`, `TfHotPlate::SaveFile()`
- 相關表單：[cHotPlate.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cHotPlate.h)
