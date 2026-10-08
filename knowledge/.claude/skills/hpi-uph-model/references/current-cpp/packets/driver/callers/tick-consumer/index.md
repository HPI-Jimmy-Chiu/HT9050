# Tick、設定 consumer 與 Aux COM 啟停

同一 UPH Skill 的下一層局部證據：四個 UTF-8 來源、十二個完整 cpp 定義。來源 pin `06f5742d99f9e2436ffc8e796eecf248d6a6be9d`、原文與 hash 見 [manifest](source-manifest.json)。只讀來源，沒有執行函式、開啟 COM 或修改 INI。

- [Tick 次序](tick.md)：g_inited、PollHandler、設定發布與 Aux 重啟請求。
- [Bridge 同步與關閉](bridge.md)：SPEA／TCP／GPIB／RS232／TTL 分流、QA 與客戶 guard。
- [Aux 設定 consumer](consumer.md)：INI 初值、recipe framing、客戶預設與連線標記。
- [TComm 啟停](transport.md)：明確 SIM、COM fallback、reader／writer 與 closer 等待。
- [機型、版本與未查界線](limits.md)：HT9050 與其他 Handler 的共同入口及待查差異。

回 [caller 索引](../index.md)、[設定發布與 Hub](../settings-hub/index.md)。此層未完成完整 caller、封包容量／ABI 或實機 UPH 語意驗證。

## 來源定位

- [TesterCommWiring.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06f5742d99f9e2436ffc8e796eecf248d6a6be9d/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/TesterCommWiring.cpp)：blob `d39313d6e99ac35d5b419cbb05c8f09fb3501572`。
- [GpibAux.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06f5742d99f9e2436ffc8e796eecf248d6a6be9d/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibAux.cpp)：blob `54c045def600083e8b643d0f877cafbca3eccc17`。
- [HandlerBridgeCtl.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06f5742d99f9e2436ffc8e796eecf248d6a6be9d/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerBridgeCtl.cpp)：blob `cbb4d86c63045ea649cec9bcdf807dad1b6b2f67`。
- [Comm.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06f5742d99f9e2436ffc8e796eecf248d6a6be9d/HT9011UC_Cpp_V3.33.906.0/vclcompat/Comm.cpp)：blob `2aabfcc00f8debf78993a33fcd1ec73c3d17335d`。
