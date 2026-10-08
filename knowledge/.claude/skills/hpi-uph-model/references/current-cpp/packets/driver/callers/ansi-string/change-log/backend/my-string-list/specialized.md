# 專用writer、行數與2D mapping

[上層](index.md)；本頁說明完整函式正文，不代表實際寫檔、刪檔或customer流程已執行。

## 指定名稱與行數

| Function | Gate／操作／清空 |
| --- | --- |
| MySaveFileByFileNameAndType | 刷新時間；AutoSave=false或Count=0 return，未檢查MyList null。目錄JamAlarmLogTxt／asLotId，名稱含asFileName、日期、LotID與fileType；header在新檔時加，CRLF轉LF、fopen a／fputs，走到底就Clear，即使開檔失敗。 |
| MySaveFileByFileName | 無AutoSave或Count gate；先建立asPathName，把asPathName與asFileName組合，header／CRLF轉LF／fopen a／fputs後Clear。來源同一asFileName兼做sprintf目標與實參，重用既有AnsiString數值／printf查證，不新增alias或ABI實測。 |
| GetLastLine | GetFileName會建directory；已有檔案時先CreateFile GENERIC_READ、share=0探測並close，再用ifstream／getline計行。開檔失敗保持0；結尾設sLastFileName。數字不代表buffer Count或實際record數。 |
| MyInsertToFile | AutoSave gate在原文被註解。若sLastFileName不存在，只更新為GetFileName，沒有寫Msg；存在才probe後LoadFromFile，Count>iCount且iCount>0時Insert，否則Add，再整檔SaveToFile。結尾清File與MyList。 |

MyInsertToFile catch呼叫 `SaveTryCatchLog(slEventLog->Text, ...)`，沒有null guard；Text是繼承的base清單，不能當成內部MyList。CreateFile探測與TStringList IO分兩段，不能假設同一handle／lock保護整段rewriting。

`SaveTryCatchLog(asMessage, Func)` 是free function，使用全域SystemYear…SystemMSec，沒有自己刷新；路徑TryCatchLog／年／TryCatchLog.txt，最後呼叫WriteDataToFile。不能套物件GetTimeInfo的member時間，也未查其外部helper的成功契約。

## SG Jam

`MySaveSGJamCountToFile(bDelete,bflag)` 先GetTimeInfo及GetYesterdayInfo；gate是 `HTAutoSave==false && bDelete==false`，所以刪除請求即使AutoSave關閉仍走下去。bflag選Yesterday或當日日期；目錄HTPath／SGJamCount／年／月，MaxLineCount用秒級名稱，其他用YYYYMMDD_RawData.csv。bDelete呼叫DeleteFile，否則header／CRLF轉LF／fopen a追加；最後Clear，沒有核DeleteFile／fputs結果。本輪只保存原文，沒有呼叫刪檔。

## 2D mapping：customer與row index分開

`MyInsert2DMappingToFile(s2DID,iPlaceToUnload)` 只有barcode=false且sortingBy2DIDList=false才早退，與AutoSave無關。QUALCOMM只要求SysLotID非空，category是iPlaceToUnload的字串或E；其他客戶還要求CusLotID與CusDevGrp。名稱用三個lot欄位組2D_MappingResult csv，不因QUALCOMM放寬門檻而移除這些filename欄位。

已有檔案逐row以CommaText拆cell；首個逗號之前有空白時，先把2D ID空白暫換底線再拆，然後還原第0 cell為原ID。讀Strings[0]沒有Count gate；vclcompat越界及CSV quoting行為須循既有 [分隔文字／IO](../../../../string-list/delimited-io/index.md)，不可套BCB6例外敘述為現行結果。

| 匹配後 | 更新cell |
| --- | --- |
| QUALCOMM | Strings[10]=category，這個分支沒有>=11 gate。 |
| VS、非QUALCOMM且Count>=11 | 有bin時Strings[6]=place+1，否則Strings[5]=V。 |
| RC1、同上 | 有bin時Strings[8]=place+1，否則Strings[7]=V。 |
| 其他run mode、同上 | 有bin時Strings[10]=place+1，否則Strings[9]=V。 |

重組str3時第0 cell刪雙引號，其餘直接以逗號串接，沒有用CommaText writer重新quote。只有iCol>0才Delete／Insert並保存匹配row：第0row匹配被排除。QUALCOMM在iCol<=0的else會Add新row，所以首row匹配可能追加同ID新row；歷史「第一列不能寫回」註解不能省略這個現行customer分支。

找不到row時str3仍由最後一列拆出的list2D組成，通常受iCol>0阻擋；QUALCOMM另追加缺列。目標原先不存在時File未Load，直接SaveToFile寫空清單，沒有加入s2DID；不存在路徑與QUALCOMM的「已有檔案追加」不可合併。結尾清File／MyList／list2D並delete兩個暫存list，沒有try/catch或持久性檢查。
