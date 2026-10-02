# OutArm Y-Pitch 縮 pitch 換行程 —— 大 IC 前後排互撞（JAM0203）

> 建立：2026-08-31 (RogerYang)
> 案例來源：芯云（CUSTOMER_CODE=781）HT9046LS，S/N **PPLD2924**，V3.33.908.8
> 修正落點：`HT9011UC_Code_V3.33.908.16_20260827_NB_AI`（待編譯 / 待上機驗證）

---

## 0. ⚠ 讀碼前必看：磁碟上有 5 個 OutArm 模式檔不在 `HT9045.bpr` 裡

`aoutarm9045_*.cpp` 有一批**存在於磁碟但沒有被編譯**的孤兒檔。分析時若照檔名去找模式專屬檔，
會讀到完全不會執行的碼（本案我第一次就栽在這裡）。

| 檔案 | 在 `HT9045.bpr`？ | `iInArmType` 實際分派到 |
|---|---|---|
| `aoutarm9045_2x2_4_14.cpp` | ❌ **不在** | `e9045_2x2_4_12/_13/_14` → `DoOutArm_9045_2x2_4()`（`aoutarm9045_2x2_4.cpp`） |
| `aoutarm9045_2x2_4_23.cpp` | ❌ **不在** | 無對應 enum，純孤兒 |
| `aoutarm9045_2x8_32.cpp` | ❌ **不在** | `e9045_2x8_32` → `DoOutArm_9045_2x8_8()`（`aoutarm9045_2x8_8.cpp`） |
| `aoutarm9045S_1x4_4.cpp` | ❌ **不在** | `e9045_1x4_4_13` → `DoOutArm_9045_1x4_4S()`（`aoutarm9045_1x4_4S.cpp`，**不同檔**） |
| `aoutarm9045_1x4_4_Back.cpp` | ❌ **不在** | `e9045_1x4_4_Back` 的 dispatcher 分支是**空 stub（只有 `//`）** |
| `aoutarm9045_2x4_16.cpp` | ❌ **不在** | — |

**檢查方式**（分析任何 OutArm 模式問題前先做）：
```powershell
$t=[System.IO.File]::ReadAllText("<code>\HT9045.bpr",[System.Text.Encoding]::GetEncoding(950))
[regex]::Matches($t,'aout[A-Za-z0-9_]*\.obj') | ForEach-Object { $_.Value } | Sort-Object -Unique
```
再對 `aoutarm9045.cpp` 的 `DoOutArm_9045()` dispatcher（約 L610-700）確認 `iInArmType` 的實際落點。

> 注意 InArm 側不一樣：`ainarm9045_2x2_4_14.obj` **有**在 project 裡。
> 「InArm 有模式專屬檔」不代表「OutArm 也有」，兩邊要各自確認。

---

## 1. 什麼時候載入這份文件

看到以下任一情形：

- 客戶說「Out Arm 吸嘴直接把料碰掉了」、「撞料的時候 Y PITCH 靠得太近」
- `JAM0203 Out arm device drop error`，且 `CheckOutSuckICFallDown JAM0203 - Picker Position` 顯示**八支 Z 全在 50（安全高度）**
- StateRecord 的 `Motor.xls` 裡 **`MOutArmPitchY` 目前位置 == 目標位置 == `Tech.iOutArmY15Pitch`**（Y-Pitch 停在機構最小值）
- 現象**偶發**：斷電推拉沒干涉、往返 loop 幾十次沒失步、跑 dummy 一小時不複現，但生產時晚上又發生
- 關鍵字：`iMovePitchY`、`IN_OUT_ARM_Y_PITCH_MIN`、`PSoftLimitN`、`ShrinkOutArmYPitchForSoftLimit`、
  `MOutArmPitchY`、`AutoCalculateOutArmYClosePitch`、Tray 最後一排、縮 Y pitch

---

## 2. 機制（為什麼會撞）

可變 Y-Pitch 機型（`USE_OUT_ARM_Y_PITCH==iXYPitchVariable`）放料到 Auto/Fix Tray 時：

```
iYPos = Prod.YStart[tray][i][j] - iy * AutoForm->YPitch     // 基準排(Row1)對應的格子
if(iOutArmPlaceOrder==0)
    iYPos -= iMovePitchY;                                   // 要讓 Row0 吸嘴對到這一格, Y 軸要再往負向多走一個 pitch
```

因為 `iOutArmYBase==1`（**Row1 是 Y-Pitch 機構的固定端，Row0 隨 pitch 移動**），放前排（`iOutArmPlaceOrder==0`）時 Y 軸必須額外往負向走 `iMovePitchY`。
Tray 排數越多、越靠後面那幾排，`iYPos` 越負 —— **最後一排會踩到 `MOutArmY` 的負向軟體極限**。

