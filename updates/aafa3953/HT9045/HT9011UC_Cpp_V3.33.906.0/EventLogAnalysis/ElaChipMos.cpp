// ===========================================================================
//  EventLogAnalysis/ElaChipMos.cpp -- see ElaChipMos.h.  AI(W906-ELA-R4) 20260927 (St02-E).
//  Also Hub::RunUploadJob (declared in ElaHub.h): the EL_UPLOAD_JAMWEEK / EL_UPLOAD_BYFILE_N10 jobs of the worker, and
//  since W22 (AI(W906-ELA-W22) 20260928) EL_UPLOAD_SUMMARY (N25-4) / EL_UPLOAD_EVENTLOG (N25-5).
// ===========================================================================
#include "EventLogAnalysis/ElaChipMos.h"
#include "EventLogAnalysis/ElaHub.h"
#include "EventLogAnalysis/ElaReports.h"   // Clock, AutoJobsEnabled (R2)

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

namespace ela {

ElaUploadContext::ElaUploadContext() : now(0.0), factory(0), slot(0) {}

// ---------------------------------------------------------------------------
//  local files
// ---------------------------------------------------------------------------
namespace {

// golden FileInfo::EnsureDirectoriesExist (FileInfo.cpp:284-312): every '\' level, stop at the first CreateDirectory
// failure.  '/' counts as a separator too: the W906_HT9045LOG_ROOT seam may come with forward slashes (the ctest
// containment root); golden's literal paths have none, so golden paths give the same levels.
void EnsureDirectoriesExist(const std::string& path)
{
    std::string p = path;
    if (!p.empty() && (p[p.size() - 1] == '\\' || p[p.size() - 1] == '/')) p.erase(p.size() - 1);
    size_t pos = 0;
    while (pos <= p.size())
    {
        size_t next = p.find_first_of("\\/", pos);
        if (next == std::string::npos) next = p.size();
        const std::string cur = p.substr(0, next);
        if (!cur.empty() && ::GetFileAttributesA(cur.c_str()) == INVALID_FILE_ATTRIBUTES)
            if (!::CreateDirectoryA(cur.c_str(), NULL)) return;
        pos = next + 1;
    }
}

// golden FileInfo::PathCombin for a LOCAL folder (FileInfo.cpp:272-277): add '\' unless the folder ends in one.  Not
// FtpPathCombin: a seam root holding '/' would make it an "FTP path"; for golden's literal paths both give the same.
std::string LocalCombine(const std::string& dir, const std::string& file)
{
    if (dir.empty()) return file;
    const char last = dir[dir.size() - 1];
    return (last == '\\' || last == '/') ? dir + file : dir + "\\" + file;
}

// golden WriteDataToFile (Common.cpp:584-608): OPEN_ALWAYS, append, the data + "\n"
void AppendLine(const std::string& path, const std::string& data)
{
    HANDLE h = ::CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    ::SetFilePointer(h, 0, NULL, FILE_END);
    const std::string s = data + "\n";
    DWORD w = 0;
    ::WriteFile(h, s.data(), (DWORD)s.size(), &w, NULL);
    ::CloseHandle(h);
}

bool WriteWhole(const std::string& path, const std::string& data)
{
    ::DeleteFileA(path.c_str());                  // plan §3 item 3: rebuilt, not appended to
    HANDLE h = ::CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD w = 0;
    const BOOL ok = ::WriteFile(h, data.data(), (DWORD)data.size(), &w, NULL);
    ::CloseHandle(h);
    return ok && w == (DWORD)data.size();
}

std::string FmtDT(double t)
{
    int y, m, d, h, n, s, ms;
    DecodeDateTime(t, &y, &m, &d, &h, &n, &s, &ms);
    char b[40];
    std::snprintf(b, sizeof(b), "%04d/%02d/%02d %02d:%02d:%02d", y, m, d, h, n, s);
    return b;
}

std::string Int(long v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%ld", v);
    return b;
}

bool IsLogOnly(const IElaFtp* f) { return std::strcmp(f->Kind(), "logonly") == 0; }

bool Retryable(const FtpStatus& st)
{
    return st.err == kFtpErrTimeout || st.err == kFtpErrConnect || st.err == kFtpErrCommand || st.err == kFtpErrOther ||
           st.err == kFtpErrSizeUnsupported;
}

std::string Trim(const std::string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (unsigned char)s[a] <= ' ') ++a;
    while (b > a && (unsigned char)s[b - 1] <= ' ') --b;
    return s.substr(a, b - a);
}

// AI(W906-ELA-W22) 20260928: golden std::ifstream + std::getline in text mode (uAnalysisProductionLog.cpp:104-118):
// lines end at '\n', a CR before it goes (text mode), a lone CR stays; no empty line after the last '\n'
bool ReadLinesLf(const std::string& path, std::vector<std::string>* lines)
{
    lines->clear();
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    std::string all;
    char b[8192];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) all.append(b, n);
    std::fclose(f);
    size_t pos = 0;
    while (pos < all.size())
    {
        size_t e = all.find('\n', pos);
        const bool last = e == std::string::npos;
        if (last) e = all.size();
        std::string line = all.substr(pos, e - pos);
        if (!last && !line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        lines->push_back(line);
        pos = e + 1;
    }
    return true;
}

// the day count of a TDateTime the way FormatString sees it (DateTimeToMs rounds to the millisecond first)
long long DayOf(double t) { return DateTimeToMs(t) / 86400000LL; }

std::string DatedFolder(const std::string& logRoot, const char* name, double now)
{
    int y, m, d, h, n, sec, ms;
    DecodeDateTime(now, &y, &m, &d, &h, &n, &sec, &ms);
    char b[96];
    std::snprintf(b, sizeof(b), "\\%s\\%d\\%d\\%d", name, y, m, d);      // golden "%d\\%d\\%d": no zero pad
    return logRoot + b;
}

}  // namespace

