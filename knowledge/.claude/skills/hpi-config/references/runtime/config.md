# 目前main資料層

唯讀基準main `f57d93f15`，以下定位在HT9011UC_Cpp_V3.33.906.0；未執行建構子或任何讀取函式。

cprod.cpp::ReadLastSetIni從AuthPath組config.ini，先FileRW_IniConfig_ChangeCBListProperty、CustomerFunctionSelect與ReadLastDataFile；其中機型／Machine ID等仍經CheckAndReadIniDataGeneral另取Gerneral層，不可由IniConfig名稱假設所有欄位都存在config.ini。讀取也可能補鍵／建目錄，不能拿呼叫它當純唯讀驗證。

cprod.cpp::ReadConfigByRecipe用GetRecipePath與asFileNameConfigByRecipe，對elConfig_byRecipe呼叫HTEditList_ReadEditTextFromFile及HTEditList_InitialDataToEdit。IniConfig／LastSet／TestMode是不同結構；Recipe切換與保存caller另看 [LotInfo／Recipe](../../../hpi-lotinfo-recipe/SKILL.md)。

FileRW/IniConfig.gen.inc綁cbN07_EnableSecs與IniConfig.bEnable_SECS_GEM，INI section SECS GEM、key Enable SECS GEM；依分支可固定或ReadFromFile。判SECS開關看此實際變數與caller，不拿Gerneral.ini中同名概念的鍵代替。原「key從未被讀」是原掃描紀錄，本次只核對此綁定，沒有宣稱重掃所有版本。

database.cpp::SYSTEM_MODULAR::ReadGeneralIni同時涉及D:\GPIB9045\system\general.ini與asGeneralPath的Handler Gerneral.ini，先辨兩層Model／設定來源。FileRW/HSys.cpp的保存後重讀也可能補鍵、重讀config及客戶設定；原mapping表不等於純讀API。
