# Tester 通訊（GPIB／DIO／RS232／TCPIP）移植帳本

計畫：`docs/GPIB_20260926_INTEGRATION_PROPOSAL.md`（skill `ht9045-gpib-bridge` §8）。本檔記錄翻譯基準、每個階段做了什麼、刻意偏離與待辦。

## 基準

| 來源 | 版本 | 用途 |
|---|---|---|
| GPIB 橋接程式 | `D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525`（H9046_32GPIB.exe） | GpibEngine（P1） |
| RS232／TTL 橋接程式 | `D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410`（RS232Standard.exe） | Rs232Engine（P4） |
| golden Handler | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy` | `OnMyCopyMsg`、atester 結果路徑、TesterTCP（P2、P5） |

BCB 線（BCB Handler ⇄ BCB 橋接 exe）照舊維護，與 C++ 線互不相接（裁決 5）。兩支橋接程式之後改版，都要回來這裡對一次差異。

## 裁決（使用者 20260926，全文在計畫 §7）

1 IO 通訊即時、畫面可略延遲；2 非 9045 機型全帶；3 補搬帳本；4 四種通訊一併整合、DIO 走 RS232Standard；5 BCB 接 BCB、C++ 接 C++；6 設定統一在 cTesterIF；7 `atester.cpp` 直接讀 DIO 卡的分支不翻；8 GPIB 內建的 RS232 是 AMD／ATC 輔助線，留在 GpibEngine；9 GPIB／RS232／TCPIP 共用一條執行緒；10 32 站 `makeDutPanel` 共用；11 通訊執行緒必須與機台執行緒分開、不能互相干擾。

## 階段紀錄

### P0 契約凍結＋骨架（20260926）

- `MessageDef.cpp`：`GPIBVersion`／`RS232Version` 由 `12.13.883` 改為 `12.13.905.0`（與 GPIB 橋接 V12.13.905.0 相同；`GPIBVersionCheck` 仍 12.13）。`MessageDef.h` 的封包結構三邊逐行相同（GPIB 與 RS232 的 `MessageDef.h` diff 為 0，V906 只多 CCD 結構），不動。全樹沒有其他地方比對舊字串。
- 新目錄 `TesterComm/`（只用 `WebBridge/Sync.h` 與 Win32，不連任何機台程式碼）：
  - `SyncMailbox`：`SendMessage(WM_COPYDATA)` 的行程內替身。送方阻塞到對方處理完；等待期間繼續處理送給自己的請求，所以雙向巢狀不會死結；處理函式只在擁有該邊的執行緒上跑。另加逾時（預設 5 秒）與 `Reset()`。
  - `TesterEngine`：引擎介面（`Start`／`RunOnce`／`OnHandlerMessage`／`Stop`／`IsUp`），以及對應 golden `TTL_MODE`／`GPIB_MODE`／`RS232_MODE`／`TCP_IP_MODE` 的列舉。
  - `TesterCommThread`：唯一的一條通訊執行緒（裁決 9）。迴圈是「處理信箱 → `RunOnce()` → 等信箱事件或 `RunOnce` 要的毫秒數」；引擎丟出的例外被接住並計數，執行緒繼續跑；心跳計數給 watchdog 用。
  - `TesterCommHub`：依 `iTestType` 只啟動一個引擎；換類型時照 golden `CloseGpibProgram()`→`RunTestProgram()` 的順序先停舊的（join）、清信箱、再起新的；沒註冊的類型就不起引擎。
- `CMakeLists.txt`：新 target `ht9045_testercomm`（尚未連進 wb_serve）。
- ctest `TesterComm_IPC`（`tests/test_testercomm_ipc.cpp`）：啟動、同步送、兩層雙向巢狀且順序與 SendMessage 相同、逾時後送方 250 ms 內放行且晚到的完成被釋放、引擎送回 Handler、兩種例外、執行緒身分隔離、換類型、關機。
- ⚠ **未編譯、未跑 ctest**：STEVEN-NB3 沒有 MinGW／CMake。請在有工具鏈的機器跑 `build.bat gate`，比對 ctest 失敗清單（常駐 5 個）加上 `TesterComm_IPC` 通過。

### P1 GPIB 引擎（20260926）

golden `H9046_32GPIB`（V12.13.905.0＋Steven 20260926 的 HT9050 四處）整支搬進 `TesterComm/Gpib/`，放在 `namespace gpibbridge`（橋接程式的全域變數與 Handler 同名，行程內會撞名）。函式本體逐行照 golden，只做 `TRANSLATION_RULES.md` 列的機械替換；8 個翻譯檔各自附「只有這些行和 golden 不同」的 diff 核對。

| 檔案 | golden 範圍 |
|---|---|
| `GpibBridge.h` | Main.h `TSerialPoll`（去 TForm）、cmydef.h、RS232.h／DummyArt.h 的類別；headless 元件替身（TALed、TStatusBar、TTimer、TMyDutPanel）；V906 管線成員 |
| `GpibDriver.h/.cpp` | NI-488.2：`gpib-32.dll` 執行期載入（`ibfindA`…`ThreadIbcnt` 12 個），`ibsta/iberr/ibcnt` 每次呼叫後更新；`SimGpibDriver` 給 ctest |
| `GpibGlobals.cpp` | cmydef.cpp 全部、MessageDef.cpp 的橋接端實例、Main.cpp 檔案層全域；`ResetBridgeGlobals()` |
| `GpibUi.cpp` | 建構子（建出全部元件並套 Main.dfm 預設值）、FormCreate／FormShow／FormClose、Timer1Timer、ReadLastDataFile／WriteLastDataFile、各小按鈕、TMyDutPanel |
| `GpibTestGpib.cpp` | TestGPIB（Main.cpp:1458-3080） |
| `GpibCore.cpp` | CheckGSBINONString、Check2Dsum、CheckBINONString、ProcessHMountConnect、SendCaptureFinish、ProcessMessage、ProcessAddress、MyGPIBWrite、InitialStartValue、SendMSG_CMD 四支、SVON_CLOSE（gate） |
| `GpibHandlerMsg.cpp` | 橋接端 OnMyCopyMsg（Main.cpp:3139-3917） |
| `GpibCommands.cpp` | DoOverDrive、DoRecontact、ProcessStatusString*、InitialBarcodeList、SetParameter（5004-6580） |
| `GpibCastleAmdHana.cpp` | AMD／ATC 輔助線（TimerTMode、QueryRDY、SendMode、CommAMDReceiveData）、Delta Castle、HANA ART |
| `GpibAux.cpp` | RS232.cpp（GPIB 內建的 AMD 輔助 RS232，**不是** RS232Standard）、DummyArt.cpp |
| `GpibEngine.h/.cpp` | 不是 golden：把上面這些跑在 TesterComm 執行緒上的轉接層 |

`GpibEngine` 取代 golden 由行程本身提供的東西：

| golden（自己一個行程） | 這裡（TesterComm 執行緒） |
|---|---|
| WinMain：CreateForm(SerialPoll)→OnCreate、CreateForm(fDummyART)、CreateForm(fRS232Main)、Run→OnShow | `Start()`：同樣順序 |
| TMyThread：`SleepEx(1)`＋`Synchronize(Process)` | `RunOnce()` 每次跑 ProcessAddress＋ProcessMessage（bEnableThread 且 InitialOK 時），回傳 1 ms |
| TTimer（Timer1 300 ms、TimerTMode 10 ms、DummyARTTimer1） | `RunOnce()` 依 Enabled／Interval 觸發；Enabled 由 false 變 true 時重新計時 |
| TComm 在表單執行緒觸發 OnReceiveData | COM 讀取執行緒只 `QueueRx()`，`DrainRx()` 在 TesterComm 執行緒呼叫 golden 處理函式（補一個 NUL，golden 用 strlen） |
| FindWindow("TfMain", …) → HMountWnd | HMountWnd＝信箱代號；OnMyCopyMsg 的視窗身分檢查照舊，P2 的 Handler 端要填 `GpibEngine::HandlerWndToken()`／`BridgeWndToken()` |
| SendMessage(WM_COPYDATA) 雙向 | SyncMailbox（同步、等待中繼續處理對方送來的） |
| Close() → OnClose → 行程結束 | `RequestClose()` 只立旗標；下一次 `RunOnce()` 最外層才跑 FormClose，引擎變 down，等 hub 停它 |
| Handler 常重開 exe（atester 逾時後 RunTestProgram、AMD 2DIDFormat、換測試類型） | 每次 `Start()` 先 `ResetBridgeGlobals()`（134 個全域回到 golden 初值）並 `++g_bridgeLife`；有函式內 static 的 golden 函式在開頭「re-arm」 |

ctest `TesterComm_GPIB`（`tests/test_testercomm_gpib.cpp`）：
- 預設（不寫磁碟）：整個橋接的連結檢查（取 `GpibEngine::Create` 位址就會把 10 個 .cpp 全部拉進連結，任何漏定義都會在這裡失敗）、NI 包裝層與 SimGpibDriver、TTimer 模擬、Check2Dsum（同時釘住 vclcompat `SubString(0,n)` 與 BCB6 相同）、引擎防呆。
- 完整生命週期（`HT9045_GPIB_FULL_TEST=1` 才跑）：模擬驅動下啟動 → Handler 端收到 `MSG_CMD_Version`（"V12.13.905.0"）與 `MSG_CMD_TesterMode` → Handler 送 `MSG_CMD_CloseGpib` → 引擎自己關；跑兩輪，第二輪要再收到 Version（驗證 re-arm）。要手動開是因為 golden 會寫固定路徑：`D:\GPIBLOG\Log`、`D:\gpib9045\system`、`D:\RS232Log`、`D:\RS232Standard\System\Setup.ini`，以及 `D:\GPIB9045\system\general.ini` 的缺鍵預設值；兩個 ini 路徑變數由 `GpibEngine::OverrideIniPaths` 指到 %TEMP%。

⚠ **未編譯、未跑 ctest**（STEVEN-NB3 沒有 MinGW／CMake）。

#### P1 刻意偏離 golden（逐項都在程式碼旁有 `//AI(W906-GB-P1)`）