// ===========================================================================
//  N25-3
// ===========================================================================
std::string N25_3GateReason(const N25Settings& s)
{
    if (!s.o10) return "O10 (O06-1 EnableAutoSaveEventLog) is off: golden runs no analyzer job then (#22 D-a)";
    if (std::atoi(s.custCode.c_str()) != 851)
        return "CUSTOMER_CODE " + s.custCode + " is not CC_ChipMos_ZHUBEI 851 (golden loads N25-3 only for 851)";
    if (!s.enableJamLog) return "N25-3 bN25_3_EnableULJamLog is off (#22 D-f)";
    if (Trim(s.host).empty()) return "N25-2 FTP host is empty (#22 D-f)";
    if (Trim(s.jamLogPath).empty()) return "N25-3 FTP path is empty (golden uChipMosZHUBEI_Func.cpp:188)";
    return std::string();
}

double NextN25Slot(double now)
{
    return std::floor(now) + 1.0 + EncodeTime(0, 0, 1, 0);
}

bool SaveJamCodeFor7Days(const ElaUploadContext& c, const std::string& saveFolder, std::string* jamFileName, int* rows)
{
    *jamFileName = "Jam Rate.txt";                                              // golden :27
    const std::string path = LocalCombine(saveFolder, *jamFileName);           // :28
    // golden :30-34: the pickers Now()-8 00:00:00.000 .. Now()-1 EncodeTime(23, 59, 59, 59), then GetEventLogTextToVec.
    // R1's (ElaCore) on an Analyzer of this job's own: golden moves the page's pickers and clears its sl*List; here the
    // page's Analyzer is not touched.  D-d: the folder is scanned now (golden: the list of the last query).
    Analyzer a(c.opt);
    std::vector<LogRecord> recs;
    a.GetEventLogTextToVec(std::floor(c.now - 8.0), EncodeTime(0, 0, 0, 0), std::floor(c.now - 1.0),
                           EncodeTime(23, 59, 59, 59), LoadFavorite(c.eventLogDir), recs);   // :34
    std::string text = "date,time,unitName,alarmCode,recovery,stoppedTime,duplicate,message,errorPart\n";   // :36-37
    int n = 0;
    for (size_t i = 0; i < recs.size(); ++i)
    {
        const LogRecord& r = recs[i];
        if (r.alarmCode.find("JAM") == std::string::npos) continue;             // :42 AnsiPos("JAM")>0
        // :44-53 -- D-e: UTF-8 (the BCB-written EventLogTxt is cp950)
        text += ToUtf8(r.date + "," + r.time + "," + r.unitName + "," + r.alarmCode + "," + r.recovery + "," +
                       r.stoppedTime + "," + r.duplicate + "," + r.message + "," + r.errorPart) + "\n";
        ++n;
    }
    if (rows) *rows = n;
    return WriteWhole(path, text);
}

