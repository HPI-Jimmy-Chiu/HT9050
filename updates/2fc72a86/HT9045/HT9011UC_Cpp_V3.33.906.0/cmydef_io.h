//==============================================================================
#ifndef cmydef_ioH
#define cmydef_ioH
//AI(W906-S1) 20261007 (Ifor01; W-124): split out of cmydef.h (header-slimming plan s A5, S1) -- C-F + the IO lines of H.
//  Lines moved verbatim, original order; cmydef.h still includes all three, so no includer changes (that is S2).
#include "cmydef_core.h"
//---- old cmydef.h :302-2522 (C C_* cylinders, D Sn* sensors, E Sw* switches, F M* motors)
extern const int C_TrayZ_Selector  ;
extern const int C_TrayY_Fixer     ;
extern const int C_Auto1Side_Fixer ;
extern const int C_Auto2Side_Fixer ;
extern const int C_Auto3Side_Fixer ;
extern const int C_Auto1_Selector  ;
extern const int C_Auto2_Selector  ;
extern const int C_Auto3_Selector  ;
extern const int C_TrayX_UpDown    ;
extern const int C_EmptyLoaderZ_Select;
extern const int C_ColorLoaderZ_Select;
extern const int C_Empty_Fix;
extern const int C_TrayCover     ;      //Steven 20140409 : Auto Retest
extern const int C_CatchTray_Fix;
extern const int C_Color_Fix;
// loader

extern const int C_LoaderEdgePush;
extern const int C_Auto1EdgePush;
extern const int C_Auto2EdgePush;
extern const int C_Auto3EdgePush;

extern const int C_Load_Up;
extern const int C_Load_Middle;
extern const int C_Color_Up;
extern const int C_Color_Middle;
extern const int C_Empty_Up;
extern const int C_Empty_Middle;
extern const int C_TrayVibration;
extern const int C_HotplateVibration;
extern const int C_CoolingValve;            //20111130  Dell
extern const int C_Auto1_Up            ;    //Auto 3上頂汽缸
extern const int C_Auto2_Up            ;    //Auto 2上頂汽缸
extern const int C_Auto3_Up            ;    //Auto 3上頂汽缸
extern const int C_Auto1LoaderZ_Select ;    //Auto 1分離汽缸
extern const int C_Auto2LoaderZ_Select ;    //Auto 2分離汽缸
extern const int C_Auto3LoaderZ_Select ;    //Auto 3分離汽缸
extern const int C_Fix1LoaderZ_Select;      //Fix 1分離汽缸   kevin 20120718 代號不能改
extern const int C_Fix2LoaderZ_Select;      //Fix 2分離汽缸
extern const int C_Fix3LoaderZ_Select;      //Fix 3分離汽缸
extern const int C_Auto2_Middle;            //kevin 20120725
extern const int C_Shuttle_Knocker_1   ;    //Shuttle敲敲
extern const int C_Shuttle_Knocker_2   ;    //Shuttle敲敲
extern const int C_InputRotateKIT      ;    //Steven 20121001 : 旋轉Kit
extern const int C_OutputRotateKIT     ;    //Steven 20121001 : 旋轉Kit
extern const int C_FixTray_FullPlace   ;    //Steven 20140310 : Fix3滿盤氣缸
extern const int C_DockYAxisOn         ;    //Steven 20140310 : One Touch Docking
extern const int C_DockYAxisOff        ;    //Steven 20140310 : One Touch Docking
extern const int C_DockXAxisOn         ;    //Steven 20140310 : One Touch Docking
extern const int C_DockXAxisOff        ;    //Steven 20140310 : One Touch Docking
extern const int C_CatchTray_FixOn    ;    //ChungHung 20140624 : Auto Retest
extern const int C_CatchTray_FixOff   ;    //ChungHung 20140624 : Auto Retest
extern const int C_TurnTrayArm        ;    //ChungHung 20140701 : AutoRetest
extern const int C_TurnTrayArmLock    ;    //ChungHung 20140814 : AutoRetest
extern const int C_OCRLight_Up        ;    //wei 20150720 OCR觸發
extern const int C_OCRLight_Down      ;    //wei 20150720 OCR觸發
extern const int C_SLK1_Clamp         ;    //JerryYang 20160524
extern const int C_SLK1_Unclamp       ;    //JerryYang 20160524
extern const int C_SLK2_Clamp         ;    //JerryYang 20160524
extern const int C_SLK2_Unclamp       ;    //JerryYang 20160524
extern const int C_Socket_Clamp       ;    //JerryYang 20160524
extern const int C_Socket_Unclamp     ;    //JerryYang 20160524
extern const int C_LoaderUpPress      ;    //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
extern const int C_HingeLookOn        ;    //wei 20170413
extern const int C_HingeLookOff       ;    //wei 20170413

extern const int C_CassetteArmCatchOn ;    //wei 20180702 MR
extern const int C_CassetteArmCatchOff;    //wei 20180702 MR
extern const int C_LoadPortYOn        ;    //wei 20180702 MR
extern const int C_LoadPortYOff       ;    //wei 20180702 MR
extern const int C_LoadPortCatchOn    ;    //wei 20180702 MR
extern const int C_LoadPortCatchOff   ;    //wei 20180702 MR

extern const int C_TrayBracketUpOn    ;    //wei 20180702 MR
extern const int C_TrayBracketUpOff   ;    //wei 20180702 MR
extern const int C_TrayBracketOpenOn  ;    //wei 20180702 MR
extern const int C_TrayBracketOpenOff ;    //wei 20180702 MR
extern const int C_StackedTrayCatchOn ;    //wei 20180702 MR
extern const int C_StackedTrayCatchOff;    //wei 20180702 MR
extern const int C_StackedTrayLockOn  ;    //wei 20180702 MR
extern const int C_StackedTrayLockOff ;    //wei 20180702 MR
//Sam 20190112 LM
//==>
extern const int C_LoadRobotX  ;
extern const int C_UnloadRobotX ;
//<==
//Sam 20190112 LM
extern const int C_Auto1UpPress       ;    //JerryYang 20190423 新增unloader壓tray
extern const int C_Auto2UpPress       ;    //JerryYang 20190423 新增unloader壓tray
extern const int C_Auto3UpPress       ;    //JerryYang 20190423 新增unloader壓tray

extern const int C_InFlipper1         ;     //Frank 20210612 : Flipper Function
extern const int C_InFlipper1Lock     ;
extern const int C_InFlipper2         ;
extern const int C_InFlipper2Lock     ;
extern const int C_InFlipper3         ;
extern const int C_InFlipper3Lock     ;

extern const int C_OutFlipper1         ;     //Frank 20210612 : Flipper Function
extern const int C_OutFlipper1Lock     ;
extern const int C_OutFlipper2         ;
extern const int C_OutFlipper2Lock     ;
extern const int C_OutFlipper3         ;
extern const int C_OutFlipper3Lock     ;

extern const int C_LoaderCasstteLock  ;
extern const int C_EmptyCasstteLock   ;
extern const int C_ColorCasstteLock   ;
extern const int C_Auto1CasstteLock   ;
extern const int C_Auto2CasstteLock   ;
extern const int C_Auto3CasstteLock   ;

extern const int C_TeachGlassUp        ;
extern const int C_Shuttle1Precisor    ;
extern const int C_Shuttle2Precisor    ;

extern const int C_MultileEmptyY_On             ;                               //KaiChen 20200716 ：OHT
extern const int C_MultileEmptyY_Off            ;
extern const int C_MultileEmptyCatch_On         ;
extern const int C_MultileEmptyCatch_Off        ;
extern const int C_MultileEmptyBracketUp_On     ;
extern const int C_MultileEmptyBracketUp_Off    ;
extern const int C_MultileEmptyCornerPush       ;
extern const int C_MultileEmptyLoaderZ_Select   ;
extern const int C_MultileEmptyZ_Select         ;
extern const int C_TrayBracket2UpOn             ;
extern const int C_TrayBracket2UpOff            ;
extern const int C_MultileEmptyLock             ;
extern const int C_TrayBracketOpen2On           ;
extern const int C_TrayBracketOpen2Off          ;

extern const int C_CSTHoldDown                  ;
extern const int C_CSTHoldDown2                 ;
extern const int C_LoaderCarEdgePush            ;                               //JimmyChiu 20220408 For Laser Scan

extern const int C_InAreaAlignment              ;                               //ChungHung 20210113 add for Alignment CCD
extern const int C_OutAreaAlignment             ;                               //ChungHung 20210113 add for Alignment CCD

extern const int C_PlacementArm                 ;                               //JimmyChiu 20220908 add Pickup Error Placement
extern const int C_TesterSidePush               ;                               //Richard 20220321 : 渠梁Side Push

extern const int C_Load2CasstteLock             ;
extern const int C_Load2_Middle                 ;
extern const int C_Load2_Up                     ;
extern const int C_Tray2Z_Selector              ;
extern const int C_Load2UpPress                 ;
extern const int C_Tray2Y_Fixer                 ;
extern const int C_Load2EdgePush                ;
extern const int C_Load2TrackFloodgate          ;
extern const int C_Load2PushBack_Push           ;
extern const int C_Load2PushBack_Back           ;
extern const int C_Load2Separate                ;
extern const int C_Load2SeparateRL              ;
extern const int C_Load2SeparateRR              ;
extern const int C_Load2SeparateFL              ;
extern const int C_Load2SeparateFR              ;                               //Steven 20240822 : For HT-9046AU

//Ztex 2023.04.13 Add HT-1032 IO ==>
extern const int C_LoadTrackFloodgate ;
extern const int C_EmptyTrackFloodgate;
extern const int C_ColorTrackFloodgate;
extern const int C_Auto1TrackFloodgate;
extern const int C_Auto2TrackFloodgate;
extern const int C_Auto3TrackFloodgate;
extern const int C_SafeDoor1Lock;
extern const int C_SafeDoor2Lock;
extern const int C_SafeDoor3Lock;
extern const int C_SafeDoor4Lock;
extern const int C_SafeDoor5Lock;
extern const int C_SafeDoor6Lock;
extern const int C_SafeDoor7Lock;
extern const int C_SafeDoor8Lock;
extern const int C_Shuttle1Floodgate;
extern const int C_Shuttle2Floodgate;
extern const int C_OutShuttle1Floodgate;                                        //Ifor 20240620 add:Out Shuttle Floodgate
extern const int C_OutShuttle2Floodgate;                                        //Ifor 20240620 add:Out Shuttle Floodgate
extern const int C_OutArmSmallY;                                                //AI(W906-HT9050-RULE10) 20261004: cmydef.cpp
//Ztex 2023.04.13 Add HT-1032 IO <==

extern const int C_LoaderPushBack_Push      ;
extern const int C_EmptyPushBack_Push       ;
extern const int C_ColorPushBack_Push       ;
extern const int C_Auto1PushBack_Push       ;
extern const int C_Auto2PushBack_Push       ;
extern const int C_Auto3PushBack_Push       ;
extern const int C_LoaderSeparate      ;
extern const int C_EmptySeparate       ;
extern const int C_ColorSeparate       ;
extern const int C_Auto1Separate       ;
extern const int C_Auto2Separate       ;
extern const int C_Auto3Separate       ;

extern const int C_UnderTrayArmYCatch  ;

//Ztex 2023.04.26 Add HT-1032 IO Exhaust Air ==>
extern const int C_EnhaustAirVentOpen ;
extern const int C_EnhaustAirVentClose;
extern const int C_LUpEnhaustAirOpen   ;
extern const int C_LUpEnhaustAirClose  ;
extern const int C_RUpEnhaustAirOpen  ;
extern const int C_RUpEnhaustAirClose ;
//Ztex 2023.04.26 Add HT-1032 IO Exhaust Air <==

extern const int C_CatchMagazineTray      ;  //JerryYang 20220909 : add magazine
extern const int C_CatchMagazineTray1     ;
extern const int C_Auto3BlockZ            ;
extern const int C_MagYTrayOut            ;
extern const int C_TrayXFloodgate1      ;
extern const int C_TrayXFloodgate2      ;
extern const int C_TrayXFloodgate3      ;
extern const int C_TrayXFloodgate4      ;
extern const int C_FixTray_UpDown       ;
extern const int C_LoaderPushBack_Back      ;
extern const int C_EmptyPushBack_Back       ;
extern const int C_ColorPushBack_Back       ;
extern const int C_Auto1PushBack_Back       ;
extern const int C_Auto2PushBack_Back       ;
extern const int C_Auto3PushBack_Back       ;

