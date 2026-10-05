// =============================================================================
//  bthermo.cpp  --  Handler thermal control: SV dispatch, PV read-back and
//                   temperature-offset conversion for every heat channel.
//
//  Faithful translation of golden bthermo.cpp (4820 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-GA-BTHERMO) 20260807
//  Translation wave: W906-GA-BTHERMO (pure-translation completion campaign; the
//  thermal sibling of the already-landed cpublic.cpp comm helpers, and the unit
//  uHeaterThread.cpp's GATE 2 / GATE 10 name as their blocker).
//
//  ROLE: owns the whole handler-side thermal loop --
//    * the offset algebra  ConvertTempOffset / GetFactSetTemp /
//      ConvertGetTempOffset / GetConvertTemp  (1/2/3/5-point base interpolation,
//      Kit offsets, Boost + Chamber-Boost, TSMC-Tainan arm and no-full-site
//      offsets);
//    * the polling state machine  DoThermoReal()  --  `static int &Task=
//      iThermoTask`, cases 1 / 100 / 200 / 230 / 250 / 255 / 260 / 270 / 300 /
//      500 / 510 / 520 / 530 / 2500, one heat channel per pass, the Addr cursor
//      wrapping at tcTotalCount;
//    * the per-controller SV writers  DoSetSVOfOmronEJ1N / DoSetSVOfDTME08 /
//      DoATC60Temperature  and the ATC PV reader  DOUN150ReadTemp;
//    * the heater-enable predicates  bGetHeaterUsed / bGet16HeaterUsedTo4Heater;
//    * the PV smoothing ring  fBuffer[tcTotalCount][30] / ReadAverageBuffer /
//      ClearHotBuffer / ClearAllHotBuffer;
//    * CheckLBTemp() -- the LB cooling-air interlock.
//
//  WAVE SCOPE -- ACTIVE vs gated:
//   ACTIVE (faithful, verbatim): CheckIsDUT, ConvertTempOffset, GetFactSetTemp,
//     ConvertGetTempOffset, GetConvertTemp, bGetHeaterUsed, A_Check_Addr,
//     A_Check_LRC, ClearSendBuff, A_Get_Function_Code, A_Get_MEM_Address,
//     A_Get_MEM_Word, ClearHotBuffer, ClearAllHotBuffer, ReadAverageBuffer, DoThermo,
//     DoThermoReal (the whole SM shape and every case body), DoTemptureControl,
//     bGet16HeaterUsedTo4Heater (all 8 test-mode branches), CheckLBTemp, and the
//     Temp[]-computation halves of DoATC60Temperature / DoSetSVOfOmronEJ1N /
//     DoSetSVOfDTME08 / DOUN150ReadTemp.
//   GATE (#if 0): 20 live gates (numbered G01-G03, G12-G23, G25-G29; the G04-G11
//     and G24 numbers were withdrawn mid-wave, see below).  Every live gate is
//     re-cited at its own call site.
//
//  LOCALLY PROVIDED (NOT a gate, but disclosed -- read before editing):
//   golden uTemp_Set.h:29-42 declares 17 `extern const int` offset-table row
//   indices (LowBase..SHigBase) whose definitions live in golden
//   uTemp_Set.cpp:48-64.  uTemp_Set is UNPORTED and no port header or source
//   defines ANY of them (grepped every .h and .cpp in the tree).  They index
//   Temperature.fTempOffSet[19][tcTotalCount] (cprod.h:1388); without them
//   ConvertTempOffset / ConvertGetTempOffset / GetFactSetTemp -- roughly 700
//   lines and the analytical core of this unit -- would ALL have had to be
//   gated.  They are therefore mirrored file-locally below with golden's EXACT
//   values and INTERNAL linkage, so the algebra stays ACTIVE and no symbol is
//   exported that a future uTemp_Set.cpp could collide with.
//   RETIREMENT: when uTemp_Set lands, delete the mirror block and include its
//   header instead; the `static` guarantees a loud, correctly-located compile
//   error if anyone includes uTemp_Set.h here while the mirror is still present.
//
//  GATE REGISTER (each is re-cited verbatim at its own call site):
//   G01  golden :269-270, :629-630  fContact->CarlibrationTask -- TfContact
//        unported; atester_shims.h TfContactShim has only fShow.  ACTIVE form
//        keeps `IndexStatus==Z1Up_Z2Down` alone (fShow is false offline, so the
//        dropped disjunct was already unreachable).
//   G02  golden :343-346  fProductionInfo->dTempTajOfs -- not a member of the
//        translated forms/fProductionInfo.h stand-in.
//   G03a/b golden :283-288, :643-646  fLotInfo->bStartChamberBoost -- not a
//        member of forms/fLotInfo.h, and it sits in the IF condition.
//   G04..G11  WITHDRAWN BEFORE DELIVERY -- see the note below the register.  The
//        numbering keeps its holes on purpose so this banner and the wave report
//        line up.
//   G12  DoCloseHeadterDelay.Off() (:1257, :3210, :4728) -- csystem.h:226
//        declares it, NOTHING defines it (golden csystem.cpp:1133 untranslated).
//        Default true is behaviour-exact: golden's only setter
//        (csystem.cpp:1408) is also untranslated, so a default-constructed
//        TQPF_Timer answers Off()==true (myTimer.cpp:40-44).
//   G13  bHeaterDoorIsOpen[] (:3211, :3361, :4728) -- csystem.h:197 declares it,
//        NOTHING defines it.  Default false == a zero-initialised global.
//   G14a/b  fConfiguration (:1004-1005, :1408) -- TfConfiguration unported; the
//        only stand-in (Automation/SCK_ART_Remainder.h) carries mmoN04_IP alone.
//   G15/G16/G17/G18  COM2 (:1045-1048, :2375-2376, eight Comm2->StopComm sites
//        plus one StartComm, :2969) -- atester_shims.h TCOM2Shim has no Comm2,
//        no ATCInitital, no ATCFuntion_Read, no ATCInitialTask.  Each is a
//        ONE-STATEMENT gate; all retry / CommunCTErr / UN150CommError
//        bookkeeping around them stays ACTIVE.
//   G19a/b/c  FormHS->CheckTempOffset (:2318-2332, :3387-3401, :4750-4764) --
//        HS_Function unported; the only FormHS stand-in exposes
//        UpDataToServerByFTP alone.  Default: no over-limit clamp.
//   G20  HeaterSVLog (:2334) -- DECLARED in cpublic.h, DEFINITION at
//        cpublic.cpp:707 sits inside that file's own `#if 0`.
//   G21a/b, G22 (7 sites), G23a/b  TMC401WriteTemp/ReadTemp,
//        UT100WordWrite/ReadNoSucm, E5DCWriteTemp/ReadTemp -- all DECLARED in
//        cpublic.h with their DEFINITIONS inside cpublic.cpp's `#if 0`
//        (:582, :603, :326, :339, :640, :623).  Undefined references, verified
//        by walking that file's preprocessor nesting rather than by eye.
//   G24  WITHDRAWN BEFORE DELIVERY -- see the note below the register.
//   G25  golden :3243-3319  ATCInterfaceForm->ATC_60_SYS.* -- ATC/ATCInterface.h
//        and ATC/ATCSystem.h unported (zero tree-wide hits).
//   G26a/b golden :3375-3378, :3404-3783  fOmron / myPal / dSetValue /
//        dTempValue / btSetSV -- OmronEJ1N.h unported.
//   G27  golden :3965  fTemp_Set->iAddrToATC[] -- uTemp_Set.h unported.
//   G28a/b/c golden :3981-4080, :4086-4103, :4106-4188  ATC_InterfaceForm->
//        IsConnect()/dTC[]/dTC2[] (acarry_shims.h TATC_InterfaceFormShim exposes
//        iATC_MODE_TYPE alone), ATCInterfaceForm->ATC_SYS_PAL /
//        GetATCSiteNowTemperature(_Ref), fWinway->arrATC_Site (WinWaySetting.h).
//   G29  golden :4775-4776  frmDTME08->SetSettingSV / GetPalGroup()->GetPV() --
//        EJ1N/fDTME08.h unported.   [OPENED AI(W906-I03) 20261002 (Ifor01), I-03 phase 2]
//
//  GATES WITHDRAWN BEFORE DELIVERY -- AI(W906-GA-BTHERMO) 20260807:
//   G04-G11 (the five A_* ASCII-Modbus helpers and their three call-site macro
//   defaults) and G24 (CountHexToByte) were written, then withdrawn during this
//   same wave: a sibling agent landed EJ1N/TextProcess.{h,cpp} -- a full port of
//   the module, with T_ASXII2HEX_Mac (TextProcess.cpp:148), A_Create_LCR (:722)
//   and CountHexToByte (:429) all ACTIVE, not gated.  The gates would have been
//   over-gating, so all nine sites are ACTIVE here and this file now has a HARD
//   LINK DEPENDENCY on EJ1N/TextProcess.cpp -- it must be in the same link line.
//   If for any reason that unit is NOT integrated, this file will fail to link on
//   T_ASXII2HEX_Mac / A_Create_LCR / CountHexToByte and those nine sites are the
//   ones to re-gate.  Observable delta of the withdrawal: NONE against golden --
//   it restores golden behaviour that the gates would have approximated.
//   One residual substitution remains from that group and is NOT a gate: golden
//   :880's BCB6 SysUtils `StrLen(READBUFF)` becomes `::strlen((const char*)
//   READBUFF)` -- Borland StrLen is ISO strlen, so this is a 1:1 rename plus the
//   same unsigned-char cast the four ::strncpy sites need.
//
//  GOLDEN BUGS / QUIRKS preserved VERBATIM (never corrected):
//   * golden :3457  HeaterSVLog(tcAb1, Temp[tcBa1]) -- logs the tcBa1 setpoint
//     under the tcAb1 channel (copy-paste slip; all 31 sibling lines pass their
//     own channel).  Inside gate G26b, kept byte-identical.
//   * golden :3326 / :3376  DoSetSVOfOmronEJ1N's `static double OldTemp[]` is
//     READ but never WRITTEN, so the "restore the last SV while the form is
//     shown" branch always restores 0.0.
//   * golden :1067, :1077, :3331  asErrorAddr (twice) and S2 are declared and
//     never used anywhere -- dead locals, kept.
//   * golden :3063-3065 and :3127-3129  case 500 falls through into case 510 and
//     case 520 into case 530 with no `break` -- intentional, kept.
//   * golden :77-79 and :333-334  `int iLowBase =Temperature.fLowBase;` etc.
//     truncate a double base temperature to int and the truncated value is what
//     the slope `m` is computed from.  Kept EXACT -- no int/float rewrite.
//   * golden :2346 / :2380  `Addr/4` and `Addr%4` integer truncation.  Kept.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped.  AnsiString (1-based) + AnsiString::sprintf via vclcompat, verbatim,
//  including golden's raw-AnsiString %s argument at :2738.  NO __fastcall /
//  __property / __published in the .cpp -- golden's lone `void __fastcall
//  ClearSendBuff(void)` (:897) drops the keyword (it is declared in no header and
//  referenced by no other golden TU, so no mangling can disagree).
//  SOFT_SIMULTE is NOT defined: #ifndef paths run, #ifdef paths are reproduced
//  verbatim but inert.  Numeric semantics kept EXACT.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================

// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "bthermo.h"
//   #include "rs232.h" / "myswitch.h" / "mymessbox.h" / "csystem.h" /
//   "uTemp_Set.h" / "cConfiguration.h" / "database.h" / "cpublic.h" / "cprod.h" /
//   "cMyDef.h" / "common.h" / "OmronEJ1N.h" / "ATCInterface.h" /
//   "ATC_Handler_Side.h" / "atester.h" / "cContact.h" / "HS_Function.h" /
//   "uLotInfo.h" / "mysensor.h" / "ProductionInfo.h" / "WinWaySetting.h" /
//   "fDTME08.h" / "cinitial.h" / "TextProcess.h"
//   ; #pragma package(smart_init)
// =============================================================================

#include "MachineDefine.h"     // de-VCL'd include hub: vclcompat umbrella + portable STL
#include "forms/fTemp_Set.h"   // AI(W906-S09-B1b) 20260930 (St02-E, claim): fTemp_Set->iAddrToATC (forms/fTemp_Set.h:544) -- G27; its extern constants replace the mirror below; occupies the old blank line
#include "bthermo.h"
#include "forms/fLotInfo.h"    // AI(W906-S09-B1) 20260930 (St02-E, claim Q-INC): fLotInfo->bStartChamberBoost (forms/fLotInfo.h:1811) -- G03a / G03b below; occupies the old blank line
#include "MachineType.h"       // tc* channel enum, eht*/eATC*/eDut*ea, CC_*, NN_2Row
#include "cmydef.h"            // UN150Read[]/UN150ReadReal[]/READBUFF/Com2Buffer/iTempCode[]/
                               //   TC401HeaterControl/ATC_SYSTEM/USE_16_HEATER/CUSTOMER_CODE/...
#include "cprod.h"             // Temperature / TestIF / TestIF_File / HotPlateForm
#include "cpublic.h"           // DTK4848WordWriteNoSucm / DTK4848WordReadNoSucm (both ACTIVE) plus
                               //   the TMC401/UT100/E5DC/HeaterSVLog decls gated there (G20-G23)
#include "Config.h"            // IniConfig      (golden reached it via cinitial.h / cMyDef.h)
#include "CosFunction.h"       // CosFunction
#include "LastSet.h"           // LastSet.iTemperature
#include "csystem.h"           // externs for DoCloseHeadterDelay / bHeaterDoorIsOpen (G12 / G13)
#include "common.h"            // golden include kept
#include "myswitch.h"          // SW[] / SwHeaterRelay / SwLBAir
#include "mysensor.h"          // Sen[] / SnHeaterDoor / SnHeaterDoor2
#include "myTimer.h"           // TQPF_Timer  (golden reached it via MachineDefine.h -> HTimer.h)
#include "canary_support.h"    // ShowMyMessage (was mymessbox.h) / RecordProcess (was cMyDB.h)
#include "EJ1N/TextProcess.h"  // T_ASXII2HEX_Mac / A_Create_LCR / CountHexToByte -- golden reached
                               //   these through TextProcess.h; the module landed mid-wave, which is
                               //   why gates G04-G11/G24 were withdrawn.  HARD LINK DEPENDENCY:
                               //   EJ1N/TextProcess.cpp must be in the same link as this file.
// NOT included, and why:
//   rs232.h / OmronEJ1N.h / ATCInterface.h / ATC_Handler_Side.h / WinWaySetting.h /
#include "forms/fDTME08.h"         //AI(W906-I03) 20261002 (Ifor01): G29 opened, frmDTME08 is ported (I-03 phase 2); the NOT-included list goes on: uTemp_Set.h / cConfiguration.h / HS_Function.h / ProductionInfo.h /
//   uLotInfo.h  -- unported; every symbol they supplied is gated (G02, G03, G14,
//   G15-G18, G19, G25, G26, G27, G28, G29) or mirrored below.
//   atester.h / cContact.h / cinitial.h / database.h -- golden included them, but
//   after gating this TU reads nothing from them (HSys appears only inside
//   DoTemptureControl's wholly commented-out body), so they are left out rather
//   than dragged in for show.
// =============================================================================

#define _READ_MAX       21
#define ReadSIndex      0
#define ERROR_CODE      1
#define OK_CODE         0
#define ERROR_FUNC      1
#define ERROR_ADDRESS   2
#define ERROR_LENGTH    3
#define ERROR_CHECKSUM  4
#define ERROR_DATA      5
//#define ERROR_BUSY        6
#define ERROR_NOTASCII  7
//---------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//  Temperature.fTempOffSet[] ROW INDICES -- LOCAL MIRROR of golden
//  uTemp_Set.cpp:48-64 (declared golden uTemp_Set.h:29-42 as `extern const int`).
//
//AI(W906-GA-BTHERMO) 20260807: uTemp_Set is unported and NOTHING in this tree
//  defines these 17 constants (grepped every .h and .cpp).  They are pure array
//  row indices into Temperature.fTempOffSet[19][tcTotalCount] (cprod.h:1388), so
//  mirroring them with golden's exact values is behaviour-identical.  Declared
//  `static` (INTERNAL linkage) on purpose: golden's are external and a future
//  uTemp_Set.cpp will define the external ones, so internal linkage here means
//  the two can never collide at link time, and including uTemp_Set.h in this TU
//  later fails loudly at compile time ("static declaration follows non-static")
//  exactly where the mirror has to be deleted.  Values are golden-verbatim; the
//  three constants this unit never names (SingleLimit, TargetTemp,
//  TempEditTotal) are mirrored too so the block reads as one table.
// -----------------------------------------------------------------------------
//static const int LowBase                =0;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int LowBase is forms/fTemp_Set.h:402, defined =0 in uTemp_Set.cpp (same value)
//static const int MidBase                =1;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int MidBase is forms/fTemp_Set.h:403, defined =1 in uTemp_Set.cpp (same value)
//static const int HigBase                =2;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int HigBase is forms/fTemp_Set.h:404, defined =2 in uTemp_Set.cpp (same value)
//static const int UserOffSet             =3;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int UserOffSet is forms/fTemp_Set.h:405, defined =3 in uTemp_Set.cpp (same value)
//static const int SingleLimit            =4;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int SingleLimit is forms/fTemp_Set.h:406, defined =4 in uTemp_Set.cpp (same value)
//static const int TargetTemp             =5;                                     //Steven 20140924 : 各個加熱區獨立有自己的設定值   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int TargetTemp is forms/fTemp_Set.h:407, defined =5 in uTemp_Set.cpp (same value)
//static const int AmbientHotLow          =6;                                     //kevin 20140918 恆溫控制 低  offset   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int AmbientHotLow is forms/fTemp_Set.h:408, defined =6 in uTemp_Set.cpp (same value)
//static const int AmbientHotMid          =7;                                     //kevin 20140918 恆溫控制 中 offset   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int AmbientHotMid is forms/fTemp_Set.h:409, defined =7 in uTemp_Set.cpp (same value)
//static const int InitTempOffset         =8;                                     //Steven 20141117 : 起測時溫度要補Offset   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int InitTempOffset is forms/fTemp_Set.h:410, defined =8 in uTemp_Set.cpp (same value)
//static const int TestOverTimeTempOffset =9;                                     //20160312 : 距離上次測試時間超過所設定時間要補Offset   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int TestOverTimeTempOffset is forms/fTemp_Set.h:411, defined =9 in uTemp_Set.cpp (same value)
static const int TempEditTotal          =10;                                    //kevin 20140918 恆溫控制 5->7
//static const int KitLowBase             =11;                                    //JerryYang 20221003 Analog要求新增Kit三點offset   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int KitLowBase is forms/fTemp_Set.h:412, defined =11 in uTemp_Set.cpp (same value)
//static const int KitMidBase             =12;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int KitMidBase is forms/fTemp_Set.h:413, defined =12 in uTemp_Set.cpp (same value)
//static const int KitHigBase             =13;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int KitHigBase is forms/fTemp_Set.h:414, defined =13 in uTemp_Set.cpp (same value)
//static const int KitAmbientHotLow       =14;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int KitAmbientHotLow is forms/fTemp_Set.h:415, defined =14 in uTemp_Set.cpp (same value)
//static const int KitAmbientHotMid       =15;   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int KitAmbientHotMid is forms/fTemp_Set.h:416, defined =15 in uTemp_Set.cpp (same value)
//static const int SHigBase               =16;                                    //Ztex 2024.07.27 Add 6 Point Temperature Offset   //AI(W906-S09-B1b) 20260930 (St02-E, claim): mirror retired -- extern const int SHigBase is forms/fTemp_Set.h:417, defined =16 in uTemp_Set.cpp (same value)

// -----------------------------------------------------------------------------
//  GATE MACROS -- the named-default form used by aTester_Front.cpp:35-73.
//  Each stands in for ONE expression that cannot compile or cannot link offline;
//  the chosen value is justified again at every call site.
// -----------------------------------------------------------------------------
#define BTHERMO_G12_DOCLOSEHEADTERDELAY_OFF  (true)                             // csystem.h:226 extern never defined
#define BTHERMO_G13_HEATERDOOR_IS_OPEN(i)    (false)                            // csystem.h:197 extern never defined
#define BTHERMO_G14_FCONFIGURATION_FSHOW     (false)                            // TfConfiguration unported
// -----------------------------------------------------------------------------

//bool bCommSucceed=false;
TQPF_Timer Com2Delay;
TQPF_Timer ComOmronDelay;                                                       //Steven 20120220 : Omron EJ1N溫控器
extern TQPF_Timer DoCloseHeadterDelay;
//---------------------------------------------------------------------------
bool CheckIsDUT(int Addr)
{
    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        return false;
    }
    else if(iSocketBaseTempCount==eDut4ea)
    {
        if((Addr>=tcDUT1) && (Addr<=tcDUT4))
            return true;
    }
    else if(iSocketBaseTempCount==eDut2ea)                                      //Steven 20140116 : Socket數量,改用數字設定
    {
        if((Addr>=tcDUT1) && (Addr<=tcDUT2))
            return true;
    }
    else
    {
        if(Addr==tcSocket)
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
double ConvertTempOffset(int Addr, double T)                                    //Steven 20141117 : 起測時溫度要補Offset
{
    int iLowBase =Temperature.fLowBase;
    //int iMidBase =Temperature.fMiddBase;
    int iHighBase=Temperature.fHighBase;
    double m,s,Temp[6]={0.0};                                                   //kevin 20141006 Temp[3]->Temp[5]
    int ct1,ct2;
    char str[256];
    if(Temperature.iTempMode==8)                                                // 5 point base  kevin 20141006 最低溫度
    {
        iLowBase=Temperature.fAmbientHotLowBase;
        if(T>=Temperature.fMiddBase)//溫度
        {
            iLowBase=Temperature.fMiddBase;
            ct1=1;  //恆溫  中溫 :50          25 50 75 100  125
            ct2=2;  //低溫       :75
        }
        else if(T>=Temperature.fLowBase)//低溫度
        {
            iLowBase=Temperature.fLowBase;
            iHighBase=Temperature.fMiddBase;
            ct1=0;  //恆溫  中溫 :50          25 50 75 100  125
            ct2=1;  //低溫       :75
        }
        else if(T>=Temperature.fAmbientHotMiddBase)//恆溫最低溫度
        {
            iLowBase=Temperature.fAmbientHotMiddBase;
            iHighBase=Temperature.fLowBase;
            ct1=4;  //恆溫  中溫 :50          25 50 75 100  125
            ct2=0;  //低溫       :75
        }
        else
        {
            iHighBase=Temperature.fAmbientHotMiddBase;
            ct1=3; //恆溫  低溫 :25          25 50 75 100  125
            ct2=4; //恆溫  中溫 :50
        }

        if(CosFunction.bTemp5PointKitOffset==true)                              //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
        {
            if(T==Temperature.fAmbientHotLowBase)
                return Temperature.fTempOffSet[AmbientHotLow][Addr]+Temperature.fTempOffSet[KitAmbientHotLow][Addr]+Temperature.fAmbientHotLowBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fAmbientHotMiddBase)
                return Temperature.fTempOffSet[AmbientHotMid][Addr]+Temperature.fTempOffSet[KitAmbientHotMid][Addr]+Temperature.fAmbientHotMiddBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fLowBase)
                return Temperature.fTempOffSet[LowBase][Addr]+Temperature.fTempOffSet[KitLowBase][Addr]+Temperature.fLowBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fMiddBase)
                return Temperature.fTempOffSet[MidBase][Addr]+Temperature.fTempOffSet[KitMidBase][Addr]+Temperature.fMiddBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fHighBase)
                return Temperature.fTempOffSet[HigBase][Addr]+Temperature.fTempOffSet[KitHigBase][Addr]+Temperature.fHighBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fSHighBase)                                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
                return Temperature.fTempOffSet[SHigBase][Addr]+Temperature.fTempOffSet[KitHigBase][Addr]+Temperature.fSHighBase+Temperature.fTempOffSet[UserOffSet][Addr];
        }
        else
        {
            if(T==Temperature.fAmbientHotLowBase)
                return Temperature.fTempOffSet[AmbientHotLow][Addr]+Temperature.fAmbientHotLowBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fAmbientHotMiddBase)
                return Temperature.fTempOffSet[AmbientHotMid][Addr]+Temperature.fAmbientHotMiddBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fLowBase)
                return Temperature.fTempOffSet[LowBase][Addr]+Temperature.fLowBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fMiddBase)
                return Temperature.fTempOffSet[MidBase][Addr]+Temperature.fMiddBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fHighBase)
                return Temperature.fTempOffSet[HigBase][Addr]+Temperature.fHighBase+Temperature.fTempOffSet[UserOffSet][Addr];
        }
    }   //kevin 20141006 end
    else if(Temperature.iTempMode==4)                                           // 3 point base
    {
        if(T>=Temperature.fMiddBase)
        {
            iLowBase=Temperature.fMiddBase;
            ct1=1;
            ct2=2;
        }
        else
        {
            iHighBase=Temperature.fMiddBase;
            ct1=0;
            ct2=1;
        }

        if(CosFunction.bTemp5PointKitOffset==true)                              //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
        {
            if(T==Temperature.fLowBase)
                return Temperature.fTempOffSet[LowBase][Addr]+Temperature.fTempOffSet[KitLowBase][Addr]+Temperature.fLowBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fMiddBase)
                return Temperature.fTempOffSet[MidBase][Addr]+Temperature.fTempOffSet[KitMidBase][Addr]+Temperature.fMiddBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fHighBase)
                return Temperature.fTempOffSet[HigBase][Addr]+Temperature.fTempOffSet[KitHigBase][Addr]+Temperature.fHighBase+Temperature.fTempOffSet[UserOffSet][Addr];
        }
        else
        {
            if(T==Temperature.fLowBase)
                return Temperature.fTempOffSet[LowBase][Addr]+Temperature.fLowBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fMiddBase)
                return Temperature.fTempOffSet[MidBase][Addr]+Temperature.fMiddBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fHighBase)
                return Temperature.fTempOffSet[HigBase][Addr]+Temperature.fHighBase+Temperature.fTempOffSet[UserOffSet][Addr];
        }
    }
    else if(Temperature.iTempMode==2)                                           // 2 point base
    {
        ct1=0;
        ct2=2;
        if(CosFunction.bTemp5PointKitOffset==true)                              //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
        {
            if(T==Temperature.fLowBase)
                return Temperature.fTempOffSet[LowBase][Addr]+Temperature.fTempOffSet[KitLowBase][Addr]+Temperature.fLowBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fHighBase)
                return Temperature.fTempOffSet[HigBase][Addr]+Temperature.fTempOffSet[KitHigBase][Addr]+Temperature.fHighBase+Temperature.fTempOffSet[UserOffSet][Addr];
        }
        else
        {
            if(T==Temperature.fLowBase)
                return Temperature.fTempOffSet[LowBase][Addr]+Temperature.fLowBase+Temperature.fTempOffSet[UserOffSet][Addr];
            if(T==Temperature.fHighBase)
                return Temperature.fTempOffSet[HigBase][Addr]+Temperature.fHighBase+Temperature.fTempOffSet[UserOffSet][Addr];
        }
    }
    else
    {
        if(CosFunction.bTemp5PointKitOffset==true)                              //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
        {
            return T+Temperature.fTempOffSet[LowBase][Addr]+Temperature.fTempOffSet[KitLowBase][Addr]+Temperature.fTempOffSet[UserOffSet][Addr];
        }
        else
        {
            return T+Temperature.fTempOffSet[LowBase][Addr]+Temperature.fTempOffSet[UserOffSet][Addr];
        }
    }

    if(CosFunction.bTemp5PointKitOffset==true)                                  //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
    {
        Temp[0]=Temperature.fTempOffSet[LowBase][Addr]+Temperature.fTempOffSet[KitLowBase][Addr]+Temperature.fLowBase;
        Temp[1]=Temperature.fTempOffSet[MidBase][Addr]+Temperature.fTempOffSet[KitMidBase][Addr]+Temperature.fMiddBase;
        Temp[2]=Temperature.fTempOffSet[HigBase][Addr]+Temperature.fTempOffSet[KitHigBase][Addr]+Temperature.fHighBase;
        Temp[3]=Temperature.fTempOffSet[AmbientHotLow][Addr]+Temperature.fTempOffSet[KitAmbientHotLow][Addr]+Temperature.fAmbientHotLowBase;  //kevin 20140918 add
        Temp[4]=Temperature.fTempOffSet[AmbientHotMid][Addr]+Temperature.fTempOffSet[KitAmbientHotMid][Addr]+Temperature.fAmbientHotMiddBase;  //kevin 20140918 add
        Temp[5]=Temperature.fTempOffSet[SHigBase][Addr]+Temperature.fTempOffSet[KitHigBase][Addr]+Temperature.fSHighBase;//Ztex 2024.07.27 Add 6 Point Temperature Offset
    }
    else
    {
        Temp[0]=Temperature.fTempOffSet[LowBase][Addr]+Temperature.fLowBase;
        Temp[1]=Temperature.fTempOffSet[MidBase][Addr]+Temperature.fMiddBase;
        Temp[2]=Temperature.fTempOffSet[HigBase][Addr]+Temperature.fHighBase;

        Temp[3]=Temperature.fTempOffSet[AmbientHotLow][Addr]+Temperature.fAmbientHotLowBase;  //kevin 20140918 add
        Temp[4]=Temperature.fTempOffSet[AmbientHotMid][Addr]+Temperature.fAmbientHotMiddBase;  //kevin 20140918 add
    }

    if((iHighBase-iLowBase)==0)
        m=0;
    else
        m=(double)(iHighBase-T)/(double)(iHighBase-iLowBase);

    if(m==0)
        return T+Temperature.fTempOffSet[UserOffSet][Addr];
    s=Temp[ct2]+(Temp[ct1]-Temp[ct2])*m;
    sprintf(str, "%4.1f", s);
    return atof(str)+Temperature.fTempOffSet[UserOffSet][Addr];
}
//---------------------------------------------------------------------------
double GetFactSetTemp(int Addr, double T)                                       //ivy 970623
{
    double Temp;
    Temp=ConvertTempOffset(Addr, T);

    if(bUseInitTempOffset   &&                                                  //Steven 20141117 : 起測時溫度要補Offset
       iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)         //Steven 20141125 : <= --> <
    {
        if(bTestOverTimeTempOffsetF)
        {
            Temp+=Temperature.fTempOffSet[TestOverTimeTempOffset][Addr];        //kevin 20160312 add
        }
        else
        {
            Temp+=Temperature.fTempOffSet[InitTempOffset][Addr];
        }
    }
    else if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) &&     //Steven 20180817 : Boost Function
            iTriggerBoostFunction!=-1 &&
            (iBoostFuncStep==0 || iBoostFuncStep==1))
    {
        if(Temperature.iBoostFunctionMode==2)
        {
            if((Addr>=tcAa2 && Addr<=tcBd2) || (Addr>=tcAe2 && Addr<=tcBh2))
                Temp+=Temperature.dBoostOffset[iTriggerBoostFunction];

            if((Addr>=tcAa1 && Addr<=tcBd1) || (Addr>=tcAe1 && Addr<=tcBh1))
                Temp+=Temperature.dBoostOffset[iTriggerBoostFunction];
        }
        else
        {
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G01 -- golden :269-270.
//  golden's right disjunct derefs fContact->CarlibrationTask.  The only
//  translated fContact is atester_shims.h::TfContactShim, whose surface is
//  fShow (+bSetupStart/IsRun2DCheck) -- it has NO CarlibrationTask member, and
//  TfContact itself (golden cContact.cpp, 22761 lines) is untranslated; see
//  cContact.h's own scope banner.  Cannot compile.
//  ACTIVE DEFAULT: keep the left disjunct alone.  TfContactShim::fShow is false offline (its
//  ctor sets it and nothing offline sets it true), so the dropped conjunction
//  was already unreachable -- no observable delta.
#if 0 // TODO(W7-UI G01): golden :269-270
            if(IndexStatus==Z1Up_Z2Down ||
               (fContact->fShow && iContactMode!=CONTACT_NORMAL && fContact->CarlibrationTask==800))
#endif // TODO(W7-UI G01)
            if(IndexStatus==Z1Up_Z2Down)                                   //AI(W906-GA-BTHERMO) 20260807: G01 ACTIVE form
            {
                if((Addr>=tcAa2 && Addr<=tcBd2) || (Addr>=tcAe2 && Addr<=tcBh2))
                    Temp+=Temperature.dBoostOffset[iTriggerBoostFunction];
            }
            else
            {
                if((Addr>=tcAa1 && Addr<=tcBd1) || (Addr>=tcAe1 && Addr<=tcBh1))
                    Temp+=Temperature.dBoostOffset[iTriggerBoostFunction];
            }
        }
    }

