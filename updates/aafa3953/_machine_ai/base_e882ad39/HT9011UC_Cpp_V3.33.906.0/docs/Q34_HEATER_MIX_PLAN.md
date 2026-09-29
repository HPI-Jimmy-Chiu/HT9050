# Q34 溫控器混廠牌設定計畫（只寫文件，不改程式）

> AI(W906-Q34) 20260927（St02）。唯讀調查結果，**這份沒有改任何程式**。
> 標記：**[R]** = 讀過程式碼；**[I]** = 推論，要再確認。
> 樹名（每個 golden 行號都寫樹名）：
> - **V906@main**：`v906/steven-gpib-widget`（與 main ded836fe 在這些檔上相同）
> - **V906@st02**：`v906/steven-st02-on-cbridge`／St01 的 `v906/steven-cbridge-review6`
> - **906_0625_Steven**：`HT9011UC_Code_V3.33.906.0_20260625_Steven`（St02 的 906 golden，Steven #34）
> - **912**：`HT9011UC_Code_V3.33.912.0_20260908_Jimmy`
>
> Steven 的原話（St01 轉達 5839b4b6）：「1.相同:要選index的溫控器 與 其他位置的溫控器 1.不同:每個溫控器單獨設定, 並且可以指定站號」

## 裁決更新：S137 Q15（Steven 20260927，St01 RULINGS_20260926 S137 定案）

> 「預設選 3 No Heater, 然後B 開頁不寫檔」；「Q15要跟這個一起做」。

1. **缺鍵的預設改成 3（No Heater）**，不再是 -9999（跟著 `HEATER_CTRL_TYPE`）。所以第 3 節「每個通道的廠牌怎麼決定」改成：
   - `HeaterInsOpt_x` 是 0..4 → 用它；
   - `HeaterInsOpt_x` 缺鍵 → **3（這個通道沒有加熱器）**；
   - `HeaterInsOpt_x` 明寫 -9999（912 存檔時藏起來的通道）→ 照 912 的意思「沿用」：Index 組先看 `HEATER_CTRL_TYPE_INDEX`，再看 `HEATER_CTRL_TYPE`。
2. **打開 HandlerSys 頁不寫檔**（B）。第 5 節「開頁會把 71 個 -9999 寫進共用的 Gerneral.ini」的附註**不再適用**：讀取時缺鍵不寫回，只在按存檔時寫。
3. ⚠ **要請 Steven 確認的一點（Q34-S8）**：現場機台的 Gerneral.ini 目前**一個 `HeaterInsOpt_*` 鍵都沒有**（`D:\HT9045\system\Gerneral.ini` 只有 `HEATER_CTRL_TYPE=2`）。若照「缺鍵＝3」，底層改成逐通道判斷（第 3 節改動 1-3）之後，這種舊檔的**每一個通道都會變成沒有加熱器**，加熱器全部不動作。
   - 建議的移轉規則：**71 個鍵全部都缺（舊檔）時，所有通道照 `HEATER_CTRL_TYPE`**（等於現在的行為）；只要檔案裡有任何一個 `HeaterInsOpt_*` 鍵，缺的那些才當 3。
   - 或者：第一次存檔前頁面把舊檔當成「全部同一廠牌」顯示，存檔時才寫出 71 個明確的值。
4. ctest `heater_mix` 的 T1 改成：舊檔（只有 `HEATER_CTRL_TYPE=2`）→ 71 個都解析成 2（照上面的移轉規則），**而且檔案不被寫入**；另加 T1b：有部分鍵的檔案，缺的解析成 3。

## 待回覆的問題（先看這裡）

**給 Steven**

