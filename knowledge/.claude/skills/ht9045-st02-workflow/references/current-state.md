# St02 現況板（更新：2026-10-02 14:5x，換帳號前收尾）

> 新 session 先讀這份。我是 **St02-E**；派工的協調者是 **St02-M**（session 名稱會變，目前 **github-62**，uds `\\.\pipe\LOCAL\cc-msg-2e6f870585ba6e0d521e943f8e46a02b`（14:00 重啟後 St02-M＝github-62，我＝github-de），以最新訊息的 `from=` 為準）；St01 的協調者是 **ST01-M**，St01 的工程是 **ST01-E**。
> 規則：每個 commit 兩組態編譯、不執行；St01 跑 ctest（§2 那一列要寫測試名＋「請 St01 代跑」，ST01-M 1002 起只代跑這種列）。每次 push 後都更新這份。⚠ 1002 07:4x Steven 對 St02-M 說「你如果能跑得起來的話, 可以做測試」——St02-E 這邊還沒生效（只是轉述，St02-E 的排程指令仍寫「編譯只編不跑」），已在 St02-E 的 session 直接問 Steven，等他回。
> 裁決要問的：一律交給 St02-M，由它轉 ST01-M 彙整給 Steven；不直接問 Steven、不寄信。
> 5 小時觸發：本 session 設了 cron `58 */5 * * *`（只在本 session 有效）。
> Steven 要求加人手：P4、W10、ELA W15／18／19、Q41 由四個 helper agent 做，只在本機 commit；**St02-E 審過後才推**。

## A. 各樹的狀態

| 樹／分支 | 狀態 |
|---|---|
| `D:\HT9045` `v906/steven-gpib-widget` | **已推 `3c434d7f`**（裁決文件、merge main 56039beb、skills＋研究參考）。main 已含 St02 到 014e9094 的所有工作（ed7df426；St01 跑 13／13 過）。 |
| `D:\AI_TempFile\st02-on-cbridge` `v906/steven-st02-on-cbridge` | **已推 `7f0c24b1`**：`64e2c048` Q9、`d1a2aee3` merge St01 de534b23、`7f0c24b1` Q24（兩組態 0 errors；對 St01 1d20e08b／main 56039beb merge-tree 乾淨）。等 St01 合、跑 Security_LoginDatBook。 |
| `D:\AI_TempFile\st02-p4` `v906/steven-p4-wip`（只在本機） | **P4 helper 在做**：3 個 WIP commit＋merge gpib-widget adfe5fd6（`063d2c3c`），之後依計畫做 (a)／選用的 seam 強化／(b)／(c)。obj 根 `D:\AI_TempFile\st02-p4-obj`。 |
| `D:\AI_TempFile\st02-ela` `v906/steven-ela-wip`（只在本機） | **ELA helper 在做** W15／W18／W19（照 research-ela-w15-w18-w19.md）；cObserver.cpp :1351-1385 已認領，但 helper 只給 diff，St02-E 先送 St02-M 做 merge-tree 再套。obj 根 `D:\AI_TempFile\st02-ela-obj`。 |
| `D:\AI_TempFile\st02-q41` `v906/steven-q41-wip`（只在本機） | **Q41 helper 在做**：A 段 Setup.TesterIF TI-1～TI-5＋W6；B 段其他 33 列先出認領清單（page-wire JS 是 St01 登記的，要先經 St02-M 認領；ht9045_wire_engine.js 是 Jimmy 的，不碰；「從資料庫選」下拉不做；Speed 等 ST01-E 同意）。清單：`D:\AI_TempFile\st02-q41-Q41_INVENTORY_20260927_from_St01_23efc733.md`。obj 根 `D:\AI_TempFile\st02-q41-obj`。 |
| `D:\AI_TempFile\st02-w10` `v906/steven-w10-wip`（只在本機） | **W10 helper 在做**：(1)～(9)＋R1／R3／S-a＋R2（照 `research-r2-tcp-command-joining.md`）；702 只在 tick Poll 外面包 try/catch。obj 根 `D:\AI_TempFile\st02-w10-obj`。 |

### 16:13 檢查點（helper 都還沒交件）
- P4：`b4712e5f`（(a) W7＋ARM3），兩組態 0 errors；(b)(c) 在做。
- W10：還沒 commit（18 改＋8 新檔），sim 編譯中。
- ELA：`1042d4cc` W15、`2ceca61c` W18、`82196ca3` W19、`22e97891` 文件；ship 0 errors，sim 重編中；cObserver 的 diff 等回報。
- Q41：還沒 commit（A 段 TesterIF，4 檔），sim 0 errors。
- 16:40：gpib-widget 推到 `bf2690aa`（文件）；本機 `00772497` 已 merge main 00f9a882（有程式）→ 兩組態編譯中，過了再推。st02-on-cbridge 下次推之前要 merge St01 head（現在 5b7fe37c）。

### 10-02 14:5x 狀態（換帳號前收尾；新 session 從這裡接；St02-E 寫）
- **St02-E Session ID：`c8311755-ee3b-4c2b-9f5f-bc5682ac9613`**（session 名 github-de；St02-M＝github-62，uds `\\.\pipe\LOCAL\cc-msg-2e6f870585ba6e0d521e943f8e46a02b`；ST01-M 現在是 github-da）。Steven 14:4x：帳號週用量 90%，要換帳號。沒有 helper 在跑；所有 worktree 都乾淨（都 commit 了）；沒有推半成品。
- main `8f3cdc53`（批 43：!101 C8、!104 C11 已合；套件 125 機台整合 283f8382 已在 main）。
- ⚠ 測試仍然只編譯不跑：Steven 對 St02-M 說過「你如果能跑得起來的話, 可以做測試」，但 St02-E 自己的排程指令還寫「編譯只編不跑」，已在 St02-E 的 session 直接問 Steven、還沒回 ⇒ 照舊 compile＋node --check，§2 寫「請 St01 代跑」＋測試名。
- **已推、等人：**
  - MR !114 ADAM6024（Steven 直接指示）：`v906/st02-adam6024` `0469e8ae`（wt `D:\AI_TempFile\st02-s18`）；EP 寫出開關 `W906_ADAM_EP_LIVE` 預設關；St02-M 14:24 寄信請 Jimmy 審核；St01 14:28 同意 MainClose.cpp :118／:789-793／:1068；筆電的行等 Jimmy；St01 約 16:0x 代跑 4 個 ADAM ctest＋TesterComm_Handler＋St02_Timer3＋MainClose。認領清單 docs/handoff/ST02_ADAM6024_CLAIMS_20261002.md（本機 `D:\AI_TempFile\st02-claims\ADAM_CLAIM_SHEET_vs_main_d14fa206.txt`）。
  - MR !108 0128 補測試：`v906/st02-0128-test` `f176c6db`（wt st02-s17）；§2 請 gate＋St01 代跑 St02_Timer3。
  - MR !105 ADAM skill（.claude/skills/ht9045-adam6024）：`64b62161`；只有 skill，可直接合。
- **本機、還沒推（從這裡接）：**
  1. **LI-9 F1**：`v906/st02-li9-f1-r` `05cf7c6e`（wt `D:\AI_TempFile\st02-s14`，從 main 8f3cdc53）＝`8f94db63`＋`f9eeb124`＋`05cf7c6e`（六條認領全部同意＋WebCmdGuard kExemptNames act.lotInfoFtp.state；St01 條件都照做：#1 #2 同一個 commit、#4 在 FormLock 下、:52「後 22 列」→ 20）。⚠ **sim 建置連結失敗**：St01 新的 test_e021_observer（tests/CMakeLists.txt:6644，f3e2574b）編 ChanAction.cpp 但沒有 JsonBridge/actions/LotInfoFtp.cpp ⇒ W906_LotInfoFtpAct undefined；test_mv_trays（:3566）、test_main_record_clear（:3570，St02 2e58459b2）同樣只列 MainRecordClear.cpp。**接續**：三行都在同一行後面加 ` ../JsonBridge/actions/LotInfoFtp.cpp`（:6644 是 St01 的行＝認領），兩組態重編（log `s09close\li9r_*`），merge-tree 一行再推 MR。
  2. **OB-7（SG_JamCount）→ C6（OB-5）**：E-021 已在 main ⇒ 解鎖；計畫稿 docs/C7_FTPSAVE_SGJAM_PLAN_20261002.md；先對 8f3cdc53 重新推導 Data.Observer.html:234 的認領（批 43 a7e1633f 動過 Observer 頁）。
  3. **C9 Teach（G2＋G1）**：`v906/st02-c9-r` `01c0dc1d`＋`v906/st02-c9-g1` `114f8192`（wt st02-s15），基準是套件 125 之前的 58cb0888。**接續**：rebase 到 main；**拿掉 G3**（機台已接吸嘴 IO 60cc29f6、伺服鈕 71132e21、TTL 反灰 00906385）；保留 G2（36 顆旋轉鈕、Sht Go Latch、Set All Z Move）＋G1（Pitch，#46 A）；全部認領行重新推導（WebMotorAccess.cpp kActions／檔尾、test 釘子、motor-access.json＋shim、HW.teach.html:81、sync_web.py :275），舊的 ST02_C9_CLAIMS_20261002.md 要換掉。
  4. **C10 HANA RMS**：`v906/st02-c10-hana` `83fd05e7`（wt st02-s16，基準 2a2c62c0；`3b34408a`＝筆電 3 行認領，本機）。**接續**：rebase 到 main、對 ST02_C10_CLAIMS_20261002.md 重查行號、等筆電同意後推。
  5. **C12 按鈕普查**（只量測分類、不改程式，在 main 上）：還沒開始；golden 對照 helper 12:5x 被用量上限中斷（scratchpad\c12 有半成品腳本）；需要 headless 瀏覽器實跑，等 Steven 在 St02-E 確認或請 St01 代跑。
  6. **C13**（fLotInfo.Timer1 每 100 ms，呼叫 St01 的 `W906_TfLotInfo_Timer1Timer()`，LotInfo_E020.h:29）：等 E-020（St01 q59 b2047c1c）進 main 才編得過。
  7. **C14 Bin Display（新卡，Steven 14:4x）**：① skill ② C++ 照 golden BinDisplay\MyBinDisp ③ fShowBinSelect tsUnloadMap「Bin Display Status」的網頁。**還沒開始、還沒通知 Jimmy**；換帳號後等 Steven 說開始再做。
- **今天學到的（都寫進記憶或 SKILL §5）**：golden 看不到的按鈕先查；頁面讀設定檔原值之前看 golden 是不是 edit list 固定值／另有預設；同一行加呼叫要放在既有 `//` 前；區塊範圍的函式宣告在匿名 namespace 的函式裡會變成匿名 namespace 的函式（MainClose.cpp:118 的教訓）；靜態初始化的表單建構子工作可能沒跑（0128，記憶 static-init-facade-constructor）；測試函式別叫 Mark／None／Odd／Even／Space。