// golden N25_UploadJamDataToFTP (:184-255); since W22 also N25_UploadSummaryCountToFTP (:257-326, the same steps) and
// UploadEventLog's call (:386).  label / missingTag = the FTP_Log texts of the job.  AI(W906-ELA-W22) 20260928
static bool UploadMainAndBackup(const ElaUploadContext& c, const char* label, const char* missingTag,
                                const std::string& user, const std::string& password, const std::string& host,
                                const std::string& sourcesPath, const std::string& targetPath,
                                const std::string& jamFileName, N25Result* r)
{
    if (Trim(sourcesPath).empty() || Trim(targetPath).empty()) return false;          // golden :188-191
    r->backupName = BackupFileName(jamFileName, c.now);                                 // :193-200
    FtpEndpoint ep;                                                                     // :202-204 (N25-2 account)
    ep.host = host;
    ep.user = user;
    ep.password = password;
    FtpLog log(c.logRoot);
    log.SetSecrets(user, password);
    const std::string local = LocalCombine(sourcesPath, jamFileName);
    if (FtpLocalFileSize(local) < 0)                                                    // :223 (checked first here)
    {
        r->detail = "local file missing: " + local;
        log.Add(c.now, "Upload Fail", missingTag, sourcesPath, jamFileName);
        return false;
    }
    FtpSessionLock one;                                                                 // plan §3 item 6
    if (c.slot && c.slot->Stopping())
    {
        r->detail = "stopping: no session started";
        return false;
    }
    IElaFtp* f = (c.factory ? c.factory : &NewNullFtp)();
    ++r->factoryCalls;
    r->logOnly = IsLogOnly(f);
    if (c.slot) c.slot->Set(f);
    log.Add(c.now, "FTP", label, ep.host, targetPath, f->Kind());
    const FtpStatus cs = f->Connect(ep);                                                // :211-212
    bool ok = false;
    if (!cs.ok)
    {
        r->detail = "connect: " + MaskSecrets(cs.text, user, password);
        r->retryable = Retryable(cs) || cs.err == kFtpErrLogin;
        log.Add(c.now, "FTP Failure", "Connection Failed", ep.host, r->detail);
    }
    else
    {
        r->connected = true;
        // golden :216-221: base, base/<hostname>, base/<hostname>/BackUp -- each CWD, on failure MKD (results ignored)
        EnsureDirOneLevel(f, targetPath, &log, c.now);
        const std::string target = FtpPathCombin(targetPath, c.hostName);               // :217
        EnsureDirOneLevel(f, target, &log, c.now);
        const std::string backup = FtpPathCombin(target, "BackUp");                     // :220
        EnsureDirOneLevel(f, backup, &log, c.now);
        r->remoteMain = FtpPathCombin(target, jamFileName);
        r->remoteBackup = FtpPathCombin(backup, r->backupName);
        const UploadVerify m = UploadAndVerify(f, local, target, jamFileName, &log, c.now);          // :225
        const UploadVerify b = UploadAndVerify(f, local, backup, r->backupName, &log, c.now);        // :235 (tried
        r->mainOk = m.ok;                                                                           //   even when :225
        r->backupOk = b.ok;                                                                         //   failed)
        ok = m.ok && b.ok;                     // golden :238-243: the BackUp result alone decided
        if (!m.ok) r->detail = "main " + m.step + ": " + MaskSecrets(m.st.text, user, password);
        if (!b.ok)
            r->detail += (r->detail.empty() ? "" : "; ") + std::string("BackUp ") + b.step + ": " +
                         MaskSecrets(b.st.text, user, password);
        if (!ok) r->retryable = true;
        if (ok)
        {
            r->detail = r->remoteMain + " + " + r->remoteBackup;
            if (m.nameOnly || b.nameOnly) r->detail += " (name only: SIZE unsupported, listing without sizes)";
        }
    }
    if (c.slot) c.slot->Set(0);
    f->Close();                                                                         // TfFTP::~TfFTP -> Close
    delete f;
    return ok;
}

bool N25_UploadJamDataToFTP(const ElaUploadContext& c, const N25Settings& s, const std::string& sourcesPath,
                            const std::string& targetPath, const std::string& jamFileName, N25Result* r)
{
    return UploadMainAndBackup(c, "N25-3 Jam log upload", "File not exist N25-3", s.user, s.password, s.host, sourcesPath,
                               targetPath, jamFileName, r);
}

N25Result UploadJamCode(const ElaUploadContext& c, const N25Settings& s)
{
    N25Result r;
    r.gate = N25_3GateReason(s);
    if (!r.gate.empty()) return r;                                                      // D-f: no transport at all
    int y, m, d, h, n, sec, ms;
    DecodeDateTime(c.now, &y, &m, &d, &h, &n, &sec, &ms);
    char b[64];
    std::snprintf(b, sizeof(b), "\\JamWeek\\%d\\%d\\%d", y, m, d);                      // golden :331-332 (no zero pad)
    r.saveFolder = c.logRoot + b;
    EnsureDirectoriesExist(r.saveFolder);                                               // :333
    const std::string hadUpload = LocalCombine(r.saveFolder, "HadUpload.txt");          // :334-335
    if (FileExistsA(hadUpload))                                                         // :336-339
    {
        r.alreadyUploaded = true;
        return r;
    }
    SaveJamCodeFor7Days(c, r.saveFolder, &r.jamFile, &r.records);                       // :345
    const bool ok = N25_UploadJamDataToFTP(c, s, r.saveFolder, s.jamLogPath, r.jamFile, &r);   // :341 / :346
    if (ok && r.logOnly)
    {
        r.detail = "log only: nothing sent, HadUpload.txt not written; " + r.detail;
        r.retryable = false;
    }
    else if (ok)
    {
        // plan §3 item 2: only now, with golden's two lines (:227 / :237 -- the BackUp line names the BackUp folder,
        // golden prints the main one) and its marker (:348)
        AppendLine(hadUpload, "Upload path:" + r.remoteMain);
        AppendLine(hadUpload, "Upload backup path:" + r.remoteBackup);
        AppendLine(hadUpload, "HadUpload");
        r.hadUploadWritten = FileExistsA(hadUpload);
    }
    return r;
}

