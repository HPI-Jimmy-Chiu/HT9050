# 警報插入 caller 與 RS232 同名方法

[上層檔案型清單](../index.md)。本輪從既有 [專用 writer](../specialized.md)
接到 `forms/fNote_ShowError.cpp::ShowMotorErrorMessage` 的實際插入呼叫者，
並分開 `TesterComm/Rs232` 的同名方法。來源釘在 `26a6dcf0b8c1acf106a06e6cf4e700d20053d703`。

| 要查的問題 | 文件 |
|---|---|
| 先記行數、兩次 flush、插入 index 與欄位 | [Handler 警報插入](handler.md) |
| RS232 同名方法的 buffer、補空格及例外界線 | [RS232 專用方法](rs232.md) |
| 普查範圍、機型／客戶、版本及未查項 | [證據與適用界線](evidence.md) |
| 三個新完整函式、六來源 hash 與三既有 context | [來源清單](source-manifest.json) |
| 2050 份來源檔的兩個符號字面命中 | [普查資料](symbol-census.json) |

新增 3 完整 CPP function、3 原文頁；Public 的 GetLastLine／MyInsertToFile
及 LogObjects 的 SaveEventLog 只重核既有正文，不算新增完成。
這是靜態流程查證；沒有執行警報、檔案 writer、馬達、通訊、build 或實機測試。

## Notice capture／ack 延伸（20261009）

[計時、退役、auth 與 held 回覆](notice/index.md)：新增6完整CPP、4header inline、6選定區段；PassTime未在capture重設、motor與running gate差異、mailbox先退役後close及held先回應的界線已記，完整延後鏈待續。
