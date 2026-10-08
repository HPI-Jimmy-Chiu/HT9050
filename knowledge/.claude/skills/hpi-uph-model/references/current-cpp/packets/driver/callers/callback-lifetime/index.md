# TComm callback、Impl 與接收佇列

同一 UPH Skill 的局部延伸：三個 UTF-8 來源、六個完整 cpp 定義與一個完整 Impl struct；pin `576b3eb3dd5851131526ee3bd8cf67a52100a47c`、原文與 hash 見 [manifest](source-manifest.json)。三個 cpp 與 struct 接續上一輪 intake 並重驗，三個 cpp 新增選讀。

- [釋放與 Impl](lifetime.md)：TComm destructor、Impl 初值與資源釋放的證據界線。
- [reader／SIM callback](callbacks.md)：直接呼叫、暫存 buffer 與停止旗標。
- [佇列與 UI caller](queue.md)：QueueRx 複製、DrainRx 分流與 Update 按鈕。
- [版本、機型與待查](limits.md)：共同項／差異、歷史量測與 runtime 界線。

回 [caller 索引](../index.md)、[Aux 表單／DCB](../form-settings/index.md)、[stop／closer](../tick-consumer/transport.md)。沒有呼叫選讀函式、注入資料、開 COM、寫 INI 或啟動機台；完整生命期與實機 UPH 仍待續。

## 來源定位

- [Comm.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/576b3eb3dd5851131526ee3bd8cf67a52100a47c/HT9011UC_Cpp_V3.33.906.0/vclcompat/Comm.cpp)：blob `2aabfcc00f8debf78993a33fcd1ec73c3d17335d`。
- [GpibEngine.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/576b3eb3dd5851131526ee3bd8cf67a52100a47c/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibEngine.cpp)：blob `d671029e13e0de9aa333e10ca5e34bae55f9dba4`。
- [GpibUi.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/576b3eb3dd5851131526ee3bd8cf67a52100a47c/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibUi.cpp)：blob `e0f8c1fbf2b85783a4f89b4be7d802fad9f38488`。
