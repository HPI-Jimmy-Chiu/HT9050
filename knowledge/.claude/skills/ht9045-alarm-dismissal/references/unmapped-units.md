# 落到 `else → palSys` 的 230 個 unit —— 分級與待填表

//Steven 20260924

`ShowErrorUnit()` 只顯式涵蓋 109 個 unit，其餘 **230 個**走最後一個 `else`，
一律送 `palSys`。**這是 golden 的既有行為**（`note.cpp:4776-4779`），照做＝忠實。

問題在於：其中有一批**家族已經有專屬 panel、只是漏掉**的軸，
報警時紅框會跑到「System」，等於指錯位置。本表把 230 個分四級，
A 級是要補的，C 級維持 `palSys` 才對。

| 級 | 數 | 意思 | 動作 |
|---|--:|---|---|
| **A** | 81 | 家族已有 panel，是遺漏 | **補進對照表**（建議 panel 已填，待確認）|
| **A?** | 24 | 有合理的既有 panel，但命名不保證 | 逐條判定後升 A 或降 B |
| **B1** | 33 | 畫面上沒有，但**已定案要長出新面板** | 照 §新面板規格 加到 `note.dfm` |
| **B2** | 42 | 畫面上沒有這個 panel，尚未定案 | 要新增面板才談得上對應 |
| **C** | 50 | 本機不存在的選配機構 | **維持 `palSys`**，不要動 |

> 「建議 panel」是依常數名的家族前綴推的，**不是** golden 的既有事實。

## Tray 軌的 Z / Y 規則（20260924 修正）

料盤軌的兩族軸**不同歸屬**，這條是 Steven 20260924 指出後回查 golden 確認的：

| 軸族 | panel | golden 依據 |
|---|---|---|
| `M{軌}Z`（`MLoaderZ` `MEmptyZ` `MColorZ` `MAuto1-6Z` `MLoad2Z`）| `pal{軌}_Car` | `cmydef.cpp:2658` `iTrayZMotor[]` 是**料車升降**（`iosetview.cpp:3489` 走 `Prod.TrayZ_Up/Mid`）；對齊 golden 既有的 `MMTrayZ → palLoad_Car` |
| `M{軌}Y` / `_CCW`（`MLoaderY` `MEmptyY` `MColorY` `MAuto1-6Y` `MLoad2Y`）| `pal{軌}` | `asendic.cpp:1294` `iStepMotor[]` 是 `LOAD_Y_USE_MOTOR` 的**軌道進出盤**；對齊 golden 既有的 `MMTrayY → palLoad` |

> ⚠ **HT9050 沒有料車。** `Mot_Table_9050.csv` 裡 `MLoaderZ` `MEmptyZ` `MAuto1-3Z`
> 都是 `Enable=1`（軸存在）、`MColorZ` 是 `Enable=0`（沒有 Color 軌），
> 但 9050 的 Tray Table 是固定檯面、沒有車。
> 告警視窗永遠是 `Alert.Note.html`（HT9045 的 `note.dfm`），
> 所以 **HT9050 報 `MLoaderZ` 會亮一個該機種沒有的「料車」面板**。
> 這是既有架構缺陷（見 SKILL.md §9.4），不是本表的問題；
> `確認` 欄照 HT9045 的事實填單一值即可，機種差異由 `mv9045` / `mv9050` 兩欄表達。

## 怎麼填「確認」欄

一格填一個 token，**只認下面五種**：

| 填什麼 | 意思 | 結果 |
|---|---|---|
| `v` | 同意建議 panel | 照「建議 panel」那欄寫進對照表 |
| `palXxx` | 建議錯了，正解是這個 | 用填的這個 panel（會檢查它真的存在於 note.dfm）|
| `x` | 不該有對應 | 維持 `palSys`，並從待辦移除 |
| `?` | 不確定，要我去查 | 我查 golden 後回報，不自行決定 |
| *（空白）* | 尚未判定 | 不動 |

`v` 也接受 `✓` `V` `o` `y`；`x` 也接受 `✗` `X` `n`。大小寫不拘。

### 兩個省事的寫法

1. **整段同意**：在該段第一列填 `v`，其餘留空，然後在 `備註` 欄寫
   `套用到 #1-#18`。連續區間用 `#起-#迄`，不連續用逗號分隔。
