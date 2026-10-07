//AI(W0-TAIL) 20260626: de-VCL cprod.cpp. Dropped MachineDefine.h (VCL+drivers) +
//  the app headers (mymotor/common/csystem/main/cmydef/database/uLotInfo/...).
//  ACTIVE: the global var definitions below (Prod/IniConfig/CosFunction/Offset/
//  ARM_OFFSET* arrays/dtStartLot/dtEndLot/tP62MachineStopTimer) -- the W0 contract
//  globals. All function bodies are gated TODO(W6) (they reach untranslated app code).
#include "cprod.h"
#include "vclcompat/vcl_compat.h"   //AI(W0-TAIL) 20260626: explicit (also via cprod.h->MachineType.h)

//HANDLER_SETUP THS;
PROD_INFO_ST     Prod;
SYSTEM_DEVICE_FORM DeviceForm;
SYSTEM_DEVICE_FORM DeviceForm_File;
SYSTEM_DEVICE_FORM DeviceForm_NET;                                              //Ifor 20181023 add SCC要求FTP資料卡控加入Contact相關資料
SYSTEM_TRAY_FORM TrayForm;
TRAY_TYPE_PARA HotPlateForm;
TRAY_TYPE_PARA HotPlateForm_File;
TRAY_TYPE_PARA UserDefForm[4];
TRAY_TYPE_PARA UserDefForm_File[4];

TRAY_TYPE_PARA *LoadForm;
TRAY_TYPE_PARA *AutoForm[eTrayCount];
//TRAY_TYPE_PARA *FixForm[eTrayCount];
TRAY_TYPE_PARA *EmptyForm ;
TRAY_TYPE_PARA *ColorForm ;
//TRAY_TYPE_PARA *OutputForm[eTrayCount];

SYSTEM_TEMPERATURE Temperature;
LD_ULDTIME Ld_UldDelayTime;

SYSTEM_TEST_IF TestIF;
SYSTEM_TEST_IF TestIF_File;
SYSTEM_TEST_MODE TestMode;                                                      //Steven 20111019

SYSTEM_BIN_SELECT BinSelect[8];                                                 //ChungHung 20141002 add for KYEC AutoRetest 3->5 //ChungHung 20111110 //Ifor 20170316 (wei) add MRT Mode 5->8

// 2013.12.03 , Joye , KYEC FTP --------------->>                               //20140103 wei
SYSTEM_TRAY_FORM    TrayForm_NET;
TRAY_TYPE_PARA      HotPlateForm_NET;
SYSTEM_TEMPERATURE  Temperature_NET;
SYSTEM_TEST_IF      TestIF_NET;
SYSTEM_BIN_SELECT   BinSelect_NET[8];                                           //ChungHung 20141002 add for KYEC AutoRetest 3->5    //Ifor 20170316 (wei) add MRT Mode 5->8
// 2013.12.03 , Joye , KYEC FTP ---------------<<

TTL_DATA TTLCfg;
//CCD_DATA CCDBarCode;

//DATA Data;
PASS_WORD USER;
//LAST_GENERAL_COUNT LastCount;   //Steven 20110801
HT9045_CONFIG IniConfig;
TAutoTeachOffset AutoTeachOffset;
HT9045_COUSTOMER_FUNCTION CosFunction;
LAST_LEVEL_SET LevelSet;

INPUT_LIMIT InputLimit;                                                         //Steven 20090805
DUMMY_VACUUM DummyVacuum;                                                       //Jou 980805
//TMotionnetIO tMotionnetIO;
TAOISetup tAOISetup;                                                            //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
TATCData tATCData;                                                              //2014-05-30    Dell    for ATC6.0
//TRAY_DATA TrayDef;
//TECH TempTech;
RUN_OFFSET Offset;
RUN_OFFSET Offset_File;
RUN_INFO RunInfo;
INVISIBLE_OFFSET InvisibleOffset;
//RUN_INFO RunInfo2;
class ARM_OFFSET *InArmOffSet[InOfsTotal];                                      //Steven 20140425 : 重整Offset
class ARM_OFFSET *OutArmOffSet[OutOfsTotal];
class ARM_OFFSET *InArmOffSet_File[InOfsTotal];                                 //Steven 20140425 : 重整Offset
class ARM_OFFSET *OutArmOffSet_File[OutOfsTotal];
class ARM_OFFSET *SortArmOffSet[SortOfsTotal];                                  //RogerYang 20250417 for HT9046AU add
class ARM_OFFSET *SortArmOffSet_File[SortOfsTotal];                             //RogerYang 20250417 for HT9046AU add
ARM_CONDITION ArmSpeed[SpeedPartTotal];
ARM_CONDITION ArmSpeed_File[SpeedPartTotal];
ARM_CONDITION AutoArmSpeed[SpeedPartTotal];
SHUTTLE_SPEED SHSpeed;
SHUTTLE_SPEED SHSpeed_File;
MAGAZINE_SPEED MGSpeed;                                                         //JerryYang 20220909 : add magazine
MAGAZINE_SPEED MGSpeed_File;

Teach_Pos Teach;                                                                //kevin 20190305 teach pos .ini

const int RT=0;
const int FT=1;
const int OffT=2;                                                               //ChungHung 20111110 add
const int RT_ART=3;                                                             //ChungHung 20141002 add for KYEC AutoRetest
const int FT_ART=4;                                                             //ChungHung 20141002 add for KYEC AutoRetest
const int RT_MRT=5;                                                             //Ifor 20170316 (wei) add MRT Mode
const int FT_MRT=6;                                                             //Ifor 20170316 (wei) add MRT Mode
//const int EQC=7;

const int OffLine=0;
const int OnLine=1;

bool bInstallRotate=false;
const int IFaceErr=555;
RESERVE_EMPTY_POINT ReserverEmptyPoint[1000];                                   //ChungHung 20111215 嘗試將Fix3放滿
RESERVE_EMPTY_POINT ReserverEmptyPointAutoClean[20];
SHUTTLE_THREAD SThreadPara;                                                     //Steven 20110407 : Shuttle Thread的變數

const bool bReadFile=true;
const bool bWriteFile=false;
SYSTEM_SCANNER_AOI_IF ScannerAOIIF;                                             // 2012.12.10 , Joye , AMD HT-7046M
const AnsiString asFileNameConfigByRecipe=AnsiString("configByRecipe.ini");     //JimmyChiu 20220601 : config儲存跟隨recipe

AnsiString asNoRTBinFix[3]={"", "", ""};                                        //RogerYang 20250604 偉測不可複測bin功能

//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//==>
AUTOTEACH_POINT InputAtuoTeachTable[TotalInArmAOAType];                         //KenHsieh 20211214 : AOA add AutoClean(5->6)
AUTOTEACH_POINT OutputAtuoTeachTable[TotalOutArmAOAType];
AUTOTEACH_POINT InputAtuoTeachTableCal[TotalInArmAOAType];                      //KenHsieh 20211214 : AOA add AutoClean(5->6)
AUTOTEACH_POINT OutputAtuoTeachTableCal[TotalOutArmAOAType];
//<==
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
MyIndexArmRec myIAR_Test;                                                       //Jimmychiu 20240916 : Check the temperature during index arm testing
TDateTime dtStartLot=0.0;
TDateTime dtEndLot=0.0;
bool bNeedManualCheckEmptyTray=false;                                           //Jimmychiu 20250826 : Open Door Check Loader After TrayEnd
bool bNeedOneByOnePickInArm=false;                                              //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
//AI(ht9045-v906) 20260809: PT-W5c integrate -- bDoRTCLearning is declared extern in
// cprod.h:1360 and read at uHeaterThread.cpp:603, but its golden home is cContact.cpp:97,
// which is untranslated, so the port carried an undefined reference that only surfaced
// when csystem.cpp entering the link pulled uHeaterThread.cpp.obj out of the archive.
// Golden writes `bool bDoRTCLearning;` at file scope -- zero-initialised, no ctor -- so
// defining it here beside its cprod.h declaration is byte-for-byte golden behaviour, and
// is more faithful than gating the reader. WHOEVER TRANSLATES cContact.cpp: golden's
// definition is at cContact.cpp:97; delete this line then, or it is a duplicate.
bool bDoRTCLearning=false;                                                      //Ifor 20260226 add: Contact Mode 執行RTC Learn 不開啟Hot Air
TQPF_Timer tP62MachineStopTimer;                                                //Jimmychiu 20251205 : First Tray Check On Unloader
// =============================================================================
//  AI(W4) 20260626: Ungated RUN_INFO ctor/dtor + InitialData.
//
//  RUN_INFO RunInfo is a static global (defined above).  Its ctor fires at
//  static-init time and requires RUN_INFO::RUN_INFO(), ~RUN_INFO(), and
//  InitialData() to be linked.  All three are self-contained (TStringList,
//  AnsiString, std::vector, enum constants -- all already available).
//  Without them test_motor_w4 link fails with "undefined reference to ctor".
// =============================================================================
#include "cmydef.h"     // AI(W4) 20260626: tcTotalCount, SystemDate etc. used in InitialData
#include "Public/MyStringList.h"   // AI(W906-W150) 20261007 (St02-E): TMyStringList (Path / FileName) for the lifted slTestLog lines :3071-:3072; occupies the old blank line
// =============================================================================
//  AI(GA1-B2) 20260804: ungate pass -- headers now real/linkable in this tree that
//  were not when W0-TAIL gated this whole file.  See _ga1_b2_report.md for the
//  full function-by-function reconciliation (ungated / narrow-gated / whole-
//  function-gated, with exact blocking symbols for everything still gated).
// =============================================================================
#include "common.h"             // AuthPath/DataPath/OffsetPath/DefaultPath; ReadWriteIni/ReadIniData/
                                 // CheckAndReadIniData(+General)/WriteIniData(+NoLog/General);
                                 // GetRecipeFileName/GetRecipePath; MyForceDirectories/MySleep; WriteDataToFile
#include "database.h"           // HSys (SYSTEM_MODULAR) -- .MyGem is HTGem* (fwd-decl only, see narrow gates)
#include "csystem.h"            // HasICUnderMachine/HasAnyICInMachine
#include "mycylin.h"            // Cylinder[]
#include "mysensor.h"           // Sen[]
#include "Motor/mymotor.h"      // MOT[]
#include "Automation/AMR.h"     // AMR (TTeraPowerAMR)
#include "aHotPlateSubstrate.h" // InArmSuck/FTestSuck/TestSocket (TMyKitSuck) + MyDBIProcess(2-arg)
#include "canary_support.h"     // LastSet (LAST_GENERAL_SET)/ShowErrorMessage/RecordProcess
#include "acatchtray_shims.h"   // NewRecordProcess
#include "forms/fMain.h"        // fMain
#include "forms/fLotInfo.h"     // fLotInfo
#include "forms/fMesSystem.h"   //AI(W906-POOL2) 20261008 (Ifor01): fMesSystem for the VTEST jam-rate file name (golden cprod.cpp:1182)
#include "SECSGEM/uHGemClass.h" //AI(W906-POOL2) 20261008 (Ifor01): complete HTGem for HSys.MyGem->UpdateDataPath (golden cprod.cpp:2165 / :2364 / :3001)
#include "forms/fAGV.h"         // fAGV
#include "forms/fTrayForm.h"    // fTrayForm
//Steven 20260924 (S12-C, 使用者：「C HTEditList、D 二進位這兩個的讀寫檔先完成吧」)：本檔不能 include Public/HTEditList.h（與 aHotPlateSubstrate.h 的 TList／uPlateInfo 重複定義），
//   所以 elConfig／cbLastSet／elConfig_byRecipe 的三個方法改經 FileRW/_EditList.cpp 的轉接函式呼叫；
//   golden 的呼叫順序與參數不變，只是 `el->方法(…)` 寫成 `HTEditList_方法(el, …)`。
class HTEditList;
extern HTEditList *elConfig;
extern HTEditList *cbLastSet;
extern HTEditList *elConfig_byRecipe;
bool HTEditList_ReadEditTextFromFile(HTEditList *el, AnsiString Path, AnsiString FileName);   void (*W906_SimNetHook)(int iPhase) = 0;   //AI(W906-W58) 20260930 (St02-E): SIM-only network-key mask (SimNet/SimNetMask.cpp, installed by wb_serve); NULL in SHIP and in every other program -- phases 0 / 1 / 2 at :3217 / :3263 / :3323
bool HTEditList_SaveEditTextToFile(HTEditList *el, AnsiString Path, AnsiString FileName);
bool FileRW_ProxyChecked(const char* form, const char* name);
void FileRW_IniConfig_ChangeCBListProperty();   //Steven 20260924 (S12-C)：FileRW/IniConfig.cpp（golden fConfiguration->ChangeCBListProperty）   //Steven 20260924 (S12-C 寫方向)：FileRW/_EditList.cpp（TCheckBox 替身的 Checked）
void HTEditList_InitialDataToEdit(HTEditList *el);
#include "atester_shims.h"      // IsNNMode/ADAM_Rang/COM2
#include "Automation/SCK_ART_Remainder.h"  // TastCategory / fConfiguration
#include "Interface/InterfaceSYS.h"        // EL_UPDATE_PARAMETER/SendCommand_EventLog
#include <algorithm>            // std::sort
#include <io.h>                 // open/close/access -- bare MinGW names, no wrapper needed
                                 // (same verified convention as SECSGEM/uHGemEquipment.cpp:47-52)
#include <fcntl.h>              // O_RDONLY/O_RDWR
#include "LogObjects.h"   // AI(W906-W150) 20261007 (St02-E): W906_*LogObj() -- the golden TfMain log objects without forms/fMain.h (W-150 slice 1, skill hpi-mnetlog-split s7); occupies the old blank line
// ---------------------------------------------------------------------------
//  AI(GA1-B2) 20260804: local SysUtils-synonym shims, same established
//  per-TU convention already used by cpublic.cpp/Automation/SCK_ART_Remainder.cpp/
//  Interface/TesterTCP.cpp/SECSGEM/uHGemEquipment.cpp/SECSGEM/uHGemClass.cpp
//  (grep confirms all five carry an identical local copy -- centralizing was not
//  this task's call to make, so this file gets its own copy too).
// ---------------------------------------------------------------------------
// IncludeTrailingPathDelimiter -- BCB6 synonym for IncludeTrailingBackslash.
static inline AnsiString IncludeTrailingPathDelimiter(const AnsiString& p)
{
    return IncludeTrailingBackslash(p);
}

// SystemTimeToDateTime -- golden Delphi SysUtils function (SYSTEMTIME -> TDateTime),
// not yet in vclcompat. Built from the two real primitives vclcompat/TDateTime.h
// already provides (EncodeDate/EncodeTime); TDateTime's value_ is whole-day-count +
// day-fraction, so date+time addition is the correct composition (same idiom
// vclcompat's own TDateTime::Now() uses internally, TDateTime.cpp:72-75).
static inline TDateTime SystemTimeToDateTime(const SYSTEMTIME &st)
{
    return EncodeDate((Word)st.wYear, (Word)st.wMonth, (Word)st.wDay) +
           EncodeTime((Word)st.wHour, (Word)st.wMinute, (Word)st.wSecond, (Word)st.wMilliseconds);
}

RUN_INFO::RUN_INFO()
{
    slEventLogFile = new TStringList();
    slExe          = new TStringList();
    InitialData();
}

RUN_INFO::~RUN_INFO()
{
    slEventLogFile->Clear();
    slExe->Clear();
    delete slEventLogFile;
    delete slExe;
}

void RUN_INFO::InitialData()
{
    iUPH         = 0;
    iAvgUPH      = "0";
    iUnloadCount = 0;
    for (int i = 0; i < eTrayCount; i++)
        sT6AutoYield[i] = "";
    SystemTime      = "";
    MTBA            = "";
    MUBA            = "";
    SoftwareDate    = "";
    Factory         = "";
    MachineDefine   = "";
    SoftwareVersion = "";
    for (int i = 0; i < tcTotalCount; i++)
        ShowTempComp[i] = "";
    LotStartTime   = "2020-01-01 00:00:00";
    LotStartYear   = 2020;   // Sam 20240426: BarCoder Inspection Report
    LotStartMonth  = 1;
    LotStartDate   = 1;
    LotStartHour   = 0;
    LotStartMin    = 0;
    LotStartSec    = 0;
    LotEndTime     = "";
    LotNo          = "";
    SECSGEMVersion = "";
    slEventLogFile->Clear();
    vByLotJam.clear();
    JamRateFileName = "";
}

