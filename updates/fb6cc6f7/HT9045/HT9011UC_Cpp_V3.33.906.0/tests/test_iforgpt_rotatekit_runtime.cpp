//AI(W906-IFORGPT-IG6) 20261009: Drive the real in/out RotateKit motor SMs.
// Only the HAL is simulated, as in RotateKitRetry. No source-text checks,
// process replacements, worker threads, config loaders or hardware drivers.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "LastSet.h"
#include "Config.h"
#include "mykitsuck.h"
#include "Motor/mymotor.h"
#include "RotateKit/aRotateKIT.h"
#include "RotateKit/aRotateKIT_In.h"
#include "RotateKit/aRotateKIT_Out.h"
#include "forms/fRotate.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <cstdio>

#ifdef SOFT_SIMULTE
extern int InAngle45, InAngle90, OutAngle45, OutAngle90;
static int checks = 0, failures = 0;
static const char* scenario = "fixture";
static void check(bool ok, const char* expression, int line)
{
    ++checks;
    if (!ok)
    {
        ++failures;
        std::printf("FAIL [%s] line %d: %s\n", scenario, line, expression);
    }
}
#define CHECK(c) check((c), #c, __LINE__)

static bool doorClosed()
{
    return false;
}

class ObservedSimMotor : public TMySimMotor
{
public:
    int moves, target;
    bool zSafeHome;
    ObservedSimMotor() : moves(0), target(0), zSafeHome(false) {}
    virtual bool MoveToPos(int value)
    {
        ++moves;
        target = value;
        return TMySimMotor::MoveToPos(value);
    }
    virtual void ScanMotorStatus(bool* leds)
    {
        TMySimMotor::ScanMotorStatus(leds);
        if (leds && zSafeHome)
        {
            // Boot's Z-safe sensor window, the same fixture as RotateKitRetry.
            leds[iHomeLed] = true;
        }
    }
    void clearObservation()
    {
        moves = 0;
        target = 0;
    }
};
static ObservedSimMotor* observed[TOTAL_MOTOR];

static void fixture()
{
    W906_TestEnsureSuckList();
    for (int axis = 0; axis < TOTAL_MOTOR; ++axis)
    {
        observed[axis] = new ObservedSimMotor;
        MOT[axis].Motor = observed[axis];
        MOT[axis].Mot_Name = axis;
        observed[axis]->zSafeHome = (axis == MInArmZA || axis == MOutArmZA);
        observed[axis]->MotorIdleSafeDoorCheck = &doorClosed;
        observed[axis]->PSoftLimitP = 1000000;
        observed[axis]->PSoftLimitN = -1000000;
        observed[axis]->GearRatio = 1;
    }
    InArmSuck.iMotRow = OutArmSuck.iMotRow = 1;
    InArmSuck.iMotCol = OutArmSuck.iMotCol = 1;
    InArmSuck.iPickRow = OutArmSuck.iPickRow = 1;
    InArmSuck.iPickCol = OutArmSuck.iPickCol = 1;
    InArmSuck.iMaxRow = OutArmSuck.iMaxRow = 2;
    //AI(W906-IFORGPT-IG6) 20261009: XY helpers require boot-created offsets.
    // Static storage gives the fixture zero offsets without loading machine files.
    static ARM_OFFSET inRotateOffset;
    static ARM_OFFSET outRotateOffset;
    InArmOffSet[InOfsRotate_In] = &inRotateOffset;
    OutArmOffSet[OutOfsRotate_Out] = &outRotateOffset;
    IniConfig.bE34InOutArmPitchZOffsetSameOne = false;
    InArmSuck.iMaxCol = OutArmSuck.iMaxCol = 4;
    InArmSuck.Suck[0][0].iMotNo = MInArmZA;
    OutArmSuck.Suck[0][0].iMotNo = MOutArmZA;
    Prod.ZInArmSafe[0][0] = Prod.ZOutArmSafe[0][0] = 1500;
    observed[MInArmZA]->SetPosition(1500);
    observed[MOutArmZA]->SetPosition(1500);
    for (int row = 0; row < MAX_ARM_Row; ++row)
    {
        for (int col = 0; col < MAX_ARM_Col; ++col)
        {
            InArmSuck.Item[row][col] = OutArmSuck.Item[row][col] = NULL_IC;
            InArmSuck.bLed[row][col] = OutArmSuck.bLed[row][col] = false;
            MOT[MInRotateKit].Tray.Data[col][row] = NULL_IC;
            MOT[MOutRotateKit].Tray.Data[col][row] = NULL_IC;
        }
    }
    if (!MOT[MOutRotateKit].Tray.PordRec[0][0])
    {
        MOT[MOutRotateKit].Tray.PordRec[0][0] = new TMyProductionRecord;
    }
    USE_ROTATE_KIT = 1;
    USE_PICKER_COUNT = ep8Picker;
    InOutArmPickerUseMotor = eptUseMot;
    iInArmType = e9045_2x2_4_14;
    iInArmXBase = iInArmYBase = iOutArmXBase = iOutArmYBase = 0;
    bContinusRotate = false;
    bRunAutoSiteMapping = false;
    bAutoSiteMapHotplateSave = false;
    IniConfig.bAlarmNeedServoOff = false;
    IniConfig.bA27EnableLightScale = false;
    IniConfig.bA22MagneticScale = false;
    USE_IN_Y_IS_AUTO_PITCH = USE_OUT_Y_IS_AUTO_PITCH = false;
    TestIF.iARM_Y_PITCH = 0;
    Prod.iInArm_RotateX = 20000;
    Prod.iInArm_RotateY = 30000;
    Prod.iOutArm_RotateX = 40000;
    Prod.iOutArm_RotateY = 50000;
    Prod.iIn_iRotateA = 111;
    Prod.iOut_iRotateA = 222;
    Prod.iIn_iRotateA_Backlash = Prod.iOut_iRotateA_Backlash = 0;
}

