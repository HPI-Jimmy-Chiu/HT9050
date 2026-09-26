# golden_elec —— golden 外部元件原始碼的 UTF-8 鏡像（REQUESTS Q12）

> NB2 輔助 session，20260925 18:5x。新電腦那台沒有 `D:\HT9045\elec\Component`（0922 換機時沒有搬），所以由 NB2 提供。

| 鏡像 | 原檔（Big5） | 位元組 | SHA-256 前 16 碼 | 行數 |
|---|---|---|---|---|
| `HAlarm.cpp.txt` | `D:\HT9045\elec\Component\HAlarm.cpp`（2023-12-09） | 9,320 | `45e8db2dbc01ee6b` | 307 |
| `halarm.h.txt` | `D:\HT9045\elec\Component\halarm.h`（2021-06-29） | 1,428 | `a968d76a907c1d06` | 39 |

* 轉碼用 `cp950` 的 **strict** 模式：解不了的位元組會直接報錯，所以 0 個位元組遺失，U+FFFD 也是 0 個。只把 CRLF 換成 LF。**行號和原檔完全相同**，檔案裡沒有插入任何檔頭，引用時可以直接寫「HAlarm.cpp:110」。
* 副檔名加 `.txt`：避免被當成移植樹的 `.cpp`／`.h` 編進去。NB2 也不產 `.cpp`／`.h`。
* golden 906 的 `HT9045.bpr:194` 把 `-ID:\HT9045\elec\Component` 放在 include path，所以這就是 golden 編的那一份。
  * 只驗了宣告，沒有驗 `DCLUSR60` 套件裡的二進位是不是同一版。

---

## 1. `HAlarm::Set(iCode)`（`HAlarm.cpp:110-137`）

* **去重的單位是「同一個 HAlarm 物件的 `ErrNoList`，以整數警報碼比對」**：`:112` `if(GetStat(iCode)) return;`。`GetStat` 在 `:214-230`，逐一比對 `ErrNoList`。
  * 請求裡寫的「`SetStat`」**不存在**。整份檔只有 `GetStat`（查詢）。
* 碼不在清單裡時，會做三件事：
  * `:122` 把碼加進本物件的 `ErrNoList`。
  * `:124-127` 把 `ERR_MSG{ObjPtr=Parent, iErrCode}` 加到**全域的 `ShowAlarmList`**（FIFO，`:22`）。
  * `:136` `UpdateSystemNG()`：只要任何一個 HAlarm 物件的清單不是空的，就設 `SystemNG=true`（`:246-260`）。
* **`Parent`** 是建構子傳進來的擁有者（`:33`、`:50`）。
  * golden 氣缸用的是**全域 `Alarm`**：`golden:mycylin.cpp:83` `extern HAlarm *Alarm;`，`:84-92` 的 `SetAlarm`／`ClearAlarm` 直接轉呼叫它。
  * 這個全域物件建立在 `golden:main.cpp:22472` `Alarm = new HAlarm(this);`，定義在 `:202`。⇒ **Parent＝fMain**。
  * elec 元件 `HMotor.cpp:56`、`HCylinder.cpp:29`、`HSucker.cpp:40` 各自也 `new HAlarm(this)`（Parent＝那個元件）。但 golden 樹裡**沒有人用 `HCylinder`／`HSucker`**（grep 0 檔）。golden 自己的 `TMyCylinder`（`golden:mycylin.h:10`）不繼承它們。
* **實際效果**：所有氣缸的警報都進同一個全域物件。
  * 碼是 `31000+i`（`golden:cinitial.cpp:4537-4538`），**開和關用同一個碼**。
  * ⇒ 去重等於「**同一支氣缸**在被取出並清掉之前，只排隊一次」，不論是開還是關逾時。
* 已經被取出顯示（`PopUpAlarm`）、但還沒被清除的碼，**再 Set 也會被擋**，因為 `ErrNoList` 裡還有它。

## 2. `Clear(iCode)`／`Clear()`／`ClearAllAlarm()`

