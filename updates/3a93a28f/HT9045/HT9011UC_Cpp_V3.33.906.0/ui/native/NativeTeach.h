// ===========================================================================
//  ui/native/NativeTeach.h
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.teach as a native Win32 window -- a TABLE, v1 DISPLAY ONLY.
//  NOT in golden as a file; golden is TfTeach (V912 uteach.cpp / .h / .dfm).  Inventory: St02-E2
//  ST02_NATIVE_INVENTORY_20260929.md §2 and ST02_TEACH_TWOPARA_GAP_20260929.md.
//
//  What it shows (golden -> here):
//    TfTeach::TechPara    (TECH_PARA,    uteach.h:19-31)  -> one row per entry ("P" rows), registry order
//    TfTeach::TechTwoPara (TECH_TWOPARA, uteach.h:47-59)  -> one row per entry ("T" rows) after the P rows, both axes
//      per row: golden tab (the TTabSheet that holds the row's edit) | group | Set-button caption | Key (= the edit
//      name = the teach.ini key) | motor (MOT[MotorSelect].Alias) | Tech value (*Parameter, what golden ReadFromFile
//      copies into SetEdit->Text) | the motor's current position | Set / Go (DRAWN disabled buttons, kGridButtonOff) |
//      the second axis of a T row | the teach.ini [section] Key golden ReadFromFile reads (forms/fTeachPara.cpp:96-138)
//    pnlMotion (the selected row's axis): Panel2 (NumberAlias), edtNowPosition, pnlEncoderPos, ALed1..10,
//      edtSetToOffset (LastHomePos), labLock -> the detail grid + the lamp grid
//  The tab / group / caption of a Key comes from a table generated from the dfm2rc IR of golden uteach.dfm (see
//  NativeTeach.cpp, kKeyInfo); golden FormShow's tab / panel visibility (about 660 lines, [DEP] forms/fTeach.h:432)
//  is NOT applied in v1: every row is listed, and the note line says so.
//
//  ⚠ Display only, by construction (same as NativeMotorTest.h): this file and NativeTeach.cpp include only <windows.h>,
//    STL and NativeHost.h / NativeGrid.h / NativeMotorView.h (MotorLedOn).  No machine code is reachable from the window:
//    selecting a row is screen state (golden MotorTrayXClick -> UpdateMotorTeachMonitor's MOT[i].SetSpeed(1) is NOT
//    done), Set / Go are drawn (no HWND, no WM_COMMAND), the motion-panel buttons are WS_DISABLED with no WM_COMMAND
//    branch, Timer1's DoZHome / ProcessSingleMotorHome / DoPitch_Home are NOT run, Exit only destroys this window
//    (NOT golden pnlExitClick / FormClose).  The data comes from the wb_serve glue (NativeFormsWbServe.cpp, memory
//    reads only: no card, no file, no SetSpeed) or from the ctest.
//  ⚠ Thread: the one that creates the window and pumps messages (the wb_serve main loop), as NativeHost.h.
// ===========================================================================
#ifndef W906_UI_NATIVE_NATIVETEACH_H
#define W906_UI_NATIVE_NATIVETEACH_H

#include <string>
#include <vector>