2. **整批口頭交代**：不想開檔案改，直接講
   「`#1-#62` 全部 v，`#63` 改 `palOutSh2`，`#57`、`#58` 打 `?`」
   也一樣 —— 編號就是本表的 `#` 欄。

### 真的需要分機種時

少數情況才會用到，**一定要帶機種前綴**：

```
9045:palLoad_Car / 9050:palLoad
```

只寫 `A / B` 沒有前綴會讀不準（可能是「二選一」「主/備」「9045/9050」三種意思）。

### 範例

```
| 1 | `MInArmPitchY` | 31 | `palInArm` | `InArm` | `InPP` | v  | 套用到 #1-#4 |
| 2 | `MInArmPitchX2`| 32 | `palInArm` | `InArm` | `InPP` |    |              |
| 32| `MShuttle1Pitch`| 63| `palInSh1` | `InShuttle1` | `InShuttle` | palInSh2 | 這支是第二組 shuttle |
| 57| `MOutSortAa`   | 57 | `palOutArm`| `OutArm` | `OutPP` | ?  | Sort Arm 是獨立機構，不確定要不要併 OutArm |
```

> ⚠ 只改 `確認` 與 `備註` 兩欄。其餘欄位是機械產生的，改了下次重跑會被蓋掉
> （重跑時 `確認` 與 `備註` 會自動讀回保留）。

---

## A 級 —— 81 個（待填）

