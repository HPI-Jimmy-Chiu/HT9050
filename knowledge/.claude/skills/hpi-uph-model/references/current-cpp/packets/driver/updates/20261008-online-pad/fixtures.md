# 現有斷言與未執行界線

定位 `tests/test_testercomm_gpib.cpp` 的 `TestDriverWrappers`／`TestFullLifecycle`；兩個完整定義含註解保存於 [manifest](source-manifest.json)。本輪只讀文字，未編譯或執行。

| 來源斷言 | 可說的範圍 |
| --- | --- |
| 無driver ibonl返回含ERR | fixture存在這個斷言；沒有本輪測試結果 |
| Sim ibpad(5)後LastPrimaryAddress==5 | 覆蓋成功呼叫參數記錄；非硬體讀回 |
| LastOnline初值-1、ibonl(0,0)後0 | 覆蓋Sim記錄值；非NI board釋放驗證 |
| 清driver後LastPrimaryAddress==-1；無driver ibpad(9)後仍-1 | 覆蓋reset與無driver失敗路徑 |
| OneLife第1次後LastOnline==0，再執行第2次 | 只在full lifecycle條件被打開且實際執行時能觀察這些斷言；此輪沒有執行 |

`SimGpibDriver::ibpad` 設pad_後Recompute；Recompute設CMPL及LACS／TACS，不因FindFails設ERR。fixture所示失敗位址路徑是**無driver**，不能冒稱已覆蓋「有driver但ibpad失敗」或optional NI export缺失。

`TestFullLifecycle` 先要求 `HT9045_GPIB_FULL_TEST` 精確等於"1"，否則印skip並return。原文明載可能寫D:\GPIBLOG；此輪未設定該環境變數、未執行fixture、未改INI或開COM port。
選定函式內寫9045GPIB／HT-9045與CUSTOMER_CODE=910的scratch fixture，注入Sim，兩次OneLife後檢查thread/errors再清inject/path。其註解的aux RS232/golden說明保存為歷史；並非本輪HT9050、其他客戶或真機測試。

回 [版本入口](index.md) 與 [版本界線](versions.md)。
