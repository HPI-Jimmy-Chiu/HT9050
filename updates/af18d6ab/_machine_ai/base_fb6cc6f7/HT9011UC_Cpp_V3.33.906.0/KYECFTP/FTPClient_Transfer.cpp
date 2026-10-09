// ===========================================================================
//  KYECFTP/FTPClient_Transfer.cpp
//  Implementation of the 4 FTP-transfer session functions. See the .h for the
//  full provenance / STATE DESIGN notes -- this file additionally documents
//  every gated dependency and discovered-quirk decision inline, at its exact
//  call site.
//
//  AI(W5-Final-FTPClientTransfer) 20260711: new file.
// ===========================================================================
#include "KYECFTP/FTPClient_Transfer.h"

#include "cmydef.h"            // CUSTOMER_CODE, CC_*, bSigurd*, FTPAutomation_*,
                                // iAMD_Function, bHasFTPDownload, asSetupFileCheckList,
                                // sATCPath, ATC_SYSTEM -- transitively MachineType.h
                                // (CC_* #defines, eNewATCSystem)
#include "cprod.h"              // Temperature (SYSTEM_TEMPERATURE, .bATCActiveCooling)
#include "Config.h"             // IniConfig (HT9045_CONFIG)
#include "CosFunction.h"        // CosFunction (HT9045_COUSTOMER_FUNCTION)
#include "common.h"             // DataPath/OffsetPath/DefaultPath/LastDataPath/
                                // asATCFileTransferPath/sWhite2DIDListLoc
#include "canary_support.h"     // ShowMyMessage/RecordProcess -- real Sim bodies,
                                // already linked by every other translated leaf
                                // that needs them. Hard-boundary file: included
                                // (read-only use) only, never edited.

#include <cstdlib>              // system()
#include <cstdio>

#if defined(_WIN32)
#  include <windows.h>          // Sleep, FindFirstFileA/FindClose, SetCurrentDirectory,
                                 // WIN32_FIND_DATA, INVALID_HANDLE_VALUE
#endif

// ---------------------------------------------------------------------------
//  MyDBIProcess -- golden cMyDB.h:20 (`void __fastcall MyDBIProcess(AnsiString
//  asTable, AnsiString S1, AnsiString S2="")`, 3-arg with a defaulted S2).
//  This unit's scope needs BOTH the 2-arg shape (`MyDBIProcess("Exception",
//  "<func> <file>")`, e.g. golden :249/261/273/285/632/651/670/689) AND the
//  real 3-arg shape (`MyDBIProcess("Exception", "<func>", e.Message)`, golden
//  :310/329). The already-real stub this whole tree shares
//  (`aHotPlateSubstrate.cpp:698`, hard-boundary, not editable here) is only
//  2-arg -- same gap the parallel FTPClient_EventHandlers.cpp sibling hit and
//  resolved the same way (its own local `extern` redeclaration of the 2-arg
//  form, following the established `CCLink/MyCCLink.cpp:36` precedent of NOT
//  #including the 796-line aHotPlateSubstrate.h just for this one extern).
//  This file ADDITIONALLY defines a 3-arg overload (internal linkage) that
//  folds S2+S3 into the 2-arg shape's second parameter -- resolves via normal
//  C++ overload resolution against the extern 2-arg one (different arg
//  count -> no ODR conflict, no hard-boundary file touched).
//  JUDGMENT CALL (flag for review): the exact real 3-column DB/grid write
//  layout behind the 3-arg golden signature is unknown (cMyDB.cpp's grid-
//  write body is itself gated elsewhere) -- folding S2+" : "+S3 into one
//  string is a reasonable, but unverified, stand-in.
// ---------------------------------------------------------------------------
extern void MyDBIProcess(AnsiString S1, AnsiString S2);

// ---------------------------------------------------------------------------
//  MySleep(DWORD) -- golden common.h:261. common.h's OWN declaration is
//  itself inside a `#if 0 // TODO(wave-timing)` block (common.cpp's body is
//  the actual gate; the header re-gates the declaration alongside it) -- so
//  `#include "common.h"` alone does not expose it. A REAL, ACTIVE definition
//  already exists in `acarry_shims.cpp` (confirmed by Automation/auto9045.cpp's
//  own identical `extern void MySleep(DWORD dwMilliseconds);` redeclaration
//  + comment "REAL definition: acarry_shims.cpp") -- reused via the same
//  local `extern` redeclaration here, not a third duplicate definition.
// ---------------------------------------------------------------------------
extern void MySleep(DWORD dwMilliseconds);

// ---------------------------------------------------------------------------
//  NewRecordProcess(AnsiString,AnsiString="",AnsiString="") -- golden's log
//  sink (real declaration lives in `acatchtray_shims.h`, a non-hard-boundary
//  shared header). Redeclared locally (matching the established
//  `CCLink/MyCCLink.cpp:36` / `FTPClient_EventHandlers.cpp:29` precedent of a
//  local `extern` rather than #including a large sibling header just for one
//  free-function signature).
// ---------------------------------------------------------------------------
extern void NewRecordProcess(AnsiString S1, AnsiString S2 = "", AnsiString S3 = "");

