// =============================================================================
//  tests/test_compk_datas.cpp -- AI(W906-CSEL-NEEDREF / W906-OBS-TABVIS) 20261001: ctest CompK_DataS
//
//  Two C++ halves of the every-component check (EastSun 20261001) for the Data/Status pages:
//    A  Counter Selection -> golden TfMain::Timer1Timer's NeedRef reader (906 main.cpp:3208-3213 + DoShowUserDefFrom's
//       fTestCategory branch :8744-8752), FileRW/IniConfig_CounterSel.cpp FileRW_CounterSel_NeedRefTick():
//       NeedRef=false, SetShowCateMode (bCateByArm / Height from IniConfig.iShowCateByArm), FormShow when bShowTestCate
//       and not showing, FormClose when bShowTestCate is off; nothing at all while NeedRef is false.
//    B  Observer -> the TabVisible golden FormShow writes, as the page's tabVisible map (cObserver.cpp
//       W906_ObserverTabVisibleJson): only the sheets FormShow writes; the eight main sheets go false while
//       bShowMajorMaintenanceRecord; nullptr -> {}.
//    C  Observer Time Data -> AI(W906-OBS-TIMEDATA): lstTimeData's dfm Items until the first pgcMessage tab change, lstTimeDataClick
//       on a non-file -> "No Record!!", argument checks, pgcMessageChange's last-file selection (reads the TimeData year folder only).
//  Memory only: no golden FormShow of TfObserver / TfCounterSel runs (those read and seed machine files); nothing is
//  written (part C only lists the TimeData year folder, golden's own read). The guard still refuses outside ctest's redirect roots, the same rule as every newer test.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "Config.h"                 // IniConfig.iShowCateByArm / bShowTestCate
#include "forms/fCounterSel.h"      // fCounterSel->NeedRef
#include "forms/fTestCategory.h"    // fTestCategory
#include "forms/fObserver.h"        // fObserver / TfObserver
#include "w906_ctest_guard.h"

#include <cstdio>
#include <stdexcept>
#include <string>

