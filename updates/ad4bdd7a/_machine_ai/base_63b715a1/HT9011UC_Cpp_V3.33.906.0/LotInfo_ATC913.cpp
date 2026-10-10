// =============================================================================
//  LotInfo_ATC913.cpp -- W-224 POOL-13 MR-A (St02-E): golden 913 TfLotInfo::SetATCOffset + ConvertPackageOffset
//
//  AI(W906-POOL13) 20261010 [W906] (St02-E): new file in ht9045_sm (CMakeLists.txt:2501, same line), as LotInfo_E020.cpp /
//    forms/fMain_ATCSiteUse.cpp: the bodies need sm objects (ATC_InterfaceForm shim, fTemp_Set, fContact, GetFactSetTemp,
//    FTestSuck), and ht9045_forms (forms/fLotInfo.cpp) must not link up to ht9045_sm.  The member is declared in
//    forms/fLotInfo.h:1690.
//  golden = 913 D:\HT9045\HT9011UC_Code_V3.33.913.0_20261008_steven\uLotInfo.cpp (cp950, not in git).  RULINGS_20261008 #3
//    (new changes follow 913) / RULINGS_20261009 #6 (913 temperature features translated as 913).  The body below is the golden
//    text line by line (generated from golden 913 by St02-E's script, every changed golden line listed here):
//      golden 913 :5586-5587   HTimer ATCOFSDelay / ATCPreOFSDelay[32] -- as vclcompat::HTimer (atester_shims.h:467 declares a
//                              global fake `struct HTimer` whose Off() is always true -- this file must never use the bare name)
//      golden 913 :9643-9694   ConvertPackageOffset() -- verbatim
//      golden 913 :9696-10202  TfLotInfo::SetATCOffset -- verbatim (0618 :9405-9870 / V912 :9586-10051 are the same body without
//                              the three JerryYang 20260917 L07 blocks :9802-9829 / :9870-9875 / :10065-10071), except:
//        :9720 / :9786 / :10097  fContact->fShow -> W906_FormShowing("fContact", fContact->fShow); :10097 fContact->CarlibrationTask
//                                -> fContactForm->CarlibrationTask (W-224 Q5: the shim has no CarlibrationTask)
//        :9771 / :9920 / :10159 / :10164 / :10170 / :10180 / :10188 / :10193  the ATC sends (SetOffset / SetTC2Offset /
//                                SetMultiSensorOffset / Set2ndFunction) stay in `#if 0 // GATE(W906-POOL13-ATCSEND)`; the line
//                                after each gate records what golden would send (W906_ATC913_Rec, LotInfo_ATC913.h) -- W-224 Q3
//                                "compute + record, not sent".  Same posture as csystem.cpp:27137-27156 (GATE H3-5) and
//                                forms/fMain_ATCSiteUse.cpp G3-G5.  Nothing reaches an ATC.
//        after :10063            GOLDEN BUG guard 1 (W-224 Q4=B): a site whose j1 / j2 is outside 0..31 is skipped
//        :10143-10144            GOLDEN BUG guard 2 (W-224 Q4=B): TC2 index j3+8 / j4+8 >= 32 is not read
//  ATC_MAX_SITE here is golden ATC_Handler_Side.h:17's 40 (forms/fATCHandlerSide.h:684) -- NOT TesterComm/Gpib/GpibBridge.h:64's
//    `const int ATC_MAX_SITE = 32` (the two headers cannot share a TU; Tri_Temp_Machine writes dbATC_Offset[32..39]).
//  ATC_InterfaceForm->iATC_MODE_TYPE is the shim's field (acarry_shims.h:109-115, offline 0), so the ATC 7.0 branch is reachable
//    only when a test sets it.
//
//  NO LIVE CALLER (W-224, MR-C is a separate card): every golden 913 caller is still gated or unported in the port --
//    Command.cpp:2111-2113, aTester_Front.cpp:3743-3745, aTester_Rear.cpp:3612-3614, atester.cpp:2428-2430,
//    atester.cpp:12450-12454 (DoCheckHasTestTempChange, empty), uTemp_Set.cpp:6530-6532; NetATCTimeTimer / HS_Function
//    TESTTEMPSETTING / ShowNewATCThermo are not ported.  Only tests/test_st02_w224_atcoffset.cpp calls it.
//  Not verified on a real machine.  HT9050: USE_ATC_MODE=0 (ATC_SYSTEM=eATCUninstall), CUSTOMER_CODE=957 (CC_PTI:
//    bUseSecondATCTempOffset false) -- and no caller anyway.
// =============================================================================
#include "LotInfo_ATC913.h"
#include "forms/fLotInfo.h"         // TfLotInfo (SetATCOffset is a member, declared at :1690)
#include "MachineType.h"            // ATC_HEAD_COUNT, eNewATCSystem, eTestMode (SingleSite / DualSite / _16Site2X8 ...), Tempture_*, CC_ASE_M, rsmQAMode
#include "cmydef.h"                 // ATC_SYSTEM, iATC_Use_Heat_Count, IndexStatus, iWhichArmDown, bATC_EnableSiteMap, iATCRemoteChangeTempCnt ...
#include "cprod.h"                  // Temperature, TestIF, TestIF_File
#include "Config.h"                 // IniConfig.dATCAmbientTemperature
#include "CosFunction.h"            // CosFunction.bUseSecondATCTempOffset / bATCUseTempAdjustment / bATCUsePackageOffset / bHiSiliconFunction / bATC32UseTJMode
#include "LastSet.h"                // LastSet.iTemperature / iRunStartMode
#include "bthermo.h"                // GetFactSetTemp (golden bthermo.h:18)
#include "aHotPlateSubstrate.h"     // FTestSuck / BTestSuck (golden MyKitSuck.h)
#include "acarry_shims.h"           // ATC_InterfaceForm (the shim: iATC_MODE_TYPE only)
#include "forms/fATCHandlerSide.h"  // ATC_MAX_SITE (40), ATC_TYPE_33 / _60 / _70
#include "forms/fTemp_Set.h"        // fTemp_Set (iSiteToATC / iSiteToOfs / ControlATC60AirFlow), InitTempOffset
#include "forms/fContact.h"         // fContactForm->CarlibrationTask (W-224 Q5)
#include "cContact.h"               // CONTACT_NORMAL (golden cContact.cpp:74)
#include "atester_shims.h"          // fContact (shim, :251) -- also declares a global fake `struct HTimer`; see above
#include "W906FormShowing.h"        // W906_FormShowing
#include <windows.h>                // ZeroMemory
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern bool bUnderTest;             // golden main.h:1726 `extern bool bEcho, bUnderTest;` -- defined atester_shims.cpp:101 (as Interface/InterfaceSYS.cpp:74)

