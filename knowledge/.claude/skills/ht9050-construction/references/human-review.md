# 需要 Steven／人工審核的項目（St01＋St02）

> Steven 20260930 17:1x：「需要人工審核的要通知ST01-M寫到skill的參照裡面喔!」
> 這份是 Steven 看的待審清單，由 ST01-M 維護。St01 的項目由 ST01-M 從工程師回報整理；St02 的項目照抄 St02-M 的清單，原檔是 `D:\HT9045\docs\handoff\ST02_HUMAN_REVIEW_20260930.md`（v906/steven-handoff 分支）。
> 每一項寫：做了什麼、為什麼要人看、要看什麼、程式在哪、commit。「main」＝已在 main；「review6」＝在 St01 分支 `v906/steven-cbridge-review6`；「gw」＝在 St02 分支 `v906/steven-gpib-widget`。
> 路徑：移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`；網頁 `D:\HT9045\web\page\`；golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`。
> **維護規則**：工程師推送時，只要有「照 golden 但要上機看」「看得到的行為改變」「規則例外」，就在回報裡標出來，ST01-M 同一輪加進來。Steven 看過、或上機驗過的，改到「已看過」那一節並寫日期，不要直接刪掉。

---

## A. 上機要看（照 golden 做了，要在機台或瀏覽器上確認真的對）