extern const int C_Auto4Side_Fixer             ;
extern const int C_Auto5Side_Fixer             ;
extern const int C_Auto6Side_Fixer             ;
extern const int C_Auto4_Selector              ;           //Auto 4上頂汽缸 或 中間分離
extern const int C_Auto5_Selector              ;           //Auto 5上頂汽缸 或 中間分離
extern const int C_Auto6_Selector              ;           //Auto 6上頂汽缸 或 中間分離
extern const int C_Auto4EdgePush               ;
extern const int C_Auto5EdgePush               ;
extern const int C_Auto6EdgePush               ;
extern const int C_Auto4_Up                    ;           //Auto 4上頂汽缸
extern const int C_Auto5_Up                    ;           //Auto 5上頂汽缸
extern const int C_Auto6_Up                    ;           //Auto 6上頂汽缸
extern const int C_Auto4LoaderZ_Select         ;           //Auto 4分離汽缸
extern const int C_Auto5LoaderZ_Select         ;           //Auto 5分離汽缸
extern const int C_Auto6LoaderZ_Select         ;           //Auto 6分離汽缸
extern const int C_Fix4LoaderZ_Select          ;           //Fix 4分離汽缸
extern const int C_Fix5LoaderZ_Select          ;           //Fix 5分離汽缸
extern const int C_Fix6LoaderZ_Select          ;           //Fix 6分離汽缸
extern const int C_Auto4UpPress                ;           //Auto 4unloader壓tray
extern const int C_Auto5UpPress                ;           //Auto 5unloader壓tray
extern const int C_Auto6UpPress                ;           //Auto 6unloader壓tray
extern const int C_Auto4CasstteLock            ;
extern const int C_Auto5CasstteLock            ;
extern const int C_Auto6CasstteLock            ;
extern const int C_Auto4TrackFloodgate         ;
extern const int C_Auto5TrackFloodgate         ;
extern const int C_Auto6TrackFloodgate         ;
extern const int C_Auto4PushBack_Push          ;
extern const int C_Auto5PushBack_Push          ;
extern const int C_Auto6PushBack_Push          ;
extern const int C_Auto4Separate               ;
extern const int C_Auto5Separate               ;
extern const int C_Auto6Separate               ;
extern const int C_Auto4PushBack_Back          ;
extern const int C_Auto5PushBack_Back          ;
extern const int C_Auto6PushBack_Back          ;
extern const int C_FixedSeatTL                 ;            //Jimmychiu 20240322 : Top & Bottom Inspect
extern const int C_FixedSeatTR                 ;
extern const int C_FixedSeatBL                 ;
extern const int C_FixedSeatBR                 ;
extern const int C_TopBtmRotateLock            ;
extern const int C_LoaderSeparateRL            ;
extern const int C_LoaderSeparateRR            ;
extern const int C_LoaderSeparateFL            ;
extern const int C_LoaderSeparateFR            ;
extern const int C_EmptySeparateRL             ;
extern const int C_EmptySeparateRR             ;
extern const int C_EmptySeparateFL             ;
extern const int C_EmptySeparateFR             ;
extern const int C_ColorSeparateRL             ;
extern const int C_ColorSeparateRR             ;
extern const int C_ColorSeparateFL             ;
extern const int C_ColorSeparateFR             ;
extern const int C_Auto1SeparateRL             ;
extern const int C_Auto1SeparateRR             ;
extern const int C_Auto1SeparateFL             ;
extern const int C_Auto1SeparateFR             ;
extern const int C_Auto2SeparateRL             ;
extern const int C_Auto2SeparateRR             ;
extern const int C_Auto2SeparateFL             ;
extern const int C_Auto2SeparateFR             ;
extern const int C_Auto3SeparateRL             ;
extern const int C_Auto3SeparateRR             ;
extern const int C_Auto3SeparateFL             ;
extern const int C_Auto3SeparateFR             ;
extern const int C_EmptyEdgePush               ;
extern const int C_ColorEdgePush               ;

extern const int C_Auto4SeparateRL             ;
extern const int C_Auto4SeparateRR             ;
extern const int C_Auto4SeparateFL             ;
extern const int C_Auto4SeparateFR             ;
extern const int C_Auto5SeparateRL             ;
extern const int C_Auto5SeparateRR             ;
extern const int C_Auto5SeparateFL             ;
extern const int C_Auto5SeparateFR             ;
extern const int C_Auto6SeparateRL             ;
extern const int C_Auto6SeparateRR             ;
extern const int C_Auto6SeparateFL             ;
extern const int C_Auto6SeparateFR             ;
extern const int C_LoadCarRFIDRotArmD          ;                                //RogerYang 20250828 add for Loader Rotate Arm
extern const int C_LoadCarRFIDRotArmU          ;                                //RogerYang 20250828 add for Loader Rotate Arm
extern const int C_LoadTrayDetD                ;                                //RogerYang 20250828 add for 殘料檢氣缸
extern const int C_LoadTrayDetU                ;                                //RogerYang 20250828 add for 殘料檢氣缸
extern const int C_LoadTrayDetF                ;                                //RogerYang 20250828 add for 殘料檢氣缸
extern const int C_LoadTrayDetB                ;                                //RogerYang 20250828 add for 殘料檢氣缸

extern const int C_LoaderCarrier               ;                                //Ifor 20251216 add:Boat Carrier
extern const int C_Auto1Carrier                ;                                //Ifor 20251216 add:Boat Carrier
extern const int C_Auto2Carrier                ;                                //Ifor 20251216 add:Boat Carrier

#define CynForHome 82       //記得改!!   //AI(W906-DROPCYL) 20261005: NB2-1 R229 (W-64) -- 74 -> 82: + C_InPnPDrop1-4, C_OutPnPDrop1-4 (cmydef.cpp CynNeedHome)
extern int CynNeedHome[CynForHome];                                             //Steven 20240123 : 改用enable確認氣缸是否要復歸
//Steven 20230907 : For HT-9011UC
//==>
extern int C_AutoSide_Fixer    [MAX_AUTO_TRAY];
extern int C_Auto_Selector     [MAX_AUTO_TRAY];
extern int C_AutoEdgePush      [MAX_AUTO_TRAY];
extern int C_Auto_Up           [MAX_AUTO_TRAY];
extern int C_AutoZ_Select      [MAX_AUTO_TRAY];
extern int C_AutoUpPress       [MAX_AUTO_TRAY];
extern int C_AutoCasstteLock   [MAX_AUTO_TRAY];
extern int C_AutoTrackFloodgate[MAX_AUTO_TRAY];
extern int C_AutoPushBack_Push [MAX_AUTO_TRAY];
extern int C_AutoSeparate      [MAX_AUTO_TRAY];
extern int C_AutoPushBack_Back [MAX_AUTO_TRAY];
extern int C_FixLoaderZ_Select [MAX_AUTO_TRAY];

extern int C_AutoCarrier       [2];                                             //RogerYang 20260202 : Add for CR

extern AnsiString sJAM1101     [MAX_AUTO_TRAY];
extern AnsiString sJAM1102     [MAX_AUTO_TRAY];
extern AnsiString sJAM1103     [MAX_AUTO_TRAY];
extern AnsiString sJAM1104     [MAX_AUTO_TRAY];
extern AnsiString sJAM1106     [MAX_AUTO_TRAY];
extern AnsiString sJAM1107     [MAX_AUTO_TRAY];
extern AnsiString sJAM1108     [MAX_AUTO_TRAY];
extern AnsiString sJAM1109     [MAX_AUTO_TRAY];
extern AnsiString sJAM1110     [MAX_AUTO_TRAY];
extern AnsiString sJAM1111     [MAX_AUTO_TRAY];
extern AnsiString sJAM1112     [MAX_AUTO_TRAY];
extern AnsiString sJAM1113     [MAX_AUTO_TRAY];
extern AnsiString sJAM1114     [MAX_AUTO_TRAY];
extern AnsiString sJAM1158     [MAX_AUTO_TRAY];                                 //AI(general) 20260323 (RogerYang) : UpSafe sensor 未到位5秒 Alarm
extern AnsiString sMES1120     [MAX_AUTO_TRAY];
extern AnsiString sMES1121     [MAX_AUTO_TRAY];
extern AnsiString sMES1122     [MAX_AUTO_TRAY];
extern AnsiString sMES1123     [MAX_AUTO_TRAY];
extern AnsiString sWAR1130     [MAX_AUTO_TRAY];
extern AnsiString sWAR1151     [MAX_AUTO_TRAY];
extern AnsiString sJAM1170     [MAX_AUTO_TRAY];
extern AnsiString sMES1712     [MAX_FIX_TRAY];
extern AnsiString sMES1713     [MAX_FIX_TRAY];                                  //RogerYang 20250626 偉測不可複測bin功能
extern AnsiString sMES1720     [MAX_FIX_TRAY];
extern AnsiString sMES1721     [MAX_FIX_TRAY];
extern AnsiString sWAR1722     [MAX_FIX_TRAY];
extern AnsiString sMES1723     [MAX_FIX_TRAY];
extern AnsiString sWAR1751     [MAX_FIX_TRAY];
extern AnsiString sWAR1752     [MAX_FIX_TRAY];

extern int iC_Up          [MAX_TRACK];
extern int iC_Middle      [MAX_TRACK];
extern int iC_EdgePush    [MAX_TRACK];
extern int iTrackFloodgate[MAX_TRACK];
extern int iAutoBack      [MAX_TRACK];
extern int iAutoPush      [MAX_TRACK];
//<==
//Steven 20230907 : For HT-9011UC
extern const int C_DailyCorrelation   ;     //KaiChen 20200525 ：Daily Correlation Function
//====================================================================
extern const int SnFKPowerOff           ;
extern const int SnFKPowerOn            ;
extern const int SnFKReset              ;
extern const int SnFKPause              ;
extern const int SnFKHome               ;
extern const int SnFKStart              ;
extern const int SnFKOneCycle           ;
extern const int SnFKRetry              ;

extern const int SnFKSkip               ;
extern const int SnFKCleanOut           ;
extern const int SnFKTrayFeed           ;
extern const int SnFKTrayEnd            ;
extern const int SnFKAlarmReset         ;
extern const int SnFKCoverOpen          ;
extern const int SnRKPowerOff           ;
extern const int SnRKPowerOn            ;

extern const int SnRKReset              ;
extern const int SnRKPause              ;
extern const int SnRKHome               ;
extern const int SnRKStart              ;
extern const int SnRKOneCycle           ;
extern const int SnRKRetry              ;
extern const int SnRKSkip               ;
extern const int SnRKCleanOut           ;

extern const int SnRKTrayFeed           ;
extern const int SnRKTrayEnd            ;
extern const int SnRKAlarmReset         ;
extern const int SnRKCoverOpen          ;
extern const int SnRKManualStep         ;
extern const int SnRKManualTStart       ;
//-----------------------------------------panel sensor finish
extern const int SnLoaderTrayHasTray    ;
extern const int SnLoaderCarHasTray     ;

extern const int SnLoaderPreDete        ;

extern const int SnAuto1TrayDetect      ;
extern const int SnAuto2TrayDetect      ;
extern const int SnAuto3TrayDetect      ;

extern const int SnAuto1IsFull          ;
extern const int SnAuto2IsFull          ;
extern const int SnAuto3IsFull          ;
extern const int SnFixedTray1Detect     ;
extern const int SnFixedTray2Detect     ;
extern const int SnFixedTray3Detect     ;
extern const int SnSafeDoor1            ;
extern const int SnSafeDoor2            ;

extern const int SnSafeDoor3            ;
extern const int SnSafeDoor4            ;
extern const int SnSafeDoor5            ;
extern const int SnSafeDoor6            ;
extern const int SnSafeDoor7            ;
extern const int SnSafeDoor8            ;
extern const int SnAirIsEnough          ;
extern const int SnFrontRightEMG        ;

extern const int SnRearLeftEMG          ;
extern const int SnFMotorDown           ;
extern const int SnBMotorDown           ;
extern const int SnMotorPower           ;
extern const int SnSystemPower          ;
extern const int SnEmptyTrayHasTray1    ;
extern const int SnEmptyTrayIsFull1     ;
extern const int SnEmptyTrayIsLock1     ;

extern const int SnEmptyTrayHasTray2    ;
extern const int SnEmptyTrayIsFull2     ;
extern const int SnRearPadActive        ;
extern const int SnFrontLeftEMG         ;
extern const int SnRearRightEMG         ;
extern const int SnEPDieForce           ;
//extern const int SnCatchTrayDown        ;

extern const int SenBit0               ;
extern const int SenBit1               ;
extern const int SenBit2               ;
extern const int SenBit3               ;
extern const int SenBit4               ;
extern const int SenBit5               ;
extern const int SenBit6               ;
extern const int SenBit7               ;
extern const int SenBit8               ;
extern const int SenBit9               ;

extern const int SnAuto1_Tray_Car      ;
extern const int SnAuto2_Tray_Car      ;
extern const int SnAuto3_Tray_Car      ;

extern const int SnInPutSHT1S1           ;
extern const int SnInPutSHT1S2           ;
extern const int SnInPutSHT1S3           ;
extern const int SnInPutSHT1S4           ;
extern const int SnInPutSHT1S5           ;
extern const int SnInPutSHT1S6           ;
extern const int SnInPutSHT1S7           ;

extern const int SnInPutSHT2S1           ;
extern const int SnInPutSHT2S2           ;
extern const int SnInPutSHT2S3           ;
extern const int SnInPutSHT2S4           ;
extern const int SnInPutSHT2S5           ;
extern const int SnInPutSHT2S6           ;
extern const int SnInPutSHT2S7           ;

extern const int SnOutPutSHT1S1          ;
extern const int SnOutPutSHT1S2          ;
extern const int SnOutPutSHT1S3          ;
extern const int SnOutPutSHT1S4          ;
extern const int SnOutPutSHT1S5          ;
extern const int SnOutPutSHT1S6          ;
extern const int SnOutPutSHT1S7          ;

extern const int SnOutPutSHT2S1          ;
extern const int SnOutPutSHT2S2          ;
extern const int SnOutPutSHT2S3          ;
extern const int SnOutPutSHT2S4          ;
extern const int SnOutPutSHT2S5          ;
extern const int SnOutPutSHT2S6          ;
extern const int SnOutPutSHT2S7          ;

extern const int SnOutPutSHT1ZS1         ;
extern const int SnOutPutSHT1ZS2         ;
extern const int SnOutPutSHT2ZS1         ;
extern const int SnOutPutSHT2ZS2         ;

extern const int SnAuto1PreDete        ;
extern const int SnAuto2PreDete        ;
extern const int SnAuto3PreDete        ;
extern const int SnLoaderSureTray      ;
//extern const int SnMotorYAlarm         ;
//extern const int SnMotorZ1Alarm        ;
//extern const int SnMotorZ2Alarm        ;
extern const int SnAuto1FixCyPush      ;
extern const int SnAuto2FixCyPush      ;
extern const int SnAuto3FixCyPush      ;
extern const int SnLoaderFixCyPush     ;

extern const int SenBit10              ;
extern const int SenBit11              ;
extern const int SenBit12              ;
extern const int SenBit13              ;
extern const int SenBit14              ;
extern const int SenBit15              ;
extern const int SenBit16              ;
extern const int SenBit17              ;
extern const int SenBit18              ;
extern const int SenBit19              ;

extern const int SenEmptyHasTray                ;
extern const int SenEmptyCWDete                 ;
extern const int SenEmptySelectHasTray          ;
extern const int SenEmptyCCWDete                ;
extern const int SenEmptyCarHasTray             ;
extern const int SenColorHasTray                ;
extern const int SenColorCWDete                 ;
extern const int SenColorSelectHasTray          ;
extern const int SenColorCarHasTray             ;
extern const int SenEmptyFixCyPush              ;

extern const int SnAuto1TrayHasTray    ;
extern const int SnAuto1CWPreDetect    ;
extern const int SnHeaterDoor          ;
extern const int SnCatchTrayFix1On     ;
extern const int SnCatchTrayFix2On     ;

extern const int SnSafeLock            ;
extern const int SnRKSafeLock          ;                                        //KenHsieh 20211228 : 區分實體IO與通訊面板

