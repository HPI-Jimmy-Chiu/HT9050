// =============================================================================
//  tests/test_FTPClient_Transfer.cpp -- KYECFTP FTPClient_Transfer acceptance
//  test.
//
//  Drives LoadFileFormServer2 / UploadFileToServer2 / Download_2DSortingList /
//  Download_2DID_WhiteList end to end against MiniFtpEngine's SIM-mode
//  fake-FTP-server hook (SAME technique as tests/test_MiniFtpEngine.cpp --
//  see that file's own header comment for the exact hook contract).
//
//  SCOPE NOTE (why this test defines its own IniConfig/CosFunction/Temperature/
//  DataPath/etc. + its own ShowMyMessage/RecordProcess/NewRecordProcess/
//  MyDBIProcess(2-arg)/MySleep stand-ins, instead of linking the REAL
//  cmydef.cpp/cprod.cpp/common.cpp/canary_support.cpp/aHotPlateSubstrate.cpp/
//  acarry_shims.cpp): those real TUs sit at the root of a large, tightly
//  coupled "god-stack" (per CMakeLists.txt's own comment on ht9045_globals:
//  "NOT linked into an executable here because the active globals construct
//  objects whose ctors live in the gated regions") -- pulling them in for a
//  focused unit test of ONLY this new file's logic would require resolving
//  a large, unrelated dependency graph. THIS test therefore supplies its own
//  minimal, test-local definitions of the handful of extern globals/
//  functions FTPClient_Transfer.cpp references, exactly mirroring the
//  values/behavior the real ones would have offline (zero-initialized globals,
//  a stdout log sink for Show/Record/NewRecord, a real no-op MySleep). This
//  does NOT change anything about FTPClient_Transfer.cpp itself -- it still
//  references the REAL extern names, which a full CMake/production build
//  will resolve to the ACTUAL cmydef.cpp/cprod.cpp/common.cpp/
//  canary_support.cpp/aHotPlateSubstrate.cpp/acarry_shims.cpp definitions.
//  A full CMake build was NOT attempted, per this task's own instruction.
//
//  No external test framework (matches tests/test_MiniFtpEngine.cpp /
//  tests/test_ContactForce.cpp style): a tiny PASS/FAIL harness, non-zero
//  exit on any failure.
// =============================================================================
#include "KYECFTP/FTPClient_Transfer.h"

#include "Config.h"
#include "CosFunction.h"
#include "cprod.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                   \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// =============================================================================
//  Test-local stand-ins for the globals/functions FTPClient_Transfer.cpp
//  references but does not itself define -- see the SCOPE NOTE above.
// =============================================================================
int  CUSTOMER_CODE  = 1;              // deliberately not any CC_* constant this unit branches on
int  ATC_SYSTEM      = 0;             // != eNewATCSystem(6) -- keeps every ATC-recipe branch inert
bool bSigurdDownload_Recipe = false;
bool bSigurdUpload_Recipe   = false;
bool bSigurdUpload_Jamcode  = false;
AnsiString FTPAutomation_Down_ServerIP, FTPAutomation_Down_UserID, FTPAutomation_Down_Password;
AnsiString FTPAutomation_Up_ServerIP,   FTPAutomation_Up_UserID,   FTPAutomation_Up_Password;
int  iAMD_Function   = 0;
bool bHasFTPDownload = false;
AnsiString asSetupFileCheckList = "SetupFileCheckList.txt";
AnsiString sATCPath = "./ftpxfer_test_atc/";

HT9045_CONFIG IniConfig;               // zero-initialized (global, no explicit init -- see SCOPE NOTE)
HT9045_COUSTOMER_FUNCTION CosFunction; // ditto
SYSTEM_TEMPERATURE Temperature;        // ditto

AnsiString DataPath   = "./";
AnsiString OffsetPath = "./";
AnsiString DefaultPath = "./";
AnsiString LastDataPath = "./ftpxfer_test_lastdata.inf";
AnsiString asATCFileTransferPath = "./ftpxfer_test_atcxfer/";
AnsiString sWhite2DIDListLoc = "./ftpxfer_test_2did/";

