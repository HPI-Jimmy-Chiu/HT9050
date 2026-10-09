# Notice host 延伸：退役、held 與驗證回覆

[上層 Notice](../index.md)；[上一段 gate 與 handler](../host.md)。
來源 `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`，新增 4 完整 CPP、
5 header inline、1 結構區段。
本段追 DialogMailboxRetire／MbWait／W906_NoteAuthVerify 的完整正文，
補齊 selected region 以外的流程；舊 MbWait notifyAck 小段不另算新摘錄。

| 讀取問題 | 入口 |
|---|---|
| 雙 writer、序號、失敗與 idle slot | [退役與寫檔](retire.md) |
| held、carry、answer tail 與 wait tag | [MbWait 與搬移](carry.md) |
| payload 合法、accepted、pass 與 reply fallback | [AuthVerify](auth-verify.md) |
| 版本／客戶／機台及未完成項 | [證據界線](evidence.md) |
| 原文、hash、offset、舊覆蓋與分頁 | [來源 manifest](source-manifest.json) |
| 指定 9 符號的詞法搜尋 | [符號普查](symbol-census.json) |

局部 reference 完成；完整部署、其他 wait 入口、timer／DB 保存與 UPH S8 仍待續。
未執行 CPP、硬體、機台、runtime、build、link 或 mailbox 寫檔。

## Auth callee續查（20261009）

[Dispatcher、密碼資格與live state](auth-callees/index.md)：兩處actual call、unlock／book／select條件、PTI保留與book登出、StateJson raw欄位及識別資料界線已核；完整reader／其他wait與timer仍待續。
