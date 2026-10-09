# PumpSnapshot：generation與每個baseline的diff

[上層](index.md)；[完整body](raw/source-03.md)；[實際sibling adapters](raw/source-11.md)；[Conn已保存context](../raw/source-11.md)。

gen來自sib::SnapGeneration(snapshot)；!force且gen==lastGen_直接return，否則先lastGen_=gen，再檢查conns_.empty()。
因此沒有連線也會記住generation；force只繞過此早退，不會繞過各連線isWs／sentSnapshot／closeAfterFlush過濾。
SnapGeneration在snapshot非空時呼叫generation()、空時0。SnapRead清out、非空時取read().tags；generation與read是兩次呼叫，本body沒有把兩次綁成同一view。
正文不是呼叫TagSnapshot::diffFrom；它手動比cur與c.lastSent，勿把header的diffFrom fast path直接套過來。

| 步驟 | 本body可查的行為 |
|---|---|
| cur | 一次新shared map；pumpT0在SnapRead前，copy tags後再diff |
| frames | 本次pump區域vector，保存baseline shared_ptr及編碼frame；按pointer identity共用，並非比較baseline map內容 |
| additions／changes | 遍歷cur，以base.find與sib::ValuesEqual（a==b）建delta |
| removals | base中cur沒有的tag→sib::MakeNull；原unknown／not installed語意comment保存 |
| frame | delta.empty()用空string記「沒有變化」；否則ObjectFrom→EncodeTagObject，包type=patch／data |
| 每個peer | 空frame continue，不更新c.lastSent；非空SendJson後c.lastSent=cur，增patchesSent及patchBytes |
| 結尾統計 | statsMx下增pumpRuns、累加steady_clock微秒duration，pumpTags=cur->size；有conns但全被過濾仍會到這裡 |

patchBytes是JSON字串size，沒有在此加WS／TCP overhead；patchesSent發生在SendJson佇列階段，並不是peer收到或套用成功。
frames持有舊baseline的原address-reuse理由、20260926 CPU／0.7s／14 HMI／約6400tags與20260930計時comment完整保留，都是來源歷史記錄，不是本輪量測。
TagSnapshot.h所述UI單一publisher、private view與historical concurrent test原文[前](raw/source-06.md)／[後](raw/source-07.md)保留；TagSnapshot.cpp／全部publish caller與runtime未在本五body驗證。
空diff時baseline仍保留舊pointer、generation與read分開的效果，留給完整caller／publish續查，不宣稱有現場漏更新。