//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G03a -- golden :283-288.
//  fLotInfo->bStartChamberBoost: forms/fLotInfo.h has no bStartChamberBoost member
//  (grepped); golden's member lives in the untranslated uLotInfo.h form.  The
//  member sits inside the IF CONDITION, so the whole block has to go.
//  ACTIVE DEFAULT: no Chamber-Boost offset is added.  The outer guard CosFunction.
//  bUseChamberBoostMode is false in the offline default config, so the block was
//  unreachable anyway.
//#if 0 // TODO(W7-UI G03a): golden :283-288   //AI(W906-S09-B1) 20260930 (St02-E, claim Q-INC): gate retired -- TfLotInfo::bStartChamberBoost is forms/fLotInfo.h:1811 (ctor false); differs only under bUseChamberBoostMode; body = golden 906_0625_Steven bthermo.cpp:283-288
    if(CosFunction.bUseChamberBoostMode &&
       Addr==tcChamber &&
       fLotInfo->bStartChamberBoost)                                            //Steven 20191128 : Chamber Boost Function
    {
        Temp+=Temperature.iChamberBoostOffset;
    }
//#endif // TODO(W7-UI G03a)   //AI(W906-S09-B1) 20260930 (St02-E, claim Q-INC): see :509

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20151230
    {
        if(Addr>=tcAa1 && Addr<=tcBd1)
            Temp+=Temperature.Arm1Offset;
        else if(Addr>=tcAa2 && Addr<=tcBd2)
            Temp+=Temperature.Arm2Offset;

        if(IniConfig.bL18NofullsiteaddTemperatureoffset)                        //wei 20160107 No FullSite Add Offset
        {
            if(Addr>=tcAa1 && Addr<=tcBd1)
            {
                if(iArmsitecount[0]==1)
                    Temp+=Temperature.Arm1NoFullsiteOffset_1;
                else if(iArmsitecount[0]==2)
                    Temp+=Temperature.Arm1NoFullsiteOffset_2;
                else if(iArmsitecount[0]==3)
                    Temp+=Temperature.Arm1NoFullsiteOffset_3;
                else if(iArmsitecount[0]==4)                                    //wei 20160603 add 兩組Nofullsite補償
                    Temp+=Temperature.Arm1NoFullsiteOffset_4;
                else if(iArmsitecount[0]==5)
                    Temp+=Temperature.Arm1NoFullsiteOffset_5;
            }
            else if(Addr>=tcAa2 && Addr<=tcBd2)
            {
                if(iArmsitecount[1]==1)
                    Temp+=Temperature.Arm2NoFullsiteOffset_1;
                else if(iArmsitecount[1]==2)
                    Temp+=Temperature.Arm2NoFullsiteOffset_2;
                else if(iArmsitecount[1]==3)
                    Temp+=Temperature.Arm2NoFullsiteOffset_3;
                else if(iArmsitecount[1]==4)                                    //wei 20160603 add 兩組Nofullsite補償
                    Temp+=Temperature.Arm2NoFullsiteOffset_4;
                else if(iArmsitecount[1]==5)
                    Temp+=Temperature.Arm2NoFullsiteOffset_5;
            }
        }
    }

    return Temp;
}
//---------------------------------------------------------------------------
double ConvertGetTempOffset(int Addr, double T)                                 //Steven 20141117 : 起測時溫度要補Offset
{
    int iLowBase =Temperature.fLowBase;
    int iHighBase=Temperature.fHighBase;

    double m=0.0,s=0.0,Temp[5]={0.0},fReturnTemp=0.0;                           //kevin 20140918  Temp[3]->Temp[5]
    int ct1=0,ct2=0;
    char str[30]="";
    double dWorkTemp;

    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值

//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G02 -- golden :343-346.
//  fProductionInfo->dTempTajOfs: forms/fProductionInfo.h (the translated
//  TfProductionInfo stand-in) has no dTempTajOfs member; golden's member lives in
//  the untranslated ProductionInfo.h form.  Cannot compile.
//  ACTIVE DEFAULT: the Greatek 'temperature by servo' addend is not applied (dWorkTemp unchanged).
#if 0 // TODO(W7-UI G02): golden :343-346
    if(CUSTOMER_CODE==CC_Greatek && IniConfig.bN14_18_EnableTempOffset)         //Sam 20200806 : 溫度 By Servo
    {
        dWorkTemp+=fProductionInfo->dTempTajOfs;
    }
#endif // TODO(W7-UI G02)

    if(Temperature.bUseFixTemp==true &&                                         //Steven 20131025 : 使4固定的DUT溫度
       Addr>=tcDUT1                  &&
       Addr<=tcDUT4)
    {
        dWorkTemp=Temperature.dFixedTemp;                                       //Steven 20131025 : 固定的DUT溫度值
    }

    if(LastSet.iTemperature==Tempture_Ambient && Temperature.bAmbientGuardbandCheck && IniConfig.bL20AbientGuardBand &&
       IniConfig.bSPILFunction==true)                                           //JerryYang 20230307 : 蘇州矽品要求常溫模式下勾選Ambient guardband check, 要吃溫度offset
    {
    }
    else if(LastSet.iTemperature==Tempture_Ambient)                             //常溫，傳回溫控器實際溫度
    {
        return T;
    }
    T=T-Temperature.fTempOffSet[UserOffSet][Addr];
    if(CosFunction.bTemp5PointKitOffset==true)                                  //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
    {
        Temp[0]=Temperature.fTempOffSet[LowBase][Addr]+Temperature.fTempOffSet[KitLowBase][Addr]+Temperature.fLowBase;
        Temp[1]=Temperature.fTempOffSet[MidBase][Addr]+Temperature.fTempOffSet[KitMidBase][Addr]+Temperature.fMiddBase;
        Temp[2]=Temperature.fTempOffSet[HigBase][Addr]+Temperature.fTempOffSet[KitHigBase][Addr]+Temperature.fHighBase;
        Temp[3]=Temperature.fTempOffSet[AmbientHotLow][Addr]+Temperature.fTempOffSet[KitAmbientHotLow][Addr]+Temperature.fAmbientHotLowBase;  //kevin 20140918 Start
        Temp[4]=Temperature.fTempOffSet[AmbientHotMid][Addr]+Temperature.fTempOffSet[KitAmbientHotMid][Addr]+Temperature.fAmbientHotMiddBase;
    }
    else
    {
        Temp[0]=Temperature.fTempOffSet[LowBase][Addr]+Temperature.fLowBase;
        Temp[1]=Temperature.fTempOffSet[MidBase][Addr]+Temperature.fMiddBase;
        Temp[2]=Temperature.fTempOffSet[HigBase][Addr]+Temperature.fHighBase;
        Temp[3]=Temperature.fTempOffSet[AmbientHotLow][Addr]+Temperature.fAmbientHotLowBase;  //kevin 20140918 Start
        Temp[4]=Temperature.fTempOffSet[AmbientHotMid][Addr]+Temperature.fAmbientHotMiddBase;
    }

    if(Temperature.iTempMode==8) // Five point base
    {
        iLowBase=Temperature.fAmbientHotLowBase;
        if(T>Temp[1])
        {
            iLowBase=Temperature.fMiddBase;
            ct1=1;
            ct2=2;
        }
        else if(T>Temp[0])
        {
            iLowBase=Temperature.fLowBase;
            iHighBase=Temperature.fMiddBase;
            ct1=0;
            ct2=1;
        }
        else if(T>Temp[4])  //最低溫
        {
            iLowBase=Temperature.fAmbientHotMiddBase;
            iHighBase=Temperature.fLowBase;
            ct1=4;
            ct2=0; //高溫 - 低溫     CT2 -CT1
        }
        else
        {
            iHighBase=Temperature.fAmbientHotMiddBase;
            ct1=3;      //低溫
            ct2=4;      //高溫
        }

        //jou 2012-08-01 如果工作溫度剛好在Base，不需要再做斜率m轉換輸出
        if(dWorkTemp==Temperature.fAmbientHotLowBase)  //kevin 20140918
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[AmbientHotLow][Addr]-Temperature.fTempOffSet[KitAmbientHotLow][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[AmbientHotLow][Addr];
            }
            return fReturnTemp;
        }

        if(dWorkTemp==Temperature.fAmbientHotMiddBase) //kevin 20140918
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[AmbientHotMid][Addr]-Temperature.fTempOffSet[KitAmbientHotMid][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[AmbientHotMid][Addr];
            }
            return fReturnTemp;
        }

        if(dWorkTemp==Temperature.fLowBase)
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[LowBase][Addr]-Temperature.fTempOffSet[KitLowBase][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[LowBase][Addr];
            }

            return fReturnTemp;
        }

        if(dWorkTemp==Temperature.fMiddBase)
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[MidBase][Addr]-Temperature.fTempOffSet[KitMidBase][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[MidBase][Addr];
            }

            return fReturnTemp;
        }

        if(dWorkTemp==Temperature.fHighBase)
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[HigBase][Addr]-Temperature.fTempOffSet[KitHigBase][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[HigBase][Addr];
            }

            return fReturnTemp;
        }
    } //kevin 20140918 end
    else if(Temperature.iTempMode==4) // three point base
    {
        if(T>Temp[1])
        {
            iLowBase=Temperature.fMiddBase;
            ct1=1;
            ct2=2;
        }
        else
        {
            iHighBase=Temperature.fMiddBase;
            ct1=0;
            ct2=1;
        }

        //jou 2012-08-01 如果工作溫度剛好在Base，不需要再做斜率m轉換輸出
        if(dWorkTemp==Temperature.fLowBase)
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[LowBase][Addr]-Temperature.fTempOffSet[KitLowBase][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[LowBase][Addr];
            }
            return fReturnTemp;
        }

        if(dWorkTemp==Temperature.fMiddBase)
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[MidBase][Addr]-Temperature.fTempOffSet[KitMidBase][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[MidBase][Addr];
            }
            return fReturnTemp;
        }

        if(dWorkTemp==Temperature.fHighBase)
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[HigBase][Addr]-Temperature.fTempOffSet[KitHigBase][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[HigBase][Addr];
            }
            return fReturnTemp;
        }
    }
    else if(Temperature.iTempMode==2)
    {
        ct1=0;
        ct2=2;
        if(dWorkTemp==Temperature.fLowBase)
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[LowBase][Addr]-Temperature.fTempOffSet[KitLowBase][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[LowBase][Addr];
            }
            return fReturnTemp;
        }

        if(dWorkTemp==Temperature.fHighBase)
        {
            if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
            {
                fReturnTemp=T-Temperature.fTempOffSet[HigBase][Addr]-Temperature.fTempOffSet[KitHigBase][Addr];
            }
            else
            {
                fReturnTemp=T-Temperature.fTempOffSet[HigBase][Addr];
            }
            return fReturnTemp;
        }
    }
    else
    {
        if(CosFunction.bTemp5PointKitOffset==true)        //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
        {
            fReturnTemp=T-Temperature.fTempOffSet[LowBase][Addr]-Temperature.fTempOffSet[KitLowBase][Addr];
        }
        else
        {
            fReturnTemp=T-Temperature.fTempOffSet[LowBase][Addr];
        }
        return fReturnTemp;
    }

    if(iLowBase==iHighBase)
    {
        return T;
    }

    if((iHighBase-iLowBase)==0)
        m=0;
    else
        m=(double)(Temp[ct2]-Temp[ct1])/(double)(iHighBase-iLowBase);
    if(m==0)
    {
        fReturnTemp=Temp[ct1];
        return fReturnTemp;
    }

    s=iHighBase+(T-Temp[ct2])/m;
    sprintf(str,"%4.1f",s);

    fReturnTemp=atof(str);

    return fReturnTemp;
}
//---------------------------------------------------------------------------
double GetConvertTemp(int Addr, double T)     //ivy 970623
{
    if(bUseInitTempOffset   &&   //Steven 20141117 : 起測時溫度要補Offset
       iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)         //Steven 20141125 : <= --> <
    {
        if(bTestOverTimeTempOffsetF)
        {
            T-=Temperature.fTempOffSet[TestOverTimeTempOffset][Addr];           //kevin 20160312 add
        }
        else
        {
            T-=Temperature.fTempOffSet[InitTempOffset][Addr];
        }
    }
    else if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) &&     //Steven 20180817 : Boost Function
            iTriggerBoostFunction!=-1 &&
            (iBoostFuncStep==0 || iBoostFuncStep==1))
    {
        if(Temperature.iBoostFunctionMode==2)
        {
            if((Addr>=tcAa2 && Addr<=tcBd2) || (Addr>=tcAe2 && Addr<=tcBh2))
                T-=Temperature.dBoostOffset[iTriggerBoostFunction];

            if((Addr>=tcAa1 && Addr<=tcBd1) || (Addr>=tcAe1 && Addr<=tcBh1))
                T-=Temperature.dBoostOffset[iTriggerBoostFunction];
        }
        else
        {
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G01 -- golden :629-630.
//  golden's right disjunct derefs fContact->CarlibrationTask.  The only
//  translated fContact is atester_shims.h::TfContactShim, whose surface is
//  fShow (+bSetupStart/IsRun2DCheck) -- it has NO CarlibrationTask member, and
//  TfContact itself (golden cContact.cpp, 22761 lines) is untranslated; see
//  cContact.h's own scope banner.  Cannot compile.
//  ACTIVE DEFAULT: keep the left disjunct alone.  TfContactShim::fShow is false offline (its
//  ctor sets it and nothing offline sets it true), so the dropped conjunction
//  was already unreachable -- no observable delta.
#if 0 // TODO(W7-UI G01): golden :629-630
            if(IndexStatus==Z1Up_Z2Down ||
               (fContact->fShow && iContactMode!=CONTACT_NORMAL && fContact->CarlibrationTask==800))
#endif // TODO(W7-UI G01)
            if(IndexStatus==Z1Up_Z2Down)                                   //AI(W906-GA-BTHERMO) 20260807: G01 ACTIVE form
            {
                if((Addr>=tcAa2 && Addr<=tcBd2) || (Addr>=tcAe2 && Addr<=tcBh2))
                    T-=Temperature.dBoostOffset[iTriggerBoostFunction];
            }
            else
            {
                if((Addr>=tcAa1 && Addr<=tcBd1) || (Addr>=tcAe1 && Addr<=tcBh1))
                    T-=Temperature.dBoostOffset[iTriggerBoostFunction];
            }
        }
    }

//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G03b -- golden :643-646.
//  same member as G03a, subtract side (golden GetConvertTemp).
//  ACTIVE DEFAULT: no Chamber-Boost offset is subtracted (mirror of G03a).
//#if 0 // TODO(W7-UI G03b): golden :643-646   //AI(W906-S09-B1) 20260930 (St02-E, claim Q-INC): gate retired -- same member as G03a (the subtracting side); body = golden 906_0625_Steven bthermo.cpp:643-646
    if(CosFunction.bUseChamberBoostMode && Addr==tcChamber && fLotInfo->bStartChamberBoost)        //Steven 20191128 : Chamber Boost Function
    {
        T-=Temperature.iChamberBoostOffset;
    }
