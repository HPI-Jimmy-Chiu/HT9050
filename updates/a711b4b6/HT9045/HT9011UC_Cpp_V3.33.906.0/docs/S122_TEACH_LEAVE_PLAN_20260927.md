# S122 查證與實作方案：關掉 Teach／Motor Test ⇒ 必須重新回原點

- 交件：ST01-E 派的工程師（這一趟只讀、不改程式），2026-09-27 15:4x
- 裁決：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md 第 2 條第 18 題＝**B**（只做「關掉 Teach／Motor Test ⇒ 標成必須重新 find home」；主畫面斷線或重新整理**不**暫停生產）。背景：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:333-340（S122）、D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md:300-306（Q35）。
- 核對基準：D:\HT9045 分支 `v906/steven-cbridge-review6` HEAD `fca6248c`（15:33；開工時是 `21c1545f`，中間進來 Q40 `76058840` 與 merge `1de5005b`，本文件行號都已對 `fca6248c` 重查）。
- 三棵樹的寫法：
  - 【移植樹】＝ D:\HT9045\HT9011UC_Cpp_V3.33.906.0\
  - 【golden V912】＝ D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\（cp950，轉成 UTF-8 副本讀，行號不變）
  - 【網頁】＝ D:\HT9045\web\

---

## 0. 結論（先看這段）

1. 「必須重新 find home」在 golden 就是全域 **`fAllMotorHome = false`**；移植樹同名、同型別（bool）、同語意。解除只有一條路：整機回原點做完（`DoHomeProcess` 裡 `CheckMotorHome()` 成立才設 true）。
2. **更正一個前提**：S122-1「比 golden 嚴」不成立。golden 關掉 Teach／Motor Test **本來就會**清 `fAllMotorHome`，只是清的地方不在 FormClose，而在呼叫端：
   - 【golden V912】main.cpp:28846-28847：`fTeach->ShowModal();` 回來的下一行就是 `fAllMotorHome=false; //Steven 20110211`
   - 【golden V912】uteach.cpp:2442-2443：Teach 裡的 Motor Test 鈕 `fMotorTest->ShowModal(); fAllMotorHome=false;`
   - 另外打開 Teach 也清：【golden V912】uteach.cpp:1610（`TfTeach::FormShow`，:1417 起）
   RULINGS_20260926 S122-1 的查證只看了 FormClose 本身（uteach.cpp:2080-2100、uMotorTest.cpp:1352-1367），沒看 ShowModal 的呼叫端。⇒ S122-1 其實是「把 golden 的『ShowModal 回來』翻到網頁的視窗開關」，是**翻譯**，不是偏離。NIGHT_REPORT 第 18 題與 FROM_STEVEN §3 22:30 給 Jimmy 的「比 golden 嚴」要更正（見 §6 J3）。
3. 移植樹現況：已經在清的有三處（每個 Teach／Motor Test 按鈕命令、Motor Test 關視窗的 formClose、Teach 頁 iframe 載入時的 editlist.get）。**缺的**是：Teach 視窗按 EXIT 關掉時 C++ 什麼都不知道（Teach 頁不送任何東西）；網頁整個關掉／重新整理時靠不靠得住，要看權杖搶不搶得到。
4. 方案：C++ 每 500 ms 看一次**現成的**視窗總表（background.html 的 `ui.windows.put`，5 秒心跳），`fTeach`、`fMotorTest` 各自從「在用」變成「不在用」＝關掉 ⇒ `SystemStart==false` 時 `fAllMotorHome=false`。全部放 **St01 新檔**＋**St01 自己的兩行**（同一行插入），不改網頁、不改 WebBridge、不動 Jimmy 的檔，**不需要函式指標**。頁面**不需要**送 page 名稱（總表每個視窗已經帶 `form`）。
5. 順帶查到一個要 Jimmy 看的風險（§6 J1）：【移植樹】FileRW\Teach.cpp:259 在 HMI 載入／重新整理時可能在**運轉中**把 `fAllMotorHome` 清掉，`DoAllProcess` 會在 csystem.cpp:1629 每一拍 return ⇒ 重新整理網頁可能讓生產停住、沒有警報。這跟第 18 題 B「重新整理不停產」相反。**由程式碼推得，未實測**。

---

## 1. 查到的事實

### 1.1 golden V912：「必須重新回原點」怎麼表示、誰設、誰讀

**兩層旗標**

| 旗標 | 宣告 | 意思 | 設成「已回原點」 | 清成「要回原點」 |
|---|---|---|---|---|
| `fAllMotorHome`（bool） | 【golden V912】cmydef.cpp:283、cmydef.h:246 | 整機回原點完成 | 只有兩處：csystem.cpp:10916-10918（`DoHomeProcess` :10874，`if(CheckMotorHome()){ iHome=0; fAllMotorHome=true; …}`）、AutoTeach\AutoTeach.cpp:1192 | 很多（31 個 .cpp 有 `fAllMotorHome=false`，grep 20260927 15:4x）。例：ckernel.cpp:2557 馬達錯誤、cinitial.cpp:4066 開機、uteach.cpp:1610（FormShow）／:2290（存檔）／:2443（Motor Test 回來）／:2943（伺服鈕）、main.cpp:28847（Teach 回來） |
| `MOT[i].HomeFlag`（int） | 【golden V912】Motor\mymotor.h:49 | 單一軸回原點完成（1） | 回原點流程逐軸設 1 | 回原點流程一進去全部清 0：uhome.cpp:1386-1395（`ProcessMotorHome` case 1，JerryYang 20250429「進入HOME流程強制清除home旗標」） |

`CheckMotorHome()`（【golden V912】csystem.cpp:16619-16625）＝所有軸 `HomeFlag==1`。

**`fAllMotorHome==false` 時擋什麼（golden V912）**

| 地方 | 行 | 行為 |
|---|---|---|
| START | main.cpp:4528 `TfMain::Start`，:6390 `if(fAllMotorHome==false)` → :6431 `Home("Home by Start"); iStartIn=0; return false;` | 按 START 變成先回原點；一般組態 `bHomeByStart=true`（:6427）。回原點做完 csystem.cpp:10925 `if(bHomeByStart==true)` 那一支**不設 SoftStop ⇒ 接著生產**；不是 START 觸發的回原點走 :10989-10991 `SoftStop=true`（停在原地） |
| One Cycle | main.cpp:4477 | `if(fAllMotorHome==false || iOneCycle) return;` |
| 主畫面按鈕 | main.cpp:4248-4270（`ProcessKeyFlush`） | CleanOut、TrayEnd 反灰；O01 開時 Reset 也反灰 |
| 生產流程 | `DoAllProcess` 各段 | `if(SoftStop==true || SystemStart==false || fAllMotorHome==false) return;` |
| 進 Teach | main.cpp:28840-28843 | `ShowErrorMessage("WAR16100",…)`（"Motor not home alarm !"，提醒、不擋） |
| Teach 存檔 | uteach.cpp:2270 | 沒回原點時才問 "Sure to Save? (確定要存檔?)" |
| 回原點本身 | main.cpp:7397 `Home()`，:7496-7500 | `if(fAllMotorHome==false){ iHome=0; BtnHome->Down=false; }`（這次一定是「開始回原點」，不是「中止」） |

