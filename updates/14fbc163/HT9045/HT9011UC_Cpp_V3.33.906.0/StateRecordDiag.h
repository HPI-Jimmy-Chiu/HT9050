// =============================================================================
//  StateRecordDiag.h  --  State Record: the port-only W906_* diagnostic files.
//
//  AI(W906-S24) 20261004 (St02-E helper), card S-24 (RULINGS_20261004 #1).  NOT golden.
//
//  WHY.  Jimmy 1004: 「要提醒機台端有問題一定要做state record,方便追查問題」 and 「晚上派工確認state record要分析的內容
//  是否足夠分析？不足請補強」.  The gap analysis (St02 scratchpad s24/S24_GAP_ANALYSIS.md) found golden's State Record
//  files faithful but not enough on this port: golden relied on four screenshots (MotorView.bmp Servo / Alarm) and on a
//  person at the machine; the port has no screenshots and no file with IO / brake / dialog / HOME-flag state, and a stuck
//  tick loop cannot record at all.  Three real cases (HOME stuck at step 1520, the boot brake release, the notice that
//  would not close) could not be analysed from the record.
//
//  WHAT.  Nine extra files (seven of S-24, two of S2), all named W906_* (plus W906_Mailbox\ when the dialog mailbox is known), so golden's files
//  (Task_ListWithTime.csv, DecisionVariables.csv, MainFormSnapshot.txt, Ver.txt, ...) stay what they were and the
//  analysis skill still reads them:
//      W906_ReadMe.txt       what each W906_* file is; trigger, threads, snapshot age, build, exe time
//      W906_Health.txt       the tick loop's own pulse (publish calls, gaps), MainProc monitor, watchdog mark, 1203 monitor
//      W906_Motor1203.csv    one row per Mot_Table axis (+ MOT[] axes outside the table): table row, MOT[] engine flags /
//                            LEDs / caches, fHome->HomeClass slot, the PCIE-1203 monitor sample
//      W906_Home.txt         iHomeStep and the HOME globals, axes still to home, axes with no motor object, Home progress
//      W906_PowerBrake.txt   motor power variables, SW[] relay / brake outputs (command), Sen[] power / EMG inputs
//      W906_Dialogs.txt      fNote / MyMessageBox flags, the dialog slot, the mailbox copy
//      W906_OpLogTail.txt    the tail of %W906_OPLOG_DIR%\oplog_<yyyymmdd>.txt
//      W906_IO.csv           (S2) one row per IO point -- Sen[] / SW[] / Cylinder[] / the kit suckers: IO_Table row, address,
//                            enable, DI raw + logical, DO command + output-cache read-back, cylinder sensor pair, sucker
//                            valves + sensor + vacuum kPa
//      W906_Recent.csv       (S2) the IO and motor CHANGES of the last 600 s (at most kW906SrRecentCap rows): time, point /
//                            axis, field, old -> new; an axis row carries its cmd / enc caches (and the 1203 positions)
//
//  S2 (AI(W906-S24-S2) 20261005, St02-E; St02-M OK 11:4x).  The IO list (names, addresses, IO_Table rows) is built ONCE on
//  the tick thread and rebuilt only when the IO configuration changes (a signature of every Sen[] / SW[] / Cylinder[] /
//  sucker / MOT[] address field, checked at most once a second).  The change scan runs inside Publish every
//  kW906SrChangeScanMs (100 ms) and at every snapshot build: it reads each enabled point once, compares with the last
//  scan and stages the changes in a fixed array -- no allocation in steady state.  The changes go into a ring of
//  kW906SrRecentCap entries (oldest overwritten and counted); the writer copies at most that many, only the last
//  kW906SrRecentWindowMs, and says in the file when the cap cut the window short.  Reads that could log or pop a box on
//  every scan are not made: a point is read only after MyLaneIO.CheckPortRangeErr says 0, a VC8-guarded sucker point
//  only when the gate IsOn() asks would let it through (W906_Vc8SuckerGate without its print), and in a non-SIM build a
//  PCIE-1203 input through the 1203 route itself (one read per input byte per scan) -- what IOInputBit does, without
//  its MNetLog on a failed read.  The vacuum kPa is read at snapshot builds only, and only for VC8-registered suckers.
//
//  WHO CALLS IT (S-24 claims, laptop OK on handoff 587a830c):
//    ① cStateRecord.cpp:1387 -- the normal State Record: PublishNow + WriteFiles(NewPath) on the tick thread, before the
//       background job zips NewPath.
//    ② tools/wb_serve.cpp:5934 -- W906_SrDiagWbServeTick (wb_serve.cpp end of file) once per loop pass: installs the
//       wb_serve readers below on its first call, then W906_StateRecordDiagPublish();
//       tools/wb_serve.cpp:2522 -- GET /api/struct/staterecord.hang (socket thread) -> W906_StateRecordHangRequest
//       (StateRecordHang.cpp): a worker writes <root><yyyymmdd_hhmmss>_hang\ from the last snapshot and disk files only.
//    ⑥ web/page/Main.gbControlBtn.html:142 -- after the 15 s no-answer the page calls the hang route.
//
//  THREADS (the whole point -- a record must be possible while the tick loop is stuck).
//    * W906_StateRecordDiagPublish() / PublishNow() are TICK THREAD ONLY: the machine globals (MOT[], SW[], Sen[], fHome,
//      the PCIE-1203 samples -- Pci1203Monitor.h:1360 says single-thread) are read there and nowhere else.  The snapshot
//      is built OUTSIDE the lock and swapped in under the module's own CRITICAL_SECTION (the g_apiCache shape,
//      tools/wb_serve.cpp:6399-6441).  The first caller binds the tick thread; a call from any other thread is refused
//      and counted (publishRefused).  Calls closer than the minimum interval (default 250 ms) only update the counters.
//    * Read / Info / the writer / the hang request run on ANY thread: they copy the last snapshot under the lock and then
//      touch only that copy, their own clock and disk files.  They never read a machine global, never call ReadPos() /
//      ReadEncoderPos() / a sensor, never touch the task rings.
//
//  READERS installed by tools/wb_serve.cpp (end of file) on its first tick -- this TU cannot see its statics, and must not
//  reference ht9045::Pci1203Monitor() itself: that lives only in wb_serve's own sources, and cStateRecord.o (which calls
//  the writer) is linked into ctests (TaskListRegister) that do not compile the monitor.
//    1203 reader       (tick)        card health + every opened axis sample
//    dialog text slot  (tick)        g_alarmSlot / g_nextQid / g_dialogSeq (:322-345)
//    watchdog reader   (any thread)  g_wdPhase / g_wdSince / g_wdSeq under g_wdLock (:235-250)
//    mailbox directory (set once)    g_dialogMailboxDir (:4312)
//
//  CLAIM (3) -- Steven Q93 = A (1004 07:2x), St01 checked (3a)-(3d) 1004 11:27, on the laptop's hook list (St02-M 1005 11:4x):
//    the three modal-pending loops (tools/wb_serve.cpp :671 / :852 / :6797) and (3d) W906_ModalWaitTick's first line
//    (:7621) calling W906_SrDiagWbServeTick.  With (3d) applied the snapshot AND the S2 change scan keep running while a
//    blocking dialog is open; they pause during a blocking dialog only if hook (3d) is not applied (the files show the
//    snapshot age either way).  ctest T5.12 pins (3d).
//
//  NOT YET (each file names it):
//    claim (4) uhome.cpp:738-753 / :707-714 ProcessMotorHome's statics -- text slot kW906SrTextHomeFlags (NB2-1 to confirm).
//    claim (5) the end of WebMotorAccessLive.cpp, kBrakeAxes / g_brakeAxis -- text slot kW906SrTextBrake (after NB2-1's (m)
//              MR is on main).
//
//  FORMAT.  Plain text / CSV, UTF-8 without BOM (machine strings that are not valid UTF-8 are written as \xHH), CRLF.
//  Every .txt starts with ONE header line "# W906_<name> format=1 port-only (not golden)" followed by key=value lines in
//  a fixed order (a key is always present; an empty value = not available); the CSV has one header line, fixed columns.
//
//  ctest: St02_StateRecordDiag (tests/test_st02_staterecord_diag.cpp).
// =============================================================================
#ifndef W906_STATERECORDDIAG_H
#define W906_STATERECORDDIAG_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// AI(W906-S24-Q92) 20261004 (St02-E; Steven Q92 = A, 07:2x): the automatic hang record is ON -- StateRecordHang.cpp's monitor (started by
// tools/wb_serve.cpp's W906_SrDiagWbServeTick, claim (2)) writes one record per tick-loop stall of >= 60 s.  Read by
// W906_StateRecordAutoMonitorStart (false = it does not start).  The modal-pending exception (Q93) is claim (3).
const bool kW906StateRecordAutoOnHang = true;

