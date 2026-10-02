# §十二、20260923 審查更正（審查員獨立查證 12 條宣稱的結果） —— 全文

> 從 `SKILL.md` **§十二** 拆出（拆出日期 2026-09-23，skill 維護第二輪續拆）。
> **章節編號不變**：§十二 同名同號，R1～R9 編號不變，「我自己的更正」與「尚未查證」兩小節原位保留。原文**逐字**保留（含已被更正段推翻的原句 —— 那是稽核軌跡），
> `SKILL.md` 原處留 stub（一句話結論 ＋ 指到本檔）。
> 原本被誰引用：`JsonBridge/actions/MainClarnData.h:30`（R6）、`:61`（R8）、`JsonBridge/ChanAction.h:55`（R8）、`tests/test_sjson_chan.cpp:13`（R8）、`JsonBridge/Bindings.cpp:84`；SKILL.md §〇、§八 S3／S4 那兩列、§4.1。
> 本檔內的 `§X`／`§4.x` 交叉引用一律指 `SKILL.md` 的章節編號，`SKILL.md` 那裡搜得到、再一跳就到對應 reference；
> `references/<檔>.md` 這種路徑是以 `SKILL.md` 所在目錄為基準寫的（原文未改），在本檔內即同目錄的 `<檔>.md`。

---

## 十二、20260923 審查更正（審查員獨立查證 12 條宣稱的結果）

規格寫完當天派了一位獨立審查員把 §三 的每一條「實測值」重量一次。結論：**主要結論都站得住，有 9 處細節要改。**

⚠ 另有一條我一開始判成「審查員錯了」，後來證實**是我錯**（`SYSTEM_TEST_IF` 的成員數）。更正見本節最後一小節，那是這次最嚴重的一條 —— 它決定 S4 的規模是 788 欄還是 351 欄。

### 已確認錯誤，照這裡的數字為準

| # | 原文 | 更正 | 影響 |
|---|---|---|---|
| R1 | `DoStructUnitConvert()` 本體在 `cUnitConvert.cpp:370-693` | 本體在 **`:675`**（全檔 693 行）。`370-693` 是「`DoTestIFConvert` 到檔尾」整段 | 只是引用不精確 |
| **R2** | 「移植樹 `cSecurity.cpp:74` 起同樣 180 筆」 | ⛔ **整段在 `#if 0` 內**（gate SEC1，`:71`-`:259`，原因見 `forms/fSecurity.h`：`fMain` 沒有任何 `TSpeedButton *sbXXX`、`Graphics::TBitmap` 全樹無實作）。執行期 `mySecurityPal` 空、`iMaxLevelItem==0`。而且標號只到 `[178]`，比 golden 少一筆 | **S3 的前提沒了。**名字要嘛在 build 時從 **golden** 抽，要嘛先解 SEC1 閘（而那要先有 `sbXXX` 與 `TBitmap`，是另一個波次）。S3 動工前必須先決定走哪條 |
| R3 | golden 有 **24** 個 `DoStructUnitConvert()` 呼叫點 | **22**（24 是 grep 行數，含 `cUnitConvert.h:5` 宣告與 `.cpp:254` 定義） | 數字而已 |
| R4 | `SaveAllFile()` 對 **12 個表單**一律先 `DoIniDataToForm()` 再 `SaveSetupFile()` | **11 個**，而且 **2 個破例**：`fOffSet` 的 `DoIniDataToForm` 有條件（selector `!=-1`）但 `SaveFile` 無條件跑兩次；`fYieldMonitoring` **只有 Save、完全沒有 ToForm** | S6／S11 對 `SaveAllFile` 建模時不能用「一律」 |
| R5 | `canary_support.cpp:116 RecordProcess` 是空 shim | **不是空的** —— 它 `printf("  [RecordProcess] …")` 到 stdout。真正空的是 `acatchtray_shims.cpp:152 NewRecordProcess(){}`。另有第三種：`common.cpp:62` 一個 **TU-local `static RecordProcess(){}`**，`common.cpp` 內部的呼叫連 stdout 都到不了 | **S1 要分三種談。**照原文會做出多餘的東西、又漏掉真正沒留痕的那半 |
| R6 | （未寫）移植樹的 `Clarn_Data` | `forms/fMain.cpp:484` 是**空殼**：`void TfMain::Clarn_Data(int, AnsiString) { W906_Clarn_DataCallCount++; }`。191 行本體一行都沒翻 | **S11 的第一刀等於從零翻譯**，不是「包一層」。`fCounterClear->ClearCount` 真本體倒是有（`cCounterClear.cpp:119`），但 `Automation/auto9045.cpp:378` 另有一個 TU-local 空 no-op |
| R7 | G5 驗收用 `wb_publish --pump` | **`wb_publish`／`wb_gateway` 已於 20260918 退役**（`CMakeLists.txt:3186-3199`），而且 `wb_serve` 沒有 `--pump` 旗標 | **G5 的驗收方式要重寫** |
| R8 | （未寫）`--dry` 的保護範圍 | `wb_serve.cpp:2341-2344` 自己印：`--dry` **只隔離 C++ loader**，`/api/system`、`/api/recipe` 讀寫的是**真檔**。`struct.put`→`Persist` 正好走那條 | **S6 的驗證不能靠 `--dry` 當保護網**，只能靠「備份→驗證→刪備份」 |
| R9 | `golden cUnitConvert.cpp` 的 10 個 ×100 欄位 | golden 10 個，**移植樹只翻了 6 個**（`cUnitConvert.cpp:376-381`），缺 `dMulti2DSH1Ofs_L/R`、`dMulti2DSH2Ofs_L/R`。那四個欄位在移植樹存在但目前無其他使用者 | 翻譯缺漏，S4 要補 |

