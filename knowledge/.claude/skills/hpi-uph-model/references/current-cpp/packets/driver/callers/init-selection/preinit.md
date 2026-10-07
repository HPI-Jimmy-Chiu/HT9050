# opt-out之前已做什麼

由 [正式factory](../factory.md) 的W906_TesterCommInit接續：先安裝連線規則，g_inited未成立時才Ensure，再讀HT9045_TESTERCOMM。選讀body及原文hash見 [manifest](source-manifest.json)。

| function／變數 | 完整選讀body的行為 | 不能擴張的結論 |
| --- | --- | --- |
| W906_TesterConnectRulesInstall | 指派五個W906_Ctc*Hook函式指標，見 [規則](rules.md) | 安裝不等於當場執行五個callback，也沒有證明caller整體已離線 |
| W906_CmdServersEnsure／fMain | fMain非0且TCPCommandServer、TeraTCPResultServer兩個都0才呼叫W906_TcpServersCreate | 只有一個0時這個body不修補；不是全socket有效性驗證 |
| W906_TcpServersCreate／self | new兩個TServerSocket；result綁Connect／Disconnect／Error，command綁Connect／Disconnect／Read，lambda轉呼叫self事件 | 沒有在這個body明寫Open／Poll／port；未查constructor與事件callee，不保證不碰網路或完整副作用為0 |
| W906_CmdServerPumpInit／g_inited | 自己先Ensure；已inited、fMain為0或任一socket為0就返回 | 是另一個入口，不能把PumpInit後續行為當成opt-out返回前已執行 |
| PumpInit／bRealSockets | 通過guard後保存兩個socket、SetPolled(true)；bRealSockets為true才SetExclusiveAddr(true)、SetSimMode(false) | false分支沒有把SimMode設true；未查socket API／實際Open時機與thread生命期 |
| PumpInit／事件及stats | 保存golden read／disconnect；換Read為OnCmdRead，Disconnect先Drop再轉golden；加鎖清stats | OnCmdRead、StatsMu、framer、Poll／Shutdown及完整例外路徑不在本單元閉合 |

因此HT9045_TESTERCOMM精確字串0擋住Init後半段，但前面至少可能安裝hook與建立socket物件。不能將該旗標稱為所有Handler規則、socket及driver副作用的總關閉開關。

回 [本層索引](index.md)、[選擇](selection.md)、[界線](limits.md)。