| 函式 | 行 | 做什麼 |
|---|---|---|
| `Clear(iCode)` | `:142-199` | 碼不在本物件清單裡 ⇒ 什麼都不做，回 false（`:159-164`）。在的話，會做 4 件事：① 從 `ErrNoList` 刪掉（`:149-158`）；② 如果它還在 `ShowAlarmList` 裡沒被取走，從尾端找第一筆「同 Parent 同碼」的刪掉，只刪一筆（`:167-177`）；③ 加一筆到 `ClearAlarmList`（`:180-186`）；④ `UpdateSystemNG()`（`:196`）。回 true |
| `Clear()` | `:203-210` | 對本物件清單裡的每個碼都呼叫一次 `Clear(code)`，所以每個碼也各進一次 `ClearAlarmList` |
| `ClearAllAlarm()`（自由函式） | `:234-242` | 對**每一個** HAlarm 物件呼叫 `Clear()`，最後 `SystemNG=false` |
| `PopUpAlarm` | `:264-283` | 從 `ShowAlarmList` 取第 0 筆（FIFO），連同 Parent 和碼一起交出 |
| `PopUpClrAlarm` | `:287-306` | 從 `ClearAlarmList` 取第 0 筆。⚠ **golden 906 全樹 0 個呼叫者**（grep）⇒ `ClearAlarmList` 只增不減，golden 自己的洩漏。移植時**不需要**這條佇列 |

* 執行緒：`HALBusy` 忙等，加上 `HMutex`（`:117-119`）。`GetStat` 只拿 mutex，不看 `HALBusy`。`Set` 在鎖外先 `GetStat`，有 TOCTOU 空隙，但無害。移植樹的 facade 是單執行緒，`canary_support.cpp` 已經說明不照搬這部分。
* golden 小瑕疵：`~HAlarm`（`:59-68`、`:71-80`）刪完一筆就把 `iP=0`，接著 `iP++`，結果會略過第 0 筆。這只在解構時發生，和本題無關。

## 3. 氣缸逾時 → 佇列 → 畫面：golden 的完整鏈

1. **產生**：`TMyCylinder::Push`／`Pop` 在 `OnTryTask>=2`（或 `OffTryTask>=2`）時呼叫 `SetAlarm(On/OffAlarmCode)`，接著把 `TryTask` 歸 0。位置在 `golden:mycylin.cpp:343`、`:400`、`:492`、`:549`。
2. **取出**：`MainProc` 呼叫 `DoSystem()`（`golden:csystem.cpp:16911`）。`DoSystem` 在 `if(bDoProcess)` 時呼叫 `ProcessAlarm()`（`golden:csystem.cpp:4318-4320`），也就是**每兩拍一次**。
3. **處理**：`golden:ckernel.cpp:2501-2527` `ProcessAlarm`，每取出一筆：
   * `SoftStop=false; SoftStart=false;` ⇒ **機台停止運轉**。
   * `ScanSystemSensor();`
   * 碼是 `ALM_MOTOR_MOVE`（55555）時，當成馬達錯誤：`GetMotorAlarmCode(Comp)`，並設 `fAllMotorHome=false`。
   * **其他碼（氣缸）**：`CylinderIndexToJamCode(iCode,&Pos)`，在 `golden:note.cpp:4156`，以 `iCode-31000` 算出 JAM 碼，並指定要標示的馬達 `Pos`。接著 `ShowErrorMessage(sRef, K_RETRY, Pos)`，**跳出附 RETRY 的錯誤框**。
4. **收尾**：迴圈結束後 `ClearAllAlarm()`，清掉所有碼 ⇒ 同一支氣缸下次逾時會再排隊。
* ⇒ **會停機**：`SoftStart=false`，並且跳出 JAM 加 RETRY 的框。
* **「每兩次重試就跳一次框」本來就是 golden 的行為**：每次 `TryTask` 到 2 就 Set 一次，下一次 `ProcessAlarm` 取出、停機、跳框，然後清除。
  * 去重只擋「同一段取出間隔內重複 Set」這一種情況，擋不住每一輪各跳一次。

