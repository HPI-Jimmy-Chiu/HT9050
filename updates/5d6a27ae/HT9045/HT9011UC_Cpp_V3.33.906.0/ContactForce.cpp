// ContactForce.cpp
// Standard C++ translation of BCB6 ContactForce.cpp (V3.33.906.0_20260618).
// Source of truth: HT9011UC_Code_V3.33.906.0_20260618/ContactForce.cpp
//
// Toolchain: MinGW g++ 6.3+, C++14 or later.
//
// Translation scope: see ContactForce.h for full rationale.
// Only the pure math extracted from the four class constructors is translated.
// All VCL widget construction, global-config reads, ini I/O, event handlers,
// and cross-form calls are excluded.
//
// 1-based -> 0-based string index corrections (the #1 silent-bug risk):
//
//   BCB6 L183: Dia.Pos("Arm2_") == 0
//     AnsiString.Pos() returns 0 when NOT found (1-based; first char = 1).
//     std::string.find() returns string::npos when not found, and 0 when found
//     at position 0.  A mechanical map "== 0" would mean "found at start",
//     which is the OPPOSITE of the BCB6 intent.
//     Correct port: dia.find("Arm2_") == std::string::npos
//
//   BCB6 L185: Dia.SubString(6, Dia.Length())
//     1-based start = 6, i.e. skip chars 1-5 (5 chars = "Arm2_" or "Arm1_").
//     std::string::substr is 0-based: skip 5 chars -> substr(5).
//
//   BCB6 L441: edtCurrentType->Text.Pos("30") == 1 / Pos("60") == 1
//     1-based: == 1 means "starts with".
//     std port: text.find("30") == 0  (not translated here; UI-only).

#include "ContactForce.h"
#include <cstdlib>   // atof
#include "vclcompat/TStringList.h"   // AI(W906-COMMATEXT-FU) 20261003: NB2-1 -- SlkSplitCommaText goes through vclcompat CommaText (BCB6 since INBOX 152); this line was <cstring> "(not used; included for completeness)" -- same line, nothing below moves
#include <cstdio>    // AI(W906-FW-SLK-W31) 20260826: snprintf for the "%0.3f" ini group names

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// ParseDiameter_Standard
//   Diameter parsing for THTSLKClass (SLKV_STANDARD).
//   BCB6 source: ContactForce.cpp L171-188
//
//   Special tokens:
//     "80_Hi"  -> 80.0  (BCB6 L172-174)
//     "40x2"   -> 402.0 (BCB6 L175-178; comment: "40 dual socket")
//   Normal path:
//     When epInstall == 5 AND the string does NOT contain "Arm2_"
//     (BCB6 L181-187: Dia.Pos("Arm2_")==0 means NOT found in 1-based),
//     strip the first 5 characters (e.g. "Arm1_" prefix).
//     Then parse with atof.
//
//   BCB6 index conversion:
//     L183: Dia.Pos("Arm2_")==0  -> dia.find("Arm2_")==npos  (NOT found)
//     L185: Dia.SubString(6, Dia.Length()) -> dia.substr(5)   (skip first 5)
static double ParseDiameter_Standard(const std::string& dia, int epInstall)
{
    // BCB6 L171-174: "80_Hi" special case
    if (dia == "80_Hi")
        return 80.0;

    // BCB6 L175-178: "40x2" -> 402 (40mm dual socket sentinel)
    if (dia == "40x2")
        return 402.0;

    // BCB6 L180-188: EP_Install==5 "Arm2_" strip logic
    std::string work = dia;
    if (epInstall == 5)
    {
        // BCB6 L183: Dia.Pos("Arm2_")==0  -> AnsiString: returns 0 when NOT found
        // std port : dia.find("Arm2_")==npos means NOT found
        if (work.find("Arm2_") == std::string::npos)
        {
            // BCB6 L185: Dia=Dia.SubString(6, Dia.Length())
            // 1-based start=6 skips chars 1-5 (the "Arm1_" prefix, 5 chars).
            // std port: substr(5) -- 0-based offset 5.
            if (work.size() > 5)
                work = work.substr(5);
        }
    }

    // BCB6 L188: dDiameter=atof(Dia.c_str())
    return atof(work.c_str());
}

// ParseDiameter_Simple
//   Plain atof parse used by THTSLKIndClass, THTDieForceSLKClass,
//   and THTDieForceOneByOneSLKClass.
//   BCB6 source: L313, L404, L1545
static double ParseDiameter_Simple(const std::string& dia)
{
    return atof(dia.c_str());
}

// ComputeMinForce_Standard
//   dMinForce ladder for THTSLKClass -- includes the 402 special case.
//   BCB6 source: ContactForce.cpp L191-202
static double ComputeMinForce_Standard(double d)
{
    if (d <= 30.0) return 0.5;      // BCB6 L191-192
    if (d <  40.0) return 1.0;      // BCB6 L193-194
    if (d <  50.0) return 2.0;      // BCB6 L195-196
    if (d <  60.0) return 4.0;      // BCB6 L197-198
    if (d == 402.0) return 4.0;     // BCB6 L199-200: 40x2 sentinel (40mm dual socket)
    return 8.0;                     // BCB6 L201-202
}

