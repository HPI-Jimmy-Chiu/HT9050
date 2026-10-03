# 夜間報告 2026-10-02（五）晚 → 10-03（六）—— 進行中版本（最後更新 1003 17:2x）

> 這份從 1002 22:00 起算，整份重寫。0929 晚～1003 10:1x 的舊版（207 KB）在 git 歷史：`git show b6acab52:HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md`。
> 讀法：§0 只放要你決定的；§1 做完的都附 commit 或數字；§2 還在做或刻意沒做的；§3 哨兵抓到的；§4 清理；§5 推送清單；§6 我自己的錯。

## ★ 接手

現在 GitLab main 的程式＝**第 133 包 `45670f61`**（1003 17:14），GitHub 機台更新包今晚到現在出了五包：129 `aab3911`、130 `75c49fd`、131 `19b954b`、132 `a071554`、**133 `04237aa`**（17:16）。11:3x 起筆電當協調者（RULINGS_20261003 第 13～17 條）；第 52 批收集中（見 ★ 表）。

| 項目 | 現在 |
|---|---|
| GitLab main | 程式＝第 133 包 `45670f61`（17:14）；之後只會是文件（本份報告＝第五十一批文件） |
| GitHub 機台包 | 129 `aab3911`（02:29）、130 `75c49fd`（07:53）、131 `19b954b`（09:52）、132 `a071554`（13:2x）、133 `04237aa`（17:16）；舊包不動 |
| 第 132 包（✅ 13:2x 已推） | 共用快時鐘（RULINGS_20261002 第 7 條：加熱 20 ms、Bin 號碼面板 30 ms、GM-2 30 ms）；Teach Set All In／Out Arm Z＋Out Z All Down（All Down 在任何一軸在動時拒絕）；訊息框也停 Motor Test HOME／LoopMove（INBOX 150）；Gear Ratio 照 NB2 R171 修；St02 c912-1～4 改回 906（含拿掉 GPIB 遠端 START ⇒ START 普查 **35／33／2**）、C14 網頁頁、ADAM A3；St01 review6 E-030 第一部分；機台 WORKLOG 86-87 |
| 第 133 包（✅ 17:16 已推） | 機台建置改 -O0（第 18 條）＋整棵樹 `-fexcess-precision=fast`（第 7 條）；Arm Cell 第一次讀到 Z 離開原點就停 X／Y（R174 M1）；`CommaText` 照 BCB6（INBOX 152）；**Arm Cell 走或手動教導時任何來源的 HOME 都擋**（NB2 !146＝U14 ③）；**GO 時舊通知框開著就拒絕**（NB2 !147＝INBOX 151）；St02 !140 OB-7；Ifor !142；NB2 !143／!144／!145；St01 review6 到 `65323419`（引用整理、拿掉 912 才有的 Barcode CSV Compare 在 St01 的檔）；機台自己的 cpp 0144／0146～0149、web 0088／0089／0091～0093（含 JOG-MAXVEL、MT-COPYALL） |
| 第 134 包（⏳ b52c 模擬組態測試中） | 第 52 批 `v906/jimmy-b52b`：St02 !149（W71／W72／W73 照 912）、NB2-1 !151（Gear Ratio 手推量測＋GearRatioCal.csv 預設寫 `D:\HT9045_Log`）、機台自己的 cpp 0151／0159、web 0095／0098。gate：出貨組態＝基準 4 項＋cJSON（exe 建好後不見了，重新連結後單獨跑過＝環境）；模擬組態 19:1x 開始跑測試。**St01 18:30 審查 !151 找到 2 個中度運動安全問題 ⇒ 先不推**：M1 推過軸、沒存檔就結束量測時 HomeFlag 還是 1，之後絕對移動／Teach Go 會差一整段推的距離 ⇒ 交 NB2-1 修（20:30 前沒開工就筆電自己修），修好重跑 gate（b52c）再推；M2 按伺服 ON 時軸可能彈回推之前的位置 ⇒ ES02 卡 E-08：第一次上機只推 5 mm 以內、手離開再按伺服 ON。**20:1x 更新**：NB2-1 19:48 交 M1＋L1 修正（`efdb6d06`：推過沒存就結束也設 HomeFlag 0、頁面寫「手離開軸、站開」）＋ES02 HTDESIGNER 0.163.0 ⇒ `v906/jimmy-b52c` `68cd19e5`，gate 20:13 開跑；b52b 兩組態＝基準（出貨 cJSON、模擬 WebMotorAccess 都是環境，單獨重跑過；模擬的 PE 檢查卡在被防毒鎖住的 test_tcp_tester_real.exe，手動停掉）。**21:2x 更新**：b52c 出貨＝基準 4 ✅；模擬 PE 檢查又被防毒卡（第 3 次）→ 自動停掉、手動跑 ctest 中。**St01 20:58 自己把 review6 推進 main**（`9db913c7`，Steven 16:4x 授權；E-031＝重新產生標籤＋產生器工具，St01 兩組態 gate 過）⇒ 第 134 包改成 `v906/jimmy-b52d`＝b52c＋main：兩邊程式檔不重疊（只有 tests/CMakeLists.txt 檔尾都加測試），用「增量建置＋兩邊相關的測試」驗證後推，不再整輪重跑；同樣的規則寫給 St01（TO_STEVEN 21:2x） |
| 機台端派工 1～6（EastSun 1003 透過 Jimmy，GitHub `machine/integ-ioweb` README.txt） | ①Index 自動測高在 1203 上能不能用（⚠ 安全：門檻不觸發 Z 會一直往下壓）→ NB2-1（先唯讀分析＋列機台要量的）；②機台螢幕上被遮住／截斷的視窗清單＋修正 → ES02（E-09）；③4 個吸嘴吸一顆 IC → Frank01（F-03；週一 10:00 前沒認領改派 NB2）；④F5 期間關 HMI 視窗程式不停、視窗又跳回來 → NB2（預設：關窗＝不啟動／正常停掉 wb_serve、不再開窗）；⑤機台編譯太慢（改一個檔 61 秒，編譯只佔 3 秒）→ NB2（不碰防毒、不換 6.3.0）。機台自己的 cpp 0160～0169（1203 模組檢查、HOME 修正）排第 53 批，先請 St01 唯讀審查（W-28）。**20:4x 更新**：①NB2-1 R197 交了（今天不會動 Z，安全；照 golden 翻會卡在 555；危險的是把 6077h 直接接上）⇒ TO_ES02 E-10 量五項、§0 第 86 項；**⑥（新）開機後馬達好像激磁又放開** → NB2-1 (l)（機台端查到：(A) HOME 後照原版切電源約 4 秒、(B) 開機煞車放→鎖→放、(C) HOME 失敗 PAUSE 斷電，都照 golden；翻譯缺口是開機 Servo ON 沒到卡）；機台又推了 cpp 0170～0179（HOME 修正），第 53 批開 gate 時一起收 |
| 等你 | §0 剩 7 題：🔴 #88 開機放煞車（新，等 EastSun，建議 A）；🔴 #87 機台 HOME 修正的安全問題（新，預設 A＝照收、請 EastSun 修）；#86 Auto Height 防護（新，預設 B＝先量再翻）；其餘都是「等別人」（#85 Frank、#73 Jerry、#49 Ifor、#72 機台）；#70／#71／#47／#48／#33／#34 你 14:3x 回「全部照建議」（第 22 條），#84 回 A；#82／#83 你 11:2x 回了，TFT 三題 12:3x～12:5x 回了（第 19、20 條） |

