# browser原文與function map證據

[上層](index.md)／[manifest](source-manifest.json)／[有限census](symbol-census.json)。固定main `06c430c157a3e39c14d33a5b29219925b2879854`。
兩整檔507行：web/page 380行三頁、V906 overlay127行一頁；4原文頁／2context credit0，所有註解、metadata、原裁決／ChangeLog定位及匿名callback完整保存。
13個完整選定JavaScript函式302原文行，0新增CPP／0新增canonical主題；兩個debug inline／IIFE／Promise／event callbacks不重計。

| function／object method定位 | 函式行 | 完整檔案function anchor |
|---|---|---|
| `web/page/ht9045_dialog_host.js`／`function log(kind, text)` | 4 | [定位](raw/source-01-part-01.md#log) |
| `web/page/ht9045_dialog_host.js`／`function isAcknowledge(response)` | 6 | [定位](raw/source-01-part-01.md#isacknowledge) |
| `web/page/ht9045_dialog_host.js`／`function optionOf(response)` | 13 | [定位](raw/source-01-part-01.md#optionof) |
| `web/page/ht9045_dialog_host.js`／`function pickOption(want, offered)` | 8 | [定位](raw/source-01-part-01.md#pickoption) |
| `web/page/ht9045_dialog_host.js`／`function submitResponse(file, response)` | 147 | [定位](raw/source-01-part-01.md#submitresponse) |
| `web/page/ht9045_dialog_host.js`／`function watch()` | 18 | [定位](raw/source-01-part-01.md#watch) |
| `web/page/ht9045_dialog_host.js`／`function verifyAuth(verify)` | 28 | [定位](raw/source-01-part-01.md#verifyauth) |
| `web/page/ht9045_dialog_host.js`／`function sendAuthCancel()` | 14 | [定位](raw/source-01-part-01.md#sendauthcancel) |
| `HT9011UC_Cpp_V3.33.906.0/web-overlay/page/ht9045_dialog_host.js`／`function api()` | 4 | [定位](raw/source-02-part-01.md#api) |
| `HT9011UC_Cpp_V3.33.906.0/web-overlay/page/ht9045_dialog_host.js`／`function channelOf(fileName)` | 8 | [定位](raw/source-02-part-01.md#channelof) |
| `HT9011UC_Cpp_V3.33.906.0/web-overlay/page/ht9045_dialog_host.js`／`function encode(resp)` | 13 | [定位](raw/source-02-part-01.md#encode) |
| `HT9011UC_Cpp_V3.33.906.0/web-overlay/page/ht9045_dialog_host.js`／`submitResponse: function (fileName, response)` | 25 | [定位](raw/source-02-part-01.md#submitresponse) |
| `HT9011UC_Cpp_V3.33.906.0/web-overlay/page/ht9045_dialog_host.js`／`verifyAuth: function (payload)` | 14 | [定位](raw/source-02-part-01.md#verifyauth) |

web/page全文依序[第一頁](raw/source-01-part-01.md)、[第二頁](raw/source-01-part-02.md)、[第三頁](raw/source-01-part-03.md)；[overlay全文](raw/source-02-part-01.md)。
function anchor是完整檔案的map入口，body可能跨頁／位於後頁；offset／SHA核全文中每個完整body，活定位仍用path＋function／變數。
68舊manifest（含header-refresh-manifest不同檔名）與前批67檔保留，commands/index純追加驗原prefix；歷史原文、metadata／相容入口不改。
四來源bytes核目前main／HEAD一致，兩client只有限詞法symbols／export定位，未冒稱兩client全文caller graph／protocol或browser實測。
普通自己分支checkpoint，約17:00同批一Ready交Jimmy，不auto／self／main；!422／!412／!406已結不重交重寄，未寄信或改其他session／共用checkout。
