// =============================================================================
//  test_e022_contactct.cpp -- todo E-022 CK-1 / CK-2 / CK-3: Data.ContactCT's Count Clear, grid double-click and Yield Chart
//
//  //AI(W906-E022-CK1) 20261002 [W906] St01 new file.  todo D:\HT9045\.claude\skills\ht9050-construction\references\todo.md E-022;
//    St02 inventory D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.3; Jimmy RULINGS_20261001 #0.
//  golden (V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContactCT.cpp): btClearCountClick :944-1069,
//    sgYieldDblClick :740-886, btYieldChartClick :1071-1075.
//  Under test: the tail of cContactCT.cpp (ht9045::sjson::W906_ContactCTAct, the same object wb_serve links), reached through the
//    REAL act.* dispatch JsonBridge/ChanAction.cpp HandleActionWithTag (its :349 same-line dispatch) -- the function wb_serve.cpp:4833
//    calls; "ok" is computed the way wb_serve does it (the reply contains "executed":true).
//    [0] not open (golden FormShow not run) -> not-open; the open = W906_ContactCTJson(-1) (= contactct.get, golden FormShow)
//    [1] CK-1 guards: SystemStart -> running (golden :947); level 107 too low -> not-authorized + golden's WAR1676 (:985);
//        KYEC_LEE -> customer-gated (:955-983, fail-closed); ASE Kaohsiung with bRefreshFunction -> no level check (:952, golden);
//        nothing changes on any refusal
//    [2] CK-1 first press -> needConfirm with golden's two strings (:1001); nothing changes
//    [3] CK-1 NO -> executed, cleared:false, the contact counts and bShowSiteYield stay, but golden's tail :1050-1068 ran
//        (iIndexCount, iLowYieldCloseCount, bStandardYield, iStandardYield, iAutoTempOfsTriggerCnt, fYieldMonitoring interval counts)
//    [4] CK-1 YES -> the three arms cleared (:1018-1019), bShowSiteYield all false (:1022-1023), tail; VTEST -> only arms 0 / 1 (:1011-1014)
//    [5] CK-2 site clear: QualSite2X2 (i=(row-1)/2, j=(row-1)%2), NN_1Row QualSite2X2N col 3, row 0 -> site (0,0) (golden oddity),
//        History item (0) -> nothing, stale view / bad cell / running refused, NO -> nothing
//    [6] CK-3: fObserver->iShowYieldChart=1 (:1073), executed, open:"observer"
//    [7] bad payload, unknown act.contactCT.* name
//    [8] wiring ratchet (comments and '\r' stripped): ChanAction.cpp's dispatch, the tail's three bodies, the page files load
//        ht9045_contactct_ev.js and send the three act names (argv[1] = port tree, argv[2] = D:\HT9045\web\page, read-only)
//  Files: refuses to run outside ctest's sandbox (w906_ctest_guard.h).  Compares D:\HT9045\system's file list (name, size, mtime) and
//    D:\HT9045\config\config.ini before / after: the clear must not write any real machine file (golden saves no file here).
//  NOT COVERED: the browser's YES / NO box and the Observer window (E022_ContactCTPage, node); WAR1676's alarm frame (wb_serve hook).
// =============================================================================
#include "JsonBridge/ChanAction.h"
#include "forms/fContactCT.h"
#include "forms/fObserver.h"
#include "forms/fYieldMonitoring.h"
#include "forms/fSecurity.h"
#include "cinitial.h"           // IsNNMode
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "LastSet.h"
#include "Config.h"
#include "CosFunction.h"
#include "cSocket.h"
#include "w906_ctest_guard.h"

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

extern int iMaxLevelItem;   // cSecurity.cpp:47 (W906_SecurityBoot sets 180 in wb_serve)
extern AnsiString W906_ShowErrorMessage_LastCode;   // canary_support.h:118 (its header cannot share a TU with cMyDB.h, fNote_ShowError.cpp:602)
extern int W906_ShowErrorMessage_Count;            // canary_support.h:120

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