### 10-02 09:0x 狀態（新 session 從這裡接；St02-E 寫）
- main `8c1afb11`（批 38 之後：!97 C-2 CommView、!98 O19 合進來；!99 C7 計畫稿也在）。**批 39**（筆電 TO_STEVEN §1 08:4x 登記，約一小時）幾乎蓋到 C9 G2 的每一條認領：WebMotorAccess.cpp :95／:955／:957／:4243／:4322／:4325／檔尾／TickGoldenHome、WebMotorAccess.h :202／:415、WebMotorAccessLive.cpp :47／:732、test_web_motor_access.cpp :393＋三個數量行＋新的最後一段、motor-access.json btnAlarmReset 後 5 列、HW.teach.html :191／:379／:506／:611／:715 ⇒ **C9 等批 39 進 main 再 rebase、重新推導每一條 OLD／NEW**（St02 的檔尾區塊接在它們後面）。
- **排隊（St02-M 1002 08:1x）**：C9 G3 → C10 HANA RMS → C9 G1 → LI-9 F1＋LI-10；C8、OB-7 解鎖就插隊。
- **這一批**：`D:\AI_TempFile\st02-s13` `v906/st02-c8`（從 main `8c1afb11`）：C8 TrayForm Tp1／2／3TickUp（Setup.TrayForm.html:133 St01 07:06 同意，main 上那一行跟原本一樣）＋ W65=A（Rs232SetupCodes.h:3）＋這份現況板。
- **本機、還沒推**：
  - `D:\AI_TempFile\st02-s15` `v906/st02-c9` `91715589`：**C9 G2**（Rotate ±90、Sht1/2 Go Latch、Set All In/Out Arm Z Move），兩組態 0 錯誤、nm／DLL 過。本體在 WebMotorAccess.cpp 檔尾一塊；St02 的 ITeachSt02Ops（WebTeachSt02.h）＋live 檔 WebTeachSt02Live.cpp（wb_serve，靜態初始化安裝）；頁面 ht9045_teach_st02.js；ctest ST02C9_TeachSt02＋node ST02C9_TeachSt02Page。**HT9050 上 golden 把六顆都藏起來**（USE_ROTATE_KIT=0 :1652、USE_PICKER_COUNT=4 :1457-1458、grpShtSensor dfm :5850 永遠藏）⇒ 照 golden 拒絕＋藏。**G3 在 HT9050 看得到**（TabSheet10／tsTrayArm／tsTTLTest 沒有 TabVisible）：helper 在做（Lane 走 io.btnPanelClick、Galil 伺服走 Gali→1203 路線、TTL 用既有的）。G1 停著。STOP 停 pitch loop 的偏離＝HUMAN_REVIEW C，等 Jimmy（NIGHT_REPORT §0）。
  - `D:\AI_TempFile\st02-s14` `v906/st02-li9-f1`：**LI-9 F1** `ecab2c6b`＋`c88c3945`，兩組態 0 錯誤；`59a99084`＝別人檔的認領行（只為了本機建置）。6 條認領 St02-M 已貼（handoff e0794de7，docs/handoff/ST02_LI9_F1_CLAIMS_20261002.md）；#6 sync_web.py 被批 38 改過要重新推導。
  - `D:\AI_TempFile\st02-s16` `v906/st02-c10-hana`（main 5af718af）：**C10 HANA RMS**（W64=A，照 912 automation.cpp:2514-3158，RogerYang 註解為準）；helper 在做。
- **這段新學到的**：golden 看不到的按鈕（FormShow 的 Visible／dfm Visible=False）要先查再決定做不做；測試裡別把函式取名 Mark／None／Odd／Even／Space（vclcompat comm.h 的 TParity 列舉）。

### 10-02 05:0x 狀態（新 session 從這裡接；St02-E 寫）
- main `3ff4a55a`（批 33 之後）。10-02 合進 main 的 St02 MR：!88 D-034 計畫稿、!89 LI-12、!90 R126（M4／M5）、!91 U1 測試（第一次被退回：測試錯）、!92 TesterIF 稽核、!93 TesterIF 小鍵盤 P1／P2、!94 VTEST。
- **已推、等合**：MR !95 全頁小鍵盤「取第一個分支」稽核（文件＋唯讀腳本，給筆電 INBOX 140；HIGH 5 列都在別人的頁，由筆電改產生器）。St02-M：TrayForm Tp*TickUp 先保持 60~73（安全那一邊）。
- **等別人**：P3 qwerty.js:71（OLD／NEW 與影響清單已給，等筆電「可以」或「只改 dp 0」）；D-033（CL-4）；H-013 第 3 項（W64）；TCP 等待框 pump（§8 Q2）；E-T1-022 選項 B（Jimmy）；計時器表規則。
- **St02 隊列空的**：St02-M 在向筆電／ST01-M 要卡，不要自己開投機的工作。
- **今天學到的**（SKILL §5 第 11-14 條）：計時器重入保護照 golden 旗標；ctest 種資料種在源頭；頁面小鍵盤對 golden 一般分支；測試的 `_putenv`／`__fastcall` 的 nm。

### 10-02 01:4x 狀態（新 session 從這裡接；St02-E 寫）
- main `8b8209cc`（批 29 之後）。合進 main：!88 D-034 計畫稿（文件）、!89 LI-12 Lot Info 的 Tester TCP Show。
- **批 30 合進 main**：MR !90 `v906/st02-r126` `cec3aff1`：NB2 R126——M4 分派函式只對 golden 有旗標的計時器（Timer8／TimerESD／Timer1）做重入保護，Timer3、TemperatureStorage 直接 Due()；M5 RecordTemp 照翻、G4 打開（Observer 溫度歷史以前全 0）；G9／G17 理由、MainClose :2895／:500、LEDGER :511；G-023 我們自己的一輪（舊行號、B05 關那個不可能失敗的檢查）。
- **已推、等筆電 gate**：MR !91 `v906/st02-u1-test` `528d632a`：G023_OSReport 第 12 段，結批摘要 BY SITE 三列（G-023 C 組）＋U4。只改測試；跟 !90 的分支合也乾淨。
- **G-023 還沒測到的**：U2（P6 網路路徑守衛，需要可寫的假網路位置）、U5-U15（清單在 MR !90 說明）。NB2 的 G-023 細目筆電在要（CHAT_JIMMY 00:49），來了再補。
- **TesterIF 欄位稽核**（census 129 C3-119）：`HT9011UC_Cpp_V3.33.906.0/docs/TESTERIF_FIELD_AUDIT_20261002.md`（這張文件 MR，現況板跟著推）。C 路 88／88 跟 golden 一致、舊 28 個 PENDING 全死；真的問題在頁面的小鍵盤（6 個最大測試時間被卡成 INTEGER 60..9999、3 個 Initial Start Delay 下限 30——產生器取了 golden 第一個客戶分支；qwerty.js:71 把 13 個 0 位小數的 double 四捨五入）。下一步等 St02-M 分派。
- **D-034 結案**（S25＋SECS 主機連線沒活），計畫稿 `HT9011UC_Cpp_V3.33.906.0/docs/D034_PLAN_20261002.md`。
- **等別人**：D-033（St01 CL-4 還沒進 main；做的時候用函式指標或一個符號的後備，讓連 HandlerGpibMsg.o 的測試程式照樣連得過）、H-013 第 3 項（W64 Q1）、TCP 等待框 pump（§8 Q2）、E-T1-022 選項 B（Jimmy）、計時器表規則（E-T1-029／003 不動）。OB-5 是新設計，先不做。
- **這段新學到的**：MinGW.org 嚴格模式不宣告 `_putenv`，測試裡用 tests/test_agv_e84.cpp:158-163 的 HT9045_TEST_PUTENV 守衛（記憶 steven-nb3-toolchain）；分派函式的重入保護要逐支照 golden 有沒有「正在跑」旗標決定。

### 10-02 00:3x 狀態（新 session 從這裡接；St02-E 寫）
- main `2138c080`（批 28 之後）。10-01 晚上到 10-02 00:2x 合進 main 的 St02 MR：!79 H-013 第 1、2 項、!82 E-019 盤點、!83 DIO Delete、!86 INBOX 126（第 10 段 107 s → 1.6 s）、!87 cMyDB H1。St02 沒有在 gate 的 MR。
- **D-034（St02 那一半）結案、不寫程式**：SECS 操作員 ID 的密碼框＝golden CheckEmployeeID，只在 bUseN07_5 開（KYEC_LEE／JSCC_OS／SCC）⇒ S25；SECS 主機連線也還沒活。計畫稿收在 `HT9011UC_Cpp_V3.33.906.0/docs/D034_PLAN_20261002.md`（這一張文件 MR，現況板跟著推）。
- **下一件：LI-12**（E-019 自己的列）：Lot Info「Tester Log」分頁的 Tester TCP Show 鈕（golden uLotInfo.cpp:13841-13844）照 Q41 TI-3 開 testercomm.html 的 TCP/IP 分頁（寫 localStorage `ht9045.testercomm.tab` 再 postMessage open）。St01 的只有 `web/page/Data.LotInfo.html:417`（St02-M 已在 FROM_STEVEN §1 認領，03e33e71），其他放 St02 的 js；沒有 C++。本機先做好，St01 說「可以」才推。
- **等別人**：D-033（St01 CL-4 還沒進 main）、H-013 第 3 項（W64 Q1）、TCP 等待框 pump（§8 Q2）、E-T1-022 選項 B（Jimmy）、計時器表規則（E-T1-029／003 不動）。OB-5 是新設計（MDB 改 CSV 查詢），先不做。
- **這段新學到的**：`__fastcall` 函式的符號 `nm -C` 解不開，掃 exe 要用沒解碼的名字（記憶 fallback-symbol-checks）；建置目錄會留著別的分支的舊 exe，判斷前先看分支上有沒有那個 target。

### 10-01 21:4x 狀態（新 session 從這裡接；St02-E 寫）
- main `4b890ffb`（批 24 之後）。今天 St02 合進 main 的：!46 testercomm Test Result、!48 S-15、!51 W58、!52 ELA 加速、!59 S-13、!62 D1、!66 散熱風扇 B、!69 S-14、!74 E-T1-022 選項 A、!75 G-023。
- **已推、等筆電 gate**：MR !79 `v906/st02-h013` `96041758`：H-013 第 1、2 項（golden 912 的 `IsSafePLCIOInstall()` 與 `fSecsAlarm` 條件，裁決 11＝B）。新檔 SafePlcIOInstall.cpp、SecsAlarmForm.{h,cpp}（ht9045_globals）；Command.cpp 5 行、cmydef.h 1 行、CMakeLists.txt:517；新 ctest H013_Terms。行為不變（fSecsAlarm 一直是 NULL）。請 St01 代跑 H013_Terms、TesterComm_TcpCmdServer、I125_RealDummy、FShow_Audit。
- **這一批**：`v906/st02-e019`（worktree `D:\AI_TempFile\st02-s14`，從 main `4b890ffb` 開）：E-019 資料／狀態頁的 golden 事件盤點，只有文件 `docs/E019_DATA_STATUS_EVENTS_20261001.md`（helper 寫、St02-E 審），這份現況板也跟著它推。
- **22:3x 本機、各自一張 MR**（St02-M 21:5x／22:1x 排的）：
  - `v906/st02-cmydb-h1` `1e9290d9`（worktree `D:\AI_TempFile\st02-s13`）：cMyDB.cpp:668-670 照 golden cMyDB.cpp:504 呼叫 TimerRecordLoaderDate（`W906_MainClose_TimerRecordLoaderDate`）；test_ga1_cmydb.cpp:249 加替身。行為改變：每次 MyDBIProductionData 多寫兩列 PROCESS（HANDLER LOG／EventLogTxt），要列 HUMAN_REVIEW。
  - `v906/st02-i126-htset322` `33b545ac`（worktree `D:\AI_TempFile\st02-i126`）：INBOX 126，只改測試：TesterComm_TcpCmdServer 第 10 段兩個 bin、段尾還原，補 BinasgnOff.Data／Binasgn.Data 的檢查。
  - `v906/st02-dio-delete` `db9855af`（worktree `D:\AI_TempFile\st02-dio`）：C1-006／C3-004 兩個 DIO 頁的 Delete 鈕（新檔 web/page/ht9045_dio_delete.js，走既有的 ttlcfg.op）；pagewire 的 Setup.DIOInterfaceCfg.html 副本是筆電的，沒動。
- **等別人**：
  - H-013 第 3 項 HANA RMS（拿掉 G9）：等 W64 Q1；
  - TCP 等待框的 pump：St02-M HOLD，等 Steven 回計時器表 §8 Q2；
  - E-T1-022 選項 B（`wb_serve.cpp:4575` 的 30 ms）：等 Jimmy；
  - **計時器表規則**（RULINGS #39，Jimmy Draft MR !70）：保持現在的分派函式寫法，E-T1-022 之後不再加計時器，等 Steven 回 §8 Q1／Q2 ⇒ E-T1-029／003 和 DIO 兩頁先不動。
- **之後的工作**：D-033／D-034（等 St01 的 CL-4／D-026 合進來）；cMyDB.cpp:668 改用 `W906_MainClose_TimerRecordLoaderDate`。
- **這段新學到的規則**（都已寫進 SKILL.md §5 或記憶）：
  - 被計數的釘值（SecsCatalogue 的 SV／EC 數、START 普查、回覆字串數）跟改動同一張 MR 一起改（!62 漏改，筆電改 769→772）；
  - 別的函式庫要呼叫的新全域符號放在自己的檔；nm 確認真函式是全域 T（匿名 namespace 是小寫 t，後備會悄悄頂替）；受保護的測試（test_ela_ftp）用 `objdump -p` 看有沒有多出 DLL（W58 曾經把 WININET 帶進 ELA_Ftp）；
  - CRLF 用 Python 數位元組，不要用 grep；
  - RULINGS #40：程式／設定檔的密碼照 golden 翻、照常推，掃描只做參考；**共用區 7z 交付密碼照舊不寫進任何地方**；
  - 不要動已凍結或在 gate 的 MR；tests/CMakeLists.txt 檔尾的衝突由筆電解。
