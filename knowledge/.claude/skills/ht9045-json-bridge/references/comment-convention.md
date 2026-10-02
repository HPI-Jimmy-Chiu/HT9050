# 移植樹註解慣例（給 JsonBridge 實作者）

> 量測日 20260923，量測對象 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0`（排除 `build/`）。
> 本檔**不發明範例**：每一條規則都附移植樹裡的真實檔名與行號，照著開檔就看得到。
> 讀者：接下來寫 `JsonBridge/` 的人（Steven、JerryYang，或之後接手的任何人）。

---

## 〇、先看規模

| 項目 | 數字 | 怎麼量 |
|---|---|---|
| `AI(W906-` 出現次數 | **5,768** | `grep -rc --include=*.cpp --include=*.h "AI(W906-"` |
| 帶到 `AI(W906-XXX)` 的相異標籤 | **523** | 同上再 `sort -u` |
| 含有這種註解的檔 | **839**（排除 `build/`） | `grep -rl` |
| 標籤後面**緊接 8 位日期**的 | **5,614（97%）** | `grep -oh "AI(W906-[A-Za-z0-9_.-]*)[ ]*[0-9]\{8\}"` |

⇒ 這不是零星習慣，是全樹通行的格式。新檔照做，不要另立一套。

---

## 一、格式

```
// AI(W906-<標籤>) YYYYMMDD: <說明>
```

三個部分都有作用，缺一個就少一個功能：

| 部分 | 作用 | 反例 |
|---|---|---|
| `AI(` | 標示「這段是 AI 協作留下的」，與人手寫的 `//Steven 20260916 (R1):`、golden 自己的 `//Ifor 20191121` 分得開 | 省略 → 分不出是誰的判斷 |
| `W906-<標籤>` | **可 grep 的變更集識別碼**：同一次工作在全樹散落的每一處，用同一個標籤 | 用模組名 → grep 出來等於 `ls`，沒有新資訊 |
| `YYYYMMDD` | 陳述的**量測日**。移植樹的規則是「陳述的是陳述人當時量到的事實，不是永久的事實」（`cSocket.h:83`），沒有日期就無從判斷還能不能信 | 省略 → 過期的前提被當成現況用 |

### 1.1 真實樣本

```
WebBridgeTags.h:4          //  AI(W906-WebBridge) 20260806.  NOT in golden.
tools/wb_serve.cpp:71      #include "wb_dialog_mailbox.h"  // AI(W906-Q30-8) 20260922: 警報對話框的檔案信箱
tools/wb_serve.cpp:288     // AI(W906-Q30-8) 20260922: ⚠⚠ **kCode 守門，放在最前面。**
tools/wb_serve.cpp:304     // AI(W906-Q30-KZERO) 20260923: kcode==0 是**通知**，不是問題 —— 不進等待迴圈。
tools/wb_serve.cpp:687     //  AI(W906-FW-SYSFILE) 20260915: the five machine-config files the browser needs.
EtherCAT/Pci1203Control.cpp:1336   //AI(W906-1203ALM-4) 20260912: ⚠ THIS WAS `uiDevhand` AND THAT WAS A BUG.
cSocket.h:75               //  AI(W906-FW3-PICTL) 20260829: ⚠ **上一行那則 absence-claim 已過期**。
aHotPlateSubstrate.cpp:423 // ⚠ AI(W906-P1b) 20260920 更正：上面那段原本寫「golden 的 ctor 還設了…
```

⚠ `//` 後面空幾格、`⚠` 放在 `AI(` 前面或後面，全樹**不統一**（`Pci1203Control.cpp` 一律 `//AI(` 無空格、`wb_serve.cpp` 一律 `// AI(` 一空格）。不要為了對齊去改別人的檔；**同一個檔內保持一致**即可。

---

## 二、標籤怎麼取（`W906-` 後面那一段）

量到的 523 個標籤，**沒有一個是模組名或檔名**。它們一律是「這次工作」的代號。前十大家族：

| 前綴 | 出現次數 | 是什麼 |
|---|---|---|
| `W7` | 1,418 | W7 翻譯波次（`W7-B1b` 401、`W7-B1c` 403、`W7-L2` 164、`W7-L1-Wave3` 97…） |
| `FW` | 880 | FW 表單→web 戰役（`FW-SIG-W18` 53、`FW-BinSelUnlock` 34…） |
| `PT` | 663 | PT 移植波次（`PT-W2` 154、`PT-W3` 135、`PT-csystem-g2` 37…） |
| `FW3` | 581 | FW 第三輪（`FW3-Observer-W2` 118、`FW3-TempSet-WA` 46…） |
| `HOME` / `TEACH` / `LOT` | 137 / 33 / 38 | 主題波次 |
| `1203CTL` / `1203MON` / `1203ALM` | 113 / 85 / 50 | PCIE-1203 三個子工作 |
| `Q30` / `Q34` | — | **問題編號**（`Q30-8`、`Q30-KZERO`、`Q30-KMAP`、`Q34-1`） |
| `GA` / `GA1` / `ST` / `BA` / `BU` | 74 / 49 / 79 / 36 / — | gate 與 bring-up 波次 |

### 2.1 三種觀察得到的取法

1. **波次代號＋序號**（最常見）：`W7-B1c`、`PT-W2`、`FW3-Observer-W2`、`HOME-C2`、`1203ALM-4`、`ST-S3-B4`、`BU-C4`
2. **主題代號**，沒有波次：`WebBridge`、`SysModWire`、`AutoCleanFoundation`、`MyProductionRecord`、`TesterTCPTimer`
3. **問題／裁決編號**：`Q30-8`（NonStop 對話框第 8 問）、`Q34-1`（1203 唯讀監看）、`WEB-W2b` 後面還直接寫了「(裁決 A1)」（`tools/wb_serve.cpp:520`）

### 2.2 續作、修正、整併有固定尾碼

| 尾碼 | 意思 | 真實例 |
|---|---|---|
| `-WaveN` | 同一標籤的第 N 波 | `W906-W7-L1-Wave0`（49）、`W906-W7-L1-Wave3`（97） |
| `-fixN` / `-W3fixB` | 修同一波自己的缺陷 | `W906-W7-L1-W3fixB`（25） |
| `-integrate` | 把分支成果併回主線 | `W906-PT-W3-integrate`（35）、`W906-GA1-B2-integrate`（26） |
| `b` / `c` 尾碼 | 同一件事的第二、三次 | `W906-W7-B1b`（401）／`W906-W7-B1c`（403）、`W906-P1b`、`W906-P6b-C` |

⇒ **判準：標籤是「變更集」，不是「位置」。** 問自己一句話 ——「`grep -rn "AI(W906-<標籤>)"` 出來的東西，是不是剛好等於我這次做的事？」是，標籤就取對了。

---

## 三、什麼情況下才寫（**不是每行都寫**）

`WebBridgeTags.cpp` 有 134 KB、2,194 行，`AI(W906-` 只出現在少數幾十處。慣例是：**不寫會被誤解的地方才寫**。量到的七種情境：

### 3.1 檔案／區塊開頭：說明「這個檔為什麼存在」與「哪些不在範圍內」

`WebBridgeTags.h:1-13`

```
//  WebBridgeTags.h -- the ONE place where the machine and the browser meet.
//
//  AI(W906-WebBridge) 20260806.  NOT in golden.
//
//  WHY IT LIVES HERE AND NOT IN WebBridge/
//  WebBridge/ is deliberately free of vclcompat and of every machine header, so
//  the socket layer can be reasoned about on its own … This file is the
//  deliberate exception … Keeping that mixing confined to one translation unit
//  is the entire point -- if this were inside WebBridge/, that layer's
//  independence would be gone.
```

`cUnitConvert.cpp:1-19` 是另一種寫法：**明列 TRANSLATED 與 SKIPPED 兩張清單**，並指到「為什麼跳過」的位置。

```
//  BCB6 source: HT9011UC_Code_V3.33.906.0_20260618/cUnitConvert.cpp
//  Translation scope (W2 partial extract):
//    TRANSLATED:   iUnitMultiply100  (BCB6 lines 13-20)
//    SKIPPED:      DoTestIFConvert, DoDeviceConvert, … DoStructUnitConvert
//                  -> See cUnitConvert.h for full per-function dependency notes
```

### 3.2 `#include`：只要它是為某一次工作而加的

`tools/wb_serve.cpp:70-96` 幾乎每個 include 都帶標籤與一句用途：

```
#include "WebBridge/CommandQueue.h"   // AI(W906-FW-W1) 20260819: cmd channel e2e (--allow-cmd)
#include "WebAuth.h"                  // AI(W906-FW-W2) 20260819: auth.login verification core
#include "forms/fSetup.h"             // AI(W906-SETUP-READFILE) 20260922: fSetup->Init/ReadFile (HandlerCondition.Data [Configuration])
```

`EtherCAT/Pci1203Monitor.cpp:19` 更直白：`⚠ THIS INCLUDE IS LOAD-BEARING, NOT TIDYING.`（＝不要以為沒用到就刪）

### 3.3 守衛／提前 return：「不寫的話看起來可以刪」

`tools/wb_serve.cpp:288-303`（kCode 守門）與 `:304-320`（`kcode==0` 不進等待迴圈）。後者寫出了**三個看似不相干的症狀共用一個根因**：

```
//   而 wb_serve 是單執行緒：這一支卡住，:2872 的 `PumpTick()` 連帶停擺
//   ⇒ MainProc 不再被呼叫、DoAllProcess 不再跑，而且**其他網頁命令拿不到 ack**
//   …三個症狀、一個根因，而且看起來像三個不相干的 bug
//   （斷點不中／流程不跑／網頁 no ack within 15000ms）。
```

### 3.4 與 golden 不同、或刻意不翻譯的地方

`aHotPlateSubstrate.cpp:428-436`——逐行標 golden 行號，並把「golden 有、本樹沒有」講清楚：

```
    iMotRow      = 1;      // golden :76
    iMotCol      = 1;      // golden :77
    //  golden :80 `iMaxCnt=1;` —— ⚠ **本樹的 TMyKitSuck 沒有 iMaxCnt 這個成員**
    //  （aHotPlateSubstrate.h 的門面只帶了被讀到的欄位）。全樹 0 個讀取點，
    //  所以不是「漏設值」而是「這個欄位還沒被帶進來」。
    //  ⇒ 哪一天有人要讀它，要先在 h 裡補宣告，再把這一行解開。
    iShtRow      = 1;      // golden :83  ⚠ 移植樹原本是 2（無 golden 依據）
```

### 3.5 gate（`#if 0`／stub）：必須寫出「拆掉會怎樣」與「要跟誰一起拆」

`cinitial.cpp:7175`

```
    // BEHAVIOUR DELTA ON A REAL MACHINE: unit conversion of the recipe structs
    //   does not happen.  Must be retired TOGETHER WITH G7 -- opening G7 alone
    //   would feed unconverted values into the SetTechDataToProd_* bodies, which
    //   is WORSE than both being shut.
#if 0 // N3-G8: blocked by DoStructUnitConvert() -- port cUnitConvert.h:105 documents it as TODO(W6+W7) …
    DoStructUnitConvert();
#endif
```

### 3.6 修掉一個「看起來本來就對」的缺陷

`EtherCAT/Pci1203Control.cpp:1336-1345`

```
//AI(W906-1203ALM-4) 20260912: ⚠ THIS WAS `uiDevhand` AND THAT WAS A BUG.
//  The precondition check twenty lines up already says "the observer owns
//  the device handle" -- and then this line took PRODUCTION's handle instead.
//  In OWNED mode uiDevhand is ZERO by definition …
//  Every device-handle command below -- both DO writes -- was therefore
//  issued against handle 0 and could not work. Axis commands were fine:
//  they already came through mon->axisHandle_().
//  ⓘ In ATTACHED mode this is the same value uiDevhand holds, so the path
//  that did work is unchanged.
```

### 3.7 更正／宣告前一則註解已過期（**移植樹最有特色的一條**）

`cSocket.h:75-84`——**舊話不刪，另起一則標過期**：

```
//  AI(W906-FW3-PICTL) 20260829: ⚠ **上一行那則 absence-claim 已過期**。
//  它的量測日期是 20260807；在那之後，**cSocket.cpp:230-231 已經真的定義了
//  這兩個符號** …
//  驗證到 archive 層而不只是檔案層：
//    nm --defined-only --extern-only build_pioee1g/libht9045_sm.a
//      -> `B _NowControlBinCategory` / `B _OldControlBinCategory`（均已定義）
//  上面那句話在 20260807 是真的，**不改寫**；但不能再拿它當前提用。
//  （陳述的是陳述人當時量到的事實，不是永久的事實——陷阱 #2。）
```

同型的還有 `aHotPlateSubstrate.cpp:423`（「⇒ 那句『不在範圍內』已經過期，留著會讓人以為還有債沒還」）、以及全樹 11 處 `AI(W906-NL-ABSENCE) 20260916`（`cinitial.cpp:11045/12402/16033`、`csystem.cpp:20341/23302/23815/30516`、`ainarm2.cpp:2992`、`ainarm9045.cpp:1317`…），全部都在宣告「某個『找不到／不存在』的量測已經半過期或全過期」。

⚠ **這條規則要背下來**：absence-claim（「全樹 0 個呼叫點」「這個檔不存在」）一定要寫量測日，而且發現它過期時**不要改寫原句**，另起一則。

---

## 四、註解裡寫什麼

| 要寫 | 真實例 |
|---|---|
| **golden 檔名＋行號** | `golden cSortCT.cpp:577`、`golden :76`、`note.cpp:1235 KeyComp[]`、`common.cpp:2466` |
| **本樹檔名＋行號**（跨檔指路） | `dialog-page.js:25-28 的 ALARM_BTNS`、`:2872 的 PumpTick()`、`WebStart.cpp:1486/1914/2064/…` |
| **實測值＋量測日** | `12,358 non-zero bytes`、`HSys.* 420 keys from Gerneral.ini`、`nine SERVOPACKs at 0/1/3/10/14/30/41/124/153`、`measured 20260813`、`all 75 DO ports read 0x00` |
| **重現／驗證用的指令** | `nm --defined-only --extern-only build_pioee1g/libht9045_sm.a`（`cSocket.h:81`） |
| **為什麼不用另一個做法**（常以大寫小標開頭） | `WHY IT LIVES HERE AND NOT IN WebBridge/`（`WebBridgeTags.h:6`）、`WHY A FIXED TABLE AND NOT A DIRECTORY SCAN`（`wb_serve.cpp:698`）、`WHY RETURNING 0 IS CORRECT` 段（`wb_serve.cpp:319`） |
| **錯了會怎樣，用操作員看得到的話講** | 「在一台跑 130 C 的機台上，`0.00` 與 `---` 對站在前面的操作員意義完全不同」（`WebBridgeTags.h:20-23`）、「⇒ 關不掉、答不了、機台永久卡住，而且 log 裡什麼都沒有」（`wb_serve.cpp:293`） |
| **裁決來源** | `AI(W906-WEB-W2b) 20260917 (裁決 A1)`（`wb_serve.cpp:520`）、`FW-TEMP1's "unsafe, do not wire" verdict … was overruled by the main loop`（`WebBridgeTags.h`） |
| **精度修正**（前一句話只有部分為真） | `PRECISION ON THAT, measured 20260813: "null renders as ---" holds only for the formatters that say so -- dash/int/fixed*/pct*/hms (web/js/ui/bind.js:49-72)`（`WebBridgeTags.h:25-30`） |

**不要寫**：這一行在做什麼（程式碼自己會說）、`// TODO` 不帶標籤與日期、「暫時這樣」不寫條件。

---

## 五、標記符號（⚠ 這類）的用法

實測出現次數（排除 `build/` 以外未再細分）：

| 符號 | 次數 | 慣例用法 | 真實例 |
|---|---|---|---|
| `⚠` | **1,228** | 「不照這條會壞」／「與直覺相反」／「這個前提已過期」 | `⚠ RESCAN IS EXEMPT FROM "must be open".`（`Pci1203Control.cpp:547`） |
| `⚠⚠` | **84** | 升一級：**會讓整台機台卡死、或無聲地做錯事**的那種 | `⚠⚠ **kCode 守門，放在最前面。**`（`wb_serve.cpp:288`）、`⚠⚠ 信箱那一側也要退役。`（`:418`） |
| `ⓘ` | **86** | 補充事實，**不是行動項**；讀者不照做也不會壞 | `ⓘ In ATTACHED mode this is the same value uiDevhand holds, so the path that did work is unchanged.`（`Pci1203Control.cpp:1344`） |
| `⇒` | **260** | 從前面的事實**推出的結論或指示** | `⇒ 寧可在這裡就大聲講，也不要讓操作員對著一個沒有按鈕的框。`（`wb_serve.cpp:295`） |
| `✅` | 9 | 很少用，不建議新增 | — |

另外觀察到：
- `⚠` 可以放在 `AI(` **前面**（`// ⚠ AI(W906-P1b) 20260920 更正：`，`aHotPlateSubstrate.cpp:423`）或**後面**（`//AI(W906-1203ALM-4) 20260912: ⚠ …`）。兩種都算合格；**整句都是警告**時放前面，**只有其中一段是警告**時放後面。
- 英文區塊常把警告句整句**大寫**（`⚠ THIS WAS uiDevhand AND THAT WAS A BUG.`）；中文區塊用 `**粗體**` 代替（`⚠ **上一行那則 absence-claim 已過期**`）。

---

## 六、語言、編碼、與 golden 註解的處置

1. **語言**：全樹中英夾雜。觀察到的分界是時間——`WebBridge`／`1203*` 這些較早（2026-08 前後）的奠基區塊是英文，20260915 之後新增的（`Q30-*`、`WEB-W2b`、`SETUP-READFILE`、`P1b`）幾乎都是繁中。**規則：同一個註解區塊內不要中英混寫**；接續別人的檔就沿用該檔既有語言。
2. **編碼**：移植樹一律 **UTF-8 無 BOM**（Big5 只適用 BCB 那棵樹）。`cSocket.h:86-88` 有一條可直接沿用的收尾 gate：`Big5: this header has no Chinese comments to preserve (checked cp950-decoded golden text line by line). Final gate: ZERO U+FFFD.` —— **翻譯自 golden 的檔，註解裡要留下「U+FFFD 為零」這句驗證**。
3. **golden 自己的註解一律原樣保留**，包含作者與日期：`//Ifor 20191121 RTC 讓位時不重新讀取位置避免發生異常`（`cinitial.cpp:7178`）、`//kevin 20130722 cancel K_FIX`（golden `note.cpp:1244`）。它們是為什麼某行長那樣的唯一證據。
4. **人手寫的小註記**用 `//<人名> YYYYMMDD (<代號>):`，與 AI 區塊分得開：`//Steven 20260916 (R1): real folder, not the --dry scratch`（`wb_serve.cpp:526/530`）。

---

## 七、本專案（JsonBridge）建議用的標籤前綴

### 7.1 提議

| 標籤 | 用在哪 |
|---|---|
| `AI(W906-JSONBRIDGE-S0)` … `AI(W906-JSONBRIDGE-S11)` | 各期實作。編號直接沿用 SKILL.md §八 的 S0～S11，不另編 |
| `AI(W906-JSONBRIDGE-GEN)` | `tools/gen_sjson.py` 本身，以及它寫進 `JsonBridge/gen/*.gen.cpp` 檔頭的橫幅 |
| `AI(W906-JSONBRIDGE-RULE)` | SKILL.md §二 那些**跨期、會散落在很多檔**的裁決：陣列形狀、`dialog.response`、UTF-8 直通不轉碼、`_NET` 不開、`*.live` 唯讀 |

續作與修正照既有尾碼慣例：`-S4b`（同一期第二刀）、`-S4fix`（修自己那期的缺陷）、`-S6-integrate`（併回主線）。

#### ⚠ 更正（20260923）：S8～S11 換成 `SJSON-`，所以這個專案現在有**兩個**標籤家族

**上面那則提議不改寫**（它在 S0～S7 是真的，而且那幾期的程式碼就是照它做的）。
但從 S8 起換了家族，這裡必須記下來，否則一個 grep 只抓得到半個專案。

量法：`grep -rn "AI(W906-<家族>-" --include=*.cpp --include=*.h --include=*.txt .`，排除 `build/`。

| 家族 | 涵蓋 | 實測筆數（20260923） |
|---|---|---|
| `AI(W906-JSONBRIDGE-S0)` … `-S7` | S0～S7（已推送） | **60** |
| `AI(W906-SJSON-S8)` … `-S11` | S8～S11（本波次，未 commit） | **44** |

兩個家族**不重疊**，實測：
`grep -rn "AI(W906-JSONBRIDGE-S8\|…S9\|…S10\|…S11"` → **0 筆**；
`grep -rnE "AI\(W906-SJSON-S[0-7]\)"` → **0 筆**。
- 要一次抓完整個專案：`grep -rnE "AI\(W906-(JSONBRIDGE|SJSON)-S"`。

**為什麼換**：使用者／派工單在 S8 開工時明確指定 `W906-SJSON-S8`／`-S9`／`-S10`／`-S11`，
理由是 Jimmy 當晚要用它 grep 出「哪幾行是這一波加的」。這是指令，不是偏好。

**代價與建議**：§二 的判準是「標籤＝一個可 grep 的變更集」，兩個家族並存並不違反它
（每一期仍然各自是一個變更集），但**專案層級的 grep 多了一個要記的細節**。
若日後要統一，往哪個方向都只是一行 `sed`；**我沒有自作主張去改已推送的 S0～S7**，
因為那會讓已經發出去的 commit 訊息與信件裡的標籤對不上。

⇒ 給下一個人的規則：**接續 S8～S11 的工作用 `SJSON-`，回頭修 S0～S7 的東西用 `JSONBRIDGE-`**
（標籤跟著「那件事」走，不是跟著「檔案」走 —— §二判準）。

### 7.2 為什麼這樣取

1. **標籤必須等於一個可 grep 的變更集**（§二判準）。SKILL.md §八 已經把這件工作切成 12 期、排好相依順序，而 §七 的六個 gate（G1～G6）是**逐期套用**的。一期＝一個標籤，`grep -rn "AI(W906-JSONBRIDGE-S4)"` 回來的東西剛好就是「S4 這期碰過的所有地方」，跨 `JsonBridge/`、`JsonBridge/gen/`、`tools/wb_serve.cpp`、`tests/` 四處都抓得到。驗收時不必另外維護一份「這期改了哪些檔」的清單。
2. **和全樹最大的幾個標籤同形**。`W7-B1c`（403）、`PT-W2`（154）、`FW3-Observer-W2`（118）全都是「戰役－波次」兩段式；`JSONBRIDGE-S4` 就是同一個形狀，接手的人不用學新規則。
3. **不用單一扁平標籤**（例如只寫 `AI(W906-JSON)`）：全樹已有 523 個標籤，最大的一個也只有 403 次而且只涵蓋一個波次。一個 12 期的專案共用一個標籤，grep 出來會是幾千處，等於沒有標籤。
4. **不用模組名當標籤**（`-TESTIF`、`-LEVELSET`、`-ALARM`）：檔名已經說了是哪個模組，標籤再說一次沒有新資訊；而且同一個結構會在 S2（讀）、S6（寫）、S8（串流）被碰三次，模組名分不開這三次。既有 523 個標籤裡也找不到純模組名的先例。
5. **`GEN` 另立**，不隨期別跑：`gen/*.gen.cpp` 是產生器輸出、會整檔重寫（SKILL.md §4.9：「只由產生器寫，不手改」）。若檔頭橫幅帶期別標籤，每重跑一次產生器 `git diff` 就會出現一堆與內容無關的標籤異動。固定一個 `GEN` 讓 diff 只反映欄位變化。
6. **`RULE` 另立**：§二 那幾條裁決會出現在十幾個不相干的檔（每個 `Clamp*`、每個解析器）。它們不屬於任何一期，用期別標籤會把它們綁死在「第一次寫下它的那一期」，日後想查「所有依賴陣列形狀裁決的地方」就查不到。

### 7.3 本專案額外的三條硬性要求（來自 SKILL.md，寫成註解形式）

1. **`gen/*.gen.cpp` 每檔開頭要寫來源 golden 檔與行號範圍**（§4.9）。格式沿用 `cUnitConvert.cpp:1-19` 的檔頭：
   ```
   // ==========================================================================
   //  sjson_SYSTEM_TEST_IF.gen.cpp  -- GENERATED, DO NOT EDIT.
   //  AI(W906-JSONBRIDGE-GEN) 20260924.  NOT in golden.
   //  來源：golden HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cSetUp.cpp:2199-3010
   //        （ReadFile() 的 ReadIniData 呼叫，99 鍵）
   //  產生器：tools/gen_sjson.py  版本 <n>  產生時間 <YYYY-MM-DD HH:MM>
   //  要改欄位請改 golden 或改產生器，不要改這個檔。
   // ==========================================================================
   ```
2. **每個 `Clamp*` 的檔頭要註明搬自 golden 哪幾行**（§4.1）。例：
   ```
   //  AI(W906-JSONBRIDGE-S6) 20260924: ClampHotPlate
   //  搬自 golden cHotPlate.cpp:495-510（SaveSetupFile 裡「存檔時才做」的鉗制）。
   //  ⚠ 這是丟掉 VCL 控制項後**唯一會遺失的邏輯**（SKILL.md §3.1）。
   //  ⇒ 純函式：吃結構、回結構，不碰檔、不碰全域，才能在 dryRun 跑（§五 第 6 條）。
   ```
3. **absence-claim 一律帶量測日**。本專案已經寫下好幾條（「`Offset` 族三個檔在移植樹不存在」「`io.*`／`motor.*` 在 `WebBridgeTags.cpp` 0 個」「`UN150Read[]` 的寫入執行緒離線時不啟動」）——這些正是 `cSocket.h:75` 那種會過期的句子。寫進程式碼時**必須**帶 `20260923` 與量法，日後發現過期照 §3.7 另起一則，不改寫原句。

---

## 八、一頁檢查表

寫完一段 JsonBridge 的程式碼，對照這五題：

1. 有沒有**與 golden 不同**的地方？有 → 寫，附 golden 檔名行號。
2. 有沒有**看起來可以刪、其實不能刪**的行（守衛、include、順序）？有 → 寫，附「刪了會怎樣」。
3. 有沒有引用**別人量到的事實**？有 → 寫量測日與量法，不要只寫結論。
4. 有沒有**推翻或修正**前面某一則註解？有 → 另起一則標過期，**不要改寫原句**。
5. 用了 `⚠` 嗎？—— 確認它真的是「不照做會壞」；只是補充事實請改 `ⓘ`，只是結論請用 `⇒`。

都沒有 → **不要寫註解**。移植樹 2,194 行的 `WebBridgeTags.cpp` 只有幾十處標籤，那是刻意的。
