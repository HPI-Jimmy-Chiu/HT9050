# Frame byte組成與wrapper界線

[上層](index.md)；[header原契約與宣告](../decoder/raw/source-06.md)。

| 完整正文 | 產生的byte／條件 |
|---|---|
| [AppendLength](raw/source-01.md) | n<126用7-bit；126..65535用126＋2 byte；更大用127＋8 byte，逐shift big-endian。 |
| [EncodeFrame](raw/source-02.md) | reserve(payload.size()+14)，組FIN與opcode低4 bit，RSV零，寫length與可選mask key。 |
| [EncodeMaskedFrame](raw/source-03.md) | 把mask強制true；opcode、payload、fin與maskKey交通用encoder。 |
| [EncodeText](raw/source-04.md) | kWsText、FIN=true、mask=false；payload原樣交EncodeFrame。 |
| [EncodeBinary](raw/source-05.md) | kWsBinary、FIN=true、mask=false；同樣按std::string長度產生byte。 |
| [EncodePing](raw/source-06.md) | 複製payload；超125直接resize；kWsPing、FIN=true、mask=false。 |
| [EncodePong](raw/source-07.md) | 同樣截125 byte；kWsPong、FIN=true、mask=false。 |

header的EncodeFrame預設fin=true、mask=false、maskKey=0；MaskedFrame的fin預設true。
這些是宣告預設，實際角色要查caller；body不自行辨別client／server或強制方向。
mask=true把指定uint32 key逐shift寫成四byte big-endian，payload每byte XOR k[i & 3]；
沒有產生隨機maskKey。mask=false直接f.append(payload)，保留std::string內部NUL。

EncodeFrame只以opcode & 0x0F組首byte，不拒絕reserved opcode，也不檢查control的FIN／125上限。
它不套decoder的maxMessage_、不驗text UTF8、不管理continuation state、不發socket。
因此不能將Ping／Pong wrapper的截短保證套給generic或MaskedFrame；
也不能因函式叫EncodeText就推定其輸入已有效UTF8。decoder的拒絕條件見[完整解析](../decoder/frames.md)。

AppendLength的static helper接maskBit後直接OR；選定caller傳0或80，沒有在helper驗其他bit。
64-bit長度從size_t轉uint64後輸出，沒有另拒絕MSB；reserve大小加14也沒有獨立overflow／例外處理。
本輪不由此推定部署size_t寬度、極大輸入配置成功或全平台安全。
三種最短長度的選擇是此helper的條件，未重跑互通或網路測試。