| # | unit 常數 | 值 | 建議 panel | mv9045 | mv9050 | 確認 | 備註 / golden 行尾註解 |
|--:|---|--:|---|---|---|:--:|---|
| 1 | `MInArmPitchY` | 31 | `palInArm` | `InArm` | `InPP` | `v` | Steven 20131002 : XY變距 //ChungHung 20131231 alter AutoYPitch |
| 2 | `MInArmPitchX2` | 32 | `palInArm` | `InArm` | `InPP` | `v` | Steven 20131002 : XY變距 |
| 3 | `MOutArmPitchY` | 33 | `palOutArm` | `OutArm` | `OutPP` | `v` | Steven 20131002 : XY變距 //ChungHung 20131231 alter AutoYPitch |
| 4 | `MOutArmPitchX2` | 34 | `palOutArm` | `OutArm` | `OutPP` | `v` | Steven 20131002 : XY變距 |
| 5 | `MLoaderZ` | 35 | `palLoad_Car` | `Loader` | `Loader` | `v` | 20260924 Steven 指正：Tray Z 屬料車 |
| 6 | `MEmptyZ` | 36 | `palEmpty_Car` | `Empty` | `Empty` | `v` | 20260924 Steven 指正：Tray Z 屬料車 |
| 7 | `MColorZ` | 37 | `palColor_Car` | `Color` | — | `v` | 20260924 Steven 指正：Tray Z 屬料車 |
| 8 | `MAuto1Z` | 38 | `palAuto1_Car` | `Auto1` | `Auto1` | `v` | 20260924 Steven 指正：Tray Z 屬料車 |
| 9 | `MAuto2Z` | 39 | `palAuto2_Car` | `Auto2` | `Auto2` | `v` | 20260924 Steven 指正：Tray Z 屬料車 |
| 10 | `MAuto3Z` | 40 | `palAuto3_Car` | `Auto3` | `Auto3` | `v` | 20260924 Steven 指正：Tray Z 屬料車 |
| 11 | `MInRotateKit` | 41 | `palInArm` | `InArm` | `InPP` | `v` | 2013-04-12 Dell :旋轉站;馬達版 |
| 12 | `MOutRotateKit` | 42 | `palOutArm` | `OutArm` | `OutPP` | `v` | 2013-04-12 Dell :旋轉站;馬達版 |
| 13 | `MLoaderY` | 44 | `palLoad` | `Loader` | `Loader` | `v` | Steven 20150910 : Add for OCR |
| 14 | `MEmptyY` | 45 | `palEmpty` | `Empty` | `Empty` | `v` |  |
| 15 | `MColorY` | 46 | `palColor` | `Color` | — | `v` |  |
| 16 | `MAuto1Y` | 47 | `palAuto1` | `Auto1` | `Auto1` | `v` |  |
| 17 | `MAuto2Y` | 48 | `palAuto2` | `Auto2` | `Auto2` | `v` |  |
| 18 | `MAuto3Y` | 49 | `palAuto3` | `Auto3` | `Auto3` | `v` |  |
| 19 | `MInArmZAe` | 50 | `palInArm` | `InArm` | `InPP` | `v` | Steven 20230323 : For HT1032 |
| 20 | `MInArmPitchX3` | 51 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 21 | `MInArmPitchX4` | 52 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 22 | `MInArmZAf` | 53 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 23 | `MOutArmPitchX3` | 54 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 24 | `MOutArmPitchX4` | 55 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 25 | `MTrayZ` | 56 | `palTrayArm` | `TrayArm` | — | `v` |  |
| 26 | `MOutSortAa` | 57 | `palOutArm` | `OutArm` | `OutPP` | `v` | Steven 20240822 : For HT-9046AU |
| 27 | `MOutSortAb` | 58 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 28 | `MInArmXScale` | 59 | `palInArm` | `InArm` | `InPP` | `v` | Steven 20160426 : 磁性尺 |
| 29 | `MInArmYScale` | 60 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 30 | `MOutArmXScale` | 61 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 31 | `MOutArmYScale` | 62 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 32 | `MShuttle1Pitch` | 63 | `palInSh1` | `InShuttle1` | `InShuttle` | `v` |  |
| 33 | `MShuttle2Pitch` | 64 | `palOutSh1` | `OutShuttle1` | `OutShuttle` | `v` | wei 20160914 Auto Shuttle Sensor |
| 34 | `MInRotateB` | 65 | `palInArm` | `InArm` | `InPP` | `v` | Steven 20170329 (Wei) : Add individual rotate motor |
| 35 | `MInRotateC` | 66 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 36 | `MInRotateD` | 67 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 37 | `MInRotateE` | 68 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 38 | `MInRotateF` | 69 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 39 | `MInRotateG` | 70 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 40 | `MInRotateH` | 71 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 41 | `MOutRotateB` | 72 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 42 | `MOutRotateC` | 73 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 43 | `MOutRotateD` | 74 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 44 | `MOutRotateE` | 75 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 45 | `MOutRotateF` | 76 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 46 | `MOutRotateG` | 77 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 47 | `MOutRotateH` | 78 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 48 | `MInArmZAg` | 80 | `palInArm` | `InArm` | `InPP` | `v` | Steven 20230323 : For HT1032 |
| 49 | `MInArmZAh` | 81 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 50 | `MInArmZBe` | 86 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 51 | `MInArmZBf` | 87 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 52 | `MInArmZBg` | 88 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 53 | `MInArmZBh` | 89 | `palInArm` | `InArm` | `InPP` | `v` |  |
| 54 | `MOutArmZAe` | 90 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 55 | `MOutArmZAf` | 91 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 56 | `MOutArmZAg` | 92 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 57 | `MOutArmZAh` | 93 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 58 | `MOutArmZBe` | 94 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 59 | `MOutArmZBf` | 95 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 60 | `MOutArmZBg` | 96 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 61 | `MOutArmZBh` | 97 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 62 | `MOutSortX` | 98 | `palOutArm` | `OutArm` | `OutPP` | `v` | Steven 20240822 : For HT-9046AU |
| 63 | `MTrayBracketZ` | 102 | `palTrayArm` | `TrayArm` | — | `v` | wei 20180702 MR |
| 64 | `MOutSortY` | 106 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 65 | `MLoaderY_CCW` | 117 | `palLoad` | `Loader` | `Loader` | `v` |  |
| 66 | `MAuto1Y_CCW` | 118 | `palAuto1` | `Auto1` | `Auto1` | `v` |  |
| 67 | `MAuto2Y_CCW` | 119 | `palAuto2` | `Auto2` | `Auto2` | `v` |  |
| 68 | `MAuto3Y_CCW` | 120 | `palAuto3` | `Auto3` | `Auto3` | `v` |  |
| 69 | `MAuto4Y_CCW` | 121 | `palAuto4` | `Auto4` | — | `v` |  |
| 70 | `MAuto5Y_CCW` | 122 | `palAuto5` | `Auto5` | — | `v` |  |
| 71 | `MAuto6Y_CCW` | 123 | `palAuto6` | `Auto6` | — | `v` |  |
| 72 | `MAuto4Z` | 144 | `palAuto4_Car` | `Auto4` | — | `v` | Steven 20230907 : For HT-9011UC |
| 73 | `MAuto5Z` | 145 | `palAuto5_Car` | `Auto5` | — | `v` |  |
| 74 | `MAuto6Z` | 146 | `palAuto6_Car` | `Auto6` | — | `v` |  |
| 75 | `MAuto4Y` | 147 | `palAuto4` | `Auto4` | — | `v` |  |
| 76 | `MAuto5Y` | 148 | `palAuto5` | `Auto5` | — | `v` |  |
| 77 | `MAuto6Y` | 149 | `palAuto6` | `Auto6` | — | `v` |  |
| 78 | `MOutSortPitchX` | 156 | `palOutArm` | `OutArm` | `OutPP` | `v` | Steven 20240822 : For HT-9046AU |
| 79 | `MOutSortSht` | 157 | `palOutArm` | `OutArm` | `OutPP` | `v` |  |
| 80 | `MLoad2Z` | 158 | `palLoad2_Car` | — | — | `v` |  |
| 81 | `MLoad2Y` | 159 | `palLoad2` | — | — | `v` |  |

