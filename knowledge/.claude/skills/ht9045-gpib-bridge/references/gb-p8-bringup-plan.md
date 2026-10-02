# GB P8 機邊 bring-up 步驟書（GPIB／RS232／TTL 板：V906 TesterComm 第一次接真設備）

> 本檔是 skill `ht9045-gpib-bridge` 的 reference，把計畫 `gpib-v906-integration-plan.md` §4 表格的 **P8 那一列**展開成上機步驟。
> 寫的人：St02-E helper，2026-09-28，分支 `v906/steven-p8-plan`（基底 `v906/steven-gpib-widget` `0c4b8e25`）。
> 狀態：**只有計畫**。寫這份時沒有執行任何程式、沒有開 GPIB 卡或 COM port；所有指令、回覆、鍵名都從程式碼或 golden 抄來並附行號，程式碼定不下來的地方寫「待上機確認」。
> 順序照計畫 P8：GPIB 查詢類 → SOT/EOT 一個 cycle → 32 站 BINON；RS232 同一個順序；TTL 板先 `WINIT` 再 `WSOTS`。授權照 BU 戰役三層級（skill `bu-wave-loop`）。
> 不在本檔：TCP/IP（Handler 當 client 連 Tester，P5）、Handler 自己的 TCP 指令伺服器 7016／7017（W10，見同目錄 `tcp-command-server-7016.md`）、HANA ART／DummyART／AMD 輔助 RS232 面板（計畫 P8 列的「補齊」項，另排）。

---

## 0. 引用的樹與縮寫

| 縮寫 | 絕對路徑 | 說明 |
|---|---|---|
| 【V】 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` | V906 樹。行號量自 `0c4b8e25`（量的地方是 worktree `D:\AI_TempFile\st02-p4`）；主 checkout 若已前進，行號可能漂移 |
| 【W】 | `D:\HT9045\web\` | 網頁 |
| 【G】 | `D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\` | GPIB 橋接 golden（H9046_32GPIB，Big5）。**V906 是從 905 翻的**（帳本「基準」表 :9） |
| 【R】 | `D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410\` | RS232Standard golden（Big5）。`MainForm.cpp:16` 是 `#define DEBUG`；V906 照裁決用出貨版（DEBUG 關，帳本 :129） |
| 【H】 | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\` | Handler golden **906_0625_Steven**（Big5；行號是這棵樹的，不等於 906_20260618） |
| 帳本 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md` | 移植帳本（權威） |
| 計畫 | `D:\HT9045\.claude\skills\ht9045-gpib-bridge\references\gpib-v906-integration-plan.md` | 轉換計畫 |

⚠ 交代的 GPIB golden `D:\GPIB9045\GPIB_Code_32Site_V12.13.883.0_20250915_Jimmy_20250924` **不在 STEVEN-NB3**（20260928 查過 `D:\GPIB9045`、`D:\AI_TempFile`、`E:\HT9045_Backup`），V906 也不是從 883 翻的，所以本檔的 GPIB `Main.cpp` 行號**一律是 905**。要 883 行號的話，待有那棵樹的機器補。SKILL.md §3 表格裡的 `Main.cpp:5300` 等是舊版行號，不要跟本檔混用。

---

## 1. 上機前要先解的缺口（要改程式，不在本檔範圍）

這些是寫本檔時讀程式碼發現的；不解的話某一步一定不會過，或會有安全上的漏洞。

| # | 缺口 | 影響 | 位置 |
|---|---|---|---|
| B1 | **已做（20260928，gpib-widget `35d17d44`）** 原本的問題：**TTL：`TfMain::GetTTLState` 的送出被 gate。** 整包 `MSG_CMD_State_TTL`（48 碼 TTL 參數）組好了，但 `SendMessage(fMain->HVisionWnd, WM_COPYDATA…)` 在 `#if 0` 裡；`HandlerHwnd`／`GpibHwnd` 也沒填 | RS232 引擎要的 `MSG_CMD_State_TTL` 回答永遠收不到 → **`WINIT` 永遠不會送到 TTL 板**（§7.2）。解法方向：照 `THandlerTesterSide::SendMSG_CMD` 經 `SendToBridge` 送，並填 `HandlerWndToken()`／`HVisionWnd`，否則 RS232 引擎的身分檢查會自己關（【V】`TesterComm\Rs232\Rs232HandlerMsg.cpp:322-329`）。Command.cpp 是共用檔，先認領 | 【V】`Command.cpp:11259-11262`（送出）、`:11093-11097`（視窗代號），GATE REGISTER 第 1、2 項 `:10421-10436`；呼叫端 【V】`TesterComm\Handler\HandlerGpibMsg.cpp:2134-2136`；golden 【H】`Command.cpp:10324`（函式）、`:10497`（送出） |
| B2 | **已做（20260928，gpib-widget `35d17d44`）** 原本的問題：**TTL：開機不讀 DIO 檔**，`Prod.DIOCfg` 開機時是全 0 | `WINIT` 的 Bin 模式、SOT 邏輯、SOT 寬度都從 `Prod.DIOCfg` 來（【V】`Command.cpp:11171-11248`）。只有在 DIO 設定頁存檔之後（出貨組態才跑 `InitDIOStstus`，【V】`FileRW\TTLCfg.cpp:270-286`）才是 DIO 檔的值 | 【V】`cDIOStatus.cpp:79-100`（`W906_BootInitDIOStstus` 本體 `#if 0`，理由 `:90-96`） |
| B3 | **已做（20260928，gpib-widget `35d17d44`）** 原本的問題：**TTL 一塊板的版本門檻沒有照 golden 改成 7071601** | golden 在 `TTL_CARD_TYPE==2` 時把 `TTLRS232VerCheck` 設成 7071601；V906 那行 `#if 0`（理由「MessageDef 沒翻」已過期，【V】`MessageDef.cpp:50` 已定義成 10051302）→ 一塊板的機台，韌體版本介於 07071601～10051301 的板子在 V906 會跳 `TTL RS232 Version Error!`（【V】`TesterComm\Handler\HandlerGpibMsg.cpp:872-876`），golden 不會 | 【V】`database.cpp:1219-1227`；golden 【H】`database.cpp:1095-1098`、`MessageDef.cpp:13` |
| B4 | **已做（20260928，gpib-widget `35d17d44`）** 原本的問題：**START 不擋「找不到 bridge／版本錯」** | On-Line 時 GPIB／RS232／TTL 沒接好也能按 START；計畫 P8 列要改用 `TesterCommHub::IsUp()`。沒改之前，START 後 `GetTesterResult` 在 bridge 不在時照 golden 每 3 秒重試、不前進（帳本 P2b「T10」:219、:230-232） | 【V】`WebStart.cpp:1712-1773`（SAFETY-GATE W906-ST-W2-H，檔內標 golden `:4782-4831`） |
| B5 | **已做（20260928，gpib-widget `35d17d44`）** 原本的問題：**TTL：逾時 Skip／Retry 不會清 SOT** | `SendTTLRS232CSOTsignal` 本體 gate（GATE h4-G1）；gate 的理由「`Send_Command_TTL` 不是門面成員」已過期（【V】`forms\fMain.h:942` 已宣告，本體 【V】`Command.cpp:11267-11278`）。沒解之前逾時後不送 `@WCSOT…`，下一個 `WSOTS` 板子可能回 `@ErrSOT`，引擎走 Double SOT（§7.4） | 【V】`csystem.cpp:28746-28790`；golden 【H】`csystem.cpp:23513` 起；呼叫端 【V】`atester.cpp:4107`、`:4168` |
| B6 | **On/Off-Line 的網頁入口** | WS 動作 `act.main.testerConnect` 已接（【V】`JsonBridge\ChanAction.cpp:347` → `JsonBridge\actions\MainTesterConnect.*`，golden imgTesterClick），但主畫面的 Tester 圖示仍送舊的 `request('TfMain::imgTesterClick', …)`（【W】`page\main-control.js:139-141`，帳本 P2e :447）。上機前要決定用哪個入口切 On-Line：**待上機確認** | 同左 |
| B7 | **網頁的橋接指令沒有權限閘** | `POST /api/testercomm/<key>?cmd=` 在 wb_serve 恆可用（帳本 P3 :457；403 只有 ctest 走得到，【V】`TesterComm\Handler\TesterCommWiring.cpp:209-215`）。`click btnManualStart`（GPIB）、`click btManualTest`／`btnTTL_Manual`／`btnClearSot`（RS232／TTL）會直接打到真設備（§8） | 【V】`TesterComm\Gpib\GpibUiSnapshot.cpp:20-33`、`TesterComm\Rs232\Rs232UiSnapshot.cpp:13-19` |

---

## 2. 前置

### 2.1 組態：出貨組態（ship）

