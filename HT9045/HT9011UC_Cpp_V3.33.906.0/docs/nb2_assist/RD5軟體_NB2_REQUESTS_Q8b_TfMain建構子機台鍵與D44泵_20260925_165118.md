# Q8(b) 預勘：TfMain 建構子的 Gerneral.ini 機台鍵（逐鍵）＋ D44 泵先後

> **產出方式**：NB2 輔助 session 的子代理（workflow `nb2-requests-q6-q10-batch1`，同批 5 個 agent，全程只讀），主迴圈存檔前抽驗承重說法：
> * ✔ 移植樹 git grep '"ZSafePos"' 0 筆、Motor/mymotor.cpp:81 預設 20（屬實 ⇒ 開機沒讀）
> * ✔ golden main.cpp:1696/:1698 讀 [In Arm] ZSafePos，預設 20／50（屬實）


> 產出：NB2 輔助 session，唯讀。快照 `6b942f15`（已含 origin/main 合併 `a22405a1`）。
> golden＝906 UTF-8 鏡像 `.nb2_scratch/golden_utf8`（RULINGS §15）。
> 沒建置、沒跑 ctest、沒啟 wb_serve、**沒用 build_nb2**（它是舊的）。結論全部來自讀碼。
> 標記：【量】＝讀碼／讀檔直接看到；【推】＝推論。

---

## 0. 先看這裡（6 條）

1. **golden 建構子直接讀 26 個鍵**（`main.cpp:1340-2265`）【量】。
   分區：[System] 19 個、[In Arm] 1 個、[VENDER] 6 個。
   另有 2 段是 golden 自己註解掉的：`AUTO_EMPTY_COLOR`（:1347-1364，搬去 ReadGeneralIni）、`AGVModal`（:2107-2115）。
   建構子呼叫的其他函式都不讀 Gerneral.ini：`GetMainAuth`／`GetObserAuth` 讀的是 `Security_new.def`（`cAuthority.cpp:359`／`:210`），`SetInitialData` 讀的是 ARMS 參數檔（`main.cpp:22491`）。
2. **移植樹開機現在讀了 13 個，還有 13 個沒讀**【量】。
   * 讀了：`CUSTOMER_CODE`、`SUPPORT_2_EMPTY_EMPTY`（走 ReadGeneralIni）。另外 11 個走 Steven review6 的 `FileRW_HSys_ReadMainCtorKeys`（`FileRW/HSys.cpp:65-133`，由 `tools/wb_serve.cpp:2942` 呼叫）：`EP_Install`、8 個 `EP*`、`INOUT_ARM_PICKER_USE_MOTOR`、`ION_FAN_TYPE`。
   * 沒讀：`ZSafePos`、`INDEX_SUCKER_TYPE`、`IndexTimeSet`、[VENDER] 6 個、`SetupFileCheckList`、`bContaceTorque`、`bUse_NewAutoCleanForm`、`bUseNewCleanModeKit`。
3. **會改真機行為的只有兩個**（兩份已知的真實檔都量過）【量】：
   * `INDEX_SUCKER_TYPE`：檔案值 **1**（負壓機），移植樹恆為 **0**。
   * `ZSafePos`：檔案值 **50**，移植樹恆為 **20**。
   * 其餘 11 個沒讀的鍵，檔案值等於初值，或者使用端根本沒翻。
4. **D44 泵的先後關係成立，而且比 R14 講的更廣**【量】：
   * 泵不存在：39 個活的「吸真空請求」寫入點，讀取端是 0 個。
   * `bIndexCheck1` 只有一個設 true 的點，清除點是 0 個。
   * ⇒ 如果泵還沒翻就先載入 `INDEX_SUCKER_TYPE=1`，會有兩件事：負壓分支只下請求、不會真的吸；WAR1604 在第一次 Index check 之後就永遠不報。