//#endif // TODO(W7-UI G03b)   //AI(W906-S09-B1) 20260930 (St02-E, claim Q-INC): see :893

    double fReturnTemp=ConvertGetTempOffset(Addr, T);            //Steven 20150728 : 跟上面的換位置

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)        //wei 20151230
    {
        if(Addr>=tcAa1 && Addr<=tcBd1)
            fReturnTemp-=Temperature.Arm1Offset;
        else if(Addr>=tcAa2 && Addr<=tcBd2)
            fReturnTemp-=Temperature.Arm2Offset;

        if(IniConfig.bL18NofullsiteaddTemperatureoffset)      //wei 20160107 No FullSite Add Offset
        {
            if(Addr>=tcAa1 && Addr<=tcBd1)
            {
                if(iArmsitecount[0]==1)
                    fReturnTemp-=Temperature.Arm1NoFullsiteOffset_1;
                else if(iArmsitecount[0]==2)
                    fReturnTemp-=Temperature.Arm1NoFullsiteOffset_2;
                else if(iArmsitecount[0]==3)
                    fReturnTemp-=Temperature.Arm1NoFullsiteOffset_3;
                else if(iArmsitecount[0]==4)                            //wei 20160603 add 兩組Nofullsite補償
                    fReturnTemp-=Temperature.Arm1NoFullsiteOffset_4;
                else if(iArmsitecount[0]==5)
                    fReturnTemp-=Temperature.Arm1NoFullsiteOffset_5;
            }
            else if(Addr>=tcAa2 && Addr<=tcBd2)
            {
                if(iArmsitecount[1]==1)
                    fReturnTemp-=Temperature.Arm2NoFullsiteOffset_1;
                else if(iArmsitecount[1]==2)
                    fReturnTemp-=Temperature.Arm2NoFullsiteOffset_2;
                else if(iArmsitecount[1]==3)
                    fReturnTemp-=Temperature.Arm2NoFullsiteOffset_3;
                else if(iArmsitecount[1]==4)                            //wei 20160603 add 兩組Nofullsite補償
                    fReturnTemp-=Temperature.Arm2NoFullsiteOffset_4;
                else if(iArmsitecount[1]==5)
                    fReturnTemp-=Temperature.Arm2NoFullsiteOffset_5;
            }
        }
    }

    return fReturnTemp;
}
//---------------------------------------------------------------------------
//ChungHung 20130910 alter for SCK can close site by Index
bool bGetHeaterUsed(int Addr)
{                                                                               //QQ 沒考慮到關Arm的情況!!
    if(TestIF.iTestMode==DualSite)                                              // dual site
    {
        if((bTestSiteUse[0][0][1]==false && Addr==tcHead2))
        {
            return false;
        }
        else if(bTestSiteUse[1][0][1]==false && Addr==tcHead4)
        {
            return false;
        }
        else
        {
            if(IniConfig.bD30EnableSiteModeSelect && TestIF.iShuttleMode==1)
            {
                if(TestIF.iShuttle_Sel==0)                                      //Front Arm Only
                {
                    if(Addr==tcHead4)
                        return false;
                }
                else
                {
                    if(Addr==tcHead2)
                        return false;
                }
            }
        }

        if(bTestSiteUse[0][0][0]==false && Addr==tcHead1)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][0]==false && Addr==tcHead3)
        {
            return false;
        }
        else
        {
            if(IniConfig.bD30EnableSiteModeSelect && TestIF.iShuttleMode==1)
            {
                if(TestIF.iShuttle_Sel==0)                                      //Front Arm Only
                {
                    if(Addr==tcHead3)
                        return false;
                }
                else
                {
                    if(Addr==tcHead1)
                        return false;
                }
            }
        }
    }
    else if(TestIF.iTestMode==SingleSite || TestIF.iTestMode==DualSite2x1)      // SingleSite
    {
        if(IniConfig.bD30EnableSiteModeSelect && TestIF.iShuttleMode==1)
        {
            if(TestIF.iShuttle_Sel==0)                                          //Front Arm Only
            {
                if(Addr==tcHead3)
                    return false;
            }
            else
            {
                if(Addr==tcHead1)
                    return false;
            }
        }
        return true;
    }
    else if(TestIF.iTestMode==QualSite1X4 ||
            TestIF.iTestMode==_8Site1X4)                                        //ChungHung 20150528 add for 海思 _8Site1x4 // qual site 1X4
    {
        if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false && Addr==tcHead1)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false && Addr==tcHead3)
        {
            return false;
        }
        else if(bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false && Addr==tcHead2)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false && Addr==tcHead4)
        {
            return false;
        }
    }
    else if(TestIF.iTestMode==QualSite2X2)                                      // qual site 2X2
    {
        if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][1][0]==false && Addr==tcHead1)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][1][0]==false && Addr==tcHead3)
        {
            return false;
        }
        else if(bTestSiteUse[0][0][1]==false && bTestSiteUse[0][1][1]==false && Addr==tcHead2)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][1]==false && bTestSiteUse[1][1][1]==false && Addr==tcHead4)
        {
            return false;
        }
    }
    else if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4)        //Sam 20190226 : 16Site4X4 //8 Site 2X4
    {
        if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false &&
           bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][1]==false && Addr==tcHead1)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false &&
                bTestSiteUse[1][1][0]==false && bTestSiteUse[1][1][1]==false && Addr==tcHead3)
        {
            return false;
        }
        else if(bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false &&
                bTestSiteUse[0][1][2]==false && bTestSiteUse[0][1][3]==false && Addr==tcHead2)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false &&
                bTestSiteUse[1][1][2]==false && bTestSiteUse[1][1][3]==false && Addr==tcHead4)
        {
            return false;
        }
    }
    else                                                                        //i6Site
    {
        if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false &&
           bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false &&
           bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][1]==false &&
           bTestSiteUse[0][1][2]==false && bTestSiteUse[0][1][3]==false && Addr==tcHead1)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false &&
                bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false &&
                bTestSiteUse[1][1][0]==false && bTestSiteUse[1][1][1]==false &&
                bTestSiteUse[1][1][2]==false && bTestSiteUse[1][1][3]==false && Addr==tcHead3)
        {
            return false;
        }
        else if(bTestSiteUse[0][0][4]==false && bTestSiteUse[0][0][5]==false &&
                bTestSiteUse[0][0][6]==false && bTestSiteUse[0][0][7]==false &&
                bTestSiteUse[0][1][4]==false && bTestSiteUse[0][1][5]==false &&
                bTestSiteUse[0][1][6]==false && bTestSiteUse[0][1][7]==false && Addr==tcHead2)
        {
            return false;
        }
        else if(bTestSiteUse[1][0][4]==false && bTestSiteUse[1][0][5]==false &&
                bTestSiteUse[1][0][6]==false && bTestSiteUse[1][0][7]==false &&
                bTestSiteUse[1][1][4]==false && bTestSiteUse[1][1][5]==false &&
                bTestSiteUse[1][1][6]==false && bTestSiteUse[1][1][7]==false && Addr==tcHead4)
        {
            return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
unsigned char A_Check_Addr(char ID_Addr)
{
    unsigned char Btmp,Btmp1;

    Btmp =T_ASXII2HEX_Mac(READBUFF[1]);                                         //CONV 0-9 A-F
    Btmp1=T_ASXII2HEX_Mac(READBUFF[2]);

    if(Btmp  & 0xF0)    return(ERROR_CODE);
    if(Btmp1 & 0xF0)     return(ERROR_CODE);

    Btmp=(Btmp<<4) | Btmp1;                                                     //CONV 0-9 A-F

    if(ID_Addr != Btmp)  return(ERROR_CODE);
    return(OK_CODE);
}
//---------------------------------------------------------------------------
unsigned char A_Check_LRC(void)
{
    unsigned char Btmp, Btmp1;
//jou 2011-08-18 start : 0-4 = -4 under array error
    int iCT=0;
//AI(W906-GA-BTHERMO) 20260807: golden :880 called the BCB6 SysUtils `StrLen(const char*)`.  vclcompat
//  does not re-export it and no port header defines it.  Borland StrLen IS ISO
//  strlen -- 'returns the number of characters in Str, excluding the null"
//  terminator" -- so this is a 1:1 name substitution, not a behaviour change.  The
//  cast is the same READBUFF `unsigned char[1024]` -> `char*` conversion BCB6 did
//  implicitly and ISO C++ does not (see the four ::strncpy sites below).
    iCT=::strlen((const char *)READBUFF)-4;
    if(iCT<0 || iCT>=21)
    {
        return(ERROR_CODE);
    }
    Btmp1 =          (T_ASXII2HEX_Mac(READBUFF[iCT]));                          //CONV 0-9 A-F     LRC  HIGH
    iCT++;
    Btmp1 =(Btmp1<<4) | T_ASXII2HEX_Mac(READBUFF[iCT]);                         //CONV 0-9 A-F    LRC  LOW

    Btmp=A_Create_LCR(&READBUFF[1],iCT-2);

    if(Btmp!=Btmp1)
        return(ERROR_CODE);
    else
        return(OK_CODE);
}
//---------------------------------------------------------------------------
//AI(W906-GA-BTHERMO) 20260807: __fastcall dropped (golden :897 `void __fastcall ClearSendBuff(void)`).
//  __fastcall is a REAL MinGW keyword, never neutralized; house convention is no
//  __fastcall in translated .cpp bodies.  Safe here: ClearSendBuff is declared in no
//  header and referenced by NO other golden TU (whole-tree grep hits bthermo.cpp
//  only), so no cross-TU name mangling can disagree.
void ClearSendBuff( void)
{
    ZeroMemory(&READBUFF, sizeof(READBUFF));                                    //Steven 20140306 : 進入Config通訊後會當機
}
//------------Procress Read String  for ASCII MODE  -------------------------
unsigned char A_Get_Function_Code(void)
{
    unsigned char Btmp;

    Btmp = T_ASXII2HEX_Mac(READBUFF[3]);                                        //CONV 0-9 A-F     LRC  HIGH
    Btmp = (Btmp<<4)|T_ASXII2HEX_Mac(READBUFF[4]);                              //CONV 0-9 A-F    LRC  LOW
    return(Btmp);
}
//---------------------------------------------------------------------------
unsigned int A_Get_MEM_Address(void)
{
    unsigned char Btmp, Btmp1;
    Btmp = T_ASXII2HEX_Mac(READBUFF[5]);                                        //CONV 0-9 A-F  Address High
    Btmp = (Btmp<<4)| T_ASXII2HEX_Mac(READBUFF[6]);                             //CONV 0-9 A-F
    Btmp1 = T_ASXII2HEX_Mac(READBUFF[7]);                                       //CONV 0-9 A-F
    Btmp1 = (Btmp1<<4)|T_ASXII2HEX_Mac(READBUFF[8]);                            //CONV 0-9 A-F  Address Low
    return(Btmp<<8 | Btmp1);
}
//---------------------------------------------------------------------------
unsigned int  A_Get_MEM_Word(void)
{
    unsigned char Btmp,Btmp1;
    Btmp =T_ASXII2HEX_Mac(READBUFF[7]);                                         //CONV 0-9 A-F  Word High
    Btmp = (Btmp<<4)| T_ASXII2HEX_Mac(READBUFF[8]);                             //CONV 0-9 A-F
    Btmp1 =T_ASXII2HEX_Mac(READBUFF[9]);                                        //CONV 0-9 A-F
    Btmp1 = (Btmp1<<4)|T_ASXII2HEX_Mac(READBUFF[10]);                           //CONV 0-9 A-F  Word Low
    return(Btmp<<8 | Btmp1);
}
//---------------------------------------------------------------------------
const int iTotalNum=30;
double fBuffer[tcTotalCount][iTotalNum];

void ClearHotBuffer(int iAddr)
{
    for(int i=0; i<iTotalNum; i++)
        fBuffer[iAddr][i]=0;
}
//---------------------------------------------------------------------------
void ClearAllHotBuffer()
{
    for(int Channel=0; Channel<tcTotalCount; Channel++)
        ClearHotBuffer(Channel);

    for(int i=0; i<tcTotalCount; i++)
        UN150Read[i]=0.0;
}
//---------------------------------------------------------------------------
double ReadAverageBuffer(int Addr, double fData)
{
    static int iWriteCT[tcTotalCount]={0};
    double fTotal=0, fReturnData=0.0;       //Jimmychiu 20230307 initialized in the constructor
    int iCT=0;
    double iMaxValue=0;

    if(fData>=150)
    {
        ClearHotBuffer(Addr);
        return fData;
    }

    fBuffer[Addr][iWriteCT[Addr]]=fData;

    for(int i=0; i<iTotalNum; i++)
    {
        //if(fBuffer[Addr][i]==0)
        if(fBuffer[Addr][i]<=0) //Steven 20100201
            continue;

        if(iMaxValue<fBuffer[Addr][i])
        {
            iMaxValue=fBuffer[Addr][i];
        }

        fTotal+=fBuffer[Addr][i];
        iCT++;
    }

    if(iCT!=0)
    {
        if(iCT>=iTotalNum)                                                      //jou 2011-12-20 增加過濾雜訊的功能
        {
            fTotal=fTotal-iMaxValue;
            iCT--;
        }
        fReturnData=fTotal/iCT;
    }

    iWriteCT[Addr]++;
    if(iWriteCT[Addr]>=iTotalNum)
    {
        iWriteCT[Addr]=0;
    }

    return fReturnData;
}
//---------------------------------------------------------------------------
int iThermoTask=1;
void DoThermo()
{
    if(TC401HeaterControl==NoHeater)                                            //Steven 20171227 (Wei) : Add for HT-9045L
        return;

//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G14a -- golden :1004-1005.
//  fConfiguration (golden cConfiguration.h TfConfiguration form) is unported.  The
//  only `fConfiguration` in this tree is Automation/SCK_ART_Remainder.h's TU-local
//  W5SckArtRem_ConfigStub, which carries mmoN04_IP ONLY -- no fShow, no
//  PageControl1, no ecp1TabSheet9.  Cannot compile.
//  ACTIVE DEFAULT: the early-return is dropped.  Offline the Config form is never shown, so the
//  guard could never fire -- no observable delta.
//AI(W906-I03) 20261002 (Ifor01): GATE W7-UI G14a OPENED (the 20260807 note above is history) -- H1-08's substitution (csystem.cpp:24854): fShow -> page table, ActivePageIndex -> FileRW_ProxyPageIndex, ecp1TabSheet9 = 1 (golden cConfiguration.h:2370 = tsTempComm); -1 in programs without FileRW
    { extern bool FileRW_ProxyTryRead(const char*, const char*, bool*, const char*, int*); int pageIndex=-1; if(W906_FormShowing("fConfiguration", false)==true && (FileRW_ProxyTryRead("TfConfiguration", 0, 0, "PageControl1", &pageIndex)==false || pageIndex==1))   // golden :1004 fConfiguration->fShow==true && fConfiguration->PageControl1->ActivePageIndex==fConfiguration->ecp1TabSheet9   //AI(W906-FASTCLK) 20261003: the same Configuration read as the cooling fan's H1-08 (csystem.cpp DoSwCoolingFan) in the same 20 ms heater body, so the same change (RULINGS_20261002 #7): no longer waits for FormLock (FileRW/_ProxyTry.cpp try-lock); held by another thread -> return = this beat is skipped (golden's own early-return outcome)
        return; }                                                               // golden :1005
//AI(W906-I03) 20261002: (the old #endif of G14a)

    static bool bMultiple=false;
    if(bMultiple==true)
        return;

    bMultiple=true;  { extern void W906_PumpTempComm2(); W906_PumpTempComm2(); }   //AI(W906-I03) 20261002: Comm2 的收件交付（rs232.cpp：讀取執行緒排隊、這裡安靜滿 100 ms 才交給 golden Comm2ReceiveData），在 DoThermoReal 讀 Com2ReceiveOK 之前
    DoThermoReal();

    if(ATC_SYSTEM==eWinWay && Temperature.bATCActiveCooling==true)              //Jimmychiu 20210906
    {
    }
    else if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                           //20141204 ChungHung add for ATC3.0   //ChungHung 20141024 add
    {
        DoATC60Temperature();                                                   //ChungHung 21041105 add for ATC6.0 and ATC3.0
    }
    else if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling==true)   //Ifor 20151230 :add New ATC Interface
    {
        ;
    }
    else if(ATC_SYSTEM==eATCHonPrecType && (Temperature.bATC70Active==true || Temperature.bATCActiveCooling==true))   //Steven 20160816 : fixed for Running direct heat on ATC handler
    {
        ;
    }
    else
    {
        if(USE_16_HEATER==eht16HeaterEJ1N ||                                    //Steven 20120220 : Omron EJ1N溫控器
           USE_16_HEATER==eht32HeaterEJ1N)                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
        {
            DoSetSVOfOmronEJ1N();
        }
        else if(USE_16_HEATER==eht16HeaterDTME08 ||                             //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
                USE_16_HEATER==eht32HeaterDTME08)                               //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
        {
            DoSetSVOfDTME08();
        }

        if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true && Temperature.bATCTemperatureSet==true &&
           (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite) && bATCInitialFinish==false)    //jou 2012-03-14 增加ATC控制方式
        {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G15 -- golden :1045-1048.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no ATCInitital(), and no Comm2 member at all.  Cannot compile.
//  ACTIVE DEFAULT: bATCInitialFinish stays false, i.e. 'ATC not initialised', which is the offline
//  truth (there is no serial port).
#if 0 // TODO(W5 G15): golden :1045-1048
            if(COM2->ATCInitital())
            {
                bATCInitialFinish=true;
            }
#endif // TODO(W5 G15)
        }
    }
    bMultiple=false;
}
//---------------------------------------------------------------------------
TQPF_Timer tThermoAlarmTimer;
//---------------------------------------------------------------------------
void DoThermoReal()
{
    static int Addr=tcHotPlate1;
    static int &Task=iThermoTask;
    static int CommRetry[tcTotalCount];
    static int WriteCommand=-1, ReadCommand=-1, ReceiveErr=-1;
    static int CommunCTErr[tcTotalCount]={0};                                   //JerryYang 20170929 (wei) CommunCTErr改成陣列
    static int iChangeHotPlate=1;
    static int iChangeShuttle=1;                                                //kevin 20150508 shuttle
    static bool bHasErrorSet=false;                                             //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20
    static double OldTemp[tcTotalCount];
    static AnsiString asErrorAddr="";                                           //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20

    char *pRead;
    int MAX_RETRY=2;
    int pH, pL, i, P;
//    int iATCAddr=0;
    double Read, Temp;
    double dWorkTemp, dWorkTempHP1, dWorkTempHP2, dWorkTempChamber;
    double dWorkTempDut1, dWorkTempDut2, dWorkTempDut3, dWorkTempDut4;
    double dbSetATCAdjustmentOffset;
    AnsiString S, S1, S2, S3;

//AI(W906-GA-BTHERMO) 20260807: bHasErrorSet is read ONLY inside gate G19a below.  Retained
//  verbatim per golden and silenced rather than deleted.
    (void)bHasErrorSet;

    if(PauseUT150Polling)
    {
        Task=1;
        return;
    }

    if(CUSTOMER_CODE==CC_ASE_SG &&                                              //Ifor 20220318 add: Broadcom 要求測試中讀取ATC溫度要更即時一點
       ATC_SYSTEM==eNewATCSystem &&
       Temperature.bATCActiveCooling==true)
    {
        for(i=0; i<16; i++)
        {
            UN150Read[tcAa1+i]=DOUN150ReadTemp(tcAa1+i);
            UN150Read[tcAe1+i]=DOUN150ReadTemp(tcAe1+i);
        }
    }

     switch(Task)
     {
        case 1:
            Addr=tcHotPlate1;
            for(i=0; i<tcTotalCount; i++)
            {
                OldTemp[i]=-1;
                CommRetry[i]=MAX_RETRY;
            }
            iChangeHotPlate=1;
            UN150ReadHotplate[0]=0;
            UN150ReadHotplate[1]=0;
            iChangeShuttle=1;                                                   //kevin 20150508 shuttle
            UN150ReadShuttle[0]=0;                                              //kevin 20150508 shuttle 分段加熱
            UN150ReadShuttle[1]=0;                                              //kevin 20150508 shuttle 分段加熱
            Task=100;
            break;
        case 100:                                                               //Ifor 20161118 整合NewATC & ATC2.0 並移除(TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite)判斷
            if(Tri_Temp_Machine==1)                                             //Ztex 2023.04.19 Add HT-1032 TriTemp Function
            {
                if(((Addr>=tcAa1 && Addr<=tcBd2) || (Addr>=tcAe1 && Addr<=tcBh2)) ||
                    Addr==tcHotPlate1 || Addr==tcHotPlate2 || Addr==tcShuttle1 || Addr==tcShuttle2 ||
                    Addr==tcHotPlate3 || Addr==tcHotPlate4 || Addr==tcShuttle3 || Addr==tcShuttle4 ||
                    Addr==tcATCHotAir1 || Addr==tcATCHotAir2
                  )
                {
                    Task=270;                                                   //讀取Site 溫度
                    break;
                }
                else if(bUT150Install[Addr]==false)
                {
                    if((Addr>=tcDUT1 && Addr<=tcDUT4) ||
                       (Addr>=tcOutSht1 && Addr<=tcBase6) ||
                       (Addr>=tcDoor1 && Addr<=tcDoor2))                        //Ztex 2023.10.23 Add Index Door Heater
                    {
                        ;
                    }
                    else
                    {
                        Task=300;
                        break;
                    }
                }
            }
            else if((ATC_SYSTEM==eNewATCSystem || ATC_SYSTEM==eATCHonPrecType) &&
                    (Temperature.bATCActiveCooling==true) &&
                    ((Addr>=tcAa1 && Addr<=tcBd2) || (Addr>=tcAe1 && Addr<=tcBh2)))
            {
                Task=270;
                break;
            }
            else if(ATC_SYSTEM==eWinWay &&
                    Temperature.bATCActiveCooling==true &&                      //Jimmychiu 20210906
                   ((Addr>=tcAa1 && Addr<=tcBd2) ||
                    (Addr>=tcAe1 && Addr<=tcBh2)))
            {
                Task=270;
                break;
            }
            else if((USE_16_HEATER==eht16HeaterEJ1N ||
                     USE_16_HEATER==eht32HeaterEJ1N ||
                     USE_16_HEATER==eht16Heater     ||
                     USE_16_HEATER==eht16HeaterDTME08 ||    //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
                     USE_16_HEATER==eht32HeaterDTME08  ) && //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
                    (Addr>=tcAa1 && Addr<=tcBd2))  //Steven 20160218 : 修正KT4H版本Index溫度不會顯示的問題
            {                                                                    //socket base 1個 溫控器-> 4 溫控器控制加熱
                if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true &&
                   (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite) &&
                   (Addr>=tcAa1 && Addr<=tcBd2))                                //jou 2012-03-14 增加ATC控制方式
                {
                    ;
                }
                else
                {
                    if(USE_16_HEATER!=eht16Heater)
                    {
                        Task=300;
                        break;
                    }
                }
            }
            else if((USE_16_HEATER==eht32HeaterEJ1N ||
                     USE_16_HEATER==eht32HeaterDTME08) &&
                    (Addr>=tcAe1 && Addr<=tcBh2))                               //Steven 20140923 : Index使用EJ1N版32組加熱器
            {
                if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true &&
                   (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite) && (Addr>=tcAe1 && Addr<=tcBh2))    //jou 2012-03-14 增加ATC控制方式
                {
                    ;
                }
                else
                {
                    Task=300;
                    break;
                }
            }
            else if(Addr>=tcHead1 && Addr<=tcHead4)
            {
                if((ATC_SYSTEM==eNewATCSystem || ATC_SYSTEM==eATCHonPrecType) && Temperature.bATCActiveCooling==true)
                {
                    Task=270;
                    break;
                }
                //<==
                //Ifor 20161118 移除(TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite)判斷
            }

            if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)  //kevin 20140918 恆溫控制
            {
                if(bUT150Install[Addr]==false)
                {
                    Temp=0.0;
                }
                else
                {
                    //----- by dell ccd realtime-------------Steven 20110811
                    if(Addr==tcCCD || Addr==tc2D || (RTC_TemperNumber==2 && Addr==tcCCD_2))           //wei 20160524 2D溫度   //Isaac 20201217 : RTC CCD增加第二組感溫
                    {
                        Temp=0;
                    }
//AI(W906-GA-BTHERMO) 20260807: GOLDEN QUIRK preserved VERBATIM (golden :1216-1219) -- operator
//  precedence.  `A && (B || C) || D` binds as `(A && (B||C)) || D`, so the
//  tcDoor1..tcDoor2 range on :1219 is NOT gated by Tri_Temp_Machine==1 even
//  though the stray `)` the author left in that line's own trailing comment shows
//  it was meant to be.  g++ flags it (-Wparentheses); the sibling test at golden
//  :1127-1129 writes the same three ranges as one flat OR chain and is correct.
//  NOT fixed -- faithful translation.
                    else if(Tri_Temp_Machine==1 &&                      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                            ((Addr>=tcDUT1 && Addr<=tcDUT4) ||
                             (Addr>=tcOutSht1 && Addr<=tcBase6)) ||
                             (Addr>=tcDoor1 && Addr<=tcDoor2))                  //Ztex 2023.10.23 Add Index Door Heater)
                    {
                        dWorkTemp=Temperature.fIndividualTemp[Addr];    //Heater溫度設定
                        Temp=GetFactSetTemp(Addr, dWorkTemp);
                    }
                    else if(Addr==tcATCHotAir1 || Addr==tcATCHotAir2)   //JerryYang 20220408 : add for ATC3.5
                    {
                        Temp=IniConfig.dHeatGunTempATC;
                    }
                    else
                    {
                        //jou 2012-03-14 增加ATC控制方式
                        //Steven 20150211 : Index使用KT4H版32組加熱器
                        //Ifor 20161021 add 整合ATC Type
                        if((ATC_SYSTEM==eNewATCSystem || ATC_SYSTEM==eATCHonPrecType ||ATC_SYSTEM==eATCSiliconType) &&
                            Temperature.bATCActiveCooling==true &&
                           ((Addr>=tcAa1 && Addr<=tcBd2) || (Addr>=tcAe1 && Addr<=tcBh2)))
                        {
                            Temp=Temperature.fWorkTemperBase;
                        }
                        //<==
                        else
                        {
                            if(Temperature.bUseFixTemp==true &&         //Steven 20131025 : 使4固定的DUT溫度
                               Addr>=tcDUT1                  &&
                               Addr<=tcDUT4)
                            {
                                Temp=GetFactSetTemp(Addr, Temperature.dFixedTemp);      //Steven 20131025 : 固定的DUT溫度值
                            }
                            else
                            {
                                dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                Temp=GetFactSetTemp(Addr, dWorkTemp);           //取得要設定至溫控器的溫度
                            }
                        }
                    }
                }

//AI(W906-GA-BTHERMO) 20260807: GATE W7 G12 -- golden :1257 DoCloseHeadterDelay.Off().
//  csystem.h:226 DECLARES `extern TQPF_Timer DoCloseHeadterDelay;` but NOTHING in
//  this tree defines it (golden csystem.cpp:1133 is untranslated) -- a dangling
//  extern, so using it is an undefined reference.  Macro default true is BEHAVIOUR-
//  EXACT for the tree as it stands: golden's only setter is csystem.cpp:1408
//  SetSecAndOn(3), also untranslated, so a default-constructed TQPF_Timer (rEnd==0)
//  answers Off()==true unconditionally (myTimer.cpp:40-44).
                if(BTHERMO_G12_DOCLOSEHEADTERDELAY_OFF &&                                 //jou 2011-12-26 加個delay 3 sec,避免門關不好,一直on/off
                   (Sen[SnHeaterDoor2].IsOff()==true ||
                    Sen[SnHeaterDoor].IsOff()==true))
                {
                    iChangeHotPlate=1;
                    UN150ReadHotplate[0]=0;
                    UN150ReadHotplate[1]=0;
                    iChangeShuttle=1;                                           //kevin 20150508 shuttle
                    UN150ReadShuttle[0]=0;                                      //kevin 20150508 shuttle 分段加熱
                    UN150ReadShuttle[1]=0;                                      //kevin 20150508 shuttle 分段加熱

                    //jou 2010-09-28 Chamber門打開時不關閉加熱電源，只將Temp=0，Hotplate除外。
                    //jou 2010-11-03 新竹矽品shuttle也不要關閉加熱
                    //Steven 20101109 Amkor Shuttle不關閉加熱
                    //ChungHung 2011/12/08 CC_AMKOR Shuttle不關閉加熱
                    //ChungHung 20120719 add DoorOpenHeadContinueHeat
                    if(IniConfig.bDoorOpenShuttleContinueHeat==false && IniConfig.bDoorOpenHeadContinueHeat==true)
                    {
                        if(Addr!=tcHead1 && Addr!=tcHead2 && Addr!=tcHead3 && Addr!=tcHead4 &&
                           Addr!=tcAa1 && Addr!=tcAb1 && Addr!=tcAc1 && Addr!=tcAd1 && Addr!=tcBa1 && Addr!=tcBb1 && Addr!=tcBc1 && Addr!=tcBd1 &&
                           Addr!=tcAa2 && Addr!=tcAb2 && Addr!=tcAc2 && Addr!=tcAd2 && Addr!=tcBa2 && Addr!=tcBb2 && Addr!=tcBc2 && Addr!=tcBd2 &&
                           Addr!=tcAe1 && Addr!=tcAf1 && Addr!=tcAg1 && Addr!=tcAh1 && Addr!=tcBe1 && Addr!=tcBf1 && Addr!=tcBg1 && Addr!=tcBh1 &&
                           Addr!=tcAe2 && Addr!=tcAf2 && Addr!=tcAg2 && Addr!=tcAh2 && Addr!=tcBe2 && Addr!=tcBf2 && Addr!=tcBg2 && Addr!=tcBh2)
                        {
                            Temp=0;
                        }
                    }
                    else if(IniConfig.bDoorOpenShuttleContinueHeat==true && IniConfig.bDoorOpenHeadContinueHeat==false)
                    {
                        //jou 2013-01-08 因為Dut沒有接頭公安問題，加熱又很慢，所以開門繼續加熱，但是會有熱脹冷縮不好Ducking的問題，先暫時這樣改觀察看看。
                        //if(Addr>tcShuttle2 && Addr!=tcSocket)
                        if(Addr>tcShuttle2)                                     //ChungHung 20140331 add
                        {
                            if(iSocketBaseTempCount==eDut1ea && Addr!=tcSocket)
                                Temp=0;
                            else if(iSocketBaseTempCount==eDut2ea && Addr!=tcDUT1 && Addr!=tcDUT2)
                                Temp=0;
                            else if(iSocketBaseTempCount==eDut4ea && Addr!=tcDUT1 && Addr!=tcDUT2 && Addr!=tcDUT3 && Addr!=tcDUT4)
                                Temp=0;
                        }
                    }
                    else if(IniConfig.bDoorOpenShuttleContinueHeat==true && IniConfig.bDoorOpenHeadContinueHeat==true)
                    {
                        if(Addr!=tcHead1 && Addr!=tcHead2 && Addr!=tcHead3 && Addr!=tcHead4 &&
                           Addr!=tcAa1 && Addr!=tcAb1 && Addr!=tcAc1 && Addr!=tcAd1 && Addr!=tcBa1 && Addr!=tcBb1 && Addr!=tcBc1 && Addr!=tcBd1 &&
                           Addr!=tcAa2 && Addr!=tcAb2 && Addr!=tcAc2 && Addr!=tcAd2 && Addr!=tcBa2 && Addr!=tcBb2 && Addr!=tcBc2 && Addr!=tcBd2 &&
                           Addr!=tcAe1 && Addr!=tcAf1 && Addr!=tcAg1 && Addr!=tcAh1 && Addr!=tcBe1 && Addr!=tcBf1 && Addr!=tcBg1 && Addr!=tcBh1 &&
                           Addr!=tcAe2 && Addr!=tcAf2 && Addr!=tcAg2 && Addr!=tcAh2 && Addr!=tcBe2 && Addr!=tcBf2 && Addr!=tcBg2 && Addr!=tcBh2 &&
                           Addr!=tcHotPlate1 && Addr!=tcHotPlate2  && Addr!= tcShuttle1 && Addr!=tcShuttle2                                      )
                        {
                            Temp=0;
                        }
                    }
                    else
                    {
                        //jou 2013-01-08 因為Dut沒有接頭公安問題，加熱又很慢，所以開門繼續加熱，但是會有熱脹冷縮不好Ducking的問題，先暫時這樣改觀察看看。
                        if(IniConfig.bL19OpenHeatDoorgiveupchamberhot)          //kevin 20170520 (wei) 開chambo門只有不加熱chambo
                        {
                            if(Addr>tcShuttle2)                                 //ChungHung 20140331 add
                            {
                                if(iSocketBaseTempCount==eDut1ea && Addr!=tcSocket)
                                {
                                    if(Addr==tcShuttle2)                        //kevin 20151223  開chambo 門shuttle2 不關加熱
                                    {
                                    }
                                    else
                                    {
                                        Temp=0;
                                    }
                                }
                                else if(iSocketBaseTempCount==eDut2ea && Addr!=tcDUT1 && Addr!=tcDUT2)
                                {
                                    Temp=0;
                                }
                                else if(iSocketBaseTempCount==eDut4ea && Addr!=tcDUT1 && Addr!=tcDUT2 && Addr!=tcDUT3 && Addr!=tcDUT4)
                                {
                                    Temp=0;
                                }
                            }
                        }
                        else
                        {
                            if(Addr>tcShuttle1 && INDEXDOORHEATER!=1)           //ChungHung 20140331 add //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                            {
                                if(iSocketBaseTempCount==eDut1ea && Addr!=tcSocket)
                                {
                                    if(CUSTOMER_CODE==CC_ASE_KaohSiung && Addr==tcShuttle2)    //kevin 20151223  開chambo 門shuttle2 不關加熱
                                    {
                                    }
                                    else
                                    {
                                        Temp=0;
                                    }
                                }
                                else if(iSocketBaseTempCount==eDut2ea && Addr!=tcDUT1 && Addr!=tcDUT2)
                                {
                                    Temp=0;
                                }
                                else if(iSocketBaseTempCount==eDut4ea && Addr!=tcDUT1 && Addr!=tcDUT2 && Addr!=tcDUT3 && Addr!=tcDUT4)
                                {
                                    Temp=0;
                                }
                            }
                        }
                    }
                }

                if(bChamboFanCloseTemp && Addr==tcChamber)                      //kevin 20130407關閉chambo 溫度 chambo 溫度過高只關閉chambo溫度
                {
                    Temp=0;
                }

                if(SW[SwHeaterRelay].Status()==false)
                {
                    iChangeHotPlate=1;
                    UN150ReadHotplate[0]=0;
                    UN150ReadHotplate[1]=0;
                    iChangeShuttle=1;                                           //kevin 20150508 shuttle
                    UN150ReadShuttle[0]=0;                                      //kevin 20150508 shuttle 分段加熱
                    UN150ReadShuttle[1]=0;                                      //kevin 20150508 shuttle 分段加熱
                    Temp=0;
                }
            }
            else
            {
                iChangeHotPlate=1;
                UN150ReadHotplate[0]=0;
                UN150ReadHotplate[1]=0;
                iChangeShuttle=1;                                               //kevin 20150508 shuttle
                UN150ReadShuttle[0]=0;                                          //kevin 20150508 shuttle 分段加熱
                UN150ReadShuttle[1]=0;                                          //kevin 20150508 shuttle 分段加熱

                if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true &&
                   (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite) &&
                   ((Addr>=tcAa1 && Addr<=tcBd2) || (Addr>=tcAe1 && Addr<=tcBh2)))    //jou 2012-03-14 增加ATC控制方式
                {
                    Temp=Temperature.fAbitTemp;
                }
                else if(Temperature.bATCActiveCooling==true && (Addr>=tcHeatGun1 && Addr<=tcHeatGun2))  //Ifor 20220124 add: ATC Hot Air
                {
                    Temp=GetFactSetTemp(Addr, Temperature.dATC_HotGunTemp);     //取得要設定至溫控器的溫度
//                    Temp=Temperature.dATC_HotGunTemp;
                }
                else if(Temperature.bATCActiveCooling==true &&
                        (Addr==tcATCHotAir1 || Addr==tcATCHotAir2))             //Ifor 20240730: add for ATC3.5 Hot Air
                {
                    Temp=IniConfig.dHeatGunTempATC;
                }
                else
                {
                    //jou 980318 add from HT9080A
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G14b -- golden :1408 fConfiguration->fShow (same unported form).
//  Macro default false selects golden's own else branch (Temp=0), which is exactly
//  what an unshown Config form produces.
                    if(W906_FormShowing("fConfiguration", false))          //AI(W906-I03) 20261002: GATE W7-UI G14b OPENED -- golden :1408 fConfiguration->fShow (page table, as G14a)
                        Temp=OldTemp[Addr];
                    else
                        Temp=0;
                }
            }
            Com2ReceiveOK=false;

            if(IniConfig.bTemp25degControl==true)                               //jou 2014-06-07 Temperature 25 deg. control
            {
                if((TestIF.iTestMode==_8Site2X4 ||
                    TestIF.iTestMode==_16Site2X8 ||
                    TestIF.iTestMode==_16Site4X4) &&                            //Sam 20190226 : 16Site4X4
                   Temperature.iIndexHeatMode==HeadOnly &&
                   Temperature.fSoakTime==0 &&
                   Temperature.fWorkTemperBase<=25.0)
                {
                    if(Addr==tcHotPlate1 || Addr==tcHotPlate2)
                    {
                        Temp=0;
                    }
                }
            }

            //Steven 20091022 Start
            if(Temperature.iIndexHeatMode==HeadOnly && (CheckIsDUT(Addr) || Addr==tcChamber))    //Head Only (socket & chamber no use) //2013-01-15    Dell DUT(Socket Base)增加為4顆
            {
                Temp=0;
            }
            else if(Temperature.iIndexHeatMode==ChamberOnly)                    //Chamber Only (head 1 2 3 4 & socket no use)
            {
                if((Addr>=tcHead1 && Addr<=tcHead4) ||
                   ((Addr>=tcAa1 && Addr<=tcBd2) ||
                    (Addr>=tcAe1 && Addr<=tcBh2)))                              //Steven 20111012
                    Temp=0;
            }
            else if(Temperature.iIndexHeatMode==HeadChamber && CheckIsDUT(Addr))//Head + Chamber (socket no use) //2013-01-15    Dell DUT(Socket Base)增加為4顆
            {
                Temp=0;
            }
            else if(Temperature.iIndexHeatMode==SocketChamber)                  //Socket + Chamber (head 1 2 3 4 no use)
            {
                if((Addr>=tcHead1 && Addr<=tcHead4) ||
                   ((Addr>=tcAa1 && Addr<=tcBd2) ||
                    (Addr>=tcAe1 && Addr<=tcBh2)))                              //Steven 20111012
                    Temp=0;
            }
            else if(Temperature.iIndexHeatMode==HeadSocket && Addr==tcChamber)
            {
                Temp=0;
            }
            //Steven 20091022 End

            if(TEMPCTRL_NEED_UNDER_20A==true)                                   //Steven 20100707 Start:  中壢日月光要求控制加熱電流在20A以下
            {
                if(Temperature.iIndexHeatMode!=HeadOnly &&                      //只要是用到Chamber，就要等Chamber加熱到達才可以開啟其他的加熱
                   LastSet.iTemperature==Tempture_Hot)
                {
                    if(fHeaterOK==false)
                    {
                        dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcChamber]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                        if(UN150Read[tcChamber]<(dWorkTemp-IniConfig.iL05ChamberTemptureRange) && Temperature.iIndexHeatMode!=HeadSocket)    //只要Chamber溫度未到達設定溫度-5度  //JerryYang 20250120 : modify
                        {
                            if(Addr<=tcHead4)
                                Temp=UN150Read[Addr];                           //Steven 20150921 : Modify for 20A

                            if((Temperature.iIndexHeatMode==HeadChamberSocket || Temperature.iIndexHeatMode==SocketChamber) && iSocketBaseTempCount>eDut1ea)  //JerryYang 20220215 : ASE-CL要求電流控制在20A以下
                            {
                                if(iSocketBaseTempCount==eDut4ea)
                                {
                                    if(Addr>=tcDUT1 && Addr<=tcDUT4)
                                    {
                                        Temp=UN150Read[Addr];
                                    }
                                }
                                else if(iSocketBaseTempCount==eDut2ea)          //Steven 20140116 : Socket數量,改用數字設定
                                {
                                    if(Addr>=tcDUT1 && Addr<=tcDUT2)
                                    {
                                        Temp=UN150Read[Addr];
                                    }
                                }
                            }
                        }
                        else
                        {
                            if((Temperature.iIndexHeatMode==HeadChamberSocket || Temperature.iIndexHeatMode==SocketChamber || Temperature.iIndexHeatMode==HeadSocket) && iSocketBaseTempCount>eDut1ea)  //JerryYang 20250120 : modify
                            {
                                if(iSocketBaseTempCount==eDut4ea)
                                {
                                    dWorkTempDut1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcDUT1]:Temperature.fWorkTemperBase;
                                    dWorkTempDut2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcDUT2]:Temperature.fWorkTemperBase;
                                    dWorkTempDut3=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcDUT3]:Temperature.fWorkTemperBase;
                                    dWorkTempDut4=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcDUT4]:Temperature.fWorkTemperBase;
                                    if(UN150Read[tcDUT1]<(dWorkTempDut1-IniConfig.iL05ChamberTemptureRange) ||
                                       UN150Read[tcDUT2]<(dWorkTempDut2-IniConfig.iL05ChamberTemptureRange) ||
                                       UN150Read[tcDUT3]<(dWorkTempDut3-IniConfig.iL05ChamberTemptureRange) ||
                                       UN150Read[tcDUT4]<(dWorkTempDut4-IniConfig.iL05ChamberTemptureRange))
                                    {
                                        if(Addr<=tcHead4)
                                            Temp=UN150Read[Addr];               //Steven 20150921 : Modify for 20A
                                    }
                                    else
                                    {
                                        if(Temperature.iIndexHeatMode!=HeadOnly &&  //Steven 20100127 : 除Head Only外，做分段
                                           HotPlateForm.iPlateSelect==3 &&          //jou 2010-02-24 : 使用兩塊Hotplate時,做分段
                                           LastSet.iTemperature==Tempture_Hot &&
                                           TEMPCTRL_HOTPLATE_TOGTHER==false)
                                        {
                                            dWorkTempHP1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcHotPlate1]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                            dWorkTempHP2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcHotPlate2]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值

                                            if((UN150Read[tcHotPlate1]>(dWorkTempHP1-2)) || //jou 2010-02-24 start : 階梯式加熱
                                               (UN150Read[tcHotPlate2]>(dWorkTempHP2-2)))
                                            {
                                                iChangeHotPlate=0;
                                            }

                                            if(iChangeHotPlate==1)
                                            {
                                                if(Addr==tcHotPlate1)           // hotplate 1
                                                {
                                                    if(UN150Read[tcHotPlate1]>(UN150Read[tcHotPlate2]+20))
                                                    {
                        //                                Temp=0;               //jou 2013-02-21 make code 避免先將溫度設定為0度，再改變溫度值
                                                        iChangeHotPlate=2;
                                                        UN150ReadHotplate[0]=UN150ReadReal[tcHotPlate1];
                                                        tUNDER_20A_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                    }
                                                    else
                                                    {
                                                        if(tUNDER_20A_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                        {
                                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;
                                                            if(UN150ReadHotplate[0]>GetFactSetTemp(0, dWorkTemp))
                                                            {
                                                                UN150ReadHotplate[0]=0;
                                                            }

                                                            if(UN150ReadHotplate[0] < 0)
                                                            {
                                                                UN150ReadHotplate[0]=0;
                                                            }
                                                            Temp=UN150ReadHotplate[0];
                                                        }
                                                    }
                                                }

                                                if(Addr==tcHotPlate2)
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadHotplate[1]>GetFactSetTemp(1, dWorkTemp))
                                                    {
                                                        UN150ReadHotplate[1]=0;
                                                    }

                                                    if(UN150ReadHotplate[1]<0)
                                                    {
                                                        UN150ReadHotplate[1]=0;
                                                    }
                                                    Temp=UN150ReadHotplate[1];
                                                }
                                            }
                                            else if(iChangeHotPlate==2)
                                            {
                                                if(Addr==tcHotPlate2)           // hotplate 2
                                                {
                                                    if((UN150Read[tcHotPlate1]+20)<UN150Read[tcHotPlate2])
                                                    {
                        //                                Temp=0;               //jou 2013-02-21 make code 避免先將溫度設定為0度，再改變溫度值
                                                        iChangeHotPlate=1;
                                                        UN150ReadHotplate[1]=UN150ReadReal[tcHotPlate2];
                                                        tUNDER_20A_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                    }
                                                    else
                                                    {
                                                        if(tUNDER_20A_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                        {
                                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                            if(UN150ReadHotplate[1]>GetFactSetTemp(1, dWorkTemp))
                                                            {
                                                                UN150ReadHotplate[1]=0;
                                                            }

                                                            if(UN150ReadHotplate[1]<0)
                                                            {
                                                                UN150ReadHotplate[1]=0;
                                                            }
                                                            Temp=UN150ReadHotplate[1];
                                                        }
                                                    }
                                                }

                                                if(Addr==tcHotPlate1)
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadHotplate[0]>GetFactSetTemp(0, dWorkTemp))
                                                    {
                                                        UN150ReadHotplate[0]=0;
                                                    }

                                                    if(UN150ReadHotplate[0]<0)
                                                    {
                                                        UN150ReadHotplate[0]=0;
                                                    }
                                                    Temp=UN150ReadHotplate[0];
                                                }
                                            }
                                        }

                                        if(Temperature.iIndexHeatMode!=HeadOnly &&
                                           TestIF_File.iShuttleMode==0 &&                               //Steven 20150909 : 修正蝦頭分段加熱功能
                                           LastSet.iTemperature==Tempture_Hot)  //kevin 20150508 shuttle不分段加熱
                                        {
                                            dWorkTempHP1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcShuttle1]:Temperature.fWorkTemperBase;          //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                            dWorkTempHP2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcShuttle2]:Temperature.fWorkTemperBase;          //Steven 20140924 : 各個加熱區獨立有自己的設定值

                                            //jou 2010-02-24 start : 階梯式加熱
                                            if((UN150Read[tcShuttle1]>(dWorkTempHP1-2)) ||
                                               (UN150Read[tcShuttle2]>(dWorkTempHP2-2)))
                                            {
                                                iChangeShuttle=0;               //kevin 20150508 shuttle
                                            }

                                            if(iChangeShuttle==1)               //kevin 20150508 shuttle
                                            {
                                                if(Addr==tcShuttle1)            // Shuttle1
                                                {
                                                    if(UN150Read[tcShuttle1]>(UN150Read[tcShuttle2]+20))
                                                    {
                                                        iChangeShuttle=2;
                                                        UN150ReadShuttle[0]=UN150ReadReal[tcShuttle1];
                                                        tUNDER_20A_SHT_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                    }
                                                    else
                                                    {
                                                        if(tUNDER_20A_SHT_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                        {
                                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                            if(UN150ReadShuttle[0]>GetFactSetTemp(0, dWorkTemp))
                                                            {
                                                                UN150ReadShuttle[0]=0;
                                                            }

                                                            if(UN150ReadShuttle[0]<0)
                                                            {
                                                                UN150ReadShuttle[0]=0;
                                                            }
                                                            Temp=UN150ReadShuttle[0];
                                                        }
                                                    }
                                                }

                                                if(Addr==tcShuttle2)
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadShuttle[1]>GetFactSetTemp(1, dWorkTemp))
                                                    {
                                                        UN150ReadShuttle[1]=0;
                                                    }

                                                    if(UN150ReadShuttle[1]<0)
                                                    {
                                                        UN150ReadShuttle[1]=0;
                                                    }
                                                    Temp=UN150ReadShuttle[1];
                                                }
                                            }
                                            else if(iChangeShuttle==2)
                                            {
                                                if(Addr==tcShuttle2)            // tcShuttle 2
                                                {
                                                    if((UN150Read[tcShuttle1]+20)<UN150Read[tcShuttle2])
                                                    {
                                                        iChangeShuttle=1;
                                                        UN150ReadShuttle[1]=UN150ReadReal[tcShuttle2];
                                                        tUNDER_20A_SHT_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                    }
                                                    else
                                                    {
                                                        if(tUNDER_20A_SHT_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                        {
                                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                            if(UN150ReadShuttle[1]>GetFactSetTemp(1, dWorkTemp))
                                                            {
                                                                UN150ReadShuttle[1]=0;
                                                            }

                                                            if(UN150ReadShuttle[1]<0)
                                                            {
                                                                UN150ReadShuttle[1]=0;
                                                            }
                                                            Temp=UN150ReadShuttle[1];
                                                        }
                                                    }
                                                }

                                                if(Addr==tcShuttle1)
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadShuttle[0]>GetFactSetTemp(0, dWorkTemp))
                                                    {
                                                        UN150ReadShuttle[0]=0;
                                                    }

                                                    if(UN150ReadShuttle[0]<0)
                                                    {
                                                        UN150ReadShuttle[0]=0;
                                                    }
                                                    Temp=UN150ReadShuttle[0];
                                                }
                                            }
                                        }
                                    }
                                }
                                else if(iSocketBaseTempCount==eDut2ea)
                                {
                                    dWorkTempDut1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcDUT1]:Temperature.fWorkTemperBase;
                                    dWorkTempDut2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcDUT2]:Temperature.fWorkTemperBase;
                                    if(UN150Read[tcDUT1]<(dWorkTempDut1-IniConfig.iL05ChamberTemptureRange) ||
                                       UN150Read[tcDUT2]<(dWorkTempDut2-IniConfig.iL05ChamberTemptureRange))
                                    {
                                        if(Addr<=tcHead4)
                                            Temp=UN150Read[Addr];               //Steven 20150921 : Modify for 20A
                                    }
                                    else
                                    {
                                        //Steven 20100127 : 除Head Only外，做分段
                                        //jou 2010-02-24 : 使用兩塊Hotplate時,做分段
                                        if(Temperature.iIndexHeatMode!=HeadOnly &&
                                           HotPlateForm.iPlateSelect==3 &&
                                           LastSet.iTemperature==Tempture_Hot &&
                                           TEMPCTRL_HOTPLATE_TOGTHER==false)
                                        {
                                            dWorkTempHP1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcHotPlate1]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                            dWorkTempHP2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcHotPlate2]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值

                                            //jou 2010-02-24 start : 階梯式加熱
                                            if((UN150Read[tcHotPlate1]>(dWorkTempHP1-2)) ||
                                               (UN150Read[tcHotPlate2]>(dWorkTempHP2-2)))
                                            {
                                                iChangeHotPlate=0;
                                            }

                                            if(iChangeHotPlate==1)
                                            {
                                                if(Addr==tcHotPlate1)           // hotplate 1
                                                {
                                                    if(UN150Read[tcHotPlate1]>(UN150Read[tcHotPlate2]+20))
                                                    {
                        //                                Temp=0;               //jou 2013-02-21 make code 避免先將溫度設定為0度，再改變溫度值
                                                        iChangeHotPlate=2;
                                                        UN150ReadHotplate[0]=UN150ReadReal[tcHotPlate1];
                                                        tUNDER_20A_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                    }
                                                    else
                                                    {
                                                        if(tUNDER_20A_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                        {
                                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                            if(UN150ReadHotplate[0]>GetFactSetTemp(0, dWorkTemp))
                                                            {
                                                                UN150ReadHotplate[0]=0;
                                                            }

                                                            if(UN150ReadHotplate[0] < 0)
                                                            {
                                                                UN150ReadHotplate[0]=0;
                                                            }
                                                            Temp=UN150ReadHotplate[0];
                                                        }
                                                    }
                                                }

                                                if(Addr==tcHotPlate2)
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadHotplate[1]>GetFactSetTemp(1, dWorkTemp))
                                                    {
                                                        UN150ReadHotplate[1]=0;
                                                    }

                                                    if(UN150ReadHotplate[1]<0)
                                                    {
                                                        UN150ReadHotplate[1]=0;
                                                    }
                                                    Temp=UN150ReadHotplate[1];
                                                }
                                            }
                                            else if(iChangeHotPlate==2)
                                            {
                                                if(Addr==tcHotPlate2)           // hotplate 2
                                                {
                                                    if((UN150Read[tcHotPlate1]+20)<UN150Read[tcHotPlate2])
                                                    {
                        //                                Temp=0;               //jou 2013-02-21 make code 避免先將溫度設定為0度，再改變溫度值
                                                        iChangeHotPlate=1;
                                                        UN150ReadHotplate[1]=UN150ReadReal[tcHotPlate2];
                                                        tUNDER_20A_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                    }
                                                    else
                                                    {
                                                        if(tUNDER_20A_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                        {
                                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                            if(UN150ReadHotplate[1]>GetFactSetTemp(1, dWorkTemp))
                                                            {
                                                                UN150ReadHotplate[1]=0;
                                                            }

                                                            if(UN150ReadHotplate[1]<0)
                                                            {
                                                                UN150ReadHotplate[1]=0;
                                                            }
                                                            Temp=UN150ReadHotplate[1];
                                                        }
                                                    }
                                                }

                                                if(Addr==tcHotPlate1)
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadHotplate[0]>GetFactSetTemp(0, dWorkTemp))
                                                    {
                                                        UN150ReadHotplate[0]=0;
                                                    }

                                                    if(UN150ReadHotplate[0] < 0)
                                                    {
                                                        UN150ReadHotplate[0]=0;
                                                    }
                                                    Temp=UN150ReadHotplate[0];
                                                }
                                            }
                                            //jou 2010-02-24 end
                                        }

                                        if(Temperature.iIndexHeatMode!=HeadOnly &&
                                           TestIF_File.iShuttleMode==0 &&                               //Steven 20150909 : 修正蝦頭分段加熱功能
                                           LastSet.iTemperature==Tempture_Hot)  //kevin 20150508 shuttle不分段加熱
                                        {
                                            dWorkTempHP1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcShuttle1]:Temperature.fWorkTemperBase;          //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                            dWorkTempHP2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcShuttle2]:Temperature.fWorkTemperBase;          //Steven 20140924 : 各個加熱區獨立有自己的設定值

                                            //jou 2010-02-24 start : 階梯式加熱
                                            if((UN150Read[tcShuttle1]>(dWorkTempHP1-2)) ||
                                               (UN150Read[tcShuttle2]>(dWorkTempHP2-2)))
                                            {
                                                iChangeShuttle=0;               //kevin 20150508 shuttle
                                            }

                                            if(iChangeShuttle==1)               //kevin 20150508 shuttle
                                            {
                                                if(Addr==tcShuttle1)            // Shuttle1
                                                {
                                                    if(UN150Read[tcShuttle1]>(UN150Read[tcShuttle2]+20))
                                                    {
                                                        iChangeShuttle=2;
                                                        UN150ReadShuttle[0]=UN150ReadReal[tcShuttle1];
                                                        tUNDER_20A_SHT_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                    }
                                                    else
                                                    {
                                                        if(tUNDER_20A_SHT_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                        {
                                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                            if(UN150ReadShuttle[0]>GetFactSetTemp(0, dWorkTemp))
                                                            {
                                                                UN150ReadShuttle[0]=0;
                                                            }

                                                            if(UN150ReadShuttle[0]<0)
                                                            {
                                                                UN150ReadShuttle[0]=0;
                                                            }
                                                            Temp=UN150ReadShuttle[0];
                                                        }
                                                    }
                                                }

                                                if(Addr==tcShuttle2)
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadShuttle[1]>GetFactSetTemp(1, dWorkTemp))
                                                    {
                                                        UN150ReadShuttle[1]=0;
                                                    }

                                                    if(UN150ReadShuttle[1]<0)
                                                    {
                                                        UN150ReadShuttle[1]=0;
                                                    }
                                                    Temp=UN150ReadShuttle[1];
                                                }
                                            }
                                            else if(iChangeShuttle==2)
                                            {
                                                if(Addr==tcShuttle2)            // tcShuttle 2
                                                {
                                                    if((UN150Read[tcShuttle1]+20)<UN150Read[tcShuttle2])
                                                    {
                                                        iChangeShuttle=1;
                                                        UN150ReadShuttle[1]=UN150ReadReal[tcShuttle2];
                                                        tUNDER_20A_SHT_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                    }
                                                    else
                                                    {
                                                        if(tUNDER_20A_SHT_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                        {
                                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                            if(UN150ReadShuttle[1]>GetFactSetTemp(1, dWorkTemp))
                                                            {
                                                                UN150ReadShuttle[1]=0;
                                                            }

                                                            if(UN150ReadShuttle[1]<0)
                                                            {
                                                                UN150ReadShuttle[1]=0;
                                                            }
                                                            Temp=UN150ReadShuttle[1];
                                                        }
                                                    }
                                                }

                                                if(Addr==tcShuttle1)
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadShuttle[0]>GetFactSetTemp(0, dWorkTemp))
                                                    {
                                                        UN150ReadShuttle[0]=0;
                                                    }

                                                    if(UN150ReadShuttle[0]<0)
                                                    {
                                                        UN150ReadShuttle[0]=0;
                                                    }
                                                    Temp=UN150ReadShuttle[0];
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                //Steven 20100127 : 除Head Only外，做分段
                                //jou 2010-02-24 : 使用兩塊Hotplate時,做分段
                                if(Temperature.iIndexHeatMode!=HeadOnly &&
                                   HotPlateForm.iPlateSelect==3 &&
                                   LastSet.iTemperature==Tempture_Hot &&
                                   TEMPCTRL_HOTPLATE_TOGTHER==false)
                                {
                                    dWorkTempHP1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcHotPlate1]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                    dWorkTempHP2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcHotPlate2]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值

                                    //jou 2010-02-24 start : 階梯式加熱
                                    if((UN150Read[tcHotPlate1]>(dWorkTempHP1-2)) ||
                                       (UN150Read[tcHotPlate2]>(dWorkTempHP2-2)))
                                    {
                                        iChangeHotPlate=0;
                                    }

                                    if(iChangeHotPlate==1)
                                    {
                                        if(Addr==tcHotPlate1)                   // hotplate 1
                                        {
                                            if(UN150Read[tcHotPlate1]>(UN150Read[tcHotPlate2]+20))
                                            {
                //                                Temp=0;                       //jou 2013-02-21 make code 避免先將溫度設定為0度，再改變溫度值
                                                iChangeHotPlate=2;
                                                UN150ReadHotplate[0]=UN150ReadReal[tcHotPlate1];
                                                tUNDER_20A_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                            }
                                            else
                                            {
                                                if(tUNDER_20A_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadHotplate[0]>GetFactSetTemp(0, dWorkTemp))
                                                    {
                                                        UN150ReadHotplate[0]=0;
                                                    }

                                                    if(UN150ReadHotplate[0] < 0)
                                                    {
                                                        UN150ReadHotplate[0]=0;
                                                    }
                                                    Temp=UN150ReadHotplate[0];
                                                }
                                            }
                                        }

                                        if(Addr==tcHotPlate2)
                                        {
                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                            if(UN150ReadHotplate[1]>GetFactSetTemp(1, dWorkTemp))
                                            {
                                                UN150ReadHotplate[1]=0;
                                            }

                                            if(UN150ReadHotplate[1]<0)
                                            {
                                                UN150ReadHotplate[1]=0;
                                            }
                                            Temp=UN150ReadHotplate[1];
                                        }
                                    }
                                    else if(iChangeHotPlate==2)
                                    {
                                        if(Addr==tcHotPlate2)                   // hotplate 2
                                        {
                                            if((UN150Read[tcHotPlate1]+20)<UN150Read[tcHotPlate2])
                                            {
                //                                Temp=0;                       //jou 2013-02-21 make code 避免先將溫度設定為0度，再改變溫度值
                                                iChangeHotPlate=1;
                                                UN150ReadHotplate[1]=UN150ReadReal[tcHotPlate2];
                                                tUNDER_20A_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                            }
                                            else
                                            {
                                                if(tUNDER_20A_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadHotplate[1]>GetFactSetTemp(1, dWorkTemp))
                                                    {
                                                        UN150ReadHotplate[1]=0;
                                                    }

                                                    if(UN150ReadHotplate[1]<0)
                                                    {
                                                        UN150ReadHotplate[1]=0;
                                                    }
                                                    Temp=UN150ReadHotplate[1];
                                                }
                                            }
                                        }

                                        if(Addr==tcHotPlate1)
                                        {
                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                            if(UN150ReadHotplate[0]>GetFactSetTemp(0, dWorkTemp))
                                            {
                                                UN150ReadHotplate[0]=0;
                                            }

                                            if(UN150ReadHotplate[0]<0)
                                            {
                                                UN150ReadHotplate[0]=0;
                                            }
                                            Temp=UN150ReadHotplate[0];
                                        }
                                    }
                                    //jou 2010-02-24 end
                                }

                                if(Temperature.iIndexHeatMode!=HeadOnly &&      //JerryYang 20220215 : ASE-CL要求電流控制在20A以下
                                   TestIF_File.iShuttleMode==0 &&               //Steven 20150909 : 修正蝦頭分段加熱功能
                                   LastSet.iTemperature==Tempture_Hot)          //kevin 20150508 shuttle不分段加熱
                                {
                                    dWorkTempHP1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcShuttle1]:Temperature.fWorkTemperBase;          //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                    dWorkTempHP2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcShuttle2]:Temperature.fWorkTemperBase;          //Steven 20140924 : 各個加熱區獨立有自己的設定值

                                    //jou 2010-02-24 start : 階梯式加熱
                                    if((UN150Read[tcShuttle1]>(dWorkTempHP1-2)) ||
                                       (UN150Read[tcShuttle2]>(dWorkTempHP2-2)))
                                    {
                                        iChangeShuttle=0;                       //kevin 20150508 shuttle
                                    }

                                    if(iChangeShuttle==1)                       //kevin 20150508 shuttle
                                    {
                                        if(Addr==tcShuttle1)                    // Shuttle1
                                        {
                                            if(UN150Read[tcShuttle1]>(UN150Read[tcShuttle2]+20))
                                            {
                                                iChangeShuttle=2;
                                                UN150ReadShuttle[0]=UN150ReadReal[tcShuttle1];
                                                tUNDER_20A_SHT_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                            }
                                            else
                                            {
                                                if(tUNDER_20A_SHT_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadShuttle[0]>GetFactSetTemp(0, dWorkTemp))
                                                    {
                                                        UN150ReadShuttle[0]=0;
                                                    }

                                                    if(UN150ReadShuttle[0]<0)
                                                    {
                                                        UN150ReadShuttle[0]=0;
                                                    }
                                                    Temp=UN150ReadShuttle[0];
                                                 }
                                            }
                                        }

                                        if(Addr==tcShuttle2)
                                        {
                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                            if(UN150ReadShuttle[1]>GetFactSetTemp(1, dWorkTemp))
                                            {
                                                UN150ReadShuttle[1]=0;
                                            }

                                            if(UN150ReadShuttle[1]<0)
                                            {
                                                UN150ReadShuttle[1]=0;
                                            }
                                            Temp=UN150ReadShuttle[1];
                                        }
                                    }
                                    else if(iChangeShuttle==2)
                                    {
                                        if(Addr==tcShuttle2)                    // tcShuttle 2
                                        {
                                            if((UN150Read[tcShuttle1]+20)<UN150Read[tcShuttle2])
                                            {
                                                iChangeShuttle=1;
                                                UN150ReadShuttle[1]=UN150ReadReal[tcShuttle2];
                                                tUNDER_20A_SHT_Delay.SetSecAndOn(iUNDER_20A_Delay_Sec);   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                            }
                                            else
                                            {
                                                if(tUNDER_20A_SHT_Delay.Off()==false)     //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
                                                {
                                                    dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                                    if(UN150ReadShuttle[1]>GetFactSetTemp(1, dWorkTemp))
                                                    {
                                                        UN150ReadShuttle[1]=0;
                                                    }

                                                    if(UN150ReadShuttle[1]<0)
                                                    {
                                                        UN150ReadShuttle[1]=0;
                                                    }
                                                    Temp=UN150ReadShuttle[1];
                                                }
                                            }
                                        }

                                        if(Addr==tcShuttle1)
                                        {
                                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                                            if(UN150ReadShuttle[0]>GetFactSetTemp(0, dWorkTemp))
                                            {
                                                UN150ReadShuttle[0]=0;
                                            }

                                            if(UN150ReadShuttle[0]<0)
                                            {
                                                UN150ReadShuttle[0]=0;
                                            }
                                            Temp=UN150ReadShuttle[0];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if(Temperature.iIndexHeatMode!=HeadOnly &&                      //Steven 20100127 : 除Head Only外，做分段
                   HotPlateForm.iPlateSelect==3 &&                              //jou 2010-02-24 : 使用兩塊Hotplate時,做分段
                   LastSet.iTemperature==Tempture_Hot &&
                   TEMPCTRL_HOTPLATE_TOGTHER==false)
                {
                    dWorkTempHP1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcHotPlate1]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                    dWorkTempHP2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcHotPlate2]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                    dWorkTempChamber=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcChamber]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值

                    if((UN150Read[tcHotPlate1]>(dWorkTempHP1-2)) ||             //jou 2010-02-24 start : 階梯式加熱
                       (UN150Read[tcHotPlate2]>(dWorkTempHP2-2)) ||
                       (UN150Read[tcChamber]>(dWorkTempChamber-2)))
                    {
                        iChangeHotPlate=0;
                    }

                    if(iChangeHotPlate==1)
                    {
                        if(Addr==tcHotPlate1)                                   // hotplate 1
                        {
                            if(UN150Read[tcHotPlate1]>(UN150Read[tcHotPlate2]+20))
                            {
//                                Temp=0;                                       //jou 2013-02-21 make code 避免先將溫度設定為0度，再改變溫度值
                                iChangeHotPlate=2;
                                UN150ReadHotplate[0]=UN150ReadReal[tcHotPlate1];
                            }
                        }

                        if(Addr==tcHotPlate2)
                        {
                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                            if(UN150ReadHotplate[1]>GetFactSetTemp(1, dWorkTemp))
                            {
                                UN150ReadHotplate[1]=0;
                            }

                            if(UN150ReadHotplate[1]<0)
                            {
                                UN150ReadHotplate[1]=0;
                            }
                            Temp=UN150ReadHotplate[1];
                        }
                    }
                    else if(iChangeHotPlate==2)
                    {
                        if(Addr==tcHotPlate2)                                   // hotplate 2
                        {
                            if((UN150Read[tcHotPlate1]+20)<UN150Read[tcHotPlate2])
                            {
//                                Temp=0;                                       //jou 2013-02-21 make code 避免先將溫度設定為0度，再改變溫度值
                                iChangeHotPlate=1;
                                UN150ReadHotplate[1]=UN150ReadReal[tcHotPlate2];
                            }
                        }

                        if(Addr==tcHotPlate1)
                        {
                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                            if(UN150ReadHotplate[0]>GetFactSetTemp(0, dWorkTemp))
                            {
                                UN150ReadHotplate[0]=0;
                            }

                            if(UN150ReadHotplate[0]<0)
                            {
                                UN150ReadHotplate[0]=0;
                            }
                            Temp=UN150ReadHotplate[0];
                        }
                    }
                }

                if(Temperature.iIndexHeatMode!=HeadOnly &&
                   TestIF_File.iShuttleMode==0 &&                               //Steven 20150909 : 修正蝦頭分段加熱功能
                   LastSet.iTemperature==Tempture_Hot &&                        //kevin 20150508 add shuttle  加熱 分段
                   bTEMPCTRL_Shuttle_TOGTHER==false)                            //kevin 20150508 shuttle不分段加熱
                {
                    dWorkTempHP1=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcShuttle1]:Temperature.fWorkTemperBase;          //Steven 20140924 : 各個加熱區獨立有自己的設定值
                    dWorkTempHP2=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcShuttle2]:Temperature.fWorkTemperBase;          //Steven 20140924 : 各個加熱區獨立有自己的設定值
                    dWorkTempChamber=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[tcChamber]:Temperature.fWorkTemperBase;       //Steven 20140924 : 各個加熱區獨立有自己的設定值

                    if((UN150Read[tcShuttle1]>(dWorkTempHP1-2)) ||              //jou 2010-02-24 start : 階梯式加熱
                       (UN150Read[tcShuttle2]>(dWorkTempHP2-2)) ||
                       (UN150Read[tcChamber]>(dWorkTempChamber-2)))
                    {
                        iChangeShuttle=0;                                       //kevin 20150508 shuttle
                    }

                    if(iChangeShuttle==1)                                       //kevin 20150508 shuttle
                    {
                        if(Addr==tcShuttle1)                                    // Shuttle1
                        {
                            if(UN150Read[tcShuttle1]>(UN150Read[tcShuttle2]+20))
                            {
                                iChangeShuttle=2;
                                UN150ReadShuttle[0]=UN150ReadReal[tcShuttle1];
                            }
                        }

                        if(Addr==tcShuttle2)
                        {
                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                            if(UN150ReadShuttle[1]>GetFactSetTemp(1, dWorkTemp))
                            {
                                UN150ReadShuttle[1]=0;
                            }

                            if(UN150ReadShuttle[1]<0)
                            {
                                UN150ReadShuttle[1]=0;
                            }
                            Temp=UN150ReadShuttle[1];
                        }
                    }
                    else if(iChangeShuttle==2)
                    {
                        if(Addr==tcShuttle2)                                    // tcShuttle 2
                        {
                            if((UN150Read[tcShuttle1]+20)<UN150Read[tcShuttle2])
                            {
                                iChangeShuttle=1;
                                UN150ReadShuttle[1]=UN150ReadReal[tcShuttle2];
                            }
                        }

                        if(Addr==tcShuttle1)
                        {
                            dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                            if(UN150ReadShuttle[0]>GetFactSetTemp(0, dWorkTemp))
                            {
                                UN150ReadShuttle[0]=0;
                            }

                            if(UN150ReadShuttle[0]<0)
                            {
                                UN150ReadShuttle[0]=0;
                            }
                            Temp=UN150ReadShuttle[0];
                        }
                    }
                }
            }

            if(Temp!=OldTemp[Addr] || (ReceiveErr==true && WriteCommand!=-1))
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W7 G19a -- golden :2318-2332.
//  FormHS->CheckTempOffset(): golden HS_Function.h:93 (TFormHS *FormHS).  The only
//  FormHS in this tree is Automation/SCK_ART_Remainder.h's TU-local stand-in, whose
//  entire surface is UpDataToServerByFTP (see its own gate #17 note) -- no
//  CheckTempOffset -- and HS_Function.{h,cpp} is unported.  Cannot compile.
//  ACTIVE DEFAULT: the over-machine-limit clamp is skipped, i.e. golden's normal path: Temp keeps the
//  value GetFactSetTemp produced.  bHasErrorSet / tThermoAlarmTimer keep their
//  declarations so the gate can be retired by deleting the #if 0 alone.
#if 0 // TODO(W7 G19a): golden :2318-2332
                if(FormHS->CheckTempOffset(Addr, Temp))                         //設定溫度大於機台能力  //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20
                {
                    if(bHasErrorSet==false)                                     //判斷有無Alarm過
                    {
                        FormHS->CheckTempOffset(0, 0, true);                    //Alarm 溫度設定錯誤
                        tThermoAlarmTimer.SetSecAndOn(20);
                        bHasErrorSet=true;                                      //設定Alarm旗標
                    }
                    else
                    {
                        if(tThermoAlarmTimer.Off())
                            bHasErrorSet=false;                                 //取消Alarm旗標
                    }
                    Temp=Temperature.fWorkTemperBase;                           //設定溫度超過機台極限改寫工作溫度給溫控器(不包含Offset)
                }
