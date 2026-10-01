// ===========================================================================
//  SimNet/SimNetMask.h -- W36-1 / W58: the SIM build starts every launch with the FTP / network options OFF.
//
//  AI(W906-SIM-W36-1) 20260928 (St02-E helper) prototype; AI(W906-W58) 20260930 (St02-E): reworked to Steven's W58
//  answers (decisions-decided.md:2779-2812, 20260929 08:1x): Q1 = 要 (a ticked + saved key is written to config.ini),
//  Q2 = 要 (the "in doubt" keys are masked too), Q3 = 關 + enable (customer-forced / greyed keys are masked too and their
//  component is enabled), Q4 = W906_SIM_TCP_SERVERS (TesterComm, MR !12), Q5 = 算 ("\\" save paths count as network).
//  Note: docs/SIMNET_W58.md.  ctest: SimNet_Mask (tests/test_simnet_mask.cpp).
//
//  Who calls what (the hook lines are in files St02 does not own -- the "CLAIM LINES" commit):
//    tools/wb_serve.cpp   W906_SimNetInstall() just before FileRW_IniConfig_Boot().  SIM: installs the Handler hook
//                         W906_SimNetHook (cprod.cpp) and the ELA override (W906_ElaSetBoolOverride, ElaIniOverride.cpp).
//                         SHIP (W906_NO_SOFT_SIMULTE, MachineType.h:63-65): installs nothing, both stay NULL.
//    cprod.cpp            phase 0 at the end of ReadLastSetIni (after SetCustomerLimitationForConfig, :3217),
//                         phase 1 at the start of SaveLastSetIni (:3263), phase 2 at its end (:3323).
//
//  The three phases (Mask::Phase):
//    0  every masked key: the IniConfig field and its TfConfiguration page proxy read OFF (armed) or ON (released this
//       run).  Nothing is written.  The first phase 0 of a launch resolves the table (Env::Resolve); W58 Q3: a key the
//       golden registration forces or greys for this customer is masked too and its control enabled (a hidden fixed-off
//       "no such function" entry is not).
//    1  read what the save is about to write (Env::Choice: the proxy for an HTEditList key, the field for a
//       CheckConfigurationBeforeSave key) -> released (non-zero) or armed (0) for this run.  W58 Q1: a released key (and
//       one unticked after a release this run) keeps the operator's value, so the golden save writes it to config.ini.
//       An armed key: snapshot its file text, then field + proxy = the FILE's value, so the save rewrites the file's own
//       value -- the mask itself never reaches the file.  A forced key (Binding::forced): this run only, no snapshot.
//    2  write the snapshot back for every armed key whose text the save changed (a key that was missing is removed
//       again); then field + proxy = this run's value again.
//  The ELA (EventLogAnalysis) reads config.ini itself; Mask::ElaValue answers its override: armed (or not yet resolved)
//  -> false, released -> true, anything else (not in the table, or not masked) -> the file value.
//  A new launch = a new process = everything armed again (the state is process memory only; Mask::Arm for tests).
//
//  Elsewhere: TCP 7016 / 7017 (W58-4) is TesterComm/Handler/TesterCommWiring.cpp; network save paths (W58 Q5) are
//  EventLogAnalysis/ElaSchedule.cpp SimBlocksNetPath.  Table C keys without a network effect are listed in kRows.
//
//  W906_SIMNET_CORE_ONLY (the ctest): only the core below is compiled -- no Handler globals, no FileRW proxies.
// ===========================================================================
#ifndef SIMNET_SIMNETMASK_H
#define SIMNET_SIMNETMASK_H

#include "WebBridge/Sync.h"   // WbMutex: CRITICAL_SECTION (MinGW 6.3 win32 threads have no std::mutex)

#include <cstddef>
#include <string>
#include <vector>

