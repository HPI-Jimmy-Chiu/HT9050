# Web接收與C路PageSave：局部caller

來源：[WebBridgeServer.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp)的 `WebBridgeServer::Impl::HandleTextMessage`、[wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)的 `main`／`wc.cmd=="editlist.save"` 分支、[_EditPage.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/_EditPage.cpp)的 `PageJson`／`PageSave`，契約宣告見[_EditPage.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/_EditPage.h)。版本／function／變數作定位，不使用程式行數。

## 接收與執行是兩層

HandleTextMessage先解析命令，再拒絕readOnly或沒有queue的情形。`editlist.save`不在該接收函式的控制權例外中；`ctrlOwner_`與連線`c.id`不相等時拒絕 `not-operator`。這是WebSocket接收時的control-owner判斷，不能改稱所有HTTP API共用同一權限，也不能由此證明從排隊到執行期間的owner始終相同。owner釋放／斷線／閒置的完整併發圖未核對。

主迴圈的editlist.save分支確認tag／FindPage；`SystemStart || SoftStart`先拒絕，再以 `OpenGateRefused(tag,true,...)`重查開窗閘，之後才解析value中的widgets／answers。該分支呼叫 `W906_ReauthTake`，以其回傳拒絕理由決定能否繼續，並在臂結束清除暫存；這次只核對caller，未讀出憑證、追完reauth實作或宣稱HSys必然有另一個重新登入點。

對HSys這類已註冊PageDesc，持FormLock後呼叫 `filerw::PageSave(*pg,widgets,answers,...)`，再由CompleteCommand回覆結果。其他獨立route／HTTP入口、目前build選項與實際伺服器設定需另外核對。

## PageSave到HSys callback

PageJson在booted後呼叫formShow，記 `ShownOf()[d.tag].shown=true`與當時AccessLevel，匯出mustSend。PageSave先查booted與「同一AccessLevel下開過頁」，不符合回409；widgets非物件、缺少不在HTEditList中但saveReads需要的欄位回400，缺proxy回500。

PageSave先處理分頁／beforeApply，再依 `ELEditable`及清單`bEnable`丟棄不可改值；`ELApplyProxies`失敗或saveReads出現無法套用的型別會拒絕並按條件reload。HSys的BeforeApply及這些helper的完整副作用仍未全量核對；此處只記呼叫順序與PageSave直接分支。

之後呼叫 `d.saveFlow()`，以 `ELMarked(d.savedMark)`決定ack.saved；未標saved時reload。即使PageSave回200，ack.saved也可能是false；流程標記與回覆碼都不能取代逐檔磁碟寫入成功、備份或實機證據。

[WebCmdGuard.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebCmdGuard.cpp)的 `WebCmdGuard::Exempt/Check`另是相同命令防連點機制；其中editlist.get列為例外，不代表接收層的owner例外或HSys存檔權限。三者分開讀，不由同一個Exempt名字推論。
