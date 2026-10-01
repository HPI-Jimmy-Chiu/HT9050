// ===========================================================================
//  SimNet/SimNetMask.cpp -- see SimNetMask.h.
//  AI(W906-SIM-W36-1) 20260928 (St02-E helper) prototype; AI(W906-W58) 20260930 (St02-E): Steven's W58 answers
//  (decisions-decided.md:2779-2812) -- Q1 = 要, Q2 = 要, Q3 = 關 + enable, Q5 = 算; Q4 is TesterCommWiring.cpp (W58-4).
//
//  Part 1 (always): the key table, the raw config.ini text helpers, the Mask state machine, the SIM gate.
//  Part 2 (not with W906_SIMNET_CORE_ONLY): the Handler glue -- IniConfig fields, elConfig's HTEditList entries, the
//  TfConfiguration proxies, W906_SimNetInstall.  Only wb_serve compiles part 2 (the CMakeLists.txt claim line).
// ===========================================================================
#include "MachineType.h"          // SOFT_SIMULTE (MachineType.h:63-65: defined unless CMake passes W906_NO_SOFT_SIMULTE).
                                  //   FIRST on purpose: vclcompat + the full <windows.h> before SimNetMask.h, whose
                                  //   WebBridge/Sync.h opens it WIN32_LEAN_AND_MEAN (no rpcndr.h `byte` for part 2's
                                  //   Public/HTEditList.h -> cmydef.h:127)
#include "SimNet/SimNetMask.h"
#include "vclcompat/IniFiles.h"   // (vclcompat) -- the in-place profile rules, W906_Win32ProfileApply below

#include <cstdio>
#include <cstdlib>

