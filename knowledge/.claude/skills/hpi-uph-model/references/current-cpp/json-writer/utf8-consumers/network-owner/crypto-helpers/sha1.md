# SHA-1狀態與digest byte口徑

[上層](index.md)；[完整CPP](raw/source-01.md)／[完整header與原用途限制](raw/source-02.md)。

## 建構、Reset與ProcessBlock

[Sha1::Sha1](raw/source-01.md#constructor)呼叫[Reset](raw/source-01.md#reset)，重新寫五個h_初值、bufLen_=0、bitCount_=0、finalized_=false並清digest_。
[ProcessBlock](raw/source-01.md#processblock)由64 bytes讀16個big-endian word，擴成w[80]，四段f／k與Rol更新a至e，再累加h_。
Rol是匿名namespace inline helper，完整保存在同頁但completion credit0；body使用unsigned int與32-bit rotate式，不在此輪確認編譯器寬度或測試向量。
原RFC 3174 method 1／section 6.1／初值comment逐字保存，這是來源標註，不是本輪獨立協定符合性檢查。

## Update兩overload

[byte overload](raw/source-01.md#update-bytes)在finalized_、data==0或len==0直接返回；忽略不以錯誤回傳通知caller。
先以len*8加入unsigned long long bitCount_，補既有buf_，滿64就ProcessBlock，再逐64處理輸入，剩餘byte放buf_。
data非空但len超出實際buffer時，本段沒有驗容量；不能把interface誤讀為自動驗證pointer／長度。
[string overload](raw/source-01.md#update-string)空字串返回，否則以data.data()與size()交byte overload；用長度讀，含NUL仍屬輸入byte。
單一Sha1物件可分段Update，但沒有mutex或跨thread同物件同步；bitCount_上限與超長輸入未實測。

## Final、Sha1Raw、ToHex與Sha1Hex

[Final](raw/source-01.md#final)若finalized_已設就返回cached digest_；首次取原bitCount_，append 0x80、補零到56 mod64，再放8-byte big-endian原長度。
padding直接寫buf_並ProcessBlock，不走Update，因此原comment要求「message length不含padding」得以保留；尾端block可能多於一份。
完成後bufLen_=0，digest_依kSha1DigestBytes=20寫五word的big-endian bytes並設finalized_=true；之後Update被忽略，Reset才開新一輪。
header明說std::string在此為byte container，可能含NUL及非UTF-8；不要經JSON字串或UTF-8文字路徑直接當digest文字。
[Sha1Raw](raw/source-01.md#sha1raw)新物件→Update→Final；[ToHex](raw/source-01.md#tohex)對任意byte string輸出每byte兩個小寫hex。
[Sha1Hex](raw/source-01.md#sha1hex)是ToHex(Sha1Raw(data))，按本來源20-byte口徑為40字元；握手adapter要求RAW，不能換成此hex結果。
原header限制僅供WebSocket Accept計算、不可作需collision resistance用途，完整保存；不是新增認證／簽章功能或安全評測。