extern const int SnFPLevelOpe          ;
extern const int SnFPLevelEng          ;                                        //Steven 20190503 : 指紋辨識權限
extern const int SnFPLevelSup          ;
extern const int SnFPLevelHon          ;

extern const int SnIndexHeaterFan      ;
extern const int SnHeaterDoor2         ;
extern const int SnHeaterDoor3         ;
extern const int SnHeaterDoor4         ;
extern const int SnChamberHeatDetect   ;                                        //JerryYang 20210107 : ChamberHeatDetect
extern const int SnAuto1TrackDetect    ;
extern const int SnAuto2TrackDetect    ;
extern const int SnAuto3TrackDetect    ;
extern const int SnEmptyIsFull         ;
extern const int SnColorIsFull         ;
extern const int SenColorCCWDete       ;
extern const int SenColorFixCyPush     ;

extern const int SenEmpty1HasTray      ;
extern const int SenEmpty1CCWDete      ;
extern const int SenEmpty1CarHasTray   ;

extern const int SnLoaderUpSafedetect  ;
extern const int SnEmptyUpSafedetect   ;
extern const int SnColorUpSafedetect   ;
extern const int SnAutoUpSafedetect0   ;
extern const int SnAutoUpSafedetect1   ;
extern const int SnAutoUpSafedetect2   ;

extern const int SnIonFanAlarm         ;
extern const int SnIonFanLevelAlarm    ;
extern const int SnIonBarrierAlarm     ;
extern const int SnIonBarrierLevelAlarm;
extern const int SnIonBarrierConditionAlarm;

extern const int SnAutoColorTrayDetect0;
extern const int SnAutoColorTrayDetect1;
extern const int SnAutoColorTrayDetect2;

extern const int SnLoad2IsFull            ;
extern const int SnLoad2IsPreAlarm        ;
extern const int SnLoad2TrayHasTray_AGV   ;
extern const int SnLoad2TrayHasTray_ART   ;
extern const int SnLoad2TrayHasTray       ;
extern const int SnLoad2CarHasTray        ;
extern const int SnCheckTray2Direction    ;
extern const int SenLoad2CCWDete          ;
extern const int SnLoad2UpSafedetect      ;
extern const int SenLoad2CCWDete_2        ;
extern const int SnLoad2TrackDetect       ;
extern const int SnLoad2FixCyPush         ;
extern const int SnLoad2EdgePush          ;
extern const int SnLoad2SeparateHasTray   ;
extern const int SnLoad2SureTray          ;
extern const int SnLoad2PreDete           ;
extern const int SnCheckLoad2Direction    ;
extern const int SnLoad2CasstteDetect     ;
extern const int SnDoubleLoad2Detection   ;
extern const int SnLoad2UpPress           ;

extern const int SnSafeDoor9;

extern const int SnInPutSHT1S8;
extern const int SnInPutSHT1S9;
extern const int SnInPutSHT2S8;
extern const int SnInPutSHT2S9;
extern const int SnSafeDoor10;

extern const int SnIonFan6Alarm;
extern const int SnIonFan7Alarm;
extern const int SnIonFan8Alarm;
extern const int SnIonFan9Alarm;
extern const int SnIonFan10Alarm;
extern const int SnIonFan11Alarm;

extern const int SnEPAlarm;
extern const int SnCheckTrayDirection;
extern const int SnCheckLoadDirection;

extern const int SnNegativePressureAir;
extern const int SnNegativePressureAir2;                                        //Sam 20171110 (Steven) : 新增氣壓 Sensor
//jou 2010-11-23
extern const int SnLoaderEdgePush ;
extern const int SnAuto1EdgePush  ;
extern const int SnAuto2EdgePush  ;
extern const int SnAuto3EdgePush  ;
//----- by dell ccd realtime-------------
extern const int SnRealTimeCCDStop;
extern const int SnRealTimeCCDIndexArm;
extern const int SnRTCCDTempCtrl;
//---------------------------------------
extern const int SnUnLoaderFloating ;

extern const int SnRotateCheck;                                                 //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
extern const int SnCheckConnectIndexArm_1;                                      //20111130  Dell
extern const int SnCheckConnectIndexArm_2;                                      //20111130  Dell

extern const int SnFixFloating1;                                                //Steven 20120131 : Fix Tray置偏偵測
extern const int SnFixFloating2;
extern const int SnFixFloating3;                                                //Sam 20240129 : 新增第三組 Fix floating Sensor

extern const int SnATCAlarm1;                                                   //jou 2012-03-13 ATC Alarm 1 Sensor
extern const int SnATCAlarm2;                                                   //jou 2012-03-13 ATC Alarm 2 Sensor
extern const int SnATCAlarm3;                                                   //jou 2012-03-13 ATC Alarm 3 Sensor
extern const int SnATCAlarm4;                                                   //jou 2012-03-13 ATC Alarm 4 Sensor

extern const int SenInArmYPitch60;                                              //ChungHung 20120505 : HT9045 WS Only
extern const int SenOutArmYPitch60;                                             //ChungHung 20120505 : HT9045 WS Only
//------------------------------------------------------------------------------
// 2011.05.26 , Joye , ATC Alarm ---------->>
extern const int SnATC01ControllerHighAlarm;  //Steven 20120410 : Hontech ATC
extern const int SnATC02ControllerHighAlarm;
extern const int SnATC03ControllerHighAlarm;
extern const int SnATC04ControllerHighAlarm;
extern const int SnATC01ControllerLowAlarm;
extern const int SnATC02ControllerLowAlarm;
extern const int SnATC03ControllerLowAlarm;
extern const int SnATC04ControllerLowAlarm;
// 2011.05.26 , Joye , ATC Alarm ----------<<

extern const int SenLoaderCCWDete   ;
extern const int SnLoaderIsFull     ;
extern const int SnAuto2CWPreDetect ;
extern const int SnAuto3CWPreDetect ;
extern const int SnAuto2TrayHasTray ;
extern const int SnAuto3TrayHasTray ;
extern const int SnAutoDockingOff;                                              //ChungHung 20120718 add UseAutoDocking Check Sensor
extern const int SnAutoDockingOn;                                               //ChungHung 20120718 add UseAutoDocking Check Sensor

extern const int SnTesterDocking;                                               //jou 2012-09-13 Tester Docking
extern const int SnTrain;                                                       //ChungHung 20120911 add
extern const int SnFix3FullPlace;                                               //Steven 20121020 : Fix3滿盤

//Steven 20130201 : Kasuga離子風扇電源偵測
extern const int SnIonFanPower01;
extern const int SnIonFanPower02;
extern const int SnIonFanPower03;
extern const int SnIonFanPower04;
extern const int SnIonFanPower05;
extern const int SnIonFanPower06;
extern const int SnIonFanPower07;
extern const int SnIonFanPower08;
extern const int SnIonFanPower09;
extern const int SnIonFanPower10;
extern const int SnIonFanPower11;
extern const int SnIonFanPower12;

extern const int SnIonFan12Alarm;
extern const int SnRotateRowIn1;                                                //kevin 20130524  Dell :旋轉站;馬達版
extern const int SnRotateRowIn2;
extern const int SnRotateRowOut1;
extern const int SnRotateRowOut2;

extern const int SnSocket1;                                                     //kevin 20130429  socket sensor
extern const int SnSocket2;
extern const int SnSocket3;
extern const int SnSocket4;
extern const int SnSocket5;
extern const int SnSocket6;
extern const int SnSocket7;
extern const int SnSocket8;

extern const int SnSocket9;                                                     //Steven 20200610 : Socket sensor 改成16顆
extern const int SnSocket10;
extern const int SnSocket11;
extern const int SnSocket12;
extern const int SnSocket13;
extern const int SnSocket14;
extern const int SnSocket15;
extern const int SnSocket16;

#define iSnSocketCnt          24                                                //JerryYang 20260506 : 16->24

extern const int SnCrossSHT1S1;                                                 //2013-07-16    Dell    Shuttle cross sensor
extern const int SnCrossSHT1S2;
extern const int SnCrossSHT2S1;
extern const int SnCrossSHT2S2;
extern const int SnServo;                                                       //kevin 20140121 偵測servon 訊號
extern const int SnAutoTeach;                                                   //kevin 201400512 AUTOTEACH IN/OUT ARM SENSOR
extern const int SnEOF1      ;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern const int SnEOF2      ;
extern const int SnEOF3      ;
extern const int SnAOIResult1;
extern const int SnAOIResult2;
extern const int SnAOIResult3;
extern const int SnFix3Lock;                                                    //Steven 20140512 : For HT-9046LA

extern const int SenAutoDocking240KG;                                           //ChungHung 20140709 add for hardware control OTD
extern const int SenAutoDocking360KG;                                           //ChungHung 20140709 add for hardware control OTD

extern const int SnFixColorTrayDetect0;                                         //20140903 wei colcr Tray
extern const int SnFixColorTrayDetect1;
extern const int SnFixColorTrayDetect2;

extern const int SnLoaderColorTrayDetect;
extern const int SnEmptyColorTrayDetect;
extern const int SnColorColorTrayDetect;

extern const int SnGroundMan;                                                   //wei 20150424 add SnGroundMan偵測
extern const int SnOCRTigger;                                                   //wei 20150720 OCR觸發
extern const int SnLowSpeed;
extern const int SnOCRPosition;
extern const int SnLoaderTrayHasTray_ART;                                       //wei 20151210
extern const int SnSLK1UnclampButton;                                           //JerryYang 20160524
extern const int SnSLK2UnclampButton;
extern const int SnSocketClampPush1;                                            //JerryYang 20160606
extern const int SnSocketClampPush2;
extern const int SnSocketClampPull1;
extern const int SnSocketClampPull2;
extern const int SnSocketHasClamp1;
extern const int SnSocketHasClamp2;
extern const int SnLoaderUpPress;                                               //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray

extern const int SnSafeMode;                                                    //jou 20231016 : CE PLC safe mode
extern const int SnWaterLeakageChiller;                                         //jou 20231019 : Water Leakage Chiller
extern const int SnSmokeDetect01;                                               //Sam 20240112 : 新增煙霧偵測
extern const int SnTopBtmAOIR180;                                               //Jimmychiu 20240322 : Top & Bottom Inspect
extern const int SnFixedSeatTLOn;
extern const int SnFixedSeatTLOff;
extern const int SnFixedSeatTROn;
extern const int SnFixedSeatTROff;
extern const int SnFixedSeatBLOn;
extern const int SnFixedSeatBLOff;
extern const int SnFixedSeatBROn;
extern const int SnFixedSeatBROff;
extern const int SnTopBtmAirMaxAlarm;
extern const int SnTopBtmAirMinAlarm;
extern const int SnTopBtmRotateLockOn1;
extern const int SnTopBtmRotateLockOn2;
extern const int SnLightZORG;
extern const int SnLightZINP;
extern const int SnLightZREADY;
extern const int SnLightZSERVO;
//------------------------------------
//Steven 20161011 : TTL支援8Site
//------------------------------------
extern const int SenBit20              ;
extern const int SenBit21              ;
extern const int SenBit22              ;
extern const int SenBit23              ;
extern const int SenBit24              ;
extern const int SenBit25              ;
extern const int SenBit26              ;
extern const int SenBit27              ;
extern const int SenBit28              ;
extern const int SenBit29              ;
extern const int SenBit30              ;
extern const int SenBit31              ;
extern const int SenBit32              ;
extern const int SenBit33              ;
extern const int SenBit34              ;
extern const int SenBit35              ;
extern const int SenBit36              ;
extern const int SenBit37              ;
extern const int SenBit38              ;
extern const int SenBit39              ;

extern const int SnIndex1Connect1     ;                                         //RogerYang 20161212 : 偵測SLK獨立加熱或共用加熱
extern const int SnIndex1Connect2     ;
extern const int SnIndex2Connect1     ;
extern const int SnIndex2Connect2     ;
extern const int SnTrayCover          ;                                         //Steven 20170623 (wei) : Add for catch tray with cover

extern const int SnLoaderIsPreAlarm   ;                                         //wei 20170802 Pre alarm sensor
extern const int SnEmptyIsPreAlarm    ;
extern const int SnColorIsPreAlarm    ;
extern const int SnAuto1IsPreAlarm    ;
extern const int SnAuto2IsPreAlarm    ;
extern const int SnAuto3IsPreAlarm    ;
extern const int SnTrayArmSafePos     ;                                         //kevin 20171006 (wei) Home tray arm must on
extern const int SnTJCurrent          ;                                         //Steven 20180124 : Check ATC7.0 TJ Current
extern const int SnEmptyFull          ;                                         //wei 20170504 Use Empty Full Put Color
extern const int SnCassetteArmHave    ;                                         //wei 20180702 MR
extern const int SnBuffer1HaveCassette;
extern const int SnBuffer2HaveCassette;
extern const int SnBuffer3HaveCassette;
extern const int SnBuffer4HaveCassette;
extern const int SnBuffer5HaveCassette;
extern const int SnBuffer6HaveCassette;
extern const int SnBuffer7HaveCassette;
extern const int SnBuffer8HaveCassette;
extern const int SnBuffer9HaveCassette;
extern const int SnBuffer10HaveCassette;
extern const int SnLoadPortCatch       ;
extern const int SnCassetteCatch       ;

extern const int SnLoadPortPresent     ;
extern const int SnLoadPortPlacement1  ;
extern const int SnLoadPortPlacement2  ;
extern const int SnTrayBracketHave     ;
extern const int SnStackedTrayHave     ;
extern const int SnBuffer6HaveTray     ;

extern const int SnE84VALID            ;                                        //wei 20180702 E84
extern const int SnE84CS0              ;
extern const int SnE84CS1              ;
extern const int SnE84AMAVBL           ;
extern const int SnE84TRREQ            ;
extern const int SnE84BUSY             ;
extern const int SnE84COMPT            ;
extern const int SnE84CONT             ;
extern const int SnE84GO               ;

extern const int SnHingeTopTray        ;                                        //wei 20170405
extern const int SnSafeDoor11          ;
extern const int SnSafeDoor12          ;
extern const int SnSafeDoor13          ;
extern const int SnSafeDoor14          ;
extern const int SnSafeDoor15          ;

