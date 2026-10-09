# Stop與Wake：join順序、socket與queue

[上層](index.md)；[Stop原文](raw/source-06.md)；[Wake原文](raw/source-07.md)；[thread helper](raw/source-08.md)。

Stop的WbGuard持lifeMx_直到本body退出；先stopFlag=true，th_.joinable()時Wake後join。
join返回後running.exchange(false)取得wasRunning，再關有效wake_與listener_並各設INVALID_SOCKET。
listener／client由socket thread退出時清理的原comment沿用[ThreadMain與CloseAllSockets](../transport/connections.md)；wake由Stop在join後關閉。
本次只核正文順序，沒有量到喚醒延遲或驗證其他caller重入／物件生命週期。

outMx_保護下outQ_.clear()；pendMx_保護下pending_與pendingOrder_.clear()；最後boundPort=0。
只有wasRunning為true才WsaRelease。這是本碼平衡一次成功啟動的路徑；外部host WSA或所有exception路徑另查。
此body沒有把清queue轉成browser的取消ACK，也沒有撤回已被機台命令owner取走的動作。
ctrlOwner_未在此清零；CloseConn與批次CloseAllSockets的不同責任仍見既有transport，不重算完成。

Wake在wake_==INVALID_SOCKET時直接return；否則sendto一個'w' byte至wakeAddr_，不檢查sendto回傳值。
原「prompt: do not wait out pollIntervalMs」與self-pipe註解是設計意圖，不是本輪時序量測。
Sync.h的join會WaitForSingleObject(INFINITE)，不檢查wait回傳值，接著CloseHandle並清h_／id_。
因此這份正文沒有提供已量測的Stop timeout或Win32失敗後保證；不據此聲稱程式卡住。
