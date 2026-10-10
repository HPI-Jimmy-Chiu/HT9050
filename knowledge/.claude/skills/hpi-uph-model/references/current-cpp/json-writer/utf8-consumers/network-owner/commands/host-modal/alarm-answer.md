# 告警發出、答案與關閉順序

[上層](index.md)；[ForwardShowErrorMessage原文](raw/function-02-part-01.md)的第二頁由[evidence](evidence.md)連入。

## 前置副作用、notice與阻塞query

一般alarm先AlarmStopLikeGolden＋ShowErrorMessageRecordLikeGolden；motor-note脈絡g_w906MotorNoteMsg非空走不同前置分支。
這些步驟在g_modalServer／g_pumpQueue未就緒而return 0之前，不可從早退推為全無停機／記錄副作用。
以g_nextQid++形成qidStr，NoteAuthArm記此request；kShown是kcode與九個支援K_*的交集。
非零kcode若交集0會warn，仍PostQuery並進wait；不能說不存在支援按鈕就不阻塞。
kcode==0呼叫[DialogMailboxPostAlarm](raw/function-11-part-01.md)＋NoteNoticeCapture，EmitAlarm blocking=false後ClearAlarm就返回0；沒有PostQuery或此wait。
PostAlarm helper的blocking=true與kcode==0的notice slot／WS事件blocking=false是不同欄位與層次，保留現行原文，不自行合併成一個常數判斷。
helper把W906_AlarmTextForPost(code, motor note, errPart)所得msg／description／key、auth request與unit交AlarmPost，再把g_alarmSlot.seq寫g_dialogAlarmSeq。
其原comment的buttons死欄位與dialog-page.js行為為歷史來源聲明，本輪browser未重驗；失敗bool也不證明UI已收到。
非零kcode PostQuery、EmitAlarm blocking=true；kShown非0才PostAlarm。WS retained query與檔案信箱各自有成功／失敗責任。

## wait與匹配答案

W906ModalWaitScope kind0與WaitTagScope(qid)包住無deadline的for(;;)；carry空才FastClockModalWait(100)，仍執行fall-down防護、ModalWaitTick與TesterCommPoll。
原comment明示此wait不做HT9050 1203 Poll、IO讀前次樣本；本輪不能稱新鮮IO／硬體時序已驗證。
local.clear後先TakeCarry追加既有carry，再drain追加新queue，兩步都執行；舊carry排在fresh前面。matching cmd為modal.answer或dialog.response，hasTag且tag==qidStr。
value若是字串取其內容，第一個冒號分action／pressed；九個action對RETRY、SKIP、CLEAN_OUT、TRAY_FEED、TRAY_END、RESET、HOME、TRAIN、ONECYCLE。
非對應action以CompleteCommand false回拒絕；匹配tag、已收command與operator答案通過是不同狀態。

## gate在ACK／clear之前

BtnStart先NoteScreenStartActs；非RESET再NoteWebKeyGate；之後NoteAuthAnswerGate。拒絕會false ACK／continue，保留這個query等待有效答案。
成功先CompleteCommand true，再ClearQuery(qid)、DialogMailboxRetire、ClearAlarm；Retire的bool此caller未轉為阻塞重試，不能由true ACK推兩信箱都已idle。
BtnPause設定SoftStop／SoftStart與NoteBlockingPauseLikeGolden；剩餘local[i+1..]按原序插到carry前端、g_carryRunnable=true，然後才JamCount／Start副作用／FormCloseAlarmClear。
Start可能同步觸發另一個alarm，所以餘項需先保存，見[carry](carry.md)。pressed空或BtnStart才走AlarmAnswerStartLikeGolden。
Panel IO答案另走AlarmIoAnswer、支援K與NoteAuthIoGate，再ClearQuery／Retire／DialogCloseRequest／ClearAlarm與close side effects；沒有WebCommand ticket，不杜撰同一CompleteCommand ACK。

## 等待期間仍處理的命令

| 分支 | 本體／限制 |
|---|---|
| ui.windows.put | WebWindowRegistryPut，connId及字串value |
| cfg.resync | strictInt或Double轉page；ConfigResyncJson |
| dialog.auth | NoteAuthVerify針對本qid |
| dialog.notifyAck | WaitNotifyAckReply false；本阻塞wait不當notice ack |
| motor.stop | MotorAccessWire stopOnly=true |
| act.home.abort | HomeAbortCommand |
| io.btnPanelClick | IoPageNoGuards為true才DispatchIoClick；此caller未以1203 macro包住 |
| StateRecordDuringDialog | helper處理的state記錄分支 |
| 其他modeless／SimDI | 各原分支或WaitOtherReply false |

歷史「等待中只吃answer」comment保留；現行else鏈已有上述例外，不能把舊說明當目前完整allow-list。
gate helper／panel硬體／Start副作用未在本單元做完整callee graph、編譯或實機驗證。
