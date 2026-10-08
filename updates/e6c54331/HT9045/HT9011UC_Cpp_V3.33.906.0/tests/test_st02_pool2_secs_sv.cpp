// =============================================================================
//  test_st02_pool2_secs_sv.cpp -- POOL-2 SECSGEM/uHGemHT9045_SV.cpp: GATE [G13] and [G18] retired (golden 0618 :439-445 / :882-901),
//  GATE [G17] stays closed with its reason refreshed (no writer for fGroundMan->labValue_*).
//
//  AI(W906-POOL2-SV) 20261008 (St02-E).  Suite name (add_test): St02_Pool2SecsSv.  argv[1] = port root (reads tools/wb_serve.cpp).
//  Built like tests/test_secs_catalogue.cpp [3] (THGem + HGem + CosFunction.bEnable_SECS_GEM + HT9045Gem::AddSV), plus the boot call that
//  builds fTrayAssignment (tools/wb_serve.cpp:4062; golden's CreateForm runs before AddSV).  In memory only: no file is written.
//    [1] G13: SVID 2665/2666/2667/2754/2755/2756 registered once each, ASCII, VCL_NAME "1", pointer = fTrayAssignment->edAuto4..6/1..3Type,
//        name = golden 0618's string.
//    [2] G18: SVID 43350..43369 registered once each, ASCII, VCL_NAME "0", pointer = &fSmartDiagnostic->iPushAvgTime[0..9] /
//        &iPopAvgTime[0..9]; reading follows golden's AnsiString(Ptr) on the int (0x41 -> "A", 0 -> "").
//    [3] G17 still closed: SVID 43300..43327 not registered.
//    [4] tools/wb_serve.cpp builds fTrayAssignment before it hooks the SV catalogue publisher (the catalogue would deref NULL otherwise).
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"     // THGem, extern THGem *HGem
#include "SECSGEM/uHGemHT9045.h"        // HT9045Gem
#include "SECSGEM/SecsSvEcRegistration.h"
#include "SECSGEM/SecsSvRead.h"
#include "CosFunction.h"
#include "forms/fTrayAssignment.h"
#include "forms/fSmartDiagnostic.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

