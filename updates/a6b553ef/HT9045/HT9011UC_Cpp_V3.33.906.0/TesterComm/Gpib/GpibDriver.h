// ===========================================================================
//  TesterComm/Gpib/GpibDriver.h -- NI-488.2 surface used by the GPIB bridge, behind a swappable driver.
//
//  AI(W906-GB-P1) 20260926.  Golden links BorlandC_gpib-32.obj (Borland OMF; MinGW cannot link it) and reads the
//  NI globals ibsta / iberr / ibcnt after every call.  Here:
//    * gpibbridge::ibfind / ibrsc / ibpad / ibtmo / ibwait / ibrd / ibrsv / ibwrt / ibstop are thin wrappers that
//      call the active IGpibDriver and then copy its status into gpibbridge::ibsta / iberr / ibcnt, so golden
//      bodies that test `ibsta&LACS` right after a call compile and behave unchanged.
//    * NiGpibDriver: LoadLibrary("gpib-32.dll") + GetProcAddress of exactly these nine calls, and ThreadIbsta /
//      ThreadIberr / ThreadIbcnt for the status (per-thread in NI's DLL -- correct because every call happens on
//      the one TesterComm thread).  If the DLL is missing, every call fails with ERR set (the engine then logs
//      "Error Open GPIB0" exactly like golden when ibfind returns < 0).
//    * SimGpibDriver: scripted Tester for ctest and SOFT_SIMULTE builds (queue text the "Tester" writes, capture
//      what the bridge writes back, observe ibrsv SRQ bytes).
//  Only the constants golden actually uses are defined (values from NI Decl-32.h).
// ===========================================================================
#ifndef TESTERCOMM_GPIB_GPIBDRIVER_H
#define TESTERCOMM_GPIB_GPIBDRIVER_H

#include <deque>
#include <string>
#include <vector>

namespace gpibbridge {

// NI Decl-32.h status bits (ibsta)
const int ERR  = (1 << 15);
const int TIMO = (1 << 14);
const int END  = (1 << 13);
const int SRQI = (1 << 12);
const int RQS  = (1 << 11);
const int CMPL = (1 << 8);
const int LOK  = (1 << 7);
const int REM  = (1 << 6);
const int CIC  = (1 << 5);
const int ATN  = (1 << 4);
const int TACS = (1 << 3);
const int LACS = (1 << 2);
const int DTAS = (1 << 1);
const int DCAS = (1 << 0);
// Decl-32.h also defines BIN (1<<12) (an ibeos flag); golden Main.cpp never uses it as a status bit, so it is
// deliberately NOT defined here -- the name collides with ordinary identifiers.

// NI user variables, refreshed after every wrapper call.
extern int  ibsta;
extern int  iberr;
extern long ibcnt;

class IGpibDriver
{
public:
    virtual ~IGpibDriver() {}
    virtual const char* Name() const = 0;
    virtual int ibfind(const char* udname) = 0;
    virtual int ibrsc(int ud, int v) = 0;
    virtual int ibpad(int ud, int v) = 0;
    virtual int ibtmo(int ud, int v) = 0;
    virtual int ibwait(int ud, int mask) = 0;
    virtual int ibrd(int ud, void* buf, long cnt) = 0;
    virtual int ibrsv(int ud, int v) = 0;
    virtual int ibwrt(int ud, const void* buf, long cnt) = 0;
    virtual int ibstop(int ud) = 0;
    virtual int  Status() = 0;   // ibsta after the last call
    virtual int  Error() = 0;    // iberr
    virtual long Count() = 0;    // ibcnt
};

// Active driver (not owned).  NULL means "no GPIB card": every call returns with ERR.
void SetGpibDriver(IGpibDriver* d);
IGpibDriver* GetGpibDriver();

// golden-named wrappers
int ibfind(const char* udname);
int ibrsc(int ud, int v);
int ibpad(int ud, int v);
int ibtmo(int ud, int v);
int ibwait(int ud, int mask);
int ibrd(int ud, void* buf, long cnt);
int ibrsv(int ud, int v);
int ibwrt(int ud, const void* buf, long cnt);
int ibstop(int ud);

// Real NI driver via gpib-32.dll.
class NiGpibDriver : public IGpibDriver
{
public:
    NiGpibDriver();
    ~NiGpibDriver();
    bool Loaded() const { return module_ != 0; }
    const char* Name() const { return "ni-gpib-32.dll"; }
    int ibfind(const char* udname);
    int ibrsc(int ud, int v);
    int ibpad(int ud, int v);
    int ibtmo(int ud, int v);
    int ibwait(int ud, int mask);
    int ibrd(int ud, void* buf, long cnt);
    int ibrsv(int ud, int v);
    int ibwrt(int ud, const void* buf, long cnt);
    int ibstop(int ud);
    int  Status();
    int  Error();
    long Count();

private:
    NiGpibDriver(const NiGpibDriver&);
    NiGpibDriver& operator=(const NiGpibDriver&);
    void* module_;
    void* fn_[12];
    int lastSta_;
};

// Scripted Tester.  The bridge is a GPIB *device* (non-controller): the Tester addresses it to listen and writes a
// command (ibwait shows LACS, ibrd returns the text), or addresses it to talk (ibwait shows TACS, ibwrt succeeds).
class SimGpibDriver : public IGpibDriver
{
public:
    SimGpibDriver();
    const char* Name() const { return "sim"; }
    // Tester side
    void TesterWrite(const std::string& cmd);      // queued; bridge sees LACS until it reads it
    void SetTalkAddressed(bool on);                // bridge sees TACS (ibwrt succeeds only while on)
    std::vector<std::string> TakeBridgeWrites();   // everything the bridge ibwrt'ed since the last call
    std::vector<int> TakeSrqBytes();               // every ibrsv value since the last call
    bool FindFails;                                // make ibfind return -1 (no card)

    int ibfind(const char* udname);
    int ibrsc(int ud, int v);
    int ibpad(int ud, int v);
    int ibtmo(int ud, int v);
    int ibwait(int ud, int mask);
    int ibrd(int ud, void* buf, long cnt);
    int ibrsv(int ud, int v);
    int ibwrt(int ud, const void* buf, long cnt);
    int ibstop(int ud);
    int  Status() { return sta_; }
    int  Error() { return err_; }
    long Count() { return cnt_; }

    int PrimaryAddress() const { return pad_; }
    int TimeoutCode() const { return tmo_; }

private:
    void Recompute();
    std::deque<std::string> in_;
    std::vector<std::string> out_;
    std::vector<int> srq_;
    bool talk_;
    int sta_, err_;
    long cnt_;
    int pad_, tmo_;
};

}  // namespace gpibbridge

#endif
