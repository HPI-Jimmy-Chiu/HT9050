---
name: ht9045-gpib-bridge
description: HT9045 / HT9046LS 的 GPIB 橋接程式（H9046_32GPIB.exe，原始碼在 d:\GPIB9045）知識庫。此程式是「ATE 測試機（Tester / pgm / MES）」與「HT9045 Handler」之間的 GPIB ↔ WM_COPYDATA 橋接器，負責解析 Tester 送來的 GPIB 字串指令、轉成 MSG_CMD_* 送給 Handler、再把 Handler 回應原樣寫回 Tester。當使用者詢問 GPIB 指令、SETTEMP/SETTEMP?/SETSOAK/SETSITEMAP、H9046_32GPIB、TSerialPoll、MyGPIBWrite、SendMSG_CMD、SetParameter、ibwrt/ibrd、Tester 溫度比對失敗、Handler Temp=Settemp、Temperature check fail、溫度回應格式、各 Tester 廠牌（Advantest/Flex/93K/SPEA/RFMD/Qorvo/Delta Castle/DOOSAN/Novatek）指令差異時，應先載入此技能。關鍵字：GPIB, H9046_32GPIB, GPIB9045, SETTEMP, SETTEMP?, SETSOAK, SETSITEMAP, MyGPIBWrite, SendMSG_CMD, SetParameter, TSerialPoll, ibwrt, ibrd, MSG_CMD, Settemp +25.0, Temperature check fail, iI38SETTEMPRespondSetTemp, Tester, ATE, pgm, MES Temp。另涵蓋 GPIB 併入 V906 的轉換計畫（獨立執行緒 + 瀏覽器畫面）：GPIB 併入, GPIB in-process, GpibEngine, GpibThread, GpibIpc, SyncMailbox, IGpibDriver, gpib-32.dll, SimGpibDriver, gpib.html, gpib.* tag, TMyDutPanel, makeDutPanel, MY_DUT_PAL, OnMyCopyMsg 翻譯, GB 戰役。 另涵蓋 Handler 自己的 TCP 指令伺服器 7016／7017（golden TCPCommandServer／TeraTCPResultServer、HanderTcpIp、TCPCommandServerClientRead、HTGR／HTSET／HTSR 指令、V906 W10 的 CmdServerPump／TcpCmdFramer／polled TServerSocket）→ references/tcp-command-server-7016.md。
  V906 轉換計畫全文 → references/gpib-v906-integration-plan.md；P8 機邊 bring-up 步驟書（GPIB／RS232／TTL 上機順序、前置、log、上機前缺口 B1～B7）→ references/gb-p8-bringup-plan.md
---

# HT9045 GPIB 橋接程式（H9046_32GPIB）知識庫

## 1. 這支程式是什麼

- **輸出檔**：`H9046_32GPIB.exe`（BCB 線：與 HT9045.exe 是**兩支獨立程式**）。**V906（C++ 線）不是另一支 exe**：整支翻進 wb_serve 行程內的 `TesterComm/Gpib/`，跑在專屬通訊執行緒上（§8.6～8.7）
- **原始碼根目錄**：`d:\GPIB9045`（**不是** HT9045 版本目錄）
- **目前分析/可寫版本**：`d:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\`（20260926 起；883 版仍可供比對）
- **角色**：ATE 測試機（Tester / pgm / MES）⇄ **GPIB 橋接** ⇄ HT9045 Handler
- **編碼**：Big5（CP950），BCB6 / VCL，pre-C++11，與 HT9045 同樣限制
- 版本命名：`GPIB_Code_32Site_<Version>_<Date>[_<Owner>]`；其餘版本目錄、`back/`、`Backup/`、`H9046_32GPIB_V12*/` 一律唯讀僅供比對。
- **機型清單**（`D:\GPIB9045\system\general.ini [Version] Model`），三處要一起改：`Timer1Timer` 白名單（Main.cpp:630）、
  `ReadLastDataFile` 的 `MachineType`／`Application->Title` 分支（Main.cpp:4458）、`ProcessHMountConnect` 的 `FindWindow("TfMain", …)` 標題清單（Main.cpp:3091）。
  現有：9045GPIB／9046GPIB／9046_32GPIB／9045GPIB_12Site／2601GPIB／9055GPIB／502GPIB／7080GPIB／1032GPIB／
  **9050GPIB（Steven 20260926 新增；`MachineType=2` 比照 9046_32GPIB，因 F/B 各 2×8 吸嘴 = 32 站；視窗標題 `HT-9050`）**。
  `MachineType` 0/1/2 = `GpibString` 補位 2/4/8 個 hex（8/16/32 站，`InitialStartValue` Main.cpp:4780）。
  `SVON_CLOSE()` 內另有一份舊清單，但它沒有呼叫者（死碼），不用同步。
- GPIB 版本字串定義在 `MessageDef.cpp`（`GPIBVersion`）。

## 2. 兩條通訊鏈路

```
 [ATE Tester / pgm / MES]
        │  GPIB 字串指令 (ibwrt/ibrd)      ▲ MyGPIBWrite 原樣回寫
        ▼                                  │
 ┌──────────────────────────────────────────────┐
 │  H9046_32GPIB.exe  (TSerialPoll, Main.cpp)     │
 │  Timer1Timer 輪詢 → ProcessStatusString 派發    │
 └──────────────────────────────────────────────┘
        │  SendMSG_CMD → WM_COPYDATA       ▲ Handler 回 GHandler2Gpib->Message
        ▼  (GGpib2Handler)                 │  (ProcessAddress)
 [HT9045 Handler]  main.cpp:15369... + Command.cpp Write*Data()
