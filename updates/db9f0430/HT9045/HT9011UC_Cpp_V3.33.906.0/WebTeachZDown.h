// =============================================================================
//  WebTeachZDown.h  --  INBOX 147: the Teach page's Set All In / Out Arm Z and Out Z All Down (golden uteach.cpp)
//
//  AI(W906-TEACH-ZDOWN) 20261003.  NOT in golden: the web landing of three golden TfTeach button events,
//  golden = HT9011UC_Code_V3.33.906.0_20260618 (RULINGS_20261001 #0):
//    btnSetAllInArmZClick   uteach.cpp:4300-4365  "Set InArm Z"   (PageControl2 > Hand Pitch > InArm  > GroupBox12, uteach.dfm:3291)
//    btnSetAllOutArmZClick  uteach.cpp:4367-4435  "Set OutArm Z"  (PageControl2 > Hand Pitch > OutArm > GroupBox13, uteach.dfm:3570)
//    btnOutZAllDownClick    uteach.cpp:4542-4562  "All Down"      (PageControl2 > Output Arm > Pick Up > Panel25, uteach.dfm:9405)
//  The two Set All buttons MOVE NOTHING: every nozzle's Z edit = MOT[XArmZIndex[i][j]].ReadPos(), then minus the base nozzle's
//  (golden only fills the TEdits; the operator saves them with the page's Save, like every other teach value).
//  Out Z All Down MOVES every Out Z: SetSpeed(10) on each, then MotorMove(atoi(SetEditPickOutSht) + atoi(OutZEditPtr[i][j])).
//
//  Layers (WebMotorAccess.h header): the handlers are ONE block at the end of WebMotorAccess.cpp (motor.access actions
//  teachSetAllArmZ / teachOutZAllDown) and drive the existing paths of that file (the 1203 monitor's position, GoldenMove1203,
//  Send1203Speed, the golden object's ReadPos / SetSpeed / MotorMove) through IMotorAccessBackend, which is NOT extended.
//  What golden reads beyond it is ITeachZDownOps below: the sucker grids, golden's Z motor tables, the base nozzle,
//  USE_PICKER_COUNT, the arm's X / Y / pitch motors (the busy gates), golden's per-motor motion test of a non-1203 motor and
//  golden TOTAL_MOTOR (All Down's busy gate walks every motor, decision 1 = B 20261003).  The live
//  implementation is WebTeachZDownLive.cpp (wb_serve only; it installs itself at static initialisation).  Not installed (every
//  other binary, every older ctest) => both actions are refused with the reason, nothing is touched.
//
//  Transport (web/page/ht9045_teach_zdown_c.js): motor.access, catalog rows (motor-access.json, source uteach)
//    btnSetAllInArmZ / btnSetAllOutArmZ -> teachSetAllArmZ (kind edit),  btnOutZAllDown -> teachOutZAllDown (kind motion);
//    params.fields = {edit id: integer} -- the screen values golden reads with atoi (the page sends only values loaded from C++
//    or typed, the W5B-4 rule of TeachFieldValue).
// =============================================================================
#ifndef HT9045_WEBTEACHZDOWN_H
#define HT9045_WEBTEACHZDOWN_H

#include <string>
#include <vector>

namespace ht9045 {

// golden ePickCount ep4Picker (MachineType.h:1304) -- the `USE_PICKER_COUNT==0` branch of the two Set All handlers
//   (golden uteach.cpp:4312 / :4381, "Ifor 20170309 (wei) add HT9045S"). WebTeachZDownLive.cpp static_asserts it.
enum { kZdPicker4 = 0 };

// One arm, the way golden's handlers see it at the moment of the press.
struct TeachZArmGrid {
    int motRow, motCol;         // golden InArmSuck / OutArmSuck .iMotRow / .iMotCol   (Out Z All Down loops these, :4546-4548 / :4555-4557)
    int maxRow, maxCol;         // golden .iMaxRow / .iMaxCol                         (the Set All loops, In :4314-4316 / :4327-4329, Out :4383-4385 / :4396-4398)
    int yBase, xBase;           // golden iInArmYBase / iInArmXBase (Out: iOutArmYBase / iOutArmXBase; database.cpp:801-951)
    int pickerCount;            // golden USE_PICKER_COUNT
    int zIndex[2][8];           // golden InArmZIndex / OutArmZIndex (cmydef.cpp:3333 / :3335)
    std::vector<int> armAxes;   // MOT indexes of the arm's X, Y and pitch motors (X, Y, Pitch, PitchY, PitchX2, X3, X4) -- the busy gate
    TeachZArmGrid() : motRow(0), motCol(0), maxRow(0), maxCol(0), yBase(0), xBase(0), pickerCount(0)
    {
        for (int i = 0; i < 2; ++i) for (int j = 0; j < 8; ++j) zIndex[i][j] = -1;
    }
};

class ITeachZDownOps {
public:
    virtual ~ITeachZDownOps() {}
    // inArm true = the In arm, false = the Out arm.  false + why = cannot say (the press is refused, nothing touched).
    virtual bool ArmGrid(bool inArm, TeachZArmGrid& out, std::string& why) = 0;
    // golden VerifyMotorAction's per-motor test (uteach.cpp:5365-5389) of a motor this tree drives through its golden object (not
    //   a PCI1203 row: the 1203 monitor's axis is read through IMotorAccessBackend): an Index motor with INDEX_MOTION_CARD==0 ->
    //   ScanMotorStatus + Led[iInposLed] (golden's Galil branch); any other -> MOT[mi].Motor->MotionDone().
    //   1 = done, 0 = moving, -1 = no such motor / disabled / unknown.
    virtual int GoldenMotionDone(int motIndex) = 0;
    //AI(W906-TEACH-ZDOWN) 20261003: decision 1 = B -- golden TOTAL_MOTOR (cmydef.h), the bound of VerifyMotorAction's
    //   for(i<TOTAL_MOTOR): Out Z All Down's busy gate asks MOT[0 .. MotorCount()-1].  <= 0 = unknown -> All Down refused.
    virtual int MotorCount() = 0;
};

void            TeachZDownInstallOps(ITeachZDownOps* ops);   // WebTeachZDownLive.cpp (wb_serve); tests install a fake; 0 = none
ITeachZDownOps* TeachZDownInstalledOps();

}  // namespace ht9045

#endif  // HT9045_WEBTEACHZDOWN_H