namespace simnet {

// ---------------------------------------------------------------------------
//  The key table.  W58 Q2 = 要 (Steven 20260929 08:1x): the design note's tables A (FTP / net-drive) and B (network
//  links), and the "in doubt" table C keys that really reach the network (C below; research 20260930, the reasons for
//  the C keys left out are next to C).  Section / key = the golden registration (FileRW/IniConfig.gen.inc, generated
//  from the 912 tree cConfiguration.cpp) or the ProcessLastSetIni_* reader (cprod.cpp).  Not here: table D, and the
//  sub-options of a masked parent (N26-2, N32-2..4, N33-1, the FTP host / path fields).  A network SAVE PATH is W58
//  Q5, in the ELA (EventLogAnalysis/ElaSchedule.cpp SimBlocksNetPath).
// ---------------------------------------------------------------------------
static const Row kRows[] = {
    // A. FTP / net-drive
    {"N06",    "FTP",                             "Enable FTP",                              kSaveCopy, "cbN06_EnableFTP"},   // cprod.cpp:2377
    {"N06-1",  "FTP",                             "Enable FTP Password Download",            kSaveCopy, "cbN06_1"},           // cprod.cpp:2414
    {"N10-1",  "FTPUpLoad",                       "bEnable_FTPUpLoadLog",                    kEditList, 0},
    {"N10-2",  "FTPUpLoad",                       "bN10_UploadSummaryToFTP",                 kEditList, 0},
    {"N10-3",  "FTPUpLoad",                       "bN10_DailyUploadProdData",                kEditList, 0},
    {"N10-9",  "FTPUpLoad",                       "bN10_9_UploadUnloadTrayToFTP",            kEditList, 0},
    {"N10-11", "FTPUpLoad",                       "bN10_11_Enable_UploadFTPEventLog",        kEditList, 0},
    {"N10-12", "FTPUpLoad",                       "bN10_12_Enable_UploadFTPGPIBLog",         kEditList, 0},
    {"N40-1",  "FTPUpLoad",                       "bN40_1_HandlerDataBackUpUseFunction",     kEditList, 0},
    {"N12",    "FTP",                             "Second Enable FTP",                       kEditList, 0},
    {"N14-3",  "Handler_OEE",                     "N14_HandlerOEEFTPUpload",                 kEditList, 0},
    {"N14-12", "Handler_OEE",                     "bN14_12_ULTempLogToFTP",                  kEditList, 0},
    {"N14-13", "Handler_OEE",                     "bN14_13_ULBinQtyToFTP",                   kEditList, 0},
    {"N14-19", "Handler_OEE",                     "bN14_19_TrayMappingToFTP",                kEditList, 0},
    {"N14-8",  "Handler_OEE",                     "bN14_8_ULSetup",                          kEditList, 0},   // W58: FTP upload (ProductionInfo.cpp:1554-1582)
    {"N14-9",  "Handler_OEE",                     "bN14_9_ULQtyReport",                      kEditList, 0},   // W58: FTP upload (ProductionInfo.cpp:3380-3383)
    {"N17-1",  "Lot_Summary",                     "bN17UploadLotSummary",                    kEditList, 0},
    {"N21-1",  "bHandlerStateChangeUploadServer", "bN21_HandlerChangeStateUploadServer",     kEditList, 0},
    {"N22",    "ASECL_FTP",                       "bN22Enable_EventLog",                     kEditList, 0},
    {"N22",    "ASECL_FTP",                       "bN22Enable_ASE_CL_FTP",                   kEditList, 0},
    {"N23",    "ASECL_FTP",                       "bN23UploadJHT_Log",                       kEditList, 0},
    {"N35",    "ASECL_FTP",                       "bN35UploadJHT_Log",                       kEditList, 0},
    {"N22-1",  "HANA_FTP",                        "bN22_1_HANA_TrayMapFTP",                  kEditList, 0},
    {"N23-1",  "Murata Function",                 "bN23_1_Enable2DIDCompare",                kEditList, 0},
    {"N23-3",  "Murata Function",                 "bN23_3_UploadTestResult",                 kEditList, 0},
    {"N25-2",  "ChipMos Function",                "bN25_2_EnableUploadLog",                  kEditList, 0},
    {"N25-3",  "ChipMos Function",                "bN25_3_EnableULJamLog",                   kEditList, 0},
    {"N25-4",  "ChipMos Function",                "bN25_4_EnableUpload",                     kEditList, 0},
    {"N25-5",  "ChipMos Function",                "bN25_5_EnableUpload",                     kEditList, 0},
    {"N26-1",  "JamRawDataUpdataToFTP",           "bN26_UseJamRawDataUpdataToFTP",           kEditList, 0},
    {"N27-1",  "bUseAlarmLogXml",                 "bN27_UseAlarmLogXmlUpdataToFTP",          kEditList, 0},
    {"N30-1",  "bRecordGroundESDByTestIC",        "bN30_UseGroundESDUpdataToFTP",            kEditList, 0},
    {"N31-1",  "bAutoTmpeOfsByFTP",               "bN31_UseAutoTempOfsByFTP",                kEditList, 0},   // int radio: 0 Disable / 1 FTP / 2 Net Drive
    {"N32-1",  "bDownloadUpdateAutomatically",    "bN32_DownloadUpdatesAutomatically",       kEditList, 0},
    {"N33",    "LEADYO Function",                 "bN33_UpLoadOCRBinLogByNet",               kEditList, 0},
    {"N35-1",  "Rround_ESD_Upload",               "bN35_Ground_ESD_Upload",                  kEditList, 0},
    {"O06",    "Event Log",                       "bAlarmStatistAutoSaveNetDrive",           kEditList, 0},   // Z: -> \\NET_DRVE
    {"A32",    "Function",                        "bA32EnableFTPAutomation",                 kEditList, 0},
    {"A55",    "Function",                        "bA55EnablePMAlarmUpdateFromServerbyFTP",  kEditList, 0},
    // B. network links.  AI(W906-W58) 20261001 (St02-E): N07-1 [SECS GEM] Enable SECS GEM is NOT masked -- Jimmy RULINGS_20261001 #3 (the laptop runs
    //    SECS / host-start tests in SIM); it reads its config.ini value as in SHIP.
    {"A81",    "SECS GEM",                        "Wait SECS",                               kEditList, 0},
    {"N05",    "",                                "",                                        kSaveCopy, "cbN05_EnableRMS"},   // cprod.cpp:2210 "<RMS|Server> Enable"
    {"N05-1",  "",                                "ERMS Enable",                             kSaveCopy, "cbN05_1"},           // cprod.cpp:2219
    {"N25-1",  "ChipMos Function",                "bN25_1_EnableStartControl",               kEditList, 0},
    {"N24",    "RTM Function",                    "bN24_EnableRTM",                          kEditList, 0},
    // C. "in doubt", W58 Q2 = 要: the keys that reach the network (golden 906_0625_Steven cConfiguration.cpp lines).  Left
    //    out, no network effect: A75 (an OP permission guard, uLotInfo.cpp:1328-1334), N28 / N08-1 (local logs), N09-2
    //    (a message vs a log line), N09-4 (0 = FTP, 1 = Net Drive: no "off"), N14-11 (no consumer), N14-21 (off opens
    //    golden's auto-teach motion path, main.cpp:25164), N14-23 (a file format), N15-2 (the local ESD machine), N16
    //    (fixed 0 for every customer), Enable Check File / Check Setup File (local lot-start checks), B11 / N17-3
    //    (local by default -- a network path is Q5's rule), N14-18 (the temperature offset itself; its download is N14-4's)
    //  N07-2 [SECS GEM] Enable RCMD START (host start, :3215) is NOT masked either -- Jimmy RULINGS_20261001 #3
    {"N07-3",  "SECS GEM",                        "SECS GEM OneCycle",                       kEditList, 0},   // :3223
    {"N07-3-2", "SECS GEM",                       "SECS GEM Alarm",                          kEditList, 0},   // :3227
    {"N07-4",  "Specific",                        "N07_SecsLotCheck",                        kEditList, 0},   // :3235 (+ ProcessLastSetIni_Specific)
    {"N07-5",  "SECS GEM",                        "Enable Employee ID Cheak",                kEditList, 0},   // :3230
    {"N07-6",  "Specific",                        "bN07_6EnableUploadOSRecipe",              kEditList, 0},   // :3242
    {"N07-6-1", "Specific",                       "bN07_6CompressedFile",                    kEditList, 0},   // :3244
    {"N07-7",  "Specific",                        "bN07_7SendRecipeAsBinary",                kEditList, 0},   // :3245
    {"N13",    "ARMS",                            "bN13_EnableARMSFunction",                 kEditList, 0},   // :3442 NETDownloadDataCheck
    {"N09-1",  "Automation",                      "bN09_LotCountAutoFunc",                   kEditList, 0},   // :3274 TSV wait, FTP / net-drive upload
    {"N14-4",  "Handler_OEE",                     "N14_HandlerOEEAutoLoadMOFile",            kEditList, 0},   // :3462 FTP MO download
    {"N14-10", "Handler_OEE",                     "N14_AutoDownloadSetupFileByMO",           kEditList, 0},   // :3475 FTP setup download
    {"N14-22", "Handler_OEE",                     "bN14_21_ConfigUpdateFromServerExport",    kEditList, 0},   // :3509 config from the server
    {"N15-1",  "ESD_Control",                     "N15_ESDControlUserLevelByTxt",            kEditList, 0},   // :3543 FTP password file
    {"N41-1",  "FTPUpLoad",                       "bN41_1_HandlerDataBackUpToDiskUseFunction", kEditList, 0}, // :3912 (never registered: USE_BU5_Function 0)
};

const Row* DefaultRows(std::size_t* count)
{
    if (count) *count = sizeof(kRows) / sizeof(kRows[0]);
    return kRows;
}

// AI(W906-W58) 20260930 (St02-E): W58 Q3 (SimNetMask.h)
Q3Plan Q3Decide(bool readFromFile, bool enabled, bool visible, bool valueOn)
{
    (void)enabled;             // a masked entry is enabled whatever it was
    Q3Plan q;
    q.mask = visible || valueOn;
    q.forced = q.mask && !readFromFile;
    q.show = q.mask && !visible;
    return q;
}

// ---------------------------------------------------------------------------
//  Raw ini text (the rules of vclcompat/IniFiles.cpp EOF, W906IniApply)
// ---------------------------------------------------------------------------
namespace {

struct Line
{
    std::string text, eol;
};

bool IsBlank(char c) { return c == ' ' || c == '\t'; }

std::string Trim(const std::string& s)
{
    std::size_t b = 0, e = s.size();
    while (b < e && IsBlank(s[b])) ++b;
    while (e > b && IsBlank(s[e - 1])) --e;
    return s.substr(b, e - b);
}

std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}

bool IEq(const std::string& a, const std::string& b) { return Lower(Trim(a)) == Lower(Trim(b)); }

std::vector<Line> Split(const std::string& buf)
{
    std::vector<Line> out;
    std::size_t i = 0;
    const std::size_t n = buf.size();
    while (i < n)
    {
        std::size_t j = i;
        while (j < n && buf[j] != '\r' && buf[j] != '\n') ++j;
        Line l;
        l.text = buf.substr(i, j - i);
        if (j >= n) { out.push_back(l); break; }
        if (buf[j] == '\r' && j + 1 < n && buf[j + 1] == '\n') { l.eol = "\r\n"; i = j + 2; }
        else { l.eol = std::string(1, buf[j]); i = j + 1; }
        out.push_back(l);
    }
    return out;
}

std::string Join(const std::vector<Line>& L)
{
    std::string o;
    for (std::size_t i = 0; i < L.size(); ++i) o += L[i].text + L[i].eol;
    return o;
}

bool SectionOf(const std::string& text, std::string* name)
{
    const std::string s = Trim(text);
    if (s.empty() || s[0] != '[') return false;
    std::string body = s.substr(1);
    const std::size_t k = body.find(']');
    if (k != std::string::npos) body = body.substr(0, k);
    *name = Lower(Trim(body));
    return true;
}

bool KeyOf(const std::string& text, std::string* key)
{
    std::size_t b = 0;
    while (b < text.size() && IsBlank(text[b])) ++b;
    if (b < text.size() && text[b] == ';') return false;
    const std::size_t k = text.find('=');
    if (k == std::string::npos) return false;
    *key = Lower(Trim(text.substr(0, k)));
    return true;
}

bool ReadBytes(const std::string& path, std::string* out)
{
    out->clear();
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    char tmp[65536];
    std::size_t n;
    while ((n = std::fread(tmp, 1, sizeof(tmp), f)) > 0) out->append(tmp, n);
    std::fclose(f);
    return true;
}

// AI(W906-W58) 20261001 (St02-E): the temp file's bytes reach the disk before the rename (E2 delta m1): CreateFileA / WriteFile /
//   FlushFileBuffers.  MOVEFILE_WRITE_THROUGH alone does not flush the data of a same-volume rename.
bool WriteBytesFlushed(const std::string& path, const std::string& data)
{
    HANDLE h = ::CreateFileA(path.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD n = 0;
    const bool ok = (data.empty() || (::WriteFile(h, data.data(), static_cast<DWORD>(data.size()), &n, 0) && n == data.size())) &&
                    ::FlushFileBuffers(h);
    return ::CloseHandle(h) && ok;
}

// AI(W906-W58) 20261001 (St02-E): the write-back is atomic (St02-M, W58 phase 1 HOLD work): the whole new text goes to a temp file in the
//   same folder (flushed to the disk, WriteBytesFlushed above), which then replaces the file in one MoveFileExA -- a
//   crash or a power cut leaves either the old config.ini or the new one, never half of it.  The temp file is
//   "<file>.simnet.tmp"; it is removed on a failure (the file itself is then untouched: SimNet_Write section 5).
bool WriteBytesAtomic(const std::string& path, const std::string& data)
{
    const std::string tmp = path + ".simnet.tmp";
    if (WriteBytesFlushed(tmp, data) &&
        ::MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        return true;
    ::DeleteFileA(tmp.c_str());
    return false;
}

// the section header line and the key line (npos = not there); end = the line after the section
struct Where
{
    std::size_t sec, key, end;
};

Where Locate(const std::vector<Line>& L, const std::string& section, const std::string& key)
{
    Where w;
    w.sec = w.key = w.end = std::string::npos;
    const std::string ls = Lower(Trim(section)), lk = Lower(Trim(key));
    std::string nm;
    for (std::size_t i = 0; i < L.size(); ++i)
        if (SectionOf(L[i].text, &nm) && nm == ls) { w.sec = i; break; }
    if (w.sec == std::string::npos) return w;
    w.end = L.size();
    for (std::size_t i = w.sec + 1; i < L.size(); ++i)
        if (SectionOf(L[i].text, &nm)) { w.end = i; break; }
    for (std::size_t i = w.sec + 1; i < w.end; ++i)
        if (KeyOf(L[i].text, &nm) && nm == lk) { w.key = i; break; }
    return w;
}

bool IsConfigIni(const char* file)
{
    if (!file) return false;
    const std::string f(file);
    const std::size_t p = f.find_last_of("\\/");
    return Lower(p == std::string::npos ? f : f.substr(p + 1)) == "config.ini";
}

std::string Num(int v)
{
    char b[16];
    std::snprintf(b, sizeof(b), "%d", v);
    return b;
}

}  // namespace

bool ReadRawValue(const std::string& file, const std::string& section, const std::string& key, std::string* value,
                  bool* sectionFound)
{
    if (sectionFound) *sectionFound = false;
    if (value) value->clear();
    std::string buf;
    if (!ReadBytes(file, &buf)) return false;
    const std::vector<Line> L = Split(buf);
    const Where w = Locate(L, section, key);
    if (sectionFound) *sectionFound = w.sec != std::string::npos;
    if (w.key == std::string::npos) return false;
    if (value) *value = L[w.key].text.substr(L[w.key].text.find('=') + 1);
    return true;
}

}  // namespace simnet
namespace vclcompat {
std::string W906_Win32ProfileApply(const std::string& buf, const std::string& section, const std::string& key,
                                   const std::string& value);   // vclcompat/IniFiles.cpp: TIniFile::WriteString's rules
}
namespace simnet {

void WriteRawValue(const std::string& file, const std::string& section, const std::string& key,
                   const std::string& value)
{
    // AI(W906-W58) 20261001 (St02-E): the same bytes TIniFile::WriteString (vclcompat's in-place writer) produced, written atomically
    std::string buf;
    if (!ReadBytes(file, &buf) && ::GetFileAttributesA(file.c_str()) != INVALID_FILE_ATTRIBUTES)
        return;                                              // AI(W906-W58) 20261001 (St02-E): there but unreadable -> do not replace it with one section (E2 delta n4)
    WriteBytesAtomic(file, vclcompat::W906_Win32ProfileApply(buf, section, key, value));   // a missing file = empty, as the in-place writer does
}

bool RemoveKey(const std::string& file, const std::string& section, const std::string& key, bool dropEmptySection)
{
    std::string buf;
    if (!ReadBytes(file, &buf)) return false;
    std::vector<Line> L = Split(buf);
    const Where w = Locate(L, section, key);
    if (w.key == std::string::npos) return false;
    L.erase(L.begin() + (std::ptrdiff_t)w.key);
    if (dropEmptySection)
    {
        const std::size_t end = w.end - 1;   // one line fewer
        bool empty = true;
        for (std::size_t i = w.sec + 1; i < end && empty; ++i)
            if (!Trim(L[i].text).empty()) empty = false;
        if (empty) L.erase(L.begin() + (std::ptrdiff_t)w.sec, L.begin() + (std::ptrdiff_t)end);
    }
    return WriteBytesAtomic(file, Join(L));   // AI(W906-W58) 20261001 (St02-E): atomic (was a direct "wb" rewrite, E2 W58 n3)
}

// ---------------------------------------------------------------------------
//  Mask
// ---------------------------------------------------------------------------
Mask::Mask() : env_(0) {}

void Mask::Attach(const Row* rows, std::size_t count, Env* env)
{
    webbridge::WbGuard g(mu_);
    slots_.clear();
    for (std::size_t i = 0; i < count; ++i)
    {
        Slot s;
        s.row = rows[i];
        slots_.push_back(s);
    }
    env_ = env;
}

void Mask::Arm()
{
    webbridge::WbGuard g(mu_);
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        Slot& s = slots_[i];
        s.b = Binding();
        s.st = kPending;
        s.run = 0;
        s.snap = s.snapPresent = s.snapSection = false;
        s.snapRaw.clear();
    }
}

void Mask::Phase(int phase)
{
    webbridge::WbGuard g(mu_);
    if (!env_) return;
    if (phase == 0) Phase0();
    else if (phase == 1) Phase1();
    else if (phase == 2) Phase2();
}

void Mask::Apply(Slot& s, int value)
{
    if (s.b.bField) *s.b.bField = value != 0;
    if (s.b.iField) *s.b.iField = value;
    env_->SetProxy(s.row, s.b, value);
}

// what the golden reader makes of the snapshot (a missing key = 0, the HTEditList / CheckAndReadIniData default of
// every row in the table)
int Mask::FileValue(const Slot& s) const
{
    if (!s.snapPresent) return 0;
    const std::string t = Trim(s.snapRaw);
    char* end = 0;
    const long v = std::strtol(t.c_str(), &end, 10);
    if (end == t.c_str()) return 0;
    if (s.b.iField) return (int)v;
    return s.b.onlyOneIsOn ? (v == 1 ? 1 : 0) : (v != 0 ? 1 : 0);
}

void Mask::Phase0()
{
    int masked = 0, skipped = 0;
    std::string names;
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        Slot& s = slots_[i];
        if (s.st == kPending)
        {
            Binding b;
            const int r = env_->Resolve(s.row, &b);
            if (r < 0) continue;
            if (r == 0) { s.st = kSkipped; ++skipped; continue; }
            s.b = b;
            s.st = kArmed;
            s.run = 0;
            ++masked;
            names += std::string(names.empty() ? "" : " ") + s.row.code;
        }
        if (s.st == kArmed || s.st == kReleased) Apply(s, s.run);
    }
    if (masked || skipped)
        env_->Log("phase 0: " + Num(masked) + " network key(s) OFF for this run [" + names + "], " + Num(skipped) +
                  " not masked (not read for this customer); config.ini not written");
}

