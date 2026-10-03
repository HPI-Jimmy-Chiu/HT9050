// =============================================================================
//  Motor/EcatMotorRoute.h -- the ENGINE's PCIE-1203 axes, routed onto the card.
//
//  AI(W906-ECAT-ROUTE) 20260929: new file. INBOX 112, RULINGS_20260929 #11
//  decision 1 = A ("the laptop takes the engine -> 1203 motor route now"),
//  design docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md (sections 0-8; Q1-Q11 at the
//  document's recommended defaults, approved by the user).
//
//  WHY IT EXISTS
//      ht9045_motor is built WITHOUT HAVE_PCI1203 on purpose (CMakeLists.txt,
//      the BU-C-P5 / Q34-ARM blocks: only wb_serve carries the flag), so every
//      TMyEtherCatMotor method takes its `#else` arm and an engine (MainProc)
//      move of a 1203 axis never reaches the card. Same shape as the IO route
//      (IOBackend.h EOF, EtherCAT/Pci1203IoRoute.cpp): the `#else` arms ask an
//      INSTALLABLE route, and the process that owns the card (wb_serve) installs
//      one -- EtherCAT/Pci1203MotorRoute.cpp:
//        reads  = the 1203 monitor's samples (Pci1203AxisSample)  -- no vendor call
//        writes = TPci1203Control::Execute (EastSun's layer: allowlist, DRY/LIVE,
//                 audit log) on the monitor's own axis handles    -- no second open
//
//  THE CONTRACT THAT KEEPS THE TEST BASELINE
//      * No route installed (every ctest executable, every SOFT_SIMULTE build,
//        every build without WB_ENGINE_MOTOR_1203) = TMyEtherCatMotor behaves
//        bit for bit as before: W906_EcCall returns kEcRcNoRoute with no side
//        effect, W906_EcRead returns false, and every lazy claim / guard reads
//        exactly what it read before (design section 7.5, one row per line).
//      * This header includes NO EtherCAT header and nothing from the machine
//        model: plain POD + function pointers, so ht9045_motor gains no new
//        dependency.
//      * The route sits BELOW golden's unit conversions (TMyMotor::GetRealPos,
//        TMyEtherCatMotor::ReadPos, SetSpeed): it receives the very numbers the
//        HAVE_PCI1203 arm hands to Acm_*, in card units (pulse), and converts
//        nothing but the jog / home direction codes (design section 1.3).
//      * The route NEVER calls ShowErrorMessage (design section 4.4: in wb_serve
//        it re-enters StopAllMotor -> DecStop -> the route, unbounded). A failure
//        is a non-zero return + Pci1203Control's audit line + one rate-limited
//        console line.
//
//  W906_EC_ONLY(...)
//      Expands to its argument in the build this tree ships (HAVE_PCI1203 off)
//      and to NOTHING when HAVE_PCI1203 is on (the ht9045_pci1203_probe target
//      compiles Motor/myEthercatmotor.cpp that way on every build). It wraps the
//      route calls that sit on lines shared by both arms, so the HAVE arm's text
//      stays exactly golden's and both arms keep being type-checked.
//      ⚠ Only the macro depends on HAVE_PCI1203. The inline functions below do
//      NOT, and must not: wb_serve compiles this header WITH the flag on an SDK
//      machine while ht9045_motor compiles it without, and one inline function
//      with two bodies across TUs is an ODR violation that links silently.
// =============================================================================
#ifndef MOTOR_ECATMOTORROUTE_H
#define MOTOR_ECATMOTORROUTE_H

#if defined(HAVE_PCI1203) && (HAVE_PCI1203 + 0)
#define W906_EC_ONLY(...)
#else
#define W906_EC_ONLY(...) __VA_ARGS__
#endif

// The monitor's most recent sample for one axis, as the route hands it back.
// NOT a vendor read: at most one monitor Poll old (200 ms on wb_serve's IO clock).
struct TEcatAxisRead {
    bool           valid;      // a unique opened monitor slot has a valid sample
    bool           pending;    // design 4.2: a state-changing command has not been seen by a Poll yet,
                               //   or a DRY / refused / failed MOTION has not been followed by a stop (Q11)
    unsigned short state;      // Pci1203AxisSample::state    (STA_AX_*, raw)
    unsigned long  motionIO;   // Pci1203AxisSample::motionIO (AX_MOTION_IO_* bits, raw)
    double         cmdPos;     // Pci1203AxisSample::cmdPos   (pulse)
    double         actPos;     // Pci1203AxisSample::actPos   (pulse)
    TEcatAxisRead() : valid(false), pending(false), state(0), motionIO(0), cmdPos(0.0), actPos(0.0) {}
};

// STA_AX_* values the port compares against (EtherCAT/vendor/AdvMotDrv.h; WebMotorAccess.cpp IsReadyState...).
enum { kEcStaReady = 1, kEcStaErrorStop = 3, kEcStaHoming = 4 };

