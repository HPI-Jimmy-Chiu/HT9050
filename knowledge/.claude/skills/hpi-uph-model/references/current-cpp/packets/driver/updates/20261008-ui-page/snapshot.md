# 快照欄位的來源

定位 `BuildUiSnapshot`、`S`／`Q`／`B`／`Num`／`Lines`／`Combo`／`Check`、`TSerialPoll::UpdateLed`；完整選定原文見 [manifest](source-manifest.json)。

`SerialPoll==0` 時builder回空字串。其餘路徑從該form複製資料；`up`與driverName是caller傳入，不能由文字推導硬體online。

| 欄位 | 來源與界線 |
| --- | --- |
| `ibsta`／`iberr` | 複製全域int；NI／Sim與wrapper刷新來源見 [位址版本](../20261008-online-pad/release.md)，不是本段的實機量測 |
| `cardAddress` | `LastPrimaryAddress()`；最後成功wrapper呼叫參數快取，-1是未記錄，沒有讀回硬體位址 |
| `leds` | 13個form ALed值及label caption；與raw欄位是兩份不同資料 |
| `status` | 迴圈複製`TStatusPanels::kCount`個Panel文字；其中文字不由本builder重新校驗位址 |
| `sites` | 依`MY_DUT_PAL.size()`序列化、空指標為null；header註解的32-site不是本段對所有機台的容量證明 |
| `logs`／`rs232Aux.MemoLog` | 一般tail常數200，mmoBINON與memoAlarmCode為100；不是完整歷史log |

`UpdateLed`中ALed8先寫`ibsta&0x20`再寫`&0x10`，ALed11先寫`&0x2`再寫`&0x1`；這個完整body沒有寫ALed12／13。不能以13燈的caption／on反推完整ibsta bits；此處保留原行為，沒有改程式或golden。

## 字串與控件

`S`由AnsiString.c_str建立std::string，`Q`交`JsonEscape`，`B`回true／false文字，`Num`以24-byte局部buffer格式化long。
`JsonEscape`保存引號、反斜線、換行／CR／tab及低於0x20的跳脫，其他byte照原值通過；locale／ANSI轉碼與完整AnsiString／TStrings實作仍待查。

`Lines`只取最後tail筆，空list回空array；combo的items tail為300。Combo／Check空指標回null，並複製enabled／visible等控件值。這些是資料形狀，不是瀏覽器或控制互鎖的驗證。

回 [入口](index.md)；HTTP讀取副本見 [channel](http.md)，browser是否使用欄位見 [瀏覽器](browser.md)。