// ===========================================================================
//  N25-4 / N25-5 (W22, AI(W906-ELA-W22) 20260928, St02-E) -- see ElaChipMos.h
// ===========================================================================
static std::string N25UploadGate(const N25UploadSettings& s, const char* job, const char* key)
{
    if (!s.o10) return "O10 (O06-1 EnableAutoSaveEventLog) is off: golden runs no analyzer job then (#22 D-a)";
    if (std::atoi(s.custCode.c_str()) != 851)
        return "CUSTOMER_CODE " + s.custCode + " is not CC_ChipMos_ZHUBEI 851 (golden loads " + job + " only for 851)";
    if (!s.enable) return std::string(job) + " " + key + " is off (#22 D-f)";
    if (Trim(s.host).empty()) return "N25-2 FTP host is empty (#22 D-f)";
    if (Trim(s.path).empty()) return std::string(job) + " FTP path is empty (golden uChipMosZHUBEI_Func.cpp:261 / :188)";
    return std::string();
}

std::string N25_4GateReason(const N25UploadSettings& s) { return N25UploadGate(s, "N25-4", "bN25_4_EnableUpload"); }
std::string N25_5GateReason(const N25UploadSettings& s) { return N25UploadGate(s, "N25-5", "bN25_5_EnableUpload"); }

std::string GetColumn(const std::string& line, int colIndex)
{
    std::string result;                                                                 // golden Common.cpp:442-467
    int currentCol = 0;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i)
    {
        const char ch = line[i];
        if (ch == '"')
        {
            inQuotes = !inQuotes;
            continue;
        }
        if (ch == ',' && !inQuotes)
        {
            if (currentCol == colIndex) return result;
            ++currentCol;
            result.clear();
        }
        else
            result += ch;
    }
    return currentCol == colIndex ? result : std::string();
}

double ParseDateTimeText(const std::string& in)
{
    const std::string str = Trim(in);                                                   // Common.cpp:694 AnsiString::Trim
    if (str.empty() || str.size() < 19) return 0.0;                                     // :696-700 TDateTime()
    const size_t sp = str.find(' ');                                                    // :703 Pos(" ")
    // :704-705 SubString(1, spacePos-1) / SubString(spacePos+1, 8); no space: Pos = 0 -> "" and the first 8 characters
    const std::string dateStr = sp == std::string::npos ? std::string() : str.substr(0, sp);
    const std::string timeStr = sp == std::string::npos ? str.substr(0, 8) : str.substr(sp + 1, 8);
    return ParseDateTime(dateStr, timeStr);                                             // :706 (0 on failure = :712)
}

int ProductionDeviceCount(const std::string& prodRoot, const std::string& fileTitle, double dtstart, double dtend,
                          bool keepGoldenBugs)
{
    int sum = 0;
    // golden :69 for(dtCurrent=dtstart; dtCurrent<=dtend; dtCurrent+=1) -- the day's file of each step (W18 B: of each
    // calendar day from dtstart's to dtend's; golden misses the last one when dtend's time of day < dtstart's)
    std::vector<double> days;
    if (keepGoldenBugs)
        for (double cur = dtstart; cur <= dtend; cur += 1.0) days.push_back(cur);
    else
        for (long long d = DayOf(dtstart); d <= DayOf(dtend); ++d) days.push_back((double)d);
    for (size_t k = 0; k < days.size(); ++k)
    {
        const std::string ymd = FormatYYYYMMDD(days[k]);                                // FormatString("yyyymmdd")
        const std::string path = LocalCombine(LocalCombine(prodRoot, ymd.substr(0, 6)), // :71-74 "yyyymm" folder,
                                              fileTitle + "_" + ymd + ".csv");          //   "%s_%s.csv"
        std::vector<std::string> lines;
        if (!ReadLinesLf(path, &lines)) continue;                                       // :104 is_open
        for (size_t i = 1; i < lines.size(); ++i)                                       // :109-113 the header skipped
        {
            const std::string t = GetColumn(lines[i], eLoadTime);                       // :176
            if (t.empty()) continue;                                                    // :177
            const double in = ParseDateTimeText(t);                                     // :179
            if (dtstart <= in && in <= dtend) ++sum;                                    // :180-183
        }
    }
    return sum;
}

