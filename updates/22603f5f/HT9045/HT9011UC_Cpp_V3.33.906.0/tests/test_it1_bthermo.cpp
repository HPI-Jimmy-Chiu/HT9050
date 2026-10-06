// =============================================================================
//  test_it1_bthermo.cpp  --  AI(W906-IT1) 20261006 (Ifor01)
//
//  IT-1 第一批（TO_IFOR §3 IT-1；FROM_IFOR §1 1006 09:2x）：bthermo.cpp 照 golden 0618 解開的 4 個 `#if 0`，在模擬組態真的有跑到。
//  機台是 HT9050 的客戶碼（`CUSTOMER_CODE=CC_PTI` 957：`TFormHS::CheckTempOffset` 175 度以上算超過機台能力），工作溫度 170，
//  另加起測補償讓「要送給溫控器的 SV」到 185（測試自己用 `GetFactSetTemp` 算，不猜補償的公式）：
//
//    [1] G19a 序列埠（`DoThermoReal`，假的 KT4H 只回寫入）：DUT 1 送出去的 SV＝工作溫度 170（不是 185），WAR15194 跳一次
//    [2] I-03b 的 DTM 通道表路徑（`W906_DtmMapExchange`）套同一個檢查：開 HT9050 表，DUT 2 的 DTM 面板設定 SV＝170
//    [3] G19c DTME08（`DoSetSVOfDTME08`，表沒開）：Index Aa1（DTM 通道 0）的面板設定 SV＝170，WAR15194 跳一次
//    [4] G19b EJ1N（`DoSetSVOfOmronEJ1N`）：WAR15194 跳一次，20 秒內再呼叫不重跳（它自己的 bHasErrorSet＋tEJ1NAlarmTimer）
//    [5] G25 ATC6.0（`DoATC60Temperature`）：兩拍後 `ATC_60_SYS.SetTemp`＝工作溫度；沒連線 ⇒ Index 32 組 UN150Read＝999
//    [6] 讀原始碼：bthermo.cpp 剩 11 個 `#if 0`；四處都是 `#if 1 // was: #if 0`；沒有程式行還在用全域 `FormHS->CheckTempOffset`
//    [7] 只動記憶體：D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 前後逐位元組相同
//  反向驗證（第 15 條）見 MR 說明：把任一處改回 `#if 0`（或拿掉 [2] 那段檢查）⇒ 對應那一節變紅。
//  不開 COM 埠（Comm2 強制 SIM）、不連網路（ATC6.0 的 socket 是 SIM）。約 10～30 秒。
// =============================================================================
#include <winsock2.h>
#include "atester_shims.h"                // COM2
#include "MachineType.h"
#include "cmydef.h"
#include "cpublic.h"
#include "cprod.h"                        // Temperature
#include "forms/fTemp_Set.h"              // InitTempOffset (extern const int; bthermo.cpp reaches it the same way)
#include "Config.h"                       // IniConfig
#include "CosFunction.h"
#include "LastSet.h"
#include "bthermo.h"                      // DoThermo / DoSetSVOfDTME08 / DoSetSVOfOmronEJ1N / DoATC60Temperature / iThermoTask
#include "acarry_shims.h"
#include "mysensor.h"
#include "myswitch.h"
#include "canary_support.h"               // W906_ShowErrorMessage_LastCode / _Count
#define HT9045_ATCINTERFACE_TCOLOR_EXTRA  // EJ1N/MyOmronPanel.h (via forms/fDTME08.h) defines clGray too -- same as bthermo.cpp
#include "ATC/ATCInterface.h"             // ATCInterfaceForm->ATC_60_SYS
#include "forms/fDTME08.h"
#include "EJ1N/DtmChannelMap.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <vector>

void W906_DTME08_CreateFormBoot();
double GetFactSetTemp(int Addr, double T);

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_it1_bthermo.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}

