// =============================================================================
//  JamRules.h -- the missing-key rules of D:\HT9045\Error\English\JAM0000.dat that the Handler's Jam Code editor and
//  the Event Log Analyzer share (★W45, formerly W20: the two Jam Code editors merged into one).  Header-only.
//
//  AI(W906-ELA-W45) 20260927 (St02-E).  Research (St02-M, read only):
//  D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-w20-jamcode-editor.md §1.3, §4, §5.1.
//  Golden: Handler = 906_0625_Steven (D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven), analyzer = Rev891
//  (D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code).  Ledger: docs/ELA_PORT_LEDGER.md "W45".
//
//  Header-only for the same reason as JamIniMerge.h: the ELA library (EventLogAnalysis/ElaCore.cpp) uses it, and so
//  does the Handler side (WebSecurityJamW45.h, included by WebSecurityJam.cpp) without a new source in the wb_serve list.
//  AI(W906-ELA-W45) 20260928 (St02-E helper): Steven 0928 item 18 -- 18a W20-1 = A (the one editor is the Security
//  page's Jam tab), 18b W20-3 = B (the switch below), 18c W20-5 = A (the second box, end of this file).  Golden
//  TfSecurity's own getters keep their literals (Steven: the Handler getter is unchanged).  W20-2 / 4 / 6 / 7 / 8 / 9:
//  未明確裁決，照 golden (ledger "W45").
//
//  Both goldens read a key with CheckAndReadIniData: a MISSING key is written with the reader's default first
//  (906_0625_Steven common.cpp:449-464 bool / :432-447 int / :466-491 string; Rev891 Common.cpp:124-139 / :107-122), so
//  the first reader of a missing key decides the file.  The defaults agree everywhere but one place (research §4 C1 /
//  C2): for CC_ASE_CL 933 and CC_TERAPOWER 967 the Handler keeps its "Include MTBA" box in "<code> IncludeMTBF" and
//  defaults it to true, while the analyzer reads that key as MTBF with its own list.  That one default is the
//  ★W45 switch below (formerly W20-3; Steven 0928: B).
// =============================================================================
#pragma once

#include <cstdlib>
#include <string>

