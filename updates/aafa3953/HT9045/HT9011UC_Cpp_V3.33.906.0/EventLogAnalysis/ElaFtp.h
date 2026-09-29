// ===========================================================================
//  EventLogAnalysis/ElaFtp.h -- the ELA upload transport (plan R4), no UI.
//  AI(W906-ELA-R4) 20260927 (St02-E).  Design: D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-wininet-elaftp.md;
//  plan D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md §3 / §4 R4; ledger
//  docs/ELA_PORT_LEDGER.md "R4".
//
//  Golden: EventlogAnalyzer Rev891 TfFTP (TfFTP.cpp, FastNet TNMFTP): Connect :53-81 (TimeOut 5000, no Port / Passive set),
//  ChangeDir :144-171 (CWD, on failure MKD, one level), ChangeDirectories :83-142 (multi-level, NLST + MKD + CWD), Upload
//  :392-423 (ChangeDir then STOR of the full remote path), Close :490-512.  Golden never verifies an upload ("no exception"
//  = success); here an upload counts only when STOR, SIZE and a fresh LIST all agree (plan §3 item 1, UploadAndVerify).
//
//  Steven 20260927: the transport is Windows WinINet behind IElaFtp.  Three implementations:
//    NewNullFtp()      -- never opens anything; Connect fails with kFtpErrDisabled.  The Hub's DEFAULT, so a Hub built in a
//                         ctest (or anywhere but W906_ElaStart) cannot reach the network.
//    NewLogOnlyFtp()   -- opens nothing, writes what it would send to FTP_Log and pretends the server agreed (so the whole
//                         N25-3 path runs); callers see Kind()=="logonly" and never write HadUpload.txt for it.  No build
//                         installs it since ★W36 = C (below); the ELA_Ftp ctest still runs the N25-3 path on it.
//    NewWinInetFtp()   -- ElaFtpWinInet.cpp, the only file that includes <wininet.h>.  Installed only by W906_ElaStart
//                         (ElaService.cpp), in both builds (★W36 = C, below).
//  Whether a job connects at all is decided by the configuration (#22 D-f, golden): N25-3 bN25_3_EnableULJamLog + the N25-2
//  host / account (ElaChipMos.h N25_3GateReason); there is no extra environment switch.
//
//  Credentials: FtpEndpoint carries the password in memory only.  Every status text that leaves a transport, every FTP_Log
//  line and every history line goes through MaskSecrets (user and password -> "***").  Nothing here logs a password.
// ===========================================================================
#ifndef HT9045_ELA_ELAFTP_H
#define HT9045_ELA_ELAFTP_H

#include "WebBridge/Sync.h"

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
//  ★W36 = C (Steven 20260928): the simulation build uploads exactly like the shipping build.  W906_ElaStart installs the
//  WinINet transport in both; whether a job connects is decided only by the configuration (D-f, above).  Steven's team
//  checks it against an FTP server of their own.  AI(W906-ELA-W36) 20260928 (St02-E helper).
//  History: R4 (20260927) shipped the provisional ★W36 A here as a one-line switch, `#define W906_ELA_SIM_FTP_LOG_ONLY 1`,
//  which made W906_ELA_FTP_WININET 0 when W906_NO_SOFT_SIMULTE was not defined: the simulation build installed
//  NewLogOnlyFtp and its wb_serve never linked WinINet.  Both macros are gone.  The golden precedent quoted then is kept
//  for the SCHEDULE, not the transport: the SIM Handler never sends the N25 trigger (906_0625_Steven main.cpp:21425
//  #ifndef SOFT_SIMULTE), so ElaSchedule still starts no AUTOMATIC N25-3 / N25-4 / N25-5 in the simulation build (a
//  manual run, POST /api/ela/job, now really uploads).  Golden's SIM builds point the FTP classes at 127.0.0.1 (Rev891
//  TfFTP.cpp:15-23, uChipMosZHUBEI_Func.cpp:206-210) -- those SIM account literals are still NOT carried over.
// ---------------------------------------------------------------------------