bool SaveJamSummaryFor30Days(const ElaUploadContext& c, const std::string& saveFolder, std::string* fileName,
                             std::vector<JamInterval>* intervals, int* events)
{
    *fileName = "Jam_Summary.csv";                                                      // golden :60
    const std::string path = LocalCombine(saveFolder, *fileName);                      // :61
    // :124-131 the pickers Now()-31 00:00:00.000 .. Now()-1 EncodeTime(23, 59, 59, 59), GetSysDateTimeRange; :63 the
    // records -- R1's GetEventLogTextToVec on an Analyzer of this job's own (the page's is not touched); D-d: the
    // folder is scanned now
    Analyzer a(c.opt);
    std::vector<LogRecord> recs;
    a.GetEventLogTextToVec(std::floor(c.now - 31.0), EncodeTime(0, 0, 0, 0), std::floor(c.now - 1.0),
                           EncodeTime(23, 59, 59, 59), LoadFavorite(c.eventLogDir), recs);
    const double dtstart = a.dtStart, dtend = a.dtEnd;
    std::vector<JamInterval> iv;
    int jam = 0;
    for (size_t i = 0; i < recs.size(); ++i)                                            // :67
    {
        if (i == 0)                                                                     // :69-74
        {
            const JamInterval t = { dtstart, 0.0, 0, 0 };
            iv.push_back(t);
        }
        const LogRecord& r = recs[i];
        if (r.alarmCode.find("JAM") != std::string::npos) ++jam;                        // :76-79 AnsiPos("JAM")>0
        else if (r.recovery.find("CLEAN_OUT") != std::string::npos)                     // :80
        {
            const double t = ParseDateTime(r.date, r.time);                             // :82
            if (t == 0.0) continue;                                                     // :83-84 TDateTime()
            iv.back().dtTimeE = t;                                                      // :85-87 close the interval
            iv.back().iJamCount = jam;
            const JamInterval n = { t, 0.0, 0, 0 };                                     // :89-91 the next one
            iv.push_back(n);
            jam = 0;                                                                    // :92
        }
    }
    if (events) *events = (int)recs.size();
    if (iv.empty())
    {
        // golden :95 tIntervalRecs[size-1] of an empty vector (ledger G9): D-c -- no file, no upload
        ::DeleteFileA(path.c_str());
        if (intervals) intervals->clear();
        return false;
    }
    iv.back().dtTimeE = dtend;                                                          // :95-97
    iv.back().iJamCount = jam;
    std::string text = ReportEncode("Time,Summary,JamCount") + "\n";                   // :99-100 (WriteDataToFile: LF)
    for (size_t i = 0; i < iv.size(); ++i)                                              // :105-119 (the loop :106)
    {
        // :112-113 AnalysisProductionLog, sFileTitle = sHostName (gethostname, Analyzer.cpp:429-432)
        iv[i].iDeviceCount = ProductionDeviceCount(c.prodLogDir, c.hostName, iv[i].dtTimeS, iv[i].dtTimeE,
                                                   c.opt.keepGoldenBugs);
        // :115-118 ConvertDTToStrDateTime(dtTimeE) (Common.cpp:437-440 "YYYY/MM/DD HH:NN:SS"), devices, JAM records
        text += ReportEncode(FmtDT(iv[i].dtTimeE) + "," + Int(iv[i].iDeviceCount) + "," + Int(iv[i].iJamCount)) + "\n";
    }
    if (intervals) *intervals = iv;
    return WriteWhole(path, text);                                                      // rebuilt (golden appends)
}

bool SaveEventLog(const ElaUploadContext& c, const std::string& saveFolder, std::string* fileName, std::string* source,
                  bool* sourceMissing, std::string* detail)
{
    *fileName = "EventLog.txt";                                                         // golden :136
    const std::string dst = LocalCombine(saveFolder, *fileName);                       // :137
    // :139-140 Now()-1: GetEventLogFolder "D:\\HT9045_Log\\EventLogTxt\\%s\\%02s" (YYYY, MM) + GetEventLogName
    // "EventLogTxt_%s.csv" (YYYYMMDD), uAnalysisEventLogText.cpp:27-41 -- the root is the W906_EVENTLOG_ROOT seam
    const std::string ymd = FormatYYYYMMDD(c.now - 1.0);
    *source = LocalCombine(LocalCombine(LocalCombine(c.eventLogDir, ymd.substr(0, 4)), ymd.substr(4, 2)),
                           "EventLogTxt_" + ymd + ".csv");
    *sourceMissing = false;
    const long long n = FtpLocalFileSize(*source);
    if (n < 0)
    {
        *sourceMissing = true;                                                          // golden: CopyFile fails silently
        *detail = "no event log of yesterday: " + *source;
        return false;
    }
    if (!::CopyFileA(source->c_str(), dst.c_str(), FALSE))                              // :142 bFailIfExists=false
    {
        *detail = "CopyFile " + *source + " -> " + dst + " failed (error " + Int((long)::GetLastError()) + ")";
        return false;
    }
    const long long m = FtpLocalFileSize(dst);
    if (m != n)
    {
        *detail = "copy check: " + dst + " has " + Int((long)m) + " bytes, the source " + Int((long)n);
        return false;
    }
    return true;
}

