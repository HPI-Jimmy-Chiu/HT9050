// =============================================================================
//  test_e020_lotinfo_e020.cpp -- todo E-020 LI-6 / LI-11 / LI-13 (St01): LotInfo_E020.cpp
//
//  //AI(W906-E020-LI6) 20261002 [W906] (St01): new file. golden = V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:
//    Timer1Timer :5366-5453, RTCChangeFile :5455-5475, btChangeFileClick :10394-10418, palSecsGemMouseDown :10532-10606.
//  One executable, two ctest entries (argv[1] = mode, argv[2] = port root for the source ratchets, read only):
//    E020_LotInfoRTC   (rtc)   LI-6: Timer1 dfm default off; the VCL wrapper W906_TfLotInfo_Timer1Timer runs Timer1Timer only while
//                              Timer1 is Enabled; RTCChangeFile's guards (COM2->bCCDDummyRum, InitialOK, bDoROILearning, already armed)
//                              and arming (task 1, finish=false, D31 picks bNeedDelete); Timer1Timer's InitialOK return; the no-delete
//                              path (wrdInitial -> wrdEndTask) and the delete path (wrdSendComm -> wrdWaitReply until golden's 10 s timer
//                              runs out -> wrdEndTask); the end flags (bSendRealCCDSendStart, bRTCChangeFileFinish, Timer1 off); every
//                              COM2 statement logged as a skip with golden's string (@FILE<setup>+ / ASE KaohSiung sRtcFileName,
//                              @SITE2+ / @SITE10+ / @SITE5+, both fSetup->fShow arms); ratchets for the foreign lines cSetUp.cpp:1083 /
//                              :1093 (gate lifted) and uhome.cpp:1554-1557 (RTCChangeFile live, InitRealTimeCCDPara still gated),
//                              forms/fLotInfo.h:2333, CMakeLists.txt (LotInfo_E020.cpp in ht9045_sm on the cTestCategory.cpp line).
//    E020_LotInfoActs  (acts)  act.lotInfo.* through the REAL dispatch JsonBridge/ChanAction.cpp HandleActionWithTag (:348 same line):
//                              unknown op; changeFile visibility (tab-hidden / button-hidden as golden FormShow :571-572), branch 0
//                              executed, branches 1 / 2 / 3 gated with "GATE (W906-E020-LI11-n)", bBarcodeConnect never set;
//                              palSecsGemMouseDown bad payloads, tab-hidden (CC_GIGAS), panel-hidden (CC_MTI), golden's early returns
//                              (SystemStart, AccessLevel, CC_KYEC_LEE) fill nothing, L L R R L L fills Lot ID "0123456789" /
//                              Operator "12345" and writes AuthPath config.ini [Lot Info] in the SANDBOX, a wrong click resets the
//                              sequence, a filled Lot ID is not overwritten; ratchet on the ChanAction.cpp line.
//  Machine data: AuthPath (W906_AUTH_PATH) and the global containment variables must point into ctest's scratch, else exit 2 before
//    any Handler code runs; the real D:\HT9045\config\config.ini and D:\HT9045\system\Gerneral.ini are compared before / after.
//  Control: W906_E020_SRC_ROOT = a folder with the pre-change sources -> the ratchets must go red.
// =============================================================================
#include "JsonBridge/ChanAction.h"
#include "LotInfo_E020.h"
#include "forms/fLotInfo.h"
#include "forms/fSetup.h"
#include "forms/fMain.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "atester_shims.h"
#include "myTimer.h"
#include "Public/cJSON.h"
#include "common.h"            // ReadIniData (the sandbox config.ini read back), AuthPath
#include "w906_ctest_guard.h"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern AnsiString AuthPath;
extern AnsiString asGeneralPath;
extern TQPF_Timer WaitRtcDeleteDelay;   // LotInfo_E020.cpp (golden uLotInfo.cpp:5364, file scope)

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
bool Has(const std::string& h, const std::string& n) { return h.find(n) != std::string::npos; }

bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}
std::vector<std::string> Lines(const std::string& s)
{
    std::vector<std::string> v;
    std::string cur;
    for (char c : s) { if (c == '\n') { v.push_back(cur); cur.clear(); } else if (c != '\r') cur += c; }
    v.push_back(cur);
    return v;
}
// true = the line is compiled (not inside an #if 0 block). Handles #if 0 / #if 1 / #ifdef / #ifndef / #if <expr> / #else / #endif;
//   only a literal "#if 0" counts as dead, every other condition as live (enough for the lines checked here).
std::vector<bool> LiveMask(const std::vector<std::string>& L)
{
    std::vector<bool> live(L.size(), true);
    std::vector<int> st;   // 0 = dead (#if 0 branch), 1 = live, 2 = #if 0's #else (live)
    for (size_t i = 0; i < L.size(); ++i) {
        std::string t = L[i];
        size_t a = t.find_first_not_of(" \t");
        t = a == std::string::npos ? "" : t.substr(a);
        bool dead = false;
        for (int s : st) if (s == 0) dead = true;
        if (t.compare(0, 1, "#") == 0) {
            std::string d = t.substr(1);
            size_t b = d.find_first_not_of(" \t");
            d = b == std::string::npos ? "" : d.substr(b);
            if (d.compare(0, 2, "if") == 0) {
                const bool zero = d.compare(0, 4, "if 0") == 0 && (d.size() == 4 || d[4] == ' ' || d[4] == '\t' || d[4] == '/');
                st.push_back(zero ? 0 : 1);
            } else if (d.compare(0, 4, "else") == 0 && !st.empty()) {
                if (st.back() == 0) st.back() = 2;
            } else if (d.compare(0, 5, "endif") == 0 && !st.empty()) {
                st.pop_back();
            }
            live[i] = !dead;
            continue;
        }
        live[i] = !dead;
    }
    return live;
}
// index of the first line containing `needle` (after `from`), -1 if none
int Find(const std::vector<std::string>& L, const std::string& needle, int from = 0)
{
    for (size_t i = (size_t)(from < 0 ? 0 : from); i < L.size(); ++i) if (L[i].find(needle) != std::string::npos) return (int)i;
    return -1;
}
// code part of a line (before a // that is not inside a string literal)
std::string CodePart(const std::string& s)
{
    bool inStr = false;
    for (size_t i = 0; i + 1 < s.size(); ++i) {
        if (s[i] == '"' && (i == 0 || s[i - 1] != '\\')) inStr = !inStr;
        if (!inStr && s[i] == '/' && s[i + 1] == '/') return s.substr(0, i);
    }
    return s;
}

std::string SrcRoot(const char* argvRoot)
{
    const char* e = std::getenv("W906_E020_SRC_ROOT");
    if (e && *e) { std::printf("  (control run: W906_E020_SRC_ROOT=%s)\n", e); return e; }
    return argvRoot ? argvRoot : "";
}

// ---- LI-6 helpers ---------------------------------------------------------------------------------------------------------
void Tick(int n = 1) { for (int i = 0; i < n; ++i) W906_TfLotInfo_Timer1Timer(); }
int SkipsWith(const std::string& gate, const std::string& golden = "", const std::string& value = "\x01")
{
    int k = 0;
    for (const W906E020RtcSkip& s : W906_E020_RtcSkips())
        if (s.gate == gate && (golden.empty() || Has(s.golden, golden)) && (value == "\x01" || s.value == value)) ++k;
    return k;
}
void Arm(bool d31, bool needDelete, bool useDefault)
{
    IniConfig.bD31RTCChangeRecipeNeedreCreateModel = d31;
    if (useDefault) fLotInfo->RTCChangeFile(); else fLotInfo->RTCChangeFile(needDelete);
}