| 位置 | 偏離 | 理由 |
|---|---|---|
| 全部 | `X->Click()` 改成直接呼叫 OnClick 處理函式 | vclcompat `Click()` 是空函式；golden 3 處（OnMyCopyMsg）＋17 處（DummyArt） |
| 全部 | `Strings[i]` 傳給 `%s` 前包成 `AnsiString(...)` | vclcompat 的 Strings[] 回傳代理物件，直接進可變參數是未定義行為 |
| OnMyCopyMsg、MyGPIBWrite | `x==0`／`Task!=NULL` 改成跟 `AnsiString(0)` 比 | BCB6 的 0 走 AnsiString(int)＝"0"；vclcompat 會挑 `const char*` 多載變成跟 "" 比。OnMyCopyMsg 那處不改會讓條碼 "0" 的站被判成 BARCODEREJECT |
| TestGPIB、MyGPIBWrite、Castle | 靜態 buffer 大小 `StrLength+1` | 讀滿 2560 bytes 後 `buffer[ibcnt]=0` 寫出界 |
| TestGPIB | `MY_DUT_PAL[i]` 改 `.at(i)` | golden 靠 catch(...) 吞掉 VCL 越界，std::vector 的 [] 越界不丟例外 |
| DoOverDrive | 先送再 delete pcp | golden 先 delete 再 SendMessage（用了已釋放的記憶體） |
| Castle／AMD、GpibAux | 固定大小字串複製加上界、未初始化區域變數給 0 | 行程內越界會寫壞 Handler 的記憶體；golden 正常輸入的結果不變 |
| GpibAux OpenTesterComm | vclcompat StartComm 開不了埠時會默默轉 SIM；偵測到就丟例外 | 還原 golden 的 "Connect : Fail"／bCommConnect=false |
| GpibGlobals VerInfo | 固定回 "V12.13.905.0"（golden 讀 exe 版本資源） | 行程內沒有橋接 exe；Handler 端 golden 本來就會去掉 V |
| SVON_CLOSE | 整段 `#if 0`，回 true | golden 整段本來就在 `/* */` 裡，Main.h 也沒宣告 |
| Castle | `SerialPoll->Height=…` 兩處 `#if 0` | 沒有 TForm |
| OnHandlerMessage | GHandler2Gpib 在巢狀呼叫後還原、最外層結束設 NULL | golden 事後懸空；golden 除 OnMyCopyMsg 外沒人讀它 |

#### P1 記下但照 golden 保留的疑似 bug（不改，給 Jimmy 決定）

- 16BinGS 的 BINON 永遠不會通過：`CheckGSBINONString` 用站號當 2 byte buffer 的索引（`cBuffer[i]`，本意是 `cBuffer[0]`）。✅ 使用者 20260926 裁決 A：照 golden（連同下一條 256 bin）。
- 256 bin 的 `CheckBINONString`：`CompBuffer` 重複 `cBuffer[i]` K 次而不是取 `cBuffer[i..i+K-1]`，實際只看每站第一位數；`if(H<0 && H>255)` 永遠不成立。
- `ProcessMessage` 的 `IsTest==false;` 是比較不是指定（沒作用）。
- OnMyCopyMsg 的獨立 `MSG_CMD_GetSiteOnOff` 分支永遠走不到（已在前面的大清單裡）。
- HANA ART 分支拿 `sSiteOnOff`／`iMacStateStrLength` 當暫存，之後 RS232 的 `CZ status?` 會讀 `cMachineStateDec[6]` 66 bytes。
- TestGPIB case 1／200／600 的 `strncpy(tempbuffer, buffer…)` 在 `buffer[ibcnt]=0` 之前，可能帶上一筆較長訊息的尾巴。
- Castle 的 READTEMP／READDAQ 格式字串尾巴多一個 `%`（無效的 printf 轉換），真機要看回給 Tester 的字串。
- HANA 下拉選單比對 "DUMMYTEST_START_SRQ"，但表裡的鍵是 "DUMMYTEST_START_SRQ0x42"，DummyTest 分支走不到。
- DummyART：HP93K 送 `INPUTQTY<count> <lotid>`，FLEX 送 `INPUTQTY<lotid> <count>`，順序相反。

#### P1 留給後面階段

- **P2**：Handler 端要填 MV 的 HandlerHwnd／GpibHwnd 代號，否則每個封包都會觸發「No handler window」自關。AMD 2DIDFormat 改變時 golden 靠 Handler 重開橋接，P2 要讓 hub 能重啟同一類型（目前 `SelectTestType` 同類型是 no-op）。
- **P7**：元件由 golden 本體直接寫、沒有上鎖；網頁快照改在 TesterComm 執行緒上每 N ms 複製一份（`uiMutex` 只保護那份複本），golden 本體不用改。vclcompat `TComboBox` 的 `ItemIndex` 與 `Text` 互不連動（VCL 會連動），網頁改下拉時兩個都要設。`mmoBINON` golden 從不清，快照要限行數。
- **P8**：NI 卡、真 COM、真 Tester；Castle READTEMP 回字串核對。

#### P1 補充（20260926，與 P4 一起推）

- **筆電 gate 3e8534c9 的結果**：`WebBridge/Sync.h:151` 的 `::Sleep` 模稜兩可（筆電已在 main `04a2c66f` 修）之外，11 個 TU 0 個編譯錯、連結 0 個 undefined reference；唯一的錯是 `gpibbridge::fRS232Main`／`fDummyART` 在 `GpibAux.cpp` 與 `GpibGlobals.cpp` 各定義一次 → 刪掉 GpibAux.cpp 那兩行。之後用腳本掃過兩個 bridge：命名空間層級沒有其他重複定義（同一支腳本對修正前的檔案抓得到這 2 個）。

- **建表單的順序改成跟 VCL 一模一樣**：VCL 的 `CreateForm` 是「配記憶體並清零 → 先把全域指標指過去 → 才跑建構子」，所以 golden 的建構子（以及它呼叫到的東西）看得到 `SerialPoll`／`fRS232Main`，建構子沒設的成員讀起來是 0。原本 `X = new T` 是建構完才指定。兩個引擎都改用 `VclCreateForm()`（RS232 翻譯員發現的）。
- `TesterCommHub::Restart()`：同一個測試類型再啟動一次。golden 的 Handler 在 bridge 自己關掉後（`MSG_CMD_CloseGpib`、AMD 2DIDFormat、Tester 逾時…）會等 `WakeupGPIBdelay` 再 `CreateProcess` 一次；`SelectTestType()` 同類型維持 no-op（正在啟動中的引擎也還不是 up）。
- 引擎每 200 ms 把畫面快照丟到 `UiChannel`（見 P7）。

### P4 RS232Standard／TTL 板引擎（20260926）

golden `RS232Standard`（Rev12.13.902.0）整支搬進 `TesterComm/Rs232/`，放在 `namespace rs232std`。**DIO＝TTL 板走這支**（裁決 4），一個引擎同時服務 `RS232_MODE` 與 `TTL_MODE`；golden 自己依 `Setup.ini [SystemSetup] iTesterMode` 決定 Standard／SLT／TTL。GPIB 裡那條 AMD 輔助 RS232 是另一回事，仍在 `gpibbridge`（裁決 8）。

| 檔案 | golden 範圍 |
|---|---|
| `Rs232Bridge.h` | MainForm.h `TfRS232Main`（去 TForm，314 個元件）、cmydef.h、MyDutPanel、MyStringList、uSocketServerClient；V906 管線成員；golden 編譯開關（見下） |
| `Rs232Globals.cpp` | cmydef.cpp 全部、MessageDef 的 RS232 端實例、MainForm.cpp 檔案層全域、WriteDataToFile；`ResetRs232Globals()`（89 個全域重設 81 個，其餘 8 個是 const 與引擎自己的） |
| `Rs232Ui.cpp` | ShowVersion、建構子（建出全部元件並套 MainForm.dfm）、FormCreate／FormDestroy／FormShow／FormClose、ShowInterface、TMyDutPanel |
| `Rs232HandlerMsg.cpp` | ProcessHandlerConnect、SendMSG_CMD ×2、SendResultFinish、OnMyCopyMsg |
| `Rs232Comm.cpp` | 三個 COM 收資料處理（Tester／TTL1／TTL2）、Timer1Timer、Open／Close／SendCommandToTester（含 TTL） |
| `Rs232Log.cpp` | ShowCommData*、Setup.ini 讀寫、Bin log、TTL 手動按鈕、InitialBarcodeList |
| `Rs232Parse.cpp` | GetAnalysisString、Del1stVec、DoRevCommand（Tester 指令派發）、ReceiveData_TCPIP |
| `Rs232Support.cpp` | RS232Standard 自己的 TMyStringList（不是 Handler 的 Public/MyStringList）、uSocketServer／uSocketClient（vclcompat TServerSocket／TClientSocket，預設 SIM） |
| `Rs232Engine.h/.cpp` | 轉接層（同 GpibEngine 的設計；Timer1 300 ms、COM／socket 收資料改排隊、10 ms 迴圈） |

**golden 編譯開關**：902 那個資料夾的 `MainForm.cpp:16` 是 `#define DEBUG`（沒有註解掉），之前每一版（854／873／874、Code32Bin）都是 `//#define DEBUG`。開著 DEBUG 時 golden **不會存 BA 的 bin 結果**（`:3467-3476` 在 `#ifndef DEBUG` 裡），也不理會 `MSG_CMD_CloseGpib`——那是除錯中的快照。`RS232STD_GOLDEN_DEBUG` 預設 0＝出貨組態；設 1 就是那個快照原樣。`SOFT_SIMULTE` 每一版都開著，維持開（`RS232STD_GOLDEN_SOFT_SIMULTE`；V906 的 MachineType.h 本來也定義它，header 最後用 `#undef` 讓設 0 真的有效）。✅ **使用者 20260926 裁決 A：照出貨版，關掉 DEBUG**（＝`RS232STD_GOLDEN_DEBUG` 預設 0，程式不用改）。

ctest `TesterComm_RS232`（`tests/test_testercomm_rs232.cpp`）：預設部分做整支連結檢查、`crc_chk`＝CRC-16/MODBUS 標準檢查值（"123456789"→0x4B37、{0x01}→0x807E）、MyDeCodeASCII、TTimer 模擬、引擎防呆；完整生命週期 `HT9045_RS232_FULL_TEST=1` 才跑（golden 會寫 D:\RS232Log、開 Setup.ini 裡的 COM；測試把 COM 名指到不存在的 COM250-252）。

⚠ **未編譯、未跑 ctest**。

#### P4 刻意偏離 golden（逐項都有 `//AI(W906-GB-P4)`）

