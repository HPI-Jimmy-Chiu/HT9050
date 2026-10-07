# 正式初始化的factory與opt-out

定位 `TesterCommWiring.cpp` 的 `W906_TesterCommInit`、g_inited、HT9045_TESTERCOMM；完整選讀body見 [manifest](source-manifest.json)。

| 定位 | 選讀正式來源行為 |
| --- | --- |
| 入口 | 先W906_TesterConnectRulesInstall，再檢查g_inited；已初始化就返回 |
| 未初始化 | 先W906_CmdServersEnsure，再讀HT9045_TESTERCOMM；字串精確等於0才返回 |
| 通過opt-out | g_inited=true，取得TesterCommHub::Instance，再登錄factory |
| kTestTypeGpib／kTestTypeTcpIp | 都登錄GpibEngine::Create |
| kTestTypeRs232／kTestTypeTtl | 都登錄Rs232Engine::Create；其實作不在本單元 |
| 其後 | 建立或沿用fTesterSide、Attach、安裝forward／window座，初始化GPIB aux／TCP與RMS相關路徑 |

這個body沒有InjectDriver或機型／客戶factory分支，也沒有直接SelectTestType；實際選擇仍要追上層呼叫。登錄相同factory不等於TCP/IP封包本身只走NI，TCP pump另有初始化。

來源註解把opt-out描述為SIM回歸不啟動bridge，但W906_TesterConnectRulesInstall與W906_CmdServersEnsure在返回之前；本單元未查這兩個callee，不能因此保證完全無副作用或所有通訊都離線。

SIM注入與 [driver Start](../lifecycle/selection.md) 的NI選擇分開判讀；回 [索引](index.md)、[測試](tests.md)、[界線](limits.md)。