5. **⚠ 新發現：這個前提今天已經可以被打破**【量】。
   網頁 HandlerSys 頁存檔（`FileRW/HSys.gen.inc:3579`）會把 `INDEX_SUCKER_TYPE` 即時設成頁面上的值。golden 在 `HandlerSys.cpp:600` 也是這樣做。
   頁面開啟時從檔案讀值（`HSys.gen.inc:3042`）⇒ 在負壓機上存一次檔（答「是」），就等於在泵還沒翻之前把它載入了。
6. 低嚴重度：Steven S12 的引用行號是 **V912** 的，不是 906。
   例：`HSys.cpp:68` 寫「golden main.cpp:1772」，906 是 `:1724`，差 49。
   內容本身相同（量過：兩棵 ctor 只差 V912 在 :1456-1463 多 8 行 AOI）。依 RULINGS §15 應改成 906 行號。

---

## 1. 逐鍵表（golden 906 ＝ `main.cpp`；移植樹快照 `6b942f15`）

缺鍵時 golden 的做法，代號如下：
* **A** ＝ `CheckAndReadIniDataGeneral`：補寫預設值，不問人（golden `common.cpp:1444-1455`）
* **B** ＝ `MessageBox(MB_YESNO)` 問操作員，把回答寫回檔案
* **C** ＝ 不問，直接寫固定值
* **D** ＝ 有鍵但值是 0，強制改寫成 1
* **E** ＝ `CheckAndReadIniData(asGeneralPath,…)` 字串版：補寫預設值；值是空字串時也補寫（`common.cpp:466-490`）

「兩檔值」＝ 本機 `D:\HT9045\system\Gerneral.ini`（09-22 13:47）／ `server/config-seed/Gerneral.ini`。

