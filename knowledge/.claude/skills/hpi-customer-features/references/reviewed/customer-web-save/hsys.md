# HSys客戶碼開窗守衛與存檔路由

來源：[V906 _EditPage.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/_EditPage.cpp)的 `kOpenGates`／`GHandlerSys`／`GNotRunning`、[V912 cTemperFrom.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cTemperFrom.cpp)的 `TfTemperFrom::Panel71MouseDown`。客戶數值見[V906 MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/MachineType.h)與[V912 MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/MachineType.h)。

## 客戶條件一列

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台 |
|---|---|---|---|---|---|
| CC_ASE_KaohSiung／936、CC_ASE_KaohSiung_K12／929 | V906 GHandlerSys；V912 TfTemperFrom::Panel71MouseDown | 直接比較CUSTOMER_CODE；另有SystemStart／AccessLevel守衛，非INSTALL_或USE_旗標 | 作用中CUSTOMER_CODE為上述任一值時，兩個已讀入口先拒絕開Handler System | V906／V912這兩個入口已核對；不是所有入口與機型已驗證 | HT9050與其他Handler同題；作用中機型／客戶碼及實際caller另核對 <!-- review-record:web-hsys-open-gate --> |

不能只由ini存有936／929推定必擋：記憶體CUSTOMER_CODE可能經boot的編譯條件重設，先讀[開機reader](../customer-boot/reader.md)。兩版數值相同只支持這一列選取符號；其他別名／顯示名稱仍按原權威分流。

V906 HSys的OpenGateRefused找到GHandlerSys：客戶條件之外，GNotRunning只直接查SystemStart，之後要求 `AccessLevel>=iDefHonPrecLevel`。主迴圈editlist.save另有SystemStart與SoftStart一起拒絕；不能把GNotRunning本體說成也查SoftStart。

V912 Panel71MouseDown除了客戶碼／SystemStart／AccessLevel，還查bGreen／bYellow、滑鼠按鍵、確認答案，並含SOFT_SIMULTE條件下的後續驗證才到HandlerSystem->ShowModal。本列只對照前面的客戶條件，不把V906開窗閘宣稱為完整重現這些手勢／確認／驗證。

## 存檔到CUSTOMER_CODE

[V906 HSys.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.cpp)的 `kPage`將HSys／THandlerSystem／HW.HandlerSys.html登記到PageRegistrar，save callback為SaveFlow，savedMark為 `SaveSystemSet:write`，lists數為0，saveReads另由kHS_SaveReads列出。因此mustSend／proxy規則仍適用，不因沒有HTEditList而略過。

SaveFlow先查HS_ModelReadError與W906_HeaterMixSaveCheck，再跑HSApplyOverlapRule／HS_SaveBtnClick；只有看到SaveSystemSet:write才跑HS_ExitBtnClick與HSTakeSnapshot。HS_SaveSystemSet的CUSTOMER_CODE賦值／WriteIniDataGeneral，以及V912表單存檔與V906初始化差異，仍讀原釘住09d926ab8的[存檔caller](../customer-persistence.md)，不改原日期或將其來源默換到本次main。

共用方法是沿route→guard→proxy→callback→reader追查；機型／客戶／權限／runtime分開。這次沒有改C++、網頁或runtime，也沒有實際存檔；完整proxy可改性、模型／溫控檢查、reauth、磁碟成功、所有reader／writer與機型dispatch續查。
