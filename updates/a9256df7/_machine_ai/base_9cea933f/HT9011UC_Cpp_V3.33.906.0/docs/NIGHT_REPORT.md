# 夜間報告 2026-10-02（五）晚 → 10-04（日）—— 進行中版本（最後更新 1004 18:3x）

> 這份從 1002 22:00 起算，整份重寫。0929 晚～1003 10:1x 的舊版（207 KB）在 git 歷史：`git show b6acab52:HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md`。
> 讀法：§0 只放要你決定的；§1 做完的都附 commit 或數字；§2 還在做或刻意沒做的；§3 哨兵抓到的；§4 清理；§5 推送清單；§6 我自己的錯。

## ★ 接手

現在 GitLab main 的程式＝**第 143 包 `62074fbd`**（1004 15:0x），GitHub 機台更新包從 1002 晚到現在出了十五包：129 `aab3911`、130 `75c49fd`、131 `19b954b`、132 `a071554`、133 `04237aa`、134 `d45bcdd`、135 `822b6b5`、136 `004a93a`、137 `368dc68`、138 `8f34133`、139 `79274e8`、140 `088a141`、141 `e7b00fa`、142 `bc7174c`、**143 `da1f2b0`**。§0 剩 6 題：#98（新：NB2-1 U19 F5 開機期間兩小題，預設照現在）、#97（新：ES02／Frank01 追兩次沒回）、#96（新：1203 卡片命令失敗要跳哪個警報碼）、#91（Steven：10/05 18:00 前只記錄、之後裝置 ERROR 就整台不能動，要推翻才回）、#94 安全門、#95 Ifor01；#89／#90／#92／#93 Steven 07:2x 已代回、都照建議。同事手上的工作見下面三步。

| 項目 | 現在 |
|---|---|
| GitLab main | 程式＝第 143 包 `62074fbd`（1004 15:0x）＝第 62 批 b62a（gate 綠）：St02 !176（Observer SG 頁狀態查詢免權杖）、St02 !179（TfTemperFrom 建構子照 golden 清零）、NB2-1 !178（F5 建置加速，選用）；之前：第 142 包 `65c533be`（13:3x） |
| GitHub 機台包 | 129 `aab3911`（02:29）、130 `75c49fd`（07:53）、131 `19b954b`（09:52）、132 `a071554`（13:2x）、133 `04237aa`（17:16）、134 `d45bcdd`（22:0x）、135 `822b6b5`（1004 00:1x）、136 `004a93a`（01:57）、137 `368dc68`（04:35）、138 `8f34133`（06:10）、139 `79274e8`（07:37）、140 `088a141`（10:2x）、141 `e7b00fa`（11:4x）、142 `bc7174c`（13:3x）、**143 `da1f2b0`（1004 15:0x）**；README 機台通知另推多次（派工 1～8 的答覆、St01 V-6、State Record 常設規則、St01 急停／START、門檢）；舊包不動 |
| 第 132 包（✅ 13:2x 已推） | 共用快時鐘（RULINGS_20261002 第 7 條：加熱 20 ms、Bin 號碼面板 30 ms、GM-2 30 ms）；Teach Set All In／Out Arm Z＋Out Z All Down（All Down 在任何一軸在動時拒絕）；訊息框也停 Motor Test HOME／LoopMove（INBOX 150）；Gear Ratio 照 NB2 R171 修；St02 c912-1～4 改回 906（含拿掉 GPIB 遠端 START ⇒ START 普查 **35／33／2**）、C14 網頁頁、ADAM A3；St01 review6 E-030 第一部分；機台 WORKLOG 86-87 |
| 第 133 包（✅ 17:16 已推） | 機台建置改 -O0（第 18 條）＋整棵樹 `-fexcess-precision=fast`（第 7 條）；Arm Cell 第一次讀到 Z 離開原點就停 X／Y（R174 M1）；`CommaText` 照 BCB6（INBOX 152）；**Arm Cell 走或手動教導時任何來源的 HOME 都擋**（NB2 !146＝U14 ③）；**GO 時舊通知框開著就拒絕**（NB2 !147＝INBOX 151）；St02 !140 OB-7；Ifor !142；NB2 !143／!144／!145；St01 review6 到 `65323419`（引用整理、拿掉 912 才有的 Barcode CSV Compare 在 St01 的檔）；機台自己的 cpp 0144／0146～0149、web 0088／0089／0091～0093（含 JOG-MAXVEL、MT-COPYALL） |
| 第 134 包（✅ 22:0x 已推） | GitLab `9d8ad319`／GitHub `d45bcdd`（63 檔）：NB2-1 !151 **Gear Ratio 手推量測**（Jimmy 第 8 條）＋St01 審查 M1 的修正（推過沒存就結束也設 HomeFlag 0、頁面寫「手離開軸、站開」）；GearRatioCal.csv 預設寫機台 log 根目錄（第 22 條）；St02 !149 W71／W72／W73（保留 912，第 23 條）；機台自己的 cpp 0151／0159、web 0095／0098；ES02 HTDESIGNER 0.163.0；St01 E-031（不改行為）。gate：b52c 出貨＝基準 4、模擬＝基準 19＋WebMotorAccess 負載逾時（單獨 1.5 秒過）；PE 檢查兩次被防毒卡住、手動停掉後在完整的 build 目錄跑 ctest。README 寫了第一次用手推量測只推 5 mm、手離開再伺服 ON（E-08） |
| 第 135 包（✅ 1004 00:1x 已推） | `v906/jimmy-b53` `fe11b6ae`：NB2-1 !152（Arm Cell Z 警報）／!153（CommaText 後續）、St02 !155（S-09，測試 exe 改名去掉 setup——NB2-1 R199）／!150（C14_BinDisp，NB2-1 R198 代跑綠）、NB2-1 !159（PE 檢查開檔逾時＝鎖住只警告，今晚卡了 3 次的那個）、機台 cpp 0160～0179（1203 模組檢查、HOME 修正；部分用 -C2／手動接上，逐段比對落點；St01 V-6 的 3 個中度安全問題見 §0 #87）、合 main（第 134 包）。**22:3x 更新**：gate b53a 出貨組態建置停在一個連結錯誤（`test_mt_savemot` 找不到 `vclcompat::CommaTextCells`：!153 只幫 test_web_motor_access 連了 vclcompat，另外兩個直接編 WebMotorAccess.cpp 的測試沒連）⇒ 筆電補 `6d26d7c5`；同一個 build_ship 增量建完（67 步 0 錯誤），新版 PE 檢查 315 個通過、30 個被防毒鎖住只警告、沒有卡住。接著：出貨組態手動跑完整 ctest、模擬組態全新 gate（b53s），約 00:00 跑完。**已推：GitLab `5d6a27ae`（00:08）／GitHub `822b6b5`（45 檔）**。gate：出貨 392＝基準 4＋FShow_Audit（已修、重跑過）；模擬（全新）392＝基準 19＋WebMotorAccess 負載逾時（單獨 1.54 秒過） |
| 第 136 包（✅ 1004 01:5x 已推） | `v906/jimmy-b54` `b491816f`：NB2-1 !158（煞車放開／擋住寫進 BRAKE 紀錄，只加紀錄）、!159 後續（自測計時加寬、PE 檢查兩階段）、St02 !160（C16：SECS Use Die Force 照 V912）、Ifor01 !162（溫度條「有沒有裝」先單獨上線）；St02 !161 等 NB2-1 代跑、NB2-1 (m) 放煞車條件做好再進下一批。**已推：GitLab `519a9feb`／GitHub `004a93a`（14 檔）**。gate：出貨 392＝基準 4；模擬 392＝基準 19＋WebMotorAccess／PlcGates 負載逾時（單獨重跑 1.5／7.6 秒過） |
| 第 137 包（✅ 1004 04:3x 已推） | `v906/jimmy-b56`（第 55 批併入）：NB2-1 !163＋補修正（煞車只在上電完成而且這次上電後送過 Servo ON 才放；HOME 302 等煞車真的放開、300 給 Index Z1 送 Servo ON、煞車扣著的軸拒絕移動）、St01 !164（S-17：被取代或放棄的對話框回答後會關；dialog-bridge 1.3.1）、St02 !157／!166（註解）、NB2-1 !167（PE 檢查）、筆電哨兵分類。**已推：GitLab `8c9c27c5`／GitHub `368dc68`（30 檔）**。gate：出貨 394＝基準 4＋TesterComm_HomeJson／St02_MainTimers 負載逾時（單獨 0.75／0.77 秒過）；模擬 394＝基準 19 |
| 第 138 包（✅ 1004 06:1x 已推） | `v906/jimmy-b57`：NB2-1 !169（HT9050 上 Galil 路由沒有的 Index 軸，移動一律立即完成；修好 Z2 的移動把 Z1 鎖死）、St02 !168（計時器排程表核心上線，Timer3／Timer8／TimerESD 接空殼，行為不變）。**已推：GitLab `703effd1`／GitHub `8f34133`（20 檔）**。gate：出貨 396＝基準 4；模擬 396＝基準 19＋WebMotorAccess／WB_WsLink 負載逾時（單獨重跑都過） |
| 第 139 包（✅ 1004 07:4x 已推） | `v906/jimmy-b58`：NB2-1 !172（Servo ON 之後照 golden `ServoOnOff` → `PCIL132_ResetPos` 把命令位置設成編碼器位置；網頁 Motor Test／Teach 與引擎那條路都做；EastSun 要回 W-47：Servo ON 那一瞬間驅動器會不會先朝舊命令位置動）。**已推：GitLab `54505e0b`／GitHub `79274e8`（14 檔）**。gate：出貨 396＝基準 4；模擬 396＝基準 19＋WebMotorAccess 負載逾時（單獨重跑 1.94 秒過） |
| 第 140 包（✅ 1004 10:2x 已推） | `v906/jimmy-b59b`：NB2-1 !170（Steven #92＝A：HT9050 上 `Z1UpZ2Down` 等四支成對下壓改成報警、回失敗，不再假回「完成」）、St02 !171（S-09：開機後 OCR 狀態格馬上照工單顯示）。**St02 !165（面板實體按鍵 ScanKey＋N07 橫幅）這包沒收**：第一次的 gate b59a 出貨組態抓到 `FShow_Audit` 紅——`WebMainScanKey.cpp:289`／`:425` 兩處直接讀 `fHome->fShow`（基準 0）；那是 St02 的檔，改法由他定（TO_STEVEN 08:1x），b59a 停掉、改用 b59b 重跑。**已推：GitLab `3ffb2b00`／GitHub `088a141`（14 檔）**。gate b59b：出貨 396＝基準 4＋cJSON（08:23 被 F-Secure 誤判隔離成 Not Run，重新連結後單獨過）；模擬 396＝基準 19＋WebMotorAccess 負載逾時（單獨 1.58 秒過） |
| 第 141 包（✅ 1004 11:4x 已推） | `v906/jimmy-b60`：St02 !165（機台自己的 SCANKEY 0152＋St02 的修正：前／後面板 START／PAUSE／HOME／RESET 等照 golden `TfMain::ScanKey`，面板 START 跟網頁 START 走同一條路；N07 SECS 斷線橫幅）、St01 E-034（golden 0618 的「`==` 寫成沒作用」照 0625／V912 修好：RFID 讀不到設 NOREAD、RT 自動關 socket 真的會關、OCR 送 LOT 前清 ocrLot 旗標等）、St01 E-038 Phase A（HT9050 Index Z1 扭力改從 1203 的 6077h 讀；`[IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED` 預設 0，EastSun E-10 量完才設 1）、St01 review6（S-16 開機只載入看得到的視窗、B69 每軌 Auto ART、E-035 代碼 982 的 Visual Detection Test、D-026 拿掉 V912 才有的 WAR04217 規則、E-031 九個設定檔的標籤照 0618）、NB2-1 !175（#93＝A：HT9050 開機照 Mot_Table 決定 Index 四軸存不存在，沒有啟用列的軸 Enable=false）。**已推：GitLab `29ef9e3c`／GitHub `e7b00fa`（202 檔）**。gate b60a：出貨 403＝基準 4；模擬 403＝基準 19＋WB_WsLink 負載逾時（單獨重跑 17.5 秒過） |
| 第 142 包（✅ 1004 13:3x 已推） | `v906/jimmy-b61`：St01 E-043 第 1 顆（Steven Q96＋07:4x：開機讀完 IO_Table／Mot_Table 照 12 條規則檢查，主控台一行「[BOOT] 驗表：ERROR n／WARN n」＋每個發現一行；**10/05 18:00 前只記錄、不擋**；機台昨晚的表＝ERROR 0／WARN 44／INFO 1；EastSun 上機看 A63）、NB2-1 !177（機台派工 4：F5 期間在等待頁按 X 確認後，還沒讀設定檔就不啟動、已在跑就正常關閉，視窗不再自己跳回來；只對 F5 有效）。**已推：GitLab `65c533be`／GitHub `bc7174c`（27 檔）**。gate b61a：出貨 405＝基準 4；模擬 405＝基準 19＋WebMotorAccess／TesterComm_HomeJson 負載逾時、GaliRouteLive 負載下失敗（單獨連跑 3 次都過）、B8_Ag1_AgvIni 啟動時 exe 被鎖（單獨過） |
| 第 143 包（✅ 1004 15:0x 已推） | `v906/jimmy-b62`：St02 !176（OB-7 後續：唯讀的 `act.observerSG.state` 不用操作權杖，`queryNow`／`queryYesterday` 照舊要）、St02 !179（TfTemperFrom 建構子把狀態格的 OnOff／bFlag／iCount 清成 0，跟 golden 表單一建立就清零一樣）、NB2-1 !178（機台派工 5：CMake 選項 `W906_FAST_INCREMENTAL`，預設關；機台要快就把 F5 建置目錄重建成 Ninja，NB2 量改一個檔約 9 秒）。**已推：GitLab `62074fbd`／GitHub `da1f2b0`（24 檔）**。gate b62a：出貨 405＝基準 4；模擬＝基準 19＋GaliRouteLive 負載下失敗（單獨 1.3 秒過） |
| 機台 State Record（你 1004 00:0x） | 已提醒機台：有問題一定先按主畫面的 State Record（HOME 卡住在 Abort 之前按），資料夾推到 GitHub `dispatch/`（README、TO_ES02，W-40）；已派 St02 檢查 State Record 對今晚三個實例夠不夠分析、不夠就補（S-24，W-39；09:00 前沒開工改派 NB2-1）——RULINGS_20261004 第 1 條 |
| 機台端派工 1～8（EastSun 1003 透過 Jimmy，GitHub `machine/integ-ioweb` README.txt） | ①Index 自動測高在 1203 上能不能用（⚠ 安全：門檻不觸發 Z 會一直往下壓）→ NB2-1（先唯讀分析＋列機台要量的）；②機台螢幕上被遮住／截斷的視窗清單＋修正 → ES02（E-09）；③4 個吸嘴吸一顆 IC → Frank01（F-03；週一 10:00 前沒認領改派 NB2）；④F5 期間關 HMI 視窗程式不停、視窗又跳回來 → NB2（預設：關窗＝不啟動／正常停掉 wb_serve、不再開窗）；⑤機台編譯太慢（改一個檔 61 秒，編譯只佔 3 秒）→ NB2（不碰防毒、不換 6.3.0）。機台自己的 cpp 0160～0169（1203 模組檢查、HOME 修正）排第 53 批，先請 St01 唯讀審查（W-28）。**20:4x 更新**：①NB2-1 R197 交了（今天不會動 Z，安全；照 golden 翻會卡在 555；危險的是把 6077h 直接接上）⇒ TO_ES02 E-10 量五項、§0 第 86 項；**⑥（新）開機後馬達好像激磁又放開** → NB2-1 (l)（機台端查到：(A) HOME 後照原版切電源約 4 秒、(B) 開機煞車放→鎖→放、(C) HOME 失敗 PAUSE 斷電，都照 golden；翻譯缺口是開機 Servo ON 沒到卡）；機台又推了 cpp 0170～0179（HOME 修正），第 53 批開 gate 時一起收。**23:5x 更新——派工 7（新，23:29）**：EastSun 說今晚整機回原點一直出問題，請全面檢查開機／HOME／所有功能對照 HT9050 現用的 IO_Table／Mot_Table（例：step 1520 在等這台根本沒有的 MTestZ2，23:22 卡住；MColorZ 的 GearRatio 寫成 `0.0.071425`）。分給 St01（表的載入、1203 站號、全功能普查、氣缸／感測器規則、開機驗表，S-23）與 NB2-1（上電／煞車／HOME 全部 case／Galil 路由、馬達類別層，(o)）；回覆包照機台要的格式；⑧（1003 23:53，**筆電 01:0x 才看到**）全功能逐一對照 PCI-1203 的卡片行為（`dispatch/20261003_functions_vs_1203/`）→ St01 S-26（大部分功能列＋open items 1／4／9／10）＋NB2-1 併進 (o)（Index、Servo／煞車／EMG、單軸 HOME、§5 結構題、open items 2／3／5／6／7／8），St01 組一份 FINDINGS.md 回機台；派工 7 第一部分（St01 A1／A2／C5）已回，5 題請 EastSun 確認（W-43），C5 要你決定（§0 #91）；**1004 07:1x 再轉一則 St01 派工 8 先期提醒**（README `d624c68`）：機台的 TRAYSAFE 補丁讓 Tray Arm 左右邊防呆在自動運轉也失效；teach 還是 0 時不要按 START；HT9050 第一次 START 會一直停在 DoTestHeadMotor 12110 等 Index Z1 扭力（要等 E-038＋EastSun 的 E-10 量測）；扭力不取絕對值；**1004 08:1x 派工 8 完整回覆（St01 S-26，161 項，08:04 交，比預計早三小時）已轉 GitHub**（README `5757435` 紅字＋knowledge 鏡像）：Tray Arm 移動時 OutArm 沒被擋（碰撞風險，修好前兩個不要同時跑）、移動中驅動器警報在自動運轉不會升成警報、「Z 安全」看原點燈；M03／M22／M41／M42／M108 伺服列 ServoAlarmOn=0 要改表；附錄 A §Q 12 題＝W-48 |
| 等你 | **§0 剩 6 題**：#98（新，NB2-1 U19：F5 開機期間兩小題，不急、預設照現在）、#97（新：ES02 W-26／W-30、Frank01 W-27 追兩次沒回）、#96（新，St01 S-26：1203 卡片命令失敗要跳哪個警報碼）、#91（開機驗表：Steven 改成 10/05 18:00 前只記錄、之後 IO／馬達裝置有 ERROR 就整台不能動——不推翻就照這樣）、#94（安全門）、#95（Ifor01 沒回）。**#89／#90／#92／#93 Steven 07:2x 已代回、都照建議**（前例：#86 Steven 代回）——#92＝先擋（!170 進第 59 批），真正的修法改成移植 V910 的 DoTestHead（筆電派給 St01）；#93＝開機照 Mot_Table（NB2-1）；#89／#90＝State Record 卡住 60 秒自動錄一次、對話框開著也能按（St02 S-24） |

