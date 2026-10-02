# 調查：W10 R2——7016 收指令照 BCB RS232 程式「把命令接起來」（St02-M 20260927，唯讀調查）

> 裁決（Steven 20260927）：R2 **不能回 NG**，參考 BCB RS232 程式把命令接起來的做法處理。R1 修（回覆照實際長度送）、R3 要、S-a 開。
> 本文件取代 `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\w10-tcp-command-server-plan.md` 第 4 項「>99 byte 全丟」。
> 行號：RS232 golden＝`D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410\`（磁碟上最新、V906 TesterComm/Rs232 移植用的就是它；RS232Standard.agent.md:24 還寫 901.0，過期）；Handler golden＝906_0625_Steven `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`；V906＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`。

## 1. golden RS232 程式怎麼收指令（MainForm.cpp，除非另註）

- **緩衝**：成員 `vector<Byte> ReceiveData`（MainForm.h:407）；`CommTesterReceiveData` 每段都 append、從不清空（:1388-1394）。碎片是常態：TComm `ReadIntervalTimeout = 1`（MainForm.dfm:4007），9600 7E1，一點點間隔就觸發一次讀。
- **切框**：協定框是 STX…ETX（0x02/0x03，MainForm.h:20-21）。`GetAnalysisString`（:3159-3217）：
  - 第 0 byte 是 STX 就找第一個 ETX；找到就取中間，經 `MyDeCodeASCII`（cmydef.cpp:42+）解碼，再把到 ETX 為止的 byte 刪掉（:3187-3199）。
  - **半框**（還沒 ETX）：回 "_None_"，byte 留在緩衝等下一次讀（:3179 不進入）。
  - ACK／ENQ 當單一 byte 指令（:3202-3211）。
  - **其他**開頭回 "_CLEAR_"（:3212-3215），`DoRevCommand` 清掉**整個**緩衝（:3708-3711）——後面跟著的好框也一起丟。
- **一次讀到多個指令**：:1397-1404 迴圈「切框→分派→切框」。`iMaxDeal=10` 原本想當上限，但寫成 `|| iNowDeal>iMaxDeal`，一次讀到 11 框以上會無限迴圈；V906 翻譯時已修掉（`TesterComm/Rs232/Rs232Comm.cpp:145-160`；`TESTERCOMM_PORT_LEDGER.md:140`）。
- **上限**：沒有大小上限（等到 ETX 為止一直長）、半框也沒有逾時。
- **未知指令**：不回覆，只加計數（:3713-3719）。
- **壞值**：BA 指令 site 超範圍或 bin < 0 用 `continue` 跳過、照樣回 ACK（:3462-3465、:3503-3506）；bin ≥ 上限改成 999（:3459-3460）。
- 它的 TCP 模擬路徑 `ReceiveData_TCPIP`（只在 SOFT_SIMULTE，:379-383）每次讀都清緩衝（:3729-3744），不跨讀接。
- GPIB 程式的同款（`D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\RS232.cpp:246-272`）：同一個切框，但超過 102400 byte 就清空並 return（:255-257），迴圈用 `&&`（:266），最後丟掉剩的（:272）。

## 2. golden Handler（906_0625_Steven）的 7016

- **完全不接**：`TCPCommandServerClientRead`（Command.cpp:12762-14300）一次讀＝一個指令：`char EthernetBuffer[100]`（:12768）、`ReceiveBuf(…, ReceiveLength())`（:12778）、`AnsiString(EthernetBuffer)` 到第一個 NUL 為止（:12779）。
- **解析**：最多 4 欄放 `sData[4]`，每欄後面要有逗號才算（:12782-12804）：「HTGR,101」（沒有結尾逗號）不回；一次讀到兩個指令只回第一個，第二個的字跑進 sData[2]/[3]。
- **指令頭**：只有 "HTGR"（42 個區塊）與 "HTSET"（57 個），共 99（502 在註解裡）。
- **一個請求在哪裡結束**：沒有 ETX、沒有 CR/LF；到該 id 讀的最後一欄後面的逗號為止——多數 2 欄；3 欄：301、316、322、323、354、403、404、461、463、464、466、468、470、710、712、713、804、811、720（第 4 欄 OPID 可有可無，:13975）；4 欄：350（:13416-13417）、702（:13793／:13821）。
- **回覆**：ASCII `HTSR,<id>,…,`，沒有 CR/LF（例 :12809），少數沒有結尾逗號（720，:13947-13983）；在 :14299 經 `HandlerTCPIPResultSendProcess`（:12657-12723）送一次，裡面 `strcpy` 到 `cBuffer[500]`（:12662-12664）再廣播給所有 client；未知指令＝空字串（送 0 byte＋一行 log）。
- **Handler 自己的序列埠路徑早就在接碎片**：OCR.cpp:1371-1457（`Pos(_ETX_)` 再 `Delete`）、rs232.cpp:3255-3683（:3683「處理RTC指令連在一起」）。
- **超大輸入的先例**：一段超過 1024 byte 記 log 後丟掉，不截斷（rs232.cpp:3244-3252、OCR.cpp:1358-1367）。

## 3. V906 7016 收指令的做法（取代「>99 全丟」）

