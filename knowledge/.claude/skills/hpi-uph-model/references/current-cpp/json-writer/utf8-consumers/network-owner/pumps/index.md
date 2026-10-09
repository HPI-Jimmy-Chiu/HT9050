# WebSocket snapshot／outgoing與ACK

[socket owner上層](../index.md)；[transport](../transport/index.md)；[lifecycle](../lifecycle/index.md)。
V906固定來源`d95fc5fc504d4e2b93a68f754874a72cf1166471`：5完整CPP／132函式原文行，12原文頁含6 context；同一hpi-uph-model子樹，不新增canonical主題。

| 要查的責任 | 入口 |
|---|---|
| AckJson成功物件欄位拼接、SendAck／CompleteCommand | [ACK與ticket](ack.md) |
| generation、baseline pointer共用diff與patch計數 | [Snapshot](snapshot.md) |
| outQ_移交、connId過濾與失去收件連線 | [Outgoing](outgoing.md) |
| 原metadata／header、去重與版本／客戶／runtime界線 | [證據](evidence.md) |
| 5 body／6 context、62舊manifest與有限census | [Manifest](source-manifest.json)／[census](symbol-census.json) |

原正文：[AckJson](raw/source-01.md)、[SendAck](raw/source-02.md)、[PumpSnapshot](raw/source-03.md)、[PumpOutgoing](raw/source-04.md)、[CompleteCommand](raw/source-05.md)。
TagSnapshot.h原文[前段](raw/source-06.md)／[後段](raw/source-07.md)保留threading／歷史test comment，context與inline方法不計新函式。
ACK原metadata與歷史檔案定位[保留](raw/source-08.md)；[queue structs](raw/source-09.md)／[fields含Q30理由](raw/source-10.md)；[snapshot adapters](raw/source-11.md)／[JSON adapters](raw/source-12.md)。
既有Conn、SendJson／Enqueue／Flush與Wake不重算。剩命令驗證／owner、publish／browser／crypto、UPH容量與客戶版本、S8／846與legacy入口退役。
