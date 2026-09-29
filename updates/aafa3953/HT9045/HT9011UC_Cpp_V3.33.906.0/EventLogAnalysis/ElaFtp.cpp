// ===========================================================================
//  EventLogAnalysis/ElaFtp.cpp -- see ElaFtp.h.  AI(W906-ELA-R4) 20260927 (St02-E).
//  No <wininet.h> here: the WinINet transport is ElaFtpWinInet.cpp, so a binary that never names NewWinInetFtp never
//  links it (the ELA_Ftp ctest checks wininet.dll is not even loaded).
// ===========================================================================
#include "EventLogAnalysis/ElaFtp.h"
#include "EventLogAnalysis/ElaCore.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

namespace ela {

FtpEndpoint::FtpEndpoint()
    : port(kElaFtpPort), passive(kElaFtpPassive), connectMs(kElaFtpConnectMs), transferMs(kElaFtpTransferMs)
{
}

FtpStatus::FtpStatus() : ok(false), err(kFtpErrOther), sysErr(0), ftpCode(0) {}

FtpStatus FtpStatus::Ok(int code, const std::string& text)
{
    FtpStatus s;
    s.ok = true;
    s.err = kFtpOk;
    s.ftpCode = code;
    s.text = text;
    return s;
}

FtpStatus FtpStatus::Fail(int err, int code, const std::string& text, unsigned long sysErr)
{
    FtpStatus s;
    s.ok = false;
    s.err = err;
    s.ftpCode = code;
    s.text = text;
    s.sysErr = sysErr;
    return s;
}

// ---------------------------------------------------------------------------
//  FtpAbortSlot
// ---------------------------------------------------------------------------
void FtpAbortSlot::Set(IElaFtp* f)
{
    webbridge::WbGuard g(mu_);
    active_ = f;
    if (f && stopping_) f->Abort();
}

void FtpAbortSlot::Abort()
{
    webbridge::WbGuard g(mu_);
    stopping_ = true;
    if (active_) active_->Abort();
}

void FtpAbortSlot::Reset()
{
    webbridge::WbGuard g(mu_);
    stopping_ = false;
}

bool FtpAbortSlot::Stopping() const
{
    webbridge::WbGuard g(mu_);
    return stopping_;
}

// ---------------------------------------------------------------------------
//  Pure helpers
// ---------------------------------------------------------------------------
static std::vector<std::string> ReplyLines(const std::string& reply)
{
    std::vector<std::string> v;
    std::string cur;
    for (size_t i = 0; i < reply.size(); ++i)
    {
        const char c = reply[i];
        if (c == '\r' || c == '\n')
        {
            if (!cur.empty()) v.push_back(cur);
            cur.clear();
        }
        else cur += c;
    }
    if (!cur.empty()) v.push_back(cur);
    return v;
}

static bool IsFinalReplyLine(const std::string& l)
{
    return l.size() >= 4 && l[0] >= '0' && l[0] <= '9' && l[1] >= '0' && l[1] <= '9' && l[2] >= '0' && l[2] <= '9' &&
           l[3] == ' ';
}

int FtpReplyCode(const std::string& reply)
{
    const std::vector<std::string> v = ReplyLines(reply);
    for (size_t i = v.size(); i-- > 0;)
        if (IsFinalReplyLine(v[i])) return std::atoi(v[i].substr(0, 3).c_str());
    return 0;
}

bool ParseSizeReply(const std::string& reply, long long* n)
{
    const std::vector<std::string> v = ReplyLines(reply);
    for (size_t i = v.size(); i-- > 0;)
    {
        if (!IsFinalReplyLine(v[i])) continue;
        if (v[i].compare(0, 4, "213 ") != 0) return false;
        size_t p = 4;
        while (p < v[i].size() && v[i][p] == ' ') ++p;
        if (p >= v[i].size() || v[i][p] < '0' || v[i][p] > '9') return false;
        long long x = 0;
        for (; p < v[i].size() && v[i][p] >= '0' && v[i][p] <= '9'; ++p) x = x * 10 + (v[i][p] - '0');
        if (n) *n = x;
        return true;
    }
    return false;
}

static void ReplaceAll(std::string* s, const std::string& what, const std::string& with)
{
    if (what.empty()) return;
    size_t p = 0;
    while ((p = s->find(what, p)) != std::string::npos)
    {
        s->replace(p, what.size(), with);
        p += with.size();
    }
}

std::string MaskSecrets(const std::string& text, const std::string& user, const std::string& password)
{
    std::string s = text;
    ReplaceAll(&s, password, "***");   // the password first: a user name inside it must not leave a piece behind
    ReplaceAll(&s, user, "***");
    return s;
}

std::string FtpPathCombin(const std::string& path, const std::string& file)
{
    // golden FileInfo::PathCombin (FileInfo.cpp:260-282): a path holding '/' anywhere is an FTP path
    std::string c = path;
    if (!c.empty())
    {
        if (c[c.size() - 1] == '/' || c.find('/') != std::string::npos)
        {
            if (c[c.size() - 1] != '/') c += "/";
        }
        else if (c[c.size() - 1] != '\\')
        {
            c += "\\";
        }
    }
    return c + file;
}

std::string FtpTrimTrailingSlashes(const std::string& s)
{
    size_t n = s.size();
    while (n > 0 && s[n - 1] == '/') --n;
    return s.substr(0, n);
}

std::string FtpParentDir(const std::string& path)
{
    const std::string p = FtpTrimTrailingSlashes(path);
    const size_t k = p.rfind('/');
    if (k == std::string::npos) return std::string();
    if (k == 0) return "/";
    return p.substr(0, k);
}

std::string FtpBaseName(const std::string& path)
{
    const size_t k = path.rfind('/');
    return k == std::string::npos ? path : path.substr(k + 1);
}

std::string BackupFileName(const std::string& fileName, double now)
{
    // golden GetNameAndExtension (FileInfo.cpp:144-153): split at the LAST '.'; no '.' -> both empty (golden then builds
    // "_yyyymmdd_hhnn." -- kept; every caller passes "Jam Rate.txt")
    std::string name, ext;
    const size_t dot = fileName.rfind('.');
    if (dot != std::string::npos && dot > 0)
    {
        name = fileName.substr(0, dot);
        ext = fileName.substr(dot + 1);
    }
    int y, m, d, h, n, s, ms;
    DecodeDateTime(now, &y, &m, &d, &h, &n, &s, &ms);
    char b[48];
    std::snprintf(b, sizeof(b), "_%04d%02d%02d_%02d%02d.", y, m, d, h, n);   // :200
    return name + b + ext;
}

long long FtpLocalFileSize(const std::string& path)
{
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (!::GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &a)) return -1;
    if (a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return -1;
    return ((long long)a.nFileSizeHigh << 32) | (long long)a.nFileSizeLow;
}

std::string ElaHostName()
{
    // golden Analyzer.cpp:426-434 gethostname (WSAStartup first).  GetComputerNameExA(ComputerNameDnsHostname) is the
    // same name Winsock returns, without pulling ws2_32 into ht9045_ela.
    char b[256];
    DWORD n = sizeof(b);
    if (::GetComputerNameExA(ComputerNameDnsHostname, b, &n) && n > 0) return std::string(b, n);
    n = sizeof(b);
    if (::GetComputerNameA(b, &n) && n > 0) return std::string(b, n);
    return std::string();
}

double ElaNow()
{
    SYSTEMTIME t;
    ::GetLocalTime(&t);
    return EncodeDate(t.wYear, t.wMonth, t.wDay) + EncodeTime(t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
}

std::string ElaLogRoot()
{
    // the Handler's seam (common.cpp:240 / :425 as9045LogPath): unset or empty = the golden literal
    const char* e = std::getenv("W906_HT9045LOG_ROOT");
    return (e != 0 && *e != 0) ? std::string(e) : std::string("D:\\HT9045_Log");
}

// ---------------------------------------------------------------------------
//  Back-off
// ---------------------------------------------------------------------------
unsigned Fnv1a32(const std::string& s)
{
    unsigned h = 2166136261u;
    for (size_t i = 0; i < s.size(); ++i)
    {
        h ^= (unsigned char)s[i];
        h *= 16777619u;
    }
    return h;
}

unsigned StaggerOffsetSeconds(const std::string& machineId, unsigned windowSec)
{
    return windowSec ? Fnv1a32(machineId) % windowSec : 0u;
}

double BackoffSeconds(int n, const std::string& machineId)
{
    if (n < 1) n = 1;
    double base = 60.0;
    for (int i = 1; i < n && base < 1800.0; ++i) base *= 2.0;
    if (base > 1800.0) base = 1800.0;
    char b[16];
    std::snprintf(b, sizeof(b), "#%d", n);
    const double u = (double)(Fnv1a32(machineId + b) % 20001u) / 10000.0 - 1.0;   // [-1, 1]
    return base * (1.0 + 0.2 * u);
}

void OnUploadFailed(UploadRetry* r, double now, const std::string& machineId, double nextSlot)
{
    ++r->failures;
    if (r->failures > kMaxUploadRetries)
    {
        r->gaveUp = true;
        r->nextTry = 0.0;
        return;
    }
    const double t = now + BackoffSeconds(r->failures, machineId) / 86400.0;
    if (nextSlot > 0.0 && t >= nextSlot)
    {
        r->gaveUp = true;      // the next scheduled run takes over (plan §3 item 5)
        r->nextTry = 0.0;
        return;
    }
    r->gaveUp = false;
    r->nextTry = t;
}

void OnUploadSucceeded(UploadRetry* r)
{
    *r = UploadRetry();
}

bool RetryDue(const UploadRetry& r, double now)
{
    return !r.gaveUp && r.nextTry > 0.0 && now >= r.nextTry;
}

// ---------------------------------------------------------------------------
//  FTP_Log
// ---------------------------------------------------------------------------
static void MakeDirs(const std::string& dir)
{
    // golden MyForceDirectories: every level, errors ignored (a later open fails instead)
    for (size_t i = 0; i < dir.size(); ++i)
        if ((dir[i] == '\\' || dir[i] == '/') && i > 0 && dir[i - 1] != ':')
            ::CreateDirectoryA(dir.substr(0, i).c_str(), 0);
    ::CreateDirectoryA(dir.c_str(), 0);
}

FtpLog::FtpLog(const std::string& logRoot) : root_(logRoot), lines_(0) {}

void FtpLog::SetSecrets(const std::string& user, const std::string& password)
{
    user_ = user;
    password_ = password;
}

std::string FtpLog::FileFor(double now) const
{
    int y, m, d, h, n, s, ms;
    DecodeDateTime(now, &y, &m, &d, &h, &n, &s, &ms);
    char b[64];
    std::snprintf(b, sizeof(b), "\\UploadFile\\%04d\\%02d\\FTP_Log_%04d%02d%02d.csv", y, m, y, m, d);   // MyStringList.cpp:526 / :642
    return root_ + b;
}

void FtpLog::Write(double now, const std::string& body)
{
    const std::string file = FileFor(now);
    MakeDirs(file.substr(0, file.rfind('\\')));
    int y, m, d, h, n, s, ms;
    DecodeDateTime(now, &y, &m, &d, &h, &n, &s, &ms);
    char stamp[48];
    std::snprintf(stamp, sizeof(stamp), "%04d-%02d-%02d, %02d:%02d:%02d.%03d, ", y, m, d, h, n, s, ms);
    std::string text;
    if (::GetFileAttributesA(file.c_str()) == INVALID_FILE_ATTRIBUTES)
        text = "Date, Time, Action, S2, S3, S4, S5, S6\r\n";                   // golden FirstRow (Analyzer.cpp:222, :297-300)
    text += stamp + MaskSecrets(body, user_, password_) + "\r\n";
    // golden MySaveToFileShareMode (MyStringList.cpp:306-322): OPEN_ALWAYS, FILE_SHARE_READ, append
    HANDLE hf = ::CreateFileA(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) return;
    ::SetFilePointer(hf, 0, NULL, FILE_END);
    DWORD w = 0;
    ::WriteFile(hf, text.data(), (DWORD)text.size(), &w, NULL);
    ::CloseHandle(hf);
    ++lines_;
}

void FtpLog::Add(double now, const std::string& action, const std::string& s1, const std::string& s2,
                 const std::string& s3, const std::string& s4, const std::string& s5, const std::string& s6)
{
    Row r;
    r.push_back(action);
    r.push_back(s1);
    r.push_back(s2);
    r.push_back(s3);
    r.push_back(s4);
    r.push_back(s5);
    r.push_back(s6);
    Write(now, BcbGetCommaText(r));   // golden MyDBIProcess: SL->CommaText of {asTable, S1..S6} (Common.cpp:29-39)
}

void FtpLogDbiLine(const std::string& commaText)
{
    FtpLog(ElaLogRoot()).AddCommaText(ElaNow(), commaText);
}

void FtpLog::AddUpload(double now, const std::string& srcPath, const std::string& dstPath, const std::string& srcFile,
                       const std::string& dstFile)
{
    Write(now, "Upload, , " + srcPath + ", " + dstPath + ", " + srcFile + ", " + dstFile);   // MyStringList.cpp:145
}

// ---------------------------------------------------------------------------
//  Directory walk and verified upload
// ---------------------------------------------------------------------------
// MinGW.org msvcrt printf has no %lld: long long by hand
static std::string I64(long long v)
{
    if (v == 0) return "0";
    const bool neg = v < 0;
    unsigned long long u = neg ? 0ull - (unsigned long long)v : (unsigned long long)v;
    char b[24];
    int i = (int)sizeof(b);
    b[--i] = 0;
    while (u) { b[--i] = (char)('0' + (int)(u % 10)); u /= 10; }
    if (neg) b[--i] = '-';
    return std::string(&b[i]);
}

static std::string CodeText(const FtpStatus& st)
{
    char b[48];
    std::snprintf(b, sizeof(b), "err %d code %d sys %lu", st.err, st.ftpCode, st.sysErr);
    return std::string(b) + (st.text.empty() ? std::string() : " " + st.text);
}

static void LogCmd(FtpLog* log, double now, const char* what, const std::string& arg, const FtpStatus& st)
{
    if (!log) return;
    if (st.ok) log->Add(now, "FTP Success", std::string(what) + " successful", arg);
    else log->Add(now, "FTP Failure", std::string(what) + " failed", arg, CodeText(st));
}

bool EnsureDirOneLevel(IElaFtp* f, const std::string& path, FtpLog* log, double now)
{
    const std::string p = FtpTrimTrailingSlashes(path);   // golden ChangeDir :147
    if (p.empty()) return false;
    FtpStatus st = f->ChangeDir(p);
    LogCmd(log, now, "ChangeDir", p, st);
    if (st.ok) return true;
    st = f->MakeDir(p);                                    // golden :162
    LogCmd(log, now, "MakeDir", p, st);
    st = f->ChangeDir(p);
    LogCmd(log, now, "ChangeDir", p, st);
    return st.ok;
}

bool EnsureDirPath(IElaFtp* f, const std::string& path, FtpLog* log, double now)
{
    const std::string p = FtpTrimTrailingSlashes(path);
    const bool absolute = !p.empty() && p[0] == '/';
    std::vector<std::string> seg;
    std::string cur;
    for (size_t i = 0; i <= p.size(); ++i)
    {
        if (i == p.size() || p[i] == '/')
        {
            if (!cur.empty()) seg.push_back(cur);
            cur.clear();
        }
        else cur += p[i];
    }
    if (seg.empty())
    {
        const FtpStatus st = f->ChangeDir(absolute ? "/" : ".");
        LogCmd(log, now, "ChangeDir", absolute ? "/" : ".", st);
        return st.ok;
    }
    std::string at;
    for (size_t i = 0; i < seg.size(); ++i)
    {
        const std::string next = at.empty() ? (absolute ? "/" + seg[i] : seg[i]) : at + "/" + seg[i];
        FtpStatus st = f->ChangeDir(next);
        LogCmd(log, now, "ChangeDir", next, st);
        if (!st.ok)
        {
            st = f->MakeDir(next);           // W18 B: the new level (golden :120 MKDs sCurr, the parent)
            LogCmd(log, now, "MakeDir", next, st);
            st = f->ChangeDir(next);
            LogCmd(log, now, "ChangeDir", next, st);
            if (!st.ok) return false;
        }
        at = next;
    }
    return true;
}

UploadVerify UploadAndVerify(IElaFtp* f, const std::string& localPath, const std::string& remoteDir,
                             const std::string& remoteName, FtpLog* log, double now)
{
    UploadVerify v;
    v.localSize = FtpLocalFileSize(localPath);
    if (v.localSize < 0)
    {
        v.step = "local";
        v.st = FtpStatus::Fail(kFtpErrLocalFile, 0, "local file missing: " + localPath);
        if (log) log->Add(now, "Upload", "Can not find File", localPath);           // golden TfFTP.cpp:398
        return v;
    }
    const std::string dir = FtpTrimTrailingSlashes(remoteDir);
    if (!EnsureDirOneLevel(f, dir, log, now))                                       // golden :402-407
    {
        v.step = "cwd";
        v.st = FtpStatus::Fail(kFtpErrCommand, 0, "remote folder not reachable: " + dir);
        if (log) log->Add(now, "Upload", "Error Sources File Path", dir, remoteName);
        return v;
    }
    const std::string remote = FtpPathCombin(dir, remoteName);                      // golden :414
    FtpStatus st = f->Put(localPath, remote);
    LogCmd(log, now, "Upload", remote, st);
    if (!st.ok)
    {
        v.step = "stor";
        v.st = st;
        return v;
    }
    long long n = -1;
    st = f->Size(remote, &n);
    const bool sizeUnsupported = !st.ok && st.err == kFtpErrSizeUnsupported;
    if (st.ok)
    {
        v.remoteSize = n;
        if (n != v.localSize)
        {
            const std::string b = "local " + I64(v.localSize) + " remote " + I64(n);
            v.step = "size";
            v.st = FtpStatus::Fail(kFtpErrCommand, st.ftpCode, "SIZE mismatch: " + b);
            if (log) log->Add(now, "FTP Failure", "Verify SIZE mismatch", remote, b);
            return v;
        }
    }
    else if (!sizeUnsupported)
    {
        v.step = "size";
        v.st = st;
        LogCmd(log, now, "Verify SIZE", remote, st);
        return v;
    }
    std::vector<FtpEntry> e;
    st = f->List(dir, &e);
    if (!st.ok)
    {
        v.step = "list";
        v.st = st;
        LogCmd(log, now, "Verify LIST", dir, st);
        return v;
    }
    const FtpEntry* hit = 0;
    for (size_t i = 0; i < e.size() && !hit; ++i)
        if (!e[i].isDir && e[i].name == remoteName) hit = &e[i];
    if (!hit)
    {
        v.step = "list";
        v.st = FtpStatus::Fail(kFtpErrCommand, 0, "not in the listing: " + remoteName);
        if (log) log->Add(now, "FTP Failure", "Verify LIST missing", dir, remoteName);
        return v;
    }
    if (sizeUnsupported)
    {
        if (hit->size < 0) v.nameOnly = true;             // no SIZE, no size in the listing: name only (logged)
        else if (hit->size != v.localSize)
        {
            const std::string b = "local " + I64(v.localSize) + " listed " + I64(hit->size);
            v.step = "list";
            v.remoteSize = hit->size;
            v.st = FtpStatus::Fail(kFtpErrCommand, 0, "LIST size mismatch: " + b);
            if (log) log->Add(now, "FTP Failure", "Verify LIST size mismatch", remote, b);
            return v;
        }
        v.remoteSize = hit->size;
    }
    v.ok = true;
    v.st = FtpStatus::Ok();
    if (log)
        log->Add(now, "FTP Success", v.nameOnly ? "Upload verified (name only)" : "Upload verified", remote,
                 I64(v.localSize) + " bytes");
    return v;
}

static webbridge::WbMutex g_ftpSessionMu;   // plan §3 item 6: one transfer at a time

FtpSessionLock::FtpSessionLock() { g_ftpSessionMu.lock(); }
FtpSessionLock::~FtpSessionLock() { g_ftpSessionMu.unlock(); }

// ---------------------------------------------------------------------------
//  Null and log-only transports
// ---------------------------------------------------------------------------
namespace {

class NullFtp : public IElaFtp
{
public:
    const char* Kind() const { return "null"; }
    FtpStatus Connect(const FtpEndpoint&)
    {
        return FtpStatus::Fail(kFtpErrDisabled, 0, "null transport: this hub has no FTP (only W906_ElaStart installs one)");
    }
    void Close() {}
    void Abort() {}
    FtpStatus ChangeDir(const std::string&) { return NotConnected(); }
    FtpStatus MakeDir(const std::string&) { return NotConnected(); }
    FtpStatus Put(const std::string&, const std::string&) { return NotConnected(); }
    FtpStatus Size(const std::string&, long long*) { return NotConnected(); }
    FtpStatus List(const std::string&, std::vector<FtpEntry>*) { return NotConnected(); }
private:
    static FtpStatus NotConnected() { return FtpStatus::Fail(kFtpErrNotConnected, 0, "null transport"); }
};

// every call goes to FTP_Log as "FTP LogOnly"; nothing leaves the machine.  It remembers what was "stored" so SIZE / LIST
// agree and the whole N25-3 / N10 path runs.  It was the simulation build's transport under ★W36 A; since ★W36 = C
// (Steven 20260928, AI(W906-ELA-W36) St02-E helper) no build installs it -- the ELA_Ftp ctest still uses it.
class LogOnlyFtp : public IElaFtp
{
public:
    LogOnlyFtp() : log_(ElaLogRoot()), connected_(false) {}
    const char* Kind() const { return "logonly"; }
    FtpStatus Connect(const FtpEndpoint& ep)
    {
        char b[96];
        std::snprintf(b, sizeof(b), "%s:%d %s", ep.host.c_str(), ep.port, ep.passive ? "passive" : "active");
        log_.SetSecrets(ep.user, ep.password);
        log_.Add(ElaNow(), "FTP LogOnly", "CONNECT (nothing sent, log-only transport)", b);
        connected_ = true;
        return FtpStatus::Ok(230, "log only");
    }
    void Close()
    {
        if (connected_) log_.Add(ElaNow(), "FTP LogOnly", "CLOSE");
        connected_ = false;
    }
    void Abort() {}
    FtpStatus ChangeDir(const std::string& d) { return Cmd("CWD", d, 250); }
    FtpStatus MakeDir(const std::string& d) { return Cmd("MKD", d, 257); }
    FtpStatus Put(const std::string& local, const std::string& remote)
    {
        const long long n = FtpLocalFileSize(local);
        if (n < 0) return FtpStatus::Fail(kFtpErrLocalFile, 0, "local file missing: " + local);
        files_[remote] = n;
        return Cmd("STOR", local + " -> " + remote, 226);
    }
    FtpStatus Size(const std::string& remote, long long* n)
    {
        std::map<std::string, long long>::const_iterator it = files_.find(remote);
        if (it == files_.end()) return FtpStatus::Fail(kFtpErrCommand, 550, "log only: not stored");
        if (n) *n = it->second;
        return Cmd("SIZE", remote, 213);
    }
    FtpStatus List(const std::string& dir, std::vector<FtpEntry>* out)
    {
        out->clear();
        const std::string d = FtpTrimTrailingSlashes(dir);
        for (std::map<std::string, long long>::const_iterator it = files_.begin(); it != files_.end(); ++it)
            if (FtpParentDir(it->first) == d)
            {
                FtpEntry e;
                e.name = FtpBaseName(it->first);
                e.isDir = false;
                e.size = it->second;
                out->push_back(e);
            }
        return Cmd("LIST", d, 226);
    }
private:
    FtpStatus Cmd(const char* verb, const std::string& arg, int code)
    {
        if (!connected_) return FtpStatus::Fail(kFtpErrNotConnected, 0, "log only: not connected");
        log_.Add(ElaNow(), "FTP LogOnly", verb, arg);
        return FtpStatus::Ok(code, "log only");
    }
    FtpLog log_;
    bool connected_;
    std::map<std::string, long long> files_;
};

}  // namespace

IElaFtp* NewNullFtp() { return new NullFtp(); }
IElaFtp* NewLogOnlyFtp() { return new LogOnlyFtp(); }

}  // namespace ela
