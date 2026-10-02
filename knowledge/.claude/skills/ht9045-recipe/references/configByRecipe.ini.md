# configByRecipe.ini — 工作檔特化配置覆蓋

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\configByRecipe.ini`
**模組：** [cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp)
**常數：** `asFileNameConfigByRecipe = "configByRecipe.ini"`
**讀：** `LoadConfigByRecipe()`

> **優先順序：** 此檔設定值 > `General.ini` > 程式預設值
> 若檔案為空或欄位不存在，則使用 `General.ini` 的值。

---

## 欄位定義

| 節點 | 欄位 | 型態 | 說明 |
|------|------|------|------|
| `[Configuration]` | Handling Mode | int | 覆蓋機台運行模式 (1～9) |
| | iAutoClean_Function | int | 0=禁用, 1=啟用 AutoClean（Recipe 特化） |
| | iAutoClean_Mode | int | AutoClean 模式（41=標準） |
| | iAutoClean_IntervalContact | int | 清潔間隔接觸次數 |
| | iAutoClean_DeveicePices | int | 每次清潔 DUT 件數 |
| | iAutoClean_Tray | int | AutoClean 使用的托盤號 |
| | iAutoClean_AlarmCount | int | 清潔後自動報警計數 |
| | SocketSensor | int | Socket 感測器啟用 |
| | Octal Pitch 80 | int | 八進位 80 單位間距模式 |

---

## 範例（空檔與有值兩種）

**空檔（最常見）：**
```ini
; configByRecipe.ini — Recipe specific overrides
; 此檔為空，使用 General.ini 預設值
```

**有值（特化機台）：**
```ini
[Configuration]
iAutoClean_Function=1
iAutoClean_Mode=41
iAutoClean_IntervalContact=20
iAutoClean_DeveicePices=4
iAutoClean_Tray=3
SocketSensor=1
```

---

## 注意事項

- 大部分工作檔此檔為**空**，只在需要覆蓋 General.ini 的工作檔才填入
- 此檔僅覆蓋列出的欄位，未列欄位以 `General.ini` 為準
- 常用於同一機台跑不同 Recipe 需要不同 AutoClean 設定時

---

## 關聯程式碼

- 讀取實作：[cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp) — 搜尋 `LoadConfigByRecipe`
- General.ini 關係：ht9045-general-ini Skill
- IniConfig 結構：ht9045-config Skill
