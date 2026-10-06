# 出料側滯留偵測（WAR0495）與壓偏分級（WAR0496）

> 對應程式：`acarry.cpp DetectOutKitDrainStuck()` / `ShtOutKitHasRealIC()`、
> `aTester_Front.cpp` 與 `aTester_Rear.cpp` 的 `DoFrontTestDestroyIC/DoRearTestDestroyIC` case 200、
> `ainarm2.cpp SetShuttlefCanMoveL()`、`cMyDB.cpp` 的 `DoInsertAlarmCode`。
> 首次導入：V3.33.908.15（20260826/27, RogerYang）。

---

## 1. 案例來源：偉測 HHT-479，20260825「home 後綠燈不運行」

機台 HT-9045HA / PPLD1723，V3.33.908.8，CUSTOMER_CODE=915，8-Site (2x4) hot，7 站（Ac 關）。

### 觸發鏈

| 時間 | 事件 |
|---|---|
| 21:02:04 | `JAM0404` In shuttle 2 device floating（ErrPart 6） |
| 21:02:57 | `JAM0302` Index arm 2 device pick-up error，**ErrPart `Bd`** → RETRY → 0.8 秒後 PAUSE，人離開 30 分鐘 |
| 21:39:06 | `WAR240121 MInShuttle2 -- Motor Out Of Torque or Motor Power Off Error` |
| 21:39:08 → 21:40:21 | ALARM RESET → HOME by Start → home finish |
| 21:42 起 | START 後**綠燈但零產出、全程無警報 34 分鐘**，21 顆料滯留，最後人工清料報廢 |

HOME 會保留物料帳但清掉 retry 情境，留下「兩支 shuttle 的**出料 Kit 都滿、入料 Kit 空／半滿**」的不對稱狀態。

### 21 顆的帳（`Task_ListWithTime.csv` 檔尾）

| 位置 | 內容 | 顆數 |
|---|---|---|
| FRCarryKit | `1002` ×7 | 7 |
| BRCarryKit | `1002` ×7 | 7 |
| BTestSuck | `1002` ×6 | 6 |
| BLCarryKit[1][3] | `4` = HAS_HOT_IC ← 就是 JAM0302 的 **Bd** | 1 |
| | **合計** | **21** |

`Item = TEST_PASS(1001) + Bin`，所以 `1002` = 已測完 Bin 1 的 PASS 品。

### 五方循環等待（碼級）

```
InArm  iArmTask=1550 → ainarm_SearchPickPlate case 150
       等 BLCarryKit.LeftSideNoIC(0) 為真, 但 BL 卡著 Bd 那 1 顆 → 永不前進
            ↓
Shuttle2  AutoSHT2Task 凍在 case 10
       acarry.cpp Do_Auto_SHT2 case 10: BTestSuck 有料 → BR 有料 → D43 關
       → 落到 else: IsBLCarrKitAllHasIC()==false → break（永不下移動命令）
            ↓
後測試臂  TestYRearTask=210, 手上 6 顆已測要倒進 BR —— BR 滿
            ↓
BRCarryKit 7 顆已測要 OutArm 來收 —— 需要 Shuttle2 到 iRight
            ↓
OutArm  aoutarm9045_2x4_8.cpp case 100: FR/BR 都有料但 OutSHTxInRT() 皆 false
       → FirstEnter 永不清 → 50→100→200→300 每 110ms 空轉
            ↓
Shuttle1  AutoSHT1Task 迴圈 100→120→1→10→100
       case 10: FTestSuck 空 + FL 空 → Task=100 → case 120 MotorMove(iLeft)
       ★ 這條分支從頭到尾沒有檢查 FRCarryKit ★
```

**根因**：`Do_Auto_SHT1/2` case 10 的左右決策**只看入料側**（FTestSuck / FL、BTestSuck / BL），從不看出料 Kit 是否還壓著已測 IC。正常生產不會遇到，因為 shuttle 是左右交替；但 JAM + 失電 + HOME 之後出現不對稱狀態，所有 dispatcher 都選擇「等對方」。

**為何全程無警報**：`SetHangupMaxTime()`（cinitial.cpp）把 `Prod.iHangupMaxTime` 綁在 Max Test Time 上 —— recipe `MAX Time=600` → `600+60 = 660 秒`（11 分鐘），而操作員每 1~3 分鐘按一次 PAUSE/START/HOME 就重算，永遠等不到。

