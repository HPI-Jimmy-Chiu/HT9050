# 評估：在 Steven01 這台用「先整夾備份、跑完逐檔還原」的方式實跑網頁程式，把還沒跑過的網頁自動測試補跑完

> 讀者：Steven（決策）、ST01-E（派工與登記）。撰寫：ST01-E 派的工程師，20260927 22:xx。**只讀研究：沒有改任何程式、沒有 build、沒有啟動網頁程式。**
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6` 的 commit `c20bdec2`（c20bdec2aeae22a8307de6bae2cdb275f25bdbca；那顆改的主要檔是 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`）。
> 程式行號都是這一顆的（用 `git -C D:\HT9045 show c20bdec2:<路徑>` 讀）。寫完前重查：分支已走到 `64ade3b7`（20260927 22:02，主要檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`）。
> `c20bdec2..64ade3b7` 沒有動到 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp`，也沒有新增寫死路徑；
> 但 `64ade3b7` **多了一支沒跑過的探針**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_cc_probe.py`），已補進 §1.3 第一組 ⇒ 第一組 9 支、第一＋二組共 13 支；其餘結論不變。
> 三棵樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝**移植樹**（C++，唯一可改）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝**golden V912**（BCB6 原版，唯讀）；`D:\HT9045\web\` 開頭＝網頁。
> 寫法照 Steven 20260927 21:xx 規則：檔案一律絕對路徑；先講白話，代號放括號。

---

## 0. 一句話結論

建議 **B（不改任何人的程式，備份→實跑→逐檔比對→還原）**：在 Steven01 找一段「這台沒有別的 build、測試、網頁程式在跑」的時間，把會被寫到的機台資料夾整夾複製備份（約 580 MB，1～3 分鐘），
啟動**模擬版**網頁程式跑自動測試，跑完逐檔比對、改過的還原、新產生的刪掉，再比一次確認全部回到原樣。分四輪、每輪都還原；**第一輪只開機不測試**，
確認這台開機改到的檔跟筆電量過的清單一樣，出現清單外的檔就停下來回報。這要 Steven 同意（題目全文在 §8），而且執行當下跳出的權限提示要 Steven 本人按允許。

---

## 1. 背景／現況

### 1.1 名詞（白話）

- **網頁程式**（`wb_serve.exe`，原始碼 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`）：瀏覽器畫面背後的 C++ 機台程式。它照 BCB6 原版（golden）開機、讀設定、關站，
  所以會讀寫這台電腦 `D:\HT9045` 底下的設定檔、配方、計數檔——跟量產機同一套檔名與位置，下面叫「**真檔**」。
- **網頁自動測試**（探針，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\*.py`）：模擬操作員在網頁上開頁、點按鈕、存檔，檢查 C++ 回的東西與寫出的檔對不對。每一支都要一個正在跑的網頁程式。
- **模擬版**：建置時開了「軟體模擬」（`SOFT_SIMULTE`）的版本，所有硬體動作都是假的。這台沒有 Advantech 馬達卡的開發套件（`C:\Program Files (x86)\Advantech` 不存在），
  所以網頁程式不會去開馬達卡（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt` 第 3476～3492 行，找不到套件就不連）。
- **啟動參數**：`--port N`＝網頁連哪個連接埠（預設 8045）；`--root 資料夾`＝網頁檔案放哪裡（預設 `D:\HT9045\web`）；`--seconds N`＝限時執行，N 秒後自己走正常關站（沒給就一直跑到按 Ctrl-C）。
- **全量 gate**：兩種組態（模擬版、出貨版）整套建置＋整套 C++ 單元測試（ctest）。ctest 也會讀這台的真檔，所以跟實跑網頁程式不能同時做。
- **測試用轉向開關**（環境變數，名字都是 `W906_` 開頭）：沒設＝照原版路徑；有設＝把某一個檔或資料夾改到暫存位置。只給測試用，正式啟動器會先把它們清空
  （`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md` 第 34 行，第 20 題 A）。

### 1.2 為什麼這台一直沒跑

1. **規則**：「Steven01 這台不跑網頁程式，除非 Steven 同意；它會改機台真檔」——
   `D:\HT9045\.claude\skills\ops-ht9045-proxy-build\SKILL.md` §3、`D:\HT9045\.claude\skills\ops-ht9045-proxy-build\references\gotchas.md`、
   `D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md` §5（測試開關表）與 §6（這台怎麼驗）。
2. **歷史**：20260924～20260926 早上，St01 在這台跑過探針（備份→跑→還原）。例：`008db55e`（20260925 15:25，主要檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBuilder.cpp`）
   的 commit 訊息寫「regression 21/21 PASS … restore IDENTICAL」；`0609a14f`（20260926 08:48，主要檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTowerLight.cpp`）寫「Regression: 34 items」。
   20260926 上午 Steven「暫時先不要 build」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` 第 168 行，S51）之後只做語法檢查、不實跑；
   20260927 Steven 放寬成「沒其他事可做時可以 build」（同檔第 393 行，S159），但實跑網頁程式仍要 Steven 同意；20260927 準備「備份→跑→還原」時被權限規則擋下（`D:\HT9045\.claude\skills\ops-ht9045-proxy-build\SKILL.md` §3）。
3. **以前的「假寫入」模式已經取消**：舊參數 `--dry`（把設定檔複製到暫存再改）20260923／20260924 退場，現在帶 `--dry` 啟動，網頁程式印一行說明就結束（結束碼 2，
   `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 3683～3684 行）。開機橫幅寫明「寫入一律去真檔，保護方式是備份→驗證→刪備份」（同檔第 3736～3750 行）。
   ⚠ 兩支新探針的「用法」還寫著 `--dry`，照抄會讓網頁程式一開就結束：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_open_gate_probe.py` 第 38 行、
   `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_closetail_probe.py` 第 64 行（同檔第 37、53、57 行的「--dry 導得到／導不到」說明也過時）。

### 1.3 還沒跑過的探針（commit `c20bdec2` 時）

「最後一次實跑」依 commit 訊息判斷（「not run」「not run in a browser」「NOT e2e-tested」「no build」＝沒跑）。

**第一組：寫好後一次都沒跑過（9 支；第 9 支是寫完前重查時新出現的）**

| 探針 | 加進來的 commit（主要檔） | 白話：它在驗什麼 | 會不會寫真檔 | 要準備什麼 |
|---|---|---|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\status_countersel_probe.py` | `0b1f4204` 20260926 12:13（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig_CounterSel.cpp`） | 「計數器顯示選項」頁開頁時，每個勾選框是不是等於設定檔裡的值；改一格存檔，檔案只能差那一行，最後點回原值 | 加 `--write` 才寫 `D:\HT9045\config\config.ini` 的 [Visible] 段 | 真的瀏覽器（Edge，這台有 `C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe`） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q3_dio_owner_probe.py` | `3ee547e5` 20260927 11:07（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`） | DIO 介面設定檔只能從它自己的設定頁存；從通用的「存系統檔」入口送要被擋 | 不寫（送的都是空內容） | 無 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_probe.py` | `76058840` 20260927 15:23（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp`） | 在「加熱盤」「Tray 形式」「清潔」三頁點下拉選單，C++ 要照原版把連動欄位填好（例：選加熱盤型號，7 格換成 `D:\HT9045\system\PlateForm.csv` 那一列） | 開頁時照原版可能補寫配方的 HotPlate.Data、HandlerCondition.Data | 測試用密碼本（轉向開關 `W906_PWBOOK_PATH`） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_contactct_live_probe.py` | `c35f375e` 20260927 16:08（`D:\HT9045\web\page\ht9045_contactct_wire.js`） | 「接觸次數」頁每秒自動更新、不搶操作權、分頁藏起來時不送 | 唯讀（它自己檢查 `D:\HT9045\system\Arm*.dat` 不變） | Edge、測試密碼本 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_open_gate_probe.py` | `cb306f89` 20260927 16:26（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`） | 權限不夠或機台運轉中時，設定頁要照原版「開不起來、也存不了」 | 加 `--no-open` 完全不碰檔；允許開的頁會照原版讀檔 | 測試用權限表（轉向開關 `W906_LEVELSET_PATH`，用探針自己的 `make-levelset` 產生）；「運轉中」那一半要機台先 START |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_ta_ts_probe.py` | `4e74e8b4` 20260927 16:35（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp`） | 「Tray 分配」頁點方向圖要照原版轉下一號；「溫度設定」頁切加熱模式要換補償表；沒點就直接存要被擋 | 開頁補缺鍵；加 `--allow-save` 會寫 Tray.Data、Temperature.Data、Tester.Data、DefineTemp、`D:\HT9045\config\ATC.ini` | 測試密碼本 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_closetail_probe.py` | `c8028b21` 20260927 16:43＋`9268162b` 19:36（兩顆的主要檔都是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp`） | 關掉某些設定頁時，原版會「順手」做的收尾（例：關 Yield 頁後存起動模式）有沒有照做；運轉中不做 | 加 `--save`／`--hotplate`／`--config` 才寫：配方的 UdUld.Data、ArmCondition.Data、Binasgn*.Data、Tray.Data、Tester.Data、HotPlate.Data、HandlerCondition.Data，`D:\HT9045\system\RunMode.txt`、`D:\HT9045\system\lastdata.dat`、`D:\HT9045\system\Gerneral.ini` 的兩個 [Shuttle] 鍵；`--config` 還會切加熱器繼電器、送 ATC7 指令 | 測試密碼本；用法第 64 行的 `--dry` 要拿掉 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_observer_token_probe.py` | `742024b7` 20260927 17:03（`D:\HT9045\web\page\ht9045_observer_wire.js`） | 「觀察」頁的唯讀查詢不搶操作權，只有會改記憶體的四個動作才拿 | 唯讀（它自己檢查 lastdata*.dat、Arm*.dat、Gerneral.ini 不變） | Edge、測試密碼本 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_cc_probe.py` | `64ade3b7` 20260927 22:02（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`） | 「Configuration」設定頁：D46 的上下鍵要照原版加減、E30／E39／D21 勾選時旁邊的欄位要跟著出現或隱藏；要密碼的 A27 不能靠直接存檔繞過 | 開頁照原版讀檔；加 `--allow-save` 會跑存檔流程（回答 NO＝config.ini 不寫，但原版關窗仍會補寫 `D:\HT9045\system\Gerneral.ini` 缺的鍵、跑關窗收尾） | 測試密碼本 |

**第二組：跑過，但之後改了、改完沒再跑（4 支）**

| 探針 | 最後一次改（主要檔） | 上次實跑那一輪 | 白話 | 會不會寫真檔 |
|---|---|---|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_smartdiag_probe.py` | `62f064bf` 20260926 18:58（`D:\HT9045\web\page\ht9045_busy_util.js`；探針只加「兩次送出間隔 0.45 秒」） | `008db55e` 那一輪 | 智慧診斷頁的開檔、改格、存檔 | `D:\HT9045\system\SmartDiagnosticRecord.txt`、`SmartDiagnosticRecordReset.txt`、`SmartDiagnosticPara.ini`（`--no-write` 跳過） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_builder_probe.py` | 同上 `62f064bf` | `008db55e` 那一輪 | 配方管理頁的新增、複製、匯出、匯入、刪除 | 在 `D:\HT9045\IniData\Data`、`D:\HT9045\IniData\Offset` 建立再刪除 W906PRB*／W906IMP* 資料夾；**刪除會進資源回收筒** |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_sortct_probe.py` | `26d0b3f8` 20260926 20:46（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp`） | `0609a14f` 那一輪 | 分類計數頁的數字對不對、清除計數 | `D:\HT9045\system\lastdata.dat`、`lastdata_backup.dat`，建立 `D:\HT9045_Log\QtyData\YYYYMM\`（`--no-clear` 跳過）；它自己會呼叫 `C:\MinGW\bin\g++.exe` 量資料結構位置 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s12_form_probe.py` | `834fcc78` 20260927 11:46（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp`；舊的第一型表單讀寫退役） | `05f2695b`（20260924）那一輪 | 剩下的第二型頁面：畫面值＝C++ 給的值；改一格存檔只差一行 | `--write` 才寫配方檔 |

**第三組：20260926 早上以前跑過、之後程式改很多、沒重跑（回歸，12 支）**

之後改到的共通行為：同一個指令 0.4 秒內再送會回「busy」（防連點，`2ae40ffe` 20260926 18:55，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp`）、
唯讀查詢不用操作權（`9d790ff2` 20260927 11:46，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp`）、運轉中不能從網頁存設定、設定頁開頁閘（`cb306f89`）、
開機就建接觸力物件（`725038a6` 20260927 13:53，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp`）、關站存生產資料（`6905f8eb` 20260926 22:05，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp`）。

| 探針（都在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\`） | 加進來的 commit（主要檔） |
|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_contactct_probe.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_counterclear_probe.py` | `267425bc` 20260925 10:10（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cContactCT.cpp`） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_lotinfo_probe.py` | `c8eb63a7` 20260925 10:27（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp`）；最後改 `8af13c07` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_observer_probe.py` | `f259d400` 20260925 13:13（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp`） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_testcategory_probe.py` | `008db55e` 20260925 15:25（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cTestCategory.cpp`） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s12c_config_probe.py` | `05f2695b` 20260924 19:51（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc`）；最後改 `9a43a8cf` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s12c_offset_probe.py` | `e086de38` 20260925 09:22（`D:\HT9045\web\page\ht9045_offset_wire.js`） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s12c_page_probe.py` | `9a43a8cf` 20260924 20:56（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp`）；最後改 `8af13c07` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s1_eventlog_probe.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s7_thermo_probe.py` | `d8863d0d` 20260923 16:45（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\gen\sjson_SYSTEM_TEST_IF.gen.cpp`）；最後改 `05f2695b`／`56319e02` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\status_towerlight_probe.py` | `0609a14f` 20260926 08:48（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTowerLight.cpp`） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\showmymessage_probe.py` | `8af13c07` 20260925 22:55（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`）——那顆的訊息寫網頁訊息框「written, NOT yet tested」，但 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 2819 行註解記載 20260925 跑過它的 `--mailbox-only` 模式 ⇒ 算「部分跑過」 |

