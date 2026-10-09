// =============================================================================
//  Public/MyStringList.cpp  --  TMyStringList : the file-backed log-line accumulator
//
//  Faithful translation of golden Public/MyStringList.cpp (969 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-MyStringList) 20260807
//  Translation wave: W906-PT-MyStringList (pure-translation completion campaign;
//    Public/ leaf utility, direct sibling of Public/MyProductionRecord.cpp,
//    Public/cBootLog.cpp and Public/ExternFunction.cpp already in this directory).
//
//  ROLE: every member of TMyStringList plus the free function SaveTryCatchLog.
//  The class buffers log lines in a SEPARATE inner TStringList (`MyList`) and
//  flushes them to a date/hour-partitioned file whose name it computes itself.
//  See Public/MyStringList.h's banner for the full role statement and for the
//  two non-mechanical header decisions (TSaveType relocation, __property proxy).
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim -- all 29 golden functions, nothing omitted):
//     TMyStringList() / TMyStringList(3-arg) / ~TMyStringList
//     SetPath / SetFileName / SetFirstRow / SetMaxLineCount / SetSaveType /
//     SetAutoSave / SetSaveSameFolder / SetSaveByLotID / SetSaveFixedFile
//     SetLotData
//     AddText / AddTextWithLineNo / AddTextWithDateTime / AddTextWithDateTime2 /
//     AddTextWithDateTime3
//     GetTimeInfo
//     MySaveToFile / MySaveToFileShareMode
//     MySaveFileByFileNameAndType / MySaveFileByFileName
//     GetFileName / GetLastLine
//     SaveTryCatchLog (free function, golden :671)
//     MyInsertToFile / MySaveSGJamCountToFile / MyInsert2DMappingToFile
//   SATISFIED-BY-SHIM (so it LINKS):  ONE, and it is a TU-local static helper,
//     not a shim TU:  MySL_PathCombin  (golden `FileInfo().PathCombin(...)`,
//     MySaveToFileShareMode :259).  `class FileInfo` (golden ProductionInfo/
//     FileInfo.{h,cpp}) is untranslated tree-wide; common.cpp:2193-2232 already
//     established the precedent of inlining just PathCombin's body as a TU-local
//     static rather than dragging in the whole class, and this is a verbatim copy
//     of golden FileInfo::PathCombin (ProductionInfo/FileInfo.cpp:290-312).  It is
//     `static`, so it cannot collide with common.cpp's identically-shaped copy.
//   GATE (#if 0): NONE.  Every golden symbol this file names was verified to
//     exist and link in the current port tree before writing a line -- see the
//     DEPENDENCY VERIFICATION block below.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init) dropped.
//  AnsiString (1-based) + AnsiString::sprintf via vclcompat (verbatim; golden's
//  `%s`-with-raw-AnsiString idiom is preserved, never rewritten to .c_str()).
//  NO __fastcall / __property / __published in this .cpp -- and none in the .h
//  either, so the two agree (a __fastcall mismatch would mangle differently and
//  fail to link; see vclcompat/vcl_compat.h:24-54).
//  SOFT_SIMULTE is NOT defined; golden has no #ifdef SOFT_SIMULTE in this unit.
//  Numeric semantics kept EXACT (no int/float rewrites; SystemHour%2 / %4 / %6 /
//  %8 / %12 integer arithmetic in GetFileName is untouched).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
//
//  ---------------------------------------------------------------------------
//  DEPENDENCY VERIFICATION (re-checked against the CURRENT tree, 20260807)
//  ---------------------------------------------------------------------------
//   * MyDBIProcess          -- REAL, 2-arg, aHotPlateSubstrate.h:924 / .cpp:1030.
//       Golden's is 3-arg-with-default (`MyDBIProcess(asTable,S1,S2="")`, golden
//       cMyDB.h:20); this tree homed it as a 2-arg stand-in and EVERY ported call
//       site passes 2 args (acarry.cpp:2774.., ainarm_SearchPickPlate.cpp:1413..,
//       SECSGEM/uHGemEquipment.cpp:1108), so golden's own 2-argument call here
//       translates verbatim.
//       !! DO NOT ADD `#include "cMyDB.h"` TO THIS TU !!  cMyDB.h:81 declares the
//       golden 3-arg __fastcall spelling; with BOTH headers visible the 2-argument
//       call below becomes AMBIGUOUS (2-arg exact vs 3-arg-via-default, identical
//       conversion ranks) and this file stops compiling.  MachineDefine.h's own
//       `#include "cMyDB.h"` is inside its `#if 0` block (:34-146), so it is safe.
//   * MyForceDirectories    -- REAL, common.h:341 (1- and 2-arg forms; ungated).
//   * WriteDataToFile       -- REAL, common.h:337 (const char* overload; ungated).
//   * GetYesterdayInfo      -- REAL, cpublic.h:24.
//   * SystemYear..SystemMSec / SystemYearYesterday..SystemDateYesterday
//                           -- REAL globals, cmydef.h:226-228.  NOTE they are
//       SHADOWED inside every TMyStringList member by the class's own identically
//       named Word members (golden MyStringList.h:42-43) -- the free function
//       SaveTryCatchLog is the only place in this file that reads the GLOBALS.
//       That is golden's structure, reproduced exactly.
//   * slEventLog            -- REAL, cmydef.h:117 (`extern TMyStringList*`); this
//       file is what finally makes that pointer's type complete.
//   * IniConfig.{sMachineType,SocketHandlerID,sGPIBMachineID,bSIGURDFunction,
//       iN10UploadProductMethod} -- REAL, Config.h:203/51/204/135/1030.
//   * RunInfo.LotStartTime  -- REAL, cprod.h:2732.
//   * TestIF_File.{bEnableBarCode,bSortingBy2DIDList} -- REAL, cprod.h:1933/2015.
//   * LastSet.{iRunStartMode,iTester} -- REAL, LastSet.h:366/125 (via canary_support.h:59).
//   * rsmContinuRetest / rsmCInitialRetest -- REAL, MachineType.h:605-606.
//   * OFF_LINE              -- REAL, cmydef.h:85.
//   * CUSTOMER_CODE         -- REAL, cmydef.h:3181;  CC_ASE_KaohSiung MachineType.h:308,
//                              CC_CYPRESS :368, CC_QUALCOMM (MachineType.h).
//   * fSCKART->palLotNumber/pnlProcessCode/palTestCnt->Caption
//                           -- REAL, forms/fSCKART.h:90-93 + :140 (TfSortCTPanel
//                              = vclcompat::TPanel, FormWidgets.h:150).
//   * fLotInfo->edtSysLotID/edtCusLotID/edtCusDevGrp->Text, ->cbRunMode->Text
//                           -- REAL, forms/fLotInfo.h:26/29/65/66 + :111.
//   * StringReplace / TReplaceFlags / rfReplaceAll -- REAL, vclcompat/SysUtils.h:71-86.
//   * DeleteFile(AnsiString) -- REAL, vclcompat/SysUtils.h:94; the umbrella
//       #undefs windows.h's DeleteFile->DeleteFileA macro (vcl_compat.h:192-196)
//       BEFORE declaring it, so golden's VCL spelling survives.
//   Golden's `#include "cObserver.h"` (golden :6) contributes NOTHING used by this
//   unit -- it is the TfObserver VCL diagnostics form; no symbol from it appears
//   anywhere in golden's 969 lines.  Dropped, not gated (there is nothing to gate).
//   Golden's :7 MemoryAlarm.h and :8 HonNewDel.h are ALREADY commented out in
//   golden; kept as comments below.
//
//  ---------------------------------------------------------------------------
//  BCB6-vs-port BEHAVIOUR DELTAS this file does NOT (and must not) paper over
//  ---------------------------------------------------------------------------
//   (A) TStrings::Text.  BCB6 joins with sLineBreak (#13#10) AND terminates the
//       LAST line too, so golden's `Str=MyList->Text;` ends with a CRLF and each
//       flushed block is newline-separated on disk.  vclcompat's
//       TStringList::GetText (vclcompat/TStringList.cpp:101-108) joins with a
//       bare '\n' and appends NO trailing terminator.  Consequences, all outside
//       this unit's write boundary and therefore reported, not patched:
//         - consecutive appends run together on one physical line;
//         - golden's `StringReplace(Str,"\r\n","\n",...)` calls (:342/:376/:428/
//           :456/:795) become no-ops because there is no "\r\n" left to replace.
//       Both are properties of vclcompat, identical for every existing consumer
//       of `->Text`; fixing them belongs in vclcompat, not here.
//   (B) BCB6 zero-fills TObject descendants before the constructor runs (Delphi
//       TObject.InitInstance).  Golden's two constructors therefore leave
//       HTSaveSameFolder / HTSaveByLotID and the seven Word Systemxxx members
//       DETERMINISTICALLY zero without assigning them.  vclcompat's TStringList
//       is a plain C++ class with no such zero-fill, so ZeroInitVclFields() below
//       reproduces that observable state explicitly.  This is REPRODUCING golden
//       behaviour, not correcting it -- without it, SetLotData's %04d/%02d and
//       MySaveToFileShareMode's HTSaveSameFolder test read indeterminate values.
// =============================================================================
//
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "MyStringList.h" / "cmydef.h" / "cObserver.h"
//   //#include "MemoryAlarm.h"
//   //#include "HonNewDel.h"
//   #include "common.h" / "uLotInfo.h" / "SCK_ART.h" / "FileInfo.h"
//   ; #pragma package(smart_init)
// =============================================================================

