# 目前 main 的 Alarm 路徑

唯讀基準 main `7deec0f7604e893b58b6cce975c049d284ab8474`。只核對以下 function／變數，未開告警、執行測試機台或改 HTML／runtime 設定。

## ShowErrorMessage seam 與宿主

HT9011UC_Cpp_V3.33.906.0/canary_support.cpp 的 ShowErrorMessage 先記 W906_ShowErrorMessage_LastCode／LastKCode／LastDuplicate／LastErrPart，呼叫 W906_ShowErrorMessage_Hook；hook 無回覆才用 SimReturn。seam 本身不是完整原生Note流程。

tools/wb_serve.cpp 的 ForwardShowErrorMessage，非 g_w906MotorNoteMsg 的一般 note 先執行 W906_AlarmStopLikeGolden 與 W906_ShowErrorMessageRecordLikeGolden，再判斷宿主是否存在及 kcode。Stop helper 的 WAR1635 走 StopAllMotor(false)，其他走 true，並清 SoftStop／SoftStart／SystemStart。ShowMotorErrorMessage 的 note 已有自己的 stop／record，不能重複套一般 note 路徑。

所以 kcode==0 在宿主是通知／非阻塞，不代表機台不停；沒有宿主的 seam 也不能當實機告警停機證據。原文說「kCode==0缺通知ack」與早期缺口已具歷史日期，不能直接當現在無ack。

## 畫面鍵與客戶條件

forms/fNote_WebKeyGate.cpp::W906_NoteWebKeyGate 檢查 IsSafeLockCheck、掉料、bContactCTOverCHK／bAutoCleanCheckOpenDoor／bChangeCleanPad、bAutoRetestJam／bOpenAllDoor；SIM 與非SIM分支不同。CC_SCK 與 IniConfig 的 Index掉料條件有特定排除，不能整理成全機型同一規則。

同檔 W906_NoteScreenStartActs 在 SOFT_SIMULTE 回 true，非SIM只對 CUSTOMER_CODE==CC_SIGURD_PeiXing 生效；這是畫面 START，實體面板 START 走 W906_AlarmIoAnswer，不受此 helper 的同一限制。原文「Start與Pause都能確認」須先分物理鍵與客戶畫面路徑。

## 通知與過期框

tools/wb_dialog_mailbox.h 的 WaitTagScope 記錄活的等待tag；WaitOtherReply 只把沒有等待者的舊 modal.answer／dialog.response 回 no query pending:superseded-by。WaitNotifyAckReply 依通知slot與等待者判斷：舊／覆蓋通知可回 no-pending-notice:superseded-by，而目前活tag／not-a-notice仍留框。不要所有ok:false都關框。

CloseRequestWithRecent 保存最近關閉紀錄；web/page/dialog-bridge.js::inspectClose 處理 recent 並移除尚在queue的對象。queueStop 永不丟，queueNS 使用 NS_QUEUE_MAX=8；這些已在目前程式，原文 S-17 的日期／測試描述仍只代表原次查證。

## 機種與Code資料

web/page/ht9045_alarm_motionview.js::machineId 讀 parent.MACHINE.id，備援查 ?machine=，再回 HT9045；HT9050選 Alert.MotionView9050，resolve選mv9050／mv9045並允許unit覆寫。這條前端分流不是C++ MachineTypeChoice已完整接好的證據。

deriveFromCode／fillFromCode 用Code與codeUnits／AlarmCodeList的文本資料，紅框由arguments.position另行resolve。原文中「UnitName由position推導」已更正，保持這個分界；生成JSON的快照與即時機台資料不能混用。
