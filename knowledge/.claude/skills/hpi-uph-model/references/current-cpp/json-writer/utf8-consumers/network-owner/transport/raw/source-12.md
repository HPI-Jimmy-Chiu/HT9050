# 原文：NowMs complete helper context only; not counted as an eleventh completed function

[上層](../index.md)。固定來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `8cd0be50e14294b42774510c7fa8accc51b511d6`；SHA256 `025a619d41fcfe30f81db95b37466a18ea57e0d492ffb7e1bd89263e6b86c54b`。
保留原正文與comment；context完整函式計數0；未執行程式。

```cpp
<!-- preserved-content:start -->
unsigned long long NowMs()
{
    using namespace std::chrono;
    return static_cast<unsigned long long>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}


<!-- preserved-content:end -->
```
