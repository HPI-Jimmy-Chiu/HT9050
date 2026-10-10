# 頁面訊息、狀態與requestId

[上層](index.md)；來源 `web/page/dialog-bridge.js`，完整[sendRequest](raw/source-01-part-01.md#sendrequest)／[onPageMessage](raw/source-01-part-01.md#onpagemessage)，跨頁全文見[evidence](evidence.md)。

## 送request

sendRequest先取frames[kind]，frame不存在或尚未ready直接return；ready來源另看HT_DIALOG_READY分支。
依displayKind先比activeNS，再比active；兩者不符合就return。channel kind與顯示kind分開，不能把alarmNonStop直接當channels中的alarm鍵。
postMessage內容為HT_DIALOG_REQUEST、kind、st.request，targetOrigin是'*'；同步throw被空catch吞掉。這不是頁面收到或C++等待解除的證據。
原註解20260922的實機故障與裁決逐字保留，是歷史證據；本輪只確認固定pin正文的狀態選擇。

## 收message

onPageMessage先檢查e.data及m.type truthiness，取kind；HT_NONSTOP_RAISE在frames／e.source過濾之前交raiseNonStop並return。
其他訊息須frames[kind]存在且e.source嚴格等該frame.contentWindow；本函式沒有e.origin判斷。這只是所讀body的條件描述，沒有窮舉外部listener或安全驗證。
auth kind先交onAuthPageMessage；HT_DIALOG_READY設ready=true，再sendRequest(kind)。兩者都在active／requestId檢查之前。
isNonStop(kind)分支要求activeNS、相同displayKind、requestId嚴格等activeNS.request的requestId；只有HT_DIALOG_ACTION呼叫completeNonStop，之後return。
原註解指不停機路徑刻意不needsAuth；此處沒有把停機與不停機共用active。實體IO關閉路徑另見inspectClose／closeOne，不能由滑鼠路徑推所有解除行為。
一般分支要求active、相同displayKind及requestId嚴格相同。ACTION以m.action或空物件組definition；pressedButton falsey轉null，ALARM_RESET設closedBy。
needsAuth truthy就openAuth，否則complete。EVENT以ht-dialog-event發出kind／requestId／m.event；不由此函式驗證事件schema或執行權限。
完整auth／nonstop render／page acknowledge消費仍待接續，不列入這七函式完成數。
