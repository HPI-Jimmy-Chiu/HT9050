# 機台端 patch 整合 第三輪（cpp 0047～0129、web 0045～0082）

> 接在第二輪（`docs/MACHINE_PATCHES_20261001.md`，cpp 0045～0046、web 0034～0044、tools 0001～0109）之後。
> 裁決：RULINGS_20260930 第 11 條「一切按照機台建議」，衝突以機台為準；使用者 1002 09:5x「機台端一旦有更新，必須立即性，他那邊是測試端，可靠性高」
> 「未來夜間迴圈要優先處理機台端的」「整合完機台端版本後，要推到lab and hub」。
> 分支 `v906/jimmy-mach1002`（worktree `D:\HT9045\.claude\worktrees\mach1002`），從 origin/main `36f09560` 開。

## 0. 來源與做法

| 項目 | 內容 |
|---|---|
| 來源 | GitHub `machine/integ-ioweb`：上一輪收到 cpp 0046／web 0044（1001 01:16）。這一輪到 1002 10:39 `8ae0cb8`：cpp 0047～0129（83 顆）、web 0045～0082（38 顆），共 121 顆。機台自述＝「GitHub main 15895cf（第 123 包）＋機台改動（C++ 09ba9e9、web b1f0d95）」——**機台沒套過第 124 包** |
| 重建鏈 | `resume_20261001/mach_chain.py` 延伸（暫時 index＋`commit-tree`，不 checkout，作者／日期／說明照 patch；每一段 diff 依鏈尾那個檔是 LF 還是 CRLF 決定要不要轉）：**C++ `e5dd8d6c` → `a64c46ff`**（0047～0129）；**web `dee34e8d` → `fe231709`**（0045～0082）。0073（`Setup.Configuration.html` 存成 CRLF）第一次套不上，加了逐檔換行判斷後重跑通過 |
| 先撤第 39 批 | 筆電第 39 批（第 124 包：Teach In／Out Z All Up、Out Z All Down、Set All In／Out Arm Z）跟機台 TEACH-ZALLUP（cpp 0093、web 0064）同檔同功能 ⇒ `f511186c` 撤回（8 個檔回到 `f41916a2^`，逐檔比對 0 差異），讓機台的 commit 疊在它寫的那棵樹上 |
| 套法 | 逐顆 `git cherry-pick`（真正的三方合併）：**收 96 顆**（其中 WORKLOG 37 顆）；**不收 22 顆**＝PKG-*／MERGE-* 21 顆（機台合筆電包，筆電本來就有）＋cpp 0127 F5-EXTCON（只改機台的 `.vscode/launch.json`，機台 10:39 的通知說 main 不用收）；**保留 3 顆待裁決**＝LOGIN-HONPREC（cpp 0066）、TEACH-KB（cpp 0117、web 0078）。清單 `resume_20261001/_mach1002_picklist.txt` |
| 核對 | 整合結果 vs 機台最終樹（兩條鏈尾）：**web 只差 TEACH-KB（`ht9045_teach_kb_c.js` 與 `HW.teach.html` 載它的那一行）**；C++ 33 個檔有差，逐檔都有原因（§2）。比法：`git diff <鏈尾> HEAD`＋逐行多重集合比對（`resume_20261001/linediff.py`，分得出「搬位置」與「真的不同」） |

## 1. 衝突（逐處）

