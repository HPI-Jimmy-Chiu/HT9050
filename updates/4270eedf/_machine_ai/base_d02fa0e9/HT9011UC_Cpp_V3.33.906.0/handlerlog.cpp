// =============================================================================
//  handlerlog.cpp  --  TMyLog: generic form-change logger + Site-Status /
//                      Any-Name change loggers.
//
//  Faithful translation of golden handlerlog.cpp (553 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3, group "misc_a".
//
//  ROLE: see handlerlog.h for the full description.  In short: TMyLog snap-
//  shots then diffs either (a) an arbitrary VCL control tree's leaf values
//  (Init_Log/Comp_Log/Do_Log -> Control_Log_Value -> Compare_Diff -> Save_Log
//  and, in the SPIL branch, also slEventLog/SaveEventLogInfo), or (b) the
//  fixed Site On/Off + Site-Mapping + Shuttle-Select + Use-Suck-Mode state
//  (Save_SiteStatusLog, no control-tree walk), or (c) a single named value
//  (Save_AnyName).
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-substrate
//  -----------------------------------------------
//   ACTIVE (faithful, real body -- all 11 golden functions):
//     TMyLog::TMyLog             golden :22-28
//     TMyLog::Save_Log           golden :30-97   (ONE statement gated, see GATE (1))
//     TMyLog::Save_SiteStatusLog golden :99-270  (ONE block gated x2 sites, see GATE (2))
//     TMyLog::Save_AnyName       golden :272-336
//     TMyLog::Find_Parent        golden :338-346
//     TMyLog::Init_Log           golden :348-366
//     TMyLog::Comp_Log           golden :368-375
//     TMyLog::Control_Log_Value  golden :377-458 (map-key statements gated x2 sites, see GATE (3))
//     TMyLog::Do_Log             golden :460-487
//     TMyLog::Compare_Diff       golden :489-543 (one VCL-idiom substitution, see below)
//     Space_Num (free function)  golden :545-552
//   SATISFIED-BY-SUBSTRATE (already real in this tree, nothing shimmed here):
//     MyForceDirectories (common.h:341), DirectoryExists/ExtractFilePath/Now/
//     DecodeDate/DateTimeToStr/IncludeTrailingBackslash (vclcompat, via the
//     umbrella), IntToStr (vclcompat), TestSocket (aHotPlateSubstrate.h:635,
//     a TMyKitSuck -- ->iShtRow/->iShtCol only touched here), LastSet.
//     bUseTestSocket (LastSet.h), iCloseSiteMap/IndexSuckName/
//     iRecordSiteMapOrder/iOldShuttleMode/iOldSelShuttle/iUseSuckModeBackup/
//     asHandlingMode (cmydef.h), TestIF_File.iSiteMap/iShuttleMode/
//     iShuttle_Sel/iUseSuckMode/iTestMode (cprod.h), DualSite/QualSite1X4/
//     QualSite2X2 (MachineType.h), fSetup->fShow (forms/fSetup.h -- golden
//     csetup.h:261), slEventLog/SaveEventLog/GetTimeInfo/SystemYear.../
//     IniConfig.bSPILFunction (cmydef.h/cpublic.h/Config.h),
//     SaveEventLogInfo/NewRecordProcess (cMyDB.h -- NewRecordProcess's real
//     body is acatchtray_shims.cpp:132; this file therefore includes cMyDB.h,
//     NOT acatchtray_shims.h, because the two headers' NewRecordProcess
//     declarations carry DIFFERENT default-argument spellings for the same
//     parameters -- including both in one TU would be a hard
//     "redefinition of default argument" error).
//
//  GATE REGISTER -- three gates (one call-level, two member-access-level).
//  ---------------------------------------------------------------------------
//   (1) Application->ExeName   golden :43 (`ExtractFilePath(Application->
//       ExeName)`, inside Save_Log's `if(sFolder=="")` default-path branch).
//       Grepped the whole port tree 20260807: TApplication (the VCL
//       singleton) has NO port anywhere -- confirmed by TWO independent
//       pre-existing gates for the exact same missing surface: cpublic.cpp
//       :2147 (`#if 0 // TODO(GA1-B3): blocked by Application->ExeName`) and
//       database.cpp:401 (its own independent GA1-B6 finding). Handled with
//       the SAME #if 0/#else idiom, golden's line kept verbatim in the #if 0
//       arm.
//       FAITHFUL DEFAULT: `AnsiString("")` in place of
//       `ExtractFilePath(Application->ExeName)` -- i.e. `sFolder` becomes the
//       relative path "Log\" under the process's current working directory
//       instead of "<exe dir>\Log\". This is the most neutral substitute
//       available (not a new invented literal): it is exactly what golden's
//       OWN expression would degrade to if ExtractFilePath were handed an
//       empty string.
//       BEHAVIOUR DELTA: on a real machine, Save_Log's ini log lands in
//       "<HT9045.exe dir>\Log\"; here (until Application->ExeName lands) it
//       lands in ".\Log\" under whatever directory the process was launched
//       from.  Save_Log is one gated expression among 11 real functions in
//       this file; it is not itself called by anything in this wave's scope.
//   (2) fSetup->Panel1->Caption   golden :216-221 (two statements inside
//       Save_SiteStatusLog's `if(fSetup->fShow)` branch: the `asRecordBuffer.
//       sprintf(...)` diff-message build and the `asHandlingMode=...`
//       snapshot update, both keyed on comparing `asHandlingMode!=fSetup->
//       Panel1->Caption`).  forms/fSetup.h (this tree's real, pre-existing,
//       out-of-this-wave's-write-scope TfSetup facade) does not carry a
//       Panel1 member -- grepped forms/fSetup.h 20260807: zero hits for
//       "Panel1" -- only the members prior consumers of that facade actually
//       touch are present. Handled with the #if 0/#else idiom, golden's two
//       statements plus their guarding `if` kept verbatim in the #if 0 arm.
//       FAITHFUL DEFAULT: no-op (the "Socket Count Value Change" MES2104
//       event is simply not raised, and asHandlingMode is not advanced).
//       BEHAVIOUR DELTA: this ONE sub-branch of Save_SiteStatusLog's SPIL-off
//       diff logging never fires; every other diff this function raises
//       (Test Site Assign / Head Select / Use Pick Unit Count, golden
//       :148-214) is UNAFFECTED and stays fully active.
//   (3) ->Name (+ ->Caption on TRadioGroup)   golden :393-397 (RadGup) and
//       :449-455 (generic child P) inside Control_Log_Value.  Golden's real
//       VCL TRadioGroup/TCheckBox/TRadioButton/TComboBox/TCustomEdit (every
//       TWinControl-derived leaf, in fact) always carries ->Name (a
//       TComponent property).  This tree's vclcompat/Controls.h -- pre-
//       existing, out of this wave's write scope -- was built as a
//       deliberately "minimal surface" (its own banner: "each class holds
//       ONLY the members golden's [prior] cast-dispatch bodies touch") and
//       carries ->Name on NONE of its stock widget stand-ins; TRadioGroup
//       additionally carries no ->Caption either. Extending that shared
//       file for this one new consumer is out of this wave's authorized
//       scope (two-vs-three-file wave, rule 7). Handled with the #if 0/#else
//       idiom at both sites, golden's statements kept verbatim in the #if 0
//       arm; the surrounding sValue/sCaption extraction (PScrollBar/
//       PTrackBar/PCheckBox/PRadioButton/PTComboBox/PTEdit/PTLEdit, and the
//       recursive `Control_Log_Value((TWinControl*)P, bInit)` child walk)
//       stays ACTIVE and real either side of the gate.
//       FAITHFUL DEFAULT: no-op (no map entry is written, no diff line is
//       ever recorded for a control reached this way).
//       BEHAVIOUR DELTA: since offline this wave's TWinControl/TForm carry
//       ZERO real children on ANY object ever constructed (ControlCount==0,
//       Controls empty -- see handlerlog.h), this loop body's identification
//       step was ALREADY unreachable dead code before the gate; the gate
//       documents the reason it could not even be left as literal dead code
//       (a hard compile error, not a runtime no-op) and removes exactly that
//       compile blocker, changing nothing observable.  Retire the moment a
//       future wave gives vclcompat's stock widgets a ->Name property.
//
//  ONE VCL-IDIOM SUBSTITUTION (NOT a gate -- a real, working translation)
//  ---------------------------------------------------------------------------
//  Golden's `AnsiString(Now())` (golden :156x2, :157, :297, :503) relies on
//  BCB6's implicit `TDateTime -> AnsiString` conversion operator (real VCL
//  TDateTime formats itself via DateTimeToStr). vclcompat/TDateTime.h's
//  TDateTime intentionally offers only `operator double()` (for elapsed-time
//  arithmetic, its dominant use tree-wide) -- so `AnsiString(Now())` here
//  would silently resolve through that operator into `AnsiString(double)`,
//  printing the raw OLE-Automation day-count ("46145.79...") instead of a
//  timestamp. That is not a faithful preservation of golden's behaviour, it
//  is a translation defect the substrate gap would introduce -- so every
//  `AnsiString(Now())` below is written as `DateTimeToStr(Now())` (a real,
//  already-available vclcompat free function, brought in by the umbrella)
//  instead, which is the behaviourally-equivalent call BCB6 itself would
//  have dispatched to under the hood.  No golden call site's ARGUMENT COUNT,
//  ORDER, or intent changes -- only the spelling of one VCL implicit
//  conversion this substrate does not implement.
//
//  VCL/Borland conversions: #pragma hdrstop dropped (golden :2). golden
//  :14 "Main.h" / :15 "atester.h" / :17 "uLotInfo.h" dropped -- grepped this
//  file's own body for any symbol unique to those three headers: zero hits
//  (fMain/TestIF/fLotInfo are never referenced; TestIF_File comes from
//  cprod.h directly, which this file already includes for other reasons).
//  golden :13 "cSetUp.h" maps to this tree's forms/fSetup.h (the `fSetup`
//  global + TfSetup facade). `#include <tchar.h>` and `#include <shellapi.h>`
//  added explicitly (golden got _tcslen/_tcscpy/TCHAR/SHFileOperation/
//  SHFILEOPSTRUCT transitively via <vcl.h>; this tree's MachineDefine.h
//  brings in <windows.h> but not <tchar.h>/<shellapi.h>, and this is the
//  first ported file to need either). `IncludeTrailingPathDelimiter` is
//  supplied as a small per-TU `static inline` forwarding wrapper over
//  vclcompat's `IncludeTrailingBackslash` -- the SAME established, repeated-
//  independently-4-times convention documented at cpublic.cpp:136,
//  Interface/TesterTCP.cpp:98, SECSGEM/uHGemClass.cpp:698 and Automation/
//  SCK_ART_Remainder.h:1193-1198 (none of those wired it into vclcompat
//  itself; each TU adds its own copy).  golden's own bare, return-value-
//  discarded call `IncludeTrailingPathDelimiter(sFolder);` (golden :45,
//  :114, :287 -- NOT `sFolder=IncludeTrailingPathDelimiter(sFolder);`) is
//  kept EXACTLY as written: a real golden no-op (the computed trailing-slash
//  string is thrown away), reproduced faithfully, not "fixed".
//  No __fastcall/__property/__published in the translated bodies (golden
//  :22's own `_fastcall` -- missing its leading `__`, golden's own typo --
//  dropped like every other calling convention keyword in this tree).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std; also <windows.h>)
//------------------------------------------------------------------------------
// Version & Date  :V1.02 2010/07/08
// Programmer      :Landam
// Objective     :Opration Log
//------------------------------------------------------------------------------

