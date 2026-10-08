# 比較、大小寫與 TrimLeft

[AnsiString.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.h)；[AnsiString.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/6ce3c86225d661c67efa3da11d562722dc58efc9/HT9011UC_Cpp_V3.33.906.0/vclcompat/AnsiString.cpp)。完整選定函式與歷史註解保存在 [manifest](source-manifest.json)。

`operator==／!=` 的AnsiString兩端直接比較a.str()與b.str()；對char*兩種方向都把null換成空字串。`<／>／<=／>=`同樣直接使用std::string ordering，沒有呼叫locale collator、LowerCase／UpperCase或Trim。

因此一般`A`與`a`不等；內含NUL的完整data_參與AnsiString對AnsiString比較，char* literal／pointer端只表示NUL-terminated文字。`AnsiString(char(0))`與空字串不等，也不等於null char*。這些是正文分支的靜態例子；順序並非人類語言排序或BCB所有locale排序的證明。

[TStrings IndexOf](../../string-list/core/storage.md) 先前保存的線性`==`由此連到byte文字相等，第一個命中回index。TStringList Sort全部caller、CaseSensitive／Sorted等完整BCB模型仍待另讀，不能由header「used by Sort」註解擴大完成範圍。

UpperCase／LowerCase先copy data_，逐byte轉unsigned char再交`std::toupper／std::tolower`，結果cast回char，回新AnsiString。unsigned char轉換避開負char直接交ctype，但不是Unicode／Big5字元辨識；正文沒有固定C locale或多byte重組。有效locale、非ASCIIbytes及完整caller效果需另查，不把歷史「bytes pass through」註解概括為所有case操作都不變。

TrimLeft從前端移除unsigned byte≤0x20，回剩餘substring；後端不修剪。與已完成 [Trim](../../ini-conversion/string.md) 的兩端操作分清，包含邊緣NUL，不能套為INI別種trim規則。原物件不改，返回新字串；NUL中段不會因只掃前端而消失。

numeric conversion／printf backend、全程序setlocale、排序caller及實際BCB oracle都不屬於本單元完成項。

回 [入口](index.md)、[界線](limits.md)。
