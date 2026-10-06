// =============================================================================
//  tests/test_c14_bindisp_pane.cpp -- ST02-C14 part 3: the web "Bin Display Status" pane's C++ data path (golden 906_0625_Steven
//  TfShowBinSelect tsUnloadMap / ChangeBinDispStatus, cShowBinSelect.cpp:208-386).  AI(W906-ST02-C14) 20261002 (St02-E helper).
//  Suite: C14_BinDispPane (both configs).  Compiles ../WebBinDispStatus_St02.cpp directly (a wb_serve-only TU, the
//  test_sysinit_boot pattern) with -Wall -Wextra; god stack via RESCAN + ht9045_webbridge (TagSnapshot).
//
//  CONTAINMENT FIRST (st02_test_containment.h): exit 2 before any Handler code unless every log root and Gerneral.ini are
//  ctest's scratch.  Writes no file: a stand-in controller (a TMyBinDispOffline whose unit state the test sets) replaces the
//  real boot, its log goes to <machine_log_scratch>\BinDisplayLog and is never written; no COM port, no Timer1.
//    0. the ctest guard.
//    1. no controller (NUMBER_PANEL_TYPE 0): exactly 15 binsel.disp.* tags (= W906_BinDispStatusTagCount_St02), the
//       controller ones null, tabVisible false, 36 captions / 33 labels.
//    2. type 3, a unit in error: golden's early return (:279-292) leaves the panels unpainted without the scope; inside
//       W906_BinDispShownScope_St02 the call paints them (L orange, 1 green, 5 red, gray X, the black / red flash), the
//       status bar says "Bin display got error!!" on clRed, golden's :272 jump is counted (jumpSeq) and the facade keeps
//       ActivePageIndex 3 / ActivePage tsUnloadMap; a second call flips the flash colour and counts the jump again.
//    3. no error: no jump, the facade's old ActivePage / ActivePageIndex come back, the status bar is GetRunStatus() on
//       clBtnFace (0x8000000F as unsigned).
//    4. the labels, their colours, the group panels, and a TAB inside a caption (made a space).
//    5. source pins (argv[1] = the tree root, read only): the wb_serve claims, the CMake source line, MainTimer3.cpp's scope
//       line, the page include, the page's line count.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "database.h"
#include "common.h"
#include "forms/fShowBinSelect.h"
#include "BinDisplay/MyBinDisp.h"
#include "BinDisplay/BinDispBringUp_St02.h"
#include "Public/MyStringList.h"
#include "WebBinDispStatus_St02.h"
#include "WebBridge/TagSnapshot.h"
#include "st02_test_containment.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

// golden Graphics.hpp values (cShowBinSelect.cpp defines them TU-locally; ColorMap golden 906 :218)
static const unsigned kClGray = 0x00808080, kClRed = 0x000000FF, kClGreen = 0x00008000, kClOrange = 0x000080FF, kClBlack = 0;
static const unsigned kClBtnFace = 0x8000000Fu;

// a controller whose unit state the test sets (golden's getters read exactly these members, BinDisplay/MyBinDisp.cpp:269-277)
struct FakeBinDisp : TMyBinDispOffline
{
    void Unit(int i, bool installed, int color, int bin, bool error)
    {
        bHasUnitArray[i] = installed;
        iColorNow[i]     = color;
        iBinNow[i]       = bin;
        bHasError[i]     = error;
    }
};

static std::map<std::string, webbridge::TagValue> Stage(std::size_t* n)
{
    webbridge::TagSnapshot snap;
    snap.beginPublish();
    *n = W906_StageBinDispStatus_St02(snap);
    snap.commitPublish();
    return snap.read().tags;
}
static std::vector<std::string> Split(const std::string& s, char sep)
{
    std::vector<std::string> v;
    std::string cur;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == sep) { v.push_back(cur); cur.clear(); }
        else cur += s[i];
    }
    v.push_back(cur);
    return v;
}
static std::string Str(const std::map<std::string, webbridge::TagValue>& t, const char* k)
{
    std::map<std::string, webbridge::TagValue>::const_iterator it = t.find(k);
    return (it != t.end() && it->second.isString()) ? it->second.asString() : std::string("<not a string>");
}
static bool IsNull(const std::map<std::string, webbridge::TagValue>& t, const char* k)
{
    std::map<std::string, webbridge::TagValue>::const_iterator it = t.find(k);
    return it != t.end() && it->second.isNull();
}
static long long Int(const std::map<std::string, webbridge::TagValue>& t, const char* k)
{
    std::map<std::string, webbridge::TagValue>::const_iterator it = t.find(k);
    return (it != t.end() && it->second.isInt()) ? (long long)it->second.asInt() : -999999;
}
static int Bool(const std::map<std::string, webbridge::TagValue>& t, const char* k)   // 1 / 0 / -1 (not a bool)
{
    std::map<std::string, webbridge::TagValue>::const_iterator it = t.find(k);
    return (it != t.end() && it->second.isBool()) ? (it->second.asBool() ? 1 : 0) : -1;
}
static std::string Num(unsigned v) { char b[16]; std::snprintf(b, sizeof(b), "%u", v); return b; }

