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

## Aux 設定的 INI 共用 callee

[磁碟／memory 綁定、讀取／解析及寫入界線](ini-core/index.md) 接續 Aux SaveSetupData；分清 found、writeThrough_、歷史 header 與磁碟失敗未回傳，完整 Fast writer／store／caller 仍待續。

## INI Fast writer 與 memory store

[格式、重複名稱、記憶體與保存生命期](ini-writer-store/index.md) 接續INI共用callee；分開Fast／memory解析差異、第一個section/key、失敗不回傳與歷史destructor敘述，typed API及完整caller仍待續。

## INI typed API 與磁碟列舉

[預設、數字日期與名稱清單](ini-typed-enumeration/index.md) 補磁碟／memory分流、NUL停止、16384-byte截斷與Exists差異；深層日期／字串轉換、容器及完整caller仍待續。

## INI 日期與字串轉換

[Trim／整數、日期解析、serial與格式精度](ini-conversion/index.md) 補查typed API的轉換callee；分開非空失敗回0與caller def、Word／floor、month/minute與秒精度。完整字串容器、caller、BCB6及實機仍待續。

## TStrings／TStringList共用容器

[字串清單樹](string-list/index.md) 接續INI／GPIB consumer，分開核心、Text／proxy與後續parser／IO；保存舊pin／正文，機型／客戶caller及runtime仍待查。

## AnsiString byte與比較續查

[byte／搜尋編輯／比較與case-copy](ansi-string/bytes/index.md)：接續TStrings／INI的char*與NUL、1-based byte、比較callee；numeric／printf、locale／ABI／完整caller與runtime仍續查。

- [AnsiString數值／printf／串接局部](ansi-string/numeric-format/index.md)：assignUInt／assignDouble、ToInt／ToDouble、formatString／conv與operator+；7cpp／29inline／3歷史裁決39原文，ABI／locale／caller與913 RTL仍待查。

- [SysUtils數值與所選caller](ansi-string/sysutils-numeric/index.md)：free numeric／Format、picture與locale正文界線、UPH %s適配及Change Log numeric分類；13cpp／2inline／3interface18原文，完整caller／913／runtime續查。

- [Change Log數值caller／單位與hook](ansi-string/change-log/index.md)：22cpp／1header／5region28原文；printf實參、double gate／MM int與static buffer、caption與TempChangeLog／wb_serve接線；完整儲存／ABI／913／runtime另續。
