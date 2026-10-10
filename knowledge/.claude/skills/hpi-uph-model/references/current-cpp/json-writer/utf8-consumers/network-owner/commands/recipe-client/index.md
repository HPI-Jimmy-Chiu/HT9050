# Recipe Client：連線、ACK與token生命期

[回命令入口](../index.md)；[browser host回應責任](../browser-host/index.md)；[C++控制權判斷](../control.md)。同一hpi-uph-model樹，JS與CPP統計分開。

| 問題 | Reference |
|---|---|
| opening、連線／斷線、ACK分流及兩版差異 | [連線與ACK](connection.md) |
| cmd pending id、timeout、同步失敗及unwrapAck | [pending生命期](pending.md) |
| acquire／takeover、一次重試、idle與hold | [token生命期](token.md) |
| host API接法、版本／客戶／runtime、歷史comment界線 | [API與版本](contracts.md) |
| 全部原文／metadata與12函式maps保存 | [證據](evidence.md)／[manifest](source-manifest.json) |

12完整選定JS／196函式原文行；web/page全文909行、V906 tools/websync全文688行，10原文頁／2context credit0。
兩整檔的其他named function、API object methods、IIFE與anonymous callbacks都保存，但不追加完成credit；各API的所有caller和bridge／page仍待查。
兩檔不是同內容鏡像，不能由Git來源位置推論實機載入。僅靜態查證與引用保存，未執行JS、browser、機台或runtime。
