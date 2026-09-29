// =============================================================================
//  test_ela_ftp.cpp -- ELA plan R4: the upload transport (ElaFtp) and N25-3 / N10 BYFILE (ElaChipMos) on a scripted fake.
//
//  AI(W906-ELA-R4) 20260927 (St02-E).  Suite name (add_test): ELA_Ftp
//
//     0. network guard: wininet.dll is not loaded (the WinINet transport is not even linked into this test) -- checked
//        again at the end; the log-root seam; the Hub's default transport is the null one;
//     1. pure helpers: reply codes, SIZE reply, masking, golden PathCombin, the BackUp name, FNV-1a, stagger offset;
//     2. back-off on a fake clock: six retries 1 / 2 / 4 / 8 / 16 / 30 min +-20 %, reproducible, give up after six or at
//        the next slot, reset on success;
//     3. N25-3 success: the exact CWD / MKD / STOR / SIZE / LIST sequence (golden base / host / BackUp), Jam Rate.txt
//        (JAM rows of Now-8 .. Now-1, UTF-8, LF), HadUpload.txt with both lines; the same day again = no transport;
//     4. SIZE mismatch: fails, no HadUpload.txt, retry at +60 s +-20 %; fixed -> the retry succeeds; Jam Rate.txt rebuilt;
//     5. STOR 550 on the main file / on the BackUp copy: fails, no HadUpload.txt (golden would block the retries);
//     6. SIZE unsupported (502): the LIST size decides; listed as 0 bytes = fail; no size in the listing = "name only";
//     7. LIST without the file = fail;
//     8. connect / login failure: no CWD / STOR, Close called, the account masked everywhere; a time-out is retryable;
//     9. multi-level folder walk (N10 ChangeDirectories, golden :120 MKD fixed);
//    10. N10 BYFILE: per-row session, verified upload, a missing local file, a malformed row, its gate;
//    11. the D-f gate: O10 / customer code / bN25_3 / host / path -> the transport factory is never called; through the
//        Hub too (config.ini without bN25_3); the Hub with the gate open runs the job on the fake;
//    12. the build's transport: WinINet in both builds (★W36 = C, Steven 20260928); the log-only transport runs
//        the whole N25-3 path, writes FTP_Log, sends nothing, never writes HadUpload.txt; the null transport fails;
//    13. the abort slot: after Abort() no session starts; Set() aborts a late transport; Reset();
//    14. FTP_Log: path, header, no account anywhere; the MyDBIProcess hook target (FtpLogDbiLine) writes one line.
//  W22 (AI(W906-ELA-W22) 20260928, St02-E):
//    15. N25-4: GetColumn / ParseDateTimeText, the Production_Log device count (W18 B every calendar day vs golden's
//        step from the start time), Jam_Summary.csv (CLEAN_OUT intervals, a JAM with CLEAN_OUT recovery is a JAM only,
//        LF, rebuilt), the N25-3 remote steps, HadUpload.txt = "HadUpload" only, once a day, a failed BackUp = retry,
//        golden mode, D-c no record = nothing sent, the D-f gate;
//    16. N25-5: EventLog.txt = a byte copy of yesterday's EventLogTxt_yyyymmdd.csv, the remote steps, HadUpload.txt
//        with both lines, no file of yesterday / an O15-named file only = nothing sent (FTP_Log says so), the gate;
//    17. the Hub: EL_UPLOAD_SUMMARY / EL_UPLOAD_EVENTLOG skipped with the switches off, run in golden order with them
//        on; the schedule bodies RunN25_4OnHub / RunN25_5OnHub (verified, then nothing to do, gate, no hub).
//  Every file is under %TEMP%\ht9045_ela_ftp_<tick> (removed on a green run); W906_HT9045LOG_ROOT, W906_EVENTLOG_ROOT
//  and (W22) W906_PRODLOG_ROOT are _putenv'd there first.  The account is made up at run time (no password literal).
// =============================================================================
#include "EventLogAnalysis/ElaChipMos.h"
#include "EventLogAnalysis/ElaFtp.h"
#include "EventLogAnalysis/ElaHub.h"
#include "EventLogAnalysis/ElaReports.h"   // ela::Clock (R2), the Hub's fake clock
#include "EventLogAnalysis/ElaSchedule.h"  // W22: RunN25_4OnHub / RunN25_5OnHub (section 17)
#include "ElaFtpFake.h"

#include <windows.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cctype>                  // AI(W906-ELA-REV): MachineDataPath
#include <cstring>
#include <string>
#include <vector>

// same guard as tests/test_ela_service.cpp:27-33 (MinGW.org 6.3 strict mode declares neither putenv nor _putenv).
// MUST stay a CRT call: getenv() reads the CRT's copy, which SetEnvironmentVariableA does not update.
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static FakeFtpScript g_s;
static int g_fakeCalls = 0;
static ela::IElaFtp* FakeFactory() { ++g_fakeCalls; ++g_s.created; return new FakeFtp(&g_s); }
static int g_countCalls = 0;
static ela::IElaFtp* CountingFactory() { ++g_countCalls; return ela::NewNullFtp(); }
static double g_fakeNow = 0.0;
class FakeClock : public ela::Clock
{
public:
    double Now() const { return g_fakeNow; }
};

static bool Has(const std::string& s, const std::string& t) { return s.find(t) != std::string::npos; }

static void MkDirs(const std::string& d)
{
    for (size_t i = 3; i < d.size(); ++i)
        if (d[i] == '\\') ::CreateDirectoryA(d.substr(0, i).c_str(), 0);
    ::CreateDirectoryA(d.c_str(), 0);
}

static void Put(const std::string& p, const std::string& text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fwrite(text.data(), 1, text.size(), f); std::fclose(f); }
}

static std::string Slurp(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    return s;
}

static bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

static std::string Num(unsigned long v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%lu", v);
    return b;
}

static double Day(int y, int m, int d, int h = 0, int n = 0, int s = 1) { return ela::EncodeDate(y, m, d) + ela::EncodeTime(h, n, s, 0); }

static void ResetRemote()
{
    g_s.Reset();
    g_s.dirs.insert("/Summary");
    g_s.dirs.insert("/Summary/naslfs2");
}

// AI(W906-ELA-W22) 20260928: section 3's step sequence for another base / file (after ResetRemote the base, the host
// folder and BackUp are all made)
static std::string N25Seq(const std::string& base, const std::string& host, const std::string& name, const std::string& backup)
{
    const std::string h = base + "/" + host, b = h + "/BackUp";
    return "CONNECT ftp.test:21 active\n"
           "CWD " + base + "\nMKD " + base + "\nCWD " + base + "\n"
           "CWD " + h + "\nMKD " + h + "\nCWD " + h + "\n"
           "CWD " + b + "\nMKD " + b + "\nCWD " + b + "\n"
           "CWD " + h + "\nSTOR " + h + "/" + name + "\nSIZE " + h + "/" + name + "\nLIST " + h + "\n"
           "CWD " + b + "\nSTOR " + b + "/" + backup + "\nSIZE " + b + "/" + backup + "\nLIST " + b + "\n"
           "CLOSE\n";
}

// a Production_Log row with In Time (field 5, eLoadTime) = t (W22)
static std::string ProdRow(const std::string& t) { return "12345,20261010,I1-1,0,0," + t + ",,,\r\n"; }

static int CountOf(const std::string& s, const std::string& t)
{
    int n = 0;
    for (size_t p = s.find(t); p != std::string::npos; p = s.find(t, p + 1)) ++n;
    return n;
}

