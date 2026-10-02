# cprod.h 陣列溢位風險稽核（Prod / 全域結構）

> 稽核基準：`HT9011UC_Code_V3.33.905.8_20260616_Ken_Ifor_Jimmy_JerryYang_kevin_RogerYang`
> 日期：2026-06-16　方法：READ-ONLY 稽核，未修改任何程式碼
> 範圍：`cprod.h` 內所有結構（`PROD_INFO_ST`/Prod、`SYSTEM_DEVICE_FORM`、`SYSTEM_TRAY_FORM`、
> `SYSTEM_TEMPERATURE`、`SYSTEM_TEST_IF`、`SYSTEM_TEST_MODE`、`SYSTEM_BIN_SELECT`、`RUN_INFO`、
> `ARM_CONDITION`、`SHUTTLE_THREAD`、`INVISIBLE_OFFSET`、`TAutoTeachOffset`、`ARM_OFFSET` 等）
> 宣告的陣列，且在專案 `.cpp` 中以**變數索引**實際存取者。

---

## 結論摘要

| 嚴重度 | 數量 |
|--------|------|
| Critical | 0 |
| High | 0 |
| Low（設定/存取器強化建議） | 2 |
| SAFE-but-checked（已追蹤確認在界內） | 其餘所有變數索引陣列 |

約 120 個陣列成員經檢視，變數索引者逐一在 `.cpp` 追蹤索引來源範圍。
首輪宣告/設定值比對**未發現**可達的寫越界（最強候選 `iRunStartMode` 索引 `[2]` 已被推翻）。
但**第二輪「迴圈硬編碼上限 vs 陣列宣告大小」交叉比對**（見下方 §F）另揪出 2 個寫越界缺陷，
第三輪 **block-memory（memcpy/strcpy）掃描**（見 §G）找出 3 個 Medium 風險。

> ⚠️ 教訓：首輪只比對「陣列宣告 vs 設定值綁定」，**漏掉迴圈邊界 off-by-one**（`for(i<N)` 寫進 `[M]` 且 N>M）。
> 陣列稽核**必須**包含「迴圈上限 vs 索引陣列宣告」與「block-memory size vs 目標大小」兩類比對，缺一不可。

---

## §F. 迴圈邊界 vs 陣列宣告 交叉稽核（2026-06-16 第二輪）

方法：掃描所有 `for(...; i<N; ...)` 迴圈，將硬編碼/常數上限 N 回頭比對迴圈體內以 `i` 索引之陣列宣告大小。

### 🔴 [Critical] F-1 — iAutoCassette 系列（1 維寫+讀越界）【已修正】

| 項目 | 內容 |
|------|------|
| 迴圈 | cinitial.cpp L10384 原 `for(int i=0; i<3; i++)` |
| 陣列 | `iAutoCassetteFront/FrontBack/Rear/RearBack/ZStart`：cprod.h L1128-1134（Prod）+ LastSet.h L1042-1046（Tech）皆 `int [2]` |
| 越界 | i=2 寫越界（污染相鄰 Prod 成員）+ 讀越界（取 Tech 垃圾值），共 5 條陣列 |
| LastSet | Tech 為持久化結構（本處只讀，無破壞）；Prod 為 runtime |
| **修正** | ✅ JerryYang 20260616 改 `i<2`（註解 `3 --> 2`），與 `[2]` 宣告一致、不動 binary layout |

### 🟠 [High] F-2 — iInArm/iOutArmZBasePickerAlignmentPos（2 維寫越界，Prod/Tech 維度不一致）【已修正】

