# 本地返回與 request 狀態

上游[SendToBridge](../flow.md)將bytes交給hub，但所選body不使用回填result。本段[manifest](source-manifest.json)完整保存七函式文字；只查其本地語句與順序。

## Hub 與 mailbox 返回

`TesterCommHub::SendToEngine`遇 `engine_==0`或 `!thread_.Running()`回 `kNoReceiver`；通過後返回 `mailbox_.Send(kHandlerSide,payload,result,timeoutMs)`。宣告預設 `kDefaultSendTimeoutMs=5000`；engine選取、thread旗標生命週期與作用中介面仍待查。

`SyncMailbox::Send`把from轉成另一side；`handler_[to]==0`時在配置request前回 `kNoReceiver`。否則複製payload，清result／done／abandoned／dropped，記from、排到queue_[to]、增加sent[from]並SetEvent。這個sent統計表示本地排入，不是外部測試機確認。

| 分支 | 本地結果 | 判讀界線 |
| --- | --- | --- |
| r->done | 可選回填*result、delete、kSent | callback結果值與SendStatus分開，不推測試機成功 |
| r->dropped | delete、timeouts[from]增加、kTimeout | 可由Reset移除排隊項造成，不只時鐘逾時 |
| elapsed>=timeoutMs且仍在queue | erase／delete、kTimeout | 尚未開始的本地request移除 |
| elapsed>=timeoutMs且不在queue | abandoned=true、kTimeout | 不取消已進行callback，後續由Process路徑處理 |

檢查順序是done→dropped→elapsed；只有done分支在result非空時回填。非kSent不能把呼叫端原result值當接收回覆。

## 等待期間的 pump 與 callback

每輪先以 `PopOne(from)`取送給自己的request，再 `Process(from,in)`；`Poll(side)`也逐一PopOne／Process。PopOne在mu_下取queue首項。呼叫端是否真的在該side所屬thread仍未驗，標頭的thread契約不能當現場已遵守。

`SetHandler`在mu_下設handler／ctx；`Process`在mu_下取fn／ctx，在鎖外執行callback，fn為空則使用-1。之後增加processed[side]，abandoned時增加abandoned並delete；否則保存result、done=true，最後喚醒from。因此即使Send見done回kSent，也必須另外判讀callback結果與engine層含義。

逾時檢查在反向pump之後；等待slice最多 `kMaxWaitSliceMs=50`。這是所選本地等待切片，不是硬性5秒完成保證；同步callback時間、nested Send、旗標／重設並行、exception／OS等待失敗與完整生命週期仍待查。

`Reset`在mu_下將兩queue中的request標dropped並clear，再喚醒兩side；此body沒有遍歷已PopOne離開queue的request。這段不是全域取消／engine停止的證明。

回[界線](limits.md)與[入口](index.md)。
