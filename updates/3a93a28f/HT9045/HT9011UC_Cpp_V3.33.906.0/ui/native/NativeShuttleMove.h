// ===========================================================================
//  ui/native/NativeShuttleMove.h
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove as a native Win32 window -- v1 DISPLAY ONLY.  NOT in golden
//  as a file; golden is TfShuttleMove (V912 ShuttleMove.cpp / .dfm).  Inventory: docs/handoff/ST02_NATIVE_INVENTORY_20260929.md §4.
//
//  What it shows (golden -> here):
//    palSh1Encoder / palSh2Encoder (FormShow :137-138)       -> the encoder grid (1203: the monitor's encPos, never a card read)
//    the 8 teach edits (FormShow :100-107, Tech.*) and the
//    4 latch edits (:109-116, only with the auto latch)      -> the point grid (value, group, "locked" when SPIL :118-135)
//    the 4 TTMyTray grids (ShowShuttleSensorPosition :2001-2090) -> the tray grid: LAYOUT only (XItem x YItem); the cell numbers
//                                                               are not shown in v1, because golden fills them right after
//                                                               SetTechDataToProd(), which WRITES Prod -- not a display path
//    FormShow's enable / visible rules (:72-98, :140)        -> the flag grid, and which command buttons are shown
//    meShuttleMaintain                                        -> one line: the sequences (:161-1784) are not translated
//  Golden's command buttons are shown (golden-visible ones only) as WS_DISABLED BUTTONs; the window procedure has NO branch
//  for them.  Only Exit works, and it only closes this window -- NOT golden sbtExitClick / FormClose (:1956 / :2132), which
//  clear fAllMotorHome, set SystemStart=false and pause EtherCAT.
//
//  ⚠ Display only by construction: this file and NativeShuttleMove.cpp include only <windows.h>, STL and NativeHost.h /
//    NativeGrid.h.  Data comes from the wb_serve glue (NativeShuttleMoveGlue.cpp, memory reads only) or the ctest.
//  ⚠ Thread: the one that creates the window and pumps messages (the wb_serve main loop), as NativeHost.h.
// ===========================================================================
#ifndef W906_UI_NATIVE_NATIVESHUTTLEMOVE_H
#define W906_UI_NATIVE_NATIVESHUTTLEMOVE_H

#include <string>
#include <vector>

namespace w906native {

struct ShuttleEncoder {
    std::string alias;        // MOT[MInShuttle1/2].Alias
    bool        has;          // false = no value ("—", never 0)
    int         enc;
    std::string source, why;
    ShuttleEncoder() : has(false), enc(0) {}
};

// one teach edit (golden TEdit name)
struct ShuttlePoint {
    std::string name;         // "edInSH1Sen7DetectPos" ...
    std::string group;        // golden group box caption ("In Fiber Check" ...)
    std::string field;        // "Tech.iInSH1Sen7DetectPos" / "Prod.iInSH1SenICAddPos"
    bool        visible;      // golden shows this edit (its group is visible)
    bool        locked;       // golden Enabled=false (SPIL)
    int         value;
    ShuttlePoint() : visible(true), locked(false), value(0) {}
};

struct ShuttleTray {
    std::string name;         // "mtedInSHSen7DetectPos" ...
    int         xItem, yItem;
    bool        visible;
    ShuttleTray() : xItem(0), yItem(0), visible(true) {}
};

enum ShuttleButtonVis {       // which golden rule decides a command button's Visible
    kSmVisAlways = 0, kSmVisBarCode, kSmVisLatch, kSmVisRetry, kSmVisSensor, kSmVisSensorLatch, kSmVisTStep, kSmVisNever
};

struct ShuttleMoveState {
    ShuttleEncoder              enc[2];
    std::vector<ShuttlePoint>   points;
    std::vector<ShuttleTray>    trays;
    // golden FormShow rules
    bool        barCodeVisible;       // gbBarCode: BAR_CODE_INSTALL != ebctUninstall (:88)
    bool        latchVisible;         // gbInFiberCheckShtSnLct: In_Shuttle_Auto_Latch == eInSHAutoLtc (:109)
    bool        retryVisible;         // btRetry: SIM build only (:94-98)
    bool        sensorVisible;        // sbShuttleSensor: SHUTTLE_SENSOR_TYPE CC-Link / CanBus / EtherCAT (:89-91)
    bool        sensorLatchVisible;   // sbSensorLatch: ENABLE_OUT_SHUTTLEY_LATCH (:140)
    bool        tStepVisible;         // btnTStep: SIM and a diagonal multi-2D barcode (:2059-2078)
    bool        spilLocked;           // IniConfig.bSPILFunction: the teach edits are locked (:118-135)
    int         oneSide;              // -1 both shuttles; 0 / 1 = TestIF one-side on shuttle 1 / 2 (:77-86: the other side's buttons off)
    std::string buildConfig, why;
    bool        monitorOpen;
    unsigned long keepaliveCalls;
    ShuttleMoveState() : barCodeVisible(true), latchVisible(false), retryVisible(false), sensorVisible(false),
                         sensorLatchVisible(false), tStepVisible(false), spilLocked(false), oneSide(-1),
                         monitorOpen(false), keepaliveCalls(0) {}
};

enum ShuttleEncColumn   { kSmEncName = 0, kSmEncAlias, kSmEncValue, kSmEncSource, kSmEncCount };
enum ShuttlePointColumn { kSmPtGroup = 0, kSmPtName, kSmPtValue, kSmPtField, kSmPtState, kSmPtCount };
enum ShuttleTrayColumn  { kSmTrName = 0, kSmTrSize, kSmTrLayout, kSmTrCount };
enum ShuttleFlagRow {
    kSmFlBarCode = 0, kSmFlLatch, kSmFlRetry, kSmFlSensor, kSmFlSensorLatch, kSmFlTStep, kSmFlSpil, kSmFlOneSide, kSmFlLog,
    kSmFlCount
};

bool ShuttleMoveOpen(bool show);
void ShuttleMoveClose();
bool ShuttleMoveIsOpen();
void ShuttleMoveUpdate(const ShuttleMoveState& s);

// ---- test probes ----
void*         ShuttleMoveHwnd();
std::wstring  ShuttleMoveEncText(int row, int col);
int           ShuttleMovePointCount();              // visible points (rows)
std::wstring  ShuttleMovePointText(int row, int col);
int           ShuttleMoveTrayCount();               // visible trays (rows)
std::wstring  ShuttleMoveTrayText(int row, int col);
std::wstring  ShuttleMoveFlagText(int row);         // the value column
int           ShuttleMoveButtonCount();             // every BUTTON child
int           ShuttleMoveVisibleButtonCount();      // visible BUTTON children
int           ShuttleMoveEnabledButtonCount();      // enabled BUTTON children (v1: only Exit)
bool          ShuttleMoveButtonVisible(const wchar_t* caption);
void*         ShuttleMoveGrid(int which);           // 0 encoders, 1 points, 2 trays, 3 flags
std::wstring  ShuttleMoveSummaryText();
double        ShuttleMoveLastUpdateMs();

}  // namespace w906native

#endif  // W906_UI_NATIVE_NATIVESHUTTLEMOVE_H
