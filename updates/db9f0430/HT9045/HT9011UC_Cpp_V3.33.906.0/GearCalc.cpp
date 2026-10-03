// =============================================================================
//  GearCalc.cpp  --  see GearCalc.h (the Motor Test "Gear Ratio" tab's pure part; RULINGS_20261002 #22)
//
//  AI(W906-GEARRATIO) 20261002 [W906]: new code (golden has no such function). x87: every product / quotient that is compared
//  or rounded goes through a volatile double, so the build's excess-precision flags cannot change a result (as MtBootReal).
// =============================================================================
#include "GearCalc.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <set>

namespace ht9045 {

namespace {

std::string Pct(double fraction)                                       // 0.000302 -> "0.030 %"
{
    char b[48];
    std::snprintf(b, sizeof(b), "%.3f %%", fraction * 100.0);
    return b;
}

std::string Mm(double v)                                               // 49.62 -> "49.62"
{
    char b[48];
    std::snprintf(b, sizeof(b), "%.3f", v);
    std::string s(b);
    while (!s.empty() && s[s.size() - 1] == '0') s.erase(s.size() - 1);
    if (!s.empty() && s[s.size() - 1] == '.') s.erase(s.size() - 1);
    return s;
}

std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}

std::string Trim(const std::string& s)
{
    std::size_t b = 0, e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r')) --e;
    return s.substr(b, e - b);
}

// "12" == "012" == " 12" (one number); otherwise the text itself
bool SameIniValue(const std::string& a, const std::string& b)
{
    if (a == b) return true;
    const std::string x = Trim(a), y = Trim(b);
    if (x == y) return true;
    if (x.empty() || y.empty()) return false;
    char* ex = 0;
    char* ey = 0;
    const double dx = std::strtod(x.c_str(), &ex), dy = std::strtod(y.c_str(), &ey);
    return ex && *ex == '\0' && ey && *ey == '\0' && dx == dy;
}

std::string SlotText(const GearTeachRef& s)
{
    std::string t = s.list + "[" + std::to_string(s.index) + "]";
    if (s.list == "TechTwoPara") t += "[" + std::to_string(s.side) + "]";
    else if (s.list == "TechSuckPara") t += "[" + std::to_string(s.side / 8) + "][" + std::to_string(s.side % 8) + "]";
    return t + " [" + s.section + "] " + s.key;
}

}  // namespace

std::string GearNum(double v, int sig)
{
    char b[64];
    std::snprintf(b, sizeof(b), "%.*g", sig, v);
    return b;
}

double GearRound7(double v)
{
    if (!std::isfinite(v) || v == 0.0) return v;
    char b[64];
    std::snprintf(b, sizeof(b), "%.7g", v);
    return std::strtod(b, 0);
}

int GearRescale(int v, double oldRatio, double newRatio)
{
    if (!(std::isfinite(oldRatio) && oldRatio > 0.0) || !(std::isfinite(newRatio) && newRatio > 0.0)) return v;
    volatile double x = (double)v * newRatio;
    volatile double y = x / oldRatio;
    return (int)std::llround(y);
}

bool GearIsPlaceholder(int v) { return v >= 999999 || v <= -999999; }   //AI(W906-GEARRATIO2) 20261003: NB2 R171 L2 -- kept at |v| >= 999999 ON PURPOSE: a real limit can be large (machines/HT9050/sim_9378/Mot_Table.csv M30 MTrayX SoftLimitP 155099), so a lower threshold would call real limits placeholders and leave them un-rescaled; the machine is told to keep the placeholders exactly +-999999 (RULINGS_20261002 #69: 「拉最大」 read as keep +-999999, not +-99999). Everything hangs on it -- the Z gate (GearStaticWhy), the 50 / 100 mm caps (gearCalMove), "not rescaled" (GearPlanBuild): a table written with +-99999 would open a Z, drop the caps and rescale them (99999 -> 99214)

