// ===========================================================================
//  ui/native/NativeMotorTest.h
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.MotorTest as a native Win32 window -- v1 DISPLAY ONLY.  NOT in golden
//  as a file; golden is TfMotorTest (V912 uMotorTest.cpp / .dfm).  Inventory: docs/handoff/ST02_NATIVE_INVENTORY_20260929.md §1.
//
//  What it shows (golden control -> here):
//    per-motor rows (MotorTestClass, golden-visible motors in order)   -> the motor list (left grid); a click selects locally
//    pnlMotorAlias / edtCommandPos / pnlEncoderPos / lblRealSpeed /
//    edtHomeOffset / lblLoopCount / lblAvgTime / lblJogPTime /
//    lblJogNTime / btnMotorPower caption / pnlStop lock colour         -> the detail grid (Item / Value) of the selected motor
//    ALed1..10 (CW HOME CCW EMG ALM SCW SCCW SALM INP SVON)            -> the lamp grid (one row, ten lamps)
//    strngrdMotor (InitSpeed .. Range, UpdateMotorParameter :655-669) -> ten rows of the detail grid
//  Golden's command buttons are shown as WS_DISABLED BUTTONs and the window procedure has NO branch for them; only Exit
//  works, and it only closes this window (NOT golden FormClose).  Not done in v1 (blank on purpose): the Pressure tab
//  (Galil Gali_ReadPos is a stub), Light Scale, Motor Database.
//
//  ⚠ Display only, by construction (same as NativeMotorView.h): this file and NativeMotorTest.cpp include only <windows.h>,
//    STL and NativeHost.h / NativeGrid.h.  No machine code is reachable from the window: selecting a row is screen state
//    (golden lM00Click's MOT[i].SetSpeed(1) card write is NOT done), FormShow's Motor Power / Servo On is NOT run, the
//    lamps only read (golden UpdateMotorLed's stop-on-alarm is NOT here), Timer1's loop / home / SetSpeed is NOT run.
//    The data comes in from the wb_serve glue (NativeFormsWbServe.cpp, cached reads only) or from the ctest.
//  ⚠ Thread: the one that creates the window and pumps messages (the wb_serve main loop), as NativeHost.h.
// ===========================================================================
#ifndef W906_UI_NATIVE_NATIVEMOTORTEST_H
#define W906_UI_NATIVE_NATIVEMOTORTEST_H

#include <string>
#include <vector>

