# 時間字串與成員格式入口

[上層](index.md)；定位 `vclcompat/TDateTime.cpp::TimeToStr` 與 `TDateTime::FormatString`，既有formatter見 [完整格式路徑](../../../../../../ini-conversion/format.md)。

| 新保存定義 | 行為 |
| --- | --- |
| TimeToStr(const TDateTime& dt) | FormatDateTime(AnsiString("hh:nn:ss"),dt)。固定格式沒有日期、毫秒與AM/PM token；沒有呼叫Now或刷新Handler全域／物件時間。 |
| TDateTime::FormatString(const AnsiString& fmt) const | 直接return FormatDateTime(fmt,*this)。使用傳入fmt與既有物件，不另取時；沒有在此入口讀locale設定或修改物件。 |

FormatString的20260923 T6-CCSIR歷史註記指向golden的CheckContinusStartIsReady使用Now().FormatString，原文保留在完整header及CPP archive。本輪只完成薄入口，沒有完成該golden caller、最新913或BCB6 ABI驗證。

FormatDateTime、DateTimeToStr與StrToDateTime的完整正文已先完成並於本輪重核；token解析、month／minute、sscanf寬容與返回零值仍以原單元為準。薄入口本身不改善錯誤處理、日期合法性、encoding或formatter支援範圍。

若caller先Now再FormatString，只格式化當時物件；若caller持有舊TDateTime，這兩個入口不自行刷新。不同caller的取樣／生命週期與最終log bytes仍另查，這裡沒有執行格式化或寫檔。