void Mask::Phase1()
{
    const std::string file = env_->ConfigIniPath();
    std::string changed;
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        Slot& s = slots_[i];
        if (s.st != kArmed && s.st != kReleased) continue;
        const int c = env_->Choice(s.row, s.b);
        const RowState ns = c != 0 ? kReleased : kArmed;
        const bool unticked = (s.st == kReleased && c == 0);   // released earlier this run, unticked now: the operator's choice
        if (ns != s.st || c != s.run)
            changed += std::string(changed.empty() ? "" : " ") + s.row.code + (c != 0 ? "=ON" : "=OFF");
        s.st = ns;
        s.run = c;
        // W58 Q1 = 要: a released key -- and one unticked after a release this run -- is saved as the operator left it
        //   (config.ini gets that value; a later SHIP launch follows it).  An armed key still gets the file's own value, so
        //   the mask itself never reaches the file; phase 2 undoes anything the save added for it.  A forced key (Q3) has
        //   no file value (HTEditList never saves it): this run only.
        if (ns == kReleased || unticked || s.b.forced)
        {
            s.snap = false;
            if (unticked && s.row.kind == kEditList)
                env_->SetProxy(s.row, s.b, c);   // E2 W58 n1: the field keeps its released value until phase 2, as golden's
                                                 //   field would -- SPIL's WAR16132 (cprod.cpp:3307-3312) compares the two
            else
                Apply(s, c);
            continue;
        }
        s.snapPresent = ReadRawValue(file, s.b.section, s.b.key, &s.snapRaw, &s.snapSection);
        s.snap = true;
        Apply(s, FileValue(s));   // the golden save writes the file's own value (no masked value reaches the file)
    }
    if (!changed.empty())
        env_->Log("phase 1: this run " + changed + " (a ticked key is saved as ticked, W58 Q1)");
}

