// ===========================================================================
//  EventLogAnalysis/ElaHub.h -- Event Log Analyzer commands + worker (ELA plan P2 / P3), no UI.
//  AI(W906-ELA-P2) 20260927.  Golden: EventlogAnalyzer Rev891.0 TAnalysisProcessThread (Analyzer.cpp:154-215),
//  OnMyCopyMsg (:487-558), ReadConfig (:3090-3141), btnQueryClick (:2884-2895).  Ledger docs/ELA_PORT_LEDGER.md.
//
//  Golden shape: the Handler's SendCommand_EventLog (Interface/InterfaceSYS.cpp) sends WM_COPYDATA to the analyzer
//  window; OnMyCopyMsg only raises bEL_<cmd> (repeats coalesce); TAnalysisProcessThread runs one raised job per
//  >= 500 ms in the priority order UPDATE_PARAMETER > UPLOAD_JAMWEEK > UPLOAD_SUMMARY > UPLOAD_CHIPADV_LOTEND >
//  UPLOAD_EVENTLOG > UPLOAD_BYFILE_N10 > VTEST_MTBF_SUM.  In golden that "thread" Synchronize()s everything onto the UI
//  thread; here the jobs really run on the worker (plan: the analysis never runs on the machine tick).
//
//  Here: Hub::Post(cmd) = OnMyCopyMsg; Hub::RunOnce() = one AnalysisThreadProcess pass; Hub::Start()/Stop() = the
//  worker (WbThread, SleepEx(500) between passes).  Hub::PostQuery() = the page's Query button (golden btnQueryClick);
//  a query runs when no EL job is raised; one pending query, a newer one replaces it.  The result is a JSON snapshot
//  (Hub::SnapshotJson) for the /api/ela route -- that route, eventlog.html and the InterfaceSYS wiring are shared files
//  and are HELD (github-59), so nothing calls this outside the ctest yet.
//
//  Ported:  EL_UPDATE_PARAMETER -> ReadConfig into ElaConfig (golden widget names).  CheckAndReadIniData's write-back of
//           missing keys into config.ini is #17 ELA-3 (暫照 golden（A）, Options::jamWriteBack also covers it).
//  HELD:    the six report / upload jobs (ChipMos N25 FTP, ChipAdvanced, N10, VTEST MTBF) -- plan §5 (customer-report
//           scope, FTP via KYECFTP) is Jimmy's decision.  They are accepted, run in golden order, and recorded as
//           "not ported" in the job history.
// ===========================================================================
#ifndef HT9045_ELA_ELAHUB_H
#define HT9045_ELA_ELAHUB_H

#include "EventLogAnalysis/ElaCore.h"
#include "WebBridge/Sync.h"

#include <atomic>
#include <string>
#include <vector>

