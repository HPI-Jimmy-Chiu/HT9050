# Strings／Objects／Text 代理與快取

定位`StringsAccessor`／`ObjectsAccessor`、`StringsProxy`／`ObjectsProxy`／`TextProxy`、`owner_`／`idx_`／`c_str_cache_`。完整class／struct與所選inline／cpp定義見 [manifest](source-manifest.json)。

## 讀寫分流

| 代理 | 讀取／寫入 |
| --- | --- |
| Strings[i] | accessor產生持有owner／index的代理；轉AnsiString呼叫GetString，AnsiString／char*指派呼叫SetString |
| Objects[i] | accessor產生持有owner／index的代理；轉TObject*呼叫GetObject，TObject*指派呼叫SetObject |
| Text | 保存owner；轉AnsiString呼叫GetText，AnsiString／char*指派呼叫SetText |

StringsProxy及TextProxy明確定義同型代理copy assignment：先`AnsiString(v)`讀來源，再轉交文字指派。它們複製的是文字語意，不是把目標owner換成來源owner。
ObjectsProxy只明確宣告TObject*指派，沒有同型代理文字式overload。本段沒有compiler／完整caller比對；不得直接把Objects代理的同型copy當作對容器的SetObject，使用時要另核實overload與實際表達式。
Text代理傳文字給SetText會重建清單與null Objects；需要保存Objects指標的路徑應查 [Assign](storage.md#assign-與-objects-所有權) 的實際caller。

## StringsProxy::c_str

header inline先把`AnsiString(*this)`存入**代理自己的mutable c_str_cache_**，再回其c_str指標。accessor每次產生代理；`list->Strings[i].c_str()`中的暫存代理存活到caller完整運算式結束，不能保留其指標跨過暫存代理銷毀。
若caller另存代理物件，也仍依賴owner存活／index有效及下一次cache重寫。本段只查保存位置與原始碼，不測指標生命期、ABI、並行讀寫或全部caller。
header內20260819／20260826的修復、call-site數與BCB6比較註解完整留存；不把歷史數字寫成本輪盤點結果。

TextProxy沒有自己的c_str成員；TStringList的GetTextStr另用**list本身的textCache_**，見 [Text快取](text.md#gettextstr-快取)。兩者不可混用生命期結論。
CommaText／DelimitedText的class原文在完整header snapshot留存，詳細parser／同型proxy與檔案I/O分層另續。

回 [入口](index.md)。