void Mask::Phase2()
{
    const std::string file = env_->ConfigIniPath();
    int restored = 0;
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        Slot& s = slots_[i];
        if (s.snap)
        {
            std::string cur;
            const bool present = ReadRawValue(file, s.b.section, s.b.key, &cur);
            if (present != s.snapPresent || (present && cur != s.snapRaw))
            {
                if (s.snapPresent) WriteRawValue(file, s.b.section, s.b.key, s.snapRaw);
                else RemoveKey(file, s.b.section, s.b.key, !s.snapSection);
                ++restored;
            }
            s.snap = false;
        }
        if (s.st == kArmed || s.st == kReleased) Apply(s, s.run);
    }
    if (restored)
        env_->Log("phase 2: wrote back " + Num(restored) + " armed key(s) -- the mask itself never changes a SHIP setting");
}

bool Mask::ElaValue(const char* file, const char* section, const char* key, bool fileValue)
{
    if (!IsConfigIni(file) || !section || !key) return fileValue;
    webbridge::WbGuard g(mu_);
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        const Slot& s = slots_[i];
        const std::string sec = s.st == kPending ? std::string(s.row.section) : s.b.section;
        const std::string k = s.st == kPending ? std::string(s.row.key) : s.b.key;
        if (sec.empty() || !IEq(sec, section) || !IEq(k, key)) continue;
        if (s.st == kPending || s.st == kArmed) return false;   // not resolved yet = off (fail safe)
        if (s.st == kReleased) return true;
        return fileValue;                                        // kSkipped: not masked
    }
    return fileValue;
}