void FileRW_CounterSel_NeedRefTick();                     // FileRW/IniConfig_CounterSel.cpp
std::string W906_ObserverTabVisibleJson(TfObserver *f);   // cObserver.cpp (end of file)
void W906_ObserverTimeDataHydrate(TfObserver *f);                                 // cObserver.cpp (end of file)
void W906_ObserverTimeDataAct(TfObserver *f, const std::string &act, int arg);    // cObserver.cpp (end of file)
std::string W906_ObserverTimeDataJson(TfObserver *f);                             // cObserver.cpp (end of file)
// the C-route shared layer (FileRW/_EditPage.cpp) takes wb_serve's form lock; single-threaded test -> no-op (as test_hsys_heater_mix.cpp:666)
namespace ht9045 { namespace formjson { void FormLock() {} void FormUnlock() {} } }
// link only (not under test): cprod.cpp (ht9045_globals) calls these two; the real ones are in FileRW/IniConfig.cpp / FileRW/HSys.cpp
// (not compiled here). Without them the archive pulls FileRW/_fallback.cpp, whose other members collide with the
// FileRW/_EditList.cpp compiled into this test (same as test_hsys_heater_mix.cpp:45-49).
void FileRW_IniConfig_ChangeCBListProperty() {}
vclcompat::AnsiString FileRW_HSys_CustomerName() { return vclcompat::AnsiString("HonPrec"); }

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_compk_datas.cpp:%d]  %s\n", line, what); }
    else     {           std::printf("  ok    %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

static bool Has(const std::string& s, const char* part) { return s.find(part) != std::string::npos; }

static void PartA_CounterSelNeedRef()
{
    std::printf("A  Counter Selection NeedRef reader (golden main.cpp:3208-3213)\n");
    CHECK(fCounterSel != nullptr && fTestCategory != nullptr);
    if (fCounterSel == nullptr || fTestCategory == nullptr) return;

    // A1: Exit with "By Arm" and Tester Category ON -> mode By Arm, window shown
    fCounterSel->NeedRef = true;
    IniConfig.iShowCateByArm = 1;
    IniConfig.bShowTestCate = true;
    fTestCategory->bShow = false;
    fTestCategory->bCateByArm = false;
    FileRW_CounterSel_NeedRefTick();
    CHECK(fCounterSel->NeedRef == false);                 // :3210
    CHECK(fTestCategory->bCateByArm == true);             // SetShowCateMode :3211
    CHECK(fTestCategory->Height == 192);                  // golden cTestCategory.cpp:511-525 (#else arm)
    CHECK(fTestCategory->bShow == true);                  // DoShowUserDefFrom :8746-8747 -> Show() -> FormShow

    // A2: NeedRef already consumed -> a later tick changes nothing
    IniConfig.iShowCateByArm = 0;
    IniConfig.bShowTestCate = false;
    FileRW_CounterSel_NeedRefTick();
    CHECK(fTestCategory->bCateByArm == true);
    CHECK(fTestCategory->bShow == true);

    // A3: Exit with "Normal" and Tester Category OFF -> mode Normal, window closed
    fCounterSel->NeedRef = true;
    FileRW_CounterSel_NeedRefTick();
    CHECK(fCounterSel->NeedRef == false);
    CHECK(fTestCategory->bCateByArm == false);
    CHECK(fTestCategory->Height == 105);
    CHECK(fTestCategory->bShow == false);                 // :8751 Close() -> FormClose

    // A4: Tester Category ON while already showing -> stays shown (golden: if(bShow==false) Show())
    fCounterSel->NeedRef = true;
    IniConfig.bShowTestCate = true;
    fTestCategory->bShow = true;
    FileRW_CounterSel_NeedRefTick();
    CHECK(fTestCategory->bShow == true);
}

static void PartB_ObserverTabVisible()
{
    std::printf("B  Observer tabVisible (golden FormShow cObserver.cpp:355-651)\n");
    CHECK(W906_ObserverTabVisibleJson(nullptr) == "{}");
    TfObserver* f = fObserver;
    CHECK(f != nullptr);
    if (f == nullptr) return;
    f->tsScanner->TabVisible = false;
    f->tsTemperature->TabVisible = true;
    f->tsMDB->TabVisible = true;
    f->tsLotInfo->TabVisible = false;                     // bSPILFunction off
    f->tsDataRecord->TabVisible = false;                  // B01 / B02 off
    f->tsPrecautionsRecord->TabVisible = false;
    f->tsPrecautionLog->TabVisible = false;
    f->tsHanderMajorMaintenance->TabVisible = false;
    f->bShowMajorMaintenanceRecord = false;
    std::string s = W906_ObserverTabVisibleJson(f);
    std::printf("     %s\n", s.c_str());
    CHECK(Has(s, "\"tsScanner\":false"));
    CHECK(Has(s, "\"tsTemperature\":true"));
    CHECK(Has(s, "\"tsMDB\":true"));
    CHECK(Has(s, "\"tsLotInfo\":false"));
    CHECK(Has(s, "\"tsDataRecord\":false"));
    CHECK(Has(s, "\"tsHanderMajorMaintenance\":false"));
    CHECK(!Has(s, "tsCounter"));                          // never written by FormShow outside :595-618 -> dfm TabVisible=True stays
    CHECK(!Has(s, "tsYield"));

    f->tsDataRecord->TabVisible = true;                   // B01 on
    f->tsPrecautionsRecord->TabVisible = true;
    f->bShowMajorMaintenanceRecord = true;                // golden :595-618
    s = W906_ObserverTabVisibleJson(f);
    std::printf("     %s\n", s.c_str());
    CHECK(Has(s, "\"tsDataRecord\":true"));
    CHECK(Has(s, "\"tsPrecautionsRecord\":true"));
    CHECK(Has(s, "\"tsCounter\":false"));
    CHECK(Has(s, "\"tsTestCate\":false"));
    CHECK(Has(s, "\"tsMDBQuery\":false"));
    CHECK(Has(s, "\"tsYield\":false"));
    CHECK(Has(s, "\"tsTestInfo\":false"));
    CHECK(Has(s, "\"tsOEE_ProductionInfor\":false"));
    f->bShowMajorMaintenanceRecord = false;
}

static bool Throws(TfObserver *f, const char *act, int arg)
{
    try { W906_ObserverTimeDataAct(f, act, arg); } catch (const std::invalid_argument &) { return true; }
    return false;
}

static void PartC_ObserverTimeData()
{
    std::printf("C  Observer Time Data (golden pgcMessageChange cObserver.cpp:4766-4793, lstTimeDataClick :4841-4844)\n");
    CHECK(W906_ObserverTimeDataJson(nullptr) == "{}");
    TfObserver* f = fObserver;
    if (f == nullptr) return;
    // C1: before any pgcMessage tab change the list holds the dfm design-time Items (cObserver.dfm:1798-1809), none selected
    W906_ObserverTimeDataHydrate(f);
    CHECK(f->lstTimeData->Items->Count == 11);
    CHECK(f->lstTimeData->Items->Count > 0 && std::string(f->lstTimeData->Items->Strings[0].c_str()) == "3333");
    CHECK(f->lstTimeData->ItemIndex == -1);
    // C2: clicking '3333' -> GetTimeDataText: not a file -> RowCount 2, Cells[1][1] "No Record!!" (golden :4830-4836)
    W906_ObserverTimeDataAct(f, "timeFile", 0);
    CHECK(f->lstTimeData->ItemIndex == 0);
    std::string s = W906_ObserverTimeDataJson(f);
    std::printf("     %s\n", s.substr(0, 200).c_str());
    CHECK(Has(s, "\"rows\":2"));
    CHECK(Has(s, "No Record!!"));
    CHECK(Has(s, "\"cols\":13"));                                              // dfm ColCount=13
    // C3: out-of-range arguments are refused, nothing runs
    CHECK(Throws(f, "timeFile", 11));
    CHECK(Throws(f, "timeFile", -1));
    CHECK(Throws(f, "msgTab", 4));
    // C4: a pgcMessage tab change re-lists the year folder (read only): last file selected, or -1 when there is none
    W906_ObserverTimeDataAct(f, "msgTab", 2);
    CHECK(f->pgcMessage->ActivePageIndex == 2);
    const int n = f->lstTimeData->Items->Count;
    CHECK(f->lstTimeData->ItemIndex == (n >= 1 ? n - 1 : -1));
    // C5: the dfm Items are not put back afterwards (the dfm loads once)
    W906_ObserverTimeDataHydrate(f);
    CHECK(f->lstTimeData->Items->Count == n);
}

int main()
{
    if (!W906TestRequireCtestRedirects("CompK_DataS"))
        return 2;
    PartA_CounterSelNeedRef();
    PartB_ObserverTabVisible();
    PartC_ObserverTimeData();
    std::printf("\n%d checks, %d failed\nRESULT: %s\n", g_checks, g_fail, g_fail ? "FAIL" : "ALL PASS");
    return g_fail ? 1 : 0;
}
