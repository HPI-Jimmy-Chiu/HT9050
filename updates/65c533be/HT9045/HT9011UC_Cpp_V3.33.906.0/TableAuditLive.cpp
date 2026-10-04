// TableAuditLive.cpp -- AI(W906-E043) 20261004 [W906] (St01 ST01-E2): the live side of the boot table audit (TableAudit.h).
//   W906_TableAuditRun  (tools/wb_serve.cpp:4066, after InitialHandler + W906_BootSummary): reads IO_Table.csv / Mot_Table.csv as raw
//     text from the same paths the loader used (IoTablePath / MotTablePath, database.cpp:1701 / :1778), the two 1203 ini files and the
//     globals the rules need, runs TableAuditRunPure, prints "[BOOT] 驗表：ERROR n／WARN n" + one line per finding, and keeps them.
//   W906_TableAuditSetNote (:8340, inside W906_OpLogInit): the op log opens after the tables (:4305), so the kept lines are written
//     to it there ("AUDIT" lines; nothing when W906_OPLOG_DIR is unset, like every other op-log line).
//   COMMIT 1 = log only. Nothing here refuses HOME / START / a move (commit 2, after 10/05 18:00, Steven 1004 07:4x).
#include "vclcompat/vcl_compat.h"   // AnsiString
#include "cprod.h"                  // also MachineType.h (Type_HT9050, MAX_SENSOR_ITEM)
#include "cmydef.h"                 // IO_CARD_TYPE, AUTO_EMPTY_COLOR, USE_MR_SYSTEM, LOAD_Z_USE_MOTOR[9], MachineTypeChoice
#include "common.h"                 // IoTablePath, MotTablePath, asGeneralPath
#include "database.h"               // W906_GpibModel
#include "myswitch.h"               // SW[MAX_SWITCH_ITEM]
#include "mysensor.h"               // Sen[MAX_SENSOR_ITEM]
#include "mycylin.h"                // Cylinder[MaxCylinderItem]
#include "Motor/mymotor.h"          // MOT[MAX_TRAY_MOTOR]
#include "TableAudit.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

ht9045::TableAuditResult g_res;
bool                     g_ran = false;
std::vector<std::string> g_lines;                 // summary + findings, kept for the op log
void                   (*g_note)(const char*) = 0;

bool ReadAll(const std::string& path, std::string& out)
{
    out.clear();
    std::ifstream f(path.c_str(), std::ios::in | std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

std::string SystemDir()
{
    const std::string g = asGeneralPath.c_str();
    const std::size_t k = g.find_last_of("\\/");
    return k == std::string::npos ? std::string() : g.substr(0, k + 1);   // the same rule as wb_serve's Pci1203Modules.ini path
}

void Emit()
{
    if (!g_note) return;
    for (std::size_t i = 0; i < g_lines.size(); ++i) g_note(g_lines[i].c_str());
}

}  // namespace

void W906_TableAuditRun(bool has1203)
{
    (void)has1203;
    ht9045::TableAuditInput in;
    in.ioFound  = ReadAll(IoTablePath.c_str(), in.ioCsv);
    in.motFound = ReadAll(MotTablePath.c_str(), in.motCsv);
    in.ht9050          = W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050;   // = W906_MainCaption / ArmCellIsHt9050
    in.ioCardType      = IO_CARD_TYPE;
    in.autoEmptyColor  = AUTO_EMPTY_COLOR;
    in.useMrSystem     = USE_MR_SYSTEM != 0;
    in.loadZMotorEmpty = LOAD_Z_USE_MOTOR[1];
    in.loadZMotorColor = LOAD_Z_USE_MOTOR[2];
    in.axisIniFound    = ReadAll("D:\\HT9045\\config\\Pci1203Axis.ini", in.axisIni);   // = EtherCAT/Pci1203Control.cpp:109 kAxisIniPath
    in.modulesIniFound = ReadAll(SystemDir() + "Pci1203Modules.ini", in.modulesIni);
    char b[16];
    for (int i = 0; i < MAX_TRAY_MOTOR; ++i)
        if (!MOT[i].Alias.IsEmpty()) { std::snprintf(b, sizeof(b), "M%02d", i); in.codeMotors.push_back(b); }   // cinitial.cpp:3881-3882
    for (int i = 0; i < MaxCylinderItem; ++i)  if (!Cylinder[i].CylinderName.IsEmpty()) in.codeIoNames.push_back(Cylinder[i].CylinderName.c_str());
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i)  if (!Sen[i].Name.IsEmpty())             in.codeIoNames.push_back(Sen[i].Name.c_str());
    for (int i = 0; i < MAX_SWITCH_ITEM; ++i)  if (!SW[i].Name.IsEmpty())              in.codeIoNames.push_back(SW[i].Name.c_str());

    g_res = ht9045::TableAuditRunPure(in);
    g_ran = true;
    g_lines.clear();
    g_lines.push_back(ht9045::TableAuditSummaryLine(g_res));
    for (std::size_t i = 0; i < g_res.findings.size(); ++i) g_lines.push_back(ht9045::TableAuditFindingLine(g_res.findings[i]));
    for (std::size_t i = 0; i < g_lines.size(); ++i) std::printf("%s\n", g_lines[i].c_str());
    std::fflush(stdout);
    Emit();
}

void W906_TableAuditSetNote(void (*note)(const char*))
{
    g_note = note;
    if (g_ran) Emit();
}

int W906_TableAuditErrorCount() { return g_ran ? g_res.errors : 0; }
