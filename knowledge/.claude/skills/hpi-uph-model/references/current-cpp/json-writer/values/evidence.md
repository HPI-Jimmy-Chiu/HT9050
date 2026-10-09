# Quote與Value局部證據及共同層

[上層](index.md)；[manifest](source-manifest.json)；[詞法普查](symbol-census.json)。
來源 `97361abfd5d1a08b51caf707f928b56825f3f056`；6完整CPP／63行（含原空行）／6 reader頁／1來源。
52份舊manifest未包含這六段完整正文，保存檔blob保持。
既有Key／RawValue／BeforeValue與容器正文只核上下文，不新增完成信用。

| 適用軸 | 共同項與差異 |
|---|---|
| HT9050與其他Handler | 使用此V906 JsonWriter class者按此來源契約讀；六正文無機型條件，部署來源及publisher另核。 |
| 客戶 | Quote／value正文沒有CUSTOMER_CODE分流；輸入來源、payload欄位及權限由caller決定。 |
| 版本 | 保存V906 WebBridge C++17／UTF-8樹；V912 BCB、其他golden或客戶實跑版本未比較，未宣稱等價。 |
| runtime | 只做Git來源、原文保存與文件引用驗證；未執行writer、build、parser、HTTP／WS、browser或機台。 |

同題接[HMI主入口](../../../../../hpi-web-hmi/SKILL.md)、[JSON契約](../../../../../hpi-web-hmi/references/json/index.md)、
[原日期main查證](../../../../../hpi-web-hmi/references/runtime/index.md)及[機台分流](../../../../../hpi-web-hmi/references/machines/index.md)。
本輪核其入口blob／hash及引用，保留原裁決／來源pin，不把歷史runtime記錄改稱本輪實機測試。

活定位用函式signature／變數；offset／行號只屬固定pin metadata。
詞法普查遮罩comments與一般string／char，排除.svn；Number／String等同名包含其他class，
未逐命中解析callee或caller，不能把詞法命中行數當完整JsonWriter call graph。

SanitizeToUtf8與JsonNumber完整正文、publisher錯誤處理、buffer壽命、consumer／browser、
AskArm／parser／ACP，以及其他wait／timer／DB／thread、客戶部署與UPH S8實機仍待續。
W-195已由Steven收回，這個文件單元沒有改TESNA、程式或執行期設定。
