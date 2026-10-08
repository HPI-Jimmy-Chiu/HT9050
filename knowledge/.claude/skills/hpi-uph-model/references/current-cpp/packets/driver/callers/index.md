# Factory、注入呼叫端與測試界線

來源 pin `a7977dfa321d0d0ba7e6b75aa28ee1137817d38b`；七個UTF-8來源、九個完整cpp定義（正式初始化一個／測試八個）、七組建置宣告，共十六原文片段與hash在 [manifest](source-manifest.json)。本層仍屬同一UPH Skill。

- [正式factory](factory.md)：W906_TesterCommInit、g_inited、HT9045_TESTERCOMM。
- [Sim與IPC測試](tests.md)：注入物件、正常結束次序、fixture與opt-in範圍。
- [建置宣告](build.md)：SOFT_SIMULTE及三個ctest入口；沒有執行結果。
- [版本、機型與未查範圍](limits.md)：HT9050連線斷言與容量／ABI／實機分開。

- [TesterCommWiring.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/a7977dfa321d0d0ba7e6b75aa28ee1137817d38b/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/TesterCommWiring.cpp)：blob `d39313d6e99ac35d5b419cbb05c8f09fb3501572`。
- [test_testercomm_gpib.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/a7977dfa321d0d0ba7e6b75aa28ee1137817d38b/HT9011UC_Cpp_V3.33.906.0/tests/test_testercomm_gpib.cpp)：blob `44f6277ac0d9f1d273a19695dafc21fdc274099f`。
- [test_testercomm_handler.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/a7977dfa321d0d0ba7e6b75aa28ee1137817d38b/HT9011UC_Cpp_V3.33.906.0/tests/test_testercomm_handler.cpp)：blob `309d6d1913cab376d5e93eaeed0be95077f696b5`。
- [test_testercomm_ipc.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/a7977dfa321d0d0ba7e6b75aa28ee1137817d38b/HT9011UC_Cpp_V3.33.906.0/tests/test_testercomm_ipc.cpp)：blob `517367383de262e02db26ed67720a00bc2e781aa`。
- [CMakeLists.txt](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/a7977dfa321d0d0ba7e6b75aa28ee1137817d38b/HT9011UC_Cpp_V3.33.906.0/CMakeLists.txt)：blob `be1f43f5af6a31bf7a21944770b8b90216fef681`。
- [CMakeLists.txt](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/a7977dfa321d0d0ba7e6b75aa28ee1137817d38b/HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt)：blob `6cd1a504890a8128d4973f08e68a0ca23f6a7f13`。
- [MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/a7977dfa321d0d0ba7e6b75aa28ee1137817d38b/HT9011UC_Cpp_V3.33.906.0/MachineType.h)：blob `a2af14c500b0e3dd637651f0773ad04c842fe7e7`。

回 [driver索引](../index.md)、[建立與解除](../lifecycle/index.md)、[NI／Sim](../implementations/index.md)。

## 初始化前置與介面選擇

[前置／連線規則／有效TestType](init-selection/index.md) 補查opt-out之前的hook安裝、socket建立及非ON_LINE介面選擇；仍分清callee未閉合與runtime未驗證。

## 設定發布與Hub／thread

[設定／Aux recipe／Hub啟停](settings-hub/index.md) 接續PublishSettings與GpibAux的callee，分清seed副作用、非同步Start、Stop出口及客戶INI分流；完整caller、容量／ABI／實機仍待續。

## Tick 與 Aux 設定 consumer

[Tick／bridge設定／Aux COM](tick-consumer/index.md) 補查設定發布次序、QA close guard、INI／recipe consumer 與 COM/SIM 啟停；等待結果、完整 caller 與機台量測仍分開查證。

## Aux 表單與 transport 設定

[Aux 所有權／表單保存／DCB](form-settings/index.md) 接續 consumer 的 constructor／destructor、欄位與 INI 映射及 ApplyCommState_，分清 timeout、歷史 header 與當前來源；完整 caller／driver／實機仍待續。

## Callback 與接收佇列生命期

[reader／SIM callback、Impl與QueueRx／DrainRx](callback-lifetime/index.md) 補查 buffer 複製、closeRequested派送次序及有限等待的證據界線；不把排入佇列當送達或完整shutdown已驗證。