- 建置（PowerShell；STEVEN-NB3 只編譯，執行在機台上）：
  `$env:V906_BUILD_DIR="build_ship"; $env:V906_CMAKE_ARGS="-DW906_NO_SOFT_SIMULTE=ON"; & "D:\HT9045\HT9011UC_Cpp_V3.33.906.0\build.bat" serve`
  - `serve`＝只建 wb_serve（【V】`build.bat:92-105`）；`V906_CMAKE_ARGS` 只在該 build 資料夾第一次 configure 時生效（`build.bat:35-38`）。
  - exe：`D:\HT9045\Obj\V906\build_ship\wb_serve.exe`（OBJ ROOT＝`<repo>\Obj\V906`，`build.bat:67-79`；在 worktree 建就是 `<worktree>\Obj\V906\build_ship\`）。
- 為什麼一定要 ship：`SOFT_SIMULTE` 只在沒有 `W906_NO_SOFT_SIMULTE` 時才定義（【V】`MachineType.h:63-65`、`CMakeLists.txt:58-66`）。模擬組態下讀不到的感測器一律 ON 等（`MachineType.h:48-62` 的註解）；TTL 的 `InitDIOStstus` 只在出貨組態做（【V】`cDIOStatus.cpp:56`）；D2「機台有 IC 不准切 On/Off-Line（MES1646）」只在出貨組態擋（【V】`TesterComm\Handler\HandlerTesterConnect.cpp:61`）。
- **GPIB 驅動的選擇跟組態無關**（§2.2）。RS232Standard 引擎自己的 `SOFT_SIMULTE`（golden 每一版都開）不跟 `W906_NO_SOFT_SIMULTE` 走、維持開（【V】`TesterComm\Rs232\Rs232Bridge.h:50-51`、`:883-896`）：它讓 `uSocketServer`（127.0.0.1:59999 的模擬器 socket，vclcompat SIM 模式，不開真 socket）跟著收發，並讓 `SendCommandToTester` 一律當作 port 已連（`TesterComm\Rs232\Rs232Comm.cpp:42-43`、`:55-58` 的註解）。
- 整個關掉的退路：環境變數 `HT9045_TESTERCOMM=0` → 這一次 wb_serve 不裝安裝座、不起任何引擎（【V】`TesterComm\Handler\TesterCommWiring.cpp:86-92`；帳本 P3 :470）。

### 2.2 從 Sim 換成真的 gpib-32.dll

- **沒有設定鍵，也沒有環境變數。** `GpibEngine::Start`：有注入的 driver 就用（只有兩支 ctest 會注入：【V】`tests\test_testercomm_gpib.cpp:276`、`tests\test_testercomm_handler.cpp:183`），否則 `new NiGpibDriver`；`LoadLibraryA("gpib-32.dll")` 成功、而且 12 個進入點 `ibfindA ibrsc ibpad ibtmo ibwait ibrd ibrsv ibwrt ibstop ThreadIbsta ThreadIberr ThreadIbcnt` 全找到才用，缺一個就當沒有卡（【V】`TesterComm\Gpib\GpibEngine.cpp:117-131`、`TesterComm\Gpib\GpibDriver.cpp:64-90`）。
  - 所以 wb_serve 裡 **`SimGpibDriver` 永遠不會出現**；裝了 NI-488.2 就是真卡，沒裝就是「沒有卡」（每個 `ib*` 回 ERR，`GpibDriver.cpp:22-51`）。
- 確認方法：testercomm.html GPIB 分頁的「驅動」欄（【W】`page\testercomm.html:61`、`:331`）＝快照 `driver`（`GpibEngine.cpp:254-255`）：`ni-gpib-32.dll`＝載到真驅動（`GpibDriver.h:90`）、`none`＝沒載到。
- 使用者說的「模擬」其實是 **Off-Line**：Handler 送 `bSimulate=(LastSet.iTester==OFF_LINE)`（【V】`TesterComm\Handler\HandlerBridgeCtl.cpp:1049`；SOT 那包 `:806`），橋接收到後 log `Handler ==> Simulate GPIB`／`Handler ==> Normal GPIB`（【V】`TesterComm\Gpib\GpibHandlerMsg.cpp:238-253`；【G】`Main.cpp:3336-3349`）。Off-Line 的測試結果走橋接內部的假 bin（【V】`TesterComm\Gpib\GpibCore.cpp:965-1066`；【G】`Main.cpp:4009` 起），不碰匯流排。
- ⚠ **Off-Line 也會開卡、也會聽匯流排**：不在測試時 `ProcessMessage` 每 1 ms 叫一次 `TestGPIB()`（`GpibCore.cpp:1069-1075`），case 1 位址變了就 `ibfind("gpib0")`／`ibrsc`／`ibpad`／`ibtmo`（`TesterComm\Gpib\GpibTestGpib.cpp:113-131`），之後 `ibwait` 聽、收到字就照查詢處理（`:142-237`；【G】`Main.cpp:1524-1620`）。golden 相同。
  - Off-Line 時找不到卡**不會記 log**（`Error Open GPIB0` 只在 `bSimulate==false` 時寫：`GpibTestGpib.cpp:118-121`、`GpibCore.cpp:1230-1231`）→ Off-Line 下一定要看「驅動」欄。
  - 要完全離開匯流排：拔 GPIB 線，或用 `HT9045_TESTERCOMM=0` 重開 wb_serve。

### 2.3 NI-488.2 驅動

- 要有 NI-488.2 runtime 提供 `gpib-32.dll` 且含上面 12 個匯出（`GpibDriver.cpp:64-65`）。`LoadLibraryA` 沒寫死路徑，走 Windows DLL 搜尋順序（`GpibDriver.cpp:72`）。
- wb_serve 用 `C:\MinGW` 6.3.0 建（【V】`build.bat:63` `MINGW_BIN=C:\MinGW\bin`），應是 32 位元 exe，要配 32 位元的 `gpib-32.dll`：**待上機確認**（看 exe 屬性與 NI 裝的是哪一份）。
- 橋接是 GPIB **device（非控制器）**：`ibfind("gpib0")` → `ibrsc(ud,0)` 放掉系統控制 → `ibpad(ud,GpibAddress)` → `ibtmo(ud,LastSet.iTimeOut)`（`GpibCore.cpp:1227-1249`；【G】`Main.cpp:4262-4284`）。NI MAX 裡要有名為 `GPIB0` 的卡；卡的其他設定（NI MAX 的 primary address、System Controller 等是否會被上面三個呼叫完全蓋掉）：**待上機確認**。
- 位址：配方 `[GP-IB] Address` → `TestIF_File.iGpibAddress`（【V】`FileRW\TestIF_File_TesterIF.gen.inc:1433`）→ `MSG_CMD_ChangeGpib` 的 `GpibAddress`（`HandlerBridgeCtl.cpp:1042`）→ 橋接 `GpibAddress`（`GpibHandlerMsg.cpp:255-262`）。橋接開機先用自己 `general.ini [SystemSetup] GpibAddress`（預設 1，`TesterComm\Gpib\GpibUi.cpp:1498`），≤0 或 ≥31 一律改 1（`GpibUi.cpp:1109-1111`；【G】`Main.cpp:478`）。要跟 Tester 程式裡設定的 Handler 位址一致。
- 逾時：`general.ini [SystemSetup] TimeOut` 是 NI 的 timeout 代碼，預設 11（狀態列 `Time Out: 1s`），夾在 8～12（30 ms～3 s）（`GpibUi.cpp:1499`、`:1580-1585`，狀態列對照 `:1084-1104`；【G】`Main.cpp:4371`、`:4452-4456`）。

### 2.4 測試機型與介面設定

**配方** `D:\HT9045\IniData\Data\<配方>\Tester.Data`（DataPath＝【V】`common.cpp:225`；讀檔 `FileRW\TestIF_File_TesterIF.gen.inc:1111-1118`）：

| 鍵 | 值 | 位置 |
|---|---|---|
| `[Mode] Tester Type` | 0＝TTL_MODE（DIO＝TTL 板）、1＝GPIB_MODE、2＝RS232_MODE、3＝TCP_IP_MODE；超出範圍改寫成 1 | 【V】`cmydef.h:61-64`；`gen.inc:1118-1127` |
| `[GP-IB] Type` | `iGpibMode`＝InterfaceType_*：0 ADVAN_Type1、1 256Bin、2 16Bin、3 32Bin、4 SPEA、5 16BinGS、6 32BinGS、7 15BinT6577、8 15BinQorvo、9 Delta_Castle；Auto Retest 條件成立時改用 config 的 A10 值 | 【V】`cmydef.h:67-76`；`gen.inc:1418-1427` |
| `[GP-IB] Address` | GPIB 位址（§2.3） | `gen.inc:1433` |
| `[RS-232C] Type` | 0＝eRs232Standard、1＝eRs23232Bin | 【V】`MachineType.h:630-631`；`gen.inc:1453` |
| `[RS-232C] Bin Count` | 32Bin 模式的最大 bin；送給引擎的是 Bin Count+1 | `gen.inc:1455`；`HandlerBridgeCtl.cpp:817-820` |
| `[RS-232C] BaudRate／Bit Length／Stop Bit／Parity` | Tester COM 的格式（見下方 P6） | `gen.inc:932-935` |
| `[DIO] Type／TypeName` | DIO 設定檔（在 `D:\HT9045\iniData\DioCfg\`，【V】`common.cpp:227`）→ TTL 的 `WINIT` 參數 | `gen.inc:1264`、`:1413` |
| `[Time] MAX Time` | `TestIF.iMaxTime`：測試逾時（§4.4） | `gen.inc:879`；【V】`atester.cpp:11332-11364` |

**機台層級**：

| 檔／鍵 | 用途 | 位置 |
|---|---|---|
| `D:\HT9045\system\Gerneral.ini [System] TTL_CARD_TYPE` | 2＝一塊 TTL 板、3＝兩塊；<2 是直接讀 DIO 卡的舊分支，C++ 線**不移植**（那種機台停在 Task=100、不報警） | 【V】`database.cpp:1218`；【H】`database.cpp:1094`；帳本 :234 |
| `… [System] TTL_CARD_USE_ADDRESS` | 1＝TTL 板帶站號（框前多 `00`） | 【V】`database.cpp:1228`；【H】`database.cpp:1099` |
| `… [Version] Model／Machine ID` | 兩個橋接都讀：GPIB 的 `CHKMATCH?`／`HANDLERID?`、RS232 的 `CZ id?`／`CZ which?` | `GpibUi.cpp:1174-1175`（【G】`Main.cpp:535-536`）；【V】`TesterComm\Rs232\Rs232Parse.cpp:528-551` |
| `D:\HT9045\config\config.ini [Tester] bI38SETTEMPRespondSetTemp` | `SETTEMP?` 的回覆格式（§3.2） | 【H】`cConfiguration.cpp:2763`（預設 0）；SKILL.md §4 |
| `D:\GPIB9045\system\general.ini [Version] Model` | GPIB 引擎白名單 `9046_32GPIB／9045GPIB／9046GPIB／9045GPIB_12Site／2601GPIB／9055GPIB／502GPIB／7080GPIB／9050GPIB／1032GPIB`；不在清單＝記 `D:\GPIB9045\system\general.ini "Model" read error!!` 並自己關；缺鍵時會先寫入 `ModelNG`（CheckAndReadIniData 寫回預設） | `GpibUi.cpp:1465`、`:1289-1307`、`:1587-1639`；【G】`Main.cpp:4337`、`:620-650` |
| `… [SystemSetup] iTesterMode` | 橋接開機值；之後被 Handler 的 `MSG_CMD_TesterMode`（GPIBBin＝配方 `[GP-IB] Type`）蓋掉並寫回 | `GpibUi.cpp:1477`；`GpibHandlerMsg.cpp:303-339`；`HandlerBridgeCtl.cpp:657` |
| `… [SystemSetup]` 其他 | `StringLength`（true）、`Upper Case`（true）、`Binon Echo`（true）、`BinTotal`（16）、`iTACS_ATNTimeOut`（10 s，夾 1～120）、`iMyGpibWriteRetry`（20）／`iMyGpibWriteThreshold`（10）／`iMyGpibWriteWaitMS`（100）／`bMyGpibWriteVerboseLog`（false）、`Enable_Start_Button`（true）、`bGPIBWriteWithout_r_n`（false）、`RETURN_GPIB_VERSION`（0）。裁決 6＝A：時間類設定維持機台層級 | `GpibUi.cpp:1466-1548`；【G】`Main.cpp` 在這段＝V906 行號＋2872（例 `:4385` iTACS_ATNTimeOut、`:4398` Enable_Start_Button） |
| `D:\RS232Standard\System\Setup.ini [COMPort] CommName／BaudRate／ByteSize／StopBits／Parity` | Tester COM；預設 COM3／9600／7／1／Even | 【V】`TesterComm\Rs232\Rs232Log.cpp:425-433`；【R】`MainForm.cpp:2541-2549` |
| `… [Detail Settng] ReadIntervalTimeout／ReadIntervalTimeout_TTL` | 預設 70 | `Rs232Log.cpp:433`、`:443-444` |
| `… [COMPort_TTL] CommName`、`[COMPort_TTL_2] CommName` | TTL 板 1／2 的 COM；預設 COM3／COM8。TTL 板固定 115200 8N1 | `Rs232Log.cpp:440-441`、`:410-414`、`:559-560`；【R】`MainForm.cpp:2556`、`:2681` |
| `… [SystemSetup] iTesterMode` | 0 Standard、1 SLT、≥1000 TTL（1000+DIO Type）。**引擎開機（FormShow）用它決定開 Tester COM 還是 TTL COM** | 【V】`TesterComm\Rs232\Rs232Ui.cpp:2242-2344`；【R】`MainForm.cpp:420` |
| `… [SystemSetup] bCheckClosedSiteHasBin` | 1＝關 site 有 bin 時全部 999 並報警 | `Rs232Ui.cpp:2245`；`Rs232HandlerMsg.cpp:264-282` |

**P6 裁決**（計畫 §2.4 開頭；帳本 P6）：
- 1＝A：Tester／TTL1／TTL2 的 COM port **名稱**留在 Setup.ini（機台層級）。
- 格式（Baud／Bit／Stop／Parity）以配方為準：RS232 模式的配方存檔時 `CheckRs232StandardIni` 寫進 Setup.ini，值有變就 `CloseGpibProgram` 讓引擎重讀（【V】`FileRW\TestIF_File_TesterIF.gen.inc:1030-1102`，存檔時 `:1528`）。
- Setup.ini 的碼：ByteSize 0:5 1:6 2:7 3:8、StopBits 0:1 1:1.5 2:2、Parity 0:None 1:Odd 2:Even 3:Mark 4:Space（`gen.inc:1052`、`:1067`、`:1082`）；配方的碼（5B 新選項接在舊 index 後）在帳本 :306-310 與 【V】`TesterComm\Rs232SetupCodes.h`。
- W3：存檔擋 5 bits＋2 stop、6／7／8 bits＋1.5 stop（帳本 :316）。
- GPIB 模式配方的額外 RS232 port 是 AMD／ATC 輔助線（Q2 (a)，帳本 :346-402），不是 Tester 通訊，不在本檔。

### 2.5 先備份的檔

| 檔 | 為什麼會被寫 | 位置 |
|---|---|---|
| `D:\GPIB9045\system\general.ini` | GPIB 引擎的 CheckAndReadIniData 缺鍵寫回預設；ReadLastDataFile 寫回 5 個夾過的值；WriteLastDataFile（換位址／bin 數／tester mode／FormClose 都寫） | 【V】`TesterComm\Gpib\GpibGlobals.cpp:26-31`、`:413-448`；`GpibUi.cpp:1561-1573`、`:1671-1700` |
| `D:\GPIB9045\system\GpibString.dat` | Fullsites 字串變了就寫 | `GpibGlobals.cpp:401-407`；`GpibCore.cpp:959-963` |
| `D:\HT9045\system\Gerneral.ini` | 兩個橋接讀 `[Version] Model／Machine ID`、`[System] CUSTOMER_CODE／TTL_CARD_TYPE`，**缺鍵時寫回預設**（例 `HT-9046`）；量產機共用檔 | `GpibGlobals.cpp:26-29`；`Rs232Ui.cpp:2243-2244`；`Rs232Parse.cpp:528-531` |
| `D:\RS232Standard\System\Setup.ini` | 引擎 LoadSetupData 缺鍵寫回、Update 鈕 SaveSetupData、配方存檔 CheckRs232StandardIni | `Rs232Log.cpp:49-51`、`:380-404`；`gen.inc:1031-1102` |
| 配方 `Tester.Data` | Q2 (a)：GPIB 配方第一次抄 Setup.ini 的 4 個值＋標記 `W906_SeededFromSetupIni=1`；W1：缺 `[AutoRetest] iTesterType` 時寫 1、學到的廠牌寫回 | 帳本 :360-362、:331-333 |
| 配方 `Temperature.Data` | GPIB `SETTESTOFFSET_`／`DEVICETEMP`（W9）會寫溫度 offset，而且會累加 | SKILL.md §8.7 W9 |
| 配方 `TestMode.Data` | 切 On/Off-Line 時 SaveTestMode 寫 `[TestMode] Tester Connection`（`bLastSetInSetUpFile` 開時） | 【V】`cprod.cpp:3504-3521` |
| `D:\HT9045\system\lastdata.dat` | `MSG_CMD_SCKART_LOTRTCLEAR` 照 golden 寫（裁決 6＝B） | 帳本 :498 |

### 2.6 要看的 log

**GPIB**（引擎＝H9046_32GPIB 的翻譯）
- 即時：testercomm.html GPIB 分頁的 `lstRecord`（最近 200 行，【V】`TesterComm\Gpib\GpibUiSnapshot.cpp:50`）、`mmoBINON`（收到的 BINON 原文）、ibsta LED 列（`GpibUi.cpp:1383-1399` 的 UpdateLed）、「驅動」欄。視窗在 HMI 工作列（【W】`background.html:512`，hidden＋lazy），或直接開 `page/testercomm.html#gpib`（`testercomm.html:198-208`）。
- 檔：`D:\GPIBLOG\Log\YYYY_MM\YYYY-MM-DD HH MM SS.txt`（Save_Log，`GpibUi.cpp:1330-1356`；【G】`Main.cpp:673-699`）。**只在** lstRecord 超過 10240 行（`GpibUi.cpp:1376-1380`）、橋接關閉 FormClose（`:944-956`）、按 Save Log（`:1874-1876`）時才寫 → 上機中要檔就先按 Save Log；或 Diag Zip → `D:\GPIBLOG\Diag\GPIB_Diag_<時間>.zip`（`:1881-1944`；【G】`Main.cpp:8301-8364`）。
- 行格式 `YYYY-MM-DD, HH:MM:SS.mmm, <內容>`（`GpibUi.cpp:1366`），內容先 Trim（`:1361`），所以結尾的空白與 `\r\n` 看不到。字首 `0001`／`0100`／`0200`／`0300`／`0400`／`0500`／`0600` 是 `TestGPIB` 的 Task 號（§4）。

