# Object與Array的入棧／退棧

[上層](index.md)；[BeforeValue](../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/auth-callees/login-json/raw-value.md)；
[BeginObject](raw/source-02.md)、[EndObject](raw/source-03.md)、
[BeginArray](raw/source-04.md)、[EndArray](raw/source-05.md)原文。

| 方法 | 正文狀態轉換 |
|---|---|
| BeginObject | BeforeValue → 附加{ → push kCtxObject → needComma_=false |
| BeginArray | BeforeValue → 附加[ → push kCtxArray → needComma_=false |
| EndObject | 核頂層Object及無pending key → pop → 附加} → needComma_=true |
| EndArray | 核頂層Array及無pending key → pop → 附加] → needComma_=true |

兩個Begin沒有依ok_早退；BeforeValue報錯後，Begin仍附加開符號、入棧並改旗標。
BeforeValue可能先加逗號，因此不能只看Begin正文的開符號當成全部buffer變更。

兩個End在stack空、頂層型別不符或keyPending_為true時設ok_=false並早退；
該錯誤路徑沒有pop、附加結束符號或改needComma_。這與Key／Begin報錯後繼續寫入不同。
正常End只設共同needComma_=true，沒有從逐層context還原舊comma旗標的程式。
不要由容器堆疊存在推定每層另存全部writer狀態。

選定四正文沒有機型／客戶分支；實際publisher或value方法仍可能分流。
本輪沒有跑巢狀容器、錯誤序列、JSON parser、瀏覽器或機台驗證。