extern const int SnCassette01          ;                                        //wei 20180702 MR
extern const int SnCassette02          ;
extern const int SnCassette03          ;
extern const int SnCassette04          ;
extern const int SnCassette05          ;
extern const int SnCassette06          ;
extern const int SnCassette07          ;
extern const int SnCassette08          ;
extern const int SnCassette09          ;
extern const int SnCassette10          ;
extern const int SnMRStart             ;
extern const int SnMRPause             ;
extern const int SnMRUp                ;
extern const int SnMRDown              ;

//Sam 20190112 LM
//==>
extern const int SnCassetteArmCatch  ;
extern const int SnLoadPortHaveCassette1_1  ;
extern const int SnLoadPortHaveCassette1_2  ;
extern const int SnLoadPortHaveCassette2_1  ;
extern const int SnLoadPortHaveCassette2_2  ;
extern const int SnLoadPortHaveCassette3_1  ;
extern const int SnLoadPortHaveCassette3_2  ;
extern const int SnLoadPortHaveCassette4_1  ;
extern const int SnLoadPortHaveCassette4_2  ;
extern const int SnLoadRobotHaveCassette1   ;
extern const int SnLoadRobotHaveCassette2   ;
extern const int SnLoadRobotPosUp           ;
extern const int SnLoadRobotPosDown         ;

extern const int SnUnloadPortHaveCassette1_1;
extern const int SnUnloadPortHaveCassette1_2;
extern const int SnUnloadPortHaveCassette2_1;
extern const int SnUnloadPortHaveCassette2_2;
extern const int SnUnloadPortHaveCassette3_1;
extern const int SnUnloadPortHaveCassette3_2;
extern const int SnUnloadPortHaveCassette4_1;
extern const int SnUnloadPortHaveCassette4_2;
extern const int SnUnloadRobotHaveCassette1 ;
extern const int SnUnloadRobotHaveCassette2 ;
extern const int SnUnloadRobotPosUp         ;
extern const int SnUnloadRobotPosDown       ;

extern const int SnSafeDoor16               ;
extern const int SnSafeDoor17               ;
extern const int SnSafeDoor18               ;
extern const int SnSafeDoor19               ;
//<==
//Sam 20190112 LM

extern const int SnPreciserDetect1    ;     //Frank 20180410 (Steven) : InArm Preciser Station
extern const int SnPreciserDetect2    ;
extern const int SnLoadCell1          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell2          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell3          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell4          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell5          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell6          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell7          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell8          ;    //kevin 20190307  read load cell pass  or fail

extern const int SnHotGun1;                 //kevin 20190621  hot gun 流量異常 使用 Out 2
extern const int SnHotGun2;                 //kevin 20190621  hot gun 流量異常 使用 Out 2

extern const int SnGroundMan2         ;
extern const int SnGroundMan3         ;
extern const int SnGroundMan4         ;
extern const int SnAuto1UpPress       ;
extern const int SnAuto2UpPress       ;
extern const int SnAuto3UpPress       ;
extern const int SnWaterLeakageUp     ;     //wei 20190617 漏水檢測
extern const int SnWaterLeakageDown   ;     //wei 20190617 漏水檢測
extern const int SnWaterLeakagePlate  ;     //wei 20190617 漏水檢測
extern const int SnLoaderTrackDetect  ;     //Sam 20200316 : Loader Detect Tray
extern const int SnAuto1Z_Select1     ;    //JerryYang 20200615 ART分離汽缸sensor
extern const int SnAuto1Z_Select2     ;
extern const int SnAuto1Z_Select3     ;
extern const int SnAuto1Z_Select4     ;
extern const int SnAuto2Z_Select1     ;
extern const int SnAuto2Z_Select2     ;
extern const int SnAuto2Z_Select3     ;
extern const int SnAuto2Z_Select4     ;
extern const int SnAuto3Z_Select1     ;
extern const int SnAuto3Z_Select2     ;
extern const int SnAuto3Z_Select3     ;
extern const int SnAuto3Z_Select4     ;
extern const int SnTesterDryAir       ;   //Ifor 20200115 add: add Tester Dry Air Control

extern const int SnInFlipper1DeviceDetect ;
extern const int SnInFlipper2DeviceDetect ;
extern const int SnInFlipper3DeviceDetect ;

extern const int SnOutFlipper1DeviceDetect ;
extern const int SnOutFlipper2DeviceDetect ;
extern const int SnOutFlipper3DeviceDetect ;

extern const int SnLoaderCasstteDetect    ;
extern const int SnEmptyCasstteDetect     ;
extern const int SnColorCasstteDetect     ;
extern const int SnAuto1CasstteDetect     ;
extern const int SnAuto2CasstteDetect     ;
extern const int SnAuto3CasstteDetect     ;
extern const int SnAuto4CasstteDetect     ;                                       //RogerYang 20250825 : 新增Auto4~6
extern const int SnAuto5CasstteDetect     ;
extern const int SnAuto6CasstteDetect     ;
extern const int SnArm1SLK                ;
extern const int SnArm2SLK                ;

extern const int SnMRAuto21     ;   //wei 20180702 MR
//E84驗證用
extern const int SnE84LREQ      ;   //wei 20180702 E84
extern const int SnE84UREQ      ;   //wei 20180702 E84
extern const int SnE84VA        ;   //wei 20180702 E84
extern const int SnE84READY     ;   //wei 20180702 E84
extern const int SnE84VS0       ;   //wei 20180702 E84
extern const int SnE84VS1       ;   //wei 20180702 E84
extern const int SnE84HOAVBL    ;   //wei 20180702 E84
extern const int SnE84ES        ;   //wei 20180702 E84
extern const int SnE84POWER     ;   //wei 20180702 E84
extern const int SnBufferTop    ;   //wei 20200305 MR
extern const int SnBufferCatch  ;   //wei 20200305 MR
extern const int SnCassetteOpen ;
//KaiChen 20200716 ：OHT
//==>
extern const int SnMultileEmpty_Door               ;
extern const int SnMultileEmpty_MagazineTop        ;
extern const int SnMultileEmpty_MagazineLow        ;
extern const int SnMultileEmpty_MagazineCatch      ;
extern const int SnMultileEmpty_SelectHasTray      ;
extern const int SnMultileEmpty_HasTray            ;
extern const int SnMultileEmpty_CornerPushCyPush   ;
extern const int SnMultileEmpty_CatchHasTray       ;
extern const int SnSafeDoor20                      ;
extern const int SnSafeDoor21                      ;
extern const int SnLoadPort2Present                ;
extern const int SnLoadPort2Placement1             ;
extern const int SnLoadPort2Placement2             ;
extern const int SnTrayBracket2Have                ;
extern const int SnCassetteArmIntoLoadPort         ;
extern const int SnCassetteArmIntoLoadPort2        ;
extern const int SnBuffer7HaveTray                 ;
extern const int SnLoadPortAutoManual              ;
extern const int SnLoadPort2AutoManual             ;
extern const int SnOHTIntoLoadPort                 ;
extern const int SnOHTIntoLoadPort2                ;
extern const int SnCassetteClose                   ;
extern const int SnCassetteOpen2                   ;
extern const int SnCassetteClose2                  ;
extern const int SnCassetteSlotMap                 ;
extern const int SnCassetteFloating                ;
extern const int SnMultileEmpty_ScanTrayID         ;
extern const int SnE84_2_VALID              ;
extern const int SnE84_2_CS0                ;
extern const int SnE84_2_CS1                ;
extern const int SnE84_2_AMAVBL             ;
extern const int SnE84_2_TRREQ              ;
extern const int SnE84_2_BUSY               ;
extern const int SnE84_2_COMPT              ;
extern const int SnE84_2_CONT               ;
extern const int SnE84_2_GO                 ;
extern const int SnE84_1_VALID              ;
extern const int SnE84_1_CS0                ;
extern const int SnE84_1_CS1                ;
extern const int SnE84_1_AMAVBL             ;
extern const int SnE84_1_TRREQ              ;
extern const int SnE84_1_BUSY               ;
extern const int SnE84_1_COMPT              ;
extern const int SnE84_1_CONT               ;
extern const int SnE84_1_GO                 ;
extern const int SnMultileEmptyCatch_HasTray;
extern const int SnCSTHoldDownOff              ;
extern const int SnCSTHoldDown2Off             ;
extern const int SnMultileEmpty_ScanTray2DID   ;
extern const int SnDieDetect_Sh1               ;
extern const int SnDieDetect_Sh2               ;
extern const int SnBuffer1_Placement1          ;
extern const int SnBuffer2_Placement1          ;
extern const int SnBuffer3_Placement1          ;
extern const int SnBuffer4_Placement1          ;
extern const int SnBuffer5_Placement1          ;
extern const int SnBuffer6_Placement1          ;
extern const int SnBuffer7_Placement1          ;
extern const int SnBuffer8_Placement1          ;
extern const int SnBuffer9_Placement1          ;
extern const int SnBuffer10_Placement1         ;
extern const int SnStackedTrayYDetect_Loader   ;
extern const int SnStackedTrayYDetect_Elevator2;
extern const int SnMCUSensor1                 ;//Jimmychiu 20230630 : add color sensor MU-N in Loader
extern const int SnMCUSensor2                 ;//Jimmychiu 20230630 : add color sensor MU-N in Loader
extern const int SnMCUSensor3                 ;//Jimmychiu 20230630 : add color sensor MU-N in Loader
extern const int SnMCUSensor4                 ;//Jimmychiu 20230630 : add color sensor MU-N in Loader

extern const int SnSht1Left                   ;   //kevin 20220512 add SHUTTLE Left 位置偏移
extern const int SnSht1Right                  ;   //kevin 20220512 add SHUTTLE Left 位置偏移
extern const int SnSht2Left                   ;   //kevin 20220512 add SHUTTLE Left 位置偏移
extern const int SnSht2Right                  ;   //kevin 20220512 add SHUTTLE Left 位置偏移
extern const int SnLoaderTrayHasTray_AGV      ;   //kevin 20220520 add AGV load
extern const int SnEmptyTrayHasTray_AGV       ;   //kevin 20220520 add AGV Empty
extern const int SnColorTrayHasTray_AGV       ;   //kevin 20220520 add AGV Color
extern const int SnAseTrayBufferLeft          ;   //kevin 20220709 ASEKH 左邊放空TRAY
extern const int SnAseTrayBufferRight         ;   //kevin 20220709 ASEKH 右邊放空TRAY

extern const int SnDailyCorrelation_Open  ;   //KaiChen 20200525 ：Daily Correlation Function
extern const int SnDailyCorrelation_Close ;   //KaiChen 20200525 ：Daily Correlation Function
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//==>
//ChungHung 20210113 add for Alignment CCD start
extern const int SnInAreaAlignmentSenX ;
extern const int SnInAreaAlignmentSenY ;
extern const int SnOutAreaAlignmentSenX;
extern const int SnOutAreaAlignmentSenY;
//ChungHung 20210113 add for Alignment CCD end
//<==
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//Jimmychiu 20210902 add: ATC Winway IO ready
//==>
extern const int SnATC1Ready          ;
extern const int SnATC2Ready          ;
extern const int SnATC3Ready          ;
extern const int SnATC4Ready          ;
//<==
//Jimmychiu 20210902 add: ATC Winway IO ready

//Ztex 2023.04.13 Add HT-1032 IO ==>
extern const int SnPlate1TempOverDetect  ;
extern const int SnPlate2TempOverDetect  ;
extern const int SnShuttle1TempOverDetect;
extern const int SnShuttle2TempOverDetect;
extern const int SnHead1TempOverDetect   ;
extern const int SnHead2TempOverDetect   ;
extern const int SnHead5TempOverDetect   ;
extern const int SnHead6TempOverDetect   ;
extern const int SnHumidityAnomaly1Detect;
extern const int SnHumidityAnomaly2Detect;
extern const int SnHumidityAnomaly3Detect;
extern const int SnDryAirIsEnough        ;
extern const int SnIonBarInAirIsEnough   ;
extern const int SnIonBarOutAirIsEnough  ;
extern const int SnSafeDoor1Hatchway;
extern const int SnSafeDoor2Hatchway;
extern const int SnSafeDoor3Hatchway;
extern const int SnSafeDoor4Hatchway;
extern const int SnSafeDoor5Hatchway;
extern const int SnSafeDoor6Hatchway;
extern const int SnSafeDoor7Hatchway;
extern const int SnSafeDoor8Hatchway;
extern const int SnSafeDoor9Hatchway;
extern const int SnSafeDoor10Hatchway;
extern const int SnSafeDoor11Hatchway;
extern const int SnSafeDoor6PosFixPickPlace;
extern const int SnTriTempSafeDoor6Lock;
//Ztex 2023.04.13 Add HT-1032 IO <==
extern const int SnTrayArmZSafePos;
extern const int SnEnhaustAirFanAlarmDetect;//Ztex 2023.04.26 Add HT-1032 IO Exhaust Air
extern const int SnDockingAreaOpenCheck;//Ztex 2023.05.02 Add HT-1032 IO Docking Area Open Check
extern const int SnDewPointDetectIndexArm1 ;   //Hmy 20170603  add By 三溫機 露點SENSOR Arm1偵測
extern const int SnDewPointDetectIndexArm2 ;   //Hmy 20170603  add By 三溫機 露點SENSOR Arm2偵測

extern const int SnLoaderSeparateHasTray ;
extern const int SnEmptySeparateHasTray  ;
extern const int SnColorSeparateHasTray  ;
extern const int SnAuto1SeparateHasTray  ;
extern const int SnAuto2SeparateHasTray  ;
extern const int SnAuto3SeparateHasTray  ;

extern const int SenLoaderCCWDete_2      ;
extern const int SenEmptyCCWDete_2       ;
extern const int SenColorCCWDete_2       ;
extern const int SenAuto1CCWDete_2       ;
extern const int SenAuto2CCWDete_2       ;
extern const int SenAuto3CCWDete_2       ;

extern const int SnOpenDoorChangeKit1    ;
extern const int SnOpenDoorChangeKit2    ;
extern const int SnOpenDoorChangeKit3    ;

extern const int SnIndexCylinderDetectHead1 ;
extern const int SnIndexCylinderDetectHead2 ;
extern const int SnIndexCylinderDetectHead5 ;
extern const int SnIndexCylinderDetectHead6 ;

extern const int SnIonBar1               ;
extern const int SnIonBar2               ;
extern const int SnIonBar3               ;
extern const int SnIonBar4               ;
extern const int SnIonBar5               ;
extern const int SnIonBar6               ;
extern const int SnIonBar7               ;
extern const int SnIonBar8               ;
extern const int SnIonBar9               ;