**手動移動怎麼擋**（看單軸 `HomeFlag`，不是 `fAllMotorHome`）
- Motor Test「Go（到 Home Offset）」【golden V912】uMotorTest.cpp:1610-1622：`HomeFlag==0` ⇒ "Motor not home 馬達尚未歸零"，不動。
- Motor Test「Loop Move」uMotorTest.cpp:1321-1323：`HomeFlag==0` ⇒ 問 "Motor not home yet, sure to loop test?"。
- Teach 寸動／移動 `CheckCanMove` uteach.cpp:2461-2471：只看 EMG，**不看**有沒有回原點。

**Teach／Motor Test 的開關（golden V912）**
- `sbTeachingClick` main.cpp:28826-28855：:28829-28830 `if(SystemStart) return;`（運轉中進不去）→ :28832 權限 87 → :28835 `bResetMNet` → :28840-28843 WAR16100 → :28844 `BackUpOutputData` → :28845 `NewRecordProcess("MES2189","Enter Teach Form")` → :28846 `ShowModal` → **:28847 `fAllMotorHome=false`** → :28849-28854 「IO 被改過，要不要還原」。
- `TfTeach::FormShow` uteach.cpp:1417，**:1610 `fAllMotorHome=false`**。
- `TfTeach::FormClose` uteach.cpp:2080-2100：`fShow=false`、`btnStop->Click()`（:2096，停所有馬達）、`Set_Pitch_SetGroup()`；不碰 `fAllMotorHome`。
- Motor Test 只從 Teach 開：全樹 `fMotorTest->ShowModal` 只有 uteach.cpp:2442（grep 20260927 15:0x）；**:2443 回來就清**。uMotorTest.cpp 本身 0 處 `fAllMotorHome`；`FormClose` :1352-1367 只把 Position 1／2 寫回 MotorTest.ini、`PauseUT150Polling=false`。
- Teach 是 `ShowModal` ⇒ 開著的時候主畫面的 HOME／START 按不到（實體面板鍵照樣掃得到）。

### 1.2 移植樹的對應

| 項目 | 【移植樹】位置 |
|---|---|
| 旗標 | cmydef.cpp:287 `bool fAllMotorHome=false;`；cmydef.h:221 `SystemStart`、:222 `fAllMotorHome`、:223 `SoftStart`（都是 bool） |
| 設 true | csystem.cpp:7397-7400（`DoHomeProcess`，`CheckMotorHome` csystem.cpp:329-335） |
| 全部軸清 0 | uhome.cpp:949（`ProcessMotorHome` :706 case 1） |
| START | 網頁 START＝WS `start.run`（tools\wb_serve.cpp:5669）→ `TfMainWeb::StartFromWeb` → WebStart.cpp:3582 `if (fAllMotorHome == false)` → :3635 `Home("Home by Start"); … return false;` |
| 生產流程守衛 | csystem.cpp:695 `DoAllProcess`，:1629-2002 共 16 處 `SoftStop==true || SystemStart==false || fAllMotorHome==false` ⇒ return |
| Teach／Motor Test 開著時 MainProc 暫停 | csystem.cpp:30493-30494（`MainProc` :30196）經 `W906_FormFShow` → WebMotorAccessLive.cpp:1140 `W906_HookFShow` → `WebWindowRegistryFShowPolicy`（EastSun R8，MT-E3b/E3c） |

**已經在清 `fAllMotorHome` 的地方（移植樹）**

| # | 位置 | 什麼時候 | 看不看 SystemStart |
|---|---|---|---|
| a | WebMotorAccess.cpp:4137 `if (!be.GoldenSystemStart()) be.GoldenClearAllMotorHome();`（`MotorAccessDispatch` 開頭；本體 WebMotorAccessLive.cpp:563） | 每一個 source=`uteach`／`uMotorTest` 的 `motor.access`，**包括 Motor Test 關視窗送的 `formClose`** | 看：運轉中不清（MERGE-56bbf785 20260926） |
| b | FileRW\Teach.cpp:259 `g_formShown = true;  fAllMotorHome = false;`（`FileRW_Teach_Page` :253，WS `editlist.get tag=Teach`） | Teach 頁 iframe **載入時**（不是視窗打開時，見 1.4） | **不看** |
| c | FileRW\Teach.cpp:197（`IC_btnSaveClick`，golden uteach.cpp:2290） | Teach 存檔 | 運轉中存檔已被 R0927-7 擋（tools\wb_serve.cpp editlist.save） |
| d | JsonBridge\IoBtnPanelClick.cpp:385-399（golden uteach.cpp:3100-3130） | IO 頁按 SwFMotorBreaker／SwMotorRelay／SwServerON | — |

**移植樹沒有的（跟本案有關）**
- 主畫面 Teaching 入口是純網頁（【網頁】page\main.html:313 `sbTeaching:'teach'` → background 開視窗），**沒有進 C++** ⇒ golden `sbTeachingClick` 的 WAR16100、MES2189、IO 還原問答，以及 **:28847 那一行**都沒有對應。
- `ProcessKeyFlush` 沒翻（WebBridgeTags.cpp:2493 的註解）；`BtnOneCycleClick` 是空殼（forms\fMain.cpp:404）。
- 網頁沒有任何地方顯示 `fAllMotorHome`：tag `guard.allMotorHome`（WebBridgeTags.cpp:1248）、`pump.guard.allMotorHome`（:1216）有送；【網頁】底下 *.html／*.js 只有 page\screenshot_meta.js:6086 一筆紀錄（status "nohtml"）（`grep -rn allMotorHome D:\HT9045\web`，20260927 15:2x）。

### 1.3 具體軸的例子：Index Z1（`MTestZ1`，HT9050 的 M14）

HT9050 上 `MTestZ1`＝M14（【移植樹】WebMotorAccessLive.cpp:1151 的註解，出自 IO_Table.csv／Mot_Table.csv）。

