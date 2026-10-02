# 移植樹溫控現況、底層待改清單、兩份混廠牌方案對照

> 移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（C++，UTF-8）；golden＝V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（cp950）；網頁＝`D:\HT9045\web\page\`。只寫 `:行號` 的，是同一段前面那個檔。
> 行號以 `git -C D:\HT9045 show HEAD:<路徑>`（HEAD `89ccb4cc`，分支 `v906/steven-cbridge-review6`，20260927 14:xx）為準。
> 本檔分三塊：§1 事實（今天程式怎麼跑）；§2 待做清單（Q34 方案 D 第 ⑥ 點，**還沒有人做**）；§3 兩份方案對照（**都還是方案**）。
> ⛔ 20260927 更正：方案 D 已裁決（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` S166：D-1b、D-2＝A〔`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md` 第 7 條第 35 題〕、D-3a～D-9a；⑥ 第 4 點不做〔第 7 條第 34 題〕），St01 那一半（讀寫檔＋頁面＋ctest）已實作（commit `bc970c38`），現況見 §5。§1.1 表是實作前的樣子（下面有 ⛔ 更正表）、§3「都還沒實作」已過期。§2 底層（Jimmy）仍沒有人做。

## 1. 移植樹現況（事實）

### 1.1 HandlerSys 頁（St01 的檔）

| 步驟 | 網頁 → C++ | 程式 | 寫不寫 `D:\HT9045\system\Gerneral.ini` |
|---|---|---|---|
| 開頁 | `editlist.get` → 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:767-772` `OpenPage` → `HS_FormShow` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc:3075` `HeaterInsOpt_Read`（本體 `FileRW\HSys.cpp:96-106`，跟 golden `HandlerSys.cpp:50-60` 逐字） | `CheckAndReadIniDataGeneral`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:1643-1654`） | **缺鍵就寫 -9999**（照 golden）；已有的鍵不動 |
| 建下拉 | `FileRW\HSys.cpp:839-885` `ExtraJson` 送通道清單（替身名 `cbHeaterInsOpt_<通道>`，`:392-395` 起）；頁面 `D:\HT9045\web\page\ht9045_hsys_heater_c.js` 在 `grpHeater` 裡建 `<select>` | — | 否 |
| 點 Heater Type | 頁面只改畫面；存檔時 `FileRW\HSys.cpp:813-834` `BeforeApply` 重播 `rgHeaterTypeClick`（`FileRW\HSys.gen.inc:4054-4055` 起）只改記憶體；產生器 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\HSys.py:140-151` 把 golden `:1536`／`:1538` 兩個寫檔換成空敘述 | Q14＝B（S136，`3ee547e5`） | 按下不寫 |
| 存檔 | `editlist.save` → `FileRW\HSys.cpp:775-785` `SaveFlow` → golden `SaveSystemSet`（`FileRW\HSys.gen.inc:3644-3659`，對 golden `:779-794`） | 答「是」才寫（`SaveSystemSet:write`） | 寫 71 鍵＋`HEATER_CTRL_TYPE`，值跟 golden 相同 |
| 沒存成 | `FileRW\HSys.cpp:788-792` `Reload`＝再跑一次開頁 | — | 同開頁 |

⛔ 20260927 更正（commit `bc970c38`，Q34 方案 D＋Q15）：上表是實作前的樣子。現在（行號是 20260927 工作樹 `v906/steven-cbridge-review6` 的）：

| 步驟 | 現在的程式（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`） | 寫不寫 `D:\HT9045\system\Gerneral.ini` |
|---|---|---|
| 開頁 | `FileRW\HSys.cpp:111` `HeaterInsOpt_Read` 改成呼叫不寫檔的讀法 `W906_HeaterMixReadFile`（`:800`，golden 原文留在 `#if 0`）；產生檔 `FileRW\HSys.gen.inc:3071` 把 golden LoaderSystemSet :262-278 取代成 `W906_HeaterMixLoaderSystemSet`（`FileRW\HSys.cpp:843`） | **不寫**（71 鍵、新鍵、`HEATER_CTRL_TYPE` 都不寫）；同一頁其他 215 個讀檔呼叫照 golden，缺鍵仍補寫 |
| 建下拉 | `ExtraJson` 多帶 `heater.mix`（`FileRW\HSys.cpp:690` `W906_HeaterMixExtraJson`）；頁面 `D:\HT9045\web\page\ht9045_hsys_heater_c.js` 重做成「全機相同／各溫控器不同」＋逐通道表（§5.5） | 否 |
| 點 Heater Type | 產生檔 `FileRW\HSys.gen.inc:4068` 把 golden :1529-1538 取代成 `W906_HeaterMixTypeClick`（`FileRW\HSys.cpp:862`）：全機相同、Index＝其他＝點的廠牌，71 個通道記憶體都放那個廠牌（不放 -9999） | 按下不寫（Q14＝B 不變） |
| 存檔 | `FileRW\HSys.cpp:1287` `SaveFlow` 在 golden 存檔鈕之前先跑 `W906_HeaterMixSaveCheck`（`:877`；D-7a／D-8a 不過＝整頁不寫）；產生檔 `FileRW\HSys.gen.inc:3643` 把 golden :779-794 取代成 `W906_HeaterMixSave`（`FileRW\HSys.cpp:899`） | 寫模式三鍵＋71 鍵的實際廠牌＋（「不同」模式）站號＋`HEATER_CTRL_TYPE`（D-6a）；**刻意跟 golden 存出來的檔不同**（新鍵、D-4、D-6） |
| 沒存成 | 同上，`Reload`＝再開頁（也不寫） | 不寫 |

