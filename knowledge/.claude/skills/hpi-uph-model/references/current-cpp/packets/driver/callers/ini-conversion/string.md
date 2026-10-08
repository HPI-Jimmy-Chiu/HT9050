# Trim、str 與整數文字

定位 vclcompat/AnsiString.cpp 的 AnsiString::Trim／assignInt，AnsiString.h 的 AnsiString(int v)／str()；完整選定定義見 [manifest](source-manifest.json)。與 [ReadDateTime](../ini-typed-enumeration/typed.md) 及 [raw／trim](../ini-core/read.md) 分層讀取。

Trim 從 data_ 兩端逐 byte 掃描，static_cast<unsigned char> 後以 <= ' ' 判定（<=0x20），回傳 AnsiString(data_.substr(b,e-b))。條件包括邊緣 NUL，不把它套成 W906RdTrim 的 0x01..0x20；中間 NUL 不會因這個兩端操作消失。

str() 是 const 方法，直接回 const std::string& data_；StrToDateTime 的 s.Trim().str() 會複製到區域 std::string。這只確認所選路徑；其他 constructor、c_str、完整物件生命週期與 ABI 仍待續。

AnsiString(int v) 先 static_cast<long long>(v)，再呼叫 assignInt；後者用 std::to_string(v) 寫 data_。由此可靜態連上 WriteInteger(value) 與 WriteBool(value?1:0) 交 WriteString 的文字路徑；磁碟／memory 寫入與成功回報界線仍沿 [writer](../ini-core/write.md)／[store](../ini-writer-store/store.md)。

assignInt 關於 MinGW/MSVCRT／MSVC portable 的來源註解完整保存；本輪沒有執行標準函式庫、locale、例外或 BCB6 differential 測試，也沒有把寫出文字視為成功保存。

Trim 保留在內部 std::string 的嵌入 NUL 與後續 std::sscanf(str.c_str(),...) 的 C-string 終止要分開。ReadDateTime 的 IsEmpty 具體定義、其他字串建構／輸入容量仍未完整查證；非空 Trim 字串進 converter 的控制流見 [解析](parse.md)。
