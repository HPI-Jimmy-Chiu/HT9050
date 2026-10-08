# Format橋接、UPH輸出與Change Log所選caller

[SysUtils.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.cpp)、[SysUtils.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/vclcompat/SysUtils.h)、[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)；選定完整原文與來源 hash 在 [manifest](source-manifest.json)。

`Format(fmt,Args...)` 建空AnsiString，呼叫r.sprintf(fmt.c_str(),args...)再return；printf型別適配、AnsiString→c_str與兩次vsnprintf沿 [原callee](../numeric-format/printf-family.md)。無參數inline多載直接return fmt，沒有展開%%等格式。

`ARRAYOFCONST(x)` 本體只有x，保留傳入括號，並非BCB6 TVarRec open array。ARRAYOFCONST((a,b))的token結果仍是(a,b)，在C++呼叫引數位置是一個comma expression，不能當成兩個varargs；這是語法推導，未compile或執行。原comment所謂bare arg list不作新驗證結論。

固定pin的靜態inventory包含註解、宣告與tests，不當runtime可達普查：ARRAYOFCONST只找到既有test的單項(4+1)／(25.0)呼叫及相關comment／macro；這兩個測試不能證明多項open-array等效。沒有執行test_vclcompat。

## UPH所選writer

[ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp) 的LotRecordUPH已在 [舊writer](../../../../../writers/customer-formats.md) 保存；本輪只重新核完整body hash，不新增writer完成數。filename.sprintf用as9045UPH.c_str()與LotID Text.c_str()，資料t.sprintf的三個%s引數卻直接是AnsiString UPH_StartTime／EndTime／PauseTime，第四個%d接int iUPH。透過sprintf template的conv適配才能接到所選V906 backend；不是把BCB6 class ABI搬進C varargs的證明。

資料格式已有newline，後續WriteDataToFile依 [既有helper](../../../../../writers/common-io.md)；這是輸出字串依賴，不重算UPH、site或pause。本輪未執行寫檔；source的no caller舊comment不當今日可達性結論，完整CalculateUPH／caller與machine測試仍分開。

## INI Change Log

[common.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/39c6388c0e052532ea5933ab0c53ab7b3ecf2557/HT9011UC_Cpp_V3.33.906.0/common.cpp) 所選WriteIniData(AnsiString)現行呼叫W906_ChangeLogHook_Str（非null才呼叫）；周邊舊TryStrToFloat行已是comment。真正兩次parse在本輪保存的W906_ChangeLog_Str。W906_InstallChangeLogHooks會把hook指向此函式，但安裝是否在實際啟動路徑完成，本輪未閉合。

W906_ChangeLog_Str以ret／Value各呼叫TryStrToFloat，只用兩個bool；兩次都共用freg，後續比對再用atof(ret)／atof(Value)，不是比較freg。InitialOK必須true；兩者都是numeric且atof值不同，或兩者都不是numeric且AnsiString不同，才記錄；一邊numeric一邊非numeric不命中這個if。record後才把bHasChange設true；未命中不清此reference，所選WriteIniData在呼叫前初始化false。

FileName的Offset與Contact.Data／Group Mode／Name fSocketInitialICCheckPositionOffset分流，只決定變更文字；sprintf的%s來源有明確c_str，%0.2f來源是atof double。TempChangeLog、RecordChangeLogProcess、hook安裝、完整caller、locale與保存結果仍待續；本輪不操作INI或runtime。
