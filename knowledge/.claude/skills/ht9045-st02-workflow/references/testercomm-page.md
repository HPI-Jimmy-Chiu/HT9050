# testercomm.html（測試通訊視窗）：Test Result 分頁的共用面板與唯一的 JSON

這是 St02 的頁面和檔案。20261001 依 Steven 的要求改版，分支 `v906/st02-tc-shared-panel`。
JSON 的形狀照 St02-E2 的對照表 `D:\AI_TempFile\st02-e2\review\ST02_TC_HOME_JSON_MAP_20261001.md`，再加上 St02-M 依 golden 做的裁決（20261001，沒有問 Steven）。

## Steven 的原話（20261001）

- 「我以為site01~32的面板是共用的!」「然後通訊Log的memo也是共用的」「請協助調整畫面」
- 「截圖的元件只有一份, 在首頁」「然後主要通訊log也是只有一份, 在首頁」「其他的設定值可以根據不同的測試模式進行分頁」
- 「C++端在 32site 與 通訊log部分, 也是整合一個JSON進行發送」「不需要根據不同的格式呼叫不同的JSON」
- 「做好要寫skill」
- 14:5x（看了 main 上的舊版截圖）：「應該是要新增一個分頁叫做 test result」「裡面放 gpsite00~31的面板, 跟 MemoLog」「然後另外三頁gpib / rs232 / tcpip 裡面就不需要上面兩項」⇒ 共用的分頁叫 **Test Result**、Log 標題叫 **MemoLog**（原本叫「首頁」「主要通訊 Log」，結構不變；API 照舊是 `/api/testercomm/home`）。
- 15:0x（看了新版之後）：「我看懂了」「debug模式顯示上面的tab沒問題」「release模式下, 只留目前使用中的通訊格式就好」「記得加入到skill之中」 ⇒ 見下面「分頁：debug 全部顯示、release 只留使用中的那一個」。

## St02-M 的裁決（20261001，照 golden）

- (a) **站格不上色。** golden 三支程式都沒有改過 `plSite->Color`，一律是 clSilver；Steven 的截圖也是銀色。JSON 可以帶由文字推出來的 `state`，但網頁不依它改顏色。
- (b) **Log 照 golden 原文送**，另外附 `tag` 和 `seq`。不加標籤，也不把時間和內容拆開。
- TCP 的勾選框 `cbSiteOn` 在 golden 是死的：元件照留，`enabled: null`，不接任何指令。
- TCP 的 `mmTCPIPCommLog` 要接上，TCP 模式的 Test Result 分頁 MemoLog 才有資料。
- `binItems` 各模式照 golden 清單：GPIB 19 項、RS232 8 項、TCP 9 項。

## golden 依據（906_0625_Steven 與兩支橋接程式）

一台機台只跑**一個介面**的程式。每支程式只有**一個站台面板**和**一個通訊 memo**。三支程式的站格長得一樣：

- 群組框「Site NN」；
- 狀態方塊 `plSite`，只用文字表示狀態；
- 勾選框 `cbSiteOn`；
- 下拉 `cbBin`，選模擬測試時要回的 bin；
- 條碼行 `labOcr`。

| 介面 | 程式 | 站台面板 | 通訊 memo | 一行的格式 |
|---|---|---|---|---|
| RS232／TTL | RS232Standard（`D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410`） | MainForm.dfm:3893 `palSite`（RS232 和 TTL 共用） | :3865 `MemoLog`，由 `ShowCommData` 寫入（MainForm.cpp:2231-2249） | `YYYY-MM-DD, hh:mm:ss.zzz, [unit], "m1","m2"`（MyStringList.cpp:84） |
| GPIB | H9046_32GPIB（`D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525`） | Main.dfm:198 `palSite` | :818 `lstRecord`，由 `WriteLog` 寫入（Main.cpp:701-724，超過 10240 行清掉） | `YYYY-MM-DD, hh:mm:ss.zzz, text` |
| TCP/IP | Handler 的 TfTesterTCP | Interface/TesterTCP.dfm:116 `plSite01`～`32`（設計時就有 32 組） | :37 `mmTCPIPCommLog`，由 `AddTCPIPCommunicationLog` 寫入（TesterTCP.cpp:276-300，超過 2000 行清掉），同時抄一份到 LotInfo 的 `mmTesterLog` | `YYYY-MM-DD,hh:mm:ss.zzz,TCPIP,text`（逗號後面沒有空白） |

