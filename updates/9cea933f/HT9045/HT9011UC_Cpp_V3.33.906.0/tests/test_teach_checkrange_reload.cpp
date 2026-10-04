// =============================================================================
//  tests/test_teach_checkrange_reload.cpp
//
//  AI(W906-KB-GOLDEN) 20261004 [W906]: the Teach page's two FormShow-only texts, edShtCheckRange and InSHZDownRange, follow
//  golden: FormShow fills them on a real window open, a Save's reload never does. FileRW/Teach.cpp MirrorToProxies (:84) used
//  to mix golden ReadFile's mirror with those two FormShow lines and ran after every Save (:195) and on every editlist.get
//  (:258): after a Save, edShtCheckRange showed the OLD runtime CHECK_RANGE again (the Save writes Gerneral.ini only, :190;
//  CHECK_RANGE is re-read only by ReadTechData, cinitial.cpp:16139, via SetWorkParameter) and the next Save wrote it back.
//
//  Golden (V906 uteach.cpp, Big5, read only):
//    FormShow     :1413 -> :1419 ReadFile() ... :1753 edShtCheckRange->Text=CHECK_RANGE;
//                 :1956-1961 if(In_Shuttle_Auto_Latch==eInSHAutoLtc){ ... InSHZDownRange->Text=iInShtZRange; }
//    btnSaveClick :2261 -> :2266-2270 MessageDlg only while fAllMotorHome==false, No returns at once -> :2279 CHECK_RANGE from the
//                 edit into Gerneral.ini only -> :2280-2281 iInShtZRange written AND re-read -> :2282 SaveFile -> :2283 ReadFile()
//                 -- no FormShow: the typed text stays on screen until the form is shown again
//    ReadFile     :4781-4920: TECH_* ReadFromFile mirrors (:115, :187-188, :229), backlash :4870-4871 -- never the two edits
//    main.cpp:27824-27853 sbTeachingClick: NewRecordProcess("MES2189","Enter Teach Form") then ShowModal (-> FormShow) -- the
//                 window-open edge the port reads from filerw::OpenEnterRecord's return (FileRW/Teach.cpp:256).
//
//  The path, as wb_serve runs it (god-stack linked, nothing in between is a stand-in): FileRW_Teach_Page (WS editlist.get
//  tag=Teach), FileRW_Teach_Save (editlist.save), FileRW_Teach_MirrorProxies (GearRatioLive.cpp:156), the page table's close
//  edge W906_EditPageWindowClosed("fTeach") with the edges armed as tools/wb_serve.cpp:4389 does (W906_EditPageWindowEdgesArm),
//  and the real ReadTechData (cinitial.cpp:16079) as the boot read and as "CHECK_RANGE re-read".
//
//  argv[1] = the HT9050 snapshot teach.ini, READ ONLY (the GearTeachSave fixture; it has [Teach INI] Update2=1 and
//            [MInShuttle1] / [MInShuttle2], so ReadFile never reads d:\HT9045\system\tech.dat)
//  argv[2] = the snapshot Gerneral.ini, READ ONLY (the config keys the teach registry / ReadFile branch on, read by this
//            test's own parser, as tests/test_gear_teach_save.cpp)
//  argv[3] = the scratch directory (CMake: <build>/tests/machine_config_scratch/TeachCheckRangeReload); asTeachPath =
//            <argv[3]>\teach.ini, a copy of argv[1].
//  Gerneral.ini = asGeneralPath = ctest's per-test sandbox (W906_GENERAL_INI_PATH, tests/CMakeLists.txt ENV-ALL, seeded by
//  tests/test_bootstrap.cpp). Refused (exit 2) before any Handler code runs unless both are in ctest's scratch
//  (tests/w906_ctest_guard.h): no D:\HT9045\system file is written.
//
//  Locks:
//   [1] the first open: edShtCheckRange = the runtime CHECK_RANGE (old), InSHZDownRange = iInShtZRange (latch installed)
//   [2] old+100 / z+7 typed -> Save (YES): the sandbox Gerneral.ini has them; the runtime CHECK_RANGE is still old (golden
//       :2279), iInShtZRange is z+7 (golden :2281); the proxies keep the typed texts after the Save's own ReadFile + mirror
//   [3] the engine's automatic reload and the reload button (editlist.get, same window-open): the page reads old+100 / z+7, not
//       the stale runtime; the ReadFile half still runs (a TECH proxy and a backlash proxy set to junk come back), the
//       FormShow half does not (InSHZDownRange is not refilled from a changed iInShtZRange)
//   [4] FileRW_Teach_MirrorProxies (the Gear Ratio reload): the ReadFile half only -- old+100 stays, the junk TECH proxy is fixed
//   [5] a second Save with no widget sent writes old+100 again (before the fix: the stale old value went back to Gerneral.ini)
//   [6] answer NO: nothing written; the proxies go back to their text before that save (old+100 / z+7), not to the stale runtime
//   [7] the window closed -> a fresh open WITHOUT a re-read shows the runtime CHECK_RANGE = old, exactly what golden FormShow
//       :1753 shows (golden quirk, kept); latch not installed -> InSHZDownRange keeps its text (golden :1956 false)
//   [8] ReadTechData (the re-read, cinitial.cpp:16139) -> the window closed -> a fresh open shows old+100 and z+7
//  Control (reasoned from the code, NOT run): against the old MirrorToProxies, which wrote CHECK_RANGE on every call, the [2]
//  proxy check, the [3] reload checks, [4] edShtCheckRange, [5] and [6] proxies must go red (they all see the stale old value).
// =============================================================================
#include "forms/fTeach.h"
#include "forms/fTeachPara.h"
#include "Public/HTEditList.h"
#include "Public/cJSON.h"
#include "FileRW/_EditList.h"
#include "common.h"
#include "cmydef.h"
#include "cinitial.h"
#include "MachineType.h"
#include "w906_ctest_guard.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <direct.h>      // _mkdir

