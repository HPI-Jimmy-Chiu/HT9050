# JsonWriter容器與結果狀態

[登入／raw writer上層](../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/auth-callees/login-json/index.md)；[先前RawValue／BeforeValue](../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/auth-callees/login-json/raw-value.md)。
來源 `94ff7c1981d1ac98f01376a574098c39e1c25275`；7完整CPP與1完整inline，8正文／2來源，局部靜態查證。

| 問題 | 入口 |
|---|---|
| Key guard、逗號與keyPending_ | [Key](key.md) |
| Object／Array入棧、退棧與錯誤早退 | [容器](containers.md) |
| Ok、Str借用reference與Clear重置 | [結果與重置](result.md) |
| 機型／客戶／版本／runtime與未查caller | [證據界線](evidence.md) |
| 完整正文、來源pin／blob／hash | [保存manifest](source-manifest.json) |
| 同名符號詞法命中，class尚未逐一解析 | [詞法普查](symbol-census.json) |

本頁接續已保存的RawValue與BeforeValue，兩者不重計為新完成正文。
JsonQuote、其他value方法、實際publisher／browser consumer仍待核對。

## 接續局部：quote與value

來源 `97361abfd5d1a08b51caf707f928b56825f3f056`；[六函式quote／value子樹](../values/index.md)補JsonQuote、Null／Bool、兩Number與String。
原容器／結果正文、raw與metadata保持；Sanitize／JsonNumber與完整caller另續。
