# 原文：entire TagSnapshot.h including original threading rationale and historical test comments; no inline-body completion credit（2）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/TagSnapshot.h`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `029844dcc6b44288b46472764dd7d4de66ae5fb7add22771fccff5f22fe514a1`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->

    // OS id of the recorded publisher thread (0 until the first publish call).
    std::uint64_t publisherThreadId() const;

    // Non-copyable: this object IS the shared seam; copying one would silently
    // give a caller a second snapshot nobody publishes to.
    TagSnapshot(const TagSnapshot&) = delete;
    TagSnapshot& operator=(const TagSnapshot&) = delete;

private:
    struct Impl;   // holds the lock + both buffers; keeps windows.h out of here
    Impl* impl_;
};

} // namespace webbridge

#endif // WEBBRIDGE_TAGSNAPSHOT_H

<!-- preserved-content:end -->
```