**接下來三步**
1. **下一批（第 63 批）候選**：**⚡ Teach 數值輸入（使用者 14:1x 插單，最優先）：網頁小鍵盤照 golden（KB-GOLDEN，b19 `v906/jimmy-b63`；EastSun 的卡 E-12、請回 W-49）**、St02 !173（cpp_build 技能文件，只有文件）、St02 !180（計時器表 MR-B：St02 分派器的五個 TfMain 計時器搬進計時器表）、St02 !174（golden 的 1 ms 計時器解析度）——這兩張 NB2-1 13:4x 起在代跑；St02 !161（S-22，NB2-1 代跑中）、St01 review6 後續（E-036 (a) 隱私註解）、St02 S-24（State Record，等 Steven 在 St02 那台核准共用檔修改）；**10/05（一）白天的目標是機台安全地動起來（Steven）**；10/05 18:00 之後才收：E-043 第 2 顆（裝置 ERROR 就整台不能動）、V910 DoTestHead 移植（#92 的真正修法）。
2. **等機台／EastSun**（GitHub README 已列「請回」；機台從 1003 23:53 之後沒有推送）：W-48（S-26 附錄 A §Q 12 題，含伺服列 ServoAlarmOn=0 要改表）、W-47（Servo ON 瞬間會不會動）、W-46（實際有哪幾道門）、W-45（急停接在哪）、W-44（Index 機構三題）、W-43（派工 7 第一部分五題）、W-32／W-33（HOME 安全問題、開機煞車量測）。
3. **等同事**：St01：S-26 的程式後續要不要接（R3 Tray Arm 到位燈、R1 Z 安全、R4 全軸警報掃描；TO_STEVEN 08:1x 問了）、V910 DoTestHead 移植（#92 的真正修法，St01、跟 E-038 Phase B 一起，10/05 18:00 後）、開機驗表 E-043（#91：先只記錄，10/05 18:00 後才擋）；St02 S-24（State Record 補強；Steven 定 #89／#90＝A，共用檔修改由他自己在 St02 那台核准）；NB2-1：#93 開機照 Mot_Table 設 Index 四軸（C1-4）、!161 代跑、(p)；ES02 E-09／W-30（追 2 次）、Frank01 F-03（追 2 次，週一 10:00 前沒認領改派 NB2）、Ifor01 I-08（§0 #95）。

---

## §0 要你決定的（分流表）

分流：🔴 上機前先看／🟡 新功能的設計小題／⏳ 等你說寄出或等別人回／「舊題」＝之前留下、還沒回的。號碼沿用原本的（別的檔有引用）。

