# Sim注入與替身factory的選讀測試

以下均為測試原始碼中的安排與斷言，這次沒有執行，也沒有新增測試。完整body見 [manifest](source-manifest.json)，用檔名＋function辨認同名main。

| 檔／function | 已讀來源 | 不能擴張的結論 |
| --- | --- | --- |
| test_testercomm_gpib.cpp／TestDriverWrappers | 局部Sim，SetGpibDriver連接後測FULLSITES?、LACS／TACS／END、寫入與SRQ、FindFails，結尾SetGpibDriver(0) | wrapper的測試斷言不是NI DLL／實際Tester收件證據 |
| 同檔／TestFullLifecycle | HT9045_GPIB_FULL_TEST精確等於1才往下；建立fixture，OverrideIniPaths、局部Sim、InjectDriver(&sim)，登錄GpibEngine，呼叫OneLife兩次，結尾InjectDriver(0)、清override與scratch | opt-in與來源的正常結束順序不證明所有例外／全域還原／跨thread物件壽命 |
| 同檔／OneLife | 選GPIB、等版本／mode與UI、送CloseGpib、等待down，hub.Shutdown，斷言bridge token清除；SOFT_SIMULTE分支讀W906_SIM_TEST_BIN | 沒有執行；不能拿預期版本或bin當實際量測 |
| test_testercomm_handler.cpp／TestEndToEnd | HT9045_TESTERCOMM_E2E精確等於1才往下；局部Sim與hub／side，開始bridge、送mode與close，正常路徑先hub.Shutdown，後清InjectDriver／override | 沒有證明全部global均恢復；其他callee與異常路徑尚待查 |
| 同檔／main | W906TestInsideCtestRoots不通過就返回2，通過後才呼叫各測試（包含TestEndToEnd） | 圍堵helper與完整CMake環境值未在本單元驗證 |
| test_testercomm_ipc.cpp／MakeGpib及main | MakeGpib建立TestEngine("test-gpib")替身；main登錄替身GPIB／RS232，寫同步、nested、timeout、exception、switch／shutdown斷言 | 不是GpibEngine::Create、NI或SimGpibDriver整合測試 |

兩個注入fixture都寫Model=9045GPIB、Handler Model=HT-9045及CUSTOMER_CODE=910；TestEndToEnd先設MachineTypeChoice=Type_HT9045，局部切Type_HT9050檢查FindBridgeWindow相關斷言，再恢復該局部保存值。不能把這個切換當成9050 Model配置、容量／site或完整9050流程的驗證。

manifest另保存pin上的tracked V906 cpp／h InjectDriver文字普查：除了宣告／定義，四個呼叫語句在上述兩測試（各注入、各清0）；未追未追蹤／產生檔、其他樹、間接呼叫或執行時狀態，所以不是全runtime注入契約結案。

回 [索引](index.md)、[factory](factory.md)、[建置](build.md)、[界線](limits.md)。
