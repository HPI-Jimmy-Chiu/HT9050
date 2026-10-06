# ART的活動caller、效果與機型分流

## 活動caller

- [v912 csystem.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/csystem.cpp)：DoAllProcess以 `LastSet.iRunStartMode==rsmAutoRetest || bART_needRT2` 呼叫DoAutoRetest；此函式未直接以Type_HT9050另包ART分支，但這不證明所有機台條件都相同。
- [v906-cpp csystem.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/csystem.cpp)：活動DoAllProcess在ProcessSensorScan後先檢查W906_Ht9050DryRunOn；成立就W906_Ht9050DryRunTick後return，尚未進入W906_DoAllProcessLadder。這是馬達試運轉分流，不能當成已退役的--dry或Web API dryRun。
- V906的W906_DoAllProcessLadder用同一ART模式／bART_needRT2條件呼叫DoAutoRetest(false)。舊DoAllProcess副本在字面#if 0，不能拿其搜尋命中當活動caller。DoHomeProcess與DoART_AfterCleanOut內另有DoAutoRetest(true)初始化呼叫；初始化不等於case 300已執行。
- 活動DoAllProcess的 `MachineTypeChoice==Type_HT9050` Shuttle分流位於ladder之後；不能把這段當成ART只供其他機型或HT9050必然執行的證據。完整上游排程、機台開關及安全互鎖仍須依場景查。
- [v906-cpp CMakeLists.txt](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/CMakeLists.txt)的ht9045_sm列入AutoRetest.cpp；這只證明靜態建置來源列表，本批沒有編譯／執行。

兩版[V912 DoAutoRetest](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/AutoRetest.cpp)／[V906 DoAutoRetest](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/AutoRetest.cpp)的bReset路徑初始化Task=1；正常流程先比MInArmX／MInArmY安全位置並可能MoveInArm2XYToWait，再比MOutArmY並可能MoveOutArmXY_ToFix_Tray_Full，移動未完成即return false。

## 清計數呼叫的完成界線

[v912 main.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/main.cpp)的TfMain::Clarn_Data與[v906-cpp JsonBridge/actions/MainClarnData.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/JsonBridge/actions/MainClarnData.cpp)的ClarnDataBody都受 `IniConfig.bA61DisableCleanMUBA` 提前返回限制。Tag 0以 `LastSet.iRunStartMode<=rsmContinuRetest ? 0 : 1` 決定iMode，再逐項看 `LastSet.bCTClear[iMode][ct*]` 才呼叫ClearCount；不能說「Tag 0必清全部」。兩版MachineType.h均定義rsmContinuRetest=2、rsmAutoRetest=11，若呼叫時仍為ART模式則使用iMode=1。

[v906-cpp forms/fMain.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/forms/fMain.cpp)的TfMain::Clarn_Data先累加W906_Clarn_DataCallCount，只有W906_ClarnDataBody非空才呼叫它。InstallClarnDataBody把hook指向ClarnDataBody，[v906-cpp tools/wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)有安裝呼叫。這是特定入口的靜態連接，不保證其他binary／測試入口也已安裝；CallCount增加不能當成計數已清除。本批不呼叫任何清計數函式。

## HANA／SetLotState界線

V906的TfMainHanaART::IsHanaArtAvailable在forms/fMain.cpp實作回false，所以DoAutoRetest中依賴這個facade為真的支路不能僅憑保留文字視為可用；同case的其他OR條件另判斷。

AutoRetest.cpp的W906ART_FMAIN_SETLOTSTATE現行宏會呼叫[TfMain::SetLotState](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/forms/fMain_SetLotState.cpp)；不是舊註解所稱空樁。該函式另按TestIF_File／TestIF的TCP_IP_MODE、GPIB_MODE與W906_TesterBridgeFound分流，HANA與Renesas特定區段保留字面#if 0 gate。這些caller／gate差異不能改寫成「V906沒有任何HANA功能」，也不能宣稱tester已收命令。
