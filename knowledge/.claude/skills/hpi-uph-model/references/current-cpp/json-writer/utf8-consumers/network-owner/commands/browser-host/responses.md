# web/page的答案與notice回應

[上層](index.md)；`web/page/ht9045_dialog_host.js`的[submitResponse完整body定位](raw/source-01-part-01.md#submitresponse)，其147行跨第一／二頁，見[evidence](evidence.md)。

## 入口、Message與query對應

先在呼叫當下取HT9045Recipe且要求modalAnswer是function；這個檢查在close-response noop／external-io分支之前。
Dialog-close-response檔名前綴成立resolve close-response-noop；response.closedBy==external-io則resolve closed-by-cpp-io，不再回答C++。
response.channel==show-my-message另要求dialogResponse；requestId須truthy，selectedAction物件取name或字串，轉String／uppercase且拒NONE／空值。
Message走dialogResponse(rid,act)，沒有附pressedButton；resolve回message-answered，reject訊息含no query pending或bridge is read-only時轉message-already-closed，其餘重新throw。
這是這份adapter的明確容錯口徑，不能推為任何機台端拒絕都等於已關。

alarm的rqid由非null、非空requestId轉String；ownQuery是pendingQuery存在且rqid空或與qid字串相等。
有requestId但不是本pendingQuery時，用該rqid回答，不借別人的qid／options；無自己的query且rqid空才拒沒有待答query。
[optionOf](raw/source-01-part-01.md#optionof)依selectedAction.name／字串再fallback action.name，uppercase且NONE→null。
[pickOption](raw/source-01-part-01.md#pickoption)空offered直接回want，有offered則忽略大小寫比對但回原offered值，未找到null。
ownQuery才用pickOption；非own但有rqid交C++驗選項。answered[qid]為true拒重複送。
pressedButton truthy才拼opt+冒號+String(pressed)，送modalAnswer(qid,payload)；selectedAction.code不是C++返回值，動作／實際按START或PAUSE資訊分開。
原comment「C++只印pressed、尚未消費」是歷史；目前[C++宿主](../host-modal/alarm-answer.md)已有screen/key/auth gate與close side effects，勿沿用舊結論。

## notice與有限重試

!ownQuery、有rqid、isAcknowledge且R.rawCmd是function，走dialog.notifyAck tag=nid。
[isAcknowledge](raw/source-01-part-01.md#isacknowledge)物件name uppercase等ACKNOWLEDGE或Number(code)==0即true；字串只比ACKNOWLEDGE。是前端選路判斷，不替代C++notice id／kind驗證。
tryAck(false)成功notice-retired；拒絕訊息以no-pending-notice開頭就notice-already-closed。
以not-operator開頭、首次且keepAlive存在才keepAlive後tryAck(true)，只重試一次；其他錯誤throw使框可再按，未核實際operator租期或網路重試結果。

## modalAnswer回覆與保留狀態

呼叫前answered[qid]=true。Promise fulfilled但ack.ok===false先delete answered；gone(ack.error)含no query pending則清對應pendingQuery並alarm-already-closed，否則throw拒絕。
正常fulfilled回modal-answered且pendingQuery.qid===qid才清pending；比較為strict equality。gone用String雙方比較，兩條清除口徑不同。
Promise rejected先delete answered，再gone(e.message)或重throw。外層catch把同步例外轉Promise.reject，沒有delete answered的補償。
因此「兩個非同步失敗分支允許重試」不能擴稱所有同步／非同步失敗都清answered；本輪僅文件，不改程式或宣稱重試實測。
outer try/catch覆蓋同步轉送／JSON字串處理錯誤，Promise鏈中的throw依Promise語义處理；是否全部API都回Promise由client另查。
