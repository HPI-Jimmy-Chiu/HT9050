# DoSystemMessage 的排程與刷新條件

來源：[V906 ckernel.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Cpp_V3.33.906.0/ckernel.cpp)、[V912 ckernel.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ckernel.cpp)，宣告：[V906 ckernel.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Cpp_V3.33.906.0/ckernel.h)、[V912 ckernel.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ckernel.h)；pin `938ebc37e95314ad496b03c7c257e1abc5fb9799`。定位DoSystemMessage的iMyCounter，以及ShowRunLabel的OldFlushFlag／FlushFlag、OldiHeaterWaitTime／iHeaterWaitTime。

兩版所選DoSystemMessage完整body相同：local static iMyCounter初值0；phase 0依序呼叫ShowRunLed、ShowRunLabel，phase 3呼叫DoPanelLamp，每次後遞增，>=6回0。這是正常走完body時的六次呼叫輪轉；沒有讀ShowRunLed／DoPanelLamp完整callee、全部上層caller、重入／thread或實際tick，不能換算成固定毫秒週期。

兩版ShowRunLabel的已讀入口條件如下，順序不可合併成「只要FlushFlag為true就記時」：

1. OldFlushFlag==FlushFlag且OldiHeaterWaitTime==iHeaterWaitTime時先return。
2. 通過後先把OldFlushFlag設為FlushFlag；若FlushFlag為false且OldiHeaterWaitTime==iHeaterWaitTime，再return。
3. 通過兩條後才更新OldiHeaterWaitTime，接著還有其他狀態判斷。

入口另有FixDoor、Alarm、LOCK、EMG／Power Off的早退。讀到條件與return不表示安全IO、訊號、helper或機型設定已驗證。V906／V912 Alarm和PLC條件差異見 [界線](limits.md)。

排到ShowRunLabel、通過刷新條件、顯示PAUSE，以及 [暫停開始寫入](pause-start.md) 是不同層次。還要通過早退／SystemStart／先行狀態分支並檢查bCalculatePauseTime，才會執行所選開始值賦值；本層不宣稱每個pause事件一定被計入一次。