// ComputeMinForce_NoSpecial
//   dMinForce ladder for THTSLKIndClass / THTDieForceSLKClass /
//   THTDieForceOneByOneSLKClass -- NO 402 special case.
//   BCB6 source: L315-324 (THTDieForceOneByOneSLKClass)
//                L406-415 (THTDieForceSLKClass)
//                L1547-1556 (THTSLKIndClass)
static double ComputeMinForce_NoSpecial(double d)
{
    if (d <= 30.0) return 0.5;      // BCB6 L315/406/1547
    if (d <  40.0) return 1.0;      // BCB6 L316/407/1548 -- note: <40 (strictly less)
    if (d <  50.0) return 2.0;      // BCB6 L317/408/1549
    if (d <  60.0) return 4.0;      // BCB6 L318/409/1550
    return 8.0;                     // BCB6 L319/410/1551
}

// ComputeMaxForce
//   Shared dMaxForce formula for all four classes.
//   BCB6 source: L204, L326, L417, L1558
//   Formula: 3.14 * (dDiameter / 100.0) * (dDiameter / 100.0) * 500
//   (all double arithmetic; the constant 3.14 is used explicitly -- NOT M_PI)
static double ComputeMaxForce(double d)
{
    double r = d / 100.0;
    return 3.14 * r * r * 500.0;
}

// ---------------------------------------------------------------------------
// ComputeSlkForce
//   Public API -- see ContactForce.h for parameter docs.
// ---------------------------------------------------------------------------
SlkForceData ComputeSlkForce(const std::string& dia,
                              int tag,
                              bool bDefault,
                              int epInstall,
                              SlkVariant variant)
{
    SlkForceData result;
    result.sDiameter = dia;
    result.iTag      = tag;
    result.bShow     = bDefault;

    switch (variant)
    {
        // ------------------------------------------------------------------
        // SLKV_STANDARD: THTSLKClass constructor calc section
        // BCB6 source: ContactForce.cpp L171-204
        // ------------------------------------------------------------------
        case SLKV_STANDARD:
        {
            result.dDiameter = ParseDiameter_Standard(dia, epInstall);
            result.dMinForce = ComputeMinForce_Standard(result.dDiameter);
            result.dMaxForce = ComputeMaxForce(result.dDiameter);
            break;
        }

        // ------------------------------------------------------------------
        // SLKV_IND: THTSLKIndClass constructor calc section
        // BCB6 source: ContactForce.cpp L1545-1558
        // ------------------------------------------------------------------
        case SLKV_IND:
        {
            result.dDiameter = ParseDiameter_Simple(dia);
            result.dMinForce = ComputeMinForce_NoSpecial(result.dDiameter);
            result.dMaxForce = ComputeMaxForce(result.dDiameter);
            break;
        }

        // ------------------------------------------------------------------
        // SLKV_DIEFORCE: THTDieForceSLKClass constructor calc section
        // BCB6 source: ContactForce.cpp L404-418
        // Ifor 20191003: add Die Force custom Kit diameter
        // Derivation identical to SLKV_IND (no 402 case).
        // ------------------------------------------------------------------
        case SLKV_DIEFORCE:
        {
            result.dDiameter = ParseDiameter_Simple(dia);
            result.dMinForce = ComputeMinForce_NoSpecial(result.dDiameter);
            result.dMaxForce = ComputeMaxForce(result.dDiameter);
            break;
        }

        // ------------------------------------------------------------------
        // SLKV_DIEFORCE_OBO: THTDieForceOneByOneSLKClass constructor calc
        // BCB6 source: ContactForce.cpp L313-327
        // Eastsun 20260525: INSTALL_DOUBLE_EP_3 addition
        // int iCount = (Tag%8)+1 -- only affects caption strings (not calc);
        // the calc section is identical to SLKV_DIEFORCE.
        // ------------------------------------------------------------------
        case SLKV_DIEFORCE_OBO:
        {
            // BCB6 L215: int iCount=(Tag%8)+1
            // iCount affects group-box captions only; no effect on dDiameter,
            // dMinForce, or dMaxForce.  Preserved here as a comment for
            // reverse-lookup clarity.
            // (int iCount = (tag % 8) + 1;  -- caption only, not used in calc)
            result.dDiameter = ParseDiameter_Simple(dia);
            result.dMinForce = ComputeMinForce_NoSpecial(result.dDiameter);
            result.dMaxForce = ComputeMaxForce(result.dDiameter);
            break;
        }

        default:
            // Unknown variant; leave fields at default (0.0).
            break;
    }

    return result;
}