- `FileRW\HSys.cpp:807-808` 的說法「開頁已補好缺鍵，存檔只改寫既有的鍵、位元組跟 golden 相同」**是靠開頁會補寫**成立的；Q15（S137）改成開頁不寫之後這句不成立，要跟著改。⛔ 20260927 更正：已改——`FileRW\HSys.cpp` `BeforeApply` 上方的註解加了「⛔ 20260927 更正」段（原句保留）。
- 逐通道表與小工具在 `FileRW\HSys.cpp:108-389`（golden `MachineTypeUtility.cpp` 整檔），宣告在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys_Heater.h`。`EN_HEATER_SHEET=1` 只在 include 這個標頭的 TU 成立（`HSys_Heater.h:20-22`），其他 TU（bthermo 等）看不到這張表。⛔ 20260927 更正：行號已移（golden `MachineTypeUtility.cpp` 那一段現在是 `FileRW\HSys.cpp:126-407`，表 `:142` 起）；方案 D 的新全域與函式在同檔第 (4) 段（`:481` 起），宣告在 `FileRW\HSys_Heater.h` 檔尾。

### 1.2 溫控底層（Jimmy 的檔）

| 項目 | 移植樹位置 | 現況 | 對 golden V912 |
|---|---|---|---|
| 整台 No Heater | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\bthermo.cpp:1264` | `TC401HeaterControl==NoHeater` → return | golden `bthermo.cpp:1177` `IsNoHeaterMachine()`（同一個判斷，包成函式） |
| 第一次讀逐通道表 | — | 沒有 | golden `bthermo.cpp:1167-1175` |
| 送設定溫度 | `bthermo.cpp:2663-2698` | 看 `TC401HeaterControl`；TC401／KT4H／E5DC 的呼叫 `#if 0`（G21a／G22／G23a），DTK4848 `:2698` 有呼叫 | golden `:2520-2535` 逐通道 |
| 讀溫度 | `bthermo.cpp:2725-2760` | 同上（`#if 0` 的是 `:2732-2734`、`:2743-2745`、`:2754-2756`；DTK `:2760`） | golden `:2554-2569` |
| 選下一個 Task | `bthermo.cpp:2775-2790` | 看 `TC401HeaterControl` | golden `:2583-2598` |
| 換通道 | `bthermo.cpp:3495-3501` | 沒有 `g_iHeaterTypeIdx_SendCmd` | golden `:3215` |
| KT4H 警報值 | `bthermo.cpp:3503` | `TC401HeaterControl!=KT4H` | golden `:3219` 逐通道 |
| 通訊函式 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cpublic.cpp:292-349`（KT4H）、`:581-649`（TC401、E5DC） | `#if 0`（GA1-B3：`COM2->Comm2` 沒翻） | golden `cpublic.cpp:179-201`、`:422-488` |
| DTK4848 | `cpublic.cpp:350-381` | 有本體，寫 `g_pDTKComm`（`:157` 預設 nullptr；只有 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_cpublic_foundation.cpp:91`、`:122` 設）⇒ wb_serve 送不出去 | golden `cpublic.cpp:205-229` 經 `COM2->Comm2` |
| 溫控 COM 埠開埠與接收 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\rs232.cpp:217`、`:243-255`（RS232Init 閘住清單） | 沒翻 | golden `rs232.cpp:163-184`、`:264-283`、`:727-763` |
| Configuration 頁手動設溫／讀溫 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fConfiguration.h:2036` | 沒翻 | golden `cConfiguration.cpp:5591-5760` |
| Omron EJ1N | 移植樹 `EJ1N\` 沒有 `OmronEJ1N.cpp` | 沒翻 | golden `EJ1N\OmronEJ1N.cpp:622-626` |
| 溫控執行緒 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\StageThermo.cpp:30-42`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:877-879` | **沒有人建 HeaterThread**，wb_serve 不呼叫 DoThermo；關站時 `EndHeaterThread()` 是 noop | golden 主程式開機建 |
| 已安裝旗標 | `JsonBridge\StageThermo.cpp:50-60` | `bUT150Install[]` 全 false（golden 的寫入者幾乎都在 `main.cpp`，移植樹沒有這個檔） | golden `main.cpp` Index16Heater 等 |

核對指令（20260927 14:xx）：`git -C D:\HT9045 grep -n -E "HeaterThread|DoThermo\b" HEAD -- HT9011UC_Cpp_V3.33.906.0` 在 `uHeaterThread.cpp` 以外只有註解與 `MainClose.cpp` 的關站呼叫；`git grep -n "g_pDTKComm" HEAD -- HT9011UC_Cpp_V3.33.906.0` 在測試以外只有 `cpublic.cpp:157` 的定義。平行工作會讓這個結論過期，用之前重跑。

## 2. 底層待改清單（方案 D 第 ⑥ 點，歸 Jimmy，**沒有人做**）

出處：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 的「### Q34.」第 ⑥ 點（`fd215f72`）。這裡只列要改的位置，細節與理由看原文。

1. `bthermo.cpp:1264`：照 golden `:1167-1178` 加「第一次讀逐通道表」（用不寫檔的讀法，Q15），整台判斷改呼叫 `IsNoHeaterMachine()`。⛔ 20260927 更正（`bc970c38`）：`HeaterInsOpt_Read()` 本身已經是不寫檔的讀法（照方案 D 算模式／廠牌／站號），照 golden 在第一次執行時呼叫它就好，不用另外寫。
2. `bthermo.cpp:2663-2698`、`:2725-2760`、`:2775-2790`、`:3503`：`TC401HeaterControl==X` 換成 `IsValEqual_HeaterInsOpt(Addr, X)`；`:3497` 換通道後加 `g_iHeaterTypeIdx_SendCmd=Addr`。
3. 站號：通訊函式用「傳進來的序號＋1」（`cpublic.cpp:332`、`:343`、`:359`、`:362`、`:374`、`:377`、`:589`、`:609`、`:627`、`:644`）。方案 D 建議 bthermo 呼叫時傳「站號−1」（St01 提供 `W906_HeaterStationIdx(Addr)`），通訊函式不動；`OldTemp[]`、`UN150Read[]` 仍用通道序號。⛔ 20260927 更正（`bc970c38`）：`W906_HeaterStationIdx(Addr)` 已提供（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:787`，宣告 `FileRW\HSys_Heater.h`）：「不同」模式且 `HeaterInsAddr_<通道>` 在範圍內 → 回站號−1；否則回預設（KT4H／DTK4848／E5DC＝`Addr`、TC401＝`Addr/4`）。TC401 呼叫端照 golden `bthermo.cpp:2522`／`:2556` 傳 `(W906_HeaterStationIdx(Addr), Addr%4)`（通道仍是序號%4，D-7a）。它現在在只有 wb_serve 編的 TU 裡，要跟第 7 點一起搬。
4. No Heater 通道卡住迴圈（golden 同一缺陷，見 SKILL.md §3.1）：建議「廠牌不是 0～4 的有效溫控器（含 3）就 `Task=300`」——偏離 golden，Jimmy 決定；V912 只通報。⛔ 20260927 更正：**不做**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md` 第 7 條第 34 題：照 golden，不加 else）。
5. rs232 溫控那一半翻的時候照 912 逐通道（golden `rs232.cpp:742-760`）。
6. Configuration 頁與 OmronEJ1N 翻的時候照 912。
7. 逐通道表從 HandlerSys 的 TU 搬到 `MachineType.h`＋`MachineTypeUtility.cpp`（`ht9045_globals`，因為 bthermo 在 `ht9045_sm`、FileRW 只編進 wb_serve），同一顆 commit 刪掉 `FileRW\HSys.cpp` 的副本（兩份同名符號會踩 static archive 影子，KNOWLEDGE 陷阱 2）。⛔ 20260927 補（`bc970c38`）：方案 D 多了要一起搬的全域 `g_iHeaterInsMode`、`g_iHeaterInsIndexOpt`、`g_iHeaterInsOtherOpt`、`g_iHeaterInsAddr[71]` 與函式 `W906_HeaterMixReadFile`、`W906_HeaterStationIdx`、`W906_HeaterInsIsIndexGroup`、`W906_HeaterInsListed`、`W906_HeaterInsIndexLocked`、`W906_HeaterInsDefaultStation`（都在 `FileRW\HSys.cpp` 第 (4) 段）；頁面用的替身、`W906_HeaterMixLoaderSystemSet`／`TypeClick`／`SaveCheck`／`Save`／extra JSON 留在 HSys。
8. **前提**：溫控迴圈要先在 wb_serve 跑起來（HeaterThread、`cpublic.cpp` GA1-B3 兩段閘、rs232 溫控那一半）。在那之前，方案怎麼定都只影響畫面與檔案。

## 3. 兩份混廠牌方案對照（都還沒實作）

- **St01 方案 D**：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`「### Q34.」（`fd215f72`，HEAD 有）。Steven 選了 D（S154），D-1～D-9 待確認。⛔ 20260927 更正：D-1～D-9 已確認（S166），全文搬到 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`「### Q34.」；St01 那一半已實作（`bc970c38`，§5）。寫進檔案的是 D 的鍵名（`HeaterInsMode`／`HeaterInsIndexOpt`／`HeaterInsOtherOpt`／`HeaterInsAddr_<通道>`），下面 St02 方案的鍵名沒有用。
- **St02 平行方案**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q34_HEATER_MIX_PLAN.md`（`08c182c4` 20260927 11:46、`db66fe93` 11:55 補 S137；**只在 origin/main 與 origin/v906/steven-gpib-widget，HEAD 沒有**；讀法 `git -C D:\HT9045 show origin/main:HT9011UC_Cpp_V3.33.906.0/docs/Q34_HEATER_MIX_PLAN.md`）。

| 項目 | St01 方案 D | St02 方案 |
|---|---|---|
| 模式鍵 | `HeaterInsMode`（0 相同、1 不同） | `HEATER_CTRL_MIX`（0 相同、1 不同） |
| Index 位置廠牌 | `HeaterInsIndexOpt` | `HEATER_CTRL_TYPE_INDEX` |
| 其他位置廠牌 | 另開 `HeaterInsOtherOpt`；**不**沿用 `HEATER_CTRL_TYPE`（其他＝No Heater 時整台會不加熱） | 沿用 `HEATER_CTRL_TYPE` |
| 站號鍵 | `HeaterInsAddr_<通道>`（D-7a：TC401 只指定站號，通道仍是序號%4） | `HeaterInsSta_<通道>`＋TC401 另有 `HeaterInsCh_<通道>` |
| 舊檔沒有模式鍵 | D-1b：有下拉的 23 個裡 0～4 的值彼此不同＝不同（-9999 先換成 `HEATER_CTRL_TYPE`） | 有一個顯示的通道跟 `HEATER_CTRL_TYPE` 不同＝不同 |
| 檔裡的 -9999 | D-5a：跟著 `HEATER_CTRL_TYPE` | Index 組先看 `HEATER_CTRL_TYPE_INDEX`、再看 `HEATER_CTRL_TYPE` |
| 缺鍵（Q15＝3） | 只有「不同」模式會用到；舊檔判成「相同」所以不受影響 | 71 鍵全缺時照 `HEATER_CTRL_TYPE`，有任一鍵時缺的才當 3（它的 Q34-S8 待 Steven） |
| Index 組包含哪些 | D-2a：Head1～4＋Index 32 區，共 36 個 | 問 Steven（S1） |
| 程式放哪 | 底層照 golden 搬到 `MachineType.h`＋`MachineTypeUtility.cpp` | 同，另開 `HeaterMix.{h,cpp}`（globals）＋ctest `heater_mix` |
| golden 缺陷 | §3.1 卡在 No Heater 通道（建議 ⑥-4） | H1（存檔後 -9999 不重讀＋卡住）、H2（`PauseUT150Polling` 沒改 tracker） |

**動手前**：兩份的新鍵名要先統一（寫進 `Gerneral.ini` 的鍵一旦出貨就改不掉），並請 Steven 在 D-1～D-9 與 St02 的 S1～S7、Q34-S8 一起回答。⛔ 20260927 更正：已統一成方案 D 的鍵名（Steven S154 選 D、S166 開工）。

## 4. 例子

- 從 V899 升上來的機台（0 個 `HeaterInsOpt_`）今天在移植樹開 HandlerSys 頁：`[TempCtrl]` 段尾一次多 71 行 `HeaterInsOpt_…=-9999`（照 golden）。Q15 做完之後：開頁不寫，要按存檔才寫。⛔ 20260927 更正（`bc970c38`）：已做——開頁不寫；第一次按存檔（答「是」）才在 `[TempCtrl]` 段尾加 3＋71 行（`HeaterInsMode=0`、`HeaterInsIndexOpt`／`HeaterInsOtherOpt`＝`HEATER_CTRL_TYPE`、71 個 `HeaterInsOpt_`＝同一個廠牌；ctest `HSys_HeaterMix` 的 CaseV899 逐位元組比對）。
- 移植樹今天把 Shuttle1 的下拉改成 DTK4848 並存檔：檔案 `HeaterInsOpt_Shuttle1=4`，但移植樹的溫控仍照 `HEATER_CTRL_TYPE` 的廠牌（而且溫控迴圈沒跑）；同一份檔拿到 V912 機台，Shuttle1 會用 DTK4848、站號 3 去通訊。⛔ 20260927 更正（`bc970c38`）：方案 D 之後要先選「各溫控器不同」才能單獨改 Shuttle1；存檔另寫 `HeaterInsMode=1` 與列出通道的 `HeaterInsAddr_<通道>`（沒改＝預設站號，Shuttle1＝3）；`HEATER_CTRL_TYPE` 照 D-6a（列出的通道裡用最多的廠牌）；頁面顯示 D-9a 提示。移植樹溫控仍照 `HEATER_CTRL_TYPE`、V912 機台照 71 鍵，這兩點不變。

## 5. 方案 D 的實作（St01，20260927，commit `bc970c38`）

> ⛔ 20261002 更正（E-029／Q71～Q76，St01）：下面幾條已被取代——5.2 的「0～4」（現在 `HeaterInsOpt_`、`HeaterInsIndexOpt` 是 0～6，`HeaterInsOtherOpt` 仍 0～4，新鍵 `HeaterInsCh_<通道>`）、
> 5.2／5.4 的「只寫列出的通道（D-3a）」（現在＝這台有裝＋指定了站號的通道）、5.3 最後一條與 5.5 的「EJ1N／DTME08 時 Index 跟著其他位置、下拉停用」與 5.6 第 1、5 點
> （現在 Index 下拉自己選 EJ1N／DTM，跟 USE_16_HEATER 雙向連動；USE_16_HEATER 與 rgUse4DUT 用頁面上的值）、5.5 的「逐通道表＝D-3a 列出的通道」（現在 71 個全列，
> 兩組互斥即時顯示）、5.7 的 `W906_HeaterInsIndexLocked`（拿掉）。現況與規則：`D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` §9。

- 裁決：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` S166（Steven 20260927「開工，其他照建議」）＝D-1b、D-2a（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md` 第 7 條第 35 題：Index 位置＝Head1～4＋Index 32 區，Socket、DUT1～4、IndexESD、Door1～2 算其他）、D-3a、D-4a、D-5a、D-6a、D-7a、D-8a、D-9a；⑥ 第 4 點不做（第 7 條第 34 題）。Q15＝S137、Q14＝S136。全文 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`「### Q34.」。
- 範圍：只有 St01 的讀寫檔、頁面、ctest。底層（Q34 ⑥ 1～3、5～8：bthermo／cpublic／rs232／cConfiguration／OmronEJ1N）一行沒動，**移植樹機台溫控仍只看 `HEATER_CTRL_TYPE`**，頁面照 D-9a 提示。
- 行號是 20260927 工作樹 `v906/steven-cbridge-review6`（commit 前）的。

