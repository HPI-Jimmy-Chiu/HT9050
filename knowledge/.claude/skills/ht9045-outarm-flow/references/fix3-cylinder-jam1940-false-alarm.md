# Fix3 滿盤氣缸 JAM1940 / JAM1941 誤報 —— `Fix3CylinderDelay` 的守門旗標在恢復瞬間被搶先清掉

> 對應程式：`aoutarm9045.cpp UseFix3Cylinder()` case 30 / 31 / 45 / 50 / 51 / 65、
> `mymessbox.cpp` `TMyMessageBox::FormShow`、`aTester_Front.cpp` / `aTester_Rear.cpp` case 2092、
> `mycylin.cpp TMyCylinder::Push()/Pop()`。
> 首次定案：2026-09-08（JSCC ILD502，V3.29.908.16）。

---

## 1. 一秒判定

**`JAM1940` / `JAM1941` 緊跟在「另一個警報被排除後的 1~2 秒內」出現** → 幾乎確定是誤報，氣缸沒壞。

比對三件事就夠：

| 檢查 | 誤報的樣子 |
|---|---|
| EventLog 前一筆警報 | 是別的 alarm（JAM0302 / WAR0158…），中間隔 **30~70 秒**（操作員處理時間） |
| JAM1940 與前一行 `InArm ServoOnArrive` 的時間差 | **16 ms ~ 200 ms**（＝恢復後第一圈就報） |
| `Task_ListWithTime2.csv` 的 `Fix3CanFullTask` | 全天 `60`（完成）數百次、`45`/`65`（報警）只有 1~2 次 |

真的氣缸故障會是：`Fix3CanFullTask` 反覆 `30↔31` 或 `50↔51` 撞 `iCount>=3`、EventLog 前面沒有別的 alarm、且 `45` 連續出現。

---

## 2. 案例：JSCC ILD502，2026-09-08，當天 7 筆全是誤報

| JAM1940/1941 | 前一個警報 | 間隔 |
|---|---|---|
| 01:52:21.906 (1941) | — | — |
| 05:19:22.234 | 05:18:29 JAM0302 Ae | 53 s |
| 05:22:22.218 | 05:21:33 JAM0302 Ae | 49 s |
| 05:48:43.171 | 05:47:36 WAR0158 | 67 s |
| 06:36:21.843 | 06:35:46 JAM0302 Ac | 36 s |
| 07:48:09.156 | 07:44:07 JAM0302 Bf | — |
| **16:00:54.359** | **15:59:59 JAM0302 Ae** | **55 s** |

16:00:54 那筆在 `Task_ListWithTime2.csv` 有完整軌跡，是唯一落在 500 筆 ring 內的：

```
15:59:56.843  Fix3CanFullTask=20    ← 滿盤流程啟動, 等兩支 Shuttle 到左邊
15:59:58.875  Fix3CanFullTask=30    ← 開始推氣缸, Fix3CylinderDelay.SetSecAndOn(10)
15:59:59.343  ★ JAM0302 modal 彈出 → MainProc 凍住 52 秒
              (氣缸實際只被給了 0.468 秒)
16:00:51.656  RETRY
16:00:52.062  START → modal 解除
16:00:54.343  ★ 恢復後第一輪 MainProc（同一毫秒內，依派工順序）:
   L11172  209,  TestYRearTask
   L11173  2091, TestYRearTask
   L11174  2092, TestYRearTask   ← aTester_Rear.cpp:7368  bHangTimePause=false
   L11175  210,  TestYRearTask
   L11176  45,   Fix3CanFullTask ← 讀到 false → Task=45
16:00:54.359  JAM1940
```

全天 `Fix3CanFullTask` 值分佈：`60` ×206（正常完成）、`45` ×1、`65` ×0 → **氣缸本體正常**。

---

## 3. 機制：補丁選對了旗標，但輸在 dispatch 順序

### 相關碼

```cpp
// aoutarm9045.cpp  UseFix3Cylinder()  case 30（case 50 對稱）
case 30:
    if(Cylinder[C_FixTray_FullPlace].Push() || bByPass)
    {
        Fix3CylinderDelay.SetMSAndOn(300);
        Task=31;
    }
    else if(Fix3CylinderDelay.Off())                                // Steven 20230517 : 10 秒沒到位就 alarm
    {
        if(bHangTimePause==true)  Fix3CylinderDelay.SetSecAndOn(10); // Rogeryang 20251231 : 補丁
        else                      Task=45;                           // → JAM1940
        break;
    }
```

```cpp
// mymessbox.cpp:303-311  TMyMessageBox::FormShow —— 彈警報時
if(!iUnLoaderCount)
{
    SystemStart=false;  SoftStart=false;
    if(SystemInitialOK==true) StopAllMotor();
    bSupplyNewICTrayPause=true;
    bHangTimePause=true;            // ★ modal 開啟確實會設 true → 補丁本來會生效
}
```

