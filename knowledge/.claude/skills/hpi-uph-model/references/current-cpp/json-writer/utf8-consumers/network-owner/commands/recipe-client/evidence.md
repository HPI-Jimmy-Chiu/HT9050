# 保存與完整function maps

[上層](index.md)／[manifest](source-manifest.json)／[有限census](symbol-census.json)。
固定pin `2f2872da7fce4b7ae6eb6685885de626885ccd92`；69既有manifest與!430前批83文件保存，commands/index只追加本單元入口。
12完整JS／196函式原文行；兩整檔1597行、10原文頁含全部metadata／原註解、object APIs、IIFE及callbacks。
offset只是固定pin的重組證據；活文件以版本／source path＋function／變數定位。

| 來源 | 完整原文分頁 |
|---|---|
| `web/page/ht9045_recipe_client.js` | [source-01-part-01](raw/source-01-part-01.md)、[source-01-part-02](raw/source-01-part-02.md)、[source-01-part-03](raw/source-01-part-03.md)、[source-01-part-04](raw/source-01-part-04.md)、[source-01-part-05](raw/source-01-part-05.md)、[source-01-part-06](raw/source-01-part-06.md) |
| `HT9011UC_Cpp_V3.33.906.0/tools/websync/ht9045_recipe_client.js` | [source-02-part-01](raw/source-02-part-01.md)、[source-02-part-02](raw/source-02-part-02.md)、[source-02-part-03](raw/source-02-part-03.md)、[source-02-part-04](raw/source-02-part-04.md) |

| 來源／function | 完整body SHA256 | 函式行 |
|---|---|---|
| `web/page/ht9045_recipe_client.js`／`connect` | `4163e2bc3c904c005ac3fbcc89e03ba82df5508159e8926e601184397a7bd834` | 54 |
| `web/page/ht9045_recipe_client.js`／`cmd` | `3894986244e5c72192d4007ce38586e04f5298a720c425baef2236262eae2d67` | 1 |
| `web/page/ht9045_recipe_client.js`／`cmd0` | `7c807bbb0473872c0bbcba2adac5f2974cb613921a4f183eee95f93277fc384e` | 24 |
| `web/page/ht9045_recipe_client.js`／`unwrapAck` | `b4ba42938cc1f88b6175e7a2a13a1914eb561dd386353ce70090a81cdb0911ac` | 5 |
| `web/page/ht9045_recipe_client.js`／`acquire` | `dccb73fc248e42c49f38c013340297a361bda831c3baf3c252801172c9a0cf31` | 4 |
| `web/page/ht9045_recipe_client.js`／`takeover` | `84d132d6bd2b5e3c91cfacbb821812239d3e78a762706f10f0bf11b3127676d4` | 4 |
| `web/page/ht9045_recipe_client.js`／`tokenHeld` | `996e8719ae856e9c6f5138e1a8ffceb0047e5b47b81c3fa1152a4e86a94bbc8f` | 6 |
| `web/page/ht9045_recipe_client.js`／`armTokenIdle` | `b53889d749865086ae736c9f2eadf2520f2f68956ab67226a61725cf9395d86a` | 9 |
| `web/page/ht9045_recipe_client.js`／`tokenTrack` | `1a92e0312d4356b4f11dc8f710cb4c816d1181d0ca4f0a81cd945c186864b778` | 16 |
| `HT9011UC_Cpp_V3.33.906.0/tools/websync/ht9045_recipe_client.js`／`connect` | `2d1875af85ad1c13ab7345e3448154c3579a724fc910fcb0b75dce3b3f6210d3` | 44 |
| `HT9011UC_Cpp_V3.33.906.0/tools/websync/ht9045_recipe_client.js`／`cmd` | `b3bb5e1719e32a7e5235c8023f8574cec0d13a1692d8b1f790144b78a68e9ab3` | 24 |
| `HT9011UC_Cpp_V3.33.906.0/tools/websync/ht9045_recipe_client.js`／`acquire` | `29ae93259cbbd5cb7dbf27fd8f8beb9012317bedaba7dc44ff3c91e2671a69cf` | 5 |

body maps以source offsets／signature與全文比對；raw payload逐頁SHA256後重組，重新定位named functions確認完整邊界。
匿名callback原文有保存，但沒有額外完成credit或端到端語意驗證；函式清冊不是全caller audit。
歷史實測、舊deploy／runtime／版本聲明與metadata完整保留，不改稱本輪測試。
