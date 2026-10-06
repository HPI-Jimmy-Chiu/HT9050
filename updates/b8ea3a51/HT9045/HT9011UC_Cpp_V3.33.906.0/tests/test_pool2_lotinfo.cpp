// =============================================================================
//  test_pool2_lotinfo.cpp  --  AI(W906-POOL2) 20261006 (Ifor01)
//
//  POOL-2（FROM_IFOR §1 1006 13:5x）：forms/fLotInfo.cpp 普查（IF0_CENSUS_20261006 更正版）第二節列的 11 個 `#if 0`，
//  逐個對 golden 0618 uLotInfo.cpp 之後只有 3 個理由真的過期、照 golden 解開：
//    WA-5 :1447  InitialRefrigerantSystem 呼叫 ATC_OFFLINE_FormComInit()（本體早就照 golden 翻好，:2737）
//    WA-9 :2011  btTesterTCPShowClick 的 fTesterTCP->Show()（fTesterTCP 從 St02 g023 起就有，Show() 是移植版 no-op）
//    WD-3 :4638  LotKeyInTimeTimer 的 CC_KYEC_LEE 作業員 ID 檢查鏈（缺的 fMain->cbRunStartMode 由 W906-P10 翻進來了）
//  其餘 8 個理由還成立、照留：WC-19 SetLotStart（SAFETY 紅線）、S117-ATKAMR／S25-TSMC／S25-UTAC（S25 客戶專屬，
//  RULINGS_20260925 S25）、S117-RSM ×2（模式切換，Jimmy）、LOT-W1-MAINPANELS／S117-MAINPANELS（同一對，等 Jimmy）。
//    [1] WA-5：AirStream_Select!=0 時 InitialRefrigerantSystem 把冷凍機面板設回離線值（"-999.0"、補償值隱藏、上下限字樣），
//        bInitFormcomponent 變 true（golden :14920 先清 false，:14922 呼叫的 ATC_OFFLINE_FormComInit 在 :14974 設 true）
//    [2] WA-9：fTesterTCP 存在、btTesterTCPShowClick() 呼叫得到、不當機
//    [3] WD-3：CUSTOMER_CODE=CC_KYEC_LEE 的作業員 ID：長度不對清掉、工號頭碼範圍（一般 85～120、KLT 3～31）、
//        AMR Loader＋Re-Test 模式的 "AGV" 保留、非 Re-Test 模式 "AGV" 清掉（這一支讀 fMain->cbRunStartMode->Text）
//    [4] 讀原始碼：fLotInfo.cpp 剩 81 個 `#if 0`（84－3）；三處是 `#if 1 // was: #if 0 -- opened AI(W906-POOL2)`；
//        照留的 8 個還是 `#if 0`
//    [5] 只動記憶體：D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 前後逐位元組相同
//  反向驗證（第 15 條）見 MR 說明：任一處改回 `#if 0` ⇒ [1]／[3]／[4] 紅。
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "forms/fLotInfo.h"
#include "forms/fMain.h"
#include "forms/fTesterTCP.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <string>

extern bool bLotFirstKeyIn;      // forms/fLotInfo.cpp:148 (golden uLotInfo.cpp:95)

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_pool2_lotinfo.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static int Count(const std::string& s, const char* needle)
{
    int n=0; size_t p=0;
    while((p=s.find(needle, p))!=std::string::npos) { n++; p++; }
    return n;
}