void RunRtc(const std::string& root)
{
    TfLotInfo* f = fLotInfo;
    const bool keepInit = InitialOK, keepDummy = COM2->bCCDDummyRum, keepRoi = bDoROILearning, keepSend = bSendRealCCDSendStart;
    const int keepCC = CUSTOMER_CODE, keepMode = TestIF_File.iTestMode;
    const bool keepTwo = bUseTwoArm32Site, keepShow = fSetup->fShow, keepD31 = IniConfig.bD31RTCChangeRecipeNeedreCreateModel;
    const AnsiString keepSetup = fMain->cbSetupFileName->Text, keepRtcName = TestIF_File.sRtcFileName;

    std::printf("[1] Timer1 (dfm :14621-14626 Enabled=False) and the VCL wrapper\n");
    {
        Check(f->Timer1 != 0 && f->Timer1->Enabled == false, "Timer1 exists and starts disabled (dfm Enabled=False)");
        f->iWaitRtcDeleteTask = 7;
        Check(!W906_TfLotInfo_Timer1Timer() && f->iWaitRtcDeleteTask == 7, "disabled Timer1: the wrapper does not run Timer1Timer");
        f->iWaitRtcDeleteTask = 0;
    }

    std::printf("[2] RTCChangeFile guards (golden :5457-5459)\n");
    {
        InitialOK = true; bDoROILearning = false; f->Timer1->Enabled = false; f->bRTCChangeFileFinish = true; f->iWaitRtcDeleteTask = 0;
        COM2->bCCDDummyRum = true;
        Arm(true, true, false);
        Check(!f->Timer1->Enabled && f->bRTCChangeFileFinish && f->iWaitRtcDeleteTask == 0, "COM2->bCCDDummyRum (offline default) -> nothing armed");
        COM2->bCCDDummyRum = false; InitialOK = false;
        Arm(true, true, false);
        Check(!f->Timer1->Enabled && f->bRTCChangeFileFinish, "InitialOK=false -> nothing armed");
        InitialOK = true; bDoROILearning = true;
        Arm(true, true, false);
        Check(!f->Timer1->Enabled && f->bRTCChangeFileFinish, "bDoROILearning=true -> nothing armed");
        bDoROILearning = false;
        f->Timer1->Enabled = true; f->iWaitRtcDeleteTask = 3;
        Arm(true, true, false);
        Check(f->iWaitRtcDeleteTask == 3 && f->bRTCChangeFileFinish, "Timer1 already enabled -> not re-armed (task and finish untouched)");
        f->Timer1->Enabled = false;
    }

    std::printf("[3] RTCChangeFile arms (golden :5461-5472)\n");
    {
        f->bNeedToDeleteFile = false; f->bRTCChangeFileFinish = true;
        Arm(true, true, true);
        Check(f->Timer1->Enabled && f->iWaitRtcDeleteTask == 1 && !f->bRTCChangeFileFinish, "armed: Timer1 on, task wrdInitial (1), finish=false");
        Check(f->bNeedToDeleteFile, "default argument bNeedDelete=true with [D31] on -> bNeedToDeleteFile=true");
        f->Timer1->Enabled = false;
        Arm(true, false, false);
        Check(!f->bNeedToDeleteFile, "[D31] on, RTCChangeFile(false) -> bNeedToDeleteFile=false");
        f->Timer1->Enabled = false;
        Arm(false, true, false);
        Check(!f->bNeedToDeleteFile && f->Timer1->Enabled, "[D31] off -> bNeedToDeleteFile=false whatever is passed");
        f->Timer1->Enabled = false;
    }

    std::printf("[4] Timer1Timer returns while InitialOK=false (golden :5368-5369)\n");
    {
        Arm(true, true, false);
        InitialOK = false;
        W906_E020_RtcSkipsClear();
        Tick();
        Check(f->iWaitRtcDeleteTask == 1 && W906_E020_RtcSkips().empty(), "InitialOK=false: task stays wrdInitial, nothing logged");
        InitialOK = true;
        f->Timer1->Enabled = false;
    }

    std::printf("[5] no-delete path: wrdInitial -> wrdEndTask (golden :5376-5427, :5444-5451)\n");
    {
        CUSTOMER_CODE = 0; fMain->cbSetupFileName->Text = "RECIPE_E020"; TestIF_File.iTestMode = DualSite; fSetup->fShow = false;
        bSendRealCCDSendStart = false;
        Arm(false, true, false);                                                // [D31] off -> no delete
        W906_E020_RtcSkipsClear();
        const long t0 = W906_E020_RtcSkipTotal();
        Tick();
        Check(f->iWaitRtcDeleteTask == 4, "after wrdInitial: wrdEndTask (4) because bNeedToDeleteFile=false (:5425-5426)");
        Check(SkipsWith("W906-E020-LI6-1", "rtFileOK", "@FILERECIPE_E020+") == 1, "LI6-1 logged with golden's @FILE<cbSetupFileName>+ (:5380)");
        Check(SkipsWith("W906-E020-LI6-2", "rtSite", "@SITE2+") == 1, "LI6-2 logged with @SITE2+ (DualSite, :5409)");
        Check(SkipsWith("W906-E020-LI6-3", "rtFileOK, true") == 1 && SkipsWith("W906-E020-LI6-3", "rtSite, true") == 1, "LI6-3 both SendCommToVision logged (:5421-5422)");
        Check(f->Timer1->Enabled && !f->bRTCChangeFileFinish && !bSendRealCCDSendStart, "not finished yet after one tick");
        Tick();
        Check(SkipsWith("W906-E020-LI6-6", "rtMODEL") == 1 && SkipsWith("W906-E020-LI6-6", "rtInspEnd") == 1, "LI6-6 MODEL / InspEnd sends logged (:5446-5447)");
        Check(bSendRealCCDSendStart && f->bRTCChangeFileFinish && !f->Timer1->Enabled, "wrdEndTask: bSendRealCCDSendStart=true, bRTCChangeFileFinish=true, Timer1 off (:5448-5450)");
        Check(W906_E020_RtcSkipTotal() - t0 == 6, "six skips in all for the no-delete path, got " + std::to_string(W906_E020_RtcSkipTotal() - t0));
        Check(!W906_TfLotInfo_Timer1Timer(), "Timer1 off -> the wrapper stops running it");
    }

    std::printf("[6] delete path: wrdSendComm -> wrdWaitReply (10 s, no reply) -> wrdEndTask (golden :5428-5443)\n");
    {
        bSendRealCCDSendStart = false;
        Arm(true, true, false);
        W906_E020_RtcSkipsClear();
        Tick();
        Check(f->iWaitRtcDeleteTask == 2, "after wrdInitial: wrdSendComm (2) because bNeedToDeleteFile=true (:5423-5424)");
        Tick();
        Check(f->iWaitRtcDeleteTask == 3, "after wrdSendComm: wrdWaitReply (3)");
        Check(SkipsWith("W906-E020-LI6-4", "rtDELETE") == 1 && SkipsWith("W906-E020-LI6-5", "bRealTimeCom_ReceiveOK") == 1, "LI6-4 DELETE send and LI6-5 reply read logged once");
        Tick(3);
        Check(f->iWaitRtcDeleteTask == 3 && !f->bRTCChangeFileFinish, "inside golden's 10 s wait: still wrdWaitReply (no reply can come)");
        Check(SkipsWith("W906-E020-LI6-5") == 1, "the reply gate is not logged on every tick");
        WaitRtcDeleteDelay.SetMSAndOn(1);                                       // the 10 s of golden :5430, shortened for the test
        ::Sleep(30);
        Tick();
        Check(f->iWaitRtcDeleteTask == 4, "timer ran out -> wrdEndTask (:5434-5437)");
        Tick();
        Check(bSendRealCCDSendStart && f->bRTCChangeFileFinish && !f->Timer1->Enabled, "finished as golden with a dead RTC");
    }

    std::printf("[7] @FILE / @SITE strings (golden :5377-5419)\n");
    {
        struct S { int cust; int mode; bool two; bool show; const char* file; const char* site; };
        const S cases[] = {
            { CC_ASE_KaohSiung, SingleSite, false, false, "@FILERTCNAME_ASE+", "@SITE2+" },
            { 0, QualSite1X4, false, true,  "@FILERECIPE_E020+", "@SITE2+" },
            { 0, _8Site1X4,   false, false, "@FILERECIPE_E020+", "@SITE2+" },
            { 0, QualSite2X2, true,  true,  "@FILERECIPE_E020+", "@SITE10+" },
            { 0, QualSite2X2, true,  false, "@FILERECIPE_E020+", "@SITE10+" },
            { 0, QualSite2X2, false, true,  "@FILERECIPE_E020+", "@SITE5+" },
            { 0, QualSite2X2, false, false, "@FILERECIPE_E020+", "@SITE5+" },
        };
        TestIF_File.sRtcFileName = "RTCNAME_ASE";
        for (const S& c : cases) {
            CUSTOMER_CODE = c.cust; TestIF_File.iTestMode = c.mode; bUseTwoArm32Site = c.two; fSetup->fShow = c.show;
            Arm(false, false, false);
            W906_E020_RtcSkipsClear();
            Tick(2);
            char tag[160];
            std::snprintf(tag, sizeof(tag), "cust=%d mode=%d twoArm32=%d fSetup->fShow=%d -> %s %s", c.cust, c.mode, c.two ? 1 : 0, c.show ? 1 : 0, c.file, c.site);
            Check(SkipsWith("W906-E020-LI6-1", "", c.file) == 1 && SkipsWith("W906-E020-LI6-2", "", c.site) == 1, tag);
        }
    }

    std::printf("[8] source ratchets (foreign lines, same-line edits)\n");
    {
        std::string su, uh, fh, cm, src;
        Check(ReadAll(root + "/cSetUp.cpp", &su) && ReadAll(root + "/uhome.cpp", &uh) && ReadAll(root + "/forms/fLotInfo.h", &fh) &&
              ReadAll(root + "/CMakeLists.txt", &cm) && ReadAll(root + "/LotInfo_E020.cpp", &src), "read the five sources under " + root);
        const std::vector<std::string> S = Lines(su), U = Lines(uh), H = Lines(fh), C = Lines(cm);
        const std::vector<bool> SL = LiveMask(S), UL = LiveMask(U);
        const int c1 = Find(S, "fLotInfo->RTCChangeFile();"), c2 = Find(S, "fLotInfo->RTCChangeFile(!bFirstTime);");
        Check(c1 > 0 && SL[c1], "cSetUp.cpp: fLotInfo->RTCChangeFile(); is compiled (gate G-SU-RtcChangeFile lifted; golden V912 cSetUp.cpp:2846)");
        Check(c2 > 0 && SL[c2], "cSetUp.cpp: fLotInfo->RTCChangeFile(!bFirstTime); is compiled (golden :2854)");
        Check(c1 > 0 && Has(S[c1 - 1], "#if 1 // GATE(G-SU-RtcChangeFile) lifted") && c2 > 0 && Has(S[c2 - 1], "#if 1 // GATE(G-SU-RtcChangeFile) lifted"),
              "cSetUp.cpp: both gate lines say lifted (same line, line count unchanged)");
        const int u1 = Find(U, "fLotInfo->RTCChangeFile(false);"), u2 = Find(U, "COM2->InitRealTimeCCDPara();");
        Check(u1 > 0 && UL[u1], "uhome.cpp: fLotInfo->RTCChangeFile(false); is compiled (golden V912 uhome.cpp:1946)");
        Check(u2 > 0 && !UL[u2], "uhome.cpp: COM2->InitRealTimeCCDPara(); stays gated (TCOM2Shim has none)");
        Check(u1 > 0 && u2 == u1 + 2 && Has(U[u1 + 1], "#if 0 // GATE (W906-HOME-C2-RTCINIT)"), "uhome.cpp: call, then the narrowed gate, then InitRealTimeCCDPara");
        const int blk = u1 > 20 ? Find(U, "if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)", u1 - 20) : -1;
        Check(blk > 0 && blk < u1 && UL[blk], "uhome.cpp: the call is still inside the REAL_TIME_CCD && !bCCDDummyRum block (guard line just above it)");
        const int h = Find(H, "TStringList *W906_LotEndSkipped = new TStringList();");
        const std::string hc = h >= 0 ? CodePart(H[h]) : "";
        Check(h >= 0 && Has(hc, "void RTCChangeFile(bool bNeedDelete=true);") && Has(hc, "void Timer1Timer();") && Has(hc, "TfLotInfoTimer *Timer1 = new TfLotInfoTimer();"),
              "forms/fLotInfo.h: the three declarations sit on the S117 line, before its trailing //");
        int m = -1;
        for (size_t i = 0; i < C.size() && m < 0; ++i) {
            const std::string code = C[i].substr(0, C[i].find('#'));             // CMake: before the # comment
            if (Has(code, "cTestCategory.cpp")) m = (int)i;
        }
        Check(m >= 0 && Has(C[m].substr(0, C[m].find('#')), "LotInfo_E020.cpp"),
              "CMakeLists.txt: LotInfo_E020.cpp on the cTestCategory.cpp source line (ht9045_sm), before its # comment");
        int inSm = 0;
        for (int i = m; i >= 0; --i) {
            const std::string code = C[i].substr(0, C[i].find('#'));
            if (Has(code, "add_library(")) { inSm = Has(code, "add_library(ht9045_sm") ? 1 : 0; break; }
        }
        Check(inSm == 1, "CMakeLists.txt: that line belongs to add_library(ht9045_sm ...)");
        Check(!Has(src, "FormLock()"), "LotInfo_E020.cpp calls no FormLock() (act.* runs on the tick thread, E-021 precedent)");
    }

    InitialOK = keepInit; COM2->bCCDDummyRum = keepDummy; bDoROILearning = keepRoi; bSendRealCCDSendStart = keepSend;
    CUSTOMER_CODE = keepCC; TestIF_File.iTestMode = keepMode; bUseTwoArm32Site = keepTwo; fSetup->fShow = keepShow;
    IniConfig.bD31RTCChangeRecipeNeedreCreateModel = keepD31; fMain->cbSetupFileName->Text = keepSetup; TestIF_File.sRtcFileName = keepRtcName;
    f->Timer1->Enabled = false;
}