N25Result UploadSummaryCount(const ElaUploadContext& c, const N25UploadSettings& s)
{
    N25Result r;
    r.gate = N25_4GateReason(s);
    if (!r.gate.empty()) return r;                                                      // D-f: no transport at all
    r.saveFolder = DatedFolder(c.logRoot, "SummaryCount", c.now);                       // golden :354-356
    EnsureDirectoriesExist(r.saveFolder);                                               // :357
    const std::string hadUpload = LocalCombine(r.saveFolder, "HadUpload.txt");          // :358-359
    if (FileExistsA(hadUpload))                                                         // :360-363
    {
        r.alreadyUploaded = true;
        return r;
    }
    std::vector<JamInterval> iv;
    int events = 0;
    const bool saved = SaveJamSummaryFor30Days(c, r.saveFolder, &r.jamFile, &iv, &events);   // :365
    for (size_t i = 0; i < iv.size(); ++i)
    {
        r.records += iv[i].iJamCount;
        r.devices += iv[i].iDeviceCount;
    }
    r.intervals = (int)iv.size();
    if (!saved)
    {
        FtpLog log(c.logRoot);
        if (events == 0)
        {
            r.sourceMissing = true;
            r.detail = "no event-log record in the 30 days: no Jam_Summary.csv, nothing sent (D-c, golden :95 would crash)";
            log.Add(c.now, "Upload Fail", "No event log N25-4", r.saveFolder, r.jamFile);
        }
        else
        {
            r.retryable = true;
            r.detail = "could not write " + LocalCombine(r.saveFolder, r.jamFile);
            log.Add(c.now, "Upload Fail", "File not exist N25-4", r.saveFolder, r.jamFile);
        }
        return r;
    }
    const bool ok = UploadMainAndBackup(c, "N25-4 Jam_Summary upload", "File not exist N25-4", s.user, s.password, s.host,
                                        r.saveFolder, s.path, r.jamFile, &r);           // :366 N25_UploadSummaryCountToFTP
    if (ok && r.logOnly)
    {
        r.detail = "log only: nothing sent, HadUpload.txt not written; " + r.detail;
        r.retryable = false;
    }
    else if (ok)
    {
        AppendLine(hadUpload, "HadUpload");                                             // :368 (the marker only)
        r.hadUploadWritten = FileExistsA(hadUpload);
    }
    return r;
}

N25Result UploadEventLog(const ElaUploadContext& c, const N25UploadSettings& s)
{
    N25Result r;
    r.gate = N25_5GateReason(s);
    if (!r.gate.empty()) return r;
    r.saveFolder = DatedFolder(c.logRoot, "UploadEventLog", c.now);                     // golden :374-376
    EnsureDirectoriesExist(r.saveFolder);                                               // :377
    const std::string hadUpload = LocalCombine(r.saveFolder, "HadUpload.txt");          // :378-379
    if (FileExistsA(hadUpload))                                                         // :380-383
    {
        r.alreadyUploaded = true;
        return r;
    }
    std::string source;
    if (!SaveEventLog(c, r.saveFolder, &r.jamFile, &source, &r.sourceMissing, &r.detail))   // :385
    {
        FtpLog log(c.logRoot);
        log.Add(c.now, "Upload Fail", "File not exist N25-5", source, r.detail);
        r.retryable = !r.sourceMissing;                                                 // a failed copy may work later
        return r;
    }
    const bool ok = UploadMainAndBackup(c, "N25-5 EventLog upload", "File not exist N25-5", s.user, s.password, s.host,
                                        r.saveFolder, s.path, r.jamFile, &r);           // :386 N25_UploadJamDataToFTP
    if (ok && r.logOnly)
    {
        r.detail = "log only: nothing sent, HadUpload.txt not written; " + r.detail;
        r.retryable = false;
    }
    else if (ok)
    {
        // golden :227 / :237 (inside N25_UploadJamDataToFTP) + :388; only now, both copies verified
        AppendLine(hadUpload, "Upload path:" + r.remoteMain);
        AppendLine(hadUpload, "Upload backup path:" + r.remoteBackup);
        AppendLine(hadUpload, "HadUpload");
        r.hadUploadWritten = FileExistsA(hadUpload);
    }
    return r;
}

