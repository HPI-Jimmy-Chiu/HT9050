// =============================================================================
//  BinDisplay/BinDispBringUp_St02.h -- ST02-C14 Bin Display, the C++ bring-up (906 base).
//  AI(W906-ST02-C14) 20261002 (St02-E helper).  Body: BinDisplay/BinDispBringUp_St02.cpp (its banner has the full map).
//
//  Golden 906 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven (cp950):
//    TDataModule3                 BinDisplay\MyBinDisp.h:21-32, MyBinDisp.cpp:21 / :32-35 / :3070-3073, MyBinDisp.dfm:1-68
//    TMyBinDispCtrl::Timer1Timer  BinDisplay\MyBinDisp.cpp:284-604 (the body lives in the .cpp of this pair)
//    CreateForm(TDataModule3)     HT9045.cpp:212
//    SystemModularInitial         database.cpp:1539-1546 (only its bin-display half, :1543-1545)
//    InitialOK copies             main.cpp:10483-10485 (FormShow) / :11567-11569 (FormClose)
//    TfBinSel::FormShow pause     cBinSel.cpp:1719-1721
//  Nothing here is 912 (the 912 delta is a separate step).
// =============================================================================
#ifndef BinDispBringUp_St02H
#define BinDispBringUp_St02H

#include "vclcompat/vcl_compat.h"   // Spcomm::TComm, vclcompat::TComponent, TObject

// ---------------------------------------------------------------------------
//  golden MyBinDisp.h:21-32 -- the design-time data module that owns the two SPComm ports.
//  [W906] TDataModule (VCL) -> vclcompat::TComponent (the base vclcompat TComm already uses); __published / __fastcall dropped.
// ---------------------------------------------------------------------------
class TDataModule3 : public vclcompat::TComponent
{
public:     // golden __published
    Spcomm::TComm *BinDisp;                                                     // MyBinDisp.dfm:8-37
    Spcomm::TComm *BinDisp2;                                                    // MyBinDisp.dfm:38-67
    void DataModuleDestroy(TObject *Sender);                                    // MyBinDisp.dfm:3 OnDestroy
public:
    explicit TDataModule3(vclcompat::TComponent* AOwner);                       // golden MyBinDisp.cpp:32-35 + the .dfm stream
    virtual ~TDataModule3();
private:
    TDataModule3(const TDataModule3&);
    TDataModule3& operator=(const TDataModule3&);
};
extern TDataModule3 *DataModule3;                                               // golden MyBinDisp.cpp:21 (extern PACKAGE in MyBinDisp.h:34)

// ---------------------------------------------------------------------------
//  Boot / timer seats (called by tools/wb_serve.cpp and MainTimersSt02.cpp)
// ---------------------------------------------------------------------------
// The bin-display half of golden SystemModularInitial (database.cpp:1543-1545) + CreateForm(TDataModule3) (HT9045.cpp:212).
// Types other than 3 / 4: does nothing at all (BinDisCtrl and DataModule3 stay NULL, as main does today).
void W906_BinDispSystemModularBoot_St02();
// golden FormShow main.cpp:10483-10485: HSys.BinDisCtrl->InitialOK=InitialOK, once, on the first pass that sees InitialOK.
void W906_BinDispFormShowAt_St02();
// golden FormClose main.cpp:11567-11569: the same copy (InitialOK is false by then), once.
void W906_BinDispFormClose_St02();
// golden TfBinSel::FormShow cBinSel.cpp:1719-1721: HSys.BinDisCtrl->ProcessStopStart(false) (types 3 / 4).  Returns what it did.
const char* W906_BinDispBinSelFormShow_St02();
// The vclcompat TComm reader thread only queues; this hands each quiet chunk to golden CommBinReceiveData(2) on the tick thread.
void W906_BinDispRxPumpAt_St02(unsigned long now);
// golden Timer1 (ht9045_bindisp::TTimer): its Interval when it should run (Enabled, Interval > 0, an instance), else 0.
unsigned long W906_BinDispTimer1Interval_St02();
// One golden Timer1 tick: Timer1->OnTimer -> TMyBinDispCtrl::Timer1Timer.
void W906_BinDispTimer1Fire_St02();

// ---------------------------------------------------------------------------
//  ST02-C14 part 3, the web "Bin Display Status" pane (golden tsUnloadMap).  AI(W906-ST02-C14) 20261002 (St02-E helper).
//  Golden 906 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven (RULINGS_20261002 #20: this card is 906 only).
//  golden ChangeBinDispStatus (cShowBinSelect.cpp:208-386) paints the tab only while it is PageControl1's ActivePage
//  (:279-292; G16 paints always).  The web page cannot tell C++ which of its tabs it shows, and the facade's ActivePage is
//  never tsUnloadMap (FormShow / a tab click never reach it), so without help the pane would read panels nobody paints.
//  [W906] MainTimer3.cpp runs the once-a-second call inside this scope:
//    * for that one call the facade's ActivePage is tsUnloadMap (the tab counts as shown); afterwards the old page is back.
//      Painting is all that call does after the early return while G16 is off (the KYEC / AMD arm :340 is S25-gated), so the
//      only difference from golden is that the panels are also fresh while the operator looks at another tab -- which nobody
//      sees.
//    * golden :269-273 jumps to the tab on every call that sees an error (PageControl1->ActivePageIndex=3 :272, so once a
//      second while the error lasts).  The scope parks ActivePageIndex at -1 for the call; a 3 afterwards is that jump:
//      W906_BinDispJumpSeq_St02() counts it, and the facade keeps index 3 with ActivePage = tsUnloadMap (what the VCL setter
//      does).  No jump -> the old index comes back.
//  HUMAN REVIEW B (behaviour vs golden): "paint as if shown", and the web pane following the jump (906: every second while a
//  unit is in error, so the operator cannot stay on another tab of the BinSelect window then -- golden 906's own behaviour).
// ---------------------------------------------------------------------------
class W906_BinDispShownScope_St02
{
public:
    W906_BinDispShownScope_St02();
    ~W906_BinDispShownScope_St02();
private:
    void* keepPage_;                                                            // TTabSheet* (forms/fShowBinSelect.h); void* keeps this header facade-free
    int   keepIndex_;
    bool  armed_;
    W906_BinDispShownScope_St02(const W906_BinDispShownScope_St02&);
    W906_BinDispShownScope_St02& operator=(const W906_BinDispShownScope_St02&);
};
// How many golden :272 jumps the scope has seen (0 = none yet).  WebBinDispStatus_St02.cpp publishes it as binsel.disp.jumpSeq.
unsigned long W906_BinDispJumpSeq_St02();
// true when this process runs under ctest's redirect environment (no COM port may ever be opened then).
bool W906_BinDispUnderCtest_St02();

// ---- ctest only -----------------------------------------------------------
void W906_BinDispTestReset_St02();                                              // the once-latches, the receive queues, the counters
void W906_BinDispSetRxClock_St02(unsigned long (*clock)());                     // NULL = GetTickCount (the clock the queue stamps with)
unsigned long W906_BinDispRealPortProbes_St02();                                // real GetCOMPortStatus calls (must stay 0 under ctest)

#endif
