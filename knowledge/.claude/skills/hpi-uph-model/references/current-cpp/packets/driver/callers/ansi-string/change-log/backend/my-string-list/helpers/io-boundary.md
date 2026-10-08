# IO呼叫形狀與成功界線

[上層](index.md)；共用Writer沿用已完成的 [WriteDataToFile／版本差異](../../../../../../../../writers/common-io.md)，本輪2個Handler overload只作context。

| 層 | 可由已保存正文確認 | 本輪尚未建立 |
| --- | --- | --- |
| MyForceDirectories | 分類後呼叫exists／mkdir；int=1未核底層bool。 | 目錄或每個中間component確實可用。 |
| FileExists再open | 查詢與開檔是不同呼叫，沒有一個handle覆蓋查詢／header／寫入全段。 | 查詢後檔案狀態不變或多writer不交錯。 |
| MyStringList share-mode | 上層已保存CreateFile的GENERIC_WRITE／FILE_SHARE_READ／OPEN_ALWAYS與FILE_END定位、WriteFile、CloseHandle；未核定位／寫入結果，走到底Clear。 | 目前OS sharing規格、partial write、flush／持久性與成功落盤。 |
| common WriteDataToFile | void、fopen w／a、fputs資料再newline；AnsiString轉c_str轉呼叫，失敗沒有成功值供caller核。 | 實際CRT文字模式bytes、encoding、所有錯誤及併發結果。 |
| SysUtils DeleteFile | 自身本體用std::remove回傳==0；上層SG Jam沒有核結果。 | 真機刪檔、所有平台目錄／sharing行為、回滾或檔案安全。 |

這裡不把Windows函式呼叫實參當成已驗證的鎖檔規格。沒有找到選定filesystem TU內包覆CreateFile／WriteFile的自訂成功保證；完整preprocessor、link與OS規格需另查，不由同名DeleteFile shim推演其他API。

common.cpp相鄰CheckFileIsEmpty／ReadDataFromFile、RS232自己的writer與EventLogAnalysis helper沒有在這個新單元完成；既有原文留在各自來源，不為擴大數量而算入本輪。

本輪沒有建立測試檔、刪檔、執行writer或讀取機台log。所有失敗／併發情境是靜態界線，實際結果需另行授權與驗證。
