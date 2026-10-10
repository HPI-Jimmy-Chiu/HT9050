# 兩份host與client配對界線

[上層](index.md)。同一Git pin下檔案不同，不以檔名或日期推部署優先權；現場background.html／sync_web／runtime載入圖本輪未核。

| 路徑／版本 | 已保存正文與共同項 | 差異 |
|---|---|---|
| web/page/ht9045_dialog_host.js（380行） | WS回答、auth C++轉送、同步例外轉reject | query／answered、modalAnswer、notice rawCmd／keepAlive、close／external IO noop、prompt/cancel與結果整形 |
| V906 web-overlay/page/ht9045_dialog_host.js（127行） | WS回答、auth C++轉送、同步例外轉reject | api／channelOf／encode與窄dialogResponse／dialogAuth直接轉送，無watch／query／answered |
| web/page/ht9045_recipe_client.js（909行） | 四來源bytes／有限symbols定位 | modalAnswer、dialogResponse、dialogAuth等API；完整cmd／ACK／timeout正文待續 |
| V906 tools/websync/ht9045_recipe_client.js（688行） | 四來源bytes／有限symbols定位 | dialogResponse／dialogAuth API；不能直接配web/page host並假定modalAnswer存在 |

## overlay五個選定完整函式

[api](raw/source-02-part-01.md#api)取global.HT9045Recipe或null；每次轉送當下重新取，沒有建立client。
[channelOf](raw/source-02-part-01.md#channelof)檔名前綴Alarm-dialog-response／Message-dialog-response／Dialog-close-response／Dialog-auth-verify分alarm／message／close／auth，其他空字串。
[encode](raw/source-02-part-01.md#encode)selectedAction字串原樣或物件.name／NONE，pressed truthy才拼冒號；沒有web/page的uppercase／pickOption／NONE拒絕。
[submitResponse](raw/source-02-part-01.md#submitresponse)先要求api.dialogResponse（auth也先過此檢查）與已知channel，auth交dialogAuth(authId,JSON.stringify(response))，其他含close都dialogResponse(requestId,encode(response))。
auth方法存在檢查不在這條submit分支先做；呼叫同步錯誤由catch轉reject。verifyAuth另要求dialogAuth後直接傳payload JSON，沒有本單元web/page的回覆轉換。
原overlay metadata「web沒有版控」「窄方法不提供通用cmd」與部署OURS註記照存，是當時來源；不當現在repo／live client或部署完整事實。

## Handler、客戶、版本與UPH

兩adapter沒有直接MachineTypeChoice分流；可做HT9050與其他V906 Handler的共用文字路徑，但每台client、宏、權限與caller／runtime仍需獨立確認。
V906 UTF-8／C++17、V912 BCB6／Big5、V899唯讀客戶版分開；這些JS不是三版C++ golden等價性證明。
先取carry再appendfresh、gate／ACK／ClearQuery在[C++宿主](../host-modal/index.md)；原JScomment「其他一律modal-pending」「pressed只印log」已非現行宿主完整分支描述。
507來源行、13JS／302函式行與ACK時間／等待值都不是HP／Tray／site容量或UPH產能；本輪未啟任何browser／機台、runtime／程式或改共用checkout。