void W906_BootCreateTrayAssignment();   // forms/fTrayAssignment.cpp:1577

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
// every row of SV_ID equal to `id`; returns the count, `row` = the last one
int Rows(SecsSvEcRegistration& r, int id, int* row)
{
    int n = 0;
    const AnsiString want = AnsiString(id);
    for (int i = 0; i < r.SV_ID->Count; ++i)
        if (AnsiString(r.SV_ID->Strings[i]) == want) { ++n; *row = i; }
    return n;
}
std::string S(TStringList* l, int i) { return std::string(AnsiString(l->Strings[i]).c_str()); }
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_Pool2SecsSv -- POOL-2 SECSGEM/uHGemHT9045_SV.cpp: [G13] / [G18] retired, [G17] still closed\n");
    const std::string root = argc > 1 ? argv[1] : std::string();

    THGem gem;
    HGem = &gem;
    CosFunction.bEnable_SECS_GEM = true;
    W906_BootCreateTrayAssignment();
    Check(fTrayAssignment != 0 && fSmartDiagnostic != 0, "setup: fTrayAssignment built by the boot call, fSmartDiagnostic by static init");
    HT9045Gem h(AnsiString(""), &gem);
    h.AddSV();
    SecsSvEcRegistration& r = gem.SvEcReg;
    std::printf("  AddSV registered %d SVs\n", r.SV_ID->Count);

    // ---------------------------------------------------------------- [1]
    std::printf("[1] G13 -- golden 0618 SECSGEM/uHGemHT9045_SV.cpp:439-445\n");
    {
        struct G13 { int id; TEdit* ed; const char* name; } t[] = {
            {2665, fTrayAssignment->edAuto4Type, "Auto 4 Tray From Alias"}, {2666, fTrayAssignment->edAuto5Type, "Auto 5 Tray From Alias"},
            {2667, fTrayAssignment->edAuto6Type, "Auto 6 Tray From Alias"}, {2754, fTrayAssignment->edAuto1Type, "Auto 1 Tray From Alias"},
            {2755, fTrayAssignment->edAuto2Type, "Auto 2 Tray From Alias"}, {2756, fTrayAssignment->edAuto3Type, "Auto 3 Tray From Alias"},
        };
        for (const G13& g : t) {
            int row = -1;
            const int n = Rows(r, g.id, &row);
            const bool ok = n == 1 && S(r.SV_TYPE, row) == "64" && S(r.VCL_NAME, row) == "1" && r.SV_Ptr->Items[row] == (void*)g.ed &&
                            S(r.SV_NAME, row) == g.name;
            Check(ok, "[1] SVID " + std::to_string(g.id) + " once, ASCII, TObject* = fTrayAssignment's edit, name \"" + g.name + "\"" +
                          (n == 1 ? "" : " (registered " + std::to_string(n) + "x)"));
        }
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] G18 -- golden 0618 SECSGEM/uHGemHT9045_SV.cpp:882-901\n");
    {
        int good = 0, row = -1;
        for (int i = 0; i < 20; ++i) {
            const int id = 43350 + i;
            int* const want = i < 10 ? &fSmartDiagnostic->iPushAvgTime[i] : &fSmartDiagnostic->iPopAvgTime[i - 10];
            if (Rows(r, id, &row) == 1 && S(r.SV_TYPE, row) == "64" && S(r.VCL_NAME, row) == "0" && r.SV_Ptr->Items[row] == (void*)want)
                ++good;
            else
                std::printf("    SVID %d: not registered once / wrong type, kind or pointer\n", id);
        }
        Check(good == 20, "[2] SVID 43350..43369 once each, ASCII, raw pointer = &iPushAvgTime[0..9] / &iPopAvgTime[0..9] (" +
                              std::to_string(good) + "/20)");
        if (Rows(r, 43350, &row) == 1) {
            const int saved = fSmartDiagnostic->iPushAvgTime[0];
            AnsiString v, why;
            fSmartDiagnostic->iPushAvgTime[0] = 0x41;
            const bool a = ht9045::SecsSvReadValue(r, row, v, why) && std::string(v.c_str()) == "A";
            fSmartDiagnostic->iPushAvgTime[0] = 0;
            const bool z = ht9045::SecsSvReadValue(r, row, v, why) && std::string(v.c_str()).empty();
            fSmartDiagnostic->iPushAvgTime[0] = saved;
            Check(a && z, "[2] SVID 43350 reads the member through golden's AnsiString(Ptr): 0x41 -> \"A\", 0 -> \"\"");
        } else {
            Check(false, "[2] SVID 43350 read-back (not registered)");
        }
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] G17 stays closed (fGroundMan->labValue_* has no writer)\n");
    {
        int seen = 0, row = -1;
        for (int id = 43300; id <= 43327; ++id) seen += Rows(r, id, &row);
        Check(seen == 0, "[3] SVID 43300..43327 not registered (" + std::to_string(seen) + ")");
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] tools/wb_serve.cpp: fTrayAssignment is built before the SV catalogue publisher is hooked\n");
    {
        std::ifstream f((root + "/tools/wb_serve.cpp").c_str(), std::ios::binary);
        std::ostringstream ss;
        ss << f.rdbuf();
        const std::string ws = ss.str();
        const size_t boot = ws.find("W906_BootCreateTrayAssignment(); W906_BootCreateTrayAssignment(); }");
        const size_t hook = ws.find("ht9045::SetExtraTagPublisher(&PublishExtraTags);");
        Check(!ws.empty() && boot != std::string::npos && hook != std::string::npos && boot < hook,
              "[4] the boot call comes before SetExtraTagPublisher(&PublishExtraTags) (read " + std::to_string(ws.size()) + " bytes)");
    }

    HGem = NULL;
    std::printf("St02_Pool2SecsSv: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
