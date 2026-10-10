# ClearQuery：qid匹配與清除界線

[上層](index.md)；[完整body與Q30-REPLAY理由](raw/source-02.md)；[Upgrade](../../upgrade.md)／[完整Upgrade](../../raw/source-05.md)；[outgoing](../../pumps/outgoing.md)。

可選g_W906OpLogHook("POST", ..., "clear", ...)在取得outMx_之前執行；就算qid不匹配也會記hook，因此clear紀錄不是實際清槽成功的證據。
取得outMx_後只有pendingQueryQid_==qid才設qid為0並clear pendingQueryFrame_；不匹配保留兩欄位。
qid=0若與inactive槽相等，仍可clear frame；本body沒有額外拒絕0。普通非零query與最後slot匹配要由caller接續核對。

這個body沒有移除outQ_中的已排隊query、沒有送browser clear frame，也沒有處理answer或解除宿主等待。
若query已在queue，清保留槽不等於撤回送出；已送／已顯示訊息與新連線Upgrade補發是不同責任。
pendingQueryQid_與frame此段持outMx_更新，但完整writer、caller順序及所有thread／重入圖仍未核，不宣稱全系統race-free。

原Q30-REPLAY說明回答／放棄後需呼叫、避免下一browser收到舊警報與no query pending，以及用qid避免抹掉新題，全文保留。
原歷史「同步模型不會發生」與安全意圖是metadata；這次未重現當年故障、未執行socket／browser／機台。
SendSnapshot／ClearPendingQuery是先前候選誤名；有限三檔定位更正已記於[既有證據](../evidence.md)，本次只保存實際ClearQuery，不重計Upgrade。