### 5.1 改了哪些檔

| 檔（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 或網頁） | 內容 |
|---|---|
| `FileRW\HSys.cpp` | 第 (4) 段 `:481` 起：新全域、`W906_HeaterMixReadFile`（`:800`）、`W906_HeaterMixLoaderSystemSet`（`:843`）、`W906_HeaterMixTypeClick`（`:862`）、`W906_HeaterMixSaveCheck`（`:877`）、`W906_HeaterMixSave`（`:899`）、替身 `W906_HeaterMixCreateProxies`（`:933`）、extra JSON（`:690`）、`W906_HeaterStationIdx`（`:787`）；`HeaterInsOpt_Read`（`:111`）改呼叫不寫檔的讀法（golden 原文留 `#if 0`）；`SaveFlow`（`:1287`）存檔前檢查；`BeforeApply` 上方註解加 ⛔ 更正段 |
| `FileRW\HSys_Heater.h` | 檔尾：新鍵與規則摘要、常數（`HEATER_INS_MODE_*`、站號範圍）、宣告 |
| `tools\editlist\HSys.py` | 三條 REPLACE，`_expect` 釘住 golden 原文：LoaderSystemSet :262-278 → `W906_HeaterMixLoaderSystemSet();`、SaveSystemSet :779-794 → `W906_HeaterMixSave();`、rgHeaterTypeClick :1529-1538 → `W906_HeaterMixTypeClick(iOpt);`；拿掉舊的 :276、:1535、:1536、:1538 四條 |
| `FileRW\HSys.gen.inc` | `gen_editlist.py --only HSys` 重產（改前改後各跑兩次，SHA256 不變＝冪等）；`kHS_SaveReads` 257→256（少了 `rgHeaterType`：golden 存檔段不再讀它；頁面照樣送，`BeforeApply` 重播照舊） |
| `D:\HT9045\web\page\ht9045_hsys_heater_c.js` | Heater 分頁重做（§5.5） |
| `tests\test_hsys_heater_mix.cpp`＋`tests\CMakeLists.txt` 檔尾 | ctest `HSys_HeaterMix`（§5.8） |