std::size_t Mask::Count() const { return slots_.size(); }

const Row& Mask::RowAt(std::size_t i) const { return slots_[i].row; }

RowState Mask::State(std::size_t i)
{
    webbridge::WbGuard g(mu_);
    return slots_[i].st;
}

int Mask::RunValue(std::size_t i)
{
    webbridge::WbGuard g(mu_);
    return slots_[i].run;
}

Binding Mask::BindingAt(std::size_t i)
{
    webbridge::WbGuard g(mu_);
    return slots_[i].b;
}

int Mask::CountIn(RowState st)
{
    webbridge::WbGuard g(mu_);
    int n = 0;
    for (std::size_t i = 0; i < slots_.size(); ++i)
        if (slots_[i].st == st) ++n;
    return n;
}

// ---------------------------------------------------------------------------
//  The SIM gate
// ---------------------------------------------------------------------------
bool SimBuild()
{
#ifdef SOFT_SIMULTE
    return true;
#else
    return false;
#endif
}

bool InstallHooks(void (**phaseSlot)(int), void (*phaseFn)(int), void (*elaSetter)(ElaBoolFn), ElaBoolFn elaFn)
{
#ifdef SOFT_SIMULTE
    if (phaseSlot) *phaseSlot = phaseFn;
    if (elaSetter) elaSetter(elaFn);
    return true;
#else
    (void)phaseSlot;
    (void)phaseFn;
    (void)elaSetter;
    (void)elaFn;
    return false;
#endif
}

}  // namespace simnet