原本的處理是：踩到極限就**無條件把 Y-Pitch 縮到 `IN_OUT_ARM_Y_PITCH_MIN`（通常 25.00mm）**，用縮 pitch 換回行程。
落點本身是對的（pitch 縮多少、`iYPos` 就往正向補多少，Row0 仍精準落在同一格），
**但沒有任何「元件吃不吃得下這個 pitch」的檢查** —— pitch 收到 25mm 時，
前後排兩支吸嘴上的大尺寸元件會在空間上重疊，互撞後其中一顆被碰掉 → `JAM0203`。

> 這個病 2019 年就被記在碼裡：`AutoCalculateOutArmYClosePitch()`（`aoutarm.cpp` L3043）
> `int iMin=1500; //kevin 20190518 : OutArm 放置AUTO最後一排 合到最小PITCH IC 掉落`
> 當年只補了「換算 close pitch」那一段的下限，**沒補這條強制 `IN_OUT_ARM_Y_PITCH_MIN` 的分支**。

### 為什麼是偶發

必須三個條件同時成立：

1. 放的是 **Tray 的最後一排**（倒數第二排離極限還有一個完整 Y-Pitch，約 44mm，根本不會觸發）
2. 該趟是 **`iOutArmPlaceOrder==0`**（放前排）
3. 當下**另一排吸嘴上還有料**（若只剩一顆單顆放料，就沒有互撞對象）

所以三天只出現一次、dummy 也複現不出來。

### 🔎 最有力的旁證：InArm 側早就修過同一個病，OutArm 側沒跟上

`ainarm9045.cpp` L5705-5730（**有**在 project 裡）是同一段邏輯的 InArm 版，
已經被改成「不要無腦縮到底」了：

```cpp
if(iYPos<(MOT[MInArmY].Motor->PSoftLimitN+10))
{
    iYPos=iYPos+iMovePitchY;
    if((UserDefForm[Ld].YPitch-500)<IN_OUT_ARM_Y_PITCH_MIN)      //Steven 20230727 : 當TrayPitch比最小還小的時候,才能縮到Tray Pitch
        iMovePitchY=IN_OUT_ARM_Y_PITCH_MIN;
    else
    {
        if(UserDefForm[Ld].YPitch>6000)                          //Stevenhong 20260325 : TESNA 的IC 與Pitch 比較大
            iMovePitchY=DeviceForm.YDimension/2+1000;
        else
            iMovePitchY=UserDefForm[Ld].YPitch-500;
    }
    iYPos=iYPos-iMovePitchY;
}
```

也就是 InArm 在 2023-07-27 就已經改成「最多只縮到 TrayPitch − 5mm」，
2026-03-25 又為 TESNA 大 IC 再加一條。**OutArm 兩次都沒同步。**

⚠ 同時注意 InArm 那條 `DeviceForm.YDimension/2+1000` 分支是**潛在風險**：
`DeviceForm.YDimension` 來自 `Contact.Data [Torque Control] Y Dimension`（Die 尺寸，不是封裝），
芯云這台實際填 8.20mm → 該式會算出 `8.2/2+10 = 14.1mm`，比 25mm 還小。
目前只在 `TrayPitch > 60mm` 時才會走到，暫時沒咬到人，但值得列為待觀察項。

---

## 3. 芯云 PPLD2924 實測數字（可拿來當判讀樣板）

| 項目 | 值（pulse = 0.01mm） | mm | 來源 |
|---|---:|---:|---|
| 機台 / 版本 | HT9046LS，V3.33.908.8 | | `Ver.txt` / `Gerneral.ini` |
| 模式 | `QualSite2X2`（"4-Site (2x2)"），`iInArmType=e9045_2x2_4_14` | | `DecisionVariables.csv` |
| 元件 | recipe 名寫 39×45 mm（⚠ **證據薄弱，見下方**） | | recipe 資料夾名 `FCBGA_39X45_...` |
| Site pitch | X=12000 / Y=**6000** | 120 / 60 | `HandlerCondition.Data` |
| Auto Tray pitch | X=6570 / Y=**4435**，2 欄 × **7 排** | 65.70 / 44.35 | `Tray.Data [Type0]` |
| Y-Pitch 範圍 | MIN=**2500** / MAX=7500 | 25.00 / 75.00 | `Gerneral.ini IN_OUT_ARM_Y_PITCH_MIN/MAX` |
| Y-Pitch Teach | 25mm→**−4853**、60mm→**−1442** | | `teach.ini [MOutArmPitchY]` |
| **`MOutArmY` SoftLimitN** | **−96468** | −964.68 | `system\motor_SMC_XYPitch.DB` 欄位 `SoftLimitN` |
| 判斷門檻 `SoftLimitN+10` | −96458 | −964.58 | |
| 縮 pitch **前**原始落點 | **−96572** | −965.72 | 由 Motor.xls 目標值反推 |
| **超過極限** | **104** | **1.04 mm** | |
| 縮 pitch **後**下的 Y 命令 | −94637 | −946.37 | `Motor.xls` `MOutArmY` 目標位置 |
| 縮回來的量（4435−2500） | +1935 | +19.35 | |