### 5.2 檔案格式（`D:\HT9045\system\Gerneral.ini` 的 `[TempCtrl]`）

| 鍵 | 值 | 什麼時候寫 |
|---|---|---|
| `HEATER_CTRL_TYPE` | 0～4 | 每次存檔，照 D-6a（§5.4） |
| `HeaterInsOpt_<通道>`（71 個） | 0～4，一律寫「實際廠牌」（D-4a；沒有下拉的 48 個也寫，不寫 -9999） | 每次存檔 |
| `HeaterInsMode` | 0＝全機相同、1＝各溫控器不同 | 每次存檔 |
| `HeaterInsIndexOpt`／`HeaterInsOtherOpt` | 0～4 | 每次存檔（「不同」模式也寫，切回「相同」時用） |
| `HeaterInsAddr_<通道>` | 站號 1～247（E5DC 1～99）；TC401＝第幾台 | 只有「不同」模式存檔，只寫列出的通道（D-3a），沒改＝golden 算的預設站號（D-7a） |

### 5.3 讀檔（開頁、以及底層之後第一次跑溫控，都不寫檔）

- 讀法：`CheckIniData`（移植樹 `common.cpp:634`）在才 `ReadIniData`（`:851`），缺鍵只在記憶體補（Q15）。
- 模式＝`HeaterInsMode`；缺鍵照 D-1b：有下拉的 23 個通道裡，有鍵、值 0～4（-9999 先換成 `HEATER_CTRL_TYPE`）彼此不同＝不同，否則＝相同。沒有鍵的通道不參加比較（所以 V899 檔＝相同）。
- `HeaterInsIndexOpt`／`HeaterInsOtherOpt` 缺鍵：相同＝那 23 個的共同值（一個都沒有＝`HEATER_CTRL_TYPE`）；不同＝`HEATER_CTRL_TYPE`。
- 通道廠牌：相同＝Index 組（D-2a，36 個）用 Index、其他用其他（71 個 `HeaterInsOpt_` 只當 V912 相容副本）；不同＝自己的鍵，-9999＝`HEATER_CTRL_TYPE`（D-5a），缺鍵＝3 No Heater（Q15）。
- 站號：`HeaterInsAddr_<通道>` 讀進 `g_iHeaterInsAddr[]`（0＝沒有鍵）；只有「不同」模式用（`W906_HeaterStationIdx`）。
- `USE_16_HEATER` 是 EJ1N／DTME08（2、3、5、6）：Index 的廠牌跟著「其他位置」（§5.6 第 1 點）。