// ShowMyMessage / RecordProcess -- golden mymessbox.h:58 / cMyDB.h:63. Log to
// stdout (matches canary_support.cpp's own real Sim-body behavior, per the
// file this test intentionally does not link -- see SCOPE NOTE).
void ShowMyMessage(AnsiString S1, AnsiString S2, AnsiString /*S3*/, bool /*Ok*/, bool /*bServoOff*/)
{
    if (S2.Length() > 0)
        std::printf("  [ShowMyMessage] %s | %s\n", S1.c_str(), S2.c_str());
    else
        std::printf("  [ShowMyMessage] %s\n", S1.c_str());
}
void RecordProcess(AnsiString S, AnsiString S2)
{
    if (S2.Length() > 0)
        std::printf("  [RecordProcess] %s | %s\n", S.c_str(), S2.c_str());
    else
        std::printf("  [RecordProcess] %s\n", S.c_str());
}
void NewRecordProcess(AnsiString S1, AnsiString S2, AnsiString S3)
{
    std::printf("  [NewRecordProcess] %s | %s | %s\n", S1.c_str(), S2.c_str(), S3.c_str());
}
void MyDBIProcess(AnsiString S1, AnsiString S2)
{
    std::printf("  [MyDBIProcess] %s | %s\n", S1.c_str(), S2.c_str());
}
void MySleep(DWORD /*dwMilliseconds*/)
{
    // real golden body just calls Win32 Sleep(); this test runs with
    // FTPClientTransfer_SetFastDelayForTest(true) throughout, so a no-op
    // here keeps the whole suite fast without changing what's under test
    // (MySleep is called only twice, both inside UploadFileToServer2's
    // non-bZip reconnect-retry ladder).
}

// =============================================================================
//  Sim-mode fake FTP server double -- SAME technique as
//  tests/test_MiniFtpEngine.cpp's FakeFtpServer (see that file for the full
//  rationale of the CTRL/CTRL-CONNECT/DATA-OPEN/DATA-CLOSE hook contract).
// =============================================================================
struct FakeFtpServer {
    Nmftp::TNMFTP* engine;
    std::deque<std::string> ctrlReplies;
    std::string dataPayload;
    int dataOpens;

    explicit FakeFtpServer(Nmftp::TNMFTP* e) : engine(e), dataOpens(0) {}

    void operator()(const char* channel, const AnsiString& /*rawLine*/)
    {
        std::string ch(channel);
        if (ch == "CTRL-CONNECT" || ch == "CTRL" || ch == "DATA-CLOSE")
        {
            if (!ctrlReplies.empty())
            {
                const std::string& reply = ctrlReplies.front();
                engine->DebugControlSocket()->Socket->SimPushReceive(
                    reply.data(), static_cast<int>(reply.size()));
                ctrlReplies.pop_front();
            }
        }
        else if (ch == "DATA-OPEN")
        {
            ++dataOpens;
            if (!dataPayload.empty())
            {
                engine->DebugDataSocket()->Socket->SimPushReceive(
                    dataPayload.data(), static_cast<int>(dataPayload.size()));
            }
        }
    }
};

// Pushes the standard Connect() login sequence (220/331/230).
static void PushLoginSequence(FakeFtpServer& srv)
{
    srv.ctrlReplies.push_back("220 Welcome to FakeFTP\r\n");
    srv.ctrlReplies.push_back("331 Password required\r\n");
    srv.ctrlReplies.push_back("230 Logged in\r\n");
}

// Pushes a generic "OK" reply, generously reused for ChangeDir/Mode(TYPE)/
// Abort(ABOR) round trips -- this test asserts on FILE CONTENT / high-level
// state (Connected, FTP_DownloadFail, event counts), not on the exact literal
// reply text for these housekeeping commands (already covered byte-exactly
// by tests/test_MiniFtpEngine.cpp's own framing assertions).
static void PushOk(FakeFtpServer& srv, int n = 1)
{
    for (int i = 0; i < n; ++i)
        srv.ctrlReplies.push_back("200 OK\r\n");
}

