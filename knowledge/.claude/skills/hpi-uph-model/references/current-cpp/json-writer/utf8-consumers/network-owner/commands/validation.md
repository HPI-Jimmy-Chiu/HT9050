# HandleTextMessage：JSON與拒絕順序

[上層](index.md)；[完整body](raw/source-01.md)；[IsSaneName原文](raw/source-05.md)；[控制權](control.md)／[queue](queue.md)。

本body以cJSON_Parse(text.c_str())解析。失敗增cmdRejected，SendAck(id=0, ok=false, malformed json)後return。
欄位用cJSON_GetObjectItemCaseSensitive：type／cmd／tag須是string且valuestring非空指標才取字串；id以cJSON_IsNumber判haveId，值取valuedouble，否則0。
type=ping立即成功ACK並Delete root；它未經後面的cmd id／readOnly／queue／owner gate。
type不等於cmd或ping時Delete root後忽略，不記為cmdAccepted或cmdRejected。
cmd frame的RECV hook在欄位驗證之前；它表示收到frame，並不表示指令獲准或機台動作完成。

| 拒絕檢查次序 | 實際body |
|---|---|
| id | 必須numeric；此body沒有另加isfinite、整數或range檢查，cJSON數字解析與全部caller仍待查 |
| cmd | 空字串拒絕；IsSaneName最多64個byte，僅ASCII a-z／A-Z／0-9／點／底線／連字號 |
| tag | 非空時才查同一字符表，最多128個byte；缺tag與空字串都走空tagName |
| value | absent／null → MakeNull；bool／number／string可轉TagValue，其餘型別拒絕 |
| bridge | 尚未拒絕才查readOnly，之後查queue指標；readOnly cmd連控制權與豁免指令都不通過 |
| owner | 控制權操作與豁免見控制權頁，其餘要ctrlOwner_等於目前c.id |
| queue | 正文先登記pending，QueuePush false再回command queue full；不把每種false成因當成已驗queue容量 |

IsSaneName自身會接受空字串（迴圈零次），但cmd上游另擋空字串；tag空字串刻意跳過這個helper。
value number直接取valuedouble；值域、每個cmd的型別及硬體互鎖不是這個通用body完成的工作。
String經std::string(valuestring)建構，本頁不推定完整Unicode或內嵌NUL round trip；UTF8接收原責任見[既有owner](../index.md)。

cmdRejected只在malformed JSON、一般reject非空及QueuePush失敗三段增加。
control-held或release的not-operator走直接SendAck分支後return，本body沒有在那些分支增加cmdRejected。
cmdAccepted在QueuePush成功之後增加；它不是已dispatch、peer已收到ACK或機台已完成的計數。
Delete root逐正常分支保存於原文；本次沒有完成exception路徑／memory safety的全圖驗證。