**結論：為了 1.04 mm 的行程，程式把 Y-Pitch 收掉 19.35 mm。**

### ⚠ 「39×45」的證據強度（別當事實引用）

`Contact.Data [Torque Control]` 這台填的是 **`X Dimension=8.20 / Y Dimension=8.20`**，
而該欄位語意就是 IC 外型尺寸（見 §5「為什麼不用元件尺寸」），所以 **8.20 幾乎確定是錯資料**。
但「39×45」的**唯一來源是 recipe 資料夾名稱**，那是命名慣例，不是量測欄位。

機台資料能證明的只有一個區間：

| 界線 | 值 | 依據 |
|---|---|---|
| 上限 | tray 方向 Y 尺寸 **< 44.35 mm** | Tray Y Pitch=44.35 塞不進更大的料；`cContact.cpp:2167` 的 UI 上限也夾在 Loader Tray YPitch |
| 下限 | 有效干涉尺寸 **> 25 mm** | pitch 收到 25mm 時真的撞掉料 |

而且這個「有效干涉尺寸」是 `max(IC_Y, 吸嘴座寬度)`，機台端**無法分辨撞的是料對料還是對面吸嘴座撞到料**。
間接一致性（Hot Plate pitch 90×60 + `Use Wide Hotplate=1`、Site pitch 120×60、Tray 2 欄×7 排）
只能推翻 8.20，推不出 39。

**這不影響根因結論** —— 不管 IC 是 30 還是 39，只差 1.04mm 行程卻收掉 19.35mm 都不合理。
IC 具體尺寸只影響「IC 安全下限該設多少」，那個值本來就該由客戶填的欄位提供，不該由 RD 猜。
要確定請客戶提供 package drawing 的 body size，或直接照規格書把 Contact Form 那兩格填對。

事故簽章（`EventLogTxt_20260827.csv`）：
```
17:39:09  CheckOutSuckICFallDown JAM0203 - Picker Position  A:50, G:50, E:50, C:50, B:50, H:50, F:50, D:50
17:39:09  "02 Output Arm", JAM0203, RETRY, 465, 0, "Out arm device drop error", " B"
17:39:09  Out Arm Servo Off, X=-43412, Y=-9877
```
三天 EventLog 只有這一筆 JAM0203，且**全程沒有任何 WAR0254/0255 超極限警告**（縮 pitch 分支把它靜默吸收掉了）。

### StateRecord 判讀捷徑

1. `Motor.xls`（**BIFF 二進位，不是文字**，解法見 `ht9045-staterecord-analysis` LL-17）
   → 看 `MOutArmPitchY` 的「目前位置 / 目標位置」。
2. 拿 `teach.ini [MOutArmPitchY] setEditOutY15 / setEditOutY60` 換算 mm：
   `pitch_mm = IN_OUT_ARM_Y_PITCH_MIN + (pos − Y15) × (6000 − MIN) / (Y60 − Y15)`
3. **若 `MOutArmPitchY` 位置 == `setEditOutY15`（誤差 0），就是命中這條分支。**
   全 OutArm 碼庫只有幾支會傳 `IN_OUT_ARM_Y_PITCH_MIN` 給 `GetOutArmPitchY_9045()`，等於指紋。
   （唯一另一個候選是 `GetVariableYOutShuttleData()` 在 `dSiteYPitch<=MIN` 時的 clamp，
   但那要 site pitch ≤ 25mm，一般不成立。）
4. 對照 `MInArmPitchY` —— 正常機台它會停在 site pitch（本案 60.3mm），與 OutArm 的 25mm 形成明顯落差。
5. `Task_ListWithTime2.csv`：`OutArmTask=3301` + `PlaceToAutoTask=10`（卡在 case 10，modal alarm 凍住）。
6. `MOutArmY` 的「目標位置」是真的（`PCIL112_OutArmXYMove` 走 `MOT[].MotorMove()` → 會寫 `TargetPosition`），
   可以直接拿來反推縮 pitch 前的落點：`縮前 iYPos = 目標 − (原 pitch − 25mm)`。