| 項目 | 內容 |
|------|------|
| 迴圈 | cinitial.cpp L11372 原 `for(i<2){ for(j<8) }`（限 `MACHINE_HAS_AUTO_ALIGNMENT_CCD==true`） |
| 陣列 | Prod：cprod.h L1028/L1040 `[MAX_ARM_Row][MAX_ARM_Col]`=**[2][4]**；Tech：LastSet.h L968/L969 `[2][8]`（Steven 20230801 升級為 2 維） |
| 越界 | Tech 端 2023 已升 `[2][8]`，但 Prod 端**漏同步**仍 `[2][4]`。j 走 4~7 時 row-major flat offset 超出 8-int 配置 → 寫越界 4 個 int（破壞後續 `dInArmCCDXResolution` 等 double） |
| LastSet | Prod 為 runtime（非持久化）；Tech 只讀 |
| **修正** | ✅ Steven 20260616 改 `j<MAX_ARM_Col`（=4），j 不再超出 Prod `[2][4]`（縮迴圈上限的次選方案；Tech `[2][8]` 只讀前 4 欄，符合 Prod 設計）。記憶體安全 |

> 備註：F-2 另一可選修法是把 Prod 兩條陣列擴為 `[2][8]` 與 Tech 對齊（Prod 非持久化，不影響存檔）。實際採縮迴圈方案。

### 🟢 [Low] F-3 — iInSHSen9DetectPos2x5 / 2x6（iStep off-by-one，記憶體安全）

- 位置：ShuttleMove.cpp L1863 `for(i<MAX_Index_Col) if(i<=iStep)`。
- 其他模式 `iStep=欄數-1`（1x2→1、2x3→2、1x4→3、2x8→7），但 **2x5→iStep=5、2x6→iStep=6**（應為 4、5），多算一格。
- 因只存取 `[0][i]`，超出的 `[0][5]`/`[0][6]` flat offset 落在 `[1][0]`（仍在 2 維連續配置內）→ **非記憶體越界，僅邏輯讀到相鄰列**。
- 建議複查 2x5/2x6 的 iStep 是否應為 4/5（屬潛在邏輯瑕疵，未修）。

---

## §G. block-memory（memcpy/memset/strcpy）size 稽核（2026-06-16 第三輪）

方法：掃描 `memcpy/memmove/CopyMemory/memset/ZeroMemory/strcpy/strncpy/strcat/sprintf` 的 size 引數 vs 目標緩衝宣告大小。0 Critical / 0 High / 3 Medium / 2 Low。

### 🟡 [Medium] G-1 — Command.cpp:10491 GPIB 訊息 strncpy 未以目標大小設限

```cpp
strncpy(HHandler2Gpib.Message, Str.c_str(), Str.Length());  // 以來源長度為界，非 sizeof(Message)
```
- 目標 `char Message[2048]`（MessageDef.h struct MV）。`Str` 由多筆 bin/mapping/2D 欄位 sprintf 串接。
- 若 `Str.Length()>2048` → 寫越界覆蓋同結構後續 `UseSiteMapData[256]`/`asATC_TYPE[32]`/`MultiMessage[4096]`。非 LastSet（IPC 結構）。
- 修法：`strncpy(..., sizeof(HHandler2Gpib.Message)-1)` + 補 `Message[2047]=0`，或組裝後檢查上限。

### 🟡 [Medium] G-2 — cShowBinSelect.cpp:2336 UI 文字 strcpy 進 LastSet 固定陣列

```cpp
strcpy(LastSet.szJamClearData[0], fMain->StatusBar1->Panels->Items[6]->Text.c_str());  // 不設限
```
- 目標 `char szJamClearData[3][64]`（LastSet.h:191）→ 單格 64 bytes。來源為 StatusBar Panel 文字（執行期長度）。
- 若顯示字串 >63 字元 → 寫越界。**目標為 LastSet 持久化成員**，溢位會污染存檔狀態。
- 修法：`strncpy(..., 63)` + 補 `[63]=0`。

### 🟡 [Medium] G-3 — BarCode.cpp:8590 2D 條碼（稽核工具誤報，實際已是 strncpy）

