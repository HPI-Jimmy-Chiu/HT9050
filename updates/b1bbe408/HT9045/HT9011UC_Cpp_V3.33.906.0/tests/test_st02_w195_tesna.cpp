// ===========================================================================
//  tests/test_st02_w195_tesna.cpp -- W-195 (3) TESNA, golden 913 (RogerYang 20260707, the "Per month" event log).
//  AI(W906-W195) 20261009 (St02-E).  T1: TMyStringList::bDupYMDFile + the by-day duplicate write (Public/MyStringList.h:265 /
//  .cpp:270 / :295 / :654 + W906_SaveDupYMDFile at its end = golden 913 MyStringList.h:76, .cpp:34 / :58 / :409-432).
//
//  The objects are built with a %TEMP% scratch path, so nothing goes near D:\HT9045_Log; the test stops before any call when the
//  scratch path is a machine path (D:\HT9045... or D:\rms, unless it is ctest's machine_log_scratch -- the c59acf66 rule).
//    [1] TByMonth + bDupYMDFile: main <root>\YYYY\EventLogTxt_YYYYMM.csv and duplicate <root>\YYYY\MM\EventLogTxt_YYYYMMDD.csv
//        both "HDR\r\nROW1\r\n" (golden :410-430; the duplicate keeps CRLF).
//    [2] a second flush appends to both, the header written once.
//    [3] both constructors leave bDupYMDFile false; a default TByMonth object writes no YYYY\MM duplicate.
//    [4] FixedFile with the flag set: no duplicate (golden :410 HTSaveFixedFile==false).
//    [5] [W906] D7 (Steven 20261009, RULINGS_20261009 #11): TByDay + bDupYMDFile (the N10 + period 8 case) -- the duplicate IS the
//        main file; each row is in it once (golden 913 writes it twice).
//    [6] source pins (argv[1] = the tree root, read only): the guard on :654 before MyList->Clear(), the EOF body's golden formats.
//  T2 (same file): ProcessLastSetIni_EventLog with the GA1-B2 slEventLog gate lifted (golden 913 cprod.cpp:2416-2465 + :2464).
//    [7] slEventLog NULL -> no crash ([W906] guard).  [8] period 8 / 6 / 7, N10, O15-2 -> SaveType / bDupYMDFile / ByLot / FileName.
//    [9] period 8: SummarizeJAMreportbymonth under the log root (seam).  [10] source pins (cprod.cpp gate / guard / setter;
//        LogObjects.cpp re-apply after slEventLog is made).  T2 runs only under ctest's machine_log_scratch log root.
// ===========================================================================
#include "MachineDefine.h"
#include "Public/MyStringList.h"
#include "forms/fTemp_Set.h"        // the W-195 rule: create fTemp_Set first (see main)
#include "cmydef.h"                 // T2: slEventLog
#include "cprod.h"                  // T2: bWriteFile
#include "Config.h"                 // T2: IniConfig
#include "CosFunction.h"            // T2: CosFunction
#include "common.h"                 // T2: as9045LogPath

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

void ProcessLastSetIni_EventLog(bool bRead);   // cprod.cpp:2501 (no header declares it)

static int g_fail = 0, g_total = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}
static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
static bool Forbidden(const std::string& p)                  // machine paths, except ctest's redirect scratch (the c59acf66 rule)
{
    const std::string s = Lower(p);
    if (s.find("\\machine_log_scratch") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0 || s.compare(0, 6, "d:\\rms") == 0;
}
static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p); else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

struct Names { std::string month, day, dayDir; };
static Names NamesFor(const std::string& base, const SYSTEMTIME& t)
{
    char m[512], d[512], dd[512];
    std::snprintf(m, sizeof(m), "%s\\%04d\\EventLogTxt_%04d%02d.csv", base.c_str(), t.wYear, t.wYear, t.wMonth);
    std::snprintf(dd, sizeof(dd), "%s\\%04d\\%02d", base.c_str(), t.wYear, t.wMonth);
    std::snprintf(d, sizeof(d), "%s\\EventLogTxt_%04d%02d%02d.csv", dd, t.wYear, t.wMonth, t.wDay);
    Names n; n.month = m; n.day = d; n.dayDir = dd;
    return n;
}
// one flush of `rows`; returns false if the date changed while it ran (the object uses its own SystemYear/Month/Date)
static bool Flush(TMyStringList* sl, const char* row, Names& n, const std::string& base)
{
    SYSTEMTIME a, b;
    ::GetLocalTime(&a);
    sl->AddText(row);
    sl->MySaveToFile();
    ::GetLocalTime(&b);
    n = NamesFor(base, b);
    return a.wYear == b.wYear && a.wMonth == b.wMonth && a.wDay == b.wDay;
}

