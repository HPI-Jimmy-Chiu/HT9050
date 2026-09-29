// =============================================================================
//  Public/MyProductionRecord.cpp
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/Public/MyProductionRecord.cpp
//  Translation wave: AI(W906-MyProductionRecord) 20260721
//
//  SCOPE THIS WAVE (golden .cpp:218-654, ~34 methods): the ctor,
//  InitialRecord, DeleteProductionRecord, AddArmSiteRecord, and the
//  Add*Record/Get* leaves that purely read/write asBuffer (TStringList*,
//  indexed by eMyProdRec, Public/MyProductionRecord.h). Every external
//  symbol this slice touches was re-verified against the CURRENT target
//  tree (not assumed from an earlier recon) -- see the per-method notes
//  below for anything that turned out different from plan.
//
//  OUT OF SCOPE (golden .cpp:660-1384, left undeclared in the header --
//  see its own file-head banner): SaveRecord/SaveDataForASE/
//  SaveDataForGreatek/SaveGroundESDData/SaveRecordCleanPad/
//  SaveRecordForLeadYo/Save2DIDForJCET/SaveJHT_Record/GetProdLogFileName/
//  GetProdLogFilePath. AddIndexPickVacuum (golden .cpp:1110) also stays out
//  (physically inside that heavy block; see header note).
//
//  AddErrorRecord (golden .cpp:561-573) IS declared in the header (pre-
//  existing) but is DELIBERATELY NOT DEFINED here. Golden's body is:
//      if(bAddPosition) { ...5 asBuffer writes... }
//      SaveRecord(ErrorLog);
//  The position-recording half is pure/in-scope; the tail call is into
//  SaveRecord(), which is the OUT-OF-SCOPE heavy half above. This mirrors
//  the codebase's own established "Hard boundary" precedent for a leaf that
//  calls into a still-deferred sibling -- see atester_32Site.cpp's
//  W5_32S_ADD_TEST_RESULT_RECORD/W5_32S_SEAM ("DECLARED, no body anywhere ->
//  would be undefined-reference") and aTester_Rear.cpp:2022/2307 +
//  BarCode_Bottom2DID8CCD.cpp:1372's "#if 0 // TODO(W7) ... (TMyProductionRecord
//  has no .cpp yet)" call-site gates for this exact same method. Per this
//  wave's own brief, the choice here is the SECOND of the two offered options
//  ("gating just this one method alongside the heavy half") rather than
//  inventing a forward-declared SaveRecord() no-op stub: SaveRecord has ZERO
//  declaration anywhere in the target header today, and adding one (even as
//  a temporary stub) would prematurely stake out the heavy half's surface
//  that this wave's brief explicitly says to leave untouched. Leaving
//  AddErrorRecord itself undefined keeps that boundary exactly as crisp as
//  before (same undefined-symbol status quo for its existing unguarded call
//  sites in acarry.cpp/csystem.cpp/etc. -- no better, no worse). Its sibling
//  AddErrorRecordNoSave (golden .cpp:575-587, no SaveRecord call) has NO such
//  problem and gets a real body below.
//
//  KNOWN CONCURRENT-WAVE COLLISION -- READ BEFORE WIRING INTO ANY CMAKE
//  TARGET: a same-day sibling wave, AI(W906-AutoCleanFoundation) 20260721,
//  independently added its OWN no-op stand-in DEFINITIONS for 9 of these
//  same methods directly inside aHotPlateSubstrate.cpp (verified via git diff
//  while this wave was in flight):
//      TMyProductionRecord::TMyProductionRecord()            (allocates asBuffer only)
//      TMyProductionRecord::AddErrorRecord(...)               (no-op)
//      TMyProductionRecord::AddHPRecord(...)                  (no-op)
//      TMyProductionRecord::AddInArmHotplatePickRecord(...)   (no-op)
//      TMyProductionRecord::AddTestRecord(...)                (no-op -- see below, INTENTIONALLY not redefined here)
//      TMyProductionRecord::GetInRotationAngRecord()          (returns "")
//      TMyProductionRecord::AddPickCleanPad(...)              (no-op)
//      TMyProductionRecord::AddPickCleanPadFormShuttle(...)   (no-op)
//      TMyProductionRecord::AddPlaceCleanPad(...)             (no-op)
//  This TU provides REAL bodies for the ctor and 7 of those 8 remaining
//  methods (AddErrorRecord excepted, see above) because they are genuinely
//  in this wave's scope and behaviourally different from a no-op (e.g.
//  GetInRotationAngRecord must return the recorded angle, not "").  That
//  means linking this .cpp together with aHotPlateSubstrate.cpp WILL produce
//  "multiple definition" errors for those 7 symbols + the ctor until a
//  human/integrate-agent deletes the aHotPlateSubstrate.cpp stand-ins (see
//  that file's :472-491 for the exact lines). This .cpp is therefore NOT
//  wired into any shared CMake target by this wave -- see tests/CMakeLists.txt
//  for the isolated verification target used instead, and the wave's own
//  report for the full reconciliation recommendation.
//  AddTestRecord is the ONE exception left untouched: golden itself never
//  gave it a body (golden .h:19 declares it fully commented-out; golden .cpp
//  :305-309 is the matching commented-out shell) -- an empty no-op IS the
//  faithful translation, so the pre-existing aHotPlateSubstrate.cpp stand-in
//  already is the correct, final answer; redefining it here would be a
//  needless duplicate, not a fix.
//
//  DEPENDENCY VERIFICATION (re-checked against the CURRENT target tree,
//  20260721 -- flagging every place this differs from the task brief's
//  assumptions):
//    * CUSTOMER_CODE/CC_KYEC_LEE/CC_Greatek/CC_HANA_MICRON -- CUSTOMER_CODE
//      is `extern int` in cmydef.h:3181; the CC_* constants are #defines in
//      MachineType.h (NOT cmydef.h/cprod.h as the brief assumed) --
//      MachineType.h:230/292/330.
//    * GetDateInfoByString/GetOnlyTimeInfoByString -- real, cpublic.h:49-50 /
//      cpublic.cpp, matches brief.
//    * TestSocket, TestIF_File.iGpibMode, iTestBinCount -- real.  TestSocket
//      is `extern TMyKitSuck TestSocket;` in aHotPlateSubstrate.h:389 (NOT
//      MyKitSuck.h -- that file does not exist in the target tree; the
//      target's substrate lives in aHotPlateSubstrate.h/.cpp instead).
//      TestIF_File is `extern SYSTEM_TEST_IF TestIF_File;` (cprod.h:2577,
//      iGpibMode member at cprod.h:1673).  iTestBinCount: cmydef.h:3396.
//    * InArmSuck/OutArmSuck .Suck[][].sName -- real (aHotPlateSubstrate.h),
//      matches brief.  .Suck[][].GetIOValue() -- DOES NOT EXIST on the
//      target's TMySucker (aHotPlateSubstrate.h:106-160+; golden MyKitSuck.h
//      :108).  This is a genuine gap the brief's recon missed -- see the
//      per-method "Hard boundary" gate on AddInArmLoaderPickRecord/
//      AddOutArmShuttlePickRecord/AddInArmHotplatePickRecord below (matches
//      atester_32Site.cpp's own already-established W5_32S_ADD_INDEX_PICK_VACUUM
//      workaround for this exact same missing method).
//    * IndexSuckName[][]/iOneTrayPickCount[] -- real, cmydef.h:3239/5147,
//      matches brief.
//    * LoadForm/HotPlateForm -- real.  LoadForm is `extern TRAY_TYPE_PARA
//      *LoadForm;` (a POINTER, cprod.h:1365) -- golden calls it via `->`,
//      matches. HotPlateForm is `extern TRAY_TYPE_PARA HotPlateForm;` (BY
//      VALUE, cprod.h:1361) -- golden calls it via `.`, matches.
//    * CosFunction.bRecordGroundESDByTestIC -- real, CosFunction.h:351
//      (pulled in transitively via cprod.h:7's `#include "CosFunction.h"`).
//    * GetTimeInfo() -- real, cpublic.h:23/cpublic.cpp (fills the SystemHour/
//      SystemMin/SystemSec/SystemMSec globals below via DecodeTime).
//    * SystemHour/SystemMin/SystemSec/SystemMSec -- real, `extern Word ...` in
//      cmydef.h:226 (the brief did not call these out explicitly but the
//      AddPickCleanPad/AddPlaceCleanPad* family needs them).
//    * Now().FormatString("yyyy/mm/dd") -- golden BCB6 TDateTime member call.
//      vclcompat's TDateTime has NO FormatString method; the established,
//      repeatedly-used precedent elsewhere in this tree (ainarm9045.cpp:890,
//      Automation/SCK_ART_Remainder.cpp:1282, SECSGEM/uHGemClass.cpp:2787) is
//      `FormatDateTime(fmt, Now())` -- used the same way below.
// =============================================================================
#include "Public/MyProductionRecord.h"

#include "vclcompat/vcl_compat.h"   // AnsiString, TStringList, Now(), FormatDateTime
#include "MachineType.h"            // CC_KYEC_LEE / CC_Greatek / CC_HANA_MICRON
#include "cprod.h"                  // LoadForm, HotPlateForm, TestIF_File, CosFunction (via CosFunction.h)
#include "cmydef.h"                 // CUSTOMER_CODE, IndexSuckName[][], iOneTrayPickCount[],
                                     // iTestBinCount, SystemHour/Min/Sec/MSec
#include "cpublic.h"                 // GetDateInfoByString/GetOnlyTimeInfoByString/GetTimeInfo
#include "aHotPlateSubstrate.h"     // TMyKitSuck/TMySucker + InArmSuck/OutArmSuck/TestSocket externs

#include <cstdlib>                  // atoi

