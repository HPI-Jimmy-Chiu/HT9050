# Driver 建立、選擇與本地解除

pin `7de72296fb235b3abe2169ed0bef5a529e15742b`；兩個UTF-8來源、八個完整cpp函式與兩組宣告／十個原文片段在 [manifest](source-manifest.json)。保留在同一UPH Skill，與 [NI／Sim實作](../implementations/index.md) 銜接。

- [注入與Start選擇](selection.md)：g_injected、g_live、ownedDriver_與開始結果。
- [停止與解除](cleanup.md)：DoClose／Stop／Teardown／dtor的本地順序。
- [版本、機型與未查範圍](limits.md)：使用者、全caller、原子指標壽命與建置選擇仍待查。

來源：

- [GpibEngine.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/7de72296fb235b3abe2169ed0bef5a529e15742b/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibEngine.h)，blob `179ab4ca4f0f45c4597f7c9d190073613935c26d`。
- [GpibEngine.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/7de72296fb235b3abe2169ed0bef5a529e15742b/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibEngine.cpp)，blob `d671029e13e0de9aa333e10ca5e34bae55f9dba4`。

回 [driver索引](../index.md)；既有Hub／thread生命周期見 [先前engine查證](../../../consumers/command/transport/mailbox/engine/lifecycle.md)，payload body仍沿其 [payload](../../../consumers/command/transport/mailbox/engine/payload/index.md)，本層不重算既有完成數。
