# QueuePush：id與presence交界

[上層](index.md)；[HandleTextMessage](raw/source-01.md)／[adapter](raw/source-06.md)／[CommandQueue.h全文](raw/source-04.md)；[既有ACK](../pumps/ack.md)。

| 欄位／位置 | 這條現行路徑 |
|---|---|
| browser id | cJSON valuedouble；存PendingAck.browserId，SendAck／CompleteCommand用它回ACK |
| ticket | nextTicket_++產生unsigned long long；pending_ map key，傳QueuePush並指定WebCommand.id |
| connId | PendingAck與WebCommand都帶c.id；區分不同連線並供ACK路由，非指標 |
| tag presence | QueuePush指定hasTag=!tag.empty()，absent與空tag在此都成false |
| value presence | QueuePush指定hasValue=!value.isNull()，absent與explicit null在此都成false；有效空string不等於Null |

CommandQueue.h原comment稱WebCommand.id為browser-supplied correlation、hasTag/hasValue用於區分absent與present null／empty。
原契約完整保存；本QueuePush實際指定generated ticket，並按字串空／Null建立flags。文件同時呈現原意圖與這個caller的實際值，不修改原comment掩蓋差異。
這個adapter回q->tryPush(c)；header所述有界queue／UI timer／waitForPush與critical-section等待是原設計契約，queue實作及完整drain／dispatch仍待下一層。
kDefaultCapacity=64、capacity=0拒絕全部的原說明已保存；server的kMaxPendingAcks=4096是另一種上限，不能混成一個queue capacity。

HandleTextMessage在QueuePush之前持pendMx_登記pending_[ticket]、push pendingOrder_。
pendingOrder_長度超過4096時從front erase pending_並pop_front；這是待ACK slot eviction，並未從CommandQueue撤回已推入的動作。
原P25c「先登記再push」race修正全文保存；舊if(false)登記段亦原樣保留，不重執行或刪掉。
QueuePush失敗時erase pending_[ticket]並失敗ACK；本body未從pendingOrder_移除該ticket，後續容量影響留給完整caller／實作，不先定性為bug。
成功後增cmdAccepted，並不立即ACK；原comment說由UI處理後CompleteCommand回覆，實際dispatch需另查。
CompleteCommand不再計本單元函式；既有PendingAck／Outgoing、pending fields及Q30理由見[pumps證據](../pumps/index.md)。