- worktree：`st02-speed`（建置，detached）、`st02-s13`（h013）、`st02-s14`（e019）。

### 10-01 16:4x 狀態（新 session 從這裡接；St02-E 寫）
- main `caae69bb`：Steven 16:05-16:13 在 GitLab 直接合了 MR !46（testercomm Test Result 分頁）、!51（W58）、!52（ELA 加速），還有別人的幾張（NB2 !47、Frank、Ifor、St01、RogerYang）。St02-E 照 St02-M 的要求把 main 原樣兩組態建置：0 錯誤。St01 代跑 ctest（d897ffe1）。
- **已推、等筆電 gate**：MR !48 S-15，新 tip `e78edd3f`＝限速修正 `d93485ae`（截止時間寫法）＋合 main `caae69bb`。筆電先擋著 !48，等 St02-M 重新送 gate。
- **這一批**：`v906/st02-s13`（worktree `D:\AI_TempFile\st02-s13`），疊在 !48 上（MR 說明寫「疊在 !48、請先合 !48」）。
  - golden TimerESDTimer（E0／E2-E5 照翻、E1／E6 照 S25 閘住）、等待框時也跑的分派函式（wb_serve.cpp:7621）、AutoRetest.cpp:284、CMakeLists.txt:2776。
  - `W906_Main_TimerRecordLoaderDate` 的後備放在 St02 自己的 `MainTimerESDFallback.cpp`（只有這一個符號）。不能放進 St01 的 `FileRW/_fallback.cpp`：那個物件一被抽出來，裡面的 FileRW 替身就會跟測試直接連結的真 FileRW 撞名（第一次建置 test_ga2_c1_cinitial 失敗）。
- **下一批**：D1 PP_MUSIC＋E-SMC-003（worktree `D:\AI_TempFile\st02-d1`，`98187dc8`）。E2 先依她的認領稿審；MR 說明要列上機項目（SecsGemPath、LotData1.txt、2DBarCode 路徑）。
- **之後的順序**（St02-M）：S-14＋E-T1-022（soak 倒數先跟 Ifor01 對好）→ 散熱風扇 B（H1-08，St01 的 _EditList.cpp +8／_fallback.cpp +1，推之前建置每一支連到 _fallback.cpp 的程式）→ cMyDB.cpp:668（確認每支連 cMyDB 的程式也連得到 ht9045_sm 的後備）→ E-T1-029／003（我們的分派函式）→ DIO 兩頁（web/page/*.DIOInterFaceCFG.html）。
- **Steven 1001 直接交代**：testercomm 共用分頁叫 Test Result、Log 叫 MemoLog；debug 全部分頁、release 只留使用中的介面（都在 `references/testercomm-page.md`）；「S13 ok」。
- **新規則**：
  - 建置 log 的錯誤數用嚴格條件 `: error:|fatal error:|FAILED:|undefined reference|multiple definition`；
  - MR 有增減啟動路徑就重跑 `tools/start_sites_census.py`，同一張 MR 一起改釘住的數字（現在 34／32／2）；
  - 給其他程式用的後備符號放在自己的 archive 成員，不要放進已經有別的後備的檔。
- worktree：`st02-speed`（建置）、`st02-prep`（tc-shared-panel，已合進 main）、`st02-s13`、`st02-d1`、`st02-mainscan`（唯讀，detached main；用完切回 main）。

### 10-01 14:0x 狀態（St02-E 寫）
- main `c5be6093`。筆電第 18 批已上 main：base `TfMain::Pause` 轉到 PauseFromWeb、SECS REMOTE_START 走 `W906_RemoteRunStart`。第 19 批 gate 中，會一起合 MR !31／!33／!34。
- **已推、凍結、等筆電合**：
  - MR !31 `v906/st02-h008` `17cfeb30`：TCP 測試機連線接成真的（vclcompat TClientSocket 的輪詢模式）；
  - MR !33 `v906/st02-h012` `068164b5`：GPIB 遠端 START 走 `W906_RemoteRunStart`，STOP 照 golden `BtnPauseClick`；
  - MR !34 `v906/st02-coolfan` `891a8dc5`：散熱風扇 A＋ctest CoolFan_Rules；
  - MR !46 `v906/st02-tc-shared-panel` `7f21ca68`：testercomm 首頁共用面板，JSON `testercomm.home/1`，做法見 `references/testercomm-page.md`。
- **這一批**：`v906/st02-s15`（S-15：Timer8Timer＋TimerTemperatureStorageMinuteTimer；筆電 13:0x 同意 :605）＝`730a24ba`＋merge main `0d388fe1`＋這份現況板。兩組態建置過了就開自己的 MR。
- **接著**（St02-M 14:0x 排的順序；每批一張 MR、兩組態建置、全部掃描、merge-tree 一行；共用檔只動認領的那幾行）：
  1. W58 第一階段：`v906/st02-w58-simnet` `70036807`（E2 的小修改在上面）。Steven 13:4x 裁決 W63＝A：模擬保留 [N07-1] SECS GEM 和 [N07-2] host start，其他 58 列照舊遮。先 merge main 再建置；這份現況板到時會衝突，取 main 的版本；
  2. S-13：筆電已同意 `AutoRetest.cpp:284` 拿掉 static；`wb_serve.cpp:7621` 加 `W906_St02TimersTickFromModal()`，筆電和 St01（56b92e9b）都同意。呼叫放在 `W906_A01AutoLogoutTick(); }` 之後、`/*AI(W906-D015-A01b)` 之前，也就是那一行結尾的 `//` 之前；用行號定位，同樣的錨點文字 :5953 也有；
  3. 散熱風扇 B：`v906/st02-coolfan-b` `fd2c2c09`。St01 同意：H1-08 由 St02 加 `_EditList.cpp` 檔尾 +8 和 `_fallback.cpp` 檔尾 +1；
  4. cMyDB H1：`_fallback.cpp` 檔尾 +1（`W906_Main_TimerRecordLoaderDate` 的 fallback），St01 同意、由 St02 加；
  5. S-14：soak 倒數先跟 Ifor01 對好誰做。
- St01 不代跑 !31／!33／!34 的 ctest，由筆電第 19 批的 gate 跑。ELA_Schedule 在 St01 的出貨組態 gate 跑到 600 s 逾時，由 E2 唯讀看，St02-E 不用處理。
- **本機、等別人**：
  - `v906/st02-fidelity-1001` `38dac18a`：HOLD，併進下一批檔案重疊的批次。
- G-031 轉換器歸 St02，等 Ifor01 的 diff 清單（`docs/G031_FORMAT_DIFF.md`）出來再做。
- 密碼掃描：推之前 scratchpad `s09close\pwbranch.py <base> <tip>`（完整候選集：新加的行、commit 訊息、改到的每支檔全文，不印值）；commit 前跑 St02-M 的 `ctrl_check.py`（TAB 也算錯）。

### 10-01 10:0x 狀態（St02-E 寫）
- main `e240a3f4`（筆電 batch 14 文件；RULINGS_20261001 #0 常設授權：照 golden 翻、功能面全部接上，取代「安全＝佇列」）。
  - 已合進 main：MR !14 T3、!15 Q3、!16 SoftStart、!19 R146、!20 重掃 3 列、!21 W58 第二階段 A（`9375609f`）。
  - **已推、凍結、等合**：MR !22 `v906/st02-tc-g1g5` `c345e91a`（HandlerGpibMsg G1／G5 改叫 btClearBarcodeListClick）。
- **本機、等 Steven 答覆（預設照 Jimmy #3）**：`v906/st02-w58-simnet` `e3127609`：W58 第一階段 58 列（N07-1／N07-2 不遮）、原子寫回＋ctest SimNet_Write、`W906_SIM_NET_KEYS` 拿掉。
  - tip 兩組態 0 錯誤；`v906/st02-w58-claimtest`（NEVER PUSH，認領行）重疊後兩組態 0 錯誤＋nm；§5／密碼（新加的行＋全文）／反斜線都 0。
  - 推的順序：merge main → 最後的 tip 兩組態建置 → merge-tree 一行 → 自己一張 MR。認領行（cprod.cpp 4 行、wb_serve.cpp:4052、CMakeLists.txt:3321）要筆電同意才進 main。
- **本機、小**：`v906/st02-fidelity-1001` `38dac18a`（從 e240a3f4 開）：HandlerBridgeCtl.cpp F1 的 `/sizeof(bool)`、TcpPump.cpp ON_LINE 臂的 `Visible=false`，都在 `#if 0` 裡。
  - **HOLD**（St02-M 1001 10:1x）：不單獨開 MR（只改 `#if 0` 裡的字，單獨一張會讓筆電多跑一次 gate）。併進下一批檔案重疊或相鄰的 St02 程式批次（G14 解閘或 W58），cherry-pick 進去，並在那張 MR 的回報寫明；不要再單獨重建。
- **Ruling-0 清單**（`docs/handoff/ST02_RULING0_CANDIDATES_20261001.md`）：St02 的檔裡只為安全閘住的只有 G14（GPIB AUTO_CLEAN，等筆電 CL-4 翻 btnAutoCleanClick）；cDIOStatus.cpp:71 TTLLog 等筆電拆 cpublic.cpp:667。兩個都等筆電回答，**不要先動**。
  - **S25 維持**（St02-M 1001）：客戶專屬程式不移植是 Steven 的範圍裁決，#0 沒提到，不自己解讀；LogObjects.cpp 那 2 道照舊。
- 帳號 1001 08:52 用到週上限（10/5 16:00 重置，ST01-M）：不做投機的重建，文件 commit 一起推。
- 等別人：37 道 TesterComm 閘要別人的成員（要不要擬認領稿，問過使用者，還沒答）；筆電說「換」才改 5 支測試檔的測試資料。

### Steven 20261001 直接交代（本 session；St02-M 轉 ST01-M 登記）
- 「TTL 介面是跟RS232整合再一起的」：TTL 就是 RS232 引擎的一部分（TesterComm/Rs232，D:\RS232Log\BinLog_TTL），不另開一塊。
- 「ISA卡片版本先不移植」：ISA／舊卡那段（GpibCore.cpp 載 PCI_L112C／L122C／CMnet DLL 的程式，golden 905 本來就在 /* */ 裡）照舊閘著。
- 「必要的時候要pull」：D:\HT9045 的 gw 落後時先 fetch，快轉到 origin/main（只在乾淨、純快轉時做）。20261001 已從 05525b69 快轉到 813ca9dd。
- 回答過的：ELA 是 C++ 直接讀 EventLogTxt CSV＋運算、JSON 只用在 /api/ela 傳給 eventlog.html；GPIB／RS232(TTL)／TCPIP 各自的站台面板與 log（UiChannel 一個引擎一條；D:\GPIBLOG、D:\RS232Log、D:\HT9045_Log\TCPIP_Log）。**已被 Steven 更正**：Site01～32 和主要通訊 Log 只有一份、在首頁，C++ 送一個 JSON；log 檔照舊各寫各的（`references/testercomm-page.md`，MR !46）。
- TesterComm 64 道 `#if 0` 逐道重掃（20261001）：直接能解 0；Gpib／Rs232 都是畫面效果或 ISA；Handler 36＋Tcp 1 要別人的成員（fSCKART 10、ATC 14、fContact 6、AGV 2、Observer 2、其他 3）；只有 G1／G5 能做 → `v906/st02-tc-g1g5`。明細 scratchpad `s09close\tcrescan\tc_gates.tsv`。

