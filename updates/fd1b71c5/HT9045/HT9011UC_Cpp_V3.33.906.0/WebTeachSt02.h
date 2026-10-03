// =============================================================================
//  WebTeachSt02.h  --  St02 card ST02-C9: Teach page buttons the laptop has not wired (golden uteach.cpp)
//
//  AI(W906-ST02-C9-G2) 20261002 (St02-E helper).  NOT in golden: the web landing of golden TfTeach button events,
//  card ST02-C9 (docs/handoff/TO_STEVEN.md row 20261002 07:1x).  golden = HT9011UC_Code_V3.33.906.0_20260625_Steven.
//
//  Group G2 (this round):
//    SpeedButtonInRotatePos90Click / SpeedButtonInRotateNeg90Click   golden uteach.cpp:4605-4655 / :4657-4707 (36 buttons)
//    btnSht1GoLatchClick / btnSht2GoLatchClick                       golden uteach.cpp:4721-4735 / :4737-4751
//    btnSetAllInArmZ_MoveClick / btnSetAllOutArmZ_MoveClick          golden uteach.cpp:5948-5971 / :5973-5996
//  Group G3 (the Tray Arm lanes, the Index tab's Galil servo buttons, the TTL tab) is NOT here -- AI(W906-ST02-C9) 20261002
//    (St02-E helper): the machine wired them on main first (BtnPanelLane1-3 = IOWIDGET 60cc29f6, btnZ1 / Z2 / Arm1Y / Arm2YServo
//    = TEACH-ZALLUP 71132e21, spTTLReset / cbEnableTTLButtonUse greyed with the reason = TEACH-FORMSHOW 00906385), so St02's G3
//    was dropped in the C9 rebase onto main 2dd90ef3.
//  Group G1 (the pitch family, golden uteach.cpp:5486-5932) is parked: nothing of it is in these files yet.
//
//  Layers (WebMotorAccess.h header): the handlers are ONE block at the end of WebMotorAccess.cpp and drive the
//  existing W5-b teach-move path (TeachPrelude / Move1203 / MoveGolden / TeachRotatorBacklash ...) through
//  IMotorAccessBackend, which is NOT extended.  What golden needs beyond it is ITeachSt02Ops below: configuration
//  flags, golden's rotate-motor list, MOT[i].fCMD / InitMOTParameter, Tech, the sucker grids and the Prod Z heights.
//  The live implementation is WebTeachSt02Live.cpp (wb_serve only; it installs itself).  Not installed (every other
//  binary and every older ctest) => every teachSt02 request is refused with the reason, nothing is touched.
//
//  Transport (web/page/ht9045_teach_st02.js):
//    motor.access, action "teachSt02", catalog button "St02Teach*" (motor-access.json, kind motion), params.btn = the
//    golden button name, plus the field values golden reads with atoi (backlashIn / backlashOut, shtSpeed).
//    The page's state query: params {btn:"St02State", query:true} (sent straight to HT9045Recipe.motorAccess).
// =============================================================================
#ifndef HT9045_WEBTEACHST02_H
#define HT9045_WEBTEACHST02_H

#include <string>
#include <vector>

#include "WebMotorAccess.h"

