# Auto Clean 兩臂同動（ACSim）— NN 模式 DoIndexAutoClean 逐臂取料 + 同時下壓 + 平行放回

> 建立：2026-07-29（v5 逐臂提早取料版）。
> 目標碼：`HT9011UC_Code_V3.33.910.0_20260716_New 2D\AutoClean\AutoClean.cpp`。
> 經雙代理人流程（Agent A 提案/實作 → Agent B 風險審查）五輪迭代（v1→v5）收斂。
> **v5 關鍵物理限制**：兩臂同時開啟吸取（開放式吸氣）會造成負壓壓降→吸力下降，
> 故取料必須逐臂；已吸住的 pad 屬密封狀態流量極小，持料等待與同時下壓不受影響；
> 放回用 Destroy 吹氣，無此限制，維持平行。

## 1. 功能定位

NN 模式（`bUseTwoArm32Site`，32-Site 兩臂）Auto Clean 原生流程為**全序列**：
Arm1 取 pad → Arm1 下壓 → Arm1 放回 → Arm2 取 pad → Arm2 下壓 → Arm2 放回。

ACSim（Auto Clean Simultaneous）將取料、下壓、放回三階段全部改為**雙臂同動**：

| 階段 | 原生（序列） | ACSim v5 |
|------|-------------|----------|
| 取料 | case 400~800 → 2400~2800（且 Arm1 需等兩座到齊才開始下一輪） | **重用原生 case**，但 Arm1 在 S1 就緒時立刻先吸（不等 S2）；逐臂開吸尊重真空壓降限制 |
| 下壓 | case 900~1400 → 2900~3400（兩次） | case 4300~4620（一次，雙 Z 同壓） |
| 放回 | case 1500~1900 → 3500~3900（兩次） | case 4700~4750（一次，雙 Z 同放，Destroy 吹氣無真空限制） |

純自動判斷，**無 IniConfig 開關**（bE91 提案已於審查期移除）。

## 2. 進入條件 — `RunAutoCleanTwoArmSimultaneous()`（AutoClean.cpp ~L4444）

全部成立才走 ACSim，否則走原生序列（case 100 → 200）：

1. `bACSimDegraded==false`（本輪未降級過）
2. `bUseTwoArm32Site==true`（NN 模式）
3. `USE_INDEX_ARM_AXES==IndexArm_4_Axis`（Y1/Y2 獨立軸）
4. `TestIF_File.iAutoClean_Tray==eCKPos_CleanKit`（排除 CleanAir）
5. `Prod.iAutoClean_ContactMode==0`（排除 drop contact）
6. `DeviceForm.ContactMode!=DropPlaceShiftContact`（Y middle 變體，見 §7）
7. `TestIF.bNS7000kit==false`（同上）
8. 兩臂 Auto Clean 已啟用（同 case 1900 的 SelectArm==2 / bCleanIndexOtherArm 條件）
9. `RunAutoCleanByArmPickArm2Test()==false`（顯式排除 D58 變體）
10. `CheckAutoCleanCloseSite(1)==false`（Arm2 側未全關）
11. `fCleaning->CleanPadCountCanSupport2Arm()==true`（pad 數足夠兩臂）

## 3. 狀態機流程（case 4000~4750）

