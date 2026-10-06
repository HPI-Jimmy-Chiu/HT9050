> 保存來源：`.claude/skills/ht9045-json-bridge/references/clear-buttons-st01.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 畫面上的清除鈕（St01 接的 act.* 動作）

> 20261002 從 skill `ht9045-clearcount-flow` 搬來。那支 skill 是 RogerYang 第 1 批（dbca0182），Jimmy 20261002 在 main 撤回（7287f4d3，NIGHT_REPORT §0 #33：RULINGS_20261001 #28 先停放 RogerYang 的分支）。St01 在它檔尾加的兩段移植樹筆記原文搬到這裡，內容沒改。
> 程式在哪條分支：E-021（Observer）在 `v906/st01-q59`；E-022（Contact CT）在 `v906/steven-cbridge-review6`。兩邊進 main 前，另一條分支上看不到那段程式。
> 路徑：移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`；golden＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（V912）。

## V906 移植樹：Contact Counter Kinds 的 Count Clear／格子點兩下（todo E-022 CK-1／CK-2，St01 20261002）

- golden（V912）：`cContactCT.cpp:944-1069` TfContactCT::btClearCountClick（SystemStart return :947；等級 107 在處理器裡查 :952／:985，不夠 golden 自己跳 WAR1676；KYEC 強制重新登入 :955-983；YES／NO :1001）、`:740-886` sgYieldDblClick（沒有等級檢查；History 項目不清，HANA 例外 :846-849）。
- ⚠ golden 疑點（照做）：Count Clear 按 **NO** 也會跑 `:1050-1068` 收尾——良率監控的間隔計數、ClearYieldCount、ClearAutoSiteOffStatus、`LastSet.iIndexCount=0`、iLowYieldCloseCount、bStandardYield、iStandardYield、`LastSet.iAutoTempOfsTriggerCnt=0`。只有 YES 才清 ArmData 三支手臂（VTEST 只清兩支）與 bShowSiteYield。
- golden 這支**不存檔**：清掉的是記憶體（ArmData、LastSet），等下一次 golden 的 lastdata.dat／Arm*.dat 存檔時機才落地；MyDBIProductionData 會寫生產／事件記錄。
- 移植樹（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`）：WS `act.contactCT.clearCount`／`act.contactCT.dblClick`／`act.contactCT.yieldChart`（`JsonBridge\ChanAction.cpp:349` 同一行分派 → `cContactCT.cpp` 檔尾 `ht9045::sjson::W906_ContactCTAct`）。YES／NO 由網頁問（`D:\HT9045\web\page\ht9045_contactct_ev.js`），C++ 收到 answer 時守衛重跑；取消＝NO 也要送。主控台一行 `[E022-CK1]`／`[E022-CK2]`。
- 移植樹的 `TfContactCT::btClearCountClick`（cContactCT.cpp:1178，jimmychiu 的翻譯）確認框閘成「No」；Clarn_Data Tag 8（KYEC）／Tag 11 呼叫它時 Sender 不是按鈕，走 golden 的 else 臂（:1038-1049）與收尾，KYEC 那一支在移植樹直接 return，else 臂裡的 CalculateNowArmSiteBinQty／UpdateControlBinCount 仍閘著（那一段不是 St01 的）。網頁的 Count Clear 不經過它。
- 測試：ctest `E022_ContactCT`（tests\test_e022_contactct.cpp）、`E022_ContactCTPage`（tools\webprobe\e022_contactct_selftest.cjs）。

## V906 移植樹：Observer 的 Clear Time Data（todo E-021 OB-9，St01 20261002）

- golden（906 `cObserver.cpp:5393-5399`＝V912 `:5624-5630`，兩棵相同；20261003 E-030 補 906）：TfObserver::btnClearTimeClick＝`LastSet.iJamCount[0..2]=0` ＋ `fCounterClear->ClearCount(ctTimeData)`（`cCounterClear.cpp` 906 :245／V912 :246 那一臂，這一臂兩棵相同：`LastSet.SystemAccSecond[k][i]=0`，k<2——第三組不清——＋ `MyDBIProcess("Process","Time Data has been cleared!!")`）。沒有等級檢查、沒有確認框、運轉中也能按；不存檔（下一次 lastdata.dat 存檔才落地）。
- 移植樹（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`）：Data.Observer Counter 分頁的 Clear Time Data 鈕 → WS `act.observer.clearTime`（`JsonBridge\ChanAction.cpp:344` → `cObserver.cpp` 檔尾 `ht9045::sjson::W906_ObserverAct`）→ jimmychiu 翻好的 `TfObserver::btnClearTimeClick`（cObserver.cpp:7196，原樣呼叫）；頁面 `D:\HT9045\web\page\ht9045_observer_ev.js`，做完叫一次 observer.get timer 讓累計時間格子更新。
- 測試：ctest `E021_Observer` 第 [9] 節（iJamCount 歸 0、SystemAccSecond[0..1] 歸 0、[2] 不動）。

<!-- preserved-content:end -->
