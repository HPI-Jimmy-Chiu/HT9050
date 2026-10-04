# Golden 缺陷統籌台帳（GOLDEN_DEFECT_LEDGER）

## 目的

V906 是 BCB6 handler（golden）忠實翻譯成標準 C++ 的移植樹。翻譯紀律規定：**看到 golden
裡不合理的邏輯，一律照翻，並在 `//AI` 註解寫下它為什麼看起來錯；要改行為必須是使用者
明確決定，不可順手「修好」**（見 `AGENTS.md`／各波次 skill 的翻譯紀律章節）。

本台帳是這條紀律累積至今的**統籌記錄**——把散落在 ~90 個 port 檔案、以
`GOLDEN BUG` / `GOLDEN ODDITY` / `GOLDEN ASYMMETRY` / `GOLDEN DEFECT` / `GOLDEN QUIRK` /
`GOLDEN NOTE` / `GOLDEN GAP` / `GOLDEN RACE` / `GOLDEN LEAK` / `GOLDEN INVARIANT` /
`GOLDEN GOTCHA` / `GOLDEN TYPO` / `GOLDEN SPELLINGS` / `GOLDEN DEAD CODE` / `(B1)`~`(B17)`
FW 系列 / `GOLDEN BUG #N` / `k7-B1` 類區域編號等標記寫在程式碼裡的「golden 本身的缺陷、
已忠實翻譯過去、決定不修」的紀錄，**逐筆抽出、集中成一份可查閱的表**。

**這是統籌記錄，不是修復清單**：本台帳建立過程完全沒有修改任何既有原始碼，純掃描與彙整。
使用者 20260819 裁決原文意：golden bug 一律不修、照翻，但要一份統籌文件記錄。

## 快照基準

- 掃描樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`
- git HEAD：`c590952`（`c5909524391b5890f8630db12a8b95bbc4c25054`）
- HEAD 日期：2026-08-19 08:21:10 +0800
- 台帳建立日：2026-08-19

## 掃描範圍聲明

- 掃描 `*.cpp` / `*.h` / `*.hpp`，排除 `build_*` 系列目錄與 `tools/dfm2rc` 產物目錄。
- **`cShowBinSelect.cpp`、`forms/fShowBinSelect.h`、`tests/test_showbinselect_core.cpp`
  三檔在建立本台帳當下正被另一個波次編輯中（working tree 有未 commit 修改）**，故這三檔
  改用 `git show HEAD:<path>`（HEAD=`c590952`，等同稍早的 `d49eec0`——兩者之間沒有任何
  commit 動過這三個檔案）取已提交內容掃描，避免讀到半成品。本台帳收錄的
  `cShowBinSelect.cpp` / `forms/fShowBinSelect.h` 筆數，是該 HEAD 快照下的內容，
  **不代表目前 working tree 的最新狀態**；若日後那個波次 commit 後這兩檔又新增了
  golden 缺陷標記，需要下一次維護時補收。
- 除了任務指定的例示標記樣式外，本次掃描過程中另外發現這棵樹實際使用的完整詞彙表遠比
  例示豐富，包含 `GOLDEN QUIRK(S)`（250 筆原始命中，是全樹使用頻率最高的變體）、
  `GOLDEN DEFECT(S)`（50 筆）、`GOLDEN ASYMMETRIES`、`GOLDEN ODDITIES`（複數形）、
  `GOLDEN INCONSISTENCY`、`GOLDEN GAP`、`GOLDEN RACE`、`GOLDEN LEAK`、`GOLDEN INVARIANT`、
  `GOLDEN GOTCHA`、`GOLDEN TYPO`、`GOLDEN SPELLINGS`、`GOLDEN DEAD CODE` 等，全數視為同一
  類別（golden 本身缺陷/怪異之處被保留翻譯）納入掃描，並據此擴大波次的委派範圍。
  另外排除了明顯只是敘述性英文、並非缺陷標記的巧合鄰接詞組（如 `GOLDEN LINE(S)`、
  `GOLDEN VERBATIM`、`GOLDEN ITSELF`、`GOLDEN FILE`、`GOLDEN HEADER` 等——這些是「golden
  第N行」「逐字翻譯自golden」之類的一般敘述，不是缺陷標記）。
- **重要排除：GATE 不是 golden 缺陷。** 這棵樹另有一套完全不同概念的 `(Bx)` / 編號
  「GATE REGISTER」慣例——標記某個 golden 呼叫因為對應的 UI 元件/子系統尚未翻譯過來，
  本波次先用 fail-closed 替代值頂著、待使用者同意後才真正接線。GATE 描述的是「golden
  邏輯本身沒問題，只是 port 還沒接上」，跟「golden 本身就有缺陷」是相反的兩件事。
  本台帳**只收錄註解文字本身確實使用了上述 GOLDEN-缺陷詞彙的項目**；純 GATE 編號
  （即使剛好也叫 `(B5)`、`(B8)` 之類）一律不收。同一個檔案裡 `(Bx)` 編號可能同時存在
  「大多數是 GATE、少數是真缺陷」的情況（`forms/fShowBinSelect.h` 的 GATE REGISTER
  就是這種例子：(B1)~(B11) 全是 GATE，只有 (B12)(B13) 是真的 golden 缺陷）。
- **編號不是全域唯一識別碼。** `(Bx)`／`#N` 這類本地編號的作用域只到「同一份 banner
  註解」為止：`Command.cpp` 有自己一組連續的 B1~B8（跨 FW3-WC/WD/WE 三個 banner）；
  `forms/fShowBinSelect.h` 的 GATE REGISTER 有自己一組 B1~B13；`SCK_ART_Remainder.cpp`
  同一個檔案裡甚至有**兩組互不相干、各自從 #1 起算**的「GOLDEN BUG #N」（
  `AI(W906-Save2DSortingSummary)` 系列與 `AI(W906-SCKART-Multi)` 系列）。本台帳的
  「標記/編號」欄一律連同來源檔名/系列名一起標示，不可跨檔案比對編號。
- **GOLDEN DIVERGENCE 是相反方向的一類，仍收錄但特別標示。** 極少數註解（`Automation/
  SCK_ART.cpp`、`csystem.cpp` 的 W7C1/W7C2 SckArt seam）用的是「GOLDEN DIVERGENCE」，
  這描述的其實是 **port 自己刻意偏離 golden 真實行為**（而非 golden 本身有缺陷），
  依專案政策揭露記錄、不隱瞞修復。本台帳仍收錄（因為對照關係與風險性質很接近），但在
  「類型」欄標注為 `DIVERGENCE（port偏離golden，已揭露）`，避免與「golden自身缺陷」混淆。
- golden 位置一律照抄 port 註解裡原樣的 `golden :NNNN` 引用；未特別指名檔案者，預設是
  對應的同名 BCB6 原始檔（通常在 `HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422`）。
  本台帳**未逐筆回頭比對 golden 原始碼驗證每一個行號**——這些引用本身就是各翻譯波次
  當時對照 golden 寫下的紀錄；已知至少一筆（Command.cpp 的 `(B3)`）在整併時被下一波
  自己發現行號引用錯誤並更正過，供使用時留意。

## 統計摘要

| 項目 | 數值 |
|---|---|
| 總筆數（逐筆列出的 golden 缺陷/怪異之處） | **726 筆**（以表格內 `#` 編號列實際計數，20260825 FW-CFG-W7 當波入帳 1 筆（cConfiguration.cpp 分節 4->5 筆）＋20260825 FW-OBS-W2 當波入帳 42 筆（`cObserver.cpp` 新增分節，該檔本波前為 0 筆）＋20260825 FW-TAG1 當波入帳 1 筆（`MyTempPanel.cpp` TAG1-a，維護規則 1）＋20260825 R8 同步 3 筆（FW-BARCODE1 當波入帳）＋20260824 R7 同步 7 筆（FW-BINDISP3 當波入帳）＋20260824 R6 同步 8 筆（FW-QWKEY1 當波入帳）＋20260821 四輪補掃後＋20260824 GATE7-V 增 1 筆＋20260824 五輪同步 5 筆（FW-BINDISP1/SETUP-D/BINDISP2 當波入帳，維護規則 1）；`python3` 逐列計數，見下方「四輪補掃紀錄」） |
| 其中 BUG 類（`GOLDEN BUG` / `GOLDEN BUGS` / `GOLDEN BUG #N` / `(Bx) GOLDEN BUG` / golden copy-paste bug） | 265 筆（含 20260825 FW-CFG-W3 +1，`cConfiguration.cpp` CFG-N14esc；含 20260825 FW-OBS-W2 +11，`cObserver.cpp` 新分節；含 20260825 FW-TAG1 +1，`MyTempPanel.cpp` TAG1-a edSHighBase 漏設 Tag；含三輪新增 1 筆，`cConfiguration.h` 的 `sbUpdateHPClick` copy-paste bug） |
| 其中 QUIRK 類（`GOLDEN QUIRK` / `GOLDEN QUIRKS`） | 248 筆（含 20260825 FW-OBS-W2 +8，`cObserver.cpp` 新分節；含 SECSGEM 三檔補撈的 39 筆；含二輪補掃新增 71 筆，見下方「二輪 QUIRK 補掃紀錄」；三輪未新增 QUIRK 類；含 20260824 五輪 +1，`cSetUp.cpp` CHSetError int>63.5；含 20260825 R8 +1，`BarcodeReader.cpp` BR-q1 15-of-16 初始器） |
| 其中 DEFECT 類（`GOLDEN DEFECT` / `GOLDEN DEFECTS`） | 71 筆（含二輪補掃新增 5 筆；含三輪新增 1 筆，`BinDisplay/MyBinDisp.cpp` 的 `WriteTargetBin` off-by-one；含 20260824 R7 +1，同檔 (i) 21-into-20 sprintf stack overflow） |
| 其中 ODDITY 類（`GOLDEN ODDITY` / `GOLDEN ODDITIES`） | 75 筆（含 20260825 FW-CFG-W7 +1，cConfiguration.cpp edtSearchFunctionChange 只認 0 與 >=2 字；含 20260825 FW-CFG-W4a +1，`cConfiguration.cpp` CFG-L11tag；含 20260825 FW-OBS-W2 +10，`cObserver.cpp` 新分節；含三輪新增 16 筆，集中在 batch-5 顯示側叢集，見下方「三輪增補紀錄」；含四輪補掃新增 6 筆，集中在溫控表單 uTemp_Set.cpp/DynamicTemp.cpp 與 Command.cpp FW-CMD-C 段，見下方「四輪補掃紀錄」；含 20260824 五輪 +3：`cSetUp.cpp` ×2（pitch switch 缺 N-mode case、CoSocketComboChange 無視 Sender）＋`database.cpp` ICBD-1；含 20260824 R7 +6，`BinDisplay/MyBinDisp.cpp` (j)(k)(l)(m)(n)(o)） |
| 其中 ASYMMETRY 類（`GOLDEN ASYMMETRY` / `GOLDEN ASYMMETRIES`） | 16 筆（含三輪新增 1 筆，`BinDisplay/MyBinDisp.h` 的 `ComPort`/`ComPort2` 初始化不對稱；含 20260824 GATE7-V 新增 1 筆，Command.cpp SETSOAK_ 寫入路徑不對稱） |
| 其中 DIVERGENCE 類（port 自身偏離 golden，已揭露，非 golden 本身缺陷；csystem.cpp/SCK_ART.cpp 各 1 筆合計 2） | 2 筆 |
| 其中 GAP / LEAK / NOTE / INCONSISTENCY / GOTCHA / SPELLINGS / TYPO / DEAD CODE / INVARIANT / RACE 等罕見詞彙 | 40 筆（NOTE 28／GOTCHA 2／GAP 2／LEAK 2／INCONSISTENCY 1／SPELLINGS 1／TYPO 1／DEAD CODE 1／INVARIANT 1／RACE 1；含三輪新增 NOTE +2、GOTCHA +1，皆在 `cShowBinSelect.cpp`/`BinDisplay/MyBinDisp.h`；含 20260824 五輪 NOTE +1，`Motor/myMN200motor.cpp` BINDISP2-e；含 20260824 R6 NOTE +8，fQwertyKey G-a..d ×4＋fPassword G-P1..P4 ×4；含 20260825 R8 NOTE +2，BarcodeReader BR-n1/n2；含 20260825 FW-OBS-W2 NOTE +13，cObserver.cpp 新分節） |
| 涉及檔案數 | 137 個 `### ` 分節（20260825 FW-CFG-W3 新增 1 個分節：cConfiguration.cpp；20260825 FW-OBS-W2 新增 1 個分節：cObserver.cpp（42 筆，本檔單一分節筆數目前最高者之一）；20260825 R8 新增 1 個分節：BarcodeReader；20260824 R6 新增 2 個分節：fQwertyKey 家族、fPassword 家族；20260824 五輪新增 2 個分節：`database.cpp`、`Motor/myMN200motor.cpp`；`cSetUp.cpp` 由 stub 分節升級為 3 筆實表不重複計；含少數純交叉參照的 stub 分節；三輪增補新增 8 個分節：`forms/fLotInfo.cpp`、`forms/fSpeed.h+cSpeed.cpp`、`forms/fStartCondition.h+cStartCondition.cpp`、`forms/fConfiguration.h+cConfiguration.cpp`、`forms/fSetup.h+fSetup.cpp`、`BinDisplay/MyBinDisp.h+.cpp`、`forms/fHandlerSys.h+HandlerSys.cpp`(stub)、`tests/test_amr.cpp`(stub)；四輪補掃新增 4 個分節：`uTemp_Set.cpp+forms/fTemp_Set.h`、`DynamicTemp.cpp+forms/fDynamicTemp.h`、`MainTempMode.cpp`(stub)、`cSetUp.cpp`(stub)；`cShowBinSelect.cpp`/`forms/fShowBinSelect.h` 的 WAVE D/E 補充是既有分節的延伸子分節，不重複計檔案數；Command.cpp 本輪只新增 2 列（B-CMDC-1/2），沿用既有分節，非新分節） |
| 標示 ⚠️存疑（agent 或 orchestrator 本人判讀信心不足） | 見文末「無法判讀/存疑清單」 |

> 上列數字為 `python3 -c "..."` 對本檔表格列直接計數所得（見文末補撈紀錄的量測方式），
> 非估算值。多筆同一缺陷在不同呼叫點重複提及時，以「主要定義處」算一筆；同一缺陷若橫跨
> `.cpp`/`.h` 兩個檔案各有一段完整說明，也算一筆並在該筆的 port 位置欄列出兩個檔案。
> **逐一精確複算的權威來源永遠是本檔案下方各分節的表格本身**，此處統計僅供快速瀏覽。

### 二輪 QUIRK 補掃紀錄（20260819）

第一輪委派後，`ainarm2.cpp`（14 筆原始 QUIRK grep 命中）、`ainarm9045.cpp`（10 筆）、
`aTester_Rear.cpp`（9 筆）、`aoutarm9045.cpp`（3 筆）、`cinitial.cpp`（3 筆）、
`Public/MyStringList.cpp`（4 筆）、`OCRInsp.cpp`（2 筆）七個檔案的 QUIRK 覆蓋度被記錄為
「未經第二輪複查」的已知缺口（見上方補撈紀錄段落）。本次逐檔對這 7 個檔案重新以
`grep -inw quirk` 掃描並逐一讀取上下文判讀，結果：

- 新增 76 筆（71 QUIRK + 5 DEFECT，DEFECT 出現於 `aTester_Rear.cpp` 的
  `DoBTestSuckTestIC` 一個 banner 與 `cinitial.cpp` 的 `SetTechDataToProd_InArm` 一筆，
  這兩處原始標記是 `GOLDEN DEFECT(S)` 而非 `GOLDEN QUIRK`，但同樣是第一輪未涵蓋、
  掃描中順帶讀到即收錄）。
- 各檔新增筆數：`ainarm2.cpp` +26、`ainarm9045.cpp` +11、`aTester_Rear.cpp` +24（含 4 筆
  DEFECT）、`aoutarm9045.cpp` +4、`cinitial.cpp` +6（含 1 筆 DEFECT）、
  `Public/MyStringList.cpp` +4、`OCRInsp.cpp` +1。
- `OCRInsp.cpp` 的 3 筆原始 grep 命中中，2 筆（:493、:1056）經讀取上下文確認只是既有
  第 4 項（banner (d) 的 static/global `iOCRPosition` 遮蔽缺陷）在另外兩個引用點的重複
  提及，非獨立新缺陷，未收錄——是本次補掃過程中唯一的「排除案例」。
- 各檔新增內容集中在少數幾個先前完全沒有被讀到的函式（例如 `aTester_Rear.cpp` 的
  `DoBRTCAutoModelVerify`、`DoIndexArm1PickUpErrNeedPiggyback`、`CheckAnyCaseNeedToDoArm2`、
  `DoRearTestPurgBeforePickShuttle` 四個函式合計貢獻了 20 筆），而非均勻分散在既有已收
  錄的函式中——這與「原始 grep 樣式漏掉了整段 banner」的假設一致。
- 本次補掃**未回頭複查 BUG / ODDITY / ASYMMETRY 等其他詞彙**在這 7 個檔案是否也有類似
  缺口；範圍嚴格限定在使用者指定的 QUIRK 補掃（DEFECT 兩筆屬掃描中順帶讀到，非系統性複查
  DEFECT 詞彙覆蓋度）。

### 三輪增補紀錄（20260820）

第三輪針對 20260819 晚間至 20260820 上午新落地的 FW3 batch-5 顯示側波次逐檔掃描：
`forms/fLotInfo.{h,cpp}`、`forms/fSpeed.h`＋`cSpeed.cpp`、`forms/fStartCondition.h`＋
`cStartCondition.cpp`、`forms/fHandlerSys.h`＋`HandlerSys.cpp`、`forms/fConfiguration.h`＋
`cConfiguration.cpp`、`forms/fSetup.{h,cpp}`、`cShowBinSelect.cpp`＋`forms/fShowBinSelect.h`
（僅掃 WAVE D/E 新段，WAVE A/B/C 已在原始快照與二輪收錄過）、`BinDisplay/MyBinDisp.{h,cpp}`、
`tests/test_amr.cpp`。

- 新增 22 筆：ODDITY 16、NOTE 2、GOTCHA 1、ASYMMETRY 1、BUG 1、DEFECT 1。
- 各檔新增筆數：`forms/fLotInfo.cpp` +2、`forms/fSpeed.h`+`cSpeed.cpp` +2、
  `forms/fStartCondition.h`+`cStartCondition.cpp` +5、`forms/fConfiguration.h`+
  `cConfiguration.cpp` +4、`forms/fSetup.h`+`fSetup.cpp` +1、`cShowBinSelect.cpp`+
  `forms/fShowBinSelect.h`（WAVE D/E）+4、`BinDisplay/MyBinDisp.h`+`.cpp` +4。
  `forms/fHandlerSys.h`+`HandlerSys.cpp` 與 `tests/test_amr.cpp` 各 +0（逐行核對後確認無
  符合條件項目，已在各自分節留下排除說明，而非略過不提）。
- 掃描樣式：任務指定的 `GOLDEN ODDITY`/`GOLDEN QUIRK`/`GOLDEN DEFECT`/`GOLDEN NOTE`/
  `preserved verbatim`/`off-by`/`照翻`/`copy-paste`/`no-op`，另加本檔「掃描範圍聲明」
  已建立的完整詞彙表交叉核對（`golden's own`/`golden itself`/`kept verbatim`/`misleading`/
  `landmine`/`typo`/`mistake`/`wrong`/`incorrect` 等），逐一讀取上下文人工判讀，非機械式
  關鍵字計數。
- **排除的邊界案例**（判定為「非 golden 缺陷」，詳細理由見各自分節的排除說明，此處僅列摘要）：
  1. `forms/fLotInfo.h:246` 的「recon's row #2 ... copy-pasted onto FormDestroy by mistake」
     ——這是先前 RECON 文件本身的摘要寫錯，不是 golden 缺陷。
  2. `forms/fLotInfo.cpp:421` 的「Same defect class ... as cObserver's sgStatisticsJam」
     ——這是 port 自己的 `TStringGrid` 建構尺寸 bug（已修正），不是 golden 缺陷。
  3. `forms/fLotInfo.h` GATE (WC-3)（`ShowXMLOnLine` 儘管名叫 Show 卻寫檔 x4）——內容類別
     符合這棵樹已知的「Show/Read/Close 前綴會騙人」現象，但該處全文查無任何 `GOLDEN xxx`
     字樣，純屬 GATE REGISTER 的依賴缺口說明，依掃描範圍聲明的判準不收錄；與確有收錄的
     `forms/fStartCondition.h` (SC1)/(SC2)/(SC4) 不同——那三筆 banner 原文明確寫出
     「GOLDEN ODDITY」字樣。
  4. `cStartCondition.cpp:843` 的「DEVIATION: golden's own ... self-reference」——描述的是
     golden 用全域指標自我參照的翻譯適配說明，非缺陷/怪異標記。
  5. 使用者原提示詞列出的「cSetUp FormClose『Close 其實是丟棄未存檔』」一例：經查
     `forms/fSetup.h`/`.cpp` 全文，`FormClose` 本波仍是 32 個 queued（未翻譯）方法之一，
     未附任何 golden 缺陷註記（不同於 `cSpeed.cpp` 已翻譯並附註記的同款 `FormClose` 現象）
     ——本輪未收錄，留待該函式真正翻譯時再記錄，不可與 `forms/fSpeed.h` 的同名現象混淆。
- `BinDisplay/MyBinDisp.h` 的「DEVIATION / GOLDEN ODDITY」banner 下有 3 個子項 (a)(b)(c)，
  僅 (a)（`FlashPro` 的全域自我呼叫）與 (c)（`ComPort`/`ComPort2` 初始化不對稱）收錄；
  (b)（ctor 隱式依賴 BCB6 零值填充）判定為翻譯適配技術說明而非缺陷標記，未收錄——
  這是本輪唯一一處「同一 banner 下部分收、部分不收」的情況，供日後複核。

### 四輪補掃紀錄（20260821，orchestrator 對外工作代號 W906-LEDGER-R2）

本輪是 orchestrator 指定的「二輪 QUIRK 補掃」（外部工作代號 `W906-LEDGER-R2`），對象是
20260818-20260821 新落地的溫控/GPIB 鏡射檔：`uTemp_Set.cpp`＋`forms/fTemp_Set.h`、
`DynamicTemp.cpp`＋`forms/fDynamicTemp.h`、`Command.cpp` 的 FW-CMD-C/D/F 新段（`AI(W906-FW-CMD-C/D/F)`
標籤，:245 起、:11290 起、:15196 起）、`MainTempMode.cpp`、`cSetUp.cpp`。就本台帳自身的內部
編年順序而言，這是繼二輪（20260819）、三輪（20260820）之後的第四輪；沿用內部編年只是為了
避免跟既有段落編號衝突，跟 orchestrator 對外使用的 `R2` 代號不是同一序列，兩者對照關係在此
說明一次，供日後查找。

- 新增 6 筆，全數為 **ODDITY 類**：`Command.cpp` +2（`(B-CMDC-1)`/`(B-CMDC-2)`，皆已在
  20260820 落地當下原地標註於 `//AI(W906-FW-CMD-C)` 註解，本輪是把既有標註併入台帳，非
  本輪新發現）、`uTemp_Set.cpp` +1（`rb1PointClick`）、`DynamicTemp.cpp` +3
  （`edMaxMouseDown`/`edUpperMouseDown`/`FormShow`）。
- 任務標題雖稱「QUIRK 補掃」，但實際掃描結果顯示這批檔案使用的缺陷詞彙清一色是
  `GOLDEN ODDITY`，全樹搜尋未在這 6 個檔案內找到任何字面 `GOLDEN QUIRK` 命中——已依掃描
  範圍聲明的完整詞彙表逐一核對過（見下方各檔排除清單），非疏漏。
- 掃描樣式：任務指定的 `GOLDEN ODDITY`/`GOLDEN QUIRK`/`golden 不合理`/`照翻`/`faithful`+
  `bug`/`oddity`，另加本檔既有詞彙表全集（`GOLDEN BUG/BUGS/DEFECT(S)/ASYMMETRY/ASYMMETRIES/
  NOTE/GAP/RACE/LEAK/INVARIANT/GOTCHA/TYPO/SPELLINGS/DEAD CODE/DIVERGENCE/INCONSISTENCY`）
  逐檔 `grep -inE` 交叉核對，並額外用 `FAITHFULLY KEPT`/`kept verbatim`/`PRESERVED, NOT
  FIXED`/`not a translation artifact`/`golden itself`/`copy-paste` 等本檔已知的非標準措辭
  補一輪，避免只靠字面 `GOLDEN <詞>` 正則漏網（`ainarm9045.cpp` 的
  `ONE STRUCTURAL TRAP IN GOLDEN, FAITHFULLY KEPT` 就是這類非標準措辭的既有先例）。
- `Command.cpp` 的 FW-CMD-D（:326 起）與 FW-CMD-F（:11332 起）兩段「解閘」新段逐段核對：
  兩段目前的 `GOLDEN xxx` 標記全部是既有的 B1~B8 系列（`GOLDEN BUG (B6)` 等），已在本檔
  既有的「Command.cpp（9 筆）」分節收錄過，本輪核對後**沒有新增項目**；唯一的新標記
  （`B-CMDC-1`/`B-CMDC-2`）落在 FW-CMD-C 段（`ChangeToAlarmSetup`/`_SG`），已併入上表。
- **`Command.cpp`／`uTemp_Set.cpp`／`DynamicTemp.cpp` 的 golden 來源澄清（重要，附教訓）**：
  這三個檔案頂端的「Golden source:」/「Golden ref:」banner（`Command.cpp`:6,:3769,:8282,
  :10330,:13062,:14682,:15197；`forms/fTemp_Set.h`:6；`forms/fDynamicTemp.h`:6）明確寫的都是
  `HT9011UC_Code_V3.33.906.0_20260618`，**不是**本台帳其餘多數分節預設對照的
  `HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422`（V899）。本輪驗證 `(B-CMDC-1)`/
  `(B-CMDC-2)` 時起初誤用 V899 樹核對 golden 行號，得出「port 引用的 :7910-7917／:8048-8307
  行號跟 V899 對不上」的假結論（V899 裡 `ChangeToAlarmSetup`/`_SG` 因中間程式碼差異整體位移
  約 -70 行，`ChangeToAlarmSetup_SG` 在 V899 是 :7973 而非 :8048）；改用正確的
  906_20260618 樹重新核對後，兩筆的 golden 行號引用**逐字核對完全正確**（:7910-7917 的
  `for(i=0;i<15;i++)` 清零迴圈、:8051-8052 的兩個 `new TStringList()`、函式全範圍
  :8048-8310 內查無 `delete` 皆核實無誤）。記錄此教訓：這三個 FW-3/FW-CMD 系列檔案要對照
  906_20260618 樹，不是 V899；下次維護此區的人若沒注意頂端 banner 的「Golden source」
  聲明就照本台帳「未特別指名檔案者預設是 V899」的通則去核對，會核對錯樹並得出假的
  行號不符結論。
- **排除的邊界案例**（判定為「非 golden 缺陷」，未收錄）：
  1. `uTemp_Set.cpp:4339` 的 `Key=NULL;`（`edSoakTimeKeyPress`）——純粹是「吃掉非法按鍵
     輸入」的常見 VCL 慣用寫法，該處逐行核對後**未附任何 `GOLDEN xxx` 字樣**，不符合本
     台帳收錄判準；任務提示詞舉例的「Key=NULL 型」在本檔查無實際標註案例。
  2. `uTemp_Set.cpp:7033-7034,7068-7069` 兩處「NOT gated -- golden's own body is ...
     marked "No Use" by golden itself」——這是翻譯者說明「為什麼這個函式沒被 GATE」的
     技術備註（引用 golden 原始碼裡的 `//No Use` 行內註解，屬機台既有的功能狀態標記，
     不是缺陷用語），全句未使用任何 `GOLDEN <缺陷詞>` 字樣，未收錄。
  3. `forms/fTemp_Set.h`／`uTemp_Set.cpp` 近 30 處 `DEVIATION`
     （`*MouseDown`/`FormClose`/`*KeyUp` 等事件處理常式的簽名精簡，拿掉未被讀取的
     `TMouseButton`/`TShiftState`/`TCloseAction` 參數）——這是港版自己的**簽名層級翻譯
     決策**說明，跟本台帳收錄的 `GOLDEN DIVERGENCE`（port 刻意偏離 golden *行為*，如
     `SCK_ART.cpp` 那兩筆）是不同概念；全數未使用 `GOLDEN` 字樣，未收錄。
  4. `MainTempMode.cpp`、`cSetUp.cpp` 全檔逐行核對（含上述擴充詞彙表與非標準措辭），
     **0 個命中**——`MainTempMode.cpp` 是新翻譯的 `TfMain::ChangeTempMode` facade 缺口
     收斂檔；`cSetUp.cpp`（cSetUp Wave B，是既有「forms/fSetup.h + fSetup.cpp」分節同一
     `TfSetup` 類別的延續翻譯波次、20260821 新落地檔案，非全新類別）本輪只翻了 47 個
     golden 方法中的 3 個，皆未帶 golden 缺陷標記。兩檔各自留下 stub 分節說明「查過，
     沒有」，而非略過不提。
- 本輪**未回頭複查** BUG / QUIRK / DEFECT / ASYMMETRY 等其他詞彙在這批檔案是否有類似的
  漏網之魚（除了上述 FW-CMD-D/F 的既有 B1~B8 順帶核對外）；範圍嚴格限定在任務指定的這
  6 個檔案。

### 補撈紀錄（依使用者/協調者指示，對全樹跑 `grep -rniE "GOLDEN [A-Z]+[ ,]"` 供詞彙缺口複查）

- 第一輪委派用的搜尋樣式集中在 `GOLDEN BUG/BUGS/ODDITY/ODDITIES/ASYMMETRY/ASYMMETRIES/DEFECT/DEFECTS/QUIRK/QUIRKS/INCONSISTENCY/NOTE`，SECSGEM 子代理事後自我複查發現此樣式漏掉了「`GOLDEN <名詞>, preserved`」這種名詞在前、`GOLDEN`後面直接接非上述詞彙的變體（`GOLDEN RACE`、`GOLDEN LEAK (a)/(b)`、`GOLDEN INVARIANT`），已自行補齊 4 筆並回報。
- orchestrator 據此對全樹重跑一次 `GOLDEN [A-Z]+` 廣義掃描，額外發現以下先前未被任一委派樣式涵蓋、且已確認為真實缺陷（非 GATE）的項目，已逐筆補進對應分節：
  - `CanBus/cMyNUDN1.h`：`GOLDEN GOTCHA` 1 筆
  - `Automation/uRENESAS_Server.h`、`Automation/uRENESAS_Other.h`：`GOLDEN GAP` 各 1 筆
  - `Automation/SCK_ART.cpp` + `csystem.cpp`：`GOLDEN DIVERGENCE` 2 筆（性質特殊，見掃描範圍聲明）
  - `ainarm9045.cpp`：`GOLDEN DEAD CODE PRESERVED` 1 筆
  - `ckernel.cpp`：`GOLDEN SPELLINGS KEPT`、`GOLDEN TYPO PRESERVED`、`GOLDEN DEAD CODE PRESERVED` 各 1 筆（原子代理 grep 樣式未涵蓋這三個詞）
  - `ainarm2.cpp`：`GOLDEN ODDITIES PRESERVED` 清單塊 3 筆（原子代理的 grep 命中點鄰近另一個函式，未讀到此處的獨立清單塊）
  - `aTester_Front.cpp`：`GOLDEN ASYMMETRIES vs the Rear twin` + `GOLDEN QUIRKS PRESERVED, NOT FIXED` 兩個清單塊共 14 筆（原因同上）
- **SECSGEM 三檔 QUIRK 補撈已完成並併入**：`SECSGEM/uHGemClass.cpp`（原0筆QUIRK→補入16筆，總計22筆）、`SECSGEM/uHGemEquipment.cpp`（原0筆QUIRK→補入21筆，總計31筆）、`SECSGEM/uHGemHT9045.cpp`（原0筆QUIRK→補入2筆，總計25筆），共補入39筆，已計入上方「總筆數552筆」。補撈子代理額外排除了 `uHGemHT9045.cpp:2947`（該處只是回引 `uHGemClass.cpp` 已收錄的同一缺陷，非獨立新項目）。
- **仍未經第二輪複查的已知殘留缺口（誠實揭露，未追殺到底）**：第一批委派的 6 個子代理（手臂核心、手臂周邊、主控/溫控、SECS/GEM、vclcompat 周邊、以及間接影響到的其他叢集）在最初次掃描時使用的 grep 樣式**不含 `QUIRK`/`DEFECT`/`ODDITIES`/`ASYMMETRIES`**（那是在後續才發現全樹通用的完整詞彙表），雖然多數子代理在閱讀上下文時仍自發抓到了鄰近的 QUIRK 項目，但無法保證窮盡。除了已於上一則補完的 SECSGEM 三檔外，以下檔案的 QUIRK 覆蓋度**同樣未經專門的第二輪複查**，僅在子代理原始研究時捎帶抓到多少算多少，留給下一次維護排入待辦：`ainarm2.cpp`（14 筆原始QUIRK命中，僅補了3筆ODDITIES）、`ainarm9045.cpp`（10筆QUIRK）、`aTester_Rear.cpp`（9筆QUIRK）、`aoutarm9045.cpp`（3筆QUIRK）、`cinitial.cpp`（3筆QUIRK）、`MyStringList.cpp`（4筆QUIRK，該檔已有BUG類完整覆蓋，QUIRK是否為額外項目未逐一複查）、`OCRInsp.cpp`（2筆QUIRK）。

## 已知大案快速索引（使用者指定務必收錄）

| 代號 | 檔案 | 一句話 |
|---|---|---|
| **B4** | `Command.cpp` | `GetSiteMappingByDLL` 算好全部 32 站的 `cSiteInfo[]`，卻只 `strcpy` 站 1 的內容給呼叫端，多站機台的 GPIB 站別回報永遠只有站 1。 |
| **B13** | `cShowBinSelect.cpp` / `forms/fShowBinSelect.h` | `ShowBinSel` 的灰階顯示判斷式用的 87 字元前導空白字串，在 golden 全樹裡從未被任何寫入來源產生過，灰階分支在 golden 自己就是死碼。 |
| **FW-Q3** | `uYieldMonitoring.cpp` | 三處 `(void)(bLowYieldCloseSite[...]==true)`，本該是賦值卻誤打成比較並丟棄，使良率差異自動關站的判斷邏輯全數失效。 |
| **B12** | `cShowBinSelect.cpp` / `forms/fShowBinSelect.h` | `bUnloadHasBin[Data]=true` 兩行後被無條件蓋回 `false`、中間沒有讀取，golden 自己的 `=true` 是死碼。 |

（完整內容見各自所在分節的表格。）

---

## 各 port 檔案分節

> 以下分節由 orchestrator（本代理）直接讀取原始碼驗證後寫入，未經子代理轉手。

### Command.cpp（12 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY（未編號） | ODDITY | Command.cpp:3765-3769 | golden :7295 | `GetCZAllMassTemp` 呼叫 `t.sprintf("UNKNOWN", sizeof(t))` 卻沒有任何 `%` 格式符可消耗這個多餘參數 | 純粹是多餘引數、無害，研判是複製貼上殘留 | 20260817 |
| 2 | GOLDEN BUG (B1)（FW3-WC group） | BUG | Command.cpp:8340-8344,8378,8404 | golden :8316,:8317,:8330,:8331 | `CreateAndOpenMap` 對 `INFO *CmdData` 做 `memset(CmdData,0x00,sizeof(CmdData))`，`sizeof(CmdData)` 量到的是指標大小而非 `INFO` 結構大小，只清了前綴 | `INFO` 結構其餘記憶體不會被歸零，可能殘留舊值 | 20260818 |
| 3 | GOLDEN BUG (B2)（FW3-WC group） | BUG | Command.cpp:8345-8352,8390 | golden :8327 | `CreateFileMapping(...,sizeof(CmdData),"HandlerMemory")` 用指標大小建立記憶體對映物件，但緊接的 `MapViewOfFile` 卻要求 `sizeof(INFO)` 大小的檢視 | 真正呼叫 `CreateAndOpenMap` 時屬於真實記憶體安全風險（對映不足），惟 ByDLL 呼叫者不在本波次測試範圍 | 20260818 |
| 4 | GOLDEN BUG (B3)（FW3-WC group） | BUG | Command.cpp:8353-8362 | golden :8437-8899（`cSiteInfo` 宣告於 golden :9003，本地陣列） | `GetSiteMappingByDLL` 的 `sprintf` 格式字串每次產生 11 bytes（10可印字元+NUL），但目的陣列 `cSiteInfo[32][10]` 每列只有 10 bytes，每次呼叫都溢位 1 byte | 真正的堆疊緩衝區溢位風險，一旦被實際呼叫 | 20260818（行號經整併時更正） |
| 5 | **GOLDEN BUG (B4)（FW3-WC group，使用者指定的已知大案）** | BUG | Command.cpp:8363-8369,9128-9130 | golden :8899-8901 | `GetSiteMappingByDLL` 算好全部 `cSiteInfo[0..31]`（每站一筆格式化字串），但函式唯一的輸出寫入是 `strcpy(cSiteMap, cSiteInfo[0])`——只有站 1 的內容被複製，`cSiteInfo[1..31]` 算完即丟棄 | 每個呼叫此函式的呼叫端（如下方 `WriteSiteOnOff`）在多站機台上透過 GPIB 回報站別時永遠只回報站 1 | 20260818 |
| 6 | GOLDEN ODDITY (B5)（FW3-WD group） | ODDITY | Command.cpp:10617-10632 | golden :10958-10959,:11205 | `ChkStatus`/`SetStartMode` 用三個未初始化的裸 `char[10]` 陣列，只被沒有 `else` 兜底的 if/else-if 鏈賦值；`SetStartMode` 的 `else` 分支呼叫訊息後未 `return` 直接 fall-through 到同樣的未初始化讀取 | 若 `iRunStartMode`/`Caption` 落在判斷鏈之外，後續 `sprintf` 會讀到未初始化記憶體；重現 golden 既有的未定義行為類別，非本次翻譯新增 | 約20260818 |
| 7 | GOLDEN BUG (B6)（FW3-WD group） | BUG | Command.cpp:10633-10645,12808,12847 | golden :12024,:12051 | `SetSGCONTFAIL` 對 Head 版 `rbContsFailByHead_FTOff`/`_RTOff` 的 `Checked` 賦值少了顯示用的 `!` 負號，Socket 版兩行以上正確加了負號，屬 FT+RT 一致的複製貼上疏漏 | 一旦 `fYieldMonitoring` 群組解除 GATE 上線，Head Off checkbox 永遠不會正確顯示「已關閉」狀態 | 約20260818 |
| 8 | GOLDEN BUG (B7)（FW3-WE group） | BUG | Command.cpp:13056-13065,13364 | golden :12625-12626 | `HanderTcpIp` 關閉 `TCPCommandServer` 前的保護判斷式第二行 `TCPCommandServer->Active==false;` 把賦值誤打成比較，整段判斷變成無作用的空比較 | golden 自己「先關閉閒置伺服器」的保護機制形同虛設；本 port 因 `TCPCommandServer` 本身也不存在而雙重無效，保留註記避免未來實作真正的該物件時繼承此 typo；**AI(W906-ST02-W17) 20261003（St02-E）更正 W10 0927 的「撤回」**：golden 0618 Command.cpp:12626 確實是 `==`（1003 讀 0618 確認；V899 :12555 也是），W10 當時拿 0625 比錯了；0625_Steven :12626 與 V912 :12634 是賦值，port 照 RULINGS_20261003 第 1 條保留賦值（修正），兩邊註明在 port Command.cpp:13512 與橫幅 :13199-13206 | 約20260818 |
| 9 | GOLDEN BUG (B8)（FW3-WE group） | BUG | Command.cpp:13066-13083,14039 | golden :14778-14781 | `WriteHeadContactCount` 的例外處理區塊誤重置了另一個全域變數 `asChangeSetupFileName`（`cmydef.h:5729`，原本追蹤「GPIB剛送的setup檔名」），研判是從其他 setup 檔案相關函式複製貼上未改 | 若 `WriteHeadContactCount` 解析例外恰好在其他 setup-file 變更函式與其自身下一次讀取之間發生，會靜默清空該追蹤值 | 約20260818 |
| 10 | GOLDEN ODDITY (B-CMDC-1)（FW-CMD-C group） | ODDITY | Command.cpp:15421-15428（banner）,15839-15845（inline，`ChangeToAlarmSetup` 函式內） | golden :7867-7869（`sBinName`/`sPassOrFail`/`fYield` 宣告 `[TEST_MAX_BIN]`）,:7910-7917（Bin Alarm Yield 清零迴圈寫死 `for(i=0;i<15;i++)`） | `ChangeToAlarmSetup` 的清零迴圈硬編 `<15`，不是 `<TEST_MAX_BIN`(256) 也不是 `<iTestBinCount`，但緊接著的兩個迴圈都跑到 `iTestBinCount` | `iTestBinCount>15` 的機型（16/32-site）第 15 格之後的槽位從未被這段清零就被下面迴圈讀取；此函式本身翻譯完整（ACTIVE、無內部 gate），但目前唯一呼叫端 `SetAlarmSetup`（Command.cpp:14925）仍被 `GATE(FW3-WF)` 整段 `#if 0`，缺口暫不可達 | AI(W906-FW-CMD-C) 20260820（併入台帳 W906-LEDGER-R2 20260821） |
| 11 | GOLDEN ODDITY (B-CMDC-2)（FW-CMD-C group） | ODDITY | Command.cpp:15429-15433（banner）,15989-15992（inline，`ChangeToAlarmSetup_SG` 函式內） | golden :8051-8052（`new TStringList()` x2）；函式全範圍 :8048-8310（3 個 return 路徑皆查無 `delete`） | `ChangeToAlarmSetup_SG` 每次呼叫都 `new` 兩個 `TStringList`（`tSetup`/`tCondition`），全函式 3 個 return 路徑皆無對應 `delete`，golden 自己就帶這個 per-call heap leak | 函式本身翻譯完整、無內部 gate，但與 (B-CMDC-1) 同一個呼叫端 `SetAlarmSetup` 仍被 gate 擋住；一旦解 gate，SIGURD 分支每次收到 Site-Map alarm setup 指令都會洩漏兩個 `TStringList` 物件 | AI(W906-FW-CMD-C) 20260820（併入台帳 W906-LEDGER-R2 20260821） |
| 12 | GOLDEN ASYMMETRY (GATE7-V) | ASYMMETRY | Command.cpp:11427-11433（inline，`WriteSetSoakTimeStatus_SIGURD` 函式內，寫入點 :11454/:11466） | golden Command.cpp:10640-10642；對照 golden uTemp_Set.cpp:1986-1989（ReadTempFile）、:4214（spbSaveClick）、:4540（SaveSetupFile） | 遠端 SETSOAK_ 命令的 Temperature.Data 寫入無條件以 DataPath+recipe 組路徑，但該檔的讀取與表單自家兩條存檔路都帶 bSaveTemperatureByMachine 變體 | 開啟 by-machine 的機台上，遠端命令寫的 Temperature.Data 落在沒人讀的路徑——模式變更只活在 ChangeTempMode 的記憶體狀態，重開機後讀回 sSaveByMachine 的舊值 | AI(W906-GATE7V) 20260824 |

### cShowBinSelect.cpp / forms/fShowBinSelect.h（3 筆，取自 `git show HEAD:` 已提交內容，見上方掃描範圍聲明）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | **GOLDEN BUG (B12)（使用者指定的已知大案）** | BUG | forms/fShowBinSelect.h:253-271；cShowBinSelect.cpp:1088,1236,1243,1292 | golden :541-545 | `ShowBinSel` 對 non-BulkBox 錯誤盤重新導向的分支，先把 `bUnloadHasBin[Data]` 設 `true`（golden :543），兩行後又無條件蓋回 `false`（golden :545），中間完全沒有讀取，使前面的 `=true` 變成死碼 | 該分支下 `bUnloadHasBin[Data]` 恆為 `false`；忠實保留不修，因為改成 `=true` 會改變下游讀者（若存在）觀察到的行為，需使用者裁決 | 整併 20260819 |
| 2 | GOLDEN NOTE（未編號，附於 B12 說明中） | NOTE | cShowBinSelect.cpp:1291-1298 | golden :602-609 | `ShowBinSel` 的 Magazine-Link 內層迴圈在 `bMagazineLink[]` 全鏈為 `true` 的病態情況下會以負數索引存取 `bMagazineLink[i-j]`，形成陣列越界讀取 | 因索引 0（`eAuto1`）從未被任何 port 寫入者設為 `true`，實務上無法觸發，屬理論風險 | 整併 20260819 |
| 3 | **GOLDEN ODDITY (B13)（使用者指定的已知大案）** | ODDITY | cShowBinSelect.cpp:1322-1328 | golden :628,:2543,:2762（Pos()檢查）vs :561（真正的dots寫入器） | 判斷是否要把 bin 顯示灰階的 `Pos()` 檢查所用的 87 字元前導空格字串，實際上從未出現在任何寫入來源（dots 寫入器用的是另一個 80 字元前導點字串），`j==1` 這個條件在 golden 自身就永遠不成立，灰階分支是死碼 | 對所有 non-tNotUse 的 tray，永遠走彩色顯示分支，灰階顯示邏輯形同虛設但無害；忠實翻譯保留，未簡化刪除 | 整併 20260819 |

### cShowBinSelect.cpp / forms/fShowBinSelect.h WAVE D/E 補充（4 筆，承續上表編號 4-7；取自目前 working tree 20260820 落地的 WAVE D/E，見上方掃描範圍聲明——這兩個 Wave 是三輪增補時新落地的內容，不在原始 `git show HEAD:c590952` 快照範圍內）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 4 | GOLDEN ODDITY（WAVE D，未編號） | ODDITY | cShowBinSelect.cpp:2253；forms/fShowBinSelect.h:393-400 | golden :272 | `ChangeBinDispStatus` 對 `PageControl1->ActivePageIndex==3` 用 `==` 而非 `=`，是一個沒有任何效果的比較敘述句（golden 幾乎可以肯定是想寫 `=`） | 純粹是無效的比較敘述，對顯示行為無影響；忠實翻譯成 `(void)(...)` 保留這個 no-op，未「修正」成 `=` | AI(W906-FW3-SBS-WD) 20260820；⛔ 20261003 更新：St02 C14（`8db5c2c9`）已改成 `=3`（0625 :272／V912 :295 也是賦值），E-034（`effe255a`，Steven Q82＝A）加了 `E034_NoopEq` 釘住，屬 #20 例外（Steven 1003 常設規則） |
| 5 | GOLDEN NOTE（同 B12 說明中 NOTE 之第二現場，未獨立編號） | NOTE | cShowBinSelect.cpp:2536-2544 | golden `ChangeBinDispStatus` :207-386（Magazine-Link 迴圈） | `ChangeBinDispStatus` 自己的 Magazine-Link 內層迴圈與 `ShowBinSel`（既有 NOTE，cShowBinSelect.cpp:1291-1298）同樣手法，在 `bMagazineLink[]` 全鏈為 `true` 的病態情況下會以負數索引存取 `bMagazineLink[i-j]` | 因索引 0（`eAuto1`）從未被任何 port 寫入者設為 `true`，實務上無法觸發，屬理論風險；與既有 NOTE 同一 golden 手法在第二個函式重現 | AI(W906-FW3-SBS-WD) 20260820 |
| 6 | GOLDEN NOTE（同上手法之第三現場，i=0 變體，未獨立編號） | NOTE | cShowBinSelect.cpp:2842-2856；forms/fShowBinSelect.h:454-455 | golden `ShowBinDigital` :880-996（Magazine-Link 迴圈） | `ShowBinDigital` 自己的 Magazine-Link 迴圈外圈從 `i=0` 起算（不像手足迴圈是 `i=1`），理論上 `i==0` 時會索引到 `bMagazineLink[-1]`；但因外層 `if` 需要 `bMagazineLink[0]==true` 才會進入、而該欄位從未被任何 port 寫入者設為 `true`，實務行為與手足迴圈相同 | 同上，理論風險、實務不可觸發；忠實保留，未「對稱化」成從 i=1 起算 | AI(W906-FW3-SBS-WE) 20260820 |
| 7 | GOLDEN ODDITY（WAVE E，未編號） | ODDITY | cShowBinSelect.cpp:2866-2874；forms/fShowBinSelect.h:456-457 | golden `ShowBinDigital`（同函式，緊鄰上一列的迴圈 else 分支） | `MyBinSel[eMag1+i]->Caption=MyBinSel[eMag1+i]->Caption;` 是一個自我賦值（a=a）的無效敘述，讀起來像是刻意表達「未連動則維持原 Caption」的 else 分支，而非單純打字錯誤 | 純粹無效敘述，不影響顯示行為；連 golden 原本多出來的一個前導空格都逐字保留，未清理 | AI(W906-FW3-SBS-WE) 20260820 |

### Automation/SCK_ART.cpp + csystem.cpp（GOLDEN DIVERGENCE，2 筆，⚠️性質特殊，見掃描範圍聲明）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DIVERGENCE D1 | DIVERGENCE（port偏離golden，已揭露） | Automation/SCK_ART.cpp:38-166；csystem.cpp:4992-5010,6284-6297 | golden SCK_ART.cpp:43-49 | golden 建構子依 `CUSTOMER_CODE` 分支明確給出 `iLOTSTATUS_L/_W/_R/_A` 的真實狀態碼常數（3/1/4/6），但 `SckArtState`/`W7C1_TfSCKARTSeam`/`W7C2_TfSCKARTSeam` 三處建構子全部改成 0 初始化 | 若 `CosFunction.bUseSCKART` 等 ART 旗標未來被開啟，`SckArt_SetLotStatus` 等函式會走錯 `default:` 分支記錄成 "NONE"；目前因旗標預設 false 而不可達 | AI(W906-W7-F2) 20260729，經 F2fix/F2fix2/F2fix3 三輪修正說明 20260730-20260801 |
| 2 | GOLDEN DIVERGENCE D2 | DIVERGENCE（port偏離golden，已揭露） | Automation/SCK_ART.cpp:41-157；csystem.cpp:4992-5010,6284-6297 | golden SCK_ART.cpp:42,:102-114 | golden 對非 `CC_SCK` 客戶碼一律把 `iTesterType` 設為 1，port 的三處建構子全部固定給 0 | csystem.cpp 六處讀取點（`W7C1`一處、`W7C2`五處）的比較方向會與 golden 完全相反（該進的 ART 分支被跳過、該跳過的反而被進入），屬行為性差異；目前不可達，測試已釘住 0 值以確保未來修正是刻意決定 | 同上 |

### ainarm9045.cpp 補充（2 筆，Agent1 原始 grep 樣式未涵蓋，由 orchestrator 補讀）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEAD CODE PRESERVED, NOT DELETED | ODDITY | ainarm9045.cpp:10675-10680（函式 `bool DoAutoSkipCheck()`，定義於:10697） | golden :7368-7371 | golden 在此處留了一段整段註解掉的 `else if(IniConfig.bP17InArmFullPickFromLoader==false \&\& InArmSuck.HasIC())` 死碼，port 逐字轉錄成註解文字保留，因為它的存在正是讓後面那個 else if 屬於 ASE 分支 else-if 鏈、而非重新起始 if 判斷的關鍵語法脈絡 | 若移除這段死碼註解，會讓後續程式碼讀起來像是被重新架構過，故刻意保留純文字形式的死碼作為結構脈絡標記，對執行行為無影響 | 未標明日期（906整合期既有註解） |
| 2 | ONE STRUCTURAL TRAP IN GOLDEN, FAITHFULLY KEPT（未用標準 GOLDEN-BUG 詞彙，但明確聲明「golden behaviour and is NOT corrected」，比照收錄） | ODDITY | ainarm9045.cpp:10681-10687（同一函式） | golden（`DoAutoSkipCheck` 的 if/else-if 鏈起始判斷） | `DoAutoSkipCheck` 的 FIFO 判斷臂與 clean-out 判斷臂是 `if(CUSTOMER_CODE==CC_ASE_KaohSiung \&\& ArmSpeed[InArm].bAutoSKIP==true)` 的 else-if 延伸，在 ASE-高雄機台且 bAutoSKIP 開啟時，這兩個分支永遠不可能從這裡回傳 true | 標準 build 會強制把 `CUSTOMER_CODE` 改離 `CC_ASE_KaohSiung`，因此第一個分支在標準機台上是死碼、判斷鏈實際從 FIFO 分支開始生效；此為 golden 既有設計，不予修正 | 未標明日期 |

### CanBus/cMyNUDN1.h（1 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN GOTCHA（未編號） | GOTCHA | CanBus/cMyNUDN1.h:26-44 | golden cMyNUDN1.cpp 建構子 | golden 建構子初始化列 `: TComponent(Owner)` 所引用的 `Owner` 既非參數也非成員，實際上是編譯器在基底類別初始化列中把未限定名稱解析成「正在建構中的類別」自身尚未初始化的繼承成員（`this->Owner`），是教科書等級的未初始化讀取當作建構參數使用的未定義行為 | 本 port 因拿掉了 `TComponent` 基底（此檔翻譯決定）使問題對本檔不再適用，但姊妹檔 `cMyDNM100UD.cpp` 尚未翻譯且有相同寫法，未來翻譯該檔時必須做出相同處理 | AI(W5-CanBus-Translate) 20260710 |

### Automation/uRENESAS_Server.h（1 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | FAITHFUL GOLDEN GAP（未編號） | GAP | Automation/uRENESAS_Server.h:41-45,420-424 | golden uRENESAS_Server.cpp :2540-2572（整段被註解掉） | golden 標頭宣告了 `RENESAS_LoadWorkSetup`，但其函式本體在 golden .cpp 中整段被註解掉且從未被呼叫，port 選擇忠實地連宣告都不做（而非生出一個假 body） | 屬於死宣告，不影響行為；若未來有人誤以為此函式該有實作，本記錄可說明其在 golden 中本就是死碼 | uRENESAS 家族翻譯波次（無明確 AI 標籤日期） |

### Automation/uRENESAS_Other.h（1 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | FAITHFUL GOLDEN GAP（未編號） | GAP | Automation/uRENESAS_Other.h:42-50,324-329 | golden :218-227 | `TRENESAS_EndFileData::InitData()` 只清空最早期就存在的 8 個欄位，後續兩批新增的 5 個欄位（`HdPickUp` 與 "TwoPassBin" 四件組）因宣告在 `InitData()` 之後，未被納入清零範圍 | 這 5 個較新欄位不會被 `InitData()` 重置，其初值狀態需另外檢視；忠實保留不修，避免默默改變行為 | AI(W5-RENESAS_Other-Translate) 20260710 |

## 子代理彙整叢集（延續上方 orchestrator 直接發現的分節）

以下 11 個叢集由平行派工的子代理逐檔掃描、經 orchestrator 重新核對格式與已知案例後併入。叢集內各檔案已依筆數大致降冪排列；叢集彼此之間依各自最大宗檔案的筆數排列（如手臂核心叢集因含 ainarm9045.cpp 41 筆而排最前）。跨叢集的精確全域降冪排序在檔案數達到 ~90 個的規模下維護成本過高，故採此「叢集內精排、叢集間概排」的折衷 —— 若要查特定檔案，建議直接用文字搜尋檔名，不需依賴分節順序。

### 【手臂核心叢集】涵蓋：ainarm2.cpp / ainarm9045.cpp / aoutarm9045.cpp / atester_ProcessCount.cpp / atester_shims.h / aTester_Front.cpp / aTester_Rear.cpp

### ainarm2.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG KEPT | BUG | ainarm2.cpp:447 | golden :976-991 | ChangeHotPlateData(bool bSwapSht) 的參數 bSwapSht 在 golden 本體中從未被讀取，port 忠實保留這個從未使用的參數。 | 呼叫端傳入的 bSwapSht 值永遠不影響行為，未來若誤以為調整它能改變 Shuttle/Kit 切換邏輯會白費工。 | 20260811 |
| 2 | GOLDEN BUG -- NOT FIXED HERE, RECORDED AS REQUIRED | BUG | ainarm2.cpp:1152-1153 | golden :2620；AutoTeach/AutoTeach.cpp:1972 | golden 對 bMoveInArmXYToTeach(bool,int,int) 同時在 ainarm2.cpp:2620 與 AutoTeach/AutoTeach.cpp:1972 定義兩個內容不同、皆非 static 的版本，實際生效的一份由連結順序決定。 | AutoTeach.cpp 一旦也被翻譯進同一連結單元會立即出現 multiple definition 連結錯誤，屆時採用哪一份行為需人工決策。 | 20260811 |
| 3 | GOLDEN BUG PRESERVED (not fixed here) | BUG | ainarm2.cpp:1365 | golden :2740-2741 | bOffsetClean 清除迴圈上限只到 iSelPartData<=13，但 32-site 分支判斷的 OfsInSh1LB(27)/OfsOutSh1LB(44) 永遠落在範圍外，兩段清除是死碼。 | 32-site 機型按下「清除 Offset」時，Loader 與 Out-Shuttle 的偏移值不會真的歸零，操作者可能誤以為教導值已重置。 | 20260811 |
| 4 | GOLDEN BUGS (kept verbatim) #1 | BUG | ainarm2.cpp:1588 | golden :2995,:3028 | SetPrecisorZPos 非 one-kit 分支誤用 InArmSuck.Item[j][j] 而非 [i][j]，MAX_ARM_Col=4 下 j=2,3 讀到列邊界外的垃圾值。 | 全格 Precise 動作判斷是否排除 HAS_NULL_IC 時可能誤判整批 nozzle。 | 20260811 |
| 5 | GOLDEN BUGS (kept verbatim) #2 | ASYMMETRY | ainarm2.cpp:1588 | golden :3017,:3031 | iMoveDownAdd200 的 one-kit 分支對 iInArmPreciserPickUpZ 加 200，整格分支卻對 iInArmPreciserPlaceZ 加 200，golden 本身標註 Asymmetric。 | one-kit 與整格模式下 Precisor 的 Z 目標高度基準不一致，可能造成下壓深度不對稱。 | 20260811 |
| 6 | GOLDEN BUGS (kept verbatim) #3 | BUG | ainarm2.cpp:1588 | golden :3037 | 註解寫 if(iAction==iInitialFlag) 但實際是裸 else，任何未列舉的 iAction 值都會落入歸零分支。 | 未來新增 iAction 列舉值若忘記處理，會被靜默導向初始化分支而非報錯。 | 20260811 |
| 7 | GOLDEN BUGS FOUND (a) | BUG | ainarm2.cpp:1742 | golden :3320,:3351-3352 | case 1310 的 flag=true 從未被設回 false，使 if(flag==false) break; 恆為假，該 case 無法等待較慢的吸取完成。 | 吸取尚未完成時 SM 仍提前判斷完成，可能在 IC 未確實吸住前進入後續步驟，增加掉料風險。 | 20260811 |
| 8 | GOLDEN BUGS FOUND (b) | BUG | ainarm2.cpp:1742 | golden :3337-3340,:3361 | case 1310 把 Suck[i][j].Error==true 也當成「此 nozzle 已完成」，要到之後 Error 掃描才轉去 case 1400 處理。 | 真空異常先被誤判為正常完成一拍，才在下一階段被導正，延遲了錯誤處理的反應時間。 | 20260811 |
| 9 | GOLDEN BUGS FOUND (c) | BUG | ainarm2.cpp:1742 | golden :3216 | case 1000 故意沒有 break，同一 tick 內直接落入 case 1050 繼續執行。 | 是刻意的 fall-through 設計，若有人依直覺補上 break 會改變原本一拍內完成兩步的時序。 | 20260811 |
| 10 | GOLDEN BUGS FOUND (d) | BUG | ainarm2.cpp:1742 | golden :3212,:3312 | bDestroyflag[i][j]=!bPrecise_Z[i][j] 走訪 iMaxRow/iMaxCol，但 bDestroyflag 初始化只依 MAX_ARM_Row x MAX_ARM_Col，兩邊界各自獨立。 | 未來若兩組邊界的關係改變，可能出現陣列邊界不一致。 | 20260811 |
| 11 | GOLDEN BUGS FOUND (e) | BUG | ainarm2.cpp:1742 | golden :3368 | if(Task!=1400) 緊接在可能剛把 Task 設成 1400 的迴圈之後讀取，使迴圈最後一次寫入直接決定判斷結果。 | 屬刻意設計（KevinCheng 註解稱跳到 case1500），但寫法易誤解，維護時可能被誤改而破壞既有時序。 | 20260811 |
| 12 | GOLDEN BUGS FOUND (f) | BUG | ainarm2.cpp:1742 | golden :3184 | 手動 Offset 停止只被 AccessLevel>=1 把關，但 Sen[SnRKManualTStart] 的感測器讀取本身每個 400 tick 都會執行，不受權限限制。 | 低權限使用者雖不能觸發手動 Offset 停止流程，相關感測器輪詢仍持續進行，屬冗餘讀取。 | 20260811 |
| 13 | GOLDEN BUG (kept) | BUG | ainarm2.cpp:5327 | golden :1554-1564 | RowCanDualSite 四出口三個回傳 false，唯一可能 true 的路徑是 ArmCanSuck4IC(0)==false，XDivision==6 判斷之後緊接 return false，形成死碼。 | XDivision==6 條件邏輯上不起作用；退休舊 stub（恆 false）後行為與舊 stub 有實質差異，須留意。 | 20260811 |
| 14 | GOLDEN BUGS PRESERVED #1 | BUG | ainarm2.cpp:5844 | golden :1967-1968 | TransferHotPlateRatio 的 Y 方向超調保護測試 Y 超標，但超標時卻還原 *iXPos=iOldX，把 X 值復原而非真正超標的 Y 值。 | Y 軸真的超出保護範圍時程式不會修正 Y，反而錯誤地把 X 座標拉回舊值。 | 20260811 |
| 15 | GOLDEN BUGS PRESERVED #2 | BUG | ainarm2.cpp:5844 | golden :1875 vs :1856,:1941,:1960 | Plate2 非三溫機分支中 Y 膨脹係數乘法放在最內層 else，走 setup 檔或熱模式 Y scale 分支就不套用；X 軸與 Plate1 兩軸則無條件套用。 | 特定溫度/scale 組合下 HotPlate 的 Y 座標熱膨脹補償可能被跳過而 X 軸仍有補償，造成校正不對稱。 | 20260811 |
| 16 | GOLDEN BUGS (kept verbatim, see report) | BUG | ainarm2.cpp:6516 | golden :2390-2392 | DoShakeShuttle 附近除錯字串 str3 被 sprintf 兩次，第一次（短格式）從未被讀取就被第二次覆蓋。 | 純粹浪費一次組字串成本，不影響機構動作，但可能讓人誤判該除錯輸出有兩種格式在用。 | 20260811 |
| 17 | GOLDEN BUGS (kept verbatim, see report) | BUG | ainarm2.cpp:6516 | golden :2390-2392 | 長格式除錯字串遺漏 fCanMove 的轉換字元，8 個轉換對應 9 個參數，「fCanMove:」後面印出的其實是 fCanMoveM 的值。 | 排查 Shuttle 搖動/敲擊問題時，依此 log 判斷 fCanMove 狀態會被誤導。 | 20260811 |
| 18 | GOLDEN BUGS (kept verbatim, see report) | BUG | ainarm2.cpp:6516 | golden :2391 | 同一行除錯字串裡 bShuttleShake 的值被印了兩次。 | 除錯輸出重複顯示同一變數，不影響控制邏輯，但可能讓人誤以為有兩個不同旗標值。 | 20260811 |
| 19 | GOLDEN BUGS (kept verbatim, see report) | BUG | ainarm2.cpp:6516 | golden :2394 | ShowMyMessage(str1, str2) 呼叫時 str2 當下仍是空字串，等同只傳了一段有意義內容。 | 訊息框原本設計要顯示兩段內容，但 str2 尚未賦值就被使用，使用者只會看到一半資訊。 | 20260811 |
| 20 | GOLDEN BUG, PRESERVED | BUG | ainarm2.cpp:6740 | golden (DoKnockShuttle, essSetFlag/essEnd) | DoKnockShuttle 中 essSetFlag 把 bShuttleShake 設 true 但從不清除，essEnd 清除 bShuttleKnock 卻從未設定它，兩旗標設定/清除交叉錯置。 | 兩旗標生命週期分屬不同 SM 管理，交叉錯置可能讓其中一個旗標長期停留在非預期狀態。 | 20260811 |
| 21 | SECOND GOLDEN QUIRK | QUIRK | ainarm2.cpp:6740 | golden (DoKnockShuttle essSetFlag case) | essSetFlag 分支寫成內層再包一層 break，內層 break 多餘，兩條路徑最終都離開同一 switch。 | 純粹程式碼冗餘，無行為差異，僅可能造成閱讀時對控制流程的誤解。 | 20260811 |
| 22 | GOLDEN NOTE | NOTE | ainarm2.cpp:7646 | golden :3631-3658 | RecordShtSuperflous 中 shuttle-1 字串用「Un-superfluous」（有連字號），shuttle-2 字串用「Unsuperfluous」（無連字號），拼法不一致。 | 純文字紀錄層面不一致，若下游 MES 記錄比對字串格式時可能對不上。 | 20260811 |
| 23 | GOLDEN BUGS (kept verbatim) #1 | BUG | ainarm2.cpp:7888 | golden :3918,:3923 | SetShuttlefCanMoveL 的 if/else 兩分支各組了一次幾乎相同的 Str 字串，等於同樣的 sprintf 重複寫兩遍。 | 屬效能與可讀性冗餘，不影響 fCanMoveL 旗標本身的正確性。 | 20260811 |
| 24 | GOLDEN BUGS (kept verbatim) #2 | BUG | ainarm2.cpp:7888 | golden :3940 | 除錯檔名格式字串只有 5 個轉換卻給 8 個參數，最後一欄被 SystemYear 而非 SystemHour 填入，「每小時一檔」從未真正發生；位於恆假巨集內，目前是死碼。 | 一旦有人開啟該除錯巨集，會發現每小時的 log 檔名其實都指向同一個檔案。 | 20260811 |

TOTAL for ainarm2.cpp: 24


### ainarm2.cpp 補充（承續上表編號，續 25-27；來源：orchestrator 直接補讀 `GOLDEN ODDITIES PRESERVED` 清單塊，:2676-2684）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 25 | GOLDEN ODDITIES PRESERVED 項1 | ODDITY | ainarm2.cpp:2677 | golden :3692-3694 | `RestoreLoadeIC` 附近有一段完全空白的 `else { }`，逐位元組保留 | 純視覺留白，不影響行為 | 20260811 |
| 26 | GOLDEN ODDITIES PRESERVED 項2 | ODDITY | ainarm2.cpp:2678-2681 | golden（同函式） | 兩個方向分支刻意掃描不同的列範圍：Direction 0/1 用 `j<YItem-1`，Direction 2/3 用 `j=1...YItem`，各自排除剛重試完的那一列 | 此不對稱是刻意設計的特性，不可「對稱化」，否則會重複處理已重試過的列 | 20260811 |
| 27 | GOLDEN ODDITIES PRESERVED 項3 | ODDITY | ainarm2.cpp:2682-2684 | golden ainarm2.h:220 | golden 標頭宣告 `RestoreLoadeIC(int iLoaderY=0)` 帶預設參數，但定義本身不帶預設值，港版比照保留（預設值只該出現在宣告） | 純撰寫慣例差異，不影響行為 | 20260811 |

### ainarm2.cpp QUIRK 二輪補掃（承續上表編號，續 28-53；來源：orchestrator 對本檔全樹重跑 `grep -inw quirk`，補齊第一輪 grep 樣式未涵蓋的 14 個 QUIRK banner）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 28 | FIDELITY NOTES (golden quirks kept) 項1 | QUIRK | ainarm2.cpp:613（`MoveInArmZToPlateSafeAndCheckLoaderTray`） | golden :1140-1184 | `int ret=0;` 在每一條不會走到 JAM 的路徑上都是死值 | 純冗餘賦值，不影響行為 | 20260811 |
| 29 | FIDELITY NOTES (golden quirks kept) 項2 | QUIRK | ainarm2.cpp:614-618 | golden :1162,:1168 | `bfirstIn=true` 在 SKIP 路徑被寫兩次（:1162 在警報之前、:1168 在 SKIP 分支內），且因 :1162 在警報之前就設 true，RETRY 的回答會讓 3 振次計數從零重新起算而非接續累計 | 呼叫端依賴這個重新起算的行為，忠實保留不改 | 20260811 |
| 30 | FIDELITY NOTES (golden quirks kept) 項3 | QUIRK | ainarm2.cpp:619-620 | golden（同函式） | `LastSet.iRealDummy>=HAS_TRAY` 對 dummy-mode enum 用大於等於比較而非相等比較 | 逐字重現，不影響目前唯一使用的列舉值 | 20260811 |
| 31 | GOLDEN QUIRK PRESERVED（未編號） | QUIRK | ainarm2.cpp:789-793（`MoveInArm2XYToLoaderWait`） | golden :1396-1399 | RogerYang 20250820 把 `if(...) return false; else` 註解掉，只留下仍保持 else 縮排的 ShowMyMessage，該行現在無條件執行 | 該分支原本應被跳過的 ShowMyMessage 呼叫現在每次都會執行 | 20260811 |
| 32 | GOLDEN QUIRKS PRESERVED 項1 | QUIRK | ainarm2.cpp:1019-1024（`TransferLoaderRatio`） | golden :2161 | 於 `bInArmUseDifferentScaleBySetupFile` 分支內重新宣告 `double fi`，遮蔽 :2121 的外層 `fi` | 預期會觸發本專案 `-Wshadow` 警告；該警告屬 golden 自身，非本次翻譯引入的缺陷 | 20260811 |
| 33 | GOLDEN QUIRKS PRESERVED 項2 | QUIRK | ainarm2.cpp:1025-1027 | golden :2124 | `&&LastSet.iTemperature==Tempture_Hot`（& 前無空格）重複測試外層 if 已經測過的同一條件 | 純冗餘判斷，不改變結果 | 20260811 |
| 34 | GOLDEN QUIRKS PRESERVED 項3 | QUIRK | ainarm2.cpp:1028-1030 | golden :2122,:2159,:2172 | Hot 溫度分支 (:2122) 吞掉整段三溫機判斷，使 setup 檔 scale(:2159) 與一般 scale(:2172) 在 `iTemperature==Tempture_Hot` 時永遠不可達 | Hot 模式下這兩種 Loader scale 永遠不會被套用，維持 golden 既有行為 | 20260811 |
| 35 | GOLDEN QUIRKS PRESERVED 項4 | QUIRK | ainarm2.cpp:1031-1034 | golden :2121,同函式 | 外層 `double fi`(:2121) 宣告時未初始化但每條讀取路徑都先賦值；`*iXPos=`/`*iYPos=` 的 double 轉 int 窄化截斷是承重行為，函式內無 int/int 除法 | 兩者皆為安全的既有寫法，忠實保留不「清理」 | 20260811 |
| 36 | GOLDEN QUIRK（未編號） | QUIRK | ainarm2.cpp:2384-2389（`PreciserPitchCalculate`） | golden :3441-3444 | `else if(TestIF.iTestMode==DualSite2x1)` 分支是空的 void 函式內只剩註解掉的 `// return false;`，唯一作用是擋住 DualSite2x1 落入下面的 DualSite 分支 | `iXPitch` 因此維持 :3427 賦的一般值，含被註解的 return 都逐字保留 | 20260811 |
| 37 | GOLDEN QUIRKS PRESERVED 項1 | QUIRK | ainarm2.cpp:2475-2479（`DoVibrateOutShuttle`） | golden :3551,:3555 | `int(IniConfig.iF25VibrateTime/10.0)` 是刻意的浮點除以 10.0 後才截斷成 int，非整數除法 | 逐字保留，不因既有的 float-helper 替換回歸而改寫 | 20260811 |
| 38 | GOLDEN QUIRKS PRESERVED 項2 | QUIRK | ainarm2.cpp:2480-2481 | golden :3568-3573 | case 150 是死碼，golden 全樹沒有任何地方把 150 賦給 `iVibrateOutShuttleTask` | 逐字保留含 break，不刪除不可達分支 | 20260811 |
| 39 | GOLDEN QUIRKS PRESERVED 項3 | QUIRK | ainarm2.cpp:2482-2484 | golden :3539,:3558,:3577 | `iShuttleVibraCount` 會被遞增與歸零，但從未被讀取 | 純死狀態保存，含其函式局部 static 儲存方式一併保留 | 20260811 |
| 40 | GOLDEN QUIRKS PRESERVED 項4 | QUIRK | ainarm2.cpp:2485-2489 | golden :3537 | `AnsiString str;` 宣告後從未被使用 | 因 AnsiString 建構子非平凡，-Wall -Wextra 下不會產生 unused-variable 警告，純冗餘 | 20260811 |
| 41 | GOLDEN QUIRKS PRESERVED（未編號） | QUIRK | ainarm2.cpp:2784-2789（`VacuumOnOffLog`） | golden :3727-3744(:3739-3740) | `s.sprintf("%s", sLog)` 是無意義的字串複製；`Path` 被格式化兩次且第一次的值只用於 `MyForceDirectories`；CSV 表頭字串「Vacuum On」前的空白不一致；if 區塊本體用 3 個空白縮排而非 4 個 | 皆為既有撰寫瑕疵，不影響輸出資料正確性 | 20260811 |
| 42 | GOLDEN QUIRKS PRESERVED 項1 | QUIRK | ainarm2.cpp:3201-3204（`SetInArmUseSuckToHasTrySuckIC`） | golden :261-318 vs 姊妹函式 :201 | else 分支用寫死的 2x4（`i<2, j<4`）而非 `InArmSuck.iPickRow`/`iPickCol`，姊妹函式 `SetInArmUseSuckToHasNullIC` 卻用後者 | 2x8 格局的機台在此分支下只有第 0..3 欄會被標記 | 20260811 |
| 43 | GOLDEN QUIRKS PRESERVED 項2 | QUIRK | ainarm2.cpp:3205-3207 | golden :284-285 | JerryYang 20251013 把 `Prod.fInArmSuck4x8[iSht][i][j2]==true` 的保護判斷註解掉，使除 `CC_TSMC_TAINAN` 以外的所有客戶碼，連未使用的噴嘴也會被標成 `HAS_TRY_SUCK_IC` | 非 TSMC-Tainan 客戶碼的機台，未使用噴嘴也會被誤標 | 20260811 |
| 44 | GOLDEN QUIRKS PRESERVED 項3 | QUIRK | ainarm2.cpp:3208-3209 | golden（同函式） | `iKit32`/`iSht` 兩個計算值只在 `CC_TSMC_TAINAN` 分支內被消耗 | 非 TSMC-Tainan 客戶碼下純屬多算，不影響行為 | 20260811 |
| 45 | GOLDEN QUIRKS PRESERVED 項1 | QUIRK | ainarm2.cpp:3422-3426（`CloseSiteState`） | golden :445-462,:464-481 | Auto-Clean 分流的兩個 else 分支都只處理 `IsNNMode()==NN_2Row` 的情況，非 NN 機台若 :484-499 沒觸發，梯狀判斷會落到底端回傳初始值 0 | 非 NN 機台在特定分流下永遠得到「不需關站」的結果 | 20260811 |
| 46 | GOLDEN QUIRKS PRESERVED 項2 | QUIRK | ainarm2.cpp:3427 | golden :509,:522 | 兩處把 `iState` 賦值為 0，而 0 正是它宣告時的初始值 | 純死賦值，不改變任何行為 | 20260811 |
| 47 | GOLDEN QUIRKS PRESERVED 項3 | QUIRK | ainarm2.cpp:3428-3430 | golden :392-393 | `bCanAutoCloseSite && bPlace==false` 讓執行期關站路徑對 PICK 側刻意回報兩列都在用 | golden 既有設計，非誤判 | 20260811 |
| 48 | GOLDEN QUIRK（未編號） | QUIRK | ainarm2.cpp:3615-3619（`CheckClearAllHotICThenPickLoadIC`） | golden :542 | `HotPlateForm.iPlateSelect & 2-i` 因減法優先權高於位元 and，實際解析成 `iPlateSelect & (2-i)`，golden 自己的行內註解寫「MMPlate1+0=NO 2 HP」 | 迴圈對 i==0 測 bit1、i==1 測 bit0，刻意不加括號、逐字保留 | 20260811 |
| 49 | GOLDEN QUIRK（未編號） | QUIRK | ainarm2.cpp:3620-3623 | golden（同函式） | `iLimit` 只有在 `iPickRow==2` 且 `HotPlateYPitchCanPutAll()` 才非 0，`if(iLimit<=iCT) return true;` 在 iLimit==0 時對任何 `iCT>=0` 皆為真 | 1-row 與「無法整批放入」的情況會無條件回傳 true，屬 golden 既有行為非筆誤 | 20260811 |
| 50 | GOLDEN QUIRK（未編號） | QUIRK | ainarm2.cpp:6339-6340（`IsHotPlateCheckFinsih`） | golden :2234 | 與 part 00533（`CheckClearAllHotICThenPickLoadIC`）相同的 `iPlateSelect & 2-i` 運算子優先權巧合，在此檔的另一個函式再度出現 | 兩處互為獨立但同型的 golden 既有寫法 | 20260811 |
| 51 | GOLDEN QUIRK（未編號） | QUIRK | ainarm2.cpp:6341-6343 | golden :2236-2237 | `if(HotPlateForm.iPlateSelect==3 && i==0) continue;` 使兩盤都選取時，plate 1 整個被跳過，只有 plate 2 被計數 | golden 既有行為，不予修正 | 20260811 |
| 52 | GOLDEN QUIRK（未編號） | QUIRK | ainarm2.cpp:6344-6346 | golden :2251,:2297 | `iDiscount` 初始化為 0 且僅被賦值為 0，使 `iCT-iDiscount<iCheckCT` 等同 `iCT<iCheckCT`，是死算式 | 純冗餘運算，不改變比較結果 | 20260811 |
| 53 | GOLDEN QUIRK（未編號） | QUIRK | ainarm2.cpp:6347-6350 | golden :2260,:2265,:2267,:2271 | `bOneTimeHotPlateCheckAll` 為 false 時，函式會從 else 區塊內部直接 return，永遠不會走到 `iCheckCT` 比較，使前面整段計算在此路徑上是死碼 | golden 既有的路徑分岔，忠實保留 | 20260811 |

TOTAL for ainarm2.cpp（含二輪 QUIRK 補掃）: 53

### ainarm9045.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG kept verbatim | BUG | ainarm9045.cpp:2761 | golden :742,:820-821,:823 | MoveInArmXYToShuttle_9045 的 bool bResult 在 switch 的 default 分支從未被賦值，return bResult 回傳未定義的值。 | 出現任何未列舉的 iTestMode 時，回傳值不可預期，呼叫端可能依此誤判移動是否成功。 | 20260811 |
| 2 | GOLDEN ODDITY kept | ODDITY | ainarm9045.cpp:3448 | golden (CheckShuttleSensor_9045_2x8) | `={false, false}` 只明確初始化 bDuplicateErr[2][10] 20 個元素中的 2 個，其餘靠聚合初始化規則隱含清 0。 | 純撰寫風格不一致，實際結果與全部顯式初始化相同，不影響行為。 | 20260811 |
| 3 | GOLDEN BUG kept (do NOT fix) | BUG | ainarm9045.cpp:4105 | golden :1431,:1448 | CheckInArmYStep 用等於比較 double(dSiteYPitch*100) 與 int(iYPitch)，無法精確表示的 Y-pitch 配方會落到 else 判為 iYStep=2。 | 特定 recipe 的 Y-pitch 可能因浮點精度誤差被誤判為需兩段式，影響 UPH 但不致碰撞。 | 20260811 |
| 4 | GOLDEN ASYMMETRY kept | ASYMMETRY | ainarm9045.cpp:4113 | golden (CheckInArmYStep) | in-arm/out-arm 的 Y-step 判斷整段包在測試模式階梯的 else 分支，單列模式下 OutArmSuck.iYStep 完全不被觸碰，只有 InArmSuck.iYStep 被強制設為 1。 | 單列模式下 out-arm 的 Y-step 維持先前狀態，若殘留不同值，可能造成 in-arm 與 out-arm 的 Y 步進判斷不同步。 | 20260811 |
| 5 | GOLDEN BUGS KEPT VERBATIM #1 | BUG | ainarm9045.cpp:4229 | golden :2178 | _10Site2X5 分支只用單一條件判 ret2 OK，其餘 10 分支要求雙條件同時成立（Ifor 20200811 修正未套用到此分支）。 | 特定組態下只有 2x5 機型跳過 shuttle 感測檢查，機型間安全檢查不一致。 | 20260811 |
| 6 | GOLDEN BUGS KEPT VERBATIM #2 | BUG | ainarm9045.cpp:4229 | golden :2070-2229 | CheckShuttleSensor_9045 的 switch 沒有 default，任何未列舉的 iTestMode 都讓 ret1=ret2 維持初始值 1，最終回 false。 | 新機型若忘記加入此 switch，感測器會表現為永遠異常（保守失敗方向）。 | 20260811 |
| 7 | GOLDEN BUGS KEPT VERBATIM #3 | BUG | ainarm9045.cpp:4229 | golden :2158,:2196,:2209 | _32Site4X8N/_16Site2X8 共用 _2x8 檢查函式、_16Site4X4/_8Site2X4 共用 _2x4 檢查函式，用較小規格邏輯驗證較大規格機型。 | 4x8/4x4 佈局實際只被當 2x8/2x4 檢查，多出的感測點可能未被真正涵蓋。 | 20260811 |
| 8 | GOLDEN BUGS KEPT VERBATIM #4 | BUG | ainarm9045.cpp:4229 | golden :2126,:2141 | 兩處呼叫少打逗號後空白，與同函式其他呼叫排版不一致。 | 純排版瑕疵，不影響編譯或行為。 | 20260811 |
| 9 | GOLDEN BUGS FOUND AND DELIBERATELY PRESERVED #1 | BUG | ainarm9045.cpp:5023 | golden :2594,:2705 | AddLoadingCount 的 iSiteCol 初始化 0 後從未重新賦值，卻用來索引 Prod.RotateDutDate，非旋轉分支永遠讀第 0 欄。 | 非旋轉機型永遠套用同一份角度資料，可能導致特定站位旋轉角度補償不正確。 | 20260811 |
| 10 | GOLDEN BUGS FOUND AND DELIBERATELY PRESERVED #2 | BUG | ainarm9045.cpp:5023 | golden :2701,:2705 | 同一陣列旋轉臂啟用時用 nozzle 座標索引，未啟用時卻用 site 座標索引，混用兩套座標系統。 | 若 nozzle 與 site 座標非簡單一一對應，可能讀到不對應的資料列。 | 20260811 |
| 11 | GOLDEN BUGS FOUND AND DELIBERATELY PRESERVED #3 | BUG | ainarm9045.cpp:5023 | golden :2629 | AddLoadingCount 呼叫 AutoSkipHasIClog 傳入 (iTrayCol, iTrayRow)，緊接呼叫 AddLoadRecord 卻傳入 (iTrayRow, iTrayCol)，順序相反。 | 若其中一方參數順序假設有誤，會把 Tray 列/欄座標記錄顛倒。 | 20260811 |
| 12 | GOLDEN BUGS FOUND AND DELIBERATELY PRESERVED #4 | BUG | ainarm9045.cpp:5023 | golden :2802-2808 | if(CosFunction.bQAModeUseUnloadCnt){}else{...} 的 then 分支完全空白，所有動作寫在 else。 | 撰寫風格怪異，行為上等價於條件反過來寫，不構成功能缺陷。 | 20260811 |
| 13 | GOLDEN BUG, PRESERVED | BUG | ainarm9045.cpp:5339 | golden :2845,:2852,:2853-2857 | DoTrayEndProcess_9045 的 bHasSkip 被寫兩次，唯一讀取處是被註解掉的 MES0101 生產紀錄程式碼。 | 目前是純死狀態；若未來重新啟用該註解區塊，需留意讀取時機是否仍正確。 | 20260811 |
| 14 | GOLDEN BUG KEPT | BUG | ainarm9045.cpp:5526 | golden :2896,:2929 | DoTraySkipProcess_9045 的區域變數 bHasSkip 未初始化，但唯一讀取路徑必先賦值，目前是良性未初始化。 | 現況不會造成未定義行為，但未來若出現先讀後寫新路徑會成為真正的未定義行為地雷。 | 20260811 |
| 15 | GOLDEN BUGS PRESERVED #1 | BUG | ainarm9045.cpp:6454 | golden :4944-4952 | AutoCalculateInArmYClosePitch 的 do-while 在 tray YPitch 為 0 時，break 條件永不成立、while 條件永遠為真，形成無窮迴圈。 | Loader tray 型態未正確設定時，呼叫此函式會把主控迴圈卡死在無窮迴圈中，本清單風險最高項之一。 | 20260811 |
| 16 | GOLDEN BUGS PRESERVED #2 | BUG | ainarm9045.cpp:6454 | golden :4954 | 若上述迴圈靠 while 條件結束，InArmClose_PitchY 本次未被賦新值，卻可能被當成剛解出的結果回傳。 | 呼叫端可能拿到過期或未初始化的 Y-close-pitch 值，影響下壓/合模間距正確性。 | 20260811 |
| 17 | GOLDEN BUGS PRESERVED #3 | BUG | ainarm9045.cpp:6454 | golden :4932,:4935 | iMin 只在機構設定值嚴格大於 2500 時才被提高，低於 2500 的設定值會被靜默忽略。 | 機械上允許更小 Y-pitch 的機型仍套用 2500 保守下限，部分合法配置可能無法被選用。 | 20260811 |
| 18 | GOLDEN BUGS PRESERVED #4 | BUG | ainarm9045.cpp:6454 | golden (AutoCalculateInArmYClosePitch) | 第二參數 bCheckIsZero 在函式本體完全未被讀取，是 golden 自身的死參數。 | 呼叫端傳入的值不影響任何行為，純屬介面冗餘。 | 20260811 |
| 19 | GOLDEN BUGS KEPT VERBATIM #1 | BUG | ainarm9045.cpp:6910 | golden :5194-5195 | iCurrSuck>4 判斷因搜尋迴圈只跑 i<4，該半邊條件永遠不可能為真，理應寫成>=4。 | 未來若搜尋迴圈上限放寬，此判斷不會攔到真正越界情況，是潛在邊界檢查漏洞。 | 20260811 |
| 20 | GOLDEN BUGS KEPT VERBATIM #2 | BUG | ainarm9045.cpp:6910 | golden :5197,:5357 | 從宣告回傳型別為 int 的函式回傳 double 值 dInArmXPitch_MovePitch，會被截斷。 | 若該值帶小數，回傳值遺失小數，屬 golden 既有截斷行為。 | 20260811 |
| 21 | GOLDEN BUGS KEPT VERBATIM #3 | BUG | ainarm9045.cpp:6910 | golden :5203,:5205,:5347,:5349 | 多處把 double 運算式結果直接指派給 int& 輸出參數 iXPos，造成截斷；為機台既有真實捨入行為，不可用四捨五入取代。 | X 座標存在既有整數截斷誤差，是 golden 機台原有定位精度特性。 | 20260811 |
| 22 | GOLDEN BUGS KEPT VERBATIM #4 | ASYMMETRY | ainarm9045.cpp:6910 | golden :5300-5302 | 六個搜尋分支中只有 iPickCol==4 分支找到匹配後會 break，其餘五分支繼續跑完迴圈；golden 自身註解稱 asymmetry。 | 最終結果相同，但此不對稱寫法容易在未來修改其中一分支時引入真正行為差異。 | 20260811 |
| 23 | GOLDEN BUGS KEPT VERBATIM #5 | BUG | ainarm9045.cpp:6910 | golden :5300 | iPickCol==4 分支把 dInArmXPitch_1Step 設成整個 iTrayXPitch，其他分支則除以 2/3/4/6 後才賦值。 | 不同 picker 數配置下單步 X pitch 計算基準不一致，是 golden 既有機構設計差異。 | 20260811 |
| 24 | GOLDEN BUGS KEPT VERBATIM #6 | BUG | ainarm9045.cpp:6910 | golden :5222 | iTrayXPitch=DeviceForm.XDimension*i+200 是 double 乘 int 再加 200 後截斷成 int，i==0 時退化為常數 200。 | 屬既有浮點截斷行為，i==0 時 Tray X pitch 直接得固定值 200。 | 20260811 |
| 25 | NAME NOTE -- THIS IS A GOLDEN BUG | BUG | ainarm9045.cpp:7746（亦見:10012） | golden ainarm9045.h:78/:53 vs .cpp:5604 | golden 標頭宣告 MoveArmXYToLoader_9045（無 In），實際定義卻是 MoveInArmXYToLoader_9045（有 In），標頭宣告的符號從未被定義。 | port 忠實複製此拼字錯誤，舊 stub 綁定錯誤拼法而永遠無法被真實函式體取代，整併者須手動決定保留哪個名稱。 | 20260811 |
| 26 | GOLDEN BUGS PRESERVED #1 | BUG | ainarm9045.cpp:8023 | golden :5962-5971,:5997-6000,:6022-6030,:6058-6061 | Find_InArm_PickerMaxUseCountOnTime 標記 nozzle 不可用後仍繼續索引 Tray.Data，讀到超出配置 tray 範圍的資料。 | 讀到不屬於當前 tray 配置的殘留資料，可能誤判某超界位置有/無 IC。 | 20260811 |
| 27 | GOLDEN BUGS PRESERVED #2 | BUG | ainarm9045.cpp:8023 | golden :5973,:6033 | Tray.Data 第二列探測從未對照 Tray.YItem 做邊界檢查。 | 可能讀到超出 Y 方向配置範圍的資料，誤判第二列探測位置的 IC 狀態。 | 20260811 |
| 28 | GOLDEN BUGS PRESERVED #3 | BUG | ainarm9045.cpp:8023 | golden :5926,:5937 | isStart 只在還有空 nozzle 時才被賦值，若當下無空 nozzle，後續運算沿用先前呼叫或未初始化的舊值。 | 無空 nozzle 情境下，後續座標計算可能基於過期值，讓規劃出的吸取起點座標不準確。 | 20260811 |
| 29 | GOLDEN BUGS PRESERVED #4 | BUG | ainarm9045.cpp:8023 | golden :6088 | x=ixEnd 後緊接 break，這個賦值在跳出迴圈前完全無作用，是死賦值。 | 純無效程式碼，不影響任何後續邏輯或效能。 | 20260811 |
| 30 | GOLDEN BUGS PRESERVED #5 | BUG | ainarm9045.cpp:8023 | golden :5757 | JCET 前排 fallback 情境下，函式修改的是傳值參數 bSearchLast 本身，呼叫端 TestIF.bSearchLastMode 不受影響。 | 呼叫端原始搜尋模式設定不受此函式內部調整影響，屬 golden 既有參數語意。 | 20260811 |
| 31 | GOLDEN BUG #1 kept, integer-division class | BUG | ainarm9045.cpp:8954 | golden :6414,:6415,:6421,:6438 | Find_InArm_Single 中 golden 把原本整數除法的 block-row 索引改成浮點除法，X 側同段卻仍用整數取餘數，兩側不對稱證明是人工修改痕跡。 | YArea 在非整除情況下可能多跨一個 block row，造成 Y 方向 tray block 定位偏移。 | 20260811 |
| 32 | GOLDEN BUG #2 kept | BUG | ainarm9045.cpp:8965 | golden :6411 | bBlockEmpty 只在迴圈外初始化 true 一次、之後從未重置，第一個非空 block 出現後推進邏輯永遠無法再觸發。 | Loader block 走訪邏輯遇到第一個有料 block 後可能停止繼續找空 block。 | 20260811 |
| 33 | GOLDEN ODDITY kept | ODDITY | ainarm9045.cpp:10053 | golden (SearchAndMoveInArmXYToLoad_9045) | 三選一 if/else-if/else 中第一與第三分支動作完全相同，第一個判斷條件實際只在第二個條件也成立時才影響結果。 | 維護者若誤以為三分支各自獨立，可能誤改其中一支而不自知已與另一支重複。 | 20260811 |
| 34 | GOLDEN BUGS, PRESERVED #1 | BUG | ainarm9045.cpp:10229 | golden :7036,:7065,:7045-7057 | ProcessMES0101InArmPickLoaderError 的 iX,iY 未初始化，特定條件下搜尋迴圈永不賦值卻仍被讀取。 | loader tray 完全無 IC 的邊界情境下，座標以未初始化垃圾值被使用，可能造成不可預期的判斷。 | 20260811 |
| 35 | GOLDEN BUGS, PRESERVED #2 | BUG | ainarm9045.cpp:10229 | golden :7230 | TrayForm.LoaderToEmptyColor 陣列大小只有[2]，但索引用的 iRunStartMode 至少可到 12。 | FIFO 等模式下會讀到陣列邊界外記憶體內容，可能導致不可預期的判斷結果。 | 20260811 |
| 36 | GOLDEN BUGS, PRESERVED #3 | BUG | ainarm9045.cpp:10229 | golden :7161,:7194,:7178 | RecordProcess 紀錄標籤被兩個不同路徑重複使用，製程紀錄無法區分是哪條路徑觸發。 | 事後追查製程紀錄時無法判斷是哪種錯誤處理路徑造成，影響異常分析可追溯性。 | 20260811 |
| 37 | GOLDEN BUG (k7-B1), KEPT EXACTLY AS WRITTEN | BUG | ainarm9045.cpp:11067（banner:10915） | golden :7577-7580 | CheckOneCycleAction 結尾判斷式因運算優先權，實際相當於「(Task==iTask 且 bPickFromLoader) 或 bPickFromHotplate 或 InArmSuck.HasIC() 三者有一成立」，非排版暗示的整體且。 | Task 選擇可能被 bPickFromHotplate/InArmSuck.HasIC() 意外覆蓋成 50，是本清單中影響 one-cycle 流程走向最直接的邏輯陷阱。 | 20260811 |
| 38 | GOLDEN BUG kept | BUG | ainarm9045.cpp:11509 | golden ainarm9045.h vs .cpp | .cpp 簽名註解與標頭檔對 iType 語意（0:A row，1或2:B row）有兩種矛盾說法。 | 呼叫端若依錯的一份文件傳參數，會使 Y 座標整整偏移一個 pitch。 | 20260811 |
| 39 | GOLDEN ODDITY kept | ODDITY | ainarm9045.cpp:11516 | golden :9047,:9052-9059,:9054 | golden 先把 iMovePitchY 設值並用於呼叫，之後整段覆寫該值，特定條件下等於重新指派同一值。 | 屬撰寫冗餘，該情境下無實際效果差異，只是多做一次無意義指派。 | 20260811 |

TOTAL for ainarm9045.cpp: 39

### ainarm9045.cpp QUIRK 二輪補掃（另起新序號 40-50，避免與上方主表 1-39 及檔頭「ainarm9045.cpp 補充」1-2 兩段既有編號混淆；來源：orchestrator 對本檔全樹重跑 `grep -inw quirk`，補齊第一輪 grep 樣式未涵蓋的 12 個 QUIRK banner）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 40 | GOLDEN QUIRK kept（未編號） | QUIRK | ainarm9045.cpp:2598-2601（`ArmXCanSuck4IC_1032`） | golden :117（對照姊妹函式 `ArmXCanSuck4IC_9045` golden :90） | golden :117 的行內註解寫「3750」是從姊妹函式複製過來，姊妹函式那裡確實用來守 `iXpitchMax`，但這裡實際守的是 `iXpitchMaxX2`，數字已與常數脫鉤 | 純註解過時，不影響判斷邏輯 | 20260811 |
| 41 | GOLDEN QUIRK PRESERVED, NOT FIXED（未編號） | QUIRK | ainarm9045.cpp:3302-3307（`CheckShuttleSensor_9045_2x5`） | golden :1027 | `static bool bDuplicateErr[2][8]={false, false};` 用 2 個元素的初始化列表初始化 2x8 陣列（其餘語言自動補 0），讀起來像是每個 shuttle 一個初始值而非每個 cell 一個；`bDuplicateErr[Index]`/`bSensorOn[Index]` 也沒有對 Index 做邊界檢查 | Index 落在 [0,2) 之外會讀到範圍外記憶體 | 20260811 |
| 42 | GOLDEN QUIRKS kept as-is（未編號） | QUIRK | ainarm9045.cpp:3366-3368（`CheckShuttleSensor_9045_2x6`） | golden :1042 | `bDuplicateErr` 宣告為 `static`，使重複警報鎖存刻意跨呼叫持續；16 個元素中只有 2 個被明確初始化，其餘由語言補 0 | 兩者皆為既有設計，忠實保留 | 20260811 |
| 43 | GOLDEN QUIRK kept as-is（未編號） | QUIRK | ainarm9045.cpp:4525-4527（`CheckPlaceToShuttle`） | golden :2316 | `int iCenter;` 未初始化，但唯一會讀取它的 `_16Site2X8`/`_12Site2X6`/`_32Site4X8N` 路徑一定先賦值(:2332-2335)，golden 本身結構上是安全的 | 現況無害，忠實保留未初始化寫法 | 20260811 |
| 44 | GOLDEN QUIRK kept as-is（未編號） | QUIRK | ainarm9045.cpp:4622-4625（`AutoTrayEndMusic`） | golden :2527-2538 | 全域旗標拼字為 `bNeedMusicFinishLighAndAlarmOn`（"Ligh" 少一個 t），且此函式只 SET `...AndAlarmOn`、只 READ `...FinishLighAndAlarmOn`，兩者是不同物件 | 拼字與讀寫不對稱皆為 golden 既有設計，不予修正 | 20260811 |
| 45 | GOLDEN QUIRK KEPT (not a translation choice)（未編號） | QUIRK | ainarm9045.cpp:5691-5695（`DoRecordSkipPosition_9045`） | golden（同函式） | iTrayDir 0/1 的「保留最後一列」分支把 `iAutotrayEndYEnd` 設成 `YItem-1`，等最後一列已搜過又還原成 `YItem`（視窗重新變寬）；其他方向的鏡像程式碼則是移動 `iAutotrayEndYStart` 從 1 到 0 | 兩個方向的處理方式不對稱，屬 golden 既有設計 | 20260811 |
| 46 | GOLDEN QUIRK PRESERVED（未編號） | QUIRK | ainarm9045.cpp:6024-6025（`InArmSideAllCloseWithKit`） | golden :4251-4271 | 內層邊界是寫死的 `j<4`，不是 `InArmSuck.iMaxCol`，這個寫死的 4 正是讓 `iKit32` 形成 4 欄視窗的關鍵 | 逐字保留，不用變數化 | 20260811 |
| 47 | GOLDEN QUIRK PRESERVED (do not "fix")（未編號） | QUIRK | ainarm9045.cpp:7164-7170（`LotRecordUPH`） | golden :5372 vs :5366 | golden :5372 把 `AnsiString` 物件直接傳進 `%s` 可變參數槽（在 BCB6 下是潛在 UB，但因 AnsiString 是單一指標而恰好可用）；同函式 golden :5366 卻對兩個參數都用 `.c_str()`，兩種寫法在同一函式內並存 | port 的 vclcompat sprintf 有意支援這個 BCB6 慣用法，逐字保留這處不一致 | 20260811 |
| 48 | GOLDEN QUIRK kept as-is（未編號） | QUIRK | ainarm9045.cpp:7837-7840（`MoveInArmXYToLoader_9045`） | golden :5657-5660,:5705-5713 | golden 的 `else` 縮排整整少一階（比它自己的大括號更靠左），出現在一段攸關安全的 Y-pitch 搶救分支中；另外 golden :5705-5713 是 Eastsun 20251231 註解掉的死區塊 | 兩者皆逐字保留，重排縮排會是對安全相關分支的無聲改動 | 20260811 |
| 49 | GOLDEN QUIRK kept verbatim（未編號） | QUIRK | ainarm9045.cpp:10119-10123（`MoveInArmZToLoaderPick`） | golden :7018 | `return bResult=false;`（先賦值再回傳）逐字保留；SingleSite 分支只靠單一次 `MotorMove2Speed...` 呼叫寫 `bResult` 而不查 `flag[][]`，else 分支則忽略 `bResult` 的初始值 true，直到走完 flag 掃描才採信 | 兩分支對 `bResult` 的來源邏輯不對稱，屬 golden 既有設計 | 20260811 |
| 50 | GOLDEN LADDER QUIRKS（未編號） | QUIRK | ainarm9045.cpp:11329-11333（`DoInArmPlaceToShuttle_9045`） | golden :8723-8839(:8732-8733) | 32 支 `iInArmType` 分派梯全樹沒有 `e9045_2x4_16` 的分支（與 port 已記錄的「該檔案本就死碼」一致）；`e9045_1x4_1_Ac` 被併入 `1x1_1` 分支（golden :8732-8733）；分派順序沿用 golden 順序，含 `1x4_4_13` 排在 `1x4_2_14` 之前 | 逐字保留 golden 的分派順序與併入決定 | 20260811 |

TOTAL for ainarm9045.cpp（含二輪 QUIRK 補掃）: 50（主表39 + 檔頭補充2 + 本節11，惟三段編號互不相通，見各節標題說明）

### aoutarm9045.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG PRESERVED, NOT FIXED | BUG | aoutarm9045.cpp:3365 | golden :3995-4079 (:3999) | GetOutOffsetFromWhichAuto 在無效輸入分支中，sprintf 對整數 iWhichAuto 誤用字串格式符而非整數格式符。 | 在真正 BCB6 執行環境下該診斷訊息會印出亂碼甚至造成例外，但只在輸入已無效的分支才觸發。 | 20260811 (PT-W8) |

TOTAL for aoutarm9045.cpp: 1

### aoutarm9045.cpp QUIRK 二輪補掃（承續上表編號，續 2-5；來源：orchestrator 對本檔全樹重跑 `grep -inw quirk`，補齊第一輪 grep 樣式未涵蓋的 4 個 QUIRK banner）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 2 | GOLDEN QUIRK PRESERVED, NOT FIXED（未編號） | QUIRK | aoutarm9045.cpp:2629-2631（`GetOutArmXToShuttleOffset_9045`） | golden :1290-1301(:1295-1298) | `if(iSht==0)` 的兩個分支內容完全相同（都是 `OutArmOffSet[iOffsetPos]->GetX()`），使這個判斷式是無效分支 | 純冗餘判斷，不影響回傳值 | 20260811 (PT-W8) |
| 3 | 同一 golden quirk（X 孿生函式，未編號） | QUIRK | aoutarm9045.cpp:2648（`GetOutArmYToShuttleOffset_9045`） | golden :1303-1314 | 與 X 版同型缺陷：`if(iSht==0)` 兩分支同樣完全相同 | 純冗餘判斷，不影響回傳值 | 20260811 (PT-W8) |
| 4 | GOLDEN QUIRK PRESERVED（未編號） | QUIRK | aoutarm9045.cpp:3523-3526（`HasGapsInTheTray`） | golden :4127-4165(:4134-4135) | `GetTrayDirection()` 先把結果寫進 `Direction`，緊接著下一行就被 `AutoForm[iWhichAuto]->Direction` 覆蓋 | `GetTrayDirection` 呼叫本身仍是必要的（它同時填了六個迴圈邊界），只有 `Direction` 這個輸出被覆蓋，逐字保留 | 20260811 (PT-W8) |
| 5 | GOLDEN QUIRK PRESERVED（未編號） | QUIRK | aoutarm9045.cpp:3611-3614（`GetWhichAutoPickZ`） | golden :4208-4219(:4217) | Fix 臂分支扣掉的是 AUTO 的 place-offset（`OutArmOffSet[OutOfsAuto1+iWhichAuto]->GetPlace()`），卻加上 FIX 的 pick-up offset（`OutArmOffSet[OutOfsFix1+iWhichAuto]->GetPickUp()`），兩個 offset 基準對不上 | golden 既有的基準不一致，非本次翻譯引入，不予「修正」 | 20260811 (PT-W8) |

TOTAL for aoutarm9045.cpp（含二輪 QUIRK 補掃）: 5

### atester_ProcessCount.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG PRESERVED | BUG | atester_ProcessCount.cpp:2624 | golden :2155（cmydef.h:4824） | QA 取樣計數 bQASampleCnt 型別是 bool 而非 int，遞增語意等同設為 true，使計數在第一次達到 1 後就飽和。 | 配方要求每批抽測多於 1 顆 QA 樣本時，實際整個 lot 只會抽到第一顆，之後不再取樣。 | 20260811 (PT-W7f) |

TOTAL for atester_ProcessCount.cpp: 1

### atester_shims.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY TO PRESERVE, NOT TO FIX | ODDITY | atester_shims.h:171 | golden ckernel.cpp:56-57 vs :98-99 | WaitManualStepKey 與 WaitManualStartKey 的一鍵完成提前返回判斷式結構完全相同，但回傳值正負號相反。 | 一旦接上 Contact 表單，一鍵模式下 Step 會被自動放行而 Start 被自動擋下，讓校正 SM 停留在單步而非自由執行狀態。 | 20260803 |

TOTAL for atester_shims.h: 1

### aTester_Front.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUGS PRESERVED, NOT FIXED #1 | BUG | aTester_Front.cpp:5154 | golden :3969-3971（Rear雙生:3906-3907） | DoFRTCUseSocketFloat 的 encoder 保護判斷因運算優先權，等同「(REAL_TIME_CCD 且 Z1 檢測失敗) 或 Z2 檢測失敗」，即使 REAL_TIME_CCD 為 false，Z2 失敗仍觸發警報。 | 非 real-time-CCD 機型上 Z2 encoder 異常仍可能誤觸發 Index 馬達錯誤警報。 | 20260810 |
| 2 | GOLDEN BUGS PRESERVED, NOT FIXED #2 | BUG | aTester_Front.cpp:5154 | golden :3971 | 同一判斷中 Z2 讀值被拿去與 Z1 的安全高度比較，而非它自己的安全高度。 | Z2 安全範圍判斷基準用錯軸，可能誤判 Z2 是否處於安全位置。 | 20260810 |
| 3 | GOLDEN BUGS PRESERVED, NOT FIXED #3 | BUG | aTester_Front.cpp:5154 | golden :3947,:3964 | 靜態變數 bVerifyNG 被寫入但從未讀取，是死狀態。 | 純無效狀態保存，不影響任何判斷結果。 | 20260810 |
| 4 | GOLDEN BUGS PRESERVED, NOT FIXED #4 | BUG | aTester_Front.cpp:5154 | golden :3955 | int ret 未初始化，但唯一讀取路徑必先經賦值，目前是良性未初始化。 | 現況不構成未定義行為，但未來若新增讀取路徑而未先賦值會成為真正風險。 | 20260810 |
| 5 | GOLDEN BUG kept behaviourally identical | BUG | aTester_Front.cpp:7450 | golden :5210 | golden 省略型別依 K&R 舊規則隱含為 int 宣告靜態變數，可在 BCB6 編譯但非 ISO C++ 合法；port 選 int 忠實對應隱含型別。 | 寫入讀取皆為真假語意，int 或 bool 儲存行為完全等價，純為文件一致性考量，不影響實際運作。 | 20260810 |
| 6 | GOLDEN BUG -- KEPT, NOT FIXED | BUG | aTester_Front.cpp:10780 | golden :8098-8108 (:8100,:8102；Rear鏡像:9325/9327) | FTestNeedDestroy 第二分支與第一分支文字完全相同成為死碼，Rear 側鏡像函式有同樣重複，證明是 golden 全域性複製錯誤。 | 第二分支原本可能想檢查的其他條件從未真正被執行。 | 20260810 |
| 7 | GOLDEN BUGS / ODDITIES PRESERVED, NOT FIXED #1 | BUG | aTester_Front.cpp:11720 | golden :8477-8478,:8469,:8486,:8493 | DoArm1Suck 內層判斷重複測試外層已保證的條件，使最末端裸 else（golden 註解「怪怪的」）成為永遠不執行的死碼。 | 理論異常情境（蝦頭有料且 index 也有料）的錯誤處理分支實際永遠不會執行到。 | 20260810 |
| 8 | GOLDEN BUGS / ODDITIES PRESERVED, NOT FIXED #2 | BUG | aTester_Front.cpp:11720 | golden :8512-8513,:8520-8521 | 兩個賦值在 if 內外各寫一次，在特定路徑上被重複賦值兩次。 | 純重複賦值造成效能冗餘，最終數值結果相同，不影響邏輯正確性。 | 20260810 |
| 9 | GOLDEN BUGS / ODDITIES PRESERVED, NOT FIXED #3 | INCONSISTENCY | aTester_Front.cpp:11720 | golden :8510 vs :8483 | 同樣呼叫 MoveSuckData 的兩行縮排不一致（27 vs 28 格），golden 自身文字稱之為 cosmetic inconsistency。 | 純排版瑕疵，不影響編譯結果或執行行為。 | 20260810 |

TOTAL for aTester_Front.cpp: 9


### aTester_Front.cpp 補充（承續上表編號，續 10-23；來源：orchestrator 直接補讀 `GOLDEN ASYMMETRIES vs the Rear twin` 與 `GOLDEN QUIRKS PRESERVED, NOT FIXED` 兩個清單塊，:6904-6937；這兩個清單塊描述 DoTestYFront/Arm2 相關函式，與該檔案原表已收錄的 DoFRTCUseSocketFloat 等函式屬不同區塊，故為新增而非重複）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 10 | GOLDEN ASYMMETRIES vs Rear twin 項1 | ASYMMETRY | aTester_Front.cpp:6905-6907 | golden :5108-5121（Rear對照） | case 2030 對 `BTestSuck.Suck[i][j].On()` 無條件呼叫，僅在 `INDEX_SUCKER_TYPE==1` 才額外設定 `fiosetview->bIndexSuck[1][i][j]` 與 `bIndexCheckNoStopVaccum`；Rear 對應處是 if/else 二選一 | Front/Rear 對同一情境的處理結構不同，維護時若假設兩者對稱會誤改 | 20260811 |
| 11 | GOLDEN ASYMMETRIES vs Rear twin 項2 | ASYMMETRY | aTester_Front.cpp:6908 | golden（Rear對照） | case 2030 的停留時間是 `SetSecAndOn(1)`，Rear 對應是 0.5 | Front/Rear 兩側在此步驟的停留秒數不同，屬既有機構設計差異 | 20260811 |
| 12 | GOLDEN ASYMMETRIES vs Rear twin 項3 | ASYMMETRY | aTester_Front.cpp:6909-6911 | golden :4961（Front真break）vs Rear對應break被註解掉 | case 2030 結尾有真正的 `break;`（不落穿進 2040），Rear 對應的 break 被註解掉、因此會落穿 | Front 與 Rear 在同一步驟後是否繼續執行下一 case 的行為相反 | 20260811 |
| 13 | GOLDEN ASYMMETRIES vs Rear twin 項4 | ASYMMETRY | aTester_Front.cpp:6912-6913 | golden（Rear對照） | case 2050 把 `Task=2060` 放在 Greatek 延遲 if/else 之後才賦值，Rear 是放在延遲判斷之前 | 賦值時機順序不同，維持各自 golden 既有順序，不予統一 | 20260811 |
| 14 | GOLDEN ASYMMETRIES vs Rear twin 項5 | ASYMMETRY | aTester_Front.cpp:6914-6916 | golden（Rear對照） | case 2060 用 `IndexSuckName[i][j]` 無 NN 模式列偏移，Rear 版用 `IndexSuckName[i+iNN][j]`；因此本函式完全不宣告 `iNN` | Front 側缺少 NN 模式的列偏移邏輯，是 golden 既有的不對稱，刻意不補上 | 20260811 |
| 15 | GOLDEN ASYMMETRIES vs Rear twin 項6 | ASYMMETRY | aTester_Front.cpp:6917 | golden（Rear對照） | case 2090 的警報回報對象是 `MTestY2`，Rear 對應回報 `MTestY1` | 兩側警報回報的馬達軸別相反，屬既有機構鏡像設計 | 20260811 |
| 16 | GOLDEN ASYMMETRIES vs Rear twin 項7 | ASYMMETRY | aTester_Front.cpp:6918-6920 | golden（Rear對照） | case 2100 移動到 `(TestY1_Front, TestY2_Middle)` 並停在 `Z1Up_Z2Down`；Rear 對應移動到 `(TestY1_Middle, TestY2_Rear)` 並停在 `Z1Down_Z2Up`，兩者互為鏡像 | 屬故意設計的鏡像關係，均照原樣保留 | 20260811 |
| 17 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項1 | QUIRK | aTester_Front.cpp:6923 | golden（case 1） | case 1 沒有 `break`，設定 `Task=2030` 後直接落穿進入 case 2030 | 刻意的單一 tick 內連續動作設計，補上 break 會改變時序 | 20260811 |
| 18 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項2 | QUIRK | aTester_Front.cpp:6924 | golden（case 2200） | case 2200 沒有結尾 `break`（它是 switch 最後一個 label） | 語法上無害，因為已是最後一個 case | 20260811 |
| 19 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項3 | QUIRK | aTester_Front.cpp:6925-6929 | golden Config.h:83 | `int iIndexCheckOffSet=IniConfig.fIndexCheckOffset*100;` 把 double 乘積截斷成 int，`fIndexCheckOffset` 是 double | 保留既有截斷行為，不得改寫成四捨五入或浮點運算 | 20260811 |
| 20 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項4 | QUIRK | aTester_Front.cpp:6930-6931 | golden :4989,:5060 | `ErrPart=" "`（單一空白字元而非空字串）於 golden :4989，golden :5060 才重設為 `""` | 屬既有字串初值慣例差異，忠實保留 | 20260811 |
| 21 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項5 | QUIRK | aTester_Front.cpp:6932 | golden（同函式） | `bIndexCheckNoStopVaccum=true` 在逐站迴圈內被重複賦值 | 純冗餘賦值，不影響最終旗標值 | 20260811 |
| 22 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項6 | QUIRK | aTester_Front.cpp:6933 | golden（同函式） | `ErrPart` 是函式內的 function-static `AnsiString`，跨多次掃描持續存活 | 狀態會跨呼叫殘留，屬既有設計而非疏漏 | 20260811 |
| 23 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項7（golden自身註解澄清非bug） | QUIRK | aTester_Front.cpp:6934-6937 | golden 對照 port aTester_Rear.cpp:7113 | case 2070 透過 `MOT[MTestZ1].Gali_Two_ZAxis_Move` 抬升，即使此函式是 Arm2（Rear對稱）路徑；因為 `Gali_Two_ZAxis_Move` 是透過 `MTestZ1` handle 發出的雙 Z 軸共同移動指令，Rear 對應處做法相同 | 確認非缺陷，僅記錄以避免被誤判為打錯馬達代號 | 20260811 |

### aTester_Rear.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUGS PRESERVED, NOT FIXED #1 | BUG | aTester_Rear.cpp:4945 | golden :3906-3907（Front雙生:3969-3971） | DoBRTCUseSocketFloat 的 encoder 保護判斷同樣是運算優先權陷阱，Z2 encoder 失敗不論 REAL_TIME_CCD 是否為真都觸發警報，是 Front 側同缺陷的 Rear 版本。 | 非 real-time-CCD 機型上 Z2 encoder 異常可能被錯誤升級成 Index 馬達錯誤警報。 | 20260810 |
| 2 | GOLDEN BUGS PRESERVED, NOT FIXED #2 | BUG | aTester_Rear.cpp:4945 | golden :3907 | 同段判斷 Z2 讀值被拿去與 Z1 安全高度比較而非自己的安全高度，是 Front 側同缺陷的 Rear 版本。 | Z2 安全範圍判斷基準錯用 Z1 高度，Rear 側面臨與 Front 側相同的誤判風險。 | 20260810 |
| 3 | GOLDEN BUGS PRESERVED, NOT FIXED #3 | BUG | aTester_Rear.cpp:4945 | golden :3884,:3901 | 靜態變數 bVerifyNG 同樣被寫入但從未讀取，是死狀態。 | 不影響任何判斷結果，純屬冗餘。 | 20260810 |
| 4 | GOLDEN BUGS PRESERVED, NOT FIXED #4 | BUG | aTester_Rear.cpp:4945 | golden :3892,:3958,:3952,:3954 | int ret 未初始化，但唯一讀取路徑必先經賦值，目前良性。 | 現況安全，屬未來流程改變後可能浮現的未定義行為風險點。 | 20260810 |
| 5 | GOLDEN BUGS PRESERVED, NOT FIXED #1 | BUG | aTester_Rear.cpp:5286 | golden :4203-4224 | case 18 只在延遲結束時送出 vision 指令，但 Task 推進卻無條件執行，延遲未結束時 SM 會在完全沒送指令的狀態進入下一 case 等待。 | 進入等待 case 後只能靠逾時離開，但該計時器在此路徑上也從未被啟動，可能造成狀態機卡住。 | 20260810 |
| 6 | GOLDEN BUGS PRESERVED, NOT FIXED #2 | BUG | aTester_Rear.cpp:5286 | golden :4082-4101 | case 2 的 else 分支把即時教導值存進靜態變數再覆寫成停放值，若還原前又被第二次進入，靜態變數會被已覆寫的值再次覆蓋。 | 特定重入時序下，Index 教導高度資料可能在單次 run 中永久遺失，需重新教導才能恢復。 | 20260810 |
| 7 | GOLDEN BUGS PRESERVED, NOT FIXED #3 | BUG | aTester_Rear.cpp:5286 | golden :4090,:4100 | 備份位置變數是在對應欄位已被改寫成安全值之後才取值備份，記錄的其實是安全高度而非測試下壓高度。 | 該備份欄位名不符實，若其他邏輯依賴它還原真正測試位置，實際會拿到安全高度。 | 20260810 |
| 8 | GOLDEN BUGS PRESERVED, NOT FIXED #4 | BUG | aTester_Rear.cpp:5286 | golden :4066,:4209 | 把一個 bool 參數拿去與整數 1 比較。 | bool 轉 int 時 true 恆等於 1，行為等價，不構成實質缺陷，僅寫法怪異。 | 20260810 |
| 9 | GOLDEN BUGS PRESERVED, NOT FIXED #5 | BUG | aTester_Rear.cpp:5286 | golden :4032,:4054 | site-map 只在特定條件下重建，但靜態變數卻在每次呼叫都無條件送出，其他呼叫會送出上次留下的舊 map。 | 對外送出的 site map 可能是過期或空白資料，影響依賴此資料的下游判斷。 | 20260810 |
| 10 | GOLDEN BUG kept behaviourally identical | BUG | aTester_Rear.cpp:7437 | golden :5412 | 與 Front 側相同的 K&R 隱含 int 寫法，golden 在 Rear 側同樣省略型別，port 用 int 忠實對應。 | 純屬型別記錄一致性考量，不影響任何實際行為，是 Front 側同缺陷模式在 Rear 側的獨立重現。 | 20260810 |
| 11 | GOLDEN ODDITY PRESERVED VERBATIM | ODDITY | aTester_Rear.cpp:11128 | golden :8904-8906 | if 判斷式的 then 分支完全空白，所有實際動作都寫在 else 分支（同一行註解另含 GATE W7R3-G15/G16，屬未接線呼叫，已排除不計入）。 | 撰寫風格怪異，行為上等價於條件反過來寫，不構成功能缺陷。 | 20260810 |
| 12 | GOLDEN BUG -- KEPT, NOT FIXED | BUG | aTester_Rear.cpp:11654 | golden :9323-9333 (:9325,:9327；Front鏡像:8100/8102) | BTestNeedDestroy 第二分支與第一分支文字完全相同，是 Front 側 FTestNeedDestroy 同一複製貼上缺陷的鏡像重現。 | 第二分支原本可能想檢查的其他條件從未真正被檢查。 | 20260810 |

TOTAL for aTester_Rear.cpp: 12

### aTester_Rear.cpp QUIRK 二輪補掃（承續上表編號，續 13-36；來源：orchestrator 對本檔全樹重跑 `grep -inw quirk`，補齊第一輪 grep 樣式未涵蓋的 10 個 QUIRK/DEFECT banner，橫跨 5 個函式）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 13 | GOLDEN DEFECTS PRESERVED 項1 | DEFECT | aTester_Rear.cpp:2904-2911（`DoBTestSuckTestIC`） | golden :2342-2359(:2348) | `ErrPart+=BTestSuck.Item[i][j]+", ";` 的 `Item` 是 `int`，這其實是對 3-byte 字串常量 ", " 做指標運算（`const char*+int`）而非「附加數字」，`Item==TEST_PASS+n` 時會讀到常量字串範圍外的記憶體 | 只在 `bAlreadyTested` 中止路徑觸發的未定義行為；逐字保留是因為相同運算式在標準 C++ 下編譯結果與 golden 一致 | 20260810 |
| 14 | GOLDEN DEFECTS PRESERVED 項2 | DEFECT | aTester_Rear.cpp:2912-2918 | golden :3070 | `Task=3200;`（[P65] ARM QA Mode ReTest, Arm2）跳到整個 switch（golden :2222-3878 範圍內搜尋確認）根本不存在的 `case 3200:` | 一旦被觸發，之後每次呼叫都會落到 switch 尾端 `return false`，rear test 狀態機永久卡死；僅在 `bP65EnableArmQAMode && iP65ArmQAModeValue>0` 且全站 bin1 且 `BTestSuck.HasRealIC()` 時可達 | 20260810 |
| 15 | GOLDEN DEFECTS PRESERVED 項3 | DEFECT | aTester_Rear.cpp:2919-2921 | golden :2232,:2237 | `int &Task=iBTestSuckTestICTask, ret, iMaxDoubleContact, ret2;` 與 `bool bAlreadyTested;` 均未初始化，golden 依賴每條可達路徑都先賦值 | 目前無害，忠實保留未初始化寫法 | 20260810 |
| 16 | GOLDEN DEFECTS PRESERVED 項4（cosmetic） | DEFECT | aTester_Rear.cpp:2922-2923 | golden :3322 | case 2410 的 else 區塊收尾大括號縮排在第 17 欄而非第 13 欄 | 純排版瑕疵，不影響編譯或行為 | 20260810 |
| 17 | GOLDEN FALL-THROUGH PRESERVED（未編號） | QUIRK | aTester_Rear.cpp:5914-5917（`DoBRTCAutoModelVerify`） | golden :4548 | case 12310 結尾設 `Task=12320;` 且無 `break;`，落穿進入 case 12320 | 開真空的那一拍會立刻連帶執行 12320 本體，golden 刻意如此，不予「修正」 | 20260810 |
| 18 | GOLDEN QUIRK KEPT, NOT FIXED（未編號） | QUIRK | aTester_Rear.cpp:5918-5922 | golden :4520,:4636-4641 | case 12220 先把 Z2 移到 12300 的 `TestZ2_Test+1000`，case 12390 又重新發出 case 12320 早已在 :4560 完成過的同一句 `Gali_MotMove(Prod.TestZ2_Safe, iSpeedSlow)` | 多做一次相同的安全移動，逐字保留 | 20260810 |
| 19 | GOLDEN QUIRK KEPT（未編號） | QUIRK | aTester_Rear.cpp:5923-5927 | golden :4623 | case 12340 重新測試 `if(bHasErr)`，但 case 12330 只在 `bHasErr` 早已為 true 時才會轉進 12340（:4586 設值,:4600-4602 分流），故該測試永遠不會是 false，:4635 的無錯誤 `break;` 路徑不可達 | 沒有 else 分支可以移除，逐字保留死路徑 | 20260810 |
| 20 | GOLDEN QUIRK KEPT（未編號） | QUIRK | aTester_Rear.cpp:5928-5932 | golden :4732-4733,:4756-4757 | case 12900 清零 `bRealTimeCom_ReceiveOK[rtOPENVERIFYOK]` 卻送出 `rtOPENVERIFYNG`；case 13100 清零 `[rtALLFAILNG]` 卻送出 `rtALLFAILOK`，兩處清零與送出的通道對調 | golden 兩次都把清空通道與送出通道弄反，逐字保留 | 20260810 |
| 21 | GOLDEN QUIRK KEPT（未編號） | QUIRK | aTester_Rear.cpp:5933-5935 | golden :4475 | case 12100 的 `DoTestYRearDelay.SetSecAndOn(0.3)` 放在 i/j 雙層迴圈內，每個站都重新武裝一次而非整個 case 一次 | 逐字保留，不改成迴圈外呼叫一次 | 20260810 |
| 22 | GOLDEN QUIRK KEPT（未編號） | QUIRK | aTester_Rear.cpp:5936-5938 | golden :5081 | case 15200 最後一個測試的 `if(...)` 分支是空區塊 `{ }`，真正動作寫在 `else` | 邏輯上等同條件反過來寫，行為不變，僅風格怪異 | 20260810 |
| 23 | GOLDEN QUIRK KEPT（未編號） | QUIRK | aTester_Rear.cpp:5939-5942 | golden :4805-4809 vs :4717-4722 | case 13220 的 `bRTCRetry==true` 分支重新開啟 RTC port 並顯示逾時訊息，但不像它的孿生 :4717-4722 那樣呼叫 `COM2->DoReleaseAndInspEnd()` 就直接回到 13210 | golden 兩處孿生處理不對稱，逐字保留 | 20260810 |
| 24 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項1 | QUIRK | aTester_Rear.cpp:6943（`DoIndexArm1PickUpErrNeedPiggyback`，k8-Arm2-decide 標籤下的 Rear/Arm2 版本） | golden :5093-5254 | case 1 沒有 `break`：設 `Task=1030` 後直接落穿進入 case 1030 | 刻意的單一 tick 連續動作設計 | 20260810 |
| 25 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項2 | QUIRK | aTester_Rear.cpp:6944-6945 | golden :5136 | case 1030 結尾是 golden 自己註解掉的 `//            break;`，因此落穿進入 case 1040 | 與項1 同一種刻意連續動作設計 | 20260810 |
| 26 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項3 | QUIRK | aTester_Rear.cpp:6946 | golden（同函式，switch 最後一個 label） | case 2200 沒有結尾 break（因為它是 switch 最後一個 case） | 語法上無害 | 20260810 |
| 27 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項4 | QUIRK | aTester_Rear.cpp:6947-6950 | golden Config.h:83 | `int iIndexCheckOffSet=IniConfig.fIndexCheckOffset*100;` 把 double 乘積截斷成 int | 逐字保留既有截斷行為，不得改寫成四捨五入或浮點運算 | 20260810 |
| 28 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項5 | QUIRK | aTester_Rear.cpp:6951 | golden :5164,:5235 | `ErrPart=" "`（單一空白字元而非空字串）於 golden :5164，:5235 才重設為 `""` | 屬既有字串初值慣例差異 | 20260810 |
| 29 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項6 | QUIRK | aTester_Rear.cpp:6952 | golden（同函式） | `bIndexCheckNoStopVaccum=true` 在逐站迴圈內被重複賦值 | 純冗餘賦值，不影響最終旗標值 | 20260810 |
| 30 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項7 | QUIRK | aTester_Rear.cpp:6953 | golden（同函式） | `ErrPart` 是函式內的 function-static `AnsiString`，跨多次掃描持續存活 | 狀態會跨呼叫殘留，屬既有設計而非疏漏 | 20260810 |
| 31 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項1 | QUIRK | aTester_Rear.cpp:7230-7234（`CheckAnyCaseNeedToDoArm2`） | golden :5288-5291 | `else { bRTCAutoVerifyControlEP=false; }` 掛在最外層 RTC `if` 底下，使外層條件皆真但內層 `UseSiteHasIC`/`AlreadyTest`/`SendSiteMapToRTC` 檢查失敗時，該旗標不會被清除、維持前值 | 看似寫錯位置的 else，逐字保留 | 20260810 |
| 32 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項2 | QUIRK | aTester_Rear.cpp:7236-7238 | golden :5315-5321,:5341-5347 | `bIndexArm2SupplyLight`/`bForEgisTecTest`/`bD58..` 三條件區塊在 VacuumMode 開/關兩分支中被重複貼上，使真空-OFF 分支可能用 11040 覆蓋自己剛設的 211/215 | golden 既有的複製貼上重複，逐字保留 | 20260810 |
| 33 | GOLDEN QUIRKS PRESERVED, NOT FIXED 項3 | QUIRK | aTester_Rear.cpp:7239-7241 | golden :5307,:5339 | `iHangupCTArm2=0;` 在 ON 分支放在燈號檢查「之前」(:5307)，在 OFF 分支卻放在 211/215 賦值「之後」(:5339) | golden 刻意如此的位置不對稱 | 20260810 |
| 34 | GOLDEN QUIRKS preserved verbatim 項(a) | QUIRK | aTester_Rear.cpp:11536-11538（`DoRearTestPurgBeforePickShuttle`） | golden :9242-9321 | case 100 在間隔計時器未到期時把 `Task=200` 且無 break，直接落穿進入 case 200；只有 else 分支才有 break | 刻意的連續動作設計，逐字保留 | 20260810 |
| 35 | GOLDEN QUIRKS preserved verbatim 項(b) | QUIRK | aTester_Rear.cpp:11539-11541 | golden :9265-9269,:9286-9290,:9308-9312 | `if(...HAS_IC \|\| ...HAS_HOT_IC) ;`（空的 then）搭配 else 才真正 Off 的寫法，同一種樣式在此函式出現三次 | 純風格怪異，行為上等價於條件反過來寫 | 20260810 |
| 36 | GOLDEN QUIRKS preserved verbatim 項(c) | QUIRK | aTester_Rear.cpp:11542-11545 | golden（同函式） | `static int iCT=0;` 是函式局部 static，只有 case 1 會將它歸零，case 100 到 400 之間若中止，計數會停留在中止當下的值 | 忠實保留，不在函式進入時重置 | 20260810 |

TOTAL for aTester_Rear.cpp（含二輪 QUIRK/DEFECT 補掃）: 36

### 【主控/溫控叢集】涵蓋：ckernel.cpp / ckernel.h / cinitial.cpp / common.cpp / csystem.cpp / cTemperFrom.cpp / forms/fTemperFrom.h / bthermo.cpp / TempCtrl/TriTemp.cpp / MyTempPanel.cpp / uHeaterThread.cpp / EJ1N/MyOmronPanel.cpp / uYieldMonitoring.cpp

### ckernel.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:228,303 | golden :56-57 vs :98-99 | WaitManualStepKey在一鍵完成Auto Contact Height時回傳true，結構相同的判斷式在WaitManualStartKey卻回傳false，STEP一律放行、START一律否決 | 若對齊兩者符號會讓一鍵Auto Contact Height流程失去單步控制、變成自由運轉；此不對稱為刻意保留 | 20260803 |
| 2 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:258 | golden :77,:89 | fContact->bSetupStep=false在同一區塊內被寫入兩次，中間無讀取，第二次寫入是死碼 | 純屬冗餘賦值，不影響行為，保留是為了不對golden做整理 | 20260803 |
| 3 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:272 | golden :77,:82,:96-131 | 安全鎖檢查IsSafeLockCheck()在按鍵狀態都已被清除之後才執行，鎖定時按鍵訊號會被吞掉而非延後；WaitManualStartKey完全沒有安全鎖檢查 | 安全鎖生效時操作員的STEP按壓會被直接丟棄需再按一次；START按鍵完全不受安全鎖節流 | 20260803 |
| 4 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:283 | golden :88 | STEP處理常式在放行時連帶把START的燈號閂鎖bLampManualStart清成false，START常式沒有對稱清除 | 兩個手動燈號閂鎖的清除只單向發生，可能讓START燈號在某些時序下狀態不一致 | 20260803 |
| 5 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:375 | golden :161-183 | CheckThermo的for迴圈整段只剩continue陳述式，原本逐區溫度比對邏輯已被刪光，只留下zone-skip骨架 | CheckThermo唯一能回傳false的路徑只剩iHeaterCooling!=0，其餘所有溫度情境都視為已冷卻可測試 | 20260803 |
| 6 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:459 | golden :191-193 | StopAllDestroy內層迴圈的欄位邊界誤用InArmSuck.iMaxRow(應為iMaxCol)，且OutArmSuck也共用InArm的邊界值去清除 | 非正方形排列的取料頭在全部停止時只掃到部份欄位，其餘噴嘴的destroy電磁閥可能持續保持ON | 20260803 |
| 7 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:520 | golden :211-250 | CheckBinSet的if(j>=9) continue只單邊擋掉過大索引，從未檢查負值，魔術數字9與陣列真實大小eTrayCount(33)無關 | 若Prod.iT6CatData[i]為負值會造成iTrayPassFail[]/bIsPassT6[]陣列越界存取 | 20260803 |
| 8 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:541 | golden cprod.h:515 | bIsPassT6[]是bool但拿來比較的Prod.bIsPassBin[]型別是int，bool會被提升為int比較 | bIsPassBin若存了非0/1的值會誤判成Pass/Fail衝突 | 20260803 |
| 9 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:558 | golden cprod.h:520 | Prod.iIfErrorT6直接拿去索引iTrayPassFail[]，完全沒有任何邊界檢查 | 若iIfErrorT6設定值超出陣列範圍會造成越界讀取 | 20260803 |
| 10 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:600,638 | golden :265,:293 | ChangeUseSuckMode在特定分支下特定XDivision條件會走到空的break，刻意不寫iUseSuckMode | 特定寬熱盤+XDivision組合下iUseSuckMode不會被降到預期值，行為與其他分支不一致 | 20260803 |
| 11 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:656 | golden QualSite2X2分支 | QualSite2X2分支測試的XDivision清單只有6/8/12，比相鄰分支少了==4這個值 | XDivision==4時2x2熱模式行為會落到另一條路徑，與其餘分支不一致 | 20260803 |
| 12 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:676 | golden :332-342 | CUSTOMER_CODE判斷式的else分支與外層else分支內容逐位元組相同，只在單一輸入組合下才真的改變行為 | 客戶碼判斷式幾乎是無效程式碼，容易誤導維護者以為客製化邏輯更廣泛地生效 | 20260803 |
| 13 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:705 | golden :347-353 | _8Site2X4/_16Site4X4整個case分支的if/else兩邊皆為空大括號 | 8-site 2x4或16-site 4x4機型在HOT溫度下iUseSuckMode完全不會被此函式調整 | 20260803 |
| 14 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:851 | golden連續錯誤防呆區塊 | #ifndef SOFT_SIMULTE同時包住警示訊息與return false，若定義SOFT_SIMULTE整個連續失敗防呆會直接通過 | 本樹未定義SOFT_SIMULTE故此防呆目前有效，一旦巨集啟用防呆會靜默失效 | 20260803 |
| 15 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:906 | golden :427-430 | 判斷bYpitchNeddHome為真時卻只是把它再賦值為true，但因此跳過了最終else分支的CheckMotorHome()檢查 | 只要Y-pitch home待處理旗標為真，START時重新歸零檢查閘門就會被繞過 | 20260803 |
| 16 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:987 | golden :500 | 在真正的bLampAlarmReset=false正上方留有一行已註解掉、內容相同的死碼 | 純屬視覺雜訊，不影響行為 | 20260803 |
| 17 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:1055 | golden :543,:545 | SoftStop邊緣觸發時對in-arm與out-arm兩組噴嘴做快照/ReStart的迴圈邊界都只取自InArmSuck，OutArmSuck自己的尺寸從未被讀取 | out-arm排列比in-arm大的機型，多出來的噴嘴在PAUSE時不會被正確快照/ReStart/Normal | 20260803 |
| 18 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:2822 | golden :1735-1737 | DoPanelLamp有一個空的then分支只是為了把判斷式反過來寫，兩顆AlarmReset燈維持原狀不做改變 | 邏輯上等價於反向條件，不影響外部觀察行為 | 20260803 |
| 19 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:2950 | golden :1838-1883 | DoPanelLamp後排區塊並非前排區塊的鏡像，SwFKPause在特定路徑下永遠不被寫入，四個電源燈寫入順序也不同 | 後排面板作用中且馬達未上電或延遲未過時，前排Pause燈可能顯示過期狀態 | 20260803 |
| 20 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:3016 | golden :1885-1893 | iControlPanelMode==1的if/else兩分支都執行相同的SwFrontActiveLed寫入，差異只在多寫一行SwRearActiveLed | iControlPanelMode不等於1時後排Enable燈永遠不會被DoPanelLamp寫入 | 20260803 |
| 21 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:3035 | golden :1895 | bLampManualSetp是golden把Step拼成Setp的錯字，卻是真正承重的全域變數名稱 | 純屬命名瑕疵但具承重性，不能重新命名否則會破壞其他讀取此變數的地方 | 20260803 |
| 22 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:3196 | golden :1964 | 前排按鍵掃描區塊中有一段刻意輪詢後排的暫停感測器Sen[SnRKPause]卻仍回傳前排id SnFKPause，是刻意的跨面板接受設計 | 兩區塊共用同一個bAsePause旗標且會被提前清除，此段實際只能被真的按下後排實體按鍵觸發 | 20260803 |
| 23 | GOLDEN ASYMMETRY PRESERVED | ASYMMETRY | ckernel.cpp:3361,3581 | golden :2113/:2121 vs :2286-2298 | 前排兩個Alarm-Reset區塊都會多清一項bTesterPauseMusic=false，後排對應區塊只清三項未觸及此欄位 | 使用後排面板時若正在顯示Tester Pause提示音，按Alarm Reset無法解除該音樂閂鎖 | 20260803 |
| 24 | GOLDEN ASYMMETRY PRESERVED | ASYMMETRY | ckernel.cpp:3426 | golden :2150 vs :1973 | 後排Home區塊在bAseHome為真時設定iAseHome=1，前排對應區塊卻設定iAseHome=2 | 遠端ASE Home訊號送達時，回報的iAseHome值會依當時哪個面板作用中而不同 | 20260803 |
| 25 | GOLDEN BUG PRESERVED(headline) | BUG | ckernel.cpp:3118,3457,3698 | golden :2126,:2176,:2181,:2382-2404 | 後排按鍵掃描把前排id透過加上SnRKPowerOff轉成後排感測器索引，但後排Power-Off/Power-On本身已直接回傳後排id，被二次偏移成28/29且release sweep對錯感測器 | 長按後排Power-Off/Power-On按鍵會出現規律性的自動連發，而非正常單次觸發後保持 | 20260803 |
| 26 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:3609 | golden :2314-2340 | SLK解夾按鈕的bFlag看似按下邊緣閂鎖，實際上按住時每次掃描都在true/false間切換，真正節流靠另一個3秒計時器 | 長按解夾按鈕會讓夾持氣缸每約3秒切換一次，永無止盡地循環 | 20260803 |
| 27 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:3841 | golden :2434,:2459,:2464 | GetMotorAlarmCode內的static int iCount只有寫入，全函式都沒有任何地方讀取或遞增它 | 疑似未完成的重複抑制計數器，目前完全不影響行為 | 20260803 |
| 28 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:3856 | golden :2440 | 判斷式MOT[i].Motor==Comp && MOT[i].Motor->Enable沒有對Comp做NULL檢查 | 若Comp為NULL且巧合匹配未填入的MOT[i].Motor會造成空指標解參考，目前實務上不會觸發 | 20260803 |
| 29 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:3874 | golden :2451 | JCET客戶碼下的格式字串寫成iCcwLed%d，缺少相鄰欄位都有的冒號 | WAR240004警報記錄裡該欄位格式與其他欄位不一致，為不破壞log解析而保留 | 20260803 |
| 30 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:3899 | golden :2468-2469,:2488-2489 | GetErrorIndex()回傳9(無法辨識的錯誤)時被remap成7(伺服警報)，Galil分支重複同一remap | 操作員看到的最終警報代碼可能與馬達實際診斷結果不符 | 20260803 |
| 31 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:3918 | golden :2474,:2476,:2509 | 第二段Galil Index警報並未限制在MOT[i].Motor==Comp才執行，且在ProcessAlarm剛清SoftStop後又設回true | 通報任一馬達警報時只要Galil index警報閂鎖著就會連帶再次觸發SoftStop | 20260803 |
| 32 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:3945 | golden :2494 | 在一個void函式尾端留有一行已註解掉的return 0，是函式曾回傳int時代的殘留死碼 | 純屬視覺雜訊，無行為影響 | 20260803 |
| 33 | GOLDEN BUG PRESERVED(headline) | BUG | ckernel.cpp:4033 | golden InitialTestDelayStatus 11個if | 用11個獨立if(非else-if鏈)依序覆寫str，優先順序等於原始碼順序反過來，且15格陣列只測試11格 | 多延遲旗標同時成立時顯示的是最後一個判斷式，索引0/12/13/14的延遲原因永遠不顯示文字 | 20260803 |
| 34 | GOLDEN BUG PRESERVED | BUG | ckernel.cpp:4085 | golden :2564 | 顯示字串把First拼成Fisrt，是操作員畫面上可見的錯字 | 純文字錯字，因既有回歸測試比對字串內容而刻意保留 | 20260803 |
| 35 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.cpp:4102 | golden :2579,:2584 | 其餘九個延遲狀態字串都有Initial delay--前綴，唯獨最後兩個沒有 | 畫面顯示的延遲原因文字格式不一致，不影響邏輯行為 | 20260803 |

TOTAL(本檔): 35


### ckernel.cpp 補充（承續上表編號，續 36-38；來源：orchestrator 補撈罕見詞彙掃描）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 36 | GOLDEN SPELLINGS KEPT | SPELLINGS | ckernel.cpp:799-805 | golden cmydef.h:4688,:3332 | `bYpitchNeddHome`（Nedd）與 `bIndexCheckNoStopVaccum`（Vaccum）是 golden 自己的錯字，且是全樹沿用的真實符號名稱 | 純命名瑕疵但具承重性，不能重新命名，否則會破壞其他讀取這兩個變數的地方 | AI(W906-W7-L2) 20260803 |
| 37 | GOLDEN TYPO PRESERVED | TYPO | ckernel.cpp:3179-3182 | golden Config.h:154 | 設定欄位拼成 `bDisibleResetButton`（Disible），golden 與 port 的 Config.h 皆保留此拼字 | 純命名瑕疵，重新命名會破壞同一 struct 的其他讀取者 | AI(W906-W7-L2) 20260803 |
| 38 | GOLDEN DEAD CODE PRESERVED | DEAD CODE | ckernel.cpp:3776-3783 | golden :2411,:2413,:2417,:2421,:2425 | `DoScanLevelSensor` 的 `bFlag` 初始化為 `false` 後全函式從未被重新賦值，使四個分支開頭的 `bFlag==true \|\|` 判斷恆假、每個分支退化成純感測器測試 | 是刻意保留的「強制覆蓋開關」骨架，翻轉初始值會讓函式恆回傳最高優先權，故保留初始化式本身也不刪除 | AI(W906-W7-L2) 20260803 |

### ckernel.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY PRESERVED | ODDITY | ckernel.h:45,84 | golden ckernel.h:10,:13 | bool SendTrayBufferIsFull()在golden被逐字重複宣告兩次 | C++語法上重複宣告合法且無副作用，純屬golden的冗餘 | 20260803 |

TOTAL(本檔): 1

### cinitial.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ASYMMETRY KEPT, NOT "FIXED" | ASYMMETRY | cinitial.cpp:16087 | InitialHT9045SModule USE_PICKER_COUNT==0分支(自20161117) | 該分支停用MOT[MInArmZC/ZD/ZG/ZH](注意是ZD、ZH)但實際停用噴嘴是兩列第1、3欄，下方ep1Picker分支用規則的ZB..ZH全集，兩者對不上疑ZD/ZH為ZE/ZF之誤植 | 若確為誤植，HT-9045S/1-picker機型停用的軸與被關閉噴嘴欄位可能不完全對應 | 20260810(W906-PT-W6-n5) |

TOTAL(本檔): 1

### cinitial.cpp QUIRK 二輪補掃（承續上表編號，續 2-7；來源：orchestrator 對本檔全樹重跑 `grep -inw quirk`，補齊第一輪 grep 樣式未涵蓋的 4 個 QUIRK/DEFECT banner，橫跨 3 個函式）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 2 | TWO GOLDEN QUIRKS KEPT 項1 | QUIRK | cinitial.cpp:14378-14382（`SetTechDataToProd_Tray`） | golden :8513-8519 vs :8537-8555 | `TrayForm.LodareType==0` 分支對 `UserDefForm[iTrayType[0]]` 做 memcpy 時沒有 0..3 範圍夾限，else 分支則透過 `iTypeBuffer` 先夾限每個索引才用 | 若 `TrayForm.Loader.iTrayType` 超出範圍且 `LodareType==0`，golden 本身就會讀取越界記憶體 | 20260810(W906-PT-W6-n5) |
| 3 | TWO GOLDEN QUIRKS KEPT 項2 | QUIRK | cinitial.cpp:14383-14384 | golden（`cprod.h` 欄位本身） | golden 把欄位名拼成 `LodareType`（應為 Loader），此樹的 `cprod.h` 也確實使用這個拼法 | 純拼字沿用，非本次翻譯引入 | 20260810(W906-PT-W6-n5) |
| 4 | GOLDEN DEFECT, PRESERVED AND REPORTED（未編號） | DEFECT | cinitial.cpp:14518-14528（`SetTechDataToProd_InArm`） | golden :8786（對照 :8745-8752,:8749） | `Prod.YInArm_Plate1_Pick[i][j]=Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase]+InArmOffSet[InOfsHP2]->GetArmY(i, j);` 位於 HotPlate-2 的 Y 扇出迴圈內，卻用 Plate2 的基準與 offset 表寫入 Plate1 | 前一個迴圈(:8745-8752)算好的 HotPlate-1 每噴嘴 Y 值會被 HotPlate-2 幾何覆蓋，且 Plate2 自己的每噴嘴 Y 扇出從未真正被寫入；在 HP1/HP2 的 Y 不同且噴嘴數 >1 的機台上是真實的定位缺陷，本波次僅忠實翻譯、不修正 | 20260810(W906-PT-W6-n5) |
| 5 | GOLDEN TYPE QUIRK KEPT（未編號） | QUIRK | cinitial.cpp:14530-14533 | golden :8578,:8677-8681 | `dbTrayThick` 宣告為 `double`，卻與 int 常數 600 比較、重設為 `600.0`，再被加進一個 int 欄位（`+=dbTrayThick-635`），賦值當下會朝零截斷 | 逐字保留型別與截斷行為，不「清理」型別 | 20260810(W906-PT-W6-n5) |
| 6 | GOLDEN QUIRK KEPT（未編號） | QUIRK | cinitial.cpp:15843-15847（`ReadTechData`） | golden :13586,:13612 | `bool ret=true;` 只在宣告時賦值一次，之後從未被重新賦值，使 golden 無論教導檔是否遺失都一律回傳 true；golden 自己註解掉的 `ReadData()` 呼叫(:13587)以註解形式一併保留，記錄該函式過去曾經做過的事 | 忠實保留，不「改進」成真正檢查讀檔結果 | 20260810(W906-PT-W6-n5) |
| 7 | GOLDEN QUIRK PRESERVED, NOT FIXED（未編號） | QUIRK | cinitial.cpp:16973-16979（`SetTechDataToProd_Yield`） | golden :11627 | golden :11627 自帶行內註解「Fix == to = (was comparison, not assignment)」，記錄這行過去是無作用的比較式，現在確實是賦值 `TestIF.iAutoClean_Function=false`；同一段程式碼內 `TestIF` 與 `TestIF_File` 混用的不一致也是 golden 自己造成的 | 逐字保留這段 golden 自我修正過的賦值，以及混用型別來源的不一致 | 20260810(W906-PT-W6-n5) |

TOTAL(本檔，含二輪 QUIRK/DEFECT 補掃): 7

### common.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG, preserved verbatim | BUG | common.cpp:1781 | CheckFileIsEmpty golden :1627-1643 | fclose(pFile)在fopen失敗(pFile==NULL)時仍無條件被呼叫，對NULL FILE*呼叫fclose屬未定義行為 | 檔案不存在時此函式仍執行一次未定義行為的fclose(NULL)，屬潛在風險點 | 20260721 |
| 2 | GOLDEN BUG (found and VERIFIED empirically) | BUG | common.cpp:1785 | CheckFileIsEmpty | 回傳值語意與函式名稱相反，true代表不存在或非空、false才代表存在且為空 | 呼叫端若望文生義以為true代表檔案是空的會得到完全相反的判斷結果 | 20260721 |
| 3 | GOLDEN BUG (found and VERIFIED via gdb) | BUG | common.cpp:1835 | ReadDataFromFile | 文字模式開檔導致CRLF轉LF使fread實際讀到位元組數少於file_size，緩衝區尾端有未初始化位元組 | 呼叫端若假設回傳緩衝區尾端內容有意義，在含換行檔案上會讀到未初始化垃圾資料 | 20260721 |
| 4 | GOLDEN BUG, preserved verbatim | BUG | common.cpp:2544 | ChangeSaveFileName golden :2124-2183 | 雙引號分支用字面token四個連續雙引號當搜尋樣式，兩相鄰空字串合併成單一空字串，此分支永遠比對空樣式而非真正的雙引號字元 | 檔名清理功能中唯獨雙引號字元從未真的被替換掉 | 20260721 |

TOTAL(本檔): 4

### csystem.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | NOTE -- GOLDEN BUG PRESERVED IN THE GATED ARM | BUG | csystem.cpp:15162,16435 | golden :16428-16436(ReadFixTrayBackupData) | golden的switch(TrayIndex)只有case 0/1/2，TrayIndex大於2時pFixTray未賦值就被使用，此段目前包在GATE G4(#if 0)內不可達 | 一旦GATE G4被解除，TrayIndex>2的呼叫會使用未初始化指標，需在解除閘門前一併處理 | 20260809(W906-PT-W5-g5) |
| 2 | GOLDEN BUGS PRESERVED VERBATIM | BUG | csystem.cpp:26319 | golden :20060-20062,:20135-20138(DoInArmAutoSiteMapping) | if(...) iSiteRow=0緊接下一行無條件iSiteRow=iAutoSiteMapInArmRow把它蓋掉，2026年的1x4_8_Hot修正在golden本身即死碼 | 1x4_8_Hot機型自動site mapping的列座標修正從未真正生效 | 20260810(group h1) |
| 3 | GOLDEN BUG kept verbatim | BUG | csystem.cpp:27124,27448 | golden :24779-24792(bScanFixPortState_Analog) | golden版本在CUSTOMER_CODE==CC_MAXIM/MAXIM_THAILAND時會呼叫自己形成無限遞迴，目前整段在GATE H5-G3(#if 0)內未啟用 | 若未來改採golden csystem.cpp版邏輯，MAXIM系客戶碼機型呼叫到此會堆疊溢位當機，目前為死碼 | 20260810(group h5) |
| 4 | GOLDEN BUG KEPT (not a gate, no #if) | BUG | csystem.cpp:27177,27659 | golden :24899-24904(DoInitialCylinderCheck case 5) | 感測未歸位時設Task=5想暫停FSM，兩行後又無條件覆寫成Task=100，此防呆在golden本身從未生效，是真正照譯的作用碼 | 推缸自檢即使偵測到感測器未歸位仍照常推進十支氣缸，此道防呆自始無效 | 20260810(group h5) |
| 5 | GOLDEN ODDITY KEPT | ODDITY | csystem.cpp:29186 | golden :22481 vs :22503 | 讀取用ini鍵名SvaveFilebyDay(拼字有誤)，寫入用SaveFilebyDay且寫死的0而非欄位值，讀寫鍵名不一致 | ESD_GENERAL.SvaveFilebyDay設定實際上讀不到程式寫出的值 | 20260810(group h3) |

TOTAL(本檔): 5

### cTemperFrom.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY | ODDITY | cTemperFrom.cpp:25,274 | golden :607(ShowThermo) | 邊界檢查Addr>=sizeof(ShowTempComp)/4假設指標為4 bytes，64-bit建置下門檻會放大一倍 | 目前呼叫端Addr皆在安全範圍內尚無實害，但64-bit下不能當作真正的邊界保護 | 20260818 |
| 2 | GOLDEN ODDITY | ODDITY | cTemperFrom.cpp:241 | ShowThermo bTemperatureReady[] | static bTemperatureReady[]全函式只被寫入從未被讀取 | 死狀態變數，目前不影響行為 | 20260818 |
| 3 | GOLDEN ODDITY | ODDITY | cTemperFrom.cpp:1295,1411 | golden :1600(ShowOffYieldFun) | strShowYield[i].bFlag==true把賦值=打成比較==，變成丟棄結果的空操作 | 停用閃爍時bFlag不會被強制恆亮，面板顏色維持上次閃爍週期殘留值而非穩定顯示紅色 | 20260818；⛔ 20261003 更新：E-034（`effe255a`，Steven Q82＝A）已改成 `=true`（0625 :1600／V912 :1610 賦值；溫控第 20a 條），`TemperFromCore` 釘住，屬 #20 例外（Steven 1003 常設規則） |

TOTAL(本檔): 3

### forms/fTemperFrom.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY -- recorded, not "fixed" | ODDITY | forms/fTemperFrom.h:122 | golden :607 | (與cTemperFrom.cpp同一缺陷的表單標頭文件版說明)ShowThermo邊界檢查假設指標4 bytes，64-bit下門檻加倍 | 目前無實害，提醒未來呼叫端勿誤信其為64-bit安全邊界 | 20260818 |

TOTAL(本檔): 1

### bthermo.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUGS / QUIRKS preserved VERBATIM | BUG | bthermo.cpp:121,3967 | golden :3457 | HeaterSVLog(tcAb1, Temp[tcBa1])把tcBa1通道的值記在tcAb1底下，是複製貼上疏漏，目前在GATE W7-UI G26b(#if 0)內未啟用 | 若解除此閘門，tcAb1/tcBa1兩通道加熱設定log會互相串線 | 20260807 |
| 2 | GOLDEN BUGS / QUIRKS preserved VERBATIM | BUG | bthermo.cpp:125 | golden :3326,:3376 | static double OldTemp[]只被讀取從未被寫入，還原上次SV的分支永遠還原成0.0 | 還原機制形同失效，SV顯示恆為0.0而非真正上次設定值 | 20260807 |
| 3 | GOLDEN BUGS / QUIRKS preserved VERBATIM | BUG | bthermo.cpp:128 | golden :1067,:1077,:3331 | asErrorAddr(兩處)與S2宣告後從未被使用 | 純屬死碼，不影響行為 | 20260807 |
| 4 | GOLDEN BUGS / QUIRKS preserved VERBATIM | ODDITY | bthermo.cpp:130 | golden :3063-3065,:3127-3129 | case 500貫穿進510、case 520貫穿進530，無break | 刻意設計的fallthrough，非缺陷 | 20260807 |
| 5 | GOLDEN BUGS / QUIRKS preserved VERBATIM | ODDITY | bthermo.cpp:132 | golden :77-79,:333-334 | int iLowBase=Temperature.fLowBase把double基準截斷成int，斜率m用截斷值計算 | 溫度曲線斜率計算精度損失，刻意不做int/float修正 | 20260807 |
| 6 | GOLDEN BUGS / QUIRKS preserved VERBATIM | ODDITY | bthermo.cpp:135 | golden :2346,:2380 | Addr/4與Addr%4使用整數截斷運算 | 既有數值語意，保留不做浮點改寫 | 20260807 |

TOTAL(本檔): 6

### TempCtrl/TriTemp.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUGS PRESERVED(B1) | BUG | TempCtrl/TriTemp.cpp:186 | golden :2853,:2905(fCheckATCTempOK) | 以[0,40)索引bATC_EnablesChannel[i]，但陣列宣告大小只有32，8個元素越界讀取 | 讀到宣告範圍外記憶體，屬潛在記憶體安全風險 | 20260807 |
| 2 | GOLDEN BUGS PRESERVED(B2) | BUG | TempCtrl/TriTemp.cpp:190 | golden :641-650(fcheckDefrostStatus) | if(bHeatingStart[i])為真後立刻在同分支內設bHeatingStart[i]=true，使下方else if條件永遠不可能不同，20分鐘逾時警戒分支形同死碼 | WAR31009加熱逾時看門狗永遠不會被觸發，該保護功能虛設 | 20260807 |
| 3 | GOLDEN BUGS PRESERVED(B3) | BUG | TempCtrl/TriTemp.cpp:197 | golden :3090(fUseColdTemperatureAutoDefrost) | ARM-2分支誤用ARM-1旗標bArm1(複製貼上未換名)，導致ARM-1觸發過後ARM-2永遠無法觸發 | 雙臂低溫自動除霜中ARM-2邏輯被ARM-1狀態鎖死 | 20260807 |
| 4 | GOLDEN BUGS PRESERVED(B4) | BUG | TempCtrl/TriTemp.cpp:201 | golden dfm sgDefrostStatus RowCount=7 | 寫入用Cells[c][i+1]索引到row 7，超出RowCount=7範圍一格，例外被外層catch(...)吞掉 | 表格最後一列狀態格永遠沒真的被更新，但不會讓程式崩潰 | 20260807 |

TOTAL(本檔): 4

### MyTempPanel.cpp（1 筆，20260825 FW-TAG1 當波入帳；R2 時為 0 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG (TAG1-a) | BUG | MyTempPanel.cpp:554-569（ctor Tag 區塊，解鎖後） | golden MyTempPanel.cpp:325-346（漏），:32/:203-214（該欄位確實建立並接上讀 Tag 的 handler） | ctor 對 19 個 TEdit 中的 18 個寫 `->Tag=iTag;`，獨漏 `edSHighBase`（2024.07.27 新增的第 6 點溫度欄），但該欄位仍 `OnMouseDown=edBaseMouseDown`——與另外 16 個共用那個讀 Tag 的 handler | 該欄位在每個通道都以 VCL 預設 Tag==0 執行；因 `tcHotPlate1==0`（MachineType.h:637），Tri-Temp 機台上它會命中 `(Tag>=tcHotPlate1 && Tag<=tcShuttle2)` 臂，鍵盤上下限取 `Temperature.fWorkTemperBase` ±30 而非該通道的 `SetHeaterTemp_Max*`——上真機前需裁決 | AI(W906-FW-TAG1) 20260825 |

TOTAL(本檔): 1

（原本的 0 筆說明保留供備查：唯一相關文字為 line 946-961，原註解自稱 GOLDEN BUG 後在同一則註解中已自我撤回並確認非真實缺陷，經查證 ShowQwertyKey 簽名為 (min,max) 且 callee 自帶正規化，不計入。該撤回仍然成立，與本列無關。）

### uHeaterThread.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUGS PRESERVED VERBATIM(A) | BUG | uHeaterThread.cpp:163 | golden :1565 | if(...)>60);多一個分號，if主體變空敘述，60秒門檻判斷完全失效 | 原本要用60秒門檻限制的邏輯區塊實際上不論是否滿60秒都會執行 | 20260807 |
| 2 | GOLDEN BUGS PRESERVED VERBATIM(B) | BUG | uHeaterThread.cpp:166 | golden :516-533 | 同一判斷條件以else if連續出現兩次，第二次永遠不可能被執行到 | 該段程式碼是不可達的死碼 | 20260807 |
| 3 | GOLDEN BUGS PRESERVED VERBATIM(C) | BUG | uHeaterThread.cpp:170 | golden :941 | TempFuseLimitType型別double卻用%d格式化，格式與引數型別不符屬未定義行為 | 產生的log字串內容可能不正確，但僅限記錄用途不影響控制邏輯 | 20260807 |
| 4 | GOLDEN BUGS PRESERVED VERBATIM(D) | BUG | uHeaterThread.cpp:173 | golden :1164,:1302,:1614 | 三處警報代碼組字漏掉姊妹寫法都會加的+100，合成出不同的2位數警報代碼 | 這三個位置產生的警報代碼與同通道其餘警報碼建構方式不一致，可能造成分類誤判 | 20260807 |
| 5 | GOLDEN BUGS PRESERVED VERBATIM(E) | BUG | uHeaterThread.cpp:178 | golden :465-479 | bShuttleCooling=true賦值後緊接if(bShuttleCooling==true)判斷式恆真，else分支形同死碼 | else分支邏輯永遠不會執行到 | 20260807 |
| 6 | GOLDEN BUGS PRESERVED VERBATIM(F) | BUG | uHeaterThread.cpp:181 | golden :1416,:535 | Ambient模式讀取dTempWorkBase，但此變數只在互斥的Hot/AmbientHot模式賦值，Ambient下永遠是0.0 | Ambient模式下的Tj超溫判斷少考慮了dTempWorkBase這個基準 | 20260807 |
| 7 | GOLDEN BUGS PRESERVED VERBATIM(G) | ODDITY | uHeaterThread.cpp:185 | golden :1125-1126,:1221-1226,:1293-1298 | 多處 `&&`/`\|\|` 混用卻未加括號，C語言預設運算子優先序被完整保留 | 純粹是可讀性議題，實際運算優先序與golden完全相同不影響行為 | 20260807 |
| 8 | GOLDEN BUGS PRESERVED VERBATIM(H) | ODDITY | uHeaterThread.cpp:188 | golden :204 | AnsiString(UN150Read[10])使用寫死的索引10，而非周圍慣用的tcCCD列舉值(數值恰好相同) | 目前數值上等價無實際影響，但硬編碼索引比列舉常數更脆弱 | 20260807 |
| 9 | GOLDEN BUGS PRESERVED VERBATIM(I) | BUG | uHeaterThread.cpp:191 | golden :166,:1116 | static bool bHeatOverStop在過熱時被設成true，但全單元中沒有任何地方讀取這個變數 | 過熱狀態被記錄下來卻從未被使用，該保護狀態形同未真正發揮作用 | 20260807 |

TOTAL(本檔): 9

### EJ1N/MyOmronPanel.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG NOTED, NOT FIXED | BUG | EJ1N/MyOmronPanel.cpp:415 | golden ctor :10-65 | golden與port建構子皆未初始化iStartX/iStartY，第一次MouseDown之前若先收到MouseMove會讀到未初始化值 | 拖曳位置計算理論上可能使用垃圾值，目前功能整體被閘門擋住尚無實害，日後接上真實視窗才會顯現 | 20260811(W906-PT-W8) |

TOTAL(本檔): 1

### uYieldMonitoring.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG (faithful, golden :5249) | BUG | uYieldMonitoring.cpp:2226,2233 | golden :5249 | Auto Head分支的(void)(bLowYieldCloseSite[k][i][j]==true)把賦值=打成比較==，整句變成被丟棄結果的空操作 | 低良率自動關站的site關閉判斷在Auto Head路徑下從未真正被更新 | 20260818 |
| 2 | GOLDEN ASYMMETRY (faithful, golden :5269) | ASYMMETRY | uYieldMonitoring.cpp:2253,2257 | golden :5269 | Auto Socket分支中只有arm-1成立時寫dYield[0]，卻只有arm-2成立時寫進dYield[1]，下方判斷都只讀dYield[0] | 只裝arm-2探針的site其dYield[0]恆為0，在低良率關站判斷中永遠被誤判為良率偏低 | 20260818 |
| 3 | GOLDEN BUG (faithful, golden :5288-5289) | BUG | uYieldMonitoring.cpp:2275,2277,2278 | golden :5288-5289 | Auto Socket分支同樣的(void)(bLowYieldCloseSite[0/1][i][j]==true)==vs=typo，同一份程式碼裡兩個statement都中招 | 低良率自動關站的site關閉判斷在Auto Socket路徑下同樣從未真正被更新 | 20260818 |

TOTAL(本檔): 3


備註：MyTempPanel.cpp原始1個hit經查證為自我撤回的假缺陷(非GATE、非genuine golden bug)，不計入73；GATE項目一律未列入(僅在原始交付訊息中提及數量，未逐一重複於此格式)。

### 【手臂周邊叢集】涵蓋：asortarm.cpp / OCRInsp.cpp / BarCode/BarCode_Shuttle1_SFCAutoTune.cpp / RotateKit/aRotateKIT_In.cpp / RotateKit/aRotateKIT_Out.cpp / CanBus/cMyNUDN1.cpp / SortingBinTray/SortingBinTray.cpp / Public/MyProductionRecord.cpp / Public/MyStringList.cpp

### asortarm.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG (B1) | BUG | asortarm.cpp:446-451(亦見251) | golden :234/:255 | int ret 未初始化，在 bHasFail 為 false 的路徑下於 if(ret==K_SKIP) 被讀取，且因 GATE(1) 使 bHasFail 恆為 false，此路徑現在每次呼叫都會發生。 | 讀取未定義值理論上可能導致不可預期分支，但因 flag[][] 恆為 0 且 Vaccum_Initial_Off==4，該分支內程式碼實際不會執行，屬於更常走到但無害的隱患。 | 20260808 |
| 2 | GOLDEN BUG (B2) | BUG | asortarm.cpp:452-456 | golden :1335/:1373 | iSortArmYPosition=iRow 誤用函式參數 iRow（於 TRAY_ARM_MODE==eUnderCoveyor 路徑上尚未賦值即被讀取），研判本意應為 itStartRow。 | 在 eUnderCoveyor 模式下，SortArm Y 座標可能被寫入未初始化或錯誤的數值，可能導致定位偏移。 | 20260808 |
| 3 | GOLDEN BUG (B3) | BUG | asortarm.cpp:457-460 | golden :2862-2863,:4753-4754 | CheckSortArmXYPitch_2x4_8 與 _2x8_8 中 X、Y 偏移量互相交叉寫入。 | 若此路徑被觸發，SortArm 對飛梭的 X/Y 對位計算會整組錯置，可能造成取放偏移或撞料。 | 20260808 |
| 4 | GOLDEN BUG (B4) | BUG | asortarm.cpp:461 | golden :3106 | iShtCol=j+iPickKitCol;; 多了一個分號，屬無效但無害的雙分號。 | 純語法瑕疵，不影響邏輯行為。 | 20260808 |
| 5 | GOLDEN BUG (B5) | BUG | asortarm.cpp:462-464 | golden :3365/:3380 | if(asCheck==0 \|\| asCheck=="") 將 AnsiString 與整數 0 比較。 | vclcompat 的 AnsiString 與 VCL 行為一致地接受此比較，故無行為差異，僅是風格瑕疵。 | 20260808 |
| 6 | GOLDEN BUG (B6) | BUG | asortarm.cpp:465-467 | golden :991-998 | if(iSortWhichAuto<=6){Task=3300;if(Task!=3300)break;} 後又重複 Task=3300，內層判斷因建構方式恆為假。 | 無實際行為影響，純屬冗餘與死碼。 | 20260808 |
| 7 | GOLDEN BUG (B7) | BUG | asortarm.cpp:468-470 | golden :709 | 一個獨立的 break; 夾在兩個被註解掉的 case 標籤之間，實際上是 case 1200 的收尾。 | 該 break 目前仍是可到達且有作用的敘述，維持原樣以保留行為正確性。 | 20260808 |
| 8 | GOLDEN BUG (B8) | BUG | asortarm.cpp:471-475 | golden :2405/:2392/:2407 | GetSortArmToUnLoaderPosition 內先呼叫 AutoCalculateSortArmXClosePitch 並立刻用 dSortArmXPitch_1Step 重新計算 iXPos，使先前相同的賦值被覆蓋。 | 前一次賦值變成死碼，實際生效值來自後面的重算，行為上巧合正確但程式碼具誤導性。 | 20260808 |

### OCRInsp.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG (banner a) | BUG | OCRInsp.cpp:810(banner174-176) | golden :408 | fOCR->bOcr_ReceiveOK[fOCR->ocrLot]==false; 把比較式當成陳述式使用，該打 = 誤打成 ==，因此完全無作用。 | ocrLot 回應旗標在 LOT 送出前不會被清除，可能導致舊的回應狀態殘留誤判。 | 20260807 |
| 2 | GOLDEN BUG (banner b) | BUG | OCRInsp.cpp:938,951,959(banner177-183) | golden :533/:543/:549 | DoOCRFlow case 100 呼叫 SetTraySingleData(iOCRPosR, iOCRPosC, HAS_OCR_NG) 時把 row/col 參數對調，而檔案內其他呼叫都是正確的 (col, row)。 | NG 標記會寫入轉置後的錯誤格位，導致 OCR 讀碼失敗的標記位置與實際格位不符。 | 20260807 |
| 3 | GOLDEN BUG (banner c) | BUG | OCRInsp.cpp:996(banner184-186) | golden :584 | iPos=asBarCode.Pos("/r/n"); 誤用正斜線而非跳脫的 CR/LF，導致 CheckDataExistAndDuplicate 的 CRLF 修剪永遠不會命中。 | Str 永遠等於整個 asBarCode 未被裁切，可能造成條碼字串比對或重複判斷失準。 | 20260807 |
| 4 | GOLDEN BUG (banner d) | BUG | OCRInsp.cpp:187-192(僅 banner) | golden :641+:99 | DoOCRFlow1 內的 static int iOCRPosition 遮蔽了全域 int iOCRPosition，case 21/22/25/26 寫入函式靜態版本，而 MoveTrayArmOCRCCDToLoaderStage 讀的是全域版本。 | 在 INSTALL_OCR_YMot!=eocrYMotInstal 情境下，Y 軸目標可能讀到過期或未更新的全域值，造成移動座標錯誤。 | 20260807 |
| 5 | GOLDEN BUG (banner e) | BUG | OCRInsp.cpp:1090(banner193-196) | golden :672/:674 | LastSet.TrayCount[iOCRLotTrayCount-1]，iOCRLotTrayCount 預設為 0，故索引為 -1，屬於潛在越界讀取。 | 目前僅因 bOCRCheck 預設為 false 而未被觸發，一旦該旗標被開啟即會發生陣列越界讀取。 | 20260807 |
| 6 | GOLDEN BUG (banner f) | BUG | OCRInsp.cpp:1434(banner197-199) | golden :1013 | Buffer.sprintf("NoIC", iOCRPosR, iOCRPosC); 格式字串沒有轉換規格卻多帶兩個參數。 | 位於 #ifdef SOFT_SIMULTE 內，本組建置未定義該巨集，故此問題目前不會被執行到。 | 20260807 |
| 7 | GOLDEN BUG (banner g) | BUG | OCRInsp.cpp:1887(banner200-204) | golden :1463-1464 | case 450 的 CHECK-MAP 重試梯寫入 iBarCodeErrorC/R=iOCRPosC/R，但 check-map 真正的游標是 iTrayCol/iTrayRow。 | 錯誤訊息回報的座標會是最後一次主掃描格而非實際失敗的 check-map 格，可能誤導維修人員排查方向。 | 20260807 |
| 8 | GOLDEN BUG (banner h) | BUG | OCRInsp.cpp:210-213(僅 banner) | golden :1042 | asBarCode.Pos("?")!=0 分支與 Pos("T")==1 / Pos("F")==1 共用同一個 Delete(1,1)，導致字串任何位置出現 ? 都會被裁掉開頭字元。 | 條碼字串內含 ? 字元時，即使不在開頭也會被誤裁切第一個字元，可能造成條碼解析錯誤。 | 20260807 |

### OCRInsp.cpp QUIRK 二輪補掃（承續上表編號，續 9；來源：orchestrator 對本檔全樹重跑 `grep -inw quirk`。註：檔頭 banner 中另外兩處 `GOLDEN QUIRK` 命中點（:493,:1056）分別是既有第 4 項"banner (d)"缺陷在其另外兩個引用點的重複標註，非新缺陷，未收錄）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 9 | INTENTIONAL FALL-THROUGH（檔頭 banner 內未單獨編字母，其餘 a-h 為既有第1-8項） | QUIRK | OCRInsp.cpp:750-756（`CheckOCRInArmSuckIC`） | golden :352-354 | `case 1:` 執行 `InArmSuck.ResetAll()` 後沒有 `break`，直接落穿進入 `case 50:` 的吸取輪詢；banner 註明這是全檔唯一一處 fall-through（其餘每個 case 都有 break，經自動掃描驗證過） | 刻意設計：同一拍內先重置整組真空再輪詢吸取狀態，逐字保留不補 break | 20260807 |

TOTAL(本檔，含二輪 QUIRK 補掃): 9

### BarCode/BarCode_Shuttle1_SFCAutoTune.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | BUG NOTE 1 | BUG | BarCode_Shuttle1_SFCAutoTune.cpp:282-287 | golden :5011-5012 | 無裝置週期的 case 1400 終止時設 Task=1300、bResult=true，1300 移動飛梭後又回到 1400，游標在 1300 與 1400 間永遠乒乓循環。 | Cycle no device end 訊息與 true 回傳會每隔一次 tick 重複發生，該週期永不真正結束。 | 20260811 |
| 2 | BUG NOTE 2 | BUG | BarCode_Shuttle1_SFCAutoTune.cpp:288-291 | golden :5267 | 有裝置掃描結束時 case 11200 設 Task=13000，但整個 switch 內沒有 case 13000。 | 游標停在無人處理的值，DoSFCAutoTune_1 永遠回傳 false，有裝置週期永遠無法回報完成。 | 20260811 |
| 3 | BUG NOTE 3 | BUG | BarCode_Shuttle1_SFCAutoTune.cpp:292-296 | golden :5272-5280 | 承 BUG NOTE 2，case 11300 與 11400 因此變成無法到達的死碼，11200 疑似應設 11300。 | 有裝置週期永遠不會把飛梭移回 Prod.OutSHT[0].iLeft。 | 20260811 |
| 4 | BUG NOTE 4 | BUG | BarCode_Shuttle1_SFCAutoTune.cpp:297-300 | golden :4706/:5018 | DoSFCAutoTune_1 的 bHasIC 參數從未被讀取，有無裝置的選擇完全由 InitialSFCAutoTune1 的游標種子決定。 | 呼叫端傳入的 bHasIC 值形同虛設，僅為簽章保留，不影響行為。 | 20260811 |
| 5 | BUG NOTE 5 | BUG | BarCode_Shuttle1_SFCAutoTune.cpp:301-306 | golden :5468 | Do2DIDCheckSh1 case 1150 non-OCR row-B 分支把 Row B 解碼字串寫入 Row A 的顯示格。 | 僅影響顯示，行為上無害，但畫面顯示錯誤欄位。 | 20260811 |
| 6 | BUG NOTE 6 | BUG | BarCode_Shuttle1_SFCAutoTune.cpp:307-314 | golden :4711/:4710/:4708/:5292/:5295/:5298/:5299/:4722 | 多個 static 或區域變數宣告後從未被讀取或使用，屬於寫入即棄置的遺留變數。 | 純冗餘，刪除亦不改變行為，保留是為了讓與 golden 的 diff 不失真。 | 20260811 |
| 7 | BUG NOTE 7 | BUG | BarCode_Shuttle1_SFCAutoTune.cpp:315-317 | golden :4718 | int pos=0, ret; 中 ret 宣告時未初始化。 | 在其唯一的讀取點（case 1160/11160）之前已被賦值，故實際無害，屬瑕疵而非現行風險。 | 20260811 |

### RotateKit/aRotateKIT_In.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_In.cpp:2018-2020 | golden :1610-1616 | 該 else if 測試 iCount==0，但前兩個分支已涵蓋所有 iCount==0 情況，故此分支永遠無法到達，本意應為 iCount==1。 | Kit1 對應的判斷邏輯實質上被跳過，可能導致特定站別 iRotato_In_Row 的取料判斷不完整。 | 20260807 |
| 2 | GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_In.cpp:2804-2806 | golden :2382-2390 | 迴圈跑到 iRotateCount==0 才停止並讀取 Prod.RotationCount[iRotateCount-1]，會讀到陣列往前一格（索引 -1）。 | 潛在陣列越界讀取，僅在 iInArmType 為特定型號或 eInOutArm1Motor 時觸發。 | 20260807 |
| 3 | GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_In.cpp:2843-2846 | golden :2414 | 一段裸表達式敘述讀取 tRotate.RotationCount[rCount-1] 後即丟棄，本身無任何效果。 | 純冗餘敘述，golden 本身也無效果，不影響行為，且此值目前又被 GATE(5) 替換成常數 0。 | 20260807 |
| 4 | GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_In.cpp:3560-3564 | golden :3117-3133（重複於 :3139/:3160/:3180） | IN 臂的 skip 路徑卻用 OutArmSuck.iMaxCol 當迴圈邊界、用 iRotato_Out_Row 當列號，再寫入 InArmSuck.PordRec/SetItemData，明顯是從 aRotateKIT_Out.cpp 複製貼上未改好。 | Skip 記錄可能寫到錯誤的行或列或使用出料側的邊界值，造成 IN 臂 duplicate-error 記錄與實際站別不符。 | 20260807 |

### RotateKit/aRotateKIT_Out.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | E-1 GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_Out.cpp:584-586 | golden :174 | 4-suck 佈局的 PICK 路徑把 Z 軸移到 iOutArmRotateToUnloader_PlaceZ（放置高度）而非 _PickZ（取料高度），而 17 行前的 1X4 佈局正確使用 _PickZ。 | 若此分支被觸發，Z 軸下降高度用的是放置點而非取料點，可能導致取料失敗或撞料。 | 20260807 |
| 2 | E-3 GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_Out.cpp:690-692 | golden :273-274 | Task=1200; 把 Task 設成它已經是的值，且不會中斷，程式會直接落入下方 MoveOutArmZ 呼叫。 | 純粹是無效賦值加上非預期的 fall-through，行為上等同於原本就會執行的下一步，尚未造成觀察得到的行為差異。 | 20260807 |
| 3 | E-6 GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_Out.cpp:728-731 | golden :305 | OUT 臂缸體版寫入的是 IN 臂用的確認陣列 bInArmCheckDestroyACT，而馬達版才寫入 bOutArmCheckDestroyACT。 | Destroy 完成確認狀態被寫到錯誤的旗標陣列，可能造成 IN/OUT 臂的 destroy-confirm 狀態互相污染或遺漏。 | 20260807 |
| 4 | E-2 GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_Out.cpp:796-799 | golden :368-379 | ==1 分支先設 Task=3100，緊接著下方無條件再設一次 Task=3100（死值覆蓋），而 ==2 分支其實是依賴那行無條件賦值才生效。 | 行為上恰好正確運作，但程式碼具有誤導性，未來若誤刪無條件賦值會連帶破壞 ==2 分支。 | 20260807 |
| 5 | E-4 GOLDEN BUG preserved VERBATIM | BUG | aRotateKIT_Out.cpp:2071-2073 | golden :1619-1621 | 該 else if 測試 iCount==0，但已被上一個 else if 消耗掉，故此分支永遠無法到達，iCount 永遠不會在此變成 2。 | 與 aRotateKIT_In.cpp 的同型缺陷對稱，特定站別的 kit 選取邏輯被跳過一段。 | 20260807 |

### CanBus/cMyNUDN1.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG (preserved, not fixed) | BUG | cMyNUDN1.cpp:148-152 | golden(HT9011UC_Code_V3.33.906.0_20260618/CanBus/cMyNUDN1.cpp):52-55 | Initail() 內宣告的 unsigned char iInitailRaw[40] 從未被寫入初始值，隨後迴圈把這 40 bytes 未定義的堆疊記憶體複製進 InAreaMonitor.iRawItem[]。 | InAreaMonitor 的初始內容為不可預測的堆疊殘留值，在 DNM100 硬體實際連線前這些值可能被誤讀為有效的 I/O 監控資料。 | 20260710 |

### SortingBinTray/SortingBinTray.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG preserved VERBATIM | BUG | SortingBinTray.cpp:1595-1600,1815-1817(banner146-150) | golden :1359-1361(CalPos),:1567-1569(HAS_NULL_IC_Pos) | SortingBinTray_CalPos 與其孿生函式 _HAS_NULL_IC_Pos 中，iStart/iEnd/iStep 宣告時未初始化，只在特定方向分支中才被賦值。 | 當 TrayData.iDirection 落在未處理的方向或 iXPitch 為 0 時，這三個變數保持未定義，while(iLine!=iEnd) 迴圈會用垃圾值走訪，可能造成無窮迴圈或越界存取。 | 20260807 |
| 2 | GOLDEN BUG preserved VERBATIM | BUG | SortingBinTray.cpp:1720-1727,1815-1817(banner151-153) | golden :1478-1479(CalPos),:1646-1647(HAS_NULL_IC_Pos) | 同兩個函式中，iPosXSuck % TrayData.iXPitch 等取餘數運算，在對應的 !=0 保護判斷之前十行就已執行。 | 未設定 pitch（值為 0）的料盤在此處會直接觸發除以零，可能導致程式當機或未定義行為。 | 20260807 |
| 3 | GOLDEN BUG preserved VERBATIM | BUG | SortingBinTray.cpp:2796-2801(banner154-158) | golden :2551-2552,:2560,:2562 | SaveTrayRecord 的 sBinData 宣告為 AnsiString[10]，但寫入保護條件是 Prod.iT6CatData[i] < eTrayCount(=33)，讀取迴圈也跑到 33，索引 10-32 越界，且 sBinData[BinSelect[...].IfErrorT3] 完全無保護。 | 陣列索引越界讀寫，量產機上若 iT6CatData 或 IfErrorT3 落在 10-32 範圍，可能造成資料毀損或當機。 | 20260807 |
| 4 | GOLDEN BUG preserved VERBATIM | BUG | SortingBinTray.cpp:2838-2841(banner159-161) | golden :2537 | SaveTrayRecord 的 iAuto==100（手動 Save Log）路徑仍會執行到 MOT[iMMAuto[iAuto]]，而 iMMAuto 是 int[eTrayCount](=33)，iMMAuto[100] 為越界讀取。 | 手動存 Log 功能觸發時會發生陣列越界讀取，讀到的錯誤索引接著被用來存取 MOT[]，風險延伸到馬達或料盤資料結構。 | 20260807 |
| 5 | GOLDEN BUG (banner) | BUG | SortingBinTray.cpp:162-163(僅 banner) | golden :2442/:2468 | SaveProductionRecord 在 golden :2442 對 fLotInfo 做了 null 檢查（僅用於檔名），卻在 :2468 無條件解參考它。 | 若 fLotInfo 為 null 時仍執行到 :2468，將造成空指標解參考（存取違規）。 | 20260807 |
| 6 | GOLDEN BUG preserved VERBATIM | BUG | SortingBinTray.cpp:965-973,1143-1147(banner164-167) | golden :741(DoPickTrayIC),:911(DoPlaceTrayIC) | 警報文字組字串用的是 Suck[iOutArmYBase][iOutArmXBase].sName，但實際被檢測真空異常的噴嘴是 Suck[iOutArmYBase][iUseSuckNum]。 | 真空異常警報可能報出錯誤的噴嘴名稱，誤導維修人員排查方向。 | 20260807 |
| 7 | GOLDEN BUG preserved VERBATIM | BUG | SortingBinTray.cpp:2520-2526(banner168-170) | golden :2261 | OutArmPickTrayAlarm 的 int ret; 未初始化，且 iwhichTray==0..5 的判斷鏈沒有 else 分支兜底。 | 當 iwhichTray 落在範圍外（如 eBulkBox=18），呼叫端會拿到不確定值去比對 K_RETRY/K_SKIP/K_HOME，行為不可預期。 | 20260807 |
| 8 | GOLDEN BUG (banner) | ODDITY | SortingBinTray.cpp:171(僅 banner) | golden :2218 | //\<;== 是拼錯的區塊結尾標記，維持原樣保留。 | 純粹是註解拼寫錯誤，不影響程式行為。 | 20260807 |
| 9 | GOLDEN BUG preserved VERBATIM | BUG | SortingBinTray.cpp:2433-2439 | golden :2182-2185 | SortingBinTray_FindSuckICStatusLine 的參數 iStatus 被寫入後從未被讀取，兩個方向迴圈都是跟字面常數 NULL_IC/HAS_NULL_IC 比較，不是跟 iStatus 比較。 | 唯一呼叫端傳入的 HAS_IC 引數與 MyParameter.iICNum 覆寫值會被整個忽略，功能上等於死參數。 | 20260807 |

### Public/MyProductionRecord.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG PRESERVED VERBATIM | BUG | MyProductionRecord.cpp:1391-1401 | golden(HT9011UC_Code_V3.33.906.0_20260618/Public/MyProductionRecord.cpp):851-887 | SaveDataForGreatek 的 iSortData[eDataTotal]（今日 eDataTotal==71）只給了 43 個初始化值，其餘 28 個元素被語言自動補 0，對應到 asBuffer->Strings[0]（schedule name）。 | 每一列 Greatek 輸出記錄的尾端會重複寫入 28 次 schedule name，而 eHotPlateForm(=42) 之後新增的欄位完全不會被輸出。 | 20260811 |
| 2 | GOLDEN BUG NOTED | BUG | MyProductionRecord.cpp:1497-1505 | golden :1001/:1024 | SaveRecordCleanPad 的兩個重試分支寫入 asBuffer->CommaText（完整 71 欄記錄），但檔案的表頭與首次寫入用的是 18 欄的 asDataTitleCleanPad 版面。 | 一旦此方法被解除 GATE 上線，重試那一列的欄位數會跟檔案既有欄位對不上，造成 CSV 或 log 格式錯亂。 | 20260811（推定，鄰近 1391/1611 同波次） |
| 3 | TWO GOLDEN BUGS PRESERVED VERBATIM (1) | BUG | MyProductionRecord.cpp:1611-1617 | golden :1092 | SaveRecordForLeadYo 在 WriteProdLOG==NULL 才會進入的 else 分支內呼叫 fclose(WriteProdLOG)，即 fclose(NULL)，屬未定義行為。 | 在此路徑下呼叫會在此樹鎖定的 MSVCRT 上直接當機；姊妹函式 SaveRecordCleanPad 的同一行在 golden 中被註解掉，顯示曾有人修過但只修了一半。 | 20260811 |
| 4 | TWO GOLDEN BUGS PRESERVED VERBATIM (2) | BUG | MyProductionRecord.cpp:1611,1618-1620 | golden :1107 | SaveRecordForLeadYo 最後的 fclose(WriteProdLOG) 沒有 NULL 保護，對照 SaveRecord 對應處 :770 有 if(WriteProdLOG!=NULL) 保護。 | 同一個 NULL 指標可能透過第二條路徑到達未保護的 fclose，同樣有當機風險。 | 20260811 |
| 5 | GOLDEN BUG NOTED | BUG | MyProductionRecord.cpp:1767-1777 | golden :1252/:1256 | Save2DIDForJCET 最後一個 catch 分支寫入 asBuffer->CommaText（71 欄完整記錄而非 15 欄白名單列），並用錯誤的函式名稱字串與錯誤的路徑變數回報錯誤，明顯是從 SaveRecordForLeadYo 複製貼上未改完全。 | 一旦解除 GATE 上線，此例外分支寫出的記錄格式與檔案欄位不符，且錯誤 log 會誤指是另一個函式出錯，混淆除錯方向。 | 20260811（推定，鄰近 1611 同波次） |

### Public/MyStringList.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | TWO GOLDEN BUGS preserved VERBATIM (1) | BUG | MyStringList.cpp:963-966 | golden :695-698 | MyInsertToFile 在目標檔案尚不存在時，只重新計算 sLastFileName 就直接 return，完全沒有寫入 Msg。 | 每個新 log 檔案的第一行警報訊息會被無聲丟棄，等同該筆記錄從未落地。 | 20260807 |
| 2 | TWO GOLDEN BUGS preserved VERBATIM (2) | BUG | MyStringList.cpp:963,967-972 | golden :735 | catch 處理常式解參考全域 slEventLog（slEventLog->Text）沒有 NULL 保護，且此類別從不對自身 base TStringList Add 內容（都寫進內部的 MyList），故 ->Text 實際上恆為空字串。 | 若 slEventLog 為 NULL，本應處理例外的 catch 反而會引發存取違規；即使不是 NULL，記錄下來的訊息內容也永遠是空的。 | 20260807 |
| 3 | FOUR GOLDEN BUGS preserved VERBATIM (1) | BUG | MyStringList.cpp:1104-1108 | golden :945/:874 | if(iCol>0) 誤判，iCol 是 0-based 的比對列索引，故第一列（i==0）命中與完全沒命中無法區分，命中的更新會被當成沒命中而丟棄。 | 每個 2D-ID 對應檔案中，第一顆 IC 的資料永遠無法被寫回檔案。 | 20260807 |
| 4 | FOUR GOLDEN BUGS preserved VERBATIM (2) | BUG | MyStringList.cpp:1104,1109-1112 | golden :927-940 | str3 重建迴圈寫在列迴圈外面，導致沒有任何列命中時序列化的是 list2D 最後載入的內容（檔案最後一列），而非真正比對到的列。 | 因為與 bug(1) 的 iCol>0 測試同樣失敗而暫時無害，但兩者疊加使錯誤更難察覺與除錯。 | 20260807 |
| 5 | FOUR GOLDEN BUGS preserved VERBATIM (3) | BUG | MyStringList.cpp:1104,1113-1118 | golden :872 | list2D->Strings[0] 在檢查 Count 之前就被讀取，BCB6 原版對空白或空行會丟出 EStringListError，而 vclcompat 的 TStringList::GetString 對越界改為回傳空字串而非拋例外。 | 屬於移植框架與原版行為的落差：golden 遇到此情況會丟例外，port 版則靜默降級為無比對命中，此差異被記錄但未修補。 | 20260807 |
| 6 | FOUR GOLDEN BUGS preserved VERBATIM (4) | BUG | MyStringList.cpp:1104,1119-1122 | golden :959 | 當對應檔案尚不存在時，File 從未被載入，File->SaveToFile(sFileName) 會寫出一個空檔案，把原本要寫入的 s2DID 整筆丟棄。 | 對一個全新客戶或全新批次首次寫入 2D-ID 對應檔時，該筆資料會完全遺失。 | 20260807 |

### Public/MyStringList.cpp QUIRK 二輪補掃（承續上表編號，續 7-10；來源：orchestrator 對本檔全樹重跑 `grep -inw quirk`，補齊第一輪 grep 樣式未涵蓋的 4 個 QUIRK banner）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 7 | GOLDEN QUIRK preserved VERBATIM（未編號） | QUIRK | MyStringList.cpp:359-365（`SetLotData`） | golden :118-137 | `SetLotData` 把 `SystemYear`/`SystemMonth`/`SystemDate` 格式化進 ByLotID 資料夾路徑，卻沒有先呼叫 `GetTimeInfo()`，用的是上一次 Add*/MySave*/GetFileName 呼叫留下的值（全新物件則是 0/0/0，變成路徑「0000\00\00」） | 本檔其餘每個方法都會先呼叫 `GetTimeInfo()`，唯獨這個不會 | 20260807 |
| 8 | GOLDEN QUIRK preserved VERBATIM（未編號） | QUIRK | MyStringList.cpp:388-394（`AddText`/`AddTextWithDateTime*` 家族） | golden :143-144 | `if(FileName=="") FileName="";` 在四個 Add* 方法裡都是無作用陳述句；另外 `AddText` 用 `>=HTMaxLineCount`（加入後才 flush），三個 `AddTextWithDateTime*` 變體卻用 `>HTMaxLineCount`（加入前就 flush），golden 本身在此不對稱 | 保留是為了與 golden 逐行對應，移除不會改變行為但會破壞對應關係 | 20260807 |
| 9 | GOLDEN QUIRK preserved VERBATIM（未編號） | QUIRK | MyStringList.cpp:905-910（`GetLastLine`） | golden :648-651 | 檔案先用 `CreateFile(GENERIC_READ)` 開一次純粹當存在性/鎖定探測，立刻 `CloseHandle`，再用 `std::ifstream` 重新開檔計算行數；上一行已經呼叫過 `FileExists`，這次探測看似多餘 | 這個探測是唯一能讓「檔案被鎖定」時跳過計數（而非回傳 0）的機制，是行為而非死碼，保留 | 20260807 |
| 10 | GOLDEN QUIRK preserved VERBATIM（未編號） | QUIRK | MyStringList.cpp:1032-1038（`MySaveSGJamCountToFile`） | golden :750 | 提早返回的判斷式是 `HTAutoSave==false && bDelete==false`（AND），本檔其餘每個存檔方法用的都是 OR，使刪除請求即使 AutoSave 關閉也一定會執行；沿用自姊妹方法的註解「沒資料就不用存檔」讓這行讀起來像寫錯 | 對刪除路徑而言是刻意行為；另外區域變數 `Word Date;` 遮蔽了 vclcompat 全域的 `Date()` 函式，無害但與 BCB6 SysUtils 的 `Date()` 同名巧合 | 20260807 |

TOTAL(本檔，含二輪 QUIRK 補掃): 10

### 【SECS/GEM 叢集】涵蓋：SECSGEM/uHGemClass.cpp / SECSGEM/uHGemHT9045.cpp / SECSGEM/uHGemEquipment.cpp(+.h) / SECSGEM/SecsWireCodec.cpp / SECSGEM/SecsSvEcRegistration.h

### SECSGEM/uHGemClass.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG DISCOVERED AND PRESERVED VERBATIM | BUG | SECSGEM/uHGemClass.cpp:320（另見:3043） | golden :1988 | S6F18_AnnotatedEventReportData 的「CEID 不存在」錯誤分支呼叫 InitLocalHead(6,16,0)，S,F 應為 6,18 卻沿用 S6F16 表頭，屬複製貼上遺留 | Host 收到的 SECS 表頭與實際內容不符，可能被誤判為 S6F16 事件回報 | AI(W906-SvEcDataItem) 20260720 |
| 2 | GOLDEN BUG DISCOVERED AND PRESERVED VERBATIM | BUG | SECSGEM/uHGemClass.cpp:441（另見:3712,3726,3732） | golden :2487／:2493 | S101F4_CurrentEPPDData 不論提前返回或成功路徑都呼叫 InitLocalHead(101,2,0)，全函式從未送出自己的(101,4,0)表頭，抄自緊鄰的 S101F2 | Host 收到的 S101F4 回覆表頭其實是 S101F2 格式，可能造成交易配對或訊息序號誤判 | AI(W906-uHGemClass-Micro5) 20260721 |
| 3 | GOLDEN BUG PRESERVED VERBATIM -- DOUBLE ACKNOWLEDGE | BUG | SECSGEM/uHGemClass.cpp:565（另見:3756,3776,3803,3872） | golden :2579 + :2778-2787 | S101F6_StoreHostUploadFile 成功路徑內部已送出 LocalAcknowledge(101,6,0)，呼叫者 HTGem::S101F6() 之後又無條件再送一次相同確認 | Host 對同一筆 S101F6 上傳會收到兩次相同接受確認，可能造成上位系統交易計數重複觸發 | AI(W906-uHGemClass-Micro6/Micro7) 20260721 |
| 4 | GOLDEN BUG DISCOVERED AND PRESERVED VERBATIM | BUG | SECSGEM/uHGemClass.cpp:1721 | golden :937-964 | S2F24_TraceInitializeAcknowledgeSub 的 UINT_4_TYPE 分支中`P=new unsigned[len]`只在失敗路徑才 delete[]，成功路徑完全未釋放 | 每次成功的 Trace SVID 清單請求都會洩漏一小塊記憶體，長時間高頻呼叫會緩慢增長 | AI(W906-uHGemClass-TraceUnlock) 20260728 |
| 5 | PRESERVED GOLDEN BUG | BUG | SECSGEM/uHGemClass.cpp:4914 | golden CheckECValue 內兩個相鄰 else if(Type==FT_4_TYPE)分支 | CheckECValue 出現兩個條件相同的 FT_4_TYPE 分支，第二個內部用 double 顯然原意是 FT_8_TYPE，但條件從未改對，成為死碼 | 無實際功能影響(FT_8_TYPE 由下一個正確分支處理)，僅顯示原始碼維護疏漏 | AI(W906-uHGemClass-Unlock) 20260713 |
| 6 | GOLDEN INVARIANT | INVARIANT | SECSGEM/uHGemClass.cpp:3468 | golden S10F4/S10F6 自身設計(無額外行號引用) | TerminalMemoPtr/TerminalListboxPtr/TerminalEditPtr/TerminalPanelPtr(及其"2"版本)在 TerminalDisplayIndex(2)==1..4 時被直接解參照、完全無 NULL 檢查，golden 隱含規則是「設定 TerminalDisplayIndex 非零者必須已接好對應指標」 | 若呼叫端或測試設定 TerminalDisplayIndex 非零卻未接好對應指標，golden 與 port 都會在此當機，屬刻意保留的設計行為 | UNCERTAIN，鄰近波次推定 AI(W906-uHGemClass-Micro5) 20260721 |
| 7 | GOLDEN QUIRK PRESERVED VERBATIM (S5F6_ListAlarmData) | QUIRK | uHGemClass.cpp:389-393,2679-2686,2820-2830 | golden uHGemClass.cpp:1751-1788(具體:1754) | alarm 找不到分支呼叫 DataItemOut(0, BINARY_TYPE, \&Mode) 傳送長度為0，Mode 的值(0x80)實際上從未送上線 | Host 收到的 S5F6 alarm mode 欄位永遠是空值，若下游依賴該欄位判斷 alarm 狀態可能誤判 | 20260721 |
| 8 | golden quirk (32-bit truncating cast) | QUIRK | uHGemClass.cpp:394-398,2686,2787 | golden uHGemClass.cpp:1720 | uint8SV 宣告為64-bit，golden卻先用(unsigned)強轉strtoull結果只保留32-bit低位再存入 | alarm ID的UINT_8_TYPE數值超過32-bit範圍時，S5F6回報給Host的識別碼會被截斷成錯誤數值 | 20260721 |
| 9 | GOLDEN QUIRK preserved verbatim (S6F24 spool-wipe del指令) | QUIRK | uHGemClass.cpp:496-507,3146-3153,3169 | golden uHGemClass.cpp:2053-2079(~:2067)；同型缺陷亦見uHGemEquipment.cpp:4102/6144 | del 指令字串在\*.\*與/q/f之間缺空格，/Q /F被cmd.exe視為同一參數的一部分而非獨立switch | spool清除時「安靜、強制」效果可能失效，殘留檔案或行為與預期不符 | 20260721 |
| 10 | GOLDEN QUIRKS preserved verbatim (S7F18 bool→int比較) | QUIRK | uHGemClass.cpp:508-511,3236-3237 | golden uHGemClass.cpp:2115-2166 | DeleteDirectory的bool回傳值指派給int後以==0比較而非==false，純風格差異 | 無實際行為差異，僅程式風格保留 | 20260721 |
| 11 | GOLDEN QUIRKS preserved verbatim (S7F18 guard提早return) | QUIRK | uHGemClass.cpp:512-517,3238-3239,3272-3276 | golden uHGemClass.cpp:2143-2147 | 一次刪多個PPID時若「使用中工作檔」排在第一個，立即回覆並return，清單後續PPID完全不會被檢查 | Host以為整批已處理，實際上排在後面的PPID從未被嘗試刪除，兩端認知可能不一致 | 20260721 |
| 12 | GOLDEN QUIRKS preserved verbatim (S7F18 非ASCII item跳過) | QUIRK | uHGemClass.cpp:3240-3241,3266-3290 | golden uHGemClass.cpp:2115-2166 | for迴圈遇到非ASCII_TYPE的list item時計數器仍前進，但既不消耗資料也不報錯 | 該筆刪除請求被靜默忽略且無任何回報，wire游標認知可能跟著偏移 | 20260721 |
| 13 | GOLDEN QUIRK (ParseSECSDateTimeString) | QUIRK | uHGemClass.cpp:835-837 | golden uHGemClass.cpp:1182-1200(函式本身:1095-1216) | 曆法不合法(如月=13)時只是不設定bApplyClock，卻不像時分秒超界那樣把bRangeError設為true | S2F32對不合法日曆日期仍回覆「接受」的DTACK碼，可能誤導Host判讀 | 20260728 |
| 14 | GOLDEN QUIRK PRESERVED VERBATIM (S2F24Sub不對稱) | QUIRK | uHGemClass.cpp:1731-1739 | golden uHGemClass.cpp:886-900 | TOTSMP欄位len!=1回傳1(視為接受)，REPGSZ欄位同樣情況卻回傳-1(觸發格式錯誤)，兩段結構相同的邏輯處理不一致 | 對相同類型的格式錯誤，Handler回覆Host的行為不同，增加除錯與重送邏輯的不確定性 | 20260728 |
| 15 | GOLDEN QUIRK preserved verbatim (S1F1 W-Bit=0) | QUIRK | uHGemClass.cpp:919-923 | golden uHGemClass.cpp:89-95 | S1F1本身是request，golden卻把InitLocalHead的W-Bit參數寫死為0 | 依SECS-II協定語意可能影響部分Host端stack判斷是否需回覆本訊息，原樣保留未修正 | 20260720 |
| 16 | GOLDEN QUIRK preserved verbatim (S2F16無NULL guard) | QUIRK | uHGemClass.cpp:1626-1631 | golden uHGemClass.cpp:731-781 | 呼叫HGemPtr->MoveCheckCallBack前只檢查callback指標本身，未對HGemPtr做NULL guard | 若在無真實THGem實例情境下呼叫會直接NULL dereference，golden既有前提被保留 | 20260723 |
| 17 | golden quirk / PRESERVED GOLDEN QUIRK (S2F42 else分支缺InitLocalHead) | QUIRK | uHGemClass.cpp:127-129,2569-2574 | golden uHGemClass.cpp:1523-1574 | if分支呼叫DataItemOut前有InitLocalHead，else(格式錯誤)分支卻直接呼叫DataItemOut，未重設表頭 | 格式錯誤回覆封包表頭可能沿用前次訊息殘留設定，Host可能收到表頭錯亂的回覆 | 20260713 |
| 18 | GOLDEN QUIRK, FLAGGED NOT FIXED (S101F6路徑無清洗) | QUIRK | uHGemClass.cpp:581-590,3793-3802,3829-3831 | golden uHGemClass.cpp:2519-2581 | 200 byte的wire檔名直接串接UpLoadPath組成fopen路徑，無長度/字元集/".."/絕對路徑檢查 | 異常或惡意Host端輸入可能讓fopen目標跳脫UpLoadPath目錄 | 20260721 |
| 19 | GOLDEN QUIRK, FLAGGED NOT FIXED (S101F8路徑無清洗) | QUIRK | uHGemClass.cpp:581-590,3920-3924,3947-3949 | golden uHGemClass.cpp:2597-2639 | PathName與檔名皆為200 byte的wire輸入，直接餵入MyForceDirectories與fopen，同樣無清洗 | 與S101F6同類風險，可能在CurrentDirectory之外建立目錄或寫入檔案 | 20260721 |
| 20 | GOLDEN QUIRK (S125F2混用HGemPtr->/HGem->) | QUIRK | uHGemClass.cpp:3980-3989 | golden uHGemClass.cpp:2643-2680(:2653/:2664) | 同一函式內部分行用HGemPtr->、部分行用全域裸指標HGem->存取相同wire primitive | 正式部署下兩者指向同一活體THGem故無行為差異，但增加維護與測試時的混淆風險 | 20260721 |
| 21 | PRESERVED GOLDEN QUIRK (SetECValue IsVCL==2 bare return) | QUIRK | uHGemClass.cpp:4057-4067,4107-4110 | golden uHGemClass.cpp:2718-2721 | AnsiString*註冊的EC在SetECValue中直接return、從未寫回，但讀取路徑卻能正常讀出這類EC | 透過S2F15/SetECValue更新這類EC的值會被靜默丟棄，Host誤以為更新成功但值從未改變 | 20260721 |
| 22 | PRESERVED GOLDEN QUIRK (S2F15 ASCII分支Str洩漏) | QUIRK | uHGemClass.cpp:4287-4291,4370-4375 | golden uHGemClass.cpp(~:2955-2960) | ASCII型EC更新以new char[len+100]配置緩衝區，golden從未delete[]該記憶體 | 每次以ASCII型別更新EC都造成一次記憶體洩漏，高頻更新場景可能逐漸耗盡記憶體 | 20260716 |

### SECSGEM/uHGemHT9045.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUGS PRESERVED, NOT FIXED (a) | BUG | SECSGEM/uHGemHT9045.cpp:530 | golden :457-460 | AddReprot 迴圈變數 i 跑遍整個 CEID 範圍，卻寫死傳字面值 1 給 SetReportIDContent，ReportID 1 被重複定義約 292 次 | 事件對應的 ReportID 對照表實質全指向同一 ReportID，破壞逐一對應設計 | ~20260809 |
| 2 | GOLDEN BUGS PRESERVED, NOT FIXED (b) | BUG | SECSGEM/uHGemHT9045.cpp:534 | golden :445 | AddCEID 旁「Ceid=1，有1個ReportID」的舊註解已與程式碼(CEID 實為變數 i)不符 | 純文件缺陷，可能誤導後續維護者對此函式邏輯的理解 | ~20260809 |
| 3 | GOLDEN BUGS PRESERVED, NOT FIXED (c) | BUG | SECSGEM/uHGemHT9045.cpp:537 | golden :409/:413/:415 | AddAlarmList 在 GetAlarmCodeList 調整列數之後、ReadAlamData 可能再次調整之前鎖存 AlarmCount，迴圈用過期上限 | 若上限超出實際範圍，port 端 vclcompat::TStringGrid 會丟 std::out_of_range 直接中止，golden 是 VCL 例外對話框或靜默增長 | ~20260809 |
| 4 | GOLDEN BUG PRESERVED | BUG | SECSGEM/uHGemHT9045.cpp:2276 | golden :5350 | S7F6_ProcessProgramData 在 PPID 目錄不存在時直接 return，golden 從未 delete 先前 new 出的 TempString2 | 每次請求不存在的 PPID 目錄就洩漏一個 TStringList 物件 | AI(W906-uHGemHT9045-S7program) 20260809 |
| 5 | GOLDEN BUG PRESERVED | BUG | SECSGEM/uHGemHT9045.cpp:2305 | golden :5602 | S7F6_ProcessProgramData 在資料格式錯誤呼叫 S9F7_IllegalData 的路徑上同樣未刪除 TempString2 | 格式錯誤請求越頻繁，記憶體洩漏累積越快 | AI(W906-uHGemHT9045-S7program) 20260809 |
| 6 | GOLDEN BUGS PRESERVED, NOT FIXED (a) | BUG | SECSGEM/uHGemHT9045.cpp:2931（另見:3880,3892） | golden :4280-4281/:4292-4293 | S5F6 將 4-byte ALID 以 INT_8_TYPE(8 bytes)送出，堆疊多讀4個位元組，高32位元垃圾值一併送上線路 | Host 收到的 Alarm ID 高位元是未定義堆疊殘值，可能造成告警 ID 判讀錯誤 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 7 | GOLDEN BUGS PRESERVED, NOT FIXED (b) | BUG | SECSGEM/uHGemHT9045.cpp:2944（另見:3889） | golden :4290 | 未知警報分支 DataItemOut(0,BINARY_TYPE,&ALCD)長度給0，ALCD 從未真正送出 | Host 收到的未知警報項目缺 ALCD 欄位內容 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 8 | GOLDEN BUGS PRESERVED, NOT FIXED (c) | BUG | SECSGEM/uHGemHT9045.cpp:2948 | golden :4215 | 已驗證的 Type 被重新讀取一次，若第二次型別在六種數值型別外，ret 仍保留舊值，導致索引到未配置的指標 | 特定型別序列下可能存取未配置指標造成當機或未定義行為 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 9 | GOLDEN BUGS PRESERVED, NOT FIXED (d) | BUG | SECSGEM/uHGemHT9045.cpp:2953（另見:3992） | golden :4201/:4393 | 首次讀取失敗時直接送出本地緩衝殘留舊資料，而非回覆 S9F7_IllegalData | 格式錯誤的 S5F6 請求會讓 Host 收到內容不明、可能是上次殘留資料的錯誤回覆 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 10 | GOLDEN BUGS PRESERVED, NOT FIXED (e) | BUG | SECSGEM/uHGemHT9045.cpp:2958 | golden :702/:1087 | ECID 宣告未初始化，EClen==0 時迴圈後仍讀取未定義值 | 目前被閘控在未上線功能區塊([E8])，暫無實際可觸發路徑 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 11 | GOLDEN BUGS PRESERVED, NOT FIXED (f) | BUG | SECSGEM/uHGemHT9045.cpp:2963 | golden :777/:789 vs :623-633 | S2F15_Update 的 ASCII 分支有 delete[]，幾乎同構的 S2F15_Check 卻每次呼叫洩漏，golden 甚至留著被註解掉的修正 | S2F15_Check 每次呼叫都洩漏字元陣列記憶體 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 12 | GOLDEN BUGS PRESERVED, NOT FIXED (g) | BUG | SECSGEM/uHGemHT9045.cpp:2967 | golden :1035-1039 | `iArry[k]=temp;k++;`對40-int陣列寫入無邊界檢查，資料源自host逗號清單 | Host送超過40個站點值會造成堆疊陣列越界寫入，屬外部輸入可觸發的風險 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 13 | GOLDEN BUGS PRESERVED, NOT FIXED (h) | BUG | SECSGEM/uHGemHT9045.cpp:2972 | golden :853-866 | case 1513 的`else return -1;`使外層for中途放棄，之前套用的不回滾、之後項目不處理 | Host一次送出的EC批次可能只套用一部分且無回滾機制 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 14 | GOLDEN BUGS PRESERVED, NOT FIXED (i) | BUG | SECSGEM/uHGemHT9045.cpp:2976 | golden :539/:685 | iECID同樣未初始化，但:519的`EClen<1→return -1`保證迴圈至少執行一次，與(e)不同實際不會讀到未定義值 | 目前無實際風險，僅為與(e)對稱的程式碼形狀記錄 | AI(W906-PT-W5-S2F15-S5-S14) 20260809 |
| 15 | GOLDEN BUGS PRESERVED, NOT FIXED (j) | BUG | SECSGEM/uHGemHT9045.cpp:2980（另見:4181） | golden :6183-6207 | LocalAcknowledge放在讀取成功區塊內，讀取失敗時Host完全收不到S125F4回覆，HCACK仍為0 | 格式錯誤的S125F4請求得不到任何回應，易被上位系統誤判逾時 | AI(W906-PT-W5-S2F42) 20260809 |
| 16 | GOLDEN BUGS PRESERVED, NOT FIXED (k) | BUG | SECSGEM/uHGemHT9045.cpp:2985（另見:4166） | golden :6189-6194 | DataItemIn一律用int*接收，不管host實際Type，UINT_8/FT_8會把8 bytes寫進4-byte int | Host用8-byte型別送Level設定值會造成堆疊緩衝區溢位寫入 | AI(W906-PT-W5-S2F42) 20260809 |
| 17 | GOLDEN BUGS PRESERVED, NOT FIXED (l) | BUG | SECSGEM/uHGemHT9045.cpp:2989（另見:4109） | golden :6135-6145 | `if(HCACK==0){...}else{...}`兩分支程式碼逐字相同，判斷式無實際作用 | 純邏輯冗餘，不影響現有行為 | AI(W906-PT-W5-S2F42) 20260809 |
| 18 | GOLDEN BUGS PRESERVED, NOT FIXED (m) | BUG | SECSGEM/uHGemHT9045.cpp:2991（另見:4125） | golden :6050/:6131 vs :6054/:6151 | `new char[10000000]`(10MB)只有走到:6131才delete[]，HCACK=4提前失敗路徑直接return不釋放 | 每次被拒請求洩漏10MB，是本清單影響最大的洩漏 | AI(W906-PT-W5-S2F42) 20260809 |
| 19 | GOLDEN BUGS KEPT B1 | BUG | SECSGEM/uHGemHT9045.cpp:4263 | golden :1497/:1565 | `if(iLoadPort>=1 \|\| iLoadPort<=15)`應為`&&`，範圍檢查恆為真 | bAskStopPort/iPortStatus可能被超出[ePortTotal]範圍的索引存取 | AI(W906-PT-W5-S2F42) 20260809 |
| 20 | GOLDEN BUGS KEPT B2 | BUG | SECSGEM/uHGemHT9045.cpp:4267 | golden :2853/:2922 | `unsigned char uint1EC`未初始化，於無Action CPNAME路徑被直接讀取使用 | 用未定義堆疊殘值寫入EC資料 | AI(W906-PT-W5-S2F42) 20260809 |
| 21 | GOLDEN BUGS KEPT B3 | BUG | SECSGEM/uHGemHT9045.cpp:4269 | golden :2036/:2522 | 兩處`S.AnsiPos("REMOTE_SAVE")==1`完全相同，第二處是不可達死碼 | 純冗餘，不影響行為 | AI(W906-PT-W5-S2F42) 20260809 |
| 22 | GOLDEN BUGS KEPT B4 | BUG | SECSGEM/uHGemHT9045.cpp:4272（另見:6458,7305） | golden AUTHORITY_CHECK:2875/START_LOT:3605/START_AGV:3760 | golden在三個無關分支重複挪用訊號燈旗標當SVlen==0暫存標記，屬複製貼上殘留 | 該全域變數在port端未定義(G25/G36/G40皆閘控)，目前僅損失一次無意義寫入 | AI(W906-PT-W5-S2F42) 20260809 |
| 23 | GOLDEN BUGS KEPT B5 | BUG | SECSGEM/uHGemHT9045.cpp:4277 | golden全檔通用 | golden自身拼寫錯誤在整份檔案中一律原樣保留不修正 | 純翻譯政策聲明，非單一功能缺陷 | AI(W906-PT-W5-S2F42) 20260809 |
| 24 | GOLDEN QUIRK PRESERVED (S7F6 PPID未確認終止字元) | QUIRK | uHGemHT9045.cpp:2265-2271 | golden uHGemHT9045.cpp:5345 | PPID是未初始化的char[1024]，DataItemIn讀入後立刻用%s格式化，卻從未確認DataItemIn是否有正確NUL終止該緩衝區 | 異常情況下%s可能讀到緩衝區外的殘留記憶體內容，組出不可預期的路徑字串 | 20260809 |
| 25 | GOLDEN QUIRK, PRESERVED (ProcessS7F23FromatReceipe消耗表頭未解碼) | QUIRK | uHGemHT9045.cpp:2321-2326,2340-2344 | golden uHGemHT9045.cpp:5826-5927(消耗表頭處約:5839) | 函式消耗掉L,4清單表頭後，並未真正讀取清單內的4個資料項，就直接回傳0代表「已接受」 | 呼叫端S7F24會回覆Host HCACK=0(接受)，但訊息內容從未被解析，Host可能誤以為配方資料已被正確處理 | 20260809 |

### SECSGEM/uHGemEquipment.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG, preserved verbatim | BUG | SECSGEM/uHGemEquipment.cpp:1121 | golden :6253-6257 | ReadAlamData的catch區塊delete memoPtr之後，控制流程仍無條件落到與try區塊末尾完全相同的RowCount=/Paste陳述式，等於例外路徑對已刪除指標use-after-free | 正常路徑僅無害重複賦值；真正觸發例外時在ASan等強化環境下會偵測到懸空指標存取甚至當機 | AI(W906-uHGemEquipment) 20260716 |
| 2 | GOLDEN BUG, preserved verbatim | BUG | SECSGEM/uHGemEquipment.cpp:2346 | golden :7432 | GetCEIDContent迴圈邊界用`->RowCount`(1025)而非同型函式慣用的`->ColCount`(258)，僅因迴圈遇空字串即break才未越界 | 若某CEID列填滿全部255個可用欄位，會越界觸發vclcompat::TStringGrid的std::out_of_range | AI(W906-uHGemEquipment) 20260716 |
| 3 | GOLDEN BUG, preserved verbatim | BUG | SECSGEM/uHGemEquipment.cpp:2524 | golden :7587 | GetReportIDContent犯相同類型錯誤但方向相反：邊界用`->RowCount`(257)而非應為`->ColCount`(1026)，迴圈可能提前終止 | Report若含超過約255個SVID會被靜默截斷，Host收到不完整資料且無任何錯誤提示 | AI(W906-uHGemEquipment) 20260716 |
| 4 | GOLDEN BUG, preserved verbatim | BUG | SECSGEM/uHGemEquipment.cpp:4552 | golden :4733-4736 | if/else兩分支都把bSeprate設false，bWaitEstablishCommunicationsResponse判斷條件毫無作用 | 純邏輯冗餘，不影響現有行為 | AI(W906-uHGemEquipment-BucketC) 20260717 |
| 5 | PRESERVED GOLDEN BUG #1 | BUG | SECSGEM/uHGemEquipment.cpp:4726（同源.h:1302） | golden .cpp:4259 | DoUploadFileToHost_ForSingleFile用值拷貝而非參照讀iUploadFileToHost_ForSingleFileMaxSend，case 100算出的chunk size只寫進區域變數即消失，該成員在全golden樹從未被賦值 | 每次進入case 200都讀回預設值0，單檔上傳(S101F5/F6)永遠卡在每次傳0位元組的分段分支無法完成，port測試已實測重現 | AI(W906-UploadFamily) 20260723 |
| 6 | PRESERVED GOLDEN BUG #2 | BUG | SECSGEM/uHGemEquipment.cpp:4744 | golden .cpp:4255 | 同類錯誤：case 100開檔後`P=fopen(...)`只寫進區域變數，從未寫回成員UploadFileToHost_ForSingleFileFilePtr | 理論上會讓後續呼叫對過期/空FILE*做fread/fclose，但因#1使iReadSize恆為0，目前處於潛伏未觸發狀態 | AI(W906-UploadFamily) 20260723 |
| 7 | GOLDEN BUG, preserved verbatim | BUG | SECSGEM/uHGemEquipment.cpp:5911 | golden原註解「不要在這裡delete!!」(16.09.14.00 Roy) | clientGemRead每次收資料都new一塊EthernetBuffer，RecvMemoryBuffer->WriteBuffer只複製內容，原始配置從未delete | 每次Socket收資料就洩漏記憶體，通訊頻繁的產線環境下持續增長 | AI(W906-uHGemEquipment-BucketC) 20260717 |
| 8 | GOLDEN RACE | RACE | SECSGEM/uHGemEquipment.cpp:5954 | golden ProcessSocketReceiveData自身(未加鎖讀清RecvMemoryBuffer) | ProcessSocketReceiveData讀取並清空RecvMemoryBuffer時完全沒有取得pLockOnSocketRecvice鎖，與clientGemRead對同一資料流「有上鎖」的寫入形成競爭 | 目前單執行緒模擬測試下無害，但接上真實雙執行緒Socket收發後有資料同時讀寫、封包截斷或遺失的真實風險 | AI(W906-uHGemEquipment-BucketC) 20260717 |
| 9 | GOLDEN LEAK (a) | LEAK | SECSGEM/uHGemEquipment.cpp:6017 | golden本函式自身(ProcessBuffer全函式從未delete[]) | 收到完整一筆訊息時`ProcessBuffer=new unsigned char[Value+4]`每次重新配置，但整個函式從未在任何路徑delete[]它 | 每處理完一筆完整SECS訊息就洩漏對應長度的記憶體，通訊量越大洩漏累積越快 | AI(W906-uHGemEquipment-BucketC) 20260717 |
| 10 | GOLDEN LEAK (b) | LEAK | SECSGEM/uHGemEquipment.cpp:6102 | golden :9132(前面的delete只涵蓋部分封包路徑) | 訊息完整收完(bFirstBlock==true)時EthernetBuffer直接被設NULL，但唯一的delete[](golden:9132)只涵蓋切割中的部分封包路徑，完整收尾路徑未先delete就歸零指標 | 每完整收到一筆訊息就洩漏一次EthernetBuffer配置，與GOLDEN LEAK(a)疊加持續累積 | AI(W906-uHGemEquipment-BucketC) 20260717 |
| 11 | golden quirk (SetAlamData ALTX未使用) | QUIRK | uHGemEquipment.cpp:1051-1058 | golden uHGemEquipment.cpp:6211-6217 | 函式接收Alarm Text(ALTX)參數，但本體只寫欄位7/8/9/10，從未寫入ALTX | 呼叫端傳入的alarm文字實際上不會被保存，與呼叫端預期有落差 | 20260716 |
| 12 | GOLDEN QUIRK, preserved verbatim (ReadAlamData讀錯grid) | QUIRK | uHGemEquipment.cpp:1066-1074 | golden uHGemEquipment.cpp:6219-6260 | 讀回內容放進strGrdAlarmOld「舊版快照」，而非ReportAlarm/WriteAlamData實際使用的即時strGrdAlarm | 若誤以為此函式會刷新即時alarm定義表，會發現設定並未真的套用 | 20260716 |
| 13 | GOLDEN QUIRK, preserved (StringOut顏色參數未用) | QUIRK | uHGemEquipment.cpp:4119-4126 | golden uHGemEquipment.cpp:409-417 | StringOut(S,C)接收顏色參數C，但本體只操作Lines/LogDataString/SelStart，從未套用顏色 | 呼叫端指定的顯示顏色在此路徑完全無效 | 20260717 |
| 14 | EIGHT GOLDEN QUIRKS(1) switch fall-through | QUIRK | uHGemEquipment.cpp:4613-4615,4660-4664 | golden uHGemEquipment.cpp:4204-4207 | case 1設完delay timer與Task=100後無break，同次呼叫立即接續執行case 100 | 屬golden刻意設計，理解此行為對除錯Trace Data時機很重要，否則易誤判為漏寫break | 20260804 |
| 15 | EIGHT GOLDEN QUIRKS(2) reference-alias idiom | QUIRK | uHGemEquipment.cpp:4616-4618,4651 | golden uHGemEquipment.cpp:4195 | 用C++參照int&Task為陣列元素取別名，與同檔其他函式共用風格 | 無行為差異，但維護者若忽略其為陣列別名容易誤解生命週期 | 20260804 |
| 16 | EIGHT GOLDEN QUIRKS(3) early-exit寫陣列非別名 | QUIRK | uHGemEquipment.cpp:4619-4622,4652-4656 | golden uHGemEquipment.cpp:4198 | early-exit分支寫TraceDataResponseTask[iIndex]=1而非透過Task別名賦值，雖同一物件但寫法不一致 | 無實際行為差異，純程式一致性瑕疵 | 20260804 |
| 17 | EIGHT GOLDEN QUIRKS(4) wire type mismatch | QUIRK | uHGemEquipment.cpp:4623-4628,4671 | golden uHGemEquipment.cpp:4214 | unsigned(4 byte)的iTOTSMP_Count以UINT_2_TYPE送出，編碼器只讀送其低16 bit | 追蹤取樣次數超過65535時，Host收到的樣本計數會被截斷成錯誤值 | 20260804 |
| 18 | EIGHT GOLDEN QUIRKS(5) 冗餘重複賦值 | QUIRK | uHGemEquipment.cpp:4629-4631,4689-4694 | golden uHGemEquipment.cpp:4232-4237 | overflow分支內已設Task=1，緊接著下一行又無條件再設一次 | 無實際行為差異，純屬死碼 | 20260804 |
| 19 | EIGHT GOLDEN QUIRKS(6) 計數遞增順序看似off-by-one | QUIRK | uHGemEquipment.cpp:4632-4635,4671-4672,4689 | golden uHGemEquipment.cpp:4214-4215,4232 | iTOTSMP_Count在DataItemOut送出「之後」才自增，overflow判斷卻用已自增過的值比較 | 可能讓實際送出樣本筆數與iTOTSMP名目上限差1 | 20260804 |
| 20 | EIGHT GOLDEN QUIRKS(7) atoi窄化存入unsigned | QUIRK | uHGemEquipment.cpp:4636-4638,4649,4669 | golden uHGemEquipment.cpp:4212-4213 | itrid宣告unsigned，卻用atoi()(int)解析iTRID字串再存入 | 若iTRID字串異常帶負號，轉unsigned後會產生巨大錯誤值並送給Host | 20260804 |
| 21 | EIGHT GOLDEN QUIRKS(8) iTimeFormat邏輯重複 | QUIRK | uHGemEquipment.cpp:4639-4645,4674-4682 | golden uHGemEquipment.cpp:4218-4225(對照GetTimeInfo :324-331) | 時間戳格式判斷是GetTimeInfo內部同段邏輯的第二份獨立複本，寫入本地str而非GemClock | 目前輸出相同，但屬程式碼重複，未來只改一處會造成兩處時間格式不一致 | 20260804 |
| 22 | GOLDEN QUIRK, preserved verbatim (DoDownLoadRemoteFile case 200邏輯反向) | QUIRK | uHGemEquipment.cpp:5152-5177,5200-5225 | golden uHGemEquipment.cpp:6746-6807 | 1秒grace window未到時反而立刻切回重送狀態，window已到卻不重送也不重啟計時器 | 實際重送S7F5的頻率遠比命名暗示的更密集，可能形成請求風暴或提早放棄下載 | 20260721 |
| 23 | GOLDEN QUIRK preserved verbatim (S14F3分支用全域HGem->) | QUIRK | uHGemEquipment.cpp:5509-5516 | golden uHGemEquipment.cpp:8922 | 分派鏈中唯獨S14F3這支改用全域裸指標HGem->，其餘分支都用隱含this-> | 呼叫端若未先設定全域HGem指標，此分支會對NULL做dereference | 20260720 |
| 24 | golden quirk (EthernetBuffer區域變數遮蔽成員) | QUIRK | uHGemEquipment.cpp:5903,5927(成員宣告 uHGemEquipment.h:810-814) | golden uHGemEquipment.cpp:9035(clientGemRead本身:9008-9028) | clientGemRead與ProcessSocketReceiveData都宣告同名區域變數，把類別成員THGem::EthernetBuffer整個遮蔽 | 純既有命名遮蔽，不影響現有邏輯，但增加後續維護者理解該成員用途的困惑風險 | 20260717 |
| 25 | GOLDEN QUIRK, preserved (bInProcess單例假設) | QUIRK | uHGemEquipment.cpp:5933 | golden uHGemEquipment.cpp:9030-9207 | 防重入旗標bInProcess宣告為function-static，隱含假設全域僅有單一THGem實例 | 若系統同時存在多個活的THGem物件，會共用同一guard，其中一個處理時會誤擋另一個的封包處理 | 20260717 |
| 26 | GOLDEN QUIRKS PRESERVED VERBATIM [1] ctSpoolFile死碼 | QUIRK | uHGemEquipment.cpp:6864-6875 | golden uHGemEquipment.cpp:1898-1924 | 遞增ctSpoolFile的區塊在golden本身就已整段被註解掉，ctSpoolFile恆為0 | 同一秒內產生多個spool檔時檔名流水號固定為"000"，同名檔案互相覆蓋可能致spool資料遺失 | 20260721 |
| 27 | GOLDEN QUIRKS PRESERVED VERBATIM [2] 固定緩衝區Ptr[1024]無界限保護 | QUIRK | uHGemEquipment.cpp:6876-6884,6895,6917 | golden uHGemEquipment.cpp:1887-1977 | 讀回驗證用堆疊陣列固定1024 byte，但fread長度理論上可達WireCodec緩衝區64MB上限，golden自身無邊界檢查 | 若曾有超過1024 byte資料流入此驗證路徑會造成堆疊緩衝區溢位，屬golden既有風險 | 20260721 |
| 28 | GOLDEN QUIRKS PRESERVED VERBATIM [3] 誤導性訊息無條件印出 | QUIRK | uHGemEquipment.cpp:6885-6889,6953 | golden uHGemEquipment.cpp:1887-1977(結尾行) | 不論寫入spool檔案是否成功，結尾都無條件印出"Host not connect...write to spool buffer!!!" | 寫入成功時仍顯示看似錯誤的訊息，可能讓維運人員誤判連線狀態 | 20260721 |
| 29 | GOLDEN QUIRK PRESERVED VERBATIM (DoSpool del指令缺空格) | QUIRK | uHGemEquipment.cpp:7019-7024,7049,7173 | golden uHGemEquipment.cpp:4102,6144 | DoSpool兩處del清除指令同樣缺*.\*與/q/f間的空格，與uHGemClass.cpp S6F24同型缺陷 | /Q /F開關不會被cmd.exe當獨立參數解析，清除動作可能不如預期安靜或強制 | 20260721 |
| 30 | golden quirk (CopyStringGridAsTabFormat空列仍輸出空欄位) | QUIRK | uHGemEquipment.cpp:2953-2963 | golden uHGemEquipment.cpp:8059-8195 | 即使整列資料皆為空字串，golden仍輸出一個只含tab字元的「空欄位」行，而非真正空白行 | 已用磁碟上EventReport_CEID.def的1024筆未用資料列核對確認此為既有且被下游檔案格式依賴的行為，不可修正為真空行 | 20260716 |
| 31 | golden quirk (CheckSFFormatOnlyHead為opt-in) | QUIRK | uHGemEquipment.cpp:5241-5247 | golden uHGemEquipment.cpp:8679-8693 | 「應為head-only」格式檢查只有在chkMoreMessageAbortProcess被勾選時才真正消費並驗證該零長度資料項，未勾選視為通過 | 該checkbox未啟用的一般情境下，帶有非預期資料的S,F訊息不會被此函式攔下，格式驗證形同選用而非必要保護 | 20260720 |

### SECSGEM/uHGemEquipment.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG preserved | BUG | SECSGEM/uHGemEquipment.h:783 | golden :625(宣告)、:8942(唯一寫入處，從未被讀) | bReceiveS110F2在golden建構子從未初始化，全樹僅一處寫入、從未有程式碼讀取，等同只寫不讀的死狀態旗標 | 目前無行為影響；port端額外主動零初始化以避免真正未定義值，是此波唯一與golden位元不完全一致處 | AI(W906-SysModWire) 20260720 |
| 2 | PRESERVED GOLDEN BUG | BUG | SECSGEM/uHGemEquipment.h:1302（另見:1677） | golden .h:490(宣告) | 標頭檔宣告旁再次確認：此成員全樹只在宣告處與.cpp值拷貝讀取處出現，從未被賦值，與uHGemEquipment.cpp #1為同一缺陷 | 實際功能影響(單檔上傳卡在0位元組分段)已在uHGemEquipment.cpp #1說明，此處為標頭檔重複記錄 | AI(W906-UploadFamily) 20260723 |

### SECSGEM/SecsWireCodec.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG, preserved verbatim | BUG | SECSGEM/SecsWireCodec.cpp:2184（banner說明於:1750） | golden uHGemEquipment.cpp:1839-1844 | ProcessSML解碼FT_8_TYPE時是唯一「先讀Ptr[]再檢查邊界」的數值型別分支，其餘分支都先檢查再讀 | 訊息長度剛好在FT_8欄位中間截斷時，會先發生1 byte跨出呼叫端緩衝區的越界讀取才觸發邊界檢查中止解碼 | UNCERTAIN，banner無獨立tag，取鄰近波次AI(W5-Final-SecsWireCodec-Wave2) 20260713推定 |

### SECSGEM/SecsSvEcRegistration.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | CONFIRMED GOLDEN BUG | BUG | SECSGEM/SecsSvEcRegistration.h:206 | golden uHGemEquipment.cpp:3654(Type未初始化)、:3688(僅IsVCL==1賦值)、:3763-3767(IsVCL==2從未賦值卻被使用) | golden區域變數Type只在IsVCL==1分支被賦值，IsVCL==2(AnsiString*)分支從未賦值就直接用於Type-keyed判斷；port端改用Type=0明確初始化，而0恰等於LIST_TYPE、無分支處理 | 任何以AnsiString*註冊的Equipment Constant，註冊後查詢目前值時GetECDataValue確定性回傳空字串而非真實內容，已由test_SecsSvEcRegistration.cpp的"GetECDataValue IsVCL==2"案例重現 | AI(W906-VCW1) 20260721 |


Two things worth flagging for whoever assembles the master ledger: (1) rows marked UNCERTAIN (uHGemClass.cpp #6, SecsWireCodec.cpp #1) have their wave_date inferred from the nearest surrounding tag, not an inline tag on the banner itself — don't cite those dates as precise. (2) Given my original pass missed 4 real items on a vocabulary-pattern gap, it's worth someone spot-checking the other agents' assigned files for the same class of miss (markers like "GOLDEN <NOUN>, preserved" / "GOLDEN <NOUN> preserved verbatim" where <NOUN> isn't BUG/ODDITY/ASYMMETRY/NOTE) rather than assuming only mine had the gap.

### 【機構/馬達雜項叢集】涵蓋：aHotPlateSubstrate(+.h) / asendic 家族 / AutoRetest.cpp / AutoClean/AutoClean.cpp / RotateKit 標頭家族 / Motor/myGALILmotor.cpp+mymotor.cpp / HThreadCtrl/HThreadCtrl.cpp / PowerSavingMode.cpp / PMAlarm/PMAlarmSystem.cpp / EJ1N 家族 / uruncontrol.cpp

### aHotPlateSubstrate.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK（citation；完整文字在 aHotPlateSubstrate.h，兩檔同一缺陷已合併於此列） | QUIRK | aHotPlateSubstrate.cpp:1417-1423 + aHotPlateSubstrate.h:592-597 | golden MyKitSuck.cpp:1443-1501 vs :1503-1561 | TMyKitSuck::MoveSuckData 在搬移資料後會多呼叫 Source.PordRec[SourceR][SourceC].InitialRecord()，外觀相同的 MoveSuckDataDiff 並不執行這個呼叫 | 呼叫端（如 DoIndexAutoClean 清潔流程）若把兩函式當作可互換使用，會誤判 Source 端記錄初始化與否，可能影響清潔資料保留判斷 | 20260728 |

TOTAL (this file): 1

### aHotPlateSubstrate.h
（此檔案唯一發現與 aHotPlateSubstrate.cpp 為同一缺陷，已合併列於該檔案表格列1，此處不重複列出以免重複計數）

TOTAL (this file): 0

### asendic.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECT, DELIBERATELY MIRRORED | DEFECT | asendic.cpp:78-88 | golden asendic.cpp:28(iLifterTask[3][10])/:35(iAutoTask[3][12]) vs main.cpp:9136-9137(extern[3][7])、main.cpp:9796-9841(42筆註冊) | golden 把 iLifterTask/iAutoTask 定義為列跨距10/12，main.cpp 卻用跨距7的 extern 宣告存取，導致第0列以上的42個 QueueTaskList 註冊全部定址錯誤記憶體 | 操作面板對 Lifter/Auto 站的手動 Task List 寫入可能寫到別的陣列格位，造成狀態機錯亂或資料互相覆蓋 | 20260802 |

TOTAL (this file): 1

### asendic_Auto2.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK | QUIRK | asendic_Auto2.cpp:311-312 | golden asendic_Auto2.cpp:183-310（case1落入case100,無break） | DoAuto2TrayToFront 的 case 1 沒有 break，落入 case 100，計時器啟動與 case100 守衛判斷在同一 tick 執行 | 若誤補 break，會讓 case100 的檢查延後一個 tick，改變 Auto2 送盤到前緣的時序 | 20260729 |
| 2 | GOLDEN QUIRK | QUIRK | asendic_Auto2.cpp:461-462 | golden asendic_Auto2.cpp:321-404（同一idiom） | DoAuto2TrayToRear 的 case 1 同樣無 break 落入 case 100，與前者是同一種寫法 | 誤補 break 會改變空盤收回 Auto2 的時序行為 | 20260729 |
| 3 | GOLDEN QUIRK | QUIRK | asendic_Auto2.cpp:567-572 | golden asendic_Auto2.cpp:433（case1落入case2） | DoAuto2 的 case1 無 break 落入 case2，但行為無害：case1只武裝1000ms計時器，case2只檢查同一計時器的Off()，武裝當下不可能為真 | 純屬死路徑；標記出來是避免被誤認為 banner 只列兩處 fall-through 而遺漏、進而被誤修 | 20260729 |

TOTAL (this file): 3

### asendic_Auto_RT.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECT (A) | DEFECT | asendic_Auto_RT.cpp:173-179 | golden :949-952 vs :957、:68 | InitTrayZAutoTrayToWaitTask() 重置的是 Loader 游標 iTrayZLoadTrayToWaitTask，但它命名要初始化的狀態機實際綁定的是 iTrayZAutoTrayToWaitTask[Pos]，兩者對不上 | 呼叫此 Init 並不會重置它命名的狀態機，反而誤動到另一個仍在運作中的狀態機游標 | 20260802 |
| 2 | GOLDEN DEFECT (B) | DEFECT | asendic_Auto_RT.cpp:180-185 | golden :983 vs :988 | DoTrayZAutoTrayToWait case200呼叫AutoCylinderMiddle時把Part寫死成1而非Pos，下一個case(:988)已修同款bug卻漏改這裡 | 只有Pos==1(Auto2)剛好正確，其餘Auto站呼叫時會操作到錯誤氣缸 | 20260802 |
| 3 | GOLDEN DEFECT (C) | DEFECT | asendic_Auto_RT.cpp:186-193 | golden :394-427、AutoRetest.cpp:1108/1129/1142、main.cpp:9870-9873 | DoAutoTrayToRear case300/400正常流程下互為死循環孤島，唯一入口是操作面板Task List UI手動寫入 | 自動流程下永不執行，屬設計死路徑非缺料 | 20260802 |
| 4 | GOLDEN DEFECT (D) | DEFECT | asendic_Auto_RT.cpp:194-198 | golden cmydef.cpp:2884(iReceiveAutoTray[3]) vs :923/:932/:940 | 陣列只宣告3格卻用可達0..5的Pos索引，Auto4/5/6會寫出邊界 | 4/5/6-Auto機型下造成記憶體越界寫入，可能污染相鄰變數 | 20260802 |
| 5 | GOLDEN DEFECT (E) | DEFECT | asendic_Auto_RT.cpp:199-201 | golden :27 | iAutoReceiveTask整棵golden樹只有宣告這一行出現過，是完全未用的死變數 | 無風險，僅占用24 bytes | 20260802 |
| 6 | GOLDEN DEFECT (F) | DEFECT | asendic_Auto_RT.cpp:202-205 | golden :63 vs :176 | DoLoadNewEmptyTrayToCar_RT在「已有盤免動作」與「新盤裝載成功」兩條路徑都回傳同一值true，呼叫端無法分辨 | AutoRetest.cpp:1208呼叫端可能誤判是否真的裝了新盤 | 20260802 |
| 7 | GOLDEN DEFECT (G) | DEFECT | asendic_Auto_RT.cpp:206-212 | golden :28/:194/:315/:669 | 四個Auto陣列初始化只設[0]=1，[1..5]=0，對應switch無case 0，Auto2..6在各自Init執行前直接return false/0 | 多Auto機型開機初期部分站點狀態機暫時無動作 | 20260802 |
| 8 | GOLDEN DEFECT (H) | DEFECT | asendic_Auto_RT.cpp:213-219 | golden :32 vs :957/:978/:981 | 其他計時器皆為[MAX_AUTO_TRAY]陣列，AutoRTDelay卻是單一純量，被每個Pos共用的狀態機當作各自的1秒穩定計時器 | 兩個Auto站同時做「最後一盤」偵測時共用同一倒數，可能互相搶走計時餘量 | 20260802 |

TOTAL (this file): 8

### AutoRetest.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECT (A) | DEFECT | AutoRetest.cpp:118-122 | golden :672-675 | DoPickFromAuto_AutoRT case500的DUMMY模式分支設ret=true後直接break卻從未推進Task，Task永遠停在500 | DUMMY模擬模式下ART取料子狀態機卡死在「蓋上料盤」步驟，永不前進 | 20260807 |
| 2 | GOLDEN DEFECT (B) | DEFECT | AutoRetest.cpp:123-131 | golden cmydef.h:4018/cmydef.cpp:4191、:526/:573 | iRetestNoIC只宣告3格卻用範圍0..iAutoRight的Pos索引；預設iAutoRight==2在界內，4/5/6-Auto機型會越界 | config相依缺陷，僅超過3-Auto機型才會真的越界寫入 | 20260807 |
| 3 | GOLDEN DEFECT (C) | DEFECT | AutoRetest.cpp:132-136 | golden :38、:44 | iAuto_AutoRT_Task/bAutoOK_AutoRT只初始化[0]=1，switch無case0，Auto2~6在各自reset執行前直接return false | 與asendic_Auto_RT.cpp的G同類缺陷，多Auto機型開機初期部分狀態機暫時無動作 | 20260807 |
| 4 | GOLDEN DEFECT (D) | DEFECT | AutoRetest.cpp:137-141 | golden :474-485 | DoTrayArm_AutoRT case200生產紀錄只有3種分支(Pos0/1/其他)，Auto4/5/6全被誤記成"Auto3" | 超過3個Auto站的機型，Auto4~6生產記錄來源被錯誤標示，影響良率統計歸屬 | 20260807 |
| 5 | GOLDEN DEFECT (E) | DEFECT | AutoRetest.cpp:142-148 | golden :1558-1693、:1692；對照GPIB_QueryData :1492/:1530(2024已修) | GPIB_SetData的do-while在收到不含分號的畸形字串時會把S2重新指派回自身，形成無窮迴圈；姊妹函式已於2024年修過同款問題但這裡沒跟上 | 畸形GPIB指令字串可能讓處理執行緒卡進無窮迴圈失去回應 | 20260807 |
| 6 | GOLDEN DEFECT (F) | DEFECT | AutoRetest.cpp:149-152 | golden :768-769 | DoPlaceToLoader_AutoRT case1把Task設回自己label後落入case100，重入永遠從case1重來 | 無害(case100成功時會覆寫Task)，只是狀態機天生無法從中段恢復 | 20260807 |
| 7 | GOLDEN DEFECT (G) | DEFECT | AutoRetest.cpp:153-157 | golden :932-936、:958/:969/:1065/:1073/:1081/:1089/:1097 | DoAuto_AutoRT每次呼叫都重算iAutoTray[]，但該陣列只在未定義的SOFT_SIMULTE下才被讀取，本build下是全死工；iAutoTray[eAuto1]也從未賦值 | 純屬效能浪費，無功能風險 | 20260807 |
| 8 | GOLDEN DEFECT (H) | DEFECT | AutoRetest.cpp:158-162 | golden DoPickFromAuto_AutoRT游標 | 取料狀態機游標是單一共用int，但計時器/旗標卻是逐Auto陣列，形狀不一致；因DoTrayArm_AutoRT已序列化多Auto取料，實務無衝突 | 目前一致，但此混合形狀若未來擴充成可並行取料容易被誤用而互相干擾 | 20260807 |
| 9 | GOLDEN DEFECT (I) | DEFECT | AutoRetest.cpp:163-166 | golden :582-588 | case350逾時分支重新武裝3秒計時器並break卻不推進Task，警報每3秒重新觸發直到感測器恢復 | 感測器持續異常時操作員會看到同一警報以3秒週期不斷跳出，形成未收斂的對話框迴圈 | 20260807 |

TOTAL (this file): 9

### AutoClean/AutoClean.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK | QUIRK | AutoClean/AutoClean.cpp:5604-5612 | golden AutoClean.cpp:6470 | ErrPart以傳值方式傳入DoIndexAutoClean_Arm1PickArm2Test，函式內修改對呼叫端static ErrPart不可見；但每次真正用到ErrPart處都在同次呼叫內從Suck[][].Error重建，故無害 | 目前無可觀察影響；若未來有人誤以為ErrPart能跨函式累積訊息會踩到這個陷阱 | 20260728 |
| 2 | GOLDEN QUIRK | QUIRK | AutoClean/AutoClean.cpp:6600 | golden AutoClean.cpp:7468 | else分支內又重複檢查一次同一個已確定為false的CheckAutoCleanCloseSite(0)==false條件 | 無行為影響，僅多一次函式呼叫開銷，保留供逐行對照 | 20260728 |

TOTAL (this file): 2

### RotateKit/aRotateKIT.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRKS CARRIED OVER VERBATIM | QUIRK | RotateKit/aRotateKIT.h:41-44 | golden aRotateKIT.h:42；aRotateKIT_Out.cpp :822/:1329/:1833/:2927(皆在註解程式碼內) | extern bool bSingleDut在golden全樹只有宣告從未定義，四處出現全藏在被註解掉的程式碼中 | 無odr-use，對連結無風險，純屬死宣告 | 20260807 |
| 2 | GOLDEN QUIRKS CARRIED OVER VERBATIM | QUIRK | RotateKit/aRotateKIT.h:45-47 | golden aRotateKIT.h:32 | MoveRotateToDegree(bool,int,bool=false)同樣只宣告、全樹無定義也無呼叫 | 無風險死宣告，忠實保留不做清理式刪除 | 20260807 |

TOTAL (this file): 2

### RotateKit/aRotateKIT_In.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECTS CARRIED OVER (a) | DEFECT | RotateKit/aRotateKIT_In.h:18-22 | golden aRotateKIT_In.h:19 vs aRotateKIT_In.cpp:876 | 標頭宣告的M_MoveInArmXY_ToRotateKIT()是零參數版本，.cpp實際定義帶int iKit的重載，零參數版本從未被定義 | 無odr-use，連結無害，僅是對不上定義的孤兒宣告 | 20260807 |
| 2 | GOLDEN DEFECTS CARRIED OVER (b) | DEFECT | RotateKit/aRotateKIT_In.h:23-28 | golden aRotateKIT_In.h:24 vs aRotateKIT_In.cpp:3804 | CheckInRotateAnglePostion宣告參數命名為Y-then-X，定義與所有呼叫端都是X-then-Y，型別相同仍正確連結，只是宣告參數名寫反；"Postion"拼字錯誤也保留 | 不影響行為，但可能誤導只看標頭的維護者對參數順序的理解 | 20260807 |

TOTAL (this file): 2

### RotateKit/aRotateKIT_In.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECTS PRESERVED VERBATIM (a) | DEFECT | RotateKit/aRotateKIT_In.cpp:209-213 | golden :3117-3133,:3139-3196 | IN臂SKIP迴圈卻用OutArmSuck.iMaxCol與iRotato_Out_Row(OUT側物件)做迴圈索引，再寫入InArmSuck.PordRec/SetItemData，是從aRotateKIT_Out.cpp複製貼上忘換邊 | IN臂旋轉站SKIP時可能依錯誤的OUT側幾何跳過/不跳過IC，造成IN/OUT資料不同步 | 20260807 |
| 2 | GOLDEN DEFECTS PRESERVED VERBATIM (b) | DEFECT | RotateKit/aRotateKIT_In.cpp:214-217 | golden :2382-2390 | 迴圈跑到iRotateCount=0時讀取Prod.RotationCount[-1]，越界讀取陣列前一元素 | 讀到相鄰記憶體資料，可能讓該次旋轉角度判斷取到無意義值 | 20260807 |
| 3 | GOLDEN DEFECTS PRESERVED VERBATIM (c) | DEFECT | RotateKit/aRotateKIT_In.cpp:218-221 | golden :1610-1616 | GotInRotateKitForAxEx_2RotMot第4個else-if判斷iCount==0已被前兩分支涵蓋，是不可達死分支，原意應為iCount==1 | 無現行影響，純屬死程式碼 | 20260807 |
| 4 | GOLDEN DEFECTS PRESERVED VERBATIM (d) | DEFECT | RotateKit/aRotateKIT_In.cpp:222-224 | golden :2414 | tRotate.RotationCount[rCount-1];是讀值卻未使用的裸表達式陳述句 | 純死程式碼無副作用；本port閘門替換成同樣無害的(0); | 20260807 |
| 5 | GOLDEN DEFECTS PRESERVED VERBATIM (e) | DEFECT | RotateKit/aRotateKIT_In.cpp:225-228 | golden :1984,:2019 | for迴圈後的掛起重新檢查讀取iSuckR，若迴圈本體從未執行則讀到未初始化值 | 邊界情況下可能讀到未定義iSuckR，導致後續分支判斷不可預期 | 20260807 |
| 6 | GOLDEN DEFECTS PRESERVED VERBATIM (f) | DEFECT | RotateKit/aRotateKIT_In.cpp:229-231 | golden :180 | MoveInArmZ_ToRotateKIT_Pick非1X4分支把Z移到PlaceZ而非PickZ，複製貼上漏改欄位 | 非1X4組態下IN臂取料可能移到放置高度而非取料高度，有碰撞/取料失敗風險 | 20260807 |
| 7 | GOLDEN DEFECTS PRESERVED VERBATIM (g) | DEFECT | RotateKit/aRotateKIT_In.cpp:232-234 | golden :575-597 | DoInArmRotateKIT_Cylinder case1600全檔案無任何處賦值Task=1600，是不可達死case | 無風險，永不執行 | 20260807 |
| 8 | GOLDEN DEFECTS PRESERVED VERBATIM (h) | DEFECT | RotateKit/aRotateKIT_In.cpp:235-237 | golden :3826,:3851 | CheckInRotateSensor的iCount++在函式開頭跑一次、失敗分支又跑一次，持續失敗約25次輪詢就達到iCount>50門檻而非預期50次 | 感測器異常警報比設計預期提前約一半輪詢次數觸發 | 20260807 |
| 9 | GOLDEN DEFECTS PRESERVED VERBATIM (i) | DEFECT | RotateKit/aRotateKIT_In.cpp:238-239 | golden :3384 | iRet=M8_DoInRotateMove(rCount,bReset);; 結尾多餘的空陳述句分號 | 純語法無害多餘符號 | 20260807 |
| 10 | GOLDEN DEFECTS PRESERVED VERBATIM (j) | DEFECT | RotateKit/aRotateKIT_In.cpp:240-241 | golden :71 | int iMovePitchX=iRotateKIT_Pitch_X_H*3/2;; 同樣多餘空陳述句，且*3/2是先乘3再整數除2的截斷運算 | 整數截斷可能讓位移計算比理論值有捨去誤差 | 20260807 |
| 11 | GOLDEN DEFECTS PRESERVED VERBATIM (k) | DEFECT | RotateKit/aRotateKIT_In.cpp:242-249 | golden :936,:1111,:1136,:1956,:1991,:2580,:2611,:3117,:3139 | e1/e2MotRotate迴圈以InArmSuck.iMaxCol為界，卻索引只有4欄寬的flag[2][4]與bSuckDuplicateErr[2][4]，8-picker等寬手臂(iMaxCol>4)下會越界 | 寬手臂組態下可能覆蓋相鄰記憶體，造成不可預期的旋轉狀態誤判 | 20260807 |

TOTAL (this file): 11

### RotateKit/aRotateKIT_Out.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECT (E-1) | DEFECT | RotateKit/aRotateKIT_Out.cpp:195-197 | golden :174 | MoveOutArmZ_ToRotateKIT_Pick非1X4臂移到PlaceZ而非PickZ，複製貼上缺陷 | 非1X4組態OUT臂取料恐移到放置高度，碰撞/取料失敗風險 | 20260807 |
| 2 | GOLDEN DEFECT (E-2) | DEFECT | RotateKit/aRotateKIT_Out.cpp:198-200 | golden :368-379 | case3000在==1分支內先設Task=3100，之後又無條件再設一次，多餘死賦值；==2分支依賴此無條件賦值 | 無風險，屬冗餘 | 20260807 |
| 3 | GOLDEN DEFECT (E-3) | DEFECT | RotateKit/aRotateKIT_Out.cpp:201-204 | golden :273-274 | case1200開頭把Task設成自己已持有的值，未真正攔阻，直接落入MoveOutArmZ呼叫 | 無攔阻效果，等同拿掉此行 | 20260807 |
| 4 | GOLDEN DEFECT (E-4) | DEFECT | RotateKit/aRotateKIT_Out.cpp:205-207 | golden :1619-1621 | 第4個else if判斷iCount==0已被第2分支涵蓋，不可達 | 純死程式碼 | 20260807 |
| 5 | GOLDEN DEFECT (E-5) | DEFECT | RotateKit/aRotateKIT_Out.cpp:208-210 | golden :300/:947/:1980等 | `&&` 與 `\|\|` 混用無括號分組 | 目前結果正確(依C++優先權)，但排版易誤導讀者 | 20260807 |
| 6 | GOLDEN DEFECT (E-6) | DEFECT | RotateKit/aRotateKIT_Out.cpp:211-213 | golden :305 vs :1954 | CYLINDER版本寫入bInArmCheckDestroyACT(IN臂陣列)而非bOutArmCheckDestroyACT，MOTOR版本才正確 | CYLINDER旋轉站機型的OUT臂銷毀確認狀態被記到IN臂陣列，可能互相污染 | 20260807 |
| 7 | GOLDEN DEFECT (E-7) | DEFECT | RotateKit/aRotateKIT_Out.cpp:214-216 | golden :3535 | btest讀MInRotateKit(IN側)方向卻宣告在OUT旋轉函式內且從未使用 | 死變數，(void)btest標記保留 | 20260807 |
| 8 | GOLDEN DEFECT (E-8) | DEFECT | RotateKit/aRotateKIT_Out.cpp:217-220 | golden :2628/:2658/:3085/:3601 讀InArmSuck.iPickCol vs 對應PLACE端:954/:974/:1727/:1969讀OutArmSuck.iPickCol | OUT旋轉分支讀取IN臂的iPickCol來決定走向，而對應PLACE端程式碼卻讀OutArmSuck.iPickCol，兩側對同一判斷讀了不同物件欄位 | IN/OUT的iPickCol不同步時OUT旋轉分支依錯誤欄位選路 | 20260807 |
| 9 | GOLDEN DEFECT (E-9) | DEFECT | RotateKit/aRotateKIT_Out.cpp:221-223 | golden :934/:1115/:2598/:3211 | 迴圈以OutArmSuck.iMaxCol為界卻索引4欄寬flag[2][4] | 寬手臂(如8-picker)組態下越界寫入，同IN側項(k) | 20260807 |
| 10 | GOLDEN DEFECT (E-10) | DEFECT | RotateKit/aRotateKIT_Out.cpp:224-226 | golden :3304 | 泛用SKIP分支用IN側stride常數i2x2Suck而非i2x2Suck_Out | 兩常數不同值時j2索引偏移，對錯吸嘴格位 | 20260807 |
| 11 | GOLDEN DEFECT (E-11)（與 aRotateKIT_Out.h 列1 同一缺陷，已合併） | DEFECT | RotateKit/aRotateKIT_Out.cpp:227-230 + RotateKit/aRotateKIT_Out.h:21-25 | golden .h:16(零參數) vs .cpp:868(int iKit) | 標頭宣告M_MoveOutArmXY_ToRotateKIT()零參數版本，.cpp定義帶int iKit版本，零參數版本從未被定義也從未被呼叫 | 無連結風險，孤兒宣告 | 20260807 |
| 12 | GOLDEN DEFECT (E-12)（與 aRotateKIT_Out.h 列2 同一缺陷，已合併） | DEFECT | RotateKit/aRotateKIT_Out.cpp:231-233 + RotateKit/aRotateKIT_Out.h:26-29 | golden .h:32 vs .cpp:3914 | CheckOutRotateAnglePostion在標頭與.cpp最後兩參數命名順序相反(iTrayY,iTrayX對iTrayX,iTrayY)，型別相同仍正確連結，只是宣告參數名寫反 | 不影響行為，可能誤導讀標頭者 | 20260807 |
| 13 | GOLDEN DEFECT (E-13) | DEFECT | RotateKit/aRotateKIT_Out.cpp:234-235 | golden :3484/:3580/:3665/:3739 | M2/M4/M8的int&rCount參數宣告後從未讀寫 | 無用參數，無功能影響 | 20260807 |

TOTAL (this file): 13

### RotateKit/aRotateKIT_Out.h
（原列1、列2 與 aRotateKIT_Out.cpp 的 E-11、E-12 為同一缺陷，已合併列於該檔案表格列11、12，此處不重複列出）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | ⚠️GOLDEN DEFECTS PRESERVED IN THIS HEADER (Postion拼字說明) | DEFECT | RotateKit/aRotateKIT_Out.h:30 | golden aRotateKIT_Out.h/.cpp 兩個角度判斷函式名稱 | "Postion"（應為Position）的拼字錯誤在兩個角度判斷函式名稱(CheckOutRotateAnglePostion等)中都被保留 | 純屬命名拼字問題，不影響編譯與行為，保留是為了與golden符號名稱逐字對照 | 20260807 |

UNCERTAIN reason for row 1: 上一輪我把這條摺進了 E-12/row2 的敘述裡當附註，這輪照你的指示還原成獨立列；它在原始檔案裡confirmed是獨立的第3個bullet(banner行30)，但它本身沒有自己的golden行號/獨立呼叫端影響，比較接近文件性註記而非功能性defect，是否該算「第90列」還是該視為row2的附註，留給你判斷。

TOTAL (this file): 1

### Motor/myGALILmotor.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECT (i) | DEFECT | Motor/myGALILmotor.cpp:457-468 | golden :4158 vs note.h:466、sibling :3756 | ShowErrorMessage第4參數應為bool bDuplicateErr卻傳入字串"Gali_FindZPhase_446"，指標退化為true，誤標此警報為重複錯誤且丟棄辨識標籤 | WAR0308警報被誤判為重複錯誤(可能被抑制)，且失去可辨識來源的診斷標籤，增加排查難度 | 20260808 |
| 2 | GOLDEN DEFECT (ii) | DEFECT | Motor/myGALILmotor.cpp:469-472 | golden :1647 內嵌於:1639 | if(lSC==8)包在if(lSC==100)裡，lSC不可能同時為100又為8，"ER+OE Following Error Exceeded"訊息永不可達 | 該錯誤訊息文字實務上永不顯示，診斷資訊較不精確 | 20260808 |
| 3 | GOLDEN DEFECT (iii) | DEFECT | Motor/myGALILmotor.cpp:473-475 | golden :755-758/:951-954/:1148-1151/:2835-2838/:2848-2851 | 五處if(Pos==0){Pos=0;}自我賦值，推測是除錯中斷點掛載點 | 無行為影響，死程式碼 | 20260808 |
| 4 | GOLDEN DEFECT (iv) | DEFECT | Motor/myGALILmotor.cpp:476-480 | golden :3202-3213 vs :3279-3290 | CheckPos()3軸分支設b2=true，結構相同的CheckPos_nnMode()兩個3軸分支卻都設b2=false，導致後者3軸模式恆回傳false | nn模式3軸組態下CheckPos_nnMode判斷恆為false，依賴它的到位判斷/安全互鎖可能永遠無法通過 | 20260808 |
| 5 | GOLDEN DEFECT (v) | DEFECT | Motor/myGALILmotor.cpp:481-487 | golden :3166 對照有護的:462/:658/:710/:2650 | Gali_ReadEncoderPos解參考Motor->Enable無NULL防護，多個CheckPos*/Gali_ReadEncoder*共用此缺失 | cinitial尚未配置馬達物件前(MOT[i].Motor為NULL)呼叫會NULL指標當機，是本檔案延後掛入CMake建置的原因之一 | 20260808 |
| 6 | GOLDEN DEFECT (vi) | DEFECT | Motor/myGALILmotor.cpp:488-494 | golden :4838-4846 及 :5047-5048 | Gali_nnMode_Z1Z2_Down/_Up應分別讀MTestZ1/MTestZ2兩軸，實際上Z2對應變數也都從MTestZ1讀值，看似複製貼上漏改軸別，但為9046LS nn模式出貨版本實際依賴的行為 | 若「修正」讓Z2改讀MTestZ2，會改變9046LS nn模式已上線機台的實際運動行為，故明確保留不修 | 20260808 |

TOTAL (this file): 6

### Motor/mymotor.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECTS PRESERVED, NOT FIXED (1)（IndexPosMonitor） | DEFECT | Motor/mymotor.cpp:2849-2854 | golden :231、:239/:251 | bool Z1,Z2,Y1,Y2用回傳long的Gali_ReadEncoderPos()賦值，非零位置摺疊成true==1，比較與%d輸出都基於摺疊後的0/1值 | 位置監控訊息實際只顯示0或1而非真正編碼器座標，故障排查失真 | 20260808 |
| 2 | GOLDEN DEFECTS PRESERVED, NOT FIXED (2)（IndexPosMonitor） | DEFECT | Motor/mymotor.cpp:2855-2857 | golden :231(宣告)/:253(讀) vs :248-249(僅4軸寫) | Y2只在4軸路徑被賦值，非4軸路徑卻仍被讀取，屬未初始化讀取 | 非4軸組態下監控函式可能讀到未定義Y2值 | 20260808 |
| 3 | GOLDEN DEFECTS PRESERVED, NOT FIXED (3)（IndexPosMonitor） | DEFECT | Motor/mymotor.cpp:2858 | golden :230(宣告)/:258(return) | bRet從宣告後從未被賦值，函式永遠回傳false | 呼叫端若依賴此回傳值判斷成功與否會恆常收到false | 20260808 |
| 4 | GOLDEN DEFECTS PRESERVED, NOT FIXED (1)（ChangePosition） | DEFECT | Motor/mymotor.cpp:2997-2998 | golden :2518,:2549 | return dPos6;從int函式回傳double值，隱含向零截斷 | 位置計算小數部分被捨去，長期累積可能造成精度緩慢漂移 | 20260808 |
| 5 | GOLDEN DEFECTS PRESERVED, NOT FIXED (2)（ChangePosition） | DEFECT | Motor/mymotor.cpp:2999-3001 | golden cmydef.h:4576(iMagneticScalePos[16][1000])，j跑到999 | iMagneticScalePos[iMove][j+1]在j=999最後一輪索引到[iMove][1000]，超出陣列邊界一格 | 單元素越界讀取，可能讀到相鄰記憶體資料造成計算微小誤差 | 20260808 |

TOTAL (this file): 5

### HThreadCtrl/HThreadCtrl.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRKS preserved VERBATIM (1) | QUIRK | HThreadCtrl/HThreadCtrl.cpp:67-71 | golden 2008.10.14 header comment設計，_handle/_execute | _handle與_execute同時被呼叫端執行緒與背景ThreadProcess存取，完全無lock無volatile保護 | 若同時存在多個實例可能發生資料競爭，惟目前全系統僅ShuttleThread的HThreadCtrlShuttle一個活躍實例 | 20260807 |
| 2 | GOLDEN QUIRKS preserved VERBATIM (2) | QUIRK | HThreadCtrl/HThreadCtrl.cpp:72-76 | golden CloseThread(int iTimeOut=INFINITE) | CloseThread無條件呼叫WaitForSingleObject(_handle,iTimeOut)，即使前面early return已排除_handle==NULL情況 | 無行為影響，純控制流程冗餘 | 20260807 |
| 3 | GOLDEN QUIRKS preserved VERBATIM (3) | QUIRK | HThreadCtrl/HThreadCtrl.cpp:77-82 | golden ThreadProcess的static int iSleep=0 | iSleep是函式局部static，process內所有曾執行過ThreadProcess的實例共用同一份 | 若曾存在第二個實例的執行緒，睡眠計數會互相干擾；目前唯一已知實例是Shuttle執行緒 | 20260807 |

TOTAL (this file): 3

### PowerSavingMode.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRKS DELIBERATELY PRESERVED (a) | QUIRK | PowerSavingMode.cpp:401-411 | golden :297-300 | SetFunction()的bC05_PowerSaveVacuum分支誤寫HotModule->AlarmTmr而非VacuumModule->AlarmTmr，疑為複製貼上漏改模組 | VacuumModule->AlarmTmr永遠不被設定(保持0.0)，導致其TimeOut()下次掃描立刻逾時；同時HotModule門檻被真空halt time覆蓋，可能造成真空省電提早誤觸發、溫控省電延遲被錯誤覆寫 | 20260808 |
| 2 | GOLDEN QUIRKS DELIBERATELY PRESERVED (b) | QUIRK | PowerSavingMode.cpp:412-417 | golden :356-357 | TModule::TimeOut()跨午夜分支的bResult有補償，但iCountDown用非跨夜算式，導致跨夜當下倒數計時算成負值 | 面板倒數計時跨午夜瞬間可能算出負值(顯示層已用iCountDown>OverDayAM遮蔽)，底層計算本身仍錯 | 20260808 |
| 3 | GOLDEN QUIRKS DELIBERATELY PRESERVED (c) | QUIRK | PowerSavingMode.cpp:418-420 | golden :240-243 | SetAlarmTime把0分鐘強制轉1分鐘，並鎖上限MaxMinute==200分鐘 | 使用者無法設定「不延遲」或超過200分鐘的省電門檻，設定檔想要更長時間也會被裁切 | 20260808 |
| 4 | GOLDEN QUIRKS DELIBERATELY PRESERVED (d) | QUIRK | PowerSavingMode.cpp:421-428 | golden :268-270 | 挑選最小停機時間用TStringList字串排序(ASCII)而非數值排序，"10"會排在"9"之前，選出的"最小值"變成10 | 混合位數數字時挑出的「最短停機時間」可能非真正數值最小值，影響省電/停機觸發時機 | 20260808 |
| 5 | GOLDEN QUIRKS DELIBERATELY PRESERVED (e) | QUIRK | PowerSavingMode.cpp:429-430 | golden :49,:310 | SetFunction(bool OnOff)的OnOff參數從未被讀取，兩呼叫端都固定傳true | 目前無影響，但參數形同虛設，未來想用false停用功能會發現無效 | 20260808 |
| 6 | GOLDEN QUIRKS DELIBERATELY PRESERVED (f) | QUIRK | PowerSavingMode.cpp:431-435 | golden .h:15, .cpp:333-336 | 成員mSW被宣告卻從未賦值或讀取，建構子也未初始化它 | 因完全未使用無實際風險；本port刻意不補上mSW=NULL避免發明golden沒有的行為 | 20260808 |
| 7 | GOLDEN QUIRKS DELIBERATELY PRESERVED (g) | QUIRK | PowerSavingMode.cpp:436 | golden :347 | TimeOut()內AnsiString ss;是完全未使用的死變數 | 無行為影響 | 20260808 |
| 8 | GOLDEN QUIRKS DELIBERATELY PRESERVED (h) | QUIRK | PowerSavingMode.cpp:437-438 | golden :147-148 | OnScanTmr case3內宣告int iTmp;未用大括號建立區塊作用域 | 合法C++不影響行為，純風格瑕疵 | 20260808 |

TOTAL (this file): 8

### PMAlarm/PMAlarmSystem.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK, PRESERVED VERBATIM | QUIRK | PMAlarm/PMAlarmSystem.cpp:101-107 | golden PMAlarmSystem.h:115(聲明)；全樹(含.svn)無定義；唯一同名函式屬另一class TfPMAlarmInterFace::SaveGerneralPMAlarmToCSV()(PMAlarm/PMAlarmInterFace.cpp:1066) | PMAlarmSystem::SaveGerneralPMAlarmToCSV()在golden標頭被宣告，但整個golden樹都找不到這個函式的定義；唯一同名函式其實屬於不同class | 若曾有程式碼呼叫它，golden端本身就已是連結錯誤(表示從未被呼叫)；本port同樣不補上定義以維持忠實對照 | 20260807 |

TOTAL (this file): 1

### EJ1N/TextProcess.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRKS / BUGS preserved VERBATIM (1) | QUIRK | EJ1N/TextProcess.cpp:94-98 | golden :428,:452 | ConvertByte/Convert2Byte用unsigned int i計算pow(16,3-i)/pow(16,1-i)，i超過3/1時發生無號underflow，pow回傳+inf窄化成int，函式僅對≤4/2字元輸入有意義 | 呼叫端傳入超長十六進位字串時，結果變成不可預期極端值而非正確截斷/錯誤提示 | 20260807 |
| 2 | GOLDEN QUIRKS / BUGS preserved VERBATIM (2) | QUIRK | EJ1N/TextProcess.cpp:99-100 | golden :434,:458 | 兩函式例外文字誤拼"ConverByte"/"Conver2Byte"，少了"t" | 純log拼字錯誤，可能造成事後用正確拼字搜尋日誌時找不到 | 20260807 |
| 3 | GOLDEN QUIRKS / BUGS preserved VERBATIM (3) | QUIRK | EJ1N/TextProcess.cpp:101-104 | golden :527 | StringConvert2Hex組出S1卻從未使用(回傳S2)，且用sizeof(Target)當strncpy邊界寫入10KB堆疊緩衝區，輸入超過10240字元Target不會終止 | S1組建為浪費運算；異常長輸入下Target可能無正確字串結尾，後續當C字串使用有讀出超界資料風險 | 20260807 |
| 4 | GOLDEN QUIRKS / BUGS preserved VERBATIM (4) | QUIRK | EJ1N/TextProcess.cpp:105-109 | golden :563，ChungHung 20121030註解 | A_Create_LCR迴圈本體與while條件各遞減length一次，golden作者自己註解說明多加的if(length==0)break正是為了擋住雙重遞減在奇數長度造成的無窮迴圈/執行緒卡死 | 忠實保留golden已知並已補防護的既有缺陷，不做進一步正規化修正 | 20260807 |
| 5 | GOLDEN QUIRKS / BUGS preserved VERBATIM (5) | QUIRK | EJ1N/TextProcess.cpp:110-112 | golden :595 | GetCOMPortStatus在::CreateFile失敗後用完全相同參數重試一次，只有競態情況下才可能成功 | 對真正設定錯誤(COM埠不存在)無效，僅能處理極短暫資源競爭窗口 | 20260807 |
| 6 | GOLDEN QUIRKS / BUGS preserved VERBATIM (6) | QUIRK | EJ1N/TextProcess.cpp:113-115 | golden :16 | T_ASXII2HEX_Mac 23格對照表用 `>22\|\|<0` 邊界檢查，涵蓋7個標點符號(':'..'@')卻被填成0而非拒絕 | 非十六進位標點字元不被判定錯誤而是靜默轉成0，可能掩蓋輸入格式錯誤 | 20260807 |
| 7 | GOLDEN QUIRKS / BUGS preserved VERBATIM (7) | QUIRK | EJ1N/TextProcess.cpp:116-119 | golden :323 | HexStrToInt(char*)直接原地把呼叫端緩衝區轉大寫，若傳入字串常值(唯讀記憶體)會當機；golden與本port均無此類呼叫 | 目前無觸發風險，但函式本身存在對唯讀記憶體寫入當機的地雷 | 20260807 |

TOTAL (this file): 7

### EJ1N/uDTME08Control.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK preserved verbatim | QUIRK | EJ1N/uDTME08Control.cpp:62-68 | golden uDTME08Control.cpp:353-364(:356,:361) | ErrorCodeDescription對bdata==0x02測試兩次，第一次(:356)對應"感測器斷線"命中後，第二次(:361)"輸入資料未穩定"分支因else-if鏈永遠到不了 | "輸入資料未穩定"描述文字永不會被回傳，實際發生此情境時使用者只會看到可能不準確的"感測器斷線"訊息 | 20260807 |

TOTAL (this file): 1

### EJ1N/uModbusCommand.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRKS / BUGS preserved VERBATIM (1) | QUIRK | EJ1N/uModbusCommand.cpp:31-42 | golden :33,46,66 | memset(cSet,0x0,sizeof(cSet))中cSet是char*參數，sizeof(cSet)量到指標大小(4或8bytes)而非真實緩衝區長度，只清了最前幾bytes | 因後續程式碼會逐欄位重新賦值覆蓋，實際封包內容與golden一致，僅呼叫端本就忽略的尾端bytes未定義，無實際影響 | 20260807 |
| 2 | GOLDEN QUIRKS / BUGS preserved VERBATIM (2) | QUIRK | EJ1N/uModbusCommand.cpp:43-55 | golden :34-40,47-53,57,69-71 | 每個欄位寫入用*(unsigned int*)&cSet[N]=swap16(value)把4-byte int寫進2-byte欄位位置，連續欄位互相重疊覆蓋2bytes；golden欄位撰寫順序保證每欄位最終被後續陳述式正確覆寫回對的值 | 最終封包位元組與golden完全一致，是嚴重違反strict-aliasing但結果無害的既有寫法 | 20260807 |
| 3 | GOLDEN QUIRKS / BUGS preserved VERBATIM (3) | QUIRK | EJ1N/uModbusCommand.cpp:56-66 | golden :151 (ClearListValue) | ZeroMemory(iListValue,MaxListValue)中iListValue是int[MaxListValue](40bytes)，但傳入byte數卻是MaxListValue(10)，只清了前10 bytes | 因iListValueLen=0;才是真正生效的清空關鍵(後續讀寫都在0..iListValueLen-1範圍)，對呼叫端可觀察行為無影響 | 20260807 |
| 4 | GOLDEN QUIRKS / BUGS preserved VERBATIM (4) | QUIRK | EJ1N/uModbusCommand.cpp:67-73 | golden :155-158 (GetDataLength) | 對已是int的參數多此一舉static_cast<int>再乘0.5浮點數，最後隱含窄化回int，等同HexData/2(僅偶數輸入成立，Modbus byte count恆為偶數) | 對實際呼叫模式無行為差異，純寫法冗餘 | 20260807 |

TOTAL (this file): 4

### EJ1N/uModbusCommand.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK / GOLDEN QUIRK,preserved,NOT a gate | QUIRK | EJ1N/uModbusCommand.h:23,24,101 | golden uModbusCommand.h:41 (int GetClientDecodeReadSingleRTU(char*,int&);) — golden .cpp(160行)從未定義，EJ1N/整目錄從未呼叫 | GetClientDecodeReadSingleRTU在golden標頭宣告，但golden自己的.cpp從未定義它，EJ1N/其他單元也從未呼叫它 | 是golden自己留下的幽靈宣告，本port同樣宣告但刻意不補定義，無連結或行為風險 | 20260807 |

TOTAL (this file): 1

### uruncontrol.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK preserved VERBATIM | QUIRK | uruncontrol.cpp:89-95 | golden uruncontrol.cpp:46,:58-66 (Execute()的ct計數器) | ct計數器遞增並在超過2時重置，但ct>2與else分支最終都呼叫同一個MySleepEx(1,true)，函式外也無人讀取ct，是完全不影響行為的死計數器 | 純屬無效程式碼，不影響Execute()實際休眠行為 | 20260807 |

TOTAL (this file): 1

### 【TCP/站別管理叢集】涵蓋：Interface/TesterTCP_Socket.cpp(+.h) / cSiteUseManager.cpp

### Interface/TesterTCP_Socket.cpp

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | FAITHFUL GOLDEN QUIRK（unnumbered） | QUIRK | Interface/TesterTCP_Socket.cpp:339 | golden TesterTCP.cpp:241-274（函式範圍） | 當 LastSet.iTester==OFF_LINE 時，golden 寫的是 Msg2+=Msg2+"OffLine"，把 Msg2 整段內容複製一次而非只附加 "OffLine" 字串 | 若之後有其他路徑用到這個被異常放大的 Msg2，可能造成訊息內容錯亂或誤導 log 判讀 | 20260711 |
| 2 | GOLDEN QUIRK #1 | QUIRK | Interface/TesterTCP_Socket.cpp:426 | golden TesterTCP.cpp:349-552（函式範圍） | bTimerRun 只在入口被讀取當重入防護、每個出口都重置為 false，但整個 golden 函式從未有敘述把它設為 true，是永遠不會啟動的死碼防護 | 若誤以為此旗標真的在防止 timer 重入而據此判斷安全性，可能低估同時多次觸發 TimerProcessTCPDataTimer 的風險 | 20260720 |
| 3 | GOLDEN QUIRK #4 | QUIRK | Interface/TesterTCP_Socket.cpp:477 | golden TesterTCP.cpp:394 | 對 labOcr[i].Caption 賦值後多寫了一個字面上多餘的分號（;;） | 純語法冗餘不影響行為，但若日後用原始碼樣式做自動格式比對，可能誤判為格式錯誤 | 20260720 |
| 4 | GOLDEN QUIRK #5 | QUIRK | Interface/TesterTCP_Socket.cpp:490 | golden TesterTCP.cpp:405 | ECHOCODE 分支中 iLen 被賦值為 Str2.Pos(":") 卻從未在此分支被讀取，實際刪除長度用的是寫死常數 9 | 屬無害死值寫入，但 iLen 是跨分支共用變數（BINON 分支會重新賦值），維護者需留意兩處用途不同以免誤合併邏輯 | 20260720 |
| 5 | GOLDEN QUIRK #6（UNCERTAIN：偏風格慣用法非功能缺陷） | QUIRK | Interface/TesterTCP_Socket.cpp:497 | golden TesterTCP.cpp:406 | Str2=Str2.Delete(1,9) 是原地修改後又自我賦值的寫法，因 vclcompat 的 AnsiString::Delete 回傳 *this，屬忠實翻譯而非改寫 | 不造成行為差異，僅提醒維護者這是刻意保留的原始風格，勿「精簡」成別的寫法 | 20260720 |
| 6 | GOLDEN QUIRK #14 | QUIRK | Interface/TesterTCP_Socket.cpp:550 | golden Command.cpp:940（WritePERSITETemperature wrapper） | WritePERSITETemperature 在 TCP_IP_MODE 下永遠執行 sprintf("%s\r", sRet)，sRet 為空也得到非空字串 "\r"，使 TempArm? 回覆分支必定成立 | Tester 每次送 TempArm? 都至少收到最小值 "TempArm:" 的回覆，即便真正溫度資料是空的，可能讓 Tester 端誤以為有有效資料 | 20260720 |
| 7 | GOLDEN QUIRK #15 | QUIRK | Interface/TesterTCP_Socket.cpp:560 | golden TesterTCP.cpp:457 | 組成 TempArm 回覆前只用 StringReplace 移除 "\r"，未移除 "\n"，逐字對照 golden 保留此不對稱清理 | 若 asTCPIPTemperature 內含 "\n"，會原樣送進回覆字串，可能造成 Tester 端單行協定解析多出非預期換行 | 20260720 |
| 8 | GOLDEN QUIRK #7 | QUIRK | Interface/TesterTCP_Socket.cpp:575 | golden TesterTCP.cpp:469 | asECHO 在 Str2 被去除 \r\n 之前就複製了原始 sTCPIPRecevieData，保留了原始的內嵌 CRLF | "ECHO:" 回覆字串挾帶內嵌 CRLF，加上 SendTCPIPCommand 又在結尾另附一次 "\r\n"，可能讓 Tester 端單行解析器誤判封包邊界 | 20260720 |
| 9 | GOLDEN QUIRK #8 | QUIRK | Interface/TesterTCP_Socket.cpp:587 | golden TesterTCP.cpp:477-480 | BINON 解析時列順序用寫死 Strings[3-i] 反轉、欄寬寫死 *8、每欄用 SubString(MAX_SOCKET_COL-j,1) 逐字元右至左取值，均非以 MAX_SOCKET_ROW/COL 為基準的通用寫法 | 若日後 MAX_SOCKET_ROW/COL 常數變動（如擴充站點數），這段寫死索引不會自動調整，會直接讀錯 bin 結果對應的 site | 20260720 |
| 10 | GOLDEN QUIRK #16 | QUIRK | Interface/TesterTCP_Socket.cpp:629 | golden TesterTCP.cpp:509 | ECHOOK 分支只記錄一個空白字元 " " 到通訊 log，沿用 AddTCPIPCommunicationLog 對 " " 的特殊「空白行」慣例 | 純屬 log 風格，若有人依 log 內容做自動化解析可能誤把此行當雜訊而漏掉 ECHOOK 事件時間點 | 20260720 |
| 11 | GOLDEN QUIRK #10 | QUIRK | Interface/TesterTCP_Socket.cpp:640 | golden TesterTCP.cpp:516-525 | GetTPSVersionOK 整個分支在 golden 中已是被註解掉的死碼，此處以註解形式忠實保留，並未復活執行 | 若測試機真的送出 "GetTPSVersionOK" 前綴指令，Handler 完全不會回應也不會更新 TPS 版本資訊，依賴此握手的舊流程會卡住等不到回覆 | 20260720 |
| 12 | GOLDEN QUIRK #2 | QUIRK | Interface/TesterTCP_Socket.cpp:657 | golden TesterTCP.cpp:529-530 vs :375-376 | 函式開頭 new 出的 sBarCodeList 與 sBinList 兩個 TStringList，在所有路徑上都沒有被 delete，只有 sList 被清空刪除 | 每次進入含 BARCODE?/BINON 分支的訊息處理就洩漏兩個物件，長時間高流量運行下記憶體會持續累積、緩慢上升 | 20260720 |
| 13 | GOLDEN QUIRK #9 | QUIRK | Interface/TesterTCP_Socket.cpp:670 | golden TesterTCP.cpp:534-535 | 確認 SocketTCPIPReceiveList->Count==0 後又呼叫一次 Clear()，對已空清單清空是無意義操作 | 純屬效能上的無謂呼叫，不影響正確性，只是每次 timer tick 多一次沒有效果的方法呼叫 | 20260720 |
| 14 | GOLDEN QUIRK #12 | QUIRK | Interface/TesterTCP_Socket.cpp:678, Interface/TesterTCP_Socket.h:198 | golden TesterTCP.cpp:64（建構子預設 false）, :543（自清）; TesterTCP.h:243（宣告） | SimulateStart 整棵 golden 樹只有建構子預設 false、這裡的讀取、與讀取後自我清成 false 三個存取點，沒有任何地方會設為 true，是永遠不會被自然觸發的模擬分支 | OFF_LINE 模擬模式下 SimulateBin 的自動觸發路徑在正式環境形同死碼，只能靠測試或未來手動賦值驅動，不會因正常操作意外跑出模擬資料 | 20260720 |
| 15 | GOLDEN QUIRK #3 | QUIRK | Interface/TesterTCP_Socket.cpp:696 | golden TesterTCP.cpp:549 | catch(...) 例外處理區塊中組出的 str 診斷字串建立後從未被讀取、記錄或使用，是例外路徑中的死值寫入 | 發生例外時實際上不會留下任何可觀察的診斷紀錄，未來排查 TimerProcessTCPDataTimer 例外情況時這段 str 完全幫不上忙 | 20260720 |

### Interface/TesterTCP_Socket.h

（見 TesterTCP_Socket.cpp，同一缺陷）— 其 1 hit（line 198, "GOLDEN QUIRK"）已併入上表第 14 列（GOLDEN QUIRK #12, SimulateStart never set true），不另立表格。

### cSiteUseManager.cpp

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK Q8 | QUIRK | cSiteUseManager.cpp:183 | golden cSiteUseManager.cpp:103-106 | HPHasSpace 的 iNeedCells<=0 提前回傳 true 的判斷寫在盤別範圍檢查之前，導致對不存在的盤別（如 99）呼叫 HPHasSpace(99,0) 也回答 true | 呼叫端用 iNeedCells=0 探測超出範圍盤別是否「有空間」會得到誤導性的肯定答案，可能讓上層邏輯誤判不存在的 HotPlate 盤有可用空間而繼續操作 | 20260802 |
| 2 | GOLDEN QUIRK Q9 | QUIRK | cSiteUseManager.cpp:256 | golden cSiteUseManager.cpp:173-186 | ShadowLogBeforeSearch 沒有對 iPlate 做範圍檢查，超出範圍時 occupied/avail 回答 0，但 GetHPCapacity() 仍回答真實容量，記錄下自相矛盾的一行 log（如 occ=0 avail=0 cap=16） | 排查問題時看到這種自相矛盾的 log 行，可能誤以為資料毀損或計算錯誤，實際只是傳入無效盤別編號，容易浪費除錯時間在錯誤方向 | 20260802 |
| 3 | GOLDEN QUIRK Q3 | QUIRK | cSiteUseManager.cpp:338 | golden cSiteUseManager.cpp:251-253 | ValidateSearchResult 標為「Check 5」的區塊計算了 iAvail 與 iNozzle，卻完全沒做比較，兩值只拿去組摘要字串，永遠不會讓 bAnyWarn 變 true | 即使可用空格數（avail）小於需求噴嘴數（nozzle），驗證仍回報 "OK" 而非 "HAS_WARN"，該健康檢查形同虛設 | 20260802 |
| 4 | GOLDEN QUIRK Q4 | QUIRK | cSiteUseManager.cpp:340 | golden cSiteUseManager.cpp:253 | GetActiveInNozzleCount 在 Check 5 固定讀取 SHT 0 的噴嘴數，無論傳入 iIdx（要驗證的 shuttle 編號）是多少 | 驗證 shuttle 1（iIdx=1）時摘要顯示的其實是 shuttle 0 的噴嘴數，可能誤判 shuttle 1 自身的噴嘴使用狀況 | 20260802 |
| 5 | GOLDEN QUIRK Q1 | QUIRK | cSiteUseManager.cpp:397 | golden cSiteUseManager.cpp:301-321 | CompactSearchPlateToPlace 一開始就把 iNeedRow/iNeedCol 寫死為 1，使後面 bPitchOver12000 與 iNeedRow>=2 兩個條件區塊全變成死碼，iYStep 恆為 1 | HotPlateYPitchCanPutAll() 與 Row2CanPutHP(PlaceMode) 因此永遠不會被呼叫、iYHalf 永遠不被讀取，若想啟用多列一次性分配邏輯會發現完全不會被觸發 | 20260802 |
| 6 | GOLDEN QUIRK Q2 | QUIRK | cSiteUseManager.cpp:430 | golden cSiteUseManager.cpp:332/337 | 起始搜尋盤別 iStartPlate 直接沿用上一次搜尋殘留的全域 iPlacePlate[0]，未做範圍檢查就拿去做 % 2，殘留為負值時會讓 iP 變負 | iP 為負時 MOT[MMPlate1+iP] 會索引到 MMPlate1 之前的記憶體位置，等同存取錯誤的 motor/tray 資料結構，可能造成難以追查的資料錯亂 | 20260802 |
| 7 | GOLDEN QUIRK Q10 | QUIRK | cSiteUseManager.cpp:434 | golden :420（CanAnyNozzleReachHP 內）/ :333（CompactSearchPlateToPlace 內；原誤植為 :422/:335，20260802 已訂正） | CanAnyNozzleReachHP 與 CompactSearchPlateToPlace 只有當 iPlateSelect 恰好等於 0x03 才視為「雙盤模式」掃描兩盤，其餘任何值（包括指名 plate 1 的單盤選擇值）都只掃描 plate 0 或單一盤 | 若設定為指定使用 plate 1 的單盤模式（如 0x02），即便 plate 1 全空、plate 0 已滿，搜尋仍判定失敗，造成本可放置卻誤判為 HangUp 的情況 | 20260802 |
| 8 | GOLDEN QUIRK Q5 | QUIRK | cSiteUseManager.cpp:455, 520, 639 | golden cSiteUseManager.cpp:410-413（WillExceedMotorLimit 定義） | WillExceedMotorLimit 完全不使用三個傳入參數、永遠回傳 false，是 golden 自己刻意停用的馬達軟限位預檢查，使呼叫端的 continue 與診斷用 'M' 標記都永遠不會被觸發 | CompactSearchPlateToPlace 選位完全不會因馬達行程限位而跳過格子，若硬體確實存在無法到達的角落座標，程式仍會選中該位置並把風險轉嫁給下游真實移動指令 | 20260802 |
| 9 | GOLDEN QUIRK Q7 | QUIRK | cSiteUseManager.cpp:594 | golden cSiteUseManager.cpp:475-476 | 註解宣稱只示範顯示 "col 0 and col 7" 兩欄座標，但迴圈實際印出每一欄（0 到 iXDiv-1），註解與程式碼行為不一致 | 不影響功能，但只看註解的維護者會誤以為診斷 log 只列兩欄樣本，實際每次搜尋失敗都印出全部欄位，log 量比預期大 | 20260802 |
| 10 | GOLDEN QUIRK Q6 | QUIRK | cSiteUseManager.cpp:595 | golden cSiteUseManager.cpp:479 | 診斷用 iTgtY 計算寫成 iBaseY0 - Prod.HotPlateForm[0].iYPitch * 0，乘以字面常數 0 讓整個減項變無效運算，等同恆等於 iBaseY0 | 失敗診斷 log 中每一欄印出的 tgtY 座標其實都相同，無法反映真正的列別 Y 目標座標，可能誤導工程師以為所有欄位 Y 目標一致而干擾根因排查 | 20260802 |

### 【SCK_ART_Remainder 叢集】涵蓋：Automation/SCK_ART_Remainder.cpp(+.h)

### Automation/SCK_ART_Remainder.cpp

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG #1 (Save2DSortingSummary series) | BUG | Automation/SCK_ART_Remainder.cpp:1540,1564 | golden :3472-3480,:3492-3500 | 兩處位置各自以 MachineTypeChoice 判斷 sInfo_ProgramName 的 if/else-if 鏈，卻緊接著被無條件的 sInfo_ProgramName="HT9046LS" 覆寫，整條判斷鏈是死碼且重複出現兩次未合併 | 不論機型為 HT9045/HT9046/HT9046LS，2D Sorting Summary 檔頭的 PROGRAM_NAME 欄位一律被強制寫成 "HT9046LS"，對非 HT9046LS 機型永遠是錯誤標示 | 20260723 |
| 2 | GOLDEN BUG #2 (Save2DSortingSummary series) | BUG | Automation/SCK_ART_Remainder.cpp:1709 | golden :3636 | Str.sprintf("LOT_ID:%s", fLotInfo->edtSysLotID) 直接把 TEdit*/TfLotInfoEdit* 指標本身傳給 %s 轉換，漏掉 ->Text，讀到的是物件位址起始的原始位元組 | 屬未定義行為，輸出檔 LOT_ID 欄位會顯示亂碼位元組而非真正 Lot ID；W7-F0 把 TfLotInfoEdit 改成含 vptr 的多型物件後讀到的髒資料形狀也隨之改變，但兩者同屬未定義行為 | 20260723 |
| 3 | GOLDEN BUG #3 (Save2DSortingSummary series) | BUG | Automation/SCK_ART_Remainder.cpp:1740 | golden :3656/:3662 | SUMMARY_END_TIME 區塊重複檢查同一個 st.sLotStartTime，且其 else 分支格式字串仍寫成 "SUMMARY_START_TIME:..."，只是資料改用 RunInfo.LotEndTime | 當 sLotStartTime 為空時，輸出檔會出現兩行都標示 SUMMARY_START_TIME（第二行其實是結束時間資料），使報表閱讀者誤判兩者皆為開始時間 | 20260723 |
| 4 | GOLDEN BUG #4 (Save2DSortingSummary series) | BUG | Automation/SCK_ART_Remainder.cpp:2118 | golden :4025-4028 | ShellExecute 開啟 strFileName 的觸發條件與上方 bN17UploadLotSummary 區塊是否真的賦值給 strFileName 完全無關，該區塊被跳過時 strFileName 仍是預設空字串 | N-17 上傳功能關閉但其餘條件成立時，仍會以空字串呼叫 ShellExecute 開啟空路徑，與 SaveTestSummarySECS 的 gate #11 同一隱患 | 20260723 |
| 5 | GOLDEN BUG #1 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:2862 | golden :2051-2054 | sLotID1（含空 Lot ID 回退到 fLotInfo->edtSysLotID->Text 的邏輯）被賦值後全函式再也沒有被讀取，是一次死儲存 | 此死儲存不改變任何輸出，純粹是白費運算，不構成使用者可觀察的行為差異 | 20260802 |
| 6 | GOLDEN BUG #2(a) (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:2878 | golden :2059-2062 | int iUnloadCnt[eTrayCount] 的大括號初始化只列出32個0，但eTrayCount實際為33（MachineType.h e6TrayName 最後列舉 eMag14=32） | 單獨看無害（未列出元素仍被值初始化為0），但標記出下方 #2(b) 每輪重置迴圈同樣未擴大範圍的根因 | 20260802 |
| 7 | GOLDEN BUG #2(b) (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:2903 | golden :2074-2077 | 每個lot迭代前的重置迴圈 for(i<9) 只清空索引0..8，tray 9到32（eFix4~eMag14）不在重置範圍內，導致計數跨lot累加 | 依test PART14佈局，tray9(eFix4)的Group Bin Summary數字會跨批次持續累加：第一筆100，第二筆變成100+200=300（應為200），第三筆再變成300+400=700（應為400） | 20260802 |
| 8 | GOLDEN BUG #3 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:2893 | golden :2069-2072 | 在k==iInfo_MultiLotCnt合併總表最後一輪迭代、四個錯誤計數全為0時使用return而非break，使golden:2802無條件呼叫的LotSummary.ClearAllData()永遠不會執行 | 提前return會讓上一批次埋入的iTotalCategory[1]==777等殘留數值直接洩漏到下一個lot的統計中，造成跨批次資料污染 | 20260802 |
| 9 | GOLDEN BUG #4 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:2940 | golden :2106-2107 | bSaveData==false的提早return被放在st.sLotEndTime已覆寫、年月資料夾已建立、st.sInfo_ProgramName已重新賦值之後才發生，且同樣是return而非break | 依test PART14A實測，「不存檔」呼叫仍會寫入時間戳、改變sInfo_ProgramName、實際建立資料夾，且事先埋入的iTotalCategory[1]==99因ClearAllData被跳過而存活污染下一批次 | 20260802 |
| 10 | GOLDEN BUG #5 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:3058 | golden :2218 | 合併分支中OPERATOR_ID欄位用st.sInfoArr_OperatorID[k]組成，索引恰好比最後一個實際填入的lot槽位多一格，而同分支其餘每行都串接[0..iInfo_MultiLotCnt-1] | 合併總表的OPERATOR_ID欄位永遠空白；當iInfo_MultiLotCnt==5（陣列宣告大小正好是[5]）時，這是貨真價實的陣列越界讀取 | 20260802 |
| 11 | GOLDEN BUG #6 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:3408 | golden :2548,:2578 | Yield Summary迴圈中的iCount++是Hard Bin區塊遺留變數，在合併與by-lot分支都被遞增卻全函式再未讀取，是單純死遞增 | 不影響任何輸出數值或檔案內容，純粹是無意義運算殘留，對使用者無實際影響 | 20260802 |
| 12 | GOLDEN BUG #7 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:3505,3524 | golden :2641,:2654 | 合併與by-lot分支各有一句裸表達式讀取LotSummary陣列元素卻未賦值、未呼叫、無任何副作用，是被丟棄的讀取 | 純粹是無效運算，不影響任何輸出或狀態，以(void)轉型保留只是為了消除-Wunused-value警告 | 20260802 |
| 13 | GOLDEN BUG #8 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:3558 | golden :2688 | 存檔呼叫sList->SaveToFile(PathName+FileName)讀取不含"2"的PathName，而PathName在函式中只宣告從未被賦值（只有PathName2被賦值） | 此分支一旦觸發，檔案會被寫入空前綴的相對檔名，即寫進process當前工作目錄而非任何預期的summary資料夾，屬此函式獨有真缺陷 | 20260802 |
| 14 | GOLDEN BUG #9 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:3655 | golden :2778-2781 | ShellExecute開啟strFileName的條件與唯一真正賦值strFileName的if(bN17UploadLotSummary)區塊彼此獨立，N-17上傳關閉時strFileName仍是空字串 | 與Save2DSortingSummary、SaveTestSummarySECS共有的同一個「開空路徑」缺陷第三個出現點，N-17關閉但其餘條件成立時會以空字串呼叫ShellExecute | 20260802 |
| 15 | GOLDEN BUG #10 (SCKART-Multi series) | BUG | Automation/SCK_ART_Remainder.cpp:3262,3404,3438 | golden :2415-2418,:2542-2545,:2572-2575 | 三處相同樣式程式碼都以Prod.iTrayType[temp]（temp為1-based的iT6PosCate值）判斷tray是否在用，但同函式Group Bin區塊與姊妹函式都正確測試iTrayType[temp-1]，此處高了一格且下界用temp<0而非temp<=0 | 依test PART14佈局，GOLDEN BUG #10(a)使bin2因誤測未在用的tray2而從Hard Bin與Yield Summary消失卻仍計入Group Bin Summary；GOLDEN BUG #10(b)使未繞送的bin0仍取得一行全零Hard Bin列；兩者合計造成lot 0的Yield Summary顯示PASS 23/FAIL 100，與Handler Unload Qty(130)對不上 | 20260802 |

### Automation/SCK_ART_Remainder.h

（僅回引 .cpp 的項目，無新增）

### 【BarCode Shuttle2 叢集】涵蓋：BarCode/BarCode_Shuttle2_* 全家族

### BarCode/BarCode_Shuttle2_SFCAutoTune.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK Q1 | QUIRK | BarCode/BarCode_Shuttle2_SFCAutoTune.cpp:255；BarCode/BarCode_Shuttle2_SFCAutoTune.h:60-70 | golden BarCode_Sh2.cpp:4899（對照 sibling :4406） | DoSFCAutoTune_2 宣告 iSht=0，但其馬達是 MSht=MInShuttle2、CCD 是 iBarCode2_1/2_2，導致所有 iSht 索引查表全部讀到 Shuttle-1 的資料，日誌也印出 "Shuttle 1" | 真機執行 Shuttle-2 SFC Auto-Tune 掃描時會誤用 Shuttle-1 教導座標去驅動 Shuttle-2，可能造成對位偏移、曝光位置錯誤甚至撞機 | 20260811 |
| 2 | GOLDEN QUIRK Q2 | QUIRK | BarCode/BarCode_Shuttle2_SFCAutoTune.cpp:570；BarCode/BarCode_Shuttle2_SFCAutoTune.h:71-74 | golden BarCode_Sh2.cpp:5197 | case 10000 結尾沒有 break，直接 fall through 進入 case 11000，讓「設定完成」與「開始第一次移動」在同一個 tick 內執行 | 屬故意設計的立即動作模式；若日後在兩個 case 之間插入程式碼且未留意此 fall-through，可能導致初始化被重複或跳過 | 20260811 |
| 3 | GOLDEN QUIRK Q3 | QUIRK | BarCode/BarCode_Shuttle2_SFCAutoTune.cpp:778,784；BarCode/BarCode_Shuttle2_SFCAutoTune.h:75-85 | golden BarCode_Sh2.cpp:5401（Task=13000 無此 case）；:5404（case 11300 不可達） | 有料掃描 case 11200 結束時把 Task 設成 13000，但整個函式沒有 case 13000，且 case 11300 永遠不會被跳進去，造成有料 AT 掃描跑完最後一步後卡住且不會回傳 true | 操作員必須重新呼叫 InitialSFCAutoTune2() 才能讓有料 AT 掃描脫離卡住狀態，否則流程停滯不前 | 20260811 |
| 4 | GOLDEN QUIRK Q4 | QUIRK | BarCode/BarCode_Shuttle2_SFCAutoTune.cpp:536；BarCode/BarCode_Shuttle2_SFCAutoTune.h:86-90 | golden BarCode_Sh2.cpp:5166-5170（對照 case 11400 golden :5410） | 無料掃描 case 1400 印完結束 log 後又把 Task 設回 1300，讓 1300⇄1400 無限乒乓，每隔一個 tick 就重送一次馬達移動並回報 bResult=true；有料版本對應 case 11400 不會重設 Task | 無料 AT 循環永遠不會真正結束，會持續重送 MotorMove 到同一位置並不斷回報成功，可能造成馬達重複動作或呼叫端誤判掃描已完成多次 | 20260811 |
| 5 | GOLDEN QUIRK Q5 | QUIRK | BarCode/BarCode_Shuttle2_SFCAutoTune.cpp:824；BarCode/BarCode_Shuttle2_SFCAutoTune.h:91-95 | golden BarCode_Sh2.cpp:5422 | Do2DIDCheckSh2 同樣宣告 iSht=0，但除了 BarcodePosDelay[iSht]/BarcodeDelay[iSht] 兩個計時器外，其餘 Prod/Offset 查表都寫死索引 [1]，影響範圍僅限這兩個計時器誤用 Shuttle-1 的槽位 | 因兩個掃描目前不會同時執行，實務上暫無明顯異常，但屬與 Q1 同類的複製貼上瑕疵，日後行為改變便會互相干擾計時 | 20260811 |
| 6 | GOLDEN QUIRK Q6 | QUIRK | BarCode/BarCode_Shuttle2_SFCAutoTune.cpp:1013；BarCode/BarCode_Shuttle2_SFCAutoTune.h:96-103 | golden BarCode_Sh2.cpp:5602 | Do2DIDCheckSh2 非 OCR 分支中，Row-B 成功解碼後卻把結果寫進 Row-A 的表格儲存格，覆蓋 Row-A 自己的顯示內容，其餘分支都正確使用各自的 row | 操作員在 2D 讀碼確認畫面上會看到 Row-A 儲存格被 Row-B 條碼覆蓋，造成人工核對條碼判讀錯誤（僅影響顯示，cDeviceInf 實際資料正確） | 20260811 |
| 7 | GOLDEN QUIRK Q7 | QUIRK | BarCode/BarCode_Shuttle2_SFCAutoTune.cpp:1068；BarCode/BarCode_Shuttle2_SFCAutoTune.h:104-106 | golden BarCode_Sh2.cpp:5652 | case 1200 在「超出範圍」的 else 分支沒有 break，直接 fall through 進入 case 1300，同一 tick 內連續執行退回動作與馬達移動判斷 | 屬故意設計的立即動作模式；維護時若在中間插入程式碼而未留意，可能導致退回流程被跳過或重複執行 | 20260811 |
| 8 | GOLDEN QUIRK Q8 | QUIRK | BarCode/BarCode_Shuttle2_SFCAutoTune.cpp:842；BarCode/BarCode_Shuttle2_SFCAutoTune.h:107-111 | golden BarCode_Sh2.cpp:5434-5446（對照姊妹函式 golden :3072） | Do2DIDCheckSh2 開頭的列別切換只有二分支（if/else），缺少姊妹函式 DoBarcodeScanInShuttle_2 才有的 else if(BAR_CODE_INSTALL==ebcUseOCR) 分支 | 若機台設定為 OCR 讀碼模式，Do2DIDCheckSh2 的 Row A/B CCD 對應可能與實際硬體配置不符，導致操作員 2D 讀碼確認時選錯通道 | 20260811 |

### BarCode/BarCode_Shuttle2_SFCAutoTune.h
（見 BarCode/BarCode_Shuttle2_SFCAutoTune.cpp，同一缺陷；此 .h 的 banner 僅完整重述上表 Q1~Q8，無獨立新缺陷，故不重複列表）

### BarCode/BarCode_Shuttle2_CCDScan.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK | QUIRK | BarCode/BarCode_Shuttle2_CCDScan.cpp:507；BarCode/BarCode_Shuttle2_CCDScan.h:140-143 | golden BarCode.cpp:8271-8305（TfBarCode::bSHT_2DIDIsInsideList） | bSHT_2DIDIsInsideList 中，開頭的錯誤碼短路判斷用尚未去除 \r\n 的原始字串 Str，但下方白名單比對卻改用已裁切過的 Str1，兩者基準不一致 | 若條碼字串帶有尾端 \r\n，錯誤碼短路判斷可能因多了 \r\n 而比對失敗，使原本該判定為錯誤碼的情況改走白名單比對邏輯，造成白名單功能誤判 | 20260711 |

### BarCode/BarCode_Shuttle2_CCDScan.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK PRESERVED VERBATIM | QUIRK | BarCode/BarCode_Shuttle2_CCDScan.h:133-139（參照 BarCode/BarCode_Shuttle2_CCDScan.cpp:2339-2341,2371-2373） | golden BarCode_Sh2.cpp:84-2473（TfBarCode::DoBarcodeCCDInShuttle_2）case 4300 SPIL_FOR_QLE 清理迴圈；UNCERTAIN：註解未給出 case 4300 確切 golden 行號 | case 4300 的 SPIL_FOR_QLE 清理迴圈中，內層 if(InArmSuck.iShtRow==2 && Prod.bF18InshuttleDetect) 對同一旗標做了外層已檢查過的重複判斷 | 純冗餘判斷，不改變實際邏輯結果，風險極低，僅為程式碼可讀性上的 golden 瑕疵 | 20260711 |

（註：本檔 140-143 行另有 Str/Str1 基準不一致的重複記錄，已合併列在 BarCode_Shuttle2_CCDScan.cpp 的表格第1列，此處不重複）

### BarCode/BarCode_Shuttle2_ScanRemainder1.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRKS PRESERVED VERBATIM 項1 | QUIRK | BarCode/BarCode_Shuttle2_ScanRemainder1.h:110,112-115 | golden BarCode_Sh2.cpp:3074-3078（對照 sibling InitialBarcodeScanInShuttle2 缺此分支） | DoBarcodeScanInShuttle_2 比姊妹函式 InitialBarcodeScanInShuttle2 多出一段 OCR 列別選擇分支，兩者在同一組 row-selection 邏輯上不對稱 | 若機台切換到 OCR 讀碼模式，兩個理論上該一致的初始化/掃描函式對 Row A/B 的對應方式不同，可能導致選錯 CCD/OCR 通道 | 20260711 |
| 2 | GOLDEN QUIRKS PRESERVED VERBATIM 項2 | QUIRK | BarCode/BarCode_Shuttle2_ScanRemainder1.h:110,116-127 | golden BarCode_Sh2.cpp:3762-3810,3889-3938,3950-4006,4020-4061（對照 Shuttle-1 sibling DoBarcodeScanInShuttle_1 case 1180 有 IsNNMode()） | DoBarcodeScanInShuttle_2 的 case 1180 系列只用 InArmSuck.iShtRow==2 二分選擇 IndexSuckName[]，完全沒有呼叫 IsNNMode()，而 Shuttle-1 對應函式已有三分模式（NN_2Row/NN_1Row/else） | 這個較新的 NN 模式警報字串增強功能只套用到 Shuttle-1，Shuttle-2 未回補；若客戶機型使用 NN 模式，Shuttle-2 側的錯誤訊息 IndexSuckName 對應可能不正確 | 20260711 |
| 3 | GOLDEN QUIRKS PRESERVED VERBATIM 項3 | QUIRK | BarCode/BarCode_Shuttle2_ScanRemainder1.h:110,128-133 | golden BarCode_Sh2.cpp:4163（case 2000 前被註解掉的 break;） | case 1300 的 else 分支（iNowCheckStep 超出範圍）將 Task 設為 2000 後，golden 原始碼在緊接 case 2000 前有一行被註解掉的 break;，等同刻意讓程式落穿進入 case 2000 | 屬故意設計的立即動作模式；若誤以為缺漏的 break 是筆誤而補上，會改變原本每 tick 直接完成收尾的行為，屬高風險行為變更 | 20260711 |
| 4 | GOLDEN QUIRKS PRESERVED VERBATIM 項4 | QUIRK | BarCode/BarCode_Shuttle2_ScanRemainder1.h:110,134-139 | golden BarCode_Sh2.cpp（case 1→1000、case 1000→1100、case 3000→3100 fall-through；註解未給出確切行號） | case 1 落穿進 case 1000、case 1000 落穿進 case 1100、case 3000 落穿進 case 3100，均無 break，golden 原始就是同一 tick 內連續執行多段狀態機邏輯 | 屬本樹所有 DoXxx 狀態機共通的既有寫法而非個別錯誤，維護時仍須留意勿在中間插入程式碼造成非預期的重複/跳過執行 | 20260711 |

### BarCode/BarCode_Shuttle2_ScanRemainder2.cpp
（見 BarCode/BarCode_Shuttle2_ScanRemainder2.h，同一缺陷；此 .cpp:263 僅為指向 .h banner 的指標註解，無獨立新缺陷，故不重複列表）

### BarCode/BarCode_Shuttle2_ScanRemainder2.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK PRESERVED VERBATIM | QUIRK | BarCode/BarCode_Shuttle2_ScanRemainder2.h:39-52；BarCode/BarCode_Shuttle2_ScanRemainder2.cpp:257-264,419-454 | golden BarCode_Sh1.cpp:4224-4696（DoShuttleFloatCheck_1 有 IsNNMode() 三分）對照 golden BarCode_Sh2.cpp:4406-4878 case 1150/1350（DoShuttleFloatCheck_2，本函式，全篇無 IsNNMode()） | DoShuttleFloatCheck_2 的 case 1150/1350 組 ErrPart/sErrorPart 的 IndexSuckName 列選擇邏輯只用 InArmSuck.iShtRow==2 二分判斷，完全沒有姊妹函式 DoShuttleFloatCheck_1 已有的 IsNNMode() 三分邏輯 | NN 模式警報字串增強功能只套用到 Shuttle-1 的浮料檢查，Shuttle-2 版本未回補；若客戶機型使用 NN 模式，Shuttle-2 側 WAR0470/JAM0479 警報訊息中的 IndexSuckName 對應可能不正確 | 20260711 |

### BarCode/BarCode_Shuttle2_Scan.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK PRESERVED VERBATIM | QUIRK | BarCode/BarCode_Shuttle2_Scan.h:56-64 | golden BarCode_Sh2.cpp:2910,2930（case 3400）對照正確用法 case 1120 golden :2615,2629；函式 DoBarcodeTriggerInShuttle_2，golden BarCode_Sh2.cpp:2474-3038 | DoBarcodeTriggerInShuttle_2 的 case 3400 把結果寫入 iSH1_1BarcodePosition/iSH1_2BarcodePosition（Shuttle-1 的位置全域變數），而不是該用的 iSH2_1BarcodePosition/iSH2_2BarcodePosition，同檔案其餘用法都正確使用 SH2 版本 | Shuttle-2 觸發式掃描在此路徑上會誤寫 Shuttle-1 的條碼位置記錄，可能污染 Shuttle-1 定位/除錯資訊，或造成日後以此追蹤問題時誤判 | 20260711 |

### 【FTP/AGV/Automation 叢集】涵蓋：KYECFTP/FTPClient_* / Automation/AGV_* / Automation/AMR.cpp / Automation/MesWebService.h / Automation/SCK_TUTS.* / ATC/TCPData.cpp

### KYECFTP/FTPClient_Transfer.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | DISCOVERED GOLDEN QUIRK | QUIRK | KYECFTP/FTPClient_Transfer.cpp:172 | golden FTPClient.cpp:82-105 (dblFileSize, `if(!fname && !*fname)` 行未單獨標號) | dblFileSize() 對 fname 的空值防呆寫成 `if(!fname && !*fname) return 0.0;`，用 && 而非邏輯上該用的 \|\|，是 golden 原始判斷邏輯錯誤，port 逐字保留 | 目前呼叫端只傳 AnsiString::c_str()（不會是 NULL）故短路求值下不會真的解參考空指標；但未來若有新呼叫端直接傳可能為 NULL 的 const char*，會造成解參考 NULL 當機 | 20260711 |
| 2 | DISCOVERED GOLDEN QUIRK | QUIRK | KYECFTP/FTPClient_Transfer.cpp:1172（+ .h:60-73 STATE DESIGN 說明、.h:67 cross-ref） | golden FTPClient.cpp:1214（NMFTP3 全域變數）；Download_2DSortingList 本體 golden :4877-4953 | Download_2DSortingList() 收尾判斷 `if(NMFTP3!=NULL) delete NMFTP2;`，NMFTP3 是與本函式自己的 NMFTP2 完全無關的另一個檔案級全域變數，是明顯的複製貼上錯誤，port 逐字保留 | 只要 NMFTP3（常態為 NULL）沒被其他視窗流程碰巧設成非 NULL，本函式自己 new 出來的 NMFTP2 就永遠不會被 delete，等同每次呼叫都洩漏一個 TNMFTP 物件記憶體 | 20260711 |
| 3 | DISCOVERED GOLDEN QUIRK | QUIRK | KYECFTP/FTPClient_Transfer.cpp:1301 | golden FTPClient.cpp:1214（同一 NMFTP3）；Download_2DID_WhiteList 本體 golden :5267-5387；與第2列同一底層缺陷的第二個出現點 | Download_2DID_WhiteList() 收尾判斷與 Download_2DSortingList() 完全相同，同樣用不相干的全域 NMFTP3 決定要不要 delete 自己的 NMFTP2 | 同樣會在 NMFTP3 維持 NULL 的常見情況下造成本函式的 NMFTP2 記憶體洩漏，兩函式合計使洩漏發生機率更高 | 20260711 |

### KYECFTP/FTPClient_Transfer.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | DISCOVERED GOLDEN QUIRK（cross-ref） | QUIRK | KYECFTP/FTPClient_Transfer.h:67 | 同 FTPClient_Transfer.cpp | （見 FTPClient_Transfer.cpp 第2、3列，同一缺陷：NMFTP3 誤用） | 同上 | 20260711 |

### KYECFTP/FTPClient_EventHandlers.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | DISCOVERED GOLDEN QUIRK #4 | QUIRK | KYECFTP/FTPClient_EventHandlers.cpp:118 | golden FTPClient.cpp:1799-1829（NMFTP1ListItem；`AnsiPos("ATC_Recipe.zip")==0` 條件，"JerryYang 20190906" 註解） | FTPClientEvt_NMFTP1ListItem() 依笛摩根律展開後，只有同時含 ".Offset" 與 "ATC_Recipe.zip" 才會被排除，與註解原意（放行 ATC_Recipe.zip、其餘 .Offset 一律隱藏）相反，應為 !=0 而非 ==0 | 任何名稱含 ".Offset" 但不是 ATC_Recipe.zip.Offset.zip 的檔案都不會被過濾掉，FTP 檔案清單顯示會多出不該出現的 .Offset 檔案 | 20260711 |

### KYECFTP/FTPClient_EventHandlers.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | DISCOVERED GOLDEN QUIRK #1 | QUIRK | KYECFTP/FTPClient_EventHandlers.h:30 | golden FTPClient.cpp 綁定區塊 :137-145,429-437,4474-4482,4893-4901,5283-5291（NMFTP1xxx 實際綁在變數 NMFTP2 上）vs NMFTP2xxx 家族 :4111-4237，綁定於 :3664-3666,4038-4046 | NMFTP1Xxx 事件處理函式命名雖叫 NMFTP1，golden 實際綁在 OnXxx 屬性上的元件變數卻叫 NMFTP2，真正的 NMFTP2Xxx 家族反而掛在另一個 pFTP/NMFTP2 元件上，命名與實際綁定對不上 | 維護者若照函式名稱去找對應元件實例會找錯物件，未來修改這條 FTP 事件綁定時容易改壞另一組原本正常運作的靜默記錄流程 | 20260711 |
| 2 | DISCOVERED GOLDEN QUIRK #2 | QUIRK | KYECFTP/FTPClient_EventHandlers.h:47 | golden FTPClient.cpp:2036-2061（NMFTP1UnSupportedFunction，永不被綁定）；對照 sibling 綁定 :3664,:4046 | golden 裡 5 處事件綁定區塊完全相同，都沒有指定 OnUnSupportedFunction，使 NMFTP1UnSupportedFunction() 雖已完整實作卻是永遠不會被觸發的死函式 | 即使 FTP 伺服器真的回傳不支援的指令，golden 端也不會呼叫這個函式處理，該異常情境完全沒有被攔截或記錄 | 20260711 |
| 3 | DISCOVERED GOLDEN QUIRK #3 | QUIRK | KYECFTP/FTPClient_EventHandlers.h:64 | golden FTPClient.cpp:1987-1992 NMFTP1AuthenticationFailed, :2009-2035 NMFTP1Failure（`bool &Handled` 從未被賦值） | NMFTP1Failure 與 NMFTP1AuthenticationFailed 的 `bool &Handled` 輸出參數在函式本體內從未被賦值，連呼叫端 FireCmdFailure_ 自己也只初始化 false 後不再讀回，是個名副其實的假輸出參數 | 呼叫端無法透過 Handled 得知這兩個失敗事件是否已被自訂處理，未來想利用它做客製化攔截邏輯會發現從頭到尾都不生效 | 20260711 |

### Automation/AGV_PortScan.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | FAITHFUL GOLDEN QUIRK | QUIRK | Automation/AGV_PortScan.cpp:98（+ .h:147-155 摘要 cross-ref） | golden Automation/AGV.cpp:127-148 | E84StatusChange() 記錄 asE84StatusFlag[i][1][6]/[7] 診斷字串時，把 CONT 與 COMPT 兩個感測器狀態的文字標籤對調寫反，與 NowE84Status() 實際存進這兩個索引的資料相反（Loader i=0、Unloader i=1 兩段皆同） | 只影響寫入診斷 log 的文字說明，不影響機構動作，但工程師拿 log 判讀 CONT/COMPT 訊號時會被誤導，排查 E84 訊號問題容易導向錯誤結論 | 20260713 |
| 2 | FAITHFUL GOLDEN QUIRK | QUIRK | Automation/AGV_PortScan.cpp:721（+ .h:156-159 摘要 cross-ref） | golden Automation/AGV.cpp:1731-1733 | bScanLoadPortState_ATK() 判斷 Loader 側是否有料盤時，`MOT[MMTrayY].fHasTray` 被重複檢查兩次，研判原本該測第三顆馬達卻複製貼上漏改索引 | 實際上少檢查了一顆理應納入判斷的馬達的有盤狀態，若被漏掉的馬達才是真正該偵測的位置，可能造成 Loader 側料盤狀態誤判 | 20260713 |

### Automation/AGV_PortScan.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | FAITHFUL GOLDEN QUIRKS FLAGGED HERE（cross-ref） | QUIRK | Automation/AGV_PortScan.h:149-155 | 同 AGV_PortScan.cpp | （見 AGV_PortScan.cpp 第1列，同一缺陷：CONT/COMPT 標籤對調） | 同上 | 20260713 |
| 2 | FAITHFUL GOLDEN QUIRKS FLAGGED HERE（cross-ref） | QUIRK | Automation/AGV_PortScan.h:156-159 | 同 AGV_PortScan.cpp | （見 AGV_PortScan.cpp 第2列，同一缺陷：fHasTray 重複檢查） | 同上 | 20260713 |

### Automation/AGV_E84.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | FAITHFUL GOLDEN QUIRKS PRESERVED VERBATIM（bullet 1/5） | QUIRK | Automation/AGV_E84.cpp:79,80-87 | golden Automation/AGV.cpp:552-554,559-561 | DoE84Loader() case 2000 的兩條 else-if（iCount==1 / iPlaceWhichBuffer[0]==1）各自重複了同一梯狀判斷前一分支已測過的條件，使 "Color" 分支永遠不可能被走到 | 若客戶產線真的使用第三個（Color）AGV 緩衝區，確認對話框訊息對應欄位永遠只會顯示空字串，無法正確標示 Color 緩衝區名稱 | 20260711 |
| 2 | FAITHFUL GOLDEN QUIRKS PRESERVED VERBATIM（bullet 2/5） | QUIRK | Automation/AGV_E84.cpp:79,88-94 | golden Automation/AGV.cpp:614 vs :690,699,716,733,748,792,817,836 | DoE84Unloader() case 1 啟動 E84UnLoadDelay 用的是 iE84TimeOut_K12[0][9]（LOADER 的 TD0 欄位），而同函式其他所有逾時查詢全用 [1][...]，明顯是從 DoE84Loader 複製時忘了改索引 | Unloader 實際套用的是 Loader 側設定的逾時秒數，若兩者設定值不同，Unloader 的等待時間會與客戶設定不符 | 20260711 |
| 3 | FAITHFUL GOLDEN QUIRKS PRESERVED VERBATIM（bullet 3/5） | QUIRK | Automation/AGV_E84.cpp:79,95-98 | golden Automation/AGV.cpp:1088 vs :1090 | ShowE84Log() 先組出一份 "Loader Task: N Message: ..." 格式的 asStr 字串，卻在下一行馬上被另一格式的 str 覆蓋取代，asStr 從未被實際寫入 log | 純粹多做一次無意義的字串組裝，不影響實際寫入 log 的內容，但易讓維護者誤以為那個格式才是真正記錄格式 | 20260711 |
| 4 | FAITHFUL GOLDEN QUIRKS PRESERVED VERBATIM（bullet 4/5） | QUIRK | Automation/AGV_E84.cpp:79,99-102 | golden Automation/AGV.cpp:530-534 | DoE84Loader() case 900 有一段 if 判斷成立時的區塊完全沒有任何真正動作，只留了一則註解（"comment only"） | 這個分支目前對機台行為沒有任何實質影響，維護者若誤以為裡面藏有動作而依賴它會發現什麼都不會發生 | 20260711 |
| 5 | FAITHFUL GOLDEN QUIRKS PRESERVED VERBATIM（bullet 5/5） | QUIRK | Automation/AGV_E84.cpp:79,103-106 | golden Automation/AGV.cpp:591,670-677 | DoE84Unloader() 的 static bool bSensorStatus[3] 在 case 100 被計算寫入，但函式後續沒有任何地方再讀取它，是隻寫不讀的死狀態 | 這組陣列目前對任何後續行為都沒有影響，若維護者以為它會被拿來做某種狀態判斷而依賴其值，會發現寫入後根本沒人使用 | 20260711 |

### Automation/AMR.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | FAITHFUL GOLDEN QUIRKS（bullet 1/4） | QUIRK | Automation/AMR.cpp:39,40-42 | golden Automation/AMR.h（宣告 iAuto）vs AMR.cpp（定義用 iCate）— UNCERTAIN: 註解未給確切行號 | GetTrackBinData() 在標頭宣告的參數名稱是 iAuto，但 .cpp 實作卻用 iCate，屬於合法但名不符實的 golden 標頭/實作參數命名不一致 | 呼叫端只看得到標頭的 iAuto，若日後有人改 .cpp 內文並誤解 iCate 代表其他意義去擴充邏輯，可能誤判這個參數真正代表哪個 Auto 站 | 20260710 |
| 2 | FAITHFUL GOLDEN QUIRKS（bullet 2/4） | QUIRK | Automation/AMR.cpp:39,43-45 | golden Automation/AMR.cpp CheckUnloaderCount — UNCERTAIN: 註解未給確切行號 | CheckUnloaderCount() 的 if(iTrayFeed==1) 判斷式旁留著被註解掉的 \|\| iCleanOut==1，golden 自己沒有復活它，port 原樣保留死註解 | 目前 iCleanOut==1 情境不會被此判斷式涵蓋；若當初意圖真是兩者都要算，port 忠實延續 golden 現有（可能不完整）的行為，不擅自補上 | 20260710 |
| 3 | FAITHFUL GOLDEN QUIRKS（bullet 3/4） | QUIRK | Automation/AMR.cpp:39,46-47 | golden Automation/AMR.cpp SupplyCover（"Sam20250625 fix" 註解）— UNCERTAIN: 註解未給確切行號 | SupplyCover() 裡 ART 分支的替代處理內容整段被 golden 自己註解掉，port 同樣只當註解保留，不執行也不復原 | 目前 ART 分支不會執行那段被停用的替代邏輯，行為與 golden 出貨版一致，但代表當初那次 fix 並未真正生效在執行路徑上 | 20260710 |
| 4 | FAITHFUL GOLDEN QUIRKS（bullet 4/4） | QUIRK | Automation/AMR.cpp:39,48-50 | golden Automation/AMR.cpp ClearTrackBinData — UNCERTAIN: 註解未給確切行號 | ClearTrackBinData() 對每一個 iAuto 都用 iAutoPos!=iPos -> continue 重新掃過同一張 iCatDataT3Pos[] 表找對應項目，而非用別名/索引表快速定位，與 GetTrackBinData() 掃法一致但效率較差 | 站點數量或呼叫頻率提高時，這種逐筆比對會比查表法多花 CPU 時間，但目前規模下不影響行為正確性，屬忠實保留而非效能最佳化 | 20260710 |

### Automation/MesWebService.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK preserved VERBATIM | QUIRK | Automation/MesWebService.h:45（+ 同檔 line 108 自我 cross-ref） | golden MesWebService.h :48（#endif）先於 :49（namespace 收尾大括號）；:51-53（using namespace）在 guard 之外 | `#endif // __MesWebService_h__`（golden :48）比 namespace NS_MesWebService{...} 自己的收尾大括號（golden :49）先結束，導致 :49 的 }; 與 :51-53 的 using namespace 其實落在 include guard 保護範圍之外 | 只要這支標頭在同一編譯單元被 #include 超過一次，就會因命名空間收尾與 using namespace 未被 guard 擋下而重複執行造成編譯錯誤；目前因 MesWebService.cpp 只 include 一次而未觸發 | 20260807 |

### Automation/SCK_TUTS.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK preserved VERBATIM | QUIRK | Automation/SCK_TUTS.h:82（+ 同檔 line 163 自我 cross-ref） | golden SCK_TUTS.h :51（#endif）先於 :52（namespace 收尾大括號）；:54-56（using namespace）在 guard 之外 | #endif // __sip1_h__（golden :51）同樣搶先在 namespace NS_sip1{...} 收尾大括號（golden :52）之前結束，使 :52 的 }; 與 :54-56 的 using namespace 落在 include guard 之外，與 MesWebService.h 同型瑕疵 | 任何 TU 對此標頭做第二次 #include 就會因命名空間結尾與 using 宣告未被 guard 擋下而重複展開造成編譯失敗；目前 SCK_TUTS.cpp 自行加了一層 #if !defined(SCK_TUTSH) 包裝擋住了這個地雷 | 20260807 |

### Automation/SCK_TUTS.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK（cross-ref） | QUIRK | Automation/SCK_TUTS.cpp:85 | 同 SCK_TUTS.h | （見 SCK_TUTS.h 第1列，同一缺陷：include guard 置放位置） | 同上 | 20260807 |

### ATC/TCPData.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK preserved verbatim | QUIRK | ATC/TCPData.cpp:88 | golden TCPData.cpp :236, :455 (`data.push_back( ERROR );`) | TCPIP 相關函式中 data.push_back( ERROR );（golden :236, :455）用的是裸露的 Win32 巨集 ERROR（wingdi.h #define ERROR 0），而非本檔自訂的 TD_ERROR 巨集，兩者數值恰好都是 0 純屬巧合 | 目前 ERROR 與 TD_ERROR 剛好都等於 0，在 BCB6 與本次 port 用的 MinGW 上行為一致沒有實際風險；但若未來 wingdi.h 的 ERROR 定義改變，或此檔在未 include windows.h 的環境編譯，這行會編譯失敗或數值不再等於 TD_ERROR | 20260807 |


Note: rows marked "cross-ref"/"（見 ...，同一缺陷）" (FTPClient_Transfer.h #1, AGV_PortScan.h #1-2, SCK_TUTS.cpp #1) are stub pointers per your merge rule and are NOT counted separately in TOTAL — they're already counted once in their sibling file's row. No GATE items included anywhere above.

### 【MainCalcCore/生產資訊叢集】涵蓋：MainCalcCore.cpp(+.h) / Public/ExternFunction.cpp / ProductionInfo/FileInfo.cpp / ProductionInfo/uPAT_Function.cpp / cUnitConvert.cpp / cContact.cpp(+.h)

### MainCalcCore.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK | QUIRK | MainCalcCore.cpp:467-469 | golden main.cpp:32051-32064(字面在:32063) | ComputeJamRateRecordStrings 在 JamCount!=0 分支硬編碼字面"MTBF 1/"，並非真正的 iRecordJamRateByTime_JamCount 值 | 顯示的 Jam Rate Record 訊息「MTBF 1/...」永遠顯示數字1而非真實 Jam 次數，操作員可能誤判卡料頻率統計 | AI(W906-maincalccore) 20260727(batch 3) |
| 2 | GOLDEN QUIRK | QUIRK | MainCalcCore.cpp:479-481 | golden main.cpp:32060-32063 | 同函式的 fH 公式實際計算「每小時卡料次數」比率，卻標示為 MTBF(應為每次故障間隔小時數)，語意與計算相反 | 若依 MTBF 原始定義解讀(愈大代表愈少故障)，會得到與實際卡料頻率相反的結論 | AI(W906-maincalccore) 20260727(batch 3) |
| 3 | GOLDEN QUIRK | QUIRK | MainCalcCore.cpp:527-529 | golden main.cpp:34528-34550(34532-34540三分支間空隙) | ComputeSetESDTriTempCommand 在 Hot 且 fWorkTemperBase 落在 [10,40) 時三分支皆不成立，golden 完全不呼叫 SendCommand_ESD()，此處回傳 kNoESDTriTempCommand 哨兵值 | 溫度在10~40°C之間的樣本不會觸發任何 ESD 指令切換，可能讓 ESD 溫度模式停留在錯誤狀態而未被察覺 | AI(W906-maincalccore) 20260728(batch 4) |

### MainCalcCore.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK | QUIRK | MainCalcCore.h:212-217 | golden main.cpp:31187-31208 | ComputeSiteMapPriority 中追蹤 iAutoSiteMap 最大值的 iMax 在迴圈後從未被讀取，對回傳值毫無影響，是 golden 自身的死追蹤變數 | 純屬無效運算不影響功能正確性，但可能誤導維護者以為 iMax 有用途 | AI(ht9045-v899) 20260722(batch 1，依檔案位置推斷，未在 batch2 具名清單中) |
| 2 | GOLDEN QUIRKS | QUIRK | MainCalcCore.h:545-555 | golden main.cpp:32063(缺陷1)、main.cpp:32060(缺陷2) | 對應 MainCalcCore.cpp 兩則 JamRate 缺陷之正式文件化版本 | 與 MainCalcCore.cpp 對應行相同 | AI(W906-maincalccore) 20260727(batch 3) |
| 3 | GOLDEN QUIRK | QUIRK | MainCalcCore.h:596-603 | golden main.cpp:34532-34548 | 對應 MainCalcCore.cpp 的 ESD [10,40) 空隙缺陷之文件化版本，並記載 AmbientHot 分支與 catch-all else 回傳相同指令 | 與 MainCalcCore.cpp 對應行相同 | AI(W906-maincalccore) 20260728(batch 4) |

### Public/ExternFunction.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK | QUIRK | Public/ExternFunction.cpp:290-294 | golden Public/ExternFunction.cpp:25,27,34,36,42,44(Insert_Row)/:66,68,74,76(Delete_Row) | StringGrid 搬移迴圈欄位邊界取自暫存 TempGrid 預設幾何(ColCount5-FixedCols1=4欄)而非 pSG 本身欄數，永遠只搬移第1~4欄 | 若呼叫端傳入超過5欄的 StringGrid，第5欄以後資料在插入/刪除列時不會被搬移，可能造成資料錯位 | PT-W8 20260811 |
| 2 | GOLDEN QUIRK PRESERVED | QUIRK | Public/ExternFunction.cpp:402-408 | golden Public/ExternFunction.cpp:8-51 | StringGrid_Insert_Row 因上述邊界問題，對超過5欄的 pSG 靜默漏搬移第5欄以後資料，對少於5欄的 pSG 索引超界 | 欄數與預期不符的表格在插入列時可能靜默資料遺失或丟例外 | PT-W8 20260811 |
| 3 | GOLDEN QUIRKS PRESERVED | QUIRK | Public/ExternFunction.cpp:472-482 | golden Public/ExternFunction.cpp:81,89,94,101,107,112,114 | StringGrid_Delete_Row 兩缺陷並存：(a)搬移迴圈只動第1~4欄但清空最後一列迴圈正確用 pSG->ColCount 從第0欄開始；(b)三分支回傳的 iMaxRow 計算時機不同 | 多欄表格刪除列時最後一列全部欄位被清空但只有4欄真正搬移，可能造成資料與欄位對不齊 | PT-W8 20260811 |
| 4 | GOLDEN QUIRKS PRESERVED | QUIRK | Public/ExternFunction.cpp:585-592 | golden Public/ExternFunction.cpp:222-246 | ShowRecordTime 對 iType 非0/1/2時無防呆直接存原始值；空白 Edit 用 atoi 轉換得0，導致首次 Min. 模式呼叫把數值鉗制到0；變數名 iMSec 實際單位是 clock() tick 而非毫秒 | 顯示的耗時數字可能被誤判為毫秒，首次「最小值」讀數固定被拉低到0，影響操作員判讀 | PT-W8 20260811 |

### ProductionInfo/FileInfo.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK 1 | QUIRK | ProductionInfo/FileInfo.cpp:90-95,254 | golden ProductionInfo/FileInfo.cpp:90-95 | DecodeASCII(char*,int,int) 用 AnsiString(cStream) 而非帶長度版本建構字串，cStream 從未補 NUL 結尾 | 若緩衝區內容剛好不含 NUL，會發生 buffer over-read，可能夾帶不相關記憶體內容或造成當機 | AI(W906-PT-W2) 20260807 |
| 2 | GOLDEN QUIRK 2 | QUIRK | ProductionInfo/FileInfo.cpp:82-87,96-97,273 | golden ProductionInfo/FileInfo.cpp:113 | DecodeASCII(AnsiString,int) 中 char *a 未初始化，若開檔失敗仍無條件執行 delete[] a | 開檔失敗時可能造成記憶體損毀或程式崩潰 | AI(W906-PT-W2) 20260807 |
| 3 | GOLDEN QUIRKS/BUGS(項3) | QUIRK | ProductionInfo/FileInfo.cpp:98-107 | golden ProductionInfo/FileInfo.cpp:154,183 | GetAllFileNamesInFolder/GetAllFolderNamesInFolder 傳給 FindFirst 的 attr 遮罩對結果無篩選作用，真正篩選靠迴圈內二次檢查 | 純屬冗餘，因迴圈內有正確二次過濾行為仍正確，僅易誤導閱讀者 | AI(W906-PT-W2) 20260807 |
| 4 | GOLDEN QUIRKS/BUGS(項4) | QUIRK | ProductionInfo/FileInfo.cpp:108-112 | golden ProductionInfo/FileInfo.cpp:214 | DeleteFolderContents 用全新建構的暫存 FileInfo() 呼叫 GetAllFileNamesInFolder，而非用 this | 目前無實際功能影響(該函式未讀取實例成員)，屬程式碼寫法怪異 | AI(W906-PT-W2) 20260807 |
| 5 | GOLDEN QUIRKS/BUGS(項5) | QUIRK | ProductionInfo/FileInfo.cpp:113-119 | golden ProductionInfo/FileInfo.cpp:458(有guard)vs:440(無guard) | GetRowTitle 讀取前有 iRow<recordCount 邊界檢查，但對稱函式 GetColTitle 卻無對應 iCol 邊界檢查 | 傳入過大 iCol 呼叫 GetColTitle 會觸發 std::out_of_range 例外，可能造成未預期當機 | AI(W906-PT-W2) 20260807 |

### ProductionInfo/uPAT_Function.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN DEFECTS DELIBERATELY PRESERVED(項1) | DEFECT | ProductionInfo/uPAT_Function.cpp:55-58 | golden :487，CalculateTotal()於:479 | GetOutputCount 印「Total」列用 toutputCnt.iAuto1 而非剛算出的 iTotal，明顯 copy-paste 疏漏 | 報表「Total」欄實際等於 Auto1 站數量而非真正加總值，可能誤導生產總數判讀 | AI(W906-PT-W4-prodinfo) 20260808 |
| 2 | GOLDEN DEFECTS DELIBERATELY PRESERVED(項2) | DEFECT | ProductionInfo/uPAT_Function.cpp:59-70 | golden :551，:582 | 兩函式迴圈用 idata<=MAX_BINS(16次)寫入 Bins[idata]，Bins[15]越界一格恰好覆寫下一成員 Total，隨即被 CalculateTotal() 覆蓋回正確值 | 目前因記憶體佈局巧合無害，但迴圈邊界與 struct 佈局需一起修正，單獨修任一邊會變成真正越界寫入 | AI(W906-PT-W4-prodinfo) 20260808 |
| 3 | GOLDEN DEFECTS DELIBERATELY PRESERVED(項3) | DEFECT | ProductionInfo/uPAT_Function.cpp:71-72 | golden :1546 | 格式化字串用"%s%"，結尾多一個無對應轉換的孤立'%' | 輸出字串結尾可能多出不必要的 % 符號，僅顯示格式瑕疵 | AI(W906-PT-W4-prodinfo) 20260808 |
| 4 | GOLDEN DEFECTS DELIBERATELY PRESERVED(項4) | DEFECT | ProductionInfo/uPAT_Function.cpp:73-79 | golden :826 | 呼叫 ShowMyMessage 時傳入類 printf 的"%f"與兩個 double，但該函式非 printf 風格，"%f"字樣被原樣顯示 | 操作員看到訊息含未替換的"%f"文字而非格式化溫度值，可讀性下降 | AI(W906-PT-W4-prodinfo) 20260808 |
| 5 | GOLDEN DEFECTS DELIBERATELY PRESERVED(項5) | DEFECT | ProductionInfo/uPAT_Function.cpp:80-82 | MachineType.h:1601 | GetAverageTestTime/GetAveragePauseTime/GetDoublePercentStr 將 double 傳入回傳 float 的函式，精度被降階 | 顯示的平均時間/百分比可能有極小 float 精度誤差 | AI(W906-PT-W4-prodinfo) 20260808 |
| 6 | GOLDEN DEFECTS DELIBERATELY PRESERVED(項6) | DEFECT | ProductionInfo/uPAT_Function.cpp:83-84 | golden :422,429,613,707 | SecondsToDateTime 參數為 int，呼叫端傳 double，數值在呼叫時被截斷 | 秒數時間換算遺失小數部分，長時間累計可能產生輕微誤差 | AI(W906-PT-W4-prodinfo) 20260808 |
| 7 | GOLDEN DEFECTS DELIBERATELY PRESERVED(項7) | DEFECT | ProductionInfo/uPAT_Function.cpp:85-86 | golden :615 | GetAveragePauseTime 被標示為「Average Pure Test Time」，實際計算的是暫停時間平均值 | 報表標題誤導，可能影響機台稼動率判讀 | AI(W906-PT-W4-prodinfo) 20260808 |
| 8 | GOLDEN DEFECT INSIDE THE GATE | DEFECT | ProductionInfo/uPAT_Function.cpp:415-431(重點426-430) | golden :1203-1205，對照:1197-1198 | GetJobContactToSetting 中 edForcePerPinG->Text 被賦 TheNoOfPins、edPinCount->Text 被賦 gf，兩欄位與名稱代表的資料互相對調 | 此段目前被 GATE[G8] 擋住未接線；一旦補上真正 fContact UI 並解除 gate，若不處理，下載的 pin 數與力量值會顯示錯位 | AI(W906-PT-W4-prodinfo) 20260808 |

### cUnitConvert.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK PRESERVED | QUIRK | cUnitConvert.cpp:463-466 | golden cUnitConvert.cpp:124,126(OUT臂)vs:152,154(SORT臂) | DoArmOffsetConvert 中 SORT臂內層迴圈正確用 OutArm2Suck，但 OUT臂內層迴圈卻誤用 InArmSuck 而非 OutArmSuck | 若 InArmSuck 與 OutArmSuck 矩陣尺寸不同，OUT臂偏移量搬移範圍與其真實吸嘴矩陣不符，可能索引越界或漏轉換 | PT-W8 20260811 |
| 2 | GOLDEN QUIRK PRESERVED | QUIRK | cUnitConvert.cpp:594-596 | golden cUnitConvert.cpp:197 | DoArmSpeedConvert memcpy 複製 dRetryDown 後，下一行立刻用縮放值覆寫，該複製是多餘動作 | 純屬無害冗餘運算，不影響最終數值正確性 | PT-W8 20260811 |
| 3 | GOLDEN QUIRK PRESERVED | QUIRK | cUnitConvert.cpp:620-624 | golden cUnitConvert.cpp:220-227 | DoDefFormConvert 中 iPickUp 範圍鉗制結果寫回 recipe 端的 UserDefForm_File[i] 而非即時結構，鉗制效果具跨呼叫累積黏滯性 | recipe 中的 iPickUp 一旦被鉗制即永久改變並在下次存檔保留，無法單純從即時值回推原始 recipe 設定 | PT-W8 20260811 |

### cContact.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK | QUIRK | cContact.cpp:386-387, cContact.h:272-276 | golden cContact.cpp:18453-18540(cpp端引用為18455-18539) | ComputeTestZCompensationHight 當 dKitDiameter 非8或40.2時，整個函式本體完全不執行，iTotalOffset_1/2 完全不被觸碰或歸零 | 呼叫端若期待每次呼叫都得到新計算值，在非8/40.2套件直徑下會誤用上一輪殘留的舊補償值 | AI(W906-cContactLeaf) 20260721 |

### cContact.h
（見 cContact.cpp，同一缺陷）

### 【vclcompat + BinSelect 周邊表單叢集】涵蓋：vclcompat/TrayCore.* / vclcompat/LedCore.* / vclcompat/BtnPanelCore.* / forms/fObserver.h / forms/FormWidgets.h / cBinSel.cpp / forms/fBinSel.h / cSecurity.cpp / forms/fSecurity.h / cCounterClear.cpp

### vclcompat/TrayCore.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG | BUG | vclcompat/TrayCore.cpp:94 | golden HTray.cpp:133-136 | `SetYItem` 只重置 `bXVisible[]`，從未重置 `bYVisible[]`，即使此函式改的是 Y 軸；`SetBlockYItem` 也有同樣的複製貼上錯誤，故本類別任何 setter 都不會重置 `bYVisible[]`。 | 若程式先呼叫 `SetYVisible` 隱藏某些列，之後改變 Y 軸格數，被隱藏的列狀態不會被清除，可能導致 Tray 顯示的可見列與實際格數不同步。 | W906-W7-C1, 20260728 |
| 2 | GOLDEN BUG | BUG | vclcompat/TrayCore.cpp:133 | golden HTray.cpp:176-179 | `SetBlockYItem` 與 `SetYItem` 同樣的複製貼上瑕疵：明明是修改 Y 軸的 block 設定，卻重置 `bXVisible[]` 而非 `bYVisible[]`。 | 與上一項同一根因、同一風險（Y 軸可見性狀態與格數變更不同步），屬同一 bug 的第二個發生點。 | W906-W7-C1, 20260728 |
| 3 | GOLDEN BUG | BUG | vclcompat/TrayCore.cpp:200 | golden HTray.cpp:262-270 | `CaculateTrayParameter` 對 `FXVisibleItem`/`FYVisibleItem` 沒有防呆，若透過 `SetXVisible`/`SetYVisible` 把整列/整欄全部隱藏成 0，會造成整數除以零。 | 若操作把 Tray 所有列或欄都設為不可見，程式在計算格子座標時會發生除以零的當機風險。 | W906-W7-C1, 20260728 |
| 4 | GOLDEN BUG | BUG | vclcompat/TrayCore.cpp:223 | golden HTray.cpp:241-242 | `CellRect` 沒有防呆 `FXBlock>FXItem`（或 `FYBlock>FYItem`），會造成整數除以零。 | 若 block 設定值超過總格數，計算格子矩形座標時可能除以零當機。 | W906-W7-C1, 20260728 |
| 5 | GOLDEN BUG | BUG | vclcompat/TrayCore.cpp:237 | golden HTray.cpp:437-454 | `ConvertIndexCells` 完全忽略 `CellRect`/`DrawSingleIC` 有套用的 `iBX`/`iBY` block 偏移量，導致當 XBlockItem/YBlockItem > 1 時，命中測試（hit-test）映射錯誤。 | 點擊/命中測試在有 block 分區的 Tray 上，會落在錯誤的格子索引或完全落空；golden 有 16 個呼叫點依賴這個（有瑕疵的）映射行為，故不可修正。 | W906-W7-C1, 20260728 |
| 6 | GOLDEN BUG | BUG | vclcompat/TrayCore.cpp:244 | golden HTray.cpp:437-454 | `ConvertIndexCells` 在提前 return -1 之前就已經把傳入的 X/Y 原地修改，若呼叫端忽略回傳值又重用 X/Y，會拿到部分轉換過的座標而非原始座標。 | golden 所有呼叫點事前都有檢查回傳值，故此瑕疵目前不可觀察，但屬於介面契約上的隱性風險。 | W906-W7-C1, 20260728 |
| 7 | GOLDEN BUG | BUG | vclcompat/TrayCore.cpp:427 | golden HTray256.cpp:169-177 | `Tray256Core::CaculateTrayParameter` 與 TrayCore 相同的除以零防呆缺失，但因 `SetXItem/SetYItem` 會把 0 夾到 1，實務上無法透過公開介面觸發。 | 為求與 TrayCore 對稱而記錄，目前透過此類別自身的公開介面不可獨立觸發，風險等級低。 | W906-W7-C1, 20260728 |

### vclcompat/TrayCore.h
（無符合條件的項目）— 本檔 6 個命中皆為指向 TrayCore.cpp 對應行號的單行參照註解（如「GOLDEN BUG, see .cpp」），非獨立發現，已併入上表。

### vclcompat/LedCore.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG (behaviour-preserved-not-UB-preserved) | BUG | vclcompat/LedCore.cpp:274-289 | golden MyLedLane.cpp WriteRing/WriteIP/WritePort/WriteBit :74-104 | golden 用固定大小的 `char buf[N]`（N=2 或 3）配 `strcpy` 再手動補 NUL，對超長字串會造成真正的堆疊緩衝區溢位（UB）；此移植保留「觀察得到的行為」（截斷成前 N-1 字元）而非重現真正的記憶體越界寫入。 | 若未來有異常長的 Ring/IP/Port/Bit 設定字串，golden 上會是未定義行為（可能當機或資料毀損），此移植改為確定性截斷，行為看似一致但底層機制已不同，須留意此為刻意的「保留可觀察行為、不保留 UB」決策。 | W906-W7-C1, 20260728 |

### vclcompat/LedCore.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG | BUG | vclcompat/LedCore.h:136 | golden MyLed.h:32-34 + MyLed.cpp:19-29/72-106 | `MyLedCore`（golden `TMyLed`）的 `Port`/`Bit`/`Type` 屬性宣告為 read=sInPort 等，但 `WritePort`/`WriteBit`/`WriteType` 從未真的賦值給這些欄位，只有建構子時設過一次空字串。 | golden 端讀取 `->Port`/`->Bit`/`->Type` 永遠只會得到空字串，不管之前寫入過什麼；此移植刻意不提供對應 getter 以避免比 golden 更「能幹」。 | W906-W7-C1, 20260728 |
| 2 | GOLDEN BUG | BUG | vclcompat/LedCore.h:179 | golden MyLedLane.h:41-47 + MyLedLane.cpp:19-35/74-111 | `MyLedLaneCore`（golden `TMyLedLane`）的 `Ring`/`IP`/`Port`/`Bit`/`Type` 五個屬性同樣是 write-only 陷阱，setter 從未寫回對應 backing field；但 `Alias`/`IsISA` 兩者是真的有正確賦值（非對稱）。 | 這顆自訂控制項在整棵樹有 1214 個 .dfm 實例，是全樹數量最多的自訂控制項，write-only 陷阱影響面廣但因從無人讀取這些字串屬性而不可觀察。 | W906-W7-C1, 20260728 |

### vclcompat/BtnPanelCore.cpp
（無符合條件的項目）— 本檔 3 個命中皆為指向 BtnPanelCore.h 對應 class banner 的參照註解，非獨立發現，已併入下表。

### vclcompat/BtnPanelCore.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG | BUG | vclcompat/BtnPanelCore.h:101 | golden butPa1.cpp:21-22 AND BtnPanelLane.cpp:23-24 | `BtnPanelCoreBase` 共用建構子裡宣告了同名的區域變數（LOCAL shadow）`tcTrueColor`/`tcFalseColor`，導致真正的 protected 成員從未在建構子中被賦值；`tcTrueFontColor`/`tcFalseFontColor` 更是完全沒被觸碰。 | 這 4 個成員在第一次呼叫 `SetTrueColor`/`SetFalseColor`/`SetTrueFontColor`/`SetFalseFontColor` 之前都是未初始化的隨機記憶體；此移植用確定性的哨兵值 `0xDEADBEEF` 取代真實 UB，讓行為可測試。 | W906-W7-C1, 20260728 |
| 2 | GOLDEN BUG | BUG | vclcompat/BtnPanelCore.h:133 | golden butPa1.h:37-39+:60-63, butPa1.cpp:18-51/156-190 | `BtnPanelCore`（golden `TBtnPanel`）的 `Port`/`Bit`/`Type` 屬性同樣是 write-only 陷阱，與 LedCore.h 的 `MyLedCore` 是同一類瑕疵。 | golden `->Port`/`->Bit`/`->Type` 讀取永遠回空字串；56 個 .dfm 實例受影響但因無人讀取而不可觀察。 | W906-W7-C1, 20260728 |
| 3 | GOLDEN BUG | BUG | vclcompat/BtnPanelCore.h:177 | golden BtnPanelLane.h:15-19,58-68 + BtnPanelLane.cpp:20-59/132-155 | `BtnPanelLaneCore`（golden `TBtnPanelLane`）的 `Lane`/`IP`/`Port`/`Bit`/`Type` 五個屬性同樣是 write-only 陷阱（`Alias`/`IsISA` 例外，有正確賦值）。 | 701 個 .dfm 實例（全樹第二多的自訂控制項）受影響但不可觀察，同 LedCore 的對應瑕疵。 | W906-W7-C1, 20260728 |

### forms/fObserver.h
（無符合條件的項目）— 唯一命中（line 161）為重述 TrayCore.h 既有 `SetYItem`/`ConvertIndexCells` bug 的設計說明，非新發現，非 GATE。

### forms/FormWidgets.h
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG | BUG | forms/FormWidgets.h:65-77 | Automation/SCK_ART_Remainder.cpp:1651（golden SCK_ART.cpp:3636） | `Str.sprintf("LOT_ID:%s", fLotInfo->edtSysLotID)` 把物件指標本身傳給 `%s`（漏了 `->Text`），golden 讀到的是把物件位址開始的原始位元組當成 C 字串；本檔記錄的是「型別別名化後讀到的位元組內容改變了」這個副作用（以前是 AnsiString 內部的 std::string，現在是 vptr），但兩者本質都是「把指標位元組當字元讀」的同一類未定義行為。 | 該分支被刻意設計成不會被任何測試執行到（`bReadLotInfoFromART==true` 已避開），故此 UB 目前無實際觸發風險，但屬已知、已審查過的技術負債。 | W906-W7-F0, 20260728 |

### cBinSel.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG (B14) | BUG | cBinSel.cpp:2244-2250 | golden cBinSel.cpp:5301 | 每個 tag 的 `AOIBinTraySetting` ini key 都用固定字面值 `"AOIBinTraySetting"`，唯獨 `FT_MRT` 這個 tag 把 `GroupName` 串接上去變成 `"Bin Func MRT_FTAOIBinTraySetting"`，讀寫的是完全不同的 ini key。 | `FT_MRT` 這組設定與其他 tag 用不同的 ini key 存取，切換設定檔或跨機台複製設定時，該 tag 的 AOI 分 bin 設定會靜默讀到/寫入錯誤位置。 | W906-FW-BinSel-WB, 20260819 |
| 2 | GOLDEN BUG (B15) | BUG | cBinSel.cpp:2296-2300 | golden cBinSel.cpp:5347 | `MRT_RT` tag 的同一個設定又是第三種變體：`GroupName + " AOIBinTraySetting"`（多一個前導空白），與 (B14) 的 `FT_MRT` 變體也不同，三個 tag 三種 key 格式互不一致。 | 同上，`MRT_RT` 這組的 AOI 分 bin 設定也是獨立不一致的 ini key，跨版本/跨機型比對設定檔時容易誤判為遺失資料。 | W906-FW-BinSel-WB, 20260819 |
| 3 | GOLDEN QUIRK (B16) | QUIRK | cBinSel.cpp:3173-3183 | golden cBinSel.cpp :4094/:4106 | `mtTrayNameSetColor` 內兩處呼叫（`SetCellNumber(eItemART,...)`/`SetCellNumber(eItemCateR,...)`）漏了本方法其他所有呼叫都有的 `MyBinPanel[tag]->` 前綴，改成寫到 `TfBinSel` 自己的 class-level `mtTrayName`/`mtTrayItem`（golden cBinSel.h:73-74），而非該 tag 專屬的 widget。 | "Retest"/"CateR" 兩個標籤文字會寫到全 tag 共用的元件而非各 tag 專屬元件，形成 tag 間互相覆蓋的顯示錯誤；此方法整體目前仍在 `#if 0` GATE 內尚未接線，故暫不可觀察。 | W906-FW-BinSel-WB, 20260819 |

### forms/fBinSel.h
（無符合條件的項目）— 本檔 3 個 `(Bx)` 命中皆為交叉參照：1 筆重述 cBinSel.cpp 的 (B16)（已列於上表，非本檔獨立發現），2 筆引用「cShowBinSelect.cpp GATE (B7)」/「fShowBinSelect.h GATE (B7)」，屬另一檔案的 GATE 登錄，非本檔發現。本檔自身的 GATE 登錄（G7/G8/G9）皆為單純延遲接線項目。

### cSecurity.cpp
| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | golden bug (B9) | BUG | cSecurity.cpp:911-931 | golden `TMySecurity::TMySecurity`，約 cSecurity.cpp:818-852 | golden 建構子初始化列寫 `: TComponent(Owner)`，但 `Owner` 並非此建構子的參數，會解析成繼承來的 `TComponent::Owner` 屬性——在自己都還沒建構完成前就讀取這個屬性，等於讀取未初始化記憶體；此移植改用 `TComponent(0)` 重現「新配置記憶體通常讀回 0」的最可能實際行為。 | `TMySecurity` 目前因 GATE (SEC1) 未接線而從未被實際建構，此瑕疵暫無實際影響，但若未來 GATE 解除仍需留意此建構子的自我參照行為並非有意設計。 | W906-FW-SecCC, 20260819 |
| 2 | golden bug (B10, cosmetic/no behavioural impact) | BUG | cSecurity.cpp:47-60 | golden cSecurity.cpp:19-20（檔案層級全域）vs cSecurity.h:145（class member） | golden 同時宣告了檔案層級全域 `int iBit8;` 與 `TfSecurity` 的同名 public member `int iBit8;`；在類別內任何未加限定的 `iBit8` 一律解析到 member（一般 C++ name hiding），全域版本永遠無法從類別內部碼存取。 | 全樹已確認兩者皆從未被實際讀寫（純結構忠實保留），故此瑕疵目前純屬命名巧合、無實際行為影響。 | W906-FW-SecCC, 20260819 |
| 3 | golden bug (B11, protective-by-accident) | BUG | cSecurity.cpp:297-317 | golden `TfSecurity::FormDestroy`，約 cSecurity.cpp:241-258 | golden 先把 `DefaultImg=NULL`，緊接著才 `delete DefaultImg`，等於 delete 一個 NULL 指標（C++ 合法的空操作），而不是真正釋放 `DefaultImg` 原本指向的物件。 | `DefaultImg` 其實是借用自 `fMain->sbSetting->Glyph`（fMain 的 TSpeedButton 所擁有），若這行真的刪除了該物件，反而會造成 fMain 仍持有的 widget 產生 double-free/懸空指標；此 bug 意外地具有保護效果，仍依專案政策原樣保留翻譯。 | W906-FW-SecCC, 20260819 |

### forms/fSecurity.h
（無符合條件的項目）— 本檔 4 個 `(Bx)` 命中皆引用「cShowBinSelect.cpp GATE (B5)/(B6)」作為本表單 fail-closed 預設值設計的先例，屬另一檔案的 GATE 登錄交叉參照，非本檔獨立發現；cSecurity.cpp 的 B9/B10/B11 三筆真實缺陷定義地點在 cSecurity.cpp（已列於上表），本檔僅有一處提及「golden bug B9」的重述文字。

### cCounterClear.cpp
（無符合條件的項目）— 唯一 `(Bx)` 命中（line 560）引用「cShowBinSelect.cpp GATE (B1)」，屬另一檔案的 GATE 登錄交叉參照，僅說明 `fCounterClear->ClearCount(...)` 呼叫點目前是否可達，非本檔缺陷發現。

---

## 三輪增補（20260820）：FW3 batch-5 表單顯示側叢集

以下由 orchestrator（本代理）直接讀取原始碼掃描、未經子代理轉手，涵蓋 20260819 晚間至
20260820 上午新落地的 batch-5 顯示側波次：`forms/fLotInfo.{h,cpp}`、`forms/fSpeed.h`＋
`cSpeed.cpp`、`forms/fStartCondition.h`＋`cStartCondition.cpp`、`forms/fHandlerSys.h`＋
`HandlerSys.cpp`、`forms/fConfiguration.h`＋`cConfiguration.cpp`、`forms/fSetup.{h,cpp}`、
`BinDisplay/MyBinDisp.{h,cpp}`、`tests/test_amr.cpp`。搜尋樣式含任務指定的
`GOLDEN ODDITY`/`GOLDEN QUIRK`/`GOLDEN DEFECT`/`GOLDEN NOTE`/`preserved verbatim`/
`off-by`/`照翻`/`copy-paste`/`no-op`，另加本檔既有詞彙表全集（`golden's own`/`golden itself`/
`kept verbatim`/`misleading`/`landmine`/`typo`/`mistake`等）交叉核對，逐一讀取上下文判讀。

**排除的邊界案例（判定為「非 golden 缺陷」，未收錄）：**
- `forms/fLotInfo.h:246`「recon's row #2 ... copy-pasted onto FormDestroy by mistake」——
  這是先前 RECON 文件本身的摘要寫錯（把 FormClose 的摘要複製貼到 FormDestroy 列），
  是本專案文件的錯誤，不是 golden 原始碼的缺陷。
- `forms/fLotInfo.cpp:421`「Same defect class and same fix shape as cObserver's
  sgStatisticsJam」——這是 port 自己在 Wave B 發現並修正的**建構元件尺寸**bug
  （`TStringGrid` 預設 5x5 未依 golden .dfm 尺寸放大），是 port 的翻譯缺陷、已修正，
  不是 golden 缺陷。
- `forms/fLotInfo.h` GATE (WC-3)「ShowXMLOnLine 儘管名叫 Show 卻寫檔 x4」——內容雖符合
  這棵樹「Show/Read/Close 前綴會騙人」的已知現象類別，但該處註解全文查無任何
  `GOLDEN xxx` 字樣（純粹是 GATE REGISTER 的依賴缺口說明），依本檔「掃描範圍聲明」
  的判準（註解文字本身要明確使用 GOLDEN-缺陷詞彙）不收錄；與下面確有收錄的
  (SC1)/(SC2)/(SC4) 不同，那三筆的 banner 原文明確寫出「GOLDEN ODDITY」字樣。
- `cStartCondition.cpp:843`「DEVIATION: golden's own `fStartCondition->
  strngrdCylinderView` self-reference」——描述的是 golden 用全域指標自我參照（BCB6
  常見寫法），純屬翻譯適配說明，非缺陷/怪異標記。
- 使用者原提示詞列出的「cSetUp FormClose『Close 其實是丟棄未存檔』」一例，經查
  `forms/fSetup.h`/`forms/fSetup.cpp` 全文：`FormClose` 本身在這一波仍是**queued（未翻譯）**
  的 32 個方法之一，未附任何 golden 缺陷註記——現象本身可能是真的（cSpeed 的
  `FormClose` 已證實同款設計），但 cSetUp 這一份目前尚未有對應的 port 端文字紀錄，
  故本輪未收錄，留待該函式真正翻譯時再記錄。

### forms/fLotInfo.cpp（2 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY（未編號） | ODDITY | forms/fLotInfo.cpp:1204-1211 | golden `ShowSocketID` :11443-11516 | `ShowSocketID` 透過 3 個客戶別分支算出 `iTestCHCT`，但全函式其餘部分（含下方迴圈）從未再讀取這個變數，是 golden 自己的死儲存 | 純粹白算，不影響顯示行為；`(void)iTestCHCT;` 消掉 `-Wall` 警告但不改變行為 | AI(W906-FW3-LotInfo-WA) 20260819 |
| 2 | GOLDEN ODDITY（GATE (WC-1) 附帶說明） | ODDITY | forms/fLotInfo.h:587-593（GATE REGISTER (WC-1)）；forms/fLotInfo.cpp:3111-3118 | golden `FormShow` :373,:427 | `FormShow` 在 `IniConfig.bShowLotInfo` 分支內呼叫一次 `ResetLotInfo()`，47 行後又無條件再呼叫一次，兩次呼叫之間沒有任何狀態改變——golden 自己的重複呼叫，非翻譯選擇 | `ResetLotInfo()` 本身是 (b) write-path、本波未翻譯，兩次呼叫皆整段 `#if 0` gate；一旦未來翻譯，需注意這是刻意重現的重複呼叫而非遺漏合併 | AI(W906-FW3-LotInfo-WC) 20260819 |

### forms/fSpeed.h + cSpeed.cpp（2 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY（未編號） | ODDITY | forms/fSpeed.h:50-52,227-236；cSpeed.cpp:1309-1314 | golden `FormClose` :1272-1280 | 名叫 `FormClose` 的方法實際上呼叫 `ReadFile(); DoIniDataToForm();`，等於從磁碟重新讀回並蓋掉畫面上任何未存檔的編輯——是「丟棄」不是「儲存」；golden 自己的行內註解（JerryYang 20250411：離開頁面要刷新一次,避免誤存檔）證實是刻意設計 | 忠實翻譯保留（唯讀顯示側翻譯範圍內即為如此）；未來若有人望文生義以為 `FormClose` 會存檔，需以此記錄糾正 | AI(W906-FW3-Speed-WA) 20260819 |
| 2 | GOLDEN ODDITY（未編號） | ODDITY | forms/fSpeed.h:282-285,331 | golden :540-541（`private` 欄位宣告） | 私有欄位 `OrgStr`（`AnsiString`）在 golden `cSpeed.cpp` 全檔 0 處被讀取，golden 自己就是死欄位；port 仍保留只為讓 header 私有區段與 golden 佈局逐一對應 | 純粹無用欄位，不影響行為；保留是為結構忠實而非誤譯 | AI(W906-FW3-Speed-WA) 20260819 |

### forms/fStartCondition.h + cStartCondition.cpp（5 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY (SC1) | ODDITY | forms/fStartCondition.h:119-125；cStartCondition.cpp:217-223 | golden `FormShow` :142-147（寫入在 :145） | 名叫 `FormShow` 的方法裡藏了一個真正的 WRITE：`if(IniConfig.bShowFTandRTButtonCanClick==true && CUSTOMER_CODE!=CC_SCC){ LastSet.iStartMode=2; ...}`，這棵樹已追蹤的「Show/Read/Close 前綴會騙人」手法第 5 個實例 | `LastSet.iStartMode=2;` 目前整段 `#if 0`；配合 (SC2) 一起被 gate 後，下方 :174-179 的顯示判斷讀到的是 FormShow 呼叫前既有的 `iStartMode`，而非 golden 原本會強制寫入的值——已揭露，非靜默吸收 | AI(W906-FW3-StartCond-WA) 20260819 |
| 2 | GOLDEN ODDITY (SC2) | ODDITY | forms/fStartCondition.h:126-140；cStartCondition.cpp:252-258 | golden `FormShow` :169-172（寫入在 :171） | 同一函式內第二處隱藏 WRITE：`if(cbStartModeOnlyFT->Checked==true){ LastSet.iStartMode=1; }`，與 (SC1) 同類 | 同 (SC1)，兩者合計效應已在 banner 揭露：顯示可能與 golden 當下強制值不同步 | AI(W906-FW3-StartCond-WA) 20260819 |
| 3 | GOLDEN ODDITY (SC4) | ODDITY | forms/fStartCondition.h:152-163；cStartCondition.cpp:810-822 | golden `UpdateCylinderScreen` :1565-1572 | 名叫 `UpdateCylinderScreen`（顯示更新）的方法裡藏了一個自我修復性質的 WRITE：偵測到 `Cylinder[i].iOnOffCount<0`（計數器溢位變負值）時記錄、歸零並呼叫 `SaveCylinderLife()` 落盤，「UpdateScreen」名稱完全看不出會落盤 | 该 clamp+persist 整段 `#if 0`；周圍顯示這個計數欄位的敘述保持 ACTIVE，故溢位時會照原樣顯示負數，直到真正翻譯這段 | AI(W906-FW3-StartCond-WA) 20260819 |
| 4 | GOLDEN ODDITY（RogerYang 20260203 comment，golden :888） | ODDITY | forms/fStartCondition.h:68-79（banner 說明；`ReadWriteStartCondition` 本身本波整段未翻譯） | golden `ReadWriteStartCondition` :760-924，comment at :888 | golden 自己的行內註解記錄了一次歷史 bugfix：「修正formshow裡的fSetup->ReadFile()又調用ReadWriteFile()會造成銦片計數異常」，代表這個函式的讀取臂對呼叫順序（相對於 FormShow 的 `fSetup->ReadFile()`）敏感，golden 作者曾為此修過 race condition | 本波選擇整段排入 write-path 佇列、不拆讀寫臂，正是因為這段歷史記錄顯示拆開翻譯有重演舊 bug 的風險；記錄本身供未來真正翻譯此函式時提醒 | AI(W906-FW3-StartCond-WA) 20260819 |
| 5 | GOLDEN ODDITY（死碼，未編號） | ODDITY | cStartCondition.cpp:646-651 | golden `FormShow` 尾段 :534-535 | `FormShow` 結尾的 `myLog.Do_Log(...)` 呼叫在 golden 原始碼裡自己就已經被 `//` 註解掉（非 port 造成的翻譯缺口），port 原樣保留成註解文字 | 無行為影響，純粹是 golden 自身已死的一行，保留註解供未來讀者確認並非漏翻 | AI(W906-FW3-StartCond-WA) 20260819 |

### forms/fHandlerSys.h + HandlerSys.cpp
（無符合條件的項目）— 全檔以任務指定樣式與本檔既有詞彙表全集逐行核對，0 個 `GOLDEN BUG/ODDITY/
QUIRK/DEFECT/NOTE/...` 命中。檔案內「(d) GOLDEN-DEAD or STRUCTURALLY GATED」字樣是這一波
四分類（(a)/(b)/(c)/(d)）方法分類表的自訂分類標題本身，其下內容（4 個方法本波未宣告）是純
GATE 依賴缺口說明，非 golden 缺陷紀錄。

### forms/fSetup.h + fSetup.cpp（1 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY（未編號） | ODDITY | forms/fSetup.h:70,143-152；forms/fSetup.cpp:78（`RadioButton1KeyDown` 翻譯處的交叉參照註解） | golden `RadioButton1KeyDown` :4443-4449 | `RadioButton1KeyDown` 在 golden `cSetUp.h`/`cSetUp.cpp` 裡有宣告與定義，但目前的 `cSetUp.dfm` 裡找不到任何元件的 `OnKeyDown` 接到它（甚至連叫 `RadioButton1` 的元件都不存在，.dfm 裡只有 `RadioButton6`）——是 golden 自己的 DFM-孤兒事件處理常式 | 該方法在目前的 .dfm 佈局下永遠不會被呼叫；仍依「照翻，並在//AI 註解寫下它為什麼看起來錯」政策原樣翻譯，是否退休留待使用者決定 | AI(W906-FW3-Setup-WA) 20260820 |

### forms/fConfiguration.h + cConfiguration.cpp（5 筆）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY（未編號） | ODDITY | cConfiguration.cpp:105-114 | golden `edtC15Click` :7675-7678 | `edtC15Click` 的（gate 中）`ShowQwertyKey(...,true,1440,30)` 呼叫，Min/Max 兩個參數順序顛倒，golden 自己就是這樣寫 | 此呼叫整段 `#if 0`（write-path，本波未翻譯），記錄供未來真正接上數字鍵盤時比對 | AI(W906-FW3-Config-WA) 20260820 |
| 2 | GOLDEN ODDITY（未編號） | ODDITY | cConfiguration.cpp:116-124 | golden `edL09_Sh1LClick` :7680-7683 | 同一手法第二例：`ShowQwertyKey(...,true,1000,-1000)`，Min/Max 顛倒 | 同上 | AI(W906-FW3-Config-WA) 20260820 |
| 3 | GOLDEN ODDITY（未編號） | ODDITY | cConfiguration.cpp:153-161 | golden `edtSetIPSCQtyClick` :7700-7703 | 同一手法第三例：`ShowQwertyKey(...,true,60000,0)`，Min/Max 顛倒 | 同上 | AI(W906-FW3-Config-WA) 20260820 |
| 4 | golden copy-paste bug（未編號，RECON 4.3 記載） | BUG | forms/fConfiguration.h:67-74（banner 揭露；`sbUpdateHPClick` 本身本波未宣告） | golden `sbUpdateHPClick` :7088-7111（RECON #98） | `sbUpdateHPClick`（儲存 HP 表格的按鈕）複製貼上自 `sbUpdateTrayClick`，實際執行的卻是重新載入 TRAY 格與清除 TRAY 按鈕的 Down 狀態，而非 HP 格自己的對應動作 | `sbUpdateHPClick`/`sbUpdateTrayClick` 皆屬 (b) write-path，本波依 RECON 6.2 指示刻意不排入顯示側波次，未翻譯；記錄供未來寫入波次處理時得知這是 golden 既有瑕疵、需照翻 | RECON_cConfiguration_displayside.md §4.3（引用於 AI(W906-FW3-Config-WA) 20260820） |
| 5 | GOLDEN ODDITY（本輪新增，FW-CFG-W7） | ODDITY | cConfiguration.cpp:6484-6512 | golden `edtSearchFunctionChange` :7113-7146 | 搜尋框只處理 0 字與 >=2 字兩種長度：`if(Length()==0){全部歸位}` `else if(Length()>=2){過濾}`。**打第一個字時兩個分支都不進**，畫面停在前一次的過濾狀態。 | 使用者刪到剩一個字時看到的仍是上一輪的過濾結果，直到刪光或再打一個字才更新；純顯示，不影響設定值。 | FW-CFG-W7 逐句忠實度複驗時發現（Steven 20210730 加的快速搜尋 Config 功能） |

### BinDisplay/MyBinDisp.h + MyBinDisp.cpp（11 筆，20260824 R7 增 7 筆——FW-BINDISP3 協定子類波）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY（DEVIATION (a)，未編號） | ODDITY | BinDisplay/MyBinDisp.h:128-138；MyBinDisp.cpp:581-587 | golden `FlashPro`/`ClearAutoChangingWarn` :3129-3168 | `FlashPro` 透過全域單例 `HSys.BinDisCtrl->GetColorNow(...)`/`->StartFlash(...)` 呼叫「自己」，而不是直接 `this->...`——在正式機上 `HSys.BinDisCtrl` 本來就是 `this`，是多餘的全域間接呼叫；golden 自己的寫法，非翻譯產物 | 純粹是不必要的間接呼叫，行為與 `this->` 完全相同；忠實保留未「修正」 | AI(W906-BinDisp-WA) 20260820 |
| 5 | GOLDEN DEFECT (i) | DEFECT | MyBinDisp.cpp:1189-1191（SetBackGround_TFT）,1226-1228（SetNoBackGround_TFT） | golden MyBinDisp.cpp:1105,:1119-1123（及 :1139 同型） | sprintf 以 20 個 %c 寫進 `char cSendCommand[20]`——20 字元＋NUL＝21 bytes，**1-byte stack overflow**（TFT Magazine 版正確用 char[21]；主迴圈對 golden 字面坐實） | 每次設定背景色都越界寫 1 byte；x86 疊框下通常吃進 padding 未爆——**真面板 bring-up 前必須裁決**（忠實保留 vs 擴 buffer） | AI(W906-FW-BINDISP3) 20260824 |
| 6 | GOLDEN ODDITY (j) | ODDITY | MyBinDisp.cpp:1258,1320,1326,1412 | golden :1218,:1308 | MagazineWriteBin_HTA18/_BT008 的 log 記到過期的 SendBuffer 而非實際送出的 cSendCommand（golden 20240906 只修了 TFT 路徑） | 通訊除錯 log 內容與實際線上封包不符，誤導除錯 | AI(W906-FW-BINDISP3) 20260824 |
| 7 | GOLDEN ODDITY (k) | ODDITY | MyBinDisp.cpp:1803 | golden :1684-2240 | `anSendCommandBuf` 永不清空——multi-send 每輪追加 hex dump | log 行單調變長、記憶體無上限成長（長時運轉的慢性洩漏形狀） | AI(W906-FW-BINDISP3) 20260824 |
| 8 | GOLDEN ODDITY (l) | ODDITY | MyBinDisp.cpp:728 | golden :29 | `anSendBinCommandBuf` 宣告後全 golden 零引用 | 死全域；照翻 | AI(W906-FW-BINDISP3) 20260824 |
| 9 | GOLDEN ODDITY (m) | ODDITY | MyBinDisp.cpp:1804 | golden :1684-2240（閃爍搜尋段） | `bMagazineLink[iAuto3MagazineIndex-i]` 無下界檢查，iAuto3MagazineIndex<13 時可讀負索引 | 越界讀（UB）；實務依 magazine 配置未爆；照翻 | AI(W906-FW-BINDISP3) 20260824 |
| 10 | GOLDEN ODDITY (n) | ODDITY | MyBinDisp.cpp:2375,2479,2504,2615,2637 | golden 對應行 | `iRusStatus=2/4/1` 裸數字（實為狀態 enum 別名） | 可讀性陷阱；照翻 | AI(W906-FW-BINDISP3) 20260824 |
| 11 | GOLDEN ODDITY (o) | ODDITY | MyBinDisp.cpp:705-707（banner）,2373 | golden :2248 | DoStartSetColor 的 `static bool bFirst=false` 全檔零賦值——「只跑一次」的 Task=10 detour 其實每次 case-1 都跑 | 意圖是一次性初始化、實際每輪重跑；行為穩定但與註解意圖不符；照翻 | AI(W906-FW-BINDISP3) 20260824 |
| 2 | GOLDEN GOTCHA（DEVIATION (b)，未編號） | GOTCHA | BinDisplay/MyBinDisp.h:139-153 | golden 建構子 :39-129 | golden 建構子本身有多個 scalar/array 欄位從未賦值，隱含依賴 BCB6 `TObject::NewInstance` 在建構子執行前先把整塊實例記憶體歸零——這個「零值」從未在 golden 建構子裡明講，是隱性的 VCL 平台語意 | 若 port 不額外補一個 `ZeroInitVclFields()` 手動歸零，這些欄位在標準 C++ 下會是未初始化的隨機值而非 golden 實際的 0/false/NULL——已補上等效初始化以維持行為對等，記錄供未來比對 | AI(W906-BinDisp-WA) 20260820 |
| 3 | GOLDEN ASYMMETRY（DEVIATION (c)，未編號） | ASYMMETRY | BinDisplay/MyBinDisp.h:154-161 | golden 建構子 :86（`ComPort=4;`） | `ComPort` 在建構子裡被賦值字串 `"4"`，但同類的 `ComPort2` 在建構子裡完全沒被賦值（僅靠 `AnsiString` 自己的預設建構子變成 `""`）——這個不對稱是 golden 自己的寫法 | 兩個看似成對的欄位初始值不同源；忠實保留該不對稱，未「補齊」`ComPort2` 的預設值 | AI(W906-BinDisp-WA) 20260820 |
| 4 | **GOLDEN DEFECT（使用者指定的已知案例）** | DEFECT | MyBinDisp.cpp:89-97,451 | golden `WriteTargetBin` :608 | `WriteTargetBin` 的邊界檢查 `if(Index>MAX_BIN_UNIT) return;` 應該是 `>=MAX_BIN_UNIT`（`MAX_BIN_UNIT` 本身就是「最後一個合法索引之後」那個哨兵值，`iSetBin[MAX_BIN_UNIT][...]`/`bSetBin[MAX_BIN_UNIT]` 等陣列大小恰好是 `MAX_BIN_UNIT`）——`>` 讓 `Index==MAX_BIN_UNIT` 這個 off-by-one 值溜過檢查 | `Index==MAX_BIN_UNIT` 時會寫到 `iSetBin[Index][...]`/`bSetBin[Index]=true;`，越界寫入陣列尾端後一個元素；忠實翻譯保留 `>`（未改成 `>=`），留供使用者裁決是否修正 | AI(W906-BinDisp-WA) 20260820 |

### tests/test_amr.cpp
（無符合條件的項目）— 全檔以任務指定樣式與擴充詞彙表逐行核對，0 個 `GOLDEN BUG/ODDITY/QUIRK/
DEFECT/NOTE/...` 命中；fixture 內的所有 `golden :NNNN` 引註都只是客觀對照 golden 行號/行為，
未使用任何缺陷類詞彙。

### uTemp_Set.cpp / forms/fTemp_Set.h（1 筆，W906-LEDGER-R2 20260821 新增分節）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY（未編號） | ODDITY | uTemp_Set.cpp:620-638（`rb1PointClick`） | golden（`HT9011UC_Code_V3.33.906.0_20260618`）uTemp_Set.cpp:421-425；宣告 uTemp_Set.h:66-68,80,815 | `rb1PointClick` 把 `Sender` 轉型成 `TRadioGroup*` 再讀 `Ptr->Tag`，但 `rb1Point`~`rb6Point` 在 golden 標頭裡實際各自宣告成獨立的 `TRadioButton*` 成員，並非某個真正 `TRadioGroup` 底下的子元件——轉型型別與實際物件型別不符 | 在真實 VCL 裡因 `Tag` 屬性繼承自共同基底 `TComponent`、讀取結果與宣告時的靜態轉型型別無關，恰好無害；本 port 因這 5 顆按鈕全是裸 `TRadioButton`（非帶 `Tag` 的 `TfTemp_SetTagButton` 包裝元件），此呼叫連同其唯一消費者 `SetBasePointIMG`（自身也全 gate）一併被 GATE(dep-Tag) 整段擋住，尚不可達 | AI(W906-FW3-TempSet-WA) 20260820（併入台帳 W906-LEDGER-R2 20260821） |

### DynamicTemp.cpp / forms/fDynamicTemp.h（3 筆，W906-LEDGER-R2 20260821 新增分節）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY（未編號） | ODDITY | DynamicTemp.cpp:259-263,265-272（`edMaxMouseDown`） | golden（`HT9011UC_Code_V3.33.906.0_20260618`）DynamicTemp.cpp:152-158 | `edMaxMouseDown`（gate 中）呼叫 `ShowQwertyKey` 時，「目前值」引數讀的是 `edMin->Text` 而非 `edMax->Text`——golden 自己從 `edMinMouseDown` 複製貼上時忘了改欄位 | GATE(Q1) 已於 20260824（FW-QWKEY2）解鎖、呼叫已活（handler 未接線故 runtime 尚不可達）；一旦事件流接上，使用者點 Max 欄位彈出的數字鍵盤會帶入 Min 欄位的舊值而非 Max 自己的值（20260825 校正：解鎖後行號已位移，port 位置欄未重校） | AI(W906-FW3-DynTemp-WA) 20260820（併入台帳 W906-LEDGER-R2 20260821） |
| 2 | GOLDEN ODDITY（未編號） | ODDITY | DynamicTemp.cpp:307-314,318-330（`edUpperMouseDown`） | golden（`HT9011UC_Code_V3.33.906.0_20260618`）DynamicTemp.cpp:189-201 | `edUpperMouseDown` 幫 `Chart1->Series[17]` 畫線時色彩引數用 `TC[16]`（Series 16 的顏色）而非 `TC[17]`——golden 從姊妹函式 `edLowerMouseDown` 複製貼上時漏改索引；同檔 `btTempICLoadClick` 對 Series[16]/[17] 的呼叫確實正確分別使用 `TC[16]`/`TC[17]` | Series[17]（Upper 上限線）在圖表上會被畫成跟 Series[16]（Lower 下限線）同一顏色，兩條線在圖上難以分辨；純顯示層瑕疵，不影響溫度量測邏輯本身 | AI(W906-FW3-DynTemp-WA) 20260820（併入台帳 W906-LEDGER-R2 20260821） |
| 3 | GOLDEN ODDITY（未編號） | ODDITY | DynamicTemp.cpp:402-420（`FormShow`） | golden（`HT9011UC_Code_V3.33.906.0_20260618`）DynamicTemp.cpp:261-273 | `FormShow` 只用 `atoi(edMax->Text)<Chart1->LeftAxis->Minimum` 一個條件判斷要不要重設座標軸，但兩個分支接下來都會同時改寫 `Minimum` 與 `Maximum`——比較式只跟即將被覆寫的 `Minimum` 比、跟 `Maximum` 完全無關，是不對稱的判斷式 | 兩分支寫入 `Minimum`/`Maximum` 的先後順序相反（true 分支先設 Minimum、false 分支先設 Maximum），但因兩行是各自獨立賦值、中間無讀取，最終數值結果與寫入順序無關，純粹是 golden 自己寫法不對稱、無實際功能影響 | AI(W906-FW3-DynTemp-WA) 20260820（併入台帳 W906-LEDGER-R2 20260821） |

### MainTempMode.cpp（W906-LEDGER-R2 20260821 新增 stub 分節）
（無符合條件的項目）— 全檔（本輪新落地的 `TfMain::ChangeTempMode` facade 缺口收斂檔）以任務
指定樣式與本檔既有詞彙表全集逐行核對，0 個 `GOLDEN BUG/ODDITY/QUIRK/DEFECT/NOTE/...` 命中；
檔案內大量 `golden :NNNN` 引註都是客觀對照 golden 行號、或 `GATE(dep-...)` 依賴缺口說明，
未使用任何缺陷類詞彙。

### cSetUp.cpp（3 筆，20260824 FW-SETUP-D 波當波入帳；R2 時為 0 筆 stub 分節）

（Wave B 三方法 0 命中的原始紀錄保留於歷史：R2 時本檔尚無任何 GOLDEN 標記；FW-SETUP-D
波次翻譯 7 個方法時登記以下 3 筆，banner 彙整於 cSetUp.cpp:245-257。）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK（FW-SETUP-D） | QUIRK | cSetUp.cpp:247-249（banner）,628-629（inline，`CHSetError` 內） | golden cSetUp.cpp:1559,:1590 | `if(iYpitch>63.5)`：int 對 63.5 比較，行為等同 `>=64`，研判是 63/64 邊界的手誤寫法 | 行為與 `>=64` 完全相同故無實際差異，純可讀性陷阱；照翻 | AI(W906-FW-SETUP-D) 20260824 |
| 2 | GOLDEN ODDITY（FW-SETUP-D） | ODDITY | cSetUp.cpp:250-254（banner）,630-631（inline） | golden cSetUp.cpp:1348-1603 | CHSetError 的 pitch switch 無 QualSite2X2N/_6Site2X3N/_8Site2X4N/_32Site4X8N 各 case 也無 default——N 模式的 pitch 完全不經此層驗證 | N 模式機型的錯誤 pitch 設定不會被這層防呆擋下 | AI(W906-FW-SETUP-D) 20260824 |
| 3 | GOLDEN ODDITY（FW-SETUP-D） | ODDITY | cSetUp.cpp:255-257（banner）,1061（inline） | golden cSetUp.cpp:4752-4770 | CoSocketComboChange 無視 Sender 參數，直接讀 CoSocketCombo 成員 | 事件若被綁到其他控制，讀的仍是 CoSocketCombo；照翻 | AI(W906-FW-SETUP-D) 20260824 |

### database.cpp（1 筆，20260824 新增分節）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN ODDITY (ICBD-1) | ODDITY | database.cpp:216-221（`InstallColorBinDisplay` banner） | golden database.cpp:1686-1697 | golden 在 iType 檢查前就 `new`，非 3/4 呼叫會經 early return 洩漏該實例；且 new 後的 `BinDisCtrl==NULL` 測試在標準 C++ 永不成立 | 兩者實務不可達——唯一呼叫點（golden :1543-1545）自帶 NUMBER_PANEL_TYPE 3/4 守衛；照翻 | AI(W906-FW-BINDISP1) 20260824 |

### Motor/myMN200motor.cpp（1 筆，20260824 新增分節）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN NOTE (BINDISP2-e) | NOTE | Motor/myMN200motor.cpp:2426-2433（GATE (e) 開閘註記） | golden Motor/myMN200motor.cpp:2076-2077 | 24V 斷電恢復路徑對 BinDisCtrl 的 deref 沒有 NUMBER_PANEL_TYPE 3/4 守衛（GATE (c) 那側 :1497-1498 有）——非 3/4 機型走到即 NULL deref，golden 同炸 | 非 3/4 機型且 MN200 ring 錯誤令 iWriteErrorLogCT!=0 時當機；實務未爆研判因 MN200 機型多配 3/4 面板 | AI(W906-FW-BINDISP2) 20260824 |

### forms/fQwertyKey.h + fQwertyKey.cpp + myQwertyKeyBoard.cpp（4 筆，20260824 FW-QWKEY1 當波入帳）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN NOTE (G-a) | NOTE | forms/fQwertyKey.h:126-131 | golden myQwertyKeyBoard.cpp:290 | ShowQwertyKey 的 `CheckRange(d,min,max)` 實參順序對形參 (Value,Maximum,Minimum) 顛倒 | 行為無差（CheckRange 自身對調界處理對稱），純閱讀陷阱；照翻 | AI(W906-FW-QWKEY1) 20260824 |
| 2 | GOLDEN NOTE (G-b) | NOTE | forms/fQwertyKey.h:132-135 | golden myQwertyKeyBoard.cpp:307 | spbChangeCaseClick 迴圈 `i<47` 排除 space＋數字鍵於大小寫切換之外 | 設計如此（caption 大小寫不變），照翻 | AI(W906-FW-QWKEY1) 20260824 |
| 3 | GOLDEN NOTE (G-c) | NOTE | forms/fQwertyKey.h:136-138 | golden myQwertyKeyBoard.cpp:79-82,:93-94 | 9/0 與 -/=/[/] 被重分類 eKeyNumAndSymbol→eKeyalphabet（Steven 20210805） | N_NO_SYMBOL 不再停用/強制大寫這些鍵；照翻 | AI(W906-FW-QWKEY1) 20260824 |
| 4 | GOLDEN NOTE (G-d) | NOTE | forms/fQwertyKey.h:139-147 | golden myQwertyKeyBoard.cpp:171-178 | ShowQwertyKey 由成員函式內解參考 fQwertyKey/fQwertyKey2 雙全域——golden 靠先 CreateForm 保證非 NULL | port 內兩全域接線前呼叫＝NULL deref（與 golden pre-CreateForm 同曝險）；今日零呼叫者 | AI(W906-FW-QWKEY1) 20260824 |

### forms/fPassword.h + fPassword.cpp + Password.cpp（4 筆，20260824 FW-QWKEY1 當波入帳）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN NOTE (G-P1) | NOTE | forms/fPassword.h:174-180 | golden Password.cpp:89-102 | FormShow `if(bShowTab==0){...} else if(bShowTab==0){...}`——第二條件重複 ==0，EventLog 分頁分支是不可達死碼（應為 ==1；主迴圈對 golden :89/:96 字面坐實） | bShowTab==1 時兩支都不跑，EventLog 分頁永不被 FormShow 啟用；照翻 | AI(W906-FW-QWKEY1) 20260824 |
| 2 | GOLDEN NOTE (G-P2) | NOTE | forms/fPassword.h:181-184 | golden Password.h:92 | bShowTab 宣告 bool 卻以 0/1 整數書寫 | cosmetic，靠 bool 轉換運作；照翻 | AI(W906-FW-QWKEY1) 20260824 |
| 3 | GOLDEN NOTE (G-P3) | NOTE | forms/fPassword.h:185-189 | golden Password.cpp:118-121＋Password.dfm | SpeedButton1Click 是 dfm 孤兒 handler（dfm 零接線、無 SpeedButton1 成員） | 死碼；照翻（fSetup RadioButton1KeyDown 同型前例） | AI(W906-FW-QWKEY1) 20260824 |
| 4 | GOLDEN NOTE (G-P4) | NOTE | forms/fPassword.h:190-192 | golden Password.cpp:325-326 | CheckPassword 在 JCET_FOR_EVAN==1 時 ID 比對於迴圈內被旁路→授予「最低 index 密碼相符」的等級（order-dependent） | 等級授予依表序而非帳號；照翻 | AI(W906-FW-QWKEY1) 20260824 |

### BarcodeReader.h + BarcodeReader.cpp（3 筆，20260825 FW-BARCODE1 當波入帳）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN QUIRK (BR-q1) | QUIRK | BarcodeReader.cpp:9-12,43-47 | golden BarcodeReader.cpp:18-21 | BarcodeChange[bcTotal] 以 15 個初始器對 bcTotal==16（MachineType.h:933）——BarcodeChange[bcSECSGEM==15] 是預設空字串 | SECSGEM 觸發的登入紀錄行「... Change 」無後綴；照翻 | AI(W906-FW-BARCODE1) 20260825 |
| 2 | GOLDEN NOTE (BR-n1) | NOTE | BarcodeReader.cpp:14-15,79-80 | golden BarcodeReader.cpp:54-55 | FormShow 以寫死的 1280×1024 置中視窗，不讀實際螢幕解析度 | 非 1280×1024 螢幕上視窗偏離中心；照翻 | AI(W906-FW-BARCODE1) 20260825 |
| 3 | GOLDEN NOTE (BR-n2) | NOTE | BarcodeReader.cpp:17-22,419 | golden BarcodeReader.cpp:389 | TimerKeyIn 的 KYEC 尾段：6 碼條碼取 2 位頭碼再驗 85..120，但 2 位數上限 99——100..120 只有 7 碼/3 位頭碼臂可達（golden 自己的註解就寫「85~120 (85~120)」） | 6 碼條碼頭碼 100..120 的驗證分支永不成立；照翻 | AI(W906-FW-BARCODE1) 20260825 |

---

## 無法判讀/存疑清單

以下項目在彙整過程中經 orchestrator 或子代理本人標示信心不足，列出供日後複核：

- `RotateKit/aRotateKIT_Out.h`「Postion 拼字」項：屬於文件性附註（標頭 banner 內的說明性文字），
  是否應與同一 banner 的另一筆（零參數宣告孤兒）合併成一筆，或維持獨立列出，判斷上有彈性；
  本台帳選擇獨立列出並標示 ⚠️。
- `cBinSel.cpp` (B16)：標記詞彙用的是 `GOLDEN QUIRK` 而非 `GOLDEN BUG`，但內容經 orchestrator
  直接讀取原始碼確認是真實的行為缺陷（`mtTrayNameSetColor` 兩個呼叫漏加 `MyBinPanel[tag]->`
  前綴，寫到錯的共用元件）——已決定收錄，類型欄以 QUIRK 呈現但性質等同 BUG。
- `vclcompat/TrayCore.cpp` 原始 2 筆 `GOLDEN QUIRK`（陣列聚合初始化預設值、`FShape` 未賦值）：
  經 orchestrator 直接讀取原始碼確認這兩處純粹是「C++ 語意剛好與 golden VCL 預設值相符」的
  說明性註解，並非真實行為差異——已決定**不收錄**進主表，僅在此存疑清單留下排除紀錄。
- `Interface/TesterTCP_Socket.h` 與 `BarCode/BarCode_Shuttle2_*` 系列部分 `.h`/`.cpp` 配對：
  多組標記文字幾乎完全相同（`.h` 端通常只是重申 `.cpp` 端已完整說明的同一缺陷），已依各子
  代理回報合併為單一筆並在 port 位置欄列出兩個檔案；若日後發現其中某一對其實描述的是
  「同名但不同」的缺陷，需要拆回兩筆。
- Command.cpp 的 `(B3)` 行號引用：banner 本身註明「此引用之前錯誤，20260818 整併時更正」，
  屬於已知的歷史校正案例，收錄時採用更正後的版本，未回頭驗證更正是否完全正確。
- 本台帳的 golden 行號**未逐筆回頭比對 golden 原始碼**，一律照抄 port 端註解原文；若後續
  維護者發現行號有誤，請直接在對應列修正並在該列備註校正日期，不需要整份重新掃描。
- SECSGEM 三檔（`uHGemClass.cpp`/`uHGemEquipment.cpp`/`uHGemHT9045.cpp`）的 `GOLDEN QUIRK`
  補撈子代理已於同一次收工前完成並併入本檔（見上方「補撈紀錄」），不再是待辦事項。


### cObserver.cpp（42 筆，20260825 FW-OBS-W2 當波入帳，新增分節）

> 本檔在本波之前**零筆**（`git show HEAD~1` 的 cObserver.cpp 對六個 GOLDEN-詞彙 grep 得 0），
> 所以這 42 列全部來自 Wave 3 的三個 chunk。原始標記 **55** 個（BUG 23／NOTE 13／QUIRK 8／
> ODDITY 11／DEFECT 0，主迴圈自行 grep 複算），其中 13 個是 banner 的 inline 重複
> （11 個敘述端標記＋2 個散文交叉引用），依維護規則 6 併入所屬 banner 那一列 →
> BUG 11／NOTE 13／QUIRK 8／ODDITY 10 = 42 列。
> golden 行號由子代理逐條開 golden（cp950）核對，回報 0 筆需更正；主迴圈另抽驗 4 條
> （:550-555、:554、:3641-3644、:5385）確認無誤。
> 另有 1 筆**非 golden**的引用更正已寫進第 41 列：該註記寫「near cObserver.cpp:3045」，
> 實際邊界註解在 cObserver.cpp:3102-3108。

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN NOTE (golden :351) | NOTE | cObserver.cpp:3659 | golden :351 | FormShow 宣告 `AnsiString Str;` 之後在整整 306 行內從未被讀取。 | 純死變數，無行為影響；port 加 `(void)Str;` 消警告，功能不變。 | AI(W906-FW3-Observer-W2) 20260825 |
| 2 | GOLDEN QUIRK (golden :554) | QUIRK | cObserver.cpp:3954 | golden :554 | for-init 重新宣告 `int i`，遮蔽了 golden :350 的函式層 `i`。 | 迴圈跑完外層 `i` 仍是未初始化的舊值，日後若有人改用外層 i 會讀到非預期值。 | AI(W906-FW3-Observer-W2) 20260825 |
| 3 | GOLDEN BUG (golden :550-555) | BUG | cObserver.cpp:3959 | golden :550-555 | `tsFileName=new TStringList()` 之後整個 FormShow 都沒有 delete。 | 每開一次 Observer 表單就洩漏一個 TStringList；同檔 cbbMonthChange (:3763-3799) 有正確的 Clear+delete 可對照。 | AI(W906-FW3-Observer-W2) 20260825 |
| 4 | GOLDEN ODDITY (golden :1007) | ODDITY | cObserver.cpp:4258 | golden :1007 | Contact Count (History) 格線的 "Row-x" 列標題取自 `rgContactCountKinds->ItemIndex`，而不是自己的 History 選擇器。 | History 表頭會跟著 Kinds 選項跑；port 中該行位於 GATE FW3A-10 內，目前不會執行。 | AI(W906-FW3-Observer-W2) 20260825 |
| 5 | GOLDEN ODDITY (golden :1001 vs :965) | ODDITY | cObserver.cpp:4263 | golden :1001（對照 :965） | 非 9045 機型分支 StringGrid3 設 iLeft=48，逐字孿生的 StringGrid2 卻設 47，差一像素。 | 兩張同構表格左邊界不對齊；port 中 iLeft 最終只餵給 FW3A-10 的畫布 no-op，目前無可見效果。 | AI(W906-FW3-Observer-W2) 20260825 |
| 6 | GOLDEN ODDITY (golden :1017 vs :982) | ODDITY | cObserver.cpp:4266 | golden :1017（對照 :982） | 同一種 caption 指派，這裡寫 `AnsiString(iTotal)`、孿生函式寫裸 `iTotal`。 | BCB6 下結果完全相同，只是寫法不一致；port 中該行在 GATE FW3A-4 內不會執行。 | AI(W906-FW3-Observer-W2) 20260825 |
| 7 | GOLDEN NOTE (golden :1296) | NOTE | cObserver.cpp:4468 | golden :1296 | `MyDrawText(pCanvas, Rect, iLeft+CellWidth*i, ..., 168, 188)` 沒有字串參數，看似漏參數，實際綁到 common.h:94 真實存在的 6 參數空矩形填色多載。 | 非缺陷；此註記存在的目的是防止後續波次誤把它當翻譯漏參數而「修好」，且整個 DrawCellCategory 目前在 GATE FW3A-10 內是 no-op。 | AI(W906-FW3-Observer-W2) 20260825 |
| 8 | GOLDEN QUIRK (golden :3975) | QUIRK | cObserver.cpp:4819 | golden :3975 | `bool bEmpty[10]` 宣告 10 格，實際只有 [0]..[8] 被指派與讀取，[9] 恆為初值 true。 | 死元素本身無行為影響，但陣列大小與實際欄位數不符，日後增欄容易誤判索引。 | AI(W906-FW3-Observer-W2) 20260825 |
| 9 | GOLDEN QUIRK (golden :3992-4004 vs :4020-4032) | QUIRK | cObserver.cpp:4822 | golden :3992-4004（對照 :4020-4032） | `case 1` 與 `case 3` 的條件清單逐字完全相同。 | iType=1 與 iType=3 在語意上無法區分，維護時改一邊必漏另一邊。 | AI(W906-FW3-Observer-W2) 20260825 |
| 10 | GOLDEN QUIRK (golden :3990-4033) | QUIRK | cObserver.cpp:4825 | golden :3990-4033 | switch 沒有 `default`，iType 不是 1/2/3 時直接落到 `return true`。 | 未預期的呼叫端會被靜默回報「欄位已填齊」，必填檢查形同不存在。 | AI(W906-FW3-Observer-W2) 20260825 |
| 11 | GOLDEN ODDITY (golden :4043-4047) | ODDITY | cObserver.cpp:4894 | golden :4043-4047 | `D:\PrecautionRecord\system` 目錄在 bB01_UsePrecautionRecordFunction 判斷「之前」就被強制建立。 | 即使 Precaution 功能關閉，每台機器仍會被種出這個空目錄。 | AI(W906-FW3-Observer-W2) 20260825 |
| 12 | GOLDEN NOTE (golden :4054-4060 / :4063-4069) | NOTE | cObserver.cpp:4899 | golden :4054-4060 / :4063-4069 | 兩個選單載入迴圈都是 do-while，第一個 key 無條件先讀一次。 | 沒有 [Procaution]/[NoteContent] 段落時，下拉選單仍會多出一筆字面字串 "NULL"。 | AI(W906-FW3-Observer-W2) 20260825 |
| 13 | GOLDEN BUG (golden :4083 + :4085-4086) | BUG | cObserver.cpp:4962, 4979 | golden :4083 + :4085-4086 | `memoPtr=new TStringList;` 排在 bB01 功能旗標守衛之前，而守衛的 `return` 不 delete 它。 | Precaution 功能關閉時，每次呼叫 SavePrecautionMemoInformation 就洩漏一個 TStringList。 | AI(W906-FW3-Observer-W2) 20260825 |
| 14 | GOLDEN NOTE (golden :4091-4092) | NOTE | cObserver.cpp:4966 | golden :4091-4092 | 輸出檔名直接內嵌 `edNoteContents->Text`，操作員自由輸入的字串完全未過濾。 | 內容含 \ / : * ? " < > \| 時會組出無法寫入的路徑，存檔靜默失敗且無任何守衛。 | AI(W906-FW3-Observer-W2) 20260825 |
| 15 | GOLDEN BUG (golden :4160 + :4162-4163) | BUG | cObserver.cpp:5074, 5095 | golden :4160 + :4162-4163 | `new TIniFile(sPath)` 排在 bB01 守衛之前，守衛的 `return` 不 delete；原作者 2016 年只補了孿生函式 :4196 那條路徑。 | Precaution 功能關閉時每次呼叫 SavePrecautionParameter 洩漏一個 TIniFile。 | AI(W906-FW3-Observer-W2) 20260825 |
| 16 | GOLDEN ODDITY (golden :4156-4157) | ODDITY | cObserver.cpp:5080 | golden :4156-4157 | 目錄建立同樣排在功能旗標守衛之前，與 LoadPrecautionMenu 同形。 | 功能關閉時仍會建出 D:\PrecautionRecord\system；port 中該行在 GATE B-SAFETY-3a 內不會執行。 | AI(W906-FW3-Observer-W2) 20260825 |
| 17 | GOLDEN BUG (golden :4189 + :4191-4192) | BUG | cObserver.cpp:5145, 5155 | golden :4189 + :4191-4192 | `new TIniFile(sPath)` 排在 bB01 守衛之前，該 `return` 洩漏它；同函式 :4196 的 FileExists 出口卻有 delete。 | Precaution 功能關閉時每次呼叫 LoadPrecautionParameter 洩漏一個 TIniFile。 | AI(W906-FW3-Observer-W2) 20260825 |
| 18 | GOLDEN ODDITY (golden :4233-4237) | ODDITY | cObserver.cpp:5192 | golden :4233-4237 | 目錄建立排在 bB02_HanderMajorMaintenanceRecordFunction 守衛之前。 | Major Maintenance 功能關閉時仍會建出 D:\MajorMaintenanceRecord\system。 | AI(W906-FW3-Observer-W2) 20260825 |
| 19 | GOLDEN ODDITY (golden :4239) | ODDITY | cObserver.cpp:5195 | golden :4239 | 寫成 `FileExists(aMenuPath.c_str())`，多繞一次 c_str()，孿生的 :4049 沒有這層轉換。 | 無行為差異，只多一次 AnsiString 來回建構；port 的 FileExists 取 `const AnsiString&`，隱式轉換與 BCB6 一致。 | AI(W906-FW3-Observer-W2) 20260825 |
| 20 | GOLDEN BUG (golden :4317 + :4319-4320) | BUG | cObserver.cpp:5313, 5339 | golden :4317 + :4319-4320 | `memoPtr=new TStringList;` 排在 bB02 守衛之前，守衛的 `return` 不 delete 它。 | Major Maintenance 功能關閉時每次呼叫就洩漏一個 TStringList，與 SavePrecautionMemoInformation 是同型孿生缺陷。 | AI(W906-FW3-Observer-W2) 20260825 |
| 21 | GOLDEN BUG (golden :4327 and :4373) | BUG | cObserver.cpp:5316 | golden :4327、:4373 | 檔名組字時直接讀 `MemoUndesirablePhenomenon->Lines->Strings[0]`，完全沒有 Count>0 檢查。 | BCB6 下空 memo 會丟 EStringListError；目前唯一呼叫端 :4576 先擋掉 Count==0 所以是潛伏而非現行缺陷，任何新呼叫端會立刻踩到（port 的 vclcompat 回空字串不丟例外，改成組出尾巴為空的檔名）。 | AI(W906-FW3-Observer-W2) 20260825 |
| 22 | GOLDEN QUIRK (golden :4393) | QUIRK | cObserver.cpp:5433 | golden :4393 | `bool bEmpty[10]` 只有 [0]..[6] 被指派與讀取，[7]/[8]/[9] 恆為初值 true。 | 死元素，無行為影響，但同樣有陣列大小與欄位數不符的維護風險。 | AI(W906-FW3-Observer-W2) 20260825 |
| 23 | GOLDEN QUIRK (golden :4403-4419) | QUIRK | cObserver.cpp:5435 | golden :4403-4419 | switch 只有 `case 1`，既無其他 case 也無 `default`。 | 任何其他 iType 一律回 true，Major Maintenance 的必填檢查被靜默略過。 | AI(W906-FW3-Observer-W2) 20260825 |
| 24 | GOLDEN QUIRK (golden :4437-4448) | QUIRK | cObserver.cpp:5484 | golden :4437-4448 | memset(0) 之後 `execinfo.fMask=SEE_MASK_NOCLOSEPROCESS` 只在 FileExists 分支內設定，而 ShellExecuteEx 取得的 process handle 從未被 CloseHandle。 | 每次叫出螢幕小鍵盤洩漏一個 process handle；port 中整段在 GATE B-OSK-1 內，目前不會執行。 | AI(W906-FW3-Observer-W2) 20260825 |
| 25 | GOLDEN ODDITY (golden :4574-4581) | ODDITY | cObserver.cpp:5729 | golden :4574-4581 | 這是本區塊唯一沒有先檢查 bB02 功能旗標就驗證並存檔的處理常式，旗標檢查落在下一層的 :4319。 | 功能關閉時操作員仍會收到「未填完整」拒絕對話框、表單仍被清空，但什麼都沒被記錄下來。 | AI(W906-FW3-Observer-W2) 20260825 |
| 26 | GOLDEN BUG (golden :4604 + :4608-4611) | BUG | cObserver.cpp:5760, 5805, 5807 | golden :4604 + :4608-4611 | `memoPtr=new TStringList;` 排在兩個 early return 之前，兩條出口都不 delete。 | D:\MajorMaintenanceRecord 不存在或功能關閉時，每次按查詢就洩漏一個 TStringList。 | AI(W906-FW3-Observer-W2) 20260825 |
| 27 | GOLDEN BUG (golden :4687 + :4691-4694) | BUG | cObserver.cpp:5884, 5920, 5922 | golden :4687 + :4691-4694 | 同型：`memoPtr=new TStringList;` 排在兩個 early return 之前，兩條出口都不 delete。 | D:\PrecautionRecord 不存在或功能關閉時，每次按查詢就洩漏一個 TStringList。 | AI(W906-FW3-Observer-W2) 20260825 |
| 28 | GOLDEN QUIRK (golden :4720-4722) | QUIRK | cObserver.cpp:5887 | golden :4720-4722 | `else if(pnNoteLog->Caption==sName)` 這個分支的 body 是空的。 | 這是刻意用來吞掉 SavePrecautionMemoInformation (:4098) 寫出的區段標題列，不是漏寫；被「整理掉」的話標題會掉進最後的 else 汙染 MemoNoteLog。 | AI(W906-FW3-Observer-W2) 20260825 |
| 29 | GOLDEN NOTE (:2779-2780) | NOTE | cObserver.cpp:6161 | golden :2779-2780 | 守衛比較 `->Date`，修正卻寫回 `->DateTime`。 | 真 VCL 下兩者是同一份值所以沒事；本 port 的 facade 是三個獨立欄位，picker 1 的 Date 不會被更新，守衛下一次呼叫仍可能成立。 | AI(W906-FW3-Observer-W2) 20260825 |
| 30 | GOLDEN NOTE (:2790-2799) | NOTE | cObserver.cpp:6207 | golden :2790-2799 | 客戶碼判斷寫成空的 `if` body（只有一個裸 `;`），真正的動作放在後面的 `else return;`。 | 純可讀性問題，行為正確；反向改寫會違反本 port「不寫得更好」的忠實翻譯原則。 | AI(W906-FW3-Observer-W2) 20260825 |
| 31 | GOLDEN NOTE (:2803) | NOTE | cObserver.cpp:6210 | golden :2803 | 維護密碼字串 "27025312" 直接寫死在原始碼裡。 | 密碼無法更換且可從二進位撈出；port 中 ShowQwertyKey 無視窗故 edPassword 恆為 ""，此處目前一律提早 return，離線不可達。 | AI(W906-FW3-Observer-W2) 20260825 |
| 32 | GOLDEN NOTE (:3090/:3113/:3136/:3159) | NOTE | cObserver.cpp:6298 | golden :3090 / :3113 / :3136 / :3159 | `bSaveFlag`（:3025 初值 false）在四處被檢查、四處被設為 true，卻從未被清回 false。 | 四個自動存檔區塊中只有第一個找到檔案的那個能推進記錄日期，其餘三個靜默跳過日期更新。 | AI(W906-FW3-Observer-W2) 20260825 |
| 33 | GOLDEN NOTE (:3070-3168) | NOTE | cObserver.cpp:6301 | golden :3070-3168 | 四個近乎相同的 30 行區塊，只差路徑、旗標、標籤與查詢型別。 | 典型複製貼上重複，改一處漏三處；也是上一列 bSaveFlag 缺陷得以存在的溫床。 | AI(W906-FW3-Observer-W2) 20260825 |
| 34 | GOLDEN NOTE（未編號） | NOTE | cObserver.cpp:6521 | golden :3174（原註記未標行號，函式體 golden :3172-3175） | btAutoSaveClick 的整個函式體只有一行被註解掉的 `bAutoSaveEventLog(true);`。 | 這是刻意留空的處理常式（golden 註解說明 20161117 改由小程式上傳以免主程式延遲），不是翻譯缺口也不是 gate。 | AI(W906-FW3-Observer-W2) 20260825 |
| 35 | GOLDEN BUG (:3209) | BUG | cObserver.cpp:6570, 6596（交叉引用） | golden :3209 | ShowQwertyKey 的 min/max 兩個範圍參數對調，實際傳成 min=105、max=atoi(edYieldMin)+1。 | 一般設定（如 Min=0）下變成 min=105 > max=1 的空範圍，Yield 圖表上限欄位的輸入範圍檢查形同失效；孿生的 edYieldMinClick 參數順序是對的。 | AI(W906-FW3-Observer-W2) 20260825 |
| 36 | GOLDEN ODDITY (:3257-3258) | ODDITY | cObserver.cpp:6630, 6652 | golden :3257-3258 | `Y==0` 分支宣告 `int iCol=Y-1;`（即 -1），下一行的 `for(iCol=0; ...)` 立刻覆蓋掉。 | 死初值，無行為影響；孿生的 `Y>0` 分支才真的用 Y-1。 | AI(W906-FW3-Observer-W2) 20260825 |
| 37 | GOLDEN ODDITY (:3262-3268) | ODDITY | cObserver.cpp:6633 | golden :3262-3268 | 迴圈用 `bShowYieldAll[iRow]` 的「當前」值去寫整列 bShowYieldSeries，之後才翻轉該旗標。 | 點表頭時整列被設成旗標原本就有的狀態而非新狀態，全選/全不選的視覺回饋慢一拍。 | AI(W906-FW3-Observer-W2) 20260825 |
| 38 | GOLDEN NOTE (:3620-3627) | NOTE | cObserver.cpp:6706 | golden :3620-3627 | ReleaseNote.txt 用寫死的絕對路徑 "D:\HT9045\config\ReleaseNote.txt"。 | 機台安裝目錄一搬家版本說明就讀不到；因屬讀取操作，port 保持 ACTIVE 且刻意不套用 W906_EVENTLOG_ROOT 轉址。 | AI(W906-FW3-Observer-W2) 20260825 |
| 39 | GOLDEN BUG (:3641-3644 + :3646/:3652/:3680) | BUG | cObserver.cpp:6826, 6847, 6891 | golden :3641-3644 + :3646 / :3652 / :3680 | 結構 `aaa QQ` 未初始化，ReadFile 只在 handle 有效時執行且沒有檢查回傳值，之後三個 Memo 照樣把 QQ 印出來；CloseHandle(Fp) 還無條件對 INVALID_HANDLE_VALUE 呼叫。 | 使用者取消檔案對話框時，Memo2/3/4 會印出未初始化的堆疊內容；port 中 GATE C-log-9 使 FileName 恆為 ""，所以這條路徑必定發生（僅顯示、不寫檔，無外部副作用）。 | AI(W906-FW3-Observer-W2) 20260825 |
| 40 | GOLDEN NOTE (:3670-3672) | NOTE | cObserver.cpp:6835 | golden :3670-3672 | Memo2 同時收下 site/index/bin 三列，Memo3 與 Memo4 各只收一列。 | Memo2 是超集而非第三個平行欄位，判讀畫面 log 時容易誤以為三個 Memo 對稱。 | AI(W906-FW3-Observer-W2) 20260825 |
| 41 | GOLDEN NOTE（未編號） | NOTE | cObserver.cpp:6911 | —（此註記未引用任何 golden 行號） | 說明先前 Wave 2 只是把 pgcMessageChange (golden :4766-4793) 的原文當作範圍邊界證據抄錄在本檔註解中，並非翻譯，本波才是唯一定義。 | 純交接記錄，無行為影響；註記寫「near cObserver.cpp:3045」，實際那段邊界註解在 cObserver.cpp:3102-3108（原註記寫 :3045，已核對更正）。 | AI(W906-FW3-Observer-W2) 20260825 |
| 42 | GOLDEN BUG (:3385 of the dump, golden :5385) | BUG | cObserver.cpp:6995, 7015 | golden :5385 | 1..12 迴圈結束後又多執行一次 `TestList->Add(str);`，此時 str 仍是 12 月那一行。 | 產生的 2.bat 會有兩行相同的 12 月 XCopy；實務無害（XCopy /y 重複覆蓋），且 port 中存檔與執行那一對在 GATE C-log-10 內不會執行。 | AI(W906-FW3-Observer-W2) 20260825 |


### cConfiguration.cpp（2 筆，20260825 FW-CFG-W3／W4a 當波入帳，新增分節）

> 本檔在 FW-CFG-W1/W2/W3 三波翻譯前為 0 筆。這一筆不是人工翻閱找到的，是 **MinGW 的
> `unknown escape sequence` 警告**指出來的，隨後對 golden 全檔字串常值做「單一反斜線後接
> 非法跳脫字元」掃描複驗，**全檔正好只有這三處**，且都在 ItemN 的 Handler_OEE 區段。
> 依維護規則 6 併成一列（同一個缺陷、三個呼叫點）。

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話) | 潛在影響(一句話) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | GOLDEN BUG (CFG-N14esc) | BUG | cConfiguration.cpp:3846-3870（banner）,3871,3875-3876,3879-3880 | golden :3481, :3485, :3488 | 三個 GTK 整合的預設執行檔路徑寫成 `"C:\\GTK\EMG.exe"`——第一個反斜線有加倍、第二個沒有，於是 `\E` / `\L` / `\I` 被當成跳脫序列，目錄分隔符被吃掉；同區塊其他路徑（`"C:\\GTK_Control\\Message\\Alarm\\"`）都寫對，故為三個手誤而非慣例 | 這三個是 ini 鍵不存在時寫入的**預設值**，且是要被啟動的執行檔路徑，新機台會拿到不存在的 `C:\GTKEMG.exe`，N14_14 警報控制／N14_15 Socket 壽命／N14_16 IPSC 三個整合會安靜地不啟動；另 `\E` 在 GCC 是 GNU 擴充的 ESC(0x1B)、BCB6 給普通 `E`，故 port 與 BCB6 在第一個預設值差一位元組（兩者皆錯，都指不到真實檔案） | AI(W906-FW-CFG-W3) 20260825 |
| 2 | GOLDEN ODDITY (CFG-L11tag) | ODDITY | cConfiguration.cpp:3679-3697（banner）,3698；另 Public/HTEditList.cpp:308,430,457,483,507（被還原的 Tag 回寫） | golden :3314（`edL11_1->Tag=3`）vs :2932,:2937,:2939（`elConfig->Add(edL11_1,...)`）；寫入者 golden Public/HTEditList.cpp:149 及其四個同胞 | `edL11_1` 是全檔唯一一個 Tag 有兩個擁有者、兩種語意的 widget：ItemL 的 `elConfig->Add()` 把它的**清單索引**鏡射到 `->Tag`，之後 dispatcher 先跑 ItemL 再跑 ItemN（golden :4505 → :4507），ItemN 再用字面值 `3` 蓋掉，所以 `InitConfigEdtList()` 收工時 Tag 是 3 而不是索引 | 目前潛伏：全樹掃描顯示 1,108 個被 Add 註冊的 widget 裡只有它同時被直接指派 Tag，而且沒有任何地方用 `elConfig->FEditList->Items[edL11_1->Tag]` 查表（ChangeCBListProperty 的 13 個查表用的是 cbA09/cbD42/cbD44/cbF06/cbF11/cbF14/cbF14_1/cbF17/rgF26/cbI04/cbI06/cbP16/cbP24）；一旦未來有人改用 Tag 查 edL11_1，會取到清單第 3 項 | AI(W906-FW-CFG-W4a) 20260825 |

### RotateKit/aRotateKIT_In.cpp + RotateKit/aRotateKIT_Out.cpp —— case 4700 重試後卡在「Z 停在原點」（1 筆，20261001 Ifor01 回報、筆電核過行號後入帳）

| # | 標記/編號 | 類型 | port 位置(檔:行) | golden 位置 | 現象摘要(一句話,繁中) | 潛在影響(一句話,繁中) | 發現波次/日期 |
|---|---|---|---|---|---|---|---|
| 1 | （程式碼裡還沒有 GOLDEN 標記；來源 FROM_IFOR §3 20261001 09:1x） | BUG | aRotateKIT_In.cpp:3691-3694；aRotateKIT_Out.cpp:3806-3809 | golden 906 aRotateKIT_In.cpp:3243、aRotateKIT_Out.cpp:3335（V912 :3370／:3393 同一段） | 取料失敗重試的 case 4700 先把手臂歸零再回 case 4000，歸零後 Z 停在原點而不是安全高度，case 4000 移 XY 之前的 Z 檢查（CheckInArmZ／CheckOutArmZ）永遠成立。 | 入料臂一直重複歸零 Z、點動 pitch；出料臂綠燈亮著不動也不報警，要按 HOME 才停。Ifor 0922 在 V912 本機插 case 4710（先把 Z 移到安全高度）修過、未交付；移植樹要不要修＝NIGHT_REPORT §0 第 27 項（Jimmy 決定），在那之前照翻。【✅ **移植樹已修**（I-04，Ifor01 20261001，`v906/ifor-i04`；RULINGS_20261001 第 15 條，Jimmy 1001 13:2x「照建議」）：case 4700 之後先到 case 4710 把 Z 移到安全高度（入料臂 MoveInArmZToPlateSafe、出料臂 MoveOutArmToAutoSafe），到了才回 4000（`AI(W906-I04)`，aRotateKIT_In.cpp:3694-3695、aRotateKIT_Out.cpp:3809-3810）。同一顆照第 0 條解開入料臂的 GATE (9)（aRotateKIT_In.cpp:376）：CheckInArmZ 已有真本體（ainarm2.cpp:734），之前閘成 false＝這道 Z 互鎖在移植樹沒有作用，所以入料臂的迴圈在移植樹原本不會重現（出料臂的 CheckOutArmZ 是活的，會）。ctest `RotateKitRetry`。V912 那份走案件流程】 | 20261001 |

## 維護規則

本台帳需要**每一波**（無論是 PT 純翻譯波次、FW DFM→WEB 波次，或任何新功能開發）在程式碼裡
新增 `GOLDEN BUG` / `GOLDEN ODDITY` / `GOLDEN ASYMMETRY` / `GOLDEN DEFECT` / `GOLDEN QUIRK` /
`GOLDEN NOTE`（或本文件「掃描範圍聲明」列出的其他同義詞彙）類註記時，**同步在本檔案與對應的
`GOLDEN_DEFECT_LEDGER.html` 補上一列**，具體做法：

1. 新增 `//AI(W906-<工作代號>) YYYYMMDD:` 註解且內容是「golden 本身缺陷、忠實翻譯保留」時，
   同時在該註解裡明確使用上述詞彙之一（不要發明新詞彙——若真的需要新詞彙，先在此檔的
   「掃描範圍聲明」補登，避免下一次台帳維護又要重新做一次全樹詞彙普查）。
2. 在本檔案找到（或新增）該 port 檔案對應的分節，於表格最後一列之後插入新的一列，
   `#` 欄延續該分節內既有的最大編號 +1。
3. 若新缺陷讓某個檔案的筆數超過相鄰分節，視情況調整分節順序以維持「檔案依筆數降冪」——
   不要求每次都重新排序全檔，但半年內至少一次「補一句彙整」時應重新核對排序是否嚴重失真。
4. 同步更新「統計摘要」表格裡對應類型的估算筆數與涉及檔案數。
5. **手動同步更新 `GOLDEN_DEFECT_LEDGER.html` 的對應表格**（本文件與 html 是兩份獨立檔案，
   沒有自動產生機制；html 表格的欄位與資料須與本 md 檔逐列一致）。
6. 若新缺陷剛好與既有某一列描述同一件事（例如同一缺陷在另一個呼叫點被重新提及），
   **不要新增一列**，改成在既有那一列的「port 位置」欄追加新的檔:行引用。
7. 若不確定新項目是否為「真的 golden 缺陷」還是「GATE（尚未翻譯的真實呼叫，fail-closed
   代替值）」，參照本文件「掃描範圍聲明」的判斷準則：註解文字本身有沒有明確使用
   GOLDEN-缺陷詞彙。沒有的話就是 GATE，不進本台帳（GATE 有自己的追蹤機制，通常是各檔案
   自己的 `GATE REGISTER` banner，與本台帳是互補而非重複的兩份文件）。

