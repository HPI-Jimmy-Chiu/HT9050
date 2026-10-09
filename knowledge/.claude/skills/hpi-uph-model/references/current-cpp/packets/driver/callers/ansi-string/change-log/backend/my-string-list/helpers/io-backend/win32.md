# 既有 MyStringList 呼叫與 Win32 文件

[上層](index.md)。`Public/MyStringList.cpp::MySaveToFileShareMode`、`MyInsertToFile`、`GetLastLine` 的完整正文是既有 context；本輪不重算 writer 完成。下表的 WriteFile／Clear 序列限 MySaveToFileShareMode；其他兩個 context 各有不同路徑。

| 來源呼叫 | Win32 文件規格與界線 |
| --- | --- |
| GENERIC_WRITE、FILE_SHARE_READ、OPEN_ALWAYS、FILE_ATTRIBUTE_NORMAL | OPEN_ALWAYS 存在即開、不存在即建立，失敗以 INVALID_HANDLE_VALUE 表達；FILE_SHARE_READ 只允許相容的後續 read access，write／delete sharing 沒有因此開放。還需核既有 handle 的 access／sharing 相容性，不能把 READ share 理解成任何 reader 都必定能開。 |
| SetFilePointer(hFile,0,NULL,FILE_END) | 定位至目前檔案結尾；來源未核定位結果。它與後續 WriteFile 是兩次呼叫；不由這串實參建立跨查詢／header／buffer 的交易或物件鎖。 |
| WriteFile(...,Str.c_str(),Str.Length(),&bytesWritten,NULL) | 來源提供 buffer 與 byte 數、同步結果欄位；正文未檢查 BOOL 或 bytesWritten，也未重試或依結果保留 buffer。 |
| CloseHandle 後 Clear | 關閉與清空順序已在前批記錄；沒有以被清空狀態證明寫入成功。選定 writer 未使用 FlushFileBuffers、FILE_FLAG_WRITE_THROUGH 或 FILE_FLAG_NO_BUFFERING。 |

上述 opening／sharing 是官方 Win32 規格，不是實機讀寫測試。[CreateFileA](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea)。來源以 generic CreateFile 名稱及窄字元指標呼叫；本輪未展開完整 preprocessor／link，不據此補造已核實的 A/W 或部署工具鏈。

SetFilePointer 文件要求共享 handle 的多執行緒定位／讀写序列自行同步；來源的 FILE_END 不等於同一字串 list／global 與 caller 都有 mutex。sharing 規格也不證明各 caller 的併發設計完整。此處不是斷言所有多 writer 都能同時開同一檔。[SetFilePointer](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfilepointer)。

WriteFile 回傳與同步 bytesWritten 是檢查點。文件的非 blocking byte-mode pipe「TRUE 但較少 bytes」例子不能直接套成一般磁碟檔已重現 partial write；本輪僅記錄來源未核結果、未建立完整寫入證據。buffer bytes 不經 common.cpp 的 fputs／fopen 文字模式路徑。[WriteFile](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile)。

一般 WriteFile 可能先寫 OS buffer；FlushFileBuffers 是另有回傳值的操作。缺少明確 flush／write-through 並不能從 CloseHandle 或 MyList Clear 推定斷電持久性。檔案系統、分享路徑、硬體 cache 與斷電結果都未實測，也未增加 flush 或開檔 flags。[FlushFileBuffers](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-flushfilebuffers)、[CreateFile caching](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea#caching-behavior)。

GetLastLine 先用 read access／share=0／OPEN_EXISTING 探查，再立即關閉，以 std::ifstream 重新讀；probe 與 stream 是不同開檔，沒有一個 handle 保證兩者間檔案未變。不同 API、handle 與 CRT stream 的成功條件不能互相代替。

MyInsertToFile 在 FileExists(sLastFileName)==false 時只改用 GetFileName；在 else（查詢時既有檔）才 CreateFile／CloseHandle，再走 File 的 LoadFromFile／SaveToFile。查詢與 OPEN_ALWAYS 也不是同一步。不能把下層 TStringList writer 誤當上表的直接 WriteFile；Msg／iCount 與日期切換的上層分支仍沿用既有單元的正文。