```

| 方向 | 機制 | 關鍵函式 (Main.cpp) |
|------|------|------|
| Tester→Bridge 收指令 | GPIB `ibwrt/ibrd`，`Timer1Timer` 輪詢 | `ProcessStatusString()`（大型 if-else 指令派發鏈） |
| Bridge→Tester 回寫 | GPIB `ibwrt`（可選自動補 `\r\n`） | `MyGPIBWrite(str, Task)`（約 4588 行） |
| Bridge→Handler 送命令 | Windows `WM_COPYDATA` → `HMountWnd` | `SendMSG_CMD(CMD[, Message])`（約 4798 行），封包 `GGpib2Handler` |
| Handler→Bridge 收回應 | `WM_COPYDATA` 回呼 | `ProcessAddress()`（約 3400~3887），封包 `GHandler2Gpib` |

> **回應原樣轉發（重要）**：對查詢類指令，Bridge 在 `ProcessAddress()` 用
> `buffer.sprintf("%s", GHandler2Gpib->Message); MyGPIBWrite(buffer, sGbibTask);`
> （Main.cpp:3652-3653）把 **Handler 回的字串原封不動**寫回 Tester。
> ⇒ 回應字串的「內容/格式」由 **Handler** 決定，Bridge 不加料、不改格式。

## 3. GPIB 溫度相關指令（Tester → Bridge → Handler）

於 `ProcessStatusString()`（Main.cpp，約 5200~7300，含多套 Tester 廠牌分支）以
`Str.Pos("XXX")==1` 比對（**大小寫敏感**，呼叫前 `Str` 已 `strupr` 轉大寫）：

| Tester 送的 GPIB 字串 | 派送 MSG_CMD_* | 行為 | 位置 |
|------------------------|----------------|------|------|
| `SETTEMP?` | `MSG_CMD_HandlerTemperature` (65) | 查詢 Handler 溫度 | Main.cpp:5300, 7238 |
| `SETTEMP +<n>` / `SETTEMP_<n>` | `MSG_CMD_SetTemp` (73) | 設定 Handler 溫度，送 `TempStr` | Main.cpp:5305 |
| `SETTESTTEMP +<n>` | `MSG_CMD_SetTestTemp` (104) | 設定測試溫度 | Main.cpp:5310 |
| `DEVICETEMP` / `SETTESTOFFSET_` | `MSG_CMD_SetTJ` (75) | Tj/偏移（TSMC） | Main.cpp:5425 |
| `SETSOAK?` / `SETSOAK ` / `SETSOAK_` | `HandlerSoakTime`(64)/`SetSoakTime`(74) | 浸泡時間 | Main.cpp:5315 |
| `GETNOWALLTEMP?` | `MSG_CMD_GetNowAllTemp` (69) | 取全部溫度 | — |
| `SETPOINT?...ZONE` / `MASSTEMP?...ZONE` | `HandlerTemperature`/`ActualTemp` | Delta Castle 專用 | Main.cpp:7131 |

**指令清單 `slCmdList`**（Main.cpp:120~196）的註冊順序索引 = `MessageDef.cpp` 的 `MSG_CMD_*` 數值，兩者必須一一對應（如 `SetTemp`=73）。

### SetParameter() — 前綴剝離（Main.cpp:6358）
把 Tester 字串去掉指令前綴、存進 `TempStr` / `Sitemapstr` 成員：

| 前綴 | `Delete(1,N)` | 存入 |
|------|---------------|------|
| `SETTEMP +` | 9 | `TempStr` |
| `SETTEMP_` | 8 | `TempStr` |
| `SETSOAK_`/`SETSOAK` | 8/7 | `TempStr` |
| `SETSITEMAP_`/`SETSITEMAP ` | 11 | `Sitemapstr` |

⚠️ 注意：`SetParameter()` 內用 `AnsiString.Pos("SETTEMP +")` 屬**大小寫敏感**。
呼叫端（V12.13.905.0 Main.cpp:1581／1822／2032）傳進去的 `buffer` 在前一行已被 `strupr` **原地**轉成大寫，
所以 SetParameter 看到的通常是大寫字串；例外是 `SGSETUP_` 開頭（:1573，刻意保留小寫）。
case 600（:2765-2782）的 `S=buffer` 不轉大寫、也不呼叫 SetParameter，所以混合大小寫的指令
（`GetFFC?`、`GetTJFunction?`、`Contact Force?`、`SetSiteOnOff:`…）只有在 0600 才比得到。
（20260926 更正：舊版本節說「傳入原始大小寫」是錯的，V906 GpibCommands.cpp 翻譯時逐行核對 golden 查到。）

## 4. ⭐ 溫度比對失敗 / 「Handler Temp=Settemp +25.0」診斷

**典型客訴 log（由 Tester 端 pgm / MES 程式輸出，非 Handler、非 GPIB 程式）：**
```
pgm or MES Temp=25 , Handler Temp=Settemp +25.0
Temperature check fail, please check setting degree !!
```

**完整資料流與根因：**
1. Tester pgm 送 `SETTEMP?` 查 Handler 溫度。
2. GPIB Bridge → `MSG_CMD_HandlerTemperature` → Handler。
3. Handler 在 `TfMain::TempDataStrings()`（**HT9045** `Command.cpp:1533`）組回應字串，
   依 INI 旗標 `IniConfig.iI38SETTEMPRespondSetTemp` 決定格式：

   | 旗標值 | 回應字串 | 對應 Tester |
   |--------|----------|-------------|
   | `0`（預設/多數機台）| `+25.0` | 期望純數值的 Tester |
   | **`1`** | **`Settemp +25.0`** | kevin 20180308 特定客戶需求 |
   | `2` | `25`（整數）| DOOSAN TESNA（Steven 20250701）|

4. GPIB Bridge 把該字串**原樣**寫回 Tester（Main.cpp:3652）。
5. 若 Tester pgm 期望純數值卻收到 `Settemp +25.0` → 解析失敗 → `Temperature check fail`。

**根因＝ Handler 端設定旗標被設成 1，不是 GPIB 程式、也不是 Handler 程式 bug。**
這完全解釋「其他台不會有 Settemp 文字」（其他台 = 0）。

**修正（設定層，非改 code）：**
- 設定區段/鍵：HT9045 `[Tester] bI38SETTEMPRespondSetTemp`，把 `1` 改成 `0`（或依 Tester 需求改 `2`）。
  - 註冊：HT9045 `cConfiguration.cpp:2633`（預設 0）；實機值在 `config/config.ini` 的 `bI38SETTEMPRespondSetTemp=`。
- 也可透過 SECS **ECID 35548**「[I38] Format of SETTEMP?」設定（`SECSGEM/uHGemHT9045_EC.cpp:1433`）。
- 改完用 `SETTEMP?` 實測回應字串應為 `+25.0`（無 `Settemp` 前綴）。

> 排查順序：先確認回應字串是「Bridge 原樣轉發」→ 鎖定 Handler `TempDataStrings()` → 查 `bI38SETTEMPRespondSetTemp` 與 Tester 期望格式是否相符。GPIB 程式任何版本都**不會**自行加 `SETTEMP/Settemp` 前綴。

## 5. Tester 廠牌 / 客製旗標（cmydef.h, general.ini）

不同 Tester 走不同指令子集（`ProcessStatusString` 內分支或獨立處理函式）：
Advantest / Flex / 93K / SPEA（標準）、RFMD/Qorvo（`QRM?`/`QRA?`）、Delta Castle（`SETPOINT?`/`MASSTEMP?`）、
DOOSAN TESNA（`DUTCHK?`）、Ampere（`GetFFC?`）、Novatek（多筆查詢指令）。
影響行為的常見旗標：`i2DIDFormat`(eStandard/eAMD/eIntel)、`bRETURN_GPIB_VERSION`、`bGPIBWriteWithout_r_n`、`iTesterMode`(InterfaceType_*)、`bRunHANA_ART`。

## 6. 檔案清單（V12.13.883.0_20250915_Jimmy_20250924）

| 檔案 | 用途 |
|------|------|
| `H9046_32GPIB.cpp` | 進入點 WinMain、建立表單 |
| `H9046_32GPIB.bpr` | BCB6 專案檔 |
| `Main.cpp` / `Main.h` / `Main.dfm` | 核心 `TSerialPoll`：GPIB 輪詢、指令派發、Handler 通訊 |
| `MessageDef.cpp` / `.h` | `MSG_CMD_*` 常數與編號、版本字串 |
| `cmydef.cpp` / `.h` | 全域變數、客戶代碼、`LAST_GENERAL_SET` 設定、INI 讀寫 |
| `MyDutPanel.*` | 32 站點 UI 面板 |
| `DummyArt.*` | ART 模擬器（`bDummyART`）|
| `RS232.*` | RS232 / AMD 通訊面板（ATC 客戶）|
| `myTimer.*` | 計時器輔助 |
| `Decl-32.h` | NI GPIB 庫函式宣告（`ibwrt/ibrd/ibrsv/ibwait` 等）|

## 7. 跨技能連動

- Handler 端溫度設定/回報、`SetTemp`/`ChangeTempMode`：HT9045 `Command.cpp`、`csystem.cpp`、溫控模組。
- SECS 對應（ECID 35548 等）：載入 `ht9045-secsgem`。
- HANA / ART 自動化溫度比對（`Handler Temp : %f`，`HANA_ART.cpp`）與本案 Tester pgm 比對是**不同來源**，勿混淆。
- 併入 V906 的 C++ 工作（翻譯、CMake、WebBridge 接線）：交給 `ht9045-v906` 代理；web 畫面元件慣例：`ht9045-html-version`。

## 8. 併入 V906 的轉換計畫（GPIB／DIO／RS232／TCPIP 四種介面，獨立執行緒 + 瀏覽器畫面）

> 全文：`references/gpib-v906-integration-plan.md`（與 V906 樹 `docs/GPIB_20260926_INTEGRATION_PROPOSAL.md` 同步）。
> 使用者 20260926 裁決：**GPIB 必須是 wb_serve 行程內的獨立執行緒**、四種 Tester 介面一併整合、設定統一在 cTesterIF。狀態：P0～P7 都已落地（20260926 晚，見 **§8.7**），P6 為暫定子集＋Q2 (a)，只剩 P8 機邊 bring-up —— **以 §8.6／§8.7 為準**，§8.1～8.3 是當初的計畫文字。
> 另兩個子專案的 agent／skill 是本計畫的參考來源：`D:\GPIB9045\.github\agents\GPIB9045.agent.md`（含 `gpib-rs232-merge` 先行設計、`gpib-command-list`、`gpib-93k-art`、`gpib-hana`、`gpib-qrovo`）、
> `D:\RS232Standard\.github\agents\RS232Standard.agent.md`（含 `rs232-standard-interface`、`rs232-ttl-communication`）、`D:\.github\skills\gpib-ht9045-sync`（三邊定義同步）。

### 8.1 一句話

四種介面在 golden 是三支程式（`H9046_32GPIB.exe`、`RS232Standard.exe` 含 TTL 板模式）＋一個 Handler 內建模組（`TfTesterTCP`）＋一段 Handler 內直接讀 DIO 卡的分支，
各自存讀設定、各自畫面、同一套 `MessageDef.h`。併入後是一個 `TesterComm/` 層：GPIB 與 RS232 各一條專屬 `WbThread`（TCP 走 tick 泵、DIO 在 tick 內），
同一個 `SyncMailbox`（忠實模擬 `SendMessage(WM_COPYDATA)` 的同步＋雙向巢狀）、同一個 Handler 端 `OnMyCopyMsg`、設定只在 `TestIF_File`（cTesterIF）、
畫面 `testercomm.html` 四分頁共用 `makeDutPanel`。

### 8.2 分析時要記得的硬事實

| 事實 | 影響 |
|---|---|
| 橋接程式只用 9 個 NI 函式（`ibfind ibrsc ibpad ibtmo ibwait ibrd ibrsv ibwrt ibstop`），但 `BorlandC_gpib-32.obj` 是 OMF，MinGW 連不進 | 改 `LoadLibrary("gpib-32.dll")` 動態載入；`ibsta/iberr/ibcnt` 用 `ThreadIbsta()` 系列（per-thread，正好配獨立執行緒） |
| golden `SendMessage(WM_COPYDATA)` 是**同步**且允許雙向巢狀；橋接端 `OnMyCopyMsg` 內會再 `SendMSG_CMD` 回 Handler | 信箱必須做 pump-while-waiting，**不能**簡化成純非同步佇列 |
| `MyGPIBWrite` 最長 20×100 ms = 2 秒阻塞；`ProcessHMountConnect` 有 `SleepEx(1000)` | 全留在 GPIB 執行緒，不得碰 500 ms tick |
| V906 tag 只能由 tick 執行緒 publish | GPIB 執行緒只寫 `GpibSnapshot`（WbMutex），tick 在 `PublishExtraTags()` 複製後 stage |
| V906 現況：`SendMSG_CMD` 是空殼（`forms/fMain.cpp:446`）、`HGpib2Handler` 是 NULL（`MessageDef.cpp:267`）、golden `TfMain::OnMyCopyMsg` 1,830 行未翻、`GetTesterResult` 回 false | 這些是 P2 的工作，也是最大一塊 |
| 兩邊 `MessageDef.h` 的 VM/MV 欄位相容（V906 只多 CCD 結構）；`GPIBVersion` 橋接 `12.13.905.0` vs V906 `12.13.883` | P0 把 V906 升到 905 |
| `ProcessMessage()` 在 Main.cpp 內找不到呼叫者，是 `Unit2.cpp` 的 `TMyThread` 在叫 | 引擎迴圈以 Unit2 為準：`Sleep(1); ProcessAddress(); ProcessMessage();` |

### 8.3 階段

P0 契約凍結 → P1 引擎離線翻譯（`Gpib/GpibEngine`，SimGpibDriver 餵腳本當 ctest oracle）→ P2 Handler 端接線（翻 `OnMyCopyMsg`、解 stub、atester 解閘）
→ P3 執行緒＋信箱 → P4 `gpib.html`／`gpib.*` → P5 機邊 bring-up。約 8,000 行翻譯，建議開 GB 戰役讓夜間 loop 推 P1/P2。

### 8.4 畫面元件：TMyDutPanel 已有 HTML 模板（20260926）

- `D:\HT9045\web\page\hwidgets.js` → `HTWidgets.makeDutPanel({name,index,site,bin,on,ocr,bg,left,top})`，回傳元素帶 `setSite(t,color)/setBin/setOn/setOcr`。
- 模板頁 `IDE.WidgetTemplates.html` 第 14 節。**照執行期 `MyDutPanel.cpp` 畫、不照 dfm**：150×110、cbSiteOn 預設未勾、labOcr 空字串、
  Items `0~15/1..6/0..16/0..255`；排列 `Left=4+(index%8)*154`、`Top=(index/8)*110`（8×4）；母表單色 `8421440` → `#408080`。
