// =============================================================================
//  WebSecurityJamW45.h -- ★W45 (formerly W20) on the Handler's Jam Code page (Status.Security, WS security.jam): the
//  second check box, the analyzer's cbIncludeMTBF (jamrules::SecondBox, JamRules.h), for WebSecurityJam.cpp.
//
//  AI(W906-ELA-W45) 20260928 (St02-E helper).  Steven 0928 item 18: 18a W20-1 = A (the one editor is this page's Jam
//  tab; eventlog.html only opens it), 18b W20-3 = B (JamRules.h switch), 18c W20-5 = A (one box per key, this file).
//  Research: D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-w20-jamcode-editor.md §5.1 / §5.5 row 4.
//  Golden: Handler 906_0625_Steven cSecurity.cpp (D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven), analyzer Rev891
//  Analyzer.cpp / Analyzer.dfm (D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code).  Ledger: docs/ELA_PORT_LEDGER.md "W45".
//
//  Header-only (same reason as JamIniMerge.h: the wb_serve source list does not change).  WebSecurityJam.cpp (not
//  St02's file -- the calls below are claim requests, ledger "W45") includes THIS instead of JamIniMerge.h (its line 68;
//  this header includes JamIniMerge.h, so nothing there is lost) and calls, all on the main thread inside the
//  JAM0000.dat lock it already holds (open :794, select / save / import :990):
//    jamw45::Load(pv())          after each golden ChangeJamMessage / cbJam*Change / spbImportClick that shows a record
//                                (= the analyzer's ChangeJamMessage :2753 for that record)
//    jamw45::Apply(k, it)        in ApplyValues, for a name no golden widget took (= the operator clicking the box;
//                                since ★W59-1 also values.RichEditJamCode, the edited description -- WebSecurityJamW59.h)
//    jamw45::Save()              right after golden SaveJamLevel (op save, FormClose :463) (= the analyzer's :2772)
//    jamw45::SaveThenLoad(pv())  right after golden cbJamAreaChange / cbJamCodeChange / cbJamLangChange (they save the
//                                "from" record, then show the new one -- golden order: the Handler's keys, then box 2)
//    jamw45::WriteBox(w, pv)     at the end of WriteState's "boxes" object (boxes.cbIncludeMTBF for the page JS)
//  pv is WebSecurityJam.cpp's Priv (file = FileNameJam000, jamArea = JamArea, jamCode = JamCode).
//  I/O is golden's: common.cpp CheckAndReadIniData (bool) -- a missing key is written with its default
//  (906_0625_Steven common.cpp:449-464) -- and WriteIniData (bool), which also writes the setting-change record when
//  the value changes (:636 on), through the same INIFile singleton as every other key of this page.
//  Customer code: CUSTOMER_CODE (cmydef.h), read at every Load, as golden SaveJamLevel / GetJemIncludeMTBA read it.
//  Not in the Jam export (W20-8 未明確裁決，照 golden: golden spbExportClick has no MTBA / MTBF column) and not in the
//  import (same).  Permission: the page's own JamTabAllowed guard (W20-2 未明確裁決，照 golden -- the Handler's).
// =============================================================================
#pragma once

#include <string>

#include "JamIniMerge.h"     // WebSecurityJam.cpp:68 included this before (the named lock, the S128 merge)
#include "JamRules.h"        // jamrules::SecondBox, the keys and the defaults
#include "common.h"          // CheckAndReadIniData / WriteIniData (golden common.cpp)
#include "cmydef.h"          // CUSTOMER_CODE
#include "Public/cJSON.h"    // cJSON_IsBool / cJSON_IsTrue (ApplyValues' value)
#include "WebSecurityJamW59.h"   // AI(W906-SEC-W59) 20260929 (St02-E): ★W59-1 the editable description, reached through Apply below

namespace jamw45 {

// golden common.cpp CheckAndReadIniData / WriteIniData, bool overloads (the Handler's getters use the same ones)
struct HandlerIo
{
    bool Read(const std::string& file, const std::string& group, const std::string& name, bool def)
    {
        return CheckAndReadIniData(AnsiString(file.c_str()), AnsiString(group.c_str()), AnsiString(name.c_str()), def);
    }
    void Write(const std::string& file, const std::string& group, const std::string& name, bool value)
    {
        WriteIniData(AnsiString(file.c_str()), AnsiString(group.c_str()), AnsiString(name.c_str()), value);
    }
};

// one box for the one fSecurity form (a function-local static: one instance in the program)
inline jamrules::SecondBox& Box()
{
    static jamrules::SecondBox box;
    return box;
}

template <class P>
inline void Load(const P& pv)
{
    HandlerIo io;
    Box().Load(io, pv.file, pv.jamArea, pv.jamCode, CUSTOMER_CODE);
}

inline void Save()
{
    HandlerIo io;
    Box().Save(io);
}

template <class P>
inline void SaveThenLoad(const P& pv)
{
    Save();
    Load(pv);
}

// true = the value was the second box's and is applied (the caller lists it in "applied")
inline bool Apply(const std::string& name, const cJSON* it)
{
    if (jamw59::Apply(name, it)) return true;   // AI(W906-SEC-W59) 20260929 (St02-E): values.RichEditJamCode (★W59-1), no claim at WebSecurityJam.cpp:160
    return it != 0 && cJSON_IsBool(it) && Box().Apply(name, cJSON_IsTrue(it) ? true : false);
}

template <class W, class P>
inline void WriteBox(W& w, const P& pv)
{
    Box().WriteJson(w, pv.jamArea, pv.jamCode);
}

}  // namespace jamw45
