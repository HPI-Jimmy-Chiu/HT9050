// =============================================================================
//  test_st02_w219_temp913.cpp -- W-219: the Temperature / SYSTEM_TEMPERATURE generators read golden 913 (RULINGS_20261008 #3), so the
//  generated files carry 913's L07 "ATC second pre-compensation" (JerryYang 20260917) on top of Ifor01's data model (!420).
//
//  AI(W906-W219) 20261010 (St02-E).  Suite name (add_test): St02_W219Temp913.  argv[1] = port root (read only).  Memory only.
//    [1] tools/golden_root.py: a third tree '913' (V913_SHOWN, PINS_913) and KEEP_913 = {Temperature, SYSTEM_TEMPERATURE}; neither is left
//        in KEEP_V912.
//    [2] FileRW/Temperature.gen.inc was generated from golden 913 (header) and carries L07: the TS_TMyTempPanel adapter binds
//        ed2ndPreOffset / ed2ndPreOfsTime / ed2ndAfterOfs (golden 913 MyTempPanel.h:45-47), ReadTempFile reads [ATC] bATCPreOffset
//        (golden 913 uTemp_Set.cpp:2251-2262 block) and SaveSetupFile writes it (913 :4985), DoIniDataToForm fills ed2ndPreOffset from
//        fTempOffSet[PreOffset2nd].
//    [3] JsonBridge/gen/sjson_SYSTEM_TEMPERATURE.gen.cpp lists !420's d2ndATCPreOffset / i2ndATCPreOfsTime / d2ndATCAfterOfs and maps
//        bATCPreOffset to [ATC] bATCPreOffset.
//    [4] TMyTempPanel builds the three ed2nd* edits (MyTempPanel.cpp ctor), so the adapter never binds a NULL.
// =============================================================================
#include "MyTempPanel.h"
#include "cmydef.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
std::string Slurp(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
bool Has(const std::string& s, const char* n) { return s.find(n) != std::string::npos; }
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W219Temp913 -- Temperature / SYSTEM_TEMPERATURE generated from golden 913 (W-219)\n");
    const std::string root = argc > 1 ? argv[1] : std::string();

    std::printf("[1] tools/golden_root.py: tree 913, KEEP_913\n");
    {
        const std::string g = Slurp(root + "/tools/golden_root.py");
        Check(Has(g, "V913_SHOWN = r'D:\\HT9045\\HT9011UC_Code_V3.33.913.0_20261008_steven'") && Has(g, "TREES = ('906', 'v912', '913')") &&
                  Has(g, "PINS_913 = {'uTemp_Set.cpp':"),
              "[1] golden_root knows tree 913 (path, TREES, PINS_913) (read " + std::to_string(g.size()) + " bytes)");
        Check(Has(g, "KEEP_913 = {'Temperature':") && Has(g, "'SYSTEM_TEMPERATURE': 'W-219") &&
                  !Has(g, "    'Temperature': 'RULINGS_20261002 #20a") && !Has(g, "    'SYSTEM_TEMPERATURE': 'RULINGS_20261002 #20a"),
              "[1] KEEP_913 holds Temperature / SYSTEM_TEMPERATURE and KEEP_V912 no longer does");
    }

    std::printf("[2] FileRW/Temperature.gen.inc from golden 913, with L07\n");
    {
        const std::string t = Slurp(root + "/FileRW/Temperature.gen.inc");
        Check(Has(t, "golden D:\\HT9045\\HT9011UC_Code_V3.33.913.0_20261008_steven\\uTemp_Set.cpp"),
              "[2] header: generated from golden 913 uTemp_Set.cpp (read " + std::to_string(t.size()) + " bytes)");
        Check(Has(t, "*&ed2ndPreOffset, *&ed2ndPreOfsTime, *&ed2ndAfterOfs") && Has(t, "ed2ndPreOffset(q->ed2ndPreOffset)"),
              "[2] the TS_TMyTempPanel adapter binds ed2ndPreOffset / ed2ndPreOfsTime / ed2ndAfterOfs (golden 913 MyTempPanel.h:45-47)");
        Check(Has(t, "Temperature.bATCPreOffset      =CheckAndReadIniData(szDir, \"ATC\", \"bATCPreOffset\" , false);") &&
                  Has(t, "WriteIniData(szDir, \"ATC\", \"bATCPreOffset\" , EL<TCheckBox>(\"TfTemp_Set\", \"chkATCPreOffset\")->Checked);"),
              "[2] ReadTempFile reads and SaveSetupFile writes [ATC] bATCPreOffset (golden 913)");
        Check(Has(t, "myTempPal[i]->ed2ndPreOffset->Text  =FormatFloat(\"0.0\", Temperature.fTempOffSet[PreOffset2nd][i]);"),
              "[2] DoIniDataToForm fills ed2ndPreOffset from fTempOffSet[PreOffset2nd] (!420's 23 rows)");
    }

    std::printf("[3] JsonBridge/gen/sjson_SYSTEM_TEMPERATURE.gen.cpp\n");
    {
        const std::string s = Slurp(root + "/JsonBridge/gen/sjson_SYSTEM_TEMPERATURE.gen.cpp");
        Check(Has(s, "{ \"d2ndATCPreOffset\", offsetof(SYSTEM_TEMPERATURE, d2ndATCPreOffset)") &&
                  Has(s, "{ \"i2ndATCPreOfsTime\", offsetof(SYSTEM_TEMPERATURE, i2ndATCPreOfsTime)") &&
                  Has(s, "{ \"d2ndATCAfterOfs\", offsetof(SYSTEM_TEMPERATURE, d2ndATCAfterOfs)"),
              "[3] the three second pre-compensation arrays are fields (read " + std::to_string(s.size()) + " bytes)");
        Check(Has(s, "{ \"bATCPreOffset\", \"szDir\", \"ATC\", \"bATCPreOffset\", \"false\", false },"),
              "[3] bATCPreOffset maps to [ATC] bATCPreOffset (golden 913 SaveSetupFile)");
    }

    std::printf("[4] TMyTempPanel builds the ed2nd* edits\n");
    {
        TMyTempPanel* p = new TMyTempPanel("DUT 1", tcDUT1);
        Check(p->ed2ndPreOffset != 0 && p->ed2ndPreOfsTime != 0 && p->ed2ndAfterOfs != 0 && p->ed2ndPreOffset != p->edPreOffset,
              "[4] ed2ndPreOffset / ed2ndPreOfsTime / ed2ndAfterOfs are real, separate edits");
        delete p;
    }

    std::printf("St02_W219Temp913: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