// ---------------------------------------------------------------------------
//  The PCIE-1203 reader's output (filled on the tick thread by tools/wb_serve.cpp's reader)
// ---------------------------------------------------------------------------
struct W906SrDiag1203Sample
{
    int           slot;          // the monitor's axis index
    bool          opened, ambiguous;
    int           station, stationAxis;
    bool          valid;
    unsigned int  state;         // Acm_AxGetState (STA_AX_*)
    unsigned long motionIO;      // Acm_AxGetMotionIO raw (AX_MOTION_IO_*)
    double        cmdPos, actPos, cmdVel;
    unsigned long driveErr;
    std::string   driveErrText;
    bool          driveAlarmValid;
    unsigned int  driveAlarm;    // CoE 603Fh
    unsigned long homingSeenPoll;
    W906SrDiag1203Sample();
};

struct W906SrDiag1203
{
    bool          linked, monitor, open, disabled;
    std::string   disabledReason;
    unsigned long pollCount, pollErrors, pollMs;
    int           axesOpened;
    unsigned long lastError;
    std::string   lastErrorText;
    bool          idConflict;
    std::vector<W906SrDiag1203Sample> axes;
    W906SrDiag1203();
};

// ---------------------------------------------------------------------------
//  The snapshot (built on the tick thread, copied by readers)
// ---------------------------------------------------------------------------
struct W906SrDiagIoRow
{
    std::string kind;          // "SW" (output: value = the COMMAND, OutValue) or "Sen" (input: value = IsOn() on the tick thread)
    std::string constName;     // the cmydef.h constant, e.g. "SwMotorRelay"
    int         index;         // SW[] / Sen[] index; -1 = out of range (value then -1 too)
    std::string name;          // SW[].Name / Sen[].Name (IO_Table)
    bool        enable;
    int         type;
    int         isaBase;
    int         ring, ip, port, bit;
    int         value;         // 1 / 0; -1 = not read
    W906SrDiagIoRow() : index(-1), enable(false), type(0), isaBase(0), ring(0), ip(0), port(0), bit(0), value(-1) {}
};