| 檔 | 機台 commit | 兩邊各做了什麼 | 怎麼合 |
|---|---|---|---|
| `JsonBridge/IoBtnPanelClick.cpp` 檔頭 (h) | IOWIDGET-3（0048） | 兩邊都改 (h) 那段註解 | 用筆電版（INBOX 132 更正＋VC8 閘）——機台最終樹也是這一版 |
| `WebMotorAccessLive.cpp` | HT9050-ORG（0050） | 機台加 `g_W906OrgActiveLow` 一行 | 照機台（＝機台最終） |
| `tests/CMakeLists.txt`（4 次） | 1203-LINKLOST、HOMEMON、compK dataS、NOTE-SCREENSTART | 兩邊都在檔尾加測試 | 都留、沒有重名；HOMEMON／compK 那兩次 git 把共同的 4 行（LINK_GROUP／webbridge／`)`／`if(WIN32)`）對齊在中間，拆開成兩個完整的區塊（`resume_20261001/res_two_appends.py`）；NOTE-SCREENSTART 那次夾帶的 `test_teach_kb` 區塊不收（TEACH-KB 保留） |
| `WebBridge/WebBridgeServer.cpp` 免 token 清單 | HOMEMON（0094） | 筆電 `dialog.auth`（St01 D-026）；機台 `act.home.abort` | 兩個都留（＝機台最終那一行）；TOKEN-OFF 照舊不收 |
| `tools/wb_serve.cpp` 告警框開著時的分派 | HOMEMON | 筆電 `dialog.auth` 分支；機台 `act.home.abort` 分支 | 兩個都留（＝機台最終那一行） |
| `cObserver.cpp` 檔尾 | compK dataS（0105） | 筆電 St01 D-043（`W906_ObsFactoryNoteNeeded`）；機台 OBS-TABVIS（`W906_ObserverTabVisibleJson`） | 照機台最終的順序，兩段都留 |
| `CMakeLists.txt` wb_serve 來源清單同一行 | SETUPA-N04／KB（0107） | 筆電 `LotInfo_SECSLotStart.cpp`；機台 `IniConfig_N04.cpp`、`DeviceForm_KbExtra.cpp` | 都留；機台最終那一行的 `TeachKb.cpp` 與它的註解不收 |
| `web/page/HW.teach.html` | TEACH-HIDEAXIS（web 0057）、TEACH-HIDEPT（web 0063） | 筆電第 38 批把 `TEACH_UNWIRED_B38` 同一行接在 `];` 後面 | 先照筆電同一行留著；最後照機台自己合第 123 包的作法（MERGE-123：B38 清單獨立一行、拿掉機台已經接好的 6 顆 btnIn/OutZAllUp、btnZ1/Z2Servo、btnArm1/2YServo）整檔對齊機台最終（`2559f550`）——不對齊的話，第 38 批的灰鈕清單會把機台接好的 6 顆變灰 |

換行：這台 `core.autocrlf=true`，工作樹是 CRLF、repo 存 LF；從機台最終樹拿來的行先補 CR 再存（`final_block.py`），`git add` 轉回 LF。

## 2. 跟機台最終樹還差什麼（都是刻意的）

