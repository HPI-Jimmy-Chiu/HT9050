# 原文 09：existing_header_policy_comment_not_a_completed_function

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.h`；以 `existing_header_policy_comment_not_a_completed_function`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `fb3883d499a15e6e50f0a973ff833310592c6f81bc57af2dda8937ee396b4c69`。
來源context；完成函式計數0，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
// AI(W906-WebBridge) 20260805: minimal JSON *writer* for the web-bridge wire
// protocol (web/docs/ARCHITECTURE.md section 4). Parsing is NOT done here --
// inbound frames are parsed with the vendored cJSON (Public/cJSON.h).
//
// Serialises exactly the value kinds the tag contract uses: null, bool, 64-bit
// int, double, string, plus objects and arrays to nest them in.
//
// -------------------------------------------------------------------------
// Two behaviours are load-bearing for the browser. Both are covered by
// tests/test_wb_crypto.cpp; do not "simplify" either one.
// -------------------------------------------------------------------------
//
// 1. null and "" are DIFFERENT values.
//      Null()      ->  null      web side renders "---"  (unknown/not installed)
//      String("")  ->  ""        web side renders blank  (deliberately empty)
//    They are not interchangeable (ARCHITECTURE.md section 4 rule 3, and
//    web/README.md "Formats": Number(null) is 0, so collapsing them prints
//    "0.00" for every uninstalled heater zone).
//
// 2. Every emitted string is well-formed JSON *and* well-formed UTF-8.
//    A browser drops the ENTIRE frame on a single invalid byte, so one bad
//    legacy string would blank the whole screen rather than one field.
//
//    Escaping: the JSON-required escapes (" \ \b \f \n \r \t), all other
//    C0 control characters as \u00XX, and DEL(0x7F) left as-is (legal JSON).
//    '/' is NOT escaped (legal, and escaping it is noise).
//
//    NON-UTF-8 INPUT -- the documented policy, applied in this order:
//      a. If the whole string is already valid UTF-8, it is passed through
//         unchanged (only the escapes above are applied).
//      b. Otherwise the string is assumed to be legacy Big5 / CP950 -- which
//         is what the V899 sources and machine .ini/.csv files actually hold --
//         and is transcoded CP950 -> UTF-8. This is the common real case:
//         handler text such as Big5 B4 FA B8 D5 becomes U+6E2C U+8A66.
//         (Windows only; MultiByteToWideChar with codepage 950.)
//      c. If that transcode also fails (mixed/corrupt bytes, or non-Windows
//         build), each byte that is not valid UTF-8 on its own is replaced with
//         U+FFFD REPLACEMENT CHARACTER. ASCII bytes always survive.
//    The result is that invalid input degrades to visible replacement glyphs in
//    one field, and never to a dropped frame.
//
// 3. Doubles never serialise as "nan"/"inf"/"-inf" (those are NOT valid JSON and
//    would also drop the frame) -- non-finite values are emitted as null, which
//    the web side already renders as "---" i.e. "unknown". Finite values are
//    written with the shortest decimal form that round-trips exactly through
//    strtod, so temperatures and positions survive the trip. Formatting is
//    locale-independent: a locale whose decimal point is ',' is corrected.
//
// Layer rule: WebBridge/ must stay independent of VCL / vclcompat. Standard
// headers only (plus <windows.h> inside the .cpp for the CP950 transcode).

<!-- preserved-content:end -->
```
