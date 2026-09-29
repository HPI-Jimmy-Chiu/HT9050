// =============================================================================
//  tests/ElaFtpFake.h -- a scripted IElaFtp for the ELA_Ftp ctest (header-only, never in a production archive).
//  AI(W906-ELA-R4) 20260927 (St02-E).
//
//  A remote file system in memory (folders + file sizes) with switches for every failure the upload code must handle.
//  It records one line per call ("CONNECT host:21 active", "CWD /a", "MKD /a", "STOR /a/f", "SIZE /a/f", "LIST /a",
//  "CLOSE") and never the account.  It never touches a socket.
// =============================================================================
#ifndef HT9045_TESTS_ELAFTPFAKE_H
#define HT9045_TESTS_ELAFTPFAKE_H

#include "EventLogAnalysis/ElaFtp.h"

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

struct FakeFtpScript
{
    std::set<std::string> dirs;                  // remote folders that exist ("/" always)
    std::map<std::string, long long> files;      // remote path -> size
    bool connectFail;
    int connectErr, connectCode;
    std::string connectText;                     // may hold the account: the code under test must mask it
    std::map<std::string, int> storFail;         // remote path -> reply code
    std::map<std::string, long long> sizeLie;    // remote path -> the size SIZE reports
    bool sizeUnsupported;                        // SIZE -> 502
    int listSizeMode;                            // 0 = real size, 1 = no size (-1), 2 = 0 bytes
    std::set<std::string> listHide;              // file names LIST leaves out
    std::vector<std::string> calls;
    int created;

    FakeFtpScript() { Reset(); }
    void Reset()
    {
        dirs.clear();
        dirs.insert("/");
        files.clear();
        connectFail = false;
        connectErr = ela::kFtpErrConnect;
        connectCode = 0;
        connectText.clear();
        storFail.clear();
        sizeLie.clear();
        sizeUnsupported = false;
        listSizeMode = 0;
        listHide.clear();
        calls.clear();
        created = 0;
    }
    std::string Joined() const
    {
        std::string s;
        for (size_t i = 0; i < calls.size(); ++i) s += calls[i] + "\n";
        return s;
    }
    int Count(const std::string& prefix) const
    {
        int n = 0;
        for (size_t i = 0; i < calls.size(); ++i)
            if (calls[i].compare(0, prefix.size(), prefix) == 0) ++n;
        return n;
    }
};

class FakeFtp : public ela::IElaFtp
{
public:
    explicit FakeFtp(FakeFtpScript* s) : s_(s), connected_(false) {}
    const char* Kind() const { return "fake"; }
    ela::FtpStatus Connect(const ela::FtpEndpoint& ep)
    {
        char b[160];
        std::snprintf(b, sizeof(b), "CONNECT %s:%d %s", ep.host.c_str(), ep.port, ep.passive ? "passive" : "active");
        s_->calls.push_back(b);
        if (s_->connectFail) return ela::FtpStatus::Fail(s_->connectErr, s_->connectCode, s_->connectText);
        connected_ = true;
        return ela::FtpStatus::Ok(230, "230 logged in");
    }
    void Close() { s_->calls.push_back("CLOSE"); connected_ = false; }
    void Abort() { s_->calls.push_back("ABORT"); connected_ = false; }
    ela::FtpStatus ChangeDir(const std::string& d)
    {
        s_->calls.push_back("CWD " + d);
        if (!connected_) return NotConnected();
        if (s_->dirs.count(Norm(d))) return ela::FtpStatus::Ok(250, "250 ok");
        return ela::FtpStatus::Fail(ela::kFtpErrCommand, 550, "550 no such folder");
    }
    ela::FtpStatus MakeDir(const std::string& d)
    {
        s_->calls.push_back("MKD " + d);
        if (!connected_) return NotConnected();
        const std::string n = Norm(d);
        if (s_->dirs.count(n)) return ela::FtpStatus::Fail(ela::kFtpErrCommand, 550, "550 exists");
        if (!s_->dirs.count(Parent(n))) return ela::FtpStatus::Fail(ela::kFtpErrCommand, 550, "550 parent missing");
        s_->dirs.insert(n);
        return ela::FtpStatus::Ok(257, "257 created");
    }
    ela::FtpStatus Put(const std::string& local, const std::string& remote)
    {
        s_->calls.push_back("STOR " + remote);
        if (!connected_) return NotConnected();
        std::map<std::string, int>::const_iterator f = s_->storFail.find(remote);
        if (f != s_->storFail.end()) return ela::FtpStatus::Fail(ela::kFtpErrCommand, f->second, "STOR refused");
        if (!s_->dirs.count(Parent(remote))) return ela::FtpStatus::Fail(ela::kFtpErrCommand, 550, "550 no folder");
        const long long n = ela::FtpLocalFileSize(local);
        if (n < 0) return ela::FtpStatus::Fail(ela::kFtpErrLocalFile, 0, "local missing");
        s_->files[remote] = n;
        return ela::FtpStatus::Ok(226, "226 done");
    }
    ela::FtpStatus Size(const std::string& remote, long long* n)
    {
        s_->calls.push_back("SIZE " + remote);
        if (!connected_) return NotConnected();
        if (s_->sizeUnsupported) return ela::FtpStatus::Fail(ela::kFtpErrSizeUnsupported, 502, "502 SIZE not implemented");
        std::map<std::string, long long>::const_iterator it = s_->files.find(remote);
        if (it == s_->files.end()) return ela::FtpStatus::Fail(ela::kFtpErrCommand, 550, "550 no such file");
        std::map<std::string, long long>::const_iterator lie = s_->sizeLie.find(remote);
        if (n) *n = lie != s_->sizeLie.end() ? lie->second : it->second;
        return ela::FtpStatus::Ok(213, "213 size");
    }
    ela::FtpStatus List(const std::string& dir, std::vector<ela::FtpEntry>* out)
    {
        s_->calls.push_back("LIST " + dir);
        out->clear();
        if (!connected_) return NotConnected();
        const std::string d = Norm(dir);
        if (!s_->dirs.count(d)) return ela::FtpStatus::Fail(ela::kFtpErrCommand, 550, "550 no such folder");
        for (std::map<std::string, long long>::const_iterator it = s_->files.begin(); it != s_->files.end(); ++it)
        {
            if (Parent(it->first) != d) continue;
            ela::FtpEntry e;
            e.name = Base(it->first);
            if (s_->listHide.count(e.name)) continue;
            e.isDir = false;
            e.size = s_->listSizeMode == 1 ? -1 : (s_->listSizeMode == 2 ? 0 : it->second);
            out->push_back(e);
        }
        for (std::set<std::string>::const_iterator it = s_->dirs.begin(); it != s_->dirs.end(); ++it)
            if (*it != "/" && Parent(*it) == d)
            {
                ela::FtpEntry e;
                e.name = Base(*it);
                e.isDir = true;
                e.size = 0;
                out->push_back(e);
            }
        return ela::FtpStatus::Ok(226, "226 listed");
    }

    static std::string Norm(const std::string& p)
    {
        const std::string t = ela::FtpTrimTrailingSlashes(p);
        return t.empty() ? std::string("/") : t;
    }
    static std::string Parent(const std::string& p)
    {
        const std::string t = ela::FtpParentDir(p);
        return t.empty() ? std::string("/") : t;
    }
    static std::string Base(const std::string& p) { return ela::FtpBaseName(p); }

private:
    ela::FtpStatus NotConnected() const { return ela::FtpStatus::Fail(ela::kFtpErrNotConnected, 0, "not connected"); }
    FakeFtpScript* s_;
    bool connected_;
};

#endif