```cpp
// 實際程式碼（稽核工具誤判為 strcpy）：
strncpy(mtShowBarcodeTray[iMot][x][y], MOT[iMot].Tray.cDeviceInf[x][y].c_str(), sizeof(mtShowBarcodeTray[iMot][x][y]));
```
- **溢位風險：不存在**。`strncpy` + `sizeof(24)` 已正確限制，不會寫越界。`strncpy` 在來源較短時自動補 `\0`，**不存在殘留字元**問題。
- **殘餘隱患（Low）**：若 2D ID ≥ 24 字元，`strncpy` 填滿 24 bytes 但不補結尾 `\0`，後續 `SetCellNumber` 把 `char*` 當字串使用會 read-past-end 至下一個 `\0`（顯示亂碼）。修法：加 `mtShowBarcodeTray[iMot][x][y][23] = 0;` 確保 null-termination。
- 本項已由使用者確認，從 Medium 降為 **Low**，暫不修改，由負責人評估後決定。非 LastSet。

### 🟢 [Low] G-4 — BarCode.cpp:1791/1850/1921/1997 邊界值缺結尾 null

- `strncpy(cStr1, data, BufferLength)`，`cStr1[1024]`；上方 `BufferLength>1024 return` 已防寫越界。
- 隱患：`BufferLength==1024` 恰好填滿無結尾 null → 後續字串讀取 read-past-end。修法：上限改 `>=1024` 或寫後 `cStr1[1023]=0`。

### 🟢 [Low] G-5 — adam6024.cpp C-style sprintf（固定格式）

- L147/172/200/225/2143/2165 為固定格式 `sprintf`，風險低；緩衝宣告未逐一核對，列入後續例行檢查。

### block-memory 已排除（false positive）重點

| 類型 | 說明 |
|------|------|
| 同型別整段複製 | `memcpy(&Foo.first, &Foo_File.first, sizeof(Foo_File))` 兩者同結構型別、size 成員為首成員 → 等同整個結構複製（cUnitConvert.cpp 多處、cinitial.cpp:7001/8513、main.cpp:24360） |
| TEST_CATEGORY 整段複製 | ProductionInfo.cpp:4765/4795 `iCountCategory` 為 TEST_CATEGORY 首成員，同型別 → 安全 |
| SECS 網路 memcpy | uHGemEquipment.cpp:9120 三重保護（100MB 上限 + 來源長度驗證 + 依長度精確 new） → 安全 |
| 邊界恰好相等 | csystem.cpp:8086 `szBundleTrayID_ATK_Backup[3][256]` strncpy 255+補 0；fVATMesFileSys.cpp:1873 `cLotStartTimeForAlways[20]` 寫 19+null → 皆恰好相等，安全（含 LastSet） |
| VCL AnsiString.sprintf | `str.sprintf(...)` 為 AnsiString 成員方法（自管緩衝），非 C stdlib → 不列入風險 |

### block-memory 掃描限制（誠實揭露）

1. Medium-1/2/3 是否實際溢位取決於 GPIB 訊息/UI Panel/2D 條碼的**執行期字串長度**，靜態分析無法保證觸發。
2. 未完全靜態驗證：Command.cpp:8562/9453/12664/12732 等 `cBuffer`/`cGetXxx_Cmd` 緩衝宣告未在標頭檔搜得（疑為 .cpp 區域或 macro 間接定義），建議後續追查。
3. Motor/EtherCAT/CCLink 等第三方驅動標頭未納入掃描（依專案約束不修改）。

---

## 解析後的邊界常數（MachineType.h / cmydef.h）

