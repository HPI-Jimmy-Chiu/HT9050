# JSON 橋接層擋在哪裡 —— 需要移植的部分

> 量測日期：20260923
> 量測人：Steven（AI 協作）
> 對象：Jimmy（移植波次排程）、JerryYang（C++ 翻譯）
> 移植樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`
>
> ⚠⚠ **golden 是哪一棵，視檔案而定** —— 這份文件裡每一條引用 golden 行號時都標了樹名：
> - `bUT150Install`／`main.cpp` 那條量的是 **V912**（`HT9011UC_Code_V3.33.912.0_20260908_Jimmy`）
> - `cTemperFrom.cpp` 的翻譯來源實測是 **V908**（`HT9011UC_Code_V3.33.908.0_20260702`），
>   見第三條的五行對照表
>
> 20260923 因為沒標樹名踩了三次，其中兩次進了對外的信。
> **跨樹引用不標樹名，得到的結論會是「看起來有憑有據的錯」。**
> - ⚠ 20260926：產生器曾經用的 `D:\HT9045_ref` 那份 V912 已退場（commit `3e0ebb92`），現在 V912 一律指主 repo
>   `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`。兩份的 `main.cpp` 在 `:4795` 之後差 2 行、
>   `uYieldMonitoring.cpp` 在 `:899` 之後差 20／`:1057` 之後差 22（主 repo 較多）；20260926 下午以前寫的行號多半是
>   HT9045_ref 的，引用時要分清楚（`generators.md` 二十）。

---

## 這份文件在講什麼

JSON 橋接層（S0～S7）做到今天，**介面層已經可用**：282 個編譯期定義、
1,127 個結構欄位、71 個溫控通道、事件留痕環，全部有端點、有 schema、有 probe。

但其中很大一部分**送出去的值是 null**，不是因為橋接層沒寫好，
是因為**填那些值的 golden 程式碼還沒被翻譯到移植樹**。

橋接層對這件事的處理方式是**誠實標示而不是填 0**
（`sourcePorted` 旗標、`fields.<f>.live`、`anyLive`），
所以瀏覽器分得出「沒載入」和「值就是 0」。
這份文件把「還沒翻譯的那些」逐條列出來，讓移植波次可以排序。

⚠ **共通原則**：這裡每一條的現象都是「值是假的」，
但**修法完全不同**。混淆這三種會做白工：

| 型態 | 現象 | 修法 |
|---|---|---|
| **A 完全沒翻譯** | 函式在移植樹不存在 | 翻譯它 |
| **B 翻譯了但沒人叫** | 函式有本體、標 ACTIVE，零呼叫點 | 解開呼叫端 |
| **C 翻譯了但被閘擋住** | 本體在 `#if 0` 裡 | 退役那個閘（要先確認閘的理由消失了） |

---

## 一、⛔ 最高優先：`main.cpp` 整支不存在（型態 A）

**⛔ 部分過期（20260926，Steven 團隊核對 HEAD 8fad1522）**：「`main.cpp` 整支不存在」只剩**檔名**成立（移植樹沒有
`main.cpp`）——golden `TfMain` 的成員已陸續翻進別的檔：`forms/fMain.cpp`（49 個 `TfMain::` 定義，例
`ChangeTesterConnect :1096`，Steven02 `979eac6b`）、`WebRecipeChange.cpp`、`MainTempMode.cpp`、`FileRW/MainBoot.cpp`、
`FileRW/MainClick.cpp`、`tools/wb_serve.cpp` 的 `W906_DoReadLastData`（golden `TfMain::DoReadLastData`）等。
**本節的主體結論不變**：`bUT150Install[]` 的填值來源 `Index16Heater`／`IndexHeatMode`／`Tri_Temp_Set_Site`／
`HotplateHeatMode` 在移植樹仍 **0 個本體**（`git grep -nw` 20260926，只剩 `JsonBridge/StageThermo.*`、`MainCalcCore.h`
的註解與說明字串）。來源：St01 唯讀普查 20260926 17:xx（HEAD 8fad1522）第 4 節，本節逐條複查過。

**影響**：溫控的「哪些通道有裝加熱器」完全是假的，連帶整個 S7 的 `inst` 欄位。

`bUT150Install[]` 決定 71 個溫控通道裡哪幾個是真的存在。實測 golden V912
**`main.cpp` ＋ `csystem.cpp` 的 1,096 個活賦值**分布（全樹 raw grep 是 1,103，
另含 `cmydef.cpp:3528` 的定義初值 1 與 6 條被註解掉的）：

| 檔案 | 函式 | 賦值數 |
|---|---|---|
| `main.cpp` | `TfMain::Index16Heater(bool)` | **931** |
| `main.cpp` | `TfMain::IndexHeatMode()` | 71 |
| `main.cpp` | `TfMain::Tri_Temp_Set_Site()` | 36 |
| `main.cpp` | `TfMain::HotplateHeatMode()` | 28 |
| `main.cpp` | `TfMain::Timer2Timer()` | 18 |
| `main.cpp` | （建構子） | 1 |
| `csystem.cpp` | `OpenDUTHeat(bool)` | 11 |

**`main.cpp` 在移植樹整支不存在。** 也就是 1,085 / 1,096 ≈ **99%** 的填值來源缺席。

移植樹現況：
- `OpenDUTHeat` —— **已完整翻譯且標記 ACTIVE**（`csystem.cpp:23955` 的翻譯表），
  但**零呼叫點**（型態 B）。這只佔 1%。
- `Index16Heater` —— **不存在**。唯一痕跡是 `MainCalcCore.h:94` 的註解，
  稱它是 golden **V912** `main.cpp:19229-20881`（約 1,650 行）的巨獸，
  內含 32 組 EJ1N site-map 邏輯。
- `HotplateHeatMode` —— 0 命中。
- `IndexHeatMode` —— 移植樹有 287 個命中，但**全部是 `Temperature.iIndexHeatMode`
  這個設定欄位，與 `TfMain::IndexHeatMode()` 同名不同物**。查的時候要小心。

⇒ **建議**：`Index16Heater` 是這一條的主體，建議單獨排一個波次。
不要只解 `OpenDUTHeat` 的呼叫點——那會讓人以為修好了，實際只動到 1%。

---

## 二、⛔ 溫控讀值：執行緒存在但沒有人啟動它（型態 B）

**影響**：`temp.pv`、8 個 legacy `zone.*`、S7 的 71 個 `temp.zone.<ch>.pv`
與 `.comm`，全部是 null。

鏈路（全部已翻譯，實測）：

```
uHeaterThread.cpp:392  DoThermo()
    -> bthermo.cpp:1284   DoThermoReal()
        -> bthermo.cpp:3442  UN150Read[Addr] = DOUN150ReadTemp(Addr)
        -> bthermo.cpp:2796… UN150CommError[Addr] = …
```

`DOUN150ReadTemp` 的本體在 `bthermo.cpp:4532`，也翻譯好了。

**缺的是啟動**：全樹搜尋 `HeaterThread` 的建構／啟動點 —— **零命中**
（只有其他檔案的註解在引用它）。`tools/wb_serve.cpp` 對 `HeaterThread`
與 `DoThermo` 都是零提及。

⇒ **建議**：這條的修法是把 heater 執行緒接進開機序列，
比第一條便宜很多，而且做完之後 S7 的 `pv`／`comm` 兩欄立刻變活
（橋接層那邊只要把 `JsonBridge/StageThermo.cpp` 的 `kThermoFields`
兩個 `live` 旗標改 true，並加上讀取分支）。

⚠ 但這條有**方向性風險**：一旦 `UN150Read[]` 活了而
`kThermoFields` 沒跟著改，schema 會繼續說「不活」而值已經是真的，
**瀏覽器會把真溫度當成佔位符丟掉**。改動時兩邊要同一個 commit。

---

## 三、溫控超溫告警升級被閘擋住（型態 C，SAFETY）

移植樹 `cTemperFrom.cpp:977` 有一個 `#if 0 // GATE (T1)`。

⚠⚠⚠ **先講一件比這個閘更重要的事：`cTemperFrom.cpp` 的翻譯來源是 V908，不是 V912。**

移植樹 `cTemperFrom.cpp:12-16` 的 WAVE SCOPE 引了五個 golden 行號，逐棵比對：

| 引用 | V899 | **V908** | V912 |
|---|---|---|---|
| `ShowThermo :586-1470` | :587-1445 | **:586-1470** ✅ | :587-1480 |
| `SetShowYield :1473` | :1448 | **:1473** ✅ | :1483 |
| `ShowHotName :1772` | — | **:1772** ✅ | — |
| `TempRunShowAlarmHigh :1917` | — | **:1917** ✅ | — |
| `GATE(T1) :1294` | — | **:1293** ✅ | :1303 |

**五個全中 V908**，沒有一個中 V899 或 V912。

而 `SOFT_SIMULTE` 在各棵 golden 的狀態**不一樣**（實測）：

    V899   //#define SOFT_SIMULTE    關
    V908     #define SOFT_SIMULTE    **開**   ← 翻譯來源
    V912   //#define SOFT_SIMULTE    關

⇒ 對**真正的翻譯來源 V908** 而言，那段本來就被 `#ifndef` 拿掉了，
`#if 0` 是**忠實的**，不是翻譯者引入的偏離。

