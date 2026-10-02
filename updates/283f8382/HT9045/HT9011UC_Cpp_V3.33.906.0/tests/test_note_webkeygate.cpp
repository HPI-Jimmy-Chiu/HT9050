// ===========================================================================
//  tests/test_note_webkeygate.cpp -- AI(W906-NOTE-KEYGATE) 20261002: the alarm note's key lock for a key chosen on the screen
//  (forms/fNote_WebKeyGate.cpp = golden V906 note.cpp:2843-2870 TfNote::BtnSkipClick + :2764-2873 UpdateButtonStatus).
//  Each case sets one golden condition and checks the press is refused with its reason, then clears it and checks it passes.
//  Memory only: no IO table is loaded (IsSafeLockCheck reads disabled sensors -> false), fNote is NULL (the IsTestSitICFallDown
//  arm is covered by the source pin below); no machine file, no card.
// ===========================================================================
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "cmydef.h"
#include "Config.h"
#include "MachineType.h"

bool W906_NoteWebKeyGate(bool bOpenChamberDoor, std::string* why);
bool W906_NoteScreenStartActs();

static int g_fail = 0, g_pass = 0;
static void check(bool ok, const char* what) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", what);
    if (ok) ++g_pass; else ++g_fail;
}
static bool has(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }

int main(int argc, char** argv) {
    std::string why;
    bContactCTOverCHK = false; bAutoCleanCheckOpenDoor = false; bChangeCleanPad = false;
    bAutoRetestJam = false; bOpenAllDoor = false; bIsTestSitICFallDown = false;
    IniConfig.bIndexJamInArmAway = false; IniConfig.bD40IndexICFallDownMustPressFMotorDown = false;
    check(W906_NoteWebKeyGate(false, &why), "nothing set -> the press is accepted");

#ifdef SOFT_SIMULTE   // AI(W906-MACH1002) 20261002: golden BtnSkipClick :2847-2849 -- the SIM arm checks only IsSafeLockCheck, so these three
                      //   flags do not lock a key in SIM (fNote_WebKeyGate.cpp follows it; the machine builds ship only, so its test had only the ship arm)
    bContactCTOverCHK = true; why.clear();
    check(W906_NoteWebKeyGate(false, &why), ":2847-2849 SIM: bContactCTOverCHK is not checked -> accepted");
    bContactCTOverCHK = false;
    bAutoCleanCheckOpenDoor = true; why.clear();
    check(W906_NoteWebKeyGate(false, &why), ":2847-2849 SIM: bAutoCleanCheckOpenDoor is not checked -> accepted");
    bAutoCleanCheckOpenDoor = false;
    bChangeCleanPad = true; why.clear();
    check(W906_NoteWebKeyGate(false, &why), ":2847-2849 SIM: bChangeCleanPad is not checked -> accepted");
    bChangeCleanPad = false;
#else
    bContactCTOverCHK = true; why.clear();
    check(!W906_NoteWebKeyGate(false, &why) && has(why, "bContactCTOverCHK") && has(why, "note-key-locked"),
          ":2853 bContactCTOverCHK -> refused, reason names it");
    bContactCTOverCHK = false;
    bAutoCleanCheckOpenDoor = true; why.clear();
    check(!W906_NoteWebKeyGate(false, &why) && has(why, "bAutoCleanCheckOpenDoor"), ":2856 bAutoCleanCheckOpenDoor -> refused");
    bAutoCleanCheckOpenDoor = false;
    bChangeCleanPad = true; why.clear();
    check(!W906_NoteWebKeyGate(false, &why) && has(why, "bChangeCleanPad"), ":2857 bChangeCleanPad -> refused");
    bChangeCleanPad = false;
#endif

    bAutoRetestJam = true; bOpenAllDoor = false; why.clear();
    check(!W906_NoteWebKeyGate(false, &why) && has(why, "bOpenAllDoor"), ":2791 bAutoRetestJam && !bOpenAllDoor -> refused");
    bOpenAllDoor = true;
    check(W906_NoteWebKeyGate(false, &why), ":2791 bAutoRetestJam with the doors opened -> accepted");
    bAutoRetestJam = false; bOpenAllDoor = false;

    CUSTOMER_CODE = 0;
    IniConfig.bIndexJamInArmAway = true; IniConfig.bD40IndexICFallDownMustPressFMotorDown = true; bIsTestSitICFallDown = true;
    why.clear();
    check(!W906_NoteWebKeyGate(false, &why) && has(why, "bOpenChamberDoor"), ":2781-2789 index jam, chamber door not opened -> refused");
    check(W906_NoteWebKeyGate(true, &why), ":2781-2789 index jam, chamber door opened (TfNote member passed in) -> accepted");
    CUSTOMER_CODE = CC_SCK;
    check(W906_NoteWebKeyGate(false, &why), ":2781 CC_SCK is exempt -> accepted");
    CUSTOMER_CODE = 0; IniConfig.bIndexJamInArmAway = false; IniConfig.bD40IndexICFallDownMustPressFMotorDown = false;
    bIsTestSitICFallDown = false;
    check(W906_NoteWebKeyGate(false, &why), "all cleared again -> accepted");

    // AI(W906-NOTE-SCREENSTART) 20261002: golden BtnStartClick :3806-3824 -- the on-screen START acts only for SIGURD_PeiXing or in SIM
#ifdef SOFT_SIMULTE
    CUSTOMER_CODE = CC_PTI;  check(W906_NoteScreenStartActs(), ":3821-3822 SIM: the on-screen START calls Start()");
#else
    CUSTOMER_CODE = CC_PTI;  check(!W906_NoteScreenStartActs(), ":3816-3820 CC_PTI (this machine): the on-screen START does nothing");
    CUSTOMER_CODE = CC_SIGURD_PeiXing;  check(W906_NoteScreenStartActs(), ":3811-3815 CC_SIGURD_PeiXing: the on-screen START calls Start()");
#endif
    CUSTOMER_CODE = 0;
    // source pins (argv[1] = the port root): the web answer path calls the gate for every key but RESET, before the D-026 gate,
    // and refuses with the reason; the physical-key path keeps its own lock; the IC-fall arm is in the gate
    if (argc > 1) {
        std::ifstream f((std::string(argv[1]) + "/tools/wb_serve.cpp").c_str(), std::ios::binary);
        std::stringstream ss; ss << f.rdbuf(); const std::string src = ss.str();
        const std::size_t a = src.find("if (k != 0 && (k & kcode) != 0 && k != K_RESET) { std::string kgWhy; if (!W906_NoteWebKeyGate(");
        const std::size_t b = src.find("extern bool W906_NoteAuthAnswerGate(");
        check(a != std::string::npos && b != std::string::npos && a < b, "wb_serve.cpp: the web answer path gates the key before the D-026 password gate");
        check(src.find("g_w906NoteChamberDoorPtr = &bOpenChamberDoor;") != std::string::npos, "wb_serve.cpp: W906_AlarmIoAnswer shares its bOpenChamberDoor");
        const std::size_t s = src.find("pressed == \"BtnStart\" && !W906_NoteScreenStartActs()) {");
        check(s != std::string::npos && a != std::string::npos && s < a && s < b,
              "wb_serve.cpp: the on-screen START is refused (golden BtnStartClick) before the key gate and the D-026 gate");
        check(src.find("if (pressed.empty() || pressed == \"BtnStart\") { extern bool W906_AlarmAnswerStartLikeGolden") != std::string::npos,
              "wb_serve.cpp: the START side effect is still there for the SIM / SIGURD arm");
        std::ifstream g((std::string(argv[1]) + "/forms/fNote_WebKeyGate.cpp").c_str(), std::ios::binary);
        std::stringstream gs; gs << g.rdbuf();
        check(gs.str().find("fNote && fNote->IsTestSitICFallDown()") != std::string::npos, "fNote_WebKeyGate.cpp: the IC-fell-into-the-site arm (golden :2851)");
    } else {
        check(false, "argv[1] (the port root) missing: source pins not run");
    }
    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