- GPIB 的 `Memo1` 是 **Version 分頁**的版本紀錄（Main.dfm:1510-1525），不是通訊 log。RS232 的版本紀錄是 `MemoVer`（tsVersion）。
- log **檔**照舊各寫各的：`D:\GPIBLOG`、`D:\RS232Log`、`D:\HT9045_Log\Test_TCPIP`。這次只改畫面。
- golden 的怪寫法照留，註解已寫在程式裡：
  - GPIB 下拉第 15 項（"15"）回的是 bin 33；
  - RS232 下拉第 7 項顯示 "0..11"，實際亂數是 1～10；
  - TCP 下拉第 9 項 "1..16"（還有 -1）在 SimulateBin 落到 else：`random(15)+1`，實際只到 15（TesterTCP.cpp:565-582；E2 MR !46 m2）。

## 版面

| 分頁 | 內容 |
|---|---|
| **Test Result**（第一頁、預設） | 唯一的 Site01～32（gpSite00～31，4×8 格，`HTWidgets.makeDutPanel`）＋唯一的 MemoLog。上面一列顯示 Tester Type、來源、連線中／已停止。Log 標題顯示 `[tag] from`，例：「MemoLog　[GPIB] lstRecord」 |
| GPIB | 只放 GPIB 程式自己的設定和專屬 memo：LED、選項、AMD／HANA、tsRS232 的 COM 設定與 COM Log、mmoBINON、memoAlarmCode |
| RS232／TTL（DIO） | Standard／TTL 的 COM 設定和按鈕、MemoBinData、MemoBinData_TTL |
| TCP/IP | 連線狀態、位址、收到的資料（SocketTCPIPReceiveList）、最後一筆 |
| Version | GPIB 的 `Memo1`、RS232 的 `MemoVer` |

模式分頁**沒有**站台格子，也**沒有**主 Log。

### 分頁：debug 全部顯示、release 只留使用中的那一個（Steven 20261001 15:0x）

| 組態 | JSON `build` | 顯示的分頁 |
|---|---|---|
| 模擬（debug；`build` 目錄，SOFT_SIMULTE 開） | `"sim"` | 全部：Test Result／GPIB／RS232・TTL（DIO）／TCP/IP／Version |
| 出貨（release；`build_ship`，`W906_NO_SOFT_SIMULTE`） | `"ship"` | Test Result＋目前使用中的那一頁（`source`：gpib → GPIB；rs232 → RS232・TTL（TTL 也走 RS232Standard）；tcpip → TCP/IP）＋Version |

- 出貨版的 Version 分頁只顯示那支程式自己的版本紀錄（gpib → GPIB `Memo1`；rs232 → RS232 `MemoVer`），輪詢也只拿那一支；TCP/IP 沒有版本紀錄，所以出貨版的 TCP/IP 不留 Version。
- Tester Type 對不到介面（`source:"none"`）→ 出貨版只留 Test Result。
- 依據：golden 一台機台只跑一支介面程式，畫面上只有那一支的分頁。
- 誰決定：C++ `testercomm::HomeBuild()`（UiHome.cpp，`#ifdef W906_NO_SOFT_SIMULTE` → `"ship"`，否則 `"sim"`），放在 home JSON 的 `build`（緊接在 `schema` 後面）。網頁不自己猜組態。
- 網頁：`applyBuildTabs(h)`（testercomm.html）。只有**收到真的 home JSON** 時才更新（Demo 的假資料、離線都不改）；還沒收到之前全部分頁都顯示。被藏起來的分頁不能選（`selectTab` 改開 Test Result，Setup.TesterIF 的「Tester TCP」鈕也一樣）。
- 已知限制：分頁只在 Test Result 分頁輪詢時更新。如果視窗停在某個模式分頁時換了配方的 Tester Type，要回到 Test Result 分頁才會換成新的那一頁。
- 測試：ctest `TesterComm_HomeJson` 檢查兩組態的 `build` 值和欄位順序；網頁用假 DOM 的 `smoke_build.js`（St02-E 的 scratchpad，11 項：模擬全部、出貨 gpib／rs232／tcpip／none、Version 只拿一支、沒資料時保留、回到模擬）。

## 唯一的 JSON：`GET /api/testercomm/home`

每種模式都是**同一個形狀**，欄位順序也固定。Test Result 分頁只呼叫這一個端點，不依模式分支。

