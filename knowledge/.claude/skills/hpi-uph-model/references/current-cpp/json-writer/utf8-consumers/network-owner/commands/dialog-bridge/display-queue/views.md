# active／activeNS與DOM關窗生命期

[上層](index.md)／[renderNonStop](locators.md#rendernonstop)／[closeNonStop](locators.md#closenonstop)／[completeNonStop](locators.md#completenonstop)／[render](locators.md#render)／[closeView](locators.md#closeview)。
來源 `web/page/dialog-bridge.js`；本節只核對選定完整body，HTML consumer、auth與完整caller另待續。

## 不停機顯示與關閉

renderNonStop先指定activeNS={kind, displayKind:dk, request, openedAt:Date.now()}，再取dialogNonStop、加open class、按dataset.kind切換dnWindow的show，最後sendRequest(dk)。
沒有activeNS忙碌檢查、frames.ready檢查或等待送達；DOM節點缺失時可能在activeNS已指定後throw。
closeNonStop先activeNS=null，再移除dialogNonStop的open與各dnWindow的show。這裡不取消auth、不送回覆、不drain。
實體IO target命中activeNS時，closeOne使用此關窗再走dialog-close回覆；不是此函式自行完成C++硬體動作，見[target關閉](../close-routing/targets.md)。

## completeNonStop

沒有activeNS直接return；req取activeNS.request或{}，ch從原activeNS.kind查channels，local只嚴格比req.channel==='nonstop-local'。
!local且ch存在才submit normal回覆到ch.response；檔名按原kind，payload.channel按req.channel，並非用displayKind取channel。
回覆固定schemaVersion=1.0.0、state=completed、accepted=true、closedBy=action-button、selectedAction=ACKNOWLEDGE、nonStop=true、error=null。
requestId／requestSeq沿用req.requestId／req.seq；seq=Date.now()，completedAt是當地ISO，durationMs用Date.now()-activeNS.openedAt，不是實機量測。
submit後接catch只console.warn；不await、不return該Promise，隨後立即closeNonStop。Promise rejection不讓畫面保持開啟。
local或沒有ch時跳過submit但仍closeNonStop；非local未知kind的跳過不等於有回覆檔。
submit本身同步throw或回傳不是可catch的物件時，流程可能在closeNonStop之前停止；不能把async rejection的處理推成所有失敗都會關窗。
原註解「C++不等待」保存在835全文，本輪未查所有C++caller或故障／競態情境，不列成實機驗證。

## 一般顯示與closeView

render的displayKind falsey時取kind，先指定active，包括openedAt、submitting=false、authVerified=null。
再設定dialogBridge.className、body.dialogOpen、按dataset.kind切dbWindow，清status(displayKind, '')並sendRequest(displayKind)。
沒有既有active防護或等待sendRequest成功。一般queue入口由drain檢查active；直接render入口本body無同一檢查。
closeView先cancelAuth，再清dialogBridge.className、body.dialogOpen及各dbWindow.show；本body不把active設null。
complete何時清active、提交回覆和呼叫closeView見[原關閉流程](../close.md)。不能單靠視覺關窗推論active已清或回覆已落盤。
cancelAuth／DOM更新同步throw時，後續class清理可能未完成；本輪沒有重跑瀏覽器、故障注入或硬體測試。
