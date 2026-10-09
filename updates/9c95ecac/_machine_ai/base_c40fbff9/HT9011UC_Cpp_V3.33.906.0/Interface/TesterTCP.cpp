//---------------------------------------------------------------------------
//  Interface/TesterTCP.cpp -- see TesterTCP.h for full scope / provenance /
//    NOT-ATTEMPTED notes.
//---------------------------------------------------------------------------
#include "Interface/TesterTCP.h"

#include "Config.h"                  // IniConfig (bN06_CopyTesterFile, asN06_TesterPath, bA57_1SaveArmSpeedByMachine)
#include "CosFunction.h"             // CosFunction (bSaveArmSpeedByMachine)
#include "common.h"                  // DataPath, sSaveByMachine, WriteIniData(AnsiString,AnsiString,AnsiString,int);
                                      // transitively (via vclcompat/vcl_compat.h) also brings in the
                                      // already-ACTIVE vclcompat SysUtils free functions this file calls
                                      // directly: FileExists / IncludeTrailingBackslash. common.h also
                                      // pulls in MachineType.h (SOFT_SIMULTE).
#include "canary_support.h"          // RecordProcess, ShowMyMessage -- hard-boundary file: READ-ONLY, not edited
                                      // (used only inside Gated_MyForceDirectories below); also LastSet
                                      // (LAST_GENERAL_SET, .iTester) for CopyOSTestResult.
#include "cprod.h"                   // TestIF_File (SYSTEM_TEST_IF, .iTestType) -- for CopyOSTestResult
                                      // (cprod.h also DECLARES CheckFileExist, but its definition is
                                      // dead-gated -- see Gated_CheckFileExist above, used instead)
#include "cmydef.h"                  // TCP_IP_MODE, ON_LINE -- for CopyOSTestResult
#include "aHotPlateSubstrate.h"      // TestSocket (TMyKitSuck, .PordRec[][] TMyProductionRecord::GetSiteNo/
                                      // GetOrderOfContact) -- for CopyOSTestResult

#include <cstdlib>                   // system()