筆電（Jimmy）寫的探針（`channel_census.py`、`cmd_probe.py`、`w4_motor_probe.py`、`w5_teach_probe.py` 等）是筆電自己跑的，不在本題範圍。

### 1.4 手上現成的東西

- **執行檔**：`D:\AI_TempFile\st02-gb-p1-build\wb_serve.exe`（模擬版，由 `c20bdec2` 編出，20260927 20:48:03 連結；紀錄 `D:\AI_TempFile\st02-c20bdec2-build.log`）。
  讀它的匯入表（本評估自己寫的小程式讀檔頭，沒有執行它）：只用 Windows 內建的 KERNEL32、msvcrt、PSAPI、SHELL32、USER32、VERSION、WS2_32，不需要 MinGW 的 DLL。
  ⚠ 這個 build 資料夾是 ST01-M 替 Steven02 代編用的，下一次代編會蓋掉；而且 20260927 21:08 起它正在跑 `c20bdec2` 的全量 gate（`D:\AI_TempFile\st01-c20bdec2-gate.log`），要跑時先等 gate 結束再複製出來。
- **現成的備份／比對工具**（都在移植樹裡）：
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\realfile_guard.py`（筆電寫的）：固定 19 個真檔＋目前配方夾，snap／check／restore／drop；只管清單內的檔——
    沒有 `D:\HT9045\Error`、`D:\HT9045_Log`、levelset.dat、login.dat、Arm0～2.dat／ArmHis0～2.dat、lastdata_backup*.dat、LotSummary.csv、RunMode.txt、`D:\GPIB9045` 等。
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\system_guard.py`（筆電寫的）：整個資料夾拍快照、比對內容與修改時間；不會還原。
  - `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\flow_run.py`（筆電寫的）：**整夾比對＋還原最完整的一份**（system／config／IniData 逐檔 MD5 還原；MDB、CFG、SECS、setup.inf、CurrentSetupData.txt 另存；
    `D:\GPIB9045\system` 兩個檔；九個記錄資料夾「新檔搬走」；`D:\` 與 `D:\HT9045` 最上層有沒有多東西；跑的途中每 5 秒查有沒有別的 wb_serve.exe，有就中止）。
    但它要筆電才有的基準快照 `D:\HT9045\backup\gate_sysguard\opmode`（這台沒有），而且它跑的是「動作流程對照」，不是探針。
- **筆電實測過的開機寫檔清單**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md` 第 204 行（第 47 列，20260927 清晨在筆電實跑模擬版約 30 秒，三次都「回到基準」）。§2 的表用它當「預期會變」的底。

---

## 2. 會寫的真檔清單

### 2.1 怎麼得出來的

三個來源，互相對照：

1. **筆電實測**（上面的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md` 第 204 行）。
2. **St01 自己公告過的新增寫入**：交接檔 FROM_STEVEN（在 `D:\HT9045` 的 `origin/v906/steven-handoff` 分支，讀法 `git -C D:\HT9045 show origin/v906/steven-handoff:docs/handoff/FROM_STEVEN.md`；
   唯讀快照 `D:\HT9045_handoff\FROM_STEVEN.md` 還沒有下面 14:00 那一列）20260926 22:10 列 ⑥（每次限時執行結束時照原版關站會寫 lastdata、config.ini [O_Count]、DailyJamRate、Arm*.dat）
   與 20260927 14:00 列（開機會補寫 ContactInfo.ini；這台的設定是 40／56／60／80 四種口徑各 16 段，共 64 段 128 個鍵）。
3. **靜態普查**：把移植樹（不含 tests、第三方、ui、其他工具程式；含 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`）980 個 C++ 檔，去掉註解與 `#if 0` 區塊後，找出所有「磁碟代號開頭」的字串。結果見 §2.4。

### 2.2 表

「測試縫」＝上面講的測試用轉向開關。「誰的檔」依 git 紀錄的主要作者，實際能不能動以交接登記簿為準（`D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md` §8）。