### 10-01 01:0x 狀態（新 session 從這裡接；St02-E 寫）
- main `7d641747`。**已推、凍結**（都不要再加 commit）：
  - MR !11 `v906/st02-levelset-gate` `2985f3b8`、MR !12 `v906/st02-w58-4-simtcp` `05525b69`：等筆電合；
  - **MR !14** `v906/st02-h022-t3` `a191966a`：T3，Observer 版本兩格（筆電同意 MessageDef、ST01-E 同意 cObserver diff）；
  - **MR !15** `v906/st02-q3-fconfig` `e931eed3`：Q3 fConfiguration 8 列（St01 R1 FormLock／R2）。另外在 St01 的 5 個測試檔尾加了空的 FormLock，St02-M 已請 ST01-E 事後確認。
- **本機、等筆電**：
  - `v906/st02-w58-simnet` `e6c7afb1`（W58；最新程式 commit 是 `db0b0952`，之後只有現況板、nit、techniques）。筆電同意 6 行認領行後，照 !11 → !12 → W58 的順序推。推之前要 merge main，並在最後的分支頭重跑兩組態完整建置（St02-M 20260930 22:4x）。
  - `v906/st02-w58-claimtest` `1b2b48a4`：NEVER PUSH。
  - `v906/st02-levelset-softstart` `2d503370`：等 !11 合進 main。
  - `v906/st02-s09-rescan` `5d766431`：重掃 3 列。
- **認領稿**（`D:\AI_TempFile\st02-e2\review\`）：W58 認領稿（已送）、`ST02_W58_Q5_HANDLER_CLAIMS_20260930.md` 第 2 版（A 請現在同意、B 可以等；St02-M 經 §1 送）、R146 第 2 版、重掃 3 列。
- 規則更新：techniques §5「只有開著的網頁才更新資料」（RULINGS #12＋stage F BOOT_TAGS）。

### 09-30 21:4x 狀態（新 session 從這裡接；St02-E 寫）
- main `12fe15e4`（筆電第 6 批）。MR !11 `2985f3b8`、MR !12 `05525b69` 都還沒合；都凍結、不要再推。gw 本機＝`05525b69`。
- **本機分支，都還沒推**：
  - `v906/st02-h022-t3` `8c7c6c8c`：T3（St01 選乙），從 origin/main 開。等筆電同意 MessageDef.cpp 檔尾 10 行／.h:390；同意後 merge main → merge-tree → 開自己的 MR，標題「St02 h022-t3：Observer GPIB／TTL RS232 版本兩格照 golden 顯示」。cObserver.cpp diff 已交 ST01-E（`ST02_H022_T3_COBSERVER_DIFF_20260930.patch`）。
  - `v906/st02-w58-simnet` `db0b0952`（7 個 WIP＋現況板）：W58 照 Steven 五題做完，E2 審查（0 blocking／0 major）的 m1／m5／n1 也改了，疊在 !12 上（E2 !12 m1 改 !12 那一行）。認領稿已由 St02-M 送筆電（handoff 17496cf7）；**筆電 OK 才推**，順序 !11 → !12 → W58，回報要寫疊在哪裡。
  - `v906/st02-w58-claimtest` `1b2b48a4`：W58＋認領行（cprod.cpp :171／:3217／:3263／:3323、wb_serve.cpp:4052、CMakeLists.txt:3321），**NEVER PUSH**，只為了把 glue 編進 wb_serve。認領行用 scratchpad `s09close\w58_claims.py` 套（先核對 OLD）。
  - `v906/st02-levelset-softstart` `2d503370`：等 !11 合進 main。
  - `v906/st02-s09-rescan` `5d766431`：重掃 3 列，等筆電。
- **認領稿**（`D:\AI_TempFile\st02-e2\review\`）：`ST02_W58_CLAIMS_20260930.md`（已送筆電；送出的版本 `_as_sent.md`，§6 之後補了 E2 的 Human review）；`ST02_W58_Q5_HANDLER_CLAIMS_20260930.md`（第二階段草稿，還沒送，St02-M 之後一起送）；`ST02_R146_CLAIMS_V2_20260930.md`（照 E2 M1／m1 改過，改之前的留 `_before_m1.md`）；T3 第 2 版；重掃 3 列。
- **W58 的決定**（照 golden 和研究結果自己定、已告知 St02-M）：C 表收 15 個會連網路的鍵，加上 N14-8／N14-9，鍵表共 60 列；A75／N14-21 等刻意不收。Q3 隱藏且固定為 0 的條目不遮。Q5 用 IsRemotePath 的規則，`W906_SIM_NET_PATHS=1` 才寫，只做 ELA；Handler 端之後另外認領。研究報告：scratchpad `s09close\w58_research.md`。
- **不要碰**：JsonBridge/ChanMvTrays.cpp、ht9045_mv_trays.js（RULINGS_20260930 #12 歸筆電）。
- 沒有別的工作；要開需要新認領的工作之前先問 St02-M。

### 09-30 20:0x 狀態（新 session 從這裡接；St02-E 寫）
- **已推、凍結**：`v906/st02-levelset-gate` MR !11（`2985f3b8`），等筆電 gate；不要再推。gw 本機已快轉到 `2985f3b8`、**沒推**（遠端 gw 照舊 `236adb8e`，已在 main）。
- **排隊（MR !11 合進 main 之後）**：`v906/st02-levelset-softstart` 開新 MR：WebLevelSet.cpp:712 `if (SystemStart || SoftStart)`，文字照 FileRW/_FormEvent.cpp:77-81（E2 審查 m1）。
- **等別人回的認領稿**（都在 `D:\AI_TempFile\st02-e2\review\`）：
  - `ST02_H022_T3_CLAIM_20260930.md`（St01 選「甲／乙」）；
  - `ST02_R146_CLAIMS_V2_20260930.md`（筆電）；
  - `ST02_S09_RESCAN_CLAIMS_20260930.md` 3 列（筆電；本機 `v906/st02-s09-rescan` `5d766431`）。
- `ST02_TODO_RECONCILE_20260930.md` 已交 St02-M，轉 ST01-M 改 todo.md。

### 09-30 19:0x 狀態（St02-E 寫）
- **這批要推的**：`v906/st02-levelset-gate`（在 `D:\AI_TempFile\st02-speed`）＝gw `51f54ce4`＋合 main `f740d2b9`（`6ba9bc19`）＋WebLevelSet 存檔重查設定選單閘（St01 FYI 18:33；Q25=A＋St01 OpenGateRefused 的延伸，St02-M 18:4x 同意）。推的時候照 SKILL.md §3 開 MR，標題照 St02-M 給的。
- MR !6（到 `236adb8e`）已經進 main。gw 本機 `51f54ce4` 只多 3 顆文件 commit，都在這批裡。
- S-09 重掃認領 3 列：`v906/st02-s09-rescan` `5d766431`，照舊只在本機、等筆電，**不要跟這批混**。認領文件已補「會寫的檔」（1690）。
- 新規則：解寫檔閘的認領要列出確切會寫的檔（SKILL.md §4）。

### 09-30 17:4x 狀態（St02-E 寫）
- **S-09 收尾（St02-M 派工）**：
  - (b) `st02_status` 填完：`D:\AI_TempFile\st02-e2\review\ST02_IF0_BACKLOG_STATUS_20260930.tsv`（第 2 版；第 1 版 `_v1.tsv`）。St02-M 會放到交接分支。
  - (a) 重掃：`D:\AI_TempFile\st02-e2\review\ST02_S09_RESCAN_CLAIMS_20260930.md`。St02 自己的檔沒有可以解的；18 列交筆電。
- **本機、沒推**：
  - `D:\AI_TempFile\st02-speed` `v906/st02-s09-rescan`：`22930db9`（gw＋main `8e5a1b1b`）＋`5d766431`（3 列：asortarm 1682／1690、cmydef 610）。**等筆電同意**才推，推的時候照 SKILL.md §3 的新規則開一條 `v906/st02-s09-rescan`＋一張 MR。
  - 18 列那版 `5e887d2d`（`v906/st02-s09-rescan-claims`）已被取代：E2 審查 M1／M2，旋轉 kit 15 列撤回，等 E-001 回覆後連同 n2-7／n2-9、n5-G9／G13 做成一包（cinitial.cpp 先看 §1）。
  - gw 本機 `d2f877de` 之後的文件 commit（現況板、SKILL.md 推送段、techniques §6）等下一次程式推送一起推。
  - Q3 `4cdf1af3` 照舊等 ST01-E。
- **新規則**：推送回報要有「Human review:」一行；一批一條短分支一張 MR（SKILL.md §3、§5）。
- **之後要做（St01 phase A 上 main 之後）**：ChanMvTrays 在 `W906_StageMotionViewTrays` 開頭，`W906_PageStreamWanted("motionview")` 為 false 就 return（約 5 行，St01 評估 stream-by-open-page.md §4.4）。
- 腳本在 session scratchpad 的 `s09close\`（treestate、records、fill、rescan_pop、rescan\flipscan／alive／single／rescan_status）。

### 09-30 17:xx 狀態（St02-E 寫）
- **gw＝`236adb8e`（已推，MR !6）**。今天下午推的：
  - `232cb4e3`：near-miss Q1，11 列；
  - `08e16cf1`：Tray Edit 的 Contact 入口 `contactEditTray`；
  - `d5e348a7`：釋放檔 CalTrayICCount 2 列；
  - `236adb8e`：筆電逐行回覆後的 6 個閘，包括 aTester 前／後。
  - 其中 `08e16cf1` 已進 main（`cae0d4b4`）。
- **本機、沒推**：
  - `v906/steven-q3-fconfig` `4cdf1af3`（Q3 fConfiguration 轉接），等 ST01-E 回 R1／R2。ST01-E 同意後要先 merge gw 最新的 tip。
  - `v906/steven-freed-decisions` `8e2095c8`：只是給筆電看的 6 列，已被 `236adb8e` 取代，不要推。
- **等人的**：
  - Q3（ST01-E）；
  - FT／RT 4 段：要等 review6 的 a9f41d24 進 main，也要 St01 補 hook 或後備。GPIB 傳 true、TCP 傳 false。
- **不能碰的**：
  - 筆電的 INBOX 115 B 十段（l2n worktree，還沒推）：PowerSavingMode、VacuumUnit、ainarm9045、cinitial、csystem、uHeaterThread；
  - INBOX 121：mymotor.cpp SetHTrayPanel、cinitial.cpp SetSimuScreenPara；
  - INBOX 125：auto9045.cpp，以及 Command.cpp 的 :8364、:9908、:13178、:13697。
- 文件都在 `D:\AI_TempFile\st02-e2\review\`：
  - `ST02_S09_FREED_SCAN_20260930.md`（第 2 版）
  - `ST02_S09_FREED_STATUS_20260930.tsv`
  - `ST02_S09_FREED_DECISIONS_20260930.md`
  - `ST02_S09_Q3_FCONFIG_CLAIMS_20260930.md`

### 09-30 12:xx 狀態（St02-E 寫）
- **gw＝`232cb4e3`（已推，MR !6）**：near-miss Q1 的 11 列（`329b6f18`）＋merge main `fb46c597`。筆電 10:5x 同意 Q1；St02-M 已請筆電跑關卡。
  - gw 到 `79959a49` 已經在 main 上（`e2dac07e`）。
- **本機、沒推**：`D:\AI_TempFile\st02-speed` 的 `v906/steven-q3-fconfig` = **`4cdf1af3`**（從 232cb4e3 開），等 ST01-E 回覆 R1、R2：
  - 做的是 Q3＝A：fConfiguration 8 列改走 FileRW 轉接；
  - St01 的檔：_EditList.cpp、_fallback.cpp 檔尾各加一段；
  - 認領稿：`D:\AI_TempFile\st02-e2\review\ST02_S09_Q3_FCONFIG_CLAIMS_20260930.md`。
  - ST01-E 同意後：merge 最新 main，兩組態完整建置，merge-tree 只印一行，再併進 gw 推。gw 上如果還有本機文件 commit，用 merge，不用 ff。
- St01 B8 CT-3（Contact 的 Edit Tray）：
  - 答案：走 `act.trayEdit`，但要在我們的 WebTrayEdit.cpp 補一個 op `contactEditTray`，本體＝golden cContact.cpp:16853-16859。
  - St02-M 說要做 → 已做：`v906/steven-s10-contact`（從 232cb4e3 開），`contactEditTray` 本體＝golden :16855-16858。守門是「已經開著就拒絕」（golden ShowModal）。fshow 基準線 12 → 13。
  - 不要讓 St01 在 fContact.cpp 直接呼叫 EditTray：TrayEditForm.cpp 只編進 wb_serve，ht9045_forms 的測試會連結失敗。
- 已經不用的本機分支：`v906/steven-nm-claims`（內容已在 gw）。

### 09-30 06:3x 狀態（St02-E 寫）
- **gw＝`a6003d06`（已推，MR !6）**：7d7efee8 之上只改 TrayEditForm.cpp 12 行註解。這是 St01 條件 (b)；(a)、(c) 查過不用改。
- **本機、沒推**：`D:\AI_TempFile\st02-speed` 的 `v906/steven-nm-claims`（從 main `f7a2f5b5` 開）。E2 審查後合成一個 commit **`329b6f18`**，等筆電回覆：
  - Q-INC 2 共 11 列（135 撤回）＋過期註解標 STALE＋cObserver:5733 行尾反斜線拿掉。
  - JCET 整題撤回（S25），`e6a67b28`／`c34ed3b0` 已經拿掉。
  - 認領稿第 2 版：`D:\AI_TempFile\st02-e2\review\ST02_S09_NEARMISS_CLAIMS_20260930.md`，Q1＋Q3。
  - 這份現況板和 techniques §8 的更新只在 gw 本機 commit：照 St02-M 的提醒，等下一次程式推送一起推。到時候用 merge 把 nm-claims 併進 gw，不用 ff。
  - 已交 St02-M 發布、請 E2 審。
  - 筆電 OK 後：先 merge 最新 main，兩組態完整建置，merge-tree 只印一行，再 ff 進 gw 推。
- fConfiguration 是問題（Q3），沒有寫任何程式。
- 06:3x 核對：gw 本機比 origin 多 1 個純文件 commit，nm-claims 是 `329b6f18`，兩棵樹都沒有未 commit 的改動。
  - origin/main 還是 `f7a2f5b5`（筆電 03:50 之後沒推）。
  - 對 main、對 `origin/v906/steven-cbridge-review6` 的 merge-tree 都只印一行。
- 等的東西：
  - 筆電對 `7d7efee8`／`a6003d06` 的關卡；
  - Q1（11 列）；
  - Q3（fConfiguration A／B／C）；
  - St01 review6 `7e60e445`（等 Steven 回 Q59）。
- 日報：St02-M 06:2x 寫進交接檔（handoff `c088e065`）。
- ChangeLog：`D:\docs\ChangeLog\CHANGES_20260930_Steven02.md`，寫到第 11 列＋§10 目前狀態，St02-M 會再抄一次。
- 踩到的坑：uHGem 引用 fSpeed 會把 cSpeed.cpp.obj 拉進 12 支測試，連結失敗。語法檢查看不到，要做完整建置（techniques §8）。

### 09-29 20:3x 狀態（St02-E 寫）
- **gw＝`69a89130`（已推）**。MR !6 在 GitLab 上還開著（筆電在 GitLab 外合了 `ab1947ee`），這一批也掛在 !6；筆電代跑中（含 T1 Hot 驗收）。
  - 內容：合 main `052194b2`、S-12 加熱（SIM）、S72 尾巴 14 行、St01 第二份清單的 cprod:3009。
  - 工作樹 `D:\AI_TempFile\st02-speed` 的分支 `v906/steven-s12` 就是這個 head。
- **網頁擁有權（20260929）**：`web/page/Main.MotionView.html`、`Main.MotorView.html` 是 St02 的頁；筆電新加的 `ht9045_mv_motor.js` 是筆電的，不改。
- **INBOX 117（MotionView 手臂位置）**：提案 (a) 已被 St02-M 同意。認領文件 `D:\AI_TempFile\st02-e2\review\ST02_INBOX117_MV_SCALE_CLAIMS_20260929.md`，筆電 `Motor/mymotor.h:156` 一行唯讀取得函式**已同意**（TO_STEVEN §4 21:5x）。
  - step 0 已答：-995899 是 golden 的停放位置（HT9050 Mot_Table 的 -999999 軟體極限＋偏移），畫到畫面外是對的。
  - 做法（`v906/steven-inbox117` in st02-speed，併 main `b7df49fb`）：mymotor.h:156 取得函式；ChanMvTrays.cpp 送 `motionView.screenScale`（＋`.ver`）；ht9045_mv_trays.js 放進 `window.MV_SCREEN_SCALE`；Main.MotionView.html liveMech 照 golden 兩個教點畫直線、不夾範圍，沒有 tag 就退回舊的 `q()+OFS`。
  - **已推** gw `bb42cdf1`；St02-E2 審查（`ST02_REVIEW_OWN_117_A_20260929.md`）的 2 major／3 minor／4 nit 修在 `a539e0f6`，**已推** gw（仍 MR !6），等筆電跑關卡。
- **工作 (A)**（A1 ASE-CL、A2 OEE、A3、A4）：ST01-E 20260930 00:28 同意 11 行、Jimmy 20:4x 同意 TfMesSystem → 已跟 S-09 一起推（分支 `v906/steven-s09-lifts2`，見下）。worktree `D:\AI_TempFile\st02-jobA` 可以收掉。
- **S-09 筆電的檔（20260930 推 gw，分支 `v906/steven-s09-lifts2` in st02-speed，一組一個 commit）**：
  - cBinSel 18（第六批）；cBinSel 32（6B 24＋第七批 8）；11 支測試 TESTGUARD。
  - stale 掃描 33（A 1、B 17、C 12＋test_tcp_cmd_server:476、D 3）。E2 審查拿掉 6 列（1201／1207／1244／1282／1763／1764），原因見 `ST02_S09_STALE_SCAN_20260929.md` §7。
  - 第一批：10＋Q-INC 8。
  - **沒推、要補認領**：
    - bthermo G27（:4539）＋:4542：include fTemp_Set.h 會跟 bthermo 自己的 static 常數鏡像（:225-231）衝突，要一起把鏡像改成註解；
    - fLotInfo.cpp:5715 btnClearCountClick：TfSortCT 不是 TObject，golden 的 `(fSortCT)` 編不過，要改傳 NULL，偏離 golden。
  - **維持（寫進 st02_status）**：MainTempMode:246（INBOX 115）、fLotInfo:5795／:7233（給 Jimmy）、MyTempPanel:1050、stale 的 7 個停與 6 個拿掉。
  - 工作腳本在 session scratchpad 的 `stale\`、`lifts\`（apply_tree.py 會先核 OLD 行再套）。
- **等別人**：
  - S-10 Tray Edit（`D:\AI_TempFile\st02-p4`，`e7fb9cf1`）：筆電 (4)～(6) 已同意；等 review6（含 St01 的 (1)(2) `aeb3ad60`）進 main，以及 St01 回 (7)／(8)。缺 (8) 連結不起來，(3) 不能早於 (1)(2)。
    - 同意後把 e7fb9cf1 rebase／merge 到目前的 gw，先重查 OLD 行再套。
  - 下一份工作：St02-M 已向 ST01-M 要。
- St02 自己的檔已經沒有能照 golden 解的 `#if 0`。St01 名下的兩份清單做完（批次 1 全部略過；批次 2 只解 cprod:3009）。
- 動檔前看 TO_STEVEN §1 **和 TO_KEVIN §1**（Kevin 0929 加入，接 HT9050 Index 流程）。
- ctest／exe 這台都沒跑；Steven 本人還沒在 session 裡確認「完整測試」。