namespace ht9045 {

// golden eRotateType (MachineType.h:606-614).  WebTeachSt02Live.cpp static_asserts these against the enum.
enum {
    kSt02RotCyn = 0, kSt02Rot1Mot = 1, kSt02Rot4Mot = 2, kSt02Rot8Mot = 3,
    kSt02Rot1Mot1Dut = 4, kSt02Rot2Mot2Dut = 5, kSt02RotInOutArm1Motor = 6
};
// golden ePickCount ep16Picker (MachineType.h:1433; golden MachineType.h:1307).  Static-asserted in the Live file too.
enum { kSt02Picker16 = 3 };
// ITeachSt02Ops::GoldenMotorIndex(which)
enum { kSt02MotInShuttle1 = 1, kSt02MotInShuttle2 = 2 };
// ITeachSt02Ops::RotateMotorKind(mi): golden's list of SpeedButtonInRotatePos90Click :4628-4635
enum { kSt02RotNone = 0, kSt02RotInKit = 1, kSt02RotOutKit = 2, kSt02RotOther = 3 };

// What golden FormShow reads for the visibility of these buttons, and the iRotate_Type of the 90-degree step.
struct TeachSt02Config {
    int  useRotateKit;          // USE_ROTATE_KIT   (database.cpp:1372, ROTATE_KIT USE_ROTATE_KIT, CheckRange 0..1)
    int  rotateType;            // iRotate_Type     (database.cpp:1375, ROTATE_KIT RotateKit_Type)
    int  usePickerCount;        // USE_PICKER_COUNT (database.cpp:878,  System USE_PICKER_COUNT)
    bool shtSensorGroupShown;   // golden grpShtSensor->Visible: uteach.dfm:5850 Visible = False, and no golden statement
                                //   sets it true (0625 grep: uteach.cpp has no grpShtSensor line) => false on every machine
    TeachSt02Config() : useRotateKit(0), rotateType(0), usePickerCount(0), shtSensorGroupShown(false) {}
};

// One nozzle of golden btnSetAll*ArmZ_MoveClick's loop (i over iMotRow, j over iMotCol).
struct TeachSt02ArmZCell {
    int  row, col;              // golden i / j
    int  motIndex;              // golden InArmSuck / OutArmSuck .Suck[i][j].iMotNo
    bool inPick;                // golden `i<iPickRow && j<iPickCol` (only these are moved)
    bool targetKnown;           // [i][j] inside Prod's [MAX_ARM_Row][MAX_ARM_Col] (golden would read past it otherwise)
    int  target;                // golden Prod.ZInArm_Tray_Pick[i][j] / Prod.ZOutArm_Auto_Place[0][i][j]
    TeachSt02ArmZCell() : row(0), col(0), motIndex(-1), inPick(false), targetKnown(false), target(0) {}
};
struct TeachSt02ArmZGrid {
    int motRow, motCol, pickRow, pickCol;   // golden iMotRow / iMotCol / iPickRow / iPickCol of that kit
    std::string note;                       // non-empty: the loop was clipped to Suck[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]
    std::vector<TeachSt02ArmZCell> cells;   // golden loop order
    TeachSt02ArmZGrid() : motRow(0), motCol(0), pickRow(0), pickCol(0) {}
};

class ITeachSt02Ops {
public:
    virtual ~ITeachSt02Ops() {}
    virtual TeachSt02Config Config() = 0;
    // 0 = not in golden's rotate list, 1 = MInRotateKit, 2 = MOutRotateKit, 3 = MInRotateB..H / MOutRotateB..H
    virtual int  RotateMotorKind(int motIndex) = 0;
    // golden MOT index of kSt02MotInShuttle1 / 2 (MInShuttle1 = 11 / MInShuttle2 = 12, cmydef.cpp:2350-2351); -1 = none
    virtual int  GoldenMotorIndex(int which) = 0;
    virtual void GoldenClearFCmd(int motIndex) = 0;           // golden MOT[i].fCMD=false (memory)
    virtual void GoldenInitMOTParameter(int motIndex) = 0;    // golden MOT[i].InitMOTParameter() (memory, Motor/mymotor.cpp:374-380)
    virtual int  TechInShuttleRight(int which) = 0;           // golden Tech.iInShuttle1Right / Tech.iInShuttle2Right
    virtual TeachSt02ArmZGrid ArmZGrid(bool inArm) = 0;       // InArmSuck (true) / OutArmSuck (false) + the Prod Z heights
};

// The installed ops (WebMotorAccess.cpp EOF holds the pointer; null = not installed).
void           TeachSt02InstallOps(ITeachSt02Ops* ops);
ITeachSt02Ops* TeachSt02InstalledOps();

// ---- golden's pure parts (tests check them directly) ----
// SpeedButtonInRotatePos90Click :4613-4620: e4MotRotate / e8MotRotate / e2MotRotate2Dut -> 800, eInOutArm1Motor -> 1250, else 2000
int  TeachSt02RotateP2(int rotateType);
// FormShow :1652 grpRotate_Kit->Visible=(USE_ROTATE_KIT==1 && iRotate_Type in {e1MotRotate, e1MotRotate1Dut, eInOutArm1Motor})
bool TeachSt02RotateKitShown(const TeachSt02Config& c);
// FormShow :1457-1458 pnlInArmZ / pnlOutArmZ ->Visible=(USE_PICKER_COUNT==ep16Picker)
bool TeachSt02ArmZPanelShown(const TeachSt02Config& c);
//AI(W906-ST02-C9-G2) 20261002: the 36 buttons whose golden OnClick is SpeedButtonInRotatePos90Click / ...Neg90Click (uteach.dfm):
//   SpeedButtonInRotatePos90 / Neg90, SpeedButtonOutRotatepPos90 / Neg90 (TabSheet14 > grpRotate_Kit), btnPos90InRx / btnNeg90InRx
//   (tsRotate > grpInRotM8 > grbInRx) and btnPos90OutRx / btnNeg90OutRx (tsRotate > grpOutRotM8 > grbOutRx), x = A..H.
// FormShow :1668 tsRotate->TabVisible=(USE_ROTATE_KIT==1 && iRotate_Type in {e8MotRotate, e4MotRotate, e2MotRotate2Dut})
bool TeachSt02RotateTabShown(const TeachSt02Config& c);
// golden's own Visible of "grpRotate_Kit" (:1652) or "grbInRA".."grbOutRH" (:1690-1703; grbInRA / grbInRE: no line, the dfm's
//   True); false for any other name.  A grb is only reachable when the tab is shown too (TeachSt02RotateButtonShown).
bool TeachSt02RotateGroupShown(const TeachSt02Config& c, const std::string& group);
// one of the 36: true, positive = the Pos90 handler, group = the golden parent whose Visible decides; else false
bool TeachSt02RotateButton(const std::string& btn, bool& positive, std::string& group);
// golden can press it on this configuration (kit: grpRotate_Kit; tsRotate: the tab AND its grb); false for any other name
bool TeachSt02RotateButtonShown(const TeachSt02Config& c, const std::string& btn);

// action "teachSt02" (WebMotorAccess.cpp EOF; MotorAccessDispatch's uteach block calls it after the run / manual-teach gates)
MotorAccessOutcome TeachSt02Dispatch(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be);

// golden's form statics as C++ holds them (state query, tests)
struct TeachSt02State {
    bool setAllInArmZB;         // golden btnSetAllInArmZ_MoveClick  `static bool b` (:5950): false = next press goes to the pick height
    bool setAllOutArmZB;        // golden btnSetAllOutArmZ_MoveClick `static bool b` (:5975)
    int  shuttleSelect;         // golden `static int iShuttleSelect` (:4709), read by btnGetLatchClick (:4755)
    TeachSt02State() : setAllInArmZB(false), setAllOutArmZB(false), shuttleSelect(0) {}
};
TeachSt02State TeachSt02States();
void           TeachSt02ResetForTests();   // the statics back to golden's initial values (the ops pointer is not touched)

}  // namespace ht9045

#endif  // HT9045_WEBTEACHST02_H