struct W906SrDiagAxisRow
{
    std::string   source;      // "MotTable" (a Mot_Table row) or "MOT" (MOT[i] with Motor!=NULL && Enable that no row names)
    std::string   alias, no, cardModel;
    int           motIndex;    // "M07" -> 7; -1 = No is not M<digits>
    int           tableEnable; // Mot_Table Enable; -1 = no table row
    int           boardId, port;   // Mot_Table BoardID / Port; -1 = none
    bool          haveMot;     // motIndex is a valid MOT[] index: the MOT[] fields below are real
    bool          motorObj;    // MOT[].Motor != NULL
    bool          motorEnable; // MOT[].Motor->Enable (false without an object)
    int           homeFlag;    // MOT[].HomeFlag
    int           tHomeFlag, tHomeOrder;   // fHome->HomeClass[motIndex]->THomeFlag / THomeOrder; -1 = no slot
    bool          led[10];     // MOT[].Led[0..9] (Motor/HTMotor.h iCwLed .. iServoOn)
    int           cmdCache, targetCache, encCache;   // MOT[].Position / TargetPosition / EncoderPosition (engine caches; ReadPos() is NOT called)
    bool          canMove, canMoveR, canMoveM, canMoveL;
    int           lockCount;
    bool          is1203;      // CardModel == "PCI1203"
    int           slot1203;    // the monitor's axis slot; -1 = none (slotWhy says why)
    std::string   slotWhy;
    bool          sampleValid;
    unsigned int  state;
    unsigned long motionIO;
    double        cmdPos, actPos, cmdVel;
    unsigned long driveErr;
    std::string   driveErrText;
    bool          driveAlarmValid;
    unsigned int  driveAlarm;
    unsigned long homingSeenPoll;
    W906SrDiagAxisRow();
};

