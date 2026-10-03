// =============================================================================
//  tests/test_gear_calc.cpp
//
//  AI(W906-GEARRATIO) 20261002 [W906]: the Motor Test "Gear Ratio" tab's pure part (GearCalc.cpp; RULINGS_20261002 #22, NB2 spec
//  §3.2 / §6 / §8). Pure: no god-stack, no file, no clock. -Wall -Wextra.
//   [1] the spec §3.2 example: 50 / 100 / 200 mm commanded, 49.62 / 99.21 / 198.43 read -> k = 52088 / 52500 = 0.9921524 (7
//       significant digits), consistency 0.03 %, level ok; the back points' lost motion (0.04 mm); the same in the - direction.
//   [2] the new ratio from another old ratio (0.071425, the HT9050 elevator Z rows) to 7 significant digits.
//   [3] the rules: < 2 forward points, the longest < 50 mm, consistency > 0.1 %, |k-1| 2..10 % = confirm (EastSun's warning),
//       > 10 % rejected, k <= 0, mixed directions, dir not +-1, a forward point at c = 0, non-finite input, a bad old ratio.
//   [4] GearRound7, GearRescale (half away from zero, both signs), GearIsPlaceholder.
//   [5] the plan: pointer dedup (TechPara + elTeach on one variable = one change, two keys), TechTwoPara only the side of this axis,
//       the ep1Picker 7 / 26 -> -4 remap of TechPara (and not of TechTwoPara), TechSuckPara, the +-999999 soft limits untouched,
//       real soft limits rescaled, a placeholder-valued teach point listed, a variable on two axes listed, elTeach-only listed
//       (0 counted, not listed), other axes untouched, the preview key (stable; changes with any value).
//   [6] teach.ini parse / diff: case-insensitive names, first wins, "12" == "012", added / removed keys, CR LF.
//  CONTROL (run by hand 20261002, reported in the commit): drop the `motors.size() > 1` refusal, the placeholder test of the soft
//  limits, the TechPara-only remap, the dedup map, or the 7-digit rounding -> the named checks go red.
// =============================================================================
#include "GearCalc.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace ht9045;