int main(int argc, char** argv)
{
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char tick[32];
    std::snprintf(tick, sizeof(tick), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_w195t_" + tick;
    if (Forbidden(root)) { std::printf("STOP: scratch %s is a machine path -- nothing called\n", root.c_str()); return 1; }
    ::CreateDirectoryA(root.c_str(), 0);
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // W-195 rule (laptop batch 136)

    std::printf("-- [1]/[2] TByMonth + bDupYMDFile --\n");
    {
        const std::string base = root + "\\M1\\EventLogTxt";
        Names n;
        bool sameDay = false;
        for (int tries = 0; tries < 3 && !sameDay; ++tries) {
            RemoveTree(root + "\\M1");
            TMyStringList* sl = new TMyStringList(AnsiString(base.c_str()), "EventLogTxt", "HDR");
            sl->SaveType = TByMonth;
            sl->bDupYMDFile = true;
            sameDay = Flush(sl, "ROW1", n, base);
            if (sameDay) {
                const std::string m = Read(n.month), d = Read(n.day);
                std::printf("     month %s = [%s]\n     day   %s = [%s]\n", n.month.c_str(), m.c_str(), n.day.c_str(), d.c_str());
                check(m == "HDR\r\nROW1\r\n" && d == "HDR\r\nROW1\r\n",
                      "[1] the month file and its by-day duplicate both hold the header + ROW1 (golden 913 MyStringList.cpp:410-430, CRLF kept)");
                Names n2;
                sameDay = Flush(sl, "ROW2", n2, base);
                if (sameDay)
                    check(Read(n.month) == "HDR\r\nROW1\r\nROW2\r\n" && Read(n.day) == "HDR\r\nROW1\r\nROW2\r\n",
                          "[2] a second flush appends to both, the header once (golden :418-421)");
            }
            delete sl;
        }
        if (!sameDay) check(false, "[1]/[2] the date kept changing during the flush (3 tries)");
    }

    std::printf("-- [3] defaults --\n");
    {
        TMyStringList* a = new TMyStringList();
        const std::string base = root + "\\M3\\EventLogTxt";
        TMyStringList* b = new TMyStringList(AnsiString(base.c_str()), "EventLogTxt", "HDR");
        check(a->bDupYMDFile == false && b->bDupYMDFile == false, "[3] both constructors: bDupYMDFile false (golden 913 :34 / :58)");
        b->SaveType = TByMonth;
        Names n;
        const bool sameDay = Flush(b, "ROW1", n, base);
        check(!sameDay || (Exists(n.month) && !Exists(n.dayDir)), "[3] TByMonth without the flag: the month file only, no YYYY\\MM duplicate");
        delete a;
        delete b;
    }

    std::printf("-- [4] FixedFile --\n");
    {
        const std::string base = root + "\\M4\\EventLogTxt";
        TMyStringList* sl = new TMyStringList(AnsiString(base.c_str()), "EventLogTxt", "HDR");
        sl->SaveType = TByMonth;
        sl->FixedFile = true;
        sl->bDupYMDFile = true;
        Names n;
        const bool sameDay = Flush(sl, "ROW1", n, base);
        check(!sameDay || (Exists(base + "\\EventLogTxt.csv") && !Exists(n.dayDir)),
              "[4] FixedFile + flag: the fixed file only, no duplicate (golden :410 HTSaveFixedFile==false)");
        delete sl;
    }

    std::printf("-- [5] [W906] D7: the duplicate is the main file (N10 + period 8) --\n");
    {
        const std::string base = root + "\\M5\\EventLogTxt";
        Names n;
        bool sameDay = false;
        for (int tries = 0; tries < 3 && !sameDay; ++tries) {
            RemoveTree(root + "\\M5");
            TMyStringList* sl = new TMyStringList(AnsiString(base.c_str()), "EventLogTxt", "HDR");
            sl->SaveType = TByDay;
            sl->bDupYMDFile = true;
            sameDay = Flush(sl, "ROW1", n, base);
            if (sameDay) {
                const std::string d = Read(n.day);
                std::printf("     day %s = [%s]\n", n.day.c_str(), d.c_str());
                check(d == "HDR\r\nROW1\r\n", "[5] TByDay + flag: the day file (= the duplicate path) holds ROW1 once (Steven D7; golden 913 writes it twice)");
            }
            delete sl;
        }
        if (!sameDay) check(false, "[5] the date kept changing during the flush (3 tries)");
    }

    std::printf("-- [6] source pins --\n");
    if (argc > 1) {
        const std::string c = Read(std::string(argv[1]) + "/Public/MyStringList.cpp");
        const size_t fn = c.find("\nvoid TMyStringList::MySaveToFileShareMode()");
        const size_t end = fn == std::string::npos ? std::string::npos : c.find("\n}", fn);
        const std::string body = (fn == std::string::npos || end == std::string::npos) ? "" : c.substr(fn, end - fn);
        const size_t guard = body.find("if(bDupYMDFile==true && HTSaveFixedFile==false) { W906_SaveDupYMDFile(); }   MyList->Clear();");
        check(guard != std::string::npos && body.find("MyList->Clear();", guard + 80) == std::string::npos,
              "[6] MySaveToFileShareMode: the duplicate call right before its final MyList->Clear() (golden 913 :410 / :433)");
        const size_t b0 = c.find("\nvoid TMyStringList::W906_SaveDupYMDFile()");
        check(b0 != std::string::npos && c.find("sDupPath.sprintf(\"%s\\\\%04d\\\\%02d\", HTPath, SystemYear, SystemMonth);", b0) != std::string::npos &&
              c.find("sDupFile.sprintf(\"%s\\\\%s_%04d%02d%02d.csv\", sDupPath, HTFileName, SystemYear, SystemMonth, SystemDate);", b0) != std::string::npos,
              "[6] W906_SaveDupYMDFile carries golden 913 :413 / :416's formats");
    } else
        check(false, "argv[1] = the tree root");

    // ---- T2 (AI(W906-W195) 20261009 (St02-E) TESNA T2): the lifted GA1-B2 slEventLog block of ProcessLastSetIni_EventLog
    //   (golden 913 cprod.cpp:2416-2465 + the :2464 setter), its [W906] null guard, the SummarizeJAMreportbymonth log-root seam
    //   (golden 913 :4002 / :4029) and LogObjects.cpp's boot re-apply.  [Event Log] ini I/O off (bEventLogAutoSaveFunction false);
    //   the JAM summary writes under as9045LogPath, which must be ctest's machine_log_scratch.
    std::printf("-- [7]-[10] T2: ProcessLastSetIni_EventLog --\n");
    if (Lower(as9045LogPath.c_str()).find("machine_log_scratch") == std::string::npos) {
        check(false, "[7] as9045LogPath is not ctest's machine_log_scratch -- T2 parts not run");
    } else {
        TMyStringList* const savSl = slEventLog;
        const bool savAuto = IniConfig.bEventLogAutoSaveFunction, savN10 = IniConfig.bN10_DailyUploadProdData;
        const bool savMid = IniConfig.bO15_EventLogFileNameWithMachineID, savSame = IniConfig.bO15_EventLogSaveSameFolder;
        const bool savHi = CosFunction.bHiSiliconFunction, savByLot = CosFunction.bSaveEventLogByLotID;
        const int savP = IniConfig.iO15_SaveFilePeriod, savMeth = IniConfig.iN10UploadProductMethod;
        const AnsiString savMT = IniConfig.sMachineType, savID = IniConfig.SocketHandlerID;
        IniConfig.bEventLogAutoSaveFunction = false; IniConfig.bN10_DailyUploadProdData = false; IniConfig.bO15_EventLogFileNameWithMachineID = false;
        IniConfig.bO15_EventLogSaveSameFolder = false; CosFunction.bHiSiliconFunction = false; CosFunction.bSaveEventLogByLotID = false;
        IniConfig.sMachineType = "HT9045"; IniConfig.SocketHandlerID = "H07";

        slEventLog = NULL;
        IniConfig.iO15_SaveFilePeriod = 6;
        ProcessLastSetIni_EventLog(bWriteFile);
        check(true, "[7] slEventLog NULL (wb_serve reads config before W906_CreateLogObjects): no crash ([W906] guard on golden 913 :2416)");

        slEventLog = new TMyStringList(AnsiString((root + "\\T2\\EventLogTxt").c_str()), "EventLogTxt", "HDR");
        IniConfig.iO15_SaveFilePeriod = 8;
        ProcessLastSetIni_EventLog(bWriteFile);
        const bool p8 = slEventLog->SaveType == TByMonth && slEventLog->bDupYMDFile && slEventLog->FileName == "EventLogTxt";
        IniConfig.iO15_SaveFilePeriod = 6;
        ProcessLastSetIni_EventLog(bWriteFile);
        const bool p6 = slEventLog->SaveType == TByDay && !slEventLog->bDupYMDFile && !slEventLog->SaveByLotID;
        IniConfig.iO15_SaveFilePeriod = 7;
        ProcessLastSetIni_EventLog(bWriteFile);
        const bool p7 = slEventLog->SaveType == TByDay && slEventLog->SaveByLotID && !slEventLog->bDupYMDFile;
        check(p8 && p6 && p7, "[8] period 8 -> TByMonth + bDupYMDFile (golden 913 :2451-2452 / :2464); 6 -> TByDay, no duplicate; 7 -> ByLot (:2460-2462)");
        IniConfig.iO15_SaveFilePeriod = 8; IniConfig.bN10_DailyUploadProdData = true; IniConfig.iN10UploadProductMethod = 0;
        ProcessLastSetIni_EventLog(bWriteFile);
        const bool n10 = slEventLog->FileName == "HT9045_H07_EventLogTxt" && slEventLog->SaveType == TByDay && slEventLog->bDupYMDFile;
        IniConfig.bN10_DailyUploadProdData = false; IniConfig.bO15_EventLogFileNameWithMachineID = true; IniConfig.iO15_SaveFilePeriod = 6;
        ProcessLastSetIni_EventLog(bWriteFile);
        const bool mid = slEventLog->FileName == "EventLogTxt_H07";
        check(n10 && mid, "[8] N10 -> <MT>_<ID>_EventLogTxt + TByDay (with period 8 still flagged: the D7 case, [5]); O15-2 -> EventLogTxt_<ID> (golden :2418-2424)");

        // [9] the JAM month summary under the log root (seam)
        SYSTEMTIME t;
        ::GetLocalTime(&t);
        char yd[32], mf[64], jd[64];
        std::snprintf(yd, sizeof(yd), "%04d", t.wYear);
        std::snprintf(mf, sizeof(mf), "EventLogTxt_%04d%02d.csv", t.wYear, t.wMonth);
        std::snprintf(jd, sizeof(jd), "%4d\\JAM_Log\\%02d", t.wYear, t.wMonth);
        const std::string elRoot = std::string(as9045LogPath.c_str()) + "\\EventLogTxt";
        ::CreateDirectoryA(elRoot.c_str(), 0);
        ::CreateDirectoryA((elRoot + "\\" + yd).c_str(), 0);
        {
            std::ofstream f((elRoot + "\\" + yd + "\\" + mf).c_str(), std::ios::binary);
            f << "Date,Time,UnitName,AlarmCode\r\n2026/10/09,20:00:00,Index,JAM001\r\n2026/10/09,20:01:00,Index,WAR002\r\n";
        }
        IniConfig.bO15_EventLogFileNameWithMachineID = false; IniConfig.iO15_SaveFilePeriod = 8;
        ProcessLastSetIni_EventLog(bWriteFile);
        const std::string jam = Read(elRoot + "\\" + jd + "\\Processed_JAM.csv");
        std::printf("     Processed_JAM.csv = [%s]\n", jam.c_str());
        check(jam.find("Date,Time,UnitName,AlarmCode") == 0 && jam.find("JAM001") != std::string::npos && jam.find("WAR002") == std::string::npos,
              "[9] period 8: SummarizeJAMreportbymonth writes header + JAM rows under the log root ([W906] seam on golden 913 :4002 / :4029)");

        delete slEventLog;
        slEventLog = savSl;
        IniConfig.bEventLogAutoSaveFunction = savAuto; IniConfig.bN10_DailyUploadProdData = savN10; IniConfig.bO15_EventLogFileNameWithMachineID = savMid;
        IniConfig.bO15_EventLogSaveSameFolder = savSame; CosFunction.bHiSiliconFunction = savHi; CosFunction.bSaveEventLogByLotID = savByLot;
        IniConfig.iO15_SaveFilePeriod = savP; IniConfig.iN10UploadProductMethod = savMeth;
        IniConfig.sMachineType = savMT; IniConfig.SocketHandlerID = savID;
    }
    if (argc > 1) {
        const std::string c = Read(std::string(argv[1]) + "/cprod.cpp");
        const std::string lo = Read(std::string(argv[1]) + "/LogObjects.cpp");
        const size_t mk = lo.find("\nvoid W906_CreateLogObjects()");
        const size_t el = lo.find("slEventLog=new TMyStringList(", mk == std::string::npos ? 0 : mk);
        const size_t re = lo.find("ProcessLastSetIni_EventLog(bReadFile); }", mk == std::string::npos ? 0 : mk);
        const size_t en = mk == std::string::npos ? std::string::npos : lo.find("\n}", mk);
        check(c.find("\n#if 0 // TODO(GA1-B2): blocked by TMyStringList") == std::string::npos &&   // at a line start (:2524's comment quotes it)
              c.find("    if(fMain!=NULL && slEventLog!=NULL)") != std::string::npos &&
              c.find("        slEventLog->bDupYMDFile = (IniConfig.iO15_SaveFilePeriod==8);") != std::string::npos,
              "[10] cprod.cpp: the GA1-B2 slEventLog gate lifted, the null guard, the golden 913 :2464 setter");
        check(mk != std::string::npos && el != std::string::npos && re != std::string::npos && el < re && re < en,
              "[10] LogObjects.cpp W906_CreateLogObjects: [Event Log] re-applied after slEventLog is made (golden: ctor, then FormShow's ReadLastSetIni)");
    }

    RemoveTree(root);
    std::printf("test_st02_w195_tesna: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
