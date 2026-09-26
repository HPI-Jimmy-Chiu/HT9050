// ===========================================================================
//  LastSet.h  --  Full-tree translation of the BCB6 golden LastSet.h
//  (persisted machine-state master struct family: LAST_GENERAL_SET / TECH /
//  INFO / VPoint / TeachInfo / ALIGNTEACH)
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/LastSet.h (cp950, 1162
//  lines) -- decoded via Python open(p,'rb').read().decode('cp950','replace')
//  per this task's reading rule; 0 U+FFFD replacement characters on decode.
//  Translation wave: W906-GA1-B1 (GA-1-B1: LastSet full-volume translation)
//  Translator: AI(W906-GA1-B1) 20260804
//
//  FIDELITY CONTRACT (per golden's own banner, preserved below verbatim):
//  every field keeps golden's exact name, declared type and array dimensions,
//  in golden's exact declaration order -- "LastSet 檔案裡的結構請不要任意加
//  東西或是修改型態". This translation adds NOTHING and reorders NOTHING;
//  the only additions are these banner/quirk-disclosure COMMENTS (this file)
//  and the four externed-object DEFINITIONS (LastSet.cpp).
//
//  RELATIONSHIP TO canary_support.h's MINIMAL SHIM:
//  canary_support.h currently carries its own `struct LAST_GENERAL_SET` (66
//  fields, a hand-picked subset needed by earlier translation waves) so that
//  123 TUs can `#include "canary_support.h"` without pulling in the full
//  ~390-field struct before it existed. That shim and THIS full struct are
//  BOTH named LAST_GENERAL_SET -- an ODR collision if the same TU includes
//  both. Per this task's brief, no existing TU is retargeted here; the shim
//  retirement (pointing all 123 includers at this file and deleting the shim
//  struct) is a follow-up integration step, not part of this translation.
//  This header must NEVER be #included from canary_support.h or vice versa.
//  See _ga1_b1_report.md for the full shim/full-struct field-by-field diff.
//
//  BCB6 TYPE MAPPING: every member below is a plain C aggregate type (char,
//  int, long, float, double, bool, unsigned int, and fixed-size arrays of
//  those, plus the two nested plain-C structs VPoint/TeachInfo) -- golden
//  LastSet.h uses NO AnsiString/TStringList/TDateTime anywhere in these six
//  types (verified by full-file scan), so this header needs no vclcompat
//  include and no __fastcall handling.
//
//  Toolchain: MinGW g++ 6.3+, C++17 (matches this tree's other W906 headers).
// ===========================================================================
//---------------------------------------------------------------------------
//LastSet檔案裡的結構請不要任意加東西或是修改型態
//就算要加也是加在最下面
//有加東西請跟大家說一下~~
//---------------------------------------------------------------------------
#ifndef LastSetH
#define LastSetH
//---------------------------------------------------------------------------

typedef struct
{
    //AI(W906-GA1-B1) 20260804: quirks preserved VERBATIM from golden (translation-
    //  fidelity disclosure -- none of these are "fixed" here; golden's own banner
    //  above forbids reordering/retyping this struct even in translation, so a
    //  naive cleanup pass could otherwise silently "correct" one of these):
    //   * TYPO kept as-is: iAMRTrayConut[9] (golden LastSet.h:502) reads "Conut"
    //     not "Count" everywhere it is used across the golden tree.
    //   * "_Old" -> live migration pairs (both members still declared, the "_Old"
    //     one now dead storage, per golden's own JerryYang 20221026 comment):
    //     BinCT[4][256] (golden:106) superseded BinCT_Old[4][20] (golden:124);
    //     BinCT_ART[4][256] (golden:107) superseded BinCT_ART_Old[4][20] (golden:385).
    //   * Three generations of the same field, same misspelling ("Histroy" not
    //     "History") carried across all three: fTempHistroy[20][60] (golden:246,
    //     oldest) -> ____dTempHistroy[40][60] (golden:355, FOUR leading
    //     underscores, an intermediate dead name) -> dTempHistroy[100][60]
    //     (golden:391, current/live). All three kept; only the widest is written
    //     by current code (per golden's own Steven 20120807/20160530 comments).
    //   * Duplicate-concept pair present TWICE at different sizes/prefixes, both
    //     still declared: _iTrayCntForSortFunc[3] / _iTrayICCntForSortFunc[3]
    //     (golden:427-428, single-underscore prefix, size 3) vs
    //     iTrayCntForSortFunc[10] / iTrayICCntForSortFunc[10] (golden:494-495, no
    //     prefix, size 10) -- same stated purpose ("Tray指定數量退盤功能"), two
    //     live copies at different bounds; do not merge.
    //   * Mixed migration-flag typing: __bBinData32 is bool (golden:364) but the
    //     newer bBinData32 (no leading underscore) is declared int, not bool
    //     (golden:367), despite the boolean-style name.
    //   * "NoUse" padding fields, three different underscore conventions for the
    //     same dead/reserved intent: __NoUseINT_2x256__[2][256] (leading AND
    //     trailing double-underscore, golden:455), __NoUseInt_1x255__[255]
    //     (golden:456, same style), __NoUseChar256[256] (golden:460, leading-only).
    //   * Commented-OUT historical fields golden itself left as dead code (not
    //     translated here either, matching golden's choice to comment rather than
    //     delete -- see the //int _SoftSpeed[60] / //int __iN14_9_TrayICCnt[12]
    //     lines a few lines below, and the whole golden:58-68 speed-migration
    //     block, all carried through unchanged as comments).
    char LastOpenFilename[512];                                                 //上次開檔檔名
    int  iAutoTeachStep;                                                        //JimmyChiu 20211020 : Auto alignment mode
    long SendCT[4];

    //Steven 20160627 : 把沒用到的_SoftSpeed[60]改給[N14_9]用
    //==>
//    int  _SoftSpeed[60];                                                      //Steven 20150910 : 不夠用,停止使用
    //int __iN14_9_TrayICCnt[12];                                                 //Tested IC
    int iHdPass;                                                                //RogerYang 20250923 : 瑞薩FT-CT
    int iHdFail;                                                                //RogerYang 20250923 : 瑞薩FT-CT
    int iHdRejectATPass;                                                        //Kirin 20200109 紀錄測試完畢被拔除的OK IC數
    int iHdRejectATFail;                                                        //Kirin 20200109 紀錄測試完畢被拔除的NG IC數
    int iHdRejectBT;                                                            //Kirin 20200109 紀錄測試完畢被拔除的未測 IC數
    int iTesterMatch;                                                           //Kirin 20200109 紀錄通訊測試完畢OK IC數
    int iTesterUnmatch;                                                         //Kirin 20200109 紀錄通訊測試完畢NG IC數
    int iTestIgnore;                                                            //Kirin 20200109 紀錄E97 IC數
    int iHdPickUp;                                                              //Kirin 20200112 Loader入料數
    int iHdPass1;                                                               //Kirin 20221115 add two pass bin
    int iHdPass2;                                                               //Kirin 20221115 add two pass bin
    int iHdRejectATPass1;                                                       //Kirin 20221115 add two pass bin

    int __iN14_9_FullTrayCnt[12];                                               //Full Tray Tray Qty
    int __iN14_9_PartialTrayCnt[12];                                            //Not Full Tray Tray Qty
    int __iN14_9_PartialTrayICCnt[12];                                          //Not Full Tray IC Qty
    int __iN14_9_InHandlerICCnt[12];                                            //In Machine Tray IC Qty
    //<==
    //Steven 20160627 : 把沒用到的_SoftSpeed[60]改給 [N14_9]用

    char SystemDateRecord[512];                                                 //Steven 20100811 沒用到
    long SystemAccSecond[4][8];                                                 //Steven 20190714 : 為了時間較精準,所以改成MS //第三組改成每12Hr計算一次, 第二組為每24Hr計算一次
    int  MessageLight[8][3];
    int  MusicSelect[9];
    bool bMusicEnable;                                                          //Steven 20100811 沒用到
    long lMaxAlarmCT;                                                           //Steven 20100811 沒用到
    long lMaxMessageCT;                                                         //Steven 20100811 沒用到
    long lMaxUserCT;                                                            //Steven 20100811 沒用到
    int  iLanguageCountry;
    bool bShowCountType;                                                        //Steven 20100811 沒用到
    int  iTemperature;
    int  iStartMode;
    int  iTester;
    int  iScanner;
    int  iRealDummy;

    bool bUT150InstallReserved[8];                                              //Steven 20100811 沒用到
    int TrayLoaderSpeed_Old[12];                                                // 0                                     //Steven 20100811 沒用到
//    int InArm1Speed_Old;             // 1                                     //Steven 20100811 沒用到
//    int HotPlateSpeed_Old;           // 2                                     //Steven 20100811 沒用到
//    int InArm2Speed_Old;             // 3                                     //Steven 20100811 沒用到
//    int ShuttleSpeed_Old;            // 4                                     //Steven 20100811 沒用到
//    int IndexSpeed_Old;              // 5                                     //Steven 20100811 沒用到
//    int OutArmSpeed_Old;             // 6                                     //Steven 20100811 沒用到
//    int TrayUnLoaderSpeed;           // 7                                     //Steven 20100811 沒用到
//    int CatchTraySpeed_Old;          // 8                                     //Steven 20100811 沒用到
//    int InArm2Retry;                 // 10                                    //Steven 20100811 沒用到
//    int TesterRetry;                 // 12                                    //Steven 20100811 沒用到
//    int OutArmRetry;                 // 13                                    //Steven 20100811 沒用到
    double SoakTime;                                                            // 23                                    //Steven 20100811 沒用到
    double JamSoakTime;                                                         // 24                                    //Steven 20100811 沒用到
    double InitialWaitTime;                                                     // 25
    double CollingTime;                                                         // 26                                    //Steven 20100811 沒用到
    int LoopMovePos[180];                                                       //Steven 20120831 : 怕會記憶體破壞，改用ini
//    int LoopSpeed[45][2];                                                     //Steven 20100811 沒用到
    float Index1Torue[12];                                                      //Steven 20120831 沒用到
//    float Index1Preasure[2];                                                  //Steven 20120831 沒用到
//    float Index2Torue[2];                                                     //Steven 20120831 沒用到
//    float Index2Preasure[2];                                                  //Steven 20120831 沒用到
//    float Index1MaxTorue;                                                     //Steven 20120831 沒用到
//    float Index1MaxPreasure;                                                  //Steven 20120831 沒用到
//    float Index2MaxTorue;                                                     //Steven 20120831 沒用到
//    float Index2MaxPreasure;                                                  //Steven 20120831 沒用到
    int   ContactSet[8];
    int   ContactCountReserved[28];                                             //Steven 20100811 沒用到
//    int  AccessLevel[20];                                                     //Steven 20120808 沒用到
    double Hot_time;                                                            //Steven 20100811 沒用到
    int ShuttleSpeed2[17];                                                      //Steven 20100811 沒用到
//    int InArmZ_UpSpeed;                                                       //Steven 20100811 沒用到
//    int InArmZ_DownSpeed;                                                     //Steven 20100811 沒用到
//    int OutArmZ_UpSpeed;                                                      //Steven 20100811 沒用到
//    int OutArmZ_DownSpeed;                                                    //Steven 20100811 沒用到
//    int InArm2Accel_Speed;                                                    //Steven 20100811 沒用到
//    int InArm2Z_Up_Accel_Speed;                                               //Steven 20100811 沒用到
//    int InArm2Z_Down_Accel_Speed;                                             //Steven 20100811 沒用到
//    int Shuttle1Accel_Speed;                                                  //Steven 20100811 沒用到
//    int Shuttle2Accel_Speed;                                                  //Steven 20100811 沒用到
//    int IndexArmY_Accel_Speed;                                                //Steven 20100811 沒用到
//    int IndexArmUp_Accel_Speed;                                               //Steven 20100811 沒用到
//    int IndexArmDown_Accel_Speed;                                             //Steven 20100811 沒用到
//    int OutArmAccel_Speed;                                                    //Steven 20100811 沒用到
//    int OutArmZUp_Accel_Speed;                                                //Steven 20100811 沒用到
//    int OutArmZDown_Accel_Speed;                                              //Steven 20100811 沒用到
//    int TrayArmAccelSpeed;                                                    //Steven 20100811 沒用到
    //JerryYang 20221026 : 新增Magazine後陣列不夠大, 新增Magazine後陣列不夠大, 把unsigned int SocketContactReserved[2][4][256]拿來用
    //==>
    unsigned int BinCT[4][256];
    unsigned int BinCT_ART[4][256];
    //<==
    //JerryYang 20221026 : 新增Magazine後陣列不夠大, 新增Magazine後陣列不夠大, 把unsigned int SocketContactReserved[2][4][256]拿來用
    bool bUseOutArmCheckIndex;                                                  //Steven 20160328 : 從LastSet改為IniConfig
    double fInArmSuckDeleyTime;                                                 //Steven 20100811 沒用到
    double fInArmDestroyDeleyTime;                                              //Steven 20100811 沒用到
    double fIndexArmSuckDeleyTime;                                              //Steven 20100811 沒用到
    double fIndexArmDestroyDeleyTime;                                           //Steven 20100811 沒用到
    double fOutArmSuckDeleyTime;                                                //Steven 20100811 沒用到
    double fOutArmDestroyDeleyTime;                                             //Steven 20100811 沒用到
    bool  bUseFinishHomeCanGo;                                                  //Steven 20100811 沒用到
    bool  bUT150Install[20];                                                    //Steven 20111012 不要用了
//    int   iMaxUT150;                                                          //Steven 20100811 沒用到
    int  iUnloadFixTray;                                                        //Steven 20260202 : for ATK AMR
    int  iCCDPurgeSetupCount;                                                   //Steven 20100811 沒用到
    double  iCCDPurgeSetupTime;                                                 //Steven 20100811 沒用到
    int  iCCDPurgeRunCount;                                                     //Steven 20100811 沒用到
    long BinCT_Old[4][20];                                                      //JerryYang 20221026 : 新增Magazine後陣列不夠大, 改用其他位置
    int  bHT8040_SACNNER;                                                       //Steven 20140409 : 沒用到 Scanner
    bool bBigFan;
    bool bInArmPlaceToShuttleCheck;                                             //Steven 20100811 沒用到
    bool bC03UseCatchTray;
    int  iShuttlePitchMode;                                                     //Steven 20100811 沒用到
    bool bPasswordCanInputByMouse;                                              //Steven 20100811 沒用到
    int  iTemptureRange;                                                        //Steven 20160328 : 從LastSet改為IniConfig
    bool bD41TestSocketICCheckSkip;                                             //Steven 20160328 : [D41]強制false, 就是不能跳過index check
    bool bTemperatureSelectByDevice;                                            //Steven 20100811 沒用到
    int  iScannerSelect;                                                        //Steven 20140409 : 沒用到 Scanner
    bool bAutoAdjustTestZDown;                                                  //Steven 20100811 沒用到, 只有9040才會用
    double iWaitIndexDestroyTime;                                               //Steven 20160328 : 從LastSet改為IniConfig
    bool bClearContactOffset;                                                   //Steven 20100811 沒用到
    bool bClearPickOffset;                                                      //Steven 20100811 沒用到
    bool bTesterFinishCanHome;                                                  //Steven 20160328 : 從LastSet改為IniConfig
    bool bIndexICFallDownMustPressFMotorDown;                                   //Steven 20160328 : 從LastSet改為IniConfig
    bool bUseSingleTenmpertureLimit;                                            //Steven 20160328 : 從LastSet改為IniConfig
    int  iSingleTempLimit[20];                                                  //Steven 20111013 取消不用
    int  iOutArmPickPlaceInterval;                                              //Steven 20100811 沒用到
    bool bCheckIndexICDestroy;                                                  //Steven 20160328 : 從LastSet改為IniConfig
    double  iTestHeadCheckVacuumTime;                                           //Steven 20160328 : 從LastSet改為IniConfig
    bool bEnableSiteModeSelect;                                                 //Steven 20160328 : 從LastSet改為IniConfig
    bool bShakeShuttleWhenJam;                                                  //Steven 20160328 : 從LastSet改為IniConfig
    int  iInArmQuickModeWaitTime;                                               //Steven 20100811 沒用到
    bool bOutputShuttleSkipICMiss;                                              //Steven 20160328 : 從LastSet改為IniConfig
    bool bLoaderTraySplitFailCanSkip;                                           //Steven 20160328 : 從LastSet改為IniConfig
    int  HeadTestCT[192];                                                       //沒用到
//    int  HeadTestPassCT[4][16];                                               //沒用到
//    int  HeadTestFailCT[4][16];                                               //沒用到
    bool bHT8080PlaceToShuttleDualDropModeAndUp;                                //Steven 20100811 沒用到
    bool bRotateInMachine;                                                      //Steven 20100811 沒用到
    bool bC02InstallCCD;
    char szSupervisor[32];
    int  iInOutArmZSpeedScale;                                                  //Steven 20100811 沒用到
    double  StartDelayTime;                                                     //Steven 20100811 沒用到
    bool bEnableReadTorque;                                                     //Steven 20160328 : 從LastSet改為IniConfig
    int  iReadTorqueTimeCount;                                                  //Steven 20160328 : 從LastSet改為IniConfig
    int  iRTMUpdateMin;                                                         //Steven 20100811 沒用到
    bool bInitialICCheck;                                                       //Steven 20160328 : 從LastSet改為IniConfig
    bool bCheckOutputShuttleContiFail;                                          //Steven 20100811 沒用到
    int  iCheckOutputShuttleContiFailCount;                                     //Steven 20100811 沒用到
    bool bResetGPIBAfterOneCycleCleanOut;                                       //Steven 20160328 : 從LastSet改為IniConfig
    bool bEnableAutoTorqueOfRunTime;                                            //Steven 20100811 沒用到
    int  iAutoTorqueOfRunTimeCount;                                             //Steven 20100811 沒用到
    bool bEnableFinishTestUpWait;                                               //Steven 20160328 : 從LastSet改為IniConfig
    int  iFinishTestUpWaitHeight;                                               //Steven 20160328 : 從LastSet改為IniConfig
    double  iD21FinishTestUpWaitTime;                                           //Steven 20160328 : 從LastSet改為IniConfig
    int  iTTLPulseWidth;                                                        //Steven 20100811 沒用到
    bool bUseOutPutSHJamSetInterFaceErrorBin;                                   //Steven 20100811 沒用到
    int  iShuttleShakeSpeed;                                                    //Steven 20160328 : 從LastSet改為IniConfig
    int  iMachineModel;                                                         //Steven 20100811 沒用到
    int  InOutArmZSafe;                                                         //Steven 20100811 沒用到
    bool bTesterTimerOutNotNeedReTest;                                          //Steven 20121112 : RS232支援32Bin 改成 iBinData32
    bool bEnableMainScreenShowAlarmUse;                                         //Steven 20100811 沒用到
    int  iCCDTypeSelect;                                                        //Steven 20100811 沒用到
    bool bAutoSwitchToOperatorMode;                                             //Steven 20160328 : 從LastSet改為IniConfig
    bool bManualHeightComptibleWithNS;                                          //Steven 20121112 : RS232支援32Bin 改成 iBinData32
    int  iJamCount[3];                                                          //0:  1:Tray Feed紀錄  2:for每天記錄  //第三組改成每12Hr計算一次, 第二組為每24Hr計算一次
    int  iTrayColor[9];                                                         //Steven 20100811 沒用到
    bool bUseTrayColor;                                                         //Steven 20100811 沒用到
    bool bOutputArmPlaceSupportDifferentOffset;                                 //Steven 20100811 沒用到
    bool bEnableSocketPurgeFunction;                                            //Steven 20160328 : 從LastSet改為IniConfig
    int  iSocketPurgeCount;                                                     //Steven 20160328 : 從LastSet改為IniConfig
    double  iSocketPurgeTime;                                                   //Steven 20160328 : 從LastSet改為IniConfig
    int  iD47SocketTestedCount;                                                 //紀錄執行次數
    int  iSocketCountReserved[96];                                              //Steven 20100811 沒用到
    char szJamClearData[3][64];                                                 //Steven 20100811 沒用到
    bool bD48PowerOffEmgCanNotUseZ1Z2;                                          //Steven 20160328 : 從LastSet改為IniConfig
    int  iUseTubeLoader;                                                        //Steven 20100811 沒用到
    bool bCompelBanishIC;                                                       //Steven 20100811 沒用到
    bool bManualChangeUnLoadTubeRecountIC;                                      //Steven 20100811 沒用到
    bool EnableUnloaderTubeFullMusic;                                           //Steven 20100811 沒用到
    bool bPickFromShuttleWaitTestFinish;                                        //Steven 20100811 沒用到
    bool bEnableShuttlePurgeFunction;                                           //Steven 20160328 : 從LastSet改為IniConfig
    int  iShuttlePurgeCount;                                                    //Steven 20160328 : 從LastSet改為IniConfig
    int  Shuttle2x2SpecSensorModeOffset;                                        //Steven 20100811 沒用到
    long HeadPass[160];                                                         //2006/04/24                                        //Steven 20100811 沒用到
//    long TestCT[4][32];   //2006/04/24                                        //Steven 20100811 沒用到
    char szMachineAlias[256];                                                   //Steven 20100811 沒用到
//    int  iCatCount[4][64];                                                    //Steven 20100811 沒用到
    int iUnloaderTrayCount_ART[256];
    int  iMaxPreasureIndex;                                                     //Steven 20121112 : RS232支援32Bin 改成 iBinData32
    int  i2x2ModeSelect;                                                        //Steven 20100811 沒用到
    int iCategoryLimitCount;                                                    //Steven 20100811 沒用到
    bool bEnableCategoryLimit;                                                  //Steven 20100811 沒用到
    int iBinData[4][16];                                                        //Steven 20121112 : RS232支援32Bin 改成 iBinData32
    // for RTM
    char sTesterNo[512];                                                        //Steven 20100811 沒用到
//    char sLotNo[256];                                                         //Steven 20100811 沒用到
    int  iProdutMode;                                                           //Steven 20100811 沒用到
    char szHost[768];                                                           //Steven 20100811 沒用到
//    char szUserName[256];                                                     //Steven 20100811 沒用到
//    char szPassword[256];                                                     //Steven 20100811 沒用到
    bool bIndexReleaseHeightSetToAbsolute;                                      //Steven 20100811 沒用到
    int  iAutoOneCycleCount;                                                    //Steven 20100811 沒用到
    char szTesterName[256];                                                     //Steven 20100811 沒用到
    bool bSupportMultiDoubleContact;                                            //Steven 20121112 : RS232支援32Bin 改成 iBinData32
    int  iDoubleContactCount;                                                   //Steven 20160328 : 從LastSet改為IniConfig
    bool bOffReadTorqueDuringTest;                                              //Steven 20121112 : RS232支援32Bin 改成 iBinData32
    bool bSupportIODoubleContact;                                               //Steven 20100811 沒用到
    double  iWaitGetEpsonTime;                                                  //Steven 20100811 沒用到
    int i9043Use2SuckXPitchLimit;                                               //Steven 20100811 沒用到
    int iColorIsEmptyUnloader;                                                  //Steven 20160328 : 從LastSet改為IniConfig
    int i9043HotPlateOnlyUsesFourRows;                                          //Steven 20100811 沒用到

    int iInArmVacuumDummyOnTime[4][8];
    int iOutArmVacuumDummyOnTime[4][8];
    int iFTestArmVacuumDummyOnTime[4][8];
    int iBTestArmVacuumDummyOnTime[4][8];

    int iInArmVacuumDummyOffTime[4][8];
    int iOutArmVacuumDummyOffTime[4][8];
    int iFTestArmVacuumDummyOffTime[4][8];
    int iBTestArmVacuumDummyOffTime[4][8];

    int iCatchArmVacuumDummyOnTime;
    int iCatchArmVacuumDummyOffTime;

    int iLoaderTraySimulateTime;
    int iUnLoaderTraySimulateTime[3];

    float fTempHistroy[20][60];                                                 //Steven 20120807 : 記憶體破壞,fTempHistroy[20][60]->dTempHistroy[40][60]
    int   iTempRecordInterval;                                                  //Steven 20160328 : 從LastSet改為IniConfig
    int  iSocketCount[1536];                                                    //Steven 20100811 沒用到
    bool bUse2Suck;                                                             //Steven 20100811 沒用到
    bool bUse2SuckBefore;                                                       //Steven 20100811 沒用到
    int  iCompressedHeight;                                                     //Steven 20100811 沒用到
    char strNetEasySocketCountPath[128];                                        //Steven 20100811 沒用到
    char strNetEasySocketID[512];                                               //Steven 20100811 沒用到
//    int UPHChart[64];                                                         //Steven 20100811 沒用到
    int iCloseSiteByLowYield[2][4][8];                                          //Steven 20220223 : 紀錄Auto Site Off的位置
    char UPHIndex[512];                                                         //Steven 20100811 沒用到
    int UPHChart2[64];                                                          //Steven 20100811 沒用到
    int iAmbientDefine;                                                         //Steven 20100811 沒用到
    char strNetEasySocketAlias[32];                                             //Steven 20100811 沒用到
    bool bUsePitchSingleOffSet;                                                 //Steven 20100811 沒用到
    bool bUseOnecycleCleanOutFinishTestArmAtRear;                               //Steven 20160328 : 從LastSet改為IniConfig
    bool bInterFaceErrHeadNeedUp;                                               //ChungHung 20140516 名稱應該改為TimeOutHeadNeedUp //Steven 20160328 : 從LastSet改為IniConfig
    bool UseStandSiteMap;                                                       //Steven 20100811 沒用到
    bool bUseAfterHomeCarryAndSuckIcToRBin;                                     //Steven 20160328 : 從LastSet改為IniConfig
    int iUseWhichZsDuringDualSiteMode;                                          //Steven 20100811 沒用到
    int iMonitorWhichIndexArmTorque;                                            //Steven 20100811 沒用到
    bool bMonitorIndexTorque;                                                   //Steven 20100811 沒用到
    bool bLoadEPData;
    float fEPrefdata[100];
    float fEwrddata[100];
    int iEPValidCount;                                                          //Steven 20100811 沒用到
    int iTemperatureOffsetByDifferentType;                                      //Steven 20100811 沒用到
    bool bNotSupportPinForceInput;                                              //Steven 20100811 沒用到
    int  iD41SocketInitialICCheckPosition;
    bool AutoModeChange;                                                        //Steven 20100811 沒用到
    bool SoakTimeInitAtHoPlate;                                                 //Steven 20100811 沒用到
    bool SaveSTIfmation;                                                        //Steven 20100811 沒用到
    bool bCTClear[2][7];
    bool ClearFailTestCateAtRT;                                                 //Steven 20100811 沒用到
    bool ClearFailTraySortAtRT;                                                 //Steven 20100811 沒用到
    bool ClearFailScanCateAtRT;                                                 //Steven 20100811 沒用到
    bool ShowLoaderCT;                                                          //Steven 20160328 : 從LastSet改為IniConfig
    bool ShowContactCT;                                                         //Steven 20160328 : 從LastSet改為IniConfig
    bool ShowTestCate;                                                          //Steven 20160328 : 從LastSet改為IniConfig
    bool ShowScanCate;                                                          //Steven 20160328 : 從LastSet改為IniConfig
    bool ShowTemper;                                                            //Steven 20160328 : 從LastSet改為IniConfig
    bool ShowBinCT;                                                             //Steven 20160328 : 從LastSet改為IniConfig
    bool ShowUPH;                                                               //Steven 20160328 : 從LastSet改為IniConfig
    bool ShowIndexTime;                                                         //Steven 20160328 : 從LastSet改為IniConfig
    bool bUseTestSocket__[4][8];                                                //ChungHung 20130910 alter for SCK can close site by Index 不夠用換到下面去不能刪
    float fBalanceTheIndexOffsetValue[4];                                       //Steven 20100811 沒用到
    int bShowCateByArm;                                                         //Steven 20160328 : 從LastSet改為IniConfig
    int iRunStartMode;
    int iFormPos[5][2];                                                         //Steven 20100811 沒用到
    bool bNoUse;                                                                //Eliot 2008_01_02     //Steven 20100811 沒用到
    int iChamberTemptureRange;                                                  //Steven 20160328 : 從LastSet改為IniConfig
    bool bCCDLightOn;                                                           //Steven 20160328 : 從LastSet改為IniConfig
    bool bResetNotClearPlate;                                                   //Steven 20160328 : 從LastSet改為IniConfig
    bool bUseNewTTLBoard;                                                       //Eliot 2008_02_01
    bool bEnableTestTempIC;                                                     //Steven 20160328 : 從LastSet改為IniConfig
    bool bSystemInstallScanner;                                                 //Steven 20140409 : 沒用到 Scanner

    //Steven 20090710 Start : add XY Scale
    //XY等比例縮放                                                              //硬體相關, 不要移去IniConfig
    bool bE31OutArmUseDifferentScale;
    bool bE30InArmUseDifferentScale;
    bool bE32ShuttleUseDifferentScale;
    double fLoaderTrayXScale;
    double fLoaderTrayYScale;
    double fHotPlateXScale[2];
    double fHotPlateYScale[2];
    double fTrayXScale[6];
    double fTrayYScale[6];
    double fInShuttleXScale[2];
    double fInShuttleYScale[2];
    double fOutShuttleXScale[2];
    double fOutShuttleYScale[2];
    //Steven 20090710 End

    bool bShow_TestRate;                                                        //Show UPH for each Tray changing               //Steven 20160328 : 從LastSet改為IniConfig
    bool bShowFailAlarmCount;                                                   //jou 980717 add from ht9040 show alarm enabled //Steven 20160328 : 從LastSet改為IniConfig
    bool bShowSpeedMessage;                                                     //Steven 20090720 Show Speed on uShowMessage    //Steven 20160328 : 從LastSet改為IniConfig
    bool bShowLotInfo;                                                          //Steven 20090811                               //Steven 20160329 : 不要用這個了
    int  iOutShuttleSensorMode;                                                 //Steven 20160328 : 從LastSet改為IniConfig
    bool bCheckICWhichFirstTimeLoad;                                            //Steven 20090907           //Steven 20160328 : 從LastSet改為IniConfig
    bool bIndexPickICMissCheckTheShuttleByL7Sensor;                             //Steven 20090907       //Steven 20100811 沒用到
    int iLightOnMin;                                                            //Steven 20090919 : For Index Light on Period       //Steven 20160328 : 從LastSet改為IniConfig

    int TempPosShift_Shuttle[2][2];                                             //jou 980928 start : High tempture 130 deg Position shift // [0][0] = Shuttle 1 Left  ;  [0][1] = Shuttle 1 Right

    bool bUnloaderCounter;                                                      //Steven 20091214 : If function enabled.     //Steven 20100811 沒用到
    int  iUnloaderSetCounter;                                                   //Steven 20091214 : After how many trays on Auto 1~3 should be alarm.  //Steven 20100811 沒用到
    int  iUnloaderRealCounter[3];                                               //Steven 20091214 : How many trays on Auto 1~3 now.    //Steven 20100811 沒用到
    bool bIndexPickICShuttlePause;                                              //jou 990104 confing -> D42     //Steven 20160328 : 從LastSet改為IniConfig
    bool bFixedTrayProposeTheInitialQuestion;                                   //Steven 20100205 : Config P10  //Steven 20160328 : 從LastSet改為IniConfig
    bool bEnableChangeBinDuringTesting;                                         //Steven 20160328 : 從LastSet改為IniConfig
    int  iErrorBinAlphabet;                                                     //Steven 20100413           //Steven 20160328 : 從LastSet改為IniConfig
    bool bBalanceTheIndexOffset;                                                //Eliot 2008_03_24  //Steven 20100811 沒用到，馬克掉
    bool bInOutArmZOffsetSameOne;                                               //jou 2010-06-21            //Steven 20160328 : 從LastSet改為IniConfig
    float dD41SocketInitialCheckOffset;                                         //Steven 20100818 :[D41]
    bool bShowTimeInfo;                                                         //Steven 20100827 : [G06]       //Steven 20160328 : 從LastSet改為IniConfig
    bool bShowContactHeight;                                                    //Steven 20100827: [G07]        //Steven 20160328 : 從LastSet改為IniConfig
    bool bOutShuttleUseFrontRearSensor;                                         // jou 2010-09-09 [F11] Out Shuttle 使用前後對照Sensor檢查殘留IC    //Steven 20160328 : 從LastSet改為IniConfig
    bool bEnableAutoTrayEdgePushCylinderLoop;                                   //Steven 20100915 : [P13]跳蛋功能       //wei 20160309 LastSet-->IniConfig
    int  iEdgePushCylinderLoopDelay;                                            // 2010.08.19 , Joye [P13-1]            //Steven 20160328 : 從LastSet改為IniConfig
    bool bEnableAutoTrayRecevieDelayCount;                                      //Steven 20100915 : [P14]跳蛋功能       //wei 20160309 LastSet-->IniConfig
    int  iAutoTrayRecevieDelayCount;                                            // 2010.08.23 , Joye [P14-1]            //Steven 20160328 : 從LastSet改為IniConfig
    int  iAutoTrayRecevieLoopDelayTime;                                         // 2010.08.23 , Joye [P14-2]            //Steven 20160328 : 從LastSet改為IniConfig
    bool bResetNeedRemoveAllTray;                                               //Eliot 2010_03_05                      //Steven 20160328 : 從LastSet改為IniConfig
    int  iA01ChangeOpTime;                                                      //Steven 20101109 轉成op的時間          //Steven 20160328 : 從LastSet改為IniConfig
    bool bHasDownloadFile;                                                      //Steven 20101209 安靠要求檢查檔案有下載後,才可以按Start
    bool bInOutArmPitchZOffsetSameOne;                                          //jou 2010-12-20 Pitch & Z 縮減為一個   //Steven 20160328 : 從LastSet改為IniConfig

    int iContactCT[2];                                                          //jou 2012-06-05
    double ____dTempHistroy[40][60];                                            //Steven 20120807 : 記憶體破壞,fTempHistroy[20][60]->dTempHistroy[40][60]

    //ChungHung 20140401 change postion
    bool bUseTestSocket[2][4][8];                                               //ChungHung 20130910 alter for SCK can close site by Index

    long iIndexInputOutPut[4];                                                  //kevin 20120511 index input output 計數ic 0:input 1:output  2:outarm  3:InArm Pick

    int  iBinBoxCount;                                                          //jou 2012-12-11 support Bin Box
    int  __iBinData32[4][100];                                                  //kevin 20140305 iBinData32 換位置與大小       //Steven 20121112 : RS232支援32Bin 改成 iBinData32
    bool __bBinData32;                                                          //Steven 20121112 : RS232支援32Bin 用來將舊的資料轉到新的
    bool bInshuttleDetect;                                                      //kevin 20140206 input SHUTTLE 生產模式第9顆sensor 進入偵測是否有ic  //Steven 20160328 : 從LastSet改為IniConfig
    int  iBinData32[4][260];                                                    //kevin 20140305 iBinData32 換位置與大小                //Steven 20121112 : RS232支援32Bin 改成 iBinData32
    int  bBinData32;                                                            //Steven 20140403 : 256Bin 用來將舊的資料轉到新的

    //ChungHung 20141002 add for KYEC AutoRetest
    bool bLoaderTrayCount_ART;
    int  iLoaderTrayCount_ART;
    int  iAutoRetestCount_ART;
    int  iEmptyTrayCount_ART;
    int  iColorTrayCount_ART;
    int  __iUnloaderTrayCount_ART;
    int  iInputLoaderCount;
    bool bCleanOut_ART;
    int  iLoaderTotalTray;
    int  iASEContact;                                                           //kevin 20141020 ASE 記錄此批一CONTRACT 次數

    int  iIndexCount;                                                           //wei 20141201 Low Yield Auto Clean
    int  iRunCheckAlarmTime;                                                    //wei 20150512  Run Check Alarm Time                //Steven 20160328 : 從LastSet改為IniConfig

    long SendCT_ART[4];                                                         //kevin 20150615  ART  LOAD數量
    long BinCT_ART_Old[4][20];                                                  //kevin 20150615  ART                               //JerryYang 20221026 : 新增Magazine後陣列不夠大, 改用其他位置
    int  iBinData32_ART[4][260];                                                //kevin 20140305 iBinData32 換位置與大小
    int  SoftSpeed[200];                                                        //Steven 20150910
    int  TrayCount[10];                                                         //wei 20151117 OCR Lot check
    bool   bUpdateIndexLoadRateToLastSet;                                       //Steven 20160329 : 把Load Rate轉成LastSet
    double dIndexLoadRate[3][4];                                                //Steven 20160329 : 把Load Rate轉成LastSet, 前面是 0: HT, 1: NS, 2: Offset; 後面是 0:60, 1:56, 2:40, 3:30
    double dTempHistroy[100][60];                                               //Steven 20160530 : fTempHistroy[40][60]->dTempHistroy[100][60]
    bool bUseTestSocketEE[2][4][8];                                             //Alick 20160926 add for 第三組工程師用開關SITE
    long lShuttleCount;                                                         //Steven 20170113 : For count place to shuttle
    long _lSCKARTBinCT[20];                                                     //Steven 20170113 : For count unload count
    char strSocketID[4][8][128];                                                //JerryYang 20170206 (wei) socket ID
    int iSocketContactCount[4][8];                                              //JerryYang 20170207 (wei) socket contact次數

    //Steven 20170314 (Jou) : ART的變數移到LastSet
    //==>
    bool bWaitStartLotAutoRetestGPIB;                                           //jou 2015-10-02 Auto Retest GPIB mode
    bool bWaitEndLotAutoRetestGPIB;                                             //jou 2015-10-02 Auto Retest GPIB mode
    bool bEndLotAutoRetestGPIB;                                                 //jou 2015-10-02 Auto Retest GPIB mode
    bool bFinEndLotAutoRetestGPIB;                                              //jou 2015-10-02 Auto Retest GPIB mode
    int  iRetestFlagART;                                                        //jou 2015-10-02 Auto Retest GPIB mode
    bool bFirstTestAutoRetestGPIB;                                              //jou 2015-10-02 Auto Retest GPIB mode
    int  iSCKARTInputCT;
    //<==
    //Steven 20170314 (Jou) : ART的變數移到LastSet
    long SendCTLot;                                                             //wei 20171130 (Steven) Count By Lot
    int iSCKART_RTUnitCount;                                                    //Steven 20171226 (Wei) : Add for SCK ART

    long __OEETrayCT[20];                                                       //Steven 20190621 : OEE Function                        //位置不夠多, 改去下面
    long OEEBinCT[260];                                                         //Steven 20190621 : OEE Function
    bool bNeedSetupTeach;                                                       //JerryYang 20180921 Setup Teach功能

    int iBarcodeTrayRecFileRetestCT;                                            //jou 20190930 : Barcode Tray record file
    int __iDevice_Info_By_Tray[10];                                             //jou 20190930 : Barcode Tray record file

    int iTCPModeLotState;                                                       //Sam 20200311 : Fix TCP ART MODE
    bool bBreakSCKART;                                                          //Sam 20200311 : Fix TCP ART MODE

    int __RecodeTrayCount[10];                                                  //frank 20200814 : 每10盤記錄一次summary log
    int iLoaderCount;                                                           //Sam 20210107 : Summary Report fuction

    long __BinCT_PTI[4][20];                                                    //Sam 20210505 : PTI 要求的兩段 Low Yeild               //位置不夠多, 改去下面

    int _iTrayCntForSortFunc[3];                                                //Steven 20210608 : Tray指定數量退盤功能
    int _iTrayICCntForSortFunc[3];                                              //Steven 20210608 : Tray指定數量退盤功能

    bool bAutoSiteOffSocket[2][4][8];                                           //JimmyChiu 20211015 : only user disabled site keep disabled and other sites change to enabled

//    char cLastTimeAutoTeachDoneSetupFileName[255];
//    bool bNeedAutoTeach;
//    bool bNeedShuttleSensorCheck;
//    bool bNeedAutoHeight;
    //JimmyChiu 20211020 : Auto alignment mode
    //==>
    bool bNeedInShuttleTeach[2][2];                                             //Sam 20211130 : [0][0] InShuttle1 Kit0, [0][1] InShuttle1 Kit1, [1][0] InShuttle2 Kit0, [1][1] InShuttle2 Kit1
    bool bNeedOutShuttleTeach[2][2];                                            //Sam 20211130 : [0][0] OutShuttle1 Kit0, [0][1] OutShuttle1 Kit1, [1][0] OutShuttle2 Kit0, [1][1] OutShuttle2 Kit1
    bool bNeedUnloaderTeach;
    int  iATLoaderCount;
    //<==
    //JimmyChiu 20211020 : Auto alignment mode

    int iDaySendCT;                                                             //jou 20210108 : 上海偉測要求新增每日jam rate統計
    int iDayJamCount;                                                           //jou 20210108 : 上海偉測要求新增每日jam rate統計

    int iBinCTForAlways[4][20];                                                 //jou 20211109 : 上海偉測要求新增每日jam rate統計
    unsigned int iSiteBinCTForAlways[4][8][256];                                //jou 20211109 : 上海偉測要求新增每日jam rate統計
    unsigned int iSiteTotalCTForAlways[4][8];                                   //jou 20211109 : 上海偉測要求新增每日jam rate統計
    char cLotStartTimeForAlways[20];                                            //jou 20211109 : 上海偉測要求新增每日jam rate統計
    int iAutoTempOfsTriggerCnt;                                                 //Sam 20220406 : 溫度自動補償功能 By FTP
    long lBinCT_AutoBin[10][256];                                               //pig 20211022 AddIndexUseSortingAction

    int  __NoUseINT_2x256__[2][256];
    int  __NoUseInt_1x255__[255];
    int  iInTrayNum;                                                            //Ifor 20210621 add:Production Log Tray 連續
    int  iOutTrayNum[256];                                                      //Ifor 20210621 add:Production Log Tray 連續

    char __NoUseChar256[256];

    long OEETrayCT[256];                                                        //Steven 20190621 : OEE Function
    long BinCT_PTI[4][256];                                                     //Sam 20210505 : PTI 要求的兩段 Low Yeild
    int  iDevice_Info_By_Tray[256];                                             //jou 20190930 : Barcode Tray record file
    int  RecodeTrayCount[256];                                                  //frank 20200814 : 每10盤記錄一次summary log
    int  iN14_9_TrayICCnt[256];                                                 //Tested IC
    int  iN14_9_FullTrayCnt[256];                                               //Full Tray Tray Qty
    int  iN14_9_PartialTrayCnt[256];                                            //Not Full Tray Tray Qty
    int  iN14_9_PartialTrayICCnt[256];                                          //Not Full Tray IC Qty
    int  iN14_9_InHandlerICCnt[256];                                            //In Machine Tray IC Qty

    int OLPSetBinErr[10];                                                       //Sam 20230921 : Bin 設定錯誤不能啟動  OLPSetBinErr[0]是CATEGORY_REQUEST設定錯誤。OLPSetBinErr[1]是BINDEFINE_REQUEST設定錯誤。OLPSetBinErr[2]是FIXTRAYDEFINE_REQUEST設定錯誤
    double fLoaderTrayXScale_Hot;                                               //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
    double fLoaderTrayYScale_Hot;
    double fHotPlateXScale_Hot[2];
    double fHotPlateYScale_Hot[2];
    double fTrayXScale_Hot[6];
    double fTrayYScale_Hot[6];
    double fInShuttleXScale_Hot[2];
    double fInShuttleYScale_Hot[2];
    double fOutShuttleXScale_Hot[2];
    double fOutShuttleYScale_Hot[2];

    double fLoaderTrayXScale_Cold;
    double fLoaderTrayYScale_Cold;
    double fHotPlateXScale_Cold[2];
    double fHotPlateYScale_Cold[2];
    double fTrayXScale_Cold[6];
    double fTrayYScale_Cold[6];
    double fInShuttleXScale_Cold[2];
    double fInShuttleYScale_Cold[2];
    double fOutShuttleXScale_Cold[2];
    double fOutShuttleYScale_Cold[2];                                           //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
    int iTrayCntForSortFunc[10];                                                //Steven 20210608 : Tray指定數量退盤功能
    int iTrayICCntForSortFunc[10];                                              //Steven 20210608 : Tray指定數量退盤功能
    int iP57_InputCT;                                                           //Sam 20250605 : Loader Count AutoCleanOut

    //int REN_iCloseFTCT;                                                         //RogerYang 20250812 : 瑞薩FT-CT
//    bool REN_bRecvFTCTProdInfoCloseMessage;                                     //RogerYang 20250812 : 瑞薩FT-CT
//    int REN_iUseControlLoaderInputCount;                                        //RogerYang 20250812 : 瑞薩FT-CT

    int iAMRTrayConut[9];                                                       //Sam 20240304 : 新增 AMR 功能
    int iAMRTrayLoaderTotal;
    bool bAMRTrayFeedWait;
    bool bAMRLoaderLast;
    bool bAMRRequestSupplyTray;
    long lSCKARTBinCT[256];                                                     //Steven 20260316 : Fixed for SCK ART Bin Count
    char         szBundleTrayID_ATK_Backup[3][256];                             //AI(ht9045-atk-amr-flow) 20260421 (RogerYang) : ATK SVID backup
    int          iUnloaderTrayCount_ART_ATK_Backup[3];                          //Auto1~3 TrayCount backup
    unsigned int iBinCT_ATK_Backup[3];                                          //Auto1~3 BinCT backup
    bool         bFixHadIC_ATK_Backup[3];                                       //AI(ht9045-atk-amr-flow) 20260423 (RogerYang) : Fix1~3 HasIC backup
    bool         bAutoHadIC_ATK_Backup[3];                                      //Auto1~3 HasIC backup
    int  iBinBaseRT[256];                                                       //AI(ht9045-clearcount-flow) 20260714 (RogerYang) : SCK O25 進RT前Count Bin基準(隨iBinData32持久化;RT中途重開續RT不歸零) 256=TEST_MAX_BIN
    bool bO25_RTBaselined;                                                      //AI(ht9045-clearcount-flow) 20260714 (RogerYang) : SCK O25 本RT段是否已取基準(持久化,避免重開重取)
    //AI(W906-TOWERLIGHT) 20260926 (Steven RULINGS S45): 上面兩行照 golden V912 LastSet.h:513-514 逐字補上（型別、順序、註解）——「應該是912版本的新功能，所以需要加入」。
    //  補上後 sizeof(LAST_GENERAL_SET) 由 178896 變 179928（＝V912）；移植樹目前沒有任何讀寫者（golden 的讀寫者在 SCK O25 流程，未翻）。
} LAST_GENERAL_SET;
extern LAST_GENERAL_SET LastSet;
//---------------------------------------------------------------------------
//LastSet檔案裡的結構請不要任意加東西
//就算要加也是加在最下面
//有加東西請跟大家說一下~~
//------------------------------------------------------------------------------
typedef struct
{
    //AI(W906-GA1-B1) 20260804: quirks preserved VERBATIM from golden (same
    //  fidelity-disclosure rule as LAST_GENERAL_SET above):
    //   * 1D "_"-prefixed legacy array -> 2D live array migration pairs, gated by
    //     the `bAOAMatrix` flag below per golden's own Steven 20230801 comment
    //     ("判斷是否已經改成二維陣列"): _iInArmZBasePickerAlignmentPos[8] (old,
    //     golden ~:899) vs iInArmZBasePickerAlignmentPos[2][8] (live, golden
    //     ~:968); same pattern repeats for OutArmZBasePickerAlignmentPos,
    //     InArmCCD_Picker_PosX/Y/Z, InArmCCD_Pitch_PosX, OutArmCCD_Picker_PosX/Y/Z
    //     and OutArmCCD_Pitch_PosX (each has both an `_`-prefixed [8] legacy form
    //     and a [2][8] live form). Both forms of each pair are kept.
    //   * Four bare dead placeholders with sequential numeric suffixes and no
    //     other distinguishing name: _NoUse1, _NoUse2, _NoUse3, _NoUse4 (golden
    //     ~:784-787), each individually commented "//沒用到" in golden.
    int iZLoad;
    int iInArmLoadStageX;
    int iInArmLoadStageY;
    int iInArmLoadStagePickZ1;
    int iInArmLoadStagePickZ2;
    int iInArmPlate1X;
    int iInArmPlate1Y;
    int iInArmPlate2X;
    int iInArmPlate2Y;
    int iInArmPlatePickZ1;
    int iInArmPlatePickZ2;
    int iInArmSafeZ1;
    int iInArmSafeZ2;
    int iInArmShuttle1X;
    int iInArmShuttle1Y;
    int iInArmShuttle2X;
    int iInArmShuttle2Y;
    int iInShuttle1Left;
    int iInShuttle1Right;
    int iInShuttle2Left;
    int iInShuttle2Right;
    int iOutShuttle1Left;
    int iOutShuttle1Right;
    int iOutShuttle2Left;
    int iOutShuttle2Right;

    int iTestZ1ShutlePick;
    int iTestZ2ShutlePick;
    int iTestZDown;
    int iTestZ1ShutleWait;
    int iTestZ2ShutleWait;

    int iOutArmShuttle1X;
    int iOutArmShuttle1Y;
    int iOutArmShuttle2X;
    int iOutArmShuttle2Y;
    int iOutArmShuttlePickZ1;
    int iOutArmShuttlePickZ2;
    int iOutArmAuto1X;
    int iOutArmAuto1Y;
    int iOutArmAuto2X;
    int iOutArmAuto2Y;
    int iOutArmAuto3X;
    int iOutArmAuto3Y;
    int iOutArmSafeZ1;
    int iOutArmSafeZ2;
    int iOutArmPlaceZ1;
    int iOutArmPlaceZ2;
    int iOutArmFix1X;
    int iOutArmFix1Y;
    int iOutArmFix2X;
    int iOutArmFix2Y;
    int iOutArmFix3X;
    int iOutArmFix3Y;
    int iTrayXLoader;
    int iTrayXEmpty;
    int iTrayXColor;
    int iTrayXAuto1;
    int iTrayXAuto2;
    int iTrayXAuto3;
    int EmptyLoaderZ;
    int ColorLoaderZ;
    int iAuto1Z;
    int iOutArmXInspect;                                                        //Steven 20120921 : 沒用到
    int iOutArmYInspect;                                                        //Steven 20120921 : 沒用到
    int iOutArmZ2Inspect;                                                       //Steven 20120921 : 沒用到

    int iInArmZHeightSub[2][4];
    int iOutArmZHeightSub[2][4];

    int iShuttle1BusyHalfOffset;
    int iShuttle2BusyHalfOffset;

    int iOutArmPlaceFixZ1;

    int iInArmX40Pitch;                                                         //Steven 20131002 : XY變距      //iInArmW50Pitch
    int iInArmX120Pitch;                                                        //iInArmW80Pitch
    int iOutArmX40Pitch;                                                        //iOutArmW50Pitch
    int iOutArmX120Pitch;                                                       //iOutArmW80Pitch
    int TechInArmPitchX;
    int TechInArmPitchY;
    int TechOutArmPitchX;
    int TechOutArmPitchY;

    int iInArmRearOffsetX;
    int iInArmRearOffsetY;

    int iInArmX40Pitch2;                                                        //Steven 20131002 : XY變距       //iInArmW50Pitch2
    int iInArmX120Pitch2;                                                       //iInArmW80Pitch2
    int iOutArmX40Pitch2;                                                       //iOutArmW50Pitch2
    int iOutArmX120Pitch2;                                                      //iOutArmW80Pitch2
    //9040 start--------------
    int iHT9040TestY1_Front;
    int iHT9040TestY1_Middle;
    int iHT9040TestY2_Middle;
    int iHT9040TestY2_Rear;
    int iHT9040TestZ1_PlaceSH2;
    int iHT9040TestZ2_PickSH1;
    //9040 end-----------------

    int iXInArmRotateTECH;
    int iYInArmRotateTECH;
    int iXOutArmRotateTECH;
    int iYOutArmRotateTECH;
    int iTransKitHeight;

    int i9080Shuttle2SensorDetectPos;

    int TechUpdate_2005_10_21;
    int iInArmX40Pitch_Offset;                                                  //Steven 20131002 : XY變距 //iInArmW50Pitch_Offset
    int iInArmX120Pitch_Offset;                                                 //iInArmW80Pitch_Offset
    int iOutArmX40Pitch_Offset;                                                 //iOutArmW50Pitch_Offset
    int iOutArmX120Pitch_Offset;                                                //iOutArmW80Pitch_Offset

    int iInArmX40Pitch2_Offset;                                                 //iInArmW50Pitch2_Offset
    int iInArmX120Pitch2_Offset;                                                //iInArmW80Pitch2_Offset
    int iOutArmX40Pitch2_Offset;                                                //iOutArmW50Pitch2_Offset
    int iOutArmX120Pitch2_Offset;                                               //iOutArmW80Pitch2_Offset

    int iOutArmX40Pitch_Offset_RotateOn;                                        //iOutArmW50Pitch_Offset_RotateOn
    int iOutArmX120Pitch_Offset_RotateOn;                                       //iOutArmW80Pitch_Offset_RotateOn
    int iOutArmX40Pitch2_Offset_RotateOn;                                       //iOutArmW50Pitch2_Offset_RotateOn
    int iOutArmX120Pitch2_Offset_RotateOn;                                      //iOutArmW80Pitch2_Offset_RotateOn
    int XInArm_TubeLoadKit_Pick;
    int YInArm_TubeLoadKit_Pick;

    int XOutArm_TubeUnLoadKit_Place;
    int YOutArm_TubeUnLoadKit_Place;

    int ZInArm_TubeLoadKit_Pick;
    int ZOutArm_TubeUnLoadKit_Place;
    int ZInArm_ShuttlePlace;

    int iTubeSelectTubeBufferGet;
    int iTubeSelectTrackPut[2];

    int i9080Shuttle2SensorDetectPosFor16Site;

    int iInArmX40Pitch_Offset_RotateOn;                                         //Steven 20131002 : XY變距    //iInArmW50Pitch_Offset_RotateOn
    int iInArmX120Pitch_Offset_RotateOn;                                        //iInArmW80Pitch_Offset_RotateOn
    int iInArmX40Pitch2_Offset_RotateOn;                                        //iInArmW50Pitch2_Offset_RotateOn
    int iInArmX120Pitch2_Offset_RotateOn;                                       //iInArmW80Pitch2_Offset_RotateOn
    int iTrayXEmpty1;

    int Shuttle2ForArm1Place_Offset;
    int Shuttle2ForArm2Place_Offset;
    int Shuttle2ForArm1Place_Offset2;
    int Shuttle2ForArm2Place_Offset2;
    int OutSH1ZDetectPos;
    int OutSH2ZDetectPos;
    int iInArmShuttlePlaceZ;

    //jou 981203 start : in shuttle sensor 7
    int iInSH1Sen7DetectPos;
    int iInSH2Sen7DetectPos;
    //jou 981203 end

    //jou 2010-08-31 start : Out Shuttle Z One Row Sensor
    int OutSH1ZOneRowDetectPos;
    int OutSH2ZOneRowDetectPos;
    //jou 2010-08-31 end

    //jou 2012-05-21 start : Auto Clean
    int iInArmAutoCleanX;
    int iInArmAutoCleanY;
    //jou 2012-05-21 end

    //Steven 20120706 : 加入OCR與Y變距
    int iOcrX;
    int iOcrY;

    //Steven 20120822 : 加入Tray Z軸馬達
    int iTrayLoaderZ[6];

    //Steven 20120809 : 吸嘴自動校正
    int iInArmPickX;
    int iInArmPickY;
    int iOutArmPickX;
    int iOutArmPickY;
    //2013-01-15    Dell    Add nn Mode
    int iHT9046LSTestY1_Middle;
    int iHT9046LSTestY2_Middle;
    //Steven 20120921 : Barcode_2D
    int iInSH1BarCodePos;
    int iInSH2BarCodePos;
    int iOutSH1BarCodePos;
    int iOutSH2BarCodePos;
    //jou 2012-12-11 start : Bin Box
    int iOutArmBinBoxX;
    int iOutArmBinBoxY;
    //jou 2012-12-11 end

    //2013-04-12    Dell :旋轉站;馬達版
    int M_In_iRotateX;
    int M_In_iRotateY;
    int M_In_iRotateA;
    int M_Out_iRotateX;
    int M_Out_iRotateY;
    int M_Out_iRotateA;
    int M_In_iRotatePick;
    int M_In_iRotatePlace;
    int M_Out_iRotatePick;
    int M_Out_iRotatePlace;
    int M_In_iRotateA_Backlash;                                                 //RogerYang 20260113 : Rotator新增背隙補償
    int M_Out_iRotateA_Backlash;                                                //RogerYang 20260113 : Rotator新增背隙補償

    //Steven 20140228 : 雷射測距功能
    int iHP1LaserX;
    int iHP2LaserX;
    int iHP1LaserY;
    int iHP2LaserY;
    int iSH1Laser;
    int iSH2Laser;

    int iInArmY15Pitch;                                                         //Steven 20131002 : XY變距
    int iInArmY60Pitch;
    int iOutArmY15Pitch;
    int iOutArmY60Pitch;

    int iOutArmPlaceFix2Z1;                                                     //ChungHung 20140722 add for HT9046LA

    //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    int M_iTopView_X;
    int M_iTopView_Y;
    int M_iTopView_Pick;
    int M_iTopView_Place;
    int M_iPADView_X;
    int M_iPADView_Y;
    int M_iPADView_Z;
    int M_iBGAView_X;
    int M_iBGAView_Y;
    int M_iBGAView_Z;
    int M_iTopViewSafePos_X;
    int M_iTopViewSafePos_Y;
    int M_iTopViewKit_Zup;
    int M_iTopView_KitZ;

    int iTrayXClean;                                                            //wei 20150826 拍拍Tray X軸 Teach
    int iTrayXOCR;                                                              //wei 20151001 OCRTray X軸 Teach

    int iInArmDecay_X;                                                          //Ifor 20151209 Decay In Arm X Teach 點位
    int iOutArmDecay_X;                                                         //Ifor 20151209 Decay Out Arm X Teach 點位
    int iInArmDecay_Y;                                                          //Ifor 20151209 Decay In Arm Y Teach 點位
    int iOutArmDecay_Y;                                                         //Ifor 20151209 Decay Out Arm Y Teach 點位
    int iOutArmBinBoxZ;                                                         //kevin 20160822 add

    int iShuttle1120Pitch;                                                      //wei 20160914 Auto Shuttle Sensor
    int iShuttle1180Pitch;                                                      //wei 20160914 Auto Shuttle Sensor
    int iShuttle2120Pitch;                                                      //wei 20160914 Auto Shuttle Sensor
    int iShuttle2180Pitch;                                                      //wei 20160914 Auto Shuttle Sensor
    int iTrayMapping;                                                           //wei 20161219 Tray Mapping
    int iTrayID;                                                                //wei 20161219 Tray Mapping

    int iInRotate[2][4];                                                        //Steven 20170329 (Wei) : Add individual rotate motor
    int iOutRotate[2][4];                                                       //Steven 20170329 (Wei) : Add individual rotate motor

    int iTrayShuttle[2][6];                                                     //Steven 20170330 (Wei) : For HT-9046LM
    int iTrayRobotX[8];                                                         //Steven 20170330 (Wei) : For HT-9046LM
    int iTrayRobotY[3];                                                         //Steven 20170330 (Wei) : For HT-9046LM
    int iTrayRobotZ[4];                                                         //Steven 20170330 (Wei) : For HT-9046LM
    int iLoadHingeR[2];                                                         //Steven 20170330 (Wei) : For TSMC
    int iLoadHingeZ[2];                                                         //Steven 20170330 (Wei) : For TSMC
    int _NoUse1;                                                                //沒用到
    int _NoUse2;                                                                //沒用到
    int _NoUse3;                                                                //沒用到
    int _NoUse4;                                                                //沒用到
    //Frank 20180410 (Steven) : InArm Preciser Station
    int iInArmPreciserX;
    int iInArmPreciserY;
    int iInArmPreciserPlaceZ;
    int iPreciserOpenPitch;
    int iPreciserClosePitch;

    int iCassetteArmX[10];                                                      //wei 20180702 MR
    int iCassetteArmZ[10];                                                      //wei 20180702 MR
    int iLoadPortZ[3];                                                          //wei 20180702 MR
    int iStackedTrayX[6];                                                       //wei 20180702 MR
    int iStackedTrayZ[6];                                                       //wei 20180702 MR
    int iTrayBracketZ[2];                                                       //wei 20180702 MR

    //Sam 20190112 LM
    //==>
    int iLoadRobotZ[5];
    int iUnloadRobotZ[5];
    //<==
    //Sam 20190112 LM

    int iBottom2DIDInX;                                                         //Steven 20190308 : Bottom 2DID
    int iBottom2DIDInY;                                                         //Steven 20190308 : Bottom 2DID

    int _iTrayDeviceCnt[3];                                                     //Sam 20190405 : Tray Decive Coount

    //Sam 20181201 : Auto Alignment
    //==>
    //InArm Top CCD
    int iInTopOnCrossX;                                                         //Top CCD 影像中心到校正片 Cross 中心 X 位置(10um)
    int iInTopOnCrossY;                                                         //Top CCD 影像中心到校正片 Cross 中心 Y 位置(10um)
    int iInBaseOnBottomX;
    int iInBaseOnBottomY;
    int iInMarkX[20];                                                           //TopCCD 到 Teach Mark X 位置
    int iInMarkY[20];                                                           //TopCCD 到 Teach Mark Y 位置
    //InArm Bottom CCD
    int iInPickFoucsZ;

    //OutArm Top CCD
    int iOutTopOnCrossX;
    int iOutTopOnCrossY;
    int iOutBaseOnBottomX;
    int iOutBaseOnBottomY;
    int iOutMarkX[20];
    int iOutMarkY[20];
    //OutArm Bottom CCD
    int iOutPickFoucsZ;
    int iICBufferX;
    int iICBufferY;
    int iICBufferPlaceZ;
    //<==
    //Sam 20181201 : Auto Alignment
    int M_iBGA2View_X;
    int M_iBGA2View_Y;
    int M_iBGA2View_Z;
    //Ifor 20191211 : add Scanner AOI
    //==>
    int M_ScannerAOI_X;
    int M_ScannerAOI_Y;
    int M_ScannerAOI_Z;
    //<==
    //Ifor 20191211 : add Scanner AOI

    //KaiHuang 20200810 : Daily Correlation Function
    int iInArmDailyCorrelationX;
    int iInArmDailyCorrelationY;
    int iInArmDailyCorrelationPickZ;

    //KaiChen 20200716 : OHT
    //==>
    int iMultileEmptyZ[5];
    int iOHT_LoadPortZ[3];
    int iOHT_LoadPort2Z[3];
    int iOHT_TrayBracketZ[2];
    int iOHT_TrayBracket2Z[2];
    int iOHT_CassetteArmX[12];
    int iOHT_CassetteArmZ[12];
    int iOHT_StackedTrayX[7];
    int iOHT_StackedTrayZ[7];
    //==>
    //KaiChen 20200716 : OHT

    //Frank 20210612 : Flipper Function
    //==>
    int iInArmFlipperX[3];
    int iInArmFlipperY[3];
    int iInArmFlipperZPick[3];
    int iInArmFlipperZPlace[3];
    int iInArmFlipperR[3];
    int iOutArmFlipperX[3];
    int iOutArmFlipperY[3];
    int iOutArmFlipperZPick[3];
    int iOutArmFlipperZPlace[3];
    int iOutArmFlipperR[3];
    //<==
    //Frank 20210612 : Flipper Function

    int iIndexZForCCD[2];
    int iCCDZ;
    int iCCDXSite1x1;
    int iCCDYSite1x1;
    int iIndexZForCal[2];
    int iCalZ;
    int iCalX1x1;
    int iCalY1x1;

    int iInArmXCCDAlignmentPos;                                                 //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    int iInArmYCCDAlignmentPos;
    int iInArmXBasePickerAlignmentPos;
    int iInArmYBasePickerAlignmentPos;

    int _iInArmZBasePickerAlignmentPos[8];                                      //Steven 20230801 : 改成二維陣列

    int iOutArmXCCDAlignmentPos;
    int iOutArmYCCDAlignmentPos;
    int iOutArmXBasePickerAlignmentPos;
    int iOutArmYBasePickerAlignmentPos;

    int _iOutArmZBasePickerAlignmentPos[8];

    double dInArmCCDXResolution;                                                //ChungHung 20210113 add for Alignment CCD
    double dInArmCCDYResolution;                                                //ChungHung 20210113 add for Alignment CCD
    double dInArmCCDXRadian;                                                    //ChungHung 20210113 add for Alignment CCD
    double dInArmCCDYRadian;                                                    //ChungHung 20210113 add for Alignment CCD

    double dOutArmCCDXResolution;                                               //ChungHung 20210113 add for Alignment CCD
    double dOutArmCCDYResolution;                                               //ChungHung 20210113 add for Alignment CCD
    double dOutArmCCDXRadian;                                                   //ChungHung 20210113 add for Alignment CCD
    double dOutArmCCDYRadian;                                                   //ChungHung 20210113 add for Alignment CCD

    int _iInArmCCD_Picker_PosX[8];                                              //ChungHung 20210113 add for Alignment CCD
    int _iInArmCCD_Picker_PosY[8];                                              //ChungHung 20210113 add for Alignment CCD
    int _iInArmCCD_Picker_PosZ[8];
    int _iInArmCCD_Pitch_PosX[8];                                               //ChungHung 20210113 add for Alignment CCD
    int iInArmCCD_Pitch_PosY;                                                   //KenHsieh 20211018 : 新增X、Y Pitch數值

    int _iOutArmCCD_Picker_PosX[8];                                             ///ChungHung 20210113 add for Alignment CCD
    int _iOutArmCCD_Picker_PosY[8];                                             ///ChungHung 20210113 add for Alignment CCD
    int _iOutArmCCD_Picker_PosZ[8];
    int _iOutArmCCD_Pitch_PosX[8];                                              //ChungHung 20210113 add for Alignment CCD
    int iOutArmCCD_Pitch_PosY;                                                  //KenHsieh 20211018 : 新增X、Y Pitch數值

    int iInArmCCD_Pitch_PosX_40mm[6];                                           //KenHsieh 20211023 : 新增X Pitch 40mm teach
    int iOutArmCCD_Pitch_PosX_40mm[6];                                          //KenHsieh 20211023 : 新增X Pitch 40mm teach

    int Sht1XGabage;                                                            //kevin 20220819 add place shuttle ic error  要吸起IC放置一個地方集中
    int Sht1YGabage;                                                            //kevin 20220819 add place shuttle ic error  要吸起IC放置一個地方集中
    int Sht2XGabage;                                                            //kevin 20220819 add place shuttle ic error  要吸起IC放置一個地方集中
    int Sht2YGabage;                                                            //kevin 20220819 add place shuttle ic error  要吸起IC放置一個地方集中

    int LoadXGabage;                                                            //kevin 20220819 add LOAD X PICK ERROR 放置一個地方集中
    int LoadYGabage;                                                            //kevin 20220819 add LOAD Y PICK ERROR 放置一個地方集中
    int iGabageX;                                                               //kevin 20220819 add 放置一個地方集中
    int iGabageY;                                                               //kevin 20220819 add 放置一個地方集中

    int iInPlacementX;                                                          //JimmyChiu 20220908 add Pickup Error Placement
    int iInPlacementY;
    int iInPlacementOffsetByBasicX;
    int iInPlacementOffsetByBasicY;

    int iFix3PosL;                                                              //JimmyChiu 20220927 : Stepper Motor Control in Fix3
    int iFix3PosR;

    int iInArmX40Pitch3  ;                                                      //Frank 20230417 : Add Teaching Position
    int iInArmX120Pitch3 ;
    int iInArmX40Pitch4  ;
    int iInArmX120Pitch4 ;

    int iOutArmX40Pitch3 ;
    int iOutArmX120Pitch3;
    int iOutArmX40Pitch4 ;
    int iOutArmX120Pitch4;

    int iTrayArmZPnP[10];

    int iMagazineTray1Pos;                                                      //JerryYang 20220909 : add magazine
    int iCatchMazTray_Front;                                                    //JerryYang 20220909 : add magazine
    int iCatchMazTray_Rear;                                                     //JerryYang 20220909 : add magazine

    bool bAOAMatrix;                                                            //Steven 20230801 : 判斷是否已經改成二維陣列
    int iInArmZBasePickerAlignmentPos[2][8];                                    //Steven 20230801 : 改成二維陣列
    int iOutArmZBasePickerAlignmentPos[2][8];
    int iInArmCCD_Picker_PosX[2][8];                                            //ChungHung 20210113 add for Alignment CCD
    int iInArmCCD_Picker_PosY[2][8];                                            //ChungHung 20210113 add for Alignment CCD
    int iInArmCCD_Picker_PosZ[2][8];
    int iInArmCCD_Pitch_PosX[2][8];                                             //ChungHung 20210113 add for Alignment CCD
    int iOutArmCCD_Picker_PosX[2][8];                                           //ChungHung 20210113 add for Alignment CCD
    int iOutArmCCD_Picker_PosY[2][8];                                           //ChungHung 20210113 add for Alignment CCD
    int iOutArmCCD_Picker_PosZ[2][8];
    int iOutArmCCD_Pitch_PosX[2][8];                                            //ChungHung 20210113 add for Alignment CCD

    int iTrayAuto4Z[3];                                                         //Steven 20230907 : For HT-9011UC
    int iTrayXAuto4;
    int iTrayXAuto5;
    int iTrayXAuto6;
    int iOutArmFix4X;
    int iOutArmFix5X;
    int iOutArmFix6X;
    int iOutArmFix4Y;
    int iOutArmFix5Y;
    int iOutArmFix6Y;
    int iOutArmAuto4X;
    int iOutArmAuto5X;
    int iOutArmAuto6X;
    int iOutArmAuto4Y;
    int iOutArmAuto5Y;
    int iOutArmAuto6Y;

    int iInArmNGBinBoxX;                                                        //KenHsieh 20220923 : add Tray Map Throw IC Function
    int iInArmNGBinBoxY;
    int iInArmNGBinBoxPlaceZ;

    int iOutArmWaitX;                                                           //kevin 20230816 : add OutArm Fix Full Wait
    int iOutArmWaitY;

    int iInArmZHeightSub_16[2][4];
    int iOutArmZHeightSub_16[2][4];

    int iMagazineStandbyPos;                                                    //JerryYang 20220909 : add magazine
    int iInSmartSetup_X;                                                        //Ifor 20240423 add: Smart Setup In Arm X Teach 點位
    int iOutSmartSetup_X;                                                       //Ifor 20240423 add: Smart Setup Out Arm X Teach 點位
    int iInSmartSetup_Y;                                                        //Ifor 20240423 add: Smart Setup In Arm Y Teach 點位
    int iOutSmartSetup_Y;                                                       //Ifor 20240423 add: Smart Setup Out Arm Y Teach 點位
    int iInSmartSetup_Z;                                                        //Ifor 20240423 add: Smart Setup In Arm Z Teach 點位
    int iOutSmartSetup_Z;                                                       //Ifor 20240423 add: Smart Setup Out Arm Z Teach 點位

    int iInAutoTeachX;                                                          //Steven 20240507 : Add for auto teach
    int iInAutoTeachY;
    int iOutAutoTeachX;
    int iOutAutoTeachY;
    int iInCCDPitchPosX40mm[2][8];                                              //KenHsieh 20211023 : 新增X Pitch 40mm teach
    int iOutCCDPitchPosX40mm[2][8];                                             //KenHsieh 20211023 : 新增X Pitch 40mm teach
    int isetEditCrossBar_X;                                                     //Ztex 2024.05.10 Add In/Out Arm 16 Picker Z Teach
    int isetEditCrossBar_Y;                                                     //Ztex 2024.05.10 Add In/Out Arm 16 Picker Z Teach
    int isetEditOutCrossBar_X;                                                  //Ztex 2024.05.10 Add In/Out Arm 16 Picker Z Teach
    int isetEditOutCrossBar_Y;                                                  //Ztex 2024.05.10 Add In/Out Arm 16 Picker Z Teach

    int iMLoaderYCarPos;
    int iMLoaderYOCRPos;                                                        //Frank 20250214 add
    int iMLoaderYSurePos;

    int iInSH1SenICDetectPos;                                                   //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    int iInSH2SenICDetectPos;                                                   //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    int iInSH1SenICDetectZ1;                                                    //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    int iInSH1SenICDetectZ2;
    int iInSH2SenICDetectZ1;
    int iInSH2SenICDetectZ2;

    int iLDCassetteFront;                                                       //Frank 20251217 add
    int iLDCassetteFrontBack;
    int iLDCassetteRear;
    int iLDCassetteRearBack;
    int iLDCassetteZStart;

    int iAutoCassetteFront[2];
    int iAutoCassetteFrontBack[2];
    int iAutoCassetteRear[2];
    int iAutoCassetteRearBack[2];
    int iAutoCassetteZStart[2];

    int iSortShuttleLeft;                                                       //RogerYang 20250416 for HT9046AU add
    int iSortShuttleRight;
    int iSortArmShtStageX;
    int iSortArmShtStageY;
    int iSortArmAuto4X;
    int iSortArmAuto4Y;
    int iSortArmAuto5X;
    int iSortArmAuto5Y;
    int iSortArmAuto6X;
    int iSortArmAuto6Y;
    int iSortArmXPitchMin;
    int iSortArmXPitchMax;
    int iSortArmAutoPlaceZ;
    int iSortArmShuttlePickZ;
    int iSortArmSafeZ1;
    int iSortArmZHeightSub[2];
    int iOutArmSortShtX;
    int iOutArmSortShtY;
    int iOutArmToSortShtPlaceZ;
}TECH;
extern TECH Tech;
//---------------------------------------------------------------------------
//LastSet檔案裡的結構請不要任意加東西
//就算要加也是加在最下面
//有加東西請跟大家說一下~~
//------------------------------------------------------------------------------
enum eCommandList                                                               //JerryYang 20181126 (Steven) : support Epson DLL function
{   eSetSiteMap          =0,
    eGetSiteMap          =1,
    eSetBin              =2,
    eGetBin              =3,
    eSetTemp             =4,
    eGetTempSetting      =5,
    eGetTempActual       =6,
    eGetBinCount         =7,
    eClearBinCount       =8,
    eGetSortCount        =9,
    eClearSortCount      =10,
    eGetHandlerStatus    =11,
    eGetAlarmStatus      =12,
    eRemoteControl       =13,
    eDownloadRecipe      =14,
    eGetbincountPersite  =15,
    eSetProdMode         =16,                                                   //JerryYang 20220311 : ATP鎖定Critical parameter
    eGetProdMode         =17,
    eCommandTotal
};
//------------------------------------------------------------------------------
typedef struct                                                                  //JerryYang 20181126 (Steven) : support Epson DLL function, 這邊結構不能亂動
{
    bool        bCommandList[eCommandTotal];
    int         iCmdResult[eCommandTotal];
    char        cSetSiteMap_Cmd0[256];
    int         iSetSiteNum_Cmd0;
    char        cGetSiteMap_Cmd1[256];
    int         iSetBinTrayNum_Cmd2;
    char        cSetBinCategories_Cmd2[256];
    int         iSetBinFail_Cmd2;
    int         iGetBinTrayNum_Cmd3;
    char        cGetBinCategories_Cmd3[256];
    int         iSetTempMode_Cmd4;
    double      dSetTempVal_Cmd4;
    char        cGetTempSettings_Cmd5[256];
    char        cGetTempActual_Cmd6[320];
    int         iGetBinCategNum_Cmd7;
    int         iGetSortTrayNum_Cmd9;
    double      dGetAlarmStatus_Cmd12;
    int         iRemoteControl_Cmd13;
    char        cDownloadRecipeName_Cmd14[256];
    int         iGetBinCountCategNum_Cmd15;
    int         iGetBinCountSiteNum_Cmd15;
    int         iSetProdMode_Cmd16;                                             //JerryYang 20220311 : ATP鎖定Critical parameter
    char        cVersion[100];
}INFO;
extern INFO *CmdData;
//---------------------------------------------------------------------------
struct VPoint                                                                   //(10um)
{
    int iOriX;                                                                  //OriginalX。
    int iOriY;                                                                  //OriginalY。
    int iAccX;                                                                  //Vision AccuracyX。
    int iAccY;                                                                  //Vision AccuracyY。
    int iOffSetX;                                                               //Vision AccuracyX - OriginalX
    int iOffSetY;                                                               //Vision AccuracyY - OriginalY
};
//---------------------------------------------------------------------------
struct TeachInfo                                                                //Sam 20181201 : Auto Alignment
{
    //Top
    double dtPixelSizeX;                                                        //相機解析度 X 方向。(10um/Pixle)
    double dtPixelSizeY;                                                        //相機解析度 Y 方向。(10um/Pixle)
    VPoint vtDummy;                                                             //CCD 到 Dummy IC 位置。          計算 CCD2BasePicker 使用
    int itDummyZ;                                                               //BasePicker 到 Dummy IC Z軸吸取高度(10um)
    VPoint vtTop2BasePicker;                                                    //TopCCD 到 BasePicker 移動距離。
    VPoint vtMark[20];                                                          //TopCCD 到 Teach Mark 位置

    //Bottom
    VPoint vbBasePickerOnBottom;                                                //BasePicker On BottomCCD 位置
    double dbPixelSizeX;                                                        //相機解析度 X 方向。(10um/Pixle)
    double dbPixelSizeY;                                                        //相機解析度 Y 方向。(10um/Pixle)
    int iPickerZ[8];                                                            //Bottom CCD 的跟 Picker 8支吸嘴的焦距
};
//---------------------------------------------------------------------------
typedef struct                                                                  //Sam 20181201 : Auto Alignment
{
    TeachInfo IO[2];                                                            //InArm / OutArm  兩組資料
    int iAccuracy;
}ALIGNTEACH;
extern ALIGNTEACH AlignTeach;
//---------------------------------------------------------------------------
//LastSet檔案裡的結構請不要任意加東西
//就算要加也是加在最下面
//有加東西請跟大家說一下~~
//---------------------------------------------------------------------------
#endif
