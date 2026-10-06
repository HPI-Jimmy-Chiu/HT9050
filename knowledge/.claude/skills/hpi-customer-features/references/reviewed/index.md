# 已人工核對的客戶條件與caller

這是S8的局部靜態核對，不將6382筆詞法候選全部升級為行為結論。每列仍需搭配作用中的版本、客戶碼、機型與runtime；沒有執行機台、API、建置或清計數。

| 主題 | 已核對範圍 | Reference |
|---|---|---|
| ART | PTI等待／RunMode、AMKOR與HANA的Tag 0呼叫差異；活動caller及效果守衛 | [ART](art/index.md) |
| 客戶名稱 | 807／808過濾、898名稱覆寫、982符號與UI名稱分離；Factory caller與建置入口 | [客戶名稱](customer-names/index.md) |

人工列共8筆，以版本／檔名＋function、Task／case及關鍵變數定位。來源釘住main `09d926ab86fea6cf366bd9d6c90167d51dedf317`，[來源manifest](source-manifest.json)保留20個Git blob；候選來源仍是 `341cea3d7`，原候選／未決數不改。兩次來源間這些檔案無變更，查證日期／commit仍分開記錄。

共同方法與權威見[查證流程](../common.md)、[機型分流](../machines.md)及[資料權威](../authority.md)。本區只補新索引的人工核對，沒有覆寫所屬主題原人工客戶表，也沒有改報告地區／代理商權威。

剩餘候選的全caller、機型dispatch、建置條件與主題歸屬仍待逐批核對；[原始待補清單](../pending.md)保持可追溯，不因這8列扣除其他候選或宣稱912-only。