```cpp
// aTester_Front.cpp:7111-7112  /  aTester_Rear.cpp:7367-7368   case 2092
HangTime.SetSecAndOn(Prod.iHangupMaxTime);
bHangTimePause=false;               // ★ Index 每跑一輪 idle 迴圈就清掉
```

### 為什麼失效

`bHangTimePause` 在 modal 開啟時確實被設成 true，**但 modal 一解除，Index 的 idle 迴圈（`209→2091→2092→210`）在 MainProc 裡排在 OutArm/Fix3 之前**，case 2092 在**同一毫秒**把它清成 false。Fix3 下一個被派工時讀到的已經是 false → 落進 `Task=45`。

⇒ **補丁從來沒有機會生效**，不是旗標選錯，是守門值的生命週期比被守的那段流程短。

### `bHangTimePause` 的正確語意（別再誤解）

**「hang-up 看門狗暫停中」＝ 機台現在停著有正當理由，不要算進 hang 時間。**
不是「InArm 剛放完料」。全專案 ~150 個設 true 的點分兩類：

| 類別 | 例子 |
|---|---|
| 等一個合理的東西 | `CheckHeaterOK()==false`（`atester.cpp` 110/210/310、`aTester_Front/Rear` case 1/210）、`bD37EnableManualProcess` 等人按手動 Start、`iTesterDucking>0`、Precisor、溫度過低 |
| 剛完成一個動作段落 | InArm case 100 / 500 / 1100 / 1550 / 2000 / 15000 |

清除點只有 4 個有效位置，核心是 Front/Rear 的 case 2092（與 `HangTime.SetSecAndOn` 成對）。
主要讀取者：`aTester_Front.cpp:7156` case 210、`atester_32Site.cpp:735/2018` 的 hang 判定、`ainarm9045.cpp:3604/3619`。

**⚠ 它的生命週期由 Index 迴圈主宰（每輪清一次），所以拿它守「秒級的機構動作計時器」本質上就不可靠。**

---

## 4. 修法（都只動 `aoutarm9045.cpp`，不碰任何共用碼）

> ⚠ **`TMyCylinder::Push()` / `Pop()` 不需要動，也不要動。** 它們內部（`mycylin.cpp:383` / `532`）
> **已經正確用 `bHandlerPause` 守自己的 `TOn`**，是這裡的參考實作，不是問題所在。
> 問題只在外層 `UseFix3Cylinder()` 自己那顆 `Fix3CylinderDelay`。

### 方案 B（建議）—— 自足式 gap-guard，不依賴任何外部旗標

比照 `acarry.cpp DetectOutKitDrainStuck()` 的 `hStuckGap` 手法（已上線驗證）：函式每次被呼叫時，
若距上次被呼叫超過 N 秒，代表中間被 modal / Pause 凍過，就把 `Fix3CylinderDelay` 重新上膛。

```cpp
// UseFix3Cylinder() 進入 switch 之前
static TQPF_Timer hFix3Gap;                                                     //AI : 距上次進入本函式的間隔
bool bFix3Frozen=hFix3Gap.Off();                                                //true = 中間被凍住過(modal/Pause)
hFix3Gap.SetSecAndOn(5);
if(bFix3Frozen)
    Fix3CylinderDelay.SetSecAndOn(10);                                          //AI : 凍結期間不算, 重新給氣缸 10 秒
```
然後 case 30 / 50 的 `if(bHangTimePause==true)` 可以保留（多一層保險），也可以拿掉。

- **優點**：完全自足、不受 dispatch 順序影響、不依賴 `bHangTimePause` / `bHandlerPause` 任何一個的生命週期。
- **風險**：極低。唯一副作用是「真的卡住時，第一次報警會晚 ≤5 秒」。
- **注意**：`hFix3Gap` 要放在**函式最前面**（`FIX3_FULL_PLACE` 分支判斷之前），否則非 Cylinder 機型不會刷新它。

### 方案 A（最小改動）—— 補讀 `bHandlerPause`

```cpp
if(bHangTimePause==true || bHandlerPause==true)  Fix3CylinderDelay.SetSecAndOn(10);
else                                             Task=45;
```
四處：case 30（L983）、case 31（L1002）、case 50（L1036）、case 51（L1057）。

- **優點**：一行改動；`bHandlerPause` 是全專案既有的「被暫停過」旗標（40+ 讀取點，`asendic_*` / `acatchtray` / `mycylin` / `atester` 都在用），**只是多讀一個既有全域，不動任何共用碼**；同一支 `aoutarm9045.cpp` 的 case 30/40（L1238/L1257，JimmyChiu 20230808）已有先例。
- **⚠ 待驗證的風險**：`bHandlerPause` 在 `csystem.cpp:19531` 要 `iHandlerStartCount>3` 才清 false，而 `iHandlerStartCount++` 在 L19534 每個 MainProc pass 加一次 → 恢復後大約只維持 **3~4 個 pass（數十毫秒）**。**這個窗口是否穩定涵蓋到 Fix3 被派工的那一圈，需要上機量測**。若窗口不夠，方案 A 會變成「有時擋得住有時擋不住」。

