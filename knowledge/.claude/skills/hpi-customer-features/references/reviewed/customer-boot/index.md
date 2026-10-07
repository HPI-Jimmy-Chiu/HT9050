# 開機讀入與功能選擇：局部查證

同題整合HT9050與其他Handler；這次釘住main36625f91b，原341cea3d7候選樹與09d926ab8的ART／名稱查證不改。這是靜態reader／caller局部查證，沒有執行機台、存檔、登入、API或建置。

| 問題 | Reference |
|---|---|
| 機型守衛、CUSTOMER_CODE來源、缺鍵與編譯條件 | [讀入與runtime路徑](reader.md) |
| CustomerFunctionSelect入口／呼叫順序、直接賦值的版本差異 | [功能選擇入口](feature-selection.md) |
| 已核對檔案的Git blob與範圍 | [來源manifest](source-manifest.json) |
| 表單／Web存檔後重新讀入 | [存檔caller](../customer-persistence.md) |

本區另有3列局部客戶條件；原ART／名稱的8列保持不變，共11列仍只是人工局部查證。846個原未決候選、所有consumer／callee、機型dispatch與主題歸屬沒有因此完成。

HT9050對應V906的9050GPIB分支；V912 ReadGeneralIni此入口未列9050GPIB，不以這一點推論V912全樹或客戶功能完全不支援HT9050。機台身分、實際CUSTOMER_CODE、編譯巨集與runtime另外確認。
