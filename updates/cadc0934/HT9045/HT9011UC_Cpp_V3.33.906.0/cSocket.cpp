// =============================================================================
//  cSocket.cpp  --  per-socket / per-arm / per-lot pass-fail-bin counters
//
//  Faithful translation of golden cSocket.cpp (1271 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 (group "socket").
//
//  ROLE: implements every method of TMySocket / TArm / TLotSummary /
//  TEST_CATEGORY (declared cSocket.h) and defines the file's 8 extern globals.
//  cSocket.cpp:108 (`unsigned int iBinCT[TEST_MAX_BIN];`, a TMySocket data
//  member) is the "per-socket bin count array" the AGENTS.md/CLAUDE.md project
//  memory flags as NOT the same thing as ProductData -- kept exactly where
//  golden has it (TMySocket member, this translation's cSocket.h line ~18,
//  golden cSocket.h:17/cSocket.cpp is its home file); nothing in this wave
//  moves or aliases it.
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim -- all 63 golden functions, nothing omitted):
//     TMySocket:  TMySocket() :23 / ~TMySocket() :46 / ClearALLCT() :61 /
//       ClearBySite() :84 / SetbBinCodeStatus :91 / SetTesterBin :100 /
//       GetSelBinCT :147 / GetSelTrayCT :152 / GetTotal :173 / GetPCA :178 /
//       GetBySitePCA :184 / GetPassCT :190 / GetFailCT :195 / GetIFError :200 /
//       SetPassCT :205 / SetFailCT :221 / SetIFErr :237 / SetBinCT :242 /
//       GetByBinLowYieldPassCT :248 / GetByBinArmYieldPassCT :253 /
//       GetByBinSiteYieldPassCT :258 / SetByBinLowYieldPassCT :263 /
//       SetByBinArmYieldPassCT :268 / SetByBinSiteYieldPassCT :273 /
//       GetByBinLowYieldPassPCA :278 / GetByBinArmYieldPassPCA :284 /
//       GetByBinSiteYieldPassPCA :290                                   (27 fns)
//     TArm:  TArm(AnsiString) :298 / ReadFile :327 / ~TArm() :499 /
//       WriteFile :527 / GetPCA :574 / GetBySitePCA :580 / GetPassCT :586 /
//       GetFailCT :595 / GetTotalCT :604 / GetSelBin :610 / ClearALLCT() :619 /
//       ClearALLCT(ROW,COL) :636 / ClearBySite :646 / SetArmBinCodeStatus :663 /
//       SetArmSKTData :668 / InitContactCT :679 / SetContactCT :684 /
//       GetContactCT :689 / SetPassCT(ROW,COL,V) :694 / SetFailCT(ROW,COL,V) :702 /
//       SetBinCT(ROW,COL,Bin,V) :710 / SetIFErr(ROW,COL,V) :720 /
//       GetByBinLowYieldPCA :728                                        (23 fns)
//     TLotSummary:  TLotSummary() :745 / ~TLotSummary() :750 /
//       ClearAllData :754 / ClearRTData :765 / SetIsRTBin :780 / AddCount :800 /
//       AddByLotCount :806 / AddByLotLoadCount :867 / ReadFile :928 /
//       WriteFile :969                                                  (10 fns)
//     TEST_CATEGORY:  ClearCount :1001 / UpdataCount :1029 / UpdataYield :1256
//                                                                         (3 fns)
//     27+23+10+3 = 63 = golden_fns.
//   Plus the 8 file-scope extern globals (golden :14-21): ArmData/ArmDataLot/
//   ArmHistory/ArmData_AutoClean (TArm*[3]), LotSummary (TLotSummary),
//   TastCategory/OldControlBinCategory/NowControlBinCategory (TEST_CATEGORY).
//   SATISFIED-BY-SUBSTRATE (already real in this tree, nothing shimmed here):
//     Prod / TestIF / TestIF_File (cprod.h), IsNNMode()+None_NN+NN_1Row
//     (atester_shims.h body atester_shims.cpp:242 + MachineType.h enum),
//     TestSocket (aHotPlateSubstrate.h:635), MyDBIProcess (aHotPlateSubstrate.h:924),
//     MyForceDirectories/WriteIniData/CheckAndReadIniData (common.h),
//     FileDataCompare (cprod.h:3288, body cprod.cpp:1558),
//     StringReplace/TReplaceFlags/rfReplaceAll (vclcompat/SysUtils.h via the
//     vcl_compat.h umbrella), iTestBinCount/NEW_MAX_Index_Col/iAutoRight/
//     sTotalLotID/iE1Count/iE2Count/iE3Count/iByBinTotal[] (cmydef.h).
//
//  GATE REGISTER -- exactly ONE gate family (2 call sites, same missing
//  member, both inside TLotSummary's "by inner lot" cross-check):
//   (1) fSCKART->iInfo_MultiLotCnt / fSCKART->sInfoArr_InnerLotID[j]
//       -- golden :848/:850 (AddByLotCount) and :910/:912 (AddByLotLoadCount).
//       Grepped the whole port tree (excluding build*/) before writing this:
//       forms/fSCKART.h's `class TfSCKART` (the real global `fSCKART`, the
//       ONLY object this tree's `fSCKART->` can mean) has NO `iInfo_MultiLotCnt`
//       or `sInfoArr_InnerLotID` member -- confirmed by reading the whole class
//       body (forms/fSCKART.h:60-140). Those two golden TfSCKART fields DO
//       have a translated home, but on a DIFFERENT struct with no global
//       instance this file can reach: Automation/SCK_ART_Remainder.h's
//       `struct SckArtRemainderState` (iInfo_MultiLotCnt :715,
//       sInfoArr_InnerLotID[5] :718) is a plain value type every one of its
//       own module's functions takes as a `SckArtRemainderState &st`
//       parameter -- there is no extern global of that type for an unrelated
//       TU (this one) to name. This is therefore a genuine "no compiled body
//       anywhere reachable" case for the fSCKART-shaped call, not a guess.
//       Handled with the tree's #if 0/#else macro-pair idiom (aRotateKIT.cpp
//       :43-73 precedent): golden's call stays VERBATIM in the #if 0 arm; the
//       ACTIVE arm skips the loop (bound 0).
//       WHY 0 IS THE FAITHFUL DEFAULT: `Automation/SCK_ART_Remainder.cpp:82`'s
//       ctor initializer list sets golden's own field to `iInfo_MultiLotCnt(0)`
//       -- i.e. golden's OWN object starts this counter at 0 before any
//       multi-lot/ART configuration is loaded, and a `for(j=0;j<0;j++)` loop
//       runs zero times, which is bit-for-bit what a 0-count loop bound does
//       in golden too. Not an invented value -- it is golden's own documented
//       ctor default.
//       BEHAVIOUR DELTA, STATED PLAINLY: on a real machine running a
//       multi-inner-lot ATK/ART job (`iInfo_MultiLotCnt>1`), golden's
//       AddByLotCount/AddByLotLoadCount cross the 2D-mapping CSV against each
//       configured inner lot ID and bump `iByLotCountCategory[j]` /
//       `iByLotTotalCategory[j]` / `iByLotLoadCount[j]`; this build's gated
//       arm never touches those three arrays, so per-inner-lot ATK/ART
//       counting stays at zero on hardware until this gate is retired.
//       Retire the moment either (a) `SckArtRemainderState` grows a reachable
//       global instance, or (b) TfSCKART gains the two fields directly.
//
//  VCL/Borland conversions:
//    * `_fastcall` dropped everywhere (golden :23/:46/:61/... throughout);
//      matches the already-translated cSocket.h.
//    * `ZeroMemory` (golden :756-758/:1003-1019) kept AS-IS -- real WinAPI
//      macro, already in use tree-wide (e.g. acarry.cpp:7358) via the
//      <windows.h> MachineDefine.h pulls in; not a VCL-only construct.
//    * `unsigned long` -> `AnsiString` is AMBIGUOUS in this tree's
//      vclcompat/AnsiString.h: it declares `AnsiString(int)`,
//      `AnsiString(unsigned int)`, `AnsiString(long)`, `AnsiString(long long)`
//      and `AnsiString(double)` but no `AnsiString(unsigned long)` overload,
//      and on this LLP64 (MinGW/Win32) build `unsigned long` converts to
//      BOTH `long` and `long long` by an equal-rank standard conversion --
//      confirmed by an actual compile (`g++ ... error: call of overloaded
//      'AnsiString(long unsigned int&)' is ambiguous`), not a guess. Golden's
//      TMySocket::Pass/Fail/Total and every TArm::Get*CT() return exactly
//      `unsigned long`, and TArm's ClearALLCT/SetArmSKTData/SetPassCT/
//      SetFailCT/SetBinCT/SetIFErr all wrap one of those calls directly in
//      `AnsiString(...)` when mirroring into the sPass/sFail/sTotal/iIFErr
//      TStringLists that SECS GEM reads per-site. Every such call below adds
//      an explicit `(long)` cast (same disambiguation idiom already used
//      tree-wide for the analogous `AnsiString((int)EnumValue)` pattern, e.g.
//      tests/test_uHGemClass.cpp:349) -- NOT a value change: every counter
//      here is a test/bin tally, always non-negative and always far below
//      LONG_MAX, so `(long)` and the golden `unsigned long` format to the
//      identical decimal text.
//    * `FileDataCompare(char*,char*)` (golden :365/:1558 decl at cprod.h:3288)
//      takes non-const `char*`; called here with `AnsiString::c_str()`
//      (`const char*`), so every call adds `const_cast<char*>(...)` -- same
//      fix already established by cprod.cpp:1734 for this exact function.
//    * `Str.sprintf(...)`, `AnsiString::Pos/SubString/Length`, TStringList
//      `Strings[i]=` / `CommaText=` / `Add` / `Clear` / `LoadFromFile` /
//      `SaveToFile` all used exactly as golden wrote them (house-style
//      vclcompat surface, no rewrite).
//    * golden's unparenthesised `A && (B && C) || D` at TLotSummary::SetIsRTBin
//      (:786-789) is reproduced with the SAME line breaks and NO added
//      parentheses (`&&` binds tighter than `||` in both C++ and golden's own
//      Object Pascal-influenced author's intent here -- adding clarifying
//      parens would be an edit, so none is added).
//    * SOFT_SIMULTE is NOT defined; golden has no #ifdef SOFT_SIMULTE in this
//      unit.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "cSocket.h" / "cprod.h" / "cmydef.h" / "common.h" / "mymessbox.h" /
//     "SCK_ART.h" / "cinitial.h"
//   ; #pragma package(smart_init)
//
//   NOT re-included here, and why:
//     mymessbox.h -- golden's own 1271 lines never call a MyMessageBox/message-
//       box symbol; contributes nothing used (checked by reading the whole
//       decoded file, not grepped-and-assumed).
//     cinitial.h  -- in THIS tree cinitial.h only declares the GA-2-C1 slice
//       (GetSHCHKPos/InitialSuckerName/.../InitialHeaterDoor); IsNNMode() (the
//       one golden cinitial.h symbol this file calls, golden cinitial.h:60)
//       lives in atester_shims.h instead (body atester_shims.cpp:242) -- see
//       SATISFIED-BY-SUBSTRATE above.
//     SCK_ART.h   -- the one golden SCK_ART.h symbol reached here is
//       `fSCKART` itself (forms/fSCKART.h in this tree, not Automation/
//       SCK_ART.h) -- included below for the GATE (1) call sites' benefit
//       even though the golden call is never compiled (matches the
//       aRotateKIT.cpp precedent of including a header purely so a `#if 0`
//       arm reads against a real declaration).
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)
#include "cSocket.h"                // this unit's own contract (TMySocket/TArm/TLotSummary/TEST_CATEGORY + the 8 extern globals defined below)
#include "cprod.h"                  // Prod (iT6CatData/iIfErrorT6/iT6PosCate/iTrayType/bART6Tray/bCateRTo6Tray), TestIF_File.bLowYieldAlarmByBin, TestIF.iSiteMap, FileDataCompare decl
#include "cmydef.h"                 // TEST_MAX_BIN family already via MachineType.h; iTestBinCount, NEW_MAX_Index_Col, iAutoRight, sTotalLotID, iE1Count/iE2Count/iE3Count, iByBinTotal[]
#include "common.h"                 // MyForceDirectories, WriteIniData, CheckAndReadIniData
#include "aHotPlateSubstrate.h"     // MyDBIProcess; TestSocket (TMyKitSuck, golden MyKitSuck.h -> this tree's substrate)
#include "atester_shims.h"          // IsNNMode() (golden cinitial.h:60; body atester_shims.cpp:242)
#include "forms/fSCKART.h"          // fSCKART -- referenced only inside GATE (1)'s #if 0 arm (see banner)
//------------------------------------------------------------------------------
TArm *ArmData[3];
TArm *ArmDataLot[3];
TArm *ArmHistory[3];
TArm *ArmData_AutoClean[3];                                                     //ChungHung 20131225 add