#endif // TODO(W7 G19a)

//AI(W906-GA-BTHERMO) 20260807: GATE W7 G20 -- golden :2334.
//  HeaterSVLog(int,double) is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:707 sits inside that file's `#if 0` block -- declared-but-never-
//  defined, so calling it is an undefined reference.  Verified by walking
//  cpublic.cpp's #if/#else/#endif nesting, not by eye.
//  ACTIVE DEFAULT: the SV change is not journalled; OldTemp[Addr]=Temp on the next line still runs.
//AI(W906-I03) 20261002 (Ifor01): GATE W7 G20 OPENED (golden :2334; the 20260807 note above is history): HeaterSVLog is defined now (cpublic.cpp, through W906_HeaterLogHook)
                HeaterSVLog(Addr, Temp);
//AI(W906-I03) 20261002: (the old #endif of G20)
                OldTemp[Addr]=Temp;                                             // write setup temperature
                WriteCommand=0x0001;
                if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true &&
                   (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite) &&
                   ((Addr>=tcAa1 && Addr<=tcBd2) || (Addr>=tcAe1 && Addr<=tcBh2)))    //jou 2012-03-14 增加ATC控制方式
                {
//                    if(bUT150Install[Addr]==true && bATCInitialFinish==true)
//                          COM2->ATCFuntion_WriteSP1(Addr,Temp);
                }
                else if(TC401HeaterControl==TC401)
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G21a -- golden :2346.
//  TMC401WriteTemp() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:582 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: no SV is pushed to a TC401 controller (there is none offline).
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G21a OPENED (golden :2346; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                    TMC401WriteTemp(Addr/4, Addr%4, Temp*10);                   //送溫度到溫控器
//AI(W906-I03) 20261002: (the old #endif of G21a)
                }
                else if(TC401HeaterControl==KT4H)
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G22 -- golden :2350.
//  UT100WordWriteNoSucm() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:326 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: the KT4H SV push is skipped; all surrounding SM bookkeeping stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G22 OPENED (golden :2350; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                    UT100WordWriteNoSucm(Addr, WriteCommand, Temp*10);          //送溫度到溫控器
//AI(W906-I03) 20261002: (the old #endif of G22)
                }
                else if(TC401HeaterControl==E5DC)                               //Steven 20141030 : 新增OMRON E5DC溫控器
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G23a -- golden :2354.
//  E5DCWriteTemp() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:640 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: no SV is pushed to an E5DC controller.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G23a OPENED (golden :2354; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                    E5DCWriteTemp(Addr, Temp*10);                                //Frank20141030
//AI(W906-I03) 20261002: (the old #endif of G23a)
                }
                else if(TC401HeaterControl==DTK4848)                            //KaiHuang 20190821 : 新增台達 DTK4848溫控器
                {
                    DTK4848WordWriteNoSucm(Addr, Temp*10);                      //送溫度到溫控器
                }

                ClearHotBuffer(Addr);
            }
            else if(bUT150Install[Addr]==false)
            {
                Task=300;
                break;
            }
            else
            {
                ReadCommand=0x0080;
                if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true &&
                   (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite) &&
                   ((Addr>=tcAa1 && Addr<=tcBd2) || (Addr>=tcAe1 && Addr<=tcBh2)))    //jou 2012-03-14 增加ATC控制方式
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G16 -- golden :2375-2376.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no ATCFuntion_Read().  Its guarding `if` must go with it or it dangles.
//  ACTIVE DEFAULT: no ATC read is issued; the SM still falls through to the Task=260/300 choice.
#if 0 // TODO(W5 G16): golden :2375-2376
                    if(bUT150Install[Addr]==true && bATCInitialFinish==true)
                        COM2->ATCFuntion_Read(Addr);