| # | 問題 |
|---|---|
| S1 | 「Index」這一組包含哪些通道？只有 Aa1..Bh2（32 個 Index 加熱器），還是 Head1-4／Socket／DUT1-4／IndexESD／Door1-2 也算？ |
| S2 | 「Index 的溫控器」是現有的 rgHeater（`USE_16_HEATER`：4／16／32 組 × KT4H／EJ1N／DTME08），還是新的廠牌鍵？現在 eht16Heater／eht32HeaterKT4H 的機台只能全部用 KT4H。`HEATER_CTRL_TYPE_INDEX` 預設要跟 `HEATER_CTRL_TYPE` 一樣，還是從 `USE_16_HEATER` 推？ |
| S3 | 站號四種廠牌都要能設嗎？用十進位輸入？TC401 現在還有在用嗎？ |
| S4 | DTME08／DTMN08（HT9050，走乙太網路）要不要當成一種逐通道廠牌？若在 `HeaterInsOpt_*` 放第 6 種值，912 會讀成「沒有廠牌」（912 `MachineTypeUtility.cpp:111-112`），所以要另開一個 V906 自己的鍵。 |
| S5 | 頁面要不要把 912 藏起來的 48 個通道顯示出來？ |
| S6 | 底層改成逐通道判斷廠牌是 912 的行為，等於 RULINGS_20260926 第 26 條（「底層照 906」）的例外，跟 RULINGS_20260927 第 6 條同一類。需要裁決。 |
| S7 | 混廠牌的機台如果裝回 912 以前的版本（V899／906 BCB6），每一顆溫控器都會用 `HEATER_CTRL_TYPE` 的協定去驅動。要不要在安裝包或開機時擋？ |

**給 Jimmy**

| # | 問題 |
|---|---|
| J1 | 請確認 H1（第 2 節）。V906 要不要修（存檔後重新解析）？要不要回報給 V912 的負責人？ |
| J2 | 請確認 H2。V906 要不要在送出指令的當下就設定 tracker？ |
| J3 | 不同協定能不能共用一條 9600 8N1 的匯流排？TC401 是 Modbus RTU，KT4H／DTK 是 Modbus ASCII，E5DC 是 CompoWay/F。只要混進 TC401，站號一定會撞，站號覆寫就變成必要。 |
| J4 | bthermo 的改動 1-3（現在沒有執行效果）要現在就合，還是等 Comm2 移植時一起？ |
| J5 | 912 把 `HeaterInsOpt_Read` 放在畫面檔（HandlerSys.cpp），V906 搬到 globals 可以嗎？ |

## 0. 題目引用的行號不在 main 上

- `FileRW/HSys.cpp:92-103`（HeaterInsOpt_Read）與 `FileRW/HSys_Heater.h:23-24` **只在 St01 的分支上**（c913d5e5「S61 TC401 heater per-channel」）[R]。
- 在 V906@main，`FileRW/HSys.cpp:92-103` 是 EP_MAXKPA 的讀取，`HSys_Heater.h` 不存在。
- main 的 C++ 裡沒有 `g_tHeaterInsInfo`，只有 `FileRW/HSys.gen.inc` 的 `#if 0` 區塊 [R]。
- V906@st02 `HSys_Heater.h:23-24` 自己寫明：這張表目前只有 HandlerSys 頁讀寫；溫控流程不讀它，讀的是 `TC401HeaterControl`＝`[TempCtrl] HEATER_CTRL_TYPE`。

## 1. 現況：整台機器一種廠牌

**鍵與值** [R]
- Gerneral.ini `[TempCtrl] HEATER_CTRL_TYPE` 讀進 `int TC401HeaterControl`。
- 值：TC401=0、KT4H=1、E5DC=2、NoHeater=3、DTK4848=4。
  - V906@main `cmydef.cpp:226-231`、`cmydef.h:149-154`
  - 906_0625_Steven `cmydef.cpp:222-227`、`cmydef.h:141`
  - 912 `cmydef.cpp:222-227`、`cmydef.h:178`

**V906 讀寫處** [R]
- 開機讀：`database.cpp:541`（預設 KT4H）。
- HandlerSys 頁：`FileRW/HSys.gen.inc:3072`／`:3080` 讀、`:3641` 寫；912 的逐通道分支在 `#if 0` 裡（`:3069-3083`、`:3641-3659`）。

**V906@main 所有使用 `TC401HeaterControl` 的地方** [R]

