# byte 儲存、NUL 與 char* 邊界

[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)；[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)。完整選定函式與歷史註解保存在 [manifest](source-manifest.json)。

`data_` 是 std::string；本 shim 沒有實作 BCB AnsiString reference counting／code-page conversion。Length 是 `static_cast<int>(data_.size())` 的 **byte 數**，IsEmpty 看 std::string 是否零長，不能把 UTF-8／Big5 byte 長度當 Unicode 字數。

| 入口 | selected body 行為 |
| --- | --- |
| 預設／copy constructor | 預設空儲存；copy 複製 data_ |
| const char* | null 改成空字串；非 null 沿 NUL-terminated constructor |
| const char*＋int len | null 為空；非 null 且 len>0 使用明示 byte 長度，len≤0 長度0 |
| const std::string& | 複製完整 std::string，含 embedded NUL |
| char | 建立長度1的字串，即使 char 為 NUL |
| = AnsiString／char*／char | 分別複製 data_／沿 NUL-terminated文字（null空）／單byte |
| += AnsiString／char*／char | 分別 append完整 data_／非null文字（null不改）／單byte |

靜態例子：`AnsiString(char(0))` Length=1、IsEmpty=false，c_str 的第一 byte 卻為NUL；明示 `std::string("a\0b",3)` 可保存3 bytes，char* constructor只取前面的a。char*＋len要求來源緩衝區至少含指定bytes；正文沒有驗證buffer長度，不能把此入口用作安全截取任意指標。

`c_str()` 直接回 data_.c_str()；`str()` 是前單元已保存的 const std::string& 入口。它們均非擁有資料的獨立copy。依所有權／生命期推導，不能保留暫存 AnsiString 的 pointer／reference 跨過物件銷毀；修改原字串後也需重新取得，不承諾pointer穩定。本輪沒有測 ABI、配置失敗、self-alias char* assignment／append或跨執行緒。

`operator[]` 把 i−1轉size_t後直接索引，沒有範圍guard；合法caller需 `1≤i≤Length()`。可寫char&不表示UTF-8字元編輯：只改一byte可能破壞多byte序列。i−1與Length的int容量界線未檢查，本輪不宣稱超大字串安全。

回 [入口](index.md)、[搜尋／編輯](search-edit.md)、[界線](limits.md)。
