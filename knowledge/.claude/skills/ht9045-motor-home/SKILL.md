---
name: ht9045-motor-home
description: "HT9045 馬達回 Home / 單軸 Home 機制核心知識庫。Use when: 撰寫或除錯任何單軸 home 程式、客戶反應馬達抖動 / Home 30s timeout / Home 卡住、自動 home 子流程 (如 Check Device 進場前自動 home MLoaderY)、看到 HomeFlag 判斷不一致、混淆 Home() / MotorHome() / MotorInitial() 三個函式用途、Home 後速度殘留導致漂移。關鍵字：HomeFlag, MotorHome, MotorInitial, Home(), HomeReset, HomeObject, iMyHomeTask, ResetTime, RESET_TIMES, auto-home, Home All, Home timeout, 馬達抖動, MLoaderY home, Teach Home, fAllMotorHome, S122, WebTeachLeave, 關 Teach 清旗標, Home by Start, 瀏覽器全關停產, V906 移植樹, ProcessSingleMotorHome。"
---

# HT9045 馬達回 Home 機制 SPEC

> 來源：2026-05-05 V899 客戶端 MLoaderY auto-home timeout 除錯（cTrayMapping case 10/15/20 第一版誤用 `Home()` + `HomeFlag==2` 判斷成功，導致馬達抖動 30 秒 timeout）。
>
> 適用：所有透過 `MOT[]` 操作的馬達（mymotor.cpp 統一介面）。

## 1. 三個函式用途分清楚

| 函式 | 層級 | 用途 | 副作用 |
|------|------|------|--------|
| `MOT[i].Home()` | **底層** | 直接呼叫硬體 `Motor->HomeObject()` 觸發一次 home 動作 | **不更新 `HomeFlag`**、不跑狀態機 |
| `MOT[i].MotorHome(bool Flag)` | **中層狀態機** | 完整 home 序列：Reset → Home → 等 sensor → Retry → 設 HomeFlag | 會更新 `HomeFlag`；`Flag=true` 重置 timeout |
| `MOT[i].MotorInitial()` | **重置** | 清狀態：`Task=1`、`HomeFlag=0`、reset `ResetTime=RESET_TIMES*0.1s` | 不啟動 home，只清旗標 |

**重點**：要等 `HomeFlag` 變化的程式碼，**必須呼叫 `MotorHome(false)`**，不能呼叫 `Home()`。

## 2. HomeFlag 三態語意

| 值 | 意義 | 進入條件 |
|----|------|----------|
| `0` | 未開始 / 進行中 | `MotorInitial()` 後初始狀態 |
| **`1`** | **Home 成功** | `MotorHome` 內偵測 `Motor->HomeFlag()` 為 true，或馬達 disable 直接 pass |
| **`2`** | **Home 失敗** | retry 用完（`iHomeRetryCT==0`）/ 30s `ResetTime` 超時 |

**常見誤解**：
- ? `if(HomeFlag==2)` 當作「home 完成」→ 實際是失敗
- ? `if(HomeFlag!=0)` 當作「不在 home 中」→ 1/2 都會通過但語意混亂
- ? 成功判斷：`if(HomeFlag==1)`
- ? 失敗判斷：`if(HomeFlag==2)`
- ? 進行中：`if(HomeFlag==0)`

## 3. 標準單軸自動 Home 子流程範本

適用情境：某入口（如 Check Device、AutoStart 模擬）需要在 motor 沒 home 過時自動補 home，又不想要求使用者手動按 Home All。

