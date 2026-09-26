# NB2 輔助產出索引（舊筆電 JIMMYCHIU-NB2 → 新電腦）

> 使用者 20260924 20:0x 指示：舊筆電的 session 當**輔助角色**，
> 「不要親自改 Code，但是可以幫忙分析或製作工具，讓新電腦可以加速開發和理解」，
> 「每次執行任務前，都從 git 下載最新進度」，Loop 到使用者回來叫停。

## 給新電腦：怎麼用這個資料夾

1. 每輪 `git pull` 後讀本檔的「最新條目」（最新的在最上面）。
2. 這裡的東西全部是**建議＋證據**，不是裁決。要不要採用由你判斷。
   標 **「待 Jimmy」** 的是要使用者裁決的，**不要自己做**。
3. 想要我做什麼分析或工具，就寫進 `docs/nb2_assist/REQUESTS.md`。那個檔歸你，我只讀，每輪都會先處理它。
4. 我的 push 可能讓你的 push 被拒一次。pull 之後重推即可，因為路徑不重疊，合併一定乾淨。

## 我遵守的邊界

| 可以 | 不做 |
|---|---|
| 新增／修改 `docs/nb2_assist/**`、`tools/nb2_assist/**` | 改任何既有檔（含 `.cpp/.h`、CMake、ctest、其他 docs、INBOX） |
| 唯讀分析：`git show`／`git grep`、讀 golden 的 **UTF-8 鏡像** | 建置進 CMake target、跑 ctest、跑 `wb_serve`、碰 `system/` `config/` |
| 獨立工具（Python），放在 `tools/nb2_assist/` 子目錄 | 在 `tools/` 頂層放 `.ps1`（`wave_wrapup_gate.ps1:404` 會把未登記的頂層 ps1 判成 gate 清單不完整） |

分析一律在獨立的 detached worktree `D:\HT9045\_wt_assist` 做，每輪都對齊 `origin/feat/v912-port` 最新。
golden 只讀鏡像，不直接開原樹（見工具 1）。

---

> **給 Jimmy：要你決定的事彙整在 `docs/nb2_assist/PENDING_JIMMY.md`（依急迫性排序，一頁）。**

## 最新條目

### 📣 R28 — 20260925 09:4x：**使用者裁決三件（NB2 轉達，給新電腦照做）**

1. **HT9050 是 HT9046 家族**：使用者原話「他的確是HT9046家族，透過machine type來分類」⇒ 夜間報告 §0 第 4 件＝**選 A**。
   依 R18 的清單：凡是 `MachineTypeChoice` 分派裡列了 `Type_HT9046` 的都加 `Type_HT9050`（G 類 7 處＋H 類 4 處，含 `database.cpp:1529` 會寫回 Gerneral.ini 的那處，照規矩先備份）；
   只給 `Type_HT9046_LS`／`Type_HT1032`、刻意不含 HT9046 的 L 類 4 處**不加**。這是 NB2 對「透過 machine type 分類」的理解，有疑問請在夜間報告問使用者。
2. **Index Z 扭力上限（出貨組態）**：使用者原話「要對齊原BCB6版本做法」⇒ `atester.cpp:6126`／`:6757` 的 `COM2->iWriteAndCheckMotorTorque` 照 golden 翻成真的 RS232 寫入（R14 S1／R16），不再用回 1 的替身。
3. **NB2 範圍**：使用者原話「你的協助任務範圍是針對906 C++部分，其他版號不要處理」⇒ NB2 之後不再分析 V912／V899 等其他版號；R24 報告裡 V912 那一段僅供參考，PENDING 1b 已移除。

PENDING_JIMMY 同步更新：第 1、4、1b、19 條移到「已裁決」（19 已由 `feacf1a8` 做掉）。

### R27 — 20260925 09:1x：哨兵——Steven review6 合併後 ODR／COMDAT／替身**沒有新問題**
報告：`RD5軟體_NB2哨兵_review6合併後_20260925_091034.md`。

* COMDAT 衝突 9 個、集合與 R12a 逐項相同；同名 static 14；巨集接縫 61；重複類別 9。
* 小事：`FileRW/TestIF_File_SetUp.gen.inc:4045` 的閘理由說 `Save_SiteStatusLog` 不存在，但它已宣告在 `handlerlog.h:160`（給 Steven 的產生器）。

### R26 — 20260925 08:3x：覆核 `9dd66076`（W4-d：R22／R23 補完）＋`4ea858da`（tech.dat 一律不寫）——**承重四條到位**
報告：`RD5軟體_NB2覆核_W4-d_9dd66076_R22R23補完_20260925_083733.md`。

* R22（1203 軸看 Mot_Table Enable，任何建置都擋）、W4C-1（HOME 門開拒絕）、W4C-3（home 前送歸零速度）、R25（主執行緒抄表＋CRITICAL_SECTION）：NB2 親驗 ✅。
* 新電腦另外抓到 W4-b1 自己的缺陷：golden 在 SOFT_SIMULTE 建置把 `Motor->Enable` 全設 false，所以原本的 Move1203 在預設建置擋掉所有 1203 移動。
* NB2 這邊的 W4 安全項目前沒有未結的；剩實機驗證（ORG 感測、DS402 方向、到位判斷）。

### ⚠ R25 — 20260925 08:0x：親驗 R23 §2「W4-c 覆蓋掛鉤跨執行緒」——**屬實，升為高（有卡的機台）**
報告：`RD5軟體_NB2親驗_W4-c即時值覆蓋_跨執行緒_20260925_080846.md`。

* `/api/struct/motor/runtime`：頁面每 1 秒要一次，由 `ApiRoute` 服務，跑在 **socket 執行緒**（`WebBridgeServer.h:199`）。
  → `MotorRuntimeJson` → `W906_MotorOverlay` 直接讀 `Pci1203Monitor` 的樣本，並複製 `std::string driveErrText`。
* 主執行緒的 `Poll()` 同時在寫那個字串（`Pci1203Monitor.cpp:2371`），`Rescan()` 會 `Close()`＋`Open()` 重建樣本。三個檔裡鎖的數量都是 **0**。
  監看器 header 明寫「NOT THREAD-SAFE … Do not poll from the socket thread」。
* ⇒ 字串一讀一寫是未定義行為，而且**正好在驅動器報錯時發生**；Rescan 期間可能讀到已被清空或重建的樣本。
* 建議：照 EastSun 的 `PublishExtraTags`／`TagSnapshot` 模式，在主迴圈的 `W906_MotorAccessTick` 把欄位複製進有鎖的快取，socket 執行緒只讀快取。

### ⚠ R24 — 20260925 07:4x：查核夜間報告 §0 第 6 件（tech.dat）——**不是對齊，是版本差；另量到 V912 把 TECH 兩個欄位搬到結尾**
報告：`RD5軟體_NB2查核_tech.dat版面_夜間報告第6件_20260925_073808.md`。新工具 #19 `struct_layout_across_trees.py`。

* 那個 3792 bytes 的 `tech.dat` 是 **V899 版面**寫的：模型算 V899 的 TECH＝3792，剛好等於檔案大小；golden 906、移植、V912 都是 3872。
  906 是在結尾多加 19 個 SortArm 欄位（20 個 int）⇒ 選 B（照 golden 寫）**對 V899 安全**。夜間報告「會整批錯位」在 V899 上不成立。
* ⚠ **V912 把 `M_In/Out_iRotateA_Backlash` 從 byte 724 搬到結尾**，版面跟 V899／906 都不同，而且 V912 與 906 同為 3872 bytes。
  ⇒ W5-a 的「大小相同才寫」擋不住 V912 寫的檔；V899 → V912 升級時若走遷移路徑（缺 Update2），第 161～405 個欄位會錯位 8 bytes，並被永久寫進 teach.ini。待 Jimmy（V912 量產）。

### ⚠⚠ R23 — 20260925 07:0x：覆核 `c7da6bed`（W4-b2 HOME／LoopMove）——**機台旁試 HOME 或 LoopMove 之前先補六條**
報告：`RD5軟體_NB2覆核_W4-b2_c7da6bed_HOME_LoopMove_20260925_070700.md`（workflow 3 審查＋1 反駁，25 條全部確認；6 條「高」主迴圈親驗）。

* 先說不是問題的：golden 單軸歸零本來就**沒有**「Z 先上來」的跨軸前提；方向對應 124／+1、128／-1 正確。
* 高：
  * **W4C-1** HOME 沒安全門檢查（golden `Home()`／`MotorHome()` 門開就不動）；
  * **W4C-2** Arm Z 歸零後移 ZSafePos 繞過所有互鎖（門／鎖／軟體極限）；
  * **W4C-3** HOME 沒設歸零速度。EastSun 自己量過 `Acm_AxHome` 會拿卡上當下的 PTP 速度去填 6099h ⇒ 剛 jog 100% 的軸會用 jog 高速找原點；
  * **W4C-4** LoopMove／HOME 沒有 golden Timer1Timer 的三道閘（畫面關、安全鎖、告警），死人開關只管 jog；
  * **W4C-5** LoopMove 頁面與伺服端狀態會分岔，「按停」可能變「開始」；
  * **W4C-6** HOME／LoopMove 進行中，其他 jog／移動照樣放行（golden 每個運動鈕都擋 `btnHome->Down || btnLoopMove->Down`）。
