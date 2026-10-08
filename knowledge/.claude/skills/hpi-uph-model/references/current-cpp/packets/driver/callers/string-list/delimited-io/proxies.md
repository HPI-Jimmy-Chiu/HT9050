# CommaText／DelimitedText 代理

來源：[TStringList.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.cpp)；[TStringList.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.h)。定位兩個完整 class、owner_、各 ctor、`operator AnsiString()`、兩種 value assign 及 same-proxy assign；[manifest](source-manifest.json) 保存完整函式與 class 內歷史註解。

| 操作 | CommaTextProxy | DelimitedTextProxy |
| --- | --- | --- |
| 讀取／轉 AnsiString | owner_->GetCommaText() | owner_->GetDelimitedText() |
| AnsiString／char* 指派 | owner_->SetCommaText(v) | owner_->SetDelimitedText(v) |
| 同型 proxy 指派 | 先 AnsiString(v)，再目前目的 proxy 的 setter | 同左，使用目的端 DelimitedText setter |

same-proxy assign 表達序列化來源清單、再解析進目的清單；沒有把來源 owner_ 指標改綁到目的 proxy，也不是 Objects 指標複製。原 header 20260819「24 call sites／9 TUs」是歷史量測，本單元沒有新增全樹 census。

## 目的端參數決定再解析

CommaText 來源 getter 與目的 setter 均固定逗號／雙引號，目的端 setter 會永久改寫目的 Delimiter／QuoteChar。DelimitedText 則來源先用來源欄位輸出，目的用自己目前欄位解析；**same-proxy assign 沒有複製來源欄位**。

靜態例子：來源 Delimiter=分號，項 `["a","b"]`，輸出 `a;b`；目的 Delimiter=逗號時，目的解析成一項 `a;b`。它不等於 `TStringList::Assign` 同時複製欄位與 Objects 指標的行為，見 [核心 storage](../core/storage.md)。

自指派也經過序列化／再解析，可能正規化內容、重建基底 Objects，CommaText 還會改目的欄位；不能套用 `Assign(src==this)` 的 early return。完整生命期／null owner、char* 到 AnsiString 的深層語意與跨執行緒使用尚待後續查證。

回 [入口](index.md)、[parser](parser.md)、[核心 proxy](../core/proxies.md)、[界線](limits.md)。
