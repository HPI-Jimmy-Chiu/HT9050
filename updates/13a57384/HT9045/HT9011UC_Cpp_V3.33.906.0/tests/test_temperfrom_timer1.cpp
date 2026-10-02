// =============================================================================
//  test_temperfrom_timer1.cpp  --  AI(W906-TP1) 20261002 (Ifor01)
//
//  TP-1（docs/handoff/TO_IFOR.md §4 1002 08:5x；E019 §3.14）：golden TfTemperFrom 的 FormShow（:128-510）、Timer1Timer
//  （:1617-1682）、RecordTemp（:1975-1990）與移植的兩個入口（cTemperFrom.cpp 檔尾）。
//
//    [1] 開機那次 Show（W906_TemperFromBootShow＝golden DoShowUserDefFrom :8731-8732）：fTemperFrom 從 NULL 建出來、bShow、
//        Timer1 啟動；16 組加熱器時 ShowTempComp／NameTempComp 換到 _2 面板（:233-280），DUT 1 顆用 hlTempDut（:267-268）；
//        4 組＋DUT 4 顆換到 Dut_A1..A4、HeatGun _2（:300-313）；REAL_TIME_CCD＋RTC_TemperNumber<2 ⇒ CCD_2 面板藏起來（:185-192）；
//        CCD2_TEMPER ⇒ hlName2D_2 標題 "CCD2"（:456-457）；ESD ⇒ hlNameESD "ESD"（:468-469）；DUT 2 顆 ⇒ A3／A4 藏起來（:478-481）
//    [2] Timer1Timer：bShow=false／InitialOK=false 不做事（:1619-1622）；71 個通道的 Caption 照抄到 RunInfo.ShowTempComp（SECS，:1641）
//    [3] CCD 冷卻（:1645-1670）：ATC_SYSTEM>eATC60 且不是 eNonChamber ⇒ IO 頁沒開就 SW[SwCCDCooling].On()；IO 頁開著不動；
//        REAL_TIME_CCD 但 SwCCDCooling Enable=false ⇒ 不動（golden 先看 Enable）；兩者都不是 ⇒ 不動。只看 OutValue：
//        Enable=false 時 On() 只設欄位、不碰硬體（myswitch.cpp:74-87），不載 IO 表
//    [4] RecordTemp：bStartRecord 時每分鐘一行「HH:MM:SS, 71 個 Caption」（:1978-1988），同一分鐘不再加
//    [5] ht9045::W906_TemperFromTimer1Tick：Timer1 沒開不做事；開了照 VCL 第一次等一個 Interval（100 ms）
//    [6] 讀原始碼：wb_serve.cpp 在 W906_BootTestCategory 前、同一行呼叫 W906_TemperFromBootShow；WebBridgeTags.cpp 每拍呼叫
//        tick；ShowATCThermo 那行在 GATE (TP1-A)；ShowThermo 的 GATE (T1) 還是 #if 0（argv[1]＝移植樹根目錄，唯讀）
//    [7] 只動記憶體：D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 前後逐位元組相同
// =============================================================================
#include "forms/fTemperFrom.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"        // RunInfo
#include "Config.h"       // IniConfig
#include "LastSet.h"
#include "myswitch.h"     // SW[]
#include "atester_shims.h"// fiosetview
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

extern TPanel *ShowTempComp[tcTotalCount];
extern TPanel *NameTempComp[tcTotalCount];
void W906_TemperFromBootShow();
namespace ht9045 { void W906_TemperFromTimer1Tick(); }

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_temperfrom_timer1.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static bool Has(const std::string& s, const char* needle) { return s.find(needle)!=std::string::npos; }