---

## 4. 程式鏈（實際會執行的那條）

```
OutArmTask 3301
 └ DoOutArm_9045()                      aoutarm9045.cpp  dispatcher (L661-666)
    └ DoOutArm_9045_2x2_4()             aoutarm9045_2x2_4.cpp        ← _12/_13/_14 三種都走這支
       └ DoOutArmPlaceToAuto_9045()     aoutarm9045.cpp L3130  (iPlaceToAutoTask = CSV 的 PlaceToAutoTask)
          case 10:  (迴圈, SetOutArm 回 false 就 break, 下個 scan 再進來)
            ├ CheckOutSuckICFallDown(false)                 ← JAM0203 從這裡報出來
            └ SetOutArm_9045()                aoutarm9045.cpp L2546
               └ SearchUnLoadTrayUpDown_9045()
                  └ DoMoveOutArmXYToPlace_9045(..., RealMove=true)   aoutarm9045.cpp L2386
                     └ if(iYPos < MOT[MOutArmY].Motor->PSoftLimitN+10) → 縮 Y-Pitch   ★缺陷點 (L2437)
                        └ OutArmContinuousMove_9045() → OutArmPitchMove()
                           └ MOT[MOutArmPitchY].MotorMove(...)       mymotor.cpp L3411
```

`aoutarm9045_2x2_4.cpp` **沒有**自己的縮 pitch 區塊，一律走通用的 `aoutarm9045.cpp:2437`。

**時序（重要，別誤判）**：case 10 是迴圈。
第一次進 case 10 → 掉料檢查 OK（料都在）→ `SetOutArm_9045()` 啟動移動並下 Y-Pitch=25mm →
pitch 收合過程中兩排料互撞 → 下一個 scan 再進 case 10 → 掉料檢查抓到 → `JAM0203`。
所以 log 上「Z 全在 50、還沒下壓」是必然，**不是吸嘴漏氣、不是黏料**。

### 缺陷分佈（原始碼有 7 處，但只有 2 處會執行）

| 檔案 | 原始行（908.16） | 在 `HT9045.bpr`？ |
|---|---|---|
| **`aoutarm9045.cpp`** | **2437** | ✅ **會執行**（通用放料路徑，所有 in-project 模式都走這裡） |
| **`Magazine.cpp`** | **939** | ✅ **會執行**（Magazine 取料側，變數是 `iOutArmPickOrder`，且無 `iYVariable` 那行） |
| `aoutarm9045_2x2_4_14.cpp` | 1180 | ❌ 不在 project |
| `aoutarm9045_2x2_4_23.cpp` | 1095 | ❌ 不在 project |
| `aoutarm9045_2x8_32.cpp` | 1285 | ❌ 不在 project |
| `aoutarm9045S_1x4_4.cpp` | 1032 | ❌ 不在 project |
| `aoutarm9045_1x4_4_Back.cpp` | 1152 | ❌ 不在 project |

那 5 個孤兒檔的分支裡另有一個誤用 `GetInArmPitchY_9045()`（In 不是 Out）的死碼，
因為檔案本身沒編譯，**一併不處理**；若哪天把這些檔加回 project，記得同步套用本節的修法。

---

## 5. 修法（20260831，已進 908.16_AI）

### 設計取向（兩層）

**第 1 層：只縮「行程缺口 + 1mm 餘裕」，不縮到底。**
**第 2 層：不得縮破「IC Y 尺寸 + 1mm」（吸嘴吸在 IC 正中央，pitch 就是兩顆料的中心距）。**

```
iSafeMin = max(IN_OUT_ARM_Y_PITCH_MIN, DeviceForm.YDimension + 100)   // YDimension=0 表示沒填 -> 不判斷
if(iSafeMin >= iMovePitchY) return iMovePitchY;                        // 沒有可縮空間 -> 不縮, 由呼叫端跳過該格
iNeed = (PSoftLimitN + iKeep) - iYPos                                 // iKeep = 100 (1mm)
iNew  = max(iMovePitchY - iNeed, iSafeMin)
iYPos += (iMovePitchY - iNew)                                         // pitch 縮多少, Y 就往正向補多少 -> 落點不變
```

芯云實例：`iNeed=204` → `4435 → 4231`（42.31mm），`iYPos = SoftLimitN+100`。
若客戶把 `Y Dimension` 填成 39.00 → `iSafeMin=4000`，42.31mm 仍在上面，**行為完全不變**。

