# 全域時間、Yesterday與物件member

[上層](index.md)；定位全域 `cpublic.cpp::GetTimeInfo()`／`GetYesterdayInfo()`、`cmydef_core.h` 宣告與 `cmydef.cpp` 定義；物件member仍見 [MyStringList buffer](../properties-buffer.md)。

## 三組狀態

| 寫入者 | 時間取樣與目標 |
| --- | --- |
| Handler全域GetTimeInfo | static TDateTime=Now()；DecodeDate填全域SystemYear／Month／Date，DecodeTime填SystemHour／Min／Sec／MSec。 |
| Handler全域GetYesterdayInfo | 另一個static TDateTime=TDateTime(Now().Val()-1.0)；只DecodeDate填全域SystemYearYesterday／MonthYesterday／DateYesterday。 |
| TMyStringList::GetTimeInfo | 已完成的物件函式，用自己的static TDateTime與同名member欄位；不因此刷新上述Handler全域欄位。 |

GetYesterdayInfo不更新當日全域年月日或時分秒；物件先GetTimeInfo再呼叫Yesterday時，是兩次Now取樣。跨日期／時間調整可能得到不同時點的組合，這是呼叫順序界線，沒有做時區、DST、併發或跨日重現。static物件與共享Word欄位沒有在選定正文中加鎖，不能推定為原子snapshot。

## 宣告與初值

cmydef.h的20261007 S1註記說正文拆到三個子header；相容umbrella include cmydef_core.h。當前全域宣告在core，沒有照舊MyStringList banner的位置重建或修改第二套宣告。

cmydef.cpp把SystemHour／Min／Sec／MSec、SystemYear／Month與Yesterday年月日設9999；SystemDate沒有顯式initializer，作為靜態儲存期全域會先零初始化。這組初值不同於MyStringList constructor的member零值。SaveTryCatchLog讀全域且沒有自己刷新時間，故不能把物件writer剛刷新member當成free-function日期已更新的證據。

DecodeDate／DecodeTime已在 [日期轉換單元](../../../../../ini-conversion/index.md) 完成；本輪只重核2個正文。Now內部時鐘來源、TDateTime完整日期序列／負值、DST與部署時區待另查。原20260626 Val()-1.0移植註記逐字保留，不代表本輪重測BCB6或最新913。

N10的08:00／20:00切班和SG Jam的Yesterday旗標位於上層filename／writer，helper本體只提供日期欄位，不自行實作班別或UPH公式。

## 取時與日期入口補充（20261009）

[Now／Date／Time與TDateTime薄入口](time-backend/index.md)：7剩餘cpp／2region共9新原文；既有10時間body、完整header與epoch constant只重核context，完整CPP archive不增完成數。前述Now內部來源待續由此補齊，OS規格／DST／單調性／併發、檔案API與caller／版本實機仍待續。
