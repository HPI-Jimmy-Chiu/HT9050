// =============================================================================
//  WebBridgeRecipeBcb.h -- "what BCB6 actually reads" for one recipe / system ini value.
//
//  AI(W906-RECIPE-BCB-R13) 20260926: RULINGS_20260926 #13 (user 7A: the browser shows the value BCB6 read,
//  not the file text).  golden reads these documents with TIniFile (V912 common.cpp:331 `new TIniFile`,
//  :562 ReadString), and BCB6 TIniFile::ReadString is kernel32 GetPrivateProfileStringA with a 2048-byte
//  buffer.  The rules below are the ones vclcompat/IniFiles.cpp measured against kernel32 with ctypes
//  (W906RdTrim :587-593, W906RdLookup's quote strip :661, W906RdGetString's size-1 cut :667-680):
//    1. trim 0x01..0x20 from both ends (tab, CR, space ... -- NOT only ' ');
//    2. then, if what is left is >= 2 chars and starts and ends with the SAME quote (" or '), drop that pair
//       (inner blanks survive: `QS= " a " ` reads as ` a `);
//    3. cut to 2047 bytes.
//  Input is the store's `raw` (everything after the first '=' on the line, verbatim -- TIniStore, IniFiles.cpp:222).
//  The store picks the FIRST section of a name and the FIRST key in it, exactly like kernel32, so the pair
//  (store raw -> this function) equals GetPrivateProfileStringA for every key the store enumerates; ctest
//  WB_RecipeDoc group 14 checks that against the real kernel32 on randomised files.
//  Known difference (documented, not reachable in IniData today): a section header with two or more ']'
//  -- the store takes the LAST ']', kernel32 the first.
//  Header-only on purpose (no CMake edit): the one caller is WebBridgeRecipeDoc.cpp.
// =============================================================================
#ifndef HT9045_WEBBRIDGE_RECIPE_BCB_H
#define HT9045_WEBBRIDGE_RECIPE_BCB_H

#include <string>

namespace ht9045 {

inline std::string RecipeBcbValue(const std::string& raw)
{
    std::size_t b = 0, e = raw.size();
    while (b < e && (unsigned char)(raw[b] - 1) < 0x20u) ++b;            // 0x01..0x20
    while (e > b && (unsigned char)(raw[e - 1] - 1) < 0x20u) --e;
    if (e - b >= 2 && raw[b] == raw[e - 1] && (raw[b] == '"' || raw[b] == '\'')) { ++b; --e; }
    std::string v = raw.substr(b, e - b);
    if (v.size() > 2047) v.resize(2047);                                  // 2048-byte ReadString buffer
    return v;
}

}  // namespace ht9045

#endif  // HT9045_WEBBRIDGE_RECIPE_BCB_H
