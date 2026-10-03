// =============================================================================
//  WebTeachZDownLive.cpp  --  the live ITeachZDownOps (WebTeachZDown.h): golden's globals behind Set All In / Out Arm Z and
//  Out Z All Down (golden uteach.cpp:4300-4435 / :4542-4562)
//
//  AI(W906-TEACH-ZDOWN) 20261003.  Linked into wb_serve ONLY (root CMakeLists.txt, next to WebTeachSt02Live.cpp); it installs
//  itself at static initialisation (WebMotorAccess.cpp EOF holds the pointer, a constant-initialised 0, so the order of the two
//  TUs does not matter).  Every other binary links no ops and both actions there are refused with "not installed".
//  Read only: it copies golden's grids / tables / base nozzle / USE_PICKER_COUNT / TOTAL_MOTOR and asks golden's per-motor motion
//  test of a non-1203 motor (golden VerifyMotorAction's: MotionDone, or for a Galil Index motor ScanMotorStatus + Led[iInposLed] --
//  that one refreshes MOT[i].Led[] from the card, as golden VerifyMotorAction does on every tick while Teach is open).
//  No file, no IO, no other golden memory write here: the reads and moves go through WebMotorAccess.cpp (the 1203 monitor,
//  GoldenMove1203 / Send1203Speed = EastSun's 1203 layer, the golden object's ReadPos / SetSpeed / MotorMove).
//  InArmSuck / OutArmSuck through aHotPlateSubstrate.h -- the header the machine's W906_TeachFixedMotorsLive and ArmCellLive.cpp
//  read them through (since AI(W906-A4-6) it includes mykitsuck.h, the one live TMyKitSuck layout).
// =============================================================================
#include "WebTeachZDown.h"

#include <cstdio>

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"          // ePickCount (ep4Picker)
#include "cmydef.h"               // USE_PICKER_COUNT, InArmZIndex / OutArmZIndex, iIn/OutArmY/XBase, MInArmX .. MOutArmPitchX4, TOTAL_MOTOR, INDEX_MOTION_CARD, MTestY1 / Z1 / Z2 / Y2
#include "aHotPlateSubstrate.h"   // InArmSuck / OutArmSuck (the live TMyKitSuck, mykitsuck.h layout since A4-6)
#include "Motor/mymotor.h"        // MOT[MAX_TRAY_MOTOR], iInposLed (Motor/HTMotor.h)

// the header's copy of golden's enum must be golden's value (int casts: two different enum types, -Wenum-compare)
static_assert((int)ep4Picker == (int)ht9045::kZdPicker4, "WebTeachZDown.h kZdPicker4 != golden ep4Picker (MachineType.h)");

namespace ht9045 {
namespace {

class ZdLiveOps : public ITeachZDownOps {
public:
    ZdLiveOps() { TeachZDownInstallOps(this); }

    bool ArmGrid(bool inArm, TeachZArmGrid& g, std::string& why) override
    {
        const TMyKitSuck& k = inArm ? InArmSuck : OutArmSuck;                   // golden uteach.cpp:4314 / :4383 / :4546
        g = TeachZArmGrid();
        g.motRow = k.iMotRow; g.motCol = k.iMotCol;
        g.maxRow = k.iMaxRow; g.maxCol = k.iMaxCol;
        g.yBase = inArm ? iInArmYBase : iOutArmYBase;                           // golden :4335 / :4405
        g.xBase = inArm ? iInArmXBase : iOutArmXBase;
        g.pickerCount = USE_PICKER_COUNT;                                       // golden :4312 / :4381
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 8; ++j) g.zIndex[i][j] = inArm ? InArmZIndex[i][j] : OutArmZIndex[i][j];   // cmydef.cpp:3333 / :3335
        if (inArm) {
            const int ax[7] = { MInArmX, MInArmY, MInArmPitch, MInArmPitchY, MInArmPitchX2, MInArmPitchX3, MInArmPitchX4 };
            g.armAxes.assign(ax, ax + 7);
        } else {
            const int ax[7] = { MOutArmX, MOutArmY, MOutArmPitch, MOutArmPitchY, MOutArmPitchX2, MOutArmPitchX3, MOutArmPitchX4 };
            g.armAxes.assign(ax, ax + 7);
        }
        why.clear();
        return true;                                                            // the [2][8] bounds are checked by the caller (WebMotorAccess.cpp EOF)
    }
    int GoldenMotionDone(int mi) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR) return -1;
        if (MOT[mi].CardType == AnsiString("PCI1203")) return -1;               // the 1203 monitor's axis: golden's object cannot read it (caller asks the monitor)
        if (MOT[mi].Motor == 0 || !MOT[mi].Motor->Enable) return -1;            // golden VerifyMotorAction's own guard (uteach.cpp:5369-5370 / :5382-5384)
        //AI(W906-TEACH-ZDOWN) 20261003: decision 1 = B -- All Down asks every motor now, the Index ones too: golden's Galil branch for
        //  them (uteach.cpp:5365-5379; the port's VerifyMotorAction, csystem.cpp, has it in the same place, after the 1203 rows).
        if (INDEX_MOTION_CARD == 0 &&
            (MOT[mi].Mot_Name == MTestY1 || MOT[mi].Mot_Name == MTestZ1 || MOT[mi].Mot_Name == MTestZ2 || MOT[mi].Mot_Name == MTestY2)) {
            MOT[mi].ScanMotorStatus();
            return MOT[mi].Led[iInposLed] ? 0 : 1;                              // golden: Led[iInposLed] -> "*Lock by %s moveing"
        }
        return MOT[mi].Motor->MotionDone() ? 1 : 0;
    }
    int MotorCount() override { return TOTAL_MOTOR < MAX_TRAY_MOTOR ? TOTAL_MOTOR : MAX_TRAY_MOTOR; }   // golden VerifyMotorAction for(i<TOTAL_MOTOR) over MOT[MAX_TRAY_MOTOR]
};

ZdLiveOps g_zdLiveOps;                                                          // installs itself (wb_serve lists this TU directly)

}  // namespace
}  // namespace ht9045