```cpp
// 觸發條件：HomeFlag != 1
case ENTRY_TASK:
    if(MOT[mid].HomeFlag != 1)
    {
        // 1. 安全互鎖：關掉所有「會跟此軸干涉的固定機構氣缸」
        //    (台車載具上的氣缸隨軸移動，不需要動)
        Cylinder[C_FIXED_OBSTRUCTION].Off();
        tHomeWait.SetSecAndOn(2);                // 等氣缸退回
        Task = AUTO_HOME_WAIT;
        return false;
    }
    // ... 正常流程

case AUTO_HOME_WAIT:
    if(!tHomeWait.Off())                         // 等 2 秒到
        return false;
    // 2. Sensor sanity check：氣缸真的退回了嗎？
    if(Cylinder[C_FIXED_OBSTRUCTION].OnSensor() &&
      !Cylinder[C_FIXED_OBSTRUCTION].OffSensor())
    {
        asError = "C_FIXED_OBSTRUCTION sensor still ON. 機台不可移動，請檢查氣壓。";
        return false;                            // 中止避免撞機
    }
    MOT[mid].MotorInitial();                     // 3. 重置 home 狀態機
    tHomeWait.SetSecAndOn(30);                   //    外部 timeout 守門員
    Task = AUTO_HOME_RUN;
    return false;

case AUTO_HOME_RUN:
    MOT[mid].MotorHome(false);                   // 4. 必呼叫 MotorHome (非 Home)
    if(MOT[mid].HomeFlag == 1)                   //    成功
    {
        Task = ENTRY_TASK;                       //    回主流程
    }
    else if(MOT[mid].HomeFlag == 2)              // 5. 馬達自己判定失敗 → 立刻報錯
    {
        asError = "Auto-home failed (motor reported HomeFlag=2). Check home sensor / servo alarm.";
    }
    else if(tHomeWait.Off())                     // 6. 外部 timeout 兜底
    {
        asError = "Auto-home external timeout (30s).";
    }
    return false;
```

## 4. 反模式（會抖動 / 會 timeout）

```cpp
// ? 反模式 1：直接呼叫 Home() 並等 HomeFlag 變化
case 20:
    MOT[mid].Home();                  // 每 loop 觸發 HomeReset() → Task 一直被踢回 1
    if(MOT[mid].HomeFlag == 2) ...    // HomeFlag 永遠不會變（Home() 不更新它）
                                      // 馬達剛動就被打斷 → 抖動 → 30s timeout
```

```cpp
// ? 反模式 2：把 HomeFlag==2 當成功
if(MOT[mid].HomeFlag == 2)
{
    // 進入後續流程 → 實際 home 失敗，後續 MotorMove 會用無效座標撞限位
}
```

```cpp
// ? 反模式 3：不呼叫 MotorInitial 直接 MotorHome
// 若上次 home 結果是 HomeFlag=2，狀態機殘留，需要 MotorInitial() 才能重新觸發
```

## 5. 與全機 Home 的差異

| 項目 | uhome.cpp 全機 Home | 單軸 Auto Home (本 SPEC) |
|------|-------------------|------------------------|
| 觸發 | 使用者按 Home All / 開機 | 程式入口檢測未 home |
| 涵蓋範圍 | 所有 `TOTAL_MOTOR` | 單一 motor |
| 序列 | `MotorInitial()` 全部 → 持續呼叫 `MotorHome()` 直到 ret==1 | 同 SPEC §3 |
| 安全保護 | 全機氣缸 reset / safe pos | **只關自己這軸會撞到的固定機構氣缸** |

## 6. 速度狀態還原陷阱（與 Home 直接相關）

`TMyMotor::GetSpeed()` 回傳 **`Motor->ReadSpeed()` 瞬間 RPM**（停下時 0~5），**不是設定值**。

? 錯誤備份還原：
```cpp
int spd = MOT[m].GetSpeed();   // 抓到 0 (馬達停著)
MOT[m].SetSpeed(slow);         // 工作
MOT[m].SetSpeed(spd);          // 把 0 當 100% 還原 → 後續 Home/Move 龜速漂移、撞限位
```

? 正確：直接還原為已知預設值
```cpp
MOT[m].SetSpeed(100);          // 用業務上 known-good 預設值
```

uhome.cpp L3549 / L3559 對 `MLoaderY` 的處理就是直接 `SetSpeed(100)`，這是業界標準做法。

## 7. Teach 畫面為何「都正常」

Teach 介面呼叫的是「使用者按按鈕一次 → MotorHome 跑完整序列」單次模式，不會在外層迴圈中再呼叫 `Home()`，所以不會踩到反模式 1。**客戶反應 Teach OK 但程式內 Home 失敗 → 99% 是程式呼叫方式錯誤，不是硬體問題**，先檢查程式再叫客戶查 servo。