| 真檔 | 誰寫、什麼時候 | 有沒有測試縫 | 沒有的話是誰的檔 |
|---|---|---|---|
| `D:\HT9045\system\Gerneral.ini` | 開機讀、補缺鍵；開始服務時寫「程式已關閉＝0」、正常關站寫 1（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 4536、5965 行）；關 Contact／Bin 頁的收尾補 [Shuttle] 兩鍵；事件紀錄分析器補機台 ID | 有：`W906_GENERAL_INI_PATH`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 第 155 行）。但測試機通訊另有 6 處寫死（關掉測試機通訊就不會走到） | 寫死的在 Steven02 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib\GpibAux.cpp` 第 682、795 行、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib\GpibGlobals.cpp` 第 149、329 行、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Rs232\Rs232Globals.cpp` 第 96、215 行 |
| `D:\HT9045\system\teach.ini` | 開機補寫接觸高度相對值、bAOAMatrix，區段之間補空行（照原版） | 有：`W906_TEACH_INI_PATH`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 第 305 行） | — |
| `D:\HT9045\system\lastdata.dat`、`lastdata_backup.dat`、`lastdata_backup2.dat` | 開機、換配方、切運轉模式、分類計數清除、關站（寫入函式 WriteLastDataFile） | **等於沒有**：只有 ctest 用得到的 `W906_CTEST_LASTDATA_DIR_<程序編號>`，名字綁程序編號，外面設不到正式的網頁程式（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp` 第 4262～4280 行，刻意設計） | Jimmy：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp` 第 1713～1778、2026～2062 行 |
| `D:\HT9045\config\config.ini` | 上面的 WriteLastDataFile 每次一定寫（Vibrate_Time、P65_QAMode、SocketContact、[O_Count] 接觸壽命）；批號開始寫 [Lot Info]；計數器顯示頁寫 [Visible]；事件紀錄分析器補缺鍵 | 大部分有：`W906_AUTH_PATH`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 第 139～145、168 行）。寫死的：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSetUp.cpp` 第 996、1005、1009 行（[RTC] Enable 的讀寫）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.gen.inc` 第 830、2108 行、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaHub.cpp` 第 52 行 | Jimmy（cSetUp.cpp）、St01（TrayForm.gen.inc）、Steven02（ElaHub.cpp）。⚠ `D:\HT9045\config` 有 47 個檔在 `D:\HT9045` 的 git 裡（config.ini、LastSet.ini、ATC.ini、FormPos.def 都是），被改到時 ST01-E 的工作樹會顯示「已修改」 |
| `D:\HT9045\config\LastSet.ini`、`ATC.ini` 等 | 存最後設定；溫度頁存檔寫 ATC.ini | LastSet 走 `W906_AUTH_PATH`；ATC.ini 寫死（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp` 第 3049 行、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.gen.inc` 第 3887、6076 行） | Jimmy、St01 |
| `D:\HT9045\system\ContactInfo.ini` | 開機就建接觸力物件（`725038a6` 起）：缺的口徑補段補鍵（這台 128 個鍵）；沒這個檔時整份建立，還會寫 `Gerneral.ini` 的 EP 8 個鍵 | 沒有 | Jimmy `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp` 第 26 行；St01 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.gen.inc` 第 334 行 |
| `D:\HT9045\system\machinerecord.dat`、`machinerecordRealCCD.dat` | 開機讀機台紀錄後照原版回存 | 有：`W906_MACHINERECORD_DIR`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cinitial.cpp` 第 9472 行） | — |
| `D:\HT9045\system\Arm{0,1,2}.dat`、`ArmHis{0,1,2}.dat`、`ArmByLot{0,1,2}.dat` 與各自 `_backup.dat`（18 檔） | 正常關站寫接觸計數；批號開始清 ArmByLot | 有：同上 `W906_MACHINERECORD_DIR`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSocket.cpp` 第 539～541 行） | — |
| `D:\HT9045\system\MachineLife.ini` | 開機讀氣缸壽命，缺鍵補預設 | 沒有 | Jimmy `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 第 334 行 |
| `D:\HT9045\System\LotSummary.csv` | 正常關站整份覆寫（32 列 × 256 欄） | 沒有 | Jimmy `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSocket.cpp` 第 1159、1200 行 |
| `D:\HT9045\system\RunMode.txt` | 正常關站存起動模式；關 Yield 頁的收尾 | 沒有 | St01 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp` 第 1027 行 |
| `D:\HT9045\system\BinCount.txt` | 開機讀；清計數時刪；按 Exit 關程式時寫 | 有：`W906_BINCOUNT_PATH`（⚠ 設成空字串＝空路徑，不是「沒設」） | — |
| `D:\HT9045\system\levelset.dat` | 權限頁存檔 | 有：`W906_LEVELSET_PATH`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp` 第 2123～2129 行） | — |
| `D:\HT9045\system\login.dat` | 權限頁關窗時照原版重寫密碼檔 | 一半：`W906_LOGINDAT_PATH` 只管 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 第 665～669 行；原版的重寫在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp` 第 1460、1474 行寫死——三個密碼相關開關任一有設，這一段整個跳過（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp` 第 122～125 行） | Jimmy（cprod.cpp） |
| `D:\HT9045\system\SmartDiagnostic*.txt／.ini` | 智慧診斷頁存檔；檔在之後每次氣缸動作都重寫 | 沒有 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fSmartDiagnostic.cpp` 第 160～163 行 |
| `D:\HT9045\IniData\Data\<配方>\*.Data`（HotPlate、HandlerCondition、Tester、Tray、Temperature、UdUld、ArmCondition、Binasgn…） | 開設定頁時照原版補缺鍵（例：清潔頁每次開都寫 HandlerCondition.Data 的 iIndexArmAutoCleanCnt）；存檔；存檔後照原版在配方夾寫檢查碼 | **對網頁程式等於沒有**：有 `W906_INIDATA_ROOT`，但網頁程式一看到它有設就拒絕服務（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 3756～3760 行；裁決 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260917.md` A1.2） | 裁決層級，不是某個人的檔 |
| `D:\HT9045\IniData\Data\`、`D:\HT9045\IniData\Offset\` 底下新建／刪除的資料夾 | 配方管理頁（builder 探針） | 同上 | 刪除進資源回收筒 `D:\$RECYCLE.BIN`（不在任何比對範圍內） |
| `D:\HT9045\IniData\SocketCount.ini`、`D:\HT9045\IniData\DefineAutoClean\AutoClean.data` | 起始條件頁、清潔頁 | 沒有 | St01 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.gen.inc` 第 1332 行起、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.gen.inc` 第 776 行起 |
| `D:\HT9045\SetUp.inf` | 換配方時寫第一行 | 有：`W906_SETUPINF_PATH`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 第 230 行） | — |
| `D:\HT9045\Error\BootLog.txt` | 每次開機附加一行 | 沒有 | Jimmy `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Public\cBootLog.cpp` 第 28 行 |
| `D:\HT9045\Error\AlarmCodeList.txt` | 開機載入告警碼目錄後存檔 | 沒有 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cMyDB.cpp` 第 320 行（Steven02 的 cMyDB 工作） |
| `D:\HT9045\Error\English\JAM0000.dat` | 權限頁的 Jam 分頁存檔；事件紀錄分析器查當天時補缺碼 | 沒有 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp` 第 272、2085 行；Steven02 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.cpp` 第 68 行 |
| `D:\HT9045_Log\` 的 EventLogTxt、SaveEventLog、Production_Log、CleanPad_Log、SocketIDLog、Summary_Lot、E84DataTxt、TCP_Data | 事件紀錄、生產紀錄等 | 有：各自一個 `W906_…_ROOT`（共 8 個；表在 `D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md` §5） | — |
| `D:\HT9045_Log\` 其餘（JamRate_Daily＝每日 Jam 率、QtyData、MDB_UpdateLog、SiteUseMgr、UPH、HomeLog…） | 正常關站整份覆寫 `D:\HT9045_Log\JamRate_Daily\<機型>_<機台ID>_<日期>_DailyJamRate.txt`；分類計數清除建 QtyData；開機寫 MDB_UpdateLog；Site Use Manager 每小時一檔 | 沒有（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 第 150～340 行的 76 個 `D:\HT9045_Log` 路徑只有 4 個有縫，另 1 個由有縫的根目錄組出來） | 大多 Jimmy（common.cpp） |
| `D:\GPIB9045\system\general.ini`、`GpibString.dat`，`D:\GPIBLOG`、`D:\RS232Log` | 測試機通訊引擎（開機約 1 秒後照原版啟動）寫 LastFile=、GpibString 與通訊紀錄 | 沒有；但可以整個關掉：`HT9045_TESTERCOMM=0`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\TesterCommWiring.cpp` 第 86 行） | Steven02 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib\GpibGlobals.cpp` 第 407 行 |
| `D:\RS232Standard\system\setup.ini` | Handler System 頁存檔；RS232 引擎 | 沒有 | Jimmy `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\HandlerSys.cpp` 第 245 行；St01 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc` 第 3135 行 |
| `D:\UnloaderInfo\` | 存機台紀錄時 | 有：`W906_UNLOADERINFO_ROOT`（⚠ 空字串＝空路徑） | — |
| `D:\HT9045_StateRecord\` | 只有按「State Record」時 | 沒有 | Jimmy `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cStateRecord.cpp` 第 1040 行 |
| `D:\HT9045_Backup\` | 配方存檔後的備份，只有特定客戶碼＋A24 開著時 | 沒有 | St01 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBackup.cpp` 第 76 行 |
| `D:\HT9045\web\JSON\runtime\`（網頁訊息框的信箱檔） | 開機把上次留下的請求重設成閒置；C++ 跳訊息框時寫 | 跟著啟動參數 `--root` 走（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 4312、4334 行） | — |

另有兩個「整個關掉」的開關（不是 `W906_`，但一樣只給測試用）：`HT9045_TESTERCOMM=0`（不啟動測試機通訊）、`HT9045_ELA=0`（不啟動事件紀錄分析器，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp` 第 149 行）。
還有 `W906_NO_BROWSER_WAKE=1`：跳出要回答的訊息框而沒有網頁連著時，不要自動開瀏覽器（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 7528 行；帶 `--seconds` 時本來就不開）。

### 2.3 測試縫的四個陷阱