- GroupBox 標題畫在自己範圍內（框線從字高一半起），因為縱向 pitch = 高度，凸出框外會蓋到上一列的 labOcr。

### 8.5 使用者裁決（20260926，全文見 reference §7）

1. **GPIB 的 IO 通訊必須即時；HTML 畫面可以有一點點延遲** → 同步信箱（pump-while-waiting）定案，畫面走快照＋tick publish。
2. **非 9045 機型分支全部都要進來**。
3. 補搬帳本 **OK**（`docs/GPIB_PORT_LEDGER.md`）。
4. **範圍擴大：GPIB、DIO、RS232、TCPIP 四種通訊一併整合**。DIO 用 RS232 版本，原始碼 `D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410`
   （`MainForm.cpp` 3,746 行；與 GPIB 端同一套 `MessageDef.h`、同一個 `MyDutPanel.cpp`、同樣 `WM_COPYDATA`）。**TMyDutPanel 與畫面可共用** → `makeDutPanel` 共用，`testercomm.html` 一頁多分頁。
5. **短期 BCB 版本會存在，但 BCB 接 BCB；C++ 版本串接 C++ 的 GPIB** → V906 不做外掛舊 exe 相容路徑；BCB 橋接 exe 那條線照舊維護（HT9050 已加）。

6. **設定統一在 cTesterIF**（原本每支程式各自存讀）→ `TestIF_File` 唯一來源，`Setup.ini`／`general.ini` 的 recipe 級欄位併入。
7. **DIO 直接讀卡分支（`atester.cpp` `TTL_MODE && TTL_CARD_TYPE<2`）新機台不適用**，C++ 線的 DIO = RS232Standard 的 TTL 板模式；該分支不翻。
8. **GPIB 內建的 RS232（`RS232.cpp`／`CommAMD`）是 AMD／ATC 輔助線**，與 RS232Standard 的測試工作不同，留在 `GpibEngine`。
9. **GPIB／RS232／TCPIP 共用一條執行緒**（生產中只用一種通訊）：`TesterCommThread` ＋ `TesterCommHub` 依 `iTestType` 只啟動一個引擎，`SyncMailbox` 一對。
10. **32 站 `makeDutPanel` 四種模式共用同一個**。
11. **通訊執行緒必須與機台執行緒分開、不能互相干擾**：三個接點（信箱／快照／指令 inbox）以外零共用狀態、雙向禁呼清單、tick 最多被擋一拍、分開的 watchdog 心跳、P3 故障注入驗收。

