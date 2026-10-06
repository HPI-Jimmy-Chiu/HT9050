> 保存來源：`.claude/skills/ht9045-index-flow/references/case-crossarm-z-safe-interlock-deadlock.md`，main `e184ef205`。以下保留原文；原文中的機型／版本與「裁決、提案、已實作」仍依原標註。當前實作狀態先看 [共同與差異](../../common.md)。

<!-- preserved-content:start -->
# Case：跨臂（Front↔Rear）Z 安全位互鎖用嚴格 `==` 卡死（「下壓後 hangup」，單臂模式必現）

> 與 `ht9045-staterecord-analysis` skill 的 [deadlock-patterns.md → Pattern #27](../../../../ht9045-state-record-analysis/references/deadlock-patterns.md) 互相引用（同一案例）。
> 本文從 **Index 模組機制** 角度切入（`DoInterFaceErrorStep` 在做什麼、為什麼會卡）；Pattern #27 從 **StateRecord 訊號特徵** 角度切入（怎麼從 CSV/Motor.xls/Galil_LOG/EventLog 看出來）。

## 一句話定義

`DoInterFaceErrorStep(ZAxisSelect)`（`atester.cpp`）是「某一支 Index 臂想把 Z 軸退回安全位之前，先確認**另一支臂**已經精確站在它自己的安全位」的跨臂互鎖檢查。它**只等待，不主動幫對向那支軸補位**，而且用的是**嚴格 `==` 相等**，沒有容差。**單臂模式下（只用 Arm2），Arm1 從未被主動測試，一旦它沒有精確停在安全位，這個互鎖就會永久卡住**——兩支測試頭全部靜止，但 `MainProc` 仍在跑、零 alarm、畫面看起來一切正常（客戶描述常是「Index 下壓後 hangup」「綠燈亮但不動作」）。

**⚠️ 2026-09-12 第二次擷取（含診斷 log）後的重大修正，務必先看這段再往下讀**：
- ~~「PAUSE 打斷移動造成殘差」~~ **不成立**——第二次擷取證實死結在**全程沒有 PAUSE**、僅執行一次 HOME 之後就已經存在。
- ~~「卡住時可嘗試 HOME」~~ **證實無效**——客戶現場對著已卡住的機台按了兩次 PAUSE→HOME→START，兩次 HOME 都正常完成，但 Z1 **兩次都精確停回同一個數值**，死結不變。
- ~~「還不足以安全修改」~~ ~~**已改判為可以修改**~~ —— 診斷 log 證實深層互鎖旗標 `IndexZCanMove` 健康（見第 5 節），且行為是決定性、可重現的。**但 2026-09-22 再修正：見下。**

**⚠️ 2026-09-22 第二次重大修正（由偉測 HHT-509 案反推），影響 §4 與 §5，請一併看**：
- ~~「`Z1Enc=-200` 是 `Z1Safe=200` 的正負鏡像 ⇒ Z1 停在錯誤位置」~~ **已推翻**。`-200` 是 `cContact.cpp` `Do_LoadCellAutoHigh()` **case 161（Load Cell 校高「上升方便取料」）主動送的合法目標值**，與機型無關（literal 寫死）。最可能的來源是**校高之後 Z1 停在那裡、單臂模式下沒人送它回 +200**，不是 Home 收尾沒走完。→ §4
- ~~建議修法「互鎖判斷失敗時主動補發 `Gali_MotMove(TestZ_Safe)`」~~ **已作廢**。那是在防撞互鎖成立的當下下動作指令。參照 Ifor 20260903 AMKOR PH 的同型修法：只清 stale latch／接手 in-flight move，**不下新動作指令**。已升格為 `pre-release-check` **D9 / D10**。→ §5
- **下一步不是寫碼，是翻 HHT-280 凍結前的 EventLog 找 Contact／Auto Height 足跡**（`Test Arm1_Contact` / `Contact Offset` / `MES2144 USER login`）。
- **目前確認的最小觸發條件：單臂模式（僅 Arm2 測試）+ 執行一次 HOME**，不需要 PAUSE、不需要特定時機。
- **客戶已改用雙臂正常模式生產，確認無此問題**——這是目前最有效、已驗證的現場迴避手段。

## 1. 涉及的函式與位置

| 函式 | 檔案 | Task 變數 | 角色 |
|---|---|---|---|
| `DoInterFaceErrorStep(int ZAxisSelect)` | `atester.cpp`（~3859-3974，908.18；~3886-4001，912.0） | `iDoInterFaceErrorStepTask`（獨立、非本文重點） | 跨臂互鎖 + tester timeout 復歸的共用函式（**第 1 層**）|
| `DoBTestSuckTestIC()` case 2500 | `aTester_Rear.cpp`（~3388，908.18；~3433，912.0） | `iBTestSuckTestICTask`（CSV 名 `RearTestSuckTestICTask`） | Rear(Arm2) 呼叫 `DoInterFaceErrorStep(TESTZ2UP)` |
| `DoFTestSuckTestIC()` case 2500（對稱） | `aTester_Front.cpp` | `iFTestSuckTestICTask`（CSV 名 `FrontTestSuckICTask`） | Front(Arm1) 呼叫 `DoInterFaceErrorStep(TESTZ1UP)` |
| `GetTesterResult()` | `atester.cpp`（~861 起） | `iTestTask`（CSV 名 `TestTask`） | 等待 tester echo（`bEcho`），`case 60` 是常見等待點 |
| `TMyMotor::Gali_MotMove()` / `Gali_ReadPos()` | **`Motor\myGALILmotor.cpp`（唯讀／第三方驅動）** | — | **第 2 層**：`IndexZCanMove[0]/[1]` 跨軸互斥旗標；`Gali_ReadPos()` 對非 Y1 軸做正負號轉換 |
| `TMyMotor::GalilTwoY_Move()` | **`Motor\myGALILmotor.cpp`（唯讀）** | — | **第 3 層**：Y 軸換位前檢查 `Z1/Z2 encoder < -200` 即拒絕移動 |
| `TMyMotor::Gali_Two_ZAxis_Move()` | **`Motor\myGALILmotor.cpp`（唯讀）** | — | 把 Z1、Z2 綁進**同一條** Galil 指令一起送出；Home 收尾也用它 |
| Z1 Home 收尾（懷疑點，未 100% 鎖定）| `uhome.cpp`（case 2100~3000 一帶，多次呼叫 `Gali_Two_ZAxis_Move(Prod.TestZ1_Safe,...)`）| — | 單臂模式下 Z1 的 Home 收尾流程，疑似在此處把 Z1 停在與安全位正負相反的位置 |

## 2. 第 1 層：`DoInterFaceErrorStep` case 1 的互鎖邏輯

```cpp
switch(Task)
{
    case 1:
        if(ZAxisSelect==TESTZ1UP)
        {
            if(MOT[MTestZ2].Gali_ReadPos()==Prod.TestZ2_Safe)   // 要退 Z1 之前，先確認 Rear(Z2) 已精確在安全位
            {
                ... 才會去 Gali_MotMove 移動 MTestZ1 ...
            }
            // 條件不成立：什麼都不做，Task 停在 1，下一輪再檢查一次
        }
        else  // TESTZ2UP
        {
            if(MOT[MTestZ1].Gali_ReadPos()==Prod.TestZ1_Safe)   // 要退 Z2 之前，先確認 Front(Z1) 已精確在安全位
            {
                ... 才會去 Gali_MotMove 移動 MTestZ2 ...
            }
        }
        break;
    ...
}
```

這一層只是「確認」，不負責「修正」——這件事本身沒有錯（是刻意設計的碰撞防護），問題在於「對向軸精確回到安全位」這件事本來就不保證會發生，而這一層完全不主動處理。

## 3. 第 2 層（唯讀驅動層）：`IndexZCanMove[0]/[1]` 互斥旗標——**已證實健康，不是本案的阻擋者**

`Motor\myGALILmotor.cpp` 的 `Gali_MotMove()` 開頭：

```cpp
if(Mot_Name==MTestZ1){ if(IndexZCanMove[0]==false) return false; IndexZCanMove[1]=false; }
else if(Mot_Name==MTestZ2){ if(IndexZCanMove[1]==false) return false; IndexZCanMove[0]=false; }
```

**任何一支軸只要在移動，就會把「對向那支軸」的許可旗標鎖死**，直到自己這支軸真正確認到位才解鎖對方。這是比 `DoInterFaceErrorStep` 更底層、更早生效的互鎖。

**2026-09-12 診斷結果（連續 7.5 分鐘、31 筆記錄）：`IndexZCanMove[0]=1 IndexZCanMove[1]=1` 從頭到尾沒變過。** 這代表這道深層旗標**沒有被鎖死**，是健康、可用的狀態。若在應用層主動命令 Z1 移動，這個命令**不會**在這一關被擋下來——原本擔心的「修了也白修」的風險已被現場數據排除。

`Gali_MotMove()`「輪詢中」分支裡確實還有一段可疑邏輯（`if(TargetPosition!=Pos){ MovFlag=false; return false; }`，目標值對不上就悄悄放棄且不釋放對向許可），**但這次的診斷顯示 `MovFlag=0`（兩軸都不在動）且旗標健康，代表這條路徑目前沒有被觸發卡在中途**——這支軸就只是單純靜止在錯誤位置，沒有人去驅動它，而不是被這個放棄分支鎖死。

⚠️ 這段程式碼在 `Motor\` 資料夾底下，屬於本專案規則裡的**唯讀第三方驅動區，不能直接修改**——但正因為這次確認它不是阻擋者，**不需要修改它也能解掉死結**（見第 5 節修法）。

## 4. 第 3 層（唯讀驅動層）：`GalilTwoY_Move` 的 Z 軸互鎖 + 正負號轉換

```cpp
iIndexZ1Pos=MOT[MTestZ1].Gali_ReadEncoderPos();
iIndexZ2Pos=MOT[MTestZ2].Gali_ReadEncoderPos();
if(iIndexZ1Pos<-200 || iIndexZ2Pos<-200)   // 兩支手臂的 Z 軸都要檢查，不是只看在用的那支
{
    if(iRetryCnt<100){ iRetryCnt++; return false; }
    bGalilTwoYMoveFlag=false;
    ShowIndexMotorError(...);
}
```

這證實：即使某支手臂沒在測試，Y 軸換位（`GalilTwoY_Move`，每個測試循環都會呼叫）仍然持續在檢查它的 Z 軸位置。

**正負號轉換（`Gali_ReadPos()`，`myGALILmotor.cpp:2644`）**：除了 `MTestY1` 之外，所有軸的 `Gali_ReadPos()` 都會把原始 Galil 讀值再取一次負號（`Position=-Pos*GearRatio`），`Gali_MotMove()` 送指令前也會對非 Y1 軸的目標值取負號（`myGALILmotor.cpp:963-964` `if(Mot_Name!=MTestY1) Pos=-Pos;`，再經 `GetRealPos()` 換算成 Galil 原始單位）——兩邊都取負號會互相抵銷，正常情況下呼叫端傳入的「工程座標」目標值應該等於軸到位後 `Gali_ReadPos()` 讀回來的值。

> ### ⚠️ 2026-09-22 重大修正：`-200` **不是**「正負鏡像的錯誤位置」，而是程式會主動送的合法目標
>
> 原本這裡寫「`Z1Enc=-200` 恰好是 `Z1Safe=200` 的正負鏡像 ⇒ Z1 停在一個方向相反的錯誤位置」。
> **這個推論已被反例推翻。**
>
> **反例來源**：偉測 HHT-509（HT-9046LS，V3.33.912.0）2026-09-22 StateRecord 的 `Galil_LOG`，
> `PAY=3125` 出現 **14 次**（Galil `Y` 軸 = `MTestZ1`，由 `Gali_GetMOT()`／`myGALILmotor.cpp:732-742` 確認）。
> 依 `Gali_MotMove()` 的 `Pos=-Pos` + `GetRealPos()` 換算，`PAY=3125` ⇔ **`Gali_MotMove(MTestZ1, -200)`**。
> 同一份 log 的 `PAY=-3125`（＝工程座標 **+200** ＝ `Prod.TestZ1_Safe`）出現 39 次，兩者互為對照，換算關係可雙向驗證。
>
> **來源函式（已定位）**：
> ```cpp
> // cContact.cpp  TfContact::Do_LoadCellAutoHigh(int iIndex)   case 161
> case 161:                                    //kevin 20140612  上升方便取料
>     if(iIndex==0) { if(MOT[MTestZ1].Gali_MotMove(-200, GotIndexZSpeed(iSpeedZ*1000))) Task=160; }
>     else          { if(MOT[MTestZ2].Gali_MotMove(-200, GotIndexZSpeed(iSpeedZ*1000))) Task=160; }
> ```
> 即 **Load Cell 自動校高（Auto Height）流程的「上升方便取料」停靠點**，Z1／Z2 都有。
> HHT-509 那 14 次正好對應客戶 2026-09-21 整天在做校高（EventLog 有大量 `Test Arm1/2_Contact Offset` 與 `Diameter_30.000mm_ContactOffset` 變更）。
>
> **對本案（HHT-280）的意義 —— 這是一條比「鏡像」有用得多的線索**：
> HHT-280 的 Z1 穩定停在 **-200**，正是 `Do_LoadCellAutoHigh` case 161 會停的位置。
> 所以最可能的來源不是「Home 收尾沒走完」，而是：**進生產之前做過（或中途離開）Load Cell 校高，Z1 被停在 -200，之後沒有任何流程把它送回 `Prod.TestZ1_Safe`（+200）** ——
> 因為單臂模式下 Arm1 從來不被驅動。
>
> **【推論，待查證】** 兩個查證動作，都很便宜：
> 1. **翻 HHT-280 出事前的 EventLog**，看凍結之前有沒有進 Contact／Auto Height 畫面的足跡
>    （`Test Arm1_Contact` / `Contact Offset` / `Diameter_*_ContactOffset` 類 ChangeLog、`MES2144 USER login`）。
> 2. **查 `uhome.cpp` 在單臂模式下到底有沒有驅動 Z1**。
>
> **這兩者構成一個二選一的分叉，務必先解開**：本案現場按了兩次 HOME，Z1 兩次都還是 -200。
> - 若 HOME **根本沒碰** Z1 ⇒ 與「校高把它留在 -200」完全相容，**校高說成立**。
> - 若 HOME **有驅動** Z1 卻把它停在 -200 ⇒ 校高只是巧合，**真根因在 Home 收尾**，回頭走下面那條舊假說。
>
> ⚠ 機型不同（HT-9045HA vs HT-9046LS），座標值不保證通用，但 `-200` 這個 literal 是寫死在 `cContact.cpp` 的，與機型無關。

**原本的 `Gali_Two_ZAxis_Move` 假說（保留，優先序下調）**：`Gali_Two_ZAxis_Move()` 會把 Z1、Z2 綁進同一條 Galil 指令一起送出，只要程式呼叫這個「雙 Z 軸」版本（`DoTestHeadMotor` 的 Arm1 自檢、Home 收尾、或 `DoInterFaceErrorStep` 的 `bContactCTOverCHK` 分支），閒置的 Z1 就會跟著被送一組指令；若 Home 收尾（`uhome.cpp` case 2100~3000）在單臂模式下沒走到最後送 `Prod.TestZ1_Safe` 的那幾行，就可能停在中繼點。**這條仍未排除，但在「校高停靠點」這條線查完之前不應優先投入。**

## 5. 目前是否足以修改程式碼？——**「能不能改」可以；「改成什麼形狀」還沒答案（2026-09-22 再修正）**

### 判斷框架（可套用到未來同類「卡在互鎖判斷式」的死結）

前三項回答的是**「我能不能改」**；第 ④ 項（2026-09-22 補）回答的是**「改成什麼形狀才安全」**。
> 原本只有 ①②③ 就下修法，是這份文件 2026-09-12 版最大的漏洞 —— **能改 ≠ 改成什麼都行。**

1. **卡住的那一層，底下還有沒有更深一層的保護旗標／鎖？** 有的話，用唯讀診斷 log 把它的即時值印出來確認，不要用猜的。⚠ **量到的結果只對這台、這次有效，不可外推**（見本節末）。
2. **卡住的行為是隨機殘留、還是穩定可重現？** 隨機殘留適合先做被動保護（容差/逾時）；**穩定重現同一個數值**代表是決定性邏輯問題，主動修正會可靠生效。
3. **修正手段需不需要動到唯讀／第三方驅動區？** 只要解法能完全落在應用層，即使根因藏在唯讀區，仍然可以修。
4. **（新）這個分支被允許做什麼？** 先問**「軸現在待的位置合不合法」**：
   - **合法**（它本來就該在那裡，只是動作被打斷）→ 壞的是**帳**，修法＝清 stale latch／接手 in-flight move，**不要動軸**。
   - **不合法**（真的停在錯誤位置）→ 才需要考慮導正，且必須另外舉證幾何安全。
   - **不知道** → **還不能提修法**。這一題的答案決定修法形態，不是可選的背景調查。
   完整規則見 `pre-release-check` 的 **D9（互鎖失敗分支預設不下動作指令）** 與 **D10（誰該清／為什麼沒清到／對稱另一側為什麼沒事）**。

### 本案套用結果

| 檢查項 | 結果 |
|---|---|
| ① `IndexZCanMove[0]/[1]` 即時值 | **健康（1/1），連續 7.5 分鐘、31 筆診斷未變** → 這台這次不是阻擋者，通過 |
| ② 隨機殘留 vs 決定性重現 | **決定性**：客戶現場兩次 PAUSE→HOME→START，Z1 兩次都精確停回同一數值 -200 → 通過 |
| ③ 修法是否要動唯讀區 | **不需要**：解法可完全落在 `atester.cpp` → 通過 |
| ④ **Z1 停在 -200 是合法位置還是錯誤位置？** | ⚠ **已有強候選答案，待查證**。§4 已確認 `-200` 是 `Do_LoadCellAutoHigh` case 161（Load Cell 校高「上升方便取料」）會主動送的**合法目標值**，不是鏡像錯誤。⇒ 若成立，Z1 是**合法地停在校高停靠點**、只是單臂模式下沒人把它送回 +200，**修法形態應為①清帳／導正，而非②接手 in-flight move**。**查證動作**：翻 HHT-280 凍結前的 EventLog 找 Contact／Auto Height 足跡。 |

**結論（2026-09-22 版）**：①②③ 過，④ **從「未知」升級為「有強候選、待一次 EventLog 查證」** → **先查證，再提修法**。
2026-09-12 版寫的「不需要先查清楚 Home 為什麼會讓 Z1 跑掉」是**錯的** —— 那正是第 ④ 項要回答的問題，而且它現在看起來根本不是 Home 的問題。

> ⚠ **第 ① 項的結論不可外推**：本案量到 `IndexZCanMove` 健康，但 AMKOR PH 同型案（Ifor 20260903）的註解是 *"IndexZCanMove stays locked"*。**同一個旗標，不同案子答案相反** —— 這反而印證了第 ① 項「要量不要猜」，但也代表每一台都要各自量。偉測 HHT-509（2026-09-22）那份 StateRecord 就完全沒有這項 dump。

### 建議修法（尚未落地，待授權與版本資料夾）

> ⚠️ **2026-09-22 修正：原本這裡建議的「主動補發移動指令」方向已作廢，不要照著做。** 保留原文於本節末供對照。
> 作廢依據：同型缺陷的實際落地修法（Ifor 20260903，AMKOR PH 32-site，見下）走的是**完全相反的原則**，而且那個原則是對的。已升格為 `pre-release-check` 的 **D9 / D10**。

**原則（D9）：修法落在「互鎖／防撞判斷成立 → break / return」的分支裡時，預設不得發出新的動作指令。**
那個分支之所以會執行，前提就是系統當下判定幾何不安全；在裡面補 `Gali_MotMove` 等於在「系統說不能動」的瞬間動軸。**失敗成本不對稱——改錯不是再 hang 一次，是撞機。**

允許的三種無動作形態：

| 形態 | 做法 | 附帶條件 |
|---|---|---|
| ① 清掉已失效的 latch | 把卡住流程的旗標設回 false | 守門條件必須把**該 latch 想保護的東西重新驗證一次**（sensor／encoder，不可只看 command position） |
| ② 接手已發出、還在飛的動作 | 用**同一個目標值**再呼叫一次 `Gali_MotMove` 當 poll，追到到位 | 不得更換目標值（`Gali_MotMove` 輪詢分支遇 `TargetPosition!=Pos` 會靜默放棄且不釋放對向許可） |
| ③ 補逃生／逾時出口 | `TQPF_Timer` + alarm，讓上游放棄這一步而不是卡死整台 | 不可用計數式防抖（迴圈 ms 級，`iCount>=100` 等於沒有） |

**原則（D10）：下修法之前先回答三問** —— ①這個狀態在正常流程由誰清除？②這次為什麼沒走到那裡（被 `break`/`return` 提前跳過，還是清完又被設回去）？③**對稱的另一側（Z1↔Z2、Front↔Rear）為什麼沒事**？兩側不對稱處往往就是 race window。答不出任一問＝還沒定位到該改的地方。

#### 參照實作：Ifor 20260903 AMKOR PH 的同型修法（行號為 912.2）

| 代號 | 位置 | 做什麼 |
|---|---|---|
| **A** | `acarry.cpp:6711`（Shuttle2）／`4876`（Shuttle1 對稱側） | 互鎖失敗分支內，確認 shuttle 已到位不動且底下無料後**清掉 stale latch**。註解明寫 *"No motor command is issued in this branch."* 守門用 `InShtInRT()`＝command ±2 **且** encoder ±9 |
| **B** | `aTester_Rear.cpp:1260`／`aTester_Front.cpp:1235`／`atester_32Site.cpp:1727` | 原本 `Task=1` 會把已送進 Galil 的 Z 移動變孤兒、`IndexZCanMove` 一直鎖著；改成**接手 in-flight move 追到到位**。註解明寫 *"No new motion is started here."* |
| **D** | `acarry.cpp:6916` | Shuttle2 側清完旗標後多一個 `break`、Shuttle1 側沒有 → 多跑那一拍把旗標重新設回 true（`6748`）＝**re-arm race**。移除該 `break` |
| 閘門 | `acarry.cpp:113` `IsShtChkFlagDeadlockFixEnable()` | 目前**只對 `CC_AMKOR_Philippines` 回 true**；註解寫 *"Stage 1 … widen after field verification"* |

⚠️ 閘門判準（見 `pre-release-check` D9 可執行版）：這個缺陷的觸發條件是 **config（`iShuttleMode=1` 單臂）**，不是客戶。用客戶碼卡等於保證每個新客戶各撞一次 —— 偉測 HHT-509 已於 **2026-09-22** 撞上（見 `ht9045-staterecord-analysis`）。但放寬之前要先做 **shadow log 證明修法真的會觸發**，不要在沒有效性證據的情況下直接動防撞路徑。

#### 本案要做什麼（取代原建議）

1. **先補診斷 dump**，不改行為：`b1/b2ShuttleMoveToRight`、`MOT[MTestZ1/Z2].MovFlag`、`IndexZCanMove[0]/[1]`、`Gali_ReadPos()` vs `Prod.TestZ{1,2}_Safe` 的即時值。現行 StateRecord 這幾項都沒有。
2. **先回答 D10 三問**：`DoInterFaceErrorStep` 的互鎖在正常流程由誰把對向軸送回安全位？這次為什麼沒送？Front↔Rear 兩側寫法有沒有不對稱？——本案這三問**目前都還沒有答案**，所以還不該提具體修法。
3. 追 `uhome.cpp` Z1 Home 收尾在哪個 case 停下。**這條原本標「可選」，現在升為必要** —— 因為「Z1 停的位置到底合不合法」決定了該用形態①（清帳）還是③（逃生出口），不是無關項。

<details>
<summary>已作廢的原建議（僅供對照，勿採用）</summary>

> **優先**：`DoInterFaceErrorStep()` case 1 的判斷式失敗時，主動對「對向那支軸」補發一次 `Gali_MotMove(Prod.TestZ{1,2}_Safe, MOT[...].GailSpeed)`，而不是純等待。由於 `IndexZCanMove` 已證實健康，這個命令送得出去；一旦軸真正到位，`Gali_ReadPos()` 就會等於 `Prod.TestZ{1,2}_Safe`，互鎖自然解除。
>
> **可選、非必要**：追出 `uhome.cpp` Z1 Home 收尾流程中實際在哪個 case 停下、為何方向相反，從根因避免 Z1 一開始就跑到錯誤位置。這個調查不影響上面那條修法的可行性，可以之後再做。

**為什麼作廢**：
1. 它在防撞互鎖成立的當下下動作指令（違反 D9）。
2. 它的前提「不需要先查清楚 Home 為什麼會讓 Z1 跑掉」是錯的 —— 那正是決定修法形態的關鍵；當時 case doc 自己也寫了「這條因果鏈的最後一環尚未 100% 鎖定」，卻仍宣告可修。
3. `IndexZCanMove` 在本案量到健康（1/1），但 AMKOR PH 同型案 Ifor 的註解是 *"IndexZCanMove stays locked"* —— **那次量測不可外推到別台**。

</details>

## 6. 與其他「Index 卡住」案例的區別

| 案例 | 互鎖對象 | 前置條件 | 訊號 |
|---|---|---|---|
| Pattern #1（deadlock-patterns.md） | TestZ2 vs Shuttle（`IsTestZ2NotSafeShuttle2CanNotMove()`） | 需先發生 JAM0302 + 操作員選 SKIP | `AutoSHT2Task=210` |
| **本案（Pattern #27）** | **Front Z1 vs Rear Z2（`DoInterFaceErrorStep`）** | **單臂模式（僅 Arm2 測試）+ 執行一次 HOME 即可，不需要 PAUSE** | **`RearTestSuckTestICTask=2500` + `TestTask=60`** |
| Pattern #19（deadlock-patterns.md） | `DoFront/RearTestDestroyIC` case 500 的 `Gali_MotMoveNoWait` 永不回 true | 丟料後 Z 軸退不回 Safe，無 PAUSE 前提 | `FrontTestDestroyICTask=500`（或 Rear）|

## 7. 安全位教點（供查詢，不建議當修法）

`Prod.TestZ1_Safe` 與 `Prod.TestZ2_Safe` **共用同一個教點來源** `Tech.iTestZ1ShutleWait`（`cinitial.cpp` 無條件賦值給兩者），UI 對應教點畫面欄位 `setEditTestZSafePos`（掛在 `MTestZ1` 軸下）。本案 teach.ini 實際值 10，但會被 `iGalil_Z_SafePos`（固定常數 200，註解「避免 Home sensor 誤判」）floor clamp 到 200——這與 2026-09-12 診斷抓到的 `Z1Safe=200` 吻合。

⚠️ **這個教點本身沒有問題**，是 Z1 沒有被驅動到那個位置，不是教點設錯，**調整這個欄位無法解決本案死結**，不建議當作處置手段（教點是機差校正基準，見團隊既有共識：勿隨意調整教點當 workaround）。

## 8. 案例

### 案例一：2026-09-12 15:31:28（首次擷取，尚無診斷 log）

偉測（CC_VTEST=915）HT-9045HA / HHT-280，`V3.33.908.17`（908.18/912.0 程式碼相同，非回歸）。單臂模式（[D30] `iShuttleMode=1, iShuttle_Sel=1`＝只用 Arm2；非 [D58] 一吸一測——config `bUseArm1PickPlaceArm2Test=1` 只是機台層總開關已解鎖，但這份 recipe 的 `HandlerCondition.Data: bArm1PickPlaceArm2Test=0` 並未實際啟用）。當時判讀 Motor.xls 顯示 `MTestZ1` 現在值 -199、目標 0（後來確認這個「目標」欄位不是 `Prod.TestZ1_Safe`，而是某次舊呼叫留下的 `TargetPosition` 殘值，具參考誤導性）。StateRecord：`D:\!偉測\20260912 下壓後hangup\2026-09-12 15_31_28`。

### 案例二：2026-09-12 19:02:45（第二次擷取，**已含診斷 log，關鍵修正證據**）

同一台機台，已升級至 `V3.33.912.1`（合併了診斷 patch）。18:47:36 HOME（18:48:27 完成）→ 全程無 PAUSE 正常生產（load tray、one cycle、clean out）→ 18:55:12 起診斷 log 開始每 5 秒記一筆，連續 **31 筆、跨 7.5 分鐘，數值完全不變**：
```
Z1Enc=-200 Z1Safe=200 Z1Tar=0 Z1Mov=0, Z2Enc≈-11526 Z2Tar=11526 Z2Mov=0, IndexZCanMove[0]=1 IndexZCanMove[1]=1
```
期間操作員又按了兩次 PAUSE→HOME→START（18:56:01/18:59:43，兩次 HOME 皆回報 `WAR2208` 正常完成），**Z1 兩次都精確停回同一個 -200**，死結不變。19:02:42 PAUSE、19:02:45 手動 State Record。**客戶隨後改用雙臂正常模式生產，確認無此問題**——目前現場採用中的迴避方案。StateRecord：`D:\!偉測\20260912 下壓後hangup\2026-09-12 19_02_45`。

<!-- preserved-content:end -->
