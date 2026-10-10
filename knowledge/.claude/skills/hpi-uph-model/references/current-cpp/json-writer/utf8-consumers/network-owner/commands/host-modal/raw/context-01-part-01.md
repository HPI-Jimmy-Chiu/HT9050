# 原文：complete file prologue metadata; historical --dry example is not current launch guidance（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 65行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
//Steven 20260916
// ----------------------------------------------------------------------
// 新增二進位定長陣列的 struct 投影：GET /api/system/levelset 與 WS 指令
// system.levels.put。Status.Security 的 179 組 radio 存的是
// system\levelset.dat（LAST_LEVEL_SET，256 個小端 int32），
// 不是 config\Security_new.def——後者是 cAuthority.cpp 讀的表單 Enable/Disable 旗標，
// 與那 179 組沒有對應關係。原本的 SysFileTable 只有 ini 與 csv 兩種形狀，
// 且註解明文把二進位排除（「要等 C++ 端依 struct 逐欄位輸出投影」），
// 這次補的就是那個投影。細部見 AI(W906-FW-LEVELSET)。
// 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
// ----------------------------------------------------------------------

//Steven 20260916
// ----------------------------------------------------------------------
// recipe.doc.put 的 ack 補上 {changed, identical, notFound}（原本只傳 perr，
// 計數只 printf 到主控台）。瀏覽器需要 notFound 才能執行「對照表有錯就整頁拒寫」，
// 需要 changed 才能把即將寫入的內容給操作員確認。
// 搭配 WebBridge/WebBridgeServer.cpp 的 AckJson 修正一起看，那邊才是根因。
// 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
// ----------------------------------------------------------------------

//Steven 20260915
// ----------------------------------------------------------------------
// 新增 /api/system/（40 支機台設定檔，可讀寫）、/api/text/（6 個純文字記錄來源，唯讀）、
// WS 指令 system.file.put、--allow-system-write 旗標、ApiRoute() 單一路由分流器。
// 844 -> 1668 行。細部標記見各處 AI(W906-FW-SYSFILE / -TEXT / -DIO)。
// 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
// ----------------------------------------------------------------------

// =============================================================================
//  tools/wb_serve.cpp -- run the web HMI against the REAL handler data layer.
//
//  AI(W906-WebBridge) 20260806.
//
//  This is the WB-2 milestone in runnable form: load the machine config, stand
//  up the bridge, serve D:\HT9045\web, and publish real tag values to whatever
//  browser connects.
//
//      wb_serve.exe                 -> http://127.0.0.1:8045/?src=ws
//      wb_serve.exe 9000            -> different port
//      wb_serve.exe --root <dir>    -> serve a different web root
//      wb_serve.exe --seconds 20    -> exit after N seconds (for scripted runs)
//
//  WHAT IT IS NOT
//  Not the product. The handler proper will own this server on its own UI
//  thread once GA-3 lands the god-stack in HT9045.exe (see LoadMachineConfig in
//  database.h). This exe exists so the whole path can be exercised, and looked
//  at, before that.
//
//  SAFETY POSTURE -- inherited, not re-decided
//    * loopback only, and read-only: WebBridgeConfig defaults both that way
//      because this endpoint can eventually command machine motion
//      (web/docs/ARCHITECTURE.md section 6, questions 2 and 3). This file does
//      not widen either.
//    * LoadMachineConfig() SEEDS missing keys, i.e. it WRITES to asGeneralPath.
//      That is the real system\Gerneral.ini. Running this on a machine is
//      therefore the same class of act as starting the handler -- which is the
//      point, but it is stated here rather than discovered. --dry copies the
//      config to a scratch file first and leaves the real one alone.
//
//  WHAT THE BROWSER WILL SHOW
//  Mostly "---". That is correct, not broken: only ~8 of the tags have a source
//  that the port actually loads today. WebBridgeTags.h carries the measured
//  live/dead inventory and the reason for each.
// =============================================================================

<!-- preserved-content:end -->
```
