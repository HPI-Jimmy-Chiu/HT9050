# RawValue不會修正StateJson的正文

[上層](index.md)；[已保存StateJson](../state.md)；[RawValue原文](raw/source-03.md)；
[BeforeValue原文](raw/source-04.md)。

`JsonWriter::RawValue` 先呼叫BeforeValue，再buf_.append(jsonText)，最後回*this。
這個正文不加引號、不escape、不解析JSON，也沒有按ok_決定是否略過append。
先前WebLogin_StateJson動態文字直接放進手工引號；若該正文含不合法字元，
從這兩個callee可推論RawValue不會替它修正。這是源碼條件推論，未重現runtime錯誤。

## Key／comma檢查與內容驗證不同

BeforeValue在stack非空、頂層kCtxObject且keyPending_為false時寫ok_=false；
它沒有早退，仍會依needComma_附加逗號，接著清keyPending_並設needComma_=true。
因此「回傳writer reference」不等於寫入成功；錯誤狀態也不等於buf_沒有改變。
本函式只讀容器頂層與兩旗標，沒有查看jsonText正文，也不把ok_恢復true。

Object的Key、Array／Object入棧退棧、Str／Ok與consumer如何檢查錯誤仍待讀；
單靠這兩個方法不能證明完整writer的合法JSON契約或reader容錯行為。
輸入編碼、quote／control-character實際資料與瀏覽器效果沒有實機或執行驗證。
