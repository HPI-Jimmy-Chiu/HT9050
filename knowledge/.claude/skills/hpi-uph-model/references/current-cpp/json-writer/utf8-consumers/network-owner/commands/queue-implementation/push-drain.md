# 入列與取出

[上層](index.md)；[tryPush原文](raw/source-01.md#trypush)／[drain原文](raw/source-01.md#drain)。

## CommandQueue::tryPush

`WbGuard g(impl_->lock)`下比`q.size() < capacity`，複製WebCommand至deque，再寫`pushedUs`的steady_clock microseconds並更新peak。
capacity=0會拒絕每次push；size等於capacity時false，沒有等待空位或悄悄追加。
離開鎖後，成功時若ev存在先SetEvent，再accepted.fetch_add(1UL)；拒絕則rejected.fetch_add(1UL)，最後回bool。
SetEvent回傳未檢查；event失敗不改變本段accepted分支。成功入列不代表執行、browser已收到或query已回答。
原comment的caller owes browser ok:false ack保留；caller如何ACK見既有命令入口，這段queue本身不送ACK。

原設計「never waits」「UI stalled returns false」是歷史意圖；本體確有mutex／push_back配置／時間讀取。
無wait-for-space迴圈不能推成鎖完全不等待、無配置延遲或有最壞時限保證；未量測UI／socket或驗證全系統排程。
accepted／rejected統計在鎖外，與queue／event變更不是一個跨欄位原子快照。

## CommandQueue::drain

先建立local deque taken；鎖內只`q.swap(taken)`，鎖外讀n、`out.reserve(out.size()+n)`及front-to-back copy。
舊out內容保留，新取出命令以FIFO追加；回傳這次n，並drained.fetch_add(static_cast<unsigned long>(n))。
原O(1)-locked／UI THREAD ONLY／execution outside lock全文保存；本段只搬出命令，沒有執行命令。
reserve／copy在swap之後；配置例外、失敗恢復與完整caller生命期尚未查證，不宣稱交易式回復或全程thread-safe。
drainedCount是搬出計數，不能當完成／成功執行／UPH生產計數。
