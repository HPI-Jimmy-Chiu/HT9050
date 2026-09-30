// ===========================================================================
//  JsonBridge/ChanMvTrays.h -- S118: the main-screen tray producer for web/page/Main.MotionView.html.
//
//  AI(W906-S118) 20260928 (St02-E).  NOT golden (a new producer); what it reads is golden's own tray state.
//
//  Golden: each main-screen tray panel is bound to a tray "motor" (906_0625_Steven cinitial.cpp:6430-6464 and
//  :6470-6471, e.g. `MOT[MMTrayY].SetHTrayPanel(fMain->mtLoader);`), and the panel draws that motor's TMyTray
//  (mytray.h: XItem / YItem, Data[col][row]).  In V906 the panel does not exist: TTrayMotor::SetHTrayPanel is a no-op
//  and pHTray stays NULL (Motor/mymotor.h:325-345).  The state is still there -- MOT[m].Tray -- so this producer
//  publishes it for the browser instead of a VCL panel.
//
//  Shape (agreed with the laptop): the page reads state.motionView.trays.<name>.cells[row][col]
//  (web/page/Main.MotionView.html liveCell / livePanel :952-970, trays.loader / hotPlate<n> / auto<n> / fix<n>
//  :1020-1031).  Tags are scalar, so every tray is ONE string tag holding its JSON:
//      motionView.trays.<name>  = {"name":"loader","xItem":10,"yItem":20,"ver":3,"cells":[[..row 0..],[..row 1..]]}
//      motionView.trays.ver     = int, +1 whenever any tray changed
//      motionView.trays.keys    = JSON array of the <name>s, in table order
//  cells[row][col] = the raw Tray.Data[col][row] (row < YItem, col < XItem) -- the item value, no display
//  adjustment (the TTMyTray colour / text do not exist in V906; the page treats 0 and 65535 as empty, liveHas :952).
//  "Only on change": a tray's JSON (and its ver) is rebuilt only when its size or one of its used cells differs from
//  the last tick (an exact compare of at most 30 x 70 ints); the patch protocol then sends the tag only then.
//
//  Which trays: every golden-bound MAIN-SCREEN (fMain) tray -- golden binds them on every machine type
//  (cinitial.cpp:6430-6471 test no machine type).  Not here: fAutoAlignment's two (:6468-6469) and
//  fObserveMagazine's fourteen (:6476-6489) -- other forms.  The HT9050 list is a question for Steven (ledger S118);
//  the page decides what to draw (AUTO_EMPTY_COLOR, the machine profile), as it does today.
//
//  Header-only and dependency-free: the table is an X-macro list of (name, motor constant, golden panel) -- this header
//  keeps the names as text, ChanMvTrays.cpp takes the addresses of the same cmydef.h constants from the same list, so
//  the two can never disagree -- and the change / JSON logic needs nothing, so the ctest (MotionView_Trays) covers them
//  without MOT[], the machine libraries or any file.  The MOT-reading stager is ChanMvTrays.cpp.
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_CHANMVTRAYS_H
#define HT9045_JSONBRIDGE_CHANMVTRAYS_H

#include <cstdio>
#include <string>
#include <vector>