1. **配方資料夾不能轉**：`W906_INIDATA_ROOT` 一設，網頁程式直接拒絕服務（上表）。探針最常碰到的就是配方檔，所以「用現有開關把全部轉走」做不到。
2. **lastdata 三個檔轉不了**：刻意只給 ctest（上表）。理由跟 `--dry` 退場一樣：正式程式不能有「看起來存成功、其實寫到別處」的執行期開關。
3. **空字串的意思不一樣**：有 6 個開關設成空字串會變成「空路徑」而不是「沒設」（`W906_BINCOUNT_PATH`、`W906_UNLOADERINFO_ROOT`、`W906_IOTABLE_PATH`、`W906_MOTTABLE_PATH`、`W906_TCPDATA_ROOT`、`W906_SUMMARYLOT_ROOT`；另外 `W906_EVENTLOG_ROOT` 在觀察頁那一邊也是、`W906_E84DATA_ROOT` 會變成「\」）。要關就整個移除。
4. **開機只印 18 個**：網頁程式開機會把有設的開關印出來（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 7672～7690 行），但名單只有 18 個，
   `W906_LEVELSET_PATH`、`W906_LOGINDAT_PATH`、`W906_SOCKETIDLOG_ROOT` 不在裡面 ⇒ 事後從開機紀錄看不出這三個有沒有設，要自己另外記。
   另外，三個密碼相關開關任一有設，權限頁關窗時「重寫密碼檔」那一段會整個跳過——用它們跑的測試看不到那一段。

### 2.4 靜態普查的數字（`c20bdec2`）

- 去掉註解與 `#if 0` 後，**磁碟代號開頭的字串 841 個，分布在 109 個檔**；其中 61 個（7%）已經經過測試縫；第一層根目錄 49 種。
- 依根目錄：`D:\HT9045\…` 366（其中 `D:\HT9045\system` 188、`D:\HT9045\IniData` 32、`D:\HT9045\PMAlarm` 32、`D:\HT9045\Error` 28、`D:\HT9045\config` 25）、`D:\HT9045_Log` 234、`D:\RMS` 45、`D:\GPIB9045` 29、
  `C:\GTK_Control` 15、`D:\RS232Standard` 14、`D:\HandlerLog` 12、`D:\RS232Log` 12，其餘 40 種各 1～11 個（多半是特定客戶才走得到）。
- 依檔案：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 223、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\` 118、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp` 83、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\` 43、
  `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp` 35、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP\` 30、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Automation\` 28、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\` 28。
- 直接呼叫 Windows／C 檔案函式、不經過相容層的地方（`git grep` 行數，含少量註解行）：開檔 `fopen` 133 行／45 檔、`CreateFile` 45／19、`CopyFile` 87／26、`DeleteFile` 60／33、
  整夾複製刪除 `SHFileOperation` 39／5、`FindFirstFile` 18／12、`Get/WritePrivateProfileString` 31／14、啟動外部程式（例 `D:\HT9045\7z.exe` 壓縮，指令列裡帶路徑）79／20。
- 限制：只抓得到「寫死在字串裡」的路徑；由變數組出來的（例 配方夾＋配方名）看不到；也混了少數只是訊息文字的字串。所以這是**下限與規模感**，不是完整清單。

### 2.5 這台的資料量（20260927 21:4x 量）

| 資料夾 | 檔數 | 大小 |
|---|---|---|
| `D:\HT9045\system` | 459 | 59 MB |
| `D:\HT9045\config` | 65 | 3 MB |
| `D:\HT9045\IniData` | 4,488 | 80 MB |
| `D:\HT9045\Error` | 2,288 | 6 MB（筆電沒有這個資料夾，這台有） |
| `D:\HT9045\MDB`、`D:\HT9045\CFG`、`D:\HT9045\SECS`、`D:\HT9045\PMAlarm` | 3＋21＋49＋7 | 約 21 MB |
| `D:\HT9045\web` | 805 | 98 MB |
| `D:\HT9045_Log` | 152 | 404 MB |
| `D:\GPIB9045\system`、`D:\RS232Standard\system`、`D:\UnloaderInfo`、`D:\GPIBLOG`、`D:\RS232Log`、`D:\HandlerLog`、`D:\SECS_GEM_LOGS` | 5＋1＋1＋27＋13＋7＋1 | 各 1 MB 以下 |

`D:\HT9045_StateRecord`、`D:\HT9045_Backup`、`D:\AutoCleanLogs` 這台目前沒有（跑完若出現＝這次跑出來的）。

---

## 3. 同時開第二個網頁程式會撞到什麼

| 共用的東西 | 會怎樣 | 怎麼避 |
|---|---|---|
| 網頁程式自己的連接埠（預設 8045，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 3619 行；VS Code F5 也用 8045） | 第二個開不起來就結束。**⚠ 但撞埠保護不了真檔**：開機讀寫檔全部發生在開連接埠之前（同檔第 4368～4370 行開不起來才 `return 1`），撞到時真檔已經被寫了，而且不會走正常關站 | 用 `--port 8046`；跑前先確認 8046 沒人用 |
| 機台通訊伺服器（SECS/GEM、ATC7、OLP、TSV 等，依設定檔開） | 用「對整個網路開放＋允許重複綁定」的方式開（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\ServerSocket.cpp` 第 319、325 行）：Windows 上兩個程式可以同時綁同一個埠，外面的連線會進哪一個不固定 | 同一時間只開一個網頁程式；第一輪看開機紀錄這台開了哪些伺服器 |
| Jam 設定檔的共用鎖（`Local\HT9045_JAM0000_dat`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamIniMerge.h` 第 37 行、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.cpp` 第 447 行） | 同一個登入工作階段裡的兩個程式共用這把鎖；寫 JAM0000.dat 時最多等 2 秒，等不到那一頁回「busy」；不會壞檔，但兩個程式寫的是同一個真檔 | 同上 |
| 測試機通訊引擎（GPIB／RS232） | 「只能開一個」只在程式內部判斷（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib\GpibEngine.cpp` 第 108～110 行），兩個程式會各開一個，搶同一個 COM 埠或 GPIB 卡，都寫 `D:\GPIB9045\system\general.ini` | 沙盒執行一律 `HT9045_TESTERCOMM=0` |
| 網頁訊息框信箱 `D:\HT9045\web\JSON\runtime\` | 第二個程式開機會把信箱重設成閒置（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 4334 行），把第一個程式正在等操作員回答的訊息框清掉 | `--root` 指到複製出來的網頁資料夾 |
| VS Code F5（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\.vscode\launch.json`） | 按 F5 會再起一個網頁程式寫真檔；我們的比對與還原會把它的寫入混進來或蓋掉 | 執行時段不按 F5；備份工具每 5 秒查有沒有別的 wb_serve.exe，有就中止（照 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\flow_run.py` 第 60～72、329 行的做法） |
| ctest／全量 gate／替 Steven02 代編 | ctest 會讀這台真的 `D:\HT9045\system`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\flow_run.py` 檔頭第 10 行）；我們的寫入或還原會讓它假失敗 | 執行時段 ST01-M 暫停代編、ST01-E 不跑 gate |
| 筆電動作流程對照的「看到別的 wb_serve 就中止」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\flow_run.py` 第 60 行 `other_wb_serve`） | 它只看**同一台電腦**的程序清單 ⇒ 筆電看不到 Steven01 的網頁程式，兩台互不影響。若有人在 Steven01 跑它，它一開始就會因為看到 wb_serve.exe 而拒絕；而且它需要的基準快照 `D:\HT9045\backup\gate_sysguard\opmode` 這台沒有 | 無 |
| 探針開的無頭 Edge 除錯埠（`--dbg`） | 兩支同時跑會撞 | 探針一支接一支跑 |

---

## 4. 選項

### A：搬家——讓網頁程式把所有讀寫都改到一個暫存根目錄

**A-1：改程式，加一個總轉向開關（例 `W906_DATA_ROOT`）**

- **做法（白話）**：程式裡所有寫死的 `D:\HT9045\…`、`D:\HT9045_Log\…`、`D:\GPIB9045\…` 等，在開檔前一律換成「暫存根目錄＋原路徑」。跑之前把 `D:\HT9045\system`、`config`、`IniData`（與其他資料夾）整套複製到暫存根目錄。
- **例子**：要改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 223 處寫死路徑（146 種，Jimmy 的檔）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\` 43 處（Steven02 的檔）、
  `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\` 118 處（St01 的檔）…；還有約 500 行直接呼叫 Windows 開檔／複製／刪除的地方，加上 `D:\HT9045\7z.exe` 指令列裡的路徑。**漏一處，那一處就照樣寫真檔**，而測試照樣通過。
- **要改的檔與主人**：§2.4 那 109 個檔（主要是 Jimmy、Steven02、St01，另有 1203 開卡那一段的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.cpp` 第 109 行，筆電與機台端都改過）；或在相容層 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\` 集中改（共用元件，改了要全量 ctest）再補直接呼叫的那 500 行。
  另外 **要推翻兩條裁決**：「配方資料夾被轉向時網頁程式拒絕服務」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260917.md` A1.2）與「不可以有會把存檔導到別處的執行期開關」（`--dry` 退場，20260923／20260924）。
  若要守住第二條，只能做成「另一個建置組態才有、出貨版完全沒有」——等於第三種組態，這台每多一個組態全量 build 多 20～40 分鐘。
  探針也要改：約 12 支把比對用的路徑寫死在 `D:\HT9045\…`（例 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_probe.py` 第 50～51 行、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_sortct_probe.py` 第 52～55 行），要加「資料根目錄」參數（St01 的檔）。
