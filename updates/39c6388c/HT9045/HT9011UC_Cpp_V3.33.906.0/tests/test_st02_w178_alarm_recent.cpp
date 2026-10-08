// =============================================================================
//  test_st02_w178_alarm_recent.cpp -- W-178 part 2 (contract 1.3.2): Alarm-dialog-request carries recent[] when 2 or more requests
//  are outstanding, so several alarms posted in one tick (W-175: the low-yield notices of one One Cycle finish) reach the page one by one.
//
//  AI(W906-W178) 20261008 (St02-E).  Suite name (add_test): St02_W178AlarmRecent.  Only tools/wb_dialog_mailbox.h (header-only) is
//  exercised, against a fresh %TEMP%\ht9045_w178_<tick> folder (removed when green).
//    [1] one request: the file is byte-identical to AlarmRequestJson (+ "\n") -- no "recent" (what every caller saw before).
//    [2] three notices in a row (no retire): top-level = the newest; recent[] = the three, oldest first, each a pending request.
//    [3] a blocking alarm on top: four.
//    [4] more than 8 outstanding: the newest 8.
//    [5] AlarmRetire: the idle JSON, byte-identical, no recent; the next post is a single request again.
//    [6] the js shim carries the same JSON.
// =============================================================================
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Public/cJSON.h"
#include "../tools/wb_dialog_mailbox.h"

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
std::string Slurp(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
std::string Str(cJSON* o, const char* k)
{
    cJSON* v = o ? cJSON_GetObjectItem(o, k) : 0;
    return (v && v->valuestring) ? v->valuestring : "";
}
double Num(cJSON* o, const char* k)
{
    cJSON* v = o ? cJSON_GetObjectItem(o, k) : 0;
    return v ? v->valuedouble : -1;
}
// "id1,id2,..." of recent[] (ascending-seq check folded in: returns "" when seq is not strictly ascending or an entry is not pending)
std::string RecentIds(cJSON* r)
{
    cJSON* a = r ? cJSON_GetObjectItem(r, "recent") : 0;
    if (!a) return "(none)";
    std::string ids;
    double last = -1;
    for (int i = 0; i < cJSON_GetArraySize(a); ++i) {
        cJSON* e = cJSON_GetArrayItem(a, i);
        if (Str(e, "state") != "pending" || Num(e, "seq") <= last) return "";
        last = Num(e, "seq");
        ids += (ids.empty() ? "" : ",") + Str(e, "requestId");
    }
    return ids;
}
}  // namespace

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W178AlarmRecent -- W-178 part 2: Alarm-dialog-request.recent[] (contract 1.3.2)\n");
    char tmp[MAX_PATH] = {0};
    ::GetTempPathA(MAX_PATH, tmp);
    const std::string dir = std::string(tmp) + "ht9045_w178_" + std::to_string((unsigned long)::GetTickCount());
    ::CreateDirectoryA(dir.c_str(), 0);
    ::CreateDirectoryA((dir + "\\js").c_str(), 0);
    const std::string aJson = dir + "\\Alarm-dialog-request.json", aShim = dir + "\\js\\Alarm-dialog-request.js";
    w906dlg::AlarmSlot slot;
    unsigned long long seq = 100;

    // ---------------------------------------------------------------- [1]
    std::printf("[1] one outstanding request: unchanged\n");
    Check(w906dlg::AlarmPost(slot, dir, seq, "1", "WAR0701", 0, 505, "", "WAR0701", true, "", "Site1"), "[1] AlarmPost");
    const std::string one = w906dlg::AlarmRequestJson(101, "1", "WAR0701", 0, 505, "", "WAR0701", true, "", "Site1", "");
    Check(Slurp(aJson) == one + "\n" && one.find("\"recent\"") == std::string::npos, "[1] the file is byte-identical to AlarmRequestJson, no recent[]");

    // ---------------------------------------------------------------- [2]
    std::printf("[2] three notices in one tick\n");
    w906dlg::AlarmPost(slot, dir, seq, "2", "WAR0702", 0, 505, "", "WAR0702", true, "", "Arm2");
    w906dlg::AlarmPost(slot, dir, seq, "3", "WAR0703", 0, 505, "", "WAR0703", true, "", "Site3");
    {
        cJSON* r = cJSON_Parse(Slurp(aJson).c_str());
        Check(r != 0, "[2] the file parses");
        Check(Str(r, "requestId") == "3" && Num(r, "seq") == 103, "[2] the top level is the newest (requestId 3, seq 103)");
        Check(RecentIds(r) == "1,2,3", "[2] recent[] = 1,2,3 -- oldest first, each pending, seq ascending (" + RecentIds(r) + ")");
        if (r) cJSON_Delete(r);
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] a blocking alarm on top\n");
    w906dlg::AlarmPost(slot, dir, seq, "4", "JAM0301", 1, 3, "", "JAM0301", true);
    {
        cJSON* r = cJSON_Parse(Slurp(aJson).c_str());
        Check(Str(r, "requestId") == "4" && RecentIds(r) == "1,2,3,4", "[3] four, the blocking one last (" + RecentIds(r) + ")");
        if (r) cJSON_Delete(r);
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] more than kPostedMax (8) outstanding\n");
    for (int i = 5; i <= 11; ++i) w906dlg::AlarmPost(slot, dir, seq, std::to_string(i), "WAR0799", 0, 505, "", "WAR0799", true);
    {
        cJSON* r = cJSON_Parse(Slurp(aJson).c_str());
        Check(RecentIds(r) == "4,5,6,7,8,9,10,11", "[4] the newest 8 (" + RecentIds(r) + ")");
        if (r) cJSON_Delete(r);
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] the js shim\n");
    {
        const std::string s = Slurp(aShim), j = Slurp(aJson);
        Check(s.find("\"recent\":[") != std::string::npos && j.size() > 1 && s.find(j.substr(0, j.size() - 1)) != std::string::npos,
              "[6] js\\Alarm-dialog-request.js carries the same JSON (recent[] included)");
    }

    // ---------------------------------------------------------------- [5]
    std::printf("[5] retire -> idle, then a single request again\n");
    Check(w906dlg::AlarmRetire(slot, dir, seq), "[5] AlarmRetire");
    Check(Slurp(aJson) == w906dlg::AlarmIdleJson(seq) + "\n" && Slurp(aJson).find("\"recent\"") == std::string::npos,
          "[5] the idle JSON, byte-identical, no recent[]");
    const unsigned long long next = seq + 1;
    w906dlg::AlarmPost(slot, dir, seq, "12", "WAR0701", 0, 505, "", "WAR0701", true, "", "Site1");
    Check(Slurp(aJson) == w906dlg::AlarmRequestJson(next, "12", "WAR0701", 0, 505, "", "WAR0701", true, "", "Site1", "") + "\n",
          "[5] the next post after idle is a single request again (byte-identical)");

    if (g_fail == 0) {
        ::DeleteFileA(aJson.c_str());
        ::DeleteFileA(aShim.c_str());
        ::RemoveDirectoryA((dir + "\\js").c_str());
        ::RemoveDirectoryA(dir.c_str());
    }
    std::printf("St02_W178AlarmRecent: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
