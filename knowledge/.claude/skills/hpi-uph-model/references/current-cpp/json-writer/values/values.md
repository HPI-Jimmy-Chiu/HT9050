# Value薄入口：BeforeValue之後仍append

[上層](index.md)；[BeforeValue既有正文](../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/auth-callees/login-json/raw-value.md)；[Quote](quote.md)；
[Ok／Str／Clear](../containers/result.md)。

| 方法 | BeforeValue之後的正文 | 原文 |
|---|---|---|
| Null() | buf_.append("null") | [Null](raw/source-02.md) |
| Bool(bool v) | 依v附加"true"或"false" | [Bool](raw/source-03.md) |
| Number(wb_int64 v) | buf_.append(JsonNumber(v)) | [整數overload](raw/source-04.md) |
| Number(double v) | buf_.append(JsonNumber(v)) | [浮點overload](raw/source-05.md) |
| String(const std::string& v) | buf_.append(JsonQuote(v)) | [String](raw/source-06.md) |

五方法都先呼叫BeforeValue，再append並回*this；沒有用ok_條件早退，也沒有本地catch。
因此不能把BeforeValue設ok_=false推定為本方法自動阻止buffer寫入。
BeforeValue可能先改逗號／pending狀態；value正文只有後段append，兩段來源分開讀。
回傳JsonWriter&是鏈式介面，不等同成功回傳值；是否檢查Ok由caller另核。

Null與Bool的literal在此可見；兩Number的數字格式與非有限值處理委派JsonNumber。
不能由Number(double)存在推定小數點locale、精度、NaN／Infinity或JSON格式已驗證。
String委派JsonQuote；它與既有RawValue直接附加raw文字的用途不同。
此五正文沒有CUSTOMER_CODE、MachineType或iO15_SaveFilePeriod分支；
實際客戶欄位、publisher與部署機型條件不因此相同。

本輪只保存選定薄入口與Quote；未執行錯誤序列、parser、數值格式／browser或機台。
完整JsonNumber／Sanitize、caller成功檢查、buffer生命週期與thread同步仍待續。