extern const int SnTrayArmHasTray        ;
extern const int SnInAreaAlignmentSenZ   ;                                      //KenHsieh 20211110 ： AutoAlignment add Auto Z
extern const int SnOutAreaAlignmentSenZ  ;                                      //KenHsieh 20211110 ： AutoAlignment add Auto Z

extern const int SnAuto4_Tray_Car          ;                                       //Steven 20230907 : For HT-9011UC
extern const int SnAuto4PreDete            ;
extern const int SnAuto4TrackDetect        ;
extern const int SnAuto4FixCyPush          ;
extern const int SnAuto4EdgePush           ;
extern const int SnAuto4UpSafedetect       ;
extern const int SnAuto4SeparateHasTray    ;
extern const int SnAuto4TrayHasTray        ;
extern const int SnAuto4CWPreDetect        ;
extern const int SenAuto4CCWDete_2         ;
extern const int SnAuto4Z_Select1          ;
extern const int SnAuto4Z_Select2          ;
extern const int SnAuto4Z_Select3          ;
extern const int SnAuto4Z_Select4          ;
extern const int SnAuto4IsPreAlarm         ;
extern const int SnAuto4IsFull             ;
extern const int SnAuto4UpPress            ;
extern const int SnAutoColorTrayDetect4    ;

extern const int SnAuto5_Tray_Car          ;
extern const int SnAuto5PreDete            ;
extern const int SnAuto5TrackDetect        ;
extern const int SnAuto5FixCyPush          ;
extern const int SnAuto5EdgePush           ;
extern const int SnAuto5UpSafedetect       ;
extern const int SnAuto5SeparateHasTray    ;
extern const int SnAuto5TrayHasTray        ;
extern const int SnAuto5CWPreDetect        ;
extern const int SenAuto5CCWDete_2         ;
extern const int SnAuto5Z_Select1          ;
extern const int SnAuto5Z_Select2          ;
extern const int SnAuto5Z_Select3          ;
extern const int SnAuto5Z_Select4          ;
extern const int SnAuto5IsPreAlarm         ;
extern const int SnAuto5IsFull             ;
extern const int SnAuto5UpPress            ;
extern const int SnAutoColorTrayDetect5    ;

extern const int SnAuto6_Tray_Car          ;
extern const int SnAuto6PreDete            ;
extern const int SnAuto6TrackDetect        ;
extern const int SnAuto6FixCyPush          ;
extern const int SnAuto6EdgePush           ;
extern const int SnAuto6UpSafedetect       ;
extern const int SnAuto6SeparateHasTray    ;
extern const int SnAuto6TrayHasTray        ;
extern const int SnAuto6CWPreDetect        ;
extern const int SenAuto6CCWDete_2         ;
extern const int SnAuto6Z_Select1          ;
extern const int SnAuto6Z_Select2          ;
extern const int SnAuto6Z_Select3          ;
extern const int SnAuto6Z_Select4          ;
extern const int SnAuto6IsPreAlarm         ;
extern const int SnAuto6IsFull             ;
extern const int SnAuto6UpPress            ;
extern const int SnAutoColorTrayDetect6    ;

extern const int SnFixedTray4Detect        ;
extern const int SnFixedTray5Detect        ;
extern const int SnFixedTray6Detect        ;
extern const int SnFix5ColorTrayDetect     ;
extern const int SnFix4ColorTrayDetect     ;
extern const int SnFix6ColorTrayDetect     ;
extern const int SnAuto4TrayDetect         ;
extern const int SnAuto5TrayDetect         ;
extern const int SnAuto6TrayDetect         ;

extern const int SnTrayArmTrayDetect1      ;
extern const int SnTrayArmTrayDetect2      ;
extern const int SnTrayArmTrayDetect3      ;
extern const int SnTrayArmTrayDetect4      ;

extern const int SnIonBar10                ;
extern const int SnIonBar11                ;

extern const int SnElectricControlBox      ;                                    //ChungHung 20230718 add for Safe plc
extern const int SnAllSafeDoor             ;                                    //ChungHung 20230718 add for Safe plc
extern const int SnAllEMG                  ;                                    //KenHsieh 20250212 : 新增PLC 斷線可瞬間判斷EMG及安全門

extern const int SnTesterAlarm  ;
extern const int SnDoubleLoadDetection     ;                                    //Steven 20240426 : 偵測loader疊盤

extern const int SnAuto1HasCoverTray;                                           //JerryYang 20241021 : Unloader增加第二組Sensor檢查是否有cover tray
extern const int SnAuto2HasCoverTray;
extern const int SnAuto3HasCoverTray;
extern const int SnAuto4HasCoverTray;
extern const int SnAuto5HasCoverTray;
extern const int SnAuto6HasCoverTray;
extern const int SnLoadIonGun;                                                  //Ifor 20230427 add:Loader Ionizer Gun
extern const int SnLoadLightGat;                                                //Ifor 20230427 add:LD/ULD light gate

extern int SnAutoTrayCar       [MAX_AUTO_TRAY];
extern int SnAutoZSelect1      [MAX_AUTO_TRAY];
extern int SnAutoZSelect2      [MAX_AUTO_TRAY];
extern int SnAutoZSelect3      [MAX_AUTO_TRAY];
extern int SnAutoZSelect4      [MAX_AUTO_TRAY];
extern int SnAutoPreDete       [MAX_AUTO_TRAY];
extern int SnAutoUpSafe        [MAX_AUTO_TRAY];
extern int SnAutoTrackDetect   [MAX_AUTO_TRAY];
extern int SnAutoTrayDetect    [MAX_AUTO_TRAY];
extern int SnAutoColorTrayDete [MAX_AUTO_TRAY];
extern int SnAutoSeparate      [MAX_AUTO_TRAY];
extern int SnAutoFixCyPush     [MAX_AUTO_TRAY];
extern int SnAutoEdgePush      [MAX_AUTO_TRAY];
extern int SnAutoTrayHasTray   [MAX_AUTO_TRAY];
extern int SnAutoCWPreDetect   [MAX_AUTO_TRAY];
extern int SnAutoCCWDete       [MAX_AUTO_TRAY];
extern int SnAutoIsPreAlarm    [MAX_AUTO_TRAY];
extern int SnAutoIsFull        [MAX_AUTO_TRAY];
extern int SnAutoUpPress       [MAX_AUTO_TRAY];
extern int SnAutoHasCoverTray  [MAX_AUTO_TRAY];

extern int SnAutoBoatActDetect [2];                                             //RogerYang 20260202 : Add for CR

extern int iC_SeparateRL       [MAX_AUTO_TRAY];
extern int iC_SeparateFL       [MAX_AUTO_TRAY];
extern int iC_SeparateRR       [MAX_AUTO_TRAY];
extern int iC_SeparateFR       [MAX_AUTO_TRAY];

extern int SnFixColorTrayDete  [MAX_FIX_TRAY];
extern int SnFixedTrayDetect   [MAX_FIX_TRAY];

extern const int iSafeDoor[MAX_SAFE_DOOR_CNT];                                  //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
extern const int iSafeDoorPosition[MAX_SAFE_DOOR_CNT];                          //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
extern AnsiString asSafeDoorAlarm[MAX_SAFE_DOOR_CNT];                           //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
extern const int iSafeDoorHatchway[MAX_HATCH_DOOR_CNT];
extern const int iHatchwaySafeDoorPosition[MAX_HATCH_DOOR_CNT];
extern AnsiString asHatchwaySafeDoorAlarm[MAX_HATCH_DOOR_CNT];
extern const int iIonFan[MAX_IONFAN]       ;                                    //Steven 20100226
extern const int iHTIonBar[MAX_HTIONFAN]   ;                                    //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
extern const int iIonFanPower[MAX_IONFAN]  ;                                    //Steven 20130201 : Kasuga離子風扇電源偵測
extern const int iIonBar[MAX_IONBAR]       ;                                    //Ztex 2024.12.22 Add For HT1032AT IonBar
extern const int BackSenBit0               ;
extern const int BackSenBit1               ;
extern const int BackSenBit2               ;
extern const int BackSenBit3               ;
extern const int BackSenBit4               ;
extern const int BackSenBit5               ;
extern const int BackSenBit6               ;
extern const int BackSenBit7               ;
extern const int BackSenBit8               ;
extern const int BackSenBit9               ;
extern const int BackSenBit10              ;
extern const int BackSenBit11              ;
extern const int BackSenBit12              ;
extern const int BackSenBit13              ;
extern const int BackSenBit14              ;
extern const int BackSenBit15              ;
extern const int BackSenBit16              ;
extern const int BackSenBit17              ;
extern const int BackSenBit18              ;
extern const int BackSenBit19              ;

//Alick 20161011 (Steven) : TTL支援8Site
extern const int BackSenBit20              ;
extern const int BackSenBit21              ;
extern const int BackSenBit22              ;
extern const int BackSenBit23              ;
extern const int BackSenBit24              ;
extern const int BackSenBit25              ;
extern const int BackSenBit26              ;
extern const int BackSenBit27              ;
extern const int BackSenBit28              ;
extern const int BackSenBit29              ;
extern const int BackSenBit30              ;
extern const int BackSenBit31              ;
extern const int BackSenBit32              ;
extern const int BackSenBit33              ;
extern const int BackSenBit34              ;
extern const int BackSenBit35              ;
extern const int BackSenBit36              ;
extern const int BackSenBit37              ;
extern const int BackSenBit38              ;
extern const int BackSenBit39              ;

extern const int SnMagazineSafeDoor               ; //JerryYang 20220909 : add magazine
extern const int SnMagazineDetect                 ;
extern const int SnMagazineTrackDetect            ;
extern const int SnMagazineTrackDetect2           ; //Sam 20221116 : Magazine TrayArm 自動補 Tray
extern const int SnMagazineTrackSelectDetect      ;
extern const int SnMagazineSafeDoor2              ;
extern const int SnMagazineSafeDoor3              ;
extern const int SnMagazineDetectTop              ;
extern const int SnMagazineHasTrayInside          ;
extern const int SnEPDetect                       ; //kevin 20230608 EP 流量計 偵測
//Ifor 20211005 add Tray 載盤上升下降前判斷是否有異常
//==>
extern const int SnLoader_Detect            ;
extern const int SnEmpty_Detect             ;
extern const int SnColor_Detect             ;
extern const int SnAuto1_Detect             ;
extern const int SnAuto2_Detect             ;
extern const int SnAuto3_Detect             ;
extern const int SnAuto4_Detect             ;
extern const int SnAuto5_Detect             ;
extern const int SnAuto6_Detect             ;
//<==
//Ifor 20211005 add Tray 載盤上升下降前判斷是否有異常
extern const int SnIonFanCar                ;                                   //Ifor 20220310 add: Bin Car Ion Fan Check
extern const int SnIonFanCarPower           ;                                   //Ifor 20220816 add: Bin Car Ion Fan Power Check
extern const int SnEPFlowmeter;                                                 //Ifor 20240326 add: EP流量計監控

extern const int SnSocket17              ;
extern const int SnSocket18              ;
extern const int SnSocket19              ;
extern const int SnSocket20              ;
extern const int SnSocket21              ;
extern const int SnSocket22              ;
extern const int SnSocket23              ;
extern const int SnSocket24              ;
extern const int SnSocket25               ;
extern const int SnSocket26               ;
extern const int SnSocket27               ;
extern const int SnSocket28               ;
extern const int SnSocket29               ;
extern const int SnSocket30               ;
extern const int SnSocket31               ;
extern const int SnSocket32               ;
extern const int SnChamberDryAir         ;                                      //Ifor 20240919 add: Chamber Dry Air
extern const int SnLoadCarRFIDSW         ;                                      //RogerYang 20250828 : add for Loader Rotate Arm

extern const int SnLoaderCarrier1        ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto1Carrier1         ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto2Carrier1         ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnLoaderCarrier2        ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto1Carrier2         ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto2Carrier2         ;                                      //Ifor 20251216 add:Boat Carrier

extern const int SnLoaderBoatActDetect   ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto1BoatActDetect    ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto2BoatActDetect    ;                                      //Ifor 20251216 add:Boat Carrier

//===================================================================
extern const int SwFKPowerOff                  ;
extern const int SwFKPowerOn                   ;
extern const int SwFKReset                     ;
extern const int SwFKPause                     ;
extern const int SwFKHome                      ;
extern const int SwFKStart                     ;
extern const int SwFKOneCycle                  ;
extern const int SwFKRetry                     ;

extern const int SwFKSkip                      ;
extern const int SwFKCleanOut                  ;
extern const int SwFKTrayFeed                  ;
extern const int SwFKTrayEnd                   ;
extern const int SwFKAlarmReset                ;
extern const int SwFKCoverOpen                 ;
extern const int SwRKPowerOff                  ;
extern const int SwRKPowerOn                   ;

extern const int SwRKReset                     ;
extern const int SwRKPause                     ;
extern const int SwRKHome                      ;
extern const int SwRKStart                     ;
extern const int SwRKOneCycle                  ;
extern const int SwRKRetry                     ;
extern const int SwRKSkip                      ;
extern const int SwRKCleanOut                  ;

extern const int SwRKTrayFeed                  ;
extern const int SwRKTrayEnd                   ;
extern const int SwRKAlarmReset                ;
extern const int SwRKCoverOpen                 ;
extern const int SwRKManualStep                ;
extern const int SwRKManualTStart              ;
extern const int SwTowerRed                    ;
extern const int SwTowerGreen                  ;

extern const int SwTowerYellow                 ;
extern const int SwFMotorBreaker               ;
extern const int SwBMotorBreaker               ;
extern const int SwMotorRelay                  ;
extern const int SwHeaterRelay                 ;
extern const int SwMusic1                      ;
extern const int SwMusic2                      ;
extern const int SwMusic3                      ;

extern const int SwMusic4                      ;
extern const int SwTestPassLed                 ;
extern const int SwTestFailLed                 ;
extern const int SwFrontActiveLed              ;
extern const int SwRearActiveLed               ;

