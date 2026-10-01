// ===========================================================================
//  EventLogAnalysis/ElaHub.cpp -- see ElaHub.h.  AI(W906-ELA-P2) 20260927.
// ===========================================================================
#include "EventLogAnalysis/ElaHub.h"
#include "EventLogAnalysis/ElaReports.h"   // DoVTestSaveSummary, AutoJobsEnabled, Clock (R2)
#include "vclcompat/IniFiles.h"
#include "EventLogAnalysis/ElaSchedule.h"   // AI(W906-ELA-R5) 20260927 (St02-E): R5 ElaSchedule wiring (ScheduleTick on the worker)
#include "EventLogAnalysis/ElaOee.h"        // AI(W906-ELA-W48) 20260928 (St02-E helper): the snapshot's "oee" (ComputeOee / OeeJson)
#include <cstdio>
#include <cstdlib>
#include <cstring>

using vclcompat::AnsiString;
using vclcompat::TIniFile;

namespace ela {

const char* CommandName(int cmd)
{
    static const char* const k[EL_COMMAND_TOTAL] = { "EL_UPDATE_PARAMETER", "EL_UPLOAD_JAMWEEK", "EL_UPLOAD_CHIPADV_LOTEND",
                                                     "EL_UPLOAD_SUMMARY", "EL_UPLOAD_EVENTLOG", "EL_UPLOAD_BYFILE_N10",
                                                     "EL_VTEST_MTBF_SUM" };
    return (cmd >= 0 && cmd < EL_COMMAND_TOTAL) ? k[cmd] : "?";
}

ElaConfig::ElaConfig()
    : cbO06(false), chkO06AlarmHistroy(false), chkO06AlarmStatist(false), chkO06Production(false),
      chkO06UseNetDrive(false), chkO06TimePeriod(false), cbO06TimePeriod(0), cbN10_1(false), cbN10_2(false),
      cbN10_3(false), rgN10_3_1(0), rgN10_4(0), cbN34(false), cbO19_1(false), cbO19_2(false), cbO19_4(false), coO19_3(0),
      coO19_5(0), o10Stored(false)
{
}

// AI(W906-ELA-P3) 20260927: the Handler's own redirect seams (unset / empty = the golden literal), read at construction:
//   W906_EVENTLOG_ROOT (cObserver.cpp :1304, the same EventLogTxt the Handler writes and its viewer reads),
//   W906_PRODLOG_ROOT (common.cpp :253 asTravelingLogPath), W906_GENERAL_INI_PATH (common.cpp :155 asGeneralPath).
//   config.ini has no seam; ctests inject HubPaths.
static std::string EnvOr(const char* name, const char* goldenLiteral)
{
    const char* e = std::getenv(name);
    return (e != 0 && *e != 0) ? std::string(e) : std::string(goldenLiteral);
}

std::string ElaConfigIniPath()   // ElaHub.h.  AI(W906-ELA-REV) 20260928 (St02-E)
{
    const char* e = std::getenv("W906_AUTH_PATH");
    return (e != 0 && *e != 0) ? std::string(e) + "config.ini" : std::string("D:\\HT9045\\config\\config.ini");
}

QueryRequest::QueryRequest()
    : startDate(0), startTime(0), endDate(0), endTime(0),
      eventLogDir(EnvOr("W906_EVENTLOG_ROOT", "D:\\HT9045_Log\\EventLogTxt")),
      prodLogDir(EnvOr("W906_PRODLOG_ROOT", "D:\\HT9045_Log\\Production_Log")), topFilter(0), rgFilter(0), top5Row(1),
      includeAlarmList(false)
{
    for (int i = 0; i < eUnitNameTotal; ++i) area[i] = true;
    for (int i = 0; i < eByFuncTotal; ++i) func[i] = true;
}

HubPaths::HubPaths()
    : configIni(ElaConfigIniPath()),   // AI(W906-ELA-REV) 20260928: the Handler's W906_AUTH_PATH seam
      generalIni(EnvOr("W906_GENERAL_INI_PATH", "D:\\HT9045\\system\\Gerneral.ini")),
      summaryDir(EnvOr("W906_RMS_ROOT", "D:\\RMS\\")),   // AI(W906-ELA-P7a) 20260928: golden Analyzer.cpp:2563
      timeDataDir(DefaultTimeDataDir()) {}                  // AI(W906-ELA-W48B) 20260928: ★W48-2 = B (ElaOee.h)

// ---------------------------------------------------------------------------
//  golden Common.cpp CheckAndReadIniData (bool :124, int :107, AnsiString :141)
// ---------------------------------------------------------------------------
static AnsiString A(const std::string& s) { return AnsiString(s.c_str()); }

bool IniCheckAndReadBool(const std::string& file, const std::string& group, const std::string& name, bool value,
                         bool writeBack, int* writes)
{
    TIniFile ini(A(file));
    if (!ini.ValueExists(A(group), A(name)))
    {
        if (writeBack) { ini.WriteBool(A(group), A(name), value); if (writes) ++*writes; }
        return value;
    }
    return ini.ReadBool(A(group), A(name), value);
}

int IniCheckAndReadInt(const std::string& file, const std::string& group, const std::string& name, int value,
                       bool writeBack, int* writes)
{
    TIniFile ini(A(file));
    if (!ini.ValueExists(A(group), A(name)))
    {
        if (writeBack) { ini.WriteInteger(A(group), A(name), value); if (writes) ++*writes; }
        return value;
    }
    return ini.ReadInteger(A(group), A(name), value);
}

std::string IniCheckAndReadString(const std::string& file, const std::string& group, const std::string& name,
                                  const std::string& value, bool writeBack, int* writes)
{
    TIniFile ini(A(file));
    if (!ini.ValueExists(A(group), A(name)))
    {
        if (writeBack) { ini.WriteString(A(group), A(name), A(value)); if (writes) ++*writes; }
        return value;                                                       //JerryYang 20170711 (Steven)
    }
    std::string str = ini.ReadString(A(group), A(name), A(value)).c_str();
    if (str == "" && value != "")                                           //Steven 20160323 : Fixed when value is NULL
    {
        str = value;
        if (writeBack) { ini.WriteString(A(group), A(name), A(value)); if (writes) ++*writes; }
    }
    return str;
}

// ---------------------------------------------------------------------------
//  UTF-8 for the snapshot
// ---------------------------------------------------------------------------
static bool IsValidUtf8(const std::string& s)
{
    size_t i = 0;
    while (i < s.size())
    {
        const unsigned char c = (unsigned char)s[i];
        int n = 0;
        if (c < 0x80) n = 0;
        else if ((c & 0xE0) == 0xC0 && c >= 0xC2) n = 1;
        else if ((c & 0xF0) == 0xE0) n = 2;
        else if ((c & 0xF8) == 0xF0 && c <= 0xF4) n = 3;
        else return false;
        for (int k = 1; k <= n; ++k)
            if (i + k >= s.size() || ((unsigned char)s[i + k] & 0xC0) != 0x80) return false;
        i += n + 1;
    }
    return true;
}

std::string ToUtf8(const std::string& s)
{
    if (IsValidUtf8(s)) return s;
    const int wn = ::MultiByteToWideChar(950, 0, s.data(), (int)s.size(), NULL, 0);
    if (wn <= 0) return std::string();
    std::vector<wchar_t> w((size_t)wn);
    ::MultiByteToWideChar(950, 0, s.data(), (int)s.size(), &w[0], wn);
    const int un = ::WideCharToMultiByte(CP_UTF8, 0, &w[0], wn, NULL, 0, NULL, NULL);
    if (un <= 0) return std::string();
    std::string out((size_t)un, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, &w[0], wn, &out[0], un, NULL, NULL);
    return out;
}

static std::string J(const std::string& raw)
{
    const std::string s = ToUtf8(raw);
    std::string o = "\"";
    for (size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = (unsigned char)s[i];
        if (c == '"') o += "\\\"";
        else if (c == '\\') o += "\\\\";
        else if (c < 0x20)
        {
            char b[8];
            std::snprintf(b, sizeof(b), "\\u%04x", c);
            o += b;
        }
        else o += (char)c;
    }
    return o + "\"";
}

static std::string JArr(const std::vector<std::string>& v)
{
    std::string o = "[";
    for (size_t i = 0; i < v.size(); ++i) { if (i) o += ','; o += J(v[i]); }
    return o + "]";
}

static const size_t kMaxRows = 5000;   // per grid in the snapshot; "<name>Truncated":true when cut

static std::string JGrid(const char* name, const std::vector<Row>& g)
{
    std::string o = "\"";
    o += name;
    o += "\":[";
    const size_t n = g.size() < kMaxRows ? g.size() : kMaxRows;
    for (size_t i = 0; i < n; ++i) { if (i) o += ','; o += JArr(g[i]); }
    o += "],\"";
    o += name;
    o += "Truncated\":";
    o += (g.size() > kMaxRows) ? "true" : "false";
    return o;
}

static std::string Num(long v) { char b[24]; std::snprintf(b, sizeof(b), "%ld", v); return b; }
static const char* B(bool v) { return v ? "true" : "false"; }

static std::string NowHms()
{
    SYSTEMTIME t;
    ::GetLocalTime(&t);
    char b[32];
    std::snprintf(b, sizeof(b), "%04d/%02d/%02d %02d:%02d:%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    return b;
}

// ===========================================================================
//  Hub
// ===========================================================================
Hub::Hub(const Options& o, const HubPaths& p)
    : opt_(o), paths_(p), analyzer_(o), hasQuery_(false), configWrites_(0), seq_(0), busy_(false), stop_(false),
      clock_(&DefaultClock())
{
    for (int i = 0; i < EL_COMMAND_TOTAL; ++i) flags_[i] = false;   // golden ClearAllActive misses BYFILE_N10 (zero-filled in BCB)
}

Hub::~Hub()
{
    Stop();
}

void Hub::Post(int cmd)
{
    if (cmd < 0 || cmd >= EL_COMMAND_TOTAL) return;
    webbridge::WbGuard g(mu_);
    flags_[cmd] = true;
}

void Hub::PostQuery(const QueryRequest& q)
{
    webbridge::WbGuard g(mu_);
    query_ = q;
    hasQuery_ = true;
    base_ = q;                               // golden: the pickers / folders / filters stay as the page left them
}

void Hub::SetClock(const Clock* c)
{
    webbridge::WbGuard run(runMu_);
    clock_ = c ? c : &DefaultClock();
}

void Hub::AddHistory(const std::string& line)
{
    webbridge::WbGuard g(mu_);
    history_.push_back(NowHms() + " " + line);
    if (history_.size() > 100) history_.erase(history_.begin());
}

void Hub::AddJob(const JobRecord& jIn)
{
    JobRecord j = jIn;
    j.at = NowHms();
    webbridge::WbGuard g(mu_);
    jobs_.push_back(j);
    if (jobs_.size() > 50) jobs_.erase(jobs_.begin());
}

static JobRecord JobOf(int cmd, bool ran, const std::string& note)
{
    JobRecord j;
    j.cmd = CommandName(cmd);
    j.ran = ran;
    j.note = note;
    return j;
}

void Hub::RecordUploadJob(int job)
{
    // AI(W906-ELA-R2 follow-up) 20260927 (St02-E): every EL_* job leaves a job record.  RunUploadJob (R4, ElaChipMos.cpp)
    //   reports through the history; its last line (after "<name>: ") becomes the record's note, "skipped..." = not run.
    const std::string name = CommandName(job);
    std::string note;
    {
        webbridge::WbGuard g(mu_);
        const std::string tag = name + ": ";
        if (!history_.empty())
        {
            const size_t k = history_.back().find(tag);
            if (k != std::string::npos) note = history_.back().substr(k + tag.size());
        }
    }
    AddJob(JobOf(job, note.compare(0, 7, "skipped") != 0 && !note.empty(), note));
}

std::vector<JobRecord> Hub::Jobs() const
{
    webbridge::WbGuard g(mu_);
    return jobs_;
}

bool Hub::RunOnce()
{
    webbridge::WbGuard run(runMu_);
    // golden AnalysisThreadProcess (:154-196): the first raised flag in this order wins, is cleared, and runs
    static const int kOrder[EL_COMMAND_TOTAL] = { EL_UPDATE_PARAMETER, EL_UPLOAD_JAMWEEK, EL_UPLOAD_SUMMARY,
                                                  EL_UPLOAD_CHIPADV_LOTEND, EL_UPLOAD_EVENTLOG, EL_UPLOAD_BYFILE_N10,
                                                  EL_VTEST_MTBF_SUM };
    int job = -1;
    bool query = false, save = false;
    QueryRequest q;
    std::string saveName;
    {
        webbridge::WbGuard g(mu_);
        for (int k = 0; k < EL_COMMAND_TOTAL && job < 0; ++k)
            if (flags_[kOrder[k]]) { flags_[kOrder[k]] = false; job = kOrder[k]; }
        if (job < 0 && hasQuery_) { q = query_; hasQuery_ = false; query = true; }
        // AI(W906-ELA-P7a) 20260928: Save Summary after a pending query, so Query then Save saves that query
        else if (job < 0 && hasSave_) { saveName = saveName_; hasSave_ = false; save = true; }
    }
    if (job < 0 && !query && !save) return false;
    busy_.store(true);
    if (job == EL_UPDATE_PARAMETER)
    {
        ReadConfig();  ScheduleReloadConfig();   // AI(W906-ELA-W44-1) 20260928 (St02-E helper): re-base the N10-3 schedule at once on EL_UPDATE_PARAMETER (the Configuration save) instead of within 60 s
        AddHistory(std::string(CommandName(job)) + ": ReadConfig done");
        AddJob(JobOf(job, true, "ReadConfig done"));
    }
    else if (job == EL_UPLOAD_JAMWEEK || job == EL_UPLOAD_BYFILE_N10 || job == EL_UPLOAD_SUMMARY || job == EL_UPLOAD_EVENTLOG)
    {
        RunUploadJob(job);   // AI(W906-ELA-R4) 20260927 (St02-E): N25-3 / N10 BYFILE on IElaFtp (ElaChipMos.cpp); W22: N25-4 / N25-5
        RecordUploadJob(job);
    }
    else if (job == EL_VTEST_MTBF_SUM)
    {
        RunVTest();                          // AI(W906-ELA-R2) 20260927: the HOLD is lifted (#22 ruled, plan R2)
    }
    else if (job == EL_UPLOAD_CHIPADV_LOTEND)
    {
        RunChipAdv();                        // AI(W906-ELA-R3) 20260927: the HOLD is lifted (#22 ruled, plan R3)
    }
    else if (job >= 0)
    {
        // AI(W906-ELA-W22) 20260928 (St02-E): every EL_* command has a body now (W22 took the last two, N25-4 / N25-5);
        // kept for a command added later
        AddHistory(std::string(CommandName(job)) + ": no body for this command");
        AddJob(JobOf(job, false, "no body for this command"));
    }
    else if (save)
    {
        RunSaveSummary(saveName);            // AI(W906-ELA-P7a) 20260928
    }
    else
    {
        RunQuery(q);
    }
    busy_.store(false);
    ++seq_;
    return true;
}

void Hub::PostSaveSummary(const std::string& name)
{
    webbridge::WbGuard g(mu_);
    saveName_ = name;
    hasSave_ = true;
}

void Hub::RunSaveSummary(const std::string& name)
{
    // golden btnSaveSummaryClick (Rev891 Analyzer.cpp:2557-2570): SaveDialog1->FileName = "D:\\RMS\\" + yyyy-mm-dd; on
    //   Execute, SaveSummary(ExtractFilePath, ExtractFileName) = <folder><sHandlerID>-SummaryData_<name>.txt from the grids
    //   on screen (:2614-2688).  Here: the folder is HubPaths::summaryDir (the page sends only the name, no path), and
    //   the grids are the page's last query.  A report job (VTEST, ChipAdv) queries on analyzer_ as well, so when one ran
    //   after the page's query, that query is run again first (its files are re-read: rows written since are included).
    JobRecord j;
    j.at = NowHms();
    j.cmd = "SaveSummary";
    if (!hasUserQuery_)
    {
        j.note = "not written: no query yet (press Query first)";
        AddHistory("Save Summary: " + j.note);
        AddJob(j);
        return;
    }
    std::string again;
    if (!analyzerIsUser_)
    {
        BtnQuery(analyzer_, lastUserQuery_);
        analyzerIsUser_ = true;
        again = " (the page's query was run again first: a report job used the analyzer since)";
    }
    std::string full, err;
    SysDate sys;
    j.ran = true;
    j.written = SaveSummary(analyzer_, paths_.summaryDir, name, true, lastUserQuery_.includeAlarmList, *clock_, &full, &err,
                            &sys);
    j.path = full;
    j.hasRange = true;
    j.from = analyzer_.dtStart;
    j.to = analyzer_.dtEnd;
    j.note = (j.written ? "wrote " + full : "not written: " + err + " (" + full + ")") + again;
    AddHistory("Save Summary: " + j.note);
    AddJob(j);
}

void Hub::ReadConfig()
{
    // golden TfrmELA::ReadConfig (:3090-3141), in golden order
    const std::string& f = paths_.configIni;
    const bool wb = opt_.jamWriteBack;   // #17 ELA-3 A covers config.ini too (golden writes missing keys)
    int w = 0;
    ElaConfig c;
    c.cbO06              = IniCheckAndReadBool(f, "Event Log", "EnableAutoSaveEventLog", false, wb, &w);
    // AI(W906-ELA-R5) 20260927 (St02-E): the O10 box's stored key for EffectiveO10 (ElaReports.h); golden's analyzer does
    //   not read it, so no write-back (cConfiguration.cpp:3980 is the Handler's)
    c.o10Stored          = IniCheckAndReadBool(f, "Event Log", "bO10UseEventLogSaver", false, false, 0);
    c.chkO06AlarmHistroy = IniCheckAndReadBool(f, "Event Log", "EnableAutoSaveAlarmHistroy", false, wb, &w);
    c.chkO06AlarmStatist = IniCheckAndReadBool(f, "Event Log", "EnableAutoSaveAlarmStatist", false, wb, &w);
    c.chkO06Production   = IniCheckAndReadBool(f, "Event Log", "EnableAutoSaveProductiont", false, wb, &w);
    c.chkO06UseNetDrive  = IniCheckAndReadBool(f, "Event Log", "bAlarmStatistAutoSaveNetDrive", false, wb, &w);
    c.chkO06TimePeriod   = IniCheckAndReadBool(f, "Event Log", "EnanleTimePeriodSaveLog", false, wb, &w);
    c.cbO06TimePeriod    = IniCheckAndReadInt(f, "Event Log", "TimePeriodSaveLog", 0, wb, &w);

    c.edO06_FilePath     = IniCheckAndReadString(f, "Event Log", "AutoSaveEventLogPath", "D:\\RMS", wb, &w);
    c.edtO06AlarmHistroy = IniCheckAndReadString(f, "Event Log", "AutoSaveAlarmHistroyPath", "D:\\RMS", wb, &w);
    c.edtO06AlarmStatist = IniCheckAndReadString(f, "Event Log", "AutoSaveAlarmStatistPath", "D:\\RMS", wb, &w);
    c.edtO06Production   = IniCheckAndReadString(f, "Event Log", "AutoSaveProductionPath", "D:\\RMS", wb, &w);

    c.edtO06_Remote      = IniCheckAndReadString(f, "Event Log", "asAlarmRemoteDirectory", "\\\\NET_DRVE", wb, &w);
    c.edtO06_Local       = IniCheckAndReadString(f, "Event Log", "asAlarmLocalDirectory", "Z:", wb, &w);

    c.cbN10_1            = IniCheckAndReadBool(f, "FTPUpLoad", "bEnable_FTPUpLoadLog", false, wb, &w);
    c.cbN10_2            = IniCheckAndReadBool(f, "FTPUpLoad", "bN10_UploadSummaryToFTP", false, wb, &w);
    c.cbN10_3            = IniCheckAndReadBool(f, "FTPUpLoad", "bN10_DailyUploadProdData", true, wb, &w);
    c.rgN10_3_1          = IniCheckAndReadInt(f, "FTPUpLoad", "iN10UploadProductMethod", 1, wb, &w);
    c.rgN10_4            = IniCheckAndReadInt(f, "FTPUpLoad", "iN10UploadMethod", 1, wb, &w);

    c.edN10UserName      = IniCheckAndReadString(f, "FTPUpLoad", "cN10FtpUserName", "", wb, &w);
    c.edN10Password      = IniCheckAndReadString(f, "FTPUpLoad", "cN10FtpPassword", "", wb, &w);
    c.edN10Host          = IniCheckAndReadString(f, "FTPUpLoad", "cN10FtpHost", "", wb, &w);
    c.edN10UploadPath    = IniCheckAndReadString(f, "FTPUpLoad", "cN10FtpUplaodPath", "\\", wb, &w);
    c.edtN10_6           = Num(IniCheckAndReadInt(f, "FTPUpLoad", "iUploadToHostIntervalTime", 5, wb, &w));
    c.edtN10_8           = IniCheckAndReadString(f, "FTPUpLoad", "sN10UploadDrivePath", c.edO06_FilePath, wb, &w);

    c.edtN25_2_Name      = IniCheckAndReadString(f, "ChipMos Function", "sN25_2_FTPUserName", "handler", wb, &w);
    c.edtN25_2_Password  = IniCheckAndReadString(f, "ChipMos Function", "sN25_2_FTPPassword", "handler", wb, &w);
    c.edtN25_2_Host      = IniCheckAndReadString(f, "ChipMos Function", "sN25_2_FTPHost", "10.20.50.3", wb, &w);
    c.edtN25_3_LogJamPath = IniCheckAndReadString(f, "ChipMos Function", "sN25_3_JamLogFTPPath", "/Summary/naslfs2/Handler/", wb, &w);
    c.edtN25_4_UploadPath = IniCheckAndReadString(f, "ChipMos Function", "sN25_4_UploadPath", "/Summary/naslfs2/Handler/", wb, &w);
    c.edtN25_5_UploadPath = IniCheckAndReadString(f, "ChipMos Function", "sN25_5_UploadPath", "/Summary/naslfs2/Handler/", wb, &w);
    c.edN04_ID           = IniCheckAndReadString(paths_.generalIni, "Version", "Machine ID", "29828", wb, &w);

    c.cbN34              = IniCheckAndReadBool(f, "N34 Function", "bN34_GenerateOEEAlarmRpt", false, wb, &w);
    c.edN34              = IniCheckAndReadString(f, "N34 Function", "sN34_OEEAlarmRptPath",
                                                 "D:\\HT9045_Log\\Product_Loader\\OEEAlarmRpt", wb, &w);

    c.cbO19_1            = IniCheckAndReadBool(f, "Event Log", "bO19_AutoRecordReportByEveryDay", false, wb, &w);
    c.cbO19_2            = IniCheckAndReadBool(f, "Event Log", "bO19_AutoRecordReportByEveryWeek", false, wb, &w);
    c.coO19_3            = IniCheckAndReadInt(f, "Event Log", "iO19_WeekPeriod", 1, wb, &w);
    c.cbO19_4            = IniCheckAndReadBool(f, "Event Log", "bO19_AutoRecordReportByEveryMonth", false, wb, &w);
    c.coO19_5            = IniCheckAndReadInt(f, "Event Log", "iO19_MonthPeriod", 0, wb, &w);
    c.edtO19_6           = IniCheckAndReadString(f, "Event Log", "asO19_SavePath", "D:\\MTBF_Summary", wb, &w);
    c.sCustCode          = IniCheckAndReadString(paths_.generalIni, "System", "CUSTOMER_CODE", "000", wb, &w);

    {
        webbridge::WbGuard g(mu_);
        config_ = c;
        configWrites_ += w;
    }
    analyzer_.SetCustCode(c.sCustCode);   // golden sCustCode drives GetEventLogText's VTEST (915 / 919) file-name rule
}

bool BtnQuery(Analyzer& a, const QueryRequest& q)
{
    // golden btnQueryClick (:2884-2895): SetStringGrid (with ImportProductionLod(2): both file lists) -> GetEventLogText
    //   (whose tail runs the four UpdateSg* with the UI state) -> ListProductionLog
    a.sHandlerID = q.handlerId;
    const std::vector<std::string> ev = LoadFavorite(q.eventLogDir);
    const std::vector<std::string> prod = LoadFavorite(q.prodLogDir);
    a.exceptions.clear();
    a.SetRange(q.startDate, q.startTime, q.endDate, q.endTime);
    a.GetEventLogText(ev);
    a.UpdateSgTop5Filter(q.topFilter);
    a.UpdateSgFailAndAlarm();
    a.UpdateSgTop5(q.top5Row);
    a.UpdateSgByFilter(q.rgFilter, q.area, q.func);
    return a.ListProductionLog(prod);
}

void Hub::RunQuery(const QueryRequest& q)
{
    const bool prodOk = BtnQuery(analyzer_, q);   // (AI(W906-ELA-R2): shared with the EL_* report jobs)
    lastUserQuery_ = q;                           // AI(W906-ELA-P7a) 20260928: what Save Summary saves
    hasUserQuery_ = true;
    analyzerIsUser_ = true;
    PublishSnapshot(prodOk);
    const Analyzer& a = analyzer_;
    AddHistory("query " + FormatYMD(a.dtStart) + " .. " + FormatYMD(a.dtEnd) + ": " + Num((long)a.filesRead.size()) +
               " event log file(s)" +
               (a.dupRowsSkipped ? ", " + Num(a.dupRowsSkipped) + " row(s) already counted from another file" : std::string()) +
               (prodOk ? "" : ", Production_Log aborted"));
}

void Hub::RunVTest()
{
    // golden AnalysisThreadProcess :191-195 -> DoVTestSaveSummary (ElaReports).  D-a (Steven 20260927): only with O10
    //   on -- golden's analyzer, and so this job, does not exist otherwise.
    ElaConfig c;
    QueryRequest ui;
    {
        webbridge::WbGuard g(mu_);
        c = config_;
        ui = base_;
    }
    const std::string name = CommandName(EL_VTEST_MTBF_SUM);
    if (!AutoJobsEnabled(c))
    {
        const std::string why = "skipped -- O10 is off (" + EffectiveO10Source() + ", D-a)";   // AI(W906-ELA-R5)
        AddHistory(name + ": " + why);
        AddJob(JobOf(EL_VTEST_MTBF_SUM, false, why));
        return;
    }
    const ReportResult r = DoVTestSaveSummary(analyzer_, c, ui, *clock_);
    if (r.queried) analyzerIsUser_ = false;       // AI(W906-ELA-P7a): analyzer_ now holds the job's range
    RecordReportJob(EL_VTEST_MTBF_SUM, r);
}

void Hub::RecordReportJob(int cmd, const ReportResult& r)
{
    // St02-E 20260927 (deviation): the job's query stays in its job record; the page's snapshot is not replaced (golden's
    //   window showed the job's range afterwards)
    JobRecord j = JobOf(cmd, r.ran, r.note);
    j.written = r.written;
    j.path = r.path;
    j.hasRange = r.hasRange;
    j.from = r.rangeFrom;
    j.to = r.rangeTo;
    AddJob(j);
    AddHistory(std::string(CommandName(cmd)) + ": " + r.note);
}

void Hub::RunChipAdv()
{
    // golden AnalysisThreadProcess :173-178 -> ChipAdvancedFunc chip; chip.AnalysisLog() (ElaReports, R3).  The analyzer
    //   checks no N34 switch (the Handler did, before sending); D-a: only with O10 on.  AI(W906-ELA-R3) 20260927
    ElaConfig c;
    QueryRequest ui;
    {
        webbridge::WbGuard g(mu_);
        c = config_;
        ui = base_;
    }
    if (!AutoJobsEnabled(c))
    {
        const std::string why = "skipped -- O10 is off (" + EffectiveO10Source() + ", D-a)";   // AI(W906-ELA-R5)
        AddHistory(std::string(CommandName(EL_UPLOAD_CHIPADV_LOTEND)) + ": " + why);
        AddJob(JobOf(EL_UPLOAD_CHIPADV_LOTEND, false, why));
        return;
    }
    ChipAdvancedFunc chip;
    const ReportResult r = chip.AnalysisLog(analyzer_, c, ui, *clock_);
    if (r.queried) analyzerIsUser_ = false;       // AI(W906-ELA-P7a)
    RecordReportJob(EL_UPLOAD_CHIPADV_LOTEND, r);
}

void Hub::PublishSnapshot(bool prodOk)
{
    const Analyzer& a = analyzer_;
    std::string o = "{";
    o += "\"queryAt\":" + J(NowHms());
    o += ",\"range\":[" + J(FormatYMD(a.dtStart) + " " + ConvertDTToTime(a.dtStart - (long long)a.dtStart)) + "," +
         J(FormatYMD(a.dtEnd) + " " + ConvertDTToTime(a.dtEnd - (long long)a.dtEnd)) + "]";
    o += ",\"iDate\":" + Num(a.iDate);
    o += ",\"productionLogOk\":" + std::string(B(prodOk));
    o += ",\"summary\":" + JArr(a.mmoSummary);
    o += "," + JGrid("byDay", a.sgByDay);
    o += "," + JGrid("byHour", a.sgByHour);
    o += "," + JGrid("top5Filter", a.sgTop5Filter);
    o += ",\"top5FilterRowCount\":" + Num(a.iTop5FilterRowCount);
    o += "," + JGrid("top5Alarm", a.sgTop5Alarm);
    o += "," + JGrid("top5", a.sgTop5);
    o += "," + JGrid("alarm", a.sgAlarm);
    o += "," + JGrid("fail", a.sgFail);
    o += "," + JGrid("byArea", a.sgByArea);
    o += ",\"byAreaCount\":" + Num(a.byAreaCount);
    o += ",\"filesRead\":" + JArr(a.filesRead);
    o += ",\"dupRowsSkipped\":" + Num(a.dupRowsSkipped);   // AI(W906-ELA-W19) 20260927: W19 B row de-dup
    o += ",\"exceptions\":" + JArr(a.exceptions);
    // AI(W906-ELA-W48) 20260928 (St02-E helper): ★W48 OEE (Steven 0928 = A) -- the OEE tab's per-day test / down / idle
    //   from this query's own lists (ElaOee.h; no second pass over them), cut at the report clock's Now() (Hub::SetClock).
    //   PublishSnapshot runs only from RunQuery, right after BtnQuery: user queries only, jobs never replace it.
    //   AI(W906-ELA-W48B) 20260928 (St02-E helper): the rules are Steven's now (0928 09:3x: W48-1 / W48-2 = B /
    //   W48-3 = B); W48-2 adds the off segment from the Handler's TimeData rows, read here (read only, a few
    //   thousand rows a year) for the query range.
    {
        std::vector<std::string> tdFiles;
        const std::vector<TimeDataRow> td = ReadTimeData(paths_.timeDataDir, a.dtStart, a.dtEnd, &tdFiles);
        o += ",\"oee\":" + OeeJson(ComputeOee(a, td, clock_->Now(), OeeOptions()), paths_.timeDataDir);
    }
    o += "}";
    {
        webbridge::WbGuard g(mu_);
        snapshot_ = o;
    }
}

static std::string JTime(double dt)
{
    return J(FormatYMD(dt) + " " + ConvertDTToTime(dt - (long long)dt));
}

static std::string JJob(const JobRecord& j)
{
    return "{\"at\":" + J(j.at) + ",\"cmd\":" + J(j.cmd) + ",\"ran\":" + B(j.ran) + ",\"written\":" + B(j.written) +
           ",\"path\":" + J(j.path) + ",\"range\":" + (j.hasRange ? "[" + JTime(j.from) + "," + JTime(j.to) + "]" : std::string("null")) +
           ",\"note\":" + J(j.note) + "}";
}

std::string Hub::SnapshotJson(unsigned long* seq) const
{
    webbridge::WbGuard g(mu_);
    if (seq) *seq = seq_.load();
    const ElaConfig& c = config_;
    std::string o = "{\"seq\":" + Num((long)seq_.load()) + ",\"busy\":" + B(busy_.load());
    o += ",\"history\":" + JArr(history_);
    o += ",\"jobs\":[";                                    // AI(W906-ELA-R2) 20260927: the EL_* job results (R6)
    for (size_t i = 0; i < jobs_.size(); ++i) { if (i) o += ','; o += JJob(jobs_[i]); }
    o += "]";
    // the configuration golden shows on its tabs; the two FTP passwords are left out on purpose
    o += ",\"config\":{\"O06\":" + std::string(B(c.cbO06)) + ",\"O06AlarmHistroy\":" + B(c.chkO06AlarmHistroy) +
         ",\"O06AlarmStatist\":" + B(c.chkO06AlarmStatist) + ",\"O06Production\":" + B(c.chkO06Production) +
         ",\"O06FilePath\":" + J(c.edO06_FilePath) + ",\"N10Enable\":" + B(c.cbN10_1) + ",\"N10Host\":" + J(c.edN10Host) +
         ",\"N25Host\":" + J(c.edtN25_2_Host) + ",\"N34\":" + B(c.cbN34) + ",\"N34Path\":" + J(c.edN34) +
         ",\"O19Week\":" + B(c.cbO19_2) + ",\"O19SavePath\":" + J(c.edtO19_6) + ",\"machineId\":" + J(c.edN04_ID) +
         ",\"custCode\":" + J(c.sCustCode) + "}";
    o += ",\"result\":" + (snapshot_.empty() ? std::string("null") : snapshot_);
    return o + "}";
}

ElaConfig Hub::Config() const
{
    webbridge::WbGuard g(mu_);
    return config_;
}

int Hub::ConfigWrites() const
{
    webbridge::WbGuard g(mu_);
    return configWrites_;
}

void Hub::Entry(void* self)
{
    Hub* h = static_cast<Hub*>(self);
    // golden TAnalysisProcessThread::Execute (:208-215): do { job; SleepEx(500, true); } while(!Terminated)
    do
    {
        h->RunOnce();   ela::ScheduleTick();   // AI(W906-ELA-R5) 20260927 (St02-E): R5 ElaSchedule wiring: the timed jobs run on this worker, every 500 ms (no-op when not installed)
        ::SleepEx(500, TRUE);
    } while (!h->stop_.load());
}

bool Hub::Start()
{
    stop_.store(false);
    ftpSlot_.Reset();     // AI(W906-ELA-R4) 20260927 (St02-E): a restarted worker may connect again
    return thread_.start(&Hub::Entry, this);
}

void Hub::Stop()
{
    if (!thread_.joinable()) return;
    stop_.store(true);
    ftpSlot_.Abort();     // AI(W906-ELA-R4) 20260927 (St02-E): cancel a transfer in flight (no 30 s wait at shutdown)
    thread_.join();
}

}  // namespace ela