**接下來三步**
1. 第五十二批（收集中）：St02 S-20 主迴圈＋N07 !137／!138（MR 來了才收，普查會變 36／34／2）、NB2-1 (d) Arm Cell 停全部馬達＋告警／(a)(f) Gear Ratio 手推＋紀錄資料夾／(b) CommaText 後續、St01 E-034（`==`→`=`）與 E-031、Ifor I-08（ATC 冷熱校正檔）、測試主表跟上機台快照（Mot_Table 15 列、IO_Table 24 列，要連測試預期一起改）、機台之後推的修正。
2. 等同事：St02 S-20 主迴圈＋N07、S-22 TFT D／E／A、C14b；NB2 手推量測（`v906/jimmy-gearhand` `8ce18104` 是半成品）；St01 S-19 備援演練、S-21 E-032 `==`；機台 W-14（`[FASTCLK]`、Bin 面板、ADAM）。
3. 每輪：`team_status.py`、`mr_scan.py`、`followup_due.py`、回同事的認領、更新筆電心跳。

---

## §0 要你決定的（分流表）

分流：🔴 上機前先看／🟡 新功能的設計小題／⏳ 等你說寄出或等別人回／「舊題」＝之前留下、還沒回的。號碼沿用原本的（別的檔有引用）。

| # | 事項 | 白話＋例＋選項（建議） | 你回之前的預設 | 在哪 |
|---|---|---|---|---|
| ~~83~~ | ~~🔴 **HT9050 出貨版第 132 包起照 golden 每 20 ms 跑加熱監控；機台 `HEATER_CTRL_TYPE=4`（COM12）但實機是網路型 DTM，溫度會讀成 999；室溫無影響（讀不到的警報只在運轉中、加熱或恆溫工單才跳）**~~ | ✅ **1003 11:2x 你回 A**（RULINGS_20261003 第 11 條）：加熱測試前 EastSun 先把 `HEATER_CTRL_TYPE` 改 3（先備份），或等 I-03b；已寫進第 132 包說明與 TO_ES02 | 照第 132 包機台通知：室溫照常；加熱測試前改 3 或等 I-03b | `74ae1585`（b18）合併說明；`machines/HT9050/snapshot/machine_params/D_HT9045_system/Gerneral.ini:302`／`:327` |
| ~~82~~ | ~~🔴 **訊息框跳出時也結束 Teach 的 Z All Up 回原點與 Light Scale 回原點（golden 不會結束，但那時馬達已經停了）**~~ | ✅ **1003 11:2x 你回 A：維持**（RULINGS_20261003 第 12 條） | A | b18 `ad4c0589`（`v906/jimmy-teachzdown`，還沒上 GitLab）；golden `uMotorTest.cpp:921-926`、`uteach.cpp:4505` |
| 88 | 🔴 **開機那幾秒，8 支 Z 軸的煞車可能在馬達電源還沒上來前就放開（EastSun 說的「激磁又放開」）** | 白話：EastSun 問「軟體一開起來，好像先激磁又放激磁」。NB2-1 查完（R200）：回原點時斷電約 4 秒、回原點失敗斷電都照 golden；但移植樹自己的「逐軸放煞車」在開機時看到上一輪留下的 SVON 就放開 8 支 Z 軸的煞車，那時馬達電源繼電器還沒 ON（1203 的 SVON 斷電不會掉），只剩 `SnMotorPower` 一道在擋；之後上電又鎖、4 秒後再放。例：Z 軸靠煞車撐著，煞車一放、驅動器又沒電，軸就可能往下滑。**A（NB2-1 建議）** 放煞車條件加回「上電完成且延遲數完」（開機只剩 golden 那一次；運轉中照 EastSun 0929 規則）；**B** 維持現狀。這是 EastSun 的規則，已在 21:24 請他先量四項、再決定（GitHub README、TO_ES02 E-11） | 等 EastSun（他同意 A 就改） | NB2-1 R200／URGENT U17；W-33 |
| 87 | 🔴 **機台自己寫的 HOME 修正有 3 個中度安全問題（St01 審查），機台現在就在跑** | 白話：今晚機台端（MC01＋EastSun）為了讓 1203 的整機回原點能用，推了 20 顆修正（cpp 0160～0179）。St01 讀了 0160～0170：**M-a** HOME 途中馬達電源掉了又回來，程式會自動對所有軸重置警報並重新激磁、繼續 HOME——就算是急停或開門造成的斷電也一樣（例：有人開門伸手進去，門一關、電一回來，軸就自己動起來）；**M-b** 真的有警報的軸每秒重試 20 次；**M-c** 驅動器回原點只看到 READY 就算完成，原點可能是錯的。已經在 20:55 透過 GitHub README 和 ES02 請 EastSun 先看、由機台端修。**A（建議）** 第 53 批照機台現在的樣子收進 main（機台本來就在跑，收進去不改變機台）、修正等 EastSun 做；**B** 先請 EastSun 把 M-a 改回 golden 的急停流程（斷電就停 HOME）再收；**C** 這些 HOME 修正先不收進 main（main 跟機台會越差越多，之後的 patch 會越來越難套） | A | FROM_STEVEN §3 1003 20:41（St01 V-6）；GitHub README `02a5409`；W-32 |
| 86 | 🟡 **HT9050 的 Index 自動測高（Auto Height）：解閘之前要不要加 golden 沒有的防護** | 白話：EastSun 問 Auto Height 在 1203 上能不能用。NB2-1 讀完（R197）：**今天不會動 Z**（還沒翻，安全）；照 golden 翻過來會卡住（等不到扭力讀值，golden 沒有逾時）；真正危險的是把 1203 的扭力讀值（6077h）直接接上——單位差 10 倍、正負號可能讓「壓到設定扭力就停」永遠不成立，Z 會一直往下到高度下限。例：門檻 40 %，6077h 讀到 -350（＝-35 %）⇒ golden 的 `>=` 永遠不成立。**A** 現在照 golden 翻，同時加三個移植專用的防護（扭力來源沒確認前拒絕執行並跳警報；case 555 等不到就停軸跳警報；扭力橋寫 |6077h|÷10）；**B（建議）** 先維持閘著，等機台照 E-10 量完五項（單位、正負號、上限有沒有用），再照 A 做；**C** 照 golden 原樣翻、不加防護（會卡在 555，沒有用） | B | NB2-1 R197（`v906/nb2-assist` README）；TO_ES02 E-10 |
| 85 | ⏳ **Frank 的 2×4 與放 Auto 盤修法（W-12）追了兩次沒回** | 白話：Frank 本人要回兩題——Carry kit／Index 吸嘴是不是 2×4（是的話 V906 自己加的 12 處 HT9050 判斷要拿掉）、`DoPlaceTrayToAuto_9050(0)` 怎麼修（固定放 Auto1 還是放缺盤的 Auto）。1002 09:3x 問，今天 14:3x、18:5x 追了兩次；Frank01 從 1002 07:47 就沒推過東西（週末）。**A** 打電話問一次；**B（建議）** 等週一（那 12 處不拿掉、Auto 修法照現狀） | B | `WAITING_REPLIES` W-12／W-01；TO_FRANK §4 |
| ~~84~~ | ~~🟡 **主畫面面板的實體 RESET 鍵要不要跑 golden 完整的 Reset**（NB2 R188 M3）~~ | ✅ **1003 14:1x 你回 A：維持現在這樣**（RULINGS_20261003 第 21 條；跟畫面 RESET 鈕同，0930 第 5 條） | A | NB2 R188 M3；St02 S-20 |
| ~~70~~ | ~~🟡 **Arm Cell 補強（NB2 R165）留下三個設計小題**（新功能、golden 沒有）~~ | ✅ **1003 14:3x 你回：全部照建議**（RULINGS_20261003 第 22 條）——①維持、等 ES02 量（E-08）②比照 HT160 停全部馬達＋跳告警 ③改成拒絕；②③交 NB2 | ①②③都維持現狀 | `v906/jimmy-armcell2` `8743153b`；NB2 R174 |
| ~~71~~ | ~~🟡 **Motor Test「Gear Ratio」分頁的小題**（②已改由第 81 項處理）~~ | ✅ **1003 14:3x 你回：全部照建議**（第 22 條）——①留備份 ③維持 60 秒沒進展 ④沒設時寫機台紀錄資料夾（交 NB2，跟手推一起） | 照現在的做法 | `v906/jimmy-gearratio` `5c43b54d`；NB2 R171 |
| 73 | ⏳ **Jerry（W-15）Timetick 的修改沒推** | 白話：Jerry 說他實驗過把主迴圈一拍（`kServeTickMs`，現在 500 ms）改小，你 1002 15:4x 說測過就收（RULINGS_20261002 第 19 條），但他一直沒推，追了兩次。你 10:3x 選「寄一封信」⇒ **1003 11:06 已寄出**（你 11:0x 說「寄出」；寄件備份已確認），等 Jerry 推分支＋MR | 等 Jerry 回；4 小時沒動靜照帳本再追 | RULINGS_20261003 第 10 條；`docs/handoff/WAITING_REPLIES.md` W-15；TO_JERRY §4 |
| 49 | ⏳ **Ifor 0922 的 V912 RotateKit 修正是哪個客戶、哪台機台、週報哪一列** | 白話：Ifor 0922 修了 V912 的一個問題（RotateKit 取料失敗、重試後手臂卡住）。V912 是量產維護版，修正要走案件流程（建 case、BCB6 建置、出 release note），所以要先知道是哪個客戶、哪台機、週報哪一列；這台和 Ifor 都沒有紀錄。例：「超豐 HT9046A 第 3 號機，週報第 8 列」。你 10:3x 說看不懂 ⇒ 筆電已白話說明，並寫信問 Ifor（0922 那份 RotateKit 分析是哪個客戶／哪台機）——**1003 11:06 已寄出**（寄件備份已確認；帳本 W-18）；你知道的話直接回一句更快 | 等 Ifor 回；Ifor 那條分支先不推 | RULINGS_20261003 第 10 條；FROM_IFOR 1002 08:4x；`D:\HT9045\backup\ifor_wip_20261001\V912_RotateKit_4710_Ifor20260922.patch`；W-18 追問兩次（1003 15:3x、19:5x）還沒回 |
| 72 | ⏳ **機台端（W-14）還沒回的兩件：ADAM-6024、接地監測板／OTD**（原第 54 項 W-07 併進來） | 白話：要機台給唯讀的現場事實。①**ADAM-6024**（EP 下壓力）：`ADAMTCP.dll` 在哪（跟 wb_serve.exe 同目錄嗎）、`Gerneral.ini` 的 `EP_Install`／`INSTALL_DOUBLE_EP`、172.16.8.110 回不回 ping——決定總開關 `W906_ADAM_EP_LIVE` 什麼時候能開。②**接地監測板／OTD**：機台 ini 寫 `USE_GROUND_MAN=0`、`USE_OTD=0`（cpp 0131 代答了 W-07），第 131 包 README 仍請機台確認現場有沒有裝。Bin 面板那一半你 10:3x 回了（RULINGS_20261003 第 9 條，§1）。**A（建議）**你或 EastSun 看一眼機台回一句；**B**再等機台下一次推送 | 再等；ADAM 總開關維持關 | `WAITING_REPLIES` W-14／W-07；GitHub 第 129、131 包「請回」 |
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