* 中：完成判斷少了 ORG 感測；換軸／伺服／setRate 不停 loop；阻塞告警時 STOP 被回 modal-pending；W4-c 的覆蓋掛鉤在 socket 執行緒讀非執行緒安全的監看器。

### ⚠ R22 — 20260925 06:3x：覆核 `02f22d05`（R21 六條補完）——五條到位，**W4B-1 在預設建置仍開著**
報告：`RD5軟體_NB2覆核_R21補完_02f22d05_20260925_063613.md`。

* 飛梭閘門、回傳碼（`CmdFailed`）、死人開關、輪詢新鮮度、Index 速度鈕：✅。
* ⚠ W4B-1 的 `selectable` 在 `SOFT_SIMULTE` 下一律 true（照 golden `lM00Click` 的 `#ifndef`）。
  但 golden 真正擋住的是 `InitMotor` 的 `if(!Enable) return true;`（`myEthercatmotor.cpp:172-175`，**無 `#ifdef`**，任何組態都不 open 那一軸）。
  移植樹的 1203 命令與 `SOFT_SIMULTE` 無關，而預設建置是 SOFT_SIMULTE＋1203 LIVE。
  ⇒ 在有卡的機台上，`DoJog`／`DoHome`／`DoLoopMove`（Enable 檢查各 0 筆）仍然會動 Enable=0 的 1203 軸。
  建議：1203 軸不論組態，Enable=0 一律拒絕。

### ⚠ R21 — 20260925 06:0x：覆核 `5a8baf11`（W4-b1 運動按鈕）——**HT9050 有人在機台旁試運動之前，先補三條**
報告：`RD5軟體_NB2覆核_W4-b1_5a8baf11_MotorTest運動_20260925_060542.md`（workflow 4 審查＋1 反駁；17 條確認、2 條推翻；承重的 NB2 親驗；最新 `354ef34e` 上仍成立）。

* **W4B-1（高，潛在）**：jog 沒有 Enable 檢查。共用入口 `MotionPrelude` 也沒有 ⇒ W4-b2 的 HOME／LoopMove 同樣沒有。
  golden 在選馬達（`uMotorTest.cpp:740-743`）與開軸（`myEthercatmotor.cpp:172-173`）兩處就擋掉；1203 監看器會把每個實體軸都 open。
  今天 HT9050 19 個 1203 列全 Enable=1 ⇒ 還碰不到。**補在 `MotionPrelude` 一處即可**。
* **W4B-2（高，條件式）**：`Move1203` 沒有 golden 的飛梭閘門互鎖（`mymotor.cpp:652-697`：`SHUTTLE_FLOODGATE==1` 時先開閘，兩顆 OffSensor 確認才動）。
  HT9050 的 MInShuttle1（M11）是開放的 1203 軸，閘門氣缸 Enable=1。修法不必等機台端回答：照 golden 搬進 `Move1203`，它本來就在執行期讀 `SHUTTLE_FLOODGATE`。
* **W4B-3（中）**：卡片回傳碼（`.ret`）全檔沒看。停止／MoveAbs／設速度在卡片回錯誤時都報成功；golden 報 WAR16122；EastSun 自己判 `issued && ret==0`。
* 中：W4B-4 jog 沒有伺服端死人開關（WS 斷線不會停）；W4B-5 READY 與基準位置用最舊 200 ms 的快取；W4B-6 Index 軸速度參數鈕寫 0（golden 寫 `ReadSpeed()`）。
* W4-b2、W4-c 下一輪審。

### R20 — 20260925 05:3x：覆核 `e3ef5518`（W3-12c）屬實＋查核夜間報告 §0 第 5 件（1203 方向慣例）屬實
報告：`RD5軟體_NB2覆核_W3-12c_e3ef5518_方向慣例查核_20260925_053547.md`。

* 新建置上用工具 #15／#16／#17 重量：
  * `UpdateMyKitSuckDelayTimeToProd`、`InitialMachine`、`SaveMachineRecord`、`SetHangupMaxTime` 現在都有活的呼叫者；
  * 同名 static 15 → 14；
  * cinitial STRONG 13 → 4（剩的都是已判讀的誤判）。
* 還沒拆的（低優先）：`AutoClean.cpp:186-187`、`asortarm.cpp:554` 的 `Set*ArmSpeed` 空殼，`aoutarm9045.cpp:217`。
  AutoClean 本身不改 OutArm／SortArm 速度，影響小。
* §0 第 5 件的四個說法全部量過屬實（11/19 軸 Direction=1 的清單完全一致；EastSun 用 Pn000 不看 Mot_Table）。補一個細節：golden 的 `RealG00` 其實有反號，但它的唯一入口 `G00` 全樹 0 呼叫者；`mymotor.cpp:632-633` 有被註解掉的上層反號 ⇒ 不一致是歷史造成的。

### R19 — 20260925 05:0x：覆核 `3878bcc9`（W4-a MotorTest：STOP／jog 放開／伺服接進 C++）——**忠實，沒有安全退步**
報告：`RD5軟體_NB2覆核_W4-a_3878bcc9_MotorTest停止伺服_20260925_050258.md`。

* STOP、teach 頁 STOP、jog 放開、伺服逐行對 golden（`uMotorTest.cpp:850-910`／`:1647-1671`、`uteach` btnStopClick、`StopAllMotor` 預設參數）都忠實。
  兩處刻意偏離（放開不理 Home／LoopMove 早退、伺服目標不用共用 static）都往更安全或照 EastSun 的方向。
* `motor.stop` 免權杖：豁免列是完全比對，`stopOnly` 鎖死 `action=="stop"`（`WebMotorAccessLive.cpp:192`）。
* 建議 W4-b 前順手改兩個：
  * **RW4-1（低）**：DoStop 用 `accepted` 計「sent」，dry 建置會說送了 N 軸、實際 0；
  * **RW4-2（中）**：STOP 有軸拒絕停止時，頁面仍顯示綠色 ok（`motor-access.js` sendStop）。
* ⚠ **NB2 撤回 PENDING_JIMMY 第 6 條**：使用者 20260924 18:49 的通則（控制跟 EastSun）早就裁決了那四個衝突。NB2 R1 21:0x 列的時候沒讀到，是 NB2 的錯；新電腦的判讀正確。

### R18 — 20260925 04:3x：覆核 `bd90b948`（忠實、驗得好）＋量測夜間報告 §0 第 4 件——HT9050「家族分派」**不是同一種問題**
報告：`RD5軟體_NB2覆核_bd90b948_HT9050家族分派普查_20260925_043213.md`。新工具 #18 `machine_family_dispatch.py`。

* 「37 處」量過：golden **47 處**（`MachineTypeChoice` 同時提到 HT9046／HT9046_LS／HT1032 至少兩個）；移植活 **15 處**（未含 HT9050）＋閘內 9 處（BDE／UI，實際上 0 處要現在決定）。
* 15 處拆三類：
  * **G 類 7 處**（格數不變式，和 `bd90b948` 同理由）：`VacuumUnit.cpp:156` 的 `iIndexColMax`，HT9050 今天只拿到一半；`InitShuttleThreadParameter:12935`；`ChangeSite` 的 5 種模式換位 `:17830`／`:18525`／`:18556`／`:18598`／`:18640`。
    **建議先量**：用 `MachineSuckers_HT9050`（2c）夾具把 iTestMode 換成 QualSite1X4／_8Site1X4／_16Site4X4／Octal_80Kit 各跑一次，看有沒有 SenPort＝0。有的話就是 RA-01 同族，和身分裁決無關。
  * **H 類 4 處**：機台身分。其中 `database.cpp:1529` 會把 `USE_46_*` **寫回 Gerneral.ini**。
  * **L 類 4 處**：只有 HT9046_LS＋HT1032 的 Y-latch、START 8 picker；HT9050 若是 HT9046 型就**不該**加。
  * ⇒ 選項 A「全部加」會把 HT9050 做成 LS 型。
* PENDING_JIMMY 第 1 條改寫成這個問題。

### ⚠ R17 — 20260925 03:5x：覆核 W3-12 第一、二批（`bda2d6fc`／`65bc0e58`，快照 `ea8a7f91`）——**第一批解開的 5 個閘在執行期到不了**
報告：`RD5軟體_NB2覆核預勘_W3-12_cinitial_呼叫端閘_同名static攔截_20260925_035629.md`。新工具 #15 `tu_function_map.py`、#16 `gated_live_calls.py`、#17 `static_shadow_census.py`；修工具 #7 的缺席詞彙。