// ===========================================================================
//  Part 2: the Handler glue (wb_serve only)
// ===========================================================================
#ifndef W906_SIMNET_CORE_ONLY

#include "FileRW/_EditList.h"   // filerw::ELFind, HTEditList / THTEdit (Public/HTEditList.h), TCheckBox / TRadioGroup
#include "Config.h"             // IniConfig
#include "CosFunction.h"        // CosFunction
#include "cmydef.h"             // CUSTOMER_CODE
#include "common.h"             // AuthPath

// The SHIP build (W906_NO_SOFT_SIMULTE) compiles none of the glue: W906_SimNetInstall is empty, nothing is installed and
// W906_SimNetHook / the ELA override stay NULL.
#ifdef SOFT_SIMULTE

extern void (*W906_SimNetHook)(int iPhase);                                            // cprod.cpp:171 (a claim line)
void W906_ElaSetBoolOverride(bool (*fn)(const char*, const char*, const char*, bool));   // EventLogAnalysis/ElaIniOverride.cpp (AI(W906-B20-ELAWININET) 20261001: was ElaService.cpp)

namespace {

// HTEditList reads / saves only the FIRST entry of a (group, key) (HTEditList.cpp mapKeyValue): that one is live.
THTEdit* FirstEntry(HTEditList* el, const char* section, const char* key)
{
    const AnsiString sec(section), k(key);
    for (int i = 0; i < el->FEditList->Count; ++i)
    {
        THTEdit* e = static_cast<THTEdit*>(el->FEditList->Items[i]);
        if (e && e->IniGroupName == sec && e->IniKeyName == k) return e;
    }
    return 0;
}

class HandlerEnv : public simnet::Env
{
public:
    std::string ConfigIniPath() { return std::string(AuthPath.c_str()) + "config.ini"; }   // cprod.cpp:2193 / :3135

