# 答案消耗、used 與結果保存

來源：[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的 `TakeAnswer`、`W906_ReauthHasAnswer`、`W906_ReauthHasAnswerFor`、`W906_ReauthSetupDoPassword`、`W906_ReauthConfigM01`、`W906_ReauthConfigI37` 及 `Keep(const W906ReauthResult&)`。

## 選取與清理

TakeAnswer 先確認 `g.present` 與 `g.tag`，再找第一個尚未 `g.used[i]`、且符合指定 point（有指定時）的答案。複製給回傳值後標 used，再清原 stash 的 password／userId；找不到則回傳新建的答案物件。這個 used 只表示此函式取走答案，不表示登入或儲存成功。回傳副本的兩個欄位由選讀 wrapper 在 callee 返回後 Wipe；完整例外與其他副本生命週期尚未查完。

HasAnswer 確認相同 tag 尚有未用答案；HasAnswerFor 另比 point。兩者都沒有在所讀 body 做登入或磁碟驗證。Keep 的 Result overload 直接 append `g.results`；同檔 Keep(Note) 是另一個 overload，本次不把它的容量／去重策略套給 reauth results。

## 三個 wrapper 的差異

| 函式 | 選取／呼叫 | 所讀失敗分支 |
| --- | --- | --- |
| W906_ReauthSetupDoPassword | TakeAnswer(kSetupTag) 未另指定 point；W906_Reauth 的 setupNeedPassword 來自 bNeedPassword | 加入需要密碼且原未勾選的 RTC／OCR reverted 項；實際 UI 還原 caller 待查 |
| W906_ReauthConfigM01 | TakeAnswer(kConfigTag, m01)；呼叫 W906_Reauth | 對 changed 呼叫存在的 revert callback，並把每項加入 reverted；callback 效果待查 |
| W906_ReauthConfigI37 | TakeAnswer(kConfigTag, i37_1)；呼叫 W906_DoPasswordMBox | 加入 kI37[0] reverted 項；DoPasswordMBox 本次未讀完，不套 M01 判斷 |

三者都清答案副本、Keep(Result)、在 bHandled 指標存在時寫入 r.handled，最後回傳 bFlag。wrapper 的 bFlag、handled、reverted 與[儲存結果／Ack](../customer-reauth/clear-ack.md)仍分開看；完整 caller、所有 stash／結果寫者、point 決定與跨連線並行尚未閉合。