**RS232／TTL**（引擎＝RS232Standard 的翻譯）
- `D:\RS232Log\LOG\YYYY\MM\RS232_Log_YYYYMMDD HH.log`，兩小時一檔（HH＝偶數），欄位 `Date, Time, Action, Message, Hex`（`Rs232Ui.cpp:1814-1817`；【V】`TesterComm\Rs232\Rs232Support.cpp:275-306`；【R】`MainForm.cpp:361-364`）。
- Bin log：`D:\RS232Log\BinLog\YYYY_MM\YYYY_MM_DD_HH MM SS_Rs232_BinData.log`、`D:\RS232Log\BinLog_TTL\YYYY_MM\…_TTL_Rs232_BinData.log`（超過 10000 行或關閉時才存，`Rs232Log.cpp:485-551`、`Rs232Ui.cpp:2376`；【R】`MainForm.cpp:2607-2675`）。
- 頁面 RS232／TTL 分頁：`MemoLog`（`[T->H]`／`[H->T]`／`[Handler ==> RS232]`／`[RS232 ==> Handler]`／`[TTL1]`）、`MemoBinData`、`MemoBinData_TTL`、`MemoVer`、`comm`（Tester／TTL1／TTL2 三個連線旗標）（`Rs232UiSnapshot.cpp:4-12`）。
- ⚠ TTL 板的版本（`VERS`）只在勾了 `cbShowLog_TTL` 或 `cbSaveLog_TTL` 時才解析（帳本 :155）→ TTL bring-up 先勾 Show Log。