| 位置 | 函式 | 判斷 |
|---|---|---|
| `bthermo.cpp:1264` | DoThermo | `==NoHeater` → return |
| `bthermo.cpp:2663/2674/2685/2696` | DoThermoReal case 100，寫 SV | 依廠牌；`:2671/:2682/:2693` 在 `#if 0`（G21a/G22/G23a），只有 DTK `:2698` 是活的 |
| `bthermo.cpp:2725/2736/2747/2758` | case 100，讀 PV | 同上（`:2733/:2744/:2755` 閘著，`:2760` 活的） |
| `bthermo.cpp:2775/2779/2783/2787` | case 100，下一個 Task | 依廠牌 |
| `bthermo.cpp:3503` | case 500，KT4H 警報 | `!=KT4H` |
| `cObserver.cpp:3831` | 溫度分頁顯示 | `!=NoHeater` |
| `uTemp_Set.cpp:3249`、`FileRW/Temperature.gen.inc:4092` | ReadTempFile | `==NoHeater` |

- [I] V906 目前**沒有任何溫控器匯流排的通訊**：TMC401／UT100／E5DC 的送出函式在 `#if 0`（`cpublic.cpp:292-349`、`:581-649`）；DTK 寫到 `g_pDTKComm`，只有測試會設它；rs232 的 Comm2 那一半閘著（`rs232.cpp:245-251`）。
- 906_0625_Steven 另外還有：cConfiguration UpdateUT150Comm（`:5515-5638`）、rs232 `:163`／`:736`／`:741`、OmronEJ1N `:604`、main.cpp FormShow／Index16Heater／ToolLoadICO。這些在 V906 都還沒翻。

## 2. V912 的逐通道模型（71 個鍵、`g_tHeaterInsInfo`）[R]

**開關與結構**
- `EN_HEATER_SHEET 1`、`eHeaterType_Count = tcTotalCount (71)`、`eHeaterInsOpt_Count 5`、`DEFAULT_HEATER_INS_OPT KT4H`（912 `MachineType.h:657-713`）。
- 「沒有值」＝`INVALID_INT_VAL_NEG -9999`（912 `cmydef.h:169`）。

**表**（912 `MachineTypeUtility.cpp:30-102`；V906@st02 的副本 71/71 一致）
- 鍵＝`[TempCtrl] HeaterInsOpt_<ShowName>`，索引＝`eTempControll`：HotPlate1-2、Shuttle1-2、Head1-4、Socket、Chamber、CCD、Aa1..Bd2、HeatGun1-2、DUT1-4、Ae1..Bh2、2D、LB、IndexESD、CCD_2、ATCHotAir1-2、OutSht1-2、Base1-6、HotPlate3-4、Shuttle3-4、Door1-2、LBUp／LBDown。
- 頁面顯示 23 個通道，**藏 48 個，包含全部 32 個 Index 加熱器**。
- -9999＝沿用。

**預設值**
- 缺鍵時讀取會把 **-9999 寫進 ini**（912 `common.cpp:1444-1455`），記憶體回退成 `HEATER_CTRL_TYPE`。
- 現場 `D:\HT9045\system\Gerneral.ini` 沒有這 71 個鍵，`HEATER_CTRL_TYPE=2`。

**HandlerSys 頁（912 HandlerSys.cpp）**
- 載入 `:262-278`：顯示的通道廠牌全部相同時，rgHeaterType 顯示那個廠牌。
- 存檔 `:782-793`：71 個鍵全寫；藏起來的通道寫 -9999（記憶體也是）。
- 點 rgHeaterType `:1519-1538`：立刻寫。

**底層**
- DoThermo 只讀一次表（static 保護，`bthermo.cpp:1167-1175`），然後 `IsNoHeaterMachine()`（`:1177`）。
- DoThermoReal 用 `IsValEqual_HeaterInsOpt(Addr,X)`：`:2520-2595`、`:3219`。
- tracker `g_iHeaterTypeIdx_SendCmd`（`:1164` 定義，case 300 `:3215` 設定）決定 rs232 `Comm2ReceiveData` 用哪個解析（`:742/:747`）。
- cConfiguration `:5612-5741` 也逐通道判斷。
- `IsNoHeaterMachine()` 仍是整台機器的判斷（`MachineTypeUtility.cpp:122-127`）。

**只有一條匯流排**
- `[TempCtrl] COM_PORT`，固定 9600 8N1（912 `rs232.cpp:268-273`）。
- 站號由通道推出：KT4H／DTK `:%02X` Addr+1；E5DC `%02d` Addr+1；TC401 unit＝Addr/4+1、ch＝Addr%4。
- **912 沒有逐通道的站號鍵。**

