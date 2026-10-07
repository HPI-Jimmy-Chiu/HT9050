# MSG_CMD_UPH 與命令表

來源 pin、blob、四份檔案編碼與十三片段見 [manifest](source-manifest.json)。

| 版本 | 宣告／定義定位 | 已查結果 |
| --- | --- | --- |
| V906 | MessageDef.h 的 MSG_CMD_UPH extern；MessageDef.cpp 的 MSG_CMD_UPH 定義 | `const unsigned int MSG_CMD_UPH =132` |
| V912 | MessageDef.h 的 MSG_CMD_LIST_B(X) 內 MSG_CMD_UPH row | `X(132, MSG_CMD_UPH, "UPH?", …)` |
| V912 | MessageDef.h 的 MSG_CMD_LIST(X) 與 X extern 展開 | A／B／C 表合併，產生 `extern const unsigned int name` |
| V912 | MessageDef.cpp 的 X 定義展開 | `const unsigned int name = idx`，因此該 UPH row 定義 132 |
| V912 | MessageDef.cpp 的 MSG_CMD_NAMES／MSG_CMD_COUNT | 同表將 str 展開為名稱陣列，count 由陣列 sizeof 比例產生 |

V912 的 cpp 不需直接出現字面 `MSG_CMD_UPH`；只搜尋該 cpp 的名稱會漏掉巨集產生的定義。此處是來源文字與巨集規則的靜態判讀，沒有執行編譯器或核對二進位符號。

UPH row 的 `"UPH?"` 與 Novatek 歷史註解保留；沒有查完整命令 parser、命令表索引讀者或所有客戶分派，不能由 row 認定僅 Novatek 支援。V906 此選定常數宣告沒有命令字串。

V912 命令表開頭的原註解提到 Handler／GPIB／RS232 三程式一同修改與進版號，已保存來源歷史；本單元未修改程式，也未查各部署程式版本或封包契約。

命令編號相同不能證明 VM／MV 的 ABI、跨版本互通、傳送後接收或 UPH 值口徑相同。消費端的既存局部查證見 [Command consumer](../consumers/command/index.md)；完整接收鏈與 driver 仍依 [界線](limits.md) 續查。

回 [封包索引](index.md)。