## 8. 診斷 log 必備欄位

馬達 home 異常的 log 必須包含：
```
Pos    : MOT[m].ReadPos()                   // 看馬達有沒有移動
Spd    : MOT[m].GetSpeed()                  // 確認速度設定（停下時不準但仍可看趨勢）
Home   : (int)MOT[m].HomeFlag               // 0/1/2 三態
Sensor : Cylinder[interlock].OnSensor() / OffSensor()  // 互鎖氣缸實際狀態
```

範例見 [cTrayMapping.cpp `DumpLaserScanState()`](../../HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/cTrayMapping.cpp)。

## 9. 已知案例索引

| 日期 | 案例 | 失誤點 | 修正 |
|------|------|--------|------|
| 2026-05-05 | 欣銓 V899 MLoaderY auto-home timeout | 用 `Home()` + 等 `HomeFlag==2` | 改 `MotorHome(false)` + 等 `HomeFlag==1`；新增 `==2` 立即報錯 |
| 2026-05-05 | RestoreLoaderYSpeedRaw 用 GetSpeed 備份 | 把瞬間 RPM 當設定值 | hotfix：永遠 `SetSpeed(100)` |

## 10. 實作位置參考

- `Motor/mymotor.cpp` L1602-1742：`MotorInitial / Home / MotorHome` 完整實作
- `cTrayMapping.cpp` case 10/15/20：標準自動 Home 子流程範例
- `uhome.cpp` ProcessMotorHome：全機 Home 主控（透過 `HomeClass` push_back + `THomeFlag`）

---

<!-- Steven 20260918: 下方 §11 自 .agents\skills\ht9045-motor-home 併入。
     原因：.agents\skills 退場，而這一節是它唯一真正獨有的內容
     （.claude 與 .github 全樹 grep THomeClass / InitialHomeClass / THomeOrder 皆查無）。
     完整稽核：D:\docs\ops\weekly\2026\09\20260918\20260918_Steven_skills_duplication_audit.md -->

## 11. HomeClass：整機回 Home 的資料驅動註冊表

> 來源：uhome.h `vector<THomeClass *> HomeClass`（fHome 表單成員）。
> 補足本 skill 第 1~10 節的「單軸 MotorHome/HomeFlag 邏輯」之上，**整台機台怎麼編排回 Home**這一層。

### 11.1 一筆 = 一顆馬達的 Home 設定

`THomeClass(int MotNo, int Order, bool bView=true)`（uhome.cpp 建構式）三參數：

| 成員 | 來源參數 | 意義 |
|------|----------|------|
| `index` | MotNo | 對應哪顆馬達（= MOT[] 索引） |
| `HomeOrder` / `THomeOrder` | Order | 回 Home 的階段：FIRST_HOME=100 / SECOND_HOME=200 / THREE_HOME=300（MachineType.h） |
| `Visible` | bView | 這台機台有沒有這顆軸（同時決定 UI 顯示 + 是否參與 Home） |
| `THomeFlag` | — | 執行期工作旗標：1=待回 / 0=完成或跳過（與 MOT[].HomeFlag 分開！） |

建構式內若 `Visible==true` 才 new 出 labName/edPos/ledHome 三個 VCL 元件掛到 fHome->Panel1；否則掛到 palUnUse（隱藏池）。

### 11.2 用 config flag 做機型自動適配

`TfHome::InitialHomeClass()`（uhome.cpp，static bInitial 保證只建一次）以一連串 `push_back` 把所有軸註冊進來，**第三參數塞設定旗標**，讓同一份程式碼適配各種機型，不必為每台機台寫不同 Home 流程。常見旗標：