| 位置 | 偏離 | 理由 |
|---|---|---|
| 多處 | `asTester+=iStart[j]` 等 `AnsiString+=int` 改成 `+=AnsiString(...)` | BCB6 只有 `+=(const AnsiString&)`，int 變十進位字；vclcompat 會挑 `+=(char)` 送出原始 byte，每個 `@WSOTS...` 框都會壞 |
| 收資料迴圈 ×2 | `while(!none \|\| iNowDeal>iMaxDeal)` 在下一個命令會是 "_None_" 時跳出 | 一次收到 11 個以上命令時 golden 會無窮迴圈（"_None_" 是不動點），還會每次漏一個 TStringList、最後 iNowDeal 溢位 |
| TTL 收資料 | 89 處固定位移讀取越界時回 0；"no SOT" 分支的負索引寫入只保留範圍內的 | 短封包時 golden 讀寫 vector／陣列之外（在 BCB6 是寫到 `GGpib2Handler.iCommand`） |
| BARCODE?／GET2DID? | ✅ **使用者 20260926 裁決 B：修成會回答，比照 GPIB 的通訊格式**。golden 從 i==0 取 `CommaText[i]`（BCB6 AnsiString 1 起算且有範圍檢查 → ERangeError → 從不回答，854／873／874 都一樣）；改成取第 1～Length 個字。內容＝GPIB 橋接的回覆：BARCODE? 回 `BARCODE:<清單>;`，GET2DID? 回「反序清單＋結尾逗號」（原本少那個逗號）；RS232 的 `[STX]…[ETX]` 外框與 ENQ／ACK 交握不變 | 使用者裁決 |
| 固定大小字串 | 9 個 `strcpy` 改有上界的複製（`cJamCode[5]`、`cSoakTime[5]`…）；送出時照 golden 的位元組 | golden 正常資料就會溢位（" 0110"＋'\0' 是 6 bytes），行程內會寫壞 Handler 的記憶體 |
| OpenTesterComm* | vclcompat StartComm 開不了會默默轉 SIM；偵測到就照 SPComm 丟例外 | 還原 golden 的 "Connect:FAIL" |
| Timer1Timer | `SendToBack`／`SetForegroundWindow(HMountWnd)` `#if 0` | 沒有視窗；HMountWnd 只是代號 |
| Rs232HandlerMsg | `HHandler2Gpib`（golden 從沒寫過的那份）改成檔內 static | 否則會綁到 Handler 的 `::HHandler2Gpib`，從 TesterComm 執行緒讀 Handler 狀態 |

#### P4 記下但照 golden 保留的疑似 bug（給 Jimmy 決定）

- 兩塊 TTL 板：板 2 的框寫到板 1 的 COM（`SendCommandToTester_TTL(Data,1)` 用 CommTester_TTL）；板 2 的 bin 存進板 1 的 `Result[0..3]`；`OpenTesterComm_TTL(1)` 名稱空時清的是 `bCommConnect[1]`。**真的兩板機台會受影響。** ✅ 使用者 20260926 裁決 A：先照 golden，等實機驗證再決定（P8）。
- CD、CN 永遠回 `[STX][ETX]`：OnMyCopyMsg 沒有 `MSG_CMD_HanderIDRS232`／`MSG_CMD_GetSiteOnOff` 分支，Handler 的回答落到 "Command is not supported!"。規格（`D:\RS232Standard\.github\skills\rs232-standard-interface\SKILL.md`）寫 CD＝回傳機台 ID、CN＝`[xx,yy,z]…` 的 Site／Channel／啟停。✅ 使用者 20260926 裁決 A：照 golden（空回覆）。
- CF 永遠回 "8"；多個分支把 ENQ 推進 SendData 前沒先清。
- `MSG_CMD_AbortTest` 的 STX 'A' 'B' ETX 放進 SendStartData 卻送 SendData，abort 框從沒送到 Tester。
- `delete List` 在三個提早 return 前被跳過（每個未知命令漏一個 TStringList）。
- ShowCommData_TTL 只有勾著 Show Log 或 Save Log 時才會偵測 TTL 板版本（`VERS`）。

### P2a Handler 端（20260926）

golden V912 `main.cpp` 裡跟 bridge 程式對話的 TfMain 程式碼，翻成 `TesterComm/Handler/` 的 `THandlerTesterSide`（全域命名空間，這是 Handler 程式碼）。

| 檔案 | golden 範圍 |
|---|---|
| `HandlerTesterSide.h/.cpp` | 類別宣告；V906 TfMain 門面缺的 golden 成員（bFind、HVisionWnd、WakeupGPIBdelay、lblGPIBWND、mmo1、tESDError、bAMDRs232ConnectError、bHasPin1Error、bReceivePPSELECT、bTriggerESC）；管線（SendToBridge、FindBridgeWindow、StartBridgeProgram、HandlerWndToken、Sink） |
| `HandlerGpibMsg.cpp` | OnMyCopyMsg 前段＋`case WM_GPIB_Program`（:16481-16530、:16536-17819）、ProcessARTMessage（:15974-16480）、ResetForESC（:8028-8038） |
| `HandlerBridgeCtl.cpp` | ProcessHVisionConnect（:18315-18493）、WakeupGPIB（:18499-18596）、SendMSG_CMD ×2＋DeviceMapSRQ（:18734-18859）、SendMSG_TestMode（:18866-18959）、RunTestProgram（:18961-19193）、CloseGpibProgram（:29134-29160）、InitialBarCodeList（:33010-33038） |

- **為什麼另開類別**：V906 的 TfMain 門面已經有這段程式用到的 134 個成員裡的 109 個（Command.cpp 的 Write*／Get* 回覆、TempDataStrings…），其餘約 20 個要加就得改 `forms/fMain.h`（不在認領範圍）。所以缺的放在 THandlerTesterSide，其他照 golden 寫成 `fMain->X`。
- **替換**：`SendMessage(HVisionWnd, WM_COPYDATA…)` → `SendToBridge`；找 bridge 視窗的 `FindWindow` → `FindBridgeWindow()`（跑著的引擎代號）；`CreateProcess(H9046_32GPIB.exe／RS232Standard.exe)` → `StartBridgeProgram()`（hub 選類型，引擎自己關掉就 `Restart()`）；`this->Handle` → `HandlerWndToken()`（＝引擎的 HMountWnd）。
- 回覆 bridge 時 `HHandler2Gpib.HandlerHwnd／GpibHwnd` 填這兩個代號，bridge 端 golden 的視窗身分檢查照舊成立（填錯 bridge 會自己關）。
- `HGpib2Handler` 在訊息處理期間指向封包；巢狀訊息結束後還原外層；最外層結束後指向「最後一包的持久複本」（golden 事後指向已釋放的 WM_COPYDATA 暫存區，而 Command.cpp 會讀 `HGpib2Handler->cReturn`，NULL 會當機）。
- **gate**（HandlerBridgeCtl.cpp 14 個）：SPEA Interface.exe、ESD、EventLogSaver／EventlogAnalyzer 的啟動與找視窗（都不是 tester bridge，各有自己的計畫）；`fConfiguration->cbI17`、`fAutomation->PrepareHANARMSConnect`、`fTesterTCP->AddTCPIPCommunicationLog／SendTCPIPCommand`（V906 以 `TesterTCPSocket_*` 自由函式實作，改接留給 P5 之後）、`bP65QAING`、`fSCKART->iLOTSTATUS_T`。
- **gate**（HandlerGpibMsg.cpp 26 組、42 個 `#if 0`；登記表在檔頭）：多數是 V906 門面還沒有的成員——fSCKART（iCurrentStatus、iLOTSTATUS_A/F/L、GetLotStatus）、fAGV（ATK AMR 兩個旗標）、fContact（TfContactShim 缺 DMC 七個欄位）、ATC_InterfaceForm（shim 只有 iATC_MODE_TYPE，READTJ／QUERYTJ／SET_SLOPE_OFFSET 等十餘處；**回覆照送，所以 Tester 會收到 SETTINGOK 但 ATC 其實沒被設定**——✅ 使用者 20260926 裁決 7＝B：改回「失敗」，見下方 P2c）、TfObserver 兩個版本面板、MSG_CMD_RemoteStart／Stop（V906 MessageDef 沒有）、hPauseAlarmDelay（V912 才有）。條件式短路的地方只 gate 那個運算元，其餘分支照 golden。
- **兩個要 Jimmy 決定的 gate**：G3 `WriteLastDataFile(false)`（golden :16065）是**安全 gate**，不是缺符號——它會寫固定的正式檔 `D:\HT9045\system\lastdata.dat`，Command.cpp 的同一個呼叫（S3）也 gate 著（✅ 使用者 20260926 裁決 6＝B：打開照 golden 寫，見 P2c）；G24 `asGPIBAutoHeightZPos*100`（:17567）是 AnsiString 乘 int，vclcompat 沒有這個運算子、BCB6 怎麼解也還沒確認；golden 從沒給這個變數值，所以 READZPOS 現在不回。
- 新定義的全域：`bGpibMode`（golden main.cpp:15693，V906 沒有全域版；之後翻 :22711-22717 的人要 extern 這個）、`bStartTestSD[4]`、`iStartTestSD_Task`。

#### P2a 記下的差異

1. `FTestIF->bIsResetRs232Standard`（設定改了要重開 RS232Standard 重讀）在類型沒變時，StartBridgeProgram 不會重啟；今天沒有人會設這個旗標（它的寫入端還 gate 著），P6 要補「強制重啟」。
2. `TTL_MODE` 且 `TTL_CARD_TYPE<2`：golden 啟動的是 GPIB exe（梯子的 else 臂）；這裡一律 RS232Standard（使用者裁決：新機台不用直讀卡分支）。
3. SPEA 模式 golden 會 WM_CLOSE 掉 GPIB／RS232 bridge，這裡沒有對應（SPEA 整條不在範圍）。
4. golden 找 bridge 視窗的梯子只列 7 個機型；HT9050 依裁決 25 解碼成 HT9046_LS，所以走得到 `9046_32GPIB` 那一臂。
5. **版本基準（✅ 使用者 20260926 14:3x 裁決：「912 是新版本，可以拿 906 的補充 912 的就好」＝以 906 的翻譯為底、補上 912 的新增，目標是 912）**：這三個檔翻自 **V912** main.cpp，但 V906 既有的 Command.cpp（這段程式要呼叫的 102 個 Write*／Get*／Set*）翻自 **golden 906**（`906.0_20260618`；比對用的是本機的 `906.0_20260625_Steven`，0618 的 7z 有密碼）。906→912 的差：`case WM_GPIB_Program` 約 8 行（ATC_TYPE_36、ResetHandlerArmCache、bPauseAlarmDelayActive）；ProcessARTMessage 多 51 行（Qorvo pause alarm 延遲、MSG_CMD_RemoteStart／Stop）；WakeupGPIB 多 HANA RMS；SendMSG_CMD×2／RunTestProgram 把 `#ifdef AMD_Version` 改成執行期 `i2DIDFormat==eAMD`、P65 QA 條件改寫；其餘函式相同。Command.cpp 164 個方法只有 4 個不同（MachineStatus、GetHandlerStatusByDll、RemoteControl、TCPCommandServerClientRead），這段只呼叫到 MachineStatus（差 2 行）。912 才有的部分用到 V906 沒有的符號，所以是 G9／G12／G15／G16／G22 這幾個 gate。建議照第 26 條「底層照 906」把這幾處改回 906 文字（gate 也跟著消失），912 的新增列入之後補搬。


