# 送答順序與成功回傳界線

[上層](index.md)；完整[submit](raw/source-01-part-01.md#submit)／[submitRest](raw/source-01-part-01.md#submitrest)，來源 `web/page/dialog-bridge.js`。

## 宿主失敗轉下一段

HTDialogHost存在且submitResponse是function時，submit先try Promise.resolve(HTDialogHost.submitResponse(responseName+'.json',response))。
宿主同步throw轉Promise.reject；宿主Promise rejection由first.catch印warn後呼叫submitRest。宿主fulfilled直接回它的結果，不再走其他段。
沒有宿主API則直接回submitRest。原20260924「沒有待答query」與同步throw案例完整保留，不能寫成所有傳輸例外都已被submit包住。
first.catch中的submitRest同步throw會成為該Promise rejection；直接return submitRest的路徑則可能同步throw，兩條順序須分開。

## submitRest的三種傳輸

HTJsonWriter存在且ready() truthy，直接return write(responseName,response)；本body沒有try或Promise.resolve包裝它，也不在write失敗後自動再走WebView。
其後若chrome.webview.postMessage是function，送HT_DIALOG_RESPONSE（file追加.json）並Promise.resolve('webview-posted')。
WebView分支沒有等待C++回覆；postMessage同步throw不在本body處理。不要把webview-posted稱實際寫檔、ACK或機台解除。
再下一段global.postMessage(...,'*')後才檢查isFileDebug；因此postMessage不是最後成功fallback，普通模式仍回transport-not-connected的Promise rejection。
isFileDebug在全文context中要求location.protocol==='file:'且location.search符合/[?&]mode=debug/；兩項同時符合才把response保存在__HT_DEBUG_RESPONSES__並resolve('debug-local')。
單純mode=debug的HTTP頁面不符合這支判定；debug-local的原warn明說未送C++，不能當機台驗證。regex是否匹配其他query尾綴按原式保存，不改寫成已完整解析參數。
宿主modalAnswer／dialogResponse與Recipe ACK路徑另見[宿主](../browser-host/index.md)及[Recipe](../recipe-client/index.md)；本函式不證明每一傳輸共用同一server契約。
