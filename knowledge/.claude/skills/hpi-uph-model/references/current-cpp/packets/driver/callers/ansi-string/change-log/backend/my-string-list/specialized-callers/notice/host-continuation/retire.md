# 退役成功對應哪一層

[上層](index.md)；[前段 handler 順序](../host.md)；[完整原文](source-manifest.json)。

`wb_serve.cpp::DialogMailboxRetire` 在目錄空時回 false；否則呼叫
`wb_dialog_mailbox.h::AlarmRetire`，失敗印出訊息，回傳該 bool。
原先 buffer 截斷、改 std::string 及舊測試註解保存，未重新執行其測試。

## 序號先增，兩份 writer 回 true 才清 slot

AlarmRetire 目錄空時不增序號；非空先 `++seqCounter`，組 AlarmIdleJson，
再 MailboxPut。回 false 時沒有執行 `slot=AlarmSlot()`，但序號已增。
因此失敗後 slot 還在，不代表 seqCounter 未變；後續重試會再增序號。
成功才把 slot 重設預設值。前段 NotifyAckHandle 在 retire false 時不進 close，
然而本檔未提供已替換外部檔案的回滾。

AlarmIdleJson 固定 state=idle、requestId空、buttons空、auth.required=false，
仍有 blocking=true、closePolicy=acknowledge-only。不能僅看 blocking 欄位
推定 idle 信箱仍有等待中的實體對話框。seq 經 AlarmU64 轉十進位文字。

## MailboxPut 是兩次單檔替換

先 AtomicWrite `<name>.json`，JSON尾端加 LF；再組兩行 CRLF 的 JS墊片，
AtomicWrite `js/<name>.js`。第一次 false 仍會嘗試第二次，
只在兩個 bool 不同時印「只寫成一半」，最終回 `okJson && okShim`。
沒有把兩個 path 放進同一交易，也沒有在其中一份成功時恢復舊內容。
原註解的 file: 只讀墊片與20260922端到端現象是歷史敘述，
本輪未重新驗證頁面啟動模式、外部 reader 或現場檔案。

## AtomicWrite 的 true 與落盤證據

寫 `<path>.tmp_wb`，fopen wb 失敗就 false。非空 body 呼叫 fwrite，
再 fflush、fclose；正文沒有檢查這三個結果。
接著最多20次 MoveFileExA(MOVEFILE_REPLACE_EXISTING)，成功即 true，
每次失敗 Sleep(50)，20次全敗才印訊息、remove暫存檔並 false。
重試次數與參數是靜態定位，不是本輪量到的延遲或時限保證。

所以這份 true 證明替換API回成功；不證明先前寫滿全部body、耐久保存，
或兩份外部reader已看到一致版本。沒有讀回或 byte hash 校驗。
fopen失敗與替換全敗是不同路徑；暫存檔名固定，完整並行 writer 條件未查。
這些是現行正文可見的限制，沒有重現磁碟錯誤或判定現場故障。