| 類 | 檔 | 原因 |
|---|---|---|
| 保留待裁決 | `MachineType.h`（`W906_LOGIN_HONPREC_ALWAYS`）、`WebLogin.cpp`、`FileRW/Main_A01AutoLogout.cpp`、`tests/test_d015_a01_autologout.cpp` | LOGIN-HONPREC：出貨組態開機就 HonPrec、A01 閒置不降級；機台 commit 自己寫「交機前一定要註解掉」；跟 RULINGS_20260925 第 4 條（真機照 golden Operator，被否決的就是這種開發期開關）相反 |
| 保留待裁決 | `FileRW/TeachKb.cpp`、`FileRW/Teach.cpp`、`CMakeLists.txt`、`tests/CMakeLists.txt`、`tests/test_teach_kb.cpp`；web `ht9045_teach_kb_c.js`、`HW.teach.html` 一行 | TEACH-KB：EastSun 1002 裁決 C（照 golden 9046LS 臂的範圍；機台實測這台 11 個在用的位置會被夾，點開小鍵盤按 OK 或 Abort 都夾）vs 使用者 1002 09:1x 第 44 項 **A**（Teach 先不夾、之後用馬達軟體極限） |
| 機台本地暫時設定（照舊不收） | `WebBridge/WebBridgeServer.cpp`／`.h`、`tools/wb_serve.cpp` 3 行、`MachineType.h`（`W906_WEB_TOKEN_ENFORCE`） | TOKEN-OFF（0018） |
| 〃 | `cinitial.cpp` | TEMP-DOORS（0014／0015） |
| 機台沒跟上筆電（更新包本來就排除） | `.vscode/launch.json`（另含 0127）、`build.bat`、`tools/pe_truncation_check.ps1`、`tools/webprobe/f5_contract_probe.cjs` | `mkpkg` 的 EXCLUDE；機台用自己的 F5 設定（0925 的 OBJROOT 沒進機台） |
| 機台上的殘檔 | `FileRW/TestIF_File.cpp`、`JsonBridge/gen/form_TfDIOFrom／TfHotPlate／TfSpeed／TfTrayAssignment.gen.cpp`、`form_registry.gen.cpp`、`tools/formbridge/` 3 支、`tools/gen_formjson.py`、`tests/test_formbridge_testerif.cpp` | 筆電 0926／0927 已刪（`f89be4ce`、`834fcc78`）；更新包只複製不刪（`deleted_in_main.txt` 有列），機台上還在；不在建置清單 |
| 只差換行／順序 | `ship/Error/AlarmCodeList.txt`、`ship/.gitattributes`（逐行相同，只差 CRLF）；`tools/wb_serve.cpp` 檔尾兩段的順序（多一行分隔線）；`cStateRecord.cpp` 兩行註解（第一輪刻意保留，MACHINE_PATCHES_20260930 §0） | 程式碼相同 |
| 筆電在第 123 包之後的 | `docs/NIGHT_REPORT.md`、`docs/RULINGS_20261002.md`、本檔 | 文件 |
| HTDESIGNER | `tools/vscode-htdesigner/` | 重建鏈不含 tools；筆電 0.157＝機台 tools 0144。機台 tools 0136（138a：Alias 清單略過 IO 表 `#...` 列，`lib/aliasedit.js`＋測試）與 0137（HANDOVER.md）不在 main、也不在 ES02 分支 ⇒ RULINGS_20261002 第 14 條（外掛只由 ES02 改）⇒ 轉給 ES02（TO_ES02 §4） |

## 3. 這一輪收進來的東西（白話）

- **wb_serve 開機 10～20 秒就自己結束**（cpp 0128 OBS-TEMPSERIES）：第 116 包起每分鐘的溫度紀錄會畫 Observer 溫度圖，那張圖開機時一條曲線都沒建，`Series[0]` 丟 `std::out_of_range` ⇒ terminate。照原版在第一次用到時補建曲線與下拉選項。web 0082 TAGS-RECONNECT：主框架斷線每 2 秒重連。
- HT9050／1203：原點判斷改用 1203 ORG（LOW＝在原點；HT9050-ORG／ORG-ENG）；ECAT-VC4-ODM1 獨立分支（4 通道、奇數 DO 吸偶數 DO 破、閥值 SDO 寫入打開；VC4／VC4-SDO／VC4-ODD），VC8 站號用吸嘴列自己的 IP（VC8-IP-2）；1203 斷線 10 秒跳 WAR16152（1203-LINKLOST）；BUSY 軸也能關程式（CLOSE-BUSY）；Enable=0 的非 1203 軸不擋 Teach 的 Z 檢查（TEACH-ZDISABLED）。
- Teach：照原版 FormShow 顯示（TEACH-FORMSHOW）、藏掉沒有的軸（TEACH-HIDEAXIS／HIDEPT）、激磁教導預設（TEACH-SVON）、Home OK 燈（TEACH-HOMEOK）、Set／Go 先標格子（TEACH-GOHINT）、動作中鎖定（TEACH-LOCK）、Speed Adjust（TEACH-SPEEDBAR）、換軸 JOG 速度（TEACH-SPD-AXIS）、Z All Up＋Index Servo（TEACH-ZALLUP）、Tray Z 教導點（TEACH-TRAYZ）、三軸教導點（TEACH-3AXES）、1,491 元件逐一檢查（TEACH-UNWIRED-2／TYPED／ALLCOMP）。
- Home Monitor 真的資料＋Abort Home（HOMEMON）；IO 頁（IOWIDGET 1～3、IOSV-FORMSHOW、IO-FORMSHOW-OUT）；Vacuum Unit（VACUNIT-OPENFAIL／NOCONFIRM／FIT）；Motor Test（MT-SPDLIVE、MT-SAVEMOT-2、MOTORVIEW-LED）；警報 Note（NOTE-KEYGATE、NOTE-SCREENSTART）；不停機小窗（NONSTOP-SEM）；全站元件檢查（COMPK-DATAS／SETUPA／MOTORB／IOS／KB-MERGE，約 17,800 個元件）；截掉修正（CLIPFIX）；選單（MENU15-CRISP、MENU-MINGLIU）；F5 等舊程式關完（F5WAIT；F5-PROGRESS 的 tasks.json 那半機台自己撤回了）。

