# nowSerialWithMs與獨立取時

[上層](index.md)；定位 `vclcompat/TDateTime.cpp` 的 `nowSerialWithMs()`、`Now()`、`Date()`、`Time()`。

| 條件／Function | 現行來源路徑 |
| --- | --- |
| _WIN32 | 區域SYSTEMTIME st，呼叫::GetLocalTime(&st)，以st年月日、時分秒與wMilliseconds交serialFromTm。 |
| 非_WIN32 | std::time(0)，區域tm lt，localtime_r(&t,&lt)，以lt年月日／時分秒、固定msec=0交serialFromTm。 |
| Now | 一次nowSerialWithMs，封裝成TDateTime，保留日期與時間小數。 |
| Date | 一次nowSerialWithMs，std::floor後封裝；時間小數移除。 |
| Time | 一次取s，回傳s-floor(s)；只保留同次取樣的日內小數。 |

nowSerialWithMs的static是函式的連結範圍，沒有static取樣快取；每次進入都重新取時。三個factory也沒有共用一筆snapshot。若caller分別取Date與Time或多次Now，各值可能跨日或跨時鐘調整；Time單次s的日期／小數則來自同筆值。

## 可確認與尚未確認

來源使用平台local-time入口、以年月日重組double序列；沒有使用steady_clock、tick-counter或UTC轉換／時區offset欄位。因此由這個來源不能建立單調耗時保證，不能把兩筆Now相減當成已驗證的硬體／UPH耗時。

Windows banner的millisecond precision是對所取wMilliseconds欄位的原始說明；不是時鐘實際解析度、準確度、更新間隔或跨平台一致性量測。POSIX明確傳0毫秒，不從std::time或localtime_r取得小數。

POSIX路徑未檢查time的失敗值與localtime_r回傳；lt沒有在宣告時初始化。若底層失敗，本體沒有自己提供已驗證的fallback或錯誤碼。這是靜態錯誤處理缺口，未觸發或重現失敗。

_WIN32與POSIX是同一完整body內的編譯分支，兩支原文皆保存；實際部署採哪支須核建置與可執行檔。此輪沒有查OS時區／DST規格、改系統時間、執行clock或驗證多人同時取樣。