通知信規則：GitLab 推送後寄 Jimmy 的信，標題必須是 `[HT9045 Gitlab 推送通知]`。

### 8.6 實作現況（as-built，20260926）

> 權威：V906 樹 `docs/TESTERCOMM_PORT_LEDGER.md`。這裡只記分析時要知道的形狀。

- **一條通訊執行緒**（裁決 9）：`TesterComm/TesterCommThread`＋`TesterCommHub`，依 `TestIF_File.iTestType` 只起一個引擎；GPIB_MODE／TCP_IP_MODE→`gpibbridge::GpibEngine`（golden 在 TCP 模式也開 GPIB exe），RS232_MODE／TTL_MODE→`rs232std::Rs232Engine`（DIO＝TTL 板）。引擎自己關掉時 `Restart()`（＝golden WakeupGPIB 重開 exe）。
- **bridge 程式整支翻**，不是抽「非 UI 部分」：`TesterComm/Gpib/`（H9046_32GPIB V12.13.905.0）、`TesterComm/Rs232/`（RS232Standard Rev12.13.902.0），各自一個 namespace（golden 全域與 Handler 同名），元件用 vclcompat headless 替身，本體逐行照 golden（規則在各目錄 `TRANSLATION_RULES.md`）。
- **行程內替身**：`SendMessage(WM_COPYDATA)`→`SyncMailbox`（同步、等待中處理對方送來的）；`FindWindow`→代號（HMountWnd＝信箱、bridge＝表單指標，golden 的視窗身分檢查照舊）；`Close()`→旗標、最外層才 FormClose；COM／socket 收資料→排隊到通訊執行緒；每次啟動重設全域與函式內 static（golden 每次是新行程）；建表單照 VCL CreateForm（先指定全域指標、清零、再建構）。
- **NI-488.2**：`LoadLibrary("gpib-32.dll")` 12 個進入點；沒有驅動時每個 ib* 回 ERR，golden 自己記 "Error Open GPIB0"。`SimGpibDriver` 給 ctest。
- **Handler 端**：`TesterComm/Handler/THandlerTesterSide`（golden main.cpp 的 case WM_GPIB_Program、SendMSG_CMD、SendMSG_TestMode、RunTestProgram、CloseGpibProgram、ProcessHVisionConnect、WakeupGPIB）；V906 TfMain 門面已有的 109 個成員照 golden 寫 `fMain->X`，缺的放在這個類別。
- **TCP/IP**：沒有 bridge 程式，golden TfTesterTCP 是 Handler 表單 → `TesterComm/Tcp/TcpPump` 在 Handler 執行緒上跑 V906 既有的 `Interface/TesterTCP_Socket`。（這是 Handler 當 **client** 連 Tester；Handler 當 **server** 收 MES／AMR 指令的 7016／7017 是另一件事，見 §8.8。）
- **畫面**：`TesterComm/UiChannel`（引擎每 200 ms 發快照、頁面指令在通訊執行緒上套用並呼叫 golden 處理函式）＋`web/page/testercomm.html`（三分頁、32 站共用 makeDutPanel）；wb_serve 的 `/api/testercomm` 路由與 tick 呼叫由 `TesterCommWiring` 提供，已接進 wb_serve（P3 H1～H7，§8.7）。
- **golden 的坑**（翻譯時量到、照留並記帳）：RS232Standard 902 資料夾是 `#define DEBUG` 的除錯快照（開著就不存 BA 結果；翻譯預設關）；RS232 的 BARCODE?／GET2DID? 從不回答（0 起算取 1 起算的 AnsiString）；GPIB 16BinGS 的 BINON 永遠不過、256 bin 只看第一位數；兩塊 TTL 板寫錯 COM／欄位。
- **vclcompat 陷阱**（每個翻譯檔都要注意）：`X->Click()` 是空的（直接呼叫 OnClick）；`Strings[i]` 是代理物件（進 `sprintf` 前包 `AnsiString(...)`）；`AnsiString==0` 在 BCB6 是 =="0"；`AnsiString+=int` 在 BCB6 是十進位字（vclcompat 會變成原始 byte）；`TComboBox` 的 ItemIndex 與 Text 不連動。