namespace jamrules {

// ---- the file --------------------------------------------------------------------------------------------------
// golden literal "D:\HT9045\Error\English\JAM0000.dat" (906_0625_Steven cSecurity.cpp:216; Rev891 Analyzer.cpp:296);
// W906_JAM0000_PATH when set (getenv not NULL and not "", the D5 rule) -- ctests point it into %TEMP%.
inline std::string Jam0000Path()
{
    const char* e = std::getenv("W906_JAM0000_PATH");
    return (e != 0 && *e != 0) ? std::string(e) : std::string("D:\\HT9045\\Error\\English\\JAM0000.dat");
}

// ---- the two customers whose Handler editor uses the IncludeMTBF key ---------------------------------------------
// 906_0625_Steven MachineType.h:300 CC_ASE_CL 933, :337 CC_TERAPOWER 967; cSecurity.cpp:1335-1336 (read) / :1138-1139
// (write) test exactly these two.
const int kCcAseCl = 933;
const int kCcTeraPower = 967;
inline bool IsMtbfAlias(int customerCode) { return customerCode == kCcAseCl || customerCode == kCcTeraPower; }
// the Gerneral.ini [System] CUSTOMER_CODE text ("933"), as ELA keeps it (ela::Options::custCode)
inline bool IsMtbfAlias(const std::string& custCode) { return IsMtbfAlias(std::atoi(custCode.c_str())); }

// ---- key names (section = the area name, "01 Input Arm" .. "31 Cylinder", or a CSV UnitName for the analyzer) -----
inline std::string LevelKey(const std::string& code) { return code; }                           // the level
inline std::string IncludeMtbaKey(const std::string& code) { return code + " IncludeMTBA"; }
inline std::string IncludeMtbfKey(const std::string& code) { return code + " IncludeMTBF"; }
// the key behind the Handler's "Include MTBA" box and TfSecurity::GetJemIncludeMTBA: IncludeMTBF for 933 / 967,
// IncludeMTBA for everyone else (906_0625_Steven cSecurity.cpp:1335-1353 read, :1138-1146 write)
inline std::string HandlerIncludeMtbaKey(const std::string& code, bool mtbfAlias)
{
    return mtbfAlias ? IncludeMtbfKey(code) : IncludeMtbaKey(code);
}

// ---- defaults of a missing key --------------------------------------------------------------------------------
// the level: Rev891 Analyzer.cpp:2806 (int 0); the Handler reads the string "0" (906_0625_Steven cSecurity.cpp:1211-1212)
// and then applies its customer clamps (:1213-1293), which are the Handler's and not a default (not here)
inline int DefaultJamLevel() { return 0; }

// "Include MTBA" -- the same rule on both sides: "JAM" anywhere in the code (AnsiString::Pos) and one of the areas
// 01..05.  Rev891 Analyzer.cpp:2820-2832 = 906_0625_Steven cSecurity.cpp:1342-1354.
inline bool DefaultIncludeMtba(const std::string& area, const std::string& code)
{
    return code.find("JAM") != std::string::npos &&
           (area == "01 Input Arm" || area == "02 Output Arm" || area == "03 Index Unit" || area == "04 Input Shuttle" ||
            area == "05 Output Shuttle");
}

// the analyzer's MTBF list: area "24 Motor", or one of 24 codes (exact).  Rev891 Analyzer.cpp:2846-2871 (golden lists
// WAR01300 twice, :2847-2848).
inline bool AnalyzerMtbfListed(const std::string& area, const std::string& code)
{
    static const char* const kCodes[] = { "WAR01300", "WAR01301", "WAR0348", "JAM0407", "JAM0408", "WAR1635", "WAR1636",
                                          "WAR1638", "WAR1639", "WAR1690", "WAR1691", "WAR1692", "WAR1693", "WAR1694",
                                          "WAR1695", "WAR1696", "WAR16109", "WAR2201", "WAR2202", "WAR2203", "WAR16150",
                                          "WAR16151", "WAR16152", "MES16119" };
    if (area == "24 Motor")
        return true;
    for (size_t i = 0; i < sizeof(kCodes) / sizeof(kCodes[0]); ++i)
        if (code == kCodes[i])
            return true;
    return false;
}

// ★W45 (formerly W20-3): the default of a MISSING "<code> IncludeMTBF" key where the two goldens disagree -- only
// CC_ASE_CL 933 / CC_TERAPOWER 967 (research §4 C1, example §4.2-1).
//   A  golden on each side (the first reader writes the file): the Handler (its "Include MTBA" box) true, the analyzer
//      its MTBF list.  906_0625_Steven cSecurity.cpp:1338 / Rev891 Analyzer.cpp:2846-2878.
//   B  one rule, the Handler's: missing = true for 933 / 967 on both sides (the analyzer deviates).
//   C  one rule, the analyzer's: missing = the MTBF list on both sides (the Handler deviates).
//   Every other customer: the two sides never read the key with different defaults (the Handler never reads it).
// AI(W906-ELA-W45) 20260928 (St02-E helper): Steven 0928 item 18b = B.  Only ELA deviates from its own golden (ELA's
//   ela::Options::goldenJamMtbfDefault = true still gives Rev891's list, for the #23 oracle); the Handler's value under
//   B is golden's own, and golden TfSecurity::GetJemIncludeMTBA is not changed.
enum IncludeMtbfRule { kIncludeMtbfGoldenPerSide = 'A', kIncludeMtbfHandlerWins = 'B', kIncludeMtbfAnalyzerWins = 'C' };
static const IncludeMtbfRule kIncludeMtbfRule = kIncludeMtbfHandlerWins;   // ★W45 ONE-LINE SWITCH: B (Steven 0928) / A / C

// the analyzer's (ELA JamConfig::GetJemIncludeMTBF) default for a missing "<code> IncludeMTBF"
inline bool DefaultIncludeMtbfAnalyzer(const std::string& area, const std::string& code, bool mtbfAlias,
                                       IncludeMtbfRule rule = kIncludeMtbfRule)
{
    if (mtbfAlias && rule == kIncludeMtbfHandlerWins)
        return true;
    return AnalyzerMtbfListed(area, code);
}

// the Handler's (TfSecurity::GetJemIncludeMTBA) default for a missing HandlerIncludeMtbaKey(code, alias)
inline bool DefaultIncludeMtbaHandler(const std::string& area, const std::string& code, bool mtbfAlias,
                                      IncludeMtbfRule rule = kIncludeMtbfRule)
{
    if (mtbfAlias)
        return rule == kIncludeMtbfAnalyzerWins ? AnalyzerMtbfListed(area, code) : true;   // golden :1338 `, true)`
    return DefaultIncludeMtba(area, code);
}

// =============================================================================
//  ★W45 18c -- the merged editor's second check box.  AI(W906-ELA-W45) 20260928 (St02-E helper).
//  Steven 0928 W20-5 = A: one box per key.  Box 1 is the Handler's cbIncludeMTBA (golden, unchanged: key
//  HandlerIncludeMtbaKey, TfSecurity::ChangeJamMessage / SaveJamLevel).  Box 2 is the analyzer's cbIncludeMTBF
//  (Rev891 Analyzer.dfm:2029-2042 "Include MTBF"; loaded Analyzer.cpp:2753, saved :2772), bound to THE OTHER key, so that
//  every key either golden editor can change is still changeable, and no two boxes write one key:
//    other customers  "<code> IncludeMTBF" -- the analyzer's own key; caption "Include MTBF" (golden's); default
//                     DefaultIncludeMtbfAnalyzer (= ELA JamConfig::GetJemIncludeMTBF's).
//    933 / 967        "<code> IncludeMTBA" -- the analyzer's MTBA key (:2771), which the Handler's box does not write for
//                     them (it writes IncludeMTBF, 906_0625_Steven cSecurity.cpp:1138-1141); caption
//                     "Include MTBA (Analyzer)" (research §5.1 option A: golden's "Include MTBA" would read the same as
//                     box 1); default DefaultIncludeMtba (= GetJemIncludeMTBA's, both goldens).
//  With 18b = B every key has ONE missing-key default whichever reader comes first (ctest Jam_Rules section 5).
// =============================================================================
const char* const kSecondBoxId = "cbIncludeMTBF";                              // the analyzer's component name
inline std::string SecondBoxKey(const std::string& code, bool mtbfAlias)
{
    return mtbfAlias ? IncludeMtbaKey(code) : IncludeMtbfKey(code);
}
inline std::string SecondBoxCaption(bool mtbfAlias) { return mtbfAlias ? "Include MTBA (Analyzer)" : "Include MTBF"; }
inline bool DefaultSecondBox(const std::string& area, const std::string& code, bool mtbfAlias,
                             IncludeMtbfRule rule = kIncludeMtbfRule)
{
    return mtbfAlias ? DefaultIncludeMtba(area, code) : DefaultIncludeMtbfAnalyzer(area, code, false, rule);
}

// The second box as a virtual TCheckBox for the Handler's page (golden TfSecurity has no such component; the
// Handler's glue is WebSecurityJamW45.h).  Io needs
//     bool Read(file, group, name, bool def)    golden CheckAndReadIniData (bool): a missing key is written with def
//     void Write(file, group, name, bool value) golden WriteIniData (bool)
// (the Handler passes common.cpp's; ctest Jam_Rules a TIniFile in %TEMP%).  The caller holds the JAM0000.dat lock.
//   Load   the analyzer's ChangeJamMessage line :2753 for the record the page shows (area, code; the language does not
//          matter).  No FileExists check (Handler-owned data: the Handler's getters just read / wrote the same file).
//   Apply  the operator clicking the box: only after a Load, and only for kSecondBoxId (false = not this box).
//   Save   the analyzer's SaveJamLevel line :2772 for the record Load read: nothing when area or code is empty
//          (:2759-2766, the same check as the Handler's :1065 / :1125).  No FileExists check (Handler-owned data: the
//          Handler's SaveJamLevel has just written the file; the analyzer's :2768 is for its off-line use).
//   WriteJson  boxes.cbIncludeMTBF {checked, visible, enabled, caption, key}.  Shown only while the record the page
//          shows is the one Load read (a guard reply before the first Load hides it; always enabled otherwise --
//          the analyzer never hides or disables it).
class SecondBox
{
public:
    SecondBox() : loaded_(false), alias_(false), checked_(false) {}