| # | 鍵 [區段] | golden 行 | 預設 | 缺鍵時 | 移植樹讀了沒（檔:行） | 變數初值（移植樹＝golden） | 使用端（移植樹活碼行數，主要檔） | 兩檔值 | 與 D44 泵 |
|---|---|---|---|---|---|---|---|---|---|
| 1 | CUSTOMER_CODE [System] | :1693（＋ASE 對映 :1700-1706） | 0 | A | ✔ `database.cpp:359`（ReadGeneralIni；golden 在 `database.cpp:327` 也先讀一次，ctor 再讀一次，結果相同） | `cmydef.cpp:3415` 0 | 2304 行／219 檔（全樹） | 790／868 | 無 |
| 2 | ZSafePos [In Arm] | :1695-1698 | CC_SCK→20，其他→50 | A | **✘ 沒讀** | `Motor/mymotor.cpp:81` `int ZSafePos = 20;`（golden `mymotor.cpp:41` 同值） | 63：`cinitial.cpp` 39（`Prod.ZInArmSafe`／`ZOutArmSafe`／`ZSortArmSafe`、`SetScreenScale`）、`mymotor.cpp:4449/4483/5355/5390`（Z 上升）、`WebMotorAccess.cpp:1043-1087`（單軸歸零 case 500）、aoutarm／ainarm／asortarm 的 Z 目標 | **50／50** | 無（和泵無關；但要在 InitialHandler 之前讀） |
| 3 | EP_Install [System] | :1724-1735 | 0 | **B**「Machine have install Electrons-Press(EP)?」（:1727） | ✔ `FileRW/HSys.cpp:69-80`；缺鍵時移植樹直接取 YES → 寫 1（:71-76） | `cmydef.cpp:3054` 0 | 26：`adam6024.cpp:131/159/163/516`、`AutoClean.cpp` 4 處、`ContactForceLoad.cpp:93/133/218`、`atester.cpp:8937`、`csystem.cpp:7474`、`cConfiguration.cpp:1536`、`forms/fContact.cpp:975` | 3／3 | 無（屬 Q8(c) EP 子系統） |
| 4-11 | EP_MAXKPA、EP_MAXA、EP_MINMPA、EP_MINA_FeedBack、EPDual_MAXKPA／MAXAFB／MINMPA／MinAFB [System] | :1737-1745（只在 EP_Install 鍵存在且 ≠0 時讀） | 499.0／5.013／0.001／0.908／899.0／4.905／0.001／0.968 | A（EP_Install 缺鍵那一輪不讀，維持 0.0） | ✔ `HSys.cpp:83-91`；另外 `ContactForceLoad.cpp:95-108`（golden `ContactForce.cpp:982-1003`） | `cmydef.cpp:3055-3071` 全是 0.0 | 各 3-7：`adam6024.cpp`、`atester.cpp`、`ContactForceLoad.cpp` | 例 EP_MAXKPA 900／899 | 無 |
| 12 | INOUT_ARM_PICKER_USE_MOTOR [System] | :1749-1766 | 1 | **C** 寫 1（題目 golden 自己註解掉，:1752-1753）；**D** 值=0 時強制寫 1（:1761-1765） | ✔ `HSys.cpp:95-112` | `cmydef.cpp:3416` 0（開機後被改成 1） | 115：`forms/fHome.cpp` 30、`forms/fMotorTest.cpp` 30、`mymotor.cpp` 12、`cinitial.cpp` 8、`csystem.cpp` 6 | 1／1 | 無 |
| 13 | SUPPORT_2_EMPTY_EMPTY [System] | :1768-1779 | 0 | B「…Empty-Unloader track(NO.7 Track)?」—— 但 golden `database.cpp:438` 已先補寫過，這題只在 bHandlerModel=false 時才會跳 | ✔ `database.cpp:547`（`HSys.cpp:113-115` 有註明） | `cmydef.cpp:3050` false | 11：`acatchtray.cpp`、`asendic_Empty.cpp`、`asendic_Auto2.cpp`、`WebBridgeTags.cpp` | 0／0 | 無 |
| 14 | ION_FAN_TYPE [System] | :1993-2006 | 1 | **B**「Machine Using KEYENCE's ION FAN?」（:1996） | ✔ `HSys.cpp:117-130`；缺鍵時取 YES → 寫 1（:121） | `cmydef.cpp:3078` 1 | 7：`csystem.cpp`（`:16447`／`:16666` DoSystem ION Fan 暫停） | 1／3 | 無 |
| 15 | **INDEX_SUCKER_TYPE** [System] | :2009-2021 | 0 | **B**「index sucker use negative press?（index真空產生器使用負壓系統?）」（:2011） | **✘ 開機沒讀**；⚠ 網頁 HandlerSys 存檔會即時設值：`HSys.gen.inc:3579` `INDEX_SUCKER_TYPE=…rgIndexSuckerType…->ItemIndex;` | `cmydef.cpp:3167` 0 | **112 行／14 檔**：`aTester_Rear` 29、`aTester_Front` 25、`atester` 25、`atester_32Site` 13、`ckernel` 6、`csystem` 6（含 `:16497` WAR1604）、`AutoClean` 4、`cinitial.cpp:623`（InitSucker） | **1／1** | **有：泵必須先翻，見 §2** |
| 16 | IndexTimeSet [System] | :2023-2032 | 0 | C 寫 0 | ✘ | `cmydef.cpp:3621` false | 1：`csystem.cpp:23961`（K15 用的假 index time，`random(25)`） | 0／0 | 無 |
| 17 | ASEK15UsePW [VENDER] | :2034-2043 | 0 | C 寫 0 | ✘ | `cmydef.cpp:3626` false | **0**（golden 用在 `main.cpp:27471`，那段移植樹沒翻） | 有鍵（值不抄） | 無 |
| 18 | ASEK15PassWord [VENDER] | :2045-2055 | 缺鍵時寫整數 0；有鍵時用字串預設（原始碼內建，不抄） | C | ✘ | `cmydef.cpp:3627` "" | **0**（golden `main.cpp:27476`） | 有鍵 | 無 |
| 19 | EPuser [VENDER] | :2057-2058 | 原始碼內建字串 | E | ✘（初值與 golden 預設相同，`cmydef.cpp:4616`） | 同左 | **0**（golden `cConfiguration.cpp:6643`、`HS_Function.cpp:3884`） | 有鍵 | 無 |
| 20 | EPPass [VENDER] | :2059-2060 | 原始碼內建字串 | E | ✘（初值＝golden 預設） | `cmydef.cpp:4616` | **0**（golden `cConfiguration.cpp:6644/6789`） | 有鍵 | 無 |
| 21 | ConfigPass [VENDER] → sPassWord | :2061-2062 | 原始碼內建字串 | E | ✘ | `cmydef.cpp:4893` "" | **0**（golden `cConfiguration.cpp:6793`） | 有鍵 | 無 |
| 22 | GigasFTPPassWWord [VENDER] → sGigasFTPPassWord | :2064-2068（只在 CC_GIGAS 讀） | 原始碼內建字串 | E | ✘ | `cmydef.cpp:5444` "" | **0**（golden `cConfiguration.cpp:7659`） | 缺／— | 無 |
| 23 | SetupFileCheckList [System] | :2070-2071 | "HisiATC_SetupCheckList_HT9045.dat" | A | ✘ | `cmydef.cpp:5210` "" | 2：`KYECFTP/FTPClient_Transfer.cpp:424/426`（只在 CC_KYEC_LEE 用；`Pos("")` 回 0，`vclcompat/AnsiString.cpp:52` ⇒ 永遠不命中） | 有／有 | 無 |
| 24 | bContaceTorque [System] | :2073-2082 | 0 | C 寫 0 | ✘ | `cmydef.cpp:3733` false | 4：`cinitial.cpp:19045/19092`、`uhome.cpp:2313/2369` | 0／0 | 無 |
| 25 | bUse_NewAutoCleanForm [System] | :2085-2094 | 0 | C 寫 0 | ✘ | `cmydef.cpp:3981` false | 3：`AutoClean.cpp:3626/4074/4087` | 0／0 | 無 |
| 26 | bUseNewCleanModeKit [System] | :2096-2105 | 0 | C 寫 0 | ✘ | `cmydef.cpp:4018` false | 1：`cinitial.cpp:15052`（AutoClean 取料 X 位置，屬運動） | 0／0 | 無 |

