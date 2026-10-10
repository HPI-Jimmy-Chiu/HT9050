# auth結果、資料生命期與取消

[上層](index.md)；`web/page`的[verifyAuth](raw/source-01-part-01.md#verifyauth)／[sendAuthCancel](raw/source-01-part-01.md#sendauthcancel)完整body在第三頁，見[evidence](evidence.md)。

## verifyAuth轉送及結果

呼叫當下要求HT9045Recipe.dialogAuth是function，且verify有authId、target.requestId；缺少時Promise.reject，沒有拿輸入帳密自行比對。
先JSON.stringify(verify)，成功後若verify.credentials存在就把其password設空字串；已序列化payload仍含原內容，直到Promise兩callback設payload=null。
相同authPrompt.authId才設submitted=true；目前此檔取消分支沒有以submitted當拒絕條件，不推成取消一定不會再送。
dialogAuth(verify.authId,payload) fulfilled取ack或空物件；value是字串則try JSON.parse，parse結果是object才替換r，parse錯保留ack。
r.accepted===true且prompt id相同才清authPrompt；回schema1.0.0／dialog-auth completed、accepted嚴格bool、accessLevel、userId null、message／stage／reason。
accepted false仍是這條fulfilled結果，不是此函式自動reject；bridge如何關登入層尚未完整核。
async rejection清payload後throw新的拒絕錯誤；outer同步catch回Promise.reject。早退／serialize失敗在password清理之前，不能稱所有路徑都清輸入或已作記憶體抹除。
原password不console／不storage意圖與歷史裁決正文完整保存；這段不做跨client／browser storage安全評測或現場權限判斷。

## cancel與兩種事件

sendAuthCancel先取authPrompt再清global；無prompt.action.requestId或dialogAuth不存在直接return，此helper並不保證每條路都回Promise。
msg含schema、dialog-auth、Date.now seq、authId、pending／cancelled=true、target alarm requestId、pendingAction name／code／pressedButton；不附credentials。
try送dialogAuth(...,JSON.stringify(msg))且catch只log warn；同步錯誤也log，未restore authPrompt或重試。log自身try/catch吞console錯誤。
message listener對HT_DIALOG_ACTION且kind alarm記lastAlarmAction，其他kind清空；HT_DIALOG_AUTH_CANCEL且kind auth呼sendAuthCancel。
ht-dialog-auth detail.state==prompt才保存authId、當時lastAlarmAction與submitted=false。此檔listener正文未檢查event.origin／source，不能推完整跨頁來源驗證；來源及部署需另查。
本輪保存anonymous listener與回覆callbacks全文，但不新增其function完成數或做browser事件實測。

## overlay的不同口徑

[V906 overlay完整原文](raw/source-02-part-01.md#verifyauth)只取api，要求dialogAuth，再String(payload.authId||空)及JSON.stringify(payload||空)直接轉送。
沒有這份web/page的target驗證、password清理、ack.value解碼、result shaping、prompt追蹤或cancel listener；不能把任一份host的保證套給另一份。