// Initialize the function-local cursor/kit from its actual entry, rather than
// assuming that the previous scenario left a particular static value.
static void initialize(int dut, double pitchX, double pitchY)
{
    tRotate.DutNum = dut;
    tRotate.RotateKit_PitchX = pitchX;
    tRotate.RotateKit_PitchY = pitchY;
    tRotate.bUseDifferentAngle = false;
    tRotate.bPassBinNoRotate = false;
    FrmRotate->bShowRotateBySite = false;
    iRotate_Type = eInOutArm1Motor;
    iRotato_In_Row = iRotato_Out_Row = 0;
    TestIF.iTestMode = TestIF_File.iTestMode = _8Site2X4;
    DeviceForm.XDimension = 1000;
    TestIF_File.dSiteXPitch = 0;
    iInArmRotateKit = 1;
    M_DoInArmRotateKIT_Motor();
    iOutArmRotateKit = 1;
    M_DoOutArmRotateKIT_Motor();
}

static void xyCase(const char* label, int dut, double pitchX, double pitchY,
                   bool largeDevice, int expectedInX, int expectedOutX,
                   int expectedInY, int expectedOutY)
{
    scenario = label;
    initialize(dut, pitchX, pitchY);
    CHECK(iRotateKIT_Pitch_X_H == static_cast<int>(pitchX*100));
    CHECK(iRotateKIT_Pitch_Y_H == static_cast<int>(pitchY*100));
    iRotato_In_Row = iRotato_Out_Row = 1;
    DeviceForm.XDimension = largeDevice ? ArmMaxPitch : 1000;
    bIn_ICRotationCompleteOnKit = bOut_ICRotationCompleteOnKit = false;
    //AI(W906-IFORGPT-IG6) 20261009: Each case must issue its own XY commands.
    observed[MInArmX]->SetPosition(-9000);
    observed[MInArmY]->SetPosition(-9000);
    observed[MOutArmX]->SetPosition(-9000);
    observed[MOutArmY]->SetPosition(-9000);
    observed[MInArmX]->clearObservation();
    observed[MInArmY]->clearObservation();
    observed[MOutArmX]->clearObservation();
    observed[MOutArmY]->clearObservation();
    iInArmRotateKit = iOutArmRotateKit = 4000;
    // Bound the actual multi-tick Z/pitch/XY flow; failed convergence is a
    // fixture/test failure, never a reason to relax a product interlock.
    for (int tick = 0; tick < 64 && iInArmRotateKit == 4000; ++tick)
    {
        M_DoInArmRotateKIT_Motor();
        Sleep(5);
    }
    for (int tick = 0; tick < 64 && iOutArmRotateKit == 4000; ++tick)
    {
        M_DoOutArmRotateKIT_Motor();
        Sleep(5);
    }
    CHECK(iInArmRotateKit != 4000);
    CHECK(iOutArmRotateKit != 4000);
    CHECK(observed[MInArmX]->moves > 0 && observed[MInArmX]->target == expectedInX);
    CHECK(observed[MInArmY]->moves > 0 && observed[MInArmY]->target == expectedInY);
    CHECK(observed[MOutArmX]->moves > 0 && observed[MOutArmX]->target == expectedOutX);
    CHECK(observed[MOutArmY]->moves > 0 && observed[MOutArmY]->target == expectedOutY);
    CHECK(MOT[MInArmX].ReadPos() == expectedInX && MOT[MInArmY].ReadPos() == expectedInY);
    CHECK(MOT[MOutArmX].ReadPos() == expectedOutX && MOT[MOutArmY].ReadPos() == expectedOutY);
}

