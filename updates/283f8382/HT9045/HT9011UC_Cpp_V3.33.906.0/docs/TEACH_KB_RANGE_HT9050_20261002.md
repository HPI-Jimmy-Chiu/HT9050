# Teach 頁小鍵盤上下限：HT9050 機台實測（2026-10-02，機台端，唯讀）

> 給：EastSun（裁決）、筆電端（GitLab INBOX 140 的修正）。
> 起因：筆電第 119 包的安全提醒「網頁 Teach 頁幾乎沒有教導值範圍保護，golden 會擋」。
> 這份回答的是另一半：**這台的教導值放進 golden 的範圍，會發生什麼事。**

## 結論

**在這台 HT9050 上照 golden 補上 Teach 小鍵盤範圍，會把正確的教導值改掉。不要照原樣套用。**

- 這台的機型代號解成 `Type_HT9046_LS`（`database.cpp:517`，9050GPIB，使用者 0926 裁決），
  所以 golden `TfTeach::setEditLimitClick`（`uteach.cpp:3728-4250`）會選 **9046LS 那一臂**的範圍。
- 這台**正在用的 11 個教導位置**（非 0）落在 9046LS 範圍**外面**；其中 **10 個剛好落在 golden 的 HT9045 那一臂**裡。
  ⇒ 這台的機構尺寸比較像 HT9045，不像 9046LS（至少 Teach 的 X／Y 範圍是這樣）。
- golden 小鍵盤超出範圍時**不擋、不提示，直接夾到邊界**（`myQwertyKeyBoard.cpp:285-291` `CheckRange`），
  而且**按 Cancel 也會夾**：Cancel 先把原值放回（`:364`），`ShowModal` 回來後一樣跑 `CheckRange`。
  ⇒ 套用 9046LS 範圍後，操作員**只要點一下**這 11 個欄位（就算按 Cancel），
  例如 Auto1 Y 就會從 -55618 變成 -60000（差 4.4 mm），存檔後機台照那個值跑。

## 這台的分支值（實測）

| 變數 | 值 | 來源 |
|---|---|---|
| `MachineTypeChoice` | `Type_HT9046_LS` | `D:\GPIB9045\system\general.ini` `Model=9050GPIB` → `database.cpp:517` |
| `CUSTOMER_CODE` | 957（`CC_PTI`） | `D:\HT9045\system\Gerneral.ini` |
| `AUTO_EMPTY_COLOR` | 0 | 同上 |
| `USE_IN_OUT_ARM_Y_PITCH` | 0（`iXPitch60`） | 同上 |
| `USE_HOTPLATE_TYPE`／`HOT_PLATE_POSITION`／`FIX3_FULL_PLACE` | 0／0／0 | 同上 |
| `USE_PICKER_COUNT` | 1 | 同上 |
| `USE_INDEX_ARM_AXES` | 0（`IndexArm_4_Axis`） | 同上 |
| `Top_Scanner_AOI`／`SubModel` | 0／0 | 同上 |
| 馬達軟體極限 | 有列的軸多半 -999999～999999 | `D:\HT9045\system\Mot_Table.csv`（48 列） |
| 教導值 | — | `runcfg\system\teach.ini`（F5 的 `W906_TEACH_INI_PATH`，10/01 21:57 存過） |

## 做法

golden `uteach.dfm` 裡每一個 `TEdit` 的 `OnClick`（**523 欄、40 個處理函式**，全部列舉，沒有抽樣），
照 golden 原文算出這台會走的範圍，再拿 `teach.ini` 的現值去比（欄位到 ini 鍵的對照用 `ht9045_wire_hwteach.js` 的 sysFields）。
處理函式分兩種：
- **固定數字**：`setEditLimitClick`（101 欄，依機型／設定分支）、`setEditZ1AClick`（92 欄，100～-3000）、
  `SetEditPickLoaderClick`（23 欄，-500～-3000）等。
- **該軸的軟體極限**：`setEditInXClick` 等 30 個，用 `MOT[軸].Motor->PSoftLimitP／N`。

## 結果（523 欄）

| 狀態 | 欄數 |
|---|---|
| 現值在範圍內 | 194 |
| **現值在範圍外** | **41**（非 0 的 11、值為 0 的 30） |
| ini 沒有這個鍵／沒有對照 | 213 |
| 軸不在 Mot_Table | 63 |
| golden 這台不開小鍵盤（`AUTO_EMPTY_COLOR<3`：Fix4X／Fix5X／Fix6X／Auto4X／Auto5X） | 5 |
| 執行期才知道（`fIndexDownPos`、`CosFunction.iLimitMaxSpeed`、選取的馬達） | 7 |

### 非 0、在 9046LS 範圍外的 11 個（這台真的在用的位置）

