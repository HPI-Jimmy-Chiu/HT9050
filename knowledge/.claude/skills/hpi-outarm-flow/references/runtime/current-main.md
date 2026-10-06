# OutArm目前main與原文界線

20261006只讀核對main `372b91908`，未編譯或執行機台。來源定位使用function／關鍵變數，保存件行號僅供歷史查找。

- `aoutarm.cpp::DoOutArm`先清FRCarryKit／BRCarryKit的HAS_NULL_IC，對HAS_IC／HAS_HOT_IC設測試Bin資料，再`SetFixTrayMiddleDtata`、`DoOutArm_9045`；資料更新不等於真實取料完成。
- `aoutarm9045.cpp::DoOutArm_9045`先看TestingStop、servo-off、Destroy、QA、pause、reset、auto-alignment、laser；`USE_PICKER_COUNT==ep1Picker`優先走`DoOutArm_9045_All_1Picker`，再按`iInArmType`分派。
- 此dispatcher的auto-alignment區仍有#if0；DualSiteUseOneSuck分支callee也在#if0且有not translated訊息，`e9045_1x4_4_Back`分支空白。不能把可見分派表當全部機型已接入。
- `aoutarm9045_All_1Picker.cpp::DoOutArm_9045_All_1Picker`用`int &Task=OutArmTask`；`DoPickFromShuttle_9045_All_1Pick`取料後才到搜盤／`DoOutArmPlaceToAuto_9045`／後處理。原文說所有模式對稱仍須核對各callee與gate。
- `MoveOutArmToShuttleIncludeZ_9045_All_1Pick`依`W906_Ht9050OrgHome(MTrayX)!=-2`設`g_W906OutArmRule10=2`包住continuous move；放料`aoutarm9045.cpp`對`iOutPutTray==eAuto1`設同旗標1，之後清0。SmallY條件與Y伺服是不同裝置，旗標存在不代表實機設定或每個盤型都已驗證。
- `aoutarm.cpp::MoveOutArmXY_ToFix_Tray_Full`有NULL driver防護、servo-off、E90／TrayArm避碰與客戶分流；`bMoveY`與`IniConfig.bE90_OutArmFixFullExtraY`須分開。HT9050沒有Fix是硬體裁決，不能只因generic函式仍在就說HT9050有Fix。

## 查證來源

- [aoutarm.cpp](../../../../../HT9011UC_Cpp_V3.33.906.0/aoutarm.cpp)：DoOutArm／MoveOutArmXY_ToFix_Tray_Full／bMoveY。
- [aoutarm9045.cpp](../../../../../HT9011UC_Cpp_V3.33.906.0/aoutarm9045.cpp)：DoOutArm_9045／USE_PICKER_COUNT／iInArmType／g_W906OutArmRule10。
- [單Picker](../../../../../HT9011UC_Cpp_V3.33.906.0/aoutarm9045_All_1Picker.cpp)：DoOutArm_9045_All_1Picker／OutArmTask／MoveOutArmToShuttleIncludeZ_9045_All_1Pick。
- [原流程與案例](../flow/original-entry.md)。
