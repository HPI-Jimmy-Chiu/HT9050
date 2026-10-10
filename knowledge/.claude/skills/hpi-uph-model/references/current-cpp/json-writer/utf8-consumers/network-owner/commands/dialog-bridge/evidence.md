# Dialog bridge來源與保存驗證

[上層](index.md)／[manifest](source-manifest.json)／[symbol census](symbol-census.json)。
固定pin `f527888567ba11e5b6a2324323d750bf78de58e9`；七完整JS175行，原檔835行、五頁／一context credit0。
原文頁的anchors是function maps入口，body可以跨頁，使用manifest offsets與SHA逐段核對；不是只保存宣告或call行。

| 原文分頁 | payload SHA256 |
|---|---|
| [source-01-part-01.md](raw/source-01-part-01.md) | `44a64e001697071fd513049c301417a6486971bd3fb08fbefd95cdf53f27121c` |
| [source-01-part-02.md](raw/source-01-part-02.md) | `c18f0d1b32ded87e1556fc1c372254524dc72a48bec3c88297557a969eb4c469` |
| [source-01-part-03.md](raw/source-01-part-03.md) | `87ffe9c1ebe57be91bd2f5722f66dc51dd4d29924ef89ff51db2843e60c8b1fc` |
| [source-01-part-04.md](raw/source-01-part-04.md) | `25cc55e83a8eeffec684c7c8fc970fb630a3705501de4323f48730bdb458c6ed` |
| [source-01-part-05.md](raw/source-01-part-05.md) | `83ff0582151a9477e352dbe73087d0e37b96cc1a0fb3646b2db45bd9273ab8ec` |

| 完整function | 原文行 | SHA256 |
|---|---:|---|
| `sendRequest` | 24 | `0144390d515c5df7b5058d34965f03a0a0c24be8cdeb6d034000399394f49d74` |
| `onPageMessage` | 34 | `dd1838d0b417a77c8a6c11e43d6dd58c1fedee3df8b25dc865dc3f22328d78ad` |
| `submit` | 29 | `8f2a84e73ebb542e88e316422631869042b81d1d8a136ccb4e13914028652a9c` |
| `submitRest` | 30 | `403c6b36fc5bdd1eec41cf5e7fb13b0e0d25f836eef0d37aa2ce21bb73e13935` |
| `submitCloseResponse` | 17 | `f8f15fdb80a2c13f6782aeed2ee9cd9ec743699a4334772fce10dbdfb6282cc8` |
| `complete` | 35 | `48676b44cfc685ae8e3f65c21b9c93898aa746a75afaa6ed6382bf2f4e3a8b6a` |
| `rejectClose` | 6 | `ea93a60d8b7b3d9c586f770ea52273c5e1eaacaf630210d4dd9ba1297e7b6c04` |

全文逐頁SHA重組，重新定位七body與signature／offset／原文SHA核對；70既有manifest與Recipe舊21文件保留，commands/index只追加。
所有原comment、metadata及未選函式／callback保存；context不增加完成credit，詞法census不稱完整call graph。
與最新main原始碼bytes比較及兩層Skill驗證結果另寫本單元RD5軟體日報；未實機測試。
