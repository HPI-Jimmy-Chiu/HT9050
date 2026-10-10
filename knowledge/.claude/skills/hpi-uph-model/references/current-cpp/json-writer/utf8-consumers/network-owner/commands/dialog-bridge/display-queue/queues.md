# requestId去重、FIFO與drain

[上層](index.md)／[alreadySeen](locators.md#alreadyseen)／[enqueue](locators.md#enqueue)／[drain](locators.md#drain)。
來源 `web/page/dialog-bridge.js` 的queueStop／queueNS／seenIds；原完整頁與常數沿用[保存證據](evidence.md)。

## alreadySeen

falsey id直接false且不加入seenIds；truthy id以indexOf比對，已存在回true，否則push。
seenIds超過SEEN_MAX=64時shift最舊一筆；這是有界記錄，移出後同id可再次入列。
只比requestId，不帶channel、kind、requestSeq；seenIds由兩queue共同使用，沒有每channel獨立集合。
成功push seenIds發生在enqueue放queue之前；丟舊、出列、關窗並未在本段把id移出seenIds。

## enqueue

alreadySeen(request && request.requestId)為true時回false，沒有建立item。
其餘建立kind／displayKind／request／Date.now()的at；request物件直接引用，沒有複製或完整schema檢查。
isNonStop(displayKind)決定queueNS或queueStop，依傳入displayKind而非原kind分流。

| queue | 規則 | 本body的回覆 |
|---|---|---|
| queueStop | push FIFO；length >= STOP_DEPTH_WARN=3時console.error，保留所有項目 | 無submit或丟棄回覆 |
| queueNS | push FIFO；length > NS_QUEUE_MAX=8時持續shift最舊項並console.warn | 無submit或丟棄回覆 |

STOP_DEPTH_WARN的原註解寫「超過」，實際body是>=；本文件依條件描述，原註解保存不改。
兩項上限不可互換：3是stop警告門檻，8是nonstop等待queue容量；activeNS是另外的已顯示物件，不算進queueNS.length。
正常完成後回true表示入列，不證明畫面已顯示、回覆已送達或C++採用。
console列舉code時讀request.arguments；任意不合格式的request不由本body普遍防護。原文保留，不以本輪文件整理修程式。

## drain

先在!active且queueStop非空時shift一筆並render；再在!activeNS且queueNS非空時shift一筆並renderNonStop。
兩個if分別看自己的active，可在同一輪各顯示一筆；不是全域單一modal鎖。
shift發生在render之前；DOM或sendRequest同步throw時本body沒有重新入列，且stop那次throw會阻止本輪後續nonstop分支。
本body不while清空所有queue。關窗後由後續drain或輪詢取下一筆，closeNonStop／completeNonStop本身沒有呼叫drain。
近期輪詢的lastSeq推進與enqueue順序見[recent路由](../close-routing/polling.md)；讀到入列、顯示與ACK是不同階段。