static void SetSentinel() { for(int i=0; i<tcTotalCount; i++) RunInfo.ShowTempComp[i]="sentinel"; }
static bool AllSentinel() { for(int i=0; i<tcTotalCount; i++) if(RunInfo.ShowTempComp[i]!="sentinel") return false; return true; }

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("TemperFromTimer1"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::printf("test_temperfrom_timer1 (TP-1): golden TfTemperFrom FormShow / Timer1Timer / RecordTemp\n");
    const std::string sGen0=Slurp("D:\\HT9045\\system\\Gerneral.ini");
    const std::string sCfg0=Slurp("D:\\HT9045\\config\\config.ini");

    InitialOK=false;
    bSystemClose=false;
    ATC_SYSTEM=eATCUninstall;
    REAL_TIME_CCD=false;
    CCD2_TEMPER=false;
    Index_ESDAir=false;
    IniConfig.bShowFunctionWindow=false;

    // ---- [1] boot show ---------------------------------------------------------------------------
    std::printf("[1] boot show (golden DoShowUserDefFrom :8731-8732 -> FormShow)\n");
    CHECK(fTemperFrom==NULL, "[1] no form before the boot show (not created at static init: the ctor reads CUSTOMER_CODE)");
    USE_16_HEATER=eht16HeaterEJ1N;
    iSocketBaseTempCount=eDut1ea;
    W906_TemperFromBootShow();
    CHECK(fTemperFrom!=NULL, "[1] the boot show creates fTemperFrom");
    if(fTemperFrom==NULL) { std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail); return 1; }
    TfTemperFrom* f=fTemperFrom;
    CHECK(f->bShow==true, "[1] bShow (golden :130)");
    CHECK(f->Timer1->Enabled==true, "[1] Timer1 enabled (golden :509)");
    CHECK(ShowTempComp[tcHotPlate1]==f->hlTempPlate1_2 && NameTempComp[tcHotPlate1]==f->hlNamePlate1_2, "[1] 16 heaters: HotPlate1 -> the _2 panels (golden :233 / :272)");
    CHECK(ShowTempComp[tcShuttle2]==f->hlTempShuttle2_2 && ShowTempComp[tcSocket]==f->hlTempDut_2, "[1] 16 heaters: Shuttle2 / Socket -> _2 (golden :236 / :238)");
    CHECK(ShowTempComp[tcLB]==f->hlTempLB && ShowTempComp[tcLBUp]==f->hlTempLBUp, "[1] 16 heaters: LB / LB Up (golden :243-245)");
    CHECK(ShowTempComp[tcDUT1]==f->hlTempDut && NameTempComp[tcDUT1]==f->hlNameDut, "[1] DUT 1 ea: hlTempDut / hlNameDut (golden :267-268)");
    CHECK(NameTempComp[tcSocket]==f->hlNameDut_2, "[1] Socket name -> hlNameDut_2 (golden :277)");

    USE_16_HEATER=eht4Heater;
    iSocketBaseTempCount=eDut4ea;
    REAL_TIME_CCD=true; RTC_TemperNumber=1;
    CCD2_TEMPER=true; Index_ESDAir=true;
    W906_TemperFromBootShow();
    CHECK(fTemperFrom==f, "[1] a second show reuses the form");
    CHECK(ShowTempComp[tcDUT1]==f->hlTempDut_A1 && ShowTempComp[tcDUT4]==f->hlTempDut_A4, "[1] 4 heaters + DUT 4 ea: DUT1..4 -> Dut_A1..A4 (golden :307-310)");
    CHECK(ShowTempComp[tcHeatGun1]==f->hlTempHeatGun1_2 && NameTempComp[tcChamber]==f->hlNameChamber_3, "[1] 4 heaters + DUT 4 ea: HeatGun _2, Chamber _3 (golden :304 / :312)");
    CHECK(ShowTempComp[tcCCD_2]==f->hlTempCCD_2_2 && f->hlTempCCD_2_2->Visible==false && f->hlNameCCD_2_2->Visible==false, "[1] REAL_TIME_CCD, RTC_TemperNumber<2: CCD_2 panels hidden (golden :185-192)");
    CHECK(ShowTempComp[tcCCD]==f->hlTempCCD_3, "[1] REAL_TIME_CCD, 4 heaters + DUT 4 ea: CCD -> _3 (golden :200-201)");
    CHECK(f->hlName2D_2->Caption=="CCD2" && NameTempComp[tc2D]==f->hlName2D_2, "[1] CCD2_TEMPER: hlName2D_2 'CCD2' (golden :456-457)");
    CHECK(f->hlNameESD->Caption=="ESD" && NameTempComp[tcIndexESD]==f->hlNameESD, "[1] Index_ESDAir: hlNameESD 'ESD' (golden :468-469)");
    iSocketBaseTempCount=eDut2ea;
    W906_TemperFromBootShow();
    CHECK(f->hlTempDut_A3->Visible==false && f->hlNameDut_A4->Visible==false, "[1] DUT 2 ea: A3 / A4 hidden (golden :478-481)");
    REAL_TIME_CCD=false; CCD2_TEMPER=false; Index_ESDAir=false;
    USE_16_HEATER=eht16HeaterEJ1N; iSocketBaseTempCount=eDut1ea;
    W906_TemperFromBootShow();

    // ---- [2] Timer1Timer guards and the SECS strings ----------------------------------------------
    std::printf("[2] Timer1Timer\n");
    SetSentinel();
    InitialOK=false;
    f->Timer1Timer();
    CHECK(AllSentinel(), "[2] InitialOK=false -> nothing (golden :1621)");
    InitialOK=true;
    f->bShow=false;
    f->Timer1Timer();
    CHECK(AllSentinel(), "[2] bShow=false -> nothing (golden :1619)");
    f->bShow=true;
    f->Timer1Timer();
    bool bCopy=true;
    for(int i=tcHotPlate1; i<tcTotalCount; i++)
        if(RunInfo.ShowTempComp[i]!=ShowTempComp[i]->Caption) bCopy=false;
    CHECK(bCopy, "[2] all 71 RunInfo.ShowTempComp = the channel panel captions (golden :1641, SECS)");
    CHECK(!AllSentinel(), "[2] the captions were written");

    // ---- [3] CCD cooling --------------------------------------------------------------------------
    std::printf("[3] CCD cooling (golden :1645-1670)\n");
    SW[SwCCDCooling].Enable=false;
    SW[SwCCDCooling].OutValue=false;
    ATC_SYSTEM=eNewATCSystem; REAL_TIME_CCD=false;
    fiosetview->fShow=false;
    f->Timer1Timer();
    CHECK(SW[SwCCDCooling].OutValue==true, "[3] ATC_SYSTEM > eATC60, IO page closed -> SW[SwCCDCooling].On() (golden :1666-1669)");
    SW[SwCCDCooling].OutValue=false;
    fiosetview->fShow=true;
    f->Timer1Timer();
    CHECK(SW[SwCCDCooling].OutValue==false, "[3] IO page open -> untouched (golden :1668)");
    fiosetview->fShow=false;
    ATC_SYSTEM=eNonChamber;
    f->Timer1Timer();
    CHECK(SW[SwCCDCooling].OutValue==false, "[3] eNonChamber -> untouched");
    ATC_SYSTEM=eATCUninstall; REAL_TIME_CCD=true;
    SW[SwCCDCooling].OutValue=true;
    f->Timer1Timer();
    CHECK(SW[SwCCDCooling].OutValue==true, "[3] REAL_TIME_CCD with SwCCDCooling Enable=false -> neither On nor Off (golden checks Enable, :1653 / :1661)");
    REAL_TIME_CCD=false;
    SW[SwCCDCooling].OutValue=false;
    f->Timer1Timer();
    CHECK(SW[SwCCDCooling].OutValue==false, "[3] no RTC, no ATC 6.0+ -> untouched");

    // ---- [4] RecordTemp ---------------------------------------------------------------------------
    std::printf("[4] RecordTemp (golden :1975-1990)\n");
    f->bStartRecord=true;
    f->iStartMin=-1;
    SystemHour=10; SystemMin=20; SystemSec=30;
    const int n0=f->Memo1->Lines->Count;
    f->Timer1Timer();
    CHECK(f->Memo1->Lines->Count==n0+1, "[4] a new minute -> one line");
    if(f->Memo1->Lines->Count==n0+1)
    {
        const AnsiString aline=f->Memo1->Lines->Strings[n0];
        const std::string line(aline.c_str());
        int commas=0; for(size_t i=0; i<line.size(); i++) if(line[i]==',') commas++;
        CHECK(line.compare(0, 8, "10:20:30")==0, "[4] the line starts with HH:MM:SS");
        CHECK(commas==tcTotalCount, "[4] 71 captions after the time");
    }
    f->Timer1Timer();
    CHECK(f->Memo1->Lines->Count==n0+1, "[4] same minute -> no second line");
    SystemMin=21;
    f->Timer1Timer();
    CHECK(f->Memo1->Lines->Count==n0+2, "[4] next minute -> another line");
    f->bStartRecord=false;

    // ---- [5] the tick ----------------------------------------------------------------------------
    std::printf("[5] ht9045::W906_TemperFromTimer1Tick\n");
    f->Timer1->Enabled=false;
    SetSentinel();
    ht9045::W906_TemperFromTimer1Tick(); ::Sleep(150); ht9045::W906_TemperFromTimer1Tick();
    CHECK(AllSentinel(), "[5] Timer1 disabled -> no call");
    f->Timer1->Enabled=true;
    ht9045::W906_TemperFromTimer1Tick();
    CHECK(AllSentinel(), "[5] first tick after enable waits one Interval (VCL)");
    ::Sleep(150);
    ht9045::W906_TemperFromTimer1Tick();
    CHECK(!AllSentinel(), "[5] due tick runs Timer1Timer");
    bSystemClose=true;
    SetSentinel();
    ::Sleep(150);
    ht9045::W906_TemperFromTimer1Tick();
    CHECK(AllSentinel(), "[5] after FormClose (bSystemClose) -> no call");
    bSystemClose=false;

    // ---- [6] source pins --------------------------------------------------------------------------
    std::printf("[6] source pins (%s)\n", root.c_str());
    const std::string sWs=Slurp(root+"/tools/wb_serve.cpp");
    const size_t pT=sWs.find("W906_TemperFromBootShow(); }"), pC=sWs.find("W906_BootTestCategory(); }");
    CHECK(pT!=std::string::npos && pC!=std::string::npos && pT<pC && sWs.substr(pT, pC-pT).find('\n')==std::string::npos,
          "[6] wb_serve: W906_TemperFromBootShow right before W906_BootTestCategory, same line (golden DoShowUserDefFrom order)");
    const std::string sWb=Slurp(root+"/WebBridgeTags.cpp");
    CHECK(Has(sWb, "W906_TemperFromTimer1Tick(); }"), "[6] WebBridgeTags.cpp PumpTick calls W906_TemperFromTimer1Tick");
    const std::string sTf=Slurp(root+"/cTemperFrom.cpp");
    const size_t pA=sTf.find("#if 0 // GATE (TP1-A)");
    CHECK(pA!=std::string::npos && sTf.find("fLotInfo->ShowATCThermo();", pA)!=std::string::npos, "[6] ShowATCThermo stays in GATE (TP1-A) (ATC on hold, s0 #48)");
    CHECK(Has(sTf, "#if 0 // GATE (T1)"), "[6] ShowThermo's GATE (T1) (WAR15xx alarms) still closed");

    // ---- [7] machine files ------------------------------------------------------------------------
    CHECK(Slurp("D:\\HT9045\\system\\Gerneral.ini")==sGen0, "[7] Gerneral.ini unchanged");
    CHECK(Slurp("D:\\HT9045\\config\\config.ini")==sCfg0, "[7] config.ini unchanged");

    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
