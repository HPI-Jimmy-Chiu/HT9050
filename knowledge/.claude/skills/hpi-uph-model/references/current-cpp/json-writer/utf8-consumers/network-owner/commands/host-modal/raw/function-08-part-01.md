# 原文：W906_IsOutputCmd（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 14行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
static bool W906_IsOutputCmd(const std::string& c)
{
#ifdef WB_PUMP_1203_CONTROL
    //AI(W906-IOWEB-P25c) 20260925: + the three STOP commands (laptop review Q2-1): a stop must not wait
    //  behind a Poll either. They still obey the barrier rule in W906_ServiceOutputs, so a stop never
    //  overtakes a jog / move / home queued before it ("stop first, move after" is the dangerous order).
    return c == "io.btnPanelClick" || c == "pci1203.do.setBit" || c == "pci1203.do.setByte"
        || c == "pci1203.ax.stop" || c == "pci1203.ax.emgStop" || c == "motor.stop";
#else
    (void)c;
    return false;              // no command surface in this binary: nothing to run early
#endif
}


<!-- preserved-content:end -->
```