- **風險**：「看起來隔離了、其實有漏」——20260911 就發生過「通過的測試改掉機台正在用的配方」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 第 170～196 行的註解）。要證明沒漏，得錄下程式實際開過的每一個路徑（例如用微軟的 Process Monitor），而那本身又是一次實跑。
  之後每翻一段原版程式都可能多一個寫死路徑，要一道常駐的掃描關卡防它退化。
- **工作量**：程式 3～5 人天＋三位主人各自審＋兩組態全量 gate＋兩條裁決重議。
- **能驗到什麼**：全部探針；做完之後可以隨時跑、甚至跟 gate 同時跑（前提是真的沒漏）。

**A-2：不改程式，用 Windows 內建的「拋棄式沙盒」（Windows Sandbox）**

- **做法（白話）**：Windows 11 專業版內建一個「關掉就整個消失」的小虛擬電腦。它裡面只有 C 槽，可以在裡面把一個資料夾掛成 D 槽，把機台資料複製進去，網頁程式寫死的 `D:\…` 就全部落在沙盒裡；真的 D 槽只用「唯讀」方式分享進去，物理上寫不到。
- **例子**：把 `D:\AI_TempFile\wbrun_c20bdec2\` 唯讀分享進沙盒，裡面有網頁程式執行檔、`D:\HT9045\system`／`config`／`IniData`／`Error`／`web` 的複本、Python；沙盒開機後在裡面把 `C:\sbx\D` 掛成 D 槽，跑探針，結果寫到一個可寫的分享資料夾；關掉沙盒，裡面改過的東西全部消失。
- **要改的檔與主人**：程式 0；要系統管理員打開 Windows 功能（需重開機、BIOS 要開虛擬化）。**這台目前看起來沒開**：`C:\Windows\System32\WindowsSandbox.exe`、`C:\Windows\System32\vmcompute.exe` 都不存在（未驗證能不能開、公司政策准不准）。
- **風險**：真檔零風險（寫不到）；不確定的是環境：沙盒裡的 Python（`C:\Users\steven\AppData\Local\Programs\Python\Python314` 唯讀分享進去能不能直接跑）、分類計數探針要的 `C:\MinGW\bin\g++.exe`、Edge（沙盒內建）都要試過。
- **工作量**：第一次準備約半天；之後每次只要複製資料（約 250 MB，記錄資料夾 404 MB 可選）。
- **能驗到什麼**：跟 B 一樣全部探針，而且不必挑獨佔時段（沙盒裡的程式碰不到這台的真檔與 gate）。

### B：不改程式——現有開關＋整夾備份，跑完逐檔比對還原（建議，用 B-1）

**B-1：全部走真路徑，只設測試一定要的開關，其餘整夾備份還原（建議）**

- **做法（白話）**：像搬家前先拍照。跑之前把會被寫到的資料夾整夾複製一份、記下每個檔的指紋（SHA256）；跑完再算一次指紋，改過的從備份放回去、新多出來的刪掉、不見的補回來，再算一次確認全部跟跑之前一樣。
  只設測試一定要的四個開關：測試用密碼本（`W906_PWBOOK_PATH`）、測試用權限表（`W906_LEVELSET_PATH`，只在開頁閘那支）、不啟動測試機通訊（`HT9045_TESTERCOMM=0`）、不啟動事件紀錄分析器（`HT9045_ELA=0`），加上不自動開瀏覽器（`W906_NO_BROWSER_WAKE=1`）。
- **例子**：今晚 23:00～02:00 這台只跑這件事。第一輪只開機 2 分鐘就正常關站，比對後預期看到：`D:\HT9045\system\ContactInfo.ini` 多了 128 個鍵、`D:\HT9045\Error\BootLog.txt` 多一行、
  `D:\HT9045\system\lastdata.dat` 與 `D:\HT9045\config\config.ini` 被照原版改寫、`D:\HT9045_Log\JamRate_Daily\` 多一個當天的檔——都在筆電量過的清單裡 ⇒ 全部還原、進下一輪。
  若出現清單外的檔（例：某個配方的 HandlerCondition.Data 被改），就停下來先回報 Steven，不往下跑。
- **備份範圍**（約 580 MB，這台複製與算指紋約 1～3 分鐘）：
  - 整夾備份＋逐檔指紋（跑完還原）：`D:\HT9045\system`、`D:\HT9045\config`、`D:\HT9045\IniData`、`D:\HT9045\Error`、`D:\HT9045\MDB`、`D:\HT9045\CFG`、`D:\HT9045\SECS`、`D:\HT9045\PMAlarm`、
    `D:\HT9045\setup.inf`、`D:\HT9045\CurrentSetupData.txt`、`D:\GPIB9045\system`、`D:\RS232Standard\system`、`D:\UnloaderInfo`。
  - 記錄資料夾（整夾備份；跑完新檔搬出、改過的還原）：`D:\HT9045_Log`、`D:\GPIBLOG`、`D:\RS232Log`、`D:\HandlerLog`、`D:\SECS_GEM_LOGS`、`D:\RMS`、`D:\SaveRecord`、`D:\PrecautionRecord`。
    ⚠ 筆電的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\flow_run.py` 對記錄資料夾只處理「新檔」，**已存在的記錄檔被附加內容不會還原**——這裡要補上。
  - 只看有沒有多出來：`D:\` 與 `D:\HT9045` 最上層（例 `D:\HT9045_StateRecord`、`D:\HT9045_Backup`）、資源回收筒 `D:\$RECYCLE.BIN`。
  - 網頁資料夾不在範圍內：網頁程式用 `--root` 指到複本 `D:\AI_TempFile\wbrun_<commit>\web`（從 `D:\HT9045\web` 複製，98 MB），信箱檔寫在複本裡。
- **要改的檔與主人**：別人的檔一個都不動。St01 自己：
  - 新寫一支備份／比對／還原小工具（建議新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\wbrun_guard.py`，照抄 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\flow_run.py` 第 216～246 行（備份）與第 360～424 行（還原）的邏輯，補上「改過的記錄檔還原」「別的 wb_serve 出現就中止」「標記檔＋當掉後補還原」）。
  - 兩支探針的用法說明拿掉 `--dry`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_open_gate_probe.py` 第 38 行、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_closetail_probe.py` 第 37、53、57、64 行）。
- **風險**：
  - 跑到一半程式當掉或電腦重開 ⇒ 真檔停在「改過」的狀態，直到有人還原（標記檔 `D:\AI_TempFile\wbrun_ACTIVE.json`＋`restore` 指令補救，跟 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\flow_unswap.py` 同一個想法）。
  - 時段內有人按 F5、跑 ctest 或代編 ⇒ 比對與還原會混進別人的寫入（獨佔時段＋每 5 秒查）。
  - `D:\HT9045\config` 的 47 個檔在 git 裡，跑的期間 ST01-E 的工作樹會看到它們「已修改」；時段內 ST01-E 不 commit，跑前跑後各記一次 `git -C D:\HT9045 status --porcelain -- config`。
  - 配方管理探針的「刪除」進資源回收筒，不在還原範圍，要另外檢查、清掉。
  - 還原把「照原版應該發生的改變」也倒回去了——那正是我們要的（探針負責驗證，還原負責回到原樣），但要把每一輪「改了哪些檔」存下來當證據，不能只留「CLEAN」。
- **工作量**：小工具約半天；第一次整套（四輪、25 支探針）約 3～4 小時；之後每次約 1～2 小時。
- **能驗到什麼**：第一組、第二組 13 支＋第三組 12 支回歸；另外量到**這台自己**的開機寫檔清單（跟筆電比；這台的設定跟筆電不同，例 ContactInfo.ini 的 128 個鍵只會在這台發生）。走真路徑，最接近實機。

**B-2：能轉的先轉（約 10 個開關），剩下的備份還原**

- **做法**：Gerneral.ini、teach.ini、config 資料夾、setup.inf、machinerecord、BinCount、8 個記錄子資料夾都用開關轉到暫存；轉不了的（lastdata 三檔、ContactInfo.ini、MachineLife.ini、LotSummary.csv、RunMode.txt、配方資料夾、`D:\HT9045\Error` 三個檔、GPIB／RS232 設定、其餘記錄）照 B-1 備份還原。
- **例子**：計數器顯示探針預設比對 `D:\HT9045\config\config.ini`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\status_countersel_probe.py` 第 124 行），但 C++ 已經寫到暫存的 config.ini ⇒ 探針回報「沒變」，看起來通過，其實比錯檔。
- **為什麼不建議**：一半在暫存、一半在真檔，網頁看的跟 C++ 用的可能不是同一份；還原範圍並沒有少多少（配方與 lastdata 仍要備份）；而且有 3 個開關開機不會印出來、6 個開關空字串會變空路徑（§2.3）。

### C：這台繼續不跑，交給筆電或 HT9050 機台

- **做法（白話）**：St01 把「要跑哪幾支、怎麼啟動、預期結果、要準備的測試密碼本與權限表」寫成一頁，交給 Jimmy 在筆電跑（他那邊已經每次 gate 後都實跑並逐檔還原，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md` 第 45、56 行），或排到 HT9050 機台（EastSun）。
- **例子**：St01 今晚把 13 支的跑法寫進交接檔，Jimmy 排進筆電夜間實跑；結果最快隔天回來，有錯要再來回一次。
- **要改的檔與主人**：程式 0；只改兩支探針用法的 `--dry`（St01）。
- **風險**：這台零風險；但慢、佔 Jimmy 的時間（Steven 20260926 說過「Jimmy 可能沒空管」，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` 第 286 行）；
  HT9050 機台是真機、跑的是出貨組態，碰加熱器或起停的探針要機台端的人在旁（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md` 第 16、17 題），也會跟機台 bring-up 搶時間。