extern const int SwClear0                      ;
extern const int SwClear1                      ;
extern const int SwStart0                      ;
extern const int SwStart1                      ;
extern const int SwACTrayY                     ;
extern const int SwACAuto1                     ;
extern const int SwACAuto2                     ;
extern const int SwACAuto3                     ;
extern const int SwServerON                    ;
extern const int SwManualZ1                    ;
extern const int SwManualZ2                    ;
extern const int SwACEmptyCW                   ;
extern const int SwACEmptyCCW                  ;
extern const int SwACColorCW                   ;
extern const int SwClear2                      ;
extern const int SwClear3                      ;
extern const int SwStart2                      ;
extern const int SwStart3                      ;
extern const int SwCCDAir                      ;
extern const int SwACAutoCW                    ;  // for auto as loader use
extern const int SwACAuto1Mode                 ;  // for auto as loader use
extern const int SwSiteMode0                   ;
extern const int SwSiteMode1                   ;
extern const int SwSiteMode2                   ;
extern const int SwSiteMode3                   ;
extern const int SwAuto1SelectSlow             ;
extern const int SwCCDDestroy                  ;
extern const int SwReadTorue                   ;
extern const int SwBigFan                      ;
extern const int Sw10Bit                       ;
extern const int SwDut0                        ;
extern const int SwDut1                        ;
extern const int SwDut2                        ;
extern const int SwDut3                        ;
extern const int SwSafeLock                    ;
extern const int SwRKSafeLock                  ;    //KenHsieh 20211228 : 區分實體IO與通訊面板
extern const int SwCCDLight                    ;
extern const int SwHeaterFan                   ;
extern const int SwSocketClean                 ;
extern const int SwACColorCCW                  ;
extern const int SwShuttleFan                  ;
extern const int SwTesterDoubleContact         ;
extern const int SwTesterPower                 ;
extern const int SwACEmpty1CW                  ;
extern const int SwACEmpty1CCW                 ;
extern const int SwDutHeaterCoolFan            ;
extern const int SwLoaderBin                   ;
extern const int SwEmpty1Bin                   ;
extern const int SwEmpty2Bin                   ;
extern const int SwAuto1Bin                    ;
extern const int SwAuto2Bin                    ;
extern const int SwAuto3Bin                    ;
//Steven 20090917 Start
extern const int SwFix1Bin                     ;
extern const int SwFix2Bin                     ;
extern const int SwFix3Bin                     ;
extern const int SwFix4Bin                     ;
extern const int SwFix5Bin                     ;
extern const int SwFix6Bin                     ;
//Steven 20090916 Start
extern const int SwZ1SuckMode0                 ;
extern const int SwZ1SuckMode1                 ;
extern const int SwZ2SuckMode0                 ;
extern const int SwZ2SuckMode1                 ;
//Steven 20090916 End
extern const int SwShuttleCooling              ;    //jou 2010-06-09
extern const int SwCCDCooling                  ;    //Steven 20110705
extern const int SwEpArm1                      ;    //Steven 20110708
extern const int SwEpArm2                      ;    //Steven 20110708
extern const int SwHeaterFanSpeed              ;    //Steven 20110725
extern const int SwRotateCheckClear            ;    //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor

//Dell 20111111 Start : 加入Digital E/P
extern const int SwEP_D0;
extern const int SwEP_D1;
extern const int SwEP_D2;
extern const int SwEP_D3;
extern const int SwEP_D4;
extern const int SwEP_D5;
extern const int SwEP_D6;
extern const int SwEP_D7;
extern const int SwEP_D8;
extern const int SwEP_D9;
//Dell 20111111 End
extern const int SwCoolingFan_Blower;                                           //20111130  Dell
extern const int SwSafeDoorLock;                                                //20111130  Dell
extern const int SwIndexIonFan;                                                 //jou 2012-03-13 index離子槍出風開關控制

extern const int SwIndexChangeToque1;                                           //jou 2012-06-21 Enable index I/O Change Toque
extern const int SwIndexChangeToque2;                                           //jou 2012-06-21 Enable index I/O Change Toque
extern const int SwACLoaderCCW;                                                 //Loader退Tray
extern const int SwACAuto2CW;                                                   //Auto2進Tray
extern const int SwACAuto3CW;                                                   //Auto3進Tray

extern const int SwHeatGun;                                                     //ChungHung 20121107 add
extern const int SwCDAGun;                                                      //Steven 20181012 : 使用熱風槍吹冷風
extern const int SwVacuumPumpTogetherOn;                                        //Dell  for HT9046LS 雙幫浦模式
extern const int SwAirConditioner;                                              //Steven 20131011 : 冷氣機
extern const int SwLoadCellA;                                                   //kevin 20190306  add load Cell  read 1
extern const int SwLoadCellB;                                                   //kevin 20190306  add load Cell  read 2

extern const int SwHotplateCooling;                                             //jou 2013-11-07
extern const int SwCarRecord;                                                   //wei 2013-12-09
extern const int SwIonFanClean;                                                 //Isaac 20210609 : IO觸發IonFan清針

extern const int SwLoad2Bin      ;                                              //Steven 20240822 : For HT-9046AU
extern const int SwLoad2AirClean ;
extern const int SwACLoad2CCW    ;
extern const int SwACTray2Y      ;
extern const int SwLoad2Vibration;

extern const int SwUnDock;                                                      //Steven 20140310 : One Touch Docking
extern const int SwDockError;                                                   //Steven 20140310 : One Touch Docking
extern const int SwStartTest1;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern const int SwStartTest2;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern const int SwStartTest3;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern const int SwOCRTigger;                                                   //wei 20150720 OCR觸發

extern const int SwTesterAirCooling;                                            //jou 2016-01-07 Tester Air Cooling Switch
extern const int SwFanDirection;                                                //wei 20160215大風扇方向
extern const int SwIonRelay;                                                    //Ifor 20190114 : add Chamber 開啟時 關閉 Ion 離子槍 吹氣

//------------------------------------
//Steven 20161011 : TTL支援8Site
//------------------------------------
extern const int SwClear4                      ;
extern const int SwClear5                      ;
extern const int SwClear6                      ;
extern const int SwClear7                      ;
extern const int SwStart4                      ;
extern const int SwStart5                      ;
extern const int SwStart6                      ;
extern const int SwStart7                      ;
extern const int SwDut4                        ;
extern const int SwDut5                        ;
extern const int SwDut6                        ;
extern const int SwDut7                        ;
extern const int Sw10Bit2                      ;
extern const int SwIonBarPower                 ;                                //kevin 20170310 (wei) Ion bar power
extern const int SwShuttleVibration1           ;                                //JerryYang 20171006 (wei) Shuttle 震動馬達
extern const int SwShuttleVibration2           ;
extern const int SwPurgeAir                    ;                                //kevin 20180928 add load board blower
extern const int SwDieClean                    ;
extern const int SwDieCleanSuck                ;                                //wei 20210326
extern const int SwLightOff                    ;
extern const int SwAirOff                      ;
extern const int SwCaselevatorZ                ;
extern const int SwTrayBracketZ                ;
extern const int SwE84VALID                    ;
extern const int SwE84CS0                      ;
extern const int SwE84CS1                      ;
extern const int SwE84AMAVBL                   ;
extern const int SwE84TRREQ                    ;
extern const int SwE84BUSY                     ;
extern const int SwE84COMPT                    ;
extern const int SwE84CONT                     ;
extern const int SwE84GO                       ;

extern const int SwE84LREQ                    ;
extern const int SwE84UREQ                    ;
extern const int SwE84VA                      ;
extern const int SwE84READY                   ;
extern const int SwE84VS0                     ;
extern const int SwE84VS1                     ;
extern const int SwE84HOAVBL                  ;
extern const int SwE84ES                      ;
extern const int SwE84POWER                   ;

extern const int SwCassette01          ;   //wei 20180702 MR
extern const int SwCassette02          ;   //wei 20180702 MR
extern const int SwCassette03          ;   //wei 20180702 MR
extern const int SwCassette04          ;   //wei 20180702 MR
extern const int SwCassette05          ;   //wei 20180702 MR
extern const int SwCassette06          ;   //wei 20180702 MR
extern const int SwCassette07          ;   //wei 20180702 MR
extern const int SwCassette08          ;   //wei 20180702 MR
extern const int SwCassette09          ;   //wei 20180702 MR
extern const int SwCassette10          ;   //wei 20180702 MR
extern const int SwMRStart             ;   //wei 20180702 MR
extern const int SwMRPause             ;   //wei 20180702 MR
extern const int SwMRUp                ;   //wei 20180702 MR
extern const int SwMRDown              ;   //wei 20180702 MR

extern const int SwMRError             ;   //wei 20180702 MR
extern const int SwMRUnLoadReady       ;   //wei 20180702 MR
extern const int SwMRPresence          ;   //wei 20180702 MR
extern const int SwMRPlacement         ;   //wei 20180702 MR
extern const int SwMRLoadReady         ;   //wei 20180702 MR
extern const int SwMRManualMode        ;   //wei 20180702 MR
extern const int SwMRLight             ;   //wei 20180702 MR
extern const int SwBufferArmZ          ;   //wei 20200302 MR

//KaiChen 20200716 ：OHT
//==>
extern const int SwTrayBracket2Z       ;
extern const int SwCaselevator2Z       ;
extern const int SwMultileEmptyZ       ;

extern const int SwOHT_UnloadReady_1       ;
extern const int SwOHT_CarrierPresence_1   ;
extern const int SwOHT_CarrierPlacement_1  ;
extern const int SwOHT_LoadReady_1         ;
extern const int SwOHT_ManualMode_1        ;
extern const int SwOHT_AutoMode_1          ;
extern const int SwOHT_Error_1             ;

extern const int SwOHT_UnloadReady_2       ;
extern const int SwOHT_CarrierPresence_2   ;
extern const int SwOHT_CarrierPlacement_2  ;
extern const int SwOHT_LoadReady_2         ;
extern const int SwOHT_ManualMode_2        ;
extern const int SwOHT_AutoMode_2          ;
extern const int SwOHT_Error_2             ;

extern const int SwMultileEmpty_ScanTray_Open       ;
extern const int SwMultileEmpty_ScanTray_Close      ;
extern const int SwMultileEmpty_ScanTrayID          ;

extern const int SwE84_2_LREQ                    ;
extern const int SwE84_2_UREQ                    ;
extern const int SwE84_2_VA                      ;
extern const int SwE84_2_READY                   ;
extern const int SwE84_2_VS0                     ;
extern const int SwE84_2_VS1                     ;
extern const int SwE84_2_HOAVBL                  ;
extern const int SwE84_2_ES                      ;
extern const int SwE84_2_POWER                   ;

extern const int SwE84_1_LREQ                    ;
extern const int SwE84_1_UREQ                    ;
extern const int SwE84_1_VA                      ;
extern const int SwE84_1_READY                   ;
extern const int SwE84_1_VS0                     ;
extern const int SwE84_1_VS1                     ;
extern const int SwE84_1_HOAVBL                  ;
extern const int SwE84_1_ES                      ;
extern const int SwE84_1_POWER                   ;

extern const int SwSafeDoorLock_LoadPort1        ;
extern const int SwSafeDoorLock_LoadPort2        ;
//<==
//KaiChen 20200716 ：OHT

extern const int SwSocketClean2        ;   //JerryYang 20190715 Clean air arm1 arm2分開控制
extern const int SwTesterDryAirSwitch  ;   //Ifor 20200115 : add Tester Dry Air Control
//Sam 20190112 LM
//==>
extern const int SwLoadDoorLock       ;
extern const int SwLoadRobotZ         ;
extern const int SwUnloadDoorLock     ;
extern const int SwUnloadRobotZ       ;
extern const int SwSafeDoorLockLM     ;
extern const int SwIndEpArm1          ;
extern const int SwIndEpArm2          ;
//<==
//Ifor 20210622 add: ATC Switch TJ
//==>
extern const int SwTjSignal01;
extern const int SwTjSignal02;
extern const int SwTjSignal03;
extern const int SwTjSignal04;
extern const int SwTjSignal05;
extern const int SwTjSignal06;
extern const int SwTjSignal07;
extern const int SwTjSignal08;
//<==
//Ifor 20210622 add: ATC Switch TJ
extern const int SwATCHeatGun;  //JerryYang 20220408 : add for ATC3.5
extern const int SwLBAir;       //JerryYang 20220923 : LB吹氣function
extern const int SwDryAirSwitch ;//Ztex 2023.04.13 Add HT-1032 IO
extern const int SwColdAirSwitch;//Ztex 2023.04.13 Add HT-1032 IO
extern const int SwTriTempSafeDoor6Lock;//Ztex 2023.04.13 Add HT-1032 IO
extern const int SwEnhaustAirFanPowerOn;//Ztex 2023.04.26 Add HT-1032 IO Exhaust Air
//====================================================================
extern const int BackSwStart0                  ;
extern const int BackSwStart1                  ;
extern const int BackSwStart2                  ;
extern const int BackSwStart3                  ;
//Alick 20161011 (Steven) : TTL支援8Site
extern const int BackSwStart4                  ;
extern const int BackSwStart5                  ;
extern const int BackSwStart6                  ;
extern const int BackSwStart7                  ;
extern const int BackSwDut0                    ;
extern const int BackSwDut1                    ;
extern const int BackSwDut2                    ;
extern const int BackSwDut3                    ;
extern const int BackSwDut4                    ;
extern const int BackSwDut5                    ;
extern const int BackSwDut6                    ;
extern const int BackSwDut7                    ;
extern const int SwLoaderVibration             ;
extern const int SwAutoCoolDown                ;   //kevin 20201223 AutoCool down
extern const int SwMagazineMotorBreaker        ;  //JerryYang 20220909 : add magazine
extern const int SwMagazineSafeDoorLock        ;
extern const int SwMagazineSafeDoor2LockOn     ;
extern const int SwMagazineSafeDoor2LockOff    ;
extern const int SwESDAntennaRelay1            ;
extern const int SwESDAntennaRelay2            ;
extern const int SwESDAntennaRelay3            ;
extern const int SwESDAntennaRelay4            ;
extern const int SwESDAntennaRelay5            ;
extern const int SwESDAntennaRelay6            ;
extern const int SwESDAntennaRelay7            ;
extern const int SwESDAntennaRelay8            ;

extern const int SwLoaderAirClean              ;

extern const int SwACAuto4    ;                                                 //Steven 20230907 : For HT-9011UC
extern const int SwACAuto4CW  ;
extern const int SwACAuto5    ;
extern const int SwACAuto5CW  ;
extern const int SwACAuto6    ;
extern const int SwACAuto6CW  ;