* **RW-01**：n4-5a／5b／5c（重吹、破壞延時）在 `UpdateMyKitSuckDelayTimeToProd` 裡，n4-7（`CheckKitSuck.ClearAll`）在 `InitialMachine` 裡——
  **兩個函式都是外部 0、本檔 0 呼叫**（nm＋TU 內）。唯一的呼叫點仍閘著：`cinitial.cpp:8308` GATE n2-2、`:9612` GATE n2-13；
  AutoClean 那一條打到檔內 `static void UpdateMyKitSuckDelayTimeToProd() {}`（`AutoClean/AutoClean.cpp:188`）。
  n2-2／n2-13 的理由是「0 definitions tree-wide」，本體就在同一支檔 ⇒ 失效；程式自己的註解寫「EXPIRED -- OPEN THIS GATE AT INTEGRATION」。
  今天真機上 `DestroyAgainCount` 停在 0 ⇒ 重吹永遠不做。
* **RW-02**：同型的還有 n2-4（`SetHangupMaxTime`）、n2-15／n2-16／n2-22（`SaveMachineRecord`）。六個一起試編 **OK**（附負對照）。
  ⚠ 開 n2-15／16／22 ⇒ 開機會寫 `D:\HT9045\system\machinerecord.dat`（golden 行為；ctest 碰不到，wb_serve 會）——放進備份清單。
  漏掉的原因：W3-12 的範圍是「被其他檔引用的 21 個」，`LoadMachineRecord` 只被本檔 `InitialHandler` 叫（在開機路徑上）；
  §12 把 `DoSetupSystemToProd` 記成 0 閘，實際 9 個。
* **RW-03**：15 個同名 `static`（nm `t`）攔截別檔的真本體（nm `T`），全是空的／回常數。應拆的：AutoClean `:186-188` 三個（出料臂／分類臂速度、吸嘴延時）、
  `asortarm.cpp:554 SetSortArmSpeed`；`aoutarm9045.cpp:217 MoveOutArmZToPlateSafe` 回 true 而 asortarm 已有真本體——NB2 親驗**不是撞機**
  （XY 移動前 `MoveOutArmToAutoSafe` 會先抬 Z），但略過 golden 的 `ZOutArmSafe` 高度與 Z 軸失步偵測。
* 全樹（工具 #16）：「閘理由點名的函式現在已有定義」STRONG 82 個，單獨開能編 44 個；cinitial 13 個人工判讀 9 個真的失效（約 7 成）。
  ⚠ `csystem.cpp:16200`（G04 會回 home）、`:15843`／`:16689`（馬達電源）、`cObserver.cpp:7062`（寫 2.bat 並執行）開前一定要重問。

### R16 — 20260925 03:2x：出貨組態「假成功」替身普查（新工具 `shipping_fake_success.py`）＋待 Jimmy 總表

* 找「只在非 SOFT_SIMULTE 才編進來的程式臂裡，呼叫到 no-op／回常數替身」的呼叫點（模擬組態永遠不跑，真機才發生，而且回報成功）：**6 個**。
* **⚠ 扭力上限寫入兩支 Index Z 都中**：`atester.cpp:6126`（Arm 0）與 **`:6757`（Arm 1）**，`COM2` 被導到回 1 的替身類別。其餘 4 個是 R8 已列的 `MoveSuckData` 錯誤回退。
* 新增 **`PENDING_JIMMY.md`**：把 R1～R16 的待 Jimmy 彙整成一頁（急 2、翻譯方向 12、流程文件 5）。
* 報告：`RD5軟體_NB2_出貨組態假成功替身普查_20260925_031755.md`

### R15 — 20260925 03:1x：對話框替身「自動答案」普查（新工具 `dialog_stub_semantics.py`）

* 4 個 `ShowMyMessageBox_YES_NO` 替身都回 0（golden 1＝是、2＝否，ShowModal 不會回 0），12 個活呼叫點逐一代入：
* **⚠ 被自動同意 3 題**：**`OmronLaser/LaserSensorShuttle.cpp:1313`／`:1610`「確定要儲存測距數值？」⇒ 沒經確認就 `SaveShuttleLaserValue` 存校正值**、
  `csystem.cpp:8937`「Initial Start???」、`:8910`「Check bin setting?」。
* 被當成不同意 4 題（fail-closed，但網頁上沒有地方可以回答）：RTC Calibration 前的「請確認 Socket 中沒有 IC 或異物」、「Load No Tray?」、「Tester Ready?」×2。
* 回傳值沒用到 4 處（`cinitial.cpp:8123` 起，開機吸嘴有料提示）：golden 會 ShowModal 擋住，移植樹直接往下跑。
* 根因與 R1-01 同族：網頁 HMI 還沒有阻塞式 YES/NO 通道。過渡做法**待 Jimmy**（回 0 還是回 2）。報告：`RD5軟體_NB2_對話框替身自動答案普查_20260925_031414.md`

### ⚠ R14 — 20260925 03:1x：預勘 D44 泵／開機讀檔序列／翻譯佇列——**兩個出貨組態的安全 P0**（NB2 親驗）

* **S1：出貨組態下 Index Z 扭力上限寫不進驅動器。** `atester.cpp:5618` `#define COM2 (&W7T1_com2_ext)`，替身 `iWriteAndCheckMotorTorque` 固定回 1；
  呼叫點 `:6126`（`Prod.iMaxPreasure`）在 `#ifdef SOFT_SIMULTE` 的 **`#else` 臂**。全樹沒有真的 `TCOM2::iWriteAndCheckMotorTorque` ⇒ 要翻 golden `rs232`。
* **S2：`ShowMyMessageBox_YES_NO` 替身回 0，但 golden「是」＝1、「否」＝2**（`golden:mymessbox.cpp` 尾段）。替身註解寫「NO(0)」是錯的。
  ⇒ `csystem.cpp:8937`「Initial Start???」（`if(ret==2) return false; else …`）**被自動當成按了「是」**。過渡做法（回 0 還是回 2）**待 Jimmy**。
* S3：`INDEX_SUCKER_TYPE`、`EP_Install` 等 `TfMain` 建構子機台鍵**一個都沒載入**（恆 0）⇒ 負壓機／雙 EP 機分支全死。**待 Jimmy**。
* S4：`InitialGaliDelayCount()` 0 個呼叫者 ⇒ Galil 到位確認次數停在 2（golden 預設 5）。
* D44：失效的第一原因是 `bNeedCheck` 讀取端恆 false（閘理由在 A4-6 後過期），不是泵；泵照 golden 翻進 `TfiosetviewShim` 試編 OK；G13 要排在泵之後。
* 開機讀檔：golden `DoReadLastData` 34 個讀檔器，移植樹 18 有對應、3 走別路、**13 缺**（全表在報告附錄 P2）。
* 報告：`RD5軟體_NB2預勘_D44泵_開機序列_翻譯佇列_20260925_031105.md`

### R13 — 20260925 02:2x：覆核 `2c12c408` mytray 重翻（忠實）＋⚠ 更正 NB2 自己的 R7

* **⚠ 更正**：R7 §B 說「`W906ART_ChangeTempMode`、`W7L1A_SaveUnloaderInfo` 是 0 呼叫點的死碼、建議刪」——**錯，不要刪**。
  兩個都經同 TU 的巨集別名被呼叫（`AutoRetest.cpp:1966`：ART 的 `fMain->ChangeTempMode` 目前是 no-op；`asendic_Auto.cpp:1465`：SaveUnloaderInfo）。
  工具已修（別名呼叫也算），「no-op＋真本體已存在＋活呼叫點」修正為 **50 個**（清單在報告 §3）。R7 報告本身也加了更正橫幅。
