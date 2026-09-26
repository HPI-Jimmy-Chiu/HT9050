# Motor Test／Teach 馬達命令對照表（這台 HT9050 機台版）

> AI(W906-MT-TABLE) 20260926。**這份是什麼**：真機（HT9050＋Advantech PCIE-1203）上 `HW.MotorTest.html`／`HW.teach.html`
> 每一顆馬達按鈕，從網頁送出的 action → C++ 哪一支處理 → 落到哪一層 → golden 哪一行 → 哪一段 ctest 驗過 → 還要在機台上驗什麼。
> 全部從程式碼讀出來，每一列都附程式位置。
>
> * 描述的版本：C++ 樹 `integ/ioweb-8484bdb4` **HEAD `7961939`**（MT-FIX1a）；網頁樹 **`a5454b9`**（MT-FIX1 web）。
>   涵蓋的 commit：C++ `4b21dbe` `9fd97ff` `2c1afab` `eef1a78` `b90b0ca` `0d253a0` `5a0f02d` `5dc6706` `8929d13` `7961939`；
>   web `aaf0694` `d288f1f` `8336746` `ef20fb8` `b4c613e` `a5454b9`。
> * 日期：2026-09-26。
> * **不取代 `docs/W4_PROGRESS.md`**。那份是筆電的（38 個命令、W4-b2 當時的狀態），它的 §2 對這台已經過時；
>   本檔只描述這台機台樹今天的程式。兩份衝突時以程式碼為準，並回頭改筆電那份（由筆電改，這裡不動它）。
> * 本檔沒有執行任何會開卡的東西；唯一執行的是純邏輯的假後端測試（§7）。

---

## §0 怎麼讀

### 縮寫

| 縮寫 | 檔案 |
|---|---|
| `WMA` | `WebMotorAccess.cpp`（解析＋分派，純邏輯） |
| `WMAL` | `WebMotorAccessLive.cpp`（真實後端，只連進 wb_serve） |
| `uMT` | golden `HT9011UC_Code_V3.33.906.0_20260618\uMotorTest.cpp`（Big5，唯讀） |
| `uT` | golden `...\uteach.cpp`（Big5，唯讀） |
| `cs` | `csystem.cpp`（引擎） |
| `T[n]` | `tests/test_web_motor_access.cpp` 的第 [n] 段（ctest 名稱 **`WebMotorAccess`**，`tests/CMakeLists.txt:4048`） |
| `E3b-X` | `tests/test_mt_e3b_engine.cpp` 的 PART X（ctest **`MT_E3b_Engine`**，`tests/CMakeLists.txt:4076`） |
| `MT.html` | `web/page/HW.MotorTest.html` |
| `teach.html` | `web/page/HW.teach.html` |
| `ma.js` | `web/page/motor-access.js` |

⚠ `T[...]` 裡 **[9] 印了兩次**：`:1069` 那段是「ack 形狀」，`:1366` 那段是「MT-E1 歸零分支＋reloadMotorData」。本檔把後者寫成 **T[9b]**。

### 「層」欄

| 層 | 意思 | 程式位置 |
|---|---|---|
| **1203** | EastSun 的 `TPci1203Control`，經 `Pci1203Control()->Execute()` 打在 **1203 監看器開好的軸槽**上（不另開軸） | `WMAL:201-215` |
| **MOT** | golden 馬達物件 `MOT[i]`（非 1203 軸唯一的路；這台實際上走不到，見 §1） | `WMAL:222-257`、`:369-396` |
| **SW** | golden `SW[]` 輸出（繼電器、煞車），經 1203 IO 路由送到卡上，來源標成 `"web"` | `WMAL:519-564` |
| **ui** | golden 只動畫面，C++ 回 done、不碰任何東西（ack 的 `layer:"ui"`） | `WMA:3098-3106` |
| **file** | 寫 `D:\LightScale\...` 檔案 | `WMA:2651-2734` |
| **拒絕** | C++ 回 ok=false＋理由，頁面顯示成錯誤；**後端一次都不呼叫** | `WMA:3089-3097`、T[3] |

### 一個按鈕怎麼走到卡上

```
頁面按鈕 → ma.js send()（互斥；motion 期間只留 btnStop，ma.js:213-283）
  → ht9045_recipe_client.js motorAccess：control.acquire 與 motor.access 背靠背送（:433-443）
    或 motorStop：不取權杖（:458-461）
  → tools/wb_serve.cpp:4994（motor.access／motor.stop 臂）
  → W906_MotorAccessWire（WMAL:810-831；motor.stop 鎖死只收 action=="stop"，:819-823）
  → MotorAccessParse（WMA:3017）→ MotorAccessDispatch（WMA:3080-3147）
```

* `motor.stop` **免操作權杖**（`WebBridge/WebBridgeServer.cpp:1446`），走輸出優先佇列（`tools/wb_serve.cpp:5469`、`:5605-5610`），
  **告警框開著時也照樣處理**（`tools/wb_serve.cpp:666`）；其他 `motor.access` 在告警框期間回 `modal-pending`（同一行）。
* 跨拍的工作（HOME、LoopMove、Motor Power 的 1 秒、Light Scale）由 500 ms 拍子 `W906_MotorAccessTick`（`tools/wb_serve.cpp:4103`）
  與每次 1203 Poll 之後的 `W906_MotorAccessPollTick`（`tools/wb_serve.cpp:4160`）推進（`WMA:2744-2795`）。
* 1203 的每一個停止都是 golden `DecStop` = `Acm_AxStopDec` ＋ `Acm_AxSetExtDrive(ax,0)`（`WMA:210-225`）。

### 運動命令共同的前提（1203 軸）

`MotionPreludeFor`（`WMA:507-534`）：有選馬達 → 馬達表查得到 → `MOT[i].Motor` 存在 → 可選（出貨組態＝Mot_Table Enable，`WMAL:318-322`）
→ Mot_Table Enable=1 → 1203 控制層在 → 監看器對得到「站＋站內軸」（對不到就拒絕並記進 1203 稽核）→ 需要位置的命令拒絕 Direction=1。
再加：上一個命令之後要等一次新的 Poll（`WMA:152-161`，否則回「監看器還沒更新…稍候再按」）。

---

## §1 這台的軸（2026-09-26 唯讀量測，要用就重量）

* 讀的是 `D:\HT9045\system\Mot_Table.csv`（5,343 B，mtime 2026-09-25 16:30:17，MD5 `BC2E4C69…30C6`）。這台 F5 的出貨組態 wb_serve
  讀的就是它（IOWEB-P16）。48 列：PCI1203 19、MN200 16、SMC 13。
* **Enable=1 的 19 列全部是 PCI1203**，而且全部 **Direction=0、GearRatio=1、SoftLimit = ±999999**。
  ⇒ 這台的運動全部走 **1203 層**；MOT 層只剩 Enable=0 的列，出貨組態選不到（`WMAL:318-322`，`WMA:514`）。
  ⇒ `kDirPending`（Direction=1 拒絕，`WMA:495-496`）今天不會觸發；卡片單位＝使用者單位。
  ⇒ **軟體極限實質上不存在**（±999999）。
* ⚠ **這個檔跟 repo 裡的 `machines/HT9050/Mot_Table.csv` 不一樣**（MD5 `79CCD044…CE7F`）：M00 M01 M14 M18 M19 M35 M36 M38 M39 M40 M108
  這 11 列在 repo 副本是 Direction=1，機台檔是 0。golden 的 EtherCAT jog 方向是 `Acm_AxJog(ax, Direction ? 1 : 0)`（`WMA:750-756`），
  所以這 11 軸的 JogP 實體方向跟 golden 原設定**相反** —— 方向一定要機邊看（§6）。`E3b-G` 讀的是 repo 副本，不是機台檔。
* 建置：F5 用 `build_integ_ship_x86`（`.vscode/launch.json:105`），CMakeCache `W906_NO_SOFT_SIMULTE=ON` ⇒ **SOFT_SIMULTE 沒定義**（真機臂）。
  `MachineType.h:106` `WB_PUMP_1203_CONTROL_LIVE` 開著 ⇒ 1203 命令真的送到卡上（ack 不會出現 `NOT issued (dry)`）。