namespace {
// NOTE: this 3-arg overload of MyDBIProcess is declared in the SAME
// anonymous-namespace scope; per normal C++ unqualified-lookup rules calls
// to the bare `MyDBIProcess` name from WITHIN this namespace would find only
// this overload (the extern 2-arg one lives in an outer scope, and
// unqualified lookup stops at the first scope where the name is found) --
// the internal `::MyDBIProcess(...)` call below is deliberately
// GLOBAL-qualified to reach the real 2-arg extern instead.
void MyDBIProcess(AnsiString S1, AnsiString S2, AnsiString S3)
{
    ::MyDBIProcess(S1, S2 + " : " + S3);
}

// ---------------------------------------------------------------------------
//  Exception -- golden's ambient VCL `Exception` class (System.hpp), base of
//  every BCB6 exception, exposing `.Message` (AnsiString). Nothing in this
//  tree has needed it before (grepped globally -- zero hits for a bare
//  `class Exception` anywhere, in either vclcompat or any translated leaf).
//  JUDGMENT CALL (new additive infra, flag for review): kept file-LOCAL
//  (anonymous namespace, internal linkage) rather than hoisted into
//  vclcompat/vcl_compat.h -- that umbrella header is being concurrently
//  edited by other parallel units in this same batch (confirmed via `git
//  status` showing it already modified before this unit started), so adding
//  a new globally-visible type there risks a silent ODR collision if another
//  parallel unit independently invents the same primitive in a different
//  header. An anonymous-namespace-local class has internal linkage and is
//  therefore IMMUNE to any such cross-TU collision, at the cost of not being
//  reusable by a future sibling unit (which would need to invent its own, or
//  a human should hoist this one into vclcompat/ once the batch settles).
//
//  BEHAVIORAL NOTE (discovered gap, flag for review): NOTHING in this unit's
//  actual call graph ever throws an `Exception` -- MiniFtpEngine.h's own file
//  banner documents that Connect/ChangeDir/MakeDirectory/Download/Upload/
//  Nlist/Delete NEVER throw (failures surface only via events + Connected/
//  LastErrorNo state, by design). The ONE call in this unit's scope that CAN
//  genuinely throw a C++ exception -- `StrToInt(IniConfig.N06_FtpPort)` on a
//  malformed port string -- throws bare `std::runtime_error` (vclcompat/
//  AnsiString.h's own documented behavior), which `catch(Exception&)` does
//  NOT match (unrelated types). Every one of this unit's 4 functions ALSO
//  keeps golden's own trailing bare `catch(...)`, so that path still can't
//  escape the function uncaught -- but the `catch(Exception&e)` blocks
//  themselves are consequently DEAD CODE in this translation (preserved
//  verbatim for structural/textual fidelity to golden, not because they can
//  fire). See the "mkdir-then-cd-again" note at UploadFileToServer2's JCET
//  branch below for the one place this dead-ness has an actual behavioral
//  consequence worth flagging.
// ---------------------------------------------------------------------------
class Exception
{
public:
    AnsiString Message;
    explicit Exception(const AnsiString& Msg = AnsiString()) : Message(Msg) {}
};

// ---------------------------------------------------------------------------
//  delay(int ms) -- golden FTPClient.cpp:66-80: a literal busy-wait
//  `while(...) { Application->ProcessMessages(); ... }`, kept alive so the
//  VCL message pump could keep servicing the REAL (async) TNMFTP's socket
//  events while golden's own code "waited". MiniFtpEngine's Connect/Download/
//  Upload/Nlist/Delete are all SYNCHRONOUS (MiniFtpEngine.h's own "Async vs
//  Sync" design note) -- there is no message pump to service and nothing that
//  could still be "in flight" by the time control reaches one of these retry
//  loops, so `Application->ProcessMessages()` has no equivalent here and is
//  simply omitted; a portable blocking Sleep(ms) reproduces golden's only
//  OTHER observable effect (elapsed wall-clock time).
//  JUDGMENT CALL (flag for review): `g_FastDelayForTest` is an ADDITIVE
//  test-only escape hatch (NOT part of golden) -- golden's own constants
//  (50/100/200/500ms, several per call site) sum to multiple seconds per
//  scenario, which would make this unit's own tests slow for no behavioral
//  gain (MiniFtpEngine's Sim mode is already fully synchronous/deterministic,
//  so there is nothing for a real wall-clock wait to "let settle"). Defaults
//  to false (real Sleep, golden-faithful timing).
// ---------------------------------------------------------------------------
bool g_FastDelayForTest = false;

void FTPXferDelay(int ms)
{
    if (g_FastDelayForTest)
        return;
#if defined(_WIN32)
    if (ms > 0)
        ::Sleep(static_cast<DWORD>(ms));
#endif
}

// ---------------------------------------------------------------------------
//  dblFileSize -- golden FTPClient.cpp:82-105 (file-scope helper, needed by
//  LoadFileFormServer2's ftp-setup-file-size-check loop). Translated
//  faithfully, including a genuine pre-existing golden quirk: preserved, NOT
//  fixed (see inline note below).
// ---------------------------------------------------------------------------
double dblFileSize(const char* fname)
{
    // DISCOVERED GOLDEN QUIRK (preserved verbatim, NOT fixed): golden reads
    // `if(!fname && !*fname) return 0.0;` -- using `&&` where the evident
    // intent ("guard against a null OR an empty string") would need `||`.
    // With `&&`, if `fname` were ever genuinely NULL, C++ would still have to
    // evaluate the right operand `!*fname` to compute the `&&` (short-circuit
    // only skips the RIGHT operand when the LEFT is false) -- an actual
    // NULL-pointer dereference. This is dormant/unreachable in practice: this
    // unit's one call site always passes `AnsiString::c_str()`, which never
    // returns NULL (even for an empty AnsiString, it returns a pointer to a
    // real `'\0'`) -- so `!fname` is always false, the `&&` short-circuits
    // there, and `!*fname` is never evaluated. Kept exactly as golden wrote
    // it; flagged for human review in case a future caller ever passes a
    // possibly-NULL `const char*` here directly.
    if (!fname && !*fname)
        return 0.0;
#if defined(_WIN32)
    HANDLE h;
    WIN32_FIND_DATA info;
    if ((h = FindFirstFileA(fname, &info)) != INVALID_HANDLE_VALUE)
    {
        FindClose(h);
        if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) // Is it a file?
        {
            union
            {
                struct { DWORD low, high; } lh;
                __int64 size;                       // MS large int extension
            } file;
            file.lh.low  = info.nFileSizeLow;
            file.lh.high = info.nFileSizeHigh;
            return static_cast<double>(file.size);  // will be cast to double
        }
        // It's a directory, not a file
    }
#endif
    return 0.0;                                     // No such name.
}

// ===========================================================================
//  GATED DEPENDENCIES -- cross-form / cross-module calls this unit's 4
//  functions make into forms/units that have no facade home yet. Every one
//  is a TU-local, internal-linkage, conservative-default stand-in, cited at
//  its call site below and summarized in the hand-off report.
// ===========================================================================

// ---- Gate #1: GetLastOpenFN() (golden common.h:68 / common.cpp:1252-1281) -
//   Declared in common.h but the real BODY is gated `#if 0 // TODO(wave-path)`
//   in common.cpp (a different unit's file, out of THIS unit's write scope).
//   TU-local duplicate -- SAME pattern already used twice elsewhere in this
//   tree (Automation/auto9045.cpp's W5FA_GetLastOpenFN, Automation/
//   SCK_ART_Remainder.cpp's simplified stand-in) -- a faithful copy of
//   golden's real body (reads the first line of the LastData.inf-style file
//   at the already-real `LastDataPath` global via vclcompat's real
//   TStringList::LoadFromFile). Delete this 3rd duplicate once common.cpp's
//   gate lifts (would otherwise ODR-conflict with the real definition).
AnsiString FTPXfer_GetLastOpenFN()
{
    AnsiString Str = "Fail Open";
    vclcompat::TStringList MyList;
    if (FileExists(LastDataPath))
    {
        MyList.LoadFromFile(LastDataPath);
        if (MyList.Count == 0)                                   // Jimmychiu 20250426: fixed for the content of setup being empty
        {
            ShowMyMessage("The content of setup.inf is empty!");
        }
        else if (AnsiString(MyList.Strings[0]).Trim() == "")
        {
            ShowMyMessage("The content of setup.inf is NULL!");
        }
        else
        {
            Str = MyList.Strings[0];
        }
    }
    else
    {
        ShowMyMessage("LastData does not exist");
    }
    return Str;
}
#define GetLastOpenFN FTPXfer_GetLastOpenFN

// ---- Gate #2: fLotInfo->DownloadFromServer / DownloadFromServer_TSMC ------
//   golden uLotInfo.h:1278/1280 (`TfLotInfo` real methods -- read the whole
//   recipe-download pipeline into the live Prod/TestIF_File runtime state +
//   persists it). Not on FormsFacade.h's TfLotInfo today (grepped -- confirmed
//   absent). Both golden call sites discard the bool return value (bare
//   statements) -- conservative no-op stand-ins return false.
//   NEW FormsFacade wiring point (integrate agent -- see hand-off report):
//     bool DownloadFromServer(AnsiString sDLFileName, bool bFromFTP=true);
//     bool DownloadFromServer_TSMC(AnsiString sOrgFileName, AnsiString sDLFileName);
bool Gated_LotInfo_DownloadFromServer(const AnsiString& /*sDLFileName*/, bool /*bFromFTP*/ = true)
{
    return false;
}
bool Gated_LotInfo_DownloadFromServer_TSMC(const AnsiString& /*sOrgFileName*/, const AnsiString& /*sDLFileName*/)
{
    return false;
}