// ---- acts --------------------------------------------------------------------------------------------------------------------
std::string Act(const char* op, const std::string& value)
{
    const std::string cmd = std::string("act.lotInfo.") + op;
    const std::string r = ht9045::sjson::HandleActionWithTag(cmd, value, std::string());
    std::printf("    %s %s -> %s\n", cmd.c_str(), value.c_str(), r.substr(0, 260).c_str());
    return r;
}
std::string Click(const char* b, int seq) { return Act("palSecsGemMouseDown", std::string("{\"button\":\"") + b + "\",\"seq\":" + std::to_string(seq) + "}"); }
bool Exec(const std::string& r) { return Has(r, "\"executed\":true"); }

void RunActs(const std::string& root)
{
    TfLotInfo* f = fLotInfo;
    const int keepCC = CUSTOMER_CODE, keepBar = BAR_CODE_INSTALL, keepAcc = AccessLevel;
    const bool keepEn = TestIF_File.bEnableBarCode, keepSub = TestIF_File.b2DUseSubJob, keepSubF = CosFunction.b2DUseSubJobFunction;
    const bool keepStart = SystemStart, keepSecs = IniConfig.bEnable_SECS_GEM, keepLock = CosFunction.bLotStartLockCriticalPara;
    bBarcodeConnect = false;

    std::printf("[1] unknown op\n");
    {
        const std::string r = Act("nope", "{}");
        Check(!Exec(r) && Has(r, "\"guard\":\"unknown-action\"") && Has(r, "changeFile") && Has(r, "palSecsGemMouseDown"), "unknown op -> unknown-action with the list");
    }

    std::printf("[2] act.lotInfo.changeFile = golden btChangeFileClick :10394-10418\n");
    {
        CUSTOMER_CODE = 0;
        BAR_CODE_INSTALL = 0;
        std::string r = Act("changeFile", "{}");
        Check(!Exec(r) && Has(r, "\"guard\":\"tab-hidden\""), "no barcode installed: tsBarCode hidden (FormShow :571) -> tab-hidden");
        BAR_CODE_INSTALL = ebctUseCCDMode; CosFunction.b2DUseSubJobFunction = false;
        r = Act("changeFile", "{}");
        Check(!Exec(r) && Has(r, "\"guard\":\"button-hidden\""), "CCD mode without the sub-job function: btChangeFile hidden (FormShow :572) -> button-hidden");
        CosFunction.b2DUseSubJobFunction = true; TestIF_File.bEnableBarCode = false;
        r = Act("changeFile", "{}");
        Check(Exec(r) && Has(r, "\"branch\":0") && Has(r, "bEnableBarCode is off"), "bEnableBarCode off: golden does nothing -> executed, branch 0");
        TestIF_File.bEnableBarCode = true; TestIF_File.b2DUseSubJob = false;
        r = Act("changeFile", "{}");
        Check(Exec(r) && Has(r, "\"branch\":0") && Has(r, "no branch"), "CCD mode, setup sub job off: no golden branch -> executed, branch 0");
        TestIF_File.b2DUseSubJob = true;
        r = Act("changeFile", "{}");
        Check(!Exec(r) && Has(r, "\"guard\":\"gated\"") && Has(r, "\"branch\":3") && Has(r, "GATE (W906-E020-LI11-3)"), "CCD mode + sub job: branch 3 refused with GATE (W906-E020-LI11-3)");
        BAR_CODE_INSTALL = ebctInShtIntel;
        r = Act("changeFile", "{}");
        Check(!Exec(r) && Has(r, "\"branch\":1") && Has(r, "GATE (W906-E020-LI11-1)"), "in-shuttle Intel: branch 1 refused with GATE (W906-E020-LI11-1)");
        BAR_CODE_INSTALL = ebctEtherNetCCD;
        r = Act("changeFile", "{}");
        Check(!Exec(r) && Has(r, "\"branch\":2") && Has(r, "GATE (W906-E020-LI11-2)"), "Cognex EtherNet: branch 2 refused with GATE (W906-E020-LI11-2)");
        Check(bBarcodeConnect == false, "bBarcodeConnect never set alone");
        std::string why;
        Check(W906_E020_btChangeFileClick(&why) == 2 && Has(why, "GATE (W906-E020-LI11-2)") && W906_E020_btChangeFileClick(nullptr) == 2, "the callable returns the branch, whyNot optional");
    }

    std::printf("[3] act.lotInfo.palSecsGemMouseDown = golden palSecsGemMouseDown :10532-10606\n");
    {
        const std::string cfg = std::string(AuthPath.c_str()) + "config.ini";
        CUSTOMER_CODE = 0; SystemStart = false; AccessLevel = iDefHonPrecLevel; IniConfig.bEnable_SECS_GEM = false; CosFunction.bLotStartLockCriticalPara = false;
        f->edtSysLotID->Text = ""; f->edtSysOperatorID->Text = "";
        std::string r = Act("palSecsGemMouseDown", "[1]");
        Check(!Exec(r) && Has(r, "\"guard\":\"bad-payload\""), "value not an object -> bad-payload");
        r = Act("palSecsGemMouseDown", "{}");
        Check(!Exec(r) && Has(r, "\"guard\":\"bad-payload\""), "no button -> bad-payload");
        r = Act("palSecsGemMouseDown", "{\"button\":\"x\"}");
        Check(!Exec(r) && Has(r, "\"guard\":\"bad-payload\""), "button x -> bad-payload");
        CUSTOMER_CODE = CC_GIGAS;
        r = Click("left", 1);
        Check(!Exec(r) && Has(r, "\"guard\":\"tab-hidden\""), "CC_GIGAS: tsLotID hidden (FormShow :1035) -> tab-hidden");
        CUSTOMER_CODE = CC_MTI;
        r = Click("left", 2);
        Check(!Exec(r) && Has(r, "\"guard\":\"panel-hidden\""), "CC_MTI, no SECS / ATP lock: palSecsGem hidden (:367-371) -> panel-hidden");
        CUSTOMER_CODE = 0;
        Click("middle", 3);                                                     // middle resets golden's static iStep in every case
        const char* seq6[] = { "left", "left", "right", "right", "left", "left" };
        SystemStart = true;
        for (int i = 0; i < 6; ++i) r = Click(seq6[i], 10 + i);
        Check(Exec(r) && Has(r, "\"goldenReturn\":\"SystemStart\"") && f->edtSysLotID->Text == "" && f->edtSysOperatorID->Text == "",
              "SystemStart: golden returns at :10537 -> nothing filled");
        SystemStart = false; AccessLevel = iDefHonPrecLevel - 1;
        for (int i = 0; i < 6; ++i) r = Click(seq6[i], 20 + i);
        Check(Has(r, "\"goldenReturn\":\"AccessLevel<iDefHonPrecLevel\"") && f->edtSysLotID->Text == "", "below HonPrec level: golden returns at :10538 -> nothing filled");
        AccessLevel = iDefHonPrecLevel; CUSTOMER_CODE = CC_KYEC_LEE;
        for (int i = 0; i < 6; ++i) r = Click(seq6[i], 30 + i);
        Check(Has(r, "\"goldenReturn\":\"CC_KYEC_LEE\"") && f->edtSysLotID->Text == "", "CC_KYEC_LEE: golden returns at :10539 -> nothing filled");
        CUSTOMER_CODE = 0;
        Click("middle", 40);
        for (int i = 0; i < 5; ++i) r = Click(seq6[i], 41 + i);
        Check(f->edtSysLotID->Text == "" && Has(r, "\"filled\":false"), "five clicks of the six: nothing yet");
        Click("right", 46);                                                     // step 5 wants left: right resets (:10594-10604)
        r = Click("left", 47);
        Check(f->edtSysLotID->Text == "" && f->edtSysOperatorID->Text == "", "wrong sixth click resets the sequence: nothing filled");
        Click("middle", 48);
        for (int i = 0; i < 6; ++i) r = Click(seq6[i], 50 + i);
        Check(Exec(r) && Has(r, "\"filled\":true") && Has(r, "\"seq\":55"), "L L R R L L -> filled (reply echoes seq)");
        Check(f->edtSysLotID->Text == "0123456789" && f->edtSysOperatorID->Text == "12345", "Lot ID 0123456789 / Operator ID 12345 (golden :10597-10601)");
        Check(Has(r, "\"lotId\":\"0123456789\"") && Has(r, "\"operatorId\":\"12345\""), "the reply carries the new values");
        const AnsiString got = ReadIniData(AuthPath + "config.ini", "Lot Info", "Lot ID", AnsiString("<none>"));
        Check(got == "0123456789", "SetLotID wrote [Lot Info] Lot ID into the SANDBOX " + cfg + " (got " + std::string(got.c_str()) + ")");
        f->edtSysOperatorID->Text = "";
        for (int i = 0; i < 6; ++i) r = Click(seq6[i], 60 + i);
        Check(f->edtSysLotID->Text == "0123456789" && f->edtSysOperatorID->Text == "12345", "Lot ID already set: kept; empty Operator ID filled again");
        f->edtSysLotID->Text = "LOT-KEEP";
        for (int i = 0; i < 6; ++i) r = Click(seq6[i], 70 + i);
        Check(f->edtSysLotID->Text == "LOT-KEEP" && Has(r, "\"filled\":false"), "a filled Lot ID is not overwritten (:10597)");
        f->edtSysLotID->Text = ""; f->edtSysOperatorID->Text = "";
    }

    std::printf("[4] ratchet: JsonBridge/ChanAction.cpp dispatch (same line as act.main.clearRecord, before its //)\n");
    {
        std::string ca;
        Check(ReadAll(root + "/JsonBridge/ChanAction.cpp", &ca), "read JsonBridge/ChanAction.cpp");
        const std::vector<std::string> L = Lines(ca);
        const int i = Find(L, "if (cmd == \"act.main.clearRecord\")");
        const std::string code = i >= 0 ? CodePart(L[i]) : "";
        Check(i >= 0 && Has(code, "if (cmd.compare(0, 12, \"act.lotInfo.\") == 0)") && Has(code, "return W906_LotInfoE020Act(cmd, payloadJson);"),
              "act.lotInfo.* dispatch is code on the clearRecord line, before the trailing //");
    }

    CUSTOMER_CODE = keepCC; BAR_CODE_INSTALL = keepBar; AccessLevel = keepAcc; TestIF_File.bEnableBarCode = keepEn; TestIF_File.b2DUseSubJob = keepSub;
    CosFunction.b2DUseSubJobFunction = keepSubF; SystemStart = keepStart; IniConfig.bEnable_SECS_GEM = keepSecs; CosFunction.bLotStartLockCriticalPara = keepLock;
}

}  // namespace