（比 InArm 側 Steven 20230727 的「縮到 TrayPitch−500」更保守：本案只縮 2.04mm，InArm 式會縮 5mm。）

### 幾何模型：吸嘴在 IC 正中央

```
邊到邊間隙 = pitch − IC_Y        （單邊 = (pitch − IC_Y)/2）
```

| 狀態 | Y-Pitch | 間隙 |
|---|---:|---:|
| 正常（Tray pitch） | 44.35 | +5.35 |
| 舊碼縮到底 | 25.00 | **−14.00（重疊）** |
| 改後（芯云實例） | 42.31 | +3.31 |

這不是我自己假設的模型 —— X 方向 `aoutarm.cpp:3130` 就是這樣算：
`OutArmClose_PitchX = DeviceForm.XDimension*iOutArmXStep + 100.0;  //要比IC大一點, 避免撞到`
所以「中心距 = IC 尺寸 + **1.00mm**」是本家慣例，Y 方向沿用同一個 `+100`。

**只有第 1 層時的失效邊界**（記錄下來，說明為什麼一定要加第 2 層）：
`縮後 pitch = TrayPitch − (E + iKeep)`，E = 超出極限量。
要 ≥ IC+1mm ⟹ **E ≤ TrayPitch − IC − 200**。芯云是 `4435−3900−200 = 335 = 3.35mm`，
實際 E 只有 1.04mm 所以過關 —— **但那是運氣，不是保護**。

### `DeviceForm.X/YDimension` 就是 IC 外型尺寸（我一開始判錯，已更正）

我最初以為它是 Die 尺寸而否決了 IC 判斷，那是錯的。四個證據：

| 位置 | 證據 |
|---|---|
| `cContact.cpp:2165-2187` | UI 把它夾在 `[2.0, Loader Tray X/Y Pitch]`，註解「ic大小最大不能比Tray的X pitch大」 |
| `cTrayMapping.cpp:2949` | 傳給 CCD 的註解「**W、H 代表 IC 大小(單位:mm)**」 |
| `cSpeed.cpp:136` | 「IC 大於 25mm 時，一次吸 4 個只能使用 Fix Mode」 |
| `aoutarm.cpp:3130` | 「要比IC大一點, 避免撞到」 |

**軸向也對**：UI 夾的上限是 Loader Tray 的 X/Y Pitch ⇒ 這個欄位是以 **Tray 方向**定義的，
而 Y-pitch 縮 pitch 發生在放 Tray 端，兩者同軸 ✓。

**單位 / 型別**：`SYSTEM_DEVICE_FORM.YDimension` 是 `double`（`cprod.h:1168`）；
runtime 的 `DeviceForm` 經 `cUnitConvert.cpp:69-70` `iUnitMultiply100()` 換成 **0.01mm**，
與 `iMovePitchY` 同單位，可直接比較 ✓。

**沒有任何 contact force 計算引用這兩個欄位**（全 `c*.cpp` grep 過，force 走 `Torque / Pin Number / Force Per Pin`），
所以叫客戶改它**不會動到壓力**。

⚠ 但芯云填 8.20 讓這一層對他們**目前是空的**（820+100 < 2500），
所以**第一步必須請客戶把 Contact Form 的 X/Y Dimension 照規格書填對** ——
同一個欄位還被 `cSpeed.cpp`（IC>25mm 強制 Fix Mode）、`csystem.cpp:754/764`、
`cinitial.cpp:7415/7538`、`AutoCalculateOutArmXClosePitch()`（`bOutArmXOverLimit` 後才啟用，
屆時會算出 8.2mm 級 X pitch → **X 方向也會撞**）一起使用，現在全部形同無效。

### 必須配逃生出口，否則變靜默死迴圈

只加 clamp 會出現這條路徑：
```
縮到 IC 下限 -> iYPos 仍 < PSoftLimitN
  -> OutArmContinuousMove_9045() 的 if(iY<=PSoftLimitN) -> WAR0255(0 個按鈕) -> return false
    -> DoMoveOutArmXYToPlace_9045 回 false -> SetOutArm_9045 回 false
      -> DoOutArmPlaceToAuto_9045 case 10 沒推進 -> 下個 scan 再進 case 10 -> 無限重試
```
所以把 Y 條件併進**同一個函式裡既有的 X 逃生出口**（原 `aoutarm9045.cpp` L2454）：
`該格標 HAS_NULL_IC + ReserveEmptyPoint() + return false`。