1. 開機 → START（第一次）→ `fAllMotorHome==false` ⇒ Home by Start → `ProcessMotorHome` case 1 把 164 軸（含 `MTestZ1`）的 `HomeFlag` 清 0 → 逐軸回原點 → `CheckMotorHome()` 成立 → `fAllMotorHome=true` → 接著生產。
2. 停機（`SystemStart=false`），操作員開 Teach，在 Teach 頁把 `MTestZ1` 往下寸動 5000 pulse 找接觸高度 ⇒ 這一下 `motor.access`（uteach／jogN）就已經清 `fAllMotorHome`（WebMotorAccess.cpp:4137）。`MOT[MTestZ1].HomeFlag` 仍是 1（編碼器參考沒丟）。
3. 操作員**另開一個瀏覽器分頁**（或按實體面板 HOME 鍵）做一次整機回原點 → `fAllMotorHome=true`；回到 Teach 又用手推了一下 Z 軸（伺服關）或只是看一看就按 EXIT。
4. golden：EXIT 一回到主畫面 main.cpp:28847 就清 ⇒ 下一次 START 先回原點。移植樹現在：Teach 頁按 EXIT **不送任何東西** ⇒ `fAllMotorHome` 仍是 true ⇒ START 直接生產，`MTestZ1` 用的是舊的原點參考。**S122 要補的就是這一格。**
5. 補上之後：EXIT → C++ 下一拍看到總表 `fTeach` 由 open 變 closed ⇒ `fAllMotorHome=false` → START ⇒ Home by Start → `MTestZ1` 跟其他軸一起重回原點 → 接著生產。

### 1.4 Teach／Motor Test 兩個網頁怎麼跟 wb_serve 連

**實際檔名**
- 【網頁】page\HW.teach.html（760 行）＋ page\ht9045_wire_hwteach.js ＋ page\ht9045_wire_engine.js ＋ page\motor-access.js ＋ page\ht9045_recipe_client.js
- 【網頁】page\HW.MotorTest.html（1935 行）＋ page\ht9045_wire_hwmotortest.js ＋ 同上三支
- 宿主【網頁】background.html：WINDOWS 表 :460 `{id:'teach', form:'fTeach', src:'page/HW.teach.html', hidden:true, fixed:true}`、:461 `{id:'motortest', form:'fMotorTest', …}`；`MODAL_POLICY` :567-574 teach level 0、motortest level 1（全螢幕互斥層，Motor Test 疊在 Teach 上＝golden ShowModal）。入口 release.html／debug.html／debug9050.html 都載 background.html；main.html:313 另有 `sbMotorTest:'motortest'`（網頁可從主選單直接開 Motor Test，golden 只能從 Teach 開）。
- 其他同名但無關：Main.MotorView.html、Main.MotionView*.html、Alert.MotionView*.html（看板，不是 fTeach／fMotorTest）。

**「分頁」在這裡是什麼**：瀏覽器通常只有**一個**分頁（background.html）。Teach、Motor Test 是裡面的「視窗」（div＋iframe），而且非 lazy ⇒ 開站就載入 iframe；關視窗只是藏起來（background.html:780-800 `closeWin`，`display:none`），iframe 不卸載、頁面自己不會知道（所以才有 HT_WIN 訊息）。全螢幕層沒有最小化（:774 `if(modalStack.indexOf(id)>=0) return;`）。

**連線**
- **每個 document 各一條 WebSocket**：ht9045_recipe_client.js:141 `connect()`，每個 iframe 自己一份 `sock`。background.html 自己一條（:312 載 client）用來推視窗總表、收 tag；Teach iframe、Motor Test iframe 各自一條。連線本身**不帶**「我是哪一頁」。
- **wb_serve 怎麼知道某個視窗關了**——只有「視窗總表」這一條：
  - background.html：`openWin` :746／`closeWin` :780 → `setWinState` :665 → `scheduleRegistry`（60 ms）→ `pushRegistry` :849 → WS `ui.windows.put`（免權杖 WebBridge\WebBridgeServer.cpp:1446-1448；防連點白名單 WebCmdGuard.cpp:81）。每個視窗帶 `form`（golden 表單名）與 `state`（never／open／minimized／closed），`buildRegistry` :826-842。開站起點 teach／motortest 是 `never`（:913）。連著時**每 5 秒心跳重送全量**（:1088-1092）；重連後重送全量（:1076-1080）。
  - C++ 收：tools\wb_serve.cpp:5820-5840（主分派），另外三個等待迴圈 :631-642、:847、:6791 也收 → `WebWindowRegistryPut`（WebWindowRegistry.cpp:85）**按 connId 各存一份**。
  - C++ 查：`WebWindowRegistryQuery` WebWindowRegistry.cpp:171（只要有新鮮的回報就只看新鮮的；全部過期才照契約 §6 往「開著」倒）；`WebWindowRegistryFShowPolicy` :365（從沒收過任何總表 ⇒ false）。**過期＝15 秒沒新訊框**（:16 `kStaleAfterMsDefault = 15000`；`NowMs` 是 `time(0)*1000`，秒級）。
- **連線關閉事件：沒有對外介面**。WebBridgeServer.cpp:887-896 `CloseConn` 只在內部放掉權杖、減 `LiveWebSocketCount`；WebWindowRegistry.h:40-48 自己寫明「今天沒有對外暴露連線關閉事件，所以用年齡認定 stale」。伺服器 ping 15 秒、45 秒沒聲音就斷（WebBridge\WebBridgeServer.h:94-95）。
- **beforeunload**：這兩頁不用。Motor Test 用 `pagehide`（HW.MotorTest.html:1082，盡量送 formClose）；Teach 沒有。
- **Motor Test 頁自己的關頁通知**（EastSun MT-E3／MT-FIX1）：HT_WIN（background.html:678-683 `postWinState`）→ HW.MotorTest.html:1033 `onHtWin` → :1073 `formClosePage` → :1007 `syncCppForm` → `motor.access formClose`（被拒時 1.5～10 秒重送到 ack）；頁面重新載入時 HT_WIN `initial` 為關 ⇒ 再送一次 formClose（:1042-1046）。C++ 端走 WebMotorAccess.cpp:4137 ⇒ 已經會清。
- **Teach 頁**：**沒有** HT_WIN／pagehide／formShow／formClose（`grep` HW.teach.html、ht9045_wire_hwteach.js 0 筆，20260927 15:1x）。只有 iframe 載入時的 `editlist.get`（ht9045_wire_engine.js:2046 `attach` → :2108 `load()` → :1298-1299 → :1188 `gbLoad` → `HT9045Recipe.editlistGet`，要權杖：ht9045_recipe_client.js:412-415、:220-223）以及每次按鈕的 `motor.access`。`editlist.get` **不在**免權杖清單（WebBridgeServer.cpp:1448 只豁免 contactct.get／counterclear.get／observer.get）⇒ 開站時一群 iframe 搶同一個權杖，Teach 那一條不一定搶得到。