// What TMyEtherCatMotor asks for -- one entry per vendor call family of its HAVE arm.
enum TEcatOp {
    kEcStopDec = 1,   // Acm_AxStopDec            -> kCmdAxStop
    kEcStopEmg,       // Acm_AxStopEmg            -> kCmdAxEmgStop
    kEcExtDrive,      // Acm_AxSetExtDrive(v)     -> kCmdAxSetExtDrive   v = 0 / 1
    kEcJog,           // Acm_AxJog(vendor dir)    -> kCmdAxJogStart      v = VENDOR code 0 (POS) / 1 (NEG)
    kEcMoveAbs,       // Acm_AxMoveAbs(v)         -> kCmdAxMoveAbs       v = pulse
    kEcMoveRel,       // Acm_AxMoveRel(v)         -> kCmdAxMoveRel       v = pulse
    kEcSetSpeed,      // Acm_SetF64Property       -> kCmdAxSetSpeed      which = kEcSpd*
    kEcSetLimit,      // Acm_SetF64Property       -> kCmdAxSetLimit      which = kEcLim*
    kEcSvOn,          // Acm_AxSetSvOn(v)         -> kCmdAxSvOn          v = 0 / 1
    kEcResetError,    // Acm_AxResetError         -> kCmdAxResetError
    kEcSetCmdPos,     // Acm_AxSetCmdPosition(v)  -> kCmdAxSetCmdPos     (refused on a DS402 axis, Q1)
    kEcSetActPos      // Acm_AxSetActualPosition  -> kCmdAxSetActPos     (refused on a DS402 axis, Q1)
};
// kEcSetSpeed's `which`, in Pci1203SpeedParam order (the route maps them one to one).
enum { kEcSpdInit, kEcSpdRun, kEcSpdAcc, kEcSpdDec, kEcSpdJogInit, kEcSpdJogRun, kEcSpdJogAcc,
       kEcSpdJogDec, kEcSpdMaxVel, kEcSpdMaxAcc, kEcSpdMaxDec };
// kEcSetLimit's `which`.        -> kLimitSwPelValue / kLimitSwMelValue
enum { kEcLimSwPel, kEcLimSwMel };

// Route return codes (0 = SUCCESS). 0x7E prefix = not an Advantech range, same family as
// EtherCAT/Pci1203IoRoute.cpp's 0x7E00000x codes.
enum {
    kEcRcNoRoute    = 0x7E000101,   // no route installed (the default everywhere but an armed wb_serve)
    kEcRcNotClaimed = 0x7E000102,   // Open_Axis never claimed this (station, axis) = golden's handle 0
    kEcRcNoControl  = 0x7E000103,   // no armed TPci1203Control
    kEcRcNoCard     = 0x7E000104,   // monitor absent / card not open / monitor Disabled
    kEcRcNoAxis     = 0x7E000105,   // 0 or 2+ opened monitor slots for (station, axis) -- never picks
    kEcRcRefused    = 0x7E000106,   // Execute refused it (the audit line says why)
    kEcRcDs402Coord = 0x7E000107,   // Q1: a coordinate write on a DS402 (or unidentified) drive
    kEcRcBadOp      = 0x7E000108    // an op / which the route does not know
};

// The route. All entries are keyed by golden's (iBoardID, iPortID) = the monitor's (station, stationAxis),
// never by a monitor slot (Rescan renumbers slots, EtherCAT/Pci1203IoRoute.cpp:67-69).
struct TEcatMotorRoute {
    bool          (*bind)(int board, int port, int motorId, bool direction);   // Open_Axis: CLAIM, never open. Direction=1 refused (Q5)
    bool          (*read)(int board, int port, TEcatAxisRead* out);
    unsigned long (*call)(int board, int port, int op, int which, double v);   // 0 = SUCCESS
    bool          (*initCfg)(int board, int port, int motorClass, bool sensorType, bool in1Logic,
                             double maxVel, double maxAcc, double maxDec);     // InitMotor's ladder + table (design 1.2)
    int           (*homeStart)(int board, int port, bool homeDir, double hi, double lo, double acc, double dec);   // 1 = issued
    int           (*homeDone)(int board, int port);                              // 1 done, 0 not yet
    bool          (*homeCardSide)(int board, int port);                          // last homeStart was card-side (zero after)
};

// The one route pointer, in an inline function's static: one object program-wide (C++17 inline
// linkage), no .cpp needed -- ht9045_motor and wb_serve's route TU share it.
// AI(W906-ENG1203) 20260929: (review build #6: a function-local static of an inline function is one object
// program-wide since C++98 -- which is why the C++14 lanes work too.) The only production writer of this slot is
// W906_InstallPci1203MotorRoute (pci1203_control_gate.ps1 check 6 counts SetEcatMotorRoute( / W906_EcRouteSlot_().
inline const TEcatMotorRoute*& W906_EcRouteSlot_() { static const TEcatMotorRoute* s = 0; return s; }
inline void SetEcatMotorRoute(const TEcatMotorRoute* r) { W906_EcRouteSlot_() = r; }   // 0 = back to the stub arms
inline const TEcatMotorRoute* EcatMotorRoute() { return W906_EcRouteSlot_(); }