## A? 級 —— 24 個（待判定）

| # | unit 常數 | 值 | 可能的 panel | 確認 | 備註 / golden 行尾註解 |
|--:|---|--:|---|:--:|---|
| 1 | `MLoadHingeR` | 83 | `palLoad` | `v` | Steven 20170330 (Wei) : For TSMC |
| 2 | `MLoadHingeZ` | 84 | `palLoad` | `v` | Steven 20170330 (Wei) : For TSMC |
| 3 | `MCCDX` | 107 | `palCCD` | `palOutSh1` | 20260924 Steven 裁定（硬體事實）：Fine Pitch CCD 裝在 Out Shuttle 1；golden 只看得到 uhome.cpp:266-268 的 USE_FINE_PITCH 閘，沒有歸屬證據 |
| 4 | `MCCDY` | 108 | `palCCD` | `palOutSh1` | 同 #3 |
| 5 | `MCCDZ` | 109 | `palCCD` | `palOutSh1` | 同 #3 |
| 6 | `MFix3Full` | 143 | `palFix3` | `v` | JimmyChiu 20220927 : Stepper Motor Control in Fix3 |
| 7 | `MInSh1LtcSenZ1` | 160 | `palInSh1` | `palInSh1` | KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料 |
| 8 | `MInSh1LtcSenZ2` | 161 | `palInSh1` | `palInSh1` |  |
| 9 | `MInSh2LtcSenZ1` | 162 | `palInSh2` | `palInSh2` |  |
| 10 | `MInSh2LtcSenZ2` | 163 | `palInSh2` | `palInSh2` |  |
| 11 | `MMHot1RecBuf` | 183 | `palPlate1` | `v` | jou 2011-12-26 加入記憶尚未完成吸取的位置 |
| 12 | `MMHot2RecBuf` | 184 | `palPlate2` | `v` | jou 2011-12-26 加入記憶尚未完成吸取的位置 |
| 13 | `MMTrayLoader` | 199 | `palLoad` | `v` | wei 20180702 MR |
| 14 | `MMTrayEmpty` | 200 | `palEmpty` | `v` | wei 20180702 MR |
| 15 | `MMTrayConversion` | 201 | `palColor` | `v` | wei 20180702 MR |
| 16 | `MMTrayAuto1` | 202 | `palAuto1` | `v` | wei 20180702 MR |
| 17 | `MMTrayAuto2` | 203 | `palAuto2` | `v` | wei 20180702 MR |
| 18 | `MMTrayAuto3` | 204 | `palAuto3` | `v` | wei 20180702 MR |
| 19 | `MMFixTray1` | 205 | `palFix1` | `v` | Steven 20100205 : 暫存Fix資料用 |
| 20 | `MMFixTray2` | 206 | `palFix2` | `v` | Steven 20100205 : 暫存Fix資料用 |
| 21 | `MMFixTray3` | 207 | `palFix3` | `v` | Steven 20100205 : 暫存Fix資料用 |
| 22 | `MMFixTray4` | 263 | `palFix4` | `v` | Steven 20100205 : 暫存Fix資料用 |
| 23 | `MMFixTray5` | 264 | `palFix5` | `v` | Steven 20100205 : 暫存Fix資料用 |
| 24 | `MMFixTray6` | 265 | `palFix6` | `v` | Steven 20100205 : 暫存Fix資料用 |