補充【量】：
* 「移植樹活碼行數」用只認字面 `#if 0` 的小腳本量（`.nb2_scratch/agent_tmp/q8b/live.py`），不含註解行、不含 `tests/`。
* 所有變數的初值，移植樹與 golden `cmydef.cpp` 完全相同。
* **鍵值有差、且會影響行為的只有 #2 和 #15。**
* #19-22 的值是密碼字串，本報告不抄；golden 預設值寫在原始碼裡。
* **HT9050 機台端的 Gerneral.ini 沒進版控**（`machines/HT9050/` 只有 IO_Table／Mot_Table／Pci1203*）⇒ 那台的值量不到。
  間接證據【推】：`machines/HT9050/IO_Table.csv:78-79` 的 `SnNegativePressureAir`／`SnNegativePressureAir2` Enable=1 ⇒ HT9050 很可能也是負壓機（`INDEX_SUCKER_TYPE=1`）。

---

## 2. D44 泵、`bIndexCheck1`、WAR1604 的相依鏈

「D44」是設定項 `IniConfig.bD44CheckIndexICDestroy`（Index 回黏檢查，golden `Config.h:538`）。
R14 所說的「泵」是 `Tfiosetview::ProcessIndexSuckDestroy1/2`（golden `iosetview.cpp:1922`／`:1881`）。
泵的工作：逐格讀 `bIndexSuck[][][]`（吸真空請求）與 `bIndexDestroy[][][]`（破真空請求），呼叫 `TMySucker::Suck()`／`Destroy()`，全部做完才回 true。
**負壓機的 Index 吸嘴就靠它執行吸／破真空。**

### 2.1 鏈上每一環（golden vs 移植樹）