| M | Alias | BoardID/Port | HomeDirection → 歸零方法（DS402 時） | 特殊處理 |
|---|---|---|---|---|
| M00 | MInArmX | 0/0 | 0 → `Acm_AxHome 128, -1` | Light Scale 軸（rgAxis 1，也是沒點選時的預設） |
| M01 | MInArmY | 1/0 | 1 → `124, +1` | Light Scale 軸（rgAxis 2） |
| M03 | MInArmZA | 3/0 | 0 → 128 | arm Z：歸零後去 ZSafePos；煞車群組 InOutArmZ |
| M11 | MInShutte1 | 30/1 | 1 → 124 | MInShuttle1：`SHUTTLE_FLOODGATE==1` 時移動前先開閘（`WMAL:404-415`） |
| M14 | MTestZ1 | 14/0 | 1 → 124 | R2：當一般 1203 軸（`WMAL:300-307`）；**JogHigh 900000、Acc 9000000**，第一次動之前先看；煞車群組 Index |
| M17 | MOutShuttle1 | 10/0 | 1 → 124 | |
| M18 | MOutShuttle2 | 10/1 | 0 → 128 | |
| M19 | MOutArmX | 0/1 | 1 → 124 | Light Scale 軸（rgAxis 3） |
| M20 | MOutArmY | 1/1 | 0 → 128 | Light Scale 軸（rgAxis 4） |
| M22 | MOutArmZA | 3/1 | 0 → 128 | arm Z；煞車群組 InOutArmZ；HomeHigh＝HomeLow＝100 |
| M30 | MTrayX | 30/0 | 1 → 124 | |
| M35 | MLoaderZ | 35/0 | 0 → 128 | Z 堆疊速度內插；煞車群組 Cassette；**HomeLow 300 > HomeHigh 100** |
| M36 | MEmptyZ | 36/0 | 0 → 128 | Z 堆疊；**不在任何煞車群組**；HomeLow 300 > HomeHigh 100 |
| M38 | MAuto1Z | 38/0 | 0 → 128 | Z 堆疊；Cassette；HomeLow 300 > HomeHigh 100 |
| M39 | MAuto2Z | 39/0 | 0 → 128 | Z 堆疊；Cassette；HomeLow 300 > HomeHigh 100 |
| M40 | MAuto3Z | 40/0 | 0 → 128 | Z 堆疊；**不在任何煞車群組**；HomeLow 300 > HomeHigh 100 |
| M41 | MInRotate | 41/0 | 1 → 124 | |
| M42 | MOutRotate | 41/1 | 1 → 124 | |
| M108 | MCCDY | 108/0 | 0 → 128 | |

依據：方法 `WMA:1211-1215`；arm Z 名單 `WMAL:308-316`；Z 堆疊 `WMAL:296-297`＋`WMA:3001`；閘門 `WMAL:323`；煞車群組 `WMAL:973-990`。
`MLightScale`＝MOT[79]（`cmydef.cpp:2417`）在 Mot_Table **沒有列** ⇒ 光學尺讀值 0（`WMAL:613-633`）。

---

## §2 48 個命令（catalog = `web/JSON/motor-access.json`，48 個命令、37 個 action）

編號照 catalog 的順序（**與 W4_PROGRESS.md §2 的編號不同**：那份的 uteach 是 27–38，這裡是 37–48）。
「頁面」欄是網頁送出的位置；`getParams` 會自動替每一筆補上 `speed=edtSpeed`、`currentPos`、`softLimitP/N`（`MT.html:1643-1651`）。

### 2-1 運動（uMotorTest）

| # | 按鈕 → action | 頁面 | C++ 入口 | 層 | golden | ctest | 機邊待驗 |
|---|---|---|---|---|---|---|---|
| 1 | `sbMotorTest_JogP` → `jogP`（按住） | `MT.html:1666-1680` pointer 事件＋`setPointerCapture` | `WMA:715-774` `DoJog` | 1203：只設 jog 家族 `CFG_AxJog*` 4 個（值＝捲軸上次寫的，`WMA:732-735`）→ `Acm_AxSetExtDrive(ax,1)` → `Acm_AxJog(+1)` | `uMT:862` MouseDown（放開 `:900`） | T[7] `:792-834`、T[10] W4B-1/4/5 `:1137`,`:1173-1193`、T[11] `:1229-1239`,`:1268`,`:1327`、T[12] `:1521-1567`、T[13] R2 `:1948-1953`、T[14] `:2235-2260` | ①方向（見 §1 Direction 的差異）②`ExtDrive(1)`＋`Acm_AxJog` 在這張卡真的會動（main 當初量到 0 位移是沒有 ExtDrive，`WMA:757-758`）③速度＝`PJogHighSpeed×捲軸%`（M14 是 900000）④放開、滑出按鈕、切走視窗都會停 |
| 2 | `sbMotorTest_JogN` → `jogN`（按住） | 同上 | 同上 | 同上，`Acm_AxJog(-1)` | `uMT:812` | 同上 | 同上 |
| — | jog 放開 → `stop`（button 是那顆 jog 鍵） | `ma.js:290-301`；每條放開路徑都送（`ma.js:246-261`），ack 比放開晚回也會補送（`ma.js:273`） | `WMA:278-322` `DoJogRelease`（`IsJogButton` `:272`） | 1203：**只停那一軸**（StopDec＋ExtDrive 0）；不做 StopAllMotor | `uMT:900-910` | T[4b] `:624-659`、T[7] `:802` | 放開後確實減速停；ExtDrive 回 0（下一次 jog 會重送 1） |
| 3 | `sbMotorTest_MoveP` → `moveRelative` | `MT.html:1683-1686`（`interval`、`speed`＝捲軸位置） | `WMA:777-795` → `Move1203` `:654-696` | 1203：PTP 4 個速度（有 speed 時）→ `Acm_AxMoveAbs(目前 cmdPos＋interval)` | `uMT:1252` | T[7] `:836-878`、T[10] `:1152-1159`,`:1190`、T[11] `:1270` | ①目標以**命令位置 cmdPos** 算，不是編碼器 ②±999999 等於沒有軟體極限 ③M11 的閘門（Gerneral.ini `SHUTTLE_FLOODGATE`） |
| 4 | `sbMotorTest_MoveN` → `moveRelative` | `MT.html:1687-1690`（送負的 interval，C++ 依按鈕名取絕對值，`WMA:790-792`） | 同上 | 同上，目前位置 − interval | `uMT:1224` | 同上 | 同上 |
| 5 | `btnGo` → `moveAbsolute` | `MT.html:1692`（目標＝`edtHomeOffset`；`speed`＝edtSpeed 由 getParams 帶） | `WMA:798-810` → `Move1203` | 1203：IsMotorCanRun（EMG）→ HomeFlag≠0 → PTP 速度 → `Acm_AxMoveAbs` | `uMT:1605-1619` | T[7] `:880-891`、T[10] `:1165`、T[11] `:1275` | ①HomeFlag=2（歸零失敗）**也會放行**（golden 同樣只擋 0，`uMT:1613`）②DS402 歸零後 `edtHomeOffset`（LastHomePos）通常≈0，所以 Go 大多是回 0 ③HOME／Loop 進行中**不擋**（golden 沒有那行，`WMA:241-243`） |
| 6 | `btnGoSoftP` → `moveSoftLimitP` | `MT.html:1698` | `WMA:814-824` | 目標取 C++ 的 PSoftLimitP；golden `Tar>=PSoftLimitP` ⇒ **一定被拒**（`WMA:667-671`） | `uMT:1097` | T[7] `:893-898`、T[11] `:1272` | 這台按了不會動（照 golden），只要確認回的是拒絕 |
| 7 | `btnGoSoftN` → `moveSoftLimitN` | `MT.html:1699` | 同上 | 同上，`<=` 拒絕 | `uMT:1105` | T[7] `:900-902` | 同上 |
| 8 | `btnHome` → `home`（帶 `start`＝按下後的狀態） | `MT.html:1700-1703`；換選軸時頁面會先送 `start:false`（`MT.html:492-493`） | `WMA:1228-1284`；開始 `StartHome1203` `:1168-1226`；完成偵測 `TickHomes` `:2075-2169` | 1203：一按就 HomeFlag=0 → 門開拒絕 → 看驅動器：DS402 → 先把 **PTP** 設成歸零速度（VelLow=HomeLow、VelHigh=HomeHigh、Acc/Dec=資料庫值）→ `Acm_AxHome(124/128, ±1)`；非 DS402 → 卡片式 MODE12＋歸零後 SetCmd/SetAct(0)（`:1137-1167`）；判斷不出 → 拒絕。完成：HOMING→READY ⇒ HomeFlag=1；ERROR_STOP ⇒ 2；5 秒沒進 HOMING ⇒ 2；180 秒逾時 ⇒ 停＋2（`:1011-1018`）；arm Z 再去 ZSafePos。磁性尺軸與非 1203 拒絕 | `uMT:1113-1175`＋ uhome.cpp ProcessSingleMotorHome | T[8] `:933-996`、T[11] `:1305-1353`、T[9b] `:1366-1418`、T[12] LastHomePos `:1665-1690`、T[13] `:1966` | ①每軸方向（§1 表）②歸零速度：DS402 的 6099h 是 `Acm_AxHome` 拿**當下 PTP** 去填（`WMA:1195-1198`）——量 6099h:1/:2、609Ah ③**M35/M36/M38/M39/M40 會送 VelLow 300 > VelHigh 100**，卡片怎麼處理 ④HOMING 狀態真的出現、READY 後 HomeFlag=1 ⑤M03/M22 去 ZSafePos 的值 ⑥`Pci1203DriveKind` 對每一站都答得出 1（`WMAL:174-193`） |
| 9 | `btnLoopMove` → `loopMove`（帶 `start`、`mode`、`pos1/pos2`、`waitTime`、`confirmNotHomed`） | `MT.html:1704-1740`（All 模式另帶 `motors` 與 `pos1.<Alias>`／`pos2.<Alias>`） | 單軸 `WMA:1435-1503`＋`TickLoop` `:2209-2260`；All `:1360-1432`＋`TickLoopAll` `:2177-2207`＋`GoldenMove1203` `:1303-1344` | 1203：單軸 pos1→等→pos2→計數；到位＝READY 且 cmdPos 在 ±2 內；每段 120 秒逾時；半途被擋就停。All：每拍對每一個勾選軸下 golden MotorMove，全部回非 0 才換段；**被拒的（門 −1、極限 −2/−3）也算到位**；沒有逾時 | `uMT:1300-1336`＋ DoLoopMove `uMT:393` | T[8] `:998-1043`、T[11] `:1244-1301`、T[12] 計時 `:1701-1778`、T[13] All `:1985-2037`、T[14] `:2209-2229` | ①來回位置與等待 ②**waitTime 30/60（3/6 秒）會被 VerifyMotorAction 的「2 秒沒動」停掉**（golden 同，`cs:29994-29996`）③到位看 cmdPos 不看編碼器 ④All 模式速度：每軸用自己上一次的 %（沒設過 = 1%，`WMA:1396-1403`） |

