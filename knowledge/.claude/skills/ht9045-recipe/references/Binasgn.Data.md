# Binasgn.Data 系列 — Bin 分類設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\Binasgn*.Data`
**模組：** [cBinSel.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cBinSel.cpp)
**結構體：** `SYSTEM_BIN_SELECT BinSelect[8]`
**讀：** `TfBinSel::ReadFile()` ｜ **寫：** `TfBinSel::SaveFile()`

---

## 7 個分類檔對應測試模式

| 檔案名稱 | 測試模式 | 載入時機 | `BinSelect[]` 索引 |
|---------|---------|---------|-------------------|
| `Binasgn.Data` | FT（線上第一測） | 一般運行 | `BinSelect[FT=1]` |
| `BinasgnOff-Line.Data` | OffLine（線下測試） | 線下模式切換 | `BinSelect[OffT=2]` |
| `BinasgnOff.Data` | OffLine 副本 | 備用 | — |
| `Binasgn_ART.Data` | ART（自動複測） | ART 模式啟用 | `BinSelect[RT_ART=3]` / `FT_ART=4` |
| `Binasgn_MRT.Data` | MRT FT（多次複測-第一測） | MRT 模式啟用 | `BinSelect[FT_MRT=6]` |
| `Binasgn_MRT_RT.Data` | MRT RT（多次複測-複測階段） | MRT 複測時 | `BinSelect[RT_MRT=5]` |
| `BinasgnOff_ART.Data` | OffLine + ART | OffLine ART 模式 | 複合模式 |

**BinSelect 陣列索引：** RT=0, FT=1, OffT=2, RT_ART=3, FT_ART=4, RT_MRT=5, FT_MRT=6

---

## 欄位定義

### `[I/F Error]` — 介面接觸錯誤
| 欄位 | 型態 | 說明 |
|------|------|------|
| Bin | int | 指定 Bin 編號（通常=5）；-1=不使用 |
| Tray | string | 放置托盤（Fix3, Auto1, ...） |
| Contact | int | 1=計入接觸失敗次數 |
| Cons.Fail | int | 1=連續失敗計數 |
| Fail Percent | int | 1=Fail 百分比計數 |
| Scan | int | 掃描類型 |

### `[Category0～9]` — 測試分類
| 欄位 | 型態 | 說明 |
|------|------|------|
| Bin | int | 分類對應的 Bin 編號（0=Pass） |
| Contact | int | 1=此分類接觸失敗計數 |
| Cons.Fail | int | 1=連續失敗計數模式 |
| Fail Percent | int | 1=Fail 百分比計數模式 |
| Fail Percent Limit | float | Fail 百分比上限 (%)；0=不限 |
| Fail Percent Ignore | int | 百分比統計忽略件數 |
| Fail Count | int | 1=Fail 計數模式 |
| Fail Count Ignore | int | 計數統計忽略件數 |
| Fail Count Limit | int | Fail 計數上限；0=不限 |
| Special Bin By Arm | int | 1=同臂特殊 Bin 判斷 |
| Special Bin Count By Arm | int | 同臂計數閾值 |
| Special Bin By Socket | int | 1=同 Socket 特殊 Bin 判斷 |
| Special Bin Count By Socket | int | 同 Socket 計數閾值 |
| By Bin Low Yield | int | 1=此 Bin 啟用低產率報警 |
| By Bin and Site Compare Arm Yield | int | 1=Bin+Site vs 臂 產率對比 |
| By Bin Compare Site Yield | int | 1=Bin vs Site 產率對比 |
| Tray | string | 分配托盤（Auto1, Auto2, Fix7, ...） |
| Scan | int | 掃描類型 |

---

## 範例

```ini
[I/F Error]
   Bin=5
   Tray=Fix3
   Contact=0
   Cons.Fail=0
   Fail Percent=0
   Scan=0
[Category0]
   Bin=0
   Tray=Auto1
[Category1]
   Bin=1
   Fail Percent=0
   Fail Percent Limit=0.0000
   Fail Percent Ignore=0
   Tray=Auto1
[Category4]
   Bin=4
   Cons.Fail=1
   Fail Percent=0
   Fail Percent Limit=10.0000
   Fail Percent Ignore=200
   Fail Count=0
   Fail Count Limit=0
   Tray=Fix7
```

---

## 注意事項

- `BinSelect` 陣列共 8 個，索引 RT=0, FT=1, OffT=2, RT_ART=3, FT_ART=4, RT_MRT=5, FT_MRT=6
- `iBinData32` 陣列有上限，勿越界 → 參見 [ht9045-array-audit](file:///d:\HT9045\.github\skills\ht9045-array-audit\SKILL.md) Skill
- Category 編號從 0 開始，0 通常對應 Pass (Bin=0)
- `Tray` 欄位必須與實際 Auto/Fix Tray 編號對應，否則出料會放錯托盤

---

## 關聯程式碼

- 結構體定義：[MachineType.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h) — `SYSTEM_BIN_SELECT`
- 讀寫實作：[cBinAssin.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cBinAssin.cpp) — `ReadBinAssin()`, `WriteBinAssin()`
- 表單：[cBinAssin.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cBinAssin.h)
- Yield Alarm：ht9045-yield-flow Skill
- BinSelect 邊界：ht9045-array-audit Skill
