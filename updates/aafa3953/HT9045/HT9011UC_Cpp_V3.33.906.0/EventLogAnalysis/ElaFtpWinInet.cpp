// ===========================================================================
//  EventLogAnalysis/ElaFtpWinInet.cpp -- the WinINet IElaFtp (plan R4, Steven 20260927: FTP transport = Windows WinINet).
//  AI(W906-ELA-R4) 20260927 (St02-E).  Design: D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-wininet-elaftp.md §3.
//  The ONLY file of the tree that includes <wininet.h>; referencing NewWinInetFtp is what links WinINet into a binary
//  (W906_ElaStart in both builds since ★W36 = C, 20260928, and the compile-only check tests/ela_ftp_wininet_link.cpp).
//  No ctest runs this file (it needs a network); both configurations must compile and link it.
//
//  Golden it replaces: Rev891 TfFTP on FastNet TNMFTP (TfFTP.cpp:53-81 Connect, :144-171 ChangeDir, :392-423 Upload,
//  :490-512 Close).
//    session   one InternetOpenA(INTERNET_OPEN_TYPE_DIRECT) per Connect (an IE proxy never reroutes FTP), options
//              CONNECT_TIMEOUT = connectMs (golden TimeOut 5000), CONNECT_RETRIES = 1, control / data send / receive
//              timeouts = transferMs; InternetConnectA(INTERNET_SERVICE_FTP, passive ? INTERNET_FLAG_PASSIVE : 0).
//    Put       FtpPutFileA(FTP_TRANSFER_TYPE_BINARY) -- binary, so the server's size is the local byte count.
//    Size      FtpCommandA "TYPE I" then "SIZE <path>", reply "213 <n>" from InternetGetLastResponseInfoA.  NOT
//              FtpGetFileSize: that needs an FtpOpenFile(GENERIC_READ) handle, which starts a RETR and blocks the session.
//    List      CWD into the folder, FtpFindFirstFileA(NULL pattern, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE)
//              + InternetFindNextFileA until ERROR_NO_MORE_FILES, find handle closed at once (one find per session).
//              NULL lists the current folder, so "Jam Rate.txt" (a blank) is never a LIST argument; RELOAD = never
//              WinINet's cached listing.
//    timeouts  CONNECT_TIMEOUT is not reliable for FTP, so every blocking call runs under a watchdog: a WbThread waits on
//              an event for the deadline and, when it passes, closes hOpen (InternetCloseHandle on the parent cancels a
//              blocked call on its children -- the documented way).  Deadlines: connect = connectMs + 1 s; STOR =
//              transferMs + size / 32 KiB/s; other commands = transferMs.  WbThread has no timed join, hence the event.
//    Abort()   any thread (Hub::Stop): closes hOpen the same way, so shutdown never waits out a 30 s transfer.
//    errors    GetLastError(); the server's last reply (InternetGetLastResponseInfoA, grown until it fits) gives the
//              code; 12xxx text from FormatMessageA on wininet.dll.  Every text is passed through MaskSecrets (a 331 /
//              530 reply often echoes the account) before it leaves this file.
//  Threading: one transport per job, created, used and closed on the Hub worker; only the watchdog and Abort() call
//  InternetCloseHandle from another thread (under mu_, once).
// ===========================================================================
#include "EventLogAnalysis/ElaFtp.h"

#ifdef UNICODE
#error "ElaFtpWinInet.cpp needs UNICODE off: MinGW wininet.h declares FtpFindFirstFileA with the TCHAR LPWIN32_FIND_DATA"
#endif

#include <windows.h>
#include <wininet.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace ela {
namespace {

std::string Trimmed(const std::string& s)
{
    size_t n = s.size();
    while (n > 0 && (s[n - 1] == '\r' || s[n - 1] == '\n' || s[n - 1] == ' ')) --n;
    return s.substr(0, n);
}

// the last server reply on THIS thread (WinINet keeps it per thread)
std::string LastResponse()
{
    std::vector<char> b(1024, 0);
    for (int tries = 0; tries < 4; ++tries)
    {
        DWORD err = 0, n = (DWORD)b.size();
        if (::InternetGetLastResponseInfoA(&err, &b[0], &n)) return Trimmed(std::string(&b[0], n));
        if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) return std::string();
        b.assign((size_t)n + 16, 0);
    }
    return std::string();
}

