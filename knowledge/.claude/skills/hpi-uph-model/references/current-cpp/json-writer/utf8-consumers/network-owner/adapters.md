# Adapter：server角色與ready訊息消耗

[上層](index.md)；[AcceptKey原文](raw/source-01.md)；[EncodeServerFrame原文](raw/source-02.md)；[TryDecode原文](raw/source-03.md)；[Frame original per-connection adapter context原文](raw/source-09.md)；[Decoder original per-connection adapter context and PendingBytes原文](raw/source-10.md)；[Conn original per-connection adapter context原文](raw/source-11.md)。

`AcceptKey(clientKey)`直接回`ComputeAcceptKey`；不在adapter驗key格式。
`EncodeServerFrame(opcode,payload)`直接交`EncodeFrame`，FIN=true、mask=false、maskKey=0。
這個wrapper固定server輸出角色；generic encoder的control／opcode／UTF8檢查界線沿用[encoder](../encoder/index.md)，不能替它添加保證。

每個Conn持有自己的`Decoder`；其`dec`建構requireMaskedInput=true、cap=64 KiB。
`ready`非空時，`TryDecode`先交付既有訊息，不先Feed新的`in`。
`ready`空時，若dec已Failed立即回-1；否則`in`非空就Feed，再清`in`，把got逐個排到ready。
Feed回false但已有got時先交完它們；ready耗盡後下一次才反映-1。Feed失敗前完成的訊息與錯誤不能混成全批丟棄。
Feed後仍無ready回0等待；未完成frame／fragment由WsDecoder持有，不能再由socket caller按Frame.consumed erase。

交付時取ready front，設opcode與payload、FIN=true、masked=true、consumed=0，再pop。
masked=true是此adapter寫的metadata；實際mask檢查由requireMaskedInput的decoder完成，不能把欄位當原始frame逐欄轉存。
FIN=true來自完整重組訊息契約。配合這條路徑，ProcessWsBytes中的Conn.fragment／fragmenting分支不會由本adapter產生；原碼與註解完整保留。
`PendingBytes`adapter合併dec.PendingBytes與dec.FragmentBytes；其量不是`c.in.size()`或ready中已交付完整訊息的總量。
完整decoder責任沿用[五完整函式](../decoder/index.md)，本單元不重計helper body。