```
case 100 ─(RunAutoCleanTwoArmSimultaneous)→ 4000
4000  EP 補寫電壓 + GalilTwoY_Move(Y1→Front, Y2→Rear) + 快照初始化 + 60s 供料計時
4010  逐座鎖定（S2 就緒也順手先鎖）：
      ├─ S1 鎖定即出發（不等 S2）→ iContactCount=0、InitDoTestZHome、
      │   bACSimActive=true（唯一 set 點）→ Task=400（原生 Arm1 取料）
      └─ 60s 無進展 → 跳 **WAR0359**（K_RETRY|K_SKIP，errPart="Shuttle 1"）：
          RETRY=重裝計時續等；SKIP=降級 A：解鎖早鎖的 S2 → Task=200（原生序列）
─── Arm1 取料（重用原生 400~800，含 JAM0312/D43 完整錯誤處理）───
400~800  原生單臂取料；S2 同時自由左右補料（window 未罩住 4015 以下範圍）
800   [ACSim] 刷新 BL 快照 + 重裝 60s 計時 → Task=4015
4015  Arm1 真空密封持料等待：S2 InRT+HasIC 即鎖 → iContactCount=0、
      InitDoTestZHome → Task=2400（原生 Arm2 取料）
      └─ 60s 無進展 → 跳 **WAR0359**（K_RETRY|K_SKIP，errPart="Shuttle 2"）：
          RETRY=重裝計時續等（Arm1 續密封持料）；SKIP=降級 D：bACSimActive=false、
          bACSimDegraded=true、iContactCount=0 → Task=900（Arm1 持料序列下壓，1900 後原生 2200 接 Arm2）
─── Arm2 取料（重用原生 2400~2800，含 JAM0313/D43）───
2400~2800  原生單臂取料；期間 S1 由新增 guard 子句強制維持鎖定
2800  [ACSim] 歸零 bTestSuckUse（避免 Arm2 錯誤標記遮蔽 Arm1 同座標掃描，
      skip 位置已由 Item==HAS_NULL_CLEAN_IC 過濾保障）→ Task=4300
─── 同時下壓（與 v4 相同）───
4300  runtime 自檢 Y middle 保護 → 違反 → 降級 C（Task=650）
      通過 → GalilTwoY_Move(Y1→Middle, Y2→Middle) → 4400
4400~4620  雙 Z 同壓迴圈 + 掉料掃描 + JAM0314/0315（同 v4，未動）
─── 平行放回（與 v4 相同）───
4700~4750  雙 Y 回 Front/Rear → 雙 Z 同下 place 高 → 合併釋放（JAM0327 各臂
      子狀態 4725/4726）→ 雙 Z Safe → 4750 解鎖雙 Shuttle → Task=3900（原生終態）
```

## 4. 關鍵全域變數（AutoClean.cpp L52~64 附近）

| 變數 | 說明 |
|------|------|
| `bACSimActive` | ACSim 執行中。**唯一 set 點 = 4010 S1 鎖定**（v5 不再等雙鎖）；true 期間 Task 橫跨原生取料 [400,800]/[2400,2800] 與 4015/[4300,4750]。clear：4750/3900/降級 A/C/D/`InitialSet`（L742-743）/1900 轉接分支 |
| `bACSimDegraded` | 本輪已降級，禁止再進 ACSim。reset：Auto Clean 整輪結束與 Reset |
| `bACSimLock1/2`（static） | 4010/4015 各 shuttle 鎖定旗標 |
| `bACSimArm1Drop/2Drop`、`bACSimFFall/BFall[][]` | 下壓掉料旗標與 map（供 4620 JAM skip 標記） |
| `bACSimDupErr2[][]` | arm2 平行放回 JAM0327 去重 map（arm1 用原生 `bDuplicateErr`） |
| `ACSimErrPart2` | arm2 掉料位置字串（JAM0315） |
| `iACSimSnapFL/BL`、`iACSimKitTask`、`ACSimSupplyTimer` | 4010/4015 供料進度快照與 60s 降級計時（case 800 轉接時刷新 BL 快照並重裝） |

> v4 的 `bACSimSuckFin2[][]`（arm2 平行吸取 map）已隨平行取料廢除而移除。

## 5. 互鎖與 Guard（改動點清單）

1. **交叉 guard**（DoIndexAutoClean 開頭，~L7509，v5 拆為兩子句）：
   - `bACSimActive && (Task∈[2400,2800] 或 Task∈[4015,4750]) && Shuttle1 IsCanMove` → return
     （Arm1 持料期間 S1 必須維持鎖定；Arm1 取料段 [400,800] 由原生 `[400,1900]` 子句罩住）
   - `bACSimActive && Task∈[4300,4750] && Shuttle2 IsCanMove` → return（同壓+放回期間）
   重點：**兩子句都不蓋 650-670/2650-2670（D43 錯誤流）**，讓原生「解鎖→左移補料→
   回右回鎖」編排不受干擾；4000/4010 也不在範圍（bACSimActive 尚未 set）。