### 1.5 行的主人（`git blame`，HEAD `fca6248c`；git 作者 jimmychiu＝Jimmy 筆電、HT9045 Machine (V906)／EastSun-machine＝機台端 EastSun、Steven＝Steven01／Steven02）

| 範圍 | 主人（commit） |
|---|---|
| 【移植樹】WebWindowRegistry.cpp:85-170（Put）、.h 全檔 | Jimmy（P6-a `f21860e6`、P6-b `22b485bc`） |
| 【移植樹】WebWindowRegistry.cpp:171-240（Query 的「新鮮蓋過過期」） | Jimmy `f21860e6`＋機台端 EastSun（MT-FIX1 `7414cc6f`） |
| 【移植樹】WebMotorAccess.cpp:4117-4140（dispatch 開頭，:4137 那一道） | Jimmy（W4-a `3878bcc9`、W5-b `a3db68f2`）＋機台端合併（MERGE-56bbf785 `60c6f75f`，加上 SystemStart 條件） |
| 【移植樹】WebMotorAccessLive.cpp:555-570（`GoldenClearAllMotorHome`） | Jimmy `a3db68f2` |
| 【移植樹】WebMotorAccessLive.cpp:1185-1200（fShow hook 安裝） | 機台端 EastSun（MT-E3c `2507c383`、MT-FIX1b `3b49a063`） |
| 【移植樹】FileRW\Teach.cpp:250-262（:259） | Jimmy（W5-a `ad7561d4`；:259 W5-b `a3db68f2`） |
| 【移植樹】tools\wb_serve.cpp:5805-5845（主分派的 ui.windows.put） | Jimmy（P6-a `f21860e6`） |
| 【移植樹】tools\wb_serve.cpp:625-650（等待迴圈的 ui.windows.put） | 機台端 EastSun（MT-FIX1 `7414cc6f`）＋Steven（S0 `a165cc0c`）＋Jimmy |
| 【移植樹】tools\wb_serve.cpp:4598（MotorAccessTick 那一行） | Jimmy／機台端（Steven `6db687d4` 已把 St01 的 tick 搬走、原樣還原） |
| 【移植樹】**tools\wb_serve.cpp:5953**（S113／S97／S121 那一行） | **Steven01**（`6db687d4`） |
| 【移植樹】**CMakeLists.txt:3368**（`WebOlp.cpp  WebCmdGuard.cpp` 那一行） | **Steven01**（S107-3） |
| 【移植樹】WebBridge\WebBridgeServer.cpp:887-896（CloseConn）、:224（connId） | Jimmy（WB-0 `2eaee455`、CONNID `19844f8e`、MODAL-WAKE） |
| 【網頁】background.html:664-870、:1055-1095（四態、開關、總表） | Steven（`66458981` 進版控；總表是 Steven 20260918 W906-FW-WINREG）＋機台端 EastSun（MT-E3 `00ec3fb6`：`postWinState` :671-683、5 秒心跳 :1083-1092；lazy 視窗 `96175092`） |
| 【網頁】page\HW.MotorTest.html:995-1090 | 機台端 EastSun（MT-E3 `00ec3fb6`、MT-FIX1 `69b0fd13`） |
| 【網頁】page\HW.teach.html | Steven（`a016aa01` 進版控）＋Jimmy（W5-b `a3db68f2`、`378fbb77`） |
| 【網頁】page\ht9045_recipe_client.js、page\ht9045_wire_engine.js | 登記在筆電（TO_STEVEN §1「第 13 條」那一列） |

登記簿（D:\HT9045_handoff\TO_STEVEN.md §1，13:00 快照＋origin/main 重查）：本方案要動的檔**沒有**被登記；Q40 同事（form.event）已在 `76058840` 交件，現在工作樹 tools\wb_serve.cpp／CMakeLists.txt 沒有未 commit 的修改（15:36 `git status`）。實作前仍照規矩重查一次。

### 1.6 順帶查到、跟本案有關的風險

1. **FileRW\Teach.cpp:259 運轉中也清**（Jimmy 的行，§6 J1）。Teach iframe 是開站就載入的（background.html:906-907，非 lazy 直接給 src）⇒ 每次開 HMI／重新整理，Teach 頁 `DOMContentLoaded` 就送 `editlist.get Teach` → `FileRW_Teach_Page` → `fAllMotorHome=false`，**不看 SystemStart**。生產中重新整理網頁、而 Teach 那一條剛好搶到權杖 ⇒ `SystemStart` 仍是 true、`fAllMotorHome` 變 false ⇒ `DoAllProcess` 在 csystem.cpp:1629 每一拍 return ⇒ 排程停住、沒有警報。golden 不會：運轉中進不了 Teach（main.cpp:28829）。同一個問題在 WebMotorAccess.cpp:4137 已經在 20260926 用 `!SystemStart` 修過（MERGE-56bbf785 的註解就是在講這件事）。**未實測，由程式碼推得**；會不會發生取決於開站時哪個 iframe 先拿到權杖。
2. **motor.access 在回原點進行中不擋**：WebMotorAccess.cpp:4172 只擋 `SystemStart`；`grep SoftStart WebMotorAccess.cpp` 0 筆。golden 的 Teach 是 ShowModal，回原點時開不了；網頁用第二個 HMI 分頁做得到。不在本案，列給 Jimmy 知道（§6 J4）。
3. golden `sbTeachingClick` 另外三件沒翻：進 Teach 的 WAR16100、MES2189 事件紀錄、關 Teach 後的 IO 還原問答（main.cpp:28840-28854）。不在本案（§6 J4）。

---

## 2. 判斷規則：什麼叫「關掉了」

### 2.1 規則（C++ 端，每 500 ms 一拍）