* mytray：22 個函式與 golden 逐敘述相同、ClearData 修對了；唯一 medium（RM-02）被反駁 agent 降為 low。
  RM-06：UnloaderInfo 只在日期資料夾存在時才會寫（golden 缺陷照翻）——**真機驗證前把 `D:\UnloaderInfo\` 加進備份清單**。
* 報告：`RD5軟體_NB2覆核_W3-11_mytray_更正R7_20260925_022207.md`

### ⚠⚠ R12 — 20260925 02:0x：覆核 n4-1／ChangeSite／開機順序——**RA-01（high）：HT9050 照文件改 Model=9050GPIB 後，ChangeSite 會用全 0 接線蓋掉有效站的吸嘴 IO**

* **RA-01（反駁 agent confirmed、NB2 親驗）**：n4-1 的 2×8 分支（`cinitial.cpp:10888-10890`）只收 HT9046／HT9046_LS／HT1032；
  `Type_HT9050`（移植樹新增的 enum，golden 沒有）落到 else ⇒ F/BTestSuck 與備份 **2×4**，但 `database.cpp:518` 給 `NEW_MAX_Index_Col=8`、InitSucker 綁 2×8、IO 表也是 2×8。
  備份只做到 col 0-3 ⇒ `Backup[i][4..7]` 的接線是 ctor 的 0 ⇒ c22dcb12 打開的還原閘（例 `cinitial.cpp:17787` `CopySuck(&FTestSuckBackup.Suck[0][5], &FTestSuck.Suck[0][0])`）
  在某些測試模式下**把 0 接線寫進有效站**。**而 `docs/HT9050_1203_BRINGUP_PARAMETERS.md:146` 正叫機台端把 Model 改成 `9050GPIB`**。
  今天 HT9050 吸嘴 Enable 全 0，真空還不會動；Enable 一打開就會驅動錯的點。
  **待 Jimmy**：(a) HT9050 維持 `9046_32GPIB`＋改文件 :146；或 (b) n4-1 第一個分支加 `Type_HT9050`（要揭露）。**裁決前建議通知機台端先不要改 Model。**
* RA-02／RB-22（medium）：`MachineSuckers_HT9050` 用的是預設機種 `Type_HT9045`、而且沒呼叫 ChangeSite ⇒ RA-01 測不到。建議加 `Type_HT9046_LS`／`Type_HT9050` 兩組並跑 ChangeSite（目前的碼 Type_HT9050 組應該會紅）。
* RB-01（medium）：開機 `fBinSel->ReadParam/ReadFile`（`tools/wb_serve.cpp:3178`）仍在 TrayAssignment→QAMode→Magazine（`:3600`）之前；golden 是 QAMode(:8926)→Magazine(:8928)→**Bin(:8930-8932)**。R3 RJ-03 只修好一半。
* 忠實的部分：n4-1 本體逐行＝golden、沒有 kit 超過陣列上限；ChangeSite／SetInOutArmParameter 正規化後與 golden **依序**逐行相同；開機順序「格數→備份→還原」成立；W3-10 馬達預期值與 Q4 一致。
* 報告：`RD5軟體_NB2覆核_n4-1_ChangeSite_開機順序_20260925_020307.md`

### R12a — 20260925 01:5x：新電腦 00:22-01:18 的 commit **沒有引入新的 ODR／COMDAT 問題**＋新工具 `nb2_dashboard.py`

* NB2 在 `4fdc1833`（含 `8445ed2f` n4-1、`c22dcb12` ChangeSite 47 閘、`93042c72` 開機順序）增量建置 `build_nb2`（Build OK，沒跑 ctest），重跑五支普查：
  COMDAT 衝突 **9 → 9（同一組）**、重複類別 9 → 9、LIKELY-STALE 閘 152 → 151（n4-1 開了）、替身與巨集接縫不變。
* **新工具 `tools/nb2_assist/nb2_dashboard.py`**：一次跑五支普查，印儀表板並和上一次快照比對（新出現／消失的項目逐條列）。
  建議每個波次**開頭與結尾**各跑一次：`python tools/nb2_assist/nb2_dashboard.py --build-dir <你的 build 目錄>`（含 COMDAT 約 1.5 分鐘）。
* 第 12 輪對這幾顆 commit 的逐敘述覆核還在跑，下一個條目回報。

### R11 — 20260925 01:4x：**巨集接縫拆除清單**（58 個，快照 `b2bee76c`）

* **A 可以直接拿掉 15、B 拿掉並補 include／宣告 25（每個都附補哪一行＋試編）、C 拿掉但要人在機台旁驗 14**、D 維持 2、E 待 Jimmy 2（`OCRInsp.cpp` 的 `fOCR`／`fLotInfo`：真類別還缺 11／4 個成員）。
* 優先：**W7C1 三件**（開 G-PTk3／G25／G28 之前）、**`csystem.cpp:4157` `MySleep`**（MainProc 兩處現在就走替身）。
  `WriteLastDataFile`＋16 個 LastSet 影子要同一顆；`DoInArm_SuckerMap` 要和 `DoSiteMappingResult` 一起（反駁 agent 更正）。
* 反駁 agent 補：W7C1 替身將來也會吃掉 G25／G28a／G28b 寫 `Gerneral.ini` 的那幾行（NB2 親驗：今天都在 `#if 0` 裡）。
* 報告：`RD5軟體_NB2巨集接縫拆除清單_20260925_013855.md`
* ⓘ **新電腦 00:22-01:18 已採用前幾輪的建議**：`8445ed2f` 開 GATE n4-1（Q5）、`93042c72` 開機順序照 golden（R3 RJ-03）、`c22dcb12` 開 ChangeSite 47 閘＋COPYBACKUP（Q5／RB-3）、`39f3a04d` W3-10 測試（Q4）。NB2 下一輪覆核這幾顆。

### R10 — 20260925 00:4x：檔案層巨集接縫普查（新工具 `macro_seam_census.py`）

* 找 `#define 真名 W*_替身` 這種**讓真名在 TU 後半段靜默變成替身**的巨集（grep 真名看到的是「正常的呼叫」，最容易被騙）。
  全樹 118 個，**真名在別處有真定義、範圍內還有活用點的 61 個**。
* 前幾名：**`OCRInsp.cpp:400` `#define fOCR W906OCR_fOCR`（到檔尾、79 個活用點；真的 `fOCR` 在 `forms/fOCR.cpp:122`）**、
  `Automation/auto9045.cpp:103` `GetLastOpenFN`（36，本地重寫）、`myGALILmotor.cpp` 的 `SaveFile`／`CheckTestZ*`、`OCRInsp.cpp:401` `fLotInfo`（13）…
* ⚠ 表單物件被換成 TU 內替身 ⇒ **同一份狀態被切成兩份**（OCRInsp 寫給 `fLotInfo` 的東西，真的 `fLotInfo` 看不到）。
* 報告：`RD5軟體_NB2_檔案層巨集接縫普查_20260925_004120.md`；下一輪對前 20 名逐一比對。

### R9 — 20260925 00:4x：替身普查 v2＋⚠ csystem.cpp W7C1 巨集陷阱（快照 `21d29895`）

* **更正 R7**：把移植樹的**自由函式**也算成真定義之後，303 個替身裡 **99 個的真本體已存在（no-op 56）**；no-op＋還有活呼叫點的 **12 個**（表在報告 §1）。
* ⚠ **`csystem.cpp:2575-2584` 用檔案層巨集把 `WriteIniData`／`GetLastOpenFN`／`DataPath` 換成空殼**，一路生效到 `:26275`。
  受影響的活碼：**`:3106-3117` [I49] 清料後把備份的 Contact Mode 與兩臂接觸高度寫回 `Contact.Data`**（現在 no-op）、`:7505` I06 寫 `config.ini`。
  [I49] 的**拉高**那一側在 `atester.cpp:3838`（`ProcessTesterTimeOut`，今天在 G-PTk3 的 `#if 0` 裡），用的是**真的** `WriteIniData`
  ⇒ **一旦打開 G-PTk3，配方的接觸高度會被加高寫進去、而且不會還原**。三個替身的理由已過期（真函式都在 `common.cpp`）。
  **建議在打開 G-PTk3 之前或同一顆 commit 拿掉這三個巨集替身。**
* 翻譯佇列 20 個（替身在、移植樹沒真本體、golden 有）：第一名 `W64bT2_ProcessIndexSuckDestroy2`（`aTester_Rear.cpp:409`，**回 `true`、9 個呼叫點**；
  golden 是 `fiosetview->ProcessIndexSuckDestroy2(1)` 的 D44 Index 回黏檢查，golden `iosetview.cpp:1881` 40 行）⇒ 後臂回黏檢查目前永遠「通過」。
* 報告：`RD5軟體_NB2替身普查v2_W7C1巨集陷阱_20260925_003838.md`

### R8 — 20260925 00:3x：**替身換真方法清單**（26 個 `W*_X` 替身，快照 `ba442747`）

* **A 可直接換 14 個**：前臂 6、後臂 7、RotateKit 1。替身本體與真方法、golden **三者逐敘述相同**（機械比對＋反駁 agent 自寫腳本重驗全部 confirmed），換了**零行為變化**。
  NB2 獨立試編：`aTester_Front.cpp` 10 個呼叫點一起換成真方法 ⇒ OK。逐行換法表見報告 §1。
* B 換會恢復 golden 行為（只動記憶體）：`W64bT2_SetHasNullIcToNullIc` 等。
* **C 一批、要人在機台旁驗**：`MoveSuckData` 前臂／後臂／32-site（含 32-site 的 SPLIT／MERGE，三個巨集必須一次換）。
  現在 no-op ⇒ 狀態機停在破真空那一步；換了之後會往下走、下 Index Z 與飛梭指令。**`#ifndef SOFT_SIMULTE` 裡的 4 處只有出貨組態編得到**。
* D 待 Jimmy：`W5SCKART_ACCESSFILE`（它對到的 `fSCKART.cpp:69` 本身也是替身）。
* 報告：`RD5軟體_NB2替身換真方法清單_20260925_003420.md`

### R7 — 20260925 00:1x：活碼替身遮蔽真方法普查（新工具 `shadow_stub_census.py`）

