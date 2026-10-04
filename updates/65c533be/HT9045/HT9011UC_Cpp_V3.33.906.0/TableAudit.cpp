// TableAudit.cpp -- AI(W906-E043) 20261004 [W906] (St01 ST01-E2): the pure table audit (rules T1-T12, see TableAudit.h).
//   No machine globals here (tests/test_tableaudit.cpp links this file alone). The loader facts it mirrors are database.cpp
//   LoadIoData :1697-1763 / TIODATA :1872-2038 and LoadMotData :1774-1840 / TMOTDATA :2507-3131, cinitial.cpp InitCylinder
//   :4851-5040 / :5242-5278 (S-23 A1, docs/handoff/S23_FINDINGS_A1A2C5_20261004.md s2).
#include "TableAudit.h"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>

#include "EtherCAT/Pci1203ModuleCheck.h"   // Pci1203ModuleIniParse (pure; the 1203 module check's own ini reader)

namespace ht9045 {
namespace {

std::string Trim(const std::string& s)
{
    std::size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) --b;
    return s.substr(a, b - a);
}

std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> out;
    std::string cur;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') { out.push_back(cur); cur.clear(); }
        else if (text[i] != '\r') cur += text[i];
    }
    if (!cur.empty()) out.push_back(cur);
    if (!out.empty() && out[0].size() >= 3 && (unsigned char)out[0][0] == 0xEF && (unsigned char)out[0][1] == 0xBB && (unsigned char)out[0][2] == 0xBF)
        out[0] = out[0].substr(3);   // UTF-8 BOM
    return out;
}

std::vector<std::string> Cells(const std::string& line)
{
    std::vector<std::string> out;
    std::string cur;
    for (std::size_t i = 0; i < line.size(); ++i) {
        if (line[i] == ',') { out.push_back(cur); cur.clear(); }
        else cur += line[i];
    }
    out.push_back(cur);
    return out;
}

bool IsInt(const std::string& s)
{
    std::size_t i = 0;
    if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
    if (i >= s.size()) return false;
    for (; i < s.size(); ++i) if (s[i] < '0' || s[i] > '9') return false;
    return true;
}
bool IsHex(const std::string& s)
{
    std::size_t i = 0;
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) i = 2;
    if (i >= s.size()) return false;
    for (; i < s.size(); ++i) {
        const char c = s[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return false;
    }
    return true;
}
bool IsFloat(const std::string& s)
{
    std::size_t i = 0;
    bool digits = false, dot = false;
    if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
    for (; i < s.size(); ++i) {
        const char c = s[i];
        if (c >= '0' && c <= '9') digits = true;
        else if (c == '.' && !dot) dot = true;
        else if ((c == 'e' || c == 'E') && digits) { ++i; if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i; if (i >= s.size()) return false; for (; i < s.size(); ++i) if (s[i] < '0' || s[i] > '9') return false; return true; }
        else return false;
    }
    return digits;
}
std::string Norm(const std::string& s) { return IsInt(s) ? std::to_string(std::atoi(s.c_str())) : s; }

struct Ctx {
    TableAuditResult r;
    void Add(int level, const char* rule, const std::string& text)
    {
        TableAuditFinding f;
        f.level = level; f.rule = rule; f.text = text;
        r.findings.push_back(f);
        if (level == kAuditError) ++r.errors; else if (level == kAuditWarn) ++r.warns; else ++r.infos;
    }
};

std::string Join(const std::vector<std::string>& v, std::size_t maxN)
{
    std::string s;
    for (std::size_t i = 0; i < v.size() && i < maxN; ++i) s += (i ? ", " : "") + v[i];
    if (v.size() > maxN) s += ", ...（共 " + std::to_string(v.size()) + "）";
    return s;
}

// ---------------------------------------------------------------- IO_Table
const char* const kIoCols[15] = { "IOType", "Alias", "Lane", "ModuleType", "IP", "Port", "Bit", "InType", "ISABase", "Enable",
                                  "OnAlarmTime", "OffAlarmTime", "OnDelayTime", "OffDelayTime", "Note" };
enum { cType, cAlias, cLane, cModule, cIp, cPort, cBit, cInType, cIsa, cEnable, cOnAl, cOffAl, cOnDl, cOffDl, cNote };

