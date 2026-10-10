# YES/NO答案與Message信箱

[上層](index.md)；[ForwardShowMyMessageBoxYesNo完整原文](raw/function-03-part-01.md)。

## 開啟與等待

g_modalServer／g_pumpQueue未就緒先return0，位於此函式的MotorAccessArmCellOnPopup／YesNoShowLikeGolden之前；與alarm早退副作用不同。
static s_showing已設或YesNoPreCloseLikeGolden==3會return3；正常popup走ArmCell、ShowLikeGolden後取得新qid。
PostQueryOptions kind yes-no以YES／NO與s1發出，接[DialogMailboxPostYesNo](raw/function-05-part-01.md)。
信箱helper：空dir false，++g_dialogSeq後YesNoRequestJson以s1／s2／s3空pointer轉空字串，填SystemInitialOK及iUnLoaderCount==0，寫Message-dialog-request。
requestedSideEffects.pauseHandler與runtime.systemInitialOK是原函式提供的欄位；不是本輪實際量到的機台狀態。
wait kind1同樣無deadline；carry空時FastClockModalWait(100)，fall-down／tick／TesterCommPoll後local.clear後先TakeCarry，再drain追加fresh，兩步都執行。

## 答案與close

matching modal.answer／dialog.response須hasTag且tag==qidStr；第一冒號拆value／pressed。
[YesNoValueOf](raw/function-07-part-01.md)只把完全相同YES→1、NO→2，其餘0；不做大小寫轉換或trim。
有效答案先CompleteCommand true、ClearQuery(qid)、[DialogMailboxRetireMessage](raw/function-06-part-01.md)。
把尚未走過的local餘項按序插到carry前端並g_carryRunnable=true後，才YesNoCloseLikeGolden(s1,s3,v)、s_showing=false與return v。
query清除與Message信箱寫idle不同步保證：empty dir早退；MessageIdleJson空則warn不寫；MailboxPut失敗warn，未回傳caller可重試的bool。
原「下次重整會再彈」comment照存，但本輪browser重整未測；true ACK不證明信箱退役成功或所有caller自動START。

## 例外命令與歷史理由

等待中現行else鏈仍處理cfg.resync、ui.windows.put、motor.stop stopOnly、act.home.abort、SimDI、IoPageNoGuards放行時的IO，以及StateRecordDuringDialog。
dialog.notifyAck在此用WaitNotifyAckReply false；其餘modeless／非本tag答案依原分支WaitOtherReply false。
完整YESNO歷史rationale在[evidence](evidence.md)連入；保留昔日「其他都modal pending」comment，另以現行body上述分支定位。
YesNoShow／PreClose／CloseLikeGolden的實作與客戶各版未在本段完整對照，來源中的golden／裁決聲明不當本輪實機驗證。