而且移植樹**沒有把 `#ifndef SOFT_SIMULTE` 換掉** —— `#if 0` 是巢狀在它裡面的
第二層（`:968` 的 `#ifndef` 包著 `:977` 的 `#if 0`）。移植樹自己的
`MachineType.h:64` 預設就 `#define SOFT_SIMULTE`，所以**預設 build 外層就已經
把整段拿掉**，`#if 0` 只有在 `-DW906_NO_SOFT_SIMULTE=ON` 才咬得到。

⇒ **真正的落差是 V908 → V912 的版本差**（V912 把 `SOFT_SIMULTE` 關掉了，
所以 V912 量產機會發這個警報），**不是翻譯者引入的**。
修法要兩步：**先裁決要對齊哪一棵 golden**，再談退不退 T1 ——
只退 T1 會被外層的 `#ifndef` 繼續遮住。

T1 的 SAFETY 分類仍然成立，變的是理由不是結論。

**界線要講清楚（三層，不要混）**：

1. **升級被閘擋住**：`WAR15xx` 的「連續 3 秒後升級成機台警報」在移植樹是啞的。
2. **分類的程式碼沒被閘擋**：`iTempOverShowAlarmT[]`（over=1／below=2／ok=0）
   的寫入在 `cTemperFrom.cpp:796,800,810,819,823,833`，不在任何 `#if 0` 裡。
3. **但分類事實上也沒在跑**：`ShowThermo` 在移植樹**零生產呼叫點**
   （全樹 4 個命中全在 `tests/test_temperfrom_core.cpp`；
   golden 的驅動 `TfTemperFrom::Timer1Timer` 沒有被翻譯）。
   ⇒ 所以「分類活著」指的是程式碼沒被 gate，**不是**它有在執行。

⚠ 第 2、3 層合起來是一個陷阱：`iTempOverShowAlarmT[]` 是真的全域
（`extern cmydef.h:5018`，定義 `cmydef.cpp:3528`），**看起來可以直接讀**。
但它永遠停在零初值，而 **`0` 就是 `"ok"`**。
照著「它是可讀全域」去接，會送出 71 個「一切正常」給從來沒被量過的通道。
這是 null-vs-0 規則換了件衣服：可讀的東西存在，但它的 0 跟「從沒算過」是同一個位元樣式。

這個閘的分類是 **SAFETY**，不是缺相依（`ShowErrorMessage`／`MMSystem`／
`SystemSec` 都已經在且接好了），所以**要由人明示同意才退役**，不是順手打開。

⚠ 還有一層條件不要漏：那段升級**本來就不是無條件的**。
V912 `:1293-1295` 外面包著

    if(LastSet.iTemperature==Tempture_Ambient
       && Temperature.bAmbientGuardbandCheck
       && IniConfig.bL20AbientGuardBand)

而且 `SystemStart==false || iHome!=0` 會把計秒歸零。
⇒ 正確說法是「V912 量產機**在常溫模式、配方開 guardband、Config L20 開**時會叫」，
不是「一定會叫」。

⇒ **對 JSON 契約的影響已經寫進 `JsonBridge/StageThermo.h` 檔頭**：
將來 `temp.zone[ch].state` 送出 `"over"` 時，**移植樹**的機台不會因此發警報。
HTML 端會很合理地假設「畫面紅了 = 機台已經知道了」，在移植樹上不成立。

⚠ 另外 `ATC/ATCSystem.cpp:527-720` 有一整批 `WAR15xxx` 是 ATC 自己的告警，
與這條無關，查的時候不要混在一起。

---

## 四、`TfContact::ReadFile()` 沒有本體（型態 A）

**影響**：`deviceForm.file` 綁定的 **66 個欄位全是 0**，而 0 在這棵樹上是
合法的座標、合法的模式、合法的壓力。

golden `cContact.cpp:365` 的 `TfContact::ReadFile()` 負責把 Contact.Data
讀進 `DeviceForm_File`。移植樹**沒有這個函式的本體**（實測 grep 零命中，
只有橋接層自己的註解在引用它）。

橋接層的處置：`JsonBridge/Bindings.cpp` 給這個綁定標了
`sourcePorted = false` 與 `filledBy = "golden TfContact::ReadFile() @cContact.cpp:365"`，
所以 `GET /api/struct/deviceForm.file` 會誠實回報那 66 個 0 不可信。

> **✅ 結案（Steven 20260924）**：JerryYang `ca4e903` 把 `TfContact::ReadFile()` 翻進
> `forms/fContact.cpp:1475`，並接上 wb_serve 開機序列（log「contact.* chain loaded」）。
> 實測 QPM5577_8：`deviceForm.file` 的 `IndexContact = [-134, -134.28]`、`ForcePerPinN = 0.2156`，
> 與 Contact.Data 一致；`deviceForm.live` 經 `DoStructUnitConvert()` 為 `[-13400, -13428]`。
> `Bindings.cpp` 的 `deviceForm.file`／`deviceForm.live`／`testIF.live` 已改 `sourcePorted = true`
> （`testIF.live` 仍有「10 個 ×100 欄位只翻 6 個」的缺口，說明字串照實保留）。

---

## 五、權限名稱層被閘擋住（型態 C，gate SEC1）

`cSecurity.cpp:71-262` 是 `#if 0`。裡面是 `mySecurityPal.push_back(new TMySecurity(...))`
那一整排，也就是**每個權限項目的名字**（`"[00] Main - Tools"` 這種）。

實測數量：移植樹 `[00]`～`[178]` 共 **179 筆**。
（⚠ 直接 `grep -c push_back` 會得到 180，因為 `:930` 是一行註解。）

⚠⚠ **「移植樹比 golden 少一筆」是錯的說法** —— 又是拿錯棵 golden 比：

    V908（翻譯來源）  179   ← 與移植樹**完全吻合**
    V912              180

⇒ 名字的**數量**沒有缺口，缺的只是那 179 筆被 `#if 0` 關著。
V912 多的那一筆是版本落差，跟這個閘無關。

**影響**：`levelSet` 綁定的 `AccessLevel[256]` 只有數字沒有名字，
瀏覽器顯示權限清單時沒有可讀標籤。

⚠ 另外一條獨立的問題：`levelSet` 的 `AccessLevel[256]` **實測全是 0**，
但橋接層標 `sourcePorted = true`。`GetLevelSet()` 在移植樹有本體，**也有呼叫點**
（建構子 `cSecurity.cpp:271`、`FormShow :328`）—— 但 **wb_serve 從沒建構過 `TfSecurity`**
（`tools/wb_serve.cpp` 零處 `new TfSecurity`），所以兩個呼叫點都沒機會跑（型態 B，
卡的不是函式而是整個表單物件）。「開機沒被呼叫」的推論成立：否則 `:1668` 那條
`[163]`「一律 3」的鉗制會留下痕跡。
⇒ 這是「`sourcePorted = true` 仍不足以保證值是真的」的第一個實例：
那個旗標說的是「函式存在」，不是「開機時有跑」。

---

## 六、留痕（RecordProcess 族）三個入口都不是真的

**影響**：`log.event` 收到的瀏覽器操作留痕，實際上沒有落到任何持久化的地方。

三個入口的移植樹現況：

| 入口 | 現況 |
|---|---|
| `canary_support.cpp:116-124` | 只 `printf` 到 stdout |
| `acatchtray_shims.cpp:152` | 空 body `{}` |
| `common.cpp:62` | **TU-local `static RecordProcess(...) {}`**，吞掉 41 個呼叫 |

⚠ `common.cpp:62` 那個最值得注意。它吞掉的 41 個呼叫分三類（實測）：

| 類別 | 數量 | 內容 |
|---|---|---|
| INI 族 | 27 | `Read NULL INI on …` 11、`Read _Mem` 5、`Replace` 1、`Write NULL INI on …` 10 |
| **例外** | **11** | `"Exception …"` |
| 目錄 | 2 | `"Directory value is NULL!"` |

⇒ 不只是**設定檔讀寫失敗無聲**，**例外也被吞掉** —— 後者更嚴重，
因為那 11 個是程式已經知道出事了、正要留痕的時候被靜音。

它自己的 TODO 寫 `TODO(wave-cMyDB): remove this stub once cMyDB is translated and linked.`

相關的下游也還關著（型態 C）：
- `cMyDB.cpp:999` `#if 0`（`TODO(GA1-B4-integrate)`）—— `MyDBIProcess` 的真本體。
  實際連進 exe 的是 `aHotPlateSubstrate.cpp:1264` 的計數器替身。
- `cMyDB.cpp:276` / `:344` / `:400` `#if 0`（`TODO(GA1-B4)`）—— 事件記錄相關，
  卡在 `slEventLog` 的 `TMyStringList` 型別還不透明。
- `cpublic.cpp:778` `ProductionLog()` —— 寫 `asProductionLogPath\*.logs`，也在閘內。

⚠ **這條對 JSON 橋接層有安全意涵**：`log.event` 目前不在權杖豁免名單的管制內
（它是被豁免的），理由寫「代價是任何瀏覽器都能寫 ring，可接受，因為 ring 有界（200 筆）」。
**那個理由成立只因為 `cMyDB.cpp:999` 這個閘還關著。**
那個閘一退役，未持權杖的瀏覽器就同時取得 sqlite 與檔案寫入兩條路，兩條都無界。
⇒ **退役 `GA1-B4-integrate` 的時候，必須同時重新檢視 `log.event` 的豁免。**

---

## 七、`fTemperFrom` 單例不存在（型態 A，影響 S7 v2）

