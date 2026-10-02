# Handler 的 TCP 指令伺服器 7016／結果伺服器 7017（golden TCPCommandServer／TeraTCPResultServer；V906 W10）

> 權威帳本：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md`「W10＝B」一節（偏離、上機要看）。
> 計畫與裁決：`D:\HT9045\.claude\skills\ht9045-st02-workflow\references\w10-tcp-command-server-plan.md`；R2 提案：`C:\Users\steven\.claude\skills\ops-st02-manager\references\research-r2-tcp-command-joining.md`。
> golden＝`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`（寫「906_0625_Steven」）；V906＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`。
> 這**不是** GPIB bridge 的一部分：是 Handler 自己的兩個 `TServerSocket`，給 MES／AMR／Tester 主機用 ASCII 指令問狀態、下設定、遠端啟動。

## 1. golden 怎麼做

- 物件：`main.h:121-122`；`main.dfm:17358-17377` 兩台都 `Active=False`、`Port=0`、`stNonBlocking`。TCPCommandServer 綁 OnClientConnect／Disconnect／**Read**；TeraTCPResultServer 綁 Connect／Disconnect／**Error**（沒有 Read）。
- 開：`HanderTcpIp`（Command.cpp:12623-12655）寫死 7016／7017，`Close → Port → Open`；失敗 → catch 「Socket Server Open Error!!」（ShowMyMessage＋log）。呼叫點：FormShow main.cpp:10974-10977（開機）、Start() main.cpp:6036-6041（`bHandlerResultConnect==false` 時再開一次）；:6042-6046 開關關掉時兩台 `Active=false`。
- 開關：`CosFunction.bEnableHandlerResultServer`，只有 Greatek 956／TeraPower 967／TeraProbe 804 設 true（V906 `CosFunction.cpp` :1645／:1962／:2010，預設 false :4399）。
- 收：`TCPCommandServerClientRead`（Command.cpp:12762-14301）**一次讀＝一個指令**：`char EthernetBuffer[100]`＋`ReceiveBuf(…, ReceiveLength())`（≥101 byte 蓋堆疊）；最多 4 欄 `sData[0..3]`，每欄後面要有逗號才算。
- 指令：`HTGR,<id>,…`（查詢，42 個）與 `HTSET,<id>,…`（設定／動作，56 個），共 98 個活的（HTSET,502 在註解裡）。
- 回：ASCII `HTSR,<id>,<欄位>,`，**沒有 CR/LF**；`HandlerTCPIPResultSendProcess`（:12657-12723）`strcpy` 進 `cBuffer[500]` 再**廣播給所有 7016 client**（例外重試 3 次）；未知指令＝空字串（送 0 byte＋一行 log）。
- 怪處（照留）：HTSET,317 回 `HTSET,317,…`（頭是 HTSET）；HTSET,701 在 V906 回 `HTSR,701,,`；720／721 的 NG 回覆沒有結尾逗號；702 在 `iTesterType==1` 時回兩次；333 在 Start 自己拒絕時照樣回 OK。
- log：`asTCPIPPath`（`D:\HT9045_Log\TCPIP_Log\YYYY_MM_DD\YYYY_MM_DD_HH.txt`），一行 `yyyy/mm/dd hh:nn:ss.zzz [handle][port][ #Receive#  ] …`。

## 2. V906 的形狀（W10，20260927）

| 層 | 檔（V906 樹內） | 做什麼 |
|---|---|---|
| socket | `vclcompat/ServerSocket.*` 檔尾 POLLED 模式 | `SetPolled(true)`＋`SetSimMode(false)`：Open 非阻塞 bind／listen，**不開執行緒**；`Poll()` 在呼叫者執行緒上 accept／recv 並觸發 golden 事件；`SetExclusiveAddr`（SO_EXCLUSIVEADDRUSE）、`SetBindAddress`、`BoundPort()`、`SimFailNextOpen()`；每連線 64 KiB 佇列上限（滿了不讀，不丟 byte） |
| 物件 | `forms/fMain.h:1009`、`forms/fMain.cpp` 檔尾 `W906_TcpServersCreate`；`TesterComm/Tcp/CmdServerPump.*` `W906_CmdServersEnsure()` | 兩台（Sim）＋golden dfm 綁定。**建構子那個呼叫（`forms/fMain.cpp:235`）寫在註解後面、沒有生效**，等筆電認領（`docs/handoff/ST02_FMAIN_CLAIM_20260928.md`）；在那之前由 `W906_CmdServersEnsure()` 建（W906_TesterCommInit 在 `HT9045_TESTERCOMM=0` 退出之前叫、pump Init 也叫；已經有就不做事） |
| pump | `TesterComm/Tcp/CmdServerPump.*` | Init：兩台 polled＋real＋exclusive（`HT9045_TCPCMD_SIM=1` 留 Sim）；第一個 pass＝golden FormShow 的 `HanderTcpIp()`；每 pass `Poll()`＋把完整命令一個個交給 golden 本體（每連線 ≤10）；例外在這裡接住（`#Exception#`，TODO(W10-702)） |
| 框 | `TesterComm/Tcp/TcpCmdFramer.*` | R2：每連線緩衝，依 id 欄數切框（§3） |
| 本體 | `Command.cpp:16385` 起（行數不變） | :16390 `W906_TcpCmdTakeInput` 取框；其餘 98 個分支照 golden；檔尾 `W906_TcpCmdRunFrame` |
| 回覆 | `Command.cpp:13548` 起 | R1：直接送 `sMessage` 實際長度 |
| 接線 | `TesterComm/Handler/TesterCommWiring.cpp` :90（Ensure）／:116／:123／:131／:155 | Ensure／Init／Poll（對話框等待，R3）／Tick／Shutdown；`HT9045_TESTERCOMM=0` 只建物件、不裝 pump。**20260928 `c102083d` 以前 Init／Tick／Shutdown 三個呼叫都寫在同一行的註解後面，W10 在 wb_serve 其實沒跑** |
| 333／334 | `forms/fMain.h` 檔尾 `W906_RemoteRun`；`tools/wb_serve.cpp:3795` 安裝 | Start → `TfMainWeb::StartFromWeb`、Pause → `PauseFromWeb`；**不碰基底 `TfMain::Start`**；沒裝／手動教導中 → NG |

