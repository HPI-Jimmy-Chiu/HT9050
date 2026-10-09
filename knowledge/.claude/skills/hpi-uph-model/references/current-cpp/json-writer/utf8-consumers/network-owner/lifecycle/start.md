# Start：同步準備與未核的thread建立結果

[上層](index.md)；[Start全文](raw/source-05.md)；[Sync.h全文](raw/source-08.md)；[WSA與helper](platform.md)。

Start的WbGuard持有lifeMx_；running.load()為true即回true，未重作配置或確認thread當下狀態。
WsaAcquire失敗即false；成功後stopFlag=false，再在呼叫執行緒建立listener。
原SO_REUSEADDR理由、MinGW／MSVC與同步失敗comment完整保留；它們不是本次OS契約／建置查證。

| 分支 | 正常回傳路徑中的清理／狀態 |
|---|---|
| listener socket失敗 | errOut非空才寫error、WsaRelease、false |
| bind address解析失敗或ai空 | ai非空才free，關listener設INVALID_SOCKET、寫error、release、false |
| listener bind或listen失敗 | 先保存WSA error，再關listener／設INVALID_SOCKET、寫error、release、false |
| wake socket失敗 | 關listener／設INVALID_SOCKET、寫error、release、false |
| wake bind或getsockname失敗 | 關wake與listener並各設INVALID_SOCKET、寫error、release、false |

bind address空字串改用127.0.0.1；getaddrinfo先AF_INET／SOCK_STREAM／AI_NUMERICHOST，rc非0時flags改0再試。
取ai->ai_addr的sin_addr，freeaddrinfo後才bind；完整名稱解析或外部OS行為未驗證。
listener的getsockname成功令boundPort為實際port，失敗則store cfg.port；失敗不在此回false。
其後SetNonBlocking(listener_)沒有成功狀態可供Start判斷。
wake採AF_INET／SOCK_DGRAM／IPPROTO_UDP，bind loopback及port 0，再getsockname寫wakeAddr_，SetNonBlocking(wake_)結果也不核。

## 最後狀態與bool邊界

lastGen_=0、running.store(true)，接著呼叫th_.start(ThreadEntry,this)，**忽略bool回傳**，最後回true。
Sync.h的start在已有handle或CreateThread回NULL時回false；因此本Start的true不是「已核thread建立成功」的證據。
Sync.h的new Payload沒有在本Start做exception處理；表格描述明示正常回傳分支，不是所有例外或資源狀態的證明。
本次不修改C++，也沒有觀察到實機啟動失敗。

boundPort在wake建立前更新；上述wake失敗分支沒有把它重設0，running此時尚未store true。
其後[Stop](stop.md)會boundPort=0，但呼叫端何時讀取／是否一定呼叫Stop仍待完整caller續查。
這段也未清ctrlOwner_／重置ctrlLastCmdMs_；控制權重啟語意留給命令owner，不能推定現場故障或跨客戶配置。