`forms/fTemperFrom.h:46` 明說這一波**不宣告** `extern TfTemperFrom *fTemperFrom;`，
因此 `cSetUp.cpp:1103` 與 `:1209` 兩處是 `#if 0`（`GATE(G-SU-TemperFrom)`）。

**影響**：S7 現在不能驅動 `ShowThermo` 去算 `ready`／`state`／`sv` 三個結論欄位。

好消息是 `TfTemperFrom` 的 panel 成員都是 NSDMI `= new TPanel()`
（`forms/fTemperFrom.h:339-362`），所以理論上自己 `new` 一個就能驅動，
ctor（`cTemperFrom.cpp:139-196`）會把 71 個 panel 填進全域 `ShowTempComp[]`。

⚠ 但 `ShowTempComp[]` / `NameTempComp[]` 是**全域**（`cTemperFrom.cpp:108-109`），
第二個實例會互蓋。所以要走這條路，必須是**單一、長生命週期**的實例。

---

## 八、S7 專屬：`sv` 上線時有 15 個通道 golden 從未賦值

不是移植缺口，是 golden 本身的行為，但**會在 `sv` 欄位上線那天咬人**，先記著。

`cTemperFrom.cpp:321-333` 的 tri-temp else 分支是一個 `for(i<14)` **搜尋**
（不是 if/else 鏈）。`Addr` 不在 `SITE_Ch[14]` 裡的時候 `SetTemp` 一次都沒被寫，
接著在 `:369` 被拿去比較（`:364` 那個讀取在 `bNow_Is_ATC==true` 的 if 內，
這個 else 分支走不到；第一個真的讀到未賦值 `SetTemp` 的是 `:369`）。

受影響的 15 個通道：`tcHead1-4`、`tcSocket`、`tcChamber`、`tcCCD`、
`tcHeatGun1/2`、`tc2D`、`tcLB`、`tcIndexESD`、`tcCCD_2`、`tcLBUp`、`tcLBDown`。

golden 是 `double SetTemp;`（未初始化）→ 讀未定義值；
移植樹加了 `=0.0`（`cTemperFrom.cpp:257-263`）把它變成確定的 0。
**程式改動本身是良性的**（甚至更好）。

⚠ 但移植樹 `:257-263` 那段註解宣稱
「Every path through this function's if/else-if chains assigns SetTemp … before reading them」
—— **這句話是錯的**，會讓下一個人以為 `sv` 永遠有意義。

⇒ **對 S7 的具體要求**：`sv` 真的 stage 的那一天，
這 15 個通道必須送 **null**，不能送 0。
這是 null-vs-0 規則第一次落在**翻譯過的程式碼內部**而不是 tag 層。

---

## 九、S7 專屬：`bTemperatureReady[]` 的初值本身就是壞的

`cTemperFrom.cpp:238`：

```cpp
static bool bTemperatureReady[tcTotalCount]={true};
```

C++ 聚合初始化只把 `[0]` 設成 true，**其餘 70 個是 false**。
golden 同寫法、同結果。而註解說的意圖是「沒有使用溫控器要當讀到」
（真正補 true 的是 `:903` 與 `:1154`）。

它是 `ShowThermo` 的 function-local static，沒有 extern，而且**只寫不讀**
——golden 也一樣（grep 整棵 golden，每個命中都在這個函式內、都在賦值左邊）。

⇒ **它從來沒被讀過，所以從來沒有人發現它的初值是錯的。**
哪一天要把它當 tag 送出去，S7 就是它的**第一個消費者**，
等於同時要決定「照翻（`[0]` 特別）還是修」——
**那是 golden 行為裁決，不是橋接層可以順手決定的。**
今天不 stage 是刻意的。

---

## 十、~~`iTo3Unload[]` 宣告了但沒人填~~ ⛔ **整節作廢（20260923 同日撤回）—— 它早就填好了**

> **這一節從頭到尾是錯的**，撤回原因寫在這裡，原文保留在下方（本檔慣例：更正不改寫原句）。
>
> `iTo3Unload[]` **有被填**，而且是 **20260817** 就填好的：
>
> | | 檔:行 |
> |---|---|
> | 填值的 same-TU static initializer | `cmydef.cpp:4281` `struct W906_TrayIndexMapInit` |
> | `iTo3Unload[]` 的 33 筆賦值 | `cmydef.cpp:4386` 起（`iTo3Unload[eAuto1]=e3Auto1;` …） |
> | 實例（讓建構子真的跑） | `cmydef.cpp:4431` `W906_TrayIndexMapInit g_w906TrayIndexMapInit;` |
> | 有進 link 的證據 | `nm build/CMakeFiles/ht9045_globals.dir/cmydef.cpp.obj` 同時有 `__ZN12_GLOBAL__N_122g_w906TrayIndexMapInitE` 與 `_iTo3Unload` |
>
> ⇒ QtyLog 六格**不會**全寫 `BinCT[0][0]`；下面列的 6 個「受害使用點」
>   （`aoutarm.cpp:3332`、`asortarm.cpp:4024/:4071`、`AutoRetest.cpp:1848`、
>   `cBinSel.cpp:1131/:1263/:1265`）**一個都不成立**。排序建議表的順位 7 一併撤銷。
>
> ⚠⚠ **怎麼錯的，值得記下來**：本節自己附的複驗指令是
> ```
> grep -rn "iTo3Unload *\[[^]]*\] *=" --include=*.cpp . | grep -v "^./build/"   # 應為零
> ```
> 照跑會得到 **33 筆**（光 `cmydef.cpp` 就 33 筆）。
> **也就是說：驗證指令寫好了，但沒有真的跑過。**寫下一條「複驗指令」而不執行它，
> 比不寫更危險 —— 它讓後面每一個讀者都以為這條被驗過了。
>
> ⓘ 錯誤來源不是憑空捏造，是**抄了過期敘述**：`WebBridgeTags.cpp:899-904` 至今仍寫著
>   「DECLARED but never initialized … reads all-zero today」，那段寫於 20260817 的 Fix1 **之前**。
>   ⇒ 汙染源也要一起修，否則下一個人會再抄一次（見待辦 #14）。

<details><summary>以下為已作廢的原文（保留供稽核）</summary>

### ~~十、`iTo3Unload[]` 宣告了但沒人填 → QtyLog 六格全寫同一個 Bin（型態 A）~~

**現象**：`act.main.clarnData` 寫出的 QtyLog 那一行，六個 Tray 的分類計數**全部是同一個值**
（`BinCT[0][0]`），看起來像「六個站的產出一模一樣」。

**原因**：`iTo3Unload[]` 在移植樹只有宣告，**從未被賦值** ——
golden 是在**還沒翻譯的 `TfMain` 建構子**裡填的。未初始化的全域讀出來是全 0，
所以 `LastSet.BinCT[0][iTo3Unload[i]]` 六次都變成 `LastSet.BinCT[0][0]`。

| | 檔:行 |
|---|---|
| 受害的寫入點 | `JsonBridge/actions/MainClarnData.cpp:154`、`:156`（golden `main.cpp:15476`／`:15478`） |
| 早就記錄過這件事的地方 | `WebBridgeTags.cpp:899-904`（「DECLARED but never initialized … reads all-zero today」） |
| **不只影響 QtyLog** | `aoutarm.cpp:3332` 的 `CheckOutArmXYScaleByAutoTeach(..., iTo3Unload[iWhichAuto])` 也吃這個全 0 陣列；`asortarm.cpp:4024`／`:4071`、`AutoRetest.cpp:1848`、`cBinSel.cpp:1131`／`:1263`／`:1265` 同 |

⚠ `WebBridgeTags.cpp` 當時的做法是**繞開它**（直接用 `e3Auto1..e3Fix3` 常數），
  所以 `sort.*.count` 那 6 個 tag 是對的。`MainClarnData.cpp` **沒有繞**，
  因為它是 golden 的逐字翻譯 —— 繞開就不是翻譯了。
  ⇒ 這一條要修的是**填值的那段**，不是這兩個使用點。

**修法**：翻譯 `TfMain` 建構子裡填 `iTo3Unload[]` 的那段（與第一條 `main.cpp` 同一個波次）。

---

</details>

---

## 十一、`fSortCT->ShowLoadingIC()`／`ShowSortIC()` 是 no-op facade（型態 A）

**現象**：`act.main.clarnData` 回 `executed:true`，但 `LastSet.SendCT[2]` 與
`RunInfo.iUnloadCount` **不會動**。畫面上「送出/卸載總數」永遠停在原值。

**原因**：golden `cSortCT.cpp` **整支沒有翻譯**（這棵樹沒有 `cSortCT.cpp`），
`forms/fSortCT.cpp` 的這兩個方法是離線空殼。

**這兩個 golden 本體不是純 UI**（所以空掉不是中性的），`forms/fSortCT.h:45-63` 已寫明：

| golden 本體 | 空掉之後少做了什麼 |
|---|---|
| `ShowLoadingIC`（golden `cSortCT.cpp:210-279`） | Auto-Site-Map 時不再清 `LastSet.SendCT[2]`／`SendCT_ART[2]`；`TestIF.bContinuousLoader`／`bContinuousLoader_RT` 兩臂的 **WAR07324 不會升起**，`ProcessPiggyBackFunction()`（`iWhoTriggerPiggyBack=pbtContinualLoader`）**不會觸發** |
| `ShowSortIC`（golden `cSortCT.cpp:345+`） | 不再把 `LastSet.BinCT[0][]` 匯總進 `RunInfo.iUnloadCount`／`iUnloadCount_ART`；`iSECSGEMPass`／`iSECSGEMFail`／`iATRPassCount`／`iATRFailCount`／`iATRTotalCount` 全部保持原值 |

