// =============================================================================
//  test_pool2_csystem.cpp  --  AI(W906-POOL2) 20261007 (Ifor01)
//
//  POOL-2 `csystem.cpp`（FROM_IFOR §1 1007 14:4x）：普查第二節 17 個候選，在目前的 main 用閘名重新定位、逐個對 golden 0618 看過——
//  12 個理由已過期、照 golden 原行改開（讀 fShow 的三處照 FShow_Audit 規則經 W906_FormShowing）（`#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261007 (Ifor01): <閘> reason expired`）；
//  5 個保持關著（G01a／G01b 缺 TfCleaning::btnResetCleanCountClick；G21 兩份 TMySucker 版面不同；G22 安全門 0.5 秒寬限：
//  可以用 W906_FormShowing("fTeach", …) 照 golden 開，但它放寬停機時的門檢查，先問 Jimmy（FROM_IFOR §3）；W906-FLOW-1（荷重元）：
//  DoTestHeadMotorLoadCell 還是空殼，開了 bLoadCellTest 會讓流程每輪空轉）。
//    [1] 讀原始碼（argv[1] = 移植樹根目錄）：12 個開的閘各剛好一個開閘註記、閘內是 golden 原文那一行；5 個關的仍是 `#if 0`
//    [2] 執行 G35：InitDoArmZHome() 後跑一次 DoArmZHome()（第 1 步），bBackArmZHomeFlag 變 true（golden csystem.cpp:4931）
//    [3] 執行 G8：fiosetview->fShow==true 時 ProcessCCDLight() 照 golden :19130-19133 直接 return，不動 SwCCDLight；
//        fShow==false 時照舊走到最後的 else 關燈
//  反向驗證見 MR 說明：任一個改回 `#if 0` ⇒ [1] 紅；G35 改回 ⇒ [2] 紅；G8 改回 ⇒ [3] 紅。
// =============================================================================
#include "csystem.h"
#include "cmydef.h"
#include "atester_shims.h"                // fiosetview (TfiosetviewShim::fShow)
#include "myswitch.h"                     // SW[]
#include "w906_ctest_guard.h"
#include <cstdio>
#include <string>
#include <vector>

extern bool bBackArmZHomeFlag;                  // csystem.cpp file scope (golden csystem.cpp:145; no header declares it)

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_pool2_csystem.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string();
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static std::vector<std::string> Lines(const std::string& s)
{
    std::vector<std::string> out;
    size_t p=0, q;
    while(p<s.size())
    {
        q=s.find('\n', p);
        if(q==std::string::npos) q=s.size();
        std::string l=s.substr(p, q-p);
        if(!l.empty() && l[l.size()-1]=='\r') l.erase(l.size()-1);
        out.push_back(l);
        p=q+1;
    }
    return out;
}
static std::string Trim(const std::string& l)
{
    size_t a=l.find_first_not_of(" \t");
    return a==std::string::npos ? std::string() : l.substr(a);
}