bool Has(const std::string& h, const char* n) { return h.find(n) != std::string::npos; }

// what wb_serve.cpp:4833 does with the reply
bool AckOk(const std::string& r) { return Has(r, "\"executed\":true"); }

std::string Act(const char* cmd, const std::string& value)
{
    const std::string r = ht9045::sjson::HandleActionWithTag(cmd, value, std::string());
    std::printf("    %s %s -> %s\n", cmd, value.c_str(), r.substr(0, 300).c_str());
    return r;
}

bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}

void ListDir(const std::string& root, std::map<std::string, std::string>* out)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((root + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        char buf[96];
        std::snprintf(buf, sizeof(buf), "%lu/%lu/%lu/%lu", (unsigned long)fd.nFileSizeHigh, (unsigned long)fd.nFileSizeLow,
                      (unsigned long)fd.ftLastWriteTime.dwHighDateTime, (unsigned long)fd.ftLastWriteTime.dwLowDateTime);
        (*out)[name] = buf;
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

// source without // and /* */ comments, string literals kept, '\r' dropped (#if 0 blocks are not in these tails)
std::string StripComments(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (c == '\r') continue;
        if (c == '"' || c == '\'') {
            const char q = c;
            o += c;
            for (++i; i < s.size(); ++i) {
                o += s[i];
                if (s[i] == '\\' && i + 1 < s.size()) { o += s[++i]; continue; }
                if (s[i] == q || s[i] == '\n') break;
            }
            continue;
        }
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') { while (i < s.size() && s[i] != '\n') ++i; o += '\n'; continue; }
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '*') { i += 2; while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) ++i; ++i; continue; }
        o += c;
    }
    return o;
}
std::string StripHtmlComments(const std::string& s)
{
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s.compare(i, 4, "<!--") == 0) { const std::size_t e = s.find("-->", i + 4); if (e == std::string::npos) break; i = e + 2; continue; }
        if (s[i] != '\r') o += s[i];
    }
    return o;
}

void SeedCounts(unsigned v)
{
    for (int k = 0; k < 3; ++k)
        for (int r = 0; r < MAX_SOCKET_ROW; ++r)
            for (int c = 0; c < MAX_SOCKET_COL; ++c) {
                ArmData[k]->ArmSKET[r][c]->ClearALLCT();
                ArmData[k]->SetPassCT(r, c, v);
                ArmData[k]->SetFailCT(r, c, 1);
            }
}
unsigned long SiteTotal(int k, int r, int c) { return ArmData[k]->ArmSKET[r][c]->GetTotal(); }

void SeedTail()
{
    LastSet.iIndexCount = 77;
    LastSet.iAutoTempOfsTriggerCnt = 5;
    iLowYieldCloseCount = 3;
    bStandardYield = true;
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 8; ++j) iStandardYield[i][j] = 9;
    fYieldMonitoring->iFailAlarmSiteMaxYieldIntervalCount = 4;
    fYieldMonitoring->iFailAlarmSiteYieldIntervalCount = 4;
    fYieldMonitoring->iAutoClean_FailAlarmSiteYieldIntervalCount = 4;
    for (int i = 0; i < 32; ++i) fYieldMonitoring->bShowSiteYield[i] = true;
}
bool TailRan()
{
    bool z = LastSet.iIndexCount == 0 && LastSet.iAutoTempOfsTriggerCnt == 0 && iLowYieldCloseCount == 0 && !bStandardYield &&
             fYieldMonitoring->iFailAlarmSiteMaxYieldIntervalCount == 0 && fYieldMonitoring->iFailAlarmSiteYieldIntervalCount == 0 &&
             fYieldMonitoring->iAutoClean_FailAlarmSiteYieldIntervalCount == 0;
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 8; ++j) z = z && iStandardYield[i][j] == 0;
    return z;
}
bool TailUntouched() { return LastSet.iIndexCount == 77 && LastSet.iAutoTempOfsTriggerCnt == 5 && iLowYieldCloseCount == 3 && bStandardYield; }
bool ShowSiteAll(bool v) { for (int i = 0; i < 32; ++i) if (fYieldMonitoring->bShowSiteYield[i] != v) return false; return true; }