⚠ 對測試的後果（`fSortCT.h` 原文）：**「一個期待 sort 之後 unload count 會動的測試，
  必須自己直接驅動那些計數器，不能靠這個呼叫。」**

⚠ 另有第二個閘：TU-local 的 `W7C2_FSORTCT_SHOWLOADING()`／`W7C2_FSORTCT_SHOWSORT()` 巨集
  把**同樣這兩個 golden 呼叫**也擋住了。解閘那天**兩處要一起**，只解一邊會得到「一半有匯總一半沒有」。
  ⚠ 行號（20260923 實測，**不要用 `fSortCT.h:64` 寫的 `csystem.cpp:2820-2821`，那是過期值，
    指到的是 `iLongTimePauseCount`／`bLongTimePause`，與 fSortCT 無關**）：
    定義 `csystem.cpp:6631-6632`；呼叫點 `csystem.cpp:8296-8297` 與 `:8381-8382`。
    來源 `forms/fSortCT.h:64` 也要一起更正（待辦 #17）。

**修法**：翻譯 `cSortCT.cpp`（型態 A），同時退役 `csystem.cpp` 那兩個巨集（型態 C）。

---

## 十二、`MyForceDirectories(QtyData\YYYYMM)` 跑在守衛前面 —— **這不是缺陷，是忠於 golden**

放這裡是為了**擋掉未來的「順手修好」**，不是要排波次。

`ClarnDataBody()` 的前四行順序是：

```
asPath.sprintf("%s%04d%02d\\", asQtyDataPath, SystemYear, SystemMonth);   // MainClarnData.cpp:143  (golden :15465)
MyForceDirectories(asPath);                                               // MainClarnData.cpp:144  (golden :15466)

if (IniConfig.bA61DisableCleanMUBA == true)                               // MainClarnData.cpp:146  (golden :15468)
    return;                                                               // MainClarnData.cpp:147  (golden :15469)
```

⇒ **即使 `bA61DisableCleanMUBA` 開著（整支立刻 return、一個字都不寫），
  `D:\HT9045\QtyData\YYYYMM\` 這個目錄還是會被建出來。**

golden 就是這個順序（`main.cpp:15465-15469`），所以**照翻**。
把 `MyForceDirectories` 移到守衛後面會比較「乾淨」，但那是**改行為**，
依 `gl-wave-loop` 的忠實判準要使用者裁決，不是翻譯者自己決定。

⚠ 觀察到的副作用：跑 `act.main.clarnData` 的 probe 會在量產機上長出空月份目錄。
  這是預期的，不要當成 bug 去追。

---

## 十三、`ATC.ini` 沒被讀 → 開機把配方的 `Chiller Temp` 改寫掉（型態 A，**會寫壞配方檔**）

**⛔ 已結案（commit `322d68a3`，20260925 08:49；記錄員 20260926 15:3x 核對）**：
`forms/fATCHandlerSide.cpp:625` 已補 `W906_ReadATCIni()`（讀 `system\ATC.ini` 填
`iATC_MODE_TYPE`），開機序列已呼叫——`tools/wb_serve.cpp:3123`：
`if (bBoot) { extern void W906_ReadATCIni(); W906_ReadATCIni(); }`，註解標明「golden
main.cpp:9794-9797 ReadATCModeType，早於 DoReadLastData→ReadTempFile」，與本節「修法」
一節要求的時機一致。下文原始症狀描述、根因分析保留供對照，**不代表現況**；未重新複驗
（開機前後 SHA256 比對）。

Steven 20260924 實測（配方 QPM5577_8，wb_serve @ 038a9b7）。

**症狀**：wb_serve 開機後 `Temperature.Data [ATC] Chiller Temp` 從 **-20 變成 5**，寫回檔案。
每次開機都會發生；網頁 `/api/recipe/temperature` 也跟著顯示 5。

**根因**：`system\ATC.ini:2` 是 `iATC_MODE_TYPE=33`，但移植樹的 `TfATCHandlerSide`
只在建構子把它設成 `ATC_TYPE_UNSET`（`forms/fATCHandlerSide.cpp:177`、`.h:723` 註解「ctor-only this wave」），
**從來沒有讀 ATC.ini**。`TfTemp_Set::ReadTempFile()`（`uTemp_Set.cpp:2748-2767`）照 `iATC_MODE_TYPE`
挑鉗制範圍：`ATC_TYPE_33` 是 `CheckRange(..., 30, -20)`，其他走 `else` 的 `CheckRange(..., 40, 5)`。
UNSET 落進 `else`，-20 被夾成 5，再由 `:2796-2797` `WriteIniData(szDir,"ATC","Chiller Temp",...)` 寫回。

**翻譯本身和 golden 逐字一致**（golden `uTemp_Set.cpp:2562-2613`），缺的是 ATC.ini 的讀檔。
golden 在同一台機器上會保留 -20。

**這不是 ca4e903 造成的**：`temp.*` 鏈自 20260820（W906-FW-TEMP2）就在開機路徑上，當時寫到 `--dry` 的暫存夾；
20260917 A1 之後改寫真實配方夾。沒有拿舊 exe 比對過，只能說「至少從 A1 起」。

**同一次開機的其他寫檔**（都是 golden 行為，**不是**缺陷，列出來方便比對）：
`Contact.Data` 補 `bUseDieForce=0`、`Tester.Data` 補 `iQAD22DoubleContactCount=1`（Jerry Q-4 請 Steven 確認的種值路徑，實測會觸發）、
`Temperature.Data` 補 `UseTC2Offset=0`、`Binasgn_ART.Data`／`BinasgnOff_ART.Data` 補 `BinTrayLinked(...)`／`[Auto4..6]`／`[Fix7..12]` 等；
另外建立空的 `IniData\Offset\<配方>\`（golden `cOffSet.cpp:2019` `MyForceDirectories`）。

**修法**：把 golden 讀 `ATC.ini` 的那段（`ATC_InterfaceForm` 的讀檔，填 `iATC_MODE_TYPE`）翻進開機序列，
且要在 `ReadTempFile()` 之前。S12 的「存檔後重讀」會讓這個問題**每次存 Temperature 頁都再發生一次**，
所以 Temperature 頁接上 S12 之前要先修。

**複驗**：開機前後對 `IniData\Data\<配方>\Temperature.Data` 取 SHA256；或看 `/api/struct/temperature` 的
`iATCChillerTemp` 與檔案值是否一致。

---

## 十四、`TFTestIF::ReadTestIFFile()` 還在 GATE (F-5)（型態 C，**待辦**，使用者 20260924 指示列入）

**⛔ 已結案（20260926，Steven 團隊核對 HEAD 8fad1522）**：讀檔端不再缺——C 路 `FileRW/TestIF_File_TesterIF.cpp`
（commit `8af13c07`）由產生器直接轉 golden `TFTestIF::ReadTestIFFile`（`cTesterIF.cpp:563`），開機走
`FileRW_TesterIF_BootReadTestIFFile()`（`TestIF_File_TesterIF.cpp:268`）、換配方走 `FileRW_TesterIF_ReadTestIFFile()`
（`:253`），呼叫點 `tools/wb_serve.cpp:3231`（`W906_DoReadLastData`，golden `DoReadLastData` 的 `FTestIF->ReadTestIFFile()`
那一格）；B 路存檔後重讀也改叫它（`:2995`）；`DoIniDataToForm` 在 `:3460`。依賴本節的 A 形狀 `TFTestIF` bridge
（`FileRW/TestIF_File.cpp`、`kBridge_TFTestIF`、ctest `FormBridgeTesterIF`）已於 commit `f89be4ce` 退役；
`Setup.TesterIF.html` 在 `GOLDEN_BRIDGE`（`web/page/ht9045_wire_engine.js:1058`）走 C 路。
移植樹自己那份 `forms/fTesterIF.cpp:800-1223` **仍在 `#if 0 // GATE (F-5)`**，但已經沒有人需要它（真正在跑的讀檔器是
FileRW 那一份）；要不要退役這道閘是另一件事，不擋任何頁面。原本「解閘後從 `tools/formbridge/TFTestIF.py` 拿掉
`sourceGap`」的修法因此作廢（那支設定已移到 `tools/formbridge/_retired/`）。下文原始描述保留供對照，**不代表現況**。

Steven 20260924 實測。**影響**：`TestIF_File` 的測試介面欄位（`iTestType`、`iMaxTime`、`dStartDelayTime`、RS-232／GPIB／TCP-IP、
Initial Delay 一整組…）在 wb_serve 裡**全是初值**：全樹只有它讀 Tester.Data 的 `[Mode]`／`[Time]`／`[GP-IB]`／`[RS-232C]`／`[Tester TCPIP]`
等區段（`grep '"Tester Type"'` 移植樹只命中產生檔）。所以 S12 第二型的 TesterIF bridge（`FileRW/TestIF_File.cpp`，20260924 深夜由 `WriteFile/` 改名）雖然做好了，
`/api/form/Setup.TesterIF.html` 帶 `sourceGap`：畫面不覆蓋、`form.save` 回 409。

本體已翻（`forms/fTesterIF.cpp:807-1223`，416 行），被閘住是因為依賴：
- **會寫配方**：`:818` `iTestType > rgInterfaceType->Items->Count-1` 時寫回 `Tester Type=1`（⚠ 移植樹 `Items` 若是空的，`Count-1=-1`，**每次讀都會改寫**）、
  `[DIO] TypeName`、`[RS-232C] BaudRate`、`ATKRecipeInfo->SaveFile()`。