//===========================================================================
//  Gated dependencies (TU-local, internal linkage -- cannot collide with any
//  other translation unit). Each stands in for a genuinely not-yet-available
//  golden dependency; cited at every call site below.
//===========================================================================
namespace {

// ---------------------------------------------------------------------------
// Gated_MyForceDirectories -- golden common.h:262, declared inside common.h's
// own `#if 0 // TODO(wave-file)` block (this TU cannot forward-declare the
// real name without an ODR/redeclaration hazard once common.h is ungated).
// This is a VERBATIM copy of the already-reviewed faithful re-implementation
// established by the sibling Interface/TesterTCP_Socket.cpp (same golden
// dependency, same real body read from common.cpp:1667-1717) -- reproduced
// here rather than shared via a header because each gated dependency in this
// project is intentionally TU-local (see docs/KNOWLEDGE.md); not scope creep,
// just the established per-TU gate idiom applied to the same golden function.
// AI(W906-fire-verify) 20260716: restating the sibling file's own caveat here
// too (fire-wave fidelity review noted a reader of this file in isolation
// would otherwise miss it) -- golden's `catch(Exception&e)` branch (return
// -2, e.Message) is merged into this generic `catch(...)` (return -3, fixed
// message). Harmless at both call sites in THIS file (their int return value
// is discarded either way), but worth knowing before reusing this helper
// somewhere that DOES branch on -2 vs -3.
// ---------------------------------------------------------------------------
int Gated_MyForceDirectories(AnsiString Directory, AnsiString Function = "")
{
    if (Directory == "")
    {
        RecordProcess("Directory value is NULL!", Function);
        return -1;
    }

    if (Directory[Directory.Length()] != '\\')
    {
        int attr = FileGetAttr(Directory);
        if (attr != -1)
        {
            if (!(attr & faDirectory))
                Directory = ExtractFilePath(Directory);
        }
        else
        {
            AnsiString sExt = ExtractFileExt(Directory).LowerCase();
            if (sExt == ".txt" || sExt == ".csv")
                Directory = ExtractFilePath(Directory);
        }
    }

    if (Directory == "")
    {
        RecordProcess("Directory value is NULL!", Function);
        return -1;
    }
    else
    {
        try
        {
            if (DirectoryExists(Directory) == false)
                ForceDirectories(Directory);
        }
        catch (...)
        {
            AnsiString Str;
            Str.sprintf("%s -- %s", Directory, Function);
            ShowMyMessage("Create directory fail!", Str, Str);
            return -3;
        }
    }

    return 1;
}

// ---------------------------------------------------------------------------
// IncludeTrailingPathDelimiter -- BCB6 synonym for IncludeTrailingBackslash
// (same TU-local synonym idiom already established by cpublic.cpp:135-138,
// which carries the identical "AI(...) -- BCB6 synonym" note). Both golden
// CopyRecipeToTester (:1065) and CopyRecipeFromTester (:1085) call it under
// this exact name.
// ---------------------------------------------------------------------------
inline AnsiString IncludeTrailingPathDelimiter(const AnsiString& p)
{
    return IncludeTrailingBackslash(p);
}

// ---------------------------------------------------------------------------
// Gated_CheckFileExist -- golden cprod.cpp:1388-1398 (Ken 20210702
// "AddPadInterface"). Declared (real, un-gated) in cprod.h:3245, but its
// DEFINITION sits inside cprod.cpp's own `#if 0 // TODO(W6)` dead block
// (verified this wave: that block spans cprod.cpp lines 184-4036, swallowing
// this function along with hundreds of others -- nothing in this ported tree
// called CheckFileExist before this wave, which is why the gap surfaced only
// now, at link time). golden's own body calls Borland-RTL-only low-level
// handle functions `_rtl_open`/`_rtl_close` (BCB6 <io.h>), which do not exist
// under MinGW. Reimplemented here with the portable `fopen(...,"rb")`/
// `fclose` pair, which preserves golden's actual OBSERVABLE behavior: golden
// itself is NOT a plain existence check (that's what vclcompat FileExists()
// already gives you, used elsewhere in this very file) -- its access()-then-
// open two-step returns true only if the path both exists AND can right now
// be opened for read, false otherwise (including a real file access() finds
// but the open step can't open). fopen(...,"rb") collapses to exactly the
// same two true/false outcomes in one portable call. Same TU-local gate
// idiom as Gated_MyForceDirectories above (this project's established
// pattern: a stand-in confined to internal linkage in the ONE TU that needs
// it, not a new shared header).
// ---------------------------------------------------------------------------
bool Gated_CheckFileExist(AnsiString cFName)
{
    FILE *fp = fopen(cFName.c_str(), "rb");
    if (fp == NULL)
        return false;
    fclose(fp);
    return true;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// MySleep(DWORD) -- golden common.h:261. common.h's own declaration is inside
// a `#if 0 // TODO(wave-timing)` block, so `#include "common.h"` alone does
// not expose it. A REAL, ACTIVE offline no-op definition already exists in
// acarry_shims.cpp (confirmed reused the same way by Automation/auto9045.cpp,
// CCLink/MyCCLink.cpp and KYECFTP/FTPClient_Transfer.cpp's own identical
// `extern void MySleep(DWORD dwMilliseconds);` redeclaration + "REAL
// definition: acarry_shims.cpp" comment) -- reused here via the same local
// extern redeclaration, not a new/fourth duplicate definition. NOTE: this
// wave's CMakeLists.txt is not yet wired for this new file (integrate stage
// owns that), so this extern is unresolved until this TU is linked alongside
// acarry_shims.cpp; test_TesterTCP.cpp (this wave's own test) supplies its
// own local no-op definition instead, matching the tests/test_FTPClient_
// Transfer.cpp / tests/test_MyCCLink.cpp precedent for a standalone test binary.
// ---------------------------------------------------------------------------
extern void MySleep(DWORD dwMilliseconds);

//===========================================================================
//  golden TfTesterTCP::CopyRecipeToTester(AnsiString FileName), TesterTCP.cpp:1058-1077.
//===========================================================================
void TesterTCP_CopyRecipeToTester(AnsiString FileName)
{
    AnsiString str1, str2, str3;
    // Steven 20250327/20230710: OS測試機的工作檔也要上傳
    // ("the OS Tester's own work-file also needs to be uploaded")
    if (IniConfig.bN06_CopyTesterFile == true)
    {
        str1.sprintf("%s%s\\OS_Setting.zip", DataPath, FileName);
        str2.sprintf("%s", IncludeTrailingPathDelimiter(IniConfig.asN06_TesterPath));
        if (FileExists(str1))
        {
            str3.sprintf("e \"%s\" -o\"%s\" -y", str1, str2);

            // AI(W906-D-Interface-TesterTCP) 20260716: golden calls
            //   hInstance = ShellExecute(this,"open","D:\\HT9045\\7z.exe",
            //                            str3.c_str(),NULL,SW_HIDE);
            //   if(int(hInstance)<=32) { ; }   // golden's own empty-branch idiom, TesterTCP.cpp:1070-1073
            // (async fire-and-forget launch; golden's follow-up MySleep(500)
            // is its own throttle to give the just-launched external process a
            // head start before returning). VCL ShellExecute has no
            // vclcompat/portable-C++ equivalent anywhere in this tree yet
            // (would pull in a new <shellapi.h>+shell32 link dependency not
            // shared by anything else translated so far). Translated instead
            // to this project's OWN established idiom for invoking this exact
            // 7z.exe, already used by Automation/auto9045.cpp's
            // DoULRequest/DoDLRequest and KYECFTP/FTPClient_Transfer.cpp: a
            // system() call on a "D:\\HT9045\\7z.exe ..." command line.
            // AI(W906-fire-verify) 20260716: fire-wave fidelity review flagged
            // (medium) that a bare system() call BLOCKS the caller until 7z
            // actually finishes, unlike golden's async ShellExecute+MySleep(500)
            // best-effort throttle -- a real behavior divergence, dormant only
            // because nothing wires this function to a live call site yet.
            // Fixed here by routing through the cmd.exe `start /B` launcher
            // (still system(), still zero new headers/deps, same idiom as the
            // rest of this codebase's 7z.exe seams) so the external process is
            // detached/backgrounded instead of blocking -- this also restores
            // the ORIGINAL purpose of the golden-faithful MySleep(500) call
            // right below (a head-start throttle for a still-running external
            // process), which a blocking system() had made a redundant no-op.
            // `start` reads its first quoted token as a window title (which
            // would otherwise misparse if `cmd` itself started with a quoted
            // path) -- the explicit `""` empty title below sidesteps that
            // ambiguity unconditionally, regardless of `cmd`'s own contents.
            AnsiString cmd;
            cmd.sprintf("D:\\HT9045\\7z.exe %s", str3);   // capital "D:" preserved verbatim from golden :1069
            AnsiString shellCmd;
            shellCmd.sprintf("start \"\" /B %s", cmd);
            system(shellCmd.c_str());

            MySleep(500);
        }
    }
}

//===========================================================================
//  golden TfTesterTCP::CopyRecipeFromTester(AnsiString FileName), TesterTCP.cpp:1079-1097.
//===========================================================================
void TesterTCP_CopyRecipeFromTester(AnsiString FileName)
{
    AnsiString str1, str2, str3;
    // Steven 20250327/20230710: OS測試機的工作檔也要上傳
    // ("the OS Tester's own work-file also needs to be uploaded")
    if (IniConfig.bN06_CopyTesterFile == true)
    {
        str1.sprintf("%s%s\\OS_Setting.zip", DataPath, FileName);
        str2.sprintf("%s%s.ini", IncludeTrailingPathDelimiter(IniConfig.asN06_TesterPath), FileName);
        if (FileExists(str2))
        {
            str3.sprintf("a -tzip \"%s\" \"%s\"", str1, str2);

            // AI(W906-D-Interface-TesterTCP) 20260716: same ShellExecute ->
            // system() translation rationale as CopyRecipeToTester above
            // (golden TesterTCP.cpp:1089 `ShellExecute(this,"open",
            // "D:\\HT9045\\7z.exe",str3.c_str(),NULL,SW_HIDE);`, return value
            // discarded by golden too -- no `hInstance` even captured here).
            // AI(W906-fire-verify) 20260716: same `start /B` non-blocking fix
            // as CopyRecipeToTester above -- see that function's comment for
            // the full rationale (fire-wave fidelity review, medium severity).
            AnsiString cmd;
            cmd.sprintf("D:\\HT9045\\7z.exe %s", str3);   // capital "D:" preserved verbatim from golden :1089
            AnsiString shellCmd;
            shellCmd.sprintf("start \"\" /B %s", cmd);
            system(shellCmd.c_str());

            MySleep(2000);   // Steven 20240105: add delay (golden TesterTCP.cpp:1090)
        }
        else
        {
            // golden's own dead branch: a commented-out
            // `str.sprintf("N06, step 1 : File %s does not exists!", str2);`
            // with no live statement -- empty-branch idiom, nothing to
            // translate (golden TesterTCP.cpp:1093-1095).
        }
    }
}

//===========================================================================
//  golden TfTesterTCP::btnSaveClick(TObject *Sender), TesterTCP.cpp:1099-1115.
//===========================================================================
void TesterTCP_btnSaveClick(bool bChkAuto1_OSChecked,
                             bool bChkAuto2_OSChecked,
                             bool bChkAuto3_OSChecked,
                             AnsiString LastFileName)
{
    AnsiString szDir = "";
    if (CosFunction.bSaveArmSpeedByMachine && IniConfig.bA57_1SaveArmSpeedByMachine)
    {
        szDir = sSaveByMachine;
    }
    else
    {
        szDir.sprintf("%s%s", DataPath, LastFileName);
    }
    Gated_MyForceDirectories(szDir);
    szDir += "\\ArmCondition.Data";
    WriteIniData(szDir, "Output Arm", "JSCC_OS_Use Auto1", bChkAuto1_OSChecked ? 1 : 0);
    WriteIniData(szDir, "Output Arm", "JSCC_OS_Use Auto2", bChkAuto2_OSChecked ? 1 : 0);
    WriteIniData(szDir, "Output Arm", "JSCC_OS_Use Auto3", bChkAuto3_OSChecked ? 1 : 0);
}

//===========================================================================
//  AI(W906-W7-L4-fix) 20260802: TEST-OBSERVABLE SEAMS for
//  TesterTCP_CopyOSTestResult's three environment-touching calls. Full
//  rationale (why they exist, which two mutations survived without them, and
//  the exact production-behaviour guarantee) lives in the matching banner in
//  Interface/TesterTCP.h -- not repeated here.
//
//  Invariant enforced by every seam below, in this order:
//    1. record the arguments unconditionally (this is what makes the target
//       FILENAME -- the only place iContactIndex/iTesterCh become visible --
//       observable to a test);
//    2. if a hook is installed, call the hook INSTEAD of the real function;
//    3. otherwise call the real function with the arguments unchanged and
//       return its result unchanged.
//  Hooks are NULL at static-init and are only ever written by tests, so step 3
//  is the only step production takes.
//===========================================================================
W7L4TesterTCP_ForceDirectoriesHookFn W7L4TesterTCP_ForceDirectoriesHook = NULL;
W7L4TesterTCP_CheckFileExistHookFn   W7L4TesterTCP_CheckFileExistHook   = NULL;
W7L4TesterTCP_CopyFileHookFn         W7L4TesterTCP_CopyFileHook         = NULL;

AnsiString W7L4TesterTCP_LastForceDirectoriesPath     = "<unset>";
AnsiString W7L4TesterTCP_LastForceDirectoriesFunction = "<unset>";
int        W7L4TesterTCP_ForceDirectoriesCount        = 0;

AnsiString W7L4TesterTCP_CheckFileExistPath[W7L4TESTERTCP_MAX_RECORDED];
int        W7L4TesterTCP_CheckFileExistCount          = 0;

AnsiString W7L4TesterTCP_CopyFileSource[W7L4TESTERTCP_MAX_RECORDED];
AnsiString W7L4TesterTCP_CopyFileTarget[W7L4TESTERTCP_MAX_RECORDED];
bool       W7L4TesterTCP_CopyFileFailIfExists[W7L4TESTERTCP_MAX_RECORDED];
int        W7L4TesterTCP_CopyFileCount                = 0;

void W7L4TesterTCP_ResetSeamRecording()
{
    W7L4TesterTCP_LastForceDirectoriesPath     = "<unset>";
    W7L4TesterTCP_LastForceDirectoriesFunction = "<unset>";
    W7L4TesterTCP_ForceDirectoriesCount        = 0;
    W7L4TesterTCP_CheckFileExistCount          = 0;
    W7L4TesterTCP_CopyFileCount                = 0;
    for (int i = 0; i < W7L4TESTERTCP_MAX_RECORDED; i++)
    {
        W7L4TesterTCP_CheckFileExistPath[i]   = "<unset>";
        W7L4TesterTCP_CopyFileSource[i]       = "<unset>";
        W7L4TesterTCP_CopyFileTarget[i]       = "<unset>";
        W7L4TesterTCP_CopyFileFailIfExists[i] = false;
    }
}

namespace {

// AI(W906-W7-L4-fix) 20260802: seam #1 -- stands in front of golden's
// MyForceDirectories("D:\\HT9045_Log\\OSTestResult","CopyOSTestResult")
// (golden TesterTCP.cpp:674) so a test can observe WHICH directory was
// force-created without a real production directory ever appearing on disk.
int Seam_MyForceDirectories(AnsiString Directory, AnsiString Function)
{
    W7L4TesterTCP_LastForceDirectoriesPath     = Directory;
    W7L4TesterTCP_LastForceDirectoriesFunction = Function;
    W7L4TesterTCP_ForceDirectoriesCount++;

    if (W7L4TesterTCP_ForceDirectoriesHook != NULL)
        return W7L4TesterTCP_ForceDirectoriesHook(Directory, Function);

    return Gated_MyForceDirectories(Directory, Function);
}

// AI(W906-W7-L4-fix) 20260802: seam #2 -- stands in front of golden's
// CheckFileExist(sSourceFileName) (golden TesterTCP.cpp:681 and :692). Records
// every probe in call order, and lets a test DECIDE the answer, which is the
// only way to reach golden's copy branch on a machine with no Z: drive.
bool Seam_CheckFileExist(AnsiString cFName)
{
    if (W7L4TesterTCP_CheckFileExistCount < W7L4TESTERTCP_MAX_RECORDED)
        W7L4TesterTCP_CheckFileExistPath[W7L4TesterTCP_CheckFileExistCount] = cFName;
    W7L4TesterTCP_CheckFileExistCount++;

    if (W7L4TesterTCP_CheckFileExistHook != NULL)
        return W7L4TesterTCP_CheckFileExistHook(cFName);

    return Gated_CheckFileExist(cFName);
}

// AI(W906-W7-L4-fix) 20260802: seam #3 -- stands in front of golden's
// CopyFile(source,target,false) (golden TesterTCP.cpp:684 and :695). Records
// BOTH arguments of EVERY call: the target string is the whole point, it is
// the only place golden's iContactIndex (%06d) and iTesterCh (%02d, and with
// it the golden `-1` at :671) are externally visible. Takes const char* rather
// than AnsiString purely so the two call sites keep golden's own
// `...c_str(), ...c_str(), false` shape verbatim.
bool Seam_CopyFile(const char *sSource, const char *sTarget, bool bFailIfExists)
{
    if (W7L4TesterTCP_CopyFileCount < W7L4TESTERTCP_MAX_RECORDED)
    {
        W7L4TesterTCP_CopyFileSource[W7L4TesterTCP_CopyFileCount]       = AnsiString(sSource);
        W7L4TesterTCP_CopyFileTarget[W7L4TesterTCP_CopyFileCount]       = AnsiString(sTarget);
        W7L4TesterTCP_CopyFileFailIfExists[W7L4TesterTCP_CopyFileCount] = bFailIfExists;
    }
    W7L4TesterTCP_CopyFileCount++;

    if (W7L4TesterTCP_CopyFileHook != NULL)
        return W7L4TesterTCP_CopyFileHook(AnsiString(sSource), AnsiString(sTarget), bFailIfExists);

    return CopyFile(sSource, sTarget, bFailIfExists);
}

} // anonymous namespace

// AI(W906-W7-L4-fix) 20260802: pass-through provers -- see TesterTCP.h. These
// call the SAME three seams above that the production body calls, so a test
// can demonstrate on scratch paths that a NULL hook really does reach the real
// MyForceDirectories / CheckFileExist / CopyFile. No production caller.
int W7L4TesterTCP_ProveForceDirectoriesSeam(AnsiString Directory, AnsiString Function)
{
    return Seam_MyForceDirectories(Directory, Function);
}

bool W7L4TesterTCP_ProveCheckFileExistSeam(AnsiString cFName)
{
    return Seam_CheckFileExist(cFName);
}

bool W7L4TesterTCP_ProveCopyFileSeam(AnsiString sSource, AnsiString sTarget, bool bFailIfExists)
{
    return Seam_CopyFile(sSource.c_str(), sTarget.c_str(), bFailIfExists);
}

//===========================================================================
//  golden TfTesterTCP::CopyOSTestResult(int iSuckRow, int iSuckCol), golden
//  TesterTCP.cpp:660-697 (38 lines).
//
//  AI(W906-W7-L4) 20260802: new function this wave. NO VCL widget reads at
//  all (unlike the sibling PlaceOSTestResultToTray, out of scope -- see
//  TesterTCP.h's SCOPE banner) so no extract-calc-core parameter promotion
//  was needed; the golden member-function signature carries over unchanged
//  as a free function.
//
//  *** REACHABILITY BANNER *** -- this function has NO production caller
//  anywhere in this ported tree. golden's own callers of CopyOSTestResult
//  (searched across the full golden tree) are themselves not yet translated
//  in this codebase -- this is expected forward progress, not an oversight,
//  and is disclosed here per this project's standing rule (a prior wave
//  shipped an unreachable state machine with no such note and it had to be
//  found later with `nm`). It becomes reachable once whichever golden call
//  site invokes it (an OS-Tester TCP result-processing path, sibling to the
//  already-translated TesterTCP_Socket.cpp TimerProcessTCPDataTimer/
//  SimulateBin machinery) is itself translated and wired to call
//  TesterTCP_CopyOSTestResult.
//
//  Silent-trap checklist, verified against golden line-by-line (not assumed):
//    - 1-based AnsiString indexing: N/A -- this function does no
//      SubString/Pos on 1-based indices; the only "-1" is the int arithmetic
//      below, not string indexing.
//    - integer division: NONE anywhere in this function (no `/` operator at
//      all) -- nothing to keep int/int here.
//    - iTesterCh = GetSiteNo()-1 (golden :671): GetSiteNo() returns an int
//      (atoi() over the stored site-number string, Public/MyProductionRecord
//      .cpp) so this is plain int subtraction, matching golden exactly --
//      no float promotion risk (no operands here are floating point).
//    - error code sign/width: N/A -- CopyOSTestResult has no return value at
//      all (golden `void __fastcall`) and does not consult CopyFile's bool
//      result (golden discards it too, :684/:695 -- preserved verbatim,
//      not a translation gap).
//    - byte order: N/A -- no binary/wire data, only AnsiString text paths and
//      a whole-file CopyFile passthrough.
//    - fixed-width buffers: N/A -- every string here is AnsiString (dynamic),
//      matching golden; no fixed char[] buffers anywhere in this function.
//    - sprintf format widths/types: `%06d`/`%02d`/`%d` all consume `int`
//      arguments (iContactIndex, iTesterCh) -- matches golden's own
//      Device%06d_%02d.TXT / Device%d.TXT format strings verbatim (golden
//      :677/:679/:683/:688/:690/:694).
//===========================================================================
void TesterTCP_CopyOSTestResult(int iSuckRow, int iSuckCol)
{
    if (TestIF_File.iTestType != TCP_IP_MODE)   // Steven 20230116 : OS測試機準備送出工作檔名 (golden :662)
        return;

#ifndef SOFT_SIMULTE
    if (LastSet.iTester != ON_LINE)             // Steven 20231113 : 修正TCP/IP Test為Off Line (golden :666)
        return;
#endif

    AnsiString sSourceFileName, sTargetFileName, cString;   // cString: golden's own declared-but-unused
                                                              // local (golden :670) -- preserved verbatim,
                                                              // not a translation omission.
    int iTesterCh     = TestSocket.PordRec[iSuckRow][iSuckCol].GetSiteNo() - 1;
    int iContactIndex = TestSocket.PordRec[iSuckRow][iSuckCol].GetOrderOfContact();

    // AI(W906-W7-L4-fix) 20260802: the three environment-touching calls in the
    // rest of this body now go through Seam_MyForceDirectories /
    // Seam_CheckFileExist / Seam_CopyFile instead of naming
    // Gated_MyForceDirectories / Gated_CheckFileExist / CopyFile directly.
    // With no test hook installed each seam calls precisely those same three
    // functions with the same arguments and returns the same result, so this
    // is a pure observability change, not a behaviour change (see the seam
    // banner above and TesterTCP.h). Argument lists and control flow below are
    // otherwise untouched and still line-for-line golden.
    Seam_MyForceDirectories("D:\\HT9045_Log\\OSTestResult", "CopyOSTestResult");

#ifdef SOFT_SIMULTE
    sSourceFileName.sprintf("D:\\RMS\\Device%d.TXT", iTesterCh);
#else
    sSourceFileName.sprintf("Z:\\Device%d.TXT", iTesterCh);
#endif
    if (Seam_CheckFileExist(sSourceFileName) == true)
    {
        sTargetFileName.sprintf("D:\\HT9045_Log\\OSTestResult\\Device%06d_%02d.TXT", iContactIndex, iTesterCh);
        Seam_CopyFile(sSourceFileName.c_str(), sTargetFileName.c_str(), false);
    }

#ifdef SOFT_SIMULTE
    sSourceFileName.sprintf("D:\\RMS\\Device%d.TXT", iTesterCh);
#else
    sSourceFileName.sprintf("Z:\\FailPin_Device%d.TXT", iTesterCh);
#endif
    if (Seam_CheckFileExist(sSourceFileName) == true)
    {
        // golden quirk, preserved verbatim (golden :694 vs :683): this SECOND
        // copy builds the SAME target filename as the first one, so when both
        // sources exist the FailPin file silently overwrites the Device file.
        sTargetFileName.sprintf("D:\\HT9045_Log\\OSTestResult\\Device%06d_%02d.TXT", iContactIndex, iTesterCh);
        Seam_CopyFile(sSourceFileName.c_str(), sTargetFileName.c_str(), false);
    }
}