- **工作量**：St01 寫交付說明約 1 小時；對方每輪約半天；來回至少一天。
- **能驗到什麼**：跟 B 一樣的探針，但慢；也看不到這台自己的資料會怎樣。

### 四個做法並排

| | A-1 改程式總轉向 | A-2 Windows 沙盒 | **B-1 備份還原（建議）** | C 交給別人 |
|---|---|---|---|---|
| 要改別人的程式 | 要（約 109 檔，三位主人） | 不用 | 不用 | 不用 |
| 要推翻裁決 | 兩條 | 不用 | 不用 | 不用 |
| 真檔風險 | 有漏就寫真檔，而且不易發現 | 零 | 中低：當掉時要補還原；獨佔時段 | 這台零 |
| 要系統管理員 | 不用 | 要（開功能、重開機） | 不用 | 不用 |
| 第一次多久能跑 | 一週以上 | 半天（若准開） | 半天 | 看 Jimmy |
| 能不能跟 gate 同時跑 | 能（若沒漏） | 能 | 不能 | — |

---

## 5. 建議

**選 B-1**，理由：

1. 不動 Jimmy、Steven02、機台端任何一個檔，也不用重議已裁決的規則；St01 只寫一支自己的小工具、改自己兩支探針的說明。
2. 做法有先例而且量過：筆電一直用「整夾比對＋還原」實跑（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\flowcmp\flow_run.py`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md` 第 204 行三次都回到基準）；St01 自己 0924～0926 早上也這樣跑過。
3. 真路徑最接近實機：探針驗到的就是出貨版會碰到的那些檔，而不是轉向後的複本。
4. 第一輪只開機、先比對，有一個「看到清單外的檔就停」的關卡，風險可控。

若之後這台需要常常跑（例：每天兩次以上），再考慮 A-2（要系統管理員，先問公司政策）；**不建議 A-1**（規模大、要推翻兩條裁決、動三位主人的檔，而且無法證明沒漏）。

---

## 6. 建議的執行步驟（選 B-1 時）

### 6.0 前提（缺一不跑）

- Steven 同意（§8），並指定時段；執行當下跳出的權限提示由 Steven 本人按允許（代理不能自己改權限設定）。
- 時段內這台沒有別的：ST01-M 暫停替 Steven02 代編、ST01-E 不跑 gate／ctest、Steven 不按 VS Code F5、沒有別的 wb_serve.exe。
  檢查：`tasklist /FI "IMAGENAME eq wb_serve.exe"`、`tasklist /FI "IMAGENAME eq ctest.exe"`、`tasklist /FI "IMAGENAME eq cc1plus.exe"` 都要是空的；`netstat -ano | findstr :8046` 沒有結果。
- `D:\AI_TempFile\st01-c20bdec2-gate.log` 已出現結束行（正在跑的全量 gate 結束）。
- `D:\AI_TempFile\wbrun_ACTIVE.json` 不存在（上一次沒有留下未還原的執行）。

### 6.1 準備（不啟動網頁程式）