- **動測試機通訊**：`fTesterTCP` 的 `TimerTCPIPConnect`／`TimerProcessTCPData` 開關、`ClientSocket_TCPIP->Close`、`fMain->SendMSG_CMD`。
- 碰別的表單：`fLotInfo->labTCPIPSimulate`／`SettsChipAdvVisible`、`fSCKART`、`fShowBinSelect->ShowInitialString`、還被閘住的 `CheckRs232StandardIni`（F-6）。

依分工表，讀檔（#1）是 **JerryYang** 的範圍。解閘後：從 `tools/formbridge/TFTestIF.py`（20260924 深夜起，設定拆成一表單一檔，見 `generators.md`）拿掉 `sourceGap`、重跑、
在開機鏈與 `ReloadRecipeDocAfterSave("tester")` 加上 `FTestIF->ReadTestIFFile()`，TesterIF 頁就會自動改用 C++ 值並開放 `form.save`。

**複驗**：`GET /api/form/Setup.TesterIF.html` 的 `sourceGap`／`saveable`；或看 `widgets.rgInterfaceType.itemIndex` 是否等於 Tester.Data 的 `[Mode] Tester Type`。

---

## 十五、`slEventLog` 全樹是 NULL——golden `TfMain` 建構子那段沒翻（型態 A，20260926 新增）

**影響**：任何要讀 `slEventLog->Path`／`FileName` 的函式，移植樹一呼叫就解參考 NULL 當掉；
目前用「呼叫前判 NULL、印一行跳過」擋著，所以看起來「沒事」，但底下那條事件 csv 落檔路徑
其實整條沒在跑。

golden `TfMain` 建構子 `main.cpp:1550-1560` new 出一個 `TMyStringList *slEventLog`（事件記錄
寫檔器）。移植樹全樹 grep `slEventLog *=` 只有測試碼賦值過，本體從未建構。目前已知的守衛點：
`FileRW/MainBoot.cpp` 的 `W906_FRWBoot_JamRawDataRecord()`（`4dee7107`）在呼叫
`fObserver->StatisticalJamCount()`（`cObserver.cpp:3320`，第一句就讀 `slEventLog->Path`）前先判
`slEventLog!=NULL`，NULL 就跳過並印一行。golden 全樹另外還有 46 處用到它，多數也是
`slEventLog!=NULL` 才進去的判斷式，所以現象是「安靜地什麼都不做」而不是當掉——這正是它一直
沒被人發現的原因。

**修法**：翻譯 `TfMain` 建構子 `:1550-1560` 那段（`slEventLog = new TMyStringList(...)`，含它的
建構參數與初值），本體翻好之後，所有 `!=NULL` 守衛會自動成立，行為回到 golden，不用再動任何
呼叫點。**要不要現在做，待 Steven 決定**（見 ChangeLog `CHANGES_20260926_Steven.md` §11.9／§12.4）。
⛔ **20260926 更新**：已決定——S72（`slEventLog` 補建）併入 Steven 另一個 session（Steven02，STEVEN-NB3）的
S-04 cMyDB CSV 版移植 P1，延到 GPIB／Tester 通訊之後（ChangeLog §11.17、§12.5）；不再是「待 Steven 決定」。
⛔ **20260927 複驗（St01，HEAD 227b79db）**：`slEventLog` 已經建了——Steven02 的 cMyDB P1（`80bcd1fb`）新檔 `LogObjects.cpp` 的
`W906_CreateLogObjects()` 照 golden `TfMain` 建構子 new（`LogObjects.cpp:67`／`:73`，依 `IniConfig.bSPILFunction` 二選一），wb_serve 開機
`tools/wb_serve.cpp:4166` 呼叫（`5368493b`）；兩顆經 merge `29554f87` 進本分支。下面「複驗」與檔尾複驗指令十五「建構點應為零」
已過期（現在應命中這兩行）。`FileRW/MainBoot.cpp` 那道 NULL 守衛現在還擋不擋得到、`StatisticalJamCount` 那條鏈會不會真的跑，
**待確認**（沒有 build、沒有執行）。細節歸 skill `ht9045-mydb`（Steven02）。

**複驗**：`grep -rn "slEventLog" --include=*.cpp --include=*.h .`，數一下 `!=NULL`／`==NULL` 判斷
的呼叫點數量（目前應該只有 `MainBoot.cpp` 這一處是移植樹自己加的守衛，其餘引用都還是原樣的
golden 呼叫，沒有加判斷——這些呼叫點目前是不是也都被更上層的 GATE 擋住，需要另外查證，本節
只確認了 `StatisticalJamCount` 這一條鏈）。

---

## 十六、開機少 golden `TfMain::FormShow` 的 `SetRunStartMode`——Tester Off-Line 機台開機顯示 Re-Test（型態 B，20260926 新增）

**影響**：`Tester Connection=0`（Off-Line）的配方，golden 開機後 `iTestRunMode` 是 `OffT`
（畫面顯示「Off-Line」、分 bin 用 Off-Line 表），移植樹開機後卻是 `RT`（顯示「Re-Test」、分 bin
用 RT 表），要等操作員換過一次配方，`cbSetupFileNameChange` 那條鏈才會把它變回 `OffT`。

**這不是「Off-Line 跟 golden 不一致」的缺陷**——`SetTestRunMode()` 本體已經翻好（Jimmy
`0dcc9c2c`，`RunStartMode.cpp:938`）且與 golden 逐句相同（NB2 R62 覆核過），**換配方**呼叫它的
那條路徑也已經接上（`a97739ae`，golden `main.cpp:25771`）。缺的只是**開機**那一次：golden
`TfMain::FormShow` `main.cpp:10050-10086`（註解「必須在 `SetStartModeData()` 之後」）在 ASM 條件
不成立時會呼叫 `SetRunStartMode(eRunStartMode(LastSet.iRunStartMode))`；移植樹的開機鏈
`tools/wb_serve.cpp` 的 `W906_DoReadLastData(bBoot)` 沒有這一段，所以 `iTestRunMode` 停在
建構時的初值（`RT`），一直到換配方才被 `SetRunStartMode` 修正。

**修法**：在 `W906_DoReadLastData(bBoot)` 的 `bBoot` 分支裡，比照 `main.cpp:10050-10086` 補上
`SetRunStartMode` 呼叫（連同它前面判斷 ASM 是否啟用的那段條件）。這是**開機序列的缺口**，
交給 Jimmy（他正在動 `TfMain` 開機序列，見 ChangeLog `CHANGES_20260926_Steven.md` §11.1／§12.5）。

**複驗**：開機後立刻讀 `/api/struct` 的 `lastset.runStartMode`／`lastset.tester`，`Tester
Connection=0` 的配方應該直接是 `OffT`，不必先換一次配方。

---

## 十七、`INDEX_SUCKER_TYPE` 只解了一半——開機讀了，但存檔仍即時寫記憶體（型態 C，20260926 新增）

**背景**：golden `TfMain` 建構子讀 `INDEX_SUCKER_TYPE` 那一段（`:2058-2070`）原本整段在
`#if 0 // GATE(W906-CTORKEYS-SUCKER)` 裡，理由是「泵 `ProcessIndexSuckDestroy1/2` 與
`bNeedCheck` 讀取端還是替身，先載入會讓 `WAR1604` 被 `bIndexCheck1` 永久關掉」。

**20260926 重查現況**：解閘條件只到一半——`bNeedCheck` 讀取端（`033358a2`，W2 批 D）已經接回
真的 `TMyKitSuck` 成員，但**泵 `ProcessIndexSuckDestroy1/2` 仍是替身**（`aTester_Front.cpp:207`、
`atester_32Site.cpp:205`、`AutoClean/AutoClean.cpp:288` 都恆回 `true`；真本體在
`forms/fIoSetView.h:658-659`，標 GATE S-08／S-09）。所以這個閘目前**維持關著**（Jimmy
`212c8e1d` 的判斷保留），開機不讀這個鍵。

**但有一個不對稱**：**網頁 `HandlerSys` 存檔會即時寫**——`FileRW/HSys.gen.inc:3584` 是全樹唯一
「活的」`INDEX_SUCKER_TYPE` 寫入點，存檔當下直接把畫面選的值寫進記憶體，**不受這道 GATE 管**。
也就是說：開機時這個變數是 golden 建構子的 DFM 預設值（因為 GATE 還關著），但操作員只要開一次
`HandlerSys` 頁存檔，記憶體裡的值就會被網頁的選項蓋掉——**不是空的洞，是「開機路徑閘著、
存檔路徑沒閘」這種不一致**。

**分類**：SAFETY 相關（`WAR1604` 那條互鎖）。**交給 Jimmy**：等泵的兩個替身翻完，這道 GATE 才能
真的解開；解開前要不要也讓 `HSys.gen.inc:3584` 那個寫入點暫停，待 Jimmy 決定（NB2 R50 Q8b-J2
已經記錄這個不對稱，本節只是把它併進 porting-gaps 統一追蹤）。

**複驗**：`grep -n "ProcessIndexSuckDestroy" aTester_Front.cpp atester_32Site.cpp AutoClean/AutoClean.cpp`
——三處目前都應該是 `return true;` 之類的替身；解閘那天這條複驗要變成有真本體。

---

## 十八、`BinCount.txt` 三個寫點在 SEAM S2 空替身裡——移植樹目前完全不寫這個檔（型態 C，20260926 新增，含 0925 稽核更正）

