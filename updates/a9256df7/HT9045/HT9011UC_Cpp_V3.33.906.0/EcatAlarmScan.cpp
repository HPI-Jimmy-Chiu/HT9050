// =============================================================================
//  EcatAlarmScan.cpp -- AI(W906-E045) 20261004 [W906] (St01 ST01-E2): see EcatAlarmScan.h (S-26 R4 + Steven Q98).
//  The pure rule first (no machine state), then the live layer (MOT[], the 1203 route's sample, the engine globals).
// =============================================================================
#include "EcatAlarmScan.h"

#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL)
#include "Motor/mymotor.h"          // MOT[] / Led[] / Alias
#include "Motor/myEthercatmotor.h"  // TMyEtherCatMotor (protected iBoardID / iPortID), ResetState
#include "Motor/EcatMotorRoute.h"   // W906_EcRead, TEcatAxisRead, kEcStaErrorStop
#include "cmydef.h"                 // TOTAL_MOTOR, SystemStart, iHome, MachineTypeChoice, SnMotorPower
#include "MachineType.h"            // Type_HT9050
#include "database.h"               // W906_GpibModel
#include "mysensor.h"               // Sen[]
#include "canary_support.h"         // ShowMyMessage, RecordProcess

#include <cstdio>
// ---------------------------------------------------------------------------------------------------------------
//  The live layer (the pure rule is inline in EcatAlarmScan.h)
// ---------------------------------------------------------------------------------------------------------------
namespace {
struct EcatAddr : TMyEtherCatMotor {                                 // the address read of Motor/EcatMotorRoute.cpp:41-45
    static short TMyEtherCatMotor::* Board() { return &EcatAddr::iBoardID; }
    static short TMyEtherCatMotor::* Port()  { return &EcatAddr::iPortID; }
};

TEcAlarmRow g_rows[TOTAL_MOTOR];
void (*g_sink)(const char*) = 0;

void Note(const char* line)
{
    if (g_sink) { g_sink(line); return; }
    std::printf("%s\n", line);
    RecordProcess(AnsiString(line));
}

bool IsHt9050()
{
    return W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050;   // = TableAuditLive.cpp / W906_MainCaption
}

// Motor power out = HT9050's IsIndexMotorOutOfPower (csystem.cpp:19975, EastSun ruling 20260926: EMG || SnMotorPower off),
// read from the SENSORS ONLY. IsEMGPressed() itself (csystem.cpp:19795) is not called: on a press it locks every brake
// group and servo-offs every motor -- golden calls it once per DoSystem pass, calling it once per scanned row would repeat
// that. The sensor terms below are IsEMGPressed's own (golden :1418-1440): the four EMG buttons, SnServo when enabled,
// the PLC all-EMG input when the PLC safety IO is used.
bool MotorPowerOut()
{
    if (Sen[SnFrontLeftEMG].IsOff() || Sen[SnFrontRightEMG].IsOff() ||
        Sen[SnRearLeftEMG].IsOff()  || Sen[SnRearRightEMG].IsOff())   return true;
    if (Sen[SnServo].Enable && Sen[SnServo].IsOff())                  return true;
    if (Enable_PLCSafety_IO && Sen[SnAllEMG].IsOff())                 return true;
    return Sen[SnMotorPower].IsOff();
}

// "M%02d" = the MOT[] index, as Mot_Table, NumberAlias ("[35] MLoaderZ", Motor/mymotor.cpp SetAlias) and the JAM code
// (MotorIndexToJamCode = "WAR24%03d" of the index) number it; golden's unused `M %02d` text in ScanAllMotorStatus is index+1.
std::string LatchedList()
{
    std::string s;
    for (int i = 0; i < TOTAL_MOTOR; ++i) {
        if (!g_rows[i].latched) continue;
        char b[96];
        std::snprintf(b, sizeof(b), "%sM%02d %s", s.empty() ? "" : ", ", i, MOT[i].Alias.c_str());
        s += b;
    }
    return s;
}
}  // namespace

