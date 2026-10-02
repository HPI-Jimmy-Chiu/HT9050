# 取放料演算法（對照程式碼）

模板裡每一條配位規則都對得上 `HT9011UC_Code_V3.33.910.0` 的實作。
**以程式碼為主，模擬畫面／影片只是佐證。**

## 目錄

- [1. Tray 取料](#1-tray-取料)
  - [1.4 ⚠ Fixed 模式 ＝ One by one](#14--fixed-模式--one-by-one一次只吸一顆)
- [2. HotPlate 放料（滾動游標）](#2-hotplate-放料滾動游標)
  - [2.1 ⚠ 滿手時不要走 Row=1](#21--滿手時不要走-row1實測缺陷)
  - [2.2 水位：用探位不要用格數](#22-水位用探位不要用格數)
- [3. HotPlate 取料](#3-hotplate-取料)
- [4. Kit / Socket / 出料](#4-kit--socket--出料)
- [5. Shuttle 三站雙 Kit](#5-shuttle-三站雙-kit)
- [6. 幾何上的硬限制](#6-幾何上的硬限制)

---

## 1. Tray 取料

### 1.1 鐵則：吸滿才走

**InArm 一定要把吸嘴吸滿才去放料。** 只有 Tray 已空、真的湊不滿時才允許部分放料（tray end）。
一趟放料可以拆成多個放下動作（上下排分拆），但「該趟第一個放料動作」時吸嘴必須是滿的。

### 1.2 位置計算

`ainarm9045.cpp` `GetInArmToLoaderPosition(iSelRow, &iXPos, &iYPos, iRow, iCol)`：

```c
iYPosition = iRow;                          // ★ 純量：一次只有一個盤列
iXPosition[0] = iCol;                       // iPickCol==4 時
iXPosition[1] = iCol + 1*iInArmXStep;
iXPosition[2] = iCol + 2*iInArmXStep;
iXPosition[3] = iCol + 3*iInArmXStep;
```

- `iInArmXStep` = 一次跳幾個盤欄
- `iYPosition` 是純量 → **Tray 端一次只能下一排吸嘴**（除非有 Y 變距軸且 Y 倍數為整數）
- `GetInArmToLoaderPosition_Single()` 是單支吸嘴版：它會把後面的吸嘴
  `InArmSuckUse[iSelRow][j] = false` 全部關掉 —— 尾欄湊不成群組時走這條

### 1.3 間距倍數的選法

Fixed 模式（`One by one = 1`）的碼是 first-fit：

```c
for(int i=1; i<UserDefForm[Ld].XDivision; i++){
    iInArmXStep = i;
    InArmClose_PitchX = UserDefForm[Ld].XPitch * iInArmXStep;
    if(InArmClose_PitchX >= iXpitchMin && InArmClose_PitchX <= iXpitchMax){
        dInArmXPitch_1Step     = InArmClose_PitchX;
        dInArmXPitch_MovePitch = InArmClose_PitchX * (吸嘴數-1);
        return InArmClose_PitchX;
    }
}
```

Open/Close 模式（`One by one = 0`，實機常態）**每次下針可重新選間距、中間允許空格**。
模板的選擇規則（可調）：

1. 這一針能吸最多支
2. 同支數取**較小**間距（行程短、殘留欄位規則交錯）
3. Y 取最小倍數

### 1.4 ⚠ Fixed 模式 ＝ One by one，一次只吸一顆

**不要把 Fixed 理解成「一次多支、只是間距鎖定」** —— ini key 本身就叫 `One by one`
（`cSpeed.cpp:497` 讀、`:1582` 寫；`rgInArmPitch->ItemIndex` 1 = Fixed = `bVariModeFIX=true`）。

```c
// ainarm9045.cpp  AdjustInArmClosePitchCondition()
if(ArmSpeed[InArm].bVariModeFIX==false)             // Open/Close
     bCanPick2ICAtOnceTime = (iResult==-1) ? false : true;
else bCanPick2ICAtOnceTime = false;                 // ★ Fixed 一律一次一顆

// ainarm9045.cpp  MoveInArmXYToLoader_9045()  L5696
if(bCanPick2ICAtOnceTime==true) GetInArmToLoaderPosition(...);
else                            iMovePitchX=GetInArmToLoaderPosition_Single(...);  //one by one suck

// ainarm9045.cpp  GetInArmToLoaderPosition_Single()  L5219
for(int i=0;i<4;i++) if(InArmSuckUse[iSelRow][i]){
    iCurrSuck=i;
    for(int j=i+1;j<4;j++){
        InArmSuckUse[iSelRow][j]  =false;
        InArmSuckUse[1-iSelRow][j]=false;           // ★ 連另一排也關掉
    }
    break;
}
```

另外三處佐證：

| 位置 | 內容 |
|---|---|
| `ainarm9045.cpp:7314` | Loader 取料失敗要改一顆一顆吸，做法就是 `bVariModeFIX=true`（Jimmychiu 20250924「Suck one by one when a pickup error occurs at the loader」）|
| `aoutarm.cpp:2414`、`Magazine.cpp:810` | OutArm／Magazine 側 `bVariModeFIX==true \|\| bUseOnebyOne` → One by one 排列 |
| `AdjustInArmClosePitchCondition()` 後半 | Device Map 檢查、`IsRun2DCheck()`、FIFO site order、`Loader.Direction>=4`、Tray Block mode 也一律 `bCanPick2ICAtOnceTime=false` |

所以 **Fixed 是安全／檢查模式，不是省時模式**。
實測（250 顆、8 吸嘴）：Open/Close 下針 100 次 / 464 s；
Fixed 下針 **250 次**（一顆一針）/ 887 s，且**有無 Y 變距完全相同**
（單支模式連另一排都關掉，Y 變距派不上用場）。

間距仍要照走：`AutoCalculateInArmXClosePitch()` 在 Fix mode 也會執行
（Steven 20240509「移到外面, 避免Fix mode沒執行」），Tray 端固定在 first-fit 的
`InArmClose_PitchX`；`GetInArmToLoaderPosition_Single()` 用
`(iInArmXBase - iRealUseSuck) * dInArmXPitch_1Step` 補償「用的是第幾支吸嘴」。

### 1.4 掃描順序：列優先

先把「目前最前面那一個還有料的列」整列取完才換列；同一列內再取最大可吸支數。
驗證用不變量：**Tray 取料的列號不可回頭**。

---

## 2. HotPlate 放料（滾動游標）

**權威來源**：`ainarm_SearchPlacePlate.cpp` → `SearchPlacePlateXItem8_8Suck()`
（函式名依 HotPlate 欄數選：`XItem8_8Suck` = 8 欄 8 吸嘴；還有 `XItem6_/10_/12_/16_` 等版本）

```c
spacX = (bPitchOver12000) ? 1 : 2;                    // 跨距未超過 12000 → 跳一欄
spacY = iYHalf;

if(HotPlateYPitchCanPutAll() && iy + iYHalf < HotPlateForm.YDivision)
     { Row = (ArmRow1NotICForPlaceToPlate() && InArmSuck.HasIC()) ? 1 : 2; Col = 4; }
else { Row = 1; Col = 4; }                            // ★ 游標到最後 iYHalf 列 → 只放一排

bSuccess = CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
if(!bSuccess){
    iy++;                                              // ★ 列是內圈
    if(iy >= HotPlateForm.YDivision){
        iy = 0;
        ++ix;                                          // ★ 列掃完才換欄組
        if(ix >= HotPlateForm.XDivision/4) ix = 0;     // ★ 欄組上限 = XDivision/4
    }
}
```

四個要點：

| # | 行為 | 影響 |
|---|---|---|
| 1 | `iPlacePlateX/Y` 是**持續游標**，成功時不重置 | 盤面呈滾動掃描，不是每次從 (0,0) 重找 |
| 2 | `iy` 內圈、`ix` 外圈 | **同一欄組先掃完所有列，才換欄組** |
| 3 | `ix < XDivision/4` | 8 欄 → `ix ∈ {0,1}`，剛好兩個欄組（奇/偶欄） |
| 4 | `iy + iYHalf >= YDivision` → `Row=1` | **游標到最後 iYHalf 列時自動只放一排** |

`Row=1` 時放哪一排由「哪一排還有料」決定（`ArmRow1NotICForPlaceToPlate()`）；
剩下的那一排會在下一次搜尋時放到游標的下一個位置。

### 2.1 ⚠ 滿手時**不要**走 `Row=1`（實測缺陷）

第 4 點的 `Row=1` 是給「只剩一排有料」用的（第一個分支的判斷就是
`ArmRow1NotICForPlaceToPlate()`）。**吸嘴滿手時如果也走這條**，
會把上排 4 顆放在第 R 列、下排 4 顆放在第 R+1 列（游標只前進 1）。

這兩列相距 1，而吸嘴兩排固定相距 `iYHalf` 列 —— 於是這 8 顆
**永遠湊不回一組**，之後每次只能取 4 顆、多跑一趟 Shuttle。
實機病徵：**「HotPlate 尾盤明明還有 8 顆料，InArm 只取 4 顆就去放 Shuttle」。**

而且這不是排程能救的：取料群組 == 放料群組（§3 帳本），
放料當下分錯組，取料端就沒有補救的餘地。

模板的做法：

```js
if(both){                                   /* 上下排都有料 = 滿手 */
  if(pcY+IYHALF<HPF.rows && hpFree(pcY,pcX,2)) return {R:pcY,C:pcX,rows:2};
}else{                                      /* 只剩一排（Tray 已空的尾批） */
  if(hpFree(pcY,pcX,1)) return {R:pcY,C:pcX,rows:1};
}
pcY++; ...                                  /* 滿手在最後 iYHalf 列 = 視同失敗，前進游標 */
```

代價：配不到對的那幾列**用不到**。這是幾何必然（§6.1），不是缺陷 ——
`rows=16, iYHalf=3` → 可用 14 列，容量 112/128。

### 2.2 水位：用「探位」不要用「格數」

盤上放到「探不到整組空位」為止，剩下的那一組就是 `HAS_NULL_IC`。
一趟 HotPlate 之旅 = **先放滿新料、再取走 soak 最久的一組**。

⚠ 判斷「放不放得下」**不可以只看剩幾格空**：最後 `iYHalf` 列的空格湊不成整組，
用格數判斷會誤判有位 → 放料失敗、手上還有料 → 主循環空轉。必須實際探位：

```js
function hasFullSlot(){
  var R,C;
  for(R=0;R+IYHALF<HPF.rows;R++) for(C=0;C<HP_STEP;C++) if(hpFree(R,C,2)) return true;
  return false;
}
```

好處是水位自己會落在正確值（可用容量 − 吸嘴數），不需要維護魔術常數。

---

## 3. HotPlate 取料

### 3.1 FIFO

**取 soak 最久的那一組**（`hpAt` 最小）。
若用列號升序找，會永遠在前幾排循環、後半段的料 soak 過頭卻永不取走 —— 這是實測踩過的缺陷。

### 3.2 Site ↔ 吸嘴 必須對得上

一個 Carry Kit 只有「吸嘴數」個 Site 槽位，所以取料群組必須滿足：

```
上排 4 格的 site == SITE_MAP[0..3]
下排 4 格的 site == SITE_MAP[4..7]
```

`SITE_MAP` 取自實機記錄（本機 `[4,3,2,1,8,7,6,5]`，X 方向反向）。
**不檢查這條會取到「8 顆全是 B 排 site」**，放進 Kit 時撞同一個槽位 → 漏料。

### 3.3 整組同一條 Shuttle

`PickHPRec` 每個 team 只有一個 `Sht` 欄位 → 取料群組的 8 格必須 `hpS` 一致。
這也是 HotPlate 逐格要記 `WhichShuttle` 的原因。

### 3.4 湊不成整組時

依序退而求其次：整組 → 同列同 Shuttle 的 n 顆（n = 吸嘴欄數、2、1），同樣取最舊。
`base` 由該幾顆的 site 決定屬於上排（0）或下排（`ARM.cols`）。
實機記錄裡的孤兒群就是 `Suck = 10000000`（只下一支）。

---

## 4. Kit / Socket / 出料

| 動作 | 規則 |
|---|---|
| 放 In-Kit | slot = `site - 1`；槽位已被佔用時必須先把 Kit 送測（不可覆蓋） |
| Index 取料 | 需 Shuttle 在 `iRight`（`InSHT?InRT()`） |
| Index → Socket | Galil `LI` 三段：Z 先抬 safe → 四軸同動 → 到位才下壓；Socket 為互斥資源 |
| Index 放 Out-Kit | 需 Shuttle 在 `iLeft`（`InShtInLF()`，`DoFrontTestDestroyIC` case 200） |
| OutArm 取料 | 需 Shuttle 在 `iRight`（`OutSHT?InRT()`） |
| OutArm 放盤 | 受同一組 X 變距限制；Bin 盤與 Loader 盤同一個 Tray form → 尾欄同樣要少支放 |
| Bin 盤滿 | `DoAutoReceiveBinTray` → `TrayMoveOut` → `CatchNewTrayFromBuffer` |
| Tray Arm 停位 | 平時停在 Empty 軌（Color 以左）＝ Zone H 干涉區外，只在退盤時進 Auto 區 |

---

## 5. Shuttle 三站雙 Kit

同一條 Shuttle 軌只有一顆馬達、兩個停點（`Prod.InSHT[n].iLeft` / `.iRight`），
托板上有兩個 Kit（`FLCarryKit` = In-Kit、`FRCarryKit` = Out-Kit），間距＝行程：

| Shuttle 位置 | 站 A（InArm） | 站 B（Index/Socket） | 站 C（OutArm） |
|---|---|---|---|
| `iLeft` | In-Kit ← InArm 放未測料 | Out-Kit ← Index 放已測料 | — |
| `iRight` | — | In-Kit → Index 取未測料 | Out-Kit → OutArm 取已測料 |

依據：`csystem.cpp:701` `bool OutSHT1InRT(){return InSHT1InRT();}` → In/Out Shuttle 是同一個物件；
Index 取料要 `InRT()`、放料要 `InLF()` → Index 固定在站 B，是 Kit 在動；
InArm / OutArm 各只有一個 Shuttle X 教點 → A、B、C 是三個不同 X，間距＝行程。

---

## 6. 幾何上的硬限制

這些是算得出來的必然結果，不是排程能解的：

### 6.1 列對配對數上限

吸嘴兩排固定差 `iYHalf` 列，把 `{0..rows-1}` 用 `(R, R+iYHalf)` 配對時，
以 `iYHalf` 取模的各分支若有奇數個節點，每個奇數分支會剩 1 列。

`rows=16, iYHalf=3` → 分支大小 6/5/5 → 最多 **7 對＝14 列**，剩 2 列配不到對。

那 2 列只能靠「只放一排」用到，而「只放一排」會生出取不滿的 team（§2.1）——
所以正解是**讓那 2 列空著**：可用容量 14×8 = **112**，不是 128。
把「盤面格數」當容量會導致水位設錯、主循環空轉。

### 6.2 Tray 欄數與吸嘴數不合時的尾欄成本

`TRAY.cols` 不是「吸嘴欄數 × 合法倍數」的整數倍時，尾端幾欄湊不出完整群組，
只能少支甚至單支下針。例：10 欄、4 支、k=2 → 覆蓋 8 欄，剩 2 欄；
而 k=1 的 12.60 mm 低於機構下限 13.33 mm，所以那 2 欄無法配對。

驗算：k=3 可切成 `{1,4,7,10}{2,5,8}{3,6,9}` 三組無剩料，但後兩組只有 3 支、
填不滿吸嘴一排（一排固定 4 支），還是要補一針 → 兩條路下限都一樣。
**這是料盤選型問題**，Open/Close 的跳格自由度換不到針數。

### 6.3 Y 變距才是上料節拍的槓桿

無 Y 變距時 Tray 端一次最多「吸嘴欄數」顆；有 Y 變距且 Y 倍數合法時兩排同時下，
等效吸嘴數翻倍，下針次數大致砍半。HotPlate／Kit 側的 Y 間距仍必須等於 Site Y 節距，
所以 **Y 也要跟著變距兩次**。