**0925 稽核前提已作廢**：0925 的盤點認為「移植樹會寫 `BinCount.txt`、且每次執行都用 0 蓋掉」，
20260926 查證後這個前提不成立——**移植樹目前根本不寫這個檔**。`csystem.cpp` 的三個寫點
（`:5170`／`:5227`／`:8161`）都呼叫 `ReadWriteBinCountMode`，但這個名字在 `csystem.cpp:4179-4180`
被 `#define ReadWriteBinCountMode W7C2_ReadWriteBinCountMode` 換成 SEAM S2 的空替身（檔內
`:28195-28203` 的註解自己寫明是空的），一路到 `#undef`（`:28296`）才恢復；三個寫點全部落在這
段巨集替換範圍內，所以呼叫了也不會真的寫檔。golden 還有第四個寫點在程式結束時
（`main.cpp:29127`），移植樹目前也沒有。

**讀的那一半已經補上**（`4dee7107`）：`FileRW/MainBoot.cpp` 的 `W906_FRWBoot_BinCountRead()` 在
開機呼叫 `ReadWriteBinCountMode(true)`（這次是真本體，golden `main.cpp:9609`，不在 SEAM S2 的
巨集範圍內），把 `BinCount.txt` 讀回 `iByBinTotal[256]`／`iSVByBinCount[TEST_MAX_BIN]`。先落地
讀檔的順序是對的：等 SEAM S2 那三個寫點退役、變成真本體時，開機已經先讀過檔，寫回去的才是
累計值，不會被寫點自己的空替身吃掉、也不會第一次執行就被覆蓋成 0。

**修法**：SEAM S2 退役（把 `csystem.cpp` 那三處的巨集包裝拿掉，換回真的
`ReadWriteBinCountMode` 呼叫）交給 Jimmy；golden 結束時的第四個寫點移植樹也還沒有，一併排入。

⛔ **20260927 更新（St01，HEAD 227b79db）**：「移植樹目前完全不寫這個檔」已不完全成立——(1) 主畫面 Exit 鈕 step 1 照 golden
`sbCloseProgramClick :29129`（主 repo V912；上文的 `:29127` 是 HT9045_ref 行號）呼叫**真本體** `ReadWriteBinCountMode(false)`
（`FileRW/MainClose.cpp:1270`，不在 `csystem.cpp` SEAM S2 巨集範圍內，`6905f8eb`）＝上文說的「第四個寫點」接上了；`FormClose` 本身
沒有這一行，所以 `--seconds` 到期結束不寫。(2) Counter Clear 的 `ClearCount(ctTesterCategory)` 照 golden 刪 `BinCount.txt`
（`cCounterClear.cpp:152-167` GATE CC1 解開，`6905f8eb`；路徑一樣吃 `W906_BINCOUNT_PATH`）。`csystem.cpp` 那三個寫點仍在
SEAM S2 空替身裡（HEAD 227b79db：`#define` `:4186`、寫點 `:5176`／`:5233`／`:8167`、`#undef` `:28451`；檔案變長了，行號比上文大）。

**複驗**：`grep -n "define ReadWriteBinCountMode\|undef ReadWriteBinCountMode" csystem.cpp` 確認
巨集範圍還在；`grep -n "W906_FRWBoot_BinCountRead" tools/wb_serve.cpp` 確認開機讀已接。

---

## 十九、筆電 `IO_CARD_TYPE=1` 時 IO 表沒綁——第 42 條 IO 解除這台機器測不到（環境限制，不是缺陷，20260926 新增）

放在這裡是為了**提醒下一個接手的人不要在這台筆電上追這個「測不出來」的假象**，不是要排波次。

這台筆電的 `Gerneral.ini` 是 `IO_CARD_TYPE=1`，開機 log 印出 `[BOOT] IO_Table rows=0、Sen
named/enabled=0/40`——IO 表沒綁、`Sen[].Name` 全空。`sim.di.set` 因此找不到任何感測器名字可以
模擬觸發。`RULINGS_20260925.md` 第 42 條「訊息框等待期間由 C++ 讀 IO、通知 html 端解除」的程式
已經在 `8af13c07` 裡，但**在這台筆電上沒有 IO 表可以測**，不是程式沒接、也不是解除邏輯有問題。

**修法**：不是程式問題，要在有正確 IO 表（`IO_CARD_TYPE` 對應真實機種／或至少表有綁定）的機器
或機台端測。若之後要在筆電上驗這條邏輯，可以考慮另外造一份測試用的 IO 表（不動量產設定），
但這是測試環境的工作，不是移植缺口。

**複驗**：開機 log 的 `[BOOT] IO_Table rows=` 那一行，`rows>0` 且 `Sen named/enabled` 不是
`0/40` 時，這台機器才有條件測第 42 條。

---

## 二十、HT9050 上 golden IO 物件打不到 1203 卡——`uiDevhand` 只在 `INSTALL_ETHETCAT` 成立時填（型態待確認，20260926 新增）

出處：S48／S49 唯讀調查報告第一節
（`D:\docs\ops\weekly\2026\09\20260926\RD5軟體20260926_093427_S48_S49_唯讀調查報告_交給Jimmy.md`）。
本節只是把它併進 porting-gaps 統一追蹤，細節與複驗步驟仍以那份報告為準。

golden 的 IO 物件（`uiDevhand`）只在 `INSTALL_ETHETCAT` 這個編譯期／組態旗標成立時才會填值；
機台端 `SHUTTLE_SENSOR_TYPE=0`、`VacuUnitType=0` 的組態下，這條路徑沒有被觸發，所以 HT9050 上
S48（硬體指令鈕）要接的那幾個動作，目前連 golden 物件本身都摸不到 1203 卡，不是移植樹翻譯
的問題。**待確認**：`INSTALL_ETHETCAT` 是不是就是唯一入口、還有沒有其他組態能觸發同一段填值——
本次沒有重新查證，直接引用調查報告的結論。**交給 Jimmy**（連同 S48／S49 一起看）。

---

## 二十一、網頁存檔不檢查運轉狀態——golden 設定鈕只在停機時按得到（偏離 golden 的缺閘，**SAFETY 相關，待 Jimmy 決定**，20260926 新增）

**golden**：設定工具列 `palSetup` 只能從 `TfMain::sbSettingClick` 打開，它第一件事就是 `if(SystemStart) return;`，
再過 `fSecurity->Insufficient(0)`（主 repo V912 `main.cpp:29030-29036`）。工具列上的設定鈕 `MyToolBtn[0..9]`
（＝`sbTrayForm`、`sbPlateForm`、`sbTrayAssign`、`sbTempOffset`、`sbContact`、`sbTester`、`sbBin`、`sbSetup`、
`sbLdUld`、`sbExitSetup`，`main.cpp:1756-1771`）因此**只在停機、進了設定模式才按得到**——golden 的設定表單在
運轉中打不開，也就存不了檔。

**移植樹**：`WS editlist.save`（`tools/wb_serve.cpp:5292-5339`）、通用層 `filerw::PageSave`（`FileRW/_EditPage.cpp:76-201`）、
手寫 `FileRW/IniConfig.cpp` 都**不檢查 `SystemStart`**（20260926 grep 三處 0 命中）；只有 golden 存檔流程自己有檢查的會
照 golden 擋（`FileRW/*.gen.inc` 裡只有 `Offset_File.gen.inc`、`TTLCfg.gen.inc` 各 1 處提到 `SystemStart`）。
等於**運轉中也能從網頁存配方／設定**。影響會被放大：S88 之後溫度頁存檔會跑 `MainTempOffsetTail()` →
`DoStructUnitConvert()`／`SetWorkParameter()`（`generators.md` 十九）；SetUp／TesterIF 的 golden 存檔流程也會
`SetWorkParameter()`（重算 site map）——這些在 golden 的運轉中根本做不到。

**分類**：不是翻譯錯，是「golden 靠 UI 不給按」這道閘在網頁上消失了。SAFETY 相關（運轉中改參數），
**交 Jimmy 決定放在哪**（全部 `editlist.save`／`form.save` 一起擋，還是逐表單照 golden 的入口擋）——ChangeLog
`CHANGES_20260926_Steven.md` §11.36c「通用風險」、§12.5 最後一項；已請 github-02 記進 `docs/handoff/FROM_STEVEN.md`
（同 §11.36c）。同類先例：`RULINGS_20260926.md` 第 8 條（S48 網頁硬體鈕「只在沒有運轉中時有效」）。

**修法方向（待 Jimmy 定案，不要自己加）**：在 `editlist.save`／`form.save` 進入點（或 `PageSave` 開頭）判 `SystemStart`，
拒存並回 golden 同義訊息；`editlist.get`（開頁）要不要也擋另議——golden 運轉中連開都開不了，但網頁開頁只是讀。

**複驗**：`grep -n "SystemStart" FileRW/_EditPage.cpp FileRW/IniConfig.cpp`（加閘之前應為 0）；
`sed -n '5292,5339p' tools/wb_serve.cpp | grep -c SystemStart`（同上）。

---

## 二十二、St01 這一輪照建議先做的 `[W906]` 偏離與時序差異（20260926～27，**Steven 可推翻**，20260927 新增）

