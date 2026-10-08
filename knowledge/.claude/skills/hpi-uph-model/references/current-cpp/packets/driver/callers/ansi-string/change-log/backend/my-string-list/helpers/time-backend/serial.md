# 序列算術與EncodeDate

[上層](index.md)；既有完整演算法沿用 [日期序列](../../../../../../ini-conversion/serial.md)、[輸入解析](../../../../../../ini-conversion/parse.md) 與 [範圍](../../../../../../ini-conversion/limits.md)。本輪只重核daysFromCivil／civilFromDays／serialFromTm／splitSerial／Decode／EncodeTime等既有正文，新增EncodeDate入口。

| 本輪問題 | 現行來源與界線 |
| --- | --- |
| EncodeDate(Word year, Word month, Word day) | 直接serialFromTm(year,month,day,0,0,0,0)，封裝TDateTime；正文沒有日期range／閏日檢查或invalid-date例外。不能把它當成輸入合法性驗證。 |
| epoch／單位 | 已保存constant kOleEpochDaysFrom1970由daysFromCivil(1899,12,30)而來；serialFromTm加日期差及(hour*3600+min*60+sec+msec/1000)/86400小數。 |
| Now來源 | broken-down local年月日組成序列；沒有保存UTC epoch、時區offset或時區規則。 |
| Date／Time | Date取floor(serial)，Time取serial-floor(serial)。沒有透過DecodeTime重新取樣或再四捨五入。 |
| GetYesterdayInfo | 上層已完成函式取Now().Val()-1.0再DecodeDate；這是減一個序列日，沒有重查前一天時區offset。不能推定跨DST時等於24小時實際經過時間。 |

## 負值註解與本體分開

splitSerial的既有註解說time part是absolute fractional magnitude，但正文先floor，再serial-floor，並將floor值轉成日期。以有限值-0.25靜態代入，floor=-1、小數=0.75：沿已保存epoch得到1899-12-29與18:00。這是公式推演，沒有執行C++或重測BCB6；不能依該註解宣稱負日期與原生OLE／Delphi完全相容。

TDateTime.h的單double儲存、implicit double、加減及Val已在既有完整header保存；本輪context完全相同。這些算術本身不提供合法範圍、飽和、有限值、thread同步或單調保證。日期／時間factory沒有把platform wall-clock取樣變成硬體耗時模型。

原始banner與全部CPP註解逐byte保存；未修正演算法、原註解或caller，未重新計算已完成單元數。
