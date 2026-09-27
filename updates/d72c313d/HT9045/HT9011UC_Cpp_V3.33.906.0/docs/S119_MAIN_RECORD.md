# S119 主畫面 Record 分頁（main.dfm tsRecord）移植帳本

> St02（Steven02，分支 `v906/steven-gpib-widget`）。github-59 GO（FROM_STEVEN §1 00:02 認領；20260927 裁決 a／c／d）。
> golden 一律是 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（cp950），行號都是那一份的。

## golden 盤點

| 元件（main.dfm:16128-16262） | golden 程式 | 結論 |
|---|---|---|
| `palCounter`（"Counter"）＋ `sgDebugRecord`（TStringGrid 5×9） | 全樹**只有**表頭四格會被寫：`sbLaguageClick` main.cpp:8921-8926（中：本次記錄／累計記錄／系統記錄／前次記錄）、:8942-8945（英：`Current  `／`Aggregate`／`System   `／`Last     `） | 資料列從來沒人寫；表頭要等語言切換 |
| `spbClearRecord`（CLEAR） | `spbClearRecordClick` 906 :30062-30091（912 :31109-31138）：`bShowMainDebugRecord==false` 就 return；`fProductionInfo->CalculateNowArmSiteBinQty(true)`、`ArmData[0..2]->ClearALLCT()`、`fProductionInfo->UpdateControlBinCount(true)`、`UpdateRecordScreen(true)`；沒有確認框 | `act.main.clearRecord` |
| `memoAutoClean` ＋ `btSavelog`（Save Log） | FormShow :11147-11153 只在 `#ifdef DEBUG_AUTO_CLEAN` 顯示，MachineType.h 那行是註解；`AddAutoCleanMessage` :31295-31308、`btSavelogClick` :31310-31317（存 `d:\AutoCleanLogs\*.csv`） | 出貨版藏起來 |
| `AseRecordMemo` | 程式裡零引用；OnDblClick＝`meShuttle2DblClick` 906 :28427-28431（912 :29471-29475；清 meShuttle2／meShuttle1，不是清自己） | 永遠空；雙擊＝`act.main.meShuttle2Dbl` |

- `bShowMainDebugRecord` 預設 false（CosFunction.cpp:4208 InitialCosFunction），只有 `CC_SIGURD_HUKOU`（:1074）、`CC_RICHTEK`（:2959）打開 ⇒ 其他客戶按 CLEAR 是 no-op。

## golden 怪處（照留）

- **表頭開機是空的**（#18 那一類：golden 的明顯怪處，先照 golden）：`sgDebugRecord` 的表頭只在 `sbLaguageClick` 寫，
  開機後到第一次切換語言之前四格都是空的。github-59 20260927 裁決 (a)：照 golden；網頁有語言切換事件就接在那個事件上，
  不在頁面載入時填。網頁的語言切換＝background.html 廣播的 `{type:'HT_LANG', lang}`：`zh*` 用中文字樣，其他（en／ja／ko）用英文字樣
  （golden 只有 `iLanguageCountry` 0／1 兩種）。頁面重新載入後表頭又是空的，直到下一次切換。

## V906 做了什麼（20260927）

- `JsonBridge/actions/MainRecordClear.h/.cpp`（新）：
  - `W906_SpbClearRecordClick()`＝golden 本體逐行；`W906_MeShuttle2DblClick()`＝`fMain->meShuttle2->Clear(); fMain->meShuttle1->Clear();`。
  - `act.main.clearRecord`／`act.main.meShuttle2Dbl`：守衛 `forms-not-created`、`debug-record-off`（golden 906 :30064，912 :31111）；
    `{"dryRun":true}` 只跑守衛（RULINGS 第 12 條，同 clarnData）；**沒有確認框**（github-59 (c)：照 golden）；
    單一操作員權杖由 WebBridgeServer 在分派前擋，跟其他 act.main.* 一樣（唯讀開關 `SetReadOnly(!allowCmd)` 自 ZEROARG 起恆為可寫，wb_serve.cpp:3670）。
  - R1（20260927 起改成函式指標，見下一節「R1」）：~~`UpdateRecordScreen(true)` 保持 `#if 0`~~——本體是 St01 S113 的 `W906_TfMain_UpdateRecordScreen`（FileRW/MainRecord.cpp），
    只在 cbridge-review6、只編進 wb_serve；S113 進 main 之後再接（不先做 hook）。golden 本體不看 Attr，少叫這一次只是少一拍稼動時間累計。
  - meShuttle1／2 是 `TfMainMemo`：筆電的 AI(W906-MEMO)（`42ea830b`，INBOX 第 75 列結案）起真的存行，`Clear()` 真的清空；ctest `MainRecord_Clear` 第 4 部分驗「dryRun 不清、執行後兩個都是 0 行」（20260927 更新）。