std::string SysText(DWORD e)
{
    char* msg = 0;
    const bool inet = e >= INTERNET_ERROR_BASE && e < INTERNET_ERROR_BASE + 1000;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS |
                        (inet ? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM);
    const DWORD n = ::FormatMessageA(flags, inet ? (LPCVOID)::GetModuleHandleA("wininet.dll") : NULL, e, 0,
                                     (LPSTR)&msg, 0, NULL);
    std::string s = (n && msg) ? Trimmed(std::string(msg, n)) : std::string();
    if (msg) ::LocalFree(msg);
    char b[32];
    std::snprintf(b, sizeof(b), "error %lu", (unsigned long)e);
    return s.empty() ? std::string(b) : std::string(b) + ": " + s;
}

class WinInetFtp;

// closes the session when a blocking call outlives its deadline
class Watchdog
{
public:
    Watchdog(WinInetFtp* owner, unsigned ms);
    ~Watchdog() { Stop(); }
    void Stop();
    bool Fired() const { return fired_.load(); }
private:
    static void Entry(void* self);
    WinInetFtp* owner_;
    unsigned ms_;
    HANDLE ev_;
    bool started_;
    std::atomic<bool> fired_;
    webbridge::WbThread thread_;
    Watchdog(const Watchdog&);
    Watchdog& operator=(const Watchdog&);
};

class WinInetFtp : public IElaFtp
{
public:
    WinInetFtp() : hOpen_(NULL), hConn_(NULL), openClosed_(false), aborted_(false) {}
    ~WinInetFtp() { Close(); }
    const char* Kind() const { return "wininet"; }

    FtpStatus Connect(const FtpEndpoint& ep)
    {
        Close();
        if (aborted_.load()) return FtpStatus::Fail(kFtpErrAborted, 0, "aborted");
        ep_ = ep;
        HINTERNET h = ::InternetOpenA("HT9045-ELA", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!h)
        {
            const DWORD e = ::GetLastError();
            return FailFrom(e, std::string(), false);
        }
        {
            webbridge::WbGuard g(mu_);
            hOpen_ = h;
            openClosed_ = false;
        }
        if (aborted_.load())            // an Abort() that came before hOpen_ was stored could not cancel it
        {
            Close();
            return FtpStatus::Fail(kFtpErrAborted, 0, "aborted");
        }
        DWORD v = ep.connectMs;
        ::InternetSetOptionA(h, INTERNET_OPTION_CONNECT_TIMEOUT, &v, sizeof(v));
        v = 1;
        ::InternetSetOptionA(h, INTERNET_OPTION_CONNECT_RETRIES, &v, sizeof(v));
        v = ep.transferMs;
        ::InternetSetOptionA(h, INTERNET_OPTION_SEND_TIMEOUT, &v, sizeof(v));
        ::InternetSetOptionA(h, INTERNET_OPTION_RECEIVE_TIMEOUT, &v, sizeof(v));
        ::InternetSetOptionA(h, INTERNET_OPTION_DATA_SEND_TIMEOUT, &v, sizeof(v));
        ::InternetSetOptionA(h, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &v, sizeof(v));
        HINTERNET c = NULL;
        DWORD e = 0;
        std::string resp;
        bool fired = false;
        {
            Watchdog wd(this, ep.connectMs + 1000u);
            c = ::InternetConnectA(h, ep.host.c_str(), (INTERNET_PORT)ep.port,
                                   ep.user.empty() ? NULL : ep.user.c_str(),
                                   ep.password.empty() ? NULL : ep.password.c_str(), INTERNET_SERVICE_FTP,
                                   ep.passive ? INTERNET_FLAG_PASSIVE : 0, 0);
            e = ::GetLastError();
            resp = LastResponse();
            wd.Stop();
            fired = wd.Fired();
        }
        if (!c)
        {
            const FtpStatus s = FailFrom(e, resp, fired);
            Close();
            return s;
        }
        {
            webbridge::WbGuard g(mu_);
            if (openClosed_) return FtpStatus::Fail(fired ? kFtpErrTimeout : kFtpErrAborted, 0, "session closed");
            hConn_ = c;
        }
        return FtpStatus::Ok(FtpReplyCode(resp), Mask(resp));
    }

    void Close()
    {
        HINTERNET c = NULL, h = NULL;
        bool closed = false;
        {
            webbridge::WbGuard g(mu_);
            c = hConn_;
            h = hOpen_;
            closed = openClosed_;
            hConn_ = NULL;
            hOpen_ = NULL;
            openClosed_ = false;
        }
        if (closed) return;                 // CancelHandles closed hOpen, and with it hConn
        if (c) ::InternetCloseHandle(c);
        if (h) ::InternetCloseHandle(h);
    }