## B1 級 —— 33 個（新面板 `palMagazineTray`，已定案）

Steven 20260924 裁定：Magazine 家族不留在 C 級，**另外長出一個 `palMagazineTray`**。

### 新面板規格

| 項目 | 值 | 依據 |
|---|---|---|
| 名稱 | `palMagazineTray` | 新增 |
| Parent | `pnlTrayCar` | 與 `palAuto3_Car` 同一層 |
| 位置 | `Left=624 Top=24 Width=70 Height=120`（**與 `palAuto3_Car` 完全重疊**）| `note.dfm` 的 `palAuto3_Car` |
| Panel5 相對 y | **405** | `pnlTrayCar`(381) + 24 |
| 顯示條件 | `AUTO3_IS_MAGAZINE == 1` | `cmydef.h:3081`；`acatchtray.cpp` / `aoutarm9045.cpp` 全程用這個旗標分岔 |
| 互斥 | 顯示時 `palAuto3_Car` 隱藏 | 同一個 Auto3 位置，硬體二選一 |

⚠ **必須跟著 `palAuto3_Car` 一起搬位置。** `note.cpp:1415-1432`：
`AUTO_EMPTY_COLOR>=3` 且 `USE_OUT_SORT_ARM != eartUninstall` 時
`palAuto3_Car->Left=856`。新面板漏掉這段，9 軌機種上會停在 624 疊到 Auto6 上。

✅ **Panel5 y=405 > 355，落在內嵌 Motion View 的切線之外** ——
不會被蓋住，整機圖也不需要新增模組（`mv9045` / `mv9050` 都是 `—`）。

| # | unit 常數 | 值 | 建議 panel | 確認 | 備註 / golden 行尾註解 |
|--:|---|--:|---|:--:|---|
| 1 | `MMagazine` | 140 | `palMagazineTray` | `palAuto3` | JerryYang 20220909 : add magazine |
| 2 | `MCatchMgzTray` | 141 | `palMagazineTray` | `v` |  |
| 3 | `MMagYTrayOut` | 142 | `palMagazineTray` | `v` |  |
| 4 | `MMMagazineTary1` | 215 | `palMagazineTray` | `v` | JerryYang 20220909 : add magazine |
| 5 | `MMMagazineTary2` | 216 | `palMagazineTray` | `v` |  |
| 6 | `MMMagazineTary3` | 217 | `palMagazineTray` | `v` |  |
| 7 | `MMMagazineTary4` | 218 | `palMagazineTray` | `v` |  |
| 8 | `MMMagazineTary5` | 219 | `palMagazineTray` | `v` |  |
| 9 | `MMMagazineTary6` | 220 | `palMagazineTray` | `v` |  |
| 10 | `MMMagazineTary7` | 221 | `palMagazineTray` | `v` |  |
| 11 | `MMMagazineTary8` | 222 | `palMagazineTray` | `v` |  |
| 12 | `MMMagazineTary9` | 223 | `palMagazineTray` | `v` |  |
| 13 | `MMMagazineTary10` | 224 | `palMagazineTray` | `v` |  |
| 14 | `MMMagazineTary11` | 225 | `palMagazineTray` | `v` |  |
| 15 | `MMMagazineTary12` | 226 | `palMagazineTray` | `v` |  |
| 16 | `MMMagazineTary13` | 227 | `palMagazineTray` | `v` |  |
| 17 | `MMMagazineTary14` | 228 | `palMagazineTray` | `v` |  |
| 18 | `MMMagazineTaryTop` | 229 | `palMagazineTray` | `v` |  |
| 19 | `MMMagazineBuffer` | 230 | `palMagazineTray` | `v` |  |
| 20 | `MMBackupMagazineTary1` | 231 | `palMagazineTray` | `v` |  |
| 21 | `MMBackupMagazineTary2` | 232 | `palMagazineTray` | `v` |  |
| 22 | `MMBackupMagazineTary3` | 233 | `palMagazineTray` | `v` |  |
| 23 | `MMBackupMagazineTary4` | 234 | `palMagazineTray` | `v` |  |
| 24 | `MMBackupMagazineTary5` | 235 | `palMagazineTray` | `v` |  |
| 25 | `MMBackupMagazineTary6` | 236 | `palMagazineTray` | `v` |  |
| 26 | `MMBackupMagazineTary7` | 237 | `palMagazineTray` | `v` |  |
| 27 | `MMBackupMagazineTary8` | 238 | `palMagazineTray` | `v` |  |
| 28 | `MMBackupMagazineTary9` | 239 | `palMagazineTray` | `v` |  |
| 29 | `MMBackupMagazineTary10` | 240 | `palMagazineTray` | `v` |  |
| 30 | `MMBackupMagazineTary11` | 241 | `palMagazineTray` | `v` |  |
| 31 | `MMBackupMagazineTary12` | 242 | `palMagazineTray` | `v` |  |
| 32 | `MMBackupMagazineTary13` | 243 | `palMagazineTray` | `v` |  |
| 33 | `MMBackupMagazineTary14` | 244 | `palMagazineTray` | `v` |  |

