# vclcompat TComm 的 SIM 與啟停

定位 `vclcompat/Comm.cpp` 的 `SetSimMode`、`IsSimMode`、`StartComm`、`StopComm`、`W906_CommCloserProc`、`W906_CommJoinCloser`，六個完整定義見 [manifest](source-manifest.json)。這是 Aux consumer 的局部 transport 證據，不是所有 driver 的共同契約。

## 明確 SIM 與 fallback

SetSimMode 設 bSimForced，只有尚未 bOpen 才同步 bSim；IsSimMode 只回 bSim。開啟中的 SIM 請求不一定立刻改 getter，故須分清物件生命期與 caller 次序。

StartComm 已 bOpen 就早退；否則清 simTx。Windows 且未 bSimForced 才嘗試真 COM：

1. 等上一輪 closer，CommName 非空才 CreateFileA，以獨占、同步 I/O 開啟。
2. handle 有效時先設 hFile／bSim=false／bOpen=true，再 ApplyCommState_。
3. 清 reader stop、reset stop event、設 self，建立 reader；清 writer stop／txq，再建立 writer event 與 thread。
4. 返回。此 body 沒有檢查 CreateThread／CreateEventA 的回傳值，也不以 thread 啟動確認更新 bOpen。

明確 SIM、非 Windows 或開 COM 失敗會設 bSim=true、bOpen=true。本 body 不以 throw 回報 CreateFileA 失敗；不能擴寫成「整支函式任何狀況都不會 throw」。ApplyCommState_、reader／writer 的完整路徑仍待查。

Aux 外層會把非預期 fallback 轉為失敗，見 [consumer](consumer.md)；但外層探測與內層開啟是兩個動作。LoadSetupData 讀出的 CommName 與呼叫 StartComm 前是否另有正規化，仍需補 caller，不能假設外層前綴自動寫回物件。

## StopComm 與 closer

未 bOpen 就早退。Windows 真 COM 才走下列段：

- 設 writer stop，喚醒 event，最多等待 writer 5000 ms，隨後關 thread handle 與 event。
- 設 reader stop／stop event，最多等待 reader 2000 ms，再關 thread handle。
- 有 hFile 時先等既有 closer，再建立新 closer thread 去 CloseHandle；若建立失敗則當場 CloseHandle；之後將 hFile 設 INVALID_HANDLE_VALUE、清 self。

最後 bOpen=false，bSim 恢復 bSimForced。這些 WaitForSingleObject 的結果沒有檢查；5000／2000 ms 不能當作「worker 已退出」的證據，整段也不等於固定 7 秒上限。

W906_CommCloserProc 在 Windows 只 CloseHandle 後回 0；W906_CommJoinCloser 對非空 closer 等 5000 ms 後關 thread handle／清指標，同樣沒檢查 wait 結果。StartComm 會走此 helper；不能單憑原註解宣稱任何 driver 下都已完成 close、reopen 絕不競爭。

保留原註解的特定機台 CloseHandle 毫秒數為歷史量測，不是本輪量測，也不是 HT9050 與其他機台的 UPH 固定開銷。