1. **全部讀進來**：`ReceiveLength()` 全數讀進 heap 緩衝，不用 `char[100]`。
2. **每個連線一個緩衝**：SocketHandle → string 的 map，append 進去；斷線／錯誤、`HanderTcpIp` Close/Open 時清掉該項。
3. **切框**（對應 `GetAnalysisString`）：每次最多 10 框（用 `&&`），剩下的下一個 Poll tick 繼續，即使沒有新 byte。
   - a. 框與框之間的 CR/LF/空白/TAB/NUL 直接略過。
   - b. 緩衝只是 "HTGR," 或 "HTSET," 的開頭一部分 → 等更多 byte。
   - c. 不是指令頭開頭 → 丟到下一個指令頭（或全部），記 `#Discard# n bytes`。
   - d. 指令頭之後讀 id，再等該 id 需要的逗號數（§2 的表，預設 2；用工具從 golden 的 `sData` 用法產生、ctest 驗）。逗號不夠、後面也還沒有新指令頭 → 先留著（像 RS232 的 "_None_"）；逗號還不夠就出現新的 "HTGR,"/"HTSET," → 丟掉這一框、記 `#Incomplete#`（不回；golden 對這種碎片本來也不回），從新指令頭重來。
   - e. 完整了 → 從指令頭到那個逗號是一框（720 若第 4 欄已在緩衝也一起取），從緩衝刪掉，交給**不改動**的 golden 本體（`sData` 解析、if 鏈、廣播回覆、每框一行 `#Receive#` log）。
4. **上限：每連線等待中最多 2048 byte**（golden 最大的真實請求是 HTSET,403，文字 < 1023 字，約 1040 byte）。超過又沒有完整框 → 丟到下一個指令頭、記 `#Overflow# N bytes dropped (limit 2048)`、不回、連線不斷。**不截斷**（截掉的 LotID 或數字會變成 client 沒送過的指令，例如 720、461-470）。vclcompat 的 64 KiB 佇列上限照舊當外層限制。
5. **半框不設逾時**（同 RS232）：下一個指令頭（3d）、上限、斷線會清掉。

**偏離 golden 的地方與原因**

| # | 改動 | 原因 |
|---|---|---|
| 1 | heap 讀取取代 `char[100]` | 區網任何主機都能觸發的堆疊溢位 |
| 2 | 每連線緩衝＋接起來 | Steven 裁決，照 RS232（:1391-1394、:3179）；要每連線分開，否則兩個 client 的碎片會混在一起 |
| 3 | 一次讀多框、依序各自回 | golden 只回第一個而且是亂的；照 RS232 :1397-1404，但不帶它的 `||` bug |
| 4 | 框的結尾依每個 id 需要的欄數決定 | 協定沒有 ETX；不這樣做，「HTSET,322,」先到、「5,」後到會寫到 bin 0 |
| 5 | 雜訊只丟到下一個指令頭，不整個清掉 | RS232 :3708 會把雜訊後面的好指令、或 client 的 CRLF 之後的指令一起丟 |
| 6 | 2048 byte 上限、超過丟掉並記 log | RS232 沒有上限，在網路埠上不安全；同 GPIB RS232.cpp:255 與 Handler rs232.cpp:3246 |
| 7 | 被新指令頭切斷的框丟掉 | TCP 不掉 byte，只有格式錯的輸入才會這樣；避免空寫或寫錯 |

**不變**：解析、99 個分支、回覆位元組、廣播給所有 client、未知指令送 0 byte。
**副作用**：HTSET,403 的文字現在真的能到 `ShowMyMessage`（最多 1022 字）；golden 的 `<1023` 檢查在 100 byte 緩衝下根本碰不到。

**ctest 原本的溢位項改成**：「HTGR,1」＋「01,」→ 一個回覆；「HTGR,101,HTGR,102,」→ 兩個回覆、照順序；「HTSET,322,」＋「5,」→ 寫 bin 5 不是 bin 0；一次 11 框以上 → 全部回、不卡；4096 byte 垃圾 → 記 log、下一個指令照樣回；結尾 CRLF 的指令可用；A 的半框與 B 的完整指令不會混。

## 4. HTSET,322／323 的索引超範圍

- **golden**：`iData1=atoi(sData[2])`（:13329／:13345），`BinSelect[iTestRunMode].iDBContact[iData1]=1`／`=0`（:13336／:13350）；陣列 `iDBContact[TEST_MAX_BIN]`（cprod.h:2611），`TEST_MAX_BIN=256`（MachineType.h:409），在 `BinSelect[8]` 裡（cprod.h:2681）。回覆一律 `HTSR,322,DoubleContact_On,%d,`（:13341）／`HTSR,323,DoubleContact_Off,%d,`（:13354），**沒有 NG 分支**。V906 同樣在 Command.cpp:17043-17071。
- **實際只用到 `iTestBinCount` 以下的索引**（16／17／33／255 或 iRs232MaxBinCount，cTesterIF.cpp:908-925；存 cBinSel.cpp:2553-2558、讀 :5512-5520、執行期複製 cinitial.cpp:11443-11448）⇒ golden 對範圍內但 ≥ iTestBinCount 的寫入本來就看不到效果。
- **建議：只保護陣列寫入**（同 RS232 跳過壞的 BA 項但照樣 ACK，:3462-3465）：
  ```
  if(iData1>=0 && iData1<TEST_MAX_BIN) BinSelect[iTestRunMode].iDBContact[iData1]=…;
  else TCPIPCommunicationLog("#Ignore# HTSET,322 index %d out of range 0..255");
  ```
  其餘照 golden：換頁、`spbSaveClick`（只是把沒變的資料再存一次）、同一個回覆字串並回填索引。**不夾到 0 或 255**（會把 client 沒指定的 bin 開雙接觸，是真的改變機台行為）。非數字照 golden 經 `atoi` 變 bin 0（在範圍內）。跟 golden 唯一差別是記憶體安全。用 `TEST_MAX_BIN` 而不是 `iTestBinCount` 當上限（外觀一樣，但不跟著目前 tester 模式變）。

尚未處理：HTSET,702 `StrToInt` 的例外攔截。