    void Abort()
    {
        aborted_.store(true);
        CancelHandles();
    }

    // the watchdog / Abort: close hOpen once (that cancels a blocked call on its children)
    void CancelHandles()
    {
        webbridge::WbGuard g(mu_);
        if (hOpen_ && !openClosed_)
        {
            ::InternetCloseHandle(hOpen_);
            openClosed_ = true;
        }
    }

    FtpStatus ChangeDir(const std::string& d)
    {
        HINTERNET c = Conn();
        if (!c) return NotConnected();
        DWORD e = 0;
        std::string resp;
        bool fired = false;
        BOOL r;
        {
            Watchdog wd(this, ep_.transferMs);
            r = ::FtpSetCurrentDirectoryA(c, d.c_str());
            e = ::GetLastError();
            resp = LastResponse();
            wd.Stop();
            fired = wd.Fired();
        }
        return r ? FtpStatus::Ok(FtpReplyCode(resp), Mask(resp)) : FailFrom(e, resp, fired);
    }

    FtpStatus MakeDir(const std::string& d)
    {
        HINTERNET c = Conn();
        if (!c) return NotConnected();
        DWORD e = 0;
        std::string resp;
        bool fired = false;
        BOOL r;
        {
            Watchdog wd(this, ep_.transferMs);
            r = ::FtpCreateDirectoryA(c, d.c_str());
            e = ::GetLastError();
            resp = LastResponse();
            wd.Stop();
            fired = wd.Fired();
        }
        return r ? FtpStatus::Ok(FtpReplyCode(resp), Mask(resp)) : FailFrom(e, resp, fired);
    }

    FtpStatus Put(const std::string& local, const std::string& remote)
    {
        const long long n = FtpLocalFileSize(local);
        if (n < 0) return FtpStatus::Fail(kFtpErrLocalFile, 0, "local file missing: " + local);
        HINTERNET c = Conn();
        if (!c) return NotConnected();
        const unsigned long long extra = (unsigned long long)n * 1000ull / 32768ull;   // 32 KiB/s floor
        const unsigned ms = ep_.transferMs + (unsigned)(extra > 3600000ull ? 3600000ull : extra);
        DWORD e = 0;
        std::string resp;
        bool fired = false;
        BOOL r;
        {
            Watchdog wd(this, ms);
            r = ::FtpPutFileA(c, local.c_str(), remote.c_str(), FTP_TRANSFER_TYPE_BINARY, 0);
            e = ::GetLastError();
            resp = LastResponse();
            wd.Stop();
            fired = wd.Fired();
        }
        return r ? FtpStatus::Ok(FtpReplyCode(resp), Mask(resp)) : FailFrom(e, resp, fired);
    }

    FtpStatus Size(const std::string& remote, long long* n)
    {
        FtpStatus st = Command("TYPE I");
        if (!st.ok) return st;
        st = Command("SIZE " + remote);
        if (!st.ok)
        {
            if (st.ftpCode == 500 || st.ftpCode == 502 || st.ftpCode == 504) st.err = kFtpErrSizeUnsupported;
            return st;
        }
        long long x = -1;
        if (!ParseSizeReply(st.text, &x)) return FtpStatus::Fail(kFtpErrCommand, st.ftpCode, "unexpected SIZE reply: " + st.text);
        if (n) *n = x;
        return st;
    }