### 09-29 18:xx 狀態（新 session 從這裡接；St02-E 寫）
- **gw＝`150d154f`（已推，掛在 MR !6；St02-M 決定沿用 !6）**：`1892a8de` 之後又推了三個。
  - `8a1aeadd` Q45 B5：WebLogin_BookCompare 拆出來、stOperatorClick 的 SetUp 臂；1～1369 行固定，St01 的 W906_Reauth 接在 1419 行之後。
  - `8b475dda`：:1372 行尾的反斜線。掃描漏掉的原因見 techniques §1。
  - `150d154f`：S-09 第四批，cMyDB.cpp:664 fContactCT->SaveSiteYield。
  - St01 代跑 tip。
- **St02 自己的檔已經沒有能照 golden 解的 `#if 0`**：還閘著的 15 個列在 handoff `docs/handoff/ST02_S09_BATCH4_20260929.md`。St02-M 已向 St01-M 要工作。
- **S72 尾巴**：14 行認領清單，`docs/handoff/ST02_S72_TAIL_CLAIMS_20260929.md`（fLotInfo.cpp 7 行＋筆電兩支測試 7 行）。**等擁有者回答，不要先放到分支上。**
- **E-016 原生視窗**：St01 開了 MR !7（`c3161cb1`＝我們的 `449c5cd7`＋proto＋main＋debcbdc6，代跑全綠）。
  - 之後的修改開在 `c3161cb1` 上的新分支，**先告訴 St02-M**；native-forms-st02 與 gw 都不要動。
- **B10c**：TesterIF 關窗不用另外接（Q41 (d) 的 TIF_OnPageClosed 已經照 golden 做）；St01 不會在 WindowEdgeTails 加 FTestIF。
- **A8**：cMyDB P4 已經在 main，St01 自己接（RecordProcess、放在 DestroyLogObjects 之前）。

### 09-29 16:1x 狀態（St02-E 寫）
- **MR !6 已合 main**（`53b2b198`；origin/main `a84d25cc` 的 TO_STEVEN §4）。
- **gw 下一張 MR**：做在 `D:\AI_TempFile\st02-speed` 的分支 `v906/steven-s09-b3`，是 origin/main `a84d25cc` 加上以下四個 commit：
  - `9e3aa18f` S-09 第三批；
  - `683e6c53` merge W59-1（`4410f0c3..8fe431e8`）；
  - `a9e9172b` St01 同意的 W59 認領 5 行；
  - 這份現況板。
  - 兩組態完整編譯；對 main、review6 `43811c8f` merge-tree 各一行。
  - St02-M 驗 hash 放行後：`D:\HT9045` 的 gw 從 `f9b63b37` 快轉到這個 head，帶 `-o merge_request.create` 推。
- **原生視窗**（`D:\AI_TempFile\st02-w10`，`v906/steven-native-forms-st02`）：**已推 `449c5cd7`**（teach 小修：golden 建構時隱藏的列標灰、Reset Offset＝golden 的 edtSetToOffset、測試 260 列）。四組態編譯過；對 proto `70af7218` merge-tree 一行。
  - ST01-E3 用 `449c5cd7`＋main 開給 Jimmy 的審查 MR；**那張 MR 開好之前不要再推這個分支**，後續放新分支。
- **S-10 Tray Edit**（`D:\AI_TempFile\st02-p4`，`v906/steven-s10-trayedit`，只在本機，**先停著**）：`865e305c`＋`e7fb9cf1`（St02-E2 兩點：ShowMyMessage 放開 FormLock 後才顯示；操作員直接關窗＝golden Cancel）。
  - 認領 **8 處**：`D:\AI_TempFile\st02-e2\review\ST02_S10_TRAYEDIT_CLAIMS_20260929.md`；3.7／3.8 是 St01 `WebPageTable.cpp:463`／`:630`，同一行。patch：同目錄 `ST02_S10_TRAYEDIT.patch`。
  - St01 15:04／15:39：(1) 頁面表那一列、(2) WINDOWS 那一項 St01 自己做（review6 `aeb3ad60`）；(3) `ht9045_sortct_wire.js:220` 同意。
  - 還在等：筆電 (4)～(6)，以及 (7)／(8)。在那之前連結不起來。
- **S-09 第三批**（`D:\AI_TempFile\st02-speed`，`v906/steven-s09-b3`，從 origin/main `a84d25cc` 開，只在本機）：共 69 列（St02 自己的 TesterComm 檔）。
  - 解 4 個（`9e3aa18f`，已在上面那張 MR 裡）：G25 `HandlerGpibMsg.cpp:1948`、G4／G11／G14 `HandlerBridgeCtl.cpp:127`／`:475`／`:909`。G14 會用 TCP 送 SOT，請筆電上機驗。
  - G1／G5 等 INBOX #68；其餘 63 列理由還在。st02_status：`D:\AI_TempFile\st02-e2\review\ST02_S09_B3_STATUS_20260929.tsv`（St02-M 已貼 handoff `861f1029`）。