| # | 誰 | 項目 | commit | 要看什麼 | 程式在哪 |
|---|---|---|---|---|---|
| A1 | St01 | **通知框按「確認」可以關（INBOX 119，Jerry J-5）** | main `ed4716f2` | 馬達 JAM 或 kcode==0 的通知跳出來時，按「確認」會關掉；重新整理後不會再跳；會停機的警報框不受影響。只做過 node 模擬測試，**沒有在瀏覽器實際點過** | `D:\HT9045\web\page\ht9045_dialog_host.js` |
| A2 | St01 | **標籤文字跟著 golden（INBOX 108）** | review6→main `cb9787ad` | 各 C 路設定頁的標籤，開頁、重新整理後顯示的是 golden 執行時的文字；輸入框、下拉選單、帶勾選框的標籤不被改掉。**沒有在瀏覽器看過** | `D:\HT9045\web\page\ht9045_wire_engine.js` gbApply |
| A3 | St01 | **Windows 登出／關機時送停機（D-012 A3W）** | review6→main `42e6607e` | 真的在機台上登出、關機一次：馬達有停；下次開機出現 golden 的「上次異常關閉」提示（照 BCB，登出／關機不寫 Program Close）。D-012 A9 上機檢查清單在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q44_EXIT_TODO_20260928.md` | `FileRW\MainClose.cpp`、`tools\wb_serve.cpp:4536` |
| A4 | St01 | **B8 會動機台的 9 項**（SU-8、TS-11、TS-12、CL-4、CT-3、OS-1b、CC-E11、M-5、BC-4） | 還沒做 | 要在 HT9050 上、有人在旁邊才能做和驗；TS-11／TS-12／BC-4 要 HT9050 可能沒有的硬體。風險與順序在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md` | — |
| A5 | St01 | **E84 交握（B8 AG-1，Q60 ok）** | review6 `b45d4482`＋`a9f89386`＋`eb60d61f` | 只有 Gerneral.ini AGVModal=1 **而且** `D:\HT9045\config\AGV.ini` E84 Enable=1 的機台會動（HT9050 沒有 E84、Steven01 AGVModal=0）。有 E84 的機台上：開 AGV 頁或開機後，E84 交握照 golden 跑；Initial Load／Unload 會關掉那一側 6 個 E84 輸出、交握回第 1 步 | `FileRW\TestIF_File_AGV.*`、`web\page\ht9045_agv_c.js` |
| A6 | St01 | **Q57 的 10 支網頁探針修正（Q58 不重跑）** | main `329296b4` | 只做過離線自測（33 項 0 失敗），**沒有對伺服器跑過**；以後有別的原因跑 Q50 時順便確認 | `tools\webprobe\` |
| A7 | 筆電（原 St01） | **「有開的頁才送」串流**——RULINGS_20260930 #12（Jimmy 20:4x）改成筆電整個接手；St01 寫好的第 1 階段在分支 `v906/st01-stream-s1`（`ca661a18`）給筆電參考 | 筆電 | 每一階段做完要量：瀏覽器分頁的傳輸量、主迴圈整理 tag 的時間；Steven01 要實量時需要 Steven 同意跑 SIM wb_serve（筆電 SIM 也能量第 1 階段） | 評估 `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\stream-by-open-page.md` |
| A8 | St02 | Tray Edit 頁（S-10） | main `3dc2f7c9` | 詳見 St02 清單 A1 | `TrayEditForm.cpp`、`web\page\HW.TrayEdit.html` |
| A9 | St02 | Contact 頁 Edit Tray 入口 | main `08e16cf1` | 詳見 St02 清單 A2（St01 的 CT-3 接上之後按一次） | `WebTrayEdit.cpp` |
| A10 | St02 | 模擬組態加熱執行緒（S-12） | main `69a89130` | SIM 用 Hot 配方跑，入料臂 1→10→50→75、塔燈「加熱中」→「運轉中」 | — |
| A11 | St02 | Motion View 手臂照 golden 兩個教點換算（INBOX 117） | main `a539e0f6` | SIM 看手臂、飛梭停在 golden 的站點；軸讀數是馬達脈波 | `JsonBridge\ChanMvTrays.cpp`、`web\page\Main.MotionView.html` |
| A12 | St02 | GPIB G14：TCP_IP_MODE＋ON_LINE 時送 SOT | main `9e3aa18f` | 上機確認 SOT 真的送出、TCPIP log 有寫 | `TesterComm\` |
| A13 | St02 | T1 重播 Tray Edit 那一段 | — | 筆電跑 T1 時看 Tray Edit 那段 | `tools\flowcmp\script_T1.json` |
| A14 | St01 | **Setup.Contact 重開視窗時照 golden 重算力量數值（D-019）** | review6 `7cbb478d` | 只做過 node 模擬；請在瀏覽器上關掉再打開 Setup.Contact，看力量數值有沒有重算。只有 [D27] 在 2-Site／2x1／2x2NN 時數值才看得出變化 | `D:\HT9045\web\page\ht9045_contact_slk.js` §18 |
| A15 | St02 | Observer 頁 GPIB 版本／TTL RS232 版本兩格（H-022 T3） | MR !14 `a191966a` | 詳見 St02 清單 A7：先顯示 NA，收到第一個 MSG_CMD_Version 回覆後顯示版本；TTL 那格只有裝 TTL 板的機台會變；沒有 ctest 驗頁面值 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp`:8085、`TesterComm\Handler\HandlerGpibMsg.cpp`:882／:906 |

## B. 看得到的行為改變（照 golden，但跟今天機台上看到的不一樣）