**Handler**
- 警報：WAR07352（測試逾時）、WAR07326（關 site 有 bin）、WAR07327（BINON 沒有 0x41）、WAR07328（BINON 沒有 FULLSITES）、WAR07317（RS232 BA 沒有 CE）（`HandlerGpibMsg.cpp:1248-1297`；【H】`main.cpp:16332-16379`；【V】`atester.cpp:3957-4019`）。
- 事件紀錄：`GPIB Close - <來源>`（`HandlerBridgeCtl.cpp:945`）、`GPIB Command …`（`HandlerGpibMsg.cpp:628-734`）；P4 落地後寫到 `D:\HT9045_Log\EventLogTxt` 與 `D:\HT9045_Log\SaveEventLog\HANDLER LOG_<HandlerID>_yyyy_mm_dd.csv`（`progress-st02.md` #24 的「上機要看」）。
- 分 bin 計數：Status.ShowBinSelect 視窗（【W】`page\Status.ShowBinSelect.html`，`background.html:428`），tag `binsel.cat.counts`／`binsel.cat.sum`（【V】`WebBridgeTags.cpp:3093-3107`）。

---

## 3. 第一步：GPIB 查詢類

### 3.1 引擎起來時應該看到的

條件：配方 `[Mode] Tester Type=1`。Handler tick 每 1000 ms 跑一次 ProcessHVisionConnect（【V】`TesterComm\Handler\TesterCommWiring.cpp:24`、`:132-137`），找不到 bridge 就 WakeupGPIB，之後每 10 秒重試（`HandlerBridgeCtl.cpp:258-263`）。GPIB log 依序：

1. `=== Program On @ Revision V12.13.905.0 ===`（`GpibUi.cpp:1198`；版本字串是固定的，帳本 :82）。
2. 約 1 秒後 `GPIB run with Handler : <Model>`，同時送 `MSG_CMD_Version`（`V12.13.905.0`）與 `MSG_CMD_TesterMode` 給 Handler（`GpibCore.cpp:836-847`；【G】`Main.cpp:3121-3124`）。Handler 只比對主次版 12.13，不符就跳 `GPIB Version Error!! …`（`HandlerGpibMsg.cpp:887-900`）。
3. `Handler ==> Change tester mode ADVAN_Type1`（或其他 InterfaceType 的字）（`GpibHandlerMsg.cpp:303-326`；【G】`Main.cpp:3405-3423`）。
4. `Handler ==> Change GPIB Setting Start`、`Handler ==> Normal GPIB`（On-Line）或 `Handler ==> Simulate GPIB`（Off-Line）、`Handler ==> CHANGE ADDR : <n>`（位址有變才有）、`Handler ==> GPIB MODE`／`Other MODE`、`Handler ==> Test Bin Count : 16`、`Handler ==> Change GPIB Setting End`（`GpibHandlerMsg.cpp:234-275`；【G】`Main.cpp:3334-3370`）。
5. On-Line 而且是 GPIB 模式：`Find GPIB`（`GpibCore.cpp:1241`；【G】`Main.cpp:4276`）；找不到卡則一直記 `Error Open GPIB0`，10 次後不再試（`GpibCore.cpp:1227-1238`）。

### 3.2 查詢指令與預期回覆

Tester 送來的字先轉大寫再比「開頭相同」（`strupr` 後 `Pos(...)==1`，`GpibTestGpib.cpp:190-208`；SKILL.md §3 更正）。第一步**只送下表這些問句**。

| Tester 送 | 橋接做什麼 | Tester 收到（bytes） | V906 | golden |
|---|---|---|---|---|
| `SETTEMP?` | 送 `MSG_CMD_HandlerTemperature`(65) → Handler `WriteTempData` → 橋接原樣轉回 | `TempDataStrings()`＋`" \r\n"`。`[Tester] bI38SETTEMPRespondSetTemp`：**0**（預設）常溫 `+25.0 \r\n`、加熱 `+<設定溫度>.0 \r\n`；**1** 常溫 `Settemp +<fAbitTemp，%0.1f> \r\n`、加熱 `Settemp +<設定溫度>.0 \r\n`；**2** 整數 `25 \r\n`；Delta Castle 且 0：`25.0\n \r\n` | `TesterComm\Gpib\GpibCommands.cpp:468-472`；轉回 `GpibHandlerMsg.cpp:494-586`；Handler `HandlerGpibMsg.cpp:1093-1096`、【V】`Command.cpp:1838-1882` | 【G】`Main.cpp:5398-5402`、`:3601`、`:3680-3681`；【H】`Command.cpp:1543-1586` |
| `VERSION?` | 橋接自己回（每次都讀 general.ini） | `general.ini [SystemSetup] Version`（預設 `IFUNT200 Version B17`）＋`\r\n`；A10-3 開時 16／32BinGS 回 `ADVANTEST M4871 Rev.1.P3 FULLSITEB`、其他 `IFUNT200 Version B17` | `GpibCommands.cpp:424-440` | 【G】`Main.cpp:5354-5370` |
| `CHKMATCH?`／`CHKMACH?` | 自己回 | `<Gerneral.ini [Version] Model>\r\n`（預設 `HT-9046`）；`RETURN_GPIB_VERSION=1` 時 `NEWOI-V12.13.905.0-HT9045W` | `GpibCommands.cpp:441-452`；`GpibUi.cpp:1174`、`:1184` | 【G】`Main.cpp:5371-5383`、`:535` |
| `*IDN?` | 自己回 | `HONTECH`；2DID 格式是 AMD 時 `1 Hontech,ROGERS,0,Rogers V12.13.905.0` | `GpibCommands.cpp:454-466` | 【G】`Main.cpp:5384-5396` |
| `HOSTNAME?` | 自己回 | 這台電腦的名稱（GetComputerName） | `GpibCommands.cpp:223-228` | 【G】`Main.cpp:5153-5158` |
| `HANDLERID?` | 自己回 | Handler config 的 I25（`IniConfig.iI25UseGPIBFormat`，裁決 4A）開：`<Gerneral.ini [Version] Machine ID>\r\n`（預設 `29828`）；關：**不回** | `GpibCommands.cpp:531-538`；`GpibUi.cpp:1176-1180` | 【G】`Main.cpp:5457-5464`、`:536` |
| `SETSOAK?` | 送 `MSG_CMD_HandlerSoakTime` → Handler `WriteSoakTimeData` → 轉回 | 格式：**待上機確認**（Handler 本體沒逐行核） | `GpibCommands.cpp:483-487`；`HandlerGpibMsg.cpp:1089-1091` | 【G】`Main.cpp:5413`；【H】`Command.cpp:1766` |
| `SITEMAP?`（或不帶參數的 `SETSITEMAP`） | 送 `MSG_CMD_HandlerSiteMap` → `WriteSiteMapData` | **待上機確認** | `GpibCommands.cpp:524-529`；`HandlerGpibMsg.cpp:1085-1087` | 【G】`Main.cpp:5450-5455` |
| `HANDLER ID?`／`ID?` | 送 `MSG_CMD_HandlerID` → `WriteHandlerID` | **待上機確認** | `GpibCommands.cpp:539-544`；`HandlerGpibMsg.cpp:1081-1083` | 【G】`Main.cpp:5465-5470`；【H】`Command.cpp:1484` |

⚠ 第一步**不要送會寫東西的指令**：`SETTEMP +`／`SETTEMP_`（改機台溫度，`GpibCommands.cpp:473-477`）、`SETSOAK `／`SETSOAK_`（`:488-492`）、`SETSITEMAP `／`SETSITEMAP_`（`:494-498`）、`DEVICETEMP`／`SETTESTOFFSET_`（W9，寫配方 Temperature.Data，`:597-603`），以及 `BINMAP_`、`SETUP_`、`SGSETUP_`、`SETSTARTMODE_`、`PPSELECT` 一類。

### 3.3 一個查詢在 log 裡的樣子（以 SETTEMP? 為例）

```
0001 LISTEN:SETTEMP?                         GpibTestGpib.cpp:201      【G】Main.cpp:1583
GPIB to Handler <== HandlerTemperature        GpibCore.cpp:1529         【G】Main.cpp:4909
Handler ==> HandlerTemperature : +25.0        GpibHandlerMsg.cpp:578    【G】Main.cpp:3673
0001 TALK:+25.0                               GpibCore.cpp:1304         【G】Main.cpp:4672
```

`0001` 是閒置時的 Task 號（`sGbibTask`＝`%04d` 的 `iGbibTask`，`GpibHandlerMsg.cpp:43-44`）；指令名稱來自 `slCmdList`（`HandlerTemperature` 在 `GpibUi.cpp:663`）。

### 3.4 預期／失敗時看哪裡／停止與還原