---

## §3 紅燈／回歸（哨兵抓到什麼）

- **防毒拖慢 → 每個 gate 階段幾十到上百支逾時（不是回歸）**：新建好的 exe 第一次啟動被掃毒卡住（CPU 只 13～28%）。單獨重跑：第 131 包出貨 103／103、模擬 68／68 全過；第 130 包剩 HSys_HeaterMix（398／0 全過但 183 秒）與 DTME08Cycle；第 129 包剩 DTME08Cycle（連不到的位址 connect 要 <1 秒，負載下 1.1～6 秒；1003 09:5x 請 Ifor 看）。第 132 包的 PE 截斷檢查也被拖了約 40 分（10:14～10:5x，同 INBOX 149 那一型），結果通過：338 個 exe/dll 結構完整。
- **gate 抓到的合併問題**：b49a 出貨組態連結失敗——St02 !128 的新測試直接編 `WebMotorAccess.cpp`，要 `GearCalc.cpp`（`a2ed9c57` 修）；C14_BinDisp 紅＝golden 在 type-3 埠上會送一串 14 個 Magazine-TFT 還原訊框，St02 測試的假匯流排把它們當成面板訊框（St01 代跑也紅），St02 `1616a034` 修、合進 `e1d913d7`。
- **機台回報 Adam6024_Pressure 紅（套第 129 包後；4q／5e／5f／9t）**：根因是機台用 WinLibs g++ 16.2，`double == 5.6` 判不相等（BCB6 與 g++ 6.3.0 判相等）；EP 總開關關著，只有測試不同。St02 A3（!135，第 132 包）先修他的檔；整棵樹對齊是第 7 條（第 51 批 fp）。
- **控制字元哨兵（線 D）**：發現它一直掃舊樹（§6），1003 08:30 修好後掃 b19：St01 `docs/handoff/AUDIT_IO_20260926.md:3` 的 `\r` 被寫成 CR（0927 講過兩次沒改，1003 09:0x 第三次請他修）；`atester_ProcessCount.cpp` 的 `\x0b` 兩處 golden 也有（照翻）；手冊與抽出的參考檔可忽略。
- **NUMCMP 普查（第五十批的樹，04:5x）**：PASS，A=2 B=11 B′=3 C=30。
- **配方母體**：66 個目錄／65 份有 Contact.Data（多的那個是 FT005054，已知）。
- **真實檔**：07:5x 單獨跑 HSys_HeaterMix 時看到 `system` 有變化，是同時在跑的 b49b 模擬 ctest 暫時寫真實檔又還原；之後 `system` 跟 01:0x 基準逐檔相同。
- **磁碟**：`.claude\worktrees` 的 build 輸出 10:3x 量 58 GB、13:2x 量 82 GB（第 132 包 gate 又加了 b18 的 12 GB）；13:2x 清掉 6 棵已上 main 的（§4），之後總量 17G。
- **gate b51b 抓到 ObserverCore**（兩組態都紅、單獨也紅）：測試餵沒加引號的 `Motion Part` 表頭卻預期整串讀回＝舊 vclcompat 的切法；BCB6 沒加引號的空白也會切。程式（第 51 批 CommaText）對、測試預期錯 ⇒ `0ef6970c` 改成 BCB6 結果＋有引號例。其他測試沒有同類失敗。
- **gate b51c 在 cmake 設定就失敗**（`CMakeDetermineCompilerABI_CXX.bin cannot be read`，防毒鎖剛產生的探測檔）：前 2 分鐘剛跑過 NUMCMP 哨兵的 cmake 設定。清 build、等 45 秒重開 b51c2 就過。之後 NUMCMP 排在 gate 之後跑。
- **NUMCMP（第 51 批 round c）**：PASS，A=2 B=11 B′=3 C=30，跟 1002 基準一樣。
- ⚠ **機台端暫時跳過「氣壓不足」告警**（MC01 cpp 0158 BYPASS-WAR1603，EastSun 1003 下午）：在機台的 `MachineType.h` 打開 `W906_BYPASS_WAR1603_AIR`，golden DoSystem 的 Air not enough 那一段不跳。**只在機台本地，筆電不收進共用的樹**；已請機台氣源修好就拿掉（TO_ES02 17:3x）。
- **機台端與 St02 重複做了面板鍵**（MC01 cpp 0152 SCANKEY 系列 vs St02 S-20）：兩份都先不收，請 St02 對過再決定以哪份為底（TO_STEVEN 17:3x）。
- **gate 的 PE 檢查被防毒卡住（今晚 2 次，環境不是回歸）**：b52b 模擬組態卡在 `tests\test_tcp_tester_real.exe`（19:09 起，CPU 0，重跑一次又卡在同一個檔）、b52c 出貨組態卡在 `tests\test_pause_forward.exe`（20:24 起）——剛建好的 exe 一開檔就停住，build.bat 到不了 ctest。做法：停掉那支檢查、在完整的 build 目錄手動跑 ctest（b52b 兩組態都等於基準；b52c 用 `b52c_rest.sh` 自動處理）。請 NB2 讓檢查「開檔逾時就報 LOCKED、不當成壞檔」（卡 (k)，W-29）。

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
| 本批（第五十一批文件，含本份報告） | — | 第 133 包文件；INBOX 151／152 ✅；TO_ES02／TO_STEVEN／TO_IFOR／CHAT |

---

## §6 我自己犯的錯與更正

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
