# WsDecoder的文字拒絕時點

[上層](index.md)；[完整validator](raw/source-03.md)；
[class契約](raw/source-14.md)；[constructor](raw/source-15.md)。
constructor把validateUtf8_初始化為true；SetValidateUtf8(on)可改開關。
class原註解提及違規1007與RFC，保存為原契約；本輪未執行協定測試或重新認證標準符合性。

本單元只保存TryOneFrame的三個UTF8相關分支：

| 分支原文 | 條件與交付順序 |
|---|---|
| [close reason](raw/source-11.md) | validateUtf8_為true且m.closeReason無效，Fail(kWsCloseInvalidPayload,...)後return -1。 |
| [continuation完成](raw/source-12.md) | frag_+=payload；!fin先return 1；fin時搬到m.payload並清frag_／fragOpcode_，再只驗kWsText，失敗return -1。 |
| [單frame](raw/source-13.md) | validateUtf8_為true且opcode=kWsText才檢查；失敗return -1；通過才out->push_back(m)。 |

分片前段不在這個完成分支逐chunk判無效；最後fin對重組後whole message檢查。
停用validateUtf8_或binary訊息不走這些text拒絕條件；其他frame大小／opcode／mask／close code規則
不因本單元選定三分支而宣稱已全部查證或重跑。
Fail的全部狀態變更、Feed到network的關閉及browser處理仍需查其子callee／caller。
本class的同名member沒有把資料送SanitizeToUtf8，也不能用JSON repair替代這些原拒絕路徑。
