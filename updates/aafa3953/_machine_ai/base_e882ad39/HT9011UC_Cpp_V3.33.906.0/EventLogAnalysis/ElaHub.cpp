// ===========================================================================
//  EventLogAnalysis/ElaHub.cpp -- see ElaHub.h.  AI(W906-ELA-P2) 20260927.
// ===========================================================================
#include "EventLogAnalysis/ElaHub.h"
#include "vclcompat/IniFiles.h"

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
      coO19_5(0)
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

QueryRequest::QueryRequest()
    : startDate(0), startTime(0), endDate(0), endTime(0),
      eventLogDir(EnvOr("W906_EVENTLOG_ROOT", "D:\\HT9045_Log\\EventLogTxt")),
      prodLogDir(EnvOr("W906_PRODLOG_ROOT", "D:\\HT9045_Log\\Production_Log")), topFilter(0), rgFilter(0), top5Row(1)
{
    for (int i = 0; i < eUnitNameTotal; ++i) area[i] = true;
    for (int i = 0; i < eByFuncTotal; ++i) func[i] = true;
}

HubPaths::HubPaths()
    : configIni("D:\\HT9045\\config\\config.ini"),
      generalIni(EnvOr("W906_GENERAL_INI_PATH", "D:\\HT9045\\system\\Gerneral.ini")) {}

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
    : opt_(o), paths_(p), analyzer_(o), hasQuery_(false), configWrites_(0), seq_(0), busy_(false), stop_(false)
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
}

void Hub::AddHistory(const std::string& line)
{
    webbridge::WbGuard g(mu_);
    history_.push_back(NowHms() + " " + line);
    if (history_.size() > 100) history_.erase(history_.begin());
}

bool Hub::RunOnce()
{
    webbridge::WbGuard run(runMu_);
    // golden AnalysisThreadProcess (:154-196): the first raised flag in this order wins, is cleared, and runs
    static const int kOrder[EL_COMMAND_TOTAL] = { EL_UPDATE_PARAMETER, EL_UPLOAD_JAMWEEK, EL_UPLOAD_SUMMARY,
                                                  EL_UPLOAD_CHIPADV_LOTEND, EL_UPLOAD_EVENTLOG, EL_UPLOAD_BYFILE_N10,
                                                  EL_VTEST_MTBF_SUM };
    int job = -1;
    bool query = false;
    QueryRequest q;
    {
        webbridge::WbGuard g(mu_);
        for (int k = 0; k < EL_COMMAND_TOTAL && job < 0; ++k)
            if (flags_[kOrder[k]]) { flags_[kOrder[k]] = false; job = kOrder[k]; }
        if (job < 0 && hasQuery_) { q = query_; hasQuery_ = false; query = true; }
    }
    if (job < 0 && !query) return false;
    busy_.store(true);
    if (job == EL_UPDATE_PARAMETER)
    {
        ReadConfig();
        AddHistory(std::string(CommandName(job)) + ": ReadConfig done");
    }
    else if (job >= 0)
    {
        // plan §5 (customer reports, FTP via KYECFTP) is Jimmy's decision: accepted in golden order, not run
        AddHistory(std::string(CommandName(job)) + ": not ported (HOLD, ELA plan §5 / progress-st02 #22)");
    }
    else
    {
        RunQuery(q);
    }
    busy_.store(false);
    ++seq_;
    return true;
}

void Hub::ReadConfig()
{
    // golden TfrmELA::ReadConfig (:3090-3141), in golden order
    const std::string& f = paths_.configIni;
    const bool wb = opt_.jamWriteBack;   // #17 ELA-3 A covers config.ini too (golden writes missing keys)
    int w = 0;
    ElaConfig c;
    c.cbO06              = IniCheckAndReadBool(f, "Event Log", "EnableAutoSaveEventLog", false, wb, &w);
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

void Hub::RunQuery(const QueryRequest& q)
{
    Analyzer& a = analyzer_;
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
    const bool prodOk = a.ListProductionLog(prod);

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
    o += ",\"exceptions\":" + JArr(a.exceptions);
    o += "}";
    {
        webbridge::WbGuard g(mu_);
        snapshot_ = o;
    }
    AddHistory("query " + FormatYMD(a.dtStart) + " .. " + FormatYMD(a.dtEnd) + ": " + Num((long)a.filesRead.size()) +
               " event log file(s)" + (prodOk ? "" : ", Production_Log aborted"));
}

std::string Hub::SnapshotJson(unsigned long* seq) const
{
    webbridge::WbGuard g(mu_);
    if (seq) *seq = seq_.load();
    const ElaConfig& c = config_;
    std::string o = "{\"seq\":" + Num((long)seq_.load()) + ",\"busy\":" + B(busy_.load());
    o += ",\"history\":" + JArr(history_);
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
        h->RunOnce();
        ::SleepEx(500, TRUE);
    } while (!h->stop_.load());
}

bool Hub::Start()
{
    stop_.store(false);
    return thread_.start(&Hub::Entry, this);
}

void Hub::Stop()
{
    if (!thread_.joinable()) return;
    stop_.store(true);
    thread_.join();
}

}  // namespace ela