// one LotKeyInTimeTimer tick on the CC_KYEC_LEE branch with a 10-character Lot ID (so the Lot-ID half calls no SetLotID)
static std::string OperatorAfterTick(const char* op)
{
    fLotInfo->edtSysLotID->Text="0123456789";
    fLotInfo->edtSysOperatorID->Text=op;
    fLotInfo->LotKeyInTimeTimer();
    return std::string(fLotInfo->edtSysOperatorID->Text.c_str());
}

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("POOL2_LotInfo"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::printf("test_pool2_lotinfo (POOL-2): forms/fLotInfo.cpp WA-5 / WA-9 / WD-3 opened, 8 kept\n");
    static const char* const kReal[2]={ "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini" };
    std::string before[2];
    for(int i=0; i<2; i++) before[i]=Slurp(kReal[i]);
    CHECK(fLotInfo!=NULL, "fLotInfo exists (forms/fLotInfo.cpp eager new)");
    if(fLotInfo==NULL) { std::printf("test_pool2_lotinfo: %d passed, %d failed\n", g_pass, g_fail); return 1; }

    // ---- [1] WA-5 ----------------------------------------------------------------------------------
    std::printf("[1] WA-5 InitialRefrigerantSystem -> ATC_OFFLINE_FormComInit\n");
    {
        const int as0=AirStream_Select, tc0=Total_Compressor;
        AirStream_Select=1; Total_Compressor=0;
        TLabel* const lab[8]={ fLotInfo->LabRefrigerantValue1, fLotInfo->LabRefrigerantValue2, fLotInfo->LabRefrigerantValue3, fLotInfo->LabRefrigerantValue4,
                               fLotInfo->LabRefrigerantValue5, fLotInfo->LabRefrigerantValue6, fLotInfo->LabRefrigerantValue7, fLotInfo->LabRefrigerantValue8 };
        for(int i=0; i<8; i++) lab[i]->Caption="stale";
        fLotInfo->LabRefrigerantAdjustValue3->Visible=true;
        fLotInfo->labRefCopm2LpValue_8->Caption="stale";
        fLotInfo->labRefrigerantMachineHighLimit->Caption="stale";
        fLotInfo->labRefrigerantMachineLowLimit->Caption="stale";
        fLotInfo->bInitFormcomponent=false;
        fLotInfo->InitialRefrigerantSystem();
        int nOff=0;
        for(int i=0; i<8; i++) if(std::string(lab[i]->Caption.c_str())=="-999.0") nOff++;
        CHECK(nOff==8, "[1] all 8 refrigerant value labels back to the offline \"-999.0\" (golden :14977)");
        CHECK(std::string(fLotInfo->labRefCopm2LpValue_8->Caption.c_str())=="-999.0", "[1] compressor #2 Lp value label (index 7) -> \"-999.0\"");
        CHECK(fLotInfo->LabRefrigerantAdjustValue3->Visible==false, "[1] adjust-value label hidden (golden :14985)");
        CHECK(std::string(fLotInfo->labRefrigerantMachineHighLimit->Caption.c_str())=="Comp#2 Hp Over High Limit : -999.0", "[1] high-limit caption (golden :14988)");
        CHECK(std::string(fLotInfo->labRefrigerantMachineLowLimit->Caption.c_str())=="Comp#2 Hp Over Low Limit : -999.0", "[1] low-limit caption (golden :14989)");
        CHECK(fLotInfo->bInitFormcomponent==true, "[1] bInitFormcomponent true after InitialRefrigerantSystem (golden :14920 false, then :14974 true)");
        // AirStream_Select==0: golden returns before anything (both methods)
        AirStream_Select=0;
        lab[0]->Caption="kept";
        fLotInfo->bInitFormcomponent=false;
        fLotInfo->InitialRefrigerantSystem();
        CHECK(std::string(lab[0]->Caption.c_str())=="kept" && fLotInfo->bInitFormcomponent==false, "[1] AirStream_Select==0: nothing touched (golden :14879 early return)");
        AirStream_Select=as0; Total_Compressor=tc0;
    }

    // ---- [2] WA-9 ----------------------------------------------------------------------------------
    std::printf("[2] WA-9 btTesterTCPShowClick -> fTesterTCP->Show()\n");
    CHECK(fTesterTCP!=NULL, "[2] fTesterTCP exists (forms/fTesterTCP.cpp:49)");
    fLotInfo->btTesterTCPShowClick();
    CHECK(true, "[2] btTesterTCPShowClick() returns (Show() is the port-only no-op, forms/fTesterTCP.h:377)");

    // ---- [3] WD-3 ----------------------------------------------------------------------------------
    std::printf("[3] WD-3 LotKeyInTimeTimer, CC_KYEC_LEE operator ID\n");
    {
        const int cc0=CUSTOMER_CODE;
        const bool ok0=InitialOK, klt0=bEnable_KLT_Function, amr0=TrayForm.bEnableAMR, amrL0=TrayForm.bEnableAMRLoader;
        const std::string rsm0=std::string(fMain->cbRunStartMode->Text.c_str());
        CUSTOMER_CODE=CC_KYEC_LEE; InitialOK=true; bLotFirstKeyIn=false;
        TrayForm.bEnableAMR=false; TrayForm.bEnableAMRLoader=false; bEnable_KLT_Function=false;
        CHECK(OperatorAfterTick("12345")=="", "[3] 5 characters -> cleared (golden :11592-11597, length <6)");
        CHECK(OperatorAfterTick("12345678")=="", "[3] 8 characters -> cleared (length >7)");
        CHECK(OperatorAfterTick("0912345")=="0912345", "[3] 7 characters, head 091 in 85..120 -> kept (golden :11610-11614, :11624-11631)");
        CHECK(OperatorAfterTick("200123")=="", "[3] 6 characters, head 20 outside 85..120 -> cleared");
        bEnable_KLT_Function=true;
        CHECK(OperatorAfterTick("200123")=="200123", "[3] KLT: head 20 in 3..31 -> kept (golden :11616-11623)");
        CHECK(OperatorAfterTick("0912345")=="", "[3] KLT: head 091 outside 3..31 -> cleared");
        bEnable_KLT_Function=false;
        TrayForm.bEnableAMRLoader=true;
        fMain->cbRunStartMode->Text="Re-Test Continuous";
        CHECK(OperatorAfterTick("AGV")=="AGV", "[3] AMR Loader + cbRunStartMode \"Re-Test Continuous\": \"AGV\" kept (golden :11584-11591)");
        fMain->cbRunStartMode->Text="Re-Test Initial Start";
        CHECK(OperatorAfterTick("AGV")=="AGV", "[3] AMR Loader + \"Re-Test Initial Start\": \"AGV\" kept");
        fMain->cbRunStartMode->Text="Initial Start";
        CHECK(OperatorAfterTick("AGV")=="", "[3] AMR Loader + \"Initial Start\": \"AGV\" is 3 characters -> cleared (reads fMain->cbRunStartMode->Text)");
        TrayForm.bEnableAMRLoader=false; TrayForm.bEnableAMR=true;
        CHECK(OperatorAfterTick("AGV")=="AGV", "[3] bEnableAMR: first arm taken whatever the text (golden's `A || (B) && C` = A || (B && C))");
        CUSTOMER_CODE=CC_PTI;
        CHECK(OperatorAfterTick("12345")=="12345", "[3] CC_PTI (HT9050): the KYEC_LEE branch is not entered -> operator ID untouched");
        CUSTOMER_CODE=cc0; InitialOK=ok0; bEnable_KLT_Function=klt0; TrayForm.bEnableAMR=amr0; TrayForm.bEnableAMRLoader=amrL0;
        fMain->cbRunStartMode->Text=rsm0.c_str();
    }

    // ---- [4] source pins ---------------------------------------------------------------------------
    std::printf("[4] source\n");
    {
        const std::string s=Slurp(root+"/forms/fLotInfo.cpp");
        const int nIf0=Count(s, "\n#if 0");
        std::printf("    #if 0 left: %d\n", nIf0);
        CHECK(nIf0==81, "[4] fLotInfo.cpp: 81 `#if 0` left (84 before POOL-2: WA-5, WA-9, WD-3 opened)");
        CHECK(Count(s, "#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261006 (Ifor01): WA-5 ")==1, "[4] WA-5 opened in place");
        CHECK(Count(s, "#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261006 (Ifor01): WA-9 ")==1, "[4] WA-9 opened in place");
        CHECK(Count(s, "#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261006 (Ifor01): WD-3 ")==1, "[4] WD-3 opened in place");
        CHECK(Count(s, "#if 0\r\n    SetLotStart(\"fLotInfo::FormShow\", true);")+Count(s, "#if 0\n    SetLotStart(\"fLotInfo::FormShow\", true);")==1,
              "[4] WC-19 SetLotStart in FormShow still shut (SAFETY red line)");
        CHECK(Count(s, "#if 0 // GATE (W906-PROD-S117-ATKAMR)")==3, "[4] all three S117-ATKAMR still shut (S25; :6716 compiles but S25 holds; :6720 / :6726 still miss sTrackOutType_ATK / TfAGV::bATK_AMR_DoHostLotStart)");
        CHECK(Count(s, "#if 0 // GATE (W906-PROD-S117-S25-TSMC)")==1, "[4] S25-TSMC still shut (S25)");
        CHECK(Count(s, "#if 0 // GATE (W906-PROD-S117-S25-UTAC)")==1, "[4] S25-UTAC still shut (S25)");
        CHECK(Count(s, "#if 0 // GATE (W906-PROD-S117-RSM)")==2, "[4] both S117-RSM SetRunStartMode still shut (mode switch, Jimmy)");
        CHECK(Count(s, "#if 0 // GATE (W906-LOT-W1-MAINPANELS)")==1 && Count(s, "#if 0 // GATE (W906-PROD-S117-MAINPANELS)")==1,
              "[4] the MAINPANELS pair (lock at Lot Start / unlock at Lot End) still shut together (Jimmy)");
    }

    // ---- [5] real files ---------------------------------------------------------------------------
    for(int i=0; i<2; i++) CHECK(Slurp(kReal[i])==before[i], "[5] real machine file unchanged");

    std::printf("test_pool2_lotinfo: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
