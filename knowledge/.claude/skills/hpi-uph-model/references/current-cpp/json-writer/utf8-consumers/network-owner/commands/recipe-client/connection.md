# 連線、opening與ACK分流

[上層](index.md)；`web/page/ht9045_recipe_client.js`的[connect](raw/source-01-part-01.md#connect)，V906 `tools/websync`的[connect](raw/source-02-part-01.md#connect)。maps涵蓋完整body，跨頁見[evidence](evidence.md)。

## 共用順序與主頁版補充

sock存在且readyState嚴格等1，直接Promise.resolve(sock)；其後才檢查opening，有opening就沿用。
主頁版先等linkWait，再以HT9045Link.open(wsUrl())或直接WebSocket建連線；linkWait初始化另含script載入與fallback context，不能把單一open timeout稱整個connect的總時限。
建立socket的同步throw走catch，設opening=null並reject。這份body另設cfg.timeoutMs的openTmo，預設15000在cfg；時間到若sock===s直接return，否則清opening、卸掉四個handler、嘗試close並reject。
onopen／onerror清openTmo；onopen設sock、清opening再resolve。onclose沒有清openTmo，也沒有直接settle尚在opening的Promise；兩處條件要分開讀，不能寫「所有close立即reject connect」。
opening是在new Promise／then的回傳之後指定；同步catch中的null與最後外層assignment存在順序界線，本輪沒有跑event-loop／hub驗證，不能把「清opening」文字擴成所有同步失敗均可重新連線。
舊socket回呼沒有世代比對；是否能影響新sock須連同hub／所有caller另查，不作實測故障結論。

## close與訊息

兩版close都把sock=null、haveToken=false、winSupported=null、winRefusal清空、tagStat.connected=false；逐一reject pending並delete。
主頁版還清tokenTimer；這個close處理不是release已由server確認的證據。onerror本身不逐一reject現有pending；不要與onclose混寫。
JSON.parse失敗直接return；falsey m直接return。主頁版link.token owner===false先清haveToken與tokenTimer並return，其他owner值也在此type分支return。
ACK分支先處理!m.ok且error嚴格等not-operator，主頁版即使id沒有pending也清haveToken。overlay版沒有這段token失效處理。
有pending[m.id]才先取p、delete，再依m.ok truthiness resolve整份m或reject Error(m.error或command refused)，不是嚴格m.ok===true。
未命中pending的ACK仍return，不交tagFrame；非ACK交tagFrame。完整tagFrame／訂閱consumer／seq loss補救不列入本單元12函式完成。

## overlay界線

tools/websync版直接new WebSocket、沒有本body中的linkWait／HT9045Link／openTmo／link.token分支；並不由本輪替它補齊主頁功能。
兩檔完整原文與歷史註解照存，檔名相同不代表部署同一版；實機服務root與sync成果另查。
