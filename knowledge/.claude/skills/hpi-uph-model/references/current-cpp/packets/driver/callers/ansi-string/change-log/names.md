# TempChangeLog 的名稱與索引

[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)、[cpublic.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/cpublic.cpp)、[common.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common.cpp)；完整所選原文、介面／歷史與 byte／body hash 見 [manifest](source-manifest.json)。

TempChangeLog(Group,Name) 回傳顯示名稱，未改 caller 傳入的原 INI key。Group／Name 比對用 AnsiString Pos 與相等，SubString 沿已完成 1-based byte 語意，atoi 沒有完整字串／errno驗證；不把解析成功、負號或非法 byte 當成新防護。

| 分支／變數 | 正文路徑 |
| --- | --- |
| AmbientHotLowOffSet／AmbientHotMidOffSet／Low OffSet／Mid. OffSet／High OffSet／User OffSet／SingleTempLimit／Init Temp OffSet／TestOverTime Temp OffSet | Name.Pos("CH")==1 才取尾端 atoi；SingleTempLimit 以 iSiteAdd、其他以 iSiteAdd-1，各 guard 後取 asTempCtrl |
| Group ATC、ATCTempOffset | 從所選 SubString 尾端 atoi；iTestMode==_8Site2X4 且 bOctal_16Kit、iSiteAdd>=2 才減2；[0,32)才對32個 ATC 名稱加 OffSet_ |
| Group InitialMode | iInitialDelay／iInitialDelay_ 及 dInitialDelay_10 prefix 各依正文選10個 delay caption；還有既有具名 key 改顯示名稱 |
| Time Stary Delay | 舊 Stary spelling 分支顯示 Start；原 key／原文保留 |
| Group Mode | Offset／check position 等明確 name 改顯示 caption，不是自由文字替換 |
| Group Configuration、iAutoClean_MotorSpeed[ | atoi(Name.SubString(23,1)) 只解析一個 byte，再以[0,4)選 speed caption，不是任意長度 bracket 整數解析 |
| Group Configuration 其他 AutoClean key | 按完整 key 改相應 caption，完整對照正文在 manifest |

不命中、或 guard 不通過時 Name 保留原值；atoi 遇無數字可給0，所以 guard 不表示語法已校驗。ATC 的 site mapping 受 iTestMode／bOctal_16Kit／tcTotalCount與 asTempCtrl內容影響，沒有以 Type_HT9050 分支選整套機台身分。

上述只核 selector、索引與完整原文。原註解中的 BCB／golden 名稱、裁決說明與歷史 line marker 保留；未用913／客戶機台重新確認 caption文字或實際可達。需要名稱時從 function／變數查本頁，再到固定pin；活文件不靠會移動的 source line number。

回 [numeric](values.md)、[caption](captions.md)、[界線](limits.md)。
