# Filename與enum順序

[上層](index.md)；定位 `TSaveType`、`GetFileName()`、`HTSaveType`、`HTSaveFixedFile`、`bFilePathWithDate`、`bUseFTRT`、`IniConfig.iN10UploadProductMethod`、`LastSet.iRunStartMode` 與 `LastSet.iTester`。

## Folder先於filename

GetFileName先GetTimeInfo；HTPath空時透過Path proxy改回 `D:\HandlerLog`，FileName空的賦值仍是空。然後MyForceDirectories建立所選目錄，最後return名稱；單純取得filename也可能建立directory，故本輪不執行此function。

| 條件／順序 | Folder |
| --- | --- |
| HTSaveFixedFile=true | HTPath，不按date分類。 |
| bFilePathWithDate=false | HTPath。 |
| bFilePathWithDate且HTSaveType>=TByMonth | HTPath／年。 |
| 同上但HTSaveType>=TByDay | HTPath／年／月。 |
| 其餘period、N10方法=0 | HTPath／年／月／日。 |
| 其餘period、N10方法非0且小時<8 | 呼叫GetYesterdayInfo，用全域Yesterday日期的folder。8點起用當日。 |

TSaveType明確順序為MaxLineCount=0、Hour=1、2Hour=2、4Hour=3、6Hour=4、8Hour=5、12Hour=6、Day=7、Month=8、Year=9、Min=10。`>=` 依ordinal，因此每分鐘TByMin落在年度folder；不能按「每分鐘」名稱臆改folder層級。

## Filename分支

表中的prefix為HTFileName；日期／時間來自物件member，Yesterday只在指定N10分支替換。

| SaveType | Filename形狀 |
| --- | --- |
| FixedFile優先 | prefix.csv；忽略其他period／FT RT／offline suffix。 |
| MaxLineCount | prefix_YYYYMMDD HHMMSS.csv。這是時間命名，不是讀磁碟行數後輪替。 |
| Hour | prefix_YYYYMMDD HH.csv；bUseFTRT時HH後加_FT或_RT。 |
| 2／4／6／8Hour | iHour=SystemHour-SystemHour%period，prefix_YYYYMMDD HH.csv；本體沒有FT／RT。 |
| 12Hour、N10方法0 | 以hour%12分00／12時段，prefix_YYYYMMDD HH.csv。 |
| 12Hour、N10方法非0 | <08:00用Yesterday日期2000；08:00～19:59用當日0800；>=20:00當日2000，名稱prefix_YYYYMMDDHH00.csv，日期時間之間沒有空格。 |
| Day | prefix_YYYYMMDD.csv，offline／FT RT次序見下方。 |
| Month | prefix_YYYYMM.csv。 |
| Min | prefix_YYYYMMDD_HHMM.TXT，bUseFTRT時加_FT或_RT。 |
| 最後else | prefix_YYYY.csv；包含TByYear及其他未被前分支接受的enum值。 |

Day在CUSTOMER_CODE=CC_CYPRESS且iTester=OFF_LINE時優先用_TesterOffline.csv；否則才判bUseFTRT。FT／RT在Hour／Day／Min三個分支使用：rsmContinuRetest或rsmCInitialRetest為RT，其餘FT；12Hour與其他period不套同一suffix。

N10非0可使早上<8的Hour／2～8Hour folder是昨天，但這些檔名的日期仍用當日；12Hour另有自己的Yesterday檔名分支。folder date與filename date需分開查，不能概括成所有SaveType一律08:00換日。

GetYesterdayInfo是外部helper，本輪沒有新完成其正文；時區、DST、機台現在時間、值域或日期字面是否符合外部upload需求未驗證。輸出規格僅描述現行靜態分支，沒有重命名既有機台檔。
