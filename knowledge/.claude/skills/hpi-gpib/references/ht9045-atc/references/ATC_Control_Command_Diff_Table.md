# ATC Control Command 差異對照表

以下只列出 Control Command 在 ATC Interface Define.xlsx、Handler_Command_Report.md 與 ATC_Handler_Side.cpp 之間有差異的項目，並依 Code 由小到大排序。

| Code | Excel | 報告 | 原始碼 | 差異說明 |
| --- | --- | --- | --- | --- |
| 1072 | ATC_READ_FUNCTION_STATUS | ATC_READ_FUNCTION_STATUS | ATC_READ_NetWork_VALVE | Excel 與報告一致，但原始碼名稱不同，實作落在水閥相關命名。 |
| 1091 | HANDLER_SLK_LAYOUT | HANDLER_SLK_LAYOUT | ATC_AIRMACHINE_STATUS2 | Excel 與報告一致，但原始碼改成 TriTemp 相關命名。 |
| 1099 | ATC_GETDEWPOINTTEMP | ATC_GETDEWPOINTTEMP | ATC_DewPoint | Excel 與報告一致，但原始碼改成 DewPoint 命名。 |
| 1100 | ATC_51_SET_TJ_WATCHDOG | ATC_51_SET_TJ_WATCHDOG | ATC_51_Set_TJ_WATCHDOG | 只有大小寫差異，功能意義相同。 |
| 1121 | ATC_SET_WValve_ENABLED | 無 | 無 | 這是 Excel 先新增的命令，報告與原始碼未列出。 |
| 1122 | ATC_Recipe_By_Channel | ATC_Recipe_By_Channel | 無 | 報告有列出，但原始碼未列出。 |
| 1123 | ATC_Recipe_By_Public | ATC_Recipe_By_Public | 無 | 報告有列出，但原始碼未列出。 |
| 1124 | ATC_SET_CHILLER_ENABLED | 無 | 無 | 這是 Excel 先新增的命令，報告與原始碼未列出。 |
| 1125 | ATC_SET_TC_WATER_VALVE | ATC_SET_TC_WATER_VALVE | ATC_Set_TC_WATER_VALVE | 只有大小寫差異，功能意義相同。 |
| 1129 | ATC_SET_MULTI_TC_OFFSET | ATC_SET_MULTI_TC_OFFSET | ATC_MultiSensorOfs | Excel 與報告一致，但原始碼改成 MultiSensorOfs 命名。 |
| 1130 | ATC_CHILLER_WATERWARNING | ATC_CHILLER_WATERWARNING | 無 | Excel 與報告有列出，但原始碼未列出。 |
| 1131 | ATC_Multi_Temperature_Control | ATC_Multi_Temperature_Control | ATC_MultiSensorEnabled | Excel 與報告一致，但原始碼改成 MultiSensorEnabled 命名。 |
| 1132 | ATC_Handler_Transmit_Recipe | ATC_Handler_Transmit_Recipe | 無 | Excel 與報告有列出，但原始碼未列出。 |
| 1133 | ATC_DEFROSTING | 無 | 無 | 這是 Excel 先新增的命令，報告與原始碼未列出。 |
| 1134 | ATC_Star_Transmit_Recipe | ATC_STAR_TRANSMIT_RECIPE | 無 | Excel 與報告只有大小寫差異，原始碼未列出。 |
| 1135 | ATC_SET_SINGLE_OFFSET_MTP | 無 | 無 | 這是 Excel 先新增的命令，報告與原始碼未列出。 |
| 1136 | ATC_51_ATO_AdjustValve | ATC_51_ATO_AdjustValve | 無 | Excel 與報告有列出，但原始碼未列出。 |
| 1137 | ATC_HulkMode | ATC_HulkMode | 無 | Excel 與報告有列出，但原始碼未列出。 |