// The serial side: Comm2 in SIM.  A fake KT4H echoes every SV write (function 06) -- golden case 250 takes the echo as the
// acknowledgement -- and remembers the last value written to register 0001 per station (":%02X" = Addr+1).
struct FakeKT4H
{
    std::string acc;
    std::map<int, int> sv;
    void Serve()
    {
        const std::vector<char>& v=COM2->Comm2->SimTxBuffer();
        acc.append(v.begin(), v.end());
        COM2->Comm2->SimClearTx();
        for(;;)
        {
            const size_t a=acc.find(':');
            if(a==std::string::npos) { acc.clear(); return; }
            const size_t e=acc.find("\r\n", a);
            if(e==std::string::npos) { acc.erase(0, a); return; }
            const std::string f=acc.substr(a, e+2-a);
            acc.erase(0, e+2);
            if(f.size()<17) continue;
            if(f.substr(3, 2)=="06")
            {
                if(f.substr(5, 4)=="0001")
                    sv[(int)std::strtol(f.substr(1, 2).c_str(), 0, 16)-1]=(int)std::strtol(f.substr(9, 4).c_str(), 0, 16);
                COM2->Comm2->SimInjectReceive(f.data(), (Spcomm::Word)f.size());
            }
        }
    }
};

static FakeKT4H g_kt;
static bool Spin(DWORD ms, const std::function<bool()>& done)
{
    const DWORD t0=::GetTickCount();
    while(::GetTickCount()-t0<ms)
    {
        for(int k=0; k<16; k++) { DoThermo(); g_kt.Serve(); }
        if(done()) return true;
        ::Sleep(1);
    }
    return done();
}
static int CountWar15194Since(int c0, const AnsiString& lastBefore)
{
    (void)lastBefore;
    return (W906_ShowErrorMessage_LastCode==AnsiString("WAR15194")) ? (W906_ShowErrorMessage_Count-c0) : 0;
}
// raise Addr's start-up offset until GetFactSetTemp(Addr, base) >= target; returns the unclamped SV
static double PushOver(int Addr, double base, double target)
{
    Temperature.fTempOffSet[InitTempOffset][Addr]=0.0;
    const double c=GetFactSetTemp(Addr, base);
    Temperature.fTempOffSet[InitTempOffset][Addr]=target-c;
    return GetFactSetTemp(Addr, base);
}

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("IT1_Bthermo"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::printf("test_it1_bthermo (IT-1 batch 1): bthermo.cpp gates G19a / G19b / G19c / G25 opened\n");
    static const char* const kReal[2]={ "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini" };
    std::string before[2];
    for(int i=0; i<2; i++) before[i]=Slurp(kReal[i]);

    WSADATA wsa;
    ::WSAStartup(MAKEWORD(2, 2), &wsa);
    COM2->Comm2->SetSimMode(true);
    COM2->Comm2->StartComm();

    const double kBase=170.0;
    InitialOK=true;
    bSystemClose=false;
    PauseUT150Polling=false;
    CUSTOMER_CODE=CC_PTI;                                   // HT9050 (957): over the limit from 175 (forms/fHS.cpp CheckTempOffset)
    Tri_Temp_Machine=0;
    ATC_SYSTEM=0;
    ATC_InterfaceForm->iATC_MODE_TYPE=0;
    Temperature.bATCActiveCooling=false;
    LastSet.iTemperature=Tempture_Hot;
    Temperature.fWorkTemperBase=kBase;
    Temperature.bUseFixTemp=false;
    Temperature.bUseIndividualTemp=false;
    CosFunction.bUseIndividulTempSet=false;
    Temperature.iIndexHeatMode=HeadChamberSocket;
    IniConfig.bTemp25degControl=false;
    IniConfig.bEnableKT4HAlarm1=false;
    SHUTTLE_COOLING=0.0;
    bUseInitTempOffset=true;                                // GetFactSetTemp adds fTempOffSet[InitTempOffset][Addr] while
    iInitContactCount=0;                                    //   iInitContactCount < iCintactCntForTempOffsetAtInitial
    Temperature.iCintactCntForTempOffsetAtInitial=1;
    bTestOverTimeTempOffsetF=false;
    Sen[SnHeaterDoor].Enable=false;                         // no I/O: a disabled sensor is never "off" -> the heater door reads closed
    Sen[SnHeaterDoor2].Enable=false;
    SW[SwHeaterRelay].Enable=true;                          // heater relay ON (no I/O card: an unrouted base -> Status() = OutValue)
    SW[SwHeaterRelay].ISABase=-1;
    SW[SwHeaterRelay].Type=1;
    SW[SwHeaterRelay].OutValue=true;
    for(int i=0; i<tcTotalCount; i++) { bUT150Install[i]=false; UN150CommError[i]=false; UN150Read[i]=0; UN150ReadReal[i]=0; }

    // ---- [1] G19a: the serial write branch -------------------------------------------------------
    std::printf("[1] G19a -- DoThermoReal, KT4H\n");
    TC401HeaterControl=KT4H;
    USE_16_HEATER=eht4Heater;
    DtmMap_Select("");
    const double over1=PushOver(tcDUT1, kBase, 185.0);
    std::printf("    DUT 1: unclamped SV %.2f, work temperature %.2f\n", over1, kBase);
    CHECK(over1>=175.0 && over1!=kBase, "[1] precondition: DUT 1's SV is over the PTI limit (>= 175) and differs from the work temperature");
    bUT150Install[tcDUT1]=true;
    iThermoTask=1;
    Com2ReceiveOK=false;
    COM2->Comm2->SimClearTx();
    int c0=W906_ShowErrorMessage_Count;
    const bool b1=Spin(30000, []{ return g_kt.sv.count(tcDUT1)!=0; });
    CHECK(b1, "[1] the KT4H got an SV write for DUT 1");
    std::printf("    KT4H register 0001 for DUT 1 = %d\n", b1 ? g_kt.sv[tcDUT1] : -1);
    CHECK(b1 && g_kt.sv[tcDUT1]==(int)(kBase*10), "[1] DUT 1's SV = the work temperature x10 (1700), not the over-limit value -- G19a ran");
    CHECK(CountWar15194Since(c0, "")>=1, "[1] WAR15194 (temperature setting over the machine's limit) raised");

    // ---- [2] the DTM channel map path (I-03b) gets the same check ---------------------------------
    std::printf("[2] DTM channel map path (W906_DtmMapExchange)\n");
    USE_16_HEATER=eht32HeaterDTME08;
    W906_DTME08_CreateFormBoot();                            // 32 panels; no socket needed: the exchange writes the panel
    CHECK(frmDTME08->W906_Control()!=NULL && frmDTME08->GetChannelNumber()==32, "[2] DTM form built (32 channels)");
    DtmMap_Select("HT9050");
    const int ch2=DtmMap_ChannelOf(tcDUT2);
    const double over2=PushOver(tcDUT2, kBase, 185.0);
    CHECK(ch2==5 && over2>=175.0, "[2] precondition: DUT 2 = DTM channel 5, its SV over the limit");
    for(int i=0; i<tcTotalCount; i++) bUT150Install[i]=false;
    bUT150Install[tcDUT2]=true;
    iThermoTask=1;
    const bool b2=Spin(30000, [ch2]{ return frmDTME08->GetPalGroup(ch2)->GetSettingSV()!=0.0; });
    std::printf("    DTM panel %d setting SV = %.2f\n", ch2, frmDTME08->GetPalGroup(ch2)->GetSettingSV());
    CHECK(b2 && frmDTME08->GetPalGroup(ch2)->GetSettingSV()==kBase, "[2] mapped DUT 2's DTM setpoint = the work temperature (170), not the over-limit value");

    // ---- [3] G19c: DoSetSVOfDTME08 (map off) --------------------------------------------------------
    std::printf("[3] G19c -- DoSetSVOfDTME08\n");
    DtmMap_Select("");
    const double over3=PushOver(tcAa1, kBase, 185.0);
    CHECK(iTempCode[0]==tcAa1 && over3>=175.0, "[3] precondition: DTM channel 0 = Index Aa1, its SV over the limit");
    for(int i=0; i<tcTotalCount; i++) bUT150Install[i]=false;
    bUT150Install[tcAa1]=true;
    c0=W906_ShowErrorMessage_Count;
    DoSetSVOfDTME08();
    std::printf("    DTM panel 0 setting SV = %.2f\n", frmDTME08->GetPalGroup(0)->GetSettingSV());
    CHECK(frmDTME08->GetPalGroup(0)->GetSettingSV()==kBase, "[3] Aa1's DTM setpoint = the work temperature (170) -- G19c ran");
    CHECK(CountWar15194Since(c0, "")==1, "[3] WAR15194 raised once by DoSetSVOfDTME08 (its own bHasErrorSet)");

    // ---- [4] G19b: DoSetSVOfOmronEJ1N --------------------------------------------------------------
    std::printf("[4] G19b -- DoSetSVOfOmronEJ1N\n");
    USE_16_HEATER=eht16HeaterEJ1N;
    c0=W906_ShowErrorMessage_Count;
    DoSetSVOfOmronEJ1N();
    const int n4a=CountWar15194Since(c0, "");
    DoSetSVOfOmronEJ1N();
    CHECK(n4a==1, "[4] WAR15194 raised by the EJ1N path -- G19b ran");
    CHECK(W906_ShowErrorMessage_Count-c0==1, "[4] ... once: the second call within 20 s does not raise it again (tEJ1NAlarmTimer)");

    // ---- [5] G25: ATC6.0 / 3.0 ----------------------------------------------------------------------
    std::printf("[5] G25 -- DoATC60Temperature\n");
    ATC_SYSTEM=eATC60;
    ATCInterfaceForm->ATC_60_SYS.SetTemp=0;
    for(int a=tcAa1; a<=tcBd2; a++) UN150Read[a]=0.0;
    for(int k=0; k<3; k++) DoATC60Temperature();
    std::printf("    ATC_60_SYS.SetTemp = %.2f, UN150Read[tcAa1] = %.1f, [tcBd2] = %.1f\n", ATCInterfaceForm->ATC_60_SYS.SetTemp, UN150Read[tcAa1], UN150Read[tcBd2]);
    CHECK(ATCInterfaceForm->ATC_60_SYS.SetTemp==kBase, "[5] the sequencer's first step sent the work temperature (SetTargetTemperature)");
    CHECK(UN150Read[tcAa1]==999.0 && UN150Read[tcBd2]==999.0 && UN150Read[tcAd2]==999.0, "[5] ATC host not connected (SIM socket) -> Index UN150Read = 999 (golden :3304-3310)");
    ATC_SYSTEM=0;

    // ---- [6] source pins ----------------------------------------------------------------------------
    std::printf("[6] source\n");
    {
        const std::string bt=Slurp(root+"/bthermo.cpp");
        int nIf0=0;
        size_t p=0;
        while((p=bt.find("\n#if 0", p))!=std::string::npos) { nIf0++; p++; }
        CHECK(nIf0==11, "[6] bthermo.cpp: 11 `#if 0` left (15 before IT-1; G01 x2, G02, G15, G16, G18, G26a, G26b, G28a, G28b, G28c stay)");
        CHECK(bt.find("#if 1 // was: #if 0 // TODO(W7 G19a)")!=std::string::npos && bt.find("#if 1 // was: #if 0 // TODO(W7 G19b)")!=std::string::npos &&
              bt.find("#if 1 // was: #if 0 // TODO(W7 G19c)")!=std::string::npos && bt.find("#if 1 // was: #if 0 // TODO(W7-UI G25)")!=std::string::npos,
              "[6] G19a / G19b / G19c / G25 opened in place");
        bool bGlobal=false;
        size_t q=0;
        while((q=bt.find("FormHS->CheckTempOffset", q))!=std::string::npos)
        {
            const size_t ls=bt.rfind('\n', q);
            const std::string head=bt.substr(ls+1, q-ls-1);
            if(head.find("//")==std::string::npos && (q<3 || bt.compare(q-3, 3, "BT_")!=0)) bGlobal=true;
            q++;
        }
        CHECK(!bGlobal, "[6] no code line calls the global FormHS (SCK_ART_Remainder.h's stub) -- all go through BT_FormHS()");
        CHECK(bt.find("W906_DtmMapExchange(Addr, Temp, OldTemp[Addr], CommunCTErr[Addr], bHasErrorSet);")!=std::string::npos, "[6] the map exchange gets DoThermoReal's bHasErrorSet");
    }

    // ---- [7] real files ---------------------------------------------------------------------------
    for(int i=0; i<2; i++) CHECK(Slurp(kReal[i])==before[i], "[7] real machine file unchanged");

    ::WSACleanup();
    std::printf("test_it1_bthermo: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