// ---------------------------------------------------------------------------
// ComputeEpMaxVoltage
//   Self-contained EP voltage interpolation.
//   BCB6 source: ContactForce.cpp L1588-1598 (TfContactForce::Button1Click)
//
//   Formula (exact):
//     dMaxVol = (((dMidVol - dMinVol) / 5) * 9) + dMinVol
//
//   All arithmetic is double.  /5 is double division (no integer truncation).
//   The BCB6 operands are all double (dMinVol, dMidVol are double locals,
//   initialized from .ToDouble() at L1592-1594).
// ---------------------------------------------------------------------------
double ComputeEpMaxVoltage(double dMinVol, double dMidVol)
{
    // BCB6 L1596: dMaxVol=(((dMidVol - dMinVol)/5)*9)+dMinVol;
    return (((dMidVol - dMinVol) / 5.0) * 9.0) + dMinVol;
}

// ===========================================================================
// AI(W906-FW-SLK-W31) 20260826: THE SLK TABLE -- container, query, and
// PARSE-ONLY loaders.  Rationale, golden line map, and the parse/IO split are
// documented once, in ContactForce.h; not repeated here.  Every function below
// cites the golden lines it translates.
//
// Golden source for the whole section:
//   HT9011UC_Code_V3.33.906.0_20260618/ContactForce.cpp  (ctor fill + ReadFile)
//   HT9011UC_Code_V3.33.906.0_20260618/cContact.cpp      (the SLK walk)
//   HT9011UC_Code_V3.33.906.0_20260618/MachineType.h     (CheckRange template)
// All read cp950 on 20260826.
// ===========================================================================

// ---------------------------------------------------------------------------
// TokenAt
//   Bounds-safe stand-in for BCB6 `TStringList::Strings[i]`.
//   BCB6 RAISES EStringListError when i >= Count; golden relies on the two CSVs
//   having matching lengths and does not check (e.g. :480 indexes
//   slSLKTypeVisible with an i that runs over slSLKType).  This port cannot
//   raise a VCL exception, so a missing token becomes "" -- which then yields
//   atoi("")==0, i.e. NOT visible.  Documented as a deviation in the header;
//   chosen because defaulting to VISIBLE would invent an entry golden never
//   shows.
// ---------------------------------------------------------------------------
static std::string TokenAt(const std::vector<std::string>& tokens, size_t i)
{
    return (i < tokens.size()) ? tokens[i] : std::string();
}

// ---------------------------------------------------------------------------
// FormatF3
//   BCB6 `AnsiString::sprintf("...%0.3f...", d)` field, isolated so the four
//   group-name builders below stay one line each.
// ---------------------------------------------------------------------------
static std::string FormatF3(double d)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%0.3f", d);
    return std::string(buf);
}

// ---------------------------------------------------------------------------
// FormatI
//   BCB6 `%d` field.
// ---------------------------------------------------------------------------
static std::string FormatI(int v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", v);
    return std::string(buf);
}

// ---------------------------------------------------------------------------
// SlkClampRange
//   Local copy of the BCB6 `CheckRange` template, MachineType.h:1519-1540.
//   Deliberately NOT `#include "MachineType.h"` -- see ContactForce.h's banner
//   (that header would drag -Wpedantic diagnostics into a target with no
//   carve-out, and CMakeLists.txt is outside this wave's write boundary).
//
//   NOTE THE SWAPPED-BOUNDS ARM.  It is not defensive padding: golden's
//   ReadFile CALLS IT SWAPPED, e.g. `CheckRange(v, 0.8, 1.5)` at :1061, where
//   Maximum(0.8) < Minimum(1.5).  Dropping that arm would clamp LoadRate to
//   0.8 for every value above 0.8 instead of to the intended [0.8, 1.5].
// ---------------------------------------------------------------------------
static double SlkClampRange(double dValue, double dMaximum, double dMinimum)
{
    if (dMaximum < dMinimum)                        // MachineType.h:1522
    {
        if (dValue > dMinimum)      return dMinimum;    // :1524-1525
        else if (dValue < dMaximum) return dMaximum;    // :1526-1527
        else                        return dValue;      // :1528-1529
    }
    else
    {
        if (dValue > dMaximum)      return dMaximum;    // :1533-1534
        else if (dValue < dMinimum) return dMinimum;    // :1535-1536
        else                        return dValue;      // :1537-1538
    }
}

