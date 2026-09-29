// ===========================================================================
//  tests/test_note_servooff.cpp
//
//  AI(W906-SHOWERR) 20260929: golden TfNote::FormShow's alarm servo-off (note.cpp:2035-2128),
//  TfNote::W906_FormShowServoOff in forms/fNote_ShowError.cpp (ht9045_sm); host tools/wb_serve.cpp W906ModalWaitScope(0).
//  No motor card here: MOT[].Motor is NULL, so ServoOnOff / ReadEncoderPos do not reach hardware
//  (Motor/mymotor.cpp:1341 / :276) and ReadEncoderPos returns MOT[].Position; InArmZSafe / OutArmZSafe see no Z axis
//  (-1 = safe).  For the Out Shuttle half MOT[MTestZ1/2] get a disabled HTMotor, which makes Gali_ReadEncoderInRandge
//  true (Motor/myGALILmotor.cpp:4316-4321).
//
//    [1] "Input Arm" -> In Arm off, positions = MOT[MInArmX/Y].Position; a second call keeps the first positions
//    [2] "Input Shuttle" / "Output Shuttle" / "Index Unit" -> In Arm off
//    [3] MOT[MInArmX] cannot move -> nothing
//    [4] "Output Arm" -> In Arm untouched; Out Arm off only once its half is live (kOutArmHalfLive)
//    [5] "Output Shuttle" -> In Arm off, and Out Arm off once its half is live
//    [6] IniConfig.bAlarmNeedServoOff false -> nothing
//    [7] E44 + JAM0508 / JAM0509 -> Out Shuttle 1 / 2 off only once that half is live (kOutShuttleHalfLive); E44 off -> nothing
//  RecordProcess writes the golden "... Servo Off, X=..." lines through the log seams (ctest's machine_log_scratch).
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include "forms/fNote.h"
#include "Motor/mymotor.h"
#include "Motor/HTMotor.h"
#include "common.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "w906_ctest_guard.h"

static const bool kOutArmHalfLive     = true;    // GATE(W906-SHOWERR) S2 in forms/fNote_ShowError.cpp (lands with csystem.cpp H2-G1)
static const bool kOutShuttleHalfLive = true;    // GATE(W906-SHOWERR) S3 in forms/fNote_ShowError.cpp (lands with csystem.cpp H2-G2 / H2-G3)

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_note_servooff.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static void Clear()
{
    fNote->bMyServoOffInArm = false;  fNote->bMyServoOffOutArm = false;
    fNote->bMyServoOffOutShuttle1 = false;  fNote->bMyServoOffOutShuttle2 = false;
    fNote->iMyServoOffInArmPosX = 0;  fNote->iMyServoOffInArmPosY = 0;
    fNote->iMyServoOffOutArmPosX = 0;  fNote->iMyServoOffOutArmPosY = 0;
    fNote->iMyServoOffOutShuttle1Pos = 0;  fNote->iMyServoOffOutShuttle2Pos = 0;
}
static void Unit(const char* unit, const char* code = "JAM0101")
{
    fNote->edUnitName->Text = AnsiString(unit);
    fNote->edErrorCode->Text = AnsiString(code);
}