### 非近期回歸

| 環節 | 最早可查到 | 894→908.12 變動 |
|---|---|---|
| SHT2 case10 `IsBLCarrKitAllHasIC()==false → break` | V3.32.811（2023-11）已在，註解 `Steven 20160530` | 900 加 `iTestMode!=SingleSite`（只放寬 1-site）；908.12 加 NN 2DID pre-scan（走 BR 空分支，不適用） |
| SHT1 case10「FTestSuck 空 + FL 空 → Task=100」不看 FR | 894→908.12 逐行相同 | 同上 |
| OutArm case100 需 `OutSHTxInRT()` | 811→908.12 只有 `HasIC()`→`UseSiteHasIC()` 換名 | 語意不變 |
| InArm case150 等 L kit 清空 | 811 在 `ainarm9045_2x4_8.cpp` 已是同形狀 | 908.0 只重構 `ep1Picker` 分支，ep8Picker 行為不變 |

Log 旁證：State Record 內附的前 4 個班次 `HOME pressed` 皆為 0、UPH 記錄 197~283 筆；事故班 `HOME pressed=5`、UPH 只有 19 筆。`JAM0302` 與 `WAR240121` 在這 3.5 天內各只出現 1 次。**程式漏洞約 10 年前就在，變新的是觸發組合。**

---

## 2. 為什麼只做偵測，不自動把 shuttle 推去右邊

原設計是「滯留 30 秒 → 強制 `Task=200`（往右）讓 OutArm 收料」，**已否決**，理由：

1. `TMyMotor::MotorMove()`（`Motor/mymotor.cpp:874-935`）在下命令前檢查
   `fCanMove / fCanMoveR / fCanMoveM / fCanMoveL` 與 `mapLockList` ——
   **任一 false 就 `PCIL132_StopMotor()` 並不下命令。這是全向鎖，不是方向鎖。**
2. 偉測三份快照都是 `MInShuttle2 fCanMoveL=false`（其餘 true）。所以就算把 Task 改成 200，
   case 210 的 `MotorMove(Prod.InSHT[1].iRight)` 會走進 StopMotor 分支；
   MInShuttle 且 `GearRatio>1` 時 cmd(68) < target(38498)-1 → **回 -3**，
   而 case 210 只認 `==1` → 變成 **Task 卡在 210 的新靜默卡死**，比原本 case 10 還在高速循環更難診斷。
3. `IsCanMove()`（`mymotor.cpp:406`）＝四個 flag 全 true。拿它當守門則逃生**永不觸發**。
4. 真正的鎖點是**別的模組的預約**（見 §3），在還沒查出上鎖者之前不做自動解鎖。

### 往右路徑的實體守門（在 case 210，不是 case 200）

| 守門 | 位置 | 檢查 | 開關 |
|---|---|---|---|
| `IsTestZ1/Z2NotSafeShuttleXCanNotMove()` | acarry.cpp | `MTestY1/2` 在 Front/Rear 範圍 + `MTestZ1/2` 低於 `TestZx_Safe` → StopMotor + 報警 → break；另查 Z Home LED | **無開關，永遠生效** |
| `DoInOutARM_SHT_MoveSafe()` | acarry.cpp | InArm/OutArm 任一 Z 的 `Led[iHomeLed]==false`（吸嘴不在上）且 XY 落在該 shuttle 範圍 → StopMotor 並把 Z 移回安全位 | ⚠ 被 `IniConfig.bF21InOutArmZMotorPrivate` 包住，**偉測 F21=0 → 跳過** |

補充：`IsTestZ1NotSafe...` 裡的 `ShowIndexAndShuttlePositionNotCorrect()` 只在 `MOT[MInShuttle1].IsCanMove()` 為真時才叫，所以 shuttle 已被鎖住時連這道警報都不會跳，直接 silent break。

---

## 3. 未解的承重點：誰在 home 之後重新鎖 `fCanMoveL`

HomeStep case 1600（`uhome.cpp:4183-4188`）把**全馬達** `fCanMove/L/M/R` 清成 true（log 證實 22:10:41.643 走到 1600），但 22:13:09 快照 `MInShuttle2.fCanMoveL` 又是 false → 有人重新上鎖。