// ---------------------------------------------------------------------------
// SlkSplitCommaText
//   vclcompat TStringList::CommaText itself (AI(W906-COMMATEXT-FU) 20261003; was a copy of the old parseDelimited,
//   which INBOX 152 replaced by BCB6's), so the tree keeps ONE CommaText behaviour.  Golden call sites:
//   ContactForce.cpp:457, :461, :495, :499, :605, :609, :619, :623.
// ---------------------------------------------------------------------------
std::vector<std::string> SlkSplitCommaText(const std::string& sCsv)
{   { vclcompat::TStringList sl; sl.CommaText = vclcompat::AnsiString(sCsv); std::vector<std::string> v; for (int k = 0; k < sl.Count; ++k) v.push_back(vclcompat::AnsiString(sl.Strings[k]).str()); return v; }   //AI(W906-COMMATEXT-FU) 20261003: NB2-1 (laptop task (b), CHAT_JIMMY 1003 14:0x; INBOX 152 follow-up) -- golden ContactForce.cpp :455-457 / :459-461 / :493-499 / :603-623 `sl->CommaText = ...` IS vclcompat TStringList::SetCommaText, which is BCB6's parse since 71be4468 (an unquoted item ends at any char <= ' ', "a,b," -> a,b,"", "" -> no item); the copy below was the OLD vclcompat parser, now different -- kept under #if 0 so no line below moves
#if 0 // AI(W906-COMMATEXT-FU) 20261003: the pre-INBOX-152 parser (vclcompat parseDelimited copy) -- replaced by the line above; this line was `const char cDelim = ',';`
    const char cQuote = '"';

    std::vector<std::string> out;
    std::string::size_type i = 0;
    const std::string::size_type n = sCsv.size();

    while (i <= n)
    {
        // skip leading blanks (<= ' ') before a token
        while (i < n && static_cast<unsigned char>(sCsv[i]) <= ' ' && sCsv[i] != cDelim)
            ++i;

        std::string field;
        if (i < n && sCsv[i] == cQuote)
        {
            ++i;                                    // opening quote
            while (i < n)
            {
                if (sCsv[i] == cQuote)
                {
                    if (i + 1 < n && sCsv[i + 1] == cQuote) { field += cQuote; i += 2; }
                    else                                    { ++i; break; }   // closing quote
                }
                else
                {
                    field += sCsv[i];
                    ++i;
                }
            }
        }
        else
        {
            while (i < n && sCsv[i] != cDelim) { field += sCsv[i]; ++i; }
            // trim trailing blanks on an unquoted field
            while (!field.empty() &&
                   static_cast<unsigned char>(field[field.size() - 1]) <= ' ')
                field.erase(field.size() - 1);
        }
        out.push_back(field);

        // A TRAILING delimiter re-enters the loop once with i==n and emits the
        // single trailing empty field -- BCB6 turns "a,b," into {a,b,""}.
        if (i < n && sCsv[i] == cDelim) { ++i; continue; }
        break;
    }
    if (sCsv.empty())
        out.clear();                                // BCB6: empty source -> empty list
#endif // AI(W906-COMMATEXT-FU) 20261003: was `return out;` (the old parser's end)
}

// ---------------------------------------------------------------------------
// SlkTokenIsUsable
//   golden's per-token filter, written identically at :465, :507, :520, :613
//   and :630:  `Strings[i]!="" && atof(Strings[i].c_str())>15.0`.
// ---------------------------------------------------------------------------
static bool SlkTokenIsUsable(const std::string& sToken)
{
    return (!sToken.empty()) && (atof(sToken.c_str()) > 15.0);
}

// ---------------------------------------------------------------------------
// SlkForceTable -- ctor stuffs fields only (see header).
// ---------------------------------------------------------------------------
SlkForceTable::SlkForceTable()
    : variant(SLKV_STANDARD), bLoaded(false), bNeedWriteFile(false), items()
{
}

void SlkForceTable::Clear()
{
    items.clear();
    bLoaded        = false;
    bNeedWriteFile = false;
}