void InitialMotorName();                  // cinitial.cpp:3295 (no header declares it; as tests/test_gear_teach_save.cpp:69)
void FileRW_Teach_Boot();                 // FileRW/Teach.cpp:247 (wb_serve boot: elTeach + golden InitialTeachEditList)
int  FileRW_Teach_Page(std::string* json);                                      // FileRW/Teach.cpp:253 (tools/wb_serve.cpp:129)
int  FileRW_Teach_Save(const std::string& widgetsJson, const std::string& answersJson, std::string* ack, std::string* err);   // :311
void FileRW_Teach_MirrorProxies();        // FileRW/Teach.cpp:376
void W906_EditPageWindowEdgesArm();       // FileRW/_EditPage.cpp:1169 (tools/wb_serve.cpp:4389 arms it at boot)
void W906_EditPageWindowClosed(const char* goldenForm);   // FileRW/_EditPage.cpp:1170 (the page table's close edge)

// Link-only, NOT under test (the same stand-ins and reasons as tests/test_gear_teach_save.cpp:76-83): cprod.cpp (ht9045_globals)
//   calls FileRW_IniConfig_ChangeCBListProperty, whose body is FileRW/IniConfig.cpp (not compiled here) -- without this the linker
//   pulls FileRW/_fallback.cpp, a duplicate of FileRW_ProxyChecked in the real FileRW/_EditList.cpp; the FormJson lock
//   (JsonBridge/FormJson.cpp) is wb_serve only and this test has one thread.
void FileRW_IniConfig_ChangeCBListProperty() {}
namespace ht9045 { namespace formjson { void FormLock() {} void FormUnlock() {} } }

