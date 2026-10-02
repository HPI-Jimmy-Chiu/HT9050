# Tray.Data — 托盤型號配置

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\Tray.Data`
**模組：** [cTray.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cTray.cpp)
**結構體：** `TRAY_TYPE_PARA TrayTypeData[]` 陣列
**讀：** `ReadTrayFile()` ｜ **寫：** `WriteTrayFile()`

---

## 欄位定義

每個 `[TypeN]`（N=0, 1, 2,...）節點定義一種托盤型號：

| 欄位 | 型態 | 值域/說明 | 備註 |
|------|------|----------|------|
| Name | char[] | 托盤型號名稱（自由字串） | 顯示於 UI |
| X Start | float | DUT 陣列 X 起始位置 (mm) | 相對托盤左上角 |
| Y Start | float | DUT 陣列 Y 起始位置 (mm) | — |
| X Pitch | float | DUT 橫向間距 (mm) | 兩顆 DUT 中心距 |
| Y Pitch | float | DUT 縱向間距 (mm) | — |
| Think | float | 托盤厚度 (mm) | 影響 Z 軸吸取高度計算 |
| X Division | int | X 方向 DUT 數（列） | 典型 2～20 |
| Y Division | int | Y 方向 DUT 數（行） | 典型 5～40 |
| Pick Up | float | 吸取高度相對值 (mm) | 通常 68.0 |
| Block NumberX | int | X 方向區塊編號（分區托盤） | 0=不使用 |
| Block NumberY | int | Y 方向區塊編號 | 0=不使用 |
| Block PitchX | float | 區塊 X 間距 (mm) | — |
| Block PitchY | float | 區塊 Y 間距 (mm) | — |
| Block XStart | float | 區塊 X 起始 (mm) | — |
| Block YStart | float | 區塊 Y 起始 (mm) | — |
| Block XItem | int | 區塊內 X 項目數 | — |
| Block YItem | int | 區塊內 Y 項目數 | — |
| Block Tray Type Size | float | 區塊托盤大小 (mm) | — |
| Memo | string | 備註 | 自由輸入 |

---

## 範例

```ini
[Type0]
Name=Polrais
X Start=36.000000
Y Start=36.000000
X Pitch=63.900000
Y Pitch=60.750000
Think=10.160000
X Division=2
Y Division=5
Pick Up=68.000000
Block NumberX=0
Block NumberY=0
Block PitchX=0.000000
Block PitchY=0.000000
Memo=

[Type1]
Name=QFN3X3_14X35
X Start=8.070000
Y Start=7.880000
X Pitch=9.200000
Y Pitch=8.800000
Think=6.350000
X Division=14
Y Division=35
Pick Up=68.000000
Block NumberX=0
Block NumberY=0
Memo=
```

---

## 注意事項

- `Think`（托盤厚度）直接影響 InArm/OutArm 的 Z 軸吸取位置計算
- `Pick Up=68.0` 為相對值，實際吸取高度 = `Pick Up` + 座標偏移
- `Block` 系列欄位用於分區托盤（如盒型 Carrier），一般矩形 Tray 均設為 0
- 各型號按需要配置，Type0 通常為預設/標準型號

---

## 關聯程式碼

- 結構體定義：[MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h) — `TRAY_TYPE_PARA`
- 讀寫實作：[cTray.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cTray.cpp) — `ReadTrayFile()`, `WriteTrayFile()`
- 表單：[cTray.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cTray.h)
- CatchTray 流程：ht9045-catchtray-flow Skill
