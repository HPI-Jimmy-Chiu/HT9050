// ===========================================================================
//  EventLogAnalysis/EventLogCsv.h -- the ONE EventLogTxt CSV row splitter, header-only.
//  AI(W906-ELA-W15) 20260927 (St02-E).  Steven 20260927 W15 = B: split on commas only, a quoted field is one field,
//  and the Handler's log viewer (cObserver tsEventLogTxt) and the analyzer (ElaCore) share this one parser.
//
//  Rules = golden 912 cObserver.cpp:3812-3846, 912 only -- not in 906 (AddEventLogCsvField / SplitEventLogCsvLine, V899 20260817,
//  CASE-FOREHOPE_NINGBO-20260813-001), which V906 cObserver.cpp had at :1351-1385; since a9b93d61 the viewer's
//  SplitEventLogCsvLine (cObserver.cpp:1374-1385) calls this, so the viewer did not change:
//    - the line is cut at every ',' outside quotes; every '"' toggles "inside quotes" (a "" inside a quoted field
//      toggles twice, so it stays inside);
//    - each raw field is trimmed of chars <= ' ' at both ends (AnsiString::Trim); if it then starts with '"', that
//      quote is dropped, a last '"' is dropped too, and every "" becomes " (StringReplace rfReplaceAll).  A field that
//      does not start with a quote is kept as it is (no "" replacement);
//    - text inside the quotes is kept: the Handler's "\t" placeholder stays one TAB, " 08:45:33.712" keeps its blank
//      (ElaCore's date / time validators trim, golden Common.cpp:630 / :657);
//    - an unclosed quote runs to the end of the line; a trailing comma gives one more (empty) field; an empty line
//      gives one empty field (callers skip rows narrower than they need);
//    - byte-wise: cp950 trail bytes (0x40-0x7E, 0xA1-0xFE) and UTF-8 never contain ',' (0x2C) or '"' (0x22), so the
//      bytes are split as they are, no transcoding.
//  Why header-only: cObserver.cpp is in ht9045_sm, which never links ht9045_ela (ht9045_ela links vclcompat +
//  ht9045_nmftp only; wb_serve puts ht9045_ela ahead of the machine libraries, which never reference it back), and
//  vclcompat mirrors BCB6 only.  Standard C++ (the C++11 subset MinGW.org 6.3 has); no VCL types.
//  Golden 906_0625_Steven cObserver.cpp:3848 / :3892 / :3927 still has `tsRow->CommaText=...` (BCB6 blank splitting);
//  golden Rev891 Analyzer.cpp:833 too -- this splitter is the deliberate W15 deviation (ledger ELA_PORT_LEDGER.md W15).
// ===========================================================================
#ifndef HT9045_ELA_EVENTLOGCSV_H
#define HT9045_ELA_EVENTLOGCSV_H

#include <string>
#include <vector>

namespace ela {

namespace detail {
// golden 912 AddEventLogCsvField (cObserver.cpp:3812-3823; 912 only, not in 906)
inline void AddEventLogCsvField(std::vector<std::string>* row, const std::string& raw)
{
    size_t b = 0, e = raw.size();
    while (b < e && (unsigned char)raw[b] <= ' ') ++b;                 // asField=asField.Trim();
    while (e > b && (unsigned char)raw[e - 1] <= ' ') --e;
    std::string f = raw.substr(b, e - b);
    if (!f.empty() && f[0] == '"')
    {
        f.erase(0, 1);                                                  // drop the opening quote
        if (!f.empty() && f[f.size() - 1] == '"')                        // an unclosed one is tolerated, like CommaText
            f.erase(f.size() - 1);
        std::string o;                                                  // StringReplace("\"\"", "\"", rfReplaceAll)
        o.reserve(f.size());
        for (size_t i = 0; i < f.size(); ++i)
        {
            o += f[i];
            if (f[i] == '"' && i + 1 < f.size() && f[i + 1] == '"') ++i;
        }
        f.swap(o);
    }
    row->push_back(f);
}
}  // namespace detail

// golden 912 SplitEventLogCsvLine (cObserver.cpp:3825-3846; 912 only, not in 906)
inline std::vector<std::string> SplitEventLogCsv(const std::string& line)
{
    std::vector<std::string> row;
    size_t start = 0;
    bool inQuote = false;
    for (size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == '"')
            inQuote = !inQuote;
        else if (line[i] == ',' && !inQuote)
        {
            detail::AddEventLogCsvField(&row, line.substr(start, i - start));
            start = i + 1;
        }
    }
    detail::AddEventLogCsvField(&row, line.substr(start));
    return row;
}

}  // namespace ela

#endif