```
{ "schema": "testercomm.home/1",
  "build": "sim" | "ship",              // 模擬（debug）／出貨（release）：出貨版網頁只留使用中的那一頁
  "testType": 0..3,                     // TestIF_File.iTestType
  "testTypeName": "TTL" | "GPIB" | "RS232" | "TCP/IP" | "",
  "source": "rs232" | "gpib" | "tcpip" | "none",     // 站格是哪一支 golden 程式填的
  "up": bool,                           // 那支引擎在跑；TCP 是 TCP 模式而且 ON-LINE
  "siteCount": 32,
  "binItems": [...],                    // 這個模式的 cbBin 清單（GPIB 19／RS232 8／TCP 9）
  "binEditable": bool,                  // 下拉可以送 site <i> bin <n>
  "enabledEditable": bool,              // 勾選框可以送 site <i> on 0|1；TCP 是 false
  "sites": [32 × {"no","caption","text","state","color","enabled","bin","binText","ocr"}],
  "log": {"tag": "GPIB"|"RS232"|"TTL"|"TCPIP", "from": "lstRecord"|"MemoLog"|"mmTCPIPCommLog",
          "seq": n,                     // 到目前為止加進來的行數
          "lines": [最後 200 行，golden 原文]} }
```

- **testType 對應**：
  - 依據是 cmydef.h:61-64；golden 讀檔時預設 0（cTesterIF.cpp:572-578）。
  - 0 TTL → `rs232`（TTL 走 RS232Standard，裁決 T17）；1 GPIB → `gpib`；2 RS232 → `rs232`；3 TCP_IP → `tcpip`。
  - 其他值 → `source:"none"`，欄位照舊、內容是預設值。
- **每一格的欄位**：
  - `text` 是 `plSite->Caption` 原文。
  - `state` 由 `text` 推出來，只當參考，網頁不用它上色：
    - "T" → testing；
    - 數字 → bin；
    - "999" → error；
    - "----" → off；空白且沒勾 → off；
    - "--" 或樣板的 "01" → idle。golden 的 bin 一律是 `%d`／`%4d`，不會有前導 0，所以 "01" 一定是樣板字。
  - `color` 一律是 clSilver（12632256）。
  - `enabled`：1 → true，0 → false，TCP 為 null。
  - `bin` 是 ItemIndex，-1 表示沒選；`binText` 是 cbBin 的 Text。
- **TCP 那一份**：
  - 站格標題在 dfm 是固定字，所以補成 "Site NN"；
  - `binItems` 用 dfm :96 的 9 項；
  - Log 從 `fLotInfo->W906_TesterLogTail(200, …)` 讀。移植樹的 `mmTCPIPCommLog` 只是計數的替身（Interface/TesterTCP_Socket.h），原文只存在 golden 抄的那份 `mmTesterLog`，兩份在 2000 行時一起清掉。
  - ⚠ 不要用 `mmTesterLog->Lines->Get(i)`：那是 `TfMainMemoLines` 的非虛擬函式，對 `TfLotInfoLogMemo` 每一行都會回空字串。第一版就踩過這個坑。
- **log.seq 的算法**（`testercomm::HomeLogSeq`）：
  - 引擎每次只送最後 200 行。C++ 拿這次的尾端和上一次的尾端比重疊，找出新增了幾行（取能對上的最少新增數；完全對不上就算全部是新的）。
  - golden 清掉 memo 不影響累計。
  - 每支引擎在自己的執行緒上各有一個 `static HomeLogSeq`。
- **網頁怎麼接 Log**：
  - `tag`／`from` 變了，或 `seq` 變小 → 整段換掉；
  - 否則新增 k＝`seq`−上次的 `seq` 行；k 比 `lines` 多 → 整段換掉；
  - 網頁自己最多留 2000 行。
- **引擎還沒發過**：`up:false`、`binItems:[]`、兩個 editable 都是 false、32 格預設值、`log.seq:0`、`lines:[]`，欄位一樣。
- **POST** `/api/testercomm/home?cmd=site <0..31> on 0|1`、`site <0..31> bin <index>`：
  - 轉給目前介面的引擎（GPIB／RS232）；
  - TCP/IP 只收 `bin`，由 TcpPump 設 `cbSimulateBin[i].ItemIndex`，golden 的 SimulateBin 會讀它；
  - 其他指令回 200 `queued:false`。