static int g_pass = 0;
static int g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }
static bool Near(double a, double b, double tol) { return std::fabs(a - b) <= tol; }
static GearMeas M(double c, double a, int dir) { GearMeas m; m.c = c; m.a = a; m.dir = dir; return m; }
static std::vector<GearMeas> Spec()
{
    std::vector<GearMeas> v;
    v.push_back(M(50, 49.62, 1)); v.push_back(M(100, 99.21, 1)); v.push_back(M(200, 198.43, 1));
    return v;
}
// a forward-only set scaled by k exactly (consistent)
static std::vector<GearMeas> Scaled(double k)
{
    std::vector<GearMeas> v;
    v.push_back(M(50, 50 * k, 1)); v.push_back(M(100, 100 * k, 1)); v.push_back(M(200, 200 * k, 1));
    return v;
}
static GearTeachRef Ref(const int* p, const char* list, int index, int side, const char* sec, const char* key, int motor)
{
    GearTeachRef r;
    r.ptr = p; r.list = list; r.index = index; r.side = side; r.section = sec; r.key = key; r.rawMotor = motor; r.value = p ? *p : 0;
    return r;
}
static const GearTeachChange* ChangeOf(const GearPlan& p, const int* ptr)
{
    for (std::size_t i = 0; i < p.teach.size(); ++i) if (p.teach[i].ptr == ptr) return &p.teach[i];
    return 0;
}
static int ManualWith(const GearPlan& p, const std::string& sub)
{
    int n = 0;
    for (std::size_t i = 0; i < p.manual.size(); ++i) if (Has(p.manual[i].where + " " + p.manual[i].why, sub)) ++n;
    return n;
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);

    std::printf("[1] the spec §3.2 example\n");
    {
        const GearFit f = GearFitCompute(1.0, Spec());
        std::printf("    k=%.17g new=%.17g consistency=%.17g level=%s why=%s\n", f.k, f.newRatio, f.consistency, f.level.c_str(), f.why.c_str());
        CHECK(f.computed && f.nForward == 3 && f.nBack == 0, "three forward points, computed");
        CHECK(Near(f.k, 52088.0 / 52500.0, 1e-15), "k = sum(c a) / sum(c c) = 52088 / 52500");
        CHECK(f.newRatio == std::strtod("0.9921524", 0), "new ratio = 1 x k to 7 significant digits = 0.9921524");
        CHECK(Near(f.ki[0], 0.9924, 1e-12) && Near(f.ki[1], 0.9921, 1e-12) && Near(f.ki[2], 0.99215, 1e-12), "k_i = 0.9924 / 0.9921 / 0.99215");
        CHECK(Near(f.consistency, 0.0003 / (52088.0 / 52500.0), 1e-12), "consistency = (max k_i - min k_i) / k = 0.0003 / 0.9921524");
        char b[32]; std::snprintf(b, sizeof(b), "%.2f", f.consistency * 100.0);
        CHECK(std::string(b) == "0.03", "consistency is 0.03 % (the spec's number)");
        CHECK(f.level == "ok" && f.ok() && f.why.empty() && Near(f.maxSpan, 200.0, 0), "|k-1| = 0.78 % <= 2 %: ok, longest 200 mm");
        CHECK(!f.hasBacklash, "no back point -> no backlash");

        std::vector<GearMeas> v = Spec();
        v.push_back(M(100, 99.25, -1)); v.push_back(M(50, 49.66, -1)); v.push_back(M(0, 0.04, -1));
        const GearFit g = GearFitCompute(1.0, v);
        CHECK(g.level == "ok" && g.nBack == 3 && Near(g.k, f.k, 0), "back points do not change k");
        CHECK(g.hasBacklash && g.backlashPairs == 3 && Near(g.backlash, 0.04, 1e-9), "lost motion 0.04 mm from the three back points (c = 0 pairs with the gauge zero)");
        std::vector<GearMeas> n;
        n.push_back(M(-50, -49.62, 1)); n.push_back(M(-100, -99.21, 1)); n.push_back(M(-200, -198.43, 1));
        n.push_back(M(-100, -99.25, -1)); n.push_back(M(0, -0.04, -1));
        const GearFit h = GearFitCompute(1.0, n);
        CHECK(h.level == "ok" && Near(h.k, f.k, 1e-15) && h.newRatio == f.newRatio, "the same measurement in the - direction: the same k and ratio");
        CHECK(h.hasBacklash && h.backlashPairs == 2 && Near(h.backlash, 0.04, 1e-9), "- direction: the lost motion is still +0.04 mm (sign of the direction applied)");
        std::vector<GearMeas> u = Spec(); u.push_back(M(75, 74.9, -1));
        CHECK(GearFitCompute(1.0, u).backlashPairs == 0, "a back point at a position no forward point had -> not paired");
    }

    std::printf("[2] another old ratio\n");
    {
        const GearFit f = GearFitCompute(0.071425, Spec());
        CHECK(f.level == "ok" && f.newRatio == std::strtod("0.07086448", 0), "0.071425 x 0.99215238 = 0.07086448 (7 significant digits)");
        const GearFit g = GearFitCompute(0.1, Scaled(1.0));
        CHECK(g.level == "ok" && g.newRatio == std::strtod("0.1", 0) && Near(g.consistency, 0.0, 1e-15), "readings equal to the commands: k = 1, the ratio stays 0.1");
    }

    std::printf("[3] the rules\n");
    {
        std::vector<GearMeas> one; one.push_back(M(200, 198.43, 1));
        GearFit f = GearFitCompute(1.0, one);
        CHECK(f.level == "reject" && Has(f.why, "至少要 2 個往前點") && f.computed, "one forward point -> rejected (k still shown)");
        std::vector<GearMeas> shortv; shortv.push_back(M(20, 19.85, 1)); shortv.push_back(M(40, 39.69, 1));
        f = GearFitCompute(1.0, shortv);
        CHECK(f.level == "reject" && Has(f.why, "50"), "the longest forward distance 40 mm < 50 mm -> rejected");
        std::vector<GearMeas> bad = Spec(); bad[1].a = 99.50;
        f = GearFitCompute(1.0, bad);
        CHECK(f.level == "reject" && Has(f.why, "量測不一致") && f.consistency > 0.001, "99.50 instead of 99.21 -> consistency > 0.1 % -> rejected");
        std::vector<GearMeas> edge = Spec(); edge[0].a = 50 * 0.99215 * (1 + 0.0009); edge[1].a = 100 * 0.99215; edge[2].a = 200 * 0.99215;
        f = GearFitCompute(1.0, edge);
        CHECK(f.level == "ok" && f.consistency < 0.001, "consistency 0.09 % -> accepted");
        edge[0].a = 50 * 0.99215 * (1 + 0.0011);
        CHECK(GearFitCompute(1.0, edge).level == "reject", "consistency 0.11 % -> rejected");
        CHECK(GearFitCompute(1.0, Scaled(0.981)).level == "ok", "|k-1| = 1.9 % -> ok");
        f = GearFitCompute(1.0, Scaled(0.979));
        CHECK(f.level == "confirm" && f.ok() && Has(f.why, "EastSun") && Has(f.why, "再確認"), "|k-1| = 2.1 % -> confirm, with EastSun's warning");
        CHECK(GearFitCompute(1.0, Scaled(1.099)).level == "confirm", "|k-1| = 9.9 % -> confirm");
        f = GearFitCompute(1.0, Scaled(0.899));
        CHECK(f.level == "reject" && !f.ok() && Has(f.why, "10.000 %"), "|k-1| = 10.1 % -> rejected");
        std::vector<GearMeas> neg; neg.push_back(M(50, -49.6, 1)); neg.push_back(M(100, -99.2, 1));
        f = GearFitCompute(1.0, neg);
        CHECK(f.level == "reject" && Has(f.why, "相反"), "readings with the opposite sign -> k < 0 -> rejected");
        std::vector<GearMeas> mix; mix.push_back(M(50, 49.6, 1)); mix.push_back(M(-100, -99.2, 1));
        CHECK(Has(GearFitCompute(1.0, mix).why, "有正有負"), "forward points in both directions -> rejected");
        std::vector<GearMeas> d0 = Spec(); d0[1].dir = 0;
        CHECK(Has(GearFitCompute(1.0, d0).why, "方向只能是"), "dir 0 -> rejected");
        std::vector<GearMeas> c0 = Spec(); c0[0].c = 0;
        CHECK(Has(GearFitCompute(1.0, c0).why, "命令距離卻是 0"), "a forward point at c = 0 -> rejected");
        std::vector<GearMeas> nf = Spec(); nf[2].a = std::nan("");
        CHECK(Has(GearFitCompute(1.0, nf).why, "有限"), "a NaN reading -> rejected");
        CHECK(Has(GearFitCompute(0.0, Spec()).why, "齒輪比") && Has(GearFitCompute(-1.0, Spec()).why, "齒輪比"), "old ratio 0 / negative -> rejected");
        CHECK(GearFitCompute(1.0, std::vector<GearMeas>()).level == "reject", "no point at all -> rejected");
    }

    std::printf("[4] rounding helpers\n");
    {
        CHECK(GearRound7(0.992152380952381) == std::strtod("0.9921524", 0), "GearRound7(0.99215238095) = 0.9921524");
        CHECK(GearRound7(12345.678) == std::strtod("12345.68", 0) && GearRound7(1.0) == 1.0 && GearRound7(0.0) == 0.0, "GearRound7: 12345.68 / 1 / 0");
        CHECK(GearRescale(10000, 1.0, std::strtod("0.9921524", 0)) == 9922 && GearRescale(-10000, 1.0, std::strtod("0.9921524", 0)) == -9922,
              "10000 x 0.9921524 = 9921.524 -> 9922; -10000 -> -9922");
        CHECK(GearRescale(3, 2.0, 3.0) == 5 && GearRescale(-3, 2.0, 3.0) == -5 && GearRescale(1, 2.0, 3.0) == 2,
              "half away from zero: 4.5 -> 5, -4.5 -> -5, 1.5 -> 2");
        CHECK(GearRescale(70004, 0.071425, std::strtod("0.07086448", 0)) == 69455, "70004 x 0.07086448 / 0.071425 = 69454.63 -> 69455");
        CHECK(GearRescale(123, 0.0, 1.0) == 123 && GearRescale(123, 1.0, -1.0) == 123, "a bad ratio leaves the value as it is");
        CHECK(GearIsPlaceholder(999999) && GearIsPlaceholder(-999999) && GearIsPlaceholder(1000000) && !GearIsPlaceholder(999998) && !GearIsPlaceholder(0),
              "placeholder: |v| >= 999999");
    }

    std::printf("[5] the plan\n");
    {
        int xLoad = 10000, yLoad = 5000, xTwo = 20000, yTwo = 7000, onlyEl = 1234, zeroEl = 0, ep1v = 3000, ph = 999999, both = 4000, suck = 50, zA = 777;
        std::vector<GearTeachRef> refs;
        refs.push_back(Ref(&xLoad, "TechPara", 0, 0, "MInArmX", "setEditInArmLoadX", 0));
        refs.push_back(Ref(&yLoad, "TechPara", 1, 0, "MInArmY", "setEditInArmLoadY", 1));
        refs.push_back(Ref(&xTwo, "TechTwoPara", 2, 0, "MInArmX", "setEditPlateX", 0));
        refs.push_back(Ref(&yTwo, "TechTwoPara", 2, 1, "MInArmY", "setEditPlateY", 1));
        refs.push_back(Ref(&xLoad, "elTeach", 9, 0, "InArm", "iLoadX", -1));                      // the same variable as TechPara[0]
        refs.push_back(Ref(&onlyEl, "elTeach", 10, 0, "ArmAlignment", "InArmAlignmentPick1_X", -1));
        refs.push_back(Ref(&zeroEl, "elTeach", 11, 0, "ArmAlignment", "InArmAlignmentPick2_X", -1));
        refs.push_back(Ref(&ep1v, "TechPara", 3, 0, "MInArmZE", "setEditInZE", 7));                // ep1Picker: 7 -> 3
        refs.push_back(Ref(&ph, "TechPara", 4, 0, "MInArmX", "setEditPh", 0));
        refs.push_back(Ref(&both, "TechPara", 5, 0, "MInArmX", "setEditBoth", 0));
        refs.push_back(Ref(&both, "TechTwoPara", 6, 1, "MOutArmX", "setEditBoth2", 19));           // the same variable on another axis
        refs.push_back(Ref(&suck, "TechSuckPara", 0, 1, "InArmZSub", "ZSub01", 3));
        refs.push_back(Ref(&zA, "TechTwoPara", 7, 1, "MInArmZE", "setEditTwoZE", 7));             // TechTwoPara: no remap -> stays 7
        const std::function<int(int)> ep1 = [](int m) { return (m == 7 || m == 26) ? m - 4 : m; };
        const std::function<int(int)> none = [](int m) { return m; };
        const double nr = std::strtod("0.9921524", 0);
        GearPlanInput in; in.alias = "MInArmX"; in.mi = 0; in.oldRatio = 1.0; in.newRatio = nr; in.softP = 999999; in.softN = -999999;
        GearPlan p = GearPlanBuild(in, refs, ep1);
        std::printf("%s", GearPlanText(p).c_str());
        CHECK(p.ok && p.why.empty() && p.key.size() == 16, "MInArmX: a plan and a 16-digit key");
        const GearTeachChange* c = ChangeOf(p, &xLoad);
        CHECK(c && c->oldV == 10000 && c->newV == 9922 && c->slots.size() == 2 && c->keys.size() == 2 && c->keys[0] == "setEditInArmLoadX" && c->keys[1] == "iLoadX",
              "dedup: TechPara[0] + elTeach (one variable) = ONE change 10000 -> 9922 with both keys");
        CHECK(ChangeOf(p, &xTwo) && ChangeOf(p, &xTwo)->newV == GearRescale(20000, 1.0, nr) && !ChangeOf(p, &yTwo),
              "TechTwoPara: only the X side (MotorSelect[0] = MInArmX) is rescaled, the Y side is not");
        CHECK(!ChangeOf(p, &yLoad) && !ChangeOf(p, &ep1v) && !ChangeOf(p, &suck), "other axes' values are not touched");
        CHECK(ManualWith(p, "InArmAlignmentPick1_X") == 1 && ManualWith(p, "InArmAlignmentPick2_X") == 0 && p.zeroSkipped == 1,
              "elTeach-only 1234 listed for a manual check; the 0 one counted, not listed");
        CHECK(!ChangeOf(p, &ph) && ManualWith(p, "setEditPh") == 1 && ManualWith(p, "佔位") == 1, "a teach value of 999999 is listed, not rescaled");
        CHECK(!ChangeOf(p, &both) && ManualWith(p, "setEditBoth") == 1 && ManualWith(p, "MOT[0]、MOT[19]") == 1,
              "a variable registered to MInArmX and MOutArmX is listed, not rescaled");
        CHECK(p.softPPlaceholder && p.softNPlaceholder && p.softPNew == 999999 && p.softNNew == -999999, "±999999 soft limits: untouched");
        CHECK(p.changedCount == 2 && p.teach.size() == 2, "two changes for MInArmX");

        GearPlanInput i3 = in; i3.alias = "MInArmZA"; i3.mi = 3;
        GearPlan p3 = GearPlanBuild(i3, refs, ep1);
        CHECK(ChangeOf(p3, &ep1v) && ChangeOf(p3, &ep1v)->newV == GearRescale(3000, 1.0, nr) && ChangeOf(p3, &suck) && !ChangeOf(p3, &zA),
              "ep1Picker: TechPara MotorSelect 7 belongs to MOT[3] (golden remap) -> rescaled with MInArmZA; TechSuckPara of MOT[3] too; TechTwoPara 7 is not remapped");
        GearPlanInput i7 = in; i7.alias = "MInArmZE"; i7.mi = 7;
        GearPlan p7 = GearPlanBuild(i7, refs, ep1);
        CHECK(!ChangeOf(p7, &ep1v) && ChangeOf(p7, &zA), "ep1Picker: calibrating MOT[7] leaves the remapped TechPara slot alone (the TechTwoPara 7 side is its own)");
        GearPlan p7n = GearPlanBuild(i7, refs, none);
        CHECK(ChangeOf(p7n, &ep1v), "without ep1Picker (no remap) the TechPara 7 slot is MOT[7]'s");

        GearPlanInput real = in; real.softP = 80000; real.softN = -500;
        GearPlan pr = GearPlanBuild(real, refs, ep1);
        CHECK(!pr.softPPlaceholder && !pr.softNPlaceholder && pr.softPNew == GearRescale(80000, 1.0, nr) && pr.softNNew == GearRescale(-500, 1.0, nr) &&
              pr.softPNew == 79372 && pr.softNNew == -496, "real soft limits rescaled: 80000 -> 79372, -500 -> -496");
        CHECK(pr.key != p.key, "a different soft limit -> a different preview key");
        CHECK(GearPlanBuild(in, refs, ep1).key == p.key, "the same input -> the same key");
        xLoad = 10001; refs[0].value = 10001; refs[4].value = 10001;
        CHECK(GearPlanBuild(in, refs, ep1).key != p.key, "a teach value that changed since the preview -> a different key");
        GearPlanInput bad = in; bad.newRatio = 0.0;
        CHECK(!GearPlanBuild(bad, refs, ep1).ok, "new ratio 0 -> no plan");
        GearPlanInput inv = in; inv.softP = 10; inv.softN = 10;
        CHECK(!GearPlanBuild(inv, refs, ep1).ok, "soft limits that do not stay ordered -> no plan");
    }

    std::printf("[6] teach.ini parse / diff\n");
    {
        const std::string a = "[InArm]\r\nAutoCleanPick=-1640\r\n[MInArmX]\r\nsetEditInArmLoadX=10000\r\nsetEditPlateX=20000\r\nsetEditPlateX=1\r\n[Index]\r\niLoadCellY1=0012\r\n";
        const std::string b = "[inarm]\r\nautocleanpick = -1640\r\n[MInArmX]\r\nsetEditInArmLoadX=9922\r\nsetEditPlateX=19843\r\n[Index]\r\niLoadCellY1=12\r\nnewKey=5\r\n";
        GearIni ia = GearIniParse(a);
        CHECK(ia["minarmx"]["seteditplatex"].value == "20000" && ia["minarmx"]["seteditplatex"].key == "setEditPlateX", "first key wins; names kept for display");
        const std::vector<GearIniDelta> d = GearIniDiff(a, b);
        int load = 0, plate = 0, added = 0, other = 0;
        for (std::size_t i = 0; i < d.size(); ++i) {
            if (d[i].key == "setEditInArmLoadX" && d[i].oldV == "10000" && d[i].newV == "9922") ++load;
            else if (d[i].key == "setEditPlateX" && d[i].newV == "19843") ++plate;
            else if (d[i].key == "newKey" && !d[i].hadOld && d[i].hasNew) ++added;
            else ++other;
        }
        CHECK(load == 1 && plate == 1 && added == 1 && other == 0 && d.size() == 3,
              "diff: the two changed values and the added key; case / blanks around '=' / 0012 vs 12 are not differences");
        CHECK(GearIniDiff(a, a).empty(), "a file against itself: no difference");
        const std::vector<GearIniDelta> r = GearIniDiff(b, "[MInArmX]\nsetEditInArmLoadX=9922\n");
        int removed = 0; for (std::size_t i = 0; i < r.size(); ++i) if (r[i].hadOld && !r[i].hasNew) ++removed;
        CHECK(removed == 4, "keys missing from the new text are reported as removed (LF-only text too)");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
