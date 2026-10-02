// =============================================================================
//  EtherCAT/Pci1203GaliRouteCore.h -- the Galil-string -> PCIE-1203 translator
//  behind Motor/GaliRoute.h, as PURE logic over an IO seam (ctest-able).
//
//  AI(W906-INDEXZ-1203) 20260929: new file (RULINGS_20260929 section 5; design
//  INDEXZ_1203_DESIGN_20260929.md sections 4.2-4.5).
//  AI(W906-INDEXZ) 20260930: INBOX 113 redo after the cc426093 review (revert 94cd23d7):
//    #1 a stop cancels the home job for good (HomePoll never answers "done" for it);
//    #2 VS0;SP0 during a move HALTS it and the move RESUMES (golden Galil semantics);
//    #6 a failed stop is latched (Fault()) and always printed (LogForce), never silent;
//    + stops that bypassed the route (NoteForeignStop, the INBOX 112 review HIGH-1 rule).
//  AI(W906-INDEXZ-1203) 20260930: review round 2 of a8c3b04e (three adversarial reviews):
//    A  the SystemStart resume rule (b) is GONE -- it resumed on the first route call after SystemStart turned true,
//       i.e. on ckernel's "KS4,4,4,4;VT..." string, BEFORE golden's safe-door check (port ckernel.cpp:1038-1041 =
//       golden ckernel.cpp:526-531, jou 20171030) and motor-power check. The only resume left is golden's own:
//       DoSystem's one-shot "VS<v>;SP<s>,<s>,<s>,<s>;" (GATE G22, lifted to golden csystem.cpp:4657-4672 in the
//       same change), and only on the tick thread;
//    B  "DP<x>,<y>" (golden TestZ1SetPos) defines Z1's command position (kCmdAxSetCmdPos) instead of being dropped;
//    D1 VS0;SP0 during a route-served HOME halts it (golden HM at speed 0 stays in progress) and the resume string
//       restarts it; ST / AB still cancel it;
//    D2 a stop outside the route by an OPERATOR (Motor Test / Teach / the pci1203 page = golden "ST" on the index
//       axis) ends a halt and asks for golden ST's bookkeeping; the ALARM sweep's stop (golden StopAllMotor skips
//       the index axes, PCIL132_StopMotor) keeps it;
//    D3 a halt ends when another writer (the shared ledger) commanded the axis after the route's last command;
//    D4 forced lines are held back per failure KEY (not per text), so G04's alternating ST / VS0;SP0 does not flood;
//    D5 a failed zeroing after a card-side home fails the home (was: logged, then "done" on an unzeroed encoder);
//    D6 the DecStop results before a card-side home are checked (failed stop latched, home not started);
//    D7 off the owner thread nothing reads the monitor or the ledger (confinement, see THREADS below).
//
//  The golden Index state machines keep talking Galil to "axis Y" (= MTestZ1).
//  This class answers the ~15 string shapes that can reach Z1 on HT9050
//  (design table 4.3) the way a Galil card would, reading the 1203 monitor's
//  samples and writing through TPci1203Control::Execute -- both only through
//  IGaliRouteIo, so no vendor call, no monitor, no control object in here.
//
//  UNITS (Motor Test R2 and the Teach page use the same card numbers):
//    golden Galil Z: command count  PAY=n  with n = GetRealPos(-flow)
//                    TDY  = commanded count (Gali_ReadPos = -TD*GearRatio)
//                    TPY  = encoder count, opposite sign (Gali_ReadEncoderPos = +TP*GR)
//    route         : kCmdAxMoveAbs value = -n ; TDY = round(-cmdPos) ; TPY = round(+actPos)
//                    "DP..,v" / "DPY=v" -> kCmdAxSetCmdPos value = -v (so TDY reads v afterwards)
//    Speeds are Galil counts/s sent raw (golden never scales them by GearRatio):
//    run = min(SP, PJogHighSpeed), init = min(InitSpeed, run),
//    acc/dec = min(AC/DC, the Mot_Table Acc/Dec) -- EastSun Q1/Q4.
//
//  SAFETY RULES (design 4.5)
//    * never ShowErrorMessage: a refused or failed motion POISONS the axis --
//      "MG_BG" answers moving and "TI" reports the Z1 alarm bit until a stop
//      (ST / AB / VS0;SP0) or a successful ResetError (SH on an ERROR_STOP axis).
//      golden's own machinery then raises the JAM (ScanIndexMotorCanMove before
//      the next move; the TIMO latch -> ALM_MOTOR_MOVE -> the ckernel sweep).
//    * a stale sample never looks done: "moving" until a VALID sample from a
//      poll that ended after the last command by the route OR by Motor Test
//      (the shared ledger), with the card open; after a stop that bypassed the
//      route, until a poll that STARTED after it has ended.
//
//  THREADS (D7, AI(W906-INDEXZ-1203) 20260930)
//    The owner thread is the one that installed the route (the wb_serve tick loop, which also runs the monitor's
//    Poll and every TPci1203Control::Execute). On any OTHER thread every entry point is CONFINED: it calls none of
//    CardOpen / PollCount / Slot / Sample / DriveKind / Exec / Caps / NoteIssued / LastIssued / SleepMs / OrgHome (the
//    monitor, the control object and Motor Test's ledger are single-threaded), only OnOwnerThread / NowMs / Log /
//    LogForce. What such a call does:
//      motion (BGY, a resume string, DP, a home)    -> refused, nothing sent, latched as a failed motion, the axis
//                                                     POISONED (golden then raises the JAM on the tick thread);
//      a stop (ST / AB / VS0;SP0)                   -> NOT SENT (TPci1203Control is not thread-safe), latched as a
//                                                     failed stop, a home job CANCELLED, the axis POISONED;
//      a query                                      -> the conservative answer (MG_BG 1, TI alarm, TS 0x8E, TD / TP
//                                                     the last value seen on the owner thread);
//      NoteForeignStop                              -> uses the slot / pollCount last seen on the owner thread.
//    The binding (EtherCAT/Pci1203GaliRoute.cpp) serialises every entry point with one lock, so this state is never
//    written by two threads at once.
//
//  HALT / RESUME (review #2; rule (b) removed by A above)
//    golden StopAllMotor(true) sends "VS0;SP0,0,0,0;" (Motor/myGALILmotor.cpp:5764):
//    on a Galil card every running move decelerates to speed 0 and stays IN
//    PROGRESS (MG_BG = 1) until a new speed arrives -- golden DoSystem sends
//    "VS<v>;SP<s>,<s>,<s>,<s>;" once after START (golden csystem.cpp:4657-4672,
//    the bStartKeyPressCheck one-shot, GATE G22 in the port -- lifted) and the move goes on to its target.
//    golden only gets there after ScanSystemSensor's safe-door check and DoSystem's CountMotorPowerDelay: a START
//    with the door open never reaches it (SystemStart is false again), so the halted move does not resume.
//    The 1203 has no "speed 0, still in progress": the route STOPS the axis and
//    keeps the move (target, speeds) -- or the home job (D1) -- as HALTED ("MG_BG" = 1, "TS" moving) and re-issues
//    it (4 x SetSpeed + MoveAbs to the same target; or the home from its start) on golden's resume string only:
//    an SP with a non-zero Y speed and no BG, on the owner thread.
//    ST / AB / a new BG / an operator stop outside the route (D2) / another writer's command (D3) end a halt.
// =============================================================================
#ifndef ETHERCAT_PCI1203GALIROUTECORE_H
#define ETHERCAT_PCI1203GALIROUTECORE_H

