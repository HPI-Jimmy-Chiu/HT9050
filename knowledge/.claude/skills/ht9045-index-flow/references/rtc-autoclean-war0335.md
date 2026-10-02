# WAR0335「RTC arm 1 error!」— Auto Clean 後 RTC 不回應 @ARM2+

> 案例來源：SCK HT9045L-94（CC_SCK=947、RTC 1.0），2026-01 / 2026-05 兩份 state record。
> 客戶三次升降版驗證：V3.21.751 不發生 → 886.2/904.1 大量發生 → 降回 751 又不發生。

## 目錄

1. [症狀簽章（30 秒辨識）](#1-症狀簽章30-秒辨識)
2. [RTC Index Arm 握手機制](#2-rtc-index-arm-握手機制)
3. [根因：r780 把 Full View 預設全域打開](#3-根因r780-把-full-view-預設全域打開)
4. [發作需要的三個條件](#4-發作需要的三個條件)
5. [為何機台會自行復原（且每 3 次才報警一次）](#5-為何機台會自行復原且每-3-次才報警一次)
6. [修法](#6-修法)
7. [驗證判準](#7-驗證判準)
8. [判讀陷阱與教訓](#8-判讀陷阱與教訓)
9. [程式碼錨點表](#9-程式碼錨點表)

---

## 1. 症狀簽章（30 秒辨識）

Event Log 出現以下時序即可確定是本案，**不必再查硬體/線材**：

```text
... "ONE CYCLE Finished by Auto Clean."
... "Start Full View Check..."
... "REALTIME CCD Start Full View Check"
... "REALTIME CCD Full OK"
... "Start Auto Cleaning..."
... "Auto Clean Finish!"
+6.8 ~ 7.2 秒
... WAR0335 "RTC arm 1 error!"  DoTestYFront      ← 或
... "WAR0335 auto retry com port."  DoTestYFront   ← 無聲版（更早期的徵兆）
```

判斷要點：

- **每一次** `Auto Clean Finish!` 後都會出現上面兩者之一；其他時間 0 次。
- 出現節奏固定為 **retry, retry, alarm** 循環（見 §5）。
- 只看 alarm 次數會嚴重低估：實際發生次數 = alarm 次數 × 3。

## 2. RTC Index Arm 握手機制

生產中每個 index 循環：

1. `DoTestYFront()` case 108/109（Z 上升時）送 `@ARM2+`（`rtArmIndex2`），設 `iWaitIndexArm1` 200ms。
2. case 115 等硬體訊號 `Sen[SnRealTimeCCDIndexArm]`（sensor 207）turn ON 才准下壓。
3. 逾時未 ON → 重送 `@ARM2+`，`iRealCCDSendArmCT++`。
4. `iRealCCDSendArmCT>15`（≈ 16 × 200ms = 3.2 秒）→ 呼叫 `COM2->OpenRTCComPortAgain()` 重開 COM port，並依其回傳值決定是否跳 WAR0335。

RTC 必須處於「檢測模式」（收過 `@START+`）才會驅動該 IO 線。Full View（`@FULLM+` / `@FULLT+`）會讓 RTC 離開該狀態。

## 3. 根因：r780 把 Full View 預設全域打開

SVN revision **等於** build number（repo：`file://backsrv/RD/.../SVN/HT9011UC_Code_V3.20`），可直接 `svn cat -r <build> <file>` 取任一版本。

| Revision | 日期 | `InitialCosFunction()` 內 `bFullTestBeforeAutoClean` |
|---|---|---|
| r751 | 2022-12-08 | `false`（當時位於 `cprod.cpp`，尚無 `CosFunction.cpp`）|
| r777 | 2023-05-22 | `false` |
| **r780** | **2023-05-31** | **`true`** ← `//Sam 20230517 : 全部開啟` |
| r886 / r904 / r908 | 2025-10 起 | `true` |

`FUNC_CC_SCK()` **從未設定**這個旗標 → 繼承全域預設。呼叫順序保證客戶函式會蓋掉全域預設：

```text
CustomerFunctionSelect()            cprod.cpp
  └─ InitialCosFunction()           CosFunction.cpp
       ├─ …  bFullTestBeforeAutoClean = true     ← 全域預設先跑
       └─ 最後一行 DoCustomerFunction()          ← case CC_xxx: FUNC_CC_xxx()
```

於是 r780 之後，SCK 每次 Auto Clean 前都會多做一次 Full View Check，而 **AutoClean 是全程式唯一「做完 Full View 之後不下 `@RELEASE+`/`@END+` 就直接往下跑」的呼叫點**（其他呼叫點如 `DoTestHeadMotor` 的 `iCASE_REAL_CCD4`、`DoTestYFront` case 66 都會緊接 `DoReleaseAndInspEnd()`）。

## 4. 發作需要的三個條件

缺一不可，這是「全域打開卻只有一台客訴」的原因：

| # | 條件 | 查法 |
|---|---|---|
| 1 | 機台真的裝 RTC | `Gerneral.ini [System] REAL_TIME_CCD=1`（逐台選配，預設 false，讀取點 `database.cpp`）|
| 2 | Auto Clean 後的復歸路徑**跳過** RTC 收尾 | `DoTestHeadMotor()` case 1 四選一：走 `bIndexCheckCanTurnOff`(+D71) 或 `bAfterAutoCleanNoIndexCheck`(+D69=1/2) → `Task=1500` **不收尾**；走 else → `DoReleaseAndInspEndByFlag()` **有收尾** |
| 3 | Auto Clean 在**生產中**觸發 | `TestMode.Data` 的 `iAutoClean_Mode` / `iAutoClean_IntervalContact`。Initial Start 觸發的 Auto Clean 位於 MainProc 中 `@START+` 之前，清完會補送 `@START+` → 不發作 |

開啟條件 2 相關旗標的客戶碼（908.12 盤點）：

- `bIndexCheckCanTurnOff`：HONPREC_QC、**SCK**、Greatek、WIN_PAC、Allegro_Philippines、Elmos_Germany
- `bAfterAutoCleanNoIndexCheck`：SJ_Semiconductor、JCET、AMKOR_Korea

> 這 9 個客戶碼若日後加裝 RTC 並在生產中做 Auto Clean，會出現同一症狀。

## 5. 為何機台會自行復原（且每 3 次才報警一次）

**復原機制**：MainProc 的 RTC 區塊（收尾派送 + `@START+`）位於 `if(SystemStart)` **之內**，生產中每個掃描都會跑。`WAR0335` 處理式內 `bSendRealCCDSendStart=true;`（`//Sam 20250220 : 修正一直報警 RTC WAR0335 RTC Arm1 Error! 問題`）就是靠這條把 `@START+` 補出去 —— 所以那行不是修根因，而是「撞了警報之後才啟動的補救」。

**報警節奏**：`OpenRTCComPortAgain()` 每次呼叫 `iOpenRTCComPortAgainCount++`，只有 `>OpenRTCComPortAgainMaxCount(=2)` 才回 true 並歸零。因此固定是

```text
第1次 → 回 false → 只記 "WAR0335 auto retry com port."（無聲，且順手重開 COM port）
第2次 → 回 false → 同上
第3次 → 回 true  → 跳 WAR0335 停機
```

2026-01-29 log 的 7 次事件序列（alarm / retry / retry / alarm / retry / retry / alarm）與此完全吻合。

## 6. 修法

### 方案 B（已採用，20260815 進 908.12）— 補上 Full View 收尾

`AutoClean.cpp` `DoAutoCleanKit()` case 3：

```cpp
        case 3:
            if(fContact->DoFullViewCheck()==true)
            {
                COM2->DoReleaseAndInspEndByFlag();                              //AI(ht9045-index-flow) 20260815 (RogerYang) : Full View後補送@RELEASE+/@END+並重啟RTC檢測,避免Auto Clean後RTC不回應@ARM2+ (WAR0335)
                bFullViewCheckFinish=true;
                Task=5;
            }
            break;
```

**版本差異（會影響有沒有效，務必分辨）**：

| 基底版本 | 應呼叫 | 行為 |
|---|---|---|
| ≥ 908.x | `DoReleaseAndInspEndByFlag()` | 只設旗標，由 MainProc 一次一條送 `@RELEASE+`→ack→`@END+`→ack→`@START+`（JerryYang 20260730 `RTC command send one by one`）|
| ≤ 907.x / 904.x | `DoReleaseAndInspEnd()` | 同步送 `@RELEASE+`/`@END+`，中間 `MySleep(100)`，最後呼叫 `InitRealTimeCCDPara()` |

兩者最終都會呼叫 `InitRealTimeCCDPara()`，把 `bSendRealCCDSendStart` / `bSendRealCCDSendVerify` / `bRealCCDSendArm` 設為 true，MainProc 隨即補送 `@START+`。

**已驗證的無害性（以 SCK 設定為例）**：`InitRealTimeCCDPara()` 會清掉 `bRealTimeCom_ReceiveOK[]`（rtVISIONON 除外），但下游三道關卡都會放行 ——
D33 `bRTCInitStartVerify=0` → 走 else 直接 `bSendRealCCDSendVerifyOK=true`；
ROI Check 只針對 TSMC_TAINAN / Microchip_Phil / `bSPILFunction` → 其餘走 else 放行；
ROI Count 雖 `bRTC_ROICount=true`，但判斷式看的是 `bRealCCDROICountCheckOK`（本函式不動它，生產中恆 true）→ 放行。

**殘留風險**：ByFlag 版的派送器在等 ack 時會 `return` 出 MainProc，而 `DoAllProcess()`（驅動 Auto Clean 與各手臂）排在其後。正常握手只佔約 30+30 個掃描（毫秒級）；但若 RTC 完全不回 ack，會進入 10 秒逾時重送迴圈，期間整台暫停並可能跳出 `RTC Release Time Out Error` / `RTC End Time Out Error`。同步版（904.x）無此風險，只多 200ms 阻塞。

### 方案 A（備援）— 客戶碼關掉 Full View

在 `FUNC_CC_SCK()` 內加：

```cpp
    CosFunction.bFullTestBeforeAutoClean                                        =false; //RogerYang 20260815 : SCK RTC1.0 Auto Clean前不做Full View,避免RTC停在Full View模式導致WAR0335
```

等同還原 r751 行為。`bFullTestBeforeAutoClean` 全程式只有 `AutoClean.cpp` 一處使用，影響面單純；代價是失去「Auto Clean 前確認 socket 無殘料」這道檢查。

## 7. 驗證判準

改版後看 Event Log：**每次 `Auto Clean Finish!` 之後 10 秒內，不應再出現 `WAR0335`，也不應再出現 `WAR0335 auto retry com port.`**。

後者是無聲徵兆，比警報更早反映成效；兩者都消失才算修好，只剩後者代表沒修到。

## 8. 判讀陷阱與教訓

1. **「客戶函式沒設定 = 沒開啟」是錯的**。CosFunction 旗標多數由 `InitialCosFunction()` 給全域預設，客戶函式只做覆寫。判某旗標實際值，要同時看全域預設 + 客戶函式，且以呼叫順序（客戶函式在最後）決定勝負。
2. **執行期證據優先於程式碼推論**：log 有沒有 `Start Full View Check...` 就是旗標實際值的鐵證。
3. **alarm 次數 ≠ 發生次數**。凡是經過 `OpenRTCComPortAgain()` 的警報都有 2 次無聲重試，實際故障率是 alarm 的 3 倍。
4. **回歸案要先確定 build 對應的 revision**。本 repo revision == build number，`svn cat -r 751 <file>` 即可取回三年前的碼；不要因為本機沒有該版資料夾就放棄比對。
5. **韓國版版號顯示 V3.21.xxx 是寫死標籤**，`Ver.txt` 首行的 `V3.21.886.2` 就是 `V3.33.886.2`，勿當成不同分支。
6. 排除過的無關項：`bRTC20GiveWayCheck` 的 guard 在新版被註解掉（751 有、904 無）——現場 recipe `bRTC20GiveWayCheck=0`，兩版行為相同，中性；`DoFullViewCheck()` 本體 r751 與 904 逐行相同。

## 9. 程式碼錨點表

行號以 **V3.33.908.12** 為準，**使用前務必重新 grep**。

| 位置 | 內容 |
|---|---|
| `AutoClean/AutoClean.cpp:4651` | `bFullTestBeforeAutoClean && REAL_TIME_CCD && !bCCDDummyRum` → `Task=3` |
| `AutoClean/AutoClean.cpp:4673` | case 3；**修正插入點（4676）** |
| `cContact.cpp` `DoFullViewCheck()` | `@FULLM+` → `@LIGHTON` → Y 移動 → `@CHECKNULL+`＋`@FULLT+`；case 600 回 true |
| `aTester_Front.cpp:6604` 一帶 | case 115 等 `SnRealTimeCCDIndexArm`、`iRealCCDSendArmCT>15`、WAR0335 |
| `aTester_Front.cpp:6622` | `ShowErrorMessage("WAR0335", ...)` |
| `aTester_Front.cpp:6625` | `bSendRealCCDSendStart=true;`（Sam 20250220 補救）|
| `atester.cpp:5980-6035` | `DoTestHeadMotor()` case 1 四選一分支（決定有沒有收尾）|
| `atester.cpp:6028` | else 分支的 `DoReleaseAndInspEndByFlag()` |
| `csystem.cpp:17337` | `if(SystemStart)`（RTC 區塊都在其內，生產中每掃描執行）|
| `csystem.cpp:18754-18823` | ByFlag 派送器：`@RELEASE+`／`@END+` 一次一條等 ack |
| `csystem.cpp:18932` | `SendCommToVision(rtInspStart)` = `@START+` |
| `csystem.cpp:19078` | `DoAllProcess()`（排在 RTC 區塊之後，故派送器 `return` 會暫停機台）|
| `rs232.cpp:3752 / 3768` | `DoReleaseAndInspEnd()` / `DoReleaseAndInspEndByFlag()` |
| `rs232.cpp:3688` 一帶 | `InitRealTimeCCDPara()` — 設 Start/Verify/SendArm 旗標並清 ReceiveOK |
| `rs232.cpp:695` | `OpenRTCComPortAgain()` — `MaxCount=2`，第 3 次才回 true |
| `CosFunction.cpp:4168` | 全域 `bFullTestBeforeAutoClean=true`（Sam 20230517）|
| `CosFunction.cpp:1259-1299` | `FUNC_CC_SCK()`（方案 A 插入處）|
| `CosFunction.cpp:4482` | `DoCustomerFunction()` — 位於 `InitialCosFunction()` 最後一行 |