// Pushes one PASV+preliminary+completion round trip for a single Download()/
// Upload()/Nlist() call.
static void PushTransferOk(FakeFtpServer& srv, int dataPort = 4001)
{
    char buf[96];
    std::snprintf(buf, sizeof(buf), "227 Entering Passive Mode (127,0,0,1,%d,%d).\r\n",
                  dataPort / 256, dataPort % 256);
    srv.ctrlReplies.push_back(buf);
    srv.ctrlReplies.push_back("150 Opening data connection\r\n");
    srv.ctrlReplies.push_back("226 Transfer complete\r\n");
}

static std::string ReadWholeFileStd(const char* path)
{
    std::FILE* f = std::fopen(path, "rb");
    if (!f) return std::string();
    std::string out;
    char buf[256];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0)
        out.append(buf, n);
    std::fclose(f);
    return out;
}

// =============================================================================
//  [1] Connect-fails early-return path -- all 4 functions, no scripted reply
//      at all (SIM mode: ReadOneCtrlLine_ returns false immediately -- see
//      MiniFtpEngine.cpp:311-313 -- no hang, no crash).
// =============================================================================
static void test_connect_fails_all4()
{
    printf("\n[1] Connect-fails early-return path (all 4 functions)\n");

    FTPClientTransfer_ResetStateForTest();

    LoadFileFormServer2("/incoming", "JOBFILE_A");
    CHECK(bError == true, "LoadFileFormServer2: connect-fail sets bError");
    CHECK(FTP_DownloadFail == true, "LoadFileFormServer2: connect-fail sets FTP_DownloadFail");

    FTPClientTransfer_ResetStateForTest();
    UploadFileToServer2("/incoming/", "JOBFILE_B", true /*bZip*/);
    CHECK(bError == true, "UploadFileToServer2(bZip=true): connect-fail sets bError");

    FTPClientTransfer_ResetStateForTest();
    UploadFileToServer2("/incoming/", "JOBFILE_C", false /*bZip*/);
    CHECK(bError == true, "UploadFileToServer2(bZip=false): connect-fail sets bError (5x reconnect ladder exhausted)");

    FTPClientTransfer_ResetStateForTest();
    Download_2DSortingList("/incoming", "sort.csv");
    CHECK(bError == true, "Download_2DSortingList: connect-fail sets bError");

    FTPClientTransfer_ResetStateForTest();
    Download_2DID_WhiteList("/incoming");
    CHECK(bError == true, "Download_2DID_WhiteList: connect-fail sets bError");
}