#include <map>
#include <string>
#include <vector>

#include "EtherCAT/Pci1203Control.h"   // Pci1203Cmd / Pci1203CmdResult (plain structs, no link dependency)

namespace ht9045 {

// One monitor sample of the routed axis (Pci1203AxisSample's four fields).
struct GaliRouteSample {
    unsigned       state;     // Acm_AxGetState raw (STA_AX_*: 0 DISABLE 1 READY 3 ERROR_STOP 4 HOMING 5 PTP ...)
    unsigned long  motionIO;  // Acm_AxGetMotionIO raw (AX_MOTION_IO_*)
    double         cmdPos;    // card units
    double         actPos;
    GaliRouteSample() : state(0), motionIO(0), cmdPos(0.0), actPos(0.0) {}
};

// Ceilings for a move, read from the golden motor object before every move
// (Motor Test's Reload Motor Data can change them in place).
struct GaliRouteCaps {
    unsigned jogHigh;         // Motor->PJogHighSpeed
    unsigned initSpeed;       // Motor->InitSpeed
    double   accDb, decDb;    // Motor->GetAccDataBase() / GetDecDataBase()
    GaliRouteCaps() : jogHigh(0), initSpeed(0), accDb(0.0), decDb(0.0) {}
};

class IGaliRouteIo {
public:
    virtual ~IGaliRouteIo() {}
    // owner thread only (THREADS above) -------------------------------------------------------------------------
    virtual bool          CardOpen() = 0;                              // monitor present, open, polling, card().open
    virtual unsigned long PollCount() = 0;                             // monitor card().pollCount
    virtual int           Slot(std::string& why) = 0;                  // the routed axis's monitor slot, -1 = none (why)
    virtual bool          Sample(int slot, GaliRouteSample& s) = 0;    // false = no valid, opened sample
    virtual int           DriveKind(int slot) = 0;                     // 1 DS402 servo, 0 another drive, -1 unknown
    virtual Pci1203CmdResult Exec(const Pci1203Cmd& c) = 0;            // TPci1203Control::Execute
    virtual void          Caps(GaliRouteCaps& c) = 0;
    virtual void          NoteIssued(int slot, unsigned long poll) = 0;       // shared ledger (Motor Test's g_issuedPoll)
    virtual bool          LastIssued(int slot, unsigned long& poll) = 0;      // shared ledger
    virtual void          SleepMs(int ms) = 0;
    //AI(W906-INDEXZ2) 20261002: ORG polarity -- Z1's "at home" for the TS byte's HOME bit, decided by the TREE's rule, not
    //  by the route: 1 at home, 0 not at home, -1 unknown (treated as NOT at home: fail-closed), -2 no machine rule applies
    //  (golden's EtherCAT decode of the ORG bit: set = at home, golden myEthercatmotor.cpp:860). slot / motionIO are the
    //  valid sample the route answers from. The live binding returns W906_Ht9050OrgHome(MTestZ1) (Motor/mymotor.h; on
    //  HT9050 the ORG bit is read by the axis's Mot_Table SensorType -- 1: LOW = at home, 0: HIGH = at home, golden
    //  InitMotor's uOrgLogic, AI(W906-HT9050-ORG-ST) 20261002; M14 has SensorType 1) -- the value
    //  TMyMotor::ScanMotorStatus writes into Led[iHomeLed] after Gali_ScanMotStatus, so the two scans agree.
    virtual int           OrgHome(int slot, unsigned long motionIO) = 0;
    // any thread --------------------------------------------------------------------------------------------------
    virtual unsigned long NowMs() = 0;
    virtual bool          OnOwnerThread() = 0;                         // the thread that installed the route (the tick loop)
    virtual void          Log(const std::string& line) = 0;            // may be rate-limited by the binding
    virtual void          LogForce(const std::string& line) = 0;       // AI(W906-INDEXZ) 20260930: never dropped by a budget (failed stops, review #6)
};

// One statement of a Galil command string as the route reads it.
//   "SPY=900" -> cmd SP, axes "Y", assign, value "900"
//   "SP,1,2"  -> cmd SP, fields {"", "1", "2"}      (field k = axis X,Y,Z,W)
//   "BGYZ"    -> cmd BG, axes "YZ"      "MG_BGy" -> cmd MG, arg "BGY"
//   "AB1"     -> cmd AB, fields {"1"}   "ST" -> cmd ST (bare)
// Axis letters are folded to upper case and A/B/C/D -> X/Y/Z/W; S/T (vector
// planes) are kept as they are.
struct GaliStmt {
    std::string              cmd;
    std::string              axes;
    std::string              arg;
    bool                     assign;
    std::string              value;
    std::vector<std::string> fields;
    GaliStmt() : assign(false) {}
};
// Split on ';' and parse every non-empty statement. Returns the count.
int GaliSplit(const char* data, std::vector<GaliStmt>& out);

// Galil "TS" byte for the routed axis (golden Gali_ScanMotStatus decode:
// Led[iCwLed]=!(b&0x08), Led[iHomeLed]=!(b&0x02), Led[iCcwLed]=!(b&0x04),
// Led[iInposLed]=b&0x80). Mapped like golden's EtherCAT decode
// (myEthercatmotor.cpp:1166-1174): CW LED = LMT-, CCW LED = LMT+, HOME = ORG.
// AI(W906-INDEXZ2) 20261002: HOME = orgHome (IGaliRouteIo::OrgHome): 1 at home; 0 / -1 not at home; -2 the ORG bit read
// as golden's EtherCAT decode (set = at home -- the only mapping before 20261002; wrong for HT9050's M14, SensorType 1).
// No valid sample -> 0x8E (moving, no limit, not home).
long GaliRouteTsByte(bool moving, bool sampleValid, unsigned long motionIO, int orgHome);

// The installer's check of the M14 (MTestZ1) Mot_Table row: PCI1203, Enable 1,
// BoardID/Port >= 0, Direction 0 (the route maps card = flow like Motor Test,
// RULINGS_20260925 #13 6B; a flipped row is refused, not guessed). false + why.
bool GaliRouteRowOk(const std::string& cardModel, int enable, int boardId, int port, int direction, std::string& why);

// AI(W906-INDEXZ) 20260930: the installer's whole decision as a pure function (review #5: it used to install
// without an open card). Every fact is gathered by EtherCAT/Pci1203GaliRoute.cpp; the order of the checks is the
// order of the refusal text. true = install; false + why.
struct GaliRouteInstallFacts {
    bool        linked;             // Pci1203ControlLinked(): the binary has the SDK (HAVE_PCI1203)
    bool        controlArmed;       // Pci1203Control() != 0
    bool        monitorPresent;     // AI(W906-INDEXZ-1203) 20260930: review C -- Pci1203Monitor() != 0 (false = called before Pci1203MonitorEnable: a boot-order bug)
    bool        cardUsable;         // monitor present, holds a device handle, not Disabled, card().open (review #5)
    int         indexMotionCard;    // INDEX_MOTION_CARD in memory (must stay 0: MTestZ1 is the Galil index axis)
    bool        haveRow;            // Mot_Table has an M14 row
    std::string cardModel;          // ... its CardModel / Enable / BoardID / Port / Direction
    int         enable, boardId, port, direction;
    int         slotHits;           // opened monitor slots at (BoardID, Port) -- exactly 1, none stationAmbiguous
    int         slotAmbiguous;
    bool        z1Present;          // MOT[MTestZ1].Motor exists and is enabled
    bool        routeTaken;         // another Gali route is installed (one owner per axis)
    int         engineRowsAtAddr;   // TMyEtherCatMotor rows at (BoardID, Port): the engine motor route could claim it
    GaliRouteInstallFacts()
        : linked(false), controlArmed(false), monitorPresent(false), cardUsable(false), indexMotionCard(0), haveRow(false),
          enable(0), boardId(-1), port(-1), direction(0), slotHits(0), slotAmbiguous(0), z1Present(false),
          routeTaken(false), engineRowsAtAddr(0) {}
};
bool GaliRouteInstallOk(const GaliRouteInstallFacts& f, std::string& why);

enum { kGaliHomeRunning = 0, kGaliHomeDone = 1, kGaliHomeFailed = -1 };

// AI(W906-INDEXZ) 20260930: the build's install decision, made ONCE by the preprocessor in
// EtherCAT/Pci1203GaliRouteCore.cpp (W906_IDXZ_GATE, from MachineType.h) -- the engine motor route's four values
// (Pci1203MotorRoute.h kEcGate*). The installer (EtherCAT/Pci1203GaliRoute.cpp) branches on it; the ctest compares
// it with the macros (tests/test_gali_route_core.cpp part K); ht9045_indexz_route_armed_probe compiles the Core with
// the arming macros and a static_assert that the value is kGaliGateArmedBuild.
enum { kGaliGateSim = 1, kGaliGateOff = 2, kGaliGateNoStartRing = 3, kGaliGateArmedBuild = 4 };
int GaliRouteCompiledGate();

// AI(W906-INDEXZ) 20260930: review #6 -- what the route latched (never cleared while wb_serve runs).
struct GaliRouteFault {
    unsigned long stopFailed;       // stops (ST / AB / VS0;SP0 / the ExtDrive(0) after them / a home's DecStop) that did not reach the card or came back with an error
    unsigned long motionFailed;     // motions refused / failed (not DRY) -- they also poison the axis
    unsigned long lastPoll;         // pollCount when the last one happened
    std::string   lastWhat;         // e.g. "ST: Stop refused: ..."
    GaliRouteFault() : stopFailed(0), motionFailed(0), lastPoll(0) {}
};
std::string GaliRouteFaultText(const GaliRouteFault& f);   // one line for the operator, "" when nothing is latched

// NoteForeignStop's answer.
//   kGaliForeignMoveAborted  : the alarm sweep cut a RUNNING (not halted) move -- the caller does golden "ST"'s
//                              bookkeeping (W906_GaliRouteForeignStopBookkeeping), so golden re-issues the move
//   kGaliForeignStBookkeeping: AI(W906-INDEXZ-1203) 20260930 (D2) -- an OPERATOR stop = golden "ST" on the index
//                              axis: the move / halt / home is over, the caller does golden ST's bookkeeping always
enum { kGaliForeignNotOurs = 0, kGaliForeignNoted = 1, kGaliForeignMoveAborted = 2, kGaliForeignStBookkeeping = 3 };

class TGaliRouteCore {
public:
    TGaliRouteCore();
    void Bind(IGaliRouteIo* io);                 // resets every piece of state
    bool Command(const char* data, long* reply); // true = claimed
    int  HomeStart(bool homeDirection, unsigned homeHigh, unsigned homeLow, double acc, double dec);
    int  HomePoll();
    // AI(W906-INDEXZ) 20260930: a stop of monitor slot `slot` that reached Execute WITHOUT this route. Call only for a
    //   stop that went out (issued OK, or DRY). Not our slot -> kGaliForeignNotOurs. Always: the axis stays "moving"
    //   until a poll that STARTED after the stop has ended, and the failed mark is KEPT (only this route's own stop
    //   clears it). Then, AI(W906-INDEXZ-1203) 20260930 (D2):
    //     alarmSweep = true  (W906_MotorAccessOnAlarm's Stop1203 of every opened axis -- the port's 1203 half of golden
    //                         StopAllMotor, whose PCIL132_StopMotor SKIPS the index axes, golden Motor/mymotor.cpp:1864-
    //                         1867): a HALTED move / home stays halted (golden only sent VS0;SP0 to the Galil); a home
    //                         that was not halted is cancelled; a running move is over -> kGaliForeignMoveAborted;
    //     alarmSweep = false (an operator stop: Motor Test / Teach STOP, jog release, dead-man, the pci1203 page --
    //                         golden's operator stop on an index axis is Gali_Command("ST"), golden uMotorTest.cpp:
    //                         851-857 / 901-907 / 1654-1656, uteach.cpp:1270 / 2192): the halt ends, the move is over, a
    //                         home job is cancelled -> kGaliForeignStBookkeeping (golden myGALILmotor.cpp:577-592).
    int  NoteForeignStop(int slot, bool alarmSweep);