bool W906_EcAlarmScanRow(int i)
{
    if (i < 0 || i >= TOTAL_MOTOR) return false;
    TEcAlarmIn in;
    in.ht9050 = IsHt9050();
    if (!in.ht9050) return false;                                    // every other model: golden, untouched
    TMyEtherCatMotor* const e = dynamic_cast<TMyEtherCatMotor*>(MOT[i].Motor);
    if (e == 0) return false;
    in.isEcatRow = true;
    in.enable    = e->Enable;
    TEcatAxisRead s;
    in.readOk     = in.enable && W906_EcRead(e->*EcatAddr::Board(), e->*EcatAddr::Port(), s);
    in.pending    = s.pending;
    in.state      = s.state;
    in.motionIO   = s.motionIO;
    in.homing     = SystemStart && iHome == 1;
    in.armed      = fAllMotorHome;                                   // fully homed since boot / the last motor power-off

    TEcAlarmRow& r = g_rows[i];
    const bool alarmSample = in.readOk && (((s.motionIO >> 1) & 1ul) != 0 || s.state == kEcStaErrorStop);
    if (alarmSample || r.attempted || r.latched) in.outOfPower = MotorPowerOut();   // the IO reads only when they can matter
    const TEcAlarmStep st = W906_EcAlarmStep(in, r);
    char b[320];
    if (st.raise) {
        if (in.readOk && !in.pending) MOT[i].ScanMotorStatus();      // the CW / CCW / soft-limit LEDs for GetErrorIndex, same sample
        MOT[i].Led[iAlarmLed] = true;                                // golden's raise condition reads it
        r.ledForced = true;
    } else if (r.ledForced && (st.cleared || st.powerReset)) {
        MOT[i].Led[iAlarmLed] = false;                               // the episode is over: the LED this file set goes with it
        r.ledForced = false;
    }
    if (st.noteAlarm) {
        std::snprintf(b, sizeof(b), "W906 ECALARM M%02d %s: drive alarm (state=%u motionIO=0x%lx)%s", i,
                      MOT[i].Alias.c_str(), (unsigned)in.state, in.motionIO,
                      in.homing ? " during HOME -- HOME's InitMotor resets it"
                                : (!r.armed ? " -- not homed since power-up: no clear attempt, HOME resets it" : ""));
        Note(b);
    }
    if (st.resetError) {
        e->ResetState();                                             // golden :1764-1773 Acm_AxResetError, through the route (pending after it)
        std::snprintf(b, sizeof(b), "W906 ECALARM M%02d %s: one clear-alarm attempt (ResetError) issued (Steven Q98)", i, MOT[i].Alias.c_str());
        Note(b);
    }
    if (st.cleared) {
        std::snprintf(b, sizeof(b), "W906 ECALARM M%02d %s: alarm cleared -- the axis must be homed again (Steven Q88)", i, MOT[i].Alias.c_str());
        Note(b);
    }
    if (st.latchedNow) {
        std::snprintf(b, sizeof(b), "W906 ECALARM M%02d %s: alarm could NOT be cleared -- power-cycle the whole machine, then HOME (START / HOME refused until then)",
                      i, MOT[i].Alias.c_str());
        Note(b);
        char en[256], zh[256];
        std::snprintf(en, sizeof(en), "Motor M%02d %s drive alarm could not be cleared. Power off the whole machine, power on again, then HOME.", i, MOT[i].Alias.c_str());
        std::snprintf(zh, sizeof(zh), "馬達 M%02d %s 驅動器警報無法清除，請整機斷電重開後再回原點（HOME）。", i, MOT[i].Alias.c_str());
        ShowMyMessage(AnsiString(en), AnsiString(zh));
    }
    if (st.powerReset) {
        std::snprintf(b, sizeof(b), "W906 ECALARM M%02d %s: motor power out / EMG -- the alarm episode is reset (one new clear attempt after power returns)", i, MOT[i].Alias.c_str());
        Note(b);
    }
    return st.raise;
}

bool W906_EcAlarmStartAllowed(const char* where)
{
    const std::string list = LatchedList();
    if (list.empty()) return true;
    char b[512];
    std::snprintf(b, sizeof(b), "W906 ECALARM: START/HOME refused (%s) -- drive alarm not cleared on %s; power-cycle the whole machine, then HOME",
                  where ? where : "", list.c_str());
    Note(b);
    ShowMyMessage(AnsiString(("Drive alarm could not be cleared (" + list + "). Power off the whole machine, power on again, then HOME.").c_str()),
                  AnsiString(("驅動器警報無法清除（" + list + "），請整機斷電重開後再回原點（HOME）。").c_str()));
    return false;
}

bool W906_EcAlarmMotionAllowed(std::string* why)
{
    const std::string list = LatchedList();
    if (list.empty()) return true;
    if (why) *why = "drive alarm not cleared on " + list + " -- power-cycle the whole machine, then HOME";
    return false;
}

int W906_EcAlarmLatchedCount()
{
    int n = 0;
    for (int i = 0; i < TOTAL_MOTOR; ++i) if (g_rows[i].latched) ++n;
    return n;
}

void W906_EcAlarmSetNote(void (*sink)(const char*)) { g_sink = sink; }

void W906_EcAlarmResetAll()
{
    for (int i = 0; i < TOTAL_MOTOR; ++i) g_rows[i] = TEcAlarmRow();
}
