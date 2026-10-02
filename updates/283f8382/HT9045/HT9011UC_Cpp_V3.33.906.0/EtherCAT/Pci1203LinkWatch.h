// =============================================================================
//  EtherCAT/Pci1203LinkWatch.h -- "1203 斷線 10 秒 -> 跳異常": the link-lost watch.
//
//  //AI(W906-1203-LINKLOST) 20261001: EastSun 1001「1203 如果斷線10秒 要跳出異常」
//    -- if the PCIE-1203 EtherCAT link stays lost for 10 s, the HMI must show an
//    alarm. This header is the DECISION (pure, header-only, no vendor header, no
//    <windows.h>, no clock of its own); the host (tools/wb_serve.cpp EOF,
//    W906_Pci1203LinkWatchTick) feeds it one sample after every monitor Poll()
//    and raises the alarm it asks for.
//
//  ## WHICH SIGNAL (only what TPci1203Monitor already samples; no new vendor call)
//    1. Per-station EtherCAT AL state. Poll() re-reads EVERY discovered station
//       with Acm_DevGetSlaveStates on every IO tick (Pci1203Monitor.cpp Poll(),
//       the "EtherCAT ring" loop) and keeps stateValid / state / lastError in
//       Pci1203SlaveSample. A station counts as UP only when that read succeeded
//       AND its state is EC_SLAVE_STATE_OP (0x08, vendor AdvMotDrv.h). Read failed,
//       INIT, PREOP, BOOT or SAFEOP = down. Strict OP is golden's own per-slave test
//       (golden EtherCAT/MyEtherCAT.cpp `if(SlaveState!=EC_SLAVE_STATE_OP)` ->
//       WAR16151, port EtherCAT/MyEtherCAT.cpp:531).
//    2. The card stopped answering: the monitor auto-disables itself after
//       kMaxConsecutiveFailures (10) polls in which EVERY read failed
//       (Pci1203Monitor.cpp Poll() "self-disable"), and then Poll() returns at
//       once and the station samples freeze -- so Disabled() is a loss by itself.
//    3. The card is not open (card().open == false): production closed it
//       (attached mode "detached"), or an operator Refresh (Rescan) failed and left
//       the monitor closed (Pci1203Monitor.cpp Rescan()).
//    4. The scan finds no station at all while the card is open (a Refresh / the
//       zero-scan retry swept an empty ring), or it finds stations but none of
//       them is in OP (a Refresh re-learnt a ring that is not up -- see below).
//    NOT used: Acm_DevCheckEvent (golden's EVT_DEV_DISCONNET, needs
//    Acm_DevEnableEvent -- a write) and FT_MasCyclicCnt_R0/_R1 (read once in
//    Open(), not per Poll). Both would be new vendor calls in the read-only TU.
//
//  ## WHICH STATIONS ARE WATCHED
//    A station is watched once it has been seen UP (OP) -- so a module that never
//    reached OP since boot (wrong ID, no power, a drive parked in SAFEOP+ERR) is
//    shown by the 1203 page as it is today but never alarms here. "Unaddressable"
//    stations are skipped: their state read lands on their address twin
//    (Pci1203Monitor.h Pci1203SlaveSample::unaddressable).
//    ⚠ A SCAN IS THE TOPOLOGY OF RECORD. When the set of scanned stations changes
//    (an operator Refresh, the zero-scan retry, an attached-mode re-attach -- the
//    only places ScanSlaves_() runs), watched stations that the new scan no
//    longer finds stop being watched. So "remove a module, press Refresh" re-learns
//    the ring instead of alarming forever; the price is that a Refresh pressed
//    while a mid-ring cable is out also re-learns the shorter ring. A Refresh that
//    finds NO station, or fails, is still a loss (signals 3 / 4).
//
//  ## WHEN IT STARTS WATCHING
//    Not before the link has been UP once since boot: the card open, not disabled,
//    and at least one station in OP. The monitor's initial Open() / scan runs
//    synchronously before the tick loop (tools/wb_serve.cpp Pci1203MonitorEnable)
//    and a cold-boot ring that comes up late is only swept again by the zero-scan
//    retry -- none of that alarms. A build without the vendor SDK (Open() says "not
//    linked", disabled from the start) and a machine whose ring never comes up
//    never arm, so they never alarm. Once armed it stays armed for the process.
//
//  ## THE TIMER AND "ONCE"
//    The first lost sample starts the timer (the caller's tick, wrap-safe 32-bit
//    subtraction like the monitor's own GetTickCount arithmetic). If every sample
//    after it is still lost and kPci1203LinkLostAlarmMs has passed, Step() returns
//    kAlarm ONCE. Still lost -> nothing more. One UP sample (every watched station
//    in OP and the card answering) ends the episode and clears the timer and the
//    "raised" mark; a new loss needs another full kPci1203LinkLostAlarmMs.
//    ⓘ "Continuous" means: every sample we took said lost. While the tick thread
//    is held elsewhere (a blocking alarm dialog does not Poll) no sample is taken;
//    a gap is not evidence that the link came back.
//    The alarm itself is the golden one and is cleared the golden way (the
//    operator acknowledges the note); this class never clears or re-raises it.
// =============================================================================
#ifndef HT9045_ETHERCAT_PCI1203LINKWATCH_H
#define HT9045_ETHERCAT_PCI1203LINKWATCH_H