**這一節不是「值是假的」缺口**，是 St01（Steven01）在 S91～S121 這一輪交件時，遇到 golden 行為在 wb_serve／網頁上會出事
（寫壞真檔、不安全）或做不到（沒有對應的觸發點、別人的段落還沒接）時，**照建議先做、偏離 golden** 的地方。題目、選項、
St01 建議與目前狀態的權威版本在 skill `ht9050-construction` 的 `references/todo.md`「★ 待 Steven 決定」→「St01 已照建議先做、
Steven 可推翻」（R 組，本節只**引用**，不另下結論）；這裡把其中**偏離 golden** 的挑出來、附程式位置，方便 Jimmy／Steven02 對照。
程式裡這類地方多半標 `[W906]`（例 `FileRW/MainClose.cpp`）。行號是 HEAD 227b79db；golden 行號是主 repo V912。

| ★ | 偏離了什麼（golden → 移植樹） | 方向 | 程式位置 | commit |
|---|---|---|---|---|
| R49 | golden 關程式（機台內有 IC、`bShowLotInfo`）把 Lot Info 頁的裝置名／溫度寫回 `config.ini [Server]` Product Name／Product Temp（`main.cpp:12051-12057` `SaveRmsInfo`）；開機讀回那一步當時還在 GATE WC-1 → `bd40ffcb` 先做「兩個值都空就不寫」。**⛔ 20260927 已恢復照 golden**：`61c96910` 把 WC-1 翻好接上（`FileRW/MainBoot.cpp` `W906_FRWBoot_ResetLotInfo`，`tools/wb_serve.cpp:4162`），守衛變數留著、值改成 `false`（改回 `true` 就恢復舊做法） | 不寫壞真檔 → 已照 golden | `FileRW/MainClose.cpp:811`（`s_bRmsBlankGuard`） | `bd40ffcb`／`61c96910` |
| R50 | golden 沒讀到機型（`bHandlerModel=false`）時開機就 Terminate、走不到 `SetRunStartMode` → wb_serve 照常跑，所以**不裝** `SaveRunMode` 本體（裝了的話一換模式就把 `RunMode.txt` 寫成 `RunMode=0`） | 不寫壞真檔 | `FileRW/MainClose.cpp:1035-1046`（`W906_FRW_InstallSaveRunMode`） | `bd40ffcb` |
| R41 | 同一狀態（`bHandlerModel=false`）下 Exit／`--seconds` 結束：golden 沒有這條路 → **照樣停機、只是不存檔**（`W906_ProdCloseSave` 在那時不寫 lastdata 等） | 安全 | `FileRW/MainClose.cpp:1073-1074`、檔頭 `:56-58` | `6905f8eb`／`a684f171` |
| R43 | golden `Timer1` 要 `SystemInitialOK==true` 才累計 Observer 時間（`main.cpp:3263`）；wb_serve 沒有人設它（golden `FormShow :10092` 才設）→ **只看 `InitialOK`** | 不然功能永遠不動 | `FileRW/MainRecord.cpp:60-66`、`:357-360` | `0b38b6b5` |
| R44 | golden 系統計時起點在 `SYSTEM_MODULAR` 建構子（`database.cpp:47`，移植樹整段 `#if 0`）→ 在 wb_serve 開機讀完機台設定那一行補（`W906_MainRecordBootLatch`，`tools/wb_serve.cpp:3864`）；沒補過就在第一拍補 | 時序近似 | `FileRW/MainRecord.cpp:345-350`、`:362-367` | `0b38b6b5` |
| R46 | golden 告警框開著時框自己的 Timer 會呼叫主畫面 Timer1，Jam Time 照樣累計 → 移植樹框開著時主迴圈不跑（`W906_ModalWaitTick` 是 Jimmy 登記的段），**目前不累計**；片段已交 Jimmy | 時序差異（待 Jimmy） | `0b38b6b5` 本文 | `0b38b6b5` |
| R25 | golden 每次關程式存一次 `SaveJamRateByDay(false)` → **一個行程只存一次**（Exit 存完但程式沒結束、之後 `--seconds` 再存的話，記憶體已被第一次清掉，會把檔寫成 0 筆） | 不寫壞真檔 | `FileRW/MainClose.cpp:146-152`、`:1098-1107`（`s_bJamRateSavedOnClose`） | `6905f8eb` |
| R40 | golden 沒有 `--seconds`；wb_serve `--seconds` 到期結束時**也跑 golden `FormClose` 的停機順序**（停馬達、鎖煞車、關加熱器繼電器／風扇／蜂鳴器，也寫 machinerecord 等） | 安全 | `tools/wb_serve.cpp:5964`、`FileRW/MainClose.cpp:1076` | `a684f171` |
| R29 | golden `Timer10` 在告警框開著時照跑 → Counter Clear 的收尾（`W906_CounterRefreshTick`，在 PumpTick 裡）要等框關掉才做 | 時序差異 | `WebBridgeTags.cpp:636`（呼叫）／`:3297`（本體） | `6905f8eb` |
| R32 | Test Information 的 Index 時間平均：golden 測完才更新（`RecordTimeInfo`）→ 暫用 `RunInfo.IndexTime` 在 Index 完成時算；Jimmy 的 `RecordTimeInfo`（W2-3）翻好就自動讓位——todo E-009：已改用 `d3c93dea` | 時序差異（照設計已讓位；沒有 build 驗過） | `cObserver.cpp` St01 段（`b40b140f` 本文） | `b40b140f` |
| R8 | DIO 設定檔刪除（golden `spbDeleteClick` `DIOInterFaceCFG.cpp:249`：表單開著就能刪、檔名不限）→ **多兩道守衛**：每次重查 `Insufficient(31)`、只准刪 DioCfg 清單上的 `*.ini` | 加嚴 | `FileRW/TTLCfg.cpp:348` 起 | `d606b1d8` |
| R9 | LotData 清單的 Clear List 裡 golden 那一行 `SendCCDCommand`（`uLotInfo.cpp:10191`）→ **閘住**、記 `ELTodo`（相機通訊介面歸屬未定；golden 那一行在 `Msg2` 為空時只寫 CCD 通訊 log，`WebLotInfo.cpp:73`／`:222`） | 功能少 | `WebLotInfo.cpp:222` | `26d0b3f8` |
| R11 | Clear List golden 一按就清 → 網頁**兩段式確認**（移植樹清除前不寫 golden 會寫的 .xls） | 防誤觸 | `web/page/ht9045_lotinfo_wire.js:257-264` | `26d0b3f8` |
| R23 | Lot End golden（非 OEE 機台）一按就結批 → 網頁**兩段式確認** | 防誤觸 | `WebLotInfo.cpp:516`（`lotEnd.state`／`lotEnd`） | `5d58c98d` |
| R53 | golden 沒有連點保護（VCL 表單單一操作者）→ 伺服器 `WebCmdGuard` 擋同一指令 400 ms 內重送；`observer.get` 的四個 Yield act 也擋（Steven S107「全部按鈕要防連點」） | 防連點 | `WebCmdGuard.cpp:107`；總則見 skill `ht9045-html-json` `web-bridge-json-contract.md` §1.3 | `2ae40ffe`／`8314e3bd` |
| R55 | golden Y 軸上下限框是 `OnClick` 才跳鍵盤 → 網頁用「拿到焦點」，Tab 經過也會送一次 | UI 差異 | `web/page/ht9045_observer_wire.js:501` | `8314e3bd` |

**沒有列進來的**（看起來像錯，但這一輪是**照翻 golden**，不是偏離）：R4 Drop 寫成 "1"、R33 HP 存完重讀 Tray 表、R34 存一次再讀第 16 欄會掉、
R45 `/60>25`、R54 Clear 不重畫。R35（Tray／HP 表空欄位不加引號）是照 BCB6 `CommaText`、偏離的是移植樹的 `vclcompat`
（只在 `FileRW/CfgTrayPlate.cpp:83` 的 `W906_Bcb6CommaText` 照 BCB6 寫，共用元件沒改）。題號 R1～R55 的完整清單以 todo.md 為準；
Steven 推翻或條件改變（例 R49 的 WC-1）之後，本表要跟著改。

**複驗**：見本檔最後「複驗指令」二十二。

---

## 排序建議