enum { kW906SrTextHomeFlags = 0, kW906SrTextBrake = 1, kW906SrTextDialogs = 2, kW906SrTextSlots = 3 };

// ---------------------------------------------------------------------------
//  S2 -- the IO list (W906_IO.csv) and the change ring (W906_Recent.csv)
// ---------------------------------------------------------------------------
enum { kW906SrIoSen = 0, kW906SrIoSw = 1, kW906SrIoCyl = 2, kW906SrIoSuck = 3 };
// the three addresses of a point: Sen = a (DI); SW = a (DO); Cylinder = a (out) b (on sensor) c (off sensor);
// sucker = a (suck valve) b (destroy valve) c (vacuum sensor)
enum { kW906SrIoAddrs = 3 };
// why a value was not read (W906SrDiagIoValue::why); 10 + n = MyLaneIO.CheckPortRangeErr answered n
enum { kW906SrWhyRead = 0, kW906SrWhyUnused = 1, kW906SrWhyDisabled = 2, kW906SrWhyNoAddress = 3, kW906SrWhyNoX64 = 4,
       kW906SrWhyIsaBase = 5, kW906SrWhyVc8Gate = 6, kW906SrWhyRing0 = 7, kW906SrWhyRoute = 8, kW906SrWhyStub0 = 9,
       kW906SrWhyRange = 10 };

struct W906SrDiagIoAddr
{
    bool        used;          // this point has the address at all
    bool        out;           // an output (the output cache is read) or an input
    bool        enable;        // the owner's enable for this address (SW.Enable, Cylinder.OnSenEnable, sucker OnEnable ...)
    std::string name;          // the IO_Table name of the address (Sen.Name, Cylinder.OnSensorName, sucker OnPortName ...)
    int         ioRow;         // HSys.mapIOTable[name]; -1 = not in IO_Table
    int         isaBase, ring, ip, port, bit, type;
    W906SrDiagIoAddr() : used(false), out(false), enable(false), ioRow(-1), isaBase(0), ring(0), ip(0), port(0), bit(0), type(0) {}
};

struct W906SrDiagIoPoint
{
    int         kind;          // kW906SrIoSen ...
    int         index;         // Sen[] / SW[] / Cylinder[] index; sucker: kit * 32 + row * 8 + col
    std::string kit;           // sucker: the TMyKitSuck variable ("InArmSuck" ...) + "[row][col]"; else empty
    std::string name;          // Sen.Name / SW.Name / CylinderName / SuckerName
    bool        enable;        // Sen / SW / Cylinder / sucker Enable
    W906SrDiagIoAddr a[kW906SrIoAddrs];
    W906SrDiagIoPoint() : kind(0), index(-1), enable(false) {}
};

struct W906SrDiagIoValue       // POD: copied into every snapshot
{
    signed char   raw[kW906SrIoAddrs];      // the bit read (input) or the output cache bit (output); -1 = not read
    unsigned char why[kW906SrIoAddrs];      // kW906SrWhy*
    short         rc[kW906SrIoAddrs];       // the route's return code (kW906SrWhyRoute) or the range-check code
    signed char   cmd;                      // SW OutValue / Cylinder Status / sucker Status; -1 = none (Sen)
    signed char   cmd2;                     // Cylinder bCylinderOn; -1 = none
    bool          kPaValid;                 // sucker vacuum (snapshot builds only, VC8-registered suckers only)
    double        kPa;
};

struct W906SrDiagIoList        // immutable once published (shared by the snapshots)
{
    unsigned long long gen;    // 1, 2, ... one per (re)build
    unsigned long      builtTick;
    std::string        builtAt;
    unsigned long      signature;
    std::vector<W906SrDiagIoPoint> points;
    int                axes;   // MOT[] axes the change scan watches
    W906SrDiagIoList() : gen(0), builtTick(0), signature(0), axes(0) {}
};