- **預期**：3.1 的五行都出現；「驅動」欄是 `ni-gpib-32.dll`；每個問句一行 `LISTEN`、一行 `TALK`，Tester 收到上表的字串。
- **失敗時**：
  - 驅動欄 `none` → gpib-32.dll 沒載到或少進入點（§2.2、§2.3）。
  - `GPIB : No handler window, close GPIB,  _Timer1_` → `general.ini [Version] Model` 不在白名單（`GpibUi.cpp:1289-1307`）；`GPIB : No handler window, close GPIB, _OnMyCopyMsg_` → Handler 送來的視窗代號不對（`GpibHandlerMsg.cpp:68-74`）。
  - 一直 `0001 Wait TACS:…`、最後 `MyGPIBWrite GIVE UP after 20 retries` → Tester 沒有把橋接定址成 talker（`GpibCore.cpp:1386-1407`；【G】`Main.cpp:4761-4772`）；`ibwrt fail … [retry=…ibsta=0x…]` 那行有 ibsta 位元（`GpibCore.cpp:1349-1352`），想看每一次等待就把 `bMyGpibWriteVerboseLog` 設 1。
  - 沒有 `LISTEN` → 位址不一致、線沒接、或剛結束一個 cycle 的 **8 秒內**閒置時不讀匯流排（`IsTestDelay`，`GpibCore.cpp:894`、`GpibTestGpib.cpp:144-147`；【G】`Main.cpp:1526`；golden 相同）。
  - `HANDLERID?` 沒有回覆 → I25 關著是 golden 行為。
- **停止／還原**：切回 Off-Line（B6 的入口），ChangeTesterConnect 收尾會 `CloseGpibProgram`，1 秒內依新模式重起（帳本 P2d :290）；要完全停掉就關 wb_serve 或以 `HT9045_TESTERCOMM=0` 重開；最後把 §2.5 備份的檔放回去。
- Off-Line 時問句照樣會回（閒置聽匯流排那條路不看 `bSimulate`，§2.2），所以可以先在 Off-Line（機台不跑測試）做這一步：**待上機確認**。

---

## 4. 第二步：GPIB SOT／EOT 一個 cycle

### 4.1 建議順序

1. **只有橋接、機台不動**：GPIB 分頁勾要測的 site → `Manual Start`（`click btnManualStart`）。它把勾的 site 設成 `iStart`、`bSimulate=false`、`IsTest=true`，走下面同一條 SRQ 流程（`GpibUi.cpp:1417-1455`；【G】`Main.cpp:4289`）；要 `general.ini Enable_Start_Button=true` 才按得到（`GpibUi.cpp:1526`、`:1540`）。結束時結果一樣會送一包給 Handler（`SendCaptureFinish`），Handler 沒在等所以不會分 bin；留下的狀態會不會影響下一個 cycle：**待上機確認**。
2. **Handler 帶的一個 cycle**：On-Line、一個 site、一顆 IC（或 One Cycle）。

### 4.2 流程（Handler 帶的 cycle）

| # | 誰 | 做什麼 | log | V906 | golden |
|---|---|---|---|---|---|
| 1 | Handler | atester Task 55 → `fMain->RunTestProgram(true, flag2)`：`MSG_CMD_NONE`，`Site[]`、`IsTest=true`、`bSimulate`、`GpibAddress`、`GPIBBin=iTestBinCount`、`iLotStatus` 0（double contact 1／2／3）；接著 `SetTestTimeOutTimer` | — | 【V】`atester.cpp:1652`、`:1680`；`HandlerBridgeCtl.cpp:684-918` | 【H】`atester.cpp:1530`；【H】`main.cpp:18351` 起 |
| 2 | 橋接 | 收 `MSG_CMD_NONE`：`iStart[]`；回一包 `Result="ECHO"` → Handler `bExist=true` | `Handler ==> START TEST`、`Handler ==> Site Mapping : …` | `GpibHandlerMsg.cpp:588-705`；`HandlerGpibMsg.cpp:1928-1935` | 【G】`Main.cpp:3683-3780`；【H】`main.cpp:16952` |
| 3 | 橋接 | ProcessMessage：`IsTest` → Task 100 → `TestGPIB()` | — | `GpibCore.cpp:1069-1079` | 【G】`Main.cpp:3953` 起 |
| 4 | 橋接 | case 100：`ibrsv(0x41)` 要求服務（double contact：0x42／0x43／0xC1） | `0100 SRQ:0x41` | `GpibTestGpib.cpp:367-402` | 【G】`Main.cpp:1749-1784` |
| 5 | Tester | serial poll（讀狀態位元組）後送 `FULLSITES?` | — | — | — |
| 6 | 橋接 | case 200：收到 `FULLSITES?` → `ibrsv(0x00)` 清掉 SRQ，組 `Fullsites %08X\r\n`（Upper Case 開時大寫；一個 site 都沒有就結束） | `0200 LISTEN:FULLSITES?`、`0200 SRQ:0x00` | `GpibTestGpib.cpp:431-505` | 【G】`Main.cpp:1813-1890` |
| 7 | 橋接 | case 300：`MyGPIBWrite(GpibString,"0300")`（等 TACS） | `0300 TALK:Fullsites 00000001` | `GpibTestGpib.cpp:591-629` | 【G】`Main.cpp:1973-2010` |
| 8 | Tester | 測試，送 `BINON:…;`（§5） | `0400 LISTEN:BINON:…` | `GpibTestGpib.cpp:630-1215` | 【G】`Main.cpp:2012-2600` |
| 9 | 橋接 | case 500：等 TACS，回 `ECHO:<BINON 冒號後原文>` | `0500 TALK:ECHO:…` | `GpibTestGpib.cpp:1275-1336` | 【G】`Main.cpp:2654-2715` |
| 10 | Tester | 送 `ECHOOK`（`ECHOOK:ONECYCLE` 會要求 Handler One Cycle） | `0600 LISTEN:ECHOOK` | `GpibTestGpib.cpp:1386-1450` | 【G】`Main.cpp:2765-2830` |
| 11 | 橋接 | ret=1 → 32 站結果放進 `Result[]` → `SendCaptureFinish` | `GPIB to Handler <== <32 個數字，site32…site1>,` | `GpibCore.cpp:1079-1136`、`:862-893` | 【G】`Main.cpp:3919-3950` |
| 12 | Handler | `MSG_CMD_NONE`（不是 ECHO）：依配方 Site Map 填 `iBin`、`tTestResult`，`bEcho=true`，GetTesterResult 往下走 | — | `HandlerGpibMsg.cpp:1928-2105` | 【H】`main.cpp:16952` 起 |

### 4.3 SRQ 與 serial poll

- 橋接是非控制器：只能 `ibrsv(<狀態位元組>)` 要求服務，serial poll 由 Tester（控制器）發起、NI 驅動回應；**橋接的程式碼看不到 poll 本身**，log 只有 `SRQ:0x..` 那行。poll 有沒有發生：**待上機確認**（用 NI 的 I/O Trace／NI Spy 看）。
- 這個 cycle 用到的：`0x41` 開測（`GpibTestGpib.cpp:401-402`）、`0x00` 收到 FULLSITES? 後清掉（`:447-448`）。其他：`0x42`／`0x43`／`0xC1` double contact（`:370-389`）、`0xC0`／`0x48` ART 的 lot 狀態（`:160-172`）、`0x44` RFMD ESC（`:1634-1637`）、`0x59` OverDrive／ReContact／Device map（`GpibHandlerMsg.cpp:186-227`）。

### 4.4 逾時長什麼樣子

- **Handler 端**：`SetTestTimeOutTimer`：兩個 per-arm 計時器＝`[Time] MAX Time`，`TestTimeOut`＝MAX Time＋10 秒（【V】`atester.cpp:11332-11364`；帳本 :236-249）；per-arm 的判斷在 【V】`aTester_Front.cpp:4239`、`:4268`，`aTester_Rear.cpp:4090`、`:4120`（帳本 :238）。到期 → `ProcessTesterTimeOut` → 警報 **WAR07352**，按鍵依 `IniConfig.iI22TestTimeOutOption`：0 Skip、1 Retry、2 Skip＋Retry、其他 Home（部分客戶碼只能 Skip）（`atester.cpp:3957-4019`；【H】`atester.cpp:3594-3640`）。
  - Skip → 送 `MSG_CMD_TimeOutSkip`，橋接記 `Handler ==> Test Time Out - SKIP`（`atester.cpp:4168-4175`；`GpibHandlerMsg.cpp:94-97`；【G】`Main.cpp:3195`）。
  - Retry：已收到 echo 就重讀結果；否則 `RunTestProgram(false)`（橋接記 `Handler ==> HALT TEST`，`GpibHandlerMsg.cpp:614`）＋`MSG_CMD_TimeOutRetrySend` → `Handler ==> Test Time Out - Retry and Resend SOT`，I12 開時改送 RetryWait → `… Retry and Wait Result`（`atester.cpp:4021-4076`；`GpibHandlerMsg.cpp:98-105`；【G】`Main.cpp:3199-3203`）。
  - 沒有操作員按時，`ShowErrorMessage` 預設 K_RETRY，會一直重試（帳本 :229）。
- **橋接端**（不會自己報 Handler 警報，只記 log）：
  - Tester 一直不送 `FULLSITES?`：停在 Task 200（FullSite 逾時被強制關掉，`GpibUi.cpp:1575`），最後由 Handler 的 WAR07352 收尾。
  - 回 Fullsites 等不到 TACS 10 秒：`Wait to reply Fullsites Time Out!`（`GpibTestGpib.cpp:592-619`；【G】`Main.cpp:1976-1999`）。
  - 回 ECHO 等不到 TACS：`iTACS_ATNTimeOut`（預設 10 秒）後 `GPIB ==> TACS and ATN Check Time Out (ECHO BINON)`，回 Task 400 再等 BINON（`GpibTestGpib.cpp:1358-1383`）。
  - 每一次寫都可能 `Wait TACS`×20（100 ms）→ `GIVE UP`（§3.4）。這 2 秒只卡通訊執行緒，不卡機台執行緒（計畫 §2.1 第 7 條）。

### 4.5 預期／失敗時看哪裡／停止與還原

