# Property、初始化與緩衝

[上層](index.md)；原文以 `TMSLProperty<V>`、`BindProperties()`、`ZeroInitVclFields()`、兩個constructor、9setter、`SetLotData()`、`AddText*()`、`GetTimeInfo()` 與 destructor定位。

## Proxy與物件

- header的 `TMSLProperty<V>` 保存Owner、pointer-to-data-member與setter。讀取走ReadField，寫入走WriteMethod；proxy對proxy的assignment先轉值再寫setter，沒有把另一個Owner複製過來。
- `BindProperties()` 綁定Path／FileName／FirstRow／MaxLineCount／SaveType／AutoSave／FixedFile／SaveSameFolder／SaveByLotID，共9組；setter本體皆只賦值HT欄位。本層未建立未Bind讀寫或物件copy／move的安全保證。
- 原 `__property default=` 是歷史DFM streaming metadata，初始化以constructor正文為準；同名 `MyList` 指向另外建立的 `TStringList`，不是繼承清單。

| 初值 | 預設constructor | 三參數constructor |
| --- | --- | --- |
| MaxLineCount | 1000 | 1 |
| Path／FileName／FirstRow | Path=`D:\HandlerLog`；FirstRow空；FileName由字串初值形成空值 | 取參數sPath／sFileName／sFirstRow |
| SaveType | TByDay | CUSTOMER_CODE=CC_ASE_KaohSiung時TByHour，其餘TByDay |
| AutoSave／FixedFile | true／false | true／false |
| bFilePathWithDate／bUseFTRT／bHanaTrayMap | true／false／false | true／false／false |
| 其他 | ZeroInit將SaveSameFolder、SaveByLotID置false，7個時間Word置0；sPrevFileName空、bChangeFile=false | 同左 |

## 容量判斷順序

| API | 時間列 | 門檻與順序 |
| --- | --- | --- |
| AddText | Msg原值 | 先Add，再Count>=HTMaxLineCount，呼叫MySaveToFile後Clear。 |
| AddTextWithLineNo | 僅sprintf `%s` 後轉AddText | 本體沒有產生行號。 |
| AddTextWithDateTime | 日期、時間及毫秒；SIGURD flag省略時間後逗號的額外空白 | 原buffer Count>HTMaxLineCount時先flush＋Clear，再Add本列。 |
| AddTextWithDateTime2 | 日期用斜線、時間到秒，回傳格式化Str | 同樣 `>`、flush在Add前。 |
| AddTextWithDateTime3 | 日期一格、時分秒一格、毫秒另格，接Msg／Msg2 | 同樣 `>`、flush在Add前。 |

所以三個date-time入口允許buffer先達max+1，下一次呼叫才flush；不能把它們寫成與AddText相同的 `>=` 行為。AddText*不在入口檢查AutoSave；AutoSave=false使一般writer早退，但AddText*門檻分支仍會Clear，不能推論「關閉AutoSave永遠保留buffer」。

## 時間與Lot

`GetTimeInfo()` 使用static TDateTime取Now，再填物件自己的SystemYear…SystemMSec；同名全域時間不等於這組member，thread安全未建立。`SetLotData(ID, Time, LotFileName)` 本體沒有GetTimeInfo；ID空會清sLotFileName，否則使用外部提供名稱，或以當時member時間組 `HTPath\ByLotID\年\月\日`，檔名含machine type、SocketHandlerID、LotID與LotStartTime。剛constructor後未刷新時間就呼叫的目錄數值可能仍是0；這裡只記呼叫前提，未執行重現。

destructor在try內先MySaveToFile、Clear及delete MyList，catch轉兩參數MyDBIProcess。flush早退時cleanup仍往下走；若try中拋例外，後續cleanup不保證執行。這不是成功落盤或無例外的證明。