// one change (W906_Recent.csv row)
enum { kW906SrChgSen = 0, kW906SrChgSw = 1, kW906SrChgCyl = 2, kW906SrChgSuck = 3, kW906SrChgAxis = 4, kW906SrChgDrive = 5,
       kW906SrChgNote = 6 };
struct W906SrDiagChange
{
    unsigned long  tick;       // GetTickCount() of the scan
    unsigned char  hh, mm, ss;
    unsigned short ms;
    unsigned char  kind;       // kW906SrChg*
    unsigned char  field;      // W906SrDiagChangeFieldName
    int            index;      // the point's / axis's index (as in W906_IO.csv / MOT[])
    int            oldV, newV;
    bool           haveCaches; // an axis: MOT[].Position / EncoderPosition at the change
    int            cmdCache, encCache;
    bool           havePos;    // a drive row: the 1203 sample's cmdPos / actPos
    double         cmdPos, actPos;
    char           name[48];   // the point / axis name (truncated)
};
const char* W906SrDiagChangeKindName(int kind);
const char* W906SrDiagChangeFieldName(int field);

struct W906SrDiagRecent        // the writer's copy of the ring: oldest first, only the last windowMs, at most cap rows
{
    std::vector<W906SrDiagChange> rows;
    unsigned long      nowTick;
    unsigned int       cap, windowMs;
    std::size_t        inRing;             // entries in the ring when copied (any age)
    unsigned long long total;              // changes recorded since start
    unsigned long long droppedByCap;       // overwritten because the ring was full
    W906SrDiagRecent() : nowTick(0), cap(0), windowMs(0), inRing(0), total(0), droppedByCap(0) {}
};

const std::size_t   kW906SrRecentCap = 20000;
const unsigned int  kW906SrRecentWindowMs = 600000;
const unsigned int  kW906SrChangeScanMs = 100;
const unsigned int  kW906SrIoSignatureMs = 1000;
const std::size_t   kW906SrStageMax = 256;     // changes staged per scan before a flush into the ring

struct W906SrDiagSnapshot
{
    unsigned long long seq;            // 0 = nothing published yet
    unsigned long      tickThread;     // GetCurrentThreadId() of the thread that built it
    unsigned long      builtTick;      // GetTickCount() when built
    std::string        builtAt;        // local "yyyy-mm-dd hh:mm:ss.mmm"
    unsigned long      buildUs;        // how long the build took (microseconds)
    // MainProc monitor (csystem.cpp:534-570)
    unsigned int  mainProcCalls;
    std::string   mainProcLastEnter;
    double        mainProcSilentSec;
    bool          mainProcAlive10;
    // PCIE-1203 monitor, card level (p1203Reader = the wb_serve reader is installed and filled the rest)
    bool          p1203Reader;
    W906SrDiag1203 p1203;              // axes left empty here: they are matched into `axes` below
    // HOME
    bool          homeObj;             // fHome != NULL (the fields below are real)
    int           homeStep;
    bool          homeShow, homeAbort;
    int           testZTask;
    bool          allMotorHome, systemStart, softStart, softStop, autoShuttleHome;
    int           homeClassCount;
    std::vector<std::string> homeProgress;   // fHome->ListBox1, newest first, at most 10
    // power / brake
    bool          motorPowerState;
    int           motorPowerOnDelay;
    bool          emgPressed;
    std::vector<W906SrDiagIoRow> io;
    // dialogs
    bool          noteObj, noteShow;
    int           noteAlarmType;
    bool          mboxObj, mboxShow, mboxVisible;
    // axes
    std::vector<W906SrDiagAxisRow> axes;
    // the text slots (built by the owner's reader on the tick thread)
    bool          textHooked[kW906SrTextSlots];
    std::string   text[kW906SrTextSlots];
    // S2: the IO list (shared, rebuilt only when the IO configuration changes) and its values at this build
    std::shared_ptr<const W906SrDiagIoList> ioList;
    std::vector<W906SrDiagIoValue>          ioValues;   // parallel to ioList->points
    bool          ioRoute1203;         // a non-SIM build reads PCIE-1203 inputs through Pci1203IoRoute(): installed?
    W906SrDiagSnapshot();
};