extern const int SwSafeDoor1HatchwayLock       ;                                //ChungHung 20230718 add for Safe plc start
extern const int SwSafeDoor2HatchwayLock       ;
extern const int SwSafeDoor3HatchwayLock       ;
extern const int SwSafeDoor4HatchwayLock       ;
extern const int SwSafeDoor5HatchwayLock       ;
extern const int SwSafeDoor6HatchwayLock       ;
extern const int SwSafeDoor7HatchwayLock       ;
extern const int SwSafeDoor8HatchwayLock       ;

extern const int SwElectricControlBoxLock1     ;
extern const int SwElectricControlBoxLock2     ;
extern const int SwElectricControlBoxLock3     ;
extern const int SwElectricControlBoxLock4     ;
extern const int SwElectricControlBoxLock5     ;
extern const int SwElectricControlBoxLock6     ;
extern const int SwElectricControlBoxLock7     ;                                //ChungHung 20230718 add for Safe plc end

extern const int SwInArmZBreaker               ;                                //add One sucker with rotate
extern const int SwOutArmZBreaker              ;                                //add One sucker with rotate

extern const int SwLoadCarRFIDZBreaker         ;                                //RogerYang 20250828 add for Loader Rotate Arm
extern const int SwBottomBlower;
extern const int SwFixedSeatTLOn;                                               //Jimmychiu 20240322 : Top & Bottom Inspect
extern const int SwFixedSeatTLOff;
extern const int SwFixedSeatTROn;
extern const int SwFixedSeatTROff;
extern const int SwFixedSeatBLOn;
extern const int SwFixedSeatBLOff;
extern const int SwFixedSeatBROn;
extern const int SwFixedSeatBROff;
extern const int SwCCDZBreaker;
extern const int SwTopBtmRotateLockOn;
extern const int SwTopBtmRotateLockOff;
extern const int SwLightStart;
extern const int SwPRGSEL0;
extern const int SwPRGSEL1;
extern const int SwPRGSEL2;
extern const int SwPRGSEL3;
extern const int SwPRGSEL4;
extern const int SwPRGSEL5;
extern const int SwPRGSEL6;
extern const int SwLightOrg;
extern const int SwDryAirUseHandler            ;
extern const int SwDryAirUseATCCar             ;
extern const int SwMultiEp;                                                     //Ifor 20250618 add:Auto Switch Multi EP
extern const int SwCassetteLDMotBreaker;                                        //Ifor 20251216 add:Boat Carrier
extern const int SwCassetteAuto1MotBreaker;                                     //Ifor 20251216 add:Boat Carrier
extern const int SwCassetteAuto2MotBreaker;                                     //Ifor 20251216 add:Boat Carrier
extern const int SwCassetteEmptyMotBreaker;                                     //AI(W906-BRAKE-EMPTY-AUTO3) 20260930: HT9050 M36 MEmptyZ brake (IO_Table st16 ch25)
extern const int SwCassetteAuto3MotBreaker;                                     //AI(W906-BRAKE-EMPTY-AUTO3) 20260930: HT9050 M40 MAuto3Z brake (IO_Table st16 ch28)

extern int SwAutoCCW[MAX_AUTO_TRAY];
extern int SwAutoCW [MAX_AUTO_TRAY];

//====================================================================
extern const int MInArmX       ;
extern const int MInArmY       ;
extern const int MInArmPitch   ;
extern const int MInArmZA      ;  //0=i+j*2 (0, 0)
extern const int MInArmZB      ;  //1=i+j*2 (1, 0)
extern const int MInArmZC      ;  //2=i+j*2 (0, 1)
extern const int MInArmZD      ;  //3=i+j*2 (1, 1)
extern const int MInArmZE      ;  //4=i+j*2 (0, 2)
extern const int MInArmZF      ;  //5=i+j*2 (1, 2)
extern const int MInArmZG      ;  //6=i+j*2 (0, 3)
extern const int MInArmZH      ;  //7=i+j*2 (1, 3)
extern const int MInShuttle1   ;
extern const int MInShuttle2   ;
extern const int MTestY1       ;
extern const int MTestZ1       ;
extern const int MTestZ2       ;
extern const int MTestY2       ;
extern const int MOutShuttle1  ;
extern const int _CCDX         ;
extern const int MOutShuttle2  ;
extern const int MOutArmX      ;
extern const int MOutArmY      ;
extern const int MOutArmPitch  ;
extern const int MOutArmZA     ;
extern const int MOutArmZB     ;
extern const int MOutArmZC     ;
extern const int MOutArmZD     ;
extern const int MOutArmZE     ;
extern const int MOutArmZF     ;
extern const int MOutArmZG     ;
extern const int MOutArmZH     ;
extern const int MTrayX        ;
extern const int MInArmPitchY  ;    //Steven 20131002 : XY變距
extern const int MInArmPitchX2 ;    //Steven 20131002 : XY變距 //ChungHung 20131231 alter AutoYPitch
extern const int MOutArmPitchY ;    //Steven 20131002 : XY變距 //ChungHung 20131231 alter AutoYPitch
extern const int MOutArmPitchX2;    //Steven 20131002 : XY變距

extern const int MLoaderZ      ;
extern const int MEmptyZ       ;
extern const int MColorZ       ;
extern const int MAuto1Z       ;
extern const int MAuto2Z       ;
extern const int MAuto3Z       ;
extern const int MInRotateKit  ;        //2013-04-12    Dell :旋轉站;馬達版
extern const int MOutRotateKit ;        //2013-04-12    Dell :旋轉站;馬達版
extern const int MAOIKit       ;        //2014-03-04    Dell    for SPIL WLP Add 5S Inspection

extern const int MLoaderY      ;        //Steven 20150910 : Add for OCR
extern const int MEmptyY       ;
extern const int MColorY       ;
extern const int MAuto1Y       ;
extern const int MAuto2Y       ;
extern const int MAuto3Y       ;
//extern const int MTapeReelFront;        //Steven 20150910 : Add for Tape Reel
//extern const int MReelRotateR  ;
//extern const int MTapeReelZ    ;
//extern const int MTapeShuttle1 ;
//extern const int MTapeShuttle2 ;
//extern const int MTapeInX      ;
//extern const int MTapeInZ      ;
extern const int MInArmZAe       ;        //Steven 20230323 : For HT1032
extern const int MInArmPitchX3 ;
extern const int MInArmPitchX4 ;
extern const int MInArmZAf     ;
extern const int MOutArmPitchX3;
extern const int MOutArmPitchX4;
extern const int MTrayZ        ;

extern const int MOutSortX     ;                                                //Steven 20240822 : For HT-9046AU
extern const int MOutSortY     ;
extern const int MOutSortPitchX;
extern const int MOutSortAa    ;
extern const int MOutSortAb    ;
extern const int MOutSortSht   ;
extern const int MLdCarRotArm  ;                                               //RogerYang 20250828 add for Loader Rotate Arm

extern const int MInArmXScale  ;        //Steven 20160426 : 磁性尺
extern const int MInArmYScale  ;
extern const int MOutArmXScale ;
extern const int MOutArmYScale ;

extern const int MShuttle1Pitch ;       //wei 20160914 Auto Shuttle Sensor
extern const int MShuttle2Pitch ;       //wei 20160914 Auto Shuttle Sensor

//Steven 20170329 (Wei) : Add individual rotate motor
//==>
extern const int MInRotateB    ;
extern const int MInRotateC    ;
extern const int MInRotateD    ;
extern const int MInRotateE    ;
extern const int MInRotateF    ;
extern const int MInRotateG    ;
extern const int MInRotateH    ;
extern const int MOutRotateB   ;
extern const int MOutRotateC   ;
extern const int MOutRotateD   ;
extern const int MOutRotateE   ;
extern const int MOutRotateF   ;
extern const int MOutRotateG   ;
extern const int MOutRotateH   ;
extern const int MLightScale   ;

extern int MInRotate[MAX_ARM_Row][MAX_ARM_Col];
extern int MOutRotate[MAX_ARM_Row][MAX_ARM_Col];
//<==
//Steven 20170329 (Wei) : Add individual rotate motor

extern const int MPreciser     ;    //Steven 20180212 : 定位器  ;
//extern const int MTrayRobotX   ;    //Steven 20170330 (Wei) : For HT-9046LM
//extern const int MTrayRobotY   ;    //Steven 20170330 (Wei) : For HT-9046LM
//extern const int MTrayRobotZ   ;    //Steven 20170330 (Wei) : For HT-9046LM
//extern const int MNoUse82      ;
extern const int MArmAlignment ;    //Steven 20240507 : 只是為了Teaching存檔方便
extern const int MLoadHingeR   ;    //Steven 20170330 (Wei) : For TSMC
extern const int MLoadHingeZ   ;    //Steven 20170330 (Wei) : For TSMC

extern const int MInArmZAg    ;         //Steven 20230323 : For HT1032
extern const int MInArmZAh    ;
extern const int MInArmZBe    ;
extern const int MInArmZBf    ;
extern const int MInArmZBg    ;
extern const int MInArmZBh    ;

extern const int MOutArmZAe  ;
extern const int MOutArmZAf  ;
extern const int MOutArmZAg  ;
extern const int MOutArmZAh  ;
extern const int MOutArmZBe  ;
extern const int MOutArmZBf  ;
extern const int MOutArmZBg  ;
extern const int MOutArmZBh  ;

extern const int MCasArmX      ;    //wei 20180702 MR
extern const int MCasArmZ      ;    //wei 20180702 MR
extern const int MTrayBracketZ ;    //wei 20180702 MR
extern const int MStackedTrayX ;    //wei 20180702 MR
extern const int MStackedTrayZ ;    //wei 20180702 MR
//extern const int MLoadRobotZ   ;    //Sam 20190112 LM
extern const int MUnloadRobotZ ;    //Sam 20190112 LM
extern const int MCaselevatorZ ;    //wei 20180702 MR
extern const int MMagazine     ;    //JerryYang 20220909 : add magazine
extern const int MCatchMgzTray ;    //JerryYang 20220909 : add magazine
extern const int MMagYTrayOut  ;    //JerryYang 20220909 : add magazine

extern const int MMTrayZ       ;
extern const int MMEmptyZ      ;
extern const int MMColorZ      ;
extern const int MMAuto1Z      ;
extern const int MMAuto2Z      ;
extern const int MMAuto3Z      ;        //ChungHung 20140317 add Auto Retest

extern const int MManualTray1  ;
extern const int MManualTray2  ;
extern const int MManualTray3  ;
extern const int MMTrayY       ;
extern const int MMTrayY_Car   ;
extern const int MMPlate1      ;
extern const int MMPlate2      ;
extern const int MMAuto1       ;
extern const int MMAuto2       ;
extern const int MMAuto3       ;
extern const int MMAuto1_Car   ;
extern const int MMAuto2_Car   ;
extern const int MMAuto3_Car   ;
extern const int MMEmpty       ;
extern const int MMColor       ;
extern const int MMEmpty_Car   ;
extern const int MMColor_Car   ;
extern const int MMEmpty1      ;
extern const int MMEmpty1_Car  ;

extern const int MMHot1RecBuf  ;   //jou 2011-12-26 加入記憶尚未完成吸取的位置
extern const int MMHot2RecBuf  ;   //jou 2011-12-26 加入記憶尚未完成吸取的位置

extern const int MMAutoCleanKit;   //jou 2012-05-21 Auto Clean
extern const int MMOCR         ;   //Steven 20120626 : OCR

//extern const int MMInRotateKit ;   //jou 2013-03-01 Rotate kit
//extern const int MMOutRotateKit;   //jou 2013-03-01 Rotate kit
extern const int MMBulkboxKit;     //kevin 20160822

extern const int MMCABuffer1   ;   //wei 20180702 MR
extern const int MMCABuffer2   ;   //wei 20180702 MR
extern const int MMCABuffer3   ;   //wei 20180702 MR
extern const int MMCABuffer4   ;   //wei 20180702 MR
extern const int MMCABuffer5   ;   //wei 20180702 MR
extern const int MMCABuffer6   ;   //wei 20180702 MR
extern const int MMCABuffer7   ;   //wei 20180702 MR
extern const int MMCABuffer8   ;   //wei 20180702 MR
extern const int MMCABuffer9   ;   //wei 20180702 MR
extern const int MMCABuffer10  ;   //wei 20180702 MR
extern const int MMLoadPort    ;   //wei 20180702 MR

extern const int MMTrayLoader  ;   //wei 20180702 MR
extern const int MMTrayEmpty   ;   //wei 20180702 MR

extern const int MMTrayConversion; //wei 20180702 MR
extern const int MMTrayAuto1   ;   //wei 20180702 MR
extern const int MMTrayAuto2   ;   //wei 20180702 MR
extern const int MMTrayAuto3   ;   //wei 20180702 MR

extern const int MMDailyCorrelationKit; //KaiHuang 20200606 : For ASE-CL Daily Correlation

//Sam 20190112 LM
//==>
extern const int MCCDX         ;
extern const int MCCDY         ;
extern const int MCCDZ         ;
/*extern const int M1_1X         ;
extern const int M1_1Y         ;
extern const int M1_1R         ;
extern const int M1_2X         ;
extern const int M1_2Y         ;
extern const int M1_2R         ;
extern const int M1_3X         ;
extern const int M1_3Y         ;  */
extern const int MLdCarRotArm  ;                                                //RogerYang 20250828 add for Loader Rotate Arm
extern const int MLoaderY_CCW  ;
extern const int MAuto1Y_CCW   ;
extern const int MAuto2Y_CCW   ;
extern const int MAuto3Y_CCW   ;
extern const int MAuto4Y_CCW   ;
extern const int MAuto5Y_CCW   ;
extern const int MAuto6Y_CCW   ;
extern const int M1_3R         ;
extern const int M1_4X         ;
extern const int M1_4Y         ;
extern const int M1_4R         ;

extern const int MInFlipper1   ;        //Frank 20210612 : Flipper Function
extern const int MInFlipper2   ;
extern const int MInFlipper3   ;
extern const int MOutFlipper1  ;
extern const int MOutFlipper2  ;
extern const int MOutFlipper3  ;

extern const int M1_5X         ;
extern const int M1_5Y         ;
extern const int M1_5R         ;
extern const int M1_6X         ;
extern const int M1_6Y         ;
extern const int M1_6R         ;
extern const int M1_7X         ;
extern const int M1_7Y         ;
extern const int M1_7R         ;
extern const int M1_8X         ;
extern const int M1_8Y         ;
extern const int M1_8R         ;

