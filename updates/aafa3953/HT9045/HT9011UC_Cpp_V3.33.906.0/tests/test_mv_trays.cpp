// =============================================================================
//  test_mv_trays.cpp -- S118: the main-screen tray producer's table, JSON and "only on change" rule
//  (JsonBridge/ChanMvTrays.h, header-only).
//
//  AI(W906-S118) 20260928 (St02-E).  Suite name (add_test): MotionView_Trays
//
//    1. the binding table = golden 906_0625_Steven cinitial.cpp:6430-6464 / :6470-6471 (fMain panels): 34 trays,
//       unique names, motor constants and panels, the fifteen names the page reads, spot checks (by name);
//    2. the JSON: cells[row][col] = Data[col][row], only the XItem x YItem used cells, raw values (0, 65535,
//       negatives), the size clamp; the keys list;
//    3. only on change: the first update publishes (ver 1); the same cells again do not; a used cell or the size
//       changing does (ver +1, JSON rebuilt); a cell outside the used area does not.
//  Memory only: no MOT[], no file, no socket.  (The MOT-reading stager is JsonBridge/ChanMvTrays.cpp.)
// =============================================================================
#include "JsonBridge/ChanMvTrays.h"
#include <cstdio>
#include <cstring>
#include <set>
#include <string>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

using namespace ht9045::mvtray;

static int g_data[kMaxCol][kMaxRow];

static const TrayBinding* Find(const char* name)
{
    const std::vector<TrayBinding>& t = TrayBindings();
    for (size_t i = 0; i < t.size(); ++i)
        if (std::strcmp(t[i].name, name) == 0) return &t[i];
    return 0;
}