**912 跟 Steven 的要求差在哪**
1. 沒有「相同／不同」模式。
2. Index 組沒有自己的廠牌（Index 通道被藏起來，每次存檔都被改回 -9999）。
3. 沒有站號。

**912 的風險**（[I]，請 Jimmy 確認）
- **H1**：HandlerSys 存檔後，藏起來的通道在記憶體是 -9999，DoThermo 又不會重讀（static 保護），所以每一種廠牌的 IsValEqual 都是 false；case 100 沒有任何分支設 Task，掃描就停在那個通道。bthermo 負責 Index 加熱器的機型（`USE_16_HEATER` = eht16Heater／eht32HeaterKT4H）會碰到。已安裝的通道設成 NoHeater 也一樣。
- **H2**：`PauseUT150Polling` 重設 Task／Addr 但沒改 tracker，恢復後第一筆回覆可能用上一個通道的格式解析。

## 3. V906 的設計提案

**原則**
- 底層每個通道只看到**一個已解析好的廠牌**，用 912 的 `IsValEqual_HeaterInsOpt`，不改。
- 「模式」與「Index 組廠牌」是頁面上的方便設定，存檔時展開成 71 個鍵。
- 912 的鍵名與意義位元組相容；V906 新增的鍵是附加的（912 不讀、存檔也不動它們）。

**檔案格式**（Gerneral.ini `[TempCtrl]`）

```
HEATER_CTRL_TYPE=0..4       ; golden：整台／「其他位置」的廠牌；3 = 沒有加熱器的機台
HeaterInsOpt_<name>=0..4    ; 912 的 71 個鍵照抄；-9999 = 沿用
HEATER_CTRL_MIX=0|1         ; 新增：0 相同，1 不同
HEATER_CTRL_TYPE_INDEX=0..4 ; 新增：Index 組的廠牌（相同模式用）
HeaterInsSta_<name>=n       ; 新增：站號（十進位，照溫控器上的設定）；沒有／-9999 = golden 公式
HeaterInsCh_<name>=0..3     ; 新增，只有 TC401（如果 TC401 還在範圍內）
```

**每個通道的廠牌怎麼決定**（也就是移轉規則）
- `HeaterInsOpt_x` 是 0..4 就用它；
- 否則 x 在 Index 組、且 `HEATER_CTRL_TYPE_INDEX` 是 0..4，就用它；
- 否則用 `HEATER_CTRL_TYPE`。
- 所以只有 `HEATER_CTRL_TYPE` 的檔案行為跟現在完全一樣；912 的檔案照原樣讀得懂。

**新鍵的讀寫**
- 新鍵**讀的時候不自動寫回**，缺鍵就保持缺、預設值用推的。
- 模式缺鍵時：只要有一個顯示的通道跟 `HEATER_CTRL_TYPE` 不同就是 1，否則 0。
- 相同模式存檔時 71 個鍵全寫明確的值，所以 912 讀同一個檔會得到同樣的廠牌；之後若被 912 存檔把 Index 通道改回 -9999，`HEATER_CTRL_TYPE_INDEX` 會把它們補回來。

**底層要改的地方（全部是 Jimmy 的檔）**

| # | V906@main 位置 | 改法 | 依據 |
|:-:|---|---|---|
| 1 | `bthermo.cpp:1262-1265` | 加「只讀一次」＋V906 的解析；`==NoHeater` 改 `IsNoHeaterMachine()` | 912 `:1167-1177` |
| 2 | `bthermo.cpp` 13 處（`:2663-2787`、`:3503`） | `TC401HeaterControl==X` → `IsValEqual_HeaterInsOpt(Addr,X)` | 912 `:2520-2595`、`:3219` |
| 3 | case 300 `:3495-3501` | 加 `g_iHeaterTypeIdx_SendCmd=Addr` 與它的定義 | 912 `:3215`／`:1164` |
| 4 | 站號（Steven 確認後才做） | 13 個送出呼叫（`:2671, 2682, 2693, 2698, 2733, 2744, 2755, 2760, 3520, 3532, 3543, 3614, 3625`）傳解析後的匯流排位址；cpublic 的送出函式照 golden | V906 自己的 |
| 5 | rs232 Comm2 那一半（閘著） | 移植時翻 **912** 的版本（`:164-184`、`:727-763`） | 912 |
| 6 | UpdateUT150Comm（沒翻） | 移植時翻 912 `:5590-5741` | 912 |
| 7 | OmronEJ1N Timer1Timer（沒翻） | `IsNoHeaterMachine()` | 912 `:623` |
| — | `database.cpp:541`、`cObserver.cpp:3831`、`uTemp_Set.cpp:3249`、`Temperature.gen.inc:4092` | 不用改：整台機器的判斷，912 也照舊用全域 | — |