- 執行緒：全部在 tick 執行緒（pump 第一個 Tick 記下它；別的執行緒呼叫 Poll 什麼都不做）。golden 的 VCL 訊息迴圈等價。
- 巢狀：命令開了模態等待（403 的 ShowMyMessage、333 → StartFromWeb 的告警）時，等待迴圈的 `W906_TesterCommPoll` 會跑下一個命令（golden ShowModal 也一樣）；`W906_TcpCmdRunFrame` 會還原外層的框。
- 開發機（CC 868）`bEnableHandlerResultServer=false` ⇒ 不 listen；START 時 golden 的 else 支把兩台 `Active=false`（WebStart.cpp:3483-3487）。⚠ 所以兩台物件**一定要先存在**：物件是空的，除了 Greatek／TeraPower／TeraProbe 以外的客戶按 Start 就當掉（`c102083d` 以前就是這樣，靠 Ensure 補上）。

## 3. 切框（R2；照 RS232 golden `D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410 MainForm.cpp` :1388-1405／:3159-3217／:3708-3711）

1. 框＝`HTGR,`／`HTSET,` ＋ id ＋ 該 id 要的逗號數；框之間的 CR／LF／空白／TAB／NUL 略過。
2. 欄位數（`TcpCmdFramer.cpp` 的表，`tools/tcp_cmd_fields_census.py --check` 從 Command.cpp 產生並核對，golden 0625 結果相同）：

   | 欄數 | 指令 |
   |---|---|
   | 3 | HTSET 301 316 322 323 354 403 404 461 463 464 466 468 470 710 712 713 804 811；720（第 4 欄 OPID 選填，已在緩衝同一行就一起取） |
   | 4 | HTSET 350、702 |
   | 2 | 其他全部（含所有 HTGR） |

3. 半框等下一次讀，沒有逾時；雜訊丟到下一個指令頭（`#Discard#`）；被新指令頭切斷的框丟掉（`#Incomplete#`）；**2048 byte 上限**，超過整段丟（`#Overflow# N bytes dropped (limit 2048)`），**不截斷**、不回、連線不斷。
4. 不回 NG（Steven 20260927）。HTSET,322／323 的索引超出 0..255：只不寫陣列、記 `#Ignore#`、回覆照 golden。

## 4. 還沒開的

- ~~HTSET,354 的兩個寫入~~ **已開（★W40＝A，Steven 0928）**：直接寫 Contact.Data（N＝FormatFloat("0.0000",DeviceForm_File.ForcePerPinN)、G＝命令值）；HTSET,701 整段（S3 WriteLastDataFile；fSCKART 門面缺 iCurrentStatus／iLOTSTATUS_A，A11）。
- 7017 的 HTSR,501 推播（atester_ProcessCount.cpp:2549-2576 GATE J，缺 `fObserver->TimeInfoGrid`）：7017 listen 但不推。
- HTSET,702 的例外：**★W41＝A（Steven 0928）只記錄、不回覆**。通則：golden 在該介面沒處理的指令＝不支援，只留通訊紀錄，不捏造 NG／OK。

## 5. ctest（本機只編譯；St01 跑）

`TesterComm_TcpCmdFramer`、`TesterComm_TcpCmdServer`（fMain 兩台全程 Sim；loopback 真 socket 只用 127.0.0.1 臨時 port；開頭先 `W906_CmdServersEnsure()`）、`TesterComm_TcpCmdFieldsCensus`、`START_SitesCensus`（34 31 3）。`TesterComm_Handler` 會跑 W906_TesterCommInit，所以先設 `HT9045_TCPCMD_SIM=1`。wb_serve 的 ctest 如果用 Greatek／TeraPower／TeraProbe，也要設 `HT9045_TCPCMD_SIM=1`（不然 golden HanderTcpIp 會開真的 7016／7017）。

## 6. 安全（上機前讀）

打開後，區網任何主機都能（golden 本來就這樣）：啟動／暫停機台（333／334，只在 TeraPower 或 bRemoteLotStart）、用任意 LotID 開批（720）、拿 Supervisor（310，不用密碼）、清計數與寫 lastdata、改運轉模式、跳阻塞對話框（403）、叫 AMR 補盤（812）、開關 site（404）、改 config.ini 與配方 *.Data（S-a）。防火牆只開給 MES／AMR 主機。上機要看的清單在帳本 W10 節。