int main()
{
    printf("MotionView_Trays\n");

    // 1. the table
    {
        const std::vector<TrayBinding>& t = TrayBindings();
        std::set<std::string> names, motors, panels;
        for (size_t i = 0; i < t.size(); ++i)
        {
            names.insert(t[i].name);
            motors.insert(t[i].motor);
            panels.insert(t[i].goldenPanel);
        }
        CHECK(t.size() == 34 && kTrayCount == 34 && names.size() == 34 && motors.size() == 34 && panels.size() == 34,
              "1. 34 golden fMain bindings, unique names, motors and panels (the count the stager checks too)");
        static const char* const kPage[] = { "loader", "hotPlate1", "hotPlate2", "auto1", "auto2", "auto3", "auto4",
                                             "auto5", "auto6", "fix1", "fix2", "fix3", "fix4", "fix5", "fix6" };
        bool page = true;
        for (size_t i = 0; i < sizeof(kPage) / sizeof(kPage[0]); ++i)
            if (!Find(kPage[i])) page = false;
        CHECK(page, "1. the fifteen names Main.MotionView reads (trays.loader / hotPlate<n> / auto<n> / fix<n>)");
        const TrayBinding* lo = Find("loader");
        const TrayBinding* h1 = Find("hotPlate1");
        const TrayBinding* f4 = Find("fix4");
        const TrayBinding* a6 = Find("auto6");
        const TrayBinding* rk = Find("outRotateKit");
        CHECK(lo && std::strcmp(lo->motor, "MMTrayY") == 0 && std::strcmp(lo->goldenPanel, "mtLoader") == 0 &&
                  h1 && std::strcmp(h1->motor, "MMPlate1") == 0 && f4 && std::strcmp(f4->motor, "MManualTray4") == 0 &&
                  a6 && std::strcmp(a6->motor, "MMAuto6") == 0 && rk && std::strcmp(rk->goldenPanel, "tmyOutputRotateKit") == 0,
              "1. MOT[MMTrayY] -> mtLoader, MMPlate1 -> hotPlate1, MManualTray4 -> fix4, MMAuto6 -> auto6 (cinitial.cpp:6430-6464)");
        CHECK(!Find("magazineTray1") && !Find("alignmentTray"),
              "1. fObserveMagazine / fAutoAlignment trays are not main-screen trays (:6468-6469, :6476-6489)");
        CHECK(TrayKeysJson().compare(0, 34, "[\"hotPlate1\",\"hotPlate2\",\"loader\",") == 0 &&
                  TrayKeysJson()[TrayKeysJson().size() - 1] == ']',
              "1. motionView.trays.keys lists the names in table order");
    }

    // 2. the JSON
    {
        std::memset(g_data, 0, sizeof(g_data));
        for (int c = 0; c < kMaxCol; ++c)
            for (int r = 0; r < kMaxRow; ++r)
                g_data[c][r] = c * 100 + r;
        CHECK(TrayJson("t", 3, 2, 1, g_data) == "{\"name\":\"t\",\"xItem\":3,\"yItem\":2,\"ver\":1,\"cells\":[[0,100,200],[1,101,201]]}",
              "2. cells[row][col] = Data[col][row], only XItem x YItem");
        g_data[0][0] = 65535;
        g_data[1][0] = -1;
        CHECK(TrayJson("t", 2, 1, 7, g_data) == "{\"name\":\"t\",\"xItem\":2,\"yItem\":1,\"ver\":7,\"cells\":[[65535,-1]]}",
              "2. raw values as they are (65535, negatives): no display adjustment");
        CHECK(TrayJson("t", 0, 5, 1, g_data) == "{\"name\":\"t\",\"xItem\":0,\"yItem\":5,\"ver\":1,\"cells\":[[],[],[],[],[]]}" &&
                  TrayJson("t", 3, 0, 1, g_data) == "{\"name\":\"t\",\"xItem\":3,\"yItem\":0,\"ver\":1,\"cells\":[]}",
              "2. an unsized tray: empty rows / no rows");
        const std::string big = TrayJson("t", 99, -4, 1, g_data);
        CHECK(big == "{\"name\":\"t\",\"xItem\":30,\"yItem\":0,\"ver\":1,\"cells\":[]}",
              "2. the size is clamped to the array (30 x 70, never negative)");
        const std::string full = TrayJson("t", 99, 99, 1, g_data);
        CHECK(full.find("\"xItem\":30,\"yItem\":70") != std::string::npos && full.find("2969") != std::string::npos &&
                  full.find("3000") == std::string::npos,
              "2. a full 30 x 70 tray: the last cell is Data[29][69] = 2969, nothing past the array");
    }

    // 3. only on change
    {
        std::memset(g_data, 0, sizeof(g_data));
        TrayState st;
        CHECK(UpdateTray(st, "loader", 4, 3, g_data) && st.ver == 1 && st.seen &&
                  st.json == TrayJson("loader", 4, 3, 1, g_data),
              "3. the first update publishes (ver 1)");
        const std::string first = st.json;
        CHECK(!UpdateTray(st, "loader", 4, 3, g_data) && st.ver == 1 && st.json == first,
              "3. the same cells again: no change, the same JSON (the tag patch sends nothing)");
        g_data[3][2] = 1;                                    // a used cell (col 3 < 4, row 2 < 3)
        CHECK(UpdateTray(st, "loader", 4, 3, g_data) && st.ver == 2 && st.json.find("[0,0,0,1]]") != std::string::npos,
              "3. a used cell changed: ver 2, JSON rebuilt");
        g_data[4][0] = 9;                                    // outside the used columns
        g_data[0][3] = 9;                                    // outside the used rows
        CHECK(!UpdateTray(st, "loader", 4, 3, g_data) && st.ver == 2,
              "3. a cell outside XItem x YItem changed: not a change");
        CHECK(UpdateTray(st, "loader", 5, 3, g_data) && st.ver == 3 && st.xItem == 5 &&
                  st.json.find("\"cells\":[[0,0,0,0,9],") != std::string::npos,
              "3. the size changed: ver 3, the new column appears");
        CHECK(UpdateTray(st, "loader", 0, 0, g_data) && st.ver == 4 && st.json.find("\"cells\":[]") != std::string::npos &&
                  !UpdateTray(st, "loader", 0, 0, g_data),
              "3. back to unsized: one change, then none");
        TrayState other;
        CHECK(UpdateTray(other, "auto1", 0, 0, g_data) && other.ver == 1 && other.json.find("\"name\":\"auto1\"") != std::string::npos,
              "3. every tray keeps its own version");
    }

    printf("MotionView_Trays: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