#include "MachineDefine.h"          // golden's include hub: vclcompat umbrella + <windows.h>/<fstream>/<iostream>/<string> + `using namespace std;`
// golden :2  #pragma hdrstop                -- dropped (translation-time edit)

#include "Public/MyStringList.h"    // TMyStringList / TSaveType / SaveTryCatchLog
#include "cmydef.h"                 // CUSTOMER_CODE, OFF_LINE, SystemYear.., SystemYearYesterday.., slEventLog
// golden :6  #include "cObserver.h"          -- TfObserver VCL form; contributes no symbol used here (see banner)
// golden :7  //#include "MemoryAlarm.h"      -- already commented out in golden
// golden :8  //#include "HonNewDel.h"        -- already commented out in golden
#include "common.h"                 // MyForceDirectories / WriteDataToFile
#include "MachineType.h"            // CC_ASE_KaohSiung / CC_CYPRESS / CC_QUALCOMM / rsmContinuRetest / rsmCInitialRetest
#include "Config.h"                 // IniConfig (also reached via cprod.h)
#include "cprod.h"                  // RunInfo.LotStartTime / TestIF_File.bEnableBarCode / .bSortingBy2DIDList
#include "cpublic.h"                // GetYesterdayInfo()
#include "canary_support.h"         // LastSet (LAST_GENERAL_SET) via LastSet.h:587
#include "FormsFacade.h"            // golden "uLotInfo.h" -> forms/fLotInfo.h (fLotInfo); golden "SCK_ART.h" -> forms/fSCKART.h (fSCKART)
#include "aHotPlateSubstrate.h"     // MyDBIProcess(AnsiString,AnsiString) -- see the DO-NOT-ADD-cMyDB.h warning in the banner
// golden :12 #include "FileInfo.h"           -- class FileInfo untranslated; replaced by the TU-local MySL_PathCombin below
#include <fstream>                  // std::ifstream / std::getline (GetLastLine, golden :652-660)
#include <string>                   // std::string                  (GetLastLine, golden :655)

//---------------------------------------------------------------------------
// golden :15  #pragma package(smart_init)   -- dropped (translation-time edit)
//---------------------------------------------------------------------------

//===========================================================================
//  MySL_PathCombin -- TU-local static stand-in for golden's
//  `FileInfo().PathCombin(sPath, sFile)` (the ONE FileInfo use in this unit,
//  golden :259).  Body is a verbatim copy of golden FileInfo::PathCombin
//  (ProductionInfo/FileInfo.cpp:290-312), read and confirmed line-by-line.
//  `class FileInfo` does not exist anywhere in this translated tree; the
//  precedent for inlining just this helper instead of dragging the whole
//  untranslated class in is common.cpp:2193-2232 (Common_PathCombin), which
//  cites in turn Automation/SCK_ART_Remainder.cpp's W5SckArtRem_* stand-ins.
//  `static` = internal linkage, so this cannot collide with common.cpp's copy.
//
//  ONE known, deliberate, sub-millisecond delta: golden constructs a temporary
//  `FileInfo()` whose DESTRUCTOR (FileInfo.cpp:25-33) ends with `Sleep(1)`
//  ("避免非同步互搶資源").  A free function has no destructor, so that 1 ms
//  pause is gone.  PathCombin itself touches no FileInfo member state
//  (FileInfo.cpp:290-312 reads only its two arguments), so the RESULT is
//  identical; only the incidental delay differs.  Recorded here rather than
//  reproduced, because reproducing a destructor's Sleep would be inventing
//  behaviour, not translating it.
//===========================================================================
static AnsiString MySL_PathCombin(const AnsiString& sPath, const AnsiString& sFile)
{
    AnsiString combinedPath=sPath;
    if(!combinedPath.IsEmpty())
    {
        if(combinedPath[combinedPath.Length()]=='/' || combinedPath.Pos("/"))   // FTP path format
        {
            if(combinedPath[combinedPath.Length()]!='/')
            {
                combinedPath+="/";
            }
        }
        else                                                                    // Local path
        {
            if(combinedPath[combinedPath.Length()] != '\\')
            {
                combinedPath+="\\";
            }
        }
    }
    combinedPath+=sFile;
    return combinedPath;
}