    // read-only state (tests, the boot line, the web diag)
    bool                  Poisoned()   const { return poisoned_; }
    const std::string&    PoisonWhy()  const { return poisonWhy_; }
    bool                  Halted()     const { return halted_; }
    bool                  HomeHalted() const { return halted_ && haltHome_; }
    bool                  MoveInProgress() const { return moveInProgress_; }
    bool                  HomeActive() const { return homeActive_; }
    bool                  HomeCancelled() const { return homeCancelled_; }
    unsigned long         MovesIssued() const { return moves_; }
    unsigned long         StopsIssued() const { return stops_; }
    unsigned long         Resumes()    const { return resumes_; }
    unsigned long         ForcedHeld() const { return heldTotal_; }   // forced lines held back by the per-key cooldown so far (D4), all keys
    const GaliRouteFault& Fault()      const { return fault_; }

private:
    struct Obs {
        bool            cardOpen;
        int             slot;
        bool            valid;
        bool            pending;
        GaliRouteSample s;
        std::string     why;
        Obs() : cardOpen(false), slot(-1), valid(false), pending(false) {}
    };
    struct MoveRec {                             // the last move this route issued (the HALT / RESUME rule)
        long sp, ac, dc, pa;
        bool hasAc, hasDc;
        MoveRec() : sp(0), ac(0), dc(0), pa(0), hasAc(false), hasDc(false) {}
    };
    struct HomeRec {                             // AI(W906-INDEXZ-1203) 20260930 (D1): the home job's parameters, re-issued on resume
        bool     dir;
        unsigned high, low;
        double   acc, dec;
        HomeRec() : dir(true), high(0), low(0), acc(0.0), dec(0.0) {}
    };
    struct ForceRec {                            // AI(W906-INDEXZ-1203) 20260930 (D4): one cooldown per failure key
        unsigned long ms;
        unsigned long held;
        ForceRec() : ms(0), held(0) {}
    };
    Obs  Observe();
    bool Pending(int slot);
    bool MovingOf(const Obs& o) const;           // pending / poisoned / halted / unobservable / a motion state
    void NoteDone(const Obs& o);                 // a query saw the move finished -> not in progress any more
    void Poison(const std::string& key, const std::string& why);
    void Issued(int slot);
    bool Failed(const Pci1203CmdResult& r) const;
    std::string Why(const Pci1203CmdResult& r) const;
    unsigned long Pc();                          // pollCount on the owner thread; the last one seen there otherwise (D7)
    void LatchStopFailure(const std::string& cmd, const std::string& reason);
    void LatchMotionFailure(const std::string& what);
    void Force(const std::string& key, const std::string& line);   // LogForce, one line per KEY per 10 s (held ones counted)
    bool ReplacedSince(int slot);                // D3: another writer commanded the slot after the route's last command
    void EndHaltIfReplaced(int slot);            // D3
    void Resume(long sp, const char* why, bool thr);
    int  IssueHome(const Obs& o);                // the drive's home (DS402 or card-side), from HomeStart and the resume

