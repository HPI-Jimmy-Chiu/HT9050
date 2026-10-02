// ===========================================================================
//  JsonBridge/ChanHome.h -- the web Home Monitor (web/page/HW.home.html, golden TfHome uhome.dfm / uhome.cpp).
//  AI(W906-HOMEMON) 20261001.
//
//  TWO THINGS, both on the tick thread (the thread that runs MainProc -> ProcessMotorHome, so no lock is needed):
//
//  1. TAGS  W906_StageHomeMonitor(snap, streamWanted), called from PublishExtraTags (tools/wb_serve.cpp), i.e. inside
//     PublishHandlerTags before commitPublish().  Staged ONLY while the window is open: some browser has the Home Monitor
//     open or minimized (streamWanted = W906_PageStreamWanted("home"), RULINGS_20260930 #12, the same gate as
//     motionView.trays.*), or the home sequence has it on screen (fHome->fShow, golden FormShow).  Otherwise nothing is
//     staged: the family leaves the snapshot, the server sends each tag once as null, and the page is closed anyway.
//       home.fShow       bool    fHome->fShow   (golden uhome.h:70; FormShow :4847 / FormClose :4870)
//       home.fAbort      bool    fHome->fAbort  (golden uhome.h:71; sbAbortHomeClick :4983)
//       home.step        int     fHome->iHomeStep (golden uhome.h:69; 1 = idle)
//       home.resetOk     bool    fHome->Panel2->Visible -- the "Reset OK" panel (golden FormShow :4846 false, case 1100 :3516 true)
//       home.rows        string  JSON array, one object per Visible THomeClass in vector order (golden layout loop :357-375):
//                                {"slot":i,"motor":index,"name":labName->Caption,"x":labName->Left,"y":labName->Top,
//                                 "pos":edPos->Text,"lamp":W906_HomeLedState[i]}
//                                name = golden :82 MOT[MotNo].NumberAlias; pos = golden :90 "0" then ShowMotorHomePos (:694/:696);
//                                lamp = golden ShowLed's attr (:664-680): 0 off (clSilver) / 1 clLime / 2 clRed / 3 clYellow
//       home.log         string  JSON array of the newest ListBox1 lines, newest first (golden only Insert(0, ...) and
//                                Clear()), at most kHomeLogMax
//       home.log.count   int     ListBox1->Items->Count (all lines, also those beyond kHomeLogMax)
//
//  2. COMMAND  act.home.abort -> golden TfHome::sbAbortHomeClick (uhome.cpp:4980-4986: GaliMotorServoOff("sbAbortHomeClick")
//     + fAbort=true + Close()), only while fHome->fShow -- golden's button is on the form, and golden shows that form only
//     from the home sequence (ProcessMotorHome case 20, uhome.cpp:2368).  Not shown -> refused ("not-open: ..."), nothing
//     is called.  No confirm box (golden has none).  W906_HomeAbortWire is the testable core; W906_HomeAbortCommand
//     completes the WebBridge command.  Dispatched by tools/wb_serve.cpp in the main drain and, [W906] like motor.stop
//     (AI(W906-W4D) 20260925: a stop is never held behind a box), in the three blocking-dialog waits; allowed while closing
//     (FileRW/MainClose.cpp CmdAllowedWhileClosing); exempt from the operator token (WebBridge/WebBridgeServer.cpp, as
//     motor.stop) and from the anti-double-click guard (WebCmdGuard.cpp, "stop direction", as pause.run).
//     ⚠ The modal-wait part is NOT golden: under a VCL ShowModal box the modeless Home Monitor's button cannot be clicked.
//       What golden does have during a box is TfHome::Timer1 (10 ms, uhome.dfm) -> ScanKey: the panel PAUSE key calls
//       sbAbortHomeClick (uhome.cpp:4874-4889) -- timers run in the modal loop.  (That timer is switched off by FormClose,
//       :4871, and nothing switches it on again, so golden has it only until the first close after boot.)
// ===========================================================================
#ifndef JSONBRIDGE_CHANHOME_H
#define JSONBRIDGE_CHANHOME_H

#include <cstddef>
#include <string>

namespace webbridge { class TagSnapshot; class WebBridgeServer; struct WebCommand; }
class TfHome;

namespace ht9045 { namespace homemon {

const int kHomeLogMax = 100;     // newest ListBox1 lines sent as home.log (golden's list box scrolls; ~38 Insert sites per run)

// The two JSON strings, from the given form (tests pass the global fHome).  lamp may be 0 (every lamp null).
std::string RowsJson(const TfHome& h, const int* lamp, int lampCount);
std::string LogJson(const TfHome& h, int maxLines);

// golden "the Home Monitor is on screen" = fHome->fShow (the C++ member, program state; R142 = A).
bool HomeShown();

} }

// Stages the home.* family when streamWanted or the home sequence shows the form; returns the number of tags staged (0 or 7).
std::size_t W906_StageHomeMonitor(webbridge::TagSnapshot& snap, bool streamWanted);

// act.home.abort.  true = golden sbAbortHomeClick ran (ack = JSON object); false = refused / failed (ack = the reason).
bool W906_HomeAbortWire(const std::string& valueJson, std::string& ack);
void W906_HomeAbortCommand(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc);

#endif // JSONBRIDGE_CHANHOME_H