inline unsigned long W906_EcCall(int board, int port, int op, int which, double v)
{
    const TEcatMotorRoute* r = EcatMotorRoute();
    return (r && r->call) ? r->call(board, port, op, which, v) : (unsigned long)kEcRcNoRoute;
}
inline bool W906_EcRead(int board, int port, TEcatAxisRead& s)
{
    s = TEcatAxisRead();
    const TEcatMotorRoute* r = EcatMotorRoute();
    return r && r->read && r->read(board, port, &s) && s.valid;
}
// TMyEtherCatMotor::SetSpeed's HAVE arm (golden :618-731), in its order: VelLow, VelHigh, Acc, Dec, and with
// bSetJog the four CFG_AxJog* with the same values. ⚠ GAP: golden's last jog write CFG_AxJogVLTime=0 is an I32
// property and Pci1203Control has no I32 setter (the same gap WebMotorAccess.cpp:568-569 records) -- not sent.
inline void W906_EcSetSpeed(int board, int port, double velLow, double velHigh, double acc, double dec, bool jog)
{
    W906_EcCall(board, port, kEcSetSpeed, kEcSpdInit, velLow);
    W906_EcCall(board, port, kEcSetSpeed, kEcSpdRun,  velHigh);
    W906_EcCall(board, port, kEcSetSpeed, kEcSpdAcc,  acc);
    W906_EcCall(board, port, kEcSetSpeed, kEcSpdDec,  dec);
    if (!jog) return;
    W906_EcCall(board, port, kEcSetSpeed, kEcSpdJogInit, velLow);
    W906_EcCall(board, port, kEcSetSpeed, kEcSpdJogRun,  velHigh);
    W906_EcCall(board, port, kEcSetSpeed, kEcSpdJogAcc,  acc);
    W906_EcCall(board, port, kEcSetSpeed, kEcSpdJogDec,  dec);
}
// motorClass: 0 = golden Servo_Motor arm, 1 = Rotate_Motor, 2 = otherwise (Pci1203Control.h kInitCfgMotor*).
inline bool W906_EcInitCfg(int board, int port, int motorClass, bool sensorType, bool in1Logic,
                           double maxVel, double maxAcc, double maxDec)
{
    const TEcatMotorRoute* r = EcatMotorRoute();
    return r && r->initCfg && r->initCfg(board, port, motorClass, sensorType, in1Logic, maxVel, maxAcc, maxDec);
}
inline int W906_EcHomeStart(int board, int port, bool homeDir, double hi, double lo, double acc, double dec)
{
    const TEcatMotorRoute* r = EcatMotorRoute();
    return (r && r->homeStart) ? r->homeStart(board, port, homeDir, hi, lo, acc, dec) : 0;
}
inline int W906_EcHomeDone(int board, int port)
{
    const TEcatMotorRoute* r = EcatMotorRoute();
    return (r && r->homeDone) ? r->homeDone(board, port) : 0;
}
inline bool W906_EcHomeCardSide(int board, int port)
{
    const TEcatMotorRoute* r = EcatMotorRoute();
    return r && r->homeCardSide && r->homeCardSide(board, port);
}

// AI(W906-ENG1203) 20260929: review HIGH-1 -- golden PCIL132_StopMotor's bookkeeping for a stop that did NOT go
// through the route (WebMotorAccess's Stop1203 / Stop1203All, the pci1203 page's ax.stop): `fCMD=false` on every
// MOT[] row whose motor is the TMyEtherCatMotor at (board, port), under PCIL132_StopMotor's own guard (Motor != NULL,
// Enable, not one of the four Index names -- golden mymotor.cpp:1859-1874; those rows never move through the route, Q4).
// Without it golden MotorMovePosition takes the next READY after that stop as ARRIVAL (fCMD still true,
// mymotor.cpp:5743-5745) although the axis stopped short; with it, the engine compares the position and re-issues,
// exactly what it does after golden's own stop. Returns the rows reset. No route installed = 0, nothing touched.
// Defined in Motor/EcatMotorRoute.cpp (ht9045_motor); called by the wb_serve glue W906_EngineRouteForeignStop.
int W906_EcForeignStopResetFcmd(int board, int port);

// AI(W906-INDEXZ) 20260930: one owner per axis (INBOX 113). How many MOT[] rows hold a TMyEtherCatMotor built for
// (board, port) -- the only objects whose Open_Axis can make the engine motor route CLAIM that 1203 axis. The Index Z1
// route (EtherCAT/Pci1203GaliRoute.cpp) refuses to install when this is not 0 for the M14 row's address. On HT9050
// with INDEX_MOTION_CARD==0 the answer for (14, 0) is 0: MOT[MTestZ1] is a TMyGALILMotor (cinitial.cpp Galil arm)
// (tests/test_machine_motors.cpp part (5)). Pure MOT[] read, route installed or not.
int W906_EcEngineMotorsAt(int board, int port);

#endif  // MOTOR_ECATMOTORROUTE_H
