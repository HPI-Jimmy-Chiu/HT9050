# 所選 UPH 分派與 MyGPIBWrite

## OnMyCopyMsg 的命令群組片段

[manifest](source-manifest.json)中 OR 群組包括 `iRecvCommand==MSG_CMD_UPH`。群組按 slCmdList->Count 選 log文字，`MSG_CMD_Get_All`有自己的 log 分支；其餘使用 Str。之後以 `buffer.sprintf("%s",GHandler2Gpib->Message)`複製文字，呼叫 `MyGPIBWrite(buffer,sGbibTask)`。

這個片段未使用 MyGPIBWrite 的 bool 返回；完整 OnMyCopyMsg 的資料驗證／前置 gate、其他分支及 iRecvCommand 的解碼仍未查完。log命令名不證明外部送達。

## MyGPIBWrite 的設定與返回

本體使用兩個 `static char[StrLength+1]` buffer，s_life 與 g_bridgeLife 不同時清空。來源舊註解對 golden尾端／相容行為的說明保留為歷史，未執行重驗。

| 控制變數 | 所選本地語句 |
| --- | --- |
| iCT | 預設10，InterfaceType_Delta_Castle時5；正值 iMyGpibWriteThreshold 覆寫，達 retryMax時壓到 retryMax-1 |
| iRetryMax／iWaitMS | LastSet正值採用，否則20／100 |
| bGPIBWriteWithout_r_n | false時，字串不存在 CR／LF才各自追加；不是一律搬移／重寫尾端 |

迴圈先 UpdateLed。`(ibsta&TACS) && !(ibsta&ATN)`成立時呼叫 `ibwrt`、寫 log，直接設 bOK=true 並 break；所選語句在設 bOK前沒有另判 ibwrt的 ERR 或外部 ack。故 return true只描述這個本地分支。

否則 ibwait／UpdateLed；i>iCT後若 LACS且非ATN，改走 ibrd與讀取log再break。其餘失敗路徑記錄狀態，bNEXTSTEP_CMD為true時清旗標並回false；之後 SleepEx(100,false)，ERR時嘗試 ibfind／ibrsc／ibpad／ibtmo，ibfind<0則回false。尚未超過threshold的路徑增加 iWaitCount，依 verbose條件log，再 SleepEx(iWaitMS,false)。

最後只有 `!bOK && iWaitCount>=iRetryMax`才印 GIVE UP，清 bNEXTSTEP_CMD並回 bOK。這個條件不是對所有早退／break失敗的統一計數；`iWaitCount*iWaitMS`的log也不是實測端到端時間。

驅動包裝、UpdateLed／ibsta來源、錯誤返回與 ack、全部 writer caller、設定來源及並行 static buffer 契約仍待查；沒有執行任何寫入、重試或GPIB卡操作。回[界線](limits.md)與[入口](index.md)。
