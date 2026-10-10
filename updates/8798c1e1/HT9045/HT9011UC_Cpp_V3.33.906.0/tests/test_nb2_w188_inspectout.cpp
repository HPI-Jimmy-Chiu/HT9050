// =============================================================================
//  AI(W906-W188) 20261010 (NB2-1): W-188 #9 -- InspectOutArmPosition (E74 debug position check) = golden 913
//  (GitLab honprec/rd/rd5/ht9045_913 main e9908638) aoutarm9045.cpp:4169 (iOutPlaceToRotate=2), :4171-4328, aoutarm9045.h:76.
//  913 (ht9045-v899 20260811) vs the port's 0618 body:
//    * picks / rotator places are checked again (0618 `return;  //Steven 20241220` commented out, :4199);
//    * a place to BulkBox is not checked (:4223-4224) -- eBulkBox == MOutShuttle2 == 18, eFix12 == MOutShuttle1 == 17;
//    * the shuttle encoder formula only for picks (:4231-4233); the BulkBox position block commented out (:4243-4248);
//    * the shuttle X offset uses the same column step as GetOutArmToShtCellPos (:4288);
//    * the rotator target is each hole, from iOutArm_RotateX/Y and the rotate-kit pitches (:4309-4313).
//  [1] rotator place: the hole under the arm passes, another hole reports "Place to Out Rotator [r, c]" with 913's numbers.
//  [2] place to BulkBox: no check.   [3] a pick from shuttle 1 is checked ("Pick from Shuttle 1").   [4] source ratchets.
//  Use: only through ctest (NB2_W188InspectOut).
// =============================================================================
#include "aoutarm9045.h"
#include "aHotPlateSubstrate.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "canary_support.h"
#include "mykitsuck.h"
#include "Motor/mymotor.h"
#include "forms/fRotate.h"
#include "RotateKit/aRotateKIT.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

