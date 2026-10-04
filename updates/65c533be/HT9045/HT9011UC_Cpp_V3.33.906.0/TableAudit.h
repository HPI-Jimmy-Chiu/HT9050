// =============================================================================
//  TableAudit.h  --  AI(W906-E043) 20261004 [W906] (St01 ST01-E2): the boot-time table audit (todo E-043, Steven 1004 07:2x Q96 = B;
//    design: docs/handoff/S23_FINDINGS_A1A2C5_20261004.md s5 on v906/steven-handoff, rule table approved by Steven 07:4x).
//    golden has no such check (not-golden, a new function).
//
//  WHAT: after IO_Table.csv / Mot_Table.csv are loaded and bound, re-read them as RAW text (the loader's atoi / atof already turned
//    a typo into 0, so the bound tables cannot show it) and report what the loader does silently: non-numeric cells, GearRatio 0,
//    a row forced to Enable 0 by an empty cell, duplicate 1203 axes / output points, cylinder enable mismatches, AUTO_EMPTY_COLOR
//    overrides, Pci1203Axis.ini / Pci1203Modules.ini mismatches, names the code uses that the tables lack.
//  LEVELS: ERROR = certain harm on this machine (Steven: an IO / motor device ERROR => the machine must not move);
//          WARN = probably wrong, logged only; INFO = counts only.
//  COMMIT 1 (this one, for the 10/05 machine run): log only -- console "[BOOT] ..." + op log "AUDIT" lines. Nothing is blocked.
//  COMMIT 2 (after 10/05 18:00, Steven 07:4x): HOME / START / manual moves refused while an ERROR exists; stop / abort never.
//
//  RULES (S-23 C5 table, final):
//    T1  ERROR  HT9050 with [System] IO_CARD_TYPE != 4; or IO_CARD_TYPE 2/3/4 and a table missing / its header rejected
//               (the loader then reads nothing and nothing binds)
//    T2a ERROR  an Enable-1 MOT row with a non-numeric number cell or GearRatio <= 0; an Enable-1 IO row with a non-numeric
//               Lane / ModuleType / IP / Port / Bit / InType / ISABase (atoi makes it a silent address 0)
//    T2b WARN   the same on an Enable-0 row, or a non-numeric alarm / delay time
//    T3  WARN   Enable 1 in the table, but an empty cell makes the loader force it to 0 (database.cpp :1980 / :3091-3094)
//    T4  ERROR  two Enable-1 PCI1203 MOT rows with the same (BoardID, Port) = station / sub-axis
//    T5a ERROR  two Enable-1 OUTPUT rows (Cylinder / Switch / Sucker_On / Sucker_Off) at the same (ISABase, IP, Port, Bit)
//    T5b WARN   two Enable-1 INPUT rows (Sensor / Cylinder_On / Cylinder_Off / Sucker) at the same address
//    T6  WARN   a cylinder whose output row and _On / _Off rows disagree on Enable (Cylinder[].Enable comes from the output row only)
//    T7  WARN   golden's AUTO_EMPTY_COLOR tail (cinitial.cpp:5242-5278) overrides an Empty / Color cylinder against its table row
//    T8  WARN   duplicate / empty Alias, short rows, a cell with a space (CommaText splits there), Mot header quirks
//    T9  ERROR  HT9050: an Enable-1 MOT row whose CardModel is not PCI1203 (no such card here)
//    T10 INFO   names the code binds that have no table row (counts + the first few)
//    T11 WARN   a Pci1203Axis.ini section that maps to no Enable-1 PCI1203 axis, has only one of pelLogic / melLogic, or a value != 0/1
//    T12 WARN   an Enable-1 1203 IO station (ISABase 3) that Pci1203Modules.ini does not list
//
//  Pure part (TableAuditRunPure) has no machine globals: tests/test_tableaudit.cpp feeds it text. The live part (TableAuditLive.cpp,
//    W906_TableAuditRun / W906_TableAuditSetNote) reads the files and globals; it is called from tools/wb_serve.cpp:4066 / :8340.
// =============================================================================
#ifndef HT9045_TABLEAUDIT_H
#define HT9045_TABLEAUDIT_H

#include <string>
#include <vector>

namespace ht9045 {

enum TableAuditLevel { kAuditInfo = 0, kAuditWarn = 1, kAuditError = 2 };

struct TableAuditFinding {
    int         level;      // TableAuditLevel
    std::string rule;       // "T1" .. "T12" ("T2a" / "T2b" / "T5a" / "T5b")
    std::string text;       // what and where (file, line, row name, value)
    TableAuditFinding() : level(kAuditInfo) {}
};

struct TableAuditInput {
    bool        ioFound, motFound;          // the files exist
    std::string ioCsv, motCsv;              // their raw text
    bool        ht9050;                     // W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050
    int         ioCardType;                 // [System] IO_CARD_TYPE
    int         autoEmptyColor;             // [System] AUTO_EMPTY_COLOR
    bool        useMrSystem;                // USE_MR_SYSTEM
    bool        loadZMotorEmpty, loadZMotorColor;   // LOAD_Z_USE_MOTOR[1] / [2]
    bool        axisIniFound;    std::string axisIni;      // D:\HT9045\config\Pci1203Axis.ini
    bool        modulesIniFound; std::string modulesIni;   // <system>\Pci1203Modules.ini
    std::vector<std::string> codeMotors;    // "M%02d" of every MOT slot the code names (T10)
    std::vector<std::string> codeIoNames;   // Cylinder / Sen / SW names the code binds (T10)
    TableAuditInput() : ioFound(false), motFound(false), ht9050(false), ioCardType(0), autoEmptyColor(0), useMrSystem(false),
                        loadZMotorEmpty(false), loadZMotorColor(false), axisIniFound(false), modulesIniFound(false) {}
};

struct TableAuditResult {
    std::vector<TableAuditFinding> findings;
    int errors, warns, infos;
    TableAuditResult() : errors(0), warns(0), infos(0) {}
};

TableAuditResult TableAuditRunPure(const TableAuditInput& in);
// "[BOOT] 驗表：ERROR n／WARN n（INFO n）" -- the boot summary line Steven asked for (Q96)
std::string      TableAuditSummaryLine(const TableAuditResult& r);
// "TABLEAUDIT ERROR T4 ..." -- one line per finding (console and op log)
std::string      TableAuditFindingLine(const TableAuditFinding& f);
// The first ERROR finding as "<rule> <text>" ("" = none) -- the refusal text commit 2 will use
std::string      TableAuditFirstError(const TableAuditResult& r);

}  // namespace ht9045

// Live (TableAuditLive.cpp; tools/wb_serve.cpp calls these):
void W906_TableAuditRun(bool has1203);                        // :4066, after InitialHandler + W906_BootSummary
void W906_TableAuditSetNote(void (*note)(const char*));       // :8340, inside W906_OpLogInit: flushes the buffered lines to the op log
int  W906_TableAuditErrorCount();                             // for commit 2's gates and the boot summary

#endif  // HT9045_TABLEAUDIT_H