[I] 改動 1-3 在 Comm2 匯流排移植之前沒有執行效果，可以先合、先測，不必等硬體。

**檔案放哪裡**
- **新檔 `MachineTypeUtility.cpp`**，放 `ht9045_globals`：912 原樣＋`HeaterInsOpt_Read`＋顯示用的對照表（912 `HandlerSys.cpp:32-60`）。放 globals 是因為 bthermo 在 `ht9045_sm`，而 FileRW 只編進 wb_serve。
- **`MachineType.h`**：加 912 `:657-714` 那一段＋`INVALID_INT_VAL_*`。
- **新檔 `HeaterMix.{h,cpp}`**（globals，V906 自己的）：鍵名、解析、站號對照＋衝突檢查、新鍵的讀寫。
- **V906@st02 `FileRW/HSys.cpp`**：搬走的本體（`:69-389`）**在同一個 commit 刪掉**，不然會踩到靜態庫遮蔽的陷阱（KNOWLEDGE gotcha 2）。`HSys_Heater.h` 改成 include `MachineType.h`。ExtraJson（`:826`）加新鍵。存檔後重新解析（修 H1；這是偏離，Jimmy 決定）。
- **網頁** `ht9045_hsys_heater_c.js`＋`HW.HandlerSys.html`：模式選項；相同 → 兩個下拉（Index／其他）；不同 → 逐通道下拉＋站號欄；Index 通道顯示出來。

**ctest `heater_mix`**（只連 globals；暫存的 Gerneral.ini 走 `asGeneralPath` 轉向，照 `tests/test_binsel_core.cpp:232-240`）
- T1 舊檔（只有 `HEATER_CTRL_TYPE=2`）：71 個都解析成 2；golden 的 -9999 會被寫入；新鍵不寫。
- T2 912 的檔：照讀，存檔結果跟 912 `:782-793` 一致。
- T3 `HEATER_CTRL_TYPE=3` → `IsNoHeaterMachine()`。
- T4 相同模式，Index=1／其他=2。
- T5 912 把 Index 改成 -9999 後，INDEX 鍵補回。
- T6 不合法的值。
- T7 站號用假 TComm 驗實際送出的位元組（`tests/test_cpublic_foundation.cpp:91-137`）。
- T8 站號衝突要被抓出來。

## 4. 誰負責哪些檔

- **Jimmy**（全部 jimmychiu）：`bthermo.cpp`（7a6a8a5a，唯一一個 commit）、`cpublic.*`、`rs232.cpp`、`cConfiguration.cpp`、`cmydef.cpp`、`database.*`、`MachineType.h`（另有機台端／EastSun 的 commit）、`cObserver.cpp`、`uTemp_Set.cpp`。
- **頁面**是 St01 的資料讀寫工作（V906@st02 `HSys.cpp`／`HSys_Heater.h`／`ht9045_hsys_heater_c.js`，c913d5e5）。
- **St02 能做的**：`HeaterMix.{h,cpp}`＋`heater_mix` 測試（新檔，這台只編譯）；MachineTypeUtility 的搬移做成 patch 給 Jimmy 看。bthermo／rs232／cpublic／cConfiguration 都不碰。

## 5. 附註

- 打開 V906@st02 的 HandlerSys 頁，會把 71 個 -9999 鍵寫進共用的 Gerneral.ini。這照 912 golden，BCB6 讀取端會忽略，但它確實是對共用檔的寫入。
