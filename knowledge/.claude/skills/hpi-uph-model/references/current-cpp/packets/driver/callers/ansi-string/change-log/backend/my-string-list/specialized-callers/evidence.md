# 證據、機型及未完成範圍

[上層](index.md)；[完整正文／hash](source-manifest.json)；[字面普查](symbol-census.json)。

來源釘在 `26a6dcf0b8c1acf106a06e6cf4e700d20053d703`。新增 ShowMotorErrorMessage 以及 rs232std
GetLastLine／MyInsertToFile 三個完整函式、六來源路徑；三個既有 callee 只重核 context。
舊 46 份 source-manifest、入口 metadata、原文、裁決、HTML／byte snapshot 與相容入口保留。
三新正文分別為 145、19、37 行（含原摘錄終止換行）；不重算 canonical Skill 完成數。

## 普查界線

V906 的 .cpp／.h／.hpp／.cc／.cxx／.c 共 2050 檔；不搜尋 .svn。
移除一般 comments、字串與字元 literals 的位置保留遮罩後，找
`GetLastLine(`／`MyInsertToFile(`。結果包含宣告、定義和 test，沒有把每個命中當 caller。
production 的非宣告／定義字面位置是 fNote_ShowError 的兩次 method 呼叫；
tests/test_ptw1_mystringlist.cpp 另有 GetLastLine test 呼叫，本輪未執行它。
這是明確範圍的詞法資料，沒有解析 overload graph、raw string、宏、別名、函式指標或外部程式。
命中行號是 immutable source pin metadata；活文件用 function／變數定位。

## 共同項與差異

| 軸 | 適用界線 |
|---|---|
| HT9050／其他 Handler | 檔案行數、buffer、名稱及落盤分開的讀法共用；只有採這條 Public log caller 的流程才適用，未證明每台都啟用。 |
| 客戶／機型 | SPILFunction 決定欄位；其他客戶／MachineType 的完整前後流程待查，不由同名 log 推定。 |
| tester RS232 | namespace／signature／buffer 與 Handler 分開；902 歷史 banner 仍是 tester 原來源註記。 |
| 版本 | 本輪核現行 V906 C++17／UTF-8；V912、唯讀 V899、Steven 新 golden 913 及部署版本差異需另附來源。 |
| runtime | sLastFileName 和 D: log 路徑是分析對象；未讀寫 runtime／機台設定或觸發警報。 |

正文中 StopAllMotor、Gali_Command、IndexMotorBreakerOFF 是原流程，
本次只保存並分析，沒有呼叫它們。歷史 nm／ctest／golden 行號原文保存，不當成新驗證。
選定 motor alarm 的 F1 是 gated SaveJamCodeFile；FTP transport、服務／上傳目錄與
非 motor alarm path、notice ack 後更新、同步／重入、部署 heap／磁碟錯誤及 UPH S8 實機仍待續。
