# V906 GPIB快照、HTTP消費與頁面hook

固定來源 `530357c6d489ee2a9cafda718cf294f07c6af7d0`；接續 [解除與位址觀測](../20261008-online-pad/index.md)，舊節點保持原pin。
13來源、30完整cpp定義、7完整JS定義、2完整header、1完整struct與17選定區段，共57原文及byte／text／body hash見 [manifest](source-manifest.json)。選定區段不冒充完整wb_serve、WebBridgeTags或browser查證。

| 問題 | 路由 |
| --- | --- |
| raw欄位、LED、widget、log及JSON字串 | [快照](snapshot.md) |
| channel副本、HTTP狀態、入列與reset | [HTTP及命令](http.md) |
| hook安裝、TSerialPoll列、want／wseq與tag | [頁面程式狀態](pages.md) |
| 輪詢、raw欄位未直接呈現、每頁want消費 | [瀏覽器](browser.md) |
| HT9050／其他Handler、客戶、runtime與剩餘 | [版本界線](limits.md) |

## 固定來源

- [HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibUiSnapshot.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibUiSnapshot.cpp)，blob `e457c03b2c4f5552329b0921b6c8d8edd8e88346`。
- [HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibUi.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibUi.cpp)，blob `e0f8c1fbf2b85783a4f89b4be7d802fad9f38488`。
- [HT9011UC_Cpp_V3.33.906.0/TesterComm/UiChannel.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/TesterComm/UiChannel.cpp)，blob `a72129640ca95a918b6bae73a123aaf42d4d6c8e`。
- [HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/TesterCommWiring.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/TesterCommWiring.cpp)，blob `d39313d6e99ac35d5b419cbb05c8f09fb3501572`。
- [HT9011UC_Cpp_V3.33.906.0/TesterComm/UiHome.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/TesterComm/UiHome.cpp)，blob `ae4e689090046c4643c015fa59cb206af52a5c12`。
- [HT9011UC_Cpp_V3.33.906.0/WebPageTable.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/WebPageTable.cpp)，blob `1cac422a52da19d63e50d4d6084e50d80241230b`。
- [HT9011UC_Cpp_V3.33.906.0/WebBridgeTags.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/WebBridgeTags.cpp)，blob `43a1bbe92fe47837726ad0d15d68db76bff21226`。
- [web/page/testercomm.html](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/web/page/testercomm.html)，blob `58b336469012e8a0e34e9742467a275436f8979c`。
- [web/background.html](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/web/background.html)，blob `d6f9a5b91ff60f6c30f914c2dc39af98539110ae`。
- [HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibUiSnapshot.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibUiSnapshot.h)，blob `27292c59b21c7d79b5615020e5d579f60a6590d9`。
- [HT9011UC_Cpp_V3.33.906.0/TesterComm/UiChannel.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/TesterComm/UiChannel.h)，blob `b8a4582b37d94c2b726c9781dfdaabc8719007e5`。
- [HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)，blob `37a7c8b31f36fea2a1435ea1c38505a59e6925b1`。
- [HT9011UC_Cpp_V3.33.906.0/csystem.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/530357c6d489ee2a9cafda718cf294f07c6af7d0/HT9011UC_Cpp_V3.33.906.0/csystem.cpp)，blob `8f446ebc2d91d076131af0dc4c7103fc85ee8ffe`。

回 [driver入口](../../index.md)。原文註解中的日期、裁決、行號與歷史量測完整保存，但本段沒有啟動程式、測試或機台。
