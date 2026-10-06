// =============================================================================
//  test_pool2_heaterthread.cpp  --  AI(W906-POOL2) 20261006 (Ifor01)
//
//  W-111 第②項（TO_IFOR §3 15:3x）：uHeaterThread.cpp THeaterThread::HeaterThreadProcess 的 GATE 1／3／4
//  （CheckATC6System／HeaterDoorIsOpen／DoHeaterOn，golden uHeaterThread.cpp:60／:62／:64）照 golden 解開——
//  當初理由「csystem.cpp 沒有本體」已過期（csystem.cpp:19359／:14386／:19421）。
//  ⚠ 機台行為不變：這份 HeaterThreadProcess 只有 Execute() 會呼叫，而移植版沒有任何地方啟動那條執行緒
//  （Resume() 是 no-op，uHeaterThread.h）；機台上真正在跑的加熱節拍是 FastClockJobs.cpp W906_FastClockHeaterBeat
//  （出貨組態，同樣五個呼叫、golden 順序；模擬組態走 St02 的 HeaterSimTick）。
//    [1] 讀原始碼：uHeaterThread.cpp 的 HeaterThreadProcess 五個呼叫都開著、照 golden 順序，三處是原行改的
//        `#if 1 // was: #if 0 // TODO(W7-csystem) GATE n ... W-111`；檔裡不剩 TODO(W7-csystem) 的 `#if 0`
//    [2] 讀原始碼：FastClockJobs.cpp W906_FastClockHeaterBeat 是同樣五個呼叫、同樣順序（兩份一致）
//    [3] 執行：THeaterThread 建起來、Resume() 回來（不起執行緒），InitialOK==false 時 HeaterThreadProcess() 什麼都不做
//  反向驗證（第 15 條）見 MR 說明：任一處改回 `#if 0` ⇒ [1] 紅。
// =============================================================================
#include "uHeaterThread.h"
#include "cmydef.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <string>
#include <vector>

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_pool2_heaterthread.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string();
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
// the body of `head` up to the first "\n}" at column 0, with every line that is under `#if 0` dropped
static std::vector<std::string> LiveCalls(const std::string& s, const char* head)
{
    std::vector<std::string> out;
    size_t a=s.find(head);
    if(a==std::string::npos) return out;
    size_t b=s.find("\n}", a);
    std::string body=s.substr(a, b-a);
    static const char* const kCalls[5]={ "CheckATC6System();", "DoThermo();", "HeaterDoorIsOpen();", "CheckHeater();", "DoHeaterOn();" };
    bool dead=false;
    size_t p=0, q;
    while((q=body.find('\n', p))!=std::string::npos || p<body.size())
    {
        if(q==std::string::npos) q=body.size();
        std::string l=body.substr(p, q-p);
        p=q+1;
        size_t f=l.find_first_not_of(" \t\r");
        if(f==std::string::npos) continue;
        std::string t=l.substr(f);
        if(t.compare(0, 5, "#if 0")==0) { dead=true; continue; }
        if(t.compare(0, 6, "#endif")==0) { dead=false; continue; }
        if(dead || t.compare(0, 2, "//")==0) continue;
        static const std::string kMark="if (g_W906PhaseMark) g_W906PhaseMark(";   //AI(W906-POOL2) 20261006: laptop batch-80 integration -- skip the machine's PASSPROF phase marks (cpp 0235) in front of each call
        while(t.compare(0, kMark.size(), kMark)==0) { size_t e=t.find("\");"); if(e==std::string::npos) break; size_t g=t.find_first_not_of(" \t", e+3); t=(g==std::string::npos)?std::string():t.substr(g); }
        for(const char* c : kCalls) if(t.compare(0, std::string(c).size(), c)==0) out.push_back(c);
    }
    return out;
}

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("POOL2_HeaterThread"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::printf("test_pool2_heaterthread (W-111 #2): uHeaterThread.cpp GATE 1 / 3 / 4 opened as golden 0618\n");
    const std::vector<std::string> golden={ "CheckATC6System();", "DoThermo();", "HeaterDoorIsOpen();", "CheckHeater();", "DoHeaterOn();" };

    // ---- [1] uHeaterThread.cpp ---------------------------------------------------------------------
    const std::string u=Slurp(root+"/uHeaterThread.cpp");
    const std::vector<std::string> uc=LiveCalls(u, "void THeaterThread::HeaterThreadProcess(void)");
    std::printf("    HeaterThreadProcess live calls: %d\n", (int)uc.size());
    CHECK(uc==golden, "[1] HeaterThreadProcess: CheckATC6System, DoThermo, HeaterDoorIsOpen, CheckHeater, DoHeaterOn all live, golden :60-:64 order");
    CHECK(Count(u, "#if 1 // was: #if 0 // TODO(W7-csystem) GATE 1:")==1 && Count(u, "#if 1 // was: #if 0 // TODO(W7-csystem) GATE 3:")==1 &&
          Count(u, "#if 1 // was: #if 0 // TODO(W7-csystem) GATE 4:")==1, "[1] GATE 1 / 3 / 4 opened in place");
    CHECK(Count(u, "opened AI(W906-POOL2) 20261006 (Ifor01): W-111")==3, "[1] the three openings carry the W-111 note");
    CHECK(Count(u, "\n#if 0 // TODO(W7-csystem)")==0,"[1] no TODO(W7-csystem) `#if 0` left in uHeaterThread.cpp (line-start match: the opened lines quote it after `was:`)");

    // ---- [2] FastClockJobs.cpp (the live heater beat) ----------------------------------------------
    const std::string fc=Slurp(root+"/FastClockJobs.cpp");
    const std::vector<std::string> fcc=LiveCalls(fc, "void W906_FastClockHeaterBeat()");
    CHECK(fcc==golden, "[2] FastClockJobs.cpp W906_FastClockHeaterBeat: the same five calls in the same order (the two copies agree)");

    // ---- [3] runtime: no thread, nothing done before InitialOK -------------------------------------
    {
        const bool ok0=InitialOK;
        InitialOK=false;
        THeaterThread* t=new THeaterThread(true);
        t->Resume();
        t->HeaterThreadProcess();
        CHECK(true, "[3] THeaterThread constructed, Resume() returns (no OS thread), HeaterThreadProcess() with InitialOK==false returns");
        InitialOK=ok0;
        delete t;
    }

    std::printf("test_pool2_heaterthread: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