| 欄位 | 現值 | 9046LS 那一臂（這台會選的） | HT9045 那一臂 | 在 HT9045 臂內？ |
|---|---|---|---|---|
| `setEditLoaderY` | -55884 | -70000～-74000（`:3781`） | -53000～-100000（`:3793`） | ✅ |
| `setEditInSht1Y` | -37718 | -41000～-55000（`:3871`） | -35000～-41000（`:3875`） | ✅ |
| `setEditOutSht1Y` | -37595 | -48000～-55000（`:3971`） | -35000～-41000（`:3976`） | ✅ |
| `setEditFix1X` | -35657 | -40000～-46000（`:4026`） | -33000～-39000（`:4035`） | ✅ |
| `setEditFix2X` | -21371 | -25000～-32000（`:4049`） | -18000～-24000（`:4058`） | ✅ |
| `setEditFix3X` | -7061 | -11000～-20000（`:4072`） | -4000～-10000（`:4081`） | ✅ |
| `setEditAuto1Y` | -55618 | -60000～-74000（`:4112`） | -53000～-59000（`:4116`） | ✅ |
| `setEditAuto2Y` | -55633 | -60000～-74000（`:4112`） | -53000～-59000（`:4116`） | ✅ |
| `setEditAuto3Y` | -55601 | -60000～-74000（`:4112`） | -53000～-59000（`:4116`） | ✅ |
| `setOutPickY` | -57881 | -60000～-74000（`:4112`） | -53000～-59000（`:4116`） | ✅ |
| `setOutPickX` | -53548 | -12000～-19000（`:4169`） | -13000～-19000（`:4174`） | ❌ 兩臂都不在 |

`setOutPickX` 在 golden 和 `setEditAuto3X` 共用範圍；這台的值在 -53548，兩臂都差很遠。

### 值為 0、在範圍外的 30 個（這台大概沒用到的位置）

點一下就會被夾成邊界值（例如 0 → -500）：
`setEdtHP1LaserY`（-46000～-53000）、`SetEditPickInRotate`（-500～-3000）、`SetEditPlacePreciserZ`（-500～-3000）、`SetEditPlaceInRotate`（-500～-3000）、`SetEditLDCassetteZStart`（-500～-3000）、`SetEditLDFrontBack`（-500～-3000）、`SetEditLDFront`（-500～-3000）、`SetEditLDRearBack`（-500～-3000）、`setEditFix6Y`（-7000～-20000）、`setEditFix4Y`（-7000～-20000）、`setEditFix5Y`（-7000～-20000）、`SetEditPlaceOutRotate`（-500～-3000）、`SetEditPlaceFix2`（-500～-3000）、`edtBinBoxZ`（-500～-3000）、`SetEditPickOutRotate`（-500～-3000）、`SetEditAuto1Front`（-500～-3000）、`SetEditAuto1FrontBack`（-500～-3000）、`SetEditAuto1Rear`（-500～-3000）、`SetEditAuto1RearBack`（-500～-3000）、`SetEditAuto1CassetteZStart`（-500～-3000）、`setEditScannerAOIX`（-13000～-16000）、`setEditScannerAOIY`（-17000～-20000）、`setEditTopViewX`（-13000～-16000）、`setEditTopViewY`（-30000～-35000）、`setEditPADViewX`（-13000～-16000）、`setEditPADViewY`（-23000～-27000）、`setEditBGAViewX`（-13000～-16000）、`setEditBGAViewY`（-17000～-20000）、`setEditSafePosX`（-13000～-19000）、`setEditSafePosY`（-10000～-15000）

### 軸不在 Mot_Table 的 63 欄（範圍來自軟體極限，這台沒有那一軸）

MInRotateKit（18 欄）、MInSh1LtcSenZ1（1）、MInSh2LtcSenZ1（1）、MInSh1LtcSenZ2（1）、MInSh2LtcSenZ2（1）、MFix3Full（2）、MOutSortPitchX（2）、MOutSortSht（2）、MLoadHingeR（2）、MCasArmX（1）、MCasArmZ（10）、MCaselevatorZ（3）、MStackedTrayX（6）、MStackedTrayZ（6）、MTrayBracketZ（2）、MUnloadRobotZ（5）。
移植樹對「表裡沒有的軸」的 `PSoftLimitP／N` 是多少沒有查；如果是 0／0，套用後那些欄位一點就變 0。

## 給筆電端（INBOX 140）

修產生器／加伺服器端交叉檢查時，**這台會選 9046LS 那一臂**。照 golden 原樣送到這台，上面 11 個真實位置會被夾掉。
機台端整合筆電包時會先用這份的方法重量一次；**範圍會改變現值的包，不會直接套**，會先問 EastSun。

## 要 EastSun 決定的

1. Teach 範圍在 HT9050 上要用哪一套：
   - **A（建議）**：先不照 golden 夾。只保留軟體極限那 30 個處理函式（這台多半是 ±999999，等於沒限制），固定數字的那批不套。
     之後 Mot_Table 的軟體極限設成這台真實的行程，就自動有保護。
   - B：固定數字照 golden 的 **HT9045 臂**（11 個裡有 10 個在範圍內；`setOutPickX` 還是會被夾）。
   - C：照 golden 的 9046LS 臂（**會改掉上面 11 個值**，不建議）。
2. 這台的 `MachineTypeChoice` 照 0926 裁決解成 9046LS；但 Teach 範圍顯示這台的尺寸更接近 HT9045。
   要不要讓 Teach 範圍（或更多地方）另外判 HT9050，是更大的題目，記在這裡讓你知道。

沒有改任何程式碼、沒有寫任何機台檔案。
