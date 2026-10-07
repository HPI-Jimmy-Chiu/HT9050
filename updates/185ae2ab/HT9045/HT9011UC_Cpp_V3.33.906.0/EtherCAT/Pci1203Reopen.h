// =============================================================================
//  EtherCAT/Pci1203Reopen.h -- the 1203 "re-open card" DECISION (pure: no vendor header, no clock, no I/O).
//
//  AI(W906-1203REOPEN) 20261007: EastSun 1007「只要開卡異常或是 有沒有偵測到的模組 需要加個 重新開卡流程 可能需要用到reset ring」,
//  rulings (AskUserQuestion 1007): automatic while the machine is stopped AND an operator button ("兩種都要"); reset only the
//  ring that has the problem ("只重置出問題的那個環"); every module-check problem kind retries ("所有問題都重開"); 10 s between
//  tries, 3 tries per fault, the operator button gives 3 more; alarms: open fail WAR16150, re-open failed WAR16157; simulation
//  first. Plan D:\HT9045\_reopen_1007\PLAN_1203_REOPEN.md.
//
//  AI(W906-1203REOPEN-2) 20261007: review 20261007 -- the first version tried again every 10 s without waiting for the module
//  check's VERDICT (which itself needs up to 10 s after the link is back, longer while a reset ring comes back up), so every
//  try interrupted the previous one, and the 3rd try raised WAR16157 one tick after it was issued. Now every attempt waits for
//  a verdict: a fault reported again (= still failing) or Recovered(); no verdict within kVerdictMs counts as a failure. The
//  next try is kGapMs after that failure; the alarm comes only after the 3rd attempt's failure. Also: an operator re-open with
//  no fault keeps its 3-try budget when a fault shows up during it (it gave 1 + 3 before).
//
//  The host (tools/wb_serve.cpp EOF W906_Pci1203ReopenTick) reports faults and results and does what Step() asks:
//    Fault(kRfOpenFail)            the card did not open (boot, or a re-open that left it closed)
//    Fault(kRfModules, ringMask)   the module check (Pci1203ModuleCheck.h, WAR16154) found problems on these rings
//                                  (bit 0 = ring 0 motor drives, bit 1 = ring 1 IO modules)
//    Recovered()                   the card is open and the module check has no mismatch to report
//    AttemptDone(now)              the action Step() asked for has been carried out; its verdict follows
//    AttemptNotIssued(now)         the action could not be carried out at all (no control / refused): an immediate failure
//    OperatorRetry()               the 1203 page button: 3 more tries now (a manual re-open when there is no fault)
//  Step(stopped, nowMs, &rings) returns at most one action per call:
//    kRaResetRing (rings = the rings to reset), kRaReopenCard, kRaAlarm (once, after the 3rd failed try),
//    kRaRecovered (once, after Recovered()), or kRaNone.
//  Nothing is attempted while `stopped` is false (machine running / HOME / an axis moving -- the host decides).
//  Signed millisecond differences: tick wrap safe. A fault reported again MERGES its rings and keeps the try count.
// =============================================================================
#ifndef HT9045_ETHERCAT_PCI1203REOPEN_H
#define HT9045_ETHERCAT_PCI1203REOPEN_H

namespace ht9045 {

enum Pci1203ReopenFault { kRfNone = 0, kRfOpenFail = 1, kRfModules = 2, kRfManual = 3 };
enum Pci1203ReopenAct   { kRaNone = 0, kRaResetRing = 1, kRaReopenCard = 2, kRaAlarm = 3, kRaRecovered = 4 };

class Pci1203Reopen
{
public:
    static const int           kTries    = 3;
    static const unsigned long kGapMs    = 10000;   // after a failed try, before the next
    static const unsigned long kVerdictMs = 30000;  // how long an attempt waits for the module check's verdict

    Pci1203Reopen() : fault_(kRfNone), rings_(0), tries_(0), waiting_(false), awaiting_(false), failSeen_(false),
                      alarmed_(false), recovered_(false), haveNext_(false), nextAt_(0), deadline_(0) {}

    void Fault(int kind, unsigned ringMask = 0)
    {
        if (kind != kRfOpenFail && kind != kRfModules) return;
        if (fault_ == kRfNone) {                                        // a new fault: fresh budget, try at once
            fault_ = kind; rings_ = 0; tries_ = 0; alarmed_ = false; haveNext_ = false;
        } else if (fault_ == kRfManual || kind == kRfOpenFail) {
            fault_ = kind;                                              // keep the budget; a closed card outranks a ring problem
        }
        if (kind == kRfModules) rings_ |= (ringMask & 3u);
        if (awaiting_) failSeen_ = true;                                // the verdict of the pending attempt: still failing
        recovered_ = false;
    }
    void Recovered()
    {
        if (fault_ == kRfNone) return;
        fault_ = kRfNone; rings_ = 0; tries_ = 0; waiting_ = false; awaiting_ = false; failSeen_ = false;
        alarmed_ = false; haveNext_ = false;
        recovered_ = true;
    }
    void AttemptDone(unsigned long nowMs)
    {
        if (!waiting_) return;
        waiting_ = false; awaiting_ = true; failSeen_ = false; deadline_ = nowMs + kVerdictMs;
    }
    void AttemptNotIssued(unsigned long nowMs)
    {
        if (!waiting_) return;
        waiting_ = false; awaiting_ = false; failSeen_ = false; haveNext_ = true; nextAt_ = nowMs + kGapMs;
    }
    void OperatorRetry()
    {
        if (fault_ == kRfNone) { fault_ = kRfManual; rings_ = 0; }
        tries_ = 0; alarmed_ = false; haveNext_ = false; awaiting_ = false; failSeen_ = false;
        recovered_ = false;
    }

    int Step(bool stopped, unsigned long nowMs, unsigned* ringsOut)
    {
        if (ringsOut) *ringsOut = 0;
        if (recovered_) { recovered_ = false; return kRaRecovered; }
        if (fault_ == kRfNone || waiting_) return kRaNone;
        if (awaiting_) {
            if (!failSeen_ && (long)(nowMs - deadline_) < 0) return kRaNone;   // still waiting for the verdict
            awaiting_ = false; failSeen_ = false;                       // failed (reported again, or no verdict in time)
            haveNext_ = true; nextAt_ = nowMs + kGapMs;
        }
        if (tries_ >= kTries) {
            if (!alarmed_) { alarmed_ = true; return kRaAlarm; }
            return kRaNone;
        }
        if (!stopped) return kRaNone;
        if (haveNext_ && (long)(nowMs - nextAt_) < 0) return kRaNone;
        ++tries_; waiting_ = true;
        if (fault_ == kRfModules && rings_ != 0) {
            if (ringsOut) *ringsOut = rings_;
            return kRaResetRing;
        }
        return kRaReopenCard;                                           // open fail, manual, or a module fault with no ring
    }

    int      FaultKind() const { return fault_; }
    unsigned Rings()     const { return rings_; }
    int      Tries()     const { return tries_; }
    bool     Waiting()   const { return waiting_; }
    bool     Awaiting()  const { return awaiting_; }
    bool     Alarmed()   const { return alarmed_; }

private:
    int           fault_;
    unsigned      rings_;
    int           tries_;
    bool          waiting_, awaiting_, failSeen_, alarmed_, recovered_, haveNext_;
    unsigned long nextAt_, deadline_;
};

}  // namespace ht9045

#endif  // HT9045_ETHERCAT_PCI1203REOPEN_H