#include "handlerlog.h"             // this unit's own contract (TMyLog + the 4 new TWinControl-family stand-ins)
#include "cprod.h"                  // TestIF_File (SYSTEM_TEST_IF: iSiteMap/iShuttleMode/iShuttle_Sel/iUseSuckMode/iTestMode)
#include "cMyDB.h"                  // SaveEventLogInfo, NewRecordProcess (real body acatchtray_shims.cpp:132) -- see file head for why NOT acatchtray_shims.h
#include "cmydef.h"                 // slEventLog / SaveEventLog, iCloseSiteMap, IndexSuckName, iRecordSiteMapOrder, iOldShuttleMode, iOldSelShuttle, iUseSuckModeBackup, asHandlingMode
#include "Public/MyStringList.h"    // TMyStringList (real class -- cmydef.h only forward-declares it; slEventLog->AddTextWithLineNo/AddTextWithDateTime need the complete type)
#include "aHotPlateSubstrate.h"     // TestSocket (TMyKitSuck: ->iShtRow/->iShtCol)
#include "LastSet.h"                // LastSet (LAST_GENERAL_SET::bUseTestSocket[2][4][8])
#include "common.h"                 // MyForceDirectories
#include "forms/fSetup.h"           // fSetup (TfSetup facade: ->fShow) -- golden csetup.h
#include "MachineType.h"            // DualSite / QualSite1X4 / QualSite2X2 (eTestMode)
#include <tchar.h>                  // _tcslen / _tcscpy (golden used them via <vcl.h>)
#include <shellapi.h>               // SHFileOperation / SHFILEOPSTRUCT / FO_DELETE / FOF_SILENT / FOF_NOCONFIRMATION
bool W906_FormShowing(const char* goldenForm, bool member);   //AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表的單一函式（與 W906FormShowing.h／csystem.h:440 同一個宣告，本體 csystem.cpp:30049）；本檔不 include csystem.h ⇒ 宣告放在這個原本的空白行，不移動行號
// ---------------------------------------------------------------------------
//  Local shim for a SysUtils helper not yet in vclcompat -- same established
//  convention as cpublic.cpp:136 / Interface/TesterTCP.cpp:98 /
//  SECSGEM/uHGemClass.cpp:698 / Automation/SCK_ART_Remainder.h:1193-1198.
// ---------------------------------------------------------------------------
static inline AnsiString IncludeTrailingPathDelimiter(const AnsiString& p)
{
    return IncludeTrailingBackslash(p);
}

