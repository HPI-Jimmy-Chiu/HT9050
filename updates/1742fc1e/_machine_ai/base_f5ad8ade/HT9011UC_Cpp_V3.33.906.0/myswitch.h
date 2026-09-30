// =============================================================================
//  myswitch.h  --  TMySwitch per-point output switch controller
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/myswitch.h
//  Translation wave: W4-IO (IO HAL layer)
//  Translator: AI(W4-IO) 20260626
//
//  Key changes vs. BCB6 original:
//    - `_fastcall` removed (vclcompat macro neutralizes it).
//    - `AnsiString` from vclcompat/vcl_compat.h.
//    - BCB6 qualified ctor name `TMySwitch::TMySwitch()` -> `TMySwitch()`.
//    - All fields and methods preserved verbatim.
// =============================================================================
#ifndef myswitchH
#define myswitchH

#include "vclcompat/vcl_compat.h"   // AnsiString, __fastcall no-op

class TMySwitch {
public:
    TMySwitch();  // BCB6: _fastcall TMySwitch::TMySwitch()

    AnsiString Name;
    AnsiString Using;
    int  Ring;
    int  IP;
    int  Port;
    int  Bit;
    bool OutValue;
    bool SetValue;
    int  Type;
    void On();
    void Off();
    bool Status();
    bool Enable;
    void OnOff(bool bOn);
    int  ISABase;
};

#define MAX_SWITCH_ITEM 370
#define MAX_TTL_SWITCH  16      // Steven 20161011: TTL 8-site support

extern class TMySwitch SW[MAX_SWITCH_ITEM];
extern class TMySwitch SWBackup[MAX_TTL_SWITCH];  // Alick 20161011 (Steven): TTL 8-site support

void InitialSwitch();
void CopySwitch(TMySwitch *Source, TMySwitch *Target);

#endif // myswitchH
