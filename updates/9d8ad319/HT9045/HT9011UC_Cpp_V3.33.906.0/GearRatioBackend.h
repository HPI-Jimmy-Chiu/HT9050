// =============================================================================
//  GearRatioBackend.h  --  the Motor Test "Gear Ratio" tab: what the dispatcher (WebMotorAccess.cpp EOF) asks the machine
//                          for, as a base of IMotorAccessBackend (every default refuses / says "unknown")
//
//  AI(W906-GEARRATIO) 20261002 [W906]: RULINGS_20261002 #22 (NB2 spec RD5軟體_NB2規格_MotorTest頁GearRatio校正分頁_20261002_213826.md
//  §5, on origin/v906/nb2-assist). golden has no such function. A separate base class so WebMotorAccess.h changes by three
//  same-line edits only (its line numbers are cited elsewhere): IMotorAccessBackend : public IGearRatioBackend.
//  The defaults are fail-closed -- the older fakes and every backend that does not implement them make the three actions
//  refuse with a reason: an arm X / Y "cannot say" (-1), the EMG "pressed / unknown", the Teach window "open / unknown",
//  no teach registry, no write. The live bodies are GearRatioLive.cpp (wb_serve only; reads the god-stack on the tick
//  thread), forwarded by WebMotorAccessLive.cpp's LiveBackend.
//
//  The three actions (kActions, live; Motor Test only):
//    gearCalMove       (motion)  params {distanceMm (signed, a multiple of 0.01 mm), speedPct (1..20), begin (bool)} --
//                                one measurement move; begin=true = a new session whose first move is the backlash take-up
//                                (|d| <= 5 mm; its target is the gauge zero). ack result "moving" + target / cardTarget /
//                                arriveCmdPos (= what runtime position.cmdPos reads at arrival) + the session.
//    gearRatioPreview  (control, query) params {oldRatio, measC[], measA[], measDir[]} -- the fit and the plan, writes nothing.
//    gearRatioSave     (control) the same params + previewKey (the preview's) + confirmLarge (needed for a 2..10 % change) --
//                                backup -> Mot_Table (3 cells) -> memory -> teach.ini -> verify (restore both files and the
//                                memory on any failure) -> this axis HomeFlag = 0 (no axis zeroed) -> GearRatioCal.csv.
//    gearHandBegin     (control) AI(W906-GEARHAND) 20261003, RULINGS_20261003 #8: a hand-push session on one PCI1203 linear axis --
//                                E0 = the encoder now; the operator uses the servo button (OFF -> pushes it to the ruler mark ->
//                                ON) and C++ takes E1; preview / save then take {hand:true, rulerMm} instead of the three arrays
//                                (same fit convention, same save path). No move is ever commanded.
//  The measurement arrays are three parallel numeric arrays because MotorAccessParse carries numeric arrays only (an array
//  of objects -- the spec's measurements:[{c,a,dir}] -- would be dropped by the parser without a word).
// =============================================================================
#ifndef HT9045_GEARRATIOBACKEND_H
#define HT9045_GEARRATIOBACKEND_H

#include <string>
#include <utility>
#include <vector>
#include "GearCalc.h"