| 環 | golden 906 | 移植樹 `6b942f15` | 狀態 |
|---|---|---|---|
| 設 `bIndexCheck1=true` | `atester.cpp:5708`（DoTestHeadMotor case 200000，真空自檢） | `atester.cpp:5777`，活碼。`iProcessIndexSuckDestroyCnt==0‖>15` 成立時設；瑞薩 FT-CT 沒開時計數永遠是 0 ⇒ **每次 Index check 都會設** | 【量】 |
| 清除點 1 | `atester.cpp:5719-5724` case 300000 `if(INDEX_SUCKER_TYPE==0 ‖ fiosetview->ProcessIndexSuckDestroy1()) { bIndexCheck1=false; …}` | `atester.cpp:5788-5794`：`#if 0 // TODO(W7)` 空區塊，然後直接 `Task=400000` | **缺** |
| 清除點 2 | `atester.cpp:5750-5772` case 600000 開頭：`CheckIndexArmInitState()` 互鎖、NULL_IC 吸嘴 `Normal()`、`bIndexCheck1=false; bIndexCheck2=false;`（:5769-5770） | `atester.cpp:5800` 起 case 600000 直接從 CCD 選擇段開始，golden :5750-5772 整段不在 | **缺** |
| 清除點 3 | `csystem.cpp:4536-4544` DoSystem：`if(bIndexCheck1 && fiosetview->ProcessIndexSuckDestroy1()) bIndexCheck1=false;` | `csystem.cpp:16546-16556` `#if 0 // GATE G17` | **閘** |
| 讀取端（WAR1604） | `csystem.cpp:4495-4533`：`if(INDEX_SUCKER_TYPE==1 && LastSet.iRealDummy!=DUMMY)` → 負壓氣源 `SnNegativePressureAir`／`2` IsOff → `if(bIndexCheck1‖bIndexCheck2){}` else `StopAllMotor(); ShowErrorMessage("WAR1604"…)` | `csystem.cpp:16497-16533`，逐行相同，活碼。DoSystem 由 MainProc 呼叫（`csystem.cpp:30055`），MainProc 跑在 server tick 上（`WebBridgeTags.cpp:632`） | 【量】 |
| 泵本體 | `iosetview.cpp:1881-1961` | **不存在**。引擎用的全域 `fiosetview` 是 `TfiosetviewShim`（`atester_shims.h:355-361`、`atester_shims.cpp:380`），它只有 `bIndexSuck[2][4][8]`。`forms/fIoSetView.h:603/631` 的真類別有 `bIndexDestroy`／`ResetIndexDestroy()`，但沒有泵，也沒有全域實例（`fIoSetView.h:83-93` 有說明） | 【量】 |
| 泵的替身 | — | **11 個回 true 的本地替身**：`atester.cpp:5517/5518`、`aTester_Front.cpp:207/1081/6985`、`aTester_Rear.cpp:409/7003`、`atester_32Site.cpp:205/206`、`AutoClean.cpp:288/289` | 【量】 |
| 泵的閘住呼叫點 | — | `aTester_Front.cpp:7030`（k7-G1）、`:8150`（K1F10）、`:8295`（K1F11）、`:9607`（W7F3-G02）、`:10375`（W7F3-G04）；`aTester_Rear.cpp:7052`（k8-G1）、`:8169`（K1G11）、`:8312`（K1G12）、`:9534`（G16）、`:10361`（W7R3-G03）、`:10636`（W7R3-G07）、`:10653`（W7R3-G08）；`csystem.cpp:16546`（G17）；`atester.cpp:4415` 起的 G-PT 區塊（:4538/:4755）與 `:9613` 起的 G-PTk 區塊（:9653） | 【量】 |
| 請求端 | 引擎在 `INDEX_SUCKER_TYPE==1` 分支下請求 `fiosetview->bIndexSuck[a][i][j]=true` | **39 個活的寫入點，0 個活的讀取點**（AutoClean 2、aTester_Front 9、aTester_Rear 12、atester 8、atester_32Site 8） | 【量】 |
| `bNeedCheck` 讀取端（D44 本身） | 真成員 | 仍然恆 false：`aTester_Front.cpp:197-199`、`aTester_Rear.cpp:359-360`、`atester_32Site.cpp:196-197`（R14 的結論，本快照重驗仍成立） | 【量】 |