1. 建執行資料夾 `D:\AI_TempFile\wbrun_c20bdec2\`（實際用當時要驗的 commit 命名）。
2. 複製執行檔：`D:\AI_TempFile\st02-gb-p1-build\wb_serve.exe` → `D:\AI_TempFile\wbrun_c20bdec2\wb_serve.exe`；記下它的修改時間與 `git -C D:/AI_TempFile/st02-gb-p1 rev-parse HEAD`（要等於要驗的 commit，且修改時間晚於那顆 commit）。
3. 複製網頁：`robocopy D:\HT9045\web D:\AI_TempFile\wbrun_c20bdec2\web /E`（98 MB）。
4. 測試用密碼本：`D:\AI_TempFile\wbrun_c20bdec2\pwbook.txt`，一行「`S12TEST 3 S12PW`」（格式：帳號 等級 密碼，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 第 574 行註解）。
5. 測試用權限表：`C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_open_gate_probe.py make-levelset D:\AI_TempFile\wbrun_c20bdec2\q41_levelset.dat`。
6. 把兩支探針用法裡的 `--dry` 拿掉（§4 B-1，St01 一個 commit，不影響程式）。
7. 備份：`python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\wbrun_guard.py snap r1`（範圍見 §4 B-1；寫標記檔；另記 `git -C D:\HT9045 status --porcelain -- config web` 到 `D:\AI_TempFile\wbrun_c20bdec2\git_before.txt`）。

### 6.2 第 1 輪：只開機（決定要不要往下跑）

- 啟動（PowerShell，先清掉所有 `W906_*` 環境變數，只留這一輪要的）：
  `$env:HT9045_TESTERCOMM='0'; $env:HT9045_ELA='0'; $env:W906_NO_BROWSER_WAKE='1'; D:\AI_TempFile\wbrun_c20bdec2\wb_serve.exe --seconds 120 --port 8046 --root D:\AI_TempFile\wbrun_c20bdec2\web *> D:\AI_TempFile\wbrun_c20bdec2\r1_console.txt`
  （PowerShell 5.1 的 `*>` 會寫成 UTF-16，檢視時注意；見 `D:\HT9045\.claude\skills\ops-ht9045-proxy-build\references\gotchas.md` 第 2 條。）
- 等它自己結束（`--seconds 120` 到時間走正常關站）→ `wbrun_guard.py check r1`：列出改過／新增／不見的檔。
- **判斷**：每一個改過的檔都要在「預期清單」裡——§2.2 表中標「開機」「正常關站」的那幾列（Gerneral.ini、teach.ini、lastdata 三檔、config.ini、ContactInfo.ini、machinerecord、Arm* 18 檔、MachineLife.ini、LotSummary.csv、RunMode.txt、BootLog.txt、AlarmCodeList.txt、
  `D:\HT9045_Log` 的 JamRate_Daily／MDB_UpdateLog／SiteUseMgr）。**有清單外的檔就停，還原後回報 Steven，不跑第 2 輪。**
- `wbrun_guard.py restore r1` → `wbrun_guard.py check r1` 要「全部未變」→ 比 `git status` 跟跑前一樣 → 刪標記檔。

### 6.3 第 2 輪：不寫檔的探針（一次開機跑完）

啟動同上，另加 `$env:W906_PWBOOK_PATH='D:\AI_TempFile\wbrun_c20bdec2\pwbook.txt'; $env:W906_LEVELSET_PATH='D:\AI_TempFile\wbrun_c20bdec2\q41_levelset.dat'`，`--seconds 1800`。依序：

1. `python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q3_dio_owner_probe.py --port 8046`
2. `python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_open_gate_probe.py --port 8046 --levelset D:\AI_TempFile\wbrun_c20bdec2\q41_levelset.dat --no-open`
3. `python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_contactct_live_probe.py --port 8046 --user S12TEST --password S12PW --serve-log D:\AI_TempFile\wbrun_c20bdec2\r2_console.txt`
4. `python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_observer_token_probe.py --port 8046 --user S12TEST --password S12PW`（不加 `--mutate`）
5. `python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_probe.py --port 8046 --user S12TEST --password S12PW`（開頁可能補寫配方檔——照原版，列進本輪的改變清單）
6. `python D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_cc_probe.py --port 8046 --user S12TEST --password S12PW`（不加 `--allow-save`）

結束 → check → 記下改變清單 → restore → check 全部未變。

### 6.4 第 3 輪：會寫檔的探針（每一支各開一次機、各還原一次，出錯才分得清是誰）

依序，每支一輪（開機參數同第 2 輪）：

1. `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_ta_ts_probe.py`：先不加 `--allow-save`，再加一次；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_cc_probe.py` 加 `--allow-save` 一次。
2. `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q41_closetail_probe.py`：先只跑停機時的 S1（不加 `--save`），再加 `--save`，再加 `--hotplate`；**`--config` 這一輪先不跑**（會切加熱器繼電器、送 ATC7 指令；模擬版在這台會不會真的連網路上的 ATC 控制器未驗證，§9）。
3. `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\status_countersel_probe.py --write --file D:\HT9045\config\config.ini --edit cbUPH`
4. `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_smartdiag_probe.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_builder_probe.py`（跑完檢查資源回收筒）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_sortct_probe.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\s12_form_probe.py --write …`

### 6.5 第 4 輪：回歸（§1.3 第三組 12 支）

一次開機跑唯讀的、會寫的各自一輪，做法同上。

### 6.6 以後再說：要「機台運轉中」的那一半

`q41_open_gate_probe.py --expect-running`、`q41_closetail_probe.py` 的 R 段要先在模擬版按 START（會寫批號、清計數、lastdata 等一大串）。另開一輪，等前四輪都乾淨再做。

### 6.7 每一輪怎麼證明沒漏

- 備份範圍內每一個檔的指紋跟跑之前一樣；沒有新檔、沒有不見的檔。
- 記錄資料夾：沒有留下新檔；已存在的檔內容跟跑之前一樣。
- `D:\` 與 `D:\HT9045` 最上層沒有多出資料夾；資源回收筒沒有這次的 W906PRB*／W906IMP*。
- `git -C D:\HT9045 status --porcelain -- config web` 跟跑之前一樣。
- 沒有 wb_serve.exe 還在跑；8046 沒人用；標記檔已刪。
- 備份在全部回到原樣之後才刪（照 Jimmy 20260918「備份→驗證→刪備份」）；每一輪的改變清單、主控台紀錄、探針輸出留在 `D:\AI_TempFile\wbrun_c20bdec2\`。

### 6.8 中途當掉怎麼辦

看到 `D:\AI_TempFile\wbrun_ACTIVE.json` 還在 ⇒ 先 `wbrun_guard.py restore <那一輪>`、`check` 到全部未變，才做別的事（包括 build 與 ctest）。

---

### 6.x ⛔ 20260928 補：新版網頁程式的 START 規則（commit `6273f82f` 起）

- Steven 20260928 定了「沒有畫面不准 START」（RULINGS_20260926 S168）：從 `6273f82f` 起，網頁程式在**沒有任何 HMI 畫面連著**時拒絕所有 START（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:1148`）。
- 探針不經過 `D:\HT9045\web\background.html` 直接送 `start.run` 會被拒。要先送一份 `ui.windows.put`，內容 `{"main":{"form":"fMain","state":"open"}}`，並保持 WebSocket 連著（過期但連線還在＝照最後一次回報）。
- 運轉中或馬達在動時畫面全關滿 10 秒，機台照 golden PAUSE 停下並跳 MES1690（R122）；探針跑運轉類測試時，WebSocket 不要中途斷。
- 20260928 晚上第一次跑用的 exe 是 `b4e9549b` 編的，還沒有這條規則；之後換新版 exe 時照上面做。守門腳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\wbrun_guard.py` 的探針參數要跟著加。

### 6.y ⛔ 20260928 晚上第一次實跑的結果與兩個教訓

- 結果：run 資料夾 `D:\AI_TempFile\q50-run\20260928_213235\`（exe＝`b4e9549b` 的 SIM 版），21:32～00:43，20 輪**全部 CLEAN**（真檔全部還原、沒有清單外的變動）；探針 11 個 PASS、20 個 FAIL，摘要在該資料夾的 `summary.md`。
- 教訓 1（開機預期清單）：第 1 輪第一次（19:19）停在「9 個預期外的檔」，Steven 選「加進預期清單、繼續跑」。其中目前配方 `D:\HT9045\IniData\Data\<配方>\HandlerCondition.Data` 的值會被改寫（Clean Pad 顆數、下壓力、清潔次數）——**查證是 golden 原本就會這樣**：golden 開機讀兩次配方（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:9325`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:9337`），每次都先讀再存 Auto Clean 設定（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:2531-2535`）。之後跑要把目前配方、`D:\UnloaderInfo\`、`D:\HT9045_Log\EventLogTxt\`、`D:\HT9045_Log\SaveEventLog\` 放進 `--expect-extra`。
- 教訓 2（測試帳號登入失敗）：20 個 FAIL 大多第一步就是 `auth.login S12TEST` 失敗、之後 `not-authorized`。⛔ 更正（ST01-E 20260929 07:0x，看 run 的 `r\<輪>\console.txt`）：**原因是防連點，不是密碼本**——模擬版開機就自動登入 HonPrec（主控台 `login: mode=book … boot AccessLevel=3 (HonPrec)`），探針第一次 `auth.login` 收到 `already logged in` → `auth.logout` → 約 130 ms 後再 `auth.login`，第二次被 WebCmdGuard 擋掉（主控台 `[cmdguard] busy, not run: cmd=auth.login … arrived 131 ms after the same command finished … window 400 ms`；同一個指令 400 ms 內再送回 busy）。文字版 `W906_PWBOOK_PATH` 其實有生效（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:411`：有指定密碼本就不走 login.dat），先前寫「要改用 W906_LOGINDAT_PATH」是錯的。修法：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\` 9 支探針（formevent_*、evb3_*、q41_closetail／q41_speed_ts7、s12_form_probe 的 ws_login）登出後等 0.5 秒再登入。**寫新探針時**：凡是「同一個指令連送兩次」（重試、登出再登入）中間要隔 ≥ 0.5 秒，或 wb_serve 用 `W906_CMDGUARD_MS=0` 啟動（但那樣防連點本身的測試 A3 要 `--skip-busy`）。下次 Q50 要重跑這 20 個探針確認沒有別的原因。

### 6.z ⛔ 20260929 晚上 Q57（第二次實跑）：開機預期清單要每次都帶，而且清單會長

- 第一次啟動（19:47，run `D:\AI_TempFile\q50-run\20260929_194735`）第 1 輪就停：12 個檔在預期清單外（已全部還原、再比對 0 差異）。**原因之一是 ST01-M 沒帶昨晚 Steven 同意過的 `--expect-extra`**——那幾個不在 `wbrun_guard.py` 內建的 EXPECTED_BOOT 裡，**每次跑都要照帶**。
- 另外 8 個是 `b4e9549b` → `10cac033` 之間新出現的開機寫檔：目前配方的 `Binasgn_ART.Data`、`BinasgnOff_ART.Data`、`Contact.Data`、`Temperature.Data`、`Tester.Data`，以及 `D:\HT9045_Log\ASE log\`（EventTracker）、`D:\HT9045_Log\Heater_On_Off_LOG\`。Steven 20:5x 選「全部加進預期清單，今晚照跑」；這 8 個是不是 golden 開機本來就會寫，交 ST01-E 查（不是就記成待修）。
- 第二次啟動（20:56，run `D:\AI_TempFile\q50-run\20260929_205659`）用的參數（配方名照當時的目前配方）：

```
wbrun_guard.py --all --wb-serve <釘住的 SIM exe> --label "<exe commit>; probes at <HEAD>" ^
  --expect-extra "d:\ht9045\inidata\data\<配方>\handlercondition.data" ^
  --expect-extra "d:\ht9045\inidata\data\<配方>\binasgn_art.data"  --expect-extra "d:\ht9045\inidata\data\<配方>\binasgnoff_art.data" ^
  --expect-extra "d:\ht9045\inidata\data\<配方>\contact.data"      --expect-extra "d:\ht9045\inidata\data\<配方>\temperature.data" ^
  --expect-extra "d:\ht9045\inidata\data\<配方>\tester.data"       --expect-extra "d:\unloaderinfo\*" ^
  --expect-extra "d:\ht9045_log\eventlogtxt\*"  --expect-extra "d:\ht9045_log\saveeventlog\*" ^
  --expect-extra "d:\ht9045_log\ase log\*"      --expect-extra "d:\ht9045_log\heater_on_off_log\*"
```

- **做法**：跑之前先 `--dry-plan`；第 1 輪停在預期外的檔時，把新出現的檔跟上一次 run 的 `guard.log` 第 1 輪比對（哪些是之前就同意的、哪些是新的），列給 Steven 決定，不要自己加。exe 要先複製到自己的資料夾釘住（例 `D:\AI_TempFile\q50_20260929\wb_serve_10cac033_sim.exe`），免得之後的 gate 蓋掉。
- 建議（給 ST01-E）：確認是 golden 開機會寫的，就把它們加進 `wbrun_guard.py` 的 EXPECTED_BOOT，之後不用每次帶。
- ⛔ 20260930 00:1x 更新（ST01-E）：上面這些檔**已內建**進 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\wbrun_guard.py` 的 EXPECTED_BOOT（標 `AI(W906-Q57-BOOT)`），之後跑**不用再帶 `--expect-extra`**。逐檔查過都是 golden 開機寫：目前配方的 HandlerCondition／Binasgn_ART／BinasgnOff_ART／Contact／Temperature／Tester `.Data` 只補缺鍵的預設值（CheckAndReadIniData；golden 開機呼叫點 V912 main.cpp:9293／:9327／:9339／:9341／:9351／:9357、:1045），`D:\UnloaderInfo\`，以及 `D:\HT9045_Log\` 的 ASE log、EventLogTxt、SaveEventLog、Heater_On_Off_LOG、TimeData（log 物件開機就建；TimeData 是整點紀錄，開機跨整點就會寫）。
- ⛔ 同時：探針自己建又刪掉的測試配方（`D:\HT9045\IniData\Data\W906PRB*`／`W906IMP*`、`IniData\Offset\` 同名）被 golden 的刪配方送進資源回收筒，Q57 從 r3i 起每輪都報 NOT-CLEAN（檔案其實 CLEAN）。守門腳本現在認得這種自己的回收項目（讀 `$I` 的原始路徑，只認這兩個前綴），直接刪掉那一對 `$I`／`$R`、主控台印一行，不算殘留；其他回收項目照舊報。

- ⛔ 20260930 02:28 更新（ST01-E 派的工程師分診，標 `AI(W906-Q57-TRIAGE)`）：Q57（`D:\AI_TempFile\q50-run\20260929_210245\`）的 10 個探針 FAIL（summary.md 的「first failure」欄還藏了 3 個：formevent_ta_ts T0、data_observer V／W）**全部不是程式錯**：7 個是探針的預期值比之後照 golden 補的事件舊（事件表長大：`b08ae6ad`、`875d3499`、`f14484e1`、`70aa17e8`、`ff497e5d`；SYSTEM_TEST_IF 788→796 `551c7398`；canary 替身刪掉 `06f8ef0a`；S113 起 golden 每拍累加開機時間 `0b38b6b5`），3 個是探針自己的邏輯或時間（A3 把「changed 裡沒有」當成改回；countersel 兩次存檔只隔 182 ms 被 WebCmdGuard 400 ms 擋；observer 畫面與回應分兩次讀、計時器在中間重畫）。改的 7 支都是 St01 的：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\formevent_probe.py、formevent_cc_probe.py、formevent_ta_ts_probe.py、status_countersel_probe.py、data_observer_probe.py、s1_eventlog_probe.py、data_counterclear_probe.py（commit 見 git log）。**還沒實跑驗證**，下一次 Q50 才算數。
- ⛔ 教訓：探針寫「剛好 N 個」或「值不變」之前，先想這個值會不會照 golden 自己長（每拍累加的計時、整點紀錄、登入寫的事件）；會長的就寫「只能往上、上限多少」，不要寫「不變」。同一個指令連送要隔 ≥ 0.5 秒（同 §6.y）。

## 7. 「做完」長什麼樣

- 一張結果表：25 支探針各一列——探針完整路徑、跑的是哪個 commit 的哪個執行檔（修改時間）、通過／失敗項目數、失敗的原文、主控台紀錄的位置。
- 每一輪一份「這次改了哪些真檔」清單，跟 §2.2 的預期比對過；第 1 輪那份就是**這台的開機寫檔清單**，可以回給筆電對照。
- 每一輪都有「還原後全部未變」的證據；最後 `D:\AI_TempFile\wbrun_ACTIVE.json` 不存在、備份已刪。
- 探針失敗一律記成「待修」清單交給主人（St01 的自己修、別人的走交接檔），不在同一個時段裡邊跑邊改。
- 兩支探針用法的 `--dry` 已拿掉；若選 B，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\wbrun_guard.py` 進樹，之後照同一套步驟跑。