GearFit GearFitCompute(double oldRatio, const std::vector<GearMeas>& m, const GearFitLimits& lim)
{
    GearFit F;
    F.oldRatio = oldRatio;
    F.level = "reject";
    if (!(std::isfinite(oldRatio) && oldRatio > 0.0)) { F.why = "目前的齒輪比不是正的有限數字（" + GearNum(oldRatio) + "）"; return F; }
    std::vector<const GearMeas*> fwd, back;
    for (std::size_t i = 0; i < m.size(); ++i) {
        const GearMeas& x = m[i];
        if (!std::isfinite(x.c) || !std::isfinite(x.a)) { F.why = "第 " + std::to_string(i + 1) + " 點的命令距離或讀數不是有限的數字"; return F; }
        if (x.dir == 1) {
            if (x.c == 0.0) { F.why = "第 " + std::to_string(i + 1) + " 點是往前點，命令距離卻是 0（0 是量具歸零的位置）"; return F; }
            fwd.push_back(&x);
        } else if (x.dir == -1) {
            back.push_back(&x);
        } else {
            F.why = "第 " + std::to_string(i + 1) + " 點的方向只能是 +1（往前）或 -1（往回）";
            return F;
        }
    }
    F.nForward = (int)fwd.size();
    F.nBack = (int)back.size();
    int sgn = 0;
    for (std::size_t i = 0; i < fwd.size(); ++i) {
        const int s = fwd[i]->c > 0.0 ? 1 : -1;
        if (sgn == 0) sgn = s;
        else if (s != sgn) { F.why = "往前點的命令距離有正有負：一次量測只能往一個方向走"; return F; }
    }
    if (fwd.empty()) { F.why = "沒有往前點（至少要 " + std::to_string(lim.minForward) + " 個）"; return F; }

    volatile double sca = 0.0, scc = 0.0;
    for (std::size_t i = 0; i < fwd.size(); ++i) {
        volatile double ca = fwd[i]->c * fwd[i]->a;
        volatile double cc = fwd[i]->c * fwd[i]->c;
        sca = sca + ca;
        scc = scc + cc;
        const double span = std::fabs(fwd[i]->c);
        if (span > F.maxSpan) F.maxSpan = span;
        volatile double ki = fwd[i]->a / fwd[i]->c;
        F.ki.push_back((double)ki);
        if (i == 0 || ki < F.kMin) F.kMin = ki;
        if (i == 0 || ki > F.kMax) F.kMax = ki;
    }
    volatile double k = sca / scc;
    F.k = k;
    F.computed = std::isfinite(F.k);
    if (F.computed && F.k > 0.0) {
        volatile double spread = (F.kMax - F.kMin) / F.k;
        F.consistency = spread;
        volatile double raw = oldRatio * F.k;
        F.newRatio = GearRound7(raw);
    }
    // the lost motion at the same commanded position (shown, never fitted)
    double sum = 0.0;
    for (std::size_t i = 0; i < back.size(); ++i) {
        const GearMeas* b = back[i];
        bool paired = false;
        double aFwd = 0.0;
        if (std::fabs(b->c) <= 1e-6) { paired = true; aFwd = 0.0; }                 // back at the zero: the gauge zero (a = 0)
        else for (std::size_t j = 0; j < fwd.size() && !paired; ++j)
            if (std::fabs(fwd[j]->c - b->c) <= 1e-6) { paired = true; aFwd = fwd[j]->a; }
        if (!paired) continue;
        volatile double lost = (b->a - aFwd) * (double)sgn;
        sum += lost;
        ++F.backlashPairs;
    }
    if (F.backlashPairs > 0) { F.hasBacklash = true; F.backlash = sum / F.backlashPairs; }

    const double dev = F.computed ? std::fabs(F.k - 1.0) : 0.0;
    if (F.nForward < lim.minForward)
        F.why = "至少要 " + std::to_string(lim.minForward) + " 個往前點（目前 " + std::to_string(F.nForward) + " 個）";
    else if (F.maxSpan < lim.minSpanMm)
        F.why = "最長的往前距離要 ≥ " + Mm(lim.minSpanMm) + " mm（目前 " + Mm(F.maxSpan) + " mm）：距離太短量不準";
    else if (!F.computed || !(F.k > 0.0))
        F.why = "讀數的方向跟命令相反（k = " + GearNum(F.k, 7) + "）：量具方向或正負號輸入錯了";
    else if (F.consistency > lim.maxSpread)
        F.why = "量測不一致（可能打滑、量錯或背隙沒吃掉）：各點比值相差 " + Pct(F.consistency) + "，上限 " + Pct(lim.maxSpread);
    else if (dev > lim.confirmBand)
        F.why = "實際距離跟命令差 " + Pct(dev) + "，超過 " + Pct(lim.confirmBand) +
                "：差這麼多通常是驅動器電子齒輪或單位設錯，不是機構誤差 —— 不接受（請先問 EastSun）";
    else if (!(std::isfinite(F.newRatio) && F.newRatio > 0.0))
        F.why = "算出來的新齒輪比不是正的有限數字（" + GearNum(F.newRatio) + "）";
    else if (dev > lim.okBand) {
        F.level = "confirm";
        F.why = "實際距離跟命令差 " + Pct(dev) + "（超過 " + Pct(lim.okBand) +
                "）：差這麼多通常是驅動器電子齒輪或單位設錯，不是機構誤差，請先問 EastSun；確定要存，要再確認一次";
    } else {
        F.level = "ok";
        F.why.clear();
    }
    return F;
}