//------------------------------------------------------------------------------
//AI(GA1-B2) 20260804: ==== W0-TAIL's single big gate retired; see report ====
//  Ungated everything below whose golden dependencies now have a real declared+
//  defined home in this tree (LastSet/common.h/database.h/csystem.h/mycylin.h/
//  mysensor.h/Motor/mymotor.h/Automation/AMR.h/aHotPlateSubstrate.h/
//  canary_support.h/acatchtray_shims.h/forms facades/atester_shims.h/
//  Automation/SCK_ART_Remainder.h/Interface/InterfaceSYS.h -- all #included
//  above). What is STILL gated is narrow (a specific block or, in two cases,
//  a whole function) and carries a precise 'TODO(GA1-B2): blocked by <symbol>@
//  <location>' comment at the gate site -- see _ga1_b2_report.md for the full
//  function-by-function table.  The '_fastcall' (BCB single-underscore typo)
//  ARM_OFFSET ctor/dtor below is fixed to plain 'ARM_OFFSET::' here, matching
//  cprod.h:178-179's own note that this was always the intended faithful form.
ARM_OFFSET::ARM_OFFSET()                                                        //Steven 20140510 Start: Secs Gem
{
    SingleOffSet= new ARM_SINGLE_PARAM();

    tArmOffset=new TStringList();
    tArmPickOffset=new TStringList();
    tArmPlaceOffset=new TStringList();

    for(int i=ofsArmX; i<ofsArmTotal; i++)
        tArmOffset->Add("0");

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            tArmPickOffset->Add("0");
            tArmPlaceOffset->Add("0");
        }
    }
}
//------------------------------------------------------------------------------
ARM_OFFSET::~ARM_OFFSET()                                                       //AI(GA1-B2) 20260804: _fastcall typo fixed, matches cprod.h:179
{
    try
    {
        tArmOffset->Clear();                                                    //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        tArmPickOffset->Clear();                                                //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        tArmPlaceOffset->Clear();                                               //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete SingleOffSet;
        delete tArmOffset;
        delete tArmPickOffset;
        delete tArmPlaceOffset;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~ARM_OFFSET");
    }
}
//------------------------------------------------------------------------------
void ARM_OFFSET::SetPickUp(int iX, int iY, double dData)                        //iZ==>which Z only
{
    if(iX<InArmSuck.iMotRow && iY<InArmSuck.iMotCol)
    {
        SingleOffSet->dPickUpOffSet[iX][iY]=dData;
        tArmPickOffset->Strings[iX*MAX_ARM_Row+iY]=AnsiString(dData);           //A, B, C, ..., H   //Steven 20140510 : Secs Gem
    }
}
//------------------------------------------------------------------------------
void ARM_OFFSET::SetPlace(int iX, int iY, double dData)                         //iZ==>which Z only
{
    if(iX<InArmSuck.iMotRow && iY<InArmSuck.iMotCol)
    {
        SingleOffSet->dPlaceOffSet[iX][iY]=dData;
        tArmPlaceOffset->Strings[iX*MAX_ARM_Row+iY]=AnsiString(dData);          //A, B, C, ..., H   //Steven 20140510 : Secs Gem
    }
}
//------------------------------------------------------------------------------
void ARM_OFFSET::SetOneByOne(bool flag)
{
    bOneByOne=flag;
}
//------------------------------------------------------------------------------
void ARM_OFFSET::SetX(double Pos)
{
    dArmX=Pos;
    tArmOffset->Strings[ofsArmX]=AnsiString(Pos);                               //Steven 20140510 : Secs Gem
}
//------------------------------------------------------------------------------
void ARM_OFFSET::SetY(double Pos)
{
    dArmY=Pos;
    tArmOffset->Strings[ofsArmY]=AnsiString(Pos);                               //Steven 20140510 : Secs Gem
}
//------------------------------------------------------------------------------
void ARM_OFFSET::SetVariable(double Pos)
{
    dArmVariable=Pos;
    dXPitch[0]=Pos;
    tArmOffset->Strings[ofsArmPitch1]=AnsiString(Pos);                          //Steven 20140510 : Secs Gem
}
//------------------------------------------------------------------------------
void ARM_OFFSET::SetVariableY(double Pos)
{
    dArmVariableY=Pos;
    tArmOffset->Strings[ofsArmPitchY]=AnsiString(Pos);                          //Steven 20140510 : Secs Gem
}                                                                               //ChungHung 20131231 alter AutoYPitch
//------------------------------------------------------------------------------
void ARM_OFFSET::SetVariable2(double Pos)
{
    dArmVariable2=Pos;
    dXPitch[1]=Pos;
    tArmOffset->Strings[ofsArmPitch2]=AnsiString(Pos);                          //Steven 20140510 : Secs Gem
}                                                                               //Steven 20131002 : XY變距
//------------------------------------------------------------------------------
void ARM_OFFSET::SetVariable3(double Pos)
{
    dArmVariable3=Pos;
    dXPitch[2]=Pos;
    tArmOffset->Strings[ofsArmPitch3]=AnsiString(Pos);                          //Steven 20140510 : Secs Gem
}                                                                               //Steven 20131002 : XY變距
//------------------------------------------------------------------------------
void ARM_OFFSET::SetVariable4(double Pos)
{
    dArmVariable4=Pos;
    dXPitch[3]=Pos;
    tArmOffset->Strings[ofsArmPitch4]=AnsiString(Pos);                          //Steven 20140510 : Secs Gem
}                                                                               //Steven 20131002 : XY變距
//------------------------------------------------------------------------------
void ARM_OFFSET::SetPickUp(double Pos)
{
    dPickUp=Pos;
    tArmOffset->Strings[ofsArmPick]=AnsiString(Pos);                            //Steven 20140510 : Secs Gem
}
//------------------------------------------------------------------------------
void ARM_OFFSET::SetPlace(double Pos)
{
    dPlaceUp=Pos;
    tArmOffset->Strings[ofsArmPlace]=AnsiString(Pos);                           //Steven 20140510 : Secs Gem
}
//------------------------------------------------------------------------------
int GetSiteCount(bool IncludeCloseSite)
{
    int iSiteCount=0;

    if(IncludeCloseSite==true)
    {
        if(TestIF_File.iTestMode==SingleSite)                                   //1x1
        {
            iSiteCount=1;
        }
        else if(TestIF_File.iTestMode==DualSite)                                //1x2
        {
            iSiteCount=2;
        }
        else if(TestIF_File.iTestMode==TriSite1X3)                              //Frank 20160329 add for 1x3_4
        {
            iSiteCount=3;
        }
        else if(TestIF_File.iTestMode==QualSite1X4 ||
                TestIF_File.iTestMode==_8Site1X4)                               //ChungHung 20150528 add for 海思 _8Site1x4 //1x4
        {
            iSiteCount=4;
        }
        else if(TestIF_File.iTestMode==QualSite2X2 ||                           //2x2
                TestIF_File.iTestMode==QualSite2X2N)
        {
            iSiteCount=4;
        }
        else if(TestIF_File.iTestMode==DualSite2x1)                             //2x1
        {
            iSiteCount=2;
        }
        else if(TestIF_File.iTestMode==_6Site2X3 ||                             //ChungHung 20140115 add for 2x3_6
                TestIF_File.iTestMode==_6Site2X3N)
        {
            iSiteCount=6;
        }
        else if(TestIF_File.iTestMode==_8Site2X4 ||                             //2x4
                TestIF_File.iTestMode==_8Site2X4N)
        {
            iSiteCount=8;
        }
        else if(TestIF_File.iTestMode==_10Site2X5)                              //2x5   //wei 20190614 10 site
        {
            iSiteCount=10;
        }
        else if(TestIF_File.iTestMode==_12Site2X6)                              //2x6
        {
            iSiteCount=12;
        }
        else if(TestIF_File.iTestMode==_16Site2X8 ||
                TestIF_File.iTestMode==_16Site4X4)                              //Sam 20190226 : 16Site4X4 //2x8  //Eliot 2009_12_25
        {
            iSiteCount=16;
        }
        else if(TestIF_File.iTestMode==_32Site4X8M ||
                TestIF_File.iTestMode==_32Site4X8N)                             //4x8   //ChungHung 20130627 alter TestIF--->TestIF_File 修正無法跑32Site
        {
            iSiteCount=32;
        }
    }
    else
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                if(TestIF_File.iSiteMap[i][j]>0)                                //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
                    iSiteCount++;
            }
        }
    }

    return iSiteCount;
}
//------------------------------------------------------------------------------
//Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案
//------------------------------------------------------------------------------
ATK_RECIPE_INFO *ATKRecipeInfo;
ATK_RECIPE_INFO::ATK_RECIPE_INFO()
{
    slBinPassFail=new TStringList();
    SL           =new TStringList();
    slSortgate   =new TStringList();
    SL->Clear();
    slBinPassFail->Clear();
    slSortgate->Clear();
    STX.sprintf("%c", 0x02);
    ETX.sprintf("%c", 0x03);
}
//------------------------------------------------------------------------------
ATK_RECIPE_INFO::~ATK_RECIPE_INFO()
{
    try
    {
        SL->Clear();
        slBinPassFail->Clear();
        slSortgate->Clear();
        delete slBinPassFail;
        delete SL;
        delete slSortgate;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~ATK_RECIPE_INFO");
    }
}
//------------------------------------------------------------------------------
//Steven 20171221 (Wei) : Modify information for ATK
//------------------------------------------------------------------------------
void ATK_RECIPE_INFO::SaveFile()
{
    if(CUSTOMER_CODE!=CC_AMKOR_Korea &&
       CUSTOMER_CODE!=CC_AMKOR_China)
        return;

    int iBinSelCT=0, Data;
    AnsiString S, FileName, FileName2;
    bool bHasBin[eTrayCount];
    ZeroMemory(bHasBin, sizeof(bHasBin));
    S=GetLastOpenFN();
    FileName.sprintf("D:\\HT9045\\IniData\\Data\\%s\\Information.txt", S);
    FileName2.sprintf("D:\\eRMS\\Information.txt");
    MyForceDirectories("D:\\eRMS\\");
    SiteCount.sprintf("Site=%d", GetSiteCount(false));
    if(LastSet.iTemperature==Tempture_Ambient ||
       Temperature.iMachineTempMode==1)
    {
        WorkTemperBase.sprintf("Temp=25");
        SoakTime.sprintf("Soaktime=0");
    }
    else
    {
        WorkTemperBase.sprintf("Temp=%0.0f", Temperature.fWorkTemperBase);
        SoakTime.sprintf("Soaktime=%0.0f", Temperature.fSoakTime);
    }
    slBinPassFail->Clear();

    if(BinSelect[iTestRunMode].iCatDataT3Pos[0]==0)
    {
//        slBinPassFail->Add("0E");                                               //Steven 20210712 : Mark for #R160624-ATK-H9-01
    }
    else
    {
        if(Prod.bIsPassBin[0])
        {
            slBinPassFail->Add("0G");
        }
        else
        {
            slBinPassFail->Add("0R");
        }
    }
    int iSum=0;                                                                 //Jimmychiu 20230628 : Fixed Change the Binprofile value to G from R in information.txt
    for(int i=1; i<iTestBinCount; i++)                                          //Steven 20171204 (Wei) : TEST_MAX_BIN --> iTestBinCount
    {
        if(BinSelect[iTestRunMode].iCatDataT3Pos[i]==ePosNoUse)
        {
//            slBinPassFail->Add(AnsiString(i)+"R");                            //Steven 20210712 : Mark for #R160624-ATK-H9-01
        }
        else
        {
            iSum=BinSelect[iTestRunMode].iCatDataT3Pos[i]-1;                    //Jimmychiu 20230628 : Fixed Change the Binprofile value to G from R in information.txt
            if(iSum<0)
                slBinPassFail->Add(AnsiString(i)+"R");
            else if(BinSelect[iTestRunMode].iStackDefFailCate[iSum]==0)         //stack define fail
                slBinPassFail->Add(AnsiString(i)+"G");
            else
                slBinPassFail->Add(AnsiString(i)+"R");
        }
    }

    //Sortgate=1:1/2/3/4/5,2:6/8,3:7/9,4:10,5:11,6:12
    slSortgate->Clear();
    sSortgate="Sortgate=";
    if(TrayForm.iFixTrayMode)
    {
        iBinSelCT=9;                                                            //use up down
    }
    else
    {
        iBinSelCT=6;
    }

    for(int i=0; i<iBinSelCT; i++)
    {
        slSortgate->Add(AnsiString(""));
    }

    for(int i=0; i<iTestBinCount; i++)
    {
        Data=BinSelect[iTestRunMode].iCatDataT3Pos[i];
        if(Data<=ePosNoUse)                                                     //kevin 20140317 256 bin 0 start
          continue;

        for(int j=0; j<iBinSelCT; j++)
        {
            if(j==Data-1)
            {
                if(bHasBin[j]==false)
                    slSortgate->Strings[j]=slSortgate->Strings[j]+AnsiString(i);
                else
                    slSortgate->Strings[j]=slSortgate->Strings[j]+AnsiString("/")+AnsiString(i);
                bHasBin[j]=true;
            }
        }
    }

    if(Prod.bLinkTo6Tray[eAuto2]==true)
    {
        slSortgate->Strings[4]=slSortgate->Strings[3];
    }

    if(Prod.bLinkTo6Tray[eAuto3]==true)
    {
        slSortgate->Strings[5]=slSortgate->Strings[4];
    }

    for(int i=0; i<iBinSelCT; i++)
    {
        slSortgate->Strings[i]=AnsiString(i+1)+AnsiString(":")+slSortgate->Strings[i];
    }

    sSortgate+=slSortgate->CommaText;
    //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
    //<==
    int iSitemap=0,iRow=0,iCol=0;
    GetSiteNumberString(iSitemap, iRow, iCol);
    int iDir=SiteMapDirection();
    AnsiString sSmartBin=AMR.GetNormalFailBin();                                //Spencerlin 20260202 : 取出非 AutoRetest 或 CateR 之 Fail Bin 為 SmartBin
    AnsiString sUsedSitesNumber=GetUsedSitesNumber(iDir);
    sSitemap="Sitemap=";
    sSitemap+=GetParameterFormat("s", IntToStr(iSitemap));
    sSitemap+=GetParameterFormat("r", IntToStr(iCol));                          //Jimmychiu 20231128 : ATK think col is row and row is col
    sSitemap+=GetParameterFormat("c", IntToStr(iRow));
    sSitemap+=GetParameterFormat("d", IntToStr(iDir));
    sSitemap+=GetParameterFormat("no", sUsedSitesNumber);
    //<==
    //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
    SL->Clear();
    SL->Add(SiteCount);
    SL->Add(WorkTemperBase);
    SL->Add(SoakTime);
    SL->Add("Binprofile="+slBinPassFail->CommaText);
    SL->Add(sSortgate);
    SL->Add(sSitemap);                                                          //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
    SL->Add("Smartbin="+sSmartBin);                                             //RogerYang 20260407: 改大小寫 //Spencerlin 20260202 : 取出非 AutoRetest 或 CateR 之 Fail Bin 為 SmartBin
    SL->SaveToFile(FileName);
    SL->SaveToFile(FileName2);                                                  //Steven 20200317 : ATK說要存兩份
}
//------------------------------------------------------------------------------
void ATK_RECIPE_INFO::GetSiteNumberString(int &iSitemap, int &iRow, int &iCol)  //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
{
    iSitemap=iRow=iCol=0;
    if(TestIF_File.iTestMode==SingleSite)
    {
        iSitemap=1;
        iRow=1;
        iCol=1;
    }
    else if(TestIF_File.iTestMode==DualSite)
    {
        iSitemap=2;
        iRow=1;
        iCol=2;
    }
    else if(TestIF_File.iTestMode==TriSite1X3)
    {
        iSitemap=3;
        iRow=1;
        iCol=3;
    }
    else if(TestIF_File.iTestMode==QualSite1X4)
    {
        iSitemap=4;
        iRow=1;
        iCol=4;
    }
    else if(TestIF_File.iTestMode==DualSite2x1)
    {
        iSitemap=2;
        iRow=2;
        iCol=1;
    }
    else if(TestIF_File.iTestMode==QualSite2X2 ||
            TestIF_File.iTestMode==QualSite2X2N)
    {
        iSitemap=4;
        iRow=2;
        iCol=2;
    }
    else if(TestIF_File.iTestMode==_6Site2X3 ||
            TestIF_File.iTestMode==_6Site2X3N)                                  //Steven 20220425 : 2X3NN Mode
    {
        iSitemap=6;
        iRow=2;
        iCol=3;
    }
    else if(TestIF_File.iTestMode==_8Site2X4 ||
            TestIF_File.iTestMode==_8Site2X4N)                                  //Wei 20231211 : 2X4NN Mode
    {
        iSitemap=8;
        iRow=2;
        iCol=4;
    }
    else if(TestIF_File.iTestMode==_10Site2X5)
    {
        iSitemap=10;
        iRow=2;
        iCol=5;
    }
    else if(TestIF_File.iTestMode==_12Site2X6)
    {
        iSitemap=12;
        iRow=2;
        iCol=6;
    }
    else if(TestIF_File.iTestMode==_16Site2X8)
    {
        iSitemap=16;
        iRow=2;
        iCol=8;
    }
    else if(TestIF_File.iTestMode==_16Site4X4)
    {
        iSitemap=16;
        iRow=4;
        iCol=4;
    }
    else if(TestIF_File.iTestMode==_32Site4X8N ||
            TestIF_File.iTestMode==_32Site4X8M)
    {
        iSitemap=32;
        iRow=4;
        iCol=8;
    }
    else
    {
        iSitemap=0;
        iRow=0;
        iCol=0;
    }
}
//------------------------------------------------------------------------------
AnsiString ATK_RECIPE_INFO::GetUsedSitesNumber(int iDir)                        //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
{
    AnsiString sReturn="";
    if(iDir==0)
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(TestIF_File.iSiteMap[i][j]<0)
                    TestIF_File.iSiteMap[i][j]=0;

                if(i==0 && j==0)
                {}
                else
                {
                    sReturn+=",";
                }
                sReturn+=AnsiString().sprintf("%d", TestIF_File.iSiteMap[i][j]);
            }
        }
    }
    else
    {
        sReturn=AnsiString().sprintf("%d", TestSocket.iShtCnt);
    }
    return sReturn;
}
//------------------------------------------------------------------------------
int ATK_RECIPE_INFO::SiteMapDirection()                                         //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
{
    //單列由左到右是1，單行由上到下是2，單列由右到左是3，單行由下到上是4，由上到下且由左到右是5，由右到左且由下到上是6，由上到下且由左到右是7，由下到上且由右到左是8，由右到左且由上到下是9，由左到右且由下到上是10，由上到下且由右到左是11，由下到上且由左到右是12
    int order=0, iExpectedValue=1;
    bool bCompliant=false;
    //判斷是否關Site，有關為0
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestIF_File.iSiteMap[i][j]==0)
            {
                return order;
            }
        }
    }

    if(TestSocket.iShtRow==1)                                                   //單列
    {
        //單列由左到右是1
        iExpectedValue=1;
        bCompliant=true;
        for(int i=0; i<TestSocket.iShtCol; i++)
        {
            if(TestIF_File.iSiteMap[0][i]!=iExpectedValue)
            {
                bCompliant=false;
                break;
            }
            iExpectedValue++;
        }

        if(bCompliant==true)
            return 1;
        //單列由右到左是3
        iExpectedValue=1;
        bCompliant=true;
        for(int i=TestSocket.iShtCol-1; i>=0; i--)
        {
            if(TestIF_File.iSiteMap[0][i]!=iExpectedValue)
            {
                bCompliant=false;
                break;
            }
            iExpectedValue++;
        }

        if(bCompliant==true)
            return 3;
    }
    else if(TestSocket.iShtCol==1)                                              //單行
    {
        //單行由上到下是2
        iExpectedValue=1;
        bCompliant=true;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            if(TestIF_File.iSiteMap[i][0]!=iExpectedValue)
            {
                bCompliant=false;
                break;
            }
            iExpectedValue++;
        }

        if(bCompliant==true)
            return 2;
        //單行由下到上是4
        iExpectedValue=1;
        bCompliant=true;
        for(int i=TestSocket.iShtRow-1; i>=0; i--)
        {
            if(TestIF_File.iSiteMap[i][0]!=iExpectedValue)
            {
                bCompliant=false;
                break;
            }
            iExpectedValue++;
        }

        if(bCompliant==true)
            return 4;
    }
    else
    {
        //由上到下且由左到右是5
        iExpectedValue=1;
        bCompliant=true;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(TestIF_File.iSiteMap[i][j]!=iExpectedValue)
                {
                    bCompliant=false;
                    break;
                }
                iExpectedValue++;
            }
        }

        if(bCompliant==true)
            return 5;
        //由右到左且由下到上是6
        iExpectedValue=1;
        bCompliant=true;
        for(int i=TestSocket.iShtRow-1; i>=0; i--)
        {
            for(int j=TestSocket.iShtCol-1; j>=0; j--)
            {
                if(TestIF_File.iSiteMap[i][j]!=iExpectedValue)
                {
                    bCompliant=false;
                    break;
                }
                iExpectedValue++;
            }
        }

        if(bCompliant==true)
            return 6;
        //由上到下且由左到右是7
        iExpectedValue=1;
        bCompliant=true;
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                if(TestIF_File.iSiteMap[i][j]!=iExpectedValue)
                {
                    bCompliant=false;
                    break;
                }
                iExpectedValue++;
            }
        }

        if(bCompliant==true)
            return 7;
        //由下到上且由右到左是8
        iExpectedValue=1;
        bCompliant=true;
        for(int j=TestSocket.iShtCol-1; j>=0; j--)
        {
            for(int i=TestSocket.iShtRow-1; i>=0; i--)
            {
                if(TestIF_File.iSiteMap[i][j]!=iExpectedValue)
                {
                    bCompliant=false;
                    break;
                }
                iExpectedValue++;
            }
        }

        if(bCompliant==true)
            return 8;
        //由右到左且由上到下是9
        iExpectedValue=1;
        bCompliant=true;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=TestSocket.iShtCol-1; j>=0; j--)
            {
                if(TestIF_File.iSiteMap[i][j]!=iExpectedValue)
                {
                    bCompliant=false;
                    break;
                }
                iExpectedValue++;
            }
        }

        if(bCompliant==true)
            return 9;
        //由左到右且由下到上是10
        iExpectedValue=1;
        bCompliant=true;
        for(int i=TestSocket.iShtRow-1; i>=0; i--)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(TestIF_File.iSiteMap[i][j]!=iExpectedValue)
                {
                    bCompliant=false;
                    break;
                }
                iExpectedValue++;
            }
        }

        if(bCompliant==true)
            return 10;
        //由上到下且由右到左是11
        iExpectedValue=1;
        bCompliant=true;
        for(int j=TestSocket.iShtCol-1; j>=0; j--)
        {
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                if(TestIF_File.iSiteMap[i][j]!=iExpectedValue)
                {
                    bCompliant=false;
                    break;
                }
                iExpectedValue++;
            }
        }

        if(bCompliant==true)
            return 11;
        //由下到上且由左到右是12
        iExpectedValue=1;
        bCompliant=true;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=TestSocket.iShtCol-1; j>=0; j--)
            {
                if(TestIF_File.iSiteMap[i][j]!=iExpectedValue)
                {
                    bCompliant=false;
                    break;
                }
                iExpectedValue++;
            }
        }

        if(bCompliant==true)
            return 12;
    }
    return order;
}
//------------------------------------------------------------------------------
AnsiString ATK_RECIPE_INFO::GetParameterFormat(AnsiString sName, AnsiString sValue)                                     //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt
{
    return AnsiString().sprintf("%s:%s%s", sName, sValue, GetForwardSlash());
}
//------------------------------------------------------------------------------
//AI(GA1-B2) 20260804: RUN_INFO::RUN_INFO()/~RUN_INFO()/InitialData() deleted from
//  here -- dead duplicates of the ALREADY-LIVE copies at this file's top (W4 wave,
//  lines ~133-178), which compiled and ran as the static-init path since that
//  wave landed. Leaving both would be a redefinition (hard compile error), and
//  the two bodies are textually identical (byte-diffed this pass) -- nothing was
//  lost by removing this copy.
void RUN_INFO::InitialDailyData()                                               //Steven 20250528 : By Day Jam Rate
{
    iDailyCount=0;
    iToday=SystemDate;
    sToday.sprintf("%04d-%02d-%02d", SystemYear, SystemMonth, SystemDate);
    vDailyJam.clear();
    DailyJamFileName="";
}
//---------------------------------------------------------------------------
void RUN_INFO::SetLotStartTime()                                                //Sam 20240426 : Add BarCoder Inspection Report
{
    LotStartTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d",                       //pig 2014.05.09 KYEC Issue
                    SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    LotStartYear    =SystemYear;
    LotStartMonth   =SystemMonth;
    LotStartDate    =SystemDate;
    LotStartHour    =SystemHour;
    LotStartMin     =SystemMin;
    LotStartSec     =SystemSec;
}
//------------------------------------------------------------------------------
void RUN_INFO::AddAlarm(AnsiString Jam, AnsiString Mess)
{
    if(Jam.AnsiPos("JAM")!=0)
    {
        vJamIter=vByLotJam.find(Jam);
        if(vJamIter==vByLotJam.end())
        {
            JAM_COUNT Temp;
            Temp.JamCode=Jam;
            Temp.Message=Mess;
            Temp.iCount=1;
            vByLotJam[Jam]=Temp;
        }
        else
        {
            vJamIter->second.iCount++;
        }

        if(iToday!=SystemDate)
        {
            SaveJamRateByDay();
            InitialDailyData();
        }

        vJamIter=vDailyJam.find(Jam);
        if(vJamIter==vDailyJam.end())
        {
            JAM_COUNT Temp;
            Temp.JamCode=Jam;
            Temp.Message=Mess;
            Temp.iCount=1;
            vDailyJam[Jam]=Temp;
        }
        else
        {
            vJamIter->second.iCount++;
        }
    }
}
//------------------------------------------------------------------------------
bool struct_cmp_by_count(JAM_COUNT a, JAM_COUNT b)
{
    return a.iCount>b.iCount;
}
//------------------------------------------------------------------------------
void (*W906_SaveJamRateByDayBody)(bool) = 0;  void RUN_INFO::SaveJamRateByDay(bool bUpload)                                   //Steven 20250528 : By Day Jam Rate  //AI(W906-JAMDAY) 20261001: W906_SaveJamRateByDayBody = the body when wb_serve installs one (see below)
{   if(W906_SaveJamRateByDayBody!=0) W906_SaveJamRateByDayBody(bUpload);   //AI(W906-JAMDAY) 20261001: census 129 (e) E-FT2-011 -- the day rollover in AddAlarm (:1043-1047) cleared yesterday's counts without saving them; the golden-faithful body is FileRW/MainClose.cpp W906_RunInfo_SaveJamRateByDay (wb_serve only), runs here BEFORE InitialDailyData() as golden; FTP / N10 upload stays gated there
#if 0 // TODO(GA1-B2): blocked by FileInfo@ProductionInfo/FileInfo.h+.cpp (golden class,   [AI(W906-JAMDAY) 20261001: STALE -- FileInfo IS ported (ProductionInfo/FileInfo.cpp:457 PathCombin); the real reason is the library split (this file is ht9045_globals, FileInfo is ht9045_sm), hence the hook above]
      // NOT ported anywhere in this tree) -- FileInfo().PathCombin(sDailyJamPath,sFileName)
      // computes DailyJamFileName below, which every subsequent line reads or writes;
      // partial-ungating just that one line would leave DailyJamFileName stale/empty and
      // silently wrong for the rest of the function, so the whole body stays gated
      // together. Also depends on FormHS (golden HS_Function.h, not ported) for the
      // upload branch. InitialDailyData() at the tail is independent and stays live.
    AnsiString str, sFileName, sPath;
    int iJamCount=0, iTag;

    TStringList *slReport=new TStringList();

    MyForceDirectories(sDailyJamPath);

    sFileName.sprintf("%s_%s_%s_DailyJamRate.txt", IniConfig.sMachineType, IniConfig.SocketHandlerID, sToday);
    DailyJamFileName=FileInfo().PathCombin(sDailyJamPath, sFileName);           //Steven 20250812 : 修正上傳檔名

    str.sprintf("Date: %s", sToday);
    slReport->Add(str);

    str.sprintf("Unloading counter: %d", iDailyCount);
    slReport->Add(str);

    iJamCount=0;
    for(vJamIter=vByLotJam.begin(); vJamIter!=vByLotJam.end(); vJamIter++)
    {
        iJamCount+=vJamIter->second.iCount;
    }

    str.sprintf("Jam counter: %d", iJamCount);
    slReport->Add(str);

    if(iJamCount==0)
    {
        str.sprintf("MUBJ: 0/%d", iDailyCount);
    }
    else
    {
        if((iDailyCount/iJamCount)<1)
            str.sprintf("MUBJ: 1/1");
        else
            str.sprintf("MUBJ: 1/%d", iDailyCount/iJamCount);
    }
    slReport->Add(str);

    iTag=0;
    std::vector<JAM_COUNT> vec;
    for(vJamIter=vDailyJam.begin(); vJamIter!=vDailyJam.end(); vJamIter++)
    {
        JAM_COUNT Temp;
        Temp.JamCode=vJamIter->second.JamCode;
        Temp.Message=vJamIter->second.Message;
        Temp.iCount =vJamIter->second.iCount;
        vec.push_back(Temp);
    }

    sort(vec.begin(), vec.end(), struct_cmp_by_count);
    for(unsigned int i=0; i<vec.size(); i++)
    {
        iTag++;
        str.sprintf("%d %s %d %s", iTag, vec[i].JamCode, vec[i].iCount, vec[i].Message);
        slReport->Add(str);
    }

    slReport->SaveToFile(DailyJamFileName);
    slReport->Clear();
    vec.clear();
    delete slReport;

    if(bUpload==true &&
       IniConfig.bN10_DailyUploadProdData==true)                                //Steven 20250527 : 上傳Jam Rate
    {
        if(FileExists(DailyJamFileName)==true)
        {
            if(IniConfig.iN10UploadMethod==0)
            {
                FormHS->UpDataToServerByFTP(ExtractFilePath(DailyJamFileName), ExtractFileName(DailyJamFileName), "JamRateDaily");
            }
            else
            {
                sPath.sprintf("%sJamRateByDay\\%04d", IncludeTrailingPathDelimiter(IniConfig.sN10UploadDrivePath), SystemYear);
                if(MyForceDirectories(sPath, "[N10] Upload_JamRateByDay_Log")!=1)
                {
                    ;
                }
                else
                {
                    str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", DailyJamFileName, sPath);
                    slExe->Add(str);

                    try
                    {
                        slExe->SaveToFile("D:\\HT9045_Log\\JamRateByDay.bat");
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TFormHS::UpDataToServer_KYEC");
                    }

                    for(int i=0; i<slExe->Count; i++)
                    {
                        RecordProcess(slExe->Strings[i]);
                    }

                    try
                    {
                        ExecZipCommand("D:\\HT9045_Log\\JamRateByDay.bat", " ");
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TFormHS::UpDataToServer_KYEC ExecZipCommand");
                    }

                    slExe->Clear();
                }
            }
        }
    }
#endif // TODO(GA1-B2): FileInfo (see banner above)

    InitialDailyData();
}
//------------------------------------------------------------------------------
void RUN_INFO::ReadJamRateByDay()                                               //Steven 20250528 : By Day Jam Rate
{
    AnsiString str, sFileName;
    int iPos;
    InitialDailyData();

#if 0 // TODO(GA1-B2): blocked by FileInfo@ProductionInfo/FileInfo.h+.cpp (golden class,
      // NOT ported anywhere in this tree) -- same DailyJamFileName-spine reasoning as
      // SaveJamRateByDay above: FileInfo().PathCombin(...) below feeds every subsequent
      // line (LoadFromFile/parse), so the whole body (after the top InitialDailyData()
      // reset, which stays live) is gated together.
    MyForceDirectories(sDailyJamPath);

    sFileName.sprintf("%s_%s_%s_DailyJamRate.txt", IniConfig.sMachineType, IniConfig.SocketHandlerID, sToday);
    DailyJamFileName=FileInfo().PathCombin(sDailyJamPath, sFileName);           //Steven 20250812 : 修正上傳檔名

    if(FileExists(DailyJamFileName)==false)
        return;

    TStringList *slReport=new TStringList();
    TStringList *sList=new TStringList();
    slReport->LoadFromFile(DailyJamFileName);
    sList->Delimiter=' ';

    for(int i=0; i<slReport->Count; i++)
    {
        str=slReport->Strings[i];
        if(str.AnsiPos("Date:")==1)
        {
        }
        else if(str.AnsiPos("Unloading counter:")==1)
        {
            iPos=str.AnsiPos(":");
            str=str.SubString(iPos+1, str.Length());
            iDailyCount=atoi(str.c_str());
        }
        else if(str.AnsiPos("Jam counter:")==1)
        {
        }
        else if(str.AnsiPos("MUBJ:")==1)
        {
        }
        else
        {
            sList->Clear();
            sList->CommaText=str;
            JAM_COUNT Temp;
            Temp.JamCode=sList->Strings[1];
            Temp.iCount =atoi(sList->Strings[2].c_str());
            sList->Delete(2);
            sList->Delete(1);
            sList->Delete(0);
            str=StringReplace(sList->Text, "\r\n", " ", TReplaceFlags()<<rfReplaceAll);
            Temp.Message=str;
            vDailyJam[Temp.JamCode]=Temp;
        }
    }

    sList->Clear();
    slReport->Clear();
    delete slReport;
    delete sList;
#endif // TODO(GA1-B2): FileInfo (see banner above)
}
//------------------------------------------------------------------------------
void RUN_INFO::SaveJamRateByLot(bool bUpload)                                   //Steven 20200415 : SCC要By Lot Jam Rate
{
    AnsiString str, sFileName, sPath;
    int iJamCount=0, iTag;

    TStringList *slReport=new TStringList();
    if(bLotStart==false || LotNo=="")
        return ;

    if(IniConfig.bVTESTFunction==true)                                          //jou 20241015 : by lot jamstat報告要求
    {
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GA1-B2 reason expired -- fMesSystem is ported (forms/fMesSystem.h extern, forms/fMesSystem.cpp new-ed at file scope) with lbledtCustLotNum / LabeledEditLotNo / lbledtC1; golden cprod.cpp:1182
      // TfMesSystem form, no forms/fMesSystem.h port exists) -- VTEST-mode jam-rate
      // filename branch. Narrow gap: only affects customers with IniConfig.bVTESTFunction
      // true (VTEST is a minority config); sJamRatePath/sFileName keep their prior value
      // when this flag is set, instead of the fMesSystem-derived name.
        sJamRatePath="D:\\PnPh\\report\\LotAlarm";
        sFileName.sprintf("%s-%s-%s-%s-%s-%s-%04d%02d%02d%02d%02d%02d.txt",     IniConfig.SocketHandlerID,
                                                                                fMesSystem->lbledtCustLotNum->Text,
                                                                                fMesSystem->LabeledEditLotNo->Text,
                                                                                fMesSystem->lbledtC1->Text,
                                                                                fLotInfo->cbRunMode->Text,
                                                                                fLotInfo->cbProcess->Text,
                                                                                SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
#endif // TODO(GA1-B2): fMesSystem
    }
    else
    {
        sFileName.sprintf("%s_%s_%04d%02d%02d-%02d%02d%02d_%s_%s_%s_JamRateByLot.txt",                                  //Steven 20250812 : 修正上傳檔名
                                                                                IniConfig.sMachineType,
                                                                                IniConfig.SocketHandlerID,
                                                                                SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec,
                                                                                LotNo,
                                                                                fLotInfo->cbRunMode->Text,
                                                                                fLotInfo->cbProcess->Text);
    }
    MyForceDirectories(sJamRatePath);
    JamRateFileName.sprintf("%s\\%s", sJamRatePath, sFileName);

    str.sprintf("Lot No: %s", LotNo);
    slReport->Add(str);

    str.sprintf("Lot start time to end time: %s-%s", LotStartTime, LotEndTime);
    slReport->Add(str);

    str.sprintf("Unloading counter: %d", TastCategory.iTotalSocket);
    slReport->Add(str);

    iJamCount=0;
    for(vJamIter=vByLotJam.begin(); vJamIter!=vByLotJam.end(); vJamIter++)
    {
        iJamCount+=vJamIter->second.iCount;
    }

    str.sprintf("Jam counter: %d", iJamCount);
    slReport->Add(str);

    if(iJamCount==0)
    {
        str.sprintf("MUBJ: 0/%d", TastCategory.iTotalSocket);
    }
    else
    {
        if((TastCategory.iTotalSocket/iJamCount)<1)
            str.sprintf("MUBJ: 1/1");
        else
            str.sprintf("MUBJ: 1/%d", TastCategory.iTotalSocket/iJamCount);
    }
    slReport->Add(str);

    iTag=0;
    std::vector<JAM_COUNT> vec;
    for(vJamIter=vByLotJam.begin(); vJamIter!=vByLotJam.end(); vJamIter++)
    {
        JAM_COUNT Temp;
        Temp.JamCode=vJamIter->second.JamCode;
        Temp.Message=vJamIter->second.Message;
        Temp.iCount =vJamIter->second.iCount;
        vec.push_back(Temp);
    }

    sort(vec.begin(), vec.end(), struct_cmp_by_count);
    if(CUSTOMER_CODE==CC_SCC)
    {
        for(unsigned int i=0; i<vec.size(); i++)
        {
            str.sprintf("%d %s %s %d", iTag, vec[i].JamCode, vec[i].Message, vec[i].iCount);
            slReport->Add(str);
        }
    }
    else
    {
        for(unsigned int i=0; i<vec.size(); i++)
        {
            str.sprintf("%d %s %d %s", iTag, vec[i].JamCode, vec[i].iCount, vec[i].Message);
            slReport->Add(str);
        }
    }

    slReport->SaveToFile(JamRateFileName);
    slReport->Clear();
    vec.clear();
    delete slReport;

    if(bUpload==true &&
       IniConfig.bN10_UploadSummaryToFTP==true)                                 //Steven 20250527 : By Lot Jam Rate
    {
        if(FileExists(JamRateFileName)==true)
        {
            if(IniConfig.iN10UploadMethod==0)
            {
#if 0 // TODO(GA1-B2): blocked by FormHS@HS_Function.h (golden TFormHS form, not ported   [AI(W906-POOL2) 20261008: STALE -- TFormHS is ported (forms/fHS.h) but the live FormHS global is the SCK_ART test stub (Automation/SCK_ART_Remainder.h W5SckArtRem_FormHSStub, records the call, no FTP); stays until a real TFormHS instance exists]
      // anywhere in this tree) -- the iN10UploadMethod==0 (direct-FTP) upload branch only.
                FormHS->UpDataToServerByFTP(IncludeTrailingPathDelimiter(sJamRatePath), sFileName, "JamRateByLot");
#endif // TODO(GA1-B2): FormHS
            }
            else
            {
                sPath.sprintf("%sJamRateByLot\\%04d", IncludeTrailingPathDelimiter(IniConfig.sN10UploadDrivePath), SystemYear);
                if(MyForceDirectories(sPath, "[N10] Upload_JamRateByLot_Log")!=1)
                {
                    ;
                }
                else
                {
                    str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", JamRateFileName, sPath);
                    slExe->Add(str);

                    try
                    {
                        slExe->SaveToFile("D:\\HT9045_Log\\JamRateByLot.bat");
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TFormHS::UpDataToServer_KYEC");
                    }

                    for(int i=0; i<slExe->Count; i++)
                    {
                        RecordProcess(slExe->Strings[i]);
                    }

                    try
                    {
                        ExecZipCommand("D:\\HT9045_Log\\JamRateByLot.bat", " ");
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TFormHS::UpDataToServer_KYEC ExecZipCommand");
                    }

                    slExe->Clear();
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
bool WriteData(char *cFName, char *ptr, int size)                               //jou 2013-01-15 machinerrecoder.dat save fail
{
    DWORD wtfz;
    HANDLE Fp;

    Fp=CreateFile(cFName, FILE_SHARE_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(Fp!=INVALID_HANDLE_VALUE)
    {
        WriteFile(Fp, ptr, size, &wtfz, NULL);
        CloseHandle(Fp);
        return true;
    }
    else
    {
        ShowErrorMessage("WAR1682", 0, MMSystem, 0, AnsiString(cFName));
    }
    CloseHandle(Fp);
    return false;
}
//------------------------------------------------------------------------------
bool ReadData(char *cFName,char *ptr,int size)                                  //jou 2013-01-15 machinerrecoder.dat save fail
{
    DWORD rdfz;
    HANDLE Fp;

    Fp=CreateFile(cFName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if(Fp!=INVALID_HANDLE_VALUE)
    {
        ReadFile(Fp, ptr, size, &rdfz, NULL);
        CloseHandle(Fp);
        return true;
    }
    else
    {
        ShowErrorMessage("WAR1681", 0, MMSystem, 0, AnsiString(cFName));
    }
    CloseHandle(Fp);
    return false;
}
//------------------------------------------------------------------------------
void SavePassword()
{
    FILE *Fp=fopen("d:\\HT9045\\system\\login.dat", "wb");
    if(Fp!=NULL)
    {
        fwrite((char *)&USER.RecordCT, sizeof(PASS_WORD), 1, Fp);
        fclose(Fp);
    }
    else
    {
        ShowErrorMessage("WAR1682", 0, MMSystem, 0, "login.dat");
    }
}
//------------------------------------------------------------------------------
void ReadPassword()
{
    FILE *Fp=fopen("d:\\HT9045\\system\\login.dat", "rb");
    if(Fp!=NULL)
    {
        fread((char *)&USER.RecordCT, sizeof(PASS_WORD), 1, Fp);
        fclose(Fp);
    }
    else
    {
        memset(&USER.RecordCT, 0, sizeof(PASS_WORD));                           //Steven 20200514 : USER.ID[0][0] --> USER.RecordCT 解除記憶體破壞
    }
}
//------------------------------------------------------------------------------
bool CheckFileExist(AnsiString cFName)                                          //Ken 20210702 AddPadInterface
{
    //AI(GA1-B2) 20260804: _rtl_open/_rtl_close (BCB6 RTL internals) -> open/close;
    //  bare names, no wrapper, same verified convention as SECSGEM/uHGemEquipment.cpp:47-52.
    if(access(cFName.c_str(), 0)==0)
    {
        int fhandle;
        if((fhandle=open(cFName.c_str(), O_RDONLY))==-1)
            return false;
        close(fhandle);
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool CheckFileCanAccess(char *cFName)
{
    int fhandle;
    if((fhandle=open(cFName, O_RDWR))==-1)
        return false;
    close(fhandle);
    return true;
}
//------------------------------------------------------------------------------
void InitialReserveEmptyPoint()
{
    for(int j=0; j<1000; j++)                                                   //ChungHung 20111215 嘗試將Fix3放滿
    {
        ReserverEmptyPoint[j].bHasData=false;
    }
}
//------------------------------------------------------------------------------
bool EnableFix3UseCylinder()                                                    //JimmyChiu 20220927 : Stepper Motor Control in Fix3
{
    if(AUTO_EMPTY_COLOR>=3)
        return false;

    return (FIX3_FULL_PLACE==Fix3K_UseCylinder ||
            FIX3_FULL_PLACE==Fix3K_UseCylinder46LA ||
            FIX3_FULL_PLACE==Fix3K_UseStepperMotor);
}
//------------------------------------------------------------------------------
void ReserveEmptyPoint(int iAutoTray, int iXpos, int iYpos)
{
    if((EnableFix3UseCylinder()) && iAutoTray==MManualTray3)                    //ChungHung 20140722 add for HT9046LA
        return;

    if(iAutoTray==MMAuto1 ||iAutoTray==iAutoRight ||                            //jou 20170719 (wei) : 修正outarm 放置到 Auto3 時會空格異常
       iAutoTray==iFixRight || iFixRightHalf)
    {
        for(int i=0; i<1000; i++)                                               //ChungHung 20111215 嘗試將Fix3放滿
        {
            if(ReserverEmptyPoint[i].bHasData==false)
            {
                ReserverEmptyPoint[i].bHasData=true;
                ReserverEmptyPoint[i].iXPoint=iXpos;
                ReserverEmptyPoint[i].iYPoint=iYpos;
                ReserverEmptyPoint[i].iWhichAuto=iAutoTray;
                break;
            }
        }
    }
}
//----------------------------------------------------------------------------
void ReversionEmptyPoint()
{
    int iXpos, iYpos;

    if(MOT[MMAuto1].Tray.FullIC())
    {
        for(int i=0; i<1000; i++)                                               //ChungHung 20111215 嘗試將Fix3放滿
        {
            if(ReserverEmptyPoint[i].bHasData &&
               ReserverEmptyPoint[i].iWhichAuto==MMAuto1)                       //ChungHung 20111215 嘗試將Fix3放滿
            {
                ReserverEmptyPoint[i].bHasData=false;
            }
        }
        return;
    }

    for(int i=0; i<1000; i++)                                                   //ChungHung 20111215 嘗試將Fix3放滿
    {
        if(ReserverEmptyPoint[i].bHasData)
        {
            ReserverEmptyPoint[i].bHasData=false;
            iXpos=ReserverEmptyPoint[i].iXPoint;
            iYpos=ReserverEmptyPoint[i].iYPoint;
            MOT[ReserverEmptyPoint[i].iWhichAuto].SetTraySingleData(iXpos, iYpos, NULL_IC);
        }
    }
}
//-------------------------------------------------------------------------
bool FileDataCompare(char *cFName1, char *cFName2)
{
    FILE *FileHandle1, *FileHandle2;
    int iFileLength1, iFileLength2;
    char pszBuffer1[102400], pszBuffer2[102400];

    FileHandle1 = std::fopen(cFName1, "rb");
    FileHandle2 = std::fopen(cFName2, "rb");

    if((FileHandle1==NULL) || (FileHandle2==NULL))
    {
        if(FileHandle1!=NULL)
            std::fclose(FileHandle1);
        if(FileHandle2!=NULL)
            std::fclose(FileHandle2);
        return false;
    }
    std::fseek(FileHandle1, SEEK_SET, 0);
    std::fseek(FileHandle2, SEEK_SET, 0);
    std::fseek(FileHandle1, 0L, SEEK_END);
    std::fseek(FileHandle2, 0L, SEEK_END);
    iFileLength1=ftell(FileHandle1);
    iFileLength2=ftell(FileHandle2);
    std::fseek(FileHandle1, SEEK_SET, 0);
    std::fseek(FileHandle2, SEEK_SET, 0);
    if(iFileLength1>=102399)
        iFileLength1=102398;
    if(iFileLength2>=102399)
        iFileLength2=102398;

    std::fread(pszBuffer1, iFileLength1+1, 1, FileHandle1);
    std::fread(pszBuffer2, iFileLength2+1, 1, FileHandle2);

    std::fclose(FileHandle1);
    std::fclose(FileHandle2);

    if(iFileLength1!=iFileLength2)
    {
        return false;
    }

    for(int i=0; i<iFileLength1; i++)
    {
        if(pszBuffer1[i]!=pszBuffer2[i])
        {
            return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
unsigned long GetFileLength(char* path)
{
    unsigned long pos=0;
    unsigned long len = 0;

    if(FileExists(path)==false)
        return 0;

    FILE *fileName=std::fopen(path, "rb");
    if(fileName!=NULL)
    {
        pos=ftell(fileName);
        fseek(fileName, 0L, SEEK_END);
        len=ftell(fileName);
        fseek(fileName, pos, SEEK_SET);
        fclose(fileName);
    }
    return len;
}
//---------------------------------------------------------------------------
void TestModeDutOnOffToLastSetUseTestSocket()                                   //Steven 20231018 : fixed for bLastSetInSetUpFile
{
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<8; j++)                                                  //ChungHung 20130910 alter for SCK can close site by Index
        {
            LastSet.bUseTestSocket[0][i][j]=TestMode.iDutOnOff[0][i][j];
            LastSet.bUseTestSocket[1][i][j]=TestMode.iDutOnOff[1][i][j];
            LastSet.bUseTestSocketEE[0][i][j]=TestMode.iDutOnOffEE[0][i][j];    //Steven 20241220 : add
            LastSet.bUseTestSocketEE[1][i][j]=TestMode.iDutOnOffEE[1][i][j];
        }
    }
}
//---------------------------------------------------------------------------
void LastSetUseTestSocketToTestModeDutOnOff()                                   //Steven 20231018 : fixed for bLastSetInSetUpFile
{
    if(HasICUnderMachine() ||                                                   //Steven 20160518 : 機台內有料的話, 不能變更狀態
       HasAnyICInMachine())                                                     //kevin 20160818 機台生產中不能改資料
    {
        TestMode.iTemperatureMode   =LastSet.iTemperature;
        TestMode.iRunMode           =LastSet.iRealDummy;
        TestMode.iTestConnection    =LastSet.iTester;
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<8; j++)
            {
                TestMode.iDutOnOff[0][i][j]=LastSet.bUseTestSocket[0][i][j];
                TestMode.iDutOnOff[1][i][j]=LastSet.bUseTestSocket[1][i][j];
                TestMode.iDutOnOffEE[0][i][j]=LastSet.bUseTestSocketEE[0][i][j];                                        //Steven 20241220 : add
                TestMode.iDutOnOffEE[1][i][j]=LastSet.bUseTestSocketEE[1][i][j];
            }
        }
        SaveTestMode();
    }
    else
    {
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<8; j++)
            {
                LastSet.bUseTestSocket[0][i][j]=TestMode.iDutOnOff[0][i][j];
                LastSet.bUseTestSocket[1][i][j]=TestMode.iDutOnOff[1][i][j];
                LastSet.bUseTestSocketEE[0][i][j]=TestMode.iDutOnOffEE[0][i][j];                                        //Steven 20241220 : add
                LastSet.bUseTestSocketEE[1][i][j]=TestMode.iDutOnOffEE[1][i][j];
                bTestSiteUse[0][i][j]=TestMode.iDutOnOff[0][i][j];
                bTestSiteUse[1][i][j]=TestMode.iDutOnOff[1][i][j];
                bTestSiteUseEE[0][i][j]=TestMode.iDutOnOffEE[0][i][j];          //Steven 20241220 : add
                bTestSiteUseEE[1][i][j]=TestMode.iDutOnOffEE[1][i][j];
            }
        }
    }
}
AnsiString W906_LastDataPath(const char* goldenPath);   // AI(W906-LASTDATA-TESTDIR) 20260926: 定義在本檔尾；這一行原本是 //---- 分隔線（同一行放宣告，行數不變）
bool ReadLastDataFile()
{
    String sFileName1, sFileName2;
    unsigned long iSize1=0, iSize2=0;
    DWORD rdfz;
    HANDLE Fp;

    ReadTestMode();                                                             //Ifor 20161116 Fix 開關Site Even 紀錄異常  //Steven 20231018 : move up
    LastSet.bD41TestSocketICCheckSkip=false;                                    //Steven 20160328 : [D41]強制false, 就是不能跳過index check
    //ChungHung 20120829 add
    iSize1 = GetFileLength(const_cast<char*>(W906_LastDataPath("D:\\HT9045\\system\\lastdata.dat").c_str()));   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    iSize2 = GetFileLength(const_cast<char*>(W906_LastDataPath("d:\\HT9045\\system\\lastdata_backup.dat").c_str()));   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    FILE *Fp3;
    if(iSize1==0 && iSize2==0)
    {
        //讀取備份的資料
        Fp3=std::fopen(W906_LastDataPath("D:\\HT9045\\system\\lastdata_backup2.dat").c_str(), "rb");   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
        if(Fp3!=NULL)
        {
            std::fread((char *)&LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET), 1, Fp3);
            std::fclose(Fp3);

            if(CosFunction.bLastSetInSetUpFile)                                 //Steven 20111019
            {
                LastSetUseTestSocketToTestModeDutOnOff();                       //Steven 20231018 : fixed for bLastSetInSetUpFile
                LastSet.iTemperature    =TestMode.iTemperatureMode;
                LastSet.iRealDummy      =TestMode.iRunMode;
                LastSet.iTester         =TestMode.iTestConnection;
                TestModeDutOnOffToLastSetUseTestSocket();                       //Steven 20231018 : fixed for bLastSetInSetUpFile
            }
            return true;
        }
        else
        {
            Fp3=NULL;                                                           //Jimmychiu 20230307 Null pointer dereference
        }
    }
    //ChungHung 20120829 end

    Fp=CreateFile(W906_LastDataPath("D:\\HT9045\\system\\lastdata.dat").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    if(Fp!=INVALID_HANDLE_VALUE)
    {
        ReadFile(Fp, (char *)&LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET), &rdfz, NULL);
        CloseHandle(Fp);

        sFileName1=W906_LastDataPath("d:\\HT9045\\system\\lastdata.dat");   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
        sFileName2=W906_LastDataPath("d:\\HT9045\\system\\lastdata_backup.dat");   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
        if(FileExists(sFileName2))
        {
            //AI(GA1-B2) 20260804: const_cast -- FileDataCompare's cprod.h decl takes char*
            //  (non-const, BCB6 looseness; body never writes through either pointer), and
            //  cprod.h is read-only, so the fix is at this call site.
            if(FileDataCompare(const_cast<char*>(sFileName1.c_str()),const_cast<char*>(sFileName2.c_str()))==false)
            {
                Fp=CreateFile(W906_LastDataPath("D:\\HT9045\\system\\lastdata_backup.dat").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
                if(Fp!=INVALID_HANDLE_VALUE)
                {
                    ReadFile(Fp, (char *)&LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET), &rdfz, NULL);
                    CloseHandle(Fp);
                }
                else
                {
                    CloseHandle(Fp);

                    strncpy(LastSet.LastOpenFilename, "", sizeof(LastSet.LastOpenFilename));
                    //LastSet.Version=900;
                    WriteLastDataFile();
                    return false;
                }
            }
        }
    }
    else
    {
        CloseHandle(Fp);
        Fp=CreateFile(W906_LastDataPath("D:\\HT9045\\system\\lastdata_backup.dat").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）

        if(Fp!=INVALID_HANDLE_VALUE)
        {
            ReadFile(Fp, (char *)&LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET), &rdfz, NULL);
            CloseHandle(Fp);
        }
        else
        {
            CloseHandle(Fp);
            strncpy(LastSet.LastOpenFilename, "", sizeof(LastSet.LastOpenFilename));
            //LastSet.Version=900;
            WriteLastDataFile();
            return false;
        }
    }

    if(CosFunction.bLastSetInSetUpFile)                                         //Steven 20111019
    {
        //Ifor 20161229add 開啟Last Set In SetUp File 功能時,Setup File 中無TestMode資料導致異常問題
        //==>
        AnsiString szDir=GetRecipeFileName("TestMode.Data");
        if(FileExists(szDir)==true)
        {
            if(CosFunction.bProgramStartOnLine &&
               SystemInitialOK==false &&
               LastSet.iTester!=ON_LINE &&
               LastSet.iRealDummy!=REALLY)                                      //Sam 20210423 : 程式開啟時切換為 OnLine/Real
            {
                LastSet.iTester=ON_LINE;
                LastSet.iRealDummy=REALLY;
            }

            LastSetUseTestSocketToTestModeDutOnOff();                           //Steven 20231018 : fixed for bLastSetInSetUpFile
            if(bLoadMachineRecord)                                              //JerryYang 20160614 用來判斷是否執行過LoadMachineRecord函式 避免還沒讀取到機台資料Lastset就被改掉
            {
                if(LastSet.iTester!=TestMode.iTestConnection)                   //Steven 20211103 : 新增Log,避免讀檔改狀態沒有紀錄
                {
                    if(TestMode.iTestConnection==OFF_LINE)                      //Steven 20140815
                    {
                        if(IniConfig.bI27_ManualSortMode &&
                           bRunManualSortMode==true)                            //Steven 20150915 : For TSMC 手動整盤功能
                            NewRecordProcess("MES2145", "XXXX  Tester MANUAL MODE  XXXX");
                        else
                            NewRecordProcess("MES2146", "XXXX  Tester OFF-Line  XXXX");
                    }
                    else
                    {
                        NewRecordProcess("MES2147", "VVVV  Tester ON-Line  VVVV");
                    }
                }

                if(LastSet.iRealDummy!=TestMode.iRunMode)
                {
                    if(TestMode.iRunMode==REALLY)                               //Steven 20140815
                        NewRecordProcess("MES2148", "VVVV  Run Mode : REALLY  VVVV");
                    else if(TestMode.iRunMode==DUMMY)
                        NewRecordProcess("MES2149", "XXXX  Run Mode : DUMMY  XXXX");
                    else
                        NewRecordProcess("MES2150", "XXXX  Run Mode : TRAY ONLY  XXXX");
                }

                if(LastSet.iTemperature!=TestMode.iTemperatureMode)
                {
                    if(TestMode.iTemperatureMode==Tempture_Hot)
                    {
                        NewRecordProcess("MES2151", "Change Hot Mode");
                    }
                    else if(TestMode.iTemperatureMode==Tempture_AmbientHot)     //kevin 20140918 常溫加熱 恆溫控制
                    {
                        NewRecordProcess("MES2152", "Change Ambient Hot Mode");
                    }
                    else
                    {
                        NewRecordProcess("MES2153", "Change Ambient Mode");
                    }
                }

//                bLoadMachineRecord=false;   //jou 20170926    //Steven 20200703 : mark, 改成false會導致後面都不更新
                LastSet.iTemperature    =TestMode.iTemperatureMode;
                LastSet.iRealDummy      =TestMode.iRunMode;
                LastSet.iTester         =TestMode.iTestConnection;
            }
            TestModeDutOnOffToLastSetUseTestSocket();                           //Steven 20231018 : fixed for bLastSetInSetUpFile
        }
        //<==
        //Ifor 20161229 (Steven) add 開啟Last Set In SetUp File 功能時,Setup File 中無TestMode資料導致異常問題
    }

    if(LastSet.iRunStartMode==rsmQAMode && bQAModeFinishCleanOut==true)         //Steven 20141023 : QA做完後的Bin
    {
        if(fMain!=NULL &&
           CUSTOMER_CODE!=CC_KYEC_LEE)                                          //Ifor 20201027 add:KYEC QA Mode 不切Offline 模式
        {
            fMain->ModifyTester(OFF_LINE);                                      //Steven 20191218 : 整合修改LastSet.iTester
        }
    }

    if(LastSet.__bBinData32==false)                                             //Steven 20121112 : RS232支援32Bin 用來將舊的資料轉到新的
    {
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<=15; j++)
            {
                LastSet.iBinData32[i][j]=LastSet.iBinData[i][j];
            }
        }
        LastSet.__bBinData32=true;                                              //Steven 20140403 : 256Bin 用來將舊的資料轉到新的
        LastSet.bBinData32=true;                                                //Steven 20140403 : 256Bin 用來將舊的資料轉到新的
    }

    if(LastSet.bBinData32==false)                                               //Steven 20140403 : 256Bin 用來將舊的資料轉到新的
    {
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<100; j++)                                            //JimmyChiu 20230307 : <= --> <
            {
                LastSet.iBinData32[i][j]=LastSet.__iBinData32[i][j];
            }
        }
        LastSet.__bBinData32=true;
        LastSet.bBinData32=true;
    }

    if(CUSTOMER_CODE==CC_SCS ||                                                 //Steven 20130610 : SCS要求強制開啟
       CUSTOMER_CODE==CC_ASE_M)                                                 //Ifor 20190926 : add ASEM強制開啟[A03]
    {
        IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin=true;
    }

    #ifdef SOFT_SIMULTE
    for(int i=0; i<TOTAL_MOTOR; i++)
        LastSet.SoftSpeed[i]=(LastSet.SoftSpeed[i]<=1000)?10000:LastSet.SoftSpeed[i];                                   //Steven 20161211 : provide initial soft speed as 10000
    #endif

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {                                                                           //jou 2014-03-28 SPIL Handler  On-line & Offline Switch Flow
        LastSet.MusicSelect[6]=3;
        LastSet.MessageLight[6][0]=0;
        LastSet.MessageLight[6][1]=0;
        LastSet.MessageLight[6][2]=2;
    }

   
    bool bflag=false;                                                           //ChungHung 20140401 add fix LastSet 開關Site 照成的HangUp 因為同時改到LastSet所照成
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            for(int k=0; k<8; k++)
            {
                LastSet.bUseTestSocket[i][j][k]=LastSet.bUseTestSocket[i][j][k]!=0?true:false;
                bflag=LastSet.bUseTestSocket[i][j][k] | bflag;
            }
        }
    }

    if(bflag==false)
    {
        for(int j=0; j<4; j++)
        {
            for(int k=0; k<8; k++)
            {
                LastSet.bUseTestSocket[0][j][k]=LastSet.bUseTestSocket__[j][k];
                LastSet.bUseTestSocket[1][j][k]=LastSet.bUseTestSocket__[j][k];
                LastSet.bUseTestSocket__[j][k]=false;                           //jou 2015-06-02 LastSet.bUseTestSocket__[j][k]==false => =false
            }
        }

        if(fMain!=NULL)
            fMain->ShowTestHeadComp(false);                                     //jou 2015-06-02 有修改開關Site需要重繪畫面
    }
    LastSet.bD41TestSocketICCheckSkip=false;                                    //Steven 20160328 : [D41]強制false, 就是不能跳過index check
    if(Tri_Temp_Machine==1)                                                     //Ztex 2024.02.19 Add Temp Pos Shift For Shuttle
    {
        LastSet.TempPosShift_Shuttle[0][0]=15;
        LastSet.TempPosShift_Shuttle[0][1]=-40;
        LastSet.TempPosShift_Shuttle[1][0]=15;
        LastSet.TempPosShift_Shuttle[1][1]=-40;
    }
    else if(FIX3_FULL_PLACE==Fix3K_ShortShuttle)                                //wei 20160328 修改一開始就讀取資料
    {
        LastSet.TempPosShift_Shuttle[0][0]=15;
        LastSet.TempPosShift_Shuttle[0][1]=90;
        LastSet.TempPosShift_Shuttle[1][0]=15;
        LastSet.TempPosShift_Shuttle[1][1]=90;
    }
    else if(ATC_SYSTEM>eATCUninstall)
    {
        LastSet.TempPosShift_Shuttle[0][0]=15;
        LastSet.TempPosShift_Shuttle[0][1]=60;
        LastSet.TempPosShift_Shuttle[1][0]=15;
        LastSet.TempPosShift_Shuttle[1][1]=60;
    }
    else if(MachineTypeChoice==Type_HT9046_LS && SubMachineType==Type_HT9016C)  //Jimmychiu 20230207 Add TempPosShift for HT9016C
    {
        LastSet.TempPosShift_Shuttle[0][0]=29;
        LastSet.TempPosShift_Shuttle[0][1]=9;
        LastSet.TempPosShift_Shuttle[1][0]=29;
        LastSet.TempPosShift_Shuttle[1][1]=9;
    }
    else
    {
        LastSet.TempPosShift_Shuttle[0][0]=40;
        LastSet.TempPosShift_Shuttle[0][1]=130;
        LastSet.TempPosShift_Shuttle[1][0]=40;
        LastSet.TempPosShift_Shuttle[1][1]=130;
    }

    if(CosFunction.bContactCountSavedByRecipe)                                  //Jimmychiu 20240826 : Contact count saved by recipe
    {
        AnsiString S=GetLastOpenFN();
        AnsiString szDir=AnsiString().sprintf("%s%s\\TestMode.Data", DataPath, S);
        for(int i=0; i<2; i++)                                                  //jou 2012-06-05
        {
            LastSet.ContactSet[i]    =ReadIniData(szDir, "TestMode", AnsiString().sprintf("ContactSet_%d",i), 0);
            LastSet.iContactCT[i]    =ReadIniData(szDir, "TestMode", AnsiString().sprintf("ContactCT_%d",i), 0);
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20210112 add: KYEC 要求Start Mode 資料寫死不可修改
    {
        LastSet.iStartMode=2;
        LastSet.bCTClear[0][0]=true;
        LastSet.bCTClear[0][1]=true;
        LastSet.bCTClear[0][2]=true;
        LastSet.bCTClear[0][3]=true;
        LastSet.bCTClear[0][4]=true;
        LastSet.bCTClear[0][5]=false;
        LastSet.bCTClear[0][6]=true;
    }

    LastSet.bUseNewTTLBoard=true;
    return true;
}
//---------------------------------------------------------------------------
bool WriteLastDataFile(bool BackUp2, bool bNotContact)
{
    bool flag=true;

    LastSet.bBinData32=true;                                                    //Steven 20121112 : RS232支援32Bin 用來將舊的資料轉到新的

    DWORD wtfz;
    HANDLE Fp;

    //ChungHung 20120829 add
    if(BackUp2)
    {
        Fp=CreateFile(W906_LastDataPath("D:\\HT9045\\system\\lastdata_backup2.dat").c_str(), FILE_SHARE_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
        if(Fp!=INVALID_HANDLE_VALUE)
        {
            WriteFile(Fp, (char *)&LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET), &wtfz, NULL);
        }
        CloseHandle(Fp);
    }
    //ChungHung 20120829 end

    if(FileExists(W906_LastDataPath("D:\\HT9045\\system\\lastdata.dat")))   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    {
        Fp=CreateFile(W906_LastDataPath("D:\\HT9045\\system\\lastdata.dat").c_str(), FILE_SHARE_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    }
    else
    {
        Fp=CreateFile(W906_LastDataPath("D:\\HT9045\\system\\lastdata.dat").c_str(), FILE_SHARE_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    }
    try                                                                         //JerryYang 20220923 : add 例外處理
    {
        if(Fp!=INVALID_HANDLE_VALUE)
        {
            WriteFile(Fp, (char *)&LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET), &wtfz, NULL);
        }
        else
        {
            ShowErrorMessage("WAR1682", 0, MMSystem, 0, AnsiString("lastdata.dat"));
            flag=false;
        }
        CloseHandle(Fp);
    }
    catch(...)
    {
        RecordProcess("Exception of WriteFile() lastdata.dat");
        CloseHandle(Fp);
    }

    if(FileExists(W906_LastDataPath("D:\\HT9045\\system\\lastdata_backup.dat")))   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    {
        Fp=CreateFile(W906_LastDataPath("D:\\HT9045\\system\\lastdata_backup.dat").c_str(), FILE_SHARE_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    }
    else
    {
        Fp=CreateFile(W906_LastDataPath("D:\\HT9045\\system\\lastdata_backup.dat").c_str(), FILE_SHARE_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);   //AI(W906-LASTDATA-TESTDIR) 20260926: 路徑鉤子（本檔尾）
    }

    if(Fp!=INVALID_HANDLE_VALUE)
    {
        WriteFile(Fp, (char *)&LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET), &wtfz, NULL);
    }
    else
    {
        ShowErrorMessage("WAR1682", 0, MMSystem, 0, AnsiString("lastdata_backup.dat"));
        flag=false;
    }
    CloseHandle(Fp);

    AnsiString str1;
    AnsiString str2;
    AnsiString str3;
    AnsiString sPath=W906_LastDataPath((AuthPath+"config.ini").c_str());   //AI(W906-LASTDATA-TESTDIR) 20260926: 本函式一定會寫 config.ini（Vibrate_Time、P65_QAMode、SocketContact、O_Count），ctest 一樣轉進沙盒（本檔尾）

    if(bNotContact==false)
    {
        AnsiString sRecipePath="";                                              //AI(ht9045-config) 20260508 (RogerYang) : VTEST銦片資料跟隨Recipe
        if(IniConfig.bVTESTFunction && CosFunction.bUseHeadContactCount)
            sRecipePath=GetRecipeFileName("HandlerCondition.Data");

        //Steven 20260925：golden 912 cprod.cpp:1989-2017 —— O_Count 落檔改 by-arm（iArm<2 × iPos<16），陣列索引 [x][iArm][iPos]；
        //  906 寫法照 FTestSuck 列×欄存 [x][i][j]，2x4 機台 pos 4-7 在 Data.StartCondition 改了、存了，重開就不見。
        for(int iArm=0; iArm<2; iArm++)                                         //AI(ht9045-config) 20260728 (RogerYang) : 全客戶統一by-arm;iPos範圍與ProcessLastSetIni_Count一致,避免讀16寫8的不對稱
        {
            for(int iPos=0; iPos<16; iPos++)
            {
                str1.sprintf("ContactSet%d_%d", iArm, iPos);
                str2.sprintf("HeadContactCount%d_%d", iArm, iPos);
                str3.sprintf("HeadContactCountHistory%d_%d", iArm, iPos);

                //Steven 20211215 : 改成不存Change Log
                if(sRecipePath!="")                                             //AI(ht9045-config) 20260508 (RogerYang) : VTEST x=0銦片寫入Recipe
                {
                    WriteIniDataNoLog(sRecipePath, "O_Count", "O_14"+AnsiString(str1), IniConfig.ContactSet[0][iArm][iPos]              );
                    WriteIniDataNoLog(sRecipePath, "O_Count", "O_14"+AnsiString(str2), IniConfig.HeadContactCount[0][iArm][iPos]        );
                    WriteIniDataNoLog(sRecipePath, "O_Count", "O_14"+AnsiString(str3), IniConfig.HeadContactCountHistory[0][iArm][iPos] );
                }
                else
                {
                    WriteIniDataNoLog(sPath, "O_Count", "O_14"+AnsiString(str1), IniConfig.ContactSet[0][iArm][iPos]              );
                    WriteIniDataNoLog(sPath, "O_Count", "O_14"+AnsiString(str2), IniConfig.HeadContactCount[0][iArm][iPos]        );
                    WriteIniDataNoLog(sPath, "O_Count", "O_14"+AnsiString(str3), IniConfig.HeadContactCountHistory[0][iArm][iPos] );
                }
                WriteIniDataNoLog(sPath, "O_Count", "O_15"+AnsiString(str1), IniConfig.ContactSet[1][iArm][iPos]              );
                WriteIniDataNoLog(sPath, "O_Count", "O_15"+AnsiString(str2), IniConfig.HeadContactCount[1][iArm][iPos]        );
                WriteIniDataNoLog(sPath, "O_Count", "O_15"+AnsiString(str3), IniConfig.HeadContactCountHistory[1][iArm][iPos] );
                WriteIniDataNoLog(sPath, "O_Count", "O_16"+AnsiString(str1), IniConfig.ContactSet[2][iArm][iPos]              );
                WriteIniDataNoLog(sPath, "O_Count", "O_16"+AnsiString(str2), IniConfig.HeadContactCount[2][iArm][iPos]        );
                WriteIniDataNoLog(sPath, "O_Count", "O_16"+AnsiString(str3), IniConfig.HeadContactCountHistory[2][iArm][iPos] );
            }
        }
    }

    if(IniConfig.iVibratorHP1>100000000)                                        //JerryYang 20200612 振動馬達作動時間累計
    {
        IniConfig.iVibratorHP1=0;
        RecordProcess("Clear HP vibrator time");
    }

    if(IniConfig.iVibratorSht1>100000000)
    {
        IniConfig.iVibratorSht1=0;
        RecordProcess("Clear Shuttle1 vibrator time");
    }

    if(IniConfig.iVibratorSht2>100000000)
    {
        IniConfig.iVibratorSht2=0;
        RecordProcess("Clear Shuttle2 vibrator time");
    }

    if(IniConfig.iVibratorUnloader>100000000)
    {
        IniConfig.iVibratorUnloader=0;
        RecordProcess("Clear Unloader vibrator time");
    }

    //Steven 20211215 : 改成不存Change Log
    WriteIniDataNoLog(sPath, "Vibrate_Time", "iVibratorHP1",        IniConfig.iVibratorHP1     );
    WriteIniDataNoLog(sPath, "Vibrate_Time", "iVibratorSht1",       IniConfig.iVibratorSht1    );
    WriteIniDataNoLog(sPath, "Vibrate_Time", "iVibratorSht2",       IniConfig.iVibratorSht2    );
    WriteIniDataNoLog(sPath, "Vibrate_Time", "iVibratorUnloader",   IniConfig.iVibratorUnloader);

    //Ifor 20260424: [P65] save QA retest count for persistence across program restart
    WriteIniDataNoLog(sPath, "P65_QAMode", "iP65QAReTestCount",     iP65QAReTestCount          );    //Steven 20260925：golden 912 cprod.cpp:2050-2051

    if(CosFunction.bUseSocketContactCount)                                      //Sam 20220720 : 新增一組 Socket Count
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                str1.sprintf("SocketContactSet%d", 1+i*8+j);
                str2.sprintf("SocketContactCount%d", 1+i*8+j);
                str3.sprintf("SocketContactCountHistory%d", 1+i*8+j);

                IniConfig.SocketContactSet[i][j]           = ReadWriteIni(sPath, "SocketContact", AnsiString(str1), IniConfig.SocketContactSet[i][j], 10, false);
                IniConfig.SocketContactCount[i][j]         = ReadWriteIni(sPath, "SocketContact", AnsiString(str2), IniConfig.SocketContactCount[i][j], 0, false);
                IniConfig.SocketContactCountHistory[i][j]  = ReadWriteIni(sPath, "SocketContact", AnsiString(str3), IniConfig.SocketContactCountHistory[i][j], 0, false);
            }
        }
    }

    if(CosFunction.bContactCountSavedByRecipe)                                  //Jimmychiu 20240826 : Contact count saved by recipe
    {
        AnsiString S=GetLastOpenFN();
        AnsiString szDir=AnsiString().sprintf("%s%s\\TestMode.Data", DataPath, S);
        for(int i=0; i<2; i++)                                                  //jou 2012-06-05
        {
            WriteIniData(szDir, "TestMode", AnsiString().sprintf("ContactSet_%d",i),   LastSet.ContactSet[i]);
            WriteIniData(szDir, "TestMode", AnsiString().sprintf("ContactCT_%d",i),    LastSet.iContactCT[i]);
        }
    }
    return flag;
}
//---------------------------------------------------------------------------
//  以下是Steven將原本的 lastdata.dat改成 LastSet.ini 20100811
//---------------------------------------------------------------------------
void ProcessLastSetIni_RMS(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";
    AnsiString str, Str1, Backup;
    int iLength;
    str=(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)?"RMS":"Server";

    if(bRead)
    {
        if(IniConfig.bShowLotInfo)
        {
            if(CUSTOMER_CODE==CC_KYEC_LEE)                                      //Ifor 20161229 (Steven) dd KYEC 喬智要求 關閉N05功能選項
            {
                IniConfig.bEnableRms  = false;                                  //Ifor 20161229 KYEC 喬智要求強制關閉 Enable RMS 功能
                IniConfig.bCheckFile  = false;                                  //Ifor 20161229 KYEC 喬智要求強制關閉 Enable check file 功能
                IniConfig.bEnableRmsCheckSetupFile=false;                       //Ifor 20230516 add: TFAMD 要求工作檔驗證
            }
            else
            {
                IniConfig.bEnableRms  =CheckAndReadIniData(sPath, str, str+" Enable",       false);
                IniConfig.bCheckFile  =CheckAndReadIniData(sPath, str, "Enable Check File", false);                     //Steven 20101208
                IniConfig.bEnableRmsCheckSetupFile  =CheckAndReadIniData(sPath, str, "Enable Check Setup File", false);                                         //Ifor 20230516 add: TFAMD 要求工作檔驗證
            }

            IniConfig.sRmsPath    =CheckAndReadIniData(sPath, str, str+" Path",         AnsiString("D:\\RMS"));
            IniConfig.fAmbientTemp=CheckAndReadIniData(sPath, str, "Ambient Temp",      40.0);                          //Steven 20101208
            if(CosFunction.bUseERMS)
            {
                IniConfig.bEnableErms =CheckAndReadIniData(sPath, str, "ERMS Enable",   false);                         //Steven 20160711 : 使用進階版RMS
                IniConfig.sErmsPath   =CheckAndReadIniData(sPath, str, "ERMS Path",     AnsiString("D:\\RMS"));
                if(IniConfig.bEnableErms)
                    IniConfig.bEnableRms=false;
            }
            else
            {
                IniConfig.bEnableErms=false;
            }

            iLength=IniConfig.sRmsPath.Length();

            if(IniConfig.sRmsPath.SubString(iLength, 1) == "\\")
            {
                IniConfig.sRmsPath=IniConfig.sRmsPath.SubString(1, iLength-1);
            }

            if(CosFunction.bRTCalarmUnload)                                     //jou 20170210 (Steven) : RTC alarm image unload
            {
                IniConfig.bN05_RTCalarmUnload   =CheckAndReadIniData(sPath, str, "RTC alarm unload Enable",       false);
                IniConfig.asN05_RTCalarmUnload  =CheckAndReadIniData(sPath, str, "RTC alarm unload Path",         AnsiString("D:\\RTC"));
            }
            else
            {
                IniConfig.bN05_RTCalarmUnload=false;
            }

            if(CUSTOMER_CODE==CC_SCC ||
               CUSTOMER_CODE==CC_SCK ||
               CUSTOMER_CODE==CC_ETRENDTECH ||                                  //Steven 20230302 : add for 逸昌
               CUSTOMER_CODE==CC_AMD_M ||                                       //Ifor 20200915 add: TF_AMD download recipe
               CosFunction.bDownloadRecipeLevelMode)                            //jou 2016-01-06 download recipe 增加權限模式選擇
            {
                IniConfig.sRmsDownPath=CheckAndReadIniData(sPath, str, str+" Download Path", AnsiString("D:\\HT9045"));
                iLength=IniConfig.sRmsDownPath.Length();
                if(IniConfig.sRmsDownPath.SubString(iLength, 1) == "\\")
                {
                    IniConfig.sRmsDownPath=IniConfig.sRmsDownPath.SubString(1, iLength-1);
                }
            }
            else if(IniConfig.bSPILFunction==true)                              //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
            {
                if(IniConfig.bEnableRms)
                {
                    Backup=DataPath;
                    DataPath="D:\\HT9045\\IniData\\Data\\Active\\";
                    if(DirectoryExists(DataPath)==false)
                    {
                        SetCurrentDirectory("D://");   //AI(GA1-B2) 20260804: _T() neutralized (ANSI build, _T(x)==x), same convention as Automation/auto9045.cpp:2233/:2307
                        MyForceDirectories(DataPath);
                        Str1.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", Backup+GetLastOpenFN(), DataPath+GetLastOpenFN());
                        system(Str1.c_str());
                        MySleep(50);
                    }

#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GA1-B2 reason expired -- HTGem is complete through SECSGEM/uHGemClass.h (included above); HTGem::UpdateDataPath is the golden one-liner (SECSGEM/uHGemClass.cpp); golden cprod.cpp:2165
      // only in SECSGEM/uHGemClass.h, which ht9045_globals cannot include without creating
      // the same CMake target cycle root CMakeLists.txt:470 already documents avoiding)
                    if(HSys.MyGem!=NULL)
                        HSys.MyGem->UpdateDataPath(DataPath);                   //Steven 20140902 : SECS GEM
#endif // TODO(GA1-B2): HTGem
                }
            }

            if(IniConfig.bVTESTFunction==true)                                  //jou 20200409 : VTest Mes system
            {
                IniConfig.asMesSyURLPath    =CheckAndReadIniData(sPath, str, "asMesSyURLPath",      AnsiString("http://www.honprec.com/TEIP/AjaxProcess.aspx"));
                IniConfig.asMesSyACodePath  =CheckAndReadIniData(sPath, str, "asMesSyACodePath",    AnsiString("123"));
                IniConfig.asMesSyActionPath =CheckAndReadIniData(sPath, str, "asMesSyActionPath",   AnsiString("456"));

//                #ifdef BETA_VTestSummaryFile                                            //RogerYang 20250809 偉測Summary文件修改
                IniConfig.asSummaryReportPath =AnsiString("D:\\HandlerSummary");                                        //RogerYang 20251011 : 取消空格
//                #else
//                    IniConfig.asSummaryReportPath =CheckAndReadIniData(sPath, str, "asSummaryReportPath",  IniConfig.sN10UploadDrivePath );
//                #endif

                IniConfig.asByTimeOEEPath     =CheckAndReadIniData(sPath, str, "asByTimeOEEPath"    ,  AnsiString("D:\\RMS") );
                IniConfig.asUPHReportPath     =CheckAndReadIniData(sPath, str, "asUPHReportPath"    ,  AnsiString("D:\\RMS") );

                //jou 20221104 : VTest CreateManualEOCAP function;
//                IniConfig.bCreateManualEOCAP      =CheckAndReadIniData(sPath, str, "bCreateManualEOCAP", false);
                IniConfig.asCreateManualEOCAP_URL =CheckAndReadIniData(sPath, str, "asCreateManualEOCAP_URL", AnsiString("http://192.168.10.216/vt_mes/MesWebService.asmx"));
                IniConfig.asQueryEocapStatusURL   =CheckAndReadIniData(sPath, str, "asQueryEocapStatusURL", AnsiString("http://192.168.115.133:9014/api/mes/queryEocapStatus"));

                //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
                IniConfig.asGetRcsCheckingResultUrl      =CheckAndReadIniData(sPath, str, "asGetRcsCheckingResultUrl",     AnsiString("http://192.168.10.216/vt_mes/ajaxprocess.aspx?"));
                IniConfig.asGetRcsCheckingResultACode    =CheckAndReadIniData(sPath, str, "asGetRcsCheckingResultACode",   AnsiString("65195845153489435181"));
                IniConfig.asGetRcsCheckingResultAction   =CheckAndReadIniData(sPath, str, "asGetRcsCheckingResultAction",  AnsiString("GetRcsCheckingResult"));
                IniConfig.bGetRcsCheckingResult           =CheckAndReadIniData(sPath, str, "bGetRcsCheckingResult",    false);
            }
        }
    }
    else
    {
        if(IniConfig.bShowLotInfo)
        {
            if(CosFunction.bUseERMS)
            {
                WriteIniData(sPath, str, "ERMS Enable",       IniConfig.bEnableErms);                                   //Steven 20160711 : 使用進階版RMS
                WriteIniData(sPath, str, "ERMS Path",         IniConfig.sErmsPath);
                if(IniConfig.bEnableErms)
                    IniConfig.bEnableRms=false;
            }

            if(CosFunction.bRTCalarmUnload)                                     //jou 20170210 (Steven) : RTC alarm image unload
            {
                WriteIniData(sPath, str, "RTC alarm unload Enable", IniConfig.bN05_RTCalarmUnload);
                WriteIniData(sPath, str, "RTC alarm unload Path",   IniConfig.asN05_RTCalarmUnload);
            }

            if(IniConfig.sRmsPath=="")                                          //Steven 20110528 : 預防路徑消失
                IniConfig.sRmsPath=CheckAndReadIniData(sPath, str, str+" Path", AnsiString("D:\\HT9045"));
            else
                WriteIniData(sPath, str, str+" Path", IniConfig.sRmsPath);

            if(CUSTOMER_CODE==CC_SCC ||
               CUSTOMER_CODE==CC_SCK ||
               CUSTOMER_CODE==CC_ETRENDTECH ||                                  //Steven 20230302 : add for 逸昌
               CUSTOMER_CODE==CC_AMD_M ||                                       //Ifor 20200915 add: TF_AMD download recipe
               CosFunction.bDownloadRecipeLevelMode)                            //jou 2016-01-06 download recipe 增加權限模式選擇
            {
                if(IniConfig.sRmsDownPath=="")                                  //Steven 20110528 : 預防路徑消失
                    IniConfig.sRmsDownPath=CheckAndReadIniData(sPath, str, str+" Download Path", AnsiString("D:\\HT9045"));
                else
                    WriteIniData(sPath, str, str+" Download Path", IniConfig.sRmsDownPath);
            }

            WriteIniData(sPath, str, "Ambient Temp", IniConfig.fAmbientTemp);   //Steven 20110421

            if(CUSTOMER_CODE==CC_KYEC_LEE)                                      //Ifor 20161229 (Steven) add KYEC 喬智要求 關閉N05功能選項
            {
                WriteIniData(sPath, str, str+" Enable"      , false);           //Ifor 20161229 KYEC 喬智要求強制關閉 Enable RMS 功能
                WriteIniData(sPath, str, "Enable Check File", false);           //Ifor 20161229 KYEC 喬智要求強制關閉 Enable check file 功能
                WriteIniData(sPath, str, "Enable Check Setup File", false);
            }
            else
            {
                WriteIniData(sPath, str, str+" Enable"      , IniConfig.bEnableRms);
                WriteIniData(sPath, str, "Enable Check File", IniConfig.bCheckFile);                                    //Steven 20110421
                WriteIniData(sPath, str, "Enable Check Setup File", IniConfig.bEnableRmsCheckSetupFile);                //Steven 20110421
            }
        }
    }

    if(IniConfig.bSPILFunction==true ||
       CUSTOMER_CODE==CC_HANA_MICRON ||
       CUSTOMER_CODE==CC_SCK)
        IniConfig.bClearLotInfoWhenTrayFeed=CheckAndReadIniData(sPath, str, "ClearLotInfoWhenTrayFeed", false);         //Steven 20240916 : Tray Feed之後, 要不要清除Device Name
    else
        IniConfig.bClearLotInfoWhenTrayFeed=CheckAndReadIniData(sPath, str, "ClearLotInfoWhenTrayFeed", true);
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_FTP(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";
    AnsiString str, Backup;
    if(CosFunction.bFTPFunction || IniConfig.bFTPJamCodeUpload)
    {
        IniConfig.bEnableFTP        =ReadWriteIni(sPath, "FTP", "Enable FTP",        IniConfig.bEnableFTP,      false,                                              bRead);
        if(CUSTOMER_CODE==CC_KYEC_LEE || IniConfig.bSPILFunction==true)
        {
            IniConfig.FtpUserName       =ReadWriteIni(sPath, "FTP", "FTP User Name",     IniConfig.FtpUserName,     (IniConfig.bFTPJamCodeUpload)?"SPIL":"KYEC",    bRead);
        }
        else
        {
            IniConfig.FtpUserName       =ReadWriteIni(sPath, "FTP", "FTP User Name",     IniConfig.FtpUserName,     "",                                             bRead);
        }

        IniConfig.FtpPassword       =ReadWriteIni(sPath, "FTP", "FTP Password",      IniConfig.FtpPassword,     "1234",                                             bRead);
        IniConfig.FtpHost           =ReadWriteIni(sPath, "FTP", "FTP Host",          IniConfig.FtpHost,         "127.0.0.1",                                        bRead);
        IniConfig.FtpDownloadPath   =ReadWriteIni(sPath, "FTP", "FTP Download Path", IniConfig.FtpDownloadPath, "/",                                                bRead);
        IniConfig.FtpUplaodPath     =ReadWriteIni(sPath, "FTP", "FTP Upload Path",   IniConfig.FtpUplaodPath,   "/",                                                bRead);
        IniConfig.iHDEnable         =ReadWriteIni(sPath, "FTP", "FTP HD Enable",     IniConfig.iHDEnable,       1,                                                  bRead);
        IniConfig.iServerEnable     =ReadWriteIni(sPath, "FTP", "FTP Server Enable", IniConfig.iServerEnable,   0,                                                  bRead);
        IniConfig.N06_FtpPort       =ReadWriteIni(sPath, "FTP", "FTP Port",          IniConfig.N06_FtpPort,     "21",                                               bRead);  //Ifor 20201015 add:使用者自定義 FTP Port

        if(IniConfig.bVTESTFunction==true)
            IniConfig.FtpTransMode      =ReadWriteIni(sPath, "FTP", "FTP Transfer Mode", IniConfig.FtpTransMode,    2,                                              bRead);  //Steven 20230719 : 加入FTP傳輸模式
        else
            IniConfig.FtpTransMode      =ReadWriteIni(sPath, "FTP", "FTP Transfer Mode", IniConfig.FtpTransMode,    0,                                              bRead);  //Steven 20230719 : 加入FTP傳輸模式

        //Landam 20110704
        if(IniConfig.FtpDownloadPath!="/")
        {
            if(IniConfig.FtpDownloadPath.SubString(IniConfig.FtpDownloadPath.Length(), 1)!="/")
               IniConfig.FtpDownloadPath=IniConfig.FtpDownloadPath + "/";
        }

        if(IniConfig.FtpUplaodPath!="/")
        {
            if(IniConfig.FtpUplaodPath.SubString(IniConfig.FtpUplaodPath.Length(), 1)!="/")
                IniConfig.FtpUplaodPath=IniConfig.FtpUplaodPath + "/";
        }

        //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
        IniConfig.bFtpPasswordDownload      =ReadWriteIni(sPath, "FTP", "Enable FTP Password Download",  IniConfig.bFtpPasswordDownload,    false,  bRead);
        IniConfig.FtpPasswordDownloadPath   =ReadWriteIni(sPath, "FTP", "FTP Password Download Path",    IniConfig.FtpPasswordDownloadPath, "/",    bRead);
        if(IniConfig.FtpPasswordDownloadPath!="/")
        {
            if(IniConfig.FtpPasswordDownloadPath.SubString(IniConfig.FtpPasswordDownloadPath.Length(), 1)!="/")
                IniConfig.FtpPasswordDownloadPath=IniConfig.FtpPasswordDownloadPath + "/";
        }
    }

#if 0 // TODO(GA1-B2): blocked by TfLotInfo@forms/fLotInfo.h missing members tsASECLEventLog/
      // ts2DSort/grpManualSet2D/grp2DLotInfo (golden uLotInfo.h TTabSheet/TGroupBox controls,
      // not present on the ported facade)
    if(fLotInfo!=NULL)                                                          //Steven 20181224 : For ASE-CL
    {
        fLotInfo->tsASECLEventLog->TabVisible=IniConfig.bN22Enable_EventLog;
        fLotInfo->ts2DSort->TabVisible=(CosFunction.bSortingBy2DList &&         //JerryYang 20230322 : 2D sort lot info UI修改
                                        (IniConfig.iN23DownloadMethod==3 ||
                                         IniConfig.bN23UseLotInfoFile==true));  //Steven 20240830 : 2D sort 手動選檔案方式

        fLotInfo->grpManualSet2D->Visible=(IniConfig.iN23DownloadMethod==3);
        fLotInfo->grp2DLotInfo->Visible=(IniConfig.bN23UseLotInfoFile==true);
    }
#endif // TODO(GA1-B2): TfLotInfo tsASECLEventLog/ts2DSort/grpManualSet2D/grp2DLotInfo

#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GA1-B2 reason expired -- TfLotInfo has edtLine / edtProcessName / edtProduct (forms/fLotInfo.h); golden cprod.cpp:2321
      // edtProcessName/edtProduct (golden uLotInfo.h TEdit controls, Murata 2DID compare
      // feature, not present on the ported facade)
    if(CUSTOMER_CODE==CC_Murata)                                                //Steven 20200409 : Murata 2DID比對功能
    {
        if(fLotInfo!=NULL)
        {
            fLotInfo->edtLine->Text=IniConfig.sN23_2_Line;
            fLotInfo->edtProcessName->Text=IniConfig.sN23_2_Process;
            fLotInfo->edtProduct->Text=IniConfig.sN23_2_Product;
        }
    }
#endif // TODO(GA1-B2): TfLotInfo edtLine/edtProcessName/edtProduct

//    IniConfig.sN23_4_URL        =ReadWriteIni(sPath, "2DID Search Function", "sN23_4_URL",              IniConfig.sN23_4_URL,               AnsiString("D:\\RMS\\"), bRead);  //JerryYang 20241104 : 支援2DID白名單功能  //JerryYang 20250521 : Mark掉
//    IniConfig.sN23_5_UploadPath =ReadWriteIni(sPath, "2DID White list", "sN23_5_UploadPath",            IniConfig.sN23_5_UploadPath,        AnsiString("D:\\RMS\\"), bRead);
    if(bRead)
    {
        IniConfig.FtpUseSystemCallToUnZip=true;

        if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                       //ChungHung 20150413 add for TSMC   //ChungHung 20150415 add for TSMC
        {
            if(IniConfig.bEnableFTP)                                            //ChungHung 20150413 add for TSMC
            {
                Backup="D:\\HT9045\\IniData\\Data\\";
                DataPath="D:\\HT9045\\IniData\\DataFTP\\";
            }
            else
            {
                Backup="D:\\HT9045\\IniData\\DataFTP\\";
                DataPath="D:\\HT9045\\IniData\\Data\\";
            }

            if(DirectoryExists(DataPath)==false)
            {
                SetCurrentDirectory("D://");   //AI(GA1-B2) 20260804: _T() neutralized (ANSI build, _T(x)==x), same convention as Automation/auto9045.cpp:2233/:2307
                MyForceDirectories(DataPath);
                MySleep(50);
            }

            if(DirectoryExists(GetRecipePath())==false)
            {
                str.sprintf("XCOPY /y/a/e/c/i/h/f/r \"%s\" \"%s\"", Backup+GetLastOpenFN(), DataPath+GetLastOpenFN());
                system(str.c_str());
                MySleep(50);
            }

#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GA1-B2 reason expired -- HTGem is complete through SECSGEM/uHGemClass.h (included above); HTGem::UpdateDataPath is the golden one-liner (SECSGEM/uHGemClass.cpp); golden cprod.cpp:2364
      // only in SECSGEM/uHGemClass.h, which ht9045_globals cannot include without creating
      // the same CMake target cycle root CMakeLists.txt:470 already documents avoiding)
            if(HSys.MyGem!=NULL)
                HSys.MyGem->UpdateDataPath(DataPath);                           //Steven 20140902 : SECS GEM
#endif // TODO(GA1-B2): HTGem
        }
    }
    else
    {
        IniConfig.FtpUseSystemCallToUnZip=ReadWriteIni(sPath, "FTP", "FtpUseSystemCallToUnZip",   IniConfig.FtpUseSystemCallToUnZip, true, bRead);              //Steven 20140609
    }
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_EventLog(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini", str;
    int iLength;

    if(IniConfig.bEventLogAutoSaveFunction)
    {
        IniConfig.iNextEventLogRecordSpace=1;
        iLength=IniConfig.asEventLogAutoSavePath.Length();
        if(IniConfig.asEventLogAutoSavePath.SubString(iLength, 1)=="\\")
        {
            IniConfig.asEventLogAutoSavePath=IniConfig.asEventLogAutoSavePath.SubString(1, iLength-1);
        }

        for(int i=0; i<7; i++)
        {
            str.sprintf("bAutoSaveLogWeek[%d]", i);                             //Steven 20140902 : Fixed
            IniConfig.bAutoSaveLogWeek[i]=ReadWriteIni(sPath, "Event Log", str, IniConfig.bAutoSaveLogWeek[i], true, bRead);                                    //jou 2012-10-15 Auto Save Log 支援 Week 選擇
        }
        IniConfig.iO15_SaveFilePeriod = ReadWriteIni(sPath, "Event Log", "iO15_SaveFilePeriod", IniConfig.iO15_SaveFilePeriod, 0, bRead);                       //StevenHong 20260128 : 客戶TESNA把Eventlog設定成一個月
        IniConfig.bO10UseEventLogSaver=IniConfig.bO06_EventLogAutoSave;
    }

#if 0 // TODO(GA1-B2): blocked by TMyStringList@cmydef.h:15 (opaque forward declaration   [AI(W906-POOL2) 20261008: STALE -- TMyStringList is complete (Public/MyStringList.h) and slEventLog is created by LogObjects.cpp (W-150 LOG-SPLIT, St02-E 1007); left for that in-progress work]
      // tree-wide, zero members defined anywhere -- confirmed also by GA-1-B4's cMyDB.cpp
      // pass) -- slEventLog->FileName/SaveType/SaveSameFolder/SaveByLotID all need a
      // complete type. TByDay/TBy12Hour/TByHour/TBy2Hour/TBy4Hour/TBy6Hour/TBy8Hour/
      // TByMonth (the enum values assigned to ->SaveType) are likewise only meaningful
      // once that type lands.
    if(fMain!=NULL)
    {
        if(IniConfig.bN10_DailyUploadProdData)                                  //Steven 20180514 : JCET吳如春要求每日上傳Event Log, Jam統計表, MTBF, MUBF資料
            str.sprintf("%s_%s_EventLogTxt", IniConfig.sMachineType, IniConfig.SocketHandlerID);
        else if(IniConfig.bO15_EventLogFileNameWithMachineID)
            str.sprintf("EventLogTxt_%s", IniConfig.SocketHandlerID);
        else
            str.sprintf("EventLogTxt");
        slEventLog->FileName=str;

        if(IniConfig.bN10_DailyUploadProdData)
        {
            if(IniConfig.iN10UploadProductMethod==0)
            {
                slEventLog->SaveType=TByDay;
            }
            else
            {
                slEventLog->SaveType=TBy12Hour;
            }
        }
        else
        {
            if(IniConfig.iO15_SaveFilePeriod==0)
                slEventLog->SaveType=TByHour;
            else if(IniConfig.iO15_SaveFilePeriod==1)
                slEventLog->SaveType=TBy2Hour;
            else if(IniConfig.iO15_SaveFilePeriod==2)
                slEventLog->SaveType=TBy4Hour;
            else if(IniConfig.iO15_SaveFilePeriod==3)
                slEventLog->SaveType=TBy6Hour;
            else if(IniConfig.iO15_SaveFilePeriod==4)
                slEventLog->SaveType=TBy8Hour;
            else if(IniConfig.iO15_SaveFilePeriod==5)
                slEventLog->SaveType=TBy12Hour;
            else if(IniConfig.iO15_SaveFilePeriod==8)                           //StevenHong 20260128 : 客戶TESNA把Eventlog設定成一個月
                slEventLog->SaveType=TByMonth;
//            else if(IniConfig.iO15_SaveFilePeriod==7)                         //Steven 20250520 : 加入By Lot存檔
//                slEventLog->SaveType=TByLot;
            else                                                                //if(IniConfig.iO15_SaveFilePeriod==6)                         //Steven 20230705 : Event log不可以by月份存檔, 檔案太大會打不開
                slEventLog->SaveType=TByDay;
        }

        slEventLog->SaveSameFolder=IniConfig.bO15_EventLogSaveSameFolder;       //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾
        slEventLog->SaveByLotID=(CosFunction.bHiSiliconFunction ||
                                 CosFunction.bSaveEventLogByLotID ||            //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
                                 IniConfig.iO15_SaveFilePeriod==7);
    }
#endif // TODO(GA1-B2): TMyStringList (slEventLog)

    if(IniConfig.iO15_SaveFilePeriod==8)                                        //Stevenhong 20260318 : TESNA 把Eventlog report summarize by month
    {
        TAlarm1 helper;
        helper.SummarizeJAMreportbymonth();
    }
    //<==
    //Steven 20170829 (wei) : Event Log檔案存檔設定
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Count(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.sLotID                            =ReadWriteIni(sPath, "Count", "Lot ID",                          IniConfig.sLotID,                          "",    bRead);  //Steven 20140814 : Add for ASE_M

    AnsiString str1;
    AnsiString str2;
    AnsiString str3;

    AnsiString sRecipePath="";                                                  //AI(ht9045-config) 20260508 (RogerYang) : VTEST銦片資料跟隨Recipe
    if(IniConfig.bVTESTFunction && CosFunction.bUseHeadContactCount)
        sRecipePath=GetRecipeFileName("HandlerCondition.Data");

    //Steven 20260925：golden 912 cprod.cpp:2476-2505 —— O_Count 讀回改 by-arm（iArm<2 × iPos<16），陣列索引 [x][iArm][iPos]；
    //  906 寫法照 FTestSuck 列×欄讀 [x][i][j]，跟執行期 ProcessHeadContactCount（atester_ProcessCount.cpp:1317 iPos=j*2+i）
    //  的 by-arm 排列不一致，2x4 機台 pos 4-7 讀不回來。鍵名格式不變（O_1xContactSet<a>_<p>），既有鍵對應同一個陣列元素。
    for(int iArm=0; iArm<2; iArm++)                                             //AI(ht9045-config) 20260728 (RogerYang) : 全客戶統一by-arm;iPos涵蓋ContactSet dim3全16槽,含2x8每臂16個head
    {
        for(int iPos=0; iPos<16; iPos++)
        {
            str1.sprintf("ContactSet%d_%d", iArm, iPos);
            str2.sprintf("HeadContactCount%d_%d", iArm, iPos);
            str3.sprintf("HeadContactCountHistory%d_%d", iArm, iPos);

            if(sRecipePath!="")                                                 //AI(ht9045-config) 20260508 (RogerYang) : VTEST x=0銦片跟Recipe
            {
                IniConfig.ContactSet[0][iArm][iPos]               = ReadWriteIni(sRecipePath, "O_Count","O_14"+AnsiString(str1), IniConfig.ContactSet[0][iArm][iPos],              6000, bRead);
                IniConfig.HeadContactCount[0][iArm][iPos]         = ReadWriteIni(sRecipePath, "O_Count","O_14"+AnsiString(str2), IniConfig.HeadContactCount[0][iArm][iPos],           0, bRead);
                IniConfig.HeadContactCountHistory[0][iArm][iPos]  = ReadWriteIni(sRecipePath, "O_Count","O_14"+AnsiString(str3), IniConfig.HeadContactCountHistory[0][iArm][iPos],    0, bRead);
            }
            else
            {
                IniConfig.ContactSet[0][iArm][iPos]               = ReadWriteIni(sPath, "O_Count","O_14"+AnsiString(str1), IniConfig.ContactSet[0][iArm][iPos],              6000, bRead);
                IniConfig.HeadContactCount[0][iArm][iPos]         = ReadWriteIni(sPath, "O_Count","O_14"+AnsiString(str2), IniConfig.HeadContactCount[0][iArm][iPos],           0, bRead);
                IniConfig.HeadContactCountHistory[0][iArm][iPos]  = ReadWriteIni(sPath, "O_Count","O_14"+AnsiString(str3), IniConfig.HeadContactCountHistory[0][iArm][iPos],    0, bRead);
            }

            IniConfig.ContactSet[1][iArm][iPos]               = ReadWriteIni(sPath, "O_Count","O_15"+AnsiString(str1), IniConfig.ContactSet[1][iArm][iPos],              6000, bRead);
            IniConfig.HeadContactCount[1][iArm][iPos]         = ReadWriteIni(sPath, "O_Count","O_15"+AnsiString(str2), IniConfig.HeadContactCount[1][iArm][iPos],           0, bRead);
            IniConfig.HeadContactCountHistory[1][iArm][iPos]  = ReadWriteIni(sPath, "O_Count","O_15"+AnsiString(str3), IniConfig.HeadContactCountHistory[1][iArm][iPos],    0, bRead);

            IniConfig.ContactSet[2][iArm][iPos]               = ReadWriteIni(sPath, "O_Count","O_16"+AnsiString(str1), IniConfig.ContactSet[2][iArm][iPos],              6000, bRead);
            IniConfig.HeadContactCount[2][iArm][iPos]         = ReadWriteIni(sPath, "O_Count","O_16"+AnsiString(str2), IniConfig.HeadContactCount[2][iArm][iPos],           0, bRead);
            IniConfig.HeadContactCountHistory[2][iArm][iPos]  = ReadWriteIni(sPath, "O_Count","O_16"+AnsiString(str3), IniConfig.HeadContactCountHistory[2][iArm][iPos],    0, bRead);
        }
    }

    IniConfig.iVibratorHP1      =ReadWriteIni(sPath, "Vibrate_Time", "iVibratorHP1",        IniConfig.iVibratorHP1,      0, bRead);                             //JerryYang 20200612 振動馬達作動時間累計
    IniConfig.iVibratorSht1     =ReadWriteIni(sPath, "Vibrate_Time", "iVibratorSht1",       IniConfig.iVibratorSht1,     0, bRead);
    IniConfig.iVibratorSht2     =ReadWriteIni(sPath, "Vibrate_Time", "iVibratorSht2",       IniConfig.iVibratorSht2,     0, bRead);
    IniConfig.iVibratorUnloader =ReadWriteIni(sPath, "Vibrate_Time", "iVibratorUnloader",   IniConfig.iVibratorUnloader, 0, bRead);

    //Steven 20260925：golden 912 cprod.cpp:2512 —— 912 新鍵 [P65_QAMode] iP65QAReTestCount 讀回（寫入端在 WriteLastDataFile）。
    //  注意：移植樹 aTester_Front/Rear 的 P65 計數仍是較舊的 per-retest ++（golden 912 是 += 實際 IC 數），落檔值語意跟著移植樹執行期。
    iP65QAReTestCount           =ReadWriteIni(sPath, "P65_QAMode",   "iP65QAReTestCount",    iP65QAReTestCount,           0, bRead);    //Ifor 20260424: [P65] load QA retest count for persistence across program restart

    for(int i=0; i<4; i++)                                                      //Sam 20220720 : 新增一組 Socket Count
    {
        for(int j=0; j<8; j++)
        {
            str1.sprintf("SocketContactSet%d", 1+i*8+j);
            str2.sprintf("SocketContactCount%d", 1+i*8+j);
            str3.sprintf("SocketContactCountHistory%d", 1+i*8+j);

            IniConfig.SocketContactSet[i][j]           = ReadWriteIni(sPath, "SocketContact",AnsiString(str1), IniConfig.SocketContactSet[i][j], 0, bRead);
            IniConfig.SocketContactCount[i][j]         = ReadWriteIni(sPath, "SocketContact",AnsiString(str2), IniConfig.SocketContactCount[i][j], 0, bRead);
            IniConfig.SocketContactCountHistory[i][j]  = ReadWriteIni(sPath, "SocketContact",AnsiString(str3), IniConfig.SocketContactCountHistory[i][j], 0, bRead);
        }
    }
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Tester(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";

    if(IniConfig.bI36TestTimeOut)
        IniConfig.bD52InterFaceErrHeadNeedUp=true;                              //kevin 20161108 test timer out ARM 上升 讓工程師手動取下IC

    if(CosFunction.bHaveFIFOMode)                                               //Steven 20170302 (wei) : FIFO MODE
    {
        IniConfig.iI37_LockLoaderDirection  =ReadWriteIni(sPath, "Tester", "iI37_LockLoaderDirection",  IniConfig.iI37_LockLoaderDirection, 0,     bRead);
    }
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Tray(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.iOCRConditions                                =ReadWriteIni(sPath, "Tray", "iOCRConditions",                           IniConfig.iOCRConditions,                              0,      bRead, true, 0, 2);  //ChungHung 20121002 add OCR Function

    if(CUSTOMER_CODE==CC_ASE_CL)
        IniConfig.bRecordSkipPosition                       =ReadWriteIni(sPath, "Tray", "bRecordSkipPosition",                      IniConfig.bRecordSkipPosition,                         false,  bRead);  //jou 2013-05-30 Record Skip position

#if 0 // TODO(GA1-B2): blocked by TfSpeed::ReadFile -- the fSpeed FACADE now   [AI(W906-POOL2) 20261008: STALE -- ReadFile IS declared and translated now (forms/fSpeed.h, cSpeed.cpp); the real blocker is the link graph: this file is ht9045_globals, so calling it pulls cSpeed.obj -> FileRW/Ld_UldDelayTime.cpp (FileRW_LdUld_Boot / _ReadFile) into every exe that links Prod -- 279 test exes failed to link when opened]
      // exists (forms/fSpeed.h, W906-FW3-Speed-WA 20260820) but deliberately
      // does NOT declare ReadFile: it is an 830-line "Read but writes"
      // function (WriteIniData x3 / SendMSG_CMD x4) queued as write path.
      // Gate stays until that wave lands. (Original premise "no forms/
      // fSpeed.h port exists" is stale as of 20260820 -- updated so nobody
      // re-derives it.)
    if(LoaderUnload_StepMotor)                                                  //Sam 20201221 : Tray y step motor by machine
    {
        fSpeed->ReadFile();
    }
#endif // TODO(GA1-B2): fSpeed->ReadFile (write path)
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Index(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.iLotIDLength                              =ReadWriteIni(sPath, "Configuration", "Lot ID Length",  IniConfig.iLotIDLength,      10, true, true, 1, 10);  //Frank 20170531 (Steven) add LotID 7碼

    IniConfig.iD36_RTCAutoVerifyReleaseHeight           =ReadWriteIni(sPath, "Index", "iD36_RTCAutoVerifyReleaseHeight",            IniConfig.iD36_RTCAutoVerifyReleaseHeight,              0,      bRead);  //jou 2014-06-24 RTC 自動進行Model驗證
    IniConfig.iD36_RTCAutoVerifyPickHeight              =ReadWriteIni(sPath, "Index", "iD36_RTCAutoVerifyPickHeight",               IniConfig.iD36_RTCAutoVerifyPickHeight,                 0,      bRead);  //jou 2014-06-24 RTC 自動進行Model驗證
    IniConfig.iD36_RTCAutoVerifyReleaseHeight           =CheckRange(IniConfig.iD36_RTCAutoVerifyReleaseHeight, 30, -30);
    IniConfig.iD36_RTCAutoVerifyPickHeight              =CheckRange(IniConfig.iD36_RTCAutoVerifyPickHeight, 30, 0);

    if(IniConfig.bD43AutoRetryWhenIndexPickErr)
        IniConfig.bD43IndexDropErrorCanRetryandSkip=true;

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        IniConfig.iD44TestHeadCheckVacuumTime=200;
    }
    else
    {
        IniConfig.iD44TestHeadCheckVacuumTime               =ReadWriteIni(sPath, "Index", "iD44TestHeadCheckVacuumTime",                IniConfig.iD44TestHeadCheckVacuumTime,             (int)LastSet.iTestHeadCheckVacuumTime,  bRead, true, 100000, 100);
    }
    IniConfig.bIndexAddPressEP                          =ReadWriteIni(sPath, "Index", "bIndexAddPressEP",                           IniConfig.bIndexAddPressEP,                                 false,  true);  //jou 20171026 (wei) : 測試中加壓EP
    IniConfig.iIndexAddPressEP_Time                     =ReadWriteIni(sPath, "Index", "iIndexAddPressEP_Time",                      IniConfig.iIndexAddPressEP_Time,                            3,      true,true, 1, 10);  //jou 20171026 (wei) : 測試中加壓EP
    IniConfig.dIndexAddPressEP_Kg                       =ReadWriteIni(sPath, "Index", "dIndexAddPressEP_Kg",                        IniConfig.dIndexAddPressEP_Kg,                              1.0,    true,true, 0.1, 3.0);  //jou 20171026 (wei) : 測試中加壓EP
    IniConfig.dIndexVibrateEP_Kg                        =ReadWriteIni(sPath, "Index", "dIndexVibrateEP_Kg",                         IniConfig.dIndexVibrateEP_Kg,                               0.2,    true,true, 0.1, 2.0);  //jou 20171026 (wei) : 測試中加壓EP

    IniConfig.iGalilSpeedAcc                            =ReadWriteIni(sPath, "Index", "iGalilSpeedAcc",                             IniConfig.iGalilSpeedAcc,                             45000000,     true,true, 45000000, 900000000);
    IniConfig.iGalilSpeedDec                            =ReadWriteIni(sPath, "Index", "iGalilSpeedDec",                             IniConfig.iGalilSpeedDec,                             45000000,     true,true, 45000000, 900000000);

    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210604 : IndexY偵測範圍名子統一成IniConfig.GaliPosRange
    {
        IniConfig.GaliPosRange        =ReadWriteIni(sPath, "Index", "GaliPosRange",             IniConfig.GaliPosRange,                10,    bRead,true, 5, 100);  //Isaac 20201012 : index Y超過範圍，做一次Tmode
    }
    else
    {
        IniConfig.GaliPosRange        =ReadWriteIni(sPath, "Index", "GaliPosRange",             IniConfig.GaliPosRange,                50,    bRead,true, 50, 100);  //Isaac 20201012 : index Y超過範圍，做一次Tmode
    }

    if(LastSet.bUpdateIndexLoadRateToLastSet==false)
    {
        //Steven 20160329 : 把Load Rate轉成LastSet, 前面是 0: HT, 1: NS, 2: Offset; 後面是 0:60, 1:56, 2:40, 3:30
        LastSet.dIndexLoadRate[0][0]                        =ReadWriteIni(sPath, "Index", "Index60mmLoadRate",                          IniConfig.dIndex60mmLoadRate,                           1.03,   bRead, true, 1.5, 0.5);  //jou 2011-06-10
        LastSet.dIndexLoadRate[0][1]                        =ReadWriteIni(sPath, "Index", "Index56mmLoadRate",                          IniConfig.dIndex56mmLoadRate,                           0.90,   bRead, true, 1.5, 0.5);  //wei 20151005 add 56mm
        LastSet.dIndexLoadRate[0][2]                        =ReadWriteIni(sPath, "Index", "Index40mmLoadRate",                          IniConfig.dIndex40mmLoadRate,                           1.03,   bRead, true, 1.5, 0.5);  //Steven 20110704
        LastSet.dIndexLoadRate[0][3]                        =ReadWriteIni(sPath, "Index", "Index30mmLoadRate",                          IniConfig.dIndex30mmLoadRate,                           0.90,   bRead, true, 1.5, 0.5);  //jou 2011-06-10
        LastSet.dIndexLoadRate[1][0]                        =ReadWriteIni(sPath, "Index", "Index60mmLoadRate_NS",                       IniConfig.dIndex60mmLoadRate_NS,                        1.03,   bRead, true, 1.5, 0.5);  //wei 20150303   京元NS浮動頭
        LastSet.dIndexLoadRate[1][1]                        =ReadWriteIni(sPath, "Index", "Index56mmLoadRate_NS",                       IniConfig.dIndex56mmLoadRate_NS,                        0.90,   bRead, true, 1.5, 0.5);  //wei 20151005 add 56mm
        LastSet.dIndexLoadRate[1][2]                        =ReadWriteIni(sPath, "Index", "Index40mmLoadRate_NS",                       IniConfig.dIndex40mmLoadRate_NS,                        1.03,   bRead, true, 1.5, 0.5);  //wei 20150303   京元NS浮動頭
        LastSet.dIndexLoadRate[1][3]                        =ReadWriteIni(sPath, "Index", "Index30mmLoadRate_NS",                       IniConfig.dIndex30mmLoadRate_NS,                        0.90,   bRead, true, 1.5, 0.5);  //wei 20150303   京元NS浮動頭
        LastSet.dIndexLoadRate[2][0]                        =ReadWriteIni(sPath, "Index", "Index60mmLoadRate_Offset",                   IniConfig.dIndex60mmLoadRate_Offset,                    0.0,    bRead, true, 0.5, -0.5);  //2014-06-26    Dell    for TSMC 高溫Load cell offset
        LastSet.dIndexLoadRate[2][1]                        =ReadWriteIni(sPath, "Index", "Index56mmLoadRate_Offset",                   IniConfig.dIndex56mmLoadRate_Offset,                    0.0,    bRead, true, 0.5, -0.5);  //wei 20151005 add 56mm
        LastSet.dIndexLoadRate[2][2]                        =ReadWriteIni(sPath, "Index", "Index40mmLoadRate_Offset",                   IniConfig.dIndex40mmLoadRate_Offset,                    0.0,    bRead, true, 0.5, -0.5);  //2014-06-26    Dell    for TSMC 高溫Load cell offset
        LastSet.dIndexLoadRate[2][3]                        =ReadWriteIni(sPath, "Index", "Index30mmLoadRate_Offset",                   IniConfig.dIndex30mmLoadRate_Offset,                    0.0,    bRead, true, 0.5, -0.5);  //2014-06-26    Dell    for TSMC 高溫Load cell offset
        LastSet.bUpdateIndexLoadRateToLastSet=true;
        WriteLastDataFile();
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        IniConfig.bL09HotTempShuttleNoAddPos=true;                              //JerryYang 20230204 : L23 -> L09   //kevin 20200812 no use shuttle offset

//    IniConfig.bL09HotTempShuttleNoAddPos                    =ReadWriteIni(sPath, "Tempture", "bL23HotTempShuttlenoAddPos",              IniConfig.bL09HotTempShuttleNoAddPos,                   0.0,     false,true, 0.0, 0.0);  //JerryYang 20230204 : 多餘的, Mark掉     //kevin 20200812 add
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_InOutArm(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";
    AnsiString szDir="", szDir1="";

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151022 強制使用本機的Offset
    {
        szDir.sprintf("%sDefineOffset", DefaultPath);
        szDir1.sprintf("%s%s", OffsetPath, GetLastOpenFN());
        if(!DirectoryExists(szDir))
        {
            MyForceDirectories(szDir);
            szDir+="\\Position Offset.Data";
            szDir1+="\\Position Offset.Data";
            if(!DirectoryExists(szDir))
                CopyFile(szDir1.c_str(),szDir.c_str(),true);
        }
    }

    //Ifor 20161121 add Use Fix3 Full Tray By CosFunction
    //==>
    if(FIX3_FULL_PLACE!=Fix3K_Uninstall)                                        //Ifor 20161209 如果有安裝 Fix3 Full Place 強制關閉 Fix3 Full Tray 功能
        CosFunction.bUseFix3FullTray=false;                                     //Steven 20250911 : Mark for 硬體 fix3 full 可以關閉
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_QA_Mode(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";

    if(IniConfig.bQAMode)                                                       //Steven 20111005
    {
        IniConfig.bQAModeFirstIn    =ReadWriteIni(sPath, "QA Mode", "First In",         IniConfig.bQAModeFirstIn,       true,   bRead);
        IniConfig.iBackUpTesterMode =ReadWriteIni(sPath, "QA Mode", "Tester Mode",      IniConfig.iBackUpTesterMode,    1,      bRead);
        IniConfig.bBackUpInArmMode  =ReadWriteIni(sPath, "QA Mode", "In Arm Mode",      IniConfig.bBackUpInArmMode,     false,  bRead);
        IniConfig.bBackUpAutoFeed   =ReadWriteIni(sPath, "QA Mode", "AutoFeed Mode",    IniConfig.bBackUpAutoFeed,      false,  bRead);
    }
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_SingleTempLimit(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";
    AnsiString str;

    for(int i=tcHotPlate1; i<tcTotalCount; i++)                                 //Steven 20111013
    {
        if(IniConfig.bVTESTFunction==true)                                      //jou 20231101 : 客戶要求 temp頁面 Single Limit 不使用
        {
            IniConfig.dSingleTempLimit[i]=0;
        }
        else
        {
            str.sprintf("CH%02d", i);
            IniConfig.dSingleTempLimit[i]=ReadWriteIni(sPath, "SingleTempLimit", str, IniConfig.dSingleTempLimit[i], 0.0, bRead, true, 0.0, 10.0);
        }
    }
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Contact_Force(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.iContactForceMap[0][0]    =ReadWriteIni(sPath, "Contact Force", "30MM_10KG", IniConfig.iContactForceMap[0][0], 196, bRead, true, 0,                                 4095);  //Steven 20111107 : Contact Force的公式
    IniConfig.iContactForceMap[0][1]    =ReadWriteIni(sPath, "Contact Force", "30MM_60KG", IniConfig.iContactForceMap[0][1], 933, bRead, true, IniConfig.iContactForceMap[0][0],  4095);  //Steven 20111107 : Contact Force的公式
    IniConfig.iContactForceMap[1][0]    =ReadWriteIni(sPath, "Contact Force", "40MM_10KG", IniConfig.iContactForceMap[1][0], 163, bRead, true, 0,                                 4095);  //Steven 20111107 : Contact Force的公式
    IniConfig.iContactForceMap[1][1]    =ReadWriteIni(sPath, "Contact Force", "40MM_60KG", IniConfig.iContactForceMap[1][1], 873, bRead, true, IniConfig.iContactForceMap[1][0],  4095);  //Steven 20111107 : Contact Force的公式
    IniConfig.iContactForceMap[2][0]    =ReadWriteIni(sPath, "Contact Force", "60MM_10KG", IniConfig.iContactForceMap[2][0], 130, bRead, true, 0,                                 4095);  //Steven 20111107 : Contact Force的公式
    IniConfig.iContactForceMap[2][1]    =ReadWriteIni(sPath, "Contact Force", "60MM_60KG", IniConfig.iContactForceMap[2][1], 813, bRead, true, IniConfig.iContactForceMap[2][0],  4095);  //Steven 20111107 : Contact Force的公式
    IniConfig.iContactForceMap[3][0]    =ReadWriteIni(sPath, "Contact Force", "56MM_10KG", IniConfig.iContactForceMap[3][0], 130, bRead, true, 0,                                 4095);  //Steven 20111107 : Contact Force的公式    //wei 20151005 add 56mm
    IniConfig.iContactForceMap[3][1]    =ReadWriteIni(sPath, "Contact Force", "56MM_60KG", IniConfig.iContactForceMap[3][1], 813, bRead, true, IniConfig.iContactForceMap[3][0],  4095);  //Steven 20111107 : Contact Force的公式    //wei 20151005 add 56mm
//    IniConfig.bD04MinForceByFile        =ReadWriteIni(sPath, "Contact Force", "bD04MinForceByFile", IniConfig.bD04MinForceByFile, 0, bRead, false);                                             //Steven 20190314 : Min force is read from file
//    IniConfig.dD04MinForceByFile        =ReadWriteIni(sPath, "Contact Force", "dD04MinForceByFile", IniConfig.dD04MinForceByFile, 1.0, bRead, true, 1.0, 120.0);

    if(bRead)
    {
        IniConfig.iEP_Min_KG=ReadWriteIni(sPath, "Contact Force", "iEP_Min_KG", IniConfig.iEP_Min_KG, 16, bRead);       //jou 2013-07-19 EP Min KG
    }
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Socket(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";
    if(IniConfig.bSocketCommunication==false)
        IniConfig.bEnableSocketCommunication    =false;
    else
        IniConfig.bEnableSocketCommunication    =ReadWriteIni(sPath, "Socket", "bEnableSocketCommunication", IniConfig.bEnableSocketCommunication,  false,          bRead);  //ChungHung 20130112 add for ASE_KR Socket Tester
    IniConfig.SocketTimeOut                     =ReadWriteIni(sPath, "Socket", "SocketTimeout",              IniConfig.SocketTimeOut,               30,             bRead);
    IniConfig.SocketIP                          =ReadWriteIni(sPath, "Socket", "SocketIP",                   IniConfig.SocketIP,                    "127.0.0.1",    bRead);
    IniConfig.SocketPort                        =ReadWriteIni(sPath, "Socket", "SocketPort",                 IniConfig.SocketPort,                  "1024",         bRead);
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Visible(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";
    IniConfig.bShowUPH                          =ReadWriteIni(sPath, "Visible", "bShowUPH",                     IniConfig.bShowUPH,                     LastSet.ShowUPH,  bRead);
    IniConfig.bShowIndexTime                    =ReadWriteIni(sPath, "Visible", "bShowIndexTime",               IniConfig.bShowIndexTime,               LastSet.ShowIndexTime,  bRead);
    IniConfig.bShowTimeInfo                     =ReadWriteIni(sPath, "Visible", "bShowTimeInfo",                IniConfig.bShowTimeInfo,                LastSet.bShowTimeInfo,  bRead);
    IniConfig.bShowContactHeight                =ReadWriteIni(sPath, "Visible", "bShowContactHeight",           IniConfig.bShowContactHeight,           LastSet.bShowContactHeight,  bRead);
    IniConfig.bShowLoaderCT                     =ReadWriteIni(sPath, "Visible", "bShowLoaderCT",                IniConfig.bShowLoaderCT,                LastSet.ShowLoaderCT,  bRead);
    IniConfig.bShowContactCT                    =ReadWriteIni(sPath, "Visible", "bShowContactCT",               IniConfig.bShowContactCT,               LastSet.ShowContactCT,  bRead);
    IniConfig.bShowTestCate                     =ReadWriteIni(sPath, "Visible", "bShowTestCate",                IniConfig.bShowTestCate,                LastSet.ShowTestCate,  bRead);
    IniConfig.bShowScanCate                     =ReadWriteIni(sPath, "Visible", "bShowScanCate",                IniConfig.bShowScanCate,                LastSet.ShowScanCate,  bRead);
    IniConfig.bShowTemper                       =ReadWriteIni(sPath, "Visible", "bShowTemper",                  IniConfig.bShowTemper,                  LastSet.ShowTemper,  bRead);
    IniConfig.bShowBinCT                        =ReadWriteIni(sPath, "Visible", "bShowBinCT",                   IniConfig.bShowBinCT,                   LastSet.ShowBinCT,  bRead);
    IniConfig.iShowCateByArm                    =ReadWriteIni(sPath, "Visible", "iShowCateByArm",               IniConfig.iShowCateByArm,               (LastSet.bShowCateByArm)?1:0,  bRead);
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Auto_Clean(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";
    if(bRead)
    {
        if(CosFunction.bAutoCleanShuttleDisable==true)                          //jou 2013-02-27 Auto Clean disable shuttle sensor detect
        {
            if(CUSTOMER_CODE==CC_KYEC_LEE       ||
               CosFunction.bHiSiliconFunction   ||
               CUSTOMER_CODE==CC_KYEC_XILINX    )                               //wei 20160308 Auto clean 強制開啟shuttle sensor偵測
                IniConfig.bAutoCleanShuttleDisable=false;
            else
                IniConfig.bAutoCleanShuttleDisable=ReadWriteIni(sPath, "Auto Clean", "bAutoCleanShuttleDisable", IniConfig.bAutoCleanShuttleDisable, true, bRead);  //jou 2013-02-27 Auto Clean disable shuttle sensor detect
        }
    }
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Barcode_Reader(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";

    if(CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_KYEC_XILINX)
    {
        IniConfig.iA11BarcodeTime  =180;                                        //wei 20150909 強制設定180s
        IniConfig.bA11BarcodeTime  =true;
    }
}
//---------------------------------------------------------------------------
void ProcessLastSetIni_Specific(bool bRead)
{
    AnsiString sPath=AuthPath+"config.ini";
    //Sam 20221018 : 修正 LockByFile 問題 Mark
    //IniConfig.bF06_Active       =ReadWriteIni(sPath, "Specific", "F06_Active",      IniConfig.bF06_Active,      true,   bRead);                     //Steven 20140627 : Add for ASE-CL -- F06 打勾
    //IniConfig.bF06_Enable       =ReadWriteIni(sPath, "Specific", "F06_Enabled",     IniConfig.bF06_Enable,      false,  bRead);                     //Steven 20140627 : Add for ASE-CL -- F06 Enable
    IniConfig.bD41_Active       =ReadWriteIni(sPath, "Specific", "D41_Active",      IniConfig.bD41_Active,      false,  bRead);                                 //Steven 20140627 : Add for ASE-CL -- D41 打勾  //jou 2015-10-19 bD41_Active true->false
    IniConfig.bD41_Enable       =ReadWriteIni(sPath, "Specific", "D41_Enabled",     IniConfig.bD41_Enable,      false,  bRead);                                 //Steven 20140627 : Add for ASE-CL -- D41 Enable
    IniConfig.iD41_Position     =ReadWriteIni(sPath, "Specific", "D41_Position",    IniConfig.iD41_Position,    1,      bRead, true, 0,   1);                   //Steven 20140627 : Add for ASE-CL -- D41 Inside/Above
    IniConfig.dD41_Offset       =ReadWriteIni(sPath, "Specific", "D41_Offset",      IniConfig.dD41_Offset,      2.0,    bRead, true, 0.0, 10.0);                //Steven 20140627 : Add for ASE-CL -- D41 高度
    IniConfig.bD42_Active       =ReadWriteIni(sPath, "Specific", "D42_Active",      IniConfig.bD42_Active,      true,   bRead);                                 //JerryYang 20160220 add for Amkor-Philipine -- D42 打勾
    IniConfig.bD42_Enable       =ReadWriteIni(sPath, "Specific", "D42_Enabled",     IniConfig.bD42_Enable,      false,  bRead);                                 //JerryYang 20160220 add for Amkor-Philipine -- D42 Enable
    IniConfig.bD44_Active       =ReadWriteIni(sPath, "Specific", "D44_Active",      IniConfig.bD44_Active,      true,   bRead);                                 //JerryYang 20160220 add for Amkor-Philipine -- D44 打勾
    IniConfig.bD44_Enable       =ReadWriteIni(sPath, "Specific", "D44_Enabled",     IniConfig.bD44_Enable,      false,  bRead);                                 //JerryYang 20160220 add for Amkor-Philipine -- D44 Enable
    //Sam 20221018 : 修正 LockByFile 問題 Mark
    //IniConfig.bP24_Active       =ReadWriteIni(sPath, "Specific", "P24_Active",      IniConfig.bP24_Active,      true,   bRead);                     //JerryYang 20160220 add for 矽格北興 -- P24 打勾
    //IniConfig.bP24_Enable       =ReadWriteIni(sPath, "Specific", "P24_Enabled",     IniConfig.bP24_Enable,      false,  bRead);                     //JerryYang 20160220 add for 矽格北興 -- P24 Enable

    IniConfig.bF26_Enable       =ReadWriteIni(sPath, "Specific", "F26_Enabled",     IniConfig.bF26_Enable,      false,  bRead);                                 //Sam 20220527 : for 矽格湖口 -- F26 Enable
    //Sam 20221018 : 修正 LockByFile 問題 Mark
    //IniConfig.bI06_Active       =ReadWriteIni(sPath, "Specific", "I06_Active",      IniConfig.bI06_Active,      true,   bRead);                     //Sam 20220527 : for 矽格北興 -- I06 打勾
    //IniConfig.bI06_Enable       =ReadWriteIni(sPath, "Specific", "I06_Enabled",     IniConfig.bI06_Enable,      false,  bRead);                     //Sam 20220527 : for 矽格北興 -- I06 Enable
    if(CUSTOMER_CODE==CC_TSMC_TAINAN ||                                         //kevin 20180214 add
       CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //wei 20160726 TSMC GPIB Lot End
    {
        IniConfig.bI31_1GPIBLotEnd=ReadWriteIni(sPath, "Specific", "I31_GPIBLotEnd",  IniConfig.bI31_1GPIBLotEnd,   false,  bRead);                             //wei 20160726 TSMC GPIB Lot End
        IniConfig.bN07_EnableSecsLotCheck=ReadWriteIni(sPath, "Specific", "N07_SecsLotCheck",  IniConfig.bN07_EnableSecsLotCheck,   false,  bRead);             //wei 20160727 Secs Gem Lot Check
        IniConfig.bI31_2GPIBLotStart     =ReadWriteIni(sPath, "Specific", "cbI31_2GPIBLotStart",  IniConfig.bI31_2GPIBLotStart,   false,  bRead);               //kevin 20190613 add GPIB LotStart
    }
    else if(CosFunction.bGPIBLotEnd==true)                                      //Sam 20181030 : Add GPIBLotEnd Function
    {
        IniConfig.bI31_1GPIBLotEnd=ReadWriteIni(sPath, "Specific", "I31_GPIBLotEnd",  IniConfig.bI31_1GPIBLotEnd,   false,  bRead);
        IniConfig.bN07_EnableSecsLotCheck=false;
        IniConfig.bI31_2GPIBLotStart     =ReadWriteIni(sPath, "Specific", "cbI31_2GPIBLotStart",  IniConfig.bI31_2GPIBLotStart,   false,  bRead);               //kevin 20190613 add GPIB LotStart
    }
    else
    {
        IniConfig.bI31_1GPIBLotEnd=false;
        IniConfig.bN07_EnableSecsLotCheck=false;
        IniConfig.bI31_2GPIBLotStart =false;                                    //kevin 20190613 add
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        IniConfig.bD41_Active=false;
    else
        IniConfig.bD41_Active       =ReadWriteIni(sPath, "Specific", "D41_Active",      IniConfig.bD41_Active,      false,  bRead);                             //Steven 20140627 : Add for ASE-CL -- D41 打勾  //jou 2015-10-19 bD41_Active true->false

    if(CosFunction.bLockD41ByFile)                                              //Steven 20140627 : Add for ASE-CL
    {
        IniConfig.iD41SocketInitialICCheckPosition        =IniConfig.iD41_Position;
        IniConfig.dD41SocketInitialCheckOffset            =IniConfig.dD41_Offset;
    }
    //Sam 20221018 : 修正 LockByFile 問題 Mark
}
//---------------------------------------------------------------------------
void SetCustomerLimitationForConfig()
{
    //Steven 20260924 (S12-C, 使用者：「C HTEditList、D 二進位這兩個的讀寫檔先完成吧」)
    //   GATE GA1-B2（cbLastSet）退役：閘的理由「not declared anywhere」已過期 —— cbLastSet 在
    //   Public/HTEditList.cpp:202 定義、HTEditList.h 宣告；FileRW/IniConfig.cpp 開機時照 golden
    //   main.cpp:1541 `cbLastSet=new HTEditList;` 建立並由 TfConfiguration::InitConfigEdtList 註冊。
    if(cbLastSet!=NULL)
    {
        HTEditList_ReadEditTextFromFile(cbLastSet, AuthPath, "LastSet.ini");
        HTEditList_InitialDataToEdit(cbLastSet);
    }

//    if(cbTest!=NULL)
//    {
//        cbTest->ReadEditTextFromFile(AuthPath, "LastSet.ini");
//        cbTest->InitialDataToEdit();
//    }

    //Steven 20260924 (S12-C, 使用者：「C HTEditList、D 二進位這兩個的讀寫檔先完成吧」)
    //   GATE GA1-B2（elConfig）退役：elConfig 在 Public/HTEditList.cpp:196 定義；FileRW/IniConfig.cpp 開機時
    //   照 golden main.cpp:1533 建立並由 InitConfigEdtList 註冊 1,5xx 筆。golden 在讀檔後對 IniConfig 的鉗制與修正
    //   （KYEC／ASE／TSMC 客戶碼、Socket 溫度上限、互斥旗標…）跟著生效。只有依賴 fMain／fLotInfo 缺件成員的
    //   幾行留在 GATE (S12-C1) 內（pnlPowerSaving／cbDisableSiteMappingCheck／tsRFMD／btnESCFunction／slTestLog（AI(W906-W150) 20261007 (St02-E)：slTestLog 那段已解，見 :3066）
    //   在 forms/fMain.h、forms/fLotInfo.h 都沒有）。
    if(elConfig!=NULL)
    {
        HTEditList_ReadEditTextFromFile(elConfig, AuthPath, "config.ini");
        ADAM_Rang(IniConfig.iD26EPEncoderRange);                                //Steven 20250113 : 修正沒傳入Adam range設定值

        #ifdef SOFT_SIMULTE
        IniConfig.bM01EnableMonitorFunction=false;
        #endif

#if 0 // GATE (S12-C1)：fMain->pnlPowerSaving／cbDisableSiteMappingCheck 在 forms/fMain.h 沒有
        if(fMain!=NULL)                                                         //Steven 20240124 : Add protection
        {
            if(IniConfig.bC05_PowerSaveTemp==false &&
               IniConfig.bC05_PowerSaveMotor==false &&
               IniConfig.bC05_PowerSaveVacuum==false)                           //Steven 20240109 : Power saving for vacuum pump
            {
                fMain->pnlPowerSaving->Visible=false;
            }
            else
            {
                fMain->pnlPowerSaving->Visible=true;
            }

            if(CosFunction.bUseOpenCloseSiteMapAtAnyTime==true)                 //Steven 20221228 : Add
            {
                fMain->cbDisableSiteMappingCheck->Visible=IniConfig.bI21EnableASM;
            }
        }
#endif // GATE (S12-C1)

        if(IniConfig.bA09_ByArmCloseSite &&
           IniConfig.bA09_1_AutoCloseArm)                                       //Steven 20220819 : 單Arm Site全關時, 就把Arm關了
        {
            IniConfig.bD30EnableSiteModeSelect=true;
        }

//#if 0 // GATE (S12-C1)：fLotInfo->tsRFMD／btnESCFunction 在 forms/fLotInfo.h 沒有   //AI(W906-S09-ST01B2) 20260929 (St02-E, claim): gate retired -- both members exist now (tsRFMD forms/fLotInfo.h:2300, btnESCFunction :1819); golden 906_0625_Steven cprod.cpp:2850-2851 (V912 :2869-2870)
        fLotInfo->tsRFMD->TabVisible=(IniConfig.bI41EnableEmptySocketCheck);    //Steven 20201022 : For RFMD Empty Socket Check Funstion.
        fLotInfo->btnESCFunction->Enabled=(IniConfig.bI41_6_Manual);
//#endif // GATE (S12-C1)   //AI(W906-S09-ST01B2) 20260929: see :3009

        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            if(IniConfig.iA01ChangeOpTime<=60)
                IniConfig.iA01ChangeOpTime=60;                                  //kevin 20171024 add
        }
        else if(CUSTOMER_CODE==CC_KYEC_LEE)                                     //wei 20150903 鎖定A01
        {
            IniConfig.iA01ChangeOpTime=600;                                     //Ifor 20170808 (wei) KYEC 喬智要求 360 => 600
        }

        if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                       //wei 20170216 (Steven) TSMC ATR 只做一次RT
            IniConfig.iAutoRetestLimit=1;

        if(CUSTOMER_CODE==CC_KYEC_LEE ||
           CUSTOMER_CODE==CC_KYEC_XILINX)
        {
            IniConfig.iA11BarcodeTime=180;                                      //wei 20150909 強制設定180s
        }

        if(Temperature.fWorkTemperBase+IniConfig.iSocketTemptureRangeOver+10>=TempFuseLimitType)
        {                                                                       //Steven 20170711 (Wei) : 修正烏龍麵說一直Alarm socket溫度異常的問題
            IniConfig.iSocketTemptureRangeOver=TempFuseLimitType-Temperature.fWorkTemperBase-11;
        }

        if(IniConfig.bL11_8ATCUseTemperatureCompare==true &&                    //Ifor 20151029 無開啟[L11_5]功能不可開啟[L11_8]功能
           Temperature.bUseReferTempSensor==false)
        {
            IniConfig.bL11_8ATCUseTemperatureCompare=false;
        }

        if(IniConfig.bF36OutShtLoseICResetSetAllToErr==true)                    //JerryYang 20250430 : add
        {
            IniConfig.bF19OutShuttleLoseICNeedPiggyback=false;
            IniConfig.bO01_ResetNeedClearAndCheckHP=false;
        }

        if(IniConfig.asN12_FtpUplaodPath!="/")
        {
            if(IniConfig.asN12_FtpUplaodPath.SubString(IniConfig.asN12_FtpUplaodPath.Length(), 1)!="/")
                IniConfig.asN12_FtpUplaodPath=IniConfig.asN12_FtpUplaodPath + "/";
        }

        if(CosFunction.bTestTimeOutShowSkipAndHome)                             //JerryYang 20231208 SPIL test timeout只能選SKIP或是HOME
        {
            if(IniConfig.iI22TestTimeOutOption==1 || IniConfig.iI22TestTimeOutOption==2)
            {
                IniConfig.iI22TestTimeOutOption=0;
            }
        }

        HTEditList_InitialDataToEdit(elConfig);

//#if 0 // GATE (S12-C1)：fMain->slTestLog 在 forms/fMain.h 沒有   //AI(W906-W150) 20261007 (St02-E): STALE (forms/fMain.h:1292, LogObjects.cpp:241) -- lifted, golden 0618 cprod.cpp:2905-2912
        if(W906_TestLogObj()!=NULL && !W906_SimNetPathBlocked(IniConfig.sN28_Path))                                                         //Steven 20240124 : Add protection
        {
            if(CUSTOMER_CODE==CC_SCK)                                           //Steven 20210608 : JSCK OEE Function.
            {
                W906_TestLogObj()->Path=IniConfig.sN28_Path;   //AI(W906-W150) 20261007 (St02-E): golden :2909 (fMain->slTestLog) -- golden :2905 checks fMain!=NULL; the port checks the object (null when fMain is) and, W58 Q5, leaves a NETWORK [N28] path unapplied in the SIM build (W906_SimNetPathBlocked, common.cpp:2785: SIM + UNC / DRIVE_REMOTE; ship: never) -- the TestLog then stays on its local default
                W906_TestLogObj()->FileName=IniConfig.sN28_IP;   //AI(W906-W150) 20261007 (St02-E): golden :2910
            }
        }
//#endif // GATE (S12-C1)   //AI(W906-W150) 20261007 (St02-E): lifted
    }
    ReadConfigByRecipe();                                                       //JimmyChiu 20220601 : config儲存跟隨recipe
#if 0 // TODO(GA1-B2): blocked by TfMain@forms/fMain.h missing member ShowFunctions
      // (golden main.h method, not present on the ported facade)
    if(fMain!=NULL)                                                             //Steven 20240124 : Add protection
        fMain->ShowFunctions();                                                 //Steven 20240123 : 顯示功能列表
#endif // TODO(GA1-B2): TfMain::ShowFunctions
}
//---------------------------------------------------------------------------
void ReadConfigByRecipe()                                                       //JimmyChiu 20220601 : config儲存跟隨recipe
{
    AnsiString szDir=GetRecipePath();
    //Steven 20260924 (S12-C, 使用者：「C HTEditList、D 二進位這兩個的讀寫檔先完成吧」)
    //   GATE GA1-B2（elConfig_byRecipe）退役：同 cbLastSet／elConfig。
    if(elConfig_byRecipe!=NULL)                                                 //JimmyChiu 20220601 : config儲存跟隨recipe
    {
        HTEditList_ReadEditTextFromFile(elConfig_byRecipe, szDir, asFileNameConfigByRecipe);
        HTEditList_InitialDataToEdit(elConfig_byRecipe);
    }
}
//------------------------------------------------------------------------------
int GetColorSensorOnLoaderByMUN()                                               //Jimmychiu 20230630 : add color sensor MU-N in Loader
{
    bool b1=Sen[SnMCUSensor1].IsOn();
    bool b2=Sen[SnMCUSensor2].IsOn();
    bool b3=Sen[SnMCUSensor3].IsOn();
    bool b4=Sen[SnMCUSensor4].IsOn();
    return (b4<<3)|(b3<<2)|(b2<<1)|b1;
}
//---------------------------------------------------------------------------
bool GetColorSensorIsMapping(AnsiString &sErrorMsg)                             //Jimmychiu 20230630 : add color sensor MU-N in Loader
{
    int iColorSenNum=GetColorSensorOnLoaderByMUN();
    bool bReturn=true;                                                          //非以上模式不判斷且不警報
#if 0 // TODO(GA1-B2): blocked by TfTrayForm@forms/fTrayForm.h missing method GetColorSensor
      // (golden asendic_Loader.cpp's real color-sensor lookup; forms/fTrayForm.h's own file
      // head already flags this exact call site as a known gap). RISK NOTE for the
      // integrator: with this gated, bReturn stays at its "true" default for FT/RT modes
      // too (not just the "neither mode" default it was meant for), i.e. fail-OPEN on the
      // color-sensor-mapping check rather than fail-closed -- flagged, not silently chosen.
    if(iTestRunMode==FT)
    {
        bReturn=fTrayForm->GetColorSensor("ColorSensor_FT")->IsColorEable(iColorSenNum,sErrorMsg);
        sErrorMsg+="(FT)";
    }
    else if(iTestRunMode==RT)
    {
        bReturn=fTrayForm->GetColorSensor("ColorSensor_RT")->IsColorEable(iColorSenNum,sErrorMsg);
        sErrorMsg+="(RT)";
    }
#endif // TODO(GA1-B2): TfTrayForm::GetColorSensor
    #ifdef SOFT_SIMULTE
    bReturn=true;
    #endif
    return bReturn;
}
//---------------------------------------------------------------------------
void ReadLastSetIni()
{
    AnsiString sPath=AuthPath+"config.ini";                                     //JerryYang 20160603

    //Steven 20260924 (S12-C，審查 M5)：GATE GA1-B2 退役。golden `if(fConfiguration!=NULL) fConfiguration->ChangeCBListProperty();`
    //   —— FileRW/IniConfig.cpp 的 IC_ChangeCBListProperty（由 golden 產生）；「fConfiguration!=NULL」＝
    //   TfConfiguration 建構完成（建構子內 :117 那次 fConfiguration 還是 NULL，所以開機那次不跑，同 golden）。
    FileRW_IniConfig_ChangeCBListProperty();                                    //Steven 20190813 : 必須先修改權限才能修改顯示

    CustomerFunctionSelect();                                                   //  客戶功能選擇區
    ReadLastDataFile();

    IniConfig.sMachineType                                  =CheckAndReadIniDataGeneral("Version",          "Model",        AnsiString("HT-9046"));             //kevin 20130425  //Steven 20140606 : For Secs Gem
    IniConfig.sGPIBMachineID                                =CheckAndReadIniDataGeneral("Version",          "Machine ID",   AnsiString("29828"));               //kevin 20130425  //Steven 20140606 : For Secs Gem
//    IniConfig.SocketHandlerID                               =CheckAndReadIniDataGeneral("Version",          "Machine ID",   AnsiString("29828"));
    SetSocketHandlerID(CheckAndReadIniDataGeneral("Version",          "Machine ID",   AnsiString("29828")));            //Jimmychiu 20220805 SocketHandlerID can not be NULL
    IniConfig.RMSTesterID                                   =CheckAndReadIniDataGeneral("Version",          "Tester ID",    AnsiString("HT9046"));              //Ifor 20231219 add Tester ID
    AnsiString szDir=GetRecipePath();
    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        char PcName[255] ;                                                      //Steven 20110131 : 電腦名稱
        unsigned long PcNameLen=255;                                            //Steven 20110131 : 電腦名稱長度
        GetComputerName(PcName, &PcNameLen);                                    //Steven 20110131
        IniConfig.SocketHandlerID=AnsiString(PcName);
    }

    if(!DirectoryExists(szDir))
    {
        MyForceDirectories(szDir);
        bSetupFileNotExist=true;                                                //Ifor 20160822 add 啟動找不到設定中的Setup File檔案Alarm 警示
    }

    szDir=GetRecipeFileName("HotPlate.Data");
    if(IniConfig.bE43AutoCleanUseHotplate)
    {
        HotPlateForm.iPlateSelect   =ReadIniData(szDir, "Hotplate Form", "Using Flag",   0);
        HotPlateForm.iPlateSelect   =HotPlateForm.iPlateSelect & 0x02;
        WriteIniData(szDir, "Hotplate Form", "Using Flag", HotPlateForm.iPlateSelect);
    }

    if(IniConfig.bRecordSkipPosition==true)                                     //JerryYang 20160603 強制開啟bRecordSkipPosition後要先存檔再讀取,避免ini中沒啟用時此功能會失效
    {
        WriteIniData(sPath, "Tray", "bRecordSkipPosition", IniConfig.bRecordSkipPosition);                              //jou 2013-05-30 Record Skip
    }

#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GA1-B2 reason expired -- HTGem is complete through SECSGEM/uHGemClass.h (included above); HTGem::UpdateDataPath is the golden one-liner (SECSGEM/uHGemClass.cpp); golden cprod.cpp:3001
      // only in SECSGEM/uHGemClass.h, which ht9045_globals cannot include without creating
      // the same CMake target cycle root CMakeLists.txt:470 already documents avoiding)
    if(HSys.MyGem!=NULL)
        HSys.MyGem->UpdateDataPath("D:\\HT9045\\IniData\\Data\\");              //Steven 20140902 : SECS GEM
#endif // TODO(GA1-B2): HTGem

    if(IniConfig.bA09_ByArmCloseSite==true)                                     //ChungHung 20130910 alter for SCK can close site by Index 兩個有衝突
        CosFunction.bOneCycleCanChangeArm=false;                                //ChungHung 20140505 alter ==--->=

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20180918 add
    {
        IniConfig.iA01ChangeOpTime=60;
    }

#if 0 // TODO(GA1-B2): blocked by TfMain@forms/fMain.h missing member pnlUnitSpeedDisplay
      // (golden main.h TPanel, not present on the ported facade)
    if(InitialOK==true && fMain!=NULL)
    {
        fMain->pnlUnitSpeedDisplay->Visible=IniConfig.bA26MotorSpeedSortDisplay;                                        //Ifor 20171228 (Steven) : add 判斷是否顯示 Motor Speed Display
    }
#endif // TODO(GA1-B2): TfMain::pnlUnitSpeedDisplay

    ProcessLastSetIni_RMS               (bReadFile);
    ProcessLastSetIni_FTP               (bReadFile);
    ProcessLastSetIni_EventLog          (bReadFile);
    ProcessLastSetIni_Count             (bReadFile);
    ProcessLastSetIni_Tester            (bReadFile);
    ProcessLastSetIni_Tray              (bReadFile);
    ProcessLastSetIni_Index             (bReadFile);
    ProcessLastSetIni_InOutArm          (bReadFile);
    ProcessLastSetIni_QA_Mode           (bReadFile);
    ProcessLastSetIni_SingleTempLimit   (bReadFile);
    ProcessLastSetIni_Contact_Force     (bReadFile);
    ProcessLastSetIni_Socket            (bReadFile);
    ProcessLastSetIni_Visible           (bReadFile);
    ProcessLastSetIni_Auto_Clean        (bReadFile);
    ProcessLastSetIni_Barcode_Reader    (bReadFile);
    ProcessLastSetIni_Specific          (bReadFile);
    SetCustomerLimitationForConfig();   if (W906_SimNetHook) W906_SimNetHook(0);   //AI(W906-W58) 20260930 (St02-E): W36-1 / W58 phase 0 -- SIM: the network keys read OFF for this run (config.ini not written)

    if(USE_SOCKET_SENSOR==999)                                                  //JerryYang 20200408 socket sensor改為硬體選項,預設值使用舊版 Config C08功能
    {
        WriteIniDataGeneral("System", "USE_SOCKET_SENSOR", IniConfig.bC08_SocketSensor);
        USE_SOCKET_SENSOR=CheckAndReadIniDataGeneral("System",   "USE_SOCKET_SENSOR", 0);
        if(USE_SOCKET_SENSOR==0)
        {
            SOCKET_AMP_QTY=0;
        }
        else
        {
            if(USE_COLOR_TRAY_SENSOR)                                           //裝Color sensor預設4 ea
            {
                SOCKET_AMP_QTY=CheckRange(CheckAndReadIniDataGeneral("System",   "SocketSenAmpQty",  4), 0, iSnSocketCnt);
            }
            else
            {
                SOCKET_AMP_QTY=CheckRange(CheckAndReadIniDataGeneral("System",   "SocketSenAmpQty",  8), 0, iSnSocketCnt);
            }
        }
    }

    if(Cylinder[C_Shuttle_Knocker_1].Enable==false &&
       Cylinder[C_Shuttle_Knocker_2].Enable==false)                             //Steven 20160509 : 氣缸沒裝就不要啟用功能
    {
        IniConfig.bF14KnockShuttle=false;
        IniConfig.bF14_1KnockShuttleFirst=false;
    }

#if 0 // TODO(GA1-B2): blocked by TfMainHanaART@forms/fMain.h missing method   [AI(W906-POOL2) 20261008: STALE -- SetHandlerWaitingData exists now but as the offline no-op of the untranslated HanaART subsystem (forms/fMain.h); stays until HanaART is translated]
      // SetHandlerWaitingData (golden main.h HANA ART helper, not present on the ported
      // TfMainHanaART stand-in)
    if(IniConfig.bA10_6_HANA_ART_TestMode_Enable==true)                         //JimmyChiu 20241023 HANA ART Function
    {
        int iHD_Mode=IniConfig.iA10_6_HANA_ART_TestMode;
        fMain->hanaART->SetHandlerWaitingData(IniConfig.sMachineType,
                                              IniConfig.SocketHandlerID,
                                              IniConfig.sMachineType,
                                              iHD_Mode);
    }
#endif // TODO(GA1-B2): TfMainHanaART::SetHandlerWaitingData
   
}
//---------------------------------------------------------------------------
void SaveLastSetIni()
{   if (W906_SimNetHook) W906_SimNetHook(1);   //AI(W906-W58) 20260930 (St02-E): W58 phase 1 -- SIM: a ticked key is saved as ticked (Q1), an armed key gets the file's own value
    WriteIniDataGeneral("Version", "Model",      IniConfig.sMachineType);       //kevin 20130425  //Steven 20140606 : For Secs Gem
    WriteIniDataGeneral("Version", "Machine ID", IniConfig.sGPIBMachineID);     //kevin 20130425  //Steven 20140606 : For Secs Gem
    WriteIniDataGeneral("Version", "Machine ID", IniConfig.SocketHandlerID);    //kevin 20130425  //Steven 20140606 : For Secs Gem
    WriteIniDataGeneral("Version", "Tester ID",  IniConfig.RMSTesterID);        //Ifor 20231219 add Tester ID

    ProcessLastSetIni_RMS               (bWriteFile);
    ProcessLastSetIni_FTP               (bWriteFile);
    ProcessLastSetIni_EventLog          (bWriteFile);
    ProcessLastSetIni_Count             (bWriteFile);
    ProcessLastSetIni_Tester            (bWriteFile);
    ProcessLastSetIni_Tray              (bWriteFile);
    ProcessLastSetIni_Index             (bWriteFile);
    ProcessLastSetIni_InOutArm          (bWriteFile);
    ProcessLastSetIni_QA_Mode           (bWriteFile);
    ProcessLastSetIni_SingleTempLimit   (bWriteFile);
    ProcessLastSetIni_Contact_Force     (bWriteFile);
    ProcessLastSetIni_Socket            (bWriteFile);
    ProcessLastSetIni_Visible           (bWriteFile);
    ProcessLastSetIni_Auto_Clean        (bWriteFile);
    ProcessLastSetIni_Barcode_Reader    (bWriteFile);
    ProcessLastSetIni_Specific          (bWriteFile);
//    if(IniConfig.bEnable_SECS_GEM==true)                                      //JerryYang 20170215 (Steven) Mark for 修改Run check觸發條件
//    {
//        bHasSaveSet=true;                                                     //Ifor 20151204 新增判斷機台有無修改設定檔
//    }

//Steven 20260924 (S12-C 寫方向)：GATE GA1-B2 退役 —— cbLastSet／elConfig／elConfig_byRecipe 由 FileRW/IniConfig.cpp 依 golden
//   main.cpp:1532-1543 建立、TfConfiguration 建構子註冊（具名替身）。`el->SaveEditTextToFile(…)` 經
//   Public/HTEditList.cpp 的轉接函式呼叫（本檔不能 include HTEditList.h，見檔頭）。
//   golden 的 fConfiguration->cbN07_EnableHostStart->Checked 讀的是 TfConfiguration 的替身
//   （FileRW_ProxyChecked，FileRW/_EditList.cpp）。
    if(cbLastSet!=NULL)
    {
        HTEditList_SaveEditTextToFile(cbLastSet, AuthPath, "LastSet.ini");   // golden: cbLastSet->SaveEditTextToFile(AuthPath, "LastSet.ini");
    }

//    if(cbTest!=NULL)
//    {
//        cbTest->SaveEditTextToFile(AuthPath, "LastSet.ini");
//    }

    if(elConfig!=NULL)
    {
        if(IniConfig.bSPILFunction==true &&                                     //JerryYang 20250423 : SPIL順信要求, 偵測到Run check被關閉時
            FileRW_ProxyChecked("TfConfiguration", "cbN07_EnableHostStart")==false &&   // golden: fConfiguration->cbN07_EnableHostStart->Checked==false
            IniConfig.bRCMDStart==true)
        {
            ShowErrorMessage("WAR16132", 0, MMSystem, false, "");               //In shuttle 1 check have duplicate barcode in lot error!
        }

        HTEditList_SaveEditTextToFile(elConfig, AuthPath, "config.ini");     // golden: elConfig->SaveEditTextToFile(AuthPath, "config.ini");
        SendCommand_EventLog(EL_UPDATE_PARAMETER, "1");
    }

    AnsiString szDir=GetRecipePath();
    if(elConfig_byRecipe!=NULL)                                                 //Sam 20220921 : config儲存跟隨recipe
    {
        HTEditList_SaveEditTextToFile(elConfig_byRecipe, szDir, asFileNameConfigByRecipe);   // golden: elConfig_byRecipe->SaveEditTextToFile(szDir, asFileNameConfigByRecipe);
    }
    if (W906_SimNetHook) W906_SimNetHook(2);   }   //AI(W906-W58) 20260930 (St02-E): W58 phase 2 -- SIM: an armed key's file text written back (the mask never changes a SHIP setting)
//---------------------------------------------------------------------------
void SaveRmsInfo(AnsiString Name, AnsiString Temp)                              //Steven 20110527
{
    AnsiString sPath=AuthPath+"config.ini", str="";

    str=(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)?"RMS":"Server";        //ChungHung 20130621 add SCK RMS
    if(IniConfig.bShowLotInfo)
    {
        IniConfig.sProductName=Name;
        IniConfig.sProductTemp=Temp;

        WriteIniData(sPath, str, "Product Name", Name);
        WriteIniData(sPath, str, "Product Temp", Temp);
    }
}
//---------------------------------------------------------------------------
void ReadRmsInfo()                                                              //Steven 20110527
{
    AnsiString sPath=AuthPath+"config.ini", str="";

    str=(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)?"RMS":"Server";        //ChungHung 20130621 add SCK RMS
    if(IniConfig.bShowLotInfo)
    {
        IniConfig.sProductName=CheckAndReadIniData(sPath, str, "Product Name", AnsiString(""));
        IniConfig.sProductTemp=CheckAndReadIniData(sPath, str, "Product Temp", AnsiString(""));
    }
}
//---------------------------------------------------------------------------
void SaveEventLogAutoSaveInfo()                                                 //Steven 20110603
{
    AnsiString sPath=AuthPath+"config.ini", str="";
    if(IniConfig.bEventLogAutoSaveFunction)
    {
        WriteIniData(sPath, "Event Log", "EventLogRecordDate", IniConfig.dtEventLogLastRecordDate);
        //AI(GA1-B2) 20260804: .FormatString(fmt) -> FormatDateTime(fmt,dt) -- vclcompat has
        //  no TDateTime::FormatString member; FormatDateTime is the real free-function
        //  equivalent (vclcompat/TDateTime.h:67), same established mechanical translation
        //  as GA-1-B4 (cMyDB.cpp) already applied.
        IniConfig.sEvenLogDataTime=FormatDateTime("yyyy/mm/dd hh:mm:ss", IniConfig.dtEventLogLastRecordDate);              //Ifor 20160621 新增Even Log Record Date Time 字串格式 避免不同系統產生異常問題
        WriteIniData(sPath, "Event Log", "sEvenLogDataTime", IniConfig.sEvenLogDataTime);
    }
}
//---------------------------------------------------------------------------
void ReadEventLogAutoSaveInfo()                                                 //Steven 20110603
{
    AnsiString sPath=AuthPath+"config.ini", str="";
    AnsiString SDate;
    //AI(GA1-B2) 20260804: (Now()-1) is ambiguous under vclcompat::TDateTime -- it has
    //  BOTH operator-(const TDateTime&) (taking int 1 via the converting TDateTime(double)
    //  ctor) AND operator double() (letting built-in double subtraction compete), so GCC
    //  cannot pick one. Made explicit via .Val() (same idiom GA-1-B4 used for cMyDB.cpp's
    //  TDateTime+double ambiguity). .FormatString(fmt) -> FormatDateTime(fmt,dt), same as above.
    SDate=FormatDateTime("yyyy/mm/dd hh:mm:ss", TDateTime(Now().Val()-1.0));
    if(IniConfig.bEventLogAutoSaveFunction)
    {
        try
        {
            IniConfig.sEvenLogDataTime=CheckAndReadIniData(sPath, "Event Log", "sEvenLogDataTime", SDate);              //真的沒有資料的話就追朔1天 //Ifor 20160621 EventLogRecordDate 修改由 sEvenLogDataTime 字串轉換成系統時間，避免不同系統&時間格式不同發生異常
        }
        catch(...)
        {
            MyDBIProcess("Exception", "ReadEventLogAutoSaveInfo");
            IniConfig.sEvenLogDataTime=SDate;
        }

        if(IniConfig.sEvenLogDataTime=="" ||
           IniConfig.sEvenLogDataTime=="NULL")                                  //Ifor 20160621 sEvenLogDataTime字串時間若無資料往前追朔1天
            IniConfig.sEvenLogDataTime=SDate;

        try
        {
            SYSTEMTIME SysTime;                                                 //Ifor 20160621 EvenLog Recode Data 修改固定格式 yyyy/mm/dd hh:mm:ss
            GetLocalTime(&SysTime);                                             //Ifor 20160829 add 無給初始化會有負數出現導致異常
            SysTime.wYear   = atoi(IniConfig.sEvenLogDataTime.SubString(1, 4).c_str());
            SysTime.wMonth  = atoi(IniConfig.sEvenLogDataTime.SubString(6, 2).c_str());
            SysTime.wDay    = atoi(IniConfig.sEvenLogDataTime.SubString(9, 2).c_str());
            SysTime.wHour   = atoi(IniConfig.sEvenLogDataTime.SubString(12,2).c_str());
            SysTime.wMinute = atoi(IniConfig.sEvenLogDataTime.SubString(15,2).c_str());
            SysTime.wSecond = atoi(IniConfig.sEvenLogDataTime.SubString(18,2).c_str());
            IniConfig.dtEventLogLastRecordDate=SystemTimeToDateTime(SysTime);   //Ifor 20160621 字串時間轉目前系統時間格式
            IniConfig.sEventLogLastRecordDate=FormatDateTime("yyyy/mm/dd hh:mm:ss", IniConfig.dtEventLogLastRecordDate);   //Ifor 20160621 修改Secs Gem 時間格式固定 yyyy/mm/dd hh:mm:ss
        }
        catch(...)
        {
            MyDBIProcess("Exception", "ReadEventLogAutoSaveInfo");
        }
    }
}
//---------------------------------------------------------------------------
void SaveTasterInfo()                                                           //Steven 20110603
{
    AnsiString sPath=AuthPath+"config.ini", str="";
    if(CosFunction.bFTPFunction)
    {
        WriteIniData(sPath, "Taster", "Taster Map File",        IniConfig.N06_TasterListMap);                           //Steven 20121018 : Handler與測試機連線的IP
        WriteIniData(sPath, "Taster", "Taster List File",       IniConfig.N06_TasterListFile);
        WriteIniData(sPath, "Taster", "N06 Taster Recipe Path", IniConfig.asN06_TesterPath);
        WriteIniData(sPath, "Taster", "Taster Input Method",    IniConfig.TasterInputMethod);                           //Steven 20110311 : Taster輸入方法
        WriteIniData(sPath, "Taster", "Taster Type",            IniConfig.TasterType);                                  //Steven 20110305 : Taster型號
        WriteIniData(sPath, "Taster", "Taster No",              IniConfig.TasterNo);                                    //Steven 20110305 : Taster號碼
        WriteIniData(sPath, "Taster", "Taster Name",            IniConfig.TasterName);                                  //Steven 20110311 : Taster名稱
    }
}
//---------------------------------------------------------------------------
void ReadTasterInfo()                                                           //Steven 20110603
{
    AnsiString sPath=AuthPath+"config.ini", str="";
    if(CosFunction.bFTPFunction)
    {
        if(CUSTOMER_CODE==CC_TSMC_TAINAN)
        {
            IniConfig.N06_TasterListMap     ="";
            IniConfig.N06_TasterListFile    ="";
        }
        else if(CUSTOMER_CODE==CC_JSCC_OS)                                      //長電微電子 (JSCC OS部門)
        {
            IniConfig.asN06_TesterPath      =CheckAndReadIniData(sPath, "Taster", "N06 Taster Recipe Path",  AnsiString("Z:\\"));
            IniConfig.N06_TasterListFile    ="";
            IniConfig.N06_TasterListMap     ="";
        }
        else
        {
            IniConfig.N06_TasterListMap     =CheckAndReadIniData(sPath, "Taster", "Taster Map File",  AnsiString("D:\\KyecData\\Mapping\\TestMap.txt"));        //Steven 20121018 : Handler與測試機連線的IP
            IniConfig.N06_TasterListFile    =CheckAndReadIniData(sPath, "Taster", "Taster List File", AnsiString("D:\\RMS\\tester list.txt"));
        }
        IniConfig.TasterInputMethod =CheckAndReadIniData(sPath, "Taster", "Taster Input Method",  0);                   //Steven 20110311 : Taster輸入方法
        IniConfig.TasterType        =CheckAndReadIniData(sPath, "Taster", "Taster Type", AnsiString(""));               //Steven 20110305 : Taster型號
        IniConfig.TasterNo          =CheckAndReadIniData(sPath, "Taster", "Taster No",   AnsiString(""));               //Steven 20110305 : Taster號碼
        IniConfig.TasterName        =CheckAndReadIniData(sPath, "Taster", "Taster Name", AnsiString(""));               //Steven 20110311 : Taster名稱
    }
}
//---------------------------------------------------------------------------
void ReadRmsPath()                                                              //Steven 20110623
{
    int iLength=0;
    AnsiString sPath=AuthPath+"config.ini", str="";

    str=(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)?"RMS":"Server";        //ChungHung 20130621 add SCK RMS
    if(IniConfig.bShowLotInfo)
    {
        if(CosFunction.bDownloadRecipeLevelMode)                                //jou 2016-01-06 download recipe 增加權限模式選擇
        {
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GA1-B2 reason expired -- TfLotInfo has coLevelMode (forms/fLotInfo.h); golden cprod.cpp:3263
      // (golden uLotInfo.h TComboBox, not present on the ported facade). Narrow gap:
      // only reached when CosFunction.bDownloadRecipeLevelMode is set (a narrow
      // customer feature); IniConfig.sRmsPath keeps its prior value in that case
      // instead of being (re)computed from this branch.
            if(fLotInfo->coLevelMode->Text!="Normal")
                IniConfig.sRmsPath=CheckAndReadIniData(sPath, str, str+" Download Path",    AnsiString("D:\\RMS"));
            else
                IniConfig.sRmsPath=CheckAndReadIniData(sPath, str, str+" Path",             AnsiString("D:\\RMS"));
#endif // TODO(GA1-B2): TfLotInfo::coLevelMode
        }
        else
        {
            IniConfig.sRmsPath    =CheckAndReadIniData(sPath, str, str+" Path",             AnsiString("D:\\HT9045"));
        }

        iLength=IniConfig.sRmsPath.Length();
        if(IniConfig.sRmsPath.SubString(iLength, 1) == "\\")
        {
            IniConfig.sRmsPath=IniConfig.sRmsPath.SubString(1, iLength-1);
        }

        if(CUSTOMER_CODE==CC_SCC ||
           CUSTOMER_CODE==CC_SCK ||                                             //ChungHung 20130621 add SCK RMS
           CUSTOMER_CODE==CC_AMD_M)                                             //Ifor 20200915 add: TF_AMD KL版 新增 RMS
        {
            IniConfig.sRmsDownPath=CheckAndReadIniData(sPath, str, str+" Download Path", AnsiString("D:\\HT9045"));
            iLength=IniConfig.sRmsDownPath.Length();
            if(IniConfig.sRmsDownPath.SubString(iLength, 1) == "\\")
            {
                IniConfig.sRmsDownPath=IniConfig.sRmsDownPath.SubString(1, iLength-1);
            }
        }
    }
}
//---------------------------------------------------------------------------
//注意!! 開關Site模式，修改時要小心
//---------------------------------------------------------------------------
void SaveTestMode()                                                             //Steven 20111019
{
    AnsiString S;
    if(InitialOK==false)                                                        //jou 20171019 (wei) : 修正uTemp_set未讀取就進入此function
        return;

    if(CosFunction.bLastSetInSetUpFile==false)                                  //Steven 20161211 Add
        return;

    AnsiString szDir=GetRecipeFileName("TestMode.Data");

    if(Temperature.iMachineTempMode==0)
        TestMode.iTemperatureMode=Tempture_Hot;
    else  if(Temperature.iMachineTempMode==1)
        TestMode.iTemperatureMode=Tempture_Ambient;

    WriteIniData(szDir, "TestMode", "Tester Connection",    TestMode.iTestConnection);
    WriteIniData(szDir, "TestMode", "Temperature Mode",     TestMode.iTemperatureMode);
    WriteIniData(szDir, "TestMode", "Running Mode",         TestMode.iRunMode);

    if(IsNNMode()==NN_1Row)
    {
        for(int j=0; j<TestSocket.iMaxCol; j++)
        {
            S.sprintf("Dut %s2", IndexSuckName[0][j]);
            if(CosFunction.bFTRTDifferentDutOnOff==true)                        //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
            {
                if(iRunStartMode==FT)
                    WriteIniData(szDir, "DutOnOff", S, TestMode.iDutOnOff[1][0][j]);
                else
                    WriteIniData(szDir, "DutOnOff_RT", S, TestMode.iDutOnOff[1][0][j]);

                if(IniConfig.bI35UseThirdSiteControlByEngineer)                 //Steven 20241220 : add
                {
                    if(iRunStartMode==FT)
                        WriteIniData(szDir, "DutOnOffEE", S, TestMode.iDutOnOffEE[1][0][j]);
                    else
                        WriteIniData(szDir, "DutOnOffEE_RT", S, TestMode.iDutOnOffEE[1][0][j]);
                }
            }
            else
            {
                WriteIniData(szDir, "DutOnOff", S, TestMode.iDutOnOff[1][0][j]);
                if(IniConfig.bI35UseThirdSiteControlByEngineer)                 //Steven 20241220 : add
                    WriteIniData(szDir, "DutOnOffEE", S, TestMode.iDutOnOffEE[1][0][j]);
            }
        }

        for(int j=0; j<TestSocket.iMaxCol; j++)
        {
            S.sprintf("Dut %s", IndexSuckName[1][j]);
            if(CosFunction.bFTRTDifferentDutOnOff==true)                        //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
            {
                if(iRunStartMode==FT)
                    WriteIniData(szDir, "DutOnOff", S, TestMode.iDutOnOff[0][0][j]);
                else
                    WriteIniData(szDir, "DutOnOff_RT", S, TestMode.iDutOnOff[0][0][j]);

                if(IniConfig.bI35UseThirdSiteControlByEngineer)                 //Steven 20241220 : add
                {
                    if(iRunStartMode==FT)
                        WriteIniData(szDir, "DutOnOffEE", S, TestMode.iDutOnOffEE[0][0][j]);
                    else
                        WriteIniData(szDir, "DutOnOffEE_RT", S, TestMode.iDutOnOffEE[0][0][j]);
                }
            }
            else
            {
                WriteIniData(szDir, "DutOnOff", S, TestMode.iDutOnOff[0][0][j]);
                if(IniConfig.bI35UseThirdSiteControlByEngineer)                 //Steven 20241220 : add
                    WriteIniData(szDir, "DutOnOffEE", S, TestMode.iDutOnOffEE[0][0][j]);
            }
        }
    }
    else
    {
        if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)                    //Steven 20170111 : 沒用到的Arm不存取資料
        {
        }
        else
        {
            for(int i=0; i<TestSocket.iMaxRow; i++)
            {
                for(int j=0; j<TestSocket.iMaxCol; j++)
                {
                    S.sprintf("Dut %s", IndexSuckName[i][j]);
                    if(CosFunction.bFTRTDifferentDutOnOff==true)                //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
                    {
                        if(iRunStartMode==FT)
                            WriteIniData(szDir, "DutOnOff", S, TestMode.iDutOnOff[0][i][j]);
                        else
                            WriteIniData(szDir, "DutOnOff_RT", S, TestMode.iDutOnOff[0][i][j]);

                        if(IniConfig.bI35UseThirdSiteControlByEngineer)         //Steven 20241220 : add
                        {
                            if(iRunStartMode==FT)
                                WriteIniData(szDir, "DutOnOffEE", S, TestMode.iDutOnOffEE[0][i][j]);
                            else
                                WriteIniData(szDir, "DutOnOffEE_RT", S, TestMode.iDutOnOffEE[0][i][j]);
                        }
                    }
                    else
                    {
                        WriteIniData(szDir, "DutOnOff", S, TestMode.iDutOnOff[0][i][j]);
                        if(IniConfig.bI35UseThirdSiteControlByEngineer)         //Steven 20241220 : add
                            WriteIniData(szDir, "DutOnOffEE", S, TestMode.iDutOnOffEE[0][i][j]);
                    }
                }
            }
        }

        if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)                    //Steven 20170111 : 沒用到的Arm不存取資料
        {
        }
        else                                                                    //ChungHung 20130910 alter for SCK can close site by Index
        {
            for(int i=0; i<TestSocket.iMaxRow; i++)
            {
                for(int j=0; j<TestSocket.iMaxCol; j++)
                {
                    S.sprintf("Dut %s2", IndexSuckName[i][j]);
                    if(CosFunction.bFTRTDifferentDutOnOff==true)                //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
                    {
                        if(iRunStartMode==FT)
                            WriteIniData(szDir, "DutOnOff", S, TestMode.iDutOnOff[1][i][j]);
                        else
                            WriteIniData(szDir, "DutOnOff_RT", S, TestMode.iDutOnOff[1][i][j]);

                        if(IniConfig.bI35UseThirdSiteControlByEngineer)         //Steven 20241220 : add
                        {
                            if(iRunStartMode==FT)
                                WriteIniData(szDir, "DutOnOffEE", S, TestMode.iDutOnOffEE[1][i][j]);
                            else
                                WriteIniData(szDir, "DutOnOffEE_RT", S, TestMode.iDutOnOffEE[1][i][j]);
                        }
                    }
                    else
                    {
                        WriteIniData(szDir, "DutOnOff", S, TestMode.iDutOnOff[1][i][j]);
                        if(IniConfig.bI35UseThirdSiteControlByEngineer)         //Steven 20241220 : add
                            WriteIniData(szDir, "DutOnOffEE", S, TestMode.iDutOnOffEE[1][i][j]);
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
//注意!! 開關Site模式，修改時要小心
//---------------------------------------------------------------------------
void ReadTestMode()                                                             //Steven 20111019
{
    if(CosFunction.bLastSetInSetUpFile==false)                                  //Steven 20121211 (wei) : Fixed for site on off
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                iCloseSiteMap[0][i][j]=LastSet.bUseTestSocket[0][i][j];         //kevin 20161003 Site 順序
                iCloseSiteMap[1][i][j]=LastSet.bUseTestSocket[1][i][j];         //kevin 20161003 Site 順序
            }
        }
        return;
    }

    AnsiString S, S2="", szDir=GetRecipeFileName("TestMode.Data");

    TestMode.iTestConnection    =ReadIniData(szDir, "TestMode", "Tester Connection", ON_LINE);
    TestMode.iTemperatureMode   =ReadIniData(szDir, "TestMode", "Temperature Mode", Tempture_Hot);                      //kevin 20141202 造成無 法切恆溫
    TestMode.iRunMode           =ReadIniData(szDir, "TestMode", "Running Mode", REALLY);

    if(CosFunction.bProgramStartOnLine &&
       SystemInitialOK==false &&
       TestMode.iTestConnection!=ON_LINE &&
       TestMode.iRunMode!=REALLY)                                               //Sam 20210423 : 程式開啟時切換為 OnLine/Real
    {
        TestMode.iTestConnection=ON_LINE;
        TestMode.iRunMode=REALLY;
        WriteIniData(szDir, "TestMode", "Tester Connection",    TestMode.iTestConnection);
        WriteIniData(szDir, "TestMode", "Running Mode",         TestMode.iRunMode);
    }

    if(IsNNMode()==NN_1Row)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            S2.sprintf("Dut %s2", IndexSuckName[0][j]);
            if(CosFunction.bFTRTDifferentDutOnOff==true)                        //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
            {
                if(iRunStartMode==FT)
                    TestMode.iDutOnOff[1][0][j] =ReadIniData(szDir, "DutOnOff", S2, LastSet.bUseTestSocket[1][0][j]);
                else
                    TestMode.iDutOnOff[1][0][j] =ReadIniData(szDir, "DutOnOff_RT", S2, LastSet.bUseTestSocket[1][0][j]);

                if(IniConfig.bI35UseThirdSiteControlByEngineer)                 //Steven 20241220 : add
                {
                    if(iRunStartMode==FT)
                        TestMode.iDutOnOffEE[1][0][j]=ReadIniData(szDir, "DutOnOffEE", S, LastSet.bUseTestSocketEE[1][0][j]);
                    else
                        TestMode.iDutOnOffEE[1][0][j]=ReadIniData(szDir, "DutOnOffEE_RT", S, LastSet.bUseTestSocketEE[1][0][j]);
                }
            }
            else
            {
                TestMode.iDutOnOff[1][0][j] =ReadIniData(szDir, "DutOnOff", S2, LastSet.bUseTestSocket[1][0][j]);
                if(IniConfig.bI35UseThirdSiteControlByEngineer)                 //Steven 20241220 : add
                    TestMode.iDutOnOffEE[1][0][j]=ReadIniData(szDir, "DutOnOffEE", S2, LastSet.bUseTestSocketEE[1][0][j]);
            }

            S.sprintf("Dut %s", IndexSuckName[1][j]);
            if(CosFunction.bFTRTDifferentDutOnOff==true)                        //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
            {
                if(iRunStartMode==FT)
                    TestMode.iDutOnOff[0][0][j]=ReadIniData(szDir, "DutOnOff", S, LastSet.bUseTestSocket[0][0][j]);
                else
                    TestMode.iDutOnOff[0][0][j]=ReadIniData(szDir, "DutOnOff_RT", S, LastSet.bUseTestSocket[0][0][j]);

                if(IniConfig.bI35UseThirdSiteControlByEngineer)                 //Steven 20241220 : add
                {
                    if(iRunStartMode==FT)
                        TestMode.iDutOnOffEE[0][0][j]=ReadIniData(szDir, "DutOnOffEE", S, LastSet.bUseTestSocketEE[0][0][j]);
                    else
                        TestMode.iDutOnOffEE[0][0][j]=ReadIniData(szDir, "DutOnOffEE_RT", S, LastSet.bUseTestSocketEE[0][0][j]);
                }
            }
            else
            {
                TestMode.iDutOnOff[0][0][j]=ReadIniData(szDir, "DutOnOff", S, LastSet.bUseTestSocket[0][0][j]);
                if(IniConfig.bI35UseThirdSiteControlByEngineer)                 //Steven 20241220 : add
                    TestMode.iDutOnOffEE[0][0][j]=ReadIniData(szDir, "DutOnOffEE", S, LastSet.bUseTestSocketEE[0][0][j]);
            }

            LastSet.bUseTestSocket[1][0][j]=TestMode.iDutOnOff[1][0][j];
            LastSet.bUseTestSocket[0][0][j]=TestMode.iDutOnOff[0][0][j];
            LastSet.bUseTestSocketEE[1][0][j]=TestMode.iDutOnOffEE[1][0][j];    //Steven 20241220 : add
            LastSet.bUseTestSocketEE[0][0][j]=TestMode.iDutOnOffEE[0][0][j];

            iCloseSiteMap[0][0][j]=TestMode.iDutOnOff[0][0][j];                 //kevin 20161003 Site 順序
            iCloseSiteMap[1][0][j]=TestMode.iDutOnOff[1][0][j];                 //kevin 20161003 Site 順序
        }
    }
    else
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)  //Sam 20170515 (wei) TestIF 改為 TestIF_File 修正切換工作檔開關 Sit 的問題 //Steven 20170111 : 沒用到的Arm不存取資料
                {
                    if(CUSTOMER_CODE==CC_KYEC_LEE ||                            //Ifor 20170608 (wei) add 開關ARM 會造成自動關Site問題
                       CUSTOMER_CODE==CC_SIGURD_PeiXing ||                      //KaiChen 20190123 ：Fix 開關 ARM 會造成自動關 Site 問題
                       TestIF_File.bArm1PickPlaceArm2Test==true)                //JerryYang 20290901 : 修正一丟一測ATC ARM2沒有溫度的問題
                    {
                    }
                    else
                    {
                        TestMode.iDutOnOff[0][i][j]=false;
                        TestMode.iDutOnOffEE[0][i][j]=false;                    //Steven 20241220 : add
                    }
                }
                else
                {
                    S.sprintf("Dut %s", IndexSuckName[i][j]);
                    if(CosFunction.bFTRTDifferentDutOnOff==true)                //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
                    {
                        if(iRunStartMode==FT)
                            TestMode.iDutOnOff[0][i][j]=ReadIniData(szDir, "DutOnOff", S, LastSet.bUseTestSocket[0][i][j]);
                        else
                            TestMode.iDutOnOff[0][i][j]=ReadIniData(szDir, "DutOnOff_RT", S, LastSet.bUseTestSocket[0][i][j]);

                        if(IniConfig.bI35UseThirdSiteControlByEngineer)         //Steven 20241220 : add
                        {
                            if(iRunStartMode==FT)
                                TestMode.iDutOnOffEE[0][i][j]=ReadIniData(szDir, "DutOnOffEE", S, LastSet.bUseTestSocketEE[0][i][j]);
                            else
                                TestMode.iDutOnOffEE[0][i][j]=ReadIniData(szDir, "DutOnOffEE_RT", S, LastSet.bUseTestSocketEE[0][i][j]);
                        }
                    }
                    else
                    {
                        TestMode.iDutOnOff[0][i][j]=ReadIniData(szDir, "DutOnOff", S, LastSet.bUseTestSocket[0][i][j]);
                        if(IniConfig.bI35UseThirdSiteControlByEngineer)         //Steven 20241220 : add
                            TestMode.iDutOnOffEE[0][i][j]=ReadIniData(szDir, "DutOnOffEE", S, LastSet.bUseTestSocketEE[0][i][j]);
                    }
                }

                if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)  //Sam 20170515 (wei) TestIF 改為 TestIF_File 修正切換工作檔開關 Sit 的問題 //Steven 20170111 : 沒用到的Arm不存取資料
                {
                    TestMode.iDutOnOff[1][i][j]=false;                          //JerryYang 20251017 : 關ARM2 ATC應該要關SITE
                    TestMode.iDutOnOffEE[1][i][j]=false;
                }
                else
                {
                    S2.sprintf("Dut %s2", IndexSuckName[i][j]);
                    if(CosFunction.bFTRTDifferentDutOnOff==true)                //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
                    {
                        if(iRunStartMode==FT)
                            TestMode.iDutOnOff[1][i][j] =ReadIniData(szDir, "DutOnOff", S2, LastSet.bUseTestSocket[1][i][j]);
                        else
                            TestMode.iDutOnOff[1][i][j] =ReadIniData(szDir, "DutOnOff_RT", S2, LastSet.bUseTestSocket[1][i][j]);

                        if(IniConfig.bI35UseThirdSiteControlByEngineer)         //Steven 20241220 : add
                        {
                            if(iRunStartMode==FT)
                                TestMode.iDutOnOffEE[1][i][j]=ReadIniData(szDir, "DutOnOffEE", S, LastSet.bUseTestSocketEE[1][i][j]);
                            else
                                TestMode.iDutOnOffEE[1][i][j]=ReadIniData(szDir, "DutOnOffEE_RT", S, LastSet.bUseTestSocketEE[1][i][j]);
                        }
                    }
                    else
                    {
                        TestMode.iDutOnOff[1][i][j] =ReadIniData(szDir, "DutOnOff", S2, LastSet.bUseTestSocket[1][i][j]);
                        if(IniConfig.bI35UseThirdSiteControlByEngineer)         //Steven 20241220 : add
                            TestMode.iDutOnOffEE[1][i][j]=ReadIniData(szDir, "DutOnOffEE", S2, LastSet.bUseTestSocketEE[1][i][j]);
                    }
                }

                LastSet.bUseTestSocket[1][i][j]=TestMode.iDutOnOff[1][i][j];
                LastSet.bUseTestSocket[0][i][j]=TestMode.iDutOnOff[0][i][j];
                LastSet.bUseTestSocketEE[1][i][j]=TestMode.iDutOnOffEE[1][i][j];                                        //Steven 20241220 : add
                LastSet.bUseTestSocketEE[0][i][j]=TestMode.iDutOnOffEE[0][i][j];

                iCloseSiteMap[0][i][j]=TestMode.iDutOnOff[0][i][j];             //kevin 20161003 Site 順序
                iCloseSiteMap[1][i][j]=TestMode.iDutOnOff[1][i][j];             //kevin 20161003 Site 順序
            }
        }
    }
}
//---------------------------------------------------------------------------
//V3.27T.561 Ifor 20180212 (Steven) : add FTP DownLoad Temp Mode By SetupFile
//---------------------------------------------------------------------------
void SaveTempMode()
{
    if(InitialOK==false)
        return;

    if(CosFunction.bFTPDownLoadTempModeBySetupFile==false)
        return;

    AnsiString szDir=GetRecipeFileName("Temperature.Data");
    WriteIniData(szDir, "Mode",       "WorkTempMode", LastSet.iTemperature);
}
//---------------------------------------------------------------------------
void SaveTempModeByDLL()                                                        //JerryYang 20181126 (Steven) : support Epson DLL function
{
    if(InitialOK==false)
        return;

    if(CosFunction.bDLLCommands==false)
        return;

    AnsiString szDir=GetRecipeFileName("Temperature.Data");
    WriteIniData(szDir, "Mode",       "Mode", Temperature.iMachineTempMode);
}
//------------------------------------------------------------------------------
//  客戶功能選擇區
//------------------------------------------------------------------------------
// AI(W906-PT-W3-ungate) 20260808: ALL SEVEN `#if 0` GATES IN THIS FUNCTION ARE GONE.
//   Every one of them said "XXX() has ZERO bodies tree-wide (CosFunction
//   customer-function wave untranslated)".  That premise died when
//   `CosFunction.cpp` landed in PT-W3: the seven bodies are
//     InitialCosFunction   CosFunction.cpp:4063     KoreaFunction      :2854
//     VTEST_Funtion        CosFunction.cpp:2484     SingaporeFunction  :2927
//     SPILFunction         CosFunction.cpp:3172     MaximFunction      :3314
//     SIGURDFunction       CosFunction.cpp:3635
//   and `CosFunction.cpp` is registered in ht9045_globals -- the SAME archive as
//   this file (CMakeLists.txt), so there is no link-order question either.
//   Declarations were already visible with no new include: `cprod.h:7` includes
//   `CosFunction.h` (which declares six of the seven at :489-494) and
//   `cprod.h:3282` declares `SingaporeFunction()` itself -- exactly the shape
//   golden has, where cprod.cpp also does NOT include CosFunction.h directly.
//
//   ⚠ THIS IS A BEHAVIOUR CHANGE, AND A WIDE ONE.  It is the root cause WB-1
//   measured as "IniConfig's hundreds of feature flags are all 0"
//   (see database.cpp:316's note: 140 distinct flags read at 1,091 sites, nothing
//   setting them).  `InitialCosFunction()` is not a small helper -- it is the
//   several-hundred-assignment function that establishes every flag's DOCUMENTED
//   DEFAULT, and those defaults are NOT all false.  Ones that become true/non-zero
//   the moment this runs include `IniConfig.bEventLogAutoSaveFunction`,
//   `bIndexJamInArmAway`, `bChangeTempAutoSetDown`, `bAlarmNeedServoOff`,
//   `bUseFix3`, `bLastLoaderAutoCleanOut`, `bEnableInOutArmPlaceSkipSuckDetect`,
//   `bAbnormalStartCheck`, `CosFunction.bUseTrayUpDownSet`,
//   `CosFunction.bLastSetInSetUpFile`, and `iTempeAlarmSecond_Over=30` /
//   `iTempeAlarmSecond_Below=40`.  Until now every one of those read as 0/false.
//
//   ORDERING, OBSERVED NOT ASSUMED (and left exactly as golden has it): this
//   function runs BEFORE config.ini is read.  `database.cpp:324` calls
//   `CustomerFunctionSelect()` and then `ReadLastSetIni()`, and `ReadLastSetIni()`
//   (this file, :3092) calls it AGAIN at :3103 before `ReadLastDataFile()` and
//   before the `CheckAndReadIniDataGeneral` block at :3106+.  So on both passes
//   `InitialCosFunction()` has just reset `IniConfig.bKoreaFunction` and friends to
//   false, which means the six customer-profile branches below cannot fire from a
//   config.ini value on that pass.  That is golden's own sequence -- NOT repaired
//   here, and deliberately not "improved": the customer profile that does fire in
//   golden comes from CUSTOMER_CODE inside these bodies, not from these six flags.
void CustomerFunctionSelect()
{
    InitialCosFunction();                                                       //Steven 20240926 : 重新整理客戶功能

    if(USE_AUTO_RETEST==eartInstall)                                            //ChungHung 20141002 add for KYEC AutoRetest
        CosFunction.bOffLineBin=true;

    if(IniConfig.bIndexArm2SupplyLight==true ||                                 //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
       TestIF_File.bForEgisTecTest==true     ||                                 //Steven 20140922 : Arm2當作指紋測試
       (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                         //kevin 20150127 Arm1 下壓 arm2 測試
        TestIF_File.bArm1PickPlaceArm2Test==true))                              //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
    {
        IniConfig.bShuttleMode50=false;                                         //與 IniConfig.bIndexArm2SupplyLight 功能互斥
    }

    if(IniConfig.bKoreaFunction==true)
    {
        KoreaFunction();
    }

    if(IniConfig.bVTESTFunction==true)
    {
        VTEST_Funtion();
    }

    if(IniConfig.bSingaporeFunction)                                            //Steven 20120910 : 新加坡代理商的需求
    {
        SingaporeFunction();
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        SPILFunction();
        if(CUSTOMER_CODE==CC_XINYUN)                                            //Steven 20230222 : 要可以拉動
            IniConfig.bShowFormByInitPos=false;
    }

    if(IniConfig.bMaximFunction==true)                                          //JerryYang 20190522 Maxim統一軟體功能
    {
        MaximFunction();
    }

    if(IniConfig.bSIGURDFunction==true)                                         //KaiChen 20200506 ：矽格統一軟體功能
    {
        SIGURDFunction();
    }

    if(USE_ROTATE_KIT)                                                          //kevin rotate motor     //Steven 20131202
        IniConfig.bHaveRotateShuttle=false;

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //Steven 20170424 (wei) : new XY變距  //JerryYang 20251218 : IN/OUT ARM支援不同模組
        IniConfig.bDisableSelectSearchLast=true;

    if(CosFunction.bHiSiliconFunction ||                                        //Ifor 20160516 京元要求銦片 Life Time 功能
       (IniConfig.bVTESTFunction && ATC_SYSTEM > eATCUninstall))                //AI(ht9045-config) 20260507 (RogerYang) : VTEST銦片LifeTime功能
    {
        CosFunction.bUseHeadContactCount=true;
//        CosFunction.bHeadContactCountByRecipe=true;                             //Steven 20241030 : 銦片 Life Time 功能by工作檔  //RogerYang 20260515 : dead flag, 未被任何邏輯讀取
    }

    if(AUTO3_IS_MAGAZINE==1)                                                    //JerryYang 20230515 : P27跟Magazine衝突，先不使用
    {
        CosFunction.bSortingBinTraywhenCleanOut=false;
        CosFunction.bSortingBinTrayWhenTrayFeed=false;
    }

    if(REAL_TIME_CCD==true)                                                     //Steven 20110705 : 使用Real CCD要強制關內建的
    {                                                                           //Ifor 20150720 :放在最下面
        IniConfig.bEnableCCDUSETCPIP=false;
        if(!COM2->bCCDDummyRum)                                                 //Steven 20230109 : RTC沒開Index就不要動
            IniConfig.bShuttleMode50=true;                                      //暫時先打開，等RTC關Arm功能改好
        else
            IniConfig.bShuttleMode50=false;
    }
    else
    {
        IniConfig.bShuttleMode50=false;
    }

    CosFunction.bFullTrayAlarmAfterUnloadEnd=fAGV->IsSPIL_AMR();                //Steven 20251216 : for AMR

    if(bCanRunSCKART==true)                                                     //Steven 20170707 (wei) : Add for SCK ART
    {
        IniConfig.bRecordSkipPosition=true;
    }

    if(IniConfig.bShowLotInfo ||
       CosFunction.bFTPFunction ||
       USE_AUTO_RETEST==eartInstall)                                            //kevin 20150819 add ART畫面定位
    {
        IniConfig.bShowLotInfo=true;
    }

    if(REAL_TIME_CCD || (ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber))         //20141204 ChungHung add for ATC3.0 //2014-05-30    Dell    for ATC6.0
    {
        IniConfig.bShowLotInfo=true;
    }

    if(IniConfig.bEnableRms || IniConfig.bEnableErms || CUSTOMER_CODE==CC_SCK)  //ChungHung 20130621 add SCK RMS
    {
        IniConfig.bShowLotInfo=true;
    }

    if(CosFunction.bEnable_SECS_GEM)
    {
        if(IniConfig.bEnable_SECS_GEM)
        {
            IniConfig.bShowLotInfo=true;
        }
    }

    if(IniConfig.bSocketCommunication)                                          //ChungHung 20130112 add for ASE_KR Socket Tester
    {
        IniConfig.bShowLotInfo=true;
    }

    if(INSTALL_OCR)
    {
        IniConfig.bShowLotInfo=true;
    }

    if(IniConfig.bI28_OnOffSiteOnTheFly ||                                      //Steven 20150924 : 隨時開關Site功能
       (CosFunction.bLowYieldAutoSiteOff && iRunStartMode==FT &&
        (TestIF_File.bLowYieldAutoSiteOff ||                                    //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
         TestIF_File.bLowYieldAutoSiteOffByContiFail)))                         //Steven 20200420 : Continue fail, auto site off
    {
        bCanAutoCloseSite=true;                                                 //Steven 20200420 : 整合Auto Site Off
    }
    else
    {
        bCanAutoCloseSite=false;
    }

#if 0 // TODO(GA1-B2): blocked by fTemp_Set@not declared anywhere in ported tree (golden   [AI(W906-POOL2) 20261008: STALE -- fTemp_Set IS ported (forms/fTemp_Set.h / uTemp_Set.cpp); the real blocker is vclcompat::TRadioGroup without Controls[] (per-item Visible, golden cprod.cpp:3785-3797)]
      // form, no forms/fTemp_Set.h port exists)
    if(fTemp_Set!=NULL)                                                         //Steven 20240206 : 預先決定Index Heat Mode是否要顯示
    {
        bool bOldStatus=fTemp_Set->rgIndexHeatMode->Enabled;
        fTemp_Set->rgIndexHeatMode->Enabled=true;
        fTemp_Set->rgIndexHeatMode->Controls[HeadOnly          ]->Visible=true;
        fTemp_Set->rgIndexHeatMode->Controls[ChamberOnly       ]->Visible=(ATC_SYSTEM==eATCUninstall);
        fTemp_Set->rgIndexHeatMode->Controls[HeadChamber       ]->Visible=(ATC_SYSTEM==eATCUninstall && IniConfig.bNoHeadaddChamberOption==false);
        fTemp_Set->rgIndexHeatMode->Controls[SocketChamber     ]->Visible=(ATC_SYSTEM==eATCUninstall);
        fTemp_Set->rgIndexHeatMode->Controls[HeadSocket        ]->Visible=(IniConfig.bHeadSocketMode);                  //jou 2012-05-30 增加 Head + Socket Mode
        fTemp_Set->rgIndexHeatMode->Controls[HeadChamberSocket ]->Visible=(ATC_SYSTEM==eATCUninstall && IniConfig.bHeadChamberSocketMode);                      //2013-11-20    Dell    for TSMC Add Chamber + Head +Socket
        fTemp_Set->rgIndexHeatMode->Enabled=bOldStatus;
    }
#endif // TODO(GA1-B2): fTemp_Set
}
//------------------------------------------------------------------------------
AnsiString asInArmAOAFileName[TotalInArmAOAType]={"D:\\HT9045\\System\\AutoTeach_Loader.dat",
                                                  "D:\\HT9045\\System\\AutoTeach_Plate1.dat",
                                                  "D:\\HT9045\\System\\AutoTeach_Plate2.dat",
                                                  "D:\\HT9045\\System\\AutoTeach_Shuttle1.dat",
                                                  "D:\\HT9045\\System\\AutoTeach_Shuttle2.dat",
                                                  "D:\\HT9045\\System\\AutoTeach_AutoClean.dat"};

AnsiString asInArmAOAFileName_Cal[TotalInArmAOAType]={"D:\\HT9045\\System\\AutoTeach_Loader_Cal.dat",
                                                      "D:\\HT9045\\System\\AutoTeach_Plate1_Cal.dat",
                                                      "D:\\HT9045\\System\\AutoTeach_Plate2_Cal.dat",
                                                      "D:\\HT9045\\System\\AutoTeach_Shuttle1_Cal.dat",
                                                      "D:\\HT9045\\System\\AutoTeach_Shuttle2_Cal.dat",
                                                      "D:\\HT9045\\System\\AutoTeach_AutoClean_Cal.dat"};
//------------------------------------------------------------------------------
bool ReadAutoTeachTable_InArm()                                                 //KenHsieh 20211208 : In/Out Arm分開讀檔，避免InArm做完直接寫入導致OutArm點位錯誤
{
    for(int i=0; i<TotalInArmAOAType; i++)                                      //JerryYang 20241119 : fix AOA
    {
        //AI(GA1-B2) 20260804: const_cast -- ReadAutoTeachTable's cprod.h decl takes
        //  char* (non-const); cprod.h is read-only, so the fix is at the call site.
        ReadAutoTeachTable(const_cast<char*>(asInArmAOAFileName[i].c_str()), &InputAtuoTeachTable[i]);
    }

    for(int i=0; i<TotalInArmAOAType; i++)
    {
        ReadAutoTeachTable(const_cast<char*>(asInArmAOAFileName_Cal[i].c_str()), &InputAtuoTeachTableCal[i]);
    }

    return true;
}
//------------------------------------------------------------------------------
AnsiString asOutArmAOAFileName[TotalOutArmAOAType]={"D:\\HT9045\\System\\AutoTeach_Auto1.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Auto2.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Auto3.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Auto4.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Auto5.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Auto5.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Fix1.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Fix2.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Fix3.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Fix4.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Fix5.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_Fix6.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_OutShuttle1.dat",
                                                    "D:\\HT9045\\System\\AutoTeach_OutShuttle2.dat"};

AnsiString asOutArmAOAFileName_Cal[TotalOutArmAOAType]={"D:\\HT9045\\System\\AutoTeach_Auto1_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Auto2_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Auto3_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Auto4_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Auto5_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Auto5_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Fix1_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Fix2_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Fix3_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Fix4_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Fix5_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_Fix6_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_OutShuttle1_Cal.dat",
                                                        "D:\\HT9045\\System\\AutoTeach_OutShuttle2_Cal.dat"};
//------------------------------------------------------------------------------
bool ReadAutoTeachTable_OutArm()                                                //KenHsieh 20211208 : In/Out Arm分開讀檔，避免InArm做完直接寫入導致OutArm點位錯誤
{
    for(int i=0; i<TotalOutArmAOAType; i++)
    {
        //AI(GA1-B2) 20260804: const_cast, same reasoning as ReadAutoTeachTable_InArm above.
        ReadAutoTeachTable(const_cast<char*>(asOutArmAOAFileName[i].c_str()), &OutputAtuoTeachTable[i]);
    }

    for(int i=0; i<TotalOutArmAOAType; i++)
    {
        ReadAutoTeachTable(const_cast<char*>(asOutArmAOAFileName_Cal[i].c_str()), &OutputAtuoTeachTableCal[i]);
    }
    return true;
}
//---------------------------------------------------------------------------
bool ReadAutoTeachTable(char* cFName, AUTOTEACH_POINT* DataBuf)
{
    if(FileExists(AnsiString(cFName))==false)
        return false;

    FILE *Fp=fopen(cFName, "rb");
    if(Fp!=NULL)
    {
        fread(DataBuf, sizeof(AUTOTEACH_POINT), 1, Fp);
        fclose(Fp);
        return true;
    }
    else
    {
        //strcat(str,"AutoTeach File open error !");
        //ShowRecordMessage(str);
    }
    return false;
}
//------------------------------------------------------------------------------
bool WriteAutoTeachTable(char* cFName, AUTOTEACH_POINT* DataBuf)
{
    FILE *Fp=fopen(cFName, "wb");
    if(Fp!=NULL)
    {   fwrite(DataBuf, sizeof(AUTOTEACH_POINT), 1, Fp);
        fclose(Fp);
        return true;
    }
    else
    {
    }
    return false;
}
//---------------------------------------------------------------------------
void MyIndexArmRec::Clear()
{
    iStartTest=0;                                                               //Steven 20250102 : fixed for [I54]
    bArmTempErr=false;
    for(int iR=0; iR<MAX_SOCKET_ROW; iR++)
    {
        for(int iC=0; iC<MAX_SOCKET_COL; iC++)
        {
            bSuckTempErr[iR][iC]=false;
        }
    }
}
//------------------------------------------------------------------------------
void MyIndexArmRec::SetStartTest(int iArm)
{
    Clear();
    iStartTest=iArm;
}
//------------------------------------------------------------------------------
void MyIndexArmRec::SetTempErr(int iSuckRow, int iSuckCol)
{
    if(iSuckRow<MAX_SOCKET_ROW &&
       iSuckCol<MAX_SOCKET_COL &&
       iStartTest!=0)
    {
        bArmTempErr=true;
        bSuckTempErr[iSuckRow][iSuckCol]=bArmTempErr;
    }
}
//------------------------------------------------------------------------------
bool MyIndexArmRec::GetArmTempErr()
{
    if(CosFunction.bCheckTempDuringIndexArmTesting==false ||                    //Steven 20250102 : fixed for [I54]
       IniConfig.bI54_Enable==false)
    {
        return false;
    }
    else
    {
        return bArmTempErr;
    }
}
//------------------------------------------------------------------------------
bool MyIndexArmRec::GetSuckTempErr(int iSuckRow, int iSuckCol)
{
    return bSuckTempErr[iSuckRow][iSuckCol];
}
//------------------------------------------------------------------------------
void __fastcall TAlarm1::SummarizeJAMreportbymonth()                            //Stevenhong 20260318 : TESNA 把Eventlog report summarize by month
{
    Word y, m, d;
    DecodeDate(Now(), y, m, d);
    AnsiString sourcePath;
    sourcePath.sprintf("D:\\HT9045_Log\\EventLogTxt\\%04d\\EventLogTxt_%04d%02d.csv", y, y, m);

    if(!FileExists(sourcePath))
        return;

    TStringList *fileData = new TStringList();
    fileData->LoadFromFile(sourcePath);

    int RowCount = fileData->Count;
    std::vector<TAlarm1> JamAlarms;
    TStringList *rowDetail = new TStringList();

    for(int i = 1; i < RowCount; i++)
    {
        rowDetail->CommaText = fileData->Strings[i];
        if (rowDetail->Count > 3)
        {
            //AI(GA1-B2) 20260804: AnsiString(...) wrap -- vclcompat::StringsProxy (the
            //  Strings[] indexer's return type) has no .Pos() of its own; it does carry
            //  operator AnsiString() (vclcompat/TStringList.h:93), so an explicit
            //  construction gets a real AnsiString to call .Pos() on (AnsiString.h:104-106).
            if (AnsiString(rowDetail->Strings[3]).Pos("JAM") == 1)
            {
                TAlarm1 rowData;
                rowData.FullRow = fileData->Strings[i];
                JamAlarms.push_back(rowData);
            }
        }
    }
    delete rowDetail;
    AnsiString dirPath;
    dirPath.sprintf("D:\\HT9045_Log\\EventLogTxt\\%4d\\JAM_Log\\%02d", y, m);
    ForceDirectories(dirPath);
    AnsiString savePath = dirPath + "\\Processed_JAM.csv";
    if (RowCount > 0)
    {
        WriteDataToFile(savePath, fileData->Strings[0], true);
    }
    for(unsigned int j=0; j<JamAlarms.size(); j++)
    {
        WriteDataToFile(savePath, JamAlarms[j].FullRow, false);
    }

    delete fileData;
}
//------------------------------------------------------------------------------
//AI(GA1-B2) 20260804: (was the close of W0-TAIL's single big gate; retired --
//  see the matching banner at this file's ARM_OFFSET ctor for the new scheme)
//------------------------------------------------------------------------------
// AI(W906-LASTDATA-TESTDIR) 20260926: lastdata*.dat 的路徑鉤子 —— **只有 ctest 會轉向**。
//   golden 把 lastdata.dat／lastdata_backup.dat／lastdata_backup2.dat 寫死在 D:\HT9045\system\（上面 15 處），而 WriteLastDataFile 另外一定會寫 AuthPath+"config.ini"（:2085；Vibrate_Time、P65_QAMode、SocketContact、O_Count 接觸壽命計數）。
//   UpdateMainOperateMode 翻活之後，開機、讀配方、切換運轉模式都會走到 WriteLastDataFile，而 ctest -j8 並行跑，
//   沒有這個鉤子就會把機台真實的最後狀態（開著的工單、計數）蓋成測試的值（0911 事故同一類）。
//   怎麼轉：tests/test_bootstrap.cpp（只連進測試執行檔）在 main() 之前設環境變數
//   W906_CTEST_LASTDATA_DIR_<本行程 PID>＝它自己的沙盒目錄；變數名綁 PID，所以外面的人設不到正式的 wb_serve
//   （跟 --dry 退場的理由同一件事：正式程式不能有「看起來存成功、其實寫到別處」的執行期開關）。
//   沒被轉向時原封回傳 golden 的字串（連 d:／D: 大小寫都不變）；被轉向時回「沙盒目錄＋檔名」。
AnsiString W906_LastDataPath(const char* goldenPath)
{
    char name[64];
    std::sprintf(name, "W906_CTEST_LASTDATA_DIR_%lu", static_cast<unsigned long>(GetCurrentProcessId()));
    char dir[MAX_PATH];
    const DWORD n = GetEnvironmentVariableA(name, dir, sizeof(dir));
    if (n == 0 || n >= sizeof(dir))
        return AnsiString(goldenPath);
    const char* leaf = std::strrchr(goldenPath, '\\');  { const char* f = std::strrchr(goldenPath, '/'); if (f != 0 && (leaf == 0 || f > leaf)) leaf = f; }   //AI(W906-ENV-ALL) 20260926: '/' too -- W906_AUTH_PATH is a forward-slash CMake path, so (AuthPath+"config.ini") had no backslash and the whole path was glued onto the sandbox dir (LastDataSandbox :84 caught it once W906-ENV-ALL gave that test the roots)
    return AnsiString(dir) + AnsiString(leaf ? leaf + 1 : goldenPath);
}