| # | 事項 | 白話＋例＋選項（建議） | 你回之前的預設 | 在哪 |
|---|---|---|---|---|
| 103 | 🟡 **Teach 頁「Set To Offset」在還沒按 HOME 時，要不要擋**（KB-GOLDEN 2/2 覆核；golden 不擋） | 白話：「Set To Offset」是把 HOME 量到的原點值抄進目前選的教導點。golden 不管那一格有沒有值都照抄（uteach.cpp:2202）——還沒按過 HOME 時那一格是空的，抄過去之後按存檔，那個教導點會變成 **0**，之後 Go 就會走到 0。第 63 批已照 golden 補上「HOME 做完自動填值」，抄空值時狀態列會跳警告，但**照 golden 還是會抄**。選項：A＝照 golden（現在這樣，只警告）；B＝空的就不抄、不送，狀態列說明（golden 沒有這道防呆）。**建議 B**（少一個「存成 0」的機會；只差在 golden 本來就會寫壞的情況）。 | A（照 golden，只警告） | `web/page/HW.teach.html` btnSetToOffset；`tools/webprobe/teach_kb_golden_selftest.cjs` 第 4 段 |
| 101 | 🟡 **HT9050 自動運轉中，1203 軸驅動器跳警報時要顯示哪一個警報字**（St01 E-045＝S-26 R4，FROM_STEVEN §3 1004 15:40；修法計畫已做好，誰做／何時做等 Steven） | 白話：現在自動運轉時，1203 的軸如果在移動中跳驅動器警報，軟體**不會報警**——那一站會一直等下去（golden V912 自己的 1203 分支也有同一個洞）。St01 的修法是讓 golden 原本的「馬達警報」那一段也看得到 1203 的警報。要你定的只是字：Mot_Table 裡 ServoAlarmOn=0 的軸（例：W-48 提到的 M03／M22／M41／M42／M108）跳的時候，用 **WAR24MMM7**（golden 的「位置錯誤，請回原點再重新啟動」）還是 **WAR24MMM8**。**建議 WAR24MMM7**（St01 的預設，跟 golden 用同一個字）。 | WAR24MMM7 | FROM_STEVEN 1004 15:40；TO_ES02 E-045（`220edfa7`） |
| 102 | ⏳ **機台端（MC01／EastSun）W-32／W-33／W-43～W-46 都追了兩次沒回**（週日機台沒人；照「追兩次沒回就請你決定」的規則） | 白話：都是要機台現場回答的唯讀題（HOME 安全、開機煞車、派工 7 的五題、Index 機構、急停接在哪、有哪幾道門），每一包的 GitHub README「請回」都有列。選項：A＝週一 EastSun 上班後隨下一包 README 再問一次；B＝你打電話問。**建議 A**（週日沒人在機台）。 | A | WAITING_REPLIES W-32／33／43～46；GitHub README「請回」 |
| 99 | 🔴 **機台的 Teach 數值被夾成 9046LS 的邊界——最可能就是今天回報的「Teach 欄位數值輸入有問題」**（機台自己的 TEACH-KB，EastSun 10/02 裁決 C；main 照你 #51＝A 不夾） | 白話：機台 10/02 自己加了「照 golden 9046LS 範圍夾值」的小鍵盤，可是這台 HT9050 的實際位置都在那個範圍外。例：Auto1 Y 實際 -55618，9046LS 範圍 -60000～-74000 ⇒ 點開按 OK（**連按 Abort 也一樣**）就變 -60000；Loader Y -55884 → -70000；Out Pick X -53548 → -19000——共 11 個在用的點；10/03 歸零、要重教的 Loader X／Y 一點就變 19000／-70000。也就是**這些點打不進正確的數字**（只能用 Set To 寫目前位置）。選項：**A＝機台也改回不夾，跟 main 一樣**（拿掉 HW.teach.html 載入 ht9045_teach_kb_c.js 那一行）；B＝改用 HT9045 臂的範圍（Out Pick X 仍會被夾）；C＝維持。**建議 A**，等有 HT9050 自己的範圍（或照你 #44 用馬達軟體極限）再加回。 | 機台的設定由 EastSun 決定：筆電只在 GitHub README 附證據＋建議 A，**不替機台改** | TO_ES02 E-12、W-49、GitHub README |
| 100 | 🟡 **Teach 頁 136 個「有範圍的參數欄」，超出範圍時存檔會無聲丟掉**（golden 也不提示，但 golden 的小鍵盤會先夾，所以打不出範圍外的值） | 白話：elTeach 那批參數欄在 C++ 有範圍（例：AOA pitch 0～4000、Radian ±10、某些 Z -3000～-500）。main 照 #51＝A 小鍵盤不夾 ⇒ 打 4321 到 0～4000 的欄位、按存檔顯示成功、重開又是舊值，**沒有任何提示**（瀏覽器端到端探針 1004 15:4x 實測）。選項：**A＝這 136 個參數欄的小鍵盤照 golden 給範圍（按 OK 時夾）；位置點（TECH 那 517 個）照 #51 仍不夾**；B＝不夾，但存檔後提示「哪些欄位超出範圍沒寫入」；C＝維持。**建議 A**（跟 golden 一樣、跟存檔用的範圍一致，不碰 #51 的位置點）。 | C（維持，不改） | `tools/webprobe/teach_kb_e2e_probe.py` 的 (f)；`FileRW/Teach.gen.inc` |
| 98 | 🟡 **F5（開機）期間的兩個小題**（NB2-1 U19；機台派工 4「F5 期間關 HMI 視窗停不下來」已修成 MR !177，這兩題是剩下的；不急） | 白話：①**D1**：開機途中跳出「擋住流程的對話框」時，要不要把 HMI 視窗叫回來？現在會叫回來（MODAL-WAKE）。不叫回來的話對話框沒人看得到，wb_serve 會一直等在那裡、還佔著 1203 卡。②**D2**：操作畫面剛跳出來、還在載入的那 5～7 秒內按 X，要不要也算「關掉這次 F5」？現在不算，30 多秒後視窗會再出現。要改得動 `web/background.html`（同事的檔）。例：EastSun 按 F5 後馬上又按 X 關掉，等一下視窗又自己跳回來——那就是 ②。選項：**D1＝A 照舊叫回來（建議）**／B 不叫回來；**D2＝B 先不做，等機台真的遇到再說（建議）**／A 也算關掉 | 都照現在這樣 | `docs/nb2_assist/URGENT.md` U19、README R217（`v906/nb2-assist` `e212a0fd`）；MR !177 |
| 96 | 🟡 **1203 卡片命令失敗（停不下來、移動被拒）要跳哪個警報碼**（St01 S-26 R5／10-3；跟 golden 不同的地方要你定） | 白話：golden 在 EtherCAT 卡命令失敗時跳 WAR16122；移植樹這 67 個框都在 `#if HAVE_PCI1203` 裡、沒編進來，所以今天卡片拒絕移動＝站別一直等、停止失敗只在 op log 寫一行。St01 建議補成「延後的一次性警報」（不在 1203 路由與輪詢裡跳框、有防重入，也蓋到被拒的速度設定）。問題是 WAR16122／16123 在警報目錄裡的說明文字是 Loader／三小時巡檢的意思（golden `asendic_Loader.cpp:1648` 也用同一個碼）。例：Index Z1 停止命令失敗，畫面跳 WAR16122，說明卻寫 Loader 的事——操作員會找錯地方。選項：**B 新開一個 W906 自己的警報碼＋中英說明（建議；跟你 1003 #70② Arm Cell 用 W906 自己的告警碼同一個做法）**；A 照 golden 用 WAR16122（說明文字對不上）；C 先不跳框（維持只寫 op log）。註：補了之後是「出錯就停下來報警」，方向跟 Steven 07:4x「裝置有異常 error 機台就不可以動」一致 | 維持只寫 op log（現在這樣）；St01 先做其他項 | `docs/handoff/S26_FINDINGS_20261004.md`（main）總表第 9、10 項，附錄 A R5、附錄 B 10-3 |
| 97 | ⏳ **ES02 與 Frank01 各有題目追了兩次都沒回**（W-26、W-30、W-27；照「追兩次沒回就請你決定要不要打電話」的規則） | 白話：①ES02 的 E-09（機台螢幕上被遮住／截斷的視窗清單＋修正，機台派工 2）沒人認領；②ES02／EastSun 的 W-30（派工 6：「激磁又放開」是軟體一開起來的前幾秒，還是按 HOME 之後）沒回；③Frank01 的 F-03（VC4 的 4 個吸嘴吸一顆 IC，機台派工 3）沒人認領。三題都是 1003 19:4x～20:4x 派的，1004 00:3x、05:1x 各追過一次；ES02 最後一次推送是 1003 18:16，Frank01 是 1002 07:47。今天是週日，三題都不擋 10/05 機台要「安全地動起來」那件事。例：W-30 的答案是用來確認 #88（開機後要先上電、Servo ON 才放煞車，第 137 包已上）有沒有對到機台看到的現象。選項：**A 週一 10:00 照預設改派（建議）**——E-09 還沒人認領就改給 St02（網頁畫面是他那條線），F-03 照原卡片改給 NB2，W-30 等 EastSun 上班回；B 你現在打電話或傳訊息給 EastSun／Frank；C 現在就改派 | 照 A：週一 10:00 改派；之前不再追 | `docs/handoff/WAITING_REPLIES.md` W-26／W-27／W-30；TO_ES02 §4、TO_FRANK §4 |
| 95 | ⏳ **Ifor01 的 I-08 追了兩次沒回**（W-22） | 白話：你 1003 14:3x 定了 ATC 常溫的溫度校正檔照 912 分冷熱兩份（#47＝A），派給 Ifor01 當 I-08；14:3x 派、18:5x 第 1 次追、St01 23:24 代問、1004 00:3x 第 2 次追，都沒回一句「接或不接」。Ifor01 其實在線（00:10 還交了 !162），只是沒回這一題。例：照 #47＝A，ATC 常溫的溫度校正要分 `Temperature_ATC.Data`／`Temperature_ATC_Cold.Data` 兩份讀；沒人做之前，移植樹還是只讀一份。選項：**A 你打個電話或傳訊息問 Ifor（建議）**；B 改派別人（例：St01 或 NB2）；C 先放著 | 等 | `docs/handoff/WAITING_REPLIES.md` W-22；TO_IFOR §4 |
| 94 | 🔴 **安全門：HT9050 該檢查哪幾道門**（St01 S-23 後半；跟 golden 不同的修法要你定） | 白話：golden 開機時不管表怎麼寫，都會強制打開 SnSafeDoor1／2／3／6～9 的門檢；HT9050 上門 2／3／6～9 接的是不存在的模組，讀起來永遠是「開著」，所以 main 上 START、閒置時的 Motor Test／Teach 都會被擋。機台端為了能動，自己加了暫時補丁把門 1／2／3／6～10 全部停用——**連唯一真的門 SnSafeDoor1 也停了，所以機台今天開門軟體也不會擋**（README 已用紅字提醒人在旁邊要小心）。例：EastSun 打開前門伸手進去，這時有人在網頁按 JOG，軟體不會因為門開著而拒絕。選項：**A 只強制打開「現用表上 Enable 1」的門（SnSafeDoor1 照開，其他照表），HT9050 才這樣、其他機種照 golden；等 EastSun 回實際有哪幾道門（W-46）再做（建議，St01 也建議）**；B 照 golden 全部強制打開（HT9050 會一直被擋，要先補那幾個模組）；C 維持機台的暫時補丁（沒有門檢） | 程式不動；機台照舊用自己的補丁；README 已警告 | `docs/handoff/S23_FINDINGS_B_C3_C4_20261004.md`（main）；TO_ES02 02:58 |
| 89／90／92／93 | ✅ **Steven 1004 07:2x 已代回，全部照建議**（St01 轉，FROM_STEVEN §3 07:26；前例 #86）——不用你回，要推翻再說 | #92＝**A**：先擋——NB2-1 !170 進第 59 批（HT9050 上 Index 成對下壓指令報警、不再假回「完成」）；真正的修法**不是**馬達層「成對→只動 Z1」，而是移植 V910 的 DoTestHead（Frank 的 910 Index 流程，移植樹沒有，`csystem.cpp:657`），筆電派給 St01、Steven 07:4x 說 10/05 18:00 後才做（跟 E-038 Phase B 同一塊程式，一個人做免得互相衝突；Frank01 F-03 都沒回，另外通知他）。#93＝**A**：開機就照 Mot_Table 設 Index 四軸存不存在，只 HT9050（取代你 0930 的 D1＝A；NB2-1 MR !175，第 141 包已上）。#89＝**A**：主迴圈卡住 60 秒以上自動錄一次 State Record（只寫 `W906_*` 檔）。#90＝**A**：對話框開著也能按 State Record（只讀的移植例外）；#89／#90 由 St02 S-24 做，共用檔修改 Steven 自己在 St02 那台核准（Q97＝A）。另：E-038 Q90＝A（不加重力基準，照 golden）、Q91＝A（Phase B 交 St01） | 照 Steven 的答案做 | FROM_STEVEN §3 07:26（`v906/steven-handoff` `71f8df8a`）；St01 `decisions-decided.md`（review6 `3f6183c4`） |
| 91 | ⚠ **開機驗表（C5）：Steven 改成「10/05 18:00 前只記錄，之後裝置 ERROR 就整台不能動」**——18:00 前＝建議 A（只記錄、不擋）；18:00 後比原本的 B 還嚴：IO／馬達裝置有 ERROR 時 HOME、START、手動移動（Motor Test／JOG／Teach）都拒絕，停止／中止不擋。不推翻就照這樣（St01 S-23；golden 沒有，算新功能） | 白話：開機讀完 IO_Table／Mot_Table 之後，照 12 條規則把「安靜地變成 0 或被停用」的問題寫進 op log 與主控台，開機摘要多一行「驗表：ERROR n／WARN n」；不跳視窗、不擋開機。例：今晚 St01 查到 M37 的齒輪比寫成 `0.0.071425`（多一個點），讀成 0、沒有任何訊息；`AUTO_EMPTY_COLOR=0` 把表上開著的 Empty 氣缸全部停用；Index 安全門跟 SnSafeDoor1 是同一個點——有驗表的話開機就各有一行 WARN。選項：**A 做，只寫紀錄、不擋 START（建議；跑一週看誤報再決定要不要擋）**；B 做，而且 ERROR（例：HT9050 上 IO_CARD_TYPE 不是 4、1203 軸號重複）擋 START；C 不做。另：齒輪比讀成 0 時程式要不要另外當成 1（golden 只有停用的分支有這個保護）——建議不加，表改對＋驗表會抓到 **Steven 07:2x 選 B，07:4x～07:5x 補充**：「明天有個任務是機台要可以動起來，先以安全能動為主，可以先不考慮擋」「但是明天18:00後要按照標準流程走」「io 馬達的裝置有異常error的時候，機台就不可以動」「一開始就開不了，怎麼會還可以home?」⇒ St01 分兩顆：第 1 顆（只記錄＋開機摘要）先合；第 2 顆（擋 HOME／START／手動移動）10/05 18:00 之後才合；跑到一半的驅動器警報也照同一原則（S-26 R4）。**你要一直只記錄（A）的話**：第 2 顆合進來之前說一聲。 | 照 Steven：E-043 第 1 顆先合（只記錄）；第 2 顆（擋）10/05 18:00 後才合 | `docs/handoff/S23_FINDINGS_A1A2C5_20261004.md` §5、§7（main） |
| ~~83~~ | ~~🔴 **HT9050 出貨版第 132 包起照 golden 每 20 ms 跑加熱監控；機台 `HEATER_CTRL_TYPE=4`（COM12）但實機是網路型 DTM，溫度會讀成 999；室溫無影響（讀不到的警報只在運轉中、加熱或恆溫工單才跳）**~~ | ✅ **1003 11:2x 你回 A**（RULINGS_20261003 第 11 條）：加熱測試前 EastSun 先把 `HEATER_CTRL_TYPE` 改 3（先備份），或等 I-03b；已寫進第 132 包說明與 TO_ES02 | 照第 132 包機台通知：室溫照常；加熱測試前改 3 或等 I-03b | `74ae1585`（b18）合併說明；`machines/HT9050/snapshot/machine_params/D_HT9045_system/Gerneral.ini:302`／`:327` |
| ~~82~~ | ~~🔴 **訊息框跳出時也結束 Teach 的 Z All Up 回原點與 Light Scale 回原點（golden 不會結束，但那時馬達已經停了）**~~ | ✅ **1003 11:2x 你回 A：維持**（RULINGS_20261003 第 12 條） | A | b18 `ad4c0589`（`v906/jimmy-teachzdown`，還沒上 GitLab）；golden `uMotorTest.cpp:921-926`、`uteach.cpp:4505` |
| ~~88~~ | ~~🔴 **開機那幾秒，8 支 Z 軸的煞車可能在馬達電源還沒上來前就放開（EastSun 說的「激磁又放開」）**~~ | 白話：EastSun 問「軟體一開起來，好像先激磁又放激磁」。NB2-1 查完（R200）：回原點時斷電約 4 秒、回原點失敗斷電都照 golden；但移植樹自己的「逐軸放煞車」在開機時看到上一輪留下的 SVON 就放開 8 支 Z 軸的煞車，那時馬達電源繼電器還沒 ON（1203 的 SVON 斷電不會掉），只剩 `SnMotorPower` 一道在擋；之後上電又鎖、4 秒後再放。例：Z 軸靠煞車撐著，煞車一放、驅動器又沒電，軸就可能往下滑。**A（NB2-1 建議）** 放煞車條件加回「上電完成且延遲數完」（開機只剩 golden 那一次；運轉中照 EastSun 0929 規則）；**B** 維持現狀。這是 EastSun 的規則，已在 21:24 請他先量四項、再決定（GitHub README、TO_ES02 E-11） | 等 EastSun（他同意 A 就改） | NB2-1 R200／URGENT U17；W-33；✅ **你 1003 23:0x 回：A（放煞車加回上電條件；NB2-1 做 MR，下一包）**（RULINGS_20261003 第 24 條） |
| ~~87~~ | ~~🔴 **機台自己寫的 HOME 修正有 3 個中度安全問題（St01 審查），機台現在就在跑**~~ | 白話：今晚機台端（MC01＋EastSun）為了讓 1203 的整機回原點能用，推了 20 顆修正（cpp 0160～0179）。St01 讀了 0160～0170：**M-a** HOME 途中馬達電源掉了又回來，程式會自動對所有軸重置警報並重新激磁、繼續 HOME——就算是急停或開門造成的斷電也一樣（例：有人開門伸手進去，門一關、電一回來，軸就自己動起來）；**M-b** 真的有警報的軸每秒重試 20 次；**M-c** 驅動器回原點只看到 READY 就算完成，原點可能是錯的。已經在 20:55 透過 GitHub README 和 ES02 請 EastSun 先看、由機台端修。**A（建議）** 第 53 批照機台現在的樣子收進 main（機台本來就在跑，收進去不改變機台）、修正等 EastSun 做；**B** 先請 EastSun 把 M-a 改回 golden 的急停流程（斷電就停 HOME）再收；**C** 這些 HOME 修正先不收進 main（main 跟機台會越差越多，之後的 patch 會越來越難套） | A | FROM_STEVEN §3 1003 20:41（St01 V-6）；GitHub README `02a5409`；W-32；✅ **你 1003 23:0x 回：A（照收；M-a／M-b／M-c 由 EastSun 修）**（RULINGS_20261003 第 24 條） |
| ~~86~~ | ~~🟡 **HT9050 的 Index 自動測高（Auto Height）：解閘之前要不要加 golden 沒有的防護**~~ | ✅ **Steven 1003 21:4x 回**（透過 St01，FROM_STEVEN §3 21:47）：「你這題丟回去給 EastSun，他有手冊，等他確認」⇒ **B**：維持閘著，請 EastSun 照 Yaskawa SGDXS 手冊確認 6077h 的單位、正負號、驅動器端扭力上限（St01 已寫進 TO_ES02，也查了公開手冊當對照）。你要改再說 | B | FROM_STEVEN §3 1003 21:47；TO_ES02（St01 21:45／22:00 兩列）|
| ~~85~~ | ~~⏳ **Frank 的 2×4 與放 Auto 盤修法（W-12）追了兩次沒回**~~ | 白話：Frank 本人要回兩題——Carry kit／Index 吸嘴是不是 2×4（是的話 V906 自己加的 12 處 HT9050 判斷要拿掉）、`DoPlaceTrayToAuto_9050(0)` 怎麼修（固定放 Auto1 還是放缺盤的 Auto）。1002 09:3x 問，今天 14:3x、18:5x 追了兩次；Frank01 從 1002 07:47 就沒推過東西（週末）。**A** 打電話問一次；**B（建議）** 等週一（那 12 處不拿掉、Auto 修法照現狀） | B | `WAITING_REPLIES` W-12／W-01；TO_FRANK §4；✅ **你 1003 23:0x 回：B（等週一）**（RULINGS_20261003 第 24 條） |
| ~~84~~ | ~~🟡 **主畫面面板的實體 RESET 鍵要不要跑 golden 完整的 Reset**（NB2 R188 M3）~~ | ✅ **1003 14:1x 你回 A：維持現在這樣**（RULINGS_20261003 第 21 條；跟畫面 RESET 鈕同，0930 第 5 條） | A | NB2 R188 M3；St02 S-20 |
| ~~70~~ | ~~🟡 **Arm Cell 補強（NB2 R165）留下三個設計小題**（新功能、golden 沒有）~~ | ✅ **1003 14:3x 你回：全部照建議**（RULINGS_20261003 第 22 條）——①維持、等 ES02 量（E-08）②比照 HT160 停全部馬達＋跳告警 ③改成拒絕；②③交 NB2 | ①②③都維持現狀 | `v906/jimmy-armcell2` `8743153b`；NB2 R174 |
| ~~71~~ | ~~🟡 **Motor Test「Gear Ratio」分頁的小題**（②已改由第 81 項處理）~~ | ✅ **1003 14:3x 你回：全部照建議**（第 22 條）——①留備份 ③維持 60 秒沒進展 ④沒設時寫機台紀錄資料夾（交 NB2，跟手推一起） | 照現在的做法 | `v906/jimmy-gearratio` `5c43b54d`；NB2 R171 |
| ~~73~~ | ~~⏳ **Jerry（W-15）Timetick 的修改沒推**~~ | 白話：Jerry 說他實驗過把主迴圈一拍（`kServeTickMs`，現在 500 ms）改小，你 1002 15:4x 說測過就收（RULINGS_20261002 第 19 條），但他一直沒推，追了兩次。你 10:3x 選「寄一封信」⇒ **1003 11:06 已寄出**（你 11:0x 說「寄出」；寄件備份已確認），等 Jerry 推分支＋MR | 等 Jerry 回；4 小時沒動靜照帳本再追 | RULINGS_20261003 第 10 條；`docs/handoff/WAITING_REPLIES.md` W-15；TO_JERRY §4；✅ **你 1003 23:0x 回：等**（RULINGS_20261003 第 24 條） |
| ~~49~~ | ~~⏳ **Ifor 0922 的 V912 RotateKit 修正是哪個客戶、哪台機台、週報哪一列**~~ | 白話：Ifor 0922 修了 V912 的一個問題（RotateKit 取料失敗、重試後手臂卡住）。V912 是量產維護版，修正要走案件流程（建 case、BCB6 建置、出 release note），所以要先知道是哪個客戶、哪台機、週報哪一列；這台和 Ifor 都沒有紀錄。例：「超豐 HT9046A 第 3 號機，週報第 8 列」。你 10:3x 說看不懂 ⇒ 筆電已白話說明，並寫信問 Ifor（0922 那份 RotateKit 分析是哪個客戶／哪台機）——**1003 11:06 已寄出**（寄件備份已確認；帳本 W-18）；你知道的話直接回一句更快 | 等 Ifor 回；Ifor 那條分支先不推 | RULINGS_20261003 第 10 條；FROM_IFOR 1002 08:4x；`D:\HT9045\backup\ifor_wip_20261001\V912_RotateKit_4710_Ifor20260922.patch`；W-18 追問兩次（1003 15:3x、19:5x）還沒回；✅ **你 1003 23:0x 回：不知道 ⇒ 等 Ifor**（RULINGS_20261003 第 24 條） |
| ~~72~~ | ~~⏳ **機台端（W-14）還沒回的兩件：ADAM-6024、接地監測板／OTD**（原第 54 項 W-07 併進來）~~ | 白話：要機台給唯讀的現場事實。①**ADAM-6024**（EP 下壓力）：`ADAMTCP.dll` 在哪（跟 wb_serve.exe 同目錄嗎）、`Gerneral.ini` 的 `EP_Install`／`INSTALL_DOUBLE_EP`、172.16.8.110 回不回 ping——決定總開關 `W906_ADAM_EP_LIVE` 什麼時候能開。②**接地監測板／OTD**：機台 ini 寫 `USE_GROUND_MAN=0`、`USE_OTD=0`（cpp 0131 代答了 W-07），第 131 包 README 仍請機台確認現場有沒有裝。Bin 面板那一半你 10:3x 回了（RULINGS_20261003 第 9 條，§1）。**A（建議）**你或 EastSun 看一眼機台回一句；**B**再等機台下一次推送 | 再等；ADAM 總開關維持關 | `WAITING_REPLIES` W-14／W-07；GitHub 第 129、131 包「請回」；✅ **你 1003 23:0x 回：B（再等機台）**（RULINGS_20261003 第 24 條） |
| ~~34~~ | ~~⏳ **Frank01 的 9050 流程（F-01b）：剩下要 Frank 本人回的**~~ | ✅ **1003 14:3x 你回：照建議 B 再等**（第 22 條）——W-12 14:3x 第 1 次追問 | Frank 確認前那 12 處不拿掉 | `WAITING_REPLIES` W-01／W-12；TO_FRANK §4 |
| ~~47~~ | ~~舊題：**ATC 常溫的溫度校正檔要不要照 912 分「冷／熱」兩份**（Ifor 1002 08:4x 建議）~~ | ✅ **1003 14:3x 你回：照建議 A**（第 22 條）——整棵樹照 912 分冷熱，交 Ifor01（I-08） | 維持現狀（SetTemp 照 906） | FROM_IFOR 1002 08:4x 第二列 |
| ~~48~~ | ~~舊題：**ATC 那幾張卡（LI-3～LI-5）要不要解除「先暫緩」**（RULINGS_20260927 第 6 條）~~ | ✅ **1003 14:3x 你回「全部照建議」**；這題沒標建議 ⇒ 照預設 **B 維持暫緩**（第 22 條）；要解除再說 | B | FROM_IFOR 1002 08:4x 第三列 |
| ~~33~~ | ~~舊題：**請 Steven 以後「有改程式」的 MR 不要自己在 GitLab 網頁按合併**~~ | ✅ **1003 14:3x 你回：照建議 A**（第 22 條）——已合的不撤，以後改程式的 MR 先給筆電 gate | 已合的不撤 | TO_STEVEN §4；RULINGS_20261002 第 13 條 |