// ---- Gate #3: fNote->bSendJamCodeToFTP = false ----------------------------
//   golden FTPClient.cpp:811/832 (`TfNote::bSendJamCodeToFTP`, Note.h/.cpp --
//   Note.cpp is not translated at all yet). WRITE-ONLY from this unit's
//   perspective (golden unconditionally clears it; nothing in THIS unit's own
//   call graph ever reads it back) -- a bare no-op stand-in is behaviorally
//   complete for this unit; a future Note.cpp translation should replace
//   this with a real facade write.
void Gated_Note_ClearSendJamCodeToFTP()
{
}

// ---- Gate #4: fTesterTCP->CopyRecipeFromTester(sULFileName) --------------
//   golden Interface/TesterTCP.h (`TfTesterTCP::CopyRecipeFromTester`,
//   RichEdit/TMemo OS report-generation family). Explicitly flagged as NOT
//   attempted by the parallel Interface/TesterTCP_Socket.{h,cpp} sibling unit
//   (its own file banner lists CopyRecipeFromTester under "NOT attempted" --
//   confirmed by reading that header before writing this gate, per this
//   task's own instruction not to guess). No facade exists for it under any
//   name. Conservative no-op stand-in (golden call site discards nothing --
//   it's a bare statement with no return value used).
void Gated_TesterTCP_CopyRecipeFromTester(const AnsiString& /*sULFileName*/)
{
}

// ---- Gate #5: FileListBox1->Directory -------------------------------------
//   golden FTPClient.h:85 (`TFileListBox *FileListBox1`, __published VCL
//   widget) -- read exactly once, golden :399
//   (`AnsiString ActivePath=FileListBox1->Directory+"\\";`), only consumed by
//   the CC_TSMC_TAINAN zip-upload branch (golden :510-548).
//   JUDGMENT CALL (flag for review): no facade/UI exists to read a real bound
//   directory offline. Conservative default returns the already-real
//   `DataPath` global -- chosen because the PARALLEL non-TSMC branch just a
//   few lines below (golden :550-706) builds the IDENTICAL zip path shape
//   from `DataPath` instead of `ActivePath` for every other customer, so
//   `DataPath` is the best-evidenced stand-in for "wherever the real
//   FileListBox1 widget happens to be pointed at" for a machine with no UI.
AnsiString Gated_GetFileListBox1Directory()
{
    return DataPath;
}

// ---- Gate #6: fLotInfo->edtASECL_LotID->Text / cbRunModeASECL->Text -------
//   golden uLotInfo.h:576/1043 (`TEdit *edtASECL_LotID`, `TComboBox
//   *cbRunModeASECL`) -- not on FormsFacade.h's TfLotInfo today (grepped --
//   confirmed absent; distinct from the already-present `cbRunMode`/
//   `cbProcess` members). Conservative default: both return "" (no ASE_CL lot
//   active offline). Consequence (traced, not guessed): with LotID=="",
//   `str1` still gets a value (`"_ASSY.xml"`, golden's own unconditional
//   default assignment before the if/else-if chain, :5325); with
//   RunMode=="", NONE of the 7 named modes ("CORR"/"FT1"/"RT1"/.../"QT5")
//   match, so `str2` (the local download PATH) stays the empty AnsiString for
//   the whole function -- `FileExists("")` is false (skips the stale-file
//   delete) and `NMFTP2->Download(str1, "")` degrades to "download this file
//   to nowhere" (MiniFtpEngine's WriteWholeFile fails its own fopen("","wb")
//   quietly, no crash) rather than corrupting any real path. Flagged for
//   review since this is the one place a wrong default (a real ASE_CL LotID
//   guess) could plausibly matter for that specific customer.
AnsiString Gated_LotInfo_edtASECL_LotID_Text()
{
    return AnsiString("");
}
AnsiString Gated_LotInfo_cbRunModeASECL_Text()
{
    return AnsiString("");
}

} // anonymous namespace

// ===========================================================================
//  STATE DESIGN globals -- see the .h banner for full provenance. Defined
//  here (this is the one TU that owns them until a future sibling unit
//  translates the rest of FTPClient.cpp).
// ===========================================================================
bool           FTP_DownloadFail = false; // golden FTPClient.cpp:46
bool           bPIDTransferErr  = false; // golden FTPClient.h:159 (never assigned by this TU -- see .h note)
Nmftp::TNMFTP* NMFTP3           = NULL;  // golden FTPClient.cpp:1214 (never assigned by this TU -- see .h note
                                          // and the "DISCOVERED GOLDEN QUIRK" comment at Download_2DSortingList below)

void FTPClientTransfer_ResetStateForTest()
{
    FTP_DownloadFail = false;
    bPIDTransferErr  = false;
    NMFTP3           = NULL;
    FTPClientEvt_ResetStateForTest();
}

void FTPClientTransfer_SetFastDelayForTest(bool bFast)
{
    g_FastDelayForTest = bFast;
}

