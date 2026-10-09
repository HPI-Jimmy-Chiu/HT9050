# Notice 局部證據與適用範圍

[上層](index.md)；[來源 manifest](source-manifest.json)；[字面普查](symbol-census.json)。

來源 `c90d8d22bb2d34c532443386469de717aef2e671`；4 個現行 V906 檔案，6 完整 CPP／4 header inline／6 選定區段，
16 原文摘錄、18 reader 分頁。對 47 舊 manifest 用同路徑原文包含比對，
忽略尾端換行而不改正文；本段沒有重算已保存完整函式。
原文保留 template、enum、snapshot、gates、metadata 與歷史裁決註解。

## 驗證的界線

2051 個 .cpp／.h／.hpp／.cc／.cxx／.c，以一般 comments／string／char 遮罩
搜尋 10 個指定符號；不搜尋 .svn。命中含 prototype、definition、call 與 test，
沒有把每個命中算成 caller；宏、raw string、函式指標、外部 link 未作語意解析。
source offset／普查行號只屬釘住版本的保存 metadata；活文件以 function／變數定位。
推前核來源 hash、重組分頁、引用／錨點、入口與舊資源，再整合新 main 重驗。
沒有執行 tests/test_notice_ack.cpp、build、link、timer、DB、馬達或通訊。

| 差異軸 | 本輪界線 |
|---|---|
| HT9050／其他 Handler | snapshot、時間與回執分開的讀法共用；只有採這條 V906 notice host 的機台才可套選定流程，沒有證明全部機台啟用。 |
| 客戶 | active ASE_SG refusal、KYEC_LEE retest 小段依源碼條件；SPIL event log 欄位見上層，不推定所有客戶相同。 |
| 版本 | 核現行 V906 C++17／UTF-8；歷史 906 golden／V912 行號是原註記，未重新比較 V912／唯讀 V899／新913完整正文。 |
| runtime | 分析本檔 static／全域值與 host 狀態，不讀寫機台設定或操作現場。 |

下一段可接完整 held carry／retire writer／auth verify／timer 和其他 wait 入口；
FTP／upload、mode／checkbox、thread／reentry、UPH S8容量／校正實機仍未完成。
本段是文件整理局部完成，沒有宣稱整個 Skill、golden close 或機台驗證完成。
