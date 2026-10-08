# 1-based 搜尋、slice 與原位編輯

[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)；[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)。完整選定函式與歷史註解保存在 [manifest](source-manifest.json)。

| function | 正文分支 |
| --- | --- |
| posImpl／Pos | 空sub回0；std::string::find缺席回0，命中offset+1 |
| Pos(char*) | null改空sub，結果0；char overload以長度1 std::string，可找NUL |
| AnsiPos | 直接forward Pos，沒有獨立multibyte walker |
| lastDelimImpl／LastDelimiter | 空delims回0；find_last_of找任一delimiter byte，命中offset+1 |
| SubString | start≤0改1；len≤0或start>Length回空；len限制到尾端可用byte數 |
| Delete | index<1／index>Length／count≤0不改；count限制到尾端 |
| Insert | index<1改1；index>n+1改n+1，插到尾端；使用s.data_完整bytes |
| SetLength | n<0改0，再data_.resize；增長由std::string補NUL bytes |

以上為所讀body加std::string呼叫語意的靜態推導，沒有執行C++／oracle。ASCII例子：`abc.Pos("b")`⇒2，`abc.SubString(0,2)`⇒ab；空sub⇒0。embedded NUL以AnsiString／std::string入口可參與搜尋，char*搜尋入口仍在第一NUL結束。

LastDelimiter是delimiter **byte集合**，不是任意Unicode字元集合；Big5 trail byte若恰是delimiter可能被當分隔符。AnsiPos的歷史註解已說單byte相同，但不能據此承諾BCB MBCS或UTF-8完整相容。SubString／Delete／Insert同樣按bytes，沒有保護code point邊界。

正常int容量以外，Length縮窄、p轉int再+1、i−1、n+1算術與分配例外都未guard。本單元沒有capacity／overflow／平台ABI實測，也沒有將self-insert或caller使用閉環宣稱為已驗證。

回 [入口](index.md)、[儲存](storage.md)、[比較](comparison-case.md)、[界線](limits.md)。