// The module's own counters (not part of the snapshot: they move on every pass, also when no snapshot is built).
struct W906SrDiagModuleInfo
{
    unsigned long      tickThread;     // 0 = no publish yet
    unsigned long long publishCalls;   // calls from the tick thread
    unsigned long long snapshotsBuilt;
    unsigned long long refused;        // calls from another thread (refused)
    bool               havePass;
    unsigned long      lastPassTick;   // GetTickCount() of the last call
    std::string        lastPassAt;     // local time of the last call
    unsigned long      lastGapMs, maxGapMs;
    unsigned int       minIntervalMs;
    // S2: the change scan and the ring
    unsigned int       changeScanMs;
    unsigned long long ioScans;
    unsigned long      ioScanLastUs, ioScanMaxUs;
    unsigned long long ioScanSumUs;
    unsigned long long ioListGen, ioListRebuilds;
    unsigned long      ioListPoints, ioListAxes;
    std::size_t        recentCap, recentInRing;
    unsigned long long recentTotal, recentDroppedByCap;
    W906SrDiagModuleInfo() : tickThread(0), publishCalls(0), snapshotsBuilt(0), refused(0), havePass(false),
                             lastPassTick(0), lastGapMs(0), maxGapMs(0), minIntervalMs(0),
                             changeScanMs(0), ioScans(0), ioScanLastUs(0), ioScanMaxUs(0), ioScanSumUs(0), ioListGen(0),
                             ioListRebuilds(0), ioListPoints(0), ioListAxes(0), recentCap(0), recentInRing(0), recentTotal(0),
                             recentDroppedByCap(0) {}
};

struct W906SrDiagWatchdogInfo
{
    bool               hooked;         // a reader is installed (tools/wb_serve.cpp end of file)
    bool               ok;             // and it answered
    std::string        phase;
    unsigned long      sinceTick;
    unsigned long long seq;
    W906SrDiagWatchdogInfo() : hooked(false), ok(false), sinceTick(0), seq(0) {}
};

struct W906SrDiagMailboxInfo
{
    std::string dir;                   // "" = not handed over
    std::string state;                 // "not-set" / "ok" / "missing"
    int         copied, skipped;
    W906SrDiagMailboxInfo() : state("not-set"), copied(0), skipped(0) {}
};

struct W906SrDiagWriteContext
{
    std::string   trigger, writtenAt, build, exePath, exeTime;
    unsigned long nowTick;
    unsigned long writerThread;
    W906SrDiagWriteContext() : nowTick(0), writerThread(0) {}
};

// ---------------------------------------------------------------------------
//  Tick side
// ---------------------------------------------------------------------------
enum { kW906SrPublishRefused = -1, kW906SrPublishSkipped = 0, kW906SrPublishBuilt = 1 };
int  W906_StateRecordDiagPublish();                         // TICK THREAD ONLY, once per pass (claim ②, via W906_SrDiagWbServeTick)
int  W906_StateRecordDiagPublishNow();                      // TICK THREAD ONLY, ignores the minimum interval (claim ①)
void W906_StateRecordDiagSetMinIntervalMs(unsigned int ms); // default 250; 0 = build on every call
void W906_StateRecordDiagSetChangeScanMs(unsigned int ms);  // S2: default 100; 0 = scan on every call
void W906_StateRecordDiagSetRecentCap(std::size_t cap);     // S2 (tests): default kW906SrRecentCap; clears the ring

// Readers (null = not installed).  The 1203 reader and the text readers run INSIDE Publish on the tick thread (they may
// read tick-owned state); the watchdog reader runs on the WRITER's thread and must lock what it reads itself.
typedef void (*W906SrDiag1203Fn)(W906SrDiag1203& out);
void W906_StateRecordDiagSet1203Reader(W906SrDiag1203Fn fn);
typedef std::string (*W906SrDiagTextFn)();
void W906_StateRecordDiagSetTickText(int slot, W906SrDiagTextFn fn);
typedef bool (*W906SrDiagWatchdogFn)(std::string& phase, unsigned long& sinceTick, unsigned long long& seq);
void W906_StateRecordDiagSetWatchdogReader(W906SrDiagWatchdogFn fn);
void W906_StateRecordDiagSetMailboxDir(const std::string& dir);   // the dialog mailbox (*.json copied into W906_Mailbox\)

