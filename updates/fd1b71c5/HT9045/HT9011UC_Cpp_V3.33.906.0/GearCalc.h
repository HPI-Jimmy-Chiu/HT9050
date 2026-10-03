// =============================================================================
//  GearCalc.h  --  the Motor Test "Gear Ratio" tab: the fit of a measurement, the new GearRatio, and the rescale plan of
//                  one axis' teach points (pure arithmetic and text, no global, no motion, no file)
//
//  AI(W906-GEARRATIO) 20261002 [W906]: RULINGS_20261002 #22 (NB2 spec RD5軟體_NB2規格_MotorTest頁GearRatio校正分頁_20261002_213826.md
//  §3 / §6, on origin/v906/nb2-assist). golden has NO such function: a new design the user ruled on (the four questions all "A":
//  the Motor Test page; save rescales THIS axis' teach points and soft limits, preview and backup first; several distances,
//  there and back, the fitted value; the software Mot_Table GearRatio, the drive's electronic gear stays).
//
//  Units. GearRatio = user units (0.01 mm) per card pulse: user = pulse x GearRatio (golden TMyEtherCatMotor::ReadPos,
//  Motor/myEthercatmotor.cpp), card = user / GearRatio (MotorUserToCard). A measurement commands c mm and the gauge reads
//  a mm; with k = a / c the true ratio is GearRatio x k. A teach point T (user units) taught at pulse p reads p x old; the
//  same pulse reads p x new after the change, so the point keeps its physical place with T' = round(T x new / old) (spec §6).
//
//  The fit (spec §3.2). Forward points (dir +1, c != 0), least squares through the origin: k = sum(c a) / sum(c c);
//    new ratio = old x k to 7 significant digits; k_i = a_i / c_i, consistency = (max k_i - min k_i) / k.
//    Back points (dir -1) only estimate the backlash, shown and never fitted: at the same commanded position the lost motion is
//    (a_back - a_fwd) x sign(the measurement direction), averaged; c = 0 pairs with the gauge zero (a = 0). ⚠ The spec words it
//    "往前讀數減往回讀數" -- that is the NEGATIVE of the lost motion for a measurement in the + direction; this file reports the
//    lost motion itself (>= 0 for a normal axis), the page labels it so.
//    Rules (NB2 defaults, spec §3.2; GearFitLimits): >= 2 forward points, the longest forward |c| >= 50 mm, consistency <= 0.1 %,
//    |k - 1| <= 2 % ok, 2 % < |k - 1| <= 10 % "confirm" (a second confirmation and EastSun's warning), > 10 % rejected.
//
//  The plan (spec §6): every registry slot of the teach page (TechPara / TechTwoPara / TechSuckPara / elTeach) the backend lists,
//    grouped by the VARIABLE it points at (one variable registered twice is rescaled once -- e.g. Teach.iLoadCellY1 is a
//    TechPara slot and an elTeach slot); a variable is this axis' when every attributed slot says this axis (TechPara through
//    golden's single-axis remap GoldenTeachRemap -- ep1Picker 7 / 26 -> -4, uteach.cpp:3372 / :3409; TechTwoPara one side
//    only, its own MotorSelect[side]; TechSuckPara MotorSelect[i][j]); a variable attributed to this axis AND another one is
//    not rescaled and is listed; an elTeach-only variable has no axis and is listed when it is not 0. Soft limits: the
//    +-999999 placeholders are not rescaled (they would become 991999-style values); a teach value at the placeholder
//    magnitude is not rescaled either (listed). The preview key is an FNV-1a hash of the plan text: gearRatioSave recomputes
//    the plan and refuses when the key differs from the one the operator previewed.
//
//  Linkage: WebMotorAccess.cpp (the three actions) and its ctest; wb_serve. No god-stack header may be included here.
// =============================================================================
#ifndef HT9045_GEARCALC_H
#define HT9045_GEARCALC_H

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace ht9045 {

// one reading of the measurement (mm): c = the commanded distance from the gauge zero (signed), a = the gauge's reading of
// the same distance (same sign convention), dir = +1 forward (away from the zero), -1 back (toward the zero)
struct GearMeas {
    double c = 0.0, a = 0.0;
    int    dir = 0;
};

struct GearFitLimits {
    int    minForward  = 2;        // at least two forward points
    double minSpanMm   = 50.0;     // the longest forward |c| (mm)
    double maxSpread   = 0.001;    // consistency (fraction): 0.1 %
    double okBand      = 0.02;     // |k - 1| <= 2 %: ok
    double confirmBand = 0.10;     // 2 % < |k - 1| <= 10 %: confirm; above: rejected
};

struct GearFit {
    std::string level;             // "ok" | "confirm" | "reject"
    std::string why;               // reject: the first rule that failed; confirm: the warning (zh)
    bool        computed = false;  // k / consistency are valid numbers (even when rejected, so the page can show them)
    double      oldRatio = 0.0, k = 0.0, newRatio = 0.0;
    int         nForward = 0, nBack = 0;
    double      maxSpan = 0.0;     // the longest forward |c| (mm)
    double      kMin = 0.0, kMax = 0.0, consistency = 0.0;
    std::vector<double> ki;        // forward points, in the order given
    bool        hasBacklash = false;
    double      backlash = 0.0;    // mm, the mean lost motion (see the file head)
    int         backlashPairs = 0;
    bool ok() const { return level == "ok" || level == "confirm"; }
};

GearFit GearFitCompute(double oldRatio, const std::vector<GearMeas>& m, const GearFitLimits& lim = GearFitLimits());

double GearRound7(double v);                                  // 7 significant digits ("%.7g" round trip); non-finite / 0 as is
int    GearRescale(int v, double oldRatio, double newRatio);  // round(v x new / old), half away from zero
bool   GearIsPlaceholder(int v);                              // |v| >= 999999 (Mot_Table's soft-limit placeholder)
std::string GearNum(double v, int sig = 15);                  // "%.<sig>g"

// One registry slot of the teach page that holds a teach value (filled by the backend: WebMotorAccessLive / the test fake).
struct GearTeachRef {
    const int*  ptr = nullptr;     // the variable (the dedup key)
    std::string list;              // "TechPara" | "TechTwoPara" | "TechSuckPara" | "elTeach"
    int         index = -1;        // the slot's index in its list
    int         side = 0;          // TechTwoPara: 0 / 1; TechSuckPara: i * 8 + j; else 0
    std::string section, key;      // teach.ini [section] key (TechPara / TechTwoPara: MOT[MotorSelect].Alias; elTeach: its group)
    int         rawMotor = -1;     // MotorSelect as registered (-1 = none: elTeach)
    int         value = 0;         // *ptr now
};
struct GearTeachChange {
    const int*  ptr = nullptr;
    int         oldV = 0, newV = 0;
    std::string where;             // the first slot: "TechPara[12] [MInArmX] setEditInArmLoadX"
    std::vector<std::string> slots;   // every slot of the variable (same text form)
    std::vector<std::string> keys;    // their ini keys (the verify step: these may change in teach.ini, nothing else)
};
struct GearManualItem {
    std::string where;
    bool        hasValue = false;
    int         value = 0;
    std::string why;               // zh
};
struct GearPlanInput {
    std::string alias;
    int         mi = -1;
    double      oldRatio = 0.0, newRatio = 0.0;
    int         softP = 0, softN = 0;
};
struct GearPlan {
    bool        ok = false;
    std::string why;
    GearPlanInput in;
    int         softPNew = 0, softNNew = 0;
    bool        softPPlaceholder = false, softNPlaceholder = false;
    std::vector<GearTeachChange> teach;   // the variables of this axis (deduplicated), registry order; newV may equal oldV
    int         changedCount = 0;         // teach entries whose newV != oldV
    int         zeroSkipped = 0;          // elTeach-only variables equal to 0 (not listed one by one)
    std::vector<GearManualItem> manual;   // not rescaled, listed for a manual check
    std::string key;                      // the preview key (GearPlanKey)
};
// remapTechPara: golden's single-axis teach remap of a TechPara MotorSelect (the backend's GoldenTeachRemap)
GearPlan GearPlanBuild(const GearPlanInput& in, const std::vector<GearTeachRef>& refs,
                       const std::function<int(int)>& remapTechPara);
std::string GearPlanText(const GearPlan& p);  // the canonical text the key hashes (also handy in logs)
std::string GearPlanKey(const GearPlan& p);   // FNV-1a 64 of GearPlanText, 16 hex digits

// teach.ini as the program reads it: section -> key -> value (TMemIniFile-like: case-insensitive names, the first one wins,
// blanks around names trimmed, the value is everything after the first '=' with a trailing CR removed)
struct GearIniEntry { std::string section, key, value; };
typedef std::map<std::string, std::map<std::string, GearIniEntry> > GearIni;   // lower-case section -> lower-case key -> entry
GearIni GearIniParse(const std::string& text);
struct GearIniDelta { std::string section, key, oldV, newV; bool hadOld = false, hasNew = false; };
// every key that differs (added, removed, or a different value; "12" and "012" are the same number and do not differ)
std::vector<GearIniDelta> GearIniDiff(const std::string& oldText, const std::string& newText);

}  // namespace ht9045

#endif  // HT9045_GEARCALC_H