* 全樹 303 個 `W*_X` 替身（檔內 static 函式／`#define` 巨集），**27 個的 X 已有真本體**。
* ⚠ **no-op 而且還有呼叫點的 5 個**：**`W5_32S_MOVESUCKDATA`（`atester_32Site.cpp:227`，10 個呼叫點）**、`W64bT2_SetHasNullIcToNullIc`（後臂 4）、
  `W64bT2_MoveSuckData`（後臂 3，含 `:644`）、`W64B_MoveSuckData`（前臂 2）、`W64bT2_AddIndexPlaceShuttleRecord`（後臂 1）
  ⇒ **`MoveSuckData` 在前臂、後臂、32-site 三條測試流程都被 no-op 吃掉**。建議與 R6 C 類的 537 同批換成真方法、要人在機台旁驗。
* 17 個**非 no-op 的本地重寫**（`W7Ck4_CopyFrom`、`W7*_MoveAllItem`、`W7*_CheckVaccumIsIniaialON`…）要逐一比「本地版 vs 真方法 vs golden」⇒ 下一輪。
* 報告：`RD5軟體_NB2_活碼替身遮蔽真方法_20260925_000811.md`

### R6 — 20260925 00:0x：**開閘清單**（主流程 52 閘第三級判讀，快照 `b207d40f`）

* **A 可直接開 23 個**（忠實、前提失效、不碰真實檔／硬體；NB2 逐檔 `--together` 一起試編 7 支檔全部 OK）：
  `aTester_Front.cpp` 532/1020/2413/9568/9863、`aTester_Rear.cpp` 1996/2068/2353/8772/9793/10015、`cinitial.cpp` 11354、
  `ainarm2.cpp` 2245/7863、`ainarm9045.cpp` 953/11547、`asortarm.cpp` 775/892/919、`csystem.cpp` 10414/13805/20696/23983。
* B 要成組開 11、**C 開但要人在機台旁驗 6**（含 ★`aTester_Front.cpp:537` MoveSuckData 與 n4-1）、D 維持閘住 9（其中 4 個是退役替身死碼，建議刪）、E 待 Jimmy 3。
* ⚠ **閘普查掃不到的同類洞**（NB2 親驗）：`aTester_Front.cpp:1076` `W64B_MoveSuckData`、`aTester_Rear.cpp:367` `W64bT2_MoveSuckData`
  是**活碼 no-op 替身**，遮蔽 A4-6 之後已存在的 `TMyKitSuck::MoveSuckData`。**後臂 destroy `aTester_Rear.cpp:644`（golden Rear:517）有和前臂 537 一樣的卡死洞**。
  建議換成真的 `dst.MoveSuckData(src,r,c)`，與 537 同批、同樣要人在機台旁驗。
* 報告：`RD5軟體_NB2開閘清單_主流程52閘_20260925_000516.md`

### R5 — 20260924 23:3x：291 個「理由失效」候選逐一**試開試編**（第二級），主流程 52 個打開就編得過

* 新工具 **`ungate_trial.py`**（單一閘）／**`ungate_batch.py`**（批次）：把 TU 複製到暫存、只在副本上拿掉那一對 `#if 0`／`#endif`，
  用 `build.ninja` 裡該 TU 的真旗標跑 `g++ -fsyntax-only`。**原始碼樹不動**。291 個候選 1 分 37 秒。
* 結果：**OK 98**（單獨打開能編）、FAIL 191（131 個是 `not declared in this scope`）、SKIP 2。
  對照組 `uHGemHT9045.cpp:5566` 如預期 FAIL（`'class TfNote' has no member named 'ReturnCode'`），證明這個檢查會失敗。
* **主流程檔、閘內有程式碼、單獨 OK 的 52 個**，建議順序：**n4-1（`cinitial.cpp:10885`）→ aTester_Front／Rear 18 個 → 手臂流程 → 其餘**。
* ⚠ OK 只到第二級：不含連結（`*_shims.cpp` 的 3 個可能撞重複定義）、不含多閘一起開、不含行為。
* 報告：`RD5軟體_NB2_試開閘編譯_20260924_232717.md`

### R4 — 20260924 23:2x：新工具「閘理由重驗器」＋全樹候選清單（快照 `c3dcb1d8`）

* **`tools/nb2_assist/stale_gate_reasons.py`**：掃全樹 2,496 個字面 `#if 0` 閘的理由註解（`#if 0` 那行＋閘內與閘上方緊接的註解），
  抽出「X 不存在／no such method／unregistered」裡的 X，查 X 現在在不在活碼（`A->b` 會先解 A 的型別，只在那個類別裡找 b）。6 秒。
* 結果：理由含 absence 字樣 842 個 → **LIKELY-STALE 152**（點名的全部都在）、STALE-PRIMARY 139（要人看）。
  **主流程檔裡的 LIKELY-STALE 69 個**（`aTester_Front/Rear`、`ainarm*`、`aoutarm*`、`asortarm`、`cinitial`、`csystem`…），
  包括 R3 人工確認過的 `aTester_Front.cpp:537`／`:540`、`aoutarm.cpp:1800`、`asortarm.cpp:892`、`aTester_Rear.cpp:10083`，以及 **GATE n4-1（`cinitial.cpp:10885`）**。
* 精準度：抽 8 筆，3 筆確定失效、3 筆可能、2 筆假陽性 ⇒ **候選清單，不是判決**；開閘前照規矩重問理由、量第二／三級。
* 建議：每個 W2（解 `#if 0`）波次開頭跑一次。報告：`RD5軟體_NB2_閘理由重驗候選_20260924_232141.md`
* ⓘ 順手修了工具 2（`dup_class_census.py`）的 CRLF 問題：`class X` 與 `{` 分兩行的 CRLF 檔原本會漏抓。重跑結果不變（9 個）。

### R3 — 20260924 23:1x：回覆 Q5＋覆核 W3-8／W3-9／8bfbab2f（快照 `07b80c94`）

**先看這兩件（NB2 親驗）：**

1. 🔴 **Q5：47 個 ChangeSite 還原閘「只缺吸嘴四件、今天都活了」，但要先開 GATE n4-1（`cinitial.cpp:10885-10992`）。**
   全樹 49 個 `SetItemAmount(` 全部在 n4-1 裡 ⇒ 每個 `TMyKitSuck` 都停在 ctor 的 1×1（19 號起就是這樣，不是 W3-8 造成的）
   ⇒ W3-9 的備份端只帶到 `[0][0]`，先開還原端會用全 0 的接線蓋掉活格線（604 個敘述）。
   **n4-1 自己的閘理由（等 SetItemAmount／SetMotorCount 落到 TMyKitSuck）在 A4-6 之後已失效**，而且它的呼叫者是活的（`cinitial.cpp:8025`、`tools/wb_serve.cpp:3042`）。
   順序建議：**n4-1 → 47 個 N1-G4 → COPYBACKUP 閘（RB-3）**。ChangeSite **不需要再接呼叫者**（N3-G6 已於 0921 退役，`cinitial.cpp:7126` 是活的）。
2. 🟡 **RA-14：A4-6 之後，至少 17 個閘的理由已經失效但還沒開**，例如 `aTester_Front.cpp:537`（測試主流程 `DoFrontTestDestroyIC` 跳過 golden 的 `FRCarryKit.MoveSuckData`）。建議排一次「閘理由重驗」。

**其他 medium**：RB-1（W3-9 訊息說 ChangeSite 到不了，錯）、RB-3（COPYBACKUP 閘理由失效）、
**RJ-03（開機 fQAMode 先於 fTrayAssignment，與 golden 相反 ⇒ 錯的預設值寫進真實 `Tester.Data`）**、
RJ-01（`TfTrayMappingForm` 是 static 物件，五個安裝旗標在 `main()` 前算好 ⇒ 所有機台都是 false）、RJ-02（`DoIniDataToForm` 裡的連續 FAIL 門檻鉗制被當 UI 閘掉）。
**備份清單要加**：作用中配方的 `Contact.Data`（每次開機無條件寫一個鍵）、`Tester.Data`。

報告：`RD5軟體_NB2回覆Q5_ChangeSite還原端地圖_20260924_230846.md`、`RD5軟體_NB2覆核_W3-8_W3-9_8bfbab2f_20260924_230846.md`；工具 `changesite_gate_map.py`。

### R3a — 20260924 22:4x：`8ff6c754`（W3-8 兩套 TMyKitSuck 合一）獨立驗證三層都通過（快照 `07b80c94`）

| 層 | 做法 | W3-8 之前（`e35be7f6`） | W3-8 之後（`07b80c94`） |
|---|---|---|---|
| 原始碼 | `dup_class_census.py` | 重複限定名 11 個（含 `TMyKitSuck`、`TMySucker`） | **9 個，兩者都消失** |
| 佈局 | `offsetof` 探針（mingw32，只 include 各自標頭） | `mykitsuck.h`：sizeof 42,472／`iMotRow` 31,648；`aHotPlateSubstrate.h`：11,848／7,868 | **兩邊相同**：sizeof 42,728／`iMotRow` 31,904 ⇒ `cOffSet.cpp:63` 的活 ODR 已消失 |
| 連結 | `build.bat quick`（`build_nb2`，Build OK，沒跑 ctest）＋ `comdat_odr_census.py` | 9 筆 COMDAT 大小衝突 | **同樣 9 筆，沒有新增**（沒有任何吸嘴相關的 COMDAT 衝突） |