#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ht9045 {

//  ★ THE ONE PLACE the 10 s lives (EastSun 1001「斷線10秒」).
enum { kPci1203LinkLostAlarmMs = 10000 };
//  How many down stations the reason names one by one; the rest are counted.
enum { kPci1203LinkLostListMax = 4 };
//  The reason goes into golden ShowErrorMessage's errPart (event log / HANDLER
//  LOG csv / the note's message), so it is kept short and CSV / SQL-quote safe.
enum { kPci1203LinkLostTextMax = 400 };

// One scanned station as this watch sees it.
struct Pci1203LinkStation {
    int            ring;
    int            addr;         // the SlaveIP the monitor reads it with
    bool           aliasValid;
    unsigned short alias;        // the module's dial setting (ESC 0x0012)
    bool           readOk;       // this Poll's Acm_DevGetSlaveStates succeeded
    unsigned short state;        // raw EC_SLAVE_STATE_* (meaningful when readOk)
    unsigned long  err;          // the failed read's return code (when !readOk)
    Pci1203LinkStation()
        : ring(-1), addr(-1), aliasValid(false), alias(0)
        , readOk(false), state(0), err(0) {}
};

// One sample: what the monitor says right after a Poll().
struct Pci1203LinkInput {
    bool          open;          // card().open -- we hold a device handle
    bool          disabled;      // Disabled() -- the monitor stopped polling
    std::string   disabledWhy;   // disabledReason()
    std::string   lastErrorText; // card().lastErrorText
    std::vector<Pci1203LinkStation> stations;   // present + addressable, scan order
    Pci1203LinkInput() : open(false), disabled(false) {}
};

inline bool Pci1203LinkStationUp(const Pci1203LinkStation& s)
{
    return s.readOk && (s.state & 0x0Fu) == 0x08u;           // EC_SLAVE_STATE_OP
}

inline const char* Pci1203LinkStateName(unsigned short state)
{
    switch (state & 0x0Fu) {                                 // EC_SLAVE_STATE_* low nibble (AdvMotDrv.h)
        case 0x01u: return "INIT";
        case 0x02u: return "PREOP";
        case 0x03u: return "BOOT";
        case 0x04u: return "SAFEOP";
        case 0x08u: return "OP";
        default:    return "NONE";
    }
}

