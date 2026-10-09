# 原文 13：TestJsonNumbers

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tests/test_wb_crypto.cpp`；以 `TestJsonNumbers`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `cc6c1a73d008d9047d4a23628065070230144b37167e97f78ffb877a162bf2e0`。
來源context；完成函式計數0，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
static void TestJsonNumbers() {
    // Integers are exact, including the 64-bit extremes (a tag like a lifetime
    // contact count is int64).
    CHECK_STR(JsonNumber(static_cast<wb_int64>(0)),   "0");
    CHECK_STR(JsonNumber(static_cast<wb_int64>(130)), "130");
    CHECK_STR(JsonNumber(static_cast<wb_int64>(-1)),  "-1");
    CHECK_STR(JsonNumber(static_cast<wb_int64>(9223372036854775807LL)),
              "9223372036854775807");
    CHECK_STR(JsonNumber(static_cast<wb_int64>(-9223372036854775807LL - 1LL)),
              "-9223372036854775808");

    // RFC 8259 s6: Infinity and NaN are NOT permitted in JSON. They must become
    // null (which the web side renders "---"), never "nan"/"inf".
    const double zero = 0.0;
    const double inf  = 1.0 / zero;
    const double nan  = zero / zero;
    CHECK_STR(JsonNumber(inf),  "null");
    CHECK_STR(JsonNumber(-inf), "null");
    CHECK_STR(JsonNumber(nan),  "null");
    CHECK_STR(JsonValue::MakeDouble(nan).ToJson(),  "null");
    CHECK_STR(JsonValue::MakeDouble(inf).ToJson(),  "null");
    CHECK_STR(JsonValue::MakeDouble(-inf).ToJson(), "null");
    {
        JsonWriter w;
        w.BeginObject().Key("temp.pv").Number(nan).EndObject();
        CHECK_STR(w.Str(), "{\"temp.pv\":null}");
        CHECK(w.Str().find("nan") == std::string::npos);
        CHECK(w.Str().find("inf") == std::string::npos);
        CHECK(w.Str().find("NaN") == std::string::npos);
    }

    // Finite doubles: representative temperature / position values, and the
    // requirement that an integral double is still a legal JSON number.
    CHECK_STR(JsonNumber(0.0),      "0");
    CHECK_STR(JsonNumber(130.0),    "130");
    CHECK_STR(JsonNumber(-40.5),    "-40.5");
    CHECK_STR(JsonNumber(11.0),     "11");
    CHECK_STR(JsonNumber(0.125),    "0.125");

    // Round-trip precision: a decimal that is not exactly representable must
    // still come back as the same double. 0.1 and 1/3 are the standard traps.
    {
        static const double kVals[] = {
            0.1, 0.2, 0.3, 1.0 / 3.0, 130.25, -40.125, 1234.5678,
            98.599999999999994, 2.2250738585072014e-308, 1.7976931348623157e308,
            -0.0, 1e-7, 123456789.123456789
        };
        const int n = static_cast<int>(sizeof(kVals) / sizeof(kVals[0]));
        for (int i = 0; i < n; ++i) {
            const std::string text = JsonNumber(kVals[i]);
            CHECK(text.find("nan") == std::string::npos);
            CHECK(text.find("inf") == std::string::npos);
            const double back = std::strtod(text.c_str(), 0);
            if (back != kVals[i]) {
                ++g_fail;
                std::printf("FAIL [%s:%d]  double did not round-trip: %.17g -> [%s] -> %.17g\n",
                            __FILE__, __LINE__, kVals[i], text.c_str(), back);
            }
            ++g_total;
            // Locale-independence: a ',' decimal separator would be invalid JSON.
            CHECK(text.find(',') == std::string::npos);
        }
    }
}

<!-- preserved-content:end -->
```
