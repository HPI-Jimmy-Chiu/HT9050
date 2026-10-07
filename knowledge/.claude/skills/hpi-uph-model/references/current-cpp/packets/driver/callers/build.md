# 建置旗標與ctest入口宣告

定位V906根CMakeLists.txt的W906_NO_SOFT_SIMULTE、ht9045_testercomm、ht9045_testercomm_handler及wb_serve；tests/CMakeLists.txt以三個test target／ctest名稱定位，原文見 [manifest](source-manifest.json)。

| 宣告 | 選讀來源 |
| --- | --- |
| W906_NO_SOFT_SIMULTE option | 預設OFF；ON時add_compile_definitions(W906_NO_SOFT_SIMULTE) |
| MachineType.h | #ifndef W906_NO_SOFT_SIMULTE才define SOFT_SIMULTE |
| ht9045_testercomm STATIC | 列GpibDriver／GpibEngine與其他GPIB、Hub／thread／mailbox、RS232來源 |
| ht9045_testercomm_handler STATIC | 列TesterCommWiring／Handler與TCP來源 |
| wb_serve library列表 | 含上述兩testercomm library與其他機台庫 |
| test_testercomm_ipc／TesterComm_IPC | 連ht9045_testercomm；使用TestEngine替身 |
| test_testercomm_gpib／TesterComm_GPIB | 連testercomm／globals／core／vclcompat；main會呼叫TestFullLifecycle，後者自己檢查opt-in |
| test_testercomm_handler／TesterComm_Handler | 連Handler與testercomm、RESCAN機台庫，WIN32另加ws2_32；main先圍堵，再包含E2E呼叫 |

這些是來源宣告，不是本機CMake cache、compile command、連結成功或ctest通過。SOFT_SIMULTE本身不等於InjectDriver(&sim)：本層兩個Sim注入body沒有以該宏包住注入；GpibEngine選擇還要依 [Start](../lifecycle/selection.md) 的實際指標分支。

沒有讀完完整target usage／preset／環境圍堵，也沒有執行configure、build、ctest或任何driver。回 [索引](index.md)、[測試](tests.md)、[界線](limits.md)。
