# DoAutoRetest的客戶分支（局部核對）

來源：[v912 AutoRetest.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/AutoRetest.cpp)、[v906-cpp AutoRetest.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/09d926ab86fea6cf366bd9d6c90167d51dedf317/HT9011UC_Cpp_V3.33.906.0/AutoRetest.cpp)；客戶符號的數值按各版MachineType.h，不跨版套用別名。

表中的「行為」指條件成立後的程式敘述，實際完成效果另讀[caller與守衛](callers-effects.md)。兩版bReset初始化與正常流程前的安全位置檢查都已讀，不能跳過它們直接宣稱case可達。

| 客戶碼 | 函式／Task | 開關／條件 | 行為差異 | 來源版本 | 相關機台 |
|---|---|---|---|---|---|
| CC_PTI | DoAutoRetest／case 250 | hanaART.IsHanaArtAvailable未成立；bB03_TesterReport；!bWaitEndLotAutoRetestGPIB | break，維持Task 250等待；HANA可用的前置if會略過這個else if | V912／V906 Cpp | HT9050及其他Handler須先核對ART caller、客戶碼與tester；V906 facade目前回false |
| CC_PTI | DoAutoRetest／case 300 | 進入case內放行條件後，bB03_TesterReport | 保留cbRunMode.Text；其他客戶／B03未開才寫RT | V912／V906 Cpp | 同上；RunMode字串不證明tester已開始RT |
| CC_AMKOR_Korea | DoAutoRetest／case 300 | 進入case內放行條件後，CUSTOMER_CODE相符 | 呼叫Clarn_Data(0, "Auto Retest")，再走共用Tag 7呼叫 | V912／V906 Cpp | HT9050及其他Handler共用此條件，清除項目仍受bA61DisableCleanMUBA與bCTClear限制 |
| CC_HANA_MICRON | DoAutoRetest／case 300 | 進入case內放行條件後，CUSTOMER_CODE相符 | V912與AMKOR以OR相連，會呼叫Tag 0；V906同一位置只列AMKOR，HANA仍走共用Tag 7 | V912對照V906 Cpp | 僅此函式／case的差異；不稱全HANA功能為912-only，也不以機型名猜客戶 |

<!-- review-record:art-pti-wait -->
<!-- review-record:art-pti-runmode -->
<!-- review-record:art-amkor-clear -->
<!-- review-record:art-hana-clear-difference -->

case 300的共用放行條件為 `LastSet.bWaitStartLotAutoRetestGPIB`，或 `hanaART.IsHanaArtAvailable && IsContactAvailable`，或 `CosFunction.iAutoRetestTCPmode==2 && TestIF_File.bRENESAS_EnableFTCT`。通過後兩版都呼叫 `Clarn_Data(7, "WaitStartLotAutoRetestGPIB Sorting Count")`、設bResult=true及Task=400；V906的Contact方法是W906ART_HANAART映射，不能僅憑函式名推定現場接線。

case 250通過等待後呼叫SetLotState(4)並轉Task 300。V906的W906ART_FMAIN_SETLOTSTATE現行定義已呼叫TfMain::SetLotState；舊「no-op」註解不能取代現行定義。被呼叫函式中的tester、bridge與HANA gate仍需分開核對。