static void routeCases()
{
    scenario = "Dut8 routes through 2550";
    initialize(tDutType_8, 40, 60);
    Prod.RotationTimeIn = Prod.RotationTimeOut = 1;
    Prod.RotationCount[0] = Prod.OutRotationCount[0] = 90;
    iInArmRotateKit = iOutArmRotateKit = 2510;
    M_DoInArmRotateKIT_Motor();
    M_DoOutArmRotateKIT_Motor();
    CHECK(iInArmRotateKit == 2550);
    CHECK(iOutArmRotateKit == 2550);
    scenario = "per-site bypasses 2550";
    FrmRotate->bShowRotateBySite = true;
    iInArmRotateKit = iOutArmRotateKit = 2510;
    M_DoInArmRotateKIT_Motor();
    M_DoOutArmRotateKIT_Motor();
    CHECK(iInArmRotateKit == 2600);
    CHECK(iOutArmRotateKit == 2600);
}

static void offsetCase(int degrees, bool offsetEnabled)
{
    scenario = offsetEnabled ? "in/out one-motor offset" : "other motor has no offset";
    initialize(tDutType_4, 40, 60);
    iRotate_Type = offsetEnabled ? eInOutArm1Motor : e1MotRotate;
    SetMotorResolution(InAngle45, InAngle90, true);
    SetMotorResolution(OutAngle45, OutAngle90, false);
    Prod.RotationTimeIn = Prod.RotationTimeOut = 1;
    Prod.RotationCount[0] = Prod.OutRotationCount[0] = degrees;
    tRotate.iRotateOffset[0] = 137;
    tRotate.iRotateOffset[1] = -249;
    observed[MInRotateKit]->SetPosition(-9000);
    observed[MOutRotateKit]->SetPosition(-9000);
    observed[MInRotateKit]->clearObservation();
    observed[MOutRotateKit]->clearObservation();
    iInArmRotateKit = iOutArmRotateKit = 2510;
    M_DoInArmRotateKIT_Motor();
    M_DoOutArmRotateKIT_Motor();
    int quarter = degrees/90;
    if (quarter == 3)
    {
        quarter = -1;
    }
    else if (quarter == -3)
    {
        quarter = 1;
    }
    const int pulses = offsetEnabled ? 1250 : 2000;
    const int inGoal = quarter*pulses+111+(offsetEnabled && quarter != 0 ? 137 : 0);
    const int outGoal = quarter*pulses+222+(offsetEnabled && quarter != 0 ? -249 : 0);
    for (int tick = 0; tick < 8 && iInArmRotateKit == 2600; ++tick)
    {
        M_DoInArmRotateKIT_Motor();
    }
    for (int tick = 0; tick < 8 && iOutArmRotateKit == 2600; ++tick)
    {
        M_DoOutArmRotateKIT_Motor();
    }
    CHECK(observed[MInRotateKit]->moves > 0 && observed[MInRotateKit]->target == inGoal);
    CHECK(observed[MOutRotateKit]->moves > 0 && observed[MOutRotateKit]->target == outGoal);
    CHECK(MOT[MInRotateKit].ReadPos() == inGoal);
    CHECK(MOT[MOutRotateKit].ReadPos() == outGoal);
}

static void rotationCountCase(bool match)
{
    scenario = match ? "in per-site RotationCount match" : "in per-site RotationCount no match";
    initialize(tDutType_8, 40, 60);
    FrmRotate->bShowRotateBySite = true;
    iRotate_Type = e1MotRotate;
    SetMotorResolution(InAngle45, InAngle90, true);
    Prod.RotationTimeIn = 2;
    Prod.RotationCount[1] = 90;
    tRotate.RotationCount[1] = 90;
    tRotate.RotateDutDate[0][0][3] = match ? 90 : 180;
    MOT[MInRotateKit].Tray.Data[3][0] = HAS_IC;
    observed[MInRotateKit]->SetPosition(-9000);
    observed[MInRotateKit]->clearObservation();
    iInArmRotateKit = 2510;
    M_DoInArmRotateKIT_Motor();
    CHECK(iInArmRotateKit == 2600);
    M_DoInArmRotateKIT_Motor();
    if (match)
    {
        CHECK(observed[MInRotateKit]->moves > 0 && observed[MInRotateKit]->target == 2111);
    }
    else
    {
        CHECK(observed[MInRotateKit]->moves == 0);
        CHECK(MOT[MInRotateKit].ReadPos() == -9000);
    }
    MOT[MInRotateKit].Tray.Data[3][0] = NULL_IC;
}