### 5.4 存檔（`SaveSystemSet` 答「是」之後）

- 寫的順序：`HeaterInsMode` → `HeaterInsIndexOpt` → `HeaterInsOtherOpt` → 71 個 `HeaterInsOpt_` →（不同模式）列出通道的 `HeaterInsAddr_` → `HEATER_CTRL_TYPE`。缺的鍵加在 `[TempCtrl]` 段尾最後一個鍵後面（移植樹 `vclcompat\IniFiles.cpp:500`，同 BCB6 WritePrivateProfileString），既有的鍵原地改值（縮排、空白行不動）。
- D-6a：相同＝Index 與其他都是 No Heater 才寫 3，否則寫其他，其他是 No Heater 時寫 Index；不同＝列出的通道裡用最多的廠牌（不算 No Heater，同票取通道順序先出現的；列出的都是 No Heater 才改數 71 個；全部都是才寫 3）。
- 存檔前擋（`SaveFlow` 在 golden 存檔鈕之前；擋下＝**整頁**什麼都不寫，訊息列出原因）：D-7a 站號不是數字、超出 1～247（E5DC 1～99）；D-8a「不同」模式列出的通道同廠牌同站號（TC401 是同一台同一通道才算，同一台不同通道正常）——訊息寫出是哪兩個通道；列出的通道沒選廠牌；Index／其他沒選。
- 記憶體：`g_tHeaterInsInfo[]` 放實際廠牌（golden 存完留 -9999），模式三個全域與站號同步。
- D-9a：「不同」模式、Index≠其他、或改過站號時，存檔回應多一則提示（頁面上也一直顯示）。
- 存完照舊跑 golden 離開鈕的資料半段（`HSys.ReadGeneralIni()`），`TC401HeaterControl` 讀回新的 `HEATER_CTRL_TYPE`。