> 本輪從 §0 拿掉的：#4（過期）、#41（信 1002 07:01 已寄）、#53（進行中，移到 §2）、#54（併進 #72）、#74～#81（你 1003 回了，見 §1），以及所有劃掉的舊列（舊版在 git 歷史）。

---

## §1 做完了（附證據，不是形容詞）

時間範圍：1002（五）22:00 → 1003（六）13:3x。

### 1. 五個機台更新包

| 包 | GitLab main／GitHub | 帶了什麼（白話） | gate（負載逾時的都單獨重跑） |
|---|---|---|---|
| 129 | `13a57384` 02:22／`aab3911` 02:29（第四十五＋四十六批） | Teach 頁 **Arm Cell**（指定吸嘴移到指定料盤格，只移動；Z 先抬到安全高度、確認在原點才動 X/Y）；Motion View 換成 HT9050 專用畫面（NB2 !124＋筆電補 `Machine-profile.js` 墊片、軸讀數即時）；`machines/HT9050/` 換成機台 19:29 正本（NB2 !123）；Ifor !115 DTME08（不武裝）＋!121 TP-1；St02 ADAM-6024（Steven 網頁合的，保留、總開關關）；HOME 照 golden 呼叫 RunTestProgram；Jerry !107；St01 q59；St02 !116 Lot Info FTP 頁；MES16441 說明檔；`build.bat` 截斷檢查被砍掉時判失敗（INBOX 149）；HTDESIGNER 0.162 | 出貨 351 支：55 敗 → 單獨重跑 51 支 50 過；模擬 351 支：53 敗＝19 基準＋34 → 33 過；兩組態只剩 DTME08Cycle（負載下計時）；system／config／IniData 0 變動 |
| 130 | `577c41c5` 07:42／`75c49fd` 07:53（第四十七＋四十八批） | St01 review6 `81a106d2`（160 顆，E-021～E-029：HandlerSys Heater 分頁 EJ1N／DTM 71 通道、主畫面溫度條、ShowBinSelect、Contact CT）；Contact 頁 START（CT-3b）與自動 Offset START（OS-1b）照 golden 接上 ⇒ START 普查 34／32／2 → **36／34／2**（HT9050 上 Contact START 其實不啟動，見 §6）；**INDEXZ2**：Index Z1 走 1203（`WB_ENGINE_INDEXZ_1203` 開）；機台 cpp 0131～0140（開機對 M35 跑 InitMotor、HOME 燈反相、WORKLOG 78～85；ZHOME-200 收了又撤 `f7bce07e`）；主表 Mot_Table＝機台 21:56（`5bfef4d9`）；**ENGHOME**（`0eb965d1`，main 也開）、**LOGIN-HONPREC**（`73dde6f2`，開機就是 HonPrec，交機前要關）；Ifor TP-1b | 出貨 369 支：31 敗（4 基準＋27 逾時）→ 重跑 26 過、WebMotorAccess 單獨 1.2 秒過；模擬 369 支：142 敗（19＋123）→ 重跑 114 過，逐支再跑只剩 HSys_HeaterMix（不設上限 398／0 全過，但 183 秒＞120 秒上限）與 DTME08Cycle；system／config 0 變動 |
| 131 | `fd1b71c5` 09:48／`19b954b` 09:52（第四十九批） | **Arm Cell 補強**（R165：任何告警／通知／訊息框都停；GO 前與每拍查告警燈與 EMG；X/Y 移動中 Z 離開原點就停；到位看編碼器 ±0.1 mm／3 秒；X 被拒就不送 Y；新鈕 All Z Up）；**Motor Test Gear Ratio 分頁**（量測→預覽→存檔；存檔改 Mot_Table GearRatio＋teach.ini，先備份、寫完比對、不符還原；63 個對照突變全紅）；St02 !128 C9 G2（HT9050 上照 golden 隱藏）、!127 C14 Bin 顯示器（開機開 COM14）、!126 C10 HANA；NB2 !129 面板 PAUSE 關通知框 | b49b：出貨 375 支 107 敗＝4 基準＋103 → **103／103** 過；模擬 375 支 87 敗＝19 基準＋68 → **68／68** 過 |
| 132 | `db9f0430` 13:13／`a071554` 13:2x（第五十批） | **共用快時鐘**（加熱 20 ms 照 golden THeaterThread、Bin 號碼面板 30 ms、GM-2 30 ms；HT9050 的 `HEATER_CTRL_TYPE=4` 會讀成 999，加熱測試前改 3＝第 11 條）；Teach Set All In／Out Arm Z＋Out Z All Down（任何一軸在動就拒絕）；訊息框停 Motor Test HOME／LoopMove（INBOX 150）；Gear Ratio 照 NB2 R171 修；St02 c912-1～4 改回 906（GPIB 遠端 START／STOP 拿掉 ⇒ START 普查 **35／33／2**）、C14 Bin 狀態網頁頁、ADAM A3；St01 E-030 第一部分；機台 WORKLOG 86-87；README 寫了 Bin 顯示器改 type 4／COM14 的機台步驟（第 19 條＋NB2-2 A6 兩個檢查）與第 130 包 Contact START 的更正 | b50b：出貨 382 支失敗＝基準 4（負載逾時 3 支單獨重跑全過）；模擬 382 支失敗＝基準 19（WB_WsLink 單獨重跑過） |
| 133 | `45670f61` 17:14／`04237aa` 17:16（第五十一批） | 機台建置改 -O0（第 18 條）＋整棵樹 `-fexcess-precision=fast`（第 7 條）；Arm Cell 第一次讀到 Z 離開原點就停 X／Y（R174 M1）；`CommaText` 照 BCB6（INBOX 152）；**Arm Cell 走或手動教導時任何來源的 HOME 都擋**（NB2 !146＝U14 ③）；**GO 時舊通知框開著就拒絕**（NB2 !147＝INBOX 151）；St02 !140 OB-7；Ifor !142；NB2 !143／!144／!145；St01 review6 到 `65323419`（引用整理、拿掉 912 才有的 Barcode CSV Compare 在 St01 的檔）；機台自己的 cpp 0144／0146～0149、web 0088／0089／0091～0093（含 JOG-MAXVEL、MT-COPYALL） | b51c2：出貨 387 支失敗＝基準 4；模擬 387 支失敗＝基準 19（都不用重跑） |