**終止性已追盡**：
1. 標掉該格 → `SearchTrayToPlace_9045()` / `SearchUnLoadTrayUpDown_9045()` 換別的格
2. 若整盤沒別的格 → `iRow<0||iCol<0` → `bOverTray=true` → case 10 `MOT[..].SetTray(HAS_IC)` → **整盤視為滿盤退掉**
3. 放料成功後 case 400/500 呼叫 `ReversionEmptyPoint()`（`cprod.cpp:1450`）把保留格還原成 `NULL_IC`
   → 被跳過的格子**不會永久損失**（除非 `MOT[MMAuto1].Tray.FullIC()`，那時本來就要退盤）

**擺放位置很重要**：逃生出口必須在 `if(RealMove==false) return bFlag;` **之後**，
否則 dry-run 也會去標 `HAS_NULL_IC`，把好格子誤標成空。既有 X 出口本來就在那個位置。

### ⛔ 不可改 `AutoCalculateOutArmYClosePitch()`

`aoutarm.cpp` L3041 那支函式有**全域副作用**：寫 `OutArmClose_PitchY`，
且在 `USE_OUT_Y_IS_AUTO_PITCH==false` 分支寫 `iOutArmYStep`。
而 `iOutArmYStep` 決定「這一趟要填 Tray 的哪幾格」
（`aoutarm9045.cpp` `DoOutArmPlaceToAuto()`、`aoutarm.cpp` `SearchUnLoadTrayUpDown`）。
改它會連動改掉 Tray 帳。**只能改呼叫端的區域變數 `iMovePitchY`。**

### 「縮不夠就跳過該格」為什麼不是 regression

一開始我擔心：小元件機台（如 8×8 BGA）本來就依賴縮到 25mm 才放得進最後一排，
加拒放會讓它從「正常放」變成「少放一格」。**追下去發現這個擔心不成立**：

| 情境 | 舊碼 | 新碼 |
|---|---|---|
| 縮到 25mm 就夠（小元件） | 正常放 | `iSafeMin=2500` 不變 → **正常放，行為完全相同** |
| 縮到 25mm 才夠、但元件大（芯云這類） | 放下去 → **撞料 JAM0203** | 被 IC 下限夾住 → **跳過該格** ← 這就是要修的 |
| 縮到 25mm 仍不夠 | `iYPos` 仍超極限 → `OutArmContinuousMove_9045` 報 **WAR0255 → return false → case 10 無限重試**（舊碼本來就放不了，只是卡著） | 跳過該格 → 繼續生產 → **嚴格更好** |

所以新碼唯一改變行為的地方，是把「撞料」和「WAR0255 卡住」換成「少放一格」。
兩者都是改善，不需要 Config 開關保護。

### 落點與帳為何不變（連動失效檢查結論）

| 項目 | 結論 |
|---|---|
| 放料落點 | **不變**（pitch 與 `iYPos` 同步補償，數學上可證） |
| Tray 帳（填哪幾格） | 不變（`iOutArmYStep` 由 `AutoCalculateOutArmYClosePitch()` 獨立重算，不吃縮過的 `iMovePitchY`） |
| `iOutArmPlaceOrder==1` 那一趟 | 不進此分支，不受影響 |
| 既有能正常放最後一排的機台 | 行為等價：原本能放的仍能放，只是收得比以前少、pitch 馬達走更短（略快） |
| `GetOutArmPitchY_9045()` 內插 | 新 pitch 落在 [MIN, 6000] 內，是**內插不是外插**，數學有效 |
| 磁性尺 / Auto Clean / Rotator / AOI / `bOutArmXOverLimit` | 全不相干 |

### 邊界（新碼比舊碼更安全的一點）

`iMovePitchY <= IN_OUT_ARM_Y_PITCH_MIN` 時直接 return。
**舊碼在 `iMovePitchY==0`（recipe Y-Pitch 沒設）時會 `iYPos -= 2500`，反而把 Y 推得更負、更超極限。**

### 實際改動（只有 3 個檔案，全部在 `HT9045.bpr` 內）

| 檔案 | 內容 |
|---|---|
| `aoutarm9045.cpp` | ① 新增 `ShrinkOutArmYPitchForSoftLimit()`（含 IC 下限），放在 `GetOutArmPitchY_9045()` 之後<br>② 通用呼叫端（原 L2437 區塊）<br>③ 既有 X 逃生出口（原 L2454）加上 Y 條件 |
| `aoutarm9045.h` | 加宣告（放在 `GetOutArmPitchY_9045` 宣告下一行） |
| `Magazine.cpp` | 只做「只縮必要量」，**不加 IC 下限** |