### 5.5 頁面（`D:\HT9045\web\page\ht9045_hsys_heater_c.js`）

- 最上面「溫控器廠牌：○ 全機相同 ○ 各溫控器不同」（替身 `rgHeaterInsMode`）。
- 「Index 位置溫控器」（`cbHeaterInsIndexOpt`，灰字列出 36 個通道）、「其他位置溫控器」（`cbHeaterInsOtherOpt`）。EJ1N／DTME08 版本 Index 下拉停用、旁邊顯示「Omron EJ1N（依 Index Heater Counts）」或「DTME08（依 Index Heater Counts）」，值跟著其他位置。
- 逐通道表（D-3a 列出的通道，每欄 20 列）：通道｜組別（Index／其他）｜廠牌（`cbHeaterInsOpt_<通道>`，有下拉的 23 個沿用 golden 那一組、Index 32 區另建）｜站號（`edHeaterInsAddr_<通道>`，空白＝預設）｜預設站號（灰字；TC401 顯示「第 N 台・通道 M」）。「全機相同」時唯讀、廠牌顯示算出來的、站號欄藏起來；切回「各溫控器不同」會還原切換前的逐通道值。沒列出的通道元件放在看不見的暫存區（引擎照常套值與送回，後端也不收）。
- 「Index Items」分頁的 Heater Type 單選：點一下＝全機相同、Index＝其他＝那個廠牌（不寫檔，存檔才寫）。
- D-9a 提示條（橘框）：「目前溫控只看 HEATER_CTRL_TYPE（單一廠牌、預設站號），這些設定要等底層翻完才生效」。