2. **6 處 shuttle window**（`Do_Auto_SHT1/2` case 200/1100/2000）：
   原生 `[400,1900)`/`[2400,3900)` 條件各加 `|| (bACSimActive && Task∈[4300,4750])`。
   **4015 刋意排除在外**：Arm1 取料與 4015 等待期間，S2 需自由左右補料；若 window
   罩住 4015 會把空的 S2 強拉到右側鎖死 → 永遠等不到 pad → 誤觸降級 D。
3. **錯誤處理哲學（v5）**：取料重用原生 case，JAM0312/0313 + D43 補料重試在
   `bACSimActive==true` 下直接套用原生機制（單臂情境與原生完全相同，安全）；
   不再需要 v4 的降級 B。

## 6. Resume / 降級路由（case 100 與 1900 的內容路由）

- case 100（歸零後 resume）依吸嘴內容路由：
  `FTestSuck HAS_CLEAN_IC→650`、`CLEAN_FINISH→1500`、
  `BTestSuck HAS_CLEAN_IC→2650`、`CLEAN_FINISH→3500`（後兩分支為 ACSim 新增，
  **必須排在既有 SelectArm==2→2200 分支之前**）。
- case 1900（原生 arm1 放回結束）新增內容路由：`BTestSuck` 尚有 pad → 2650/3500 接手 arm2
  （降級 C 後 arm1 走原生放回、arm2 由此接手；降級 D 時 BTestSuck 空 → 原生路由 2200）。
- `InitialSet()`（L742-743）：`iDoIndexAutoCleanTask=1` + `bACSimActive=false`，
  **不清 bACSimDegraded**（降級狀態需維持到整輪結束）。

## 7. Y Middle 保護自檢（降級 C）

`DEBUG_GALIL_CHECK_Y`（MachineType.h）使 `GalilTwoY_Move`（myGALILmotor.cpp）對
Y1/Y2 同時進 Middle 有保護式；NN 模式 `Prod.TestY1_Middle/TestY2_Middle` 帶 ∓7000
偏移（`SetTechDataToProd_Index()`，cinitial.cpp）屬「等號擦邊」通過。
case 4300 進場先自檢：
`Prod.TestY1_Middle > (Tech.iHT9040TestY1_Middle-7000) && Prod.TestY2_Middle < (Tech.iHT9040TestY2_Middle+7000)`
→ 成立即降級 C（Task=650，雙臂 pad 交由原生序列處理）。
`DropPlaceShiftContact` / `bNS7000kit` 變體會改 Middle 值 → 已在入口條件排除。

## 8. 效率與真空限制演進（2026-07-29，兩次機台驗證回饋）

**第一次回饋（v3→v4）**：取放仍序列、僅下壓平行，3 個效率問題：
1. Shuttle1 補滿 pad 時 Arm1 不會先去吸（4010 等雙鎖）
2. 兩臂吸取分兩次
3. 放回也分兩次

v4 將取料/放回全平行化（雙 Z 同下、合併吸取迴圈）。

**第二次回饋（v4→v5）**：兩臂同時開吸（開放式吸氣）造成**負壓壓降→吸力下降**。
v5 對策：
- 取料改回逐臂，但 **Arm1 在 S1 就緒時立刻開吸**（解決問題 1）；
  總時間 = max(供料 S1+取料, 供料 S2) + 另一臂取料，吸取藏入另一座供料窗口。
- Arm1 取完後真空**密封持料**（密封狀態流量極小，不影響 Arm2 開吸）。
- 同時下壓（雙臂皆密封持料）與平行放回（Destroy 吹氣）無真空限制 → 維持。
- 廢除 v4 平行吸取 case 4020~4080 與降級 B；取料錯誤直接由原生 JAM/D43 處理。