// ---------------------------------------------------------------------------
// SlkForceTable::ResolveKitDiameter
//   golden cContact.cpp:18710-18727, inside TfContact::CalculateTotalAirForce.
//
//   Preserved exactly:
//     * NO `break` -- golden keeps scanning, so the LAST match wins (:18720).
//     * d2 is recomputed from the item text on EVERY iteration (:18713-18718);
//       it is loop-invariant, but hoisting it would silently diverge if anyone
//       later makes the key per-entry, so the shape is kept.
//     * The "40x2" -> 402 sentinel (:18715-18718) and the 402 -> 40 collapse
//       that happens ONLY on a match (:18724-18725).
//
//   NOT applied here, on purpose: golden's `if(iTag==-1 || iTag>SLKClass.size())
//   iTag=0;` at :18761.  That belongs to the caller.  Worth flagging while it
//   is in view: golden's guard uses `>` where the array bound needs `>=`, and
//   it forces iTag to 0 even when the table is EMPTY -- so golden's :19008 /
//   :19012 `SLKClass[iTag]->dContactOffset*` would index an empty vector if it
//   were ever reached with dKitDiameter outside {20,28,30,40,58,60,80}.  With
//   dKitDiameter stuck at its 30.0 initialiser it is not reached, which is why
//   the latent fault has never fired.  A port that resolves properly must NOT
//   reproduce that dereference; check `bMatched` first.
// ---------------------------------------------------------------------------
SlkKitResolve SlkForceTable::ResolveKitDiameter(const std::string& sItemText) const
{
    SlkKitResolve result;                                       // iTag=-1, 30.0 -- :18683/:18680

    for (size_t i = 0; i < items.size(); ++i)                   // :18710
    {
        const double d1 = items[i].dDiameter;                   // :18712
        double d2 = atof(sItemText.c_str());                    // :18713
        if (sItemText == "40x2")                                // :18714-18715
            d2 = 402;                                           // :18717

        if (d1 == d2)                                           // :18720
        {
            result.bMatched     = true;
            result.iTag         = static_cast<int>(i);          // :18722
            result.dKitDiameter = d2;                           // :18723
            if (result.dKitDiameter == 402)                     // :18724
                result.dKitDiameter = 40;                       // :18725
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// SlkForceTables / ContactForceTables
//   Port home for golden's four `fContactForce->*` vectors.
//   Function-local static: constructed on FIRST USE, never at static-init time.
// ---------------------------------------------------------------------------
SlkForceTables::SlkForceTables()
    : SLKClass(), SLKIndClass(), DieForceSLKClass(), DieForceOneByOneSLKClass()
{
    SLKClass.variant                 = SLKV_STANDARD;       // golden THTSLKClass
    SLKIndClass.variant              = SLKV_IND;            // golden THTSLKIndClass
    DieForceSLKClass.variant         = SLKV_DIEFORCE;       // golden THTDieForceSLKClass
    DieForceOneByOneSLKClass.variant = SLKV_DIEFORCE_OBO;   // golden THTDieForceOneByOneSLKClass
    //AI(W906-P2b-CF) 20260919: 兩個 token 數的初值。
    iSlkTypeIndTokens   = 0;
    iDieForceTypeTokens = 0;
}

SlkForceTables& ContactForceTables()
{
    static SlkForceTables tables;
    return tables;
}

// ---------------------------------------------------------------------------
// Ini defaults -- golden ContactForce.cpp:439/:440/:448/:449/:452/:453/
// :579/:582/:586/:591/:592.
// ---------------------------------------------------------------------------
const char* SlkDefaultTypeCsv(bool bCustomerKyecLee)
{
    return bCustomerKyecLee ? "28,40,58,56"     // :439
                            : "30,40,60,56";    // :448
}

const char* SlkDefaultVisibleCsv(bool bCustomerKyecLee)
{
    return bCustomerKyecLee ? "1,1,1,1"         // :440
                            : "1,1,1,0";        // :449
}

const char* SlkDefaultTypeIndCsv()    { return "20,30"; }   // :452
const char* SlkDefaultVisibleIndCsv() { return "1,0";   }   // :453

const char* SlkDefaultDieForceTypeCsv() { return "20,30,40,50"; }   // :579 / :591

const char* SlkDefaultDieForceVisibleCsv(bool bSpilFunction)
{
    // :580-587 -- SPIL gets all four visible; everyone else hides the fourth.
    // (:592's no-file arm uses the non-SPIL string unconditionally, which is a
    // golden asymmetry, not a transcription slip -- reproduce it at the CALL
    // SITE by passing false on that path, not by changing this helper.)
    return bSpilFunction ? "1,1,1,1"    // :582
                         : "1,1,1,0";   // :586 / :592
}

// ---------------------------------------------------------------------------
// SlkComputeNeedWriteFile -- golden :437-445.
//   1-based Pos() conversion: `Text.Pos("30")==1` is "STARTS WITH", because
//   AnsiString::Pos is 1-based and returns 0 when absent.  std port: find()==0.
// ---------------------------------------------------------------------------
bool SlkComputeNeedWriteFile(const std::string& sTypeCsv, bool bCustomerKyecLee)
{
    if (!bCustomerKyecLee)                                      // :437 / :446
        return false;
    return (sTypeCsv.find("30") == 0) || (sTypeCsv.find("60") == 0);   // :441
}

// ---------------------------------------------------------------------------
// LoadSlkTable -- golden ContactForce.cpp:455-491 (SLKClass fill).
// ---------------------------------------------------------------------------
void LoadSlkTable(SlkForceTable& table,
                  const std::string& sTypeCsv,
                  const std::string& sVisibleCsv,
                  const SlkLoadOptions& opt)
{
    table.Clear();
    table.variant        = SLKV_STANDARD;
    table.bNeedWriteFile = SlkComputeNeedWriteFile(sTypeCsv, opt.bCustomerKyecLee);  // :441-444

    const std::vector<std::string> slSLKType        = SlkSplitCommaText(sTypeCsv);     // :455-457
    const std::vector<std::string> slSLKTypeVisible = SlkSplitCommaText(sVisibleCsv);  // :459-461

    for (size_t i = 0; i < slSLKType.size(); ++i)                                      // :463
    {
        const std::string& sType = slSLKType[i];
        if (!SlkTokenIsUsable(sType))                                                  // :465
            continue;

        const int  iSlot    = static_cast<int>(i);
        const bool bVisible = (atoi(TokenAt(slSLKTypeVisible, i).c_str()) != 0);       // (bool)atoi(...)

        if (sType == "402")                                                            // :467
        {
            // :469 -- the 40mm dual-socket sentinel is CONSTRUCTED as "40x2";
            // ComputeSlkForce's SLKV_STANDARD arm then maps that to 402.0.
            table.items.push_back(ComputeSlkForce("40x2", iSlot, bVisible,
                                                  opt.iEpInstall, SLKV_STANDARD));
        }
        else if (opt.iEpInstall == 5)                                                  // :471
        {
            // :473-476 -- TWO entries per token, BOTH tagged i.
            // Consequence, golden's and preserved: ParseDiameter_Standard strips
            // the 5-char prefix only when "Arm2_" is ABSENT, so the "Arm1_" entry
            // parses to the real diameter and the "Arm2_" entry parses to 0.0.
            table.items.push_back(ComputeSlkForce("Arm1_" + sType, iSlot, bVisible,
                                                  opt.iEpInstall, SLKV_STANDARD));
            table.items.push_back(ComputeSlkForce("Arm2_" + sType, iSlot, bVisible,
                                                  opt.iEpInstall, SLKV_STANDARD));
        }
        else
        {
            table.items.push_back(ComputeSlkForce(sType, iSlot, bVisible,              // :480
                                                  opt.iEpInstall, SLKV_STANDARD));
        }

        if (opt.bCustomerAseSg && sType == "80")                                       // :483-485
        {
            // :487 -- note the tag is i+1, which COLLIDES with the next token's
            // tag.  Golden's; kept.  Nothing indexes SLKClass by iTag (the walk
            // returns the VECTOR position), so the collision is inert.
            table.items.push_back(ComputeSlkForce("80_Hi", iSlot + 1, bVisible,
                                                  opt.iEpInstall, SLKV_STANDARD));
        }
    }
    table.bLoaded = true;
}

// ---------------------------------------------------------------------------
// LoadSlkIndTable -- golden ContactForce.cpp:493-539 (SLKIndClass fill).
//   `epInstall` is not forwarded to ComputeSlkForce here because the
//   SLKV_IND arm parses with a plain atof (golden :1545) and ignores it.
// ---------------------------------------------------------------------------
void LoadSlkIndTable(SlkForceTable& table,
                     const std::string& sTypeIndCsv,
                     const std::string& sVisibleIndCsv,
                     const std::string& sVisibleCsv,
                     const SlkIndLoadOptions& opt)
{
    table.Clear();
    table.variant = SLKV_IND;

    const std::vector<std::string> slSLKTypeInd        = SlkSplitCommaText(sTypeIndCsv);     // :493-495
    const std::vector<std::string> slSLKTypeIndVisible = SlkSplitCommaText(sVisibleIndCsv);  // :497-499
    const std::vector<std::string> slSLKTypeVisible    = SlkSplitCommaText(sVisibleCsv);     // :459-461

    if (opt.bDoubleEpMulti)                                                                  // :501
    {
        for (size_t i = 0; i < slSLKTypeInd.size(); ++i)                                     // :503
        {
            for (int j = 0; j < 8; ++j)                                                      // :505
            {
                const std::string& sType = slSLKTypeInd[i];
                if (!SlkTokenIsUsable(sType))                                                // :507
                    continue;
                const bool bVisible = (atoi(TokenAt(slSLKTypeIndVisible, i).c_str()) != 0);
                table.items.push_back(ComputeSlkForce(sType,                                 // :509
                                                      static_cast<int>(i) * 8 + j,
                                                      bVisible, 0, SLKV_IND));
            }
        }
    }
    else
    {
        for (size_t i = 0; i < slSLKTypeInd.size(); ++i)                                     // :516
        {
            for (int j = 0; j < 16; ++j)                                                     // :518
            {
                const std::string& sType = slSLKTypeInd[i];
                if (!SlkTokenIsUsable(sType))                                                // :520
                    continue;

                const int iSlot = static_cast<int>(i) * 16 + j;

                if (((opt.iIndEPCnt == 4) ||                                                 // :522
                     (opt.iIndEPCnt == 8 && opt.bTestModeDualSite)) &&                       // :523
                    (j == 2 || j == 3 || j >= 6))                                            // :524
                {
                    table.items.push_back(ComputeSlkForce(sType, iSlot, false,               // :526
                                                          0, SLKV_IND));
                }
                else if (opt.iIndEPCnt == 8 && (j >= 8))                                     // :528
                {
                    table.items.push_back(ComputeSlkForce(sType, iSlot, false,               // :530
                                                          0, SLKV_IND));
                }
                else
                {
                    // :534 -- READS slSLKTypeVisible (the SLKClass visible list),
                    // NOT slSLKTypeIndVisible.  Verified against the golden source
                    // 20260826.  Verbatim; do not "correct" it.
                    const bool bVisible = (atoi(TokenAt(slSLKTypeVisible, i).c_str()) != 0);
                    table.items.push_back(ComputeSlkForce(sType, iSlot, bVisible,
                                                          0, SLKV_IND));
                }
            }
        }
    }
    table.bLoaded = true;
}

// ---------------------------------------------------------------------------
// SlkDieForceVisibleCsvForSpil -- golden ContactForce.cpp:595-601.
// ---------------------------------------------------------------------------
std::string SlkDieForceVisibleCsvForSpil(const std::string& sDieForceTypeCsv,
                                         const std::string& sDieForceVisibleCsv,
                                         bool bSpilFunction)
{
    if (bSpilFunction)                                          // :595
    {
        if (sDieForceTypeCsv == "20,30,40,50")                  // :597
            return "1,1,1,1";                                   // :599
    }
    return sDieForceVisibleCsv;
}

// ---------------------------------------------------------------------------
// LoadDieForceSlkTable -- golden ContactForce.cpp:603-615.
// ---------------------------------------------------------------------------
void LoadDieForceSlkTable(SlkForceTable& table,
                          const std::string& sDieForceTypeCsv,
                          const std::string& sDieForceVisibleCsv)
{
    table.Clear();
    table.variant = SLKV_DIEFORCE;

    const std::vector<std::string> slDieForceSLKType        = SlkSplitCommaText(sDieForceTypeCsv);     // :603-605
    const std::vector<std::string> slDieForceSLKTypeVisible = SlkSplitCommaText(sDieForceVisibleCsv);  // :607-609

    for (size_t i = 0; i < slDieForceSLKType.size(); ++i)                                              // :611
    {
        const std::string& sType = slDieForceSLKType[i];
        if (!SlkTokenIsUsable(sType))                                                                  // :613
            continue;
        const bool bVisible = (atoi(TokenAt(slDieForceSLKTypeVisible, i).c_str()) != 0);
        table.items.push_back(ComputeSlkForce(sType, static_cast<int>(i), bVisible,                    // :614
                                              0, SLKV_DIEFORCE));
    }
    table.bLoaded = true;
}

// ---------------------------------------------------------------------------
// LoadDieForceOneByOneSlkTable -- golden ContactForce.cpp:617-636.
//   golden iterates slDieForceOneByOneSLKType (:626) yet tests/reads
//   slDieForceSLKType (:630/:632); :605 and :619 assign BOTH from the same
//   edtDieForceCurrentType->Text, so they cannot differ and one CSV is enough.
// ---------------------------------------------------------------------------
void LoadDieForceOneByOneSlkTable(SlkForceTable& table,
                                  const std::string& sDieForceTypeCsv,
                                  const std::string& sDieForceOneByOneVisibleCsv)
{
    table.Clear();
    table.variant = SLKV_DIEFORCE_OBO;

    const std::vector<std::string> slType    = SlkSplitCommaText(sDieForceTypeCsv);            // :617-619
    const std::vector<std::string> slVisible = SlkSplitCommaText(sDieForceOneByOneVisibleCsv); // :621-623

    for (size_t i = 0; i < slType.size(); ++i)                                                 // :626
    {
        for (int j = 0; j < 8; ++j)                                                            // :628
        {
            const std::string& sType = slType[i];
            if (!SlkTokenIsUsable(sType))                                                      // :630
                continue;
            const bool bVisible = (atoi(TokenAt(slVisible, i).c_str()) != 0);
            table.items.push_back(ComputeSlkForce(sType,                                       // :632
                                                  static_cast<int>(i) * 8 + j,
                                                  bVisible, 0, SLKV_DIEFORCE_OBO));
        }
    }
    table.bLoaded = true;
}

// ---------------------------------------------------------------------------
// SlkIniGroupName -- golden ContactForce.cpp:1040-1060.
// ---------------------------------------------------------------------------
std::string SlkIniGroupName(const SlkForceData& entry,
                            bool  bCustomerAseSg,
                            int   iEpInstall,
                            bool& bHasDiameter,
                            bool* pbContactHighOffsetTabVisible)
{
    if (bCustomerAseSg && entry.dDiameter == 80)                        // :1040
    {
        if (bHasDiameter == false)                                      // :1042
        {
            bHasDiameter = true;                                        // :1044
            if (pbContactHighOffsetTabVisible != 0)
                *pbContactHighOffsetTabVisible = entry.bShow;           // :1046
            return "Diameter_" + FormatF3(entry.dDiameter) + "mm";      // :1045
        }
        return "Diameter_" + FormatF3(entry.dDiameter) + "_Himm";       // :1050
    }
    else if (iEpInstall == 5)                                           // :1053
    {
        // :1055 -- the RAW token ("Arm1_30"/"Arm2_30"), not the parsed number.
        return "Diameter_" + entry.sDiameter + "mm";
    }
    return "Diameter_" + FormatF3(entry.dDiameter) + "mm";              // :1059
}

// ---------------------------------------------------------------------------
// DieForceSlkIniGroupName -- golden ContactForce.cpp:1081.
// ---------------------------------------------------------------------------
std::string DieForceSlkIniGroupName(const SlkForceData& entry)
{
    return "DieForceDiameter_" + FormatF3(entry.dDiameter) + "mm";
}

// ---------------------------------------------------------------------------
// DieForceOneByOneSlkIniGroupName -- golden ContactForce.cpp:1093-1096.
// ---------------------------------------------------------------------------
std::string DieForceOneByOneSlkIniGroupName(const SlkForceData& entry, int j)
{
    if (j < 4)                                                                          // :1093
        return "DieForceDiameter_" + FormatF3(entry.dDiameter) + "mm_Arm1_" + FormatI(j + 1);   // :1094
    return "DieForceDiameter_" + FormatF3(entry.dDiameter) + "mm_Arm2_" + FormatI(j - 3);       // :1096
}

// ---------------------------------------------------------------------------
// SlkIndIniGroupName -- golden ContactForce.cpp:1149-1152 (MULTI) and :1168.
// ---------------------------------------------------------------------------
std::string SlkIndIniGroupName(const SlkForceData& entry, int j, bool bDoubleEpMulti)
{
    if (bDoubleEpMulti)                                                                 // :1143
    {
        if (j < 4)                                                                      // :1149
            return "Diameter_" + FormatF3(entry.dDiameter) + "mm_Arm1_" + FormatI(j + 1);   // :1150
        return "Diameter_" + FormatF3(entry.dDiameter) + "mm_Arm2_" + FormatI(j - 3);       // :1152
    }
    return "Diameter_" + FormatF3(entry.dDiameter) + "mm_" + FormatI(j);                // :1168
}

// ---------------------------------------------------------------------------
// ApplySlkIniValues -- golden ContactForce.cpp:1061-1065.
//   Bound pairs are golden's argument order, SWAPPED where golden swaps them.
// ---------------------------------------------------------------------------
void ApplySlkIniValues(SlkForceData& entry, const SlkIniValues& raw)
{
    entry.dLoadRate         = SlkClampRange(raw.dLoadRate,          0.8,  1.5);    // :1061
    entry.dLoadRate_NS      = SlkClampRange(raw.dLoadRate_NS,       0.8,  1.5);    // :1062
    entry.dHotOffset        = SlkClampRange(raw.dHotOffset,         0.5, -0.5);    // :1063
    entry.dContactOffset    = SlkClampRange(raw.dContactOffset,    10.0, -10.0);   // :1064
    entry.dContactOffset_NS = SlkClampRange(raw.dContactOffset_NS, 10.0, -10.0);   // :1065
}

// ---------------------------------------------------------------------------
// ApplySlkPairIniValues -- golden :1082-1083, :1097-1098, :1153-1154, :1169-1170.
//   All four sites clamp the same two fields with the same two bound pairs;
//   only the ini DEFAULT differs (1.0 for DieForce ContactOffset at :1083, 0.0
//   elsewhere) and defaults belong to the deferred IO call, not here.
// ---------------------------------------------------------------------------
void ApplySlkPairIniValues(SlkForceData& entry, double dLoadRate, double dContactOffset)
{
    entry.dLoadRate      = SlkClampRange(dLoadRate,       0.8,  1.5);
    entry.dContactOffset = SlkClampRange(dContactOffset, 10.0, -10.0);
}

// ---------------------------------------------------------------------------
// SlkTrackBarPosition -- golden ContactForce.cpp:1067-1068.
//   `dTemp=dLoadRate*(double)100.0;` then `Position=int(dTemp);`
//   The explicit (double) cast is golden's (its comment says it forces double
//   first to dodge a float rounding error) and the int() TRUNCATION is kept.
// ---------------------------------------------------------------------------
int SlkTrackBarPosition(double dLoadRate)
{
    const double dTemp = dLoadRate * static_cast<double>(100.0);
    return static_cast<int>(dTemp);
}

// ---------------------------------------------------------------------------
// ApplyKyecDiameterRemap -- golden ContactForce.cpp:1109-1125, MEMORY HALF ONLY.
//   The persistence half (:1126-1139: edtCurrentType/edtVisible rewrite,
//   WriteIniData, rgKitDiameter rebuild, WriteFile) is deferred with the rest
//   of the IO half -- see the header for what that costs a caller.
// ---------------------------------------------------------------------------
void ApplyKyecDiameterRemap(SlkForceTable& table)
{
    table.bNeedWriteFile = false;                           // :1111
    for (size_t i = 0; i < table.items.size(); ++i)         // :1112
    {
        if (table.items[i].dDiameter == 30)                 // :1114
        {
            table.items[i].dDiameter = 28;                  // :1116
            table.items[i].sDiameter = "28";                // :1117
        }
        else if (table.items[i].dDiameter == 60)            // :1119
        {
            table.items[i].dDiameter = 58;                  // :1121
            table.items[i].sDiameter = "58";                // :1122
        }
        table.items[i].bShow = true;                        // :1124
    }
}
