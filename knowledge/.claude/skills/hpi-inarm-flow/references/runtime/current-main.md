# 目前main入口與歷史文件界線

20261006只讀核對main `372b91908`；未執行程式或機台。查證採function／關鍵變數，保存件的舊程式碼行號不作目前定位。

1. `ainarm2.cpp::DoInArm`先看`bInitialStartIndexCheckDone`、`iHPHangUpCount`、`bDoingF16`、QA、auto-alignment、Index jam／drop等guard；`iHPHangUpCount`處會記錄、報WAR0150與清HOME旗標。不是Task不變就代表特定取料函式壞了。
2. `ainarm9045.cpp::DoInArm_9045`重新計算`bRunAutoSiteMapping`，檢查停機／servo／Destroy／pause／reset／laser；最後`USE_PICKER_COUNT==ep1Picker`優先呼叫`DoInArm_9045_All_1Pick`，其後才由`iInArmType`分派。
3. 同檔`DoInArm_9045_Type`依`TestIF_File.iTestMode`、`iUseSuckMode`、`dSiteXPitch`、`Prod.HotPlateForm`與`CheckPickerMode`設`iInArmType`／`iXStep`。Single Picker的dispatcher優先與這份Type表是兩件事，不能只讀表就推最終callee。
4. `ainarm9045_All_1Pick.cpp::DoInArm_9045_All_1Pick`以`int &Task=iArmTask`處理流程，Hot模式仍有`CheckHasSpaceToPlace_9045`與Task=500路線；此名稱是通用單Picker流程，不是按HT9050專設的新函式。
5. 仍須辨活函式與保存body：`DoInArm`的auto-alignment helper呼叫留有#if0；`DoInArm_9045`內DualSiteUseOneSuck分支除了callee還有「not translated」訊息。不能因dispatcher可見就宣稱所有機型完整可用。
6. 原SKILL含202605～202609的掛機案例、ASM借還方案回退、修正／未修摘要及多棵版本。原文完整保存；本批不把每個「已修」翻成目前main或V912出貨驗證，亦不重算歷史呼叫數。

目前`aHotPlateSubstrate.h`的精簡TMySucker／TMyKitSuck鏡像在#if0退役區段，extern註明與mykitsuck.h型別相同（A4-6 20260924）。ainarm2.cpp較早移植註解「兩套layout不同」不能當今日類別現況；仍依實際include與型別追資料，不擅自換標頭。

## 查證來源

- [吸嘴宣告與退役鏡像](../../../../../HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.h)：InArmSuck／TMyKitSuck／A4-6退役區段。
- [ainarm2.cpp](../../../../../HT9011UC_Cpp_V3.33.906.0/ainarm2.cpp)：DoInArm／bInitialStartIndexCheckDone／iHPHangUpCount／bInArmNeedToSafePos。
- [ainarm9045.cpp](../../../../../HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp)：DoInArm_9045／DoInArm_9045_Type／USE_PICKER_COUNT／iInArmType。
- [Single Picker](../../../../../HT9011UC_Cpp_V3.33.906.0/ainarm9045_All_1Pick.cpp)：DoInArm_9045_All_1Pick／iArmTask／CheckHasSpaceToPlace_9045。
- [搜尋放熱盤](../../../../../HT9011UC_Cpp_V3.33.906.0/ainarm_SearchPlacePlate.cpp)／[搜尋取熱盤](../../../../../HT9011UC_Cpp_V3.33.906.0/ainarm_SearchPickPlate.cpp)：SearchPlateToPlace／HotplateDataConversion與原文指定函式。
- [原流程與裁決](../flow/original-entry.md)／[V899吸取調查](../vacuum/original-entry.md)。