namespace ela {

// golden Common.h:31-41 enum EventLog_COMMAND (the Handler sends these values)
enum EventLog_COMMAND
{
    EL_UPDATE_PARAMETER = 0, EL_UPLOAD_JAMWEEK = 1, EL_UPLOAD_CHIPADV_LOTEND = 2, EL_UPLOAD_SUMMARY = 3,
    EL_UPLOAD_EVENTLOG = 4, EL_UPLOAD_BYFILE_N10 = 5, EL_VTEST_MTBF_SUM = 6, EL_COMMAND_TOTAL
};
const char* CommandName(int cmd);

// golden ReadConfig (:3090-3141): the values it puts into the widgets, under the golden widget names
struct ElaConfig
{
    bool cbO06, chkO06AlarmHistroy, chkO06AlarmStatist, chkO06Production, chkO06UseNetDrive, chkO06TimePeriod;
    int cbO06TimePeriod;
    std::string edO06_FilePath, edtO06AlarmHistroy, edtO06AlarmStatist, edtO06Production, edtO06_Remote, edtO06_Local;
    bool cbN10_1, cbN10_2, cbN10_3;
    int rgN10_3_1, rgN10_4;
    std::string edN10UserName, edN10Password, edN10Host, edN10UploadPath, edtN10_6, edtN10_8;
    std::string edtN25_2_Name, edtN25_2_Password, edtN25_2_Host, edtN25_3_LogJamPath, edtN25_4_UploadPath,
        edtN25_5_UploadPath, edN04_ID;
    bool cbN34;
    std::string edN34;
    bool cbO19_1, cbO19_2, cbO19_4;
    int coO19_3, coO19_5;
    std::string edtO19_6;
    std::string sCustCode;
    ElaConfig();
};

// The page's query (golden: the date-time pickers, labPath / lblProdPath, cbTopAlarmFilter, rgFilter, the check boxes)
struct QueryRequest
{
    double startDate, startTime, endDate, endTime;
    std::string eventLogDir;   // golden labPath, default D:\HT9045_Log\EventLogTxt (W906_EVENTLOG_ROOT when set)
    std::string prodLogDir;    // golden lblProdPath (dirlstProdLog), default D:\HT9045_Log\Production_Log (W906_PRODLOG_ROOT)
    int topFilter;             // cbTopAlarmFilter 0 JAM / 1 WAR / 2 all
    int rgFilter;              // 0/1 By Area, 2 By Function
    bool area[eUnitNameTotal];
    bool func[eByFuncTotal];   // indexed by list (efRTC..efYield) -- see ElaCore.h about golden's crossed boxes
    int top5Row;               // UpdateSgTop5(ARow); golden's query uses 1
    std::string handlerId;
    QueryRequest();
};

// CheckAndReadIniData (golden Common.cpp:107-167): a missing key is written with the default when writeBack; the
// AnsiString form also rewrites a key whose value is "" while the default is not.  *writes counts the writes.
bool IniCheckAndReadBool(const std::string& file, const std::string& group, const std::string& name, bool value,
                         bool writeBack, int* writes);
int IniCheckAndReadInt(const std::string& file, const std::string& group, const std::string& name, int value,
                       bool writeBack, int* writes);
std::string IniCheckAndReadString(const std::string& file, const std::string& group, const std::string& name,
                                  const std::string& value, bool writeBack, int* writes);

// Text from BCB-written logs is cp950 (Big5); V906-written text is UTF-8.  Valid UTF-8 passes; otherwise cp950 -> UTF-8.
std::string ToUtf8(const std::string& s);

struct HubPaths
{
    std::string configIni;     // golden D:\HT9045\config\config.ini
    std::string generalIni;    // golden asGeneralPath D:\HT9045\system\Gerneral.ini (W906_GENERAL_INI_PATH when set)
    HubPaths();
};

class Hub
{
public:
    Hub(const Options& o, const HubPaths& p);
    ~Hub();

    void Post(int cmd);                      // golden OnMyCopyMsg: raise bEL_<cmd> (repeats coalesce); bad values ignored
    void PostQuery(const QueryRequest& q);   // the Query button; replaces a query that has not started yet
    bool RunOnce();                          // one golden AnalysisThreadProcess pass, else the pending query; false = idle

    bool Start();                            // the worker thread (RunOnce + SleepEx(500)), golden TAnalysisProcessThread
    void Stop();                             // stop and join

    std::string SnapshotJson(unsigned long* seq) const;   // last query result + state + job history (UTF-8 JSON)
    unsigned long Seq() const { return seq_.load(); }
    bool Busy() const { return busy_.load(); }
    ElaConfig Config() const;
    int ConfigWrites() const;

private:
    static void Entry(void* self);
    void ReadConfig();
    void RunQuery(const QueryRequest& q);
    void AddHistory(const std::string& line);

    Options opt_;
    HubPaths paths_;
    Analyzer analyzer_;                      // one per Hub, alive across queries like golden's single TfrmELA (#18 leaks)
    webbridge::WbMutex runMu_;               // RunOnce is not re-entrant: the worker and a direct caller never overlap
    mutable webbridge::WbMutex mu_;          // guards flags_, pending query, snapshot, history, config
    bool flags_[EL_COMMAND_TOTAL];
    bool hasQuery_;
    QueryRequest query_;
    std::string snapshot_;
    std::vector<std::string> history_;
    ElaConfig config_;
    int configWrites_;
    std::atomic<unsigned long> seq_;
    std::atomic<bool> busy_;
    std::atomic<bool> stop_;
    webbridge::WbThread thread_;
};

}  // namespace ela

#endif