### 2-2 停止、伺服、電源（uMotorTest）

| # | 按鈕 → action | 頁面 | C++ 入口 | 層 | golden | ctest | 機邊待驗 |
|---|---|---|---|---|---|---|---|
| 10 | `btnStop` → `stop`（走 `motor.stop`，免權杖） | `MT.html:1741`（先 allBtnUp） | `WMA:327-388` `DoStop` | MOT：golden `StopAllMotor(true)`＋`bSingleHome=false`（`WMAL:222-233`）；1203：**每一個監看器開成功的軸**都送 StopDec＋ExtDrive 0；取消 HOME／Loop 工作；有軸拒絕停止 ⇒ `partial:true`（頁面顯示紅色） | `uMT:1647-1659` | T[4] `:590-620`、T[7] `:920-929`、T[10] `:1144`,`:1168`、T[11] `:1236` | ①監看器開成功的軸（應涵蓋 Enable=1 的 19 軸）都減速停，ack 的 `stopSent` 等於開成功的軸數、`stopRefused`=0 ②別的分頁拿著權杖時照樣停得了 ③告警框開著時照樣停得了 ④**Light Scale 的掃描不會因此停**（§5-7） |
| 11 | `btnServoOff` → `servoToggle` | `MT.html:1742-1746`（知道 servoOn 才帶 `servoOn:!現值`） | `WMA:407-487` `DoServo` | 1203：`Acm_AxSetSvOn(ax, 0/1)`。目標：頁面明講 > 實際 SVON 取反 > golden 函式內 static；先取消 HOME／Loop | `uMT:1661-1671` | T[5] `:663-725`、T[10] `:1141`、T[11] `:1235`,`:1301` | ①SVON 位元（motionIO `0x4000`，`WMAL:52`）真的跟著驅動器 ②Z 軸 Servo ON 之後，該煞車群組**全部軸**都 ON 時，下一個 DoSystem 才放煞車（§5-4） |
| 12 | `btnMotorPower` → `motorPowerToggle`（帶 `confirmOff`、`expectRelayOn`） | `MT.html:958-993`（繼電器開著先問「確定要關掉馬達電源？」） | `WMA:2323-2365` `DoMotorPower`；跨拍 `:2283-2299` | SW：先把 `SW[SwMotorRelay].OutValue` 照卡片回讀同步（`WMAL:521-535`）。**關→開**：DoMotorPowerOn（繼電器 ON＋Index/Magazine/Cassette 煞車**鎖住**）→ 1 秒（跨拍，不凍結主執行緒）→ `SW[SwServerON].On()`。**開→關**：先 Stop1203 全軸，再 `fHome->GaliMotorServoOff`（StopAllMotor、繼電器與 ServerON 關、bMotorPowerState=false、SystemStart=false、fAllMotorHome=false、全部煞車鎖住；HomeFlag 保留）。按下時以為的狀態跟現值不同 ⇒ 不切（`:2335-2341`）；1 秒內再按 ⇒ 拒絕 | `uMT:1621-1645` | T[8] `:1062`、T[13] `:1829-1859`、T[14] `:2171-2204`、E3b-B、E3b-F | ①繼電器（`SwMotorRelay`，ring1 st1 ch29，`cs:16052`）與 ServerON 實際切換 ②1203 DO 回讀與 OutValue 同步 ③**關電再開電後 bMotorPowerState 仍是 false ⇒ Z 煞車一直鎖住，直到按機台 HOME**（§5-4，由程式推得，未在機台量過） |

### 2-3 參數與畫面（uMotorTest）