#### P2a 的 912 補充清單（依上面第 5 點的裁決）

Handler 端三個檔已經是 912 的文字；要補的是它依賴、V906 還沒有的 912 新增：

| 912 新增 | 在哪裡 | 屬於 | 補完後拿掉的 gate |
|---|---|---|---|
| Tester Pause 逾時才響（RogerYang 20260626）：`bPauseAlarmDelayActive`、`hPauseAlarmDelay` | golden 912 cmydef.h:5006-5007／cmydef.cpp:5049-5050、ckernel.cpp:744-749／:2146-2147／:2160-2161、uLotInfo.cpp:12548 | 測試通訊。cmydef／fLotInfo 已補（P2c）；ckernel 延後（使用者 20260926：「加到代辦事項，目前不改沒關係」） | G22 已拿掉；G15 等 ckernel |
| Qualcomm 遠端 START／STOP（JerryYang 20260828）：`MSG_CMD_RemoteStart`＝204、`MSG_CMD_RemoteStop`＝205 | golden 912 MessageDef.h:231-232；Handler 端的分支還要 `fiosetview->fShow`（V906 的 `TfiosetviewShim`，atester_shims.h:355，沒有這個成員） | 測試通訊（MessageDef 已請認領）；GPIB 橋接 905 還不會送這兩個 | G16 |
| P65 QA 模式 `bP65QAING`（Ifor 20260407） | golden 912 cmydef.cpp:6017；設定它的在 atester.cpp:3830、aTester_Front／Rear、csystem.cpp:8778／12944 | 跟 P2b（atester 解閘）一起 | G12 |
| Command.cpp 4 個方法：3 個加 `fSecsAlarm->Visible`、MachineStatus 改 `IsSafePLCIOInstall()` | golden 912 Command.cpp | ✅ 使用者 20260926 裁決 11＝B：**測試通訊這邊一起補，先不做（待辦）**。S10F3 SECS 警報視窗（TSecsAlarmForm，V906 沒有）、安全 PLC 型別 `IsSafePLCIOInstall`（筆電第 20 條） | 無（這段只呼叫 MachineStatus，差 2 行） |
| HANA RMS 互鎖 `PrepareHANARMSConnect` | golden 912 Automation/automation.cpp:3051 | ✅ 使用者 20260926 裁決 11＝B：**測試通訊這邊一起補，先不做（待辦）**。912 整套 HANA RMS（連線、設定、A77），V906 完全沒有 | G9 保留（補完才拿掉） |

ctest `TesterComm_Handler`（`tests/test_testercomm_handler.cpp`）：預設部分——整個 Handler 端連結（RESCAN 機台庫）、身分代號＝hub 信箱、沒有 bridge 時 golden 的防呆、Sink 在本執行緒跑 TesterMode／Version 封包不丟例外；端到端（`HT9045_TESTERCOMM_E2E=1`）——真的 GpibEngine（模擬 NI 驅動）＋Handler 端同一個 hub：bridge 找到 Handler 送 Version／TesterMode、ProcessHVisionConnect 找到 bridge、SendMSG_TestMode 通過 bridge 的身分檢查（bridge 沒自己關）、CloseGpibProgram 把它關掉。測試用一個 TfMain 子類別把 `fMain->SendMSG_CMD` 轉給 `fTesterSide`（就是 P2b 要在 fMain.cpp 做的事）。

⚠ **未編譯**。

### P2b 接線（20260926，使用者裁決 C-1：「由 st02 直接進行這一部分的移植工作」）

**(a) TfMain 門面的安裝座**（`forms/fMain.h` 檔尾 `W906_TesterForward`，同檔既有的 `W906_ClarnDataBody`／`W906_StateRecordBody` 作法）：
- golden TfMain 的 `SendMSG_CMD`×2、`RunTestProgram`、`CloseGpibProgram`、`SendMSG_TestMode`、`WakeupGPIB`、`SendMSG_CMD_DeviceMapSRQ`，本體在 `THandlerTesterSide`（ht9045_testercomm_handler）。TfMain 的這幾個成員只經一張函式指標表轉過去：**ht9045_forms 不能直接連 Tester 通訊的庫**（很多 ctest 只連機台庫），直接呼叫 `fTesterSide` 會讓那些 ctest 連結失敗。
- `W906_TesterCommInit()` 填表、`W906_TesterCommShutdown()` 先清表再拆。沒裝（測試執行檔、wb_serve 的 H3 之前）＝原本的離線 no-op，`RunTestProgram` 回 false（golden `bFind==false` 的答案）。
- 所以 wb_serve 的 H7（`ht9045_testercomm_handler ht9045_testercomm` 放在 RESCAN 群組前面）P2b 之後也不用搬進群組：機台庫不會反過來參照這兩個庫。
- `forms/fMain.cpp`：:446-447 兩個空殼原行改成轉發（行號不動）；新的 5 個成員與表的定義加在檔尾。`forms/fMain.h`：類別裡多 5 行宣告、檔尾加表。
- St01 S86（`973f2540`，INSTALL_OCR≠0 在開機、換配方、開 OCR 頁送 EnableBarCode／DisableBarCode／EnablePin1Function／DisablePin1Function）：bridge 還沒找到時，單參數 `SendMSG_CMD` 照 golden `if(bFind==false) return`；雙參數版與 `SendMSG_CMD_DeviceMapSRQ` golden 本來就不查 bFind，但 hub 沒有引擎時 `SendToEngine` 直接回 `kNoReceiver`。兩種都不會等信箱逾時。ctest `TesterComm_Handler` 第 6 項驗這個（1 秒內返回、裝／卸表）。

**(b) atester.cpp 四個 golden 區段**：原本是「golden 原文 `#if 0`＋一個回預設值的空殼」，現在都換成**活的翻譯**（906 為底補 912）。每個函式的 banner 有自己的 gate 登記。

| 函式 | 範圍（splice 前行號） | golden 912 | gate | 912 補的 |
|---|---|---|---|---|
| GetTesterResult | :806-2548 → 1961 行 | :849 起 | T01～T20（T10 已解，見下） | Murata record、Barcode CSV Compare、HANA RMS 互鎖（:1048／:1052-1067／:1396-1398） |
| ProcessTestResult | :2596-3581 → 1107 行 | :2641-3599 | R01～R10 | 無（906＝912）；補 golden 全域 `ProcessTestResultDelay` |
| ProcessTesterTimeOut | :3583-3865 → 336 行 | :3614-3879 | O01～O08 | [P65] QA 旗標在 Skip 時清掉（:3826-3835） |
| DoIndexSocketCheck（連 InitialIndexSocketCheckTask） | :11050-11209 → 181 行 | :11330 起 | S01～S04（COM2 視覺四個呼叫） | 無；補 golden 全域 `ESCDelay`、`iESCError` |

**T10**：golden 的 `fMain->bFind` 經安裝座多一格 `BridgeFound` 讀 `THandlerTesterSide::bFind`，由 `forms/fMain.h` 的 `W906_TesterBridgeFound()` 提供。沒裝＝false，走 golden「bridge 沒找到」那條路（TestTimeOut 3 秒、Task=9999），golden 邏輯不變。

**SAFETY gate**（會寫正式檔或會誤動作的，一律不放）：
- R01 `SaveASECLNewTestLogInfo`、R10 `WhenTestRecordTemperatureLog_3Sigma`：寫 log 檔。
- O07：Contact.Data 備份＋改寫正式配方檔。
- ~~T16 `if(TestTimeOut.Off())`~~：**已放開（20260926 晚）**，見下面「翻譯缺口：SetTestTimeOutTimer」。

**執行期行為的改變（要告訴代編／回歸的人）**：
- 跟 splice 之前比：
  - ProcessTestResult 以前永遠回 0，現在照 golden 回 0／1／2。等結果中回 0；沒有待測或 bNeedReplunge_RFMD 回 1；介面錯誤時預設 K_RETRY 回 2。
  - ProcessTesterTimeOut 以前永遠回 0（Skip）。現在沒人按時 ShowErrorMessage 預設 K_RETRY，On-Line 逾時會回 **2（Retry）**。沒有操作員時會一直重試，這跟 golden 等人按是同一個選擇點。
- Off-Line 的測試循環現在走 golden 的 tester 路徑：GetTesterResult 在 `GPIB_MODE || OFF_LINE || TCP_IP_MODE` 時要 bridge 在。
  - 預設（P3 已接）＝GPIB 引擎的 simulate 回 bin，跟 golden 靠 H9046_32GPIB.exe 的 simulate 一樣。
  - `HT9045_TESTERCOMM=0` 時沒有 bridge，照 golden 每 3 秒重試、不前進。SIM 回歸如果要跑有 IC 的測試循環，不要設這個開關。
  - 沒有真 IC 時（HasRealIC()==false），呼叫端照舊強制 ret=1，跟以前一樣。
- `TTL_CARD_TYPE<2` 的直讀卡分支照使用者裁決關著（T17），那種機台會停在 Task=100、不報警。

**翻譯缺口：SetTestTimeOutTimer（20260926 晚，github-59 查 BCB 發現，On-Line 的真 bug）**
- 原本 `atester.cpp` 的 `SetTestTimeOutTimer` 是 `#if 0 // TODO(W7)` 空殼，三個計時器都沒設。
  - 但 `aTester_Front.cpp:4239/:4268`、`aTester_Rear.cpp:4090/:4120` 的 `LastSet.iTester==ON_LINE && h?TestTimeOutDelay.Off()` 是 live。
  - 沒設的 TQPF_Timer 一開始就是 Off()（`myTimer.cpp:14-17/:40-44`）。
  - ⇒ **On-Line 每次送 SOT 後立刻判測試逾時**（WAR07352）。Off-Line 不受影響（有 ON_LINE 條件），FullSite echo 那一臂除外。