> 注意 `uhome.cpp:2478-2481` 那一組**只清 `Motor->Enable==false` 的軸**，不含 shuttle。

已排除：

| 候選 | 排除依據 |
|---|---|
| `CheckInShuttleSensor_Latch()`（`fCanMoveL` 最大宗寫入者，8 處） | 偉測 `Gerneral.ini In_Shuttle_Auto_Latch=0`（`eInSH8Sen`）→ 所有呼叫點被 `==eInSHAutoLtc` 擋掉，整組不執行 |
| `DoShakeShuttle` / `DoKnockShuttle` / `DoKnockShuttleFirst` | `ShakeShuttleTask` 最後變動 21:02:03.269、`KnockShuttleTask` 21:02:03.506，都在兩次 home **之前** |

唯一候選：**`CheckOneCycleAction()`（`ainarm9045.cpp:7542 / 7611`，「把蝦頭鎖定, 避免 index position error」）** ——
一個名字是「判斷」卻帶「上鎖」副作用的函式。其 2x4_8 呼叫點（`ainarm9045_2x4_8.cpp:2466`）被 `iOneCycle`/`iCleanOut` 包住，但 `OneCycleTask` 在 22:12:06/13/15 確實有變動（5→5→3），所以**不能排除**。

### ✅ 2026-09-08 定案（JSCC ILD502，V3.29.908.16，2x6 12-site，D43 開）

**重鎖者 = InArm `DoInArmPlaceToShuttle_9045_2x6_8` case 2000**（`ainarm9045_2x6_8.cpp:1911-1954`；各模式檔同形狀）。
Home 後 InArm 手上有料、In kit（BL）殘留 1 顆（Index 取料異常沒拿走的 Ae）→ case 1900 `BLCarryKit.RightSideNoIC(iShtKitStep)==false` 放不下去 → 1910 → 1930 → 2000：
`InSHT2InLF()` 為真、BL 有 `NULL_IC` → `bCanFreeShuttle=false` → **`SetShuttlefCanMoveL(1, false, __FUNC__, "2000")`** → Task=1900 → 每 ~125 ms 一圈重鎖。
StateRecord 檔尾 `InArmPlaceToShuttleTask` 1900/1910/1930/2000 四值各 125 筆、`MInShutte2 fCanMoveL=false`、`AutoSHT2Task=1`（不是 10）。

**這也是看門狗盲區**：SHT2 停在 case 1（`IsCanMove()==false`）→ `DetectOutKitDrainStuck(1)` 掛在 case 10 永遠不被呼叫 → Home 後 23 分鐘零 alarm。修法 F3 = case 1 也呼叫一次（observe-only，gap guard 已內建）。

**Home 前為什麼 L=1、Ltgl=0**：InArm 被 `bShuttle2HasPickErr`（aTester_Rear.cpp:1627）擋在 PlaceToShuttle case 100 → 回 Task=1 → 不進 2000。Home 在 csystem.cpp:10760 清掉它，InArm 才放行、才開始重鎖。

**本案的上游（HHT-479 沒有的部分）**：`aTester_Rear.cpp:1733 case 306` D43 分支只有 `BRCarryKit.UseSiteNoIC()` 才有出口；取料異常若落在 OutArm 兩趟收 BR 的中間（BR 非空）→ 不設旗標、不進 320、**不報 JAM0302**、無 timeout；同刻 OutArm 第一趟料放 Fix3 → `UseFix3Cylinder` case 1（`aoutarm9045.cpp:906`）`bShuttleMoveToLeftforFix3=true` → shuttle 被拉回左邊切斷第二趟 → 五方等待成形 → WAR0495 於 98 秒後正確報出。

完整簽章、機制、修法 F1~F3 與現場處置見 `ht9045-staterecord-analysis/references/deadlock-patterns.md` **Pattern #25**。

⚠ 上表「唯一候選 `CheckOneCycleAction()`」對偉測 HHT-479 仍待證；JSCC 案已證實**另一個**重鎖者，兩案 Home 後的 InArm 狀態不同（偉測 InArm 在 `iArmTask=1550` HP 取料等 `LeftSideNoIC`，JSCC InArm 手上有料在 PlaceToShuttle）。

