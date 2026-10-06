> 保存來源：`.claude/skills/ht9045-recipe/references/Information.txt.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Information.txt — ATK 資訊摘要（自動產生）

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\Information.txt`
**模組：** [cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp)
**生成函數：** `ATK_RECIPE_INFO::SaveFile()`
**客戶：** ATK（`D:\eRMS\`）、AMKOR Korea/China

> ⚠️ 此檔由程式自動產生，**勿手動修改**。

---

## 欄位說明

| 欄位 | 說明 | 對應程式碼 |
|------|------|----------|
| `Site=N` | 啟用的 DUT 總數 | `GetSiteCount(false)` |
| `Temp=N` | 工作溫度 (°C)；Ambient 時固定輸出 25 | `Temperature.fWorkTemperBase` |
| `Soaktime=N` | 浸泡時間 (秒)；Ambient 時固定輸出 0 | `Temperature.fSoakTime` |
| `Binprofile=0G,1G,...,NR` | 各 Bin 的 Pass(G)/Fail(R) 狀態列表 | `BinSelect[iTestRunMode].iCatDataT3Pos[]` |
| `Sortgate=1:1,2:2,...` | Category:Bin 映射（逗號連接多 Bin） | `BinSelect[].iCatDataT3Pos[]` |
| `Sitemap=s:N/r:R/c:C/d:D/no:...` | Site 排列資訊 | `GetSiteNumberString()`, `SiteMapDirection()` |
| `SmartBin=N` | 非 ART/MRT 模式的 Fail Bin 編號 | `AMR.GetNormalFailBin()` |

### Sitemap 方向 `d` 值對照

| d | 排列方向 |
|---|---------|
| 0 | 無規律 |
| 1 | 單列由左→右 |
| 2 | 單行由上→下 |
| 3 | 單列由右→左 |
| 4 | 單行由下→上 |
| 5 | 先行由上→下，再左→右 |
| 6 | 先行由右→左，再下→上 |
| 7 | 先列由上→下，再左→右 |
| 8 | 先列由下→上，再右→左 |
| 9 | 先行由右→左，再上→下 |
| 10 | 先行由左→右，再下→上 |
| 11 | 先列由上→下，再右→左 |
| 12 | 先行由下→上，再左→右 |

---

## 範例

```
Site=8
Temp=25
Soaktime=0
Binprofile=1G,2G,3G,4G,5R,6R
Sortgate=1:1,2:2,3:3,4:4,5:5,6:6
Sitemap=s:8/r:4/c:2/d:5/no:1,2,3,4,5,6,7,8/
SmartBin=4
```

---

## 注意事項

- **Ambient 模式**（Temperature Mode=1）時，`Temp` 固定輸出 `25`，`Soaktime` 固定輸出 `0`
- 此檔格式由 ATK/AMKOR 工廠端系統解析；格式不符會導致上傳失敗
- `Binprofile` 中 G=Good/Pass, R=Reject/Fail
- `Sortgate` 格式為 `Category:Bin`，多個連接用逗號

---

## 關聯程式碼

- 生成函數：[cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp) — `ATK_RECIPE_INFO::SaveFile()`
- Sitemap 方向計算：搜尋 `SiteMapDirection` in [cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp)
- GetSiteCount：搜尋 `GetSiteCount` in [cprod.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cprod.cpp)

<!-- preserved-content:end -->
