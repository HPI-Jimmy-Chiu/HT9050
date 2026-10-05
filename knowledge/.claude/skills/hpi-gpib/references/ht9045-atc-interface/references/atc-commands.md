# ATC Interface Define - Full Command Table

來源：ATC Interface Define.xlsx（137 命令，1001-1137）

| CODE | Command | Description | Example | Other |
|------|---------|-------------|---------|-------|
| 0001 | ATC_NO_THIS_COMMAND | ATC not Support this command |  |  |
| 1001 | ATC_RECIPE_FILE | ATC Change Recipe File |  |  |
| 1002 | ATC_SET_TEMP | Set ATC Run Temperature | Handler   @1002,channels,n,n,n...... //(5.5C  n=55) ATC          @1002.1.1# |  |
| 1003 | ATC_SET_TOFS | Temp. Offset |  |  |
| 1004 | ATC_SITE_ENABLED | Set Use Site | Handler   @1004,channels,n,n,n.....t //n:0(Disable)1(Enable) ATC          @1004.1.1# |  |
| 1005 | ATC_SET_TRM | Temp. Reading Mode |  |  |
| 1006 | ATC_SET_MAX_TEMP | Set ATC MAX Working Temperature |  |  |
| 1007 | ATC_RUN_STOP | ATC Run & Stop | Handler   @1007,1,n#                         //n:0(ATC Stop) 1(ATC RUN) ATC          @1007.1.1# |  |
| 1008 | ATC_INITIAL_SET | ATC Initial all Setting |  |  |
| 1009 | ATC_MODE_TYPE | ATC Mode Type |  |  |
| 1010 | ATC_READ_TEMP | Read ATC Temperature | Handler   @1010,1,channels# ATC          @1010,(channels*2+1),Tc1,Tj1,Tc2,Tj2,……,XXXX# |  |
| 1011 | ATC_HANDLER_ARM | Inform Handler Use Arm |  |  |
| 1012 | ATC_TEST_START | Inform Handler Testing or Test finish | Handler   @1012,1,n#            //n=0(Start Test)   n=1(Test End) Handler   @1012,2,n,2DID#   //n=0(Start Test)   n=1(Test End) , 2DID=Device 2D Barcode ATC          @1012,1,1# | 2024/3/25 add 2DID (TSMC) |
| 1013 | ATC_SEND_TJ | Send TJ Temperature to ATC |  | Not been used |
| 1014 | ATC_ERROR | ATC Error Message |  |  |
| 1015 | ATC_SET_SINGLE_TEMP | Set Single Site Temperature |  |  |
| 1016 | ATC_SET_SINGLE_OFFSET | Set Single Site Offset |  |  |
| 1017 | ATC_CANCEL_MESSAGE | Inform ATC Close Alarm Form |  |  |
| 1018 | ATC_CHILLER_STATUS | Query Chiller ststus |  | For ATC 3.0 & 6.0 |
| 1019 | ATC_SET_PID | Set ATC PID |  |  |
| 1020 | ATC_READ_PID | Read ATC PID |  |  |
| 1021 | ATC_USE_TSD | Use TSD Function |  | For ATC 7.0 |
| 1022 | ATC_EMG_UP_DOWN | Handler EMG Up or Down |  | For ATC 7.0 |
| 1023 | ATC_READ_TEMP_2 | Read ATC Second Temperature |  |  |
| 1024 | ATC_READ_SETTEMP | read ATC set temperature |  |  |
| 1025 | ATC_STATE | read ATC state |  |  |
| 1026 | ATC_RUN_MODULE | Run ATC Module |  | For ATC 5.0 |
| 1027 | ATC_STOP_MODULE | Stop ATC Module |  | For ATC 5.0 |
| 1028 | ATC_SET_2ND_RANGE_P | Setting second point temperature Positive range. |  | 通知ATC設定第二感溫點Alarm 範圍正極限 |
| 1029 | ATC_SET_2ND_RANGE_N | Setting second point temperature Negative range. |  | 通知ATC設定第二感溫點Alarm 範圍負極限 |
| 1030 | ATC_SET_2ND_TIME | Setting second point temperature interval time. |  | 通知ATC設定第二感溫點Alarm 時間 |
| 1031 | ATC_SW_VER | ATC Software Version |  |  |
| 1032 | ATC_FW_VER | ATC Firmware Version |  |  |
| 1033 | ATC_TIME_SYNC | Time synchronization to Handler |  |  |
| 1034 | ATC_SET_2ND_ENABLED | Setting second point temperature Function enabled. |  | 通知ATC設定第二感溫點功能 |
| 1035 | ATC_READ_OFFSET | Read ATC Set Offset |  |  |
| 1036 | ATC_GET_NOW_RECIPE | Get Now Recipe File | Handler   @10361,1# ATC          @1036,1,Recipe# |  |
| 1037 | ATC_GET_RECIPE_LIST | Get Recipe File List | Handler   @10371,1# ATC          @1037,n,List# |  |
| 1038 | ATC_SET_TjMode | Set Tj mode for AMD | Handler   @1038,x,n,n,n…..#   //n:Control Mode (0:TC   1:TJ    2:TS) ATC          @1038,1,1# |  |
| 1039 | ATC_SET_CHILLER_TEMP | set chiller temperature | Handler   @1039,1,n#   //n:Temperature ATC          @1039,1,1# |  |
| 1040 | ATC_SEND_TEMP_READY | send temperature ready | Handler    ATC          @1040,Channel,n,n,n,...#   //Channel:Use Channel   n:0(not ready) 1(ready) |  |
| 1041 | ATC_SITE_2ND_CHECK | Site Check Second Temperature | Handler   @1041,x,n,n,n,n,n….#   //n:0(Disable)  1(Enable) ATC          @1041,1,1# |  |
| 1042 | ATC_SET_TJ_OFFSET | set Tj offset | Handler   @1042,x,n,n,n…….#  //n: Offset ATC          @1042,1,1# |  |
| 1043 | ATC_SET_SINGLE_TJ_OFFSET | set Tj single offset | Handler   @1043,2,Site,Offset#   ATC          @1043,2,Site,Offset# |  |
| 1044 | ATC_RUN_SELFTEST | run ATC self-test | Handler   @1044,1,1#   ATC          @1044,1,1# |  |
| 1045 | ATC_GET_2ND_STATUS | Get second sensor function enabled. | Handler   @1045,1,n#  //讀取n組 ATC          @1045,n+1)*2,Active,Interval,Limit_p,Limit_n,Limit_p,Limit_n,……….# |  |
| 1046 | ATC_MANUAL_SELFTEST | Manual ATC selt-test functin | Handler   @1046,1,1#   ATC          @1046,1,1# |  |
| 1047 | ATC_SELFTEST_STATUS | Get self-test status | Handler   @1047,1,1#   ATC          @1047,1,n#   //n:1(+5c)  1(-5c) 2(+0c) |  |
| 1048 | HANDLER_STATUS | Check Handler Status | Handler   @1048,1,n#   //n:Status HANDLER          @1048,1,1# | Handler ASK "Are you there?" |
| 1049 | ATC_AIRMACHINE_STATUS | Query Air Machine ststus | Handler   @1049,1,n#   //n:Ststus ATC          @1049,1,1# | For ATC 5.1 |
| 1050 | ATC_LOT_START | Lot Start &  Lot ID | Handler   @1050,1,Lot_ID# |  |
| 1051 | ATC_LOT_END | Lot End | Handler   @1051,1,Lot_ID# |  |
| 1052 | ATC_SELFTEST_RESULT | Query Self-Test Result | Handler   @1052,1,n#     //讀取n組 ATC          @1052,n,m1,m2,m3…...#   //n: Data Count  m:Self-Test Result | 詢問Self-Test結果 回傳資料0:失敗 1:成功 |
| 1053 | HANDLER_ABNORMAL_SITE | Handler send abnormal site to ATC | Handler   @1053,n,Fail Channel,Fail Channel,…….# | Handler傳送有異常的Site給ATC |
| 1054 | ATC_GET_SN | Get ATC power supply serial number | Handler   @1054,1,1#   ATC          @1054,Channel,SN1,SN2,SN3…...#   //回覆所有Channel的序號 |  |
| 1055 | ATC_SHUTDOWN | ATC SHUTDOWN | Handler   @1055,1,n#   //n:Shutdown ATC          @1055,1,1# | for 1028c |
| 1056 | ATC_51_FREONRECOVER | ATC FREON RECOVER | Handler   @1056,1,n#   //n:Recover ATC          @1056,1,1# | for 勝麗 |
| 1057 | HANDLER_CAN_SELFTEST | ATC Query Handler can self-test? | Handler   @1057,1,n#   //n:Test? HANDLER          @1057,1,1# | Eliot 20171211 |
| 1058 | ATC_51_SET_REF_ENABLED | Setting refrigerator enabled. | Handler   @1058,1,n#   //n:Enabled. ATC          @1058,1,1# | for HT3012CT |
| 1059 | ATC_51_GET_REF_STATUS | Query refrigerator Status. | Handler   @1059,1,n#   //n:Status. ATC          @1059,1,1# | for HT3012CT |
| 1060 | ATC_51_SET_DEFROST | Setting ATC defrost. | Handler   @1060,1,n#   //n:Defrost. ATC          @1060,1,1# | for ASE_CL |
| 1061 | ATC_60_SET_AIRVALVE | Setting open & close air valve | Handler   @1061,1,n#   //n:Valve ATC          @1061,1,1# | for Qualcomm |
| 1062 | ATC_READ_SOCKETTEMP | read ATC socket temperature | Handler   @1062,1,1#   ATC          @1062,4,Socket1,Socket2,Socket3,Socket4# | for ANST |
| 1063 | HANDLER_FFC_SETTING | FFC Data | Handler   @1063,1+(n*3),Arm,StartTime,EndTime,Offset,StartTime,EndTime,Offset……...#    ATC          @1063,1,1# |  |
| 1064 | HANDLER_ATO_RECORD | Record Auto Temperature Offset data | Handler   @1064,1,n#   //n:Data HANDLER          @1064,1,1# |  |
| 1065 | HANDLER_FFC_ENABLED | FFC ENABLED | Handler   @1065,1,n#   //n:0(Disable)  1(Enable) ATC          @1065,1,n# |  |
| 1066 | HANDLER_FFC_TRIGGER | FFC Trigger | Handler   @1066,x,n,n,n,n,n….#   //n:0(Disable)  1(Enable) ATC |  |
| 1067 | ATC_GET_CONTROL_MODE | Controller Control Mode Status | Handler   @1067,1,1# ATC          @1067,Channel,n,n,n…..#     //回覆所有Channel的控制模式 |  |
| 1068 | HANDLER_PFC_ENABLED | Power Following Control ENABLED | Handler   @1068,8,n,n,n,n,n,n,n,n#  //n:0(Disable)  1(Enable) ATC          @1068,1,1# |  |
| 1069 | ATC_GET_PFC_PARAMETER | Get Power Following Parameter | Handler   @1069,1,1# ATC          @1069,280,Parameter#    //Parameter:280組參數資料 |  |
| 1070 | ATC_SET_PFC_PARAMETER | Set Power Following Parameter | Handler   @1070,21,Temp,Parameter#  //Temp:溫度段 Parameter:20組參數資料 ATC          @1069,280,Data#    //Data:280組參數資料 |  |
| 1071 | ATC_READ_WATER_VALVE | Read ATC Water valve value | Handler   @1071,1,4# ATC          @1071,4,WV1,WV2,WV3,WV4#    //WV : Water Valve |  |
| 1072 | ATC_READ_FUNCTION_STATUS | Read ATC Function Status | Handler   @1072,1,1# ATC          @1072,1,CheckNetwork#    //CheckNetwork : 0(Disable)  1(Enable) |  |
| 1073 | ATC_GET_HEAT_OUTPUT | Read ATC Heat output value | Handler   @1073,1,n#   //n:Value ATC          @1073,1,1# | output data : %5.2f |
| 1074 | ATC_AUTO_TCTS_ENABLED | Auto Tc Ts switch function | Handler   @1074,1,n#     // n:0(Disable)  1(Enable) ATC          @1074,1,1# | 自動Tc/Ts控溫切換功能 |
| 1075 | HANDLER_TEST_SITEMAPPING | Handler send site mapping | @1075,8,1,0,0,0,0,2,0,0# |  |
| 1076 | ATC_SET_TJ_ENABLED | Set TJ Function enabled | @1076,1,1# |  |
| 1077 | ATC_SET_TJ_PARAMETER | Set TJ slope and offset | @1077,2,-82.52165,-207.1066# (參數1: TJ slope / 參數2 :TJ offset) |  |
| 1078 | ATC_51_SET_ExtraHeatGun |  | @1078,2,160,250# @1078,2,加熱棒啟動溫度 , 加熱棒設定度# |  |
| 1079 | ATC_51_READ_ExtraHeatGun |  | @1079,1,3# @#1079,3,加熱棒現在溫度 ,加熱棒啟動溫度 , 加熱棒設定度# |  |
| 1080 | ATC_SWAP_CH2CH3_DATALOG | Swap Ch2 And Ch3 Data Pos In The Log | Handler   @1080,1,n#     //n:0(Disable)     n:1(Enabled) ATC          @1080,1,n#     //n:0(Disable)     n:1(Enabled) |  |
| 1081 | ATC_READ_TCTJ_WATER_VALVE | Read ATC TC &TJ Water valve value | Handler   @1081,1,1#                      //讀取1組 ATC          @1081,2,TcWV,TjWV#   //TcWV: Tc水閥開度 , TjWV: Tj水閥開度 |  |
| 1082 | ATC_READ_TJ_PARAMETER | Read TJ slope and offset | Handler   @1082,1,1,#                      //讀取1組 ATC          @1082,2,Slope,Offset#   //Slope:Tj Slope, Offset: Tj Offset |  |
| 1083 | ATC_READ_HEATING | Read ATC Heating | Handler @1083,1,8#    ATC        @1083,8,n,n,n,n,n,n,n,n#                   Data=n,n,n,n,n,n,n,n 分別表示Site1,2,3,4,5,6,7,8 加熱量狀態 n:0(Heating NG)     n:1(Heating OK)                  假設0,1,0,0,0,0,0,0表示Site2加熱量到達 其他SITE 尚未完成                  假設1,1,1,1,1 |  |
| 1084 | ATC_GET_SLOPE_OFFSET | Get the Slope/Offset value | Handler   @1084,1,1#     讀取1組 ATC          @1084,2,Slope,Offset#   (Tj Slope/Offset) |  |
| 1085 | ATC_SET_SLOPE_OFFSET | Set the Slope/Offset value | Handler   @1085,2,Slope,Offset# 設定一組 ATC          @1085,2,Slope,Offset#   (Tj Slope/Offset) |  |
| 1086 | ATC_GET_TJ_VOLTAGE | Get the Tj Voltage | Handler   @1086,1,8#    讀取8個Tj電壓 ATC          @1086,8,Voltage1,Voltage2,Voltage3,Voltage4,Voltage5,Voltage6,Voltage7,Voltage8,# |  |
| 1087 | ATC_RECORD_TJ_TEMP | Record Tj Temp(Max/Min/Avg) | Handler   @1087,2,m,n#     //m:-1:none 0:Arm1 1:Arm2 2:Both Arm         n:0(Stop)  1(record) ATC          @1087,2,m,n#     //m:-1:none 0:Arm1 1:Arm2 2:Both Arm         n:0(Stop)  1(record) |  |
| 1088 | ATC_QUERY_TJ_TEMP | Query Tj Temp(Max/Min/Avg) | Handler   @1088,1,1#     //讀取1組 ATC          @1088,3,MaxTj,MinTj,AvgTj#   //MaxTj: Tj最大值 , MinTj: Tj最小值 , AvgTj:Tj平均值 |  |
| 1089 | ATC_51_SET_REF_ADJUST | Read ATC5.1 set refrigerator valve | Handler   @1089,8,1,2,3,4,-5,-6,-7,-8# ATC          @1089,1,1# |  |
| 1090 | ATC_51_AUTOLOAD_RECIPE | ATC5.1 Autoload golden recipe by Temp | Handler   @1090,1,1#    //0:關閉  1:開啟 ATC          @1090,1,1# |  |
| 1091 | HANDLER_SLK_LAYOUT | Handler send the SLK layout | Handler   @1091,2,8,8# ATC          @1091,1,1# |  |
| 1092 | ATC_GET_HEATER_POWER | Get the Heater Output Power | Handler   @1092,1,n#    讀取n組 ATC          @1092,n,n,n……………...#  回傳值 0~65535 (0~100%) | 2023/4/27 Eliot Add |
| 1093 | ATC_KL_SV | KL set temp to Handler | ATC          @1093,channels,n,n,n……# (85= 850) Handler   @1093,1,1# | 2023/6/7 Dell Add |
| 1094 | ATC_KL_OFFSET | KL set offset to Handler | ATC          @1094,channels,n,n,n……# (5.5= 55) Handler   @1094,1,1# | 2023/6/7 Dell Add |
| 1095 | ATC_KL_TRIGGER | KL set start KL control by Handler | Handler   @1095,1,n#  //n = 0:關閉  1:開啟 ATC          @1095,1,1# | //2023-06-12 Zek |
| 1096 | ATC_KL_TRIGGER_STATUS | KL set start Handler read status | Handler   @1096,1,1# ATC          @1096,1,n#  //n = 0:關閉  1:開啟 | //2023-06-12 Zek |
| 1097 | ATC_READ_TEMP_3 | Read ATC Multi Temperature | Handler   @1097,1,Channels# ATC          @1097,Channels*Count,n,n,n……# (85= 850) | 2023/11/06 Cheng Add |
| 1098 | ATC_LOAD_RECIPE_SINGLE | Load single channel config from Recipe File | Handler   @1098,3,Recipe File Name,Set Temperature,Channel# ATC          @1098,1,1# | 2023/8/9 Eliot Add |
| 1099 | ATC_GETDEWPOINTTEMP | Read handler dew point temperature | Handler   @1099,3,n,n,n#  (Index,InShuttle,OutShuttle) (85= 850) ATC          @1099,1,1# | 2023/08/11 Cheng Add (1096->1099) |
| 1100 | ATC_51_SET_TJ_WATCHDOG | setting wathchdog parameters | Handler   @1100,4,1,3,0.67,0.75#  參數1 啟用=1/關閉=0,參數2 SOT delay 秒數,參數3 voltage range,參數4 voltage range ATC          @1100,1,1# | //Dell  2023-11-07 |
| 1101 | ATC_AIRMACHINE_TEMP | Read Air Machine Temperature | Handler   @1101,1,c#         //c:Channels ATC          @1101,c,t…..#     //c:Channels   t:Temp | For ATC 5.1 |
| 1102 | ATC_SAVE_RECIPE_PROHIBIT | Saving recipe data is prohibited | Handler   @1102,1,n#  //n = 0:關閉  1:開啟 ATC          @1102,1,1# | 2023/12/13 Eliot |
| 1103 | ATC_READ_CHILLER_TEMP | Read chiller temperature | Handler   @1103,1,1#    ATC          @1103,1,n1,n2,n3#  //n1:Chiller SV,  n2:#1 now temp,  n3:#2 now temp | 2024/1/10 |
| 1104 | ATC_SET_PF_PARAMETER | Set PF slope and offset | //n1 = 0:關閉  1:開啟 ; n2 = full ;n3 = slope ;n4 = w-gain ;n5 = many to one Handler   @1104,5,1,1900,239.3,1.2,2# ATC          @1104,1,1# | 2024/01/22 |
| 1105 | ATC_SET_T2OFS | Set Tc2 Offset | Handler   @1105,Channel Count,n,n,n,n,n….#   (n: 25=2.5c) ATC          @1105,1,1# | 2024/3/11 Eliot |
| 1106 | ATC_SET_SINGLE_T2OFS | Set Single Site Tc2 Offset | Handler   @1106,2,Channel,n#   (n: 25=2.5c) ATC          @1106,1,1# | 2024/3/11 Eliot |
| 1107 | ATC_SET_HEAT_OUTPUT_LIMIT | Set Heat Output Limit | Handler   @1107,Channel Count,n,n,n,n,n….#   (n: 0~100% , 20=20%) ATC          @1107,1,1# | 2024/3/18 Eliot |
| 1108 | ATC_GET_CTRL_MODE | Get ATC Controller Control Mode | Handler   @1108,1,c#               //c:Channels ATC          @1108,c,m,m…..#     //c:Channels   m:control mode | 2024/4/26 Eliot |
| 1109 | ATC_READ_HYGON_STATUS | READ HYGON_STATUS | Handler   @1109,1,Channels# ATC          @1109,Channels*Count,a,b,c,d,e,f,h,i,j,k,l…..#               (Count=12) | 2024/5/06 Cheng |
| 1110 | ATC_READ_WATER_FLOW | Read water flow value | Handler   @1110,1,c#              //c:Channels ATC          @1110,c,w,w…..#      //c:Channels   w:water flow value | 2024/5/14 Eliot |
| 1111 | ATC_SET_CoolingValue | SET Cooling Value | Handler   @1111,8,100,90,100,100,90,100,100,100#  (range 0% to 100%) ATC          @1111,1,1# | 2024/08/16 Dell |
| 1112 | ASIF_TJ_EFUSED | LVTS cal item, after Tester read out chip's data | Handler   @1112,3,ASIF_TJ_EFUSED,"EFUSE_DATA1:[1:99999,2:99999,3:99999]","EFUSE_DATA2:[1:99999,2:99999,3:99999]"#                                                                                                      //1: is site No. ATC          @1112 | For MTK ASIF 20240827 Steven |
| 1113 | ASIF_TJ_REQUEST | Send Tj data (all LVTS sensors) | Handler   @1113,1,ASIF_TJ_REQUEST# ATC          @1113,3,ASIF_TJ_EFUSED,"LVTS1_A:[1:99999,2:99999,3:99999]","LVTS2_A:[1:99999,2:99999,3:99999]"#                                                                                                    //1: is | For MTK ASIF |
| 1114 | ASIF_TJ_FB | Send Tj data (all LVTS sensors) | Handler   @1114,1,ASIF_TJ_FB# ATC          @1114,3,ASIF_TJ_FB,"LVTS1_A:[1:99999,2:99999,3:99999]","LVTS1_B:[1:99999,2:99999,3:99999]"#                                                                                                    //1: is site No. | For MTK ASIF |
| 1115 | ASIF_TJ_CPU | Send Tj data by specific LVTS sensors | TBD | For MTK ASIF |
| 1116 | ASIF_TJ_SOC | Send Tj data by specific LVTS sensors | TBD | For MTK ASIF |
| 1117 | ASIF_TJ_GPU | Send Tj data by specific LVTS sensors | TBD | For MTK ASIF |
| 1118 | ASIF_TJ_FPC | Send Tj data by specific LVTS sensors | TBD | For MTK ASIF |
| 1119 | ASIF_TJ_EOT | Test End. Batch feedback Tj data during testing. | TBD | For MTK ASIF |
| 1120 | HANDLER_2DID | Device 2DID name stores thermo profile. | Handler   @1120,1+n,Arm,2DID,2DID……...#    ATC          @1120,1,1# | AMD-US  20240829 Eliot |
| 1121 | ATC_SET_WValve_ENABLED | Set Water Valve Enabled | Handler   @1121,Channels,n,n,n…..#   //n = 0:關閉  1:開啟 ATC          @1121,1,1# | 2024/10/08 Cheng |
| 1122 | ATC_Recipe_By_Channel | ATC Recipe by Channel parameters | Handler   @1122,1,Channel#   //channel Range= 1~16 ATC          @1122,m,Channel,n,n,n…….#   m: data count , n:data 	TC : P/I/D 	TS : P/I/D 	TJ : P/I/D 	TC: Water Valve 	TJ: Water Valve 	Check Water Flow Function | 2024/10/25 Eliot |
| 1123 | ATC_Recipe_By_Public | ATC Recipe by public parameters | Handler   @1123,1,1#   ATC          @1123,m,n,n,n…….#   m: data count , n:data 	Water Valve Control Mode (Function-2 page) 	Flow control P/I/D (Function-2 page) 	Use Different Flow function 	DiffFlow_SV1_Temperature 	DiffFlow_SV2_Temperature 	D | 2024/10/25 Eliot |
| 1124 | ATC_SET_CHILLER_ENABLED | Set Chiller Enabled | Handler   @1124,Channel,n,n,n,...#   //Channel:Use Channel   n:0(Disable)  1(Enable) ATC          @1124,1,1# | 2025/01/16 Cheng |
| 1125 | ATC_SET_TC_WATER_VALVE | Set ATC Tc Water Valve | Handler   @1125,Channels,n,n,n,...#   //n:Tc Water Valve Value (n: 0~100%, 20=20% , -1 = 維持水閥不修改) ATC          @1125,1,1# | 2025/04/16 EvanHsieh |
| 1126 | HANDLER_ALARM | Handler To ATC Alarm Message | Handler   @1126,1,HandlerAlarmCode#    ATC          @1126,1,1# | 2025/0428 EvanHsieh |
| 1127 | ATC_SET_DYNAMIC_PID | Set ATC Dynamic PID | Handler   @1127,Channels,n,n,n,..#     //(n:第幾組PID參數) ATC          @1127,1,1# | 2025/0512 Dustin |
| 1128 | ATC_READ_DYNAMIC_PID | Read the current set of ATC Dynamic PID | Handler   @1128,1,channels#    ATC          @1128,channels,n,n,n…….#     n:data | 2025/0522 Dustin |
| 1129 | ATC_SET_MULTI_TC_OFFSET | Set Multi Tc Offset | Handler   @1129,channels,a1,a2,a3,a4,b1,b2,b3,b4,….# (a1=site1的Tc1 \\| a2=site1的Tc2 \\| b3=site2的Tc3 \\| b4=site2的Tc4... ) ATC          @1129,1,1# Example  @1129,8,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32 | 2025/08/12 Evan 目前支援ATC3.5/3.6 (4區感溫點Tc1~Tc4) |
| 1130 | ATC_CHILLER_WATERWARNING | ATC CHILLER WATERWARNING | Handler   @1130,1,n,#      //(n:1 水位<85,n:0 水位>85) ATC          @1130,1,1,# | 2025/09/02 水位<85提示 回傳1 |
| 1131 | ATC_Multi_Temperature_Control | Enable/Disable MTC | Handler   @1131,4,TC1,TC2,TC3,TC4# (TC1 =1 -> Enable , TC2 = 0 -> Disable...) ATC          @1131,1,1# | 20251010 Evan 目前ATC3.5/3.6支援四區感溫點(Tc1~Tc4 |
| 1132 | ATC_Handler_Transmit _Recipe | Transmit recipe file between ATC and Handler. | @1132,2,FileName,FileSize# @1132,1,1# //收到FileName & FileSize後，回傳1132,1,1表示可以傳送檔案了 | 20251021 Evan 請搭配FileTransfer.cpp/.h使用 |
| 1133 | ATC_DEFROSTING | ATC DEFROSTING | Response @1133,12,n,n,n,n…..# // 12: 分別表示 max site //     n = 0 表示 未除霜 //     n = 1 表示 除霜中 | 20251217 Victor |
| 1134 | ATC_Star_Transmit_Recipe | Handler requests to send RECIPE. | Handler  @1134,1,1# ATC         @11341,1,# | 20260122 Evan ATC收到1134後開始回傳1132 |
| 1135 | ATC_SET_SINGLE_OFFSET_MTP | ATC_SET_SINGLE_OFFSET_MTP | ATC     @1135,1,12,# Handler @1135,5,12,769,769,769,769,#  //12: ch   //769 offset 1~4 | 20260226 victor mutil offset for MTP |
| 1136 | ATC_51_ATO_AdjustValve | Auto K-Temp adjust valve | Handler @1136,8,n,n,n,n,n,n,n,n#    //n:1(Adjust valve first time)     n:2(Adjust valve second time)                                                            //n:0(Not Adjust valve)   調整時請按照0 -> 1 -> 2 的順序 ATC        @1136,1,1# | 20260211 Allen 自動K溫HotPlate太冷 需降低開度(統一減5) 開度下限暫定65 |
| 1137 | ATC_HulkMode | On/Off HulkMode for TSMC | Handler @1137,1,n#  //n = 0:關閉  1:開啟 ATC        @1137,1,1# | 20260402 for TSMC_Hulk |