| 常數 | 值 | 來源 |
|------|----|----|
| `MAX_ARM_Row` | 2 | MachineType.h L382 |
| `MAX_ARM_Col` | 4 | MachineType.h L383 |
| `MAX_Index_Row` | 2 | MachineType.h L384 |
| `MAX_Index_Col` | 8 | MachineType.h L385 |
| `MAX_SOCKET_ROW` | 4 | MachineType.h L386 |
| `MAX_SOCKET_COL` | 8 | MachineType.h L387 |
| `MAX_AUTO_TRAY` | 6 | MachineType.h L396 |
| `MAX_TRACK` | 9 | MachineType.h L397 |
| `MAX_FIX_TRAY` | 6 | MachineType.h L398 |
| `eTrayCount` | 33 | MachineType.h（eAuto1=0..eMag14=32） |
| `eFix6 / eFix12` | 11 / 17 | MachineType.h L1077 / L1083 |
| `TEST_MAX_BIN` | 256 | — |
| `iSnSocketCnt` | 24 | cmydef.h L1001（JerryYang 20260506：16→24） |
| `RT / FT` | 0 / 1 | cprod.cpp L111-112 |

---

## A. 風險候選 / 維度不對稱（Low）

| # | 陣列 | 宣告 | 結構/全域 | 存取位置 | 索引變數與範圍 | 風險類型 | 嚴重度 |
|---|------|------|-----------|----------|----------------|----------|--------|
| 1 | `iSocketSensor[24]` | `int [24]`（=`iSnSocketCnt`） | `SHUTTLE_THREAD SThreadPara` | atester.cpp L4372 / L4590 / L6189、atester_32Site.cpp L1374 | `i < TestIF_File.iSocketCount`；`iSocketCount` 來自 INI「SocketCountt」（預設 8），只有下限夾擠（>=4 / >=2），**無上限夾擠到 24** | 外部（INI）值缺上界保護 | Low |
| 2 | `dXPitch[4]` | `double [4]` | `ARM_OFFSET`（`GetXPitch(int iX)`） | `return dXPitch[iX];`（cprod.h 存取器） | 由呼叫端傳入 `iX`，存取器內部不夾擠 | accessor 無防護 | Low |

> 註：本次掃描未找到 `GetXPitch` 的越界呼叫端；存取器本身信任呼叫端。
> `iSocketSensor` 的填值迴圈在 cinitial.cpp L15093 使用 `i<iSnSocketCnt`(24)；只有當 INI `SocketCountt>24`（32-site 誤編 INI）時才會讀越界，屬設定邊界問題。

---

## B. SAFE-but-checked（已追蹤確認在界內）

| 陣列 | 維度 | 判定安全的理由 |
|------|------|----------------|
| `LoaderToEmptyColor[2]`、`AutoFromEmptyColor[2][33]` | [2]、[2][33] | 第一維 = 全域 `iRunStartMode`，只會是 RT(0)/FT(1)/init 0 → [0,1]。第二維 `iWhichAuto-1`/`AutoTarget`/`eAuto2` ≤ 5 < 33。（acatchtray.cpp L1971/2089/2472/2716/4178） |
| `XStart[33][2][4]`、`YStart`、`ZPlace`、`ZPick` | [33][2][4] | 第一維 = tray id（iOutPutTray/iWhichAuto）< 33；`[i][j]` 迴圈受 `i<MAX_ARM_Row`(2)、`j<MAX_ARM_Col`(4) 限制（aoutarm9045S_1x4_4.cpp L976-997）。`[iOutArmYBase][iOutArmXBase]` 為臂基準軸（0..1 / 0..3）。 |
| `iDutOnOff[2][4][8]`、`iDutOnOffEE[2][4][8]` | [2][4][8] | `[0/1][i][j]`，socket row/col 迴圈對齊 MAX_SOCKET_ROW(4)/COL(8)（cprod.cpp L1556-1597）。 |
| `iTempShiftOffset[3][30]`、`fTempShiftOffset[3][30]` | [3][30] | `for(i<3)` 寫入（cOffSet.cpp ~L3654）。 |
| `fTempShiftOffsetRatio[2][30]`、`fTempShiftScaleRatio[2][30]` | [2][30] | 寫 `[0][i]`/`[1][i]`，`for(i<30)`。第一維 [2] vs [3] 為設計差異（ratio = 2 段）。 |
| `iMagazineTrayPos[15]` | [15] | 僅常數索引 0-14（cinitial.cpp L10339-10364）。 |
| `iXTrayAuto[6]`、`iXTrayAuto_ART[6]` | [6] | 索引 iWhichAuto/AutoTarget = 實體 auto-tray slot，與 MOT[MMAuto1_Car+iWhichAuto]/iMMAuto[AutoTarget] 平行，受 MAX_AUTO_TRAY(6) 限制（acatchtray.cpp L4079/L8861）。 |
| `RotateDutDate[2][4][8]` | [2][4][8] | `[0/1][iSuckRow][iSuckCol]`，suck row/col 在 [4]/[8] 內。 |

