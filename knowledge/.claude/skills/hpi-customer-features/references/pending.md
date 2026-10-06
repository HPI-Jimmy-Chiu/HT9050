# 待補與孤兒候選

來源commit `341cea3d7c7136606223490c91de59019e6c0995`；此處是詞法／文件待核對清單，不能當成機台缺陷。

| 項目 | 實際數量／下一步 |
|---|---|
| 定位候選 | 6382列；其中direct-comparison 4691，其他是symbol／flag引用 |
| file-scope或unresolved | 846列；按原檔與symbol定位、檢查定義／macro／複雜語法，不冒稱精確function |
| 字面停用提示 | 152列；完整建置旗標與caller未求值 |
| 本版MachineType未列CC定義 | 0組版本／符號；可能來自其他宣告，先核對，不能擅自新增客戶 |
| 定義在範圍內無引用 | 1個符號；可能有間接macro／其他樹，不推論不存在功能 |
| 全樹主題owner／行為 | 未完成；先使用[既有20題路由](topics/index.md)，不按檔名自動認領他人表 |
| 機型與實機 | 未做全量dispatch／caller／runtime驗證，HT9050和其他Handler差異保持待核對 |

## 編碼提示

ASCII識別符仍可搜尋；下列檔在指定版本decoder出現替換字元，中文註解與非ASCII語法需另核對原bytes。沒有改檔或重編碼。

| 版本 | 檔案 | 替換字元數 |
|---|---|---:|
| v912 | HT9011UC_Code_V3.33.912.0_20260908_Jimmy/Automation/SCK_TUTS.cpp | 2 |
| v912 | HT9011UC_Code_V3.33.912.0_20260908_Jimmy/Automation/SCK_TUTS.h | 2 |
| v912 | HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cSiteUseManager.cpp | 18 |
| v912 | HT9011UC_Code_V3.33.912.0_20260908_Jimmy/uShowMessage.cpp | 6 |

## 非本批依賴

UPH缺少HT9050原稿的部分仍待來源；不為補齊索引捏造內容。舊入口退休仍等部署／caller盤點，不刪相容路徑。