⚠ 全程式有 **60+ 處直接寫 `MOT[].fCanMoveL`**（約 20 個檔，含 `cContact.cpp` 與各模式檔），統一走 `SetShuttlefCanMoveL()` **不是小重構**，目前未做。

---

## 4. 偵測邏輯（`DetectOutKitDrainStuck`）

### 定位

`Do_Auto_SHT1/2` case 10 的**最頂端**各一行 `DetectOutKitDrainStuck(0/1);`。
函式回傳 void、**不改 `Task`、不動任何互鎖旗標** → 控制流零改動。

### signature（全部成立才計時）

| 條件 | 不成立時的動作 |
|---|---|
| `iOneCycle==0 && iCleanOut==0` | 清計時＋清滯留次數 |
| 出料 Kit 有「真料」（`ShtOutKitHasRealIC`）| 清計時＋清滯留次數 |
| 入料 Kit **未**滿（`IsFL/BLCarrKitAllHasIC()==false`）| 只解除計時 |
| `OutArmSuck.HasIC()==false` | 只解除計時 |
| shuttle 停在 `Prod.InSHT[iSht].iLeft ±2` | 只解除計時 |

持續 `dShtStuckSec`(30) 秒 → 寫一筆 EventLog；累計 `iShtStuckMax`(3) 次 → **WAR0495**（`K_RETRY|K_HOME|K_CLEAN_OUT`），有出口、不做無限重試。

### 三個設計要點

1. **`ShtOutKitHasRealIC()` 自寫，不用現成的兩個 helper**
   - `UseSiteHasIC()` 的判斷是 `Item!=NULL_IC`，**連 `HAS_NULL_IC(5)` 都算有料** ——
     單邊 shuttle 模式把不用的 kit 設成 HAS_NULL_IC、或 lose-IC 流程刻意保留非 NULL_IC 值，
     都會讓 signature 一直成立 → 誤報。
   - `HasRealIC()` 的型別過濾對（排除 NULL_IC / HAS_NULL_IC / HAS_NULL_CLEAN_IC），
     但掃的是 `iMaxRow/iMaxCol` 而非實際使用的 `iShtRow/iShtCol`。
   - 故自寫：掃 `iShtRow/iShtCol`＋排除那三種 NULL 型別。
     若 `iShtRow/iShtCol` 為 0 則回 false（fail-safe，不會誤報）。

2. **用 `TQPF_Timer` 不用計數**
   case 10 是每 3~4ms 一圈的高速迴圈，計數式門檻 0.1 秒就被吃掉（見
   `ht9045-staterecord-analysis` LL-15）。

3. **`hStuckGap`（5 秒 gap guard）**
   `TQPF_Timer` 走實際時間，而 Pause / 警報 modal 期間本函式不會被呼叫但時間照跑；
   不擋掉會在解除 modal 的第一圈就誤觸發。
   同樣手法見 `csystem.cpp DoCleanOutFinishCheck()` 的 `hCleanOutWDGap`、
   `ainarm_SearchPickPlate.cpp` case 150 的 `dtWait150LastTick`。

### 效能

`bOutHasIC` / `bInAllHas` 是 ≤16 次整數比較。位置判斷刻意用**快取的 `MOT[].Position`**
（`ReadPos()` 回傳的就是這個成員，單位與 `Prod.InSHT[].iLeft` 相同）而不是
`CompareCommandPos()`，避免 case 10 每圈多一次 shuttle 硬體讀取。
停在原地的 shuttle，`Position` 就是最後一次命令值＝停的位置，對「持續 30 秒」的判斷毫秒級舊值無影響。

### EventLog 格式

```
ShtOutKitStuck Sht2 (1/3) cmd=68 enc=69 iLeft=68
 | CanMove=1 R=1 M=1 L=0 Ltgl=0 Lock=0[-]
 | InLF=1 InArmHasIC=0 OutArmHasIC=0 PickHPTask=150 OutArmTask=300
 | FL=0 FR=1 BL=1 BR=1 LastSetL=false@DoXXX-2260
```

判讀重點：