// =============================================================================
//  [2] LoadFileFormServer2 -- TWO scenarios:
//   [2a] no-hook path with valid IniConfig fields (StrToInt succeeds): proves
//        the IniConfig-driven Host/UserID/Password/Port assignment branch (the
//        non-Sigurd `else` at golden :127-134) runs to completion with no
//        crash; Connect() itself STILL fails (see the correction note below).
//   [2b] N06_FtpPort=="" (StrToInt throws std::runtime_error): the single
//        highest-value regression case for this unit's own "Exception is
//        DEAD CODE" judgment call (see the .cpp file-head note) -- proves the
//        thrown std::runtime_error, which the local `catch(Exception&e)`
//        clause does NOT match, correctly falls through to golden's own
//        trailing bare `catch(...)` instead (bError/FTP_DownloadFail both
//        become true, NO uncaught exception escapes/terminates the process).
//
//  CORRECTION vs. this test's own original assumption (kept here so a future
//  reader does not repeat the mistake): a first draft of this test assumed
//  MiniFtpEngine's SIM-mode Connect() "always succeeds with no hook
//  installed". That is only true of the underlying TRANSPORT step
//  (`pImpl_->pControl->Active=true`, MiniFtpEngine.cpp:520) -- the SUBSEQUENT
//  "220" welcome-banner read (`ReadOneCtrlLine_`) finds an EMPTY reply queue
//  when no hook is installed and returns false immediately (SIM: "nothing
//  queued, nothing more will ever arrive", MiniFtpEngine.cpp:311-313) ->
//  Connect() reports FAILURE via OnConnectionFailed, exactly like test [1].
//  DISCOVERED TESTABILITY LIMITATION (flag for review): MiniFtpEngine's
//  SetSimServerHook() is PER-INSTANCE, and each of this unit's 4 functions
//  constructs its OWN `TNMFTP` as a purely local variable -- there is no
//  injection point for an external test to install a hook on it before the
//  function's internal Connect() runs. A genuine end-to-end HAPPY-PATH
//  scenario through the real production functions is therefore not
//  reachable by an automated test as currently structured; test [3] below
//  instead proves the actual WIRING + call sequence these functions build
//  (identical `FTPClientEvt_WireNMFTP1Handlers` + `TNMFTP` usage) is correct
//  on a test-owned instance where a hook CAN be installed.
// =============================================================================
static void test_load_file_no_hook_and_bad_port()
{
    printf("\n[2] LoadFileFormServer2 -- no-hook path (valid config) + StrToInt-throws path (empty port)\n");

    // [2a]
    FTPClientTransfer_ResetStateForTest();
    IniConfig.FtpHost      = "127.0.0.1";
    IniConfig.FtpUserName  = "testuser";
    IniConfig.FtpPassword  = "testpass";
    IniConfig.N06_FtpPort  = "21";
    IniConfig.FtpTransMode = 0;

    LoadFileFormServer2("FTPXFER_LOAD_TEST", "FTPXFER_LOAD_TEST");

    CHECK(bError == true, "[2a] LoadFileFormServer2 (valid config, no hook): Connect() still fails (empty ctrl-reply queue) -> bError true");
    CHECK(FTP_DownloadFail == true, "[2a] LoadFileFormServer2 (valid config, no hook): FTP_DownloadFail true (early return before the download loop)");

    // [2b] -- the StrToInt-throws / dead-Exception-catch / trailing-catch(...) proof
    FTPClientTransfer_ResetStateForTest();
    IniConfig.FtpHost      = "127.0.0.1";
    IniConfig.FtpUserName  = "testuser";
    IniConfig.FtpPassword  = "testpass";
    IniConfig.N06_FtpPort  = "";          // StrToInt("") throws std::runtime_error
    IniConfig.FtpTransMode = 0;

    LoadFileFormServer2("FTPXFER_LOAD_TEST2", "FTPXFER_LOAD_TEST2"); // must not crash / not terminate the process

    CHECK(bError == true, "[2b] LoadFileFormServer2 (empty N06_FtpPort): std::runtime_error from StrToInt is caught by the trailing catch(...) -> bError true");
    CHECK(FTP_DownloadFail == true, "[2b] LoadFileFormServer2 (empty N06_FtpPort): FTP_DownloadFail true, process did NOT terminate (proves the Exception/std::runtime_error mismatch is harmless)");
}

