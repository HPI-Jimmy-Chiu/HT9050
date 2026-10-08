# 兩條瀏覽器消費路徑

定位testercomm.html的`renderGpib`／`stateBadge`／`getJson`／`poll`／`pollAllowed`／`post`、API／PERIOD與host message區段；background.html的`applyPageTable`／PAGES_APPLIED／ui.pages listener。
完整7個JS函式與所選安裝區段見 [manifest](source-manifest.json)。本段沒有開瀏覽器、GET／POST API或量測延遲。

## GPIB畫面走HTTP

API為`/api/testercomm/`；PERIOD=300，timer定期呼叫poll且安裝後先呼叫一次。引擎200ms註解及輪詢300ms常數不是端到端實測延遲。
`pollAllowed`要求document非hidden；收過parent的HT_WIN後還要求winShown，未hosted的頁面可輪詢。listener只接parent且不同於自己window，開窗邊緣立即poll。

`poll`在demo時render離線資料；live路徑以inflight避免同時多次請求，只取當前tab需要的key。getJson要求HTTP ok，再解析JSON；失敗映射null、stateBadge顯示離線。文字連線中只由snapshot.up判斷，不是NI bus探測。

`renderGpib`顯示LED／status／控件／部分logs，沒有取`ibsta`／`iberr`／`cardAddress`。在此pin整份testercomm.html中這三個literal及`reset flags`出現數均為0；這是文字搜尋證據，不是所有頁面語意或動態輸入的證明。raw欄位存在於API JSON，不應寫成已在這個renderer直接呈現；也沒有由本段證明reset按鈕已連上。

`post`在demo只console記錄，其餘把cmd URL encode後用POST；此函式沒有消費HTTP status或queued body，catch亦不顯示錯誤。完整postChain／其他command UI與transport ack仍待查。

## 程式開關走ui.pages tag

background有HT9045Tags時安裝listener，交`applyPageTable`。它解析字串或使用object，拒不含rows／forEach的資料；每row要求id／want／wseq且不能等於PAGES_APPLIED內已消費的序號。
它**先記下wseq，再確認id在WIN_STATE**；id不存在就返回。open與minimized都當作on，只有want=open且off時openWin，want=close且on時closeWin。

每分頁的PAGES_APPLIED初值空，重載會重新消費目前要求；相同wseq在同一頁不重做。這是本地去重狀態，不是C++或所有HMI收到的ack。
PAGES_CHECKED僅首次比較WINDOWS／C++ rows（排除SHOT_WINDOWS與含冒號的id），結果放HT_PAGES_MISMATCH並console；未比較完整DOM、profile機型或窗體生命週期。

回 [入口](index.md)；原始program hooks與tag序列見 [頁面狀態](pages.md)。