AnsiString Space_Num(AnsiString S, int n);
//------------------------------------------------------------------------------
TMyLog::TMyLog()
{
    fName="";
    sUser="";
    sFolder="";
    Delimiter="\t";
}
//------------------------------------------------------------------------------
void TMyLog::Save_Log()
{
    TDateTime tt= Now();
    Word SystemYear, SystemMonth, SystemDate;
    DecodeDate(tt, SystemYear, SystemMonth, SystemDate);
    AnsiString str1, str2;

    str1.sprintf("%04d_%02d\\", SystemYear, SystemMonth);
    str2.sprintf("log_%04d_%02d_%02d.ini", SystemYear, SystemMonth, SystemDate);

    FILE * pFile;
    AnsiString fp="", s_cFolder;
    if(sFolder=="")
#if 0 // GATE (1): golden :43 Application->ExeName -- see file-head GATE REGISTER for the full grep/rationale/behaviour-delta.
        sFolder.sprintf("%sLog\\", ExtractFilePath(Application->ExeName));
#else
        sFolder.sprintf("%sLog\\", AnsiString(""));                            // GATE (1) faithful default: relative ".\Log\" (see file head)
#endif

    IncludeTrailingPathDelimiter(sFolder);
    s_cFolder=sFolder+str1;
    MyForceDirectories(s_cFolder);

    fp=s_cFolder+str2;
    pFile=fopen(fp.c_str() ,"a");
    if(pFile!=NULL)
    {
        AnsiString tmps;
        for(unsigned i=0; i<loglist.size(); ++i)
        {
            tmps=loglist[i]+"\n";
            fputs(tmps.c_str(), pFile);
            if(i==loglist.size()-1)
                fputs("\n", pFile);
        }
        fclose(pFile);
    }

    //----------------clear_log folder  ----------------------------------
    if(SystemMonth<=2)
    {
        SystemMonth=SystemMonth + 10;
        SystemYear--;
    }
    else
    {
        SystemMonth=SystemMonth-2;
    }

    str1.sprintf("%04d_%02d", SystemYear, SystemMonth);
    s_cFolder=sFolder+str1;

    if((DirectoryExists(s_cFolder)))
    {
        int len = _tcslen(s_cFolder.c_str());
        TCHAR *pszFrom = new TCHAR[len+2];
        _tcscpy(pszFrom, s_cFolder.c_str());
        pszFrom[len] = 0;
        pszFrom[len+1] = 0;

        SHFILEOPSTRUCT      opStr;

        opStr.hwnd          = NULL;
        opStr.pTo           = NULL;
        opStr.wFunc         = FO_DELETE;
        opStr.fFlags        = FOF_SILENT | FOF_NOCONFIRMATION;
        opStr.pFrom         = pszFrom;
        SHFileOperation(&opStr);
        delete [] pszFrom;
    }
    //-------------end of clear log---------------------------------------------
}
//------------------------------------------------------------------------------
void TMyLog::Save_SiteStatusLog()                                               //JerryYang 20160617 記錄開關site的狀態
{
    TDateTime tt=Now();
    Word SystemYear, SystemMonth, SystemDate;
    DecodeDate(tt, SystemYear, SystemMonth, SystemDate);
    AnsiString str1, str2;

    str1.sprintf("%04d_%02d\\",SystemYear, SystemMonth);
    str2.sprintf("log_%04d_%02d_%02d.ini", SystemYear, SystemMonth, SystemDate);

    FILE * pFile;
    AnsiString fp="", s_cFolder;
    if(sFolder=="")
        sFolder.sprintf("%s", "D:\\HT9045_Log\\ChangeLog\\");

    IncludeTrailingPathDelimiter(sFolder);
    s_cFolder=sFolder+str1;
    MyForceDirectories(s_cFolder);

    fp=s_cFolder+str2;
    pFile=fopen(fp.c_str(), "a");
    if(pFile!=NULL)
    {
        AnsiString tmps, tmps1, tmps2, tmps3, tmps4, Atmps, Atmps1, Atmps2, Atmps3;
        AnsiString asRecordBuffer;

        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(i==TestSocket.iShtRow-1 && j==TestSocket.iShtCol-1)
                {
                    tmps1=tmps1+IntToStr(int(LastSet.bUseTestSocket[0][i][j]));
                }
                else
                {
                    tmps1=tmps1+IntToStr(int(LastSet.bUseTestSocket[0][i][j]))+",";
                }

                if(i==TestSocket.iShtRow-1 && j==TestSocket.iShtCol-1)
                    tmps2=tmps2+IntToStr(int(LastSet.bUseTestSocket[1][i][j]));
                else
                    tmps2=tmps2+IntToStr(int(LastSet.bUseTestSocket[1][i][j]))+",";

                tmps3=tmps3+IntToStr(int(iCloseSiteMap[0][i][j]))+",";

                Atmps1=Atmps1+IntToStr(int(TestIF_File.iSiteMap[i][j]))+",";
                Atmps2=Atmps2+IntToStr(int(iRecordSiteMapOrder[i][j]))+",";     //kevin 20161003

                if(iRecordSiteMapOrder[i][j]!=TestIF_File.iSiteMap[i][j])       //Ifor 20161220 (Steven) add Site Mapping change Even
                {
                    asRecordBuffer.sprintf("Site %s Value Change %s -> %s", IndexSuckName[i][j], IntToStr(int(iRecordSiteMapOrder[i][j])), IntToStr(int(TestIF_File.iSiteMap[i][j])));
                    NewRecordProcess("MES2105", "Site Mapping", asRecordBuffer);//Ifor 20170106 需再確認MES號碼
                    iRecordSiteMapOrder[i][j] = TestIF_File.iSiteMap[i][j];     //Ifor 20161220 Site Mapping 變更後需存檔避免紀錄錯誤
                }
            }
        }
        tmps.sprintf("\n(Null)\t\t\tfSiteStatusArm1\t\t\t%s\t\t%s\n(Null)\t\t\tfSiteStatusArm2\t\t\t%s\t\t%s\n", tmps1, DateTimeToStr(Now()), tmps2, DateTimeToStr(Now()));
        Atmps.sprintf("\n(Null)\t\t\tTest Site Assign ->\t\%s\t\t%s\n", Atmps1, DateTimeToStr(Now()));
        tmps4.sprintf("CloseOpenSite : %s  -> %s\n", tmps3, tmps1);

        fputs(tmps.c_str(), pFile);
        fputs(Atmps.c_str(), pFile);
        fclose(pFile);
        Atmps3.sprintf("\t\%s\ -> %s\n", Atmps2, Atmps1);

        if(W906_FormShowing("fSetup", fSetup->fShow))  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批1：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
        {
            if(Atmps2!=Atmps1)                                                  //Ifor 20161116 Site Map 未修改 不紀錄
                NewRecordProcess("MES2106", "Test Site Assign", Atmps3);

            if(iOldShuttleMode!=TestIF_File.iShuttleMode ||                     //Ifor 20161220 (Steven) add Head Select Mode Even
               iOldSelShuttle!=TestIF_File.iShuttle_Sel)
            {
                if(iOldShuttleMode==TestIF_File.iShuttleMode &&
                   iOldSelShuttle!=TestIF_File.iShuttle_Sel)
                {
                    if(TestIF_File.iShuttle_Sel==0)
                        asRecordBuffer.sprintf("Shuttle 2 only -> Shuttle 1 only");
                    else
                        asRecordBuffer.sprintf("Shuttle 1 only -> Shuttle 2 only");
                }
                else if(iOldShuttleMode!=TestIF_File.iShuttleMode)
                {
                    if(TestIF_File.iShuttleMode==0)
                    {
                        if(iOldSelShuttle==0)
                            asRecordBuffer.sprintf("Shuttle 1 only -> Normal");
                        else
                            asRecordBuffer.sprintf("Shuttle 2 only -> Normal");
                    }
                    else
                    {
                        if(TestIF_File.iShuttle_Sel==0)
                            asRecordBuffer.sprintf("Normal -> Shuttle 1 only");
                        else
                            asRecordBuffer.sprintf("Normal -> Shuttle 2 only");
                    }
                }

                NewRecordProcess("MES2102", "Head Select Value", asRecordBuffer);
                iOldShuttleMode= TestIF_File.iShuttleMode;
                iOldSelShuttle= TestIF_File.iShuttle_Sel;
            }

            if(iUseSuckModeBackup!=TestIF_File.iUseSuckMode)                    //Ifor 20161116 Add Use Suck Mode select Change Even Log 未修改不紀錄
            {
                if(TestIF_File.iTestMode==DualSite ||
                   TestIF_File.iTestMode==QualSite1X4 ||
                   TestIF_File.iTestMode==QualSite2X2)                          //Ifor 20161116 add 判斷是否使用Arm Suck Mode
                {                                                               //Ifor 20161220 (Steven) Use Pick Unit Count Change Even Modify
                    asRecordBuffer.sprintf("Use %s pick unit Pick -> Use %s pick unit Pick", IntToStr(iUseSuckModeBackup), IntToStr(TestIF_File.iUseSuckMode));
                    NewRecordProcess("MES2103", "Use Pick Unit Count Value Change", asRecordBuffer);
                    iUseSuckModeBackup=TestIF_File.iUseSuckMode;
                }
            }

#if 0 // GATE (2): golden :216-221 fSetup->Panel1->Caption -- see file-head GATE REGISTER for the full grep/rationale/behaviour-delta.
            if(asHandlingMode!=fSetup->Panel1->Caption)                         //Ifor 20161117 add Handler Mode Change Even Log 未修改不紀錄
            {                                                                   //Ifor 20161220 (Steven) Socket Count Change Even Modify
                asRecordBuffer.sprintf("%s -> %s", asHandlingMode, fSetup->Panel1->Caption);
                NewRecordProcess("MES2104", "Socket Count Value Change ", asRecordBuffer);
                asHandlingMode=fSetup->Panel1->Caption;
            }
#else
            // GATE (2) faithful default: no-op -- forms/fSetup.h carries no
            // Panel1 member (see file head); the MES2104 event is not raised
            // and asHandlingMode is not advanced.
#endif
        }
        else
        {
            //Ifor 20161219 (Steven) 開關 Site Even Log改至mtDutOnOffMouseUp 處理
            //==>
            //if(CUSTOMER_CODE != CC_KYEC_LEE)
            //{
            //    if(tmps3!=tmps1)
            //        RecordProcess(tmps4);                                     //kevin 20161003
            //}
            //<==
            //Ifor 20161219 add Kyec 開關Site Even Log
        }
    }

    //----------------clear_log folder  ----------------------------------
    if(SystemMonth<=2)
    {
        SystemMonth=SystemMonth + 10;
        SystemYear--;
    }
    else
    {
        SystemMonth=SystemMonth-2;
    }

    str1.sprintf("%04d%s%02d", SystemYear, "_", SystemMonth);
    s_cFolder= sFolder + str1;

    if((DirectoryExists(s_cFolder)))
    {
        int len = _tcslen(s_cFolder.c_str());
        TCHAR *pszFrom = new TCHAR[len+2];
        _tcscpy(pszFrom, s_cFolder.c_str());
        pszFrom[len] = 0;
        pszFrom[len+1] = 0;

        SHFILEOPSTRUCT      opStr;

        opStr.hwnd          = NULL;
        opStr.pTo           = NULL;
        opStr.wFunc         = FO_DELETE;
        opStr.fFlags        = FOF_SILENT  | FOF_NOCONFIRMATION;
        opStr.pFrom         = pszFrom;
        SHFileOperation(&opStr);
        delete [] pszFrom;
    }
    //-------------end of clear log---------------------------------------------
}
//------------------------------------------------------------------------------
void TMyLog::Save_AnyName(AnsiString ComponentName,AnsiString AData)            //kevin 20160728 記錄狀態
{
    TDateTime tt=Now();
    Word SystemYear, SystemMonth, SystemDate;
    DecodeDate(tt, SystemYear, SystemMonth, SystemDate);
    AnsiString str1, str2;

    str1.sprintf("%04d%s%02d%s", SystemYear, "_", SystemMonth, "\\");
    str2.sprintf("%s%04d%s%02d%s%02d%s", "log_", SystemYear, "_", SystemMonth, "_", SystemDate, ".ini");

    FILE * pFile;
    AnsiString fp="", s_cFolder;
    if(sFolder=="")
        sFolder.sprintf("%s", "D:\\HT9045_Log\\ChangeLog\\");

    IncludeTrailingPathDelimiter(sFolder);
    s_cFolder=sFolder+str1;
    MyForceDirectories(s_cFolder);

    fp=s_cFolder+str2;

    pFile=fopen(fp.c_str(), "a");
    if(pFile!=NULL)
    {
        AnsiString tmps,tmps1,tmps2,Atmps,Atmps1,Atmps2;
        Atmps.sprintf("\n(Null)\t\t\t%s->\t\%s\t\t%s\n", ComponentName, AData, DateTimeToStr(Now()));
        fputs(tmps.c_str(), pFile);
        fputs(Atmps.c_str(), pFile);
        fclose(pFile);
    }

    //----------------clear_log folder  ----------------------------------
    if(SystemMonth<=2)
    {
        SystemMonth=SystemMonth+10;
        SystemYear--;
    }
    else
    {
        SystemMonth=SystemMonth-2;
    }

    str1.sprintf("%04d%s%02d", SystemYear, "_", SystemMonth);
    s_cFolder=sFolder+str1;

    if((DirectoryExists(s_cFolder)))
    {
        int len=_tcslen(s_cFolder.c_str());
        TCHAR *pszFrom=new TCHAR[len+2];
        _tcscpy(pszFrom, s_cFolder.c_str());
        pszFrom[len]=0;
        pszFrom[len+1]=0;

        SHFILEOPSTRUCT      opStr;

        opStr.hwnd          = NULL;
        opStr.pTo           = NULL;
        opStr.wFunc         = FO_DELETE;
        opStr.fFlags        = FOF_SILENT  | FOF_NOCONFIRMATION;
        opStr.pFrom         = pszFrom;
        SHFileOperation(&opStr);
        delete [] pszFrom;
    }
    //-------------end of clear log---------------------------------------------
}
//------------------------------------------------------------------------------
void TMyLog::Find_Parent(TObject *PCtrl)
{
    TForm          *FrmPtr     = dynamic_cast <TForm          *>(PCtrl);
    TWinControl    *pC         = dynamic_cast <TWinControl    *>(PCtrl);
    if(FrmPtr==NULL && pC!=NULL)
        Find_Parent(pC->Parent);
    else if(FrmPtr!=NULL)
        fName=FrmPtr->Name;
}
//------------------------------------------------------------------------------
void TMyLog::Init_Log(TObject *PCtrl, AnsiString tmpUser, AnsiString tmpFolder)
{
    if(tmpFolder!="")
        sFolder=tmpFolder;

    sUser=tmpUser;
    loglist.clear();
    TForm       *FrmPtr     = dynamic_cast <TForm       *>(PCtrl);

    if(FrmPtr!=NULL)
        fName= FrmPtr->Name;
    else
        Find_Parent(PCtrl);

    if(dynamic_cast <TWinControl *>(PCtrl) != NULL)
    {
        Control_Log_Value((TWinControl *)PCtrl,true);
    }
}
//------------------------------------------------------------------------------
void TMyLog::Comp_Log(TObject *PCtrl)
{
    if(dynamic_cast <TWinControl *>(PCtrl) != NULL)
    {
        Control_Log_Value ((TWinControl *)PCtrl,false);
        Save_Log();
    }
}
//------------------------------------------------------------------------------
void TMyLog::Control_Log_Value(TWinControl  *PCtrl, bool bInit)
{
//    TPanel      *PanelPtr   = dynamic_cast <TPanel      *>(PCtrl);
//    TTabSheet   *ShtPtr     = dynamic_cast <TTabSheet   *>(PCtrl);
//    TForm       *FrmPtr     = dynamic_cast <TForm       *>(PCtrl);
//    TGroupBox   *PGroupBox  = dynamic_cast <TGroupBox   *>(PCtrl);
//    TScrollBox  *ScrBox     = dynamic_cast <TScrollBox  *>(PCtrl);
    TRadioGroup *RadGup     = dynamic_cast <TRadioGroup *>(PCtrl);
    if(RadGup!=NULL)
    {
        AnsiString tmps;
        if(RadGup->ItemIndex!=-1)
            tmps=RadGup->Items->Strings[RadGup->ItemIndex];
        else
            tmps=" ";

#if 0 // GATE (3a): golden :393-397 RadGup->Name / ->Caption -- see file-head GATE REGISTER.
        if(bInit)
            mapLogValue[fName+"_"+RadGup->Name+"_"+RadGup->Caption]=AnsiString(RadGup->ItemIndex);
        else
            Compare_Diff(fName+"_"+RadGup->Name+"_"+RadGup->Caption, AnsiString(RadGup->ItemIndex), tmps);
#else
        // GATE (3a) faithful default: no-op -- vclcompat::TRadioGroup carries
        // neither ->Name nor ->Caption (see file head); this branch is
        // unreachable dead code offline regardless (RadGup is never non-NULL
        // -- zero real TRadioGroup instances anywhere in the object graph).
#endif
    }
    else
    {
        for(int iP=0; iP<PCtrl->ControlCount; iP++)
        {
            TControl *P=PCtrl->Controls[iP];
            if(dynamic_cast <TWinControl *>(P)!=NULL)
            {
                Control_Log_Value((TWinControl *)P, bInit);      // recursive
            }

            //目前只記錄這些元件內容
            TScrollBar      *PScrollBar   = dynamic_cast <TScrollBar    *>(P);
            TTrackBar       *PTrackBar    = dynamic_cast <TTrackBar     *>(P);
            TCheckBox       *PCheckBox    = dynamic_cast <TCheckBox     *>(P);
            TRadioButton    *PRadioButton = dynamic_cast <TRadioButton  *>(P);
            TComboBox       *PTComboBox   = dynamic_cast <TComboBox     *>(P);
            TEdit           *PTEdit       = dynamic_cast <TEdit         *>(P);
            TLabeledEdit    *PTLEdit      = dynamic_cast <TLabeledEdit  *>(P);  //Steven 20200715 : 補上TLabeledEdit

            AnsiString sValue="NULL", sCaption=" ";
            if(PScrollBar!=NULL)
            {
                sValue=AnsiString(PScrollBar->Position);
            }
            else if(PTrackBar!=NULL)
            {
                sValue=AnsiString(PTrackBar->Position);
            }
            else if(PCheckBox!=NULL)
            {
                sCaption=PCheckBox->Caption.Trim();
                sValue=(PCheckBox->Checked?"1":"0");
            }
            else if(PRadioButton!=NULL)
            {
                sCaption=PRadioButton->Caption.Trim();
                sValue=(PRadioButton->Checked?"1":"0");
            }
            else if(PTComboBox!=NULL)
            {
                sValue=AnsiString (PTComboBox->Text);
            }
            else if(PTEdit!=NULL)
            {
                sValue=PTEdit->Text;
            }
            else if(PTLEdit!=NULL)
            {
                sValue=PTLEdit->Text;
            }

            if(sValue!="NULL")
            {
#if 0 // GATE (3b): golden :452/:454 P->Name -- see file-head GATE REGISTER.
               if(bInit)
                   mapLogValue[fName+"_"+P->Name]=sValue;
               else
                   Compare_Diff(fName+"_"+P->Name, sValue, sCaption);
#else
               // GATE (3b) faithful default: no-op -- vclcompat::TControl
               // (the shared base of every leaf type dynamic_cast'd above)
               // carries no ->Name (see file head).  Unreachable dead code
               // offline regardless: PCtrl->ControlCount is always 0 (no
               // real TWinControl anywhere in the object graph has
               // children), so this loop body never executes today.
#endif
            }
        }
    }
}
//------------------------------------------------------------------------------
bool TMyLog::Do_Log(TObject *PCtrl, AnsiString tmpUser, AnsiString tmpFolder)
{
    bool bRetrun=false;

        if(tmpFolder!="")
            sFolder=tmpFolder;

        sUser=tmpUser;
        loglist.clear();
        TForm       *FrmPtr     = dynamic_cast <TForm       *>(PCtrl);

        if(FrmPtr!=NULL)
            fName=FrmPtr->Name;
        else
            Find_Parent(PCtrl);

        if(dynamic_cast <TWinControl *>(PCtrl) != NULL)
        {
            Control_Log_Value((TWinControl *)PCtrl, false);
            if(loglist.size()>0)
            {
    //            Save_Log();                                                   //Steven 20170605 (wei) : 減少LOG數量
                SaveEventLog();
                bRetrun=true;
            }
        }
        return bRetrun;
}
//------------------------------------------------------------------------------
void TMyLog::Compare_Diff(AnsiString Sfind, AnsiString Svalue, AnsiString sCaption)
{
    std::map<AnsiString,AnsiString >::iterator mapIter;
    mapIter=mapLogValue.find(Sfind);
    TStringList *SL;                                                            //Steven 20161115 : EventLog存成文字檔
    SL=new TStringList();
    AnsiString Str;

    if(mapIter!=mapLogValue.end())
    {
        if(Svalue!=mapIter->second)
        {
            AnsiString tmps,s1;
            tmps=mapIter->second+" => "+Svalue;
            s1.sprintf("%-16s%s%-26s%s%-22s%s%-22s%s%-32s", sUser, Delimiter, Sfind, Delimiter, tmps, Delimiter, DateTimeToStr(Now()), Delimiter, sCaption);
            loglist.push_back(s1);

            SL->Clear();
            if(IniConfig.bSPILFunction==true)                                   //Steven 20240604 : SPIL格式的event log
            {
                GetTimeInfo();
                Str.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

                SL->Add("ChangeLog");                                           //UnitName
                SL->Add("\t");                                                  //AlarmCode
                SL->Add(Str);                                                   //OccurDateTime
                SL->Add("\t");                                                  //Recovery
                SL->Add("\t");                                                  //StopedTime
                SL->Add("\t");                                                  //Duplicate
                SL->Add(Sfind);                                                 //Message
                SL->Add(tmps+", "+sCaption);                                    //ErrPart
                slEventLog->AddTextWithLineNo(SL->CommaText);
            }
            else
            {
                SL->Add("ChangeLog");
                SL->Add("\t");
                SL->Add(sUser);
                SL->Add("\t");
                SL->Add("\t");
                SL->Add(Sfind);
                SL->Add(tmps+", "+sCaption);
                slEventLog->AddTextWithDateTime(SL->CommaText);
            }
            SaveEventLogInfo("210000000", SL->CommaText, 21, " ");
            mapIter->second=Svalue;
        }
    }
    else
    {
        mapLogValue[Sfind]=Svalue;
    }
    SL->Clear();                                                                //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete SL;
}
//------------------------------------------------------------------------------
AnsiString Space_Num(AnsiString S, int n)
{
    if(S.Length()>n)
        return "";

    AnsiString S1="                                                          ";
    return S1.SubString(1, n-S.Length()+1);
}
//------------------------------------------------------------------------------
//AI(W906-S93) 20260927: free-function door for TUs that cannot include handlerlog.h (it clashes with language.h) --
//  the FileRW TfSetup save flow (golden cSetUp.cpp:4129 `myLog.Save_SiteStatusLog();`) calls it as a block-scope extern
//  from its global-scope gen.inc (St01 tools/editlist/TestIF_File_SetUp.py).  myLog: cmydef.cpp:3483 (cmydef.h:3261).
void W906_SaveSiteStatusLog() { myLog.Save_SiteStatusLog(); }