namespace ht9045 {
namespace mvtray {

const int kMaxCol = 30;   // = _MAX_COL_ITEM (mytray.h); ChanMvTrays.cpp checks it
const int kMaxRow = 70;   // = _MAX_ROW_ITEM

// golden 906_0625_Steven cinitial.cpp:6430-6464, :6470-6471 (fMain panels), in golden order: X(name, motor, panel).
// The first fifteen page names are the ones the page and the old runtime contract use
// (web/JSON/Runtime-bridge-contract.json motionView.trays.keys); the rest are the golden panel names in camelCase.
#define HT9045_MV_TRAY_LIST(X)                              \
    X(hotPlate1,     MMPlate1,        mtPlate1)             \
    X(hotPlate2,     MMPlate2,        mtPlate2)             \
    X(loader,        MMTrayY,         mtLoader)             \
    X(loaderBuffer,  MMTrayY_Car,     mtLoaderBuffer)       \
    X(ocr,           MMOCR,           mtOCR)                \
    X(fix1,          MManualTray1,    mtFix1)               \
    X(fix2,          MManualTray2,    mtFix2)               \
    X(fix3,          MManualTray3,    mtFix3)               \
    X(fix4,          MManualTray4,    mtFix4)               \
    X(fix5,          MManualTray5,    mtFix5)               \
    X(fix6,          MManualTray6,    mtFix6)               \
    X(auto1,         MMAuto1,         mtAuto1)              \
    X(auto2,         MMAuto2,         mtAuto2)              \
    X(auto3,         MMAuto3,         mtAuto3)              \
    X(auto4,         MMAuto4,         mtAuto4)              \
    X(auto5,         MMAuto5,         mtAuto5)              \
    X(auto6,         MMAuto6,         mtAuto6)              \
    X(auto1Car,      MMAuto1_Car,     mtAuto1Car)           \
    X(auto2Car,      MMAuto2_Car,     mtAuto2Car)           \
    X(auto3Car,      MMAuto3_Car,     mtAuto3Car)           \
    X(auto4Car,      MMAuto4_Car,     mtAuto4Car)           \
    X(auto5Car,      MMAuto5_Car,     mtAuto5Car)           \
    X(auto6Car,      MMAuto6_Car,     mtAuto6Car)           \
    X(empty,         MMEmpty,         mtEmpty)              \
    X(emptyCar,      MMEmpty_Car,     mtEmpty_Car)          \
    X(color,         MMColor,         mtColor)              \
    X(colorCar,      MMColor_Car,     mtColor_Car)          \
    X(empty1,        MMEmpty1,        mtEmpty1)             \
    X(empty1Car,     MMEmpty1_Car,    mtEmpty1_Car)         \
    X(autoClean,     MMAutoCleanKit,  tmyAutoClean)         \
    X(inRotateKit,   MInRotateKit,    tmyInputRotateKit)    \
    X(outRotateKit,  MOutRotateKit,   tmyOutputRotateKit)   \
    X(inArmAoaTray,  MMInArmAOATray,  mtInArmAutoAlignmentTray)  \
    X(outArmAoaTray, MMOutArmAOATray, mtOutArmAutoAlignmentTray)

#define HT9045_MV_COUNT_ONE(n, m, p) +1
enum { kTrayCount = 0 HT9045_MV_TRAY_LIST(HT9045_MV_COUNT_ONE) };
#undef HT9045_MV_COUNT_ONE

struct TrayBinding
{
    const char* name;          // the key under state.motionView.trays
    const char* motor;         // the MOT[] index constant (cmydef.h), as text
    const char* goldenPanel;   // golden's fMain panel, for the ledger / tests
};

inline const std::vector<TrayBinding>& TrayBindings()
{
    static const TrayBinding k[] = {
#define HT9045_MV_TEXT(n, m, p) { #n, #m, #p },
        HT9045_MV_TRAY_LIST(HT9045_MV_TEXT)
#undef HT9045_MV_TEXT
    };
    static const std::vector<TrayBinding> v(k, k + sizeof(k) / sizeof(k[0]));
    return v;
}

// one tray, as the page reads it; xItem / yItem are clamped to the array (golden SetXYItem clamps the same way)
inline int ClampItems(int v, int maxV) { return v < 0 ? 0 : (v > maxV ? maxV : v); }

inline std::string TrayJson(const std::string& name, int xItem, int yItem, unsigned long ver,
                            const int (*data)[kMaxRow])
{
    const int nx = ClampItems(xItem, kMaxCol), ny = ClampItems(yItem, kMaxRow);
    char b[96];
    std::snprintf(b, sizeof(b), "\",\"xItem\":%d,\"yItem\":%d,\"ver\":%lu,\"cells\":[", nx, ny, ver);
    std::string o = "{\"name\":\"" + name + b;
    for (int r = 0; r < ny; ++r)
    {
        if (r) o += ',';
        o += '[';
        for (int c = 0; c < nx; ++c)
        {
            if (c) o += ',';
            std::snprintf(b, sizeof(b), "%d", data[c][r]);   // cells[row][col] = Data[col][row]
            o += b;
        }
        o += ']';
    }
    return o + "]}";
}

// what the producer remembers about one tray between ticks
struct TrayState
{
    bool seen;                 // published at least once
    int xItem, yItem;          // clamped size last published
    std::vector<int> cells;    // the used cells last published, row by row
    unsigned long ver;         // +1 on every change (1 = first publish)
    std::string json;          // TrayJson of that state -- re-staged every tick, rebuilt only on change
    TrayState() : seen(false), xItem(0), yItem(0), ver(0) {}
};

// the tray's cells now; true = changed since the last call (ver +1, json rebuilt).  The first call is a change.
inline bool UpdateTray(TrayState& st, const std::string& name, int xItem, int yItem, const int (*data)[kMaxRow])
{
    const int nx = ClampItems(xItem, kMaxCol), ny = ClampItems(yItem, kMaxRow);
    bool changed = !st.seen || nx != st.xItem || ny != st.yItem;
    if (!changed)
    {
        size_t k = 0;
        for (int r = 0; r < ny && !changed; ++r)
            for (int c = 0; c < nx; ++c, ++k)
                if (st.cells[k] != data[c][r]) { changed = true; break; }
    }
    if (!changed)
        return false;
    st.seen = true;
    st.xItem = nx;
    st.yItem = ny;
    st.cells.assign((size_t)nx * (size_t)ny, 0);
    size_t k = 0;
    for (int r = 0; r < ny; ++r)
        for (int c = 0; c < nx; ++c, ++k)
            st.cells[k] = data[c][r];
    ++st.ver;
    st.json = TrayJson(name, nx, ny, st.ver, data);
    return true;
}

// motionView.trays.keys
inline std::string TrayKeysJson()
{
    const std::vector<TrayBinding>& t = TrayBindings();
    std::string o = "[";
    for (size_t i = 0; i < t.size(); ++i)
    {
        if (i) o += ',';
        o += '"';
        o += t[i].name;
        o += '"';
    }
    return o + "]";
}

}  // namespace mvtray
}  // namespace ht9045

#endif
