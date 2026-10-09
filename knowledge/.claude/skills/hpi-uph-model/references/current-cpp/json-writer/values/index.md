# JsonWriter的quote與value入口

[容器／結果上層](../containers/index.md)；[既有RawValue／BeforeValue](../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/auth-callees/login-json/raw-value.md)。
來源 `97361abfd5d1a08b51caf707f928b56825f3f056`；6完整CPP／63原文行／1來源，局部靜態查證。

| 問題 | 入口 |
|---|---|
| JsonQuote先處理編碼再逐byte escape | [Quote](quote.md) |
| Null／Bool／兩Number overload／String的狀態連接 | [Value](values.md) |
| 版本、客戶、HT9050／其他Handler與未查callee | [證據界線](evidence.md) |
| 完整正文、pin／blob／hash與reader分頁 | [保存manifest](source-manifest.json) |
| 同名詞法命中，尚未解析class／caller語義 | [詞法普查](symbol-census.json) |

既有Key、RawValue、BeforeValue與容器方法只連接上下文，不重計完成。
SanitizeToUtf8、JsonNumber完整實作及publisher／browser consumer仍待續。