### ⛔ 我自己的更正：那條「反向推翻」是錯的，審查員原本就對

本節第一版寫著「審查員把 `SYSTEM_TEST_IF` 說成 788/796 是錯的，351 才對」。**那是我的量測缺陷**，審查員的 788/796 才是對的。

我的 `struct_size.py` 從 `} SYSTEM_TEST_IF;` 往回做大括號配對，**但配對是在未剝註解的原始碼上跑的**。而 `cprod.h:2156`（golden `:2174`）正好是：

```cpp
bool    bAlarm4EnableIntervalYield;   //20150604 Mylin Interval Total Yield Difference {
```

行尾註解裡有一個 `{`。反向配對咬住它，於是我量到的「結構」其實只是它的**尾巴**。

可重現（20260923 實測）：

```
sed -n '2156,2575p' cprod.h | sed 's://.*::' | tr -cd '{' | wc -c   ->  0     # 我的「結構」裡零個 {
sed -n '1650p' cprod.h                                              ->  {     # 真正的開頭
sed -n '1651p' cprod.h                                              ->  int  iTestMode;
sed -n '1651,2574p' cprod.h | sed 's://.*::' | tr -cd ';' | wc -c   ->  788
grep -n "double\s*dSiteXPitch" cprod.h                              ->  1656
```

最硬的一條不靠任何計數：`dSiteXPitch` 在 `:1656`，比我宣稱的起點 `:2156` 早 500 行，而 `TestIF.dSiteXPitch` 在 `cUnitConvert.cpp:376` 被指派且編得過。若結構真的從 2156 開始，那行不可能編譯。

**正確數字：`SYSTEM_TEST_IF` 移植樹 788 個成員（55 個陣列）、golden 796。** 我的 351/357 是 tail-only，漏掉 head 區的 437/439 個成員。

⚠ **方向是反的，而且反得很嚴重**：`a165cc0` 的 commit message 與 20260923 13:47 那封信都寫著「照審查員改會去找 437 個不存在的欄位」。實際上那 437 個是真實存在的成員，**照 351 做的產生器會靜默漏掉一半以上**。自我檢查：表⑧ 目前唯一標 `bridgeStaged` 的三個欄位裡，有兩個是 `TestIF_File.dSiteXPitch`／`dSiteYPitch`，它們在 `cprod.h:1656/1657` —— 正落在被我說成「不存在」的那一區。計畫已經在橋接自己模型說不存在的欄位了。

⇒ **S4 的規模是 788 欄不是 351 欄。** 產生器一律要從**剝過註解**的原始碼做配對。

留這一整段（含我錯的版本與更正）不是為了記帳，是因為它示範了兩件事：推翻別人時，推翻本身要能重現；而「我獨立量了三次都一樣」不構成證據 —— 三次用的是同一支有缺陷的腳本。

### 尚未查證

`DoAutoCleanKit` 未來若經 MainProc 階梯解閘會不會讓 `DoStructUnitConvert` 變成可達 —— 審查員只查到「目前跨檔零呼叫者」，沒有做完整 call graph。

