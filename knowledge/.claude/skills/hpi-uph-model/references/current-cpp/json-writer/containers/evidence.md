# Writer容器局部證據與共同層

[上層](index.md)；[manifest](source-manifest.json)；[詞法普查](symbol-census.json)。
來源 `94ff7c1981d1ac98f01376a574098c39e1c25275`；7完整CPP／1inline，58行原文／8reader頁／2來源。
核對51份舊UPH manifest，八正文沒有已完整保存的同正文；舊保存檔blob維持。
RawValue／BeforeValue僅作前段契約連接，不重算新完成單元。

| 適用軸 | 共同項與差異 |
|---|---|
| HT9050／其他Handler | 使用此V906 JsonWriter class者按同一來源契約讀；八正文無機型條件，不等於所有機台publisher與部署相同。 |
| 客戶 | writer正文無CUSTOMER_CODE分支；登入來源、payload欄位與權限仍依各caller條件，不能由此套用單一客戶結果。 |
| 版本 | 來源為V906 C++17／UTF-8 WebBridge；V912 BCB、其他golden與部署版本另核，未宣稱跨版本等價。 |
| runtime | 只核來源與保存；未執行writer、JSON parser、HTTP／WS、build、瀏覽器或機台，未讀實際帳密／環境／機台設定。 |

同題路由接既有[HMI入口](../../../../../hpi-web-hmi/SKILL.md)、
[JSON契約](../../../../../hpi-web-hmi/references/json/index.md)、
[原日期main查證](../../../../../hpi-web-hmi/references/runtime/index.md)與
[機台分流](../../../../../hpi-web-hmi/references/machines/index.md)；各頁原pin／裁決日期保持。
本輪只核這四入口的blob／hash與引用存在，不重新宣稱其歷史runtime查證已重驗。

活查證以class／function／變數定位，offset／行號只是來源pin metadata。
普查遮罩comments與一般string／char，排除.svn，包含同名Clear／Key／Str等其他class；
class尚未逐命中解析，不能把詞法行數當JsonWriter caller總量或完整call graph。

仍待JsonQuote／其餘value、caller錯誤檢查與buffer生命週期、browser consumer、
AskArm／parser／binary override，以及其他wait／timer／DB／thread和UPH S8實機。
原文保存與靜態推論不等於已重現JSON故障或完成實機驗證。