// ===========================================================================
//  LoadFileFormServer2 -- golden FTPClient.cpp:107-389.
// ===========================================================================
void LoadFileFormServer2(AnsiString FtpPath, AnsiString Source)
{
    TNMFTP* NMFTP2;                                       // Landam: use a dynamic component, more stable
    bool bRunOffset = true;
    double dFileSize[3] = {0, 0, 0};                      // Ifor 20160907 FTP Download File Size
    try
    {
        NMFTP2 = new TNMFTP(NULL);                        // golden: new TNMFTP(this) -- no owning form exists offline

        // -- parameters --------------------------------------------------------
        NMFTP2->Vendor  = NMOS_AUTO;
        NMFTP2->TimeOut = 20000;                          // Landam
        NMFTP2->Passive = true;                           // Steven 20121020: experiment
        if (bSigurdDownload_Recipe == true)
        {
            if (IniConfig.FtpHost != "")
                NMFTP2->Host = FTPAutomation_Down_ServerIP;
            NMFTP2->UserID   = FTPAutomation_Down_UserID;
            NMFTP2->Password = FTPAutomation_Down_Password;
        }
        else
        {
            if (IniConfig.FtpHost != "")
                NMFTP2->Host = IniConfig.FtpHost;
            NMFTP2->UserID   = IniConfig.FtpUserName;      // edtUserName->Text;
            NMFTP2->Password = IniConfig.FtpPassword;      // edtPassword->Text;
            NMFTP2->Port     = StrToInt(IniConfig.N06_FtpPort); // Ifor 20201015: user-defined FTP port
        }

        // -- events --------------------------------------------------------------
        FTPClientEvt_WireNMFTP1Handlers(*NMFTP2);          // golden's own 9-line NMFTP1* wiring block (:137-145)

        // -- connect --------------------------------------------------------------
        NMFTP2->Connect();

        for (int i = 0; i < 10; ++i)
        {
            FTPXferDelay(50);
            if (NMFTP2->Connected)
                break;
        }

        if (!NMFTP2->Connected)
        {
            ShowMyMessage("FTP Server is not connected", "");
            delete NMFTP2;
            bError = true;

            FTP_DownloadFail = true;                       // 2013.12.03, Joye, KYEC FTP  20140103 wei

            return;
        }

        // -- downloaded work-file gets a "_NET" suffix -------------------------
        AnsiString str01, str02, str03, str04, str05, str06; // JerryYang 20190906: ATC work-file goes via Handler upload/download
        AnsiString sDLFileName = Source;

        if (CUSTOMER_CODE == CC_KYEC_LEE && Source.Pos(asSetupFileCheckList) != 0)
        {
            str01 = "D:\\HT9045\\system\\" + asSetupFileCheckList;
            str02 = Source;
            bRunOffset = false;
        }
        else
        {
            sDLFileName = sDLFileName + "_NET";
            str01 = DataPath + sDLFileName + ".zip";
            str02 = Source + ".zip";
            str03 = OffsetPath + sDLFileName + ".Offset";
            str04 = Source + ".Offset";                    // JerryYang 20170727 (Steven): renamed Offset->offset, filenames must match exactly or download fails
            if (CosFunction.bUseATCFileTransfer == true)    // Eastsun 20260522 integration: ATC FileTransfer uses .dat
            {
                str05 = DataPath + sDLFileName + "ATC_Recipe" + ".dat";
                str06 = Source + "ATC_Recipe" + ".dat";
            }
            else
            {
                str05 = DataPath + sDLFileName + "ATC_Recipe" + ".zip"; // JerryYang 20190906: ATC work-file goes via Handler upload/download
                str06 = Source + "ATC_Recipe" + ".zip";
            }

            if (CUSTOMER_CODE == CC_ASE_N ||                // JerryYang 20170727 (Steven): ASE Nantong doesn't download offset
                IniConfig.bSIGURDFunction ||                // KaiChen 20200611: SIGURD doesn't download offset
                IniConfig.bVTESTFunction == true ||
                CUSTOMER_CODE == CC_JSCC_OS)                // RogerYang 20260127: JSCC_OS add
            {
                bRunOffset = false;
            }
        }

        if (FtpPath.SubString(FtpPath.Length(), 1) != "/")
        {
            NMFTP2->ChangeDir(FtpPath + "/");
        }
        else
        {
            NMFTP2->ChangeDir(FtpPath);
        }

        if (CUSTOMER_CODE == CC_PTI)                        // Sam 20231221: PTI reported a 10035 error popping up.
        {
        }
        else
        {
            if (IniConfig.FtpTransMode == 0)                // Steven 20230719: add FTP transfer mode
                NMFTP2->Mode(MODE_ASCII);
            else if (IniConfig.FtpTransMode == 1)
                NMFTP2->Mode(MODE_IMAGE);
            else
                NMFTP2->Mode(MODE_BYTE);
        }

        for (int i = 0; i < 3; i++)                          // Ifor 20160907 add ftp setup file check size
        {
            FTPXferDelay(200);
            NMFTP2->Download(str02, str01);                  // download the file
            for (int j = 0; j < 10; j++)
            {
                FTPXferDelay(100);
            }
            dFileSize[i] = dblFileSize(str01.c_str());
            RecordProcess("FTP DownLoad File Size" + IntToStr(i + 1) + "_" + FloatToStr(dFileSize[i]) + "KB"); // yunghsin 20160906
        }
        bHasFTPDownload = true;                              // Ifor 20240422 add: FTP work-file download
        if (CUSTOMER_CODE == CC_AMD_M && iAMD_Function == 1) // Ifor 20240419 add: upload password-related data
        {
            try
            {
                FTPXferDelay(200);
                str01 = "D:\\HT9045\\Error\\English\\JAM0000.dat";
                str02 = "/JAM0000.dat";
                NMFTP2->Download(str02, str01);
            }
            catch (...)
            {
                MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2 JAM0000.dat");
            }

            try
            {
                FTPXferDelay(200);
                str01 = "C:\\winnt\\system32\\tech.com";
                str02 = "/tech.com";
                NMFTP2->Download(str02, str01);
            }
            catch (...)
            {
                MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2 tech.com");
            }

            try
            {
                FTPXferDelay(200);
                str01 = "d:\\HT9045\\system\\levelset.dat";
                str02 = "/levelset.dat";
                NMFTP2->Download(str02, str01);
            }
            catch (...)
            {
                MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2 levelset.dat");
            }

            try
            {
                FTPXferDelay(200);
                str01 = "D:\\HT9045\\system\\unknown.com";
                str02 = "/unknown.com";
                NMFTP2->Download(str02, str01);
            }
            catch (...)
            {
                MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2 unknown.com");
            }
        }

        if (bRunOffset == true)
        {
            if (CUSTOMER_CODE == CC_TSMC_TAINAN)             // ChungHung 20150413 add for TSMC
            {
                Gated_LotInfo_DownloadFromServer_TSMC(Source, sDLFileName); // golden: fLotInfo->DownloadFromServer_TSMC(...) -- Gate #2
            }
            else if (CUSTOMER_CODE == CC_Greatek)            // Sam 20171019 (wei): after Setup File Download completes, delete the original setup file, keep only one on this machine
            {
                AnsiString strSetupfile = GetLastOpenFN();
                Gated_LotInfo_DownloadFromServer_TSMC(strSetupfile, sDLFileName); // golden: fLotInfo->DownloadFromServer_TSMC(...) -- Gate #2
            }
            else
            {
                try
                {
                    FTPXferDelay(200);
                    NMFTP2->Download(str04, str03);          // jou 2012-12-22: download the Offset file (start)
                    FTPXferDelay(100);
                }
                catch (Exception& e)
                {
                    MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2", e.Message);
                }
                catch (...)
                {
                    MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2");
                }

                if (CosFunction.bUseFTPDownLoadATCRecipe == true && // Ifor 20191115: add FTP DownLoad ATC Recipe
                    ATC_SYSTEM == eNewATCSystem &&                  // JerryYang 20190906: ATC work-file goes via Handler upload/download
                    Temperature.bATCActiveCooling == true)
                {
                    try
                    {
                        FTPXferDelay(200);
                        NMFTP2->Download(str06, str05);
                        FTPXferDelay(100);
                    }
                    catch (Exception& e)
                    {
                        MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2", e.Message);
                    }
                    catch (...)
                    {
                        MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2");
                    }
                }
                Gated_LotInfo_DownloadFromServer(sDLFileName); // golden: fLotInfo->DownloadFromServer(sDLFileName) -- Gate #2
            }
        }
        else if (CUSTOMER_CODE == CC_ASE_N ||                 // JerryYang 20170727 (Steven): ASE Nantong doesn't download offset
                 IniConfig.bSIGURDFunction ||                  // KaiChen 20200611: SIGURD doesn't download offset
                 IniConfig.bVTESTFunction == true ||
                 CUSTOMER_CODE == CC_JSCC_OS)                  // RogerYang 20260127: JSCC_OS add
        {
            Gated_LotInfo_DownloadFromServer(sDLFileName);     // golden: fLotInfo->DownloadFromServer(sDLFileName) -- Gate #2
        }

        if (CUSTOMER_CODE == CC_PTI)                            // Sam 20231221: PTI reported a 10035 error popping up.
        {
        }
        else
        {
            NMFTP2->Mode(MODE_ASCII);                           // Steven 20230719: add FTP transfer mode
        }
        NMFTP2->Abort();
        FTPXferDelay(100);
        NMFTP2->RequestCloseSocket();
        FTPXferDelay(500);

        FTP_DownloadFail = false;                               // 2013.12.03, Joye, KYEC FTP  20140103 wei
        delete NMFTP2;

        if (dFileSize[0] != dFileSize[1] ||                      // Ifor 20160907 add ftp setup file check size
            dFileSize[1] != dFileSize[2] ||
            dFileSize[2] != dFileSize[0])
        {
            FTP_DownloadFail = true;
            ShowMyMessage("FTP DownLoad File Size Error");
        }
        else
        {
            RecordProcess("FTP DownLoad File Size Verify Ok!");
        }
    }
    catch (Exception& e)                                        // DEAD in this translation -- see the file-head "Exception" note
    {
        MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2", e.Message);
        FTP_DownloadFail = true;                                 // 2013.12.03, Joye, KYEC FTP       20140103 wei
        bError = true;
        delete NMFTP2;
    }
    catch (...)
    {
        MyDBIProcess("Exception", "TfFTPClient::LoadFileFormServer2");
        FTP_DownloadFail = true;                                 // 2013.12.03, Joye, KYEC FTP       20140103 wei
        bError = true;
        delete NMFTP2;
    }
}