// ===========================================================================
//  N10 BYFILE
// ===========================================================================
std::string N10ByFileGateReason(const N10Settings& s)
{
    if (!s.o10) return "O10 (O06-1 EnableAutoSaveEventLog) is off: golden runs no analyzer job then (#22 D-a)";
    if (s.uploadMethod != 0) return "N10 iN10UploadMethod is not 0 (FTP) (#22 D-f)";
    if (Trim(s.host).empty()) return "N10 FTP host is empty (#22 D-f)";
    return std::string();
}

bool N10_UploadDataToFTP(const ElaUploadContext& c, const N10Settings& s, const std::string& sourcesPath,
                         const std::string& targetPath, const std::string& sourcesFile, const std::string& targetFile,
                         N10Result* r)
{
    FtpLog log(c.logRoot);
    log.SetSecrets(s.user, s.password);
    log.AddUpload(c.now, sourcesPath, targetPath, sourcesFile, targetFile);            // golden :151
    FtpEndpoint ep;                                                                     // :148-150 (N10 account; the
    ep.host = s.host;                                                                   //   analyzer ignores the N10
    ep.user = s.user;                                                                   //   port / passive settings)
    ep.password = s.password;
    FtpSessionLock one;
    if (c.slot && c.slot->Stopping()) return false;
    IElaFtp* f = (c.factory ? c.factory : &NewNullFtp)();
    ++r->factoryCalls;
    if (IsLogOnly(f)) r->logOnly = true;
    if (c.slot) c.slot->Set(f);
    bool ok = false;
    const FtpStatus cs = f->Connect(ep);                                                // :152-153
    if (!cs.ok)
    {
        log.Add(c.now, "FTP Failure", "Connection Failed", ep.host, MaskSecrets(cs.text, s.user, s.password));   // :178-181
        ++r->failed;
    }
    else
    {
        EnsureDirPath(f, targetPath, &log, c.now);                                      // :158 ChangeDirectories
        const std::string local = LocalCombine(sourcesPath, sourcesFile);
        if (FtpLocalFileSize(local) >= 0)                                               // :159
        {
            const UploadVerify v = UploadAndVerify(f, local, targetPath, targetFile, &log, c.now);   // :161
            if (v.ok)
            {
                ok = true;
                ++r->uploaded;
            }
            else
            {
                log.Add(c.now, "Upload Fail", "Upload Fail N10", sourcesPath, targetPath, sourcesFile, targetFile);   // :168
                ++r->failed;
            }
        }
        else
        {
            log.Add(c.now, "Upload Fail", "File not exist N10", sourcesPath, targetPath, sourcesFile, targetFile);   // :174
            ++r->missing;
        }
    }
    if (c.slot) c.slot->Set(0);
    f->Close();
    delete f;
    return ok;
}

N10Result UploadByFile_N10(const ElaUploadContext& c, const N10Settings& s)
{
    N10Result r;
    r.gate = N10ByFileGateReason(s);
    if (!r.gate.empty()) return r;
    const std::string list = c.logRoot + "\\UploadFile\\UploadFile.csv";               // golden :396
    bool ok = false;
    const std::vector<std::string> lines = BcbLoadLines(list, &ok);                    // :398-400
    r.listFound = ok;
    if (!ok) return r;
    for (size_t i = 1; i < lines.size(); ++i)                                           // :402 (row 0 = header)
    {
        // golden :404 TStringList CommaText; W15 B (the Hub's options) = commas only, a quoted field is one field
        const Row fl = c.opt.bcbCommaText ? BcbCommaText(lines[i]) : SplitEventLogCsv(lines[i]);
        if (fl.size() != 4) continue;                                                   // :405
        ++r.rows;
        N10_UploadDataToFTP(c, s, fl[0], fl[1], fl[2], fl[3], &r);                      // :407
        if (c.slot && c.slot->Stopping()) break;
    }
    r.detail = Int(r.rows) + " row(s): " + Int(r.uploaded) + " verified, " + Int(r.failed) + " failed, " + Int(r.missing) +
               " local file(s) missing" + (r.logOnly ? " (log only: nothing sent)" : "");
    return r;
}