//==============================================================================
//  ArmData / ArmDataLot / ArmHistory / ArmData_AutoClean BOOTSTRAP
//  AI(pt-wave) 20260811 PT-W8.
//
//  WHY THIS EXISTS.  PT-W8 retired the one-line ProcessCount stand-in
//  (atester_shims.cpp:148) and landed golden's real body
//  (atester_ProcessCount.cpp, golden :1749).  That body's first statements are
//  `ArmData[Index]->SetContactCT(1);` / `ArmHistory[Index]->SetContactCT(1);`,
//  and the four arrays above are declared exactly as golden declares them --
//  bare pointer arrays, hence NULL.  Golden CONSTRUCTS them somewhere this port
//  does not have yet: main.cpp:2141-2148, a `for(int i=0;i<3;i++)` loop.
//  main.cpp is the TfMain form unit and is not translated, so nothing ever ran
//  those four lines and W5_Atester32Site SEGFAULTed at cSocket.cpp:841 with
//  `this=0x0` the moment the real body became reachable.
//
//  This is the SAME class as PT_CAMPAIGN_PLAN section 8's NULL-global table,
//  and the same treatment its PickFromHPList/PlaceToCleanList entry already
//  received (aHotPlateSubstrate.cpp, HPListBootstrap).  NOTE FOR THAT TABLE:
//  these four are NOT in it.  The 20260807 sweep matched golden main.cpp's
//  `X = new T;` sites against port files defining same-named bare pointers, and
//  golden's form here is `ArmData[i] = new TArm(...)` -- an ARRAY-ELEMENT
//  assignment, which that pattern does not match.  The table is therefore
//  incomplete, not wrong; re-run nullsweep.py with array-element handling.
//
//  ON STATIC-INITIALISATION ORDER.  Standard C++ gives no cross-TU ordering
//  guarantee, so this is only safe because all four are read from ordinary
//  runtime functions and NEVER from another TU's static initialiser -- checked
//  across every reader (SECSGEM/uHGemHT9045_SV.cpp, cSocket.cpp,
//  atester_ProcessCount.cpp, Automation/auto9045.cpp, csystem.cpp,
//  Automation/SCK_ART.cpp, SECSGEM/uHGemHT9045.cpp), not assumed.  TArm's ctor
//  (cSocket.cpp:453) only allocates its own TStringLists/TMySockets and fills
//  its own fields -- it reads no other global, so it cannot depend on another
//  TU being initialised first.
//
//  It does what golden main.cpp:2141-2148 does and nothing more, and it retires
//  the moment main.cpp lands.
//==============================================================================
namespace {
struct ArmDataBootstrap
{
    ArmDataBootstrap()
    {
        for(int i=0; i<3; i++)                                                  //Steven 20110801 : 改成初始化後讀檔
        {
            if(ArmData[i]          == NULL) ArmData[i]          = new TArm("Arm"+AnsiString(i));            // golden main.cpp:2143
            if(ArmHistory[i]       == NULL) ArmHistory[i]       = new TArm("ArmHis"+AnsiString(i));         // golden main.cpp:2144
            if(ArmData_AutoClean[i]== NULL) ArmData_AutoClean[i]= new TArm("ArmAutoClean"+AnsiString(i));   // golden main.cpp:2145
            if(ArmDataLot[i]       == NULL) ArmDataLot[i]       = new TArm("ArmByLot"+AnsiString(i));       // golden main.cpp:2146
        }
    }
};
ArmDataBootstrap g_armDataBootstrap;
}
//==============================================================================
TLotSummary LotSummary;
TEST_CATEGORY TastCategory;
TEST_CATEGORY OldControlBinCategory;                                            //Sam 20200525 : Control Bin
TEST_CATEGORY NowControlBinCategory;                                            //Sam 20200525 : Control Bin
//------------------------------------------------------------------------------
TMySocket::TMySocket()
{
    sBinPassFail=new TStringList();
    sBinCT      =new TStringList();

    for(int i=0; i<TEST_MAX_BIN; i++)
    {
        sBinPassFail->Add("-1");
        sBinCT      ->Add("0");
    }

    ClearALLCT();
    for(int i=0; i<TEST_MAX_BIN; i++)
    {
        SetByBinLowYieldPassFail[i]=false;                                      //Steven 20140828 : By Bin Yield Monitor
        SetByBinArmYieldPassFail[i]=false;                                      //Steven 20140828 : By Bin Arm Yield Monitor
        SetByBinSiteYieldPassFail[i]=false;                                     //Steven 20140828 : By Bin Site Yield Monitor
        SetBinPassFail[i]=-1;
        iBinCT[i]=0;
        iByBinTotal[i]=0;                                                       //kevin 20180705 (wei) bin 數量 Bin total[0]
    }
}
//------------------------------------------------------------------------------
TMySocket::~TMySocket()
{
    try
    {
        sBinPassFail->Clear();                                                  //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        sBinCT->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete sBinPassFail;
        delete sBinCT;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~TMySocket");
    }
}
//------------------------------------------------------------------------------
void TMySocket::ClearALLCT()
{
    Pass=0;
    Fail=0;
    Total=0;
    iIFErr=0;

    BySiteFail=0;                                                               //kevin 20130710 by site yield record alarm 後清除}
    BySitePass=0;                                                               //kevin 20130710 by site yield record alarm 後清除}
    BySiteTotal=0;                                                              //kevin 20130710 by site yield record alarm 後清除}

    iByBinLowYieldPass =0;                                                      //Steven 20140828 : By Bin Yield Monitor
    iByBinArmYieldPass =0;                                                      //Steven 20140828 : By Bin Arm Yield Monitor
    iByBinSiteYieldPass=0;                                                      //Steven 20140828 : By Bin Site Yield Monitor

    for(int i=0; i<TEST_MAX_BIN; i++)
    {
        iBinCT[i]=0;
        sBinPassFail->Strings[i]="0";
        sBinCT      ->Strings[i]="0";
    }
}
//------------------------------------------------------------------------------
void TMySocket::ClearBySite()
{
    BySiteFail=0;                                                               //kevin 20130710 by site yield record alarm 後清除}
    BySitePass=0;                                                               //kevin 20130710 by site yield record alarm 後清除}
    BySiteTotal=0;                                                              //kevin 20130710 by site yield record alarm 後清除}
}
//------------------------------------------------------------------------------
void TMySocket::SetbBinCodeStatus(int Bin, int Status, bool bByBinLowYieldPass, bool bByBinArmYieldPass, bool ByBinSiteYieldPass)//set BinPassFail
{
    SetBinPassFail[Bin]             =Status;
    sBinPassFail->Strings[Bin]      =AnsiString(Status);
    SetByBinLowYieldPassFail[Bin]   =bByBinLowYieldPass;                        //Steven 20140828 : By Bin Yield Monitor
    SetByBinArmYieldPassFail[Bin]   =bByBinArmYieldPass;                        //Steven 20140828 : By Bin Arm Yield Monitor
    SetByBinSiteYieldPassFail[Bin]  =ByBinSiteYieldPass;                        //Steven 20140828 : By Bin Site Yield Monitor
}
//------------------------------------------------------------------------------
void TMySocket::SetTesterBin(int BinValue)
{
    if(BinValue>=iTestBinCount || BinValue<0)                                   //Steven 20140509 : Modify
    {
        iIFErr++;
    }
    else
    {
        iBinCT[BinValue]++;
        if(SetByBinLowYieldPassFail[BinValue])                                  //Steven 20140828 : By Bin Yield Monitor
        {
            iByBinLowYieldPass++;
        }

        if(SetByBinArmYieldPassFail[BinValue])                                  //Steven 20140828 : By Bin Arm Yield Monitor
        {
            iByBinArmYieldPass++;
        }

        if(SetByBinSiteYieldPassFail[BinValue])                                 //Steven 20140828 : By Bin Site Yield Monitor
        {
            iByBinSiteYieldPass++;
        }

        sBinCT->Strings[BinValue]=AnsiString(iBinCT[BinValue]);
    }

    if(BinValue>=iTestBinCount || BinValue<0)                                   //Steven 20111026 : 記憶體破壞      //Steven 20140424 : iMaxBin --> iTestBinCount
    {
        Fail++;
        BySiteFail++;                                                           //kevin 20130710 by site yield record alarm 後清除}
    }
    else if(SetBinPassFail[BinValue]==1)                                        //pass //Steven 20240701 : 0 --> 1 (Pass為 1)
    {
        Pass++;
        BySitePass++;                                                           //kevin 20130710 by site yield record alarm 後清除
    }
    else
    {
        Fail++;
        BySiteFail++;                                                           //kevin 20130710 by site yield record alarm 後清除
    }

    Total++;
    BySiteTotal++;                                                              //kevin 20130710 by site yield record alarm 後清除
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetSelBinCT(int Bin)
{
    return iBinCT[Bin];
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetSelTrayCT(int iTray)
{
    int iT6=0;
    int iTrayCnt=0;
    for(int i=0; i<iTestBinCount; i++)
    {
        iT6=(iT6>iTestBinCount-1)?Prod.iIfErrorT6:Prod.iT6CatData[i];
        if(iT6==iTray)
        {
            iTrayCnt+=iBinCT[i];
        }
    }

    if(iTray==Prod.iIfErrorT6)
    {
        iTrayCnt+=iIFErr;
    }

    return iTrayCnt;
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetTotal()
{
    return Pass+Fail;
}
//------------------------------------------------------------------------------
double TMySocket::GetPCA()
{
    double dRetrun=ChangeToFloat(double(Pass), double(Total));                  //Steven 20250820 : 針對除以0加上保護
    return dRetrun;
}
//------------------------------------------------------------------------------
double TMySocket::GetBySitePCA()                                                //kevin 20130710 by head sit計數
{
    double dRetrun=ChangeToFloat(double(BySitePass), double(BySiteTotal));
    return dRetrun;
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetPassCT()
{
    return Pass;
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetFailCT()
{
    return Fail;
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetIFError()
{
    return iIFErr;
}
//------------------------------------------------------------------------------
void TMySocket::SetPassCT(double Value)
{
    Pass=Value;
    Total=Pass+Fail;

    if(Value==0)                                                                //Steven 20140926 : 修正Yield顯示超過100%的問題
    {
        iByBinLowYieldPass =0;                                                  //Steven 20140828 : By Bin Yield Monitor
        iByBinArmYieldPass =0;                                                  //Steven 20140828 : By Bin Arm Yield Monitor
        iByBinSiteYieldPass=0;                                                  //Steven 20140828 : By Bin Site Yield Monitor
    }

    BySitePass=Value;                                                           //kevin 20130710 by HEAD Site
    BySiteTotal=BySitePass+BySiteFail;                                          //kevin 20130710 by Head Site
}
//------------------------------------------------------------------------------
void TMySocket::SetFailCT(double Value)
{
    Fail=Value;
    Total=Pass+Fail;

    if(Value==0)                                                                //Steven 20140926 : 修正Yield顯示超過100%的問題
    {
        iByBinLowYieldPass =0;                                                  //Steven 20140828 : By Bin Yield Monitor
        iByBinArmYieldPass =0;                                                  //Steven 20140828 : By Bin Arm Yield Monitor
        iByBinSiteYieldPass=0;                                                  //Steven 20140828 : By Bin Site Yield Monitor
    }

    BySiteFail=Value;                                                           //kevin 20130710 by Head Site
    BySiteTotal=BySitePass+BySiteFail;                                          //kevin 20130710 by Head Site
}
//------------------------------------------------------------------------------
void TMySocket::SetIFErr(double Value)                                          //Steven 20110801
{
    iIFErr=Value;
}
//------------------------------------------------------------------------------
void TMySocket::SetBinCT(int Bin, double Value)
{
    iBinCT[Bin]=Value;
    sBinCT->Strings[Bin]=AnsiString(Value);
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetByBinLowYieldPassCT()                               //Steven 20140828 : By Bin Yield Monitor
{
    return iByBinLowYieldPass;
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetByBinArmYieldPassCT()                               //Steven 20140828 : By Bin Arm Yield Monitor
{
    return iByBinArmYieldPass;
}
//------------------------------------------------------------------------------
unsigned long TMySocket::GetByBinSiteYieldPassCT()                              //Steven 20140828 : By Bin Site Yield Monitor
{
    return iByBinSiteYieldPass;
}
//------------------------------------------------------------------------------
void TMySocket::SetByBinLowYieldPassCT(unsigned long Value)                     //Steven 20140828 : By Bin Yield Monitor
{
    iByBinLowYieldPass=Value;
}
//------------------------------------------------------------------------------
void TMySocket::SetByBinArmYieldPassCT(unsigned long Value)                     //Steven 20140828 : By Bin Arm Yield Monitor
{
    iByBinArmYieldPass=Value;
}
//------------------------------------------------------------------------------
void TMySocket::SetByBinSiteYieldPassCT(unsigned long Value)                    //Steven 20140828 : By Bin Site Yield Monitor
{
    iByBinSiteYieldPass=Value;
}
//------------------------------------------------------------------------------
double TMySocket::GetByBinLowYieldPassPCA()                                     //Steven 20140828 : By Bin Yield Monitor
{
    double dRetrun=ChangeToFloat(double(iByBinLowYieldPass), double(Total));    //Steven 20250820 : 針對除以0加上保護
    return dRetrun;
}
//------------------------------------------------------------------------------
double TMySocket::GetByBinArmYieldPassPCA()                                     //Steven 20140828 : By Bin Arm Yield Monitor
{
    double dRetrun=ChangeToFloat(double(iByBinArmYieldPass), double(Total));
    return dRetrun;
}
//------------------------------------------------------------------------------
double TMySocket::GetByBinSiteYieldPassPCA()                                    //Steven 20140828 : By Bin Site Yield Monitor
{
    double dRetrun=ChangeToFloat(double(iByBinSiteYieldPass), double(Total));
    return dRetrun;
}
//------------------------------------------------------------------------------
//this is Class TArm Start
//------------------------------------------------------------------------------
TArm::TArm(AnsiString FileName)                                                 //Steven 20110801 : 改成初始化後讀檔
{
    sPass =new TStringList();
    sFail =new TStringList();
    sTotal=new TStringList();
    iIFErr=new TStringList();

    Pass=0;
    Fail=0;
    Total=0;
    iSKETInArmCT=0;
    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
//        for(j=0; j<NEW_MAX_Index_Col; j++)                                    //Steven 20110420 : 用這個會項賽
            sPass ->Add("0");
            sFail ->Add("0");
            sTotal->Add("0");
            iIFErr->Add("0");

            ArmSKET[i][j]=new TMySocket();
        }
    }
    iContactCT=0;
    Name=FileName;
    bHasFile=false;
}
//------------------------------------------------------------------------------
void TArm::ReadFile()                                                           //Steven 20110801 : 改成初始化後讀檔
{
    extern AnsiString W906_MachineRecordRedirect(const AnsiString&); AnsiString asFileName=W906_MachineRecordRedirect("D:\\HT9045\\system\\"+Name+".dat");   //AI(W906-MT-FIX1) 20260926: test containment (ctest Automation wrote these), golden literal when W906_MACHINERECORD_DIR is unset
    AnsiString asBackFile=W906_MachineRecordRedirect("D:\\HT9045\\system\\"+Name+"_backup.dat");   //AI(W906-MT-FIX1) 20260926
    MyForceDirectories("D:\\HT9045\\system\\");
    bHasFile=FileExists(asFileName);

    unsigned long size=0;
    unsigned long temp[MAX_SOCKET_ROW][MAX_SOCKET_COL][19]={0};                 //jou 2012-09-23 預設值為0
    unsigned long tempMax[4][8][TEST_MAX_BIN]={0};                              //jou 2012-09-23 預設值為0
    unsigned long tempMax4[4][8][TEST_MAX_BIN+4]={0};                           //jou 2014-04-22 因為RS232 100 BIN需要再加4
    unsigned long tempMax5[4][8][104]={0};  //kevin 20140614
    AnsiString asIniFileName=W906_MachineRecordRedirect("D:\\HT9045\\system\\"+Name+".ini");   //AI(W906-MT-FIX1) 20260926
    AnsiString Str;

    if(bHasFile)
    {
        FILE *Fp=fopen(asFileName.c_str(), "rb");

        if(Fp!=NULL)
        {
            fseek(Fp, 0L, SEEK_END);
            size=ftell(Fp);
            fseek(Fp, 0L, SEEK_SET);

            if(size==2432)
                fread((char *)&temp[0], sizeof(temp), 1, Fp);
            else if(size==12800)
                fread((char *)&tempMax[0], sizeof(tempMax), 1, Fp);
            else if(size==13312)                                                //kevin 20140614
                fread((char *)&tempMax5[0], sizeof(tempMax5), 1, Fp);
            else
                fread((char *)&tempMax4[0], sizeof(tempMax4), 1, Fp);

            fclose(Fp);

            if(FileExists(asBackFile))
            {
                if(FileDataCompare(const_cast<char*>(asFileName.c_str()), const_cast<char*>(asBackFile.c_str()))==false)   // AI(W906-PT-W2) 20260807: const_cast -- FileDataCompare's cprod.h decl takes char*, same fix as cprod.cpp:1734
                {
                    Fp=fopen(asBackFile.c_str(), "rb");

                    if(Fp!=NULL)
                    {
                        fseek(Fp, 0L, SEEK_END);
                        size=ftell(Fp);
                        fseek(Fp, 0L, SEEK_SET);

                        if(size==2432)
                            fread((char *)&temp[0],sizeof(temp), 1, Fp);
                        else if(size==12800)
                            fread((char *)&tempMax[0],sizeof(tempMax), 1, Fp);
                        else if(size==13312)                                    //kevin 20140614
                            fread((char *)&tempMax5[0], sizeof(tempMax5), 1, Fp);
                        else
                            fread((char *)&tempMax4[0],sizeof(tempMax4), 1, Fp);
                        fclose(Fp);
                    }
                    else
                    {
                        bHasFile=false;
                    }
                }
            }
        }
        else
        {
            Fp=fopen(asBackFile.c_str(), "rb");

            if(Fp!=NULL)
            {
                fseek(Fp, 0L, SEEK_END);
                size=ftell(Fp);
                fseek(Fp, 0L, SEEK_SET);

                if(size==2432)
                    fread((char *)&temp[0], sizeof(temp), 1, Fp);
                else if(size==12800)
                    fread((char *)&tempMax[0], sizeof(tempMax), 1, Fp);
                else if(size==13312)                         //kevin 20140614
                    fread((char *)&tempMax5[0], sizeof(tempMax5), 1, Fp);
                else
                    fread((char *)&tempMax4[0], sizeof(tempMax4), 1, Fp);

                fclose(Fp);
            }
            else
            {
                bHasFile=false;
            }
        }
    }
    else
    {
        bHasFile=false;
    }

    if(bHasFile==true)
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {                                                                   //Steven 20140509 : Modify Function
                if(size==2432)
                {
                    SetPassCT(i, j, temp[i][j][0]);
                    SetFailCT(i, j, temp[i][j][1]);
                    ArmSKET[i][j]->GetTotal();
                    for(int k=0; k<15; k++)                                     //Steven 20140616 : iTestBinCount --> 15
                        SetBinCT(i, j, k, temp[i][j][3+k]);
                    SetIFErr(i, j, temp[i][j][18]);
                }
                else if(size==12800)
                {
                    SetPassCT(i, j, tempMax[i][j][0]);
                    SetFailCT(i, j, tempMax[i][j][1]);
                    ArmSKET[i][j]->GetTotal();
                    for(int k=0; k<iTestBinCount; k++)                          //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                        SetBinCT(i, j, k, tempMax[i][j][3+k]);
                    SetIFErr(i, j, tempMax[i][j][iTestBinCount+3]);
                }
                else if(size==13312)                                            //kevin 20140614
                {
                    SetPassCT(i, j, tempMax5[i][j][0]);
                    SetFailCT(i, j, tempMax5[i][j][1]);
                    ArmSKET[i][j]->GetTotal();
                    for(int k=0; k<iTestBinCount; k++)                          //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                        SetBinCT(i, j, k, tempMax5[i][j][3+k]);
                    SetIFErr(i, j, tempMax5[i][j][iTestBinCount+3]);
                }
                else
                {
                    SetPassCT(i, j, tempMax4[i][j][0]);
                    SetFailCT(i, j, tempMax4[i][j][1]);
                    ArmSKET[i][j]->GetTotal();
                    for(int k=0; k<iTestBinCount; k++)                          //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                        SetBinCT(i, j, k, tempMax4[i][j][3+k]);
                    SetIFErr(i, j, tempMax4[i][j][iTestBinCount+3]);
                }

                if(TestIF_File.bLowYieldAlarmByBin)
                {
                    Str.sprintf("Site%d-%d", i, j);
                    ArmSKET[i][j]->SetByBinLowYieldPassCT(CheckAndReadIniData(asIniFileName, "iByBinLowYieldPass",  Str, 0));                   //Steven 20140828 : By Bin Yield Monitor
                    ArmSKET[i][j]->SetByBinArmYieldPassCT(CheckAndReadIniData(asIniFileName, "iByBinArmYieldPass",  Str, 0));
                    ArmSKET[i][j]->SetByBinSiteYieldPassCT(CheckAndReadIniData(asIniFileName, "iByBinSiteYieldPass", Str, 0));
                }
            }
        }
    }
    else
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                SetPassCT(i, j, 0);
                SetFailCT(i, j, 0);
                ArmSKET[i][j]->GetTotal();
                for(int k=0; k<iTestBinCount; k++)                              //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                    SetBinCT(i, j, k, 0);
                SetIFErr(i, j, 0);

                Str.sprintf("Site%d-%d", i, j);
                ArmSKET[i][j]->SetByBinLowYieldPassCT(0);                       //Steven 20140828 : By Bin Yield Monitor
                ArmSKET[i][j]->SetByBinArmYieldPassCT(0);
                ArmSKET[i][j]->SetByBinSiteYieldPassCT(0);
            }
        }
    }
}
//------------------------------------------------------------------------------
TArm::~TArm()
{
    try
    {
        WriteFile();
        for(int i=0; i<MAX_SOCKET_ROW; i++)
            for(int j=0; j<MAX_SOCKET_COL; j++)
    //        for(j=0; j<NEW_MAX_Index_Col; j++)                                //Steven 20110420 : 用這個會項賽
            {
                if(ArmSKET[i][j]!=NULL)                                         //Steven 20161220 (jou) : 修正delete方式
                    delete ArmSKET[i][j];
            }

        sPass->Clear();                                                         //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        sFail->Clear();                                                         //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        sTotal->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        iIFErr->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete sPass;
        delete sFail;
        delete sTotal;
        delete iIFErr;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~TArm");
    }
}
//------------------------------------------------------------------------------
void TArm::WriteFile()                                                          //Steven 20110801 : 改成初始化後讀檔
{
    extern AnsiString W906_MachineRecordRedirect(const AnsiString&); AnsiString asFileName=W906_MachineRecordRedirect("D:\\HT9045\\system\\"+Name+".dat");   //AI(W906-MT-FIX1) 20260926: test containment (ctest Automation wrote these), golden literal when W906_MACHINERECORD_DIR is unset
    AnsiString asBackFile=W906_MachineRecordRedirect("D:\\HT9045\\system\\"+Name+"_backup.dat");   //AI(W906-MT-FIX1) 20260926

    AnsiString asIniFileName=W906_MachineRecordRedirect("D:\\HT9045\\system\\"+Name+".ini");   //AI(W906-MT-FIX1) 20260926
    AnsiString Str;
    MyForceDirectories("D:\\HT9045\\system\\");
    bHasFile=FileExists(asFileName);

    unsigned long temp[MAX_SOCKET_ROW][MAX_SOCKET_COL][TEST_MAX_BIN+4]={0};     //jou 2012-09-23 預設值為0        //Steven 20140122 : 記憶體破壞, +4
    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            temp[i][j][0]=ArmSKET[i][j]->GetPassCT();
            temp[i][j][1]=ArmSKET[i][j]->GetFailCT();
            temp[i][j][2]=ArmSKET[i][j]->GetTotal();
            for(int k=0; k<iTestBinCount; k++)
                temp[i][j][3+k]=ArmSKET[i][j]->GetSelBinCT(k);
            temp[i][j][iTestBinCount+3]=ArmSKET[i][j]->GetIFError();

            if(TestIF_File.bLowYieldAlarmByBin)
            {
                Str.sprintf("Site%d-%d", i, j);
                WriteIniData(asIniFileName, "iByBinLowYieldPass",  Str, ArmSKET[i][j]->GetByBinLowYieldPassCT());                   //Steven 20140828 : By Bin Yield Monitor
                WriteIniData(asIniFileName, "iByBinArmYieldPass",  Str, ArmSKET[i][j]->GetByBinArmYieldPassCT());
                WriteIniData(asIniFileName, "iByBinSiteYieldPass", Str, ArmSKET[i][j]->GetByBinSiteYieldPassCT());
            }
        }
    }

    FILE *Fp=fopen(asFileName.c_str(), "wb");
    if(Fp!=NULL)
    {
        fwrite((char *)&temp[0], sizeof(temp), 1, Fp);
        fclose(Fp);
    }

    Fp=fopen(asBackFile.c_str(), "wb");
    if(Fp!=NULL)
    {
        fwrite((char *)&temp[0], sizeof(temp), 1, Fp);
        fclose(Fp);
    }
}
//------------------------------------------------------------------------------
double TArm::GetPCA()
{
    Total=Pass+Fail;
    return ChangeToFloat(double(Pass), double(Total));
}
//------------------------------------------------------------------------------
double TArm::GetBySitePCA()                                                     //kevin 20130710 by sit計數
{
    BySiteTotal=BySitePass+BySiteFail;
    return ChangeToFloat(double(BySitePass), double(BySiteTotal));
}
//------------------------------------------------------------------------------
unsigned long TArm::GetPassCT()
{
    Pass=0;
    for(int i=0; i<MAX_SOCKET_ROW; i++)
        for(int j=0; j<MAX_SOCKET_COL; j++)
            Pass+=ArmSKET[i][j]->GetPassCT();
    return Pass;
}
//------------------------------------------------------------------------------
unsigned long TArm::GetFailCT()
{
    Fail=0;
    for(int i=0; i<MAX_SOCKET_ROW; i++)
        for(int j=0; j<MAX_SOCKET_COL; j++)
            Fail+=ArmSKET[i][j]->GetFailCT();
    return Fail;
}
//------------------------------------------------------------------------------
unsigned long TArm::GetTotalCT()
{
    Total=GetPassCT()+GetFailCT();                                              //Steven 20150518 : 修正Contact Count
    return Total;
}
//------------------------------------------------------------------------------
unsigned long TArm::GetSelBin(int iBin)                                         //Sam 20240131 : 取得此  Arm 的 Bin 數量
{
    int iSum=0;
    for(int i=0; i<MAX_SOCKET_ROW; i++)
        for(int j=0; j<MAX_SOCKET_COL; j++)
            iSum+=ArmSKET[i][j]->GetSelBinCT(iBin);
    return iSum;
}
//------------------------------------------------------------------------------
void TArm::ClearALLCT()
{
    int iDut;
    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            ArmSKET[i][j]->ClearALLCT();
            iDut=i*MAX_SOCKET_COL+j;
            sTotal->Strings[iDut]=AnsiString((long)ArmSKET[i][j]->GetTotal());
            sPass ->Strings[iDut]=AnsiString((long)ArmSKET[i][j]->GetPassCT());
            sFail ->Strings[iDut]=AnsiString((long)ArmSKET[i][j]->GetFailCT());
            iIFErr->Strings[iDut]=AnsiString((long)ArmSKET[i][j]->GetIFError());
        }
    }
}
//------------------------------------------------------------------------------
void TArm::ClearALLCT(int ROW, int COL)                                         //Steven 20140509 : For Secs GEM
{
    ArmSKET[ROW][COL]->ClearALLCT();
    int iDut=ROW*MAX_SOCKET_COL+COL;
    sTotal->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetTotal());
    sPass ->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetPassCT());
    sFail ->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetFailCT());
    iIFErr->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetIFError());
}
//------------------------------------------------------------------------------
void TArm::ClearBySite()                                                        //kevin 20130710
{
    int iDut;
    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            ArmSKET[i][j]->ClearALLCT();
            iDut=i*MAX_SOCKET_COL+j;
            sTotal->Strings[iDut]=AnsiString((long)ArmSKET[i][j]->GetTotal());
            sPass ->Strings[iDut]=AnsiString((long)ArmSKET[i][j]->GetPassCT());
            sFail ->Strings[iDut]=AnsiString((long)ArmSKET[i][j]->GetFailCT());
            iIFErr->Strings[iDut]=AnsiString((long)ArmSKET[i][j]->GetIFError());
        }
    }
}
//------------------------------------------------------------------------------
void TArm::SetArmBinCodeStatus(int ROW, int COL, int BinValue, int Status, bool bByBinLowYieldPass, bool bByBinArmYieldPass, bool ByBinSiteYieldPass)
{
    ArmSKET[ROW][COL]->SetbBinCodeStatus(BinValue, Status, bByBinLowYieldPass, bByBinArmYieldPass, ByBinSiteYieldPass);
}
//------------------------------------------------------------------------------
void TArm::SetArmSKTData(int ROW, int COL, int Data)
{
    ArmSKET[ROW][COL]->SetTesterBin(Data);
    int iDut=ROW*MAX_SOCKET_COL+COL;
    sTotal->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetTotal());
    sPass ->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetPassCT());
    sFail ->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetFailCT());
    iIFErr->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetIFError());
    Total=GetPassCT()+GetFailCT();                                              //Steven 20150518 : 修正Contact Count
}
//------------------------------------------------------------------------------
void TArm::InitContactCT()
{
    iContactCT=0;
}
//------------------------------------------------------------------------------
void TArm::SetContactCT(int iCT)
{
    iContactCT+=iCT;
}
//------------------------------------------------------------------------------
unsigned long TArm::GetContactCT()
{
    return iContactCT;
}
//------------------------------------------------------------------------------
void TArm::SetPassCT(int ROW, int COL, double Value)                            //Steven 20140509 : For Secs GEM
{
    ArmSKET[ROW][COL]->SetPassCT(Value);
    int iDut=ROW*MAX_SOCKET_COL+COL;
    sPass ->Strings[iDut]=AnsiString(Value);
    sTotal->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetTotal());
}
//------------------------------------------------------------------------------
void TArm::SetFailCT(int ROW, int COL, double Value)                            //Steven 20140509 : For Secs GEM
{
    ArmSKET[ROW][COL]->SetFailCT(Value);
    int iDut=ROW*MAX_SOCKET_COL+COL;
    sFail ->Strings[iDut]=AnsiString(Value);
    sTotal->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetTotal());
}
//------------------------------------------------------------------------------
void TArm::SetBinCT(int ROW, int COL, int Bin, double Value)                    //Steven 20140509 : For Secs GEM
{
    ArmSKET[ROW][COL]->SetBinCT(Bin, Value);
    int iDut=ROW*MAX_SOCKET_COL+COL;
    sTotal->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetTotal());
    sPass ->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetPassCT());
    sFail ->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetFailCT());
    iIFErr->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetIFError());
}
//------------------------------------------------------------------------------
void TArm::SetIFErr(int ROW, int COL, double Value)                             //Steven 20140509 : For Secs GEM
{
    ArmSKET[ROW][COL]->SetIFErr(Value);
    int iDut=ROW*MAX_SOCKET_COL+COL;
    iIFErr->Strings[iDut]=AnsiString(Value);
    sTotal->Strings[iDut]=AnsiString((long)ArmSKET[ROW][COL]->GetTotal());
}
//------------------------------------------------------------------------------
double TArm::GetByBinLowYieldPCA()                                              //Steven 20141125
{
    double Sum=0.0;
    double Sum2=0.0;

    for(int i=0; i<MAX_Index_Row; i++)                                          //Steven 20141125
    {
        for(int j=0; j<NEW_MAX_Index_Col; j++)
        {
            Sum+=ArmSKET[i][j]->GetByBinLowYieldPassCT();
        }
    }
    Sum2=GetTotalCT();

    return ChangeToFloat((double)Sum*100.0, (double)Sum2);
}
//------------------------------------------------------------------------------
TLotSummary::TLotSummary()
{
    ClearAllData();
}
//------------------------------------------------------------------------------
TLotSummary::~TLotSummary()
{
}
//------------------------------------------------------------------------------
void TLotSummary::ClearAllData()
{
    ZeroMemory(iCountCategory, sizeof(iCountCategory));
    ZeroMemory(iTotalCategory, sizeof(iTotalCategory));
    ZeroMemory(iLastTotalCategory, sizeof(iLastTotalCategory));
    LotSummary.iLoadTotal=0;
    iE1Count=0;                                                                 //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
    iE2Count=0;
    iE3Count=0;
}
//------------------------------------------------------------------------------
void TLotSummary::ClearRTData()
{
    for(int k=0; k<TEST_MAX_BIN; k++)
    {
        if(bIsRTBin[k]==true)
        {
            iTotalCategory[k]=0;
            for(int i=0; i<MAX_SOCKET_ROW*MAX_SOCKET_COL; i++)
            {
                iCountCategory[i][k]=0;
            }
        }
    }
}
//------------------------------------------------------------------------------
void TLotSummary::SetIsRTBin()
{
    int iBinTray;
    for(int i=0; i<TEST_MAX_BIN; i++)
    {
        iBinTray=Prod.iT6CatData[i];
        if(iBinTray>0 &&
           (iBinTray<=iAutoRight &&
            Prod.bART6Tray[iBinTray]==true) ||
           Prod.bCateRTo6Tray[iBinTray]==true)
        {
            bIsRTBin[i]=true;
        }
        else
        {
            bIsRTBin[i]=false;
        }
    }
}
//------------------------------------------------------------------------------
void TLotSummary::AddCount(int iSite, int iBin)                                 //Steven 20190726 : ATK ART Lot Count
{
    iCountCategory[iSite][iBin]++;
    iTotalCategory[iBin]++;
}
//------------------------------------------------------------------------------
void TLotSummary::AddByLotCount(int iSite, int iBin, AnsiString s2DID)
{
    AnsiString Str, str, str2="", str3="", sFileName;

    sFileName.sprintf("D:\\HT9045_Log\\2D_MappingResult\\MultiLot_VS_Result_%s.csv", sTotalLotID);

    TStringList *File, *list2D;
    File=new TStringList();
    list2D=new TStringList();

    MyForceDirectories("D:\\HT9045_Log\\2D_MappingResult\\");
    if(FileExists(sFileName)==true)
    {
        File->LoadFromFile(sFileName);
    }

    for(int i=0; i<File->Count; i++)
    {
        AnsiString s, s1, s2, s3, s4;
        int iPos1, iPos2;
        s=File->Strings[i];

        iPos1=s.Pos(" ");
        iPos2=s.Pos(",");
        if(iPos1>0 && iPos2>0 && iPos1<iPos2)                                   //表示2D有空格
        {
            s1=s.SubString(1, iPos2-1);                                         //2D
            s3=s.SubString(iPos2+1, s.Length());
            s2=StringReplace(s1, " ", "_", TReplaceFlags()<<rfReplaceAll);
            s4=s2+","+s3;
            list2D->CommaText=s4;
            list2D->Strings[0]=s1;
        }
        else
        {
            list2D->CommaText=File->Strings[i];
        }

        if(list2D->Strings[0]==s2DID)
        {
            if(list2D->Count>=4)
            {
#if 0   // GATE (1): fSCKART->iInfo_MultiLotCnt / fSCKART->sInfoArr_InnerLotID -- golden :848-855. See file banner GATE REGISTER.
                for(int j=0; j<fSCKART->iInfo_MultiLotCnt; j++)
                {
                    if(list2D->Strings[4]==fSCKART->sInfoArr_InnerLotID[j])
                    {
                        iByLotCountCategory[j][iSite][iBin]++;
                        iByLotTotalCategory[j][iBin]++;
                    }
                }
#else
                // GATE (1) faithful default: loop bound 0 (golden's own
                // TfSCKART ctor default for iInfo_MultiLotCnt before any
                // multi-lot ART config is loaded -- Automation/
                // SCK_ART_Remainder.cpp:82). No reachable global instance of
                // the struct that actually carries these two fields in this
                // tree (SckArtRemainderState) exists for this TU to bind to.
                for(int j=0; j<0; j++)
                {
                }
#endif
            }
        }
    }

    File->Clear();
    delete File;

    list2D->Clear();
    delete list2D;
}
//------------------------------------------------------------------------------
void TLotSummary::AddByLotLoadCount(AnsiString s2DID)                           //Steven 20190726 : ATK ART Lot Count
{
    AnsiString Str, str, str2="", str3="", sFileName;

    sFileName.sprintf("D:\\HT9045_Log\\2D_MappingResult\\MultiLot_VS_Result_%s.csv", sTotalLotID);

    TStringList *File, *list2D;
    File=new TStringList();
    list2D=new TStringList();

    MyForceDirectories("D:\\HT9045_Log\\2D_MappingResult\\");

    if(FileExists(sFileName)==true)
    {
        File->LoadFromFile(sFileName);
    }

    for(int i=0; i<File->Count; i++)
    {
        AnsiString s, s1, s2, s3, s4;
        int iPos1,iPos2;
        s=File->Strings[i];

        iPos1=s.Pos(" ");
        iPos2=s.Pos(",");
        if(iPos1>0 && iPos2>0 && iPos1<iPos2)                                   //表示2D有空格
        {
            s1=s.SubString(1, iPos2-1);                                         //2D
            s3=s.SubString(iPos2+1, s.Length());
            s2=StringReplace(s1, " ", "_", TReplaceFlags()<<rfReplaceAll);
            s4=s2+","+s3;
            list2D->CommaText=s4;
            list2D->Strings[0]=s1;
        }
        else
        {
            list2D->CommaText=File->Strings[i];
        }

        if(list2D->Strings[0]==s2DID)
        {
            if(list2D->Count>=4)
            {
#if 0   // GATE (1): fSCKART->iInfo_MultiLotCnt / fSCKART->sInfoArr_InnerLotID -- golden :910-917. See file banner GATE REGISTER.
                for(int j=0; j<fSCKART->iInfo_MultiLotCnt; j++)
                {
                    if(list2D->Strings[4]==fSCKART->sInfoArr_InnerLotID[j])
                    {
                        LotSummary.iByLotLoadCount[j]++;
                    }
                }
#else
                // GATE (1) faithful default: see AddByLotCount's identical gate above.
                for(int j=0; j<0; j++)
                {
                }
#endif
            }
        }
    }

    File->Clear();
    delete File;

    list2D->Clear();
    delete list2D;
}
//------------------------------------------------------------------------------
void TLotSummary::ReadFile()                                                    //Steven 20190726 : ATK ART Lot Count
{
    AnsiString FileName="D:\\HT9045\\System\\LotSummary.csv";
    TStringList *List =new TStringList();
    TStringList *List2=new TStringList();
    int iCount;

    if(FileExists(FileName))
    {
        List->LoadFromFile(FileName);
        iCount=0;
        for(int i=0; i<MAX_SOCKET_ROW*MAX_SOCKET_COL; i++)
        {
            if(List->Count>iCount)
            {
                List2->CommaText=List->Strings[iCount];
                for(int k=0; k<List2->Count; k++)
                {
                    iCountCategory[i][k]=atoi(AnsiString(List2->Strings[k]).c_str());   // AI(W906-PT-W2) 20260807: StringsProxy has no c_str() -- explicit AnsiString(...) cast, same idiom as database.cpp:1780
                }
            }
            iCount++;
        }

        if(List->Count>iCount)                                                  //JerryYang 20210209 : 修正重開程式lot summary遺失的問題
        {
            List2->CommaText=List->Strings[iCount];
            for(int k=0; k<List2->Count; k++)
            {
                iTotalCategory[k]=atoi(AnsiString(List2->Strings[k]).c_str());   // AI(W906-PT-W2) 20260807: StringsProxy has no c_str() -- explicit AnsiString(...) cast, same idiom as database.cpp:1780
            }
        }
    }
    else
    {
        ClearAllData();
    }
    delete List;
    delete List2;
}
//------------------------------------------------------------------------------
void TLotSummary::WriteFile()                                                   //Steven 20190726 : ATK ART Lot Count
{
    AnsiString FileName="D:\\HT9045\\System\\LotSummary.csv";
    TStringList *List=new TStringList();
    AnsiString Str;
    List->Clear();

    for(int i=0; i<MAX_SOCKET_ROW*MAX_SOCKET_COL; i++)
    {
        Str="";
        for(int k=0; k<TEST_MAX_BIN; k++)
        {
            if(k!=0)
                Str+=",";
            Str+=AnsiString(iCountCategory[i][k]);
        }
        List->Add(Str);
    }

    Str="";
    for(int k=0; k<TEST_MAX_BIN; k++)
    {
        if(k!=0)
            Str+=",";
        Str+=AnsiString(iTotalCategory[k]);
    }
    List->Add(Str);

    List->SaveToFile(FileName);
    delete List;
}
//------------------------------------------------------------------------------
void TEST_CATEGORY::ClearCount()
{
    ZeroMemory(iCountCategory, sizeof(iCountCategory));
    ZeroMemory(iCountHeadTotal, sizeof(iCountHeadTotal));
    ZeroMemory(iCountSocketTotal, sizeof(iCountSocketTotal));
    ZeroMemory(iCountPassHead, sizeof(iCountPassHead));
    ZeroMemory(iCountPassSocket, sizeof(iCountPassSocket));
    ZeroMemory(iTotalCategory, sizeof(iTotalCategory));

    ZeroMemory(iBySiteCate, sizeof(iBySiteCate));
    ZeroMemory(iBySiteTotal, sizeof(iBySiteTotal));
    ZeroMemory(iBySitePass, sizeof(iBySitePass));
    ZeroMemory(iBySiteFail, sizeof(iBySiteFail));

    ZeroMemory(dBySiteCate, sizeof(dBySiteCate));
    ZeroMemory(dBySitePass, sizeof(dBySitePass));
    ZeroMemory(dBySiteFail, sizeof(dBySiteFail));

    ZeroMemory(iUnloadCnt, sizeof(iUnloadCnt));

    iTotalSocket=0;
    iPassSocket =0;
    iFailSocket =0;
    iRejectCount=0;
    dPassYield  =0.0;
    dFailYield  =0.0;
}
//==============================================================================
void TEST_CATEGORY::UpdataCount(bool bCheckYield)
{
    int iDut=0, iArm=0, iSrcRow=0;   //AI(W906-W187) 20261009 (St02-E, claim): + iSrcRow, golden 913 cSocket.cpp:1059
    ClearCount();

    if(bCheckYield==true)
    {
        if(IsNNMode()==None_NN)
        {
            for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
            {
                for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                {
                    if(TestIF.iSiteMap[iRow][iCol]>0)
                    {
                        iDut=TestIF.iSiteMap[iRow][iCol]-1;
                        for(iArm=0; iArm<2; iArm++)
                        {
                            iCountCategory[iArm][iRow][iCol][iTestBinCount]  =ArmData[iArm]->ArmSKET[iRow][iCol]->GetIFError();
                            iTotalCategory[iTestBinCount]                   +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetIFError();
                            for(int iCat=0; iCat<iTestBinCount; iCat++)             //每個category
                            {
                                iCountCategory[iArm][iRow][iCol][iCat]  =ArmData[iArm]->ArmSKET[iRow][iCol]->GetSelBinCT(iCat);
                                iTotalCategory[iCat]                   +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetSelBinCT(iCat);
                                iBySiteCate[iDut][iCat]                +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetSelBinCT(iCat);
                            }
                            iCountSocketTotal[iRow][iCol]      +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                            iCountHeadTotal[iArm][iRow][iCol]  +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                            iTotalSocket                       +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                            iBySiteTotal[iDut]                 +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                            iCountPassSocket[iRow][iCol]       +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetPassCT();
                            iCountPassHead[iArm][iRow][iCol]   +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetPassCT();
                            iPassSocket                        +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetPassCT();
                            iBySitePass[iDut]                  +=ArmData[iArm]->ArmSKET[iRow][iCol]->GetPassCT();
                        }
                    }
                }
            }
        }
        else if(IsNNMode()==NN_1Row)
        {
            for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
            {
                for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                {
                    if(TestIF.iSiteMap[iRow][iCol]>0)
                    {
                        iArm=(iRow==0)?1:0;  iSrcRow=(iArm==0)?0:iRow;   //AI(W906-W187) 20261009 (St02-E, claim): golden 913 cSocket.cpp:1105 (Ifor 20260807: read ArmSKET by the arm-local row, as ProcessArmCount writes it -- atester_ProcessCount.cpp iRow32)
                        iDut=TestIF.iSiteMap[iRow][iCol]-1;
                        iCountCategory[iArm][iRow][iCol][iTestBinCount]  =ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetIFError();
                        iTotalCategory[iTestBinCount]                   +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetIFError();
                        for(int iCat=0; iCat<iTestBinCount; iCat++)
                        {
                            iCountCategory[iArm][iRow][iCol][iCat]  =ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                            iTotalCategory[iCat]                   +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                            iBySiteCate[iDut][iCat]                +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                        }
                        iCountSocketTotal[iRow][iCol]      +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iCountHeadTotal[iArm][iRow][iCol]  +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iTotalSocket                       +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iBySiteTotal[iDut]                 +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iCountPassSocket[iRow][iCol]       +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iCountPassHead[iArm][iRow][iCol]   +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iPassSocket                        +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iBySitePass[iDut]                  +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                    }
                }
            }
        }
        else
        {
            for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
            {
                for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                {
                    if(TestIF.iSiteMap[iRow][iCol]>0)
                    {
                        iArm=(iRow==0 || iRow==1)?1:0;  iSrcRow=(iArm==0)?(iRow-2):iRow;   //AI(W906-W187) 20261009 (St02-E, claim): golden 913 cSocket.cpp:1136 (Ifor 20260807: read ArmSKET by the arm-local row, as ProcessArmCount writes it -- atester_ProcessCount.cpp iRow32)
                        iDut=TestIF.iSiteMap[iRow][iCol]-1;
                        iCountCategory[iArm][iRow][iCol][iTestBinCount]  =ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetIFError();
                        iTotalCategory[iTestBinCount]                   +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetIFError();
                        for(int iCat=0; iCat<iTestBinCount; iCat++)
                        {
                            iCountCategory[iArm][iRow][iCol][iCat]  =ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                            iTotalCategory[iCat]                   +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                            iBySiteCate[iDut][iCat]                +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                        }
                        iCountSocketTotal[iRow][iCol]      +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iCountHeadTotal[iArm][iRow][iCol]  +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iTotalSocket                       +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iBySiteTotal[iDut]                 +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iCountPassSocket[iRow][iCol]       +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iCountPassHead[iArm][iRow][iCol]   +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iPassSocket                        +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iBySitePass[iDut]                  +=ArmData[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                    }
                }
            }
        }
    }
    else                                                                                //Steven 20250603 : by lot summary
    {
        if(IsNNMode()==None_NN)
        {
            for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
            {
                for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                {
                    if(TestIF.iSiteMap[iRow][iCol]>0)
                    {
                        iDut=TestIF.iSiteMap[iRow][iCol]-1;
                        for(iArm=0; iArm<2; iArm++)
                        {
                            iCountCategory[iArm][iRow][iCol][iTestBinCount]  =ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetIFError();
                            iTotalCategory[iTestBinCount]                   +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetIFError();
                            for(int iCat=0; iCat<iTestBinCount; iCat++)             //每個category
                            {
                                iCountCategory[iArm][iRow][iCol][iCat]  =ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetSelBinCT(iCat);
                                iTotalCategory[iCat]                   +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetSelBinCT(iCat);
                                iBySiteCate[iDut][iCat]                +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetSelBinCT(iCat);
                            }
                            iCountSocketTotal[iRow][iCol]      +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                            iCountHeadTotal[iArm][iRow][iCol]  +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                            iTotalSocket                       +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                            iBySiteTotal[iDut]                 +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetTotal();
                            iCountPassSocket[iRow][iCol]       +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetPassCT();
                            iCountPassHead[iArm][iRow][iCol]   +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetPassCT();
                            iPassSocket                        +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetPassCT();
                            iBySitePass[iDut]                  +=ArmDataLot[iArm]->ArmSKET[iRow][iCol]->GetPassCT();
                        }
                    }
                }
            }
        }
        else if(IsNNMode()==NN_1Row)
        {
            for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
            {
                for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                {
                    if(TestIF.iSiteMap[iRow][iCol]>0)
                    {
                        iArm=(iRow==0)?1:0;  iSrcRow=(iArm==0)?0:iRow;   //AI(W906-W187) 20261009 (St02-E, claim): golden 913 cSocket.cpp:1202 (Ifor 20260807: read ArmSKET by the arm-local row, as ProcessArmCount writes it -- atester_ProcessCount.cpp iRow32)
                        iDut=TestIF.iSiteMap[iRow][iCol]-1;
                        iCountCategory[iArm][iRow][iCol][iTestBinCount]  =ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetIFError();
                        iTotalCategory[iTestBinCount]                   +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetIFError();
                        for(int iCat=0; iCat<iTestBinCount; iCat++)
                        {
                            iCountCategory[iArm][iRow][iCol][iCat]  =ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                            iTotalCategory[iCat]                   +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                            iBySiteCate[iDut][iCat]                +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                        }
                        iCountSocketTotal[iRow][iCol]      +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iCountHeadTotal[iArm][iRow][iCol]  +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iTotalSocket                       +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iBySiteTotal[iDut]                 +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iCountPassSocket[iRow][iCol]       +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iCountPassHead[iArm][iRow][iCol]   +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iPassSocket                        +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iBySitePass[iDut]                  +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                    }
                }
            }
        }
        else
        {
            for(int iRow=0; iRow<TestSocket.iShtRow; iRow++)
            {
                for(int iCol=0; iCol<TestSocket.iShtCol; iCol++)
                {
                    if(TestIF.iSiteMap[iRow][iCol]>0)
                    {
                        iArm=(iRow==0 || iRow==1)?1:0;  iSrcRow=(iArm==0)?(iRow-2):iRow;   //AI(W906-W187) 20261009 (St02-E, claim): golden 913 cSocket.cpp:1233 (Ifor 20260807: read ArmSKET by the arm-local row, as ProcessArmCount writes it -- atester_ProcessCount.cpp iRow32)
                        iDut=TestIF.iSiteMap[iRow][iCol]-1;
                        iCountCategory[iArm][iRow][iCol][iTestBinCount]  =ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetIFError();
                        iTotalCategory[iTestBinCount]                   +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetIFError();
                        for(int iCat=0; iCat<iTestBinCount; iCat++)
                        {
                            iCountCategory[iArm][iRow][iCol][iCat]  =ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                            iTotalCategory[iCat]                   +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                            iBySiteCate[iDut][iCat]                +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetSelBinCT(iCat);
                        }
                        iCountSocketTotal[iRow][iCol]      +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iCountHeadTotal[iArm][iRow][iCol]  +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iTotalSocket                       +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iBySiteTotal[iDut]                 +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetTotal();
                        iCountPassSocket[iRow][iCol]       +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iCountPassHead[iArm][iRow][iCol]   +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iPassSocket                        +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                        iBySitePass[iDut]                  +=ArmDataLot[iArm]->ArmSKET[iSrcRow][iCol]->GetPassCT();
                    }
                }
            }
        }
    }

    iFailSocket=iTotalSocket-iPassSocket;
    for(iDut=0; iDut<MAX_SOCKET_ROW*MAX_SOCKET_COL; iDut++)
    {
        iBySiteFail[iDut]=iBySiteTotal[iDut]-iBySitePass[iDut];
    }

    for(int i=0; i<eTrayCount; i++)
    {
        if(Prod.iTrayType[i]!=tNotUse)
        {
            for(int j=0; j<=iTestBinCount; j++)
            {
                int temp=Prod.iT6PosCate[j];
                if(temp<=0 && j!=iTestBinCount)
                    continue;
                if(i==temp-1)
                {
                    iUnloadCnt[i]+=iTotalCategory[j];
                }

                if(Prod.iIfErrorT6==i && j==iTestBinCount)
                {
                    iUnloadCnt[i]+=iTotalCategory[j];
                }
            }
        }
    }

    UpdataYield();
}
//==============================================================================
void TEST_CATEGORY::UpdataYield()
{
    for(int iDut=0; iDut<MAX_SOCKET_ROW*MAX_SOCKET_COL; iDut++)
    {
        dBySitePass[iDut]=ChangeToFloat((double)iBySitePass[iDut], (double)iBySiteTotal[iDut]);
        dBySiteFail[iDut]=ChangeToFloat((double)iBySiteFail[iDut], (double)iBySiteTotal[iDut]);
        for(int iCat=0; iCat<iTestBinCount; iCat++)
        {
            dBySiteCate[iDut][iCat]=ChangeToFloat((double)iBySiteCate[iDut][iCat], (double)iBySiteTotal[iDut]);
        }
    }

    dPassYield=ChangeToFloat((double)iPassSocket, (double)iTotalSocket);
    dFailYield=ChangeToFloat((double)iFailSocket, (double)iTotalSocket);
}

//------------------------------------------------------------------------------
// AI(W906-T4-U82) 20260924: wb_serve 開機呼叫 —— golden TfMain::FormShow main.cpp:11154 `LotSummary.ReadFile();`
//   （ATK ART Lot Count，D:\HT9045\System\LotSummary.csv → iCountCategory／iTotalCategory；檔案不存在時 ClearAllData() 只清記憶體）。
//   純讀取：TLotSummary::WriteFile（上方）在移植樹沒有任何活的呼叫者，所以補讀不會引出寫檔。
//   消費端是 SCK/ATK ART（TestIF_File.bSCKART_EnableART），在 U31 補上之前恆 false ⇒ 目前忠實但沒有行為效果。
//   包成函式是為了讓 tools/wb_serve.cpp 用區塊內 extern 呼叫，不必在它的檔頭加 include（會位移全檔行號）。
void W906_BootReadLotSummary()
{
    LotSummary.ReadFile();                                                      // golden main.cpp:11154
}