GitHub 包資料夾檔數：129＝程式 125＋輔助 83、130＝99＋75、131＝56＋38、132＝87＋68、133 合計 296（README 寫 125／99／94／155／296，131 起那個數字是合計）。

### 2. 你回的裁決

**1003 10:0x～10:1x（`RULINGS_20261003.md`，`afc7b56f`）**

| §0 | 題目 | 你的回答 | 第幾條 |
|---|---|---|---|
| 78 | Steven 的常設規則「912 是修正或明顯比較好就留 912」vs 第 20 條 | **B**：全面接受；做的人自己判斷，兩邊行號都註明、帳本記一列 | 第 1 條 |
| 80 | St02 打不開 golden 0618 | 用統一的共用區 7z 密碼自己解（密碼不寫在這裡）；之後照 0618 翻，已合的用 NB2 R179 指紋比對逐項查 | 第 2 條 |
| 79 | St02 N07 SECS/GEM 斷線警報（!137／!138） | 照建議：收，但跟 INBOX 86 同一批（主畫面 Alarm Reset 消得了音之後） | 第 3 條 |
| 76 | 網頁表單產生器的 golden 來源 | St01 負責網頁，照他的意見（`tools/golden_root.py`） | 第 4 條 |
| 74 | Contact 頁送測試機的下壓力字串 | 維持 912 | 第 5 條 |
| 75 | A02 開著時 OP 存檔保護 | B：留 912 | 第 6 條 |
| 77 | 機台 WinLibs g++ 16.2 的浮點 | A：整棵樹 `-fexcess-precision=fast`，先用 objdump 證明 g++ 6.3.0 機器碼不變 | 第 7 條 |

**1003 10:3x（`RULINGS_20261003.md` 第 8～10 條，`4a8e4ea3` 10:47）**
- **第 8 條＝#81 Gear Ratio 手推量測 A（「要」）**：分頁加手推模式（記起點 encoder → Servo Off → 推到刻度 → Servo On → 記 encoder → 輸入尺上距離 → 預覽 → 存檔，teach.ini 一起換算；軟體全程不下移動指令），第五十一批交給 Gear Ratio 子代理（INBOX 154）。
- **第 9 條＝#72 的 Bin 面板那一半**：是 TFT 面板、COM 埠可以自己設 ⇒ `NUMBER_PANEL_TYPE=3` 不動，`[NUMBER_PANEL] COM_PORT` 設成實際接的埠（快照現在是 COM14，改之前先備份 `Gerneral.ini`；沒對上就照 golden 每 60 秒跳框）。#72 只剩 ADAM-6024 與接地監測板／OTD。
- **第 10 條＝#73、#49**：#73 寄一封信給 Jerry、#49 另擬一封問 Ifor，兩封都等你說「寄出」（仍在 §0）。

**1003 11:2x～13:1x（`RULINGS_20261003.md` 第 11～20 條）**
- **第 11 條＝#83 A**：HT9050 做任何加熱測試前，先把 `HEATER_CTRL_TYPE` 改 3（先備份 `Gerneral.ini`、改完重開 wb_serve），或等 Ifor 的 I-03b（`2e75b6ec`）。
- **第 12 條＝#82 A**：訊息框跳出時也結束 Teach 的 Z All Up／Light Scale 回原點——維持（`2e75b6ec`）。
- **第 13～17 條**（11:3x～12:1x）：筆電改當協調者（回答你＋分派追蹤＋合 MR／gate＋出機台包；開發與分析寫卡給同事；子代理最多 1～2 個）；St01 當備援整合者（筆電心跳與 main 都 4 小時沒動才接手，GitHub 包仍只由筆電推）；通用工具放 `tools/laptop_ops/` 共享；同事 MR 的新測試要附「故意改壞會紅」；每人建心跳、筆電每輪回報上線狀況（`2e29eaf3`、`e8b15118`、`5c7b0dd1`）。
- **第 18 條**：機台建置改 -O0；機台端的 AI 簡稱 MC01（`d06c869f`）。
- **第 19 條**：HT9050 的 Bin 顯示器 `NUMBER_PANEL_TYPE` 3 改 4（TFT）、COM14（`0b5354cd`）；附註 NB2-2 A6 的兩個機台檢查：裝置管理員確認埠、`[NUMBER_PANEL2]` 設同一埠、驗法看 BinDisplayLog（`20a4c926`）。筆電之前寫「3＝TFT」是錯的（§6）。
- **第 20 條**：V912 修好的 TFT 行為 D（DoCycle 一直跑）／E（顏色／Bin 號回讀）／A（數量變了才送）交 St02 移植，卡 S-22（`c40a45c0`）。

**1002 22:xx（`RULINGS_20261002.md`）**：第 22 條 Gear Ratio 四題都 A（`cedc9598`）；第 23 條一次回 15 題（`b84064b4`）：#68 ENGHOME＝A（main 也開）、#56 ADAM 保留＝第 20c 條、#65 HANA＝第 20b 條、#66 溫控照 Steven、#67 Q-A 留／Q-B 拿掉／Q-C 先留、#62＝A、#59／#60＝B、#58 DTME08 先不武裝、#64＝A、#61 知識明文放 GitHub、#50 LOGIN-HONPREC 進 main、#51＝A、#57 搬到 skill、#69 軟極限維持 ±999999、不卡。

### 3. 合進 main 的同事 MR（照 git log）

| 誰 | MR | 內容 | main 上的合併 commit | 包 |
|---|---|---|---|---|
| Ifor | !115／!121／!109 | DTME08（不武裝）／TP-1 溫度條／SetTempSave 測試 | `f3ad5779`／`e7cba2bb`／`c83dddd1` | 129 |
| Jerry | !107 | J-12 IndexCheck4Site、J-14 | `414a74f9`＋新 tip `f830a782` | 129 |
| St01 | q59 到 `4da8c8e9` | E-020、D-034 | `a5ff6f92` | 129 |
| St02 | !116／!108 | Lot Info FTP 頁／測試 | `0f9c1101`／`d412ef10` | 129 |
| NB2 | !123／!124／!113 | 機台正本＋工單／主畫面跟 gpibModel／文件 | `3007ba9c`／`b70d8a62`／`2184be3a` | 129 |
| Steven／ES02 | !103／HTDESIGNER 0.162 | skill 更正／外掛 | `7389ee45`／`e9caeed1` | 129 |
| St01 | review6 `81a106d2` | 160 顆，E-021～E-029 | `f30c284d` | 130 |
| St02／Ifor | !116 tip／!121 tip＝!125 | 註解改引 906／TP-1b | `41d74b71`／`5878c447` | 130 |
| St02 | !127（＋測試修正 tip `1616a034`） | C14 Bin 顯示器（照 906） | `68de75ef`＋`e1d913d7` | 131 |
| St02 | !126／!128 | C10 HANA（照 912，第 20b 條）／C9 G2 | `549a4db9`／`850fe839` | 131 |
| NB2 | !129 | 面板 PAUSE 關通知框（INBOX 124） | `86eb94eb` | 131 |
| St02 | !139／!141 | 稽核帳＋4 支 skill／C12 按鈕普查 | `5967db16`／`952f5d2a` | 只有文件 |
| St02 | !130／!132／!134／!136 | c912-1～4（912 才有的改回 906） | `a4670dc0`／`0f041fbd`／`d9ff82c9`／`7e605675` | 132 |
| St02 | !131／!135 | C14 網頁 Bin Display Status 頁／ADAM A3 | `381eee91`＋`4fc26324`／`5c18c9ae` | 132 |
| St01 | review6 `e2adab0b`、`6c7f30d9` | E-030 第一部分（Q-B 拿掉、Q-A 留） | `87d27fb1`、`7769fd0a` | 132 |

| St02 | !140 | OB-7 Observer SG_JamCount Query Now／Yesterday（NB2 R180：忠於 0618） | 第 51 批 b51 | 133 |
| Ifor | !142 | DTME08Cycle 測試計時說明 | 第 51 批 | 133 |
| NB2-1 | !143／!144／!145 | F5 建置順序／MotorPoints_HT9050 釘 HOME 燈／`mutation_check.py` | 第 51 批 | 133 |
| NB2-1 | !146／!147 | **HOME 全來源擋 Arm Cell／手動教導**（U14 ③）／**GO 時舊通知框開著就拒絕**（INBOX 151） | b51 `08e92dab`／`63c6362a` | 133 |
| St01 | review6 到 `65323419` | E-030 3～4 引用整理、Barcode CSV Compare（St01 的檔）、E-032 引用改 0618 | b51 `02133d8d`（兩檔衝突，逐處斷言後解） | 133 |
| Steven | !148 | 日報技能改 RD5 統一格式（只有文件） | main `06d40c50` | 只有文件 |

NB2 !133（主表換 21:56）跟 `5bfef4d9` 是同一份，ack 不另合。

### 4. NB2 的覆核（R165～R180、R183～R190）

| R | 時間 | 看了什麼 → 結論 | 用在哪 |
|---|---|---|---|
| R165 | 1002 22:1x | Arm Cell 對照 HT160：規格漏 3 項（2 項安全） | 第 131 包補強 `1df2b97a` |
| R166 | 22:3x | 機台 21:56 快照：1203 軸 19 列 SensorType 全 0 | 主表 `5bfef4d9`（130） |
| R167 | 23:1x | 認領 INBOX 124 | NB2 !129（131） |
| R168 | 1003 00:5x | St02 C14：逐句忠於 golden；1 高（開機後顯示器不會啟動）、3 中 | 高的那條 **R185 撤回**：main 開機有經 SetStartModeData 呼叫 InitShowBinDigital（NB2-2 A5 也這樣查） |
| R169 | 01:3x | St02 C9 G2：6 個 handler 忠於 golden；合併釘子 59／43／38；M2 同臂其他軸在動時 Set All Z 按得下去 | 合併照改；M2 轉 St02 |
| R170 | 01:5x | St02 C10 HANA：本體＝V912 逐句；2 中（真 socket 路沒有會失敗的測試、GB_CT 永遠 FAIL） | 還沒轉（HT9050 不走 HANA） |
| R171 | 02:4x | 筆電 Gear Ratio：沒有高；2 中（中止時量測段不停、teach.ini 真寫檔沒測試） | 第 132 包 `36d575e6` 修 M1／M2／L1／L2／L5 |
| R172／R173 | 03:1x／03:2x | MR !133 主表；Ifor !125 的事實與行號都對 | ack；第 130 包 |
| R174 | 04:2x～05:0x | Arm Cell 補強：R165 都做到；1 中（Z 原點監看比 HT160 慢、不先停 X） | 第 51 批 azo |
| R175 | 04:4x～05:1x | Arm Cell 主體＋功能測試：沒有高；6 中，最要緊 C1（URGENT U14）：ENGHOME 開了之後，網頁以外的全軸 HOME 可能跟 Arm Cell 搶同一軸 | ⚠ 還沒排（§2） |
| R176 | 07:1x | 晨間摘要 | — |
| R177 | 07:5x | St02 N07：0618 沒有這功能；H1 主畫面 Alarm Reset 消不了蜂鳴器 | §0 #79 → 第 3 條 |
| R178 | 08:2x | HT9050（客戶碼 957）出貨版按 Contact 頁 START 照 golden 不會啟動 | TO_ES02 09:0x 更正（§6） |
| R179 | 09:0x | NB2 也只有 0618 → 改給「檔／函式指紋」做法 | W-16 結案、W-17 |
| R180 | 10:0x | St02 !140 OB-7：忠於 0618、沒有高；1 中（SIGURD 機台 Loader Count／Rate 停住＝St02 決定保留） | 第 51 批照收；低的幾點在 TO_STEVEN 10:2x |
| R183 | 1003 11:xx | 協作機制：同一輪回認領、MR 的新測試附「故意改壞會紅」、看 NB2-1 心跳 | 照收（RULINGS_20261003 第 15～17 條、night-loop 技能開場） |
| R184／R186 | 12:3x／12:5x | `MotorPoints_HT9050` 沒釘 9050GPIB 的 HOME 燈（改壞一個字面仍綠）→ 補釘子後 11／11 抓得到 | MR !144 → 第 51 批 |
| R185 | 12:4x | 撤回 R168 H1 | 見上 |
| R187 | 13:0x | `mutation_check.py` 上 main，大家的 MR 都能附「故意改壞會紅」 | MR !145 → 第 51 批 |
| R188 | 13:4x | 覆核第 51 批：ARMCELL-ZORG 沒有高／中；SCANKEY 1 高（面板 HOME 沒擋 Arm Cell）＋4 中 | 高的那條＝!146（統一在 TfMain::Home 擋）；中的給 St02 S-20；M3＝§0 #84（你回 A） |
| R189／R190 | 14:5x／15:0x | !146 HOMEBLOCK／!147 ARMCELL-NOTICE（附反向驗證） | 第 133 包 |

### 5. 文件與交接