void LevelOk(bool ok)
{
    iMaxLevelItem = 180;
    LevelSet.AccessLevel[107] = 2;
    AccessLevel = ok ? 3 : 1;
}

void SetMode(int mode)
{
    TestIF.iTestMode = mode;
    TestIF_File.iTestMode = mode;   // IsNNMode() reads TestIF_File (cinitial.cpp:7419), sgYieldDblClick reads TestIF (golden as is)
    W906_ContactCTJson(-1);         // contactct.get = golden FormShow (ShowFormComp sets RowCount for the mode)
}

}  // namespace

int main(int argc, char** argv)
{
    extern AnsiString as9045LogPath, asSaveEventLogPath, asGeneralPath, AuthPath;
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asGeneralPath", asGeneralPath.c_str(), "AuthPath", AuthPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("E022_ContactCT", rt))
        return 2;
    const std::string root = argc > 1 ? argv[1] : "";
    const std::string web = argc > 2 ? argv[2] : "";

    std::map<std::string, std::string> sys0, sys1;
    std::string cfg0, cfg1;
    ListDir("D:\\HT9045\\system", &sys0);
    ReadAll("D:\\HT9045\\config\\config.ini", &cfg0);

    CUSTOMER_CODE = CC_HONPREC_QC;
    SystemStart = false;
    IniConfig.bVTESTFunction = false;
    IniConfig.bI29EnableYieldRecord = false;     // MyDBIProductionData -> SaveSiteYield writes nothing
    CosFunction.bLowYieldUseContactCounts = false;
    CosFunction.bSmartAutoClean = false;
    LevelOk(true);

    std::printf("[0] not open\n");
    {
        fContactCT->bShow = false;
        const std::string r = Act("act.contactCT.clearCount", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"not-open\""), "before golden FormShow -> not-open");
        const std::string r2 = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":1,\"yieldType\":3}");
        Check(Has(r2, "\"guard\":\"not-open\""), "dblClick before FormShow -> not-open");
        const std::string r3 = Act("act.contactCT.yieldChart", "{}");
        Check(Has(r3, "\"guard\":\"not-open\""), "yieldChart before FormShow -> not-open");
        SetMode(QualSite2X2);
        Check(fContactCT->bShow && fContactCT->rgYieldType->ItemIndex == 3, "W906_ContactCTJson(-1) = golden FormShow: bShow, ItemIndex 3");
    }

    std::printf("[1] CK-1 guards\n");
    {
        SeedCounts(5); SeedTail();
        SystemStart = true;
        std::string r = Act("act.contactCT.clearCount", "{\"answer\":\"yes\"}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"running\"") && Has(r, "cContactCT.cpp:947"), "SystemStart -> running (golden :947)");
        SystemStart = false;
        Check(SiteTotal(0, 0, 0) == 6 && TailUntouched(), "running: nothing changed");

        LevelOk(false);
        const int n0 = W906_ShowErrorMessage_Count;
        r = Act("act.contactCT.clearCount", "{\"answer\":\"yes\"}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"not-authorized\"") && Has(r, "cContactCT.cpp:985"), "level 107 too low -> not-authorized (golden :985)");
        Check(W906_ShowErrorMessage_Count == n0 + 1 && W906_ShowErrorMessage_LastCode == "WAR1676", "golden Insufficient(107) shows WAR1676");
        Check(SiteTotal(1, 1, 1) == 6 && TailUntouched(), "not-authorized: nothing changed");
        r = Act("act.contactCT.clearCount", "{}");
        Check(Has(r, "\"guard\":\"not-authorized\""), "level checked before the YES/NO box too");

        CUSTOMER_CODE = CC_KYEC_LEE;
        LevelOk(true);
        r = Act("act.contactCT.clearCount", "{\"answer\":\"yes\"}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"customer-gated\""), "KYEC_LEE -> customer-gated (golden :955-983 not in the facade)");
        Check(SiteTotal(2, 0, 0) == 6 && TailUntouched(), "customer-gated: nothing changed");

        CUSTOMER_CODE = CC_ASE_KaohSiung;
        LevelOk(false);
        bRefreshFunction = true;
        r = Act("act.contactCT.clearCount", "{}");
        Check(Has(r, "\"needConfirm\":true"), "ASE Kaohsiung + bRefreshFunction: golden :952 skips the level check");
        bRefreshFunction = false;
        r = Act("act.contactCT.clearCount", "{}");
        Check(Has(r, "\"guard\":\"not-authorized\"") && Has(r, "cContactCT.cpp:952"), "ASE Kaohsiung, no bRefreshFunction, level low -> not-authorized (:952)");
        CUSTOMER_CODE = CC_HONPREC_QC;
        LevelOk(true);
    }

    std::printf("[2] CK-1 first press\n");
    {
        SeedCounts(5); SeedTail();
        const std::string r = Act("act.contactCT.clearCount", "{}");
        Check(!AckOk(r) && Has(r, "\"needConfirm\":true") && Has(r, "Do you want to clear the data?") && Has(r, "Warning!!") &&
              Has(r, "cContactCT.cpp:1001"), "no answer -> needConfirm with golden's text and caption (:1001)");
        Check(SiteTotal(0, 0, 0) == 6 && TailUntouched() && ShowSiteAll(true), "needConfirm: nothing changed");
        const std::string r2 = Act("act.contactCT.clearCount", "{\"answer\":null}");
        Check(Has(r2, "\"needConfirm\":true"), "answer null = not answered yet");
    }

    std::printf("[3] CK-1 NO\n");
    {
        SeedCounts(5); SeedTail();
        const std::string r = Act("act.contactCT.clearCount", "{\"answer\":\"no\"}");
        Check(AckOk(r) && Has(r, "\"cleared\":false") && Has(r, "\"answer\":\"no\""), "NO -> executed, cleared:false");
        Check(SiteTotal(0, 0, 0) == 6 && SiteTotal(2, 3, 7) == 6 && ShowSiteAll(true), "NO: contact counts and bShowSiteYield stay");
        Check(TailRan(), "NO: golden's tail :1050-1068 still ran (iIndexCount, iLowYieldCloseCount, bStandardYield, iStandardYield, "
                         "iAutoTempOfsTriggerCnt, yield-monitor interval counts)");
    }

    std::printf("[4] CK-1 YES\n");
    {
        SeedCounts(5); SeedTail();
        std::string r = Act("act.contactCT.clearCount", "{\"answer\":\"yes\"}");
        Check(AckOk(r) && Has(r, "\"cleared\":true") && Has(r, "\"totalsAfter\":[0,0,0]"), "YES -> executed, cleared, totals 0");
        Check(ArmData[0]->GetTotalCT() == 0 && ArmData[1]->GetTotalCT() == 0 && ArmData[2]->GetTotalCT() == 0, "YES: three arms cleared (:1018-1019)");
        Check(ShowSiteAll(false) && TailRan(), "YES: bShowSiteYield all false (:1022-1023) + tail");

        SeedCounts(5); SeedTail();
        IniConfig.bVTESTFunction = true;
        r = Act("act.contactCT.clearCount", "{\"answer\":\"yes\"}");
        IniConfig.bVTESTFunction = false;
        Check(AckOk(r) && ArmData[0]->GetTotalCT() == 0 && ArmData[1]->GetTotalCT() == 0 && ArmData[2]->GetTotalCT() != 0,
              "VTEST: only arms 0 / 1 cleared (golden :1011-1014)");
    }

    std::printf("[5] CK-2 grid double-click\n");
    {
        SetMode(QualSite2X2);
        SeedCounts(5);
        const int rows = fContactCT->sgYield->RowCount;
        std::printf("    QualSite2X2 RowCount=%d ColCount=%d nn=%d\n", rows, (int)fContactCT->sgYield->ColCount, IsNNMode());
        std::string r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":3,\"yieldType\":3}");
        Check(!AckOk(r) && Has(r, "\"needConfirm\":true") && Has(r, "cContactCT.cpp:853"), "Kind(%) item: needConfirm (golden :853)");
        Check(SiteTotal(0, 1, 0) == 6, "needConfirm: nothing changed");
        r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":3,\"yieldType\":3,\"answer\":\"yes\"}");
        Check(AckOk(r) && Has(r, "\"site\":[1,0]"), "row 3 -> site (1,0) = ((3-1)/2, (3-1)%2) (golden :752-753)");
        Check(SiteTotal(0, 1, 0) == 0 && SiteTotal(1, 1, 0) == 0 && SiteTotal(2, 1, 0) == 0 && SiteTotal(0, 0, 0) == 6 && SiteTotal(0, 1, 1) == 6,
              "YES: site (1,0) cleared on all three arms (:875-877), the others stay");

        SeedCounts(5);
        r = Act("act.contactCT.dblClick", "{\"col\":2,\"row\":0,\"yieldType\":3,\"answer\":\"yes\"}");
        Check(AckOk(r) && Has(r, "\"site\":[0,0]") && SiteTotal(0, 0, 0) == 0, "fixed row 0 -> site (0,0) (golden oddity: (0-1)%2=-1 clamped, :836-837)");

        SeedCounts(5);
        r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":3,\"yieldType\":3,\"answer\":\"no\"}");
        Check(AckOk(r) && Has(r, "\"cleared\":false") && SiteTotal(0, 1, 0) == 6, "NO: nothing cleared");

        r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":3,\"yieldType\":0}");
        Check(Has(r, "\"guard\":\"stale-view\""), "page shows item 0, form on item 3 -> stale-view");
        r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":99,\"yieldType\":3}");
        Check(Has(r, "\"guard\":\"bad-cell\""), "row outside sgYield -> bad-cell");
        SystemStart = true;
        r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":3,\"yieldType\":3,\"answer\":\"yes\"}");
        SystemStart = false;
        Check(Has(r, "\"guard\":\"running\"") && SiteTotal(0, 1, 0) == 6, "SystemStart -> running (golden :743), nothing cleared");

        W906_ContactCTJson(0);   // golden rgYieldType click item 0 (History)
        Check(fContactCT->rgYieldType->ItemIndex == 0, "rgYieldType on History");
        r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":3,\"yieldType\":0,\"answer\":\"yes\"}");
        Check(AckOk(r) && Has(r, "\"asked\":false") && SiteTotal(0, 1, 0) == 6, "History item: golden bClear=false -> nothing (:846-851)");
        CUSTOMER_CODE = CC_HANA_MICRON;
        r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":3,\"yieldType\":0}");
        Check(Has(r, "\"needConfirm\":true"), "HANA_MICRON: History item clears too (golden :846-847)");
        CUSTOMER_CODE = CC_HONPREC_QC;

        SetMode(QualSite2X2N);
        SeedCounts(5);
        std::printf("    QualSite2X2N RowCount=%d nn=%d\n", (int)fContactCT->sgYield->RowCount, IsNNMode());
        r = Act("act.contactCT.dblClick", "{\"col\":3,\"row\":2,\"yieldType\":3,\"answer\":\"yes\"}");
        Check(AckOk(r) && Has(r, "\"site\":[1,1]") && Has(r, "\"armSites\":[[0,0,1],[2,1,1]]"),
              "NN_1Row col 3 row 2 -> ArmData[0](0,1) + ArmData[2](1,1) (golden :865-871)");
        Check(SiteTotal(0, 0, 1) == 0 && SiteTotal(2, 1, 1) == 0 && SiteTotal(1, 0, 1) == 6, "NN_1Row: arm 1 untouched");
        SetMode(QualSite2X2);
    }

    std::printf("[6] CK-3 Yield Chart\n");
    {
        fObserver->iShowYieldChart = 0;
        const std::string r = Act("act.contactCT.yieldChart", "{}");
        Check(AckOk(r) && fObserver->iShowYieldChart == 1 && Has(r, "\"open\":\"observer\""), "golden :1073 iShowYieldChart=1; open observer");
    }

    std::printf("[7] payload / names\n");
    {
        std::string r = Act("act.contactCT.clearCount", "{\"answer\":\"maybe\"}");
        Check(Has(r, "\"guard\":\"bad-payload\""), "answer other than yes/no -> bad-payload");
        r = Act("act.contactCT.clearCount", "not json");
        Check(Has(r, "\"guard\":\"bad-payload\""), "not JSON -> bad-payload");
        r = Act("act.contactCT.dblClick", "{\"col\":1.5,\"row\":1,\"yieldType\":3}");
        Check(Has(r, "\"guard\":\"bad-payload\""), "fractional col -> bad-payload");
        r = Act("act.contactCT.dblClick", "{\"col\":1,\"row\":1}");
        Check(Has(r, "\"guard\":\"bad-payload\""), "dblClick without yieldType -> bad-payload");
        r = Act("act.contactCT.nope", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"unknown-action\""), "unknown act.contactCT.* -> unknown-action");
        r = Act("act.main.noSuchThing", "{}");
        Check(Has(r, "unknown-action") && !Has(r, "act.contactCT"), "other act.* still reach ChanAction's own refusal");
    }

    std::printf("[8] wiring ratchet\n");
    if (root.empty() || web.empty()) {
        Check(false, "argv[1] (port tree) and argv[2] (web\\page) are required");
    } else {
        std::string s;
        Check(ReadAll(root + "/JsonBridge/ChanAction.cpp", &s), "read JsonBridge/ChanAction.cpp");
        s = StripComments(s);
        Check(Has(s, "if (cmd.compare(0, 14, \"act.contactCT.\") == 0) { std::string W906_ContactCTAct(const std::string&, const std::string&); return W906_ContactCTAct(cmd, payloadJson); }"),
              "ChanAction.cpp: the act.contactCT.* dispatch is live code");
        Check(ReadAll(root + "/cContactCT.cpp", &s), "read cContactCT.cpp");
        s = StripComments(s);
        Check(Has(s, "std::string W906_ContactCTAct(const std::string &cmd, const std::string &payloadJson)") &&
              Has(s, "return E022_ClearCount(q);") && Has(s, "return E022_DblClick(q);") && Has(s, "return E022_YieldChart();"),
              "cContactCT.cpp tail: the three bodies are live code");
        Check(Has(s, "f->btYieldChartClick(nullptr);"), "CK-3 calls the translated golden btYieldChartClick");
        Check(ReadAll(web + "/Data.ContactCT.html", &s), "read Data.ContactCT.html");
        s = StripHtmlComments(s);
        Check(Has(s, "<script src=\"ht9045_contactct_ev.js\"></script>") && Has(s, "<script src=\"ht9045_busy_util.js\"></script>"),
              "Data.ContactCT.html loads ht9045_contactct_ev.js and ht9045_busy_util.js");
        Check(ReadAll(web + "/ht9045_contactct_ev.js", &s), "read ht9045_contactct_ev.js");
        s = StripComments(s);
        Check(Has(s, "'act.contactCT.clearCount'") && Has(s, "'act.contactCT.dblClick'") && Has(s, "'act.contactCT.yieldChart'"),
              "ht9045_contactct_ev.js sends the three act names");
        Check(ReadAll(web + "/ht9045_contactct_wire.js", &s), "read ht9045_contactct_wire.js");
        s = StripComments(s);
        Check(Has(s, "if (!window.HT9045ContactCTEv) $('btClearCount').addEventListener('click', notWired);"),
              "ht9045_contactct_wire.js: 'not wired' only when the ev file is missing");
    }

    ListDir("D:\\HT9045\\system", &sys1);
    ReadAll("D:\\HT9045\\config\\config.ini", &cfg1);
    Check(sys0 == sys1, "D:\\HT9045\\system: no file written (names, sizes, mtimes unchanged)");
    Check(cfg0 == cfg1, "D:\\HT9045\\config\\config.ini unchanged");

    std::printf("%d/%d checks passed (E022_ContactCT)\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