int main()
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asProductionLogPath", asProductionLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NoteServoOff", rt))
        return 2;
    CHECK(fNote != 0);
    if (fNote == 0) { std::printf("FAIL: no fNote\n"); return 1; }
    const bool saveNeed = IniConfig.bAlarmNeedServoOff, saveE44 = IniConfig.bE44EnableLoseDeviceOutShuttleServoOff;
    IniConfig.bAlarmNeedServoOff = true;  IniConfig.bE44EnableLoseDeviceOutShuttleServoOff = false;
    MOT[MInArmX].Position = 1234;   MOT[MInArmY].Position = 5678;
    MOT[MOutArmX].Position = 4321;  MOT[MOutArmY].Position = 8765;
    MOT[MInShuttle1].Position = 111;  MOT[MInShuttle2].Position = 222;

    std::printf("[1] Input Arm\n");
    Clear();  Unit("Input Arm");
    fNote->W906_FormShowServoOff();
    CHECK(fNote->bMyServoOffInArm == true);
    CHECK(fNote->iMyServoOffInArmPosX == 1234);
    CHECK(fNote->iMyServoOffInArmPosY == 5678);
    CHECK(fNote->bMyServoOffOutArm == false);
    MOT[MInArmX].Position = 99;
    fNote->W906_FormShowServoOff();                                            // golden :2042 bMyServoOffInArm==false guard
    CHECK(fNote->iMyServoOffInArmPosX == 1234);
    MOT[MInArmX].Position = 1234;

    std::printf("[2] Input Shuttle / Output Shuttle / Index Unit\n");
    const char* const inUnits[] = { "Input Shuttle", "Output Shuttle", "Index Unit" };
    for (int i = 0; i < 3; i++) {
        Clear();  Unit(inUnits[i]);
        fNote->W906_FormShowServoOff();
        CHECK(fNote->bMyServoOffInArm == true);
    }

    std::printf("[3] In Arm X cannot move\n");
    Clear();  Unit("Input Arm");
    MOT[MInArmX].fCanMove = false;
    fNote->W906_FormShowServoOff();
    CHECK(fNote->bMyServoOffInArm == false);
    MOT[MInArmX].fCanMove = true;

    std::printf("[4] Output Arm\n");
    Clear();  Unit("Output Arm", "JAM0201");
    fNote->W906_FormShowServoOff();
    CHECK(fNote->bMyServoOffInArm == false);
    CHECK(fNote->bMyServoOffOutArm == kOutArmHalfLive);
    CHECK(fNote->iMyServoOffOutArmPosX == (kOutArmHalfLive ? 4321 : 0));
    CHECK(fNote->iMyServoOffOutArmPosY == (kOutArmHalfLive ? 8765 : 0));

    std::printf("[5] Output Shuttle -> both arms\n");
    Clear();  Unit("Output Shuttle", "JAM0501");
    fNote->W906_FormShowServoOff();
    CHECK(fNote->bMyServoOffInArm == true);
    CHECK(fNote->bMyServoOffOutArm == kOutArmHalfLive);

    std::printf("[6] bAlarmNeedServoOff false\n");
    IniConfig.bAlarmNeedServoOff = false;
    Clear();  Unit("Output Shuttle", "JAM0508");
    IniConfig.bE44EnableLoseDeviceOutShuttleServoOff = true;
    fNote->W906_FormShowServoOff();
    CHECK(fNote->bMyServoOffInArm == false);
    CHECK(fNote->bMyServoOffOutArm == false);
    CHECK(fNote->bMyServoOffOutShuttle1 == false);
    IniConfig.bAlarmNeedServoOff = true;  IniConfig.bE44EnableLoseDeviceOutShuttleServoOff = false;

    std::printf("[7] E44 JAM0508 / JAM0509\n");
    HTMotor z1, z2;  z1.Enable = false;  z2.Enable = false;
    HTMotor* const saveZ1 = MOT[MTestZ1].Motor;  HTMotor* const saveZ2 = MOT[MTestZ2].Motor;
    MOT[MTestZ1].Motor = &z1;  MOT[MTestZ2].Motor = &z2;
    CHECK(MOT[MTestZ1].Gali_ReadEncoderInRandge(Prod.TestZ1_Safe) == true);
    Clear();  Unit("Output Shuttle", "JAM0508");
    fNote->W906_FormShowServoOff();                                            // E44 off
    CHECK(fNote->bMyServoOffOutShuttle1 == false);
    IniConfig.bE44EnableLoseDeviceOutShuttleServoOff = true;
    Clear();  Unit("Output Shuttle", "JAM0508");
    fNote->W906_FormShowServoOff();
    CHECK(fNote->bMyServoOffOutShuttle1 == kOutShuttleHalfLive);
    CHECK(fNote->iMyServoOffOutShuttle1Pos == (kOutShuttleHalfLive ? 111 : 0));
    CHECK(fNote->bMyServoOffOutShuttle2 == false);
    Clear();  Unit("Output Shuttle", "JAM0509");
    fNote->W906_FormShowServoOff();
    CHECK(fNote->bMyServoOffOutShuttle2 == kOutShuttleHalfLive);
    CHECK(fNote->iMyServoOffOutShuttle2Pos == (kOutShuttleHalfLive ? 222 : 0));
    CHECK(fNote->bMyServoOffOutShuttle1 == false);
    MOT[MTestZ1].Motor = saveZ1;  MOT[MTestZ2].Motor = saveZ2;

    Clear();
    IniConfig.bAlarmNeedServoOff = saveNeed;  IniConfig.bE44EnableLoseDeviceOutShuttleServoOff = saveE44;
    std::printf("\n%d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