extern const int MFix3Full     ;    //JimmyChiu 20220927 : Stepper Motor Control in Fix3

extern const int MTopAOIArmX   ;
extern const int MTopAOIArmY   ;
extern const int MTopAOIArmR   ;
extern const int MTopAOICCDZ   ;
extern const int MTopAOIElevZ1 ;
extern const int MTopAOIElevZ2 ;

extern const int MMLoadPort1   ;
extern const int MMLoadPort2   ;
extern const int MMLoadPort3   ;
extern const int MMLoadPort4   ;
extern const int MMUnloadPort1 ;
extern const int MMUnloadPort2 ;
extern const int MMUnloadPort3 ;
extern const int MMUnloadPort4 ;
//<==
//Sam 20190112 LM

extern const int MAuto4Z;                                                       //Steven 20230907 : For HT-9011UC
extern const int MAuto5Z;
extern const int MAuto6Z;
extern const int MAuto4Y;
extern const int MAuto5Y;
extern const int MAuto6Y;

extern const int MLoad2Z       ;                                                //Steven 20240822 : For HT-9046AU
extern const int MLoad2Y       ;

extern const int MInSh1LtcSenZ1;                                                //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
extern const int MInSh1LtcSenZ2;
extern const int MInSh2LtcSenZ1;
extern const int MInSh2LtcSenZ2;

extern const int MMFixTray1    ;    //Steven 20100205 : 暫存Fix資料用
extern const int MMFixTray2    ;    //Steven 20100205 : 暫存Fix資料用
extern const int MMFixTray3    ;    //Steven 20100205 : 暫存Fix資料用

//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//==>
extern const int MMInArmAOATray ;
extern const int MMOutArmAOATray;
extern const int MMAOASampleTray;
extern const int MMAOASamplePlate;
//<==
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT

extern const int MMSystem      ;
extern const int MMInShuttle   ;
extern const int MMOutShuttle  ;
extern const int MMIndex       ;
extern const int MMTemperature ;
extern const int MMInterface   ;
extern const int MMScanner     ;
extern const int MMCCD         ;
extern const int MMAutoClean   ;

extern const int MMSafeDoor1   ;
extern const int MMSafeDoor2   ;
extern const int MMSafeDoor3   ;
extern const int MMSafeDoor4   ;
extern const int MMSafeDoor5   ;
extern const int MMSafeDoor6   ;
extern const int MMSafeDoor7   ;
extern const int MMSafeDoor8   ;
extern const int MMSafeDoor9   ;
extern const int MMSafeDoor10  ;
extern const int MManualTrayAll;

//Steven 20130205 : 離子風扇異常顯示
extern const int MMIonFan01    ;
extern const int MMIonFan02    ;
extern const int MMIonFan03    ;
extern const int MMIonFan04    ;
extern const int MMIonFan05    ;
extern const int MMIonFan06    ;
extern const int MMIonFan07    ;
extern const int MMIonFan08    ;
extern const int MMIonFan09    ;
extern const int MMIonFan10    ;
extern const int MMIonFan11    ;
extern const int MMIonFan12    ;
//Ifor 20160830 add ATC 異常位置顯示
extern const int MMATC_Handler      ;
extern const int MMATC_TCPIP        ;
extern const int MMATC_NI           ;
extern const int MMATC_ATC          ;
extern const int MMATC_Chiller      ;
extern const int MMATC_RS232        ;
extern const int MMATC_Head         ;
extern const int MMATC_PowerSupply  ;
extern const int MMATC_WaterValve   ;
extern const int MMSafeDoor11       ;
extern const int MMSafeDoor12       ;
extern const int MMSafeDoor13       ;
extern const int MMSafeDoor14       ;
extern const int MMSafeDoor15       ;
extern const int MMSafeDoor16       ;
extern const int MMSafeDoor17       ;   //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
extern const int MMSafeDoor18       ;
extern const int MMSafeDoor19       ;
extern const int MMSafeDoor20       ;
extern const int MMSafeDoor21       ;
extern const int MMSafeDoor22       ;

//KaiChen 20200716 ：OHT
//==>
extern const int MMMultileEmpty        ;
extern const int MMMultileEmpty_Catch  ;
extern const int MMMultileEmpty_Z      ;
//<==
//KaiChen 20200716 ：OHT

//JerryYang 20220909 : add magazine
//==>
extern const int MMMagazineTary1    ;
extern const int MMMagazineTary2    ;
extern const int MMMagazineTary3    ;
extern const int MMMagazineTary4    ;
extern const int MMMagazineTary5    ;
extern const int MMMagazineTary6    ;
extern const int MMMagazineTary7    ;
extern const int MMMagazineTary8    ;
extern const int MMMagazineTary9    ;
extern const int MMMagazineTary10   ;
extern const int MMMagazineTary11   ;
extern const int MMMagazineTary12   ;
extern const int MMMagazineTary13   ;
extern const int MMMagazineTary14   ;
extern const int MMMagazineTaryTop  ;

extern const int MMMagazineBuffer   ;

extern const int MMBackupMagazineTary1;
extern const int MMBackupMagazineTary2;
extern const int MMBackupMagazineTary3;
extern const int MMBackupMagazineTary4;
extern const int MMBackupMagazineTary5;
extern const int MMBackupMagazineTary6;
extern const int MMBackupMagazineTary7;
extern const int MMBackupMagazineTary8;
extern const int MMBackupMagazineTary9;
extern const int MMBackupMagazineTary10;
extern const int MMBackupMagazineTary11;
extern const int MMBackupMagazineTary12;
extern const int MMBackupMagazineTary13;
extern const int MMBackupMagazineTary14;
//<==
//JerryYang 20220909 : add magazine

extern const int MMAuto4Z      ;                                                //Steven 20230907 : For HT-9011UC
extern const int MMAuto5Z      ;
extern const int MMAuto6Z      ;
extern const int MManualTray4  ;
extern const int MManualTray5  ;
extern const int MManualTray6  ;
extern const int MMAuto4       ;
extern const int MMAuto5       ;
extern const int MMAuto6       ;
extern const int MMAuto4_Car   ;
extern const int MMAuto5_Car   ;
extern const int MMAuto6_Car   ;
extern const int MMFixTray4    ;
extern const int MMFixTray5    ;
extern const int MMFixTray6    ;
extern const int MTopAOIArmX;                                                   //Ian 20230823 Top AOI Function
extern const int MTopAOIArmY;
extern const int MTopAOIArmR;
extern const int MTopAOICCDZ;
//---- old cmydef.h :5809-6078 (was outside the guard): the HT9050 C_* / Sn* indexes
extern const int C_MobileTrayTableSelect         ;   //Eastsun 20260811 : add for 9050: Mobile Tray Table Select
extern const int C_MobileCassetteSelect          ;   //Eastsun 20260811 : add for 9050: Mobile Cassette Select
extern const int C_MobileAirBlow                 ;   //Eastsun 20260811 : add for 9050: Mobile Air Blow
extern const int C_LoaderDrawerLock              ;   //Eastsun 20260811 : add for 9050: Loader Drawer Lock
extern const int C_Auto1DrawerLock               ;   //Eastsun 20260811 : add for 9050: Auto1 Drawer Lock
extern const int C_Auto2DrawerLock               ;   //Eastsun 20260811 : add for 9050: Auto2 Drawer Lock
extern const int C_Auto3DrawerLock               ;   //Eastsun 20260811 : add for 9050: Auto3 Drawer Lock
extern const int C_EmptyDrawerLock               ;   //Eastsun 20260811 : add for 9050: Empty Drawer Lock
//extern const int C_OutArmSmallY                  ;   //Eastsun 20260811 : add for 9050: Out Arm small Y move   //AI(W906-PKG146) 20261005: declared once above (RULE10)
extern const int C_InPnPDrop1                    ;   //Eastsun 20260811 : add for 9050: In P&P Drop prevent 1
extern const int C_InPnPDrop2                    ;   //Eastsun 20260811 : add for 9050: In P&P Drop prevent 2
extern const int C_InPnPDrop3                    ;   //Eastsun 20260811 : add for 9050: In P&P Drop prevent 3
extern const int C_InPnPDrop4                    ;   //Eastsun 20260811 : add for 9050: In P&P Drop prevent 4
extern const int C_OutPnPDrop1                   ;   //Eastsun 20260811 : add for 9050: Out P&P Drop prevent 1
extern const int C_OutPnPDrop2                   ;   //Eastsun 20260811 : add for 9050: Out P&P Drop prevent 2
extern const int C_OutPnPDrop3                   ;   //Eastsun 20260811 : add for 9050: Out P&P Drop prevent 3
extern const int C_OutPnPDrop4                   ;   //Eastsun 20260811 : add for 9050: Out P&P Drop prevent 4
extern const int C_DieClean                      ;   //Eastsun 20260811 : add for 9050: Die Clean (Cylinder version, coexist w/ SwDieClean)
extern const int C_CasArm_Z                      ;   //Eastsun 20260818 : 9050 CasArm Z-axis extend (renamed from C_CatchTray_Z)
extern const int C_ALGrip                       ;   //Eastsun 20260811 : add for 9050: AL Gripper valve output
extern const int C_LoaderEdgeClip            ;   //Eastsun 20260812 : EdgeClip: Loader clip after separate (single tray)
extern const int C_Auto1EdgeClip             ;   //Eastsun 20260812 : EdgeClip: Auto1 clip after separate (single tray)
extern const int C_Auto2EdgeClip             ;   //Eastsun 20260812 : EdgeClip: Auto2 clip after separate (single tray)
extern const int C_Auto3EdgeClip             ;   //Eastsun 20260812 : EdgeClip: Auto3 clip after separate (single tray)
extern const int C_EmptyEdgeClip             ;   //Eastsun 20260812 : EdgeClip: Empty clip after separate (single tray)
extern const int C_CasArm_Clip               ;   //Eastsun 20260818 : 9050 CasArm clip (Buffer arm gripper cylinder MVSC-220-4E2C-6A-DC24)
//==== end local additions ====


//==== Eastsun 20260820 : missing externs + forward decls (auto-merged) ====
extern const int SnMobileTrayHasTray;   //Eastsun
extern const int SnMobileCassetteCount;   //Eastsun
extern const int SnMobileTrayCount;   //Eastsun
extern const int SnLoaderDrawerPosOver;   //Eastsun
extern const int SnAuto1DrawerPosOver;   //Eastsun
extern const int SnAuto2DrawerPosOver;   //Eastsun
extern const int SnAuto3DrawerPosOver;   //Eastsun
extern const int SnEmptyDrawerPosOver;   //Eastsun
extern const int SnLoaderDrawerPosSafe;   //Eastsun
extern const int SnAuto1DrawerPosSafe;   //Eastsun
extern const int SnAuto2DrawerPosSafe;   //Eastsun
extern const int SnAuto3DrawerPosSafe;   //Eastsun
extern const int SnEmptyDrawerPosSafe;   //Eastsun
extern const int SnLoaderDrawerHasTray;   //Eastsun
extern const int SnAuto1DrawerHasTray;   //Eastsun
extern const int SnAuto2DrawerHasTray;   //Eastsun
extern const int SnAuto3DrawerHasTray;   //Eastsun
extern const int SnEmptyDrawerHasTray;   //Eastsun
extern const int SnLoaderDrawerDoor;   //Eastsun
extern const int SnAuto1DrawerDoor;   //Eastsun
extern const int SnAuto2DrawerDoor;   //Eastsun
extern const int SnAuto3DrawerDoor;   //Eastsun
extern const int SnEmptyDrawerDoor;   //Eastsun
extern const int SnSupportFoot;   //Eastsun
extern const int SnUnderTrayArmReset;   //Eastsun
extern const int SnMultiBinStorePos1;   //Eastsun
extern const int SnMultiBinStorePos2;   //Eastsun
extern const int SnMultiBinStorePos3;   //Eastsun
extern const int SnMultiBinStorePosOK;   //Eastsun
extern const int SnMultiBinTrayNotReady;   //Eastsun
extern const int SnMultiBinTrayOK1;   //Eastsun
extern const int SnMultiBinPushReset;   //Eastsun
extern const int SnMultiBinTrayOK2;   //Eastsun
extern const int SnMultiBinPushOK;   //Eastsun
extern const int SnMultiBinEjectOK;   //Eastsun
extern const int SnMultiBinClampOK;   //Eastsun
extern const int SnEFEMReset;   //Eastsun
extern const int SnEFEMHasIC;   //Eastsun
extern const int SnCasArmHasTray;   //Eastsun
extern const int SnMobileTrayTableSelect1;   
extern const int SnMobileTrayTableSelect2;   
extern const int SnMobileTrayTableSelect3;   
extern const int SnMobileTrayTableSelect4;   
extern const int SnMobileCassetteSelect1;   
extern const int SnMobileCassetteSelect2;   
extern const int SnLoaderEdgeClip1;   
extern const int SnLoaderEdgeClip2;   
extern const int SnLoaderEdgePush1;   
extern const int SnLoaderEdgePush2;   
extern const int SnAuto1EdgeClip1;   
extern const int SnAuto1EdgeClip2;   
extern const int SnAuto1EdgePush1;   
extern const int SnAuto1EdgePush2;   
extern const int SnAuto2EdgeClip1;   
extern const int SnAuto2EdgeClip2;   
extern const int SnAuto2EdgePush1;   
extern const int SnAuto2EdgePush2;   
extern const int SnAuto3EdgeClip1;   
extern const int SnAuto3EdgeClip2;   
extern const int SnAuto3EdgePush1;   
extern const int SnAuto3EdgePush2;   
extern const int SnC_ALGrip1;   
extern const int SnC_ALGrip2;   
extern const int SnC_ALGrip3;   
extern const int SnC_ALGrip4;   
extern const int SnC_ALGrip5;   
extern const int SnC_ALGrip6;   
extern const int SnC_ALGrip7;   
extern const int SnC_ALGrip8;   
extern const int SnC_ALGrip9;   
extern const int SnC_ALGrip10;   
extern const int SnC_ALGrip11;   
extern const int SnC_ALGrip12;   
extern const int SnEmptyEdgeClip1;   
extern const int SnEmptyEdgeClip2;   
#endif
