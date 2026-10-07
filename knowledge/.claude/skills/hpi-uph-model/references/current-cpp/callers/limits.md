# 字面盤點、版本差異與查證界線

pin `f2a1d78160e9f44f6bb351f0a3552ae36e87a031` 的 2289 個追蹤cpp／h只掃兩個指定root，排除.svn／docs／tests；先做byte原詞搜尋，僅10個正命中來源檔strict解碼再mask註解／字串，保留13個函式形狀（7 call、3 definition、3 declaration）。清單SHA／各語句／guard見 [manifest](source-manifest.json)。這不涵蓋巨集、別名、function pointer、其他root或完整runtime可達性。

V906 ainarm9045_2x4_16_shims.cpp的空AddLoadingCount定義在#ifndef HT9045_2x4_16_SHIMS_DEFINED內的#if 0；宣告guard與body停用分開。V906 2x8_32含兩個同名DoArmPickFromLoadStage_9045_2x8_32定義：讀到的短stub只return false，含所選AddLoadingCount call的長body則在#if 0。所選body用definition_index／signature／SHA／guard定位，不能只按名字取第一個body或把停用舊稿當目前計數流程。

V912所選標準、1x2_1、2x4_16、2x8_32 call沒有字面停用guard；這不等於確認它們已建置、已分派或在HT9050／其他機台執行。V912 Magazine.cpp只是原文候選，去掉註解／字串後沒有本詞函式形狀，不據此推定Magazine不參與UPH。

七完整caller body hash僅保存／定位，真正已讀為七個call附近區段；另完整讀一個return-false短stub。未讀全部機構流程／Task／retry、初始化與旗標writer／callee、Make／CMake／MachineType dispatch、asendic_Loader、其他counter、DB／SECS／UI／CSV consumer與實際寫檔。HT9050／其他Handler、客戶與runtime／site／容量／校正沿用 [同題機型樹](../../machines/index.md)，仍需版本與現場資料。

前層source pin、原稿／metadata／裁決／資源／相容入口不改。沒有執行C++、build、Suck／IO、API／LIVE、Home、Profiler、CSV writer、機台或runtime。