GearPlan GearPlanBuild(const GearPlanInput& in, const std::vector<GearTeachRef>& refs, const std::function<int(int)>& remapTechPara)
{
    GearPlan P;
    P.in = in;
    if (!(std::isfinite(in.oldRatio) && in.oldRatio > 0.0) || !(std::isfinite(in.newRatio) && in.newRatio > 0.0)) {
        P.why = "齒輪比不是正的有限數字（舊 " + GearNum(in.oldRatio) + "、新 " + GearNum(in.newRatio) + "）";
        return P;
    }
    P.softPPlaceholder = GearIsPlaceholder(in.softP);
    P.softNPlaceholder = GearIsPlaceholder(in.softN);
    P.softPNew = P.softPPlaceholder ? in.softP : GearRescale(in.softP, in.oldRatio, in.newRatio);
    P.softNNew = P.softNPlaceholder ? in.softN : GearRescale(in.softN, in.oldRatio, in.newRatio);
    if (P.softPNew <= P.softNNew) {
        P.why = "換算後的正向軟體極限 " + std::to_string(P.softPNew) + " 不大於負向 " + std::to_string(P.softNNew) + " —— 不換算";
        return P;
    }
    struct Var { const int* ptr; std::vector<const GearTeachRef*> slots; };
    std::vector<Var> vars;
    std::map<const int*, std::size_t> at;
    for (std::size_t i = 0; i < refs.size(); ++i) {
        const GearTeachRef& r = refs[i];
        if (!r.ptr) continue;
        std::map<const int*, std::size_t>::const_iterator it = at.find(r.ptr);
        if (it == at.end()) { at[r.ptr] = vars.size(); Var v; v.ptr = r.ptr; v.slots.push_back(&r); vars.push_back(v); }
        else vars[it->second].slots.push_back(&r);
    }
    for (std::size_t i = 0; i < vars.size(); ++i) {
        const Var& v = vars[i];
        std::set<int> motors;
        for (std::size_t s = 0; s < v.slots.size(); ++s) {
            const GearTeachRef* r = v.slots[s];
            if (r->list == "elTeach") continue;
            int mo = r->rawMotor;
            if (r->list == "TechPara" && remapTechPara && mo >= 0) mo = remapTechPara(mo);
            if (mo >= 0) motors.insert(mo);
        }
        const int val = v.slots[0]->value;
        if (motors.empty()) {                                                   // elTeach only: no axis
            if (val == 0) { ++P.zeroSkipped; continue; }
            GearManualItem mi;
            mi.where = SlotText(*v.slots[0]); mi.hasValue = true; mi.value = val;
            mi.why = "教導頁 elTeach 的值沒有馬達歸屬：不自動換算，請人工確認是不是這一軸的位置";
            P.manual.push_back(mi);
            continue;
        }
        if (!motors.count(in.mi)) continue;                                     // another axis' value
        if (motors.size() > 1) {
            std::string list;
            for (std::set<int>::const_iterator m = motors.begin(); m != motors.end(); ++m)
                list += (list.empty() ? "MOT[" : "、MOT[") + std::to_string(*m) + "]";
            GearManualItem mi;
            mi.where = SlotText(*v.slots[0]); mi.hasValue = true; mi.value = val;
            mi.why = "同一個變數同時登記給 " + list + "：不自動換算，請人工確認";
            P.manual.push_back(mi);
            continue;
        }
        if (GearIsPlaceholder(val)) {
            GearManualItem mi;
            mi.where = SlotText(*v.slots[0]); mi.hasValue = true; mi.value = val;
            mi.why = "值是 ±999999 佔位值：不換算";
            P.manual.push_back(mi);
            continue;
        }
        GearTeachChange c;
        c.ptr = v.ptr;
        c.oldV = val;
        c.newV = GearRescale(val, in.oldRatio, in.newRatio);
        c.where = SlotText(*v.slots[0]);
        for (std::size_t s = 0; s < v.slots.size(); ++s) { c.slots.push_back(SlotText(*v.slots[s])); c.keys.push_back(v.slots[s]->key); }
        if (c.newV != c.oldV) ++P.changedCount;
        P.teach.push_back(c);
    }
    P.ok = true;
    P.key = GearPlanKey(P);
    return P;
}