namespace simnet {

// kEditList: an elConfig HTEditList entry -- the save writes the page PROXY (HTEditList::SaveEditTextToFile) and copies it
//            into the field.  kSaveCopy: not in any HTEditList -- golden CheckConfigurationBeforeSave copies the proxy into
//            the IniConfig field and ProcessLastSetIni_RMS / _FTP (bWriteFile) write the FIELD.
enum RowKind { kEditList = 0, kSaveCopy = 1 };

// kPending: not resolved yet (the edit lists are not registered).  kSkipped: not masked this launch.
enum RowState { kPending = 0, kSkipped = 1, kArmed = 2, kReleased = 3 };

struct Row
{
    const char* code;      // the golden item ("N10-1"), for logs
    const char* section;   // config.ini section ("" = the Env decides: the RMS rows, cprod.cpp:2196 "RMS" / "Server")
    const char* key;       // config.ini key ("" = the Env decides)
    RowKind kind;
    const char* proxy;     // kSaveCopy: the TfConfiguration proxy name; kEditList: 0 (the HTEditList entry's control)
};

// W58 Q2 = 要: tables A + B and table C's network keys, minus N07-1 / N07-2 (Jimmy RULINGS_20261001 #3): 58 rows.  *count = the number of rows.
const Row* DefaultRows(std::size_t* count);

// What Env::Resolve found for one row.  Exactly one of bField / iField is set.
struct Binding
{
    std::string section, key;   // the names the file uses
    bool* bField;               // a boolean IniConfig field
    int* iField;                // an int IniConfig field (the N31 radio group: 0 = Disable)
    void* control;              // opaque for the Env (kEditList: the THTEdit SourceControl)
    bool onlyOneIsOn;           // how the reader parses the file for a bool: true = only "1" (HTEditList ReadInteger==1),
                                //   false = any non-zero (CheckAndReadIniData -> ReadBool)
    bool forced;                // W58 Q3: a customer-forced key (HTEditList bReadFromFile == false) -- no file value: HTEditList
                                //   never saves it, so a release lasts this run only and nothing is snapshotted / restored
    Binding() : bField(0), iField(0), control(0), onlyOneIsOn(true), forced(false) {}
};

// The Handler side (the glue in SimNetMask.cpp) or a test fake.
class Env
{
public:
    virtual ~Env() {}
    virtual std::string ConfigIniPath() = 0;
    // 1 = mask it (*b filled), 0 = do not mask it this launch, -1 = not ready (ask again at the next phase 0)
    virtual int Resolve(const Row& row, Binding* b) = 0;
    virtual void SetProxy(const Row& row, const Binding& b, int value) = 0;
    // phase 1: the value the golden save is about to write (the proxy for kEditList, the field for kSaveCopy)
    virtual int Choice(const Row& row, const Binding& b) = 0;
    virtual void Log(const std::string& line) = 0;
};

class Mask
{
public:
    Mask();
    void Attach(const Row* rows, std::size_t count, Env* env);   // also arms
    void Arm();                                                  // a new launch: every row pending, nothing released
    void Phase(int phase);                                       // 0 / 1 / 2 (above); anything else is ignored
    // the ELA override; file = the ini the ELA reads (only a file named config.ini is masked)
    bool ElaValue(const char* file, const char* section, const char* key, bool fileValue);

    std::size_t Count() const;
    const Row& RowAt(std::size_t i) const;
    RowState State(std::size_t i);
    int RunValue(std::size_t i);           // this run's value (0 = off)
    Binding BindingAt(std::size_t i);
    int CountIn(RowState s);

private:
    struct Slot
    {
        Row row;
        Binding b;
        RowState st;
        int run;
        bool snap, snapPresent, snapSection;
        std::string snapRaw;
        Slot() : st(kPending), run(0), snap(false), snapPresent(false), snapSection(false) {}
    };
    void Apply(Slot& s, int value);
    int FileValue(const Slot& s) const;
    void Phase0();
    void Phase1();
    void Phase2();

    webbridge::WbMutex mu_;
    std::vector<Slot> slots_;
    Env* env_;
};

// The raw config.ini text, with the Win32 profile rules vclcompat's in-place writer uses (vclcompat/IniFiles.cpp EOF):
// the FIRST [section] whose trimmed name matches (ASCII case-insensitive), inside it the first line whose text before
// the first '=' matches the key (trimmed, case-insensitive; a line starting with ';' is a comment).
// ReadRawValue: *value = everything after that '=', verbatim.  *sectionFound (optional) = the section exists.
bool ReadRawValue(const std::string& file, const std::string& section, const std::string& key, std::string* value,
                  bool* sectionFound = 0);
// WriteRawValue: vclcompat TIniFile::WriteString (Win32 WritePrivateProfileString, in place, value verbatim).
void WriteRawValue(const std::string& file, const std::string& section, const std::string& key,
                   const std::string& value);
// RemoveKey: removes that one line (and, when dropEmptySection, the section header too if nothing but blank lines is
// left in it).  false = no such key.
bool RemoveKey(const std::string& file, const std::string& section, const std::string& key, bool dropEmptySection);

// AI(W906-W58) 20260930 (St02-E): W58 Q3 for one elConfig HTEditList entry (Steven 20260929 08:1x 「模擬版時. 關 (改成不反灰 也就是元件要enable)」),
// from its registration (THTEdit bReadFromFile / bEnable / bVisible) and its value at the first phase 0:
//   hidden and off -> not masked: nothing to turn off, and the control stays as golden left it (this is also the
//                     registration's "this customer has no such function" fallback, bNoShow, bDisable, bFixedValue, 0);
//   hidden and on  -> masked, and SHOWN in SIM so it can be released (E2 W58 m1, VTEST N10-3 gen.inc:6324; a SIM-only
//                     deviation: golden hides it);
//   shown          -> masked; a greyed one is enabled.
// A masked entry is always enabled (bEnable + SourceControl->Enabled).  forced = bReadFromFile == false (HTEditList
// never saves it: a release lasts this run only).
struct Q3Plan
{
    bool mask, forced, show;
};
Q3Plan Q3Decide(bool readFromFile, bool enabled, bool visible, bool valueOn);

// The SIM gate.  SimBuild() = SOFT_SIMULTE is defined in this build (MachineType.h:63-65).
typedef bool (*ElaBoolFn)(const char* file, const char* section, const char* key, bool fileValue);
bool SimBuild();
// SIM: *phaseSlot = phaseFn, elaSetter(elaFn), true.  SHIP: touches nothing, false.
bool InstallHooks(void (**phaseSlot)(int), void (*phaseFn)(int), void (*elaSetter)(ElaBoolFn), ElaBoolFn elaFn);

}  // namespace simnet

#ifndef W906_SIMNET_CORE_ONLY
// tools/wb_serve.cpp, before FileRW_IniConfig_Boot (a claim line).  SHIP: does nothing.
void W906_SimNetInstall();
#endif

#endif  // SIMNET_SIMNETMASK_H