void SetMyKitSuckItemAmount();
void EnsureArmOffsetObjects();

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::vector<std::string> ReadLines(const std::string& path)
{
    std::vector<std::string> v;
    std::ifstream f(path.c_str(), std::ios::binary);
    std::string s;
    while (std::getline(f, s)) {
        if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
        v.push_back(s);
    }
    return v;
}
static int Count(const std::vector<std::string>& L, const std::string& n)
{
    int c = 0;
    for (size_t i = 0; i < L.size(); ++i) if (L[i].find(n) != std::string::npos) ++c;
    return c;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("NB2_W188InspectOut"))
        return 2;
    std::printf("==== W-188 #9 InspectOutArmPosition -> golden 913 ====\n");

    char msg[300];
    W906_TestEnsureSimMotors();
    SetMyKitSuckItemAmount();
    EnsureArmOffsetObjects();
    for (int i = 0; i < eTrayCount; ++i) AutoForm[i] = &TrayForm.Auto[i];
    OutArmSuck.iPickStep = 1;
    OutArmSuck.Item[iOutArmYBase][iOutArmXBase] = HAS_IC;
    InputLimit.iOffsetXYHigh = 1;                     // +-100 pulses
    AUTO3_IS_MAGAZINE = 0;
    IniConfig.bE74_InspectArmPosition = true;

    // ---------------------------------------------------------------- [1] place to the out rotator
    std::printf("-- [1] place to rotator --\n");
    tRotate.ColCount = 2;
    iRotateKIT_Start_X_H = 1000;  iRotateKIT_Pitch_X_H = 2000;
    iRotateKIT_Start_Y_H = 3000;  iRotateKIT_Pitch_Y_H = 4000;
    Prod.iOutArm_RotateX = 500000; Prod.iOutArm_RotateY = 600000;
    Prod.iOutArmRotateToUnloaderX = 0; Prod.iOutArmRotateToUnloaderY = 0;   // the 0618 single point
    // hole [0, 0]: X = 500000 + 1000 - 2000*(1-0) = 499000; Y = 600000 - 3000 - 4000*(0-iOutArmYBase)
    const int y00 = 600000 - 3000 - 4000 * (0 - iOutArmYBase);
    MOT[MOutArmX].Motor->SetPosition(499000);
    MOT[MOutArmY].Motor->SetPosition(y00);
    W906_ShowMyMessage_Reset();
    InspectOutArmPosition(MOutRotateKit, iOutArmYBase, iOutArmXBase, 0, 0, iOutPlaceToRotate);
    std::snprintf(msg, sizeof msg, "arm over hole [0, 0]: within tolerance, no message (%d)", W906_ShowMyMessage_Count);
    CHECK(W906_ShowMyMessage_Count == 0, msg);
    W906_ShowMyMessage_Reset();
    InspectOutArmPosition(MOutRotateKit, iOutArmYBase, iOutArmXBase, 1, 1, iOutPlaceToRotate);
    const int y11 = 600000 - 3000 - 4000 * (1 - iOutArmYBase);
    char want[120];
    std::snprintf(want, sizeof want, "Place to Out Rotator [1, 1] Pos(Y=%d, X=%d)", y11, 501000);
    const std::string got1 = W906_ShowMyMessage_LastS1.c_str();
    std::snprintf(msg, sizeof msg, "arm over [0, 0], target hole [1, 1]: one message \"%s\" (count %d: %.120s)", want, W906_ShowMyMessage_Count, got1.c_str());
    CHECK(W906_ShowMyMessage_Count == 1 && got1.find(want) != std::string::npos, msg);

    // ---------------------------------------------------------------- [2] place to BulkBox: not checked
    std::printf("-- [2] place to BulkBox --\n");
    Prod.iOutArmBinBoxX = 0; Prod.iOutArmBinBoxY = 5000;          // 0618 target Y = 5000 - 5000*200: far from the arm
    W906_ShowMyMessage_Reset();
    InspectOutArmPosition(eBulkBox, iOutArmYBase, iOutArmXBase, 0, 0, iOutPlaceToAuto);
    std::snprintf(msg, sizeof msg, "place to BulkBox: no check, no message (%d)", W906_ShowMyMessage_Count);
    CHECK(W906_ShowMyMessage_Count == 0, msg);

    // ---------------------------------------------------------------- [3] a pick from shuttle 1 is checked again
    std::printf("-- [3] pick from shuttle 1 --\n");
    MOT[MOutArmX].Motor->SetPosition(7000000);
    MOT[MOutArmY].Motor->SetPosition(7000000);
    W906_ShowMyMessage_Reset();
    InspectOutArmPosition(MOutShuttle1, iOutArmYBase, iOutArmXBase, 0, 0, iOutPickFromSht);
    const std::string got3 = W906_ShowMyMessage_LastS1.c_str();
    std::snprintf(msg, sizeof msg, "arm far from shuttle 1: one \"Pick from Shuttle 1\" message (count %d: %.120s)", W906_ShowMyMessage_Count, got3.c_str());
    CHECK(W906_ShowMyMessage_Count == 1 && got3.find("Pick from Shuttle 1 [0, 0]") != std::string::npos, msg);
    IniConfig.bE74_InspectArmPosition = false;

    // ---------------------------------------------------------------- [4] source ratchets
    std::printf("-- [4] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> C = ReadLines(root + "/aoutarm9045.cpp");
    const std::vector<std::string> H = ReadLines(root + "/aoutarm9045.h");
    CHECK(Count(C, "int  iOutPlaceToAuto=1;   int  iOutPlaceToRotate=2;") == 1 && Count(H, "extern int iOutPlaceToAuto;   extern int iOutPlaceToRotate;") == 1,
          "iOutPlaceToRotate defined in code and declared in the header");
    CHECK(Count(C, "        if(iAction!=iOutPlaceToAuto && (iTarget==MOutShuttle1 ||") == 1 && Count(C, "           iTarget==MOutShuttle2))") >= 1,
          "a place to Fix12 / BulkBox does not take the shuttle encoder formula");
    CHECK(Count(C, "                    HardwarePosX+=(iXoffset/3)*(iSuckCol*OutArmSuck.iPickStep-iOutArmXBase);") == 1,
          "shuttle X offset uses the GetOutArmToShtCellPos column step");
    CHECK(Count(C, "//            if(iTarget==eBulkBox)") == 1 && Count(C, "    if(iAction==iOutPlaceToAuto && iTarget==eBulkBox) return;") == 1,
          "BulkBox: early return, the position block commented out");

    std::printf("==== W-188 #9: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