    FtpStatus List(const std::string& dir, std::vector<FtpEntry>* out)
    {
        out->clear();
        FtpStatus st = ChangeDir(dir);
        if (!st.ok) return st;
        HINTERNET c = Conn();
        if (!c) return NotConnected();
        DWORD e = 0;
        std::string resp;
        bool fired = false;
        bool ok = true;
        {
            Watchdog wd(this, ep_.transferMs);
            WIN32_FIND_DATAA fd;
            std::memset(&fd, 0, sizeof(fd));
            HINTERNET hf = ::FtpFindFirstFileA(c, NULL, &fd, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
            if (!hf)
            {
                e = ::GetLastError();
                ok = (e == ERROR_NO_MORE_FILES);          // an empty folder
            }
            else
            {
                do
                {
                    if (std::strcmp(fd.cFileName, ".") != 0 && std::strcmp(fd.cFileName, "..") != 0)
                    {
                        FtpEntry en;
                        en.name = fd.cFileName;
                        en.isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                        en.size = ((long long)fd.nFileSizeHigh << 32) | (long long)fd.nFileSizeLow;
                        out->push_back(en);
                    }
                    std::memset(&fd, 0, sizeof(fd));
                } while (::InternetFindNextFileA(hf, &fd));
                e = ::GetLastError();
                ok = (e == ERROR_NO_MORE_FILES);
                ::InternetCloseHandle(hf);                 // one find per session: close it at once
            }
            resp = LastResponse();
            wd.Stop();
            fired = wd.Fired();
        }
        if (ok && !fired) return FtpStatus::Ok(FtpReplyCode(resp), Mask(resp));
        return FailFrom(e, resp, fired);
    }

private:
    FtpStatus Command(const std::string& cmd)
    {
        HINTERNET c = Conn();
        if (!c) return NotConnected();
        DWORD e = 0;
        std::string resp;
        bool fired = false;
        BOOL r;
        {
            Watchdog wd(this, ep_.transferMs);
            r = ::FtpCommandA(c, FALSE, FTP_TRANSFER_TYPE_BINARY, cmd.c_str(), 0, NULL);
            e = ::GetLastError();
            resp = LastResponse();
            wd.Stop();
            fired = wd.Fired();
        }
        return r ? FtpStatus::Ok(FtpReplyCode(resp), Mask(resp)) : FailFrom(e, resp, fired);
    }

    HINTERNET Conn()
    {
        webbridge::WbGuard g(mu_);
        return (openClosed_ || aborted_.load()) ? NULL : hConn_;
    }

    std::string Mask(const std::string& s) const { return MaskSecrets(s, ep_.user, ep_.password); }

    FtpStatus NotConnected() const
    {
        return FtpStatus::Fail(aborted_.load() ? kFtpErrAborted : kFtpErrNotConnected, 0,
                               aborted_.load() ? "aborted" : "not connected");
    }

    FtpStatus FailFrom(DWORD e, const std::string& resp, bool fired) const
    {
        const int code = FtpReplyCode(resp);
        int err = kFtpErrOther;
        if (fired) err = kFtpErrTimeout;
        else if (aborted_.load()) err = kFtpErrAborted;
        else if (e == ERROR_INTERNET_TIMEOUT) err = kFtpErrTimeout;
        else if (e == ERROR_INTERNET_LOGIN_FAILURE || e == ERROR_INTERNET_INCORRECT_PASSWORD ||
                 e == ERROR_INTERNET_INCORRECT_USER_NAME || code == 530)
            err = kFtpErrLogin;
        else if (e == ERROR_INTERNET_NAME_NOT_RESOLVED || e == ERROR_INTERNET_CANNOT_CONNECT ||
                 e == ERROR_INTERNET_CONNECTION_ABORTED || e == ERROR_INTERNET_CONNECTION_RESET)
            err = kFtpErrConnect;
        else if (e == ERROR_INTERNET_OPERATION_CANCELLED) err = kFtpErrAborted;
        else if (e == ERROR_INTERNET_EXTENDED_ERROR || code >= 400) err = kFtpErrCommand;
        std::string text = (e == ERROR_INTERNET_EXTENDED_ERROR && !resp.empty()) ? resp : SysText(e);
        if (e != ERROR_INTERNET_EXTENDED_ERROR && !resp.empty()) text += " / " + resp;
        if (fired) text = "deadline passed, session closed: " + text;
        return FtpStatus::Fail(err, code, Mask(text), e);
    }

    webbridge::WbMutex mu_;
    HINTERNET hOpen_, hConn_;
    bool openClosed_;
    std::atomic<bool> aborted_;
    FtpEndpoint ep_;
};

Watchdog::Watchdog(WinInetFtp* owner, unsigned ms)
    : owner_(owner), ms_(ms), ev_(::CreateEventA(NULL, TRUE, FALSE, NULL)), started_(false), fired_(false)
{
    if (ev_) started_ = thread_.start(&Watchdog::Entry, this);
}

void Watchdog::Stop()
{
    if (ev_)
    {
        ::SetEvent(ev_);
        if (started_) thread_.join();
        ::CloseHandle(ev_);
        ev_ = NULL;
        started_ = false;
    }
}

void Watchdog::Entry(void* self)
{
    Watchdog* w = static_cast<Watchdog*>(self);
    if (::WaitForSingleObject(w->ev_, w->ms_) == WAIT_TIMEOUT)
    {
        w->fired_.store(true);
        w->owner_->CancelHandles();
    }
}

}  // namespace

IElaFtp* NewWinInetFtp() { return new WinInetFtp(); }

}  // namespace ela
