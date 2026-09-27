#ifndef cprodH
#define cprodH

//#include "MachineDefine.h"
#include "MachineType.h"                                                        //Steven 20130809 : OK
#include "Config.h"
#include "CosFunction.h"
#include <map>
#include "myTimer.h"   //AI(W0-TAIL) 20260626: TQPF_Timer tP62MachineStopTimer is held by value (was via MachineDefine.h)

//   IniConfig.iUseGPIBFormat=0;//kevin 20130705 台積電通訊規格  0:HT    1:NS
//-------------------------------------------------------------------------
//typedef struct {
//    char FileName[12][128];
//}HANDLER_SETUP;
//extern HANDLER_SETUP THS;
//-------------------------------------------------------------------------
typedef struct {
    char cModeName[128];
    int iSTLogicMode;
    int iStartType;
    int iOneSTChannel;
    unsigned int iSTPluseWidth;
    int iDutType;
    int iDutBfOnTime;
    int iDutAfOffTime;
    int iCateLogicMode;
    int iCateBitLength;
    int iCateParity;
    int iCateDataType;
}TTL_DATA;
extern TTL_DATA TTLCfg;
//-------------------------------------------------------------------------
typedef struct {
    int iXItem;
    int iYItem;
    int iXPitch;
    int iYPitch;
    int iXEdge;
    int iYEdge;
    int iRotate;
    int iMethod;
} TRAY_DATA ;                                                                   //Steven 20110808 沒用到但是不能砍
//extern TRAY_DATA TrayDef;
//-------------------------------------------------------------------------
typedef struct {
    int Year;
    int Month;
    int Day;
    int Hour;
    int Minute;
    int Second;
    int Reserved[5];                                                            //  保留
}DATE_INFO;                                                                     //Steven 20110808 沒用到但是不能砍
//-------------------------------------------------------------------------
typedef struct {
    int iRight;
    int iLeft;
    int iRightForIndexZ2;
    int Reserved[4];
}SM_SHUTTLE;
//------------------------------------------------------------------------
typedef struct {
    int Baud_Rate;
    int Bit_Length;
    int Stop_Bit;
    int Parity;
}RS232_DATA;
//========================================> Auto Teach Z OFFSET   kevin 20210321
typedef struct
{
    //這是使用網路硬碟做工作檔下載---------------------
    double dXPos[10];                                                           //X: load 0 shuttlt1: 1  shuttlt2: 2  HP1:3 HP:4
    double dYPos[10];                                                           //Y: load 0 shuttlt1: 1  shuttlt2: 2  HP1:3 HP:4
    double dPick[10][MAX_ARM_Row][MAX_ARM_Col];                                 //Z :load 0 shuttlt1: 1  shuttlt2: 2  HP1:3 HP:4    Za:0 ZB=1...
}TAutoTeachOffset;
extern TAutoTeachOffset AutoTeachOffset;
//===============================================================================
typedef struct
{
    bool bOnLine;
    int iEnableScannerMode;
    double fScannerReadTimeout;
    int iIntervalCounter;
    int iRetryCounter;
    int iBallDamageType;
    bool bBDTotalFunction;
    bool bBDTotalContiFunction;
    bool bBDSiteFunction;
    bool bBDSiteContiFunction;
    int iBDTotalCounter;
    int iBDTotalContiCounter;
    int iBDSiteCounter;
    int iBDSiteContiCounter;

    int iAOIFailBinType;                                                        //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項;
    int ScannerIfError;                                                         // 2012.12.16 , Joye , AMD HT-7046M
    int ScannerIfErrorAndTestPass;                                              //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項

    bool bDBAlramAutoResetCount;                                                // 2012.12.18 , Joye , AMD HT-7046M

    int iAOITotalCount;
    int iBDTotalCounterLast;
    int iBDTotalContiCounterLast;
    int iBDSite1CounterLast;
    int iBDSite2CounterLast;
    int iBDSite1ContiCounterLast;
    int iBDSite2ContiCounterLast;
    int iStartDelayTime;
    int iTimeOut;

    int i2DInterval_Socket1;
    int i2DInterval_Socket2;

    AnsiString sSCANNER_ComPort;
    AnsiString sSCANNER_BaudRate;
    AnsiString sSCANNER_ByteSize;
    AnsiString sSCANNER_StopBit;
    AnsiString sSCANNER_Parity;
    double fScannerICGain;                                                      //Ifor 20200525 add:依據Tray Y Pitch Gain 決定不使用範圍
    bool bScanAOIUseLGAMode;                                                    //Ifor 20211022 add: AOI LGA Mode
    double fLGAWaitTime;                                                        //Ifor 20211022 add: AOI LGA Mode
    bool bLGAModeDirection;                                                     //Ifor 20230202 add: AOI LGA Mode Direction
    //Ifor 20200902 add: TFAMD Top AOI
    //==>
    bool bTopOnLine;
    int iEnableTopScannerMode;
    double fTopScannerReadTimeout;
    int iTopIntervalCounter;
    int iTopRetryCounter;
    int iTopStartDelayTime;
    int iTopTimeOut;
    int TopScannerIfError;
    AnsiString sTopSCANNER_ComPort;
    AnsiString sTopSCANNER_BaudRate;
    AnsiString sTopSCANNER_ByteSize;
    AnsiString sTopSCANNER_StopBit;
    AnsiString sTopSCANNER_Parity;
    //<==
    //Ifor 20200902 add: TFAMD Top AOI
    bool bEnabledPositionByAOI;                                                 //Ifor 20251031 add: Move Position Provided By AOI  //AI(W906-FRW-S69) 20260926: 照 golden V912 cprod.h:140 補（Steven 13:5x「補」）；只新增、不改既有欄位，這個結構沒有二進位存檔
}SYSTEM_SCANNER_AOI_IF;
extern SYSTEM_SCANNER_AOI_IF ScannerAOIIF;
//-------------------------------------------------------------------------
//typedef struct {
//    int iQuantity[4];
//    int iFail[4];
//    int iRate[4];
//}CCD_DATA;
//extern CCD_DATA CCDBarCode;
//==============================================================================
class ARM_SINGLE_PARAM
{
    public:
    //Eliot 2009_12_23 start
        double dPosOffSetX[4][8];
        double dPosOffSetY[4][8];
        double dPickUpOffSet[4][8];
        double dPlaceOffSet[4][8];
    //Eliot 2009_12_23 end
};

class ARM_OFFSET
{
    private:
    public:
        bool   bOneByOne;
        double dArmX;
        double dArmY;
        double dArmVariable;
        double dXPitch[4];                                                      //Steven for HT1032
        double dPickUp;
        double dPlaceUp;
        double dArmVariableY;                                                   //ChungHung 20131231 alter AutoYPitch
        double dArmVariable2;                                                   //Steven 20131002 : XY變距
        double dArmVariable3;                                                   //Steven 20131002 : XY變距
        double dArmVariable4;                                                   //Steven 20131002 : XY變距

        ARM_OFFSET();   //AI(W0-TAIL) 20260626: was '_fastcall ARM_OFFSET::ARM_OFFSET();' (BCB single-underscore typo + illegal in-class qualifier)
        ~ARM_OFFSET();  //AI(W0-TAIL) 20260626: was '_fastcall ARM_OFFSET::~ARM_OFFSET();' (BCB single-underscore typo + illegal in-class qualifier)
        ARM_SINGLE_PARAM *SingleOffSet;
        TStringList *tArmOffset;                                                //Steven 20140510 : Secs Gem

        TStringList *tArmPickOffset;                                            //Steven 20140510 : Secs Gem
        TStringList *tArmPlaceOffset;                                           //Steven 20140510 : Secs Gem

        void SetOneByOne(bool flag);
        void SetX(double Pos);
        void SetY(double Pos);
        void SetVariable(double Pos);
        void SetVariableY(double Pos);                                          //ChungHung 20131231 alter AutoYPitch
        void SetVariable2(double Pos);                                          //Steven 20131002 : XY變距
        void SetVariable3(double Pos);
        void SetVariable4(double Pos);
        void SetPickUp(double Pos);
        void SetPlace(double Pos);

        bool GetOneByOne(){return bOneByOne;}
        double GetX(){return dArmX;}
        double GetY(){return dArmY;}
        double GetPickUp(){return dPickUp;}
        double GetPlace(){return dPlaceUp;}
        double GetXPitch(int iX){return dXPitch[iX];}                           //Steven for HT1032
        double GetVariable(){return dArmVariable;}
        double GetVariableY(){return dArmVariableY;}                            //ChungHung 20131231 alter AutoYPitch
        double GetVariable2(){return dArmVariable2;}                            //Steven 20131002 : XY變距
        double GetVariable3(){return dArmVariable3;}                            //Steven 20131002 : XY變距
        double GetVariable4(){return dArmVariable4;}                            //Steven 20131002 : XY變距
        double GetArmX(int iX, int iY)
        {
            if(bOneByOne)
                return SingleOffSet->dPosOffSetX[iX][iY];
            else
                return -dArmX;
        }
        double GetArmY(int iX, int iY)                                          //iZ==>which Z only
        {
            if(bOneByOne)
                return SingleOffSet->dPosOffSetY[iX][iY];
            else
                return -dArmY;
        }
        double GetPickUp(int iX, int iY)                                        //iZ==>which Z only
        {
            return SingleOffSet->dPickUpOffSet[iX][iY];
        }
        double GetPlace(int iX, int iY)                                         //iZ==>which Z only
        {
            return SingleOffSet->dPlaceOffSet[iX][iY];
        }
        void SetPickUp(int iX, int iY, double dData);                           //iZ==>which Z only
        void SetPlace(int iX, int iY, double dData);                            //iZ==>which Z only
};
extern  class ARM_OFFSET *InArmOffSet[InOfsTotal];                              //Steven 20140425 : 重整Offset
extern  class ARM_OFFSET *OutArmOffSet[OutOfsTotal];                            //Steven 20140425 : 重整Offset
extern  class ARM_OFFSET *InArmOffSet_File[InOfsTotal];                         //Steven 20140425 : 重整Offset
extern  class ARM_OFFSET *OutArmOffSet_File[OutOfsTotal];                       //Steven 20140425 : 重整Offset
extern  class ARM_OFFSET *SortArmOffSet[SortOfsTotal];                          //RogerYang 20250417 for HT9046AU add
extern  class ARM_OFFSET *SortArmOffSet_File[SortOfsTotal];                     //RogerYang 20250417 for HT9046AU add

typedef struct
{
    double dTrayZseparate[MAX_TRACK];
    double iIndexArmPickUp[2];
    double iIndexArmPlace[2];
    double iIndexArmContact[2];
    double iSHRightPod[2];
    double iSHHalft[2];
    double iTrayArmX[MAX_TRACK];
    double iSHLeftPod[2];
    double iSHLeft2D[2];                                                        //Steven 20151218 : Offset for 2d reader
    double iTrayArmX_ART[MAX_TRACK];                                            //kevin 20170831 (Steven) ART Auto-> Load offset
    double iPreciserOpen;                                                       //Frank 20180410 (Steven) : InArm Preciser Station
    double iPreciserClose;
    double dInArmPickUp[InOfsTotal];                                            //KenHsieh 20220914 : 新增SECS用
    double dOutArmPickUp[OutOfsTotal];                                          //KenHsieh 20220914 : 新增SECS用
    double dSortArmPickUp[SortOfsTotal];                                        //RogerYang 20250417 for HT9046AU add
}RUN_OFFSET;
extern RUN_OFFSET Offset;
extern RUN_OFFSET Offset_File;

typedef struct
{
    int LoaderXOffset;
    int LoaderYOffset;
    int LoaderZOffset;
    int LoaderXPOffset;
    int LoaderXP2Offset;
    int LoaderYPOffset;

    int HotPlate1XOffset;
    int HotPlate1YOffset;
    int HotPlate1ZOffset;
    int HotPlate1XPOffset;
    int HotPlate1XP2Offset;
    int HotPlate1YPOffset;

    int HotPlate2XOffset;
    int HotPlate2YOffset;
    int HotPlate2ZOffset;
    int HotPlate2XPOffset;
    int HotPlate2XP2Offset;
    int HotPlate2YPOffset;

    int InSht1XOffset;
    int InSht1YOffset;
    int InSht1ZOffset;
    int InSht1XPOffset;
    int InSht1XP2Offset;
    int InSht1YPOffset;

    int InSht2XOffset;
    int InSht2YOffset;
    int InSht2ZOffset;
    int InSht2XPOffset;
    int InSht2XP2Offset;
    int InSht2YPOffset;

    int OutSht1XOffset;
    int OutSht1YOffset;
    int OutSht1ZOffset;
    int OutSht1XPOffset;
    int OutSht1XP2Offset;
    int OutSht1YPOffset;

    int OutSht2XOffset;
    int OutSht2YOffset;
    int OutSht2ZOffset;
    int OutSht2XPOffset;
    int OutSht2XP2Offset;
    int OutSht2YPOffset;

    int Auto1XOffset;
    int Auto1YOffset;
    int Auto1ZOffset;
    int Auto1XPOffset;
    int Auto1XP2Offset;
    int Auto1YPOffset;

    int Auto2XOffset;
    int Auto2YOffset;
    int Auto2ZOffset;
    int Auto2XPOffset;
    int Auto2XP2Offset;
    int Auto2YPOffset;

    int Auto3XOffset;
    int Auto3YOffset;
    int Auto3ZOffset;
    int Auto3XPOffset;
    int Auto3XP2Offset;
    int Auto3YPOffset;

    int Fix1XOffset;
    int Fix1YOffset;
    int Fix1ZOffset;
    int Fix1XPOffset;
    int Fix1XP2Offset;
    int Fix1YPOffset;

    int Fix2XOffset;
    int Fix2YOffset;
    int Fix2ZOffset;
    int Fix2XPOffset;
    int Fix2XP2Offset;
    int Fix2YPOffset;

    int Fix3XOffset;
    int Fix3YOffset;
    int Fix3ZOffset;
    int Fix3XPOffset;
    int Fix3XP2Offset;
    int Fix3YPOffset;

    int InarmPickOffSet[2][4];
    int InarmReleaseOffset[2][4];

    int OutarmPickOffSet[2][4];
    int OutarmReleaseOffset[2][4];

    //pig 20240215 AddTempShiftOffset
    int iTempShiftOffset[3][30];
    double fTempShiftOffsetRatio[2][30];
    double fTempShiftOffset[3][30];
    double fTempShiftScaleRatio[2][30];
    //pig 20240215 AddTempShiftOffset
}INVISIBLE_OFFSET;
extern INVISIBLE_OFFSET InvisibleOffset;

