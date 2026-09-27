// =============================================================================
//  forms/fOCR.cpp  --  definitions for the fOCR facade
//
//  AI(W906-W7-L1-Wave0) 20260801: NEW FILE.  See forms/fOCR.h for the golden
//  provenance and the offline-default rationale.
//
//  AI(W906-FW3-OCR1) 20260827: WAVE FW3-OCR1.  Added IsOCRCommandTrigger,
//  CheckOCRWordType, InitInsp -- see forms/fOCR.h banner for scope, denominator
//  and the OCRInsp.cpp seam hand-off.  Ctor now explicit (was implicit-default)
//  because bOcr_ReceiveOK[] needs the golden zero-init loop; kept to ONLY that
//  loop (ctor/Init split convention, forms/fContact.h (D-2) precedent) -- the
//  many golden ctor fields this facade does not carry (sOCR_Send[]/sOCR_Recv[]/
//  _STX_/_ETX_/Tester members/ListOCR*/etc.) are simply not declared, which is
//  this campaign's established "thin facade" shape, not an oversight.
// =============================================================================
#include "forms/fOCR.h"
#include "Config.h"      // IniConfig.iOCRTriggerMode / .asOCRWordType / .iOCRWordCount
#include <cctype>         // isalpha / isdigit (golden OCR.cpp:2511, :2518)
#include <cstdio>         // sprintf (golden OCR.cpp:2505-2506)

// golden OCR.cpp:63-64 -- file-scope consts (NOT declared in OCR.h). Owned
// here, not retirement debt: this wave IS "the wave that lands golden OCR.cpp"
// for these two names, and IsOCRCommandTrigger is their only consumer.
const int OCR_TRIGGER_MODE_SWITCH  = 0;
const int OCR_TRIGGER_MODE_COMMAND = 1;

// golden OCR.cpp:72-209 (TfOCR::TfOCR ctor) -- PORT SPLIT: only the
// bOcr_ReceiveOK[] zero-init subloop (golden :94, :99) is reproduced, because
// it is the only golden ctor field this thin facade carries.
TfOCR::TfOCR()
{
    for(int i=0; i<OCR_MAX_CMD; i++)
        bOcr_ReceiveOK[i]=false;
}

// Offline there is no OCR ring light to drive, so this is a no-op sink -- the
// same shape every other hardware-side facade method in forms/ uses.
void TfOCR::ChangeLightValue(int /*Channel*/, int /*Value*/) {}

// golden OCR.cpp:1700-1703 -- verbatim.  PROVENANCE NOTE (not a translation
// bug): IniConfig.iOCRTriggerMode has no loader anywhere in the ported tree
// yet (grepped 20260827 -- fOCR_DoIniDataToForm, golden OCR.cpp:2212, is the
// only ReadIniData site and it is excluded this wave, see report). IniConfig
// is a static-storage global (cprod.cpp:50) with no explicit initialiser, so
// the field reads its zero-init default = OCR_TRIGGER_MODE_SWITCH, and this
// function returns false -- which is ALSO golden's own designed fallback for
// "key missing / out of range" (OCR.cpp:2213-2215 clamps to
// OCR_TRIGGER_MODE_SWITCH).  So the unloaded-config answer is faithful to
// golden's own safety default, not an invented one; a customer .ini actually
// set to COMMAND mode will read differently from golden until a load path is
// wired.  This is the same "false" the OCRInsp.cpp seam hardcodes today
// (W906OCR_TfOCRSeam::IsOCRCommandTrigger, OCRInsp.cpp:341) -- for a different
// reason (real logic reaching the golden fallback, not a stub literal).
bool TfOCR::IsOCRCommandTrigger()
{
    return (IniConfig.iOCRTriggerMode==OCR_TRIGGER_MODE_COMMAND);
}

// golden OCR.cpp:1063-1077 -- verbatim (8 flags, the INSPECTION-result subset
// of bOcr_ReceiveOK[]).
void TfOCR::InitInsp()
{
    bOcr_ReceiveOK[ocrInspOk       ]=false;
    bOcr_ReceiveOK[ocrInspNg       ]=false;
    bOcr_ReceiveOK[ocrInspErr2001  ]=false;
    bOcr_ReceiveOK[ocrInspErr2002  ]=false;
    bOcr_ReceiveOK[ocrInspErr2003  ]=false;
    bOcr_ReceiveOK[ocrInspErr2004  ]=false;
    bOcr_ReceiveOK[ocrInspErr3001  ]=false;
    bOcr_ReceiveOK[ocrInspErr3002  ]=false;
}

// golden OCR.cpp:2499-2526.  PROVENANCE NOTE: same shape as
// IsOCRCommandTrigger above -- IniConfig.asOCRWordType has no loader in the
// ported tree (grepped 20260827; only writer is fOCR_DoIniDataToForm, excluded
// this wave) and defaults to "" (AnsiString default ctor), so the
// `if(IniConfig.asOCRWordType!="")` gate below is never taken today and this
// always returns true -- again matching golden's own OCR.cpp:2503 shape (the
// whole per-character check is SKIPPED, not "passed", when no word-type mask
// is configured) and matching the OCRInsp.cpp seam's hardcoded `return true;`
// (OCRInsp.cpp:345) for the same underlying reason as IsOCRCommandTrigger.
bool TfOCR::CheckOCRWordType(AnsiString asOCR)
{
    char str[300]="", str1[300]="";

    if(IniConfig.asOCRWordType!="")
    {
        //AI(W906-FW3-OCR1) 20260827: golden passes the raw AnsiString straight
        // into "%s" for the C-library ::sprintf (OCR.cpp:2505-2506) -- legal in
        // BCB6 because its AnsiString is layout-compatible with char*.
        // vclcompat::AnsiString wraps std::string (not trivially copyable
        // through '...'), so .c_str() is added here; the bytes landing in
        // str/str1 are identical either way. Mechanical VCL->C++ fixup, not a
        // behavior change.
        sprintf(str, "%s",  IniConfig.asOCRWordType.c_str());
        sprintf(str1, "%s", asOCR.c_str());
        //AI(W906-FW3-OCR1) 20260827: golden loop bound is `<=`, not `<`
        // (OCR.cpp:2507) -- reads iOCRWordCount+1 characters against a
        // "how many characters" count. Looks like an off-by-one; kept
        // verbatim, not "fixed".
        for(int i=0; i<=IniConfig.iOCRWordCount; i++)
        {
            if(str[i]=='A')
            {
                if(isalpha(str1[i])==false)
                {
                    return false;
                }
            }
            else if(str[i]=='N')
            {
                if(isdigit(str1[i])==false)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

TfOCR *fOCR = new TfOCR();