> ⚠ `MMBackupMagazineTary1`–`14`（231–244）在 golden **只有 `cinitial.cpp:3351-3364`
> 的 `SetAlias`，其餘全樹零使用** —— 今天不會有任何路徑把它們送進 `ShowErrorUnit`。
> 一併歸到 `palMagazineTray` 無害，但要不要拆成獨立的
> `palBackupMagazineTray` 可以等它真的被用到再決定。

## B2 級 —— 42 個（畫面上沒有 panel，尚未定案）

| # | unit 常數 | 值 | 備註 / golden 行尾註解 |
|--:|---|--:|---|
| 1 | `MAOIKit` | 43 | 2014-03-04 Dell for SPIL WLP Add 5S Inspection |
| 2 | `MLightScale` | 79 |  |
| 3 | `MArmAlignment` | 82 | Steven 20240507 : 只是為了Teaching存檔方便 |
| 4 | `MPreciser` | 85 | Steven 20180212 (Wei) : 定位器 |
| 5 | `MInFlipper1` | 110 | Frank 20210612 : Flipper Function |
| 6 | `MInFlipper2` | 111 | Frank 20210612 : Flipper Function |
| 7 | `MInFlipper3` | 112 | Frank 20210612 : Flipper Function |
| 8 | `MOutFlipper1` | 113 | Frank 20210612 : Flipper Function |
| 9 | `MOutFlipper2` | 114 | Frank 20210612 : Flipper Function |
| 10 | `MOutFlipper3` | 115 | Frank 20210612 : Flipper Function |
| 11 | `MTopAOIArmX` | 150 |  |
| 12 | `MTopAOIArmY` | 151 |  |
| 13 | `MTopAOIArmR` | 152 |  |
| 14 | `MTopAOICCDZ` | 153 |  |
| 15 | `MTopAOIElevZ1` | 154 |  |
| 16 | `MTopAOIElevZ2` | 155 |  |
| 17 | `MMAutoCleanKit` | 185 | jou 2012-05-21 Auto Clean |
| 18 | `MMBulkboxKit` | 187 | kevin 20160822 |
| 19 | `MMScanAOI` | 208 | Ifor 20211026 add:ScanAOI Tray |
| 20 | `MMInArmAOATray` | 209 | KenHsieh 20210813 : add CCD AUTO ALIGNMENT |
| 21 | `MMOutArmAOATray` | 210 |  |
| 22 | `MMAOASampleTray` | 211 |  |
| 23 | `MMAOASamplePlate` | 212 |  |
| 24 | `MInPlacementX` | 213 | JimmyChiu 20220908 add Pickup Error Placement |
| 25 | `MInPlacementY` | 214 |  |
| 26 | `MMDailyCorrelationKit` | 274 | KaiHuang 20200606 : For ASE-CL Daily Correlation |
| 27 | `MMAutoClean` | 508 | kevin 20120518 autoclean |
| 28 | `MMSafeDoor11` | 520 |  |
| 29 | `MMSafeDoor12` | 521 |  |
| 30 | `MMSafeDoor13` | 522 |  |
| 31 | `MMSafeDoor14` | 523 |  |
| 32 | `MMSafeDoor15` | 524 |  |
| 33 | `MMSafeDoor16` | 525 | JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT |
| 34 | `MMSafeDoor17` | 526 |  |
| 35 | `MMSafeDoor18` | 527 |  |
| 36 | `MMSafeDoor19` | 528 |  |
| 37 | `MMSafeDoor20` | 529 |  |
| 38 | `MMSafeDoor21` | 530 |  |
| 39 | `MMSafeDoor22` | 531 |  |
| 40 | `MMMultileEmpty` | 571 |  |
| 41 | `MMMultileEmpty_Catch` | 572 |  |
| 42 | `MMMultileEmpty_Z` | 573 |  |