**為什麼 `Magazine.cpp` 不加 IC 下限**（刻意收斂）：
- 它是**取料**（`iOutArmPickOrder` / `iPickWhichBuff`），既有 X 逃生出口（`Magazine.cpp` L964-982）
  的做法是把**來源格**標 `HAS_NULL_IC` —— 在取料側等於「帳上變空、實體還在盤上」＝**製造殘料**。
- 而且它的 X 出口第一個分支是 `iPickWhichBuff==2` 時把 X 拉回極限內（「差5mm內的, 放不下就讓它歪歪的放下去」），
  這個補救對 Y 不成立，**不能直接把 Y 條件併進同一個 if**。
- 沒有 Magazine 機可驗證前不動它。只縮必要量已比原版嚴格更好、且不新增任何無出口路徑。

log 格式（`RecordProcess`，同值只記一次、上限 50 筆）：
```
OutArm YPitch shrink by SoftLimitN : generic place, YPos -96572->-96368, Pitch 4435->4231, LimitN=-96468, Need=204, SafeMin=2500
```
結尾 tag：
- 無 tag → 只縮了必要量，沒碰到任何下限（**正常**）
- `, CLAMPED-BY-IC` → 被 IC 尺寸下限夾住 → 該機台的 Tray 最後一排本質上放不下，會走逃生出口跳過該格
- `, CLAMPED-TO-MIN` → 被機構最小 pitch 夾住（代表 `Y Dimension` 沒填或很小）

### 動手順序提醒

`aoutarm9045.cpp` **同一檔案兩處**（helper 在前段、呼叫端在 L2437）→ 必須**由下往上**做：
先改 L2437 區塊，再插 helper，否則行號位移。

Big5 檔案改動只能用位元組層行替換（讀 bytes → 只重寫目標行 → 寫回），
中文片段以 UTF-8 撰寫再轉 CP950，並做 round-trip 檢查。
驗證重點：非 ASCII 位元組數不該無故變動、落單 LF 必須為 0、字面 `?`（0x3F）不得增加。

---

## 6. 硬體 / 設定面的解（優先於程式解）

程式修法是通用保險；**個別機台的根本解通常在機構或 teach**。

| 方案 | 做法 | 風險 |
|---|---|---|
| **A. 放寬 `SoftLimitN`（首選）** | 芯云：−96468 → −96800（多 3.3mm）。改 Motor Test 頁或馬達表 `SoftLimitN` | ⚠ **必須先現場實測**：把 `MOutArmY` JOG 到 −96468，量到機械硬止檔／拖鏈／線材還剩多少，至少要 5mm。<br>⚠ 不利訊號：`MInArmY=−96404` 與 `MOutArmY=−96468` 只差 64 pulse 且非整數 → 看起來是當初頂到機構量出來的值，可能沒剩多少 |
| **B. Auto Tray Y offset 往正向挪 1.5~2mm** | +115 pulse 以上就跳不進那個 if | ⚠ Tray 口袋 Y 餘隙 = 44.35 − 元件 Y；元件 39mm 時單邊只有 ~2.6mm，挪 2mm 幾乎吃光 → 放料貼壁。<br>⚠ teach 點是機差校正基準，不建議當第一選擇 |
| **C. Tray `Y Division` 減 1（放棄最後一排）** | 零風險、立刻有效 | 每盤少 2 顆（本案約 −14% 裝盤數）。純止血 |

⛔ **不要**直接把 `Gerneral.ini` 的 `IN_OUT_ARM_Y_PITCH_MIN` 改大。
它同時是 `GetOutArmPitchY_9045()` 內插公式的錨點
（`r = Y15 + m×(w − MIN)`，`m = (Y60 − Y15)/(6000 − MIN)`），
改了整條 pitch 換算全跑掉，除非同步重 teach Y15。

---

## 7. 驗證測項（風險轉負向測試）