namespace w906native {

// one TECH_PARA (two = false) or one TECH_TWOPARA (two = true).  Slot 1 is used only when two = true.
struct TeachPoint {
    int           index;          // TechPara[index] or TechTwoPara[index]
    bool          two;
    std::string   key[2];         // Key / Key[0..1] (= the golden edit name = the teach.ini key)
    int           motIndex[2];    // MotorSelect; -1 = none
    std::string   alias[2];       // MOT[MotorSelect].Alias ("" = golden ReadFromFile reads nothing and sets 0)
    std::string   tab, group, label;   // golden tab path / GroupBox / Set-button caption of key[0] ("" = not found)
    bool          goldenHidden;   // TeachKeyInfo::goldenHidden of key[0] (AI(W906-NATIVE-ST02) 20260929 (St02-E, St02-E2 teach review note 2)
    // ---- values (null = has* false, never 0) ----
    bool          hasValue[2];    // false = the Parameter pointer is NULL
    int           value[2];       // *Parameter (Tech.<field> / Teach.<field>)
    TeachPoint() : index(-1), two(false), goldenHidden(false)
    {
        motIndex[0] = motIndex[1] = -1;
        hasValue[0] = hasValue[1] = false;
        value[0] = value[1] = 0;
    }
};

// one distinct motor that some row uses (the glue computes it ONCE per tick, not once per row)
struct TeachAxis {
    int           motIndex;       // MOT[] index
    std::string   alias, numberAlias;   // MOT[i].Alias / MOT[i].NumberAlias (golden Panel2)
    int           motorType;      // HTMotor::MotorType; -1 = no drive object
    bool          hasNow;         int now;          // edtNowPosition (golden ReadPos; 1203: monitor cmdPos)
    bool          hasEnc;         int enc;          // pnlEncoderPos (golden: ReadEncoderPos for MotorType 1 / 3, else ReadPos)
    bool          hasHomeOffset;  int homeOffset;   // edtSetToOffset (Motor->LastHomePos)
    int           servoOn, alarm, busy, inPos;      // -1 = null
    int           homeFlag;                         // 0 / 1 / 2, -1 = null
    bool          ledKnown;       unsigned long motionIO;  unsigned state;   // ALed1..10 (MotorLedOn)
    std::string   quality, source, errText;         // "good" | "partial" | "nosource"; source; why there is no value
    TeachAxis()
        : motIndex(-1), motorType(-1), hasNow(false), now(0), hasEnc(false), enc(0), hasHomeOffset(false), homeOffset(0),
          servoOn(-1), alarm(-1), busy(-1), inPos(-1), homeFlag(-1), ledKnown(false), motionIO(0), state(0) {}
};

// page-wide state.  known = false: no source hooked -> those rows are "—".
struct TeachPage {
    bool          known;
    int           activeMotor;    // C++'s fTeach->ActiveMotorIndex (the web page's teach axis); -1 = none
    std::string   setToOffset;    // golden edtSetToOffset->Text: filled only after a home from teach (V912 Timer1 :1399 / :1406);
                                  //   AI(W906-NATIVE-ST02) 20260929 (St02-E, St02-E2 teach review note 3) -- was the axis LastHomePos always
    TeachPage() : known(false), activeMotor(-1) {}
};

struct TeachSummary {
    std::string   buildConfig;
    std::string   provider;
    std::string   why;            // no axis has a value
    std::string   registryWhy;    // why there are no rows (fTeach not constructed ...)
    int           techParaCount, twoParaCount;   // registry sizes as read
    int           distinctMotors;
    unsigned long overlayCalls;   // overlay hook calls in the last snapshot (= once per distinct motor)
    bool          monitorOpen;
    int           monitorAxes;
    unsigned long pollCount;
    unsigned long keepaliveCalls;
    TeachSummary() : techParaCount(0), twoParaCount(0), distinctMotors(0), overlayCalls(0), monitorOpen(false),
                     monitorAxes(0), pollCount(0), keepaliveCalls(0) {}
};

// table columns (grid column index)
enum TeachColumn {
    kTcColNo = 0, kTcColTab, kTcColGroup, kTcColLabel, kTcColKey, kTcColMotor, kTcColValue, kTcColPos, kTcColSet, kTcColGo,
    kTcColKey2, kTcColMotor2, kTcColValue2, kTcColPos2, kTcColIni, kTcColCount
};
// motion-panel rows (the Item / Value grid; the value is column 1)
enum TeachDetailRow {
    kTdRowPoint = 0, kTdRowTab, kTdRowLabel, kTdRowValue, kTdRowIni, kTdRowPanel2, kTdRowAxis, kTdRowNow, kTdRowEnc,
    kTdRowMotorType, kTdRowSetToOffset, kTdRowServo, kTdRowAlarm, kTdRowBusy, kTdRowInPos, kTdRowHomeFlag, kTdRowNow2,
    kTdRowSpeed, kTdRowMoveTo, kTdRowLock, kTdRowActiveWeb, kTdRowQuality, kTdRowSource, kTdRowWhy, kTdRowCount
};

// Key -> golden tab / group / Set caption (generated from the dfm2rc IR, NativeTeach.cpp).  0 = not in the table.
struct TeachKeyInfo {
    const char*   key;
    const char*   tab;            // "Input Arm", "Hand Pitch", "AOA > InArm" ... (outer > inner TTabSheet caption)
    const char*   group;          // enclosing TGroupBox caption(s), "" = none
    const char*   label;          // the row's Set-button caption, "" = the button is shared by several rows
    bool          goldenHidden;   // AI(W906-NATIVE-ST02) 20260929 (St02-E, St02-E2 teach review note 2): golden builds the row with Visible=false (its edit / buttons hidden
                                  //   at construction, 906 uteach.cpp:61-75): registry fTeachRegistry.cpp:288-293 / :672-679
};
const TeachKeyInfo* TeachFindKey(const std::string& key);
int                 TeachKeyTableSize();
const TeachKeyInfo* TeachKeyAt(int i);                 // test probe: the i-th entry (0 = out of range)

bool TeachOpen(bool show);
void TeachClose();
bool TeachIsOpen();
void TeachUpdate(const std::vector<TeachPoint>& points, const std::vector<TeachAxis>& axes, const TeachPage& page,
                 const TeachSummary& sum);

// ---- test probes ----
void*         TeachHwnd();
int           TeachRowCount();                        // rows shown (after the filter)
std::wstring  TeachCellText(int row, int column);
int           TeachRowPoint(int row);                 // index into the points vector; -1 = none
std::wstring  TeachDetailText(int row);               // the Value column of a motion-panel row
std::wstring  TeachLampText(int led);                 // "1" / "0" / "—"
int           TeachSelectedPoint();                   // index into the points vector; -1 = none
int           TeachPanelMotor();                      // the panel's MOT[] index; -1 = none
void          TeachSelect(int row);                   // as a click on that (visible) row (screen state only)
bool          TeachSetFilter(const std::string& tab); // "" = all, else a top-level tab name (UTF-8); false = not offered
bool          TeachSetFilterItem(int item);           // 0 = all; the last item may be "(no tab)"; false = no such item
int           TeachFilterCount();                     // combo items (including "all")
std::wstring  TeachFilterText(int item);
int           TeachButtonCount();                     // every BUTTON child
int           TeachEnabledButtonCount();              // enabled BUTTON children (v1: only Exit)
void*         TeachListGrid();
void*         TeachDetailGrid();
void*         TeachLampGrid();
std::wstring  TeachSummaryText();
std::wstring  TeachNoteText();
double        TeachLastUpdateMs();

}  // namespace w906native

#endif  // W906_UI_NATIVE_NATIVETEACH_H