static void differentAngleCase(bool different)
{
    scenario = different ? "out different angle selects +90" : "out common angle selects -90";
    initialize(tDutType_4, 40, 60);
    tRotate.bUseDifferentAngle = different;
    tRotate.iRotateOffset[1] = -249;
    Prod.RotationTimeOut = 4;
    Prod.RotateDutDate[0][0][1] = 90;
    Prod.RotateDutDate[1][0][1] = 180;
    Prod.OutRotationCount[3] = -90;
    Prod.OutRotationCount[2] = 90;
    observed[MOutRotateKit]->SetPosition(-9000);
    observed[MOutRotateKit]->clearObservation();
    iOutArmRotateKit = 2550;
    M_DoOutArmRotateKIT_Motor();
    CHECK(iOutArmRotateKit == 2600);
    const int goal = (different ? 1250 : -1250)+222-249;
    for (int tick = 0; tick < 8 && iOutArmRotateKit == 2600; ++tick)
    {
        M_DoOutArmRotateKIT_Motor();
    }
    CHECK(observed[MOutRotateKit]->moves > 0 && observed[MOutRotateKit]->target == goal);
    CHECK(MOT[MOutRotateKit].ReadPos() == goal);
}

static void passBinCase(bool feature, bool configured, bool passed)
{
    scenario = "out pass-bin transfer condition";
    initialize(tDutType_4, 40, 60);
    iRotate_Type = e1MotRotate1Dut;
    OutArmSuck.iMaxCol = OutArmSuck.iMaxRow = 1;
    CosFunction.bPassBinNoRotate = feature;
    tRotate.bPassBinNoRotate = configured;
    OutArmSuck.bPass[0][0] = passed;
    OutArmSuck.Item[0][0] = HAS_NULL_IC; // golden virtual filler: no vacuum I/O
    MOT[MOutRotateKit].Tray.Data[0][0] = NULL_IC;
    iOutArmRotateKit = 1300;
    M_DoOutArmRotateKIT_Motor();
    const bool skipped = feature && configured && passed;
    CHECK(OutArmSuck.Item[0][0] == (skipped ? HAS_NULL_IC : NULL_IC));
    CHECK(MOT[MOutRotateKit].Tray.Data[0][0] == (skipped ? NULL_IC : HAS_NULL_IC));
    CHECK(iOutArmRotateKit == 2000);
    OutArmSuck.Item[0][0] = NULL_IC;
    OutArmSuck.iMaxCol = 4;
    OutArmSuck.iMaxRow = 2;
}
#endif

int main()
{
#ifndef SOFT_SIMULTE
    std::puts("SKIP: IG-6 is a SOFT_SIMULTE motor-flow test; SHIP does not execute it.");
    return 77;
#else
    if (!W906TestRequireCtestRedirects("IforGPT_RotateKitRuntime"))
    {
        return 2;
    }
    std::setvbuf(stdout, 0, _IONBF, 0);
    fixture();
    xyCase("Dut8 fractional taught pitch", tDutType_8, 41.25, 60.5, false,
           24125, 44125, 36050, 56050);
    xyCase("Dut4 large IC pitch40", tDutType_4, 40, 60, true,
           17334, 37334, 30000, 50000);
    xyCase("Dut4 large IC pitch80", tDutType_4, 80, 60, true,
           17334, 37334, 30000, 50000);
    xyCase("Dut4 small IC", tDutType_4, 40, 60, false,
           20000, 40000, 30000, 50000);
    routeCases();
    const int degrees[] = {-270, -180, -90, 0, 90, 180, 270};
    for (unsigned i = 0; i < sizeof(degrees)/sizeof(degrees[0]); ++i)
    {
        offsetCase(degrees[i], true);
    }
    offsetCase(90, false);
    rotationCountCase(true);
    rotationCountCase(false);
    differentAngleCase(true);
    differentAngleCase(false);
    passBinCase(true, true, true);
    passBinCase(false, true, true);
    passBinCase(true, false, true);
    passBinCase(true, true, false);
    std::printf("IforGPT_RotateKitRuntime: %d/%d checks passed; %d failed\n",
                checks-failures, checks, failures);
    return failures ? 1 : 0;
#endif
}