//  Wrap-safe elapsed milliseconds on a 32-bit tick (GetTickCount), whatever the
//  width of unsigned long on the build.
inline unsigned long Pci1203LinkElapsedMs(unsigned long nowMs, unsigned long sinceMs)
{
    return static_cast<unsigned long>(static_cast<unsigned int>(
        static_cast<unsigned int>(nowMs) - static_cast<unsigned int>(sinceMs)));
}

//  ',' -> ';', quotes -> '`', control characters -> ' ', clipped to `max` bytes
//  without splitting a UTF-8 sequence. Applied to every text this watch hands out.
inline std::string Pci1203LinkSafeText(const std::string& s, std::size_t max)
{
    std::string t;
    t.reserve(s.size() < max ? s.size() : max);
    for (std::size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == ',') c = ';';
        else if (c == '\'' || c == '"') c = '`';
        else if ((unsigned char)c < 0x20u) c = ' ';
        t += c;
    }
    if (t.size() > max) {
        std::size_t k = max;
        while (k > 0 && ((unsigned char)t[k] & 0xC0u) == 0x80u) --k;   // never cut inside a character
        t.resize(k);
        if (t.size() >= 3) t.replace(t.size() - 3, 3, "...");
    }
    return t;
}

//  The monitor -> sample adapter. A template so the test can hand it a fake with
//  the same five members TPci1203Monitor has (card() / Disabled() /
//  disabledReason() / slaveCount() / slave(i)). Reads only what Poll() already
//  stored; calls nothing on the card.
template <class Mon>
inline Pci1203LinkInput Pci1203LinkInputFromMonitor(const Mon& mon)
{
    Pci1203LinkInput in;
    in.open          = mon.card().open;
    in.disabled      = mon.Disabled();
    if (in.disabled) in.disabledWhy = mon.disabledReason();
    in.lastErrorText = mon.card().lastErrorText;
    const int n = mon.slaveCount();
    for (int i = 0; i < n; ++i) {
        const auto& s = mon.slave(i);
        if (!s.present || s.unaddressable) continue;
        Pci1203LinkStation st;
        st.ring       = s.ring;
        st.addr       = s.addr;
        st.aliasValid = s.aliasValid;
        st.alias      = s.alias;
        st.readOk     = s.stateValid;
        st.state      = s.state;
        st.err        = s.lastError;
        in.stations.push_back(st);
    }
    return in;
}

class Pci1203LinkWatch {
public:
    enum Event {
        kNone = 0,
        kArmed,       // the link is up for the first time since boot: watching starts
        kLost,        // first lost sample of an episode: the timer starts
        kAlarm,       // lost for kPci1203LinkLostAlarmMs: raise the alarm NOW (once per episode)
        kRestored     // the episode ended: every watched station is in OP again
    };
    struct Result {
        Event         event;
        bool          lost;        // this sample says the link is lost (armed only)
        unsigned long lostForMs;   // since the first lost sample (0 when not lost)
        std::string   text;        // kAlarm: the alarm's errPart; others: a log line
        Result() : event(kNone), lost(false), lostForMs(0) {}
    };

    Pci1203LinkWatch() : armed_(false), lost_(false), raised_(false), lostSince_(0), alarms_(0) {}