// ---------------------------------------------------------------------------
//  Any thread
// ---------------------------------------------------------------------------
W906SrDiagSnapshot   W906_StateRecordDiagRead();            // a copy of the last snapshot (seq 0 = none yet)
W906SrDiagModuleInfo W906_StateRecordDiagInfo();
// S2: the ring's last windowMs as of nowTick (at most cap rows, oldest first; the copy under the lock is bounded by the cap)
W906SrDiagRecent     W906_StateRecordDiagRecent(unsigned long nowTick);

struct W906SrDiagWriteOptions
{
    std::string trigger;
    bool        opLogDirSet;        // false = %W906_OPLOG_DIR%; true = opLogDir ("" = as if not set)
    std::string opLogDir;
    bool        nowSet;             // false = GetLocalTime / GetTickCount; true = the fields below (tests)
    int         nowY, nowMo, nowD, nowH, nowMi, nowS, nowMs;
    unsigned long nowTick;
    std::size_t opLogMaxLines, opLogMaxBytes;
    W906SrDiagWriteOptions();
};
// Writes the nine W906_* files (+ W906_Mailbox\ when the mailbox directory is known) into `dir` (created when missing).
// Returns how many of the nine were written (kW906SrDiagFileCount = all); -1 = the directory could not be created.
// Nothing else in `dir` is touched.
int W906_StateRecordDiagWriteFiles(const std::string& dir, const char* trigger);
int W906_StateRecordDiagWriteFilesEx(const std::string& dir, const W906SrDiagWriteOptions& o, std::string* why);

const int kW906SrDiagFileCount = 9;                         // AI(W906-S24-S2) 20261005 (St02-E): was 7; + W906_IO.csv, W906_Recent.csv
extern const char* const kW906SrDiagFiles[kW906SrDiagFileCount];   // the names, W906_ReadMe.txt first
extern const char* const kW906SrDiagMotorCsvHeader;
extern const char* const kW906SrDiagIoCsvHeader;
extern const char* const kW906SrDiagRecentCsvHeader;
const char* const kW906SrDiagMailboxSubdir = "W906_Mailbox";
const std::size_t kW906SrOpLogTailMaxLines = 3000;
const std::size_t kW906SrOpLogTailMaxBytes = 1024u * 1024u;
const int         kW906SrMailboxMaxFiles = 64;
const unsigned long kW906SrMailboxMaxBytes = 1024ul * 1024ul;

// ---------------------------------------------------------------------------
//  Pure formatters (ctest T1)
// ---------------------------------------------------------------------------
std::string W906SrDiagSafe(const std::string& s);           // control chars -> ' ', invalid UTF-8 bytes -> \xHH
std::string W906SrDiagCsvField(const std::string& s);       // Safe + quoted when it holds , " or edge spaces
const char* W906SrDiagAxisStateText(unsigned int state);    // STA_AX_* -> "READY" ...; "?" otherwise
std::string W906SrDiagFormatReadMe(const W906SrDiagSnapshot& s, const W906SrDiagModuleInfo& m, const W906SrDiagWriteContext& c);
std::string W906SrDiagFormatHealth(const W906SrDiagSnapshot& s, const W906SrDiagModuleInfo& m, const W906SrDiagWatchdogInfo& wd,
                                   const W906SrDiagWriteContext& c);
