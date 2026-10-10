# 控制權與完全比對豁免

[上層](index.md)；[完整HandleTextMessage](raw/source-01.md)／[原owner欄位](raw/source-07.md)。
ctrlOwner_是socket層控制權連線id，0表示無持有人；ctrlLastCmdMs_原comment限定socket thread。
原atomic／race-free意圖全文保留；本輪未驗所有writer／生命週期／thread safety。

| cmd | 通過先前readOnly／queue等檢查後的行為 |
|---|---|
| control.acquire | owner=0或等於c.id才存c.id、更新ctrlLastCmdMs_並成功ACK；否則control-held |
| control.takeover | 上述條件多一個cmdName==takeover，允許取代當前owner；原loopback／HMI理由保留，不當本輪部署事實 |
| control.release | 自己持有才存0並成功ACK；否則not-operator；此分支未更新ctrlLastCmdMs_ |
| 非豁免指令 | owner不符則一般reject=not-operator；符合才更新ctrlLastCmdMs_，接著queue流程 |

auth.家族用compare(0,5,"auth.")前綴豁免；其餘19個名稱逐一完全比對，沒有泛用ui.／cfg.／log.／*.get豁免：

| 同類名稱（僅為閱讀分組） | 完全比對字串 |
|---|---|
| 回報／組態 | ui.windows.put、cfg.resync、log.event |
| 告警／通知回答 | modal.answer、dialog.response、dialog.notifyAck、dialog.auth |
| 停止方向 | motor.stop、act.home.abort |
| 原三查詢 | contactct.get、counterclear.get、observer.get |
| Vacuum／Pad | vacuum.get、vacuum.open、vacuum.close、pad.get、pad.close |
| SG／畫面按鍵 | act.observerSG.state、panel.key |

這些分組不構成新的權限規則；名稱以固定pin的條件鏈為準。
豁免只跳過這一段owner比較及ctrlLastCmdMs_更新，仍受先前名稱、值型別、readOnly與queue檢查。
不能把所有豁免當純讀：原comment說observer.get的Yield acts改記憶體、另有per-act gate；vacuum.open及panel.key也須接續實際handler核對。
原背景頁持權不放、cfg.resync／log.event、告警START、stop、Q2/S124、D026、Pad、SG與panel.key裁決／歷史ctest說明全保留在body。
該body沒有重新核實這些歷史實測與下游安全閘；不由文件新增／取消任何豁免。

owner指令的成功ACK只表示socket server狀態分支完成；不經CommandQueue，也不代表機台控制動作已完成。
非豁免owner更新閒置時間在QueuePush之前；後面即使queue失敗，該時間更新已發生。
token閒置／斷線釋放責任沿用[已完成transport](../transport/index.md)，本文不重做CloseConn／PumpLiveness。
