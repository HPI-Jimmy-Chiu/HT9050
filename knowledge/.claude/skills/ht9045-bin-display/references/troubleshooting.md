# Bin 顯示器排錯

路徑記號同 `SKILL.md` §0（只寫 `檔名:行`＝golden 912 根目錄；`machines\` 在 repo 根目錄）。先分清楚看的是**實體顯示器**還是**畫面的 Bin Display Status 分頁**（tsUnloadMap）：分頁顯示的是「顯示器 ack 過的值」，不是要送的值。

## 0. V906 上全部灰底「X」

不是故障：main 上 `HSys.BinDisCtrl` 在 wb_serve 是 NULL、Timer1Timer 是空殼、ChangeBinDispStatus 被 GATE D1～D6 閘成一律灰「X」，網頁分頁是靜態「---」。見 `port-status.md` §1。St02 的 C14 bring-up 合併前都會是這樣。

## 1. 顯示「X」

| 樣子 | 原因 | 查什麼 |
|---|---|---|
| 灰底「X」 | 該格 `UnitHasInstall=false`（`cShowBinSelect.cpp:365-369`） | ① InstallColorBinDisplay 本來就不裝：`AUTO3_IS_MAGAZINE=0` 的 Mag、`AUTO_EMPTY_COLOR<3` 的 Auto4～6／Fix7～12、BulkBox、`AUTO_EMPTY_COLOR=0 && SUPPORT_2_EMPTY_EMPTY=0` 的 Empty／Color（`database.cpp:1707-1732`）；② 開機讀版本失敗超過 5 次被移除（`MyBinDisp.cpp:2787-2808`；G16 關時每一格都會被移除，G16 開時只移除 Loader／Empty／Color）⇒ 線、電源、站號、COM 埠、鮑率 |
| 橘底「X」 | `GetBinNow` 是 -1／123：這個盤沒有分配任何 bin（`DoShowBinDigital :1394-1396` 填橘） | 正常；要它顯示就去 Bin Select 設定把 bin 分給這個盤 |
| 全部紅「X／0」 | SPIL 客戶碼離線（`:1399-1415`） | 客戶分支，V906 照 S25 不做 |
| lblX 顯示「X」 | ShowBinSel：沒有 Magazine 時的 Mag（type 3 橘、其他灰，`:749-766`）、非 FixTrayMode 多出來的 Fix（`:768-786`） | 正常 |

## 2. 顯示「0」或紅「0」

- 紅「0」＝ `iColorNow=1`、`iBinNow=0`：COM 掉線重設後的初值（`MyBinDisp.cpp:464-468`、`:479-483`），之後要等顯示器 ack 才會換。
- **906 的 TFT（type 4）永遠紅「0」**：906 的 TFT 路徑從不更新 `iColorNow`／`iBinNow`；912 的修正 E（`WriteTargetBin :651-655` 等）才會跟著變。以 912 為準就不會有。
- type 3 一直紅「0」⇒ 顏色／bin 的 ack 沒回來或比對不到（`DoStartSetColor :2493-2526`、`DoStartSetBin :2133-2220`）；開 C14 看 BinDisplayLog 的回覆。
- 實體 TFT 顯示「000」：bin 0 是合法的 bin（`%03d`，`:941`）。

## 3. 顏色不對

- 紅／綠＝`Prod.iIsFailT6[j]>0` 才紅（`DoShowBinDigital :1320-1326`）⇒ 看 Bin Select 的 pass／fail 設定。Fix2／3 Link 時顏色跟 Fix1（`:1330-1342`）。
- 橘＝沒用到的盤，或 Loader／Empty／Color。
- 藍（912、只在 TFT）＝AOI fail 盤（Top/Bottom AOI，`:1346-1392`）。type 3 沒有藍色（三色 LED）。
- 黑紅每秒交替＝該格錯誤（`GerErrNow`），見 §4。
- Auto1／Auto2 紅黑快閃＝P66 換盤警示（`bP66AutoChangingFlashWarn`），換盤完成 `ClearAutoChangingWarn` 會恢復；沒恢復就看 `acatchtray.cpp:7431-7432`／`csystem.cpp:7647-7648` 有沒有走到。
- 分頁顏色和實體不一樣：分頁顯示 ack 過的值；同一顆 pnlX 也被 ShowBinSel 用 `tcBinColor[]`（9 色）寫，最後寫的贏。分頁沒開時 ChangeBinDispStatus 不畫（除非 G16 或 KYEC／AMD）。

## 4.「Bin display got error!!」與通訊錯誤訊息

- 狀態列紅底「Bin display got error!!」＝錯誤掃描找到任一格：`GerErrNow`，或「i>=3 而且沒裝」（`cShowBinSelect.cpp:244-273`）。所以 Auto1～3、Fix1～6 被開機讀版本移除時也算錯誤；Mag（沒 Magazine）、BulkBox、Auto4～6、Fix7～12、AMR 的 Loader～Auto2、912 AOI 的 Fix1～6 不檢查。
- 偉測（VTEST）沒裝 Auto1～3 顯示器 ⇒ 一直有錯；912 讓它不強制跳頁（`:291-292`）。
- 訊息：
  - G16 開：「Please check bin display. It have communication error! Error part: …」／「請確認Bin顯示器的狀態! 異常位置: …」，60 秒一次，會 StopComm＋重新初始化（`:385-406`）。
  - KYEC_LEE／AMD：「Please check bin display. It have communication error!」／「請確認Bin顯示器的狀態。」，每個 one-cycle 一次（客戶分支）。
- 912 的 State Record「BinDisplay Diag」區（`main.cpp:27132-27172`）：每格 Inst／Err／**Ver**／Bin／Color。`Ver=0`＝從沒讀回版本（沒裝，或 COM 根本沒開）；`Ver≠0`＝曾經通過、後來斷線——兩種現場處置完全不同。

## 5. COM 開不起來／沒有 COM

依序查：
1. `NUMBER_PANEL_TYPE` 是 3 或 4？不是的話根本沒有 BinDisCtrl。
2. SIM 建置（`SOFT_SIMULTE`）永遠不開埠（`MyBinDisp.cpp:363-447`）。
3. `InitialOK`：golden 在 FormShow 設（`main.cpp:10919-10921`）；沒設 Timer1Timer 每拍直接 return（`MyBinDisp.cpp:299-300`）。
4. `bStopProcess`：cBinSel 開著時會 `ProcessStopStart(false)` 暫停（`cBinSel.cpp:1757-1759`），下一次 DoShowBinDigital 才 `ProcessStopStart(true)`。
5. `ComPort` 是空字串 ⇒ case 1 每拍安靜地 return（`:335-339`）；database 只補「COM」前綴，不補空值（`database.cpp:536-544`）。
6. BinDisplayLog（`D:\HT9045_Log\BinDisplayLog`，開關埠的行不管 C14 都寫）：
   - `BinDisp, Start Comm OK., \\.\COMx`：開成功。
   - `BinDisp, Stop Comm OK., …`（開機就出現）：`GetCOMPortStatus` 失敗＝埠不存在或被別的程式佔住（另一個 Handler、終端機工具、BinDispTester）。
   - `Start Comm NG!` 加畫面「Error open com port」：StartComm 丟例外（`:375-381`）。
7. USB 轉 RS-485 重新編號或拔插：case 100 會偵測（`GetCOMPortStatus` 又成功＝handle 掉了）並重新初始化（`:458-489`）。

## 6. 接錯埠

- `[NUMBER_PANEL] COM_PORT` 給 CommBin（全部 TFT、legacy 大部分），`[NUMBER_PANEL2] COM_PORT` 只給 legacy 的 Fix1～12 且 `AUTO_EMPTY_COLOR>=3`（`hardware-and-protocol.md` §4）。
- ⚠ **Timer1Timer case 1 不管型態都會試著開 CommBin2**（`:404-423`）：`GetCOMPortStatus(ComPort2)` 成功就獨佔那個埠。
- HT9050 要確認（推論，未上機）：PC 的 COM1＝BIN、COM2＝Multi Bin、**COM4＝通訊面板 ELC-001**（`ht9050-hw\references\hardware-overview.md:101-104`）。模擬組的 `[NUMBER_PANEL2] COM_PORT=COM4`（`machines\HT9050\sim_9378\Gerneral.ini:600`）照搬到機台的話，Bin 顯示器會在開機時把 COM4 搶走。golden 的 `[IndexDriver] COM_PORT` 預設也是 COM1（`database.cpp:521`），兩邊都用預設會撞到 BIN 埠。真值等第 127 包（W-14）。
- 症狀：Start Comm OK 但每格都被移除或報錯（沒有回覆）；或別的裝置開始收到亂碼。
- 站號：legacy 的站號是 `Addr+0x20`／`+0x26`（`hardware-and-protocol.md` §5），TFT 用 `iAddArrayTFT[]`；顯示器本身的站號設定要對上它的位置。

## 7. 紀錄去哪裡看

- `D:\HT9045_Log\BinDisplayLog`：開關埠一定寫；逐封包（`Send, [3A] [20] …`／`Recv, …`）要 `bC14SaveBinDisplayLog=1`（`MyBinDisp.cpp:741-760`）。
- State Record 的 BinDisplay Diag（912 才有）。
- EventLog：BinDisplay 模組除了 catch 例外，不寫 EventLog（912 `main.cpp:27133-27135` 的說明）。