namespace {

int g_pass = 0;
int g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", std::string(msg).c_str()); ++g_pass; } \
        else      { printf("  FAIL: %s  (line %d)\n", std::string(msg).c_str(), __LINE__); ++g_fail; } \
    } while (0)

const char* const kForm  = "TfTeach";                                          // FileRW/Teach.cpp:57
const char* const kSaveQ = "Sure to Save? (確定要存檔?)";                      // FileRW/Teach.cpp:60 = golden uteach.cpp:2268 (the page's answer key)

std::string ReadAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
bool CanRead(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); return f.good(); }
bool WriteAll(const std::string& p, const std::string& d)
{
    std::ofstream f(p.c_str(), std::ios::binary | std::ios::trunc);
    f.write(d.data(), (std::streamsize)d.size());
    return f.good();
}
std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}
std::string Norm(std::string s)                                                // lower case, backslashes (path compare)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return Lower(s);
}
std::string I2S(long long v) { return std::to_string(v); }                    // not "%lld": MinGW 6.3's msvcrt printf has no ll

// an INI text as this test reads it, independent of the writer: (lower section, lower key) -> value, trailing blanks cut
typedef std::map<std::pair<std::string, std::string>, std::string> IniMap;
IniMap ParseIni(const std::string& t)
{
    IniMap m;
    std::string sec;
    std::size_t b = 0;
    while (b < t.size()) {
        std::size_t e = t.find('\n', b);
        if (e == std::string::npos) e = t.size();
        std::string line = t.substr(b, e - b);
        b = e + 1;
        while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == ' ' || line[line.size() - 1] == '\t')) line.erase(line.size() - 1);
        if (line.empty()) continue;
        if (line[0] == '[') {
            const std::size_t c = line.find(']');
            sec = Lower(line.substr(1, c == std::string::npos ? std::string::npos : c - 1));
            continue;
        }
        const std::size_t q = line.find('=');
        if (q == std::string::npos) continue;
        m[std::make_pair(sec, Lower(line.substr(0, q)))] = line.substr(q + 1);
    }
    return m;
}
std::string IniGet(const IniMap& m, const char* sec, const char* key)
{
    IniMap::const_iterator it = m.find(std::make_pair(Lower(sec), Lower(key)));
    return it == m.end() ? std::string("(missing)") : it->second;
}
std::string GenIni(const char* key)                                        // the sandbox Gerneral.ini [Shuttle] <key>, read from disk
{
    return IniGet(ParseIni(ReadAll(asGeneralPath.c_str())), "Shuttle", key);
}

// the page's view after an editlist.get: lists.<list>.entries[] id -> text (FileRW/Teach.cpp FileRW_Teach_Page, EntryList)
typedef std::map<std::string, std::string> Texts;
Texts PageTexts(const std::string& json, const char* list)
{
    Texts m;
    cJSON* root = cJSON_Parse(json.c_str());
    const cJSON* lists = root ? cJSON_GetObjectItemCaseSensitive(root, "lists") : nullptr;
    const cJSON* l = lists ? cJSON_GetObjectItemCaseSensitive(lists, list) : nullptr;
    const cJSON* e = l ? cJSON_GetObjectItemCaseSensitive(l, "entries") : nullptr;
    for (const cJSON* it = e ? e->child : nullptr; it; it = it->next) {
        const cJSON* id = cJSON_GetObjectItemCaseSensitive(it, "id");
        const cJSON* tx = cJSON_GetObjectItemCaseSensitive(it, "text");
        if (cJSON_IsString(id) && cJSON_IsString(tx)) m[id->valuestring] = tx->valuestring;
    }
    if (root) cJSON_Delete(root);
    return m;
}
std::string RowText(const Texts& m, const std::string& id)
{
    Texts::const_iterator it = m.find(id);
    return it == m.end() ? std::string("(no row)") : it->second;
}

// the editlist.save ack: saved, and the golden question(s) asked with the answer used (FileRW/_EditList.cpp SessionJson)
struct Ack { bool parsed = false; bool saved = false; int asked = 0; int answer = -1; };
Ack ParseAck(const std::string& a)
{
    Ack r;
    cJSON* root = cJSON_Parse(a.c_str());
    if (!root) return r;
    r.parsed = true;
    r.saved = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(root, "saved")) != 0;
    const cJSON* s = cJSON_GetObjectItemCaseSensitive(root, "session");
    const cJSON* q = s ? cJSON_GetObjectItemCaseSensitive(s, "asked") : nullptr;
    for (const cJSON* it = q ? q->child : nullptr; it; it = it->next) {
        if (++r.asked != 1) continue;
        const cJSON* n = cJSON_GetObjectItemCaseSensitive(it, "answer");
        if (cJSON_IsNumber(n)) r.answer = n->valueint;
    }
    cJSON_Delete(root);
    return r;
}