### 8.7 落地狀態（20260926 晚，分支 `v906/steven-gpib-widget`；權威仍是 `docs/TESTERCOMM_PORT_LEDGER.md`）

- **已落地**（都未在 St02 編譯；P1 已在 St01／筆電建置通過）：
  - P0 骨架、P1 GPIB 引擎、P4 RS232Standard／TTL 引擎、P5 TCP/IP pump。
  - P2a Handler 端 `THandlerTesterSide`。
  - P2b 接線：fMain 門面經 `forms/fMain.h` 檔尾的安裝座 `W906_TesterForward` 轉過去（ht9045_forms 不能直接連 Tester 通訊的庫）；atester.cpp 四個 golden 區段換成活的翻譯。
  - P2c／P2d：Off-Line 規則、ChangeTesterConnect 本體。
  - P2e：網頁 On/Off-Line 按鈕 `act.main.testerConnect`。
  - P2f：golden Timer2Timer 的 Handler→bridge 設定同步 `SyncBridgeSettings`／`SendMessageToGpibProg`。
  - P3 H1～H7 接進 wb_serve：`/api/testercomm` 路由、Init／Tick／Poll／Shutdown、連結。
  - P7 `testercomm.html`（GPIB 分頁含 golden tsRS232 那組）；視窗登記在 `web/background.html` WINDOWS 表 id `testercomm`（:512，hidden＋lazy，工作列按鈕；golden＝H9046_32GPIB.exe 自己的視窗，20260927）。
  - P6 暫定子集（5B／Q3(1)A／4A／3A）＋Q2 (a)。
  - ESD G3（20260927）：ProcessHVisionConnect 照 golden `FindWindow("TfESDMain","ESD_Monitor")` 設 `fMain->HESDWnd`，所以 `SendCommand_ESD`（S121 Exit 的 ESD_SYSTEM_CLOSE 等）會送到已經在跑的 ESD 程式；不啟動程式。G5 ＝ B（RULINGS_20260927 #24，20260927 已做）：照 golden 翻 HT IonBar 上電序列，**拿掉 `WakeupESD()`**（不啟動 ESD_Program.exe）；ESD 程式手動開著時，約 50 秒後對感測器 on 的 IonBar 送 PowerOn。`WakeupESDdelay` 放在 THandlerTesterSide。沒在真機驗過（要真 ESD 程式＋IonBar 控制器）。
  - NUMCMP（20260927，NB2 R89）：BCB6 的「數字 != AnsiString」是 Variant 數值比較，移植樹是字串比較。`Rs232Support.cpp` `uSocketClient::MatchClientSetting` 改成 `ClientSocket->Port!=atof(GetSocketPort().c_str())`（"05000" 不再每拍重連；不用 ToDouble，非數字會丟例外）；`uSocketServer::MatchServerPort` :700 保持（只多寫一次同一個 int）。TesterComm 其他地方掃過沒有。
  - golden 行號（20260927）：規定對照 906_20260618；本 skill 與 TesterComm/Handler 的行號多半是 912（St02 這台只有加密 7z）。912↔906 本體稽核在 `docs/ST02_GOLDEN906_AUDIT.md`——RunTestProgram 的 P65 重測值、AMD 執行期判斷、Multi2D、NonTestToRBin 等是 912 行為，等 Jimmy／Steven 決定。
  - **⛔ 20261003 改成以 906 為準**（`HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261002.md` 第 20 條「只做 906、912 不當翻譯來源」＋第 23 條第 6 項「已在 main 的 912 內容逐件改回 906」）：上一條與 TESTERCOMM 帳本 0926 的「906 為底補 912、目標 912」作廢。本機對照用 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven`（0618 是加密 7z；Steven 0927 裁決先用 0625，main.cpp :28427 之前起 +78）。St02 的清理是四張 MR（都在筆電第 50 批、還沒進 main `577c41c5`）：
    - **改回 906**：!130 `aba800e9`（H-013 MachineStatus 的 `IsSafePLCIOInstall` 改回 906 `Enable_PLCSafety_IO`、拿掉 GPIB 遠端 START／STOP，見下一條）；!132 `b2ab72d1`（拿掉 `fSecsAlarm` 三個條件；Qorvo Tester Pause 蜂鳴改回 906 馬上響，906 main.cpp:15806）；!134 `ef721c80`（AMD 執行期條件改回 906＝只有 Delta Castle，`HandlerBridgeCtl.cpp:487`／`:543`／`:797`；拿掉 912 的 Multi2D `iStatus` 區塊）；!136 `02fe28f0`（GetTesterResult＝906 :1038-1058，拿掉 912 的 Barcode-CSV／WAR04217 與 Murata NonTestToRBin 紀錄；AOI with912 改回 906）。
    - **保留 V912**（Steven 1003 05:4x 常設規則「912 比較好就留 912、兩邊行號都註明」，註解寫「#20 exception (Steven 1003 standing rule)」；!136）：P65 ARM-QA 重測 0x42（`HandlerBridgeCtl.cpp:753`）、ChangeTesterConnect 的「Silent run mode change」紀錄、Data.Observer 事件記錄拆欄。這條常設規則跟第 20 條不同，在 NIGHT_REPORT §0 第 78 項等 Jimmy；他回之前**不做新的 912 搬移**。
    - **溫控（#20a）與 HANA（#20b）照 912**：OnGpibProgramMsg 的 Dynamic PID 收 ATC 3.6、G23 `ResetHandlerArmCache`（閘著）、W9 的溫度 offset 重載（St02 912 自查表把 W9 歸 #20a 邊界案例）；WakeupGPIB 的 HANA `PrepareHANARMSConnect`（G9）由 C10（MR !126，筆電第 49 批）解閘。
    - **還在等 Steven**（客戶專屬，經 ST01-M）：Qorvo 蜂鳴延遲、AMD 執行期條件、Murata NonTestToRBin 要不要依常設規則回到 912；程式現在是 906。逐項表在 `docs/ST02_GOLDEN906_AUDIT.md` 的「1002-1003 現況」。
  - **GPIB 遠端 START／STOP（`MSG_CMD_RemoteStart`＝204／`MSG_CMD_RemoteStop`＝205）不在 906**：是 912 才有的 JerryYang 20260828 Qualcomm 功能（912 main.cpp:16431-16472、MessageDef.h:231-232；golden 906 與 `D:\GPIB9045` 都沒有）。MR !130（`2298e664`）把 `HandlerGpibMsg.cpp` 的兩個分支拿掉、MessageDef 的兩行改成註解（行數不變）⇒ **204／205 沒有分支接、不回覆、只留通訊紀錄**（「沒寫到的指令＝不支援」）。`W906_REMOTE_START_WIRED`（0926 裁決 8＝B 的開關）跟著消失；`W906_RemoteRunStart` 保留（golden 906 HTSET,333 `Command.cpp:13381` 與 SECS RCMD REMOTE_START `uHGemHT9045.cpp:2321` 在用）；`START_SitesCensus` 的釘子在 !130 自己的基底上是 `34 32 2` → `33 31 2`（少一個 GPIB 遠端 START）；main 已有 St01 review6 的 `36 34 2`，筆電第 50 批合起來是 `35 33 2`（CHAT_JIMMY 1003 07:1x）。上機見人工審核 B30（`docs/handoff/ST02_HUMAN_REVIEW_20260930.md`，`v906/steven-handoff` 分支）。
  - 剩 P8 機邊 bring-up（NI 卡、真 COM、真 Tester）。
- **使用者裁決（除 §8.5 的 1～11）**：
  - **Off-Line**＝一律走 GPIB 引擎的 simulate，用 GPIB 的設定（`OffLineGpibWay()`／`EffectiveBridgeTestType()`）。RS232／TTL 配方 Off-Line 時不開 Tester／TTL 的 COM port；這一點跟 golden 不同。
  - **ChangeTesterConnect**：golden 本體翻在 `forms/fMain.cpp` 檔尾（906_0625_Steven main.cpp:12064-12257；912 :12581-12778 跟 906 相同，只多兩行「Silent run mode change」RecordProcess（CASE-FOREHOPE_NINGBO-20260920-001）——那兩行照 Steven 1003 常設規則保留 V912、標「#20 exception (Steven 1003 standing rule)」，MR !136 `02fe28f0`，人工審核 C10）。`W906_ChangeTesterConnect_Sim` 覆寫仍優先。D5（RTC 訊息＋重開）照裁決不做。**D1／D2／D3／D4／D6／D7 已做（20260928，St02-E；原本「先列待辦」，Steven 0926 20:37「可以做的就先做」）**：每一項的 golden 原文仍在 fMain.cpp 的 `#if 0` 裡當對照，旁邊一行同行呼叫 hook；安裝座全在 `LogObjects.cpp` 檔尾（宣告 `LogObjects.h`，ht9045_db 跟 ht9045_forms 一定一起連），所以 fMain.cpp 那一個 commit 可以單獨扣住或丟掉。本體：D4（強制回 Operator，906 :12104-12131）在 `WebLogin.cpp` `W906_WebLoginForceOperator`（靜態初始化時裝）；D1 權限（:12072-12074）、D2 機台有 IC 不准切換 MES1646（:12076-12087，`#ifndef SOFT_SIMULTE`：模擬組態永遠不擋）、D3 I27 手動整盤（:12093-12098）、D6 ON_LINE→2D_SORT（:12144-12152）、D7 ASM On-Line 那一支（:12212-12237，＋912 那行 Silent run mode RecordProcess，同 Off-Line 那支）在 `TesterComm/Handler/HandlerTesterConnect.cpp`，`W906_TesterConnectRulesInstall()` 由 `W906_TesterCommInit` 在 HT9045_TESTERCOMM=0 退出之前呼叫。沒裝（每支 ctest，除了 WebLogin_ForceOperator／TesterConnect_Rules）＝原本的行為。按鈕（act.main.testerConnect）本身先做 `fSecurity->Insufficient(8)`，所以 D1 對按鈕沒有新效果，只擋將來不帶 bRemote 的呼叫者；SECS 遠端 bRemote=true、Msg=false。
  - **RS232Standard 902 的 `#define DEBUG`**：翻譯預設關（那個資料夾是除錯快照）。
  - **P6**（Steven 20260927 全部裁決，W1～W6）：
    - 5B：RS232 新選項接在舊 index 後。
    - Q3(1)A（W3）：擋 Windows 不收的格式。
    - 4A（W5）：HANDLERID? 格式看 Handler 的 config。
    - 3A／Q1(a)：INPUTQTY 廠牌跟 Handler 目前的值，Qorvo 固定 1、DummyART 用自己學的。
    - **W1＝(b)**：Handler 的值以配方為準。`forms/fSCKART.cpp` 的 `AccessFile` 真的讀寫配方 Tester.Data `[AutoRetest] iTesterType`（只這一個鍵），
      LOTSTATUS→0／SRQMASK→1 照 golden 學到之後寫回。缺鍵＝1（93K，偏離 golden 的 0）。
      ⚠ 開機／換配方的讀取會把 `iTesterType=1` 寫進沒有這個鍵的配方。ctest `TesterComm_W1ArtBrand`。
      W1b：csystem.cpp 的 W7C1／W7C2 影子值不再有人讀，六處＋CheckNeedRT 都讀 `fSCKART->iTesterType`（test_w7_f2 PART C C7／C8）。
    - **W9＝A**：GPIB `SETTESTOFFSET_`／`DEVICETEMP`（MSG_CMD_SetTJ）與 SECS TEMP_OFFSET 的溫度 offset 照 golden 寫進配方 Temperature.Data（uTemp_Set.cpp S3 打開、SECS G34 打開），GPIB 寫成功後照 912 重載記憶體（屬溫控通訊，St02 912 自查表 `docs/handoff/ST02_912_AUDIT_20261002.md` §2.2 歸 RULINGS_20261002 第 20a 條、邊界案例，沒有改回）；會累加。ctest `TesterComm_W9RemoteTempOffset`；上機要看見 TESTERCOMM 帳本 W9 節。
    - Q2 (a)（已裁決）：GPIB 程式的額外 RS232 port 跟配方走，每份 GPIB 配方第一次從 Setup.ini 抄一次。
    - Q5（W6＝A）：**已做**（St02-E Q41 phase A，`v906/steven-q41-wip`）：`D:\HT9045\web\page\Setup.TesterIF.html:56` 三組 radio 補新選項；GPIB 模式露出 RS232 分頁、只有 pnlRS232 四個可改（`D:\HT9045\web\page\ht9045_testerif_c_wire.js` (7)，後端 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp` 檔尾 TIF_Q41_W6*）；W3 格式檢查也套 GPIB 模式（待確認）。同一批：TI-1 換 DIO 檔即時換 lstTTL 摘要（extra.dioPreview）、TI-3「Tester TCP」鈕開 testercomm 視窗 TCP/IP 分頁、TI-4／TI-5 存檔成功後跑 golden FormClose＋主畫面 sbTesterClick 尾段（S88 做法）。W4：以後另做轉檔工具。
  - 傳遞用 `TesterComm/HandlerSettings.h`（atomics，−1＝沒發布就照 golden），不動 MessageDef 封包。
- **On-Line 一送 SOT 就判測試逾時（WAR07352）**：
  - 原因是 `SetTestTimeOutTimer` 空殼，沒設的 `TQPF_Timer` 一開始就 Off()。已照 golden 912 atester.cpp:10920-10955 逐字翻，T16 同時放開。這支 906 與 912 相同（906_0625_Steven atester.cpp:10747-10782；`docs/ST02_GOLDEN906_AUDIT.md`「相同」清單），以 906 為準不用改程式、只是引用。
  - `atester_32Site.cpp:337-338` 原本是 static 替身，也換成 extern 真全域。
  - 新的 `h?TestTimeOutDelay` 類全域若只在 .cpp 定義、窄 header 沒宣告，要用 extern 取得，**不要再做 TU 內替身**。
- **匿名 namespace 裡的區域 extern（wb_serve :439／:2867／:6759 踩過）**：
  - 在 `namespace { ... }` 裡的函式內寫 `extern void X();`，宣告的是匿名 namespace 的 X，會連結不到真的全域 X。
  - 要在檔案層級（namespace 外）宣告，或放進 header。Jimmy 修過 H1b／H5 這兩處。
- **`HT9045_TESTERCOMM=0`**：
  - 效果：整個不裝（沒有安裝座、沒有引擎、Tick 空轉、頁面顯示離線）。
  - 用途：只給「不能看到 bridge」的 SIM 回歸。
  - **不要用在要跑有 IC 測試循環的 SIM 回歸**：GetTesterResult 在 GPIB／OFF_LINE／TCP 模式要 bridge 在，沒有 bridge 時照 golden 每 3 秒重試、不前進。
- **ctest**：
  - `TesterComm_IPC`／`_GPIB`／`_RS232`／`_Handler`（E2E 要 `HT9045_TESTERCOMM_E2E=1`）／`_TestTimeOutTimer`／`_P6GpibAux`。／`_TcpCmdFramer`／`_TcpCmdServer`／`_TcpCmdFieldsCensus`（W10，§8.8）。
  - 會寫正式路徑（D:\GPIBLOG、D:\RS232Log）的完整生命週期測試都要手動開。

### 8.8 Handler 的 TCP 指令伺服器 7016／7017（W10＝B，20260927，St02-E；分支 `v906/steven-w10-wip`）

- 全文：`D:\HT9045\.claude\skills\ht9045-gpib-bridge\references\tcp-command-server-7016.md`；權威帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md`「W10＝B」。
- golden：兩個 `TServerSocket`（main.h:121-122），`HanderTcpIp` 寫死 7016／7017，只在 `CosFunction.bEnableHandlerResultServer`（Greatek 956／TeraPower 967／TeraProbe 804）開；98 個 `HTGR,…`／`HTSET,…` 指令，回 `HTSR,…,`（無 CR/LF），廣播給所有 client。
- V906：`vclcompat/ServerSocket` 的 **POLLED 模式**（事件在 tick 執行緒上觸發，不開執行緒）＋`TesterComm/Tcp/CmdServerPump`（第一個 pass＝golden FormShow 的 HanderTcpIp；對話框等待中照收，R3）＋`TesterComm/Tcp/TcpCmdFramer`（R2：每連線緩衝、依 id 欄數切框、2048 上限整段丟不截斷、不回 NG）。
- HTSET,333／334 只經 `W906_RemoteRun`（fMain.h 檔尾，wb_serve 安裝）到 `StartFromWeb／PauseFromWeb`，不碰基底 `TfMain::Start`。
- 還沒開：701／S3（fSCKART 門面欄位）、7017 的 501 推播（GATE J）。已裁：354＝A 直接寫 Contact.Data（★W40，0928）；702 例外＝A 只記錄不回覆（★W41，0928）。
