# Key：guard報錯後仍寫入

[上層](index.md)；[原文](raw/source-01.md)；[BeforeValue](../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/auth-callees/login-json/raw-value.md)。

`JsonWriter::Key` 在stack_空、頂層不是kCtxObject或已有keyPending_時寫ok_=false。
這個guard沒有return；後續仍處理needComma_、附加JsonQuote(name)與冒號，
最後寫keyPending_=true並回*this。因此非法呼叫也可能改buf_；reference回傳不代表成功。

needComma_為true時先加逗號並清為false，讓下一次value的BeforeValue不在冒號後加逗號。
下一次value由已保存的BeforeValue清keyPending_並設needComma_=true；
這是兩段正文的狀態連接，沒有執行呼叫序列或驗證實際輸出。

Key使用JsonQuote；本單元尚未保存其完整實作，不能由函式名推定編碼／控制字元契約。
本方法沒有按既有ok_=false略過寫入，也沒有自行把ok_恢復true。
完整caller是否先檢查容器或事後檢查Ok另續。