1. 「Teach 在用」＝ `WebWindowRegistryFShowPolicy("fTeach")`；「Motor Test 在用」＝ `WebWindowRegistryFShowPolicy("fMotorTest")`。**跟 MainProc 暫停用的是同一個判斷**（csystem.cpp:30493-30494 經 WebMotorAccessLive.cpp:1140），所以「C++ 不再把 Teach 當開著、MainProc 恢復」那一刻，就是「標成要重新回原點」那一刻，兩件事不會分岔。
2. 兩個表單**各自**記上一拍的值。某一個由「在用」變「不在用」＝**關掉**。（分開記，才照得到 golden uteach.cpp:2443：從 Teach 開的 Motor Test 按 EXIT 回到 Teach，這一下就要清。）
3. 關掉時：`SystemStart==false` ⇒ `fAllMotorHome=false`；`SystemStart==true` ⇒ **不清**，主控台記一行（理由見 R3）。
4. 開啟的那一下（「不在用」變「在用」）也清＝golden FormShow uteach.cpp:1610（**要不要做見 R2**）。
5. 「在用」的定義完全沿用總表現有的政策（契約 §6＋MT-FIX1），本案不改它：
   - 瀏覽器說 `open`／`minimized` ⇒ 在用。
   - 說 `never`／`closed` ⇒ 不在用。
   - 有好幾條連線時：**只要有新鮮（15 秒內）的回報，就只看新鮮的**（新鮮的之間取聯集）；全部都過期、而最後說過 `open` ⇒ **仍算在用**（保守）。
6. 每一個邊緣都印一行（不管 `fAllMotorHome` 原本是什麼），上機才看得到判斷有沒有發生。

### 2.2 為什麼這樣分得出「關掉」與「只是切走」

- **重新整理** 會讓 background.html 重新載入，`WIN_STATE` 重設成 `never`（:913），新連線第一個總表就說 `never`；舊連線最後說的 `open` 最多 15 秒後過期 ⇒ 新鮮的蓋過過期的 ⇒ 變「不在用」。
- **單純斷線重連**（頁面沒重新載入）：background.html 重連後送的是**目前的** `WIN_STATE`（Teach 還開著就是 `open`）⇒ 不會變「不在用」。
- **切到別的瀏覽器分頁／把瀏覽器縮小**：`WIN_STATE` 不變；心跳可能被瀏覽器節流變慢，最壞是 15 秒後過期，過期仍算「在用」⇒ 不會變「不在用」。
- **同一個 HMI 分頁裡切到別的視窗**：Teach／Motor Test 是全螢幕互斥層，切不過去；也沒有最小化鈕。

### 2.3 例子（Teach 為例；Motor Test 同理）

| # | 情境 | 總表看到的 | 算不算關掉 | 何時生效 |
|---|---|---|---|---|
| 1 | Teach 按 EXIT（或 ✕） | `fTeach`: open → closed | **算** | 下一拍（≤ 0.5 秒＋60 ms） |
| 2 | 從 Teach 開 Motor Test，Motor Test 按 EXIT 回到 Teach | `fMotorTest`: open → closed；`fTeach` 仍 open | **算**（fMotorTest 那一個；golden uteach.cpp:2443） | 下一拍 |
| 3 | Teach 開著按 F5 重新整理 | 新連線 `never`；舊連線 `open` 到過期 | **算** | ≤ 15 秒（舊連線過期時） |
| 4 | Teach 開著直接關掉瀏覽器，之後沒人開 | 只剩過期的 `open` | 先**不算**（仍當開著；MainProc 也仍暫停） | 下一次任何人開 HMI、第一個總表送到時才算 |
| 5 | 瀏覽器當掉／電腦睡眠 | 同 4 | 同 4 | 同 4 |
| 6 | 網路／WS 斷線後自動重連（頁面沒重載） | 重連後仍 `open` | **不算** | — |
| 7 | Teach 開著切到別的瀏覽器分頁 2 分鐘 | 心跳變慢或過期，仍 `open` | **不算** | — |
| 8 | 同一台電腦開兩個 HMI 分頁，A 開著 Teach 切走被節流、B 一直新鮮送 `never` | A 過期後被 B 蓋掉 | **會算（誤判）** | A 過期時（≥ 15 秒） |
| 9 | Teach EXIT 後 0.3 秒內又開 | 可能兩拍都看到 open | 可能漏掉這一下 | 下一次關掉時照樣算 |
| 10 | 運轉中（SystemStart）Teach 由開變關 | open → closed | 看到了但**不清**，只記一行 | —（R3） |

例 8 的誤判代價：只是下一次 START 先回原點。而且 golden Teach 開著的整段期間 `fAllMotorHome` 本來就是 false（FormShow uteach.cpp:1610 已清），所以這個誤判不會造成 golden 沒有的狀態。
例 4／5 的空窗：這段期間網頁上按不到 START（沒有網頁），MainProc 也因為「Teach 仍開著」暫停；唯一的漏洞是實體面板 START，而且只在「Teach 開著時有人另外做過整機回原點、之後又沒碰任何 Teach 按鈕」才有差（見 §7）。

---

## 3. 實作步驟

**前置**
- 0-1. `git fetch`，重讀 TO_STEVEN §1（origin/main）與 FROM_STEVEN §1（origin/v906/steven-handoff），確認下列檔／行沒有人登記。
- 0-2. 在 FROM_STEVEN §1 認領：「S122（RULINGS_20260927 #18=B）：新檔 `WebTeachLeave.h/.cpp`、`tests/test_teachleave.cpp`；`CMakeLists.txt:3368` 同一行；`tests/CMakeLists.txt` 檔尾；`tools/wb_serve.cpp:5953` 同一行（St01 的 tick 行）」。
- 0-3. 動 tools\wb_serve.cpp 前先量兩組態語法基準（上次是 0 錯／102 警告，Q40 進來後要重量）。

**步驟**（路徑都在【移植樹】）