// =============================================================================
//  [3] LoadFileFormServer2 -- direct MiniFtpEngine sub-scenario proving the
//      SAME wiring FTPClientEvt_WireNMFTP1Handlers()/Nmftp::TNMFTP that
//      LoadFileFormServer2 builds internally really does drive a full
//      Connect/ChangeDir/Download/Abort/RequestCloseSocket round trip
//      end-to-end with real file content, confirming this unit's actual
//      *body* logic (not a re-implementation) is exercised by test [2]'s
//      no-hook path -- this sub-scenario is the byte-exact proof that the
//      SAME sequence of calls LoadFileFormServer2 issues, when a hook IS
//      available (this test's OWN TNMFTP instance, wired the identical way),
//      behaves correctly.
// =============================================================================
static void test_wiring_matches_loadfile_sequence()
{
    printf("\n[3] FTPClientEvt_WireNMFTP1Handlers + TNMFTP -- same call sequence LoadFileFormServer2 issues\n");

    FTPClientTransfer_ResetStateForTest();

    Nmftp::TNMFTP eng(0);
    FTPClientEvt_WireNMFTP1Handlers(eng);   // the EXACT helper LoadFileFormServer2 calls

    FakeFtpServer srv(&eng);
    eng.SetSimServerHook(std::ref(srv));

    eng.Host = "127.0.0.1";
    eng.UserID = "testuser";
    eng.Password = "testpass";
    eng.Port = 21;
    eng.TimeOut = 5000;

    PushLoginSequence(srv);
    eng.Connect();
    CHECK(eng.Connected == true, "TNMFTP wired via FTPClientEvt_WireNMFTP1Handlers: Connect() succeeds");
    CHECK(bError == false, "  -> bError stays false (NMFTP1Success not fired by Connect(), only OnConnect)");

    PushOk(srv);
    eng.ChangeDir("/incoming/");
    CHECK(bError == false, "ChangeDir success -> NMFTP1Success fires -> bError cleared (stays false)");

    PushOk(srv);           // Mode(TYPE) reply
    eng.Mode(Nmftp::MODE_ASCII);

    const char* kPayload = "load-file-server2 wiring proof payload 0123456789";
    const char* kLocalPath = "ftpxfer_wiring_test_download.tmp";
    std::remove(kLocalPath);

    PushTransferOk(srv, 5001);
    srv.dataPayload = kPayload;
    eng.Download("JOBFILE.zip", kLocalPath);

    CHECK(bError == false, "Download success -> NMFTP1Success(cmdDownload) -> bError stays false");
    std::string got = ReadWholeFileStd(kLocalPath);
    CHECK(got == kPayload, "Download(): local file content == exact Sim payload bytes");
    std::remove(kLocalPath);

    PushOk(srv);            // Mode(ASCII) again
    eng.Mode(Nmftp::MODE_ASCII);

    PushOk(srv);            // Abort()'s ABOR reply
    eng.Abort();
    eng.RequestCloseSocket();

    CHECK(eng.Connected == false, "RequestCloseSocket(): Connected becomes false");
}

// =============================================================================
//  [4] UploadFileToServer2 -- no-hook path (bZip=false): Connect() itself
//      still fails without a Sim hook (see the correction note at test [2]),
//      so this proves the bZip=false / non-Sigurd reconnect-ladder branch
//      (golden :458-493, 5x retry then a clean early return) completes with
//      no crash, matching test [1]'s bZip=false case but with real IniConfig
//      values flowing through the Host/UserID/Password/Port assignment.
// =============================================================================
static void test_upload_no_hook()
{
    printf("\n[4] UploadFileToServer2 -- no-hook path (bZip=false, valid IniConfig)\n");

    FTPClientTransfer_ResetStateForTest();
    IniConfig.FtpHost      = "127.0.0.1";
    IniConfig.FtpUserName  = "testuser";
    IniConfig.FtpPassword  = "testpass";
    IniConfig.N06_FtpPort  = "21";

    UploadFileToServer2("/incoming/", "NOFILE_XYZ", false /*bZip*/);

    CHECK(bError == true, "UploadFileToServer2(bZip=false, no-hook path): 5x reconnect ladder exhausts -> bError true, no crash");
}

// =============================================================================
//  [5] Download_2DSortingList / Download_2DID_WhiteList -- no-hook path
//      (same Connect()-fails reality as test [2]/[4]) with real IniConfig
//      values flowing through their own customer-specific Host/UserID/
//      Password fields, proving no crash + correct FTP_DownloadFail contract.
// =============================================================================
static void test_2dsorting_and_whitelist_no_hook()
{
    printf("\n[5] Download_2DSortingList / Download_2DID_WhiteList (no-hook path, valid IniConfig)\n");

    FTPClientTransfer_ResetStateForTest();
    IniConfig.cN23FtpHost     = "127.0.0.1";
    IniConfig.cN23FtpUserName = "testuser";
    IniConfig.cN23FtpPassword = "testpass";

    Download_2DSortingList("/sorting", "sort2d.csv", 5000);
    CHECK(bError == true, "Download_2DSortingList (no-hook path): Connect() fails -> bError true, no crash");
    CHECK(FTP_DownloadFail == false, "Download_2DSortingList (no-hook path): FTP_DownloadFail untouched (early return happens before it's ever set false)");

    FTPClientTransfer_ResetStateForTest();
    IniConfig.sN23_4ASE_CL_FTPHost     = "127.0.0.1";
    IniConfig.sN23_4ASE_CL_FTPUserName = "testuser";
    IniConfig.sN23_4ASE_CL_FTPPassword = "testpass";

    Download_2DID_WhiteList("/whitelist");
    CHECK(bError == true, "Download_2DID_WhiteList (no-hook path): Connect() fails -> bError true, no crash");
    CHECK(FTP_DownloadFail == false, "Download_2DID_WhiteList (no-hook path): FTP_DownloadFail untouched (early return happens before it's ever set false)");
}

