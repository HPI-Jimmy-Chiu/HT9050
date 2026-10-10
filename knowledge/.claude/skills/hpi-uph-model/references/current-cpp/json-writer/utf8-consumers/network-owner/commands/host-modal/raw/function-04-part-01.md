# 原文：W906_ServiceOutputs（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 50行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
void W906_ServiceOutputs(int phase)
{
    static bool busy = false;
    if (busy || g_pumpQueue == 0 || g_modalServer == 0) return;
    if (!g_carryRunnable && g_pumpQueue->size() == 0) return;          // the common case: one lock, nothing to do
    busy = true;
    //AI(W906-IOWEB-P25c) 20260925: carry cap (laptop review Q2-3). If the carry already holds a command
    //  that stops the scan (not an output, not one of the four inert ones), nothing behind it may run
    //  early anyway -- so leave the rest in the BOUNDED queue (64; "command queue full" keeps its
    //  meaning) instead of moving it into this unbounded deque.
    bool barrier = false;
    for (std::size_t b = 0; b < g_carry.size() && !barrier; ++b)
        barrier = !W906_IsOutputCmd(g_carry[b].cmd) && !W906_IsInertCmd(g_carry[b].cmd);
    if (!barrier) {
        std::vector<webbridge::WebCommand> fresh;
        g_pumpQueue->drain(fresh);
        for (std::size_t k = 0; k < fresh.size(); ++k) g_carry.push_back(fresh[k]);
    }
    g_carryRunnable = false;
    bool ran = false;
    for (;;) {
        //  Re-scanned from the front every time: a dispatch may itself touch g_carry
        //  (a blocking alarm inside it would run the modal wait, which takes g_carry).
        std::size_t k = 0;
        while (k < g_carry.size() && !W906_IsOutputCmd(g_carry[k].cmd) && W906_IsInertCmd(g_carry[k].cmd)) ++k;
        if (k >= g_carry.size() || !W906_IsOutputCmd(g_carry[k].cmd)) break;
        const webbridge::WebCommand wc = g_carry[k];
        g_carry.erase(g_carry.begin() + (long)k);  { extern bool W906_Q44CmdRefused(const std::string&, const std::string&, std::string*); std::string q44Why; if (W906_Q44CmdRefused(wc.cmd, (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(), &q44Why)) { g_modalServer->CompleteCommand((unsigned long long)wc.id, false, q44Why); continue; } }   //AI(W906-D012) 20260929 [W906] Q44 A2: same allow-list as the main dispatch head (this output-first path bypasses it): while closing io.btnPanelClick / pci1203.do.* are refused here, the stops still run
        if (!ran && phase > 0) W906_IoTiming(phase + 1);   // close the phase this interrupts (click log)
        ran = true;
        WdMark2(phase == 3 ? "1203 Poll > dispatch(output-first): " : "dispatch(output-first): ", wc.cmd.c_str());
#ifdef WB_PUMP_1203_CONTROL
        if (wc.cmd == "io.btnPanelClick") W906_DispatchIoClick(*g_modalServer, wc);
        else if (wc.cmd == "motor.stop") {              // stop-only by construction (stopOnly = true)
            std::string msAck;
            const bool msOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(),
                                                   (long long)wc.id, msAck, true);
            g_modalServer->CompleteCommand((unsigned long long)wc.id, msOk, msAck);
        }
        else                              W906_Dispatch1203Ex(*g_modalServer, wc, true);
#endif
    }
    if (ran) {
        g_outputsServed = true;
        if (phase > 0) W906_IoTiming(phase);               // and reopen it
        WdMark(phase == 3 ? "1203 Poll" : "tick");
    }
    busy = false;
}


<!-- preserved-content:end -->
```