struct Opened { const char* gate; const char* golden; };   // golden 0618 statement right under the gate (whitespace-trimmed prefix)
static const Opened kOpened[12]={
    { "GATE(g3-G07) reason expired", "LotSummary.ClearAllData();" },                                    // golden :6282
    { "GATE(g3-G08) reason expired", "LotSummary.ClearAllData();" },                                    // golden :6301
    { "GATE G11 reason expired",     "if(fAGV->Use_AMR() &&" },                                         // golden :16087
    { "GATE G8 reason expired",      "if(W906_FormShowing(\"fiosetview\", fiosetview->fShow))" },                                         // golden :19130
    { "GATE G02 reason expired -- fTemperFrom", "if(fTemperFrom->TempRunShowAlarmHigh())" },             // golden :3969
    { "GATE G17 reason expired",     "if(bIndexCheck1 && fiosetview->ProcessIndexSuckDestroy1())" },    // golden :4536
    { "GATE G35 reason expired",     "bBackArmZHomeFlag       =true;" },                                // golden :4931
    { "GATE G02 reason expired -- HasAreaOverAmbientTemp_DUT", "if(HasAreaOverAmbientTemp_DUT(dTemp))" }, // golden :808
    { "GATE G09 reason expired",     "if(IniConfig.bIOFormCanControlHeaterFan && W906_FormShowing(\"fiosetview\", fiosetview->fShow)==true)" }, // golden :1246
    { "GATE G10 reason expired",     "if(W906_FormShowing(\"fiosetview\", fiosetview->fShow)==false)" },                                  // golden :1251
    { "GATE H3-2 reason expired",    "fTemp_Set->edtCurrTemp->Text=FormatFloat(\"0.00\", dBoostOffset);" }, // golden :22280
    { "GATE H3-4 reason expired",    "fTemp_Set->edtCurrTemp->Text=FormatFloat(\"0.00\", dBoostOffset);" }, // golden :22310
};
static const char* const kKept[5]={       // still `#if 0`: the reason holds (see the header of this file)
    "#if 0 // GATE G01a -- golden csystem.cpp:3577",
    "#if 0 // GATE G01b -- golden csystem.cpp:3586",
    "#if 0 // GATE G21 -- golden csystem.cpp:2481-2482",
    "#if 0 // GATE G22 -- golden csystem.cpp:2608-2624",
    "#if 0 // GATE(W906-FLOW-1): golden :9530-9534",
};

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("POOL2_CSystem"))
        return 1;
    const std::string root=argc>1 ? argv[1] : "..";
    const std::vector<std::string> L=Lines(Slurp(root+"/csystem.cpp"));
    CHECK(L.size()>30000, "[1] csystem.cpp read (argv[1] = the port root)");

    std::printf("-- [1] source: 12 gates open as golden, 5 kept --\n");
    for(int g=0; g<12; g++)
    {
        int hits=0, at=-1;
        for(size_t i=0; i<L.size(); i++)
            if(L[i].find("opened AI(W906-POOL2) 20261007 (Ifor01): ")!=std::string::npos && L[i].find(kOpened[g].gate)!=std::string::npos)
                { hits++; at=(int)i; }
        char m[200];
        std::snprintf(m, sizeof(m), "[1] %s: exactly one opening note (found %d)", kOpened[g].gate, hits);
        CHECK(hits==1, m);
        if(hits!=1) continue;
        std::snprintf(m, sizeof(m), "[1] %s: the line is `#if 1 // was: #if 0 -- ...`", kOpened[g].gate);
        CHECK(Trim(L[at]).compare(0, 21, "#if 1 // was: #if 0 -")==0, m);
        std::snprintf(m, sizeof(m), "[1] %s: the golden statement follows the gate", kOpened[g].gate);
        CHECK(at+1<(int)L.size() && Trim(L[at+1]).compare(0, std::string(kOpened[g].golden).size(), kOpened[g].golden)==0, m);
    }
    for(int k=0; k<5; k++)
    {
        int hits=0;
        for(size_t i=0; i<L.size(); i++)
            if(Trim(L[i]).compare(0, std::string(kKept[k]).size(), kKept[k])==0) hits++;
        char m[200];
        std::snprintf(m, sizeof(m), "[1] kept closed: %s (found %d)", kKept[k], hits);
        CHECK(hits==1, m);
    }

    std::printf("-- [2] G35: DoArmZHome step 1 raises bBackArmZHomeFlag (golden :4931) --\n");
    bBackArmZHomeFlag=false;
    InitDoArmZHome();
    DoArmZHome();
    CHECK(bBackArmZHomeFlag==true, "[2] after DoArmZHome() step 1, bBackArmZHomeFlag==true");
    bBackArmZHomeFlag=false;
    InitDoArmZHome();

    std::printf("-- [3] G8: ProcessCCDLight returns at once while the IO-set view is shown (golden :19130-19133) --\n");
    const bool savedCCD=REAL_TIME_CCD;
    REAL_TIME_CCD=false;
    fiosetview->fShow=false;
    SW[SwCCDLight].OutValue=true;
    ProcessCCDLight();
    CHECK(SW[SwCCDLight].OutValue==false, "[3] fShow==false: the final else switches SwCCDLight off (unchanged path)");
    fiosetview->fShow=true;
    SW[SwCCDLight].OutValue=true;
    ProcessCCDLight();
    CHECK(SW[SwCCDLight].OutValue==true, "[3] fShow==true: golden returns before touching SwCCDLight");
    fiosetview->fShow=false;
    REAL_TIME_CCD=savedCCD;

    std::printf("test_pool2_csystem: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
