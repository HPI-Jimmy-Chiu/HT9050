# ACK文字、ticket與queue

[上層](index.md)；[AckJson](raw/source-01.md)／[SendAck](raw/source-02.md)／[CompleteCommand](raw/source-05.md)；[structs](raw/source-09.md)／[fields](raw/source-10.md)。

AckJson用ostringstream輸出type=ack與id；先static_cast<long long>(id)，再以轉回double是否等於id選整數或double格式。
此body沒有先檢查id有限性／範圍；上游輸入限制及所有caller仍待查，不把本文當成完整數字格式契約或已發生故障。
ok=false用sib::QuoteString(error)輸出error；該adapter轉JsonQuote，見[原實作](raw/source-12.md)與[既有JSON quote](../../../values/quote.md)。
ok=true只以首尾大括號及長度>=2判斷第三參數：去掉括號，inner非空即用逗號直接拼進ack頂層。
此處未parse inner，也沒有在body拒絕重名key；空物件或不符首尾形狀的success字串不添欄位。
原Steven20260916說明含changed／identical／notFound、browser規則及修正理由，全文保留；本輪未執行browser或核全部handler輸出。
原D:\docs與backup ChangeLog位置只保存為歷史metadata，並未宣稱這次讀過或重作當日修正。

| 入口 | 正文順序／證據界線 |
|---|---|
| SendAck | SendJson(AckJson(...))、可選ACK log hook、持statsMx增acksSent；enqueue並非peer receipt |
| CompleteCommand | 持pendMx查ticket；找不到直接return，找到複製connId／browserId並erase pending_；放鎖後持outMx建Outgoing／AckJson並push，放鎖後Wake與可選DONE hook |
| PendingAck | 保存connId與double browserId；ticket是pending_ key；兩種id勿混為同一欄位 |
| 原「connection already gone」comment | 保存為原理由；找不到ticket的所有可能成因不是本body能證明的 |

CompleteCommand在本body沒有從pendingOrder_刪ticket。容量／eviction／close路徑須接續HandleTextMessage與既有CloseConn對照，不先定性為bug。
CompleteCommand→PumpOutgoing→SendJson這條路徑不經SendAck，不能把acksSent推為所有完成ACK數。
DONE hook在原comment標tick thread；字面「Neither blocks」亦為原意圖。實際body有WbGuard與字串／queue工作，本輪沒有量測阻塞或deadline。
清pending或queue不等於撤回已被owner取走的機台動作；收到command result／peer顯示／硬體完成要分別查證。