- 修法：照 golden 912 atester.cpp:10920-10955 逐字翻（906 :10747-10782 同文），逐行比對 0 差異。
  - `hFTestTimeOutDelay`（Index 0）／`hBTestTimeOutDelay`（Index 1）＝`TestIF.iMaxTime`，`bInitialMaxTime` 時用 `iInitialMaxTime`。
  - `TestTimeOut` 一律再設 MaxTime+10（外層的重送／等待）。
  - 兩個 per-arm 計時器 golden 經 `aTester_Front.h:19`／`aTester_Rear.h` 拿到；V906 的窄 header 沒宣告，所以在函式前加 `extern`。
- 同時放開 T16（golden :1832-1854）：
  - TTL 且 TTL_CARD_TYPE 2/3：I12 → Task 60（繼續等），否則 Task 55（重送 SOT）。
  - 其他介面：`bExist==false` → Task 55。
  - 這是 golden 行為，不是使用者選項；gate 的 SAFETY 理由隨計時器變真而消失。
- 呼叫點：atester.cpp 19 處、aTester_Front.cpp 5 處、aTester_Rear.cpp 5 處、atester_32Site.cpp 4 處，數目都跟 golden 一樣，全部 live。
- **已修（github-59 GO，S85 登記並通知 Jimmy）**：`atester_32Site.cpp:337-338`（Jimmy 的檔）用自己的 `static TQPF_Timer W5_32S_hFTestTimeOutDelay`＋`#define hFTestTimeOutDelay` 替身。
  - `SetTestTimeOutTimer` 設的是 aTester_Front.cpp 的真全域，設不到這個替身。
  - 32-site 雙臂流程（`DoTestSuckTestIC_TwoArm32Site`，atester.cpp:7957、csystem.cpp:32868 呼叫，live）在 On-Line 仍會立刻逾時（:3336／:3364）。
  - 修法兩行、不增減行：`static` 那行換成 `extern TQPF_Timer hFTestTimeOutDelay;`，`#define` 那行換成註解；:327-336 的原註解不動。
  - 檢查：檔內沒有別處用替身的真名；extern 型別與 aTester_Front.cpp:3085 相同（都在同一個庫 ht9045_sm、都不在 namespace 內）。
- ctest `TesterComm_TestTimeOutTimer`（`tests/test_set_test_timeout_timer.cpp`）：
  - 沒設的計時器立刻 Off()；
  - Index 0／1 設對的那個、另一個不動；
  - 過了 iMaxTime 才 Off()，TestTimeOut 還在（+10）；
  - bInitialMaxTime 用 iInitialMaxTime。
  - 用 QueryPerformanceCounter 的真時間（沒有假時鐘），50～600 ms，不碰磁碟。
- O07（逾時重置時改寫 Contact.Data）照舊關著，等使用者裁決。

**還沒放開、之後可以放的**：
- R06 `fMesSystem->AutoSiteMapPass`、R09 SECS `EventReport`：各需要 atester.cpp 多 include 一個標頭。
- R04 ATC `TestFinish`／`HandlerArm`：跟 G23 同一件，等 ATC。
- O03／O04 `bP65CanRunQA`／`bP65QAhasTouchDown`：V906 沒有。

⚠ 未編譯（這台沒有 MinGW）；每段都做了識別字對 include 範圍的解析檢查、兩個 SOFT_SIMULTE 組態的括號平衡、`#if` 平衡。


### P2d On-Line／Off-Line 切換 ChangeTesterConnect 與 Off-Line 的 bridge 規則（20260926，使用者裁決，github-59 轉達）

- **ChangeTesterConnect 本體**：golden 912 main.cpp:12581-12778 翻在 `forms/fMain.cpp` 檔尾。原本 :510 的替身行改成一行指向說明，不動行號。
  - `W906_ChangeTesterConnect_Sim != 0` 仍然先回，當成測試用的覆寫；0 才跑 golden。
  - 做：切到 Off-Line（:12671-12688）、直接給模式（:12690-12695）、Off-Line → On-Line 的基本路徑（ModifyTester(ON_LINE)＋MES2157）、收尾（SaveTestMode、MES2145/6/7、SCKART RunDummy、UpdateMainOperateMode、CloseGpibProgram、ASM 的 Off-Line 臂、LoadTestModePicture）。
  - 先不做（gate＋TODO「使用者裁決先不做」）：
    - D1 權限檢查
    - D2 機台內有 IC 就拒絕（MES1646）
    - D3 I27 Manual Sort：連條件一起關，所以 I27 機台直接上 On-Line
    - D4 SPIL 強制登出與 TemperatureEditDisable
    - D6 ON_LINE → 2D_SORT：條件留著、本體關掉，所以不換模式
    - D7 ASM 的 On-Line 臂：條件留著、本體關掉，所以不會掉進 Off-Line 臂
  - 偏離 golden（使用者裁決）：D5 RTC——config.ini 的 RTC Enable=0 照寫；不顯示「Program need to restart…」，也不設 restart 旗標。
  - 保留 golden :12583-12604 裡的 `if(SystemStart) return 1;` 與 `oldMode`：一個是執行中不准換模式的保護，一個是 ASM 收尾要用的舊值，都不是權限檢查。
- **Off-Line 的 bridge 規則**（`THandlerTesterSide::OffLineGpibWay()`／`EffectiveBridgeTestType()`）：
  - 不是 ON_LINE（跟 golden bSimulate 同一個判斷）又是 RS232／TTL 配方時，起 GPIB 引擎的 simulate，不開 Tester／TTL 的 COM port。
  - 封包照 GPIB 的欄位填：SendMSG_TestMode 走 GPIB 臂的 `GPIBBin=iGpibMode`；RunTestProgram 是 bSupport32Bin=true、GpibAddress=iGpibAddress、GPIBBin=iTestBinCount。
  - 用到的地方：ProcessHVisionConnect 的視窗梯子、WakeupGPIB 的啟動臂與找視窗、iRunning_Mode 的重啟判斷、StartBridgeProgram。
  - **只對 RS232／TTL 偏離 golden**：golden 在 Off-Line 仍跑 RS232Standard.exe 自己的 simulate，COM port 照開。GPIB 與 TCP/IP 配方在 golden 本來就走 GPIB bridge。
  - 切換的時機：ChangeTesterConnect 收尾的 CloseGpibProgram 關掉 bridge，下一次 ProcessHVisionConnect（1 秒內）依新的有效型別重起。
- 查證：
  - `MSG_CMD_State_TTL` 是 RS232Standard 開好 TTL 板後的通知，Handler 只回 GetTTLState（HandlerGpibMsg.cpp:2139），沒有流程在等它。所以 Off-Line 用 GPIB 引擎不會卡住。
  - Off-Line 的 TTL 配方測試走 GetTesterResult 的 `OFF_LINE` 臂，也就是 bridge simulate，不走 TTL 板那條路。
- ⚠ **20260926 晚上起的行為改變**：筆電的 OPMODE 波次（`7304dcef`，main `21323505`）把 `TfMain::UpdateMainOperateMode` 換成 golden 本體（main.cpp:12803-13127，`forms/fMain_OperateMode.cpp`）。
  - 呼叫路徑：forms/fMain.cpp:507 的 `W906_UpdateMainOperateModeHook`，wb_serve 在 InitialHandler 之前裝上。
  - ChangeTesterConnect 收尾照 golden 的順序呼叫它：SaveTestMode → 紀錄 → RunDummy → **UpdateMainOperateMode** → CloseGpibProgram → ASM → LoadTestModePicture（合併後核對過）。
  - 所以在 wb_serve 裡 On↔Off 切換會真的切加熱 relay、送 ATC 指令、寫 lastdata，跟 golden 一樣。
  - ctest 沒裝 hook，仍是計數空殼，所以不受影響。
- D4 的 `TemperatureEditDisable()` 現在是真的 TfMain 成員（OPMODE，golden :27047）。**不再被缺函式擋住**；依使用者裁決「先關著、標 TODO」仍然 gate，等使用者決定要不要接。
- W7 探針 `tests/test_w7_f1_wall2_probe.cpp:471`（使用者裁決 St02 改）：只改第一個檢查。改成 seam 為 0 時跑 golden、回 0、`LastSet.iTester` 讀回 ON_LINE，再用 ModifyTester 還原。

### P6 設定統一——暫定子集（20260926 晚，**暫照建議，待使用者確認**；使用者說明天直接看 todo.md）

使用者 C-2 的裁決加上 github-59 轉達的暫定範圍。全部可以退回；還沒回答的題目寫在 skill `ht9050-construction` 的 `todo.md`（備份在 `progress-st02.md`）。

- **5B**：RS232 新選項接在舊 index 後面，舊配方的值意義不變。
  - Bit Length：2＝5 bits、3＝6 bits
  - Stop Bit：2＝1.5
  - Parity：3＝Mark、4＝Space
  - 對照表：`TesterComm/Rs232SetupCodes.h`（header-only）。範圍外照 golden 的 else。
  - `tools/editlist/TestIF_File_TesterIF.py` 的 CheckRs232StandardIni 三段 REPLACE。golden 原文留在產生檔的 `#if 0`。
  - 產生檔重產：`FileRW/TestIF_File_TesterIF.gen.inc`（`--only`，跟 .py 一起 commit）。
    這台先用原本的 .py 重產一次，跟已 commit 的檔差異 0 行，證明這台的 golden 912 跟 St01 那台相同；之後才用新的 .py 重產。
  - 同時拿掉過期的 gate：golden :557 `fMain->CloseGpibProgram(__FUNC__)`。P2b(a) 之後它會轉到 bridge；RS232 參數一改，bridge 關掉再拉起來重讀 Setup.ini，跟 golden 一樣。
  - `FileRW/TestIF_File_TesterIF.cpp` 的 `FileRW_TesterIF_Boot`：在三組選項的 Items 後面接上新選項。網頁（`web/page/Setup.TesterIF.html:56`）**沒改**，因為 Q5 還沒回。
- **Q3(1) 暫定 A**：`SaveFlow` 在介面是 RS232 時，擋掉 Windows 不接受的組合（5 bits＋2 stop、6／7／8 bits＋1.5 stop），不寫檔。對照 `W906_Rs232FramingValid`。
- **4A（Q4 暫定 A）**：
  - `GpibUi.cpp` FormShow 不再讀 GPIB 自己 general.ini 的 `iUseGPIBFormat`，改用 Handler 的 `IniConfig.iI25UseGPIBFormat`；沒有 Handler 時照 golden 預設 0。golden 原文留在 `#if 0`。
  - `GpibCommands.cpp` 的 `HANDLERID?` 讀當下的值。
