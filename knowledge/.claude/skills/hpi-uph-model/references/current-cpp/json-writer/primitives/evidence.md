# JSON primitive保存與適用界線

[上層](index.md)；[manifest](source-manifest.json)；[詞法census](symbol-census.json)。
來源pin `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；8完整CPP／213原文行／3來源檔。
13 raw讀頁包含8函式與5context；原header policy、U+FFFD、Cp950 Win32 wrapper與
[TestJsonUtf8Policy](raw/source-12.md)／[TestJsonNumbers](raw/source-13.md)均完整保存，context計數0。
原註解、warning、日期、作者、工具鏈測量表及測試assert保存，不換成新驗證結果。

| 適用軸 | 本輪證據 |
|---|---|
| HT9050／其他Handler | 共同層是本pin V906 WebBridge實作；8正文無MachineType／CUSTOMER_CODE分流。實際publisher及consumer另核。 |
| Windows／其他平台 | Cp950ToUtf8定義及呼叫有_WIN32 gate；非Windows有效UTF-8後直接repair fallback。 |
| 客戶／版本 | CP950的V899／machine檔文字是原註解；未核所有客戶字串來源，不等同V912 BCB／V899／golden913版本實作。 |
| runtime／驗證 | 只有Git來源、摘錄、分頁重組、metadata及引用保存；未執行cpp、build、測試、API、browser或機台。 |

53 prior manifest去重包含已交付且已合main的!401；去重pin `82aad3ad2ccf00ccbb182d5376427da867ee2298`，
本單元開工main已包含!401；去重仍保留交付pin，不重計舊函式或Ready完成數。
詞法普查沿用原intake pin `7af285f31b3c7b826e057a910ef5e17e9456b8c4`；遮罩comments／string的命中不是完整語意call graph，
不是本輪重新普查全main，也不把其他class同名命中全算成JsonWriter caller。
活定位用function／變數；offset／行號只供固定pin保存查證。

跨題接[HMI入口](../../../../../hpi-web-hmi/SKILL.md)，保留JSON／runtime／機台分流的原日期界線。
後續仍含完整caller／publisher／browser、locale部署與CRT、thread／重入、版本客戶對照，以及
UPH容量／校正實機與S8／846候選；本局部單元完成不等於整個JSON或Skill backlog完成。
W-195仍歸St02-E；!401 main已核且ST02-M回報22:08通知已寄；三批各按Batch ID不重寄。
