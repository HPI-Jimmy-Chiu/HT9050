# Channel副本、HTTP回覆與reset

定位`UiChannel::Instance`／constructor／`Publish`／`Read`／`Clear`／`Post`／`Take`／`Dropped`、`W906_TesterCommHttp`、`KnownKey`、`HomeKeyOfTestType`、`ApplyUiCommand`／`SetComboIndex`／`ClearAllFlag`。
完整原文與兩個header見 [manifest](source-manifest.json)。Engine的RunOnce／Teardown原文沿 [前段版本](../20261008-online-pad/index.md)，不是重做已完成的driver單元。

## 讀取鏈

Engine把builder文字Publish到`gpib`；channel用WbGuard保護副本並增加seq。`Read`在key不存在或seq為0時回false，有值時複製json／seq；HTTP成功直接回json，沒有把channel seq額外包到body作為送達ack。

- HTTP base為`/api/testercomm`；不符合prefix／分隔符的path不接。base空key只允GET／HEAD並列出四key。
- KnownKey為gpib／rs232／tcpip／home。一般GET／HEAD：channel未發布回404，已有副本回200；home另有組合分流，不能把home成功等同GPIB成功。
- `HomeKeyOfTestType`：0→rs232（TTL）、1→gpib、2→rs232、3→tcpip，其餘空字串。
- wb_serve選定H1b區段把回覆交HTTP server；HEAD保留contentLength再清body。只保存此delegation與allowCmd安裝座，完整server／路由安裝、URL解碼及網路transport仍待驗。

`Clear`只erase snaps／cmds，沒有erase last_或歸零dropped_。seq增量與互斥副本，不保證跨程序重啟連續、實機freshness或HTTP成功送達。

## 入列與執行是不同步驟

| HTTP條件 | 返回 |
| --- | --- |
| POST且allowCmd=false | 403 |
| query cmd空或長度>512 | 400 |
| home不符合所選engine的site命令條件 | 200 queued:false |
| 同key／相同cmd距上次<400ms | 200 queued:false |
| 成功Post | 202 queued:true |
| 其餘method | 405 |

`Post`先檢查exact repeat、再更新last_；queue上限64，滿了丟最舊並增加dropped_，新命令仍可入列。`Take`複製佇列再清空。
202是入列結果，不代表engine已Take、命令已執行、測試機已回答或畫面已更新。allowCmd全啟動流程未查完，原註解的always-true敘述不提升為本輪runtime結論。

## reset及解析界線

`ApplyUiCommand("reset flags")`呼叫`ClearAllFlag`並回true。該body寫log、把iMainTask／iGbibTask設1、清LastSet多個flags與IsTest，並呼叫`InitialBarcodeList`及`WriteLastDataFile`；它包含保存呼叫，這不是純畫面操作。本段未呼叫此命令，也未驗深層保存／機台副作用。

`SetComboIndex`對<-1或>=Items count直接返回；caller仍可能呼叫對應handler及回true。因此true不能單獨證明選值已變。site索引用atoi，這段不是對全部輸入的嚴格語法驗證；widget enabled／指標檢查只是此分支條件，不代表完整機台安全互鎖。

回 [入口](index.md)；browser POST回覆消費見 [瀏覽器](browser.md)。
