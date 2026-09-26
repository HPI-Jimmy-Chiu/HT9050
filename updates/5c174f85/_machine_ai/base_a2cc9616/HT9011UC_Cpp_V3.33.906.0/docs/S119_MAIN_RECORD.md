# S119 主畫面 Record 分頁（main.dfm tsRecord）移植帳本

> St02（Steven02，分支 `v906/steven-gpib-widget`）。github-59 GO（FROM_STEVEN §1 00:02 認領；20260927 裁決 a／c／d）。
> golden 一律是 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（cp950），行號都是那一份的。

## golden 盤點

| 元件（main.dfm:16128-16262） | golden 程式 | 結論 |
|---|---|---|
| `palCounter`（"Counter"）＋ `sgDebugRecord`（TStringGrid 5×9） | 全樹**只有**表頭四格會被寫：`sbLaguageClick` main.cpp:8921-8926（中：本次記錄／累計記錄／系統記錄／前次記錄）、:8942-8945（英：`Current  `／`Aggregate`／`System   `／`Last     `） | 資料列從來沒人寫；表頭要等語言切換 |
| `spbClearRecord`（CLEAR） | `spbClearRecordClick` :31109-31138：`bShowMainDebugRecord==false` 就 return；`fProductionInfo->CalculateNowArmSiteBinQty(true)`、`ArmData[0..2]->ClearALLCT()`、`fProductionInfo->UpdateControlBinCount(true)`、`UpdateRecordScreen(true)`；沒有確認框 | `act.main.clearRecord` |
| `memoAutoClean` ＋ `btSavelog`（Save Log） | FormShow :11147-11153 只在 `#ifdef DEBUG_AUTO_CLEAN` 顯示，MachineType.h 那行是註解；`AddAutoCleanMessage` :31295-31308、`btSavelogClick` :31310-31317（存 `d:\AutoCleanLogs\*.csv`） | 出貨版藏起來 |
| `AseRecordMemo` | 程式裡零引用；OnDblClick＝`meShuttle2DblClick` :29471-29475（清 meShuttle2／meShuttle1，不是清自己） | 永遠空；雙擊＝`act.main.meShuttle2Dbl` |

- `bShowMainDebugRecord` 預設 false（CosFunction.cpp:4208 InitialCosFunction），只有 `CC_SIGURD_HUKOU`（:1074）、`CC_RICHTEK`（:2959）打開 ⇒ 其他客戶按 CLEAR 是 no-op。

## golden 怪處（照留）

- **表頭開機是空的**（#18 那一類：golden 的明顯怪處，先照 golden）：`sgDebugRecord` 的表頭只在 `sbLaguageClick` 寫，
  開機後到第一次切換語言之前四格都是空的。github-59 20260927 裁決 (a)：照 golden；網頁有語言切換事件就接在那個事件上，
  不在頁面載入時填。網頁的語言切換＝background.html 廣播的 `{type:'HT_LANG', lang}`：`zh*` 用中文字樣，其他（en／ja／ko）用英文字樣
  （golden 只有 `iLanguageCountry` 0／1 兩種）。頁面重新載入後表頭又是空的，直到下一次切換。

## V906 做了什麼（20260927）

- `JsonBridge/actions/MainRecordClear.h/.cpp`（新）：
  - `W906_SpbClearRecordClick()`＝golden 本體逐行；`W906_MeShuttle2DblClick()`＝`fMain->meShuttle2->Clear(); fMain->meShuttle1->Clear();`。
  - `act.main.clearRecord`／`act.main.meShuttle2Dbl`：守衛 `forms-not-created`、`debug-record-off`（golden :31111）；
    `{"dryRun":true}` 只跑守衛（RULINGS 第 12 條，同 clarnData）；**沒有確認框**（github-59 (c)：照 golden）；
    單一操作員權杖由 WebBridgeServer 在分派前擋，跟其他 act.main.* 一樣（唯讀開關 `SetReadOnly(!allowCmd)` 自 ZEROARG 起恆為可寫，wb_serve.cpp:3670）。
  - gate R1：`UpdateRecordScreen(true)` 保持 `#if 0`——本體是 St01 S113 的 `W906_TfMain_UpdateRecordScreen`（FileRW/MainRecord.cpp），
    只在 cbridge-review6、只編進 wb_serve；S113 進 main 之後再接（不先做 hook）。golden 本體不看 Attr，少叫這一次只是少一拍稼動時間累計。
  - meShuttle1／2 還是 `TfMainMemo` 替身（Clear 不做事）；jimmychiu 的 INBOX 第 75 列改指 `vclcompat::TMemo` 之後才真的清。
- `web/page/Main.Record.html`：CLEAR 旁加狀態列；`memoAutoClean`／Save Log 照 golden 藏；載入 `ht9045_recipe_client.js` 與新的
  `ht9045_mainrecord_wire.js`（CLEAR、AseRecordMemo 雙擊、`HT_LANG` 時寫表頭）。分派沒接時按鈕回「分派未接」，不假裝清了。
- ctest `MainRecord_Clear`（`tests/test_main_record_clear.cpp`，tests/CMakeLists.txt 我的區段）：debug record 關著時不清、dryRun 不清、
  payload 壞掉只預覽、打開時 ArmData[0..2] 歸零、meShuttle2Dbl 跑得到。只動記憶體。

## 共用檔接線（20260927，github-59 GO，FROM_STEVEN §1 `a31aaaf7`）

- `JsonBridge/ChanAction.cpp`：:15 include、:348-349 兩行分派、:353 拒絕訊息補兩個名字（同一行）、:409-410 兩個 schema 物件。
- 根 `CMakeLists.txt` wb_serve 來源清單 :3375、`tests/CMakeLists.txt` test_sjson_chan 來源 :3753 加 `MainRecordClear.cpp`
  （只有這兩個 target 編 ChanAction.cpp）。
- `tests/test_sjson_chan.cpp` :342-343：兩個動作用 `dryRun:true` 分派走得到（不是 unknown-action、不清任何東西）。

## 還沒做

- R1：S113 進 main 後接 `UpdateRecordScreen(true)`。
- ⚠ 未編譯（本機沒有編譯器）；網頁 `node --check` 通過，沒在瀏覽器看過。