// WS editlist.get tag=Teach / editlist.save tag=Teach, as tools/wb_serve.cpp:5211 / :5325 call them
std::string TeachPage(const char* what, int* st)
{
    std::string json;
    *st = FileRW_Teach_Page(&json);
    printf("  editlist.get (%s): %d, %u bytes\n", what, *st, (unsigned)json.size());
    return json;
}
Ack TeachSave(const char* what, const std::string& widgets, const std::string& answers, int* st)
{
    std::string ack, err;
    *st = FileRW_Teach_Save(widgets, answers, &ack, &err);
    const Ack a = ParseAck(ack);
    printf("  editlist.save (%s): %d, saved=%d, asked=%d, answer=%d%s%s\n", what, *st, (int)a.saved, a.asked, a.answer,
           err.empty() ? "" : ", err=", err.c_str());
    return a;
}
std::string TypedWidgets(int sht, int z)
{
    return "{\"edShtCheckRange\":{\"text\":\"" + I2S(sht) + "\"},\"InSHZDownRange\":{\"text\":\"" + I2S(z) + "\"}}";
}

// the named proxies themselves (FileRW/_EditList.h EL: the same object FileRW/Teach.cpp's P() returns)
std::string PxText(const std::string& name) { return filerw::EL<TEdit>(kForm, name.c_str())->Text.c_str(); }
void PxSet(const std::string& name, const char* text) { filerw::EL<TEdit>(kForm, name.c_str())->Text = AnsiString(text); }

}  // namespace

