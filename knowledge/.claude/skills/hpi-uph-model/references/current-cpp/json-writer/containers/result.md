# Ok、Str與Clear的契約分界

[上層](index.md)；[Ok原文](raw/source-06.md)、[Clear原文](raw/source-07.md)、
[Str inline原文](raw/source-08.md)；[RawValue](../../packets/driver/callers/ansi-string/change-log/backend/my-string-list/specialized-callers/notice/host-continuation/auth-callees/login-json/raw-value.md)。

`JsonWriter::Ok()` 回 `ok_ && stack_.empty() && !keyPending_`。
這三條件關乎已記錄錯誤、容器是否未關及pending key；正文不解析buf_、
不檢查RawValue的字串語法，也沒有root值數量檢查。不能把Ok稱為完整JSON有效性驗證。

`Str() const` 直接回 `const std::string&` 指向buf_，沒有先檢查Ok、複製字串或封存結果。
可從正文確定它是借用reference；caller如何複製／保存、writer生命週期及後續修改仍待查。
不要把Str存在當成caller已處理失敗或buffer已永遠固定的證據。

`Clear()` 清buf_與stack_，將needComma_及keyPending_設false、ok_設true。
這是本方法的重置動作；不能由方法存在推定publisher每次均有呼叫Clear，
也沒有在這三正文中驗證thread同步／重入或consumer完成讀取。
constructor與其餘value／quote／consumer實作不在本單元完成計數。
