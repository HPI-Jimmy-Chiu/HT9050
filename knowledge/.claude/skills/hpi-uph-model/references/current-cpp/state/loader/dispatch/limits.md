# 查證界線與下一步

[manifest](source-manifest.json) 分開記三個完整 caller body 的保存 hash、六個已讀 caller 入口／case、三個已讀 HT9050 完整函式文字，以及八個 HT9050 本地區段。後三函式仍只核對本地敘述，完整機台語意／callee／設定未驗證；前三函式沒有以保存 hash 充當完整閱讀。

`DoLoadNewICTray_9050` 的定義在同一行前面還有 helper 宣告；只認行首 `bool` 的搜尋會漏掉它。證據定位使用 lexical mask、完整 signature 與括號邊界；byte／blob／body／case 及前後版本相同內容可重現。

下一步沿 `DoCatchFromLoader_9050`、實際 InArm／Task 派工、`CalculateUPH` caller、旗標與 counter 所有寫者追完整取樣鏈，再查 DB／SECS／UI／CSV 的使用與保存成功。盤面設 `HAS_IC`、供盤函式 true、SECS 事件與 UPH 每顆口徑分別查證；容量／site／校正不能從模擬值 5 或 HTML 預設值推定。

本單元不是全樹 caller census，也不判定客戶機台 bug、實際旗標永不觸發或 UPH 功能缺失。與 [同題機型樹](../../../../machines/index.md)、[狀態生命週期](../../index.md) 及 [前層 Loader 旗標](../index.md) 一起讀。未執行機台、C++／build、IO、API、LIVE、Home 或 runtime 設定操作。
