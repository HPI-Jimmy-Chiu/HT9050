# host API、來源版本與歷史註解界線

[上層](index.md)；[兩整檔證據](evidence.md)；[已完成host差異](../browser-host/variants.md)／[主頁答案](../browser-host/responses.md)／[auth轉送](../browser-host/auth.md)；[C++宿主回答](../host-modal/index.md)。

## bounded API對照

以下方法全部原文存在兩全文context；本單元完成credit只計12個named function，API object method、匿名callback及其他named function保存但credit0。

| API | web/page client | V906 tools/websync client |
|---|---|---|
| modalAnswer | modal.answer；tag=String(qid)，value=String(option) | 本檔沒有此API property |
| dialogResponse | dialog.response；String(requestId或空字串)，String(actionAndPressed或空字串) | 同形方法 |
| dialogAuth | dialog.auth；String(authId或空字串)，String(payloadJson或空字串) | 同形方法 |
| rawCmd | cmd(name,extra)直接轉送 | 本檔沒有此API property |
| keepAlive | control.acquire成功→haveToken=true＋armTokenIdle | 本檔沒有此API property |
| release | 先清本地旗標／timer再送control.release | 成功回覆才清旗標 |

兩份dialogResponse／dialogAuth沒有前置acquire；不能因此推出C++在所有版本免operator gate，須接[既有C++控制權判斷](../control.md)的版本與cmd例外判斷。
主頁原註解仍寫「dialogResponse呼叫者0／兩窄方法／沒有通用cmd」；目前主頁client已有rawCmd，已完成host的show-my-message分支也會用dialogResponse。原文保留為當時記錄，不能當現行全repo consumer census。
modalAnswer正文String(qid)與dialogResponse的String(requestId||空字串)不同，0／null等轉換不能混寫。是否接受由server當前body決定。
密碼只轉送的歷史分工保留；server驗證、client序列化內容生命期及auth結果採host／C++ reference的限定範圍，不自行執行登入。

## 版本、機型、客戶與runtime

web/page主頁版及V906 tools/websync的原始碼、metadata與裁決理由分開固定pin；兩檔不同，不能稱同內容鏡像或由檔名判現場服務的是哪一份。
相同協定可能供HT9050與其他Handler共用，但每台實際client／bridge、server版本、客戶修改與部署root沒有在這12函式範圍證實。
V912量產BCB6／Big5與V899客戶唯讀碼不因這份V906／UTF-8 JS reference完成而等同已查；HT9050配置與工作檔也沒有被修改或同步到runtime。
歷史metadata含0917 build、--dry、七指令、20260911 measured、固定舊檔案行號、D盤舊Change Log與500ms streaming說明，全部存原文；現行流程不用--dry，歷史測量不是本輪實機或現行dispatch白名單證據。
本輪只新增Skill／文件和靜態保存驗證，沒有啟動browser、server、build、機台程式或runtime；deployment／sync_web apply亦未執行。

完整dialog-bridge／頁面consumer、tagFrame／訂閱與載入graph、main dispatch／monitor caller、UPH容量、版本客戶與S8／846／legacy仍待續。
