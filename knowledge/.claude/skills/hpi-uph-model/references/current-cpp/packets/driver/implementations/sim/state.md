# Sim 的狀態與控制參數

定位 ctor、Recompute、ibfind／ibrsc／ibpad／ibtmo／ibwait／ibstop 與 header Status／Error／Count；完整原文見 [manifest](../source-manifest.json)。

| 定位 | 選讀來源行為 |
| --- | --- |
| ctor | FindFails／talk_為false，sta_／err_／cnt_／pad_／tmo_為0，容器依預設定義建立 |
| Recompute | 每次重設sta_=CMPL；in_非空加LACS，talk_為true加TACS；沒有重設err_或cnt_ |
| ibfind，FindFails為true | 設sta_=ERR、err_=0，返回-1；這分支沒有清cnt_ |
| ibfind，FindFails為false | Recompute，返回0 |
| ibrsc／ibstop | Recompute後返回sta_ |
| ibpad／ibtmo | 分別保存v到pad_／tmo_，Recompute後返回sta_ |
| ibwait | 兩參數在定義中都未命名，只Recompute後返回sta_；本body沒有等待／mask篩選 |
| Status／Error／Count | inline分別返回sta_／err_／cnt_ |

因此 CMPL 或重新計算 sta_ 不表示上一筆 err_／cnt_ 已歸零。這是選讀來源的狀態保留，不是 runtime 測試結果；pad_／tmo_ 如何被使用仍要查 caller。回 [Sim 索引](index.md)、[讀寫](read-write.md)。