- **W59**（`D:\AI_TempFile\st02-elasched`，`8fe431e8`）：St01 已同意那 5 行，已放進上面那張 MR。
- **S72 尾巴**（St02-M 16:0x 交辦，排在 B3 後面）：`forms\fLotInfo.cpp` :5463／:5482／:7143 `slEventLog->SetLotData`。
  - 照 golden 解開；ctest 安全在測試端處理，不在產品碼加 NULL 檢查。
  - 一個只讀 helper 在查哪些 ctest 會走到；fLotInfo.cpp 的認領清單交 St02-M。
- ctest：St02-M 轉述「可以直接進行完整測試」，但 Steven 本人沒在這個 session 確認，**仍然不跑**任何 exe／ctest（這台只編譯）。

### 09-29 11:0x 狀態（新 session 從這裡接）
- **MR !3 已合**：筆電 08:09 照 `7dfcad71` 合進 main（`c7a9a342`）；St01 關卡的 4 個測試失敗由 `a2276dd8`（09:23，只改測試）修掉。凍結結束。
  - 我們自己做的同樣四個修正（分支 `v906/steven-mr3-fixes`）作廢、沒推。ELA_Schedule 加速（約 70 萬 → 9 萬 Tick、300 秒牆鐘上限）留在 st02-w10 的 `v906/steven-mr3-fix-st02-w10` `3f93430f`，St01 那台再逾時才提。
- **下一張 MR（gw）**＝本機排隊 9 個 commit ＋ merge origin/main `14233eb0` ＋ A18 `1c2cc527`（54 行註解認領）＋ A5 (b) `0817e68f`（UploadEventLogFile）＋ 這次現況板。
  - 在 `D:\AI_TempFile\st02-p4` 分支 `v906/steven-queue-merge-0929` 做、兩組態編譯；St02-M 看過 hash 後才快轉 `D:\HT9045` 的 gw，帶 `-o merge_request.create` 推。
  - merge-tree 對 main 一行；對 St01 review6：`277baaea` 一行，新的 `af97ec9f` 跟 **main 本身**在 `FileRW/TestIF_File_SetUp.gen.inc`、`tools/editlist/TestIF_File_SetUp.py` 衝突（不是我們的檔，St01 要自己合 main）。
  - 同一行註解後呼叫掃描：1 筆，main 帶進來的 `FileRW/IniConfig.cpp:117` `IC_EvCreateProxies();`（875d3499，不是我們的檔，已報 St02-M）。