std::string W906SrDiagFormatMotorCsv(const W906SrDiagSnapshot& s);
std::string W906SrDiagFormatHome(const W906SrDiagSnapshot& s);
std::string W906SrDiagFormatPowerBrake(const W906SrDiagSnapshot& s);
std::string W906SrDiagFormatDialogs(const W906SrDiagSnapshot& s, const W906SrDiagMailboxInfo& mb);
// chunk = the last bytes of the op log (fromFileStart = the chunk is the whole file): drops a BOM at the file start or the
// partial first line otherwise, keeps the last maxLines lines, each through W906SrDiagSafe and ended with CRLF.
std::string W906SrDiagTailLines(const std::string& chunk, bool fromFileStart, std::size_t maxLines, std::size_t* nLines);
std::string W906SrDiagFormatOpLogTail(const std::string& source, const char* state, unsigned long long fileBytes,
                                      const std::string& tailText, std::size_t tailLines, std::size_t maxLines,
                                      std::size_t maxBytes);
std::string W906SrDiagFormatIoCsv(const W906SrDiagSnapshot& s);                  // S2
std::string W906SrDiagFormatRecentCsv(const W906SrDiagRecent& r);                // S2
const char* W906SrDiagWhyText(int why);                                         // S2: kW906SrWhy* -> "disabled" ...

// ---------------------------------------------------------------------------
//  StateRecordHang.cpp -- the hang route (any thread; claim ② routes GET /api/struct/staterecord.hang here).
//  Starts a worker that writes the W906_* files from the LAST published snapshot (and disk files) into
//  <root><yyyymmdd_hhmmss>_hang\ and answers JSON; waits for it at most waitMs.  It does not run golden's DoStateRecord,
//  copy the golden log set or zip (those need the tick).  One at a time; at most one per minGapMs.
// ---------------------------------------------------------------------------
std::string W906_StateRecordHangRequest(const std::string& query);   // root D:\HT9045_StateRecord\ (golden SDataPath), 5 s gap, 1.5 s wait
std::string W906_StateRecordHangRequestAt(const std::string& root, const std::string& trigger, unsigned int minGapMs,
                                          unsigned int waitMs, std::string* folderOut);

// tests only: unbind the tick thread, clear the snapshot / counters / readers / mailbox dir, interval back to 250 ms;
// S2: the IO list, the change scan state and the ring cleared, scan interval 100 ms, cap kW906SrRecentCap
void W906_StateRecordDiagTestReset();

// ---------------------------------------------------------------------------
//  AI(W906-S24-Q92) 20261004 (St02-E; Steven Q92 = A, 07:2x): the automatic hang record (StateRecordHang.cpp).  NOT golden.
//  A STALL = the tick thread has not published for thresholdMs (60 s): it publishes once per main-loop pass (claim (2))
//  and, with claim (3), once per blocking-dialog wait pass -- and the watchdog phase is not a blocking dialog's wait
//  ("modal wait ..." / "yes/no wait ..."): a box waiting for the operator is not a stall.  ONE record per stall: keyed
//  by the lastPassTick the stall began from; a new pass ends the stall.  The record is the hang route's (W906_* files
//  from the last snapshot, any thread), trigger "auto-stall", root D:\HT9045_StateRecord\.
// ---------------------------------------------------------------------------
W906SrDiagWatchdogInfo W906_StateRecordDiagWatchdog();          // the installed watchdog reader, called now (any thread)
void W906_StateRecordAutoMonitorStart();                         // once (later calls do nothing); a 1 s polling thread
bool W906_StateRecordAutoStallStepAt(const std::string& root, unsigned long nowTick, bool havePass, unsigned long lastPassTick,
                                     const std::string& wdPhase, unsigned int thresholdMs, unsigned int waitMs,
                                     std::string* folderOut);    // one monitor step; true = a record was started
unsigned long W906_StateRecordAutoCount();                       // records started so far
void W906_StateRecordAutoTestReset();                            // tests only

// AI(W906-S24-Q93) 20261004 (St02-E; Steven Q93 = A, St01 OK 11:27): the State Record button while a blocking box waits ([W906] exception).  The three
// waits (tools/wb_serve.cpp :670 / :855 / :6797) ask W906_StateRecordDuringDialog (wb_serve end of file) before their
// S-17 reply; it serves a command only when this says yes -- act.main.stateRecord and nothing else, so dialog.notifyAck /
// modal.answer / dialog.response ... still reach the S-17 path untouched (ctest St02_StateRecordDiag T7).
bool W906_StateRecordDialogTakes(const std::string& cmd);

#endif // W906_STATERECORDDIAG_H