ⓘ 合一後的完整版比 W3-8 之前的 `mykitsuck.h` 大 256 bytes（42,472 → 42,728）——多了成員。是不是 golden 本來就有的，由第 3 輪的覆核 agent 對照 golden 判斷，下一個條目回報。

### R2 — 20260924 22:2x：回覆 Q1～Q4（分析快照 `e35be7f6`；抽查在 `c6528dfe`）

| Q | 報告 | 一句話結論 |
|---|---|---|
| **Q1** | `RD5軟體_NB2回覆Q1_StringGrid越界語意_20260924_222022.md` | 改成 BCB6 語意，靜態看**只有 ctest `uHGemEquipment` 會紅**（`test_uHGemEquipment.cpp:306-308`）；SECS 已知四處行為改變都是「回到 golden」。⚠ 反駁 agent 補了 3 處漏列的依賴，並提醒「讀越界也長大」的改法會讓同列參考懸空 ⇒ 建議只有寫入才長大。附工具 `grid_cells_census.py` |
| **Q2** | `RD5軟體_NB2回覆Q2_uPadInterface預勘_20260924_222022.md` | uPadInterface 完全沒翻；**面板與托盤步進馬達共用同一個 COM 埠，歸 `dmTrayMotor`**（golden `TrayStepMotor.cpp:507`）⇒ 要一起翻 `RS232Init`／接收分流。六個閘只要 4 個方法，但狀態只有 `Main232()` 會更新。`ScanPannelKey` 今天 0 個正式呼叫者。待 Jimmy 7 件 |
| **Q3** | `RD5軟體_NB2回覆Q3_GetOffsetPath閘_20260924_222022.md` | 只有兩段被這條宣稱閘住。**⚠ G2（S7F4）不是「延後生效」**：golden 會呼叫 `ClearAllSetupFile` **即時刪除其他配方**（含教導補償）⇒ 維持閘住，理由要改寫。ainarm2 那段可開（0 呼叫者、零行為變更） |
| **Q4** | `RD5軟體_NB2回覆Q4_W3馬達段預勘_20260924_222022.md` | if 那半**沒有漏譯**；開機 InitMotor 與 EastSun `Pci1203AxisIniTick` 寫的屬性**交集是空的**；`LimitLogic` 對 1203 軸沒被用。**⚠ M108 MotorID=1080 超出 `m_Axishand[999]`**（golden 同，有卡就寫出物件尾端，待 Jimmy）。附工具 `motexp.py`（§B 手算值的模擬器）。報告開頭有一條 NB2 抽查時抓到的 agent 錯誤更正 |

**下一輪**：Q5（ChangeSite N1-G4 還原端地圖）＋覆核 `8ff6c754` W3-8（兩套 TMyKitSuck 合一）與 `2726306d` W3-9（InitSucker 解閘），
並用 ODR 普查工具在新版上**獨立驗證合一的結果**。

### R1.5 ODR 普查 — 20260924 21:3x（原始碼 `e35be7f6`；NB2 自己建置 `build_nb2`，沒跑 ctest）

* **新工具 2 支**：`dup_class_census.py`（原始碼層：同名類別的多份活定義，7 秒）、
  `comdat_odr_census.py`（編譯器層：同名 COMDAT 區段大小不同，795 個 obj 約 1 分鐘）。
  建議 A4-6 歸位前後各跑一次比對。
* 🟡 **`TTimer` 有 4 份定義、佈局不同**（`fGroundMan.h:456` 是 `int Interval`，其他是 `bool Enabled`）。
  實測 wb_serve.exe 留下的 ctor 是 `movb $0,4(%eax)`（bool 版）⇒ fGroundMan 的 `Interval` 只清 1 byte。
  今天無害（`fGroundMan.cpp:100` 使用前就指派），但它是潛伏的 ODR，建議合成一份。
* 🟡 **NB2 建出來的 wb_serve 是武裝的**（`PCIE-1203 ARMED on wb_serve`；48 個 TU 帶 `HAVE_PCI1203=1`；import `ADVMOT.dll`）。
  CLAUDE.md 那句「這台筆電 ⇒ HAVE_PCI1203 關」只對沒裝 SDK 的 JIMMYCHIU-NB 成立。**待 Jimmy**。
* 報告：`RD5軟體_NB2_ODR普查_20260924_212746.md`

### R1 結果 — 20260924 21:0x（分析快照 `05f2695b`；§A 的 ODR 在 `0de428ac` 上重驗）

**先看這三件（都由 NB2 主迴圈親手重驗過）：**

1. 🔴 **`cOffSet.cpp:63` 讓雙 `TMyKitSuck` 的 ODR 變成活的**（JerryYang `8bfbab2f`）。
   實測（mingw32）：完整版 `iMotRow` 偏移 31,648、精簡版 7,868；精簡版物件只有 11,848 位元組。
   ⇒ 開機 `fOffSet->ReadFile()`（`tools/wb_serve.cpp:2635`／`:3433`）讀到物件尾端外約 19.8 KB 的值當迴圈上界，
   結果不是「臂補償值靜默沒讀進來」就是「`dPosOffSetX[4][8]` 越界寫入」。
   **修法：`:63` 改 `#include "aHotPlateSubstrate.h"`**（精簡版有這兩個欄位，`cOffSet.cpp` 只用這兩個）。
   是否先通知 JerryYang：**待 Jimmy**。→ 報告：`RD5軟體_NB2預勘_W3吸嘴段_20260924_210127.md` §A
2. 🔴 **`MotorMove` 的錯誤分支呼叫 `ShowMyMessage`，但移植樹的 `ShowMyMessage` 不會暫停、不會停馬達、不擋 tick**
   （golden 會 `bHandlerPause=true`＋`StopAllMotor()`＋`ShowModal()`）。呼叫端用真值判斷 ⇒ 錯誤碼被當成「到位」。
   S3 武裝前要翻 `mymessbox` 的語意，或列成 🔴 gate。→ 覆核報告 §0 R1-01
3. 🔴 **`ProcessSingleMotorHome` 是回 `true` 的替身**（`acatchtray_shims.cpp:134`），接在活的 TrayZ／InArm PitchY／Rotate 流程上
   ⇒ 單軸回原點沒回就回報完成。golden 本體在 `uhome.cpp:397`／`:415`。→ W4W5 報告開頭第 3 點

**其他**：TIniFile write-through 不是原子寫入（`fopen("wb")`，中途當機會讓 `Gerneral.ini` 靜默回預設；樹裡有現成的
`MoveFileExA` 寫法可抄）；MotorTest 頁在線上模式**根本沒送出命令**，非 motion 類按鈕會顯示假成功；
兩顆 W3 commit 的閘門基準把 `dfm2rc_idempotent` 當既有失敗，與 CLAUDE.md 20260917 的更正相反。

**翻譯本體大致忠實**：MyLaneIo 三路分派、myio 六個 IO 函式、MotorMove 家族、B1 八支本體，逐敘述對 golden 都是 0 差異。

**REQUESTS Q1～Q3 已收到**（`0de428ac`），排在第 2 輪，下一個條目回報。

### R1 開工 — 20260924 20:1x（對齊 origin `05f2695b`）

* 建立本資料夾與工具 1（golden UTF-8 鏡像）。第 1 輪 5 個 agent 覆核今天的 C++ commit，並預勘 W3 吸嘴段與 W4/W5。

---

## 工具索引（`tools/nb2_assist/`）

| # | 工具 | 用途 | 用法 |
|---|---|---|---|
| 1 | `golden_utf8_mirror.py` | 把 golden（Big5）做成**行號不變**的 UTF-8 鏡像，讓 Read／Grep 與 agent 直接看得到中文註解；分析對鏡像做，碰不到 golden 本身。每檔驗行數不變，驗不過就整支失敗 | `python tools/nb2_assist/golden_utf8_mirror.py --out D:\HT9045\<你的scratch>\golden_utf8`；`--check` 只查鏡像有沒有過期 |

| 2 | `dup_class_census.py` | 原始碼層 ODR 候選：同一限定名的 class/struct 在 ≥2 個檔有活的定義（字面 `#if 0` 當死碼），附前幾個成員、inline 本體數、guard、直接 includer 數 | `python tools/nb2_assist/dup_class_census.py [--all] [--json OUT]` |
| 3 | `comdat_odr_census.py` | 編譯器層 ODR 證據：同名 COMDAT 區段（inline 函式、隱式 ctor、vtable）在不同 .obj 大小不同；標出是否同一 target／跨 ht9045_* 庫 | `python tools/nb2_assist/comdat_odr_census.py <build_dir> [--jobs 8] [--bytes]` |

| 4 | `grid_cells_census.py` | Q1：全樹活的 `Cells[..][..]` 讀寫，依外層迴圈上界初分 a／b／?，對應 golden 行，另列依賴「越界會丟例外」的地方 | `python tools/nb2_assist/grid_cells_census.py [--class a] [--dependents] [--all --format tsv]` |
| 5 | `motexp.py` | Q4：照 golden 的 `LoadMotData`＋`TMOTDATA`＋`InitialMotorParameter` 解表規則，算出每軸會建哪個類別、iAdder、Enable 與各參數（寫第三級測試的預期值用） | `python tools/nb2_assist/motexp.py machines/HT9050/Mot_Table.csv <INDEX_MOTION_CARD> [--all]` |

