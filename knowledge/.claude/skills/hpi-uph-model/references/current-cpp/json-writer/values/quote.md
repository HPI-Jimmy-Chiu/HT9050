# JsonQuote：Sanitize呼叫與逐byte escape

[上層](index.md)；[完整原文](raw/source-01.md)；[Key](../containers/key.md)。

`JsonQuote(const std::string& raw)` 先把 `SanitizeToUtf8(raw)` 的回傳值存到s，
再建立out、reserve s.size()+2、加起始雙引號，逐byte讀s並以unsigned char分支。
結束時加尾雙引號並回傳out；沒有由這段正文直接驗證SanitizeToUtf8的內部契約。

| byte | 正文附加的內容 |
|---|---|
| 雙引號、反斜線 | 對應的JSON escape |
| backspace／form feed／newline／carriage return／tab | \b／\f／\n／\r／\t |
| 其他小於0x20的C0 byte | \u00加兩個小寫hex字元 |
| 其餘byte | 轉回char附加原byte |

這是Sanitize回傳值上的byte分支；不能稱為本函式逐Unicode code point處理。
原註解稱高byte在此已是valid UTF-8，並說RFC 8259禁止raw C0；註解完整保留，
本輪沒有另查標準全文或跑編碼測試，不能把註解宣告當作新驗證結果。
空字串會走到兩個quote的附加路徑，沒有特設空字串早退。

既有Key用JsonQuote處理name，本單元String用它處理value；兩處參數用途分開。
此函式不收JsonWriter物件，正文沒有讀ok_、stack_、keyPending_或needComma_。
SanitizeToUtf8如何處理不合法UTF-8、ACP或替代字元，完整caller的輸入來源與長度，
仍須保存對應callee／caller後判斷；本輪未執行writer／parser／browser或runtime。
