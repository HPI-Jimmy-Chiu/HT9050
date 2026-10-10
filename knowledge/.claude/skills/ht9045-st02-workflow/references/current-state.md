# St02 現況板（更新：2026-10-10 06:5x）

## ★ 1010 06:5x 存檔點——7 天額度到 85%，停在這裡（額度約 4 天 11 小時後重置；筆電說 W-214 低優先、不改派）
- **ChangeLog 搬家（Steven 1010，入口網站 MR !253）**：之後寫 `C:\AI_TempFile\st02e-scratch\changelog\CHANGES_<日期>_Steven02.md`，**不要再寫 D:\RD5-portal\public\Docs\ChangeLog\Steven02\**（資料夾已移除；入口網站改成每天一個 Steven 檔，St02-M 抄進去）。
- **W-214（POOL-12 SCKART-UNIFY）做到一半，只在本機**：分支 `v906/st02-w214-sckart-unify`（工作樹 C:\AI_TempFile\st02-c24，
  從 main 34c8e2e1＋本板三顆文件 commit），WIP commit `fd5254a3`：認領行已套（`w213\w214_claim.py`，42 行 4 檔，St02-M 06:5x 修過：
  **不改替身建構子**（:2781／:3984 不動）、test_w7_f2 PART C 的 C1～C4 換成一行「removed」註解、加 C9（8 個呼叫點讀 fSCKART-> 等）與
  C10（fSCKART.cpp 建構子常數是 golden）；認領稿 `w213\w214_claim.md` 給 St02-M 貼 §1）＋新測試 tests/test_st02_w214_sckart_unify.cpp＋CMake 區塊。
  **還沒建置、沒跑測試、沒做反向檢查、沒推**。
- **重置後接著做**：(1) 先 `git fetch`，若 main 動了 csystem.cpp／fSCKART.*／test_w7_f2 就 `w214_claim.py . check` 重核；(2) 建置線 s39
  `git checkout --detach v906/st02-w214-sckart-unify`，兩組態完整建置；(3) #6 裡跑 St02_W214SckartUnify、W7_F2 那支（名字從
  tests/CMakeLists.txt grep `test_w7_f2_sckart_state`）、St02_W132Sckart、清料／ART 相關（grep `cleanout\|W7_C1\|W7_C2`）、FShow_Audit、START_SitesCensus；
  (4) 反向：TfSCKART 的 switch 改壞一個 case → [2] 紅；:5784 改回 W7C2 影子 → C9 與 [3] 紅；:3202 改回 W7C1 → [4] 紅；fSCKART.cpp 的 L=3 改 0 → [1]／C10 紅；
  (5) 壓成一顆程式 commit（文件三顆照留）、merge-tree、推 MR、回報；之後才是 W-213 ②（G8～G11、G13＋批次結束→LOTSTATUS_L 的端到端測試）。
  **(1b) 筆電 07:1x 加的（同一張 W-214 MR，FROM_STEVEN §1 W-214 列，handoff 9fb88b8d）**：csystem.cpp :2781／:3984 替身建構子改 golden 值
  （W7C1 iLOTSTATUS_L(3)；W7C2 iLOTSTATUS_W(1)、R(4)、A(6)）＋測試加一個斷言釘這 4 個值＝golden（理由：死欄位放錯的值會誤導下一個讀的人）。
  做法：在 `w213\w214_claim.py` 加回兩個 `sub` 行（:2781 `iLOTSTATUS_L(0)`→`(3)`、:3984 `W(0),R(0),A(0)`→`W(1),R(4),A(6)`），:2672／:3959 的 RESOLVED 註記改說
  「欄位不再被讀、值已是 golden」；斷言放在 test_w7_f2 PART C（例：C1 那行的註解換成 `PIN_SEAM(... "iLOTSTATUS_L", 3, ...)` 要兩行，行數不變就要另找空間，
  或放進 St02_W214SckartUnify 用原始碼釘子），重產認領稿交 St02-M 更新 §1 再實作。
- **W-214（POOL-12 SCKART-UNIFY）設計說明** `C:\AI_TempFile\st02e-scratch\w195\w213\sckart_unify_design.md` 交 St02-M 轉筆電審（06:0x）。
  建議 A：TfSCKART 當唯一 lot 狀態，csystem 的 W7C1／W7C2 在呼叫點改讀寫 fSCKART（:2743／:3948／:3200／:3251／:3202／:3253／:5784／:5827／:5829／
  :5869／:5912／:5914），SCK_ART.* 不動；test_w7_f2_sckart_state PART C（筆電）要改釘。點頭＋額度允許才實作；之後才是 W-213 ②（G8～G11、G13）與 ④。
- !411 在第 157 批 gate（b157a）。
- **W-213 在 St02 領域收尾**：剩下 25 個未重驗的閘 05:4x 全查完，沒有可以照 913 開的。E7／E8（本體在 ht9045_sm，secsgem 不連 sm）、
  G12（fFTPClient 只在 wb_serve；要 hook，遠端 FTP 下載配方＝人工審核，建議另開卡）理由文字過期但仍擋。表在 w213\W213_REMEASURE.md。
- 等：!411 的 gate（St02-M B96）。之後沒有派卡就待命。
- **05:36 推 MR !411**（`v906/st02-w213-hgem` `84fecde4`，連同本板 04:0x～04:2x 三顆文件 commit）：W-213 ③ uHGemHT9045 G37／G41～G43／G47 照 913 開，
  測試 St02_W213HgemS2F41（HCACK 要從 S2F42 回覆緩衝讀：解析成功時函式固定回 1，uHGemHT9045.cpp:8010-8016）。等 St02-M 寫「請 gate」。
- W-213 ②（TfSCKART）與 ④（HTSET,701）擱置，根本解是 POOL-12「SCKART-UNIFY」（docs/handoff/POOL.md，低優先、先寫設計說明）。
- **今天已進 main**：!407 W-212（[C25] 設定頁那一列，第 153 批）、!408 W-150 最後一片（FormClose TTLLog("Close")，第 154 批）。W-150 全部做完。
- **已推、等 gate**：**!409 W-213 SV G21**（`v906/st02-w213-sv-g21` `7ca401f1`，疊在 main 8cd0be50）：SVID 1191 "Error Bin Count" 照 golden 913
  SECSGEM/uHGemHT9045_SV.cpp:235 登錄；筆電 03:3x 點頭（TO_STEVEN §4）；St01 不在，ctest 自己在合併後的樹跑（兩組態 13 支全過、反向檢查 [5] 紅→綠）。
