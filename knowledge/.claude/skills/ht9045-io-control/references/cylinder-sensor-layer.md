# 氣缸層／感測器層：缺列或 Enable 0 時，各個函式實際回什麼（V906 移植樹）

> 來源：S-23 第 2 部分（B＋C3＋C4，St01，20261004），HT9050 現用 IO_Table／Mot_Table 全面盤點。
> 完整清單與每一項的檔:行：`docs/handoff/S23_FINDINGS_B_C3_C4_20261004.md`（`v906/steven-handoff`）。
> 行號是 GitLab main dfa9a22c 的，會漂；引用請用「檔名＋函式名」。讀表的機制（哪一列決定 Enable、空欄位強制停用、缺列的物件長什麼樣）見
> [table-loading-port.md](table-loading-port.md)；本檔講**呼叫端拿到什麼**，以及因此會卡住或報錯的寫法。

## 1. 氣缸 `Cylinder[]`（`mycylin.cpp`）

| 函式 | 輸出列缺或 Enable 0（`Enable==false`） | 自己那一顆感測列缺或 Enable 0 |
|---|---|---|
| `On()`／`Off()`／`OnSwitch()`／`OffSwitch()` | 什麼都不寫 | — |
| `GetOutBit()` | false | — |
| `OnStatus()`／`OffStatus()`（原始讀值） | 照讀它自己的感測 | **永遠 false** |
| `OnSensor()`／`OffSensor()` | **true** | 改看另一顆的反相；兩顆都沒有 ⇒ true |
| `Push()`／`Pop()` | **完全不看 Enable**：OnSwitch 不動作，接著若對應感測（Push 看 `_On`、Pop 看 `_Off`）是開的就等它 ⇒ SystemStart 時兩次逾時報 31000+i，否則永遠回 false | 感測沒開 ⇒ 延遲 OnDelayTime（預設 1＝0.1 秒）後回 true |

- `Enable` 只看**輸出列**（Alias＝氣缸名）；`_On`／`_Off` 列只決定 `OnSenEnable`／`OffSenEnable`（`cinitial.cpp` InitCylinder）。
- ⚠ **開機程式會蓋掉表**：InitCylinder 綁完表之後，`AUTO_EMPTY_COLOR==0` 把 Empty／Color 8 顆、`USE_AUTO_RETEST==eartUninstall` 把 `C_Auto_Up[]`、`C_AutoZ_Select[]`
  強制成 false——**表上是 Enable 1 也一樣**。HOME 的復歸清單（SetCylinderResetStateHomeBegin 看 Enable）會跟著變少。
- 三種情況：
  1. **整顆不存在**（輸出、兩顆感測都關）：Push／Pop 0.1 秒後成功、OnSensor／OffSensor 為 true ⇒ 流程「安靜成功」。只要真的沒有這顆就對。
  2. **輸出關、感測開**（HT9050：C_Shuttle1Floodgate、C_OutShuttle1Floodgate）：Push／Pop 等真的感測，永遠不到位 ⇒ 報警或卡住。這是**表錯**。
  3. **用原始讀值當到位判斷**（`OnStatus()`／`OffStatus()`／`GetOutBit()`）：不存在的氣缸永遠「沒到位」⇒ 卡住或走錯分支。
- ⚠ **`OnSensor()`／`OffSensor()` 的 true 對「等到位」是對的，對「錯誤檢查」是錯的**：例 `ainarm9045.cpp` CheckLoaderHasTray 把
  `Cylinder[C_TrayY_Fixer].OffSensor()==true` 當「Fixer 沒退」⇒ C_TrayY_Fixer Enable 0 時每次夾 Loader 都 JAM0903。這種呼叫點要補 golden 的
  `Cylinder[x].Enable &&`，不能靠改氣缸層。
- 建議規則（C3，HT9050 分支，其他機種照 golden）：`Push()`／`Pop()` 在 `Enable==false` 時直接回 true；「輸出關、感測開」開機寫 op log 當表錯；
  原始讀值語意不變，把拿原始讀值或 OnSensor／OffSensor 當錯誤判斷的少數呼叫點逐一補 Enable 判斷。

## 2. 感測器 `Sen[]`（`mysensor.cpp`）