## 9. 歷史迭代與審查教訓（勿再犯）

| 版本 | 內容 |
|------|------|
| v1 | guard 蓋到鎖定 case → 死鎖；供料無 timeout → hang |
| v2 | case 1900 解鎖 vs 交叉 guard 自鎖；iContactCount 未歸零 silent failure |
| v3 | 實作 + M1 修正（4600 Safe 高度複掃、4520 移除 100ms delay、D58 顯式排除） |
| v4 | 平行取料/放回、arm2 獨立狀態陣列、降級 B、終態停 3900 |
| v5 | 取料改回逐臂（真空壓降限制）、Arm1 提早開吸、新 case 4015 等鎖 S2、降級 D、廢 4020~4080/降級 B、2800 歸零 bTestSuckUse |
| v5-R3 | 使用者回饋：補料逾時無聲降級不可接受 → 4010/4015 逾時改跳 **WAR0359**（RETRY 續等/SKIP 才降級），AlarmDescription.ini 新增中英說明 |

- `iContactCount` 是 DoIndexAutoClean 的 function-static，原生 case 200/2200 歸零；ACSim 跳過原生 case 必須在 4010/4015/降級 D 歸零。
- 共用陣列 `bSuckFinish`/`bTestSuckUse`/`bDuplicateErr` 為序列設計；v5 逐臂取料後 `bTestSuckUse` 在 2500 會被 Arm2 重初始化而涗掉 Arm1 標記 → 2800 轉接點整陣列歸零，skip 位置改由 `Item==HAS_NULL_CLEAN_IC` 過濾保障（與 skip 標記同時設定，等價）。
- **真空物理限制**：兩臂同時開放式吸氣會壓降；密封持料、同時下壓、Destroy 吹氣皆不受限。任何未來平行化提案都必須先檢視此限制。
- **既有缺陷（另開單，非 ACSim 引入）**：case 2660 結尾誤寫 `bAutoCleanShuttle1HasPickErr=false`（應為 Shuttle2）。

## 10. 降級路徑總表（v5）

| 降級 | 觸發點 | 條件 | 後續 |
|------|--------|------|------|
| A | 4010 | S1 供料 60s 無進展 → WAR0359，操作員按 SKIP | 解鎖早鎖的 S2 → Task=200 原生序列從頭跑（RETRY 則重裝計時續等） |
| C | 4300 | Y middle 保護自檢違反（無警報，自動） | Task=650（Arm1 持料序列下壓；1900 後 BTestSuck 路由 2650 接 Arm2） |
| D | 4015 | S2 供料 60s 無進展 → WAR0359，操作員按 SKIP | Task=900（Arm1 持料序列下壓；1900 後原生路由 2200 接 Arm2 全原生流程）（RETRY 則續等） |

（v4 降級 B：平行吸取錯誤，已隨平行取料廢除；取料錯誤現由原生 JAM0312/0313+D43 在 ACSim 內直接處理，不降級）

## 11. 驗證紀錄

- 2026-07-29 v5-R3（WAR0359）：BCB6 編譯 0 errors（EXE 11:20，30,048,256 bytes），Big5 檢查 PASS。
  Alarm MDB `AlarmList` 尚需補 WAR0359 紀錄（MDB Updater 交付），否則警報主文字空白。
- 2026-07-29 v5：BCB6 編譯 0 errors（EXE 10:21），Big5 檢查 PASS，Agent B 審查 PASS
  （9 場景逐項追蹤：正常流/雙臂 D43/降級 A・C・D/共用陣列/resume/window 收緊）。
- 2026-07-29 v4：BCB6 編譯 0 errors（EXE 09:40），後因真空壓降限制被 v5 取代。
- 發版前提醒：`MachineType.h` L43 `SOFT_SIMULTE` 須註解（release guard）。
