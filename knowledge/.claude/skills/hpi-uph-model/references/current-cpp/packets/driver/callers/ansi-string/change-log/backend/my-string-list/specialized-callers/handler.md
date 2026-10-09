# Handler motor alarm：行數不是保留位置

[上層](index.md)；callee 詳見既有 [專用 writer](../specialized.md)
及 [主要追加 writer](../writer.md)。本頁只讀 V906 現行正文，沒有觸發警報或硬體操作。

## 真正呼叫的是 Public 類別

`forms/fNote_ShowError.cpp` 包含 `Public/MyStringList.h`；
`slEventLog` 的 `MyInsertToFile(Msg, iCount)` 接受文字與 index。
RS232 專用 overload 只有一個 int，不能因兩者同名就接到 tester log。
`ShowMotorErrorMessage` 在 `InitialOK==false` 記 Exception 後 return；
Code 等於 WAR 時轉 ShowErrorMessage 並 return。以下鏈只描述走過這兩個 early return 的路徑。

## 行數、flush 與插入的先後

| 順序 | function／變數 | 靜態意義 |
|---|---|---|
| 1 | MyDBIEvent、ProductionLog、ErrShowToForm | 處理事件／欄位；這些後端在前批文件，不等於本插入已成功。 |
| 2 | slEventLog 非 NULL → GetLastLine | 設 fNote->iAlarmLine；GetLastLine 也把當時 GetFileName 選到的名稱記入 sLastFileName。 |
| 3 | 第一次 SaveEventLog | 刷出原有緩衝，與行數取樣不同步驟。 |
| 4 | W906_ShowMotorErrorMessage_Hook | 有 hook 才送通知；此 host 路徑在本文不作完整可達／link 查證。 |
| 5 | 組 SL，第二次 SaveEventLog | 呼叫後再用 SL->CommaText 與 fNote->iAlarmLine-1 插入。 |

兩個 slEventLog 的 null guard 各自包住取行數／flush 與插入，沒有把檔案鎖住。
`LogObjects.cpp::SaveEventLog` 先 MySaveToFile，再 GetFileName，並用
`RunInfo.slEventLogFile->Text.AnsiPos(FileName)` 決定要不要 Add 名稱；
該名稱清單的文字子字串命中不等於檔案寫入回執或完整檔名集合比對。
SaveEventLog 為 void；它沒有檢查 MySaveToFile 的落盤結果。

`GetLastLine` 的數字是檔案 reader 當時算到的行數，不含未 flush 的 MyList，
也不是下一筆 alarm 的保留槽。當時行數為 n，caller 傳 n-1；
Public writer 只有「已有檔案、File->Count>index 且 index>0」才 Insert，否則 Add。
因此 n 為 0 或 1 時沒有正 index；n>=2 時是否插入還取決於重讀檔案的 Count。
不把這個數字命名為 record ID，也不假設它就是檔尾。

## 名稱漂移與新檔條件

GetLastLine 先依當時日期／SaveType 選名稱；MySaveToFile 的名稱另有時間取樣，
而插入讀 sLastFileName。期間若跨檔名分段，flush 的目的地與已記名稱可能不同；
此為靜態條件推論，未執行跨日／跨段測試。
Public MyInsertToFile 若檢查 sLastFileName 不存在，只更新 GetFileName，沒有寫 Msg。
caller 的兩次 SaveEventLog 可能在插入前建立同一檔案，也可能因沒有 buffer 而早退；
不能一概稱「每個首次警報都丟失」或「flush 保證建立檔案」。
CreateFile probe、close 與 LoadFromFile／SaveToFile 分開，沒有整段交易鎖或成功回執。

## SPIL 欄位與非阻塞通知

`IniConfig.bSPILFunction` 為 true：Str 是單一日期時間欄；SL 依序加
sJamArea、sJamCode、Str、Recovery、PassTime、iDuplicateError、Message、MotorAlarmNo。
其他分支先加日期與帶毫秒時間，後加同類資訊及 GetLastOpenFN。
這是 SL->CommaText 的欄位來源，不是驗證部署 CSV 的 quoting、locale 或 ABI。
SystemYear…SystemMSec 是既有全域值，這段組字串沒有自己刷新。

原文描述通知非阻塞、PAUSE 結果先套 SoftStop／SoftStart；正文在 hook 返回後即組
Recovery／PassTime 並寫入。它沒有等待操作員回覆後再取實際停止時間。
完整 notice capture／ack、其他警報 caller 與更新行的策略仍待查，
不將 source banner 的歷史驗證敘述當成這次機台測試。

`SaveJamCodeFile` 位於同一函式的 `#if 0` F1 gate；
這證明選定 motor alarm 片段未經這條 FTP 檔案呼叫，不證明整個系統沒有 upload。