| 步 | 檔 | 位置 | 做法 | 主人 |
|---|---|---|---|---|
| 1 | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h | 新檔 | 宣告純邏輯（見下）＋`W906_TeachLeaveTick` | St01 |
| 2 | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp | 新檔 | 只 include `WebTeachLeave.h`、`WebWindowRegistry.h`、`<cstdio>`；**不 include cmydef.h**（旗標用參數傳進來），所以可以只跟 WebWindowRegistry.cpp＋cJSON 一起連進 ctest | St01 |
| 3 | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt | :3368（`add_executable(wb_serve` 清單裡 `WebOlp.cpp  WebCmdGuard.cpp   # AI(W906-CMDGUARD)…` 那一行，St01 的） | **同一行**：在 `WebCmdGuard.cpp` 後、`#` 前插 `  WebTeachLeave.cpp`，註解尾端補 `AI(W906-S122) 20260927：…`。行數不變 | St01 |
| 4 | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp | :5953（`}  if (pumpBeat) { … W906_MainRecordTimer1Tick(); …}  { extern void W906_SortCTTimer1Tick(); W906_SortCTTimer1Tick(); }  /* AI(W906-PROD-S113) …*/  { extern bool W906_ServeQuitDue(); … }` 那一行，St01 的） | **同一行**：在 `W906_SortCTTimer1Tick(); }` 之後、`/* AI(W906-PROD-S113)` 之前插：`  if (pumpBeat) { extern void W906_TeachLeaveTick(bool*, bool); W906_TeachLeaveTick(&fAllMotorHome, SystemStart); }  /* AI(W906-S122) 20260927 … */`。`fAllMotorHome`／`SystemStart` 由 wb_serve.cpp:78 的 `cmydef.h` 看得到。行數不變。**用內容找行，不要只信行號**（`grep -n "W906_ServeQuitDue()) break"`） | St01 |
| 5 | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_teachleave.cpp | 新檔 | 用 `WebWindowRegistryPut` 餵訊框、`WebWindowRegistryAgeConnForTest` 讓舊連線過期、`WebWindowRegistryResetForTest` 分隔案例；測 §2.3 的 1～8、10（案例見 §4.1） | St01 |
| 6 | D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt | 檔尾（現在 4602 行）附加 | 比照 test_winregistry（:3085-3097）＋test_webcmdguard 的 `-Wall;-Wextra`：`add_executable(test_teachleave test_teachleave.cpp ${CMAKE_SOURCE_DIR}/WebTeachLeave.cpp ${CMAKE_SOURCE_DIR}/WebWindowRegistry.cpp ${CMAKE_SOURCE_DIR}/Public/cJSON.c)`、include `${CMAKE_SOURCE_DIR}`、WIN32 連 ws2_32、`add_test(NAME WebTeachLeave …)`、TIMEOUT 60 | St01 |
| 7 | 文件 | — | 更正 S122「比 golden 嚴」的說法（RULINGS／INBOX／NIGHT_REPORT 第 18 題由誰寫由誰改）；FROM_STEVEN §2 交件、§3 給 Jimmy 的 J1～J4；ht9050-construction todo／done 由登記的人處理 | St01-M／各主人 |

**要不要函式指標：不用。** 本體只編進 wb_serve、也只在 wb_serve 呼叫；ctest 連的是純邏輯那一半（不含 `fAllMotorHome`），不會出現「lib 裡要呼叫 wb_serve 才有的本體」那種情況（wbserve-conventions §3 的條件不成立）。

**不需要改的**：網頁（background.html 已經帶 `form`、已經有心跳）；WebBridge；WebWindowRegistry；WebMotorAccess（:4137 那一道照留，跟新的 tick 重複清無害，兩邊都是寫 false）。

**程式骨架**（給實作參考；名字可改，邏輯照 §2.1）

```cpp
// WebTeachLeave.h  —— AI(W906-S122) 20260927 [W906]
namespace ht9045 {
struct TeachLeaveState { bool teachWas = false, mtWas = false; };
enum TeachLeaveAct  { kTLNone = 0, kTLClear = 1, kTLSkipRunning = 2 };
struct TeachLeaveVerdict { TeachLeaveAct act; const char* form; bool closed; };  // form/closed：第一個邊緣
// 純邏輯：兩個表單各自比上一拍；clearOnOpen＝R2 的答案（A＝true）
TeachLeaveVerdict TeachLeaveStep(TeachLeaveState& s, bool teachInUse, bool mtInUse,
                                 bool systemStart, bool clearOnOpen);
// 讀總表（FShowPolicy）＋套用；給 ctest 用（自帶 state）
TeachLeaveVerdict TeachLeaveTickWith(TeachLeaveState& s, bool* allMotorHome, bool systemStart);
}
void W906_TeachLeaveTick(bool* allMotorHome, bool systemStart);   // wb_serve:5953 呼叫；static state

// WebTeachLeave.cpp 核心
TeachLeaveVerdict TeachLeaveStep(TeachLeaveState& s, bool t, bool m, bool run, bool clearOnOpen) {
    TeachLeaveVerdict v = { kTLNone, "", false };
    struct F { bool* was; bool now; const char* form; } f[2] = {
        { &s.teachWas, t, "fTeach" }, { &s.mtWas, m, "fMotorTest" } };
    for (int i = 0; i < 2; ++i) {
        const bool closed = *f[i].was && !f[i].now, opened = !*f[i].was && f[i].now;
        *f[i].was = f[i].now;
        if (!closed && !(opened && clearOnOpen)) continue;
        if (v.act == kTLNone) { v.form = f[i].form; v.closed = closed; }
        v.act = run ? kTLSkipRunning : kTLClear;
    }
    return v;
}
TeachLeaveVerdict TeachLeaveTickWith(TeachLeaveState& s, bool* amh, bool run) {
    const TeachLeaveVerdict v = TeachLeaveStep(s, WebWindowRegistryFShowPolicy("fTeach"),
                                               WebWindowRegistryFShowPolicy("fMotorTest"), run, kClearOnOpen);
    if (v.act == kTLClear && amh) { const bool was = *amh; *amh = false;
        std::printf("[S122] %s %s -> fAllMotorHome=false (was %d; golden main.cpp:28847 / uteach.cpp:2443%s)\n", …); }
    else if (v.act == kTLSkipRunning)
        std::printf("[S122] %s %s while SystemStart=1 -> not cleared (golden cannot reach this: main.cpp:28829)\n", …);
    return v;
}
```

註解照 wbserve-conventions §7：這是**照 golden**（V912 main.cpp:28847、uteach.cpp:2443、uteach.cpp:1610），另寫「[W906] 不在 golden：用視窗總表的邊緣代替 ShowModal 回來；運轉中不清（golden 到不了這個狀態）」。

---

## 4. 驗證

### 4.1 這台（Steven01）：不跑 wb_serve，只做語法檢查

- 兩組態（SIM＝預設、SHIP＝加 `-DW906_NO_SOFT_SIMULTE`）對 `WebTeachLeave.cpp`、`tests/test_teachleave.cpp`、`tools/wb_serve.cpp` 各跑一次（指令見 D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md §6）；新檔要 0 錯 0 警告（`-Wall -Wextra`），wb_serve 跟改之前量的基準相同（commit 本文寫 `= base`）。
- `git diff -U0` 檢查：tools\wb_serve.cpp 與 CMakeLists.txt 每個 hunk 都是 `-N +N`（一行換一行）；tests\CMakeLists.txt 只有檔尾一個 hunk。
- 手上沒有其他工作時（Steven S159）：在 St01 自己的 build 目錄只 build `test_teachleave` 一個目標並跑（純邏輯、不讀寫任何機台檔、秒級）。看 exe 時間戳與 exit code（`-k` 會留舊 exe）。
- 交件寫明「wb_serve 未 build、未跑」。
- ctest `WebTeachLeave` 的案例：