namespace w906native {

// one motor of golden MotorTestClass (the list shows the golden-visible ones, in MotorTestClass order)
struct MotorTestAxis {
    int           row;          // HSys.MotTable index
    std::string   alias, no;    // No "M07" -> MOT[7]
    std::string   cardModel;
    int           boardId, port;
    // ---- values (null = has* false / -1, never 0) ----
    bool          hasCmd;       int cmd;          // edtCommandPos: ReadPos (1203: monitor cmdPos)
    bool          hasEnc;       int enc;          // pnlEncoderPos: ReadEncoderPos (1203: monitor encPos)
    bool          hasSpeed;     int speed;        // lblRealSpeed: GetSpeed() (the engine's cached iSpeed, no card read)
    bool          hasHomeOffset; int homeOffset;  // edtHomeOffset: Motor->LastHomePos
    int           servoOn, alarm, busy, inPos;    // -1 = null
    int           homeFlag;                       // 0 / 1 / 2, -1 = null
    bool          ledKnown;     unsigned long motionIO;  unsigned state;   // ALed1..10 (same decode as MotorLedOn)
    int           loopJob, homeJob;               // -1 = null
    bool          hasLoopCount; unsigned long loopCount;   // lblLoopCount (dwLoopCount)
    // strngrdMotor (UpdateMotorParameter): engine memory only
    bool          hasParams;
    unsigned      initSpeed, jogHigh, jogLow, homeHigh, homeLow, range;
    int           softP, softN;
    double        acc, dec;
    std::string   quality, source, errText;       // "good" | "partial" | "nosource"; source; why there is no value
    MotorTestAxis()
        : row(-1), boardId(-1), port(-1), hasCmd(false), cmd(0), hasEnc(false), enc(0), hasSpeed(false), speed(0),
          hasHomeOffset(false), homeOffset(0), servoOn(-1), alarm(-1), busy(-1), inPos(-1), homeFlag(-1),
          ledKnown(false), motionIO(0), state(0), loopJob(-1), homeJob(-1), hasLoopCount(false), loopCount(0),
          hasParams(false), initSpeed(0), jogHigh(0), jogLow(0), homeHigh(0), homeLow(0), range(0), softP(0), softN(0),
          acc(0), dec(0) {}
};

// the whole page (golden labels that are one per form, not per motor).  known = false: no source hooked -> every row "—".
struct MotorTestPage {
    bool          known;
    std::string   selectedMotor;                  // C++'s ActiveIndex alias (the web page's selection); "" = none
    bool          hasJogP, hasJogN, hasAvg;
    int           jogP, jogN;                     // ms
    double        avg;
    bool          hasPower, relayOn, motorPowerState, powerPending;
    bool          hasLock, locked;
    std::string   lockText;
    MotorTestPage() : known(false), hasJogP(false), hasJogN(false), hasAvg(false), jogP(0), jogN(0), avg(0),
                      hasPower(false), relayOn(false), motorPowerState(false), powerPending(false),
                      hasLock(false), locked(false) {}
};

struct MotorTestSummary {
    std::string   buildConfig;
    std::string   provider;
    std::string   why;             // no axis has a value
    std::string   pageWhy;         // why the page-wide rows are blank
    bool          monitorOpen;
    int           monitorAxes;
    unsigned long pollCount;
    unsigned long keepaliveCalls;
    MotorTestSummary() : monitorOpen(false), monitorAxes(0), pollCount(0), keepaliveCalls(0) {}
};

// motor list columns (grid column index)
enum MotorTestListColumn { kMtColNo = 0, kMtColAlias, kMtColCmd, kMtColEnc, kMtColServo, kMtColAlarm, kMtColHome, kMtColCount };
// detail rows (the Item / Value grid; the value is column 1)
enum MotorTestDetailRow {
    kMtRowAlias = 0, kMtRowCmd, kMtRowEnc, kMtRowSpeed, kMtRowHomeOffset, kMtRowHomeFlag, kMtRowServo, kMtRowAlarm,
    kMtRowBusy, kMtRowInPos, kMtRowLoopCount, kMtRowLoopJob, kMtRowHomeJob,
    kMtRowInitSpeed, kMtRowJogHigh, kMtRowJogLow, kMtRowHomeHigh, kMtRowHomeLow, kMtRowSoftP, kMtRowSoftN,
    kMtRowAcc, kMtRowDec, kMtRowRange,
    kMtRowJogPTime, kMtRowJogNTime, kMtRowAvgTime, kMtRowPower, kMtRowLock, kMtRowSelectedWeb,
    kMtRowQuality, kMtRowSource, kMtRowWhy, kMtRowCount
};

bool MotorTestOpen(bool show);
void MotorTestClose();
bool MotorTestIsOpen();
void MotorTestUpdate(const std::vector<MotorTestAxis>& axes, const MotorTestPage& page, const MotorTestSummary& sum);

// ---- test probes ----
void*         MotorTestHwnd();
int           MotorTestListCount();
std::wstring  MotorTestListText(int item, int column);
std::wstring  MotorTestDetailText(int row);              // the Value column of a detail row
std::wstring  MotorTestLampText(int led);                // "1" / "0" / "—"
int           MotorTestSelected();                       // index into the axes vector; -1 = none
void          MotorTestSelect(int listRow);              // as a click on that list row (screen state only)
int           MotorTestButtonCount();                    // every BUTTON child
int           MotorTestEnabledButtonCount();             // enabled BUTTON children (v1: only Exit)
void*         MotorTestListGrid();
void*         MotorTestDetailGrid();
void*         MotorTestLampGrid();
std::wstring  MotorTestSummaryText();
double        MotorTestLastUpdateMs();

}  // namespace w906native

#endif  // W906_UI_NATIVE_NATIVEMOTORTEST_H
