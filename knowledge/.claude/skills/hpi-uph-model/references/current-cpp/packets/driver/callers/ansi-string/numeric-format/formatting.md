# 數值格式、多載與串接

[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)；[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)。完整選定函式、模板與歷史裁決正文保存在 [manifest](source-manifest.json)。

| 入口 | selected body |
| --- | --- |
| unsigned int／long constructor | 轉long long，呼叫已完成assignInt |
| long long constructor | assignInt(v) |
| unsigned long long constructor | assignUInt(v)，保留unsigned專用路徑 |
| double constructor | assignDouble(v) |
| = int／unsigned int／long／long long | assignInt路徑後return *this |
| = unsigned long／unsigned long long | assignUInt路徑後return *this |
| = double | assignDouble(v)後return *this |

`assignUInt` 用std::to_string的unsigned long long多載；20260826歷史裁決指出unsigned64超過signed64上限不能借道assignInt。本單元保存原裁決，沒有把當時GetECInformation golden行號當成新的913查證。

`assignDouble` 用64-byte buf與snprintf `%.15g`，再由NUL-terminated buf建std::string並指派data_；正文未檢查snprintf回傳值、沒有額外指數正規化或固定locale。歷史註解的US小數點與「faithful」是原作者主張，不能代替本輪對不同locale／ABI的實測。15是有效位數格式精度，不是固定小數位數，也不保證double精確round-trip。

數值assignment把數字轉成文字；`= char`是前單元的單byte路徑。不要把int多載與char多載混為同一語意。header有unsigned long assignment，沒有同名unsigned long constructor；平台long寬度與隱式多載選擇仍待ABI／實際caller核對，不從Windows路徑推論所有target一致。

9個free operator+均回傳新AnsiString：AnsiString／char*／char的5個多載先copy左值，再+=右值；int／double各左右2個多載先用constructor提升數字，再串接。沒有在本批宣稱unsigned／其他型別均有explicit +多載。兩側原物件不由所選body原位修改；char*與embedded NUL沿 [byte儲存界線](../bytes/storage.md)，數值串接沿上述格式路徑。

回 [入口](index.md)、[解析](parsing.md)、[界線](limits.md)。