std::string GearPlanText(const GearPlan& p)
{
    std::string t = p.in.alias + "|" + std::to_string(p.in.mi) + "|" + GearNum(p.in.oldRatio, 17) + ">" + GearNum(p.in.newRatio, 17) +
                    "|P" + std::to_string(p.in.softP) + ">" + std::to_string(p.softPNew) + (p.softPPlaceholder ? "#" : "") +
                    "|N" + std::to_string(p.in.softN) + ">" + std::to_string(p.softNNew) + (p.softNPlaceholder ? "#" : "") + "\n";
    for (std::size_t i = 0; i < p.teach.size(); ++i) {
        const GearTeachChange& c = p.teach[i];
        t += "T|" + c.where + "|" + std::to_string(c.oldV) + ">" + std::to_string(c.newV) + "|";
        for (std::size_t s = 0; s < c.slots.size(); ++s) t += c.slots[s] + ";";
        t += "\n";
    }
    for (std::size_t i = 0; i < p.manual.size(); ++i)
        t += "M|" + p.manual[i].where + "|" + (p.manual[i].hasValue ? std::to_string(p.manual[i].value) : std::string("-")) + "|" + p.manual[i].why + "\n";
    t += "Z|" + std::to_string(p.zeroSkipped) + "\n";
    return t;
}

std::string GearPlanKey(const GearPlan& p)
{
    const std::string t = GearPlanText(p);
    unsigned long long h = 14695981039346656037ULL;
    for (std::size_t i = 0; i < t.size(); ++i) { h ^= (unsigned char)t[i]; h *= 1099511628211ULL; }
    static const char kHex[] = "0123456789abcdef";                     // not "%016llx": MinGW 6.3's msvcrt printf has no ll
    std::string s(16, '0');
    for (int i = 15; i >= 0; --i) { s[(std::size_t)i] = kHex[h & 0xFu]; h >>= 4; }
    return s;
}

GearIni GearIniParse(const std::string& text)
{
    GearIni ini;
    std::string sec, secLow;
    std::size_t b = 0;
    while (b <= text.size()) {
        std::size_t e = text.find('\n', b);
        if (e == std::string::npos) e = text.size();
        const std::string line = Trim(text.substr(b, e - b));
        b = e + 1;
        if (line.empty() || line[0] == ';') { if (e == text.size()) break; continue; }
        if (line[0] == '[') {
            const std::size_t c = line.find(']');
            sec = Trim(line.substr(1, c == std::string::npos ? std::string::npos : c - 1));
            secLow = Lower(sec);
            ini[secLow];
        } else {
            const std::size_t q = line.find('=');
            if (q != std::string::npos) {
                const std::string key = Trim(line.substr(0, q));
                if (!key.empty()) {
                    std::map<std::string, GearIniEntry>& s = ini[secLow];
                    const std::string kl = Lower(key);
                    if (!s.count(kl)) { GearIniEntry en; en.section = sec; en.key = key; en.value = Trim(line.substr(q + 1)); s[kl] = en; }
                }
            }
        }
        if (e == text.size()) break;
    }
    return ini;
}

std::vector<GearIniDelta> GearIniDiff(const std::string& oldText, const std::string& newText)
{
    const GearIni A = GearIniParse(oldText), B = GearIniParse(newText);
    std::vector<GearIniDelta> out;
    std::set<std::pair<std::string, std::string> > seen;
    for (int pass = 0; pass < 2; ++pass) {
        const GearIni& X = pass == 0 ? A : B;
        for (GearIni::const_iterator s = X.begin(); s != X.end(); ++s)
            for (std::map<std::string, GearIniEntry>::const_iterator k = s->second.begin(); k != s->second.end(); ++k) {
                const std::pair<std::string, std::string> id(s->first, k->first);
                if (!seen.insert(id).second) continue;
                GearIniDelta d;
                d.section = k->second.section; d.key = k->second.key;
                GearIni::const_iterator as = A.find(s->first), bs = B.find(s->first);
                if (as != A.end()) { std::map<std::string, GearIniEntry>::const_iterator ak = as->second.find(k->first); if (ak != as->second.end()) { d.hadOld = true; d.oldV = ak->second.value; } }
                if (bs != B.end()) { std::map<std::string, GearIniEntry>::const_iterator bk = bs->second.find(k->first); if (bk != bs->second.end()) { d.hasNew = true; d.newV = bk->second.value; } }
                if (d.hadOld && d.hasNew && SameIniValue(d.oldV, d.newV)) continue;
                out.push_back(d);
            }
    }
    return out;
}

}  // namespace ht9045
