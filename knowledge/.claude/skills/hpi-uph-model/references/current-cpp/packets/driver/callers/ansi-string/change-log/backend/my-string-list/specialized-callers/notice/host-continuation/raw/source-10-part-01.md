# 原文 10／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `WaitTagScope_complete_struct`；種類 `regions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `7506e266263036603ba121d78a4ae4dcc3ed31f68d16e70bd233126d9cdc5a55`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
struct WaitTagScope {
    explicit WaitTagScope(const std::string& tag) : tag_(tag) { WaitingTags().push_back(tag_); }
    ~WaitTagScope()
    {
        std::vector<std::string>& v = WaitingTags();
        for (size_t i = v.size(); i-- > 0; ) if (v[i] == tag_) { v.erase(v.begin() + (long)i); break; }
    }
private:
    std::string tag_;
    WaitTagScope(const WaitTagScope&);
    WaitTagScope& operator=(const WaitTagScope&);
};

<!-- preserved-content:end -->
```
