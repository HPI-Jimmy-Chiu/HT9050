// ===========================================================================
//  TesterComm/Gpib/GpibDriver.cpp -- see GpibDriver.h.  AI(W906-GB-P1) 20260926.
// ===========================================================================
#include "TesterComm/Gpib/GpibDriver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstring>

namespace gpibbridge {

int  ibsta = 0;
int  iberr = 0;
long ibcnt = 0;

namespace {
IGpibDriver* g_driver = 0;

// no card: golden behaviour when ibfind fails is "ERR set, ud < 0"
int NoCard()
{
    ibsta = ERR;
    iberr = 0;   // EDVR
    ibcnt = 0;
    return ERR;
}

int Refresh(int ret)
{
    ibsta = g_driver->Status();
    iberr = g_driver->Error();
    ibcnt = g_driver->Count();
    return ret;
}
}  // namespace

void SetGpibDriver(IGpibDriver* d) { g_driver = d; }
IGpibDriver* GetGpibDriver() { return g_driver; }

int ibfind(const char* n)                   { if (!g_driver) { NoCard(); return -1; } return Refresh(g_driver->ibfind(n)); }
int ibrsc(int ud, int v)                    { if (!g_driver) return NoCard(); return Refresh(g_driver->ibrsc(ud, v)); }
int ibpad(int ud, int v)                    { if (!g_driver) return NoCard(); return Refresh(g_driver->ibpad(ud, v)); }
int ibtmo(int ud, int v)                    { if (!g_driver) return NoCard(); return Refresh(g_driver->ibtmo(ud, v)); }
int ibwait(int ud, int mask)                { if (!g_driver) return NoCard(); return Refresh(g_driver->ibwait(ud, mask)); }
int ibrd(int ud, void* buf, long cnt)       { if (!g_driver) return NoCard(); return Refresh(g_driver->ibrd(ud, buf, cnt)); }
int ibrsv(int ud, int v)                    { if (!g_driver) return NoCard(); return Refresh(g_driver->ibrsv(ud, v)); }
int ibwrt(int ud, const void* buf, long cnt){ if (!g_driver) return NoCard(); return Refresh(g_driver->ibwrt(ud, buf, cnt)); }
int ibstop(int ud)                          { if (!g_driver) return NoCard(); return Refresh(g_driver->ibstop(ud)); }

// ---------------------------------------------------------------------------
//  NiGpibDriver
// ---------------------------------------------------------------------------
namespace {
enum { F_find, F_rsc, F_pad, F_tmo, F_wait, F_rd, F_rsv, F_wrt, F_stop, F_sta, F_err, F_cnt };
typedef int (__stdcall *FnS)(const char*);
typedef int (__stdcall *FnII)(int, int);
typedef int (__stdcall *FnI)(int);
typedef int (__stdcall *FnRd)(int, void*, long);
typedef int (__stdcall *FnWr)(int, const void*, long);
typedef int (__stdcall *FnV)(void);
const char* const kNames[12] = { "ibfindA", "ibrsc", "ibpad", "ibtmo", "ibwait", "ibrd", "ibrsv", "ibwrt",
                                 "ibstop", "ThreadIbsta", "ThreadIberr", "ThreadIbcnt" };
}  // namespace

NiGpibDriver::NiGpibDriver() : module_(0), lastSta_(ERR)
{
    for (int i = 0; i < 12; ++i)
        fn_[i] = 0;
    HMODULE h = ::LoadLibraryA("gpib-32.dll");
    if (!h)
        return;
    bool ok = true;
    for (int i = 0; i < 12; ++i)
    {
        fn_[i] = reinterpret_cast<void*>(::GetProcAddress(h, kNames[i]));
        if (!fn_[i])
            ok = false;
    }
    if (!ok)
    {
        ::FreeLibrary(h);
        for (int i = 0; i < 12; ++i)
            fn_[i] = 0;
        return;
    }
    module_ = h;
}

NiGpibDriver::~NiGpibDriver()
{
    if (module_)
        ::FreeLibrary(static_cast<HMODULE>(module_));
}

#define NI_CALL(T, idx, args) (fn_[idx] ? reinterpret_cast<T>(fn_[idx]) args : (lastSta_ = ERR))
int NiGpibDriver::ibfind(const char* n)            { if (!fn_[F_find]) return -1; return reinterpret_cast<FnS>(fn_[F_find])(n); }
int NiGpibDriver::ibrsc(int ud, int v)             { return NI_CALL(FnII, F_rsc, (ud, v)); }
int NiGpibDriver::ibpad(int ud, int v)             { return NI_CALL(FnII, F_pad, (ud, v)); }
int NiGpibDriver::ibtmo(int ud, int v)             { return NI_CALL(FnII, F_tmo, (ud, v)); }
int NiGpibDriver::ibwait(int ud, int m)            { return NI_CALL(FnII, F_wait, (ud, m)); }
int NiGpibDriver::ibrd(int ud, void* b, long c)    { return NI_CALL(FnRd, F_rd, (ud, b, c)); }
int NiGpibDriver::ibrsv(int ud, int v)             { return NI_CALL(FnII, F_rsv, (ud, v)); }
int NiGpibDriver::ibwrt(int ud, const void* b, long c) { return NI_CALL(FnWr, F_wrt, (ud, b, c)); }
int NiGpibDriver::ibstop(int ud)                   { return NI_CALL(FnI, F_stop, (ud)); }
#undef NI_CALL
int  NiGpibDriver::Status() { return fn_[F_sta] ? reinterpret_cast<FnV>(fn_[F_sta])() : lastSta_; }
int  NiGpibDriver::Error()  { return fn_[F_err] ? reinterpret_cast<FnV>(fn_[F_err])() : 0; }
long NiGpibDriver::Count()  { return fn_[F_cnt] ? (long)reinterpret_cast<FnV>(fn_[F_cnt])() : 0; }   // Decl-32.h:366 int ThreadIbcnt(void)

// ---------------------------------------------------------------------------
//  SimGpibDriver
// ---------------------------------------------------------------------------
SimGpibDriver::SimGpibDriver() : FindFails(false), talk_(false), sta_(0), err_(0), cnt_(0), pad_(0), tmo_(0) {}

void SimGpibDriver::Recompute()
{
    sta_ = CMPL;
    if (!in_.empty())
        sta_ |= LACS;
    if (talk_)
        sta_ |= TACS;
}

void SimGpibDriver::TesterWrite(const std::string& cmd) { in_.push_back(cmd); Recompute(); }
void SimGpibDriver::SetTalkAddressed(bool on) { talk_ = on; Recompute(); }

std::vector<std::string> SimGpibDriver::TakeBridgeWrites()
{
    std::vector<std::string> v;
    v.swap(out_);
    return v;
}

std::vector<int> SimGpibDriver::TakeSrqBytes()
{
    std::vector<int> v;
    v.swap(srq_);
    return v;
}

int SimGpibDriver::ibfind(const char*)
{
    if (FindFails) { sta_ = ERR; err_ = 0; return -1; }
    Recompute();
    return 0;   // one board descriptor
}
int SimGpibDriver::ibrsc(int, int)      { Recompute(); return sta_; }
int SimGpibDriver::ibpad(int, int v)    { pad_ = v; Recompute(); return sta_; }
int SimGpibDriver::ibtmo(int, int v)    { tmo_ = v; Recompute(); return sta_; }
int SimGpibDriver::ibwait(int, int)     { Recompute(); return sta_; }
int SimGpibDriver::ibstop(int)          { Recompute(); return sta_; }

int SimGpibDriver::ibrd(int, void* buf, long cnt)
{
    cnt_ = 0;
    if (!in_.empty() && cnt > 0)
    {
        std::string s = in_.front();
        in_.pop_front();
        long n = (long)s.size() < cnt ? (long)s.size() : cnt;
        std::memcpy(buf, s.data(), (size_t)n);
        cnt_ = n;
    }
    Recompute();
    sta_ |= END;
    return sta_;
}

int SimGpibDriver::ibrsv(int, int v)
{
    srq_.push_back(v);
    Recompute();
    return sta_;
}

int SimGpibDriver::ibwrt(int, const void* buf, long cnt)
{
    if (!talk_)
    {
        err_ = 6;   // EABO-like: not addressed to talk
        sta_ = ERR | TIMO;
        cnt_ = 0;
        return sta_;
    }
    out_.push_back(std::string(static_cast<const char*>(buf), static_cast<size_t>(cnt)));
    cnt_ = cnt;
    err_ = 0;
    Recompute();
    return sta_;
}

}  // namespace gpibbridge
