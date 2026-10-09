# 原文 09／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `AlarmU64`；種類 `complete_header_inline_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `7f9d991f370edd498e791f9f3f878424682e0cd3fd20c0fd4a65717be4d968b0`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
inline std::string AlarmU64(unsigned long long v)             // decimal by hand: MinGW 6.3's -Wformat does not know %llu (msvcrt)
{
    char b[24];
    int i = 23;
    b[i] = '\0';
    do { b[--i] = (char)('0' + (int)(v % 10u)); v /= 10u; } while (v != 0 && i > 0);
    return std::string(b + i);
}

<!-- preserved-content:end -->
```