- **預期**：4.2 的 log 依序出現；Handler 在 MAX Time 內拿到結果，沒有 WAR07352；GPIB 分頁 site 面板顯示 bin（`%4d`，關的 site `----`，`GpibTestGpib.cpp:1056-1074`、`:1172-1186`）。
- **失敗時**：停在 `0100 SRQ:0x41` 之後沒有 `0200 LISTEN` → Tester 沒收到 SRQ／沒 poll／位址錯；Handler 端 WAR07327／WAR07328 → Tester 沒照流程（§5.3）；`Test Time : n Sec`（`lblTestTime`，`GpibCore.cpp:1133-1135`）可看單次時間。
- **停止／還原**：WAR07352 選 Skip（全部進 error bin，`atester.cpp:4152`）；切 Off-Line；必要時關 wb_serve。

---

## 5. 第三步：32 站 BINON

### 5.1 格式（預設 ADVAN_Type1、`iBinSelect<=16`）

`BINON:` ＋ 4 組 8 碼、組間逗號、結尾 `;`（或 `\r`、`\n`、`\0`），共 42 字：

```
位置  0-5    6-13        14   15-22       23   24-31       32   33-40       41
      BINON: [site32…25] ,    [site24…17] ,    [site16…9]  ,    [site8…1]   ;
```

- 每一碼是一個 site 的 bin，十六進位 `0-9`、`A-F`（大小寫都收）；**每組最左邊是編號大的 site**，最右一碼是 site 1（解碼 `pstr[33+i]→result[7-i]` 等，`GpibTestGpib.cpp:795-842`；長度與字元檢查 `GpibCore.cpp:318-500`，`:428-500` 是這個分支）。
- `iBinSelect`＝Handler 送的 bin 數（`iTestBinCount`，預設 16，【V】`cmydef.cpp:3604`）：17 → `0-9`、`A-G`；33 → `0-9`、`A-W`；其他 → 256 bin，一站 3 位數（`GpibTestGpib.cpp:844-1030`）。
- ⚠ 16BinGS／32BinGS 的 BINON 永遠不過、256 bin 只看每站第一位數：照 golden 保留（裁決 A，帳本 :89-90）→ **bring-up 先用 ADVAN_Type1（`[GP-IB] Type=0`）或 16Bin，不要用 GS 與 256 bin**。

### 5.2 範例

**例 A：開 site 1～4**（Handler `Site[0..3]=1`）
- Fullsites：`data` 的 bit0＝site 1（`GpibTestGpib.cpp:451-455`）→ `Fullsites 0000000F\r\n`。
- Tester：`BINON:00000000,00000000,00000000,00003121;` → site1＝1、site2＝2、site3＝1、site4＝3。
- 橋接回：`ECHO:00000000,00000000,00000000,00003121;`（`BINON:` 後面那段原文，`GpibTestGpib.cpp:1296-1316`），`MyGPIBWrite` 補 `\r\n`（`GpibCore.cpp:1284-1290`；`bGPIBWriteWithout_r_n=1` 時不補）。
- Tester：`ECHOOK` → Handler 收到 `Result[0..3]={1,2,1,3}`，其餘 0；log `GPIB to Handler <== 0,0,…,0,3,1,2,1,`。

**例 B：開 site 1、2、17、32**
- Fullsites：`Fullsites 80010003\r\n`。
- Tester：`BINON:A0000000,00000005,00000000,00000021;` → site32＝10（A）、site17＝5、site2＝2、site1＝1。

### 5.3 會出錯的情況

| 情況 | 橋接 log | 結果 | 位置 |
|---|---|---|---|
| 長度不足 41 或逗號／字元錯 | `Tester ==> CHECK BINON STRING LENGTH ERROR`／`… CHARACTER ERROR - n` | 長度錯＝32 站全 9999、字元錯＝該站 9999；`StringLength` 開時不回 ECHO，直接以正常結束（return 1）把這些 9999 交 Handler | `GpibTestGpib.cpp:672-682`、`:1030`、`:1198-1212` |
| 關的 site 有 bin（0 與 10 例外：10 視同 0） | `Tester ==> CLOSE SITE HAS BIN ERROR` | 送 `MSG_CMD_CloseSiteHaveBin` → Handler **WAR07326**；Binon Echo 開時照樣回 ECHO | `GpibTestGpib.cpp:1040-1096`；`HandlerGpibMsg.cpp:1248-1251`；【H】`main.cpp:16332` |
| 沒送 0x41 就來 BINON | `Tester ==> BINON WITHOUT 0x41 ERROR` | `MSG_CMD_BinonWithout0x41` → **WAR07327** | `GpibTestGpib.cpp:1097-1113`；`HandlerGpibMsg.cpp:1252-1263`；【H】`main.cpp:16340` |
| 沒問 FULLSITES? 就來 BINON | `Tester ==> BINON WITHOUT FULLSITES ERROR` | `MSG_CMD_BinonWithoutFullsite` → **WAR07328** | `GpibCore.cpp:1104-1108`；`HandlerGpibMsg.cpp:1281-1297`；【H】`main.cpp:16372` |
| bin <0 或 >iBinSelect | `Tester ==> BIN IS <0 OR >MAX_BIN ERROR` | 開的 site 全改 16（iBinSelect≤15）或 256 | `GpibTestGpib.cpp:1129-1167` |
| Tester 回 `ECHONG` | `0600 LISTEN:ECHONG` | 第一次回 Task 400 重讀 BINON；第二次 error（全部 -1、`bError`） | `GpibTestGpib.cpp:1479-1497`；`GpibCore.cpp:1140-1160` |

### 5.4 Handler 端怎麼確認

- GPIB 分頁 32 站面板的 bin 與 Tester 送的一致（§4.5）。
- Handler 依配方 Site Map：`iBin[i][j]=Result[iSiteMap[i][j]-1]`；開著的 site 收到 bin 0 且 `bDutflag` → 257（`HandlerGpibMsg.cpp:1976-2004`）→ 先確認配方 Site Map 的編號跟 Tester 的 site 編號對得上。
- `asRecordTestResult`＝BINON 原文（橋接放在 `cReturn`，`GpibTestGpib.cpp:659`；Handler `HandlerGpibMsg.cpp:2108`），WAR07352 的訊息會帶它。
- Status.ShowBinSelect 的 `binsel.cat.counts` 跟著加：**待上機確認**（計數本體沒在本檔逐行核）。
- 32 站全開做一次（`Fullsites FFFFFFFF`），再做幾個關 site 的組合，確認不會誤報 WAR07326。

### 5.5 預期／失敗時看哪裡／停止與還原

- **預期**：每站 bin 正確、沒有 WAR07326～07328、ShowBinSelect 計數增加。
- **失敗時**：看 `mmoBINON` 的原文與 `0400 LISTEN:` 那行，對 5.1 的位置表；Site Map 對錯看 Handler 的 site 對照。
- **停止／還原**：同 §4.5。

---

## 6. RS232（Standard）

### 6.1 前置

- 配方 `[Mode] Tester Type=2`、`[RS-232C] Type` 0（Standard；1＝32Bin，最大 bin＝Bin Count＋1，否則 16：`Rs232HandlerMsg.cpp:498-506`、`HandlerBridgeCtl.cpp:817-820`）。
- **Handler 要 On-Line**：Off-Line 時 RS232／TTL 配方改走 GPIB 引擎的 simulate、不開 COM（使用者裁決，帳本 P2d :285-293）；而且 RS232 引擎在 Off-Line 下 `CE` 不回 site（`Rs232Parse.cpp:282`）。
- Setup.ini：`[SystemSetup] iTesterMode=0`（開機才會開 Tester COM，`Rs232Ui.cpp:2242-2334`）、`[COMPort] CommName`＝接 Tester 的 COM；格式由配方寫進來（§2.4 P6）。預設 9600／7／1／Even（Rs232 skill 的規格同，`D:\RS232Standard\.github\skills\rs232-standard-interface\SKILL.md`）。
- 起來時 RS232 log：`[RS232] Connect:OK <COMx>`（失敗：`Connect:FAIL`／`Tester COM setting is NULL!!`，`Rs232Comm.cpp:711-778`）、`[RS232] RS232 run with Handler`，並送 `MSG_CMD_AskArmTestMode`、`MSG_CMD_Version`（`Rs232HandlerMsg.cpp:138-154`；【R】`MainForm.cpp:633-634`）；Handler 送 TesterMode 之後 `… Change tester mode RS232 Standard.`（`Rs232HandlerMsg.cpp:705-741`；【R】`MainForm.cpp:1165-1196`）。第一次連上時 Handler 會不會送 TesterMode（它只在模式碼變了時送，`HandlerBridgeCtl.cpp:1073` 起）：**待上機確認**。

### 6.2 第一步：查詢

框：Tester 送 `[STX]<指令>[ETX]`（0x02…0x03），橋接回 `[ENQ]`(0x05)，Tester 回 `[ACK]`(0x06)，橋接才送 `[STX]<資料>[ETX]`（`Rs232Parse.cpp:139-195`、`:232-258`；【R】`MainForm.cpp:3159-3270`）。

