# 一般追加writer與鏡像

[上層](index.md)；定位 `MySaveToFile()`、`MySaveToFileShareMode()`、`MySL_PathCombin()`、`MyList`、`HTAutoSave`、`bHanaTrayMap`、`sLastFileName`、`sPrevFileName`、`bChangeFile`。

## Gate與分流

`MySaveToFile()` 只轉呼叫share-mode。share-mode先刷新member時間；HTAutoSave=false、MyList=null或Count=0即return。HANA旗標路徑還要求 `fSCKART` 的lot number／process code caption非空，未滿足時早退保留buffer；本體沒有先檢查form pointer是否null。

| 分支 | 目標與狀態 |
| --- | --- |
| bHanaTrayMap=true | 目錄 `D:\HT9045_Log\Hana_TrayMap\年\lot caption`；名稱含GPIBMachineID、process／lot caption、HTFileName、RunInfo.LotStartTime與test-count caption，以.txt保存，更新sLastPathName／sLastNameNoPath／sLastFileName。 |
| 一般 | GetFileName產生目標；舊sLastFileName非空且變更時，設sPrevFileName與bChangeFile=true，再更新sLastFileName。本體沒有把bChangeFile重設false。 |
| SaveSameFolder=true且非FixedFile | 另存 `HTPath\AllEventLog`；TByMaxLineCount用秒級csv名稱，其他SaveType一律每日csv。 |
| SaveByLotID=true且非FixedFile、HTLotID非空 | 使用SetLotData準備的sLotFileName另存；本體不重建Lot檔名。 |

HANA分支不走後兩個mirror。FixedFile只關掉一般分支的mirror，不能據此把HANA命名改成固定csv。

## 追加與失敗結果

各share-mode目標不存在且HTFirstRow非空時，把header、CRLF與MyList->Text串接；存在時只有buffer文字。CreateFile使用GENERIC_WRITE／FILE_SHARE_READ／OPEN_ALWAYS，成功後SetFilePointer到FILE_END、WriteFile(Str.c_str(), Str.Length())、CloseHandle。

本體未核SetFilePointer／WriteFile的成功值或bytesWritten，也沒有成功布林回傳。只要走到底，MyList就Clear：開檔INVALID_HANDLE_VALUE仍會清，某個mirror失敗也不阻止後續目標。多目標不是transaction、沒有回滾或持久性保證；拋例外是否到達Clear需分開看。

| 換行 | 現行正文 |
| --- | --- |
| HANA／一般主檔 | 本體直接用MyList->Text，沒有轉CRLF。 |
| AllEventLog／ByLotID鏡像 | 保存前StringReplace把CRLF改LF。 |
| 原header與Text來源 | 目前vclcompat TStringList::GetText逐item附CRLF，因此主檔與鏡像可有不同bytes；底層平台文字模式另見專用writer。 |

cpp的20260807歷史banner仍說GetText用裸LF／無尾端delimiter；原文保留，但現行GetText已不同。本輪以4來源pin／body hash重核，沒有把舊說法搬成現在事實；GetText屬既有 [TStrings核心](../../../../string-list/core/text.md) 範圍，不重算新完成。

`MySL_PathCombin(sPath,sFile)` 若path有任何 `/`，就採FTP式 `/`，否則補反斜線後加file；不是URI解析器。歷史banner記 temporary FileInfo destructor的Sleep(1)未複製，僅保留當年說明，不宣稱最新golden一致或新增runtime驗證。
