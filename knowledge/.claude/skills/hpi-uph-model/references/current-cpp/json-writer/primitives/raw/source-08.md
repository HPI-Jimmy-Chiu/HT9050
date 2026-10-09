# 原文 08：JsonNumber(double)

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `JsonNumber(double)`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `6f1b8c983aa82d36478482d7f842e9732b7c249c47f4f86c5fabb7a5dc07ae30`。
完整CPP函式，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
std::string JsonNumber(double v) {
    // NaN and +/-Inf are not representable in JSON. Emitting them makes the
    // browser throw on JSON.parse and lose the whole frame, so map to null --
    // which the web side already renders as "---" (unknown).
    if (v != v) {
        return "null";                       // NaN
    }
    // AI(W906-BA-Cyl13) 20260911: this test used to be
    //     if (v > 1.7976931348623157e308 || v < -1.7976931348623157e308)
    // i.e. "bigger than DBL_MAX spelled out in decimal, therefore infinite".
    // With x87 excess precision that is wrong: FLT_EVAL_METHOD == 2 evaluates the
    // LITERAL at 80-bit, where decimal 1.7976931348623157e308 sits just below the
    // true DBL_MAX (1.79769313486231570815e308), so DBL_MAX compares GREATER than
    // its own spelling and the largest finite reading was silently emitted as JSON
    // null -- which the web side renders as "---", i.e. a perfectly good value
    // shown as "unknown".
    //
    // ⚠ MEASURED ON BOTH TOOLCHAINS IN THIS TREE, because they disagree, and that
    // is the part worth writing down (probe: scratchpad/fltprobe2.cpp, a noinline
    // function taking `double v` by value, matching this call shape):
    //
    //                              old_test(DBL_MAX)   isinf(DBL_MAX)
    //   MinGW.org 6.3.0 (mingw32)          0                 0     agree
    //   MinGW-W64 16.2.0 (i686)            1                 0     <-- fires
    //
    // build.bat pins MinGW.org 6.3.0, so the ORACLE line never showed this. The
    // 16.2.0 toolchain does, and this tree just acquired build_x64.bat and
    // build_nonoracle.bat, which is exactly how it becomes reachable here.
    //
    // (A first probe of mine reported "does not reproduce" on both. That probe
    // declared `volatile double v`, which forces a double-precision memory
    // round-trip and destroys the excess precision being measured -- it answered
    // a different question. The instrument was the bug, not the claim.)
    //
    // Ask the classification predicate instead of inventing a threshold: isinf is
    // exact regardless of evaluation precision. Comparing against the DBL_MAX
    // macro would also work -- both sides then widen identically -- but it still
    // reads as a magnitude test for what is really a class test.
    if (std::isinf(v)) {
        return "null";                       // +/-Inf
    }

    // Shortest form that round-trips exactly. 15 digits covers essentially all
    // real temperature/position values; 16 and 17 are the exactness backstops.
    char buf[64];
    for (int prec = 15; prec <= 17; ++prec) {
        std::snprintf(buf, sizeof(buf), "%.*g", prec, v);
        std::string text(buf);
        ForceJsonDecimalPoint(&text);
        if (std::strtod(text.c_str(), 0) == v) {
            return text;
        }
    }
    std::snprintf(buf, sizeof(buf), "%.17g", v);
    std::string text(buf);
    ForceJsonDecimalPoint(&text);
    return text;
}


<!-- preserved-content:end -->
```
