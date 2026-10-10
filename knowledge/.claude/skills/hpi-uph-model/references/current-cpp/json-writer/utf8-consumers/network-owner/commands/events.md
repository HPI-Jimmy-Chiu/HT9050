# PostAlarm／PostQuery：queue與query保留狀態

[上層](index.md)；[PostAlarm](raw/source-02.md)／[PostQuery](raw/source-03.md)／[IsoLocalNow](raw/source-05.md)。

| 入口 | JSON／queue順序 |
|---|---|
| PostAlarm | type=alarm、code、text、at；connId=0廣播item持outMx_加入outQ_，之後alarmsSent++、Wake、可選POST hook |
| PostQuery | type=query、qid、code、kcode、options、at；connId=0加入outQ_時另存同份pendingQueryFrame_與pendingQueryQid_，之後queriesSent++、Wake、可選POST hook |

string經sib::QuoteString，沿用[JSON adapter](../pumps/raw/source-12.md)；qid與kcodeMask直接stream輸出。
at非空便使用caller字串，不在這兩body檢查格式；空值才用IsoLocalNow。
IsoLocalNow用time(0)，MSC走localtime_s，其餘localtime失敗才清零tm，格式為YYYY-MM-DDTHH:MM:SS，沒有timezone suffix。
本helper沒有在MSC分支檢查localtime_s回傳；不把字串格式推為跨機台一致時區、成功取得時間或實測保證。

PostQuery按固定陣列順序選bit，options是名稱字串；多bit按下列順序，沒命中仍輸出空陣列：

| 順序 | bit | name |
|---:|---:|---|
| 1 | 0x0002 | SKIP |
| 2 | 0x0001 | RETRY |
| 3 | 0x0008 | TRAY_FEED |
| 4 | 0x0010 | TRAY_END |
| 5 | 0x0004 | CLEAN_OUT |
| 6 | 0x0020 | RESET |
| 7 | 0x0040 | HOME |
| 8 | 0x0080 | TRAIN |
| 9 | 0x0200 | ONECYCLE |

原Q30-KMAP rationale保存少按鈕／死結、golden顯示順序、K_FIX取消及K_PAUSE／K_START不在KeyComp的理由。
本輪只核這份V906原文，未重新比golden913、note.cpp／cmydef或重現當年故障；不補出來源沒有的按鈕。

PostQuery的兩個pending欄位保存最後設定的query frame／qid，不是多筆query歷史。
原Q30-REPLAY說明給之後連上者補發，snapshot後的query重播已見既有[DoWebSocketUpgrade](../upgrade.md)完整原文：持outMx複製非零qid的frame後，鎖外SendJson。這個body沿用不重計；ClearQuery／qid回答匹配與宿主／browser仍待接續，不能由「存了frame」宣稱已驗端到端重播。
PostAlarm本body沒有同樣的pending query保存；[PumpOutgoing](../pumps/outgoing.md)的當下連線過濾／queue移交沿用既有證據。
alarmsSent／queriesSent在加入outQ_後增加，POST hook原「screen was shown」comment保留；這些不是peer已收、已顯示或操作員回答的證據。

定位更正：先前待辦的SendSnapshot／ClearPendingQuery是在這三個固定來源檔未定位到的暫稱；現以DoWebSocketUpgrade／ClearQuery為準，不作全repo不存在的推論。
