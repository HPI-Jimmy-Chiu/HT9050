# 初始化前置與有效介面選擇

來源pin `f271ad84cb7e9329a559c97e2519d8fc5fcf9f73`；四個UTF-8來源、十二個完整cpp定義與原文hash見 [manifest](source-manifest.json)。這是同一UPH Skill的局部caller查證，接續上一層factory。

- [opt-out前置與socket建立](preinit.md)：W906_TesterConnectRulesInstall、W906_CmdServersEnsure、W906_TcpServersCreate與PumpInit。
- [連線規則hook](rules.md)：安裝指標與五個callback的來源行為分開。
- [有效介面與啟動選擇](selection.md)：OffLineGpibWay、EffectiveBridgeTestType、StartBridgeProgram。
- [版本／機型／客戶界線](limits.md)：共同條件、設定差異、NI／Sim與完整runtime未查範圍。

- [HandlerTesterConnect.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f271ad84cb7e9329a559c97e2519d8fc5fcf9f73/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerTesterConnect.cpp)：blob `fd1b0ec4b7b2ef123526763f6b1f0eb9c51df9b0`。
- [CmdServerPump.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f271ad84cb7e9329a559c97e2519d8fc5fcf9f73/HT9011UC_Cpp_V3.33.906.0/TesterComm/Tcp/CmdServerPump.cpp)：blob `f38980f1c0ea735986f883773e36ad679a5970e1`。
- [fMain.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f271ad84cb7e9329a559c97e2519d8fc5fcf9f73/HT9011UC_Cpp_V3.33.906.0/forms/fMain.cpp)：blob `f415f90b76866dd0b7e5e5e90e868f10cb1e6100`。
- [HandlerTesterSide.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f271ad84cb7e9329a559c97e2519d8fc5fcf9f73/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerTesterSide.cpp)：blob `c2934ec58fbc8d8150c762ed99c2c33c7f164b21`。

回 [factory與測試索引](../index.md)、[driver建立與解除](../../lifecycle/index.md)。