    bool DoMove(long sp, bool hasSp, long ac, bool hasAc, long dc, bool hasDc, long pa, bool hasPa, bool thr);
    void DoStop(bool emg, bool sp0, bool thr);
    void DoServo(bool on, bool thr);
    void DoDefinePos(long v, bool thr);          // B: golden "DP" on Y
    long TiByte(const Obs& o);
    void HomeFail(const std::string& why);
    void CancelHome(const char* by);

    IGaliRouteIo* io_;
    bool          onThr_;        // D7: the current entry point runs on the owner thread
    bool          poisoned_;
    std::string   poisonWhy_;
    bool          halted_;       // VS0;SP0 caught one of our moves / our home: stopped on the card, still "in progress" for golden
    bool          haltHome_;     // D1: ... and it was the home job
    bool          moveInProgress_;   // our last move was issued and nobody has seen it finish, stop or be replaced
    MoveRec       lastMove_;
    HomeRec       home_;
    bool          lmHasY_;       // the last LM (vector axes) included Y
    bool          haveOwn_;
    unsigned long ownPoll_;      // pollCount when the route last issued a command
    bool          haveForeign_;
    unsigned long foreignPoll_;  // pollCount when a stop bypassed the route (pending through the NEXT whole poll)
    bool          invalidSeen_;
    unsigned long invalidSincePoll_;
    bool          haveLast_;
    long          lastTd_, lastTp_;
    bool          tdHeld_;       // B: a DP defined TD; answer it until a sample newer than the DP is in
    int           lastSlot_;     // D7: the slot last resolved on the owner thread
    unsigned long lastPc_;       // D7: the pollCount last read on the owner thread
    // home
    bool          homeActive_, homeCancelled_, homeCardSide_, sawHoming_, readySeen_, zeroed_, zeroWait_;
    unsigned long homeStartMs_, readySinceMs_, zeroWaitMs_;
    unsigned long moves_, stops_, resumes_;
    GaliRouteFault fault_;
    std::map<std::string, ForceRec> force_;
    unsigned long heldTotal_;
};

}  // namespace ht9045

#endif  // ETHERCAT_PCI1203GALIROUTECORE_H
