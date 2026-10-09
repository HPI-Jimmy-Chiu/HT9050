# Reset、Fail與Feed的狀態責任

[上層](index.md)；[class](../raw/source-14.md)；[constructor](../raw/source-15.md)。

| 完整正文 | 狀態與返回 |
|---|---|
| [Reset](raw/source-01.md) | 清buf_、frag_、fragOpcode_、failed_、sawClose_、closeReason_；closeCode_回normal。 |
| [Fail](raw/source-02.md) | 首次failed_才保存code／reason；NULL reason變空字串；一直回false。 |
| [Feed string](raw/source-03.md) | chunk.data()／chunk.size()轉交byte overload，包含內部NUL。 |
| [Feed bytes](raw/source-04.md) | failed_先回false；非NULL且len非0先append整個chunk，再循環TryOneFrame。 |

Reset保留requireMask_、validateUtf8_、maxMessage_；不重新套constructor預設。
constructor預設驗UTF8，header default max為1 MiB，server角色requireMaskedInput預設true；
caller可指定角色／上限，SetValidateUtf8可改開關；不能將預設當成部署現值。

Feed的out為NULL時改用本地sink，仍消費資料，但完成訊息不交還caller。
data為NULL或len為0時不append，仍試著解析既有buf_；沒有對NULL＋非0長度另設錯誤。
TryOneFrame返回1代表有進展，不代表一定產生out訊息；0代表front frame未齊，Feed回true；
-1代表失敗，Feed回false。單chunk可解多frame，也可留下未完整frame；
若較後frame失敗，先前已append到*out的完成訊息不回滾，既有*out內容保留。

maxMessage_檢查位於TryOneFrame，在Feed已append整個chunk之後。
原註解「BEFORE buffering」指解析器在等待該payload／重組前檢查宣告長度；
不能從這段推出Feed輸入chunk或整個buf_有絕對記憶體上限，也未提供配置失敗處理。
PendingBytes只回buf_.size()；FragmentBytes另回frag_.size()，不能只讀前者就稱兩者總和。

Fail只設錯誤旗標、code與reason，沒有發送close或關socket。
class原契約要求caller用EncodeClose並關閉socket；本輪尚未查實際network owner履行方式。
SawClose只是一個接收close的旗標，不會使Feed停止drain；後續frame仍可能在本body被解析。
這些是固定來源正文的靜態順序，未跑分chunk、heap、socket或實機測試。
