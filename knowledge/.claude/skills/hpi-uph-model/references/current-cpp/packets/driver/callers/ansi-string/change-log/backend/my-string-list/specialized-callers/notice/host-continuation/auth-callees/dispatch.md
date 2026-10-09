# 兩處實際AuthVerify caller

[上層](index.md)；[驗證正文](../auth-verify.md)；[原文](source-manifest.json)。

`tools/wb_serve.cpp` 對 W906_NoteAuthVerify 的四個詞法命中包含兩份extern prototype。
真正呼叫是 ForwardShowErrorMessage 的blocking wait與main的notice dispatcher。
保存第一處完整實體行（含相鄰gate及歷史comment），第二處整段auth臂；
沒有把一行上的其他分支稱新完成的完整函式。

## Blocking與notice綁定

ForwardShowErrorMessage在dialog.auth分支，以wc.tag或空字串傳authId，
value必須是string才傳正文，currentId取該等待框qidStr、blocking=true。
naOk與naReply直接傳g_modalServer->CompleteCommand。

main只在g_alarmSlot.kind==kNotice時取其requestId，其他kind傳空currentId，
blocking=false；同樣傳tag／string value，naOk與naReply直接CompleteCommand。
前段驗證正文的currentId空會早退false；處理有效payload的true不等於accepted=true，
caller沒有將JSON accepted轉成CompleteCommand的bool。

main同一選段還保留notifyAck gatePassed的erase與auth gate／ack分支。
AuthVerify的pass是後續gate消耗，這兩處call自己沒有呼叫retire或AckLikeGolden。
完整WebCommand transport、guard／token豁免與頁面消費仍待讀；
原comment的「回應不含密碼」或「一次性通行」不是本輪完整部署驗證。

已保存的MbWait完整正文沒有這個dialog.auth處理臂；其generic未支援命令路徑
不能直接套用ForwardShowErrorMessage的blocking處理。其他wait是否接auth要逐個核對。