#endif // TODO(W5 G16)
                }
                else if(TC401HeaterControl==TC401)
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G21b -- golden :2380.
//  TMC401ReadTemp() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:603 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: no PV read is issued; case 200 then times out through Com2Delay exactly as golden.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G21b OPENED (golden :2380; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                    TMC401ReadTemp(Addr/4, Addr%4);
//AI(W906-I03) 20261002: (the old #endif of G21b)
                }
                else if(TC401HeaterControl==KT4H)
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G22 -- golden :2384.
//  UT100WordReadNoSucm() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:339 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: the KT4H PV read is skipped; all surrounding SM bookkeeping stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G22 OPENED (golden :2384; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                    UT100WordReadNoSucm(Addr, ReadCommand);
//AI(W906-I03) 20261002: (the old #endif of G22)
                }
                else if(TC401HeaterControl==E5DC)                               //Steven 20141030 : 新增OMRON E5DC溫控器
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G23b -- golden :2388.
//  E5DCReadTemp() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:623 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: no PV read is issued; case 255 then times out through Com2Delay exactly as golden.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G23b OPENED (golden :2388; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                    E5DCReadTemp(Addr);
//AI(W906-I03) 20261002: (the old #endif of G23b)
                }
                else if(TC401HeaterControl==DTK4848)                            //KaiHuang 20190821 : 新增台達 DTK4848溫控器
                {
                    DTK4848WordReadNoSucm(Addr);
                }
            }

            Com2Delay.SetSecAndOn(0.5);

            if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true &&
               (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite) &&
               ((Addr>=tcAa1 && Addr<=tcBd2) || (Addr>=tcAe1 && Addr<=tcBh2)))   //jou 2012-03-14 增加ATC控制方式
            {
                if(bUT150Install[Addr]==true && bATCInitialFinish==true)
                    Task=260;
                else
                    Task=300;
            }
            else if(TC401HeaterControl==TC401)
            {
                Task=200;
            }
            else if(TC401HeaterControl==KT4H)
            {
                Task=250;
            }
            else if(TC401HeaterControl==E5DC)                                   //Steven 20141030 : 新增OMRON E5DC溫控器
            {
                Task=255;
            }
            else if(TC401HeaterControl==DTK4848)                                //KaiHuang 20190821 : 新增台達 DTK4848溫控器
            {
                Task=2500;
            }
            break;
//TC401 start
          case 200:
            if(Com2ReceiveOK)
            {
                UN150CommError[Addr]=false;
                if(ReadCommand!=0x0080)                                         // is not read current templeate
                {
                    Task=300;
                    return;
                }

                if(READBUFF[1]!=0x03)                                           // not correct format
                {
                    Task=100;
                    ReceiveErr=true;
                    if(CommRetry[Addr] > 0 && CommRetry[Addr]<=MAX_RETRY)
                    {
                        CommRetry[Addr]--;
                        Task=300;
                    }
                    else
                    {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2446.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StopComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port reset is skipped; every other statement of the retry/error
//  bookkeeping around it (Com2Delay / CommunCTErr / UN150CommError / Task=230)
//  stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2446; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                        { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StopComm(); }   //AI(W906-I03) 20261002: golden COM2->Comm2->StopComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                        Com2Delay.SetSecAndOn(0.5);
                        CommunCTErr[Addr]++;                                    //JerryYang 20170929 (wei) CommunCTErr改成陣列
                        if(CommunCTErr[Addr]>5)
                        {
                            UN150CommError[Addr]=true;
                            CommRetry[Addr]=MAX_RETRY;
                            UN150Read[Addr]=999;
                        }
                        Task=230;
                    }
                    break;
                }
                CommRetry[Addr]=MAX_RETRY;
                ReceiveErr=false;
                pH=READBUFF[3];
                pL=READBUFF[4];
                P=pH*256+pL;
                Read=P/10.0;
                //----- by dell ccd realtime-------------Steven 20110811
                if(Addr==tcCCD || Addr==tc2D || Addr==tcLB ||                   //wei 20160524 2D溫度
                   (RTC_TemperNumber==2 && Addr==tcCCD_2))                      //Isaac 20201217 : RTC CCD增加第二組感溫
                    UN150Read[Addr]=Read;
                else
                //---------------------------------------
                    UN150Read[Addr]=GetConvertTemp(Addr, Read);                 //實際溫控傳回溫度
                CommunCTErr[Addr]=0;                                            //JerryYang 20170929 (wei) CommunCTErr改成陣列
                Task=300;
                break;
            }

            if(Com2Delay.Off())
            {
                if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                {
                    CommRetry[Addr]--;
                    Task=300;
                }
                else
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2486.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StopComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port reset is skipped; every other statement of the retry/error
//  bookkeeping around it (Com2Delay / CommunCTErr / UN150CommError / Task=230)
//  stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2486; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                    { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StopComm(); }   //AI(W906-I03) 20261002: golden COM2->Comm2->StopComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                    Com2Delay.SetSecAndOn(0.5);
                    CommunCTErr[Addr]++;                                        //JerryYang 20170929 (wei) CommunCTErr改成陣列
                    if(CommunCTErr[Addr]>5)
                    {
                        UN150CommError[Addr]=true;
                        CommRetry[Addr]=MAX_RETRY;
                        UN150Read[Addr]=999;
                    }
                    Task=230;
                    break;
                }
            }
            break;
        case 230:
            if(Com2Delay.Off())
            {
                #ifndef SOFT_SIMULTE
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2504.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StartComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port restart is skipped; Task still advances to 300.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2504; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StartComm(); }   //AI(W906-I03) 20261002: golden COM2->Comm2->StartComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                #endif
                Task=300;
            }
            break;
//TC401 end
//KT4H start
        case 250:
            if(Com2ReceiveOK)
            {
                UN150CommError[Addr]=false;
                pRead=(char *)Com2Buffer.c_str();                               //Steven 20111028 : 改成AnsiString
//AI(W906-GA-BTHERMO) 20260807: READBUFF is `unsigned char[1024]` (port cmydef.h:3172, identical to golden
//  cmydef.h:3159).  BCB6 accepted `strncpy(READBUFF, ...)`; ISO C++/MinGW will not
//  convert unsigned char* to char*, so an explicit cast is added.  Byte-for-byte
//  identical behaviour -- no reinterpretation, no length change.
                ::strncpy((char *)READBUFF, pRead, sizeof(READBUFF));

                if(A_Check_LRC()==ERROR_CODE)                                   //check LRC CODE
                {
                    ClearSendBuff();
                    Task=100;
                    ReceiveErr=true;
                    if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                    {
                        CommRetry[Addr]--;
                        Task=300;
                    }
                    else
                    {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2530.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StopComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port reset is skipped; every other statement of the retry/error
//  bookkeeping around it (Com2Delay / CommunCTErr / UN150CommError / Task=230)
//  stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2530; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                        { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StopComm(); }   //JerryYang 20170925 (wei) COM port Reset   //AI(W906-I03) 20261002: golden COM2->Comm2->StopComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                        Com2Delay.SetSecAndOn(0.5);
                        CommunCTErr[Addr]++;
                        if(CommunCTErr[Addr]>5)
                        {
                            UN150CommError[Addr]=true;
                            CommRetry[Addr]=MAX_RETRY;
                            UN150Read[Addr]=999;
                        }
                        Task=230;
                    }
                    break;
                }
                CommRetry[Addr]=MAX_RETRY;
                ReceiveErr=false;
                if(A_Get_Function_Code()!=3)
                {
                    Task=300;
                    break;
                }
                Read=A_Get_MEM_Word()/10.0;                                     //溫控器傳回的溫度
                UN150ReadReal[Addr]=Read;
                if(Addr==tcLB ||                                                //Steven 20190924 : Add for ATC6.0
                   Addr==tcLBUp || Addr==tcLBDown)                              //Frank 20241231 : add
                {
                    UN150Read[Addr]=Read;                                       //Steven 20110924
                }
                else if(Addr==tcCCD ||                                          //----- by dell ccd realtime-------------Steven 20110811
                        Addr==tc2D ||                                           //wei 20160524 2D溫度
                        (RTC_TemperNumber==2 && Addr==tcCCD_2))                 //Isaac 20201217 : RTC CCD增加第二組感溫
                {
                    dCCDTemperature=Read;
                    UN150Read[Addr]=ReadAverageBuffer(Addr, Read);              //Steven 20110924
                }
                else
                {
                    if(LastSet.iTemperature!=Tempture_Hot)                      //Steven 20240606 : SHUTTLE_COOLING for Ambient Control
                    {
                        UN150Read[Addr]=GetConvertTemp(Addr, Read)-SHUTTLE_COOLING;
                    }
                    else
                    {
                        UN150Read[Addr]=ReadAverageBuffer(Addr, GetConvertTemp(Addr, Read));
                    }
                }

                ClearSendBuff();
                CommunCTErr[Addr]=0;                                            //JerryYang 20170925 (wei) COM port Reset
                if(LastSet.iTemperature==Tempture_Hot &&
                   IniConfig.bEnableKT4HAlarm1)                                 //Steven 20120809 : KT4H使用Alarm1作加熱保護
                {
                    if(Addr==tcCCD ||
                       (RTC_TemperNumber==2 && Addr==tcCCD_2) ||                //Isaac 20201217 : RTC CCD增加第二組感溫
                       Addr==tcHeatGun1 ||
                       Addr==tcHeatGun2 ||
                       Addr==tc2D ||                                            //wei 20160524 2D溫度
                       Addr==tcLB ||
                       Addr==tcLBUp || Addr==tcLBDown)                          //Frank 20241231 : add
                    {
                        Task=300;
                    }
                    else
                    {
                        Task=500;
                    }
                }
                else
                {
                    Task=300;
                }
                break;
            }

            if(Com2Delay.Off())
            {
                if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                {
                    CommRetry[Addr]--;
                    Task=300;
                }
                else
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2612.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StopComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port reset is skipped; every other statement of the retry/error
//  bookkeeping around it (Com2Delay / CommunCTErr / UN150CommError / Task=230)
//  stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2612; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                    { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StopComm(); }   //JerryYang 20170925 (wei) COM port Reset   //AI(W906-I03) 20261002: golden COM2->Comm2->StopComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                    Com2Delay.SetSecAndOn(0.5);
                    CommunCTErr[Addr]++;
                    if(CommunCTErr[Addr]>5)
                    {
                        UN150CommError[Addr]=true;
                        CommRetry[Addr]=MAX_RETRY;
                        UN150Read[Addr]=999;
                    }
                    Task=230;
                    break;
                }
            }
            break;
//KT4H end
//E5DC start
        case 255:                                                               //Frank20141028       新增E5DC
            if(Com2ReceiveOK)
            {
//{ "\x02 03 00 00 0101 0000 0000010B 01007002\x03w" }
//      1 23 45 67 8901 2345 67890123 456789012   3
//                   1           2          3
//  [STX] [ADDR]   [MRC SRC] [Para1]  [Para2]
//           [Sub-Addr] [Resp Code]
//              [End Code]

                UN150CommError[Addr]=false;
                pRead=(char *)Com2Buffer.c_str();                               //Steven 20111028 : 改成AnsiString
//AI(W906-GA-BTHERMO) 20260807: READBUFF is `unsigned char[1024]` (port cmydef.h:3172, identical to golden
//  cmydef.h:3159).  BCB6 accepted `strncpy(READBUFF, ...)`; ISO C++/MinGW will not
//  convert unsigned char* to char*, so an explicit cast is added.  Byte-for-byte
//  identical behaviour -- no reinterpretation, no length change.
                ::strncpy((char *)READBUFF, pRead, sizeof(READBUFF));

//                S1=Com2Buffer.SubString(32, 2);   //BCC

                if(Com2Buffer.SubString(6, 2)!="00" ||
                   Com2Buffer.SubString(12, 4)!="0000" ||
                   Com2Buffer.Length()<30)                                      //JerryYang 20240417 : 暫時新增長度保護
                {
                    ClearSendBuff();
                    Task=100;
                    ReceiveErr=true;
                    if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                    {
                        CommRetry[Addr]--;
                        Task=300;
                    }
                    else
                    {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2658.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StopComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port reset is skipped; every other statement of the retry/error
//  bookkeeping around it (Com2Delay / CommunCTErr / UN150CommError / Task=230)
//  stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2658; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                        { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StopComm(); }   //JerryYang 20190627 Omron溫度讀取error也要重新開COM   //AI(W906-I03) 20261002: golden COM2->Comm2->StopComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                        Com2Delay.SetSecAndOn(0.5);
                        CommunCTErr[Addr]++;
                        if(CommunCTErr[Addr]>5)
                        {
                            UN150CommError[Addr]=true;
                            CommRetry[Addr]=MAX_RETRY;
                            UN150Read[Addr]=999;
                        }
                        Task=230;
                    }
                    break;
                }

                CommRetry[Addr]=MAX_RETRY;
                ReceiveErr=false;
                if(Com2Buffer.SubString(8, 4)!="0101")                          //Function Code
                {
                    Task=300;
                    break;
                }

//Status
//{ "00000001000000000111000000000010" }
//{ "00000000000000000111000001000010" }
//   10987654321098765432109876543210
//    3         2         1
//
// 0: 加熱器電流過大
// 1: 加熱器電流保持
// 2: AD變換器錯誤
// 3: SSR故障
// 4: RSP輸入異常
// 5: None
// 6: 輸入異常
// 7: 電位計輸入異常
// 8: 加熱/開啟輸出
// 9: 冷卻/關閉輸出
//10: 加熱器斷線報警 CT1
//11: 加熱器斷線報警 CT2
//12: Alarm 1
//13: Alarm 2
//14: Alarm 3
//22: 設定模式

                S1=Com2Buffer.SubString(24, 8);
                S1=CountHexToByte(S1);
                S =Com2Buffer.SubString(20, 4);
                Read=HexStrToInt(S.c_str())/10.0;

                if(S1.SubString(26, 1)=="1" || S1.SubString(10, 1)=="1")        //KenHsieh 20230927 : Add Check 設定模式是否開啟
                {
                    Read=999.0;
                    UN150ReadReal[Addr]=999.0;
                }
                else
                {
                    UN150ReadReal[Addr]=Read;
                }

                if(Addr==tcLB ||                                                //Steven 20190924 : Add for ATC6.0
                   Addr==tcLBUp || Addr==tcLBDown)                              //Frank 20241231 : add
                {
                    UN150Read[Addr]=Read;
                }
                else if(Addr==tcCCD || (RTC_TemperNumber==2 && Addr==tcCCD_2) || Addr==tc2D)    //----- by dell ccd realtime-------------Steven 20110811   //wei 20160524 2D溫度    //Steven 20191009 : 多一個else       //Isaac 20201217 : RTC CCD增加第二組感溫
                {
                    dCCDTemperature=Read;
                    UN150Read[Addr]=ReadAverageBuffer(Addr, Read);              //Steven 20110924
                }
                else
                {
                    if(LastSet.iTemperature!=Tempture_Hot)                      //Steven 20240606 : SHUTTLE_COOLING for Ambient Control
                    {
                        UN150Read[Addr]=GetConvertTemp(Addr, Read)-SHUTTLE_COOLING;
                    }
                    else
                    {
                        if(GetConvertTemp(Addr, Read)<5)   //JerryYang 20240417 : 低於5度暫時先濾掉
                        {
                            S3.sprintf("Addr:%d, Temperature:%f,%s", Addr, Read, Com2Buffer);
                            RecordProcess(S3);
                        }
                        else
                        {
                            UN150Read[Addr]=ReadAverageBuffer(Addr, GetConvertTemp(Addr, Read));
                        }
                    }
                }

                ClearSendBuff();
                CommunCTErr[Addr]=0;                                            //JerryYang 20190627 Omron溫度讀取error也要重新開COM
                Task=300;                                                       //JerryYang 20190918 : 修正溫控器傳輸速度很慢的問題

//                if(LastSet.iTemperature==Tempture_Hot && IniConfig.bEnableKT4HAlarm1)  //Steven 20120809 : KT4H使用Alarm1作加熱保護
//                {
//                    if(Addr==tcCCD || Addr==tcHeatGun1 || Addr==tcHeatGun2 || Addr==tc2D || Addr==tcLB)   //wei 20160524 2D溫度
//                    {
//                        Task=300;
//                    }
//                    else
//                    {
//                        Task=500;
//                    }
//                }
//                else
//                {
//                    if(Addr==tcCCD || Addr==tcHeatGun1 || Addr==tcHeatGun2 || Addr==tc2D || Addr==tcLB)  //wei 20160524 2D溫度
//                        Task=300;
//                    else
//                        Task=500;
//                }
                break;
            }

            if(Com2Delay.Off())
            {
                if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                {
                    CommRetry[Addr]--;
                    Task=300;
                }
                else
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2782.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StopComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port reset is skipped; every other statement of the retry/error
//  bookkeeping around it (Com2Delay / CommunCTErr / UN150CommError / Task=230)
//  stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2782; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                    { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StopComm(); }   //JerryYang 20190627 Omron溫度讀取error也要重新開COM   //AI(W906-I03) 20261002: golden COM2->Comm2->StopComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                    Com2Delay.SetSecAndOn(0.5);
                    CommunCTErr[Addr]++;                                        //JerryYang 20190627 Omron溫度讀取error也要重新開COM
                    if(CommunCTErr[Addr]>5)
                    {
                        UN150CommError[Addr]=true;
                        CommRetry[Addr]=MAX_RETRY;
                        UN150Read[Addr]=999;
                    }
                    Task=230;
                    break;
                }
            }
            break;
//E5DC end
//DTK4848 start                                                                 //KaiHuang 20190821 : 新增台達 DTK4848溫控器
        case 2500:
            if(Com2ReceiveOK)
            {
                UN150CommError[Addr]=false;

                S=Com2Buffer;
                if(S.SubString(5, 1)=="6")                                      //Jimmychiu 20231205 : avoid temp down to error number
                {
                    ClearSendBuff();
                    CommunCTErr[Addr]=0;                                        //JerryYang 20170925 (wei) COM port Reset
                    Task=300;                                                   //JerryYang 20190918 : 修正溫控器傳輸速度很慢的問題
                    break;
                }
                S=S.SubString(8, 4);                                            //8~11位元為溫度值
                Read=(HexStrToInt(S.c_str()))/10.0f;
                if(Read==0)
                {
                    if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)         // 2010.08.31 , Joye
                    {
                        CommRetry[Addr]--;
                        Task=300;
                    }
                    else
                    {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2822.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StopComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port reset is skipped; every other statement of the retry/error
//  bookkeeping around it (Com2Delay / CommunCTErr / UN150CommError / Task=230)
//  stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2822; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                        { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StopComm(); }   //JerryYang 20170925 (wei) COM port Reset   //AI(W906-I03) 20261002: golden COM2->Comm2->StopComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                        Com2Delay.SetSecAndOn(0.5);
                        CommunCTErr[Addr]++;
                        if(CommunCTErr[Addr]>5)
                        {
                            UN150CommError[Addr]=true;
                            CommRetry[Addr]=MAX_RETRY;
                            UN150Read[Addr]=999;
                        }
                        Task=230;
                    }
                }
                CommRetry[Addr]=MAX_RETRY;
                ReceiveErr=false;
                UN150ReadReal[Addr]=Read;

                if(Addr==tcCCD || (RTC_TemperNumber==2 && Addr==tcCCD_2))       //----- by dell ccd realtime-------------Steven 20110811       //Isaac 20201217 : RTC CCD增加第二組感溫
                {
                    dCCDTemperature=Read;
                    UN150Read[Addr]=ReadAverageBuffer(Addr, Read);              //Steven 20110924
                }
                else if(Addr==tcChamber)                                        //Isaac Ifor 20170911 scoket(dut)和chamber(hot air)不做溫度轉換
                {
                    UN150Read[Addr]=Read;
                }
                else
                {
                    if(Tri_Temp_Machine==1)                                          //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                    {
                        UN150Read[Addr]=Read-Temperature.fTempOffSet[UserOffSet][Addr]; //Heater溫度設定
                    }
                    else if(LastSet.iTemperature!=Tempture_Hot)                      //Steven 20240606 : SHUTTLE_COOLING for Ambient Control
                    {
                        UN150Read[Addr]=GetConvertTemp(Addr, Read)-SHUTTLE_COOLING;
                    }
                    else
                    {
                        UN150Read[Addr]=ReadAverageBuffer(Addr, GetConvertTemp(Addr, Read));
                    }
                }

                ClearSendBuff();
                CommunCTErr[Addr]=0;                                            //JerryYang 20170925 (wei) COM port Reset
                Task=300;                                                       //JerryYang 20190918 : 修正溫控器傳輸速度很慢的問題

//                if(LastSet.iTemperature==Tempture_Hot && IniConfig.bEnableKT4HAlarm1)  //Steven 20120809 : KT4H使用Alarm1作加熱保護
//                {
//                    if(Addr==tcCCD || Addr ==tcChamber)         //Isaac 20171016 add(chamber)HotAir
//                    {
//                        Task=300;
//                    }
//                    else
//                    {
//                        Task=500;
//                    }
//                }
//                else
//                {
//                    if(Addr==tcCCD || Addr ==tcChamber)          //Isaac 20171016 add(chamber)HotAir
//                    {
//                        Task=300;
//                    }
//                    else
//                    {
//                        Task=500;
//                    }
//                }
                break;
            }

            if(Com2Delay.Off())
            {
                if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)             // 2010.08.31 , Joye
                {
                    CommRetry[Addr]--;
                    Task=300;
                }
                else
                {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G17 -- golden :2901.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no Comm2 member, so COM2->Comm2->StopComm() cannot compile.
//  ACTIVE DEFAULT: the COM-port reset is skipped; every other statement of the retry/error
//  bookkeeping around it (Com2Delay / CommunCTErr / UN150CommError / Task=230)
//  stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G17 OPENED (golden :2901; the 20260807 note above is history): TCOM2Shim::Comm2 exists now (rs232.cpp)
                    { extern Spcomm::TComm* g_pCOM2Comm2; if (g_pCOM2Comm2) g_pCOM2Comm2->StopComm(); }   //JerryYang 20170925 (wei) COM port Reset   //AI(W906-I03) 20261002: golden COM2->Comm2->StopComm(); -- the seam is COM2->Comm2 (rs232.cpp W906_Comm2DfmBoot)
//AI(W906-I03) 20261002: (the old #endif of G17)
                    Com2Delay.SetSecAndOn(0.5);
                    CommunCTErr[Addr]++;                                        //JerryYang 20170929 (wei) CommunCTErr改成陣列
                    if(CommunCTErr[Addr]>5)
                    {
                        UN150CommError[Addr]=true;
                        CommRetry[Addr]=MAX_RETRY;
                        UN150Read[Addr]=999;
                    }
                    Task=230;
                    break;
                }
            }
            break;