    // One sample. `nowMs` = the caller's millisecond tick; `clock` = the same
    // instant as wall-clock text (only stored, for "since when").
    Result Step(const Pci1203LinkInput& in, unsigned long nowMs, const std::string& clock)
    {
        Result r;

        // 1. The scanned set; a change means a scan ran: keep only watched stations it still found.
        std::vector<Key> keys;
        for (std::size_t i = 0; i < in.stations.size(); ++i)
            keys.push_back(Key(in.stations[i].ring, in.stations[i].addr));
        std::sort(keys.begin(), keys.end());
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
        if (keys != scanKeys_) {
            std::vector<Key> keep;
            for (std::size_t i = 0; i < watched_.size(); ++i)
                if (std::binary_search(keys.begin(), keys.end(), watched_[i])) keep.push_back(watched_[i]);
            watched_.swap(keep);
            scanKeys_ = keys;
        }

        // 2. Learn every station in OP while the card answers.
        const bool cardOk = in.open && !in.disabled;
        int upNow = 0;
        if (cardOk) {
            for (std::size_t i = 0; i < in.stations.size(); ++i) {
                if (!Pci1203LinkStationUp(in.stations[i])) continue;
                ++upNow;
                const Key k(in.stations[i].ring, in.stations[i].addr);
                std::vector<Key>::iterator it = std::lower_bound(watched_.begin(), watched_.end(), k);
                if (it == watched_.end() || *it != k) watched_.insert(it, k);
            }
        }

        // 3. Arm on the first UP sample; until then there is nothing to lose.
        bool justArmed = false;
        if (!armed_ && cardOk && upNow > 0) { armed_ = true; justArmed = true; }
        if (!armed_) return r;

        // 4. Lost? Card-level reasons first, then the stations.
        std::string why;
        bool lost = false;
        if (in.disabled) {
            lost = true;
            why = "card not answering - monitor auto-disabled: " + in.disabledWhy;
        } else if (!in.open) {
            lost = true;
            why = "card not open" + (in.lastErrorText.empty() ? std::string() : " - " + in.lastErrorText);
        } else if (keys.empty()) {
            lost = true;
            why = "the scan finds no EtherCAT station";
        } else {
            lost = StationsDown_(in, why);
            if (!lost && upNow == 0) {
                //  Nothing watched is down, yet nothing scanned is in OP either: a
                //  Refresh / re-attach re-learnt the ring (step 1) while it is not up.
                char b[96];
                std::snprintf(b, sizeof(b), "none of the %d scanned station%s is in OP",
                              (int)keys.size(), keys.size() == 1 ? "" : "s");
                lost = true;
                why = b;
            }
        }

        // 5. Transitions.
        if (lost) {
            const bool newlyLost = !lost_;
            if (newlyLost) { lost_ = true; lostSince_ = nowMs; lostClock_ = clock; }
            r.lost      = true;
            r.lostForMs = Pci1203LinkElapsedMs(nowMs, lostSince_);
            if (!raised_ && r.lostForMs >= (unsigned long)kPci1203LinkLostAlarmMs) {
                raised_ = true;
                ++alarms_;
                char head[96];
                std::snprintf(head, sizeof(head), "1203 link lost %lu.%lu s since %s - ",
                              r.lostForMs / 1000ul, (r.lostForMs % 1000ul) / 100ul, lostClock_.c_str());
                r.event = kAlarm;
                r.text  = Pci1203LinkSafeText(head + why, kPci1203LinkLostTextMax);
            } else if (newlyLost) {
                char tail[96];
                std::snprintf(tail, sizeof(tail), " (alarm WAR16152 if still lost after %d s)",
                              (int)(kPci1203LinkLostAlarmMs / 1000));
                r.event = kLost;
                r.text  = Pci1203LinkSafeText("1203 link lost - " + why + tail, kPci1203LinkLostTextMax);
            }
        } else {
            if (lost_) {
                char buf[160];
                std::snprintf(buf, sizeof(buf), "1203 link restored after %lu ms (lost since %s; %s)",
                              Pci1203LinkElapsedMs(nowMs, lostSince_), lostClock_.c_str(),
                              raised_ ? "WAR16152 was raised - the operator acknowledges it"
                                      : "under the limit - no alarm");
                r.event = kRestored;
                r.text  = Pci1203LinkSafeText(buf, kPci1203LinkLostTextMax);
            }
            lost_ = false; raised_ = false; lostSince_ = 0; lostClock_.clear();
        }
        if (r.event == kNone && justArmed) {
            char tail[64];
            std::snprintf(tail, sizeof(tail), " (alarm WAR16152 if lost for %d s)",
                          (int)(kPci1203LinkLostAlarmMs / 1000));
            r.event = kArmed;
            r.text  = Pci1203LinkSafeText("1203 link up - watching " + RingSummary_(in) + tail,
                                          kPci1203LinkLostTextMax);
        }
        return r;
    }