### 2.2 如果先載入 `INDEX_SUCKER_TYPE=1`、泵還沒翻，會怎樣

1. **WAR1604 被永久關掉**【量＋推】。
   在第一次 Index check 之前它是活的。之後 `bIndexCheck1` 被閂在 true，沒有任何清除點。
   ⇒ 那次開機剩下的時間，負壓氣源失壓不會停機。
2. **負壓分支只下請求、不吸真空**【量碼，後果是推】。
   例：`aTester_Front.cpp:8135-8161`，`INDEX_SUCKER_TYPE==0` 時執行 `BTestSuck.Suck[i][j].On()`；`else` 時只設 `fiosetview->bIndexSuck[1][i][j]=true`，接著是 K1F10 的 `flag1=true`（:8155）。
   然後 case 10082（:8171）`if(DoTestYFrontDelay.Off() ‖ INDEX_SUCKER_TYPE==1)` 連 0.5 秒的等待也跳過，Z 軸直接動。
   同形的寫法在 39 個請求點上都存在。
3. **只翻 case 300000 還不夠**【推】。
   泵如果仍是回 true 的替身，`bIndexCheck1` 會被清掉、WAR1604 回來了，但真空自檢其實沒吸。
   這是一個看起來正常的假成功 ⇒ **泵一定要是真的。**
4. InitSucker 的負壓調整有時序要求【量】。
   `cinitial.cpp:623`（golden `:520`）在 `INDEX_SUCKER_TYPE==1` 時把 TestSuck 的 `OnAlarmTime` 除以 10、上限 50。
   它在 InitialHandler → InitHontechHardware → InitSucker 這條路上跑（`wb_serve.cpp:3118`；golden `main.cpp:9559` → `cinitial.cpp:5836/5818`）。
   ⇒ 載入點要在 `wb_serve.cpp:3118` 之前。`FileRW_HSys_ReadMainCtorKeys`（`:2942`）的位置是對的，與 golden「ctor → FormShow InitialHandler」同序。
   走 HandlerSys 存檔中途改值時，這個調整不會重跑。golden 也一樣：它的存檔會跳「Please restart the program…」，`HSys.gen.inc:3987`。
5. **今天就可以觸發**【量】。
   `HSys.gen.inc:3042` 開頁時讀檔（兩份已知檔都是 1），`:3578-3579` 存檔時寫檔並即時賦值。存檔要頁面回答「是」（`:3425` `ELAsk(...)==1`）。
   ⇒ 泵翻完之前，在負壓機上存 HandlerSys 頁，就等於第 1、2 點提前發生。
   這是 golden 行為照翻（golden `HandlerSys.cpp:599-600`）；問題出在泵沒翻，不是存檔錯。

---

## 3. 建議翻譯順序

依據：RULINGS 第 25 條「要翻；D44 泵在前」，第 37 條「D44 泵放哪、三個閘…由新電腦判斷」。

