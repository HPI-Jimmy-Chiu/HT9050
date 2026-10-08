# printf模板、暫存與backend

[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)；[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)。完整選定函式、模板與歷史裁決正文保存在 [manifest](source-manifest.json)。

| 入口 | data_操作 |
| --- | --- |
| sprintf／printf variadic template | 用formatString(fmt,conv(args)...)覆寫data_，回*this |
| cat_printf variadic template | 同樣format後append，回*this |
| 三個zero-arg多載 | 無variadic參數，分別覆寫／覆寫／append |
| conv(const AnsiString&) | 回s.c_str() |
| conv(T v) | 其他型別pass-through，不作format specifier型別檢查 |

模板以值傳Args...，AnsiString實參copy在本次call內經conv轉char*；所選路徑是同步formatString。這是靜態生命期推論，不能延伸成跨call pointer、其他自訂型別varargs或所有compiler ABI都已驗證。

`formatString`：null fmt回空std::string；va_start後va_copy到ap2，用vsnprintf(0,0,fmt,ap)取needed並va_end(ap)。needed<0時va_end(ap2)後回空；否則配置needed+1 bytes vector，用ap2再vsnprintf，va_end(ap2)，最後由buf.data()與明示needed長度建std::string。

正文第二次vsnprintf回傳值沒有再檢查；配置／格式錯誤與兩次結果一致性、平台vsnprintf支援仍待查。它按第一次needed分配一次，不是迴圈重試長度。所有格式specifier仍須與pass-through varargs的實際型別相合；conv只處理AnsiString，不能把它當通用型別安全formatter。

null fmt或needed<0回空時，覆寫型sprintf／printf會把data_變空，append型cat_printf則append空序列；沒有另發本shim自訂錯誤。明示needed建字串可保存格式產生的embedded NUL與其後bytes；其後c_str／%s等NUL-terminated入口仍可能只見前段，見 [byte界線](../bytes/storage.md)。這些是所選body的推論，本輪未執行格式例子、讀機台或測locale／ABI。

回 [入口](index.md)、[數值格式](formatting.md)、[界線](limits.md)。
