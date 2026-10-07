# CSV 局部證據的查證界線

[manifest](source-manifest.json) 保存八份 source 的 blob／byte SHA、十一個指定 body 的 signature／namespace／guard／SHA、六個宣告。outer guard 都沒有字面 inactive；V906 FOREHOPE 的空 body 是本體內容，不是此次篩選排除了內容。

這一單元補足指定 writer 與 common／V912 FileInfo 的靜態讀取；不覆寫前一單元 pin8f213da4f 的 CalculateUPH／AddLoadingCount／日期 helper 證據。兩版 ainarm9045.cpp 的 blob 在兩個 pin 間相同，但沒有因 blob 相同就把全部函式視為已讀。

仍待閉合：所有 writer／caller、counter／pause／reset、site helper 與機型分派、DB／SECS／UI／CSV consumer、檔案成功／回讀／錯誤呈現、編譯與連結版本、日期與文字平台等價、容量與量測校正。

輸出成功也要核對執行帳號／ACL。[RULINGS_20261007 第8條指標](../../resources.md#目前保存裁決) 保存 Jerry 本人的裁決及他台靜默寫入失敗回報：裝機流程修目錄授權，`wb_serve` 不升權。本單元未檢查本機 ACL 或執行權限操作，不能據此判定本次 UPH 保存成功或失敗。

HT9050 與其他 Handler 共同使用 [UPH 主題機型樹](../../machines/index.md)；本頁不把 V912 客戶 writer 認定為 HT9050 作用中功能，也不把歷史 Per-Tray、HTML 排程／動畫、Profiler 設計與目前原碼混成實機量測。

原文、metadata、資源、舊入口與裁決正文照舊保存。沒有 C++ 建置／執行、API／LIVE、檔案寫入、機台動作或 runtime 修改；保存結果與完整 UPH 語意仍未驗證。
