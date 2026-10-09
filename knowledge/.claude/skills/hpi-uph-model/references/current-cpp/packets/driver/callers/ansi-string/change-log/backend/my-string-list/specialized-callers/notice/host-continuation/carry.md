# MbWait：等待期間 held，離開時搬回 carry

[上層](index.md)；[先前 selected notifyAck 區段](../host.md)；[原文](source-manifest.json)。

`wb_serve.cpp::MbWait` 宣告 local、W906ModalWaitScope、WaitTagScope，
再宣告 w906NoticeHeld 與 W906NoticeRelease。held 的 notifyAck 不會在
每圈直接送回 g_carry；原註解保留之前取回同命令而忙轉的歷史原因。
本輪沒有跑回歸測試，亦未把100ms等待參數當成操作員反應時間。

## 每圈與 answer tail

g_carry 為空時才 FastClockModalWait(*g_pumpQueue,100)，接著安全維護、
ModalTick(2)、TesterCommPoll、清 local、W906_TakeCarry(local)、queue.drain(local)。
TakeCarry 按 deque 順序 push_back 至 out，再 clear carry、runnable=false；
它不自己執行命令，沒有回傳執行或成功結果。

符合當前 qid 的 modal.answer／dialog.response 才走 answer 路徑：
先驗選項；OK／PAUSE 在 bWaitSecsGemReply 為真時拒絕；成功回覆後，
把 local 尚未處理的尾段插到 g_carry 開頭、設 runnable=true，return answer。
modeless answer、simDI、cfg resync、windows put、motor stop、home abort 等
各有正文分支；不能由這個 wait推定所有命令都被保留或執行成功。

## NotifyAck 與解構順序

Auth gate false 直接回拒絕。通過才 push 至 held 並記 gatePassed；
Refusal無reason、NotifyAckDecide==retire 時，先回 notice=held／runsWhen=qid closes。
held 回覆路徑沒有呼叫 retire 或 AckLikeGolden，仍非 close／落盤回執。

return 時 W906NoticeRelease 先解構，把整個 held向量插到 carry最前面、
設 runnable=true。因此 held排列在 answer分支已放回的local尾段之前；
在各容器內保留原順序，沒有證明跨所有來源的全域到達順序。
之後 WaitTagScope 才移除 waiting tag。其解構在 WaitingTags 中從尾端
移除第一個等於自身tag的項目；ctor會push一次，copy與assignment是private。
解構搬移與 tag退出的先後能由宣告順序定位，但非另個thread的同步保證。

每圈末 W906MbIoDismiss 非空便 return IO字串；已走完整個local loop，
沒有與answer分支相同的未處理尾段搬移。held仍由解構送回。
這裡沒有重跑 held命令；下一個TakeCarry caller才取走它。
普查可找其餘wait／main drain，完整鏈與巢狀等待仍待逐一保存核對。
不把RAII稱交易或例外安全保證：搬移容器可能有自己的失敗條件，未做例外實測。
