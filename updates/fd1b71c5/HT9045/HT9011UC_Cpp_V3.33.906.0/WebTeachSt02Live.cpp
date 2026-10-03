// =============================================================================
//  WebTeachSt02Live.cpp  --  the live ITeachSt02Ops (WebTeachSt02.h): golden's globals behind the St02 teach buttons
//
//  AI(W906-ST02-C9-G2) 20261002 (St02-E helper).  Linked into wb_serve ONLY (root CMakeLists.txt, next to
//  WebMotorAccessLive.cpp); it installs itself at static initialisation (WebMotorAccess.cpp EOF holds the pointer, a
//  constant-initialised 0, so the order of the two TUs does not matter).  Every other binary links no ops and every
//  teachSt02 request there is refused with "not installed".
//
//  Read only, except the two golden memory writes the buttons make on the golden motor object (MOT[i].fCMD=false and
//  MOT[i].InitMOTParameter(), golden uteach.cpp:4637 / :4731).  No file, no card, no IO here: the moves go through
//  WebMotorAccess.cpp (Move1203 = EastSun's 1203 layer, MoveGolden = the golden object).
//  AI(W906-ST02-C9) 20261002 (St02-E helper): St02's group G3 (lanes / Galil servo / TTL: io.btnPanelClick, Gali_Command, fMain's
//    TTL members) was dropped in the C9 rebase onto main 2dd90ef3 -- the machine wired those buttons on main first.
// =============================================================================
#include "WebTeachSt02.h"

#include <cstdio>

#include "vclcompat/vcl_compat.h"
#include "cmydef.h"           // USE_ROTATE_KIT / iRotate_Type / USE_PICKER_COUNT, MInRotateKit.. / MInShuttle1 / 2
#include "cprod.h"            // Prod.ZInArm_Tray_Pick / Prod.ZOutArm_Auto_Place
#include "LastSet.h"          // Tech.iInShuttle1Right / iInShuttle2Right
#include "MachineType.h"      // eRotateType, ePickCount, MAX_ARM_Row / MAX_ARM_Col, MAX_AUTO_TRAY
#include "Motor/mymotor.h"    // MOT[MAX_TRAY_MOTOR]
#include "mykitsuck.h"        // InArmSuck / OutArmSuck (the linked TMyKitSuck, golden MyKitSuck.h layout since A4-6)

namespace ht9045 {
namespace {

// the header's copies of golden's enums must be golden's values (int casts: two different enum types, -Wenum-compare)
static_assert((int)e1MotRotate == (int)kSt02Rot1Mot && (int)e4MotRotate == (int)kSt02Rot4Mot && (int)e8MotRotate == (int)kSt02Rot8Mot &&
              (int)e1MotRotate1Dut == (int)kSt02Rot1Mot1Dut && (int)e2MotRotate2Dut == (int)kSt02Rot2Mot2Dut &&
              (int)eInOutArm1Motor == (int)kSt02RotInOutArm1Motor && (int)eCynRotate == (int)kSt02RotCyn,
              "WebTeachSt02.h kSt02Rot* != golden eRotateType (MachineType.h)");
static_assert((int)ep16Picker == (int)kSt02Picker16, "WebTeachSt02.h kSt02Picker16 != golden ep16Picker (MachineType.h)");

class St02LiveOps : public ITeachSt02Ops {
public:
    St02LiveOps() { TeachSt02InstallOps(this); }

    TeachSt02Config Config() override
    {
        TeachSt02Config c;
        c.useRotateKit        = USE_ROTATE_KIT;                                 // golden FormShow :1652 reads the same globals
        c.rotateType          = iRotate_Type;
        c.usePickerCount      = USE_PICKER_COUNT;                               // golden FormShow :1457-1458
        c.shtSensorGroupShown = false;                                          // golden uteach.dfm:5850 Visible = False; nothing sets it true
        return c;
    }
    int RotateMotorKind(int mi) override                                        // golden SpeedButtonInRotatePos90Click :4628-4635
    {
        if (mi < 0) return kSt02RotNone;
        if (mi == MInRotateKit)  return kSt02RotInKit;
        if (mi == MOutRotateKit) return kSt02RotOutKit;
        if (mi == MInRotateB  || mi == MInRotateC  || mi == MInRotateD  || mi == MInRotateE  ||
            mi == MInRotateF  || mi == MInRotateG  || mi == MInRotateH  ||
            mi == MOutRotateB || mi == MOutRotateC || mi == MOutRotateD || mi == MOutRotateE ||
            mi == MOutRotateF || mi == MOutRotateG || mi == MOutRotateH) return kSt02RotOther;
        return kSt02RotNone;
    }
    int GoldenMotorIndex(int which) override
    {
        if (which == kSt02MotInShuttle1) return MInShuttle1;
        if (which == kSt02MotInShuttle2) return MInShuttle2;
        return -1;
    }
    void GoldenClearFCmd(int mi) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR) return;
        MOT[mi].fCMD = false;                                                   // golden uteach.cpp:4637 / :4689
    }
    void GoldenInitMOTParameter(int mi) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR) return;
        MOT[mi].InitMOTParameter();                                             // golden uteach.cpp:4731 / :4747
    }
    int TechInShuttleRight(int which) override
    {
        return (which == kSt02MotInShuttle2) ? Tech.iInShuttle2Right : Tech.iInShuttle1Right;   // golden :4733 / :4749
    }
    TeachSt02ArmZGrid ArmZGrid(bool inArm) override                             // golden btnSetAll*ArmZ_MoveClick :5952-5965 / :5977-5990
    {
        const TMyKitSuck& k = inArm ? InArmSuck : OutArmSuck;
        TeachSt02ArmZGrid g;
        g.motRow = k.iMotRow; g.motCol = k.iMotCol; g.pickRow = k.iPickRow; g.pickCol = k.iPickCol;
        int rows = g.motRow, cols = g.motCol;
        if (rows > _MAX_SUCK_ROW_ITEM || cols > _MAX_SUCK_COL_ITEM) {           // golden would index Suck[][] past its size
            char b[200];
            std::snprintf(b, sizeof(b), "iMotRow x iMotCol = %d x %d is larger than Suck[%d][%d]; the loop stopped at the array",
                          g.motRow, g.motCol, _MAX_SUCK_ROW_ITEM, _MAX_SUCK_COL_ITEM);
            g.note = b;
            if (rows > _MAX_SUCK_ROW_ITEM) rows = _MAX_SUCK_ROW_ITEM;
            if (cols > _MAX_SUCK_COL_ITEM) cols = _MAX_SUCK_COL_ITEM;
        }
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j) {
                TeachSt02ArmZCell c;
                c.row = i; c.col = j;
                c.motIndex = k.Suck[i][j].iMotNo;
                c.inPick = (i < k.iPickRow && j < k.iPickCol);
                c.targetKnown = (i < MAX_ARM_Row && j < MAX_ARM_Col);
                if (c.targetKnown) c.target = inArm ? Prod.ZInArm_Tray_Pick[i][j] : Prod.ZOutArm_Auto_Place[0][i][j];
                g.cells.push_back(c);
            }
        return g;
    }
};

St02LiveOps g_st02LiveOps;                                                      // installs itself (wb_serve lists this TU directly)

}  // namespace
}  // namespace ht9045
