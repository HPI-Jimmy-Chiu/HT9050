# AckLikeGolden：先判別 snapshot，再做 close

[上層](index.md)；[Host 順序](host.md)；[完整原文](source-manifest.json)。

## Refusal 不是 Ack 本體的內建 gate

W906_NoteNoticeAckRefusal 只在 valid、requestId 相等且 goldenNote 時檢查拒絕。
目前 active 條件是 CUSTOMER_CODE==CC_ASE_SG 且 bTesterSendPause。
原 B1 HandlerResultServer／SpecialPanel 片段在 #if 0，原文保存；
SpecialPanel 的另一個現行檢查在 WebLogin auth gate，不能一概稱密碼完全未接。
歷史 running bypass 已在此函式改成 if(false)，不是現行拒絕豁免。
Refusal 不會清 valid 或退役 mailbox。AckLikeGolden 自己不呼叫它，
所以正常 host 的前置 gate 與直接呼叫本體的條件不能混用。

## 回傳值與 pause 分開

| snapshot 條件 | 本體行為 | 輸出 |
|---|---|---|
| 非 mine | 不做 close | false、pause=3、jam=false、passSec=0 |
| mine 但非 goldenNote | 清 valid，沒有 golden close 副作用 | true、3／false／0 |
| mine 且 goldenNote、非 motor | 先清 valid，再進 close | true、pause=0，其餘依正文 |
| mine 且 goldenNote、motor | 先清 valid，再進 close | true、pause=1，跳過重套 SoftStop／SoftStart |

bPress 在本體固定 true；pause=2 沒有在這份本體賦值，header JSON 仍保留
skipped-machine-running 字串。不能用字串存在證明此分支現在可達。
motor answerApplied 跳過兩個啟停旗標，即使後來 live 狀態改變也沒有在這裡重套；
不能把 banner 的「重啟後 ack 一律再設暫停旗標」套到所有 motor notice。
valid 在副作用前清除，序列化重複呼叫不會再進同一 close；
未證明並行安全、交易回滾或中途例外後可重新補做。

## Active 副作用的資料來源

依正文順序：重設 bStartMoveSpeed；非 motor 才設 SoftStop／SoftStart；
SECS 啟用時 EventReport(DoPause)，並 SendCommand_ESD(ESD_SYSTEM_STOP)；
清通知／alarm flags。Jam 以 snapshot alarmType==1、duplicateError!=1，
配合 live LastSet.iRealDummy==REALLY 與 lamp 條件，呼叫 W906_CheckRecordJamType。
之後 W906_NoteFormCloseAlarmClear、速度預設與 lamp 清理、SW[SwManualZ1].Off。
這些是來源副作用定位，本輪沒有執行任何硬體或通訊呼叫。

接著 PassTime 取 timer 值、KeyCode=0 的 Recovery 清空、
MyDBUEventRecover 使用 snapshot eventId。此函式沒有檢查資料庫保存結果；
passSec 的輸出是該 DWORD 值轉 unsigned long，不是落盤證明。
後段重設測試／stop timer、特定 CC_KYEC_LEE retest flags、門旗標及
auto-clean／Galil delay timer 等；細節全部保存在分頁原文。

## Gate 與歷史差異保留

444 行正文中的 B2～B5、C1～C10 與 HGem close 等 #if 0／註解片段保留，
不把它們算成現行執行。active 小段及巢狀 customer 判斷依原文核對，
不能由本輪 close 局部推定完整 golden FormClose 已移植或每個客戶流程一致。
SaveErrEventLog、其他 post tail、FTP／SOP、完整 auth 驗證與部署行為仍待續查。