- **TO_ES02**：第 129、130、131 包各一張上機卡；**E-07**（St01 上機檢查 A35～A56，A56 測前先備份 `Gerneral.ini`；`62276ffc` 08:25）；**R178 更正**第 130 包那張卡與 A35（`79158ea2` 08:59）。
- **W-16／W-17（0618 對 0625）**：NB2 R179 推了 0618 指紋 `fp_0618.tsv`（707 個檔／11,144 個函式，只有檔名／函式名／行號／雜湊，`3c6b7fa4` 記帳）；St02 10:10 回（`v906/steven-handoff` `17aa245b` `docs/handoff/W17/`）：**29 個檔／31 個函式程式不同，只在 0625 有的只有 `WriteN07Log`**（`d5c58657`）。
- **追問**：W-14、W-15 第二次（`2f00b1a5`）→ §0 #72／#73。
- **§0 新題**：#74～#76（`1523a564`）、#77（`10dc448b`）、#78（`89f03cca`）、#79（`62276ffc`）、#80（`79158ea2`）、#70／#71（`f147a919`）、#81（`b6acab52`）。
- **INBOX**：150／151（`f147a919`）、152／153（`c1513c8d`）、154（`b6acab52`；10:47 改成「要，第 51 批」）、155（`d5c58657`）。
- **RULINGS_20261003 第 8～10 條**＋W-14／W-15 註記、TO_ES02 §4 Bin TFT 的 COM 一列（`4a8e4ea3`）。
- **TO_STEVEN**：機台回報 Adam6024_Pressure 紅轉 St02（`8b95b212`）；HSys_HeaterMix 在模擬組態變慢（183 秒，第 46 批約 40 秒）請 St01 查；N07 認領 OK（`6177b5b8`）。
- **CLAUDE.md** START 普查改 36／34／2（`3dfb0687`）。
- **GitHub knowledge**：筆電的知識明文鏡像 `5e200c5`（891 檔；交付 7z 密碼在 7 個交接檔遮掉；掃描 0），之後每包重鏡像。
- **11:0x～13:1x 的文件**：NIGHT_REPORT 整份重寫（`28db124f`）；機台 10:48 快照鏡像（`a9fd23be`）；TO_STEVEN S-18（`fcb3f8e0`）；RULINGS 第 11～20 條（`2e75b6ec`→`20a4c926`）；`tools/laptop_ops/` 12 支工具＋README、`heartbeat.py`／`team_status.py`（`e8b15118`、`5c7b0dd1`、`08c11ee7`）；交接 12:2x（`e52f9618`）；「3＝TFT」更正（`db982352`）。
- **給同事的卡**：S-19 St01 備援演練、S-20 St02 主迴圈、S-21 St01 E-032 `==`、S-22 St02 TFT D／E／A；TO_ES02 第 132 包上機卡（本批）。

---

## §2 佇列中——刻意沒做或還在做（附原因與建議）

### 第 132 包（第五十批）——✅ 13:2x 已推（GitLab `db9f0430`／GitHub `a071554`）
- **共用快時鐘**（`74ae1585`）：主迴圈加一個共用排程：加熱 20 ms（golden THeaterThread）、Bin 號碼面板 30 ms、GM-2 30 ms；冷卻風扇不再在 tick 執行緒等 FormLock。模擬組態量到：主迴圈每秒 17.2 → 53.8 圈，加熱每秒 47.4 次（golden 50），沒有超時／重入／例外。沒做：框擋住時快時鐘不跑（第 51 批 ml）、`timeBeginPeriod(1)`（ml）、HT9050 加熱讀 999（§0 #83）。
- **Teach Set All In／Out Arm Z＋Out Z All Down**（INBOX 147，`64496feb`）：照 golden `uteach.cpp:4300`／`:4367`／`:4542`；All Down 先把每顆 Out Z 設 10% 速度再走；All Down 在**任何一軸在動時拒絕**（筆電 08:1x 的安全預設；Set All 照舊只看同一臂）；STOP 與關 Teach 會停。
- **訊息框停 Motor Test HOME／LoopMove**（INBOX 150，`ad4c0589`）；多做的那一步＝§0 #82。
- **Gear Ratio 照 R171 修**（`36d575e6`）：結束時停自己那一軸、100 mm 上限從起點算、新 ctest GearTeachSave 走真的 teach.ini 寫檔。HT9050 的 teach.ini 第一次存檔會被拒並還原（檔案還不是 Teach 頁存過的格式），畫面提示「先在 Teach 頁按一次存檔」。
- **St02 c912-1～4**：EMG 條件、GPIB 遠端 START／STOP、fSecsAlarm、Qorvo 蜂鳴、GetTesterResult、AOI 參數改回 906；P65 ARM-QA 留 912（第 1 條）。START 普查 36／34／2 → **35／33／2**（`tools/start_sites_census.py --check 35 33 2` PASS）。另 !131 C14 網頁頁、!135 ADAM A3（WinLibs 浮點容差）、St01 E-030 第一部分、機台 WORKLOG 86-87（`ba11590e`）。
- **推完了**：b18 合 origin/main（65 檔，只有文件／機台快照鏡像／Python 工具，沒有要編譯的）→ GitLab `db9f0430` 13:13 → GitHub `a071554`（155 檔；權杖／私鑰／7z 密碼掃描 0；knowledge 從 `db9f0430` 重鏡像 897 檔）。

### 第 133 包（第五十一批）——✅ 已推（GitLab `45670f61`／GitHub `04237aa`）
見 ★ 表。子代理全部收掉（11:3x 起筆電改當協調者）；主迴圈三件交 St02（S-20），手推量測交 NB2，N07 !137／!138 跟 St02 的主迴圈 MR 一起收（第 3 條）。

### INBOX 狀態

| INBOX | 事 | 狀態 |
|---|---|---|
| 86 | 主畫面面板實體鍵（golden `TfMain::ScanKey`） | 交 St02（S-20，`v906/jimmy-mainloop` `07306ba2`）；跟 N07 一張 MR，也解 NB2 R177 H1（Alarm Reset 消音） |
| 145 | St01 卡在筆電的兩件：① CT-3c Contact 頁 Index Z 寸動（要 contact 狀態機；INDEXZ 開關第 130 包已開）② LI-6（Draft MR !70 計時器排程表） | 還沒排；原本排在 Arm Cell 後面，Arm Cell 已做完 |
| 147／150 | Teach Out Z All Down／Set All；訊息框停 HOME／LoopMove | ✅ 第 132 包（`db9f0430`） |
| 151 | Arm Cell GO 時舊通知框不擋 | ✅ 第 133 包（NB2 !147；你 14:3x 回 #70 ③＝A） |
| 152 | vclcompat `CommaText` 照 BCB6 | ✅ 第 133 包 |
| 153 | `bEnableBarcodeCSVCompare` 是 912 才有的 | St01 review6 拿掉了 St01 檔裡的；`atester.cpp`／`WebLogin.cpp`／`wb_serve.cpp` 等 16 檔還在用，第 52 批後再排 |
| 154 | Gear Ratio 手推量測 | #81＝A；交 NB2（`v906/jimmy-gearhand` `8ce18104` 是半成品） |
| 155 | St02 普查 D 類 36 顆 | 等 St01 代跑點擊探針定案再分工 |

### §0 #53「看得到按鈕但沒功能」——普查做完靜態那一半
St02 MR !141（`952f5d2a`，只有文件＋三支唯讀腳本）：64 頁、1,859 顆；462 顆沒有任何 script 指名，其中 254 顆是 golden 元件：A 121（有 C++ 處理器）／B 92（沒有）／**D 36（golden 會動馬達或寫輸出）**／E? 5；標「s:」的是暫定。下一步：St01 代跑點擊探針（要 Edge）→ D 類照 golden 接（第 0 條，不用問；INBOX 155）→ 按檔案歸屬分給 St02／St01／筆電；機台正在做的頁（Teach／IO／Vacuum／Motor）先問機台。

### 刻意沒做（照裁決）
N07 等 INBOX 86（第 3 條）；ADAM 總開關 `W906_ADAM_EP_LIVE` 關著（等 #72）；DTME08 不武裝（#58＝A）；網頁 Lot Start 不接 golden 完整檢查（1002 第 4 條＝B）；Gear Ratio 的 Z 軸在軟極限還是 ±999999 時是灰的（分頁靠它認佔位值）。

### NB2 查到、git 上還沒人接（建議）
- **R175 C1（URGENT U14）**：ENGHOME 開了之後，網頁以外的全軸 HOME（SECS RCMD HOME、automation 的 DoHomeAndStart、ESD `DoAutoDecayCheck`）不會被 Arm Cell 擋，可能同時動同一軸；最壞是 Z 以 1% 下降時引擎在動 X/Y。NB2 建議：S0 遇到 SoftStart／回原點中就拒絕、每拍查 HomeFlag、`TfMain::Home` 先問 `MotorAccessStartBlocked`。main 與各分支 git grep「R175／U14」0 筆。**建議排進第 51 批**（會動機構的安全題）。
- ~~**R168 H1**：開機後 Bin 顯示器不會啟動~~ ⇒ NB2 R185 撤回（main 開機有經 SetStartModeData 呼叫 InitShowBinDigital）。
- R170 的兩個中：HT9050 不走 HANA，不急。

### 第 137 包（第五十六批，含第五十五批）——✅ 1004 04:3x 已推（GitLab `8c9c27c5`／GitHub `368dc68`）
- **為什麼停掉 b55a**：01:1x 我請 NB2-1 說明 HOME 第 300 步送 Servo ON 之後、煞車放開之前會不會先動；NB2-1 R207（02:52）查到兩件：原版 !163 會讓 Index Z1（走 Galil 路由那一軸）**這一輪永遠沒有 Servo ON**（golden 的 ServoOnOff 看到燈已亮就不送），所以它的煞車永遠放不開、HOME 會一直等；另外第 302 步可能比煞車放開早一個 tick 就交給第一個移動。補修正 `50054e3c`：302 等煞車真的放開（5 秒沒放開就停 HOME 並列出軸）、300 一定給 Z1 送 Servo ON、煞車扣著的軸拒絕移動命令。b55a 跑到模擬 150／394 時停掉，改成第 56 批＝第 55 批＋補修正＋St02 !157／!166＋NB2-1 !167（PE 檢查遇到防毒擋住的 exe 只警告）＋main，一次 gate。
- NB2-1 !163（先 Servo ON 才放煞車，你的第 24 條；筆電 01:1x 要它在 MR 說明裡證明「煞車放開之前不會命令那一軸移動」）、St01 !164（S-17 對話框卡住的修正 A～E）、筆電的 absence 哨兵分類（只動註解與工具資料）。
- **St01 01:57 的先期提醒已轉機台**（GitHub README `061d5a5`，紅字）：IO 表上 4 個急停輸入、SnServo、SnSystemPower 都停用，軟體只靠 SnMotorPower 察覺急停；自動運轉 Index 的 Z1 不會下壓（跟 §0 #92 同一件）——在修好之前請不要按 START（W-45）。

### 排在後面的
- **第 57 批＝第 138 包**（✅ 1004 06:1x 已推：GitLab `703effd1`／GitHub `8f34133`）：NB2-1 !169、St02 !168。**下一批候選**：St02 !165（兩支測試修好再代跑）、St02 S-09 開機補設 MR、NB2-1 (n) Servo ON 編碼器回寫 MR；NB2-1 !170 等 #92。
- **第 58 批＝第 139 包**（✅ 1004 07:4x 已推：GitLab `54505e0b`／GitHub `79274e8`）：NB2-1 !172。
- **第 59 批＝第 140 包**（✅ 1004 10:2x 已推：GitLab `3ffb2b00`／GitHub `088a141`）：NB2-1 !170（Index 下壓擋法，Steven #92＝A）、St02 !171（S-09 開機 OCR 狀態格）。St02 !165 原本也在（`v906/jimmy-b59` `69f86dd1`，gate b59a），出貨組態抓到 `FShow_Audit`（2 處直接讀 `fHome->fShow`）⇒ 退回 St02、改用 `v906/jimmy-b59b` 重跑。
- **第 60 批＝第 141 包**（✅ 1004 11:4x 已推：GitLab `29ef9e3c`／GitHub `e7b00fa`）：St02 !165、St01 E-034、E-038 Phase A、review6、NB2-1 !175。
- **第 61 批＝第 142 包**（✅ 1004 13:3x 已推：GitLab `65c533be`／GitHub `bc7174c`）：St01 E-043 第 1 顆、NB2-1 !177。
- **第 62 批＝第 143 包**（✅ 1004 15:0x 已推：GitLab `62074fbd`／GitHub `da1f2b0`）：St02 !176、St02 !179、NB2-1 !178。
- **St02 S-24（State Record 補強）卡在 Steven 那台的權限檢查**（改別人擁有的檔被擋）：要 Steven 選 A（允許這次修改）／B（他自己改）／C（先停在分析）；筆電不代改。
- **`CheckEPRange`（TFormHS）**：只有 KYEC_LEE＋KLT 功能用得到（HT9050 是 CUSTOMER_CODE 957），列進筆電的待辦，客戶碼專屬的排最後；`RecordEPLog_HS`／`ReadMultiEP` 在 golden 沒有任何呼叫（唯一一處被註解掉），照 golden 不翻、閘的原因已更正（St02 S-25）。

---

## §3 紅燈／回歸（哨兵抓到什麼）