struct IoRow {
    int line; std::string type, alias; bool enabledRaw, forced0, effective, numeric; int isa;
    std::string ip, port, bit;
};
bool IsOutputType(const std::string& t) { return t == "Cylinder" || t == "Switch" || t == "Sucker_On" || t == "Sucker_Off"; }

// ---------------------------------------------------------------- Mot_Table
const char* const kMotCols[29] = { "Motorname", "Alias", "SoftLimitN", "SoftLimitP", "BoardID", "Port", "IP", "Direction", "GearRatio",
    "HomeDirectior", "HomeHighSpeed", "HomeLowSpeed", "InitSpeed", "JogHighSpeed", "JogLowSpeed", "Rate", "Enable", "ServoAlarmOn",
    "Range", "1P2P", "SensorType", "SimulateSpeed", "CardModel", "Acc", "Dec", "EncodeType", "PickLimit", "LimitLogic", "In1Logic" };

struct MotRow { int line; std::string name, alias, card, board, port; bool effective; };

std::map<std::string, int> HeaderIndex(const std::vector<std::string>& h)
{
    std::map<std::string, int> m;
    for (std::size_t i = 0; i < h.size(); ++i) m[Trim(h[i])] = (int)i;
    return m;
}

}  // namespace

TableAuditResult TableAuditRunPure(const TableAuditInput& in)
{
    Ctx c;
    const bool tablesUsed = in.ioCardType == 2 || in.ioCardType == 3 || in.ioCardType == 4;   // database.cpp:1212, cinitial.cpp:504 ...
    if (in.ht9050 && in.ioCardType != 4)
        c.Add(kAuditError, "T1", "[System] IO_CARD_TYPE=" + std::to_string(in.ioCardType) + "：HT9050 要 4（PCI1203_IO）；"
              + (tablesUsed ? std::string("") : std::string("這個值兩張表都不讀、什麼都綁不到（database.cpp:1212）")));

    // ---------------- IO_Table
    std::vector<IoRow> io;
    std::map<std::string, int> ioAliasLine;
    if (tablesUsed && !in.ioFound) c.Add(kAuditError, "T1", "IO_Table.csv 不存在：IO 什麼都綁不到");
    if (in.ioFound) {
        const std::vector<std::string> L = Lines(in.ioCsv);
        const std::vector<std::string> head = L.empty() ? std::vector<std::string>() : Cells(L[0]);
        std::map<std::string, int> hx = HeaderIndex(head);
        std::vector<std::string> missing;
        for (int k = 0; k < 15; ++k) if (!hx.count(kIoCols[k])) missing.push_back(kIoCols[k]);
        if (head.size() != 15 || !missing.empty() || L.size() < 2) {
            c.Add(tablesUsed ? kAuditError : kAuditWarn, "T1", "IO_Table.csv 表頭／內容讀不進來（表頭 " + std::to_string(head.size()) + " 欄，要剛好 15 欄"
                  + (missing.empty() ? std::string("") : "；缺 " + Join(missing, 15)) + (L.size() < 2 ? "；沒有資料列" : "")
                  + "）：載入程式整檔不讀（database.cpp:1714-1745）");
        } else {
            int col[15];
            for (int k = 0; k < 15; ++k) col[k] = hx[kIoCols[k]];
            for (std::size_t li = 1; li < L.size(); ++li) {
                if (Trim(L[li]).empty()) continue;
                std::vector<std::string> v = Cells(L[li]);
                const int line = (int)li + 1;
                if (v.size() < 14) { c.Add(kAuditWarn, "T8", "IO_Table.csv:" + std::to_string(line) + " 只有 " + std::to_string(v.size()) + " 格（少於 14 格：整列當成空的，database.cpp:1878）"); continue; }
                while (v.size() < 15) v.push_back("");
                for (std::size_t k = 0; k < v.size(); ++k)
                    if (Trim(v[k]).find(' ') != std::string::npos)
                        c.Add(kAuditWarn, "T8", "IO_Table.csv:" + std::to_string(line) + " 第 " + std::to_string(k + 1) + " 欄「" + Trim(v[k]) + "」中間有空白（CommaText 會在空白處切欄，後面的欄位全部錯位）");
                IoRow r;
                r.line = line; r.type = Trim(v[col[cType]]); r.alias = Trim(v[col[cAlias]]);
                const std::string en = Trim(v[col[cEnable]]);
                const std::string isaS = Trim(v[col[cIsa]]);
                r.isa = IsInt(isaS) ? std::atoi(isaS.c_str()) : 0;
                r.enabledRaw = en == "1";
                r.ip = Trim(v[col[cIp]]); r.port = Trim(v[col[cPort]]); r.bit = Trim(v[col[cBit]]);
                const std::string where = "IO_Table.csv:" + std::to_string(line) + " " + (r.alias.empty() ? std::string("（沒有名稱）") : r.alias);
                if (r.type.empty() && r.alias.empty()) continue;   // a spacer row: never binds
                r.numeric = true;
                const int addr[7] = { cLane, cModule, cIp, cPort, cBit, cInType, cIsa };
                for (int k = 0; k < 7; ++k) {
                    const std::string s = Trim(v[col[addr[k]]]);
                    if (s.empty()) continue;
                    const bool hexPort = addr[k] == cPort && (r.isa == 1 || r.isa == 2 || r.isa == 4);   // HexStrToInt, database.cpp:1950-1955
                    if (hexPort ? IsHex(s) : IsInt(s)) continue;
                    r.numeric = false;
                    c.Add(r.enabledRaw ? kAuditError : kAuditWarn, r.enabledRaw ? "T2a" : "T2b",
                          where + " 的 " + kIoCols[addr[k]] + "＝「" + s + "」不是數字：載入程式安靜地當成 " + (hexPort ? "-1" : "0") + (r.enabledRaw ? "（Enable 1：位址錯了還是會動作）" : ""));
                }
                const int times[4] = { cOnAl, cOffAl, cOnDl, cOffDl };
                for (int k = 0; k < 4; ++k) {
                    const std::string s = Trim(v[col[times[k]]]);
                    if (!s.empty() && !IsInt(s)) c.Add(kAuditWarn, "T2b", where + " 的 " + kIoCols[times[k]] + "＝「" + s + "」不是數字，當成 0");
                }
                if (!en.empty() && !IsInt(en)) c.Add(kAuditWarn, "T2b", where + " 的 Enable＝「" + en + "」不是數字，當成 0（停用）");
                const std::string lane = Trim(v[col[cLane]]);
                r.forced0 = r.port.empty() || r.bit.empty() || ((isaS.empty() || r.isa == 0) && (lane.empty() || r.ip.empty()));   // database.cpp:1910 / :1932 / :1946 / :1962 / :1980
                if (r.enabledRaw && r.forced0)
                    c.Add(kAuditWarn, "T3", where + " 表上 Enable 1，但有位址欄是空的，載入程式會強制成 0（database.cpp:1980）");
                r.effective = r.enabledRaw && !r.forced0 && !r.alias.empty();
                if (!r.type.empty() && r.alias.empty())
                    c.Add(kAuditWarn, "T8", "IO_Table.csv:" + std::to_string(line) + " " + r.type + " 沒有名稱（Alias 空白）：永遠綁不到（database.cpp:1729）");
                if (!r.alias.empty()) {
                    if (ioAliasLine.count(r.alias))
                        c.Add(kAuditWarn, "T8", where + " 名稱重複（第一列 IO_Table.csv:" + std::to_string(ioAliasLine[r.alias]) + " 生效，這一列綁不到，database.cpp:1727-1739）");
                    else ioAliasLine[r.alias] = line;
                }
                io.push_back(r);
            }
        }
    }
    // T5: duplicate addresses among effective rows
    {
        std::map<std::string, std::vector<const IoRow*> > outs, ins;
        for (std::size_t i = 0; i < io.size(); ++i) {
            const IoRow& r = io[i];
            if (!r.effective || !r.numeric) continue;
            const std::string key = std::to_string(r.isa) + "/" + Norm(r.ip) + "/" + Norm(r.port) + "/" + Norm(r.bit);
            (IsOutputType(r.type) ? outs : ins)[key].push_back(&r);
        }
        for (int pass = 0; pass < 2; ++pass) {
            const std::map<std::string, std::vector<const IoRow*> >& m = pass == 0 ? outs : ins;
            for (std::map<std::string, std::vector<const IoRow*> >::const_iterator it = m.begin(); it != m.end(); ++it) {
                if (it->second.size() < 2) continue;
                std::vector<std::string> who;
                for (std::size_t k = 0; k < it->second.size(); ++k) who.push_back(it->second[k]->alias + "（:" + std::to_string(it->second[k]->line) + "）");
                c.Add(pass == 0 ? kAuditError : kAuditWarn, pass == 0 ? "T5a" : "T5b",
                      std::string(pass == 0 ? "兩個 Enable 1 的輸出" : "兩個 Enable 1 的輸入") + "在同一個位址（ISABase/IP/Port/Bit " + it->first + "）：" + Join(who, 8));
            }
        }
    }
    // T6 / T7: cylinders
    std::map<std::string, const IoRow*> byAlias;
    for (std::size_t i = 0; i < io.size(); ++i) if (!io[i].alias.empty() && !byAlias.count(io[i].alias)) byAlias[io[i].alias] = &io[i];
    std::map<std::string, bool> cylOut;   // the loader's Cylinder[].Enable from the output row (cinitial.cpp:5018-5021)
    for (std::size_t i = 0; i < io.size(); ++i) {
        const IoRow& r = io[i];
        if (r.type != "Cylinder" || r.alias.empty() || cylOut.count(r.alias)) continue;
        cylOut[r.alias] = r.enabledRaw && !r.forced0;
        const IoRow* on  = byAlias.count(r.alias + "_On")  ? byAlias[r.alias + "_On"]  : 0;
        const IoRow* off = byAlias.count(r.alias + "_Off") ? byAlias[r.alias + "_Off"] : 0;
        const bool outE = cylOut[r.alias];
        const bool onE = on && on->enabledRaw && !on->forced0, offE = off && off->enabledRaw && !off->forced0;
        if ((on && onE != outE) || (off && offE != outE)) {
            std::string s = r.alias + "（:" + std::to_string(r.line) + "）輸出 Enable " + (outE ? "1" : "0") + "，_On " + (on ? (onE ? "1" : "0") : "沒有") + "、_Off " + (off ? (offE ? "1" : "0") : "沒有") + "：";
            s += outE ? "到位感測停用 ⇒ 到位判斷一律立刻成功（mycylin.cpp:205-227）" : "氣缸停用 ⇒ On()/Off() 不動作，但直接讀感測的程式看得到真實輸入（mycylin.cpp:262、:304）";
            c.Add(kAuditWarn, "T6", s);
        }
    }
    if (in.ioFound && tablesUsed) {
        static const char* const kEC[8] = { "C_EmptyLoaderZ_Select", "C_ColorLoaderZ_Select", "C_Empty_Fix", "C_Color_Fix",
                                            "C_Color_Up", "C_Color_Middle", "C_Empty_Up", "C_Empty_Middle" };
        bool forced[8];
        if (in.autoEmptyColor == 0) { for (int k = 0; k < 8; ++k) forced[k] = false; }
        else if (in.autoEmptyColor == 1 && in.useMrSystem) { const bool m[8] = { true, false, true, false, false, false, true, true }; for (int k = 0; k < 8; ++k) forced[k] = m[k]; }
        else { for (int k = 0; k < 8; ++k) forced[k] = true; if (in.loadZMotorEmpty) forced[7] = false; if (in.loadZMotorColor) forced[5] = false; }
        for (int k = 0; k < 8; ++k) {
            const bool have = cylOut.count(kEC[k]) != 0, table = have && cylOut[kEC[k]];
            if (forced[k] == table) continue;
            c.Add(kAuditWarn, "T7", std::string(kEC[k]) + "：表上 " + (have ? (table ? "Enable 1" : "Enable 0") : "沒有這一列") + "，但 AUTO_EMPTY_COLOR="
                  + std::to_string(in.autoEmptyColor) + " 讓程式把它" + (forced[k] ? "打開" : "關掉") + "（golden cinitial.cpp:5242-5278，不看表）"
                  + (forced[k] && !have ? "；沒有表格列 ⇒ 輸出位址是建構子的 0" : ""));
        }
    }

    // ---------------- Mot_Table
    std::vector<MotRow> mot;
    std::set<std::string> motNames;
    if (tablesUsed && !in.motFound) c.Add(kAuditError, "T1", "Mot_Table.csv 不存在：馬達什麼都綁不到");
    if (in.motFound) {
        const std::vector<std::string> L = Lines(in.motCsv);
        const std::vector<std::string> head = L.empty() ? std::vector<std::string>() : Cells(L[0]);
        std::map<std::string, int> hx = HeaderIndex(head);
        std::vector<std::string> missing;
        for (int k = 0; k < 29; ++k) if (!hx.count(kMotCols[k])) missing.push_back(kMotCols[k]);
        const bool onlySimSpeed = missing.size() == 1 && missing[0] == "SimulateSpeed";
        if (head.size() != 29 || (!missing.empty() && !onlySimSpeed) || L.size() < 2) {
            c.Add(tablesUsed ? kAuditError : kAuditWarn, "T1", "Mot_Table.csv 表頭／內容讀不進來（表頭 " + std::to_string(head.size()) + " 欄，要 29 欄"
                  + (missing.empty() ? std::string("") : "；缺 " + Join(missing, 29)) + (L.size() < 2 ? "；沒有資料列" : "") + "）：載入程式整檔不讀（database.cpp:1799）");
        } else {
            if (onlySimSpeed) c.Add(kAuditWarn, "T8", "Mot_Table.csv 缺 SimulateSpeed 欄：載入程式照樣通過（>=28，database.cpp:1799），而且會蓋掉其他缺欄的錯誤");
            std::map<std::string, int> nameLine;
            std::map<std::string, std::vector<const MotRow*> > addr;
            for (std::size_t li = 1; li < L.size(); ++li) {
                if (Trim(L[li]).empty()) continue;
                std::vector<std::string> v = Cells(L[li]);
                const int line = (int)li + 1;
                if (v.size() < 29) { c.Add(kAuditWarn, "T8", "Mot_Table.csv:" + std::to_string(line) + " 只有 " + std::to_string(v.size()) + " 格（少於 29 格：整列當成空的，database.cpp:2514）"); continue; }
                MotRow m;
                m.line = line;
                m.name  = Trim(v[hx["Motorname"]]); m.alias = Trim(v[hx["Alias"]]); m.card = Trim(v[hx["CardModel"]]);
                m.board = Trim(v[hx["BoardID"]]);   m.port  = Trim(v[hx["Port"]]);
                if (m.name.empty()) continue;
                const std::string where = "Mot_Table.csv:" + std::to_string(line) + " " + m.name + " " + m.alias;
                if (motNames.count(m.name)) c.Add(kAuditWarn, "T8", where + " Motorname 重複（第一列 Mot_Table.csv:" + std::to_string(nameLine[m.name]) + " 生效）");
                else { motNames.insert(m.name); nameLine[m.name] = line; }
                const std::string en = hx.count("Enable") ? Trim(v[hx["Enable"]]) : std::string();
                const bool enabledRaw = en == "1";
                std::vector<std::string> empties;
                for (int k = 0; k < 29; ++k) {
                    const std::string col = kMotCols[k];
                    if (col == "Motorname" || col == "Alias" || col == "CardModel") continue;
                    if (col == "IP" && m.card != "SYNTEK") continue;                                              // database.cpp:2655-2676
                    if (col == "PickLimit" && m.alias != "MTestZ1" && m.alias != "MTestZ2") continue;             // :3046-3064
                    if (!hx.count(col)) continue;
                    const std::string s = Trim(v[hx[col]]);
                    if (s.empty()) { empties.push_back(col); continue; }
                    const bool isF = col == "GearRatio" || col == "Acc" || col == "Dec";
                    const bool ok = isF ? IsFloat(s) : (col == "Port" && m.card == "MC88X1") ? IsHex(s) : IsInt(s);
                    if (!ok) {
                        c.Add(enabledRaw ? kAuditError : kAuditWarn, enabledRaw ? "T2a" : "T2b",
                              where + " 的 " + col + "＝「" + s + "」不是數字：載入程式安靜地當成 " + (isF ? std::to_string(std::atof(s.c_str())) : std::string("0")));
                        continue;
                    }
                    if (col == "GearRatio" && std::atof(s.c_str()) <= 0.0)
                        c.Add(enabledRaw ? kAuditError : kAuditWarn, enabledRaw ? "T2a" : "T2b", where + " 的 GearRatio＝" + s + " ≤ 0（移動與回原點會除以這個值，Motor/myEthercatmotor.cpp:1424／:1452／:1698／:1884）");
                }
                if (enabledRaw && !empties.empty())
                    c.Add(kAuditWarn, "T3", where + " 表上 Enable 1，但 " + Join(empties, 29) + " 是空的，載入程式會強制成 0（database.cpp:3091-3094）");
                m.effective = enabledRaw && empties.empty();
                if (in.ht9050 && m.effective && m.card != "PCI1203")
                    c.Add(kAuditError, "T9", where + " Enable 1，但 CardModel＝「" + m.card + "」：HT9050 只有 PCI1203 卡");
                mot.push_back(m);
            }
            for (std::size_t i = 0; i < mot.size(); ++i)
                if (mot[i].effective && mot[i].card == "PCI1203") addr[Norm(mot[i].board) + "/" + Norm(mot[i].port)].push_back(&mot[i]);
            for (std::map<std::string, std::vector<const MotRow*> >::const_iterator it = addr.begin(); it != addr.end(); ++it) {
                if (it->second.size() < 2) continue;
                std::vector<std::string> who;
                for (std::size_t k = 0; k < it->second.size(); ++k) who.push_back(it->second[k]->name + " " + it->second[k]->alias + "（:" + std::to_string(it->second[k]->line) + "）");
                c.Add(kAuditError, "T4", "兩個 Enable 1 的 PCI1203 軸在同一個站號／軸號 BoardID/Port " + it->first + "：" + Join(who, 8));
            }
        }
    }

    // ---------------- T11: Pci1203Axis.ini
    if (in.axisIniFound) {
        const std::vector<std::string> L = Lines(in.axisIni);
        std::string sec;
        std::map<std::string, std::map<std::string, std::string> > secs;
        std::vector<std::string> order;
        for (std::size_t i = 0; i < L.size(); ++i) {
            const std::string t = Trim(L[i]);
            if (t.empty() || t[0] == ';' || t[0] == '#') continue;
            if (t[0] == '[' && t[t.size() - 1] == ']') { sec = t.substr(1, t.size() - 2); if (!secs.count(sec)) order.push_back(sec); secs[sec]; continue; }
            const std::size_t eq = t.find('=');
            if (eq != std::string::npos && !sec.empty()) secs[sec][Trim(t.substr(0, eq))] = Trim(t.substr(eq + 1));
        }
        for (std::size_t i = 0; i < order.size(); ++i) {
            const std::string& s = order[i];
            int st = -1, ax = -1;
            if (std::sscanf(s.c_str(), "station%d.axis%d", &st, &ax) != 2) { c.Add(kAuditWarn, "T11", "Pci1203Axis.ini [" + s + "] 不是 stationN.axisM 的格式"); continue; }
            const MotRow* hit = 0;
            for (std::size_t k = 0; k < mot.size(); ++k)
                if (mot[k].effective && mot[k].card == "PCI1203" && Norm(mot[k].board) == std::to_string(st) && Norm(mot[k].port) == std::to_string(ax)) { hit = &mot[k]; break; }
            const std::map<std::string, std::string>& kv = secs[s];
            if (!hit) c.Add(kAuditWarn, "T11", "Pci1203Axis.ini [" + s + "] 對不到任何 Enable 1 的 PCI1203 軸（Mot_Table BoardID/Port " + std::to_string(st) + "/" + std::to_string(ax) + "）");
            const bool pel = kv.count("pelLogic") != 0, mel = kv.count("melLogic") != 0;
            if (pel != mel) c.Add(kAuditWarn, "T11", "Pci1203Axis.ini [" + s + "]" + (hit ? "（" + hit->name + " " + hit->alias + "）" : std::string("")) + " 只有 " + (pel ? "pelLogic" : "melLogic") + "，少了 " + (pel ? "melLogic" : "pelLogic") + "：那一個極限邏輯照驅動器存的值");
            for (std::map<std::string, std::string>::const_iterator it = kv.begin(); it != kv.end(); ++it)
                if ((it->first == "pelLogic" || it->first == "melLogic") && it->second != "0" && it->second != "1")
                    c.Add(kAuditWarn, "T11", "Pci1203Axis.ini [" + s + "] " + it->first + "=" + it->second + " 不是 0／1：開機套用時會忽略（EtherCAT/Pci1203Control.cpp:2939-2950）");
        }
    }

    // ---------------- T12: IO 1203 stations vs Pci1203Modules.ini
    {
        std::set<int> ioStations;
        for (std::size_t i = 0; i < io.size(); ++i)
            if (io[i].effective && io[i].numeric && io[i].isa == 3 && IsInt(io[i].ip)) ioStations.insert(std::atoi(io[i].ip.c_str()));
        if (!ioStations.empty()) {
            if (!in.modulesIniFound) {
                c.Add(kAuditInfo, "T12", "沒有 Pci1203Modules.ini：IO 表的 1203 站號沒有比對（" + std::to_string(ioStations.size()) + " 個站）");
            } else {
                std::vector<Pci1203ModuleEntry> ents;
                std::string why;
                if (!Pci1203ModuleIniParse(in.modulesIni, ents, why)) {
                    c.Add(kAuditWarn, "T12", "Pci1203Modules.ini 讀不懂（" + why + "）：IO 表的 1203 站號沒有比對");
                } else {
                    std::set<int> listed;
                    for (std::size_t k = 0; k < ents.size(); ++k) listed.insert(ents[k].station);
                    std::vector<std::string> miss;
                    for (std::set<int>::const_iterator it = ioStations.begin(); it != ioStations.end(); ++it) if (!listed.count(*it)) miss.push_back(std::to_string(*it));
                    if (!miss.empty()) c.Add(kAuditWarn, "T12", "IO 表 Enable 1 的 1203 站號不在 Pci1203Modules.ini 裡：" + Join(miss, 32));
                }
            }
        }
    }

    // ---------------- T10: names the code binds that the tables lack
    {
        std::vector<std::string> m;
        for (std::size_t i = 0; i < in.codeMotors.size(); ++i) if (!motNames.count(in.codeMotors[i])) m.push_back(in.codeMotors[i]);
        if (!m.empty()) c.Add(kAuditInfo, "T10", "程式有命名、Mot_Table 沒有的馬達槽 " + std::to_string(m.size()) + " 個（停用的空殼，cinitial.cpp:4000-4021）：" + Join(m, 10));
        std::vector<std::string> n;
        for (std::size_t i = 0; i < in.codeIoNames.size(); ++i) if (!byAlias.count(in.codeIoNames[i]) && !ioAliasLine.count(in.codeIoNames[i])) n.push_back(in.codeIoNames[i]);
        if (!n.empty()) c.Add(kAuditInfo, "T10", "程式有用、IO_Table 沒有的名稱 " + std::to_string(n.size()) + " 個（停用）：" + Join(n, 10));
    }
    return c.r;
}

std::string TableAuditSummaryLine(const TableAuditResult& r)
{
    return "[BOOT] 驗表：ERROR " + std::to_string(r.errors) + "／WARN " + std::to_string(r.warns) + "（INFO " + std::to_string(r.infos) + "）"
           + (r.errors ? " -- 有 ERROR：表格要先改（AI(W906-E043)，op log 的 AUDIT 行列出是哪一列）" : "");
}

std::string TableAuditFindingLine(const TableAuditFinding& f)
{
    const char* lv = f.level == kAuditError ? "ERROR" : f.level == kAuditWarn ? "WARN" : "INFO";
    return std::string("TABLEAUDIT ") + lv + " " + f.rule + " " + f.text;
}

std::string TableAuditFirstError(const TableAuditResult& r)
{
    for (std::size_t i = 0; i < r.findings.size(); ++i)
        if (r.findings[i].level == kAuditError) return r.findings[i].rule + " " + r.findings[i].text;
    return std::string();
}

}  // namespace ht9045