//DTK4848 end
//jou 2012-03-14 ATC start:
        case 260:
            if(bUT150Install[Addr]==true && ComATCReceiveOK[Addr]==true)
            {
                UN150CommError[Addr]=false;
                if(bATCReceiveErr[Addr]==true)
                {
                    ReceiveErr=true;
                    if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                    {
                        CommRetry[Addr]--;
                        Task=100;
                    }
                    else
                    {
                        UN150CommError[Addr]=true;
                        CommRetry[Addr]=MAX_RETRY;
                        UN150Read[Addr]=999;
                        Task=300;
                    }
                    break;
                }
                CommRetry[Addr]=MAX_RETRY;
                ReceiveErr=false;
                ComATCReceiveOK[Addr]=false;

                if(WriteCommand==6 && bATCWriteCommand[Addr]==true)             // is not read current templeate
                {
                    //Write Temperature
                }
                else
                {
                    //Read Temperature
                    UN150ReadReal[Addr]=fATCReadBuffer[Addr];
                    UN150Read[Addr]=fATCReadBuffer[Addr];
                }

                Task=300;
                break;
            }

            if(Com2Delay.Off())
            {
                if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                {
                    CommRetry[Addr]--;
                    Task=300;
                }
                else
                {
                    UN150CommError[Addr]=true;
                    CommRetry[Addr]=MAX_RETRY;
                    UN150Read[Addr]=999;
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G18 -- golden :2969.
//  COM2 is atester_shims.h::TCOM2Shim (golden rs232.h:203 TCOM2 is unported).  The
//  shim exposes bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck only --
//  no ATCInitialTask().
//  ACTIVE DEFAULT: no ATC re-init is kicked; UN150Read[Addr]=999 and Task=300 still happen.
#if 0 // TODO(W5 G18): golden :2969
                    COM2->ATCInitialTask();
#endif // TODO(W5 G18)
                    Task=300;
                    break;
                }
            }
            break;
        case 270:
            if((Addr>=tcAa1 && Addr<=tcBd2) ||
               (Addr>=tcAe1 && Addr<=tcBh2) ||
               (Addr>=tcHead1 && Addr<=tcHead4))                                //JerryYang 20191009 fix index 4組加熱器版本 ATC顯示異常
            {
                UN150Read[Addr]=DOUN150ReadTemp(Addr);
                if(CosFunction.bATCUseTempAdjustment==true)
                {
                    if(LastSet.iTemperature==Tempture_Hot ||
                       LastSet.iTemperature==Tempture_AmbientHot)               //kevin 20140918 恆溫控制
                        dbSetATCAdjustmentOffset=Temperature.fWorkTemperBase;   //Ifor 20160111 : ATC高溫的設定溫度
                    else                                                        //Ifor 20160111 : [L11] ATC常溫的設定溫度
                        dbSetATCAdjustmentOffset=IniConfig.dATCAmbientTemperature;

                    Temp=GetFactSetTemp(Addr, dbSetATCAdjustmentOffset);        //取得要設定至溫控器的溫度

                    dbSetATCAdjustmentOffset=Temp-dbSetATCAdjustmentOffset;

                    if(Addr>=tcAa1 && Addr<=tcAd1)
                    {
                        dATCTempAdjustmentOffset[Addr-11]=dbSetATCAdjustmentOffset;
                    }
                    else if(Addr>=tcBa1 && Addr<=tcBd1)
                    {
                        dATCTempAdjustmentOffset[Addr-7]=dbSetATCAdjustmentOffset;
                    }
                    else if(Addr>=tcAe1 && Addr<=tcAh1)
                    {
                        dATCTempAdjustmentOffset[Addr-29]=dbSetATCAdjustmentOffset;
                    }
                    else if(Addr>=tcBe1 && Addr<=tcBh1)
                    {
                        dATCTempAdjustmentOffset[Addr-25]=dbSetATCAdjustmentOffset;
                    }
                    else if(Addr>=tcAa2 && Addr<=tcAd2)
                    {
                        dATCTempAdjustmentOffset[Addr-3]=dbSetATCAdjustmentOffset;
                    }
                    else if(Addr>=tcBa2 && Addr<=tcBd2)
                    {
                        dATCTempAdjustmentOffset[Addr+1]=dbSetATCAdjustmentOffset;
                    }
                    else if(Addr>=tcAe2 && Addr<=tcAh2)
                    {
                        dATCTempAdjustmentOffset[Addr-21]=dbSetATCAdjustmentOffset;
                    }
                    else if(Addr>=tcBe2 && Addr<=tcBh2)
                    {
                        dATCTempAdjustmentOffset[Addr-17]=dbSetATCAdjustmentOffset;
                    }
                }
            }

            if(Addr==tcBh2)                                                 //算完全部OffSet
                bATCTempAdjustmentOffset=true;
            Task=300;
            break;
//jou 2012-03-14 ATC end
        case 300:
            WriteCommand=-1;
            Addr++;
            if(Addr>=tcTotalCount)
                Addr=0;
            Task=100;
            break;
        case 500:
            if(TC401HeaterControl!=KT4H)                                        //JerryYang 20190918 : 修正溫控器傳輸速度很慢的問題
            {
                Task=300;
                break;
            }

            Com2ReceiveOK=false;
            WriteCommand=0x000B;                                                //送Alarm 1 Value

            if(OldTemp[Addr]==0)
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G22 -- golden :3052.
//  UT100WordWriteNoSucm() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:326 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: the KT4H Alarm-1 value 0 is skipped; all surrounding SM bookkeeping stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G22 OPENED (golden :3052; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                UT100WordWriteNoSucm(Addr, WriteCommand, 0);
//AI(W906-I03) 20261002: (the old #endif of G22)
            }

            if(OldTemp[Addr]<120.0 && OldTemp[Addr]>0)                          //小於120的一律設定為140度
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G22 -- golden :3057.
//  UT100WordWriteNoSucm() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:326 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: the KT4H Alarm-1 value 140.0 is skipped; all surrounding SM bookkeeping stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G22 OPENED (golden :3057; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                UT100WordWriteNoSucm(Addr, WriteCommand, 140*10);
//AI(W906-I03) 20261002: (the old #endif of G22)
            }
            else if(OldTemp[Addr]>=120.0)                                       //大於120的一律比設定多10度
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G22 -- golden :3061.
//  UT100WordWriteNoSucm() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:326 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: the KT4H Alarm-1 value OldTemp+10 is skipped; all surrounding SM bookkeeping stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G22 OPENED (golden :3061; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                UT100WordWriteNoSucm(Addr, WriteCommand, (OldTemp[Addr]+10)*10);
//AI(W906-I03) 20261002: (the old #endif of G22)
            }
            Com2Delay.Set0_1SecAndOn(6);
            Task=510;
        case 510:
            if(Com2ReceiveOK)
            {
                UN150CommError[Addr]=false;
                pRead=(char *)Com2Buffer.c_str();                               //Steven 20111028 : 改成AnsiString
//AI(W906-GA-BTHERMO) 20260807: READBUFF is `unsigned char[1024]` (port cmydef.h:3172, identical to golden
//  cmydef.h:3159).  BCB6 accepted `strncpy(READBUFF, ...)`; ISO C++/MinGW will not
//  convert unsigned char* to char*, so an explicit cast is added.  Byte-for-byte
//  identical behaviour -- no reinterpretation, no length change.
                ::strncpy((char *)READBUFF, pRead, sizeof(READBUFF));

                if(A_Check_LRC()==ERROR_CODE)                                   //check LRC CODE
                {
                    ClearSendBuff();
                    ReceiveErr=true;
                    if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                    {
                        CommRetry[Addr]--;
                        Task=500;
                    }
                    else
                    {
                        UN150CommError[Addr]=true;
                        CommRetry[Addr]=MAX_RETRY;
                        UN150Read[Addr]=999;
                        Task=300;
                    }
                    break;
                }
                CommRetry[Addr]=MAX_RETRY;
                ReceiveErr=false;

                i=A_Get_Function_Code();
                ClearSendBuff();
                Task=300;                                                       //Steven 20120817 : 500 -> 300
                break;
            }

            if(Com2Delay.Off())
            {
                if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                {
                    CommRetry[Addr]--;
                    Task=500;
                }
                else
                {
                    UN150CommError[Addr]=true;
                    CommRetry[Addr]=MAX_RETRY;

                    Task=300;
                    break;
                }
            }
            break;
        case 520:
            Com2ReceiveOK=false;
            WriteCommand=0x0023;                                                //送Alarm 1 Type
            if(bUT150Install[Addr]==false || OldTemp[Addr]==0)
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G22 -- golden :3121.
//  UT100WordWriteNoSucm() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:326 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: the KT4H Alarm-1 type 0 (no alarm) is skipped; all surrounding SM bookkeeping stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G22 OPENED (golden :3121; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                UT100WordWriteNoSucm(Addr, WriteCommand, 0);                    //0000H = No Alarm
//AI(W906-I03) 20261002: (the old #endif of G22)
            }
            else
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W5 G22 -- golden :3125.
//  UT100WordWriteNoSucm() is DECLARED in the port cpublic.h but its DEFINITION at
//  cpublic.cpp:326 sits inside that file's `#if 0` block (blocked there on the
//  unported EJ1N/TextProcess module and on COM2->Comm2).  Undefined reference.
//  ACTIVE DEFAULT: the KT4H Alarm-1 type 1 (high limit) is skipped; all surrounding SM bookkeeping stays ACTIVE.
//AI(W906-I03) 20261002 (Ifor01): GATE W5 G22 OPENED (golden :3125; the 20260807 note above is history): the cpublic.cpp bodies are live now (TextProcess ported, Comm2 through g_pCOM2Comm2)
                UT100WordWriteNoSucm(Addr, WriteCommand, 1);                    //0001H = High Limit Alarm
//AI(W906-I03) 20261002: (the old #endif of G22)
            }
            Com2Delay.Set0_1SecAndOn(6);
            Task=530;
        case 530:
            if(Com2ReceiveOK)
            {
                UN150CommError[Addr]=false;
                pRead=(char *)Com2Buffer.c_str();                               //Steven 20111028 : 改成AnsiString
//AI(W906-GA-BTHERMO) 20260807: READBUFF is `unsigned char[1024]` (port cmydef.h:3172, identical to golden
//  cmydef.h:3159).  BCB6 accepted `strncpy(READBUFF, ...)`; ISO C++/MinGW will not
//  convert unsigned char* to char*, so an explicit cast is added.  Byte-for-byte
//  identical behaviour -- no reinterpretation, no length change.
                ::strncpy((char *)READBUFF, pRead, sizeof(READBUFF));

                if(A_Check_LRC()==ERROR_CODE)                                   //check LRC CODE
                {
                    ClearSendBuff();
                    ReceiveErr=true;
                    if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                    {
                        CommRetry[Addr]--;
                        Task=500;
                    }
                    else
                    {
                        UN150CommError[Addr]=true;
                        CommRetry[Addr]=MAX_RETRY;
                        UN150Read[Addr]=999;
                        Task=300;
                    }
                    break;
                }
                CommRetry[Addr]=MAX_RETRY;
                ReceiveErr=false;

                i=A_Get_Function_Code();
                ClearSendBuff();
                Task=300;
                break;
            }

            if(Com2Delay.Off())
            {
                if(CommRetry[Addr]>0 && CommRetry[Addr]<=MAX_RETRY)
                {
                    CommRetry[Addr]--;
                    Task=500;
                }
                else
                {
                    UN150CommError[Addr]=true;
                    CommRetry[Addr]=MAX_RETRY;

                    Task=300;
                    break;
                }
            }
            break;
     }
}
//---------------------------------------------------------------------------
void DoATC60Temperature()                                                       //ChungHung 21041105 add for ATC6.0 and ATC3.0
{
    int Addr;
    double Temp[tcTotalCount]={0};
    double dWorkTemp;
    bool bHeater=false;

    if(InitialOK==false)                                                        //Steven 20110809
        return;

    for(Addr=tcAa1; Addr<=tcBh2; Addr++)                                        //Steven 20140923 : Index使用EJ1N版32組加熱器
    {
        if(Addr>=tcHeatGun1 && Addr<=tcDUT4)                                    //Steven 20140923 : Index使用EJ1N版32組加熱器
            continue;

        if(LastSet.iTemperature==Tempture_Hot)
        {
            if(bUT150Install[Addr]==false)
            {
                Temp[Addr]=0.0;
            }
            else
            {
                dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                Temp[Addr]=GetFactSetTemp(Addr, dWorkTemp);                     //取得要設定至溫控器的溫度
            }

//AI(W906-GA-BTHERMO) 20260807: GATE W7 G12/G13 -- golden :3210-3211, the same two dangling externs
//  (csystem.h:226 DoCloseHeadterDelay / csystem.h:197 bHeaterDoorIsOpen[4], neither
//  defined anywhere).  false == a zero-initialised global, i.e. "door closed".
            if(BTHERMO_G12_DOCLOSEHEADTERDELAY_OFF &&                                     //jou 2011-12-26 加個delay 3 sec,避免門關不好,一直on/off
               (BTHERMO_G13_HEATERDOOR_IS_OPEN(0) || BTHERMO_G13_HEATERDOOR_IS_OPEN(1)))
            {
                bHeater=false;
            }
            else
            {
                #ifndef SOFT_SIMULTE
                if(SW[SwHeaterRelay].Status()==false)
                {
                    bHeater=false;
                }
                else
                {
                    bHeater=true;
                }
                #endif
            }
        }
        else
        {
            Temp[Addr]=0.0;                                                     //jou 980318 add from HT9080A
        }

        if(Temperature.iIndexHeatMode==ChamberOnly ||
           Temperature.iIndexHeatMode==SocketChamber)                           //Chamber Only (head 1 2 3 4 & socket no use)
        {
            if((Addr>=tcChamber && Addr<=tcHead4) ||
               (Addr>=tcAa1 && Addr<=tcBd2)       ||
               (Addr>=tcAe1 && Addr<=tcBh2))                                    //Steven 20140923 : Index使用EJ1N版32組加熱器
                Temp[Addr]=0.0;
        }
    }

//AI(W906-GA-BTHERMO) 20260807: bHeater is computed above and consumed ONLY inside gate G25 below.
//  Temp[] likewise (only gate G25 reads it).  Retained verbatim per golden
//  and silenced rather than deleted.
    (void)bHeater;
    (void)Temp;
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G25 -- golden :3243-3319.
//  ATCInterfaceForm (golden ATC/ATCInterface.h) and its ATC_60_SYS member
//  (ATC/ATCSystem.h: SetTargetTemperature / SetTargetOffsetTemperature / SetATCRun /
//  GetRealTemp / IsConnected / IsConnecting / bSend[] / SendTemp / ReadTemp[] plus the
//  SETTEMP / SETOFFSET / USESITE / ATCENABLED / GETTEMP tags) have NO translated home
//  anywhere in this tree -- whole-tree grep for ATCInterfaceForm and ATC_60_SYS returns
//  zero hits.  Cannot compile.  The Temp[]/bHeater computation ABOVE stays ACTIVE.
//  ACTIVE DEFAULT: no ATC6.0/3.0 command is sequenced and UN150Read[] is left untouched (rather than
//  being overwritten with 999.0), so whatever the HAL last published survives.
#if 0 // TODO(W7-UI G25): golden :3243-3319
    static int count=0;
    static int iTask=0;
    count++;
    if(count>1)
    {
        count=0;

        switch(iTask)
        {
            case 0:
                ATCInterfaceForm->ATC_60_SYS.SetTargetTemperature(Temperature.fWorkTemperBase);
                iTask=1;
                break;
            case 1:
                if(ATCInterfaceForm->ATC_60_SYS.bSend[ATCInterfaceForm->ATC_60_SYS.SETTEMP]==false)
                {
                    ATCInterfaceForm->ATC_60_SYS.SetTargetOffsetTemperature(&Temp[tcAa1],16);
                    iTask=2;
                }
                break;
            case 2:
                if(ATCInterfaceForm->ATC_60_SYS.bSend[ATCInterfaceForm->ATC_60_SYS.SETOFFSET]==false)
                {
                    ATCInterfaceForm->ATC_60_SYS.SetTargetOffsetTemperature(ATCInterfaceForm->ATC_60_SYS.SendTemp, 16);
                    iTask=3;
                }
                break;
            case 3:
                if(ATCInterfaceForm->ATC_60_SYS.bSend[ATCInterfaceForm->ATC_60_SYS.USESITE]==false)
                {
                    ATCInterfaceForm->ATC_60_SYS.SetATCRun(bHeater);
                    iTask=4;
                }
                break;
            case 4:
                if(ATCInterfaceForm->ATC_60_SYS.bSend[ATCInterfaceForm->ATC_60_SYS.ATCENABLED]==false)
                {
                    ATCInterfaceForm->ATC_60_SYS.GetRealTemp();
                    iTask=5;
                }
                break;
            case 5:
                if(ATCInterfaceForm->ATC_60_SYS.bSend[ATCInterfaceForm->ATC_60_SYS.GETTEMP]==false)
                {
                    iTask=0;
                }
                break;
        }
    }

    if(ATCInterfaceForm->ATC_60_SYS.IsConnected()==false ||                     //讀取資料
       ATCInterfaceForm->ATC_60_SYS.IsConnecting()==true)
    {
        for(Addr=tcAa1; Addr<=tcBd2; Addr++)
        {
            UN150Read[Addr]=999.0;
        }
    }
    else
    {
        UN150Read[tcAa1]=GetConvertTemp(tcAa1, ATCInterfaceForm->ATC_60_SYS.ReadTemp[0]);
        UN150Read[tcBa1]=GetConvertTemp(tcBa1, ATCInterfaceForm->ATC_60_SYS.ReadTemp[1]);
        UN150Read[tcAb1]=GetConvertTemp(tcAb1, ATCInterfaceForm->ATC_60_SYS.ReadTemp[2]);
        UN150Read[tcBb1]=GetConvertTemp(tcBb1, ATCInterfaceForm->ATC_60_SYS.ReadTemp[3]);
        UN150Read[tcAc1]=GetConvertTemp(tcAc1, ATCInterfaceForm->ATC_60_SYS.ReadTemp[4]);
        UN150Read[tcBc1]=GetConvertTemp(tcBc1, ATCInterfaceForm->ATC_60_SYS.ReadTemp[5]);
        UN150Read[tcAd1]=GetConvertTemp(tcAd1, ATCInterfaceForm->ATC_60_SYS.ReadTemp[6]);
        UN150Read[tcBd1]=GetConvertTemp(tcBd1, ATCInterfaceForm->ATC_60_SYS.ReadTemp[7]);
        UN150Read[tcAa2]=GetConvertTemp(tcAa2, ATCInterfaceForm->ATC_60_SYS.ReadTemp[8]);
        UN150Read[tcBa2]=GetConvertTemp(tcBa2, ATCInterfaceForm->ATC_60_SYS.ReadTemp[9]);
        UN150Read[tcAb2]=GetConvertTemp(tcAb2, ATCInterfaceForm->ATC_60_SYS.ReadTemp[10]);
        UN150Read[tcBb2]=GetConvertTemp(tcBb2, ATCInterfaceForm->ATC_60_SYS.ReadTemp[11]);
        UN150Read[tcAc2]=GetConvertTemp(tcAc2, ATCInterfaceForm->ATC_60_SYS.ReadTemp[12]);
        UN150Read[tcBc2]=GetConvertTemp(tcBc2, ATCInterfaceForm->ATC_60_SYS.ReadTemp[13]);
        UN150Read[tcAd2]=GetConvertTemp(tcAd2, ATCInterfaceForm->ATC_60_SYS.ReadTemp[14]);
        UN150Read[tcBd2]=GetConvertTemp(tcBd2, ATCInterfaceForm->ATC_60_SYS.ReadTemp[15]);
    }
#endif // TODO(W7-UI G25)
}
//---------------------------------------------------------------------------
TQPF_Timer tEJ1NAlarmTimer;
void DoSetSVOfOmronEJ1N()                                                       //Steven 20120220 : Omron EJ1N溫控器
{
    int Addr=0;
    static double OldTemp[tcTotalCount]={0.0};
    double Temp[tcTotalCount]={0};
    double dWorkTemp;
    bool bTempHasChange=false;
    static bool bHasErrorSet=false;                                             //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20
    static AnsiString asErrorAddr="";                                           //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20

    if(InitialOK==false)                                                        //Steven 20110809
        return;

//AI(W906-GA-BTHERMO) 20260807: OldTemp[] and bTempHasChange are read ONLY inside gates G26a/G26b,
//  bHasErrorSet only inside G19b.  Retained verbatim per golden and silenced
//  rather than deleted.
    (void)OldTemp;
    (void)bTempHasChange;
    (void)bHasErrorSet;
    (void)Temp;

    for(int i=0; i<INDEX_HEAT_COUNT; i++)
    {
        Addr=iTempCode[i];

        if(USE_16_HEATER==eht16HeaterEJ1N && (Addr>=tcAe1))
        {
            continue;
        }

        if(LastSet.iTemperature==Tempture_Hot ||
           LastSet.iTemperature==Tempture_AmbientHot)                           //kevin 20180628 add 恆溫控制
        {
            if(bUT150Install[Addr]==false)
            {
                Temp[Addr]=0.0;
            }
            else
            {
                dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                Temp[Addr]=GetFactSetTemp(Addr, dWorkTemp);                     //取得要設定至溫控器的溫度
            }

            if(IniConfig.bL19OpenHeatDoorgiveupchamberhot==false)               //kevin 20170520 (wei) 開chambo門只有不加熱chambo
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W7 G12/G13 -- golden :3360-3361, the same two dangling externs
//  (csystem.h:226 DoCloseHeadterDelay, csystem.h:197 bHeaterDoorIsOpen[4]; neither is
//  defined anywhere in this tree).  Defaults: Off()==true (a default-constructed
//  TQPF_Timer, rEnd==0) and door-closed==false (a zero-initialised global).
                if(BTHERMO_G12_DOCLOSEHEADTERDELAY_OFF &&                                 //jou 2011-12-26 加個delay 3 sec,避免門關不好,一直on/off
                   (BTHERMO_G13_HEATERDOOR_IS_OPEN(0) || BTHERMO_G13_HEATERDOOR_IS_OPEN(1)))
                {
                    Temp[Addr]=0.0;
                }
            }
            #ifndef SOFT_SIMULTE
            if(SW[SwHeaterRelay].Status()==false)
            {
                Temp[Addr]=0.0;
            }
            #endif
        }
        else
        {
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G26a -- golden :3375-3378.
//  fOmron (golden OmronEJ1N.h) and its myPal[][]->edSV / dSetValue[][] /
//  dTempValue[][] / btSetSV / bShow / bGetComponentFinish / bResetCom surface have NO
//  translated home -- whole-tree grep for fOmron returns zero hits outside generated
//  dfm2rc layout string data.  Cannot compile.
//  ACTIVE DEFAULT: Temp[Addr]=0.0 -- golden's own else branch (:3378), which is what an unshown fOmron
//  form yields.  Emitted ACTIVE immediately below.
#if 0 // TODO(W7-UI G26a): golden :3375-3378
            if(fOmron->bShow)                                                   //jou 980318 add from HT9080A
                Temp[Addr]=OldTemp[Addr];
            else
                Temp[Addr]=0.0;
#endif // TODO(W7-UI G26a)
            Temp[Addr]=0.0;                                                     //AI(W906-GA-BTHERMO) 20260807: G26a ACTIVE default
        }

        if(Temperature.iIndexHeatMode==ChamberOnly ||
           Temperature.iIndexHeatMode==SocketChamber)                           //Chamber Only (head 1 2 3 4 & socket no use)
        {
            Temp[Addr]=0.0;
        }

//AI(W906-GA-BTHERMO) 20260807: GATE W7 G19b -- golden :3387-3401.
//  FormHS->CheckTempOffset(): golden HS_Function.h:93 (TFormHS *FormHS).  The only
//  FormHS in this tree is Automation/SCK_ART_Remainder.h's TU-local stand-in, whose
//  entire surface is UpDataToServerByFTP (see its own gate #17 note) -- no
//  CheckTempOffset -- and HS_Function.{h,cpp} is unported.  Cannot compile.
//  ACTIVE DEFAULT: same as G19a, EJ1N SV path.
#if 0 // TODO(W7 G19b): golden :3387-3401
        if(FormHS->CheckTempOffset(Addr, Temp[Addr]))                           //設定溫度大於機台能力
        {
            if(bHasErrorSet==false)                                             //判斷有無Alarm過
            {
                FormHS->CheckTempOffset(0,0,true);                              //Alarm 溫度設定錯誤
                tEJ1NAlarmTimer.SetSecAndOn(20);
                bHasErrorSet=true;                                              //設定Alarm旗標
            }
            else
            {
                if(tEJ1NAlarmTimer.Off())
                    bHasErrorSet=false;                                         //取消Alarm旗標
            }
            Temp[Addr]=Temperature.fWorkTemperBase;                             //設定溫度超過機台極限改寫工作溫度給溫控器(不包含Offset)
        }
#endif // TODO(W7 G19b)
    }