| 步 | 內容 | golden 906 | 放哪 | 風險／前置 |
|---|---|---|---|---|
| **1** | 補 11 個無相依的鍵：`IndexTimeSet`、`ASEK15UsePW`、`ASEK15PassWord`、`EPuser`、`EPPass`、`ConfigPass`、`GigasFTPPassWWord`、`SetupFileCheckList`、`bContaceTorque`、`bUse_NewAutoCleanForm`、`bUseNewCleanModeKit`。`CUSTOMER_CODE` 重讀＋ASE 對映可以一起放（冪等） | `main.cpp:1693`、`:1700-1706`、`:2023-2105` | `FileRW_HSys_ReadMainCtorKeys`，照 golden 順序排在 ION_FAN_TYPE 之後 | 純記憶體＋補鍵。兩份已知檔上值不變。**會補寫 Gerneral.ini 缺鍵 ⇒ 先備份**（CLAUDE.md 備份清單第一項）。順手把 HSys.cpp／wb_serve.cpp:2942 的行號改成 906 |
| **2** | `ZSafePos` | `:1695-1698` | 同一函式，放在 EP_Install 段之前（golden 順序）；必須在 `wb_serve.cpp:3118` InitialHandler 之前（cinitial 在 bring-up 時把它抄進 `Prod.Z*Safe` 與 `SetScreenScale`） | **運動變更**：已知檔上 Z 安全位置 20→50。也會改到網頁單軸歸零 case 500 的目標（`WebMotorAccess.cpp:1043-1087`）。`tests/test_web_motor_access.cpp:188` 用自己的 fake，不受影響。見待 Jimmy J3 |
| **3a** | 泵本體 `ProcessIndexSuckDestroy1/2` ＋ `bIndexDestroy` ＋ `ResetIndexDestroy` | `iosetview.cpp:1881-1961`、`:1861-1879` | 引擎實際解參考的那個物件（今天是 `TfiosetviewShim`，`atester_shims.h:355`）；`CosFunction.bD44Once4Suck` 已存在（`CosFunction.h:372`） | 第 37 條：新電腦決定 |
| **3b** | 11 個回 true 的替身換成真泵，並解開 §2.1 列的閘住呼叫點 | 各引擎原行 | 同 3a | 要與 3a 同一顆 commit，否則會出現兩個版本的泵 |
| **3c** | `bNeedCheck` 讀取端 3 處（R14 S3） | — | `aTester_Front.cpp:194-200`、`aTester_Rear.cpp:359-360`、`atester_32Site.cpp:196-197` | 真成員在 `mykitsuck.h:303` |
| **3d** | DoTestHeadMotor case 300000／400000／500000，以及 case 600000 開頭 | `atester.cpp:5718-5772`（含 `CheckIndexArmInitState()` 互鎖；case 400000 在 golden 沒有 break，會落到 500000） | `atester.cpp:5788-5809` | 這一步清掉 `bIndexCheck1/2`；**一定要在 3a 之後**（§2.2 第 3 點） |
| **3e** | csystem G17 | `csystem.cpp:4536-4544` | `csystem.cpp:16546-16556` | 依賴 3a |
| **3f** | TfMain::Timer1Timer 的暫停泵（fNote／MyMessageBox 開著時，或 `bIndexCheckNoStopVaccum && !SystemStart` 時，忙等泵完成） | `main.cpp:3098-3113` | 第 37 條：新電腦決定落在哪個執行緒 | 裡面是 `do{…MySleepEx(1,true);}while(…)` 忙等，要注意不能卡住 tick |
| **3g** | G13（`aTester_Rear.cpp` case 11035 的 `CopyFrom`） | — | `aTester_Rear.cpp:10957` 附近 | R14：一定要在 3a 之後 |
| **4** | `INDEX_SUCKER_TYPE` | `main.cpp:2009-2021` | `FileRW_HSys_ReadMainCtorKeys`，排在 ION_FAN_TYPE 之後（golden 順序），在 InitialHandler 之前 | 缺鍵行為見 J1。做完後在 `tests/test_machine_suckers.cpp:77` 那類測試加一個「泵有被呼叫」的對照 |

⇒ 簡單說：**1 → 2 → 3a～3g（泵家族，一起做）→ 4**。
步驟 1 和 2 跟泵無關，可以先做。J2 是步驟 3 做完之前的過渡期問題。

---

## 待 Jimmy