// ===========================================================================
//  UploadFileToServer2 -- golden FTPClient.cpp:390-846.
// ===========================================================================
void UploadFileToServer2(AnsiString FtpPath, AnsiString Source, bool bZip) // ChungHung 20140108 add FTP unload jam code
{
    SetCurrentDirectory("D://");                       // golden: SetCurrentDirectory(_T("D://")) -- _T() neutralized (ANSI build, _T(x)==x)
    TNMFTP* NMFTP2;                                     // Landam: use a dynamic component, more stable
    AnsiString str, str1, str2, str3;
    AnsiString sULFileName = Source;
    AnsiString ServerDir;
    AnsiString str01, str02;
    AnsiString ActivePath = Gated_GetFileListBox1Directory() + "\\"; // golden: FileListBox1->Directory -- Gate #5 // ChungHung 20150413 add for TSMC

    if (CUSTOMER_CODE == CC_KYEC_LEE || CUSTOMER_CODE == CC_KYEC_XILINX)
        bZip = true;

    try
    {
        NMFTP2 = new TNMFTP(NULL);                      // golden: new TNMFTP(this) -- no owning form exists offline

        // -- parameters ----
        NMFTP2->Vendor  = NMOS_AUTO;
        NMFTP2->TimeOut = 20000;                        // Landam
        NMFTP2->Passive = true;                         // Steven 20121020: experiment

        if (bSigurdUpload_Jamcode == true || bSigurdUpload_Recipe == true) // KaiChen 20190530: Sigurd FTP Automation
        {
            if (IniConfig.FtpHost != "")
                NMFTP2->Host = FTPAutomation_Up_ServerIP;
            NMFTP2->UserID   = FTPAutomation_Up_UserID;
            NMFTP2->Password = FTPAutomation_Up_Password;
        }
        else
        {
            if (IniConfig.FtpHost != "")
                NMFTP2->Host = IniConfig.FtpHost;
            NMFTP2->UserID   = IniConfig.FtpUserName;
            NMFTP2->Password = IniConfig.FtpPassword;
            NMFTP2->Port     = StrToInt(IniConfig.N06_FtpPort); // Ifor 20201015: user-defined FTP port
        }
        // -- events ----
        FTPClientEvt_WireNMFTP1Handlers(*NMFTP2);        // golden's own 9-line NMFTP1* wiring block (:429-437)

        // -- connect -----
        NMFTP2->Connect();
        if (bZip)                                        // ChungHung 20140108 add FTP unload jam code
        {
            for (int i = 0; i < 10; ++i)
            {
                FTPXferDelay(50);
                if (NMFTP2->Connected)
                    break;
            }

            if (!NMFTP2->Connected)
            {
                ShowMyMessage("FTP Server is not connected", "");
                delete NMFTP2;
                bError = true;
                return;
            }
        }
        else                                              // ChungHung 20140714: removed "regardless of zip", always check whether connected
        {
            for (int k = 0; k < 5; k++)
            {
                for (int i = 0; i < 10; ++i)
                {
                    FTPXferDelay(50);
                    if (NMFTP2->Connected)
                        break;
                }

                if (NMFTP2->Connected)
                {
                    break;
                }
                else
                {
                    NMFTP2->Abort();
                    MySleep(10);
                    NMFTP2->RequestCloseSocket();
                    MySleep(10);
                    NMFTP2->Connect();
                }
            }

            if (!NMFTP2->Connected)
            {
                ShowMyMessage("FTP Server is not connected", "");
                NMFTP2->Abort();
                NMFTP2->RequestCloseSocket();
                delete NMFTP2;

                bError = true;
                return;
            }
        }

        if (CUSTOMER_CODE == CC_PTI)                       // Sam 20231221: PTI reported a 10035 error popping up.
        {
        }
        else
        {
            if (IniConfig.FtpTransMode == 0)                // Steven 20230719: add FTP transfer mode
                NMFTP2->Mode(MODE_ASCII);
            else if (IniConfig.FtpTransMode == 1)
                NMFTP2->Mode(MODE_IMAGE);
            else
                NMFTP2->Mode(MODE_BYTE);
        }

        if (bZip)                                          // ChungHung 20140108 add FTP unload jam code
        {
            if (CUSTOMER_CODE == CC_TSMC_TAINAN)             // ChungHung 20150413 add for TSMC
            {
                // delete the old zip file first
                str.sprintf("del %s%s.zip", ActivePath, sULFileName); // ChungHung 20150413 add for TSMC
                system(str.c_str());

                // prepare 7-Zip
                if (FileExists("d:\\HT9045\\7z.exe") == false)
                {
                    CopyFile("C:\\Program Files\\7-Zip\\7z.exe", "d:\\HT9045\\7z.exe", false);
                }

                if (CosFunction.bUseATCFileTransfer == true && ATC_SYSTEM == eNewATCSystem && Temperature.bATCActiveCooling == true && bPIDTransferErr == false) // Eastsun 20260522 integration
                {
                    if (DirectoryExists(asATCFileTransferPath))
                    {
                        str01 = DataPath + sULFileName + "\\" + sULFileName + ".dat";
                        str02 = asATCFileTransferPath + sULFileName + ".dat";
                        CopyFile(str02.c_str(), str01.c_str(), false);
                    }
                    else
                    {
                        str.sprintf("Upload ATC recipe fail, check below path", asATCFileTransferPath);
                        RecordProcess(str);
                    }
                }

                // compress the work-file
                str.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s%s.zip\" \"%s%s\\*.*\"", ActivePath, sULFileName, ActivePath, sULFileName); // ChungHung 20150413 add for TSMC
                system(str.c_str());

                // upload the work-file
                str01 = ActivePath + sULFileName + ".zip";      // ChungHung 20150413 add for TSMC
                str02 = FtpPath + sULFileName + ".zip";
                NMFTP2->Upload(str01, str02);

                str.sprintf("del \"%s%s.zip\"", ActivePath, sULFileName); // finally delete the zip file. // ChungHung 20150413 add for TSMC
                system(str.c_str());
            }
            else
            {
                // delete the old zip file first
                str.sprintf("del %s%s.zip", DefaultPath, sULFileName);
                system(str.c_str());

                // prepare 7-Zip
                if (FileExists("d:\\HT9045\\7z.exe") == false)
                {
                    CopyFile("C:\\Program Files\\7-Zip\\7z.exe", "d:\\HT9045\\7z.exe", false);
                }

                Gated_TesterTCP_CopyRecipeFromTester(sULFileName); // golden: fTesterTCP->CopyRecipeFromTester(...) -- Gate #4 // Steven 20250327: OS tester's work-file also needs uploading

                // compress the work-file
                str.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s%s.zip\" \"%s%s\\*.*\"", DataPath, sULFileName, DataPath, sULFileName);
                system(str.c_str());

                // compress the Offset
                str.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s%s.Offset\" \"%s%s\\*.*\"", OffsetPath, sULFileName, OffsetPath, sULFileName);
                system(str.c_str());

                // compress the ATC work-file
                if (CosFunction.bUseFTPDownLoadATCRecipe == true &&
                    ATC_SYSTEM == eNewATCSystem &&
                    Temperature.bATCActiveCooling == true) // JerryYang 20190906: ATC work-file goes via Handler upload/download // Ifor 20191115: add FTP DownLoad ATC Recipe
                {
                    if (DirectoryExists(sATCPath))
                    {
                        str.sprintf("d:\\HT9045\\7z.exe a -tzip \"%s%s.zip\" \"%s\\*%s*\"", DataPath, sULFileName + "ATC_Recipe", sATCPath, sULFileName + "_");
                        system(str.c_str());
                    }
                    else
                    {
                        ShowMyMessage("Upload ATC recipe fail, check below path", sATCPath);
                    }
                }

                // upload the work-file
                str01 = DataPath + sULFileName + ".zip";
                str02 = FtpPath + sULFileName + ".zip";
                NMFTP2->Upload(str01, str02);

                // upload the Offset file
                str01 = OffsetPath + sULFileName + ".Offset";
                str02 = FtpPath + sULFileName + ".Offset";
                NMFTP2->Upload(str01, str02);

                // upload the ATC work-file
                if ((CosFunction.bUseFTPDownLoadATCRecipe == true || CosFunction.bUseATCFileTransfer == true) && // Eastsun 20260522 integration
                    ATC_SYSTEM == eNewATCSystem &&
                    Temperature.bATCActiveCooling == true && bPIDTransferErr == false) // Ifor 20191115: add FTP DownLoad ATC Recipe
                {
                    if (CosFunction.bUseATCFileTransfer == true) // Eastsun 20260522 integration: .dat is already copied directly before compression, no FTP upload needed
                    {
                        ;
                    }
                    else
                    {
                        str01 = DataPath + sULFileName + "ATC_Recipe" + ".zip";
                        str02 = FtpPath + sULFileName + "ATC_Recipe" + ".zip";
                        NMFTP2->Upload(str01, str02);
                    }
                }

                if (CUSTOMER_CODE == CC_AMD_M && iAMD_Function == 1)
                {
                    try
                    {
                        str01 = "D:\\HT9045\\Error\\English\\JAM0000.dat";
                        str02 = "/JAM0000.dat";
                        if (FileExists(str01))
                        {
                            NMFTP2->Upload(str01, str02);
                            RecordProcess("File 1 Upload done");
                        }
                        else
                        {
                            RecordProcess("File 1 Upload Fail");
                        }
                    }
                    catch (...)
                    {
                        MyDBIProcess("Exception", "TfFTPClient::UploadFileToServer2 JAM0000.dat");
                    }

                    try
                    {
                        str01 = "C:\\winnt\\system32\\tech.com";
                        str02 = "/tech.com";
                        if (FileExists(str01))
                        {
                            NMFTP2->Upload(str01, str02);
                            RecordProcess("File 2 Upload done");
                        }
                        else
                        {
                            RecordProcess("File 2 Upload Fail");
                        }
                    }
                    catch (...)
                    {
                        MyDBIProcess("Exception", "TfFTPClient::UploadFileToServer2 tech.com");
                    }

                    try
                    {
                        str01 = "d:\\HT9045\\system\\levelset.dat";
                        str02 = "/levelset.dat";
                        if (FileExists(str01))
                        {
                            NMFTP2->Upload(str01, str02);
                            RecordProcess("File 3 Upload done");
                        }
                        else
                        {
                            RecordProcess("File 3 Upload fail");
                        }
                    }
                    catch (...)
                    {
                        MyDBIProcess("Exception", "TfFTPClient::UploadFileToServer2 levelset.dat");
                    }

                    try
                    {
                        str01 = "D:\\HT9045\\system\\unknown.com";
                        str02 = "/unknown.com";
                        if (FileExists(str01))
                        {
                            NMFTP2->Upload(str01, str02);
                            RecordProcess("File 4 Upload done");
                        }
                        else
                        {
                            RecordProcess("File 4 Upload Fail");
                        }
                    }
                    catch (...)
                    {
                        MyDBIProcess("Exception", "TfFTPClient::UploadFileToServer2 unknown.com");
                    }
                }

                str.sprintf("del \"%s%s.zip\"", DataPath, sULFileName);     // finally delete the zip file.
                system(str.c_str());

                str.sprintf("del \"%s%s.Offset\"", OffsetPath, sULFileName); // finally delete the zip file.
                system(str.c_str());

                if (CosFunction.bUseFTPDownLoadATCRecipe == true &&
                    ATC_SYSTEM == eNewATCSystem &&
                    Temperature.bATCActiveCooling == true) // Ifor 20191115: add FTP DownLoad ATC Recipe
                {
                    str.sprintf("del \"%s%s.zip\"", DataPath, sULFileName + "ATC_Recipe"); // finally delete the zip file.
                    system(str.c_str());
                }
            }
        }
        else
        {
            if (CUSTOMER_CODE == CC_PTI && Source.Pos("JamAlarmLogTxt") > 0) // Sam 20170518 (wei) add path check JamAlarmLogTxt // Sam 20170502 (wei) JamAlarm Data format change // RogerYang 20170417 for PTI JamAlarm Data format
            {
                str01  = Source;                              // Sam 20170518 (wei): now a full path
                Source = Source.SubString(30, Source.Length()); // Sam 20170518 (wei): strip the leading path from the full path first "D:\\HT9045_Log\\JamAlarmLogTxt\\"
                int n = Source.Pos("\\");
                Source = Source.SubString(n + 1, Source.Length()); // remove the Lot ID path segment "//LotId"  // RogerYang 20170508 (wei) fixed n->n+1
            }
            else if (CUSTOMER_CODE == CC_JCET && Source.Pos("_FT") > 0) // RogerYang 20251210: JCET 2D FT1 white-list / FT2 comparison feature
            {
                int iPos = Source.AnsiPos("_FT");
                AnsiString sLotID = Source.SubString(1, iPos - 1);
                AnsiString str03, sCust;

                // the target path must end with "/"                    // RogerYang 20260225: revised white-list path \\customer code(first 3 chars of lot)\\lot_FTx_csv
                if (FtpPath.SubString(FtpPath.Length(), 1) != "/")
                {
                    FtpPath = FtpPath + AnsiString("/");
                }
                sCust = sLotID.SubString(0, 3);                  // take the first 3 chars of the lot ID as the customer-code folder
                str03.sprintf("%s%s/", FtpPath, sCust);

                // check the target path, layer 1: customer code (first 3 chars of lot)
                try
                {
                    NMFTP2->ChangeDir(str03);
                }
                catch (Exception& e)                              // DEAD in this translation -- see the "mkdir-then-cd-again" note below
                {
                    // create the target path
                    NMFTP2->MakeDirectory(str03);
                }
                catch (...)
                {
                    // create the target path
                    NMFTP2->MakeDirectory(str03);
                }

                str03 = str03 + sLotID + AnsiString("/");          // the final folder is the lot number
                // check the target path, layer 2: sLotID
                try
                {
                    NMFTP2->ChangeDir(str03);
                }
                catch (Exception& e)                               // DEAD in this translation -- see the "mkdir-then-cd-again" note below
                {
                    NMFTP2->MakeDirectory(str03);
                }
                catch (...)
                {
                    NMFTP2->MakeDirectory(str03);
                }
                // DISCOVERED GAP (flag for review, NOT fixed): golden's own
                // "mkdir-then-cd-again" idiom above assumes ChangeDir() THROWS
                // a VCL Exception when the remote directory does not exist yet
                // (that's the only way either `catch` clause could ever run
                // MakeDirectory). MiniFtpEngine's documented design (see the
                // file-head "Exception" note) never throws from ChangeDir --
                // failures surface only via the OnFailure event (already wired
                // to the shared NMFTP1Failure handler, which just logs +
                // sets bError; it does not call MakeDirectory). So against
                // THIS engine, a missing remote directory will silently fail
                // ChangeDir (bError=true, no exception) and this unit will
                // NOT auto-create it -- the structure is preserved 100%
                // verbatim (both catch clauses still call MakeDirectory,
                // matching golden's source text exactly), but the catch
                // clauses themselves are dead code here. A real fix would need
                // either MiniFtpEngine's ChangeDir to throw on failure (a
                // Phase-1 engine design change, out of this unit's scope) or
                // this JCET branch to check OnFailure/bError explicitly
                // instead of relying on a thrown exception.
                FtpPath = str03;
                str01.sprintf("%s%s\\%s\\", sWhite2DIDListLoc, sCust, sLotID);
                str01 += Source;
            }
            else if (bSigurdUpload_Jamcode)                        // KaiChen 20190530: Sigurd FTP Automation
            {
                str01 = "D:\\HT9045_Log\\ErrorBackup\\" + Source;
            }
            else if (bSigurdUpload_Recipe)                          // KaiChen 20190530: Sigurd FTP Automation
            {
                str01 = "D:\\HT9045_Log\\CheckingList\\" + Source;
            }
            else
            {
                str01 = "d:\\HandlerTemp\\" + sULFileName;
            }
            str02 = FtpPath + Source;
            if (FileExists(str01))                                  // ChungHung 20140704: added protection
                NMFTP2->Upload(str01, str02);
            else
                ShowMyMessage("Can not find : " + str01);            // RogerYang 20170502 (Wei) str02->str01
        }

        if (CUSTOMER_CODE == CC_PTI)                                  // Sam 20231221: PTI reported a 10035 error popping up.
        {
        }
        else
        {
            NMFTP2->Mode(MODE_ASCII);                                 // Steven 20230719: add FTP transfer mode
        }

        NMFTP2->Abort();

        NMFTP2->RequestCloseSocket();
        delete NMFTP2;
        if (bZip)                                                     // ChungHung 20140108 add FTP unload jam code
        {
            if (CUSTOMER_CODE == CC_GIGAS)                             // Isaac 20200723: GIGAS asked not to show it, a log is enough
            {
                str01 = "Upload done: " + sULFileName;
                RecordProcess(str01);                                  // log record: Upload done: JobfileName
            }
            else
            {
                ShowMyMessage("Upload done", "");
            }
        }
        else
        {
            RecordProcess("JAM Code Upload done");
            Gated_Note_ClearSendJamCodeToFTP();                        // golden: fNote->bSendJamCodeToFTP=false -- Gate #3 // Steven 20140526: bSendJamCodeToFTP now handled via Timer
        }
    }
    catch (Exception& e)                                              // Steven 20140505: try to extract the connection-anomaly message
    {
        if (bSigurdUpload_Jamcode)                                     // KaiChen 20190530: Sigurd FTP Automation
        {
            str = "FTP Server is not connected Error No : " + AnsiString(NMFTP2->LastErrorNo);
            NewRecordProcess("Message", str, e.Message);
        }
        else
        {
            ShowMyMessage(e.Message);
            if (!bZip)                                                  // ChungHung 20140108 add FTP unload jam code
            {
                str = "FTP Server is not connected Error No : " + AnsiString(NMFTP2->LastErrorNo);
                ShowMyMessage(str, "", e.Message);
            }
        }

        bError = true;
        Gated_Note_ClearSendJamCodeToFTP();                             // golden: fNote->bSendJamCodeToFTP=false -- Gate #3 // Steven 20140526: bSendJamCodeToFTP now handled via Timer
        delete NMFTP2;
    }
    catch (...)
    {
        str = "FTP Server is not connected Error No : " + AnsiString(NMFTP2->LastErrorNo);
        MyDBIProcess("Exception", str);
        bError = true;
        Gated_Note_ClearSendJamCodeToFTP();                             // golden: fNote->bSendJamCodeToFTP=false -- Gate #3 // Steven 20140526: bSendJamCodeToFTP now handled via Timer
        delete NMFTP2;
    }
}

