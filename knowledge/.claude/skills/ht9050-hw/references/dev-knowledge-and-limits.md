<!-- AI(W906-IFOR-CLAUDEMD) 20261002: moved verbatim from CLAUDE.md (Jimmy 1002 22:4x, NIGHT_REPORT s0 #57 = A, RULINGS_20261002 #23); author Ifor01 (MR !110), first merged by Steven 1002 14:55 (d732d0c7). -->

## HT9050 開發知識與極限規則（Ifor01 20261002）

> 只收 20261002 Ifor01 session 量過、有出處的事實（I-01／I-03／R126／W-05；MR !102／!106／!109）。細節與驗證紀錄在 `v906/ifor-handoff` 的 `FROM_IFOR.md` §2。

### 1. 機型架構與通訊
- HT9050＝golden `Type_HT9046_LS`＋PCIE-1203（`IO_CARD_TYPE==PCI1203_IO`）；`CUSTOMER_CODE=0`＝`CC_HONPREC_QC` ⇒ Timer2 加熱段的 QA 分支在「Configuration 停在溫度通訊頁」時生效。頁面判斷一律：`W906_FormShowing("fConfiguration",false)`＋`FileRW_ProxyPageIndex("TfConfiguration","PageControl1")==1`（`ecp1TabSheet9`=1=`tsTempComm`；冷卻風扇 H1-08 同一套）。
- 溫控：HT9050 全機台達 DTM（Ethernet）。golden DTM 路徑 `DoSetSVOfDTME08` 只管 Index 加熱器（`iTempCode[]`）；Hotplate/Shuttle/DUT/Chamber 走序列 `DoThermo` ⇒ 需 I-03b「DTM 站/CH↔`eTempControll`」通道表（待 ES02 E-05）。DTMN08 暫存器＝DTME08 的一站（`Hx###`，x＝內部站號：主機 0、DTMN08 旋鈕 1～F），golden `x*0x1000` 可定址。
- Index Z 扭力：HT9050 走 1203（CiA402 60E0h/60E1h），不開 RS232 扭力埠（`rs232.cpp:263/:265`）。
- 序列溫控＝golden TCOM2 `Comm2`（rs232.dfm:8）：`[TempCtrl] HEATER_CTRL_TYPE`（預設 KT4H）、`[TempCtrl] COM_PORT`（預設 COM2）；9600 8N1、`ReadIntervalTimeout=100`。封包：KT4H Modbus ASCII＋LRC（`:AA FF CCCC VVVV LRC CRLF`）；TMC401 Modbus RTU＋CRC16（0xA001、init 0xFFFF）；E5DC CompoWay/F `STX cmd ETX BCC CRLF`（BCC＝XOR(cmd)^ETX）；DTK4848 ASCII、寫 strlen+1（含 NUL，golden 照留）。
- 新 ATC（`eNewATCSystem`）連線在移植樹無人驅動：全域 `ATC_InterfaceForm`＝替身（只有 `iATC_MODE_TYPE`）、`NetATCTimeTimer`（uLotInfo.cpp:8558，200 ms）未翻；ATC 暫緩（RULINGS_20260927 #6）。`ATC/ATCInterface.cpp`＝舊介面，其 `ATCWatchTimerTimer` 無呼叫者。
- 節拍：PumpTick 500 ms（不可調快）；Timer2 1000 ms；golden THeaterThread 20 ms ⇒ 出貨加熱 tick 掛 serve loop 共用快時鐘（RULINGS_20261002 #7，筆電做掛點）。

### 2. 參數極限／閾值／保護
- 序列溫控逾時：`Com2Delay` 0.5 s/次、`MAX_RETRY=2`、`CommunCTErr>5` ⇒ `UN150Read=999`＋`UN150CommError`（無回應約 7 s）。單次 `DoThermo` ≤約 1 ms（不阻塞；寫入走 vclcompat 寫入執行緒）。
- Hot 模式開機第一輪：golden 對每個通道寫 SV（未裝＝0，bthermo.cpp:1489-1491），每個無回應位址等 0.5 s ⇒ 控制器少時第一輪 30 s+（照 golden）。
- 開機開埠失敗不阻塞：RS232Init 在 InitialOK=true 之前，`ShowMyMessage` 只記 Exception（golden mymessbox.cpp:765）；SOFT_SIMULTE 另略過含 " port error" 的訊息。
- Timer2 加熱段守衛：`InitialOK==false` return（Exit 時 MainClose 清 InitialOK）、`SystemStart` 時不跑、1000 ms 限速容許 250 ms。
- 溫度 offset 上限：`CheckTempOffset`（WAR15194，`forms/fHS.cpp:804`）；全域 `FormHS` 被替身佔用 ⇒ 用私有 `TFormHS` 實例。
- 網頁鍵盤：`edAmbTemp` 10～50（23～30 只屬 CC_ChipMos_ZHUBEI）。
- ATC 校正檔：906＝單一 `DefineTemp\Temperature_ATC.Data`；910/912＝Hot/AmbientHot 用 `_ATC.Data`、其餘用 `_ATC_Cold.Data`（缺檔時從 `Temperature.Data` 複製）。移植樹 C 路（網頁）＝912、`uTemp_Set.cpp`＝906（統一待 Jimmy，NIGHT_REPORT §0 #47）；V899 ATC 機升級時把 `_ATC.Data` 複製成 `_ATC_Cold.Data`（G-031）。
- SetTemp（I-01c 起活）會寫：配方 `Temperature.Data`（`[Mode] Temperature`／`[Time] Soak`，"0.0" 格式）、`DefineTemp\Temperature*.Data`（全機共用）、`config\ATC.ini`、`config.ini`、`LastSet.ini`、`Gerneral.ini [Version]`、`*.MD5`；存完讓網頁溫度頁失效（下次網頁存檔回 409「重開頁」）。C 路 `PageSave` 在沒開過頁時一律 409。

### 3. 開發與驗證固定規範
- 編輯：同一行改、行數不變；**附加的程式碼要放在該行既有 `//` 之前**（rs232.cpp:178 曾被吞進註解）。
- 工具陷阱：Bash heredoc 會把反斜線減半 ⇒ Python 用 Write 寫、反斜線用 `chr(92)`；`sed -i` 會吃 CR；`core.autocrlf=true`（index LF、工作樹 CRLF）；`build.bat` 的 `V906_CTEST_ARGS` 不可含 `| & ^`；Bash 不可遞迴 grep（用 Grep 工具）。
- library 分層：`cpublic.cpp`（ht9045_globals）在 COM2（forms）之下 ⇒ 用指標縫（`g_pCOM2Comm2`、`g_pDTKComm`，由 TCOM2Shim 建構子設）；sm 不可直接呼叫 FileRW（只連進 wb_serve）⇒ 用函式指標 hook，由 wb_serve 裝；非 wb_serve 程式靠 `FileRW/_fallback.cpp`。`test_ga1_cprod` 自己編原始碼 ⇒ 新依賴要加進它的清單。
- vclcompat `TComm`：ctest 用 `SetSimMode`／`SimTxBuffer`／`SimInjectReceive`；未開埠時 `WriteCommData` 回 false；`StartComm` 開不了會靜默轉模擬 ⇒ 要看 `IsSimMode()`。SPComm 語意：讀取執行緒只排隊，消費端安靜滿 100 ms 才整包交付。
- ctest 沙盒：一律 `W906TestInsideCtestRoots`；`DataPath/DefaultPath/OffsetPath/LastDataPath` 執行期指到 %TEMP%；`AuthPath`／`asGeneralPath` 由 env 指到 build tree 下的 scratch（判斷用「是 scratch」，不是「不在 D:\HT9045」）；路徑縫在靜態初始化時讀 env；ENV-ALL 由 `cmake_language(DEFER)` 套用到全部測試；用到 `WriteIniDataGeneral` 要先 `OpenGeneralIniFile()`；寫死的機台路徑（例 `D:\HT9045\Config\ATC.ini`）改走 `AuthPath` 等縫。
- 計時測試：Windows 的 `Sleep(2)`≈14 ms ⇒ 搭配牆鐘計時器的迴圈用 `Sleep(0)`。
- 驗收：兩組態全新 gate（`V906_CTEST_ALL=1`）；基準＝出貨 4 項（config_db、ini_helpers、config_loaders、GA1_ReadGeneralIni）、模擬 19 項（再加 15 項 SOFT_SIMULTE 相依）；`dfm2rc_idempotent`＝工作樹 ir_out 是 LF 的本機問題；`MyDB_P4_Containment` 會被同時在跑的 HT9045.exe 寫 log 觸發；`NoteJamCount` 偶發 BAD_COMMAND（重跑）。解閘一律做**變紅檢查**（舊閘放回去，測試要紅）。
- 真實檔：`tools/realfile_guard.py snap/check/restore/drop`；已存在的 snap 不會被覆蓋；MD5 目標是動態的（舊 MD5 要手動還原）；**測試只經 ctest 跑**；gate 前先查有沒有 HT9045.exe（父行程 bcb.exe）在寫 `D:\HT9045\system`，有人在跑時不還原。
- golden 基準：906_0618（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`，cp950）；C 路 FileRW＝912 的翻譯；RULINGS_20260926 Q2「底層照 906、畫面照 912」；要偏離 golden 先問 Jimmy。
- MR：改寫自己的 WIP 分支用 `--force-with-lease`；依賴未合的 MR 要在標題寫「請先合 !NNN」；`tests/CMakeLists.txt` 檔尾衝突兩段都留。
