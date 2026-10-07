# UPH 指令轉送與 bridge 局部鏈

pin `6690e980945e8ab5adf6708b1372c4fb1bf917bb`；[manifest](source-manifest.json)保存V906／V912六source、七完整函式文字／body hash及兩宣告。只查本地轉送、註冊／清除與包裝，不代表訊息送達、真機或整個UPH鏈已驗。

- [轉送與版本差異](flow.md)：`SendMSG_CMD(int, AnsiString)`、`SendMSG_CMD_Msg`、`FwdSendCmdMsg`、`SendToBridge`。
- [未查範圍與機型](limits.md)：callback生命週期、接收端、共同Handler／HT9050界線。
- 固定source：[Form wrapper](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6690e980945e8ab5adf6708b1372c4fb1bf917bb/HT9011UC_Cpp_V3.33.906.0/forms/fMain.cpp)、[轉送表](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6690e980945e8ab5adf6708b1372c4fb1bf917bb/HT9011UC_Cpp_V3.33.906.0/forms/fMain.h)、[Handler包裝](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6690e980945e8ab5adf6708b1372c4fb1bf917bb/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerBridgeCtl.cpp)、[bridge呼叫](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6690e980945e8ab5adf6708b1372c4fb1bf917bb/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerTesterSide.cpp)、[註冊／shutdown](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6690e980945e8ab5adf6708b1372c4fb1bf917bb/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/TesterCommWiring.cpp)、[V912](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6690e980945e8ab5adf6708b1372c4fb1bf917bb/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/main.cpp)。

活正文以function／欄位定位，source註解行號只保留作歷史。回[Command取值](../index.md)。

[SendToEngine／mailbox子樹](mailbox/index.md)補本地返回、反向pump與逾時分支；engine／接收端與完整生命週期仍待查。