typedef struct {
    char cFileName[256];
    DATE_INFO LastEditDate;                                                     //Steven 20110808 沒用到

    TRAY_DATA LoadForm;
    TRAY_DATA AutoForm[eTrayCount];
//    TRAY_DATA FixForm[MAX_FIX_TRAY];
    TRAY_DATA HotPlateForm[2];

    bool fInArmSuckUse[2][MAX_ARM_Row][MAX_ARM_Col];                            //9016       //ChungHung 20130910 alter for SCK can close site by Index
    bool fOutArmSuckUse[MAX_ARM_Row][MAX_ARM_Col];                              //9016
    int iHotTime;
    int XInArm_Plate1_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int YInArm_Plate1_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int XInArm_Plate2_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int YInArm_Plate2_Pick[MAX_ARM_Row][MAX_ARM_Col];

    int XInArm_AutoClean_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int YInArm_AutoClean_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_AutoClean_Place[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_AutoClean_Pick[MAX_ARM_Row][MAX_ARM_Col];

    int XInArm_Shuttle1_Place[MAX_ARM_Row][MAX_ARM_Col];
    int YInArm_Shuttle1_Place[MAX_ARM_Row][MAX_ARM_Col];
    int XInArm_Shuttle2_Place[MAX_ARM_Row][MAX_ARM_Col];
    int YInArm_Shuttle2_Place[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_Tray_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_Tray_Place[MAX_ARM_Row][MAX_ARM_Col];                            //Steven 20221114 :Add for device map function
    int ZInArm_Plate1_Place[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_Plate2_Place[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_Plate1_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_Plate2_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_Shuttle1_Place[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArm_Shuttle2_Place[MAX_ARM_Row][MAX_ARM_Col];
    int ZInArmSafe[MAX_ARM_Row][MAX_ARM_Col];
    int iInArmSafeX;                                                            //Steven 20091008
    int iInArmSafeY;                                                            //Steven 20091008
    SM_SHUTTLE InSHT[2];                                                        //0=shutlle1 1=shuttle2
    SM_SHUTTLE OutSHT[2];
    SM_SHUTTLE SortSHT;                                                         //RogerYang 20250417 for HT9046AU add
    int ZOutArm_SortShuttle_Place[MAX_ARM_Row][MAX_ARM_Col];
    int XOutArm_SortShuttle_Place[MAX_ARM_Row][MAX_ARM_Col];
    int YOutArm_SortShuttle_Place[MAX_ARM_Row][MAX_ARM_Col];
    int ZSortArm_SortShuttle_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int XSortArm_SortShuttle_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int YSortArm_SortShuttle_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int ZSortArm_Auto_Place[MAX_AUTO_TRAY][MAX_ARM_Row][MAX_ARM_Col];
    int XSortArm_Auto_Place[MAX_AUTO_TRAY][MAX_ARM_Row][MAX_ARM_Col];
    int YSortArm_Auto_Place[MAX_AUTO_TRAY][MAX_ARM_Row][MAX_ARM_Col];
    int ZSortArmSafe[MAX_ARM_Row][MAX_ARM_Col];

    int iSortArmSafeX;
    int iSortArmSafeY;
    int iSortArmXMinPitch;
    int iSortArmXMaxPitch;

    int TestY1_Front;
    int TestY1_Middle;
    int TestY2_Rear;
    int TestY2_Middle;
    int TestZ1_Pick;
    int TestZ1_Place;
    int TestZ1_Test;
    int TestZ1_Safe;
    int TestZ1_Drop_Offset;
    int TestZ2_Pick;
    int TestZ2_Place;
    int TestZ2_Test;
    int TestZ2_Safe;
    int TestZ2_Drop_Offset;
    int iAutoCleanZ_Contact[2];                                                 //Steven 20181108 : 修正Auto Clean位置異常
    int iAutoCleanZ_Drop[2];
    int iAutoCleanZ_Shift[2];
    double TestZ_Drop_Wait;                                                     //kevin 20110829  修改 可以寫入小數
    int XOutArm_Shuttle1_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int YOutArm_Shuttle1_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int XOutArm_Shuttle2_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int YOutArm_Shuttle2_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int ZOutArm_Shuttle1_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int ZOutArm_Shuttle2_Pick[MAX_ARM_Row][MAX_ARM_Col];

    int XStart[eTrayCount][MAX_ARM_Row][MAX_ARM_Col];                           //kevin 20160826 add error bin box pos       //Steven 20141029 : 將XStart & YStart改為Prod
    int YStart[eTrayCount][MAX_ARM_Row][MAX_ARM_Col];                           //kevin 20160826 add error bin box pos
    int ZPlace[eTrayCount][MAX_ARM_Row][MAX_ARM_Col];
    int ZPick [eTrayCount][MAX_ARM_Row][MAX_ARM_Col];

    int XOutArm_Auto_Place[MAX_AUTO_TRAY][MAX_ARM_Row][MAX_ARM_Col];
    int YOutArm_Auto_Place[MAX_AUTO_TRAY][MAX_ARM_Row][MAX_ARM_Col];
    int ZOutArm_Auto_Place[MAX_AUTO_TRAY][MAX_ARM_Row][MAX_ARM_Col];

    int ZOutArmSafe[MAX_ARM_Row][MAX_ARM_Col];

    int XOutArm_Fix_Place[MAX_FIX_TRAY][MAX_ARM_Row][MAX_ARM_Col];
    int YOutArm_Fix_Place[MAX_FIX_TRAY][MAX_ARM_Row][MAX_ARM_Col];
    int ZOutArm_Fix_Place[MAX_FIX_TRAY][MAX_ARM_Row][MAX_ARM_Col];
    int ZOutArm_Fix_Pick[MAX_FIX_TRAY][MAX_ARM_Row][MAX_ARM_Col];               //Ifor 20161222 add Fix Tray Pick Offset

    int iOutArmSafeX;                                                           //Steven 20091008
    int iOutArmSafeY;                                                           //Steven 20091008
    int iOutArmSafeY_FixFull;                                                   //Steven 20260428 : iOutArmSafeY + (-15000 if bE90 ON else 0); for Fix Tray Full retreat
    int iXTrayLoad;
    int iXTrayEmpty;
    int iXTraySafty;
    int iXTrayColor;
    int iXTrayAuto[MAX_AUTO_TRAY];
    bool bTesterFailAlarmByAllSite;
    int  iTesterFailCountByAllSite;
    double  iTesterMaxTime;
    double  iTesterDummyTime;
    double  dTesterStartDelayTime;
    int  iTesterInterface;
    double iWaitGetEpsonTime;                                                   //Steven 20100811 沒用到
    int XInArm_Tray_Pick[MAX_ARM_Row][MAX_ARM_Col];
    int YInArm_Tray_Pick[MAX_ARM_Row][MAX_ARM_Col];
    bool bHotPlateUse[2];
    int All_TestZ_Test_Safe;
    int  iTemperature;

    int TrayZ_Up[MAX_TRACK];                                                    //Steven 20190813 : 入Tray改用步進馬達
    int TrayZ_Mid[MAX_TRACK];                                                   //Steven 20190813 : 入Tray改用步進馬達

    int iBS1Right_Half;
    int iBS2Right_Half;

    int XOutArm_InSpect;
    int YOutArm_InSpect;
    int ZOutArm_InSpect[4];
    //int RotateType[2];   //Steven 20101019 : 沒用到，馬克掉

    //int  iOutArmSafeY;
    bool bPlateSelect[2];
    double  OutArmInspectWaitTime;

    int  ZOutArm_Auto_Pick[MAX_ARM_Row][MAX_ARM_Col];

    int  iINARM_X_PITCH;
    int  iINARM_Y_PITCH;

    bool bTesterFailAlarmBySite;
    bool bTesterFailAlarmByBin;
    int  iTesterFailCountBySite;
    int  iTesterFailCountByBin;

    int  iT6CatData [TEST_MAX_BIN];                                             //Auto1 = 0
    int  iT6PosCate [TEST_MAX_BIN];                                             //Auto1 = 1
    int  DBContact  [TEST_MAX_BIN];
    int  iIsFailBin [TEST_MAX_BIN];                                             //Steven 20240118 : bIsFail --> bIsFailBin
    int  bIsPassBin [TEST_MAX_BIN];                                             //Steven 20240118 : bIsPass --> bIsPassBin
    bool bScanner   [TEST_MAX_BIN];
    bool bConsFail  [TEST_MAX_BIN];
    bool bFailure   [TEST_MAX_BIN];
    int iIfErrorT6;

    int TestZ1_Test2;                                                           // for drop & test
    int TestZ2_Test2;                                                           // for drop & test

    int ZOutArm_Shuttle1_Place[MAX_ARM_Row][MAX_ARM_Col];
    int XInArm_LeftPlate1_Pick;
    int YInArm_LeftPlate1_Pick;
    int XInArm_LeftPlate2_Pick;
    int YInArm_LeftPlate2_Pick;
    int XInArmMachineRotatePos[4];
    int YInArmMachineRotatePos[4];
    int ZInArmRotatePick[4];
    int ZInArmRotatePlace[4];
    int XOutArmMachineRotatePos[4];
    int YOutArmMachineRotatePos[4];
    int ZOutArmRotatePick[4];
    int ZOutArmRotatePlace[4];
    int XInArm_Shuttle1_2_Place;
    int XInArm_Shuttle2_2_Place;
    int XOutArm_Shuttle1_2_Pick;
    int XOutArm_Shuttle2_2_Pick;

    int iInArmX40Pitch;
    int iInArmX120Pitch;
    int iOutArmX40Pitch;
    int iOutArmX120Pitch;

//    int iInArmX40Pitch1;                //Steven for HT1032
    int iInArmX40Pitch2;
    int iInArmX40Pitch3;
    int iInArmX40Pitch4;
//    int iInArmX120Pitch1;
    int iInArmX120Pitch2;
    int iInArmX120Pitch3;
    int iInArmX120Pitch4;
//    int iOutArmX40Pitch1;
    int iOutArmX40Pitch2;
    int iOutArmX40Pitch3;
    int iOutArmX40Pitch4;
//    int iOutArmX120Pitch1;
    int iOutArmX120Pitch2;
    int iOutArmX120Pitch3;
    int iOutArmX120Pitch4;

    int iInArmY15Pitch;
    int iInArmY60Pitch;
    int iOutArmY15Pitch;
    int iOutArmY60Pitch;

    int YOutArm_Shuttle1_2_Pick;
    int YOutArm_Shuttle2_2_Pick;

    int iOutArmX40Pitch_RotateOn;
    int iOutArmX120Pitch_RotateOn;
    int iOutArmX40Pitch2_RotateOn;
    int iOutArmX120Pitch2_RotateOn;

    int XInArm_TubeLoadKit_Pick;
    int YInArm_TubeLoadKit_Pick;

    int XOutArm_TubeUnLoadKit_Place;
    int YOutArm_TubeUnLoadKit_Place;

    int ZInArm_TubeLoadKit_Pick[2];
    int ZOutArm_TubeUnLoadKit_Place[2];
    int ZInArm_ShuttlePlace;

    int iTubeSelectTubeBufferGet;
    int iTubeSelectTrackPut[2];
    int iMaxPreasure;

    int iInArmX40Pitch_RotateOn;                                                //Steven 20131002 : XY變距
    int iInArmX120Pitch_RotateOn;
    int iInArmX40Pitch2_RotateOn;
    int iInArmX120Pitch2_RotateOn;
    int iXTrayEmpty1;
    TTL_DATA DIOCfg;
    double dFailureLimit[TEST_MAX_BIN];                                         //kevin 20140307  [15];    //jou 2012-11-28 Bin Yield Failure 的百分比值要可以輸入到小數點一位,另外也要紅底+密碼
    //Steven 20140529 Start: Fail Persent & Count同時存在
    int     iPersentIgnore  [TEST_MAX_BIN];
    bool    bFailCountEnable[TEST_MAX_BIN];
    int     iFailCountIgnore[TEST_MAX_BIN];
    int     iFailCountLimit [TEST_MAX_BIN];
    //Steven 20140529 End: Fail Persent & Count同時存在

    bool bSpecialBinByArm[TEST_MAX_BIN];                                        //ChungHung 20140730 add ContinuousFailHaveOneCycle
    unsigned int  iSpecialBinCountByArm[TEST_MAX_BIN];
    bool bSpecialBinBySocket[TEST_MAX_BIN];
    unsigned int  iSpecialBinCountBySocket[TEST_MAX_BIN];

    bool bLowYield[TEST_MAX_BIN];                                               //Steven 20140828 : By Bin Yield Monitor
    bool bArmYield[TEST_MAX_BIN];                                               //Steven 20140828 : By Bin Arm Yield Monitor
    bool bSiteYield[TEST_MAX_BIN];                                              //Steven 20140828 : By Bin Site Yield Monitor

    int iAutoCleanByBin[TEST_MAX_BIN];                                          //Steven 20160308 : By Bin count do auto clean
    int iAutoCleanBySite[TEST_MAX_BIN];                                         //Steven 20160308 : By Bin pre site count do auto clean

    bool bContsFailBySocket;                                                    //jou 980716 start : add FT/RT alarm
    unsigned int iContsFailSocketAlarmCT;
    bool bCountSpcBinContinuously;                                              //Steven 20230529 : Spc Bin Couont改成連續錯誤

    bool bContsFailByHead;
    unsigned int iContsFailHeadAlarmCT;

    bool bContsFailIgnore;                                                      //wei 20160115 銅鑼前幾顆不計算ContsFail
    int  iContsFailIgnore;                                                      //wei 20160115 銅鑼前幾顆不計算ContsFail

    bool bAllSiteFail;                                                          //Isaac 20180305 (Steven) ATK要求，只有FT要alarm，FT/RT分開Prod.bAllSiteFail
    int iAllSiteFailCount;                                                      //Steven 20230118 : All site fail RT

    bool bFailAlarmSiteYieldDifferent;
    double  dFailAlarmSiteYield;                                                //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int  iFailAlarmSiteYieldDifferentCount;

    bool bFailAlarmSiteYieldCmp;                                                //jou 2014-08-14 Site Compare Low Yield alarm
    double dFailAlarmSiteYieldCmp;                                              //jou 2014-08-14 Site Compare Low Yield alarm   //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int  iFailAlarmSiteYieldCmpCount;                                           //jou 2014-08-14 Site Compare Low Yield alarm

    bool bErrOverLmt;
    double dErrOverLmt;
    int  iErrOverLmtIg;

    bool bFailAlarmLowYield;
    double dLowYieldLimit;                                                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
    bool bSlidingWindowYield;                                                   //Hana: Sliding Window Yield  //Steven 20260331
    int  iSlidingWindowSize;                                                    //Sliding window size N        //Steven 20260331
    int iLowYieldCount;

    bool bFailAlarmLowYield_AutoClean;                                          //wei 20141201 Low Yield Auto Clean(%)
    int iLowYieldLimit_AutoClean;
    int iLowYieldCount_AutoClean;

    bool bFailAlarmLowYieldByTotal;                                             //wei 20151116 Low Yield By Total
    double dLowYieldLimitByTotal;                                               //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iLowYieldCountByTotal;

    bool bLowYieldByPicker;                                                     //Steven 20230223 : 根據Index吸嘴比較良率
    int iLowYieldCountByPicker;
    double dLowYieldByPicker;

    bool bFailAlarmLowYieldSpecial;                                             //Sam 20210505 : PTI 要求的兩段 Low Yeild
    double dLowYieldLimitSpecial;
    int iLowYieldCountSpecial1;
    int iLowYieldCountSpecial2;

    bool bLowYieldAlarmByBin;                                                   //Steven 20140828 : By Bin Yield Monitor

    bool bFailRateMode;
    unsigned int iIgnoreIC;
    int iCountAlarmAction;                                                      //Steven 20101116

    //JerryYang 20170712 (Steven) by site by bin compare percent
    //==>
    bool    bSpecBinBySiteCompareEnable[TEST_MAX_BIN];
    int     iSpecBinBySiteCompareIgnore[TEST_MAX_BIN];
    double  dSpecBinBySiteComparePercent[TEST_MAX_BIN];
    bool    bSpecBinByArmPerSiteCompareEnable[TEST_MAX_BIN];
    int     iSpecBinByArmPerSiteCompareIgnore[TEST_MAX_BIN];
    double  dSpecBinByArmPerSiteComparePercent[TEST_MAX_BIN];
    //<==
    //JerryYang 20170712 (Steven) by site by bin compare percent

    bool bSCKART_EnableSPBinAlarm;                                              //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm
    int iSCKART_SPBinSelect;                                                    //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm
    double dSCKART_SPBinAlarmYield;                                             //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm

    //jou 981203 start : in shuttle sensor 7
    int iInSHSen7DetectPos1x1[2];
    int iInSHSen7DetectPos1x2[2][2];
    int iInSHSen7DetectPos2x3[2][3];                                            //ChungHung 20140115 add for 2x3_6
    int iInSHSen7DetectPos1x4[2][4];
    int iInSHSen9DetectPos2x5[2][5];                                            //wei 20190614 10 site
    int iInSHSen9DetectPos2x6[2][6];                                            //ChungHung 20130507 add HT9045 updata for 12site 517  //Steven 20120705 : _12Site2X6
    int iInSHSen9DetectPos2x8[2][8];

    //jou 981203 end
    //Eliot 2009_12_24 start
    bool fInArmSuckUse_9To16[2][MAX_ARM_Row][MAX_ARM_Col];                      //9046 Site 9~16     //ChungHung 20130910 alter for SCK can close site by Index
    bool fOutArmSuckUse_9To16[MAX_ARM_Row][MAX_ARM_Col];                        //9046 Site 9~16
    //Eliot 2009_12_24 end

    bool         bContinuousPass;                                               //Eliot 20100708
    unsigned int iContinuousPassBin;                                            //Eliot 20100708
    unsigned int iContinuousPassBinCount;                                       //Eliot 20100708

    bool         bContinuousPassBySocket;                                       //Steven 20110915
    unsigned int iContinuousPassBinCountBySocket;                               //Steven 20110915

//    bool bAutoTrayPassFail[MAX_AUTO_TRAY];     //jou 2010-12-15
    int TestZ_Drop_Speed;

    int iOpenBin;                                                               //jou 2011-03-24 start : Auto Site Mapping

    double  iHangupMaxTime;                                                     //jou 2011-04-01

    int TestY1_Front_EndWaitPos;

    int  iOutARM_X_PITCH;                                                       //jou 2011-09-26  out arm背隙offset
    int  iOutARM_Y_PITCH;                                                       //jou 2011-09-26  out arm背隙offset

    int iQAModeCount;                                                           //Steven 20111026 : QA模式的數量
    int iQAModeRunType;                                                         //Steven 20120612 : QA做完後的動作
    int iQAModeBin;                                                             //Steven 20141023 : QA做完後的Bin
    bool bQAModeAfterTrayEnd;                                                   //Steven 20151125 : QA做完後的TrayEnd要重做QA
    bool bD22SupportMultiDoubleContact;                                         //Sam 20231117 : 整合到 QA 模式
    int iD22DoubleContactCount;                                                 //Sam 20240301 : 新增 QA 模式
    int iTestDown_Z1;
    int iTestDown_Z2;

    //Steven 20120809 : 吸嘴自動校正
    int iInArmPickX;
    int iInArmPickY;
    int iOutArmPickX;
    int iOutArmPickY;

    //Steven 20120921 : Barcode_2D
    int iInSHBarCodeDetectPos1x1[2];
    int iInSHBarCodeDetectPos1x2[2][2];
    int iInSHBarCodeDetectPos2x3[2][3];                                         //ChungHung 20140115 add for 2x3_6
    int iInSHBarCodeDetectPos1x4[2][4];
    int iInSHBarCodeDetectPos2x5[2][5];                                         //Steven 20221027 : Add for 2x5
    int iInSHBarCodeDetectPos2x6[2][6];
    int iInSHBarCodeDetectPos2x8[2][8];
    //Steven 20200810 : 一個IC使用多個2DID
    //==>
    int iInSHBarMulti2DDetectPos1x1[2][2];
    int iInSHBarMulti2DDetectPos1x2[2][2][2];
    int iInSHBarMulti2DDetectPos2x3[2][2][3];
    int iInSHBarMulti2DDetectPos1x4[2][2][4];
    int iInSHBarMulti2DDetectPos2x5[2][2][5];                                   //Steven 20221027 : Add for 2x5
    int iInSHBarMulti2DDetectPos2x6[2][2][6];
    int iInSHBarMulti2DDetectPos2x8[2][2][8];
    //<==
     //Steven 20200810 : 一個IC使用多個2DID

    int iOutSHBarCodeDetectPos1x1[2];
    int iOutSHBarCodeDetectPos1x2[2][2];
    int iOutSHBarCodeDetectPos2x3[2][3];                                        //ChungHung 20140115 add for 2x3_6
    int iOutSHBarCodeDetectPos1x4[2][4];
    int iOutSHBarCodeDetectPos2x5[2][5];                                        //Steven 20221027 : Add for 2x5
    int iOutSHBarCodeDetectPos2x6[2][6];
    int iOutSHBarCodeDetectPos2x8[2][8];

    //Steven 20160920 : IC置偏檢查
    int iInSFCPos1x1[2];
    int iInSFCPos1x2[2][2];
    int iInSFCPos1x3[2][3];
    int iInSFCPos1x4[2][4];
    int iInSFCPos1x5[2][5];                                                     //Steven 20221027 : Add for 2x5
    int iInSFCPos1x6[2][6];
    int iInSFCPos1x8[2][8];
    int iInSFCPos1x1_2[2][2];
    int iInSFCPos1x2_2[2][4];
    int iInSFCPos1x3_2[2][6];
    int iInSFCPos1x4_2[2][8];
    int iInSFCPos1x5_2[2][10];                                                  //Steven 20221027 : Add for 2x5
    int iInSFCPos1x6_2[2][12];
    int iInSFCPos1x8_2[2][16];

    //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
    int iTrayKitStartX;
    int iTrayKitStartY;
    int iTrayKitPitchX;
    int iTrayKitPitchY;

    //jou 2012-12-11 start : Bin Box
    int iOutArmBinBoxX;
    int iOutArmBinBoxY;
    //jou 2012-12-11 end

    // 2012.06.04 , Joye , Rotate KIT ----------------
    int iInArmRotateToHotPlateX;
    int iInArmRotateToHotPlateY;
    int iInArmRotateToHotPlate_PlaceZ[MAX_ARM_Row][MAX_ARM_Col];
    int iInArmRotateToHotPlate_PickZ[MAX_ARM_Row][MAX_ARM_Col];
    int iOutArmRotateToUnloaderX;
    int iOutArmRotateToUnloaderY;
    int iOutArmRotateToUnloader_PlaceZ[MAX_ARM_Row][MAX_ARM_Col];
    int iOutArmRotateToUnloader_PickZ[MAX_ARM_Row][MAX_ARM_Col];
    // -----------------------------------------------
    //2013-04-12    Dell :旋轉站;馬達版
    int iInArm_RotateX;                                                         //In Arm Teach 點位
    int iInArm_RotateY;
    int iInArm_RotatePick[MAX_ARM_Row][MAX_ARM_Col];                            //In Arm Teach Pick點位
    int iInArm_RotatePlace[MAX_ARM_Row][MAX_ARM_Col];                           //In Arm Teach Place點位
    int iOutArm_RotateX;                                                        //Out Arm Teach 點位
    int iOutArm_RotateY;
    int iOutArm_RotatePick[MAX_ARM_Row][MAX_ARM_Col];                           //Out Arm Teach Pick點位
    int iOutArm_RotatePlace[MAX_ARM_Row][MAX_ARM_Col];                          //Out Arm Teach Place點位
    int iIn_iRotateA;                                                           //回Home完成,必需回到0度點位
    int iOut_iRotateA;                                                          //回Home完成,必需回到0度點位
    int iIn_iRotateA_Backlash;                                                  //RogerYang 20260113 : Rotator新增背隙補償
    int iOut_iRotateA_Backlash;                                                 //RogerYang 20260113 : Rotator新增背隙補償
    // -----------------------------------------------

    bool bInitialAutoSiteMap;
    bool fAlreadyCheckASM[2][2][8];                                             //有使用的吸嘴, 對應AutoSiteMap
    bool fNeedToCheckASM[2][2][8];                                              //有使用的吸嘴, 對應AutoSiteMap

    bool fInArmSuck4x8[2][2][8];                                                //有使用的吸嘴, 對應到開關site
    bool bInSuckUse[2][2][8];                                                   //有使用的吸嘴, 對應到Site Mode Auto clean專用
    int  iSiteMap[2][2][8];                                                     //有使用的吸嘴, 對應到Site Map

    int TestY_Pre_MovePos;

    //Steven 20140228 : 雷射測距功能
    int iHP1LaserX;
    int iHP2LaserX;
    int iHP1LaserY;
    int iHP2LaserY;
    int iSH1Laser;
    int iSH2Laser;

    //ChungHung 20140425 add for TSMC Device
    bool bEveryFirstDeviceUseInitialDelay;
    bool bUseOtherArmToTestAfterInitialDelay;                                   //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
    bool bAfterShowAlarmMessageUseInitialDelay;
    bool bWhenHappenTestedTimeBelowUseInitialDelay;
    double iEveryFirstDeviceUseInitialDelay;
    double iInitialDelay;
    bool bWhenPressStopOverUseInitialDelay;                                     //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    double iWhenPressStopOver;                                                  //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    bool bWhenNoFullSiteUseInitialDelay;                                        //wei 20151228 No FullSite delay
    bool bTestFinishToNextTestOver;                                             //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    bool bOTDUnlockDelay;                                                       //Steven 20160818 : OTD打開Delay
    bool bAfterAutoCleanFunctionUseInitialDelay;                                //ChungHung 20141017 add for SCK Add for TSMC Device
    bool bAfterOpenHeatDoorUseInitialDelay;                                     //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    double dAfterTestedDelay;                                                   //ChungHung 20140730 add for ATK function after tested delay time
    bool bTTLUseASEJPMode;                                                      //Frank 20220408 Add TTL ASE_JP Mode
// -----------------------------------------------
    //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    int iTopView_X;
    int iTopView_Y;
    int iTopView_Pick;
    int iTopView_Place;
    int iPADView_X;
    int iPADView_Y;
    int iPADView_Z;
    int iBGAView_X;
    int iBGAView_Y;
    int iBGAView_Z;
    int iTopViewSafePos_X;
    int iTopViewSafePos_Y;
    int iTopViewKit_Zup;
    int iTopViewKit_Z;
    // -----------------------------------------------

    int iXTrayClean;                                                            //wei 20150826 拍拍Tray X軸 Teach
    int iXTrayOCR;                                                              //wei 20151001 OCRTray X軸 Teach

    int iInArmDecay_X;                                                          //Ifor 20151209 Decay In Arm X Teach 點位
    int iOutArmDecay_X;                                                         //Ifor 20151209 Decay Out Arm X Teach 點位
    int iInArmDecay_Y;                                                          //Ifor 20151209 Decay In Arm Y Teach 點位
    int iOutArmDecay_Y;                                                         //Ifor 20151209 Decay Out Arm Y Teach 點位

    //Sam 20200507 : TestIF_File > Prod
    //==>
    bool bAlarm4ContinueType_Enable;
    int iAlarm4ContinueType_IntervalCount;
    int iAlarm4ContinueType_ContinueCount;
    bool bAlarm4EnableIntervalYield;
    int iAlarm4IntervalYieldIntervalCount;
    int iAlarm4IntervalYieldContinueCount;
    int iAlarm4IntervalYieldYield;
    //<==
    //Sam 20200507 : TestIF_File > Prod

    bool bSiteToSiteYieldCmp;
    int  iSiteToSiteYieldCmp;
    int  iSiteToSiteYieldCmpCount;

    bool bHeadToHeadYieldCmp;
    int  iHeadToHeadYieldCmp;
    int  iHeadToHeadYieldCmpCount;

    bool bSiteYieldOverAlert;
    int  iSiteYieldOverAlert;
    int  iSiteYieldOverAlertCount;
    int  OutArm_BGA_Place[MAX_ARM_Row][MAX_ARM_Col];
    int  OutArm_Pad_Place[MAX_ARM_Row][MAX_ARM_Col];
    int  OutArm_ScannerAOI_Place[MAX_ARM_Row][MAX_ARM_Col];
    int  OutArm_ScannerAOI_Pick[MAX_ARM_Row][MAX_ARM_Col];                      //Jimmychiu 20240322 : Top & Bottom Inspect

    int iShuttle1120Pitch;                                                      //wei 20160914 Auto Shuttle Sensor
    int iShuttle1180Pitch;                                                      //wei 20160914 Auto Shuttle Sensor
    int iShuttle2120Pitch;                                                      //wei 20160914 Auto Shuttle Sensor
    int iShuttle2180Pitch;                                                      //wei 20160914 Auto Shuttle Sensor

    double dInitialStartDelayDec;                                               //kevin 20161213 每次減多少秒
    bool   bEnStartDelayCount;                                                  //kevin 20161214 第幾個 Count開始執行送訊號delay
    int   iStartDelayCount;                                                     //kevin 20161214 第幾個 Count開始執行送訊號delay
    int iTrayMapping;                                                           //wei 20161219 Tray Mapping
    int iTrayID;                                                                //wei 20161219 Tray Mapping
    int iTrayDeviceCnt[eTrayCount];                                             //Sam 20190405 : Tray Decive Count  //Steven 20240110 : 3 --> eTrayCount
    bool bIndexUpSpeed;                                                         //kevin 20170524 (wei) add index up speed
    bool bF18InshuttleDetect;                                                   //jou 20170418 : 矽品-世明要求修改config F18至Recipe設定

    int iXTrayLoad_ART;                                                         //kevin 20170831 (Steven) add
    int iXTrayEmpty_ART;                                                        //kevin 20170831 (Steven) add
    int iXTrayColor_ART;                                                        //kevin 20170831 (Steven) add
    int iXTrayAuto_ART[MAX_AUTO_TRAY];                                          //kevin 20170831 (Steven) ART Auto->Load offset
    int TestY1_Middle_Home;                                                     //kevin 20171023 (wei) Y1 home Z phase
    int TestY2_Middle_Home;                                                     //kevin 20171023 (wei) Y2 home Z phase

    //wei 20180606 Interval Low Yield By Site
    bool bFailAlarmIntervalLowYieldBySite;
    double dIntervalLowYieldLimitBySite;                                        //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iIntervalLowYieldCountBySite;

    //wei 20180718 Interval Low Yield By Total
    bool bFailAlarmIntervalLowYieldByTotal;
    double dIntervalLowYieldLimitByTotal;                                       //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iIntervalLowYieldCountByTotal;

    bool bTestStartToNextTestStart;                                             //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41
    double dTeststartToNextTestStart;                                           //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41  over time
    double dInitialDelay_10;                                                    //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41 wait time
    //Frank 20180410 (Steven) : InArm Preciser Station
    int iInArmPreciserX;
    int iInArmPreciserY;
    int iInArmPreciserPlaceZ[MAX_ARM_Row][MAX_ARM_Col];                         //KaiChen 20190516
    int iInArmPreciserPickUpZ[MAX_ARM_Row][MAX_ARM_Col];                        //KaiChen 20190516
    int iPreciserOpenPitch;
    int iPreciserClosePitch;

    //KaiChen 20200525 ：Daily Correlation Function
    int iInArmDailyCorrelationX;
    int iInArmDailyCorrelationY;
    int iInArmDailyCorrelationPickZ;

    int iLoadCellY1;                                                            //kevin 20190306 Arm1 在load cell 位置
    int iLoadCellY2;                                                            //kevin 20190306 Arm2 在load cell 位置
    double dLoadCellZ1Down;                                                     //kevin 20190306 Arm1 在load cell 位置
    double dLoadCellZ2Down;                                                     //kevin 20190306 Arm2 在load cell 位置

    int iCassetteArmX[10];                                                      //wei 20180702 MR
    int iCassetteArmZ[10];                                                      //wei 20180702 MR
    int iLoadPortZ[3];                                                          //wei 20180702 MR
    int iStackedTrayX[6];                                                       //wei 20180702 MR
    int iStackedTrayZ[6];                                                       //wei 20180702 MR
    int iTrayBracketZ[2];                                                       //wei 20180702 MR
    int iLoadRobotZ[5];                                                         //Sam 20190112 LM
    int iUnloadRobotZ[5];                                                       //Sam 20190112 LM

    int iLoadCEllReadPosZ1;                                                     //kevin 20190909 LoadCEll 量測高度位置
    int iLoadCEllReadPosZ2;                                                     //kevin 20190909 LoadCEll 量測高度位置
    //Ifor 20191211 : add Scanner AOI
    //==>
    int iScannerAOI_X;
    int iScannerAOI_Y;
    int iScannerAOI_Z;
    int iScannerAOI_Z_Pick;
    //<==
    //Ifor 20191211 : add Scanner AOI

    //JimmyChiu 20220908 add Pickup Error Placement
    //==>
    int iInPlacementX;
    int iInPlacementY;
    int iInPlacementOffsetByBasicX;
    int iInPlacementOffsetByBasicY;
    //<==
    //JimmyChiu 20220908 add Pickup Error Placement

    //Frank 20210612 : Flipper Function
    //==>
    int iInArmFlipperX[3];
    int iInArmFlipperY[3];
    int iInArmFlipperZPick[3][MAX_ARM_Row][MAX_ARM_Col];
    int iInArmFlipperZPlace[3][MAX_ARM_Row][MAX_ARM_Col];
    int iInArmFlipperR[3];
    int iOutArmFlipperX[3];
    int iOutArmFlipperY[3];
    int iOutArmFlipperZPick[3][MAX_ARM_Row][MAX_ARM_Col];
    int iOutArmFlipperZPlace[3][MAX_ARM_Row][MAX_ARM_Col];
    int iOutArmFlipperR[3];
    //<==
    //Frank 20210612 : Flipper Function

    int iCCDX_Center;
    int iCCDY_Center;
    int iCCDZ;
    int iKeyX_Center;
    int iKeyY_Center;
    int iKeyZ_Center;

    int iXDimension_FP;
    int iYDimension_FP;
    int iXStart_FP;
    int iYStart_FP;
    int iXVerify_FP;
    int iYVerify_FP;
    int iArmZForCCD[2];
    int iArmZForCal[2];
    double TestZ_Up_Wait;
    int TestZ_Up_Speed;
    int TestZ1_Up_Offset;
    int TestZ2_Up_Offset;
    bool bSingleUseOtherSuck;                                                   //wei 20220905 Single Use Other Suck(IN C OUT E)
    bool bSingleUseDSuck;                                                       //Frank 20241209 add Single Use D Suck
    bool bSingleInArmUseOtherSuck;                                              //Ifor 20240430 add: Single Site InArm Use Other Suck(IN C)
    //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    //==>
    int iInArmXCCDAlignmentPos;                                                 //ChungHung 20210113 add for Alignment CCD
    int iInArmYCCDAlignmentPos;                                                 //ChungHung 20210113 add for Alignment CCD
    int iInArmXBasePickerAlignmentPos;                                          //ChungHung 20210113 add for Alignment CCD
    int iInArmYBasePickerAlignmentPos;                                          //ChungHung 20210113 add for Alignment CCD

    int iInArmZBasePickerAlignmentPos[MAX_ARM_Row][MAX_ARM_Col];                //ChungHung 20210113 add for Alignment CCD

    double dInArmCCDXResolution;                                                //ChungHung 20210113 add for Alignment CCD
    double dInArmCCDYResolution;                                                //ChungHung 20210113 add for Alignment CCD
    double dInArmCCDXRadian;                                                    //ChungHung 20210113 add for Alignment CCD
    double dInArmCCDYRadian;                                                    //ChungHung 20210113 add for Alignment CCD

    int iOutArmXCCDAlignmentPos;                                                //ChungHung 20210113 add for Alignment CCD
    int iOutArmYCCDAlignmentPos;                                                //ChungHung 20210113 add for Alignment CCD
    int iOutArmXBasePickerAlignmentPos;                                         //ChungHung 20210113 add for Alignment CCD
    int iOutArmYBasePickerAlignmentPos;                                         //ChungHung 20210113 add for Alignment CCD

    int iOutArmZBasePickerAlignmentPos[MAX_ARM_Row][MAX_ARM_Col];               //ChungHung 20210113 add for Alignment CCD

    double dOutArmCCDXResolution;                                               //ChungHung 20210113 add for Alignment CCD
    double dOutArmCCDYResolution;                                               //ChungHung 20210113 add for Alignment CCD
    double dOutArmCCDXRadian;                                                   //ChungHung 20210113 add for Alignment CCD
    double dOutArmCCDYRadian;                                                   //ChungHung 20210113 add for Alignment CCD

    int iInArmCCD_Picker_OffsetX[MAX_ARM_Row][MAX_ARM_Col];                     //ChungHung 20210113 add for Alignment CCD
    int iInArmCCD_Picker_OffsetY[MAX_ARM_Row][MAX_ARM_Col];                     //ChungHung 20210113 add for Alignment CCD

    int iOutArmCCD_Picker_OffsetX[MAX_ARM_Row][MAX_ARM_Col];                    //ChungHung 20210113 add for Alignment CCD
    int iOutArmCCD_Picker_OffsetY[MAX_ARM_Row][MAX_ARM_Col];                    //ChungHung 20210113 add for Alignment CCD
    //<==
    //KenHsieh 20210813 : add CCD AUTO ALIGNMENT

    int iTrayArmZPnP[10];
    int iTrayArmZSafe[10];
    int iAutoClean_ContactMode;                                                 //Sam 20230111 : Smart Auto Clean
    int iAutoCleanDropHigh;
    int iAutoClean_ContactTime;
    int iAutoClean_ContactCount;

    int iCatchMazTray_Front;                                                    //JerryYang 20220909 : add magazine
    int iCatchMazTray_Rear;
    int iMagazineTrayPos[15];

    int  iIsFailT6[eTrayCount];
    int  iIsPassT6[eTrayCount];
    int  iTrayType[eTrayCount];
    bool bART6Tray[eTrayCount];
    bool bLinkTo6Tray[eTrayCount];
    bool bCateRTo6Tray[eTrayCount];

    int  iTo6StackDefFailCate   [8][eTrayCount];
    int  iTo6TrayType           [8][eTrayCount];
    bool bTo6AutoRetest         [8][eTrayCount];
    bool bTo6AutoLink           [8][eTrayCount];
    bool bTo6CateR              [8][eTrayCount];
    int  iTo6IfError            [8];
    int  iTo6CatData            [8][TEST_MAX_BIN];                              //Auto1 = 0
    int  iTo6CatePos            [8][TEST_MAX_BIN];                              //Auto1 = 1
    int  iQASamplingT6;
    int  iQASamplingT6Pos;

    int RotationTimeIn;                                                         //jou 20231122 : Rotate Use RT mode
    int RotationTimeOut;                                                        //jou 20231122 : Rotate Use RT mode
    int RotationCount[8];                                                       //jou 20231122 : Rotate Use RT mode
    int OutRotationCount[8];                                                    //jou 20231122 : Rotate Use RT mode
    int RotateDutDate[2][4][8];                                                 //jou 20231122 : Rotate Use RT mode
    bool bUseSocketHeating;                                                     //Ztex 2024.09.07 Add Use Socket Heating
    int iUseSocketHeating;                                                      //Ztex 2024.09.07 Add Use Socket Heating

    bool bAdaptiveLowYield;                                                     //Sam 20230914 : 自適應性良率監控
    int iAdaptiveContsLowerAlarmNor;                                            //Sam 20240726 : AI Clean
    int iAdaptiveContsLowerAlarmMin;
    int iAdaptiveYieldMax;
    int iAdaptiveYieldMin;

    int iInSmartSetup_X;                                                        //Ifor 20240423 add: Smart Setup In Arm X Teach 點位
    int iOutSmartSetup_X;                                                       //Ifor 20240423 add: Smart Setup Out Arm X Teach 點位
    int iInSmartSetup_Y;                                                        //Ifor 20240423 add: Smart Setup In Arm Y Teach 點位
    int iOutSmartSetup_Y;                                                       //Ifor 20240423 add: Smart Setup Out Arm Y Teach 點位
    int iInSmartSetup_Z;                                                        //Ifor 20240423 add: Smart Setup In Arm Z Teach 點位
    int iOutSmartSetup_Z;                                                       //Ifor 20240423 add: Smart Setup Out Arm Z Teach 點位

    int iAutoClean_InitalContactCount;                                          //Ifor 20240604 add:Auto Clean Inital Contact Count
    int iBottomBlowInX;
    int iBottomBlowInY;

    bool bRunRT;

    int iMLoaderYCarPos;                                                        //Frank 20250214 add
    int iMLoaderYOCRPos;
    int iMLoaderYSurePos;

    int iInSH1SenICDetectZ1;                                                    //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    int iInSH1SenICDetectZ2;
    int iInSH2SenICDetectZ1;
    int iInSH2SenICDetectZ2;
    int iInSH1SenICAddPos;
    int iInSH2SenICAddPos;

    int iLDCassetteFront;                                                       //Frank 20251217 add
    int iLDCassetteFrontBack;
    int iLDCassetteRear;
    int iLDCassetteRearBack;
    int iLDCassetteZStart;

    int iAutoCassetteFront[2];
    int iAutoCassetteFrontBack[2];
    int iAutoCassetteRear[2];
    int iAutoCassetteRearBack[2];
    int  AOICatData [TEST_MAX_BIN];                                             //Eastsun 20260317 : AOI 分BIN

    int iAutoCassetteZStart[2];
    int  CatData [TEST_MAX_BIN];                                                //Eastsun 20260515 F011 整合 (KYEC AMR ProcessSensorScan)
} PROD_INFO_ST;
extern PROD_INFO_ST Prod;
//Steven 20110801 Start: 改用結構方式存檔-----------------
/*typedef struct
{
    long SendCT[4];
    long BinCT[4][20];
} LAST_GENERAL_COUNT;
extern LAST_GENERAL_COUNT LastCount;    */                                      //沒用到 Mark
//Steven 20110801 End: 改用結構方式存檔-------------------
//==============================================================================
typedef struct
{
    int  AccessLevel[256];
//    int  iTrayColor[256];   //沒用到
} LAST_LEVEL_SET;
extern LAST_LEVEL_SET LevelSet;
//==============================================================================
typedef struct
{
    double IndexArmPick[2];
    double IndexPlace[2];
    double IndexDrop[2];
    double IndexContact[2];
    int ContactMode;
    int VacuumMode;
    int DummyMode;
    double dPress;
    double DropWait;
//    double Distance;
    int iPinCT;
    double XDimension;
    double YDimension;
    //Steven 20210624 : Fine Pitch
    //==>
    double XDimensionFP;
    double YDimensionFP;
    double XStartFP;
    double YStartFP;
    double VerifyXFP;
    double VerifyYFP;
    double CCDOffsetFP;
    double CalCCDOffset;
    AnsiString CalCCDIP;
    //<==
    //Steven 20210624 : Fine Pitch

//    double Stroke;
    double ForcePerPinN;
    double ForcePerPinG;                                                        //wei 20150729
    double DoubleForce;                                                         //Ifor 20190104 : Add Double EP Control
    int iPinOfDie;                                                              //Steven 20191106: Pin of Die
//    bool bDeviceLoading;
    int iHeadDeviceCT;
    double fAireForce;
    bool bSuckShuttleDeviceAfterTested;
    bool bSuckShuttleDeviceWaitOnShuttle;                                       //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
//    bool bSuckDevicesDuringTest;    //JerryYang 20170804 (Steven) Mark掉,drop contact改為強制邊吸邊測    //JerryYang 20170522 (wei) drop contact改為邊吸邊測
    bool bShuttleWaitingOutSiteChamber;
    bool bTesterSidePush;                                                       //Richard 20220321 : 渠梁Side Push
    int  iSidePushMode;                                                         //Richard 20220321 : 渠梁Side Push

    bool bPickShuttleDeviceTogether;
    //double fKitDepthOffset;       //Steven 20091116 : Kit depth offset
    int DropSpeed;
    double dKitDiameter;
    double dDieForceKitDiameter;                                                //Ifor 20191003 : add Die Force 可以自定義Kit直徑
    bool bUseDieForce;                                                          //Ifor 20220218 add:KYEC 要求新增Dual Force 開關
    double IndexContactBackUp[2];                                               //ChungHung 20130715 add ATK 顯示上次AutoHeight的值

//    double IndexContactShuttleRelBackUp[2]; //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
    double IndexContactShuttlePickUp[2];                                        //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
//    bool bUseNSKit;
    int   iSocketInitialICCheckPosition;
    double fSocketInitialICCheckPositionOffset;
    int iIndexTorqueMax;
    int iIndexTorqueCmp;

    //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue start
    bool bPurgeBeforePickShuttle;
    int  iPurgeBeforePickShuttleTime;
    int  iPurgeBeforePickShuttleInterval;
    int  iPurgeBdforePickShuttleOffSet;
    //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue end
    bool  bIndexUpSpeed;                                                        //kevin 20170524 (wei) add index up speed
    bool bUseAddWeight;                                                         //kevin 20170629 (Steven) direct heat add weight

    double dZ1Torue;                                                            //Frank 20170627 add
    double dZ2Torue;                                                            //Frank 20170627 add
    bool bEnableUseUniversalShuttle;                                            //Frank 20171030 (Steven) add Enable Universal Kit
    double dLoadCellZ1Down;                                                     //kevin 20190306 Arm1 在load cell 位置
    double dLoadCellZ2Down;                                                     //kevin 20190306 Arm1 在load cell 位置
    int iAutoHeightSHTReleaseOfs;                                               //Sam 20200217 : K高後 Shuuttle Release Height offset By SetupFile

    int iOffsetX[2][2][8];
    int iOffsetY[2][2][8];
    int iOffsetR[2][2][8];

    int iKitDiameterMode;                                                       //kevin 20210813 Kit 外徑 選擇
    double UpWait;
    int UpSpeed;
    double IndexUp[2];
    double dDropByPassDetect;
    double dSitePushWaitTime;                                                   //Richard 20220321 : 渠梁Side Push
    double DieForcePerPinN;                                                     //Ifor 20251203 add:
    double DieForcePerPinG;                                                     //Ifor 20251203 add:
}SYSTEM_DEVICE_FORM;
extern SYSTEM_DEVICE_FORM DeviceForm;
extern SYSTEM_DEVICE_FORM DeviceForm_File;
extern SYSTEM_DEVICE_FORM DeviceForm_NET;                                       //Ifor 20181023 add SCC要求FTP資料卡控加入Contact相關資料
//==============================================================================
//typedef struct
//{
//    int Type;
//    int Degree;
//    int Direction;
//    char Alias[26];
//}TRAY_TYPE;
//==============================================================================
typedef struct
{
    double XPitch;
    double YPitch;
    double XStart;
    double YStart;
    int XDivision;
    int YDivision;
    double ZDepth;
    int  Direction;
    bool TrayEndRecv;                                                           //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
    int iTrayType;
    char Alias[30];
//    AnsiString Alias;
    double iPickUp;
    int iPlateSelect;
    char cMemo[50];                                                             //Steven 20090716
//    AnsiString cMemo;
    bool bUseWideHotplate;                                                      //jou 2010-12-15
    int iBinBoxAlarm;                                                           //jou 2012-12-11 support Bin Box
    int iDeviceDirection;                                                       //jou 2013-03-25 Device Direction
    bool bEnableIndicator;                                                      //Steven 20190211 : 可以取消顯示IC方向
    int iTrayDirection;                                                         //jou 2013-03-25 Tray Direction
    int BlockNumberX;                                                           //2014-03-04    Dell    for SPIL WLP Add Tray Block
    int BlockNumberY;                                                           //2014-03-04    Dell    for SPIL WLP Add Tray Block

    double BlockXStart;                                                         //Frank 20160920 add
    double BlockYStart;                                                         //Frank 20160920 add
    double BlockPitchX;                                                         //2014-03-04    Dell    for SPIL WLP Add Tray Block
    double BlockPitchY;                                                         //2014-03-04    Dell    for SPIL WLP Add Tray Block

    double BlockTraySize;

    int BlockXItem;                                                             //Frank 20160920 add
    int BlockYItem;                                                             //Frank 20160920 add
    bool bUseThickTray;                                                         //JerryYang 20230620 : add厚tray選項

    bool bTrayHotplateCheck;                                                    //jou 20240126 : Tray & hotplate by recipe MES控制檢查

    int iCassetteZItem;                                                         //Frank 20251217 add
    double dCassetteZStart;                                                     //Frank 20251217 add
    double dCassetteZPitch;                                                     //Frank 20251217 add
}TRAY_TYPE_PARA;
//==============================================================================
typedef struct
{
    int  LodareType;
//    TRAY_TYPE_PARA Fix[eTrayCount];
    TRAY_TYPE_PARA Auto[eTrayCount];
    TRAY_TYPE_PARA Loader;
    TRAY_TYPE_PARA Empty;
    TRAY_TYPE_PARA Color;
    int  LoaderToEmptyColor[2];                                                 //   0 :empty     1:color
    int  AutoFromEmptyColor[2][eTrayCount];
    int  iRotateKIT_InputType;                                                  //InArmRotateType --> iRotateKIT_InputType
    int  iRotateKIT_OutputType;                                                 //OutArmRotateType --> iRotateKIT_OutputType
    int  iFixTrayMode;
    int iUsePickUnitCount;
    bool bAutoFeed;
    bool bColorTray;                                                            //20140903 wei colcr Tray
    bool bChkLoadDirection;                                                     //Steven 20190815 : JCET不重測Tray偵測
    bool AutoCoverInitial;
    bool AutoCoverRetest;
    int  iTrayTransportMode;
    int  iManualRemoveLoader;                                                   //Steven 20130819 : 加入開門檢查的功能  (bool --> int)
    bool bFailAutoTrayManual_FT;                                                //Steven 20150116 : 手動移除Auto Fail Bin Tray
    bool bFailAutoTrayManual_RT;                                                //Steven 20150116 : 手動移除Auto Fail Bin Tray
    int  iMagTraySource;                                                        //JerryYang 20221215 : Magazine tray由Tray arm補
    int  iMagFixTrayType;                                                       //JerryYang 20221215 : Magazine把fix區當buffer區功能
    int  iMagDisplayOrder;                                                      //Ifor 20240227 add:Magazine 顯示順序變更
    bool bMoveAfterTrayGoOut;                                                   //JerryYang 20241002 : SPIL要求功能 Trray arm等AUTO TRAY退到外面才能移動
    bool bSpecTrayCnt;                                                          //JerryYang 20250212 : add
    int iFullTrayCount;
    int iInputTrayCount;

    bool bVTestNoRTBin;                                                         //RogerYang 20250814 偉測不可複測bin功能
    AnsiString asNoRTBinFix[3];                                                 //RogerYang 20250626 偉測不可複測bin功能

//    int  bManualRemoveLoaderMode; //0 ,1by manual take out or by show message
//==============================================================================
// GEM function Start
//==============================================================================
    AnsiString asGEM_LoaderTo_FT;
    AnsiString asGEM_LoaderTo_RT;
    AnsiString asGEM_TrayType;

    bool bFixTrayLink[eTrayCount];
    bool bTrayUpDownSet[eTrayCount];                                            //wei 20160224 TSMC FIX UPDOWN
    bool bTraySortCntFunc[eTrayCount];                                          //Steven 20210608 : Tray指定數量退盤功能
    int  iTraySortCntFunc[eTrayCount][6];                                       //Steven 20210608 : Tray指定數量退盤功能

    int  iTrayOrder[eTrayCount];
    bool bIDTrayOrder[eTrayCount];

    bool bEnableAMR;
    bool bEnableAMRLoader;                                                      //Eastsun 20260515 F009 整合
    int  iReaderPos;

    int iUnloadTrayCount;                                                       //jou 20240617 : Unload tray count alarm
    bool bAutoTrayGoOutNeedDelay[3];                                            //Ifor 20260304 add: Auto Tray Go Out Need Delay
    int  ibAutoTrayGoOutDelayTime;                                              //Ifor 20260304 add: Auto Tray Go Out Delay Time
//==============================================================================
}SYSTEM_TRAY_FORM;
extern SYSTEM_TRAY_FORM TrayForm;
extern bool bDoRTCLearning;                                                     //Ifor 20260226 add: Contact Mode RTC Learn
extern TRAY_TYPE_PARA HotPlateForm;
extern TRAY_TYPE_PARA HotPlateForm_File;
extern TRAY_TYPE_PARA UserDefForm[4];
extern TRAY_TYPE_PARA UserDefForm_File[4];
extern TRAY_TYPE_PARA *LoadForm;
extern TRAY_TYPE_PARA *AutoForm[eTrayCount];
//extern TRAY_TYPE_PARA *FixForm[eTrayCount];
extern TRAY_TYPE_PARA *EmptyForm ;
extern TRAY_TYPE_PARA *ColorForm ;
//extern TRAY_TYPE_PARA *OutputForm[eTrayCount];

extern SYSTEM_TRAY_FORM TrayForm_NET;                                           // 2013.12.03 , Joye , KYEC FTP     //20140103 wei
extern TRAY_TYPE_PARA HotPlateForm_NET;                                         // 2013.12.03 , Joye , KYEC FTP
//==============================================================================
typedef struct
{
    int     iMachineTempMode;
    double  fSoakTime;
    double  fJamSoakTime;
    double  fInitialWaitTime;
    bool    bUseAbitCHK;
    int     iAbitCHKStatus;
    double  fAbitTemp;
    double  fAbitInitWaitTime;
    double  fAbitColdTime;
    int     iTempMode;
    //double  fTempOffSet[4][tcTotalCount];
    double  fTempOffSet[19][tcTotalCount];                                      //kevin 20160312 恆溫控制 9->12   //Steven 20141119 : 8 --> 9  //JerryYang 20221005 : 12->16  Ztex 2024.07.27 16 ->17
    double  fLowBase;
    double  fMiddBase;
    double  fHighBase;
    double  fWorkTemperBase;
    int     iIndexHeatMode;                                                     //Steven 20090926
    int     iInitialStart1Time;                                                 //jou 981030
    int     iInitialStart2Time;                                                 //jou 981030
    double  fChamberCoolTemp;                                                   //Steven 20111209 : Chamber降溫溫度
    bool    bAmbUsingAFan;                                                      //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動
    bool    bATCActiveCooling;
    int     iATC60Air;
    bool    bATCTemperatureSet;                                                 //jou 2012-03-26 是否透過通訊控制ATC
    bool    bATCHandlerStart;                                                   //jou 2012-03-26 Handler start檢查溫度
    bool    bATCTestStrat;                                                      //jou 2012-03-26 Test start檢查溫度
    double  dATCSP2;                                                            //jou 2012-03-26 SP2 alarm temp上限值
    double  dATCInPC[4];                                                        //jou 2012-03-26 InPC temp offset
    double  ATCTempReferRange[4];                                               //Steven 20150108 : [L11-5] For海思使用兩組感溫
    double  ATCInitialOffset[4];                                                //Steven 20151006 : Initial Temp Offset for ATC
    double  dATCConFailOffset[4];                                               //Steven 20151123 : Continue Fail Temp Offset for ATC
    int     iATCCurrentFailCount[4];                                            //Steven 20151123 : Continue Fail Temp Offset for ATC
    bool    bEnableATCConFailOffset;                                            //Steven 20151123 : Continue Fail Temp Offset for ATC
    int     iATCConFailOffsetCount;                                             //Steven 20151123 : Continue Fail Temp Offset for ATC
    double  dATCQAModeOffset[4];                                                //Steven 20151125 : QA Mode Temp Offset for ATC
    bool    bEnableATCQAModeOffset;                                             //Steven 20151125 : QA Mode Temp Offset for ATC
    bool    bEnableATCTestTimeOffset;                                           //Steven 20160216 : 測試時間太短也要Offset
    int     iATCTestTimeOffsetTime;                                             //Steven 20160216 : 測試時間太短也要Offset
    double  dATCTestTimeOffset[4];                                              //Steven 20160216 : 測試時間太短也要Offset

    int     iShuttleSoakTimeMode;                                               //jou 2012-06-29 Index Pick up need wait Soak Time
    int     iATCChillerTemp;                                                    //Steven 20120410 : Hontech ATC
    bool    bActiveHeatGun;                                                     //Steven 20120525 : 熱風槍
    bool    bActiveATCHeatGun;                                                  //JerryYang 20220408 : add for ATC3.5
    bool    bUseCDAOnly;                                                        //Steven 20181012 : 使用熱風槍吹冷風
    int     iHotGunFLowLimit_H;                                                 //KaiChen 20190729 ：Hot Gun Flow
    int     iHotGunFLowLimit_L;                                                 //KaiChen 20190729 ：Hot Gun Flow
    int     iIndexSoakTime;                                                     //2013-11-27   Dell    增加Index從Shuttle吸起IC時的Soak Time
    int     iOSTime;                                                            //2013-11-27   Dell    當發生o/s 也是要做Index soak time
    bool    bUseFixTemp;                                                        //Steven 20131025 : 使4固定的DUT溫度
    double  dFixedTemp;                                                         //Steven 20131025 : 固定的DUT溫度值
    bool    bShowFixedTemp;                                                     //Steven 20131025 : True -> 畫面上要顯示固定的DUT溫度值, False -> 畫面上顯示fWorkTemperBase溫度值
    bool    bZ2DownSocket;                                                      //kevin 20131211 一開始 Z2在下
    bool    bUseTesterDocking;                                                  //jou 2014-04-25 Tester Docking Wait delay time
    int     iUseTesterDocking;                                                  //jou 2014-04-25 Tester Docking Wait delay time
    double  fIndividualTemp[tcTotalCount];                                      //Steven 20140924 : 各個加熱區獨立有自己的設定值
    bool    bUseIndividualTemp;                                                 //Steven 20140924 : 各個加熱區獨立有自己的設定值
    double  fAmbientHotGuartbent;                                               //kevin 20141006 恆溫控制溫度
    double  fAmbientHotLowBase;                                                 //kevin 20141006 恆溫控制溫度 低
    double  fAmbientHotMiddBase;                                                //kevin 20141006 恆溫控制溫度 高
    bool    bUseTjFunction;                                                     //2014-05-30    Dell    for ATC6.0
    int     iTjMode;                                                            //2014-05-30    Dell    for ATC6.0
    int     iTjAvgTimes;                                                        //2014-05-30    Dell    for ATC6.0
    bool    bEnableTempOffsetForInitial;                                        //Steven 20141117 : 起測時溫度要補Offset
    int     iCintactCntForTempOffsetAtInitial;                                  //Steven 20141117 : 起測時溫度要補Offset
    int     iCintactDelayCntForInitTempOffset;                                  //Steven 20141117 : 起測時溫度要補Offset
    bool    bEnableChamberBoost;                                                //Steven 20191128 : Chamber Boost Function
    int     iChamberBoostTime;
    int     iChamberBoostOffset;

    bool    bATC70Active;                                                       //Eliot 2015_0105
    bool    bATC7ChannelEnabled[4];                                             //Eliot 2015_0105
    bool    bATC7TSDFunction;                                                   //Eliot 2015_0105
    double  dATC7TSDTimeOut;                                                    //Steven 20160604 : by site TSD
    AnsiString ATCFileName;                                                     //Steven 20150810 : add for ATC 7.0
    //Ifor 20150910 :ATC PID
    bool    bUsePIDControl;
    double  iATC_PID_Min_Offset[3];                                             //Steven 20151111 : 改成double, 不然SECS GEM會死翹翹
    double  iATC_PID_Max_Offset[3];                                             //Steven 20151111 : 改成double, 不然SECS GEM會死翹翹

    double  Arm1Offset;
    double  Arm2Offset;

    double  Arm1NoFullsiteOffset_1;
    double  Arm1NoFullsiteOffset_2;
    double  Arm1NoFullsiteOffset_3;
    double  Arm1NoFullsiteOffset_4;                                             //wei 20160603 add 兩組Nofullsite補償
    double  Arm1NoFullsiteOffset_5;
    double  Arm2NoFullsiteOffset_1;
    double  Arm2NoFullsiteOffset_2;
    double  Arm2NoFullsiteOffset_3;
    double  Arm2NoFullsiteOffset_4;                                             //wei 20160603 add 兩組Nofullsite補償
    double  Arm2NoFullsiteOffset_5;

    double  dATCTempOffset[32];                                                 //Ifor 20160419 add ATC 32 Site Temp Offset
    double  dATCInitTempOffset[32];                                             //Ifor 20250419 add ATC 32 Site Init Temp Offset
    double  dATCPackageOffsettemp;                                              //Ifor 20190214 : add Package Offset
    double  dATCPackageOffset[3];                                               //Ifor 20190306 : add Package Offset 三點校正
    double  dATCPackageTemp[3];                                                 //Ifor 20190306 : add Package Offset 三點校正
    double  dIndexATCSecondTempOffset[32];                                      //Steven 20181102 : For QTI全球統一Offset
    double  dIndexATCInitTempOffset[32];                                        //Steven 20190606
    bool    bUseReferTempSensor;                                                //Ifor 20160421 add 第二點溫度Sensor 開關
    bool    bATCPreOffset;
    bool    bTestCompleteWaitTemp;                                              //Ifor 20250603 add:測試完成等待溫度

    bool    bUseTestTimeBelowNeedDelay;                                         //Ifor 20250714 add:測試秒數低於設定值且為Fail Bin 下一個Contact 需要延遲
    double  dTestBelowTime;                                                     //Ifor 20250714 add:測試秒數低於設定值且為Fail Bin 下一個Contact 需要延遲
    double  dTestBelowDelayTime;                                                //Ifor 20250714 add:測試秒數低於設定值且為Fail Bin 下一個Contact 需要延遲

    double  dSocketAirCoolingOnTimer;                                           //jou 2016-04-28 Socket Air Cooling contact count trun on
    double  dSocketAirCoolingOffTimer;                                          //jou 2016-04-28 Socket Air Cooling contact count trun on
    bool bUseInitialDelayAsSoakTime;                                            //Steven 20170329 (Wei) : 使用initial delay當 Soak time
    bool bTempAlarmBinNeedToError;                                              //Steven 20251022 : Temp alarm need put to error bin

    bool    bAmbientGuardbandCheck;                                             //kevin 20180115 (Steven) add Amient Guard Band
    int     iAmbGuardband;                                                      //kevin 20180115 (Steven) add Amient Guard Band
    bool    bShuttleNoHeatUp;                                                   //Steven 20180815 : Amb Ctr mode, shuttle no heat up
    bool    bSLKNoHeatUp;                                                       //Steven 20230221 : Amb Ctr mode, SLK no heat up

    bool    bBoostFuncttion;                                                    //Steven 20180817 : Boost Function
    bool    bLBTempFunction;                                                    //Steven 20181023 : LB溫度

    int     iATC_FFCTimeOn[2][10];                                              //Ifor 20190730 : add FFC Temperature Offset
    int     iATC_FFCTimeOff[2][10];                                             //Ifor 20190730 : add FFC Temperature Offset
    double  dATC_FFCOffset[2][10];                                              //Ifor 20190730 : add FFC Temperature Offset
    bool    bATC_FFCPointUse[2][10];                                            //Ifor 20190730 : add FFC Temperature Offset
    bool    bATC_FFCEnable;                                                     //Ifor 20190730 : add FFC Temperature Offset

//    double  dLBTempMin;                 //Steven 20181023 : LB溫度
//    double  dLBTempOffset;              //Steven 20181023 : LB溫度
//    bool    bLBBoostOffset;
//    double  dLBBoostTempMin;            //Steven 20181023 : LB溫度
//    double  dLBBoostTempOffset;         //Steven 20181023 : LB溫度

    enum BoostFunction
    {
        eBMax=0,                                                                //Boost Offset Max idle
        eBMid=1,                                                                //Boost Offset Current
        eBMin=2,                                                                //Boost Offset Min idle
        eBLBI=3,                                                                //L/B Initial Temp Offset
        eBLBL=4,                                                                //L/B Low temp Offset
        eBLBB=5,                                                                //L/B Boost Offset
        ebTotal
    };
    bool    bEnableBoostOffset[ebTotal];
    double  dBoostIdleTime[ebTotal];
    double  dBoostOffset[ebTotal];
    double  dBoostDuration[ebTotal];
    double  dPostBoostDuration[ebTotal];
    double  dThreshold;                                                         //Steven 20190928 : L/B升溫的門檻值
    int     iBoostFunctionMode;
    double  dBoostTimeOut;                                                      //Steven 20181222 : Add LB升溫的Time out
    double  dTJTempRange_High;                                                  //Ifor 20190328 : add TJ Temp Over Range
    double  dTJTempRange_Low;                                                   //Ifor 20190328 : add TJ Temp Over Range
    bool    bWaitDewPoint;                                                      //Steven 20191017 : 露點計
    double  dDewPointRange;                                                     //Steven 20191017 : 露點計
    int     iDewPointAlarmInterval;                                             //Steven 20191017 : 露點計
    bool    bATC7TJMonitor;                                                     //Ifor 20181121 : Add ATC7.0 TJ Current Monitor Function
    bool    bTempCalByRecipe;                                                   //jou 20220725 : Temperature calibration by recipe
    bool b3SigmaTempMonitior_Enable;                                            //kevin 20200521 add 3 Sigma 溫度記錄比對
    int i3SigmaTempMonitior_SetCoun;                                            //kevin 20200521 add 3 Sigma Index Contract time
    double d3SigmaTempMonitior_Set3xSigmaValue;                                 //kevin 20200521 add 3 Sigma 客戶比對誤差值
    bool    bLBCoolingAirOn;                                                    //JerryYang 20220923 : LB吹氣function
    double  dLBAirOnTemp;                                                       //JerryYang 20220923 : LB吹氣function
    int     iLBTempAlmInterval;
    double  dATC_HotGunTemp;                                                    //Ifor 20220419 add:ATC Hot Gun Temp
    double  dATC_HotGunTime;                                                    //Ifor 20220419 add:ATC Hot Gun Temp
    bool bByPassChamber;                                                        //KenHsieh 20230301 : By Pass Chamber
    //Ztex 2023.04.18 Add HT-1032 ATC TJ Function
    bool bEnableTJFunction;
    int iTjSiteMapType;                                                         //JerryYang 20231128 : 傳送ATC site map
    double fSetTJ_Offset;
    double fSetTJ_Slope;
    double fTjInputVLow;                                                        //JerryYang 20231205 : ATC Tj WatchDog
    double fTjInputVHigh;
    double dDelayAfterSOT;
    double dDelayAfterSOTContinue;                                              //JerryYang 20251201 : ATC連動watchdog新增continue欄位
    bool bTjWatchdog;
    int iATCCH_Tj[16];

    bool bEnableUseSetTempRangeForTJ;
    AnsiString sEnableUseSetTempRangeForTJ;
    bool bEnableShowTjTempNotCheck;
    //Ztex 2023.04.18 Add HT-1032 ATC TJ Function

    //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    double fHumidityAnomalyDelayAlarm;
    bool bTriTempMachine_ControlHeater_OutShuttle;
    bool bTriTempMachine_ControlHeater_Index;
    bool bTriTempMachine_ControlHeater_Base;
    int iTriTempOutShuttle;
    int iTriTempIndex;
    int iTriTempBase;
    int iTriTempDefault[10];
    int iTriTempDefault_Ini[20];
    bool bUseTriTempHeater[10];
    bool bUseTriTempHeater_Ini[10];
    bool EnableAirMachineSocket;
    double fSetTempature2AirMachine;
    double fAirMachine_DockingTemperature;
    double dSetAirstreamTemperatureRang_Socket;
    bool bEnableAtcFreonRecover;
    bool bWaitTempStableTime;                                                   //Add Wait Temperature Stable Time
    int  iWaitTempStableTime;                                                   //Add Wait Temperature Stable Time
    int iTriTempMachineHeatLowTempOffSet_Ini[3];                                //add 三溫機 控制Heater Offset
    int iTriTempMachineHeatHighTempOffSet_Ini[3];                               //add 三溫機 控制Heater Offset
    double dTri_Temp_ATCTempOffset[40];

    bool bEnableArm_1_Air;
    bool bEnableArm_2_Air;
    bool bEnableSocket_Air;

    int iAirVolumeLmt;
    double dAirStreamSocket_Offset;                                             //Ztex 2023.12.19 Add Air Stream Offset
    double dAirStreamIndex_Offset;                                              //Ztex 2023.12.19 Add Air Stream Offset
    //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    //Ztex 2023.04.26 Add HT-1032 IO Exhaust Air ==>
    bool bUseOutShuttleDesoakTime;
    int iOutShuttleDesoakTime;
    bool bEnableFastCoolFunction;
    bool bEnableTesterDryAirControl;
    //Ztex 2023.04.26 Add HT-1032 IO Exhaust Air <==

    bool    bMultiZoneEnable;                                                   //wei 20240617 Multi Zone
    bool    bZoneTempEnable[4];                                                 //wei 20240617 Multi Zone
    int     iZoneTempEnable;                                                    //wei 20240617 Multi Zone
    double  dZoneTempSetting[4];                                                //wei 20240617 Multi Zone

    bool bEnableAirMachineIndex;
    double dSetIndexAirstreamTemp;
    double SetAirstreamTemperatureRang_Index;

    bool b16SiteUsetwo8SiteSLK;
    int iDefrostTimeTooLower;                                                   //Ztex 2024.03.25 Add Defrost Time Too Lower Show Alarm
    double  fSHighBase;                                                         //Ztex 2024.07.27 Add 6 Point Temperature Offset
    int     iTempReadyRange;                                                    //Ifor 20220317 add: Init Temperature offset function
    int     iHeatDoorOverTime;                                                  //Ifor 20220317 add: Init Temperature offset function
    double  dOneCycleOverTime;                                                  //Ifor 20220317 add: Init Temperature offset function
    int     iSafeDoorOverTime;                                                  //Ifor 20220317 add: Init Temperature offset function
    int     iSafeDoorDelayTime;                                                 //Ifor 20220317 add: Init Temperature offset function
    double  dAlarmNoteOverTime;                                                 //Ifor 20230330 add:Show Alarm Note Init Temperature offset function
    double dLBTempHighSettingValue;                                             //Frank 20241231 : add
    double dLBTempLowSettingValue;
    double  dATCSecondTempOffset[32];                                           //Ifor 20241015 add ATC Second Temp Offset
    int iATC_OFS_ST;                                                            //Ifor 20241118 : 測試中變溫
    double  dATCPreOffset[32];                                                  //Ifor 20241105 add ATC 預先補償 Offset
    int     iATCPreOfsTime[32];                                                 //Ifor 20241105 add ATC 預先補償 Offset 連續時間
    double  dATCAfterOfs[32];                                                   //Ifor 20241105 add ATC 預先補償結束後 Offset

    bool    bPowerFollower_Enable;                                              //Hmy 20240207 Add Send Pf Par to Atc
    int     iPowerFollower_FullPower;                                           //Hmy 20240207 Add Send Pf Par to Atc
    double  dPowerFollower_PFSlope;                                             //Hmy 20240207 Add Send Pf Par to Atc
    double  dPowerFollower_WGain;                                               //Hmy 20240207 Add Send Pf Par to Atc
    int     iPowerFollower_Many2one;                                            //Hmy 20240207 Add Send Pf Par to Atc

    bool bEnableTempRise;                                                       //Ifor 20230418 add:Index 回溫功能
    double dTempRiseTemp;                                                       //Ifor 20230418 add:Index 回溫功能
    double dTempRiseDelay;                                                      //Ifor 20230418 add:Index 回溫功能
    AnsiString ATCTypeName;                                                     //Ifor 20230828 add:送ATC TYPE 資料給GPIB
    bool    bUseTC2Offset;                                                      //KenHsieh 20240311 : add Tc2 Offset
    bool bLBTempHighAlarm_Enable;                                               //Jimmychiu 20251215 : Add LB temp alarm switch
    bool bLBTempLowAlarm_Enable;                                                //Jimmychiu 20251215 : Add LB temp alarm switch
    bool bATC_MultiSensorEnable;                                                //Ifor 20251203 add:ATC Multi Sensor Offset
    bool bATC_MultiSensorUse[4];                                                //Ifor 20251203 add:ATC Multi Sensor Offset
    double  dATC_MultiSensorOfs[32];                                            //Ifor 20251203 add:ATC Multi Sensor Offset
    int     iOffsetByRecipeMaxLimit;                                            //StevenHong 20260119 : Add Offset By Recipe Max Limit
    int     iOffsetByRecipeMinLimit;                                            //StevenHong 20260119 : Add Offset By Recipe Max Limit
}SYSTEM_TEMPERATURE;
extern SYSTEM_TEMPERATURE Temperature;
extern SYSTEM_TEMPERATURE Temperature_NET;                                      // 2013.12.03 , Joye , KYEC FTP     //20140103 wei
//==============================================================================
typedef struct
{
    int  iTestMode;
    AnsiString sTestMode;
    int iSiteMap[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    int iShuttleMode;
    int iShuttle_Sel;
    double dSiteXPitch;
    double dSiteYPitch;
    double dSiteYOffset;                                                        //Steven 20140512 : For HT-9047
    int i1x4SiteYOffset;                                                        //Steven 20200715 : for Tinton
    double dHeadXPitch;
    int iYPitchOffsetMode;

    double dPreciserXPitch;                                                     //Frank 20180410 (Steven) : InArm Preciser Station
    double dPreciserYPitch;                                                     //Frank 20180410 (Steven) : InArm Preciser Station

    double iInitialMaxTime;                                                     //jou 2011-11-09 增加initial max time set
    double iMaxTime;
    double iDummyTime;
    double dStartDelayTime;
    int iTestType;
    int iDioMode;
    AnsiString sDioName;                                                        //Steven 20101007
    int iGpibMode;
    int iGpibAddress;
    bool bAMDFunction;
    int i2DIDFormat;                                                            //JerryYang 20200422 2DID format選項改用下拉選單
    eRs232Mode  iRs232Mode;                                                     //Steven 20121112 : RS232支援32Bin
    int         iRs232MaxBinCount;                                              //Steven 20121116 : RS232回傳的最大Bin數
    RS232_DATA Rs232_Data;
    bool bContsFailBySocket;
    unsigned int iContsFailSocketAlarmCT;
    bool bCountSpcBinContinuously_FT;                                           //Steven 20230529 : Spc Bin Couont改成連續錯誤
    bool bCountSpcBinContinuously_RT;                                           //Steven 20230529 : Spc Bin Couont改成連續錯誤
    bool bContsFailIgnore;                                                      //wei 20160115 銅鑼前幾顆不計算ContsFail
    int  iContsFailIgnore;                                                      //wei 20160115 銅鑼前幾顆不計算ContsFail
    bool bContsFailByHead;
    unsigned int iContsFailHeadAlarmCT;
    bool bFailRateMode;
    unsigned int iIgnoreIC;
    int iCountAlarmAction;                                                      //Steven 20101116
    int iCountAlarmAction_RT;

    bool bSearchLastMode;
    int  iUseSuckMode;

    int iAutoClean_Function;                                                    //Auto Cleaning start
    int iAutoClean_Mode;
    int iAutoClean_IntervalContact;
    int iAutoClean_DeveicePices;
    int iAutoClean_Tray;
    int iAutoClean_AlarmCount;
    int iAutoClean_CleanCount;
    int iAutoClean_MotorSpeed[10];
    int iAutoClean_ContactMode;
    int iAutoClean_ContactTime;
    int iAutoClean_ContactCount;
    double fAutoClean_AireForce;
    double fAutoClean_DieForce;                                                 //Steven 20240719 : Die force for auto clean
    int iAutoClean_DevicePinCount;
    double fAutoClean_DevicePinForceGf;
    int iAutoClean_ContactShiftHeight;
    int iAutoClean_ContactCleanHeight;
    int iAutoClean_Fix3PickOffset;
    int iAutoClean_Shuttle2PlaceOffset;
    int iAutoClean_Shuttle2PickOffset;
    int iAutoClean_Shuttle1PickOffset;
    int iAutoClean_Shuttle1PlaceOffset;
    int iAutoClean_Shuttle1XOffset;
    int iAutoClean_Shuttle1YOffset;
    int iAutoClean_IndexPickOffset;                                             //wei 20150318 Auto clean Index Pick Offset
    int iAutoClean_IndexReleaseOffset;                                          //Jou 2015-08-22 Auto clean Index Release Offset
    double fAutoClean_ForcePerPin;                                              //kevin 20150826 獨立 pin force
    int iAutoClean_Shuttle2XOffset;                                             //ChungHung 20141103 add AutoClean Input Arm Shuttle2 Offset
    int iAutoClean_Shuttle2YOffset;

    bool bIndexShiftHeight;
    double dAutoClean_XPitch;
    double dAutoClean_YPitch;
    double dAutoClean_XStart;
    double dAutoClean_YStart;
    int iAutoClean_XDivision;
    int iAutoClean_YDivision;
    bool bCleanIndexOtherArm;
    bool bAutoClean_UseNSKit;
    bool bPOPAutoClean;                                                         //JerryYang 20190918 Pop auto clean

    double dAutoClean_XPitch_Kit;                                               //ChungHung 20130426 KYEC 希望分兩筆資料 Kit 和 Tray
    double dAutoClean_YPitch_Kit;
    double dAutoClean_XStart_Kit;
    double dAutoClean_YStart_Kit;
    int iAutoClean_XDivision_Kit;
    int iAutoClean_YDivision_Kit;

    double dAutoClean_XPitch_Tray;
    double dAutoClean_YPitch_Tray;
    double dAutoClean_XStart_Tray;
    double dAutoClean_YStart_Tray;
    int iAutoClean_XDivision_Tray;
    int iAutoClean_YDivision_Tray;

    int iAutoClean_SelectArm;                                                   //ChungHung 20131218 add for SCK request //0:Arm1 1:Arm2 2:Arm1 & Arm2

    bool bAutoClean_FailAlarmLowYield;                                          //ChungHung 20131223 add for SCK request start
    int iAutoClean_LowYieldLimit;
    int iAutoClean_LowYieldCount;

    bool bAutoClean_FailAlarmSiteYieldDifferent;
    int iAutoClean_FailAlarmSiteYield;
    int iAutoClean_FailAlarmSiteYieldDifferentCount;

    bool bAutoClean_ConseFailureBySocket_Normal;
    int iAutoClean_ConseFailureCountBySocket_Normal;
    bool bAutoClean_ConseFailureBySocket_Retest;
    int iAutoClean_ConseFailureCountBySocket_Retest;
    bool bAutoClean_ConseFailureByHead_Normal;
    int iAutoClean_ConseFailureCountByHead_Normal;
    bool bAutoClean_ConseFailureByHead_Retest;
    int iAutoClean_ConseFailureCountByHead_Retest;

    bool bAutoClean_UseTray;                                                    //ChungHung 20130402 alter
    double dBufferKitLTX;
    double dBufferKitLDX;
    double dBufferKitRTX;
    double dBufferKitRDX;
    double dBufferKitLTY;
    double dBufferKitLDY;
    double dBufferKitRTY;
    double dBufferKitRDY;
    int iIndexArmAutoCleanCnt;                                                  //Sam 20250820 : AutoClean 在 Index Arm 下壓清潔一次就++
    bool bAntiSignal;                                                           //Eliot 2008_02_01

    bool bFailAlarmSiteYieldDifferent;                                          //jou 980716 Site Yield Alarm(%)
    int  iFailAlarmSiteYield;
    double dFailAlarmSiteYield;                                                 //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int  iFailAlarmSiteYieldDifferentCount;

    bool bLowYieldAutoSiteOff;                                                  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    bool bLowYieldAutoSiteOffByArmSite;                                         //Steven 20230223 : by arm by site, auto site off
    bool bLowYieldAutoSiteOffByContiFail;                                       //Steven 20200420 : Continue fail, auto site off
    bool bLowYieldAutoSiteOffArmContiFail;                                      //Steven 20220818 : By Arm Continue fail, auto site off
    bool bLowYieldAutoSiteOffByPicker;                                          //Steven 20230223 : 根據Index吸嘴比較良率
    int  iAlarmWhenSiteOnCountLess;
    int  iCloseSiteOnHPDontTest;
    int  iCloseSiteBin;                                                         //Steven 20240409 : 關site的位置有IC不測試送指定 bin
    bool bLowYieldAutoSiteOffAlarm;                                             //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site
    int  iLowYieldAutoSiteOffAlarm;                                             //Sam 20221207 : LowYieldAutoSiteOff 新增 Alarm 幾次後再來關 Site

    bool bErrOverLmt;
    double dErrOverLmt;
    int  iErrOverLmtIg;

    bool bFailAlarmSiteYieldCmp;                                                //jou 2014-08-14 Site Compare Low Yield alarm
    int  iFailAlarmSiteYieldCmp;                                                //jou 2014-08-14 Site Compare Low Yield alarm
    double dFailAlarmSiteYieldCmp;                                              //jou 2014-08-14 Site Compare Low Yield alarm       //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int  iFailAlarmSiteYieldCmpCount;                                           //jou 2014-08-14 Site Compare Low Yield alarm
    int  iYieldAlarmCheckIntervalByCount;                                       //AI(rf360-yield-count) 20260814 (RogerYang) : RF360每累積N顆不良品觸發一次Yield檢查(取代1min時間閘)   //AI(W906-TIF912) 20260925: golden 912 cprod.h:1822 (field was missing from the port)

    bool bFailAlarmLowYield;                                                    //jou 980716 Low Yield Alarm(%)
    int iLowYieldLimit;
    double dLowYieldLimit;                                                      //JerryYang 20160530 LowYieldLimit要能設定到小數點
    bool bSlidingWindowYield;                                                   //Hana: Sliding Window Yield  //Steven 20260331
    int  iSlidingWindowSize;                                                    //Sliding window size N        //Steven 20260331
    int iLowYieldCount;

    bool bFailAlarmLowYield_AutoClean;                                          //wei 20141201 Low Yield Auto Clean(%)
    int iLowYieldLimit_AutoClean;
    int iLowYieldCount_AutoClean;

    bool bFailAlarmLowYieldByTotal;                                             //wei 20151116 Low Yield By Total
    int iLowYieldLimitByTotal;
    double dLowYieldLimitByTotal;                                               //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iLowYieldCountByTotal;

    bool bLowYieldByPicker;                                                     //Steven 20230223 : 根據Index吸嘴比較良率
    int iLowYieldCountByPicker;
    double dLowYieldByPicker;

    bool bLowYieldAlarmByBin;                                                   //Steven 20140828 : By Bin Yield Monitor

    bool bFailAlarmLowYieldSpecial;                                             //Sam 20210505 : PTI 要求的兩段 Low Yeild
    int  iLowYieldLimitSpecial;
    double dLowYieldLimitSpecial;
    int iLowYieldCountSpecial1;
    int iLowYieldCountSpecial2;

    bool bContsFailBySocket_RT;                                                 //jou 980716 start : add RT alarm
    unsigned int iContsFailSocketAlarmCT_RT;
    bool bContsFailIgnore_RT;                                                   //wei 20160115 銅鑼前幾顆不計算ContsFail
    unsigned int  iContsFailIgnore_RT;                                          //wei 20160115 銅鑼前幾顆不計算ContsFail

    bool bContsFailByHead_RT;
    unsigned int iContsFailHeadAlarmCT_RT;

    bool bFailAlarmSiteYieldDifferent_RT;
    int  iFailAlarmSiteYield_RT;
    double dFailAlarmSiteYield_RT;                                              //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int  iFailAlarmSiteYieldDifferentCount_RT;

    bool bFailAlarmSiteYieldCmp_RT;                                             //jou 2014-08-14 Site Compare Low Yield alarm
    int  iFailAlarmSiteYieldCmp_RT;                                             //jou 2014-08-14 Site Compare Low Yield alarm
    double dFailAlarmSiteYieldCmp_RT;                                           //jou 2014-08-14 Site Compare Low Yield alarm    //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int  iFailAlarmSiteYieldCmpCount_RT;                                        //jou 2014-08-14 Site Compare Low Yield alarm

    bool bFailAlarmLowYield_RT;
    int iLowYieldLimit_RT;
    double dLowYieldLimit_RT;                                                   //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iLowYieldCount_RT;

    bool bFailAlarmLowYieldByTotal_RT;                                          //wei 20151116 Low Yield By Total
    int iLowYieldLimitByTotal_RT;
    double dLowYieldLimitByTotal_RT;                                            //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iLowYieldCountByTotal_RT;

    bool bLowYieldByPicker_RT;                                                  //Steven 20230223 : 根據Index吸嘴比較良率
    int iLowYieldCountByPicker_RT;
    double dLowYieldByPicker_RT;

    bool bFailRateMode_RT;
    unsigned int iIgnoreIC_RT;
    int  iACAlarmType;                                                          //Steven 20090821 : Alarm Count's Alarm Type    - 需要計數的Alarm 種類
    int  iACGroupMethod;                                                        //Steven 20090821 : Alarm Count's Group Method  - 計數的群組方式，採用混合、單一種類或單一AlarmCode的方式
    int  iACPeriod;                                                             //Steven 20090821 : Alarm Count's Period        - 離目前多久前的Alarm要被納入計數
    int  iACCounts;                                                             //Steven 20090821 : Alarm Count's Alarm Counts  - 發生幾次後Alarm

    bool bNS7000kit;                                                            //jou 981208 1x2 1x4 use NS7000 bias kit
    bool bNS8000CS;                                                             //Steven 20120606 : 16溫控器 8Site使用Hontech頭
    bool b6CableLayoutKit;                                                      //Steven 20230613 : ATC6.1 16Site SLK
    bool b2CableLayoutKit;                                                      //Steven 20150724 : 16溫控器 2條線版本
    bool b1CableLayoutKit;                                                      //jou 2015-10-15  : 16溫控器 1條線版本
    bool bNS7000CS;                                                             //Steven 20100119 2x2 use NS7000 Change Socket
    bool b16Direct12Shuttle;                                                    //kevin 20191104 add 16 Site Direct Heater use 12 Site shuttle
    bool b16Direct8Shuttle;                                                     //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle

    bool bRotateShuttle;                                                        //Steven 20100713 : 轉轉蝦頭
    bool bOcrFunction;                                                          //Steven 20120716 : OCR
    AnsiString sOcrText;                                                        //Steven 20120716 : OCR
    int  iRotateADC;                                                            //Steven 20101018 : 轉轉蝦頭加減速
    int  iInitSpeed;                                                            //kevin 20110531 旋轉SHUTTLE 鎖最高速度
    int  iPJogHighSpeed;                                                        //kevin 20110531 旋轉SHUTTLE 鎖最高速度
    bool bContinuousPass;                                                       //Eliot 20100708
    bool bContinuousPass_RT;                                                    //Eliot 20100708
    unsigned int iContinuousPassBin;                                            //Eliot 20100708
    unsigned int iContinuousPassBin_RT;                                         //Eliot 20100708
    unsigned int iContinuousPassBinCount;                                       //Eliot 20100708
    unsigned int iContinuousPassBinCount_RT;                                    //Eliot 20100708

    bool         bContinuousLoader;                                             //jou 2010-11-17
    bool         bContinuousLoader_RT;                                          //jou 2010-11-17
    unsigned int iContinuousLoaderCount;                                        //jou 2010-11-17
    unsigned int iContinuousLoaderCount_RT;                                     //jou 2010-11-17

    bool         bContinuousContact;                                            //Steven 20110420
    bool         bContinuousContact_RT;                                         //Steven 20110420
    unsigned int iContinuousContactCount;                                       //Steven 20110420
    unsigned int iContinuousContactCount_RT;                                    //Steven 20110420

    bool         bContinuousPassBySocket;                                       //Steven 20110915
    unsigned int iContinuousPassBinCountBySocket;                               //Steven 20110915

    bool         bContinuousPassBySocket_RT;                                    //Steven 20110915
    unsigned int iContinuousPassBinCountBySocket_RT;                            //Steven 20110915
    int iQAModeCount;                                                           //Steven 20111026 : QA模式的數量
    int iQAModeRunType;                                                         //Steven 20120612 : QA做完後的動作
    int iQAModeBin;                                                             //Steven 20141023 : QA做完後的Bin
    int iQATrayDirect;                                                          //jou 2014-10-24 QA mode 增加支援吸Tray方向
    bool bQAModeAfterTrayEnd;                                                   //Steven 20151125 : QA做完後的TrayEnd要重做QA
    bool bEnableQASampling;                                                     //Steven 20190326 : QA Sampling
    int  iQASamplingBin;
    int  iQASamplingT3Pos;
    int  iQASamplingCnt;
    AnsiString sQASamplingTray;
    bool bQAD22DoubleContact;                                                   //Sam 20231117 : 整合到 QA 模式
    int  iQAD22DoubleContactCount;                                              //Sam 20240104 : 新增 QA 不計數模式
    bool bQATrayEndCloseYield100Site;                                           //Sam 20231117 : 整合到 QA 模式
    double ForcePerPinN;                                                        //kevin 20120522 cleanpad 腳數
    int iPadThickness;                                                          //kevin 20120623 Clean pad 厚度
    int iAutoClean_ArmTrayPickOffset;                                           //kevin 20120623 Arm tray 吸取OFFSET
    int iAutoClean_ArmTrayPlaceOffset;                                          //kevin 20120623 Arm tray 放OFFSET
    int iAutoClean_ArmShuttlePickOffset;                                        //kevin 20120623 Arm SHUTTLE 吸取OFFSET
    int iAutoClean_ArmShuttlePlaceOffset;                                       //kevin 20120623 Arm SHUTTLE 放OFFSET

    int          iARM_Y_PITCH;                                                  //ChungHung 20120505 HT9045WS
    int          iARM_HP_Y_PITCH;                                               //Steven 20180122 : For Y-Pitch at Hot Plate
    bool         bEnableBarCode; bool bEnableBarcodeCSVCompare;                 //Steven 20121009 : Bar Code   //AI(W906-TIF912-R10) 20260926: 同行補 golden 912 cprod.h:1951 bEnableBarcodeCSVCompare（Ifor 20260511 Barcode CSV Compare；RULINGS_20260926 第 10 條）；位置與 912 相同，不插行
    bool         bEnableBottom2D;                                               //Steven 20190308 : Bottom 2D
    double       dBottom2DOffsetX;
    double       dBottom2DOffsetY;
    bool         bEnableMulti2D;                                                //Steven 20200810 : 一個IC使用多個2DID
    int          iMulti2DXItem;
    int          iMulti2DYItem;
    double       dMulti2DXPitch;
    double       dMulti2DSH1Ofs_L;
    double       dMulti2DSH1Ofs_R;
    double       dMulti2DSH2Ofs_L;
    double       dMulti2DSH2Ofs_R;
    int          iMulti2DType;
    int          iMulti2DCount;
    int          iMulti2DMap[2][2];
    int          iBarCodeDelay;                                                 //Steven 20121009 : Bar Code
    int          iBarCodePosDelay;                                              //wei 20151126
    int          iBarCodePos1Delay;
    int          iBarCodeMinLength;
    int          iBarCodeMaxLength;
    int          iCheckSumLength;                                               //KaiChen 20191121 ：中壢日月光 2D Check Sum
    bool         bCheckSum;                                                     //KaiChen 20191121 ：中壢日月光 2D Check Sum
    int          iNoCodeDeviceToErr;                                            //Steven 20151221 : 將讀取異常的IC放到Error Bin
    bool         bCheckEnable2DIDFunction;                                      //KaiHuang 20201028 : ASE-CL 生產時檢查是否開啟 2D
    bool         bCheckEnableBottom2DIDFunction;                                //KaiHuang 20201028 : ASE-CL 生產時檢查是否開啟 Bottom 2D
    bool         bNoCodeDeviceAutoSkip;                                         //Steven 20151221 : 讀不到Code時,自動Skip跳下一顆
    int          iBarcodeRetryCount;                                            //Steven 20151221 : 讀不到Code時,自動Retry的次數
    bool         b2DTriggerMode;                                                //Steven 20151225 : 改用拍完就跑的方式
    int          i2DTriggerTime;                                                //Steven 20151225 : 拍照的等待時間
    bool         bCheckCodeByShuttle;                                           //Steven 20160428 : 檢查2D重複碼
    int          iShtDuplicateRetryCnt;                                         //Steven 20160823 : 蝦頭重複碼要可以自動Retry
    bool         bCheckCodeByLot;
    bool         bCheckLotHaveCode;                                             //wei 20160728
    bool         bEnableConsecutiveFailure;                                     //wei 20160823  Consecutive Failure
    bool         b2DUseUndefinedCMD;                                            //Ifor 20151226 :改用 自行定義 Trigger Command
    AnsiString   str2DTriggerONCMD;                                             //Ifor 20151226 :自行定義 Trigger ON Command
    AnsiString   str2DTriggerOFFCMD;                                            //Ifor 20151226 :自行定義 Trigger OFF Command
    int          iConsecutiveFailure;                                           //wei 20160823  Consecutive Failure

    bool         bLotIDVerify;                                                  //Steven 20240704 : Lot Verification function for ATK
    int          iLotIDVerifyS;
    int          iLotIDVerifyE;
    int          i2DIDStrStart;
    int          i2DIDStrEnd;
    int          iLotIDLength;
    AnsiString   sLotIDVerify;
    AnsiString   sLotIDSubstr;

    double dInitStartDelayTime;                                                 //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    int    iInitStartDelayTimeCT;                                               //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time

    int iBinAlarmByTotalOrBySite;                                               //jou 2013-04-25 BIN Alarm By Total or By Site;
    int iBinAlarmByTotalOrBySite_RT;                                            //jou 2013-04-25 BIN Alarm By Total or By Site;

    double dInitWaitTime;                                                       //jou 2013-09-25 Testing Need Stop All Motor
    double dTestingWaitTime;                                                    //jou 2013-09-25 Testing Need Stop All Motor
    bool bRetryOffsetMove;                                                      //wei 20161116 Retry時先退出再進去讀取
    double dRetryOffsetMove;                                                    //wei 20161116 Retry時先退出再進去讀取

    bool bRetryShiftOffsetMove;                                                 //wei 20161116 Retry時先退出再進去讀取(前中後)
    double dRetryShiftOffsetMove;                                               //wei 20161116 Retry時先退出再進去讀取(前中後)

    bool b2DIDYield;                                                            //Steven 20171222 (Wei) : Yield Alarm of 2DID
    bool bSetCloseSite2DIDtoEmpty;                                              //Steven 20190313 : Close site 2DID set to empty
    double d2DIDYield;                                                          //Steven 20171222 (Wei) : Yield Alarm of 2DID
    int  i2DYieldIgnoreCnt;                                                     //JerryYang 20241104 : Ignore count變更為可以修改
    int  iEnableAllSite2DIDErr;                                                 //Steven 20200702 : All site 2DID fail改成可以開關

    bool bSearch2DIDByLot;                                                      //Frank 20170316 (wei) add Search 2DID By Lot
    int  b2DIDListErrorBin;                                                     //Steven 20190604 : 2DID不在List內的另外分bin
    int  b2DIDNotExist2Error;                                                   //JerryYang 20231218 : 2DID黑名單功能
    bool bEnableMRTMode;                                                        //Ifor 20170405 (wei) add MRT Use Mode By Setup File
    bool bSaveFailImage;                                                        //Frank 20170425 (Steven) add
    int  b2DIDStringFormat;                                                     //RogerYang 20181222 新增String format選項
    bool bUseHandShakeCommunication;                                            //Ifor 20190225 :add Bar Code Use HandShake Communication
    int  i2DHandShakeTimeOut;                                                   //Ifor 20190225 :add Bar Code Use HandShake Communication
    int iSelectUseCCDSh1;                                                       //kevin 20210814 add Frank 20171011 add Shuttle Check 2DID Pos
    int iSelectUseCCDSh2;                                                       //kevin 20210814 add Frank 20171011 add Shuttle Check 2DID Pos
    bool b2DUseSubJob;                                                          //Ifor 20200807 add:In House 2D Use Sub Job Function
    bool b2DUseAnyChar;                                                         //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷
    bool bBarcodeOnlyRetry;                                                     //Ifor 20210120 add:Barcode Error Only Retry
    bool b2DUsePinInspection;                                                   //Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
    bool bSortingBy2DIDList;                                                    //JerryYang 20190313 : 2D sorting
    int  iActionOf2DNotInList;                                                  //Steven 20250707 : Action Of 2D Not In List
    bool bIndexDropICSetErrUntilOneCycle;                                       //JerryYang 20220923 : index arm drop error設ERROR BIN
    bool bOutShtLoseICSetErrUntilOneCycle;
    bool bCheckCodeByServer2DID;                                                //Jimmychiu 20230925 : read 2did in json file
    AnsiString asMes2DID_URL;                                                   //Jimmychiu 20230925 : read 2did in json file
    bool bChkMakeWhite2DIDList;                                                 //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能

    bool bUseBarcodeAutoAdjustLight;                                            //Ifor 20210408 add:Barcode 自動調整光源
    int iAutoAdjustLightTimeOut;                                                //Ifor 20210408 add:Barcode 自動調整光源
    int i2DIDYield;                                                             //Ifor 20200825 add: Yield Alarm of 2DID
    bool b2DIDAllowList;                                                        //JerryYang 20241104 : 支援2DID白名單功能

    bool bBarCodeInspReport;                                                    //Sam 20240426 : Add BarCoder Inspection Report
    bool bMostCheckLotID;                                                       //Frank QQ
    int iMostCheckLotID;                                                        //Frank QQ

    AnsiString asPrefixFileName;                                                //Frank QQ
    bool bPrefixBarcode;                                                        //Frank QQ
    bool bPrefixBarcodeCheck;                                                   //Frank QQ

    bool bNeedOpenNITrace;
    bool bNeedCloseNITrace;

    AnsiString asFTPUserName;
    AnsiString asFTPPassword;
    AnsiString asFTPHost;
    AnsiString asFTPDownloadPath;
    AnsiString asFTPDownloadFilePath;
//==============================================================================
// GEM function Start
//==============================================================================
    AnsiString asGEM_SiteMapping;
    AnsiString asGEM_DutOnOff;
    bool bOctal_80Kit;                                                          //2013-03-05    Dell    for ATK pitch 80mm kit
    bool bOctal_16Kit;                                                          //Steven 20140312 : 8Site使用16Site Kit
    bool bSquare_OctalKit;                                                      //Steven 20141224 : 2x2Site使用8Site Kit
    bool b2x2Use16SiteKit;                                                      //Steven 20191113 : 2x2Site使用16Site Kit
    bool b1x2Use1x4SiteKit;                                                     //Isaac  20200311 : 1x2Site使用1x4Site Kit
    bool bUse1x3SiteKit;                                                        //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
    bool b2x6Use2x8SitSLK;                                                      //Steven 20240807 : 12Site使用16Site Kit

    bool bEnSocketSensor;                                                       //kevin 20130504  使用SOCKET SENSOR
    bool bSocketDisibleinitialcheck;                                            //kevin 20170209 (wei) index check不偵測 check
    bool bSocketSensorCheckFloating;                                            //Steven 20180313 : 使用Socket Sensor驗證置偏
    int  iSocketCount;                                                          //kevin 20130504 使用多少顆SENSOR 最多8個
    int  iSensorCheckType[24];                                                  //Steven 20200420 : Socket Sensor功能可以選
    bool bCheckSocketFloating;                                                  //Steven 20200420 : Socket Sensor功能可以選
//------------------------------------------------------------------------------
    bool bForEgisTecTest;                                                       //Steven 20140922 : Arm2當作指紋測試
    bool bEnableShuttleLaser;                                                   //Steven 20140228 : 雷射測距功能
    bool bEnableInArmLaser;                                                     //Steven 20140228 : 雷射測距功能
    double dIcThickness;                                                        //Steven 20140228 : 雷射測距功能 IC厚度
    double dLaserThresholdSht;                                                  //Steven 20140228 : 雷射測距功能 偵測門檻值
    double dLaserThresholdHP;                                                   //Steven 20140228 : 雷射測距功能 偵測門檻值

    bool bEnableInShuttleFloatingCheck;
    bool bEnableOutShuttleFloatingCheck;

    double dInLaserThresholdSht;
    double dOutLaserThresholdSht;

    int iDeviceXOffset;
    int iDeviceYOffset;
    int iOutDeviceXOffset;
    int iOutDeviceYOffset;

    double dLaserValueOutSh[2][MAX_Index_Row][MAX_Index_Col];                   //Eastsun 20260522 整合 Out Shuttle Golden 數值
    double dLaserValueOut[2][50][50];                                           //Eastsun 20260522 整合 Out Shuttle Golden 數值

    double dLaserValue[2][MAX_Index_Row][MAX_Index_Col];                        //Steven 20140228 : 雷射測距功能 Golden數值
    double dLaserValueIn[2][50][50];                                            //Steven 20140228 : 雷射測距功能 Golden數值

    bool bEveryFirstDeviceUseInitialDelay;                                      //ChungHung 20140425 add for TSMC Device
    bool bUseOtherArmToTestAfterInitialDelay;                                   //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
    bool bAfterShowAlarmMessageUseInitialDelay;
    bool bWhenHappenTestedTimeBelowUseInitialDelay;
    double iEveryFirstDeviceUseInitialDelay;

    double iInitialDelay;
    double iInitialDelay_2;                                                     //ChungHung 20141210 add for SCK want to every event have delay
    double iInitialDelay_3;                                                     //ChungHung 20141210 add for SCK want to every event have delay
    double iInitialDelay_4;                                                     //ChungHung 20141210 add for SCK want to every event have delay
    double iInitialDelay_5;                                                     //ChungHung 20141210 add for SCK want to every event have delay
    double iInitialDelay_6;                                                     //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    double iInitialDelay_7;                                                     //wei 20151228 No FullSite delay
    double iInitialDelay_8;                                                     //kevin 20160310 測試完成到下一次完成超過所設時間須啟動 delay 時間
    double iInitialDelay_9;                                                     //Steven 20160818 : OTD打開Delay
    double dInitialDelay_10;                                                    //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41 wait time
    double iTestFinishToNextTestOver;                                           //kevin 20160310 測試完成到下一次完成超過所設時間

    double dInitialDelay_1_RT;                                                  //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    double dInitialDelay_2_RT;
    double dInitialDelay_3_RT;
    double dInitialDelay_4_RT;
    double dInitialDelay_5_RT;
    double dInitialDelay_6_RT;
    double dInitialDelay_7_RT;
    double dInitialDelay_8_RT;
    double dInitialDelay_9_RT;
    double dInitialDelay_10_RT;

    bool   bOTDUnlockDelay;                                                     //Steven 20160818 : OTD打開Delay
    bool   bWhenPressStopOverUseInitialDelay;                                   //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    double iWhenPressStopOver;                                                  //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    bool bWhenNoFullSiteUseInitialDelay;                                        //wei 20151228 No FullSite delay
    bool bTestFinishToNextTestOver;                                             //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    bool bAfterAutoCleanFunctionUseInitialDelay;                                //ChungHung 20141017 add for SCK Add for TSMC Device
    bool bAfterOpenHeatDoorUseInitialDelay;                                     //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    bool bOctal_12Kit;                                                          //ChungHung 20140508 add for SCK

    double dAfterTestedDelay;                                                   //ChungHung 20140730 add for ATK function after tested delay time
    bool bUse32Heater;                                                          //Steven 20140923 : Index使用EJ1N版32組加熱器
    bool b12SiteUse10Heater;                                                    //Steven 20140923 : Index使用EJ1N版32組加熱器
    bool bArm1PickPlaceArm2Test;                                                //kevin 20150127 Arm1 下壓 arm2 測試
    bool bArm1PickPlaceArm2Test_RunAutoClean;                                   //Jimmychiu 20230710 : Auto Clean 跑 Arm1 下壓 arm2 測試
    bool bCheckArm2Vacuum;                                                      //Steven 20150129 : 需要確認Arm2有沒有粘料
    double HotplatlXOffset;                                                     //kevin 20150209 add HOTPLATE OFFSET
    double HotplatlYOffset;                                                     //kevin 20150209 add HOTPLATE OFFSET
    double HotplatlPickOffset;                                                  //kevin 20150209 add HOTPLATE OFFSET
    double HotplatlPlaceOffset;                                                 //kevin 20150209 add HOTPLATE OFFSET
    AnsiString cAutoClean_PackageTray;                                          //kevin 20150521 Autoclean tray Package name
    double HotplatlPitchOffset;                                                 //kevin 20150526 add
    double ShuttlePitchOffset;                                                  //kevin 20150526 add

    bool    bEnablePassYieldART;                                                //kevin 20150703 啟用pass yield功能
    double  fPassYieldART;                                                      //kevin 20150703 啟用 pass yield  94.6%
    bool    bEnableOpenShortART;                                                //kevin 20150703 啟用Open/short  yield功能
    double  fOpenShortYieldART;                                                 //kevin 20150703 使用 open short yield 94.6%
    bool    bEnableRecoverART;                                                  //kevin 20150703 使用 rt1 當次比對
    double  fRecoverYieldART;                                                   //kevin 20150703 使用 rt1 當次比對

    bool    bPass             [TEST_MAX_BIN];                                   //kevin 20150703 使用各bin pass yield
    bool    bOpenShort        [TEST_MAX_BIN];                                   //kevin 20150703 使用各bin open short

    bool    bHontechLayoutKit2x2;                                               //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
    bool    bQualSite2X2Shift;                                                  //wei 20160226 TSMC X Shift
    double  dShiftXPitch;                                                       //wei 20160226 TSMC X Shift
    double  dShiftYPitch;                                                       //wei 20160226 TSMC X Shift

    bool    bAlarm4EnableIntervalYield;                                         //20150604 Mylin Interval Total Yield Difference {
    int     iAlarm4IntervalYieldIntervalCount;
    int     iAlarm4IntervalYieldContinueCount;
    int     iAlarm4IntervalYieldYield;
    bool    bAlarm4ContinueType_Enable;                                         // 2015.05.05 , Joye , Alarm4 Continue Type
    int     iAlarm4ContinueType_IntervalCount;                                  // 2015.05.05 , Joye , Alarm4 Continue Type
    int     iAlarm4ContinueType_ContinueCount;                                  // 2015.05.05 , Joye , Alarm4 Continue Type

    bool bSiteToSiteYieldCmp;
    int  iSiteToSiteYieldCmp;
    int  iSiteToSiteYieldCmpCount;

    bool bHeadToHeadYieldCmp;
    int  iHeadToHeadYieldCmp;
    int  iHeadToHeadYieldCmpCount;

    bool bSiteYieldOverAlert;
    int  iSiteYieldOverAlert;
    int  iSiteYieldOverAlertCount;
    bool bUseSLKClamp;                                                          //JerryYang 20160523
    int  iSeparabilityTest;                                                     //JerryYang 20160523 分離測試

    bool bEnableUsePreciser;                                                    //Frank 20180410 (Steven) : InArm Preciser Station
    bool bEnableRTPreciser;                                                     //Frank 20180410 (Steven) : InArm Preciser Station
    bool bEnablePlaceICInPreciser;                                              //Frank 20180410 (Steven) : InArm Preciser Station
    bool bEnablePreciserHotPlate;                                               //Kevincheng 20260525 : 渠梁常溫模式使用hotplate

    bool bEnableDeviceFlipper;

    bool bFinePitch;
    bool bFPDoubleCheck;
    bool bFPCheckAfterContact;
    bool bEnableFPContactCCD;
    bool bEnableCalCCD;

    int    iAlarm5_BySiteIntervalContactCnt;                                    //Sam 20171213 (Steven) : 超豐良率監控
    bool   bAlarm5_BySiteLowYieldEnable;                                        //Sam 20171213 (Steven) : 超豐良率監控
    double dAlarm5_BySiteLowYield;                                              //Sam 20171213 (Steven) : 超豐良率監控
    double dAlarm5_BySiteLowYieldRej;                                           //Sam 20180423 (wei) : MOFile of Yeild Download
    bool   bAlarm5_BySiteCmpYieldEnable;                                        //Sam 20171213 (Steven) : 超豐良率監控
    double dAlarm5_BySiteCmpYield;                                              //Sam 20171213 (Steven) : 超豐良率監控
    double dAlarm5_BySiteCmpYieldRej;                                           //Sam 20180423 (wei) : MOFile of Yeild Download
    bool   bAlarm5_BySiteAlarmYieldEnable;                                      //Sam 20171213 (Steven) : 超豐良率監控
    double dAlarm5_BySiteAlarmYield;                                            //Sam 20171213 (Steven) : 超豐良率監控
    double dAlarm5_BySiteAlarmYieldRej;                                         //Sam 20180423 (wei) : MOFile of Yeild Download
    int    iAlarm5_OSBin;                                                       //Sam 20171213 (Steven) : 超豐良率監控
    bool   bAlarm5_BySitePreCmpYieldEnable;                                     //Sam 20180423 (wei) : MOFile of Yeild Download
    double dAlarm5_BySitePreCmpYield;                                           //Sam 20180423 (wei) : MOFile of Yeild Download
    double dAlarm5_BySitePreCmpYieldRej;                                        //Sam 20180423 (wei) : MOFile of Yeild Download

    bool bEnableShtFloatChk;                                                    //Steven 20160920 : IC置偏檢查
    int  iSFCStartDelay;
    int  iSFCExposureTimeOut;
    int  iSFCGetResultTimeOut;
    int  iSFCAutoRetry;
    bool bSFCUse2Photo;
    int  iSFCUse2PhotoOffset;
    bool bSingleHeater;                                                         //JerryYang 20161013 新增Single site一支加熱棒模式
    bool bF18InshuttleDetect;                                                   //jou 20170418 : 矽品-世明要求修改config F18至Recipe設定
    bool bRTC20CheckFunction;                                                   //Ifor 20190712 : add RTC2.0 Check
    bool bRTC20GiveWayCheck;                                                    //Ifor 20190712 : add RTC2.0 Check
    bool bRTCICResidueCheck;                                                    //Ifor 20190712 : add RTC2.0 Check
    bool bRTC20OverFlowCheck;                                                   //Ifor 20190712 : add RTC2.0 Check
    bool bArm1UseHeat;                                                          //Ifor 20190712 : add RTC2.0 Check
    bool bArm1OnlyPlaceArm2TestAndSuck;                                         //Jimmychiu 20250722 : Arm1 Only Place Arm2 Test And Suck

    double  dSCKART_Yield;                                                      //Steven 20161123 : For SCK ART - 最低Yield
    int     iSCKART_TryCnt;                                                     //Steven 20161123 : For SCK ART - 最多執行次數
    bool    bSCKART_AutoSkip;                                                   //Steven 20170315 (wei) : For SCK ART Auto Skip
    int     iSCKART_AutoSkipCount;                                              //Steven 20170315 (wei) : For SCK ART Auto Skip
    int     iSCKART_MinAutoCloseSite;                                           //JerryYang 20220923 : 最小關site數量
    bool    bAlarmAfterSendSRQKIND2;                                            //Steven 20230119 : ATK 要在SRQKIND2之後Alarm
    bool    bSCKART_AutoSocketOff;
    int     iSCKART_AutoSocketOffMode;
    double  dSCKART_AutoSocketOffYield;
    int     iSCKART_SortMode;
    int     iSCKART_RTStartMode;
    bool    bSCKART_AutoCleanOut;
    bool    bSCKART_AutoLotEnd;
    bool    bSCKART_FinalLotEndMsg;
    bool    bSCKART_LotDeviceCheck;
    bool    bSCKART_EnableART;                                                  //Steven 20170919 (wei) : Add for SCK ART
    AnsiString sSCKART_VersionCmd;
    int iSCKLdCntLimN;                                                          //RogerYang 20251224 : Rf360需求 lotcheck加入上下限設定
    int iSCKLdCntLimP;                                                          //RogerYang 20251224 : Rf360需求 lotcheck加入上下限設定

    bool    bSCKART_EnableSPBinAlarm;                                           //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm
    int     iSCKART_SPBinSelect;                                                //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm
    double  dSCKART_SPBinAlarmYield;                                            //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm
    bool    bSCKART_RunARTWithoutCmd;                                           //Steven 20180830 : Run ART without tester command

    bool    bLowYieldForArtFT;                                                  //Steven 20220208 : Add for ATK, ART low Yield
    double  dLowYieldForArtFT;

    bool   bOutPickErrAct;                                                      //Steven 20170202 (wei): For ATJ Out Arm pickup error put to define tray
    int    iOutPickErrAuto;                                                     //Steven 20170202 (wei): For ATJ Out Arm pickup error put to define tray

    bool bEnableTrayMap;                                                        //wei 20161219 Tray Mapping
    bool bEnableTrayID;
    int  iTrayStartDelay;
    int  iTrayExposureTimeOut;
    int  iTrayGetResultTimeOut;
    int  iTrayAutoRetry;
    int  iTrayCodeMinLength;
    int  iTrayCodeMaxLength;
    int  iTrayMapCatch;
    int  iTrayMapShift;
    bool bDisableMapSuck;
    bool bEnableSuckMapCheck;
    bool bEnableTrayIDDownFTP;                                                  //wei 20170518 Tray Mapping
    bool bEnableOCRTrayIDDown;
    bool bCheckTrayIDBylot;                                                     //JerryYang 20250120 : add

    int  iTrayIDShift;                                                          //wei 20170608
    bool bEnabAutoCloseSite;                                                    //wei 20170801
    int  iLastOpenSite;                                                         //wei 20170801
    int  iTrayIDReadShift;

    bool bEnableAutoAlign;                                                      //Sam 20181201 : AutoAlignment
    int  iAlignOffsetAlarm;

    bool bEnableRFID;                                                           //wei 20180808 MR RFID
    int  iRFIDDelay;                                                            //wei 20180808 MR RFID
    int  iRFIDRetryCount;                                                       //wei 20180808 MR RFID
    bool bEnableDeviceRemain;                                                   //wei 20170317 (steven) Device Remain 殘料檢測
    bool bEnableTrayID2;                                                        //wei 20180808 MR Tray ID 2
    int  iTrayID2Shift;                                                         //wei 20180808 MR Tray ID 2
    bool bEnableTrayDeviceCnt;                                                  //Sam 20190405 : Decive Count
    int  iE84TimeOut[6];                                                        //wei 20190102 E84
    bool bStandardValue;                                                        //wei 20190111
    bool bStandardValueMedian;                                                  //wei 20190111
    int  iStandardValue;                                                        //wei 20190111
    int  iStandardValueMedian;                                                  //wei 20190111
    bool bEnableView;
    int  iTrayMapStart;
    int  iTrayDeciveCntStart;                                                   //Sam 20190405 : Decive Count
    bool bEnableTrayMapDoubleCheck;
    int  iDownMapDelayRead;
    int  iMagTraySource;                                                        //Sam 20221116 : Magazine TrayArm 自動補 Tray
    int  iMagFixTrayType;

    bool    bEnableFix2BGAAICCD;                                                //RogerYang 20180901 add 矽格湖口Demo AI CCD Function
    bool    bEnableLearningMode;
    int     iFix2BGAAICCDStartDelay;
    int     iFix2BGAAICCDExposureTimeOut;
    int     iFix2BGAAICCDGetResultTimeOut;
    int     iFix2BGAAICCDAutoRetry;
    int     iFix2BGAAICCDOutArmCycleInsp;
    double  dInspectResultThres;
    int     iBGALightScrPos;

    bool bDoNotWaitForAOIResults;                                               //Sam 20210609 : Fix AOI 軟體升級
    int iResultShowType;                                                        //Sam 20240325 : 新增 DamageTrayMapping 功能

    bool bInArmUseBackRowSuck;                                                  //jou 20161122 (Steven) In & Out arm use back row suck
    bool bOutArmUseBackRowSuck;                                                 //jou 20161122 (Steven) In & Out arm use back row suck
    bool bSpiroxTesterLotEnd;                                                   //JerryYang 20170706 JCET通知測試機lot end功能改為by工作檔
    bool bEnableUseXCenterPitch;                                                //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
    double dSiteXCenterPitch;                                                   //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
    bool bNSKitPress;                                                           //kevin 20170804 (Steven) add 另一LOAD CELL 壓力值
    bool bAllSiteFail;                                                          //kevin 20170825 (Steven) 整支ARM Fail bin
    bool bAllSiteFail_RT;                                                       //Isaac 20180305 (Steven) ATK要求，只有FT要alarm，FT/RT分開，TEST_IF.bAllSiteFail_RT
    bool bUseSocketFloat;
    bool bIndEPSLK;

    bool   bSpecBinBySiteCompareEnable[7][TEST_MAX_BIN];                        //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    double dSpecBinBySiteComparePercent[7][TEST_MAX_BIN];                       //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    int    iSpecBinBySiteCompareIgnore[7];                                      //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    bool   bSpecBinByArmPerSiteCompareEnable[7][TEST_MAX_BIN];                  //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    double dSpecBinByArmPerSiteComparePercent[7][TEST_MAX_BIN];                 //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    int    iSpecBinByArmPerSiteCompareIgnore[7];                                //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    bool   bByBinFailureEnable[7][TEST_MAX_BIN];                                //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    double dByBinFailurePercent[7][TEST_MAX_BIN];                               //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    int    iByBinFailureIgnore[7];                                              //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    bool bFailCountEnable[7][16];
    int  iFailCountLimit[7][16];
    int  iFailCountIgnore[7];
    int    iEnStartDelayCount;                                                  //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay
    double dInitialStartDelayDec[4];                                            //kevin 20180307 每次減多少秒 延遲遞增減 開始執行送訊號delay時間
    int    iStartDelayCount[5];                                                 //kevin 20180307  使用幾次 延遲遞增減 開始執行送訊號delay時間 次數

    bool bFailAlarmIntervalLowYieldBySite;                                      //wei 20180606 Interval Low Yield By Site
    int iIntervalLowYieldLimitBySite;
    double dIntervalLowYieldLimitBySite;                                        //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iIntervalLowYieldCountBySite;

    bool bFailAlarmIntervalLowYieldBySite_RT;                                   //wei 20180606 Interval Low Yield By Site
    int iIntervalLowYieldLimitBySite_RT;
    double dIntervalLowYieldLimitBySite_RT;                                     //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iIntervalLowYieldCountBySite_RT;

    bool bFailAlarmIntervalLowYieldByTotal;                                     //wei 20180718 Interval Low Yield By Total
    int iIntervalLowYieldLimitByTotal;
    double dIntervalLowYieldLimitByTotal;                                       //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iIntervalLowYieldCountByTotal;

    bool bFailAlarmIntervalLowYieldByTotal_RT;                                  //wei 20180718 Interval Low Yield By Total
    int iIntervalLowYieldLimitByTotal_RT;
    double dIntervalLowYieldLimitByTotal_RT;                                    //JerryYang 20160530 LowYieldLimit要能設定到小數點
    int iIntervalLowYieldCountByTotal_RT;

    int iAutoCleanDropHigh;                                                     //kevin 20180717 autoClean drop high
    int iAllSiteFailCount;                                                      //kevin 20180720 all site fail count
    int iAllSiteFailCountRT;                                                    //Steven 20230118 : All site fail RT
    bool bPurgeAirAfterContract;                                                //kevin 20180928 Contrarct time blower air loadboard
    int  iPurgeAirContract;                                                     //kevin 20180928 Contrarct time blower air loadboard
    bool bTestStartToNextTestStart;                                             //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41
    double dTeststartToNextTestStart;                                           //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41  over time
    bool bLoadCellMeasure;                                                      //kevin 20190907 Arm 測區次數道量測 功能;                                 //kevin 20190907 Arm 測區次數道量測
    int iLoadCellCount;                                                         //kevin 20190907 Arm 測區次數道量測
    int iContactAlarmCount[4];                                                  //Ifor 20190920 : add Contact Alarm Count
    AnsiString sKitName[4];
    int iAutoCloseSiteWhenRT;                                                   //Steven 20200225 : 切到RT的時候,要關閉Socket
    double dAutoCloseSiteYieldWhenRT;                                           //Steven 20200225 : 切到RT的時候,要關閉Socket
    int iAllSiteOnAtInitialStart;                                               //Steven 20230814 : Initial Start的時候要全開Site
    int iAutoSiteOffByGPIB;                                                     //JimmyChiu 20250715 : Auto site on/off by GPIB
    int testBin[32];                                                            //Sam 20190429 : Add CC_PTI_NEWWORK
    bool bUseTesterDry;                                                         //Ifor 20200115 : add Tester Dry Air Control
//    int iStepMotorSpeed[10];                                                  //Steven 20200529 : Loader入Tray改步進  //Steven 20200701 : 取消Tray Motor第二段速度的設定
    int iTrayZStepSpeed[12];                                                    //Steven 20200724 : 修改Tray Z馬達速度設定
    int iShakeShuttleWhenPlaceIC;                                               //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭
    bool bAutoSiteMappingOpenSite;                                              //jou 20200701 : VTEST for auto site mapping cable mount
    bool bAutoSiteMappingOneCycle;                                              //jou 20200701 : VTEST for auto site mapping cable mount
    int iOpenBin;                                                               //jou 20200928 : Auto Site Mapping Set Open BIN
    int iASMPassBin;                                                            //jou 20230221 : Auto Site Mapping Set Pass BIN

    AnsiString sRtcFileName;                                                    //kevin 20200731 RTC FILENAME
    int  iTestRule;
    int  iFPLoopCount;
    int  iFPCalCCDSpeed;
    bool iFPAutoOffset;
    int  iFPErrorCount;
    bool bEnableReadAndCheckTorque;                                             //kevin 20210804 config set up 扭力模式記錄比對
    double dReadTorque;
    double dReadTorqueDelayTime;
    AnsiString s2DFileName;                                                     //kevin 20210817 2D FILENAME

    int  i2DReadMultiLine;                                                      //Ifor 20210407 add: 自製OCR
    int  i2D1stLineLength;
    int  i2D2ndLineLength;
    AnsiString  as2DInsertString;

    AnsiString sLoaderColorSenFTEnable;                                         //Jimmychiu 20230630 : add color sensor MU-N in Loader
    AnsiString sLoaderColorSenRTEnable;                                         //Jimmychiu 20230630 : add color sensor MU-N in Loader

    bool UseRotateForHT7000HPKit;                                               //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
    bool bAutoOnecycleHomStart;                                                 //Isaac 20210821 : 全智要求定時onecycle回home,start
    int iAutoOnecycleHomStartTime;                                              //Isaac 20210821 : 全智要求定時onecycle回home,start
    AnsiString asTester_Address;                                                //wei 20211027 open short TCP/IP
    int iTester_Port;                                                           //wei 20211027 open short TCP/IP
    double dSendGetValueDelayTime;                                              //KenHsieh 20220317 : 延遲取值時間
    int iUnloaderTrayCount[3];                                                  //Frank 20220322 Add

    bool bUseLoadCCDTrayMap;                                                    //kevin 20220330 add Load 載入CCD 判斷有無IC 存在
    bool bUseAutoAddEmpty;                                                      //kevin 20220330 add Auto 123 補上蓋空盤
    bool bUseAutoTrayMap;                                                       //kevin 20220401 add Auto 123 Tray Map
    bool bDoubleUnloadTray;                                                     //kevin 20220506 add Unload Tray 2  倍的設定
    bool bTTLUseASEJPMode;                                                      //Frank 20220408 Add TTL ASE_JP Mode

    bool bSingleUseOtherSuck;                                                   //wei 20220905 Single Use Other Suck(IN C OUT E)
    bool bSingleUseDSuck;                                                       //Frank 20241209 add Single Use D Suck
    bool bSingleInArmUseOtherSuck;                                              //Ifor 20240430 add: Single Site InArm Use Other Suck(IN C)
    bool bDualSiteUseOneSuck;                                                   //wei 20251218 DualSite Use One Suck (port from HT9046LS V3.34, Ifor 20260803)   //AI(W906-TIF912) 20260925: golden 912 cprod.h:2436
    bool bPreventDropfunction;                                                  //Eastsun 20260710整合   //AI(W906-TIF912) 20260925: golden 912 cprod.h:2437

    bool bEnableAutoAlignment;                                                  //KenHsieh 20210813 : add CCD AUTO ALIGNMENT

    //pig 20220101 AddInputScaleBySetup
    bool bInArmUseDifferentScaleBySetupFile;
    double fLoaderTrayXScaleBySetupFile;
    double fLoaderTrayYScaleBySetupFile;
    double fHotPlateXScaleBySetupFile[2];
    double fHotPlateYScaleBySetupFile[2];
    double fInShuttleXScaleBySetupFile[2];
    double fInShuttleYScaleBySetupFile[2];
    //pig 20220101 AddInputScaleBySetup
    //pig 20220706 AddOutputScaleBySetup
    bool bOutArmUseDifferentScaleBySetupFile;
    double fOutShuttleXScaleBySetupFile[2];
    double fOutShuttleYScaleBySetupFile[2];
    double fUnLoaderTrayXScaleBySetupFile[10];
    double fUnLoaderTrayYScaleBySetupFile[10];
    //pig 20220706 AddOutputScaleBySetup

    bool bFix1AutoAlignmentcheck;                                               //Kenhsieh 20210915 : save Fix AutoAlignment check
    bool bFix2AutoAlignmentcheck;                                               //Kenhsieh 20210915 : save Fix AutoAlignment check
    bool bFix3AutoAlignmentcheck;                                               //Kenhsieh 20210915 : save Fix AutoAlignment check
    int iAutoAlignmentTrayEvent;                                                //0:Initial Start 1:After Home 2:Every Tray
    int iAutoAlignmentShuttleHotplateEvent;                                     //0:Initial Start 1:After Home 2:Every Tray
    int iAlignmentPointX;                                                       //Col
    int iAlignmentPointY;                                                       //Row

    int iAlignmentPlatePointX;                                                  //Col
    int iAlignmentPlatePointY;                                                  //Row

    int iAutoAlignmentTrayEvent_Z;                                              //0:Initial Start 1:After Home 2:Every Tray
    int iAutoAlignmentShuttleHotplateEvent_Z;                                   //0:Initial Start 1:After Home 2:Every Tray

    int iAutoAlignment_DeviceThick;

    int iAutoAlignmentTray_InArmZRealaseOffset;
    int iAutoAlignmentTray_InArmZPickUpOffset;

    int iAutoAlignmentTray_OutArmZRealaseOffset;
    int iAutoAlignmentTray_OutArmZPickUpOffset;

    int iAutoAlignmentCK_InArmZRealaseOffset;
    int iAutoAlignmentCK_InArmZPickUpOffset;

    int iAutoAlignmentCK_OutArmZRealaseOffset;
    int iAutoAlignmentCK_OutArmZPickUpOffset;
    int iAutoAlignment_UseFix[MAX_FIX_TRAY];                                    //Ifor 20210915 add AOA 選擇使用Fix位置
    int iAlignmentPointAutoZX;
    int iAlignmentPointAutoZY;
    AnsiString AutoAlignmentFileName;                                           //Kenhsieh 20210922 : save AutoAlignment FileName

    int iAOA_DecodeTimeOut;                                                     //Ifor 20211221 add: AOA Decode Time Out
    int InArmPickerLifeCnt[MAX_ARM_Row][MAX_ARM_Col];                           //JerryYang 20210531 add
    int OutArmPickerLifeCnt[MAX_ARM_Row][MAX_ARM_Col];                          //JerryYang 20210531 add
    int Arm1PickerLifeCnt[MAX_Index_Row][MAX_Index_Col];                        //JerryYang 20210531 add
    int Arm2PickerLifeCnt[MAX_Index_Row][MAX_Index_Col];                        //JerryYang 20210531 add
    int InOutArmLifeCntSet;

    double iFTInitialMaxTime;                                                   //JerryYang 20220516
    double iFTMaxTime;
    double iFTDummyTime;
    double dFTStartDelayTime;
    double dFTInitStartDelayTime;
    int    iFTInitStartDelayTimeCT;

    double iRTInitialMaxTime;                                                   //JerryYang 20220516
    double iRTMaxTime;
    double iRTDummyTime;
    double dRTStartDelayTime;
    double dRTInitStartDelayTime;
    int    iRTInitStartDelayTimeCT;

    double iEQCInitialMaxTime;                                                  //JerryYang 20220516
    double iEQCMaxTime;
    double iEQCDummyTime;
    double dEQCStartDelayTime;
    double dEQCInitStartDelayTime;
    int    iEQCInitStartDelayTimeCT;

    bool   bIndexPickICWhenOutShtNoIC;                                          //Steven 20221207 : Index必須在out shuttle沒料才可以吸
    bool   bEnableDelayTimeZero;                                                //Jimmychiu 20230922 : R230824-ATK-H9-01 Request add “Air purge during place device on out-shuttle” function.
    bool   bACSmart;                                                            //Sam 20230111 : Smart Auto Clean
    int    iACSmart_Count;
    int    iACSmart_Count_CTF;                                                  //Sam 20240726 : AI Clean
    int    iACSmart_ContactMode;
    int    iACSmart_ContactTime;
    int    iACSmart_ContactCount;
    int    iACSmart_DropHigh;
    bool   bUseRTCStepAsideMode;                                                //Ifor 20230908 add:RTC 讓位功能 By Setup File
    bool   bUseSocketHeating;                                                   //Ztex 2024.09.07 Add Use Socket Heating
    int    iUseSocketHeating;                                                   //Ztex 2024.09.07 Add Use Socket Heating
    bool   bAOANoUseOffset;                                                     //Ztex 2024.10.04 Add AOA Use Offset

    int  iMagDisplayOrder;                                                      //Ifor 20240227 add:Magazine 顯示順序變更
    AnsiString  asMulti2DIDStringSeparator;                                     //Ifor 20240829 add:Multi 2D ID String Separator
    bool bBarCodeMultiRecipe;                                                   //Ifor 20241031 add:使用BarCode Multi Recipe

    int  iAMRTrayCount[6];
    int  iAMRDeviceCount[6];

    int  iAMRLDNowTrayCount;
    int  iAMRLDSECSTrayCount;
    int  iAMRMagzineDeviceCount[14];                                            //Eastsun 20260515 F009 KYEC AMR fields
    AnsiString  asAMRBinSetting[3];                                             //Eastsun 20260515 F009 KYEC AMR fields

    int  iAMRARTCount;

    bool bAdaptiveLowYield;                                                     //Sam 20230914 : 自適應性良率監控
    int iAdaptiveContsLowerAlarmNor;                                            //Sam 20240726 : AI Clean
    int iAdaptiveContsLowerAlarmMin;
    int iAdaptiveYieldMax;
    int iAdaptiveYieldMin;

    bool bAdaptiveLowYield_RT;
    int iAdaptiveContsLowerAlarmNor_RT;                                         //Sam 20240726 : AI Clean
    int iAdaptiveContsLowerAlarmMin_RT;
    int iAdaptiveYieldMax_RT;
    int iAdaptiveYieldMin_RT;

    int iAdaptiveACIntervalMax;
    int iAdaptiveACIntervalMin;
    int iAdaptiveACIntervalAdj;

    bool bCreateManualEOCAP;                                                    //jou 20221104 : VTest CreateManualEOCAP function;

    int  iVaccumThrdIndexArm1[2][8];                                            //Sam 20230210 : 新增 VacuumUnit 通訊模組
    int  iVaccumThrdIndexArm2[2][8];
    int  iVaccumThrdInArm[2][8];
    int  iVaccumThrdOutArm[2][8];
    int iContactWarningCount[2];                                                //Sam 20241226 : Contact Alarm 需要先做 OneCycle

    bool bEnabledAutoCleanTimeCT;                                               //jou 20250102 : auto clean triger time count
    int iAutoCleanTimeCT;                                                       //jou 20250102 : auto clean triger time count

    double dAutoClean_InArmVacuum;                                              //Steven 20250319 : Auto Clean使用另外一組Delay Time
    double dAutoClean_InArmAirOn;
    double dAutoClean_IndexVacuum;
    double dAutoClean_IndexAirOn;

    bool   bIndexCycleTimeMonitor;                                              //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    double dIndexCycletimeMonitor;                                              //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    double dMonitorOutlier;                                                     //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    int    iMonitorWindow;                                                      //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    double dICTTolerance;                                                       //JerryYang 20220923 : Index cycle time監控
    int    iICTAction;                                                          //Steven 20250521 : 可以選擇要不要停機

    int    iInArmToShtReleaseMode;                                              //Steven 20250630 : 力旺要照順序擺/
    bool   bRENESAS_EnableFTCT;                                                 //RogerYang 20250916 : 瑞薩FT-CT

    bool bEnableE84;

    int iE84TimeOut_K12[2][11];
    int iLoaderUnloaderTrayCount[10];

    int iCassetteLDNowLayer;                                                    //RogerYang 20250206 : Add fot 9046CR
    int iCassetteAutoNowLayer[2];   int iAMRAutoStartDelay; int iAMRLoaderDetectDelay; int iAMRWaitSupplyDelay; bool bAMRICQtyAutoCleanOut;   //AI(W906-TIF912-R10) 20260926: 同行補 golden 912 cprod.h:2596-2599 [AMR] 四欄（Ifor 20260807/20260811/20260814），順序與 912 結構尾相同（緊接 iCassetteAutoNowLayer[2]，912 :2594）；RULINGS_20260926 第 10 條
}SYSTEM_TEST_IF;
extern SYSTEM_TEST_IF TestIF;
extern SYSTEM_TEST_IF TestIF_File;
extern SYSTEM_TEST_IF TestIF_NET;                                               // 2013.12.03 , Joye , KYEC FTP         //20140103 wei
typedef struct
{
    int iTestConnection;
    int iTemperatureMode;
    int iDutOnOff[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];                           //ChungHung 20130910 alter for SCK can close site by Index
    int iRunMode;
    int iDutOnOffEE[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];                         //Alick 20160923 add for 第三組工程師用開關SITE
}SYSTEM_TEST_MODE;
extern SYSTEM_TEST_MODE TestMode;                                               //Steven 20111019
//==============================================================================
/*typedef struct                                                                //Steven 20140409 : Mark Scanner
{
    bool bOnLine;
    int  Mode;
    int Rate;
    int ScanDelay;
    int IDCode;
    bool bFailAlarm;
    int FailCount;
    bool bScanCountOn;
    int ScanFailCount;
    int ScanTotalCount;
    bool ScanRateMode;
    int MG1;
    int MG2;
    int PassSet;
    double LimitTime;
}SYSTEM_SCANNER_IF;
extern SYSTEM_SCANNER_IF ScannerIF;     */
//==============================================================================
typedef struct
{
    bool    bScanner    [TEST_MAX_BIN];
    int     iDBContact  [TEST_MAX_BIN];                                         //Steven 20231205 : bDBContact --> iDBContact
    bool    bConsFail   [TEST_MAX_BIN];
    bool    bFailure    [TEST_MAX_BIN];

    int     iPersentIgnore  [TEST_MAX_BIN];                                     //Steven 20140529 Start: Fail Persent & Count同時存在
    bool    bFailCountEnable[TEST_MAX_BIN];
    int     iFailCountIgnore[TEST_MAX_BIN];
    int     iFailCountLimit [TEST_MAX_BIN];

    int     iCatDataT3Pos   [TEST_MAX_BIN];
    int     IfErrorT3;
    int     ScanInvalid;
    int     ScanFailure;
    int     ScanRework;

    bool    bScanRejectConsFail;
    bool    bScanInvalidConsFail;
    bool    bScanReworkConsFail;
    bool    bScanRejectFailure;
    bool    bScanInvalidFailure;
    bool    bScanReworkFailure;

    int     iStackDefFailCate[eTrayCount];                                      //JerryYang 20220909 : 10->eTrayCount   //Steven 20160310 : 改成有顏色的fail bin bool --> int   //Steven 20161221 : 9 --> 10

    int     ScanOnlyPass;
    int     ScanOnlyInvalid;
    int     ScanOnlyFailure;
    int     ScanOnlyRework;
    double  dFailureLimit[TEST_MAX_BIN];                                        //jou 2012-11-28 Bin Yield Failure 的百分比值要可以輸入到小數點一位,另外也要紅底+密碼

    int     iTrayType[eTrayCount];

    int     iOpenBin;                                                           //jou 2011-03-24 start : Auto Site Mapping
    bool    bAutoRetest[eTrayCount];                                            //ChungHung 20140317 add Auto Retest
    bool    bAutoLink[eTrayCount];
    bool    bMagazineLink[eTrayCount];                                          //Ifor 20231122 add Magazine Link
    bool    bBinTrayLinked[TEST_MAX_BIN];

    bool    bCateR[eTrayCount];                                                 //JerryYang 20220909 : 10->eTrayCount        //Steven 20161221 : Cate R for ART
    int     iARTTrack;                                                          //Steven 20161221 : Cate R for ART

    //ChungHung 20140730 add ContinuousFailHaveOneCycle start
    bool bSpecialBinByArm[TEST_MAX_BIN];
    unsigned int  iSpecialBinCountByArm[TEST_MAX_BIN];
    bool bSpecialBinBySocket[TEST_MAX_BIN];
    unsigned int  iSpecialBinCountBySocket[TEST_MAX_BIN];
    //ChungHung 20140730 add ContinuousFailHaveOneCycle end

    bool bLowYield[TEST_MAX_BIN];                                               //Steven 20140828 : By Bin Yield Monitor
    bool bArmYield[TEST_MAX_BIN];                                               //Steven 20140828 : By Bin Arm Yield Monitor
    bool bSiteYield[TEST_MAX_BIN];                                              //Steven 20140828 : By Bin Site Yield Monitor
    int iAutoCleanByBin[TEST_MAX_BIN];                                          //Steven 20160308 : By Bin count do auto clean
    int iAutoCleanBySite[TEST_MAX_BIN];                                         //Steven 20160308 : By Bin pre site count do auto clean
    //JerryYang 20170712 (Steven) by site by bin compare percent
    //==>
    bool bSpecBinBySiteCompareEnable[TEST_MAX_BIN];
    int iSpecBinBySiteCompareIgnore[TEST_MAX_BIN];
    double dSpecBinBySiteComparePercent[TEST_MAX_BIN];
    bool bSpecBinByArmPerSiteCompareEnable[TEST_MAX_BIN];
    int iSpecBinByArmPerSiteCompareIgnore[TEST_MAX_BIN];
    double dSpecBinByArmPerSiteComparePercent[TEST_MAX_BIN];
    //<==
    //JerryYang 20170712 (Steven) by site by bin compare percent

    //bool bCancelErrorBin;                                                     //kevin 20160724 取消 ERROR BIN設定
    int  iAOICategData  [TEST_MAX_BIN];                                         //Eastsun 20260316 : AOI 要分Bin

    bool    IfErrorT3Define;
}SYSTEM_BIN_SELECT;
//first array define 0~2 0:RT 1:FT 2:OffLine 3:RT_ART 4:FT_ART
extern SYSTEM_BIN_SELECT BinSelect[8];                                          //ChungHung 20111110 add  //ChungHung 20141002 add for KYEC AutoRetest   //Ifor 20170316 (wei) add MRT Mode 5->8
extern SYSTEM_BIN_SELECT BinSelect_NET[8];                                      //ChungHung 20141002 add for KYEC AutoRetest    // 2013.12.03 , Joye , KYEC FTP  //20140103 wei  //Ifor 20170316 (wei) add MRT Mode 5->8
//==============================================================================
typedef struct                                                                  //Alick 20160727 add for SCC 增加到1000組
{
    int  RecordCT;
    char ID[1000][30];
    char PassWord[1000][30];
    int  Level[1000];
}PASS_WORD;
extern PASS_WORD USER;
//------------------------------------------------------------------------------
struct JAM_COUNT
{
    int iCount;
    AnsiString JamCode;
    AnsiString Message;
};
//==============================================================================
struct RUN_INFO
{
    RUN_INFO();
    ~RUN_INFO();
    void InitialData();
    void SaveJamRateByLot(bool bUpload=true);                                   //Steven 20200415 : SCC要By Lot Jam Rate
    void ReadJamRateByLot();                                                    //Steven 20200415 : SCC要By Lot Jam Rate
    void SetLotStartTime();                                                     //Sam 20240426 : Add BarCoder Inspection Report
    AnsiString JamRateFileName;

    void InitialDailyData();
    void SaveJamRateByDay(bool bUpload=true);                                   //Steven 20250528 : By Day Jam Rate
    void ReadJamRateByDay();                                                    //Steven 20250528 : By Day Jam Rate
    AnsiString DailyJamFileName;
    AnsiString sToday;
    int  iToday;
    int  iDailyCount;

    int  iUPH;                                                                  //Steven 20140429 : For SECS_GEM
    AnsiString iAvgUPH;                                                         //Steven 20140429 : For SECS_GEM
    int  iUnloadCount;                                                          //Steven 20140429 : For SECS_GEM
    AnsiString sT6AutoYield[eTrayCount];                                        //JerryYang 20220909 : 10->eTrayCount  //kevin 20160819 add bulk box  //Steven 20140429 : For SECS_GEM
    AnsiString SystemTime;                                                      //Steven 20140429 : For SECS_GEM
    AnsiString MTBA;                                                            //Steven 20140429 : For SECS_GEM
    AnsiString MUBA;                                                            //Steven 20140429 : For SECS_GEM
    AnsiString SoftwareDate;                                                    //Steven 20140429 : For SECS_GEM
    AnsiString Factory;                                                         //Steven 20140429 : For SECS_GEM
    AnsiString MachineDefine;                                                   //Steven 20140429 : For SECS_GEM
    AnsiString SoftwareVersion;                                                 //Steven 20140429 : For SECS_GEM
    AnsiString ShowTempComp[tcTotalCount];                                      //Steven 20140429 : For SECS_GEM
    AnsiString LotNo;
    AnsiString LotStartTime;
    AnsiString LotEndTime;
    AnsiString TestTime;
    double     dTestTimeSec;
    AnsiString IndexCycleTime;
    AnsiString IndexTime;
    AnsiString GalilVersion;                                                    //Steven 20141202 : GalilVersion
    AnsiString ATCSoftwareVersion;
    AnsiString GPIBSoftwareVersion;
    AnsiString ESDSoftwareVersion;                                              //Ifor 20170320 (wei) add ESD Software Version
    AnsiString OCRSoftwareVersion;                                              //Ifor 20250612 add OCR Software Version
    Word LotStartYear;                                                          //Sam 20240426 : Add BarCoder Inspection Report
    Word LotStartMonth;
    Word LotStartDate;
    Word LotStartHour;
    Word LotStartMin;
    Word LotStartSec;

    TStringList *slEventLogFile;
    TStringList *slExe;
    std::map<AnsiString, JAM_COUNT> vByLotJam;
    std::map<AnsiString, JAM_COUNT> vDailyJam;
    std::map<AnsiString, JAM_COUNT>::iterator vJamIter;
    void AddAlarm(AnsiString Jam, AnsiString Message);

    bool bLotStart;                                                             //Steven 20140616 : For KYEC
    int  iYieldChart[MAX_SOCKET_ROW][MAX_SOCKET_COL][25];                       // min  unit    //jou 2014-04-01 2->4 記憶體破壞修正
    int  iYieldHour[25];                                                        // min  unit
    int  iYieldMin[25];                                                         // min  unit
    bool bSiteEnable[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    int  iUnloadCount_ART;                                                      //kevin 20150615 ART             //Steven 20140429 : For SECS_GEM
    AnsiString sT6AutoYield_ART[eTrayCount];                                    //JerryYang 20220909 : 10->eTrayCount  //kevin 20160819 //kevin 20150615 ART       //Steven 20140429 : For SECS_GEM
    int iPreAlarmPosition;                                                      //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color
    bool bSafeDoor[MAX_SAFE_DOOR_CNT];                                          //Ifor 20200416 add SafeDoor report  //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
    int iEESUGSelectSVID;                                                       //Ifor 20200529 add: SECS/GEM EESUG Offset Function
    int iEESUGModify;                                                           //Ifor 20200529 add: SECS/GEM EESUG Offset Function 0:Noraml 1:X 2:Y 3:Pitch 4:Pick 5:Place 6:PitchY 7:PitchX2 8:Contact Height Offset
    AnsiString asEESUGUnit;                                                     //Ifor 20200529 add: SECS/GEM EESUG Offset Function

    bool bHasRTCModule;                                                         //Ifor 20200811 add: Hardware module installation SVID
    bool bHasDualEPModule;
    bool bHas2DBarcodeModule;
    bool bHasHeatGunModule;
    bool bHasRotateModule;
    bool bHasATCSystemModule;
    bool bHasOCRModule;
    bool bHasPRECISERModule;
    bool bHasShuttleVibrationModule;
    bool bHasDewpointmeterModule;
    bool bHasLoaderVibrationModule;
    bool bHasTRAYVibrationModule;
    bool bHasLoaderPressTrayModule;
    bool bHasPurgeKitModule;

    int iAutoCleanTriggerCondition;                                             //Ifor 20220829 add: Report Auto Clean 觸發條件
    AnsiString SECSGEMVersion;                                                  //Ifor 20230112 add SECS GEM Version
    AnsiString asChillerTemp;
    bool bHasHotplateVibrationModule;
    bool bUseOneTouchContactHeight;
    AnsiString BarcodeSoftwareVersion;
    AnsiString PowerSavingStartTime;
    AnsiString PowerSavingEndTime;
    bool bSecsGemATCEnergySavingState;                                          //yunghsin 20231123 Add Secs/Gem Energy Saving Mode
    bool bSecsGemHeaterEnergySavingState;                                       //yunghsin 20231123 Add Secs/Gem Energy Saving Mode
    bool bSecsGemMotorEnergySavingState;                                        //yunghsin 20231123 Add Secs/Gem Energy Saving Mode
    bool bSecsGemTesterPurgeKitEnergySavingState;                               //yunghsin 20231123 Add Secs/Gem Energy Saving Mode
    bool bSecsGemDryAirEnergySavingState;                                       //yunghsin 20231123 Add Secs/Gem Energy Saving Mode
    bool bSecsGemEnergySavingState;                                             //yunghsin 20231123 Add Secs/Gem Energy Saving Mode
    bool bSafeDoorIsOpen;

    AnsiString HandlerPowerMeasurementValues;                                   //Ifor 20240514 add:Handler Power Measurement Values
    AnsiString RTCSoftwareVersion;                                              //Ifor 20241001 add:SECS GEM RTC Version
    AnsiString AOASoftwareVersion;                                              //Ifor 20241001 add:SECS GEM AOA Version
    AnsiString HandlerMinorVersion;                                             //Ifor 20241001 add:SECS GEM AOA Version
    AnsiString ATCParametersCh[32];                                             //Ifor 20241120 add:SECS GEM ATC Channel Parameters By Recipe
    AnsiString ATCParameters;                                                   //Ifor 20241120 add:SECS GEM ATC Public Parameters By Recipe

    AnsiString TempLowOffSet;                                                   //Ifor 20250624 add:SECS GEM Temp Offset
    AnsiString TempMidOffSet;                                                   //Ifor 20250624 add:SECS GEM Temp Offset
    AnsiString TempHighOffSet;                                                  //Ifor 20250624 add:SECS GEM Temp Offset
    AnsiString TempUserOffSet;                                                  //Ifor 20250624 add:SECS GEM Temp Offset
    AnsiString TempSingleLimit;                                                 //Ifor 20250624 add:SECS GEM Temp Offset
    AnsiString TempHotLowOffSet;                                                //Ifor 20250624 add:SECS GEM Temp Offset
    AnsiString TempHotMidOffSet;                                                //Ifor 20250624 add:SECS GEM Temp Offset
    AnsiString TempInitOffSet;                                                  //Ifor 20250624 add:SECS GEM Temp Offset
    AnsiString TempEOTOffSet;                                                   //Ifor 20250624 add:SECS GEM Temp Offset
};
extern RUN_INFO RunInfo;

//extern RUN_INFO RunInfo2;
typedef struct
{
    int iBodySP;
    int iZSP;
    int iVariSP;
    int iACDCBodySP;
    int iACDCZSP;
    int iACDCVariSP;
    int iVariSPY;                                                               //ChungHung 20131231 alter AutoYPitch
    int iACDCVariSPY;                                                           //ChungHung 20131231 alter AutoYPitch
    int iRetryCT;
    double dRetryDown;
    double dVacuumTI;
    double dHPVacuumTI;                                                         //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    bool bUseHPVacuum;                                                          //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    double dCTAirOn;
    bool bSuckOnDown;
    bool bAutoSKIP;
    bool bVariModeFIX;
    double dWaitOnSH;
    bool bIndexFloatCHK;
    bool bAutoSpeed;
    double dTAHeadDownIT;
    double dDestroyPauseTime;                                                   //jou 2010-12-01 start : 小IC時,會偶發性置偏
    double dDestroyAgainTime;                                                   //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    int iDestroyAgainCount;                                                     //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    double dDestroyCheckTime;                                                   //2013-08-01    Dell    modify  公司統一回吸功能
    bool bDestroyPauseCheck;                                                    //2013-08-01    Dell    modify  公司統一回吸功能
    int iAutoSkipCT;                                                            //jou 2013-09-23 Auto Skip CTS
    int     iTwoSpeedMove;                                                      //Steven 20140217 : 兩段速移動的啟動開關
    double  dTwoSpeedDistance;                                                  //Steven 20140217 : 兩段速移動的距離
    int     iTwoSpeed;                                                          //Steven 20140217 : 兩段速移動的速度
    int     iTwoADC;                                                            //Steven 20140217 : 兩段速移動的加減速
    double dReleaseDelayTime;                                                   //JerryYang 20160127 for TSMC inarm release device前delay
    int iEnableReleaseDelay;                                                    //JerryYang 20160127 for TSMC inarm release device前delay
    bool bHeightCheck;                                                          //wei 20160825 回吸偵測
    int iHeightCheck;                                                           //wei 20160825 回吸偵測
    int iEnableDieClean;                                                        //wei 20170418
    double dDieCleanDelay;                                                      //wei 20170418
    double dDieCleanHeight;                                                     //wei 20170418
    double dIndexDelayTime;                                                     //wei 20171115
    bool bIndexDelayTime;                                                       //wei 20171115
    bool bDevicConfirm;                                                         //kevin 20170510 (wei) 回吸功能開關
    bool bYPitchUseSearchLastMode;                                              //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
    bool bTwoSpeedOnlyLoader;                                                   //JerryYang 20190729 二段速功能可選擇only at loader
    int  iAutoSpeedLow;                                                         //kevin 20210913 AutoSpeed 最低速
    double dCylinderDelay;
    bool bTT_SetSpeed;                                                          //Ztex 2024.08.11 Add Test Time Set Speed
    int iEnableBottomBlower;
    int iBottomBlowerTime;
    int iBottomBlowerSpeed;
    int PrecisorOpenSp;                                                         //JerryYang 20250120 : add
    int PrecisorCloseSp;
} ARM_CONDITION;
extern ARM_CONDITION ArmSpeed[SpeedPartTotal];
extern ARM_CONDITION ArmSpeed_File[SpeedPartTotal];
extern ARM_CONDITION AutoArmSpeed[SpeedPartTotal];                              //jou 2012-05-16 Auto Speed

typedef struct                                                                  //Steven 20110407 Start: Shuttle Thread的變數
{
    bool bUseM204Mode;
    bool bExeShuttleThread;
    bool bOutYUseLatch;                                                         //Steven 20140818
    int iCHKStep;
    int base_pos[2];
    int base_posY[2];
    int base_InposY[2];                                                         //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    int base_InposY_SHSn[2];                                                    //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    int iScanSensor;
    int iShuttleThreadTask[2][2];
    int iCheckPosMax[MAX_Index_Row][MAX_Index_Col];
    int iCheckPosMin[MAX_Index_Row][MAX_Index_Col];
    int iCheckPosMaxY[MAX_Index_Row][MAX_Index_Col];
    int iCheckPosMinY[MAX_Index_Row][MAX_Index_Col];
    int iCheckInPosMaxY[MAX_Index_Row][MAX_Index_Col];                          //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    int iCheckInPosMinY[MAX_Index_Row][MAX_Index_Col];                          //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    int iCheckInPosMaxY_SHSn[2][8];                                             //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    int iCheckInPosMinY_SHSn[2][8];                                             //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    bool bInSh1HasIC_SHSn[2][8];                                                //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    bool bInSh2HasIC_SHSn[2][8];                                                //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    bool bSh1HasIC[MAX_Index_Row][MAX_Index_Col];
    bool bSh2HasIC[MAX_Index_Row][MAX_Index_Col];
    bool bShICFloating[MAX_Index_Row][MAX_Index_Col];                           //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
    bool bInSh1HasIC[MAX_Index_Row][MAX_Index_Col];                             //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    bool bInSh2HasIC[MAX_Index_Row][MAX_Index_Col];                             //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    AnsiString ErrPartSh1;
    AnsiString ErrPartSh2;
    AnsiString FloatingErrPartSh1;
    AnsiString FloatingErrPartSh2;
    int iInShSenIndex[2][10];                                                   //8->10 //2013-07-16    Dell    Shuttle cross sensor
    bool bUseInShtSen[10];                                                      //Steven 20250429 : 有用到的in sht sensor位置
    int iUseInShtStep[10];                                                      //Steven 20250429 : 有用到的in sht sensor位置
    int iOutShSenIndex[MAX_Index_Row][MAX_Index_Col];
    int iInShuttleSen7[2];                                                      //定義最後一顆Sensor的位置
    int iInShRotateCheck[2];                                                    //定義In Rotate Shuttle向右走的檢查位置
    int iOShRotateToLeftCheck[2];                                               //定義Out Rotate Shuttle向左走的檢查位置
    int iOShRotateToRightCheck[2];                                              //定義Out Rotate Shuttle向右走的檢查位置
    bool bChechInShFirst[2];                                                    //Rotate Shuttle向右走時，要先檢查In或Out

    int iSocketSensor[24];                                                      //kevin 20130504 使用SOCKETSENSOR

    int iLaserBasePos[2];                                                       //Steven 20140228 : 雷射測距功能
    int iLaserCheckPos[MAX_Index_Row][MAX_Index_Col];                           //Steven 20140228 : 雷射測距功能
    int iLaserCheckHeight[MAX_Index_Row][MAX_Index_Col];                        //Steven 20140228 : 雷射測距功能
} SHUTTLE_THREAD;
extern SHUTTLE_THREAD SThreadPara;
typedef struct
{
    int iSH1Sp;
    int iSH2Sp;
    int iSH1ACDCSp;
    int iSH2ACDCSp;
    int iCCDSp;
    int iCCDACDCSp;
//    double dDevCHKIT;
        int iStepSH;
    int iSHSortSp;                                                              //RogerYang 20250514 Add for 9046AU
    int iSHSortACDCSp;
    int iShakeCycles;                                                           //Sam 20250326 : 新增 Shake 條件設定
    int iShakeDistance;
    double dShakeDelay;
    int iShakeAccDec;
}SHUTTLE_SPEED;
extern SHUTTLE_SPEED SHSpeed;
extern SHUTTLE_SPEED SHSpeed_File;

typedef struct                                                                  //JerryYang 20220909 : add magazine
{
    int iCYSp;
    int iMZSp;
    int iCYACDCSp;
    int iMZACDCSp;
    int iCY2Sp;
    int iCY2ACDCSp;
    double dCYDecRange;
}MAGAZINE_SPEED;
extern MAGAZINE_SPEED MGSpeed;
extern MAGAZINE_SPEED MGSpeed_File;

typedef struct
{
    int iBodySP;
    int iACDCBodySP;
    int     iTwoSpeedMove;                                                      //Steven 20140217 : 兩段速移動的啟動開關
    double  dTwoSpeedDistance;                                                  //Steven 20140217 : 兩段速移動的距離
    int     iTwoSpeed;                                                          //Steven 20140217 : 兩段速移動的速度
    int     iTwoADC;                                                            //Steven 20140217 : 兩段速移動的加減速
    int iRetryCT;
}MR_SPEED;
extern MR_SPEED MRSpeed[MRSpeedPartTotal];
extern MR_SPEED MRSpeed_File[MRSpeedPartTotal];
//------------------------------------------------------------------------------
typedef struct
{
    double LD_TrayArrivalDely;
    double LD_FixTrayDely;
    double LD_StackMiddLockDelay;
    double LD_BeforeDownDelay;                                                  //Steven 20240215 : 下降前Delay
    double LD_LiftDownDelay;
    bool   LD_EnableVibrate;                                                    //JerryYang 20191001 loader震動馬達
    int    LD_VibrateLoopCnt;
    double LD_VibrateOnDelay;
    double LD_VibrateOffDelay;
    double ULD_TrayArrivalDelay;
    double ULD_FixTrayDely;
    double ULD_TrayBackDelay;
    double ULD_LiftDownDelay;
    double ULD_TrayArrivalWaitDelay;                                            //kevin 20190326 add auto 退tray 等待時間
}LD_ULDTIME;
extern LD_ULDTIME Ld_UldDelayTime;
//------------------------------------------------------------------------------
//Eliot 2009_03_06
typedef struct RESERVE_EMPTY_POINT{
    bool bHasData;
    int iXPoint;
    int iYPoint;
    int iWhichAuto;
}RESERVE_EMPTY_POINT;
//extern RESERVE_EMPTY_POINT ReserverEmptyPoint[20];
extern RESERVE_EMPTY_POINT ReserverEmptyPoint[1000];                            //ChungHung 20111215 嘗試將Fix3放滿
extern RESERVE_EMPTY_POINT ReserverEmptyPointAutoClean[20];
//------------------------------------------------------------------------------
typedef struct {
    int iOffsetXYHigh;
    int iOffsetXYLow;
    int iOffsetZHigh;
    int iOffsetZLow;
    int iOffsetUnloaderZHigh;                                                   //Steven 20210317 : 通富微要求unloader獨立設置
    int iOffsetUnloaderZLow;                                                    //Steven 20210317 : 通富微要求unloader獨立設置
    int iTempHigh;
    int iTempLow;
    int iHeaterGunH;
    int iHeaterGunL;
    double dContactHigh;                                                        //Steven 20140123 : Contact Height的Offset限制
    double dContactLow;                                                         //Steven 20140123 : Contact Height的Offset限制

    double dShuttleHigh;                                                        //ChungHung 20150115 add for ATK +/-2 mm
    double dShuttleLow;                                                         //ChungHung 20150115 add for ATK +/-2 mm
    int iContactCntAlm;                                                         //JerryYang 20200504 銦片計數限制

    int iIlitialTempHigh;                                                       //kevin 20210421 ilitial temp offset high 個別範圍
    int iIlitialTempLow;                                                        //kevin 20210421 ilitial temp offset low 個別範圍

    double dForcePerpinNHigh;
    double dForcePerpinNLow;                                                    //Isaac 20210517 : Pinforce加上上下限
    double dForcePerpinHigh;
    double dForcePerpinLow;                                                     //Steven 20220216 : Pinforce加上上下限
    double dLoaderOffsetXYHigh;                                                 //JerryYang 20220923 : 矽品蘇州要求offset limit要By區域設定
    double dLoaderOffsetXYLow;
    double dLoaderOffsetZHigh;
    double dLoaderOffsetZLow;

    double dHPOffsetXYHigh;
    double dHPOffsetXYLow;
    double dHPOffsetZHigh;
    double dHPOffsetZLow;
    double dHPOffsetZRelHigh;
    double dHPOffsetZRelLow;

    double dInShtOffsetXYHigh;
    double dInShtOffsetXYLow;
    double dInShtOffsetZHigh;
    double dInShtOffsetZLow;

    double dOutShtOffsetXYHigh;
    double dOutShtOffsetXYLow;
    double dOutShtOffsetZHigh;
    double dOutShtOffsetZLow;

    double dUnloadOffsetXYHigh;
    double dUnloadOffsetXYLow;
    double dUnloadOffsetZHigh;
    double dUnloadOffsetZLow;

    double dAutoCleanContactHigh;
    double dAutoCleanContactLow;
}INPUT_LIMIT;
extern INPUT_LIMIT InputLimit;                                                  //Steven 20090805
//------------------------------------------------------------------------------
typedef struct {
    int iArmVacuumOn;
    int iArmVacuumOff;
    int iIndexVacuumOn;
    int iIndexVacuumOff;
}DUMMY_VACUUM;
extern DUMMY_VACUUM DummyVacuum;                                                //Jou 980805
//------------------------------------------------------------------------------
/*typedef struct
{
    int IO_Dev_Num[2];
    bool bInitialScan;

    void TMotionnetIO()
    {
        bInitialScan =false;
    }
}TMotionnetIO;
extern TMotionnetIO tMotionnetIO;                                               //2013-01-15    Dell 開機的時候要先Scan Motionnet IO 數量    */
//------------------------------------------------------------------------------
typedef struct
{
    bool bEnabled;
    int iFailBin;
    int iTimeOut;
    int iStartDelayTime;
    int iTestFailFailBin;
    int iTestPassFailBin;
}TVisionStation;
typedef struct
{
    bool bEnabledAOI;
    TVisionStation tTopView;
    TVisionStation tPADView;
    TVisionStation tBGAView;
    TVisionStation tFailRunFunction;
    TVisionStation tAutoSkipBGA;
    TVisionStation tAutoSkipPadPkg;
    TVisionStation tTesterFailBin;
    TVisionStation tBGAPADView;
    TVisionStation tAOINoSort;                                                  //wei 20161207 (jou) AOI
    int iAlarmCount;
    bool bEnabledContinueAlarm;                                                 //Alick 20160712 add
    int iContAlarmCountBySite;                                                  //Alick 20160712 add
    int iContAlarmCountByArm;                                                   //Alick 20160718 add
    bool bEnabledBySiteBGAView;                                                 //Alick 20160712 add
    bool bEnabledByArmBGAView;                                                  //Alick 20160712 add
    bool bEnabledBySitePADView;                                                 //Alick 20160712 add
    bool bEnabledByArmPADView;                                                  //Alick 20160712 add

    bool bEnabledScanAOIBySiteAlarm;                                            //Ifor 20191225 : add Scan AOI Alarm
    bool bEnabledScanAOIByArmAlarm;                                             //Ifor 20191225 : add Scan AOI Alarm
    int iScanAOIAlarmCountBySite;                                               //Ifor 20191225 : add Scan AOI Alarm
    int iScanAOIAlarmCountByArm;                                                //Ifor 20191225 : add Scan AOI Alarm
    bool bEnabledScanAOIUnUseFailBin;                                           //Ifor 20200825 add:TF-AMD AOI Error 不丟至Fail Bin

    bool bEnabledTopScanAOIBySiteAlarm;                                         //Ifor 20200902 add: TFAMD Top AOI
    bool bEnabledTopScanAOIByArmAlarm;                                          //Ifor 20200902 add: TFAMD Top AOI
    int iTopScanAOIAlarmCountBySite;                                            //Ifor 20200902 add: TFAMD Top AOI
    int iTopScanAOIAlarmCountByArm;                                             //Ifor 20200902 add: TFAMD Top AOI
    bool bEnabledTopScanAOIUnUseFailBin;                                        //Ifor 20200902 add: TFAMD Top AOI
}TAOISetup;
extern TAOISetup tAOISetup;                                                     //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
//------------------------------------------------------------------------------
typedef struct
{
    double dOriginalUseOffset[tcTotalCount];
    int iTimeLogEnd;                                                            //2014-05-30    Dell    for ATC6.0
    bool bRecordTime_Tj;                                                        //2014-05-30    Dell    for ATC6.0
}TATCData;
extern TATCData tATCData;                                                       //2014-05-30    Dell    for ATC6.0
//------------------------------------------------------------------------------
//Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案
//------------------------------------------------------------------------------
class ATK_RECIPE_INFO
{
private:
    //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
    //<==
    void GetSiteNumberString(int &iSitemap,int &iRow,int &iCol);
    AnsiString STX;
    AnsiString ETX;
    AnsiString GetForwardSlash(){return AnsiString("/");}
    int SiteMapDirection();
    AnsiString GetUsedSitesNumber(int iDir);
    AnsiString GetParameterFormat(AnsiString sName, AnsiString sValue);
    //<==
    //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
public:
    ATK_RECIPE_INFO();
    ~ATK_RECIPE_INFO();
    AnsiString SiteCount;
    AnsiString WorkTemperBase;
    AnsiString SoakTime;
    TStringList *slBinPassFail;
    TStringList *SL;
    TStringList *slSortgate;
    AnsiString  sSortgate;
    AnsiString  sSitemap;
    void SaveFile();
};
extern ATK_RECIPE_INFO *ATKRecipeInfo;
//------------------------------------------------------------------------------
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//==>
typedef struct
{
    int ItemX;
    int ItemY;
    double pointX[50][50];
    double pointY[50][50];
}AUTOTEACH_POINT;

enum eInArmAOAType
{
    AOA_Loader      =0,
    AOA_Plate1      =1,
    AOA_Plate2      =2,
    AOA_InSht1      =3,
    AOA_InSht2      =4,
    AOA_AutoClean   =5,

    TotalInArmAOAType
};

enum eOutArmAOAType
{
    AOA_Auto1       =0,
    AOA_Auto2       =1,
    AOA_Auto3       =2,
    AOA_Auto4       =3,                                                         //Steven 20240428 : Add for HT9011 AOA
    AOA_Auto5       =4,
    AOA_Auto6       =5,
    AOA_Fix1        =6,
    AOA_Fix2        =7,
    AOA_Fix3        =8,
    AOA_Fix4        =9,
    AOA_Fix5        =10,
    AOA_Fix6        =11,
    AOA_OutSht1     =12,
    AOA_OutSht2     =13,
    TotalOutArmAOAType
};

extern AUTOTEACH_POINT InputAtuoTeachTable[TotalInArmAOAType];
extern AUTOTEACH_POINT InputAtuoTeachTableCal[TotalInArmAOAType];
extern AnsiString asInArmAOAFileName[TotalInArmAOAType];
extern AnsiString asInArmAOAFileName_Cal[TotalInArmAOAType];

extern AUTOTEACH_POINT OutputAtuoTeachTable[TotalOutArmAOAType];
extern AUTOTEACH_POINT OutputAtuoTeachTableCal[TotalOutArmAOAType];
extern AnsiString asOutArmAOAFileName[TotalOutArmAOAType];
extern AnsiString asOutArmAOAFileName_Cal[TotalOutArmAOAType];

bool ReadAutoTeachTable_InArm();                                                //KenHsieh 20211208 : In/Out Arm分開讀檔，避免InArm做完直接寫入導致OutArm點位錯誤
bool ReadAutoTeachTable( char* cFName, AUTOTEACH_POINT* DataBuf);
bool WriteAutoTeachTable( char* cFName, AUTOTEACH_POINT* DataBuf);
bool ReadAutoTeachTable_OutArm();                                               //KenHsieh 20211208 : In/Out Arm分開讀檔，避免InArm做完直接寫入導致OutArm點位錯誤
//------------------------------------------------------------------------------
//<==
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT

typedef struct
{
  //InArm
    int  iAutoCleanPick;                                                        //kevin 20190305 teach pos .ini

  //INDEX
    int iLoadCellY1;                                                            //kevin 20190306 Arm1 在load cell 位置
    int iLoadCellY2;                                                            //kevin 20190306 Arm2 在load cell 位置
    int iLoadCellZ1Down;                                                        //kevin 20190306 Arm1 在load cell 位置
    int iLoadCellZ2Down;                                                        //kevin 20190306 Arm2 在load cell 位置
    int iContactZ1Relative;                                                     //JerryYang 20240229 : add
    int iContactZ2Relative;
}Teach_Pos;
//==============================================================================
extern Teach_Pos Teach;                                                         //kevin 20190305 teach pos .ini
bool WriteLastDataFile(bool BackUp2=false, bool bNotContact=false);             //ChungHung 20120829 add
bool ReadLastDataFile();
bool WriteData(char *cFName, char *ptr, int size);
bool ReadData(char *cFName, char *ptr, int size);
void ClearLastSet();
void SavePassword();
void ReadPassword();
bool CheckFileCanAccess(char *cFName);
bool CheckFileExist(AnsiString cFName);                                         //Ken 20210702 AddPadInterface

extern const int RT;
extern const int FT;
extern const int OffT;                                                          //ChungHung 20111110 add
extern const int RT_ART;                                                        //ChungHung 20141002 add for KYEC AutoRetest
extern const int FT_ART;                                                        //ChungHung 20141002 add for KYEC AutoRetest
extern const int RT_MRT;                                                        //Ifor 20170316 (wei) add MRT Mode
extern const int FT_MRT;                                                        //Ifor 20170316 (wei) add MRT Mode
extern const int EQC;

extern const int OffLine;
extern const int OnLine;

extern bool bInstallRotate;
//extern bool bSaveStandardConfig;    //KaiChen 20171113 (Steven) ：超豐 將 config.ini 另外存成 config_Standard.ini        //JimmyChiu 20220114 CUSTOMER_CODE==CC_Greatek => CosFunction.bConfigStandard
//extern bool bInitalStandardConfig;  //KaiChen 20171113 (Steven) ：超豐 將 config.ini 另外存成 config_Standard.ini        //JimmyChiu 20220114 CUSTOMER_CODE==CC_Greatek => CosFunction.bConfigStandard
extern const int IFaceErr;
//Eliot 2008_05_19
void InitialReserveEmptyPoint();
bool EnableFix3UseCylinder();                                                   //JimmyChiu 20220927 : Stepper Motor Control in Fix3
void ReserveEmptyPoint(int iAutoTray,int iXpos,int iYpos);
void ReversionEmptyPoint();

extern const bool bReadFile;
extern const bool bWriteFile;
void ProcessLastSetIni_Visible(bool bRead);
//Steven 20100811 Start
void SaveLastSetIni();
void ReadLastSetIni();
void SaveRmsInfo(AnsiString Name, AnsiString Temp);
void ReadRmsInfo();
//Steven 20100811 End
void ReadConfigByRecipe();                                                      //JimmyChiu 20220601 : config儲存跟隨recipe

void CustomerFunctionSelect();                                                  //客戶功能選擇區
void KoreaFunction();
void SingaporeFunction();
void SaveEventLogAutoSaveInfo();                                                //Steven 20110603
void ReadEventLogAutoSaveInfo();                                                //Steven 20110603
void ReadTasterInfo();                                                          //Steven 20110603
void SaveTasterInfo();                                                          //Steven 20110603
void ReadRmsPath();                                                             //Steven 20110623
bool FileDataCompare(char *cFName1,char *cFName2);
void SaveTestMode();                                                            //Steven 20111019
void ReadTestMode();                                                            //Steven 20111019
void SaveTempMode();                                                            //Ifor 20180212 (Steven) : add FTP DownLoad Temp Mode By SetupFile
void SaveTempModeByDLL();                                                       //JerryYang 20181126 (Steven) : support Epson DLL function
//void ProcessLastSetIni_AlarmCodeByTCPIP(bool bRead);                          //ChungHung 20150518 add for SCK Send JamCode By TcpIp
void ProcessLastSetIni_InOutArm(bool bRead);
void ProcessLastSetIni_Monitor(bool bRead);

void FUNC_CC_SCK();
void FUNC_CC_RICHTEK();                                                         //Alick 20161206 add 立錡
int GetSiteCount(bool IncludeCloseSite=true);
extern int GetColorSensorOnLoaderByMUN();                                       //Jimmychiu 20230630 : add color sensor MU-N in Loader
extern bool GetColorSensorIsMapping(AnsiString &sErrorMsg);                     //Jimmychiu 20230630 : add color sensor MU-N in Loader
typedef struct                                                                  //Jimmychiu 20240916 : Check the temperature during index arm testing
{
    int  iStartTest;
    bool bArmTempErr;
    bool bSuckTempErr[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    void Clear();
    void SetStartTest(int iArm);
    void SetTempErr(int iSuckRow, int iSuckCol);
    bool GetArmTempErr();
    bool GetSuckTempErr(int iSuckRow, int iSuckCol);
}MyIndexArmRec;                                                                 //Jimmychiu 20240916 : Check the temperature during index arm testing
extern MyIndexArmRec myIAR_Test;
extern TDateTime dtStartLot;
extern TDateTime dtEndLot;
extern bool bNeedManualCheckEmptyTray;                                          //Jimmychiu 20250826 : Open Door Check Loader After TrayEnd
extern bool bNeedOneByOnePickInArm;                                             //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
class TAlarm1                                                                   //Stevenhong 20260318 : TESNA 把Eventlog report summarize by month
{
    public:
        AnsiString FullRow; // 用來存放符合條件的「整行原始資料」
        void __fastcall SummarizeJAMreportbymonth();
};

extern TQPF_Timer tP62MachineStopTimer;                                         //Jimmychiu 20251205 : First Tray Check On Unloader
#endif