| 欄位 | 意義 |
|---|---|
| `L=0` | `fCanMoveL=false`，shuttle 被鎖住（全向鎖，連往右也動不了） |
| `Ltgl` | 30 秒內 `fCanMoveL` 翻轉次數。**Ltgl 大 → 有人每圈重新上鎖（predicate 帶副作用）；Ltgl=0 且 L=0 → 鎖漏掉沒解。** 這是定案 §3 承重點的關鍵欄位 |
| `Lock=n[...]` | `mapLockList` 的鎖馬達內容（`GetLockCount()` / `GetLockString()`）|
| `LastSetL` | 最後一次**改變** `fCanMoveL` 的人（`SetShuttlefCanMoveL` 記的）。若顯示舊值或空值 → 上鎖者是那 60+ 處直接寫的其中之一 |

### 已知行為（要讓現場知道）

- WAR0495 是「每個滯留事件只報一次」（`bStuckAlarmed` 只在出料 Kit 清空或進 one cycle / clean out 才重置）。
  操作員按 HOME 但沒解決時不會再報第二次，只會繼續每 30 秒寫 log。
- 滯留期間 log 每 30 秒一筆不封頂（8 小時約 960 筆）。
- 若真死結時 OutArm 剛好抓著放不掉的料，會因為 `OutArmSuck.HasIC()` 而不報 ——
  那是另一種故障（OutArm 放料死結），有它自己的警報；log 缺席本身就是「不是這個 pattern」的資訊。

---

## 5. 壓偏分級（`DoFrontTestDestroyIC` / `DoRearTestDestroyIC` case 200）

### 為什麼 `InShtInLF()==false` 底下判 encoder 是進不去的

`TMyMotor::ReadPos()` 回 **command**、`ReadEncoderPos()` 回 **encoder**
（`mymotor.cpp:417/434`；`uMotorTest.cpp:942/943` 分別綁 `edtCommandPos` / `pnlEncoderPos`）。

`InShtInLF(iSht)`（`csystem.cpp:366-388`）：

```cpp
iInPos = MOT[MInShuttle1+iSht].ReadPos();          // ← command
if(Prod.InSHT[iSht].iLeft==iInPos ||               // ← 第一個判斷就是「command 剛好等於目標」
   CompareEncoderPos(Prod.InSHT[iSht].iLeft, 9)==1)
    return true;
```

**shuttle 被 index arm 拉動時 command 完全沒變**（還是 `MotorMove(iLeft)` 寫進去的值），
所以第一個 disjunct 恆真 → `InShtInLF()` 回 true → 任何掛在 `InShtInLF()==false`
底下的 encoder 判斷都不會執行。

實機證實：偉測三份 StateRecord 皆 `MInShuttle1 cmd=53=iLeft`（teach 13 + offset 40）、
`MInShuttle2 cmd=68=iLeft`（teach 28 + offset 40）。

> 這也是 `KEN\HT9011UC_Code_V3.33.906.3_20260625_JerryYang3` 的
> `//JerryYang 20260717 : 分類壓偏原因` 三分類在實機上不生效的原因：
> 進入該 block 已代表 `cmd != iLeft`，再判 `CompareCommandPos(iLeft,2)==1` 只剩
> `|cmd-iLeft| ∈ {1,2}` 這條縫；`|cmd-iLeft| ≥ 3` 走的「情況 1」與舊碼行為完全相同。

### 現在的作法

```
InShtInLF()==false → 維持舊行為(停機), 但先寫 log: ShtPlacePosErr Sht1 : cmd/enc/iLeft/ofs
InShtInLF() 通過後才獨立算 ofs = ReadEncoderPos() - Prod.InSHT[].iLeft:
    |ofs| > iShtPlaceEncStopTol → WAR0496 + fAllMotorHome=false + iHome=1 + Task=1
    |ofs| > iShtPlaceEncWarnTol → 非阻塞寫 log: ShtPulledByIndex（每 handling cycle 一次）
```

舊碼在 `InShtInLF()` 通過後是「完全不看 encoder 直接壓下去」，所以本段**只新增偵測，不放寬任何既有行為**。

### 門檻與「為什麼 StopTol 目前是停用的」

1 count = 0.01mm（`iShtPullOfs/100.0` 印 mm；實機 iLeft=53/68 也對得上）。

| 常數 | 目前值 | 意義 |
|---|---|---|
| `iShtPlaceEncWarnTol` | `20` = 0.20mm | 只寫 log，不影響流程 |
| `iShtPlaceEncStopTol` | **`100000`（暫時停用）** | 原定 `150` = 1.50mm |