---

## C. 維度不對稱觀察（非缺陷，列為注意）

- **Arm 維 [2][4] vs Socket 維 [4][8]**：`MAX_ARM_Row/Col`(2×4) 小於 `MAX_SOCKET_ROW/COL`(4×8)。
  位置陣列（`XStart` 等）以臂幾何 [2][4] 索引、迴圈以 arm 維為界，**並非**以 socket 維索引，故安全。
  新增程式碼若改用 socket row/col 直接索引 arm 維陣列，將造成越界 → 為日後高風險點，須留意。
- **`fTempShiftOffsetRatio[2][30]` vs `iTempShiftOffset[3][30]`**：第一維 2 與 3 不同，屬設計差異（ratio 僅 2 段），非缺陷。

---

## D. 建議強化（未經核准請勿套用）

> 兩項皆為防禦性強化，非現存缺陷。

1. **`iSocketCount` 上限夾擠** — 於 cSetUp.cpp 下限夾擠後（~L2606-2622）加：
   ```cpp
   if(TestIF_File.iSocketCount > iSnSocketCnt) TestIF_File.iSocketCount = iSnSocketCnt;  //add upper guard
   ```
   防止 32-site 機台誤編 INI。
2. **`ARM_OFFSET::GetXPitch(int iX)` 存取器夾擠**：
   ```cpp
   if(iX<0) iX=0; else if(iX>=4) iX=3;  //add boundary guard
   return dXPitch[iX];
   ```

---

## E. 稽核方法（複用 LastSet 流程）

每個候選陣列：
1. 記錄宣告：名稱、元素型別、維度（常數解析為數值）。
2. grep 在專案 `.cpp` 的使用點，聚焦變數索引存取。
3. 追蹤索引變數可能範圍（迴圈邊界、enum 來源、tester/bin/tray 回傳值、滑鼠/grid 事件、DLL/外部回傳）。
4. 當索引範圍可能超界、off-by-one（idx±1/idx+2 近邊界）、外部/負值缺保護、enum-count 不匹配、`||`/`&&` 方向錯時，標記為 RISK。
5. 分級 Critical / High / Low。

---

## 版本紀錄

| 版本 | 異動 |
|------|------|
| V3.33.905.8（2026-06-16）第三輪 | 新增 §G block-memory（memcpy/strcpy）掃描：0 Critical / 0 High / 3 Medium（Command.cpp:10491、cShowBinSelect.cpp:2336、BarCode.cpp:8590）/ 2 Low。 |
| V3.33.905.8（2026-06-16）第二輪 | 新增 §F 迴圈邊界交叉稽核：揪出 F-1 iAutoCassette（Critical，已修 `i<2`）、F-2 iInArm/iOutArmZBasePickerAlignmentPos（High，已修 `j<MAX_ARM_Col`）、F-3 iInSHSen9DetectPos2x5/2x6（Low）。 |
| V3.33.905.8（2026-06-16）首輪 | 首次 cprod.h 全陣列稽核（宣告/設定值比對）：0 Critical / 0 High / 2 Low。 |