static_assert(ATC_MAX_SITE == W906_ATC913_REC_MAX, "LotInfo_ATC913.h W906_ATC913_REC_MAX must equal golden ATC_MAX_SITE (40)");

// ---- the recorder (W-224 Q3): what golden would have sent to the ATC ----------------------------------------------------------
W906_ATC913_Recorder W906_ATC913_Rec;

void W906_ATC913_ResetRecorder()
{
    std::memset(&W906_ATC913_Rec, 0, sizeof(W906_ATC913_Rec));
}

void W906_ATC913_Record(int iKind, int iChCount, const double *pData, int iDataLen)
{
    if(iKind<0 || iKind>=W906_ATC913_KINDS)
        return;
    W906_ATC913_Rec.iCount[iKind]++;
    W906_ATC913_Rec.iChCount[iKind]=iChCount;
    for(int i=0; i<W906_ATC913_REC_MAX; i++)
        W906_ATC913_Rec.dLast[iKind][i]=(pData!=0 && i<iDataLen) ? pData[i] : 0.0;
    std::printf("[W906-POOL13] ATC send not sent (GATE W906-POOL13-ATCSEND): kind %d, ch %d\n", iKind, iChCount);
}

void W906_ATC913_RecordBool(bool bEnabled)
{
    W906_ATC913_Rec.iCount[W906_ATC913_SET2NDFUNCTION]++;
    W906_ATC913_Rec.bLast2nd=bEnabled;
    std::printf("[W906-POOL13] ATC send not sent (GATE W906-POOL13-ATCSEND): Set2ndFunction(%d)\n", bEnabled ? 1 : 0);
}

void W906_ATC913_NoteGuard(int iGuard, int iRow, int iCol, int iIndex)
{
    if(iGuard==1)
        W906_ATC913_Rec.iGuardSite++;
    else
        W906_ATC913_Rec.iGuardTc2++;
    std::printf("[W906-POOL13] GOLDEN BUG guard %d: row %d col %d index %d skipped\n", iGuard, iRow, iCol, iIndex);
}

// ---- golden 913 uLotInfo.cpp:5586-5587 (W-224) --------------------------------------------------------------------------------
vclcompat::HTimer ATCOFSDelay;                                                  // golden 913 :5586
vclcompat::HTimer ATCPreOFSDelay[32];                                           // golden 913 :5587  //JerryYang 20260917 : 測試中變溫預先補償持續時間(依ATC硬體位置)

//---------------------------------------------------------------------------
// ---- golden 913 uLotInfo.cpp:9643-9694 (W-224): ConvertPackageOffset, verbatim ------------------------------------------------

double ConvertPackageOffset()                                                   //Ifor 20190306 : add Package Offset 三點校正
{
    int iLowBase =Temperature.dATCPackageTemp[0];
    int iHighBase=Temperature.dATCPackageTemp[1];
    double m, s, Temp[5]={0.0};                                                 //kevin 20141006 Temp[3]->Temp[5]
    int ct1, ct2;
    char str[256];

    double dbSetATCTemp=0;

    if(LastSet.iTemperature==Tempture_Hot ||
       LastSet.iTemperature==Tempture_AmbientHot)                               //kevin 20140918 恆溫控制
        dbSetATCTemp=Temperature.fWorkTemperBase;                               //Ifor 20160111 : ATC高溫的設定溫度
    else
        dbSetATCTemp=IniConfig.dATCAmbientTemperature;                          //Ifor 20160111 : [L11] ATC常溫的設定溫度

    if(dbSetATCTemp>=Temperature.dATCPackageTemp[1])
    {
        iLowBase=Temperature.dATCPackageTemp[1];
        iHighBase=Temperature.dATCPackageTemp[2];
        ct1=1;
        ct2=2;
    }
    else
    {
        iHighBase=Temperature.dATCPackageTemp[1];
        ct1=0;
        ct2=1;
    }

    if(dbSetATCTemp==Temperature.dATCPackageTemp[0])
        return (Temperature.dATCPackageOffset[0]+Temperature.dATCPackageTemp[0])-dbSetATCTemp;
    if(dbSetATCTemp==Temperature.dATCPackageTemp[1])
        return (Temperature.dATCPackageOffset[1]+Temperature.dATCPackageTemp[1])-dbSetATCTemp;
    if(dbSetATCTemp==Temperature.dATCPackageTemp[2])
        return (Temperature.dATCPackageOffset[2]+Temperature.dATCPackageTemp[2])-dbSetATCTemp;

    Temp[0]=Temperature.dATCPackageOffset[0]+Temperature.dATCPackageTemp[0];
    Temp[1]=Temperature.dATCPackageOffset[1]+Temperature.dATCPackageTemp[1];
    Temp[2]=Temperature.dATCPackageOffset[2]+Temperature.dATCPackageTemp[2];

    if((iHighBase-iLowBase)==0)
        m=0;
    else
        m=(double)(iHighBase-dbSetATCTemp)/(double)(iHighBase-iLowBase);

    if(m==0)
        return dbSetATCTemp-dbSetATCTemp;
    s=Temp[ct2]+(Temp[ct1]-Temp[ct2])*m;
    sprintf(str,"%4.1f",s);
    return atof(str)-dbSetATCTemp;
}
//---------------------------------------------------------------------------
// ---- golden 913 uLotInfo.cpp:9696-10202 (W-224): TfLotInfo::SetATCOffset ------------------------------------------------------

