# normal與dialog-close回覆欄位

[上層](index.md)／[responseFor](locators.md#responsefor)／[closeResponseFor](locators.md#closeresponsefor)；來源 `web/page/dialog-bridge.js`，活定位用function及變數。

## responseFor

取active.request；schemaVersion字面為1.0.0，channel／requestId來自request，requestSeq沿用request.seq，seq是Date.now()。
state為completed、accepted為true、error為null，是這支建構函式填的欄位；不證明傳輸、C++採用或機台完成。
有closeRequest時closedBy用closeReason或external-io；否則definition.closedBy或action-button。completedAt、durationMs由當地時間與active.openedAt計算，不是單調clock或機台量測。
auth.required用!!reqAuth.required；verified用!!active.authVerified，authId／accessLevel來自該物件或null；本函式沒有重新驗證權限。
bypassedByCloseRequest是closeRequest存在、required truthy而authVerified falsey的組合標記；不由這個標記判斷所有外部IO是否合法。
alarm的selectedAction是name與Number(code)||0組成的物件；pressedButton在closeRequest時null，否則definition.pressedButton或null。
原fNote::Start／BtnPauseClick同ReturnCode但需分後續動作的註解逐字保存，屬來源歷史說明，C++各版本仍需核對。
其他kind的selectedAction是definition.name，sideEffects五欄全部null；null代表未由本body回填，不是馬達停止、servo off或SECS動作已完成。

## closeResponseFor

schemaVersion仍為1.0.0、channel固定dialog-close，seq用Date.now()；closeEventId另一次時間值加Math.random十六進位尾綴，沒有單靠此式證明跨session唯一。
closeRequestId／closeRequestSeq有request就直接沿用，否則null。error truthy令state=error、accepted=false、closedAt=null；falsey則completed／true並填ISO時間。
target優先取response的channel／requestId／requestSeq；沒有response才取request.target的channel／requestId或空字串，requestSeq經Number(...)||0。
有trigger物件時source直接取其source（可能undefined），沒有trigger才html-action；inputName falsey轉null。
selectedAction優先truthy resolvedAction，再用response.selectedAction，否則NONE物件；closedBy取response或closeReason。
dialogWasOpen用!!wasOpen，是呼叫者傳入的狀態，不能推成所有kind或所有IO的實機結果。
normalResponse.file在有response時由channels[active && active.kind || 'alarm'].response追加.json；無response則空字串／seq零。
complete目前在清active前呼叫本函式，再明確覆寫normalResponse.file；其他caller及active缺失情況另查，不能把alarm fallback當成所有channel正確。
完整傳輸與busy重試見[原生命期](../close.md)，它和本函式建構出的completed欄位是不同階段。