| # | 測項 | 預期 |
|---|---|---|
| 1 | 芯云 recipe，放 Auto1 **最後一排** | log 出現 `Pitch 4435->4231 ... Need=204`，**無 CLAMPED-TO-MIN**，料正常入袋、無 JAM0203 |
| 2 | 同上量測落點 | 與改版前同一格，偏移 < 0.2mm |
| 3 | 倒數第二排 | **完全不出現 shrink log** |
| 4 | 小元件 recipe（8×8 BGA，原本會縮到 25mm 的機台） | 能正常放滿最後一排；log 顯示收得比以前少或不收 |
| 5 | 故意把 Auto Tray Y offset 往負向拉 25mm 製造大缺口 | 出現 `CLAMPED-TO-MIN`，行為與舊版一致（照放），**確認沒有變成卡死或無限重試** |
| 6 | recipe Tray `Y Pitch = 0`（異常設定） | 不進 shrink、`iYPos` 不被往負向推、不報 WAR0255（舊版會） |
| 7 | Clean Out / Tray Feed / One Cycle 各一輪 | `iOutArmYStep`、Tray Data 與改版前一致 |
| 8 | Magazine 機型（若有測機） | 取料側正常（未加 IC 下限，行為應只是縮得比以前少） |
| 9 | 連跑 500 盤 | shrink log ≤ 50 筆（不洗 EventLog） |
| 10 | 把 `Y Dimension` 填 39.00 再跑測項 1 | log 出現 `SafeMin=4000`，pitch 仍 4231（>4000）→ **行為與填 8.20 時完全相同** |
| 11 | 把 `Y Dimension` 故意填 42.00（逼近 Tray pitch）| `SafeMin=4300 > 4231` → 出現 `CLAMPED-BY-IC`，該格被標 `HAS_NULL_IC` 跳過，**不可以卡死、不可以報 WAR0255** |
| 12 | 把 `Y Dimension` 填 44.00（≥ TrayPitch−100）| `iSafeMin >= iMovePitchY` → 完全不縮，直接走逃生出口跳過該格；整排跳完後整盤當滿盤退掉，**不可以 hang** |
| 13 | 測項 11/12 之後 | 下一顆料放到別的格，且 `ReversionEmptyPoint()` 有把被跳過的格還原（Tray 畫面不該永久留空洞） |

---

## 8. 這台機的三個配置查證（分析時容易踩空，先確認再推論）

| 項目 | 值 | 為什麼要查 |
|---|---|---|
| `USE_ROTATE_KIT` | **0**（旋轉器沒裝） | recipe `Rotate.Data` 有 `ActiveRotate=1, DutAngle=90` 是**殘留設定**、不會作用。若真有旋轉，tray 與 socket 的 39/45 軸向會對調，`DeviceForm.YDimension` 的軸向判斷就要重做。也對上 state record 的 `OutArmTask 3000→3010`（沒走 7000 附加功能） |
| `USE_PICKER_COUNT` | **1 = `ep8Picker`** | ⚠ `ep1Picker` 是 **4** 不是 1（`MachineType.h:1367-1371`）。若真是 `ep1Picker`，`OutArmPitchMove()` 會直接 `return 1`、**pitch 馬達根本不動**，整條「pitch 被縮」的推論就要作廢 |
| `DeviceForm.X/YDimension` 軸向 | **Tray 方向** | `cContact.cpp:2167-2184` 的 UI 上限夾在 Loader Tray X/Y Pitch |

## 9. 待辦 / 待觀察

- InArm 側 `ainarm9045.cpp` L5719-5722 的 `DeviceForm.YDimension/2+1000`（Stevenhong 20260325, TESNA）
  依賴不可靠的 Die 尺寸欄位，在 `TrayPitch > 60mm` 時會算出比 25mm 更小的 pitch。目前未咬到人，列為待觀察。
- `ainarm9045_2x8_32.cpp` L1131 也有 InArm 版縮 pitch 區塊（該檔**有**在 project），未一併檢視。
- 若日後把 5 個孤兒 `aoutarm9045_*` 檔加回 `HT9045.bpr`，需同步套用 §5 的修法。
- `Magazine.cpp` 取料側的 IC 下限未加（理由見 §5）。要補的話得先想清楚取料側的逃生出口語意
  —— 把來源格標 `HAS_NULL_IC` 會變成殘料，不能照抄放料側。
- 芯云的 `Contact.Data [Torque Control] X/Y Dimension = 8.20/8.20` 待客戶更正為實際封裝尺寸。
  更正前 IC 下限那一層對這台是空的（`820+100 < 2500`），保護只剩「只縮必要量」。

---

## 10. 相關

- 同一檔的 X 方向對照：`GetOutArmPitchX_9045()` 的 clamp（`aoutarm9045.cpp`，RogerYang 20260825，偉測 HHT-532）
  —— X-Pitch 開迴路步進超範圍會撞機械止檔掉步，也是「縮 pitch 類」缺陷家族。
- X 方向另有**元件尺寸感知**的 `iCalculateOutArmXPitch()`（Steven 20151112，
  註解「要判斷 IC 尺寸，如果尺寸小於 13mm 就不能縮到 40mm」）—— Y 方向原本完全沒有對應保護。
- 基準軸 / Y-Pitch 模組定義：`ht9045-sucker-architecture` §4.3
- StateRecord 中 `Motor.xls` 為 BIFF 二進位的解法：`ht9045-staterecord-analysis` LL-17