namespace ela {

// ---------------------------------------------------------------------------
//  待 Steven (passive): golden TfFTP never sets Passive (Rev891: `grep -i passive *.cpp *.h *.dfm` = 0 hits, 20260927), so the
//  analyzer ran in the FastNet TNMFTP component's default mode.  That default cannot be read on this machine (no NMFtp.hpp);
//  the task's provisional reading is ACTIVE.  ONE-LINE SWITCH: false = active (PORT), true = passive (PASV).  Jimmy's
//  KYECFTP/MiniFtpEngine.h:52-63 note infers a component default of true; the Handler's own N10 path sets it from
//  bN10FtpPassive (906_0625_Steven HS_Function.cpp:2134), which the analyzer ignores.
// ---------------------------------------------------------------------------
const bool kElaFtpPassive = false;

const int kElaFtpPort = 21;                       // golden TfFTP sets no Port: the TNMFTP default, 21
const unsigned kElaFtpConnectMs = 5000;           // golden TfFTP.cpp:56 TimeOut=5000
const unsigned kElaFtpTransferMs = 30000;         // plan §3 item 6 (the Handler's N10 path uses 30000, HS_Function.cpp:2134)

enum FtpTransportKind { kFtpTransportNull = 0, kFtpTransportLogOnly = 1, kFtpTransportWinInet = 2 };
// what W906_ElaStart installs in this build: WinINet in both builds since ★W36 = C (AI(W906-ELA-W36) 20260928, St02-E
// helper; the ctest checks it).  kFtpTransportLogOnly stays the kind of NewLogOnlyFtp, which no build installs now.
inline FtpTransportKind BuildFtpTransport() { return kFtpTransportWinInet; }

enum FtpErr
{
    kFtpOk = 0,
    kFtpErrDisabled,          // null transport
    kFtpErrNotConnected,      // a command before Connect / after Close
    kFtpErrConnect,           // name / route / refused
    kFtpErrLogin,             // 530 / ERROR_INTERNET_LOGIN_FAILURE
    kFtpErrTimeout,           // the watchdog fired (retryable)
    kFtpErrAborted,           // Abort() from another thread (Hub::Stop)
    kFtpErrCommand,           // the server said no (4xx / 5xx), ftpCode holds the code
    kFtpErrSizeUnsupported,   // SIZE answered 500 / 502 / 504
    kFtpErrLocalFile,         // the local file is missing / unreadable
    kFtpErrOther
};

struct FtpEndpoint
{
    std::string host, user, password;   // password: memory only, never logged
    int port;
    bool passive;
    unsigned connectMs, transferMs;
    FtpEndpoint();                      // port kElaFtpPort, passive kElaFtpPassive, kElaFtpConnectMs / kElaFtpTransferMs
};

struct FtpStatus
{
    bool ok;
    int err;                  // FtpErr
    unsigned long sysErr;     // GetLastError() of the failing call (WinINet 12xxx), 0 when none
    int ftpCode;              // last server reply code, 0 when none
    std::string text;         // masked
    FtpStatus();
    static FtpStatus Ok(int code = 0, const std::string& text = std::string());
    static FtpStatus Fail(int err, int code, const std::string& text, unsigned long sysErr = 0);
};

struct FtpEntry
{
    std::string name;
    bool isDir;
    long long size;           // -1 = the listing gave no size
};

class IElaFtp
{
public:
    virtual ~IElaFtp() {}
    virtual const char* Kind() const = 0;                                   // "null" / "logonly" / "wininet" / test fakes
    virtual FtpStatus Connect(const FtpEndpoint& ep) = 0;                   // open + login
    virtual void Close() = 0;                                               // idempotent
    virtual void Abort() = 0;                                               // ANY thread: cancel what blocks, then fail
    virtual FtpStatus ChangeDir(const std::string& dir) = 0;               // CWD
    virtual FtpStatus MakeDir(const std::string& dir) = 0;                 // MKD
    virtual FtpStatus Put(const std::string& localPath, const std::string& remotePath) = 0;   // binary STOR
    virtual FtpStatus Size(const std::string& remotePath, long long* n) = 0;                  // SIZE (TYPE I first)
    virtual FtpStatus List(const std::string& dir, std::vector<FtpEntry>* out) = 0;           // always a fresh listing
};

typedef IElaFtp* (*FtpFactory)();
IElaFtp* NewNullFtp();
IElaFtp* NewLogOnlyFtp();
IElaFtp* NewWinInetFtp();     // ElaFtpWinInet.cpp -- referencing it links WinINet (only W906_ElaStart and the link check do)

// ---------------------------------------------------------------------------
//  Cancelling a transfer from Hub::Stop.  The job registers the live transport; Abort() cancels it and marks the slot so a
//  job that has not connected yet does not start.  One per Hub.
// ---------------------------------------------------------------------------
class FtpAbortSlot
{
public:
    FtpAbortSlot() : active_(0), stopping_(false) {}
    void Set(IElaFtp* f);          // f = the transport now in use, 0 = none; aborts f at once if Abort() already ran
    void Abort();
    void Reset();                  // Hub::Start: a new worker may connect again
    bool Stopping() const;
private:
    mutable webbridge::WbMutex mu_;
    IElaFtp* active_;
    bool stopping_;
    FtpAbortSlot(const FtpAbortSlot&);
    FtpAbortSlot& operator=(const FtpAbortSlot&);
};

// ---------------------------------------------------------------------------
//  Pure helpers (tested in ELA_Ftp)
// ---------------------------------------------------------------------------
int FtpReplyCode(const std::string& reply);                          // code of the last "nnn " line, 0 = none
bool ParseSizeReply(const std::string& reply, long long* n);        // "213 <n>" (last line) -> n
std::string MaskSecrets(const std::string& text, const std::string& user, const std::string& password);
std::string FtpPathCombin(const std::string& path, const std::string& file);   // golden FileInfo::PathCombin (FileInfo.cpp:260-282)
std::string FtpTrimTrailingSlashes(const std::string& s);            // golden RemoveAllTrailingSlashes (TfFTP.cpp:514-522)
std::string FtpParentDir(const std::string& path);                   // "/a/b/c" -> "/a/b", "/a" -> "/"
std::string FtpBaseName(const std::string& path);                    // "/a/b/c.txt" -> "c.txt"
// golden GetNameAndExtension (FileInfo.cpp:144-153) + the BackUp name of N25_UploadJamDataToFTP (:198-200):
//   "<name>_yyyymmdd_hhnn.<ext>" at 'now' (TDateTime)
std::string BackupFileName(const std::string& fileName, double now);
long long FtpLocalFileSize(const std::string& path);                 // -1 = missing
std::string ElaHostName();                                           // golden gethostname (Analyzer.cpp:429-432)
double ElaNow();                                                     // local time as a BCB TDateTime
std::string ElaLogRoot();                                            // W906_HT9045LOG_ROOT seam, else golden "D:\\HT9045_Log"

// ---------------------------------------------------------------------------
//  Back-off (plan §3 items 4-5).  Pure; the clock is the caller's (TDateTime days).
// ---------------------------------------------------------------------------
unsigned Fnv1a32(const std::string& s);                              // "" -> 0x811C9DC5
unsigned StaggerOffsetSeconds(const std::string& machineId, unsigned windowSec);   // FNV-1a-32(Machine ID) mod W
const int kMaxUploadRetries = 6;                                     // 1 / 2 / 4 / 8 / 16 / 30 min, then give up
// n-th retry (n >= 1): min(60 * 2^(n-1), 1800) s x (1 + 0.2 u), u in [-1, 1] from FNV-1a-32(machineId + "#" + n)
double BackoffSeconds(int n, const std::string& machineId);
struct UploadRetry
{
    int failures;             // failed attempts since the last success
    double nextTry;           // TDateTime; 0 = nothing pending
    bool gaveUp;
    UploadRetry() : failures(0), nextTry(0.0), gaveUp(false) {}
};
// a failed attempt at 'now': the next try is now + BackoffSeconds(failures), or give up after kMaxUploadRetries retries or
// when that time is not before 'nextSlot' (the next scheduled time, TDateTime; 0 = none)
void OnUploadFailed(UploadRetry* r, double now, const std::string& machineId, double nextSlot);
void OnUploadSucceeded(UploadRetry* r);
bool RetryDue(const UploadRetry& r, double now);

// ---------------------------------------------------------------------------
//  FTP_Log (golden TMyStringList slFTPLog, Analyzer.cpp:220-224: "D:\\HT9045_Log\\UploadFile", "FTP_Log",
//  "Date, Time, Action, S2, S3, S4, S5, S6", TByDay, dated folder) -> <root>\UploadFile\<yyyy>\<mm>\FTP_Log_<yyyymmdd>.csv,
//  header when the file is new, CRLF, one line per call (written at once; golden buffers MaxLineCount=1).  Golden writes
//  every MyDBIProcess line there (Common.cpp:39); here the upload code writes its own lines.  Every line is masked.
// ---------------------------------------------------------------------------
class FtpLog
{
public:
    explicit FtpLog(const std::string& logRoot);
    void SetSecrets(const std::string& user, const std::string& password);
    // golden AddTextWithDateTime(CommaText of {action, s1..s6}) (MyStringList.cpp:175-193 + Common.cpp:23-48)
    void Add(double now, const std::string& action, const std::string& s1 = std::string(),
             const std::string& s2 = std::string(), const std::string& s3 = std::string(),
             const std::string& s4 = std::string(), const std::string& s5 = std::string(),
             const std::string& s6 = std::string());
    // golden AddTextForUpload (MyStringList.cpp:141-156)
    void AddUpload(double now, const std::string& srcPath, const std::string& dstPath, const std::string& srcFile,
                   const std::string& dstFile);
    // a ready CommaText line (golden AddTextWithDateTime(SL->CommaText), Common.cpp:39)
    void AddCommaText(double now, const std::string& commaText) { Write(now, commaText); }
    std::string FileFor(double now) const;
    int Lines() const { return lines_; }
private:
    void Write(double now, const std::string& body);
    std::string root_, user_, password_;
    int lines_;
};

// golden MyDBIProcess's FTP_Log half (Common.cpp:39): ElaFileUtil's MyDBIProcess calls its hook with the CommaText line;
// W906_ElaStart installs this one (ElaFileUtil.h SetDbiProcessHook).  <ElaLogRoot()>, now = ElaNow().
void FtpLogDbiLine(const std::string& commaText);

// ---------------------------------------------------------------------------
//  Directory walk and verified upload
// ---------------------------------------------------------------------------
// golden ChangeDir (TfFTP.cpp:144-171), one level: CWD; on failure MKD the same path; then CWD again (golden's Upload
// :402 CWDs it again before STOR).  true = the last CWD worked.
bool EnsureDirOneLevel(IElaFtp* f, const std::string& path, FtpLog* log, double now);
// golden ChangeDirectories (TfFTP.cpp:83-142), every level from the root: CWD <level>; on failure MKD <level> + CWD <level>.
// Deviation (W18 B, ledger): golden :120 MKDs the PARENT (sCurr) instead of the new level and lists with NLST first.
bool EnsureDirPath(IElaFtp* f, const std::string& path, FtpLog* log, double now);

struct UploadVerify
{
    bool ok;
    bool nameOnly;            // SIZE unsupported and the listing had no size: only the name was checked
    long long localSize, remoteSize;
    std::string step;         // "local" / "cwd" / "stor" / "size" / "list" / "" (ok)
    FtpStatus st;
    UploadVerify() : ok(false), nameOnly(false), localSize(-1), remoteSize(-1) {}
};
// golden Upload (TfFTP.cpp:392-423) + plan §3 item 1: CWD remoteDir (EnsureDirOneLevel), STOR localPath -> remoteDir/remoteName,
// SIZE == local size (500/502/504 -> the LIST size; a non-empty local file listed as 0 fails), and remoteName in a fresh
// LIST of remoteDir.  Every step is one FTP_Log line.
UploadVerify UploadAndVerify(IElaFtp* f, const std::string& localPath, const std::string& remoteDir,
                             const std::string& remoteName, FtpLog* log, double now);

// one transfer at a time in the process (the Hub worker already runs one job at a time; this also covers direct callers)
class FtpSessionLock
{
public:
    FtpSessionLock();
    ~FtpSessionLock();
private:
    FtpSessionLock(const FtpSessionLock&);
    FtpSessionLock& operator=(const FtpSessionLock&);
};

}  // namespace ela

#endif