- **防毒拖慢 → 每個 gate 階段幾十到上百支逾時（不是回歸）**：新建好的 exe 第一次啟動被掃毒卡住（CPU 只 13～28%）。單獨重跑：第 131 包出貨 103／103、模擬 68／68 全過；第 130 包剩 HSys_HeaterMix（398／0 全過但 183 秒）與 DTME08Cycle；第 129 包剩 DTME08Cycle（連不到的位址 connect 要 <1 秒，負載下 1.1～6 秒；1003 09:5x 請 Ifor 看）。第 132 包的 PE 截斷檢查也被拖了約 40 分（10:14～10:5x，同 INBOX 149 那一型），結果通過：338 個 exe/dll 結構完整。
- **gate 抓到的合併問題**：b49a 出貨組態連結失敗——St02 !128 的新測試直接編 `WebMotorAccess.cpp`，要 `GearCalc.cpp`（`a2ed9c57` 修）；C14_BinDisp 紅＝golden 在 type-3 埠上會送一串 14 個 Magazine-TFT 還原訊框，St02 測試的假匯流排把它們當成面板訊框（St01 代跑也紅），St02 `1616a034` 修、合進 `e1d913d7`。
- **機台回報 Adam6024_Pressure 紅（套第 129 包後；4q／5e／5f／9t）**：根因是機台用 WinLibs g++ 16.2，`double == 5.6` 判不相等（BCB6 與 g++ 6.3.0 判相等）；EP 總開關關著，只有測試不同。St02 A3（!135，第 132 包）先修他的檔；整棵樹對齊是第 7 條（第 51 批 fp）。
- **控制字元哨兵（線 D）**：發現它一直掃舊樹（§6），1003 08:30 修好後掃 b19：St01 `docs/handoff/AUDIT_IO_20260926.md:3` 的 `\r` 被寫成 CR（0927 講過兩次沒改，1003 09:0x 第三次請他修）；`atester_ProcessCount.cpp` 的 `\x0b` 兩處 golden 也有（照翻）；手冊與抽出的參考檔可忽略。 **1004 07:5x 再掃 b19（第 59 批）**：15 檔 3260 處，全是已知類別（手冊、golden 照翻的垂直定位字元 0x0B、抽出的參考檔、機台快照、fpfast 進度輸出裡的 CR）＋0915 的 skill 參考檔 `ours-kept-20260915.md` 檔尾多一個 CR（無害）；St01 AUDIT_IO 那處已經不在了；三個 MR 沒帶進新的。
- **NUMCMP 普查（第五十批的樹，04:5x）**：PASS，A=2 B=11 B′=3 C=30。
- **配方母體**：66 個目錄／65 份有 Contact.Data（多的那個是 FT005054，已知）。
- **真實檔**：07:5x 單獨跑 HSys_HeaterMix 時看到 `system` 有變化，是同時在跑的 b49b 模擬 ctest 暫時寫真實檔又還原；之後 `system` 跟 01:0x 基準逐檔相同。
- **磁碟**：`.claude\worktrees` 的 build 輸出 10:3x 量 58 GB、13:2x 量 82 GB（第 132 包 gate 又加了 b18 的 12 GB）；13:2x 清掉 6 棵已上 main 的（§4），之後總量 17G。
- **gate b51b 抓到 ObserverCore**（兩組態都紅、單獨也紅）：測試餵沒加引號的 `Motion Part` 表頭卻預期整串讀回＝舊 vclcompat 的切法；BCB6 沒加引號的空白也會切。程式（第 51 批 CommaText）對、測試預期錯 ⇒ `0ef6970c` 改成 BCB6 結果＋有引號例。其他測試沒有同類失敗。
- **gate b51c 在 cmake 設定就失敗**（`CMakeDetermineCompilerABI_CXX.bin cannot be read`，防毒鎖剛產生的探測檔）：前 2 分鐘剛跑過 NUMCMP 哨兵的 cmake 設定。清 build、等 45 秒重開 b51c2 就過。之後 NUMCMP 排在 gate 之後跑。
- **NUMCMP（第 51 批 round c）**：PASS，A=2 B=11 B′=3 C=30，跟 1002 基準一樣。
- **NUMCMP（第 61 批的樹＝main `65c533be`，1004 13:3x）**：PASS，A=2 B=11 B′=3 C=30，未覆核 0、沒蓋到的 TU 0，跟 1002 基準一樣——第 59～61 批的翻譯碼（E-034、ScanKey、E-038 Phase A、!171、E-043）沒有帶進新的同類錯。這次等 gate b62a 過了 cmake 設定才跑，沒有重演 1003 15:2x 的防毒鎖檔。
- ⚠ **機台端暫時跳過「氣壓不足」告警**（MC01 cpp 0158 BYPASS-WAR1603，EastSun 1003 下午）：在機台的 `MachineType.h` 打開 `W906_BYPASS_WAR1603_AIR`，golden DoSystem 的 Air not enough 那一段不跳。**只在機台本地，筆電不收進共用的樹**；已請機台氣源修好就拿掉（TO_ES02 17:3x）。
- **機台端與 St02 重複做了面板鍵**（MC01 cpp 0152 SCANKEY 系列 vs St02 S-20）：兩份都先不收，請 St02 對過再決定以哪份為底（TO_STEVEN 17:3x）。
- **gate 的 PE 檢查被防毒卡住（今晚 2 次，環境不是回歸）**：b52b 模擬組態卡在 `tests\test_tcp_tester_real.exe`（19:09 起，CPU 0，重跑一次又卡在同一個檔）、b52c 出貨組態卡在 `tests\test_pause_forward.exe`（20:24 起）——剛建好的 exe 一開檔就停住，build.bat 到不了 ctest。做法：停掉那支檢查、在完整的 build 目錄手動跑 ctest（b52b 兩組態都等於基準；b52c 用 `b52c_rest.sh` 自動處理）。請 NB2 讓檢查「開檔逾時就報 LOCKED、不當成壞檔」（卡 (k)，W-29）。
- **WebMotorAccess 在模擬組態 -j 5 下連三輪逾時（b52b、b52c、b53s），單獨跑每次都 1.5 秒過**：環境負載，不是回歸；但慢 80 倍不像單純負載，請 NB2-1 找出是哪一段在等牆鐘（卡 (p)，低優先）。
- **absence 哨兵（線 D，gate b54a 的 build_ship）**：86 條宣稱、4 個 NEEDS REVIEW，逐條開檔看過：`IniConfig`（MainClick.cpp:1465 講的是 V912 沒人讀某個欄位）、`fAGV`（講的是 golden 沒有 `fAGV->Close`，實測 0 筆）、`map2DList`（講的是 golden 只在三處整個清掉，實測一致）＝誤判，第 55 批寫回工具的分類表；**`IsMultiEPPressureRouteActive` 是真的過期**：St02 1002 已移植（`Adam6024Pressure_St02.cpp`）、G24 也解了，但 csystem.cpp 的舊註解與 `forms/fHS.h`（ReadMultiEP 的 Cat D 閘「全樹沒有」）還寫著沒有——不影響行為（ReadMultiEP 只從 RecordEPLog_HS 呼叫，那支還閘著），csystem.cpp 那行第 55 批標記，fHS.h 跟 EP 紀錄一起派 St02（卡 S-25）。控制字元哨兵：15 個檔，全是已知類別（產生的手冊、抽出的參考、golden 帶進來的），沒有新的。worktree 總量 22 GB（門檻 50 GB）。 **1004 08:0x 用第 59 批（gate b59a 的 build_ship）再跑**：selftest OK，ALL ABSENCE CLAIMS STILL HOLD（!165／!170／!171 沒有讓任何「不存在」宣稱過期）。

---

## §4 清理了什麼

- **1003 01:3x**：17 個過期的 build 目錄刪掉，`.claude\worktrees` 88 GB → 23 GB；`ost` worktree 移除（內容已在 main）；MR !68 標成已處理（main 已有 `0a036fcb`）；MR !70（Draft，計時器排程表）保留＝INBOX 145 ②。
- **1003 13:2x**：清掉 b18、b19、mach1002、fc、zd、gr2 這 6 棵 worktree 的 build 輸出（15 個目錄，刪前約 64 GB；都是內容已在 main 的批次，只刪 `Obj\V906\build_*` 與 `nb2_numcmp`，原始碼不動）。`.claude\worktrees` 之後總量 17G。還沒清的：azo（4.8 GB，第 51 批 gate 之後）、ml（1.7 GB，St02 S-20 在用）。

---

## §5 commit／push 清單

| GitLab main（推送時間） | GitHub 包 | 內容 |
|---|---|---|
| — | 1002 22:10 `3a1ebf6` | README：第 128 包第二次更正（不要改 Mot_Table SensorType；機台用全 0＋ORG-INV） |
| 1002 22:17 `a4af5c9d` | — | `machines/HT9050/snapshot/` 換機台 21:56 快照（699 檔，含 GPIB 機種檔） |
| 22:20 `cedc9598` | — | RULINGS_20261002 第 22 條 Gear Ratio＋TO_STEVEN 認領卡 |
| — | 22:36 `d3f9044` | README：W-14 第一次追問 |
| 22:37 `44d39242` | — | WAITING：W-07 機台代答、W-13 部分、W-14／W-15 第一次追問 |
| 22:52 `b84064b4` | — | RULINGS_20261002 第 23 條（你 22:4x 回 15 題） |
| — | 22:59 `5e200c5` | knowledge/ 知識明文鏡像（891 檔） |
| 1003 01:05 `a374c145` | — | CHAT_JIMMY：Arm Cell 補強請 NB2 審 |
| 02:22 `13a57384` | 129 `aab3911`（02:29） | 第四十五＋四十六批（§1） |
| 02:30 `58332fd9` | — | 第 129 包文件 |
| 02:38 `2f00b1a5` | — | W-14／W-15 第二次追問 → §0 #72／#73 |
| 02:41 `1523a564` | — | §0 #74～#76 |
| 03:38 `8b95b212` | — | TO_STEVEN：機台 Adam6024_Pressure 紅 |
| 03:48 `0ea42133` | — | CHAT_JIMMY：!126 在第 49 批 |
| 04:51 `10dc448b` | — | §0 #77（WinLibs 浮點） |
| 05:15 `c1513c8d` | — | INBOX 152／153 |
| 06:56 `89f03cca` | — | §0 #78（Steven 常設規則） |
| 07:09 `6177b5b8` | — | CHAT_JIMMY：N07 認領 OK |
| 07:42 `577c41c5` | 130 `75c49fd`（07:53） | 第四十七＋四十八批（§1） |
| 07:54 `3dfb0687` | — | 第 130 包文件；CLAUDE.md 36／34／2 |
| 08:25 `62276ffc` | — | §0 #79、INBOX 86 狀態、TO_ES02 E-07 |
| 08:59 `79158ea2` | — | TO_ES02 R178 更正、§0 #80、W-16 |
| 09:48 `fd1b71c5` | 131 `19b954b`（09:52） | 第四十九批（§1） |
| 09:54 `f147a919` | — | 第 131 包文件；§0 #70／#71、INBOX 150／151 |
| 09:57 `5967db16` | — | St02 MR !139（只有文件） |
| 09:59 `3c6b7fa4` | — | W-16 結案、W-17 |
| 10:09 `afc7b56f` | — | RULINGS_20261003 |
| 10:13 `b6acab52` | — | INBOX 154、§0 #81 |
| 10:18 `952f5d2a` | — | St02 MR !141（按鈕普查，只有文件） |
| 10:21 `d5c58657` | — | W-17 結案、INBOX 155 |
| 10:47 `4a8e4ea3` | — | RULINGS_20261003 第 8～10 條（#81＝A、Bin 面板 TFT、兩封信擬稿）＋交接註記 |
| 11:08 `28db124f` | — | NIGHT_REPORT 整份重寫 |
| 11:18 `a9fd23be` | — | 機台 10:48 快照鏡像 |
| 11:26 `fcb3f8e0` | — | TO_STEVEN S-18 |
| 11:37 `2e75b6ec` | — | RULINGS 第 11／12 條（#83、#82） |
| 11:47 `2e29eaf3` | — | 第 13 條＋CLAUDE.md（筆電改當協調者） |
| 12:06 `e8b15118`、12:09 `5c7b0dd1` | — | 協作機制、`tools/laptop_ops/`、每人心跳（第 14～17 條） |
| 12:14 `e52f9618` | — | 交接 12:2x（St02 C14b 認領 OK、S-21） |
| 12:22 `d06c869f`、12:23 `08c11ee7` | — | 第 18 條（機台 -O0、MC01）、team_status.py 換回 LF |
| 12:30 `db982352` | — | 更正筆電寫錯的「3＝TFT」 |
| 12:33 `0b5354cd` | — | 第 19 條（type 4、COM14） |
| 13:03 `c40a45c0` | — | 第 20 條（TFT D／E／A 交 St02，S-22） |
| 13:10 `20a4c926` | — | 第 19 條附註（NB2-2 A6 兩個檢查）、!144／!145 收進第 51 批 |
| 13:13 `db9f0430` | 132 `a071554`（13:2x） | 第五十批（§1） |
| 1003 13:2x 前後 `b8d4664c` | — | 第 132 包文件；CLAUDE.md START 普查 35／33／2；INBOX 147／150 ✅ |
| 13:46 `4a98160e` | — | 機台 13:28 快照鏡像（715 檔） |
| 13:46 `e74b341d` | — | TO_ES02：機台 5 支收進第 51 批；W-06 註記 |
| 13:52 `a40f0ac0` | — | Ifor 的兩題轉機台（W-19） |
| 13:59 `d52cfa49` | — | NB2 R188 派工；§0 #84 |
| 14:05 `97459b1d` | — | 第 21 條（#84＝A） |
| 14:12 `cee87742` | — | St01 14:00 三個發現交 E-031；A7 規格轉 St02；W-20 |
| 14:29 `77e94e16` | — | 第 22／23 條（你下班前「全部照建議」＋「允許參考 912」）；followup_due.py 修漏報 |
| 15:23 `1b5ea1a5` | — | 第 51 批 round c 交接（review6 衝突說明、!146／!147、MC01 0149／0093） |
| 15:35 `ade8394f` | — | St01 15:26 回覆；每日日報常駐卡（Steven）；W-18 第 1 次追問 |
| 15:50 `06d40c50` | — | Steven MR !148（日報技能，只有文件） |
| 15:51 `bc180b91` | — | St01 E-031 認領 OK；!148 註記 |
| 16:52 `c5acfae9` | — | Draft !70 計時器表提議交給 St02（跟 S-20 一起）；St01 不用另跑 !149＋!150 的 gate |
| 17:14 `45670f61` | 133 `04237aa`（17:16） | 第五十一批（§1） |
| 17:18 `12ad942c` | — | 第五十一批文件（第 133 包；INBOX 151／152 ✅） |
| — | 17:30 `e302e47` | README：機台 patch 怎麼收（第 52 批收 cpp 0151／0159、web 0095／0098；ScanKey 一串先不收） |
| 17:29～17:54 `b111825c`、`cc5a1ea7`、`6a82faeb`、`fcf9f534` | — | 第 52 批開工（gate b52a）；St02 認領 OK；S-20 擋住的 ScanKey 交給 St02 整合 |
| 18:09 `39d8c5b6`（St01 代 Steven） | — | 各交接檔的每日日報常駐卡（只有文件） |
| 18:14 `4ed8b24c` | — | !150 退出第 52 批（C14_BinDisp 4／95 紅） |
| 18:29 `5fcd211a` | — | MR !154（NB2-1 mutation_check.py 檔頭，只有工具）merge-tree 直接合 |
| 18:30 `f5f7276e` | 18:30 `e55658d` | W-19 第一次追問（TO_ES02＋README） |
| 18:55 `48813d05` | — | St02 ScanKey 比對結論；W-12 第二次追問 ⇒ §0 #85 |
| 19:23 `79978082` | — | St01 審查 !151 ⇒ 第 134 包暫停；NB2-1 任務 (g)；E-08；心跳帶 mr_seen.json |
| 19:43 `9e6c0afa` | 19:5x `a353ae5` | 機台派工 1～5 分派（NB2／ES02 E-09／Frank01 F-03）；St01 V 卡；README 回機台 |
| 19:51 `af050a59` | — | 追問：W-18 第二次、W-23 日報；W-24～W-28 時間更正 |
| 20:07 `e6c981cc` | — | MR !156（St01 laptop_ops 路徑參數，只有工具）merge-tree 直接合 |
| 20:18 `ad207c6d`、20:34 `7db5a527` | — | b52c gate 開跑；PE 檢查被防毒卡住 ⇒ NB2 卡 (k)；第 53 批分支給 St01 審 |
| 20:39 `41e6d314` | 20:3x `df49a74` | 派工 1 答案（NB2-1 R197）⇒ E-10、§0 #86；!150 進第 53 批 |
| 20:43 `bb03a4b3` | 20:4x `f7aa73f` | 派工 6 交 NB2-1 (l)；問 EastSun 時間點 |
| 20:57 `c7c5d373` | 20:55 `02a5409` | St01 V-6：機台 HOME 修正 3 個中度安全問題 ⇒ README 急件、§0 #87 |
| 20:58 `9db913c7`（St01） | — | St01 自己把 review6 推進 main（E-031，Steven 16:4x 授權，St01 gate 過） |
| 21:20 `0c7aaf43` | — | 「兩個整合者」規則提給 St01（St01 21:39 接受） |
| 21:26 `9cc8eb94` | 21:2x `567fa51` | 派工 6 答案（NB2-1 R200／U17）⇒ E-11、§0 #88；!155 測試 exe 改名 |
| 21:45 `3b63f43c`、22:00 `e3403015`（St01 代 Steven） | — | TO_ES02：Auto Height 請 EastSun 查 Yaskawa 手冊（6077h 單位／正負號） |
| **21:55 `9d8ad319`** | **134 `d45bcdd`（22:0x）** | **第五十二批（b52d）：Gear Ratio 手推量測＋M1 修正、W71～W73、機台 0151／0159／0095／0098、HTDESIGNER 0.163.0、St01 E-031** |
| 本批（第五十二批文件，含本份報告） | — | 第 134 包文件；§0 #86 ✅（Steven）；第 53 批 gate b53a 開跑 |
| **1004 00:08 `5d6a27ae`** | **135 `822b6b5`（00:1x）** | **第五十三批：機台 cpp 0160～0179 照收、!152／!153／!150／!155、整合修正** |
| 本批（1004 00:1x 文件） | — | 第 135 包文件；State Record 提醒＋S-24（RULINGS_20261004 第 1 條） |
| **1004 01:5x `519a9feb`** | **136 `004a93a`（01:5x）** | **第五十四批：NB2-1 !158（煞車判斷寫進 op log）、!159 後續、St02 !160（C16）、Ifor01 !162（inst）** |
| 本批（1004 01:5x 文件） | — | 第 136 包文件；NIGHT_REPORT 開頭更新；absence 哨兵分類；卡 S-25 |
| **1004 04:3x `8c9c27c5`** | **137 `368dc68`（04:3x）** | **第五十六批（含第五十五批）：NB2-1 !163＋補修正、St01 !164（S-17）、St02 !157／!166、NB2-1 !167、筆電哨兵分類** |
| 本批（1004 04:3x 文件） | — | 第 137 包文件 |
| **1004 06:1x `703effd1`** | **138 `8f34133`（06:1x）** | **第五十七批：NB2-1 !169（Index 不存在的軸立即完成）、St02 !168（計時器表核心）** |
| 本批（1004 06:1x 文件） | — | 第 138 包文件 |
| **1004 07:4x `54505e0b`** | **139 `79274e8`（07:4x）** | **第五十八批：NB2-1 !172（Servo ON 寫回編碼器）** |
| 本批（1004 07:4x 文件） | — | 第 139 包文件；Steven 代回 §0 #89～#93（#91 他選 B）；第 59 批 gate 開跑；開頭「接下來三步」更新 |
| **1004 10:2x `3ffb2b00`** | **140 `088a141`（10:2x）** | **第五十九批：NB2-1 !170（Index 下壓擋法）、St02 !171（S-09 開機 OCR）；!165 退回（FShow_Audit）** |
| 本批（1004 10:2x 文件） | — | 第 140 包文件 |
| **1004 11:4x `29ef9e3c`** | **141 `e7b00fa`（11:4x）** | **第六十批：St02 !165（ScanKey＋N07）、St01 E-034、E-038 Phase A、review6、NB2-1 !175（開機照 Mot_Table）** |
| 本批（1004 11:4x 文件） | — | 第 141 包文件 |
| **1004 13:3x `65c533be`** | **142 `bc7174c`（13:3x）** | **第六十一批：St01 E-043 第 1 顆（開機驗表只記錄）、NB2-1 !177（F5 關窗）** |
| 本批（1004 13:3x 文件） | — | 第 142 包文件 |
| **1004 15:0x `62074fbd`** | **143 `da1f2b0`（15:0x）** | **第六十二批：St02 !176（Observer SG 免權杖）、St02 !179（清零）、NB2-1 !178（F5 建置加速，選用）** |
| 本批（1004 15:0x 文件） | — | 第 143 包文件 |

