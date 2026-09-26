// ===========================================================================
//  tests/test_record_time_info.cpp
//
//  AI(W906-W2-3) 20260926: golden RecordTimeInfo（cObserver.cpp:1846-2134）→ cObserver_TimeInfo.cpp
//
//    [1] 起 01:02.300、止 01:04.800（2.5 s）⇒ TimeInfoGrid 第 11 列：Cells[1]=" 1: 2.300" 形式、Cells[3]＝ConvertMSecToSPC(2500)、
//        dTestSec＝2.5、RunInfo.dTestTimeSec＝2.5、RunInfo.TestTime＝Cells[3][11]（golden :2024）
//    [2] 再來一次 3.0 s ⇒ 歷史往上平移一列（第 10 列＝上一次）、平均列 Cells[3][14] 取非 0 的平均
//    [3] index cycle：第二次的開始 − 第一次的結束 ⇒ Cells[4][11]、RunInfo.IndexCycleTime＝Cells[4][11]、fObserver->dTotoalIndexCycleTime
//    [4] 跨小時（止分鐘 < 起分鐘）照 golden 加 60 分鐘
// ===========================================================================
#include <cstdio>
#include "forms/fObserver.h"
#include "cprod.h"
#include "cmydef.h"
#include "cpublic.h"
#include "vclcompat/vcl_compat.h"

struct TestTimeInfo { int iStartMin; int iStartSec; int iStartMSec; int iEndMin; int iEndSec; int iEndMSec; };
extern TestTimeInfo TestSocketTimeInfo[2];
void W906_RecordTimeInfoBody();

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_record_time_info.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static void Set(int sm, int ss, int sms, int em, int es, int ems)
{
    for (int i = 0; i < 2; ++i) {
        TestSocketTimeInfo[i].iStartMin = sm; TestSocketTimeInfo[i].iStartSec = ss; TestSocketTimeInfo[i].iStartMSec = sms;
        TestSocketTimeInfo[i].iEndMin = em;   TestSocketTimeInfo[i].iEndSec = es;   TestSocketTimeInfo[i].iEndMSec = ems;
    }
}

int main()
{
    CHECK(fObserver != 0);
    if (fObserver == 0) return 1;

    std::printf("[1] 2.5 s test -> row 11\n");
    Set(1, 2, 300, 1, 4, 800);
    W906_RecordTimeInfoBody();
    AnsiString s1 = fObserver->TimeInfoGrid->Cells[1][11];
    std::printf("    Cells[1][11]='%s' [2][11]='%s' [3][11]='%s' dTestSec=%.3f\n", s1.c_str(), fObserver->TimeInfoGrid->Cells[2][11].c_str(), fObserver->TimeInfoGrid->Cells[3][11].c_str(), dTestSec);
    CHECK(s1 == AnsiString("01: 2.300"));
    CHECK(fObserver->TimeInfoGrid->Cells[2][11] == AnsiString("01: 4.800"));
    CHECK(fObserver->TimeInfoGrid->Cells[3][11] == ConvertMSecToSPC(2500));
    CHECK(dTestSec > 2.499 && dTestSec < 2.501);
    CHECK(RunInfo.dTestTimeSec > 2.499 && RunInfo.dTestTimeSec < 2.501);
    CHECK(RunInfo.TestTime == fObserver->TimeInfoGrid->Cells[3][11]);

    std::printf("[2] a 3.0 s test shifts the history up one row\n");
    Set(1, 6, 0, 1, 9, 0);
    W906_RecordTimeInfoBody();
    CHECK(fObserver->TimeInfoGrid->Cells[3][10] == ConvertMSecToSPC(2500));
    CHECK(fObserver->TimeInfoGrid->Cells[3][11] == ConvertMSecToSPC(3000));
    CHECK(dTestSec > 2.999 && dTestSec < 3.001);

    std::printf("[3] index cycle = this start - previous end = 06.000 - 04.800 = 1.2 s\n");
    std::printf("    Cells[4][11]='%s' IndexCycleTime='%s' dTotoal=%.0f\n", fObserver->TimeInfoGrid->Cells[4][11].c_str(), RunInfo.IndexCycleTime.c_str(), fObserver->dTotoalIndexCycleTime);
    CHECK(fObserver->TimeInfoGrid->Cells[4][11] == AnsiString(" 1.200"));
    CHECK(RunInfo.IndexCycleTime == fObserver->TimeInfoGrid->Cells[4][11]);
    CHECK(fObserver->dTotoalIndexCycleTime == 1200);

    std::printf("[4] end minute < start minute wraps by 60 min (golden :1900-1903)\n");
    Set(59, 59, 0, 0, 1, 0);
    W906_RecordTimeInfoBody();
    CHECK(fObserver->TimeInfoGrid->Cells[3][11] == ConvertMSecToSPC(2000));

    std::printf("[5] AI(W906-OBS-DFM) ctor carries golden cObserver.dfm ChartYield values (:1941-1942, :1979, :1988, :1996..:2554)\n");
    CHECK(fObserver->ChartYield->LeftAxis->Maximum == 105 && fObserver->ChartYield->LeftAxis->Minimum == -5);
    CHECK(fObserver->edYieldMax->Text == AnsiString("100") && fObserver->edYieldMin->Text == AnsiString("0"));
    CHECK(fObserver->ChartYield->Series[0]->Title == AnsiString("Site Aa") && fObserver->ChartYield->Series[7]->Title == AnsiString("Site Ah"));
    CHECK(fObserver->ChartYield->Series[8]->Title == AnsiString("Site Ba") && fObserver->ChartYield->Series[31]->Title == AnsiString("Site Dh"));
    CHECK(fObserver->ChartYield->Series[0]->Active && fObserver->ChartYield->Series[31]->Active);

    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