| 案例 | 餵法 | 期待 |
|---|---|---|
| 從沒收過總表 | 不餵，tick | 無邊緣、旗標不變 |
| EXIT | conn1 teach open（tick）→ 旗標設 true → conn1 teach closed（tick） | 第二拍 kTLClear、旗標 false、form=fTeach、closed=true |
| Motor Test 回到 Teach | teach open＋motortest open → motortest closed、teach open | kTLClear、form=fMotorTest |
| 重新整理 | conn1 open；conn2 never（tick：仍在用、無邊緣）→ Age(conn1, 20000)（tick） | 第二拍 kTLClear |
| 關瀏覽器沒人再開 | conn1 open → Age(conn1, 20000)（tick） | 無邊緣（過期仍在用）；之後 conn3 never（tick）⇒ kTLClear |
| 切走被節流 | conn1 open → Age 20 s（tick）→ conn1 又送 open（tick） | 都無邊緣 |
| 斷線重連 | conn1 open → conn4 open → Age(conn1)（tick） | 無邊緣 |
| 運轉中 | open（tick）→ closed＋systemStart=true（tick）→ 再 tick systemStart=false | 第二拍 kTLSkipRunning、旗標不動；第三拍無邊緣（照 R3=A，不補清） |
| 別的表單 | fContact open → closed | 無邊緣 |
| 開啟那一下（R2=A 才有） | never → open | kTLClear、closed=false |

### 4.2 上機（機台端或筆電 F5「Web HMI」，模擬組態；RULINGS_20260927 §4：模擬驗過就算完成，實機要看什麼寫進夜間報告）

看主控台 `[S122]` 那一行＋瀏覽器主控台 `HT9045Tags.get('guard.allMotorHome')`：

1. START（模擬）做完回原點 → `guard.allMotorHome` 是 true。
2. 主選單 Teaching → （R2=A 時）主控台 `[S122] fTeach opened -> fAllMotorHome=false`。
3. Teach 按 EXIT → 1 秒內 `[S122] fTeach closed -> fAllMotorHome=false (was …)`。
4. 按 START → 主控台 `start.run: StartFromWeb returned false (SoftStart=1 …)`（tools\wb_serve.cpp:5669 起），事件紀錄出現 `MES2112 HOME pressed / Home by Start`（【移植樹】forms\fMain.cpp:776，golden 同字）→ 全部軸回原點 → `guard.allMotorHome` 回 true → **不用再按一次就接著生產**（bHomeByStart）。
5. 開 Teach → F5 重新整理 → 15 秒內出現 `fTeach closed`。
6. 開 Teach → 切到別的瀏覽器分頁 2 分鐘 → **不**出現；切回來 → 仍不出現；EXIT → 出現。
7. 開 Teach → 關掉整個瀏覽器 → 不出現；重開 HMI → 出現。
8. 主選單直接開 Motor Test → EXIT → `fMotorTest closed`。
9. Teach → Motor Test → EXIT（回到 Teach）→ 出現 `fMotorTest closed`、**不**出現 `fTeach closed`。
10. （實機、選做）Teach 開著時從實體面板按 START：主控台應看到 Teach 關掉時 `while SystemStart=1 -> not cleared`。

---

## 5. 要 Steven 決定的細節（R 題）

### S122-R1　「關掉」用哪個訊號判斷
- **背景**：C++ 今天只有 background.html 推的視窗總表（每 5 秒心跳），WebBridge 沒有「連線斷了」的對外介面（WebWindowRegistry.h:40-48）。所以重新整理要等舊連線 15 秒過期才算；整個瀏覽器關掉要等下一個 HMI 連上才算。
- **選項**：
  - **A** 只用現成總表（本方案）。不改網頁、不改 WebBridge，只動 St01 的檔。
  - **B** A＋請 Jimmy 在 WebBridgeServer 開一個「哪些連線關了」的查詢、總表收到後把那條連線當成立刻過期。重新整理、關瀏覽器幾乎立刻算（瀏覽器關分頁會送 WS close）；要改 Jimmy 的兩個共用檔（WebBridgeServer、WebWindowRegistry）。
  - **C** A＋background.html 在 `pagehide` 送最後一個總表（全部視窗 closed）。重新整理立刻算；瀏覽器當掉仍要等；改網頁共用頁（Steven 的總表段＋EastSun 的心跳段），而且要先確認不違反總表契約 §6「斷線不可當成全部關閉」（這是頁面自己說的事實，不是 C++ 猜的，但要 Steven 點頭）。
- **建議**：**A**。空窗期間 MainProc 仍把 Teach 當開著而暫停，網頁上也按不到 START；B／C 只縮短反應時間。之後如果要順便讓「重新整理後 MainProc 更快恢復」，再做 B。
- **例子**：Teach 開著按 F5——A：約 15 秒後主控台出現 `fTeach closed`；B：約 1 秒；C：約 1 秒（`pagehide` 送得出去時）。

### S122-R2　打開 Teach／Motor Test 那一下也要清嗎
- **背景**：golden 開 Teach 就清（FormShow uteach.cpp:1610），關掉又清（main.cpp:28847）。移植樹現在「開」是靠 Teach iframe 在**開站時**的 editlist.get 清（FileRW\Teach.cpp:259），不是在視窗打開時，而且看權杖。
- **選項**：**A** 開、關兩個時間點都清（照 golden）／**B** 只在關的時候清（照第 18 題字面）。
- **建議**：**A**。多的只是「照 golden 在對的時間點清」；而且有了 A，J1 那一行可以放心加 SystemStart 條件或拿掉。
- **例子**：開 Teach、按存檔——A：跟 golden 一樣先問 "Sure to Save?"；B：如果 `fAllMotorHome` 當時是 true 就不問、直接存。開了看一眼就關，A、B 結果一樣（關的時候清）。

