// =============================================================================
//  IndexZFinePitchCore.h -- ST01-C (W-77) slice 1 (SAFETY): the PURE rules of the protections that
//  atester_FinePitch.cpp (HT9050 Index FinePitch flow, 910 DoTestHeadMotorFP) adds to the 910 text.
//  No globals, no VCL, no vendor header -- both build arms, unit-tested by tests/test_fp9050_index.cpp
//  (ctest FP9050_Pure).
//
//  AI(W906-ST01C) 20261005 (St01): NEW FILE, NOT 910 / NOT GOLDEN. Plan: D:\AI_TempFile\st01e-c-fp-plan-20261005.md
//  slice 1 (Q4b / Q5 defaults; Steven has not answered yet).
// =============================================================================
#ifndef IndexZFinePitchCoreH
#define IndexZFinePitchCoreH

namespace ht9045 {
namespace fp9050 {

// ---- W-44 (Steven; RULINGS_20261005 #12 makes it part of batch C; definition Steven 1005 23:1x): Index Z1 may press to the
//      SOCKET while the shuttles' X are OUTSIDE the index's safe zone (「在安全位置之外,index就可以下壓到socket」; the Out shuttle may be
//      busy with the CCD five-side inspection) -- the decision is Frank's guard (FR-NB2 (2), RULINGS #19), called through
//      W906_Ht9050ShuttlesClearOfIndexHook (atester_FinePitch.h).  This is WHICH ticks are checked: the press-down and hold tasks of
//      the two FP state machines that move Z1 towards / hold it at the SOCKET.  Never a retreat (a lift must always be
//      allowed), never a pick / place descent onto a shuttle (the shuttle must be UNDER the index there, as E-042 B3).
//      910 itself only gates the production press once, at DoTestYFrontFP case 130 (InSHT1InLF() && OutSHT1InRT()); the
//      index check press of DoTestHeadMotorFP (12101) and the drop (40) have no shuttle check at all in 910.
enum { kSmHeadMotor = 0, kSmTestYFront = 1, kSmCount = 2 };
//  kSmHeadMotor = DoTestHeadMotorFP (910 :1983-3039):
//    40 (Z1 down to TestZ1_Test-TestZ1_Drop_Offset to drop an IC), 50 / 55 (held there);
//    12100 (falls through into 12101 in the SAME tick -- without it the 12101 descent of the tick that enters 12100 would
//           be issued unchecked; the 12110 recount loop also re-enters here), 12101 (the index-check press into the empty
//           socket), 12110 (torque wait, pressed), 12200 / 12300 / 121 / 122100 (still pressed at TestZ1_Test-Drop_Offset,
//           no lift until 122110 / 123 / 15000), 122110 (socket-check height), 122 (held there), 130 (one dispatch tick,
//           still down, before 15000 lifts).
//    Plan Q5 listed 40 / 50 / 55 / 12101 / 12110 / 122110 / 122; 12100 / 12200 / 12300 / 121 / 122100 / 130 are added by
//    the plan's own rule ("press-down / hold tasks") after tracing where Z1 is -- reported for Steven's Q5 answer.
//    NOT checked: 1 / 11 / 9 / 2 / 15000 / 1600 / 12111 / 123 / 20100 / 20400 / 21400 / 40200 (safe height or a lift),
//    12000 / 12112 / 124 / 125 / 1550 / 1700-1720 / 600 (Z1 at safe; 600 = DoTestYFinePitch, checked inside).
//  kSmTestYFront = DoTestYFrontFP (910 :1423-1840):
//    180 (DoTestZContactModeStart: the production press to TestZ1_Test), 200 / 208 / 209 / 210 (pressed, testing),
//    300 / 310 / 330 (direct / drop contact at TestZ1_Test-TestZ1_Drop_Offset).
//    NOT checked: 1000 / 1100 / 1500 (lifts), 109 / 125 (to safe), 110 / 120 (pick from the In shuttle), 1600 / 1700
//    (place on the Out shuttle), 130 (its own entry check, refuse-only), 1 / 100 / 5000.
inline bool W44PressTaskFP(int sm, int task)
{
    if (sm == kSmHeadMotor) {
        switch (task) {
            case 40: case 50: case 55:
            case 12100: case 12101: case 12110: case 12200: case 12300:
            case 121: case 122100: case 122110: case 122: case 130:
                return true;
            default:
                return false;
        }
    }
    if (sm == kSmTestYFront) {
        switch (task) {
            case 180: case 200: case 208: case 209: case 210: case 300: case 310: case 330:
                return true;
            default:
                return false;
        }
    }
    return false;
}

// ---- Q4b (Steven, default "yes, message only"): 910 MoveIndexZ refuses every descent below Prod.All_TestZ_Test_Safe
//      unless M108 MCCDY reads exactly 0 with its home LED lit -- silently, with no message and no timeout (plan F2).  The
//      port shows ONE message when that refusal has lasted kCcdYHoldMs; it commands nothing and changes no task.
//      A call more than kCcdYGapMs after the previous one starts a new window (the state machine was elsewhere / stopped).
const unsigned long kCcdYHoldMs = 5000ul;
const unsigned long kCcdYGapMs  = 2000ul;
struct CcdYHold {
    bool          active;
    bool          shown;
    unsigned long startMs;
    unsigned long lastMs;
    CcdYHold() : active(false), shown(false), startMs(0), lastMs(0) {}
};
//  One refused MoveIndexZ call at nowMs.  true ONCE per window, when the refusal has lasted >= limitMs.  Wrap-safe.
inline bool CcdYHoldStep(CcdYHold& h, unsigned long nowMs, unsigned long limitMs = kCcdYHoldMs, unsigned long gapMs = kCcdYGapMs)
{
    if (!h.active || (unsigned long)(nowMs - h.lastMs) > gapMs) {
        h.active = true;
        h.shown = false;
        h.startMs = nowMs;
    }
    h.lastMs = nowMs;
    if (!h.shown && (unsigned long)(nowMs - h.startMs) >= limitMs) {
        h.shown = true;
        return true;
    }
    return false;
}
//  A MoveIndexZ call that was NOT refused ends the window.
inline void CcdYHoldReset(CcdYHold& h) { h = CcdYHold(); }

}  // namespace fp9050
}  // namespace ht9045

#endif  // IndexZFinePitchCoreH