| 6 | `changesite_gate_map.py` | Q5：ChangeSite 的 47 個 N1-G4 閘逐閘列行範圍、golden 對應、外層條件、閘內識別字的活性；另列 ChangeSite／SetMyKitSuckItemAmount 的呼叫者 | `python tools/nb2_assist/changesite_gate_map.py`（JSON 寫到目前目錄或 `NB2_OUT`） |

| 7 | `stale_gate_reasons.py` | 閘理由重驗：`#if 0` 閘的理由寫「X 不存在」而 X 現在在活碼裡 ⇒ LIKELY-STALE 候選（含型別解析；只到第一級）。R17 補「0 definitions／occurrences／hits…」詞彙（原本漏掉 cinitial n2-2 這型）；長註解會被雜訊字稀釋成 PARTIAL ⇒ 配工具 16 用 | `python tools/nb2_assist/stale_gate_reasons.py [--show ALL] [--json OUT] [--debug NAMES]` |

| 8 | `ungate_trial.py` | 試開一個（或數個）`#if 0` 閘能不能編：複製 TU 到暫存、只改副本、用 build.ninja 的真旗標跑 `g++ -fsyntax-only`，錯誤行號換回原檔。**R8 起加 `--subst "LINE:REGEX=>REPL"`**：在副本上做任意單行替換再試編（例：把替身呼叫換成真方法） | `python tools/nb2_assist/ungate_trial.py <build_dir> <file> [<line> ...] [--together] [--subst "644:W64bT2_MoveSuckData\(BRCarryKit, BTestSuck, i, j\)=>BRCarryKit.MoveSuckData(BTestSuck, i, j)"]` |
| 9 | `ungate_batch.py` | 對 `stale_gate_reasons.py --json` 的候選逐一跑工具 8，輸出 OK／FAIL 表 | `python tools/nb2_assist/ungate_batch.py <build_dir> stale.json [--files-prefix aTester_,ainarm] [--jobs 8]` |

| 10 | `shadow_stub_census.py` | 活碼替身遮蔽真方法：`static W*_X(...)`／`#define W*_X(...)` 的 X 現在已有真定義；標出 no-op 與活呼叫點 | `python tools/nb2_assist/shadow_stub_census.py [--all] [--json OUT]` |

| 11 | `macro_seam_census.py` | 檔案層巨集接縫：`#define 真名 W*_替身` 讓真名在 TU 後半段變成替身；列生效範圍（到 `#undef` 或檔尾）、範圍內活用點、真定義所在 | `python tools/nb2_assist/macro_seam_census.py [--all] [--json OUT]` |

| 12 | `nb2_dashboard.py` | 一次跑工具 2／7／10／11（＋給 `--build-dir` 時跑 3），印儀表板並和上次快照比對 | `python tools/nb2_assist/nb2_dashboard.py [--build-dir build_xxx] [--state FILE]` |

| 13 | `dialog_stub_semantics.py` | 對話框替身（`static int W*_ShowMyMessageBox_YES_NO(...){ return N; }`）的每個呼叫點，在替身下實際被答成「是／否」 | `python tools/nb2_assist/dialog_stub_semantics.py [--json OUT]` |

| 14 | `shipping_fake_success.py` | 出貨組態才會走到的程式臂（`#ifndef SOFT_SIMULTE` 等）裡，呼叫到 no-op／回常數替身的活呼叫點（含 `#define X (&W*_obj)` 成員式替身） | `python tools/nb2_assist/shipping_fake_success.py [--json OUT]` |
| 15 | `tu_function_map.py` | 一支移植檔的每個函式 ↔ golden 同名函式：敘述集合差（不看行數）、本體內 `#if 0` 行數、替身判定、nm 外部引用、本檔呼叫（附來自哪個函式）、golden 全樹呼叫檔 → 移植沒接的；`--dump` 列出缺的敘述（golden 行號）與每段 `#if 0` 的理由 | `set NB2_GOLDEN_UTF8=<鏡像>` 後 `python tools/nb2_assist/tu_function_map.py cinitial.cpp --build-dir build_xxx [--done A,B] [--dump F,G] [--json OUT]` |
| 16 | `gated_live_calls.py` | 閘的理由點名函式 X 缺席，而 nm 顯示 X 現在已定義在某個 .obj（成員要類別也對上）＝連結層的「理由失效」；排除 RETIRED／裁決／閘內是定義的情形；`--as-stale-json` 直接餵工具 9 | `python tools/nb2_assist/gated_live_calls.py --build-dir build_xxx --as-stale-json s.json` → `ungate_batch.py build_xxx s.json --classes STRONG` |
| 17 | `static_shadow_census.py` | 同名 `static`（nm `t`）攔截別檔的全域定義（nm `T`），簽名完全相同才算；標出本體是空的／回常數、TU 內呼叫次數 | `python tools/nb2_assist/static_shadow_census.py --build-dir build_xxx [--json OUT]` |
| 18 | `machine_family_dispatch.py` | `MachineTypeChoice` 的「HT9046 家族」分派（至少提到 HT9046／HT9046_LS／HT1032 其中兩個）兩棵樹逐處對齊：活／閘內、有沒有 Type_HT9050、所在函式、接著的第一行 | `set NB2_GOLDEN_UTF8=<鏡像>` 後 `python tools/nb2_assist/machine_family_dispatch.py [--min-family 2] [--json OUT]` |
| 19 | `struct_layout_across_trees.py` | 二進位持久化結構（例 TECH→tech.dat）在 V899／golden 906／移植／V912 的欄位序列與 sizeof（int=4、double=8、bool=1、自然對齊；BCB6 `-a8` 相同），列出各樹之間的增刪與 byte 位置 | `set NB2_GOLDEN_UTF8=<鏡像>` 後 `python tools/nb2_assist/struct_layout_across_trees.py [--struct TECH] [--header LastSet.h]` |

工具 4、5、6 讀 golden 時預設找 NB2 的鏡像路徑；在別台機器用，先跑工具 1 建鏡像，再設 `NB2_GOLDEN_UTF8=<鏡像目錄>`。

NB2 實測（20260924）：848 檔、1,271,300 行，行數全部一致；5 檔有解不開的位元組（已轉成 U+FFFD，manifest 有記）。

## 報告索引（`docs/nb2_assist/`）