//------------------------------------------------------------------------------
//Steven 20160113 : 記錄每一顆IC 狀態
//------------------------------------------------------------------------------
TMyProductionRecord::TMyProductionRecord()
{
    asBuffer = new TStringList();
    for(int i=eScheduleName; i<eDataTotal; i++)
    {
        asBuffer->Add("");                                                      //kevin 20180129 add
    }
    InitialRecord();

    asDataTitleGreatek=AnsiString("Schedule name, ")                 +   //eScheduleName         =0         //Sam 20200707 : ProductionRecord for Greatek
                       AnsiString("Start time, ")                    +   //eStartTime            =1
                       AnsiString("Input tray, ")                    +   //eLoadNum              =2
                       AnsiString("Tray Form, ")                     +   //eTrayForm             =41     //Sam 20210907 : Add record
                       AnsiString("In X, ")                          +   //eLoadX                =3
                       AnsiString("In Y, ")                          +   //eLoadY                =4
                       AnsiString("In Time, ")                       +   //eLoadTime             =5
                       AnsiString("In Arm Loader Pick, ")            +   //eInArmLoaderPick      =30
                       AnsiString("Hotplate No,")                    +   //eHotplateNo           =35     //Sam 20200716 : Add record
                       AnsiString("Hotplate Form, ")                 +   //eHotPlateForm         =42     //Sam 20210907 : Add record
                       AnsiString("Hot X, ")                         +   //eHotX                 =6
                       AnsiString("Hot Y, ")                         +   //eHotY                 =7
                       AnsiString("Hot Time, ")                      +   //eHotTime              =8
                       AnsiString("In Arm Hotplate Pick, ")          +   //eInArmHotplatePick    =36     //Sam 20200716 : Add record
                       AnsiString("2DCode, ")                        +   //e2DCode               =9
                       AnsiString("In Arm Place Shuttle Site No, ")  +   //eSiteNO               =10    //1-32 //Site No -> In Arm Place Shuttle Site No
                       AnsiString("Arm No, ")                        +   //eArmNO                =11    //0: arm1  1: arm2
                       AnsiString("Index Pick Shuttle Site No, ")    +   //eIndexPickSHTSiteNo   =31
                       AnsiString("Arm Time, ")                      +   //eArmTime              =12
                       AnsiString("Order of testing, ")              +   //eOrderTest            =13    //touch-down
                       AnsiString("SOT time stamp, ")                +   //eSOTTime              =14
                       AnsiString("Index cycle time, ")              +   //eIndexCycleTime       =15    //第一顆測試 是 0 (1st device)
                       AnsiString("Test category, ")                 +   //eTestCate             =16    //測試 BIN碼
                       AnsiString("Test Mode, ")                     +   //eTestMode             =37
                       AnsiString("Index Place Shuttle Site No,")    +   //eIndexPlaceSHTSiteNo  =32
                       AnsiString("iWhichAuto, ")                    +   //eWhichAuto            =17
                       AnsiString("Out Shuttle Detect Site No,")     +   //eOutSHTDetectSiteNo   =33
                       AnsiString("Out Arm Shuttle Pick, ")          +   //eOutArmShuttlePick    =34
                       AnsiString("Output tray, ")                   +   //eOutTrayNum           =18    //Auto1 :O1-1  Auto2 :2-1  Auto3 :3-1  Fix 1:4-1 Fix 2:5-1 Fix 3:6-1
                       AnsiString("Out X, ")                         +   //eUnloadX              =19
                       AnsiString("Out Y, ")                         +   //eUnloadY              =20
                       AnsiString("Out Time, ")                      +   //eUnloadTime           =21
                       AnsiString("Out Arm X Pos, ")                 +   //eUnloadXPos           =22
                       AnsiString("Out Arm Y Pos, ")                 +   //eUnloadYPos           =23
                       AnsiString("Out Arm Time, ")                  +   //eUnloadYPos           =24
                       AnsiString("Error log, ")                     +   //eErrorCode            =25
                       AnsiString("OCR Code , ")                     +   //eOCRCode              =26
                       AnsiString("Test Time, ")                     +   //eTestTime             =27
                       AnsiString("TSD Time,   ")                    +   //eTSDTime              =28
                       AnsiString("EOT time stamp,")                 +   //eEOTTime              =29
                       AnsiString("Out Arm X pitch Pos, ")           + //eUnloadXpPos            =38  //JerryYang 20210119 : add x,y pitch位置
                       AnsiString("Out Arm X pitch2 Pos, ")          + //eUnloadXp2Pos           =39
                       AnsiString("Out Arm Y pitch Pos, ")           ; //eUnloadYpPos            =40

    //Ifor 20180510 : add 時間資訊
    //==>
    asDataTitle=AnsiString("Schedule name, ")   +       //eScheduleName   =0
                AnsiString("Start time, ")      +       //eStartTime      =1
                AnsiString("Input tray, ")      +       //eLoadNum        =2
                AnsiString("In X, ")            +       //eLoadX          =3
                AnsiString("In Y, ")            +       //eLoadY          =4
                AnsiString("In Time, ")         +       //eLoadTime       =5
                AnsiString("Hot X, ")           +       //eHotX           =6
                AnsiString("Hot Y, ")           +       //eHotY           =7
                AnsiString("Hot Time, ")        +       //eHotTime        =8
                AnsiString("2DCode, ")          +       //e2DCode         =9
                AnsiString("Site No, ")         +       //eSiteNO         =10    //1-32
                AnsiString("Arm No, ")          +       //eArmNO          =11    //0: arm1  1: arm2
                AnsiString("Arm Time, ")        +       //eArmTime        =12
                AnsiString("Order of testing, ")+       //eOrderTest      =13    //touch-down
                AnsiString("SOT time stamp, ")  +       //eSOTTime        =14    //kevin 20140918  yyyymmdd_hhmmss
                AnsiString("Index cycle time, ")+       //eIndexCycleTime =15    //第一顆測試 是 0 (1st device)
                AnsiString("Test category, ")   +       //eTestCate       =16    //測試 BIN碼
                AnsiString("iWhichAuto, ")      +       //eWhichAuto      =17
                AnsiString("Output tray, ")     +       //eOutTrayNum     =18    //Auto1 :O1-1  Auto2 :2-1  Auto3 :3-1  Fix 1:4-1 Fix 2:5-1 Fix 3:6-1
                AnsiString("Out X, ")           +       //eUnloadX        =19
                AnsiString("Out Y, ")           +       //eUnloadY        =20
                AnsiString("Out Time, ")        +       //eUnloadTime     =21
                AnsiString("Out Arm X Pos, ")   +       //eUnloadXPos     =22
                AnsiString("Out Arm Y Pos, ")   +       //eUnloadYPos     =23
                AnsiString("Out Arm Time, ")    +       //eUnloadYPos     =24
                AnsiString("Error log, ")       +       //eErrorCode      =25
                AnsiString("OCR Code , ")       +       //eOCRCode        =26
                AnsiString("Test Time, ")       +       //eTestTime       =27
                AnsiString("TSD Time,   ")      +       //eTSDTime        =28
                AnsiString("EOT time stamp, ")  +       //eEOTTime        =29     //wei 20181211 更換設定SOT
                AnsiString("In Arm Loader Pick, ")+     //eInArmLoaderPick=30     //Sam 20200623 : Add Suck record
                AnsiString("Index Pick Shuttle Site No, ")+ //eIndexPickSHTSiteNo  =31     //Sam 20200623 : Add record
                AnsiString("Index Place Shuttle Site No,")+ //eIndexPlaceSHTSiteNo =32     //Sam 20200623 : Add record
                AnsiString("Out Shuttle Detect Site No,") + //eOutSHTDetectSiteNo  =33     //Sam 20200623 : Add record
                AnsiString("Out Arm Shuttle Pick, ")      + //eOutArmShuttlePick   =34     //Sam 20200623 : Add record
                AnsiString("Hotplate No,")                + //eHotplateNo          =35     //Sam 20200716 : Add record
                AnsiString("In Arm Hotplate Pick, ")      + //eInArmHotplatePick   =36     //Sam 20200716 : Add record
                AnsiString("Test Mode, ")                 + //eTestMode            =37     //Sam 20201216 : Add record
                AnsiString("Out Arm X pitch Pos, ")       + //eUnloadXpPos         =38  //JerryYang 20210119 : add x,y pitch位置
                AnsiString("Out Arm X pitch2 Pos, ")      + //eUnloadXp2Pos        =39
                AnsiString("Out Arm Y pitch Pos, ")       + //eUnloadYpPos         =40
                AnsiString("Tray Form, ")                 + //eTrayForm            =41     //Sam 20210907 : Add record
                AnsiString("Hotplate Form, ")             + //eHotPlateForm        =42     //Sam 20210907 : Add record
                AnsiString("Tray ID, ")                   + //eTrayID              =43     //Steven 20220120 : Amlogic需要輸出SBIN
                AnsiString("SBin, ")                      + //eSBin                =44     //Steven 20220120 : Amlogic需要輸出SBIN
                AnsiString("Ground, ")                    + //eGround              =45     //Sam 20211223 : 每顆 IC 測試完成後要記錄測試當時的 Ground & ESD 數值。
                AnsiString("ESD, ")                       + //eESD                 =46     //Sam 20211223 : 每顆 IC 測試完成後要記錄測試當時的 Ground & ESD 數值。
                AnsiString("In Rotation Angle, ")         + //eInRotationAngle     =47     //Sam 20221103 : 新增In料旋轉角度
                AnsiString("Pick cleanpad data, ")              +       //ePickCleanPadData         =48 //Sam 20230616 : Add Auto Clean Record
                AnsiString("Pick cleanpad time, ")              +       //ePickCleanPadTime         =49
                AnsiString("Pick cleanpad suck, ")              +       //ePickCleanPadSuck         =50
                AnsiString("Pick cleanpad row, ")               +       //ePickCleanPadRow          =51
                AnsiString("Pick cleanpad col, ")               +       //ePickCleanPadCol          =52
                AnsiString("Place cleanpad to shuttle time, ")  +       //ePlaceCleanPadToShtTime   =53
                AnsiString("Place cleanpad to shuttle suck, ")  +       //ePlaceCleanPadToShtSuck   =54
                AnsiString("Place cleanpad to shuttle row, ")   +       //ePlaceCleanPadToShtRow    =55
                AnsiString("Place cleanpad to shuttle col, ")   +       //ePlaceCleanPadToShtCol    =56
                AnsiString("Pick cleanpad form shuttle time, ") +       //ePickCleanPadFormShtTime  =57
                AnsiString("Pick cleanpad form shuttle suck, ") +       //ePickCleanPadFormShtSuck  =58
                AnsiString("Pick cleanpad form shuttle row, ")  +       //ePickCleanPadFormShtRow   =59
                AnsiString("Pick cleanpad form shuttle col, ")  +       //ePickCleanPadFormShtCol   =60
                AnsiString("Place cleanpad time, ")             +       //ePlaceCleanPadTime        =61
                AnsiString("Place cleanpad suck, ")             +       //ePlaceCleanPadSuck        =62
                AnsiString("Place cleanpad row, ")              +       //ePlaceCleanPadRow         =63
                AnsiString("Place cleanpad col, ")              +       //ePlaceCleanPadCol         =64
                AnsiString("InArm Pick From Loader Vacuum, ")   +       //eLoaderPickVacuum         =65     //RogerYang 20251225 : 補上
                AnsiString("InArm Pick From Hotplate Vacuum, ") +       //eHPPickVacuum             =66     //RogerYang 20251225 : 補上
                AnsiString("OutArm Pick From Shuttle Vacuum, ") +       //eOSHTPickVacuum           =67     //RogerYang 20251225 : 補上
                AnsiString("Index Pick Vacuum, ")               +       //eIndexPickVacuum          =68     //RogerYang 20251225 : 補上
                AnsiString("In Arm X Pos, ")                    +       //eLoadXPos                 =69     //Sam 20250211 : Add Load Pos
                AnsiString("In Arm Y Pos, ")                    ;       //eLoadYPos                 =70
    //<==
    //Ifor 20180510 : add 時間資訊

    //Sam 20211223 : 每顆 IC 測試完成後要記錄測試當時的 Ground & ESD 數值。
    //==>
    asDataTitle_SG=AnsiString("EOT time stamp, ")   +       //eEOTTime     =29
                   AnsiString("2DCode, ")           +       //e2DCode      =9
                   AnsiString("Test category, ")    +       //eTestCate    =16    //測試 BIN碼
                   AnsiString("Test result, ")      +       //Pass=P , Fail=F
                   AnsiString("Ground, ")           +       //eGround      =45
                   AnsiString("ESD ")               ;       //eESD         =46
    //<==
    //Sam 20211223 : 每顆 IC 測試完成後要記錄測試當時的 Ground & ESD 數值。

    //Sam 20230616 : Add Auto Clean Record
    //==>
    asDataTitleCleanPad=AnsiString("Pick clean pad data, ")              +       //ePickCleanPadData         =48
                        AnsiString("Pick clean pad time, ")              +       //ePickCleanPadTime         =49
                        AnsiString("Pick clean pad suck, ")              +       //ePickCleanPadSuck         =50
                        AnsiString("Pick clean pad row, ")               +       //ePickCleanPadRow          =51
                        AnsiString("Pick clean pad col, ")               +       //ePickCleanPadCol          =52
                        AnsiString("Place clean pad to shttle time, ")   +       //ePlaceCleanPadToShtTime   =53
                        AnsiString("Place clean pad to shttle suck, ")   +       //ePlaceCleanPadToShtSuck   =54
                        AnsiString("Place clean pad to shttle row, ")    +       //ePlaceCleanPadToShtRow    =55
                        AnsiString("Place clean pad to shttle col, ")    +       //ePlaceCleanPadToShtCol    =56
                        AnsiString("Pick clean pad form shttle time, ")  +       //ePickCleanPadFormShtTime  =57
                        AnsiString("Pick clean pad form shttle suck, ")  +       //ePickCleanPadFormShtSuck  =58
                        AnsiString("Pick clean pad form shttle row, ")   +       //ePickCleanPadFormShtRow   =59
                        AnsiString("Pick clean pad form shttle col, ")   +       //ePickCleanPadFormShtCol   =60
                        AnsiString("Place clean pad time, ")             +       //ePlaceCleanPadTime        =61
                        AnsiString("Place clean pad suck, ")             +       //ePlaceCleanPadSuck        =62
                        AnsiString("Place clean pad row, ")              +       //ePlaceCleanPadRow         =63
                        AnsiString("Place clean pad col, ")              +       //ePlaceCleanPadCol         =64
                        AnsiString("Error log ")                         ;       //eErrorCode            =25
    //<==
    //Sam 20230616 : Add Auto Clean Record

    //RogerYang 20251208 : JCET 2D FT1組名稱/FT2測試碼
    asJCET2DIDList= AnsiString("2DID, ")                            +           //e2DCode           =9
                    AnsiString("SOFTBIN, ")                         +           //eSBin             =44
                    AnsiString("HARDBIN, ")                         +           //eTestCate         =16
                    AnsiString("SubLOTID, ")                        +
                    AnsiString("CustLOTID, ")                       +
                    AnsiString("Modecode, ")                        +
                    AnsiString("StartTime, ")                       +           //eSOTTime          =14
                    AnsiString("EndTime, ")                         +           //eEOTTime          =29
                    AnsiString("SITE, ")                            +           //eSiteNO           =10
                    AnsiString("In X, ")                            +           //eLoadX            =3
                    AnsiString("In Y, ")                            +           //eLoadY            =4
                    AnsiString("Out X, ")                           +           //eUnloadX          =19
                    AnsiString("Out Y, ")                           +           //eUnloadY          =20
                    AnsiString("iWhichAuto, ")                      +           //eWhichAuto        =17
                    AnsiString("PartID, ")                          ;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::DeleteProductionRecord()
{
    if(asBuffer!=0 && asBuffer!=NULL)
    {
        asBuffer->Clear();                                                      //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete asBuffer;
        asBuffer=NULL;                                                          //kevin 20170306
    }
}
//------------------------------------------------------------------------------
void TMyProductionRecord::InitialRecord()
{
    if(asBuffer->Count!=eDataTotal)
    {
        DeleteProductionRecord();
        // AI(W906-MyProductionRecord) 20260721: golden quirk, preserved
        // VERBATIM -- `TMyProductionRecord();` here is a bare function-style
        // statement, i.e. it constructs an unnamed TEMPORARY of this class and
        // immediately destroys it. It does NOT reinitialize `this` (a
        // placement-new or `*this = TMyProductionRecord()` would be needed for
        // that). Golden has carried this exact no-op-on-`this` bug since the
        // original Steven 20160113 commit; kept as-is per this wave's
        // "preserve golden bugs verbatim + comment, don't silently fix" rule.
        TMyProductionRecord();
    }

    for(int i=eLoadNum; i<eDataTotal; i++)
    {
        asBuffer->Strings[i]="";
    }
    bUse=false;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddLoadRecord(int iNum, int iR, int iC, int iXPos, int iYPos, AnsiString Name, AnsiString Time)
{
    AnsiString sTestMode="", asTrayForm="";

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_Greatek)                 //Sam 20200704 : Add Greatek
    {
        iC=iC+1;
        iR=iR+1;
    }
    AnsiString sTime=GetDateInfoByString("/")+" "+GetOnlyTimeInfoByString(":"); //Ifor 20180510 : add 時間資訊

    sTestMode.sprintf("%dx%d", TestSocket.iShtRow, TestSocket.iShtCol);
    asTrayForm.sprintf("%dx%d", LoadForm->XDivision, LoadForm->YDivision);      //Sam 20210907 : Add record

    asBuffer->Strings[eLoadNum      ]   ="I1-"+AnsiString(iNum);
    asBuffer->Strings[eLoadX        ]   =AnsiString(iC);
    asBuffer->Strings[eLoadY        ]   =AnsiString(iR);
    asBuffer->Strings[eLoadXPos     ]   =AnsiString(iXPos);                     //Sam 20250211 : Add Load Pos
    asBuffer->Strings[eLoadYPos     ]   =AnsiString(iYPos);
    asBuffer->Strings[eLoadTime     ]   =sTime;                                 //Ifor 20180510 : add 時間資訊
    asBuffer->Strings[eScheduleName ]   =Name;
    asBuffer->Strings[eStartTime    ]   =Time;
    asBuffer->Strings[eTestMode     ]   =sTestMode;                             //Sam 20201216 : Add record
    asBuffer->Strings[eTrayForm     ]   =asTrayForm;                            //Sam 20210907 : Add record
    bUse                                =true;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddHPRecord(int iPlate, int iR, int iC)
{
    AnsiString sHPNo="",asHotPlateForm="";

    if(iPlate==0)
        sHPNo="2";
    else if(iPlate==1)
        sHPNo="1";
    else
        sHPNo="na";

    asHotPlateForm.sprintf("%dx%d",HotPlateForm.XDivision,HotPlateForm.YDivision);  //Sam 20210907 : Add record

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_Greatek)                 //Sam 20200704 : Add Greatek
    {
        iC=iC+1;
        iR=iR+1;
    }

    AnsiString sTime=GetDateInfoByString("/")+" "+GetOnlyTimeInfoByString(":"); //Ifor 20180510 : add 時間資訊
    asBuffer->Strings[eHotX         ]   =AnsiString(iC);
    asBuffer->Strings[eHotY         ]   =AnsiString(iR);
    asBuffer->Strings[eHotTime      ]   =sTime;                                 //Ifor 20180510 : add 時間資訊
    asBuffer->Strings[eHotplateNo   ]   =sHPNo;                                 //Sam 20210907 : Add record
    asBuffer->Strings[eHotPlateForm ]   =asHotPlateForm;                        //Sam 20210907 : Add record
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddArmSiteRecord(int iArm, int iSite)                 //Steven 20201204 : 修正Site No在非標準Pitch的情況下有問題
{
    asBuffer->Strings[eArmNO]           =AnsiString(iArm);
    asBuffer->Strings[eSiteNO]          =AnsiString(iSite);
    AnsiString sTime=GetDateInfoByString("/")+" "+GetOnlyTimeInfoByString(":"); //Ifor 20180510 : add 時間資訊
    asBuffer->Strings[eArmTime]         =sTime;                                 //Ifor 20180510 : add 時間資訊
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddTestResultRecord(int iBin, AnsiString SBin, AnsiString ErrorLog)
{
    if(TestIF_File.iGpibMode==1)                                                //256 bin    kevin 20141031
    {
        if(iBin>=255)                                                           //JerryYang 20230322 : 最大只能255BIN
            asBuffer->Strings[eTestCate]="E";
        else
            asBuffer->Strings[eTestCate]=AnsiString(iBin);
    }
    else
    {
        if(iBin>=iTestBinCount)                                                 //JerryYang 20200812 : error bin統一記為"E"
        {
            asBuffer->Strings[eTestCate]="E";
        }
        else
        {
            asBuffer->Strings[eTestCate]=AnsiString(iBin);
        }
    }
    asBuffer->Strings[eSBin]            =SBin;                                  //Steven 20220120 : Amlogic需要輸出SBIN
    AddErrorLog(ErrorLog);
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddErrorLog(AnsiString ErrorLog)                      //Steven 20200518 : 避免重複ErrorLog不覆蓋
{
    if(ErrorLog!="")
    {
        if(asBuffer->Strings[eErrorCode]=="" || asBuffer->Strings[eErrorCode]=="na")
        {
            asBuffer->Strings[eErrorCode]=ErrorLog;
        }
        else
        {
            if(ErrorLog=="" || ErrorLog=="na")
            {
            }
            // AI(W906-MyProductionRecord) 20260721: golden `Strings[i].AnsiPos(...)`
            // -- vclcompat's StringsProxy has no passthrough AnsiString methods
            // (read-only via its conversion operator); GetString(i) returns the
            // plain AnsiString by value so AnsiPos chains -- established
            // precedent, see mycylin.cpp:826/844's identical GetString(i).c_str() fix.
            else if(asBuffer->GetString(eErrorCode).AnsiPos(ErrorLog)==0)         //避免記入重複的Error log
            {
                asBuffer->Strings[eErrorCode]=asBuffer->Strings[eErrorCode]+AnsiString("_")+ErrorLog;
            }
        }
    }
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddErrorRecordNoSave(AnsiString ErrorLog, bool bAddPosition, int iXPos, int iYPos, int iXPitch, int iXPitch2, int iYPitch)    //Steven 20190110 : 把Error紀錄拆開,不存檔,只記錄用於Retry
{
    if(bAddPosition)                                                            //Steven 20190110 : Error Log Add Motor Position.
    {
        asBuffer->Strings[eUnloadXPos]  =AnsiString(iXPos);
        asBuffer->Strings[eUnloadYPos]  =AnsiString(iYPos);

        asBuffer->Strings[eUnloadXpPos]  =AnsiString(iXPitch);                  //JerryYang 20210119 : add x,y pitch位置
        asBuffer->Strings[eUnloadXp2Pos]  =AnsiString(iXPitch2);
        asBuffer->Strings[eUnloadYpPos]  =AnsiString(iYPitch);
    }
    AddErrorLog(ErrorLog);
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddIndexCycleTimeRecord(double fIndexCycleTime)
{
    AnsiString IndexCycleTime;
    IndexCycleTime.sprintf("%6.3f", (double)fIndexCycleTime);
    asBuffer->Strings[eIndexCycleTime]=IndexCycleTime;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddOrderTestRecord(int iNum)
{
    asBuffer->Strings[eOrderTest]=AnsiString(iNum);
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddDataTimeRecord(AnsiString asStr)
{
    asBuffer->Strings[eSOTTime]=asStr;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddDataTimeEOTRecord(AnsiString asStr)                //wei 20181211 更換設定SOT
{
    asBuffer->Strings[eEOTTime]=asStr;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::Add2DIDRecord(AnsiString s2DID)
{
    asBuffer->Strings[e2DCode]=s2DID;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddUnloadRecord(int iWhichAuto, int iNum, int iTrayRow, int iTrayCol, int iXPos, int iYPos, AnsiString ascode, int iXPitch, int iXPitch2, int iYPitch, AnsiString TrayID)   //Frank 20160727 add iXpos iYpos
{
    if(CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_Greatek ||                                             //Sam 20200704 : Add Greatek
       CUSTOMER_CODE==CC_HANA_MICRON)
    {
        iTrayRow=iTrayRow+1;
        iTrayCol=iTrayCol+1;
    }
    AnsiString sTime=GetDateInfoByString("/")+" "+GetOnlyTimeInfoByString(":"); //Ifor 20180510 : add 時間資訊
    asBuffer->Strings[eWhichAuto]       =AnsiString(iWhichAuto+1);
    asBuffer->Strings[eOutTrayNum]      ="O"+AnsiString(iWhichAuto+1)+"-"+AnsiString(iNum);        //Auto1 :O1-1  Auto2 :2-1  Auto3 :3-1  Fix 1:4-1 Fix 2:5-1 Fix 3:6-1
    asBuffer->Strings[eUnloadX]         =AnsiString(iTrayCol);
    asBuffer->Strings[eUnloadY]         =AnsiString(iTrayRow);
    asBuffer->Strings[eUnloadTime]      =sTime;
    asBuffer->Strings[eUnloadXPos]      =AnsiString(iXPos);                     //Frank 20160727 add iXpos iYpos
    asBuffer->Strings[eUnloadYPos]      =AnsiString(iYPos);                     //Frank 20160727 add iXpos iYpos
    asBuffer->Strings[eUnloadPosTime]   =sTime;                                 //Ifor 20180510 : add 時間資訊
    asBuffer->Strings[eOCRCode]         =AnsiString(ascode);                    //wei 20161005
    asBuffer->Strings[eUnloadXpPos]     =AnsiString(iXPitch);                   //JerryYang 20210119 : add x,y pitch位置
    asBuffer->Strings[eUnloadXp2Pos]    =AnsiString(iXPitch2);
    asBuffer->Strings[eUnloadYpPos]     =AnsiString(iYPitch);
    asBuffer->Strings[eTrayID]          =AnsiString(TrayID);                    //Steven 20220120 : Amlogic需要輸出SBIN
    iOneTrayPickCount[1+iWhichAuto]++;                                          //kevin 20210623 Count 每輪片數量
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddTestTime(AnsiString asStr)                         //Steven 20161219 : Add Test Time in Production log
{
    asBuffer->Strings[eTestTime]=asStr;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddTSDTime(AnsiString asStr)                          //Steven 20161219 (jou) : 修改TSD Log方式
{
    asBuffer->Strings[eTSDTime]=asStr;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddInArmLoaderPickRecord(int iSuckRow, int iSuckCol)  //Sam 20200623 : Add record
{
    asBuffer->Strings[eInArmLoaderPick]=InArmSuck.Suck[iSuckRow][iSuckCol].sName;
    // AI(W906-MyProductionRecord) 20260721: golden :431-436 reads
    // InArmSuck.Suck[][].GetIOValue() (Sam 20230210 VacuumUnit) to populate
    // eLoaderPickVacuum -- TMySucker (aHotPlateSubstrate.h) has NO
    // GetIOValue() member (golden MyKitSuck.h:108; verified absent by grep --
    // a real gap this wave's recon missed). TMySucker is a foreign shared
    // class owned by another wave/file (aHotPlateSubstrate.h) that this unit
    // cannot extend -- "Hard boundary" rule, same precedent already
    // established by atester_32Site.cpp's own W5_32S_ADD_INDEX_PICK_VACUUM
    // no-op (which cites this exact GetIOValue() gap). Gate ONLY this
    // vacuum-read tail; the sName record above is real. eLoaderPickVacuum
    // stays at InitialRecord's "" default until GetIOValue() lands.
#if 0 // TODO(W-VacuumUnit) -- TMySucker::GetIOValue() not real yet (golden MyKitSuck.h:108)
    AnsiString s="";                                                            //Sam 20230210 : 新增 VacuumUnit 通訊模組
    s.sprintf("%2.1f", InArmSuck.Suck[iSuckRow][iSuckCol].GetIOValue());
    if(s=="999.9")
        asBuffer->Strings[eLoaderPickVacuum]="NA";
    else
         asBuffer->Strings[eLoaderPickVacuum]=s;
#endif
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddOutArmShuttlePickRecord(int iSuckRow, int iSuckCol)//Sam 20200623 : Add record
{
    asBuffer->Strings[eOutArmShuttlePick]=OutArmSuck.Suck[iSuckRow][iSuckCol].sName;
    // AI(W906-MyProductionRecord) 20260721: same TMySucker::GetIOValue() gap
    // as AddInArmLoaderPickRecord above -- see that method's comment.
#if 0 // TODO(W-VacuumUnit) -- TMySucker::GetIOValue() not real yet (golden MyKitSuck.h:108)
    AnsiString s="";                                                            //Sam 20230210 : 新增 VacuumUnit 通訊模組
    s.sprintf("%2.1f", OutArmSuck.Suck[iSuckRow][iSuckCol].GetIOValue());
    if(s=="999.9")
        asBuffer->Strings[eOSHTPickVacuum]="NA";
    else
        asBuffer->Strings[eOSHTPickVacuum]=s;
#endif
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddIndexPickShuttleRecord(int iSite, int iSiteRow, int iSiteCol)    //Sam 20200623 : Add record
{
    AnsiString asSite="";
    asSite.sprintf("%s-%d", IndexSuckName[iSiteRow][iSiteCol], iSite);
    asBuffer->Strings[eIndexPickSHTSiteNo]=asSite;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddIndexPlaceShuttleRecord(int iSite, int iSiteRow, int iSiteCol)    //Sam 20200623 : Add record
{
    AnsiString asSite="";
    asSite.sprintf("%s-%d", IndexSuckName[iSiteRow][iSiteCol], iSite);
    asBuffer->Strings[eIndexPlaceSHTSiteNo]=asSite;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddOutShuttleDetectRecord(int iSite, int iSiteRow, int iSiteCol)    //Sam 20200623 : Add record
{
    AnsiString asSite="";
    asSite.sprintf("%s-%d", IndexSuckName[iSiteRow][iSiteCol], iSite);
    asBuffer->Strings[eOutSHTDetectSiteNo]=asSite;
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddInArmHotplatePickRecord(int iSuckRow, int iSuckCol)//Sam 20200716 : Add record
{
    asBuffer->Strings[eInArmHotplatePick]=InArmSuck.Suck[iSuckRow][iSuckCol].sName;
    // AI(W906-MyProductionRecord) 20260721: same TMySucker::GetIOValue() gap
    // as AddInArmLoaderPickRecord above. Golden quirk PRESERVED verbatim
    // inside the gate: golden :475 reads OutArmSuck (not InArmSuck) here even
    // though the sName line just above uses InArmSuck -- a genuine golden
    // copy-paste inconsistency (this method is otherwise InArm-only), kept
    // as-is per "preserve golden bugs verbatim + comment" (moot at runtime
    // either way while GetIOValue() itself does not exist).
#if 0 // TODO(W-VacuumUnit) -- TMySucker::GetIOValue() not real yet (golden MyKitSuck.h:108)
    AnsiString s="";                                                            //Sam 20230210 : 新增 VacuumUnit 通訊模組
    s.sprintf("%2.1f", OutArmSuck.Suck[iSuckRow][iSuckCol].GetIOValue());
    if(s=="999.9")
        asBuffer->Strings[eHPPickVacuum]="NA";
    else
        asBuffer->Strings[eHPPickVacuum]=s;
#endif
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddTestModeRecord(int iTestMode)                      //Sam 20201216 : Add record
{
    asBuffer->Strings[eTestMode]=iTestMode;
}
//------------------------------------------------------------------------------
// AI(W906-MyProductionRecord) 20260721: the Get* family below chains
// AnsiString methods (.c_str()/.SubString()/.Length()) straight off
// `asBuffer->Strings[i]` in golden. vclcompat's `Strings[i]` (StringsProxy)
// is read/write property sugar with only a conversion-to-AnsiString operator
// and assignment operators -- it does NOT forward AnsiString's own methods.
// `asBuffer->GetString(i)` (TStringList's plain function accessor, returns
// AnsiString BY VALUE) is used instead so the chained calls compile --
// established precedent, see mycylin.cpp:826/844's identical
// `Strings[i].c_str()` -> `GetString(i).c_str()` fix (that file's own
// AI(W6.0) comment cites the exact same "vclcompat proxy has no c_str" gap).
int TMyProductionRecord::GetAutoX()
{
    return atoi(asBuffer->GetString(eUnloadX).c_str());
}
//------------------------------------------------------------------------------
int TMyProductionRecord::GetAutoY()
{
    return atoi(asBuffer->GetString(eUnloadY).c_str());
}
//------------------------------------------------------------------------------
int TMyProductionRecord::GetWhcihAuto()
{
    return atoi(asBuffer->GetString(eWhichAuto).c_str())-1;
}
//------------------------------------------------------------------------------
int TMyProductionRecord::GetLoaderX()
{
    return atoi(asBuffer->GetString(eLoadX).c_str());
}
//------------------------------------------------------------------------------
int TMyProductionRecord::GetLoaderY()
{
    return atoi(asBuffer->GetString(eLoadY).c_str());
}
//------------------------------------------------------------------------------
int TMyProductionRecord::GetLoaderNum()                                         //Frank 20181121 add iLoaderNum
{
    AnsiString Str;
    AnsiString sLoadNum=asBuffer->GetString(eLoadNum);
    Str=sLoadNum.SubString(4, sLoadNum.Length());
    return atoi(Str.c_str());
}
//------------------------------------------------------------------------------
int TMyProductionRecord::GetSiteNo()
{
    return atoi(asBuffer->GetString(eSiteNO).c_str());
}
//------------------------------------------------------------------------------
int TMyProductionRecord::GetOrderOfContact()
{
    return atoi(asBuffer->GetString(eOrderTest).c_str());
}
//------------------------------------------------------------------------------
AnsiString TMyProductionRecord::GetSOTTime()
{
    if(asBuffer->Strings[eSOTTime]==NULL)
        return "";
    else
        return asBuffer->Strings[eSOTTime];
}
//------------------------------------------------------------------------------
AnsiString TMyProductionRecord::GetEOTTime()
{
    if(asBuffer->Strings[eEOTTime]==NULL)
        return "";
    else
        return asBuffer->Strings[eEOTTime];
}
//------------------------------------------------------------------------------
AnsiString TMyProductionRecord::GetTestTime()
{
    if(asBuffer->Strings[eTestTime]==NULL)
        return "";
    else
        return asBuffer->Strings[eTestTime];
}
//------------------------------------------------------------------------------
AnsiString TMyProductionRecord::GetErrorLog()
{
    if(asBuffer->Strings[eErrorCode]==NULL)
        return "";
    else
        return asBuffer->Strings[eErrorCode];
}
//------------------------------------------------------------------------------
// AI(W906-MyProductionRecord) 20260721: AddErrorRecord is DELIBERATELY NOT
// DEFINED here -- see this file's top-of-file banner ("AddErrorRecord ...
// IS declared in the header ... but is DELIBERATELY NOT DEFINED here") for
// the full rationale (its golden body's only non-leaf action is the
// out-of-scope SaveRecord() tail call).
//------------------------------------------------------------------------------
void TMyProductionRecord::AddGroundRecord(AnsiString sGround)                   //Sam 20211223 : 每顆 IC 測試完成後要記錄測試當時的 Ground & ESD 數值。
{
     asBuffer->Strings[eGround]=(CosFunction.bRecordGroundESDByTestIC==true)?sGround.c_str():"NA";
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddESDRecord(AnsiString sESD)                         //Sam 20211223 : 每顆 IC 測試完成後要記錄測試當時的 Ground & ESD 數值。
{
    asBuffer->Strings[eESD]=(CosFunction.bRecordGroundESDByTestIC==true)?sESD.c_str():"NA";
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddInRotationAngRecord(int iAngle)                    //Sam 20221103 : 新增In料旋轉角度
{
    asBuffer->Strings[eInRotationAngle]=AnsiString(iAngle);
}
//------------------------------------------------------------------------------
AnsiString TMyProductionRecord::GetInRotationAngRecord()                        //Sam 20221103 : 新增In料旋轉角度
{
    return asBuffer->Strings[eInRotationAngle];
}
//------------------------------------------------------------------------------
//Sam 20230616 : Add Auto Clean Record
//==>
void TMyProductionRecord::AddPickCleanPad(int iSuckRow, int iSuckCol, int iKitRow, int iKitCol, int iData)
{
     AnsiString sTime="";
     GetTimeInfo();
     // AI(W906-MyProductionRecord) 20260721: golden :615 `Now().FormatString(...)`
     // (BCB6 TDateTime member call) -> vclcompat `FormatDateTime(fmt, Now())` --
     // established precedent (ainarm9045.cpp:890, SCK_ART_Remainder.cpp:1282,
     // uHGemClass.cpp:2787).
     sTime.sprintf("%s %02d:%02d:%02d.%03d", FormatDateTime("yyyy/mm/dd", Now()), SystemHour, SystemMin, SystemSec, SystemMSec);
     asBuffer->Strings[ePickCleanPadData]=iData;
     asBuffer->Strings[ePickCleanPadTime]=sTime;
     asBuffer->Strings[ePickCleanPadSuck]=InArmSuck.Suck[iSuckRow][iSuckCol].sName;
     asBuffer->Strings[ePickCleanPadRow]=AnsiString(iKitRow);
     asBuffer->Strings[ePickCleanPadCol]=AnsiString(iKitCol);
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddPlaceCleanPadToShuttle(int iSuckRow, int iSuckCol, int iShtRow, int iShtCol)
{
     AnsiString sTime="";
     GetTimeInfo();
     sTime.sprintf("%s %02d:%02d:%02d.%03d", FormatDateTime("yyyy/mm/dd", Now()), SystemHour, SystemMin, SystemSec, SystemMSec);
     asBuffer->Strings[ePlaceCleanPadToShtTime]=sTime;
     asBuffer->Strings[ePlaceCleanPadToShtSuck]=InArmSuck.Suck[iSuckRow][iSuckCol].sName;
     asBuffer->Strings[ePlaceCleanPadToShtRow]=AnsiString(iShtRow);
     asBuffer->Strings[ePlaceCleanPadToShtCol]=AnsiString(iShtCol);
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddPickCleanPadFormShuttle(int iSuckRow, int iSuckCol, int iShtRow, int iShtCol)
{
     AnsiString sTime="";
     GetTimeInfo();
     sTime.sprintf("%s %02d:%02d:%02d.%03d", FormatDateTime("yyyy/mm/dd", Now()), SystemHour, SystemMin, SystemSec, SystemMSec);
     asBuffer->Strings[ePickCleanPadFormShtTime]=sTime;
     asBuffer->Strings[ePickCleanPadFormShtSuck]=InArmSuck.Suck[iSuckRow][iSuckCol].sName;
     asBuffer->Strings[ePickCleanPadFormShtRow]=AnsiString(iShtRow);
     asBuffer->Strings[ePickCleanPadFormShtCol]=AnsiString(iShtCol);
}
//------------------------------------------------------------------------------
void TMyProductionRecord::AddPlaceCleanPad(int iSuckRow, int iSuckCol, int iKitRow, int iKitCol)
{
     AnsiString sTime="";
     GetTimeInfo();
     sTime.sprintf("%s %02d:%02d:%02d.%03d", FormatDateTime("yyyy/mm/dd", Now()), SystemHour, SystemMin, SystemSec, SystemMSec);
     asBuffer->Strings[ePlaceCleanPadTime]=sTime;
     asBuffer->Strings[ePlaceCleanPadSuck]=InArmSuck.Suck[iSuckRow][iSuckCol].sName;
     asBuffer->Strings[ePlaceCleanPadRow]=AnsiString(iKitRow);
     asBuffer->Strings[ePlaceCleanPadCol]=AnsiString(iKitCol);
}
//<==
//Sam 20230616 : Add Auto Clean Record
//------------------------------------------------------------------------------

// =============================================================================
//  AI(W906-PT-MyProdRec-HeavyHalf) 20260811 -- APPENDED WAVE-2 SECTION
//  (append-only; nothing above this line was rewritten, reordered or deleted)
// =============================================================================
//  ROLE
//  ----
//  The "heavy half" of TMyProductionRecord: golden Public/MyProductionRecord.cpp
//  :561 and :658-1384 -- the per-IC traveling-record FILE WRITERS (CSV append
//  sinks) plus the two path/name helpers they use.  NOBODY PUMPS THIS UNIT on a
//  timer: every function here is called synchronously from the arm/shuttle/
//  index/tester state machines at the instant one IC finishes a step
//  (AddErrorRecord from acarry/ainarm2/aoutarm/asortarm/ainarm_SearchPickPlate,
//  SaveRecord from the out-arm place-to-tray leaves, SaveRecordCleanPad from the
//  AutoClean kit leaves).  Each call appends ONE line to ONE .csv on disk and
//  then calls InitialRecord() to clear the scratch buffer for the next IC.
//
//  SUPERSEDES (cannot be edited from here -- append-only rule; MAIN LOOP please
//  reconcile): this file's ORIGINAL head banner lines 15-20 say "OUT OF SCOPE
//  (golden .cpp:660-1384 ... SaveRecord/SaveDataForASE/SaveDataForGreatek/
//  SaveGroundESDData/SaveRecordCleanPad/SaveRecordForLeadYo/Save2DIDForJCET/
//  SaveJHT_Record/GetProdLogFileName/GetProdLogFilePath ... AddIndexPickVacuum
//  also stays out", and lines 22-44 say AddErrorRecord "is DELIBERATELY NOT
//  DEFINED here".  Both statements are SUPERSEDED by this section.  The same
//  applies to Public/MyProductionRecord.h's own head banner lines 23-39.
//
//  *** DANGER -- THESE FUNCTIONS WRITE TO THE REAL MACHINE'S LOG TREE ***
//  Every path below is a LIVE production path on this very workstation
//  (D:\HT9045 IS the machine root).  Verified against the port tree's own
//  definitions, common.cpp:131/132/198/207/211/238:
//      asASELogPath          = "D:\SaveRecord"                        (SaveDataForASE)
//      asTravelingLogPath    = "D:\HT9045_Log\Production_Log"         (SaveRecord, via GetProdLogFilePath)
//      sGroundESDLogPath     = "D:\HT9045_Log\GroundESDLog"           (SaveGroundESDData)
//      asCleanPadLogPath     = "D:\HT9045_Log\CleanPad_Log"           (SaveRecordCleanPad)
//      asProductionByFilePath= "D:\HT9045_Log\Production_ByFile"      (SaveRecordForLeadYo)
//      sWhite2DIDListLoc     = "D:\HT9045_Log\White2DIDListTmp\"      (Save2DIDForJCET)
//      plus a HARD-CODED "D:\HT9045_Log\ProductionInfo\TrayMapping"   (SaveRecord, golden :696)
//  Golden's literal paths are kept VERBATIM here (per this wave's brief:
//  "prefer keeping the golden path verbatim in the body and flagging it, over
//  silently changing the path").  CONSEQUENCE: a ctest that calls SaveRecord/
//  SaveDataForASE/SaveGroundESDData/SaveRecordCleanPad/SaveRecordForLeadYo will
//  fopen(...,"a+") and APPEND A ROW TO THE REAL MACHINE'S PRODUCTION CSVs, and
//  MyForceDirectories will CREATE those directories if absent.  This is the
//  exact WriteLastDataFile hazard class (its CreateFile hard-codes
//  D:\HT9045\system\lastdata.dat).  test_MyProductionRecord.cpp must NOT be
//  extended to call any of them without first redirecting the six path globals.
//
//  WAVE SCOPE (one line per assigned golden function)
//  -------------------------------------------------
//    ACTIVE  SaveRecord           golden .cpp:660  -- 2 of its arms GATED (G-1, G-2)
//    ACTIVE  SaveDataForASE       golden .cpp:778
//    ACTIVE  SaveDataForGreatek   golden .cpp:851
//    ACTIVE  SaveGroundESDData    golden .cpp:889
//    ACTIVE  SaveRecordForLeadYo  golden .cpp:1038
//    ACTIVE  AddIndexPickVacuum   golden .cpp:1110
//    ACTIVE  GetProdLogFileName   golden .cpp:1120 -- 2 of its 6 arms GATED (G-3)
//    ACTIVE  GetProdLogFilePath   golden .cpp:1144
//    ACTIVE  SaveJHT_Record       golden .cpp:1264 -- golden's WHOLE body is inside
//                                 a block comment; an empty body IS the faithful
//                                 translation.  Not a gate.
//    GATED   Save2DIDForJCET      golden .cpp:1151 -- whole body (G-4)
//    GATED   AddErrorRecord       golden .cpp:561  -- whole body (G-5, homecoming)
//    GATED   SaveRecordCleanPad   golden .cpp:937  -- whole body (G-6, homecoming)
//    ACTIVE  file-scope AnsiString oldStartTime  golden .cpp:658
//    ACTIVE  file-scope FILE     *WriteProdLOG   golden .cpp:659 (not in the brief's
//                                 list but every writer below dereferences it; it had
//                                 ZERO occurrences anywhere in the port tree -- see
//                                 GATE REGISTER absence-claim A-4)
//
//  ORDER DEVIATION (recorded, not hidden): golden's AddErrorRecord sits at
//  :561, i.e. BETWEEN GetErrorLog (:553) and AddGroundRecord (:589), and this
//  file already carries a placeholder comment at that exact position (see
//  :708-712 above).  The append-only rule forbids inserting there, so
//  AddErrorRecord is emitted at the END of this section instead.  Golden order
//  is otherwise preserved exactly for everything else in this section.
//
//  GATE REGISTER
//  -------------
//  Every absence claim below carries THE EXACT COMMAND and THE TIME it was run.
//  ALL of them were re-run immediately before delivery (see the "RE-RUN" stamp
//  on each) precisely because sibling agents can land a file after first look.
//
//  --- A-1 ------------------------------------------------------------------
//  CLAIM: class TfBarCode (aHotPlateSubstrate.h:984-1025, the port tree's
//         ONLY home for the global `fBarCode`, declared aHotPlateSubstrate.h:1026)
//         has NO member `JCETUseMakeWhite2DIDList()` and NO member
//         `iJCETWhitelistSN`.  Golden's are BarCode/BarCode.h:961 and :963.
//  COMMAND: Grep(pattern="fBarCode|JCETUseMakeWhite2DIDList|iJCETWhitelistSN",
//           path="D:/HT9045/HT9011UC_Cpp_V3.33.906.0", glob="*.h",
//           output_mode="content")
//  FIRST RUN 2026-08-11 (this session): only aHotPlateSubstrate.h:1026's
//           `extern TfBarCode *fBarCode;` and the 17-method shim list;
//           zero hits for either JCET name in any header.
//  RE-RUN   2026-08-11 11:45:01 +0800, immediately before delivery, widened to
//           grep -rn "JCETUseMakeWhite2DIDList\|iJCETWhitelistSN" --include=*.h .
//           -> 0 hits outside this file.  Claim stands.
//  USED BY: G-1 (SaveRecord's JCET arm) and G-4 (Save2DIDForJCET's body).
//
//  --- A-2 ------------------------------------------------------------------
//  CLAIM: class TfLotInfo (forms/fLotInfo.h:23-130) has members cbRunMode(:26),
//         edtSysLotID(:29) and edtCusLotID(:65) but NO `edtStep`, NO `edtStage`
//         and NO `lbledtStarTime`.  Golden reads all three at .cpp:1126/1128/
//         1135/1137.
//  COMMAND: Read forms/fLotInfo.h (whole file, 134 lines) + Bash
//           grep -n "edtSysLotID\|edtCusLotID\|cbRunMode\|edtStep\|edtStage\|lbledtStarTime" forms/fLotInfo.h
//  FIRST RUN 2026-08-11 (this session): 3 hits (cbRunMode/edtSysLotID/
//           edtCusLotID), zero for edtStep/edtStage/lbledtStarTime.
//  RE-RUN   2026-08-11 11:45:01 +0800, immediately before delivery:
//           grep -n "edtStep\|edtStage\|lbledtStarTime" forms/fLotInfo.h
//           -> 0 hits.  Claim stands.
//  USED BY: G-3 (GetProdLogFileName's two non-default naming arms).
//
//  --- A-3 ------------------------------------------------------------------
//  CLAIM: class TfProductionInfo (forms/fProductionInfo.h:53-67) carries only
//         `AnsiString sLoadMO_TestFlow` and `virtual void CalTrayICCount(int)`;
//         it has NO `_bOEEStartLotSuccess`.  Golden reads it at .cpp:692.
//  COMMAND: Bash  sed -n '/class TfProductionInfo/,/^};/p' forms/fProductionInfo.h
//  FIRST RUN 2026-08-11 (this session): 2 members, neither is _bOEEStartLotSuccess.
//  RE-RUN   2026-08-11 11:45:01 +0800, immediately before delivery, WIDENED to
//           the whole tree:
//             grep -rn "_bOEEStartLotSuccess" --include=*.h --include=*.cpp .
//           This turned up ONE hit outside this file -- csystem.cpp:1220
//           `fProductionInfo->_bOEEStartLotSuccess==true &&`.  Followed up
//           rather than assumed: that line sits INSIDE csystem.cpp's
//           `#if 0 // GOLDEN VERBATIM ... GATE G-W7-MainProc-FIDELITY` block
//           which opens at csystem.cpp:594, so it is NOT COMPILED and is not
//           evidence of a member.  Claim stands: no header in the tree declares
//           _bOEEStartLotSuccess.
//  USED BY: G-2 (SaveRecord's OEE TrayMapping-to-FTP arm).
//
//  --- A-4 ------------------------------------------------------------------
//  CLAIM: the symbol `WriteProdLOG` (golden .cpp:659, the FILE* every writer in
//         this section uses) did not exist ANYWHERE in the port tree, so
//         defining it here at file scope cannot collide.
//  COMMAND: Grep(pattern="WriteProdLOG",
//           path="D:/HT9045/HT9011UC_Cpp_V3.33.906.0", output_mode="content")
//  FIRST RUN 2026-08-11 (this session): "No matches found".
//  RE-RUN   2026-08-11 11:45:01 +0800, immediately before delivery: the only
//           hits are this file's own definition and uses.  Claim stands.
//           Object-level proof, same timestamp: nm --defined-only on this TU's
//           .o shows `B WriteProdLOG` and `B oldStartTime` (external, BSS), and
//           nm --defined-only on aHotPlateSubstrate.o shows neither.
//  NOTE: golden declares it non-static (external linkage) and that is kept, so
//        it stays greppable and a future duplicate would be a LOUD link error
//        rather than a silent second copy.  Same for `oldStartTime` (golden :658).
//
//  --- A-5 (a PRESENCE claim, the trap-1(b) shape) ---------------------------
//  CLAIM: aHotPlateSubstrate.cpp ALREADY DEFINES no-op stand-ins for exactly
//         three TMyProductionRecord methods, two of which are in this wave's
//         target list:
//             :773 void TMyProductionRecord::AddErrorRecord(AnsiString,bool,int,int,int,int,int) {}
//             :774 void TMyProductionRecord::AddTestRecord(int,int) {}   (not mine)
//             :775 void TMyProductionRecord::SaveRecordCleanPad(AnsiString) {}
//         aHotPlateSubstrate.cpp AND Public/MyProductionRecord.cpp are BOTH
//         members of the SAME target ht9045_sm (CMakeLists.txt:1466 and :1472),
//         so a second definition here is a hard multiple-definition link error,
//         not a silent shadow.
//  COMMAND: Grep(pattern="TMyProductionRecord::", path=".../aHotPlateSubstrate.cpp",
//           output_mode="content") + Bash  sed -n '1460,1480p' CMakeLists.txt
//  FIRST RUN 2026-08-11 (this session): 3 hits at :773/:774/:775; both files in
//           ht9045_sm.
//  RE-RUN   2026-08-11 11:45:01 +0800, immediately before delivery:
//           grep -c "TMyProductionRecord::" aHotPlateSubstrate.cpp -> 3.
//           Claim stands.
//  OBJECT-LEVEL PROOF (same timestamp, the trap-1(b) test done properly rather
//  than by grep): both TUs were compiled to .o with this wave's own flags and
//  their nm --defined-only sets intersected.  Result: the ONLY names defined by
//  BOTH are five inline/COMDAT ones that were ALREADY in both objects before
//  this section existed (TMyProductionRecord::~TMyProductionRecord and four
//  vclcompat::AnsiString inlines) -- benign, pre-existing, deduplicated by the
//  linker.  ZERO of this section's new symbols appears in aHotPlateSubstrate.o,
//  i.e. the gates hold.  Conversely nm --undefined-only on aHotPlateSubstrate.o
//  still lists TMyProductionRecord::TMyProductionRecord() and ::InitialRecord()
//  as NEEDED, which is the positive proof that this archive member really is
//  extracted at link time rather than sitting unused (trap 1(a)).
//  USED BY: G-5 (AddErrorRecord) and G-6 (SaveRecordCleanPad).
//
//  --- G-1  SaveRecord's JCET white-2DID arm --------------------------------
//  (a) GOLDEN LINE: .cpp:673-676
//        else if(fBarCode->JCETUseMakeWhite2DIDList()==true)
//            Save2DIDForJCET(fLotInfo->edtSysLotID->Text, fLotInfo->edtCusLotID->Text, fLotInfo->cbRunMode->Text);
//  (b) WHY THE OFFLINE DEFAULT IS FAITHFUL: premise A-1 -- the member function
//      does not exist on the port's fBarCode, so this arm cannot be compiled at
//      all.  Deleting the arm makes the offline behaviour identical to a machine
//      whose JCETUseMakeWhite2DIDList() returns false, which is EVERY machine
//      that is not a JCET site running the FT1-whitelist/FT2-compare feature.
//      Golden's own body for that predicate is at BarCode.cpp:11318; port
//      CUSTOMER_CODE defaults to 0 (cmydef.cpp:3412 `int CUSTOMER_CODE=0;`,
//      matching no CC_* value), so false is also the value the port's own
//      default configuration produces.
//  (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS: on a real JCET machine with the
//      feature on, EVERY SaveRecord ALSO appends one whitelist row to
//      D:\HT9045_Log\White2DIDListTmp\<cust>\<sublot>\<sublot>_<mode>.csv and
//      bumps fBarCode->iJCETWhitelistSN.  With this gate the whitelist file is
//      never created and the serial never advances.  NOT equivalent -- a real
//      loss of function, deliberately deferred, not a cosmetic degradation.
//
//  --- G-2  SaveRecord's OEE "TrayMapping to FTP" redirect -------------------
//  (a) GOLDEN LINE: .cpp:692-701 (the whole `if(fProductionInfo->
//      _bOEEStartLotSuccess && IniConfig.bN14_3_OEEFTPUpload &&
//      IniConfig.bN14_19_TrayMappingToFTP){...}` block).
//  (b) WHY THE OFFLINE DEFAULT IS FAITHFUL: premise A-3 -- the member does not
//      exist, so the condition cannot be compiled.  Removing the block leaves
//      sFullFileName at the value the four lines above it just computed (the
//      normal Production_Log path), which is EXACTLY what golden does whenever
//      any of the three conjuncts is false.  All three are false in the port's
//      default configuration: _bOEEStartLotSuccess is only set true by a
//      successful OEE lot-start (unported), and IniConfig is a zero-initialised
//      static HT9045_CONFIG (Config.h:1499 `extern HT9045_CONFIG IniConfig;`),
//      so bN14_3_OEEFTPUpload (Config.h:1064) and bN14_19_TrayMappingToFTP
//      (Config.h:1105) are both false until an .ini load sets them.
//  (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS: on a Greatek/OEE-FTP machine with a
//      started lot, golden REDIRECTS this record away from the per-day
//      Production_Log csv into the single-file
//      D:\HT9045_Log\ProductionInfo\TrayMapping\TrayMappingTemp.csv (Greatek
//      column order).  With the gate, such a machine would write the ordinary
//      Production_Log row instead and the TrayMapping upload staging file would
//      never be produced.
//
//  --- G-3  GetProdLogFileName's two non-default naming arms -----------------
//  (a) GOLDEN LINES: .cpp:1125-1128 (bTryCatch==true arm) and .cpp:1134-1137
//      (bTryCatch==false arm) -- the CC_SJ_Semiconductor
//      "HandlerID_InnerLotID_Step_Stage_TimeStamp" name and the
//      IniConfig.bO24_ProductionLogByLot "HandlerID_SysLot_RunMode_StartTime"
//      name.
//  (b) WHY THE OFFLINE DEFAULT IS FAITHFUL: premise A-2 -- fLotInfo has no
//      edtStep/edtStage/lbledtStarTime, so neither arm can be compiled.  The
//      surviving `else` arm (HandlerID_YYYYMMDD.csv) is golden's own behaviour
//      for every machine that is neither CC_SJ_Semiconductor (=791,
//      MachineType.h:163) nor running Production-Log-By-Lot.  Port default
//      CUSTOMER_CODE is 0 and IniConfig.bO24_ProductionLogByLot (Config.h:1380)
//      is false in the zero-initialised default, so the surviving arm is
//      already the one golden would pick.  NOTE the gate keeps the golden
//      `else` KEYWORD inside the disabled block, which is what turns the
//      surviving sprintf into an unconditional statement -- the arm order is
//      therefore unchanged, not re-prioritised.
//  (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS: an SJ Semiconductor machine, or any
//      machine with O24 on, would silently get per-DAY files named
//      <HandlerID>_YYYYMMDD.csv instead of per-LOT files.  Records from two
//      lots on the same day would land in ONE file, and a lot spanning midnight
//      would be split across two -- a real, customer-visible difference.
//
//  --- G-4  Save2DIDForJCET, WHOLE BODY -------------------------------------
//  (a) GOLDEN LINES: .cpp:1152-1262 (the entire body; kept VERBATIM inside the
//      disabled block so the golden text is still reviewable/greppable in place).
//  (b) WHY THE OFFLINE DEFAULT IS FAITHFUL: premise A-1.  Three statements
//      (:1171 `fBarCode->iJCETWhitelistSN=0;`, :1190 `...++;`, :1208
//      `IntToStr(fBarCode->iJCETWhitelistSN)`) touch a member that does not
//      exist, and it is a HARD COMPILE BLOCKER, not a value that can be
//      defaulted.  The deliberate choice here is a WHOLE-BODY gate rather than
//      gating only those three lines, and the reason is trap-3-shaped, so it is
//      written down rather than assumed: iJCETWhitelistSN is the PartID column
//      of a CUSTOMER WHITELIST file that JCET's FT2 stage compares against.
//      Emitting rows with a fabricated or missing PartID into
//      D:\HT9045_Log\White2DIDListTmp\ would be WORSE than emitting nothing.
//      A TU-local mirror counter was considered and REJECTED as unfaithful:
//      golden does NOT own this counter here -- uLotInfo.cpp:7704 seeds it from
//      the last row of an existing whitelist file and :7709 resets it, so a
//      counter private to this file would diverge from golden on the very first
//      lot resumed from disk.  (Verified in golden: grep -rn
//      "iJCETWhitelistSN|JCETUseMakeWhite2DIDList" over
//      HT9011UC_Code_V3.33.906.0_20260618, 2026-08-11 -- 20 hits, in
//      BarCode.cpp/BarCode.h/csystem.cpp/main.cpp/uLotInfo.cpp/this file.)
//      LIVE-CALLER CHECK: the gate is self-consistent -- Save2DIDForJCET has
//      ZERO live callers in the port tree.  Its only golden call sites are this
//      file's own :675 (itself gated, G-1) and uLotInfo/csystem/main, none of
//      which are ported.  So this gate removes no reachable behaviour today.
//  (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS: identical to G-1(c).
//
//  --- G-5  AddErrorRecord, WHOLE BODY (HOMECOMING, not a capability gap) ----
//  (a) GOLDEN LINES: .cpp:561-573.
//  (b) WHY THE OFFLINE DEFAULT IS FAITHFUL: premise A-5.  This is NOT "the
//      dependency is missing" -- every symbol it needs is real and SaveRecord
//      now exists in this very file.  It is gated because a live no-op stand-in
//      body ALREADY EXISTS at aHotPlateSubstrate.cpp:773 in the SAME archive,
//      so defining it here is a multiple-definition link error, and
//      aHotPlateSubstrate.cpp is outside this wave's write boundary.  This is
//      the tree's established HOMECOMING shape -- see cMyDB.h:22-60's
//      "HOMECOMING NOTICE" for the identical pattern applied to MyDBIProcess/
//      MyDBIProcessNew/RecordProcess/NewRecordProcess.  The swap is ONE step:
//      delete aHotPlateSubstrate.cpp:773, then delete this block's guard pair.
//      Doing only one of the two is a link error either way, which is the point.
//      TRAP-3 SECOND ANSWER, and the one that actually matters: even once the
//      collision is resolved, WHETHER to un-gate is a REAL DECISION, not a
//      formality.  Golden's AddErrorRecord tail-calls SaveRecord(), and there
//      are 30+ LIVE, UNGATED call sites in the port tree today (acarry.cpp:2440/
//      2441/2468/2493/2516/2537/2558/2573/2600/2601/2628/2653/2676/2697/2718/
//      2733, ainarm2.cpp:4844, ainarm_SearchPickPlate.cpp:176, aoutarm.cpp:3013/
//      3034/3074/3163/3547/3553, aoutarm9045.cpp:713/982, asortarm.cpp:799/2248/
//      2269/2309/2398/2709/4958) all of which currently bind to the no-op.
//      Un-gating makes every one of them APPEND A ROW TO
//      D:\HT9045_Log\Production_Log\<YYYYMM>\<HandlerID>_<YYYYMMDD>.csv on
//      the real machine the moment any ctest drives those state machines.  The
//      no-op stand-in is, today, the only thing preventing that.
//  (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS: on the machine, every JAM/WAR error
//      records the arm X/Y/pitch position into the IC's traveling record AND
//      flushes that IC's whole row to the production CSV.  Gated, neither the
//      position fields nor the flush happen -- the IC's record is silently
//      dropped and the CSV has no row for it.
//
//  --- G-6  SaveRecordCleanPad, WHOLE BODY (HOMECOMING) ---------------------
//  (a) GOLDEN LINES: .cpp:938-1035.
//  (b) WHY THE OFFLINE DEFAULT IS FAITHFUL: premise A-5, identical shape to G-5
//      -- live no-op stand-in at aHotPlateSubstrate.cpp:775, same archive, file
//      outside this wave's write boundary.  Swap = delete that line, then
//      delete this block's guard pair.  And the same trap-3 second answer
//      applies with a specific, checked consequence: AutoClean/AutoClean.cpp's
//      clean-pad leaves call it, so un-gating makes an AutoClean ctest create
//      and append to D:\HT9045_Log\CleanPad_Log\<YYYYMM>\<PC_NAME>_<date>.csv
//      on the real machine.
//  (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS: the 17-column clean-pad audit trail
//      (pick pad -> place to shuttle -> pick from shuttle -> place pad, each
//      with time/sucker/row/col) is not written at all, and InitialRecord() is
//      not called at the end of a clean-pad cycle, so the scratch buffer keeps
//      the previous pad's fields.  The existing no-op has exactly that same
//      second effect today, so un-gating FIXES a latent staleness rather than
//      introducing one.
//
//  --- SEAM (active code, NOT a gate): the 3-argument MyDBIProcess -----------
//  Golden calls MyDBIProcess in BOTH shapes: 2-arg (:844 :930) and 3-arg
//  (:730 :744 :751 :765 :990 :1005 :1012 :1028 :1065 :1080 :1087 :1102 :1219
//  :1234 :1241 :1256).  The port has:
//     * a REAL 2-arg sink, `void MyDBIProcess(AnsiString,AnsiString)`, declared
//       aHotPlateSubstrate.h:933, DEFINED aHotPlateSubstrate.cpp:1099 (it also
//       feeds the W906_MyDBIProcess_Count/_LastS1/_LastS2 observation seam);
//     * a REAL 3-arg sink, `void __fastcall MyDBIProcess(AnsiString,AnsiString,
//       AnsiString)`, DEFINED SECSGEM/uHGemEquipment.cpp:3475 -- whose body
//       (verified by reading it, :3493-3495) is literally
//       `::MyDBIProcess(S1, S2); (void)S3;`, i.e. it DROPS S3;
//     * cMyDB.h:81's golden-faithful declaration `void __fastcall
//       MyDBIProcess(AnsiString,AnsiString,AnsiString="")` -- which CANNOT be
//       included here: its default argument makes it viable for 2-argument
//       calls too, turning this file's own :844/:930 2-arg calls into
//       ambiguous-call compile errors (uHGemEquipment.cpp:3468-3474 documents
//       that exact hazard and is why its own definition has no default).
//  RESOLUTION: a TU-local anonymous-namespace 3-arg overload that forwards to
//  the real 2-arg sink -- adding NO new archive dependency from ht9045_sm onto
//  ht9045_secsgem, which a plain extern of the SECSGEM symbol would have
//  created.  Established precedent in this tree, twice:
//  KYECFTP/FTPClient_Transfer.cpp:80-92 and KYECFTP/FTPClient_EventHandlers.cpp
//  (both fold S2+" : "+S3 into the 2-arg sink exactly as done here).  The
//  forward is written `::MyDBIProcess` for the same defensive reason those
//  files state.  DIFFERENCE FROM THE SECSGEM SINK, stated because it is not
//  nothing: that one DROPS S3, this one APPENDS it to S2, so the S3 payload
//  (the failing file path) survives into the observation seam instead of being
//  discarded.  Either way no DB row is written offline.
//  NOT a stand-in in the trap-1(e) sense: it does not satisfy anyone else's
//  undefined symbol, it has internal linkage, and the observation seam in
//  aHotPlateSubstrate.cpp still sees every call.
// =============================================================================

#include "Config.h"                  // IniConfig (HT9045_CONFIG): bN33_1_NetChangeFileAndData,
                                     // bASE_Report, bG11ASEReport, bN14_3_OEEFTPUpload,
                                     // bN14_19_TrayMappingToFTP, bO24_ProductionLogByLot,
                                     // SocketHandlerID
#include "common.h"                  // asASELogPath/asTravelingLogPath/sGroundESDLogPath/
                                     // asCleanPadLogPath/asProductionByFilePath/
                                     // sWhite2DIDListLoc + MyForceDirectories
#include "ProductionInfo/FileInfo.h" // FileInfo().PathCombin / .EnsureDirectoriesExist

#include <cstdio>                    // FILE, fopen, fprintf, fclose

// -----------------------------------------------------------------------------
//  See "SEAM" in the banner above.  Internal linkage; forwards to the real
//  2-arg sink (aHotPlateSubstrate.cpp:1099).
// -----------------------------------------------------------------------------
namespace {
void MyDBIProcess(AnsiString S1, AnsiString S2, AnsiString S3)
{
    ::MyDBIProcess(S1, S2 + " : " + S3);
}
} // namespace

//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:658 -- file-scope, and DEAD in golden
// too: `oldStartTime` is written/read nowhere in the entire golden tree
// (verified 2026-08-11: grep -rn "oldStartTime" over
// HT9011UC_Code_V3.33.906.0_20260618 returns exactly this one declaration).
// Kept anyway, verbatim, because the brief names it and faithfulness is cheap.
AnsiString oldStartTime="";
// golden Public/MyProductionRecord.cpp:659 -- the shared FILE* every writer
// below uses.  Golden reuses ONE global handle for all of them and relies on
// each function closing it; that is preserved (see the SaveRecordForLeadYo
// note for the fclose(NULL) bug this design causes in golden).
FILE *WriteProdLOG;

//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:660-773
// AI(W906-PT-MyProdRec-HeavyHalf) 20260811: TRANSLATION NOTES for this function
//   * golden's `fprintf(WriteProdLOG, "%s \n", asBuffer->CommaText)` passes an
//     object to C varargs.  In BCB6 that happens to work because AnsiString is
//     a single char* under the hood; in this tree AnsiString wraps std::string
//     and CommaText is a CommaTextProxy, so `->GetCommaText().c_str()` is used
//     -- same established shape as Public/cBootLog.cpp:98's `sDetail.c_str()`,
//     and the same class of fix this file already made for
//     `Strings[i].AnsiPos(...)` -> `GetString(i).AnsiPos(...)` at :453.
//   * every literal path/flag/magic string below is golden's, unchanged.
//------------------------------------------------------------------------------
void TMyProductionRecord::SaveRecord(AnsiString ErrorLog)
{
    AnsiString asStr, asPath, asFileName="";
    TStringList *SList;
    bool bHasFile=false;

    AnsiString sFilePath="", sFileName="", sFullFileName="";

    if(CUSTOMER_CODE==CC_LEADYO &&
       IniConfig.bN33_1_NetChangeFileAndData)                                   //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
    {
        SaveRecordForLeadYo();
    }
#if 0 // GATE G-1 -- golden :673-676 (fBarCode->JCETUseMakeWhite2DIDList() / fLotInfo->edtCusLotID -- see GATE REGISTER A-1, G-1)
    else if(fBarCode->JCETUseMakeWhite2DIDList()==true)                         //RogerYang 20251210 : JCET 2D FT1白名單/FT2比對功能
    {
        Save2DIDForJCET(fLotInfo->edtSysLotID->Text, fLotInfo->edtCusLotID->Text, fLotInfo->cbRunMode->Text);
    }
#endif

    if(bUse==false)                                                             //一開始就沒吸取ic 就不紀錄
    {                                                                           //目前 tray資料被清為0
        return;
    }
    AddErrorLog(ErrorLog);
    if(IniConfig.bASE_Report)                                                   //Steven 20161227 (jou) : 修改成新的方式存ASELog
    {
        return;                                                                 //kevin 20210914 mark
    }

    asPath=GetProdLogFilePath();
    asFileName=GetProdLogFileName(false);
    MyForceDirectories(asPath, "TMyProductionRecord::SaveRecord_1");
    sFullFileName=FileInfo().PathCombin(asPath,asFileName);
#if 0 // GATE G-2 -- golden :692-701 (fProductionInfo->_bOEEStartLotSuccess -- see GATE REGISTER A-3, G-2)
    if(fProductionInfo->_bOEEStartLotSuccess &&
       IniConfig.bN14_3_OEEFTPUpload &&
       IniConfig.bN14_19_TrayMappingToFTP)
    {
        sFilePath="D:\\HT9045_Log\\ProductionInfo\\TrayMapping";
        sFileName="TrayMappingTemp.csv";
        MyForceDirectories(sFilePath, "TMyProductionRecord::SaveRecord_2");
        sFullFileName=FileInfo().PathCombin(sFilePath,sFileName);
        SaveDataForGreatek(sFullFileName);
    }
#endif

    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20200707 : ProductionRecord for Greatek
    {
        SaveDataForGreatek(sFullFileName);
        InitialRecord();
        return;
    }

    bHasFile=FileExists(sFullFileName);
    if(bHasFile==false)
    {
        SList=new TStringList();                                                //Steven 20170518 (jou) : 換位置改善UPH
        SList->Clear();
        SList->Add(asDataTitle);                                                //Steven 20200416 : 整合Eventlog的title
        SList->SaveToFile(sFullFileName);
        SList->Clear();                                                         //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete SList;                                                           //Steven 20170518 (jou) : 換位置改善UPH
    }

    WriteProdLOG=fopen(sFullFileName.c_str(), "a+");
    if(WriteProdLOG!=NULL)
    {
        try
        {
            fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str());   //kevin 20151001
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TMyProductionRecord::SaveRecord - 1", sFullFileName);
            asPath=GetProdLogFilePath();                                        //Steven 20200811 : 存檔失敗的時候,改個名字再存一次看看
            asFileName=GetProdLogFileName(true);
            sFullFileName=FileInfo().PathCombin(asPath,asFileName);
            fclose(WriteProdLOG);
            WriteProdLOG=fopen(sFullFileName.c_str(), "a+");
            if(WriteProdLOG!=NULL)
            {
                try
                {
                    fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str()); //kevin 20151001
                }
                catch(...)
                {
                    MyDBIProcess("Exception", "TMyProductionRecord::SaveRecord - 2", sFullFileName);
                }
            }
        }
    }
    else
    {
        MyDBIProcess("Exception", "TMyProductionRecord::SaveRecord - WriteProdLOG is NULL", sFullFileName);
        asPath=GetProdLogFilePath();                                            //Steven 20200811 : 存檔失敗的時候,改個名字再存一次看看
        asFileName=GetProdLogFileName(true);
        sFullFileName=FileInfo().PathCombin(asPath,asFileName);
        WriteProdLOG=NULL;
        WriteProdLOG=fopen(sFullFileName.c_str(), "a+");
        if(WriteProdLOG!=NULL)
        {
            try
            {
                fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str());            //kevin 20151001
            }
            catch(...)
            {
                MyDBIProcess("Exception", "TMyProductionRecord::SaveRecord - 3", sFullFileName);
            }
        }
    }

    if(WriteProdLOG!=NULL)
        fclose(WriteProdLOG);
    InitialRecord();
}
//------------------------------------------------------------------------------
//kevin 20140918 儲存 每一顆IC 資料
//Steven 20161227 (jou) : 修改成新的方式存ASELog
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:778-849
// AI(W906-PT-MyProdRec-HeavyHalf) 20260811: WRITES D:\SaveRecord\<YYYYMM>\
// <schedule>_<pcname>_<starttime>.csv on the real machine (asASELogPath,
// common.cpp:131).  See this section's banner DANGER block.
// GOLDEN QUIRK PRESERVED: the eIndexCycleTime/eTestCate column comments below
// carry golden's OLD enum numbers (=13/=12/=2/=3...), which no longer match the
// current eMyProdRec values (eTestCate is 16, eIndexCycleTime is 15).  They are
// stale COMMENTS only -- the code itself uses the enumerators, so behaviour is
// correct; transcribed verbatim rather than "corrected".
//------------------------------------------------------------------------------
void TMyProductionRecord::SaveDataForASE(AnsiString sPcName)
{
    AnsiString asStr, asPatch, asFileName="";
    AnsiString asSendToASE1="@e02001";
    bool bHasFile=false;
    TStringList *SList;

    asPatch.sprintf("%s\\%04d%02d", asASELogPath, SystemYear, SystemMonth);
    MyForceDirectories(asPatch, "TMyProductionRecord::SaveDataForASE");
    asFileName.sprintf("%s_%s_%s.csv", ASET_ScheduleNAME, sPcName, ASET_StartTimeNAME);
    AseSaveFilePath.sprintf("%s\\%s", asPatch, asFileName);
    bHasFile=FileExists(AseSaveFilePath);

    if(bHasFile==false)
    {
        SList=new TStringList();                                                //Steven 20170519 (wei) : 換位置改善UPH
        //kevin 20170607 (wei) SList->LoadFromFile(AseSaveFilePath);
        asStr=AnsiString("Input tray, ")      +       //eLoadNum        =2
              AnsiString("X, ")               +       //eLoadX          =3
              AnsiString("Y, ")               +       //eLoadY          =4
              AnsiString("Output tray, ")     +       //eOutTrayNum     =15            //Auto1 :O1-1  Auto2 :2-1  Auto3 :3-1  Fix 1:4-1 Fix 2:5-1 Fix 3:6-1
              AnsiString("Out X, ")           +       //eUnloadX        =16
              AnsiString("Out Y, ")           +       //eUnloadY        =17
              AnsiString("Site No, ")         +       //eSiteNO         =8             //1-32
              AnsiString("Arm No, ")          +       //eArmNO          =9             //0: arm1  1: arm2
              AnsiString("Order of testing, ")+       //eOrderTest      =10            //touch-down
              AnsiString("Test category, ")   +       //eTestCate       =13            //測試 BIN別
              AnsiString("Index cycle time, ")+       //eIndexCycleTime =12            //第一次測試 是 0 (1st device)
              AnsiString("Time stamp, ")      +       //eSOTTime        =11            //kevin 20140918  yyyymmdd_hhmmss
              AnsiString("Error log")         ;       //eErrorCode      =20

        SList->Add(asStr);
        SList->SaveToFile(AseSaveFilePath);
        SList->Clear();                                                         //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete SList;                                                           //Steven 20170519 (wei) : 換位置改善UPH
    }

    asStr=asBuffer->Strings[eLoadNum        ]+","+
          asBuffer->Strings[eLoadX          ]+","+
          asBuffer->Strings[eLoadY          ]+","+
          asBuffer->Strings[eOutTrayNum     ]+","+
          asBuffer->Strings[eUnloadX        ]+","+
          asBuffer->Strings[eUnloadY        ]+","+
          asBuffer->Strings[eSiteNO         ]+","+
          asBuffer->Strings[eArmNO          ]+","+
          asBuffer->Strings[eOrderTest      ]+","+
          asBuffer->Strings[eTestCate       ]+","+
          asBuffer->GetString(eIndexCycleTime ).Trim()+","+
          asBuffer->GetString(eSOTTime        ).TrimLeft()+","+
          asBuffer->GetString(eErrorCode      ).TrimLeft();

    AseIcRecord+=asStr+":";
    if(IniConfig.bASE_Report && IniConfig.bG11ASEReport==false)       //kevin 20170306
    {
        return;
    }

    WriteProdLOG=fopen(AseSaveFilePath.c_str(), "a+");
    if(WriteProdLOG!=NULL)
    {
        try
        {
            fprintf(WriteProdLOG, "%s \n", asStr.c_str());                      //kevin 20151001
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TMyProductionRecord::SaveDataForASE");
        }
        fclose(WriteProdLOG);
    }
    WriteProdLOG=NULL;
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:851-887
// AI(W906-PT-MyProdRec-HeavyHalf) 20260811: GOLDEN BUG PRESERVED VERBATIM --
// iSortData is declared `[eDataTotal]` (eDataTotal==71 today,
// Public/MyProductionRecord.h:332) but golden supplies only 43 initialisers, so
// elements [43]..[70] are zero-filled by the language.  The loop below then runs
// i=0..eDataTotal-1 and, for those 28 tail iterations, appends
// asBuffer->Strings[0] (eScheduleName) again -- i.e. every Greatek row ends with
// the schedule name repeated 28 times, and the fields the enum gained after
// eHotPlateForm(=42) are never emitted at all.  The initialiser list was
// evidently correct when eDataTotal was 43 (asDataTitleGreatek, built in this
// file's ctor, indeed stops at eHotPlateForm) and was never grown with the enum.
// NOT FIXED, per this wave's "do not fix golden's bugs" rule.
//------------------------------------------------------------------------------
void TMyProductionRecord::SaveDataForGreatek(AnsiString asPath)                 //Sam 20200707 : ProductionRecord for Greatek
{
    AnsiString asStr="";
    bool bHasFile=false;
    TStringList *SList;

    bHasFile=FileExists(asPath);
    int iSortData[eDataTotal]={0,1,2,41,3,4,5,30,35,42,6,7,8,36,9,10,11,31,12,13,14,15,16,37,32,17,33,34,18,19,20,21,22,23,24,25,26,27,28,29,38,39,40};
    if(bHasFile==false)
    {
        SList=new TStringList();
        SList->Clear();

        SList->Add(asDataTitleGreatek);
        SList->SaveToFile(asPath);
        SList->Clear();
        delete SList;
    }

    asStr="";
    for(int i=0; i<eDataTotal; i++)
    {
        asStr+=asBuffer->Strings[iSortData[i]]+",";
    }

    WriteProdLOG=fopen(asPath.c_str(), "a+");
    if(WriteProdLOG!=NULL)
    {
        try
        {
            fprintf(WriteProdLOG, "%s \n", asStr.c_str());                      //kevin 20151001
        }
        catch(...){}
        fclose(WriteProdLOG);
    }
    WriteProdLOG=NULL;
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:889-935
// AI(W906-PT-MyProdRec-HeavyHalf) 20260811: WRITES
// D:\HT9045_Log\GroundESDLog\<lot>.csv on the real machine (sGroundESDLogPath,
// common.cpp:198).  See this section's banner DANGER block.
// Prod.bIsPassBin is `int  bIsPassBin[TEST_MAX_BIN]` in this tree (cprod.h:516,
// "Steven 20240118 : bIsPass --> bIsPassBin"), not bool; the ternary reads it as
// a truth value exactly as golden does.
//------------------------------------------------------------------------------
void TMyProductionRecord::SaveGroundESDData(AnsiString sLot)                    //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
{
    if(CosFunction.bRecordGroundESDByTestIC==false)
        return;
    int iBin=0;
    AnsiString sPassFail="F";
    AnsiString aSaveFilePath="",asStr="";
    bool bHasFile=false;
    TStringList *SList;
    MyForceDirectories(sGroundESDLogPath, "TMyProductionRecord::SaveGroundESDData");
    aSaveFilePath.sprintf("%s\\%s.csv", sGroundESDLogPath, sLot);
    bHasFile=FileExists(aSaveFilePath);
    if(bHasFile==false)
    {
        SList=new TStringList();
        SList->Add(asDataTitle_SG);
        SList->SaveToFile(aSaveFilePath);
        SList->Clear();
        delete SList;
    }

    iBin=atoi(asBuffer->GetString(eTestCate).c_str());
    if(iBin>=0 && iBin<TEST_MAX_BIN)
        sPassFail=(Prod.bIsPassBin[iBin])?"P":"F";

    asStr=asBuffer->Strings[eEOTTime ]+","+
          asBuffer->Strings[e2DCode  ]+","+
          asBuffer->Strings[eTestCate]+","+
          sPassFail                   +","+                                     //Pass=0 Fail=1
          asBuffer->Strings[eGround  ]+","+
          asBuffer->Strings[eESD     ];

    WriteProdLOG=fopen(aSaveFilePath.c_str(), "a+");
    if(WriteProdLOG!=NULL)
    {
        try
        {
            fprintf(WriteProdLOG, "%s \n", asStr.c_str());                      //kevin 20151001
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TMyProductionRecord::SaveGroundESDData");
        }
        fclose(WriteProdLOG);
    }
    WriteProdLOG=NULL;
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:937-1036 -- GATED, see GATE REGISTER G-6
// (homecoming: aHotPlateSubstrate.cpp:775 already defines a no-op with external
// linkage in the SAME archive).  Golden text is kept VERBATIM below so it is
// reviewable in place; the swap is "delete aHotPlateSubstrate.cpp:775, then
// delete this guard pair".  DO NOT do only one of the two.
// GOLDEN BUG NOTED (do not fix on un-gating): the two retry arms at golden :1001
// and :1024 write asBuffer->CommaText -- the RAW 71-field record -- into a file
// whose header/first-write is the 18-field asDataTitleCleanPad layout, i.e. the
// retry row does not match the file's own columns.  Preserved.
//------------------------------------------------------------------------------
//AI(W906-W2-CLEANPADLOG) 20260926: GATE G-6 LIFTED (homecoming) -- the stand-in at aHotPlateSubstrate.cpp (was :775, now :944) is retired in the same commit; writes go under asCleanPadLogPath, which ctest redirects via W906_CLEANPADLOG_ROOT (common.cpp); the golden retry-row bug above is preserved
void TMyProductionRecord::SaveRecordCleanPad(AnsiString ErrorLog)               //Sam 20230616 : Add Auto Clean Record
{
    AnsiString asStr, asPath, asFileName="";
    TStringList *SList;
    bool bHasFile=false;

    AnsiString sFilePath="",sFileName="",sFullFileName="";

    asPath.sprintf("%s\\%04d%02d\\", asCleanPadLogPath, SystemYear, SystemMonth);
    AddErrorLog(ErrorLog);

    MyForceDirectories(asPath, "TMyProductionRecord::SaveRecordCleanPad");
    asFileName.sprintf("%s_%04d%02d%02d.csv", PC_NAME, SystemYear, SystemMonth, SystemDate);
    asPath+=asFileName;

    bHasFile=FileExists(asPath);
    if(bHasFile==false)
    {
        SList=new TStringList();
        SList->Clear();
        SList->Add(asDataTitleCleanPad);
        SList->SaveToFile(asPath);
        SList->Clear();
        delete SList;
    }
    asStr=asBuffer->Strings[ePickCleanPadData]          +","+
          asBuffer->Strings[ePickCleanPadTime]          +","+
          asBuffer->Strings[ePickCleanPadSuck]          +","+
          asBuffer->Strings[ePickCleanPadRow]           +","+
          asBuffer->Strings[ePickCleanPadCol]           +","+
          asBuffer->Strings[ePlaceCleanPadToShtTime]    +","+
          asBuffer->Strings[ePlaceCleanPadToShtSuck]    +","+
          asBuffer->Strings[ePlaceCleanPadToShtRow]     +","+
          asBuffer->Strings[ePlaceCleanPadToShtCol]     +","+
          asBuffer->Strings[ePickCleanPadFormShtTime]   +","+
          asBuffer->Strings[ePickCleanPadFormShtSuck]   +","+
          asBuffer->Strings[ePickCleanPadFormShtRow]    +","+
          asBuffer->Strings[ePickCleanPadFormShtCol]    +","+
          asBuffer->Strings[ePlaceCleanPadTime]         +","+
          asBuffer->Strings[ePlaceCleanPadSuck]         +","+
          asBuffer->Strings[ePlaceCleanPadRow]          +","+
          asBuffer->Strings[ePlaceCleanPadCol]          +","+
          asBuffer->Strings[eErrorCode]                   ;

    WriteProdLOG=fopen(asPath.c_str(), "a+");
    if(WriteProdLOG!=NULL)
    {
        try
        {
            fprintf(WriteProdLOG, "%s \n", asStr.c_str());
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordCleanPad - 1", asPath);

            asPath.sprintf("%s\\%04d%02d\\", asCleanPadLogPath, SystemYear, SystemMonth);
            asFileName.sprintf("%s_%04d%02d%02d_1.csv", PC_NAME, SystemYear, SystemMonth, SystemDate);     //Steven 20170123 (Jou) : 修改檔案命名格式
            asPath+=asFileName;
            fclose(WriteProdLOG);
            WriteProdLOG=fopen(asPath.c_str(), "a+");
            if(WriteProdLOG!=NULL)
            {
                try
                {
                    fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str()); //kevin 20151001
                }
                catch(...)
                {
                    MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordCleanPad - 2", asPath);
                }
            }
        }
    }
    else
    {
        MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordCleanPad - WriteProdLOG is NULL", asPath);

        asPath.sprintf("%s\\%04d%02d\\", asCleanPadLogPath, SystemYear, SystemMonth);
        asFileName.sprintf("%s_%04d%02d%02d_1.csv", PC_NAME, SystemYear, SystemMonth, SystemDate);     //Steven 20170123 (Jou) : 修改檔案命名格式
        asPath+=asFileName;
//        fclose(WriteProdLOG);
        WriteProdLOG=NULL;
        WriteProdLOG=fopen(asPath.c_str(), "a+");
        if(WriteProdLOG!=NULL)
        {
            try
            {
                fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str()); //kevin 20151001
            }
            catch(...)
            {
                MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordCleanPad - 3", asPath);
            }
        }
    }

    if(WriteProdLOG!=NULL)
        fclose(WriteProdLOG);
    InitialRecord();
}
//AI(W906-W2-CLEANPADLOG) 20260926: (end of lifted gate G-6)
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:1038-1108
// AI(W906-PT-MyProdRec-HeavyHalf) 20260811: TWO GOLDEN BUGS PRESERVED VERBATIM
//   (1) golden :1092 calls fclose(WriteProdLOG) inside the `else` branch that
//       was entered PRECISELY BECAUSE WriteProdLOG==NULL -- fclose(NULL) is
//       undefined behaviour and crashes on the MSVCRT this tree targets.  Its
//       sibling SaveRecordCleanPad has the same line COMMENTED OUT
//       (golden :1017 `//        fclose(WriteProdLOG);`), which is strong
//       evidence someone hit exactly this and only patched one of the two.
//   (2) golden :1107's final fclose(WriteProdLOG) is UNGUARDED (SaveRecord's
//       equivalent at :770 is guarded by `if(WriteProdLOG!=NULL)`), so the same
//       NULL can reach it by the second path too.
//   NOT FIXED, per the "do not fix golden's bugs" rule.  Flagged in the report.
// WRITES D:\HT9045_Log\Production_ByFile\... on the real machine
// (asProductionByFilePath, common.cpp:211).
//------------------------------------------------------------------------------
void TMyProductionRecord::SaveRecordForLeadYo()                                 //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
{
    AnsiString asFileNameByFile="";
    bool bHasFile=false;
    TStringList *SList;

    FileInfo().EnsureDirectoriesExist(asProductionByFileNamePath);              //Steven 20260504 : Ensure folder exists
    bHasFile=FileExists(asProductionByFileNamePath);
    if(bHasFile==false)
    {
        SList=new TStringList();
        SList->Clear();
        SList->Add(asDataTitle);
        SList->SaveToFile(asProductionByFileNamePath);
        SList->Clear();
        delete SList;
    }

    WriteProdLOG=fopen(asProductionByFileNamePath.c_str(), "a+");
    if(WriteProdLOG!=NULL)
    {
        try
        {
            fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str());   //kevin 20151001
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordForLeadYo - 1", asProductionByFileNamePath);
            asProductionByFileNamePath.sprintf("%s\\%s\\%s\\", AnsiString(asProductionByFilePath), asStation_ByFile, asType_ByFile);         //Steven 20200811 : 存檔失敗的時候,改個名字再存一次看看
            asFileNameByFile.sprintf("%s_%s_%s_1.csv", asLotID_ByFile, asStation_ByFile, asState_ByFile);     //Steven 20170123 (Jou) : 修改檔案命名格式
            asProductionByFileNamePath+=asFileNameByFile;
            FileInfo().EnsureDirectoriesExist(asProductionByFileNamePath);      //Steven 20260504 : Ensure folder exists
            fclose(WriteProdLOG);
            WriteProdLOG=fopen(asProductionByFileNamePath.c_str(), "a+");
            if(WriteProdLOG!=NULL)
            {
                try
                {
                    fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str());        //kevin 20151001
                }
                catch(...)
                {
                    MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordForLeadYo - 2", asProductionByFileNamePath);
                }
            }
        }
    }
    else
    {
        MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordForLeadYo - WriteProdLOG is NULL", asProductionByFileNamePath);
        asProductionByFileNamePath.sprintf("%s\\%s\\%s\\", AnsiString(asProductionByFilePath), asStation_ByFile, asType_ByFile);             //Steven 20200811 : 存檔失敗的時候,改個名字再存一次看看
        asFileNameByFile.sprintf("%s_%s_%s_1.csv", asLotID_ByFile, asStation_ByFile, asState_ByFile);     //Steven 20170123 (Jou) : 修改檔案命名格式
        asProductionByFileNamePath+=asFileNameByFile;
        FileInfo().EnsureDirectoriesExist(asProductionByFileNamePath);         //Steven 20260504 : Ensure folder exists
        fclose(WriteProdLOG);                                                   // AI(W906-PT-MyProdRec-HeavyHalf) 20260811: golden :1092 -- fclose(NULL), golden bug, PRESERVED (see this function's banner)
        WriteProdLOG=fopen(asProductionByFileNamePath.c_str(), "a+");
        if(WriteProdLOG!=NULL)
        {
            try
            {
                fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str());            //kevin 20151001
            }
            catch(...)
            {
                MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordForLeadYo - 3", asProductionByFileNamePath);
            }
        }
    }

    fclose(WriteProdLOG);                                                       // AI(W906-PT-MyProdRec-HeavyHalf) 20260811: golden :1107 -- unguarded, golden bug, PRESERVED
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:1110-1118
// Pure leaf, no gate.  NOTE for the main loop: this method landing REAL kills
// the "not even DECLARED" half of the absence premise recorded at
// aTester_Front.cpp:11662 / aTester_Rear.cpp:12313 / atester_32Site.cpp:95-96
// AI(pt-wave) 20260811 PT-W8 integrate: three golden/port citations on this
// file corrected after audit -- :1393 h:275->h:332 (eDataTotal), :941
// CMakeLists 1462->1466 (1462 is a comment, 1466 is the aHotPlateSubstrate.cpp
// entry), and :233->:95-96 above.  In every case the SUBSTANTIVE claim was
// correct and only the pointer was wrong; eDataTotal really is 71 in both
// trees, the two files really are in the same target, and atester_32Site.cpp
// really does record AddIndexPickVacuum as "not declared anywhere" (:95-96 --
// note the audit that found this one misread it as the :97-98 bullet, which
// says the opposite about two DIFFERENT methods).
// (W5_32S_ADD_INDEX_PICK_VACUUM).  Their SECOND premise -- TMySucker has no
// GetIOValue() -- is UNTOUCHED and still holds, so those gates must STAY.
//------------------------------------------------------------------------------
void TMyProductionRecord::AddIndexPickVacuum(double dKpa)                       //Sam 20230210 : 新增 VacuumUnit 通訊模組
{
    AnsiString s="";
    s.sprintf("%2.1f", dKpa);
    if(s=="999.0")
        asBuffer->Strings[eIndexPickVacuum]="NA";
    else
        asBuffer->Strings[eIndexPickVacuum]=s;
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:1120-1142 -- two of six arms GATED, see
// GATE REGISTER A-2 / G-3.  The golden `else` KEYWORD is inside each guard, so
// the surviving sprintf becomes the unconditional statement without any arm
// being reordered.
//------------------------------------------------------------------------------
AnsiString TMyProductionRecord::GetProdLogFileName(bool bTryCatch)
{
    AnsiString asFileName="";
    if(bTryCatch)
    {
#if 0 // GATE G-3a -- golden :1125-1128 (fLotInfo->edtStep / edtStage / lbledtStarTime absent -- see GATE REGISTER A-2)
        if(CUSTOMER_CODE==CC_SJ_Semiconductor)                                      //Steven 20250807 : HandlerID_InnerLotID_Step_Stage_TimeStamp
            asFileName.sprintf("%s_%s_%s_%s_%s_1.csv", IniConfig.SocketHandlerID, fLotInfo->edtSysLotID->Text, fLotInfo->edtStep->Text, fLotInfo->edtStage->Text, fLotInfo->lbledtStarTime->Text);
        else if(IniConfig.bO24_ProductionLogByLot)                                  //Steven 20250519 : Production Log By Lot
            asFileName.sprintf("%s_%s_%s_%s_1.csv", IniConfig.SocketHandlerID, fLotInfo->edtSysLotID->Text, fLotInfo->cbRunMode->Text, fLotInfo->lbledtStarTime->Text);     //Steven 20170123 (Jou) : 修改檔案命名格式
        else
#endif
            asFileName.sprintf("%s_%04d%02d%02d_1.csv", IniConfig.SocketHandlerID, SystemYear, SystemMonth, SystemDate);     //Steven 20170123 (Jou) : 修改檔案命名格式
    }
    else
    {
#if 0 // GATE G-3b -- golden :1134-1137 (same absence as G-3a)
        if(CUSTOMER_CODE==CC_SJ_Semiconductor)                                      //Steven 20250807 : HandlerID_InnerLotID_Step_Stage_TimeStamp
            asFileName.sprintf("%s_%s_%s_%s_%s.csv", IniConfig.SocketHandlerID, fLotInfo->edtSysLotID->Text, fLotInfo->edtStep->Text, fLotInfo->edtStage->Text, fLotInfo->lbledtStarTime->Text);
        else if(IniConfig.bO24_ProductionLogByLot)                                  //Steven 20250519 : Production Log By Lot
            asFileName.sprintf("%s_%s_%s_%s.csv", IniConfig.SocketHandlerID, fLotInfo->edtSysLotID->Text, fLotInfo->cbRunMode->Text, fLotInfo->lbledtStarTime->Text);     //Steven 20170123 (Jou) : 修改檔案命名格式
        else
#endif
            asFileName.sprintf("%s_%04d%02d%02d.csv", IniConfig.SocketHandlerID, SystemYear, SystemMonth, SystemDate);     //Steven 20170123 (Jou) : 修改檔案命名格式
    }
    return asFileName;
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:1144-1149
// Fully active.  Returns "D:\HT9045_Log\Production_Log\<YYYYMM>\" on the real
// machine (asTravelingLogPath, common.cpp:132).
//------------------------------------------------------------------------------
AnsiString TMyProductionRecord::GetProdLogFilePath()
{
    AnsiString asPath="";
    asPath.sprintf("%s\\%04d%02d\\", asTravelingLogPath, SystemYear, SystemMonth);
    return asPath;
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:1151-1262 -- WHOLE BODY GATED, see GATE
// REGISTER A-1 / G-4.  The method itself is DEFINED (empty) so its declaration
// in the header cannot become an undefined reference; golden's text is kept
// verbatim inside the guard.
// GOLDEN BUG NOTED (do not fix on un-gating): the last catch arm, golden :1252
// and :1256, writes asBuffer->CommaText (the 71-field record, not the 15-field
// whitelist row) and reports the failure under the WRONG function name
// "TMyProductionRecord::SaveRecordForLeadYo - 3" with the WRONG path variable
// asProductionByFileNamePath -- a copy-paste from SaveRecordForLeadYo.  That is
// also why asProductionByFileNamePath (cmydef.h:5600) is referenced from inside
// this function at all.  Preserved verbatim.
//------------------------------------------------------------------------------
void TMyProductionRecord::Save2DIDForJCET(AnsiString SubLOTID, AnsiString CustLOTID, AnsiString Modecode) //RogerYang 20251210 : JCET 2D FT1白名單/FT2比對功能
{
#if 0 // GATE G-4 -- fBarCode->iJCETWhitelistSN absent from TfBarCode (BarCode/BarCode.h, partial); see GATE REGISTER A-1, G-4
    AnsiString asJCET2DIDListPath, asStr;
    AnsiString asFileNameByFile="";
    bool bHasFile=false, bPass=false;
    TStringList *SList;
    int iBin;
    AnsiString sCust=SubLOTID.SubString(0,3);                                   //RogerYang 20260225 : 修改白名單路徑 \\客戶代碼(批次前3碼)\\批次_FTx_csv
    asJCET2DIDListPath.sprintf("%s%s\\%s\\%s_%s.csv", sWhite2DIDListLoc, sCust, SubLOTID, SubLOTID, Modecode);

    bHasFile=FileExists(asJCET2DIDListPath);

    if(bHasFile==false)
    {
        SList=new TStringList();
        SList->Clear();
        SList->Add(asJCET2DIDList);
        SList->SaveToFile(asJCET2DIDListPath);
        SList->Clear();
        delete SList;
        fBarCode->iJCETWhitelistSN=0; //序號從0開始
    }

    iBin=atoi(asBuffer->GetString(eTestCate).c_str());
    if(iBin>=0 && iBin<TEST_MAX_BIN &&
        asBuffer->Strings[eTestCate]!="E")                                      //RogerYang 20260302 : 避免Errbin被寫入
        bPass=Prod.bIsPassBin[iBin];
    else
        bPass=false;

    if(asBuffer->GetString(e2DCode).Pos("ERROR")>0)                             //Error不紀錄
        return;

    if(bPass==false)                                                            //不是pass bin不列入白名單
        return;

    if(asBuffer->GetString(eUnloadTime).Trim()=="")                             //AI(ht9045-barcode-flow) 20260611 (RogerYang) : 未實際放料不列入白名單
        return;

    fBarCode->iJCETWhitelistSN++;

    FileInfo().EnsureDirectoriesExist(asJCET2DIDListPath);                      //Steven 20260504 : Ensure folder exists

    asStr=asBuffer->Strings[e2DCode]         +","+
      asBuffer->Strings[eSBin]               +","+
      asBuffer->Strings[eTestCate]           +","+
      SubLOTID                               +","+
      CustLOTID                              +","+
      Modecode                               +","+
      asBuffer->GetString(eSOTTime).TrimLeft() +","+
      asBuffer->GetString(eEOTTime).TrimLeft() +","+
      asBuffer->Strings[eSiteNO]             +","+
      asBuffer->Strings[eLoadX]              +","+
      asBuffer->Strings[eLoadY]              +","+
      asBuffer->Strings[eUnloadX]            +","+
      asBuffer->Strings[eUnloadY]            +","+
      asBuffer->Strings[eWhichAuto]          +","+
      IntToStr(fBarCode->iJCETWhitelistSN)   ;

    WriteProdLOG=fopen(asJCET2DIDListPath.c_str(), "a+");
    if(WriteProdLOG!=NULL)
    {
        try
        {
            fprintf(WriteProdLOG, "%s \n", asStr.c_str());
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TMyProductionRecord::Save2DIDForJCET - 1", asJCET2DIDListPath);

            //存檔失敗的時候,改個名字再存一次看看
            asJCET2DIDListPath.sprintf("%s%s\\%s\\%s_%s_Retry.csv", sWhite2DIDListLoc, sCust, SubLOTID, SubLOTID, Modecode);  //RogerYang 20260225 : 修改白名單路徑 \\客戶代碼(批次前3碼)\\批次_FTx_csv
            FileInfo().EnsureDirectoriesExist(asJCET2DIDListPath);              //Steven 20260504 : Ensure folder exists
            fclose(WriteProdLOG);
            WriteProdLOG=fopen(asJCET2DIDListPath.c_str(), "a+");
            if(WriteProdLOG!=NULL)
            {
                try
                {
                    fprintf(WriteProdLOG, "%s \n", asStr.c_str());
                }
                catch(...)
                {
                    MyDBIProcess("Exception", "TMyProductionRecord::Save2DIDForJCET - 2", asJCET2DIDListPath);
                }
            }
        }
    }
    else
    {
        MyDBIProcess("Exception", "TMyProductionRecord::Save2DIDForJCET - WriteProdLOG is NULL", asJCET2DIDListPath);

        //存檔失敗的時候,改個名字再存一次看看
        asJCET2DIDListPath.sprintf("%s%s\\%s\\%s_%s_Retry.csv", sWhite2DIDListLoc, sCust, SubLOTID, SubLOTID, Modecode);   //RogerYang 20260225 : 修改白名單路徑 \\客戶代碼(批次前3碼)\\批次_FTx_csv
        FileInfo().EnsureDirectoriesExist(asJCET2DIDListPath);                  //Steven 20260504 : Ensure folder exists
        fclose(WriteProdLOG);
        WriteProdLOG=fopen(asJCET2DIDListPath.c_str(), "a+");
        if(WriteProdLOG!=NULL)
        {
            try
            {
                fprintf(WriteProdLOG, "%s \n", asBuffer->GetCommaText().c_str());            //kevin 20151001
            }
            catch(...)
            {
                MyDBIProcess("Exception", "TMyProductionRecord::SaveRecordForLeadYo - 3", asProductionByFileNamePath);
            }
        }
    }

    fclose(WriteProdLOG);
#endif // GATE G-4
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:1264-1383
// NOT A GATE.  Golden's ENTIRE body is inside one /* ... */ block comment
// (golden :1265 opens it right after the brace, golden :1382 closes it on the
// same line as the last statement), so an EMPTY body IS the faithful, complete
// translation.  The commented-out golden text is transcribed verbatim below,
// including the JHT tray-map log format and the ShowMyMessage/bNeedRestartSW
// tail, so that whoever revives it has the original in place.
// (Consequence: none of ShowMyMessage / bNeedRestartSW / LastSet / OFF_LINE /
// fLotInfo->edtASECL_LotID / edInsertion / edFlowID / edCustomerDevice is a
// real dependency of this translation unit.)
//------------------------------------------------------------------------------
void TMyProductionRecord::SaveJHT_Record(AnsiString ErrorLog)                   //JerryYang 20240405 : add
{   /*
    if(LastSet.iTester==OFF_LINE)
    {
        return;
    }

    if(bUse==false)                                                             //一開始就沒吸取ic 就不紀錄
    {                                                                           //目前 tray資料被清為0
        return;
    }

    AnsiString asStr, asPath, asFileName="", sJHT="", sTime, strPath;
    TStringList *SList;
    bool bHasFile=false, bResult=false;

    AnsiString sFilePath="",sFileName="",sFullFileName="";
    asPath.sprintf("%s\\", asTravelingLogPath);
    AddErrorLog(ErrorLog);

    if(DirectoryExists(asPath)==false)
    {
        ForceDirectories(asPath);
    }
    sTime=Now().FormatString("yyyymmddhhnnss");
//    asFileName.sprintf("%s@%s@%s.log",                     fLotInfo->edtASECL_LotID->Text, fLotInfo->edInsertion->Text, fLotInfo->edFlowID->Text);     //Steven 20170123 (Jou) : 修改檔案命名格式

    asFileName.sprintf("%s@WAFERLOT@%sXX@99@SWBIN@%s.txt", fLotInfo->edtASECL_LotID->Text, fLotInfo->edInsertion->Text, fLotInfo->edFlowID->Text);     //Steven 20170123 (Jou) : 修改檔案命名格式

    asPath+=asFileName;

//    bHasFile=FileExists(asPath);
//    if(bHasFile==false)
//    {
//        SList=new TStringList();    //Steven 20170518 (jou) : 換位置改善UPH
//        SList->Clear();
//        SList->Add(asDataTitle);    //Steven 20200416 : 整合Eventlog的title
//        SList->SaveToFile(asPath);
//        SList->Clear();     //Ifor 20170603 (wei) TStringList 刪除前先 Clean
//        delete SList;       //Steven 20170518 (jou) : 換位置改善UPH
//    }

    //2024/04/10 16:26:30 $ CustomerDevice123 $ LOTID123 $ Picker# H $ Bin:1(1) $ TrayID: 20240327-00000() $ Row(Y):1 $ Col(X):2 $ DUT#1(zz) $ SoftBin:1

    sJHT.sprintf("%04d/%02d/%02d %02d:%02d:%02d $ %s $ %s $ Picker#%s $ Bin: %s(%s) $ TrayID: (%s) $ Row(Y): %s $ Col(X): %s $ DUT#%s(zz) $ SoftBin:%s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec,
                                                                                                                        fLotInfo->edCustomerDevice->Text,
                                                                                                                        fLotInfo->edtASECL_LotID->Text,
                                                                                                                        asBuffer->Strings[eOutArmShuttlePick],
                                                                                                                        asBuffer->Strings[eTestCate],
                                                                                                                        asBuffer->Strings[eTestCate],
                                                                                                                        asBuffer->Strings[eTrayID],
                                                                                                                        asBuffer->Strings[eUnloadY],
                                                                                                                        asBuffer->Strings[eUnloadX],
                                                                                                                        asBuffer->Strings[eSiteNO],
                                                                                                                        asBuffer->Strings[eTestCate]);

    WriteProdLOG=fopen(asPath.c_str(), "a+");
    if(WriteProdLOG!=NULL)
    {
        try
        {
            fprintf(WriteProdLOG, "%s \n", sJHT); //kevin 20151001
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TMyProductionRecord::SaveRecord - 1", asPath);

            //Steven 20200811 : 存檔失敗的時候,改個名字再存一次看看
            //==>
            asPath.sprintf("%s\\%04d%02d\\", asTravelingLogPath, SystemYear, SystemMonth);
            asFileName.sprintf("%s_%04d%02d%02d_1.csv", PC_NAME, SystemYear, SystemMonth, SystemDate);     //Steven 20170123 (Jou) : 修改檔案命名格式
            asPath+=asFileName;
            fclose(WriteProdLOG);
            WriteProdLOG=fopen(asPath.c_str(), "a+");
            if(WriteProdLOG!=NULL)
            {
                try
                {
                    fprintf(WriteProdLOG, "%s \n", asBuffer->CommaText); //kevin 20151001
                }
                catch(...)
                {
                    MyDBIProcess("Exception", "TMyProductionRecord::SaveRecord - 2", asPath);
                }
            }
            //<==
            //Steven 20200811 : 存檔失敗的時候,改個名字再存一次看看
        }
    }
    else
    {
        MyDBIProcess("Exception", "TMyProductionRecord::SaveRecord - WriteProdLOG is NULL", asPath);

        //Steven 20200811 : 存檔失敗的時候,改個名字再存一次看看
        //==>
        asPath.sprintf("%s\\%04d%02d\\", asTravelingLogPath, SystemYear, SystemMonth);
        asFileName.sprintf("JHT_%04d%02d%02d_1.csv", SystemYear, SystemMonth, SystemDate);     //Steven 20170123 (Jou) : 修改檔案命名格式
        asPath+=asFileName;
        fclose(WriteProdLOG);
        WriteProdLOG=fopen(asPath.c_str(), "a+");
        if(WriteProdLOG!=NULL)
        {
            try
            {
                fprintf(WriteProdLOG, "%s \n", sJHT); //kevin 20151001
            }
            catch(...)
            {
                MyDBIProcess("Exception", "TMyProductionRecord::SaveRecord - 3", asPath);
            }
        }
        //<==
        //Steven 20200811 : 存檔失敗的時候,改個名字再存一次看看

        bNeedRestartSW=true;
        ShowMyMessage("tray map log寫入失敗", "Write tray map log failed.");
    }

    fclose(WriteProdLOG);        */
}
//------------------------------------------------------------------------------
// golden Public/MyProductionRecord.cpp:561-573 -- GATED, see GATE REGISTER G-5
// (homecoming: aHotPlateSubstrate.cpp:773 already defines a no-op with external
// linkage in the SAME archive).  ORDER DEVIATION: golden puts this at :561,
// between GetErrorLog and AddGroundRecord; the append-only rule for this mirror
// forces it here.  See this section's banner "ORDER DEVIATION".
// Swap = delete aHotPlateSubstrate.cpp:773, then delete this guard pair -- and
// read G-5(b) FIRST: un-gating turns 30+ live call sites into real writes to
// D:\HT9045_Log\Production_Log on this machine.
//------------------------------------------------------------------------------
//AI(W906-W2-PRODLOG) 20260926: GATE G-5 LIFTED (homecoming) -- the stand-in at aHotPlateSubstrate.cpp (was :773, now :942) is retired in the same commit; SaveRecord writes under asTravelingLogPath, which ctest redirects via W906_PRODLOG_ROOT (common.cpp) -- production unchanged: golden D:\HT9045_Log\Production_Log
void TMyProductionRecord::AddErrorRecord(AnsiString ErrorLog, bool bAddPosition, int iXPos, int iYPos, int iXPitch, int iXPitch2, int iYPitch)
{
    if(bAddPosition)                                                            //Steven 20190110 : Error Log Add Motor Position.
    {
        asBuffer->Strings[eUnloadXPos]  =AnsiString(iXPos);
        asBuffer->Strings[eUnloadYPos]  =AnsiString(iYPos);

        asBuffer->Strings[eUnloadXpPos]  =AnsiString(iXPitch);                  //JerryYang 20210119 : add x,y pitch位置
        asBuffer->Strings[eUnloadXp2Pos] =AnsiString(iXPitch2);
        asBuffer->Strings[eUnloadYpPos]  =AnsiString(iYPitch);
    }
    SaveRecord(ErrorLog);
}
//AI(W906-W2-PRODLOG) 20260926: (end of lifted gate G-5)
//------------------------------------------------------------------------------