// ===========================================================================
//  Hub jobs (ElaHub.h)
// ===========================================================================
void Hub::RunUploadJob(int job)
{
    ElaConfig c;
    QueryRequest ui;
    {
        webbridge::WbGuard g(mu_);
        c = config_;
        ui = base_;                                   // golden labPath: the page's folder (R2's page state)
    }
    ElaUploadContext x;
    x.opt = opt_;
    x.opt.custCode = c.sCustCode;
    x.logRoot = ElaLogRoot();
    x.eventLogDir = ui.eventLogDir;                   // default W906_EVENTLOG_ROOT / D:\HT9045_Log\EventLogTxt
    x.prodLogDir = ui.prodLogDir;                     // W22: default W906_PRODLOG_ROOT / D:\HT9045_Log\Production_Log
    x.hostName = ElaHostName();
    x.machineId = c.edN04_ID;
    x.now = clock_->Now();                            // R2's Clock (Hub::SetClock; the system clock by default)
    x.factory = ftpFactory_;
    x.slot = &ftpSlot_;
    const std::string name = CommandName(job);
    if (job == EL_UPLOAD_JAMWEEK)
    {
        N25Settings s;
        s.o10 = AutoJobsEnabled(c);                   // D-a, the same gate as R2's jobs
        s.custCode = c.sCustCode;
        // the Handler's IniConfig.bN25_3_EnableULJamLog (the analyzer never read it); read-only, no write-back
        s.enableJamLog = IniCheckAndReadBool(paths_.configIni, "ChipMos Function", "bN25_3_EnableULJamLog", false, false, 0);
        s.user = c.edtN25_2_Name;
        s.password = c.edtN25_2_Password;
        s.host = c.edtN25_2_Host;
        s.jamLogPath = c.edtN25_3_LogJamPath;
        const N25Result r = UploadJamCode(x, s);
        if (!r.gate.empty()) AddHistory(name + ": skipped, " + r.gate);
        else if (r.alreadyUploaded) AddHistory(name + ": " + r.saveFolder + "\\HadUpload.txt is there (golden: once a day)");
        else if (r.ok())
        {
            OnUploadSucceeded(&n25Retry_);
            AddHistory(name + ": " + Int(r.records) + " JAM row(s), uploaded and verified (" + r.detail + ")");
        }
        else if (r.logOnly) AddHistory(name + ": " + Int(r.records) + " JAM row(s), " + r.detail);
        else
        {
            if (r.retryable) OnUploadFailed(&n25Retry_, x.now, x.machineId, NextN25Slot(x.now));
            AddHistory(name + ": FAILED (" + r.detail + ")" +
                       (r.retryable && !n25Retry_.gaveUp ? ", retry due " + FmtDT(n25Retry_.nextTry) + " (R5 schedules it)"
                                                        : std::string(", no retry")));
        }
    }
    else if (job == EL_UPLOAD_SUMMARY || job == EL_UPLOAD_EVENTLOG)
    {
        // AI(W906-ELA-W22) 20260928 (St02-E): N25-4 / N25-5.  The switches are the Handler's IniConfig.bN25_4_EnableUpload
        // / bN25_5_EnableUpload (906_0625_Steven cConfiguration.cpp:3745 / :3747, default 0), read-only; no retry state
        // here (the schedule, ElaSchedule R5, does the back-off of the timed runs)
        const bool summary = job == EL_UPLOAD_SUMMARY;
        N25UploadSettings s;
        s.o10 = AutoJobsEnabled(c);
        s.custCode = c.sCustCode;
        s.enable = IniCheckAndReadBool(paths_.configIni, "ChipMos Function",
                                       summary ? "bN25_4_EnableUpload" : "bN25_5_EnableUpload", false, false, 0);
        s.user = c.edtN25_2_Name;
        s.password = c.edtN25_2_Password;
        s.host = c.edtN25_2_Host;
        s.path = summary ? c.edtN25_4_UploadPath : c.edtN25_5_UploadPath;   // Rev891 Analyzer.cpp:3126 / :3127
        const N25Result r = summary ? UploadSummaryCount(x, s) : UploadEventLog(x, s);
        const std::string what = summary ? Int(r.intervals) + " interval(s), " + Int((long)r.devices) + " device(s), " +
                                               Int(r.records) + " JAM"
                                         : std::string("EventLog.txt");
        if (!r.gate.empty()) AddHistory(name + ": skipped, " + r.gate);
        else if (r.alreadyUploaded) AddHistory(name + ": " + r.saveFolder + "\\HadUpload.txt is there (golden: once a day)");
        else if (r.sourceMissing) AddHistory(name + ": nothing to send, " + r.detail);
        else if (r.ok()) AddHistory(name + ": " + what + ", uploaded and verified (" + r.detail + ")");
        else if (r.logOnly) AddHistory(name + ": " + what + ", " + r.detail);
        else AddHistory(name + ": FAILED (" + r.detail + ")" + (r.retryable ? ", the schedule retries it" : ", no retry"));
    }
    else if (job == EL_UPLOAD_BYFILE_N10)
    {
        N10Settings s;
        s.o10 = AutoJobsEnabled(c);
        s.uploadMethod = c.rgN10_4;
        s.user = c.edN10UserName;
        s.password = c.edN10Password;
        s.host = c.edN10Host;
        const N10Result r = UploadByFile_N10(x, s);
        if (!r.gate.empty()) AddHistory(name + ": skipped, " + r.gate);
        else if (!r.listFound) AddHistory(name + ": no " + x.logRoot + "\\UploadFile\\UploadFile.csv");
        else AddHistory(name + ": " + r.detail);
    }
}

}  // namespace ela
