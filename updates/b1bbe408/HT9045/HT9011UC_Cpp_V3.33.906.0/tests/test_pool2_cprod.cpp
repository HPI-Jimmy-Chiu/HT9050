// =============================================================================
//  test_pool2_cprod.cpp  --  AI(W906-POOL2) 20261008 (Ifor01)
//
//  POOL-2 `cprod.cpp`（FROM_IFOR §1 1008 00:1x）：檔裡 17 個 `#if 0 // TODO(GA1-B2)` 閘在 1008 main 逐個對 golden 0618 看過——
//  6 個卡住的東西已經有了、照 golden 原行改開（`#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GA1-B2 reason expired`）：
//    VTEST jam-rate 檔名（fMesSystem，golden :1182）、Murata 2DID 三欄（golden :2321）、
//    coLevelMode 決定 RMS 路徑（golden :3263）、HSys.MyGem->UpdateDataPath ×3（golden :2165／:2364／:3001）；
//  11 個保持關著（1009 起 10 個，slEventLog 由 W-195 TESNA T2 打開，AI(W906-W195) 20261009 (St02-E) TESNA T2）：FileInfo ×2（程式庫分割，已另用 hook 接）、fSpeed->ReadFile（本體有了，但 cprod.cpp 在 ht9045_globals，呼叫它會把
//  cSpeed.obj 連同 FileRW/Ld_UldDelayTime.cpp 拉進每支連 Prod 的 exe——試開時 279 支測試連結失敗）、FormHS（全域是 SCK_ART 測試替身）、ASECL 頁籤（缺 grpManualSet2D／
//  grp2DLotInfo）、slEventLog（1009 已開：W-195 TESNA T2）、ShowFunctions、GetColorSensor、pnlUnitSpeedDisplay（缺）、
//  HanaART SetHandlerWaitingData（未翻子系統的空樁）、fTemp_Set rgIndexHeatMode（vclcompat::TRadioGroup 沒有 Controls[]）。
//    [1] 讀原始碼（argv[1] = 移植樹根目錄）：6 個開閘註記各剛好一個、下一行是 golden 那一行；10 個關的仍是（1009 TESNA T2 起） `#if 0 // TODO(GA1-B2)`
//  只讀原始碼，不連專案程式庫。反向驗證見 MR 說明：任一個改回 `#if 0` ⇒ [1] 紅。
// =============================================================================
#include <cstdio>
#include <string>
#include <vector>

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_pool2_cprod.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::vector<std::string> Lines(const std::string& p)
{
    std::vector<std::string> out;
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return out;
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    size_t a=0, b;
    while(a<s.size())
    {
        b=s.find('\n', a);
        if(b==std::string::npos) b=s.size();
        std::string l=s.substr(a, b-a);
        if(!l.empty() && l[l.size()-1]=='\r') l.erase(l.size()-1);
        out.push_back(l);
        a=b+1;
    }
    return out;
}
static std::string Trim(const std::string& l)
{
    size_t a=l.find_first_not_of(" \t");
    return a==std::string::npos ? std::string() : l.substr(a);
}
static bool Starts(const std::string& s, const char* p) { return s.compare(0, std::string(p).size(), p)==0; }

struct Opened { const char* note; const char* golden; int count; };   // golden 0618 statement right under the gate
static const Opened kOpened[]={
    { "reason expired -- fMesSystem is ported",          "sJamRatePath=\"D:\\\\PnPh\\\\report\\\\LotAlarm\";", 1 },   // golden :1182
    { "reason expired -- TfLotInfo has edtLine",          "if(CUSTOMER_CODE==CC_Murata)", 1 },                        // golden :2321
    { "reason expired -- TfLotInfo has coLevelMode",      "if(fLotInfo->coLevelMode->Text!=\"Normal\")", 1 },         // golden :3263
    { "reason expired -- HTGem is complete",              "if(HSys.MyGem!=NULL)", 3 },                                // golden :2165 / :2364 / :3001
};
static const char* const kKept[]={            // the blocker named on the gate line (still `#if 0`)
    "blocked by FileInfo@ProductionInfo/FileInfo.h+.cpp (golden class,   [AI(W906-JAMDAY)",
    "blocked by FileInfo@ProductionInfo/FileInfo.h+.cpp (golden class,",
    "blocked by FormHS@HS_Function.h",
    "blocked by TfLotInfo@forms/fLotInfo.h missing members tsASECLEventLog/",
    // "blocked by TMyStringList@cmydef.h:15" -- opened by W-195③ TESNA T2
    "blocked by TfMain@forms/fMain.h missing member ShowFunctions",
    "blocked by TfTrayForm@forms/fTrayForm.h missing method GetColorSensor",
    "blocked by TfMain@forms/fMain.h missing member pnlUnitSpeedDisplay",
    "blocked by TfMainHanaART@forms/fMain.h missing method",
    "blocked by fTemp_Set@not declared anywhere",
    "blocked by TfSpeed::ReadFile -- the fSpeed FACADE now",
};

int main(int argc, char** argv)
{
    const std::string root=argc>1 ? argv[1] : "..";
    const std::vector<std::string> L=Lines(root+"/cprod.cpp");
    CHECK(L.size()>3000, "[1] cprod.cpp read (argv[1] = the port root)");
    std::printf("-- [1] source: 6 gates open as golden, 10 kept --\n");   // AI(W906-W195) 20261009 (St02-E) TESNA T2: 11 -> 10
    int opened=0;
    for(const Opened& o : kOpened)
    {
        int hits=0, good=0;
        for(size_t i=0; i+1<L.size(); i++)
            if(Starts(L[i], "#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GA1-B2 ") && L[i].find(o.note)!=std::string::npos)
            {
                hits++;
                size_t j=i+1;                             // the old gate's reason lines (`// ...`) stay under the gate
                while(j<L.size() && (Trim(L[j]).empty() || Starts(Trim(L[j]), "//"))) j++;
                if(j<L.size() && Starts(Trim(L[j]), o.golden)) good++;
            }
        char m[220];
        std::snprintf(m, sizeof(m), "[1] %s: %d opening note(s) (found %d), each followed by its golden statement (%d)", o.note, o.count, hits, good);
        CHECK(hits==o.count && good==o.count, m);
        opened+=hits;
    }
    CHECK(opened==6, "[1] 6 gates opened in all");
    int kept=0;
    for(const char* k : kKept)
    {
        int hits=0;
        for(const std::string& l : L)
            if(Starts(l, "#if 0 // TODO(GA1-B2): ") && l.find(k)!=std::string::npos) hits++;
        char m[220];
        std::snprintf(m, sizeof(m), "[1] kept closed: ...%s (found %d)", k, hits);
        // the two FileInfo gates share the shorter text: the JAMDAY one matches both patterns
        const int want=(std::string(k)=="blocked by FileInfo@ProductionInfo/FileInfo.h+.cpp (golden class,") ? 2 : 1;
        CHECK(hits==want, m);
        kept+=1;
    }
    int ga1=0;
    for(const std::string& l : L)
        if(Starts(l, "#if 0 // TODO(GA1-B2): ")) ga1++;
    char m[120];
    std::snprintf(m, sizeof(m), "[1] 10 GA1-B2 gates still `#if 0` (found %d)", ga1);   // AI(W906-W195) 20261009 (St02-E) TESNA T2: 11 -> 10
    CHECK(ga1==10, m);   // AI(W906-W195) 20261009 (St02-E) TESNA T2: slEventLog's gate lifted (cprod.cpp:2524)
    std::printf("test_pool2_cprod: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