---

## 8. 要 Steven 決定的題目全文（decisions-pending 的 Q50）

### Q50. 要不要在 Steven01 這台，用「先整夾備份、跑完逐檔還原」的方式，實際啟動網頁程式，把還沒跑過的網頁自動測試跑完？

**背景**

- 網頁版畫面的背後有一支 C++ 機台程式（網頁程式，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 編出來的 wb_serve.exe）。它照 BCB6 原版開機、關站，會改這台電腦 `D:\HT9045` 底下的設定檔、配方、計數檔——跟量產機同一套檔。
- 所以目前的規則是「Steven01 不跑網頁程式，除非 Steven 同意」。結果這兩天 Steven01 工程線（St01）寫好或改過的 13 支自動測試（模擬操作員點網頁、檢查 C++ 的反應），改完之後都沒跑過（其中 9 支一次都沒跑過），只確認了「程式編得過」，不知道「行為對不對」。
  例：「權限不夠時設定頁要開不起來」「關掉 Yield 設定頁後要照原版存起動模式」「接觸次數頁每秒自動更新」都還沒驗過。另外 12 支以前跑過的，後來程式改了很多，也沒重跑。
- 以前那種「假寫入」模式（啟動參數 `--dry`）已經取消，帶那個參數程式會直接結束，不能再用。

**選項**

- **A 搬家**：讓程式把所有讀寫都改到一個暫存資料夾。
  - A-1 改程式加一個總轉向開關：要動 Jimmy、Steven02、St01 三個人約 109 個檔、841 個寫死的路徑，還要推翻兩條已經裁決的規則（「配方資料夾被轉向時程式拒絕服務」「不可以有會把存檔導到別處的開關」）。漏一處就等於沒隔離，而且不容易發現。
  - A-2 不改程式，用 Windows 內建的「拋棄式沙盒」（一台關掉就全部消失的小虛擬電腦，真檔只唯讀分享進去）：要系統管理員打開 Windows 功能、重開機；這台目前看起來沒開，公司准不准也還不知道。
- **B 備份還原**：不改任何人的程式。找一段這台沒有別的 build、測試在跑的時間，先把會被改到的資料夾整夾複製備份（約 580 MB、1～3 分鐘），啟動模擬版網頁程式跑測試，跑完逐檔比對、改過的放回、新產生的刪掉，再比一次確認全部回到原樣。
- **C 維持現狀**：這台不跑，把測試的跑法寫好交給 Jimmy 的筆電（他那邊已經這樣跑）或 HT9050 機台去跑。

**建議**：**B**。分四輪、每輪結束都還原：①只開機不測試——確認這台開機會改的檔跟筆電量到的一樣，出現意外的檔就停下來回報；②不寫檔的測試；③會寫配方的測試（一支一輪）；④舊測試回歸。
條件：跑的時段這台只做這件事——替 Steven02 代編暫停（ST01-M）、全量建置測試不跑（ST01-E）、Steven 不按 VS Code F5；執行當下跳出的權限提示由 Steven 本人按允許。第一次整套約 3～4 小時。

**例子**

- 選 A-1：要把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 裡 223 處寫死的路徑（Jimmy 的檔）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\` 底下 43 處（Steven02 的檔）一個一個改；少改一處，那一處照樣寫真檔，而測試照樣通過。
- 選 A-2：請系統管理員打開「Windows 沙盒」；之後每次跑都在沙盒裡，關掉沙盒一切消失，不必挑時段。
- 選 B：今晚 23:00～02:00 這台只跑這件事。第一輪開機 2 分鐘，預期看到 `D:\HT9045\system\ContactInfo.ini` 多了 128 個鍵、`D:\HT9045\Error\BootLog.txt` 多一行、`D:\HT9045\system\lastdata.dat` 被照原版更新——都在筆電量過的清單裡，全部還原；若看到清單外的檔（例如某個配方檔被改），就停下來先告訴 Steven。
- 選 C：St01 把 13 支測試的跑法寫進交接檔交給 Jimmy，他排進筆電的夜間實跑；結果最快隔天回來，有錯要再來回一次。

**請 Steven 回**：A-1／A-2／B／C 選一個；選 B 請給一段時段（以及第 3 輪「會寫配方的測試」要不要一起跑，還是先只跑前兩輪）。

**目前狀態**：C（現況：Steven01 不跑網頁程式，13 支改過的自動測試都還沒跑）；等 Steven 決定。

---

## 9. 這份評估沒能驗證的事

- **這台自己的開機寫檔清單**沒量過（本評估沒有啟動網頁程式）；§2.2 的「預期會變」來自筆電的實測與 St01 的公告。這台的設定跟筆電不同（例 `D:\HT9045\system\Gerneral.ini` 的 IO_CARD_TYPE=1、HEATER_CTRL_TYPE=2，`D:\HT9045\.claude\skills\ops-ht9045-proxy-build\references\gotchas.md`），第 1 輪就是用來量它的。
- 這台依設定會開哪些機台通訊伺服器（SECS/GEM、ATC7、OLP…），以及模擬版的 ATC7 指令會不會真的試著連網路上的位址——沒查設定檔、沒實跑。
- Windows 沙盒能不能在這台打開、公司政策准不准：只看到 `C:\Windows\System32\WindowsSandbox.exe`、`vmcompute.exe` 不存在。
- 靜態普查只抓得到寫死在字串裡的路徑，由變數組出來的路徑沒有涵蓋；數字裡混有少數只是訊息文字的字串。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\showmymessage_probe.py` 到底完整跑過沒有，只有 commit 訊息與註解的間接證據（§1.3）。
- 測試密碼本用等級 3 夠不夠每一頁的權限（取決於那時的 `D:\HT9045\system\levelset.dat`）；St01 以前跑探針用的啟動腳本（探針檔頭提到的 run_sortct.sh、run_showmymessage.sh）當時放在 session 的暫存區，不在 git 裡，現在的位置不明，這次沒找。
- 13 支新探針本身會不會有錯（例 0.4 秒防連點讓連點類檢查失敗）——要跑了才知道；這正是本題要解決的事。

---

## 附：本評估怎麼量的（都在 20260927 晚上，Steven01）

- 探針的加入與最後修改：`git -C D:\HT9045 log --diff-filter=A --format="%h %ad %an" -- HT9011UC_Cpp_V3.33.906.0/tools/webprobe/<檔名>`，以及每顆 commit 的訊息（找「not run」「NOT e2e-tested」「no build」）。
- 靜態普查：本評估在暫存區寫的 Python 小程式，用 `git cat-file --batch` 讀 `c20bdec2` 的 980 個 C++ 檔，去掉 `//`、`/* */` 註解與 `#if 0` 區塊，收集磁碟代號開頭的字串，再依根目錄分類；「有測試縫」＝同一行有 `getenv`、`W906EnvPathOr`、`W906IniDataRedirect`、`W906AuthPathRedirect` 或 `W906_…(` 呼叫。
  粗略版可用 `git -C D:\HT9045 grep -n -I -E "\"[A-Za-z]:\\\\\\\\" c20bdec2 -- HT9011UC_Cpp_V3.33.906.0` 重現（含註解，行數會多一些）。
- 測試縫名單：`git -C D:\HT9045 grep -h -o -E "(getenv|W906EnvPathOr|EnvOr|W906EnvSet)\(\"W906_[A-Z0-9_]+" c20bdec2 -- "HT9011UC_Cpp_V3.33.906.0/*.cpp" "HT9011UC_Cpp_V3.33.906.0/*.h" ":!*/tests/*"` ⇒ 25 個名字，跟 `D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md` §5 一致。
- 誰的檔：`git blame` 指定行＋`git shortlog -sn` 看檔案的主要作者（git 作者「Steven」同時包含 Steven01 與 Steven02 兩台）。
- 資料量：`find`／`du -sm`（20260927 21:4x）。
- 執行檔相依：讀 `D:\AI_TempFile\st02-gb-p1-build\wb_serve.exe` 的 PE 匯入表（只讀檔，沒有執行）。