- **3A（Q1 暫定 (a)）**：`GpibCommands.cpp` 的 INPUTQTY 判斷改用 `W906_ArtTesterType()`。
  - Qorvo：固定 1，照 golden。
  - DummyART：用引擎自己學到的值。
  - 其他：Handler 目前的 `fSCKART->iTesterType`。
  - 沒有 Handler：golden 學到的值。
  - 引擎那邊 golden 的學習（L1～L4）原樣保留。
- **傳遞方式**：`TesterComm/HandlerSettings.h`（atomics，−1＝沒發布）。不動 MessageDef 的封包，三支程式的 MV／VM 版面照舊一致。
  - `THandlerTesterSide::PublishSettings()` 在三個時機發布：啟動 bridge 前、Handler 自己學到廠牌之後（HandlerGpibMsg.cpp LOTSTATUS／SRQMASK 那兩行）、每個 tick。
- **HOLD**（等使用者）：
  - ~~Q2~~：已裁決 (a)，見下面「P6 Q2 (a)」。
  - Q5：網頁。

#### P6 Q2 (a)：GPIB 程式的額外 RS232 port 跟配方走（20260926 晚，使用者裁決，github-59 轉達）

- 背景：golden H9046_32GPIB.exe 會另外開一個 COM port（RS232.cpp `TfRS232Main::CommTester`）。
  - 什麼時候開：客戶碼 0／860～863／943／985 預設開，或 GPIB 自己 general.ini `[OpenTesterComm] RS232=1`。
  - 格式讀 `D:\RS232Standard\System\Setup.ini [COMPort]`；Handler 只在 RS232 模式的配方存檔時寫這個檔（CheckRs232StandardIni）。
  - 所以 GPIB 模式配方的 `[RS-232C]` 從來沒被用過，常常是預設值。
- 2A＋Q2 (a)：
  - GPIB 模式的配方第一次跑時，把 Setup.ini 當下的值抄進配方一次；
  - 之後配方為準，GPIB 引擎的 port 用配方的 Baud／Bit Length／Stop Bit／Parity。
  - CommName、ReadIntervalTimeout 仍照 Setup.ini（不在配方裡）。
- 做法：
  - `TesterComm/Handler/HandlerGpibAux.cpp`（新檔）。
    - 適用條件：`TestIF.iTestType==GPIB_MODE`、bridge 走 GPIB 引擎、Handler 已初始化（InitialOK）、額外 port 有開；
      「有開」的判斷＝golden OpenTesterComm，只讀、不回寫預設值。
    - `W906_GpibAuxBeforeBridgeStart`（`StartBridgeProgram` 在引擎真的要（重）啟動前呼叫）：
      配方 `Tester.Data [RS-232C]` 沒有標記 `W906_SeededFromSetupIni=1` 就從 Setup.ini 抄一次
      （只寫 BaudRate／Bit Length／Stop Bit／Parity 四個 key，寫法跟 golden SaveSetupFile 一樣，再寫標記）。
    - 抄完的收尾照一般存檔：
      - 更新 `TestIF_File.Rs232_Data`；
      - 重新把配方值放進 TesterIF 頁的 widget（`FileRW_TesterIF_DoIniDataToForm`，經 hook）；不重放的話，下次在頁面存檔會把舊值寫回去；
      - `fMain->BackupSetupFile()`（配方備份＋MD5，St01 S92 進 main 後才是真的）；
      - `NewRecordProcess("", "TesterIF", ...)` 記舊值 → 新值。
    - 然後把配方的格式打包發布到 `HsGpibAuxFraming()`（HandlerSettings.h，一個 atomic，−1＝照 Setup.ini）。
  - 一份配方只抄一次：有標記就不抄。操作員之後存的值不會被蓋；golden 存檔是逐 key 寫，標記會留著。
  - Setup.ini 不在：抄引擎自己的預設值（9600／7／1／Even，就是 port 實際用的值）。
  - Setup.ini 的值不是 SPComm 的序號：該欄保留配方原值。
  - 對應表：`TesterComm/Rs232SetupCodes.h` 的反向函式（W906_Rs232BitLengthFromCode 等），含 5B 的新選項。
  - 引擎端 `GpibAux.cpp` LoadSetupData：開關 `W906_P6_GPIB_PORT_FROM_RECIPE` 改成 1。
    - 有發布就用配方的四個值，並把 GPIB 頁 RS232 分頁那四個下拉框設成不能改（改了也只撐到下次 bridge 啟動）；
    - Update（golden SetFormToData）照舊寫 Setup.ini。
  - 配方的 RS232 值之後有變（換配方，或存檔改了；機台停著、bridge 在）：
    - tick 的 `W906_GpibAuxNeedsRestart` 回 true，`CloseGpibProgram("P6 2A: ...")`，WakeupGPIB 重開；
    - 引擎重讀（同 golden CheckRs232StandardIni → CloseGpibProgram）。
    - 每次啟動只要求一次。
  - 開機順序：wb_serve 的 `W906_TesterCommInit`（:4388）在 InitialOK 之後，第一個 tick 在主迴圈，所以正常不會在配方讀進來前啟動。
    萬一 InitialOK 前就啟動了，先照 Setup.ini；第一個 InitialOK 的 tick 再判斷，配方的值跟 Setup.ini 不同才重開。
  - 只有 `W906_TesterCommInit`（wb_serve）會 `W906_GpibAuxEnable(true)`。
    ctest 直接用 THandlerTesterSide（例如 TesterComm_Handler 的 E2E）時什麼都不做，不讀也不寫正式配方、Setup.ini。
  - `web/page/testercomm.html` GPIB 分頁：加 golden Main.dfm tsRS232 那組。
    - 內容：Device、Baud Rate、Byte Size、Stop Bit、Parity、ReadIntervalTimeout、Update、COM Log（MemoLog）、連線狀態、「跟配方走」提示。
    - 快照多 `rs232Aux`，指令多這幾個欄位；停用的下拉框照 VCL 拒改。
    - golden 顯示這個分頁時才顯示。
- 不做／刻意不同：
  - 不發 SECS SaveRecipe：golden 只在操作員按存檔時發，這次不是操作員存的。
  - TCP/IP 模式的配方（也跑 GPIB 引擎）不套用，照 golden 讀 Setup.ini（裁決只講 GPIB 模式）。
  - RS232 模式配方在 Off-Line 走 GPIB 引擎時，也照 golden 讀 Setup.ini；這種配方存檔時 golden 已經把值寫進 Setup.ini，所以兩邊本來就一樣。
- 已知限制：
  - GPIB 模式時 Setup.TesterIF 頁不顯示 RS232 那組（golden 只顯示所選介面那一頁，Q5 未定）；
    要改 GPIB 配方的 RS232 值，現在得暫時切成 RS232、存檔、再切回來。
  - CloseGpibProgram 在 QA 模式的保護條件下會直接 return；那一次就不重開，要等下次 bridge 重啟才套用。
- ctest `TesterComm_P6GpibAux`（`tests/test_p6_gpib_aux.cpp`，檔案全在 `%TEMP%\ht9045_p6_gpib_aux`）：
  - 對應表兩個方向；打包值；port 開關（客戶碼預設＋general.ini）；
  - 種值：只抄一次、只寫四個 key＋標記、之後存的值不被蓋、Setup.ini 不在時用預設、非序號保留、沒配方檔回 −1；
  - 沒 Enable 時完全不動作。
- ⚠ 未編譯（這台沒有 MinGW）；網頁 `node --check` 通過，沒在瀏覽器看過。
- **不改**（照分析）：6A 的 GPIB 時間設定留在 GPIB 的 general.ini；1A／7B；ReadIntervalTimeout 系列、iRevCycleClear、bCheckClosedSiteHasBin。
- ⚠ 未編譯（這台沒有 MinGW）。

### P2f Handler → bridge 的設定同步（golden Timer2Timer；20260926，golden 原文，不需裁決）

- P6 分析時發現：golden `TfMain::Timer2Timer` main.cpp:22089-22234 與 `SendMessageToGpibProg`（:22667-22735）都沒移植。
  - 所以 `MSG_CMD_ChangeGpib` 從來沒送到 bridge。
  - GPIB 引擎的 `bSimulate` 只由這個指令設定（GpibHandlerMsg.cpp:238）。
  - 結果：P2b／P2d 的 Off-Line simulate、GPIB 位址、bin 數，在引擎那邊都還是它自己 general.ini 的值。
- 現在：
  - `THandlerTesterSide::SendMessageToGpibProg()` 與 `SyncBridgeSettings()` 放在 HandlerBridgeCtl.cpp 檔尾。
  - `W906_TesterCommTick` 在 `InitialOK && !SystemStart` 時每圈呼叫，跟 golden Timer2 的守衛一樣（:21495、:22080）。
  - 它是看變化才送：GPIB 位址、LastSet.iTester、bin 數、A10-6 有變，而且測區沒有 IC 時，依序送：
    - ChangeGpib
    - BarCode／Pin1
    - 2DIDFormat
    - FTP
  - 模式碼有變時送 SendMSG_TestMode，再從 bridge 的 ini 讀回（CheckAndReadIniData，golden）。
- gate：
  - F1 SPEA Interface.exe 那一段（plan: SPEA）。
  - F2 `hanaART->DoRunHanaART`：TfMainHanaART 門面沒有這個成員。
- 其他差異：
  - golden `fTesterTCP->OSRecipe` 改成函式內 static，因為 V906 沒有 fTesterTCP 表單。
  - CopyRecipeToTester／SendTCPIPCommand 改用 V906 的自由函式。
- 跟 P2d 的 Off-Line 規則一致：SendMessageToGpibProg 照 GPIB 欄位填；「模式碼有變」那幾臂在 Off-Line RS232／TTL 配方時走 GPIB 臂。
- 成員 `oldLastSetiTester`／`oldLastiTestBinCount`／`oldbA10_6`／`oldLastiTestMode` 照 golden 建構子初值（:2206-2211）。`oldGpibAddress` 沿用門面的 `fMain->oldGpibAddress`：ProcessHVisionConnect 找到 bridge 時設 -1，觸發重送，跟 golden 一樣。
- 筆電 21:3x（TO_STEVEN §4）回報：
  - `HandlerBridgeCtl.cpp:1133` `PageControl1Change(fShowBinSelect)` 編不過，main 已改成傳 `nullptr`（`04c72d84`）。
  - 觀察 ①：TCP/IP OS 測試機換工單那段有 4 個 `MySleep(100)`，主迴圈會停約 0.4 秒；golden 同，只在換工單那一次，不改。
  - 觀察 ②：`FTestIF->fShow==false`（TTL 卡 2/3 重送 TestMode 的條件）在網頁架構下永遠成立。
    已改讀 `W906_FormFShow("FTestIF", FTestIF->fShow==true)`（csystem.h:440，成員 OR 網頁視窗總表）。
    `FTestIF` 目前在 WebWindowRegistry.h 的「瀏覽器不會回報」名單裡，所以現在答案跟原本一樣；background.html 開始回報這個視窗後，才照 golden 的意思生效。
