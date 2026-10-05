// =============================================================================
//  mykitsuck.cpp  --  TMySucker (vacuum-nozzle) + TMyKitSuck (nozzle-grid)
//                      method bodies.  Faithful translation of golden
//                      mykitsuck.cpp (2,894 lines, BCB6, Big5/cp950).
//
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3, group "kitsuck".
//
//  See mykitsuck.h's banner FIRST -- it explains this file's relationship to   [AI(W906-B30-KITSUCK) 20261002: STALE since 0924 W3-8 A4-6 (8ff6c754) -- this pair IS the linked TMySucker / TMyKitSuck now (CMakeLists.txt:2010 `aHotPlateSubstrate.cpp  mykitsuck.cpp`); aHotPlateSubstrate.cpp:79-929's mirror is in #if 0 (NB2 R128). The banner text is history.]
//  the already-wired-in aHotPlateSubstrate.h/.cpp TMyKitSuck/TMySucker
//  substrate (same class/global names, a DIFFERENT, PARALLEL, not-yet-linked
//  translation unit; NOT added to CMakeLists.txt, per HARD RULE 7).
//
//  ROLE: SAFETY-CRITICAL vacuum-nozzle control.  TMySucker::Suck()/Destroy()
//  are the two IO-driving state machines that decide whether a real vacuum
//  line turns on/off; TMyKitSuck is the RxC grid of nozzles (a station's
//  "kit") plus every predicate the rest of the machine uses to ask "does
//  this grid hold an IC / a real IC / a specific IC type".
//
//  WAVE SCOPE -- ALL 94 golden symbols in this TU (golden line cited per
//  function; ACTIVE/faithful unless noted):
//    TMyKitSuck::TMyKitSuck                    golden :49
//    TMyKitSuck::~TMyKitSuck                   golden :90
//    TMyKitSuck::SetMyLed                      golden :108
//    TMyKitSuck::SetItemData                   golden :149
//    TMyKitSuck::SetItemAmount                 golden :189
//    TMyKitSuck::SetMotorCount                 golden :200
//    TMyKitSuck::SetPickerCount(7-arg)         golden :206
//    TMyKitSuck::SetPickerCount(2-arg)         golden :218
//    TMyKitSuck::NoIC                          golden :227
//    TMyKitSuck::FindNoIC                      golden :238
//    TMyKitSuck::HAS_NO_IC                     golden :247
//    TMyKitSuck::All_HasIC                     golden :256
//    TMyKitSuck::UseSiteFullIC                 golden :273
//    TMyKitSuck::UseSiteHasIC                  golden :288
//    TMyKitSuck::UseSiteNoIC                   golden :297
//    TMyKitSuck::ArmAll_HasICType               golden :306
//    TMyKitSuck::ShtAll_HasICType               golden :323
//    TMyKitSuck::SetNullIcToHasNullIc           golden :340
//    TMyKitSuck::SetUnuseToNullIC               golden :354
//    TMyKitSuck::SetUnuseAndHasNullICToNullIC   golden :369
//    TMyKitSuck::SetHasNullIcToNullIc           golden :385
//    TMyKitSuck::SetAllToNullIC                 golden :399
//    TMyKitSuck::All_HAS_NULL_IC                golden :406
//    TMyKitSuck::HasRealIC                      golden :432
//    TMyKitSuck::HasRealIC_Left                 golden :458
//    TMyKitSuck::HasRealIC_Right                golden :486
//    TMyKitSuck::RowHasDefineIC                 golden :514
//    TMyKitSuck::HasDefineIC                    golden :538
//    TMyKitSuck::CountRealIC                    golden :616
//    TMyKitSuck::HasIC                          golden :655
//    TMyKitSuck::LeftSideNoIC                   golden :673
//    TMyKitSuck::RightSideNoIC                  golden :700
//    TMyKitSuck::ArmLeftSideHaveRealIC          golden :730
//    TMyKitSuck::ArmRightSideHaveRealIC         golden :768
//    TMyKitSuck::ArmUpSideNoIC                  golden :793
//    TMyKitSuck::ArmDownSideNoIC                golden :823
//    TMyKitSuck::ArmRow0HaveRealIC              golden :833
//    TMyKitSuck::ArmRow1HaveRealIC              golden :843
//    TMyKitSuck::ArmUpSideHaveRealIC            golden :853
//    TMyKitSuck::ArmUpSideAllTypeIC             golden :881
//    TMyKitSuck::IsShtSuckFinish                golden :891
//    TMyKitSuck::IsShtDestroyFinish             golden :904
//    TMyKitSuck::IsPickSuckFinish               golden :917
//    TMyKitSuck::IsPickDestroyFinish            golden :930
//    TMyKitSuck::ArmDownSideHaveRealIC          golden :943
//    TMyKitSuck::ArmDownSideAllTypeIC           golden :971
//    TMyKitSuck::ClearAll                       golden :981
//    TMyKitSuck::SetAll                         golden :1006
//    TMyKitSuck::Tested                         golden :1017
//    TMyKitSuck::HasNotTestYet                  golden :1032
//    TMyKitSuck::MoveAllItem                    golden :1043
//    CopyInitSuck (free fn)                     golden :1111
//    TMyKitSuck::CopyFrom                       golden :1151
//    TMyKitSuck::MergeSuck                      golden :1198
//    TMyKitSuck::SplitSuck                      golden :1261
//    TMyKitSuck::MergeSuck1Row                  golden :1331
//    TMyKitSuck::SplitSuck1Row                  golden :1385
//    TMyKitSuck::MoveSuckData                   golden :1443
//    TMyKitSuck::MoveSuckDataDiff                golden :1503
//    TMyKitSuck::CopyFromTray                    golden :1563 (see GAP note below)
//    TMyKitSuck::CopyToTray                      golden :1614 (see GAP note below)
//    TMyKitSuck::AlreadyTest                     golden :1663
//    TMyKitSuck::AlreadyTestNotIncludeErrorBin   golden :1685
//    TMyKitSuck::PartAlreadyTest                 golden :1701
//    TMyKitSuck::HasType                         golden :1723
//    TMySucker::TMySucker                        golden :1736
//    TMySucker::~TMySucker                       golden :1801
//    TMySucker::PushOnTime                       golden :1805
//    TMySucker::PushOffTime                      golden :1827
//    TMySucker::ReStart                          golden :1849
//    TMySucker::Reset                            golden :1855
//    TMySucker::GetOnBit                         golden :1862
//    TMySucker::GetOffBit                        golden :1890
//    TMySucker::GetStatus                        golden :1918
//    TMySucker::Sensor                           golden :1943
//    TMySucker::DoOnIO                           golden :1951
//    TMySucker::DoOffIO                          golden :2020
//    TMySucker::OnSuck                           golden :2089
//    TMySucker::OffSuck                          golden :2100
//    TMySucker::OnDestroy                        golden :2107
//    TMySucker::OffDestroy                       golden :2113
//    TMySucker::On                               golden :2119
//    TMySucker::Off                              golden :2134
//    TMySucker::Normal                           golden :2143
//    TMySucker::IsSuckFinish                     golden :2151
//    TMySucker::IsDestroyFinish                  golden :2156
//    TMySucker::Suck                             golden :2161 (SAFETY-CRITICAL)
//    TMySucker::Destroy                          golden :2362 (SAFETY-CRITICAL)
//    TMySucker::FinshFullMotion                  golden :2650
//    TMySucker::ResetSuckTask                    golden :2657
//    TMyKitSuck::ClearAllError                   golden :2666
//    ClearAllManualSuckTask (free fn)             golden :2678
//    TMySucker::SetRetryCount                    golden :2673
//    TMySucker::CheckIsFallDown                   golden :2688
//    TMyKitSuck::CheckLedStatus                   golden :2698
//    TMyKitSuck::ResetAll                         golden :2716
//    TMyKitSuck::CheckVaccumIsIniaialON           golden :2732
//    TMyKitSuck::CheckVaccumIsON_AboveSocket      golden :2748
//    TMyKitSuck::SetAllRealIC2InterfaceBin        golden :2760
//    TMyKitSuck::SetAllHASIC2ErrorBin             golden :2777
//    CopyKitSuck (free fn)                        golden :2795
//    CopySuck (free fn)                           golden :2830
//    TMyKitSuck::CheckDestoryFinish                golden :2862
//    TMySucker::GetIOValue                         golden :2879
//    TMySucker::GetIOValueThread                   golden :2884
//    TMySucker::SetIOValueThread                   golden :2889
//  = 94 functions total (90 methods + 4 free functions), ported_fns = 94 = golden_fns.
//
//  NOT ported (not a golden_fns discrepancy -- there is no golden body):
//    TMyKitSuck::MoveSingalItem -- declared golden mykitsuck.h:262, but golden
//      mykitsuck.cpp has NO definition anywhere (confirmed: grepped every
//      `TMyKitSuck::` line in the golden .cpp; MoveSingalItem never appears).
//      A dead API in golden itself, not a translation gap. Declaration kept
//      verbatim in mykitsuck.h (HARD RULE 1); no body added here because
//      there is nothing to translate.
//    InitialSuckerName -- declared `extern` in golden mykitsuck.h:389 but
//      DEFINED in a different golden TU (not mykitsuck.cpp -- confirmed
//      absent from this file by the same grep). Out of this file's scope by
//      construction; declaration only, matching golden's own header.
//    SetAllRealIC2InterfaceBin (FREE function, golden mykitsuck.h:390) is a
//      SEPARATE declaration from the TMyKitSuck::SetAllRealIC2InterfaceBin
//      MEMBER (golden mykitsuck.h:276, defined golden :2760, ported above).
//      Golden mykitsuck.cpp defines only the member; the free-function
//      extern is (like InitialSuckerName) declared here and defined
//      elsewhere in golden -- not this file's job.
//
//  GATE REGISTER -- no #if 0/#else gates in this file (every symbol this
//  file's bodies call already has a real, linkable body somewhere in this
//  port tree -- MyLaneIO/IOBitOn/IOBitOff/IOOutBitStatus/IOInputBit
//  (myio.cpp, MyLaneIo.cpp), TQPF_Timer (myTimer.cpp), TTrayMotor/MOT[]
//  (mymotor.cpp), MyDBIProcess (cMyDB.h decl, body SECSGEM/uHGemEquipment.cpp),
//  ChangeToFloatNonPcnt (MachineType.h template) -- all grepped and confirmed
//  present before writing a single call to them, per this campaign's own
//  "guessing is the most common way these waves go wrong" instruction).
//
//  ONE DISCOVERED GAP, DOCUMENTED RATHER THAN GATED (it is not a missing
//  function body, so the #if 0/#else idiom does not apply): TMyKitSuck::
//  CopyFromTray/CopyToTray (golden :1563/:1614) dereference
//  `Mot.Tray.PordRec[TrayC][TrayR]->asBuffer->...` -- TMyTray::PordRec is a
//  real, already-ported member (mytray.h:63, `TMyProductionRecord
//  *PordRec[][]`), but mytray.cpp's own banner labels itself a "PARTIAL W4
//  STUB" [20260925 AI(W906-W3-11): no longer -- retranslated; ClearData now calls PordRec->InitialRecord() like golden] and its ClearData() set every PordRec[i][j] slot to NULL and never
//  allocates one (mytray.cpp:63) -- a pre-existing gap in a file this wave
//  may not touch (HARD RULE 7). Ported FAITHFULLY (golden's own dereference,
//  verbatim) because the call target (TMyTray::PordRec) genuinely exists and
//  compiles; the hazard is a NULL pointer at RUN time if CopyFromTray/
//  CopyToTray are ever exercised against a TTrayMotor whose Tray.PordRec
//  slots were never allocated by whatever future code constructs real
//  TMyProductionRecord objects for a tray. Flagged here for whoever wires
//  this file into the build.
//
//  VCL/Borland conversions: #pragma hdrstop/package(smart_init) dropped
//  (golden :2,14). No __fastcall on the two constructors/destructor defined
//  in THIS file (mykitsuck.h already drops it from the declarations; HARD
//  RULE 4). `MyDBIProcess` is called, not declared, here -- its declaration
//  (with whatever calling convention cMyDB.h already uses) arrives via
//  MachineDefine.h, so no local __fastcall mismatch is introduced.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
//  AI(W906-PT-W3) 20260807 CORRECTED INCLUDE NOTE: MachineDefine.h in THIS
//  port tree is a W0-TAIL stub -- its entire driver/app-header include block
//  (myio.h/LastSet.h/cMyDB.h/cSetUp.h/... , golden lines mirrored at :34-146)
//  sits behind an unconditional `#if 0` (MachineDefine.h:34/146), so it
//  brings in ONLY vcl_compat.h + the portable STL headers, none of golden's
//  IO/LastSet/DB surface. Each of those is therefore included EXPLICITLY
//  below instead of assumed transitive -- confirmed by a failed syntax-check
//  iteration of this very file before this comment was written.
#include "MachineDefine.h"          // de-VCL'd include hub -- vcl_compat.h (AnsiString) + portable STL only (see note above)
#include "mykitsuck.h"              // this unit's own contract (golden "MyKitSuck.h")

#include "myio.h"                   // golden (via old MachineDefine.h chain) -- IOBitOn/IOBitOff/IOOutBitStatus/IOInputBit (flat ISA/legacy port API)
#include "LastSet.h"                // golden (via old MachineDefine.h chain) -- extern LAST_GENERAL_SET LastSet; (LastSet.iRealDummy)
#include "cMyDB.h"                  // golden (via old MachineDefine.h chain) -- MyDBIProcess(AnsiString,AnsiString,AnsiString="") decl; real body SECSGEM/uHGemEquipment.cpp
#include "MyLaneIo.h"               // golden "MyLaneIo.h" -- MyLaneIO (TLaneIO) global: IOBitOn/IOBitOff/IOOutBitStatus/IOInputBit/GetIOValue/GetIOValueThread/SetIOValueThread (Ring/IP/Port-addressed)
#include "cmydef.h"                 // golden "cMydef.h" -- IC-state constants (NULL_IC/HAS_IC/...), TYPE_A/TYPE_B, INDEX_SUCKER_TYPE, bOutArmPlaceDevice[][], bRunAutoSiteMapping, bAutoSiteMapHotplateSave, iAutoSiteMapHotplatePlateC/R, SystemHour/Min/Sec/MSec, iHome, bLoadInarmAutoHigh, iTestBinCount, START_TEST/TEST_PASS -- also pulled transitively by mykitsuck.h; kept explicit to mirror golden's own include line
#include "cprod.h"                  // golden "cprod.h" -- CosFunction.RecordIndexAirOnTime (also transitively via cmydef.h)
using vclcompat::TList; using vclcompat::clBlack; using vclcompat::clGreen; using vclcompat::clRed; using vclcompat::clBlue; using vclcompat::clLime; using vclcompat::clWhite; static const TColor clAqua = 0x00FFFF00;  // AI(W906-A4-6) 20260924: 從 mykitsuck.h 移來（只在本檔範圍）。原註：golden "mymessbox.h" NOT re-included: grepped this file's translated body
// for every ShowMyMessage-family call -- zero hits. Nothing here needs it.
#include "Motor/mymotor.h"          // golden "MyMotor.h" -- class TTrayMotor (CopyFromTray/CopyToTray's Mot parameter; TMyTray Tray member)

//=====================================
TMyKitSuck InArmSuck;
TMyKitSuck FLCarryKit;
TMyKitSuck FRCarryKit;
TMyKitSuck BLCarryKit;
TMyKitSuck BRCarryKit;
TMyKitSuck OutSht3Kit;                                                          //Steven 20240822 : For HT-9046AU
TMyKitSuck FTestSuck;
TMyKitSuck BTestSuck;
TMyKitSuck OutArmSuck;
TMyKitSuck OutArm2Suck;                                                         //Steven 20240822 : For HT-9046AU
TMyKitSuck CatchTraySuck;
TMyKitSuck TestSocket;
TMyKitSuck CheckKitSuck;                                                        //20111130  Dell
TMyKitSuck AOIKit;                                                              //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
TMyKitSuck InArmPlaceSuck;                                                      //JimmyChiu 20220908 add Pickup Error Placement
//---------------------------------------------------------------------------
TList *pSuck;
TMySucker *pTempSuck;

TMyKitSuck FTestSuckBackup;
TMyKitSuck BTestSuckBackup;

TMyKitSuck InArmSuckBackup;
TMyKitSuck OutArmSuckBackup;