### 5.6 St01 的解讀（裁決沒寫到的細節；已列給 Steven 確認）

1. EJ1N／DTME08 版本 Index 下拉停用時，Index 的廠牌跟著其他位置（讀檔與存檔都是）：Index 組裡的 Head1～4 仍在溫控 COM 埠上，停用的下拉若留舊值，操作員在「全機相同」改不到 Head1～4。
2. D-6a「不同」模式「用最多通道的那個廠牌」先數列出的通道（真的在溫控 COM 埠上輪詢的），不數全部 71 個。
3. D-8a 的 TC401：同一台、同一個通道才算重複。
4. 「不同」模式沒列出的通道，存檔寫記憶體裡的實際廠牌（檔案值或開頁算的），不跟著 Index／其他。
5. `USE_16_HEATER` 用目前的全域（開頁與存檔當下），不看同一頁剛改、還沒存的 Index Heater Counts；有下拉的 23 個的可見條件照 golden 是開機算一次。

### 5.7 給 Jimmy（底層 ⑥）的介面

- 第一次跑溫控：照 golden `bthermo.cpp:1167-1175` 呼叫 `HeaterInsOpt_Read()`（已是不寫檔的讀法）。
- 廠牌：照舊 `IsValEqual_HeaterInsOpt(Addr, X)`；整台 No Heater：`IsNoHeaterMachine()`（仍看 `TC401HeaterControl`）。
- 站號：`W906_HeaterStationIdx(Addr)`＝站號−1（就是 `cpublic.cpp` 通訊函式收的 Addr）；TC401 傳 `(W906_HeaterStationIdx(Addr), Addr%4)`。
- 要搬到 `MachineType.h`＋`MachineTypeUtility.cpp` 的（§2 第 7 點）：`g_iHeaterInsMode`、`g_iHeaterInsIndexOpt`、`g_iHeaterInsOtherOpt`、`g_iHeaterInsAddr[71]`、`W906_HeaterMixReadFile`、`W906_HeaterStationIdx`、`W906_HeaterInsIsIndexGroup`、`W906_HeaterInsListed`、`W906_HeaterInsIndexLocked`、`W906_HeaterInsDefaultStation`。
- 底層做完之後，St01 拿掉 D-9a 提示（Q34 順序第 ⑤ 步）。