// ===========================================================================
//  Download_2DSortingList -- golden FTPClient.cpp:4877-4953.
// ===========================================================================
void Download_2DSortingList(AnsiString FtpPath, AnsiString Source, int iTimeOut) // RogerYang 20251217 Add for timeout // Frank 20221122: 2DID sorting for ATK
{
    TNMFTP* NMFTP2;                                       // Landam: use a dynamic component, more stable
    NMFTP2 = new TNMFTP(NULL);                             // golden: new TNMFTP(this) -- no owning form exists offline
    try
    {
        // -- parameters --------------------------------------------------------
        NMFTP2->Vendor  = NMOS_AUTO;
        NMFTP2->TimeOut = iTimeOut;                        // RogerYang 20251217 Add for timeout
        NMFTP2->Passive = true;                            // Steven 20121020: experiment
        if (IniConfig.cN23FtpHost != "")
            NMFTP2->Host = IniConfig.cN23FtpHost;
        NMFTP2->UserID   = IniConfig.cN23FtpUserName; // edtUserName->Text;
        NMFTP2->Password = IniConfig.cN23FtpPassword; // edtPassword->Text;

        // -- events --------------------------------------------------------------
        FTPClientEvt_WireNMFTP1Handlers(*NMFTP2);          // golden's own 9-line NMFTP1* wiring block (:4893-4901)

        // -- connect --------------------------------------------------------------
        NMFTP2->Connect();

        for (int i = 0; i < 10; ++i)
        {
            FTPXferDelay(50);
            if (NMFTP2->Connected)
                break;
        }

        if (!NMFTP2->Connected)
        {
            ShowMyMessage("FTP Server is not connected", "");
            delete NMFTP2;
            bError = true;
            return;
        }

        AnsiString str01, str02, str03, str04;
        AnsiString sDLFileName = Source;
        AnsiString sRmsPath = "D:\\RMS\\";                  // RogerYang 20260611: use a local variable instead, don't touch the global DataPath
        str01 = sRmsPath + sDLFileName;

        if (FtpPath.SubString(FtpPath.Length(), 1) != "/")
        {
            NMFTP2->ChangeDir(FtpPath + "/");
        }
        else
        {
            NMFTP2->ChangeDir(FtpPath);
        }

        NMFTP2->Download(Source, str01);

        NMFTP2->Abort();
        FTPXferDelay(100);
        NMFTP2->RequestCloseSocket();
        FTPXferDelay(500);

        FTP_DownloadFail = false;                           // 2013.12.03, Joye, KYEC FTP  20140103 wei
        // DISCOVERED GOLDEN QUIRK (preserved verbatim, NOT fixed): golden
        // checks `if(NMFTP3!=NULL) delete NMFTP2;` here -- `NMFTP3` is an
        // ENTIRELY UNRELATED file-scope global (golden FTPClient.cpp:1214),
        // owned for real by ShowFTPModal/FormClose/N25_ReadAutoStartFileFromFTP/
        // CheckFTPConnection (all out of this unit's scope), never this
        // function's own `NMFTP2`. This looks like a copy-paste artifact from
        // one of those sibling functions. Net effect: whenever NMFTP3 happens
        // to be NULL (its typical state when no OTHER FTP dialog session is
        // mid-flight -- the common case), this function's own `NMFTP2` is
        // NEVER deleted (a real, per-call memory leak in golden itself).
        // Reproduced exactly, including the leak: this TU's own `NMFTP3`
        // (declared in the .h) is never assigned by anything in THIS unit,
        // so it stays NULL here unless some future sibling unit (translating
        // ShowFTPModal etc., linked into the same final binary) happens to
        // have it non-NULL at the moment this function runs -- exactly
        // matching golden's own cross-function coupling, not a synthesized
        // approximation of it.
        if (NMFTP3 != NULL)
            delete NMFTP2;
    }
    catch (...)
    {
        bError = true;
        if (NMFTP3 != NULL)
            delete NMFTP2;
    }
}