| # | 按鈕 → action | 頁面 | C++ 入口 | 層 | golden | ctest | 機邊待驗 |
|---|---|---|---|---|---|---|---|
| 13 | `btnSetPosP` → `setPos1` | `MT.html:1752-1762`（頁面自己把 edtCommandPos 抄進 edPos1） | `WMA:3098-3106` | ui | `uMT:1079` | T[2]、T[3] `:584-586` | — |
| 14 | `btnSetPosN` → `setPos2` | 同上（edPos2） | 同上 | ui | `uMT:1088` | 同上 | — |
| 15 | `btnHighSpeed` → `setJogHighSpeed` | `MT.html:1776-1781` | `WMA:828-873` `DoSetParam` | MOT 記憶體：`PJogHighSpeed = SetSpeed(params.speed%) 的 s`（C++ 用 getParams 帶的 edtSpeed，不用頁面的 value，`WMA:851-854`） | `uMT:1403` | T[7] `:904-918`、T[10] `:1143`,`:1199` | 只改記憶體；要存檔走 Motor Database 分頁（§3） |
| 16 | `btnLowSpeed` → `setJogLowSpeed` | `MT.html:1782` | 同上 | 同上 → PJogLowSpeed | `uMT:1411` | 同上 | 同上 |
| 17 | `btnHomeHigh` → `setHomeHighSpeed` | `MT.html:1783` | 同上 | 同上 → PHomeHighSpeed | `uMT:1419` | 同上 | 同上 |
| 18 | `btnHomeLow` → `setHomeLowSpeed` | `MT.html:1784` | 同上 | 同上 → PHomeLowSpeed | `uMT:1427` | 同上 | 同上 |
| 19 | `btnSoftPPos` → `setSoftLimitP` | `MT.html:1785` | 同上 | MOT 記憶體：PSoftLimitP = 1203 監看器 cmdPos 換算的目前位置 | `uMT:1435` | T[7] `:911-914` | 設了之後軟體極限才真的存在（這台原本 ±999999） |
| 20 | `btnSoftNPos` → `setSoftLimitN` | `MT.html:1786` | 同上 | 同上 → PSoftLimitN | `uMT:1443` | 同上 | 同上 |
| 21 | `btnRange` → `refreshParameter` | `MT.html:1839` | `WMA:3098-3106` | ui | `uMT:1451` | T[2] | — |
| 22 | `btnRate` → `refreshParameter` | `MT.html:1840` | 同上 | ui | `uMT:1458` | T[2] | — |
| 23 | `btnSetRange` → `setRangeAndInit` | `MT.html:1849-1855` | `WMA:1658-1743`；InitMotor `:1566-1644` | 1203（EastSun R4）：SetRange（記憶體）→ **完整 golden InitMotor**：ResetError（最多 3 輪）→ 設定表 `kCmdAxSetInitCfg`（PPU、ElReact、AlmEnable、AlmReact、OrgLogic、Jerk、Inp/Alm 邏輯、脈波模式）→ MaxVel/MaxAcc/MaxDec → ResetError →（有警報再 ResetError＋100 ms）→ **SvOn 1** → **SetCommand(0)、SetPosition(0)** → HomeFlag=0。EMG 任一顆 IsOff ⇒ 什麼都不寫 | `uMT:1374-1382` | T[8] `:1053-1059`、T[11] `:1359`、T[13] `:1894-1940`；設定表本身 `Pci1203Pure` | ①**會激磁（SvOn）並把這一軸座標歸 0**，之後要重新歸零 ②每一步的回傳（ack 的 `initMotor.steps`）③M14 的 CFG_AxMaxVel＝900000 |
| 24 | `btnSetRate` → `setRateAndInit` | `MT.html:1856`（按鈕照 golden 藏著，dfm Visible=False） | 同上 | 同上，前面是 golden SetRate：dAcc 記憶體＋`PAR_AxAcc/PAR_AxDec`＝夾限後的 Rate（`WMA:2840-2857`），再 InitMotor（MaxAcc＝新的 dAcc） | `uMT:1364-1372` | T[13] `:1789-1799`,`:1930-1940` | 同上；頁面看不到這顆 |
| 25 | `btnReloadMotorData` → `reloadMotorData` | `MT.html:1862`（頁面同時重讀資料庫，`:1555`） | `WMA:1757-1801`；表值 `WMAL:675-794` | MOT：**所有馬達 HomeFlag=0** → 重讀 Mot_Table.csv 就地套回（唯讀；身分欄變了的列不套、要重開 wb_serve）→ 1203：**每一個開成功的軸** `Acm_AxSetCmdPosition(0)`＋`Acm_AxSetActualPosition(0)`。golden 沒有任何保護，這裡也沒有 | `uMT:1695-1711` | T[9b] `:1420-1442` | ①**全部軸的座標被歸 0，每一軸都要重新歸零** ②DS402 絕對值編碼器上 SetActualPosition(0) 對驅動器端座標的影響 |
| 26 | `btResetMNet` → `resetMNet`（帶 `confirm`） | `MT.html:1864-1869` | `WMA:1506-1518` | MOT：確認＋SystemStart==false 才 `ResetMNet(0,"MNet斷電","Power Off",false)`（`WMAL:440-444`） | `uMT:1718` | T[8] `:1045-1051` | 這台沒有 MN200 卡（Enable=1 的列全是 1203），確認呼叫無害 |
| 27 | `labName` → `selectMotor` | `MT.html:508-512`（排隊送，`sendLater`） | `WMA:1839-1874` | MOT＋1203：先擋 Enable=0 → 取消 HOME／Loop（**不送停止**）→ 記住選取 → jog 家族值定在 1% → golden `SetSpeed(1)`：1203 軸把 **PTP 4 個速度設成 1%** 寫到卡上 | `uMT:734-760`／`:762` | T[12] `:1474-1508` | 每次點選都會寫該軸 PTP 速度（不會動） |
| 28 | `strngrdMotor` → `setParamCell`（`row` 1–10、`value`） | `MT.html:843` | `WMA:1881-1916` | MOT 記憶體（第 8/9 列 atof，其他 atoi）→ Loop 抬起（不停）→ **該軸 HomeFlag=0** | `uMT:1176-1222` | T[12] `:1582-1613` | 只改記憶體；改完該軸要重歸零才能 Go |
| 29 | `BitBtn1` → `copyFrom`（`source` 0–24） | `MT.html:1792-1799` | `WMA:1923-1956` | MOT 記憶體：抄 JogHigh/JogLow/HomeHigh/HomeLow/SoftP/SoftN；HomeFlag 不動 | `uMT:1384-1401` | T[12] `:1616-1634` | 抄完 jog 速度不會變快：jog 用的是捲軸當時寫到卡上的值（`WMA:187-193`） |
| 30 | `scrlbrMotorSpeed` → `setSpeed`（`jog:true`、`pct` 1–100） | `MT.html:1807-1825`（拖動約每 100 ms 送一次） | `WMA:1967-2061` | 1203：PTP＋jog 家族共 8 個值寫卡；HOME 進行中不做（golden `bSingleHome`）；All 模式 Loop 進行中連勾選軸一起設。寫卡失敗時 jog 取新舊兩者**較慢**的 | `uMT:791-810` | T[12] `:1510-1576`、T[13] `:2027`、T[14] `:2235-2260` | CFG_AxJog*／PAR_AxVel* 讀回值 |
| 31 | `edtSpeed` → `setSpeed`（`jog:false`） | `MT.html:1830-1837` | 同上 | 1203：只寫 PTP 4 個值 | `uMT:1587-1603` | T[12] `:1533-1541`,`:1576` | 同上 |
| 32 | `FormShow` → `formShow` | `MT.html:1007-1019`（當成「狀態」同步，被拒 1.5–10 秒重送） | `WMA:2374-2401` | MOT＋SW：C++ 記 fShow=true、bSingleHome=false、HOME／Loop 抬起、ActiveIndex=-1；Motor Power 同步：繼電器**開**⇒ 再跑一次 DoMotorPowerOn（繼電器已開，所以只等 1 秒、不動煞車，`cs:14392-14405`）＋ServerON；繼電器**關**⇒ Stop1203 全軸＋`GaliMotorServoOff("TfMotorTest::FormShow")`（SystemStart=false、fAllMotorHome=false、bMotorPowerState=false、煞車鎖住） | `uMT:985-1077` | T[13] `:1862-1883` | ①**電源關著時打開 Motor Test 頁 = 清掉 fAllMotorHome，並觸發 §5-4 的煞車問題**（golden 同樣會跑 GaliMotorServoOff）②電源開著時打開頁面，ServerON 會再 On 一次 |
| 33 | `FormClose` → `formClose` | `MT.html:1073-1080`（同時頁面寫 MotorTest.ini，§3） | `WMA:2410-2421` | MOT：fShow=false、`PauseUT150Polling=false`；進行中的 Loop 結束（不送停止，已下的那一段走完）；HOME 與 Light Scale 繼續 | `uMT:1347-1362` | T[13] `:1877-1882`,`:2147` | 關頁後 Light Scale 仍在跑（§5-7） |

### 2-4 Light Scale（uMotorTest）

