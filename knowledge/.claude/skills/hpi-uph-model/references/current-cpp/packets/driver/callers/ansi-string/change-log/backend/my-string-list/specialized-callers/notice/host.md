# Host：退役、授權與成功回覆的不同階段

[上層](index.md)；[Ack 副作用](ack.md)；[原文](source-manifest.json)。

## Mailbox handler 的順序

`wb_dialog_mailbox.h::NotifyAckDecide` 先接受 slot 的同一 notice ID；
再查 superseded 清單，或 older ID 且沒有 TagWaiting，才判 blocking／idle／mismatch。
older 檢查在 blocking 判斷之前。request-mismatch、not-a-notice、
no-pending-notice 與 superseded 是不同判斷；後兩者 error prefix 可供頁面關閉舊框，
但這個 reader 單元沒有驗證 JavaScript handler。

`NotifyAckHandle` 依序 Decide→複製 id→refuse(id)→retire()→close(id, ack)。
前兩個拒絕及 retire 失敗皆不呼叫 close；複製 id 在退役清 slot 之前，
避免 callback 再讀已被清掉的 requestId。ack 指標由 caller 提供且沒有 NULL guard。
它的 true 表示 retire 回 true、close callback 正常返回；
沒有 close 成功回傳檢查、snapshot match 檢查或 DB／EventLog 保存回執。

## Web command 與 panel

`W906_NoticeAckCommand` 的 close lambda 忽略 AckLikeGolden 的 bool 回傳，
用 pause／jam／passSec 與 g_dialogSeq 組 NotifyAckOkJson，再 CompleteCommand。
因此 notice=retired、pause=no-golden-note 與 jam=false 仍可是一個成功回覆。
JSON 的 seq 是傳入的 retireSeq；JSON builder 自己不退役、不套 pause、不保存 DB。

`W906_NoticePanelKeyTick` 只在 notice slot、非空 ID 且 pressed==BtnPause 時繼續，
先 auth gate，再同一 handler；成功才 W906_DialogCloseRequest。
它在退役前保存 reqSeq，用來指定要關的框；不與 JSON 的 g_dialogSeq 混為同一用途。
W906_AlarmIoAnswer、retire writer、close-request transport 的完整正文／部署尚待續。

## Auth 的現行條件與 Ack 已退役條件並存

`WebLogin.cpp::W906_NoteAuthNoticeGate` 在 noteauth Lock 下 Find request；
不是 shown notice 時返回 true，交由 handler 判斷。
兩個 start flag 都 false 才先查 SpecialLocked；Refusal 有 reason 時讓 handler 回拒絕；
SystemStart 或 SoftStart 為 true 時直接返回 true，原 pause2 註解仍在。
其後消耗一次性 passValid，要求 passK==0 且 passPressed 為空，否則進 Press。

這份 running 豁免仍是現行 gate，與 AckLikeGolden 固定 bPress=true、Refusal
if(false) 的現行條件不同。此為來源差異定位，不是完整權限或現場重啟測試結論。
沒有讀取帳密／權杖，也沒有修改授權、登入或機台狀態。

## MbWait 的 held 不等於 retired

選定 `MbWait` notifyAck 區段：gate 拒絕就回 false；gate 通過先把 wc 放
w906NoticeHeld，記 g_w906NoticeGatePassed。只有 Refusal 無 reason 且 Decide==retire
才立即回 notice=held，runsWhen 指向目前等待框關閉。
此片段沒有呼叫 DialogMailboxRetire 或 AckLikeGolden；它不是 close 完成回執。
main dispatch 以 command ID erase gatePassed，若已記錄就略過第二次 gate，
再進 W906_NoticeAckCommand。原 comment 解釋 once-pass／延後處理，完整保留。
held 的搬移／所有 wait loop／晚到命令與 close sequencing 尚未完整追完，
不能說「所有 ok 都在退役後」或「本輪證明全部 deferred ack 均完成」。
