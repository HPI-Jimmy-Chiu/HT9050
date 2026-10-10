# 原文：output-first rationale, macro and extern prerequisites（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 29行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
//  AI(W906-IOWEB-P25) 20260925: OUTPUT FIRST -- the output commands' dispatch, and the
//  service that runs them ahead of everything else on the tick thread.
//
//  User 20260925 (real HT9050): 「輸出一定是要0延遲 這是工業機台 不能延遲的 讀取IO狀態是次要的
//  輸出一定要第一優先發出去 所以你迴圈價購必須更改」, then 「IO 輸出還是有延遲 我希望可以做到幾乎是瞬發」.
//  The click log (runcfg/logs/io_click_timing.csv) put the wait in three places: the 1203 Poll
//  (up to 5221 ms, its configuration sweeps -- removed in Pci1203Monitor.cpp), the api cache
//  rebuild (20-160 ms, now after the drain) and the Sleep(2) slices (now an event wait).
//
//  W906_ServiceOutputs() runs ONLY the output commands -- io.btnPanelClick, pci1203.do.setBit,
//  pci1203.do.setByte -- and is called
//    * right after the loop's sleep, and right after PumpTick,
//    * from INSIDE TPci1203Monitor::Poll(), between two stations / axes / DI bytes / DO bytes /
//      SDO reads (the monitor's yield hook -- W906_OutputYieldHook), and
//    * between the api cache's JSON builds, and between blocks of the tag publish (AI(W906-LAT-1) 20260925, W906_PublishYieldHook).
//  All on this one tick thread: the vendor API stays single-threaded (Pci1203Control.h), and
//  PumpTick never runs inside a Poll, so the engine globals the click touches are not in use.
//
//  ORDER: the queue is drained into g_carry; an output runs if every command before it in
//  g_carry is also an output or one of four that change no machine state (sys.ping, log.event,
//  ui.windows.put, cfg.resync -- these are left in place, not run). The first other command
//  stops the scan: it and everything after it wait for the main drain, in arrival order. So an
//  output never overtakes a start / stop / rescan / axis command, and outputs never reorder
//  among themselves. ⚠ pci1203.card.rescan (Close + Open under Poll's feet) is never run here.
// ===========================================================================
extern bool W906_IoBtnPanelClick(const std::string&, int, std::string&, unsigned long long);   // JsonBridge/IoBtnPanelClick.cpp
extern void W906_IoTiming(int);                                                                // same file: click-latency phases
extern bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool);          // WebMotorAccessLive.cpp (motor.stop = stop-only)


<!-- preserved-content:end -->
```
