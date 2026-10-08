# TMem 生命期、TouchSame 與失敗界線

定位 IniFiles.cpp 的 TMemIniFile constructor／destructor、W906_IniTouchSame；搭配既有 [TIniFile 綁定、UpdateFile／flush](../ini-core/binding.md) 與 [磁碟 writer](../ini-core/write.md)。完整原文見 [manifest](source-manifest.json)。

TMemIniFile constructor 走 TIniFile(fileName,kMem)，對應 writeThrough_=false 與 store_.LoadFromFile；不是 write-through 的磁碟逐次讀取。~TMemIniFile body 呼叫 flush，~TIniFile body 沒有 flush。UpdateFile 也會在 memory 模式呼叫 flush；本輪沒有驗證 caller 是否先 UpdateFile、destructor 是否抵達、FileName 是否改動或全體 ownership／shutdown。

flush 走 SaveToFile(FileName,!writeThrough_)，memory 的每次 flush 都嘗試開 wb 寫完整 store。Destructor 的歷史「不丟 mutation／idempotent」註解原樣保留；實作沒有把磁碟失敗送回 caller，不能改稱可靠保存、交易、backup 或 crash safety。

磁碟 wrapper 在 had && out==buf 時嘗試 TouchSame：Windows 只有 GetFileAttributesA **恰等於 FILE_ATTRIBUTE_ARCHIVE** 才繼續，接著以 FILE_WRITE_ATTRIBUTES 與 READ／WRITE／DELETE 分享旗標開現存檔，SetFileTime 更新 last-write；回傳 SetFileTime 是否成功。CloseHandle 沒有檢查。非 Windows 回 false，其他 attributes 或開啟失敗也回 false。

因此 out==buf 不保證只 touch：TouchSame 失敗會回原 writer 的 wb 路徑。沒有磁碟操作測試；同內容、時間戳、權限、共享鎖、其他 writer 與平台差異仍是未做的實機／環境驗證。
