# 原文：TagValue and TagSnapshot sibling adapters; context only（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `d698642283c2bcfc3171ed94eacdb776e95ca070c7993074ef7799c402d6c31d`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// --- TagValue ---------------------------------------------------------------
static TagValue MakeNull()                        { return TagValue::makeNull(); }
static TagValue MakeBool(bool v)                  { return TagValue::makeBool(v); }
static TagValue MakeNumber(double v)              { return TagValue::makeDouble(v); }
static TagValue MakeString(const std::string& v)  { return TagValue::makeString(v); }
static bool ValuesEqual(const TagValue& a, const TagValue& b) { return a == b; }

// --- TagSnapshot ------------------------------------------------------------
static unsigned long long SnapGeneration(const TagSnapshot* s)
{
    // generation() and NOT read().generation: read() copies the entire tag map
    // to build its view, and this is called on every poll iteration purely to
    // decide whether anything changed. Copying ~270 tags to learn "no" is the
    // kind of waste that only shows up under load.
    return s ? static_cast<unsigned long long>(s->generation()) : 0;
}

static void SnapRead(const TagSnapshot* s, std::map<std::string, TagValue>& out)
{
    out.clear();
    if (s) out = s->read().tags;
}


<!-- preserved-content:end -->
```