### 方案 C（保守，不改判斷邏輯）—— 只延長門檻

把 `Fix3CylinderDelay.SetSecAndOn(10)` 的 10 秒改大（例如 60 秒）。
- **優點**：改動最小、零邏輯風險。
- **缺點**：治標。操作員處理 alarm 超過 60 秒（本案有 67 秒那次）照樣誤報；且真故障時要等 60 秒才報。

**建議順序：B > A > C。** B 是唯一能保證正確的，且與既有已驗證手法一致。

> ✅ **2026-09-08 已採方案 B 落地**（`HT9011UC_Code_V3.33.908.18_20260907_1108_NB_AI`）。實作與原稿略有調整（**更保守**）：
> - `UseFix3Cylinder()` L887-889 加 `static TQPF_Timer hFix3Gap;` / `bool bFix3Frozen=hFix3Gap.Off();` / `hFix3Gap.SetSecAndOn(5);`（放在 `int &Task=` 之後、`AUTO_EMPTY_COLOR` 早退之前）。
> - **不在函式頂端無條件重設 `Fix3CylinderDelay`** —— 因為 case 31/51 把同一顆 timer 當 **300 ms 安定延遲** 用，頂端重設會塞進 10 秒停頓。改為把 `bFix3Frozen` 帶到 8 個既有的 `bHangTimePause` 判斷點：4 個 `if(bHangTimePause==true)` → `|| bFix3Frozen`（L986/1039/1158/1193，重設 delay）、4 個 `if(bHangTimePause==false)` → `&& bFix3Frozen==false`（L1005/1060/1148/1183，凍結期間不累加 `iCount`）。
> - 同時涵蓋 `Fix3K_UseCylinder`（JSCC）與 `Fix3K_UseCylinder46LA`（HT9046LA）兩個分支——同一缺陷、同一改法。
> - 保留原有 `bHangTimePause` 判斷（雙保險），**未動 `TMyCylinder::Push()/Pop()`**。`bcc32 -c` 0 error。**待上機驗證。**

---

## 5. 這是一個家族缺陷：`TQPF_Timer` 缺 gap-guard

`TQPF_Timer` 走**牆上時鐘**。alarm modal / Pause 期間 MainProc 被凍住、該狀態機不會被呼叫，
但計時器照走 → **解除凍結的第一圈就到期 → 誤報**。

| 位置 | 有無 gap-guard | 說明 |
|---|---|---|
| `TMyCylinder::Push()/Pop()`（`mycylin.cpp:383/532`）| ✅ 用 `bHandlerPause` | 參考實作 |
| `DetectOutKitDrainStuck()`（`acarry.cpp`）| ✅ `hStuckGap` 5 秒 | 參考實作 |
| `DoCleanOutFinishCheck()`（`csystem.cpp`）| ✅ `hCleanOutWDGap` | 參考實作 |
| `ainarm_SearchPickPlate.cpp` case 150 | ✅ `dtWait150LastTick` | 參考實作 |
| **`UseFix3Cylinder()` 的 `Fix3CylinderDelay`** | ❌ **本案** | 只有一個生命週期太短的 `bHangTimePause` |

**新增任何「N 秒沒到位就報警」的計時器時，一律先問：這段期間會不會被 modal 凍住？** 會的話就要 gap-guard。

---

## 6. 與 Pattern #25 的關係

同一份 StateRecord、同一天，但**兩件獨立的事**：

| | Pattern #25（綠燈不運行） | 本文（JAM1940 誤報） |
|---|---|---|
| 性質 | 五方循環死結，零 alarm | 誤報 alarm，機台沒壞 |
| 根因 | `case 306` D43 無出口 + Home 清不掉 In Kit 殘料 | `Fix3CylinderDelay` 守門旗標生命週期太短 |
| 交集 | **都被同一批 JAM0302（Arm2 Ae/Ac/Bf，當天 42 次）觸發** | 同左 |

⇒ 兩者都指向同一個現場前提：**Arm2 的 Ae / Ac / Bf 取料異常要修**。

見 [`ht9045-staterecord-analysis/references/deadlock-patterns.md` Pattern #25](file:///d:/HT9045/.github/skills/ht9045-staterecord-analysis/references/deadlock-patterns.md)。

---

## 7. 現場處置（未改版前）

- 看到 `JAM1940` / `JAM1941` **緊接在別的 alarm 之後**：按 RETRY 即可，**不需要檢查氣缸或 sensor**。
  case 45 會 `Task=30` 重來一次（`iCount` 已歸零），氣缸拿到完整的 10 秒就會正常到位。
- 只有在「EventLog 前面沒有別的 alarm」且「連續出現」時，才需要查 `C_FixTray_FullPlace` 氣缸與到位 sensor。