- `InOutArmPickerUseMotor==eptUseMot` — Picker Z 軸群（MInArmZB~ZH / MOutArmZB~ZH）
- `USE_PICKER_COUNT==ep16Picker` — 16-site 額外 Z 軸（MInArmZAe/Af/Ag/Ah…）
- `USE_ROTATE_KIT && iRotate_Type==…` — 旋轉站各馬達
- `LOAD_Z_USE_MOTOR[]` / `LOAD_Y_USE_MOTOR[]` / `USE_LdUldCassetteMode` — Loader/Empty/Color/Auto Z、Y 軸（見 ht9045-load-y-use-motor）
- `USE_MR_SYSTEM` / `AUTO3_IS_MAGAZINE` / `USE_OUT_SORT_ARM==iOutSortX40mm` 等機型旗標

### 11.3 分階段排序 = 防撞（核心精神）

`ProcessMotorHome` 把 `MotorTask`（cmydef.h extern int）依序設成 `FIRST_HOME → SECOND_HOME → THREE_HOME`，每階段只回 `THomeOrder==MotorTask` 的軸：

```cpp
MotorTask = FIRST_HOME;                       // 第一階段
for(int i=0; i<TOTAL_MOTOR; i++)
    if(fHome->HomeClass[i]->THomeFlag &&
       fHome->HomeClass[i]->THomeOrder==MotorTask)   // 只回本階段、且尚未完成的軸
        ... 啟動 home ...
```

- FIRST_HOME：Z 軸（吸嘴 / Index 下壓）先抬起回原點
- SECOND_HOME：XY / Pitch / Shuttle 等才動
- THREE_HOME：Tray、Scale、Magazine 最後
- 同階段全部 HomeFlag==1 後才推進下一階段 → **Z 先縮回，XY 才平移，避免撞 Socket / 撞料盤**。

### 11.4 平行陣列不變式（最易踩雷）

程式大量用 `HomeClass[MTestZ1]`、`HomeClass[i]`（同一個 i 又拿去 `MOT[i]`）索引，這**要求 `HomeClass[i]->index == i`**。成立前提：

> `InitialHomeClass()` 的 push_back 順序，必須與 cmydef.cpp 的馬達常數宣告順序（`MInArmX=0, MInArmY=1, MInArmPitch=2, …`）**逐一對齊**。

收尾補齊迴圈確保長度 = TOTAL_MOTOR、任何 id 可安全索引：
```cpp
for(int i=HomeClass.size(); i<TOTAL_MOTOR; i++)        // 沒明列到的馬達
    HomeClass.push_back(new THomeClass(i, THREE_HOME, false));   // 補成隱藏、最後階段
```

**維護守則**：在 `InitialHomeClass()` 新增/刪除/搬動任何 `push_back`，都可能破壞「位置 = 馬達 index」的對齊，使 `HomeClass[MXxx]` 指到錯的軸。新增軸時務必確認它在序列中的位置正好等於該馬達常數值；多半應加在尾段的補齊迴圈「之前」、且對齊新馬達 const 的順序。

### 11.5 生命週期

- 建立：`InitialHomeClass()`（開機初始化，static guard 只跑一次）
- 重置：ProcessMotorHome 開始時整批 `THomeFlag=1`；軸 disable 或 Scale 軸則設 0 跳過（如 `HomeClass[MInArmXScale]->THomeFlag=0`）
- 釋放：`TfHome::FormDestroy` 逐一 `delete` 後 `vec_clr(HomeClass)`（含 try/catch）

### 11.6 與本 skill 前段的關係

| 層級 | 負責 | 章節 |
|------|------|------|
| 整機編排 | HomeClass 註冊表 + 階段排序 + 防撞 | 本節 |
| 單軸動作 | MotorHome(false) / HomeFlag 0/1/2 判斷 | 第 1~4 節 |
| 工作副本 | THomeFlag（編排層）≠ MOT[].HomeFlag（單軸層），勿混用 | — |

---

## 12. V906 移植樹（C++）現況

移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 怎麼照 golden 清 `fAllMotorHome`（S122：網頁關／開 Teach、Motor Test，`0b166feb`）、J1（Teach 頁載入只在沒運轉時清，`8b5a91b5`）、
「所有瀏覽器關掉超過 15 秒 MainProc 會暫停」的已知風險、單軸回原點本體 `ProcessSingleMotorHome` 已照翻（`23c264b9`），見 [references/v906-port-status.md](references/v906-port-status.md)。