- 缺列或 Enable 0（空 Port／Bit 會在讀表時被強制成 0）⇒ **`IsOn()`＝false，而且 `IsOff()`＝false，`Status()`＝false**（`State=-1`）。
- 同一個實體狀態，**看呼叫端怎麼問**就得到相反的結論：
  - `if(Sen[x].IsOff()) 報警` ⇒ 永遠不報（檢查被安靜關掉）；
  - `if(Sen[x].IsOn())` ⇒「沒有／沒按」；`!IsOn()` ⇒ true；
  - 等 `IsOff()` 的狀態機 ⇒ **永遠等不到**。
  - 例（HT9050）：Loader 上料 DoSupplyNewICTray 用 `IsOff()==false` 判「有盤」，DoLoad 用 `IsOn()` 判「沒盤」⇒ 每盤都 JAM0929／MES0922；
    SnEmptyTrayIsLock1 一處讀成「鎖住」、另一處 case 2140 永遠等。
- ⚠ **golden 開機會強制開啟某些感測**，表上關了也沒用：`InitialSafeDoor` 把 SnSafeDoor1/2/3/6/7/8/9 強制 Enable（golden 的「安全門不能 Disable」）、
  `InitialHeaterDoor` 開 SnHeaterDoor2（ATC Silicon 以外也開 SnHeaterDoor）、`SenBit0..19` 一律開。HT9050 上 2/3/6～9 號門的位址是不存在的
  MotionNet 模組 ⇒ 讀不到、讀成「開著」⇒ START 被擋。
- 所以**不能一刀切**（缺＝開 或 缺＝關 都會在一半的呼叫點出錯，安全感測更是鎖死機台或藏住危險）。建議規則（C4，HT9050 分支）：每一顆感測一個「缺列策略」——
  `neutral`（現況）／`reads-on`／`reads-off`／`alias=<另一顆實際存在的輸入>`／`required`（必須存在：開機 op log 錯誤＋拒絕 START）；
  只在該列缺或 Enable 0 時才用；**要套在 golden 的強制開啟之後**；安全門只強制「表上 Enable 1」的那幾扇。
- HT9050 上的例子：EMG 四顆、SnServo、SnSystemPower 都 Enable 0 ⇒ 軟體看不到緊停，只有 SnMotorPower（Enable 1）抓得到馬達斷電；
  SnLoaderSureTray（Enable 0）對應的實際輸入是 SnLoaderTrayHasTray（Enable 1）；堆疊滿／上限保護（SnEmptyTrayIsFull、SnAutoUpSafedetect…）全停用 ⇒ 永遠「沒滿」「安全」。

## 3. 開關 `SW[]`（`myswitch.cpp`）
- 缺列或 Enable 0 ⇒ `On()`／`Off()` 不動作、`Status()`＝false（例：主畫面 Light 鈕 SwCCDLight 按了沒反應）。

## 4. 馬達 `MOT[]`（給 C1 參考）
- 每個槽都有物件，不會是 NULL：Mot_Table **依 Motorname 欄（"M%02d"＝列舉值）綁定**，不是 Alias；沒有那一列 ⇒ `new TMySMCMotor(-1)`、Enable=false。
  M164 以後是邏輯料盤槽（本來就沒有列）。
- Enable 0 的軸照 golden 走「模擬移動」（位置一步步逼近目標後回完成，沒有硬體動）；不看 Enable 直接呼叫 `Motor->` 的路徑打到離線 SMC 樁，每次回 -1。
- ⚠ 不存在的軸**回報「到位」**（例 MInShuttle2：`Led[iInposLed]=false`）⇒ 手臂把料放到不存在的 In Shuttle 2 上；Index 成對呼叫（MTestY1/Y2/Z2 Enable 0）
  讓唯一真的 MTestZ1 在自動運轉時不壓也不抬。應該回報「這一道不存在」，不是「到位」。

## 5. 查表的小工具
- （St01 的暫存工具，不在 repo）`D:\AI_TempFile\st01-s23\b_c3c4\xref.py`＋`classify.py`：掃出 SHIP 組態下所有 `Cylinder[]`／`Sen[]`／`SW[]`／`MOT[]` 引用，對照現用表、分風險類別；
  `arrays.py` 展開陣列索引（`Sen[SnAutoTrayDetect[i]]` 這類，掃描抓不到）。換機台或換表時重跑即可。