int main(int argc, char** argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc < 4 || !CanRead(argv[1]) || !CanRead(argv[2])) {
        printf("FAIL: argv[1] = the snapshot teach.ini, argv[2] = the snapshot Gerneral.ini (both read only), argv[3] = the scratch directory\n");
        return 1;
    }
    const std::string src = argv[1], cfgPath = argv[2];
    std::string dir = argv[3];
    for (std::size_t i = 0; i < dir.size(); ++i) if (dir[i] == '/') dir[i] = '\\';
    while (!dir.empty() && dir[dir.size() - 1] == '\\') dir.erase(dir.size() - 1);
    const std::string copy = dir + "\\teach.ini";
    {
        const char* const rt[] = { "asGeneralPath", asGeneralPath.c_str(), "asTeachPath (the copy)", copy.c_str(), 0 };
        if (!W906TestRequireCtestRedirects("TeachCheckRangeReload", rt)) return 2;
        if (Norm(copy) == Norm(src)) { printf("REFUSED: the copy %s is argv[1] itself (the snapshot stays read only)\n", copy.c_str()); return 2; }
    }
    for (std::size_t i = 3; i <= dir.size(); ++i) if (i == dir.size() || dir[i] == '\\') _mkdir(dir.substr(0, i).c_str());
    std::remove(copy.c_str());

    const std::string bytes0 = ReadAll(src);
    printf("source (read only) %s  %u bytes\ncopy (written)     %s\nGerneral.ini      %s\n", src.c_str(), (unsigned)bytes0.size(), copy.c_str(), asGeneralPath.c_str());
    CHECK(!bytes0.empty() && WriteAll(copy, bytes0) && ReadAll(copy) == bytes0, "setup: the snapshot teach.ini copied into the scratch directory, byte for byte");
    {
        const IniMap t0 = ParseIni(bytes0);
        bool sh1 = false, sh2 = false;
        for (IniMap::const_iterator it = t0.begin(); it != t0.end(); ++it) {
            if (it->first.first == "minshuttle1") sh1 = true;
            if (it->first.first == "minshuttle2") sh2 = true;
        }
        // forms/fTeachPara.cpp TfTeach::ReadFile: without [Teach INI] Update2=1 or [MInShuttle1] / [MInShuttle2] it reads the
        //   machine's d:\HT9045\system\tech.dat (golden's literal) -- this test refuses that input (as GearTeachSave).
        if (IniGet(t0, "Teach INI", "Update2") != "1" || !sh1 || !sh2) {
            printf("REFUSED: %s lacks [Teach INI] Update2=1 or [MInShuttle1] / [MInShuttle2] (ReadFile would read d:\\HT9045\\system\\tech.dat)\n", src.c_str());
            return 2;
        }
    }
    asTeachPath = AnsiString(copy.c_str());
    OpenGeneralIniFile();                                                       // INIFileGeneral on asGeneralPath (the sandbox); the Save's WriteIniDataGeneral needs it
    CHECK(std::string(asTeachPath.c_str()) == copy, "asTeachPath = the scratch copy");
    CHECK(InitialOK == false && SystemStart == false, "InitialOK and SystemStart are false: InitShuttleThreadParameter (the Save's tail) returns at once, the open clears fAllMotorHome");

    // ---- the machine's configuration (the keys the teach registry / ReadFile branch on), as tests/test_gear_teach_save.cpp ----
    {
        const IniMap cfg = ParseIni(ReadAll(cfgPath));
        struct K { const char* sec; const char* key; int* dst; };
        const K keys[] = {
            { "system", "IO_CARD_TYPE", &IO_CARD_TYPE },
            { "outsortarm", "USE_OUT_SORT_ARM", &USE_OUT_SORT_ARM },
            { "system", "USE_PICKER_COUNT", &USE_PICKER_COUNT },
            { "system", "AUTO_EMPTY_COLOR", &AUTO_EMPTY_COLOR },
            { "system", "INOUT_ARM_PICKER_USE_MOTOR", &InOutArmPickerUseMotor },
            { "indexdriver", "USE_INDEX_ARM_AXES", &USE_INDEX_ARM_AXES },
            { "system", "INSTALL_OCR_YMot", &INSTALL_OCR_YMot },
            { "system", "USE_IN_OUT_ARM_Y_PITCH", &USE_IN_OUT_ARM_Y_PITCH },
            { "system", "USE_OUT_ARM_Y_PITCH", &USE_OUT_ARM_Y_PITCH },
            { "system", "CUSTOMER_CODE", &CUSTOMER_CODE },
        };
        int missing = 0;
        std::string got;
        for (std::size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
            const std::string v = IniGet(cfg, keys[i].sec, keys[i].key);
            if (v == "(missing)") { ++missing; printf("    config key [%s] %s missing\n", keys[i].sec, keys[i].key); continue; }
            *keys[i].dst = std::atoi(v.c_str());
            got += std::string(got.empty() ? "" : ", ") + keys[i].key + "=" + v;
        }
        const std::string aoa = IniGet(cfg, "system", "MACHINE_HAS_AUTO_ALIGNMENT_CCD");
        if (aoa == "(missing)") ++missing; else MACHINE_HAS_AUTO_ALIGNMENT_CCD = std::atoi(aoa.c_str()) != 0;
        printf("    config: %s, MACHINE_HAS_AUTO_ALIGNMENT_CCD=%d\n", got.c_str(), (int)MACHINE_HAS_AUTO_ALIGNMENT_CCD);
        CHECK(missing == 0, "the eleven config keys read from the snapshot Gerneral.ini (the registry of the machine the teach.ini came from)");
    }

    // ---- wb_serve's boot order: MOT names, the TfTeach facade, elTeach + InitialTeachEditList, the page table's edges, ReadTechData ----
    InitialMotorName();                                                         // MOT[].Alias = the teach.ini section names
    fTeach = new TfTeach();
    FileRW_Teach_Boot();
    W906_EditPageWindowEdgesArm();                                              // tools/wb_serve.cpp:4389: from here on a close edge makes the next editlist.get a window open
    CHECK(fTeach != 0 && !fTeach->TechPara.empty() && elTeach != 0 && elTeach->FEditList->Count > 0,
          "the facade's registry and elTeach exist (" + I2S((long long)fTeach->TechPara.size()) + " TechPara, " + I2S(elTeach ? elTeach->FEditList->Count : 0) + " elTeach)");
    ReadTechData();                                                             // the boot read (SetWorkParameter): Teach.* + CHECK_RANGE / iInShtZRange (cinitial.cpp:16139-16140)
    const int old = CHECK_RANGE, z0 = iInShtZRange;
    const int typed = old + 100, zTyped = z0 + 7;
    printf("    boot: CHECK_RANGE=%d, iInShtZRange=%d (the test types %d / %d)\n", old, z0, typed, zTyped);
    CHECK(GenIni("CHECK_RANGE") == I2S(old) && GenIni("iInShtZRange") == I2S(z0),
          "boot: ReadTechData read [Shuttle] CHECK_RANGE=" + I2S(old) + ", iInShtZRange=" + I2S(z0) + " from the sandbox Gerneral.ini");
    In_Shuttle_Auto_Latch = eInSHAutoLtc;                                       // golden :1956 true (the In Shuttle latch machine) until [7]
    const std::string yes = std::string("{\"") + kSaveQ + "\":1}";
    int st = 0;

    // ---- [1] the first open ----
    printf("[1] the first open (golden sbTeachingClick -> ShowModal -> FormShow)\n");
    CHECK(PxText("edShtCheckRange").empty() && PxText("InSHZDownRange").empty(), "[1] before it both proxies are empty (golden uteach.dfm stores no Text for either)");
    const std::string j1 = TeachPage("first open", &st);
    const Texts g1 = PageTexts(j1, "general"), t1 = PageTexts(j1, "techPara");
    CHECK(st == 200 && PxText("edShtCheckRange") == I2S(old) && RowText(g1, "edShtCheckRange") == I2S(old),
          "[1] edShtCheckRange = the runtime CHECK_RANGE " + I2S(old) + " (golden FormShow :1753); page: " + RowText(g1, "edShtCheckRange"));
    CHECK(PxText("InSHZDownRange") == I2S(z0) && RowText(g1, "InSHZDownRange") == I2S(z0),
          "[1] InSHZDownRange = iInShtZRange " + I2S(z0) + " (golden :1956-1961, latch installed); page: " + RowText(g1, "InSHZDownRange"));
    std::string probe;                                                          // a TECH_PARA proxy the ReadFile mirror writes (FileRW/Teach.cpp:88-94)
    for (Texts::const_iterator it = t1.begin(); it != t1.end() && probe.empty(); ++it) if (!it->second.empty()) probe = it->first;
    CHECK(!probe.empty() && RowText(g1, "edtEditRotateInBacklash") != "(no row)", "[1] the page lists TECH_PARA rows (probe " + probe + "=" + RowText(t1, probe) + ") and the backlash row");

    // ---- [2] type old+100 / z+7, Save, answer YES ----
    printf("[2] edShtCheckRange %d -> %d, InSHZDownRange %d -> %d, Save (YES)\n", old, typed, z0, zTyped);
    Ack a = TeachSave("typed values, YES", TypedWidgets(typed, zTyped), yes, &st);
    CHECK(st == 200 && a.parsed && a.saved && a.asked == 1 && a.answer == 1, "[2] golden asked (fAllMotorHome cleared by the open, :2266-2270), YES, saved");
    CHECK(GenIni("CHECK_RANGE") == I2S(typed) && GenIni("iInShtZRange") == I2S(zTyped),
          "[2] the sandbox Gerneral.ini [Shuttle] CHECK_RANGE=" + GenIni("CHECK_RANGE") + ", iInShtZRange=" + GenIni("iInShtZRange") + " (golden :2279-2280)");
    CHECK(CHECK_RANGE == old, "[2] the runtime CHECK_RANGE is still " + I2S(old) + " (golden :2279 writes the ini only; ReadTechData re-reads it, cinitial.cpp:16139) -- the stale value the old mirror put back; now " + I2S(CHECK_RANGE));
    CHECK(iInShtZRange == zTyped, "[2] iInShtZRange = " + I2S(iInShtZRange) + " (golden :2281 re-reads it after the write)");
    CHECK(PxText("edShtCheckRange") == I2S(typed) && PxText("InSHZDownRange") == I2S(zTyped),
          "[2] after the Save's own ReadFile + mirror (FileRW/Teach.cpp:194-195) the proxies keep " + PxText("edShtCheckRange") + " / " + PxText("InSHZDownRange") + " (golden :2283: ReadFile, no FormShow)");

    // ---- [3] the engine's automatic reload, then the reload button: same window-open ----
    printf("[3] reloads in the same window-open (web/page/ht9045_wire_engine.js rule 3, the reload button)\n");
    const std::string jA = TeachPage("automatic reload after the save", &st);
    const Texts gA = PageTexts(jA, "general"), tA = PageTexts(jA, "techPara");
    CHECK(st == 200 && RowText(gA, "edShtCheckRange") == I2S(typed),
          "[3] the reload shows CHECK_RANGE " + RowText(gA, "edShtCheckRange") + " = typed and saved, not the stale runtime " + I2S(old) + " (golden: the edit keeps the typed text)");
    CHECK(RowText(gA, "InSHZDownRange") == I2S(zTyped), "[3] ... and InSHZDownRange " + RowText(gA, "InSHZDownRange"));
    PxSet(probe, "junk");
    PxSet("edtEditRotateInBacklash", "junk");
    iInShtZRange = zTyped + 500;                                                // what the FormShow half would show if it ran
    const std::string jB = TeachPage("reload button", &st);
    const Texts gB = PageTexts(jB, "general"), tB = PageTexts(jB, "techPara");
    CHECK(st == 200 && RowText(tA, probe) != "junk" && RowText(tB, probe) == RowText(tA, probe),
          "[3] the ReadFile half ran on the reload: TECH proxy " + probe + " junk -> " + RowText(tB, probe) + " (golden ReadFromFile :115)");
    CHECK(RowText(gA, "edtEditRotateInBacklash") != "junk" && RowText(gB, "edtEditRotateInBacklash") == RowText(gA, "edtEditRotateInBacklash"),
          "[3] ... and the backlash proxy junk -> " + RowText(gB, "edtEditRotateInBacklash") + " (golden ReadFile :4870)");
    CHECK(RowText(gB, "edShtCheckRange") == I2S(typed) && RowText(gB, "InSHZDownRange") == I2S(zTyped),
          "[3] the FormShow half did not run: edShtCheckRange " + RowText(gB, "edShtCheckRange") + ", InSHZDownRange " + RowText(gB, "InSHZDownRange") + " although iInShtZRange is " + I2S(iInShtZRange));
    iInShtZRange = zTyped;

    // ---- [4] the Gear Ratio reload ----
    printf("[4] FileRW_Teach_MirrorProxies (GearRatioLive.cpp:156, after its own teach.ini write + ReadFile)\n");
    PxSet(probe, "junk");
    FileRW_Teach_MirrorProxies();
    CHECK(PxText(probe) == RowText(tA, probe), "[4] the ReadFile half: TECH proxy " + probe + " junk -> " + PxText(probe));
    CHECK(PxText("edShtCheckRange") == I2S(typed) && CHECK_RANGE == old,
          "[4] edShtCheckRange stays " + PxText("edShtCheckRange") + " (runtime CHECK_RANGE " + I2S(CHECK_RANGE) + "): a Gear Ratio save is not a Teach window open");

    // ---- [5] a second Save, nothing sent ----
    printf("[5] a second Save, no widget sent (the proxies decide)\n");
    a = TeachSave("no widgets, YES", "{}", yes, &st);
    CHECK(st == 200 && a.saved && a.answer == 1, "[5] saved");
    CHECK(GenIni("CHECK_RANGE") == I2S(typed) && GenIni("iInShtZRange") == I2S(zTyped),
          "[5] Gerneral.ini keeps CHECK_RANGE=" + GenIni("CHECK_RANGE") + ", iInShtZRange=" + GenIni("iInShtZRange") + " (before the fix the stale " + I2S(old) + " went back here)");

    // ---- [6] answer NO ----
    printf("[6] typed 9999 / 8888, Save, answer NO\n");
    CHECK(fAllMotorHome == false, "[6] precondition: fAllMotorHome is false (the Save cleared it, golden :2286) -> golden asks");
    const std::string gen6 = ReadAll(asGeneralPath.c_str());
    a = TeachSave("typed 9999 / 8888, NO", TypedWidgets(9999, 8888), "{}", &st);
    CHECK(st == 200 && a.parsed && !a.saved && a.asked == 1 && a.answer == 2, "[6] asked, no answer = NO (FileRW/_EditList.cpp ELAsk), not saved");
    CHECK(ReadAll(asGeneralPath.c_str()) == gen6 && GenIni("CHECK_RANGE") == I2S(typed), "[6] Gerneral.ini untouched (golden :2269-2270 returns before :2279)");
    CHECK(PxText("edShtCheckRange") == I2S(typed) && PxText("InSHZDownRange") == I2S(zTyped),
          "[6] the proxies are back to their text before that save: " + PxText("edShtCheckRange") + " / " + PxText("InSHZDownRange") + " (not 9999 / 8888, not the stale runtime " + I2S(old) + ")");

    // ---- [7] the window closed, a fresh open, no re-read; latch not installed ----
    printf("[7] the window closed -> a fresh open without a re-read, In_Shuttle_Auto_Latch off\n");
    W906_EditPageWindowClosed("fTeach");                                        // the page table's close edge (tools/wb_serve.cpp:4389)
    In_Shuttle_Auto_Latch = 0;                                                  // eInSH8Sen (MachineType.h:1641)
    iInShtZRange = zTyped + 1000;
    const std::string j7 = TeachPage("fresh open, no re-read", &st);
    const Texts g7 = PageTexts(j7, "general");
    CHECK(st == 200 && CHECK_RANGE == old && RowText(g7, "edShtCheckRange") == I2S(old),
          "[7] edShtCheckRange = the runtime CHECK_RANGE " + RowText(g7, "edShtCheckRange") + ": golden FormShow :1753 shows the variable, which nothing re-read since the Save (golden quirk, kept)");
    CHECK(RowText(g7, "InSHZDownRange") == I2S(zTyped),
          "[7] latch off: InSHZDownRange keeps " + RowText(g7, "InSHZDownRange") + " although iInShtZRange is " + I2S(iInShtZRange) + " (golden :1956 false, :1961 skipped)");
    In_Shuttle_Auto_Latch = eInSHAutoLtc;
    iInShtZRange = zTyped;

    // ---- [8] the re-read, the window closed, a fresh open ----
    printf("[8] ReadTechData (CHECK_RANGE re-read) -> the window closed -> a fresh open\n");
    ReadTechData();
    CHECK(CHECK_RANGE == typed && iInShtZRange == zTyped,
          "[8] ReadTechData (cinitial.cpp:16139-16140, what SetWorkParameter / START runs) reads CHECK_RANGE=" + I2S(CHECK_RANGE) + ", iInShtZRange=" + I2S(iInShtZRange) + " back");
    W906_EditPageWindowClosed("fTeach");
    const std::string j8 = TeachPage("fresh open after the re-read", &st);
    const Texts g8 = PageTexts(j8, "general");
    CHECK(st == 200 && RowText(g8, "edShtCheckRange") == I2S(typed) && RowText(g8, "InSHZDownRange") == I2S(zTyped),
          "[8] the fresh open shows CHECK_RANGE " + RowText(g8, "edShtCheckRange") + " = the re-read value, InSHZDownRange " + RowText(g8, "InSHZDownRange") + " (golden FormShow :1753 / :1961)");

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
