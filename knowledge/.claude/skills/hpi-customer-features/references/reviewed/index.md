# 已人工核對的客戶條件與caller

這是S8的局部靜態核對，不將6382筆詞法候選全部升級為行為結論。每列仍需搭配作用中的版本、客戶碼、機型與runtime；沒有執行機台、API、建置或清計數。

| 主題 | 已核對範圍 | Reference |
|---|---|---|
| ART | PTI等待／RunMode、AMKOR與HANA的Tag 0呼叫差異；活動caller及效果守衛 | [ART](art/index.md) |
| 客戶名稱 | 807／808過濾、898名稱覆寫、982符號與UI名稱分離；Factory caller與建置入口 | [客戶名稱](customer-names/index.md) |
| CUSTOMER_CODE讀入與功能選擇 | Model守衛／runtime路徑、編譯條件、caller順序與直接賦值差異 | [開機與功能入口](customer-boot/index.md) |
| CUSTOMER_CODE的Web存檔入口 | WS接收owner、C路PageSave與HSys開窗條件局部查證 | [Web存檔守衛](customer-web-save/index.md) |

另有[客戶碼存檔caller](customer-persistence.md)及[開機讀入／功能入口](customer-boot/index.md)的後續局部核對；完整API／owner、巢狀callee／consumer與磁碟效果仍待補。

原ART／名稱人工客戶差異列共8筆，以版本／檔名＋function、Task／case及關鍵變數定位。來源釘住main `09d926ab86fea6cf366bd9d6c90167d51dedf317`，[來源manifest](source-manifest.json)目前保留21個Git blob；候選來源仍是 `341cea3d7`，原候選／未決數不改。兩次來源間這些檔案無變更，查證日期／commit仍分開記錄。

共同方法與權威見[查證流程](../common.md)、[機型分流](../machines.md)及[資料權威](../authority.md)。本區只補新索引的人工核對，沒有覆寫所屬主題原人工客戶表，也沒有改報告地區／代理商權威。

剩餘候選的全caller、機型dispatch、建置條件與主題歸屬仍待逐批核對；[原始待補清單](../pending.md)保持可追溯，不因這8列扣除其他候選或宣稱912-only。

開機／功能入口另有3筆局部客戶條件列，來源36625f91b及17個blob獨立保存；原8列與來源manifest不改，各次查證日期／來源分開。

20261007續補Web／HSys局部查證1列，來源de7c2bf77與10份blob獨立保存；原8＋3列保持，現在12列仍為局部靜態證據，完整owner／callee／consumer／磁碟效果未完成。

20261007續補[proxy／INI寫入局部查證](customer-storage/index.md)：核對edtCustomerCode真正父鏈、文字型別、標記先於寫入及V906 void包裝界線。原12列／6382候選／846未決與舊來源保持；仍未驗證實際磁碟成功、所有proxy寫者／最後consumer或完整交易。

20261007續補[reauth頁面分流／清除與ack局部查證](customer-reauth/index.md)：HSys不在PointOf對照表，沒帶答案仍需原存檔閘。原12列／候選／舊來源保持；完整登入、owner時序、所有結果寫者與磁碟效果仍未完成。

20261007續補[owner／queue局部時序](customer-owner/index.md)：核對接收owner、控制權變更、connId／ticket、queue／共用guard與所選存檔分支。完整HTTP／carry／callee／owner競態仍未完成；不由接收通過或ack推定持續控制權與磁碟成功。

- [重新驗證 callee：答案消耗／wrapper／權限結果（局部）](customer-reauth-callee/index.md)

- [登入結果／I37／PTI與密碼本局部對照](customer-login/index.md)

- [密碼本namespace／路徑／格式與轉碼helper（局部）](customer-login-helpers/index.md)