TMyKitSuck *ptrInSHT;                                                           //Auto Clean Kit use
TMyKitSuck ptrInSHTBackup;                                                      //Auto Clean Kit use
TMyKitSuck *ptrOutSHT;
TMyKitSuck *ptr2DIDSHT;
TMyKitSuck *ptrMulti2DIDSHT;
TMyProductionRecord OutArmPordRec[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];      //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
TMyProductionRecord SortArmPordRec[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];     //RogerYang 20250515 Add for 9046AU
//=====================================
TMyKitSuck::TMyKitSuck()
{
    for(int i=0; i<_MAX_SUCK_ROW_ITEM; i++)
    {
        for(int j=0; j<_MAX_SUCK_COL_ITEM; j++)
        {
            bLed[i][j]=false;
            Item[i][j]=NULL_IC;

            iAutoCleanRecX[i][j]=-1;                                            //ChungHung 20130628 先初始化
            iAutoCleanRecY[i][j]=-1;                                            //ChungHung 20130628 先初始化
            iBinData[i][j]=-1;                                                  //Steven 20190116 : 修正顯示錯誤, 加上初始化保護

            iWhichSite[i][j]  =-1;
            iWhichAuto[i][j]  =-1;
            iWhichIndex[i][j] =-1;                                              //ChungHung 20150205 add for ATK

            bPass[i][j]       =false;
            iCleanCount[i][j] =0;
            bFliped[i][j]     =false;

            iBinDataBackUp[i][j]=-1;                                            //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
            iCurrRotAng[i][j] =0;                                               //Steven 20170425 (wei) : Add rotate motor
            iNeedRotAng[i][j] =0;
            PordRec[i][j].InitialRecord();
        }
    }
    iMotRow=1;
    iMotCol=1;
    iMaxRow=1;
    iMaxCol=1;
    iMaxCnt=1;
    iPickRow=1;
    iPickCol=1;
    iShtRow=1;
    iShtCol=1;
    iShtCnt=1;
    iPickStep=1;                                                                //Pitch倍數, 13吸嘴就寫2, 14吸嘴就寫3
    iPickKitStep=1;
}
//----------------------------------------------------------------------------
TMyKitSuck::~TMyKitSuck()
{
    try
    {
        /*for(int i=0; i<_MAX_SUCK_ROW_ITEM; i++)
        {
            for(int j=0; j<_MAX_SUCK_COL_ITEM; j++)
            {
                PordRec[i][j].DeleteProductionRecord();
            }
        }  */
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~TMyKitSuck");
    }
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetMyLed(int iRow, int iCol, TALed *ledPtr)
{
    if(ledPtr!=NULL)
    {
        bLed[iRow][iCol]=true;
        pLed[iRow][iCol]=ledPtr;
    }

    if(bLed[iRow][iCol]==true &&
       pLed[iRow][iCol]->Visible)                                               //Steven 20220117 : 修正程式載入的時候,不會顯示有IC的地方
    {
        if(Item[iRow][iCol]!=NULL_IC)
        {
            if(Item[iRow][iCol]==HAS_IC)
                pLed[iRow][iCol]->TrueColor=clGreen;
            else if(Item[iRow][iCol]==HAS_NULL_IC)
                pLed[iRow][iCol]->TrueColor=clWhite;
            else if(Item[iRow][iCol]==HAS_HOT_IC)
                pLed[iRow][iCol]->TrueColor=(TColor)0x004080FF;
            else if(Item[iRow][iCol]==RotateOK)
                pLed[iRow][iCol]->TrueColor=clAqua;
            else if(Item[iRow][iCol]==CLEAN_FINISH_IC)                          //Steven 20130701
                pLed[iRow][iCol]->TrueColor=clBlue;
            else if(Item[iRow][iCol]==HAS_NULL_CLEAN_IC)                        //wei 20160130
                pLed[iRow][iCol]->TrueColor=clAqua;
            else if(Item[iRow][iCol]<TEST_PASS)
                pLed[iRow][iCol]->TrueColor=(TColor)0x00C08080;
            else if(Item[iRow][iCol]==TEST_PASS+1)                              //Steven 20160513 : 改為TEST_PASS+1
                pLed[iRow][iCol]->TrueColor=clLime;
            else
                pLed[iRow][iCol]->TrueColor=clRed;
            pLed[iRow][iCol]->Value=true;
        }
        else
        {
            pLed[iRow][iCol]->Value=false;
        }
        pLed[iRow][iCol]->Hint=Suck[iRow][iCol].sName;
    }
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetItemData(int iRow, int iCol, int Data, int iTarget)
{
    Item[iRow][iCol]=Data;
    if(iTarget!=-1)
    {
        iWhichAuto[iRow][iCol]=iTarget;
    }

    if(bLed[iRow][iCol]==true &&
       pLed[iRow][iCol]->Visible)
    {
        if(Data!=NULL_IC)
        {
            if(Data==HAS_IC)
                pLed[iRow][iCol]->TrueColor=clGreen;
            else if(Data==HAS_NULL_IC)
                pLed[iRow][iCol]->TrueColor=clWhite;
            else if(Data==HAS_HOT_IC)
                pLed[iRow][iCol]->TrueColor=(TColor)0x004080FF;
            else if(Data==RotateOK)
                pLed[iRow][iCol]->TrueColor=clAqua;
            else if(Data==CLEAN_FINISH_IC)                                      //Steven 20130701
                pLed[iRow][iCol]->TrueColor=clBlue;
            else if(Data==HAS_NULL_CLEAN_IC)                                    //wei 20160130
                pLed[iRow][iCol]->TrueColor=clAqua;
            else if(Data<TEST_PASS)
                pLed[iRow][iCol]->TrueColor=(TColor)0x00C08080;
            else if(Data==TEST_PASS+1)                                          //Steven 20160513 : 改為TEST_PASS+1
                pLed[iRow][iCol]->TrueColor=clLime;
            else
                pLed[iRow][iCol]->TrueColor=clRed;
            pLed[iRow][iCol]->Value=true;
        }
        else
        {
            pLed[iRow][iCol]->Value=false;
        }
    }
};
//----------------------------------------------------------------------------
void TMyKitSuck::SetItemAmount(int iRow, int iCol)
{
    if(iRow>=_MAX_SUCK_ROW_ITEM)
        iRow=_MAX_SUCK_ROW_ITEM;
    if(iCol>=_MAX_SUCK_COL_ITEM)
        iCol=_MAX_SUCK_COL_ITEM;
    iMaxRow=iRow;
    iMaxCol=iCol;
    iMaxCnt=iRow*iCol;
};
//----------------------------------------------------------------------------
void TMyKitSuck::SetMotorCount(int iRow, int iCol)
{
    iMotRow=iRow;
    iMotCol=iCol;
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetPickerCount(int _iPickRow, int _iPickCol, int _iShtRow, int _iShtCol, int _iPickStep, int _iKitStep, int _iShtStep)
{
    iPickRow    =_iPickRow;
    iPickCol    =_iPickCol;
    iShtRow     =_iShtRow;
    iShtCol     =_iShtCol;
    iShtCnt     =iShtRow*iShtCol;
    iPickStep   =_iPickStep;                                                    //Pitch倍數, 13吸嘴就寫2, 14吸嘴就寫3
    iPickKitStep=_iKitStep;
    iShtKitStep =_iShtStep;
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetPickerCount(int _iPickRow, int _iPickCol)                   //Steven 20241002 : TestSuck補上 Row / Col
{
    iPickRow    =_iPickRow;
    iPickCol    =_iPickCol;
    iShtRow     =_iPickRow;
    iShtCol     =_iPickCol;
    iShtCnt     =iShtRow*iShtCol;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::NoIC()
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            if(Item[i][j]!=NULL_IC)
                return false;
    return true;
}
//----------------------------------------------------------------------------
// 判斷是否有空位置  kevin 20120531
//----------------------------------------------------------------------------
bool TMyKitSuck::FindNoIC()
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            if(Item[i][j]==NULL_IC)
                return true;
    return false;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::HAS_NO_IC()
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            if(Item[i][j]!=NULL_IC)
                return true;
    return false;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::All_HasIC()                                                    //ChungHung 20111230
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(bLed[i][j]==true &&
               pLed[i][j]->Visible &&
               Item[i][j]==NULL_IC)                                             //Steven 20151015 : When Index drop and only have HAS_NULL_IC, will hang up
            {
                return false;
            }
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool TMyKitSuck::UseSiteFullIC()                                                //Ifor 20161215 add 判斷 Use Site 有NULL_IC
{
    for(int i=0; i<iShtRow; i++)
    {
        for(int j=0; j<iShtCol; j++)
        {
            if(Item[i][j]==NULL_IC)
            {
                return false;
            }
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool TMyKitSuck::UseSiteHasIC()
{
    for(int i=0; i<iShtRow; i++)
        for(int j=0; j<iShtCol; j++)
            if(Item[i][j]!=NULL_IC)
                return true;
    return false;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::UseSiteNoIC()
{
    for(int i=0; i<iShtRow; i++)
        for(int j=0; j<iShtCol; j++)
            if(Item[i][j]!=NULL_IC)
                return false;
    return true;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::ArmAll_HasICType(int IC_TYPE1, int IC_TYPE2)                   //Steven 20250420 : fixed for auto clean
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
//            if(bLed[i][j]==true &&
//               pLed[i][j]->Visible==true)                                     //Steven 20250326 : modified
//            {
                if(Item[i][j]!=IC_TYPE1 && Item[i][j]!=IC_TYPE2)
                    return false;
//            }
        }
    }
    return true;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::ShtAll_HasICType(int IC_TYPE1, int IC_TYPE2)                   //這個不能用在In / out arm上面
{
    for(int i=0; i<iShtRow; i++)
    {
        for(int j=0; j<iShtCol; j++)
        {
//            if(bLed[i][j]==true &&
//               pLed[i][j]->Visible==true)                                     //Steven 20250326 : modified
//            {
                if(Item[i][j]!=IC_TYPE1 && Item[i][j]!=IC_TYPE2)
                    return false;
//            }
        }
    }
    return true;
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetNullIcToHasNullIc()
{
    for(int i=0; i<iShtRow; i++)
    {
        for(int j=0; j<iShtCol; j++)
        {
            if(Item[i][j]==NULL_IC)
            {
                SetItemData(i, j, HAS_NULL_IC);                                 //Steven 20150203 : Fixed for Sucker Status
            }
        }
    }
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetUnuseToNullIC()                                             //Steven 20241017 : 清除沒用到的資料
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(i>=iShtRow ||
               j>=iShtCol)
            {
                SetItemData(i, j, NULL_IC);
            }
        }
    }
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetUnuseAndHasNullICToNullIC()                                 //Steven 20241017 : 清除沒用到的資料 與 HAS_NULL_IC資料
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(i>=iShtRow ||
               j>=iShtCol ||
               Item[i][j]==HAS_NULL_IC)
            {
                SetItemData(i, j, NULL_IC);
            }
        }
    }
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetHasNullIcToNullIc()                                         //Steven 20160524 : ADD SetHasNullIcToNullIc
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_NULL_IC)
            {
                SetItemData(i, j, NULL_IC);
            }
        }
    }
}
//----------------------------------------------------------------------------
void TMyKitSuck::SetAllToNullIC()                                               //kevin 20150505
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            SetItemData(i, j, NULL_IC);                                         //Steven 20150203 : Fixed for Sucker Status
}
//----------------------------------------------------------------------------
bool TMyKitSuck::All_HAS_NULL_IC()
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return false;
    else if(iHasNullICCount)
        return true;
    else
        return false;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::HasRealIC()
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j]==HAS_NULL_CLEAN_IC)                              //wei 20160130
                iHasNullICCount++;
            else if(Item[i][j]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::HasRealIC_Left(int iRow, int Mid)
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    int j2;
    for(int i=0; i<iRow; i++)
    {
        for(int j=0; j<Mid; j++)
        {
            j2=j;
            if(Item[i][j2]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j2]==HAS_NULL_CLEAN_IC)                             //wei 20160130
                iHasNullICCount++;
            else if(Item[i][j2]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::HasRealIC_Right(int iRow, int Mid)
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    int j2;
    for(int i=0; i<iRow; i++)
    {
        for(int j=0; j<Mid; j++)
        {
            j2=j+Mid;
            if(Item[i][j2]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j2]==HAS_NULL_CLEAN_IC)                             //wei 20160130
                iHasNullICCount++;
            else if(Item[i][j2]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::RowHasDefineIC(int iRow, int IC_TYPE)                          //Steven 20221005 : 針對上下排判斷有沒有指定的IC
{
    bool bHasDefineIC=false;
    for(int j=0; j<iMaxCol; j++)
    {
        if(IC_TYPE==NULL_IC && Item[iRow][j]==IC_TYPE)
        {
            bHasDefineIC=true;
        }
        else
        {
            if(Item[iRow][j]!=NULL_IC)
            {
                if(Item[iRow][j]==IC_TYPE)
                {
                    bHasDefineIC=true;
                }
            }
        }
    }

    return bHasDefineIC;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::HasDefineIC(int IC_TYPE)                                       //Steven 20130620 : 改成Function
{
    bool bHasDefineIC=false;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(IC_TYPE==NULL_IC && Item[i][j]==IC_TYPE)                         //Steven 20160530 : fixed for HasDefineIC
            {
                bHasDefineIC=true;
            }
            else
            {
                if(Item[i][j]!=NULL_IC)
                {
                    if(Item[i][j]==IC_TYPE)
                    {
                        bHasDefineIC=true;
                    }
                }
            }
        }
    }
    return bHasDefineIC;
}
//----------------------------------------------------------------------------
int TMyKitSuck::CountRealIC()
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    return iOtherCount;
}
//----------------------------------------------------------------------------
bool TMyKitSuck::HasIC()
{
    return !NoIC();
}
//------------------------------------------------------------------------------
bool TMyKitSuck::LeftSideNoIC(int MiddleValue)
{
    if(MiddleValue==0)                                                          //Steven 20240612 : 修正Shuttle IC判斷
    {
        for(int i=0; i<iShtRow; i++)
        {
            for(int j=0; j<iShtCol; j++)
            {
                if(Item[i][j]!=NULL_IC)
                    return false;
            }
        }
    }
    else
    {
        for(int i=0; i<iShtRow; i++)
        {
            for(int j=0; j<MiddleValue; j++)
            {
                if(Item[i][j]!=NULL_IC)
                    return false;
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::RightSideNoIC(int MiddleValue)
{
    if(MiddleValue!=0)                                                          //Steven 20240612 : 修正Shuttle IC判斷
    {
        for(int i=0; i<iShtRow; i++)
        {
            for(int j=MiddleValue; j<iShtCol; j++)
            {
                if(Item[i][j]!=NULL_IC)
                    return false;
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmLeftSideHaveRealIC(int MiddleValue)
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<MiddleValue; j++)
        {
            if(Item[i][j*2]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j*2]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmRightSideHaveRealIC(int MiddleValue)                        //For 1x2 & 2x2使用的
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;

    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<MiddleValue; j++)
        {
            if(Item[i][1+j*2]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][1+j*2]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)                                                             //JerryYang 20170703 (Steven) Fix 2x2_8 Hang up
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmUpSideNoIC()
{
    for(int j=0; j<iMaxCol; j++)
    {
        if(Item[0][j]!=NULL_IC)
            return false;
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmDownSideNoIC()
{
    for(int j=0; j<iMaxCol; j++)
    {
        if(Item[1][j]!=NULL_IC)
            return false;
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmRow0HaveRealIC()
{
    for(int j=0; j<iMaxCol; j++)
    {
        if(Item[0][j]!=NULL_IC && Item[0][j]!=HAS_NULL_IC)
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmRow1HaveRealIC()
{
    for(int j=0; j<iMaxCol; j++)
    {
        if(Item[1][j]!=NULL_IC && Item[1][j]!=HAS_NULL_IC)
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmUpSideHaveRealIC(bool left)                                 //ChungHung 20130708 add left=true
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;

    int offset=0;
    if(left)
        offset=0;
    else
        offset=4;

    for(int j=0; j<4; j++)
    {
        if(Item[0][j+offset]==HAS_NULL_IC)                                      //ChungHung 20130708 add left=true
            iHasNullICCount++;
        else if(Item[0][j+offset]==NULL_IC)                                     //ChungHung 20130708 add left=true
            iNullICCount++;
        else
            iOtherCount++;
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmUpSideAllTypeIC(int IC_TYPE, int iOffset, int iCol)         //Steven 20220930 : CheckSpecialSiteMode run Auto Clean
{
    for(int j=0; j<iCol; j++)
    {
        if(Item[0][j+iOffset]!=IC_TYPE)
            return false;
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::IsShtSuckFinish()
{
    for(int i=0; i<iShtRow; i++)
    {
        for(int j=0; j<iShtCol; j++)
        {
            if(Suck[i][j].IsSuckFinish()==false)
                return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::IsShtDestroyFinish()
{
    for(int i=0; i<iShtRow; i++)
    {
        for(int j=0; j<iShtCol; j++)
        {
            if(Suck[i][j].IsDestroyFinish()==false)
                return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::IsPickSuckFinish()
{
    for(int i=0; i<iPickRow; i++)
    {
        for(int j=0; j<iPickCol; j++)
        {
            if(Suck[i][j].IsSuckFinish()==false)
                return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::IsPickDestroyFinish()
{
    for(int i=0; i<iPickRow; i++)
    {
        for(int j=0; j<iPickCol; j++)
        {
            if(Suck[i][j].IsDestroyFinish()==false)
                return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmDownSideHaveRealIC(bool left)                               //ChungHung 20130708 add left=true;
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;

    int offset=0;
    if(left)
        offset=0;
    else
        offset=4;

    for(int j=0; j<4; j++)
    {
        if(Item[1][j+offset]==HAS_NULL_IC)
            iHasNullICCount++;
        else if(Item[1][j+offset]==NULL_IC)
            iNullICCount++;
        else
            iOtherCount++;
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::ArmDownSideAllTypeIC(int IC_TYPE, int iOffset, int iCol)       //Steven 20220930 : CheckSpecialSiteMode run Auto Clean
{
    for(int j=0; j<iCol; j++)
    {
        if(Item[1][j+iOffset]!=IC_TYPE)
            return false;
    }
    return true;
}
//------------------------------------------------------------------------------
void TMyKitSuck::ClearAll()
{
    for(int i=0; i<_MAX_SUCK_ROW_ITEM; i++)
    {
        for(int j=0; j<_MAX_SUCK_COL_ITEM; j++)
        {
            SetItemData(i, j, NULL_IC);
            PordRec[i][j].InitialRecord();
        }
    }
}
//------------------------------------------------------------------------------
void TMyKitSuck::SetAll(int Type)
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            SetItemData(i, j, Type);
        }
    }
}
//------------------------------------------------------------------------------
bool TMyKitSuck::Tested()
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]>=START_TEST)                                          // && Item[i][j]<=TEST_FAIL8)             //Steven 20180522 : mark for [D52]導致Hang up問題
            {
                return true;
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::HasNotTestYet()
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
            if(Item[i][j]==HAS_IC || Item[i][j]==HAS_HOT_IC)                    //ChungHung 20121002 alter
                return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void TMyKitSuck::MoveAllItem(class TMyKitSuck &Source)
{
    int MinRow, MinCol;
    if(Source.iMaxRow>=iMaxRow)
        MinRow=iMaxRow;
    else
        MinRow=Source.iMaxRow;

    if(Source.iMaxCol>=iMaxCol)
        MinCol=iMaxCol;
    else
        MinCol=Source.iMaxCol;

    for(int i=0; i<MinRow; i++)
    {
        for(int j=0; j<MinCol; j++)
        {
            SetItemData(i, j, Source.Item[i][j]);

            cDeviceInf[i][j]            =Source.cDeviceInf[i][j];
            b2DIDNG[i][j]               =Source.b2DIDNG[i][j];                  //Steven 20200611 : for Murata, 2DID NG不測試
            cReDeviceInf[i][j]          =Source.cReDeviceInf[i][j];
            cSBin[i][j]                 =Source.cSBin[i][j];                    //Steven 20220120 : Amlogic需要收SBIN

            iWhichSite[i][j]            =Source.iWhichSite[i][j];
            iCurrRotAng[i][j]           =Source.iCurrRotAng[i][j];              //Steven 20170425 (wei) : Add rotate motor
            iNeedRotAng[i][j]           =Source.iNeedRotAng[i][j];
            iWhichAuto[i][j]            =Source.iWhichAuto[i][j];
            iWhichIndex[i][j]           =Source.iWhichIndex[i][j];              //ChungHung 20150205 add for ATK

            bPass[i][j]                 =Source.bPass[i][j];
            iCleanCount[i][j]           =Source.iCleanCount[i][j];
            bFliped[i][j]               =Source.bFliped[i][j];

            iBinData[i][j]              =Source.iBinData[i][j];
            iBinDataBackUp[i][j]        =Source.iBinDataBackUp[i][j];           //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount

            bQATray[i][j]               =Source.bQATray[i][j];
            iAOIResult[i][j]            =Source.iAOIResult[i][j];               //Sam 20240325 : 新增 DamageTrayMapping 功能

            PordRec[i][j].asBuffer->CommaText=Source.PordRec[i][j].asBuffer->CommaText;
            PordRec[i][j].bUse          =Source.PordRec[i][j].bUse;             //Frank 20160505 add

            Source.SetItemData(i, j, NULL_IC);
            Source.PordRec[i][j].InitialRecord();

            Source.cDeviceInf[i][j]     ="";
            Source.cReDeviceInf[i][j]   ="";
            Source.cSBin[i][j]          ="";                                    //Steven 20220120 : Amlogic需要收SBIN
            Source.b2DIDNG[i][j]        =false;                                 //Steven 20200611 : for Murata, 2DID NG不測試

            Source.iWhichSite[i][j]     =-1;
            Source.iWhichAuto[i][j]     =-1;
            Source.iWhichIndex[i][j]    =-1;                                    //ChungHung 20150205 add for ATK

            Source.bPass[i][j]          =false;
            Source.iCleanCount[i][j]    =0;
            Source.bFliped[i][j]        =false;

            Source.iBinData[i][j]       =-1;
            Source.iCurrRotAng[i][j]    =0;                                     //Steven 20170425 (wei) : Add rotate motor
            Source.iNeedRotAng[i][j]    =0;

            Source.bQATray[i][j]        =false;
            Source.iAOIResult[i][j]     =0;                                     //Sam 20240325 : 新增 DamageTrayMapping 功能
        }
    }
}
//------------------------------------------------------------------------------
void CopyInitSuck(TMyKitSuck *Source, TMyKitSuck *Target, int SourceR, int SourceC, int TargetR, int TargetC)
{
    Target->PordRec        [TargetR][TargetC]  =Source->PordRec         [SourceR][SourceC];
    Target->cDeviceInf     [TargetR][TargetC]  =Source->cDeviceInf      [SourceR][SourceC];
    Target->cReDeviceInf   [TargetR][TargetC]  =Source->cReDeviceInf    [SourceR][SourceC];
    Target->cSBin          [TargetR][TargetC]  =Source->cSBin           [SourceR][SourceC];                             //Steven 20220120 : Amlogic需要收SBIN
    Target->b2DIDNG        [TargetR][TargetC]  =Source->b2DIDNG         [SourceR][SourceC];                             //Steven 20200611 : for Murata, 2DID NG不測試
    Target->iWhichSite     [TargetR][TargetC]  =Source->iWhichSite      [SourceR][SourceC];
    Target->iWhichAuto     [TargetR][TargetC]  =Source->iWhichAuto      [SourceR][SourceC];
    Target->iCurrRotAng    [TargetR][TargetC]  =Source->iCurrRotAng     [SourceR][SourceC];                             //Steven 20170425 (wei) : Add rotate motor
    Target->iNeedRotAng    [TargetR][TargetC]  =Source->iNeedRotAng     [SourceR][SourceC];
    Target->iWhichIndex    [TargetR][TargetC]  =Source->iWhichIndex     [SourceR][SourceC];                             //ChungHung 20150205 add for ATK
    Target->bPass          [TargetR][TargetC]  =Source->bPass           [SourceR][SourceC];
    Target->iCleanCount    [TargetR][TargetC]  =Source->iCleanCount     [SourceR][SourceC];
    Target->bFliped        [TargetR][TargetC]  =Source->bFliped         [SourceR][SourceC];
    Target->iBinData       [TargetR][TargetC]  =Source->iBinData        [SourceR][SourceC];
    Target->iBinDataBackUp [TargetR][TargetC]  =Source->iBinDataBackUp  [SourceR][SourceC];                             //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
    Target->bQATray        [TargetR][TargetC]  =Source->bQATray         [SourceR][SourceC];
    Target->Item           [TargetR][TargetC]  =Source->Item            [SourceR][SourceC];
    Target->bNeedCheck     [TargetR][TargetC]  =Source->bNeedCheck      [SourceR][SourceC];
    Target->bScan          [TargetR][TargetC]  =Source->bScan           [SourceR][SourceC];
    Target->iAOIStation    [TargetR][TargetC]  =Source->iAOIStation     [SourceR][SourceC];                             //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    Target->bTrayMapNGData [TargetR][TargetC]  =Source->bTrayMapNGData  [SourceR][SourceC];                             //KenHsieh 20220923 : add Tray Map Throw IC Function
    Target->iAutoCleanRecX [TargetR][TargetC]  =Source->iAutoCleanRecX  [SourceR][SourceC];                             //jou 2013-03-13 Auto Clean Record X
    Target->iAutoCleanRecY [TargetR][TargetC]  =Source->iAutoCleanRecY  [SourceR][SourceC];                             //jou 2013-03-13 Auto Clean Record Y
    Target->bNeedReTest    [TargetR][TargetC]  =Source->bNeedReTest     [SourceR][SourceC];
    Target->bNeedRotate    [TargetR][TargetC]  =Source->bNeedRotate     [SourceR][SourceC];
    CopySuck(&Source->Suck[SourceR][SourceC], &Target->Suck[TargetR][TargetC]);

    Target->cDeviceInfAutoSitemapping   [TargetR][TargetC]  =Source->cDeviceInfAutoSitemapping   [SourceR][SourceC];    //Ifor 20170925 (Steven) : add
    Target->cReDeviceInfAutoSitemapping [TargetR][TargetC]  =Source->cReDeviceInfAutoSitemapping [SourceR][SourceC];    //Ifor 20170925 (Steven) : add
    Target->iNeedRotAngAutoSitemapping  [TargetR][TargetC]  =Source->iNeedRotAngAutoSitemapping  [SourceR][SourceC];    //Ifor 20170925 (Steven) : add 預計旋轉的角度
    Target->iCurrRotAngAutoSitemapping  [TargetR][TargetC]  =Source->iCurrRotAngAutoSitemapping  [SourceR][SourceC];    //Ifor 20170925 (Steven) : add 目前產品的角度
    Target->iWhichShuttleAutoSitemapping[TargetR][TargetC]  =Source->iWhichShuttleAutoSitemapping[SourceR][SourceC];
    Target->iWhichKitAutoSitemapping    [TargetR][TargetC]  =Source->iWhichKitAutoSitemapping    [SourceR][SourceC];
    Target->iHotCountAutoSitemapping    [TargetR][TargetC]  =Source->iHotCountAutoSitemapping    [SourceR][SourceC];
    Target->pLed                        [TargetR][TargetC]  =Source->pLed                        [SourceR][SourceC];
    Target->bLed                        [TargetR][TargetC]  =Source->bLed                        [SourceR][SourceC];
}
//------------------------------------------------------------------------------
void TMyKitSuck::CopyFrom(class TMyKitSuck &Source)
{
    int MinRow, MinCol;
    if(Source.iMaxRow>=iMaxRow)
        MinRow=iMaxRow;
    else
        MinRow=Source.iMaxRow;

    if(Source.iMaxCol>=iMaxCol)
        MinCol=iMaxCol;
    else
        MinCol=Source.iMaxCol;

    for(int i=0; i<MinRow; i++)
    {
        for(int j=0; j<MinCol; j++)
        {
            SetItemData(i, j, Source.Item[i][j]);
            PordRec[i][j].asBuffer->CommaText=Source.PordRec[i][j].asBuffer->CommaText;
            PordRec[i][j].bUse  =Source.PordRec[i][j].bUse;                     //Frank 20160505 add

            cDeviceInf[i][j]    =Source.cDeviceInf[i][j];
            cReDeviceInf[i][j]  =Source.cReDeviceInf[i][j];
            cSBin[i][j]         =Source.cSBin[i][j];                            //Steven 20220120 : Amlogic需要收SBIN
            b2DIDNG[i][j]       =Source.b2DIDNG[i][j];                          //Steven 20200611 : for Murata, 2DID NG不測試

            iWhichSite[i][j]    =Source.iWhichSite[i][j];
            iWhichAuto[i][j]    =Source.iWhichAuto[i][j];
            iCurrRotAng[i][j]   =Source.iCurrRotAng[i][j];                      //Steven 20170425 (wei) : Add rotate motor
            iNeedRotAng[i][j]   =Source.iNeedRotAng[i][j];
            iWhichIndex[i][j]   =Source.iWhichIndex[i][j];                      //ChungHung 20150205 add for ATK
            bPass[i][j]         =Source.bPass[i][j];
            iCleanCount[i][j]   =Source.iCleanCount[i][j];
            bFliped[i][j]       =Source.bFliped[i][j];
            iBinData[i][j]      =Source.iBinData[i][j];
            iBinDataBackUp[i][j]=Source.iBinDataBackUp[i][j];                   //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
            bQATray[i][j]       =Source.bQATray[i][j];
            iAutoCleanRecX[i][j]=Source.iAutoCleanRecX[i][j];
            iAutoCleanRecY[i][j]=Source.iAutoCleanRecY[i][j];
            iAOIResult[i][j]    =Source.iAOIResult[i][j];                       //Sam 20240325 : 新增 DamageTrayMapping 功能
        }
    }
}
//------------------------------------------------------------------------------
//2013-01-15    Dell    Add nn Mode
//InsertCol true:1122 false:11
//                          22
void TMyKitSuck::MergeSuck(class TMyKitSuck &Source1, class TMyKitSuck &Source2, bool InsertCol)
{
    TMyKitSuck *tmSource[]={&Source1, &Source2};
    int MinRow, MinCol;
    if(Source1.iMaxRow>=iMaxRow)
        MinRow=iMaxRow;
    else
        MinRow=Source1.iMaxRow;

    if(Source1.iMaxCol>=iMaxCol)
        MinCol=iMaxCol;
    else
        MinCol=Source1.iMaxCol;

    int iIndex_ROW=0, iIndex_Col=0;
    int SourceR=0, SourceC=0;
    int TargetR=0, TargetC=0;

    for(int k=0; k<2; k++)
    {
        for(int i=0; i<MinRow; i++)
        {
            for(int j=0; j<MinCol; j++)
            {
                SourceR=i;
                SourceC=j;
                TargetR=i+iIndex_ROW;
                TargetC=j+iIndex_Col;

                SetItemData(TargetR, TargetC, tmSource[k]->Item[SourceR][SourceC]);
                iWhichSite  [TargetR][TargetC]=tmSource[k]->iWhichSite[SourceR][SourceC];
                iWhichAuto  [TargetR][TargetC]=tmSource[k]->iWhichAuto[SourceR][SourceC];
                iCurrRotAng [TargetR][TargetC]=tmSource[k]->iCurrRotAng[SourceR][SourceC];                              //Steven 20170425 (wei) : Add rotate motor
                iNeedRotAng [TargetR][TargetC]=tmSource[k]->iNeedRotAng[SourceR][SourceC];
                iWhichIndex [TargetR][TargetC]=tmSource[k]->iWhichIndex[SourceR][SourceC];                              //ChungHung 20150205 add for ATK
                bPass       [TargetR][TargetC]=tmSource[k]->bPass[SourceR][SourceC];
                iCleanCount [TargetR][TargetC]=tmSource[k]->iCleanCount[SourceR][SourceC];
                bFliped     [TargetR][TargetC]=tmSource[k]->bFliped[SourceR][SourceC];
                iBinData    [TargetR][TargetC]=tmSource[k]->iBinData[SourceR][SourceC];
                PordRec     [TargetR][TargetC].asBuffer->CommaText=tmSource[k]->PordRec[SourceR][SourceC].asBuffer->CommaText;
                PordRec     [TargetR][TargetC].bUse=tmSource[k]->PordRec[SourceR][SourceC].bUse;                        //Frank 20160505 add

                cDeviceInf[TargetR][TargetC]  =tmSource[k]->cDeviceInf[SourceR][SourceC];                               //Steven 20191126 : 補上
                cReDeviceInf[TargetR][TargetC]=tmSource[k]->cReDeviceInf[SourceR][SourceC];
                cSBin[TargetR][TargetC]       =tmSource[k]->cSBin[SourceR][SourceC];                                    //Steven 20220120 : Amlogic需要收SBIN
                b2DIDNG[TargetR][TargetC]     =tmSource[k]->b2DIDNG[SourceR][SourceC];                                  //Steven 20200611 : for Murata, 2DID NG不測試

                iAOIResult[TargetR][TargetC]  =tmSource[k]->iAOIResult[SourceR][SourceC];                               //Sam 20240325 : 新增 DamageTrayMapping 功能
            }
        }

        if(InsertCol)
        {
            iIndex_Col=MinCol;
        }
        else
        {
            iIndex_ROW=MinRow;
        }
    }
}
//------------------------------------------------------------------------------
//2013-01-15    Dell    Add nn Mode
void TMyKitSuck::SplitSuck(class TMyKitSuck &Source1, class TMyKitSuck &Source2, bool InsertCol)
{
    TMyKitSuck *tmSource[]={&Source1, &Source2};
    int MinRow, MinCol;
    if(tmSource[0]->iMaxRow>=iMaxRow)
        MinRow=iMaxRow;
    else
        MinRow=tmSource[0]->iMaxRow;

    if(tmSource[0]->iMaxCol>=iMaxCol)
        MinCol=iMaxCol;
    else
        MinCol=tmSource[0]->iMaxCol;

    int iIndex_ROW=0, iIndex_Col=0;
    int SourceR=0, SourceC=0;
    int TargetR=0, TargetC=0;

    for(int k=0; k<2; k++)
    {
        for(int i=0; i<MinRow; i++)
        {
            for(int j=0; j<MinCol; j++)
            {
                SourceR=i+iIndex_ROW;
                SourceC=j+iIndex_Col;
                TargetR=i;
                TargetC=j;

                tmSource[k]->SetItemData(TargetR, TargetC, Item[SourceR][SourceC]);
                tmSource[k]->iWhichSite[TargetR][TargetC]   =iWhichSite[SourceR][SourceC];
                tmSource[k]->iWhichAuto[TargetR][TargetC]   =iWhichAuto[SourceR][SourceC];
                tmSource[k]->iCurrRotAng[TargetR][TargetC]  =iCurrRotAng[SourceR][SourceC];                             //Steven 20170425 (wei) : Add rotate motor
                tmSource[k]->iNeedRotAng[TargetR][TargetC]  =iNeedRotAng[SourceR][SourceC];
                tmSource[k]->iWhichIndex[TargetR][TargetC]  =iWhichIndex[SourceR][SourceC];                             //ChungHung 20150205 add for ATK
                tmSource[k]->bPass[TargetR][TargetC]        =bPass[SourceR][SourceC];
                tmSource[k]->iCleanCount[TargetR][TargetC]  =iCleanCount[SourceR][SourceC];
                tmSource[k]->bFliped[TargetR][TargetC]      =bFliped[SourceR][SourceC];
                tmSource[k]->iBinData[TargetR][TargetC]     =iBinData[SourceR][SourceC];
                tmSource[k]->iBinDataBackUp[TargetR][TargetC]=iBinDataBackUp[SourceR][SourceC];                         //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                tmSource[k]->PordRec[TargetR][TargetC].asBuffer->CommaText=PordRec[SourceR][SourceC].asBuffer->CommaText;
                tmSource[k]->PordRec[TargetR][TargetC].bUse =PordRec[SourceR][SourceC].bUse;                            //Frank 20160505 add

                SetItemData(SourceR, SourceC, NULL_IC);
                iWhichSite[SourceR][SourceC]  =-1;
                iWhichAuto[SourceR][SourceC]  =-1;
                iWhichIndex[SourceR][SourceC] =-1;                              //ChungHung 20150205 add for ATK
                bPass[SourceR][SourceC]       =false;
                iCleanCount[SourceR][SourceC] =0;
                bFliped[SourceR][SourceC]     =false;
                iBinData[SourceR][SourceC]    =-1;
                iBinDataBackUp[SourceR][SourceC]=-1;                            //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                iCurrRotAng[SourceR][SourceC] =0;                               //Steven 20170425 (wei) : Add rotate motor
                iNeedRotAng[SourceR][SourceC] =0;
                PordRec[SourceR][SourceC].InitialRecord();
            }
        }

        if(InsertCol)
        {
            iIndex_Col=MinCol;
        }
        else
        {
            iIndex_ROW=MinRow;
        }
    }
}
//------------------------------------------------------------------------------
//2013-01-15    Dell    Add nn Mode
void TMyKitSuck::MergeSuck1Row(class TMyKitSuck &Source1, class TMyKitSuck &Source2)
{
    TMyKitSuck *tmSource[]={&Source1, &Source2};
    int MinRow, MinCol;
    if(Source1.iMaxRow>=iMaxRow)
        MinRow=iMaxRow;
    else
        MinRow=Source1.iMaxRow;

    if(Source1.iMaxCol>=iMaxCol)
        MinCol=iMaxCol;
    else
        MinCol=Source1.iMaxCol;

    int SourceR=0, SourceC=0;
    int TargetR=0, TargetC=0;

    for(int k=0; k<2; k++)
    {
        for(int i=0; i<MinRow; i++)
        {
            for(int j=0; j<MinCol; j++)
            {
                SourceR=0;
                SourceC=j;
                TargetR=i;
                TargetC=j;
                k=i;
                SetItemData(TargetR, TargetC, tmSource[k]->Item[SourceR][SourceC]);

                iWhichSite  [TargetR][TargetC]=tmSource[k]->iWhichSite[SourceR][SourceC];
                iWhichAuto  [TargetR][TargetC]=tmSource[k]->iWhichAuto[SourceR][SourceC];
                iCurrRotAng [TargetR][TargetC]=tmSource[k]->iCurrRotAng[SourceR][SourceC];                              //Steven 20170425 (wei) : Add rotate motor
                iNeedRotAng [TargetR][TargetC]=tmSource[k]->iNeedRotAng[SourceR][SourceC];
                iWhichIndex [TargetR][TargetC]=tmSource[k]->iWhichIndex[SourceR][SourceC];                              //ChungHung 20150205 add for ATK

                bPass       [TargetR][TargetC]=tmSource[k]->bPass[SourceR][SourceC];
                iCleanCount [TargetR][TargetC]=tmSource[k]->iCleanCount[SourceR][SourceC];
                bFliped     [TargetR][TargetC]=tmSource[k]->bFliped[SourceR][SourceC];
                iBinData    [TargetR][TargetC]=tmSource[k]->iBinData[SourceR][SourceC];

                PordRec     [TargetR][TargetC].asBuffer->CommaText=tmSource[k]->PordRec[SourceR][SourceC].asBuffer->CommaText;
                PordRec     [TargetR][TargetC].bUse=tmSource[k]->PordRec[SourceR][SourceC].bUse;                        //Frank 20160505 add

                cDeviceInf[TargetR][TargetC]  =tmSource[k]->cDeviceInf[SourceR][SourceC];                               //Steven 20191126 : 補上
                cReDeviceInf[TargetR][TargetC]=tmSource[k]->cReDeviceInf[SourceR][SourceC];
                cSBin[TargetR][TargetC]       =tmSource[k]->cSBin[SourceR][SourceC];                                    //Steven 20220120 : Amlogic需要收SBIN
                b2DIDNG[TargetR][TargetC]     =tmSource[k]->b2DIDNG[SourceR][SourceC];                                  //Steven 20200611 : for Murata, 2DID NG不測試
                iAOIResult[TargetR][TargetC]  =tmSource[k]->iAOIResult[SourceR][SourceC];                               //Sam 20240325 : 新增 DamageTrayMapping 功能
            }
        }
    }
}
//------------------------------------------------------------------------------
void TMyKitSuck::SplitSuck1Row(class TMyKitSuck &Source1, class TMyKitSuck &Source2)
{
    TMyKitSuck *tmSource[]={&Source1, &Source2};
    int MinRow, MinCol;
    if(tmSource[0]->iMaxRow>=iMaxRow)
        MinRow=iMaxRow;
    else
        MinRow=tmSource[0]->iMaxRow;

    if(tmSource[0]->iMaxCol>=iMaxCol)
        MinCol=iMaxCol;
    else
        MinCol=tmSource[0]->iMaxCol;

    int SourceR=0, SourceC=0;
    int TargetR=0, TargetC=0;
    int k=0;
    for(int i=0; i<MinRow; i++)
    {
        for(int j=0; j<MinCol; j++)
        {
            SourceR=i;
            SourceC=j;
            TargetR=0;
            TargetC=j;
            k=i;
            tmSource[k]->SetItemData(TargetR, TargetC, Item[SourceR][SourceC]);

            tmSource[k]->iWhichSite[TargetR][TargetC]   =iWhichSite[SourceR][SourceC];
            tmSource[k]->iWhichAuto[TargetR][TargetC]   =iWhichAuto[SourceR][SourceC];
            tmSource[k]->iCurrRotAng[TargetR][TargetC]  =iCurrRotAng[SourceR][SourceC];                                 //Steven 20170425 (wei) : Add rotate motor
            tmSource[k]->iNeedRotAng[TargetR][TargetC]  =iNeedRotAng[SourceR][SourceC];
            tmSource[k]->iWhichIndex[TargetR][TargetC]  =iWhichIndex[SourceR][SourceC];                                 //ChungHung 20150205 add for ATK
            tmSource[k]->bPass[TargetR][TargetC]        =bPass[SourceR][SourceC];
            tmSource[k]->iCleanCount[TargetR][TargetC]  =iCleanCount[SourceR][SourceC];
            tmSource[k]->bFliped[TargetR][TargetC]      =bFliped[SourceR][SourceC];

            tmSource[k]->iBinData[TargetR][TargetC]     =iBinData[SourceR][SourceC];
            tmSource[k]->iBinDataBackUp[TargetR][TargetC]=iBinDataBackUp[SourceR][SourceC];                             //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
            tmSource[k]->PordRec[TargetR][TargetC].asBuffer->CommaText=PordRec[SourceR][SourceC].asBuffer->CommaText;
            tmSource[k]->PordRec[TargetR][TargetC].bUse =PordRec[SourceR][SourceC].bUse;                                //Frank 20160505 add

            SetItemData(SourceR, SourceC, NULL_IC);
            iWhichSite[SourceR][SourceC]  =-1;
            iWhichAuto[SourceR][SourceC]  =-1;
            iWhichIndex[SourceR][SourceC] =-1;                                  //ChungHung 20150205 add for ATK
            bPass[SourceR][SourceC]       =false;
            iCleanCount[SourceR][SourceC] =0;
            bFliped[SourceR][SourceC]     =false;
            iBinData[SourceR][SourceC]    =-1;
            iBinDataBackUp[SourceR][SourceC]=-1;                                //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
            iCurrRotAng[SourceR][SourceC] =0;                                   //Steven 20170425 (wei) : Add rotate motor
            iNeedRotAng[SourceR][SourceC] =0;
            PordRec[SourceR][SourceC].InitialRecord();
        }
    }
}
//------------------------------------------------------------------------------
void TMyKitSuck::MoveSuckData(class TMyKitSuck &Source, int SourceR, int SourceC, int TargetR, int TargetC)
{
    if(TargetR==-1 || TargetC==-1)
    {
        TargetR=SourceR;
        TargetC=SourceC;
    }

    SetItemData(TargetR, TargetC, Source.Item[SourceR][SourceC]);
    iWhichSite[TargetR][TargetC]    =Source.iWhichSite[SourceR][SourceC];
    iWhichAuto[TargetR][TargetC]    =Source.iWhichAuto[SourceR][SourceC];
    iCurrRotAng[TargetR][TargetC]   =Source.iCurrRotAng[SourceR][SourceC];      //Steven 20170425 (wei) : Add rotate motor
    iNeedRotAng[TargetR][TargetC]   =Source.iNeedRotAng[SourceR][SourceC];
    iWhichIndex[TargetR][TargetC]   =Source.iWhichIndex[SourceR][SourceC];      //ChungHung 20150205 add for ATK
    bPass[TargetR][TargetC]         =Source.bPass[SourceR][SourceC];
    iCleanCount[TargetR][TargetC]   =Source.iCleanCount[SourceR][SourceC];
    bFliped[TargetR][TargetC]       =Source.bFliped[SourceR][SourceC];
    iBinData[TargetR][TargetC]      =Source.iBinData[SourceR][SourceC];
    iBinDataBackUp[TargetR][TargetC]=Source.iBinDataBackUp[SourceR][SourceC];   //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
    iAutoCleanRecX[TargetR][TargetC]=Source.iAutoCleanRecX[SourceR][SourceC];   //jou 2013-03-13 Auto Clean Record X
    iAutoCleanRecY[TargetR][TargetC]=Source.iAutoCleanRecY[SourceR][SourceC];   //jou 2013-03-13 Auto Clean Record Y
    cDeviceInf[TargetR][TargetC]    =Source.cDeviceInf[SourceR][SourceC];
    cReDeviceInf[TargetR][TargetC]  =Source.cReDeviceInf[SourceR][SourceC];
    cSBin[TargetR][TargetC]         =Source.cSBin[SourceR][SourceC];            //Steven 20220120 : Amlogic需要收SBIN
    b2DIDNG[TargetR][TargetC]       =Source.b2DIDNG[SourceR][SourceC];          //Steven 20200611 : for Murata, 2DID NG不測試
    bQATray[TargetR][TargetC]       =Source.bQATray[SourceR][SourceC];

    iAOIResult[TargetR][TargetC]    =Source.iAOIResult[SourceR][SourceC];       //Sam 20240325 : 新增 DamageTrayMapping 功能
    PordRec[TargetR][TargetC].asBuffer->CommaText=Source.PordRec[SourceR][SourceC].asBuffer->CommaText;
    PordRec[TargetR][TargetC].bUse  =Source.PordRec[SourceR][SourceC].bUse;     //Frank 20160505 add
    Source.SetItemData(SourceR, SourceC, NULL_IC);
    Source.PordRec[SourceR][SourceC].InitialRecord();

    if(bLed[TargetR][TargetC]==true)
    {
        if(Item[TargetR][TargetC]==NULL_IC)
            pLed[TargetR][TargetC]->Value=false;
        else
            pLed[TargetR][TargetC]->Value=true;
    }

    Source.iWhichSite[SourceR][SourceC]     =-1;
    Source.iWhichAuto[SourceR][SourceC]     =-1;
    Source.iWhichIndex[SourceR][SourceC]    =-1;                                //ChungHung 20150205 add for ATK
    Source.bPass[SourceR][SourceC]          =false;
    Source.iCleanCount[SourceR][SourceC]    =0;
    Source.bFliped[SourceR][SourceC]        =false;
    Source.iBinData[SourceR][SourceC]       =-1;
    Source.iAutoCleanRecX[SourceR][SourceC] =-1;                                //jou 2013-03-13 Auto Clean Record X
    Source.iAutoCleanRecY[SourceR][SourceC] =-1;                                //jou 2013-03-13 Auto Clean Record Y
    Source.iCurrRotAng[SourceR][SourceC]    =0;                                 //Steven 20170425 (wei) : Add rotate motor
    Source.iNeedRotAng[SourceR][SourceC]    =0;
    Source.cDeviceInf[SourceR][SourceC]     ="";
    Source.cReDeviceInf[SourceR][SourceC]   ="";
    Source.cSBin[SourceR][SourceC]          ="";                                //Steven 20220120 : Amlogic需要收SBIN
    Source.b2DIDNG[SourceR][SourceC]        =false;                             //Steven 20200611 : for Murata, 2DID NG不測試
    Source.bQATray[SourceR][SourceC]        =false;
    Source.iAOIResult[SourceR][SourceC]     =0;                                 //Sam 20240325 : 新增 DamageTrayMapping 功能
}
//------------------------------------------------------------------------------
void TMyKitSuck::MoveSuckDataDiff(class TMyKitSuck &Source, int SourceR, int SourceC, int TargetR, int TargetC)
{
    SetItemData(TargetR, TargetC, Source.Item[SourceR][SourceC]);

    iWhichSite[TargetR][TargetC]      =Source.iWhichSite[SourceR][SourceC];
    iWhichAuto[TargetR][TargetC]      =Source.iWhichAuto[SourceR][SourceC];
    iWhichIndex[TargetR][TargetC]     =Source.iWhichIndex[SourceR][SourceC];    //ChungHung 20150205 add for ATK
    iCurrRotAng[TargetR][TargetC]     =Source.iCurrRotAng[SourceR][SourceC];    //Steven 20170425 (wei) : Add rotate motor
    iNeedRotAng[TargetR][TargetC]     =Source.iNeedRotAng[SourceR][SourceC];

    bPass[TargetR][TargetC]           =Source.bPass[SourceR][SourceC];
    iCleanCount[TargetR][TargetC]     =Source.iCleanCount[SourceR][SourceC];
    bFliped[TargetR][TargetC]         =Source.bFliped[SourceR][SourceC];
    iBinData[TargetR][TargetC]        =Source.iBinData[SourceR][SourceC];
    iBinDataBackUp[TargetR][TargetC]  =Source.iBinDataBackUp[SourceR][SourceC];                                         //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount

    iAutoCleanRecX[TargetR][TargetC]  =Source.iAutoCleanRecX[SourceR][SourceC];                                         //jou 2013-03-13 Auto Clean Record X
    iAutoCleanRecY[TargetR][TargetC]  =Source.iAutoCleanRecY[SourceR][SourceC];                                         //jou 2013-03-13 Auto Clean Record Y
    cDeviceInf[TargetR][TargetC]      =Source.cDeviceInf[SourceR][SourceC];
    b2DIDNG[TargetR][TargetC]         =Source.b2DIDNG[SourceR][SourceC];        //Steven 20200611 : for Murata, 2DID NG不測試
    cReDeviceInf[TargetR][TargetC]    =Source.cReDeviceInf[SourceR][SourceC];
    cSBin[TargetR][TargetC]           =Source.cSBin[SourceR][SourceC];          //Steven 20220120 : Amlogic需要收SBIN
    bQATray[TargetR][TargetC]         =Source.bQATray[SourceR][SourceC];
    iAOIResult[TargetR][TargetC]      =Source.iAOIResult[SourceR][SourceC];     //Sam 20240325 : 新增 DamageTrayMapping 功能

    PordRec[TargetR][TargetC].asBuffer->CommaText=Source.PordRec[SourceR][SourceC].asBuffer->CommaText;
    PordRec[TargetR][TargetC].bUse=Source.PordRec[SourceR][SourceC].bUse;       //Frank 20160505 add

    Source.SetItemData(SourceR, SourceC, NULL_IC);
    if(bLed[TargetR][TargetC]==true)
    {
        if(Item[TargetR][TargetC]==NULL_IC)
            pLed[TargetR][TargetC]->Value=false;
        else
            pLed[TargetR][TargetC]->Value=true;
    }

    Source.iWhichSite[SourceR][SourceC]       =-1;
    Source.iWhichAuto[SourceR][SourceC]       =-1;
    Source.iWhichIndex[SourceR][SourceC]      =-1;                              //ChungHung 20150205 add for ATK

    Source.bPass[SourceR][SourceC]            =false;
    Source.iCleanCount[SourceR][SourceC]      =0;
    Source.bFliped[SourceR][SourceC]          =false;

    Source.iBinData[SourceR][SourceC]         =-1;
    Source.iAutoCleanRecX[SourceR][SourceC]   =-1;                              //jou 2013-03-13 Auto Clean Record X
    Source.iAutoCleanRecY[SourceR][SourceC]   =-1;                              //jou 2013-03-13 Auto Clean Record Y

    Source.cDeviceInf[SourceR][SourceC]       ="";
    Source.cReDeviceInf[SourceR][SourceC]     ="";
    Source.cSBin[SourceR][SourceC]            ="";                              //Steven 20220120 : Amlogic需要收SBIN
    Source.b2DIDNG[SourceR][SourceC]          =false;                           //Steven 20200611 : for Murata, 2DID NG不測試

    Source.iCurrRotAng[SourceR][SourceC]      =0;                               //Steven 20170425 (wei) : Add rotate motor
    Source.iNeedRotAng[SourceR][SourceC]      =0;
    Source.bQATray[SourceR][SourceC]          =false;
    Source.iAOIResult[SourceR][SourceC]       =0;                               //Sam 20240325 : 新增 DamageTrayMapping 功能
}
//------------------------------------------------------------------------------
void TMyKitSuck::CopyFromTray(int iSuckR, int iSuckC, int iSuckData, class TTrayMotor &Mot, int TrayR, int TrayC, int iTrayData, int iTarget, bool bHP2Inarm)   //Steven 20150714 : Manual Sort
{
    SetItemData(iSuckR, iSuckC, iSuckData, iTarget);
    Mot.SetTraySingleData(TrayC, TrayR, iTrayData);

    iWhichSite[iSuckR][iSuckC]      =Mot.Tray.iWhichSite[TrayC][TrayR];         //Steven 20220527 : for JCET Auto Site Map
    cDeviceInf[iSuckR][iSuckC]      =Mot.Tray.cDeviceInf[TrayC][TrayR];
    cReDeviceInf[iSuckR][iSuckC]    =Mot.Tray.cReDeviceInf[TrayC][TrayR];
    cSBin[iSuckR][iSuckC]           =Mot.Tray.cSBin[TrayC][TrayR];              //Steven 20220120 : Amlogic需要收SBIN
    b2DIDNG[iSuckR][iSuckC]         =Mot.Tray.b2DIDNG[TrayC][TrayR];            //Steven 20200611 : for Murata, 2DID NG不測試

    PordRec[iSuckR][iSuckC].asBuffer->CommaText=Mot.Tray.PordRec[TrayC][TrayR]->asBuffer->CommaText;                    //Steven 20221005 : Production Log減少記憶體使用量
    PordRec[iSuckR][iSuckC].bUse    =Mot.Tray.PordRec[TrayC][TrayR]->bUse;      //Frank 20160505 add    //Steven 20221005 : Production Log減少記憶體使用量
    iNeedRotAng[iSuckR][iSuckC]     =Mot.Tray.iNeedRotAng[TrayC][TrayR];        //預計旋轉的角度   //Steven 20170425 (wei) : Add rotate motor
    iCurrRotAng[iSuckR][iSuckC]     =Mot.Tray.iCurrRotAng[TrayC][TrayR];        //目前產品的角度
    iWhichAuto[iSuckR][iSuckC]      =Mot.Tray.iWhichAuto[TrayC][TrayR];         //JerryYang 20220909 : add magazine
    iBinData[iSuckR][iSuckC]        =Mot.Tray.iBinData[TrayC][TrayR];           //JerryYang 20220909 : add magazine
    Mot.Tray.iBinData[TrayC][TrayR] =-1;                                        //JerryYang 20220909 : add magazine
    iAOIResult[iSuckR][iSuckC]      =Mot.Tray.iAOIResult[TrayC][TrayR];         //Sam 20240325 : 新增 DamageTrayMapping 功能

    if(bRunAutoSiteMapping==true &&                                             //Ifor 20170928 (Steven) : add Auto Site Mapping Use Hotplat Data Backup
       bHP2Inarm==true)                                                         //Ifor 20180518 : add 簡化Site Mapping 旗標
    {
        iWhichShuttleAutoSitemapping[iSuckR][iSuckC] = InArmSuck.iWhichShuttleBackup;
        iWhichKitAutoSitemapping    [iSuckR][iSuckC] = InArmSuck.iWhichKitBackup;
        iHotCountAutoSitemapping    [iSuckR][iSuckC] = InArmSuck.HotCount;
        if(iSuckData==HAS_HOT_IC)
        {
            bAutoSiteMapHotplateSave=true;
            Mot.Tray.SiteMapData[TrayC][TrayR]=1;
            Mot.SetTraySingleData(TrayC, TrayR, HAS_NULL_IC);                   //Ifor 20210423 add: Auto Site Mapping 先補回Has Null IC 避免Hotplate 資料異常發生Hang up
        }
    }
    else
    {
        Mot.Tray.PordRec[TrayC][TrayR]->InitialRecord();                        //Steven 20240508 : 放完後要清空資料

        Mot.Tray.cDeviceInf[TrayC][TrayR]     ="";
        Mot.Tray.cReDeviceInf[TrayC][TrayR]   ="";
        Mot.Tray.cSBin[TrayC][TrayR]          ="";
        Mot.Tray.b2DIDNG[TrayC][TrayR]        =false;

        Mot.Tray.iWhichSite[TrayC][TrayR]     =-1;
        Mot.Tray.iWhichAuto[TrayC][TrayR]     =-1;
        Mot.Tray.iBinData[TrayC][TrayR]       =-1;
        Mot.Tray.iCurrRotAng[TrayC][TrayR]    =0;
        Mot.Tray.iNeedRotAng[TrayC][TrayR]    =0;
        Mot.Tray.iAOIResult[TrayC][TrayR]     =0;
    }
}
//------------------------------------------------------------------------------
void TMyKitSuck::CopyToTray(int iSuckR, int iSuckC, int iSuckData, class TTrayMotor &Mot, int TrayR, int TrayC, int iTrayData, int iTarget)                     //Steven 20150714 : Manual Sort
{
    if(bRunAutoSiteMapping==true &&                                             //Ifor 20180518 : add 簡化Site Mapping 旗標
       bAutoSiteMapHotplateSave==true)                                          //Ifor 20170928 (Steven) : add Auto Site Mapping Use Hotplat Data Backup
    {
        InArmSuck.iWhichShuttleBackup   =iWhichShuttleAutoSitemapping   [iSuckR][iSuckC];
        InArmSuck.iWhichKitBackup       =iWhichKitAutoSitemapping       [iSuckR][iSuckC];
        InArmSuck.HotCount              =iHotCountAutoSitemapping       [iSuckR][iSuckC];
        iAutoSiteMapHotplatePlateC      =TrayC;
        iAutoSiteMapHotplatePlateR      =TrayR;
        if(iTrayData==HAS_NULL_IC)
        {
            SetItemData(iSuckR, iSuckC, iSuckData, iTarget);
            return;
        }
    }

    Mot.SetTraySingleData(TrayC, TrayR, iTrayData);
    Mot.Tray.iWhichSite[TrayC][TrayR]   =iWhichSite[iSuckR][iSuckC];            //Steven 20220527 : for JCET Auto Site Map
    Mot.Tray.cDeviceInf[TrayC][TrayR]   =cDeviceInf[iSuckR][iSuckC];
    Mot.Tray.b2DIDNG[TrayC][TrayR]      =b2DIDNG[iSuckR][iSuckC];               //Steven 20200611 : for Murata, 2DID NG不測試
    Mot.Tray.cReDeviceInf[TrayC][TrayR] =cReDeviceInf[iSuckR][iSuckC];
    Mot.Tray.cSBin[TrayC][TrayR]        =cSBin[iSuckR][iSuckC];                 //Steven 20220120 : Amlogic需要收SBIN

    Mot.Tray.PordRec[TrayC][TrayR]->asBuffer->CommaText=PordRec[iSuckR][iSuckC].asBuffer->CommaText;                    //Steven 20221005 : Production Log減少記憶體使用量
    Mot.Tray.PordRec[TrayC][TrayR]->bUse=PordRec[iSuckR][iSuckC].bUse;          //Frank 20160505 add    //Steven 20221005 : Production Log減少記憶體使用量
    Mot.Tray.iNeedRotAng[TrayC][TrayR]  =iNeedRotAng[iSuckR][iSuckC];           //預計旋轉的角度   //Steven 20170425 (wei) : Add rotate motor
    Mot.Tray.iCurrRotAng[TrayC][TrayR]  =iCurrRotAng[iSuckR][iSuckC];           //目前產品的角度
    Mot.Tray.SiteMapData[TrayC][TrayR]  =0;                                     //Ifor 20210423 add: Auto Site Mapping 先補回Has Null IC 避免Hotplate 資料異常發生Hang up
    Mot.Tray.iWhichAuto[TrayC][TrayR]   =iWhichAuto[iSuckR][iSuckC];            //JerryYang 20231004 : Fixed for data error
    Mot.Tray.iBinData[TrayC][TrayR]     =iBinData[iSuckR][iSuckC]  ;            //JerryYang 20231004 : Fixed for data error
    Mot.Tray.iAOIResult[TrayC][TrayR]   =iAOIResult[iSuckR][iSuckC];            //Sam 20240325 : 新增 DamageTrayMapping 功能
    SetItemData(iSuckR, iSuckC, iSuckData, iTarget);

    PordRec[iSuckR][iSuckC].InitialRecord();                                    //Steven 20240508 : 放完後要清空資料

    cDeviceInf[iSuckR][iSuckC]     ="";
    cReDeviceInf[iSuckR][iSuckC]   ="";
    cSBin[iSuckR][iSuckC]          ="";
    b2DIDNG[iSuckR][iSuckC]        =false;

    iWhichSite[iSuckR][iSuckC]     =-1;
    iWhichAuto[iSuckR][iSuckC]     =-1;
    iBinData[iSuckR][iSuckC]       =-1;
    iCurrRotAng[iSuckR][iSuckC]    =0;
    iNeedRotAng[iSuckR][iSuckC]    =0;
    iAOIResult[iSuckR][iSuckC]     =0;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::AlreadyTest()
{
    bool flag=false;                                                            //Steven 20180907 : 修正避免完全沒IC也回True
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]>=TEST_PASS)                                           // && Item[i][j]<(TEST_PASS+iTestBinCount))//Steven 20200507 : 修正Test time out之後按SKIP,機台會hang up
            {
                flag=true;
            }

            if(Item[i][j]>(TEST_PASS+iTestBinCount-1) &&
               Item[i][j]!=(TEST_PASS+iTestBinCount))                           //Steven 20121112 : RS232支援32Bin 14->iTestBinCount-1, 1016->TEST_PASS+iTestBinCount
            {
                Item[i][j]=TEST_PASS+iTestBinCount;
            }
        }
    }
    return flag;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::AlreadyTestNotIncludeErrorBin()                                //Steven 20200611 : for Murata, 2DID NG不測試
{
    bool flag=false;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]>=TEST_PASS && Item[i][j]<(TEST_PASS+iTestBinCount))
            {
                flag=true;
            }
        }
    }
    return flag;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::PartAlreadyTest()
{
    bool flag1=false, flag2=false;

    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_IC)
                flag1=true;

            if(Item[i][j]>=TEST_PASS)
                flag2=false;
        }
    }

    if(flag1 && flag2)
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::HasType(int p)
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==p)
                return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
TMySucker::TMySucker()
{
    bIsRotate=false;  W906_OffDestroyStamp=0; W906_OffDestroyRaw=0;  // AI(W906-A4-6) 20260924: 觀測接縫初值（0 = 從未蓋章）
    Enable=false;  W906_iGroupCount=0; W906_pGroupMaster=0; W906_bInFan=false; W906_dwHoldLostStart=0; for(int g=0; g<4; g++) W906_pGroup[g]=0;  // AI(W906-F03-QUADVAC) 20261004: 沒有群組＝golden 行為
    Status=false;
    OnTask=1;
    OffTask=1;
    OnOff=2;
    AlarmEnable=false;
    OnAlarmTime=100;
    OffAlarmTime=100;
    OnDelayTime=0;
    OffDelayTime=0;

    OnUsing="";
    OnRing=0;
    OnIP=0;
    OnPort=0;                                                                   // 真空產生
    OnBit=0;
    OnType=0;                                                                   // A or B Type
    OnEnable=false;

    OffUsing="";
    OffRing=0;
    OffIP=0;
    OffPort=0;                                                                  // 真空破壞
    OffBit=0;
    OffType=0;                                                                  // A or B Type
    OffEnable=false;

    SenUsing="";
    SenRing=0;
    SenIP=0;
    SenPort=0;
    SenBit=0;
    SenType=0;

    Error=false;
    fAlarm=true;
    RetryCT=1;
    rct=1;
    ract=1;                                                                     //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    iManualOffTask=1;
    VacuumOnTime=120;                                                           //75    100    110
    VacuumOffTime=70;                                                           // 35     50      60
                                                                                // 3400  3300
    for(int i=0; i<20; i++)
    {
        VacuumOnTimeBuffer[i]=-1;
        VacuumOffTimeBuffer[i]=-1;
    }
    bNeedSuck=false;
    bNeedDestroy=false;
    RealTimeRefreshVacuumOnOffTime=false;                                       //Eliot 2008_04_17

    bSuckOK=true;                                                               //ChungHung 20110901 add
    bDestroyOK=true;                                                            //ChungHung 20110901 add
    iNozzleEvent=0;                                                             //Klutter 20210817 方便修正吸嘴回home異常事件             //Steven 20210825 : 吹氣完成才可以歸零
    ISABase=0;                                                                  //Nickliu 20230306 add IO Use ISABase

    sRecordStartOffDelayTime="";                                                //Sam 20220329 : Record Index Air On Time
    sRecordEndOffDelayTime="";
    iRecordOffDelayTime=0.0;
};
//------------------------------------------------------------------------------
TMySucker::~TMySucker()
{
}
//------------------------------------------------------------------------------
void TMySucker::PushOnTime()
{
    int ct=0;
    DWORD iSum=0;
    for(int i=0; i<19; i++)
        VacuumOnTimeBuffer[i]=VacuumOnTimeBuffer[i+1];
    VacuumOnTimeBuffer[19]=VacuumOnTimeRecord;
    for(int i=0; i<20 ; i++)
    {
        if(VacuumOnTimeBuffer[i]!=-1)
        {
            ct++;
            iSum+=VacuumOnTimeBuffer[i];
        }
    }

    if(ct)
        VacuumOnTime=ChangeToFloatNonPcnt((double)(iSum), (double)(ct));
    else
        return;
}
//------------------------------------------------------------------------------
void TMySucker::PushOffTime()
{
    int ct=0;
    DWORD iSum=0;
    for(int i=0; i<19; i++)
        VacuumOffTimeBuffer[i]=VacuumOffTimeBuffer[i+1];
    VacuumOffTimeBuffer[19]=VacuumOffTimeRecord;
    for(int i=0; i<20; i++)
    {
        if(VacuumOffTimeBuffer[i]!=-1)
        {
            ct++;
            iSum+=VacuumOffTimeBuffer[i];
        }
    }

    if(ct)
        VacuumOffTime=ChangeToFloatNonPcnt((double)(iSum), (double)(ct));
    else
        return;
}
//------------------------------------------------------------------------------
void TMySucker::ReStart()
{
    OnTask=1;
    OffTask=1;
}
//------------------------------------------------------------------------------
void TMySucker::Reset()
{
    ReStart();
    Error=false;
    iNozzleEvent=0;                                                             //Klutter 20210817 方便修正吸嘴回home異常事件             //Steven 20210825 : 吹氣完成才可以歸零
}
//------------------------------------------------------------------------------
bool TMySucker::GetOnBit()
{
    bool ret;
    if(OnEnable)
    {
        if(OnISABase==eMotionNet ||
           OnISABase==ePCI1203)                                                 //Sam 20230724 : 新增 PCI1203 IO 模組
        {
            ret=MyLaneIO.IOOutBitStatus(OnRing, OnIP, OnPort, OnBit, OnISABase, OnPortName);                            //Sam 20230724 : 新增 PCI1203 IO 模組
        }
        else if(OnISABase==eISABase ||                                          //Nickliu 20230309 add Cylinder ISABase Type
                OnISABase==ePCI1735U ||
                OnISABase==ePLCbase)
        {
            ret=IOOutBitStatus(OnPort, OnBit);
        }

        if(OnType==TYPE_A)
            return ret;
        else
            return !ret;
    }
    else
    {
        return false;
    }
}
//------------------------------------------------------------------------------
bool TMySucker::GetOffBit()
{
    bool ret;
    if(OffEnable)
    {
        if(OffISABase==eMotionNet ||
           OffISABase==ePCI1203)                                                //Sam 20230724 : 新增 PCI1203 IO 模組
        {
            ret=MyLaneIO.IOOutBitStatus(OffRing, OffIP, OffPort, OffBit, OffISABase, OffPortName);                      //Sam 20230724 : 新增 PCI1203 IO 模組
        }
        else if(OffISABase==eISABase ||                                         //Nickliu 20230309 add Cylinder ISABase Type
                OffISABase==ePCI1735U ||
                OffISABase==ePLCbase)
        {
            ret=IOOutBitStatus(OffPort, OffBit);
        }

        if(OffType==TYPE_A)
            return ret;
        else
            return !ret;
    }
    else
    {
        return false;
    }
}
//------------------------------------------------------------------------------
bool TMySucker::GetStatus()
{
    bool InRet;  if(W906_pGroupMaster!=0) return false;  // AI(W906-F03-QUADVAC) 20261004: 群組成員只透過主吸嘴的 W906_GetStatusAllOn／AnyOn 讀（檔尾），流程直接讀它一律當沒有真空
    if(Enable==false)
        return false;
    if(SenISABase==eMotionNet ||
       SenISABase==ePCI1203)                                                    //Sam 20230724 : 新增 PCI1203 IO 模組
    {
        InRet=MyLaneIO.IOInputBit(SenRing, SenIP, SenPort, SenBit, SenISABase, SensorName);                             //Sam 20230724 : 新增 PCI1203 IO 模組
    }
    else if(SenISABase==eISABase ||                                             //Nickliu 20230309 add Cylinder ISABase Type
            SenISABase==ePCI1735U ||
            SenISABase==ePLCbase)
    {
        InRet=IOInputBit(SenPort, SenBit);
    }

    if(SenType==TYPE_A && InRet==1)
        return true;
    else if(SenType==TYPE_B && InRet==0)
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMySucker::Sensor()
{
    if(Enable==false)
        return true;
    else
        return GetStatus();
}
//------------------------------------------------------------------------------
void TMySucker::DoOnIO(bool bOn)                                                //Steven 20230721 : 統一真空開關控制
{   if(W906_FanOut(true, bOn)) return;  // AI(W906-F03-QUADVAC) 20261004: 群組主吸嘴 → 4 個成員的開真空閥一起動；成員被單獨叫 → 不動（檔尾 W906_FanOut）
    if(OnEnable==true)
    {
        if(bOn==true)                                                           //開啟真空
        {
            if(OnType==TYPE_A)
            {
                if(OnISABase==eMotionNet ||
                   OnISABase==ePCI1203)                                         //Sam 20230724 : 新增 PCI1203 IO 模組
                {
                    MyLaneIO.IOBitOn(OnRing, OnIP, OnPort, OnBit, OnISABase, OnPortName);                               //Sam 20230724 : 新增 PCI1203 IO 模組
                }
                else if(OnISABase==eISABase ||                                  //Nickliu 20230309 add Cylinder ISABase Type
                        OnISABase==ePCI1735U ||
                        OnISABase==ePLCbase)
                {
                    IOBitOn(OnPort, OnBit);
                }
            }
            else
            {
                if(OnISABase==eMotionNet ||
                   OnISABase==ePCI1203)                                         //Sam 20230724 : 新增 PCI1203 IO 模組
                {
                    MyLaneIO.IOBitOff(OnRing, OnIP, OnPort, OnBit, OnISABase, OnPortName);                              //Sam 20230724 : 新增 PCI1203 IO 模組
                }
                else if(OnISABase==eISABase ||                                  //Nickliu 20230309 add Cylinder ISABase Type
                        OnISABase==ePCI1735U ||
                        OnISABase==ePLCbase)
                {
                    IOBitOff(OnPort, OnBit);
                }
            }
        }
        else                                                                    //關閉真空
        {
            if(OnType==TYPE_A)
            {
                if(OnISABase==eMotionNet ||
                   OnISABase==ePCI1203)                                         //Sam 20230724 : 新增 PCI1203 IO 模組
                {
                    MyLaneIO.IOBitOff(OnRing, OnIP, OnPort, OnBit, OnISABase, OnPortName);                              //Sam 20230724 : 新增 PCI1203 IO 模組
                }
                else if(OnISABase==eISABase ||                                  //Nickliu 20230309 add Cylinder ISABase Type
                        OnISABase==ePCI1735U ||
                        OnISABase==ePLCbase)
                {
                    IOBitOff(OnPort, OnBit);
                }
            }
            else
            {
                if(OnISABase==eMotionNet ||
                   OnISABase==ePCI1203)                                         //Sam 20230724 : 新增 PCI1203 IO 模組
                {
                    MyLaneIO.IOBitOn(OnRing, OnIP, OnPort, OnBit, OnISABase, OnPortName);                               //Sam 20230724 : 新增 PCI1203 IO 模組
                }
                else if(OnISABase==eISABase ||                                  //Nickliu 20230309 add Cylinder ISABase Type
                        OnISABase==ePCI1735U ||
                        OnISABase==ePLCbase)
                {
                    IOBitOn(OnPort, OnBit);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void TMySucker::DoOffIO(bool bOn)                                               //Steven 20230721 : 統一破壞開關控制
{   if(W906_FanOut(false, bOn)) return;  // AI(W906-F03-QUADVAC) 20261004: 同 DoOnIO，破真空閥
    if(OffEnable==true)
    {
        if(bOn==true)                                                           //開啟破壞
        {
            if(OffType==TYPE_A)
            {
                if(OnISABase==eMotionNet ||
                   OnISABase==ePCI1203)                                         //Sam 20230724 : 新增 PCI1203 IO 模組
                {
                    MyLaneIO.IOBitOn(OffRing, OffIP, OffPort, OffBit, OnISABase, OffPortName);                          //Sam 20230724 : 新增 PCI1203 IO 模組
                }
                else if(OnISABase==eISABase ||                                  //Nickliu 20230309 add Cylinder ISABase Type
                        OnISABase==ePCI1735U ||
                        OnISABase==ePLCbase)
                {
                    IOBitOn(OffPort, OffBit);
                }
            }
            else
            {
                if(OnISABase==eMotionNet ||
                   OnISABase==ePCI1203)                                         //Sam 20230724 : 新增 PCI1203 IO 模組
                {
                    MyLaneIO.IOBitOff(OffRing, OffIP, OffPort, OffBit, OnISABase, OffPortName);                         //Sam 20230724 : 新增 PCI1203 IO 模組
                }
                else if(OnISABase==eISABase ||                                  //Nickliu 20230309 add Cylinder ISABase Type
                        OnISABase==ePCI1735U ||
                        OnISABase==ePLCbase)
                {
                    IOBitOff(OffPort, OffBit);
                }
            }
        }
        else                                                                    //關閉破壞
        {
            if(OffType==TYPE_A)
            {
                if(OnISABase==eMotionNet ||
                   OnISABase==ePCI1203)                                         //Sam 20230724 : 新增 PCI1203 IO 模組
                {
                    MyLaneIO.IOBitOff(OffRing, OffIP, OffPort, OffBit, OnISABase, OffPortName);                         //Sam 20230724 : 新增 PCI1203 IO 模組
                }
                else if(OnISABase==eISABase ||                                  //Nickliu 20230309 add Cylinder ISABase Type
                        OnISABase==ePCI1735U ||
                        OnISABase==ePLCbase)
                {
                    IOBitOff(OffPort, OffBit);
                }
            }
            else
            {
                if(OnISABase==eMotionNet ||
                   OnISABase==ePCI1203)                                         //Sam 20230724 : 新增 PCI1203 IO 模組
                {
                    MyLaneIO.IOBitOn(OffRing, OffIP, OffPort, OffBit, OnISABase, OffPortName);                          //Sam 20230724 : 新增 PCI1203 IO 模組
                }
                else if(OnISABase==eISABase ||                                  //Nickliu 20230309 add Cylinder ISABase Type
                        OnISABase==ePCI1735U ||
                        OnISABase==ePLCbase)
                {
                    IOBitOn(OffPort, OffBit);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void TMySucker::OnSuck()                                                        // no delay,no alarm
{
    if(LastSet.iRealDummy==DUMMY)
        return;
    if(LastSet.iRealDummy==HAS_TRAY && SuckerName!="CatchSuck")                 //AI(W906-PT-W3) 20260807: vclcompat::AnsiString has no AnsiCompare() -- established tree-wide substitution `.AnsiCompare(x)!=0` -> `!=x` (precedent: BarCode_8CCD_Glue.cpp:1068)
        return;

    DoOnIO(true);                                                               //開啟真空
    bSuckOK=false;                                                              //ChungHung 20110901 add
}
//------------------------------------------------------------------------------
bool TMySucker::OffSuck()                                                       // no delay,no alarm
{
    DoOnIO(false);                                                              //關閉真空
    bSuckOK=true;                                                               //ChungHung 20110901 add
    return true;
}
//------------------------------------------------------------------------------
void TMySucker::OnDestroy()                                                     // no delay,no alarm
{
    DoOffIO(true);                                                              //打開破壞
    bDestroyOK=false;                                                           //ChungHung 20110901 add
}
//------------------------------------------------------------------------------
void TMySucker::OffDestroy()                                                    // no delay,no alarm
{
    DoOffIO(false);                                                             //關閉破壞
    bDestroyOK=true;  W906_OffDestroySeamHit();  // AI(W906-A4-6) 20260924: 觀測接縫計數（只計數，不影響 IO），見檔尾  （原行尾註解：ChungHung 20110901 add）
}
//------------------------------------------------------------------------------
void TMySucker::On()                                                            // no delay,no alarm
{
    if(LastSet.iRealDummy==DUMMY)
        return;

    if(bLoadInarmAutoHigh==false && LastSet.iRealDummy==HAS_TRAY && SuckerName!="CatchSuck")
        return;                                                                 //kevin 20171107 (wei) Auto teach open vacumm

    Status=true;
    DoOffIO(false);                                                             //關閉破壞
    DoOnIO(true);                                                               //開啟真空
    bSuckOK=false;                                                              //ChungHung 20110901 add
    bDestroyOK=true;
}
//------------------------------------------------------------------------------
void TMySucker::Off()                                                           // no delay,no alarm
{
    Status=false;
    DoOnIO(false);                                                              //關閉真空
    DoOffIO(true);                                                              //打開破壞
    bSuckOK=true;
    bDestroyOK=false;                                                           //ChungHung 20110901 add
}
//------------------------------------------------------------------------------
void TMySucker::Normal()
{
    OffSuck();
    OffDestroy();
    bSuckOK=true;                                                               //ChungHung 20110901 add
    bDestroyOK=true;                                                            //ChungHung 20110901 add
}
//------------------------------------------------------------------------------
bool TMySucker::IsSuckFinish()
{
    return bSuckOK;
}
//------------------------------------------------------------------------------
bool TMySucker::IsDestroyFinish()
{
    return bDestroyOK;
}
//------------------------------------------------------------------------------
bool TMySucker::Suck()
{
    bool InRet;                                                                 //不能直接使用 On() 因為會直接改變 Status
//    iNozzleEvent=1;                                                           //Klutter 20210817 修改屬性方便修正吸嘴回home異常事件
    bSuckOK=false;                                                              //ChungHung 20110901 add
    Error=false;
    if(LastSet.iRealDummy==DUMMY ||
       LastSet.iRealDummy==HAS_TRAY||                                           //kevin 20150618
      (LastSet.iRealDummy==HAS_TRAY && SuckerName!="CatchSuck"))
    {
        switch(OnTask)
        {
            case 1:
                if(VacuumOnTime>2000)                                           //2007/04/24 lee start
                    VacuumOnTime=100;

                TOn.SetMSAndOn(VacuumOnTime);
                OnTask=2;

                if(SuckerName.AnsiPos("InArmSuck")!=0 ||
                   SuckerName.AnsiPos("OutArmSuck")!=0)                         //Steven 20141016 : Dummy Run也要開真空破壞
                {
                    DoOffIO(false);                                             //關閉破壞
                }

                break;
            case 2:
                if(TOn.Off())
                {
                    TOnDelay.SetMSAndOn(OnDelayTime*10);
                    OnTask=50;
                }
                break;
            case 50:
                if(TOnDelay.Off())
                {
                    OnTask=1;
                    InitSuckFlag();
                    bSuckOK=true;                                               //ChungHung 20110901 add
                    return true;
                }
                break;
        }
        return false;
    }

    if(OnTask!=200)
    {
        DoOffIO(false);                                                         //關閉破壞
        DoOnIO(true);                                                           //開啟真空
    }

    if(OnTask==1 || OnTask==2)
    {
        if(Enable==true)                                                        //  has install onsensor
        {
            if(OnAlarmTime==0)
            {
                InRet=W906_GetStatusAllOn();  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都 ON 才算吸到
                if(InRet)                                                       // ok
                {
                    OnTask=100;                                                 // do on delay
                }
                else
                {
                    OnTask=1;
                    rct--;
                    if(rct>0)
                        return false;
                    Normal();                                                   //jou 2011-11-01 開真空不可能同時開破壞，所以OffSuck -> Normal
                    Error=true;
                    bSuckOK=true;                                               //ChungHung 20110901 add
//                    iNozzleEvent=0;                                           //Klutter 20210817 修改屬性方便修正吸嘴回home異常事件
                    return false;
                }
            }
            else
            {
//                StartTick=MyTickCount();
                tSuckTimer.LatchCycleTime(true);
                if(W906_GetStatusAllOn())                                                 // already on  AI(W906-F03-QUADVAC) 20261004
                {
                    OnTask=100;
                }
                else
                {
                    TOn.SetMSAndOn(OnAlarmTime*10);
                    OnTask=50;
                }
            }
        }
        else
        {
            OnTask=100;                                                         //need delay
        }
    }

    if(OnTask==50)
    {
        InRet=W906_GetStatusAllOn();  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都 ON 才算吸到
        if(InRet)                                                               // ok
        {
            OnTask=100;                                                         // do on delay
        }
        else
        {
//            if(bHandlerPause)                                                 //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
//            {
//                TOn.Set0_1SecAndOn(OnAlarmTime*10);
//                return false;
//            }

            if(TOn.Off())
            {
                if(INDEX_SUCKER_TYPE!=1)                                        //jou 2010-05-24 start : 負壓
                {
                    OnTask=1;
                    rct--;
                    if(rct>0)
                        return false;
                }

                Normal();                                                       //jou 2011-11-01 開真空不可能同時開破壞，所以OffSuck -> Normal
                OnTask=200;
                TOn.SetMSAndOn(20);                                             //jou 2011-01-24 10 -> 20 避免掉IC
                //Error=true;
                return false;
            }
            else
            {
                return false;
            }
        }
    }

    if(OnTask>=100)
    {
        if(OnTask!=200 && Enable==true)
        {
            InRet=W906_GetStatusAllOn();  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都 ON 才算吸到
            if(InRet==false)
            {
                OnTask=1;
                return false;
            }
        }

        switch(OnTask)
        {
            case 100:
                VacuumOnTimeRecord=tSuckTimer.LatchCycleTime();
                if(RealTimeRefreshVacuumOnOffTime)
                {
                    PushOnTime();
                }

                if(OnDelayTime==0)
                {
                    OnTask=1;
                    rct=RetryCT;
                    Error=false;
                    InitSuckFlag();
                    bSuckOK=true;                                               //ChungHung 20110901 add
                    return true;
                }
                else
                {
                    TOnDelay.SetMSAndOn(OnDelayTime*10);
                    OnTask=101;
//                    break;                                                    //jou 2012-01-04 加上break,不然會一直卡在Case 101: TOnDelay.Off()
                    return false;                                               //jou 2012-05-24 break會直接return true,沒有做delay的動作。
                }
            case 101:
                if(TOnDelay.Off())
                {
                    rct=RetryCT;
                    Error=false;
                    OnTask=1;
                    InitSuckFlag();
                    bSuckOK=true;                                               //ChungHung 20110901 add
                    return true;
                }
                return false;
            case 200:
                if(TOn.Off())
                {
                    Error=true;
                    OnTask=1;
                    bSuckOK=true;                                               //ChungHung 20110901 add
                    return false;
                }
                return false;
        }
    }
    Error=false;
    OnTask=1;
    InitSuckFlag();
    bSuckOK=true;                                                               //ChungHung 20110901 add
    return true;
}
//------------------------------------------------------------------------------
bool TMySucker::Destroy()
{
    bool InRet;                                                                 //不能直接使用 On() 因為會直接改變 Status

    bDestroyOK=false;                                                           //ChungHung 20110901 add
    Error=false;
    bIsRotate=false;
    iNozzleEvent=2;                                                             //Klutter 20210817 修改屬性方便修正吸嘴回home異常事件   //Steven 20210825 : 吹氣完成才可以歸零

    int pos=0, iSuckRow=0, iSuckCol=0;
//    AnsiString Data[8]={"OutArmSuckA", "OutArmSuckC", "OutArmSuckE", "OutArmSuckG", "OutArmSuckB", "OutArmSuckD", "OutArmSuckF", "OutArmSuckH"};
    pos=SuckerName.Pos("OutArmSuck");
    if(pos==1)                                                                  //kevin 20180119 (Steven) add out arm 放料中不能歸home
    {                                                                           //Steven 20240822 : 修正放料判斷
        iSuckRow=iMyRow;
        iSuckCol=iMyCol;
        bOutArmPlaceDevice[iSuckRow][iSuckCol]=true;
    }

    if(LastSet.iRealDummy==DUMMY ||
      (LastSet.iRealDummy==HAS_TRAY && SuckerName!="CatchSuck"))
    {
        switch(OffTask)
        {
            case 1:
                if(VacuumOffTime>1000)                                          // 2007_04_24 lee start
                    VacuumOffTime=50;
                TOff.SetMSAndOn(VacuumOffTime);
                OffTask=2;

                if(SuckerName.AnsiPos("InArmSuck")!=0 ||
                   SuckerName.AnsiPos("OutArmSuck")!=0)                         //Steven 20141016 : Dummy Run也要開真空破壞
                {
                    DoOnIO(false);                                              //關閉真空
                }

                break;
            case 2:
                if(TOff.Off())
                {
                    TOffDelay.SetMSAndOn(OffDelayTime*10);
                    if(CosFunction.RecordIndexAirOnTime)                        //Sam 20220329 : Record Index Air On Time
                    {
                        TOffDelay.LatchCycleTime(true);
                        iRecordOffDelayTime=0;
                        sRecordStartOffDelayTime.sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
                    }
                    OffTask=50;
                }
                break;
            case 50:
                if(TOffDelay.Off())
                {
                    if(CosFunction.RecordIndexAirOnTime)                        //Sam 20220329 : Record Index Air On Time
                    {
                        sRecordEndOffDelayTime.sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
                        iRecordOffDelayTime=TOffDelay.LatchCycleTime();
                    }

                    OffTask=1;
                    InitSuckFlag();
                    bDestroyOK=true;                                            //ChungHung 20110901 add
                    bOutArmPlaceDevice[iSuckRow][iSuckCol]=false;               //kevin 20180119 (Steven) add out arm 放料中不能歸home
                    if(iHome!=1)
                        iNozzleEvent=0;                                         //Klutter 20210817 修改屬性方便修正吸嘴回home異常事件   //Steven 20210825 : 吹氣完成才可以歸零
                    return true;
                }
                break;
        }
        return false;
    }

    if(OffTask!=300)
    {
        DoOnIO(false);                                                          //關閉真空
        DoOffIO(true);                                                          //打開破壞
    }

    if(OffTask==1 ||OffTask==2)
    {
        if(Enable)                                                              //  has install onsensor
        {
            if(OffAlarmTime==0)
            {
                InRet=W906_GetStatusAnyOn();  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個還有真空＝還沒放開
                if(InRet==false)                                                // ok
                {
                    OffTask=100;                                                // do on delay
                }
                else
                {
                    OffTask=1;
                    rct--;
                    bOutArmPlaceDevice[iSuckRow][iSuckCol]=false;               //kevin 20180119 (Steven) add out arm 放料中不能歸home
                    if(rct>0)
                        return false;
                    DoOffIO(false);                                             //打開破壞
                    Error=true;
                    iNozzleEvent=0;                                             //Klutter 20210817 修改屬性方便修正吸嘴回home異常事件     //Steven 20210825 : 吹氣完成才可以歸零
                    bDestroyOK=true;                                            //ChungHung 20110901 add
                    return false;
                }
            }
            else
            {
                tSuckTimer.LatchCycleTime(true);
                if(W906_GetStatusAnyOn()==false)                                          // already off  AI(W906-F03-QUADVAC) 20261004
                {
                    OffTask=100;
                }
                else
                {
                    TOff.SetMSAndOn(OffAlarmTime*10);
                    OffTask=50;
                }
            }
        }
        else
        {
            OffTask=100;                                                        //need delay
        }
    }

    if(OffTask==50)
    {
        InRet=W906_GetStatusAnyOn();  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個還有真空＝還沒放開
        if(InRet==false)                                                        // ok
        {
            OffTask=100;                                                        // do on delay
        }
        else
        {
//            if(bHandlerPause)                                                 //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
//            {
//                TOff.SetMSAndOn(OffAlarmTime*10);
//                return false;
//            }

            if(TOff.Off())
            {
                OffTask=1;
                rct--;
                bOutArmPlaceDevice[iSuckRow][iSuckCol]=false;                   //kevin 20180119 (Steven) add out arm 放料中不能歸home
                if(rct>0)
                    return false;
                Normal();                                                       //jou 2011-11-01 開破壞不可能同時開真空，所以OffDestroy -> Normal
                OffTask=200;
                TOff.SetMSAndOn(10);
                return false;
            }
            else
            {
                return false;
            }
        }
    }

    if(OffTask>=100)
    {
        switch(OffTask)
        {
            case 100:
                VacuumOffTimeRecord=tSuckTimer.LatchCycleTime();
                if(RealTimeRefreshVacuumOnOffTime)
                {
                    PushOffTime();
                }

                if(OffDelayTime==0)
                {
                    //OffTask=1;                                                //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
                    rct=RetryCT;
                    //Normal();                                                 //jou 2011-11-01 開破壞不可能同時開真空，所以OffDestroy -> Normal
                    OffSuck();
                    OffDestroy();
                    Error=false;

                    if(ract>=DestroyAgainCount)                                 //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
                    {
                        InitSuckFlag();
                        ract=1;
                        OffTask=1;
                        bDestroyOK=true;                                        //ChungHung 20110901 add
                        bOutArmPlaceDevice[iSuckRow][iSuckCol]=false;           //kevin 20180119 (Steven) add out arm 放料中不能歸home
                        if(iHome!=1)
                            iNozzleEvent=0;                                     //Klutter 20210817 修改屬性方便修正吸嘴回home異常事件   //Steven 20210825 : 吹氣完成才可以歸零
                        return true;
                    }
                    else
                    {
                        TAgainDestroyDelay.SetMSAndOn(DestroyAgainTime*10);
                        ract++;
                        OffTask=300;
                        return false;
                    }
                }
                else
                {
                    TOffDelay.SetMSAndOn(OffDelayTime*10);
                    if(CosFunction.RecordIndexAirOnTime)                        //Sam 20220329 : Record Index Air On Time
                    {
                        TOffDelay.LatchCycleTime(true);
                        iRecordOffDelayTime=0;
                        sRecordStartOffDelayTime.sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
                    }
                    OffTask=101;
                    break;                                                      //jou 2012-01-04 加上break,不然會一直卡在Case 101: TOffDelay.Off()
                }
            case 101:
                if(TOffDelay.Off())
                {
                    if(CosFunction.RecordIndexAirOnTime)                        //Sam 20220329 : Record Index Air On Time
                    {
                        sRecordEndOffDelayTime.sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
                        iRecordOffDelayTime=TOffDelay.LatchCycleTime();
                    }
                    rct=RetryCT;
                    //OffTask=1;                                                //ChungHung 20130413 add 針對 阿凡達IC 無法Relase delete
                    //Normal();                                                 //jou 2011-11-01 開破壞不可能同時開真空，所以OffDestroy -> Normal
                    OffSuck();
                    OffDestroy();
                    Error=false;

                    if(ract>=DestroyAgainCount)                                 //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
                    {
                        InitSuckFlag();
                        ract=1;
                        OffTask=1;
                        bDestroyOK=true;                                        //ChungHung 20110901 add
                        bOutArmPlaceDevice[iSuckRow][iSuckCol]=false;           //kevin 20180119 (Steven) add out arm 放料中不能歸home
                        if(iHome!=1)
                            iNozzleEvent=0;                                     //Klutter 20210817 修改屬性方便修正吸嘴回home異常事件   //Steven 20210825 : 吹氣完成才可以歸零
                        return true;
                    }
                    else
                    {
                        TAgainDestroyDelay.SetMSAndOn(DestroyAgainTime*10);
                        ract++;
                        OffTask=300;
                        return false;
                    }
                }
                return false;
            case 200:
                if(TOff.Off())
                {
                    Error=true;
                    iNozzleEvent=0;                                             //Klutter 20210817 修改屬性方便修正吸嘴回home異常事件   //Steven 20210825 : 吹氣完成才可以歸零
                    OffTask=1;
                    bDestroyOK=true;                                            //ChungHung 20110901 add
                    bOutArmPlaceDevice[iSuckRow][iSuckCol]=false;               //kevin 20180119 (Steven) add out arm 放料中不能歸home
                    return false;
                }
                break;
            case 300:
                if(TAgainDestroyDelay.Off())
                {
                    OffTask=1;
                }
                break;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMySucker::FinshFullMotion()
{
    if(OnTask!=1 || OffTask!=1)
        return false;
    return true;
}
//------------------------------------------------------------------------------
void TMySucker::ResetSuckTask()
{
    iNozzleEvent=0;                                                             //Klutter 20210817 方便修正吸嘴回home異常事件             //Steven 20210825 : 吹氣完成才可以歸零
    if(OnTask!=1)
        OnTask=2;
    if(OffTask!=1)
        OffTask=2;
}
//------------------------------------------------------------------------------
void TMyKitSuck::ClearAllError()
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            Suck[i][j].Error=false;
}
//------------------------------------------------------------------------------
void TMySucker::SetRetryCount(int ct)
{
    RetryCT=ct;
}
//------------------------------------------------------------------------------
void ClearAllManualSuckTask()
{
    for(int i=0; i<pSuck->Count; i++)
    {
        pTempSuck=(TMySucker*)pSuck->Items[i];
        if(pTempSuck!=NULL)
            pTempSuck->iManualOffTask=1;
    }
}
//------------------------------------------------------------------------------
void TMySucker::CheckIsFallDown()
{
    if(Enable==false || OffTask>2)
        return;
    if(GetStatus()==false)                                                      // 關閉真空
    {
        DoOnIO(false);                                                          //關閉真空
    }
}
//------------------------------------------------------------------------------
bool TMyKitSuck::CheckLedStatus()
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(bLed[i][j]==true)
            {
                if(Item[i][j]==NULL_IC && pLed[i][j]->Value==true)
                    return false;
                if(Item[i][j]!=NULL_IC && pLed[i][j]->Value==false)
                    return false;
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
void TMyKitSuck::ResetAll()
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            Suck[i][j].ResetSuckTask();
            bNeedCheck[i][j]=false;
        }
    }
    ClearAllError();
}
//------------------------------------------------------------------------------
// if sensor Off  ,off vacuum ,flag not chage
// if        On   ,if has not Real IC then off vacuum,flag=true
//                 else do not change
void TMyKitSuck::CheckVaccumIsIniaialON(int iRow, int iCol, bool &flag)
{
    if(Suck[iRow][iCol].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 殘料判斷＝群組任 1 個有真空（沒有群組＝GetStatus()）
    {
        if(Item[iRow][iCol]==NULL_IC || Item[iRow][iCol]==HAS_NULL_IC)
        {
            Suck[iRow][iCol].Normal();
            flag=true;
        }
    }
    else
    {
        Suck[iRow][iCol].Normal();
    }
}
//------------------------------------------------------------------------------
void TMyKitSuck::CheckVaccumIsON_AboveSocket(int i, int j, bool &flag)          //JerryYang 20250120 : add
{
    if(Suck[i][j].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 同上，任 1 個有真空
    {
        flag=true;
    }
    else
    {
        Suck[i][j].Normal();
    }
}
//------------------------------------------------------------------------------
void TMyKitSuck::SetAllRealIC2InterfaceBin()
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]!=NULL_IC && Item[i][j]!=HAS_NULL_IC)
            {
                SetItemData(i, j, TEST_PASS+iTestBinCount);                     //Steven 20150203 : Fixed for Sucker Status
                iBinData[i][j]=iTestBinCount;
                bPass[i][j]=false;
                bNeedReTest[i][j]=false;
            }
        }
    }
}
//------------------------------------------------------------------------------
void TMyKitSuck::SetAllHASIC2ErrorBin()
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_IC)
            {
                SetItemData(i, j, TEST_PASS+iTestBinCount);                     //Steven 20150203 : Fixed for Sucker Status
                iBinData[i][j]=iTestBinCount;
                bPass[i][j]=false;
                bNeedReTest[i][j]=false;
            }
        }
    }
}
//------------------------------------------------------------------------------
//jou 980312
void CopyKitSuck(TMyKitSuck *Source, TMyKitSuck *Target)
{
    for(int i=0; i<Source->iMaxRow; i++)                                        //2->Source->iMaxRow Eliot 2009_12_28
    {
        for(int j=0; j<Source->iMaxCol; j++)                                    //4->Source->iMaxCol Eliot 2009_12_28
        {
            Target->Suck[i][j].OnRing      =Source->Suck[i][j].OnRing;
            Target->Suck[i][j].OnIP        =Source->Suck[i][j].OnIP;
            Target->Suck[i][j].OnPort      =Source->Suck[i][j].OnPort;
            Target->Suck[i][j].OnBit       =Source->Suck[i][j].OnBit;
            Target->Suck[i][j].OnType      =Source->Suck[i][j].OnType;

            Target->Suck[i][j].OffRing     =Source->Suck[i][j].OffRing;
            Target->Suck[i][j].OffIP       =Source->Suck[i][j].OffIP;
            Target->Suck[i][j].OffPort     =Source->Suck[i][j].OffPort;
            Target->Suck[i][j].OffBit      =Source->Suck[i][j].OffBit;
            Target->Suck[i][j].OffType     =Source->Suck[i][j].OffType;

            Target->Suck[i][j].SenRing     =Source->Suck[i][j].SenRing;
            Target->Suck[i][j].SenIP       =Source->Suck[i][j].SenIP;
            Target->Suck[i][j].SenPort     =Source->Suck[i][j].SenPort;
            Target->Suck[i][j].SenBit      =Source->Suck[i][j].SenBit;
            Target->Suck[i][j].SenType     =Source->Suck[i][j].SenType;
            Target->Suck[i][j].iMotNo      =Source->Suck[i][j].iMotNo;          //Steven for HT1032
            Target->Suck[i][j].sName       =Source->Suck[i][j].sName;
            Target->Suck[i][j].iMyRow      =Source->Suck[i][j].iMyRow;          //JerryYang 20230930
            Target->Suck[i][j].iMyCol      =Source->Suck[i][j].iMyCol;
            Target->Suck[i][j].SensorName  =Source->Suck[i][j].SensorName;      //Steven 20240207 : Add
            Target->Suck[i][j].OnPortName  =Source->Suck[i][j].OnPortName;
            Target->Suck[i][j].OffPortName =Source->Suck[i][j].OffPortName;
            Target->Suck[i][j].SuckerName  =Source->Suck[i][j].SuckerName;
        }
    }
}
//------------------------------------------------------------------------------
void CopySuck(TMySucker *Source, TMySucker *Target)
{
    Target->OnRing      =Source->OnRing;
    Target->OnIP        =Source->OnIP;
    Target->OnPort      =Source->OnPort;
    Target->OnBit       =Source->OnBit;
    Target->OnType      =Source->OnType;

    Target->OffRing     =Source->OffRing;
    Target->OffIP       =Source->OffIP;
    Target->OffPort     =Source->OffPort;
    Target->OffBit      =Source->OffBit;
    Target->OffType     =Source->OffType;

    Target->SenRing     =Source->SenRing;
    Target->SenIP       =Source->SenIP;
    Target->SenPort     =Source->SenPort;
    Target->SenBit      =Source->SenBit;
    Target->SenType     =Source->SenType;
    Target->iMotNo      =Source->iMotNo;                                        //Steven for HT1032
    Target->sName       =Source->sName;
    Target->SensorName  =Source->SensorName;                                    //Steven 20240207 : Add
    Target->OnPortName  =Source->OnPortName;
    Target->OffPortName =Source->OffPortName;
    Target->SuckerName  =Source->SuckerName;
    Target->iMyRow      =Source->iMyRow;                                        //JerryYang 20230930
    Target->iMyCol      =Source->iMyCol;
}
//==============================================================================
// 函式說明: 檢查吸嘴事件已結束
//Klutter 20210817 檢查吸嘴事件已結束
//==============================================================================
bool TMyKitSuck::CheckDestoryFinish()                                           //Steven 20210825 : 吹氣完成才可以歸零
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Suck[i][j].iNozzleEvent==2)                                      //Destroy
            {
                if(Item[i][j]==NULL_IC || Item[i][j]==HAS_NULL_IC)
                    Suck[i][j].iNozzleEvent=0;
                return false;
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
double TMySucker::GetIOValue()                                                  //Sam 20230210 : 新增 VacuumUnit 通訊模組
{
    return MyLaneIO.GetIOValue(SenRing, SenIP, SenPort, SenBit, SenISABase, SensorName);
}
//------------------------------------------------------------------------------
double TMySucker::GetIOValueThread()                                            //Sam 20230210 : 新增 VacuumUnit 通訊模組
{
    return MyLaneIO.GetIOValueThread(SenRing, SenIP, SenPort, SenBit, SenISABase, SensorName);
}
//------------------------------------------------------------------------------
bool TMySucker::SetIOValueThread(double dKpa)                                   //Sam 20230210 : 新增 VacuumUnit 通訊模組
{
     return MyLaneIO.SetIOValueThread(dKpa, SenRing, SenIP, SenPort, SenBit, SenISABase, SensorName);
}
//------------------------------------------------------------------------------

// ===========================================================================
//  AI(W906-A4-6) 20260924: 以下兩組是移植樹獨有、從已退役的 aHotPlateSubstrate 精簡鏡像搬過來的（golden 沒有）。
// ===========================================================================
// 觀測接縫：每個吸嘴的 OffDestroy() 呼叫次數。epoch 由 W906_TMySucker_OffDestroy_ResetAll() 遞增，
// 蓋章落後的吸嘴讀成 0 —— 一次遞增就讓全樹的計數歸零，不需要逐一登記。
void TMySucker::W906_OffDestroySeamHit()
{
    if(W906_OffDestroyStamp != W906_TMySucker_OffDestroyEpoch)
    {
        W906_OffDestroyStamp = W906_TMySucker_OffDestroyEpoch;
        W906_OffDestroyRaw   = 0;
    }
    ++W906_OffDestroyRaw;
    ++W906_TMySucker_OffDestroyTotal;
}
int TMySucker::W906_GetOffDestroyCount() const
{
    if(W906_OffDestroyStamp != W906_TMySucker_OffDestroyEpoch)
        return 0;
    return W906_OffDestroyRaw;
}
void TMySucker::W906_ResetOffDestroyCount()
{
    W906_OffDestroyStamp = W906_TMySucker_OffDestroyEpoch;
    W906_OffDestroyRaw   = 0;
}

// SetType1ToType2ByPickCol：golden 沒有定義（只有 ainarm9045_2x4_16.cpp 的呼叫點，那支在 golden 是死碼），
// 本體照精簡鏡像原樣搬來（aHotPlateSubstrate.cpp:843，golden-by-name 的忠實重對應），讓該檔編得起來。
void TMyKitSuck::SetType1ToType2ByPickCol(int Type1, int Type2)
{
    for(int i=0; i<iPickRow; i++)
        for(int j=0; j<iPickCol; j++)
            if(Item[i][j]==Type1)
                SetItemData(i, j, Type2);
}

//==============================================================================
//  AI(W906-F03-QUADVAC) 20261004: F-03 ③ -- HT9050 picks ONE IC with the 4 nozzles of one ECAT-VC4 (In Arm, Out Arm,
//  Index).  NOT in golden (906 / 912 / 910 all have one nozzle = one IC; NIGHT_REPORT §0 F-03 "910 也沒有程式").
//  Analysis: NB2-1 R226 (docs/nb2_assist, the flow points) and R227 (the same rules, already in production on another
//  machine); the laptop's design CHAT_JIMMY 1004 22:1x; Jimmy's rulings RULINGS_20261004 #2 (In / Out Arm are the same
//  mode) and #3 (Index is 4-in-1 too; teach = nozzle A; the reversed Out Arm wiring is the hardware design).
//
//  THE SHAPE: one IC stays one data cell (Suck[0][0] = the "master"); the master carries its 3 companions.
//    * outputs: every make / break valve call on the master (DoOnIO / DoOffIO -- the golden single choke point, so
//      On / Off / OnSuck / OffSuck / OnDestroy / OffDestroy / Normal / Suck / Destroy all follow) drives all 4 at once;
//      a companion driven on its own (a fixed 2x4 loop, a multi-picker recipe) sends nothing.
//    * reads: "vacuum made" = all 4 on (Suck(), W906_GetStatusAllOn); "released / something there" = any 1 on
//      (Destroy(), W906_GetStatusAnyOn); "still holding while carrying" = all 4 on, a loss counts only after it has lasted
//      kW906HoldLostMs (W906_GetStatusHeld -- one noisy read is not a drop).  A companion's own GetStatus() reads false
//      while grouped (the flows' per-cell residue / D81 / reset loops would otherwise see the real IC under C / E / G);
//      its sensor is read only through the master.  Rows with Enable=0 are not counted (3 of 4 then).
//    * alarms and counters: the flows only ever name / count the master -> one alarm, one JAM, per IC.
//    * switches: Gerneral.ini [System] IN_ARM_VC4_GROUP / OUT_ARM_VC4_GROUP / INDEX_VC4_GROUP, 1 = grouped, default 0,
//      read once at boot after InitSucker (restart to change), read only (never seeded into the machine's file).  Not
//      the machine type: Type_HT9050 is dormant (9050GPIB decodes to Type_HT9046_LS, database.cpp:517).
//    * groups by IO-table NAME, so the wiring order does not matter (the Out Arm's A..G run VC3..VC0): In Arm
//      InArmSuckA/C/E/G, Out Arm OutArmSuckA/C/E/G, Index FTestSuckAA/BA/AB/BB.  Suck[0][0] must be the first name
//      (A / AA), otherwise the station is NOT grouped and the reason is printed (e.g. a recipe that remaps the single
//      picker -- bSingleUseOtherSuck -- makes no sense on a 4-nozzle head).
//    * no group (every switch 0, the default, and every machine but this one) = golden, line for line: every new read
//      falls back to GetStatus() and W906_FanOut returns false at its third line.
//  Diagnostics while grouped: the IO page's per-sucker lamps for C / E / G read off; the Vacuum Unit page reads the 4
//  VC channels directly.
//==============================================================================
#include "vclcompat/IniFiles.h"     // TIniFile -- read-only use below (a read never writes the file, IniFiles.h:29-31)
extern AnsiString asGeneralPath;    // common.h:60

static const unsigned long kW906HoldLostMs = 100;   // the carrying-drop debounce (R227: a loss must persist 100 ms)

// GetStatus() minus the group-member line: a companion's own sensor, read by its master.  Keep in step with GetStatus().
bool TMySucker::W906_RawSensorOn()
{
    bool InRet=false;
    if(Enable==false)
        return false;
    if(SenISABase==eMotionNet ||
       SenISABase==ePCI1203)
    {
        InRet=MyLaneIO.IOInputBit(SenRing, SenIP, SenPort, SenBit, SenISABase, SensorName);
    }
    else if(SenISABase==eISABase ||
            SenISABase==ePCI1735U ||
            SenISABase==ePLCbase)
    {
        InRet=IOInputBit(SenPort, SenBit);
    }
    if(SenType==TYPE_A && InRet==1)
        return true;
    else if(SenType==TYPE_B && InRet==0)
        return true;
    return false;
}
//------------------------------------------------------------------------------
// "vacuum made": every counted member on.  No group = GetStatus().
bool TMySucker::W906_GetStatusAllOn()
{
    if(W906_iGroupCount<=0)
        return GetStatus();
    if(Enable==false)
        return false;
    int n=0;
    for(int g=0; g<W906_iGroupCount && g<4; g++)
    {
        TMySucker* m=W906_pGroup[g];
        if(m==0 || m->Enable==false)
            continue;
        ++n;
        if(m->W906_RawSensorOn()==false)
            return false;
    }
    return n>0;
}
//------------------------------------------------------------------------------
// "something is still there": any counted member on.  No group = GetStatus().
bool TMySucker::W906_GetStatusAnyOn()
{
    if(W906_iGroupCount<=0)
        return GetStatus();
    if(Enable==false)
        return false;
    for(int g=0; g<W906_iGroupCount && g<4; g++)
    {
        TMySucker* m=W906_pGroup[g];
        if(m==0 || m->Enable==false)
            continue;
        if(m->W906_RawSensorOn())
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
// "still holding while carrying": all on; a loss reads false only once it has lasted kW906HoldLostMs.  No group =
// GetStatus() (no debounce -- golden).  The window restarts whenever all members are seen on, and whenever the master
// opens its vacuum (W906_FanOut), so a stale start from an earlier IC cannot shorten it.
bool TMySucker::W906_GetStatusHeld()
{
    if(W906_iGroupCount<=0 || Enable==false)
        return GetStatus();
    if(W906_GetStatusAllOn())
    {
        W906_dwHoldLostStart=0;
        return true;
    }
    const unsigned long now=::GetTickCount();
    if(W906_dwHoldLostStart==0)
    {
        W906_dwHoldLostStart=(now==0) ? 1 : now;
        return true;
    }
    return (now-W906_dwHoldLostStart) < kW906HoldLostMs;
}
//------------------------------------------------------------------------------
// The first line of DoOnIO / DoOffIO.  true = handled here (the caller returns), false = run the golden body.
bool TMySucker::W906_FanOut(bool bOnValve, bool bOn)
{
    if(W906_bInFan)
        return false;                       // the master's own valve, or a member driven by its master
    if(W906_pGroupMaster!=0)
        return true;                        // a grouped companion driven on its own: nothing is sent
    if(W906_iGroupCount<=0)
        return false;                       // not grouped: golden
    if(bOnValve && bOn)
        W906_dwHoldLostStart=0;             // a new hold starts
    for(int g=0; g<W906_iGroupCount && g<4; g++)
    {
        TMySucker* m=W906_pGroup[g];
        if(m==0)
            continue;
        m->W906_bInFan=true;
        if(bOnValve)
            m->DoOnIO(bOn);
        else
            m->DoOffIO(bOn);
        m->W906_bInFan=false;
    }
    return true;
}
//------------------------------------------------------------------------------
static void W906_UnwireVc4Group(TMyKitSuck& kit)
{
    for(int i=0; i<_MAX_SUCK_ROW_ITEM; i++)
        for(int j=0; j<_MAX_SUCK_COL_ITEM; j++)
        {
            TMySucker& s=kit.Suck[i][j];
            s.W906_iGroupCount=0;
            s.W906_pGroupMaster=0;
            s.W906_bInFan=false;
            s.W906_dwHoldLostStart=0;
            for(int g=0; g<4; g++)
                s.W906_pGroup[g]=0;
        }
}
//------------------------------------------------------------------------------
static TMySucker* W906_FindSuckerByName(TMyKitSuck& kit, const char* name)
{
    const AnsiString want(name);
    for(int i=0; i<_MAX_SUCK_ROW_ITEM; i++)
        for(int j=0; j<_MAX_SUCK_COL_ITEM; j++)
            if(kit.Suck[i][j].SuckerName==want)
                return &kit.Suck[i][j];
    return 0;
}
//------------------------------------------------------------------------------
// Wires kit.Suck[0][0] as the master of the 4 named suckers.  Returns 4, or 0 (not grouped; the reason is printed).
int W906_WireVc4Group(TMyKitSuck& kit, const char* const names[4], const char* station)
{
    W906_UnwireVc4Group(kit);
    TMySucker* mem[4];
    for(int g=0; g<4; g++)
    {
        mem[g]=W906_FindSuckerByName(kit, names[g]);
        if(mem[g]==0)
        {
            printf("[vc4group] %s: NOT grouped -- no sucker named %s\n", station, names[g]);
            return 0;
        }
    }
    TMySucker* master=&kit.Suck[0][0];
    if(mem[0]!=master)
    {
        printf("[vc4group] %s: NOT grouped -- Suck[0][0] is %s, not %s\n", station, master->SuckerName.c_str(), names[0]);
        return 0;
    }
    for(int g=0; g<4; g++)
    {
        master->W906_pGroup[g]=mem[g];
        if(g>0)
            mem[g]->W906_pGroupMaster=master;
    }
    master->W906_iGroupCount=4;
    printf("[vc4group] %s: %s + %s / %s / %s move and read as one nozzle\n", station, names[0], names[1], names[2], names[3]);
    return 4;
}
//------------------------------------------------------------------------------
// Called once at boot right after InitSucker() (cinitial.cpp InitHontechHardware).
void W906_InitVc4Groups()
{
    static const char* const inNames[4]  = { "InArmSuckA",  "InArmSuckC",  "InArmSuckE",  "InArmSuckG"  };
    static const char* const outNames[4] = { "OutArmSuckA", "OutArmSuckC", "OutArmSuckE", "OutArmSuckG" };
    static const char* const idxNames[4] = { "FTestSuckAA", "FTestSuckBA", "FTestSuckAB", "FTestSuckBB" };
    int swIn=0, swOut=0, swIdx=0;
    {
        TIniFile ini(asGeneralPath);
        swIn =ini.ReadInteger("System", "IN_ARM_VC4_GROUP",  0);
        swOut=ini.ReadInteger("System", "OUT_ARM_VC4_GROUP", 0);
        swIdx=ini.ReadInteger("System", "INDEX_VC4_GROUP",   0);
    }
    W906_UnwireVc4Group(InArmSuck);
    W906_UnwireVc4Group(OutArmSuck);
    W906_UnwireVc4Group(FTestSuck);
    if(swIn==1)
        W906_WireVc4Group(InArmSuck, inNames, "In Arm");
    if(swOut==1)
        W906_WireVc4Group(OutArmSuck, outNames, "Out Arm");
    if(swIdx==1)
        W906_WireVc4Group(FTestSuck, idxNames, "Index");
}