//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: BindProperties -- NO golden counterpart.
//  It is the runtime half of the __property translation described in the
//  header: each proxy is wired to exactly the read-field / write-setter pair
//  the golden `__property ... ={read=..., write=...}` line names (golden
//  MyStringList.h:78-86).  Both pointers-to-member are formed HERE, inside a
//  member of TMyStringList, which is what makes taking the address of the
//  golden-private fields and setters legal.
//  MUST be the first statement of every constructor -- the proxies are
//  default-constructed with a null Owner, and golden's ctor bodies start
//  assigning through them immediately (golden :20-23 / :37-40).
//---------------------------------------------------------------------------
void TMyStringList::BindProperties()
{
    Path          .Bind(this, &TMyStringList::HTPath,          &TMyStringList::SetPath);
    FileName      .Bind(this, &TMyStringList::HTFileName,      &TMyStringList::SetFileName);
    FirstRow      .Bind(this, &TMyStringList::HTFirstRow,      &TMyStringList::SetFirstRow);
    MaxLineCount  .Bind(this, &TMyStringList::HTMaxLineCount,  &TMyStringList::SetMaxLineCount);
    SaveType      .Bind(this, &TMyStringList::HTSaveType,      &TMyStringList::SetSaveType);
    AutoSave      .Bind(this, &TMyStringList::HTAutoSave,      &TMyStringList::SetAutoSave);
    FixedFile     .Bind(this, &TMyStringList::HTSaveFixedFile, &TMyStringList::SetSaveFixedFile);
    SaveSameFolder.Bind(this, &TMyStringList::HTSaveSameFolder,&TMyStringList::SetSaveSameFolder);
    SaveByLotID   .Bind(this, &TMyStringList::HTSaveByLotID,   &TMyStringList::SetSaveByLotID);
}
//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: ZeroInitVclFields -- NO golden counterpart.
//  Reproduces BCB6's zero-fill of a TObject descendant's instance memory before
//  the constructor body runs (Delphi TObject.NewInstance/InitInstance), which
//  vclcompat's plain-C++ TStringList base does not do.  Only the members golden's
//  two ctors do NOT assign are listed -- everything else is set by golden below
//  and must NOT be pre-set here (that would hide a real ordering bug):
//    HTSaveSameFolder / HTSaveByLotID  -- read at MySaveToFileShareMode :326/:362
//    the seven Word Systemxxx members  -- read by SetLotData :129 BEFORE any
//      GetTimeInfo() call can have run (see the golden note at SetLotData).
//  The AnsiString members (HTLotID, HTLotStartTime, sLast*, sLot*) self-
//  initialise to "" in both toolchains, so they are deliberately not touched.
//---------------------------------------------------------------------------
void TMyStringList::ZeroInitVclFields()
{
    HTSaveSameFolder    =false;
    HTSaveByLotID       =false;
    SystemHour          =0;
    SystemMin           =0;
    SystemSec           =0;
    SystemMSec          =0;
    SystemYear          =0;
    SystemMonth         =0;
    SystemDate          =0;
}
//---------------------------------------------------------------------------
TMyStringList::TMyStringList()
{
    BindProperties();                                                           //AI(W906-PT-MyStringList) 20260807: must precede the property writes below
    ZeroInitVclFields();                                                        //AI(W906-PT-MyStringList) 20260807: BCB6 TObject zero-fill (see banner delta (B))
    MaxLineCount        =1000;
    Path                ="D:\\HandlerLog";
    FirstRow            ="";
    SaveType            =TByDay;
    AutoSave            =true;
    bUseFTRT            =false;
    bFilePathWithDate   =true;                                                  //Steven 20210623 : 預設存檔要有日期當資料夾
    bHanaTrayMap        =false;
    HTSaveFixedFile     =false;
    MyList              =new TStringList();
    sPrevFileName       ="";                                                    //Steven 20250809 : 紀錄前一個的檔名
    bChangeFile         =false;
//    MyList              =HonNew(MyList, __FILE__, __FUNC__, __LINE__);
}
//---------------------------------------------------------------------------
TMyStringList::TMyStringList(AnsiString sPath, AnsiString sFileName, AnsiString sFirstRow)
{
    BindProperties();                                                           //AI(W906-PT-MyStringList) 20260807: must precede the property writes below
    ZeroInitVclFields();                                                        //AI(W906-PT-MyStringList) 20260807: BCB6 TObject zero-fill (see banner delta (B))
    MaxLineCount        =1;
    Path                =sPath;
    FileName            =sFileName;
    FirstRow            =sFirstRow;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        SaveType            =TByHour;                                           //kevin 20210518 Data too large    TByHour=TByDay;
    else
        SaveType            =TByDay;

    AutoSave            =true;
    bUseFTRT            =false;
    bFilePathWithDate   =true;                                                  //Steven 20210623 : 預設存檔要有日期當資料夾
    bHanaTrayMap        =false;
    HTSaveFixedFile     =false;
    MyList              =new TStringList();
    sPrevFileName       ="";                                                    //Steven 20250809 : 紀錄前一個的檔名
    bChangeFile         =false;
//    MyList              =HonNew(MyList, __FILE__, __FUNC__, __LINE__);
}
//---------------------------------------------------------------------------
TMyStringList::~TMyStringList()
{
    try
    {
        MySaveToFile();
        MyList->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete MyList;
//        HonDelete(MyList);
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~TMyStringList");
    }
}
//---------------------------------------------------------------------------
void TMyStringList::SetPath(AnsiString P)
{
    HTPath=P;
}
//---------------------------------------------------------------------------
void TMyStringList::SetFileName(AnsiString P)
{
    HTFileName=P;
}
//---------------------------------------------------------------------------
void TMyStringList::SetFirstRow(AnsiString P)
{
    HTFirstRow=P;
}
//---------------------------------------------------------------------------
void TMyStringList::SetMaxLineCount(int Cnt)
{
    HTMaxLineCount=Cnt;
}
//---------------------------------------------------------------------------
void TMyStringList::SetSaveType(TSaveType Type)
{
    HTSaveType=Type;
}
//---------------------------------------------------------------------------
void TMyStringList::SetAutoSave(bool P)
{
    HTAutoSave=P;
}
//---------------------------------------------------------------------------
void TMyStringList::SetSaveSameFolder(bool P)                                   //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾
{
    HTSaveSameFolder=P;
}
//---------------------------------------------------------------------------
void TMyStringList::SetSaveByLotID(bool P)                                      //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
{
    HTSaveByLotID=P;
}
//---------------------------------------------------------------------------
void TMyStringList::SetSaveFixedFile(bool P)                                    //Steven 20250716 : 固定檔名存檔
{
    HTSaveFixedFile=P;
}
//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: GOLDEN QUIRK preserved VERBATIM (golden
//  :118-137) -- SetLotData formats SystemYear/SystemMonth/SystemDate into the
//  ByLotID folder path WITHOUT calling GetTimeInfo() first, so it uses whatever
//  the last Add*/MySave*/GetFileName call happened to leave in those members
//  (0/0/0 -> "0000\00\00" on a freshly constructed object).  Every sibling
//  method in this file DOES call GetTimeInfo() first; this one does not.
//  Not fixed.
//---------------------------------------------------------------------------
void TMyStringList::SetLotData(AnsiString ID, AnsiString Time, AnsiString LotFileName)
{
    HTLotID=ID;                                                                 //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
    HTLotStartTime=Time;                                                        //Steven 20250520 : 加入By Lot存檔

    if(ID=="")
    {
        sLotFileName="";
    }
    else if(LotFileName=="")                                                    //Steven 20250809 : 修正event log上傳
    {
        LotFileName.sprintf("%s\\ByLotID\\%04d\\%02d\\%02d", HTPath, SystemYear, SystemMonth, SystemDate);
        MyForceDirectories(LotFileName, "TMyStringList::SetLotData");
        sLotFileName.sprintf("%s\\%s_%s_%s_%s_ByLotEventLog.csv", LotFileName, IniConfig.sMachineType, IniConfig.SocketHandlerID, HTLotID, HTLotStartTime);
    }
    else
    {
        sLotFileName=LotFileName;
    }
}
//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: GOLDEN QUIRK preserved VERBATIM (golden
//  :143-144) -- `if(FileName=="") FileName="";` is a no-op in all four Add*
//  methods that carry it.  Kept because removing it would change nothing but
//  would break the line-for-line correspondence with golden.  Not fixed.
//  Note also the OFF-BY-ONE inconsistency between AddText (`>=HTMaxLineCount`,
//  flush AFTER adding) and the three AddTextWithDateTime* variants
//  (`>HTMaxLineCount`, flush BEFORE adding): golden really is asymmetric here.
//---------------------------------------------------------------------------
void TMyStringList::AddText(AnsiString Msg)
{
    GetTimeInfo();

    if(FileName=="")
        FileName="";

    MyList->Add(Msg);

    if(MyList->Count>=HTMaxLineCount)
    {
        MySaveToFile();
        MyList->Clear();
    }
}
//------------------------------------------------------------------------------
void TMyStringList::AddTextWithLineNo(AnsiString Msg)
{
    AnsiString Str;
    Str.sprintf("%s", Msg);
    AddText(Str);
}
//------------------------------------------------------------------------------
void TMyStringList::AddTextWithDateTime(AnsiString Msg)
{
    AnsiString Str;
    GetTimeInfo();
    if(IniConfig.bSIGURDFunction)                                               //Sam 矽格北興俊堯 要求不要留空格。
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d,%s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Msg);
    else
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Msg);

    if(FileName=="")
        FileName="";

    if(MyList->Count>HTMaxLineCount)
    {
        MySaveToFile();
        MyList->Clear();
    }
    MyList->Add(Str);
}
//------------------------------------------------------------------------------
AnsiString TMyStringList::AddTextWithDateTime2(AnsiString Msg)
{
    AnsiString Str;
    GetTimeInfo();
    Str.sprintf("%04d/%02d/%02d %02d:%02d:%02d,%s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, Msg);

    if(FileName=="")
        FileName="";

    if(MyList->Count>HTMaxLineCount)
    {
        MySaveToFile();
        MyList->Clear();
    }
    MyList->Add(Str);
    return Str;
}
//------------------------------------------------------------------------------
void TMyStringList::AddTextWithDateTime3(AnsiString Msg, AnsiString Msg2)
{
    AnsiString Str;
    GetTimeInfo();
    Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d, %03d, %s, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Msg, Msg2);

    if(FileName=="")
        FileName="";

    if(MyList->Count>HTMaxLineCount)
    {
        MySaveToFile();
        MyList->Clear();
    }
    MyList->Add(Str);
}
//------------------------------------------------------------------------------
void TMyStringList::GetTimeInfo()
{
    static TDateTime dtPresent;
    dtPresent=Now();
    DecodeDate(dtPresent, SystemYear, SystemMonth, SystemDate);
    DecodeTime(dtPresent, SystemHour, SystemMin, SystemSec, SystemMSec);
}
//------------------------------------------------------------------------------
void TMyStringList::MySaveToFile()
{
    MySaveToFileShareMode();
}
//------------------------------------------------------------------------------
void TMyStringList::MySaveToFileShareMode()
{
    AnsiString Str;
    AnsiString sPathName_Same;
    AnsiString sFileName_Same;
    GetTimeInfo();

    if(HTAutoSave==false || MyList==NULL || MyList->Count==0)                   //沒資料就不用存檔
        return;

//    Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);

    if(bHanaTrayMap)                                                            //Steven 20250414 : HANA ART Function
    {
        if(fSCKART->palLotNumber->Caption=="" ||
           fSCKART->pnlProcessCode->Caption=="")
            return;

        sPathName_Same.sprintf("D:\\HT9045_Log\\Hana_TrayMap\\%04d\\%s", SystemYear, fSCKART->palLotNumber->Caption);
        MyForceDirectories(sPathName_Same, "TMyStringList::MySaveToFile_2");
        sLastPathName=sPathName_Same;

        sFileName_Same.sprintf("%s_%s_%s_%s_%s_%s.txt", IniConfig.sGPIBMachineID,
                                                        fSCKART->pnlProcessCode->Caption,
                                                        fSCKART->palLotNumber->Caption,
                                                        HTFileName,
                                                        RunInfo.LotStartTime,
                                                        fSCKART->palTestCnt->Caption);
        sLastNameNoPath=sFileName_Same;
        sLastFileName=MySL_PathCombin(sPathName_Same, sLastNameNoPath);         //AI(W906-PT-MyStringList) 20260807: golden :259 `FileInfo().PathCombin(...)` -- see the MySL_PathCombin banner

        if(FileExists(sLastFileName)==false && HTFirstRow!="")
        {
            Str=HTFirstRow+"\r\n"+MyList->Text;
        }
        else
        {
            Str=MyList->Text;
        }

        HANDLE hFile = CreateFile(
            sLastFileName.c_str(),         // 檔案名稱
            GENERIC_WRITE,                  // 寫入權限
            FILE_SHARE_READ,                // 允許其他程式讀取
            NULL,                           // 安全性屬性
            OPEN_ALWAYS,                    // 如果檔案不存在則建立，已存在則開啟
            FILE_ATTRIBUTE_NORMAL,          // 一般檔案屬性
            NULL                            // 無模板檔案
        );

        if(hFile!=INVALID_HANDLE_VALUE)                                         //JimmyChiu : 20250401 : 變更開檔案方式
        {
            SetFilePointer(hFile, 0, NULL, FILE_END);                           // 移動到檔案結尾，模擬追加模式
            DWORD bytesWritten;
            WriteFile(hFile, Str.c_str(), Str.Length(), &bytesWritten, NULL);
            CloseHandle(hFile);                                                 // 寫入完成後立即關閉
        }
    }
    else
    {
        Str=GetFileName();                                                      //Steven 20250809 : 修正event log上傳
        if(sLastFileName!="" &&
           sLastFileName!=Str)
        {
            sPrevFileName       =sLastFileName;                                 //Steven 20250809 : 紀錄前一個的檔名
            bChangeFile         =true;
        }
        sLastFileName=Str;

        if(FileExists(sLastFileName)==false && HTFirstRow!="")
        {
            Str=HTFirstRow+"\r\n"+MyList->Text;
        }
        else
        {
            Str=MyList->Text;
        }

        HANDLE hFile = CreateFile(
            sLastFileName.c_str(),       // 檔案名稱
            GENERIC_WRITE,               // 寫入權限
            FILE_SHARE_READ,             // 允許其他程式讀取
            NULL,                        // 安全性屬性
            OPEN_ALWAYS,                 // 如果檔案不存在則建立，已存在則開啟
            FILE_ATTRIBUTE_NORMAL,       // 一般檔案屬性
            NULL                         // 無模板檔案
        );

        if(hFile!=INVALID_HANDLE_VALUE)                                         //JimmyChiu : 20250401 : 變更開檔案方式
        {
            SetFilePointer(hFile, 0, NULL, FILE_END);                           // 移動到檔案結尾，模擬追加模式
            DWORD bytesWritten;
            WriteFile(hFile, Str.c_str(), Str.Length(), &bytesWritten, NULL);
            CloseHandle(hFile);                                                 // 寫入完成後立即關閉
        }

        if(HTSaveSameFolder==true &&                                            //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾
           HTSaveFixedFile==false)
        {
            sPathName_Same.sprintf("%s\\AllEventLog", HTPath);
            MyForceDirectories(sPathName_Same, "TMyStringList::MySaveToFile_1");

            if(HTSaveType==TByMaxLineCount)
                sFileName_Same.sprintf("%s\\%s_%04d%02d%02d %02d%02d%02d.csv", sPathName_Same, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
            else
                sFileName_Same.sprintf("%s\\%s_%04d%02d%02d.csv", sPathName_Same, HTFileName, SystemYear, SystemMonth, SystemDate);

            if(FileExists(sFileName_Same)==false && HTFirstRow!="")
                Str=HTFirstRow+"\r\n"+MyList->Text;
            else
                Str=MyList->Text;

            Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);
            hFile = CreateFile(
                sFileName_Same.c_str(),
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hFile != INVALID_HANDLE_VALUE)                                      //JimmyChiu : 20250401 : 變更開檔案方式
            {
                SetFilePointer(hFile, 0, NULL, FILE_END);
                DWORD bytesWritten;
                WriteFile(hFile, Str.c_str(), Str.Length(), &bytesWritten, NULL);
                CloseHandle(hFile);
            }
        }

        if(HTSaveByLotID==true &&                                               //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
           HTSaveFixedFile==false)
        {
            if(HTLotID!="")
            {
//                sPathName_Lot.sprintf("%s\\ByLotID\\%04d\\%02d\\%02d", HTPath, SystemYear, SystemMonth, SystemDate);
//                MyForceDirectories(sPathName_Lot, "TMyStringList::MySaveToFile_2");
//                sFileName_Lot.sprintf("%s\\%s_%s.csv", sPathName_Lot, HTLotID, HTLotStartTime);

                if(FileExists(sLotFileName)==false && HTFirstRow!="")           //Steven 20250809 : 修正event log上傳
                    Str=HTFirstRow+"\r\n"+MyList->Text;
                else
                    Str=MyList->Text;

                Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);
                hFile = CreateFile(
                    sLotFileName.c_str(),
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );

                if(hFile!=INVALID_HANDLE_VALUE)                                 //JimmyChiu : 20250401 : 變更開檔案方式
                {
                    SetFilePointer(hFile, 0, NULL, FILE_END);
                    DWORD bytesWritten;
                    WriteFile(hFile, Str.c_str(), Str.Length(), &bytesWritten, NULL);
                    CloseHandle(hFile);
                }
            }
        }
    }
    MyList->Clear();
}
//------------------------------------------------------------------------------
void TMyStringList::MySaveFileByFileNameAndType(AnsiString asFileName, AnsiString asLotId, AnsiString asFileType) //RogerYang 20170406 (Steven)
{
    AnsiString sPathName;
    AnsiString sFileName;
    AnsiString Str;
    FILE *pFile;
    GetTimeInfo();

    if(HTAutoSave==false || MyList->Count==0)
        return;

    sPathName.sprintf("D:\\HT9045_Log\\JamAlarmLogTxt\\%s", asLotId);
    MyForceDirectories(sPathName, "TMyStringList::MySaveFileByFileNameAndType");
    sFileName.sprintf("%s\\%s_%04d%02d%02d_%s_%s.csv",
                        sPathName, asFileName,
                        SystemYear, SystemMonth, SystemDate,
                        asLotId,                                                //Sam 20170428 (wei) 帶入 LotID
                        asFileType);                                            //Sam 20171215 (wei) : 檔名尾巴增加JAM/INPUT

    if(FileExists(sFileName)==false && HTFirstRow!="")
    {
        Str=HTFirstRow+"\r\n"+MyList->Text;
    }
    else
    {
        Str=MyList->Text;
    }

    Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);
    pFile=fopen(sFileName.c_str(), "a");
    if(pFile!=NULL)
    {
        fputs(Str.c_str(), pFile);
        fclose(pFile);
    }
    pFile=NULL;
    MyList->Clear();
}
//------------------------------------------------------------------------------
void TMyStringList::MySaveFileByFileName(AnsiString asPathName, AnsiString asFileName)   //Sam 20170516 (wei) 力成 SocketID Count 保存
{
    AnsiString Str;
    FILE *pFile;

    MyForceDirectories(asPathName, "TMyStringList::MySaveFileByFileName");
    asFileName.sprintf("%s\\%s", asPathName, asFileName);

    if(FileExists(asFileName)==false && HTFirstRow!="")
    {
        Str=HTFirstRow+"\r\n"+MyList->Text;
    }
    else
    {
        Str=MyList->Text;
    }

    Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);
    pFile=fopen(asFileName.c_str(), "a");
    if(pFile!=NULL)
    {
        fputs(Str.c_str(), pFile);
        fclose(pFile);
    }
    pFile=NULL;
    MyList->Clear();
}
//------------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: ORDINAL NOTE, not a change -- golden :487
//  and :491 test HTSaveType with `>=`, so the folder layout is decided by the
//  enum's ORDER, and TByMin=10 sorts ABOVE TByMonth=8.  A TByMin log therefore
//  takes the `>=TByMonth` branch (one folder per YEAR) even though its FILE name
//  is per-minute (:617-630).  Reproduced exactly; see MyStringList.h's TSaveType
//  block for where those values come from.
//------------------------------------------------------------------------------
AnsiString TMyStringList::GetFileName()
{
    AnsiString sPathName;
    AnsiString sFileName="";
    AnsiString Str;
    int iHour;
    GetTimeInfo();

    if(HTPath=="")
        Path="D:\\HandlerLog";

    if(FileName=="")
        FileName="";

    if(HTSaveFixedFile)
    {
        sPathName=HTPath;
    }
    else if(bFilePathWithDate)                                                  //Steven 20210623 : 預設存檔要有日期當資料夾
    {
        if(HTSaveType>=TByMonth)                                                //使用年月存檔的話,就By年分類
        {
            sPathName.sprintf("%s\\%04d", HTPath, SystemYear);
        }
        else if(HTSaveType>=TByDay)                                             //使用每日存檔的話,就By月分類
        {
            sPathName.sprintf("%s\\%04d\\%02d", HTPath, SystemYear, SystemMonth);
        }
        else                                                                    //其他就每年存成365個資料夾
        {
            if(IniConfig.iN10UploadProductMethod==0)                            //JerryYang 20190131 上傳production log可選擇00:00 or 08:00
            {
                sPathName.sprintf("%s\\%04d\\%02d\\%02d", HTPath, SystemYear, SystemMonth, SystemDate);
            }
            else
            {
                if(SystemHour<8)
                {
                    GetYesterdayInfo();
                    sPathName.sprintf("%s\\%04d\\%02d\\%02d", HTPath, SystemYearYesterday, SystemMonthYesterday, SystemDateYesterday);
                }
                else
                {
                    sPathName.sprintf("%s\\%04d\\%02d\\%02d", HTPath, SystemYear, SystemMonth, SystemDate);
                }
            }
        }
    }
    else
    {
        sPathName=HTPath;
    }

    MyForceDirectories(sPathName, "TMyStringList::GetFileName");
    if(HTSaveFixedFile)
    {
        sFileName.sprintf("%s\\%s.csv", sPathName, HTFileName);
    }
    else if(HTSaveType==TByMaxLineCount)
    {
        sFileName.sprintf("%s\\%s_%04d%02d%02d %02d%02d%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    }
    else
    {
        if(HTSaveType==TByHour)
        {
            if(bUseFTRT)                                                        //Steven 20220414 : JSCK OEE檔名要加上FT RT
            {
                if(LastSet.iRunStartMode==rsmContinuRetest ||
                   LastSet.iRunStartMode==rsmCInitialRetest)
                    sFileName.sprintf("%s\\%s_%04d%02d%02d %02d_RT.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour);
                else
                    sFileName.sprintf("%s\\%s_%04d%02d%02d %02d_FT.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour);
            }
            else                                                                //Steven 20220513 : For ChipMos FTP
            {
                sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour);
            }
        }
        else if(HTSaveType==TBy2Hour)
        {
            iHour=SystemHour-SystemHour%2;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy4Hour)
        {
            iHour=SystemHour-SystemHour%4;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy6Hour)
        {
            iHour=SystemHour-SystemHour%6;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy8Hour)
        {
            iHour=SystemHour-SystemHour%8;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy12Hour)
        {
            if(IniConfig.iN10UploadProductMethod==0)                            //JerryYang 20190131 上傳production log可選擇00:00 or 08:00
            {
                iHour=SystemHour-SystemHour%12;
                sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
            }
            else
            {
                if(SystemHour<8)
                {
                    GetYesterdayInfo();
                    sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.csv", sPathName, HTFileName, SystemYearYesterday, SystemMonthYesterday, SystemDateYesterday, 20);
                }
                else if(SystemHour>=20)
                {
                    sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, 20);
                }
                else
                {
                    sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, 8);
                }
            }
        }
        else if(HTSaveType==TByDay)
        {
            if(CUSTOMER_CODE==CC_CYPRESS &&
               LastSet.iTester==OFF_LINE)                                       //JerryYang 20230717 : Cypress要求區分Online/Offline
            {
                sFileName.sprintf("%s\\%s_%04d%02d%02d_TesterOffline.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate);
            }
            else
            {
                if(bUseFTRT)
                {
                    if(LastSet.iRunStartMode==rsmContinuRetest ||
                       LastSet.iRunStartMode==rsmCInitialRetest)
                        sFileName.sprintf("%s\\%s_%04d%02d%02d_RT.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate);
                    else
                        sFileName.sprintf("%s\\%s_%04d%02d%02d_FT.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate);
                }
                else
                {
                    sFileName.sprintf("%s\\%s_%04d%02d%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate);
                }
            }
        }
        else if(HTSaveType==TByMonth)
        {
            sFileName.sprintf("%s\\%s_%04d%02d.csv", sPathName, HTFileName, SystemYear, SystemMonth);
        }
        else if(HTSaveType==TByMin)
        {
            if(bUseFTRT)
            {
                if(LastSet.iRunStartMode==rsmContinuRetest ||
                   LastSet.iRunStartMode==rsmCInitialRetest)
                    sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d%02d_RT.TXT", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
                else
                    sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d%02d_FT.TXT", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
            }
            else
            {
                sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d%02d.TXT", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
            }
        }
        else //if(HTSaveType==TByYear)
        {
            sFileName.sprintf("%s\\%s_%04d.csv", sPathName, HTFileName, SystemYear);
        }
    }

    return sFileName;
}
//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: GOLDEN QUIRK preserved VERBATIM (golden
//  :648-651) -- the file is opened once with CreateFile(GENERIC_READ) purely as
//  an existence/lock probe and immediately CloseHandle'd, then re-opened with
//  std::ifstream to actually count lines.  Redundant (FileExists already ran on
//  the line above), but it is also the only thing that makes a locked file skip
//  the count instead of returning 0, so it is behaviour, not dead code.  Kept.
//---------------------------------------------------------------------------
int TMyStringList::GetLastLine()                                                //Steven 20191016 : 取得目前檔案的行數
{
    AnsiString sFileName=GetFileName();
    int iCount=0;
    GetTimeInfo();
    if(FileExists(sFileName))                                                   //Steven 20200221 : 修正沒有檔案就不要讀檔
    {
        HANDLE hFile = CreateFile(sFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile!=INVALID_HANDLE_VALUE)                                        //JimmyChiu : 20250401 : 變更開檔案方式
        {
            CloseHandle(hFile);
            std::ifstream file(sFileName.c_str());
            if (file.is_open())
            {
                std::string line;
                while (std::getline(file, line))
                {
                    iCount++;
                }
                file.close();
            } else
            {
                iCount=0;
            }
        }
    }
    sLastFileName=sFileName;
    return iCount;
}
//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: SaveTryCatchLog is a FREE function, not a
//  member -- so the SystemYear/SystemMonth/... it formats are the GLOBALS
//  (cmydef.h:226-227), NOT the class's shadowing members, and nothing here
//  refreshes them first.  The timestamp is therefore whatever the last global
//  time refresh elsewhere in the process left behind.  Golden's structure,
//  reproduced exactly.  Not fixed.
//---------------------------------------------------------------------------
void SaveTryCatchLog(AnsiString asMessage, AnsiString Func)                     //KaiChen 20200706 ：增加 Try Catch 抓問題
{
    AnsiString sPathName, sFileName, sMegTime, asLog;

    sPathName.sprintf("D:\\HT9045_Log\\EventLogTxt\\TryCatchLog\\%04d", SystemYear);
    sFileName.sprintf("%s\\TryCatchLog.txt", sPathName);
    MyForceDirectories(sPathName);
    sMegTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d %03d",
                        SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);

    asLog=sMegTime+", "+Func+", "+asMessage;
    WriteDataToFile(sFileName.c_str() , asLog.c_str());
}
//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: TWO GOLDEN BUGS preserved VERBATIM here.
//  (1) golden :695-698 -- when the target file does NOT yet exist, the method
//      only recomputes sLastFileName and RETURNS WITHOUT EVER WRITING `Msg`.
//      The very first alarm line into a fresh log file is silently dropped.
//  (2) golden :735 -- the catch handler dereferences the GLOBAL `slEventLog`
//      (`slEventLog->Text`) with no NULL guard, from inside a handler that only
//      runs when something already went wrong; if slEventLog is NULL this turns
//      a caught exception into an access violation.  Note also that `->Text` is
//      the BASE TStringList's text, and this class never Adds to itself (every
//      Add* goes to the inner `MyList`), so the logged message is always "".
//  Neither is fixed.
//---------------------------------------------------------------------------
void TMyStringList::MyInsertToFile(AnsiString Msg, int iCount)
{
//    if(HTAutoSave==false || MyList->Count==0)                                   //沒資料就不用存檔
//        return;

    TStringList *File;
    File=new TStringList();

    try                                                                         //KaiChen 20200706 ：增加 Try Catch 抓問題
    {
        if(FileExists(sLastFileName)==false)
        {
            sLastFileName=GetFileName();
        }
        else                                                                    //Steven 20200221 : 修正沒有檔案就不要讀檔
        {
            HANDLE hFile = CreateFile(
                sLastFileName.c_str(),       // 檔案名稱
                GENERIC_WRITE,               // 寫入權限
                FILE_SHARE_READ,             // 允許其他程式讀取
                NULL,                        // 安全性屬性
                OPEN_ALWAYS,                 // 如果檔案不存在則建立，已存在則開啟
                FILE_ATTRIBUTE_NORMAL,       // 一般檔案屬性
                NULL                         // 無模板檔案
            );

            if(hFile!=INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);
                File->LoadFromFile(sLastFileName);
//                Str=MyList->Text.SubString(0, MyList->Text.Length()-2);       //Steven 20251107 : 修正event log存檔
//                if(IniConfig.bSPILFunction)                                     //JerryYang 20250428 : fix SPIL event log
//                {
//                }
//                else
//                {
//                    Msg.Insert(' ', 12);
//                    Msg.Insert(' ', 26);
//                }

                if(FileExists(sLastFileName) && File->Count>iCount && iCount>0) //Steven 20200215 : 加上保護機制
                    File->Insert(iCount, Msg);
                else
                    File->Add(Msg);
                File->SaveToFile(sLastFileName);                                //Steven 20191107 : 紀錄現在的檔名
            }
        }
    }
    catch(...)
    {
        SaveTryCatchLog(slEventLog->Text, AnsiString("ShowErrorMessage_MyInsertToFile"));
    }
    File->Clear();
    MyList->Clear();
    delete File;
}
//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: GOLDEN QUIRK preserved VERBATIM (golden
//  :750) -- the early-out is `HTAutoSave==false && bDelete==false` (AND, not
//  the OR used by every sibling save method), so a delete request always runs
//  even with AutoSave off.  That is deliberate for the delete path; the comment
//  "沒資料就不用存檔" carried over from the siblings is what makes it look wrong.
//  Note also the local `Word Date;` SHADOWS vclcompat's global Date() function
//  for the rest of this body -- harmless, and identical to BCB6's SysUtils Date().
//---------------------------------------------------------------------------
void TMyStringList::MySaveSGJamCountToFile(bool bDelete, bool bflag)            //KaiChen 20200618 ：矽格，增加Jam統計頁面
{
    AnsiString Str;
    FILE *pFile;
    Word Year,Month,Date;
    GetTimeInfo();
    GetYesterdayInfo();

    if(HTAutoSave==false && bDelete==false)                                     //沒資料就不用存檔
        return;

    if(HTPath=="")
        Path="D:\\HandlerLog";

    AnsiString sPathName;
    AnsiString sFileName;

    if(bflag)
    {
        Year    =SystemYearYesterday;
        Month   =SystemMonthYesterday;
        Date    =SystemDateYesterday;
    }
    else
    {
        Year    =SystemYear;
        Month   =SystemMonth;
        Date    =SystemDate;
    }

    sPathName.sprintf("%s\\SGJamCount\\%04d\\%02d", HTPath, Year, Month);
    MyForceDirectories(sPathName, "TMyStringList::MySaveSGJamCountToFile");

    if(HTSaveType==TByMaxLineCount)
        sFileName.sprintf("%s\\%s_%04d%02d%02d %02d%02d%02d.csv", sPathName, HTFileName, Year, Month, Date, SystemHour, SystemMin, SystemSec);
    else
        sFileName.sprintf("%s\\%s_%04d%02d%02d_RawData.csv", sPathName, HTFileName, Year, Month, Date);

    if(bDelete)
    {
        DeleteFile(sFileName);
    }
    else
    {
        if(FileExists(sFileName)==false && HTFirstRow!="")
        {
            Str=HTFirstRow+"\r\n"+MyList->Text;
        }
        else
        {
            Str=MyList->Text;
        }

        Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);
        pFile=fopen(sFileName.c_str(), "a");
        if(pFile!=NULL)
        {
            fputs(Str.c_str(), pFile);
            fclose(pFile);
        }
    }
    MyList->Clear();
}
//------------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: FOUR GOLDEN BUGS preserved VERBATIM here.
//  (1) golden :945 `if(iCol>0)` -- iCol is the 0-BASED row index of the matched
//      2D-ID (golden :874 `iCol=i;`), so a match on the FIRST row (i==0) is
//      indistinguishable from "no match" and the updated row is silently thrown
//      away.  The very first IC of every mapping file cannot be written back.
//  (2) golden :927-940 -- the `str3` rebuild loop runs OUTSIDE the row loop, so
//      when no row matches it serialises whatever `list2D` was last loaded with
//      (the LAST row of the file), not the matched row.  Harmless only because
//      bug (1)'s `iCol>0` test then also fails.
//  (3) golden :872 `list2D->Strings[0]` is read before any Count check; on an
//      empty/blank line the BCB6 original raises EStringListError.  vclcompat's
//      TStringList::GetString returns "" out of range instead of throwing
//      (vclcompat/TStringList.cpp:76-79), so the port degrades to a silent
//      no-match where golden would throw -- a vclcompat property, flagged not
//      patched.
//  (4) golden :959 -- when the mapping file does not exist yet, `File` was never
//      loaded and `File->SaveToFile(sFileName)` writes an EMPTY file, discarding
//      the incoming s2DID entirely (even for CC_QUALCOMM, whose "append missing
//      row" branch at :951 only runs on the file-exists path).
//  None are fixed.
//------------------------------------------------------------------------------
void TMyStringList::MyInsert2DMappingToFile(AnsiString s2DID, int iPlaceToUnload)  //JerryYang 20230322 : add 2D mapping result
{
    if(TestIF_File.bEnableBarCode    ==false &&                                 //Nickliu 20230810 retrun for debug
       TestIF_File.bSortingBy2DIDList==false)
    {
        return ;
    }

    AnsiString sTestCate;                                                       //RogerYang 20260120 : Add for RF360 2DID sorting
    if(CUSTOMER_CODE==CC_QUALCOMM)
    {
        if(fLotInfo->edtSysLotID->Text=="")
        {
            return;
        }
        else
        {
            if(iPlaceToUnload>=0)
                sTestCate=AnsiString(iPlaceToUnload);
            else
                sTestCate="E";
        }
    }
    else if(fLotInfo->edtSysLotID->Text =="" ||
       fLotInfo->edtCusLotID->Text =="" ||
       fLotInfo->edtCusDevGrp->Text=="")
    {
        return;
    }

    AnsiString Str, str, str2="", str3="", sFileName;

    sFileName.sprintf("D:\\HT9045_Log\\2D_MappingResult\\%s_%s_%s_VS_Result.csv", fLotInfo->edtSysLotID->Text, fLotInfo->edtCusLotID->Text, fLotInfo->edtCusDevGrp->Text);

    int iCol=0;
    TStringList *File, *list2D;
    File=new TStringList();
    list2D=new TStringList();

    MyForceDirectories("D:\\HT9045_Log\\2D_MappingResult\\", "TMyStringList::MyInsert2DMappingToFile");

    if(FileExists(sFileName))
    {
        File->LoadFromFile(sFileName);
        for(int i=0; i<File->Count; i++)
        {
            AnsiString s,s1,s2,s3,s4;
            int iPos1,iPos2;
            s=File->Strings[i];

            iPos1=s.Pos(" ");
            iPos2=s.Pos(",");
            if(iPos1>0 && iPos2>0 && iPos1<iPos2)                               //表示2D有空格
            {
                s1=s.SubString(1, iPos2-1);                                     //2D
                s3=s.SubString(iPos2+1, s.Length());                            //2D
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
                iCol=i;

                if(CUSTOMER_CODE==CC_QUALCOMM)                                  //RogerYang 20260121 : Add for RF360 2DID sorting 塞bin結果
                {
                    list2D->Strings[10]=sTestCate;
                    break;
                }

                if(fLotInfo->cbRunMode->Text=="VS")
                {
                    if(list2D->Count>=11)
                    {
                        if(iPlaceToUnload>=0)
                        {
                            list2D->Strings[6]=iPlaceToUnload+1;
                        }
                        else
                        {
                            list2D->Strings[5]="V";
                        }
                    }
                }
                else if(fLotInfo->cbRunMode->Text=="RC1")
                {
                    if(list2D->Count>=11)
                    {
                        if(iPlaceToUnload>=0)
                        {
                            list2D->Strings[8]=iPlaceToUnload+1;
                        }
                        else
                        {
                            list2D->Strings[7]="V";
                        }
                    }
                }
                else
                {
                    if(list2D->Count>=11)
                    {
                        if(iPlaceToUnload>=0)
                        {
                            list2D->Strings[10]=iPlaceToUnload+1;
                        }
                        else
                        {
                            list2D->Strings[9]="V";
                        }
                    }
                }
                break;
            }
        }
        str3="";
        for(int i=0; i<list2D->Count; i++)
        {
            str=list2D->Strings[i];
            if(i==0)
            {
                str=StringReplace(str, '"', "", TReplaceFlags()<<rfReplaceAll);
                str3=str;
            }
            else
            {
                str3=str3+","+str;
            }
        }
    }

    if(FileExists(sFileName))
    {
        if(iCol>0)
        {
            File->Delete(iCol);
            File->Insert(iCol, str3);
            File->SaveToFile(sFileName);
        }
        else if(CUSTOMER_CODE==CC_QUALCOMM)                                     //RogerYang 20260121 : Add for RF360 2DID sorting 沒有出現在LIST，需新增資料
        {
            File->Add(s2DID+",,,,,,,,,,"+sTestCate);
            File->SaveToFile(sFileName);
        }
    }
    else
    {
        File->SaveToFile(sFileName);
    }

    File->Clear();
    MyList->Clear();
    delete File;

    list2D->Clear();
    delete list2D;
}
//------------------------------------------------------------------------------