### S122-R3　運轉中（SystemStart）關掉 Teach／Motor Test 怎麼辦
- **背景**：golden 到不了這個狀態（運轉中進不了 Teach，main.cpp:28829）。網頁做得到：Teach 開著時從實體面板或另一個分頁按 START。運轉中 Teach／Motor Test 的移動命令全部被擋（WebMotorAccess.cpp:4172），所以運轉期間不可能用這兩頁動過軸；運轉前動過的，當時就已經清了、START 先回原點過了。反過來，運轉中清 `fAllMotorHome` 會讓 `DoAllProcess` 每一拍 return（csystem.cpp:1629），機台停在半路、沒有警報。
- **選項**：**A** 不清，主控台記一行／**B** 記下來，等下一次停機（SystemStart 變 false）再清。
- **建議**：**A**。
- **例子**：Teach 開著、面板按 START（MainProc 因 Teach 開著暫停）→ 按 EXIT → A：MainProc 恢復、繼續生產，主控台一行 `not cleared`；B：一樣繼續生產，但下一次停機後再按 START 會先回原點。

### S122-R4　操作員看得到什麼
- **背景**：golden 被標成要回原點之後，主畫面沒有字；看得到的只有：CleanOut／TrayEnd 反灰、One Cycle 按了沒反應、再進 Teach 跳 WAR16100 "Motor not home alarm !"、按 START 先回原點再接著跑。移植樹這幾個（ProcessKeyFlush、One Cycle、WAR16100）都還沒翻，網頁也沒顯示 `guard.allMotorHome`。
- **選項**：**A** 照 golden，不加新畫面：按 START 就先回原點、回完接著跑／**B** A＋主畫面顯示「需要重新回原點」（用現成 tag `guard.allMotorHome`，只改頁面）／**C** A＋照 golden 在打開 Teach 時跳 WAR16100（要動 Jimmy 的警報框那一套，而且警報框會卡住主迴圈，要另外設計）。
- **建議**：**A**（這次）；B 可以之後當網頁小項目做；C 等 Jimmy 補 `sbTeachingClick` 其他部分時一起做（§6 J4）。
- **例子**：A：操作員 Teach 完按 EXIT、按 START → 機台先回原點（約幾十秒）→ 自動開始生產，畫面上沒有另外的提示。B：EXIT 後主畫面多一行「需要重新回原點（按 START 會先回原點）」，回原點做完自動消失。

---

## 6. 要 Jimmy（筆電）配合／知會

| # | 事 | 要 Jimmy 做什麼 |
|---|---|---|
| **J1** | 【移植樹】FileRW\Teach.cpp:259（Jimmy 的行，W5-a `ad7561d4`／W5-b `a3db68f2`）運轉中也清 `fAllMotorHome`（§1.6-1） | 請 Jimmy 點頭或自己套。原文：`    g_formShown = true;  fAllMotorHome = false;                                // AI(W906-W5-b) …`；同一行改成：`    g_formShown = true;  if (SystemStart == false) fAllMotorHome = false;   /* AI(W906-S122) 20260927：運轉中不清（golden main.cpp:28829 運轉中進不了教導頁；同 WebMotorAccess.cpp:4137 MERGE-56bbf785） */  // AI(W906-W5-b) …`（`SystemStart` 在 cmydef.h:221，Teach.cpp 已 include cmydef.h）。若 R2=A，也可以整個拿掉（S122 的 tick 在視窗打開時清，比 iframe 載入時清更接近 golden）——由 Jimmy 選 |
| J2 | 只有 R1=B 時 | WebBridgeServer 開「哪些連線關了」的查詢（例：主執行緒可以安全呼叫的 `DrainClosedConnIds`），WebWindowRegistry 加「這條連線已經不在」的入口（立刻當過期，**不是**當成全部關閉）；wb_serve 每拍 drain 一次 |
| J3 | 知會：S122-1 其實是 golden 行為 | NIGHT_REPORT 第 18 題、TO_STEVEN §4 20260926 22:4x 那列寫的「S122 比 golden 嚴」要更正（golden main.cpp:28847 `//Steven 20110211`、uteach.cpp:2443、uteach.cpp:1610）。由 St01 實作（第 18 題分工）；筆電**不用**另外做，HW.teach.html **不用**加 HT_WIN／formClose（C++ 從總表看，避免兩條路各清各的） |
| J4 | 知會（不在本案） | (1) golden `sbTeachingClick` 另外三件沒翻：WAR16100（main.cpp:28840-28843）、MES2189 事件紀錄（:28845）、關 Teach 後的 IO 還原問答（:28849-28854）；(2) `motor.access` 在回原點進行中（SoftStart）不擋，網頁第二個 HMI 分頁可以在回原點時寸動（golden 的 Teach 是 ShowModal，做不到）；(3) `ProcessKeyFlush`、`BtnOneCycleClick` 的 `fAllMotorHome` 閘沒翻 |

**EastSun（機台端）知會**：Motor Test 頁的 formClose 那條路（HW.MotorTest.html:1007-1082 → WebMotorAccess.cpp:4137）照留，不衝突；上機驗證照 §4.2。

---

## 7. 風險與沒做的

- **這一趟沒改任何程式**；§3 的步驟都還沒做，也沒 build、沒跑 ctest、沒跑 wb_serve。
- J1 的風險（重新整理可能在運轉中讓生產停住）**只由程式碼推得**，沒實測；會不會發生看開站時哪個 iframe 先拿到權杖。這是 Jimmy 的行，本方案不動它。
- 空窗（§2.3 例 3～5）：重新整理最多 15 秒、瀏覽器整個關掉要等下一個 HMI 連上。空窗內 MainProc 仍暫停、網頁按不到 START；**唯一的漏洞**是實體面板 START，而且要同時滿足「Teach 開著時有人另外做過整機回原點、之後沒再碰任何 Teach 按鈕」才會用舊原點生產。要補就選 R1=B。
- 誤判（例 8，兩個 HMI 分頁）只會讓下一次 START 多回一次原點。
- 0.5 秒內關了又開（例 9）可能漏掉那一下；下一次關掉照樣會清。golden 也不會在這麼短的時間內讓操作員做完一次整機回原點。
- 總表裡沒有 `fTeach`／`fMotorTest`（例：View-rules 把 Teach 視窗設成不建立，background.html:890）時，FShowPolicy 會一直說「在用」，本規則永遠不觸發；但那時 MainProc 也一直暫停，是既有的另一個問題，不是本案造成的。
- 直接在網址列開 `page/HW.teach.html`（不經 background.html）不在總表裡，本規則看不到；正式入口（release.html／debug.html、HT9045_Release.cmd）都經 background.html。
- 本案只處理 `fAllMotorHome`。golden 關 Teach 時 FormClose 的 `btnStop->Click()`（停所有馬達，uteach.cpp:2096）、Motor Test FormClose 寫回 MotorTest.ini，屬各頁既有的工作，不在本案。
- 交件後，「主畫面斷線／重新整理暫停生產」照第 18 題 B **不做**；S161「重新連上要按 Start」沒有觸發點，留著備用。