- `web/page/Main.Record.html`：CLEAR 旁加狀態列；`memoAutoClean`／Save Log 照 golden 藏；載入 `ht9045_recipe_client.js` 與新的
  `ht9045_mainrecord_wire.js`（CLEAR、AseRecordMemo 雙擊、`HT_LANG` 時寫表頭）。分派沒接時按鈕回「分派未接」，不假裝清了。
- ctest `MainRecord_Clear`（`tests/test_main_record_clear.cpp`，tests/CMakeLists.txt 我的區段）：debug record 關著時不清、dryRun 不清、
  payload 壞掉只預覽、打開時 ArmData[0..2] 歸零、meShuttle2Dbl 跑得到。只動記憶體。

## 共用檔接線（20260927，github-59 GO，FROM_STEVEN §1 `a31aaaf7`）

- `JsonBridge/ChanAction.cpp`：:15 include、:348-349 兩行分派、:353 拒絕訊息補兩個名字（同一行）、:409-410 兩個 schema 物件。
- 根 `CMakeLists.txt` wb_serve 來源清單 :3375、`tests/CMakeLists.txt` test_sjson_chan 來源 :3753 加 `MainRecordClear.cpp`
  （只有這兩個 target 編 ChanAction.cpp）。
- `tests/test_sjson_chan.cpp` :342-343：兩個動作用 `dryRun:true` 分派走得到（不是 unknown-action、不清任何東西）。

## R1：UpdateRecordScreen(true) 走函式指標（20260927，St01 06:00 提、github-59 §4 0261bae3 同意）

- `MainRecordClear.cpp` 同時編進 wb_serve 與兩支 ctest（MainRecord_Clear、SjsonChan），而 St01 的 `W906_TfMain_UpdateRecordScreen`
  只編進 wb_serve ⇒ 直接呼叫會讓 ctest 連結失敗。
- 所以本檔放 `void (*W906_UpdateRecordScreenBody)(bool) = 0;`（.h 宣告），CLEAR 那一行＝`if (W906_UpdateRecordScreenBody) W906_UpdateRecordScreenBody(true);`
  （forms/fMain.cpp:507 `W906_*Hook` 同一種接法）。回應裡：沒裝列在 `skipped`，裝了列在 `cleared`。
- **現在沒有人裝**，行為跟之前一樣（少那一拍稼動時間累計）。St01 在 S113 進 main 時，於自己的 wb_serve 開機那一行裝上。
- ctest `MainRecord_Clear` 第 5 部分：預設 NULL、NULL 時列為 skipped；裝上測試替身後，debug record 關著或 dryRun 都不呼叫，
  打開時在三個清除**之後**呼叫一次、參數 true、回應列為已做。

## 還沒做

- R1 的另一半：St01 在 wb_serve 開機裝 `W906_UpdateRecordScreenBody = &W906_TfMain_UpdateRecordScreen`（S113 進 main 之後）。
- ⚠ 未編譯（本機沒有編譯器）；網頁 `node --check` 通過，沒在瀏覽器看過。

## golden 行號（20260927）

- 規定是對照 golden 906（`HT9011UC_Code_V3.33.906.0_20260618`）。上面標 `906` 的行號（spbClearRecordClick :30062-30091、
  UpdateRecordScreen(true) :30090、meShuttle2DblClick :28427-28431）是 **NB2 R92 提供的，St02 自己沒驗過**——這台只有加密的 7z，
  而本機解開的 `906.0_20260625_Steven` 從 :28427 之前起比 0618 多 78 行，不能拿來當 golden 行號。
  本體已在 `906_0625_Steven` 與 912 之間逐行比過相同（見 `docs/ST02_GOLDEN906_AUDIT.md`）。
- 其他還是 912 的：`sbLaguageClick`（:8921-8945）、FormShow `DEBUG_AUTO_CLEAN`（:11147-11153）、`AddAutoCleanMessage`、`btSavelogClick`、
  `main.dfm` 行號，以及 `web/page/ht9045_mainrecord_wire.js` 檔頭——等 NB2 的工具做全面換算，不手動位移。
