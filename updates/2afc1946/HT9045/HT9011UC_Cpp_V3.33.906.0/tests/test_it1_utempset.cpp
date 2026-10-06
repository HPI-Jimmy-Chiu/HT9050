// =============================================================================
//  test_it1_utempset.cpp  --  AI(W906-IT1) 20261006 (Ifor01)
//
//  IT-1 第三批（FROM_IFOR §1 1006 11:3x）：uTemp_Set.cpp 42 個 `#if 0` 只有 1 個理由不成立——`:846` GATE(dep-SetFocus)
//  `rbTemp->SetFocus();`（vclcompat TRadioButton 繼承 TControl，`SetFocus()` 現在有，離線是 no-op）。no-op 沒有可觀察的效果，
//  而所在的 `TfTemp_Set::FormShow` 全樹沒有生產呼叫點（WebStart.cpp:173），所以本測試做兩件事：
//    [1] 解開那一行用到的東西真的存在、可以呼叫：`fTemp_Set->rbTemp` 是 TRadioButton，`SetFocus()` 呼叫得到、不改狀態
//    [2] 讀原始碼：uTemp_Set.cpp 剩 41 個 `#if 0`；`:846` 那一處是 `#if 1 // was: #if 0 // GATE(dep-SetFocus)`；
//        保留的 41 個裡，SAFETY 閘 11 個（S5／S7／S9／S11～S16）一個都沒動
//    [3] 只動記憶體：D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 前後逐位元組相同
//  `:6229` ShowLineOnTop（GATE(G-Align)，整段只有 ->BringToFront()，TControl 現在有、離線 no-op）也解開——POOL-1 編譯器探針找到的。
//  反向驗證（第 15 條）見 MR 說明：`:846` 或 `:6229` 改回 `#if 0` ⇒ [2] 紅。
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "forms/fTemp_Set.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <string>

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_it1_utempset.cpp:%d] %s\n", __LINE__, msg); } } while(0)

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

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("IT1_UTempSet"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::printf("test_it1_utempset (IT-1 batch 3): uTemp_Set.cpp :846 opened, 41 kept\n");
    static const char* const kReal[2]={ "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini" };
    std::string before[2];
    for(int i=0; i<2; i++) before[i]=Slurp(kReal[i]);

    // ---- [1] the opened line's dependency ----------------------------------------------------------
    std::printf("[1] rbTemp->SetFocus()\n");
    if(fTemp_Set==NULL) fTemp_Set=new TfTemp_Set();            // as wb_serve.cpp:3124
    TRadioButton* rb=fTemp_Set->rbTemp;
    const bool checked0=rb->Checked, visible0=rb->Visible, enabled0=rb->Enabled;
    rb->SetFocus();
    fTemp_Set->ShowLineOnTop();                               // :6229 opened: every line is ->BringToFront() (offline no-op)
    CHECK(rb!=NULL, "[1] fTemp_Set->rbTemp exists (forms/fTemp_Set.h)");
    CHECK(rb->Checked==checked0 && rb->Visible==visible0 && rb->Enabled==enabled0, "[1] SetFocus() is callable and changes no state (offline no-op, as the gate note said it would be)");

    // ---- [2] source pins --------------------------------------------------------------------------
    std::printf("[2] source\n");
    {
        const std::string s=Slurp(root+"/uTemp_Set.cpp");
        const int nIf0=Count(s, "\n#if 0");
        std::printf("    #if 0 left: %d\n", nIf0);
        CHECK(nIf0==40, "[2] uTemp_Set.cpp: 40 `#if 0` left (42 before IT-1 batch 3: :846 and :6229 opened)");
        CHECK(Count(s, "#if 1 // was: #if 0 // GATE(G-Align) -- opened AI(W906-IT1)")==1, "[2] :6229 ShowLineOnTop (G-Align, all ->BringToFront()) opened in place");
        CHECK(Count(s, "#if 1 // was: #if 0 // GATE(dep-SetFocus)")==1, "[2] :846 GATE(dep-SetFocus) opened in place");
        int nSafety=0;
        const char* const kS[]={ "#if 0 // SAFETY GATE (S5)", "#if 0 // SAFETY GATE (S7)", "#if 0 // SAFETY GATE (S9)", "#if 0 // SAFETY GATE (S11)",
                                 "#if 0 // SAFETY GATE (S12)", "#if 0 // SAFETY GATE (S13)", "#if 0 // SAFETY GATE (S14)", "#if 0 // SAFETY GATE (S15)",
                                 "#if 0 // SAFETY GATE (S16)" };
        for(const char* k : kS) nSafety+=Count(s, k);
        std::printf("    SAFETY gates still shut: %d\n", nSafety);
        CHECK(nSafety==13, "[2] all 13 SAFETY `#if 0` lines (S5, S7, S9, S11, S12, S13 x5, S14, S15, S16) still shut -- IT-1 batch 3 touched none");
    }

    // ---- [3] real files ---------------------------------------------------------------------------
    for(int i=0; i<2; i++) CHECK(Slurp(kReal[i])==before[i], "[3] real machine file unchanged");

    std::printf("test_it1_utempset: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