    int Resolve(const simnet::Row& row, simnet::Binding* b)
    {
        if (row.kind == simnet::kEditList)
        {
            if (!elConfig || !elConfig->FEditList || elConfig->FEditList->Count == 0) return -1;   // not registered yet
            THTEdit* e = FirstEntry(elConfig, row.section, row.key);
            if (!e) return 0;                                          // not an elConfig key for this customer
            // W58 Q3 = 關 + enable (Steven 20260929 08:1x 「模擬版時. 關 (改成不反灰 也就是元件要enable)」): a customer-forced
            //   (bReadFromFile == false, HTEditList.cpp:718 / :1140) or greyed (bEnable == false) key is masked too and its
            //   component enabled; a hidden one is skipped when off and shown when on (simnet::Q3Decide, SimNetMask.h).
            const bool on = (e->Content == ECBool && e->bParameter && *e->bParameter) ||
                            (e->Content != ECBool && e->iParameter && *e->iParameter);
            const simnet::Q3Plan q = simnet::Q3Decide(e->bReadFromFile, e->bEnable, e->bVisible, on);
            if (!q.mask) return 0;
            b->forced = q.forced;
            TCheckBox* cb = dynamic_cast<TCheckBox*>(e->SourceControl);
            TRadioGroup* rg = dynamic_cast<TRadioGroup*>(e->SourceControl);
            if (e->Content == ECBool && cb && e->bParameter) b->bField = e->bParameter;
            else if (rg && e->iParameter) b->iField = e->iParameter;
            else return 0;
            e->bEnable = true;                                         // only for a key that is really masked
            e->SourceControl->Enabled = true;                          // what InitialDataToEdit does next read (HTEditList.cpp:1470);
                                                                       //   now too: this read's InitialDataToEdit ran before phase 0
                                                                       //   (cprod.cpp:3064 < :3217), and the page save drops a value of
                                                                       //   a disabled control / entry (FileRW/IniConfig.cpp:273-292)
            if (q.show)                                                // E2 W58 m1 (SIM only): InitialDataToEdit applies bVisible next
            {                                                          //   read (HTEditList.cpp:1469); the page's ELOperable and the save
                e->bVisible = true;                                    //   drop rule reject an invisible control (FileRW/_EditList.cpp:237-246)
                e->SourceControl->Visible = true;
            }
            b->control = e->SourceControl;
            b->section = row.section;
            b->key = row.key;
            b->onlyOneIsOn = true;                                     // HTEditList ReadEditTextFromFile: ReadInteger()==1
            return 1;
        }
        // kSaveCopy: golden reads these only in ProcessLastSetIni_RMS / _FTP (cprod.cpp:2191-2497) under these conditions
        const std::string rms = (CUSTOMER_CODE == CC_SCC || CUSTOMER_CODE == CC_SCK) ? "RMS" : "Server";   // cprod.cpp:2196
        const std::string p = row.proxy ? row.proxy : "";
        const bool ftp = CosFunction.bFTPFunction || IniConfig.bFTPJamCodeUpload;                      // cprod.cpp:2375
        if (p == "cbN06_EnableFTP")
        {
            if (!ftp) return 0;
            b->bField = &IniConfig.bEnableFTP;
        }
        else if (p == "cbN06_1")
        {
            if (!ftp || !CosFunction.PassworDownloadByFTP) return 0;   // the page sets it only then (CheckConfigurationBeforeSave)
            b->bField = &IniConfig.bFtpPasswordDownload;
        }
        else if (p == "cbN05_EnableRMS")
        {
            if (!IniConfig.bShowLotInfo || CUSTOMER_CODE == CC_KYEC_LEE) return 0;   // cprod.cpp:2200-2207: KYEC forces it off
            b->bField = &IniConfig.bEnableRms;
        }
        else if (p == "cbN05_1")
        {
            if (!IniConfig.bShowLotInfo || !CosFunction.bUseERMS) return 0;          // cprod.cpp:2217-2227
            b->bField = &IniConfig.bEnableErms;
        }
        else
            return 0;
        b->section = row.section[0] ? std::string(row.section) : rms;
        b->key = row.key[0] ? std::string(row.key) : rms + " Enable";
        b->control = 0;
        b->onlyOneIsOn = false;                                        // CheckAndReadIniData -> ReadBool: non-zero = on
        return 1;
    }

