# TryOneFrame的檢查及派送順序

[上層](index.md)；[211行完整正文](raw/source-05.md)；[UTF8 validator](../raw/source-03.md)。

| 階段 | 固定body條件 |
|---|---|
| header不足 | 少於2 byte回0；126形式少於4、127形式少於10也回0。 |
| 長度 | 7-bit／16-bit／64-bit big-endian；127最高bit為1→1002。masked令hdr加4。 |
| header規則 | RSV非0、reserved opcode、control非FIN／>125、mask與requireMask_不符→1002。 |
| 分片順序 | 非control continuation需fragOpcode_非0；未完成分片時不得開新text／binary→1002。 |
| message上限 | 非control比較base+payLen與maxMessage_；continuation base為frag_.size()，其餘0；超限→1009。 |
| 等待payload | buf_.size()不足hdr+payLen回0；完整才copy、用四byte key XOR，然後erase frame。 |
| control | 立即產生m；close另解析status／reason；不改frag_重組狀態。 |
| data | !fin開或續frag_但不輸出；最後FIN才交完整m；text在此驗UTF8，無效→1007。 |

沒有最短長度編碼檢查：126／127承載較短長度在此body不另拒絕。
原文64-bit比較註解針對32-bit防wrap；不可推成任何架構／任意maxMessage_皆無加法overflow，
此body未對base+payLen另外檢查溢位。協定完整符合性和部署上限仍待其他證據。

control可穿插於分片間；它不走data重組上限，而走125 byte限制。
mask key只在完整frame等待通過後讀，payload先複製並unmask，buf_.erase在派送與UTF8檢查前。
因此派送失敗可能已消費該frame；Fail沒有自動清buf_／frag_，後續Feed直接回false直到Reset。

close先把sawClose_設true，再檢查payload：0 byte允許無status、1 byte→1002；
至少2 byte時parse big-endian code，只接受1000..1003、1007..1011、3000..4999；
reason在validateUtf8_開啟時驗證。無效close也可能已有SawClose=true，旗標不是close有效性證明。
通過後close m交*out並回1，Feed會繼續drain；傳回、回應close與socket生命週期屬caller責任。

continuation先frag_+=payload，FIN時m取得重組內容後先清frag_／fragOpcode_，再驗whole text。
binary不驗UTF8；停用validateUtf8_也跳過text／reason驗證；分片途中不逐chunk判UTF8。
本body使用WsDecoder static member；constructor／validator既有原文沿用，不重計完成。
規則及1002／1007／1009是本pin條件結果，未執行協定或機台驗證。