// ===========================================================================
//  Download_2DID_WhiteList -- golden FTPClient.cpp:5267-5387.
// ===========================================================================
void Download_2DID_WhiteList(AnsiString FtpPath) // JerryYang 20250616: 2DID white list
{
    TNMFTP* NMFTP2;                                       // Landam: use a dynamic component, more stable
    NMFTP2 = new TNMFTP(NULL);                             // golden: new TNMFTP(this) -- no owning form exists offline
    try
    {
        // -- parameters --------------------------------------------------------
        NMFTP2->Vendor  = NMOS_AUTO;
        NMFTP2->TimeOut = 20000;                           // Landam
        NMFTP2->Passive = true;                            // Steven 20121020: experiment
        NMFTP2->Host     = IniConfig.sN23_4ASE_CL_FTPHost;
        NMFTP2->UserID   = IniConfig.sN23_4ASE_CL_FTPUserName; // edtUserName->Text;
        NMFTP2->Password = IniConfig.sN23_4ASE_CL_FTPPassword; // edtPassword->Text;

        // -- events --------------------------------------------------------------
        FTPClientEvt_WireNMFTP1Handlers(*NMFTP2);          // golden's own 9-line NMFTP1* wiring block (:5283-5291)

        // -- connect --------------------------------------------------------------
        NMFTP2->Connect();

        for (int i = 0; i < 10; ++i)
        {
            FTPXferDelay(50);
            if (NMFTP2->Connected)
                break;
        }

        if (!NMFTP2->Connected)
        {
            ShowMyMessage("FTP Server is not connected", "");
            delete NMFTP2;
            bError = true;
            return;
        }

        AnsiString str01, str02, str03, str04, str1, str2;

        if (FtpPath.SubString(FtpPath.Length(), 1) != "/")
        {
            NMFTP2->ChangeDir(FtpPath + "/");
        }
        else
        {
            NMFTP2->ChangeDir(FtpPath);
        }

        str1.sprintf("%s_ASSY.xml", Gated_LotInfo_edtASECL_LotID_Text()); // golden: fLotInfo->edtASECL_LotID->Text -- Gate #6

        AnsiString sRunMode = Gated_LotInfo_cbRunModeASECL_Text();        // golden: fLotInfo->cbRunModeASECL->Text -- Gate #6
        if (sRunMode == "CORR")
        {
            str1.sprintf("CORR.xml");
            str2.sprintf("D:\\HT9045_Log\\2DBarCode\\CORR.xml");
        }
        else if (sRunMode == "FT1" || sRunMode == "RT1")
        {
            str1.sprintf("%s_ASSY.xml", Gated_LotInfo_edtASECL_LotID_Text());
            str2.sprintf("D:\\HT9045_Log\\2DBarCode\\%s_ASSY.xml", Gated_LotInfo_edtASECL_LotID_Text());
        }
        else if (sRunMode == "FT2" || sRunMode == "QT1" || sRunMode == "RT2")
        {
            str1.sprintf("%s_FT1.xml", Gated_LotInfo_edtASECL_LotID_Text());
            str2.sprintf("D:\\HT9045_Log\\2DBarCode\\%s_FT1.xml", Gated_LotInfo_edtASECL_LotID_Text());
        }
        else if (sRunMode == "FT3" || sRunMode == "QT2" || sRunMode == "RT3")
        {
            str1.sprintf("%s_FT2.xml", Gated_LotInfo_edtASECL_LotID_Text());
            str2.sprintf("D:\\HT9045_Log\\2DBarCode\\%s_FT2.xml", Gated_LotInfo_edtASECL_LotID_Text());
        }
        else if (sRunMode == "FT4" || sRunMode == "QT3" || sRunMode == "RT4")
        {
            str1.sprintf("%s_FT3.xml", Gated_LotInfo_edtASECL_LotID_Text());
            str2.sprintf("D:\\HT9045_Log\\2DBarCode\\%s_FT3.xml", Gated_LotInfo_edtASECL_LotID_Text());
        }
        else if (sRunMode == "FT5" || sRunMode == "QT4" || sRunMode == "RT5")
        {
            str1.sprintf("%s_FT4.xml", Gated_LotInfo_edtASECL_LotID_Text());
            str2.sprintf("D:\\HT9045_Log\\2DBarCode\\%s_FT4.xml", Gated_LotInfo_edtASECL_LotID_Text());
        }
        else if (sRunMode == "QT5")
        {
            str1.sprintf("%s_FT5.xml", Gated_LotInfo_edtASECL_LotID_Text());
            str2.sprintf("D:\\HT9045_Log\\2DBarCode\\%s_FT5.xml", Gated_LotInfo_edtASECL_LotID_Text());
        }

        if (FileExists(str2))
        {
            DeleteFile(str2);
        }

        NMFTP2->Download(str1, str2);

        NMFTP2->Abort();
        FTPXferDelay(100);
        NMFTP2->RequestCloseSocket();
        FTPXferDelay(500);

        FTP_DownloadFail = false;                            // 2013.12.03, Joye, KYEC FTP  20140103 wei
        // DISCOVERED GOLDEN QUIRK (preserved verbatim, NOT fixed): SAME
        // `NMFTP3` mix-up as Download_2DSortingList above -- see that
        // function's comment for the full explanation. This function's own
        // `NMFTP2` is likewise only deleted when the unrelated `NMFTP3`
        // global happens to be non-NULL.
        if (NMFTP3 != NULL)
            delete NMFTP2;
    }
    catch (...)
    {
        bError = true;
        if (NMFTP3 != NULL)
            delete NMFTP2;
    }
}
