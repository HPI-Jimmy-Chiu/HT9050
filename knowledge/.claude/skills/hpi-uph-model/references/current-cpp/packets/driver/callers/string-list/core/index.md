# V906 TStrings／TStringList 核心與 Text

固定來源 `d2e65d41b59d134d171a5e15204d76965bdc3cad`。2個UTF-8來源、24個完整cpp定義、17個完整header inline定義、6個完整class、2個完整struct、1份完整header與1個歷史註解區段，共51原文及byte／text／body hash見 [manifest](source-manifest.json)。重疊excerpt各自保存；不是51個獨立函式。

| 問題 | 路由 |
| --- | --- |
| TStrings介面、Count、元素／Objects、Assign、Sort／Find | [核心儲存](storage.md) |
| Strings／Objects／Text代理及c_str快取 | [代理](proxies.md) |
| CRLF、分行、GetTextStr快取與靜態例子 | [Text](text.md) |
| HT9050／其他Handler、版本／客戶／runtime與剩餘 | [界線](limits.md) |

## 固定來源

- [HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/d2e65d41b59d134d171a5e15204d76965bdc3cad/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.cpp)；blob `09dea84a6695298165cf66141c809c84abf40f7d`。
- [HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/d2e65d41b59d134d171a5e15204d76965bdc3cad/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.h)；blob `196f50a3be10776876b41acc51dc25eee609ac23`。

本節接續 [INI轉換](../../ini-conversion/index.md)、[GPIB快照](../../../updates/20261008-ui-page/snapshot.md) 所需的共用容器。保留前段pin與正文；完整caller閉合另續。
回 [string-list路由](../index.md)；本輪只有來源與文件保存查證。
