> 保存來源：`.claude/skills/ht9045-recipe/references/UdUld.Data.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# UdUld.Data — 上下料等待時間設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\UdUld.Data`
**模組：** [cLd_ULd.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cLd_ULd.cpp)
**結構體：** 上下料時間參數（Carry 相關全域變數）
**讀：** `elUdUld->ReadEditTextFromFile()` ｜ **寫：** `elUdUld->WriteEditTextToFile()`

---

## 欄位定義

| 節點 | 欄位 | 型態 | 典型值 | 說明 |
|------|------|------|--------|------|
| `[Loader Wait Time]` | Arrived | float | 0.2 | Tray 到達 Loader 位置確認等待 (秒) |
| | Lock | float | 0.2 | Tray 上鎖完成等待 (秒) |
| | Close | float | 2.0 | Tray 蓋子關閉等待 (秒) |
| | Check | float | 0.5 | Tray 狀態感測確認等待 (秒) |
| `[UnLoader Wait Time]` | Unlock | float | 0.2 | Tray 解鎖完成等待 (秒) |
| | Lock | float | 0.2 | Tray 再鎖定等待 (秒) |
| | Arrived | float | 0.2 | Tray 到達 UnLoader 位置確認等待 (秒) |
| | Check | float | 0.2 | Tray 狀態感測確認等待 (秒) |

---

## 範例

```ini
[Loader Wait Time]
Arrived=0.2
Lock=0.2
Close=2
Check=0.5
[UnLoader Wait Time]
Unlock=0.2
Lock=0.2
Arrived=0.2
Check=0.2
```

---

## 注意事項

- 等待時間過短可能導致機構動作未完成就繼續，引發 Tray 錯位或夾盤
- `Close=2.0` 針對有蓋板機構的 Loader；無蓋板時可設較小值
- 各值以機台實際機構動作時間為基準校調

---

## 關聯程式碼

- 讀寫實作：[acarry.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\acarry.cpp) — `ReadCarryFile()`, `WriteCarryFile()`
- CatchTray 流程：ht9045-catchtray-flow Skill

<!-- preserved-content:end -->