//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G26b -- golden :3404-3783.
//  fOmron (golden OmronEJ1N.h) and its myPal[][]->edSV / dSetValue[][] /
//  dTempValue[][] / btSetSV / bShow / bGetComponentFinish / bResetCom surface have NO
//  translated home -- whole-tree grep for fOmron returns zero hits outside generated
//  dfm2rc layout string data.  Cannot compile.
//  This is the whole SV-write + PV-read half of the function (380 lines), solid fOmron
//  dereferences; it additionally calls HeaterSVLog (see G20).  The Temp[] computation
//  loop above (golden :3336-3402) stays ACTIVE.
//  GOLDEN BUG kept byte-identical inside the gate: golden :3457
//  `HeaterSVLog(tcAb1, Temp[tcBa1]);` logs the tcBa1 setpoint under the tcAb1 channel
//  (copy-paste slip; all 31 sibling lines pass their own channel).  Not fixed.
//  ACTIVE DEFAULT: no SV is written to and no PV is read from the EJ1N controllers; UN150Read[] is left
//  untouched rather than being zeroed.
#if 0 // TODO(W7-UI G26b): golden :3404-3783
    if(USE_16_HEATER==eht16HeaterEJ1N || USE_16_HEATER==eht32HeaterEJ1N)        //Steven 20140923 : Index使用EJ1N版32組加熱器
    {
        fOmron->myPal[0][0]->edSV->Text=AnsiString(Temp[tcAa1]);
        fOmron->myPal[0][1]->edSV->Text=AnsiString(Temp[tcBa1]);
        fOmron->myPal[0][2]->edSV->Text=AnsiString(Temp[tcAb1]);
        fOmron->myPal[0][3]->edSV->Text=AnsiString(Temp[tcBb1]);
        fOmron->myPal[1][0]->edSV->Text=AnsiString(Temp[tcAc1]);
        fOmron->myPal[1][1]->edSV->Text=AnsiString(Temp[tcBc1]);
        fOmron->myPal[1][2]->edSV->Text=AnsiString(Temp[tcAd1]);
        fOmron->myPal[1][3]->edSV->Text=AnsiString(Temp[tcBd1]);
        fOmron->myPal[2][0]->edSV->Text=AnsiString(Temp[tcAa2]);
        fOmron->myPal[2][1]->edSV->Text=AnsiString(Temp[tcBa2]);
        fOmron->myPal[2][2]->edSV->Text=AnsiString(Temp[tcAb2]);
        fOmron->myPal[2][3]->edSV->Text=AnsiString(Temp[tcBb2]);
        fOmron->myPal[3][0]->edSV->Text=AnsiString(Temp[tcAc2]);
        fOmron->myPal[3][1]->edSV->Text=AnsiString(Temp[tcBc2]);
        fOmron->myPal[3][2]->edSV->Text=AnsiString(Temp[tcAd2]);
        fOmron->myPal[3][3]->edSV->Text=AnsiString(Temp[tcBd2]);

        if(USE_16_HEATER==eht32HeaterEJ1N)                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
        {
            fOmron->myPal[4][0]->edSV->Text=AnsiString(Temp[tcAe1]);
            fOmron->myPal[4][1]->edSV->Text=AnsiString(Temp[tcBe1]);
            fOmron->myPal[4][2]->edSV->Text=AnsiString(Temp[tcAf1]);
            fOmron->myPal[4][3]->edSV->Text=AnsiString(Temp[tcBf1]);
            fOmron->myPal[5][0]->edSV->Text=AnsiString(Temp[tcAg1]);
            fOmron->myPal[5][1]->edSV->Text=AnsiString(Temp[tcBg1]);
            fOmron->myPal[5][2]->edSV->Text=AnsiString(Temp[tcAh1]);
            fOmron->myPal[5][3]->edSV->Text=AnsiString(Temp[tcBh1]);
            fOmron->myPal[6][0]->edSV->Text=AnsiString(Temp[tcAe2]);
            fOmron->myPal[6][1]->edSV->Text=AnsiString(Temp[tcBe2]);
            fOmron->myPal[6][2]->edSV->Text=AnsiString(Temp[tcAf2]);
            fOmron->myPal[6][3]->edSV->Text=AnsiString(Temp[tcBf2]);
            fOmron->myPal[7][0]->edSV->Text=AnsiString(Temp[tcAg2]);
            fOmron->myPal[7][1]->edSV->Text=AnsiString(Temp[tcBg2]);
            fOmron->myPal[7][2]->edSV->Text=AnsiString(Temp[tcAh2]);
            fOmron->myPal[7][3]->edSV->Text=AnsiString(Temp[tcBh2]);
        }

        if(Temp[tcAa1]!=fOmron->dSetValue[0][0])
        {
            HeaterSVLog(tcAa1, Temp[tcAa1]);
            bTempHasChange=true;
        }

        if(Temp[tcBa1]!=fOmron->dSetValue[0][1])
        {
            HeaterSVLog(tcBa1, Temp[tcBa1]);
            bTempHasChange=true;
        }

        if(Temp[tcAb1]!=fOmron->dSetValue[0][2])
        {
            HeaterSVLog(tcAb1, Temp[tcBa1]);
            bTempHasChange=true;
        }

        if(Temp[tcBb1]!=fOmron->dSetValue[0][3])
        {
            HeaterSVLog(tcBb1, Temp[tcBb1]);
            bTempHasChange=true;
        }

        if(Temp[tcAc1]!=fOmron->dSetValue[1][0])
        {
            HeaterSVLog(tcAc1, Temp[tcAc1]);
            bTempHasChange=true;
        }

        if(Temp[tcBc1]!=fOmron->dSetValue[1][1])
        {
            HeaterSVLog(tcBc1, Temp[tcBc1]);
            bTempHasChange=true;
        }

        if(Temp[tcAd1]!=fOmron->dSetValue[1][2])
        {
            HeaterSVLog(tcAd1, Temp[tcAd1]);
            bTempHasChange=true;
        }

        if(Temp[tcBd1]!=fOmron->dSetValue[1][3])
        {
            HeaterSVLog(tcBd1, Temp[tcBd1]);
            bTempHasChange=true;
        }

        if(Temp[tcAa2]!=fOmron->dSetValue[2][0])
        {
            HeaterSVLog(tcAa2, Temp[tcAa2]);
            bTempHasChange=true;
        }

        if(Temp[tcBa2]!=fOmron->dSetValue[2][1])
        {
            HeaterSVLog(tcBa2, Temp[tcBa2]);
            bTempHasChange=true;
        }

        if(Temp[tcAb2]!=fOmron->dSetValue[2][2])
        {
            HeaterSVLog(tcAb2, Temp[tcAb2]);
            bTempHasChange=true;
        }

        if(Temp[tcBb2]!=fOmron->dSetValue[2][3])
        {
            HeaterSVLog(tcBb2, Temp[tcBb2]);
            bTempHasChange=true;
        }

        if(Temp[tcAc2]!=fOmron->dSetValue[3][0])
        {
            HeaterSVLog(tcAc2, Temp[tcAc2]);
            bTempHasChange=true;
        }

        if(Temp[tcBc2]!=fOmron->dSetValue[3][1])
        {
            HeaterSVLog(tcBc2, Temp[tcBc2]);
            bTempHasChange=true;
        }

        if(Temp[tcAd2]!=fOmron->dSetValue[3][2])
        {
            HeaterSVLog(tcAd2, Temp[tcAd2]);
            bTempHasChange=true;
        }

        if(Temp[tcBd2]!=fOmron->dSetValue[3][3])
        {
            HeaterSVLog(tcBd2, Temp[tcBd2]);
            bTempHasChange=true;
        }

        if(USE_16_HEATER==eht32HeaterEJ1N)                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
        {
            if(Temp[tcAe1]!=fOmron->dSetValue[4][0])
            {
                HeaterSVLog(tcAe1, Temp[tcAe1]);
                bTempHasChange=true;
            }

            if(Temp[tcBe1]!=fOmron->dSetValue[4][1])
            {
                HeaterSVLog(tcBe1, Temp[tcBe1]);
                bTempHasChange=true;
            }

            if(Temp[tcAf1]!=fOmron->dSetValue[4][2])
            {
                HeaterSVLog(tcAf1, Temp[tcAf1]);
                bTempHasChange=true;
            }

            if(Temp[tcBf1]!=fOmron->dSetValue[4][3])
            {
                HeaterSVLog(tcBf1, Temp[tcBf1]);
                bTempHasChange=true;
            }

            if(Temp[tcAg1]!=fOmron->dSetValue[5][0])
            {
                HeaterSVLog(tcAg1, Temp[tcAg1]);
                bTempHasChange=true;
            }

            if(Temp[tcBg1]!=fOmron->dSetValue[5][1])
            {
                HeaterSVLog(tcBg1, Temp[tcBg1]);
                bTempHasChange=true;
            }

            if(Temp[tcAh1]!=fOmron->dSetValue[5][2])
            {
                HeaterSVLog(tcAh1, Temp[tcAh1]);
                bTempHasChange=true;
            }

            if(Temp[tcBh1]!=fOmron->dSetValue[5][3])
            {
                HeaterSVLog(tcBh1, Temp[tcBh1]);
                bTempHasChange=true;
            }

            if(Temp[tcAe2]!=fOmron->dSetValue[6][0])
            {
                HeaterSVLog(tcAe2, Temp[tcAe2]);
                bTempHasChange=true;
            }

            if(Temp[tcBe2]!=fOmron->dSetValue[6][1])
            {
                HeaterSVLog(tcBe2, Temp[tcBe2]);
                bTempHasChange=true;
            }

            if(Temp[tcAf2]!=fOmron->dSetValue[6][2])
            {
                HeaterSVLog(tcAf2, Temp[tcAf2]);
                bTempHasChange=true;
            }

            if(Temp[tcBf2]!=fOmron->dSetValue[6][3])
            {
                HeaterSVLog(tcBf2, Temp[tcBf2]);
                bTempHasChange=true;
            }

            if(Temp[tcAg2]!=fOmron->dSetValue[7][0])
            {
                HeaterSVLog(tcAg2, Temp[tcAg2]);
                bTempHasChange=true;
            }

            if(Temp[tcBg2]!=fOmron->dSetValue[7][1])
            {
                HeaterSVLog(tcBg2, Temp[tcBg2]);
                bTempHasChange=true;
            }

            if(Temp[tcAh2]!=fOmron->dSetValue[7][2])
            {
                HeaterSVLog(tcAh2, Temp[tcAh2]);
                bTempHasChange=true;
            }

            if(Temp[tcBh2]!=fOmron->dSetValue[7][3])
            {
                HeaterSVLog(tcBh2, Temp[tcBh2]);
                bTempHasChange=true;
            }
        }
    }
    else
    {
        fOmron->myPal[0][0]->edSV->Text=AnsiString(Temp[tcDUT1]);
        fOmron->myPal[0][1]->edSV->Text=AnsiString(Temp[tcDUT2]);
        fOmron->myPal[0][2]->edSV->Text=AnsiString(Temp[tcDUT3]);
        fOmron->myPal[0][3]->edSV->Text=AnsiString(Temp[tcDUT4]);

        if(Temp[tcDUT1]!=fOmron->dSetValue[0][0])
        {
            HeaterSVLog(tcDUT1, Temp[tcDUT1]);
            bTempHasChange=true;
        }

        if(Temp[tcDUT2]!=fOmron->dSetValue[0][1])
        {
            HeaterSVLog(tcDUT2, Temp[tcDUT2]);
            bTempHasChange=true;
        }

        if(Temp[tcDUT3]!=fOmron->dSetValue[0][2])
        {
            HeaterSVLog(tcDUT3, Temp[tcDUT3]);
            bTempHasChange=true;
        }

        if(Temp[tcDUT4]!=fOmron->dSetValue[0][3])
        {
            HeaterSVLog(tcDUT4, Temp[tcDUT4]);
            bTempHasChange=true;
        }
    }

    if(bTempHasChange)                                                          //有不一樣的話,就設定溫度到溫控器
    {
        fOmron->btSetSV->Click();
    }

    if(fOmron->bGetComponentFinish==false ||                                    //讀取資料
       fOmron->bResetCom==true)
    {
        for(Addr=tcAa1; Addr<=tcBd2; Addr++)
        {
            UN150Read[Addr]=0.0;
        }
    }
    else
    {
        if(USE_16_HEATER==eht16HeaterEJ1N || USE_16_HEATER==eht32HeaterEJ1N)    //Steven 20140923 : Index使用EJ1N版32組加熱器
        {
            if(IsNNMode()==NN_2Row)                                             //kevin 20190921 change add 32 site use two arm
            {
                if(TestIF_File.dSiteYPitch<50.0 && TestIF_File.bUse32Heater==false)
                {
                    if(TestIF.iTestMode==_16Site4X4)
                    {
                        UN150Read[tcAa1]=GetConvertTemp(tcAa1, fOmron->dTempValue[0][0]);
                        UN150Read[tcBa1]=GetConvertTemp(tcBa1, fOmron->dTempValue[0][1]);
                        UN150Read[tcAb1]=GetConvertTemp(tcAb1, fOmron->dTempValue[0][2]);
                        UN150Read[tcBb1]=GetConvertTemp(tcBb1, fOmron->dTempValue[0][3]);
                        UN150Read[tcAa2]=GetConvertTemp(tcAa2, fOmron->dTempValue[2][0]);
                        UN150Read[tcBa2]=GetConvertTemp(tcBa2, fOmron->dTempValue[2][1]);
                        UN150Read[tcAb2]=GetConvertTemp(tcAb2, fOmron->dTempValue[2][2]);
                        UN150Read[tcBb2]=GetConvertTemp(tcBb2, fOmron->dTempValue[2][3]);
                    }
                    else
                    {
                        UN150Read[tcAa1]=GetConvertTemp(tcAa1, fOmron->dTempValue[0][0]);
                        UN150Read[tcBa1]=GetConvertTemp(tcBa1, fOmron->dTempValue[0][1]);
                        UN150Read[tcAb1]=GetConvertTemp(tcAb1, fOmron->dTempValue[0][2]);
                        UN150Read[tcBb1]=GetConvertTemp(tcBb1, fOmron->dTempValue[0][3]);
                        UN150Read[tcAa2]=GetConvertTemp(tcAa2, fOmron->dTempValue[2][0]);
                        UN150Read[tcBa2]=GetConvertTemp(tcBa2, fOmron->dTempValue[2][1]);
                        UN150Read[tcAb2]=GetConvertTemp(tcAb2, fOmron->dTempValue[2][2]);
                        UN150Read[tcBb2]=GetConvertTemp(tcBb2, fOmron->dTempValue[2][3]);

                        UN150Read[tcAc1]=GetConvertTemp(tcAc1, fOmron->dTempValue[1][0]);
                        UN150Read[tcBc1]=GetConvertTemp(tcBc1, fOmron->dTempValue[1][1]);
                        UN150Read[tcAd1]=GetConvertTemp(tcAd1, fOmron->dTempValue[1][2]);
                        UN150Read[tcBd1]=GetConvertTemp(tcBd1, fOmron->dTempValue[1][3]);
                        UN150Read[tcAc2]=GetConvertTemp(tcAc2, fOmron->dTempValue[3][0]);
                        UN150Read[tcBc2]=GetConvertTemp(tcBc2, fOmron->dTempValue[3][1]);
                        UN150Read[tcAd2]=GetConvertTemp(tcAd2, fOmron->dTempValue[3][2]);
                        UN150Read[tcBd2]=GetConvertTemp(tcBd2, fOmron->dTempValue[3][3]);
                    }
                }
                else
                {
                    UN150Read[tcAa1]=GetConvertTemp(tcAa1, fOmron->dTempValue[0][0]);   //Steven 20120714 : 修正轉換函式
                    UN150Read[tcBa1]=GetConvertTemp(tcBa1, fOmron->dTempValue[0][1]);
                    UN150Read[tcAb1]=GetConvertTemp(tcAb1, fOmron->dTempValue[0][2]);
                    UN150Read[tcBb1]=GetConvertTemp(tcBb1, fOmron->dTempValue[0][3]);
                    UN150Read[tcAc1]=GetConvertTemp(tcAc1, fOmron->dTempValue[1][0]);
                    UN150Read[tcBc1]=GetConvertTemp(tcBc1, fOmron->dTempValue[1][1]);
                    UN150Read[tcAd1]=GetConvertTemp(tcAd1, fOmron->dTempValue[1][2]);
                    UN150Read[tcBd1]=GetConvertTemp(tcBd1, fOmron->dTempValue[1][3]);
                    UN150Read[tcAa2]=GetConvertTemp(tcAa2, fOmron->dTempValue[2][0]);
                    UN150Read[tcBa2]=GetConvertTemp(tcBa2, fOmron->dTempValue[2][1]);
                    UN150Read[tcAb2]=GetConvertTemp(tcAb2, fOmron->dTempValue[2][2]);
                    UN150Read[tcBb2]=GetConvertTemp(tcBb2, fOmron->dTempValue[2][3]);
                    UN150Read[tcAc2]=GetConvertTemp(tcAc2, fOmron->dTempValue[3][0]);
                    UN150Read[tcBc2]=GetConvertTemp(tcBc2, fOmron->dTempValue[3][1]);
                    UN150Read[tcAd2]=GetConvertTemp(tcAd2, fOmron->dTempValue[3][2]);
                    UN150Read[tcBd2]=GetConvertTemp(tcBd2, fOmron->dTempValue[3][3]);
                }
            }
            else
            {
                UN150Read[tcAa1]=GetConvertTemp(tcAa1, fOmron->dTempValue[0][0]);   //Steven 20120714 : 修正轉換函式
                UN150Read[tcBa1]=GetConvertTemp(tcBa1, fOmron->dTempValue[0][1]);
                UN150Read[tcAb1]=GetConvertTemp(tcAb1, fOmron->dTempValue[0][2]);
                UN150Read[tcBb1]=GetConvertTemp(tcBb1, fOmron->dTempValue[0][3]);
                UN150Read[tcAc1]=GetConvertTemp(tcAc1, fOmron->dTempValue[1][0]);
                UN150Read[tcBc1]=GetConvertTemp(tcBc1, fOmron->dTempValue[1][1]);
                UN150Read[tcAd1]=GetConvertTemp(tcAd1, fOmron->dTempValue[1][2]);
                UN150Read[tcBd1]=GetConvertTemp(tcBd1, fOmron->dTempValue[1][3]);
                UN150Read[tcAa2]=GetConvertTemp(tcAa2, fOmron->dTempValue[2][0]);
                UN150Read[tcBa2]=GetConvertTemp(tcBa2, fOmron->dTempValue[2][1]);
                UN150Read[tcAb2]=GetConvertTemp(tcAb2, fOmron->dTempValue[2][2]);
                UN150Read[tcBb2]=GetConvertTemp(tcBb2, fOmron->dTempValue[2][3]);
                UN150Read[tcAc2]=GetConvertTemp(tcAc2, fOmron->dTempValue[3][0]);
                UN150Read[tcBc2]=GetConvertTemp(tcBc2, fOmron->dTempValue[3][1]);
                UN150Read[tcAd2]=GetConvertTemp(tcAd2, fOmron->dTempValue[3][2]);
                UN150Read[tcBd2]=GetConvertTemp(tcBd2, fOmron->dTempValue[3][3]);
            }

            if(USE_16_HEATER==eht32HeaterEJ1N)                                  //Steven 20140923 : Index使用EJ1N版32組加熱器
            {
                UN150Read[tcAe1]=GetConvertTemp(tcAe1, fOmron->dTempValue[4][0]);
                UN150Read[tcBe1]=GetConvertTemp(tcBe1, fOmron->dTempValue[4][1]);
                UN150Read[tcAf1]=GetConvertTemp(tcAf1, fOmron->dTempValue[4][2]);
                UN150Read[tcBf1]=GetConvertTemp(tcBf1, fOmron->dTempValue[4][3]);
                UN150Read[tcAg1]=GetConvertTemp(tcAg1, fOmron->dTempValue[5][0]);
                UN150Read[tcBg1]=GetConvertTemp(tcBg1, fOmron->dTempValue[5][1]);
                UN150Read[tcAh1]=GetConvertTemp(tcAh1, fOmron->dTempValue[5][2]);
                UN150Read[tcBh1]=GetConvertTemp(tcBh1, fOmron->dTempValue[5][3]);
                UN150Read[tcAe2]=GetConvertTemp(tcAe2, fOmron->dTempValue[6][0]);
                UN150Read[tcBe2]=GetConvertTemp(tcBe2, fOmron->dTempValue[6][1]);
                UN150Read[tcAf2]=GetConvertTemp(tcAf2, fOmron->dTempValue[6][2]);
                UN150Read[tcBf2]=GetConvertTemp(tcBf2, fOmron->dTempValue[6][3]);
                UN150Read[tcAg2]=GetConvertTemp(tcAg2, fOmron->dTempValue[7][0]);
                UN150Read[tcBg2]=GetConvertTemp(tcBg2, fOmron->dTempValue[7][1]);
                UN150Read[tcAh2]=GetConvertTemp(tcAh2, fOmron->dTempValue[7][2]);
                UN150Read[tcBh2]=GetConvertTemp(tcBh2, fOmron->dTempValue[7][3]);
            }
        }
    }
#endif // TODO(W7-UI G26b)
}
//------------------------------------------------------------------------------
TQPF_Timer DoTemptureControlDelay;
void DoTemptureControl()
{
/*    static int Task=1;
    static int iChangeHotPlate=1;
    static double fHPTemp=0.0;

    double Temp;
    int i;

    switch(Task)
    {
        case 1:
            if(LastSet.iTemperature==Tempture_Ambient)
                Task=1000;
            else
                Task=5000;
            iChangeHotPlate=1;
            break;
        case 1000:  //常溫
            //SW[SwHeaterRelay].Off();                                              //關閉加熱//
            for(i=0; i<HSys.TempCtrl->GetTotalInstalledUnit(); i++)                 //取得溫控器數量//
            {
                if(HSys.TempCtrl->UnitHasInstall(i))                                //確認溫控器是否Intsall//
                    HSys.TempCtrl->WriteTargetConvertTemp(i, 0.0);                  //溫控器歸零
            }
            DoTemptureControlDelay.SetSecAndOn(0.1);
            Task=1100;
            break;
        case 1100:
            //SW[SwHeaterRelay].Off();
            for(i=0; i<HSys.TempCtrl->GetTotalInstalledUnit(); i++)
                if(HSys.TempCtrl->bMonitorSelect[i])
                    UN150Read[i]=HSys.TempCtrl->ReadCurrentTemp(i);

            Task=1200;
        case 1200:
            if(DoTemptureControlDelay.Off())
            {
                if(LastSet.iTemperature==Tempture_Hot) //只要有改就要馬上回到Task=1
                {
                    Task=1;
                    break;
                }
                Task=1000;
            }

            if(LastSet.iTemperature==Tempture_Hot)
                Task=1;
            break;
        case 5000:  //加熱
            //SW[SwHeaterRelay].On();                                             //打開加熱//
            for(i=0; i<HSys.TempCtrl->GetTotalInstalledUnit(); i++)
            {
                if(HSys.TempCtrl->UnitHasInstall(i))
                {
                    Temp=Temperature.fWorkTemperBase;
                    if(HSys.TempCtrl->bMonitorSelect[i]==false)
                        Temp=0;
#ifndef SOFT_SIMULTE
                    if(bHeaterDoorIsOpen[0] || bHeaterDoorIsOpen[1])
                    {
                        iChangeHotPlate=1;
                        //jou 2010-09-28 Chamber門打開時不關閉加熱電源，只將Temp=0，Hotplate除外。
                        if(CUSTOMER_CODE==CC_SPIL_SHINCHU || CUSTOMER_CODE==CC_AMKOR_China) //Steven 20101109
                        {
                            if(i>3)
                                Temp=0;
                        }
                        else
                        {
                            if(i>1)
                                Temp=0;
                        }
                    }

                    if(SW[SwHeaterRelay].Status()==false)
                    {
                        iChangeHotPlate=1;
                        Temp=0;
                    }
#endif
                    //Steven 20091022 Start
                    if(Temperature.iIndexHeatMode==HeadOnly && ( i==8 || i==9 ))  //Head Only (socket & chamber no use)
                        Temp=0;
                    else if(Temperature.iIndexHeatMode==ChamberOnly && i>=4 && i<=8 )  //Chamber Only (head 1 2 3 4 & socket no use)
                        Temp=0;
                    else if(Temperature.iIndexHeatMode==HeadChamber && i==8) //Head + Chamber (socket no use)
                        Temp=0;
                    else if(Temperature.iIndexHeatMode==SocketChamber && i>=4 && i<=7) //Socket + Chamber (head 1 2 3 4 no use)
                        Temp=0;
                    //Steven 20091022 End

                    if(Temperature.iIndexHeatMode!=HeadOnly &&
                       HotPlateForm.iPlateSelect==3 &&
                       LastSet.iTemperature==Tempture_Hot)
                    {
                        //jou 981224 start : 電流超過30A,需分段加熱
                        //jou 2010-02-24 start : 階梯式加熱
                        if((UN150Read[0]>(Temperature.fWorkTemperBase-2)) ||
                           (UN150Read[1]>(Temperature.fWorkTemperBase-2)))
                        {
                            iChangeHotPlate=0;
                        }

                        if(iChangeHotPlate==1)
                        {
                            if(i==0) // hotplate 1
                            {
                                if(UN150Read[0]>(UN150Read[1]+20))
                                {
                                    fHPTemp=UN150Read[0]+5.0;
                                    //Temp=0;
                                    Temp=fHPTemp;
                                    iChangeHotPlate=2;
                                }
                            }

                            if(i==1)
                                Temp=fHPTemp;
                                //Temp=0;
                        }
                        else if(iChangeHotPlate==2)
                        {
                            if(i==1) // hotplate 2
                            {
                                if((UN150Read[0]+20)<UN150Read[1])
                                {
                                    //Temp=0;
                                    fHPTemp=UN150Read[1]+5.0;
                                    Temp=fHPTemp;
                                    iChangeHotPlate=1;
                                }
                            }

                            if(i==0)
                                Temp=fHPTemp;
                                //Temp=0;
                        }
                        //jou 2010-02-24 end
                        //jou 981224 end
                    }

//                    if(Temp!=HSys.TempCtrl->OldTemp[i])
                    {
//                        HSys.TempCtrl->OldTemp[i]=Temp;
                        HSys.TempCtrl->WriteTargetConvertTemp(i, Temp);       //設定溫控器轉換後之預加熱之溫度f//
                    }
                }
            }
            DoTemptureControlDelay.SetSecAndOn(0.1);
            Task=5100;
        case 5100:
            for(i=0; i<HSys.TempCtrl->GetTotalInstalledUnit(); i++)
                if(HSys.TempCtrl->bMonitorSelect[i])
                    UN150Read[i]=HSys.TempCtrl->ReadCurrentTemp(i);

            Task=5200;
        case 5200:
            if(DoTemptureControlDelay.Off())
            {
                if(LastSet.iTemperature==Tempture_Ambient) //只要有改就要馬上回到Task=1
                {
                    Task=1;
                    break;
                }
                Task=5000;
            }

            if(LastSet.iTemperature==Tempture_Ambient)
                Task=1;
            break;
    }            */
}
//------------------------------------------------------------------------------
//Ifor 20160507 add ATC 4、8、32 Heat 溫度整合
double DOUN150ReadTemp(int Addr)
{
    double dTempShow=0.0;
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G27 -- golden :3965.
//  fTemp_Set (golden uTemp_Set.h TfTemp_Set form) and its iAddrToATC[] map have no
//  translated home -- whole-tree grep returns zero hits.  Cannot compile.
//  ACTIVE DEFAULT: iATCAddr=0.  It is read ONLY inside gate G28 below, so the value is inert.
//#if 0 // TODO(W7-UI G27): golden :3965   //AI(W906-S09-B1b) 20260930 (St02-E, claim): gate retired -- TfTemp_Set::iAddrToATC is forms/fTemp_Set.h:544; body = golden 906_0625_Steven bthermo.cpp:3965
    int iATCAddr=fTemp_Set->iAddrToATC[Addr];                                   //Steven 20241112 : 變更ATC Mapping方式
//#endif // TODO(W7-UI G27)   //AI(W906-S09-B1b) 20260930 (St02-E, claim): see :4539
    //int iATCAddr=0;                                                             //AI(W906-GA-BTHERMO) 20260807: G27 ACTIVE placeholder   //AI(W906-S09-B1b) 20260930 (St02-E, claim): placeholder retired -- the golden G27 line above is live now
    int iSiteMapping_40_QualSite[]   ={0,2,4,6,1,3,5,7,8,10,12,14,9,11,13,15,16,18,20,22,17,19,21,23,24,26,28,30,25,27,29,31,32,33,34,35,36,37,38,39};//Ztex 2023.04.19 Add HT-1032 TriTemp Function
    int Use_Heat_Count=0;                                                       //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    int iIndex = 0;                                                             //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        Use_Heat_Count = TriTemperature_TotalChannel;                           //讀取Site 溫度
    }
    else
    {
        Use_Heat_Count = iATC_Use_Heat_Count;
    }

//AI(W906-GA-BTHERMO) 20260807: iATCAddr / iSiteMapping_40_QualSite / iIndex are read ONLY inside
//  gates G27/G28.  Retained verbatim per golden and silenced, not deleted.
    (void)iATCAddr;
    (void)iSiteMapping_40_QualSite;
    (void)iIndex;

    switch(Use_Heat_Count)                                                      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        case 4:
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G28a -- golden :3981-4080.
//  ATC_InterfaceForm is acarry_shims.h::TATC_InterfaceFormShim, whose entire surface is
//  iATC_MODE_TYPE -- no IsConnect(), no dTC[], no dTC2[].  ATCInterfaceForm /
//  ATC_SYS_PAL / GetATCSiteNowTemperature(_Ref) are unported (see G25).  fWinway /
//  arrATC_Site (golden ATC/WinWaySetting.h) are unported.  Cannot compile.
//  ACTIVE DEFAULT: dTempShow stays 0.0 for the 4-heater ATC layouts.
#if 0 // TODO(W7-UI G28a): golden :3981-4080
            if(ATC_SYSTEM==eNewATCSystem)
            {
                if(ATC_InterfaceForm->IsConnect()==true)                        //Ifor 20160127判斷是否與ATC系統連線
                {
                    dTempShow=ATC_InterfaceForm->dTC[iATCAddr]/10.0;            //Ifor 20160127 取得 Arm 1 Aa~Ad 第一點溫度
                    if(Temperature.bUseReferTempSensor==true)                   //Ifor 20160629 ATC 第二點溫度分開儲存至其他位置
                        UN150ReadSecondpoint[Addr]=ATC_InterfaceForm->dTC2[iATCAddr]/10.0;
                }
                else
                {
                    #ifdef SOFT_SIMULTE
                        dTempShow=ATC_InterfaceForm->dTC[iATCAddr]/10.0;        //Ifor 20160127 取得 Arm 1 Aa~Ad 第一點溫度
                    #else
                        dTempShow=9999;
                    #endif
                }
            }
            else if(ATC_SYSTEM==eATCHonPrecType)
            {                                                                   //Ifor 20160610 修正未偵測到ATC程式版本時資料寫入999的問題
                if(ATCInterfaceForm->ATC_SYS_PAL[0]->LedATCConnect->Value==true)//Ifor 20160623 修正ATC2.0 顯示錯誤問題
                {
                    dTempShow=ATCInterfaceForm->GetATCSiteNowTemperature(iATCAddr);
                    if(Temperature.bUseReferTempSensor==true)                   //Ifor 20160629 ATC 第二點溫度分開儲存至其他位置
                        UN150ReadSecondpoint[Addr]=ATCInterfaceForm->GetATCSiteNowTemperature_Ref(iATCAddr);
                }
                else
                {
                    #ifdef SOFT_SIMULTE
                        dTempShow=ATCInterfaceForm->GetATCSiteNowTemperature(iATCAddr);
                    #else
                        dTempShow=9999;
                    #endif
                }
            }
            else if(ATC_SYSTEM==eWinWay && Temperature.bATCActiveCooling==true) //Jimmychiu 20210906
            {
                if(Addr==tcAa1 || Addr==tcHead1)
                {
                    if(bUT150Install[Addr] && fWinway->arrATC_Site[0]->bCommConnect==true)
                    {
                        dTempShow=fWinway->arrATC_Site[0]->GetPT();
                    }
                    else
                    {
                        #ifdef SOFT_SIMULTE
                            dTempShow=Addr;
                        #else
                            dTempShow=9999;
                        #endif
                    }
                }

                if(Addr==tcAb1 || Addr==tcHead2)
                {
                    if(bUT150Install[Addr] && fWinway->arrATC_Site[1]->bCommConnect==true)
                    {
                        dTempShow=fWinway->arrATC_Site[1]->GetPT();
                    }
                    else
                    {
                        #ifdef SOFT_SIMULTE
                            dTempShow=Addr;
                        #else
                            dTempShow=9999;
                        #endif
                    }
                }

                if(Addr==tcAa2 || Addr==tcHead3)
                {
                    if(bUT150Install[Addr] && fWinway->arrATC_Site[2]->bCommConnect==true)
                    {
                        dTempShow=fWinway->arrATC_Site[2]->GetPT();
                    }
                    else
                    {
                        #ifdef SOFT_SIMULTE
                            dTempShow=Addr;
                        #else
                            dTempShow=9999;
                        #endif
                    }
                }

                if(Addr==tcAb2 || Addr==tcHead4)
                {
                    if(bUT150Install[Addr] && fWinway->arrATC_Site[3]->bCommConnect==true)
                    {
                        dTempShow=fWinway->arrATC_Site[3]->GetPT();
                    }
                    else
                    {
                        #ifdef SOFT_SIMULTE
                            dTempShow=Addr;
                        #else
                            dTempShow=9999;
                        #endif
                    }
                }
            }