## C 級 —— 50 個（維持 `palSys`）

這些是本機不存在的選配機構（CA Buffer、LoadPort/UnloadPort、
`M1_1X`..`M1_8R`、Caselevator、StackedTray、UnloadRobot 等）。
報警送 `palSys` 是**正確的**，不要為它們新增面板。

| # | unit 常數 | 值 | 備註 / golden 行尾註解 |
|--:|---|--:|---|
| 1 | `MCaselevatorZ` | 99 | wei 20180702 MR |
| 2 | `MCasArmX` | 100 | wei 20180702 MR |
| 3 | `MCasArmZ` | 101 | wei 20180702 MR |
| 4 | `MStackedTrayX` | 103 | wei 20180702 MR |
| 5 | `MStackedTrayZ` | 104 | wei 20180702 MR |
| 6 | `MUnloadRobotZ` | 105 | Sam 20190112 LM |
| 7 | `M1_1X` | 116 |  |
| 8 | `MLdCarRotArm` | 116 | RogerYang 20250828 add for Loader Rotate Arm |
| 9 | `M1_1Y` | 117 |  |
| 10 | `M1_1R` | 118 |  |
| 11 | `M1_2X` | 119 |  |
| 12 | `M1_2Y` | 120 |  |
| 13 | `M1_2R` | 121 |  |
| 14 | `M1_3X` | 122 |  |
| 15 | `M1_3Y` | 123 |  |
| 16 | `M1_3R` | 124 |  |
| 17 | `M1_4X` | 125 |  |
| 18 | `M1_4Y` | 126 |  |
| 19 | `M1_4R` | 127 |  |
| 20 | `M1_5X` | 128 |  |
| 21 | `M1_5Y` | 129 |  |
| 22 | `M1_5R` | 130 |  |
| 23 | `M1_6X` | 131 |  |
| 24 | `M1_6Y` | 132 |  |
| 25 | `M1_6R` | 133 |  |
| 26 | `M1_7X` | 134 |  |
| 27 | `M1_7Y` | 135 |  |
| 28 | `M1_7R` | 136 |  |
| 29 | `M1_8X` | 137 |  |
| 30 | `M1_8Y` | 138 |  |
| 31 | `M1_8R` | 139 |  |
| 32 | `MMCABuffer1` | 188 | wei 20180702 MR |
| 33 | `MMCABuffer2` | 189 | wei 20180702 MR |
| 34 | `MMCABuffer3` | 190 | wei 20180702 MR |
| 35 | `MMCABuffer4` | 191 | wei 20180702 MR |
| 36 | `MMCABuffer5` | 192 | wei 20180702 MR |
| 37 | `MMCABuffer6` | 193 | wei 20180702 MR |
| 38 | `MMCABuffer7` | 194 | wei 20180702 MR |
| 39 | `MMCABuffer8` | 195 | wei 20180702 MR |
| 40 | `MMCABuffer9` | 196 | wei 20180702 MR |
| 41 | `MMCABuffer10` | 197 | wei 20180702 MR |
| 42 | `MMLoadPort` | 198 | wei 20180702 MR |
| 43 | `MMLoadPort1` | 266 | Sam 20190112 LM |
| 44 | `MMLoadPort2` | 267 |  |
| 45 | `MMLoadPort3` | 268 |  |
| 46 | `MMLoadPort4` | 269 |  |
| 47 | `MMUnloadPort1` | 270 |  |
| 48 | `MMUnloadPort2` | 271 |  |
| 49 | `MMUnloadPort3` | 272 |  |
| 50 | `MMUnloadPort4` | 273 |  |