| Tester 送 | 回覆資料 | V906 | golden |
|---|---|---|---|
| `CF` | 一律 `8`（golden 怪癖，不看 site 數） | `Rs232Parse.cpp:264-276` | 【R】`MainForm.cpp:3274-3286` |
| `CZ id?` | Gerneral.ini `[Version] Model`（預設 `HT-9046`，缺鍵會寫回） | `Rs232Parse.cpp:524-543` | 【R】`MainForm.cpp:3511-3527` |
| `CZ which?` | Gerneral.ini `[Version] Machine ID` | `Rs232Parse.cpp:544-561` | 【R】`MainForm.cpp:3528-3544` |
| `CZ status?` | 先向 Handler 要 `MSG_CMD_MachineState`，回 17 位元狀態的十進位 | `Rs232Parse.cpp:562-585` | 【R】`MainForm.cpp:3545` 起 |
| `CZ testerbin?`、`CZ all masstemp?`／`CB`、`CZ sitemap?`、`CZ jam?`、`CZ soaktime?` | 同樣先向 Handler 要值再回；格式照 rs232 skill，**待上機確認** | `Rs232Parse.cpp:586-705` | 【R】`MainForm.cpp:3545-3673` |
| `CD`、`CN` | 一律 `[STX][ETX]`（空；golden 怪癖，裁決 A 照留） | `Rs232Parse.cpp:706-745`；帳本 :151 | 【R】`MainForm.cpp:3674-3710` |
| `BARCODE?`／`GET2DID?` | 會回（裁決 B，刻意偏離 golden；格式同 GPIB） | `Rs232Parse.cpp:325-396`；帳本 :142 | — |

### 6.3 第二步：一個 cycle

1. Handler `RunTestProgram` → `MSG_CMD_NONE`：`iStart[]`，面板顯示 `T`，log `[Handler ==> RS232] … START TEST`（`Rs232HandlerMsg.cpp:482-525`；【R】`MainForm.cpp:942` 起）。
2. Tester `[STX]CE[ETX]` → 橋接 `[ENQ]` → Tester `[ACK]` → 橋接 `[STX]1,2,3,4[ETX]`（要測的 site 號，1 起算、逗號分隔；沒有要測的就 `[STX][ETX]`），送出後才清 `iStart`、記「已回 CE」（`Rs232Parse.cpp:239-258`、`:277-324`；【R】`MainForm.cpp:3249-3330`）。
3. Tester 測完送 `[STX]BA<site>,<device id>,<bin>;<site>,<device id>,<bin>…[ETX]`，例 `BA1,1,1;2,1,2;3,1,1;4,1,3`（`Rs232Parse.cpp:398-507`；【R】`MainForm.cpp:3385-3495`）：
   - bin 字 `A`～`F`（大小寫）＝10～15；bin ≥ 最大 bin 數 → 999（`:443-474`）；site 超出 1～32 跳過。
   - 沒收過 CE 就來 BA：`BA WITHOUT CE ERROR!!`，全部 999，送 `MSG_CMD_BinonWithoutFullsite` → Handler **WAR07317**（`:419-423`；`Rs232HandlerMsg.cpp:250-262`；`HandlerGpibMsg.cpp:1281-1297`）。
   - 關的 site 有 bin（0 與 999 以外）：`Closed Site Have Bin ERROR!!`，`bCheckClosedSiteHasBin=1` 時全部 999 並送 `MSG_CMD_CloseSiteHaveBin` → WAR07326。
4. 橋接 `SendResultFinish` 把結果交 Handler（log `[RS232 ==> Handler]`），再回 Tester `[ACK]`（`Rs232Parse.cpp:508-521`；【R】`MainForm.cpp:684`、`:3495`）。Tester 在 BA 後面是否另帶 `[ENQ]`（註解說有機台會黏在 BA 後面）：**待上機確認**。
- 逾時：Handler 同樣是 WAR07352（§4.4）；RS232 的 Retry 走 `ProcessTesterTimeOut` 最後那個 else 臂（`atester.cpp:4115-4140`，golden 同），實際效果：**待上機確認**。
- 只有橋接、不經 Handler 的測法：RS232 分頁勾 site → `Manual Test`（`click btManualTest`）：設 `iStart`、`bManualTest=true`，Tester 送 CE 就回這些 site，結果**不交 Handler**（`Rs232Comm.cpp:468-485`；`SendResultFinish` 要 `bManualTest==false`；【R】`MainForm.cpp:1688`、`:688`）。

### 6.4 第三步：多站 bin

- 用 6.3 的 BA 帶 8／16／32 站；Handler 端確認方法同 §5.4。bin log 在 `MemoBinData` 與 `D:\RS232Log\BinLog\`（§2.6）。

### 6.5 預期／失敗時看哪裡／停止與還原

- **預期**：`comm[0]` 連線旗標 true；查詢照 6.2 回；BA 之後 Handler 分 bin、沒有 WAR07317／07326。
- **失敗時**：`[H->T] Connect:ERROR Data can not send!`（`Rs232Comm.cpp:962-966`）＝port 沒開；格式不對時 `[T->H]` 會是亂碼（對 Setup.ini 與 Tester 的 Baud／Bit／Stop／Parity）；每一筆收發都在 `D:\RS232Log\LOG`（Hex 欄）。
- **停止／還原**：切 Off-Line（RS232 引擎關掉、改起 GPIB 引擎 simulate，不開 COM：帳本 P2d）；還原 Setup.ini 與配方。

---

## 7. TTL 板（DIO）

### 7.1 前置

- B1、B2、B3、B5 已在 `35d17d44` 接上（§1）。B1 的送出只在橋接視窗已找到（HVisionWnd 有值）時才送，同 golden `SendMessage(NULL)`（St02-E2 審查 A1，20260928 修）。
- Gerneral.ini `TTL_CARD_TYPE=2`（一塊板）先做；`3`（兩塊板）有 golden 的已知錯誤（板 2 的框寫到板 1 的 COM、板 2 的 bin 放到 site 1～4 的位置，裁決 A 照留、等實機再決定，帳本 :150）。`TTL_CARD_USE_ADDRESS` 決定框前要不要 `00`。
- 配方 `[Mode] Tester Type=0`、`[DIO] Type` 選對 DIO 檔；Setup.ini `[SystemSetup] iTesterMode` ≥1000（開機才會開 TTL COM，`Rs232Ui.cpp:2242-2262`、`:2337-2345`）、`[COMPort_TTL] CommName`；TTL 板固定 115200 8N1。
- **Handler 要 On-Line**（Off-Line 走 GPIB simulate、不開 TTL COM；而且 WSOTS 要 `bSimulate==false`，`Rs232HandlerMsg.cpp:553`）。
- 起來時：`[TTL1] Connect:OK <COMx>`（失敗 `Connect:FAIL`／`TTL1 COM setting is NULL!!`，`Rs232Comm.cpp:785-870`）。RS232 分頁勾 `cbShowLog_TTL`（§2.6）。

### 7.2 WINIT（初始化）

- 誰觸發：RS232 引擎在三個時機向 Handler 要 TTL 參數（`SendMSG_CMD(MSG_CMD_State_TTL)`）：收到 Handler 的 `MSG_CMD_TesterMode`（重開 TTL COM 後，`Rs232HandlerMsg.cpp:705-731`；【R】`MainForm.cpp:1165-1191`）、連上 Handler 約 3 秒後（`Rs232HandlerMsg.cpp:162-171`；【R】`:651`）、板子開機送 `@Runing`（沒收到 INIT 時每秒一次，`Rs232Comm.cpp:373-376`；【R】`:1598`）。
- Handler 回答：`HandlerGpibMsg.cpp:2134-2136` → `fMain->GetTTLState()` 組 48 碼（B1 已接：經 `TesterComm\TesterWndSeat.h` 送；橋接視窗還沒找到時不送），順序不能換（【V】`Command.cpp:11083-11263`；【H】`Command.cpp:10324-10500`）：

| DATA | 內容 | Handler 給的值 |
|---|---|---|
| [0] | TS+5V | 一律 `0`（關） |
| [1] | Bin 模式 | `0` 5BitBit、`1` 10BitBit、`2` 5BitBinary、`3` 10BitBinary（ASE_JP 模式 `4`）；由 DIO 檔的 bit 長度與資料型態決定 |
| [2-9] | SOT Active Logic（site1～8） | DIO 檔 Start Signal Logic＝0（正）→ `11111111`，否則 `00000000` |
| [10-17] | DATA Active Logic | Cate Signal Logic 同上 |
| [18-25] | EOT Active Logic | 一律 `00000000` |
| [26-33] | DUT Active Logic | DUT Type 0／1／2 → `11111111`，其他 `00000000` |
| [34-37] | SOT 寬度（ms，≤1000） | `%04d` 的 Start Signal Pluse Width |
| [38-41] | DUT 寬度 | 一律 `0200` |
| [42-47] | BIN 等待逾時（ms，0＝無限） | 一律 `000000` |

  DIO 檔的鍵：`[Start Signal] Logic／Pluse Width`、`[Cate Signal] Logic`、`[DUT Signal] Type`（【V】`FileRW\TTLCfg.gen.inc:257-268`）。
  （計畫 §1.3 寫「47 碼」少算一碼：DATA[0]～[47] 是 48 碼，引擎 `SubString(43,6)` 取到第 48 碼，`Rs232HandlerMsg.cpp:819`；規格 `D:\RS232Standard\.github\skills\rs232-ttl-communication\SKILL.md:113-128` 同。）
- RS232 引擎收到後逐欄記 log（`Bin Mode is 5 BitBit`、`SOT Active Logic is Positive`、`SOT Width is <n>ms`、`BIN Time Out is <n>ms`…），組 `@WINIT<48 碼>`（有站號或兩塊板時 `@00WINIT…`，板 2 `@01WINIT…`）＋CRC16-Modbus 2 byte＋`#`，**TTL1 連上才送**（`Rs232HandlerMsg.cpp:748-915`，`:825-852`、`:891`；【R】`MainForm.cpp:1208-1376`）。CRC 是 `crc_chk`，ctest `TesterComm_RS232` 驗過標準檢查值（帳本 :131）。
- 板子回：把收到的框 echo 回來（引擎只記 log，`Rs232Comm.cpp:365-368`）＋`@VERS <版本>`（規格 :130）；版本經 `MSG_CMD_Version`（`bOneCycle=true`）交 Handler 比 `TTLRS232VerCheck`（`HandlerGpibMsg.cpp:869-881`；**B3**）。