### C++ 怎麼合成（TesterComm 自己的檔）

- `TesterComm/UiHome.h`／`.cpp`（ht9045_testercomm）：形狀、對應、`HomeStateOfText`、`HomeLogSeq`、`HomeFragment`、`HomeCompose`。
- 每支引擎在自己的執行緒上發 `"<key>.home"`：
  - `gpibbridge::BuildHomeFragment`（GpibUiSnapshot.cpp，由 GpibEngine.cpp 發）；
  - `rs232std::BuildHomeFragment`（Rs232UiSnapshot.cpp，由 Rs232Engine.cpp 發）。tag 依 RS232Standard 自己的 `iUseRS232Mode >= InterfaceType_TTL` 決定 TTL 或 RS232。
- `TcpPump`（Handler 執行緒，TestIF_File 在這裡）每 200 ms 做三件事：
  1. 取走 `"tcpip"` 的 bin 指令；
  2. 組 TCP 那一份；
  3. 依 iTestType 選一份，`HomeCompose` 後發 `"home"`，並記下目前介面給 POST 用（`W906_TesterCommSetHomeTestType`）。
- 路由在 `TesterComm/Handler/TesterCommWiring.cpp`：`KnownKey` 多了 `home`；內部用的 `<key>.home` 不對外。
- **GPIB／RS232 的 home 由路由在 GET 時現組**：`HomeCompose(W906_TesterCommHomeTestType(), Read("<key>.home"))`。
  - 原因：Handler 開著警報框時，主迴圈只跑 `W906_TesterCommPoll`，不跑 TcpPump，TcpPump 發的 `"home"` 會停在最後一份；golden 的 GPIB／RS232 是另外的程式，畫面照樣在動（E2 MR !46 m1）。
  - 引擎還沒發過：一樣回 200，`up:false`、欄位相同，不是 404。
  - TCP/IP、或 TcpPump 還沒跑第一拍：用 TcpPump 發的那一份。
- ⚠ 還沒做（另開卡）：警報框開著時 **TCP 通訊本身**也會停（TcpPumpTick 不在 `W906_TesterCommPoll` 裡，golden 的 VCL timer 在 ShowModal 裡照跑）。要加的話得有重入保護，因為 TCP 的處理本身也可能開框。
- 原本各引擎的 `/api/testercomm/<gpib|rs232|tcpip>` 照舊，給模式分頁用。

## 輪詢

- 只拿目前分頁要的資料：Test Result → `home`；模式分頁 → 自己的引擎；Version → gpib＋rs232。
- 只在視窗開著時輪詢（HT_WIN，同 `HW.IoSetView.html:445-456`）：
  - 視窗縮小也算開著；
  - 沒收過 HT_WIN（頁面單獨開）就照舊一直拿；
  - 開窗那一下立刻拿一次；
  - `document.hidden` 時不拿。
- Demo（離線看版面）：Test Result 分頁可以選 TTL／GPIB／RS232／TCP/IP。資料是頁面裡的假資料，形狀和 JSON 一樣，行是 golden 格式的原文；Demo 時不會送出任何指令。

## 測試與上機

- ctest `TesterComm_HomeJson`（tests/test_testercomm_home.cpp）涵蓋：
  - testType 對應；
  - `state`；
  - `seq`（新增、尾端截斷、清掉、沒有變）；
  - 每個 testType（跑／沒跑）合成後都是合法 JSON，頂層 11 個欄位順序一樣，32 格各 9 個欄位，log 4 個欄位；
  - 行原文不變（含引號、反斜線）；
  - 路由 GET home，以及 POST home 的轉送（GPIB、TTL、TCP 只收 bin）。
- 網頁：`node --check`，另外有一份假 DOM 的 smoke test（St02-E 的 scratchpad，不進版控）。
- 上機（EastSun）：每種 Tester Type 各開一次視窗，確認：
  - Test Result 分頁只有一個 32 站面板和一個 MemoLog，格子是銀色；
  - 出貨版：上面的分頁只有 Test Result、那台機台的介面那一頁、Version（TCP/IP 機台沒有 Version）；模擬版全部都在；
  - Log 標題顯示對的 `[tag] from`，行和 golden 通訊視窗一模一樣；
  - TCP 模式勾選框是灰的，下拉可以改，SimulateBin 會照改的值回 bin；
  - 模式分頁沒有站台格子。