**StopTol 必須先停用**：舊碼在 `InShtInLF()` 通過後完全不看 encoder，任何長期帶著
>1.5mm 偏移的機台（stepper 失步、機構拖住伺服拉不回、teach 值與實際停位落差大）
在舊版是「靜默壓下去、照跑」；一開這道就會**每個 cycle 報警＋回 Home＝直接停產**。
1.5mm 是推定值，沒有任何機台的偏移分布資料。

**啟用流程**：先只留 warn log 收 `ShtPulledByIndex` 的實測分布 →
確認後把 `100000` 改回 `150`（Front/Rear 各一行）。

### 不可以用 `ShowMyMessage` 做「不停機提示」

`ShowMyMessage()`（`mymessbox.cpp:761-864`）內部依序做
`bHandlerPause=true` → `StopAllMotor()` → `MyMessageBox->ShowModal()`，
**modal 會凍住 MainProc 等人按**，所以它做不到「提示但不停機」。
容許帶的提示一律走 `NewRecordProcess()`。

---

## 6. Alarm code

| 碼 | 訊息 | 說明 |
|---|---|---|
| `WAR0495` | Shuttle output kit still holds tested devices and nobody can take them out! | 出料側滯留偵測連續 3 次的出口 |
| `WAR0496` | Shuttle pulled by index arm over limit at place position, check index Z offset! | index 下壓把 shuttle 拉偏超過第二上限（目前停用） |

**為何用 WAR04xx 而不是 WAR0180/0181**：
`ShowErrorMessage()` 的 EventLog `UnitName` 來自 `MyDBIEvent(Code, ...)`，
是從**碼本身**解出單元碼（`DoInsertAlarmCode` 內 `UnitNo=atoi(Code.SubString(4,2))`），
**不是**傳入的 `Pos` 參數。所以 `WAR0180` 會被歸到「01 Input Arm」，
影響 alarm history 依單元過濾、Jam 統計、以及 SECS ALID 分組
（`ID = AlarmType*1e8 + UnitNo*1e7 + iCode`）。
Shuttle 屬「04 Input Shuttle」，`AlarmCodeList.txt` 的 WAR04xx 原本到 0494，故取 0495 / 0496。

---

## 7. 上機驗證項目

| # | 測項 | 預期 |
|---|---|---|
| 1 | 重現 8/25：手動把 FR/BR 塞已測料、FL/BL 留半滿 → HOME → START | 30 秒內出現 `ShtOutKitStuck Sht1/Sht2 (1/3)`，90 秒後 WAR0495 |
| 2 | 跑一批完整 lot | EventLog grep `ShtOutKitStuck` 應為 **0 筆** |
| 3 | Unloader 滿盤長停（MES1120） | **不得**誤報 WAR0495（`OutArmSuck.HasIC()` 與 gap guard 要擋住）|
| 4 | One Cycle / Clean Out 期間 | 偵測不作用 |
| 5 | 單邊 shuttle 模式（`TestIF.iShuttleMode==1`）| 被關掉那支不得誤報 |
| 6 | Yield alarm 觸發 half one cycle（`CanYieldAlarmRemainInSHT`）| 不得誤報 |
| 7 | 跑一批後看 `ShtPulledByIndex` / `ShtPlacePosErr` 的 `ofs` 分布 | 據此定 StopTol，並回答「JerryYang 的機台落在哪個分支」|
| 8 | 暫時把 StopTol 調小強制觸發 | WAR0496 + 回 Home 正常，`Task=1` 之後不連續彈窗 |

---

## 8. 相關

- `ht9045-staterecord-analysis`：LL-15（高速重試迴圈 vs wait-type）、LL-16（`iHangupMaxTime` 綁 Max Test Time）
- `ht9045-shuttle-flow/references/Do_Auto_SHT1_SHT2_ProcessFlow.md`
- `ht9045-index-flow`：`DoTestYFront/Rear` case 210
- `ht9045-outarm-flow`：`aoutarm9045_2x4_8.cpp` case 100 的 `FirstEnter` / `OldPos`
- `ht9045-staterecord-analysis/references/deadlock-patterns.md` **Pattern #25**：JSCC ILD502 20260908 —— 本文件 §3 重鎖者定案（InArm PlaceToShuttle case 2000）＋ `case 306` D43 無出口上游 ＋ 修法 F1~F3