| 日期 | 報告 | 一句話 |
|---|---|---|
| 0925 | `RD5軟體_NB2哨兵_review6合併後_20260925_091034.md` | review6 合併後哨兵：COMDAT 集合不變、無新 ODR／替身 |
| 0925 | `RD5軟體_NB2覆核_W4-d_9dd66076_R22R23補完_20260925_083733.md` | W4-d 補完 R22／R23／R25 承重四條親驗到位；tech.dat 一律不寫 |
| 0925 | `RD5軟體_NB2親驗_W4-c即時值覆蓋_跨執行緒_20260925_080846.md` | W4-c 覆蓋掛鉤在 socket 執行緒讀非執行緒安全的 1203 監看器（無鎖；Poll 寫字串、Rescan 重建） |
| 0925 | `RD5軟體_NB2查核_tech.dat版面_夜間報告第6件_20260925_073808.md` | tech.dat 3792＝V899 版面（不是對齊）；V912 把兩個 TECH 欄位搬到結尾 ⇒ 與 V899／906 錯位 |
| 0925 | `RD5軟體_NB2覆核_W4-b2_c7da6bed_HOME_LoopMove_20260925_070700.md` | W4-b2：HOME 無門檢查／無歸零速度、ZSafePos 繞互鎖、Loop 無 Timer1Timer 閘／狀態分岔／無互斥 |
| 0925 | `RD5軟體_NB2覆核_R21補完_02f22d05_20260925_063613.md` | R21 補完五條到位；W4B-1 在 SOFT_SIMULTE＋1203 LIVE 的預設建置仍開著 |
| 0925 | `RD5軟體_NB2覆核_W4-b1_5a8baf11_MotorTest運動_20260925_060542.md` | W4-b1：Enable 閘漏（jog/home/loop）、飛梭閘門互鎖漏、卡片回傳碼沒看；另 3 中 4 低 |
| 0925 | `RD5軟體_NB2覆核_W3-12c_e3ef5518_方向慣例查核_20260925_053547.md` | W3-12c 屬實（工具重量）；§0 第 5 件方向慣例四個說法屬實＋RealG00 是死路徑 |
| 0925 | `RD5軟體_NB2覆核_W4-a_3878bcc9_MotorTest停止伺服_20260925_050258.md` | W4-a 忠實；RW4-1 accepted≠issued、RW4-2 部分停止顯示綠色；撤回 PENDING 第 6 條 |
| 0925 | `RD5軟體_NB2覆核_bd90b948_HT9050家族分派普查_20260925_043213.md` | bd90b948 忠實；HT9050 家族分派 golden 47／移植活 15，拆成 G（格數，先量）／H（身分）／L（LS 型）三類 |
| 0925 | `RD5軟體_NB2覆核預勘_W3-12_cinitial_呼叫端閘_同名static攔截_20260925_035629.md` | W3-12 覆核：5 個已開的閘沒有呼叫者（n2-2／n2-13 仍閘）；另 4 個失效呼叫端閘；15 個同名 static 攔截；全樹 82 個連結層失效候選 |
| 0924 | `RD5軟體_NB2覆核_今日C++commit_20260924_210127.md` | 5 顆 C++ commit 逐敘述對 golden；翻譯本體忠實，問題在「接到沒翻的下游」（ShowMyMessage、INSTALL_ETHETCAT、TIniFile 非原子寫入） |
| 0924 | `RD5軟體_NB2預勘_W3吸嘴段_20260924_210127.md` | §A 活的 ODR（實測偏移）；兩套 TMySucker 欄位差異、InitSucker 閘位、歸位手術的編譯碰撞與行為差、第三級測試的手算對照 |
| 0925 | `RD5軟體_NB2_對話框替身自動答案普查_20260925_031414.md` | 12 個 YES/NO 呼叫點：3 題被自動同意（含存測距校正值）、4 題 fail-closed、4 處不擋 |
| 0925 | `RD5軟體_NB2預勘_D44泵_開機序列_翻譯佇列_20260925_031105.md` | ⚠ 出貨組態：Index Z 扭力寫不進驅動器、YES/NO 確認被自動略過；機台鍵沒載入；D44 泵簡報；開機讀檔 13 缺 |
| 0925 | `RD5軟體_NB2覆核_W3-11_mytray_更正R7_20260925_022207.md` | mytray 忠實；更正 R7 兩個「死碼」其實活著；no-op 活替身修正為 50 個 |
| 0925 | `RD5軟體_NB2覆核_n4-1_ChangeSite_開機順序_20260925_020307.md` | ⚠ RA-01 high：Type_HT9050 沒被 n4-1 分類 ⇒ ChangeSite 用 0 接線蓋有效站；測試沒跑 ChangeSite；Bin 讀取順序仍錯 |
| 0925 | `RD5軟體_NB2巨集接縫拆除清單_20260925_013855.md` | 58 接縫：A 直接拿掉 15／B 補 include 25／C 機台旁 14／D 維持 2／E 待 Jimmy 2 |
| 0925 | `RD5軟體_NB2_檔案層巨集接縫普查_20260925_004120.md` | 61 個巨集接縫遮蔽已存在的真名（fOCR 79 處、GetLastOpenFN 36 處…）；表單物件狀態被切成兩份 |
| 0925 | `RD5軟體_NB2替身普查v2_W7C1巨集陷阱_20260925_003838.md` | 99 替身真本體已存在；csystem W7C1 巨集讓 [I49] 還原失效（開 G-PTk3 會永久改配方接觸高度）；翻譯佇列 20 |
| 0925 | `RD5軟體_NB2替身換真方法清單_20260925_003420.md` | 26 替身：A 可直接換 14（三者逐敘述相同）／C MoveSuckData 一批機台旁驗／逐呼叫點換法 |
| 0925 | `RD5軟體_NB2_活碼替身遮蔽真方法_20260925_000811.md` | 27 個替身的真方法已存在；5 個 no-op 還在吃掉 golden 行為（MoveSuckData 三條測試流程） |
| 0925 | `RD5軟體_NB2開閘清單_主流程52閘_20260925_000516.md` | 52 閘最終分類（A 可直接開 23／B 成組 11／C 機台旁 6／D 維持 9／E 待 Jimmy 3）＋活碼 MoveSuckData 空殼 |
| 0924 | `RD5軟體_NB2_試開閘編譯_20260924_232717.md` | 291 候選試編：OK 98（主流程有碼 52）、FAIL 191；附開閘順序建議 |
| 0924 | `RD5軟體_NB2_閘理由重驗候選_20260924_232141.md` | 全樹 152 個 LIKELY-STALE 閘（主流程 69 個）＋139 個 STALE-PRIMARY |
| 0924 | `RD5軟體_NB2回覆Q5_ChangeSite還原端地圖_20260924_230846.md` | Q5：47 閘地圖；先開 n4-1（全樹吸嘴格線目前都是 1×1） |
| 0924 | `RD5軟體_NB2覆核_W3-8_W3-9_8bfbab2f_20260924_230846.md` | 三顆本體都忠實；17 個閘理由失效、開機讀檔順序偏離、static 表單旗標、開機寫檔清單 |
| 0924 | `RD5軟體_NB2回覆Q1_StringGrid越界語意_20260924_222022.md` | Q1：999 處 Cells 存取分類；改 BCB6 語意只紅 1 個 ctest |
| 0924 | `RD5軟體_NB2回覆Q2_uPadInterface預勘_20260924_222022.md` | Q2：面板與步進馬達共用 COM 埠；函式歸屬表＋翻譯順序 |
| 0924 | `RD5軟體_NB2回覆Q3_GetOffsetPath閘_20260924_222022.md` | Q3：兩段閘；G2 會即時刪除其他配方 |
| 0924 | `RD5軟體_NB2回覆Q4_W3馬達段預勘_20260924_222022.md` | Q4：閘位、164 軸手算、EastSun 屬性對照、M108 越界、ProcessSingleMotorHome 簡報 |
| 0924 | `RD5軟體_NB2_ODR普查_20260924_212746.md` | 11 個同名類別候選＋9 筆 COMDAT 大小不一致；TTimer 實測連結結果；NB2 上 wb_serve 是武裝的 |
| 0924 | `RD5軟體_NB2預勘_W4W5盤點_20260924_210127.md` | 38 個 motor-access 命令 ↔ C++ handler（0 個）↔ EastSun ↔ golden 按鈕；線上模式不送命令；四個語意衝突待 Jimmy |

## 低風險待辦（我這邊的佇列，依價值排序）

| # | 項目 | 狀態 |
|---|---|---|
| 0 | 每輪：覆核新電腦上一輪之後新推的 C++ commit；處理 REQUESTS 新的 Q | 常駐 |
| 1 | v2 新增的「真本體已存在」替身（99－27＝72 個）逐一比對本地版 vs 真方法（同 R8 做法） | 待做 |
| 1b | 工具 16 的 44 個「單獨開能編」逐一第三級判讀（cinitial 以外的 37 個）；工具 17 的 15 個同名 static 逐一判讀 | 待做 |
| — | W3-12 覆核（cinitial 逐函式、呼叫端閘、同名 static） | ✅ R17 |
| 0 | 覆核 `2c12c408` W3-11 mytray 整支重翻＋之後新推的 C++ commit | 下一輪 |
| — | 覆核 n4-1／ChangeSite／開機順序／W3-10 測試 | ✅ R12 |
| — | 巨集接縫 58 個逐一判讀 | ✅ R11 |
| — | 檔案層巨集接縫普查 | ✅ R10 |
| — | 翻譯佇列（替身在、golden 有、移植沒翻）20 個 | ✅ R9 |
| — | 17 個非 no-op 本地重寫比對＋no-op 換法 | ✅ R8 |
| — | 工具：活碼替身遮蔽真方法普查 | ✅ R7 |
| — | 主流程 52 閘第三級判讀（開閘清單） | ✅ R6 |
| — | 主流程候選試開試編（第二級） | ✅ R5 |
| — | 工具：閘理由重驗器 | ✅ R4 |
| — | REQUESTS Q1～Q5；覆核 W3-8／W3-9／8bfbab2f；W3-8 三層驗證 | ✅ R2／R3 |
| 1 | 覆核今天的 C++ commit（W3-1、W3-2～4、C21、B1、TIniFile） | ✅ R1 |
| 2 | 預勘 W3 吸嘴段（mykitsuck／InitSucker） | ✅ R1 |
| 3 | 預勘 W4 MotorTest 38 個命令＋W5 Teach | ✅ R1（W5 較淺） |
| 4 | 預勘 W3 其餘：`mycylin`＋`InitCylinder`、`InitialSensor` 的 `SetIOTableByNUEC1`、`MyTempPanel`（67 個 `#if 0`）、`mytray`／`myTimer` | 待做 |
| 5 | 預勘 W3 馬達段：`LoadMotData`＋`InitialMotorParameter`＋五個 `InitMotor`，以及 `TMyEtherCatMotor::InitMotor` 和 EastSun `Pci1203Axis.ini` 的寫入順序衝突 | 待做 |
| 6 | 覆核 JerryYang `8bfbab2f`（開機讀檔補齊八支 ReadFile，53 檔） | 待做 |
| 7 | 覆核 Steven `05f2695b`（S12 C 路 golden 表單橋＋WebLogin，61 檔） | 待做 |
| 8 | 每輪：覆核上一輪之後新推的 C++ commit | 常駐 |