static std::string ReadAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::ostringstream s;
    s << f.rdbuf();
    return s.str();
}
static std::string LineWith(const std::string& text, const std::string& needle)
{
    const std::size_t at = text.find(needle);
    if (at == std::string::npos) return std::string();
    const std::size_t b = text.rfind('\n', at);
    const std::size_t e = text.find('\n', at);
    return text.substr(b == std::string::npos ? 0 : b + 1, (e == std::string::npos ? text.size() : e) - (b == std::string::npos ? 0 : b + 1));
}
static int Count(const std::string& text, const std::string& needle)
{
    int n = 0;
    for (std::size_t at = text.find(needle); at != std::string::npos; at = text.find(needle, at + needle.size())) ++n;
    return n;
}
static bool BeforeComment(const std::string& line, const std::string& code)   // the code sits before any // on its line
{
    const std::size_t c = line.find(code), s = line.find("//");
    return c != std::string::npos && (s == std::string::npos || c < s);
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("C14_BinDispPane\n");
    if (!W906TestInsideCtestRoots("C14_BinDispPane"))
        return 2;
    W906_BinDispTestReset_St02();

    std::printf(" 0. the ctest guard\n");
    CHECK(W906_BinDispUnderCtest_St02());

    // the machine shape of golden's error scan (cShowBinSelect.cpp:242-267): no magazine, no AMR, G16 off
    AUTO3_IS_MAGAZINE = 0; AUTO_EMPTY_COLOR = 1; TrayForm.bEnableAMR = false;
    IniConfig.bG16BinDispNeedAlarm = false;
    InitialOK = true;
    TfShowBinSelect* const f = fShowBinSelect;
    CHECK(f != NULL);
    if (f == NULL) return 1;

    std::printf(" 1. no controller (NUMBER_PANEL_TYPE 0)\n");
    {
        NUMBER_PANEL_TYPE = 0;
        HSys.BinDisCtrl = NULL;
        std::size_t n = 0;
        const std::map<std::string, webbridge::TagValue> t = Stage(&n);
        int fam = 0;
        for (std::map<std::string, webbridge::TagValue>::const_iterator it = t.begin(); it != t.end(); ++it)
            if (it->first.compare(0, 12, "binsel.disp.") == 0) ++fam;
        CHECK(n == 15 && W906_BinDispStatusTagCount_St02() == 15 && fam == 15 && t.size() == 15);
        CHECK(Int(t, "binsel.disp.panelType") == 0 && Bool(t, "binsel.disp.tabVisible") == 0 && Bool(t, "binsel.disp.ctrl") == 0);
        CHECK(IsNull(t, "binsel.disp.inst") && IsNull(t, "binsel.disp.err") && IsNull(t, "binsel.disp.colorNow") && IsNull(t, "binsel.disp.binNow"));
        CHECK(Split(Str(t, "binsel.disp.caption"), '\t').size() == 36 && Split(Str(t, "binsel.disp.color"), ',').size() == 36);
        CHECK(Split(Str(t, "binsel.disp.lbl"), '\t').size() == 33 && Split(Str(t, "binsel.disp.lblColor"), ',').size() == 33);
        CHECK(Str(t, "binsel.disp.groups").size() == 3 && Int(t, "binsel.disp.jumpSeq") == 0);
    }

    FakeBinDisp* const bd = new FakeBinDisp();
    if (bd->slBinDispLog) bd->slBinDispLog->Path = as9045LogPath + "\\BinDisplayLog";   // the boot's seam (banner (6)); nothing is logged here
    for (int i = 0; i < eBinDispTotal; ++i) bd->Unit(i, false, 1, 0, false);
    bd->Unit(eBinDispLoader, true, 3, 111, false);
    bd->Unit(eBinDispEmpty,  true, 3, 104, false);
    bd->Unit(eBinDispColor,  true, 3, 102, false);
    bd->Unit(eBinDispAuto1,  true, 2, 1,   false);
    bd->Unit(eBinDispAuto2,  true, 1, 5,   false);
    bd->Unit(eBinDispAuto3,  true, 2, 7,   true);                               // in error: the black / red flash
    for (int i = eBinDispFix1; i <= eBinDispFix6; ++i) bd->Unit(i, true, 3, 123, false);   // 123 = "X"
    bd->Unit(eBinDispFix6, false, 1, 0, false);                                  // i>=3 and not installed = an error (golden :257-258)
    HSys.BinDisCtrl = bd;
    NUMBER_PANEL_TYPE = 3;

    std::printf(" 2. type 3 with errors: golden's early return, then the scope\n");
    {
        f->pnlLoader->Caption = "?"; f->pnlLoader->Color = 12345;
        f->PageControl1->ActivePage = f->tsTestBin; f->PageControl1->ActivePageIndex = 0;
        f->ChangeBinDispStatus();                                               // no scope: ActivePage is not tsUnloadMap -> return (:283-291)
        CHECK(std::string(f->pnlLoader->Caption.c_str()) == "?" && f->pnlLoader->Color == 12345);
        CHECK(f->PageControl1->ActivePageIndex == 3);                           // golden :272 jumped before the early return (906 assigns)
        f->PageControl1->ActivePage = f->tsTestBin; f->PageControl1->ActivePageIndex = 0;

        const unsigned long j0 = W906_BinDispJumpSeq_St02();
        { W906_BinDispShownScope_St02 asShown; f->ChangeBinDispStatus(); }
        CHECK(W906_BinDispJumpSeq_St02() == j0 + 1);
        CHECK(f->PageControl1->ActivePageIndex == 3 && f->PageControl1->ActivePage == f->tsUnloadMap);   // VCL: index 3 = tsUnloadMap
        std::size_t n = 0;
        const std::map<std::string, webbridge::TagValue> t = Stage(&n);
        const std::vector<std::string> cap = Split(Str(t, "binsel.disp.caption"), '\t'), col = Split(Str(t, "binsel.disp.color"), ',');
        CHECK(cap.size() == 36 && col.size() == 36);
        if (cap.size() == 36 && col.size() == 36)
        {
            CHECK(cap[eBinDispLoader] == "L" && col[eBinDispLoader] == Num(kClOrange));   // golden :303-307, ColorMap[3]
            CHECK(cap[eBinDispEmpty] == "E" && cap[eBinDispColor] == "C");
            CHECK(cap[eBinDispAuto1] == "1" && col[eBinDispAuto1] == Num(kClGreen));      // :320-323, ColorMap[2]
            CHECK(cap[eBinDispAuto2] == "5" && col[eBinDispAuto2] == Num(kClRed));        // ColorMap[1]
            CHECK(cap[eBinDispFix1] == "X");                                              // 123 -> "X" (:316-319)
            CHECK(cap[eBinDispFix6] == "X" && col[eBinDispFix6] == Num(kClGray));        // not installed (:333-337)
            CHECK(col[eBinDispAuto3] == Num(kClBlack) || col[eBinDispAuto3] == Num(kClRed));   // the flash (:325-331)
        }
        CHECK(Str(t, "binsel.disp.status") == "Bin display got error!!" && Int(t, "binsel.disp.statusColor") == (long long)kClRed);
        CHECK(Bool(t, "binsel.disp.tabVisible") == 1 && Bool(t, "binsel.disp.ctrl") == 1 && Int(t, "binsel.disp.panelType") == 3);
        const std::string inst = Str(t, "binsel.disp.inst"), err = Str(t, "binsel.disp.err");
        CHECK(inst.size() == 36 && inst[eBinDispLoader] == '1' && inst[eBinDispFix6] == '0' && inst[eBinDispMag1] == '0');
        CHECK(err.size() == 36 && err[eBinDispAuto3] == '1' && err[eBinDispAuto2] == '0');
        const std::vector<std::string> cn = Split(Str(t, "binsel.disp.colorNow"), ','), bn = Split(Str(t, "binsel.disp.binNow"), ',');
        CHECK(cn.size() == 36 && bn.size() == 36 && cn[eBinDispAuto1] == "2" && bn[eBinDispLoader] == "111");
        CHECK(Int(t, "binsel.disp.jumpSeq") == (long long)(j0 + 1));

        const std::string flash1 = col.size() == 36 ? col[eBinDispAuto3] : std::string();
        { W906_BinDispShownScope_St02 asShown; f->ChangeBinDispStatus(); }   // the next second
        const std::map<std::string, webbridge::TagValue> t2 = Stage(&n);
        const std::vector<std::string> col2 = Split(Str(t2, "binsel.disp.color"), ',');
        CHECK(col2.size() == 36 && col2[eBinDispAuto3] != flash1 &&
              (col2[eBinDispAuto3] == Num(kClBlack) || col2[eBinDispAuto3] == Num(kClRed)));   // black <-> red, once per call
        CHECK(Int(t2, "binsel.disp.jumpSeq") == (long long)(j0 + 2));          // 906 jumps on every call that sees an error
    }

    std::printf(" 3. no error: no jump, the old page comes back\n");
    {
        bd->Unit(eBinDispAuto3, true, 2, 7, false);
        bd->Unit(eBinDispFix6,  true, 3, 123, false);
        f->PageControl1->ActivePage = f->tsTestBin; f->PageControl1->ActivePageIndex = 0;
        const unsigned long j0 = W906_BinDispJumpSeq_St02();
        { W906_BinDispShownScope_St02 asShown; f->ChangeBinDispStatus(); }
        CHECK(W906_BinDispJumpSeq_St02() == j0);
        CHECK(f->PageControl1->ActivePage == f->tsTestBin && f->PageControl1->ActivePageIndex == 0);
        std::size_t n = 0;
        const std::map<std::string, webbridge::TagValue> t = Stage(&n);
        CHECK(Str(t, "binsel.disp.status") == std::string(bd->GetRunStatus().c_str()) && Int(t, "binsel.disp.statusColor") == (long long)kClBtnFace);
        const std::vector<std::string> cap = Split(Str(t, "binsel.disp.caption"), '\t');
        CHECK(cap.size() == 36 && cap[eBinDispAuto3] == "7");
    }

    std::printf(" 4. labels, group panels, a TAB inside a caption\n");
    {
        f->UnLoadLabel[eAuto2]->Caption = "1 2";
        f->UnLoadLabel[eAuto2]->Font->Color = (int)kClGreen;
        f->UnLoadLabel[eFix1]->Caption = "x\ty";
        f->pnlMag123->Visible = true; f->pnlFix789->Visible = false; f->pnlAuto456->Visible = true;
        std::size_t n = 0;
        const std::map<std::string, webbridge::TagValue> t = Stage(&n);
        const std::vector<std::string> lbl = Split(Str(t, "binsel.disp.lbl"), '\t'), lc = Split(Str(t, "binsel.disp.lblColor"), ',');
        CHECK(lbl.size() == 33 && lc.size() == 33);
        if (lbl.size() == 33 && lc.size() == 33)
            CHECK(lbl[eAuto2] == "1 2" && lc[eAuto2] == Num(kClGreen) && lbl[eFix1] == "x y");
        CHECK(Str(t, "binsel.disp.groups") == "101");
    }

    std::printf(" 5. source pins\n");
    if (argc > 1)
    {
        const std::string root = argv[1];
        const std::string ws = ReadAll(root + "/tools/wb_serve.cpp"), cm = ReadAll(root + "/CMakeLists.txt");
        const std::string mt3 = ReadAll(root + "/MainTimer3.cpp"), page = ReadAll(root + "/../web/page/Status.ShowBinSelect.html");
        CHECK(!ws.empty() && !cm.empty() && !mt3.empty() && !page.empty());
        const std::string decl = "std::size_t W906_StageBinDispStatus_St02(webbridge::TagSnapshot& snap);";
        const std::string call = "{ if (::W906_PageStreamWanted(\"binselect\")) n += ::W906_StageBinDispStatus_St02(snap); }";
        CHECK(Count(ws, decl) == 1 && BeforeComment(LineWith(ws, decl), decl) && LineWith(ws, decl).compare(0, 4, "} } ") == 0);
        CHECK(Count(ws, call) == 1 && BeforeComment(LineWith(ws, call), call) && LineWith(ws, call).find("return n;") != std::string::npos);
        CHECK(ws.find(decl) < ws.find("std::size_t PublishExtraTags(webbridge::TagSnapshot& snap)") &&
              ws.find("std::size_t PublishExtraTags(webbridge::TagSnapshot& snap)") < ws.find(call));
        const std::string cl = LineWith(cm, "tools/wb_serve.cpp  SimNet/SimNetMask.cpp");
        CHECK(BeforeComment(cl, "WebBinDispStatus_St02.cpp") && cl.find("WebBinDispStatus_St02.cpp") < cl.find('#'));
        const std::string scope = "W906_BinDispShownScope_St02 asShown;  fShowBinSelect->ChangeBinDispStatus();";
        CHECK(Count(mt3, scope) == 1 && BeforeComment(LineWith(mt3, scope), scope) && Count(mt3, "fShowBinSelect->ChangeBinDispStatus();") == 1);
        CHECK(Count(page, "<script src=\"ht9045_bindisp_status.js\"></script>") == 1 && Count(page, "id=\"bdMap\"") == 1);   // AI(W906-ST02-2C) 20261006: no 147-line pin (Frank FR-PR1 2C low)
    }
    else
        std::printf("  (skipped: no tree root given)\n");

    HSys.BinDisCtrl = NULL;                                                     // the stand-in is not deleted: nothing may log at exit
    std::printf("C14_BinDispPane: %d checks, %d failed\n", g_total, g_fail);
    return g_fail == 0 ? 0 : 1;
}