int main(int argc, char** argv)
{
    const std::string mode = argc > 1 ? argv[1] : "";
    const std::string name = mode == "acts" ? "E020_LotInfoActs" : "E020_LotInfoRTC";
    const char* const rt[] = { "AuthPath", AuthPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects(name.c_str(), rt))
        return 2;
    if (mode != "rtc" && mode != "acts") { std::printf("usage: test_e020_lotinfo_e020 rtc|acts <port root>\n"); return 2; }
    const std::string root = SrcRoot(argc > 2 ? argv[2] : 0);

    std::string cfg0, gen0, cfg1, gen1;
    const bool hadCfg = ReadAll("D:\\HT9045\\config\\config.ini", &cfg0), hadGen = ReadAll("D:\\HT9045\\system\\Gerneral.ini", &gen0);
    std::printf("%s (AuthPath %s)\n", name.c_str(), AuthPath.c_str());

    if (mode == "rtc") RunRtc(root); else RunActs(root);

    const bool hadCfg1 = ReadAll("D:\\HT9045\\config\\config.ini", &cfg1), hadGen1 = ReadAll("D:\\HT9045\\system\\Gerneral.ini", &gen1);
    Check(hadCfg == hadCfg1 && cfg0 == cfg1, "real D:\\HT9045\\config\\config.ini unchanged");
    Check(hadGen == hadGen1 && gen0 == gen1, "real D:\\HT9045\\system\\Gerneral.ini unchanged");
    std::printf("%s: %d passed, %d failed\n", name.c_str(), g_pass, g_fail);
    return g_fail ? 1 : 0;
}