| 順位 | 項目 | 型態 | 成本 | 解鎖什麼 |
|---|---|---|---|---|
| 1 | 二、heater 執行緒接進開機序列 | B | 小 | S7 的 `pv`／`comm` 71×2 欄立刻變活 |
| 2 | 五、`GetLevelSet()` 的呼叫點 | B | 小 | `levelSet` 的 256 個權限值 |
| **0** | **十三、`ATC.ini` 沒被讀（20260924 新增）** ⛔ 已結案（commit `322d68a3`，20260925 08:49；`forms/fATCHandlerSide.cpp:625`／`tools/wb_serve.cpp:3123`） | A | 小 | 停止開機改寫 `Chiller Temp`；S12 Temperature 頁的前置 |
| **0b** | **十四、`ReadTestIFFile` GATE (F-5)（20260924 新增，JerryYang）** ⛔ 已結案（20260926：C 路 `FileRW/TestIF_File_TesterIF.cpp` 轉 golden 讀檔器，`8af13c07`，開機／換配方 `tools/wb_serve.cpp:3231`；A 形狀 bridge `f89be4ce` 退役） | C | 中 | TesterIF 頁的 C++ 值與 `form.save`；`TestIF_File` 測試介面欄位全部變真 |
| ~~3~~ | ~~四、`TfContact::ReadFile()`~~ ✅ **20260924 結案**（`ca4e903`，見第四節） | A | 中 | `deviceForm.file` 66 欄 |
| 4 | 六、`cMyDB` GA1-B4 閘 ＋ `common.cpp:62` 替身 | C+B | 中 | 留痕真的落地；⚠ 同時要重審 `log.event` 豁免 |
| 5 | 一、`main.cpp` 的 `Index16Heater` 等 | A | **大**（~1,600 行） | S7 的 `inst` 71 欄 |
| 6 | 七、`fTemperFrom` 單例 | A | 中 | S7 的 `ready`／`state`／`sv`（先解決第八、九條的裁決） |
| — | 三、GATE (T1) 告警升級 | C | 小 | **SAFETY，要人明示同意**，不排進一般波次 |
| ~~7~~ | ~~十、`iTo3Unload[]` 的填值~~ ⛔ **撤銷** —— 20260817 `cmydef.cpp:4281/4431` 就填好了，這是幽靈待辦 | — | — | — |
| 8 | 十一、`cSortCT.cpp` ＋ `csystem.cpp:2820-2821` 兩個巨集 | A+C | 中 | `SendCT[2]`／`iUnloadCount` 匯總；WAR07324 與 piggy-back 觸發 |
| — | 十二、`MyForceDirectories` 的位置 | — | — | **不是待辦**，是「不要順手改」的備忘 |
| **9** | **十五、`slEventLog` 全樹 NULL（20260926 新增）** | A | 小 | 事件 csv 落檔；`StatisticalJamCount` 等 46 處守衛自動解除；~~待 Steven 決定要不要現在做~~ ⛔ 已決定：併入 Steven02 的 S-04 cMyDB P1、延後（ChangeLog §11.17）；⛔ 20260927：已建（`80bcd1fb`，見該節） |
| **10** | **十六、開機少 `SetRunStartMode`（20260926 新增，Jimmy）** | B | 小 | Tester Off-Line 機台開機直接顯示正確模式，不必先換一次配方 |
| — | 十七、`INDEX_SUCKER_TYPE` 半閘（20260926 新增，Jimmy） | C | — | **SAFETY，等泵替身翻完才能解**；先處理 `HSys.gen.inc:3584` 那個不對稱的存檔寫入點 |
| 11 | 十八、`BinCount.txt` SEAM S2（20260926 新增，Jimmy） | C | 中 | 三個寫點＋golden 結束時的第四個寫點都要退役空替身；⛔ 20260927：第四個寫點（Exit 鈕 `:29129`）已由 `FileRW/MainClose.cpp` 接上（`6905f8eb`），剩三個 |
| — | 十九、筆電 IO_CARD_TYPE=1 測不到（20260926 新增） | — | — | **不是待辦**，是環境限制備忘（換有 IO 表的機器測） |
| — | 二十、HT9050 1203 IO 物件打不到（20260926 新增，Jimmy） | 待確認 | — | 見 S48／S49 調查報告，本表只掛引用 |
| — | 二十一、網頁存檔不檢查運轉狀態（20260926 新增，Jimmy） | 缺閘 | 小 | **SAFETY 相關，要 Jimmy 定案**：運轉中不能從網頁存設定，與 golden 一致 |
| — | 二十二、St01 這一輪照建議先做的 `[W906]` 偏離（20260927 新增） | 偏離 | — | **不是待辦**，是給 Steven 推翻、Jimmy／Steven02 對照的清單（16 條，R49 已恢復照 golden） |

---

## 複驗指令

⚠⚠ **每一塊都自己 `cd`，不要接著上一塊跑。**
第一塊在 **golden** 跑，其餘在 **移植樹** 跑 —— 混在一起的話，
`TfContact::ReadFile`「應為零」那條會在 golden 找到本體，
`HeaterThread`「應為零」會得到 17，`cMyDB.cpp` 會說檔案不存在。
（20260923 第一版的指令塊就是這樣壞的：`cd` 進 golden 之後沒切回來，
9 條裡 7 條輸出與文件宣稱相反。）

```bash
# ── 一：bUT150Install 的填值來源分布（在 GOLDEN V912 跑）──────────────
cd D:/HT9045/HT9011UC_Code_V3.33.912.0_20260908_Jimmy

# raw 計數（含註解掉的）：main.cpp 1091 / csystem.cpp 11
grep -c "bUT150Install\[[^]]*\][[:space:]]*=[^=]" main.cpp csystem.cpp

# 活賦值的逐函式分布：931 / 71 / 36 / 28 / 18 / 1(建構子清零)
awk '/^void .*TfMain::|^void __fastcall TfMain::/{fn=$0} \
     /bUT150Install\[[^]]*\][ \t]*=[^=]/ && $0 !~ /^[ \t]*\/\// {print fn}' main.cpp \
  | sed 's/(.*//' | awk -F'TfMain::' '{print $2}' | sort | uniq -c | sort -rn
# ⚠ 最後一行印出空白名字的那筆 1 是建構子 TfMain::TfMain（main.cpp:2253-2256），
#   它是 `for(i<tcTotalCount) bUT150Install[i]=false;` 的**全清零初始化**，
#   不是「決定哪些有裝」。不要去翻它。

# ── 二～九：以下全部在 移植樹 跑 ──────────────────────────────────────
cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0

# 二：heater 執行緒的啟動點（去掉註解後應為零）
grep -rn "HeaterThread" --include=*.cpp . | grep -v "^./build/" \
  | grep -v "^./uHeaterThread" | grep -v "//"

# 三：GATE (T1)
grep -n "#if 0" cTemperFrom.cpp

# 四：TfContact::ReadFile 的本體（應為零，只有註解）
grep -rn "TfContact::ReadFile" --include=*.cpp . | grep -v "^./build/"

# 五：SEC1 閘與名字數
grep -n "^#if 0" cSecurity.cpp | head -1
grep -o 'TMySecurity("\[[0-9]*\]' cSecurity.cpp | sort -u | wc -l

# 六：三個留痕入口
sed -n '62p' common.cpp
grep -n "^#if 0" cMyDB.cpp | head -4

# 九：bTemperatureReady 的初值
sed -n '238p' cTemperFrom.cpp

# 十：iTo3Unload 應為「只有宣告，零個賦值」
grep -rn "iTo3Unload" --include=*.cpp --include=*.h . | grep -v "^./build/"
grep -rn "iTo3Unload *\[[^]]*\] *=" --include=*.cpp . | grep -v "^./build/"   # 應為零

# 十一：cSortCT.cpp 應該不存在；兩個巨集閘應該在
ls cSortCT.cpp 2>&1                      # 應為 No such file
grep -n "W7C2_FSORTCT_SHOWLOADING\|W7C2_FSORTCT_SHOWSORT" csystem.cpp

# 十二：守衛在 MyForceDirectories 之後（順序本身就是要確認的東西）
sed -n '143,147p' JsonBridge/actions/MainClarnData.cpp

# 十五：slEventLog 建構點應為零（20260926 新增）
grep -rn "slEventLog *=" --include=*.cpp --include=*.h . | grep -v "^./build/" | grep -v "^./tests/"

# 十六：開機鏈應該還沒有 SetRunStartMode（20260926 新增；解掉之後這行才會有命中）
grep -n "SetRunStartMode" tools/wb_serve.cpp

# 十七：INDEX_SUCKER_TYPE 的 GATE 與存檔即時寫點都應該還在（20260926 新增）
grep -n "GATE(W906-CTORKEYS-SUCKER)" FileRW/HSys.cpp
grep -n "INDEX_SUCKER_TYPE" FileRW/HSys.gen.inc | sed -n '1p'

# 十八：BinCount 的 SEAM S2 巨集範圍應該還在（20260926 新增）
grep -n "define ReadWriteBinCountMode\|undef ReadWriteBinCountMode" csystem.cpp
grep -n "W906_FRWBoot_BinCountRead" tools/wb_serve.cpp   # 讀的一半應該已經接上

# 十四（已結案）：C 路讀檔器應該接在開機／換配方鏈（20260926 新增）
grep -n "FileRW_TesterIF_BootReadTestIFFile\|FileRW_TesterIF_ReadTestIFFile" tools/wb_serve.cpp   # :2995、:3231
grep -n "GATE (F-5)" forms/fTesterIF.cpp                                                          # 移植樹那份仍閘著（不影響）

# 二十一：網頁存檔的運轉狀態閘（20260926 新增；加閘之前應為 0）
grep -c "SystemStart" FileRW/_EditPage.cpp FileRW/IniConfig.cpp

# 十五（20260927 更新）：slEventLog 現在應有 2 個建構點（LogObjects.cpp:67／:73，St02 80bcd1fb）——上面「應為零」已過期
grep -n "slEventLog=new" LogObjects.cpp

# 十八（20260927 補）：Exit 鈕的 BinCount 寫點（真本體，SEAM S2 範圍外）應該在
grep -n "ReadWriteBinCountMode(false);" FileRW/MainClose.cpp        # :1270

# 二十二：St01 這一輪的 [W906] 偏離應該都還在（Steven 推翻、或前提改變之後才會變）
grep -n "s_bRmsBlankGuard = " FileRW/MainClose.cpp                  # R49：61c96910 起是 false（照 golden）
grep -n "if (bHandlerModel == false)" FileRW/MainClose.cpp          # R50／R41：:1039、:1131、:1198
grep -n "s_bJamRateSavedOnClose" FileRW/MainClose.cpp               # R25
grep -n "if(InitialOK==false)" FileRW/MainRecord.cpp                # R43：只看 InitialOK（:359、:383）
grep -n '"observer.get"' WebCmdGuard.cpp                            # R53：op 級規則（:107）
```