- ⚠ 未編譯。

### P2e 主畫面 Tester 按鈕的 C++ 接線（20260926，使用者裁決「那就幫我進行 C++ 的接線吧」）

- 動作 `act.main.testerConnect`：`JsonBridge/actions/MainTesterConnect.*`。golden 912 `TfMain::imgTesterClick`（main.cpp:29732-29794；906 :28766 起程式碼相同）照翻成 `W906_ImgTesterClick()`：
  - 流程：SystemStart／`fSecurity->Insufficient(8)`／CC_ASE_CL 確認框 → `fMain->ChangeTesterConnect(10)`。
  - 之後：bOffLineBin 時 SetRunStartMode＋fShowBinSelect、CC_KYEC_XILINX 時 fBarCode->ReadFile、P53 sStackBinTemp 清空、SECS `EventReport(SECS_EVENT.SwitchTesterMode)`。
- gate：
  - E1 `DoPassword()`：TfMain 門面沒有。只影響 CC_ASE_CL，當成密碼失敗、不切換（fail-safe）。
  - E2 `fLotInfo->labTCPIPSimulate`：TCP 那段的 timer／socket 開關由 P5 TcpPump 依同一條件自己做。
- 分派與清單：
  - `JsonBridge/ChanAction.cpp` 的 include、`HandleActionWithTag` 一行分派、拒絕訊息的動作清單、`ActionSchemaJson` 一行 schema。
  - `CMakeLists.txt` wb_serve 的 source 清單、`tests/CMakeLists.txt` test_sjson_chan 的 source 清單。
  - 呼叫的執行緒：wb_serve 主迴圈的 drain，也就是 tick 執行緒，等於 golden 的主執行緒。
- ⚠ `web/page/main-control.js` **沒動**：它現在還走已退場的 HTSimulatorBridge，改頁面要另外問使用者。
- ⚠ P2d 翻好之前 `ChangeTesterConnect` 是替身，所以按了模式不會變。回應帶 before／after／modeChanged 與說明。
- ctest `SjsonChan` 加一項：SystemStart 時回 `system-running`、模式不變。

### P3 接進 wb_serve（20260926）

- `TesterComm/Handler/TesterCommWiring.*`：`W906_TesterCommInit`（註冊引擎：GPIB_MODE 與 TCP_IP_MODE→GpibEngine（golden 在 TCP 模式也開 GPIB exe）、RS232_MODE 與 TTL_MODE→Rs232Engine；建 fTesterSide）、`W906_TesterCommTick`（每圈 PollHandler＋P5 pump；每 1000 ms ProcessHVisionConnect＝golden Timer2）、`W906_TesterCommPoll`（模態等待迴圈裡只 PollHandler：golden ShowModal 期間 WM_COPYDATA 照收）、`W906_TesterCommShutdown`、`W906_TesterCommHttp`（`GET/POST /api/testercomm/<key>?cmd=`；wb_serve 的路由 hook 拿不到 request body，所以指令放 query）。
- wb_serve 的 7 處（H1～H7）**已接**（St01 15:02／15:06 回「H1～H7 OK」；H5 在第 9 條進 main 後放行）。行號對 `39180290`（已含 main `1e15c4f9`），**全部不增減行**：
  - H1a :2836（原空行）＝`static bool g_tcAllowCmd`；H1b :2867 在 `return false;` 前接 `/api/testercomm`。
  - H2 :4351 `g_tcAllowCmd = allowCmd`；H3 :4388 `W906_TesterCommInit()`；H4 :4574 `W906_TesterCommTick()`。
  - H5 :537／:806／:6759：放在筆電的 `W906_ModalWaitTick(...)` 之後、`//` 之前，是 `W906_TesterCommPoll()`。
  - H6 :5967（`server.Stop();` 下一行的空行）＝`W906_TesterCommShutdown()`。
  - H7 `CMakeLists.txt` :3404 `target_link_libraries(wb_serve PRIVATE` 同一行加兩個庫，放在 RESCAN 群組前面。機台庫不會反過來參照它們（P2b 的安裝座），所以 P2b 之後也不用搬。
- ⚠ **接上之後的執行期行為（golden）**：開機後約 1 秒，Tick 裡的 ProcessHVisionConnect（golden Timer2）找不到 bridge 就 WakeupGPIB。所以 wb_serve 會跟 golden 開機拉起 H9046_32GPIB.exe／RS232Standard.exe 一樣，啟動 GPIB 或 RS232Standard 引擎：寫它自己的 golden log、開 GPIB 卡或 COM port；沒找到時每 10 秒重試。引擎的訊息框一律是 UiNotice（網頁），不會卡住機台執行緒。
- ⚠ **Jimmy 合 main 時修的兩個連結錯**（`79060249`）：H1b :2867 與 H5 的第三個 :6759 在 wb_serve.cpp 的**匿名 namespace** 裡。
  - 原因：區塊內的 `extern void W906_TesterComm…();` 因此宣告成匿名 namespace 的成員，連結時找不到。
  - Jimmy 的改法：:439 加檔案層宣告，:2867／:6759 改成 `::W906_TesterComm…()`，行數不變。
  - 以後在 wb_serve 加 hook，先確認那一行是不是在匿名 namespace 裡。
- `POST /api/testercomm/<key>?cmd=`（P7）有同鍵 400 ms 的重複視窗，數字跟 WebSocket 的同鍵保護一樣（WS 那道保護管不到這條 HTTP 路）。
  - 同一個 key、完全相同的 cmd，在 400 ms 內再來就丟掉，回 `{"queued":false,"reason":"repeat within 400 ms"}`。
  - 理由：`click btnManualStart`／`btManualTest`／`btnDiagZip` 與幾個切換鈕每按一次就動作一次；設定值的指令重複一次也沒差。
  - 位置：`UiChannel::Post(key, cmd, windowMs)`，預設 0＝不擋，只有 HTTP 那條傳 400。
- **退出開關**：環境變數 `HT9045_TESTERCOMM=0` 讓這一次執行完全不接：不裝安裝座、不起引擎，Tick／Poll／Shutdown 什麼都不做，頁面顯示離線。給 SIM 回歸測試用；預設照 golden。
- 已知限制：bridge→Handler 的延遲上限是 tick 迴圈的 50 ms 睡眠（它只在 cmdQueue 有東西時提早醒）；golden 是立刻處理。之後可以讓 hub 在 Handler 端有信時呼叫 wb_serve 給的喚醒函式。✅ 使用者 20260926 裁決 9＝A：接受 50 ms。（使用者問「有任何測試項目需要 html → C++ 嗎？」答：沒有。測試流程全部在 C++：Tester ⇄ 通訊執行緒 ⇄ Handler 執行緒；網頁只負責顯示與手動操作，這 50 ms 是 C++ 通訊執行緒交給 Handler tick 執行緒的等待，不經過網頁。）

### P5 TCP/IP 通道（20260926）

- TCP/IP 在 golden 沒有 bridge 程式：TfTesterTCP 是 Handler 的表單，socket 事件與兩個 timer 都在主執行緒，TimerProcessTCPDataTimer 直接寫 Handler 的測試結果。所以放在 Handler 執行緒（`TesterComm/Tcp/TcpPump.*`，由 `W906_TesterCommTick` 呼叫），不是通訊執行緒。
- V906 早就翻好 `Interface/TesterTCP_Socket.*`，但沒有任何正式程式呼叫 `TesterTCPSocket_Init()` 或驅動它的 timer，而且它的 OnRead 會在 socket shim 的讀取執行緒上直接改 Handler 狀態。pump 只呼叫它的公開函式、不改那個檔：把四個事件改成排隊、在 Handler 執行緒重播 golden 處理函式；兩個 timer 依 golden 的條件（`TestIF_File.iTestType==TCP_IP_MODE && LastSet.iTester==ON_LINE`，main.cpp:11320／cTesterIF.cpp:581-618／main.cpp:29770）開關，條件由真轉假時照 golden `Close()`；TimerTCPIPConnect 1000 ms、TimerProcessTCPData 1 ms（＝每圈）。
- 頁面的 TCP/IP 分頁吃 `UiChannel` 的 "tcpip" 快照（連線狀態、位址、32 站、收到的資料）。
- ⚠ **P8 前要處理**：vclcompat `TClientSocket` 在 REAL 模式是同步 connect（golden 的 VCL socket 是非阻塞）。今天沒有人呼叫 `SetSimMode(false)`，所以不會發生；轉真 socket 時 TimerTCPIPConnectTimer 的 `Open()` 要移出 Handler 執行緒，否則 Tester 不在線時會卡住機台執行緒（違反裁決 11）。✅ 使用者 20260926 裁決 10＝A：同意到時候（P8）再改。
- gate：`fLotInfo->labTCPIPSimulate`（V906 TfLotInfo 沒有這個成員）。

### P7 核心：畫面通道與頁面（20260926，第一段）

- `TesterComm/UiChannel.*`：引擎→頁面的快照（引擎在 TesterComm 執行緒上每 200 ms 把 golden 表單畫面整理成 JSON，`Publish`）、頁面→引擎的指令（`Post`，最多排 64 筆，引擎在 `RunOnce` 取出後設定元件再呼叫 golden 的 OnClick／OnChange）。golden 本體寫元件不上鎖（照 golden），只有這份複本上鎖。
- `Gpib/GpibUiSnapshot.*`、`Rs232/Rs232UiSnapshot.*`：各自的快照欄位與指令表（檔頭列全部）。
- `web/page/testercomm.html`（main 已把 `web/page/` 收進版控，所以直接放這裡；原本放在 web-overlay/，已移過來，免得有兩份）：GPIB／RS232・TTL／TCP/IP 三分頁；32 站共用 `HTWidgets.makeDutPanel`（裁決 10）；LED 用 `makeALed`；讀 `GET /api/testercomm/<key>`、送 `POST /api/testercomm/<key>`。wb_serve 還沒接這兩條路由時頁面顯示「離線」，可勾 Demo 看版面。
- **還沒做（要先認領 wb_serve.cpp，St01 也在改）**：wb_serve 加兩條路由與 Handler tick 呼叫 `TesterCommHub::Instance().PollHandler()`；`background.html` 註冊視窗。
  - 兩件後來都做了：wb_serve 是 P3 的 H1～H7；`background.html` WINDOWS 表 id `testercomm`（:512，hidden＋lazy）是 20260927（github-59 GO，FROM_STEVEN §1 `9dcf86c0`）。

