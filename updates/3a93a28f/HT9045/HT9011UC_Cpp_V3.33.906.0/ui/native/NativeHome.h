// ===========================================================================
//  ui/native/NativeHome.h
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.home ("Motor Home Monitor") as a native Win32 window -- v1
//  DISPLAY ONLY.  NOT in golden as a file; golden is TfHome (V912 uhome.cpp / .h / .dfm).  Inventory: St02-E2
//  ST02_NATIVE_INVENTORY_20260929.md §3.  Golden line numbers below are V912
//  (D:/HT9045/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/uhome.cpp); the 906_0625_Steven tree has the same lines up to :718.
//
//  What it shows (golden control -> here):
//    THomeClass rows (InitialHomeClass golden :115, only Visible==true rows, golden order; layout loop golden :357-375:
//    15 rows per column, then the next column)           -> the row grid: 15 rows x (Name | Lamp | Pos) per column group
//      labName  = MOT[i].NumberAlias (golden :82)         -> Name
//      ledHome  (ShowLed :664-680: 0 off / 1 lime / 2 red / 3 yellow)
//                                                          -> Lamp; "—" (null lamp: white fill, grey edge) while the
//                                                             port's ShowLed is a GATE no-op (forms/fHome.cpp:485-503)
//      edPos    (ShowMotorHomePos :694 / :696; "0" from the ctor :90)
//                                                          -> Pos (the text the port keeps, forms/fHome.cpp:166/:533/:535)
//    ListBox1 (progress log, newest first: Insert(0,...)) -> the log grid (one column)
//    Panel2 + Label26 "Reset OK" (FormShow :4996 hides it, ProcessMotorHome case 1100 :3661 shows it)
//                                                          -> the status line; "—" while the port has no Panel2
//                                                             (GATE W906-HOME-C2-PANEL2, uhome.cpp:639-645 / :3418-3424)
//    sbAbortHome "Abort Home"                              -> a WS_DISABLED BUTTON; the window procedure has NO branch for it
//    SpeedButton1 "Exit"                                   -> the only enabled button: DestroyWindow() of THIS window only.
//                                                             NOT golden SpeedButton1Click -> Close() -> FormClose, which
//                                                             clears fShow -- and fShow==false stops the home sequence.
//
//  ⚠ Display only, by construction (same as NativeMotorTest.h): this file and NativeHome.cpp include only <windows.h>,
//    STL and NativeHost.h / NativeGrid.h.  No machine code is reachable from the window: no fHome->Close() / Show(),
//    no sbAbortHomeClick (GaliMotorServoOff + fAbort), no ShowMotorHomePos (it reads the card and, on Z > 5000, stops
//    every motor -- St02-E2 inventory §0.1), no ScanKey (golden Timer1: the panel Pause key runs Abort Home).
//    The data comes in from the wb_serve glue (NativeFormsWbServe.cpp HomeSnapshot, memory reads only) or from the ctest.
//  ⚠ Thread: the one that creates the window and pumps messages (the wb_serve main loop), as NativeHost.h.
// ===========================================================================
#ifndef W906_UI_NATIVE_NATIVEHOME_H
#define W906_UI_NATIVE_NATIVEHOME_H

#include <string>
#include <vector>

namespace w906native {

// one golden THomeClass row that golden shows (Visible == true), in HomeClass order
struct HomeRowData {
    int           index;        // THomeClass::index = MOT[] index (== the HomeClass slot, golden case 600 contract)
    std::string   name;         // labName->Caption (golden :82 MOT[MotNo].NumberAlias, set once by InitialHomeClass)
    bool          hasPos;       // false = no edPos object (null, never 0)
    std::string   pos;          // edPos->Text as the port keeps it
    int           lamp;         // ShowLed attr: -1 = no source (the port's ShowLed is a GATE no-op), 0 off, 1 / 2 / 3
    HomeRowData() : index(-1), hasPos(false), lamp(-1) {}
};

// the form-wide state (golden members of TfHome, one per form)
struct HomePage {
    bool          known;        // false = no fHome -> the status line is "—"
    bool          fShow;        // golden uhome.h:70 (FormShow true / FormClose false)
    bool          fAbort;       // golden uhome.h:71
    int           homeStep;     // golden uhome.h:69 iHomeStep (1 = idle)
    int           resetOk;      // Panel2->Visible ("Reset OK" + Exit): -1 = no source, 0 hidden, 1 shown
    HomePage() : known(false), fShow(false), fAbort(false), homeStep(0), resetOk(-1) {}
};

struct HomeSummary {
    std::string   buildConfig;
    std::string   provider;
    std::string   why;          // why there are no rows
    std::string   lampWhy;      // why the lamp column is "—"
    std::string   resetWhy;     // why "Reset OK" is "—"
    int           classCount;   // HomeClass.size() (hidden rows included)
    int           hiddenCount;  // Visible == false rows (golden does not show them)
    int           logLines;     // ListBox1->Items->Count
    unsigned long keepaliveCalls;
    HomeSummary() : classCount(0), hiddenCount(0), logLines(0), keepaliveCalls(0) {}
};

// golden layout loop (uhome.cpp :350-375): iMaxRowItem = 15 rows per column; per column Name (label, x 5),
// Lamp (ledHome, x 170), Pos (edPos, x 195); column pitch iLPitch = 270
const int kHomeMaxRowItem = 15;
enum HomeField { kHmFieldName = 0, kHmFieldLamp, kHmFieldPos, kHmFieldCount };

bool HomeOpen(bool show);
void HomeClose();
bool HomeIsOpen();
void HomeUpdate(const std::vector<HomeRowData>& rows, const std::vector<std::string>& log, const HomePage& page,
                const HomeSummary& sum);

// ---- test probes ----
void*         HomeHwnd();
int           HomeRowCount();                          // rows shown (motors), not grid rows
std::wstring  HomeRowText(int item, int field);        // through the grid's window procedure
std::wstring  HomeLampText(int item);                  // "—" / "0" / "1" / "2" / "3"
int           HomeLogCount();
std::wstring  HomeLogText(int line);                   // line 0 = newest (golden Insert(0,...))
std::wstring  HomeStatusText();
int           HomeButtonCount();                       // every BUTTON child
int           HomeEnabledButtonCount();                // enabled BUTTON children (v1: only Exit)
void*         HomeRowGrid();
void*         HomeLogGrid();
std::wstring  HomeSummaryText();
double        HomeLastUpdateMs();

}  // namespace w906native

#endif  // W906_UI_NATIVE_NATIVEHOME_H