// AI(W906-ELA-REV) 20260928 (St02-E): refuse-first -- no root or seam of this test may resolve into the machine's data
static bool MachineDataPath(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) s[i] = (s[i] == '/') ? '\\' : (char)std::tolower((unsigned char)s[i]);
    static const char* const k[] = { "d:\\ht9045", "d:\\rms", "d:\\mtbf_summary" };   // d:\ht9045 also covers d:\ht9045_log
    for (size_t i = 0; i < sizeof(k) / sizeof(k[0]); ++i)
        if (s.compare(0, std::strlen(k[i]), k[i]) == 0) return true;
    return false;
}

int main()
{
    printf("ELA_Ftp\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    const std::string root = std::string(tmp) + "ht9045_ela_ftp_" + Num(::GetTickCount());
    const std::string logRoot = root + "\\log";
    const std::string evRoot = root + "\\EventLogTxt";
    if (MachineDataPath(root))                                          // before anything is written
    {
        printf("  REFUSED: sandbox %s is under the machine's data (D:\\HT9045*, D:\\RMS, D:\\MTBF_Summary)\n", root.c_str());
        return 1;
    }
    MkDirs(logRoot);
    MkDirs(evRoot + "\\2026\\09");
    MkDirs(evRoot + "\\2026\\10");
    _putenv(("W906_HT9045LOG_ROOT=" + logRoot).c_str());
    _putenv(("W906_EVENTLOG_ROOT=" + evRoot).c_str());
    _putenv(("W906_PRODLOG_ROOT=" + root + "\\prod4").c_str());   // W22: the Hub's Production_Log root stays in the sandbox
    {
        const ela::QueryRequest q;                                      // the seams resolve into the sandbox, or nothing runs
        if (MachineDataPath(ela::ElaLogRoot()) || MachineDataPath(q.eventLogDir) || MachineDataPath(q.prodLogDir))
        {
            printf("  REFUSED: a seam still resolves under the machine's data (%s / %s / %s)\n", ela::ElaLogRoot().c_str(),
                   q.eventLogDir.c_str(), q.prodLogDir.c_str());
            return 1;
        }
    }
    // the account, made up at run time
    const std::string user = "usr" + Num(::GetTickCount() % 99991u + 17u);
    const std::string pw = "k" + Num((::GetTickCount() * 7u) % 999983u + 101u) + "q";

    // ---- 0 ----
    CHECK(::GetModuleHandleA("wininet.dll") == NULL, "0. wininet.dll is not loaded (the WinINet transport is not linked here)");
    CHECK(ela::ElaLogRoot() == logRoot, "0. W906_HT9045LOG_ROOT seam -> the %TEMP% sandbox");
    _putenv("W906_HT9045LOG_ROOT=");
    CHECK(ela::ElaLogRoot() == "D:\\HT9045_Log", "0. seam unset -> golden D:\\HT9045_Log (nothing written there)");
    _putenv(("W906_HT9045LOG_ROOT=" + logRoot).c_str());
    {
        ela::Options o;
        o.jamIniPath = root + "\\JAM0000.dat";
        ela::HubPaths hp;
        hp.configIni = root + "\\none.ini";
        hp.generalIni = root + "\\none2.ini";
        ela::Hub h(o, hp);
        CHECK(h.GetFtpFactory() == &ela::NewNullFtp, "0. a Hub's default transport is the null one");
    }

    // ---- 1 ----
    long long sz = -1;
    CHECK(ela::FtpReplyCode("150 opening\r\n226 done\r\n") == 226 && ela::FtpReplyCode("") == 0 &&
              ela::FtpReplyCode("213-a\r\n213 7\r\n") == 213, "1. reply code = the last final line");
    CHECK(ela::ParseSizeReply("213 1234\r\n", &sz) && sz == 1234, "1. SIZE reply 213 1234");
    CHECK(ela::ParseSizeReply("213-x\r\n213 99", &sz) && sz == 99 && !ela::ParseSizeReply("550 no such file", &sz) &&
              !ela::ParseSizeReply("213 abc", &sz), "1. SIZE reply: multi-line, 550, junk");
    CHECK(ela::MaskSecrets("331 user " + user + " pass " + pw, user, pw) == "331 user *** pass ***" &&
              ela::MaskSecrets("x", "", "") == "x", "1. MaskSecrets");
    CHECK(ela::FtpPathCombin("/a/", "b") == "/a/b" && ela::FtpPathCombin("/a", "b") == "/a/b" &&
              ela::FtpPathCombin("C:\\x", "b") == "C:\\x\\b" && ela::FtpPathCombin("C:\\x\\", "b") == "C:\\x\\b" &&
              ela::FtpPathCombin("", "b") == "b", "1. golden PathCombin (FTP '/', local '\\')");
    CHECK(ela::FtpTrimTrailingSlashes("/a//") == "/a" && ela::FtpParentDir("/a/b") == "/a" && ela::FtpParentDir("/a") == "/" &&
              ela::FtpBaseName("/a/b c.txt") == "b c.txt", "1. trailing slashes / parent / base name");
    CHECK(ela::BackupFileName("Jam Rate.txt", Day(2026, 9, 27, 13, 45, 59)) == "Jam Rate_20260927_1345.txt",
          "1. BackUp name <name>_yyyymmdd_hhnn.<ext> (golden :200)");
    CHECK(ela::Fnv1a32("") == 0x811C9DC5u && ela::Fnv1a32("a") == 0xE40C292Cu, "1. FNV-1a-32 known values");
    CHECK(ela::StaggerOffsetSeconds("", 900) == 61u && ela::StaggerOffsetSeconds("x", 0) == 0u, "1. stagger = FNV mod W");

    // ---- 2 ----
    {
        const std::string id = "M-TEST";
        const double base[7] = { 60, 120, 240, 480, 960, 1800, 1800 };
        bool inBand = true, same = true, differs = false;
        for (int n = 1; n <= 7; ++n)
        {
            // AI(W906-ELA-FTPfix) 20260928 (St02-E): all three through memory.  MinGW 32-bit x87 kept the fresh call in an
            //   80-bit register and compared it with the stored 64-bit b: "same" failed (St01 20:40, both configs).  The
            //   jitter itself differs per ID (M-TEST / M-OTHER FNV mod 20001: 15077 / 19808 at n = 1, never equal for 1..7).
            const volatile double b = ela::BackoffSeconds(n, id);
            const volatile double again = ela::BackoffSeconds(n, id);
            const volatile double other = ela::BackoffSeconds(n, "M-OTHER");
            if (b < base[n - 1] * 0.8 - 1e-9 || b > base[n - 1] * 1.2 + 1e-9) inBand = false;
            if (b != again) same = false;
            if (b != other) differs = true;
        }
        CHECK(inBand, "2. back-off n = 1..7 within +-20 % of 60 / 120 / 240 / 480 / 960 / 1800 / 1800 s");
        CHECK(same, "2. reproducible per Machine ID");
        CHECK(differs, "2. different for another Machine ID");
        const double t0 = Day(2026, 9, 27);
        const double slot = ela::NextN25Slot(t0);
        CHECK(std::fabs(slot - Day(2026, 9, 28)) < 1e-9, "2. next N25 slot = tomorrow 00:00:01 (golden main.cpp:21442)");
        ela::UploadRetry r;
        double now = t0;
        bool okSeq = true;
        for (int n = 1; n <= ela::kMaxUploadRetries; ++n)
        {
            ela::OnUploadFailed(&r, now, id, slot);
            const double want = now + ela::BackoffSeconds(n, id) / 86400.0;
            if (r.gaveUp || std::fabs(r.nextTry - want) > 1e-9 || ela::RetryDue(r, now) || !ela::RetryDue(r, r.nextTry))
                okSeq = false;
            now = r.nextTry;   // the fake clock jumps to the due time, no sleep
        }
        CHECK(okSeq && r.failures == 6, "2. six retries on the fake clock, each due exactly at its back-off");
        ela::OnUploadFailed(&r, now, id, slot);
        CHECK(r.gaveUp && !ela::RetryDue(r, now + 1.0), "2. the 7th failure gives up");
        ela::UploadRetry r2;
        ela::OnUploadFailed(&r2, slot - 30.0 / 86400.0, id, slot);
        CHECK(r2.gaveUp, "2. a retry that would reach the next slot gives up (the slot takes over)");
        ela::OnUploadSucceeded(&r);
        CHECK(r.failures == 0 && !r.gaveUp && r.nextTry == 0.0, "2. success resets");
    }

    // ---- fixtures: EventLogTxt (cp950 message on one row) ----
    const std::string hdr = "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n";
    Put(evRoot + "\\2026\\09\\EventLogTxt_20260918.csv", hdr + "2026/09/18,23:00:00,\"01 Input Arm\",JAM0188,1,5,0,\"old\",,R1\r\n");
    Put(evRoot + "\\2026\\09\\EventLogTxt_20260920.csv",
        hdr + "2026/09/20,08:10:00,\"01 Input Arm\",JAM0101,1,30,0,\"Pick fail\",,R1\r\n"
              "2026/09/20,08:20:00,\"24 Motor\",WAR2401,1,20,0,\"Motor alarm\",,R1\r\n"
              "2026/09/20,08:30:00,\"01 Input Arm\",JAM0102,1,30,1,\"dup\",,R1\r\n"
              "2026/09/20,09:00:00,\"03 Index Unit\",JAM0301,1,10,0,\"\xA4\xA4\",,R1\r\n");
    Put(evRoot + "\\2026\\09\\EventLogTxt_20260926.csv", hdr + "2026/09/26,23:59:59,\"05 Output Shuttle\",JAM0501,1,5,0,\"edge\",,R1\r\n");
    Put(evRoot + "\\2026\\09\\EventLogTxt_20260927.csv", hdr + "2026/09/27,00:00:00,\"01 Input Arm\",JAM0199,1,5,0,\"today\",,R1\r\n");

    ela::N25Settings s;
    s.o10 = true;
    s.enableJamLog = true;
    s.custCode = "851";
    s.user = user;
    s.password = pw;
    s.host = "ftp.test";
    s.jamLogPath = "/Summary/naslfs2/Handler/";
    ela::ElaUploadContext c;
    c.logRoot = logRoot;
    c.eventLogDir = evRoot;
    c.hostName = "HOSTX";
    c.machineId = "M-TEST";
    c.factory = &FakeFactory;

    // ---- 3 ----
    {
        ResetRemote();
        g_fakeCalls = 0;
        c.now = Day(2026, 9, 27);
        const ela::N25Result r = ela::UploadJamCode(c, s);
        const std::string want =
            "CONNECT ftp.test:21 active\n"
            "CWD /Summary/naslfs2/Handler\nMKD /Summary/naslfs2/Handler\nCWD /Summary/naslfs2/Handler\n"
            "CWD /Summary/naslfs2/Handler/HOSTX\nMKD /Summary/naslfs2/Handler/HOSTX\nCWD /Summary/naslfs2/Handler/HOSTX\n"
            "CWD /Summary/naslfs2/Handler/HOSTX/BackUp\nMKD /Summary/naslfs2/Handler/HOSTX/BackUp\n"
            "CWD /Summary/naslfs2/Handler/HOSTX/BackUp\n"
            "CWD /Summary/naslfs2/Handler/HOSTX\nSTOR /Summary/naslfs2/Handler/HOSTX/Jam Rate.txt\n"
            "SIZE /Summary/naslfs2/Handler/HOSTX/Jam Rate.txt\nLIST /Summary/naslfs2/Handler/HOSTX\n"
            "CWD /Summary/naslfs2/Handler/HOSTX/BackUp\nSTOR /Summary/naslfs2/Handler/HOSTX/BackUp/Jam Rate_20260927_0000.txt\n"
            "SIZE /Summary/naslfs2/Handler/HOSTX/BackUp/Jam Rate_20260927_0000.txt\nLIST /Summary/naslfs2/Handler/HOSTX/BackUp\n"
            "CLOSE\n";
        CHECK(g_s.Joined() == want, "3. golden order: base / host / BackUp (CWD fail -> MKD -> CWD), STOR + SIZE + LIST x2, CLOSE");
        if (g_s.Joined() != want) printf("%s", g_s.Joined().c_str());
        CHECK(r.ok() && r.mainOk && r.backupOk && r.connected && r.factoryCalls == 1 && g_fakeCalls == 1,
              "3. verified main + BackUp, one session");
        const std::string folder = logRoot + "\\JamWeek\\2026\\9\\27";
        CHECK(r.saveFolder == folder, "3. local folder JamWeek\\y\\m\\d (golden :332, no zero pad)");
        const std::string jam =
            "date,time,unitName,alarmCode,recovery,stoppedTime,duplicate,message,errorPart\n"
            "2026/09/20,08:10:00,01 Input Arm,JAM0101,1,30,0,Pick fail,\n"
            "2026/09/20,09:00:00,03 Index Unit,JAM0301,1,10,0,\xE4\xB8\xAD,\n"
            "2026/09/26,23:59:59,05 Output Shuttle,JAM0501,1,5,0,edge,\n";
        CHECK(Slurp(folder + "\\Jam Rate.txt") == jam && r.records == 3,
              "3. Jam Rate.txt: JAM rows of 09-19 .. 09-26, no WAR / Duplicate / out of range, UTF-8 (D-e), LF");
        CHECK(Slurp(folder + "\\HadUpload.txt") ==
                  "Upload path:/Summary/naslfs2/Handler/HOSTX/Jam Rate.txt\n"
                  "Upload backup path:/Summary/naslfs2/Handler/HOSTX/BackUp/Jam Rate_20260927_0000.txt\nHadUpload\n" &&
                  r.hadUploadWritten, "3. HadUpload.txt after both verified, BackUp line names the BackUp folder");
        CHECK(g_s.files["/Summary/naslfs2/Handler/HOSTX/Jam Rate.txt"] == (long long)jam.size(), "3. remote size = local bytes");
        const ela::N25Result again = ela::UploadJamCode(c, s);
        CHECK(again.alreadyUploaded && g_fakeCalls == 1 && !again.ok(), "3. same day again: HadUpload.txt -> no transport (golden :336)");
    }

    // ---- 4 ----
    {
        ResetRemote();
        g_fakeCalls = 0;
        c.now = Day(2026, 9, 28);
        g_s.sizeLie["/Summary/naslfs2/Handler/HOSTX/Jam Rate.txt"] = 1;
        ela::N25Result r = ela::UploadJamCode(c, s);
        const std::string folder = logRoot + "\\JamWeek\\2026\\9\\28";
        CHECK(!r.ok() && !r.mainOk && r.backupOk && r.retryable && !Exists(folder + "\\HadUpload.txt") &&
                  Has(r.detail, "SIZE mismatch"), "4. SIZE mismatch: failed, no HadUpload.txt, retryable");
        ela::UploadRetry rr;
        ela::OnUploadFailed(&rr, c.now, c.machineId, ela::NextN25Slot(c.now));
        const double wait = (rr.nextTry - c.now) * 86400.0;
        CHECK(!rr.gaveUp && wait >= 48.0 - 1e-6 && wait <= 72.0 + 1e-6, "4. first retry at +60 s +-20 %");
        g_s.sizeLie.clear();
        c.now = rr.nextTry;
        r = ela::UploadJamCode(c, s);
        CHECK(r.ok() && Exists(folder + "\\HadUpload.txt") && g_fakeCalls == 2, "4. the retry succeeds once the server agrees");
        const std::string j = Slurp(folder + "\\Jam Rate.txt");
        CHECK(j.find("date,time") == 0 && j.find("date,time", 1) == std::string::npos && r.records == 4,
              "4. Jam Rate.txt rebuilt (one header, 4 rows for 09-20 .. 09-27), not appended (plan §3 item 3)");
    }

    // ---- 5 ----
    {
        ResetRemote();
        c.now = Day(2026, 9, 29);
        g_s.storFail["/Summary/naslfs2/Handler/HOSTX/Jam Rate.txt"] = 550;
        ela::N25Result r = ela::UploadJamCode(c, s);
        CHECK(!r.ok() && !r.mainOk && r.backupOk && g_s.Count("STOR ") == 2 &&
                  !Exists(logRoot + "\\JamWeek\\2026\\9\\29\\HadUpload.txt"),
              "5. STOR 550 on the main file: BackUp still tried (golden :235), no HadUpload.txt");
        ResetRemote();
        c.now = Day(2026, 9, 30);
        g_s.storFail["/Summary/naslfs2/Handler/HOSTX/BackUp/Jam Rate_20260930_0000.txt"] = 550;
        r = ela::UploadJamCode(c, s);
        CHECK(!r.ok() && r.mainOk && !r.backupOk && !Exists(logRoot + "\\JamWeek\\2026\\9\\30\\HadUpload.txt"),
              "5. STOR 550 on the BackUp copy: no HadUpload.txt (golden :227 would have created it and blocked retries)");
    }

    // ---- 6 / 7 ----
    {
        const std::string local = root + "\\a.txt";
        Put(local, "0123456789");
        ela::FtpLog log(logRoot);
        ela::FtpEndpoint ep;
        ep.host = "ftp.test";
        const double now = Day(2026, 10, 1);
        ResetRemote();
        g_s.dirs.insert("/d");
        g_s.sizeUnsupported = true;
        {
            FakeFtp f(&g_s);
            f.Connect(ep);
            ela::UploadVerify v = ela::UploadAndVerify(&f, local, "/d", "a.txt", &log, now);
            CHECK(v.ok && !v.nameOnly && v.remoteSize == 10, "6. SIZE 502 -> the LIST size (10) decides");
            g_s.listSizeMode = 2;
            v = ela::UploadAndVerify(&f, local, "/d", "a.txt", &log, now);
            CHECK(!v.ok && v.step == "list", "6. listed as 0 bytes while the local file is not empty -> fail");
            g_s.listSizeMode = 1;
            v = ela::UploadAndVerify(&f, local, "/d", "a.txt", &log, now);
            CHECK(v.ok && v.nameOnly, "6. no SIZE, no size in the listing -> verified by name only (flagged)");
            g_s.sizeUnsupported = false;
            g_s.listSizeMode = 0;
            g_s.listHide.insert("a.txt");
            v = ela::UploadAndVerify(&f, local, "/d", "a.txt", &log, now);
            CHECK(!v.ok && v.step == "list", "7. LIST without the file -> fail");
            v = ela::UploadAndVerify(&f, root + "\\nope.txt", "/d", "a.txt", &log, now);
            CHECK(!v.ok && v.step == "local", "7. missing local file -> fail before any STOR");
        }
    }

    // ---- 8 ----
    {
        ResetRemote();
        g_s.connectFail = true;
        g_s.connectErr = ela::kFtpErrLogin;
        g_s.connectCode = 530;
        g_s.connectText = "530 User " + user + " cannot log in (password " + pw + ")";
        c.now = Day(2026, 10, 2);
        const ela::N25Result r = ela::UploadJamCode(c, s);
        CHECK(!r.connected && g_s.Count("CWD") == 0 && g_s.Count("STOR") == 0 && g_s.Count("CLOSE") == 1,
              "8. login failure: no CWD / STOR, Close called");
        CHECK(!Has(r.detail, user) && !Has(r.detail, pw) && Has(r.detail, "***") && r.retryable,
              "8. the account is masked in the result");
        ResetRemote();
        g_s.connectFail = true;
        g_s.connectErr = ela::kFtpErrTimeout;
        g_s.connectText = "deadline passed";
        c.now = Day(2026, 10, 3);
        const ela::N25Result t = ela::UploadJamCode(c, s);
        CHECK(!t.connected && t.retryable, "8. a time-out is retryable");
    }

    // ---- 9 ----
    {
        g_s.Reset();
        FakeFtp f(&g_s);
        ela::FtpEndpoint ep;
        f.Connect(ep);
        g_s.calls.clear();
        const bool ok = ela::EnsureDirPath(&f, "/a/b/c/", 0, 0.0);
        CHECK(ok && g_s.dirs.count("/a/b/c") == 1 &&
                  g_s.Joined() == "CWD /a\nMKD /a\nCWD /a\nCWD /a/b\nMKD /a/b\nCWD /a/b\nCWD /a/b/c\nMKD /a/b/c\nCWD /a/b/c\n",
              "9. every level: CWD, on failure MKD <that level> + CWD (golden :120 MKDs the parent)");
    }

    // ---- 10 ----
    {
        ResetRemote();
        g_fakeCalls = 0;
        MkDirs(root + "\\n10src");
        Put(root + "\\n10src\\f1.txt", "n10 payload");
        MkDirs(logRoot + "\\UploadFile");
        Put(logRoot + "\\UploadFile\\UploadFile.csv",
            "SourcePath,TargetPath,SourceFile,TargetFile\r\n" + root + "\\n10src,/N10/up,f1.txt,f1_up.txt\r\n" + root +
                "\\n10src,/N10/up,missing.txt,m.txt\r\nbad,row\r\n");
        ela::N10Settings n;
        n.o10 = true;
        n.uploadMethod = 0;
        n.host = "n10.test";
        n.user = user;
        n.password = pw;
        c.now = Day(2026, 10, 4);
        const ela::N10Result r = ela::UploadByFile_N10(c, n);
        CHECK(r.listFound && r.rows == 2 && r.uploaded == 1 && r.missing == 1 && r.failed == 0 && g_fakeCalls == 2,
              "10. BYFILE: 2 valid rows (one session each), 1 verified, 1 local file missing, the 2-field row ignored");
        CHECK(g_s.files["/N10/up/f1_up.txt"] == 11 && g_s.Count("CONNECT n10.test:21") == 2 && g_s.Count("MKD /N10/up") == 1,
              "10. N10 account, folders made level by level, the file stored");
        const std::string lg = Slurp(ela::FtpLog(logRoot).FileFor(c.now));
        CHECK(Has(lg, "Upload, , " + root + "\\n10src, /N10/up, f1.txt, f1_up.txt") && Has(lg, "File not exist N10"),
              "10. FTP_Log: golden AddTextForUpload line and the missing-file line");
        n.uploadMethod = 1;
        const int before = g_fakeCalls;
        const ela::N10Result off = ela::UploadByFile_N10(c, n);
        CHECK(!off.gate.empty() && g_fakeCalls == before, "10. iN10UploadMethod != 0 (network drive): no transport");
    }

    // ---- 11 ----
    {
        g_countCalls = 0;
        ela::ElaUploadContext k = c;
        k.factory = &CountingFactory;
        k.now = Day(2026, 11, 1);
        bool allClosed = true;
        for (int i = 0; i < 5; ++i)
        {
            ela::N25Settings t = s;
            if (i == 0) t.o10 = false;
            if (i == 1) t.custCode = "910";
            if (i == 2) t.enableJamLog = false;
            if (i == 3) t.host = " ";
            if (i == 4) t.jamLogPath = "";
            const ela::N25Result r = ela::UploadJamCode(k, t);
            if (r.gate.empty() || !r.saveFolder.empty()) allClosed = false;
        }
        CHECK(allClosed && g_countCalls == 0 && !Exists(logRoot + "\\JamWeek\\2026\\11"),
              "11. D-f gate (O10 / 851 / bN25_3 / host / path): the transport factory is never called, nothing written");

        // through the Hub
        const std::string cfg = root + "\\config.ini";
        const std::string gen = root + "\\Gerneral.ini";
        Put(gen, "[System]\r\nCUSTOMER_CODE=851\r\n[Version]\r\nMachine ID=M-TEST\r\n");
        Put(cfg, "[Event Log]\r\nEnableAutoSaveEventLog=1\r\n[ChipMos Function]\r\nsN25_2_FTPHost=ftp.test\r\n"
                 "sN25_2_FTPUserName=" + user + "\r\nsN25_2_FTPPassword=" + pw + "\r\n"
                 "sN25_3_JamLogFTPPath=/Summary/naslfs2/Handler/\r\n");
        ela::Options o;
        o.jamIniPath = root + "\\JAM0000.dat";
        ela::HubPaths hp;
        hp.configIni = cfg;
        hp.generalIni = gen;
        {
            ela::Hub h(o, hp);
            h.SetFtpFactory(&CountingFactory);
            h.Post(ela::EL_UPDATE_PARAMETER);
            h.RunOnce();
            h.Post(ela::EL_UPLOAD_JAMWEEK);
            h.RunOnce();
            const std::string js = h.SnapshotJson(0);
            CHECK(Has(js, "EL_UPLOAD_JAMWEEK: skipped, N25-3 bN25_3_EnableULJamLog is off") && g_countCalls == 0 &&
                      !Has(js, pw), "11. Hub: config.ini without bN25_3 -> skipped, no transport, no password in the snapshot");
        }
        // the same section (a second [ChipMos Function] would not be read): profile API, like the Handler's own writes
        ::WritePrivateProfileStringA("ChipMos Function", "bN25_3_EnableULJamLog", "1", cfg.c_str());
        {
            ResetRemote();
            g_fakeCalls = 0;
            g_fakeNow = Day(2026, 10, 5);
            ela::Hub h(o, hp);
            h.SetFtpFactory(&FakeFactory);
            FakeClock fc;
            h.SetClock(&fc);
            h.Post(ela::EL_UPDATE_PARAMETER);
            h.RunOnce();
            h.Post(ela::EL_UPLOAD_JAMWEEK);
            h.RunOnce();
            const std::string js = h.SnapshotJson(0);
            CHECK(g_fakeCalls == 1 && Has(js, "EL_UPLOAD_JAMWEEK: 1 JAM row(s), uploaded and verified") && !Has(js, pw),
                  "11. Hub: gate open -> the job runs on the installed transport (fake clock 2026-10-05)");
            CHECK(Exists(logRoot + "\\JamWeek\\2026\\10\\5\\HadUpload.txt"), "11. Hub: HadUpload.txt in the sandbox");
        }
    }

    // ---- 12 ----
    {
        // AI(W906-ELA-W36) 20260928 (St02-E helper): ★W36 = C -- the same in both builds (until then the sim build
        //   installed the log-only transport).  This test never calls W906_ElaStart; its hubs keep the null default
        //   (section 0) or get the fake / the log-only transport injected.
        CHECK(ela::BuildFtpTransport() == ela::kFtpTransportWinInet,
              "12. both builds: W906_ElaStart installs WinINet (★W36 = C; the D-f configuration decides whether it connects)");
        const double t0 = ela::ElaNow();
        ela::ElaUploadContext k = c;
        k.factory = &ela::NewLogOnlyFtp;
        k.now = Day(2026, 10, 6);
        const ela::N25Result r = ela::UploadJamCode(k, s);
        const double t1 = ela::ElaNow();
        CHECK(r.logOnly && r.mainOk && r.backupOk && !r.ok() && !r.hadUploadWritten &&
                  !Exists(logRoot + "\\JamWeek\\2026\\10\\6\\HadUpload.txt") && Has(r.detail, "log only"),
              "12. log-only transport: the whole path runs, nothing sent, no HadUpload.txt");
        const std::string lg = Slurp(ela::FtpLog(logRoot).FileFor(t0)) + Slurp(ela::FtpLog(logRoot).FileFor(t1));
        CHECK(Has(lg, "FTP LogOnly") && Has(lg, "CONNECT (nothing sent") && Has(lg, "STOR") && !Has(lg, pw) && !Has(lg, user),
              "12. log-only transport writes what it would send to FTP_Log, without the account");
        ela::ElaUploadContext z = c;
        z.factory = 0;
        z.now = Day(2026, 10, 7);
        const ela::N25Result nr = ela::UploadJamCode(z, s);
        CHECK(!nr.connected && Has(nr.detail, "null transport"), "12. null transport (factory 0): Connect refused");
    }

    // ---- 13 ----
    {
        ela::FtpAbortSlot slot;
        slot.Abort();
        g_fakeCalls = 0;
        ela::ElaUploadContext k = c;
        k.slot = &slot;
        k.now = Day(2026, 10, 8);
        const ela::N25Result r = ela::UploadJamCode(k, s);
        CHECK(g_fakeCalls == 0 && Has(r.detail, "stopping"), "13. after Abort() no session starts");
        g_s.Reset();
        FakeFtp late(&g_s);
        slot.Set(&late);
        CHECK(g_s.Count("ABORT") == 1, "13. a transport registered after Abort() is aborted at once");
        slot.Set(0);
        slot.Reset();
        CHECK(!slot.Stopping(), "13. Reset() (Hub::Start) re-arms the slot");
    }

    // ---- 14 ----
    {
        const std::string f = ela::FtpLog(logRoot).FileFor(Day(2026, 9, 27));
        const std::string lg = Slurp(f);
        CHECK(f == logRoot + "\\UploadFile\\2026\\09\\FTP_Log_20260927.csv", "14. FTP_Log path UploadFile\\yyyy\\mm\\FTP_Log_yyyymmdd.csv");
        CHECK(lg.find("Date, Time, Action, S2, S3, S4, S5, S6\r\n") == 0 && Has(lg, "Upload verified") &&
                  Has(lg, "2026-09-27, 00:00:01.000, "), "14. FTP_Log header (golden FirstRow) and dated lines");
        bool clean = true;
        const char* days[] = { "20260927", "20260928", "20260929", "20260930", "20261001", "20261002", "20261003", "20261004",
                               "20261005", "20261006", "20261007", "20261008" };
        for (size_t i = 0; i < sizeof(days) / sizeof(days[0]); ++i)
        {
            const std::string d = days[i];
            const std::string p = logRoot + "\\UploadFile\\" + d.substr(0, 4) + "\\" + d.substr(4, 2) + "\\FTP_Log_" + d + ".csv";
            const std::string t = Slurp(p);
            if (Has(t, pw) || Has(t, user)) clean = false;
        }
        CHECK(clean, "14. no FTP_Log file holds the account");
        const double t0 = ela::ElaNow();
        ela::FtpLogDbiLine("Exception,MyForceDirectories,R4-hook-probe,,,,");
        const double t1 = ela::ElaNow();
        const std::string hk = Slurp(ela::FtpLog(logRoot).FileFor(t0)) + Slurp(ela::FtpLog(logRoot).FileFor(t1));
        CHECK(Has(hk, ", Exception,MyForceDirectories,R4-hook-probe,,,,\r\n"),
              "14. FtpLogDbiLine (the MyDBIProcess hook W906_ElaStart installs) appends the CommaText line");
    }

    // ---- 15: W22 N25-4 (AI(W906-ELA-W22) 20260928) ----
    const std::string ev4 = root + "\\ev4";
    const std::string prod4 = root + "\\prod4";
    MkDirs(ev4 + "\\2026\\10");
    MkDirs(ev4 + "\\2026\\11");
    MkDirs(prod4 + "\\202610");
    Put(ev4 + "\\2026\\10\\EventLogTxt_20261001.csv", hdr + "2026/10/01,12:00:00,\"01 Input Arm\",JAM0100,1,5,0,\"before\",,R1\r\n");
    Put(ev4 + "\\2026\\10\\EventLogTxt_20261010.csv",
        hdr + "2026/10/10,08:00:00,\"01 Input Arm\",JAM0101,1,30,0,\"a\",,R1\r\n"
              "2026/10/10,09:00:00,\"16 System\",MES1601,CLEAN_OUT,0,0,\"clean out\",,R1\r\n"
              "2026/10/10,10:00:00,\"01 Input Arm\",JAM0102,1,30,0,\"b\",,R1\r\n"
              "2026/10/10,10:30:00,\"01 Input Arm\",JAM0103,CLEAN_OUT,30,0,\"jam + clean out\",,R1\r\n"
              "2026/10/10,10:40:00,\"01 Input Arm\",JAM0105,1,30,1,\"dup\",,R1\r\n"
              "2026/10/10,11:00:00,\"24 Motor\",WAR2401,1,20,0,\"w\",,R1\r\n");
    Put(ev4 + "\\2026\\10\\EventLogTxt_20261011.csv",
        hdr + "2026/10/11,07:00:00,\"16 System\",MES1601,CLEAN_OUT,0,0,\"clean out\",,R1\r\n"
              "2026/10/11,08:00:00,\"01 Input Arm\",JAM0104,1,30,0,\"d\",,R1\r\n");
    Put(ev4 + "\\2026\\11\\EventLogTxt_20261102.csv", hdr + "2026/11/02,00:00:00,\"01 Input Arm\",JAM0199,1,5,0,\"today\",,R1\r\n");
    const std::string ph = "Schedule name, Start time, Input tray, In X, In Y, In Time, Hot X, Hot Y, Hot Time\r\n";
    Put(prod4 + "\\202610\\HOSTX_20261010.csv", ph + ProdRow("\"2026/10/10 08:30:00\"") + ProdRow("\"2026/10/10 09:00:00\"") +
                                                ProdRow("") + ProdRow("\"not a time at all!!\"") + ProdRow("\"2026/10/10 20:00:00\""));
    Put(prod4 + "\\202610\\HOSTX_20261011.csv",
        ph + ProdRow("\"2026/10/11 06:00:00\"") + "12345,20261011,I1-1,0,0,\"2026/10/11 08:00:00\",,,");   // no newline at the end
    Put(prod4 + "\\202610\\HOSTY_20261010.csv", ph + ProdRow("\"2026/10/10 10:00:00\""));
    ela::N25UploadSettings u;
    u.o10 = true;
    u.enable = true;
    u.custCode = "851";
    u.user = user;
    u.password = pw;
    u.host = "ftp.test";
    u.path = "/Summary/naslfs2/Sum4/";
    ela::ElaUploadContext k4 = c;
    k4.eventLogDir = ev4;
    k4.prodLogDir = prod4;
    {
        CHECK(ela::GetColumn("a,\"b,c\",d", 1) == "b,c" && ela::GetColumn("a,b", 5) == "" && ela::GetColumn("a,b,", 2) == "" &&
                  ela::GetColumn("12345,x,\"2025/09/10 13:28:42\",y", 2) == "2025/09/10 13:28:42" && ela::GetColumn("", 0) == "",
              "15. GetColumn (golden getColumn, Common.cpp:442-467): quotes toggle and are dropped, past the end = empty");
        const double t9 = ela::EncodeDate(2026, 10, 10) + ela::EncodeTime(9, 0, 0, 0);
        CHECK(std::fabs(ela::ParseDateTimeText(" 2026/10/10 09:00:00 ") - t9) < 1e-9 &&
                  std::fabs(ela::ParseDateTimeText("2026/10/10 09:00:00.500") - t9) < 1e-9 &&
                  ela::ParseDateTimeText("2026/10/10 9:00") == 0.0 && ela::ParseDateTimeText("") == 0.0 &&
                  ela::ParseDateTimeText("2026/10/10T09:00:00") == 0.0,
              "15. ParseDateTimeText (golden ParseDateTime(AnsiString), :692-713): trim, < 19 chars / no space = 0, the time's first 8 chars");
        const double e7 = ela::EncodeDate(2026, 10, 11) + ela::EncodeTime(7, 0, 0, 0);
        CHECK(ela::ProductionDeviceCount(prod4, "HOSTX", t9, e7, false) == 3 && ela::ProductionDeviceCount(prod4, "HOSTX", t9, e7, true) == 2,
              "15. devices 10-10 09:00 .. 10-11 07:00: W18 B reads both days (3); golden steps from 09:00 and skips 10-11 (2)");
        CHECK(ela::ProductionDeviceCount(prod4, "HOSTY", t9, e7, false) == 1 && ela::ProductionDeviceCount(prod4, "HOSTZ", t9, e7, false) == 0 &&
                  ela::ProductionDeviceCount(root + "\\none", "HOSTX", t9, e7, false) == 0,
              "15. devices: <host>_yyyymmdd.csv of the given host only; no file / no folder = 0");

        ResetRemote();
        g_fakeCalls = 0;
        k4.now = Day(2026, 11, 2);
        const ela::N25Result r = ela::UploadSummaryCount(k4, u);
        const std::string folder = logRoot + "\\SummaryCount\\2026\\11\\2";
        const std::string want = "Time,Summary,JamCount\n2026/10/10 09:00:00,2,1\n2026/10/11 07:00:00,3,2\n2026/11/01 23:59:59,1,1\n";
        CHECK(r.saveFolder == folder && r.jamFile == "Jam_Summary.csv" && Slurp(folder + "\\Jam_Summary.csv") == want,
              "15. Jam_Summary.csv: SummaryCount\\y\\m\\d, one row per CLEAN_OUT interval (end time, devices, JAM records), LF");
        if (Slurp(folder + "\\Jam_Summary.csv") != want) printf("%s", Slurp(folder + "\\Jam_Summary.csv").c_str());
        CHECK(r.intervals == 3 && r.records == 4 && r.devices == 6,
              "15. 3 intervals, 4 JAM (JAM + CLEAN_OUT recovery = a JAM only; duplicate / WAR / 10-01 / today out), 6 devices");
        const std::string seq = N25Seq("/Summary/naslfs2/Sum4", "HOSTX", "Jam_Summary.csv", "Jam_Summary_20261102_0000.csv");
        CHECK(g_s.Joined() == seq && g_fakeCalls == 1, "15. the N25-3 steps: base / host / BackUp, STOR + SIZE + LIST x2, CLOSE");
        if (g_s.Joined() != seq) printf("%s", g_s.Joined().c_str());
        CHECK(r.ok() && r.mainOk && r.backupOk && r.hadUploadWritten && Slurp(folder + "\\HadUpload.txt") == "HadUpload\n",
              "15. both verified -> HadUpload.txt holds the marker only (golden :368)");
        CHECK(g_s.files["/Summary/naslfs2/Sum4/HOSTX/Jam_Summary.csv"] == (long long)want.size() &&
                  g_s.files["/Summary/naslfs2/Sum4/HOSTX/BackUp/Jam_Summary_20261102_0000.csv"] == (long long)want.size(),
              "15. remote sizes = the local bytes");
        const ela::N25Result again = ela::UploadSummaryCount(k4, u);
        CHECK(again.alreadyUploaded && g_fakeCalls == 1 && !again.ok(), "15. the same day again: HadUpload.txt -> no transport (golden :360)");
        CHECK(Has(Slurp(ela::FtpLog(logRoot).FileFor(k4.now)), "N25-4 Jam_Summary upload"), "15. FTP_Log: the N25-4 line");

        // golden mode: the device count of the second interval misses 10-11
        ela::ElaUploadContext kg = k4;
        kg.opt.keepGoldenBugs = true;
        const std::string gdir = root + "\\gold4";
        MkDirs(gdir);
        std::string gname;
        std::vector<ela::JamInterval> giv;
        int gev = 0;
        CHECK(ela::SaveJamSummaryFor30Days(kg, gdir, &gname, &giv, &gev) && giv.size() == 3 && gev == 7 &&
                  Slurp(gdir + "\\Jam_Summary.csv") ==
                      "Time,Summary,JamCount\n2026/10/10 09:00:00,2,1\n2026/10/11 07:00:00,2,2\n2026/11/01 23:59:59,1,1\n",
              "15. keepGoldenBugs: the second interval counts 2 devices (golden GetProInfoDevice :69 day steps)");

        // D-c: no record in the 30 days
        ela::ElaUploadContext ke = k4;
        ke.eventLogDir = root + "\\ev_empty";
        MkDirs(ke.eventLogDir);
        ke.now = Day(2026, 11, 3);
        const ela::N25Result re = ela::UploadSummaryCount(ke, u);
        CHECK(re.sourceMissing && !re.ok() && !re.retryable && g_fakeCalls == 1 &&
                  !Exists(logRoot + "\\SummaryCount\\2026\\11\\3\\Jam_Summary.csv") &&
                  !Exists(logRoot + "\\SummaryCount\\2026\\11\\3\\HadUpload.txt") && Has(re.detail, "D-c"),
              "15. D-c: no event-log record -> no Jam_Summary.csv, no connection (golden :95 reads an empty vector, G9)");

        // D-f gate
        g_countCalls = 0;
        ela::ElaUploadContext kc = k4;
        kc.factory = &CountingFactory;
        kc.now = Day(2026, 11, 4);
        bool closed = true;
        std::string whyOff;
        for (int i = 0; i < 5; ++i)
        {
            ela::N25UploadSettings t = u;
            if (i == 0) t.o10 = false;
            if (i == 1) t.custCode = "910";
            if (i == 2) t.enable = false;
            if (i == 3) t.host = " ";
            if (i == 4) t.path = "";
            const ela::N25Result g = ela::UploadSummaryCount(kc, t);
            if (g.gate.empty() || !g.saveFolder.empty()) closed = false;
            if (i == 2) whyOff = g.gate;
        }
        CHECK(closed && g_countCalls == 0 && !Exists(logRoot + "\\SummaryCount\\2026\\11\\4") && Has(whyOff, "bN25_4_EnableUpload"),
              "15. D-f gate (O10 / 851 / bN25_4_EnableUpload / host / path): no transport, nothing written");

        // a failed BackUp copy: no HadUpload.txt, retryable; the retry rebuilds the file (one header)
        ResetRemote();
        g_fakeCalls = 0;
        k4.now = Day(2026, 11, 5);
        g_s.sizeLie["/Summary/naslfs2/Sum4/HOSTX/BackUp/Jam_Summary_20261105_0000.csv"] = 1;
        const ela::N25Result rf = ela::UploadSummaryCount(k4, u);
        const std::string f5 = logRoot + "\\SummaryCount\\2026\\11\\5";
        CHECK(!rf.ok() && rf.mainOk && !rf.backupOk && rf.retryable && !Exists(f5 + "\\HadUpload.txt"),
              "15. BackUp SIZE mismatch: failed, retryable, no HadUpload.txt (golden: the BackUp result decides, :307-314)");
        g_s.sizeLie.clear();
        const ela::N25Result rr = ela::UploadSummaryCount(k4, u);
        CHECK(rr.ok() && Exists(f5 + "\\HadUpload.txt") && CountOf(Slurp(f5 + "\\Jam_Summary.csv"), "Time,Summary,JamCount") == 1 &&
                  g_fakeCalls == 2, "15. the retry: verified, Jam_Summary.csv rebuilt (golden appends a second header)");
    }

    // ---- 16: W22 N25-5 ----
    {
        const std::string evBytes = hdr + "2026/11/01,08:00:00,\"01 Input Arm\",JAM0101,1,30,0,\"\xA4\xA4\",,R1\r\n";
        Put(ev4 + "\\2026\\11\\EventLogTxt_20261101.csv", evBytes);
        ela::N25UploadSettings u5 = u;
        u5.path = "/Summary/naslfs2/Ev5/";
        ResetRemote();
        g_fakeCalls = 0;
        k4.now = Day(2026, 11, 2);
        const ela::N25Result r = ela::UploadEventLog(k4, u5);
        const std::string folder = logRoot + "\\UploadEventLog\\2026\\11\\2";
        CHECK(r.saveFolder == folder && r.jamFile == "EventLog.txt" && Slurp(folder + "\\EventLog.txt") == evBytes,
              "16. EventLog.txt = a byte copy of yesterday's EventLogTxt_20261101.csv (cp950 and CRLF kept: a copy, not a report)");
        const std::string seq = N25Seq("/Summary/naslfs2/Ev5", "HOSTX", "EventLog.txt", "EventLog_20261102_0000.txt");
        CHECK(g_s.Joined() == seq && g_fakeCalls == 1, "16. the N25-3 steps for EventLog.txt");
        if (g_s.Joined() != seq) printf("%s", g_s.Joined().c_str());
        CHECK(r.ok() && Slurp(folder + "\\HadUpload.txt") ==
                            "Upload path:/Summary/naslfs2/Ev5/HOSTX/EventLog.txt\n"
                            "Upload backup path:/Summary/naslfs2/Ev5/HOSTX/BackUp/EventLog_20261102_0000.txt\nHadUpload\n",
              "16. HadUpload.txt: both lines (the BackUp one names the BackUp folder) + the marker, after both verified");
        CHECK(g_s.files["/Summary/naslfs2/Ev5/HOSTX/EventLog.txt"] == (long long)evBytes.size(), "16. remote size = the source's bytes");
        CHECK(ela::UploadEventLog(k4, u5).alreadyUploaded && g_fakeCalls == 1, "16. the same day again: no transport");
        CHECK(Has(Slurp(ela::FtpLog(logRoot).FileFor(k4.now)), "N25-5 EventLog upload"), "16. FTP_Log: the N25-5 line");

        k4.now = Day(2026, 11, 4);                           // yesterday 11-03: no file
        const ela::N25Result m = ela::UploadEventLog(k4, u5);
        CHECK(m.sourceMissing && !m.retryable && g_fakeCalls == 1 && !Exists(logRoot + "\\UploadEventLog\\2026\\11\\4\\HadUpload.txt") &&
                  Has(m.detail, "EventLogTxt_20261103.csv") && Has(Slurp(ela::FtpLog(logRoot).FileFor(k4.now)), "File not exist N25-5"),
              "16. no event log of yesterday: nothing sent, no connection, FTP_Log \"File not exist N25-5\"");
        Put(ev4 + "\\2026\\11\\EventLogTxt_M-TEST_20261104.csv", hdr);
        k4.now = Day(2026, 11, 5);                           // yesterday 11-04: only an O15-named file
        const ela::N25Result o15 = ela::UploadEventLog(k4, u5);
        CHECK(o15.sourceMissing && g_fakeCalls == 1, "16. an O15 name (EventLogTxt_<ID>_yyyymmdd.csv) is not golden's plain name: nothing sent (asked, ledger W22)");
        g_countCalls = 0;
        ela::ElaUploadContext kc = k4;
        kc.factory = &CountingFactory;
        ela::N25UploadSettings off = u5;
        off.enable = false;
        const ela::N25Result g = ela::UploadEventLog(kc, off);
        CHECK(Has(g.gate, "bN25_5_EnableUpload") && g_countCalls == 0 && g.saveFolder.empty(), "16. D-f: bN25_5_EnableUpload off -> no transport");
        bool clean = true;
        for (int d = 2; d <= 7; ++d)
        {
            const std::string t = Slurp(ela::FtpLog(logRoot).FileFor(Day(2026, 11, d)));
            if (Has(t, pw) || Has(t, user)) clean = false;
        }
        CHECK(clean, "16. no FTP_Log file of 11-02 .. 11-07 holds the account");
    }

    // ---- 17: W22 through the Hub and the schedule bodies ----
    {
        _putenv(("W906_EVENTLOG_ROOT=" + ev4).c_str());      // the Hub's page folder (base_) and QueryRequest() default
        Put(ev4 + "\\2026\\11\\EventLogTxt_20261105.csv", hdr + "2026/11/05,10:00:00,\"24 Motor\",WAR2401,1,20,0,\"w\",,R1\r\n");
        Put(ev4 + "\\2026\\11\\EventLogTxt_20261106.csv", hdr + "2026/11/06,10:00:00,\"24 Motor\",WAR2401,1,20,0,\"w\",,R1\r\n");
        const std::string cfg = root + "\\config17.ini";
        const std::string gen = root + "\\Gerneral17.ini";
        Put(gen, "[System]\r\nCUSTOMER_CODE=851\r\n[Version]\r\nMachine ID=M-TEST\r\n");
        Put(cfg, "[Event Log]\r\nEnableAutoSaveEventLog=1\r\n[ChipMos Function]\r\nsN25_2_FTPHost=ftp.test\r\n"
                 "sN25_2_FTPUserName=" + user + "\r\nsN25_2_FTPPassword=" + pw + "\r\n"
                 "sN25_4_UploadPath=/Summary/naslfs2/Sum4/\r\nsN25_5_UploadPath=/Summary/naslfs2/Ev5/\r\n");
        ela::Options o;
        o.jamIniPath = root + "\\JAM0000.dat";
        ela::HubPaths hp;
        hp.configIni = cfg;
        hp.generalIni = gen;
        {
            g_countCalls = 0;
            ela::Hub h(o, hp);
            h.SetFtpFactory(&CountingFactory);
            h.Post(ela::EL_UPDATE_PARAMETER);
            h.RunOnce();
            h.Post(ela::EL_UPLOAD_EVENTLOG);
            h.Post(ela::EL_UPLOAD_SUMMARY);
            h.RunOnce();
            h.RunOnce();
            const std::string js = h.SnapshotJson(0);
            CHECK(Has(js, "EL_UPLOAD_SUMMARY: skipped, N25-4 bN25_4_EnableUpload is off") &&
                      Has(js, "EL_UPLOAD_EVENTLOG: skipped, N25-5 bN25_5_EnableUpload is off") && g_countCalls == 0 &&
                      !Has(js, "not ported") && !Has(js, pw),
                  "17. Hub: both switches off -> skipped by the D-f gate, no transport, nothing HOLD any more, no password");
        }
        ::WritePrivateProfileStringA("ChipMos Function", "bN25_4_EnableUpload", "1", cfg.c_str());
        ::WritePrivateProfileStringA("ChipMos Function", "bN25_5_EnableUpload", "1", cfg.c_str());
        {
            ResetRemote();
            g_fakeCalls = 0;
            g_fakeNow = Day(2026, 11, 6);
            ela::Hub h(o, hp);
            h.SetFtpFactory(&FakeFactory);
            FakeClock fc;
            h.SetClock(&fc);
            h.Post(ela::EL_UPDATE_PARAMETER);
            h.RunOnce();
            h.Post(ela::EL_UPLOAD_EVENTLOG);                 // posted first, runs second (golden queue order)
            h.Post(ela::EL_UPLOAD_SUMMARY);
            h.RunOnce();
            h.RunOnce();
            const std::string js = h.SnapshotJson(0);
            const std::vector<ela::JobRecord> jb = h.Jobs();
            CHECK(g_fakeCalls == 2 && Has(js, "EL_UPLOAD_SUMMARY: 3 interval(s), ") && Has(js, "6 JAM, uploaded and verified") &&
                      Has(js, "EL_UPLOAD_EVENTLOG: EventLog.txt, uploaded and verified") && !Has(js, pw),
                  "17. Hub: switches on -> both run on the installed transport (fake clock 2026-11-06), verified");
            CHECK(jb.size() >= 2 && jb[jb.size() - 2].cmd == "EL_UPLOAD_SUMMARY" && jb[jb.size() - 2].ran &&
                      jb.back().cmd == "EL_UPLOAD_EVENTLOG" && jb.back().ran,
                  "17. Hub: SUMMARY before EVENTLOG whatever the posting order (Rev891 Analyzer.cpp:161-183), both job records ran");
            CHECK(Exists(logRoot + "\\SummaryCount\\2026\\11\\6\\HadUpload.txt") &&
                      Exists(logRoot + "\\UploadEventLog\\2026\\11\\6\\HadUpload.txt"),
                  "17. Hub: both HadUpload.txt in the sandbox");
        }
        {
            ResetRemote();
            g_fakeCalls = 0;
            ela::Hub h(o, hp);
            h.SetFtpFactory(&FakeFactory);
            h.Post(ela::EL_UPDATE_PARAMETER);
            h.RunOnce();
            ela::ScheduleConfig sc;
            sc.n25_4 = true;
            sc.n25_5 = true;
            ela::JobContext ctx;
            ctx.job = ela::JOB_N25_4;
            ctx.slot = ela::LocalSecFromCivil(2026, 11, 7, 0, 0, 0);
            ctx.slotDateTime = ela::LocalSecToDateTime(ctx.slot);
            ctx.attempt = 1;
            ctx.catchUp = false;
            ctx.manual = false;
            ctx.cfg = &sc;
            ctx.slotFlags = 0;
            const ela::JobStatus a = ela::RunN25_4OnHub(&h, o, ctx);
            ctx.job = ela::JOB_N25_5;
            const ela::JobStatus b = ela::RunN25_5OnHub(&h, o, ctx);
            CHECK(a == ela::JOB_VERIFIED && b == ela::JOB_VERIFIED && g_fakeCalls == 2 &&
                      Exists(logRoot + "\\SummaryCount\\2026\\11\\7\\HadUpload.txt") &&
                      Exists(logRoot + "\\UploadEventLog\\2026\\11\\7\\HadUpload.txt"),
                  "17. schedule bodies RunN25_4OnHub / RunN25_5OnHub at the slot 2026-11-07 00:00: verified");
            CHECK(ela::RunN25_4OnHub(&h, o, ctx) == ela::JOB_NOTHING_TO_DO && ela::RunN25_5OnHub(&h, o, ctx) == ela::JOB_NOTHING_TO_DO &&
                      g_fakeCalls == 2, "17. the same slot again: HadUpload.txt -> nothing to do, no transport");
            ctx.slot += 86400;                               // 11-08: no event log of 11-07
            ctx.slotDateTime = ela::LocalSecToDateTime(ctx.slot);
            CHECK(ela::RunN25_5OnHub(&h, o, ctx) == ela::JOB_NOTHING_TO_DO && g_fakeCalls == 2 && Has(ctx.detail, "EventLogTxt_20261107.csv"),
                  "17. no file of yesterday: nothing to do (the slot is done, no retry)");
            sc.n25_4 = false;
            CHECK(ela::RunN25_4OnHub(&h, o, ctx) == ela::JOB_FAILED && Has(ctx.detail, "bN25_4_EnableUpload") && g_fakeCalls == 2 &&
                      ela::RunN25_5OnHub(0, o, ctx) == ela::JOB_NO_BODY,
                  "17. switch off: not connected (failed, not retried); no hub: no body");
        }
        _putenv(("W906_EVENTLOG_ROOT=" + evRoot).c_str());
    }

    CHECK(::GetModuleHandleA("wininet.dll") == NULL, "0. wininet.dll still not loaded at the end");
    printf("ELA_Ftp: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(root);
    else printf("  (sandbox kept for inspection: %s)\n", root.c_str());
    return g_fail ? 1 : 0;
}
