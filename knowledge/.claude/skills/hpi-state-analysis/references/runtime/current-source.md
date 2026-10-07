# V906來源／caller核對

基準main `811d950686ff6b05ab8e09183ca637c347f3be94`。只做Git來源及字面guard檢查，未建置、建立State Record、呼叫API或執行機台；來源存在／有caller不代表本輪現場驗證。

[cStateRecord.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/811d950686ff6b05ab8e09183ca637c347f3be94/HT9011UC_Cpp_V3.33.906.0/cStateRecord.cpp)的TfMain::W906_DoStateRecordBody在DumpMainFormSnapshot後呼叫W906_StateRecordDiagPublishNow與W906_StateRecordDiagWriteFiles；W906_InstallStateRecordBody將W906_StateRecordBody設為StateRecordBodyTrampoline。[wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/811d950686ff6b05ab8e09183ca637c347f3be94/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)有install呼叫，這些定位沒有字面#if 0包覆。尚未核對全部實際編譯條件、記錄busy／等待與所有runtime gate。

[StateRecordDiag.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/811d950686ff6b05ab8e09183ca637c347f3be94/HT9011UC_Cpp_V3.33.906.0/StateRecordDiag.cpp)、[StateRecordHang.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/811d950686ff6b05ab8e09183ca637c347f3be94/HT9011UC_Cpp_V3.33.906.0/StateRecordHang.cpp)及S-24已有來源；cStateRecord中的hook描述W906_ReadMe／Health／Motor1203.csv／Home／PowerBrake／Dialogs／OpLogTail／IO.csv／Recent.csv等附加診斷。這是移植附加檔與source導讀，不將全部欄位／writer效果改稱已全量驗證，也不把歷史WIP狀態套到這份main。

取得記錄時仍按提供檔的header／版本／擷取時間查，原[格式文](../source/references/staterecord-file-format.md)的golden與歷史欄位完整保存。MainProcMonitor的LastEnter等欄位只在實際存在時判讀；附加診斷缺檔不能直接認定機台停擺。

S-24程式、hook與現場測試仍遵守原擁有者／認領範圍；本批只整理文件，不改相關程式、狀態快照格式或記錄內容。

## 20261002退版與原文來源邊界

[RULINGS_20261002 第13條](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/811d950686ff6b05ab8e09183ca637c347f3be94/HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261002.md)記錄Jimmy撤回RogerYang七個Skill merge；State Record對應revert `ac3e826b50b8f6fc2d5b582f0ab8850664b89f38`，撤回merge `9ded5d43c965c0182a6518e87262e9ee5a97b5e9`。文件沒有逐段技術否決理由，不由退版自行推定所有歷史方法都錯。

本次23份原文全部取自上述811d95068基準。該基準的`ht9045-state-record-analysis/`樹與ac3e826b5退版完成時逐檔相同；沒有從已撤回merge取回原文，退版刪除的`case-testyfront-task210.md`也沒有重建。`data-analysis/`在目前main仍存在的同事材料另按原文件／日期保存，不能把它們當成恢復被撤回版本的依據。

RogerYang交接分支`v906/roger-handoff`於`185589aa84848978538ba726a5f0a9f36d711d55`的§1未認領這兩支Skill。本次不恢復退版、不改其程式或技術結論；推送前再核對最新main及是否出現同檔認領，若新增衝突只暫停相關文件整合。