void TfLotInfo::SetATCOffset(bool bFirstSetOffset, bool bOFSClose)              //Ifor 20241118 : 測試中變溫
{
    double dbATC_Offset[ATC_MAX_SITE];                                          //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    int j1, j2, j3, j4, iCol=0;
    double dbATC_InitTempOffset[ATC_HEAD_COUNT];
    double dbATCOffstBuffer[32];
    double dbATC_TC2Offset[ATC_HEAD_COUNT];
    int iSiteMapping_InitTempOffset[]   ={11,15,12,16,13,17,14,18,33,37,34,38,35,39,36,40,19,23,20,24,21,25,22,26,41,45,42,46,43,47,44,48};
    ZeroMemory(dbATC_Offset, sizeof(dbATC_Offset));
    ZeroMemory(dbATC_InitTempOffset, sizeof(dbATC_InitTempOffset));
    ZeroMemory(dbATCOffstBuffer, sizeof(dbATCOffstBuffer));
    ZeroMemory(dbATC_TC2Offset, sizeof(dbATC_TC2Offset));                       //KenHsieh 20240311 : add Tc2 Offset
    static bool bFirstChange=false;
    int iTotal_Channel;                                                         //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    double dTemp=0;
    double dATC_MultiOfs[32];

    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70)                          //Ifor 20160429 add New ATC System Offset Function from ATC 7.0
    {
        for(int iAtc=0; iAtc<4; iAtc++)
        {
            if(LastSet.iTemperature==Tempture_Hot ||
               LastSet.iTemperature==Tempture_AmbientHot)                       //kevin 20180811 (Steven) : add 恆溫控制
            {
                if(W906_FormShowing("fContact", fContact->fShow) && iContactMode!=CONTACT_NORMAL)   // [W906] W-224 Q5: golden fContact->fShow (fContact is the atester_shims.h shim; the web page counts too, as uYieldMonitoring.cpp:3586)
                {
                    dbATC_Offset[iAtc]=Temperature.dATCInPC[iAtc];              //Steven 20151123 : Contact Mode no need another offset
                }
                else if(Temperature.bEnableATCTestTimeOffset &&                 //Steven 20160216 : 測試時間太短也要Offset
                        dTestSec<Temperature.iATCTestTimeOffsetTime)
                {
                    dbATC_Offset[iAtc]=Temperature.dATCInPC[iAtc]+Temperature.dATCTestTimeOffset[iAtc];
                }
                else if(Temperature.bEnableTempOffsetForInitial &&              //Steven 20141117 : 起測時溫度要補Offset
                        iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)
                {
                    if(CUSTOMER_CODE==CC_ASE_M)                                 //Ifor 20170310 (wei) add ASEM ATC7.0 Initial Offset 溫度補償僅在Delay Time 補溫
                    {
                        if(Temperature.iCintactCntForTempOffsetAtInitial==1)
                        {
                            if(bNeedInitialTestDelay==true)
                                dbATC_Offset[iAtc]=Temperature.dATCInPC[iAtc]+Temperature.ATCInitialOffset[iAtc];
                            else
                                dbATC_Offset[iAtc]=Temperature.dATCInPC[iAtc];
                        }
                        else
                        {
                            dbATC_Offset[iAtc]=Temperature.dATCInPC[iAtc]+Temperature.ATCInitialOffset[iAtc];           //Steven 20151006 : Initial Temp Offset for ATC
                        }
                    }
                    else
                    {
                        dbATC_Offset[iAtc]=Temperature.dATCInPC[iAtc]+Temperature.ATCInitialOffset[iAtc];               //Steven 20151006 : Initial Temp Offset for ATC
                    }
                }
                else if(Temperature.bEnableATCConFailOffset &&                  //Steven 20151123 : Continue Fail Temp Offset for ATC
                        Temperature.iATCCurrentFailCount[iAtc]>=Temperature.iATCConFailOffsetCount)                     //Steven 20151209 : Modify for ATC 7.0
                {
                    dbATC_Offset[iAtc]=Temperature.dATCConFailOffset[iAtc];
                }
                else if(LastSet.iRunStartMode==rsmQAMode &&                     //Steven 20151125 : QA Mode Temp Offset for ATC
                        Temperature.bEnableATCQAModeOffset)
                {
                    dbATC_Offset[iAtc]=Temperature.dATCQAModeOffset[iAtc];
                }
                else
                {
                    dbATC_Offset[iAtc]=Temperature.dATCInPC[iAtc];              //Steven 20151112 : 修改ATC7.0 Offset
                }
            }
            else
            {
                dbATC_Offset[iAtc]=Temperature.dATCInPC[iAtc];
            }
        }
#if 0 // GATE(W906-POOL13-ATCSEND) golden 913 uLotInfo.cpp:9771 -- W-224 Q3: the ATC send is not wired (acarry_shims.h:109 shim has iATC_MODE_TYPE only; forms/fATCHandlerSide.h keeps the send undefined) -- recorded, not sent
        ATC_InterfaceForm->SetOffset(iATC_Use_Heat_Count, dbATC_Offset);        //TfLotInfo::SetATCOffset for ATC_TYPE_70
#endif // GATE(W906-POOL13-ATCSEND)
        W906_ATC913_Record(W906_ATC913_SETOFFSET, iATC_Use_Heat_Count, dbATC_Offset, ATC_MAX_SITE);   // AI(W906-POOL13) W-224: what golden :9771 would send
    }
    else
    {
        if(bFirstSetOffset==true || bNeedInitialTestDelay)
        {
            if(Temperature.bEnableTempOffsetForInitial==true)
            {
                for(int i=0; i<32; i++)
                {
                    dbATC_InitTempOffset[i]=Temperature.fTempOffSet[InitTempOffset][iSiteMapping_InitTempOffset[i]];
                }
            }

            int iNowDownArm=0;
            if(W906_FormShowing("fContact", fContact->fShow))                                                 //Ifor 20240612 add:避免Contact Mode 資料回覆錯誤
            {
                if(iWhichArmDown==1)
                {
                    iNowDownArm=Z1Down_Z2Up;
                }
                else if(iWhichArmDown==2)
                {
                    iNowDownArm=Z1Up_Z2Down;
                }
            }
            else
            {
                iNowDownArm=IndexStatus;
            }

            if(CosFunction.bUseSecondATCTempOffset==true &&                     //JerryYang 20260917 : 測試中變溫使用預先補償(依Site設定)
               Temperature.bATCPreOffset==true &&
               (bChangeTest_TempOffset!=0 || iATCRemoteChangeTempCnt>1))
            {
                for(int i=0; i<32; i++)
                {
                    dbATCOffstBuffer[i]=Temperature.dATCTempOffset[i];

                    if((iNowDownArm==Z1Down_Z2Up && i<16) ||
                       (iNowDownArm==Z1Up_Z2Down && i>=16))
                    {
                        if(iATCRemoteChangeTempCnt<=1)
                        {
                            if(ATCPreOFSDelay[i].Off() && bATC_EnableSiteMap[i]==true && bOFSClose==true)
                                dbATCOffstBuffer[i]=Temperature.dATCAfterOfs[i];
                            else
                                dbATCOffstBuffer[i]=Temperature.dATCPreOffset[i];
                        }
                        else
                        {
                            if(ATCPreOFSDelay[i].Off() && bATC_EnableSiteMap[i]==true && bOFSClose==true)
                                dbATCOffstBuffer[i]=Temperature.d2ndATCAfterOfs[i];
                            else
                                dbATCOffstBuffer[i]=Temperature.d2ndATCPreOffset[i];
                        }
                    }
                }
            }
            else if(CosFunction.bUseSecondATCTempOffset==true && bChangeTest_TempOffset!=0)
            {
                if(iNowDownArm==Z1Down_Z2Up)
                {
                    for(int i=0; i<16; i++)
                    {
                        if(bOFSClose==true)                                     //Arm 1
                            dbATCOffstBuffer[i]=0;
                        else
                            dbATCOffstBuffer[i]=Temperature.dATCSecondTempOffset[i];

                        dbATCOffstBuffer[i+16]=Temperature.dATCTempOffset[i+16];                                        //Arm 2
                    }
                }
                else if(iNowDownArm==Z1Up_Z2Down)
                {
                    for(int i=0; i<16; i++)
                    {
                        dbATCOffstBuffer[i]=Temperature.dATCTempOffset[i];      //Arm 1

                        if(bOFSClose==true)                                     //Arm 2
                            dbATCOffstBuffer[i+16]=0;
                        else
                            dbATCOffstBuffer[i+16]=Temperature.dATCSecondTempOffset[i+16];
                    }
                }
                else
                {
                    for(int i=0; i<32; i++)
                        dbATCOffstBuffer[i]=Temperature.dATCTempOffset[i];
                }
            }
            else
            {
                for(int i=0; i<32; i++)
                {
                    dbATCOffstBuffer[i]=Temperature.dATCTempOffset[i];
                }
            }

            if(CosFunction.bUseSecondATCTempOffset==true &&                     //JerryYang 20260917 : 測試中變溫預先補償不走三點校正的等待
               Temperature.bATCPreOffset==true &&
               (bChangeTest_TempOffset!=0 || iATCRemoteChangeTempCnt>1))
            {
                bFirstSetOffset=false;
            }
            else if(CosFunction.bATCUseTempAdjustment==true)                    //Ifor 20190215 : add ATC 使用 三點校正功能
            {
                if(bATCTempAdjustmentOffset==true)                              //Ifor 20190306 : add Package Offset 三點校正
                {
                    if(bFirstChange==false)                                     //重新等待計算，避免Offset 尚未更新導致補錯溫度
                    {
                        bATCTempAdjustmentOffset=false;
                        bFirstChange=true;
                        return;
                    }
                    else
                    {
                        Temperature.dATCPackageOffsettemp=ConvertPackageOffset();
                        bFirstSetOffset=false;
                        bFirstChange=false;
                    }
                }
                else
                {
                    return;
                }
            }
            else
            {
                bFirstSetOffset=false;
            }

            if(Tri_Temp_Machine==1)                                             //Ztex 2023.04.19 Add HT-1032 TriTemp Function
            {
                iTotal_Channel=TriTemperature_TotalChannel;                     //設定 TotalChannel
                int iATC_Channel[8]={2,3,0,1,65,66,63,64};
                for(int i=0; i<8; i++)
                {
                    dTemp=GetFactSetTemp(iATC_Channel[i], Temperature.fWorkTemperBase);
                    dbATC_Offset[i+32]=dTemp-Temperature.fWorkTemperBase;
                }
            }
            else
            {
                iTotal_Channel=iATC_Use_Heat_Count;
            }

            if(CosFunction.bHiSiliconFunction==true)                            //Ifor 20151216 海思專用版本 Offset 為0
            {
#if 0 // GATE(W906-POOL13-ATCSEND) golden 913 uLotInfo.cpp:9920 -- W-224 Q3: the ATC send is not wired (acarry_shims.h:109 shim has iATC_MODE_TYPE only; forms/fATCHandlerSide.h keeps the send undefined) -- recorded, not sent
                ATC_InterfaceForm->SetOffset(iTotal_Channel, dbATC_Offset);     //TfLotInfo::SetATCOffset for HiSilicon
#endif // GATE(W906-POOL13-ATCSEND)
                W906_ATC913_Record(W906_ATC913_SETOFFSET, iTotal_Channel, dbATC_Offset, ATC_MAX_SITE);   // AI(W906-POOL13) W-224: what golden :9920 would send
            }
            else
            {
                bool bUse1by2Heat=false;                                        //Ifor 20180505 (Steven) : add 一對二加熱器判斷
                if((TestIF_File.iTestMode==_12Site2X6     ||
                    TestIF_File.iTestMode==_16Site2X8     ||
                    TestIF_File.iTestMode==_16Site4X4     ||                    //kevin 20190516 add _16Site4X4
                    TestIF_File.iTestMode==_32Site4X8N)   &&
                    TestIF_File.bUse32Heater==false         )
                {
                    iCol=FTestSuck.iShtCol/2;
                    bUse1by2Heat=true;
                }

                if(TestIF.iTestMode==SingleSite && Temperature.bMultiZoneEnable)                                        //wei 20240617 Multi Zone
                {
                    iCol=4;
                }
                else if((TestIF_File.iTestMode==_8Site2X4 ||
                    TestIF_File.iTestMode==_16Site4X4) &&                       //Steven 20240425 add _16Site4X4
                   TestIF_File.bOctal_16Kit)                                    //JerryYang 20230204 : 修正8 site mode使用16site SLK溫度offset異常
                {
                    iCol=8;
                }
                else if((TestIF_File.iTestMode==_8Site2X4 ||
                         TestIF_File.iTestMode==_16Site4X4) &&                  //Steven 20240425 add _16Site4X4
                        TestIF_File.bOctal_12Kit)
                {
                    iCol=6;
                }
                else                                                            //JerryYang 20250225 : fixed for ATC offset
                {
                    iCol=FTestSuck.iShtCol;
                }

                bool bIsSTMMode=false;
                if(TestIF_File.iTestMode==_16Site2X8 && TestIF_File.bUse32Heater==false &&
                   ((TestIF_File.iSiteMap[0][1]==0 && TestIF_File.iSiteMap[0][3]==0 && TestIF_File.iSiteMap[0][5]==0 && TestIF_File.iSiteMap[0][7]==0 &&
                   TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[1][2]==0 && TestIF_File.iSiteMap[1][4]==0 && TestIF_File.iSiteMap[1][6]==0) ||
                   (TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[0][2]==0 && TestIF_File.iSiteMap[0][4]==0 && TestIF_File.iSiteMap[0][6]==0 &&
                   TestIF_File.iSiteMap[1][1]==0 && TestIF_File.iSiteMap[1][3]==0 && TestIF_File.iSiteMap[1][5]==0 && TestIF_File.iSiteMap[1][7]==0)))
                {
                    bIsSTMMode=true;
                }

            /*    if(iATC_Use_Heat_Count==8 &&                                  //Steven 20231017 : Fixed ATC 3.x temp offset
                   IsNNMode()==NN_1Row)
                {
                    int iJ1Arm[2][4]={{0, 1, 2, 3},                             //2x2 and 2x3NN mode to 8ch ATC
                                      {4, 5, 6, 7}};

                    int iJ3Arm[2][8]={{ 0,  1,  2,  3,  4,  5,  6,  7},         //2x2 and 2x3NN mode offset to 8ch ATC
                                      {16, 17, 18, 19, 20, 21, 22, 23}};

                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<iCol; j++)
                        {
                            if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) &&                          //Steven 20180817 : Boost Function
                               iTriggerBoostFunction!=-1 &&
                               iBoostFuncStep==0)
                            {
                                if(Temperature.iBoostFunctionMode==2)
                                {
                                    dbATC_Offset[iJ1Arm[i][j]]=Temperature.dATCTempOffset[iJ3Arm[i][j]]+Temperature.dIndexATCSecondTempOffset[iJ3Arm[i][j]]+Temperature.dBoostOffset[iTriggerBoostFunction];
                                }
                                else if((IndexStatus==Z1Up_Z2Down && BTestSuck.AlreadyTest()==false) ||
                                        FTestSuck.AlreadyTest()  ||
                                        (fContact->fShow && iContactMode!=CONTACT_NORMAL && fContact->CarlibrationTask==800))
                                {
                                    dbATC_Offset[iJ1Arm[i][j]]=Temperature.dATCTempOffset[iJ3Arm[i][j]]+Temperature.dIndexATCSecondTempOffset[iJ3Arm[i][j]];
                                }
                                else
                                {
                                    dbATC_Offset[iJ1Arm[i][j]]=Temperature.dATCTempOffset[iJ3Arm[i][j]]+Temperature.dIndexATCSecondTempOffset[iJ3Arm[i][j]]+Temperature.dBoostOffset[iTriggerBoostFunction];
                                }
                            }
                            else if(Temperature.bEnableTempOffsetForInitial &&  //Steven 20180815 : ATC起測時溫度要補Offset
                                   iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)
                            {
                                dbATC_Offset[iJ1Arm[i][j]]=Temperature.dATCTempOffset[iJ3Arm[i][j]]+Temperature.fTempOffSet[InitTempOffset][iJ3Arm[i][j]];      //Ifor 20160523 Arm1 陣列起始位置00
                            }
                            else
                            {
                                if(Temperature.bBoostFuncttion || Temperature.bLBTempFunction)
                                {
                                    dbATC_Offset[iJ1Arm[i][j]]=Temperature.dATCTempOffset[iJ3Arm[i][j]]+Temperature.dIndexATCSecondTempOffset[iJ3Arm[i][j]];    //Ifor 20160523 Arm1 陣列起始位置00
                                }
                                else
                                {
                                    dbATC_Offset[iJ1Arm[i][j]]=Temperature.dATCTempOffset[iJ3Arm[i][j]];                //Ifor 20160523 Arm1 陣列起始位置00
                                }
                            }

                            if(CosFunction.bATCUseTempAdjustment==true)
                            {
                                dbATC_Offset[iJ1Arm[i][j]]=dATCTempAdjustmentOffset[iJ3Arm[i][j]];
                            }

                            if(CosFunction.bATCUsePackageOffset==true)
                            {
                                dbATC_Offset[iJ1Arm[i][j]]=dbATC_Offset[iJ1Arm[i][j]]+Temperature.dATCPackageOffsettemp;
                            }
                        }
                    }
                }
                else       */
                {                                                               //Steven 20250716 : Rework for ATC temp offset
                    for(int i=0; i<FTestSuck.iShtRow; i++)                      //Ifor 20160523 ATC 4、8、32 Site OffSet 整合
                    {
                        for(int j=0; j<iCol; j++)                               //JerryYang 20250225 : fixed for ATC offset
                        {
                            j3=fTemp_Set->iSiteToOfs[0][i][j];
                            j4=fTemp_Set->iSiteToOfs[1][i][j];
                            //          Arm1                    Arm2
                            //00 01 02 03 04 05 06 07   16 17 18 19 20 21 22 23
                            //08 09 10 11 12 13 14 15   24 25 26 27 28 29 30 31
                            if(FTestSuck.iShtRow==1 &&
                               ATC_SYSTEM==eNewATCSystem &&
                               iATC_Use_Heat_Count==32 &&
                               TestIF.iTestMode==DualSite)
                            {
                                j1=j*2+i;
                                j2=j*2+i+(iATC_Use_Heat_Count/2);
                            }
                            else if(ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count==8 && TestIF.iTestMode==DualSite)
                            {
                                j1=j;
                                j2=j+(iATC_Use_Heat_Count/2);

                                j3=j*8;
                                j4=j*8+16;
                            }
                            else if(bUse1by2Heat==true && bIsSTMMode==false)    //JerryYang 20251028 : fix 交錯型 ATC offset錯誤//JimmyChiu 20211026 : #P211001-ATK-H9-02 , V3.21.701.1 , Different Temp Offset site on Temperature Page for x16 & x8 heater SLK on HT9046AT.
                            {
                                j1=j3=(j*2)+(i*8);
                                j2=j4=j1+(iATC_Use_Heat_Count/2);
                            }
                            else
                            {
                                j1=fTemp_Set->iSiteToATC[0][i][j];
                                j2=fTemp_Set->iSiteToATC[1][i][j];
                            }
                            // GOLDEN BUG (W-224, RULINGS_20261010 #5 Q4=B, guard 1; deviation approved by the laptop): golden 913 :10061-10062 take
                            //   j1 / j2 from fTemp_Set->iSiteToATC, which TfTemp_Set::InitialAddrToATC (uTemp_Set.cpp:6809-6810) leaves at -1 wherever
                            //   the kit has no ATC channel -- e.g. USE_16_HEATER==eht4Heater (the cmydef.cpp default) with a 1x4 kit: only columns 0-1
                            //   get a channel (uTemp_Set.cpp:6827), columns 2-3 stay -1.  Golden then writes dbATC_Offset[-1] / dbATC_TC2Offset[-1] (off
                            //   the local arrays, onto the stack) and reads dbATC_InitTempOffset[-1] / dGPIBATCOffset[-1].  Here such a site is skipped
                            //   (nothing is written for it, even when only one of j1 / j2 is bad); every in-range site is computed exactly as golden.
                            //   Measured: tests/test_st02_w224_atcoffset.cpp [G1].
                            if(j1<0 || j1>=ATC_HEAD_COUNT || j2<0 || j2>=ATC_HEAD_COUNT)
                            {
                                W906_ATC913_NoteGuard(1, i, j, (j1<0 || j1>=ATC_HEAD_COUNT) ? j1 : j2);
                                continue;
                            }

                            if(CosFunction.bUseSecondATCTempOffset==true &&    //JerryYang 20260917 : 測試中變溫預先補償直接套用
                               Temperature.bATCPreOffset==true &&
                               (bChangeTest_TempOffset!=0 || iATCRemoteChangeTempCnt>1))
                            {
                                dbATC_Offset[j1]=dbATCOffstBuffer[j3];
                                dbATC_Offset[j2]=dbATCOffstBuffer[j4];
                            }
                            else if(TestIF.iTestMode==SingleSite &&
                               Temperature.bMultiZoneEnable)
                            {
                                dbATC_Offset[j1]=dbATCOffstBuffer[j1];
                                dbATC_Offset[j2]=dbATCOffstBuffer[j1+16];
                            }
                            else if(CosFunction.bATCUseTempAdjustment==true)
                            {
                                dbATC_Offset[j1]=dATCTempAdjustmentOffset[j3];
                                dbATC_Offset[j2]=dATCTempAdjustmentOffset[j4];
                            }
                            else if((Temperature.bBoostFuncttion ||
                                     Temperature.bLBTempFunction) &&            //Steven 20180817 : Boost Function
                                    iTriggerBoostFunction!=-1 &&
                                    iBoostFuncStep==0 &&
                                    bUnderTest==false)
                            {
                                if(Temperature.iBoostFunctionMode==2)
                                {
                                    dbATC_Offset[j1]=dbATCOffstBuffer[j3]+Temperature.dIndexATCSecondTempOffset[j3]+Temperature.dBoostOffset[iTriggerBoostFunction];
                                    dbATC_Offset[j2]=dbATCOffstBuffer[j4]+Temperature.dIndexATCSecondTempOffset[j4]+Temperature.dBoostOffset[iTriggerBoostFunction];
                                }
                                else if((IndexStatus==Z1Up_Z2Down &&
                                         BTestSuck.AlreadyTest()==false) ||
                                        FTestSuck.AlreadyTest()  ||
                                        (W906_FormShowing("fContact", fContact->fShow) && iContactMode!=CONTACT_NORMAL && fContactForm->CarlibrationTask==800))   // [W906] W-224 Q5: CarlibrationTask lives on the real TfContact (forms/fContact.h:1502, fContactForm :1634); the shim has none
                                {
                                    dbATC_Offset[j1]=dbATCOffstBuffer[j3]+Temperature.dIndexATCSecondTempOffset[j3];
                                    dbATC_Offset[j2]=dbATCOffstBuffer[j4]+Temperature.dIndexATCSecondTempOffset[j4]+Temperature.dBoostOffset[iTriggerBoostFunction];
                                }
                                else
                                {
                                    dbATC_Offset[j1]=dbATCOffstBuffer[j3]+Temperature.dIndexATCSecondTempOffset[j3]+Temperature.dBoostOffset[iTriggerBoostFunction];
                                    dbATC_Offset[j2]=dbATCOffstBuffer[j4]+Temperature.dIndexATCSecondTempOffset[j4];
                                }
                            }
                            else if(Temperature.bEnableTempOffsetForInitial &&  //Steven 20180815 : ATC起測時溫度要補Offset
                                    iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)
                            {
                                dbATC_Offset[j1]=dbATCOffstBuffer[j3]+dbATC_InitTempOffset[j1];
                                dbATC_Offset[j2]=dbATCOffstBuffer[j4]+dbATC_InitTempOffset[j2];
                            }
                            else
                            {
                                if(Temperature.bBoostFuncttion || Temperature.bLBTempFunction)
                                {
                                    dbATC_Offset[j1]=dbATCOffstBuffer[j3]+Temperature.dIndexATCSecondTempOffset[j3];
                                    dbATC_Offset[j2]=dbATCOffstBuffer[j4]+Temperature.dIndexATCSecondTempOffset[j4];
                                }
                                else
                                {
                                    dbATC_Offset[j1]=dbATCOffstBuffer[j3];
                                    dbATC_Offset[j2]=dbATCOffstBuffer[j4];
                                }
                            }

                            if(CosFunction.bATCUsePackageOffset==true)
                            {
                                dbATC_Offset[j1]=dbATC_Offset[j1]+Temperature.dATCPackageOffsettemp;
                                dbATC_Offset[j2]=dbATC_Offset[j2]+Temperature.dATCPackageOffsettemp;
                            }

                            if(bGPIBOffsetCommand)
                            {
                                dbATC_Offset[j1]=dbATC_Offset[j1]+dGPIBATCOffset[j1];
                                dbATC_Offset[j2]=dbATC_Offset[j2]+dGPIBATCOffset[j2];
                            }

                            if(Temperature.bUseReferTempSensor &&
                               Temperature.bUseTC2Offset)                       //KenHsieh 20240311 : add Tc2 Offset
                            {
                                // GOLDEN BUG (W-224, RULINGS_20261010 #5 Q4=B, guard 2; deviation approved by the laptop): golden 913 :10143-10144 read
                                //   Temperature.dATCTempOffset[j3+8] / [j4+8], but dATCTempOffset is [32] (cprod.h:1472) and j4=(j+16)+(i*8) (uTemp_Set.cpp:6813),
                                //   so on a 2-row kit the second row of arm 2 reads [32+j] -- past the array into the next SYSTEM_TEMPERATURE fields.  Here an index
                                //   >= 32 is not read (that TC2 offset stays 0, its ZeroMemory value).  Measured: tests/test_st02_w224_atcoffset.cpp [G2].
                                //   golden 913 :10143  dbATC_TC2Offset[j1]=Temperature.dATCTempOffset[j3+8];
                                //   golden 913 :10144  dbATC_TC2Offset[j2]=Temperature.dATCTempOffset[j4+8];
                                if(j3+8<32) dbATC_TC2Offset[j1]=Temperature.dATCTempOffset[j3+8]; else W906_ATC913_NoteGuard(2, i, j, j3+8);
                                if(j4+8<32) dbATC_TC2Offset[j2]=Temperature.dATCTempOffset[j4+8]; else W906_ATC913_NoteGuard(2, i, j, j4+8);
                            }
                        }
                    }
                }

                if(Temperature.bBoostFuncttion ||
                   Temperature.bLBTempFunction)                                 //Steven 20180817 : Boost Function
                {
                    if(iTriggerBoostFuncBack!=-1 &&
                       iBoostFuncStep>=10)                                      //Cool down offset is controlled by MainProc()
                    {
                    }
                    else
                    {
#if 0 // GATE(W906-POOL13-ATCSEND) golden 913 uLotInfo.cpp:10159 -- W-224 Q3: the ATC send is not wired (acarry_shims.h:109 shim has iATC_MODE_TYPE only; forms/fATCHandlerSide.h keeps the send undefined) -- recorded, not sent
                        ATC_InterfaceForm->SetOffset(iTotal_Channel, dbATC_Offset);                                     //TfLotInfo::SetATCOffset with boost function
#endif // GATE(W906-POOL13-ATCSEND)
                        W906_ATC913_Record(W906_ATC913_SETOFFSET, iTotal_Channel, dbATC_Offset, ATC_MAX_SITE);   // AI(W906-POOL13) W-224: what golden :10159 would send
                    }
                }
                else                                                            //Ifor 20160523 送出ATC 對應 Heat Offset 資料
                {
#if 0 // GATE(W906-POOL13-ATCSEND) golden 913 uLotInfo.cpp:10164 -- W-224 Q3: the ATC send is not wired (acarry_shims.h:109 shim has iATC_MODE_TYPE only; forms/fATCHandlerSide.h keeps the send undefined) -- recorded, not sent
                    ATC_InterfaceForm->SetOffset(iTotal_Channel, dbATC_Offset);                                         //Ztex 2023.04.19 Add HT-1032 TriTemp Function //ATC 送Offset命令 TfLotInfo::SetATCOffset
#endif // GATE(W906-POOL13-ATCSEND)
                    W906_ATC913_Record(W906_ATC913_SETOFFSET, iTotal_Channel, dbATC_Offset, ATC_MAX_SITE);   // AI(W906-POOL13) W-224: what golden :10164 would send
                }

                if(Temperature.bUseReferTempSensor &&
                   Temperature.bUseTC2Offset &&
                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33)              //Steven 20260415 : TC2 Offset Rogers(TYPE_33) only
#if 0 // GATE(W906-POOL13-ATCSEND) golden 913 uLotInfo.cpp:10170 -- W-224 Q3: the ATC send is not wired (acarry_shims.h:109 shim has iATC_MODE_TYPE only; forms/fATCHandlerSide.h keeps the send undefined) -- recorded, not sent
                    ATC_InterfaceForm->SetTC2Offset(iATC_Use_Heat_Count, dbATC_TC2Offset);
#endif // GATE(W906-POOL13-ATCSEND)
                    W906_ATC913_Record(W906_ATC913_SETTC2OFFSET, iATC_Use_Heat_Count, dbATC_TC2Offset, ATC_HEAD_COUNT);   // AI(W906-POOL13) W-224: what golden :10170 would send
                if(Temperature.bATC_MultiSensorEnable==true)
                {
                    for(int i=0; i<iATC_Use_Heat_Count; i++)
                    {
                        for(int j=0; j<4; j++)
                        {
                            dATC_MultiOfs[(i*4)+j]=Temperature.dATC_MultiSensorOfs[(i*4)+j]+dbATC_Offset[i];   // GOLDEN NOTE (W-224): dATC_MultiOfs / dATC_MultiSensorOfs are [32] -- in range only while iATC_Use_Heat_Count<=8; NOT guarded (outside the Q4 approval), reported to St02-M
                        }
                    }
#if 0 // GATE(W906-POOL13-ATCSEND) golden 913 uLotInfo.cpp:10180 -- W-224 Q3: the ATC send is not wired (acarry_shims.h:109 shim has iATC_MODE_TYPE only; forms/fATCHandlerSide.h keeps the send undefined) -- recorded, not sent
                    ATC_InterfaceForm->SetMultiSensorOffset(iATC_Use_Heat_Count, dATC_MultiOfs);
#endif // GATE(W906-POOL13-ATCSEND)
                    W906_ATC913_Record(W906_ATC913_SETMULTISENSOROFFSET, iATC_Use_Heat_Count, dATC_MultiOfs, 32);   // AI(W906-POOL13) W-224: what golden :10180 would send
                }
            }
        }

        if(Tri_Temp_Machine==1)                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        {
            if(Temperature.bUseReferTempSensor==true)
#if 0 // GATE(W906-POOL13-ATCSEND) golden 913 uLotInfo.cpp:10188 -- W-224 Q3: the ATC send is not wired (acarry_shims.h:109 shim has iATC_MODE_TYPE only; forms/fATCHandlerSide.h keeps the send undefined) -- recorded, not sent
                ATC_InterfaceForm->Set2ndFunction(Temperature.bUseReferTempSensor);                                     //TriTemp 使用第二點溫度
#endif // GATE(W906-POOL13-ATCSEND)
                W906_ATC913_RecordBool(Temperature.bUseReferTempSensor);   // AI(W906-POOL13) W-224: what golden :10188 would send
        }
        else
        {
            if(CosFunction.bATC32UseTJMode==false)                              //Ifor ASEM & AMD ATC3.2 跑 TJ 不使用第二點Sensor
#if 0 // GATE(W906-POOL13-ATCSEND) golden 913 uLotInfo.cpp:10193 -- W-224 Q3: the ATC send is not wired (acarry_shims.h:109 shim has iATC_MODE_TYPE only; forms/fATCHandlerSide.h keeps the send undefined) -- recorded, not sent
                ATC_InterfaceForm->Set2ndFunction(Temperature.bUseReferTempSensor);                                     //Ifor 20160506 add ATC 第二點溫度Sensor開關 //Ifor 20160818 搬移位置ATC 7.0無第二點溫度Sensor
#endif // GATE(W906-POOL13-ATCSEND)
                W906_ATC913_RecordBool(Temperature.bUseReferTempSensor);   // AI(W906-POOL13) W-224: what golden :10193 would send
        }

        if(Temperature.bLBTempFunction && iTriggerBoostFunction==-1)
        {
            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)
                fTemp_Set->ControlATC60AirFlow(0);
        }
    }
}
//---------------------------------------------------------------------------