### J1：開機時 `INDEX_SUCKER_TYPE` 缺鍵怎麼辦
* **白話**：golden 會跳框問「Index 吸嘴是不是負壓？」，由人回答。網頁版開機時沒有人可以問。Steven 對 EP_Install／ION_FAN_TYPE 的做法是「取 YES＝寫 1」。這一鍵取 YES 就是負壓。
* **舉例**：一台正壓機換新硬碟、Gerneral.ini 缺這個鍵 → 自動寫成 1 → 走負壓分支，檢查一個不存在的負壓氣源感測器（WAR1604）。
* **選項**：
  * A 照 Steven 慣例，取 YES＝寫 1。
  * B 寫 0（等於移植樹今天的行為），並印出醒目訊息。
  * C 照 golden「問人」：開機不寫檔、維持 0，網頁跳 YES/NO（第 10 條那個通道）；答完才寫檔，並提示重開（因為 InitSucker 已經跑過）。
* **建議**：**C**。這一鍵影響真空怎麼作動，不應該猜。兩份已知檔都有這個鍵，缺鍵很少發生，發生時讓人決定最接近 golden。

### J2：泵翻完之前，網頁 HandlerSys 存檔會即時把 `INDEX_SUCKER_TYPE` 設成 1
* **白話**：這是照 golden 翻的（`HandlerSys.cpp:600`）。但因為泵還沒翻，在負壓機上存一次 HandlerSys 頁，就會讓 WAR1604 失效，負壓分支也不會吸真空。
* **舉例**：EastSun 在 HT9050 上改安全門數量、按存檔、答「是」→ 從那一刻起 `INDEX_SUCKER_TYPE=1`（`HSys.gen.inc:3579`）。
* **選項**：
  * A 照 golden 不動，另外通知機台端「泵翻完之前，不要在負壓機上存 HandlerSys 頁」。
  * B 過渡期把 `:3579` 的即時賦值閘掉（寫檔照做，重開才生效），註明是偏離 golden。
  * C 不處理，優先把步驟 3 做完。
* **建議**：**A＋C**（照忠實優先）。如果步驟 3 今天排不進去，改成 **B**。

### J3：`ZSafePos` 翻完後，Z 安全位置會從 20 變成檔案值（已知檔是 50）
* **白話**：In/Out Arm 所有 Z「抬到安全高度」的目標值都會改。網頁單軸歸零的最後一步也會改。
* **舉例**：`mymotor.cpp:5355` `InArmZMoveUp(ZSafePos, …)` 的目標由 20 變 50。單位與方向（哪個比較高）沒有量。
* **選項**：
  * A 照 golden 直接讀。
  * B 先讀並把值印出來，機台端確認 HT9050 的 `[In Arm] ZSafePos` 後才推。
  * C 不翻。
* **建議**：**B**。第 25 條已裁決「要翻」，所以問題只剩「推到 HT9050 之前要不要先確認值」。HT9050 的 Gerneral.ini 不在版控裡，我量不到。

---

## 無法驗證／限制

* HT9050 機台端 Gerneral.ini 的實際值：repo 裡沒有。只量了本機 `D:\HT9045\system\Gerneral.ini` 和 `server/config-seed/Gerneral.ini`。
* ZSafePos 的單位，以及 20 和 50 哪個比較高：沒量。
* 沒建置、沒跑 wb_serve：「HSys 存檔路徑可從網頁觸發」是讀碼（PageRegistrar `HSys.cpp:179-185`、`_editlist_sources.cmake:13`）得出的，沒有實跑。
* §2.2 第 2 點「Z 會在沒真空時就動」是從碼推的機台後果，沒有在機台上驗證。
* 活碼判斷只認字面 `#if 0`；`#ifdef SOFT_SIMULTE` 類的條件分支另外標成 cond，沒有按出貨／模擬兩種組態分開算。
* 移植樹 `atester.cpp:5797/5801/5805` 附近的註解引用「golden :5763-5780／:5783-5800／:5805-5990」，對不上 906，也對不上 V912（906 的 case 400000 在 :5726，V912 在 :5757）。來源不明，沒有追。
