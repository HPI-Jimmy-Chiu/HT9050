# carry保留順序與巢狀wait

[上層](index.md)；[W906_TakeCarry完整body](raw/function-01-part-01.md)。其同一行全域宣告／後置註解另以context完整保存。

TakeCarry在g_carry.empty時false；非空時按deque順序push_back到caller的out，清carry、g_carryRunnable=false並true。
它不清out，也不drainqueue；兩個modal caller先local.clear，因此這裡的append不等於queue整批替換契約。
本單元沿用[CommandQueue實作](../queue-implementation/index.md)，不重計其十二函式或以carry推global有界。

alarm與YESNO wait先取carry，再drain追加fresh queue；不是二選一，carry既有項排在fresh前面。答案接受後local尚未走過的i+1..按序insert到carry.begin前面。
這保住已drain的餘項與後續carry次序；在close／Start可能再進modal之前就設g_carryRunnable，新的modal仍能讀到它們。
不是mutex或多thread安全證明：g_carry／runnable／qid／s_showing／busy是宿主全域／static狀態，本輪未做所有寫入者與thread graph。

output-first每次dispatch後重新從carry頭掃描，因dispatch本身可觸發modal而消耗carry；先複製wc再erase，避免沿用失效deque索引。
barrier時不把fresh從有界queue移到carry，是局部防累積策略；carry本體依原comment是unbounded，不推總pending量上限。
legacy註解中capacity64與這些時間值不等於現場queue設定、UPH產能／HP或tray容量；部署與效能需另查。