| # | 誰 | 項目 | commit | 會變成怎樣 |
|---|---|---|---|---|
| B1 | St01 | **登出／關機後開機會跳「上次異常關閉」**（A3W，照 BCB） | review6→main `42e6607e` | BCB 在 Windows 登出／關機時不跑關閉流程；網頁版照做，只停機、不寫 MES2109 |
| B2 | St01 | **開窗權限多看 Security_new.def（D-014）** | review6 `50be588e` | `D:\HT9045\config\Security_new.def` [Main] Tool／Maintance／Teaching 是 0 的機台，工具選單、設定選單、Teach 的頁會開不了（錯誤碼 `disabled:`）；Steven01 全是 1，今天沒差 |
| B3 | St01 | **運轉中不能開 Speed 頁（D-016）** | review6 `e62f57b5` | golden 運轉中 Speed 鈕是灰的；網頁版以前只在回原點時擋 |
| B4 | St01 | **運轉中可以按的 14 顆鈕（Q61「依照golden」）** | review6 `46425cbc`＋`229aceb9` | Offset Setup Teach 排序鈕 12 顆、AGV Initial 2 顆，運轉中也能按（其他照舊擋）；權限照 golden。**比 golden 嚴一點**：開著 AGV 頁時降了權限，要重開頁才能再按 |
| B5 | St01 | **重新登入（B5，Q45 甲）** | main `699dc06d`／`084af96e` | SetUp 關 RTC／OCR、Configuration M01 會照 golden DoPassword 要求重新輸入密碼 |
| B6 | St01 | **主畫面 FT／RT 兩格照 golden 切換（B8 M-11）** | review6 `a9f41d24` | 機台裡有 IC 時不能切換 |
| B7 | St01 | **主畫面 ONE CYCLE／TRAY FEED／ALARM RESET 會真的動（筆電 FLOW-4）** | main | 提示文字已改成「C++ 已收到，下一拍照 golden 執行」；RESET 還沒接 |
| B14 | St01 | **A01 閒置自動登出（D-015）：Steven01 這台出貨版登入 120 秒後一定登出** | review6 `0812b7da`＋`3012ffb3`（gate 綠 `676d3f67`） | Steven01 是 791＝CC_SJ_Semiconductor，config.ini bAutoSwitchToOperatorMode=1、iChangeOpTime=120；golden 的 SJ 分支不管有沒有開頁都不重設計數（「時間到都要登出」，A01_2），所以**出貨版**每次登入 120 秒後就切回 Operator，Offset 開著也一樣（會照 golden 關掉 Offset 頁）；模擬版 golden 把計數固定 0，不會登出。bAutoOpenConfigA01 對 791 是 false，所以不會寫 config.ini |
| B15 | 筆電（原 St01） | **IO、Teach 頁視窗隱藏時停止輪詢（串流 2E）**——**20260930 21:5x 起整個串流改由筆電做（RULINGS_20260930 #12）；St01 這邊的 `6ea7f1eb` 在 review6 撤回，由筆電審過後自己放進 main** | 筆電 | 照 golden（iosetview.cpp:162、uteach.cpp:1366 在 fShow==false 時就返回）：網頁上把 HW.IoSetView／HW.teach 視窗關掉或隱藏後，不再每 200 ms／1 s 去要資料；Teach 的 HOME 彈窗也只在視窗顯示時跑；**關掉 Teach 視窗時 HOME 鈕會立刻彈起**（golden FormClose → btnStopClick → AllBtnUp，不送指令，縮小不會彈）。已知的小缺口：關窗後才到的 HOME 回覆會把鈕再按下，重開時才彈起。單獨開的頁面照舊輪詢 |
| B17 | St01 | **出貨版 A01 在警報框／是否框／ShowMyMessage 開著時也會登出（D-015 A01b）** | review6 `ea17dd3a` | 照 golden：golden 的 VCL Timer3 在這些阻塞框開著時照樣計時，所以時間到就登出；Steven01（791 SJ，120 秒）出貨版會碰到 |
| B18 | St01 | **工具／設定下拉選單開著時 A01 不計時；權限一降到 0 兩個選單都收起（D-015 A01b）** | review6 `ea17dd3a` | 照 golden ChangeLevelAttr（V912 main.cpp:12928-12935／:13182-13190）：不只 A01，按 Logout 或 D4 降到 0 也會收起選單，狀態列顯示一行；網頁用事件回報選單狀態，不加輪詢、不改 main.html |
| B19 | St01 | **Data.LotInfo：tag 把目前分頁藏掉時，不再重讀 Selection／Lot End（D-020）** | review6 `7cbb478d` | 照 golden：程式改分頁不會觸發 pgLotinfoChange；要等使用者自己點分頁才重讀 |
| B8 | St02 | cBinSel 50 個寫檔閘解開（S-09） | main `6818ef9e`／`fc564e32` | 讀配方檔時照 golden 把 7 個 `Binasgn*.Data` 的 Bin Func 鍵寫回（St02 清單 B1，含 18:52 補的「切 Start Mode／開機前後比對 7 個 Binasgn*.Data」） |
| B9 | St02 | SECS 真的送 S6F11 | main `4232999a` | St02 清單 B2 |
| B10 | St02 | GPIB `INDEXCYCLETIME?`／`SET_ALL?` 回真的 index 週期時間 | main `232cb4e3` | 以前回 "0" |
| B11 | St02 | 批號開始／結束寫 ByLotID（S72） | main `fc823f24` | 建 `D:\HT9045_Log\EventLogTxt\ByLotID\YYYY\MM\DD` |
| B12 | St02 | 入料臂 4 個閘＋aTester 2 個 | gw `236adb8e` | St02 清單 B5 |
| B13 | St02 | Sort Arm 放料寫生產紀錄 CSV（還沒推） | 本機 `5e887d2d` | St02 清單 E1 |
| B16 | St02 | 模擬組態的 7016／7017 預設不開真的埠（W58-4） | MR !12 `05525b69`（已進 main `3d5cd2e2`） | 只影響模擬組態：沒設 `W906_SIM_TCP_SERVERS=1` 時 TCP 指令伺服器不監聽真的埠（主控台印 `[W58-4]`）；要在模擬測 MES／AMR／tester 的人先設這個變數。模擬的 TCPIP_Log 照 golden 格式仍寫「[7016] Server Listen」但其實沒在聽。出貨組態不變（St02 清單 B6） |
| B20 | St02 | fConfiguration 8 列照 golden 寫／讀 Configuration 頁（S-09 Q3） | MR !15 `e931eed3` | 詳見 St02 清單 B7：1341 勾 I17 時 Handler 不連 HVision；1268 SPIL 主機 Enable RCMD START 會強制勾 cbN07；SetTesterID／HTSET 314／315／316／804 之後頁面值會變，上機各送一次看頁面 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`:554-579 |

## C. 規則例外（已跟規則擁有者講好，讓 Steven 知道）

| # | 誰 | 項目 | commit | 內容 |
|---|---|---|---|---|
| C1 | St02 | FShow_Audit 例外：`TrayEditForm.cpp` 13 個直接讀 fShow | main `a6003d06`／`08e16cf1` | Tray Edit 自己的 modal 狀態；St01（稽核規則擁有者）三個條件都符合後同意 |
| C2 | St02 | 權限表存檔照 golden 設定選單的開頁閘重查 | MR !11 `d6871b8f`（已進 main `3d5cd2e2`） | 運轉中、Maintance=0、等級不到 LevelSet[1] 都拒存；**刻意偏離**：golden modal 開著時按 START 關表單照樣會存，網頁運轉中一律拒存。St01 代跑兩組態 gate 綠（20:50）；網頁探針三種拒絕要 Steven 同意才跑 |
| C3 | St01 | 探針回收桶項目守門自己清（Q50 guard） | main `b1aed08d` | 守門程式會刪掉探針自己建的 W906PRB*／W906IMP* 測試配方，其他回收桶項目不動 |

## D. 還在等人的（列著知道，不用 Steven 做）

- St01 D-015 A01 閒置自動登出：C++ 已做（`0812b7da`，gate 排隊）；網頁端「藏工具／設定選單（palSetup／palConfig）」在筆電的 main.html，已問 Jimmy。
- St02 Q3 `fConfiguration` 改讀 FileRW 代理：等 ST01-E 審 St02 的設計。
- St02 `Command.cpp` 遠端 FT／RT：St01 的 `W906_Main_DoFTRTClick` 已有，排程歸 St02／筆電。
- E-001 Rotate：等 Jimmy 回 HT9050 要不要 `USE_ROTATE_KIT`（St02 清單 E2）。
- IO、Message 兩頁 C++ 沒有開頁閘（D-017 (1)）：筆電／EastSun 決定。

## 已看過

（還沒有）