    bool          armed()   const { return armed_; }
    bool          lost()    const { return lost_; }
    bool          raised()  const { return raised_; }
    unsigned long alarms()  const { return alarms_; }   // alarms raised since construction
    std::size_t   watched() const { return watched_.size(); }

private:
    typedef std::pair<int, int> Key;   // (ring, addr)

    bool IsWatched_(int ring, int addr) const
    {
        return std::binary_search(watched_.begin(), watched_.end(), Key(ring, addr));
    }

    // "ring 0: 9/9 in OP; ring 1: 18/18 in OP" over the WATCHED stations.
    std::string RingSummary_(const Pci1203LinkInput& in) const
    {
        std::map<int, std::pair<int, int> > rings;   // ring -> (up, watched)
        for (std::size_t i = 0; i < in.stations.size(); ++i) {
            const Pci1203LinkStation& s = in.stations[i];
            if (!IsWatched_(s.ring, s.addr)) continue;
            std::pair<int, int>& t = rings[s.ring];
            ++t.second;
            if (in.open && !in.disabled && Pci1203LinkStationUp(s)) ++t.first;
        }
        std::string out;
        for (std::map<int, std::pair<int, int> >::const_iterator it = rings.begin(); it != rings.end(); ++it) {
            char b[64];
            std::snprintf(b, sizeof(b), "%sring %d: %d/%d in OP", out.empty() ? "" : "; ",
                          it->first, it->second.first, it->second.second);
            out += b;
        }
        return out.empty() ? std::string("no station") : out;
    }

    // True when a watched station is down; `why` = the per-ring tally + the first few down stations.
    bool StationsDown_(const Pci1203LinkInput& in, std::string& why) const
    {
        std::string list;
        int down = 0;
        for (std::size_t i = 0; i < in.stations.size(); ++i) {
            const Pci1203LinkStation& s = in.stations[i];
            if (!IsWatched_(s.ring, s.addr) || Pci1203LinkStationUp(s)) continue;
            if (++down > (int)kPci1203LinkLostListMax) continue;
            char b[96];
            if (s.readOk)
                std::snprintf(b, sizeof(b), "r%d addr 0x%03X%s state 0x%02X %s%s", s.ring, (unsigned)s.addr,
                              AliasText_(s).c_str(), (unsigned)s.state, Pci1203LinkStateName(s.state),
                              (s.state & 0x10u) ? "+ERR" : "");
            else
                std::snprintf(b, sizeof(b), "r%d addr 0x%03X%s read failed 0x%08lX", s.ring, (unsigned)s.addr,
                              AliasText_(s).c_str(), s.err);
            if (!list.empty()) list += " / ";
            list += b;
        }
        if (down == 0) return false;
        if (down > (int)kPci1203LinkLostListMax) {
            char b[32];
            std::snprintf(b, sizeof(b), " (+%d more)", down - (int)kPci1203LinkLostListMax);
            list += b;
        }
        char cnt[48];
        std::snprintf(cnt, sizeof(cnt), " - %d station%s not in OP: ", down, down == 1 ? "" : "s");
        why = RingSummary_(in) + cnt + list;
        return true;
    }

    static std::string AliasText_(const Pci1203LinkStation& s)
    {
        if (!s.aliasValid) return std::string();
        char b[24];
        std::snprintf(b, sizeof(b), " alias 0x%04X", (unsigned)s.alias);
        return b;
    }

    bool              armed_;
    bool              lost_;
    bool              raised_;
    unsigned long     lostSince_;
    std::string       lostClock_;
    unsigned long     alarms_;
    std::vector<Key>  scanKeys_;   // the scanned set of the last sample (sorted)
    std::vector<Key>  watched_;    // stations seen in OP, still in the scan (sorted)
};

}  // namespace ht9045

#endif  // HT9045_ETHERCAT_PCI1203LINKWATCH_H