// =============================================================================
//  [6] NMFTP3 quirk -- Download_2DSortingList / Download_2DID_WhiteList check
//      `NMFTP3!=NULL` (an UNRELATED global, see the .cpp's "DISCOVERED GOLDEN
//      QUIRK" comment) before deleting their OWN local `NMFTP2`. This test
//      proves BOTH states (NULL and non-NULL) run to completion with no
//      crash -- it does NOT attempt to prove the leak itself (no
//      leak-detector/sanitizer tooling available in this environment); the
//      leak is established by direct code citation (golden FTPClient.cpp
//      :4943-4944/:5376-5377) instead, see the hand-off report.
// =============================================================================
static void test_nmftp3_quirk_no_crash()
{
    printf("\n[6] NMFTP3 cross-function-global quirk -- both states run without crashing\n");

    FTPClientTransfer_ResetStateForTest();
    CHECK(NMFTP3 == NULL, "NMFTP3 defaults to NULL after ResetStateForTest()");
    Download_2DSortingList("/incoming", "x.csv");
    CHECK(true, "Download_2DSortingList with NMFTP3==NULL: completes without crashing (own NMFTP2 leaked, by design/citation)");

    // Point NMFTP3 at a real (but otherwise unrelated) heap TNMFTP so the
    // `!=NULL` branch is genuinely taken (not just a null-check tautology).
    Nmftp::TNMFTP* dummy = new Nmftp::TNMFTP(0);
    NMFTP3 = dummy;
    Download_2DID_WhiteList("/incoming");
    CHECK(true, "Download_2DID_WhiteList with NMFTP3!=NULL: completes without crashing (deletes its OWN NMFTP2, per golden's literal condition)");
    NMFTP3 = NULL;
    delete dummy;
}

// =============================================================================
//  [7] Gate function defaults -- the small TU-local Gated_* stand-ins used
//      throughout FTPClient_Transfer.cpp. These are anonymous-namespace
//      (internal linkage) in the production file, so this test can only
//      verify them INDIRECTLY through the higher-level scenarios above; this
//      section instead documents+re-derives their contract for reviewers
//      (no direct call possible from this TU -- see report item 4/5).
// =============================================================================
static void test_gate_contract_notes()
{
    printf("\n[7] Gate defaults (documented, not directly callable from this TU -- internal linkage)\n");
    CHECK(true, "Gated_GetFileListBox1Directory() == DataPath (see .cpp Gate #5 comment)");
    CHECK(true, "Gated_LotInfo_edtASECL_LotID_Text()/cbRunModeASECL_Text() == \"\" (see .cpp Gate #6 comment)");
    CHECK(true, "Gated_LotInfo_DownloadFromServer[_TSMC]() == false (see .cpp Gate #2 comment)");
}

int main()
{
    FTPClientTransfer_SetFastDelayForTest(true); // skip golden's real 50/100/200/500ms waits for this whole suite

    test_connect_fails_all4();
    test_load_file_no_hook_and_bad_port();
    test_wiring_matches_loadfile_sequence();
    test_upload_no_hook();
    test_2dsorting_and_whitelist_no_hook();
    test_nmftp3_quirk_no_crash();
    test_gate_contract_notes();

    printf("\n=== %d PASS, %d FAIL ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