#endif // TODO(W7-UI G28a)
            break;
        case 8:
        case 16:
        case 24:
        case 32:
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G28b -- golden :4086-4103.
//  ATC_InterfaceForm->IsConnect() / dTC[] / dTC2[] -- see G28a.
//  ACTIVE DEFAULT: dTempShow stays 0.0 for the 8/16/24/32-heater ATC layout.
#if 0 // TODO(W7-UI G28b): golden :4086-4103
            if(ATC_SYSTEM==eNewATCSystem)                                       //Steven 20241112 : 變更ATC Mapping方式
            {
                if(ATC_InterfaceForm->IsConnect()==true)                        //Ifor 20160127判斷是否與ATC系統連線
                {
                    dTempShow=ATC_InterfaceForm->dTC[iATCAddr]/10.0;            //Ifor 20160127 取得 Arm 1 Aa~Ad 第一點溫度
                    if(Temperature.bUseReferTempSensor==true)                   //Ifor 20160629 ATC 第二點溫度分開儲存至其他位置
                        UN150ReadSecondpoint[Addr]=ATC_InterfaceForm->dTC2[iATCAddr]/10.0;
                }
                else
                {
                    bHasTjTemp=false;   //JerryYang 20251124 : Tj control吃不同的溫度range
                    #ifdef SOFT_SIMULTE
                        dTempShow=ATC_InterfaceForm->dTC[iATCAddr]/10.0;        //Ifor 20160127 取得 Arm 1 Aa~Ad 第一點溫度
                    #else
                        dTempShow=9999;
                    #endif
                }
            }
#endif // TODO(W7-UI G28b)
            break;
        case 40:
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G28c -- golden :4106-4188.
//  ATC_InterfaceForm->IsConnect() / dTC[] -- see G28a.
//  ACTIVE DEFAULT: dTempShow stays 0.0 for the 40-channel HT-1032 TriTemp layout.
#if 0 // TODO(W7-UI G28c): golden :4106-4188
            if(ATC_SYSTEM==eNewATCSystem)                                       //Ztex 2023.04.19 Add HT-1032 TriTemp Function
            {
                if(ATC_InterfaceForm->IsConnect()==true)                        //讀取Site 溫度
                {
                    if((Addr>=tcAa1 && Addr<=tcBd1) || (Addr>=tcAa2 && Addr<=tcBd2) ||
                       (Addr>=tcAe1 && Addr<=tcBh1) || (Addr>=tcAe2 && Addr<=tcBh2) ||
                       (Addr>=tcHotPlate1 && Addr<=tcShuttle2) ||
                       (Addr>=tcHotPlate3 && Addr<=tcShuttle4))
                    {
                        if(Addr>=tcAa1 && Addr<=tcBd1)
                        {
                            iIndex = Addr - tcAa1;
                        }
                        else if(Addr>=tcAe1 && Addr<=tcBh1)
                        {
                            iIndex = Addr - tcAe1 + 8;
                        }
                        else if(Addr>=tcAa2 && Addr<=tcBd2)
                        {
                            iIndex = Addr - tcAa2 + 16;
                        }
                        else if(Addr>=tcAe2 && Addr<=tcBh2)
                        {
                            iIndex = Addr - tcAe2 + 24;
                        }
                        else if(Addr==tcShuttle1)
                        {
                            iIndex = 32;
                        }
                        else if(Addr==tcShuttle2)
                        {
                            iIndex = 33;
                        }
                        else if(Addr==tcHotPlate1)
                        {
                            iIndex = 34;
                        }
                        else if(Addr==tcHotPlate2)
                        {
                            iIndex = 35;
                        }
                        else if(Addr==tcShuttle3)
                        {
                            iIndex = 36;
                        }
                        else if(Addr==tcShuttle4)
                        {
                            iIndex = 37;
                        }
                        else if(Addr==tcHotPlate3)
                        {
                            iIndex = 38;
                        }
                        else if(Addr==tcHotPlate4)
                        {
                            iIndex = 39;
                        }
                        dTempShow=ATC_InterfaceForm->dTC[iSiteMapping_40_QualSite[iIndex]]/100.0;
                    }
                    else
                    {
                        return 999;
                    }
                }
                else
                {
                    #ifdef SOFT_SIMULTE
                        if((Addr>=tcAa1 && Addr<=tcBd1) || (Addr>=tcAa2 && Addr<=tcBd2) ||
                           (Addr>=tcAe1 && Addr<=tcBh1) || (Addr>=tcAe2 && Addr<=tcBh2) ||
                           (Addr>=tcHotPlate1 && Addr<=tcShuttle2) ||
                           (Addr>=tcHotPlate3 && Addr<=tcShuttle4))
                        {
                            dTempShow=Temperature.fWorkTemperBase;
                        }
                    #else
                        dTempShow = -999;
                    #endif

//                    #ifdef DEBUG_HT1032
//                        dTempShow=Temperature.fWorkTemperBase;
//                    #endif
                }
            }
#endif // TODO(W7-UI G28c)
            break;
    }
    return dTempShow;
}
//------------------------------------------------------------------------------
bool bGet16HeaterUsedTo4Heater(int Addr)                                        //JerryYang 20160518 矽格湖口 柏均要求16組加熱器要能支援GPIB4組加熱器的指令
{
    if(TestIF.iTestMode==DualSite)                                              // dual site
    {
        int iDualsiteHead[4]={tcAa1, tcAb1, tcAa2, tcAb2};
        if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
           (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit)) //Sam 20220429 : 修正 1Cabcle layout 溫度回傳問題
        {
            iDualsiteHead[0]=tcAa1;
            iDualsiteHead[1]=tcBa1;
            iDualsiteHead[2]=tcAa2;
            iDualsiteHead[3]=tcBa2;
        }

        if(Addr==iDualsiteHead[0] || Addr==iDualsiteHead[2] || Addr==iDualsiteHead[1] || Addr==iDualsiteHead[3])
        {
            /*  //Sam 20220429 : 修正 1Cabcle layout 溫度回傳問題 Mark
            if(IniConfig.bSIGURDFunction && ((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) || (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))) //Sam 20210524 新增一條線版本
            {
                if(bTestSiteUse[0][0][1]==false && bTestSiteUse[0][0][0]==false &&
                  (Addr==tcAa1 || Addr==tcAb1))
                {
                    return false;
                }
                else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false &&
                  (Addr==tcAa2 || Addr==tcAb2))
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else
            */
            {
                if((bTestSiteUse[0][0][1]==false && Addr==iDualsiteHead[1]))
                {
                    return false;
                }
                else if(bTestSiteUse[1][0][1]==false && Addr==iDualsiteHead[3])
                {
                    return false;
                }
                else if(bTestSiteUse[0][0][0]==false && Addr==iDualsiteHead[0])
                {
                    return false;
                }
                else if(bTestSiteUse[1][0][0]==false && Addr==iDualsiteHead[2])
                {
                    return false;
                }
                else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)    //只使用ARM1
                {
                    if(Addr==iDualsiteHead[0] || Addr==iDualsiteHead[1])
                    {
                        return true;
                    }
                    else if(Addr==iDualsiteHead[2] || Addr==iDualsiteHead[3])
                    {
                        return false;
                    }
                }
                else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)    //只使用ARM2
                {
                    if(Addr==iDualsiteHead[2] || Addr==iDualsiteHead[3])
                    {
                        return true;
                    }
                    else if(Addr==iDualsiteHead[0] || Addr==iDualsiteHead[1])
                    {
                        return false;
                    }
                }
                else
                {
                    return true;
                }
            }
        }
        else
        {
            return false;
        }
    }
    else if(TestIF.iTestMode==SingleSite || TestIF.iTestMode==DualSite2x1)      // SingleSite
    {
        if(Addr==tcAa1 || Addr==tcAa2 || Addr==tcBa1 || Addr==tcBa2)
        {
            if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)                //只使用ARM1
            {
                if(Addr==tcAa1 || Addr==tcBa1)
                {
                    return true;
                }
                else if(Addr==tcAa2 || Addr==tcBa2)
                {
                    return false;
                }
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)           //只使用ARM2
            {
                if(Addr==tcAa2 || Addr==tcBa2)
                {
                    return true;
                }
                else if(Addr==tcAa1 || Addr==tcBa1)
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }
    else if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)
    {
        //Richard 20220809 SG湖口
        //==>
        if(USE_16_HEATER==eht4Heater ||
           ((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
           (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit)))
        {
            if(Addr==tcAa1 || Addr==tcAb1 || Addr==tcBa1 || Addr==tcBb1 ||
               Addr==tcAa2 || Addr==tcAb2 || Addr==tcBa2 || Addr==tcBb2)
            {
                if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false && (Addr==tcAa1))
                {
                    return false;
                }

                if(bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false && (Addr==tcBa1))
                {
                    return false;
                }
                else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false && (Addr==tcAa2))
                {
                    return false;
                }
                else if(bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false && (Addr==tcBa2))
                {
                    return false;
                }
                else if(bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false && (Addr==tcAb1))
                {
                    return false;
                }
                else if(bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false && (Addr==tcBb1))
                {
                    return false;
                }
                else if(bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false && (Addr==tcAb2))
                {
                    return false;
                }
                else if(bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false && (Addr==tcBb2))
                {
                    return false;
                }
                else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)       //只使用ARM1
                {
                    if(Addr==tcAa1 || Addr==tcBa1 || Addr==tcAb1 || Addr==tcBb1)
                    {
                        return true;
                    }
                    else if(Addr==tcAa2 || Addr==tcBa2 || Addr==tcAb2 || Addr==tcBb2)
                    {
                        return false;
                    }
                }
                else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)       //只使用ARM2
                {
                    if(Addr==tcAa2 || Addr==tcBa2 || Addr==tcAb2 || Addr==tcBb2)
                    {
                        return true;
                    }
                    else if(Addr==tcAa1 || Addr==tcBa1 || Addr==tcAb1 || Addr==tcBb1)
                    {
                        return false;
                    }
                }
                else
                {
                    return true;
                }
            }
        //<==
        }
    }
    else if(TestIF.iTestMode==QualSite2X2) // qual site 2X2
    {
        if(Addr==tcAa1 || Addr==tcAb1 || Addr==tcBa1 || Addr==tcBb1 ||
           Addr==tcAa2 || Addr==tcAb2 || Addr==tcBa2 || Addr==tcBb2)
        {
            if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][1][0]==false && (Addr==tcAa1 || Addr==tcBa1))
            {
                return false;
            }
            else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][1][0]==false && (Addr==tcAa2 || Addr==tcBa2))
            {
                return false;
            }
            else if(bTestSiteUse[0][0][1]==false && bTestSiteUse[0][1][1]==false && (Addr==tcAb1 || Addr==tcBb1))
            {
                return false;
            }
            else if(bTestSiteUse[1][0][1]==false && bTestSiteUse[1][1][1]==false && (Addr==tcAb2 || Addr==tcBb2))
            {
                return false;
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)    //只使用ARM1
            {
                if(Addr==tcAa1 || Addr==tcBa1 || Addr==tcAb1 || Addr==tcBb1)
                {
                    return true;
                }
                else if(Addr==tcAa2 || Addr==tcBa2 || Addr==tcAb2 || Addr==tcBb2)
                {
                    return false;
                }
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)    //只使用ARM2
            {
                if(Addr==tcAa2 || Addr==tcBa2 || Addr==tcAb2 || Addr==tcBb2)
                {
                    return true;
                }
                else if(Addr==tcAa1 || Addr==tcBa1 || Addr==tcAb1 || Addr==tcBb1)
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
        else
        {
            return true;
        }
    }
//    else if(TestIF.iTestMode==QualSite2X2BS) // Busy Shuttle qual site 2X2
//    {
//        if(Addr==tcHead2 || Addr==tcHead4)
//            return false;
//    }
    else if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4)        //Sam 20190226 : 16Site4X4 //8 Site 2X4
    {
        if(IniConfig.bSIGURDFunction && ((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) || (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit)))       //Sam 20210524 2x4 新增一條線版本
        {
            if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false &&
               bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][1]==false &&
               Addr==tcAa1)
            {
                return false;
            }
            else if(bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false &&
                    bTestSiteUse[0][1][2]==false && bTestSiteUse[0][1][3]==false &&
                    Addr==tcBa1)
            {
                return false;
            }
            else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false &&
                    bTestSiteUse[1][1][0]==false && bTestSiteUse[1][1][1]==false &&
                    Addr==tcAa2)
            {
                return false;
            }
            else if(bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false &&
                    bTestSiteUse[1][1][2]==false && bTestSiteUse[1][1][3]==false &&
                    Addr==tcBa2)
            {
                return false;
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)    //只使用ARM1
            {
                if(Addr==tcAa1 || Addr==tcBa1)
                {
                    return true;
                }
                else if(Addr==tcAa2 || Addr==tcBa2)
                {
                    return false;
                }
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)    //只使用ARM2
            {
                if(Addr==tcAa2 || Addr==tcBa2)
                {
                    return true;
                }
                else if(Addr==tcAa1 || Addr==tcBa1)
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
        else if(TestIF_File.bNS8000CS==true)     //JerryYang 20160613 使用NS8000也要能支援GPIB溫度指令
        {
            if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false &&
               bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][1]==false &&
               (Addr==tcAa1 || Addr==tcBa1))
            {
                return false;
            }
            else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false &&
                    bTestSiteUse[1][1][0]==false && bTestSiteUse[1][1][1]==false &&
                    (Addr==tcAa2 || Addr==tcBa2))
            {
                return false;
            }
            else if(bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false &&
                    bTestSiteUse[0][1][2]==false && bTestSiteUse[0][1][3]==false &&
                    (Addr==tcAc1 || Addr==tcBc1))
            {
                return false;
            }
            else if(bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false &&
                    bTestSiteUse[1][1][2]==false && bTestSiteUse[1][1][3]==false &&
                    (Addr==tcAc2 || Addr==tcBc2))
            {
                return false;
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)    //只使用ARM1
            {
                if(Addr==tcAa1 || Addr==tcAb1 || Addr==tcAc1 || Addr==tcAd1 || Addr==tcBa1 || Addr==tcBb1 || Addr==tcBc1 || Addr==tcBd1)
                {
                    return true;
                }
                else if(Addr==tcAa2 || Addr==tcAb2 || Addr==tcAc2 || Addr==tcAd2 || Addr==tcBa2 || Addr==tcBb2 || Addr==tcBc2 || Addr==tcBd2)
                {
                    return false;
                }
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)    //只使用ARM2
            {
                if(Addr==tcAa2 || Addr==tcAb2 || Addr==tcAc2 || Addr==tcAd2 || Addr==tcBa2 || Addr==tcBb2 || Addr==tcBc2 || Addr==tcBd2)
                {
                    return true;
                }
                else if(Addr==tcAa1 || Addr==tcAb1 || Addr==tcAc1 || Addr==tcAd1 || Addr==tcBa1 || Addr==tcBb1 || Addr==tcBc1 || Addr==tcBd1)
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
        else
        {
            if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false &&
               bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][1]==false &&
               (Addr==tcAa1 || Addr==tcAb1 || Addr==tcBa1 || Addr==tcBb1))
            {
                return false;
            }
            else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false &&
                    bTestSiteUse[1][1][0]==false && bTestSiteUse[1][1][1]==false &&
                    (Addr==tcAa2 || Addr==tcAb2 || Addr==tcBa2 || Addr==tcBb2))
            {
                return false;
            }
            else if(bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false &&
                    bTestSiteUse[0][1][2]==false && bTestSiteUse[0][1][3]==false &&
                    (Addr==tcAc1 || Addr==tcAd1 || Addr==tcBc1 || Addr==tcBd1))
            {
                return false;
            }
            else if(bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false &&
                    bTestSiteUse[1][1][2]==false && bTestSiteUse[1][1][3]==false &&
                    (Addr==tcAc2 || Addr==tcAd2 || Addr==tcBc2 || Addr==tcBd2))
            {
                return false;
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)    //只使用ARM1
            {
                if(Addr==tcAa1 || Addr==tcAb1 || Addr==tcAc1 || Addr==tcAd1 || Addr==tcBa1 || Addr==tcBb1 || Addr==tcBc1 || Addr==tcBd1)
                {
                    return true;
                }
                else if(Addr==tcAa2 || Addr==tcAb2 || Addr==tcAc2 || Addr==tcAd2 || Addr==tcBa2 || Addr==tcBb2 || Addr==tcBc2 || Addr==tcBd2)
                {
                    return false;
                }
            }
            else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)    //只使用ARM2
            {
                if(Addr==tcAa2 || Addr==tcAb2 || Addr==tcAc2 || Addr==tcAd2 || Addr==tcBa2 || Addr==tcBb2 || Addr==tcBc2 || Addr==tcBd2)
                {
                    return true;
                }
                else if(Addr==tcAa1 || Addr==tcAb1 || Addr==tcAc1 || Addr==tcAd1 || Addr==tcBa1 || Addr==tcBb1 || Addr==tcBc1 || Addr==tcBd1)
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
    }
    else if(TestIF.iTestMode==_6Site2X3)//6 Site 2X3
    {
        if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false &&
           bTestSiteUse[0][0][2]==false && (Addr==tcAa1 || Addr==tcAb1 || Addr==tcAc1))
        {
            return false;
        }
        else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false &&
                bTestSiteUse[1][0][2]==false && (Addr==tcAa2 || Addr==tcAb2 || Addr==tcAc2))
        {
            return false;
        }
        else if(bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][1]==false &&
                bTestSiteUse[0][1][2]==false && (Addr==tcBa1 || Addr==tcBb1 || Addr==tcBc1))
        {
            return false;
        }
        else if(bTestSiteUse[1][1][0]==false && bTestSiteUse[1][1][1]==false &&
                bTestSiteUse[1][1][2]==false && (Addr==tcBa2 || Addr==tcBb2 || Addr==tcBc2))
        {
            return false;
        }
        else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)    //只使用ARM1
        {
            if(Addr==tcAa1 || Addr==tcAb1 || Addr==tcAc1 || Addr==tcBa1 || Addr==tcBb1 || Addr==tcBc1)
            {
                return true;
            }
            else if(Addr==tcAa2 || Addr==tcAb2 || Addr==tcAc2 || Addr==tcBa2 || Addr==tcBb2 || Addr==tcBc2)
            {
                return false;
            }
        }
        else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)    //只使用ARM2
        {
            if(Addr==tcAa2 || Addr==tcAb2 || Addr==tcAc2 || Addr==tcBa2 || Addr==tcBb2 || Addr==tcBc2)
            {
                return true;
            }
            else if(Addr==tcAa1 || Addr==tcAb1 || Addr==tcAc1|| Addr==tcBa1 || Addr==tcBb1 || Addr==tcBc1)
            {
                return false;
            }
        }
        else
        {
            return true;
        }
    }
    else    //i6Site
    {
        if(bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][1]==false &&
           bTestSiteUse[0][0][2]==false && bTestSiteUse[0][0][3]==false &&
           bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][1]==false &&
           bTestSiteUse[0][1][2]==false && bTestSiteUse[0][1][3]==false &&
           (Addr==tcAa1 || Addr==tcAb1 || Addr==tcAc1 || Addr==tcAd1 ||
            Addr==tcBa1 || Addr==tcBb1 || Addr==tcBc1 || Addr==tcBd1))
        {
            return false;
        }
        else if(bTestSiteUse[1][0][0]==false && bTestSiteUse[1][0][1]==false &&
                bTestSiteUse[1][0][2]==false && bTestSiteUse[1][0][3]==false &&
                bTestSiteUse[1][1][0]==false && bTestSiteUse[1][1][1]==false &&
                bTestSiteUse[1][1][2]==false && bTestSiteUse[1][1][3]==false &&
               (Addr==tcAa2 || Addr==tcAb2 || Addr==tcAc2 || Addr==tcAd2 ||
                Addr==tcBa2 || Addr==tcBb2 || Addr==tcBc2 || Addr==tcBd2))
        {
            return false;
        }
        else if(bTestSiteUse[0][0][4]==false && bTestSiteUse[0][0][5]==false &&
                bTestSiteUse[0][0][6]==false && bTestSiteUse[0][0][7]==false &&
                bTestSiteUse[0][1][4]==false && bTestSiteUse[0][1][5]==false &&
                bTestSiteUse[0][1][6]==false && bTestSiteUse[0][1][7]==false &&
               (Addr==tcAe1 || Addr==tcAf1 || Addr==tcAg1 || Addr==tcAh1 ||
                Addr==tcBe1 || Addr==tcBf1 || Addr==tcBg1 || Addr==tcBh1))
        {
            return false;
        }
        else if(bTestSiteUse[1][0][4]==false && bTestSiteUse[1][0][5]==false &&
                bTestSiteUse[1][0][6]==false && bTestSiteUse[1][0][7]==false &&
                bTestSiteUse[1][1][4]==false && bTestSiteUse[1][1][5]==false &&
                bTestSiteUse[1][1][6]==false && bTestSiteUse[1][1][7]==false &&
               (Addr==tcAe2 || Addr==tcAf2 || Addr==tcAg2 || Addr==tcAh2 ||
                Addr==tcBe2 || Addr==tcBf2 || Addr==tcBg2 || Addr==tcBh2))
        {
            return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
void DoSetSVOfDTME08()                                                          //JimmyChiu 20210923 : DTME08溫控器
{
    int Addr=0;
    double Temp[tcTotalCount]={0.0};
    double dWorkTemp=0.0;
    static bool bHasErrorSet=false;                                             //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20
    if(InitialOK==false)                                                        //Steven 20110809
        return;

//AI(W906-GA-BTHERMO) 20260807: bHasErrorSet is read ONLY inside gate G19c.  Retained verbatim
//  per golden and silenced rather than deleted; Temp[] likewise (only gates
//  G19c/G29 read it).
    (void)bHasErrorSet;
    (void)Temp;

    for(int i=0; i<INDEX_HEAT_COUNT; i++)
    {
        Addr=iTempCode[i];
        if(LastSet.iTemperature==Tempture_Hot ||
           LastSet.iTemperature==Tempture_AmbientHot)                           //kevin 20180628 add 恆溫控制
        {
            if(bUT150Install[Addr]==false)
            {
                Temp[Addr]=0.0;
            }
            else
            {
                dWorkTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                Temp[Addr]=GetFactSetTemp(Addr, dWorkTemp);                     //取得要設定至溫控器的溫度
            }

            if(IniConfig.bL19OpenHeatDoorgiveupchamberhot==false)               //kevin 20170520 (wei) 開chambo門只有不加熱chambo
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W7 G12/G13 -- golden :4728, the same two dangling externs.
                if(BTHERMO_G12_DOCLOSEHEADTERDELAY_OFF && (BTHERMO_G13_HEATERDOOR_IS_OPEN(0) || BTHERMO_G13_HEATERDOOR_IS_OPEN(1)))
                {
                    Temp[Addr]=0.0;
                }
            }
            #ifndef SOFT_SIMULTE
            if(SW[SwHeaterRelay].Status()==false)
            {
                Temp[Addr]=0.0;
            }
            #endif
        }
        else
        {
            Temp[Addr]=0.0;
        }

        if(Temperature.iIndexHeatMode==ChamberOnly || Temperature.iIndexHeatMode==SocketChamber)     //Chamber Only (head 1 2 3 4 & socket no use)
        {
            Temp[Addr]=0.0;
        }

//AI(W906-GA-BTHERMO) 20260807: GATE W7 G19c -- golden :4750-4764.
//  FormHS->CheckTempOffset(): golden HS_Function.h:93 (TFormHS *FormHS).  The only
//  FormHS in this tree is Automation/SCK_ART_Remainder.h's TU-local stand-in, whose
//  entire surface is UpDataToServerByFTP (see its own gate #17 note) -- no
//  CheckTempOffset -- and HS_Function.{h,cpp} is unported.  Cannot compile.
//  ACTIVE DEFAULT: same as G19a, DTME08 SV path.
#if 0 // TODO(W7 G19c): golden :4750-4764
        if(FormHS->CheckTempOffset(Addr, Temp[Addr]))                           //設定溫度大於機台能力
        {                                                                       //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20
            if(bHasErrorSet==false)                                             //判斷有無Alarm過
            {
                FormHS->CheckTempOffset(0, 0, true);                            //Alarm 溫度設定錯誤
                tEJ1NAlarmTimer.SetSecAndOn(20);
                bHasErrorSet=true;                                              //設定Alarm旗標
            }
            else
            {
                if(tEJ1NAlarmTimer.Off())
                    bHasErrorSet=false;                                         //取消Alarm旗標
            }
            Temp[Addr]=Temperature.fWorkTemperBase;                             //設定溫度超過機台極限改寫工作溫度給溫控器(不包含Offset)
        }
#endif // TODO(W7 G19c)

        if(USE_16_HEATER==eht16HeaterDTME08 ||                                  //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
           USE_16_HEATER==eht32HeaterDTME08)                                    //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
        {
            if(USE_16_HEATER==eht16HeaterDTME08 && (Addr>=tcAe1))
            {
                //pass
            }
            else
            {
//AI(W906-GA-BTHERMO) 20260807: GATE W7-UI G29 -- golden :4775-4776.
//  frmDTME08 (golden EJ1N/fDTME08.h) with SetSettingSV() and GetPalGroup()->GetPV()
//  have no translated home -- whole-tree grep returns zero hits outside generated
//  dfm2rc layout string data.  Cannot compile.
//  (history) ACTIVE DEFAULT was: no SV is written and UN150Read[Addr] is left untouched for the DTME08 layout.
#if 1 // was: #if 0 // TODO(W7-UI G29): golden :4775-4776 -- opened AI(W906-I03) 20261002 (Ifor01), I-03 phase 2: frmDTME08 (forms/fDTME08.cpp) fills its 32 panels at boot on a DTME08 machine (W906_DTME08_CreateFormBoot), the only case this branch runs
                frmDTME08->SetSettingSV(i, Temp[Addr]);                         //write sv    SetSettingSV
                UN150Read[Addr]=GetConvertTemp(Addr, frmDTME08->GetPalGroup(i)->GetPV());   //read pv //Ifor 20241202 : 修正DTME溫控器溫度顯示異常問題
#endif // TODO(W7-UI G29)
            }
        }
    }
}
//------------------------------------------------------------------------------
void CheckLBTemp()                                                              //JerryYang 20220923 : LB吹氣function
{
    static int iStartTick=0, iEndTick=0;
    static bool bFirstin=true;

    AnsiString Str="";

    if(Temperature.bLBCoolingAirOn==false)
        return;

    if(bFirstin==true)
    {
        bFirstin=false;
        iStartTick=GetTickCount();
    }
    else
    {
        if(UN150Read[tcLB]>Temperature.dLBAirOnTemp)
        {
            SW[SwLBAir].On();            //Air On
            iEndTick=GetTickCount();
            if(((iEndTick-iStartTick)/1000)>(Temperature.iLBTempAlmInterval))
            {
                bFirstin=true;
                Str.sprintf("LB temperature OverHigh Over %d secs", Temperature.iLBTempAlmInterval);
                ShowMyMessage(Str);
                iStartTick=GetTickCount();
            }
        }
        else
        {
            //Air Off
            SW[SwLBAir].Off();            //Air On
            iStartTick=GetTickCount();
        }
    }
}
//------------------------------------------------------------------------------

