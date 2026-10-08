# V906 字串清單共用 callee

同一hpi-uph-model內分層續查TStrings／TStringList，依function／變數定位，保持各單元固定source pin。

- [核心、Count、Objects及Strings／Text代理](core/index.md)：完整選定定義、CRLF分行、Assign與cache；共用V906實作與機台／版本界線。
- CommaText／DelimitedText parser與proxy、檔案I/O、深層AnsiString／locale／ABI及完整caller尚待後續單元。

回 [caller索引](../index.md) 與 [INI轉換](../ini-conversion/index.md)。本樹沒有新canonical Skill，也沒有runtime驗證。

## 分隔文字與 I/O 續查

[CommaText／DelimitedText 與檔案 I/O](delimited-io/index.md)：parser、writer、proxy、byte cells 與 Load／Save 的局部單元已保存。上方待查清單是核心單元當時的狀態；這些函式的靜態查證現由本節接續，深層 AnsiString／完整 caller／runtime 仍未完成。