- 還沒做：★W59-1（網頁可編長警報說明，helper 的 golden 研究在 scratchpad `w59\`）、★W58（要筆電認領 cprod.cpp 等，現在 MR !3 合了可以提）、★W57 Q2／Q3。

### 09-29 08:3x 狀態（新 session 從這裡接）
- gw 遠端仍凍結在 `7dfcad71`（MR !3＝A：St01 全部 ctest 綠了之後筆電照 hash 合）；本機 7 個 commit 排隊（`265a11ad`…`57906aa8`、`165db46e` A6），備份 patch 在 `D:\AI_TempFile\st02-e2\review\`。
- **Steven 新規則（memory fewer-questions-follow-bcb）**：golden／既有裁決答得出來的不列題；只有 V906 新功能（例 W58）才一行問。
- Steven 0929 裁決（他的原話優先於 St02-M 的表）：
  - **W61＝B**：SystemStart 全部算「生產」，裡面再細分 test（col 27）／contact test／off-line／home／其他；閒置＝沒測試也沒 SystemStart。helper 在 st02-elaftp `v906/steven-w61-startidle` 做。
  - **W48 第一列＝A**：已做 `165db46e`（開機第一次 tick 清一次 RecordTimeData 會清的那組）。
  - **W57**：Q1 訊息頁存英文，MyMessageBox 中文列成 i18n（已產 `ST02_W57_MYMESSAGEBOX_I18N_20260929.csv`：204 則／289 呼叫點）；Q2 手打內容照存 UTF-8；Q3 可分功能 i18n 檔；Q4～Q6＝A。
  - **W58（模擬版遮網路）全部「要」**：勾選存檔要寫 config.ini；有疑問的也算網路；客戶碼強制開的也關且元件改 enable；7016／7017 模擬版預設關；`\\` 路徑算網路。原型在 st02-q41 `v906/steven-w36-1-proto` WIP `1851e6f9`；cprod.cpp 等要筆電認領（MR !3 合完後）。
  - **W59**：1＝可以改、用多國語言欄位（Error\<語言>\<碼>.dat，要處理 Big5／韓文／RTF）；2＝第二框名稱統一（golden 可能筆誤，我們定）；3＝位置我們定。
  - **W60＝B**（WinINet；KYEC 維持被動）。**W62**：同步下載可以；上機驗證用真的工作檔資料夾（ctest 仍不得碰真檔）。**A8＝C**。**U13**：照 golden 寫開機 TTL log 那一行（開機順序要認領）。
  - **R117**（D46 連按）＝C：我們的 `ht9045_config_q41.js` 本來就是每按一次排隊、等回覆才送下一個（busy 隔 450 ms 重送），不用改。

### 09-28 20:2x 狀態（新 session 從這裡接）
- 遠端 gpib-widget 仍是 `7dfcad71`（MR !3 final），**凍結**：筆電合 MR !3 之前不推（MR 跟著分支最新 commit 走）；推之前先問 St02-M。3 小時點 20:24 已過，St02-M 會請筆電照 hash 合。
- 本機扣著（gw）：`265a11ad` 托盤 JS、`156dcac3` A18、`73b2aef5` A5、本檔。MR !3 合進 main 後：merge 新 main、兩組態、merge-tree、推。
- 原生唯讀 MotorView（Steven 0928 18:0x）：v2 patch 已上交接分支（`25c5f007`，commit `4ab6dc98`；本機 st02-w10 `v906/steven-native-motorview-demo` `17277384`）。St02-E2 審 v2＝OK；可選的 v3 小修：Galil Index 軸 Speed 為 0 時 golden 照寫 0（main.cpp:8395-8397，v2 空白）、說明檔補「閒置時位置快取也停在最後值」。
- ★W42：st02-ela `4bca3c1a` CMake 已不動筆電的行，剩 cMyDB.cpp 6 行同一行要認領，等 W57。
- 等別人：Cleaning :194（St01）、UploadEventLogFile（St01，A5 認領）、A8 超豐（Steven）、U13（Steven）。

### 09-28 17:4x 狀態（新 session 從這裡接）——MR !3 final
- **MR !3 final** 就是這次推的那個 hash（見 ChangeLog §11r）：筆電在 St01 對它跑完兩組態全部 ctest 後合 MR !3。之後照 RULINGS #8 繼續推同一支分支＝下一張 MR。
- final 內容：merge main（d5994b52→f455a765→3fb74540 TESTGUARD→e882ad39）；筆電同意的認領 ★W44-1 引擎四行（`56cfa07d`）、★W48-1 三行（`4f6f4cbf`，三個 atester 檔與 cObserver.cpp 都在 ht9045_sm，連結用 RESCAN）、cStateRecord 7 行＋wb_serve.cpp:4575（`c3a39dcf`，佇列在 LogObjects.cpp 檔尾）、P8 U12（merge `4ef96d57`＋測試 `d22c9434`）、A3 正式修正 wb_serve.cpp:7630（`41976e99`）；St02-E2 審查修正 A1 送出座只在有橋接視窗時送（`e1c5748e`）、A3 每小時補一次（`8f34b568`，新檔 TesterComm/Handler/TimeDataRecordRefresh.cpp 只進 wb_serve）、A10（`cbcc129d`）、A12（`e81f45dc`）、A17（`1218fd36`）、A8 超豐 Test Time 第 37 欄（merge `ad5c64e1`）、A4 每小時 Product_Loader 檔（merge `fcf9afb1`，環境變數 W906_PRODLOADER_ROOT，O19 仍是已知缺口）；St02 測試隔離也要求 TESTGUARD 的 Gerneral.ini 沙盒（`d1d757a1`）。
- 沒放進 final：Cleaning CL-6 :194（等 St01 同意）、★W42（等 W57，CMake 要改成同一行再認領）、★W36-1 WIP、★W46、S118 托盤頁面 JS 修正（St02-E2 草稿 `D:\AI_TempFile\st02-e2\trays\`，final 之後自己一次推）、HandlerGpibMsg.cpp 8 個裸 fShow（等 review6 的 W906FormShowing.h 進 main）。
- 待決：A8 之後超豐還是 0，因為 golden 分析器 ListProductionLog 用第 5 欄篩列（golden 也一樣）；要不要偏離 golden 讓它認超豐欄位，問 St02-M。U13 開機那一行等 Steven。

### 09-28 14:2x 狀態（新 session 從這裡接）
- **14:00 重啟**：我＝St02-E（github-de），St02-M＝github-62。另一個也在當 St02-E 的 session（github-c3）已改名 **St02-E2**、停手（只做唯讀 W42 翻譯表，D:\AI_TempFile\st02-e2）；查過分支、reflog、編譯目錄都沒被動。重啟後 cron 要重建（`53 */5` 五小時＋`7-59/10` 巡檢）。
- gpib-widget 遠端 `0daefb95`（MR !3）：14:18 `b4c11660`＝merge main 1818cfa4（`982dba99`）＋`v906/steven-w48-rework`（★W48 三項裁決；wb_serve 每小時寫 TimeData＋HANDLER LOG＝golden 行為）；14:24 `ba46ad9f` ★W45 認領（WebSecurityJam.cpp 11 行、ht9045_wire_statussecurity.js :130／:216、Status.Security.html :56）＋`0daefb95` Speed 滑桿 form.event "change"＋position（St01 review6 7e1785dc X-2；29a13bdb 不再從 state 改有自己事件的元件）。
- helper：W42（st02-ela `0495e6f5`，第 1～3 步做完，推之前要自己跑兩組態）、W36-1（st02-q41 WIP `1851e6f9`，不推）都在重啟時結束，不用重開。唯讀 helper 在查 St01 B3 頁面 JS（review6 6ba451d5／b08ae6ad：cleaning／contact／offset／barcode *_ev.js）和我們頁面 JS 的重複處理 → 清單先給 St02-M 再改。
- ST01-M 13:4x／13:5x：B4 OK；START 前檢查順序＝St01 的「沒連 HMI 畫面就拒絕」先、再 golden 順序；CL-5／S-07 歸 St01；W44-1 editlist 那一半歸 St01，引擎四行仍是我們的認領（等筆電）；R120-F 與開機 `_NET` 快照改歸 St01（我們只留設計說明）；Temp_Set 共用佇列歸 St01（TS-1），之後 ts7 改用它；★W42 認領原則 OK，等 Steven（W57～W62）。
- ★W42 第 4 步（i18n.js msg 區＋gen_log_english.py）要用的翻譯表：St02-E2 做好的 `D:\AI_TempFile\st02-e2\W42_MSG_TABLE_20260928.csv`＋同目錄 README_W42_MSG_TABLE.md（212 列、299 個 golden 呼叫點；8 列 FLAG：兩組重複鍵 adam6024.cpp:817／:1381、LaserSensorShuttle.cpp:988／:1286 要合併，4 列不確定含 uPAT_Function.cpp:826 的 %f 字面 bug）。**Steven 答 W57（★W42 Q1～Q6）之前不動。**
  - 15:2x 更新：St02-E2 的第 4 步草稿在 `D:\AI_TempFile\st02-e2\draft\`（總覽 W42_STEP4_PREP_20260928.md）；表 v2＝W42_MSG_TABLE_20260928_v2.csv（0 列 FLAG；兩組重複鍵用 `zhAlt`，genFile 要保留 zhAlt 與 src）；gen_log_english.py 產 i18n.js msg 區（210 筆）＋LogEnglishTable.gen.h（171 筆完全相同＋41 樣板，static constexpr，只給一個 .cpp include）。**我要決定**：10 個多行鍵（golden \r\n）LogEnglish 怎麼存；還沒做的：搬進 tools\、ctest、msgText()、編輯器 genFile 認領。E2 只做過 node --check 與 g++ -fsyntax-only。
- 扣著：★W46、cStateRecord（等筆電）、MyDBUpdateDB seam、★W48-1 三行（等筆電）。D1／D2 **已經裝上、在運作**（wb_serve.cpp:4389 → TesterCommWiring.cpp:97）。

### 09-28 12:0x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `35d17d44`（MR !3，12:07 推）：`862b1a03` merge main 8b3a07af（含 Q52）、`7f4e30a1` Q52 TS-9＋TesterIF 註冊（W39×Q52 在 `web\page\ht9045_temp_set_c.js` 的 probe 物件合成一行）、`35d17d44` P8 B1～B5（筆電全部同意）。11:12 推過 `554d78ec`（★W45 Jam Code 編輯器 `6f276498`＋OS-4）。兩組態 0 錯（sim 687 s／ship 675 s），merge-tree 對 main fbfe9033、review6 47c7505c 各一行。
- main 已到 `fbfe9033`（HT9050 machine masters、ARTSEAM csystem.cpp、FLOW 文件）：下一批先 merge。
- **下一批**：merge main fbfe9033＋`v906/steven-w48-rework`（st02-elaftp，`9d7872d3`／`1273077d`／`1e1a8eb5`／`f03cb79c`；★W48 照 Steven 09:3x：3＝B、1＝col 27、2＝B TimeData 每小時 tick）。和 P8 B1 在我們的 `TesterComm\Handler\TesterCommWiring.cpp` include 那幾行衝突（兩邊都留）。W48-2 會讓 wb_serve 每小時寫 `D:\HT9045_Log\TimeData` 與 HANDLER LOG（golden 行為）。
- 等認領（已交 St02-M）：★W44-1 `web\page\ht9045_wire_engine.js` :1072／:1088／:1164／:1244（腳本 scratchpad `w44_1_engine.py`，scratch copy 已 `node --check`）；★W48-1 三行 aTester_Front.cpp:3263、aTester_Rear.cpp:3147、atester_32Site.cpp:393（RecordEndTestTime 接真的；先確認連結）。
- R120-F（FTP 下載工作檔畫面）只有設計，約 3～4 天，等 N-3；**出貨版開 FTP＋ON_LINE 時 START 一定被擋**：開機沒記 `_NET` 快照（golden 906 main.cpp:11054-11066；V906 只有 AutoRetest.cpp:1970-1989 寫一部分）→ 交 St01（MainBoot）＋筆電（WebStart.cpp）。
- helper 在做 2 個：★W42 第 1～3 步（st02-ela，本機到 Steven 答 Q1～Q6）、★W36-1 SimNetMask 原型（不推）。
- 扣著：★W46（Steven 最後決定；筆電偏 B）、cStateRecord（等筆電）、D1／D2、MyDBUpdateDB seam（跟 W42 master）。

### 09-28 08:0x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `c1129d17`（MR !3）。今天已推：W40＝A（`441beb53`）、裁決紀錄（`358832a1`）、Excel .xlsx（`233c5b95`）、上機清單 `docs\ST02_BRINGUP_CHECKLIST.md`（`e2f1f75b`）、W36＝C（`1c361c13`）、W38／W44／W47（`75ca7b37`）、P8 上機計畫（`c1129d17`）。
- **St01 關機中**（開太多 agent）：認領、代跑 ctest、Q52 都暫停。**同時最多 7 個 subagent（含 helper）**，滿了就排隊。
- helper 在做：★W45 Jam Code 編輯器（`v906/steven-w45-jam`，st02-q41）、★W42 訊息英文化（`v906/steven-w42-encode`，st02-ela）、★W48 OEE（`v906/steven-w48-oee`，st02-elaftp）、★W46 FTP 主動／被動（`v906/steven-w46-passive`，st02-elasched）。
  - W42／W48／W46 用 St02-M 核可的建議選項，程式與帳本標「St02 default, pending Steven」。
- 本機扣著：fMain.cpp 15 行（`c59a55b3` 在 q41-wip＋:235；等筆電認領）；★W39 `784d1573`（等 Steven）；Q52 準備 `8295f520`（等 St01 批次進 main）；試合併 `v906/steven-trial-int`（不推，W39×Q52 的解法 `scratchpad\resolve_w39_q52_tempset.py`）。
- 設計說明（scratchpad）：`W42_MESSAGES_DESIGN_NOTE_20260928.md`、`W48_OEE_DESIGN_NOTE_20260928.md`、`P4_STALE_COMMENTS_20260928.md`。
- 規則加的：客戶指定功能照 golden 格式（memory customer-fixed-format）；golden 沒處理的指令＝不支援，只留通訊紀錄；helper 報告的發現要先查證；`node --check` 可以、跑我們建的程式不行。
- 等別人：P8 缺口 B1～B7（多數不是 St02 的檔）；W44-1 指定時間 picker 要不要接；W36 模擬版排程要不要自動跑；第十五題、★W39、★W43、第十九～二十四題。

### 09-28 05:1x 狀態（新 session 從這裡接）
- gpib-widget 遠端：這次推的 skills commit（run 規則＋現況板）。程式碼跟 `8813a741` 一樣。
- **四個等裁決的分支都做好了，只在本機，Steven 答覆前不推**（St02-M 04:0x 派的）：
  - ★W39 TS-8 A：`v906/steven-w39-ts8` `784d1573`（worktree st02-speed；只改 `web\page\ht9045_temp_set_c.js`）。選 B＝同一行 `d1 <= d2` 改 `d1 >= d2`。
  - ★W40 HTSET,354 A：`v906/steven-w40-354` `4e5e0f0b`（st02-w10；Command.cpp 行數不變＋test_tcp_cmd_server §2）。選 B＝整支丟掉。裁決後要改 TESTERCOMM 帳本 :640、tcp-command-server-7016.md §4。
  - ★W42 編碼：`v906/steven-w42-encode` `58994690`／`a61047c1`（st02-ela）。預設 A；B／C＝ElaReports.cpp:161 一行。St02-M 建議 B。
  - Q52 準備：`v906/steven-q52-prep` `585f66c0`..`8295f520`（st02-q41；以 St01 `0bca1318` 為底）。TS-9（Temperature.cpp :40／:245 同一行）＋TesterIF 註冊。給 ST01-E 的 diff 已發（handoff `ST02_Q52_PREP_ST01_FILES.diff`）。St01 的批次進 main 後：merge main、兩組態、merge-tree、推。
  - W39 與 Q52 都改 `ht9045_temp_set_c.js`：合的時候要解衝突。
- **執行規則**（St02-M 定、等 Steven）：`node --check` 可以；用 node／Python 跑我們建出來的程式或測試、跑 exe／ctest 不行；只呼叫 OS API 的工具腳本可以。每個 helper brief 都寫（SKILL.md §7）。
- 扣著等認領：fMain.cpp 15 行；`Setup.Temp_Set.html:348`；atester_32Site.cpp 3 行（P4 清單 §1，diff 在 scratchpad `p4_atester32_claim.diff`）。
- helper 全部閒著。

### 09-28 04:0x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `8813a741`（MR !3）。03:32 `ecfd5ea7`：Speed／TS-7 送 form.event（看 editlist.get 的 "events" 列齊才送，St02-M 同意）；03:58 `8813a741`：P4 之後我們自己檔的 7 則過時註解改成歷史。
- P4 過時註解清單（98 則／70 檔＋2 個程式問題）：`C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\P4_STALE_COMMENTS_20260928.md`，已交 St02-M。最優先：cStateRecord.cpp 背景執行緒呼叫 RecordProcess（Jimmy 的檔）；atester_32Site.cpp 的 MyDBIProcessNew 替身（認領 diff 在 scratchpad `p4_atester32_claim.diff`，sim 編譯過）。
- 扣著等認領：fMain.cpp 15 行（:235 W10＋`c59a55b3` D1～D7／D4）；`web\page\Setup.Temp_Set.html:348`（載入 `ht9045_temp_set_c.js`，St01 的頁）。
- helper 全部閒著（worktree：st02-speed、st02-q41、st02-elasched、st02-ela、st02-p4、st02-w10）。
- 等別人：S118 wb_serve.cpp :439／:2894；TesterIF 註冊＋TS-9：**已在本機 prep 分支做好**（`D:\AI_TempFile\st02-q41` `v906/steven-q52-prep`＝gpib-widget＋merge St01 `0bca1318`；St01 那批（Q52）進 main 後：merge main、兩組態、merge-tree 各一行才推；TS-9 動了 St01 的 Temperature.cpp :40／:245，diff `D:\AI_TempFile\st02-q41\q52_prep_st01_files.diff` 要先給 ST01-E）；Steven 0928 第一～八題、★W36～W46。

### 09-28 03:1x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `5c3431ca` 之後這個 skills commit（MR !3）。今天 02:56 推 `c102083d`（W10 修正＋ELA_Ftp `ec54c042`）；03:1x 推 helper C 的審查修正（`9ff485fe` merge，a～f）＋Q41 D1～D7 本體（`5c3431ca` merge `33a7e519`，沒有呼叫端、不會運作）。
- **W10 教訓**：`aeea58e5` 的四個呼叫都寫在同一行的 `//` 後面，W10 從來沒跑、WebStart 按 Start 可能當掉。已修（`c102083d`、`W906_CmdServersEnsure`）。**推之前跑 techniques.md §5 的掃描 regex**。
- **本機扣著、等筆電認領**（St02-M 已發 `docs/handoff/ST02_FMAIN_CLAIM_20260928.md`，15 行）：fMain.cpp 第 235 行（W10）＋D1～D7／D4 的 14 行（`c59a55b3`，在 v906/steven-q41-wip）。同意後：merge、兩組態、merge-tree 各一行才推。
- St01 要重跑：TesterComm_TcpCmdServer、TesterComm_Handler、ELA_Ftp、ELA_Reports、ELA_Schedule、ELA_Service、ELA_Hub、ELA_Core、WebLogin_ForceOperator、TesterConnect_Rules。
- helper：Speed／TS-7 頁（all-event，等 eventTag）在 `D:\AI_TempFile\st02-speed` v906/steven-speed-wip 做（只改頁面 JS）；A、C、Q41 閒著。
- 接下來：S118 九個主畫面托盤的名稱＋範例資料給 St02-M（已查好：cells＝golden 的顏色索引，≥1000 要減 1000；調色盤是 HTray.cpp 的預設值）；然後 P4 的 35 檔／52 則過時註解清單（清單還要重找：transcript 與 P4 分支都沒有現成的）。
- 已知風險（helper C 回報，已記帳本 ELA_PORT_LEDGER.md:1286-1289，沒改程式）：/api/ela/* 的 POST 沒有 Origin 檢查，同一台機器瀏覽器裡的別的頁面可以盲送 POST 排工作。

### 09-28 01:1x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `08ee9c9c`（MR !3）：D4 的本體（78e8bf51，目前沒有呼叫端）也已推。
- **本機扣著**：`49c31d2c`，在 v906/steven-q41-wip，改 Jimmy 的 fMain.cpp 第 1056／1148／1149 行。筆電同意認領後，把它 merge 進 gpib-widget，兩組態編完、merge-tree 只印一行才推。
- S118 要接上：wb_serve.cpp 第 439／2894 行（等認領）、Main.MotionView.html 第 2970 行（當時記成筆電的頁；**20260929 起 Main.MotionView.html／Main.MotorView.html 是 St02 的頁**，筆電 TO_STEVEN 21:0x「改了你們的兩頁」，St02-M 確認）。
- 三個 helper 都閒著。

### 09-28 00:3x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `e439fe46`（MR !3）：ELA R1～R6、★W45 第一步、S118 托盤資料（目前沒有呼叫端）都已推。
- ChangeLog 換日：`D:\docs\ChangeLog\CHANGES_20260928_Steven02.md`。
- 進行中：D4（Q41 helper，st02-q41）。fMain.cpp 兩處要等筆電認領同意才推。
- 等認領：S118 的 wb_serve.cpp 第 439／2894 行、Main.MotionView.html 第 2970 行（當時記成筆電的頁；20260929 起是 St02 的頁，見上一條）。
- helper A（st02-ela）、C（st02-elasched）閒著；可接的工作要等裁決或協定。

### 09-27 23:1x 狀態（新 session 從這裡接；之前幾段標的時間快了約一小時，以 git commit 時間為準）
- gpib-widget 遠端 `0ddd7de2`（MR !3）：ELA R1～R5、★W45 第一步（JamRules.h）、O10 單一來源都已推。
- R6（eventlog.html 顯示工作紀錄與排程狀態）交給 helper C（st02-elasched）；helper A（st02-ela）與 Q41 helper（st02-q41）閒著。
- 等別人：Speed／TS-7 的頁面協定（ST01-E）、St01 5b73d905 進 main（TesterIF 註冊）、Steven 的 ★W36～W46。

### 22:5x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `57d66925`（MR !3）：ELA R1～R4 都已推。helper A（st02-ela）的工作完成，目前閒著；之後可以接 R6。
- 進行中：
  - C：R5，在 st02-elasched；交件前先合 gpib-widget。
  - Q41：TesterIF 關頁收尾 → CC-E2／CC-E7 頁面那一半，在 st02-q41。
- Q41 helper 的排隊：Speed 頁改送 form.event＋TS-7 頁面那一半（St01 `70aa17e8`），**等 ST01-E 回頁面協定**；同樣只改我們的 JS，沒有 eventTag 就不送事件。
- TesterIF 註冊那一行等 St01 `5b73d905` 進 main；CC-E2／E7 的 C++ 那一半是 St01 `64ade3b7`，還沒進 main。

### 22:1x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `a9d38541`（MR !3）：今天的工作加上 ELA R1＋R2＋R4 都已推上去。
- helper 進行中：
  - A：R3＋背景工作不蓋頁面快照，在 st02-ela；交件前要先合 gpib-widget。
  - C：R5，在 st02-elasched。
  - Q41：TesterIF 關頁收尾，在 st02-q41。註冊那一行等 St01 5b73d905 進 main。
- ★ 編號：W36 模擬版上傳、W37 W14、W38 VTEST、W39 TS-8、W40 W10 S-a、W41 702、W42 報表編碼、W43 O06-4 間隔、W44 N10-3、W45 Jam Code 編輯器（原 W20）、W46 FTP 主動／被動。

### 20:1x 狀態（新 session 從這裡接）
- **gpib-widget 遠端 `c0ea3021`，MR !3**：已推上去的有 ELA W15／18／19、Q41 A＋B 段（含 YM-4）、W10、P4＋W7、cMyDB 註解、main 8b5a91b5。
  本機沒有未推的東西。
- **st02-on-cbridge 已退役**：St01 合回 94f16127（ed365c68）。之後不要再推，所有工作都在 gpib-widget。
  St01 的記錄員 skill（db1b7638）每輪都會把 ST02_DAILY／ST02_CHANGELOG 併進它的日報與 ChangeLog。
- **helper 進行中**：
  - ELA A（R1→R2）：`D:\AI_TempFile\st02-ela`，分支 v906/steven-ela-wip，obj 根 st02-ela-obj；
  - ELA B（R4 WinINet ElaFtp）：`D:\AI_TempFile\st02-elaftp`，分支 v906/steven-elaftp-wip，obj 根 st02-elaftp-obj。
  - ELA C（R5 ElaSchedule）：D:/AI_TempFile/st02-elasched，分支 v906/steven-elasched-wip，obj 根 st02-elasched-obj；用工作表呼叫各工作（R2／R4 合併時再填），不改 ElaService.cpp，合併時由 St02-E 在 W906_ElaStart 接一行。CMake 新檔加在 ElaTables.cpp 那行後面。
  - 兩個都改 ht9045_ela 的 CMake：A 的新檔加在 ElaCore.cpp 那行後面，B 的新檔加在清單最後＋把 :1372 的連結改成 wininet；FTP_Log 由 B 負責。
- **ChangeLog** `D:\docs\ChangeLog\CHANGES_20260927_Steven02.md` 由我維護（每次推送後更新，St02-M 抄進交接檔）；日報區塊隨回報交給 St02-M。
- 接下來：R3 → R5 → R6，然後 W20 Jam Code 編輯器合一；低優先 S118 Tray producer、D4 WebLogin.cpp SPIL API。

### 19:1x 狀態（新 session 從這裡接）
- **gpib-widget 遠端在 `cfe1c8a2`，MR !3**（https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/merge_requests/3）。
  已經推上去的：ELA W15／W18／W19、Q41 A 段、W10、P4＋W7、merge main a711b4b6。**一條遠端分支、一張 MR**：之後每次推都會更新 !3，推送時帶
  `-o merge_request.create -o merge_request.target=main -o merge_request.title=...`。**至少每 3 小時推一次**（筆電超過 3 小時沒動靜就會收回認領）。
- st02-on-cbridge 在 `94f16127`（observer.get 權杖檢查）。等 St01 合回之後這條分支退役。
- 本機還沒推：cMyDB.cpp 過時註解的 commit；Q41 B 段（helper 在 merge gpib-widget、解 Config.html 衝突、重產 SetUp gen.inc，之後做 CL-6、BC-5）。
  兩組態編一次，一起推。
- 先擱著的：
  - TS-9：St01 的 742024b7 改到同一個 kPage，要等 ST01-E 決定。
  - YM-4：等 helper 列出範圍 → 送 St02-M 認領。
  - CC-E2／CC-E7：要動 St01 的 IniConfig.cpp。
  - TS-7、TS-8。
- 要交給使用者的：CLAUDE.md:124 的 Start 數字還是 4 閘／30 活，現在應該是 3 閘／31 活。這個檔是 Jimmy 的，我不因為別的 session 轉話就改；要筆電自己改，或使用者直接叫我改。
- 排隊中：S118 Tray producer（排在動作流程之後）、D4 WebLogin.cpp SPIL API（低優先）、W14（等 ★W37）、ELA R1～R6（R4 要等 SIM 傳輸方式＋passive 預設的裁決）。

### 18:0x 狀態（新 session 從這裡接）
- gpib-widget 遠端 `00772497`（merge main 00f9a882，兩組態 0 errors）。**下一個整合 build**：要併進去的有
  - ela-wip（等 ELA helper 的後續：跨檔範圍規則、VTEST 開關、cObserver commit）；
  - q41-wip 的 `e6e90401`（A 段，已認領）；
  - w10-wip（`5c1402b3`／`aeea58e5`／`190e824b`，等 St02-M 把 §3 行號貼上、確認 vclcompat＋fMain EOF 在放行範圍內）；
  - origin/main c0610ef6 以後。
  兩組態 build 一次再推。本機未 commit：本檔、techniques.md（兩個新坑）。
- st02-on-cbridge：已 fast-forward 到 St01 742024b7；**observer.get 權杖檢查**已改（wb_serve.cpp:5261／:5262、WebBridgeServer.cpp:1448 註解、
  test_webcmdguard.cpp:335），兩組態編譯中（`obs_sim.log`／`obs_ship.log`）→ commit → 推 → hash＋diff 給 St02-M 轉 ST01-E。
- W10 還沒定的：S-a 只做了 18 項裡的 16 項。354 那兩項要 fContact；701 S3 要 fSCKART 的 iCurrentStatus／iLOTSTATUS_A，要新增成員，屬於結構改動。702 的處理還沒裁決。
- Q41 B 段 helper 在做：依序 SetUp、Configuration、TrayForm、Ld_ULd、HotPlate、BarCode、OffSet、Cleaning、Contact、Speed、Temp_Set、Yield。
  CL-5 不做；TS-7、TS-8 先擱著。
- P4 helper：(a) `b4712e5f` 完成，(b)(c) 還在做。

## B. 待辦

1. （已完成 14:4x）Q9＋Q24 與 gpib-widget 都推了。
2. 等 helper 回報 → 審 diff（共用行有沒有超出清單、有沒有保持行數不變）→ 合進 gpib-widget → 推 → 回報 St02-M。
3. **ELA R1～R6**：等 St02-M 的研究 4（W15／W18／W19）。R4 要等 ST01-M 回兩題：SIM build 用哪種傳輸（A：sim 只記 log，建議）、passive 預設。W13 UploadProdLog 是 CopyFile 到 asN17ProductionLogPath，**不是 FTP**：要做 copy-verify（同大小＝成功），不走 IElaFtp。
3b. **Q2 補做（ST01-M 16:25；ST01-E 16:50 同意，條件：在我們的分支、行數不變；tools\wb_serve.cpp 與 WebCmdGuard.cpp 是 St01 的，先送行數給 St02-M 認領、合之前給 ST01-E diff；白名單有改就同步改 tests\test_webcmdguard.cpp 的 observer.get act 案例；St01 目前只在 wb_serve :5953 附近改 S122）**：observer.get 裡 yieldSite／yieldMax／yieldMin／yieldClear 會改記憶體，不能免權杖。WebBridgeServer.cpp:1448 維持只比對名稱；改在 observer.get act 分派處擋：不是持有權杖的操作員就回 not-operator，什麼都不改。先唯讀找出哪一層知道 client id 與控制權持有人，把確切行數送 St02-M 認領（WebCmdGuard.cpp／wb_serve 可能是 St01 或 Jimmy 的）；ctest：沒權杖時讀的 act 可用、四個改的 act 被擋；有權杖時全部可用。9d790ff2 的「These three only read」要更正。排在目前的交件之後。
3c. ST01-E 16:50：Q24 diff OK；7f0c24b1 等他們三個工程 commit 完再合，合完 ST01-M 跑 Security_LoginDatBook＋Security_JamMerge；webprobe 等 Steven。Q41 Speed 5 列放行（只改頁面 JS；不碰 FileRW\ArmSpeed_File*、tools\editlist\ArmSpeed_File.py；JS 區段仍要先認領）。W14 四點已送 Steven（★W37）。
4. **W14**：**先不寫程式**。四題已由 St02-M 轉 ST01-M：DeviceForm_File、不重新備份、Reset 失敗時要還原、JSCC 只改模式。見 `research-w14-o07-contact-raise.md`。

## C. 研究參考（St02-M 做的，已複製到本 skill 的 references）

- `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-wininet-elaftp.md`：ElaFtp 的 WinINet 設計。
  - 驗證用 `FtpCommandA "SIZE"`，加一次帶 RELOAD|NO_CACHE_WRITE 的 LIST。
  - INTERNET_OPEN_TYPE_DIRECT；逾時由 watchdog WbThread 呼叫 InternetCloseHandle 處理。
  - Hub 的 FtpFactory 預設是空傳輸，只有 W906_ElaStart 會裝 WinINet。
  - nmftp 庫留著；R4 時把 ht9045_ela 的連結從 nmftp 換成 wininet（CMakeLists :1372）。
- `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-r2-tcp-command-joining.md`：W10 R2。
  - 每條連線一個緩衝，依各 id 需要的逗號數切出一個 frame；跳過雜訊直到下一個命令開頭；每個 tick 最多 10 個 frame。
  - 上限 2048 bytes，超過就丟並記 #Overflow#；連線不斷。
  - 322／323 只保護陣列寫入，記 #Ignore#，照 golden 回覆。
  - RS232 golden 樹：`D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410`。
- `D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-w14-o07-contact-raise.md`：W14／O07。
  - golden 會寫 Contact.Data；「只改記憶體」要改的是 DeviceForm_File。
  - I49 路徑在 V906 整條是死的。
  - csystem.cpp :2578-2584 的替身要跟記憶體版還原放在同一個 commit。

## D. 今天的裁決（都已寫進計畫文件，commit c01e6821）

- Q9 備份＝A。Q24：ST01-M 放行。
- W10：R1 修；R3 要；S-a 要；R2 不回 NG，照 RS232 BCB 的接命令做法。702 的處理還沒裁決。
- #22：D-a 用 O10 閘住；D-b 開機補跑；D-c 擋 underflow；D-d 產報表時重掃；D-e 報表用 UTF-8；D-f 真連 FTP 跟設定開關走。FTP 傳輸用 WinINet。

## E. 已推的重點 commit（今天）

- st02-on-cbridge：`4fbaf7e9` S127／S128。
- gpib-widget：`d78132eb` W1、`884b3035` W1b、`5884cf6a` ELA 鎖、`834fcc78` 第一型退役、`9d790ff2` Q2＋W11、`8adbcb02` Q32、`08c182c4` Q34 文件、`db66fe93`／`32d898c1` ELA skill、`58dea595` R0、`6c33353e` merge main、`4186b125` skills、`014e9094` W9。
