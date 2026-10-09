# 原文 12：TestJsonUtf8Policy

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tests/test_wb_crypto.cpp`；以 `TestJsonUtf8Policy`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `af5436768d25dc05fd8c5f775942075351c0c845d543de57e5200c3b6aae3d6d`。
來源context；完成函式計數0，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
static void TestJsonUtf8Policy() {
    // Valid UTF-8 passes through byte-for-byte (RFC 8259 s8.1: JSON text is
    // UTF-8). "測試" = E6 B8 AC E8 A9 A6.
    const std::string utf8Ok = "\xE6\xB8\xAC\xE8\xA9\xA6";
    CHECK(IsValidUtf8(utf8Ok));
    CHECK_STR(JsonQuote(utf8Ok), "\"" + utf8Ok + "\"");

    // Legacy Big5/CP950 input: "測試" = B4 FA B8 D5. Not valid UTF-8, so the
    // documented policy (JsonWriter.h note 2b) transcodes it to the SAME UTF-8
    // bytes as above -- this is the real handler case.
    const std::string big5 = "\xB4\xFA\xB8\xD5";
    CHECK(!IsValidUtf8(big5));
#if defined(_WIN32)
    CHECK_STR(SanitizeToUtf8(big5), utf8Ok);
    CHECK_STR(JsonQuote(big5), "\"" + utf8Ok + "\"");
#endif

    // Whatever the path taken, output is ALWAYS valid UTF-8: a browser drops the
    // entire frame on one bad byte, so this is the invariant that matters.
    {
        // A lone continuation byte 0x80 is neither valid UTF-8 nor valid CP950.
        const std::string junk = std::string("ok\x80", 3) + std::string("\xFF", 1);
        const std::string clean = SanitizeToUtf8(junk);
        CHECK(IsValidUtf8(clean));
        CHECK(clean.find("ok") == 0);          // ASCII survives
        CHECK(clean.find("\xEF\xBF\xBD") != std::string::npos);  // U+FFFD used
        const std::string q = JsonQuote(junk);
        CHECK(IsValidUtf8(q));
    }
    // Every single byte value, alone, must sanitise to valid UTF-8.
    for (int i = 0; i < 256; ++i) {
        const std::string one(1, static_cast<char>(i));
        CHECK(IsValidUtf8(SanitizeToUtf8(one)));
        CHECK(IsValidUtf8(JsonQuote(one)));
    }

    // IsValidUtf8 must reject the classic malformed forms, or invalid bytes
    // would be waved through as "already UTF-8".
    CHECK(!IsValidUtf8("\xC0\xAF"));              // overlong '/'
    CHECK(!IsValidUtf8("\xE0\x80\xAF"));          // overlong, 3-byte
    CHECK(!IsValidUtf8("\xED\xA0\x80"));          // UTF-16 surrogate U+D800
    CHECK(!IsValidUtf8("\xF5\x80\x80\x80"));      // > U+10FFFF
    CHECK(!IsValidUtf8("\xE6\xB8"));              // truncated sequence
    CHECK(!IsValidUtf8("\x80"));                  // lone continuation
    CHECK(IsValidUtf8(""));
    CHECK(IsValidUtf8("plain ascii"));
    CHECK(IsValidUtf8("\xF4\x8F\xBF\xBF"));       // U+10FFFF, the max legal
}

<!-- preserved-content:end -->
```