    void SetProxy(const simnet::Row& row, const simnet::Binding& b, int value)
    {
        TControl* c = row.kind == simnet::kEditList ? static_cast<TControl*>(b.control)
                                                    : filerw::ELFind("TfConfiguration", row.proxy);
        if (TCheckBox* cb = dynamic_cast<TCheckBox*>(c)) cb->Checked = value != 0;
        else if (TRadioGroup* rg = dynamic_cast<TRadioGroup*>(c)) rg->ItemIndex = value;
    }

    int Choice(const simnet::Row& row, const simnet::Binding& b)
    {
        if (row.kind == simnet::kSaveCopy) return (b.bField && *b.bField) ? 1 : 0;   // CheckConfigurationBeforeSave copied it
        TControl* c = static_cast<TControl*>(b.control);
        if (TCheckBox* cb = dynamic_cast<TCheckBox*>(c)) return cb->Checked ? 1 : 0;
        if (TRadioGroup* rg = dynamic_cast<TRadioGroup*>(c)) return rg->ItemIndex;
        return 0;
    }

    void Log(const std::string& line) { std::printf("SimNet W58 (SIM): %s\n", line.c_str()); }
};

HandlerEnv g_env;
simnet::Mask g_mask;

void PhaseFn(int phase) { g_mask.Phase(phase); }

bool ElaFn(const char* file, const char* section, const char* key, bool fileValue)
{
    return g_mask.ElaValue(file, section, key, fileValue);
}

}  // namespace

#endif  // SOFT_SIMULTE

void W906_SimNetInstall()
{
#ifdef SOFT_SIMULTE
    static bool done = false;
    if (done) return;
    done = true;
    std::size_t n = 0;
    const simnet::Row* rows = simnet::DefaultRows(&n);
    g_mask.Attach(rows, n, &g_env);
    if (simnet::InstallHooks(&W906_SimNetHook, &PhaseFn, &W906_ElaSetBoolOverride, &ElaFn))
        std::printf("SimNet W58 (SIM): %u network keys start OFF this run (resolved at the first ReadLastSetIni); "
                    "tick + save one to turn it on (config.ini then gets the tick); SECS GEM / host start follow config.ini\n",
                    (unsigned)n);
    std::fflush(stdout);
#endif
}

#endif  // W906_SIMNET_CORE_ONLY