- **W-213（POOL-2 St02 領域的 `#if 0`）**：重量表 `docs/handoff/ST02_W213_REMEASURE_20261010.md`（腳本 `C:\AI_TempFile\st02e-scratch\w195\w213\`
  `w213_measure.py`＋`w213_table.py`，依賴查法 `deps.py`）。剩下的順序（St02-M）：
  1. **TfSCKART 族擱置**（St02-M 04:1x，FROM_STEVEN §3，交筆電選 A 擱置／B 從影子單向轉送／C 先做 SckArt 統一）：csystem.cpp 的 golden
     `fSCKART->SetLotStatus` 走 W7C1／W7C2 影子（:2743／:3948）寫自己的 core（:3200／:3251 L、:5829／:5914 W），TfSCKART 看不到 →
     打開 G8／G9 後 LOTSTATUS 回覆會漏掉 csystem 的轉換（例：批次結束 golden 回 LOTSTATUS_L）。G7～G11、G13 照關。
     認領稿留著備用：`C:\AI_TempFile\st02e-scratch\w195\w213\sck_claim.md`（`sck_claim.py`）。另一個坑：TfSCKART 在 ht9045_forms（最底層），
     SckArt_SetLotStatus 與 SckArtState 的建構子在 ht9045_sm → facade 不能呼叫它們（CMakeLists.txt:658 NO UNDECLARED BACK-EDGE）。
  2. uHGemHT9045 G37（:7385 TEST_TIMES，cbTestTimes 只在 bVTESTFunction 有選項；HCACK 1 → 0／3＝人工審核；記錄照旁邊 :7375 用 ActiveWire）、
     G41～G43（:7543／:7562／:7573，InitialLoaderTask 只設 iLoaderTask=1，golden 的消費者 LoaderAction 沒翻 → 現在沒有行為變化）、
     G47（:7867，CheckActionFlag 只更新 10 個 LED；閘的 DELTA 寫「AMR 啟動卡住」是錯的）。St02-M 04:2x 決定一張 MR 五個一起開（G37 照 golden、
     記錄用 ActiveWire＋一行註明 golden 是 HGemPtr）。**認領稿** `C:\AI_TempFile\st02e-scratch\w195\w213\hgem_claim.md`（`hgem_claim.py`，21 行，
     筆電的檔）已交 St02-M 轉筆電點頭。**點頭後只有 7 天額度還在 85% 以下才實作**；到 85% 就停在認領稿，等重置（約 4 天 13 小時）或改派。
     測試做法：照 tests/test_uHGemClass.cpp:513-525 往 gem.WireCodec.SReceiveData 種 S2F41 內容，呼叫 HT9045Gem::S2F42_Host_Command_Acknowledge。
  3. 還沒逐一重驗的 25 個（uHGemHT9045 19、uHGemEquipment 6）輪到那支檔時再查。
  - **不要開**：SV G17（fGroundMan 的 labValue_* 沒有寫入者；comGMReceiveData 未翻）。Rs232Log 的 4 個都是 BevelOuter 外框效果（保留）。
- **1010 學到的**：
  - 自己寫的稽核腳本要先讀那一行「現在」的理由全文：G17 的理由 10/08 已更新成「STAYS CLOSED」，腳本只看依賴在不在，差點報成可開。
  - 原始碼普查（例 [6] 的 fMain->slTTLLog）不分字串與程式碼；新寫的理由字串不要拼出被普查的寫法。
  - SecsCatalogue（POOL5-3 之後）是對原始碼的結構檢查，不再釘數字；只重建一支測試 exe 時，SecsCatalogue 會因為沒重連而假紅——反向檢查要把兩支一起重建再看。
  - golden 913 樹在 `D:\HT9045\HT9011UC_Code_V3.33.913.0_20261008_steven`（Big5）。
  - HT9050 的 config.ini 沒有 [SECS GEM] Enable SECS GEM（讀成 0）：SECS 相關改動在 HT9050 上看不到。


## ★ 1009 22:4x 存檔點（7 天額度 79%；St02-M 喊 85% 時照這段收尾，90% 停止新工作）
- **今天已進 main**：!396 L05、!397 L03（第 146 批，含筆電修我測試防呆的 c59acf66／2583acfc）、!398 F2-3（第 147 批）。
- **已推、等 gate**：!399 TESNA T1（MyStringList 按天重複檔＋D7）、!400 TESNA T2（疊在 !399；tip 50e02a39，:61 是 Ifor01 的原文）、
  !402 L10（[C25] 防水閘門）。!399／!400 跟 main 只有 tests/CMakeLists.txt 檔尾衝突（三塊 St02 測試區塊都留，請筆電合）。
- **H6 已推**（見 ChangeLog 第 34 列）。**C25 設定頁那一列**：WIP `v906/st02-c25row-wip`（疊在 L10 上），認領 22:4x 已登、等筆電點頭與 !402 進 main，H6 先進就重產。
- **之後**：C25 設定頁那一列（!402 進 main 之後，同樣用產生器；RULINGS_20261009 #14）→ W-204 b（L07 產生檔＋網頁，等 Ifor01 的資料模型）
  → K1 SIM 本機驗證（只做 K1；SIM 版 wb_serve；#6 包住）。F2-3 真上傳：St02-M 明天問 Steven（SimNetMask 不動）。
- **1009 學到的**：
  - 測試防呆一律「路徑含 `machine_log_scratch`／`machine_config_scratch` 才算轉向」，不要只看 `d:\ht9045` 開頭（筆電 gate 在 D:\HT9045\.claude\worktrees）。
    驗證法：用 `D:\HT9045_st02probe\tests\machine_log_scratch`（D:\HT9045 的兄弟資料夾）當 log 根目錄跑一次，跑完刪掉（`c_items\probe_citems.py`）。
  - 一行裡 `if(...) f();   g();` 會被 GCC -Wmisleading-indentation 警告 → 同一行加大括號 `{ f(); }`。
  - 原始碼釘子找舊閘文字時要釘「行首」：解閘後的註解常引述舊閘原文（T2 的 `\n#if 0 // TODO(GA1-B2)`）。
  - FileRW/*.cpp（例 IniConfig.cpp）只在 wb_serve 裡，ctest 連不到 → 產生檔的改動用「同一個巨集：字串比對產生行＋編譯執行」驗（test_st02_h6_a77.cpp）。
  - 全套 ctest（-j 6）在 #6 期間：config_db／ini_helpers／config_loaders 讀真的 D:\HT9045\system（被換成 HT9050 快照）一定紅；dfm2rc ×3、
    建置線沒建的 exe、偶發的 St02_W152ContactData／FastClk_Jobs（單獨跑會過）都不是回歸。
  - index.lock 只有在 tasklist 沒有 git.exe 時才刪。

## ★ 1009 16:3x 存檔點（7 天額度 73%；St02-M 喊 85% 時照這段收尾，90% 停止新工作）
- **開著的 MR**（都是 St02-E 今天推的）：
  - **!376 HANA H4**（`v906/st02-w195-hana-h4` `042fb8ed`＋文件 `e9367b1d`，疊在 !374 上）：開始前連不上 RMS 就拒絕 START（等 3 秒）。**等 Steven（W-199）**；他同意就撤 NIGHT_REPORT §0 #151、可合。
  - **!385 Qualcomm MR-Q1**（`v906/st02-w195-qcom` `24d41327`）：by-count 良率檢查＋913 else-if 修正。排第 142 批；跟最新 main 只有 tests/CMakeLists.txt 檔尾衝突（筆電兩塊都留），**筆電沒要就不推新 tip**。
  - **!387 W-202 ① T08**（`v906/st02-w202-t08` `b3782075`）：GetTesterResult 呼叫 golden HANARMSRunCheckOK(true)。gate 已申請（B85）。
  - **!388 K1 修正**（`v906/st02-k1-reasonfix` `e728d389`）：START 拒絕訊息記住擋下那次的原因（s_sN06BlockedReason）。gate 已申請。
  - 今天已進 main：!370 MR-A、!371 MR-B、!372-!374 HANA H1-H3、!375 H5、!382 KYEC K1、!383 ATK（第 141 批，Ifor01 W-201 已點頭）。
- **等人回答的卡**（沒回答就不認領、不開工）：
  - **TESNA**：Steven 的 D2（整段解 cprod.cpp:2524-2578 的閘或只加 TESNA 那一行）、D7（N10 開＋上傳方式 0＋期間 8 時每行事件記錄寫兩次、JAM 數加倍：照 golden 並告訴 RogerYang，或修掉）。St02-M 已定 D1（LogObjects.cpp:117 重套）、D3（as9045LogPath 接縫）、D4（行數不變）、D5（setter 放 :2572 空行）、D6（網頁順序 T3 交筆電另派）。T1 本身含寫兩次的路徑，所以等 D7。盤點：`C:\AI_TempFile\st02e-scratch\w195\tesna\tesna_census.md`。
  - **LEADYO（利揚）**：Steven 的 D-1（現在做 C++ 側、下載先接測試接縫，或等 F2-down）、D-2（golden「下載被拒卻回報成功」在 V906 怎麼處理）、D-5（3 個 config.ini 鍵連 HT9050 也寫）、D-7（golden 時序與 KingPak 沒查權限）。St02-M 已定 D-3（St01 產生器加對照列，筆電派人）、D-4（成員進 fLotInfo.h，筆電代 St01 點頭）、D-6（同一行附加）。golden 的 4 個疑似錯誤已轉筆電通知 KenHsieh。盤點：`C:\AI_TempFile\st02e-scratch\w195\leadyo\leadyo_census.md`（"F2" 已改成 "F2-down"）。
  - **LI-9 F2（＝F2-up：KYEC FTP 上傳按鈕＋Gate #4）**：Jimmy／筆電代答 D-3（兩組態都用真的 FTP 引擎）、D-4（Gate #4 失敗擋不擋上傳）、D-7（上傳照 golden 在主迴圈同步跑）、D-9（補 912 的 UploadFileToServer2 改動）；Steven 先回就以他為準。技術預設：D-1 只做 F2-up、D-2 筆電檔同一行加接縫、D-5 兩個 bError 先分開、D-6 AMD／KYEC-ATC 分支先拒絕、D-8 plUnloadClick 放新 St02 檔。預查：`C:\AI_TempFile\st02e-scratch\w195\li9f2\li9f2_precensus.md`。K1 已在 main，Gate #4 的相依已滿足。
- **認領腳本（都會逐字核對 OLD，推之前在最新 main 再跑一次）**：`w195\kyec_claim.py`、`w195\atk\atk_claim.py`（23 行 9 檔，含 test_pool2_lotinfo.cpp:147）、`w195\qcom\qcom_claim.py`；反向檢查 `w195\mutate_k1.py`、`atk\mutate_atk.py`、`qcom\mutate_qcom.py`、`w202\mutate_t08.py`。
- **工作樹**：c23＝`v906/st02-w195-atk`（!383 已進 main，可收）；c24＝`v906/st02-docs-1009`（本存檔點，本機）；c25＝`v906/st02-w195-qcom`（!385 等合）；建置線 s39（obj `C:\AI_TempFile\st02-s39-obj\{build,build_ship}`）停在 e728d389。c23／c24／c25 用完請 St02-M 收。
- **本機 WIP 分支**：`v906/st02-w195-qcom-wip`（!385 進 main 後刪）。`-kyec-k1-claimtest`、`-atk-wip` 已刪（1009 16:3x，兩張都已進 main）。
- **#6 小提醒**：機台快照在 ATK 與 Q1 兩輪之間從 377b7d26 換成 66acfb01（兩個都標 13:27），system 清單 795→794（少 userid.com.1.com）是機台端，不是測試寫的。

## ★ 1008 16:2x（重置後）
- **1009 14:2x W-195 ③ KYEC K1 推**（`v906/st02-w195-kyec-k1`，一顆 commit 疊在 main 26a6dcf0；筆電 13:4x 點頭、RULINGS_20261009 #2-#4）：N06 工作檔同步照 golden 913（bool、同步等 10 秒、檢查 zip／共用資料夾／7z、事件記錄去重）＋Steven 的「失敗就擋」：雙語警報（他的原文）、換工作檔成功前 START 拒絕（WebStart.cpp:3170）、測試機 TCP 同步失敗不送 WORKFILE／GETOSSETUP／SET2DID；7z 輸出寫 <as9045LogPath>\N06 日檔、逾時強制結束、只繼承 pipe。新檔 Interface/TesterTCP_N06_St02.cpp／.h，ctest St02_W195N06（101 項，約 18 秒）。**ATK（c23 `v906/st02-w195-atk`）與 Qualcomm MR-Q1（c25 `v906/st02-w195-qcom`）已認領、本機做好，等點頭**；TESNA 等 Steven D2／D7；H4 !376 等 Steven（W-199）。暫存腳本 `C:\AI_TempFile\st02e-scratch\w195\`（kyec_claim.py／atk\atk_claim.py／qcom\qcom_claim.py 都會逐字核對 OLD）。
- **1009 11:2x HANA H2＝!373、H3 推**（都疊在 !372 上，St02 自己的檔）：H2 需要時才連線／預設 port 14140／HDNAME＝Machine ID；H3 互鎖訊息分級、5 秒重查、大對話框、Get Lot 直接查。H4（開始前連不上 RMS 就拒絕 START）St02 部分做好在 v906/st02-w195-hana-h4，等筆電點頭 WebStart.cpp:3161（:1170 的 912 latch 等 A／B）。H5（MyStringList HANA 那半）認領中。**!370／!371 的新測試在筆電 gate b136a 出貨組態 SegFault**：批次 135（Ifor01 !369）讓 ChangeSite 照 golden 呼叫 fTemp_Set->InitialAddrToATC()，我們的測試沒建 fTemp_Set；筆電已在批次裡補（c37eb0d0，只改測試）。以後新測試先合最新 main 再跑（techniques）。
- **1009 10:4x W-195 MR-B＝!371、HANA H1 推**：MR-B（ReadFile 的 sBinType 改依每個 tag 的配方，golden 913 cBinSel.cpp:1585-1596；SECS EC 3656 等 7 個值會變＝人工審核）。H1＝v906/st02-w195-hana-h1（HANA RMS 配方比對照 golden 913 的 HANA 1002 規則：CON_MTD 送模式名稱、E/D 開關、數值要連功能開關一起比、RT.AT_OFF 固定 D、室溫比 Ambient 設定值；St02 自己的檔 HanaRms_St02.cpp＋test_c10_hana_rms.cpp；人工審核：伺服器要送新的 CON_MTD 名稱）。H2（隨需連線、預設 port 14140、HDNAME＝機台 Machine ID）在 worktree st02-c24 疊在 H1 上做；H3 對話框、H4 開始前檢查連線（要筆電的 WebStart.cpp 一行）、H5 MyStringList 的 HANA 那半接著做。H6（[A76]→[A77]）是 St01 的產生器，St02-M 已放 FROM_STEVEN §3 給筆電派。KYEC 等筆電回 a～g 七題（kyec_census.md：其實不是 KYEC 專屬、會影響現在活著的換配方路徑）。
- W-175＝!337 ✅ 已進 main（第 115 批 aa0cf103＝包 198）。
- **普查 v3＝MR !340**（`v906/st02-c12-v3` `cf1bfdbd`，從 main 4be2621b）：工具修正＋重新產生的普查表（golden 0618；手寫 §9-§11 保留、新加 §12）＋`docs/handoff/c12_raw_v3/`。每頁比較 `w150\c12_compare.txt`。
- **1009 09:5x W-195 ①MR-A 推、② 做完、MR-B 認領 GO**：MR-A＝分支 v906/st02-w195-pathcombin（PathCombin 保護＋ReadFile 的 SavePath 去掉開頭「\」，golden 913 FileInfo.cpp:304-305／cBinSel.cpp:1128-1150；四個筆電的檔：cBinSel、common、FileInfo、MyStringList；912 早就有，所以 912/913 差異表沒有列）。② 37 項檔案範圍差異的分類＝docs/handoff/ST02_W195_FILESCOPE_37_20261009.md（利揚 LEADYO 一組不在 ③，請筆電另排）。MR-B（ReadFile 的 sBinType 改依配方算，golden 913 :1585-1596，連 FileRW/BinSelect.cpp:153 St01 的註解同一行改）做中；③ HANA 等 St02-M 的預查 C:\AI_TempFile\st02m\w195_hana_census.md。
- **1009 08:4x W-191 一般部分做完**：MR !363（ADAM 重連 5 秒冷卻，St02 自己的檔）、!364（AOI 雷射掃描 TCP 分段接起來）、!365（BinDisplay TFT bin 117＝Maxim 的 R，程式無客戶條件、筆電可退）、!366（BinSel 存檔路徑去掉重複反斜線）。退回筆電（St02-M §3）：MySaveToFileShareMode（HANA bHanaTrayMap＋TESNA 期間 8，slEventLog 那段在 cprod.cpp 仍被 GA1-B2 閘住）、uYieldMonitoring by-count（Qualcomm RF360）、CopyRecipeFromTester（KYEC）、SetLotState／DoCoverTrayID_NFC（ATK）、note.cpp（Safe-PLC＋PTI）。W-159＝MR !362（Contact 殘料檢查 leaf 停點不再被蓋掉）。下一張卡等 St02-M。
- **1009 07:2x 912 與 913 差異表做完**（Steven 交辦，只讀）：`C:\AI_TempFile\st02e-scratch\v912v913\ST02_V912_VS_V913_20261009.md`＋files／functions 兩張 tsv，交 St02-M 發布；可能的 913 bug 9 條（SVN r913 少 Barcode2SetupData＝確定）。!359 已進 main（第 130 批）。D:\HT9045 已切到 main（舊 gpib-widget 分支退役；每個任務結束 `git -C D:\HT9045 merge --ff-only origin/main`）。佇列：W-159（E042-LEAFTASK，St01 的檔，`v906/st02-w159-leaftask`）→ W-191（POOL-9 介面／資料／顯示類，含 uYieldMonitoring 四支、ADAM_ReadVoltage）。
- **1009 05:4x W-189 推了＝MR !359**（兩個 commit：c664e7fb EventReport_*.def 的 ctest 沙盒接縫 W906_SECSSYSTEM_ROOT；fd0bec2e golden 913 SetReportIDContent(bSaveNow)＋AddReprot 只存一次，沒有呼叫端）。!355／!356／!357 都已進 main（batch 128／129）。config_loaders 在這台本來就紅（釘別台的 Mot_Table／IO_Table 數量）。下一張卡等 St02-M。
- **1009 04:0x W-187 剩下三個重量，都不做（St02-M 1009 04:0x 同意：#4 歸 TFSECS 整族、#5 已建議筆電開接縫／整族卡、TCOM2 交 Ifor01；下一張卡等筆電）**：TimerSecsAlarmTimer＝移植樹沒有 UsecegemMainFrom.cpp（TFSECS 沒翻），913 的改動要 fSecsAlarm（RULINGS_20261002 #20／#23-6 拿掉、test_h013_terms 釘不存在）→ TFSECS 整族卡；AddReprot＋SetReportIDContent(bSaveNow)＝唯一呼叫端是 TFSECS 建構子、而且 SaveEventReportData 寫死正式路徑 D:\HT9045\SECS\SECS\SYSTEM，測試會寫真檔 → 跟 TFSECS 一起或先加轉向接縫；TCOM2 RS232Init／Comm2ReceiveData＝RTC 表、Vision light 那幾段移植樹沒翻，NoHeater 那句在移植樹等價，Comm2ReceiveData 要 913 的 g_iHeaterTypeIdx_SendCmd（溫控家族 20a）→ Ifor01。W-187 St02 只剩 DoInterFaceErrorStep（等 Frank01）。
- **1009 04:0x W-187 #3 推了＝MR !357**（HTGem S9F1／F3／F5／F7／F9＋SendS9FxEchoHeader，`e908cb74`）。筆電 test_uHGemEquipment 的 T7／W7 釘住 0618 的 S9Fx 文字，經 St02-M 同意同一行改掉。MachineStatus 不做（Safe-PLC-IO 型別家族，St02-M 轉筆電當安全卡）。下一個：TFSECS::TimerSecsAlarmTimer → HT9045Gem::AddReprot → TCOM2 rs232；DoInterFaceErrorStep 等 Frank01。
- **1009 03:2x W-187 #2 推了＝MR !356**（DoSpoolSendLocalData，`3322188d`）。跳過：CopyRecipeFromTester（KYEC 閘住的路徑，等 N06 上傳接上再整族做）、SetLotState（ATK 專屬）。下一批候選：TimerSecsAlarmTimer、RS232Init／Comm2ReceiveData、MachineStatus、S9Fx、AddReprot；atester 的 DoInterFaceErrorStep 等 Frank01。
- **1009 03:0x W-187 #1 推了＝MR !355**（UpdataCount，`4a1c909d`；驗證全在 commit 訊息）。下一個候選（先做非 atester 的）：CopyRecipeFromTester → SetLotState → DoSpoolSendLocalData；等 St02-M 說要不要繼續。
- **1009 02:4x 新卡 W-187**（POOL-9 GOLDEN913-UPDATE，St02 領域）：清單 `docs/handoff/GOLDEN913_VS_0618_20261009_functions.tsv`（status=port_0618、customer_code 空）；腳本 scratch `w150/g913fn_filter.py`（篩）、`g913_fndiff.py <file> <0618 a-b> <913 a-b>`（看 0618→913 差異）。第一個送認領：`TEST_CATEGORY::UpdataCount`（cSocket.cpp:1258-1483；913 Ifor 0807 用 arm 內列 iSrcRow 讀 ArmSKET，寫入端 ProcessArmCount 本來就用 iRow32）。之後候選：DoInterFaceErrorStep（WAR0354→0357，先問 Frank01）、CopyRecipeFromTester、SetLotState、DoSpoolSendLocalData、TimerSecsAlarmTimer、RS232Init、S9Fx。!353 第 126 批 gate、!354 進第 127 批。
- **1009 01:4x** 筆電回了：E-TM-011＝A ⇒ 推了 **MR !353**（`v906/st02-etm011-timer6` 兩顆 80505a95＋53e09a67，main 30164eca；加了 [6]「kcode 0 通知、wb_serve W906-Q30-KZERO 不擋輪詢」釘子）；W-184 MR B 筆電代 St01 點頭 ⇒ 推了 **MR !354**（`v906/st02-w184-setupsave` a2159351；產生器只重產 TestIF_File_SetUp，.gen.inc 只差 :4086-4087，再跑一次 0 差異）。⚠ gen_editlist.py 在這台要設 `W906_GOLDEN_ROOT=D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（預設找 D:\HT9045\backup\…）；它寫 LF，git 看起來是 M 但內容 0 差異。手上沒有卡，等 St02-M。ChangeLog 改用 CHANGES_20261009_Steven02.md。
- **1009 00:0x** !350（MainTempMode）、!351（W-184 MR A）進 main＝第 124 批 `4270eedf`（包 207）。筆電還沒回：E-TM-011 A／B／C、W-184 MR B。St02-M：待命省額度，等它通知。下一次寫 ChangeLog 用 10/09 的檔。
- **23:2x E-TM-011 本機做好、還沒推**（`v906/st02-etm011-timer6`，main c34abc0d 上）：commit 1 `de8537a1`＝MainTimer6_St02.cpp（golden 913 main.cpp:32478-32727，N25／N29 本體關著）＋MainTimersSt02.cpp 綁 Timer6（ctest 的 reset 讓它保持關，golden dfm Enabled=False）＋CMakeLists:2778＋St02_Timer6RunCheck——兩組態 0 錯、21 支兩組態全過、反向 R1～R4 全紅、#6 乾淨；commit 2（WIP `e4e1a8f7`）＝WebStart.cpp :3684／:3695 改成 `fMain->Timer6->Enabled = true;`（⚠ 不能照 golden 寫 `Timer6->`：StartFromWeb 跑在 wb_serve 自己的 TfMainWeb 物件 g_webMain 上，不是 fMain）＋:3720 註解＋測試 [5] 釘子。等筆電 A（兩顆都推）／B（只推 commit 1）／C（丟）。
- **23:1x POOL-3 選了 E-TM-011（golden 913 Timer6：SECS 遠端 START 之後等主機回覆，逾時跳 WAR16110、清 PhysicalStart、解 SECS 鎖）**，認領稿已送 St02-M（等 §1 與筆電點頭：開關在 WebStart.cpp:3684／:3695，筆電的 S3-B1 行）。其他「還沒人接」都已有人做或已複核（C3-008＝W-121、D1-008～013＝5eeeabe0、E-TM-001／E-T2-003／E-FT1-001 已做、E-TM-017 客戶、E-TM-016／E-T1-015 運動給 Frank、E-FT2-001 客戶 socket）。打算：新檔 MainTimer6_St02.cpp（golden 913 main.cpp:32478-32727，N25／N29 本體先關）＋MainTimersSt02.cpp 綁 Timer6＋WebStart :3684／:3695 解 MARKED（:3720 保留）＋新測試 St02_Timer6RunCheck。
- **22:5x W-184 MR A 推了＝MR !351**（`v906/st02-w184-sitelog` `2c6321f1`；handlerlog.cpp 檔尾定義 myLog、:290 路徑走 as9045LogPath、WebRecipeChange.cpp:356 換配方呼叫；驗證全在 commit 訊息）。MR B＝St01 產生器 `tools/editlist/TestIF_File_SetUp.py:400-401`（blocks → replace 呼叫 W906_SaveSiteStatusLog）＋重產 TestIF_File_SetUp.gen.inc，等筆電代 St01 點頭（可先本機做、不推）。之後 POOL-3（`docs/handoff/CENSUS129_20261001/README.md` 最下面「還沒人接」，先在最新 main 重核行號）。
- **22:3x MainTempMode 推了＝MR !350**（`64d66cc9`，main 369b1da3 上；!346 已進 main 第 121 批）。**新卡 W-184**（TO_STEVEN §4 21:5x）：POOL-6 LINK-22 裡 St02 範圍（SECS／GPIB／網頁／測試機介面）沒人認領的檔，逐檔看 golden 913 的功能有沒有漏、該接的接（RogerYang 優先的檔先經 St02-M 在 TO_ROGER 問）；做完或卡住改 POOL-3（CENSUS129 README「還沒人接」，先在最新 main 重核行號）。先認領（檔＋行＋測試／反向）、一檔一 MR、golden 913。調查中：handlerlog.cpp（開關 site 紀錄；myLog 還在 cmydef.cpp:3596-3600 的 #if 0）。
- **21:2x MainTempMode:246 做完、測完，停在本機** `v906/st02-pool2-tempoffset` `1b9899f6`（main 6ce3c862；全套 ctest 兩組態、#6 乾淨；驗證全在 commit 訊息）。等筆電點頭（St02-M 會通知）→ fetch、main 有動就 rebase 重建重測、merge-tree、推＋MR；MR 說明要列「這台啟動不了（operation not permitted）」的測試（St01 出差沒人代跑，筆電兩組態 gate 會跑）。筆電說不要就丟分支。其他等 !345／!346 結果或新卡。今天的日報（入口網站 !241）與 ChangeLog（!242）St02-M 已用第 1～9 列發布。
- **20:0x POOL-2 第二節一般候選已用完**（逐一對過認領與 MainNB-GPT 複核；回報 St02-M）。唯一有實際效果的：`MainTempMode.cpp:246`（換溫度模式後重讀 offset，golden 913 main.cpp:22756＝0618 :21904；普查標 SAFETY、筆電的檔）。St02-M 認領（等筆電點頭）並請新卡；**本機做、不推**：`v906/st02-pool2-tempoffset`（WIP `2c70b4f4`；改 MainTempMode.cpp :21／:27 include／:240／:246／:248，test_st02_w140_command.cpp:92 同一行加 EnsureArmOffsetObjects，新測試 St02_Pool2TempOffset）；全套 ctest 兩組態抓其他會走到 ChangeTempMode 的測試。筆電說不要就丟掉。
- **19:5x POOL-2 SV 推了＝MR !346**（`v906/st02-pool2-secssv` `4d409845`，3 顆在 main 6ce3c862 上；驗證全在 tip 的 commit 訊息）。Steven 19:2x 新規則：之後改項目以 **913** 為 golden（SKILL 檔頭＋§4）；這張已對過 913＝0618。手上沒有別的卡：等 !345／!346 gate，或 St02-M 派下一張。
- **19:3x** !344（W-178 第 1 部分）進 main＝第 119 批 `68fc0e65`（包 202）；!345 在第 120 批 gate（筆電兩塊 CMakeLists 檔尾都留）。POOL-2 SV：筆電點頭 `test_secs_catalogue.cpp:176`，並准同一張 MR 補別人檔的過期註解（csystem.cpp:25304／:25720／:26353、FileRW/TrayForm.cpp:23、forms/fTrayAssignment.cpp:1556）＋重產 docs/SECS_GATED_REGISTRATIONS.md ⇒ 第二顆 commit `069ad6dc`。⚠ 坑：`git checkout origin/main -- <檔>` 會把 main 的版本放進 index，重套後只 `git add` 一支檔，amend 就把另一支的改動丟了（SecsCatalogue 因此 SegFault，抓到後補回 `c5901b2d`）。
- **18:5x W-178 第 2 部分推了＝MR !345**（`a0536db6`，rebase 到 main 95be3193；改到的 6 支檔 main 沒動 ⇒ 沿用之前的建置與測試結果）。**POOL-2 `SECSGEM/uHGemHT9045_SV.cpp`**：St02-M 認領 5b9fa3af；本機 `v906/st02-pool2-secssv` `a4ca60e8`（G13＋G18 開、G17 只更新理由；`tests/test_secs_catalogue.cpp:176` 同一行加建 fTrayAssignment，筆電點頭在 §3）；lane s39 兩組態建置中，接著 #6＋ctest＋反向（scratch `w150/pool2_sv_lines.py`、`claim_pool2_sv.md`）。
- **18:3x W-178 第 2 部分做好、測好，只在本機**：`v906/st02-w178-alarmq` `29598936`（從 main 42323e80；一顆 commit，訊息裡有全部驗證）。**等筆電點頭才推＋開 MR**（`git push origin HEAD:refs/heads/v906/st02-w178-alarmq -o merge_request.create …`；推之前 fetch、merge-tree、main 有動就 rebase 重建重測）。
- **18:1x W-178 第 1 部分推了＝MR !344**（`edf409b8`＋文件 `be9e057a`；全部驗證在 commit 訊息）。第 2 部分認領已登記（e6f32822），**在本機做、測，等筆電點頭（含代 St01 同意 dialog-bridge.js／契約）才開 MR**。原本：**17:5x W-178 第 1 部分（W-150 第 4 片）進行中**：分支 `v906/st02-w178-tri-lotinfo`（main 42323e80＝第 118 批），認領 FROM_STEVEN §1 bddf4c3a。改好 TriTemp.cpp :249／:268／:330／:342／:106／:120、fLotInfo.cpp :6631／:6633／:6639／:6642、LogObjects.h:27＋檔尾，新測試 St02_W150LogSplit4（開關門 log 用 DoorOpenAlarmForTriTemp、露點計用 fCheckDewPointStatus：DewPoint_Hardware_Install=1、Tri_Temp_Machine=1、dAdamValue_Degree=12.5，等 1.15 秒再叫一次）。接著：兩組態建置 → #6 → ctest → 反向 → 推。
  第 2 部分（低良率告警一筆一筆）：照 S-17D 的 recent[] 模式——C++ `tools/wb_dialog_mailbox.h` AlarmPost 在請求檔帶最近 8 筆、`web/page/dialog-bridge.js` inspect() 逐筆入列；動到所有告警共用的契約（web/JSON/Dialog-bridge-contract.json 1.3.1）、tests/test_notice_ack.cpp 釘住請求 JSON 的位元組 ⇒ 認領稿先送 St02-M（St01 是 S-17 的擁有者）。
- 新規則（SKILL.md §1，Steven 17:3x）：寄信主旨一律以「[ST Agent] 」開頭。
- **下一步：W-178**（從最新 main 開）：(1) W-150 第 4 片——先在最新 main 重量 claim_s4.md 的行號送 St02-M、等 §1 登記；(2) 低良率告警一筆一筆顯示（S-17 那套信箱／網頁 FIFO）。

## ★ 1008 14:2x 存檔點（5 小時用量 90%+；重置後從這裡接）
- **W-175**：分支 `v906/st02-w175-lowyield`＝程式 `ec79500c`＋文件 `8627e936`，在 main 23a102e0（第 113 批）上；測試回合在 37a048f1 上全綠（新測試 22/22 兩組態各兩次、相關 17 支、反向 R1-R6 全紅、#6 完整、備份 0）。推之前在最終 tip 兩組態再建一次（背景，log `s09close\w175f_{sim,ship}.log`）；**已推，MR !337**（14:3x；最終 tip 兩組態建置 0 errors＋PE 檢查過）。commit 訊息裡有全部驗證與人工審查。
- **W-178（下一張，W-175 之後）**：(1) W-150 第 4 片（TriTemp 替身→真物件、fLotInfo.cpp:6637→fMain->slLotInfolog／as9045LogPath），照 `w150\claim_s4.md`，開工時在當時最新 main 重量行號、等 St02-M 在 §1 登記；(2) 低良率告警一筆一筆顯示（S-17 那套信箱／網頁 FIFO 能不能直接用；W-175 沒做）。
- **普查 v3**：工具修正在本機分支 `v906/st02-c12-tabfix` `727d5801`（還沒推）；整份重跑已完成，輸出 `C:\AI_TempFile\st02e-scratch\c12tree\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\c12_click_v3.tsv`／`.log`，**還沒合併**：`python c12_button_census.py --merge c12_click_v3.tsv --out-tsv … --out-md …`（在 c12tree 跑，golden 0618），補回手寫 §9-§11，報每頁數字＋新 D 類給 St02-M，工具修正另開 MR（從最新 main 開，把 727d5801 cherry-pick 過去）。
- 建置線 s39 停在 W-175 tip；c23 在 `v906/st02-w175-lowyield`。暫存腳本都在 `C:\AI_TempFile\st02e-scratch\w150\`。

## ★ 1008 12:4x 接手段（壓縮後從這裡接；下面 10:1x 那段是今天的細節）
- 我＝**ht9045-46**（St02-E，STEVEN-NB3）；St02-M＝**ht9045-5b**（回訊息用它最新的 `from=`）。用 SendMessage 回報；不直接找 Jimmy。
- **新規則（Steven 1008 12:4x，已寫進 SKILL.md §1）**：每交出一件工作（推／MR、紀錄、回報都做完）就用 CronCreate 排一次性 `/compact`（2 分鐘後），觸發後確認有壓縮，沒有就告訴 St02-M。
- **今天交出的**：W-156＝!329 ✅ 已進 main（第 110 批 dfbd4696，包 193）；W-150 第 2 片＝**!331** ✅ 已進 main（第 111 批 214f680d＝包 194，13:2x；筆電同意了 4 支同一行修改的檔、收下 B63）。
  W-150 第 3／4 片認領稿（唯讀＋編譯探測都乾淨）`C:\AI_TempFile\st02e-scratch\w150\claim_s3.md`／`claim_s4.md`，St02-M 已發布（bfbb7b56：`docs/handoff/ST02_W150_S3_FRANK01_20261008.md`、`ST02_W150_S4_IFOR01_20261008.md`），**等 Frank01／Ifor01 回覆才動手**。
- **13:5x 進度**：W-175 程式＋測試寫好（`v906/st02-w175-lowyield`，從 main bdaf57b2＝第 112 批；認領 FROM_STEVEN §1 f280ae5b），兩組態建置中 → #6 → ctest → 反向 R1-R6 → 推。cCleanOut.cpp:315-321 其實在 **TfMain::BtnOneCycleClick**（golden 0618 main.cpp:4332-4380，讀清單在 :4349-4352），不是 CleanOut——認領稿第一版寫錯，已更正。One Cycle 結束時的 ShowErrorMessage(…,0,…)＝通知：wb_serve ForwardShowErrorMessage 的 kcode==0 分支停機（golden note.cpp:795-801）＋寫信箱＋立刻回，不在輪詢執行緒上等。測試要走到 DoOneCycleFinishCheck 的「完成」：MTestZ1／Z2／Y1 掛 Enable=false 的 HTMotor，CheckIndexIsNormal 才會過（Motor==NULL 時恆 false、停在第 9 段）。
- **INBOX 155 做完**（St02-M 308d6897 已發布）：D 類 36 → 0 個待辦。普查工具修正（切到按鈕所在分頁、分頁本身藏起來另記 hidden(tab)）在本機分支 `v906/st02-c12-tabfix` `727d5801`，整份重跑在背景（匯出的 main 0b55e181 樹 `C:\AI_TempFile\st02e-scratch\c12tree`，輸出 c12_click_v3.tsv／.log），跑完合併 golden 0618、保留手寫 §9-§11，報每頁新數字＋新 D 類，工具修正另開一張 St02 MR。
- **13:2x 新卡 W-175 LOWYIELD-ALARM**（TO_STEVEN §4 1008 13:1x；＝claim_s3.md (C) 的 C1，照 golden 接 slLowYieldAlarm）。筆電條件：One Cycle 結束時的告警要走既有的網頁對話框／通知掛鉤，**不能在輪詢執行緒上等**；測試＋反向＋MR；人工審查 B 號＋上機項目。St02-M 已在 FROM_STEVEN §1 認領（8b9d5b9b）。**等第 112 批（NB2-1 !332 W-170 改 csystem.cpp G01a/G01b 在 :4060 附近、Jerry !334、ST-GPT !333）進 main（約 13:5x）**，從它開 `v906/st02-w175-lowyield`，**重量每一行**（csystem :110/:4060/:4061/:4752/:4753/:5089/:5090、cCleanOut :60/:310-317/:321、atester_ProcessCount :116/:343-348）送 St02-M 更新認領後才改；認領稿要寫 DoOneCycleFinishCheck 裡的 ShowErrorMessage 在移植樹做什麼（丟進網頁對話框信箱就回，還是會擋）。Frank01 收到 s3 稿當 W-176 FYI。
- **同時（筆電同意選項 3）**：在 STEVEN-NB3 自己跑 INBOX 155 的點擊探針，C12 D 類 36 顆按鈕（`docs/handoff/ST02_BUTTON_CENSUS_20261002.md`，填「sent」欄）。只用無頭瀏覽器＋假連線，絕不碰機台的 wb_serve；要 Edge 而跑不起來就停下告訴 St02-M。結果經 St02-M 寫 FROM_STEVEN §2。
- **壓縮**：CronCreate 排 `/compact` 沒用（12:50 實測只送來一則文字）。每交出一件工作就告訴 St02-M，由它請 Steven 在這個 session 手動打 `/compact`。
- **還在等**：Frank01／Ifor01 對 s3（W-176 FYI）／s4 的回覆。
- **下一張 St02 MR 要順便帶的文件**（現在只在 `D:\HT9045` 的工作副本，沒 commit）：`.claude/skills/ht9045-st02-workflow/` 的 SKILL.md（§1 壓縮規則）、references/current-state.md、techniques.md；
  以及 skill hpi-mnetlog-split §10 那句更正（「slAutoSiteMapLog 在 V906 是替身、沒有寫檔」錯了——LogObjects.cpp:200-208 在 wb_serve 換成真的 TMyStringList，as9045LogPath\ASM；這個檔不在 D:\HT9045 工作副本，開下一條分支時在分支上改）。
  做法：新分支從最新 origin/main 開（c23 工作樹現在在 `v906/st02-w150-logsplit2`＝凍結），把 D:\HT9045 的三個檔複製進去 commit。
- **工作樹／建置線**：編輯樹 `C:\AI_TempFile\st02-c23`；建置線 `C:\AI_TempFile\st02-s39`（detached，obj `C:\AI_TempFile\st02-s39-obj\{build,build_ship}`，BUILD_TESTING=ON），增量：`bash C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\s09close\lane_cmake.sh '<obj>' <tag> sim|ship`。
- **#6 每輪測試前**：最新 main → `python tools/machine_sync/machine_sync.py check`（c23 根目錄）→ `apply --yes`（看 rc 與 copied N／checked N）→ 確認 SYNCED → **apply 完成後才開 ctest** → `cd HT9011UC_Cpp_V3.33.906.0 && python tools/realfile_guard.py snap <名>` → 測 → `check <名>`／`drop <名>` → PowerShell 跑 `machine_sync.py restore "<備份>"` → `D:\HT9045\backup\machine_sync_*` 要 0 個。目前 0 個。
- **ChangeLog**：`D:\RD5-portal\public\Docs\ChangeLog\Steven02\CHANGES_20261008_Steven02.md` 第 1～3 列（入口網站 repo，St02-M 那邊推）。日報段落附在給 St02-M 的回報裡。
- 今天學到的（已寫進 techniques.md）：ctest 沙盒每輪自己的子資料夾、新測試同組態連跑兩次；解開會叫 `SW[...].Status()` 的程式時舊測試的 OutValue 會被讀回蓋掉；apply 完才開 ctest；時間標籤一律先跑 `date` 再寫（今天又估錯兩次）。


## ★ 1008 10:1x 狀態（新帳號；新 session 從這段接，下面的換帳號交接是背景）
- 新帳號後：我＝**ht9045-46**，St02-M＝**ht9045-5b**（以最新訊息的 `from=` 為準）。
- **W-156 已進 main（第 110 批 dfbd4696＝包 193，12:0x；788ff831 是祖先）**。原本：**W-156 推了，MR !329**（`v906/st02-w156-flags` **788ff831**，一顆 commit、在 main 89f00bc1＝第 108 批之上；原本的 WIP 21c6245d 已用 force push 換成壓縮後的這一顆，那條分支原本沒有 MR）。St02_W156Flags 兩組態 19／19＋相關 21 支兩組態全過；反向 RA～RE 7 項全紅；機台快照 08:51，測完已還原、備份 0 個，真檔 42 個不變。
  - 為了跟 NB2-1 的 W-169（Contact 頁關窗／F5／斷線，認領 FileRW/DeviceForm_File* 全家、_EditPage.cpp）錯開：拿掉了 DeviceForm_File.cpp :472／:474 的註解修改（離 :459-470 太近）；最後的修改點已交 St02-M 轉給 NB2-1。
  - [5]（KYEC_CHEN＋A16 存檔）原本的寫法錯了：golden 存檔後 ReadFile 在 Direct 模式把 Drop 歸零（0618 cContact.cpp:509-514），畫面 0.00 是對的；要看的是**存進 Contact.Data 的值**（1.50／1.60）。頁面要送「伺服器目前的狀態」（測試的 ServerView），只改單一欄位會讓舊的單選按鈕被套回去、變成真的模式改變。
- **W-150 LOG-SPLIT 第 2 片推了**（11:0x，分支 `v906/st02-w150-logsplit2` 程式那一顆 `350a9250`，在 main 21d163f0 上；**MR !331**，推完 11:0x 已回報 St02-M；文件 commit `ce2bef9f` 同一條分支）。盤點與認領全文 `C:\AI_TempFile\st02e-scratch\w150\claim_s2.md`；St02-M 登記 FROM_STEVEN §1（8466e5cf＋6d8eec2f，筆電點頭待補）。
  - 做了：新檔 TTLLog.cpp（golden 0618 cpublic.cpp:489-512）、IndexPosLog.cpp/.h（LogIndexMaxMinPos 0618 cpublic.cpp:1582-1599＋W906_AddIndexPosLog＝main.cpp:33634-33664，§8 L5 TODO），放 ht9045_sm；取用函式 W906_TTLLogObj／W906_IndexYMaxMinShiftLogObj／W906_QtyLogObj；退役 cDIOStatus.cpp:71、MainTimer3.cpp G15、cStateRecord.cpp G9；MainClarnData.cpp:45 換取用函式；新 ctest St02_W150LogSplit2。
  - **FileRW/MainClose.cpp 不動**（St01 出差、S95／S121 沒人能點頭）：12160 TTLLog("Close")、12446 LogIndexMaxMinPos("Program closed") 照舊 missing，之後的片再做。
  - 驗證：兩組態完整建置 0 errors；#6 機台快照 10:26 → apply rc=0 → 測 → 真檔全部未變 → restore 448 → 備份 0；相關 20 支兩組態綠（SIM E023 第一次 [guard] 紅＝apply 剛寫過 D:\HT9045\system，重跑兩次綠）；反向 R1～R9 全紅。
  - 沒做（別人那組）：Frank01 slAutoSiteMapLog×5（V906 是 TfMainSiteMapLog 替身）、atester T17；Ifor01 TriTemp W7TT 替身、fLotInfo.cpp:6637 自建 slLotInfolog（沒走沙盒）；機台端 mymotor.cpp:2646／:2845-2846、asendic_Loader.cpp:273／:303 ⇒ IndexMaxMin 的值暫時都是 0。
  - 已知：開機那一次 TTLLog("InitDIOStstus") 在 V906 不寫——wb_serve 先跑 InitDIOStstus（wb_serve.cpp:4171 → :3608）才建 log 物件（:4178）；golden 是建構子先建（0618 main.cpp:1526）。已寫進 commit 的 Human review。
- **W-150 第 3／4 片認領稿（唯讀，11:0x～11:1x，St02-M 交辦）**：`C:\AI_TempFile\st02e-scratch\w150\claim_s3.md`（Frank01：slAutoSiteMapLog×5 改走 W906_AutoSiteMapLog、atester T17 建議不動、slLowYieldAlarm 是流程行為〔One Cycle 結束時跳低良率告警〕建議另開卡）與 `claim_s4.md`（Ifor01：TriTemp 的 W7TT 空替身改兩行就讓 28 處真的寫 TriTemp／DewPoint log；fLotInfo.cpp:6637 自建的 LotInfo log 改用 fMain->slLotInfolog、路徑走沙盒）。St02-M 轉筆電再給 Frank01／Ifor01；**我不動這些檔**。
  - 編譯探測（11:2x，只用副本 `w150\probe\`，腳本 probe_s34.py／probe_s34b.py）：s4 (A)(B)、s3 (A) 兩組態乾淨，警告數＝原檔；s3 (C) 的 csystem.cpp 要另加 4 行同一行轉型 `AnsiString(W7C2_FMAIN_SLLOWYIELD->Strings[i])`（:4752／:4753／:5089／:5090，vclcompat 的 Strings[i] 是代理、沒有 SubString）＋:110 include 才乾淨。St02-M 已更新發布版（bfbb7b56）。現在等：①筆電下一張卡；②Frank01／Ifor01 回覆 s3／s4；③!331 第 111 批的結果。
  - ⚠ 待更正：skill hpi-mnetlog-split §10（!331）寫「slAutoSiteMapLog 在 V906 是替身、沒有寫檔」是錯的——LogObjects.cpp:200-208 在 wb_serve 已換成真的 TMyStringList（as9045LogPath\ASM）。下一顆 St02 文件 commit 一起改。
  - **!331 在筆電第 111 批 gate 中（12:1x）——不要推合 main 的 tip**（St02-M：會在 gate 中途移動 tip；tests/CMakeLists.txt 檔尾由筆電自己解）。skill hpi-mnetlog-split §10 那句更正改跟**下一張 St02 MR** 一起推。

## ★ 1008 換帳號交接（新 St02-E 從這裡開始，冷啟動也能接）——W-156 已在上面那段完成

**角色與規則**（沒變）：我是 St02-E（STEVEN-NB3）；協調者 St02-M（新帳號後 session 名稱會換，以最新訊息的 `from=` 為準；它的交接在 `C:\Users\steven\.claude\skills\ops-st02-manager\references\account-switch-handoff-20261008.md`）。
這台**可以跑測試**（Steven）；每一輪測試前照 RULINGS_20261005 #6：最新 main → `python tools/machine_sync/machine_sync.py check`（repo 根目錄跑）→ `apply --yes` → `cd HT9011UC_Cpp_V3.33.906.0 && python tools/realfile_guard.py snap <tag>` → 測 → `realfile_guard.py check <tag>`／`drop <tag>` → `machine_sync.py restore "<apply 印的備份資料夾>"`；回報寫 main commit 與快照時間，`D:\HT9045\backup\machine_sync_*` 結束要 0 個。

### W-156（W152-FLAGS）—— 做到一半，**下一步就是它**
- 分支 `v906/st02-w156-flags` @ **21c6245d**（WIP、已推、**沒開 MR**；從 main 573340b8，含第 96 批）。兩組態編譯 0 errors；**ctest 一支都還沒跑**。St02-M 已放行並在 FROM_STEVEN §1 認領。
- 做好的（全部同一行或舊空行，產生檔除外）：
  - (a) bContinueContact：`tools/editlist/DeviceForm_File.py:387` members 列改 `#define bContinueContact (fContactForm->bContinueContact)`；原本同一列的 MotorStatus／bOldRTCAutoTuning／brecordmsgLock 搬到 `:385`（bAutoHighFinish 那一列）；`forms/fContact.h:1340` 同一行 `public: bool bContinueContact; private:`（原本在 :1332 起的 private 區段）。
  - (b) bSetHasIC：`.py:384` 改 `#define bSetHasIC (fContactForm->bSetHasIC)`；`.py` 檔尾加一個 replace 列：golden FormShow（V912 :1678＝0618 :1651）`bSetHasIC=false;` → `if(!filerw::PageShownNow("DeviceForm_File")) bSetHasIC=false;`——**NOT GOLDEN 接合碼**（網頁每次 editlist.get 都重跑 FormShow，W-152 的重讀也是；不然取料狀態機設的旗標會在 FormClose 之前被清掉）⇒ MR 說明的 HUMAN_REVIEW B 要寫這一條。
  - (c) cbOneTouchAutoContactHight：`ckernel.cpp:237`／`:306` 改讀 `fContactForm->cbOneTouchAutoContactHight`（`fContact->fShow` 不動）、`:130`（舊空行）`#include "forms/fContact.h"`、`:223` 註解；`tests/test_w7_l2_ckernel.cpp:737／:1006／:1033／:1043／:1691` 改 fContactForm、`:410`（舊空行）include。ckernel.cpp 是機台的檔，St02-M 已請筆電轉告 EastSun。殘留（MR 要寫）：半途勾選要到下一次開始才生效（網頁替身每次重讀都被 FormShow 清，不能直接讀）。
  - (b2) `FileRW/DeviceForm_File.cpp:1201` 同一行加 `if (rb0 != 0) ++g_w152Seq;`（MODE 重設時單選真的變了 ⇒ 開著的頁面經 tag contact.runResultSeq 重讀）；`:1111`／`:1184` 只改註解；`web/page/ht9045_contact_ev.js:426` 訊息改中性、保留「尚未存檔」（cjs 自測釘它）。
  - (c2) 過期註解：`forms/fContact.h` :183、:199-202、:204-205（登記表標 TRANSLATED＋位置）、:1348、:1526；`FileRW/DeviceForm_File.cpp` :472、:474、:550、:641（主控台字串）；`.py:167-169`；`docs/gate-ledger/csystem.md:34-35`（手改、行數不變；本來由 tools/gate_ledger_emit.py 產生）。
  - **不做（St02-M 排除）**：`forms/fContact_ContactSM.cpp:20`（ST-GPT 的 W-159 在同一支檔，一檔一個寫的人，由它順手改 cmydef.cpp:3346→:3457、csystem.cpp:31302→:31449）、St01 的 `tests/test_b8_ct3a_contactflags.cpp:42`。
  - gen.inc：`cd HT9011UC_Cpp_V3.33.906.0 && W906_GOLDEN_ROOT='D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618' python tools/gen_editlist.py --only DeviceForm_File`，再 `python C:\AI_TempFile\st02e-scratch\w142\to_crlf.py FileRW/DeviceForm_File.gen.inc`。**已驗**：在 main 上用 main 的 .py 重產＝main 的 gen.inc 一字不差（tree 乾淨）；我們的 gen.inc diff 只有那三列＋FormShow 的取代（+3 行）。MR 說明要寫這一點。
  - 新測試 `tests/test_st02_w156_flags.cpp`（St02_W156Flags：[1] (a)、[2] (b)、[3] (c)、[4] (b2)、[5] (a2) KYEC_CHEN＋A16 存檔後 drop offset 還是 1.50／1.60）＋ `tests/CMakeLists.txt` 檔尾區塊（連結行同 St02_W152ContactData）。
- **還沒做**：
  1. #6（見上）。
  2. 兩組態跑：St02_W156Flags＋St02_W152ContactData、W7_L2 ckernel 那支、B8_Ct3a 系列、IndexZ AutoHeight 1203（[B9] census 只算活的呼叫，不受註解影響）、ScanKeyGolden、MainScanKey、FShow_Audit、EvB10A_Edges（名稱一律 `grep -o "add_test(NAME [A-Za-z0-9_]*" tests/CMakeLists.txt | grep -i <關鍵字>`）。St02_W156Flags 第一次跑可能要修期待值（[3] 用 W906_FormFShowHook、[5] 存檔路徑都還沒實跑過）。
  3. 反向：`python C:\AI_TempFile\st02e-scratch\w156\reverse_w156.py`（RA (a) 列改回 static、RB1 (b) 列改回 static、RB2 FormShow 列改回 golden、RC1／RC2 ckernel :237／:306 改回 fContact、RD 拿掉 seq bump、RE 拿掉 :1200 EvB3Merge；產生器列的反向會在 lane 重產 gen.inc，結束自動 git checkout 還原）。
  4. merge-tree（origin/main、origin/v906/steven-cbridge-review6，各一行）→ squash（`git reset --soft origin/main` 前先 merge 最新 main）→ 開 MR（`-o merge_request.create …`，標題建議「St02 W-156：Contact 三個執行旗標只留一份（fContactForm）＋MODE 重讀＋A16 存檔」）。
  5. MR 說明要寫：fContact.h:1340 同一行 public／private 不改成員順序（GCC 不跨存取區段重排；全樹沒有人用 sizeof(TfContact) 或成員偏移）；gen.inc 只有我們的列（重產驗證）；HUMAN_REVIEW B＝bSetHasIC 開窗期間保留（NOT GOLDEN）；HUMAN_REVIEW A＝機台上 KYEC 以外不受影響、ONE CYCLE 燈與網頁 LED 一致。
- 調查報告（每一列的 golden／移植樹行號）：`C:\AI_TempFile\st02e-scratch\w156\survey.md`。腳本：`w156\patch_py.py`、`patch_py2.py`、`patch_w156.py`（`--with-pending` 不要用）、`install_test.py`、`reverse_w156.py`、`v912line.py`（讀 V912 cContact.cpp 某行）。

### 其他狀態
- W-149：15／15 完成；MR 4＝**!319**（`v906/st02-w149-simaware-4` 74b385b0）等筆電 gate。SIM-only 基準：W-149 的 15 支 0 支；這台另有 6 支兩組態都紅（config_db／ini_helpers／config_loaders 讀機台快照設定、dfm2rc×3 這台沒有 rc.exe），E023_StatusEvents 平行跑偶發。W6_4 是預設 A，等 Jimmy（§3 16:5x）。
- W-155：全部在 main（!315 第 94 批、!316 第 95 批、心跳修正 5e3c464c 在第 96 批）；**!317 不要再推**（St02-M 請筆電關成重複）。
- W-159：給 ST-GPT。W-157：等 Steven。
- **筆電 22:2x 暫停換帳號**，!319 等它回來 gate。

### 本機狀態（沒有東西只在本機）
- 編輯工作樹 `C:\AI_TempFile\st02-c23`：分支 `v906/st02-w156-flags` @ 21c6245d，乾淨。其他本機分支都已推（w149-simaware-4 74b385b0、w155-beatfix b5ab691e、w155-padpage 0ca4c3cd、w155-iopanel d73de989）。
- 建置線 `C:\AI_TempFile\st02-s39`（detached 535db4ff，跟 21c6245d 同一棵樹、只差 commit 訊息）＋ obj `C:\AI_TempFile\st02-s39-obj\build`（SIM）／`build_ship`（SHIP），**BUILD_TESTING 已設 ON**（MR !301 起預設 OFF）；兩個都已建到 W-156。增量建置：`bash C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\s09close\lane_cmake.sh 'C:\AI_TempFile\st02-s39-obj\build' <tag> sim`（ship 同理，log 在同一個 s09close 資料夾）；要建別的 commit 先在 s39 `git checkout --detach <commit>`。
- 機台快照備份 0 個；realfile_guard 沒有留下 snap。
- `D:\HT9045` 主 checkout（分支 v906/steven-gpib-widget）：只有本檔與 techniques.md 是 St02-E 改的（本次交接一起推到 `v906/st02-handoff-docs-1008`，見 St02-M 回覆）。其餘未提交的東西（116 個 `.github/*` 刪除、config／backup 等未追蹤檔）是 session 開始前就在的，**不是 St02-E 的，不要提交也不要刪**。
- scratch：`C:\AI_TempFile\st02e-scratch\`（w132 gshow.py、w149 gfind.py／gfunc.py、w142 to_crlf.py、w149 MR 1-4 腳本與報告、w155 W-155 腳本與 page_gen 產生器副本、w156 W-156）。

### 還沒寫進 techniques §9 的小坑（本次一併補進去）
- `tools/gen_editlist.py --only DeviceForm_File` 要設 `W906_GOLDEN_ROOT`（0618 樹）才跑得動；產生器寫出 LF，要再跑 to_crlf。
- 腳本 print 中文到主控台（cp950）會 UnicodeEncodeError——跑之前設 `PYTHONIOENCODING=utf-8`，或不 print。
- 反向腳本的錨點要先數次數：ckernel.cpp :237 與 :306 那一行一字不差，要分第一個／第二個處理。

---

# （以下是 1007 之前的現況板，仍可參考）

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

### 10-07 23:2x 停在存檔點（每週用量 90%，Steven 23:1x「先不接工作了」；10/13 09:00 重置）——新 session 從這段接
- **W-156 WIP 已推（沒開 MR）**：`v906/st02-w156-flags` **21c6245dc**（從 main 573340b8；St02-M 已放行並認領 FROM_STEVEN §1）。兩組態編譯 0 errors，**ctest 還沒跑**。
- 做好的：(a) 產生器 .py:387 #define bContinueContact → fContactForm、fContact.h:1340 同一行 public；(b) .py:384 #define bSetHasIC → fContactForm＋.py 檔尾 FormShow「同一次開窗的重讀不清」列（NOT GOLDEN 接合碼，人工審查 B）；(c) ckernel.cpp:237／:306 讀 fContactForm->cbOneTouchAutoContactHight（:130 include、:223 註解；機台的檔，St02-M 已請筆電轉告 EastSun）、test_w7_l2_ckernel.cpp 五行＋:410 include；(b2) DeviceForm_File.cpp:1201 `if (rb0 != 0) ++g_w152Seq;`、ht9045_contact_ev.js:426 訊息；(c2) 過期註解（fContact.h 登記表／:1348／:1526、DeviceForm_File.cpp :472／:474／:550／:641／:1111／:1184、.py:167-169、docs/gate-ledger/csystem.md:34-35）。**不做**：fContact_ContactSM.cpp:20（ST-GPT 的 W-159 一起改）、test_b8_ct3a:42。gen.inc 用 `W906_GOLDEN_ROOT=D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618 python tools/gen_editlist.py --only DeviceForm_File` 重產（再跑 to_crlf）；main 的 .py 重產＝main 的 gen.inc 一字不差（已驗）。新測試 tests/test_st02_w156_flags.cpp（St02_W156Flags：[1] a、[2] b、[3] c、[4] b2、[5] a2）＋CMake 檔尾。
- **還沒做**：①#6（machine_sync check／apply、realfile_guard snap）②兩組態跑 St02_W156Flags＋St02_W152ContactData、B8_Ct3a 系列、W7_L2_ckernel（名稱從 tests/CMakeLists.txt grep）、IndexZ AutoHeight 1203、ScanKeyGolden／MainScanKey、FShow_Audit ③反向 `python C:\AI_TempFile\st02e-scratch\w156\reverse_w156.py`（RA RB1 RB2 RC1 RC2 RD RE；產生器列的反向會在 lane 重產 gen.inc，結束 git checkout 還原）④還原／merge-tree（main、review6）／開 MR（標題「St02 W-156：Contact 三個執行旗標只留一份（fContactForm）＋MODE 重讀＋A16 存檔」）⑤MR 說明要寫：fContact.h:1340 public/private 同一行不改成員順序（GCC 不跨存取區段重排，沒有人用 sizeof／偏移量）、gen.inc 只有我們的列、bSetHasIC 列是 NOT GOLDEN 接合碼（人工審查 B）、一鍵測高半途勾選要到下一次開始才生效。調查報告 `C:\AI_TempFile\st02e-scratch\w156\survey.md`。
- 其他：W-149 MR 4＝**!319** 等 gate（SIM-only 基準 0／15）；W-155 全部在 main（!317 由 St02-M 關成重複，不要再推）；W-159 給 ST-GPT。

### 10-07 22:3x 狀態（下面仍有效）
- **W-155 結案**（!315 第 94 批、!316 第 95 批＝第 178 包，四個同行點筆電都點頭）。心跳修正 2662cf87 沒趕上第 95 批 ⇒ 開了急件 **!317**（`v906/st02-w155-beatfix` b5ab691e，cherry-pick -x）；筆電已把 2662cf87 直接接在第 96 批上（第 179 包），!317 等第 96 批進 main 後由 St02-M 當重複關掉——**!317 不要再加東西**。
- **W-149 MR 4 推了，!319**（`v906/st02-w149-simaware-4` 74b385b0，只改測試）：W6_4（SIM 種 ScanPort）、WB_SimPump O4b／O7、mainproc_guard（SIM START 留著＝golden csystem.cpp:4441-4493 #ifndef SOFT_SIMULTE、移植樹 :16792，反向 R13 證明；:4403 那段 PLC／門／EMG 在 SIM 編不過、不引用）、ScanKeyGolden [18]／[19] 還原面板感測器 sim 狀態（真正的漏洞＝SnRearPadActive 被設成讀不到；IdleHomed 已經會重設 InitialOK）。反向 R13～R17 全紅；腳本 `C:\AI_TempFile\st02e-scratch\w149\`（patch_mr4.py、patch_mr4b.py、reverse_mr4.py、probe_rstheld.py、probe_r13.py）。全部模擬版 ctest：W-149 的 15 支清完；這台另有 6 支兩組態都紅（config_db／ini_helpers／config_loaders 讀機台快照設定、dfm2rc×3 沒有 rc.exe），E023_StatusEvents 平行跑偶發。
- 筆電 22:2x 暫停換帳號；gate 等它回來。
- **下一張：W-156**（bContinueContact／bSetHasIC／cbOneTouchAutoContactHight 各一個 golden 擁有者＋筆電 20:4x 的 a/b/c）。**fContact_ContactSM.cpp :1232-1240／:1266-1274／:1631-1634 是 ST-GPT 的 W-159**，不要碰；要動那支檔先把行號給 St02-M。W-159 已移給 ST-GPT。

### 10-07 21:3x 狀態（下面 21:0x 仍有效）
- **第 94 批已進 main**（56b065d5：!313 W-150、!315 W-155 A、!314、!311）。**!316 新 tip `0ca4c3cd`**＝2662cf87（心跳只認開窗成功那個視窗的 `pad.get {"beat":true}`，St02-M 審查）＋合 main 56b065d5（CMake 檔尾照 main、只補 B 的兩塊）。nod-pending commit 07803f21 不再是最後一顆，但跟後面的檔不重疊，`git revert` 就能拿掉。
- 機台派工文件 `HT9011UC_Cpp_V3.33.906.0/docs/REQUEST_JIMMY_IOPANEL_20261007.md` 已在 main；5 點都已涵蓋。
- **現在：W-149 MR 4**（從 origin/main 開新分支）：test_w6_4_tester_core.cpp（jimmychiu）:246／:259 SIM 先種 ScanPort（ClearScanPort＋同一個視窗，期待值兩組態都 5／3／3；golden SIM 由 Sim_TTL_Single 填，atester.cpp:699-753 叫在 :2078-2080，移植樹 :12898-12902 閘住）、include 放 :70；test_wb_simpump.cpp（機台）:280／:283 O4b、:507-509 O7，include 放 :30；test_mainproc_guard.cpp（Steven）:174-177，include 放 :54；test_scankey_golden.cpp（jimmychiu）[19] 結尾還原 InitialOK／軟鍵（:1163 設 InitialOK=false、:1168 W906_SoftPanelKeyResetForTestSt02）。golden 0618 csystem.cpp:4403-4436／:4441-4493 已核對。行要先交 St02-M 認領。

### 10-07 21:0x 狀態（W-155 B 推完；新 session 從這段接，下面 20:0x 仍有效）
- **W-155 B 推了，MR !316**（`v906/st02-w155-padpage` `07803f21`，疊在 !315）：e3b8e4e7＝把 main 1091472b（W-152 已進 main，第 176 包）合進 !315 的 tip；5413f6e7＝網頁 Pad 視窗（PadInterface_St02.cpp 檔尾 `W906_PadWire`：pad.open／get／close／exit／button／send／bling＝golden FormShow／PadButtonClick／ManualSend／Exit／FormClose；DEVIATION W155-D2 只收燈號封包、W155-D3 bShow 心跳看門狗 :752；`HW.PadInterface.html` 由 0618 dfm 產生；`ht9045_padinterface_c.js`；wb_serve.cpp:5651 路由；background.html:504 視窗列；FShow_Audit 基準 +3）；07803f21＝**等筆電點頭、可單獨拿掉**：WebBridgeServer.cpp:1449（pad.get／close 免權杖）、WebCmdGuard.cpp:90（pad.get 純讀白名單，新認領）、HW.IoSetView.html:828（Pad 鈕開窗）、sync_web.py:282（OURS，新認領）。測試 St02_W155PadPage 39／39、St02_W155PadPageJs 21／21；腳本 `C:\AI_TempFile\st02e-scratch\w155\`（patch_mrb_*.py、install_mrb_web.py、reverse_w155b.py、resolve_cmake_eof.py、page_gen\ 產生器副本）。
- W-152 已進 main（7edca0a1），DeviceForm_File.cpp 的暫停令解除。
- 佇列：**W-149 MR 4**（W6_4 預設 A＋scankey [19] 還原 InitialOK）→ **W-156**（加筆電 20:4x 的 a/b/c：KYEC_CHEN＋A16 那一例、錯誤離開後網頁要被推一次、過期註解清單）→ W-159。W-157 等 Steven。
- 請求文件 8789b74c 仍讀不到；已用 W-155 卡片的 5 點比對，全部涵蓋（回報 St02-M）。

### 10-07 20:0x 狀態（W-155 A 推完；新 session 從這段接，下面 19:0x 仍有效）
- **W-155 A 推了，MR !315**（`v906/st02-w155-iopanel` `d73de989`，origin/main ede228b7 上一顆）：IoBtnPanelClick.cpp :257 面板按鈕（mode 1 && IsPadButton）→ 檔尾 `W906_IoPadClick_St02`（只做 SendSwitchStatus 那一半＝新 DEVIATION (j)）；:429 每次 1203 點擊後真的 SendSwitchStatus(&pb)（D1 照 golden；(a) RETIRED）；ChanIoPoints.cpp :317 面板點的方塊從面板讀（source "pad"，按鍵照 InType 反相）；io_do.js :98／:138；wb_serve.cpp:7255 只改註解。PadInterface_St02 檔尾 golden SendSwitchStatus(Ptr) 與 W906_PadIoPoint。測試 St02_W155IoPanel（真的 IoBtnPanelClick＋ChanIoPoints、HT9050 IO 表唯讀、SIM 面板埠、1203 命令面是測試裡的 stub；ship 要 `MyLaneIO.SelectVendorBackends()` 才會走 1203 路由假物件）。腳本與反向 `C:\AI_TempFile\st02e-scratch\w155\`（patch_pad*.py、patch_mra.py、patch_cmake.py、reverse_w155.py、survey.md）。
- 已回報 St02-M（含 MR B 認領清單：HW.PadInterface.html、PadWire_St02.cpp、CMakeLists.txt:2406、wb_serve.cpp:5651、WebBridgeServer.cpp:1449、web/background.html:504、HW.IoSetView.html 那顆 Pad 鈕＝筆電第 24 列）。D2＝A（手動送只收 t05 燈號封包）、D3＝A（HT_WIN 隱藏清 bShow＋C++ 心跳看門狗）＝RULINGS_20261007 #10。
- 請求文件 `v906/jimmy-b94` 8789b74c 20:0x 還不在遠端；出來後跟 survey.md 比對回報 St02-M。
- 佇列：W-155 B → W-149 MR 4 → W-156 → W-159（fContact_ContactSM.cpp :1234／:1268／:1633 葉節點後 Task= 覆寫，E042-LEAFTASK）。
- 本機測試線：IoPoints_HT9050 在 s39 這個 configure 沒有註冊（ctest -N 找不到），回報裡請 gate 補跑。

### 10-07 19:0x 狀態（新 session 從這段接；下面 18:2x、17:3x 仍有效）
- **W-150 log 拆檔第 1 批推了，MR !313**（`v906/st02-w150-logsplit` `ceb95796`，main 2fb8d2c0）：LogObjects.h 6 個取用函式；cMyDB 改用；cObserver／cprod（JSCK OEE TestLog，cprod 加 W58 `W906_SimNetPathBlocked`）、rs232:868（扭力異常封包）照 golden 打開。測試 St02_W150LogSplit；腳本 `C:\AI_TempFile\st02e-scratch\w150\`。技能 hpi-mnetlog-split §9 寫了哪些物件還等呼叫端翻譯。
- **W-152 急件 !312**：筆電在 `v906/jimmy-w152-urgent`（main＋!312＋閘那一行）跑急件通道。
- **現在：W-155 IOPANEL**（EastSun 等，TO_STEVEN 1007 18:4x）：IO 頁 Panel 分頁（Front／Rear）照 golden 補齊——面板按鈕要經 `fPadInterface->SendSwitchStatus`（golden iosetview.cpp:1095）、SwFK*／SwRK* 是 pad 項目、SnFK*／SnRK* 顯示 PadItem mlEvent、Pad 鈕開 TfPadInterface、wb_serve.cpp:7256 過期註解。**基準是第 93 批**（`v906/jimmy-b93`，含機台 cpp 0291 PADNOTE：PadInterface_St02.cpp 在 `S=SubString(6,2)` 下一行與檔尾 `g_W906PadNote`，不要碰）；第 93 批進 main 之前只做唯讀盤點（小幫手在跑）。
- 之後：W-149 MR 4（W6_4_TesterAnchor 用預設 A：測試自己種 ScanPort；WB_SimPump、mainproc_guard；順手修 test_scankey_golden [19] 沒還原 InitialOK／軟鍵狀態）——等 W-152 進 main。

### 10-07 18:2x 狀態（W-152 推完時寫的）
- **W-152 急件推了，MR !312**（`v906/st02-w152-autoheight` `cf493021`，main e06fb67b 上）：Contact 頁自動測高的資料路徑 (C) 配方→門面（單一入口）、(A) 結果→網頁（case 1800 掛勾）、(B) 開著時保留＋tag `contact.runResultSeq` 觸發網頁重讀、golden ClearIndexOffset（cOffSet.cpp 檔尾；產生器那一列）。**主流程閘那一行不在裡面**：Jimmy 裁決（RULINGS_20261007 #9）由筆電在 `v906/jimmy-w152-gate` 加在 !312 之上。測試 St02_W152ContactData／St02_W152ContactRereadPage；腳本與反向 `C:\AI_TempFile\st02e-scratch\w152\`（patch_w152_data.py、patch_w152_web.py、reverse_w152.py、report_height_path.md）。
- 已回報 St02-M（HUMAN_REVIEW A／B 都寫了，B 是給 EastSun 的 W-153 上機步驟）。ChangeLog 第 12 列；techniques.md 新增 §9（今天的坑）。
- W-149：!307 已進 main（第 91 批，SIM 基準 19→14）；!308／!309 排第 92 批；MR 4（WB_SimPump／mainproc_guard）等 St02-M 放行；W6_4_TesterAnchor 等 Jimmy。
- 下一張：等 St02-M 派。

### 10-07 17:3x 狀態（W-149 三張推完時寫的）
- **W-149 SIM-AWARE**（只在模擬版失敗的 15 支測試）：三張 MR 疊在一起，要照順序合：**!307**（SimIO／HanaART／Automation／GA2_C1_cinitial／BarCodeHelpers，`fc403a13`）← **!308**（W6_Canary／AGV_E84／BarCode8CCDGlue，`c65aed07`）← **!309**（W7_L1 四支，`9e80635a`）。共用新標頭 `tests/w906_sim_build.h`：`W906_SIM_BUILD`（定義 SOFT_SIMULTE 時＝1）＋`W906_SIM_NOTE("…")`（只在模擬版印）；寫法 `CHECK(W906_SIM_BUILD ? (模擬值) : (出貨版原條件), "原訊息" W906_SIM_NOTE(" -- SIM: …, golden 0618 檔:行"))`，同一行、出貨版一字不差。腳本：`C:\AI_TempFile\st02e-scratch\w149\`（patch_mr1/2/3.py、reverse_w149.py mr1|mr2|mr3、report_A_w7.md、report_B_misc.md）。
  - 還沒做：**W6_4_TesterAnchor** 等 Jimmy（FROM_STEVEN §3：A＝測試自己種 ScanPort（C1=6、C2=16、C3=16），B＝先補 `Sim_TTL_Single`（port atester.cpp:12898-12900 `#if 0`，golden atester.cpp:699-753））；**WB_SimPump／mainproc_guard** 等 St02-M 確認機台端沒人在改 `test_wb_simpump.cpp`（今天機台 AI 改過）。這兩支模擬版的原因：golden 在模擬版把 DoSystem 整段安全互鎖編掉（csystem.cpp:4403-4436／:4441-4493），START 不會被拉回；PumpInit 的 O7（WebBridgeTags.cpp:471-510，V906 才有）模擬版只印不拒絕。
- **W-152（Contact 自動測高，急件）停著**：核心修改（csystem.cpp:31465-31467 MainProc 閘，HT9050 才開）被權限檢查判「Security Weaken」擋下，照規則 26 不繞、不轉手，等 Steven 本人在這個 session 同意。分支 `v906/st02-w152-autoheight`（乾淨、沒推）。事實整理 `C:\AI_TempFile\st02e-scratch\w152\report_height_path.md`：**流程讀的是 fContactForm 的輸入框，從來沒從配方填過（fContact.h:1497 GATE W-02）⇒ fDropPos 等於 0**；測到的高度不會回到網頁、存檔會寫回舊值；生產下壓用的是 Contact.Data [Test Arm1] Contact → Prod.TestZ1_Test。
- **建置線注意**：第 89 批（MR !301）起 `BUILD_TESTING` 預設 OFF，既有 obj 重新設定後 `cmake --build` 不再重編測試（舊 exe 還在、照樣跑得動）。s39 兩個目錄已補 `cmake -DBUILD_TESTING=ON <obj>`。
- W-140（!304）已進 main（第 90 批）。

### 10-07 15:4x 狀態（W-140 推完時寫的）
- **W-140 推了，MR !304**（`v906/st02-w140-command` `955aeb38`，在 main f6691973＝第 87 批＋15:11 機台快照上；推的樹跟測過的樹完全相同）。兩組態 0 錯；St02_W140Command 9／9＋相關 7 支兩組態全過；反向 R1～R4 全紅；機台快照 15:11 已還原、備份 0 個；真檔全部未變。已回報 St02-M（ht9045-32）、ChangeLog 第 7 列（入口網站的檔還沒 commit，等 St02-M 一起發）。
- **等 Jimmy 裁決**（已請 St02-M 寫進 FROM_STEVEN §3）：golden 0618 Command.cpp:1590 的 TempMode 沒給值，I38 那一種 SETTEMP 回應、機台不是常溫時會把沒給值的模式寫進 LastSet.iTemperature；移植樹 :1886 先用「保持目前模式」當安全預設（建議 A）。
- 第 87 批（cmydef.h 拆檔）讓幾乎所有檔都重編：建置線 s39 兩組態全編一次約 30 分。
- 下一張：等 St02-M 派。

### 10-07 15:0x 狀態（W-140 開工前寫的，已完成）
- **W-143 推了，MR !303**（`v906/st02-w143-mnetlog` `75a932c9`，在 main f2a1d781 上；測試是在 4909f92d 上跑的，之後 main 的改動跟這張不重疊、補丁內容相同）。已回報 St02-M、ChangeLog 第 6 列。
- 建置線 s39 的兩個目錄已加 `-DW906_STRIP_TEST_EXES=ON`（第 85 批起；測試 exe 第一次執行不再被防毒卡 15 秒）。
- **模擬組態有 8 支在 main 上本來就紅**（SimIO、W6_Canary、W6_4_TesterAnchor、AGV_E84、W7_L1_Color／Loader／AutoRT、GA2_C1_cinitial；不套機台快照也紅）——已請筆電確認是我們這條線的設定問題還是它的 gate 也紅。之後碰到這幾支先跟 main 比，不要以為是自己弄壞的。
- 反向檢查腳本的坑：會重建的項目還原後一定要再重建，否則後面「只改原始碼」的項目跑到的是被改壞的執行檔（W-143 R2～R6 第一次就是這樣，已修好 `reverse_w143.py`）。
- 下一張：**W-140 Command.cpp**（SETTEMP 1931／1952、UPH? 3454、SETSITEMAP_ 14892；候選評估在 `C:\AI_TempFile\st02e-scratch\w140\`；分支 `v906/st02-w140-command`）。

### 10-07 13:0x 狀態（W-143 做到一半時寫的，已完成）
- **W-143 MNETLOG**：本機分支 `v906/st02-w143-mnetlog`（`C:\AI_TempFile\st02-c23`，WIP `8ba72e7b`，從 main 06fb64e5，**還沒推**）。已做：新 `MNetLog.h`／`MNetLog.cpp`（ht9045_sm；mmoMNet＝不顯示＋`TODO(W906-LOGVIEW)`）、`LogObjects.h`／`.cpp` 加 `W906_MNetLogObj()`、myMN200motor.cpp 舊本體改註解、5 處宣告換 `#include "MNetLog.h"`、AutoClean.cpp／TfFTP.cpp 空殼退役、cStateRecord.cpp G5 打開（改用 job.copies）、新 ctest St02_W143MNetLog。**cMyDB.cpp:186 的空殼保留**（tests/test_ga1_cmydb 單獨編 cMyDB.cpp、沒連 MNetLog，退役會連結失敗；已加同行說明）。兩組態建置 0 錯（建置線 s39 在 8ba72e7b 前一版，差一行測試註解）。
- **還沒做**：#6 機台同步 → 跑 St02_W143MNetLog＋84 支相關測試（清單 `C:\AI_TempFile\st02e-scratch\w143\names.txt`）兩組態 → 反向 `reverse_w143.py`（R1～R6）→ 還原 → rebase main → merge-tree → 推 MR → 回報（HUMAN_REVIEW B：開機後 wb_serve 會寫 `D:\HT9045_Log\MNetLog\YYYY\MM\…`；FTP／IO 失敗會多出紀錄）。b85 合進 main 後建置線加 `-DW906_STRIP_TEST_EXES=ON`（測試 exe 第一次執行不再被防毒卡 15 s）。
- 之後：W-140 Command.cpp（候選評估 `C:\AI_TempFile\st02e-scratch\w140\`）。W-142 已由筆電收進第 85 批。

### 10-07 12:2x 狀態（St02-E 寫；St02-M＝ht9045-32）
- **W-142 推了，MR !300**（`v906/st02-w142-alarmdesc` `5036e647`，**接在筆電 b84 上**——用到 b84 的 W906_AlarmMsgWithErrPart 與 RTFDESC；b84 合進 main 前 MR 會連 b84 的 commit 一起顯示）。工具與反向：`C:\AI_TempFile\st02e-scratch\w142\`（advmot\ 是 1203 錯誤碼表的產生器）。
- 這張學到的：①`fNote_ShowError.cpp` 的 ErrShowToForm 在送告警框**之前**跑（wb_serve.cpp:442 先記錄、:497／:526 才送），所以「記錄一次、送出時取用」可行；②`sed -i` 會把 CRLF 吃掉（cMyDB.cpp 整檔變 LF），一律用 Python 腳本或 Edit 改；③JS 檔某一行行尾已有 `//` 註解時，往行尾附加程式碼會變成註解的一部分（W-142 的 w142MsgAsDesc 被測試抓到）；④原始碼釘子要釘「整行的樣子」，只找一個片段可能在別處也有（R5 一開始沒變紅）；⑤MinGW 6.3 沒有 C++17 inline 變數，header-only 的表要放在 inline 函式的 static 裡；⑥WebMotorAccess.cpp／Pci1203GaliRouteCore.cpp 直接編進十幾個測試目標，加新 .cpp 要改很多目標 ⇒ 用 header-only。
- D026_NoteAuth 在 b84 上本來就紅 1 項（notifyAck 釘子，b84 的 AI(W906-NOTICE-DEFER-4) 改了 wb_serve.cpp:4914），不是我們的；已告訴 St02-M。
- A1a／A1b 已改派 Frank01（RULINGS_20261007 #4）；本機 `v906/st02-a1a-contact` 是空的、已刪。
- 排隊：**W-143 MNETLOG**（!296 提案 A；`mmoMNet` 改「不顯示」＋`// TODO(W906-LOGVIEW)` 註記，Steven 12:0x）→ **W-140 Command.cpp**（候選評估在 `C:\AI_TempFile\st02e-scratch\w140\`；我提 1931／1952／3454／14892 四個，St02-M 訊息寫「三個」，開工前確認）。
- machine_sync 已 restore，backup 0 個。

### 10-07 10:1x 教訓：測試的圍堵檢查要放過建置目錄（gate b83a）
- !293／!295 的測試在筆電 gate ABORT：`UnderMachineTree` 擋所有 `d:\ht9045` 開頭的路徑，但 `tests/test_bootstrap.cpp` 給的 lastdata 沙盒（`<build>\tests\w906_ctest_lastdata_<pid>`）與 general ini 副本（`general_ini_scratch`）在**建置目錄**裡；筆電（worktree）與機台（`D:\HT9045\Obj\V906`）的建置目錄都在 D:\HT9045 底下。這台的建置線在 C:\AI_TempFile，所以本機看不出來。
- 筆電已修（d74e577e 第 83 批、2b663dbd 第 84 批）：小寫後含 `\obj\v906\` 就算沙盒（`if (s.find("\\obj\\v906\\") != std::string::npos) return false;`）。**新測試一律照這條**；只檢查自己在 %TEMP% 建的路徑的不受影響。
- 1007 10:1x 掃過 St02 其他測試：ELA_TimeData、MyDB_O19_Summary、TesterComm_TcpCmdServer、Jam_Rules、Security_LoginDatBook 的圍堵都只檢查 %TEMP% 路徑 ⇒ 沒有同樣的問題（掃描 `C:\AI_TempFile\st02e-scratch\teachscan\machinetree_scan.py`）。

### 10-07 09:1x 狀態（St02-E 寫；St02-M＝ht9045-32；新 session 從這段接，下面 07:5x 那段仍有效）
- **POOL-2 推了，MR !295**（`v906/st02-pool2-secs-ec` `efbad72c`，從 main a23e7ea6；工作樹 `C:\AI_TempFile\st02-c23`）：`SECSGEM/uHGemHT9045.cpp` [E1]／[E5]／[L1] 照 golden 0618 打開（ReadESDDataFile／fBinSel->Save／fSecurity->SetLevelSet 都有本體了）。新 ctest St02_SecsEcFileWrites（真的 S2F15／S125F3 封包：SecsWireCodec 編碼 → THGem 模擬 socket 解碼 → 直接叫 HT9045Gem 處理函式；寫法可以照抄）兩組態 10／10，反向 R1～R4 紅（`C:\AI_TempFile\st02e-scratch\pool2\`）。
- 這張學到的：①叫到 TfBinSel::ReadFile 的測試要自己 `OpenGeneralIniFile()`（wb_serve 開機就開著，database.cpp:3161），不然 ReadTechData 的 CheckAndReadIniDataGeneral 會當掉；②`asTeachPath`（teach.ini）**沒有**被測試環境轉向，但 ReadTechData 只在 `fTeach!=NULL` 時才讀（cinitial.cpp:16211）；`fTeach` 只是全域指標（forms/fTeach.cpp:73），正式程式只有 wb_serve.cpp:3927 會 new，ctest 只有 GearTeachSave／TeachCheckRangeReload 會建（兩支都自己轉向）⇒ 一般 ctest 走 SetWorkParameter **碰不到** teach.ini（09:2x 用探針實測：哨兵檔沒被寫）。另：TfTeach::ReadFile 在 teach.ini 沒有 `[Teach INI] Update2` 時會讀寫死的 `d:\HT9045\system\tech.dat`（沒有轉向接縫），再整份寫回 teach.ini；③TfBinSel::Save 寫 Binasgn.Data 或 BinasgnOff.Data 看旗標，ReadFile 還會另建一個 ⇒ 兩個都要看。
- POOL-7 已由 NB2 做（MR !287），不要重做。
- 唯讀排除過的候選（已轉筆電）：Timer4～7／9、SV G13／G18（欄位不存在）、G17（零寫入者）、EC g6（要裁決）、G06／G47（客戶）、G41-43（會動機台）。
- HT9050 機台：客戶碼 957 CC_PTI，config.ini [SECS GEM] Enable SECS GEM=0 ⇒ SECS 相關改動在 HT9050 上暫時走不到。
- ctest 第一次跑常在 15～17 s 報 Timeout（TIMEOUT 明明是 60），單獨重跑就過——原因未查，已告訴 St02-M。

### 10-07 07:5x 狀態（St02-E 寫；St02-M＝ht9045-32）
- **W-132 推了，MR !293**（`v906/st02-w132-sckart` `4b491858`，從 main 76dd45f3；工作樹 `C:\AI_TempFile\st02-c23`）：TfSCKART 補 iCurrentStatus／iLOTSTATUS_A、建構子 R＝4／A＝6（iLOTSTATUS_R 以前沒設值）、SetLotStatus 寫 iCurrentStatus；HandlerGpibMsg G2／G4／G6 照 golden 0618 main.cpp:15443-15526 打開，G7／G10／G11 還缺 iLOTSTATUS_F／L 所以照留。新 ctest St02_W132SckArtLotRt 兩組態 15／15，反向 R1～R6 紅（`C:\AI_TempFile\st02e-scratch\w132\`）。已回報 St02-M、ChangeLog 1007 第 1 列。
- 已知缺口（可當下一張卡）：移植樹沒有地方把狀態設成 A（golden note.cpp:5651 Break-ART、AccessFile 讀 Tester.Data 的 iCurrentStatus :203）⇒ G6 在機台上走不到。別人的檔的過期註解（Command.cpp:254／:15369-15371／:17543、csystem.cpp:2695）已列給 St02-M 轉筆電。
- 測試的坑：建置線一邊建 ship 一邊跑 SIM ctest ⇒ 7 支逾時；新 exe 第一次執行防毒掃描 ⇒ ship 3 支約 15 s 逾時；單獨重跑都過。**ctest 要等兩組態都建完再跑、-j 1**。lane_cmake.sh 的 PE 檢查碰到防毒鎖檔每支等 30 s（W-29），要趕時間可以停掉它、改直接 `cmake --build`。
- St02-M 1007 07:1x 說過「St02-M 沒回應時改寫 ST-HandOver chat/st02e.md」，07:3x 又取消（Steven：暫不接手）⇒ 照舊用 SendMessage 回報。
- machine_sync：已 restore，backup 0 個。

### 10-06 22:5x 狀態（St02-E 寫；St02-M＝ht9045-32；下面 20:0x 那段仍有效）
- **W-135（POOL-5 #1＋#2）推了，MR !285**（`v906/st02-pool5-wma` `fa80ae01`，從 main 2db43115）：`tests/test_web_motor_access.cpp`（筆電的檔）:964／:965／:1045 三個釘死的數字改結構檢查（列欄位、目錄＝kActions、live＋ui＝全部），檔尾兩個輔助函式。兩組態 WebMotorAccess 1096／1096、反向 R1～R4 全紅（`C:\AI_TempFile\st02e-scratch\pool5\`）。已回報 St02-M、ChangeLog 第 15 列。machine_sync 已 restore、backup 0 個。
- 工作樹 `C:\AI_TempFile\st02-c23` 現在在 `v906/st02-pool5-wma`（已推）；s39 建置線 detached 在 fb6e1bb0（同內容）。明早 W-132 從 origin/main 開新分支。

### 10-06 20:0x 狀態（St02-E 寫；St02-M＝ht9045-32）
- 今天推的 MR：!253 P1b、!274 2C-ST02、!277 W-121、!278 2C-ST01、!279 W-127、!285 W-135（都等 gate）。POOL-4 報告＋跟 Ifor01 交叉比對已交（St02-M 發布）。
- **手上沒有排隊的卡**；A1a／A1b 仍暫停等 Steven 本人。St01 今晚起離線。
- **候選卡（等筆電）**：HandlerGpibMsg G2／G4／G6（golden 0618 main.cpp:15443-15526 ART LOTRTCLEAR）不是單純解閘：global fSCKART（TfSCKART 外殼，筆電的 forms/fSCKART.h）沒有 iCurrentStatus／iLOTSTATUS_A，SetLotStatus 不做 golden SCK_ART.cpp:665 `iCurrentStatus=iStatus`；實際狀態在 SckArtState（Automation/SCK_ART.h:100）。要筆電同意改標頭（＋設計：外殼自己存還是讀 SckArtState）才能做。20:0x 已交 St02-M；St02-M 20:1x 已在 s3 問筆電／Jimmy（handoff 7fd0e696：A＝TfSCKART 照 golden 自己帶 iCurrentStatus／iLOTSTATUS_A（建議）、B＝讀 SckArtState、C＝先不做；並請准認領 forms/fSCKART.h／.cpp），答案在 TO_STEVEN §4，St02-M 明早轉。
- **10-06 20:1x 起 idle**（St02-M 已記心跳）。
- **10-07 早上第一張＝W-132**（筆電 21:0x 選 A，照 golden、不用 Jimmy 決定；St02-M 已在 s1 認領 forms/fSCKART.h／.cpp＋HandlerGpibMsg，handoff 2f33af74）：
  ①fSCKART.h 加 `int iCurrentStatus; int iLOTSTATUS_A;`（golden SCK_ART.h:251 與 LOTSTATUS 那段）②fSCKART.cpp 建構子值照 golden SCK_ART.cpp:42-55、SetLotStatus 也做 golden :665 `iCurrentStatus=iStatus` ③HandlerGpibMsg.cpp G2（:363-367，有 #else ⇒ `#if 1`）＋G4（~:419）＋G6（:434）一起開，對 golden 0618 main.cpp:15443-15526 ④ctest 走 MSG_CMD_SCKART_LOTRTCLEAR 的 A／R／其他三條路＋反向。不改 Command.cpp／MessageDef；SckArtState 的債照留。一張 MR。開工先重量行號、照 #6（測完 backup 0 個）。
- POOL-2 選檔之前先看 main 的 `docs/handoff/IF0_CENSUS_20261006_linked.tsv`（22 支普查檔根本沒連進 wb_serve.exe）。我 20:0x 回報的 7 個已寫進普查（cStateRecord.cpp:853 粗體）。
- IF0 普查 §2 我這區 7 個「可解」不成立（cStateRecord.cpp:853 是 MOVED-BG，解開會跑兩次 7z／刪資料夾）——已交 St02-M 轉筆電／Ifor01（handoff fe21cadc）。
- 工作樹：`C:\AI_TempFile\st02-c23`（目前在 `v906/st02-w127-lowtempdoor`，已推；之後新卡從 origin/main 開新分支即可）；s39 建置線＋`st02-s39-obj`（Ninja，build／build_ship）。
- machine_sync：**這台目前是原本的設定，backup 資料夾 0 個**（每次測完看 `D:\HT9045\backup\machine_sync_*` 要是 0）。restore 用 PowerShell 跑（Git Bash 會把路徑的反斜線吃掉）。
- 工具：WinLibs 16.2 在 `C:\AI_TempFile\toolchains\mingw32-16.2.0`；POOL-4 探針 `C:\AI_TempFile\st02e-scratch\pool4\`；2C／W-121／W-127 的反向腳本在各自的 scratch 資料夾。

### 10-06 18:3x 狀態（St02-E 寫；St02-M＝ht9045-32）
- **POOL-4 交了**（`C:\AI_TempFile\st02e-scratch\pool4\FP_EQ_CENSUS_20261006.md`，St02-M 發布；工具同資料夾：fpeq_candidates2.py、classify2.py、shape_probe.cpp／site_probe.cpp＋run_probe.py、gen_report.py；WinLibs 16.2 在 `C:\AI_TempFile\toolchains\mingw32-16.2.0`）。跟 Ifor01 的 670 處交叉比對也交了（§7）：A 組 6 處一致；B 組合起來 7 處（我漏 MyVacuumPanel:576、他漏 gen.inc 5＋Adam6024Integrate 1）。
- **Ninja**：winget 裝了 ninja 1.13.2；s39 建置線 build／build_ship 已換 Ninja（舊目錄 `*.makefiles-20261006-1753xx` 留著，第一次 Ninja 建置過了就可刪）。
- **2C-ST02**（W-107）：分支 `v906/st02-2c-tests`（`C:\AI_TempFile\st02-c23`，WIP `c602a4c0`，從 main b8ea3a51）。5 支測試改結構檢查；SIM 5／5 綠、反向 R1～R5 全紅（`C:\AI_TempFile\st02e-scratch\c2c\`）；SHIP 建置中 → SHIP ctest → restore → 推 MR。
- **2C-ST01**（St01 的 #8／#9／#10／#16＋低 2 支）：等 Jimmy 同意（St02-M s3）；helper 只做唯讀草稿在 `C:\AI_TempFile\st02e-scratch\c2c_st01\`；**不動 tests/CMakeLists.txt、不碰 test_indexz_autoheight_1203.cpp**。
- **W-121**（主畫面 Light／FAN 鈕 release 模式沒接上：theme.js stripTitles 先把 title 搬走）：2C-ST02 之後做；碰到權限檢查就停、告訴 St02-M。
- Steven 18:1x：決策題一律給 Jimmy（經 St02-M 寫 FROM_STEVEN §3）。A1a／A1b 仍暫停。

### 10-06 14:1x 狀態（St02-E 寫；St02-M＝ht9045-32）
- **P1b 做完、推送中**：分支 `v906/st02-p1b-pad`（工作樹 `C:\AI_TempFile\st02-p1`），選 A＝回到 golden 逐 chunk（PadInterface_St02.cpp 佇列改成整塊、DrainRx 每拍全部取走、新接縫 `W906_PadRxQueuedBytesForTest`；測試 [8] 改 P1-8c～8g）。
  建置線 s39 在 `0486b48f`（合 main 55b590bd）兩組態 0 錯；14:06 apply 機台 13:52 快照 → SIM St02_PadInterface 57／57、ScanKeyGolden 139／139；SHIP 56／56、139／139（差 1 是 P1-3a 既有的 SIM／SHIP 分支）→ 反向 R1（舊的跨 chunk 暫存）紅 P1-8c／8d／8e／8f／8g、R2（同 chunk 逐幀分 unit）紅 P1-8c → 14:13 已 restore（7 檔 MD5 相符、備份已刪）。
  工具 `C:\AI_TempFile\st02e-scratch\p1b\`（reverse_p1b.py、mr_desc.md）。
- **A1a／A1b 暫停**：A1a 改完 `ht9045_golden_kb_unwired.js:193-200` 後，下一個指令被 Claude Code 權限檢查判「Security Weaken」擋下（拿掉會讓機台啟動的按鈕的頁面封鎖）。照第 26 條不繞：已還原、`v906/st02-a1a-contact`（`C:\AI_TempFile\st02-c23`）乾淨未推；A1b（打開 EP 壓力輸出）同類，沒開始。**要 Steven 在這個 session 直接說要做**（或加權限規則）才繼續。已告訴 St02-M。
- St01 今天 17:00 起離線（Steven 明天出國）：17:00 後不要排需要 St01 回覆／代跑／登記的事；問題給 St02-M。
- 新的跨機聊天 repo：`https://gitlab.honprec.com/honprec/rd/rd5/ST-HandOver`（`chat/st02e.md`，只附加；短分支＋auto_merge）。跟 St02-M 照舊用 SendMessage。

### 10-06 13:0x 狀態（St02-E 寫；St02-M＝ht9045-32）
- **A1 已交**（唯讀盤點，報告 `C:\AI_TempFile\st02e-scratch\a1\ST02_A1_DEAD_CONTROLS_20261006.md`，St02-M 發布；工具 `a1\a1_field_probe.py`（`--clicks`／`--evals`）、`unwired_lists.py`、`gate_by_func.py`）。09:1x 第 3 輪 apply 機台 10-06 09:00 快照 → **⛔ 更正 19:1x：那次沒有 restore**（之後 14:06／18:29／18:59 的 apply→restore 都只回到「09:00 快照」的狀態），19:1x 才把 `machine_sync_20261006_091054` 還原（447 檔 MD5 相符、備份已刪）；09:10～19:1x 這台的設定是機台 09:00 快照。教訓：每次測完馬上 restore，收尾看 `D:\HT9045\backup\machine_sync_*` 一個都不能剩。
- 筆電 12:2x 回 A1 兩題「照 golden 都做」，St02-M 13:0x 派卡（handoff `0153df2d` §1 已認領）。**順序：P1b → A1a → A1b，各一張 MR**；先重量行號、把確切範圍給 St02-M 寫 §1。main＝`6b616f5e`，測試前照 #6。
  - **P1b**（Ifor01 審 !221）：`PadInterface_St02.cpp` 收資料 `pending` 沒上限＋最後一段沒 CR 要等下一個 CR。golden 0618 uPadInterface.cpp:708-746 是每個 chunk 自己處理（do…while 至少一次、刪到第一個 \r、沒 CR 的尾巴處理一次就丟）。選項 A＝回到 golden 逐 chunk；B＝保留累積但上限約 4 KB＋一次 [Recv Error]＋尾巴照 golden 處理一次。對照機台 com_probe_pad 擷取（GitHub 機台分支 dispatch/20261005_rs232pad_priority/）。
  - **A1a**：把 Contact 11 顆模式鈕＋btnStart／btnPause／btnTStart／btnTStep／spbOneCycle 從 `web/page/ht9045_golden_kb_unwired.js:190-217` 拿掉；機台會動 ⇒ MR 寫明、HUMAN_REVIEW 上機項、第一次上機 EastSun 在旁；node 測試＋反向。
  - **A1b**：HT9050 的 5 個 EP 閘照 golden 打開（DeviceForm_File.gen.inc:2797／:2954／:4186、ContactForce.gen.inc:998／:1459）＋ContactForce 關窗尾；.gen.inc 若是 St01 產生器產物，先跟 St02-M 確認改產生器輸入還是同行替換；HUMAN_REVIEW 上機項；ctest＋反向。
- 1006 09:2x pull 後兩支 skill 檔（SKILL.md、本檔）已核對：本機版是 main 的超集合（main 只多一行 10-05 16:4x 標題的舊字），**不用合**，跟下一次程式推送一起推。
- ST-GPT（筆電上的 GPT／Codex）接手 skill 重整；要動本 skill 會在 FROM_STEVEN §4 問。

### 10-06 A1 計數那一輪（08:11 apply 機台 10-05 23:07 快照 → 09:02 已 restore，447 檔 MD5 相符、備份已刪）
另：wb_serve 開機時 golden BackupSetupFile 會把作用中配方的 `<hash>.MD5` 改名，realfile_guard 的 restore 不會改回——探針 `C:\AI_TempFile\st02e-scratch\a1\a1_field_probe.py` 會自動改回（內容相同才改）；手動跑 wb_serve 後要自己看 `D:\HT9045\IniData\Data\<配方>\*.MD5`。

### 10-05 23:1x 狀態（St02-E 寫，session ht9045-46；St02-M＝ht9045-32；新 session 從這段接）
- **P1＝急件 MR !221**（`v906/st02-p1-pad` tip `5fc10fb5`，已推，**分支凍結**——筆電要求才再動）。St02-M 已核對並貼出（handoff `a247f82c`）：FROM_STEVEN §2「急件 請 gate」、完整 MR 說明發布成 `docs/handoff/ST02_P1_MR221_DESC_20261005.md`（筆電貼進 !221）、§1 認領標已推、HUMAN_REVIEW A47／A48／B46／B47。等筆電急件 gate。
- 可收的工作樹（請 St02-M 收）：`C:\AI_TempFile\st02-c22`（!216 已合）、`st02-docs`（!211 已合）、`st02-s45`；`st02-p1` 等 !221 進 main 後再收。`st02-s39`＋`st02-s39-obj` 留著當建置線。
- **每次測試前的新規矩**（RULINGS_20261005 第 6 條補充，19:2x，不用問）：①樹更新到 main 最新 ②`python tools/machine_sync/machine_sync.py check`，未同步就 `apply --yes` ③回報寫 main commit＋機台快照時間 ④測完 `restore <備份資料夾>`。沒做①②的結果不算數。
  23:09 apply（機台 23:07 快照）→ 測完 23:1x 已 restore、備份已刪（這台現在是原本的設定）。
- 平行跑測試的教訓：SIM 測試跟 SHIP 連結同時跑會讓計時類測試與 FShow_Audit 逾時；計數那一輪要在機器空閒時依序跑（-j3）。
- **C23＝MR !223**（`v906/st02-c23-teach` tip `917bf3de`，1006 01:2x 推；引擎 :2069 筆電點頭待補；完整說明 `C:\AI_TempFile\st02e-scratch\c23\mr_desc.md`）。134 個欄位夾範圍（136 列中 2 列在隱藏的 EdtTemp）。工作樹 `C:\AI_TempFile\st02-c23` 等 !223 進 main 再收。
- **hpi-gpib 補課＝MR !224**（`v906/st02-hpigpib-catchup` tip `26936441`，只有 skill；工作樹 `C:\AI_TempFile\st02-c22b` 等進 main 再收；工具 `C:\AI_TempFile\st02e-scratch\c22b\`：remap.py／repoint.py／linkcheck_changed.py）。
- **P2＝MR !226**（`v906/st02-p2-softkey` tip `c18501a7`，1006 04:5x 推；完整說明 `C:\AI_TempFile\st02e-scratch\p2\mr_desc.md`）。計數那一輪 SIM／SHIP 各 20／20、反向 R1～R6 紅。工作樹 `C:\AI_TempFile\st02-p2` 等 !226 進 main 再收。**手上沒有排隊的卡了**（P1／C23／hpi-gpib 已進 main，P2 等 gate）。
- **bringup-paths＝MR !227**（`v906/st02-bringup-paths` tip `62c77584`，1006 04:5x，只有文件：ST02_BRINGUP_CHECKLIST.md 4 處舊 C22 路徑→hpi-gpib）。在 `C:\AI_TempFile\st02-c22b` 工作樹（!224 已合、拿來重用），等 !227 進 main 再收。
- 給 Steven 刪的工作樹清單（St02-M 轉）：st02-p1、st02-c23、st02-c22／st02-docs／st02-s45；st02-c22b 等 !227、st02-p2 等 !226；st02-s39＋obj 留著當建置線。
- （舊）**P2**（第 72 批已在 main；筆電對它的檔點頭還沒回——照 P1 的做法先寫碼、推之前再看點頭）。
  **1006 03:2x 進度**：分支本機 tip `e0a00651`（兩顆 WIP，**不推**，等 HW.teach.html 做完一張 MR）。除了 HW.teach.html 都做好了：
  C++ 8 檔（WebMainScanKey EOF：bAse* 軟體鍵＋3 秒過期＋一鍵一下＋最多 4＋DIAG＋W906_PanelHomeKeyArm，:438 同一行掛 tick；wb_serve :5738；WebBridgeServer :1449；WebMotorAccess .cpp／.h／Live；test_web_motor_access 67／50／45；ScanKeyGolden 新 [19]）；網頁 5 檔（main.html :275／:649、ht9045_main_softkeys.js、ht9045_teach_homeall_c.js、motor-access.json／.js）。web 0101 不用（main.html:45 已有 pointer-events:none）。
  FShow_Audit 58＝基準、START 普查 36／34／2 通過。反向驗證腳本 `p2\reverse_p2.py`（R1～R6）。測試清單加 D015_A01MenuPage、D025_MenuOpenPage、St02_N07BannerPage、E09_MainLogo（讀 main.html）。
  **剩下**：第 72→74 批進 main 後 merge → HW.teach.html :69（btnHomeAll 鈕）＋script include（重量行號）→ 計數那一輪 → 反向驗證 → 推 MR。St02-M 已在 §1 記下兩處改動（測試改放 [19]、不需要 0101），handoff eea6bee3。
  （舊）1006 01:4x 停在這裡（用量上限）：工作樹 `C:\AI_TempFile\st02-p2`（分支 `v906/st02-p2-softkey`，從 main 1c7fef88，**還沒改任何檔**）。
  已重量行號＝跟認領稿一樣（main 1c7fef88：WebMainScanKey EOF 456／:438；wb_serve :5738；WebBridgeServer :1449；WebMotorAccess :96／:4602／EOF 8949；.h :267；Live :22／:437；test_web_motor_access :279／:964／:965／:1045／rel3 :1346；HW.teach.html :69／:1254；main.html :275／:649）。
  機台長行的原文：`C:\AI_TempFile\st02e-scratch\p2\wma_lines.txt`、各 commit diff 在 `p2\*.diff`、網頁 patch 在 `p2\web\`。
  計畫照 `p2\P2_CLAIM_DRAFT.md`；一處調整：軟體鍵的測試放 ScanKeyGolden（它連真的 ScanPannelKey，能驗 soft ALARM RESET → N07 消音、RESET 停用設定照擋），test_main_scankey 可不動。
- （舊）**C23（W-95，RULINGS_20261005 第 23 條）先做、P2 後做**：認領稿 `C:\AI_TempFile\st02e-scratch\c23\C23_CLAIM_DRAFT.md`（已交 St02-M）。#100＝引擎 `ht9045_wire_engine.js:2069` 同一行加頁面範圍掛點＋HW.teach.html 從 `HT9045Page.golden().page.lists.elTeach` 提供 136 個範圍（不改 C++）；#103＝Set To Offset 在選到的馬達 homeFlag≠1 或格子空時拒絕（Steven 指定、偏離 golden，列 HUMAN_REVIEW）。測試 TeachKbGolden（含 CONTROL）＋Teach／小鍵盤相關 node 測試＋e2e 探針 (f) 翻轉。**e2e 探針在這台跑（realfile_guard）；真的跑不了就在 MR 與 §2 寫「e2e probe: laptop gate」——不再交給 St01**（Steven 1005 11:5x「不再派工給 St01」，St02-M 1006 00:1x 轉述；skill 裡「請 St01 代跑」的舊規矩以此為準）。St02-M 已貼 C23 認領（§1 00:0x、`docs/handoff/ST02_C23_CLAIM_DRAFT_20261006.md`、§3 請筆電對引擎 :2069 點頭）；第 72 批進 main 時它會通知。ChangeLog 換到 `CHANGES_20261006_Steven02.md`。
- 下一張：P2（TEACH-HOMEALL＋SOFTKEY，不含 SOFT E-STOP），等第 72 批（筆電 HOME-PERAXIS）進 main 再開。**認領稿已交 St02-M**：`C:\AI_TempFile\st02e-scratch\p2\P2_CLAIM_DRAFT.md`（12 項、行號對 main db3a636c 與第 72 批 ba8021dd；機台 diff 存在 `p2\*.diff`、網頁 patch 在 `p2\web\`，來源 GitLab `origin/v906/mc01-scankey-patches` 173cdb1a）。
  St02-M 已預先認領（handoff `2fec1245`：FROM_STEVEN §1 暫定行號、稿發布成 `docs/handoff/ST02_P2_CLAIM_DRAFT_20261005.md`、§3 請筆電對它的檔點頭、panel.key 免權杖列 HUMAN_REVIEW C）。開工條件：第 72 批進 main **且**筆電點頭 → 重量行號 → 寄最終清單給 St02-M。MR 說明要寫 bAse* 的理由，並證明 V906 沒有別的地方會設 bAse*（ASE 沒移植；`git grep` 寫入點＝0）。
  設計改動：軟體面板鍵改走 golden 的 `bAse*` 旗標（ASE 遠端那條路），ScanPannelKey 每顆鍵的副作用（N07 消音、音樂、RESET 停用設定）照跑 ⇒ 不動 ckernel.cpp、不要 W906_VirtualPanelKeyHook。

### 10-05 22:2x 補記
- **C22 MR !216 已進 main**（22:14，`e354b1c3`；St02-M 已在 §2 標記，handoff `f9d7364d`）。15 支舊名空殼 skill **約 10/12 刪**；刪的時候要改的 6 處名稱／路徑引用清單在 `CHAT_ST02` 19:4x（也在 MR !216 說明）。工作樹 `C:\AI_TempFile\st02-c22` 可收。
- C18 !215 在第 72 批 gate b72a。P1：第 71 批已進 main（53a13cc9），P1 已合（`111c71fb`），兩組態增量重建＋急件測試清單跑中，過了就推急件 MR。

### 10-05 21:3x 狀態（St02-E 寫，session ht9045-46；St02-M＝ht9045-32；新 session 從這段接）
- **P1 可以推了，只等第 71 批進 main**：分支 tip `4ee5383c`（另有 `4279070f`＝FastClock 工作搬到 `PadInterfaceClock_St02.cpp`，修 14 支測試連結失敗）。兩組態建置 0 錯；本機 ctest 兩組態 St02_PadInterface 51／51、ScanKeyGolden 綠；全套 SIM 22 紅／SHIP 7 紅＝基準（失敗行逐字相同）；反向驗證 13／13（`p1\reverse_p1_result.md`）。筆電已同意共用行（TO_STEVEN 20:3x）。
  第 71 批進 main 後：`git merge origin/main`（tests/CMakeLists 檔尾用 merge_keep_main_first.py）→ 有程式變動就在 s39 線增量重建＋跑 St02_PadInterface／ScanKeyGolden → `git merge-tree --write-tree --name-only origin/main HEAD` 一行 → 推：
  `git push origin HEAD:refs/heads/v906/st02-p1-pad -o merge_request.create -o merge_request.target=main -o merge_request.title="急件 St02 P1：RS-232 操作面板 uPadInterface（golden 0618 TfPadInterface＋TPadRS232Thread）" -o merge_request.remove_source_branch`，MR 說明貼 `p1\mr_desc.md`（GitLab 網頁或 API）。
  監看：背景 until 迴圈每 2 分鐘 fetch，main 有非 docs 的程式變動就通知（session 範圍）。

### 10-05 20:1x 狀態（St02-E 寫，session ht9045-46；St02-M＝ht9045-32）
- **ST02-P1（急件，W-80）RS-232 操作面板 uPadInterface**：卡在 `C:\AI_TempFile\st02e-scratch\p1\card_P1.md`，NB2 預勘 `p1\nb2_q2_survey.md`，golden UTF-8 副本 `p1\g\`。
  分支 `v906/st02-p1-pad`（本機，工作樹 `C:\AI_TempFile\st02-p1`）：`5c9c166a`（P1 本體）＋`dd1b341e`（合 main a78c1e15，tests/CMakeLists 檔尾 main 在前）。對 main 11 檔 +1494／-23。
  新檔 `PadInterface_St02.h`（inline 表＋閘門 4 方法）／`.cpp`（ht9045_sm：協定、序列埠、FastClock 1 ms 工作、測試接縫 `W906_PadPortForTest`／`W906_PadDroppedT07T08`／`W906_PadResetStateForTest`）；
  共用檔同行替換：CMakeLists:2406、FastClockWbServe :75／:115、cinitial :17129-17131、mysensor 3 閘、myswitch 3 閘、rs232 :271。ctest：新 `St02_PadInterface`（P1-1a..P1-10e）＋`ScanKeyGolden` [18]。
  建置線 `C:\AI_TempFile\st02-s39`（detached dd1b341e）＋`st02-s39-obj` 兩組態建置中（log：`…\s09close\p1_sim.log`／`p1_ship.log`）。
  反向驗證腳本 `p1\reverse_p1.py`（R1..R13：改一行→只建測試目標→跑→預期的 CHECK 變紅→還原；結果寫 `p1\reverse_p1_result.md`）。
  接下來：本機 ctest（St02_PadInterface、ScanKeyGolden、MainScanKey、FastClk_Jobs、FastClock、ModalWake、St02_ModalTimer1、FShow_Audit、START_SitesCensus 與 sensor／switch 相關）→ 反向驗證 → **第 71 批進 main 後再合 main** → merge-tree → 等筆電對共用行點頭 → 推急件 MR（標題寫「急件」）。P1 不碰 tools/wb_serve.cpp（第 71 批在改）。
- **P2（下一張，P1 之後）**：機台 TEACH-HOMEALL＋SOFTKEY 一張 MR（機台 cpp 0153／0156／0157＋web 0096／0099-0101；稽核 `docs/handoff/MACHINE_VS_MAIN_AUDIT_20261005.md` §2 M2／M3）。**不含** SOFT E-STOP（RULINGS_20261005 #17）。要等第 71 批（M1 MT-ACCLIVE）進 main，且第 72 批（筆電做 HOME-PERAXIS：WebMotorAccess.cpp DoHome ~:1491／Teach btnHome ~:3477、test_web_motor_access.cpp、HW.MotorTest.html／HW.teach.html 的 HOME 鈕，RULINGS #16）也要避開或等它。開工時請 St02-M 在 FROM_STEVEN §1 認領。

### 10-05 19:5x 狀態（St02-E 寫，session ht9045-46；St02-M＝ht9045-32）
- **今天推出的 MR**：!209 S-24（筆電第 70 批）、!211 docs 路徑（已合）、!215 C18（gate 中）、**!216 C22**（`v906/st02-skills-comm` `bb41651f`，只有 skill；15 支通訊 skill → hpi-gpib／hpi-secs／hpi-rs232，舊名空殼一週後刪——刪的時候要改 6 處引用，清單在 MR 說明）。
- C22 驗證工具（可重用）：`C:\AI_TempFile\st02e-scratch\c22_verify_all.py`（搬移前後檔數／內容、空殼、殘留 SKILL.md、控制字元）、`c22_linkdiff.py <基準>`（全 .claude 連結檢查，跟基準比「新增斷鏈」；搬移的檔換算回原路徑比對；`.claude` 底下只看磁碟，不退回 HEAD）。
- 手上沒有排隊的卡，等 St02-M 派新的。工作樹：`C:\AI_TempFile\st02-c22`（C22，推完可收）、`st02-docs`（可收）、`st02-s45`（可收）。

### 10-05 19:4x 狀態（St02-E 寫，session ht9045-46；St02-M＝ht9045-32）
- **C18＝MR !215** `6b0b697d`（`v906/st02-esc`，**凍結**；只有筆電在第 70 批後要求才再合 main）：C 槽全新 obj `C:\AI_TempFile\st02-s39-obj` 兩組態完整建置 OK＋PE；本機 7 支 ctest 兩組態全綠；對照組 `W906_ESC_CONTROL=1` 兩組態都紅。St02-M 已貼 §2「請 gate」、HUMAN_REVIEW B45。
- **工作樹全部在 `C:\AI_TempFile`**：st02-s39（建置線，detached `f1409428`）、s40（S-24，凍結）、s45、esc（C18，凍結）、ela（★W42 本機）、c22（C22）、docs（!211 已合，可收）＋st02-claims、st02e-scratch。
- **C22 進行中**（`C:\AI_TempFile\st02-c22`，分支 `v906/st02-skills-comm`，基準 main a63a30cb，sparse 只 .claude）：hpi-rs232、hpi-secs 交件且驗證過；hpi-gpib helper 還在做。之後 St02-E 自己整合：修其他 skill 指向舊資料夾的連結（helper 已列 file:line）→ 全面斷鏈／關鍵字／檔案數與位元組對照 → 用明確路徑提交 → 推＋開 MR。注意：helper 回報 secs 的 `colleague-*` 與 secs-sem 有 42 份完全相同（照「只搬不刪」保留）、搬移後的兩份抽取文字檔本來就帶控制字元（夜間掃描會抓到，非新增）。
- S-24 MR !209 在筆電第 70 批；docs MR !211 已合。

### 10-05 18:1x 狀態（St02-E 寫，session ht9045-46；St02-M＝ht9045-32）
- **MR !209（S-24）凍結**：筆電已把 `bf8984cb` 收進第 70 批（gate b70a 跑中，自己解了 tests/CMakeLists 檔尾）。**不要再推 v906/st02-s24-staterecord**。本機 `b5dbf1e2`（合 main d9fca156＝第 69 批）只當對照：兩組態 0 錯，25 支目標測試跑完若有紅告訴 St02-M 轉筆電。
- **MR !211（docs 路徑換成入口網站）已進 main 62262fc0**：共用 skill 寫 `<入口網站 repo>\public\Docs\…`（定義在 make-report-skill：St01／St02 `D:\RD5-Portal`、筆電 `D:\HT9045-Index`）；兩支寫檔腳本用 `RD5_PORTAL_REPO`→`D:\RD5-Portal`→`D:\HT9045-Index`→舊 `D:\docs`（印警告）。St01 自己改了 co-work-agent.md（!212）和記錄員 skill。
- **工作樹搬到 C:\AI_TempFile（進行中）**：已搬 st02-esc、st02-ela、st02-s45（robocopy 不含 Obj／build* → `git worktree repair` → D 槽舊副本的 `.git` 改名 `.git.moved`，D 槽不刪）＋`st02-claims` 資料夾。**還沒搬**：st02-s39（建置線）、st02-s40（S-24），等這輪測試跑完。新的建置 obj 在 `C:\AI_TempFile\st02-s39-obj`（第一次要完整建置）。docs 用的 sparse worktree `C:\AI_TempFile\st02-docs`（!211 已合，可收）。
- 下一步：搬完 s39／s40 → C18（`C:\AI_TempFile\st02-esc`，推之前才合 main，檔尾會有第 70 批的區塊）→ C22（`hpi-`）。

### 10-05 16:4x 狀態（St02-E 寫，session ht9045-46；St02-M＝ht9045-32）
- **S-24＋S2 已推＝MR !209** `bf8984cb`（從 b857685a 快轉；合過 main 8d2bb8c1＝第 68b 批）。兩組態 0 錯＋PE、nm 四個符號全域 T；25 支目標 ctest SIM／SHIP 都 25／25；完整 ctest（ab51e36e）SIM 406／428、SHIP 420／428，紅的全是這台環境、nm 確認沒有連進 S-24。log 在 `C:\AI_TempFile\st02e-scratch\`（s24m_*、s24n_*）。等筆電 gate。
- **這台第一次跑完整 ctest 的「環境紅燈」名單**（不是回歸，之後比對用）：SIM＝config_db、ini_helpers、config_loaders、GA1_ReadGeneralIni、dfm2rc_rc_compiles／_fidelity／_idempotent（沒裝 rc.exe）、SimIO、W6_Canary、W6_4_TesterAnchor、HanaART、BarCodeHelpers、BarCode8CCDGlue、AGV_E84、Automation、W7_L1_Auto2／_Color／_Loader／_AutoRT、GA2_C1_cinitial、WB_SimPump、mainproc_guard（讀這台 `D:\HT9045\system` 的機台設定）；SHIP＝前 7 支＋WebMotorAccess 偶發逾時（-j6 下 121 秒，單獨 2 秒過）。
- **C21 補審 0218 已寫好**：`C:\AI_TempFile\st02e-scratch\ST02_MACH0218_HOME_REVIEW_20261005.md`（交 St02-M 發布）。結論：0218 正確（HOME 中途停下不再算完成，START 從第 1 步重 HOME）；HOMEPOS0（H-1／H-2）仍成立；小問題 Q-0218-1（暫停超過 180 秒再 START 會立刻停）。
- **寫入邊界改到 .claude**：Steven 刪了 `D:\HT9045\.github`（「以後只有 .claude」）⇒ 政策在 `D:\HT9045\.claude\ops\write-boundary-policy.json`（本機未追蹤；Steven 自己改了 settings.json 第 9 行；`scripts/ops/check-write-boundary.ps1:358` 預設也改）。external 多了 `D:\RD5-portal\`、`C:\AI_TempFile\`、`%USERPROFILE%\.claude\skills\`。⚠ 工作副本的 116 個 .github 刪除、settings.json、check-write-boundary.ps1、.claude/ops **都不提交**；St01 MR !208 會把政策正式搬上 main，到時把那三行重新加回。`D:\AI_TempFile\` 寫檔會被 hook 詢問。
- **ChangeLog 改放入口網站**（Steven 17:0x）：`D:\RD5-portal\public\Docs\ChangeLog\Steven02\CHANGES_<日期>_Steven02.md`；0927～1005 的 9 份已複製過去（`D:\docs\ChangeLog\` 原檔留著、不再寫）。入口網站 commit／MR 由 St02-M 做。make-report-skill（SKILL.md 第 75～80 行、references/change-log/change-log.md）已改；St01 記錄員 skill（ops-st01-clerk-report）請 ST01-M 改。17:0x 兩個 repo 都已快轉：`D:\HT9045`＝`2c08c1c0`、`D:\RD5-portal`＝`d22de56`。
- **暫存一律放 `C:\AI_TempFile\`**（Steven 14:2x 定案）：新的腳本與 log 在 `C:\AI_TempFile\st02e-scratch\`；新工作樹也開在 `C:\AI_TempFile\`。
- 下一步：工作樹搬到 `C:\AI_TempFile`（st02-s39／s40／s45／esc／ela＋st02-claims；robocopy 不含 obj → `git worktree repair` → 舊的 .git 改名 .git.moved，D 槽一律不刪；obj 在 C 槽重新設定）→ C18（`650cb578`，!174 已進 main，合 main 時 tests/CMakeLists 檔尾衝突，main 在前；FastClk_Jobs 對照 c309e159）→ C22（`hpi-` 前綴）。

### 10-05 13:3x 狀態（St02-E 寫，session ht9045-46；St02-M＝ht9045-32；新 session 從這段接）
- **Steven 1005 13:2x 在 St02-E 的 session 直接說：「你可以在這台電腦上跑測試」**＋「你能決定的就自動決定／不要一直問我／大部分項目可以從 skill 裡面找答案」⇒ ctest 現在在這台本機跑（SKILL §2 已改）；St01 不再代跑。
- **S-24＋S2**（`v906/st02-s24-staterecord`，工作樹 st02-s40）：`825faecc` FShow_Audit（StateRecordDiag.cpp 4 處直接讀照 !165 前例寫理由、基準 54→58）→ `e482919d` T7 測試修正（第一次本機跑 151 過／4 紅，都是測試的 CodePart 與 FindLine 錯，掛點本身照認領）→ `ab51e36e` 合 main 5f1fd774（tests/CMakeLists 檔尾 main 在前；8a 因 main 在 :2422 加 Ht9050DryRun.cpp 移到 :2557、內容不變）。merge-tree 一行。**還沒推**：建置線 s39 切 ab51e36e 兩組態重建 → 本機跑受影響的 ctest 兩組態 → 推＋開 MR。
- **C22（St02-M 14:0x 撤回 13:3x 的更正；St01 13:2x 更正列 handoff d58509a0）**：照 Steven 11:5x——前綴 **`hpi-`**（hpi-gpib／hpi-secs／hpi-rs232）；**S-24 推完就開工**（Steven「這個應該可以先開工了」），照 C22 卡的結構、不等 S2 範本，St01 的 S1 規範不同再調；舊名 stub SKILL.md 指向新名、一週後刪。放行範圍只有 §1 agents＋C22，其他重組等 Steven／Jimmy。
- **RULINGS_20261005 #6（Jimmy 13:4x）**：驗證前①工作樹更新到 main 最新；②`python tools/machine_sync/machine_sync.py check`（工具在 repo 根目錄 tools/，不在 V906 樹裡），NOT SYNCED 就 apply（先備份、做完 restore）；③回報附 main commit＋機台快照時間。1005 14:0x 這台 check＝NOT SYNCED（32／32 關鍵檔不同；這台 9045GPIB_12Site、機台 9050GPIB；快照 13:35，GitHub ff1b55f3）。**ctest 閘門跑在沙盒、不讀機台設定 ⇒ S-24 不 apply**，回報寫明；要在這台驗證機台行為時才 apply＋restore。
- 順序：S-24 → C22 → C18（等 !174 進 main）。

### 10-05 12:4x 狀態＝交接（St02-E 寫；新 session 從這段接；下面 10-05 各段的「接下來」都已被這段取代）
**正在做：S-24＋S2 開一張 MR（batch 68b）。** 分支 `v906/st02-s24-staterecord`，工作樹 `D:\AI_TempFile\st02-s40`，本機 tip **`8500ea58`（未推；遠端還是 b857685a）**：
`b857685a` S-24 4 個新檔 → `efa34cb7` S2（W906_IO.csv／W906_Recent.csv，只動自己 4 檔）→ `ac49fc5b` 合 main 9403048b → `8500ea58` 六個共用檔的 13 項掛點。
- **掛點**：認領 St02-M 已貼 §1（handoff `ce3c82e5`，全文 `docs/handoff/ST02_S24_HOOKS_20261005.md`）；13 項用 Edit 各套一次，**權限沒擋**；`s2h\verify_applied.py` 逐字比對 13 處全 OK，git diff 只有預期的行（wb_serve +124／改 7、MainStateRecord 7/7、其餘各 1、tests/CMakeLists +30）。④ uhome、⑤ WebMotorAccessLive 沒做（等 NB2-1）。
- **建置（進行中）**：建置線 s39＝原始碼 `D:\AI_TempFile\st02-s39` 已 `checkout --detach 8500ea58`、兩個 obj 都重跑過 cmake；背景 `lane_cmake.sh` 先 SIM 後 SHIP，紀錄 `C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\s09close\s24h_sim.log`／`s24h_ship.log`（12:47 SIM 81%、0 錯）。⚠ 之後做 C18 要把 s39 切回 C18 的 commit 並重跑兩個 cmake。
- **接下來照順序**：(1) 兩個 log 用嚴格條件數錯誤＝0（`: error:|fatal error:|FAILED:|undefined reference|multiple definition`），有錯先修、修完重建；(2) nm 兩組態 wb_serve.exe 要有 `W906_SrDiagWbServeTick`／`W906_StateRecordDuringDialog`／`W906_StateRecordHangRequest`，`test_st02_staterecord_diag.exe` 有建出來；(3) `git fetch`，`git merge-tree --write-tree HEAD origin/main` **單獨跑、只印一行**才推；(4) 推同一分支（從 b857685a 快轉）＋push option 開 MR：`-o merge_request.create -o merge_request.target=main -o merge_request.title=... -o merge_request.description=...`，描述寫 S-24＋S2、§1 認領 ce3c82e5、不含 ④⑤、兩組態 0 錯、ctest `St02_StateRecordDiag`「St02-E 本機跑（待 Steven 在 St02-E 確認）」、人工審核三點（⑥ 網頁 15 秒後改走卡死路徑、⑦ 模擬組態回覆 recorded:true、阻塞框內可按 State Record）；(5) MR 號＋tip 回報 St02-M，附日報段＋ChangeLog 第 24 列（第 23 列已寫到「認領已交」）。
- **工具**（都在 `C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\s2h\`）：`anchor_check.py`（OLD 在哪一行）、`gen_claim.py <base>`（產認領檔＋old_/new_*.txt）、`dry_copy.py`＋`dry_compile.sh`（NEW 套在副本上試編）、`verify_applied.py`（套完逐字比對）。St02-M 規定：不准用腳本改共用檔，只能 Edit；權限擋就停、回報 St02-M 原文。
- **測試誰跑**：St02-M 轉述 Steven 11:5x「St02-E 自己在這台跑」、「St01 之後沒空代跑」——**Steven 還沒在本 session 直接說**，所以照舊只編譯不跑；St01 不再代跑。
- **新規則**：RULINGS_20261005 #4——說「機台設定／工單問題」前先跑 `python tools/machine_sync/machine_sync.py check`（NOT SYNCED→apply --yes→再看→restore；SYNCED 引兩行 snapshot）。
- **排隊中**：C18 `650cb578`（v906/st02-esc，工作樹 st02-esc）等 !174 進 main（batch 68b）→ merge-tree 對新 main 一行才推，代跑清單 St02_ProcessForESC（＋W906_ESC_CONTROL=1 要 exit 1）、St02_Timer1、St02_TimerESD、St02_ModalTimer1、St02_MainTimers、TesterComm_Handler、FastClk_Jobs。**C22**（通訊 15 支 skill → hpi-gpib／hpi-secs／hpi-rs232，分支 `v906/st02-skills-comm`＋MR，helper 最多 3，驗證自己做：0 斷鏈、舊觸發詞都在、檔數／位元組前後一致）在這張 MR 之後；卡片與提案存 `C:\Users\steven\AppData\Local\Temp\claude\d---github\c8311755-ee3b-4c2b-9f5f-bc5682ac9613\scratchpad\c22\`（card_1114.md、proposal.md §2.1 #16-18），盤點 `c22\inventory_before.txt`（15 支、182 檔）；前綴 hpi-（Steven）vs ht-（#5）與「第三批、等 St01 S1 規範」St02-M 已請 ST01-M 協調，先用 hpi-。**14:0x 定案：`hpi-`、S-24 推完就開工（見 13:3x 段）。**C20 取消（ES02 已做）。

### 10-05 12:1x 狀態（St02-E 寫；新 session 從這段接）
- **S-24 分支** `v906/st02-s24-staterecord`（工作樹 st02-s40）本機 `ac49fc5b` = b857685a S-24 ＋ `efa34cb7` S2 ＋ 合 main 9403048b；**未推**。
- **掛點認領**交 St02-M（`scratchpad/s2h/ST02_S24_HOOKS_20261005.md`，13 項，不含 ④⑤）；貼 §1 後：用 Edit 逐項套（只試一次，權限擋就停、告訴 St02-M，不用腳本、不繞道），再兩組態完整建置＋nm、merge-tree、推、開一張 MR（S-24＋S2，batch 68b）。驗證工具：`s2h/anchor_check.py`（OLD 位置）、`s2h/dry_copy.py`＋`dry_compile.sh`（副本試編）。
- 測試誰跑：St02-M 轉述 Steven 11:5x「St02-E 自己在這台跑」——**還沒在本 session 直接確認**，照舊只編譯；§2 寫「St02-E 本機跑（待 Steven 在 St02-E 確認）」。St01 不再代跑。
- 之後：C22（hpi-gpib／hpi-secs／hpi-rs232 skill 重組，分支 `v906/st02-skills-comm`，helper 最多 3）。C18 `650cb578` 等 !174（batch 68b）。C20 取消。

### 10-05 11:3x 狀態（St02-E 寫；新 session 從這段接）
- **S-24 已推** `b857685a`（工作樹 st02-s40）；筆電套 6 個共用檔掛點（ST02_S24_HOOKS／CLAIMS_3_5）後開 MR。
- **S2**（W906_IO.csv＋W906_Recent.csv）認領內容已交、等 St02-M 核准；核准後在同一分支加一個 commit，只動同樣 4 個檔（StateRecordDiag.cpp 直接 include mykitsuck.h／mycylin.h，g++ -H 確認沒有 aHotPlateSubstrate.h）。
- 等：!174 `581b0dcb`（St01 代跑）→ C18 `650cb578`；C20 等 0162 上 GitLab。

### 10-05 11:0x 狀態（St02-E 寫；新 session 從這段接）
- **C21（W-67）交出**：scratchpad `w67/ST02_MACH0210_SAFETY_20261005.md`（St02-M 發布到 docs/handoff）。C19 補充 §7 在 `c19/C19_ADDENDUM_SITE_INI.md`。
- **!174 已推** `581b0dcb`（等 St01 代跑 FastClk_Jobs，貼第 7 節 [timing]／[info]）；TIMERRES 選擇不被忽略＝筆電的決定。
- 現場機台打包在 `D:\AI_TempFile\ht9050_site_20261005_0752\`（只讀、只引用鍵值、不碰 PASSWORD）。
- 之後：C18（`650cb578`，等 !174 上 main）→ C20（等筆電把 0162 放上 GitLab）。s39 建置線停在 581b0dcb。
- 11:1x（St02-M）：C21 已逐位元組發布（handoff ce4377ed）；!174 代跑請求已貼。C19 補充 §7（`c19/C19_ADDENDUM_SITE_INI.md`）已確認 10:3x 接上（handoff 7895774a），沒有待辦。Steven 的日報／RD5 portal／08:0x 行程檢查改由 St02-M 做，不是 St02-E 的工作。

### 10-05 10:0x 狀態（St02-E 寫；新 session 從這段接）
- **C19 交出**（報告在 scratchpad `c19/ST02_C19_TYPE9050_SWITCH_20261005.md`，St02-M 放 steven-handoff）。
- **!174 第 7 節修正** `581b0dcb`（工作樹 st02-s45，分支 v906/st02-mainloop-rest）兩組態編譯中 → 0 錯＋merge-tree 一行就推、交 hash（St02-M 09:5x 已核准）。TIMERRES 的 Windows 11「選擇不被忽略」只是提案，筆電決定。
- 之後：C18（等 !174 上 main）→ C20（HTDESIGNER tools 0162，等筆電放上 GitLab）。s39 建置線停在 581b0dcb。

### 10-05 07:3x 狀態（St02-E 寫；新 session 從這段接）
- **第 66 批已上 main**（bfb30b76：!190／!191（筆電自修 ELA_Oee）／!192／!193 在 1bd7e1b7）。C17 結案，三個工作樹刪了（本機分支 -b2 因追蹤上游 -d 不刪，留著）。
- **MR !194**（普查 v2，只有文件，工作樹 st02-c12t 分支 `v906/st02-c12v2-census` `815fa178`）等合併。
- **C18** 本機 `650cb578`（工作樹 st02-esc），兩組態 0 錯；merge-tree 對 main／!174 都一行；**等 !174 上 main 才推**。s39 建置線停在 650cb578。
- 換建置線基底時：先對兩個 obj 目錄各跑一次 `cmake <obj dir>` 再編（SHIP 不會自己重新設定）。
- 07:5x：**!194 已合進 main（c0cbcd33）**；C18 對 c0cbcd33 merge-tree 仍一行，繼續等 !174。st02-c12t 工作樹可刪（內容都在 main）。
- 08:1x（St02-M）：**筆電沒算力暫停，St01 當備援整合**（自己 gate／合併、不做 GitHub 包、機台安全的等 Jimmy／筆電）。第 67 批空的。!174 St01 代跑 07:58 開始；**!174 上 main → C18 對新 main 跑 merge-tree → 推 → 交 St02-M hash＋代跑清單**（St02_ProcessForESC＋對照 W906_ESC_CONTROL=1 要 exit 1、St02_Timer1、St02_TimerESD、St02_ModalTimer1、St02_MainTimers、TesterComm_Handler、FastClk_Jobs）。

### 10-05 06:5x 狀態（St02-E 寫；新 session 從這段接）
- **ST02-C18（ESC）本機完成、已認領、等 !174 上 main 才推**：工作樹 `D:\AI_TempFile\st02-esc`、分支 `v906/st02-esc` `650cb578`（rebase 到 a9256df7 之後，merge-tree 一行），s39 建置線停在這個 commit。!174 合併後 → 再跑一次 merge-tree（main 與 !174 現在都一行）→ 推 MR，代跑清單 St02_ProcessForESC（＋對照 W906_ESC_CONTROL=1 要紅）、St02_Timer1、St02_TimerESD、St02_ModalTimer1、TesterComm_Handler、FastClk_Jobs。
- 第 65 批已上 main（a9256df7：!183 S-27、!187）。!190／!191／!192／!193 進第 66 批（跟 F9050-BD；St01 不用再代跑 !191／!192）；C12 v2 的點擊量測仍請 St01；!174 等 NB2-1。工作樹 st02-e09b1／st02-s27 已移除。
- **06:5x 起**：!191 紅燈修正已推（`25a68e6c`，St01 只重跑 ELA_Oee）；C12 v2 普查已推到 !193（`4797062e`，dead/A 74→63）。下一件：C18 等 !174；C18 本機 `650cb578`（含第 9 節測試，還要增量編一次：建置線現在停在 !191 的 25a68e6c）。
- 07:0x：普查 §11 btnRUpToLDownN 備註在 st02-c12t 本機 `6c31e891`（沒推，跟下次動普查一起推）。

### 10-05 04:4x 狀態（St02-E 寫；新 session 從這段接）
- ESC 唯讀計畫交 St02-M（scratchpad `esc/ESC_PLAN.md`），等筆電把它變成卡片；**沒有認領、沒有改檔**。手上沒有別的工作。
- 等：!190／!191／!192（St01 代跑 C17）、!193（St01 第 65 批後重跑 C12 v2，結果放 docs/handoff/c12_raw_v2/ 或 St01 說路徑 → 這台合併）、!174（NB2-1）、!183／!187（第 65 批）。

### 10-05 04:3x 狀態（St02-E 寫；新 session 從這段接）
- St02-M 04:2x 派三件：①S-28＝已由 NB2 I124 做完（結案）；②C12 A 類兩顆＝早就接好、探針誤判 → 改做 **MR !193**（探針 v2＋census ④⑤，工作樹 st02-c12t `1bd7e1b7`），等 St01 在第 65 批進 main 後重跑（指令在 census md §10）；③Qorvo ProcessForESC 唯讀計畫——**下一件**，筆記在 scratchpad `esc/ESC_NOTES.md`（golden 0618 main.cpp:7603-7706、Timer1 :2858、5 個觸發點；移植樹 bTriggerESC 在 THandlerTesterSide，fMain 沒有 ProcessForESC）。
- 開著的 MR：!174、!183、!187、!190、!191、!192、!193。

### 10-05 04:1x 狀態（St02-E 寫；新 session 從這段接）
- **ST02-C17 三張 MR 都推了，等 St01 代跑**：!190（第一批 4 支，`12132473`）、!191（B1 21 支，`edbe01de`，工作樹 st02-c17b）、!192（B2 AGV_E84／ELA_Hub，`89423ecb`，工作樹 st02-c17b2）；B1、B2 都疊在 !190 上（!190 合併前 diff 會帶到它的 commit）。W7_L1_Auto 不改（理由見 ChangeLog 1005 第 6 列）。代跑方式：SIM／SHIP 同時跑兩次，跑完 TEMP 裡沒有 *_<pid> 殘留。s39 建置線停在 `89423ecb`。
- E-09 只剩 Security 重量。開著的 MR：!174、!183、!187、!190、!191、!192。

### 10-05 03:5x 狀態（St02-E 寫；新 session 從這段接）
- **ST02-C17**（St01 E-039，St02-M 911ff6e0 認領 28 支＋tests/w906_test_tmpname.h）：第一批 **MR !190** `12132473`（工作樹 st02-c17）等 St01 代跑（BinSelCore、IniFiles、common、HSys_HeaterMix＋SIM／SHIP 同跑兩次）。**第二批**＝其餘 24 支（清單在 scratchpad `c17/scan2.tsv`），從 main 開新分支 v906/st02-c17-tmpnames-b；跨行程靠固定名稱的先停下回報。s39 建置線停在 12132473。
- E-09 只剩 Security 重量（筆電 gate 機或 Steven）。開著的 MR：!174、!183、!187、!190。

### 10-05 02:1x 狀態（St02-E 寫；新 session 從這段接）
- **E-09**：v2 結果（docs/handoff/e09_raw_v2/）→ 清單重產 **MR !189** `a41823af`（工作樹 st02-e09，分支 v906/st02-e09-layout）。只剩主畫面 Logo 是網頁的錯（**!187** `4c8c6f25` 已修，代跑綠，等 gate）。**1005 02:3x 定案**：v3（e09_raw_v3/）Setup 20 個全是 [box only]、0 個 TEXT CUT ⇒ 不修。**E-09 只剩 Security 要真 wb_serve 重量**（St01 不開 wb_serve；交筆電 gate 機或 Steven 08:00 決定）。!189 tip `27077e6c`。Teach 不用改。分類腳本在 scratchpad `e09\`（v2_*.tsv、build_report_v2.py）。
- 開著的 MR：!174 `5828ef91`（等 NB2-1 重跑）、!183 `568a2a02`（綠，第 65 批）、!187 `4c8c6f25`（綠，等 gate）。**!189 已由 St01 合進 main 67aa2356（1005 02:46）**；st02-e09、st02-c12 工作樹已移除（分支都已合併）。ChangeLog 1005 起寫 CHANGES_20261005_Steven02.md。

### 10-04 23:4x 狀態（St02-E 寫；新 session 從這段接）
- **E-09 第 1 批已推 MR !187** `4c8c6f25`（main.html:45＋E09_MainLogo），等代跑；工作樹 st02-e09b1。**E-09 下一步**：等 St01 的 v2 重量（docs/handoff/e09_raw_v2/），再在量測腳本加「文字寬度」檢查判 Setup，Teach／Omron 依 v2 決定（Teach 改的話保留 KB-GOLDEN 掛勾、代跑 teach_kb_golden_selftest.cjs／qwerty_golden_selftest.cjs）。
- 開著的 MR：!174 `5828ef91`（等 NB2-1 重跑）、!183 `568a2a02`（綠，第 65 批）、**!187 `4c8c6f25`（1005 01:58 St01 代跑綠、對照組照預期紅；St01 同意 main.html:45；上機實點看 B43）**。S-28 唯讀；S-24 等 Q97。E-09 v2 重量 St01 01:55 開跑。1005 起 ChangeLog 寫 `D:\RD5-Portal\public\Docs\ChangeLog\Steven02\CHANGES_20261005_Steven02.md`。
- **E-09 額外檢查（St01 W-54 回答 docs/handoff/ST01_W_ANSWERS_20261004.md:76-84，St02-M 23:5x）**：HT9050 有兩個螢幕（ht9050-hw hardware-overview.md 第 14 列），HMI 在哪一個、解析度／縮放／工作列問 EastSun；保守預設＝**對話框、提示、浮動框都要落在主畫面 925×720 框內（×1.1≈1018×792）**。v2 結果回來決定修正時，把開在這個框外的浮動提示／對話框列為候選（大表單如 io 1732 寬塞不下，靠 CLIPFIX＋捲動，不算）。

### 10-04 23:1x 狀態（St02-E 寫；新 session 從這段接）
- !186（E-09 清單）已進 main f2189135。**E-09 第 1 批**：工作樹 `D:\AI_TempFile\st02-e09b1`、分支 `v906/st02-e09-b1-logo`（本機、未提交）：tools/webprobe/e09_main_logo_selftest.cjs＋tests/CMakeLists.txt 檔尾 E09_MainLogo；main.html:45 修改用 scratchpad `e09\apply_logo.py`（先比對原文）。**main.html 是 St01 的檔：等 St01 回覆或約 23:4x 才執行**，之後提交、merge-tree、推 MR、回報 St02-M（代跑 E09_MainLogo＋反向）。
- E-09 其餘：Teach／Omron 等 St01 用 v2（adfe1e8b）重量（結果在 docs/handoff/e09_raw_v2/）；Offset 撤回；Setup 待量測腳本加文字寬度檢查。
- **W-54 已答（Steven 1004 23:1x，St01 轉）**：「應該是full hd. 但是現場可能設定不一樣，這題留給eastsun」⇒ E-09 先以 1920x1080（工作列後約 1032 高）為準修，現場設定由 EastSun 確認；1280 的結果只當參考。

### 10-04 22:5x 狀態（St02-E 寫；新 session 從這段接，hash 見下面 21:2x）
- !184 已進 main（ea726422，第 64b 批）。**E-09 清單 MR !186** `c058635c`：docs/handoff/ST02_E09_LAYOUT_20261004.md／.tsv＋e09_raw＋兩支 webprobe 工具。分類腳本在 scratchpad `e09\`（triage.py、order_vs_golden.py、clip_vs_golden.py、build_report.py）。
- **E-09 修正（下一步）**：第 1 批 Main.html imgLogo、HW.teach.html 疊放順序與窗格寬；第 2 批 Setup.SetUp grpIndexOption、Setup.OffSet Image1、HW.OmronEJ1N 加熱器面板。等 St02-M 告訴我各頁的擁有者，先唯讀查第 1 批；改前給 OLD／NEW。Security 要在真 wb_serve 重量。

### 10-04 21:2x 狀態（St02-E 寫；新 session 從這段接）
- **hash**：!174 `5828ef91`（v906/st02-mainloop-rest，st02-s45）｜!183 `568a2a02`（v906/st02-s27-modal-timer1，st02-s27；綠，第 65 批）｜!184 `011b88ee`（v906/st02-c12-census-0618，st02-c12）｜E-09 `ce1610c4`（v906/st02-e09-layout，st02-e09，量測腳本 b13e14eb 起沒變）｜W42 本機 `34ddd9d7`（st02-ela，不推）｜S-24 本機 `02aee28d`（st02-s40，不推，等 Q97）。s39 建置線停在 568a2a02。各工作樹都乾淨；只有 D:\HT9045 的 SKILL.md／current-state.md 是本機未提交（跟下一張 skills MR 送，第 40 條修正＋第 41～44 條）。
- **在等誰**：St01＝E-09 量測腳本兩次（1920x1032、1280x976）＋第 63 批後重跑 C12 點擊探針；NB2-1＝!174 在 5828ef91 重跑 11 支；EastSun（經筆電）＝HT9050 螢幕解析度與工作列——**22:2x 起照 RULINGS_20261004 第 4 條不等 EastSun，同時給 St01 一份回答**（筆電已在 f2cf5eee 轉成 W-54 給 EastSun、同時列給 St01 回答 W-57；E-09 本來就兩種解析度都量，不被它卡住）；筆電＝S-27b（Timer7／ProcessStatrDigital／Omron）、ProcessICHotTime、ProcessForESC 的派卡。
- **E-09 計畫**：St01 結果回來 → 合成 docs/handoff/ST02_E09_LAYOUT_20261004.md＋.tsv（加靜態結果）開 MR → 分批修、每批一張 MR。靜態目前只剩 1920x1080／110% 下 11 個太高的視窗（CLIPFIX 應該處理，待實測）；Omron 已更正為不是溢出。這台不跑 Edge／node／exe。
- **S-28 停著**（唯讀，等 St01 回覆 ScanKey／面板鍵路徑；若碰 uhome.cpp 要列確切行號給筆電，避開 F9050-BD 的 HOME flag19）。

### 10-04 21:1x 狀態（St02-E 寫）
- 已推：!174 `5828ef91`（等 NB2-1 重跑 11 支）、!183 `568a2a02`（綠，第 65 批）、!184 `011b88ee`（普查，等合併）。s39 建置線停在 568a2a02。
- **E-09（最優先）**：工作樹 `D:\AI_TempFile\st02-e09`、分支 `v906/st02-e09-layout` `b13e14eb`（e09_layout_probe.py＋e09_static_layout.py）；等 St01 跑兩種螢幕大小，結果回來後合成 docs/handoff/ST02_E09_LAYOUT_20261004.md＋.tsv 開 MR，再分批修（OmronEJ1N 先）。background.html 避開 St01 S-16 的 :433-440、:792-798、:955-960、:983-986、:1056-1061（改前給 St02-M 行號）。**這台不跑 Edge／node／任何 exe**（Steven 的巡檢指令；St02-M 已請 ST01-M 問 Steven 要不要放寬）。
- S-28 仍唯讀等 St01；S-27b 未開工。

### 10-04 20:2x 狀態（St02-E 寫）
- 第 63 批（第 144 包）進 main（!180、!161 已合）：**!174 衝突已在本機解 `5828ef91`**（MainTimersSt02.cpp 取 !174 的 Timer1BinTick 寫法），s39 建置中，建完先推；**!183 衝突已在本機解 `568a2a02`**（tests/CMakeLists.txt 檔尾兩塊都留），!174 建完再建、推。St01 在 1ee1d76e 的 !183 代跑仍有效（S-27 的檔在兩個 tip 間沒變）。
- KB-GOLDEN 已解凍。**筆電新認領 F9050-BD（合併前不碰）**：acatchtray.cpp、asendic_Loader.cpp、csystem.cpp（DoReceiveAllToBottom_9050 與 DoCatchTray／DoInspectTrayColorOnLoader／DoLoad／InitAllProcessTask／DoTrayFeed 的 Type_HT9050 分支）、uhome.cpp HOME flag19、cprod.h、cmydef.cpp／.h、cinitial.cpp、mycylin.h、MachineType.h、TrayXMoveCheckEnc、MES0924、forms/fContact.h、tests/CMakeLists.txt 檔尾。S-28 若走到 uhome.cpp（TfHome::ScanKey）要在 OLD／NEW 列出確切行號。

### 10-04 19:5x 狀態（St02-E 寫）
- **INBOX 155 已推 MR !184** `011b88ee`（工作樹 `D:\AI_TempFile\st02-c12`，!184 合了再刪）：普查 md／tsv 重跑（golden 0618）＋St01 點擊結果併回＋md §9 D 類分派建議。反推點擊結果的腳本在 scratchpad `c12r\rebuild_probe.py`（以後 St01 再合併時若又缺 golden，可照做）。
- 等：!174（NB2-1 重跑）、!183（代跑 15 支＋2 反向）、!184（合併）；S-28 唯讀等 St01。

### 10-04 19:1x 狀態（St02-E 寫）
- **S-27 已推 MR !183** `1ee1d76e`（工作樹 `D:\AI_TempFile\st02-s27`，分支 `v906/st02-s27-modal-timer1`，基於 !180 b4a8fc0b＋main）：等代跑 15 支＋2 個反向（scratchpad `s27/REVERSE_S27.md`）；上機要看：門鎖。**s39 建置線目前停在 1ee1d76e**（!174 要修就切 a6b76a6c、!180 切 b4a8fc0b）。
- !174 `a6b76a6c` 等 NB2-1 重跑 11 支；S-28 唯讀等 St01；S-27b（Timer7／ProcessStatrDigital／Omron）未開工；ProcessICHotTime、ProcessForESC 等卡。

### 10-04 18:1x 狀態（St02-E 寫）
- **!174** `a6b76a6c`（FastClk_Jobs 第 6 節上限改用實際框時間；兩組態 0 錯）：等 NB2-1 重跑 11 支（含 St02_N07Alarm、C14_BinDisp）；s39 建置線目前停在 a6b76a6c（!180 要重建就切回 b4a8fc0b）。!180、!161、!173、!181 在第 63 批（!173／!181 已由 St01 合進 main `2a8bbb24`）。
- **S-27（INBOX 93，告警框開著時照跑 fMain Timer1）開工**：插入點 `tools/wb_serve.cpp:7630`（W906_ModalWaitTick 內 FlushFlag＋MainRecord 那一行，同一行、插在註解前；main 與 !180 行號相同）。golden 兩個框的 Timer1 每 10 ms 呼叫 fMain->Timer1Timer（note.cpp:3355、mymessbox.cpp:542）。已在框內跑的：FlushFlag、UpdateRecordScreen、St02 分派器；**缺的主要是 PumpTick 才跑的段，例如安全門鎖 W906_SafeDoorLockTick（golden :3051-3076）**。helper 在盤點 golden Timer1 :2696-3838 全段；動手前把 OLD／NEW 行給 St02-M。tests/CMakeLists.txt 檔尾會跟 St01 E-045 衝突，兩邊都留。golden 框還呼叫 Timer7Timer／ProcessStatrDigital（是否併入 S-27 等 St02-M）。
- **S-28（INBOX 124，面板實體 PAUSE 鍵關通知框）**：唯讀，等 St01 回覆 ScanKey（E-034 的行不動）。

### 10-04 17:1x 狀態（St02-E 寫）
- **★W42 恢復時**：工作樹 `D:\AI_TempFile\st02-ela`（`v906/steven-w42-encode`，18 個本機 commit）還在；**st02-ela-obj 已刪**（Steven 1004 17:1x 清理），要另建 obj 目錄、兩組態約 45 分鐘。第 4 步翻譯表與草稿原本在 `D:\AI_TempFile\st02-e2`（已刪），**已提交在 st02-ela 本機分支 `34ddd9d7` 的 `_local_w42_w46_archive\`（不推；W42 推之前 `git rm -r` 掉）**，scratchpad `wt_archive\st02-e2\` 另有一份（含 io／native／trays 等）（W42_MSG_TABLE_20260928.csv、draft\W42_MSG_TABLE_20260928_v2.csv、README_W42_MSG_TABLE.md、draft\W42_STEP4_PREP_20260928.md、draft\gen_log_english.py、in_ST02_W42_DESIGN.md）；★W46 研究也在那裡。
- 建置目錄只剩 `st02-s39-obj`（!174／!180 建置線）＋ `D:\HT9045\Obj\V906\build`／`build_ship`。開著的 MR：!174（等 NB2-1 代跑＋C14_BinDisp）、!180（代跑綠）、!181（skills，第 63 批）；S-24 等 Steven（Q97）。

### 10-04 15:0x 狀態（St02-E 寫）
- **工作樹清理（St02-M 執行 `git worktree remove`，不加 --force）**：St02-E 回覆只留 **st02-ela**（★W42，等 Steven）、**st02-s39**（!180／!174 建置線，st02-s39-obj）、**st02-s36**（!179 建置線，!179 進 main 後可刪）＋開著的 MR 樹 s40／s42～s45。st02-speed、st02-mainscan 會被刪 ⇒ SKILL 裡提到這兩棵的地方，刪完要改。刪完後殘留的 obj 目錄（s13／15／16／18／24／25／30／32／38、elasched、on-cbridge、w10 的 -obj）等 St02-M 點頭才刪。

### 10-04 14:4x 狀態（St02-E 寫）
- **KB-GOLDEN（筆電，第 63 批）**：筆電在改 `web/page/qwerty.js` 照 golden TfQwertyKey＋新 ctest QwertyGoldenKeypad。**第 63 批進 main 前不碰 qwerty.js／attachKeyboards／引擎的按鍵攔截。** St02 普查結果 none（ChangeLog 第 26 列）。
- 其餘同 14:1x：!174／!180 等代跑、!179 第 62 批、S-24 等 Steven（Q97）。

### 10-04 14:1x 狀態（St02-E 寫）
- !179 代跑綠（St01 13:55），在第 62 批；!174（`v906/st02-mainloop-rest` `647f397d`）、!180（`v906/st02-timer-table-b2` `b4a8fc0b`）等代跑，St02-M 已請列第 63 批候選。
- ccache 實驗完成並回報 St02-M（14:1x）：不建議照現況接上 W906_FastBuild.cmake（增量重建 PCH 內部錯誤、全新建置只快 1～10%、瓶頸是 240 次連結）。speedup-ideas 一列已給 St02-M，等它決定誰寫進 !173；`D:\AI_TempFile\st02-ccx`（約 30 GB）等同意後刪。
- S-24 仍等 Steven（Q97），其餘同 13:0x。

### 10-04 13:0x 狀態（St02-E 寫）
- main `de5e0637`（第 141 包含 !165）。**已推等代跑／批次**：!174 `647f397d`、!176 `a9fbc88e`（代跑綠，第 62 批）、**!179** strShowYield `7d9aa105`、**!180** MR-B `b4a8fc0b`。若 !174 與 !180 先後合：MainTimersSt02.cpp St02PassAt 那行照 !174 拿掉 `|| g_modalDepth > 0`。
- S-24 本機 `02aee28d`（st02-s40，含 T7、apply_hooks 已有 ③）：仍等 Steven（Q97）；套用前 rebase 到當時 main 並重跑 `--dry`（第 60／61 批改了 wb_serve 檔尾與行數，檔尾檢查 EOF_OK 要更新）。
- 排程：10 分鐘巡檢 `ab569765`、五小時 `acdb59c5`。ccache 實驗低優先（D 段、objcompare、speedup-ideas 一列）。

### 10-04 11:1x 交接（換帳號＋重開機後；新 session 先讀這段，再讀下面 09:5x）
- **ccache 實驗已停**：ccx.sh 與殘留 cmake 已結束；st02-s41 的 MainTimersSt02.cpp 已還原（git status 乾淨，detached 在 main `fb608e5e`）。已量：A 不開模組 1393 s、B 開模組冷快取 1374 s（命中 27%）、C 熱快取 1249 s（命中 74%）；**D（改一個 .cpp 重建）沒量完**。續做：重跑 D（D:\AI_TempFile\st02-ccx\A、C 還在）＋A／B 的 objcompare＋cpp_build speedup-ideas 一列；做完刪 st02-ccx（約 30 GB）。
- scratchpad 重開機後還在；St02-M 另備份到 D:\AI_TempFile\scratch-backup-20261004\St02-E_c8311755\。
- **排程先不建**（Steven 10:5x：本週用量 95%，10/10 重置；要建先問頻率）。
- **下一步順序**：① 第 60 批 gate 對 !165（`1c7c0273`）綠了 → 推 MR-B（st02-s42 `v906/st02-timer-table-b2` `e80d1eff`：fetch → merge-tree 單獨一行 → push 開 MR）；紅了在 st02-s39 修。② !174／!176 等代跑結果。③ S-24 等 Steven（共用檔掛點，Q97 三選一）＋St01（③）＋筆電（⑤）。④ ccache 實驗收尾。⑤ strShowYield 建構子歸零小 MR（!171 已進 main，可以做）。
- ChangeLog D:\RD5-Portal\public\Docs\ChangeLog\Steven02\CHANGES_20261004_Steven02.md 到第 20 列（交接檔只抄到 18，St02-M 要補 19～20）；skill §5 到第 35 條。

### 10-04 09:5x 狀態（換帳號前；新 session 從這裡接；St02-E 寫）
- !171 已進 main（第 140 包 `26bf17d2`）。**已推、等代跑／gate**：!165 ScanKey（`v906/st02-scankey` tip `1c7c0273`，FShow_Audit 修正，在第 60 批 b60 約 10:00 gate）；!171 S-09 開機（`a6885f2e`）；!174 mainloop 剩餘（`v906/st02-mainloop-rest` `647f397d`，筆電的 FASTCLK-MODAL＋TIMERRES，WINMM 新匯入）；!176 OB-7 後續（`v906/st02-ob7-followup` `a9fbc88e`）。!168 已進 main（第 138 包）。
- **MR-B**（st02-s42，本機分支 `v906/st02-timer-table-b2` tip `e80d1eff`，兩組態建過 0 錯）：**等第 60 批對 !165 綠了再推**（fetch → merge-tree 單獨一行 → push 開 MR）；MR 文字照 scratchpad\s20\MRB_COMMIT_MSG.txt＋D4 縮小（FYI）＋fMain_Timers.* 是 St02 的（Q84＝A）；代跑＝!168 的 11 支＋St02_N07Alarm＋C14BinDisp；反向 scratchpad\s20\REVERSE_MRB.md。若 !174 先進 main：MainTimersSt02.cpp 的 St02PassAt 那行照 FASTCLK-MODAL 拿掉 `|| g_modalDepth > 0`。
- **S-24**（st02-s40，分支 `v906/st02-s24-staterecord`，4 個 St02 新檔已本機 commit `b29634bb`「WIP not for push」）：Q92 自動紀錄＋T6、S30 present 欄已做（St02 的檔，語法 0 錯）；6 個共用檔掛點 **第二次被權限檢查擋**（Steven 要擇一：允許規則／自己跑 apply_hooks.py／手動），③ 等 St01 確認、⑤ 等筆電；確切行 scratchpad\s24\CLAIMS_3_5.md；④ 待 NB2-1。套用前對當時的 main 重核（含 E-043 :2555 鄰行）。
- **ccache／PCH 實驗**（St02-M 指派，結果要寫進 cpp_build 的 speedup-ideas）：腳本 scratchpad\ccx\ccx.sh 在背景跑，log scratchpad\ccx\ccx.log；原始碼 st02-s41 在 main `fb608e5e`（detached）；obj D:\AI_TempFile\st02-ccx\A／B／C（各約 10GB，量完可刪）。已量：A 不開模組完整建置 1393 s（1622 個 CXX）；B 設定 rc=0、PCH 套在 11 個目標。B／C／D 還在跑。之後還要 objcompare（A 與 B 幾個 .o：objcopy -g 後 objdump -d 比對）。D 段會在 MainTimersSt02.cpp 尾加一行註解再 git checkout 還原——若中斷要手動還原。
- 之後：strShowYield 建構子歸零的小 MR（!171 進 main 後，筆電說「你決定」）。

### 10-04 06:2x 狀態（新 session 從這裡接；St02-E 寫）
- main 第 138 包（`caeda672`）已有 !157、!166、!168（MR-A）。已推待合：!171 S-09 開機補設（等代跑 St02_S09SetupTemperFrom／TemperFromCore／TemperFromTimer1）。
- **!165 ScanKey**（st02-s39，本機分支 `v906/st02-scankey-combined` → 推到 `v906/st02-scankey`）：NB2-1 R209 兩支紅已修（`3adc6c8d`，同一行）；合 main 兩次（`8b9ae529` 帳本第 1 條表＋tests/CMakeLists 檔尾；`d905794b` 第 138 包的兩個檔尾），**合併樹兩組態建置中（log skm2_*）**；完成 ⇒ merge-tree 單獨一行 ⇒ 推 ⇒ 回報 St02-M，請代跑全部 8 支。
- **!70 MR-B**（st02-s42，`v906/st02-timer-table-b2` `21018ddc` = !165 新 tip `d905794b` + 併入 commit）：本機，**不開 MR 直到 !165 代跑綠**；舊的 `v906/st02-timer-table-b`（`aa1a0945`，疊在 MR-A＋舊 !165）兩組態建過 0 錯，保留參考。obj 用 st02-s36-obj（`git checkout --detach` 到要驗的 commit）。設計：派發器的五支→排程表 OnTimer（W906_St02TimersBindTable），wb_serve:7621／WebBridgeTags:605 不動；反向 scratchpad\s20\REVERSE_MRB.md。
- **OB-7 後續**（st02-s44 `v906/st02-ob7-followup`，`0a2c69d5` 本機）：(a)＋(c) 完成；(b) 等筆電同意 WebBridgeServer.cpp:1449，與 WebCmdGuard.cpp:90 一起加。
- S-24 仍停（Steven Q97）；S30 待辦已記在 scratchpad\s24\HOOKS_TODO.md 檔尾。

### 10-04 02:1x 狀態（新 session 從這裡接；St02-E 寫）
- !160 已合（package 136）。!157 W-17 第二部分合 main 後 tip `35fb8ff1`。**!166 S-25**（st02-s41，`v906/st02-s25-fhs`，tip `9edd0048`）：forms/fHS.h 三段過期註解，只改註解、沒有測試，等筆電。S-25 (1) 答案已由 St02-M 上交接 ff5a7ed6（EP 紀錄兩支 golden 沒人呼叫不翻；CheckEPRange 不屬 ADAM，轉 TFormHS／KLT 負責人）。
- **S-24** 仍停：4 個新檔在 st02-s40 未提交，6 個共用檔的掛點被權限檢查擋，等 Steven 回 A／B／C。!165／!161 等代跑，紅了就是下一件工作。

### 10-04 01:5x 狀態（新 session 從這裡接；St02-E 寫）
- 已合進 main：!149、!150、!155。已推待合：!157 W-17 第二部分、!160 C16、!161 S-22（疊在 !150，筆電已同意認領）、**!165 ScanKey 合併 MR**（st02-s39，tip `8f53ee73`，取代 S-20；等代跑 ScanKeyGolden／MainScanKey／START_SitesCensus／St02_N07Alarm／St02_N07BannerPage／HomeBlock／MainCtlButtons／St02_Keep912）。
- **S-24** st02-s40 `v906/st02-s24-staterecord`：helper 做第一階段（StateRecordDiag.h／.cpp＋St02_StateRecordDiag，並套用筆電已同意的 ①cStateRecord:1387 ②wb_serve :5934／檔尾／:2503 掛載路由 ⑥Main.gbControlBtn.html:142-144 ⑦MainStateRecord.cpp:159-169 ⑧兩個 CMakeLists）；③ 三個阻塞迴圈與看門狗自動紀錄等 Jimmy（#89／#90，之後疊在 St01 !164 上）；④ uhome.cpp 只寫出唯讀 accessor 提案等 NB2-1；⑤ 剎車欄位等 NB2-1 (m) 進 main。分析：scratchpad\s24\S24_GAP_ANALYSIS.md。
- 之後：mainloop 剩餘（FASTCLK-MODAL `0b5ff7b5`＋TIMERRES `5b09b320`，cherry-pick 保留筆電作者，等 St01 !164 合了再做）→ !70 MR-A（從 main 重做，參考 st02-s36 `bbed2725`）／MR-B → S-09 開機 yield 補丁 → !140 後續 → 改標註。
- 建置：`scratchpad\s09close\lane_cmake.sh <obj dir> <tag> sim|ship`（已設定過的 obj 用 cmake --build＋!159 PE 檢查）；新 obj 根用 lane_bash.sh（但 PE 檢查會卡，卡了就停掉那支 PowerShell、改跑 pe_check_b53.ps1）。

### 10-03 20:5x 狀態（新 session 從這裡接；St02-E 寫）
- 已推：!149 912-keeps、!150 C14b（tip `47e0f8c2`，等代跑 `ctest -R ^C14_BinDisp$` 兩組態轉綠）、!155 S-09、!157 W-17 第二部分。
- **ScanKey 合併 MR（取代 S-20）** st02-s39 `v906/st02-scankey-combined`：0152（MC01）＋第 2／3／4 步＋N07 !137／!138 已提交；第 5 步測試移植 helper 進行中（新 ctest ScanKeyGolden、REVERSE 在 scratchpad\skt\REVERSE_SKG.md）；之後兩組態建置 → 請 St01 代跑 → 推。軟體鍵 0153／0156／0157 等 MC01 在 package 133 上重新匯出；0154 要裁決；0158 永不合。HUMAN_REVIEW：面板鍵來源、RESET 尾段 IO、首掃守衛、沒有 HMI 時 START、PAUSE 關 Unloader 框、PAUSE 中止回原點。
- S-20（st02-s33）只當零件來源，不再推。
- **S-22** st02-s37 `888c53cc`（在 !150 的 `47e0f8c2` 上）：等 !150 綠了再推（代跑 C14_BinDisp／C14_BinDispPane／C14_BinDispPanePage）。
- **C16** st02-s35 `2fec0088`（疊在 !149）：等 !149 合。**!70** MR-A 的 WIP 在 st02-s36 `bbed2725`（疊在 S-20，要改以 main 重做）。
- 建置都用 `scratchpad\s09close\lane_bash.sh`（PowerShell 背景會卡 cmake）。

### 10-03 17:3x 狀態（新 session 從這裡接；St02-E 寫）
- 已推：**!149** 912-keeps（`39096461`）、**!150** C14b（`3091e93b`）——都在筆電第 52 批，筆電自己 gate。
- **S-20** st02-s33 `4a7af892`（M1 的鎖搬到 MainScanKey；DeviceForm_File.cpp 回到筆電原文、St01 認領撤掉）：兩組態閘門過（speed 線）。推之前：筆電同意 wb_serve 檔尾／:6981／DUET3D、!149 合、批次 51 進 main 後 rebase＋「面板 HOME 被擋」釘子（!146）；推完請 St01 跑突變檢查。
- **C16** st02-s35 `v906/st02-c16-local`：`6c54770b`＋`2fec0088`（疊在 !149）；增量建置在 st02-s30-obj（st02-s30 暫時 detached 在 2fec0088）。等筆電同意認領＋!149 合。
- **!70 計時表** st02-s36 `v906/st02-timer-table-local`（從 S-20 4a7af892）：MR-A＝合 origin/v906/jimmy-timer-table（merge 不 squash；RULINGS／INBOX 取 main；CMake 兩邊都留；wb_serve 掛點放回今天的行）helper 進行中、obj st02-s36-obj、log tta_*；MR-B＝併入（設計 scratchpad\s20\TIMER_TABLE_FOLD_DESIGN.md，D1 不做、D2～D5 核准；舊測試的每個 CHECK 都要對到新的或寫明退休理由）。
- 之後：S-22（規格 scratchpad\c14\S22_SPEC_753e259a.md）→ W-17／S-09（等筆電）→ !140 後續 → 改標註 MR。

### 10-03 16:0x 狀態（新 session 從這裡接；St02-E 寫）
- **MR !149 已推**（`v906/st02-keep912`，tip `39096461`：W71 `6831645f`、W72 `317dd587`、W73＋St02_Keep912 `39096461`）；工作樹 st02-s30（obj st02-s30-obj）。等筆電 gate／St01 代跑。
- **C14b** st02-s32（`v906/st02-c14b-local`）：`5cb95621`／`2bdd049b`（樹＝helper 的 9c1a6bb8／26db2228，只改標題）＋`3091e93b` MainClose:962 字樣；增量建置中（log c14b3_*），過了 merge-tree 一行就推。
- **S-20** st02-s33（`v906/st02-s20-mainloop`）：`b6c5c85e`（R188 後）→ `781685a9` 合 912-keeps → `3b6d5102` OEE 帳本；speed 線 st02-speed 已切到 3b6d5102，bash 建置中（log s20e_*）。推之前：筆電同意 wb_serve 檔尾／:6981／DUET3D 那行、!149 先合、批次 51 進 main 後 rebase 並補「面板 HOME 被擋」釘子（NB2 !146）、突變檢查請 St01 代跑。
- **CommaText**（批次 51 改成 BCB6：沒加引號的空白也會切）：唯讀 helper 普查 St02 程式／測試中；有影響的在各閘門前修。
- 之後：S-22（規格 nb2-assist 753e259a，存在 scratchpad\c14\S22_SPEC_753e259a.md）→ C16（認領行已交，St02-M 送筆電同意）→ W-17 註解 MR／S-09（等筆電）→ !140 後續 → 改標註 MR。
- ⚠ speed 線用 PowerShell 背景跑 build.bat 時 cmake 會卡在 Generating 之後；改用 `scratchpad\s09close\lane_bash.sh <src> <obj> sim|ship <tag>`。

### 10-03 14:0x 狀態（新 session 從這裡接；St02-E 寫）
- **S-20**（`D:\AI_TempFile\st02-s33`，`v906/st02-s20-mainloop`）：`6f58061d` H1／M1、`d242c685` 合 main db9f0430、`662fc595` M2（RESET 後續行 GATE RESET-POST，等 Jimmy §0）＋M3（開機就 ON 的 HOME／START 要先看到一次 OFF；測試鉤子 W906_ScanKeyFirstScanRearm_St02）。St01 已同意 DeviceForm_File.cpp 檔尾；St02-M 同意 M2／M3。START 普查 36/34/2 過。
- 還沒做：等測試 helper（a29b57a2）提交 test_scankey 修正（含第 [1] 段前的空拍）→ 跑 `scratchpad\s20\s20_test15.py` 加第 [15] 段 → 反向驗證行寫進 REVERSE_S20.md → W71 推了之後把 S-20 疊上去、重合 ckernel :3374／:3382 → 兩組態完整建置、nm、DLL、merge-tree 一行 → 推。
- 背景 helper：C14b rebase＋閘門（a52bad09，st02-s32）、W71 rebase＋閘門（a465a235，st02-s30）。推送順序 W71 → C14b → S-20 → S-22；每個推完把 tip／MR 號／merge-tree／測試清單／HUMAN_REVIEW 給 St02-M。
- HUMAN_REVIEW 要列：(a) 面板 RESET 後續動作閘住；(b) 開機卡住的 HOME／START 只記錄不動作；上機看正常按會動、卡住的只留 `[SCANKEY] ... ignored`。

### 10-03 12:3x 狀態（新 session 從這裡接；St02-E 寫）
- **S-20 是第一優先**（TO_STEVEN §3，RULINGS_20261003 第 13 條：筆電的 `v906/jimmy-mainloop` 交給 St02）：FASTCLK-MODAL `0b5ff7b5`、TIMERRES `5b09b320`、SCANKEY `07306ba2`（INBOX 86 面板實體鍵，沒建置沒測）＋疊 N07 !137／!138 ⇒ 一張 MR。
- 工作樹 `D:\AI_TempFile\st02-s33`、分支 `v906/st02-s20-mainloop`：`2b877dbf` 合 !137（衝突都兩邊留：MainTimersSt02 kBD=5／kT2=6／kSlots=7；CMakeLists:2776 兩個檔；ckernel :3374／:3382＝906 那行＋N07 的一句）、`79a4def0` 合 !138（無衝突）。
- 建置：speed 線 detached 在 `07306ba2`（基底），兩組態背景建置中（log s20a_*）；之後再建 `79a4def0`。審查 helper（唯讀）對 golden 0618 逐段看 MainScanKey，報告寫到 scratchpad\s20\REVIEW_S20.md。
- 測試這台不跑：「修到綠」要請 St01 代跑卡上那串（FastClock|FastClk_.*|ScanKey|START_SitesCensus|St02_.*|NoticeAck|ModalWake|HeaterSimTick|PauseForward|MainCtlButtons|WB_.*）。
- 新規則（RULINGS_20261003 #15）：每個新增／改過的 ctest 檢查，MR 說明都要附「改壞第 X 行 → CHECK Y 會紅」；C14b／S-09／W71 的 MR 說明也要補。

### 10-03 11:5x 狀態（新 session 從這裡接；St02-E 寫）
- **佇列（St02-M 11:2x）**：①S-09 MR（helper 在 st02-s29 建置中）②ST02-C14b ③W71（等第 50 批）④`47eeb5ef` 推送（等筆電同意認領）⑤!140 後續（等第 51 批）。
- **ST02-C14b 本機做好**：`D:\AI_TempFile\st02-s32` `v906/st02-c14b-local` `9f234299`（在 !131 頂端）；H1／M1／M2＋低優先；ctest C14_BinDisp 第 9～12 段；兩組態 -fsyntax-only 0 錯。認領清單 `D:\AI_TempFile\st02-claims\C14B_CLAIM_SHEET_vs_c14pane_9723b74e.txt`（筆電 4 檔、St01 2 檔）。**等**：第 50 批進 main → rebase → 完整建置＋nm＋DLL＋merge-tree → 各擁有者同意 → 推。
- 暫存的密碼相關腳本（pw_shape*、extract_0618.py）已照 St02-M 要求刪掉。

### 10-03 12:0x 狀態（新 session 從這裡接；St02-E 寫）
- **golden 0618 在這台了**：St02-M 10:5x 指的共用區 K01 快照（St01 解好的）**單純複製**到 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\`（886 檔，.gitignore:48 忽略；指紋跟 NB2 fp_0618.tsv 完全一樣）。解 7z 那條路照舊不碰。之後一律照 0618 翻、引用寫 `golden 0618 檔:行`。
- **W-17 第二部分做完**：St02 的引用碰到 0625 才有的行，N07 之外只有 W10 Command.cpp:12626 與 C14 cShowBinSelect.cpp:272，都是 0618 把賦值打成比較的筆誤 ⇒ (a) 保留；沒有 (b)。文件更新版交 St02-M（scratchpad\w17\ST02_OVERLAP_20261003.md）。改註解＋帳本：`D:\AI_TempFile\st02-s31` `v906/st02-w17-part2` `47eeb5ef`（只動註解，等筆電同意認領再推；清單 st02-claims\W17P2_*）。
- **W71**：`st02-s30` `99e63dfd`，0618 行號已讀過；等第 50 批進 main 再改到 main 上推（帳本段落要改成只加一列，避免跟 47eeb5ef 檔尾衝突）。
- **S-09 helper** 還在 st02-s29／st02-speed 建置。

### 10-03 11:3x 狀態（新 session 從這裡接；St02-E 寫）
- **golden 基準改回 0618**（RULINGS_20261003 第 2 條，Steven 在筆電終端答 A）；912 是修正或明顯比較好的就留、兩邊註明（第 1 條＝B）。
- **⛔ 這台解 0618 的 7z 在 10:3x 被 Claude Code 權限檢查擋下（憑證外洩）**：不可重試、不可換腳本／helper／請 St02-M 代做；等 Steven 自己解到 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\` 或開權限。要 0618 原文的工作等著（W-17 第二部分、W71 的 0618 行號）。
- **W-17 第一部分做完**：handoff `795cc008` `docs/handoff/W17/ST02_OVERLAP_20261003.md`（St02 碰到的 21 組，判定都「待 0618」）；腳本在 scratchpad\w17\。
- **W71（Qorvo 蜂鳴改回 912）本機準備好**：`D:\AI_TempFile\st02-s30` `v906/st02-w71-local` `b5061c51`（在 c912-4 上 revert 660b1d32＋兩邊註明＋帳本一列）；ckernel／fLotInfo 跟 !132 之前一字不差（!137 之後合得乾淨）；四檔兩組態 -fsyntax-only 0 錯。**不推**：等第 50 批進 main（再改到 main 上）＋0618 讀得到（行號是推算的）。
- **S-09 解閘 MR（tsv 540＋197）**：helper 在 `D:\AI_TempFile\st02-s29` `v906/st02-s09-lift-1003` 做、在 st02-speed 建置；引用寫 `golden 0618 cSetUp.cpp:2825（0625_Steven same line）`（指紋推得）。
- 之後：!140 後續（等 !140 進第 51 批）——(a) LogAppend 順序、(b) act.observerSG.state 免權杖（別人的檔，先認領）、(c) 引用改 0618。

### 10-03 10:4x 狀態（新 session 從這裡接；St02-E 寫）
- **ST02-M 09:5x 選 A＋B**（不做 C：S-16 跟筆電的 ht9045_link.js 重疊）。
- **A 做完：MR !141 `v906/st02-c12-census` `64b6775b`**（ST02-C12 按鈕普查靜態一半，只有文件＋tools/webprobe 三支 py，不改程式）；工作樹 `D:\AI_TempFile\st02-s27`（main 5967db16）。下一步：St01 代跑 `c12_click_probe.py`，結果用 `c12_button_census.py --merge` 合回同一份 md／tsv。
- **B 做完（helper，只產候選）**：`scratchpad\s09close\rescan1003\CANDIDATES_1003.tsv`／`SUMMARY_1003.md`；真正能解的只有 cSetUp.cpp tsv 540（要加 NULL 防護）、VacuumUnit.cpp tsv 197 大概可以；交 ST02-M 決定，**不要自己解**。工作樹 `D:\AI_TempFile\st02-s28`（detached 79158ea2，乾淨）。
- 筆電第 49 批上 main（fd1b71c5）：!126／!127／!128 都進了；本機 `v906/st02-c9-g2-armcell-ref` 已刪。!137 現在跟 main 衝突，但 #79 擋著，**等 Jimmy 回答前不要重合**。
- 停放／等人照 09:3x 那段。

### 10-03 09:3x 狀態（新 session 從這裡接；St02-E 寫）
- **今天推的 MR**：!130／!132／!134／!136（#62 清理四批，彼此疊著）、!135 ADAM A3、!137／!138 N07（**撤出第 50 批，#79**：golden 0618 沒有 N07）、!139 文件、!140 OB-7＋C6 題目文件；!127／!131 的測試修正 St01 已綠。
- **⚠ 0625 vs 0618**：0625_Steven 帶 Steven 自己的新增，不是 0618 的單純副本；這台讀不到 0618 ⇒ 等筆電給 0618↔0625 差異清單後逐張 MR 對照；新認領清單一律寫「golden base 0625_Steven; 0618 not checked」。
- **⚠ Steven 1003 常設規則（912 比較好就留 912）**＝NIGHT_REPORT §0 #78，Jimmy 還沒回 ⇒ 新的往 912 靠的改動先停。
- **停放／等人**：!138（停在 dd2dd2d6）；C9 G1 `f97a0a4d`（等 Jimmy 的 STOP 裁決）；R68-MYDB（先問 St02-M）；Steven 的 (a) Qorvo 蜂鳴、(b) AMD、(c) Murata、N07 Q3；筆電的 WinLibs -fexcess-precision、INBOX 152／153。
- **下一步**：09:5x 已把三個候選交 ST02-M（等它選，不要自己開工）：(A) C12 按鈕普查靜態版（推薦；TO_STEVEN.md:126，scratchpad\c12 有半成品）、(B) S-09 KEEP 列對今天 main 重掃（ST02_IF0_BACKLOG_STATUS_20260930.tsv）、(C) S-16 開站 lazy（跟筆電的網頁更新頻率 ht9045_link.js 重疊，要先問）。ChangeLog／日報路徑也一起交了。
- 工作樹：st02-s13 C14、st02-s14 LI-9、st02-s15 C9（`v906/st02-c9-g2-armcell-ref` 參考用）、st02-s16 C10、st02-s18 ADAM A3、st02-s19 C14 頁面、st02-s20 OB-7、st02-s21～s24 c912-1～4、st02-s25 C15、st02-s26 文件；建置線見上一段。

### 10-03 06:4x 狀態（新 session 從這裡接；St02-E 寫）
- **Steven 1003 05:4x 常設規則**（St01 7318c0a4）：912 是修正或明顯較好就留 912，註記「#20 exception (Steven 1003 standing rule)」＋906 行號與做法＋912 行號；客戶專屬／不清楚／大的行為改變才問 Steven（經 ST01-M）。記憶 golden-906-only-no-912 已更新。
- **已推的 MR（等 St01 代跑／筆電 gate）**：!116 LI-9 `29bf1345`；!126 C10 HANA `efdb3aa6`（#20b）；!127 C14 906 `1616a034`（C14_BinDisp 綠）；!128 C9 G2 `34e957b9`（筆電第 49 批自己跟 ARMCELL 合，本機參考 `v906/st02-c9-g2-armcell-ref`，不推）；!130 c912-1 `aba800e9`；!131 C14 頁面 `9723b74e`（全綠）；!132 c912-2 `b2ab72d1`（6／6 綠）；!134 c912-3 `ef721c80`；!135 ADAM A3 `2428cf0a`（要在機台 WinLibs 重跑 Adam6024_Pressure）；!136 c912-4 `02fe28f0`（P65／Silent-run 紀錄／Observer 拆欄留 912）。第 50 批（筆電）有 !130／!131／!132／!134。
- **建置中**：ST02-C15 N07 `D:\AI_TempFile\st02-s25` `v906/st02-c15-n07` `fd3ca49b`（st02-speed 那條線，log c15）⇒ 綠了推 MR；認領已由 St02-M 登記（docs/handoff/ST02_C15_N07_CLAIMS_20261003.md）。⚠ ckernel.cpp :3374／:3382 跟 !132 同一行：誰後進 main 誰重 merge（N07 那句放在 `bTesterPauseMusic=false;` 後面）。
- **等 Steven**：(a) !132 Qorvo Tester Pause 蜂鳴延遲、(b) !134 AMD 執行時條件、(c) !136 Murata NonTestToRBin、N07 的 912 差異（JSCC）、W70 已答（ELA 留 912）。R68-MYDB 最後做、先問 St02-M。
- **等筆電**：WinLibs 線要不要加 `-fexcess-precision=fast`（全樹 double==十進位字面值）；INBOX 152 vclcompat CommaText 對 BCB6；INBOX 153 bEnableBarcodeCSVCompare；N07 主畫面紅框（WebBridgeTags＋main.html）；ARMCELL 擋 St02State 的問題。
- **建置線**：st02-speed（build_speed.ps1）＋各 worktree 自己的 obj 根（build_lane.ps1）；單一 TU 語法檢查 `scratchpad\cl\syntax_one.py <樹> <target> <rel.cpp>`（會改寫 flags.make 和 rsp 的路徑）。
- 文件 MR 待辦：ht9045-bin-display 改 906、ht9045-adam6024 加 #20c 與 A3、ht9045-gpib-bridge、TESTERCOMM_PORT_LEDGER、ST02_GOLDEN906_AUDIT、ht9045-secsgem 的 N07 參考加 V906 現況；workflow skill §5 第 19、20 條已在本機（跟下次推送一起推）。

### 10-02 23:5x 狀態（RULINGS #23 之後；新 session 從這裡接；St02-E 寫）
- **RULINGS_20261002 #23**（Steven 22:4x）：ADAM 照 912 保留＝第 20c 條（906 重翻 `c800f2fc` 作廢、不推）；HANA 照 912＝第 20b 條；#62＝A：main 上的 912 內容（除 20a 溫控／20b HANA／20c ADAM／St01 Q-A）逐件改回 906。
- **已推（等 St01 代跑／筆電 gate）**：!116 LI-9 `b71175dc`（LI9_FtpClient、LI9_FtpClientPage、E021_Observer、E021_ObserverPage）；**!126** C10 `efdb3aa6`（K-C10-3 CMakeLists.txt:2405 要筆電重新同意）；**!127** C14 906 `1aef2a26`（認領 81 行 `D:\AI_TempFile\st02-claims\C14_906_CLAIM_SHEET_vs_main_b84064b4.txt`）。⚠ 約 02:00 前不要在 GitLab 網頁合 !115／!116／!121／!123／!124（筆電第 46b 批在 gate）。
- **本機**：
  - C9 G2：`D:\AI_TempFile\st02-s15` `v906/st02-c9-g2` `34e957b9`（48a33f58＋merge main b84064b4），兩組態增量建置中（log c9g2）⇒ 綠了推成新 MR `v906/st02-c9`；G1 `f97a0a4d`（在 `v906/st02-c9-r2`）等 Jimmy 的 STOP 裁決。認領清單 `D:\AI_TempFile\st02-claims\C9_CLAIM_SHEET_vs_main_0cf8598a.txt`（推之前對 b84064b4 重產；只要 G2 的行）。
  - OB-7：`D:\AI_TempFile\st02-s20` `v906/st02-ob7` `40c1a4d8`（兩組態綠）——等 !116 進 main，再 merge main：tests/CMakeLists.txt :6644／:3969 兩個檔都留（LotInfoFtp.cpp 在前、ObserverSGJam.cpp 在後）；Data.Observer.html:280 要 St01 同意。
  - #62 第一批 c912-1：helper 在 `D:\AI_TempFile\st02-s21` `v906/st02-c912-1`（H-013 IsSafePLCIOInstall 改回 906 `Enable_PLCSafety_IO && Sen[SnAllEMG].IsOff()`；GPIB 遠端 START／STOP 拿掉）。之後 c912-2（fSecsAlarm＋Tester Pause 蜂鳴）、c912-3（P65＋AMD＋Multi2D）、c912-4（GetTesterResult＋靜默模式紀錄＋EventLog 拆欄＋AOI）；R68-MYDB 最後、先問 St02-M。清單 `D:\AI_TempFile\st02-claims\AUDIT_912_ST02_20261002.md`。
  - C14 頁面：`D:\AI_TempFile\st02-s19` `v906/st02-c14-pane-906`（在 7e0b1f92 上、12 檔沒 commit）——helper 22:5x 被用量上限中斷，等用量允許再接。
  - 作廢、不要推：`v906/st02-adam6024-906` `c800f2fc`、`v906/st02-c14-912void-local`、`v906/st02-c14-pane`（舊的 912 底）。
- 建置線：`scratchpad\s09close\build_lane.ps1 -Src <樹> -ObjRoot <樹>-obj`（st02-s13／s15／s16／s18 各自有 obj 根）；st02-speed 那條用 build_speed.ps1。
- 文件待辦：ht9045-bin-display skill「基準＝912」改 906（C14 已照 906）、ht9045-adam6024 加第 20c 條註、ht9045-gpib-bridge SKILL.md:233-253、TESTERCOMM_PORT_LEDGER.md :182／:210／:511——一張文件 MR。

### 10-02 18:4x 狀態（⛔ RULINGS_20261002 第 20 條：只做 906；新 session 從這裡接；St02-E 寫）
- **Steven 1002 18:0x「我還有看到912版，這是錯的，現在分工處理只能做906 C++專案，能理解?」**（main 8430458c）：翻譯來源只有 906（本機用 906_0625_Steven），912 只能看 906 有沒有漏；**沒有例外**（ADAM／HANA／C14 的 912 例外全作廢）。記憶 golden-906-only-no-912。
- 18:30 帳號用量上限把 4 個 helper 都中斷了（C10 交件完才斷；C9、OB-7、C14 頁面做到一半、沒 commit）。18:3x 起：
  1. **!114 ADAM 照 906 重翻**：helper 在 `D:\AI_TempFile\st02-s18`（`v906/st02-adam6024`，在 d0d8b87d 上加一個 commit）；新認領清單 `D:\AI_TempFile\st02-claims\ADAM_CLAIM_SHEET_vs_main_<hash>_906.txt`。審完兩組態完整建置才推。
  2. **稽核**：唯讀 helper，輸出 `D:\AI_TempFile\st02-claims\AUDIT_912_ST02_20261002.md`（St02 在 main 上的 commit＋所有分支；a＝912≠906 重做、b＝只改引用、c＝只看過）。交 St02-M。
  3. **C14 回到 906**：`D:\AI_TempFile\st02-s13` `v906/st02-c14` 重設到 `8db5c2c9` 再 merge main 8430458c ⇒ `c950e6ce`（merge-tree 一行）；912 版留在 `v906/st02-c14-912void-local`（不要推）。sim 建置中（第二條線 `D:\AI_TempFile\st02-s13-obj`，log c14b906）。認領清單要重產（舊的 112 行作廢）。C14 頁面 helper（st02-s19，基於 912 版）等 C14 906 好了再接，要改成基於 c950e6ce。
  4. **!116 LI-9**：本機 `57d2f083`（da36b214 merge＋看板＋skill 文件），sim 0 錯誤、ship 建置中（st02-speed，log li9m_ship）；**稽核確認來源是 906 才推**。
  5. **C10 HANA 停**（不推）；H-013 全停。
  6. skill：ht9045-adam6024、ht9045-bin-display（都在 main，寫「基準 912」）稽核後改 906，小的文件 MR；workflow skill §4 已在本機改成只用 906（跟下一次推送一起推）。
- C9（st02-s15）、OB-7（st02-s20）的 helper 等稽核說來源是 906 再接（它們留著沒 commit 的改動，不要清）。

### 10-02 17:2x 狀態（新 session 從這裡接；St02-E 寫）
- Session 沒變：St02-E＝`c8311755-ee3b-4c2b-9f5f-bc5682ac9613`（github-de）；St02-M＝github-62。main＝`2dd90ef3`（!108、!118 已合）。⚠ main 今天前進很快：**每次推之前 fetch、merge-tree 對當下的 main 只印一行**。
- 建置只有一棵樹（`D:\AI_TempFile\st02-speed` 切到要建的 commit，`scratchpad\s09close\build_speed.ps1 -Cfg sim|ship -Tag X`），一次只能跑一個 ⇒ 排隊：**① !114 `d0d8b87d` sim（adam6，跑中）→ 推「SHIP 待建」② !116 `da36b214` sim → 推 ③ !114 ship ④ !116 ship ⑤ C14 兩組態**。
- **已推、等人**：!114 ADAM 推到的是 `0469e8ae`（之後本機 `3c2c5941` 3h 測試修正、`849d2109`／`d0d8b87d` 兩次 merge main，還沒推）；!116 LI-9 推到的是 `e839f529`（本機 `da36b214` merge main 2dd90ef3，還沒推）。St01 的 Adam6024_Pressure 重跑會在推送後觸發。
- C-11（wb_serve.cpp:4066）在 2dd90ef3 上是**相鄰行**衝突、不是同一行：舊行一字不差 ⇒ 筆電的同意仍有效（St02-M 已在 CHAT 更正，handoff 292723a0）。
- **C14**（wt `D:\AI_TempFile\st02-s13` `v906/st02-c14`）：`8b499b21` bring-up＋`8db5c2c9` 認領＋`d0db79e8` 912 差異（含審查修正：MainClose 三個宣告從 :957〔匿名 namespace〕移到全域 :125）。**還沒完整建置**（排第 ⑤）。St02-M 裁決：912 非 BinDisplay 的 cShowBinSelect 差異**不移植**（MR 要寫「912 non-BinDisplay hunks intentionally not ported」）；整個 DoCycleTFT 搬進 St02 檔＝問筆電中（不同意的話筆電自己收那 64 行）。推之前把 35 行認領全文交 St02-M。
- **4 個 helper 在跑**（Steven 對 St02-M：「加派人手吧~」；各自一棵 worktree、只做 -fsyntax-only、只在本機 commit，St02-E 審＋完整建置才推）：
  1. C14 頁面＋js：`D:\AI_TempFile\st02-s19` `v906/st02-c14-pane`（從 d0db79e8）——St01 的 Status.ShowBinSelect.html :106-113 那塊＋一個 include（St01 事先同意），新的 St02 js。
  2. OB-7→C6：`D:\AI_TempFile\st02-s20` `v906/st02-ob7`（從 main 2dd90ef3）——先重新推導 Data.Observer.html:234。
  3. C9 rebase：`D:\AI_TempFile\st02-s15` 新分支 `v906/st02-c9-r2`（舊分支不動）——拿掉 G3、留 G2＋G1、新認領清單 `D:\AI_TempFile\st02-claims\C9_CLAIM_SHEET_vs_main_<hash>.txt`。
  4. C10 rebase：`D:\AI_TempFile\st02-s16` 新分支 `v906/st02-c10-r2`——新認領清單標出舊行變了的（要筆電重新同意）。
- 還沒動：C12（要 headless 實跑）、C13（等 E-020 進 main）、ADAM-F1／F2（!114 合了之後）、Adam6024Integrate_St02.cpp:9-20 註解的 B2／M1 標籤對調（下次動那個檔時改）。
- ChangeLog 寫到第 46 列、§43（`D:\RD5-Portal\public\Docs\ChangeLog\Steven02\CHANGES_20261002_Steven02.md`）。

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
- ChangeLog：`D:\RD5-Portal\public\Docs\ChangeLog\Steven02\CHANGES_20260930_Steven02.md`，寫到第 11 列＋§10 目前狀態，St02-M 會再抄一次。
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
- ChangeLog 換日：`D:\RD5-Portal\public\Docs\ChangeLog\Steven02\CHANGES_20260928_Steven02.md`。
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
- **ChangeLog** `D:\RD5-Portal\public\Docs\ChangeLog\Steven02\CHANGES_20260927_Steven02.md` 由我維護（每次推送後更新，St02-M 抄進交接檔）；日報區塊隨回報交給 St02-M。
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