## 4. 移植樹怎麼接（檔案:行＋改法＋驗法；給新電腦）

**現況**：
* `mycylin.cpp:125`（`SetAlarm`）、`:130`（`ClearAlarm`）是空的樁。
* 佇列接縫在 `canary_support.cpp:564-576`（`W906_PopUpAlarm_Push`，定義在 `:570`，**不去重**）。
* `PopUpAlarm` 在 `:605`，`ClearAllAlarm` 在 `:637`（只清 FIFO 和 `SystemNG`）。
* `ProcessAlarm` 在 `ckernel.cpp:3996`，和 golden 逐行相同，由 `csystem.cpp:16267` 在 `DoSystem` 裡呼叫。

**改法**（照 golden 的「單一全域 HAlarm」語意）：
1. 在 `canary_support.cpp` 加一份檔案範圍的 `std::vector<int> W906_GlobalAlarmCodes`，這就是 golden 全域 `Alarm` 的 `ErrNoList`。另外加兩個對外函式：
   * `W906_GlobalAlarm_Set(int code)`：
     * 如果 code 已經在清單裡，直接 return（golden `:112`）。
     * 否則加進清單，呼叫 `W906_PopUpAlarm_Push(nullptr, code)`，設 `SystemNG=true`（golden `:122-136`）。
     * ObjPtr 用 `nullptr` 就好：golden 的 Parent 是 fMain，不是馬達；`ProcessAlarm` 只有在 `ALM_MOTOR_MOVE` 時才會用到 `Comp`。
   * `W906_GlobalAlarm_Clear(int code)`：
     * 不在清單裡 ⇒ return false（golden `:159-164`）。
     * 在的話：從清單移除；從 FIFO **尾端**找第一筆 `ObjPtr==nullptr && iErrCode==code` 的刪掉，只刪一筆（golden `:167-177`）。
     * **不做** ClearAlarmList（golden 沒有人取）。
     * 接著重算 `SystemNG`。golden 的 `UpdateSystemNG` 會看所有 HAlarm 物件；移植樹的馬達路徑沒有維護清單，建議以 `!W906_GlobalAlarmCodes.empty() || W906_Alarm_QueueDepth()>0` 近似，並在註解寫明。
2. `mycylin.cpp:125/:130` 分別改成呼叫這兩個函式。註解標 `AI(W906-W3-CYLIN)`，引用 `golden_elec/HAlarm.cpp.txt:110-137`／`:142-199`。
3. `canary_support.cpp` 的 `ClearAllAlarm()` **另外把 `W906_GlobalAlarmCodes` 清空**（golden `:234-241`：每個物件 `Clear()`）。不清的話，同一支氣缸之後永遠不會再報。

**驗法**（ctest，不用硬體）：
* `SetAlarm(31005)` 連呼兩次 ⇒ `W906_Alarm_QueueDepth()==1`。
* `ProcessAlarm()` 之後 ⇒ 佇列 0、`SystemNG==false`，而且再 `SetAlarm(31005)` 會重新排隊（深度 1）。
* `SetAlarm(31005)` 後、`ProcessAlarm` 前先 `ClearAlarm(31005)` ⇒ 佇列 0。
* `ClearAlarm(31006)`（沒 Set 過）⇒ 沒有任何效果。
* 同一個測試裡加一筆 `W906_PopUpAlarm_Push(motor, ALM_MOTOR_MOVE)`，確認 `ClearAlarm` 不會刪到它（ObjPtr 不同）。

⚠ **上線面**：接上之後，氣缸逾時在真機組態會**停機並跳 JAM 框**，這正是 golden 的行為，也是 W3 稽核第 ⑺ 項要的。
* 在 V906 目前的 500 ms 節拍下（R35），氣缸狀態機本身每步就要 1.5～5.6 秒。
* `OnAlarmTime` 是以呼叫次數累計，還是以時間計，會決定「比 golden 更早或更晚逾時」。建議上機前先對一支氣缸量一次。