namespace ht9045 {

class IMotorAccessBackend;

class IGearRatioBackend {
public:
    virtual ~IGearRatioBackend() {}
    // 1 = an In arm X / Y (MInArmX / MInArmY), 2 = an Out arm X / Y (MOutArmX / MOutArmY), 0 = neither, -1 = cannot say (refused)
    virtual int  GearArmOf(int mi) { (void)mi; return -1; }
    // golden TfMotorTest::IsMotorCanRun(true)'s sensors (uMotorTest.cpp:1280-1298) WITHOUT its "EMG Stop" box (in wb_serve a
    //   ShowMyMessage holds the tick thread until it is answered): true + why = pressed, or this backend cannot tell
    virtual bool GearEmgStop(std::string& why) { why = "這個後端讀不到急停狀態（GearEmgStop）—— 不移動"; return true; }
    // the Teach window (golden fTeach) is open on the HMI: true + why = open, or cannot tell (an open Teach page would put its
    //   old values back on its next save, golden IC_UpdateTempTech -- NB2 spec §5.3)
    virtual bool GearTeachWindowOpen(std::string& why) { why = "這個後端不知道教導頁開著沒有（GearTeachWindowOpen）"; return true; }
    // every registry slot of the teach page that holds a teach value, with its value now (TechPara / TechTwoPara / TechSuckPara / elTeach)
    virtual bool GearTeachRefs(std::vector<GearTeachRef>& out, std::string& why) { out.clear(); why = "這個後端沒有教導頁的登錄表（GearTeachRefs）"; return false; }
    virtual std::string GearTeachIniPath() { return std::string(); }   // asTeachPath (common.cpp: W906_TEACH_INI_PATH, else the golden literal)
    // memory: MOT[mi].Motor->GearRatio / PSoftLimitP / PSoftLimitN (the HSys.MotTable row goes through MotTableRowSaved)
    virtual bool GearSetMotor(int mi, double ratio, int softP, int softN, std::string& why)
    { (void)mi; (void)ratio; (void)softP; (void)softN; why = "這個後端不能改馬達參數（GearSetMotor）"; return false; }
    // write through the registry's pointers (only pointers GearTeachRefs returned in this same call of the dispatcher)
    virtual bool GearSetTeach(const std::vector<std::pair<const int*, int> >& v, std::string& why)
    { (void)v; why = "這個後端不能改教導點（GearSetTeach）"; return false; }
    // golden IC_btnSaveClick's tail without its question (FileRW/Teach.cpp): save ? FileRW_Teach_SaveFile(true) : nothing; then
    //   fTeach->ReadFile(), the proxies mirrored, InitShuttleThreadParameter(), fAllMotorHome=false. note = what it reported.
    virtual bool GearTeachSaveReload(bool save, std::string& why, std::string& note)
    { (void)save; note.clear(); why = "這個後端不能寫 teach.ini（GearTeachSaveReload）"; return false; }
    virtual std::string GearCalLogDir() { return std::string(); }      // W906_OPLOG_DIR ("" = no calibration log file)  //AI(W906-GEARLOGDIR) 20261003: the live one falls back to the machine log root (GearCalLogDirOf, EOF)
    virtual std::string GearOperator() { return std::string(); }       // who is logged in, for the log
    virtual std::string GearStamp() { return std::string(); }          // "" = the local yyyymmdd_hhmmss (the tests pin it)
};

// the measurement session as C++ holds it (tests / logs); MotorAccessGearCalJson = the runtime "gearCal" block
struct MotorAccessGearCalState {
    bool          active = false;
    unsigned long id = 0;
    std::string   motor;
    int           mi = -1, axis = -1, dirSign = 0;
    double        ratio = 0.0;
    int           zeroUser = 0, lastTargetUser = 0, arriveCmdPos = 0;
    double        zeroCard = 0.0, lastTargetCard = 0.0;
    bool          placeholder = true;
    std::vector<double> cNom, cTrue;
    std::vector<int>    dir;
    std::string   why;                 // why the last session ended ("" = still running / never)
    //AI(W906-GEARHAND) 20261003: the hand-push session (WebMotorAccess.cpp HAND-PUSH block; RULINGS_20261003 #8): phase 0 = E0 taken,
    //  1 = servo OFF (pushing), 2 = servo ON (waiting for a settled sample), 3 = E1 taken; e0Card / e1Card = monitor actPos (pulses)
    bool          hand = false;
    int           handPhase = 0, pushes = 0;
    double        e0Card = 0.0, e1Card = 0.0;
};
MotorAccessGearCalState MotorAccessGearCal();
// {session:{...}|null, axes:[{motor, mi, eligible, kind, why, gearRatio, softP, softN, placeholder, homeFlag}], limits:{...},
//  fit:{...}} -- aliases = the enabled Mot_Table rows (the live side passes them; tick thread)
std::string MotorAccessGearCalJson(IMotorAccessBackend& be, const std::vector<std::string>& aliases);

// GearRatioLive.cpp (wb_serve only): the bodies of the live overrides
int         GearLiveArmOf(int mi);
bool        GearLiveEmgStop(std::string& why);
bool        GearLiveTeachWindowOpen(std::string& why);
bool        GearLiveTeachRefs(std::vector<GearTeachRef>& out, std::string& why);
std::string GearLiveTeachIniPath();
bool        GearLiveSetMotor(int mi, double ratio, int softP, int softN, std::string& why);
bool        GearLiveSetTeach(const std::vector<std::pair<const int*, int> >& v, std::string& why);
bool        GearLiveTeachSaveReload(bool save, std::string& why, std::string& note);
std::string GearLiveCalLogDir();
std::string GearLiveOperator();

//AI(W906-GEARLOGDIR) 20261003: NB2-1 -- RULINGS_20261003 #22 = s0 #71 (4) A (the laptop's CHAT_JIMMY 1003 14:3x task (f)): where
//  GearRatioCal.csv goes. W906_OPLOG_DIR set -> there (as before); not set -> the machine's log root (as9045LogPath, golden's
//  D:\HT9045_Log; ctest redirects it with W906_HT9045LOG_ROOT) -- was "" = no file. "" only when both are empty.
//  GearLiveCalLogDir (GearRatioLive.cpp) passes getenv("W906_OPLOG_DIR") and as9045LogPath; test_web_motor_access.cpp [GEARLOGDIR].
inline std::string GearCalLogDirOf(const char* oplogEnv, const std::string& machineLogRoot)
{
    return (oplogEnv && *oplogEnv) ? std::string(oplogEnv) : machineLogRoot;
}

}  // namespace ht9045

#endif  // HT9045_GEARRATIOBACKEND_H