    template <class Io>
    void Load(Io& io, const std::string& file, const std::string& area, const std::string& code, int customerCode)
    {
        file_ = file;
        area_ = area;
        code_ = code;
        alias_ = IsMtbfAlias(customerCode);
        loaded_ = true;
        checked_ = false;
        if (area != "" && code != "")
            checked_ = io.Read(file, area, SecondBoxKey(code, alias_), DefaultSecondBox(area, code, alias_));
    }
    bool Apply(const std::string& name, bool value)
    {
        if (!loaded_ || name != kSecondBoxId)
            return false;
        checked_ = value;
        return true;
    }
    template <class Io>
    bool Save(Io& io) const
    {
        if (!loaded_ || area_ == "" || code_ == "")
            return false;
        io.Write(file_, area_, SecondBoxKey(code_, alias_), checked_);
        return true;
    }
    bool Fresh(const std::string& area, const std::string& code) const
    {
        return loaded_ && area == area_ && code == code_;
    }
    template <class W>
    void WriteJson(W& w, const std::string& area, const std::string& code) const
    {
        const bool fresh = Fresh(area, code);
        w.Key(kSecondBoxId).BeginObject();
        w.Key("checked").Bool(fresh && checked_);
        w.Key("visible").Bool(fresh);
        w.Key("enabled").Bool(fresh);
        w.Key("caption").String(SecondBoxCaption(alias_));
        w.Key("key").String(fresh && code_ != "" ? SecondBoxKey(code_, alias_) : std::string());
        w.EndObject();
    }
    bool loaded() const { return loaded_; }
    bool checked() const { return checked_; }
    bool alias() const { return alias_; }
    const std::string& area() const { return area_; }
    const std::string& code() const { return code_; }

private:
    bool loaded_, alias_, checked_;
    std::string file_, area_, code_;
};

}  // namespace jamrules
