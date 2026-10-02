# ep.txt — 電吸力線性補正表

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\ep.txt`
**模組：** 真空吸取 IO 控制層
**格式：** 純文字，每行 `壓力(Pa), 吸力(gf)`
**讀：** 程式啟動時讀入查表陣列（EP Control）

---

## 格式說明

| 欄 | 說明 |
|----|------|
| 第 1 欄 | 吸嘴壓力設定值 (Pa)，步進 100，範圍 0～3500 |
| 第 2 欄 | 對應的電吸力 (gf / 克力) |

---

## 範例

```
   0, 0.0
 100, 0.7
 200, 2.0
 300, 7.7
 400,10.9
1000,25.9
2000,51.4
3500,88.4
```

---

## 注意事項

- 表格呈線性遞增關係；程式以線性插值計算中間值
- 用於 InArm/OutArm 真空流量補償（EP Control）
- `ArmCondition.Data [All] EPControl` 欄位決定是否套用此補正表
- 若 `EPControl` 為空，則此檔不生效（使用固定真空值）
- 行頭空白不影響解析

---

## 關聯程式碼

- EP Control 讀取：搜尋 `ep.txt` in [cArmCondition.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cArmCondition.cpp)
- InArm 真空流程：ht9045-inarm-flow Skill
- IO 控制層：ht9045-io-control Skill