---

## §6 我自己犯的錯與更正

- **St01 的 E-034 漏收五批**：St01 1004 03:15 在 FROM_STEVEN §1 說 E-034（照 0625／V912 修好移植樹裡 golden 0618 的「`==` 寫成沒作用」，7 個檔、模擬組態已代跑綠）可以直接收進第 55 批，我第 55～59 批都沒收，08:3x 對認領清單時才發現。原因：那一列寫在 FROM_STEVEN **§2（完成）**，`claims_pending.py` 只看 §1；03:1x 那一輪我正在處理 !163 煞車修正（b55a 停掉重排），§2 新增的列沒有逐列讀。改正：排進第 60 批；之後每輪也逐列看 FROM_STEVEN §2 新增的列（寫「ready for your batch」或附 MR 的，就排進下一批）。
- **1003 晚上漏讀兩次**：St01 18:30 對 !151 的審查（M1／M2）我 19:1x 才讀到（差一點就把有運動安全問題的第 134 包推出去）；機台 17:5x～19:33 的派工 1～6 我到 19:2x／20:4x 才看到（只看了最新幾顆 commit，沒逐顆讀 README.txt）。之後每輪讀 FROM_STEVEN 全部新列、機台 `git log` 列出全部新 commit 再判斷。
- **時間標籤寫早了**：19:3x／19:5x／20:4x 三輪的交接列比實際推送早約 10 分鐘（憑感覺寫），已更正 W-24～W-29；之後標時間前一律先 `date`。
- **兩個等待程式誤報**：PE 檢查監看用 inline `-Command`，比對到自己的命令列，6 分鐘就誤報（改成 .ps1 檔）；等 `^DONE` 又被 gate 自己那行「DONE (build failed…)」提早觸發（改成 `^DONE [0-9]`）。還有一次 merge-tree 輸出被 `head -4` 截掉，看不到 CONFLICT 行（重看完整輸出才發現 !159 有衝突）。
- **09:2x 誤報「第 49、50 批沒有第 130 包的程式」**：我拿 `577c41c5`、`13a57384` 這兩顆「只合文件」的 merge commit 去量，量錯了東西；其實 b19、b18 早就含第 130 包的程式（`0eb965d1`、`5bfef4d9`、`f30c284d` 都是祖先，`git merge-base --is-ancestor` 量過）。09:4x 更正。代價：停掉一輪 gate b50a（09:04 起，跟現在同一個 HEAD `36d575e6`），09:46 重開 b50b。教訓：「含不含某包」要拿那包的程式 commit 去量，不拿 merge commit。
- **控制字元哨兵掃錯樹好幾天**：`ctrlchar_scan.py` 寫死掃 `m0925` 這棵 worktree（最後停在 0930 18:36 的 `f5ad8ade`），所以 0930 晚上之後每晚掃的都是舊樹，報的「0 處」不算數。1003 08:30 改成吃參數（傳跟 main 同一個 commit 的 worktree）、只掃我們寫的區域、跳過 UTF-16。
- **10:1x 寫修補腳本時用了 heredoc，反斜線被減半**（CLAUDE.md 0927 記過的坑）：寫檔前的斷言抓到，沒寫進任何檔；改用 Write 工具寫成檔再跑。
- **`docs_push.sh` 不能新增檔案**：要建 `RULINGS_20261003.md` 時才發現它只能改已追蹤的檔；10:0x 補上（`git cat-file` 檢查＋`update-index --add`）。
- **第 130 包的上機卡寫錯 Contact 頁 START**：TO_ES02 1003 07:4x 那張卡（還有 GitHub 第 130 包 README、08:2x 的 E-07 A35）寫「Contact 頁 START 會真的啟動機台」。NB2 R178 對原始碼：golden `BtnStartClick` 只有開了「軟體控制按鈕」的三個客戶碼才啟動，HT9050 是 `CUSTOMER_CODE=957` ⇒ 出貨版只把主畫面兩格扭力框設成 10；自動 Offset（OS-1b）那半是對的。09:0x 在 TO_ES02 更正（`79158ea2`）。⚠ 我 10:0x 在 CHAT_JIMMY 寫「更正已進第 131 包的機台說明」不對：第 131 包 README 沒有這句，只有 knowledge 鏡像裡的 TO_ES02 帶到；README 的更正寫在 `gh_readme132.py`，第 132 包推出才會出現。
- **把 `NUMBER_PANEL_TYPE=3` 當成 TFT**（1003 10:4x：RULINGS_20261003 第 9 條、TO_ES02 10:4x、第 131 包 README）：我看程式註解判的（`BinDispBringUp_St02.cpp:266-267` 3 與 4 兩支都寫「新增 BinDisplay TFT」），沒查 golden 的設定頁。NB2-2 A5 查 golden `HandlerSys.dfm:661-666`：3＝兩位數彩色七段，4＝TFT。12:3x 更正（`db982352`），你改成 4／COM14（第 19 條）；第 131 包 README 是舊包不改，第 132 包的說明寫的是對的。
- **14:3x 寫 WAITING_REPLIES 時表格裡留了沒跳脫的 `||`，連犯兩次**：第一次讓 `followup_due.py` 把 W-12 當成已結案、漏報 29 小時；修工具時新寫的說明又寫進一次，推之前的空跑抓到。工具已改成只在沒跳脫的豎線切欄、欄數不對就大聲報。
- **15:1x 用 `git merge-tree` 試合 St01 review6 只看了輸出第一行（新 tree）**，以為沒衝突；後面其實列了兩個衝突檔。合併前再查一次才發現，改成逐處斷言後解，沒有合錯。
- **15:2x 在 gate 前跑 NUMCMP 的 cmake 設定，害 gate b51c 被防毒鎖檔卡在設定**；重開 b51c2 才過（多花約 5 分鐘）。
- **St02 四筆認領（11:1x～15:4x）筆電拖到 17:4x 才回**：巡檢每輪只看 FROM_STEVEN §3 與 CHAT 的新列，沒看 §1 的認領列（協作規則第 15 條要當輪回）。St02-M 17:28 提醒才發現；四筆都沒衝突，已批。之後每輪也讀 §1。
- **1004 01:0x 才發現機台的派工 8 漏派了約 70 分鐘**：機台 1003 23:53 推了第二份派工（`dispatch/20261003_functions_vs_1203/`，`dc075b2`），我 00:3x 巡查時看到這顆 commit 是分支最新的，卻當成派工 7（23:29 `75b31e2`，兩份標題都是 EastSun needs help），沒有打開來看。St01 00:38 的回覆寫「dispatch 7 @ 75b31e2 (unchanged at dc075b2)」才讓我回頭查。改法：巡查機台分支時逐顆列 `origin/main..` 之外的**新 commit 主旨與新增的 dispatch 資料夾**（`git diff --stat 上次..現在 -- dispatch/`），不只看最新一顆。
- **同一次查帳又找到兩個沒回機台的資料夾**（逐一比對機台分支 7 個 `dispatch/` 資料夾在我們文件裡有沒有被提到）：`20261003_homing_1203_skill`（22:43，EastSun 寫好要做成技能的「1203 回原點」知識）01:1x 照原樣收成技能 `.claude/skills/ht9050-1203-homing/`；`20261003_torque_units_manuals`（21:57，扭力單位＋驅動器手冊）St01 22:00 已拿來寫 Index Auto Height 的筆記（`e3403015`）、E-038 照它做，但沒人回機台「收到」。兩件都在 01:1x 的 README 補回。