### P2c 912 補充與裁決 6～8（20260926）

- 912 補充：`bPauseAlarmDelayActive`／`hPauseAlarmDelay`（cmydef、fLotInfo）、`bP65QAING`（cmydef）、`MSG_CMD_RemoteStart`＝204／`RemoteStop`＝205（MessageDef）、`TfiosetviewShim::fShow`（atester_shims）；拿掉 gate G12、G16、G22。
- ✅ **② ckernel.cpp 已做（20260926 晚上）**：使用者說「待辦裡面，你可以做的就先做吧」；github-59 確認沒人佔用後放行。
  - ckernel.cpp :1638（`RunState=LED_Message;` 那一行）接上 golden 912 :744-748：逾時才打開蜂鳴器，觸發一次就不再觸發。
  - :3374／:3382（FRONT 兩個 Alarm Reset 臂的 `bTesterPauseMusic=false;` 之後）接上 golden :2144-2148／:2158-2162：Reset 之後仍在 Pause 就重新計時。
  - 三處都寫在原本那一行，不動行號。REAR 那兩個臂不用改。
  - G15 同時打開：Qorvo 的 Tester Pause 照 912，等 Max Test Time 到了才響；沒設 MaxTime 時照舊馬上響。
- （原紀錄）**② ckernel.cpp 延後（使用者 20260926 15:xx 裁決：「加到代辦事項，目前不改沒關係」）**：golden 912 ckernel.cpp:744-749（逾時到了才把 `bTesterPauseMusic` 打開）與 :2144-2148／:2158-2162（Resume 時重新計時）先不搬。所以 **G15 維持關著**：Qorvo 的 Tester Pause 照 906（與 golden 沒設 MaxTime 時的舊行為）**馬上響**。只打開 G15 而不搬 ckernel 的話，計時到了沒人把蜂鳴器打開，會變成永遠不響，所以不能單獨打開。兩個全域先加（沒有人設它，無害）；G22（RESUME 時清旗標）打開，旗標永遠是 false，等於不做事。
- **裁決 6＝B（打開照 golden 寫）**：G3 拿掉，`MSG_CMD_SCKART_LOTRTCLEAR` 會照 golden 寫 `D:\HT9045\system\lastdata.dat`。ctest 不會送這個指令。
- **裁決 7＝B（ATC 接好之前改回「失敗」）**：`HandlerGpibMsg.cpp` 開關 `W906_ATC_PORTED`＝0。SET_SLOPE_OFFSET 一律回 `SETTINGNG`（解析照 golden 跑）；RESET_ATCALARM 只在 golden 會動到 ATC 的時候（`ATC_SYSTEM==eNewATCSystem` 且 `bATCActiveCooling`）回 `SETTINGNG`，其他機台照 golden 回 `SETTINGOK`（那個回答本來就不代表 ATC）。另外 5 個被 gate 的設定指令（SET_ATCCONTROLMODE、SET_ATC_TEMP、SET_WATER_VALVE、SET_DYNAMIC_PID、ASIF_TJ_EFUSED）golden 的 Handler 本來就不回，沒有「成功」可以改。
- **裁決 8＝B（真的啟動接好之前強制回「失敗」）**：開關 `W906_REMOTE_START_WIRED`＝0。遠端 START 的成功臂改回 `SETTINGNG`，而且不動 `bSECSGEMAlarm`／`bHasSaveSet`（被拒絕的 START 什麼都不改，跟「有表單開著」那一臂一樣）。STOP 照 golden。
- ⚠ **未編譯**（這台沒有 MinGW）；只做了 `#if`／大括號平衡檢查。

### 待辦（使用者 20260926 裁決後加的）

| 項目 | 狀態 | 負責 | 為什麼還沒做／前提 | 出處 |
|---|---|---|---|---|
| ~~② ckernel.cpp：golden 912 :744-749、:2144-2148／:2158-2162（Qorvo Tester Pause 等 MaxTestTime 到了才響，RogerYang 20260626），搬完同時打開 G15~~ ✅ 20260926 晚上已做 | `IMPLEMENTED` | St02 | 使用者：「加到代辦事項，目前不改沒關係」；diff 草稿在 FROM_STEVEN §3 14:29 | 使用者裁決 20260926 15:xx；帳本 P2c |
| ATC 接好後把 `W906_ATC_PORTED` 改 1（SET_SLOPE_OFFSET／RESET_ATCALARM 回 golden 的 SETTINGOK），同時拿掉 G23 | `BLOCKED` | St02 | 要等 ATC（`ATC_InterfaceForm` 真的類別，fATCHandlerSide.h:709）移進 V906 並有全域 | 裁決 7；HandlerGpibMsg.cpp G23 |
| 真的啟動接好後把 `W906_REMOTE_START_WIRED` 改 1（遠端 START 回 golden） | `BLOCKED` | St02 | `TfMain::Start` 還是離線 no-op（forms/fMain.cpp:411） | 裁決 8；HandlerGpibMsg.cpp G16 |
| 912 補充：S10F3 SECS 警報視窗 `fSecsAlarm`（TSecsAlarmForm）、安全 PLC `IsSafePLCIOInstall`（MachineStatus）、HANA RMS `PrepareHANARMSConnect`（拿掉 G9） | `NOT_STARTED` | St02 | 使用者：「測試通訊這邊一起補，先不做」 | 裁決 11；上方「P2a 的 912 補充清單」 |
| TCP 真 socket 的 connect 移出 Handler 執行緒 | `NOT_STARTED` | St02 | P8（接實機）時做 | 裁決 10；P5 |
| 兩塊 TTL 板的行為、16BinGS／256 bin 判定 | `NOT_STARTED` | St02 | 照 golden，等實機驗證再決定 | 裁決 3、4；P8 |

### ESD G3 拿掉（20260927，St02；github-59 轉 St01 §4 23:55 的待辦）

- 問題：St01 的 S121 Exit（`a684f171`，FileRW/MainClose.cpp）照 golden 送 `SendCommand_ESD(ESD_SYSTEM_CLOSE)`，但 `Interface/InterfaceSYS.cpp`
  `_SendStructMessage_Send` 在 `fMain->HESDWnd==NULL` 時直接 return，而唯一設它的地方是本檔的 G3（`#if 0`）。所以 V906 的
  `SendCommand_ESD` 全部沒送出去：S121、WebStart.cpp（SYSTEM_START／STOP、溫度、On/Off-Line）、csystem.cpp、TempCtrl/TriTemp.cpp、SECS。
- 做法：`HandlerBridgeCtl.cpp` ProcessHVisionConnect 的 G3 照 golden main.cpp:18329 打開，`HESDWnd` 是 golden TfMain 成員 → `fMain->HESDWnd`
  （forms/fMain.h:251，規則 2）。每 1000 ms 跟著 Timer2 找一次（TesterCommWiring.cpp:132）。
  - **只有 FindWindow，不啟動任何程式**：已經在跑的 ESD_Monitor（`TfESDMain`／"ESD_Monitor"）才收得到；沒在跑就跟以前一樣什麼都不送。
  - 收的那一邊（ESD 程式回 Handler 的 WM_COPYDATA、`RunInfo.ESDSoftwareVersion` 等）不在這次範圍。
- **G5 保持 `#if 0`**：第一臂 `WakeupESD()` 會 ShellExecute `D:\ESD_Program\EXE\ESD_Program.exe`（golden main.cpp:18690-18732），
  跟 G8 一樣是「Handler 啟動外部程式」，歸同一個使用者裁決（#31 那一類，選項已交 github-59）。第二臂（HT IonBar 上電序列）
  不啟動程式，但它的狀態是第一臂設的，兩臂一起等裁決。
- ctest `TesterComm_Handler` 第 7 部分：沒有 ESD 視窗 → `HESDWnd` NULL、ESD_SYSTEM_CLOSE 不送；測試自己註冊一個隱藏視窗
  （同 class／title）當替身 → 找到、收到 golden 的 M_V（TYPE_HANDLER_ESD／CommandType_ESD／ESD_SYSTEM_CLOSE）；USE_KASUGA 等旗標關掉就不送；
  視窗關掉 → 又是 NULL。PC 上真的有 ESD_Monitor 在跑時略過。
- ⚠ 未編譯（本機沒有編譯器）。

## 刻意偏離 golden（累積）

| 位置 | 偏離 | 理由 |
|---|---|---|
| `SyncMailbox` | 加了逾時與 `Reset()` | golden 的 SendMessage 會無限等；裁決 11 要求通訊端卡住時不能拖住機台執行緒 |
| `TesterCommThread` | 接住引擎例外並繼續跑 | 同上；golden 的 VCL 由 `Application->OnException` 接 |
| TcpEngine（P5 起） | 解碼結果經信箱交 tick 寫入 | golden 在主執行緒直接寫 `fMain->tTestResult`；一條通訊執行緒的規則要求改走信箱 |
| RS232 BARCODE?／GET2DID?（P4） | 會回答（GPIB 格式） | 使用者裁決 2＝B |
| SET_SLOPE_OFFSET／RESET_ATCALARM（P2c） | ATC 沒接好時回 SETTINGNG | 使用者裁決 7＝B；`W906_ATC_PORTED` |
| GPIB 遠端 START（P2c） | 真的啟動沒接好時回 SETTINGNG | 使用者裁決 8＝B；`W906_REMOTE_START_WIRED` |

## 待辦（P1 原始規劃，已完成）

- `GpibEngine`：`H9046_32GPIB` `Main.cpp` 的非 UI 部分去 VCL（`TestGPIB`、`ProcessStatusString*`、`SetParameter`、`MyGPIBWrite`、橋接端 `OnMyCopyMsg`、`ProcessAddress`、`ProcessMessage`），`static` 區域變數改成員。
- `IGpibDriver`：`LoadLibrary("gpib-32.dll")` 取 9 個 NI 函式與 `ThreadIbsta` 系列；`SimGpibDriver` 餵腳本給 ctest（腳本來源：`D:\GPIB9045\.github\skills\gpib-command-list`）。