| # | 按鈕 → action | 頁面 | C++ 入口 | 層 | golden | ctest | 機邊待驗 |
|---|---|---|---|---|---|---|---|
| 34 | `BitBtn2` → `lightScale`（`axisItem`、`moveType`、`pitch`、`delayMs`） | `MT.html:1875` | `WMA:2623-2649`；狀態機 `:2520-2602`；reset `:2605-2620` | 1203：每按一次只切換計時器（開／關），開的時候從 Task 0 開始：先單軸歸零（同 HOME 的 `StartHome1203`）→ 第一點 `PSoftLimitP−100`（去）或 `PSoftLimitN+100`（返）→ 到位 → 延遲 → 讀光學尺×(−1) → 記一行 → 下一點（± pitch）→ 到另一端 −100 結束 | `uMT:2096-2100`、LightScale `:1795-1978`、Timer2 `:2031-2037`、rgAxisClick `:2121-2133` | T[13] `:2041-2166` | 🛑 **這台第一點是 ±999899**（§5-7）。光學尺讀值 0（沒有 M79 列）。**在確認機構行程前不要按** |
| 35 | `BitBtn3` → `lightScaleSave` | `MT.html:1877` | `WMA:2677-2702` | file：`D:\LightScale\Motor{1..8}Positive.csv`（**覆寫、不備份**），清 Memo1 | `uMT:2102-2119`、SaveAsCSV `:1749-1787` | T[13] `:2076-2097` | 寫到機台 D 槽 |
| 36 | `btnSaveLogLightScaleData` → `lightScaleDataSave` | `MT.html:1879` | `WMA:2705-2734` | file：`D:\LightScale\LightScaleData_yyyymmddhhmm\` 8 個 csv；wb_serve 不建 fMotorTest，所以 mmo1..8 是空的（`WMAL:644-650`），8 個檔都是空檔 | `uMT:2051-2094` | T[13] `:2099-2111` | 同上 |

### 2-5 教導頁（uteach）

教導頁用同一個 `ma.js`，`source:'uteach'`（`teach.html:377-381`）。**除了 STOP 與伺服，運動鈕全部拒絕**（`WMA:3109-3111`、`:2736-2739`），教導設定鈕排在 W5。

| # | 按鈕 → action | 頁面 | C++ 入口 | 層 | golden | ctest | 機邊待驗 |
|---|---|---|---|---|---|---|---|
| 37 | `btnJogP` → `jogP` | `teach.html:399-400` | `WMA:3110-3111` | 拒絕（W5：golden uteach 另有 CheckCanMove／IsCanQuickJogMove） | `uT:997`（放開 `:1262`） | T[3] `:573-574` | 回拒絕、不動 |
| 38 | `btnJogN` → `jogN` | `teach.html:401-402` | 同上 | 拒絕 | `uT:1223` | 同上 | 同上 |
| — | 教導頁 jog 放開 → `stop` | `teach.html:400`,`:402`＋`ma.js:290-301` | `WMA:278-322` | 1203：只停那一軸（jog 本來就沒開始，送停止無害） | `uT:1262` | T[4b] `:638-641` | — |
| 39 | `btnMoveP` → `moveRelative` | `teach.html:403-406` | 同上 | 拒絕 | `uT:2104` | 同 #37 | 同上 |
| 40 | `btnMoveN` → `moveRelative` | `teach.html:407-410` | 同上 | 拒絕 | `uT:2205` | 同上 | 同上 |
| 41 | `btnMoveTo` → `moveAbsolute` | `teach.html:411` | 同上 | 拒絕 | `uT:2391` | 同上 | 同上 |
| 42 | `btnHome` → `home` | `teach.html:412` | 同上 | 拒絕 | `uT:2133` | T[8] `:996` | 同上 |
| 43 | `btnStop` → `stop` | `teach.html:413` | `WMA:327-388` | MOT＋1203：StopAllMotor＋`MOT[MTestY1].Gali_Command("ST")`（`WMAL:227-231`；`Tech_Part=0` 缺相依沒做）＋每個開成功的 1203 軸停止；不動 bSingleHome（只有 uMotorTest 才做，`WMA:334`） | `uT:2948` | T[4] `:608-615` | 同 #10；這台沒有 Galil，`Gali_Command` 應無作用 |
| 44 | `btnServo` → `servoToggle` | `teach.html:414`（不帶 servoOn） | `WMA:407-487` | 1203：實際 SVON 取反；讀不到就拒絕、不猜 | `uT:4287` | T[5] `:700-703` | 同 #11 |
| 45 | `btnSetTo` → `setTeachFromCurrent` | `teach.html:415` | `WMA:3092-3097` | 拒絕（queued W5） | `uT:2098` | T[3] `:565-578` | — |
| 46 | `btnSetToOffset` → `setTeachFromOffset` | `teach.html:416` | 同上 | 拒絕（W5） | `uT:2200` | 同上 | — |
| 47 | `SetButton*` → `teachSet` | `teach.html:437` | 同上 | 拒絕（W5） | `uT:3360` 起（例 `SetButton140Click`、`:3517`） | 同上 | — |
| 48 | `GoButton*` → `teachGo` | `teach.html:447` | 同上 | 拒絕（W5） | `uT:3401` 起（例 `GoButton140Click`、`:3559`） | T[3] `:576-577` | — |

---

## §3 頁面送的其他東西（不走 `motor.access`，但跟馬達頁有關）

| 做什麼 | 頁面 | C++ | 影響 |
|---|---|---|---|
| 讀馬達表與現值 | `MT.html:1302-1305`（`/api/struct/motor/config`、`runtime`） | 主執行緒每拍抄一份給 socket 執行緒讀（`WMAL:853-960`） | 唯讀。1203 軸的位置、SVON、ALM、INP、忙碌、扭力都來自監看器樣本 |
| 扭力顯示 | `MT.html:400-425` | 頁面開著且有選軸時，每拍把該軸設成 6077h SDO 焦點（`WMAL:1033-1042`）；單位判準 `EtherCAT/Pci1203Monitor.cpp:4241-4269` | `unitVerified=false` 時頁面註明「unit NOT verified」 |
| 關頁時寫 `MotorTest.ini` | `MT.html:656-690`、`:1079` | `system.file.put tag=motorTest`（`tools/wb_serve.cpp:1007`、`:4365-4458`） | **寫機台檔**。`gAllowSystemWrite` 恆為 true（`tools/wb_serve.cpp:2587`） |
| Motor Database 分頁 Save（`sbUpdate`） | 引擎 sysGrid（`web/page/ht9045_wire_hwmotortest.js:49-57`） | `system.file.put tag=motTable` → `MotTablePath`（`tools/wb_serve.cpp:985`）；增刪列 `system.csv.rows`（`:4470-4478`） | **寫 `D:\HT9045\system\Mot_Table.csv`**（有備份檔，`:4447-4453`）。參數鈕、格子、Copy From 只改記憶體，要保存只能靠這顆 |
| 視窗開關 | background.html `ui.windows.put`（`web/page/ht9045_recipe_client.js:784-820`） | `WebWindowRegistryFShowPolicy`（`WebWindowRegistry.h:153-155`）→ 引擎 `W906_FormFShow`（`WMAL:965-968`、`cs:30005-30008`） | 決定 MainProc 是否暫停、VerifyMotorAction 是否執行（§5-1、§5-2） |
| 續權杖 | `MT.html:1086-1092`（有工作在跑時每 60 秒 `keepAlive`） | `control.acquire` | 權杖閒置被收回 ⇒ jog 被死人開關停下、HOME／Loop 被取消（§5-8） |

⚠ 沒有任何頁面路徑會送 `kCmdAxTorqueLimitSet`（`7961939` 新增，`EtherCAT/Pci1203Control.h:993-1044`）。它只被 `tools/pci1203_safety_watch.cpp` 與 `tests/test_pci1203_pure.cpp` 使用；Motor Test 頁面今天**不能**設扭力上限。

---

## §4 按鈕在真機上會做什麼、哪些會動（給 EastSun）

先講結論：**會讓馬達真的轉的，只有 Motor Test 頁的 JogP/JogN、MoveP/MoveN、Go、HOME、LoopMove、Light Scale（BitBtn2）這 8 顆**。
教導頁的運動鈕全部被拒。GoSoftP/GoSoftN 在這台一定被拒（±999999）。

| 類別 | 按鈕 | 在這台上的結果 |
|---|---|---|
| 🔴 **會動** | JogP/JogN（#1-2） | 按住就 `Acm_AxJog`，放開就停；速度＝捲軸上次的 %（剛選軸時 1%） |
| | MoveP/MoveN（#3-4） | 從目前 cmdPos 走 ±間隔；速度＝捲軸位置 |
| | Go（#5） | 走到 `edtHomeOffset`；要 HomeFlag≠0 且 EMG 沒按 |
| | HOME（#8） | DS402 `Acm_AxHome` 124/128；arm Z（M03/M22）歸零後再走到 ZSafePos |
| | LoopMove（#9） | pos1↔pos2 一直來回，直到再按一次、STOP、或被各種取消條件打斷 |
| | Light Scale 開始（#34） | 🛑 先歸零，再**從 +999899（或 −999899）開始逐點走**；見 §5-7 |
| 🟠 **會切電、激磁或鎖放煞車** | Motor Power（#12） | 切 SwMotorRelay／SwServerON；關電時全部煞車鎖住、SystemStart=false、fAllMotorHome=false |
| | 伺服（#11、#44） | 該軸 SvOn 0/1；整組 Z 都 ON 之後，下一個 DoSystem 放煞車 |
| | Test Range／Test Rate（#23-24） | 完整 InitMotor：**SvOn 1＋該軸座標歸 0** |
| | 打開 Motor Test 頁（#32） | 電源關著：跑一次 GaliMotorServoOff（清 fAllMotorHome、鎖煞車）；電源開著：再跑一次 DoMotorPowerOn（只等 1 秒）＋ServerON |
| | STOP（#10、#43） | 所有開成功的 1203 軸減速停＋離開 jog 模式 |
| 🟡 **不動，但改座標或 HomeFlag（之後要重歸零）** | Reload Motor Data（#25） | **所有 1203 軸** SetCmd/SetActual(0)，全部 HomeFlag=0 |
| | 參數格子（#28） | 該軸 HomeFlag=0 |
| 🔵 **只寫卡片速度，不動** | 點選馬達（#27）、捲軸（#30）、edtSpeed（#31） | 寫 PTP（捲軸再加 jog 家族）速度 |
| ⚪ **只改記憶體** | 速度參數鈕、軟體極限鈕（#15-20）、Copy From（#29） | MOT 記憶體；存檔要到 Motor Database 分頁按 Save |
| ⚪ **只動畫面** | SetPosP/N（#13-14）、btnRange/btnRate（#21-22） | C++ 什麼都不做 |
| 📄 **寫檔** | Light Scale 存檔（#35-36）、關頁（MotorTest.ini）、Motor Database Save（Mot_Table.csv） | 寫機台 D 槽／system 檔 |
| ⛔ **一定拒絕** | GoSoftP/N（#6-7）、教導頁 jog/move/home/設定（#37-42、#45-48） | 回理由，不碰卡 |
| ❔ 其他 | Reset MNet（#26） | 這台沒有 MN200 卡，應無作用 |

---

## §5 現在生效的安全相關行為

### 5-1 VerifyMotorAction 鎖（golden 完整照翻，EastSun R8）

* 位置：`cs:30050-30153`；hook 由 wb_serve 註冊（`WMAL:1015-1027`）：moving `WMA:2798-2810`、homing `:2811-2815`、AllBtnUp `:2816-2821`、Stop1203All `:2822-2826`。
* 條件：Motor Test **或** Teach 視窗開著（瀏覽器回報的視窗總表），而且 `SystemStart==false`（`cs:30472`）。
* 任一 Enable=1 的 1203 軸「在動」（樣本不是 READY、命令後還沒有新樣本、或正在 jog），或有 HOME 工作 ⇒ **鎖**：pnlStop 變黃、
  MoveN／MoveP／LoopMove 不能按（`MT.html:905-924`；JogP/JogN golden 刻意不鎖）。
* 動作停下 **2 秒** 後 ⇒ `StopAllMotor`＋**所有開成功的 1203 軸停止**＋AllBtnUp（取消 HOME／Loop 工作）⇒ 解鎖。鎖著時安全門打開 ⇒ 同樣立刻全停。
* ⚠ **ERROR_STOP 也算「在動」**（golden MotionDone 就是 `state==READY`，T[13] `:1961`）⇒ 只要有一顆 Enable=1 的軸停在警報，鎖就**不會解除**。
* ⚠ LoopMove 的等待 3/6 秒會被「2 秒沒動」切斷（golden 同，`cs:29994-29996`）。
* 驗證：E3b-E（`test_mt_e3b_engine.cpp:274-335`）、T[13] `:1957-1974`。門開那一臂沒有測（要真的門感測器）。

### 5-2 Motor Test／Teach 開著時 MainProc 暫停（EastSun R8）

* `cs:30451-30486`：視窗開著時，MainProc 在 `ScanSystemSensor`＋`DoSystem`（`cs:30383`、`:30400`）**之後** return ——
  主流程（DoAllProcess）與停機臂（含 §5-6 的 ABORTHOME）都不跑。**DoSystem 照跑**，所以 §5-3／§5-4 的電源與煞車邏輯在頁面開著時照樣生效。
* 「開著」由瀏覽器總表決定（`WMAL:965-968`）；最小化也算開著（`WebWindowRegistry.cpp:205-208`）。⚠ 收過總表之後，
  如果**所有**回報都過期（超過 15 秒沒有新訊框）一律當成**還開著**（`WebWindowRegistry.h:104-109`、`:128-133`）⇒ 瀏覽器斷線時暫停會持續，
  直到重新回報。MT-FIX1 修掉了「HMI 重新整理後舊連線的 open 永遠留著、生產一直停住」：有新鮮回報時只看新鮮的（`WebWindowRegistry.cpp:179-186`）；
  頁面那一側另外在重新載入時補送 formClose 給 C++（`MT.html:1042-1046`）。
* 驗證：E3b-D（`test_mt_e3b_engine.cpp:251-269`）；總表政策 ctest `WebWindowRegistry`。

### 5-3 G31a：開機後自動開馬達電源（EastSun 20260926「照舊版打開」）

* `cs:16022`、`:16063-16077`：`CheckMotorPowerShutDown` 被呼叫到第 100 次時，**一次**（`flag=false`）執行 `DoMotorPowerOn()`
  （繼電器原本沒開時：繼電器 ON＋Index/Magazine/Cassette 煞車鎖住；不論如何都有 **golden 的 1 秒忙等，會凍結主執行緒 1 秒**，`cs:14381-14412`）→ `SW[SwServerON].On()`
  → `MotorPowerOnDelay=4`（`cmydef.h:34`）→ `bMotorPowerState=true`。
* 呼叫條件：DoSystem 裡 `SystemStart==false` 且 `bDoProcess`（`cs:16803-16808`），`bDoProcess` 每次 DoSystem 翻轉（`cs:16997-17003`），
  `IsSafeLockCheck()` 時整支 return（`cs:16020`）。⇒ 程式註解寫「約 100 次 DoSystem」，照程式算是約 **200 次 DoSystem**；
  以 500 ms 拍子粗估約 100 秒（實際時間請看 console，本檔沒有量）。
* 之後 4 秒 SnMotorPower 讀到 ON ⇒ G16 放煞車（受 §5-4 管）。
* 沒有對應 ctest（只有 E3b-F 測了 DoMotorPowerOn 的跨拍版）。

### 5-4 G16／G05／HOME 放煞車：該群組的軸都 Servo ON 才放（EastSun 20260926）

* 關卡 `W906_BrakeReleaseOK`（`cs:30013-30031`）擋在每一個 golden `*BreakerON()` 前面：G16 `cs:19999-20027`、G05 `cs:16375-16385`、
  HOME 開始 `uhome.cpp:2135-2139`。放行條件：`bMotorPowerState==true` **且** 群組裡每一個 Enable=1 的 PCI1203 軸都被監看器開成功、樣本有效、SVON 為 1
  （`WMAL:991-1012`）。群組（`WMAL:973-990`，**待 EastSun 在機台確認**）：
  * Index：M14 MTestZ1（M15 沒裝）
  * InOutArmZ：M03 MInArmZA、M22 MOutArmZA
  * Cassette：M35 MLoaderZ、M38 MAuto1Z、M39 MAuto2Z
  * Magazine、LDCarRotArmZ：這台沒有軸 ⇒ 直接放行
  * ⚠ **M36 MEmptyZ、M40 MAuto3Z 不在任何群組**，它們有沒有煞車、用哪個輸出要確認。
* G05 是每一個閒置的 DoSystem 都跑（`MotorPowerOnDelay<=0` 且 `iEMGPressDelay<=0`），被擋的群組下一拍再試；console 只在狀態改變時印 `brake release ALLOWED/HELD`。
* ⚠ **由程式推得、未在機台量過**：`bMotorPowerState` 只在三個地方變成 true —— G31a（每個行程一次，`cs:16075`）、面板 [Power On] 鍵
  （`cs:16125`，這台該感測器 Enable=0，`cs:16138-16141`）、機台 HOME（`forms/fMain.cpp:981`）。而 `GaliMotorServoOff` 會把它清成 false（`uhome.cpp:5006`），
  觸發它的有：Motor Power 關（#12）、**電源關著時打開 Motor Test 頁**（#32）、ABORTHOME（§5-6）。Motor Power 開（#12）本身不設 true（`cs:14440-14459`）。
  ⇒ **Motor Test 關電再開電之後，Z 群組的煞車會一直鎖住，直到按機台 HOME**；這段期間 jog Z 軸會頂著煞車。golden 放煞車不看這個旗標，所以這是本樹的新行為。
* 驗證：E3b-H（`test_mt_e3b_engine.cpp:400-434`，假 hook）。

### 5-5 G04：沒電或 EMG 時每一拍鎖 Index 並鎖全部煞車（EastSun 20260926）

* `cs:16315-16329`＋`cs:19903-19920`：這台 M14 是 PCI1203 ⇒「沒電」＝ EMG 或 SnMotorPower off。成立時每個 DoSystem：
  `LockIndexMotorAndDoHomeProcess`（iHome=1、fAllMotorHome=false、golden StopAllMotor，`cs:19923-19940`）、五組煞車全部鎖住、`DoIndexZ1Z2Free(0)`。
* ⚠ golden `StopAllMotor` 碰不到 1203 監看器開的軸（`cs:29990`；它只呼叫 golden 物件的 `PCIL132_StopMotor`，`Motor/myGALILmotor.cpp:5759-5784`）。
  G04 這一臂本身**不送 Stop1203**；1203 軸要靠之後的告警入口（§5-10）或 EMG 硬體本身。要機邊確認 EMG／斷電時軸是怎麼停的。

### 5-6 ABORTHOME：機台歸零途中停機 ⇒ 切馬達電源

* `cs:32673-32674`（golden `:18923-18924`）：歸零畫面開著（`fHome->fShow`）而進到停機拍 ⇒ `sbAbortHomeClick`（`uhome.cpp:5027-5035`）
  → `GaliMotorServoOff`（`uhome.cpp:4984-5014`）：StopAllMotor＋**Stop1203 全軸**、SwMotorRelay／SwServerON 關、bMotorPowerState=false、全部煞車鎖住、
  SystemStart=false、fAllMotorHome=false；然後 fAbort=true、關畫面。⇒ **之後要重開 Motor Power**（而且見 §5-4 的煞車問題）。
* Motor Test／Teach 開著時這一臂被 §5-2 的暫停擋住。驗證：E3b-C、E3b-D。

### 5-7 🛑 Light Scale 沒有軟體極限保護，而且停不下來（EastSun R7：完全照 golden）

* 目標：去＝`PSoftLimitP−100`，返＝`PSoftLimitN+100`（`WMA:2536-2546`）。這台四支手臂的軟體極限都是 ±999999 ⇒ **第一點就是 ±999899**，
  等於直接走到硬體極限。光學尺讀值恆為 0（沒有 MLightScale 列，`WMAL:613-633`），量到的資料沒有意義。
* 停不下來（`WMA:2425-2444`，T[13] `:2124-2147`）：
  * STOP 只停軸、**不停掃描**；golden MotorMove 在停下後把「已下命令＋READY」當成到位，記一個沒走到的點，然後**下一點照走**；
  * 告警、VerifyMotorAction 的 2 秒停止、Motor Power、關頁、操作員斷線都不停它（`WMA:2771` 不看連線、`WMA:2816-2821`）；
  * 只有**再按一次 BitBtn2** 才關計時器，而且那一段已下的移動**不會停** —— 要再按 STOP。
* 被拒的移動（門開 −1、極限 −2/−3）也算到位；rgAxis 沒點過 ⇒ 掃 MOT[0] MInArmX（`WMA:2437`）。
* 建議：機構行程與軟體極限確認之前，不要在這台按 BitBtn2。

### 5-8 操作員斷線、權杖被收回

* jog 死人開關：操作員連線消失（`ControlOwner()==0`，含權杖閒置 10 分鐘被收回）⇒ 正在 jog 的軸 Stop1203（`WMA:2747-2754`）。
* HOME／LoopMove：同樣條件或安全鎖成立 ⇒ **取消工作但不送停止**（golden 同，`WMA:2755-2767`；Poll 之後那一步也先檢查，`WMA:2776-2787`）。
  ⚠ 驅動器會把正在跑的歸零／那一段移動跑完；HomeFlag 保持 0。
* 頁面：每一條放開路徑都送停止，關視窗、pagehide 時 `releaseHeld()`（`ma.js:303-308`、`MT.html:1036`、`:1082-1085`）。

### 5-9 送出前的輪詢新鮮度

* 對一軸下過命令之後，下一個運動命令要等監看器有新樣本（≤200 ms），否則拒絕「稍候再按」（`WMA:152-161`）。停止不受限。T[10] `:1190-1193`。

### 5-10 告警

* wb_serve 的告警入口（`tools/wb_serve.cpp:5401`）⇒ `MotorAccessOnAlarm`（`WMA:2905-2926`）：取消 HOME／Loop、**Stop1203 全部開成功的軸**、
  選中軸警報燈亮 ⇒ 該軸 HomeFlag=0。Light Scale 計時器不動。T[11] `:1290-1295`、T[12] `:1636-1663`。

### 5-11 STOP 一定送得到

* `motor.stop` 免權杖、輸出優先、告警框期間照處理（§0）；C++ 端鎖死只收 `action=="stop"`，所以豁免不會變成「不拿權杖也能動馬達」（`WMAL:806-823`）。

### 5-12 歸零本身的保護

* 驅動器類型判斷不出 ⇒ 拒絕（不猜 MODE12，`WMA:1188-1194`）；PHomeHighSpeed=0 ⇒ 拒絕（`:1199-1200`）；門開 ⇒ 拒絕且 HomeFlag 已清 0（`:1171-1175`）；
  5 秒沒進 HOMING ⇒ HomeFlag=2（不會假的 1，`:2090-2093`）；180 秒逾時 ⇒ 停＋2（`:1011`、`:2130`）。

---

## §6 機邊驗證清單（照風險排）

1. **Light Scale**：行程確認前不要按（§5-7）。
2. **jog 方向**：M00 M01 M14 M18 M19 M35 M36 M38 M39 M40 M108 的 Direction 已從 1 改成 0（§1），JogP 實體方向跟 golden 原設定相反；每一軸看一次 JogP 往哪走。
3. **jog 會不會動**：`ExtDrive(1)`＋`Acm_AxJog` 這條路在這張卡第一次上機（`WMA:757-758`）。
4. **M14 速度**：JogHigh 900000、Acc 9000000，先用捲軸 1% 試。
5. **STOP**：jog 中、MoveP 中、HOME 中、LoopMove 中各按一次，看 ack 的 `stopSent`／`stopRefused` 與實際減速。
6. **歸零**：每軸 124/128 的方向、6099h／609Ah 讀回值、HOMING→READY→HomeFlag=1、M03/M22 去 ZSafePos；M35/M36/M38/M39/M40 的 VelLow 300 > VelHigh 100。
7. **煞車**：Servo ON 之後 G05 何時放；Motor Power 關→開之後是否真的一直鎖到 HOME（§5-4）；M36/M40 的煞車。
8. **G31a**：wb_serve 啟動後多久自動開電、那 1 秒主執行緒停住有沒有影響（§5-3）。
9. **EMG／斷電**：G04 成立時 1203 軸怎麼停（§5-5）。
10. **VerifyMotorAction**：有軸在 ERROR_STOP 時鎖是否解不開（§5-1）；LoopMove 等 3/6 秒被切斷。
11. **到位判斷**：Loop／HOME 用 READY＋cmdPos，不看 INP 與編碼器；量一次跟隨誤差。
12. **Reload Motor Data／Test Range**：全部（或該軸）座標歸 0 之後的行為，DS402 絕對值編碼器上 SetActualPosition(0) 的意義。
13. **SVON 位元與扭力單位**：runtime 的 servoOn／torque 跟驅動器面板（或 SigmaWin）比對；扭力 `unitVerified`。
14. **Motor Power 繼電器**：SwMotorRelay／SwServerON 實際接點、DO 回讀同步。
15. **M11 閘門**：確認 Gerneral.ini `SHUTTLE_FLOODGATE` 值，=1 時 MInShuttle1 移動前會先開閘。

---

## §7 ctest 對照

### 7-1 `WebMotorAccess`（`tests/test_web_motor_access.cpp`，假後端，不碰卡）

2026-09-26 在這台執行 `build_integ_ship_x86\tests\test_web_motor_access.exe web\JSON\motor-access.json`
（執行檔 02:13 建置，比 `WMA` 00:38、測試檔 00:42 新）：**377 passed, 0 failed**，exit 0。

| 段 | 行 | 內容 |
|---|---|---|
| [1] | `:435` | 解析 |
| [2] | `:463` | catalog 覆蓋：48 命令、37 action、live 30、ui 3、合約 5＋6 列 |
| [3] | `:560` | 誠實：queued／unknown 不碰後端；教導頁運動鈕拒絕 |
| [4] | `:590` | stop |
| [4b] | `:624` | jog 放開＝只停那一軸 |
| [5] | `:663` | servoToggle |
| [6] | `:729` | 單位與速度換算手算值 |
| [7] | `:763` | W4-b1 運動（jog／MoveP/N／Go／軟體極限／參數鈕／dry 與 partial） |
| [8] | `:933` | W4-b2 home／loop／resetMNet／range-rate |
| [9] | `:1069` | 每一種成功 ack 不帶 type/id/ok/error |
| [10] | `:1116` | NB2 R21：Enable、閘門、卡片錯誤碼、jog 死人開關、樣本新鮮度、Index 速度 |
| [11] | `:1204` | NB2 R22/R23：Enable=0、HOME 門與速度、case 500、Timer1 三道閘、明講狀態、互斥 |
| [9b] | `:1366` | MT-E1：依驅動器分支的歸零、reloadMotorData |
| [12] | `:1449` | MT-E2：selectMotor／setSpeed＋jog 速度／setParamCell／copyFrom／告警燈規則／LastHomePos／Loop 計時 |
| [13] | `:1786` | MT-E3c：Motor Power／FormShow-Close／InitMotor／R2／All 模式／引擎 hook／Light Scale |
| [14] | `:2171` | MT-FIX1：Motor Power 方向、Poll 後的閘、jog 重送捲軸值、捲軸寫卡失敗 |

**沒測的**（測試檔頭 `:32-33` 自己寫明）：真實後端 `WMAL`（馬達表對號、監看器對軸、MOT[]）—— 要 wb_serve 起來在機台上量。

### 7-2 其他相關 ctest（本次沒有執行）

| ctest | 檔 | 管什麼 |
|---|---|---|
| `MT_E3b_Engine` | `tests/test_mt_e3b_engine.cpp` | A machinerecord 圍堵、B GaliMotorServoOff、C sbAbortHomeClick、D MainProc 暫停與停機臂、E VerifyMotorAction、F 跨拍 DoMotorPowerOn、G R2（讀 repo 的 Mot_Table 副本）、H 煞車 Servo ON 關卡與 G04 判準。全部用假 hook |
| `Pci1203Pure` | `tests/test_pci1203_pure.cpp` | InitMotor 設定表（golden 順序、封閉值）、扭力換算、kCmdAxTorqueLimitSet |
| `WebWindowRegistry` | `test_winregistry`（`tests/CMakeLists.txt:3097`） | 視窗總表與 fShow 政策（§5-2） |

`MT_E3b_Engine` 連整個引擎與 IO 函式庫，本次為了不碰任何機台輸出沒有執行。

---

## §8 跟 `docs/W4_PROGRESS.md` §2 的差別（那份在這台已過時的地方）

| W4 §2 | 那份寫的 | 這台今天 |
|---|---|---|
| #1-2 jog | `moveVel ±1`＋4 個 setSpeed | golden 做法：只設 jog 家族（捲軸上次的值）＋`ExtDrive(1)`＋`Acm_AxJog`；放開＝StopDec＋ExtDrive 0（MT-E1／MT-E2／MT-FIX1） |
| #8 home | 只有 DS402 124/128 | 依驅動器分支（DS402／卡片式 MODE12／判斷不出拒絕）；Light Scale 也用同一個開始（MT-E1、MT-E3c） |
| #9 loopMove | 只有單軸 | All 模式接上；計時照 golden；Poll 之後也推進（MT-E2、MT-E3c、MT-FIX1） |
| #12 motorPowerToggle | 不做（G9 關著） | live：跨拍 DoMotorPowerOn、關電 GaliMotorServoOff、方向防呆（MT-E3b/E3c/FIX1） |
| #23-24 Range／Rate | 1203 拒絕 | 1203 做完整 golden InitMotor（EastSun R4，MT-E3c） |
| #25 reloadMotorData | 不做 | live：HomeFlag 全清、表值套回、全軸 SetCmd/SetAct(0)（MT-E1、MT-E2） |
| — | 38 個命令 | 48 個：新增 labName、strngrdMotor、BitBtn1、scrlbrMotorSpeed、edtSpeed（MT-E2），FormShow、FormClose、BitBtn2、BitBtn3、btnSaveLogLightScaleData（MT-E3c） |
| — | M14 走 Galil 拒絕 | R2：Mot_Table CardModel=PCI1203 的 Index 列當一般 1203 軸（`WMA:515-522`、`WMAL:300-307`） |
| 引擎 | 無 | VerifyMotorAction、MainProc 暫停、G31a、G04/G05/G16 煞車關卡、ABORTHOME 都已生效（§5） |
| 驗證 | 57 條 | 377 條（§7-1） |

---

## §9 讀程式時看到、本檔沒有改的事

* `MT.html:1829` 註解還寫「M14 由 C++ 拒絕」—— R2 之後 M14 是一般 1203 軸（`WMA:515-522`），註解過時。
* `WebMotorAccess.h:300` 的函式說明標題還寫「38 個 catalog 命令」，下一行才補「現在 48 個命令、37 個 action」。
* `tests/test_web_motor_access.cpp` 有兩段都印 `[9]`（`:1069`、`:1366`）。
* `btnSaveLogLightScaleData` 在 wb_serve 下永遠存出 8 個空檔（fMotorTest 沒有建立，`WMAL:644-650`）。
* §5-4 的「關電再開電後煞車鎖到 HOME」與 §5-5 的「G04 不送 Stop1203」都是從程式推出來的，需要 EastSun 決定要不要處理。