### 5.8 驗證（20260927）

- ctest `HSys_HeaterMix`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_hsys_heater_mix.cpp`）：105 項全過，SIM 組態，build 資料夾 `D:\AI_TempFile\st01e-q34-build`（只 build 這一個目標）。受測的是 wb_serve 編的同一份 `FileRW\HSys.cpp`＋產生檔＋C 路共用層，god-stack 用 RESCAN 連；只寫 `%TEMP%\ht9045_hsys_heater_mix_test\`。涵蓋 V899 檔、V912 全 -9999 檔、V912 混搭檔、新檔來回（存兩次位元組不變）、「相同」Index≠其他與 D-6a、EJ1N 鎖 Index、開頁不寫檔、存檔逐位元組比對、D-7a、D-8a（`SaveFlow` 擋下、golden 存檔鈕沒跑、檔案沒變）、Heater Type 重播不寫檔、extra JSON。
- 測試跑前跑後 `D:\HT9045\system\Gerneral.ini` SHA256 `8ECFA892…94AA`、`D:\HT9045\config\config.ini` SHA256 `7E4BD43B…82A6` 都沒變（測試程式自己也比對）。
- Steven01 這台的實檔（`test_hsys_heater_mix --example D:\HT9045\system\Gerneral.ini 2`：複製到暫存跑，原檔只讀）：開頁檔案不變；什麼都不改直接存檔 → 第 353～423 行 71 個 `HeaterInsOpt_…=-9999` 改成 `=2`，第 424～426 行多 `HeaterInsMode=0`、`HeaterInsIndexOpt=2`、`HeaterInsOtherOpt=2`，第 311 行 `HEATER_CTRL_TYPE=2` 不變，其他行一個位元組都沒動。跟 decisions-decided Q34 的例子一樣。
- 頁面：`node --check` 過；用假 DOM 跑建畫面／切模式／點 Heater Type／EJ1N 鎖定共 21 項過。**沒有在瀏覽器實測**（不跑 wb_serve）。

## 6. E-029 的實作（St01，20261002）

全文在 `D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` §9（Steven 原話、規則、給 Jimmy、驗證）。這裡只記改了哪些檔：

| 檔 | 內容 |
|---|---|
| 移植樹 `FileRW\HSys_Heater.h` | 檔尾 E029：規則全文、`W906_HEATER_INS_EJ1N`／`W906_HEATER_INS_DTM`／`W906_HEATER_INS_OPT_COUNT`、站號範圍、`g_iHeaterInsCh[]`、匯流排常數、`W906_HeaterInsBus*`／`W906_HeaterInsUnitCh`／`W906_HeaterInsU16Area`／`W906_HeaterInsU16For`；`W906_HeaterInsIndexLocked` 拿掉 |
| 移植樹 `FileRW\HSys.cpp` | 第 (4) 段重寫：`g_W906HeaterInsOptStr`（7 個）、`HmActive`（兩組互斥）、`HmLinkIndex`（Index ⇄ USE_16_HEATER）、`HmFromProxies`（台號／站＋CH、Other 只收 0～4）、`HmDuplicates`（三條匯流排）、`HmCtrlTypeFor`（不寫 5／6）、`HmReadFile(u)`、ExtraJson（71 列、`pair`、`active`、`otherOptions`、`conn`）、`W906_HeaterMixSave` 多寫 `HeaterInsCh_` 與 `[System] USE_16_HEATER`；第 (3) 段 golden 23 個下拉改 7 項；ExtraJson 頂層 `options` 改 7 個（`goldenOptions` 留 5 個） |
| 移植樹 `tests	est_hsys_heater_mix.cpp` | 舊情境照新規則改期望值（V899／V912 檔在 USE_16_HEATER=2 時 Index 區＝5、Head1～4 不算有裝）；新增 `CaseE029`（11 段） |
| 移植樹 `tools\webprobe\e029_heater_selftest.cjs`＋`tests\CMakeLists.txt` | 新 ctest `E029_HeaterPages`（node） |
| `D:\HT9045\web\page\ht9045_hsys_heater_c.js` | 重寫：規則純函式（`window.HT9045HSysHeater.rules`）、71 列、Other 5 個、真的點 `#rgHeater`、跟著 `#rgHeater`／`#rgUse4DUT` 即時顯示、連線設定唯讀 |
| `D:\HT9045\web\page\ht9045_temperfrom_strip.js`＋`i18n.js` | 5／6 的位址（`EJ1N CHx-x`、`DTM CHx-x`）、「不同」模式的站號鍵、新詞條 `Temp: Station not set` |