### 7.3 WSOTS（開測）

- 意思：`W`＝寫、`SOTS`＝「發送 SOT 給 Tester」，DATA[0～7]＝site1～8，`1`＝發送、`0`＝等待（規格 `rs232-ttl-communication\SKILL.md:145`）。
- 程式：Handler 的 SOT（`MSG_CMD_NONE`，`Site[]`）→ 引擎把 site1 放最左組成 8 碼 → `@WSOTS<8 碼>`（有站號 `@00WSOTS…`；兩塊板 `@00WSOTS<site1-4>0000`＋`@01WSOTS<site5-8>0000`），全 0 就不送；後面接 CRC＋`#`（`Rs232HandlerMsg.cpp:482-650`，`:553`、`:570-598`；【R】`MainForm.cpp:942-1100`，`:1013`、`:1030`、`:1058`）。
  - 例：開 site 1、2（一塊板、不帶站號）→ `@WSOTS11000000`＋CRC＋`#`。
- DIO 線：TTL 板依 WINIT 的邏輯與寬度，對 site1～8 的 SOT 線打脈衝；Tester 回 EOT／BIN（Bit Bit 或 Binary）、DUT 線由 DUT 寬度控制；接腳對照不在程式碼裡：**待上機確認**（看 TTL 板的規格書 `D:\RS232Standard\.github\skills\rs232-ttl-communication\references\20180822_TTL Communication with IPC by RS-232.pdf`）。
- 只有橋接的測法：RS232 分頁勾 site → `btnTTL_Manual`（送同樣的 `@…WSOTS…`，`Rs232Log.cpp:587-727`；【R】`MainForm.cpp:2703-2838`）；`btnClearSot` 送 `@WCSOT00000000`（有站號 `@00WCSOT…`、板 2 `@01WCSOT…`）清 SOT（`Rs232Log.cpp:728-780`；【R】`:2839-2913`）。

### 7.4 結果與錯誤

- 板子回 `@RBIN<8 byte>`＋CRC＋`#`，共 16 byte（帶站號多 2）；8 個 byte 是 site1～8 的 bin **原始值**（0x00＝無 bin、0x01＝bin1…，規格 :147）→ `Result[]`，兩塊板都收完才 `SendResultFinish`（`Rs232Comm.cpp:242-360`；【R】`MainForm.cpp:1473-1590`）。
  - 長度不對：`[TTL Board1] RBIN data length error!`，開的 site 999（`Rs232Comm.cpp:247-257`；【R】`:1479`）。
- 錯誤框（`Rs232Comm.cpp:377-425`）：`ErrSOF` 第一字錯、`ErrCRC` CRC 錯、`ErrEOF` 結尾錯、`ErrCMD` 指令不支援、`ErrSOT` → `Double SOT error!`：下一個 RBIN 當作上一次的結果丟掉（`Last Time Test Result to Ashbin!`）並重送 WSOTS（`:261-298`、`:402-405`；【R】`:1626`）、`ErrDAT`、`ErrSIT` 關的 site 有 bin、`ErrBIN` Bit Bit 模式一站兩個訊號。
- 板子 3 秒沒回（Timer1 300 ms×10）：送 `MSG_CMD_Version "Time Out!! No get TTL Board1 Reply!"` → Handler 跳 `TTL RS232 Board1 Not Reply!!`（`Rs232Comm.cpp:531-545`；`HandlerGpibMsg.cpp:839-856`；【R】`:1736-1750`）。
- 逾時：Handler WAR07352；Skip／Retry 會送 `@WCSOT`（B5 已接）。

### 7.5 預期／失敗時看哪裡／停止與還原

- **預期**：`[TTL1] Connect:OK`；State_TTL 那幾行參數與 DIO 檔一致；板子 echo `WINIT`、回 `VERS`；`WSOTS` 後收到 16 byte 的 `RBIN`；Handler 分 bin。
- **失敗時**：沒有任何 `WINIT` → B1（或 TTL1 沒連上）；參數全 0 → B2；`TTL RS232 Version Error!` → B3 或板子韌體太舊；`Should get TTL1 board address 00…`／`Should not get TTL1 board address…` → `TTL_CARD_USE_ADDRESS` 跟板子不一致（`Rs232Comm.cpp:203-240`）。
- **停止／還原**：`btnClearSot` 清 SOT；切 Off-Line（TTL COM 會關、改 GPIB simulate）；還原 Setup.ini、DIO 檔與配方。

---

## 8. 風險與注意

1. **真卡、真 COM、真 TTL 線會驅動真設備**：GPIB SRQ 會讓 Tester 開始測；TTL 的 SOT 線直接打到 Tester；RS232 CE 回 site 就是開測。DIO 設定頁存檔在出貨組態會跑 `InitDIOStstus`，直接設 `SW[TTL_StartData]`／`SW[TTL_Dut]`／`SwClear2`／`SwClear6` 這些 DIO 輸出（【V】`cDIOStatus.cpp:54-76`）。
2. **不要在客戶量產的 Tester 上做，除非客戶在場並同意**；先用 Tester 的測試程式或模擬器，Tester 不要載量產程式。
3. 第一步只送問句（§3.2 ⚠）；`SETTEMP +` 會改機台溫度、`SETTESTOFFSET_` 會寫配方而且會累加（W9）。
4. **網頁指令沒有權限閘**（B7）：上機期間只讓在場的人開 testercomm 頁；`Manual Start`／`Manual Test`／`btnTTL_Manual` 一按就打到設備（同鍵 400 ms 內重複會被丟掉，【V】`TesterComm\Handler\TesterCommWiring.cpp:224-233`）。
5. **B4 已接：bridge 沒找到或版本錯時，START 會照 golden 拒絕**（`WebStart.cpp`）。⚠ 設了 `HT9045_TESTERCOMM=0`（不開橋接）時，GPIB／RS232 On-Line、TCP/IP Off-Line、TTL_CARD_TYPE 2／3 的 START 都會被擋（筆電的 TTL_CARD_TYPE＝2）。
6. 會寫量產機共用檔：`D:\HT9045\system\Gerneral.ini`（兩個橋接缺鍵寫回）、`D:\GPIB9045\system\general.ini`、`D:\RS232Standard\System\Setup.ini`、配方（§2.5）。**先備份**。這台若同時跑 BCB 的 H9046_32GPIB.exe／RS232Standard.exe，兩邊共用這些檔；V906 與 BCB 橋接不能同時跑（裁決 5：BCB 接 BCB、C++ 接 C++；同一張 GPIB 卡、同一個 COM 只能給一個程式）。
7. Off-Line 不代表離開匯流排（§2.2）；要完全不碰 GPIB 就拔線或 `HT9045_TESTERCOMM=0`。
8. 不要用 16BinGS／32BinGS／256 bin 當第一個 bin 格式（§5.1）；不要先做兩塊 TTL 板（§7.1）。
9. **7016／7017 TCP 指令伺服器是另一件事**：它是 Handler 當 server 收 MES／AMR 的 `HTGR／HTSET`，能遠端 Start／Pause（`tcp-command-server-7016.md`）；出貨組態下它照 golden 客戶碼開真 socket，bring-up 時不要跟 Tester 通訊混在一起測，也不要讓測試網段連得到它（`HT9045_TCPCMD_SIM=1` 讓它維持 Sim，`TesterCommWiring.cpp:116`）。
10. 橋接的 2 秒阻塞（`MyGPIBWrite`）、`SleepEx(1000)` 只卡通訊執行緒；如果看到機台 tick 變慢，是違反裁決 11，要回報（計畫 §2.1 第 7 條、P3 驗收）。

---

## 9. 待上機確認（彙整）

| # | 項目 | 章節 |
|---|---|---|
| U1 | wb_serve 是 32 位元、配的是 32 位元 `gpib-32.dll`；NI MAX `GPIB0` 的設定會不會被 `ibrsc`／`ibpad`／`ibtmo` 完全蓋掉 | §2.3 |
| U2 | 切 On-Line 用哪個網頁入口（B6） | §1 |
| U3 | Off-Line 時問句照樣會回 | §3.4 |
| U4 | `SETSOAK?`／`SITEMAP?`／`ID?` 的回覆格式 | §3.2 |
| U5 | serial poll 是否發生（NI I/O Trace） | §4.3 |
| U6 | Manual Start 之後 Handler 留下的狀態是否影響下一個 cycle | §4.1 |
| U7 | ShowBinSelect 計數是否跟著加 | §5.4 |
| U8 | RS232 第一次連上時 Handler 是否送 TesterMode；BA 後面是否黏 `[ENQ]`；`CZ *?` 各格式；RS232 逾時 Retry 的實際效果 | §6 |
| U9 | TTL 板 SOT／EOT／BIN／DUT 的接腳對照 | §7.3 |
| U10 | 兩塊 TTL 板的 golden 錯誤在實機的表現（裁決 A 等實機） | §7.1 |
| U11 | GPIB 883 樹的行號（本機沒有 883） | §0 |
| U12 | MachineStatus 的送出（Command.cpp:15249，GATE #6）跟 B1 同一個「沒有 HVisionWnd」理由；可以用 B1 的 seat 接，但不在這次範圍（St02-M 0928） | §1 B1 |
| U13 | TTLLog 還閘著（cpublic.cpp 的 TTLLog 本體 `#if 0`），InitDIOStstus 的 TTL_Signal_LOG 檔還不會寫（不在這次範圍） | §1 B2 |
