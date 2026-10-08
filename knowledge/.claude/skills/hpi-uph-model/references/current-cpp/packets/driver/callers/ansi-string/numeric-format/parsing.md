# 整數與double解析

[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)；[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)。完整選定函式、模板與歷史裁決正文保存在 [manifest](source-manifest.json)。

`vc_parseIntBCB6` 先用已完成Trim複製字串，空字串false；以t.c_str()交給strtol，並用t.str()的開頭bytes選base。

| Trim後開頭 | 分支 | 成功檢查 |
| --- | --- | --- |
| $ | 至少2 bytes，strtol(p+1,16) | 至少讀到一字元、end指向NUL |
| 0x或0X | strtol(p,16) | 至少讀到一字元、end指向NUL |
| 其他 | strtol(p,10) | 至少讀到一字元、end指向NUL |

20260807歷史裁決保存EJ1N HexStrToInt加0x再呼叫StrToIntDef的原因；本輪沒有重新查該caller、INI parser一致性或913實作。shim只按Trim後前綴分流；例如負號在0x前的輸入不命中這裡的0x前綴條件，不能把所有signed hex語法當作已支援。

`ToInt` parse失敗時，Trim後空字串拋std::runtime_error的empty訊息，其餘拋not an integer；成功static_cast<int>(long)。`ToIntDef`只在parse返回false時用def，成功同樣轉int。正文沒有errno／ERANGE檢查，也沒有long→int範圍guard，因此不保證overflow會回def或拋錯；錯誤型別也沒有直接變成BCB的EConvertError。

`ToDouble` 同樣先Trim，空字串拋empty；strtod後以end==p或*end非NUL決定not a number，否則回double。沒有errno、finite或範圍檢查，也沒有固定locale；可解析格式、溢位結果與小數點取決於stdlib與當時locale，完整BCB相容尚未驗證。

**NUL界線（靜態推論）**：end檢查的是c_str指向的NUL-terminated序列，未比較end與完整std::string長度。明示長度的 `"12\0junk"` 中間NUL後仍有非空白bytes時，Trim保留中間NUL，strtol可在它停止且*end為NUL，使parser接受前段；strtod有同樣界線。來源「full string」註解原文保存，文件不將它擴張成embedded-NUL全byte驗證保證。本輪未執行此例。

回 [入口](index.md)、[printf族](printf-family.md)、[界線](limits.md)。