## 4. 跟筆電還沒推的批次重疊

- **第 40 批**（`v906/jimmy-b40`，Teach 選項分頁照 golden TabVisible）vs 機台 TEACH-FORMSHOW／TEACH-TABS（C++ 算 golden FormShow 的畫面半邊，含 TabVisible）⇒ 停，比對後再決定，機台已經做的不再做一份。
- **第 41 批**（`v906/jimmy-b41`，通知框確認照 golden 暫停、INBOX 123）vs 機台 NOTE-KEYGATE／NOTE-SCREENSTART ⇒ 停，比對中。
- 第 39 批撤回；Out Z All Down、Set All In／Out Arm Z 照機台的 TEACH-ZALLUP 結構重做（另一批）。

## 5. gate

b42p 兩組態全新 11:37～12:15（HEAD e7fef6ac）：出貨 332 支 4 項、模擬 332 支 19 項，失敗集合逐名＝基準；執行期資料夾前後 0／0／0；absence 3 個已知名稱；numcmp PASS A=2 B=11 B′=3 C=30、未覆核 0

## 6. 下一次

- 從 **cpp 0130／web 0083** 開始；C++ 鏈尾 `a64c46ff`、web 鏈尾 `fe231709`（tools 另計：0136／0137 轉 ES02）。
- GitHub 第 125 包以第 123 包（`8c1afb11`）為底：第 124 包作廢（機台沒套過）。

## 7. 增補（1002 下午，機台推了新的）

| 機台 patch | 內容 | 怎麼收 |
|---|---|---|
| web 0083 HSYS-ENTRY（13:38） | 底部機況列的「⚙ Handler System」入口重新看得到（0930 LAYOUT110 把每頁底部的程式對應說明列藏起來，入口剛好在那一列），搬到 OCR 那一行右邊 | web 鏈尾 `fe231709` → `4c34e9f4`；cherry-pick 進第四十四批 |
| web 0084 GRIDKB（14:05） | Motor Database 與 IO Table 表格的小鍵盤照 golden：名稱欄只能英數字、GearRatio／Acc／Dec 小數、其他整數（以前每一格都開完整英文鍵盤，機台 M37 MColorZ 的 GearRatio 被打成 `0.0.071425`，程式讀成 0） | web 鏈尾 → `78ee7510`；第四十四批的 gate 才跑 9 分鐘，停掉、併進來重跑（b44b） |
| tools 0145 HTDESIGNER-157a（13:43）＋請筆電收 tools 0136／0143／0145 | 設計外掛：0.140 的版面開 C++ 檔會關掉設計頁，之後「跳到程式碼」讀到已關的頁面而中斷（Webview is disposed），已修；第 4 層時限 180→600 秒 | 照 RULINGS_20261002 第 14 條轉 ES02（TO_ES02 §4 13:5x，main `862c2ed1`）；第 126 包的機台通知重申「外掛改動走 ES02」 |

下一次從 **cpp 0130／web 0085／tools 0146** 開始；C++ 鏈尾 `a64c46ff`、web 鏈尾 `78ee7510`。
