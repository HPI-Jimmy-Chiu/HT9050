// =============================================================================
//  WebBridgeRecipeDoc.cpp -- see WebBridgeRecipeDoc.h for the contract and for
//  why this reads through vclcompat's store (TIniStore since A5 20260924, TIniFile before) and never TMemIniFile.
//
//  AI(W906-FW-C1R) 20260911.
// =============================================================================
#include "WebBridgeRecipeDoc.h"

#include "vclcompat/IniFiles.h"
#include "vclcompat/SysUtils.h"   // FileExists
#include "vclcompat/TStringList.h"
#include "WebBridgeRecipeBcb.h"   // AI(W906-RECIPE-BCB-R13) 20260926: 第 13 條，佔用原本的空行，不移動行號
#include "WebBridge/JsonWriter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32)
#  include <windows.h>   // MoveFileExA: the atomic replace
#endif

namespace ht9045 {

namespace {

// BCB6 AnsiString::Trim strips every byte <= ' ' from both ends; IniFiles.cpp:24
// does exactly this before handing the text to StrToIntDef, so the classifier
// here has to trim the same way or it would disagree with what the machine
// reads from the same bytes.
std::string TrimBcb(const std::string& s) {
    std::string::size_type b = 0, e = s.size();
    while (b < e && static_cast<unsigned char>(s[b]) <= ' ') ++b;
    while (e > b && static_cast<unsigned char>(s[e - 1]) <= ' ') --e;
    return s.substr(b, e - b);
}

// The WHOLE trimmed text must be a decimal integer. Deliberately decimal-only:
// BCB6's StrToIntDef also accepts '$FF' and '0xFF' (IniFiles.cpp:34-41), but the
// browser contract's `type` is evidently taken from the raw's shape, and there
// is no hex value anywhere in the 1,012 .Data files measured on 20260911. A hex
// value would therefore classify as "string" -- which loses nothing, because
// `raw` carries the original spelling and the write half writes `raw` back.
bool ParseWholeDecimalInt(const std::string& t, wb_int64* out) {
    if (t.empty()) return false;
    std::string::size_type i = 0;
    if (t[i] == '+' || t[i] == '-') ++i;
    if (i >= t.size()) return false;
    for (std::string::size_type k = i; k < t.size(); ++k) {
        if (t[k] < '0' || t[k] > '9') return false;
    }
    // strtoll is C99; BCB6-era MinGW has it, and this file is C++17 anyway.
    char* end = 0;
    const long long v = ::strtoll(t.c_str(), &end, 10);
    if (!end || *end != '\0') return false;
    *out = static_cast<wb_int64>(v);
    return true;
}

// The WHOLE trimmed text must be a decimal number. Locale-independent by
// construction: the only accepted separator is '.', checked before strtod is
// asked, because a locale whose decimal point is ',' would otherwise stop at
// the '.' and report a successful partial parse of "0.20" as 0.
bool ParseWholeDecimalFloat(const std::string& t, double* out) {
    if (t.empty()) return false;
    std::string::size_type i = 0;
    if (t[i] == '+' || t[i] == '-') ++i;
    bool anyDigit = false, dot = false;
    for (std::string::size_type k = i; k < t.size(); ++k) {
        const char c = t[k];
        if (c >= '0' && c <= '9') { anyDigit = true; continue; }
        if (c == '.' && !dot)     { dot = true;      continue; }
        return false;                       // no exponent form in these files
    }
    if (!anyDigit) return false;
    // Hand-rolled rather than strtod: strtod is locale-sensitive on the decimal
    // point, and this must produce the same number on every machine.
    const bool neg = (t[0] == '-');
    std::string::size_type k = (t[0] == '+' || t[0] == '-') ? 1u : 0u;
    double whole = 0.0;
    for (; k < t.size() && t[k] != '.'; ++k) whole = whole * 10.0 + (t[k] - '0');
    double frac = 0.0, scale = 1.0;
    if (k < t.size() && t[k] == '.') {
        for (++k; k < t.size(); ++k) { frac = frac * 10.0 + (t[k] - '0'); scale *= 10.0; }
    }
    *out = (neg ? -1.0 : 1.0) * (whole + frac / scale);
    return true;
}

} // namespace

RecipeFieldType ClassifyRecipeField(const std::string& rawVerbatim) {
    const std::string t = TrimBcb(rawVerbatim);
    if (t.empty()) return kRecipeFieldString;   // empty is a string, never null
    wb_int64 i = 0;
    double   d = 0.0;
    // '.' is what separates the two numeric types in the published contract:
    // "1" is int, "2.00" is float even though its value is 2.
    if (t.find('.') == std::string::npos && ParseWholeDecimalInt(t, &i)) {
        return kRecipeFieldInt;
    }
    if (ParseWholeDecimalFloat(t, &d)) return kRecipeFieldFloat;
    return kRecipeFieldString;
}

std::string RecipeDocToJson(const vclcompat::AnsiString& path) {
    webbridge::JsonWriter w;

    // AI(W906-A5-INIREAD) 20260924: a one-shot TIniStore snapshot, no longer TIniFile. Since A5 a write-through TIniFile READS with
    // Win32 GetPrivateProfileStringA rules (values trimmed + unquoted, '#' lines are keys); this reader's published contract
    // (tests/test_wb_recipedoc.cpp groups 3-6: `raw` verbatim, '#' a comment) is the store grammar -- changing what the browser
    // shows is the user's call, not an A5 side effect. TIniStore has no destructor flush and no Write path (still NOT TMemIniFile).
    struct RecipeIni { vclcompat::TIniStore s; explicit RecipeIni(const vclcompat::AnsiString& p) { s.LoadFromFile(p); } void ReadSections(vclcompat::TStrings* d) const { s.FillSectionNames(d); } void ReadSection(const vclcompat::AnsiString& x, vclcompat::TStrings* d) const { s.FillSectionKeys(x, d); } vclcompat::AnsiString ReadString(const vclcompat::AnsiString& x, const vclcompat::AnsiString& k, const vclcompat::AnsiString& def) const { bool f = false; vclcompat::AnsiString v = s.ReadRaw(x, k, f); return f ? v : def; } } ini(path);

    // A missing file is not an error: the published documents use
    // available:false for one, and the browser renders that state.
    const bool available = vclcompat::FileExists(path);

    w.BeginObject();
    w.Key("path").String(std::string(path.c_str()));
    w.Key("available").Bool(available);
    w.Key("sections").BeginObject();

    if (available) {
        vclcompat::TStringList sections;
        ini.ReadSections(&sections);
        for (int s = 0; s < sections.GetCount(); ++s) {
            const vclcompat::AnsiString sec = sections.GetString(s);

            vclcompat::TStringList keys;
            ini.ReadSection(sec, &keys);

            w.Key(std::string(sec.c_str())).BeginObject();
            for (int k = 0; k < keys.GetCount(); ++k) {
                const vclcompat::AnsiString key = keys.GetString(k);
                // ReadString's default is only reached if the key vanished
                // between ReadSection and here, which cannot happen: the store
                // is already in memory.
                const vclcompat::AnsiString raw =
                    ini.ReadString(sec, key, vclcompat::AnsiString(""));
                const std::string rawStr(raw.c_str()); const std::string bcbStr = RecipeBcbValue(rawStr);   // AI(W906-RECIPE-BCB-R13) 20260926: BCB6 TIniFile::ReadString＝GetPrivateProfileStringA 對同一行讀到的字（golden common.cpp:331 new TIniFile、:562 ReadString）；value/type 由它推算，raw 仍逐字

                w.Key(std::string(key.c_str())).BeginObject();
                switch (ClassifyRecipeField(bcbStr)) {   // AI(W906-RECIPE-BCB-R13) 20260926: BCB6 ReadInteger／ReadFloat 解析的是 ReadString 的結果（IniFiles.cpp:566-567），例："12" 是 int 12
                    case kRecipeFieldInt: {
                        wb_int64 v = 0;
                        ParseWholeDecimalInt(TrimBcb(bcbStr), &v);
                        w.Key("value").Number(v);
                        w.Key("type").String("int");
                        break;
                    }
                    case kRecipeFieldFloat: {
                        double v = 0.0;
                        ParseWholeDecimalFloat(TrimBcb(bcbStr), &v);
                        w.Key("value").Number(v);
                        w.Key("type").String("float");
                        break;
                    }
                    default:
                        // AI(W906-RECIPE-BCB-R13) 20260926: a string's value is now what BCB6 read (bcb); `raw` below is still verbatim. Old note: The raw text, not the trimmed text: a value whose
                        // spaces are significant keeps them, and `raw` and
                        // `value` agree for strings in the published documents.
                        w.Key("value").String(bcbStr);
                        w.Key("type").String("string");
                        break;
                }
                w.Key("raw").String(rawStr); w.Key("bcb").String(bcbStr);   // AI(W906-RECIPE-BCB-R13) 20260926: 顯示用；寫入仍只認 raw（recipe.doc.put）
                w.EndObject();
            }
            w.EndObject();
        }
    }

    w.EndObject();   // sections
    w.EndObject();   // document

    // Ok() goes false on any misuse or unclosed container. Returning the buffer
    // anyway would hand the browser a truncated frame, which drops the whole
    // screen rather than one field -- so fail loudly with a valid document that
    // says nothing is available.
    if (!w.Ok()) {
        webbridge::JsonWriter e;
        e.BeginObject();
        e.Key("path").String(std::string(path.c_str()));
        e.Key("available").Bool(false);
        e.Key("sections").BeginObject().EndObject();
        e.EndObject();
        return e.Str();
    }
    return w.Str();
}

// ===========================================================================
//  C1-W -- the write half.
//
//  The value of every (section, key) the STORE would return is located by its
//  BYTE SPAN in the original file, and only those spans are replaced. Nothing
//  else is re-serialised, so comments, indentation, key order, blank lines and
//  even the lines the grammar discards survive byte-for-byte. See the header for
//  why re-serialising through TMemIniFile would silently delete 23 real
//  parameters from the one corrupted recipe on this machine.
//
//  The walk below MUST agree with vclcompat's TIniStore::LoadFromFile. It is
//  duplicated rather than shared because TIniStore does not expose spans, only
//  values -- and a value is not enough to write back in place. Every rule here
//  has a matching line in IniFiles.cpp:158-171, and the two FIRST-WINS rules
//  (section and key) were each learned the hard way on 20260911; tests
//  test_wb_recipedoc.cpp group 5 pins them for the reader, and group 9 pins them
//  for the writer.
// ===========================================================================
namespace {

// ASCII-only case fold. The store compares section and key names
// case-insensitively for bytes < 0x80 and verbatim above it
// (IniFiles.cpp:69-70), so an edit naming "test arm1" has to find [Test Arm1]
// or the browser gets a spurious notFound.
std::string FoldAscii(const std::string& s) {
    std::string o(s);
    for (std::string::size_type i = 0; i < o.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(o[i]);
        if (c >= 'A' && c <= 'Z') o[i] = static_cast<char>(c - 'A' + 'a');
    }
    return o;
}

struct ValueSpan {
    std::string::size_type begin;   // first byte after the first '='
    std::string::size_type end;     // one past the last byte before the EOL
};

bool ReadWholeFile(const char* path, std::string* out) {
    std::FILE* fp = std::fopen(path, "rb");
    if (!fp) return false;
    out->clear();
    char buf[8192];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), fp)) > 0) out->append(buf, n);
    const bool ok = (std::ferror(fp) == 0);
    std::fclose(fp);
    return ok;
}

bool WriteWholeFile(const char* path, const std::string& data) {
    std::FILE* fp = std::fopen(path, "wb");
    if (!fp) return false;
    const size_t n = data.empty() ? 0
                                  : std::fwrite(data.data(), 1, data.size(), fp);
    const bool ok = (n == data.size()) && (std::fflush(fp) == 0);
    std::fclose(fp);
    return ok;
}

// Map every (folded section, folded key) the store would expose to the byte span
// of its value. FIRST occurrence only, for both the section and the key.
void IndexValueSpans(const std::string& content,
                     std::map<std::pair<std::string, std::string>, ValueSpan>* out) {
    std::string cur;
    bool inSection = false;
    std::set<std::string> seenSections;

    std::string::size_type i = 0;
    const std::string::size_type len = content.size();
    while (i < len) {
        const std::string::size_type start = i;
        while (i < len && content[i] != '\n' && content[i] != '\r') ++i;
        const std::string::size_type lineEnd = i;
        // consume the terminator, CRLF as one -- same as IniFiles.cpp
        if (i < len && content[i] == '\r') { ++i; if (i < len && content[i] == '\n') ++i; }
        else if (i < len && content[i] == '\n') { ++i; }

        std::string::size_type b = start;
        while (b < lineEnd && static_cast<unsigned char>(content[b]) <= ' ') ++b;
        if (b >= lineEnd) continue;                       // blank
        const char c0 = content[b];
        if (c0 == ';' || c0 == '#') continue;             // comment

        if (c0 == '[') {
            std::string line = content.substr(b, lineEnd - b);
            const std::string::size_type close = line.rfind(']');
            std::string name = (close != std::string::npos && close > 0)
                                   ? line.substr(1, close - 1)
                                   : line.substr(1);
            // trim, AnsiString::Trim rules
            std::string::size_type nb = 0, ne = name.size();
            while (nb < ne && static_cast<unsigned char>(name[nb]) <= ' ') ++nb;
            while (ne > nb && static_cast<unsigned char>(name[ne - 1]) <= ' ') --ne;
            name = name.substr(nb, ne - nb);

            const std::string folded = FoldAscii(name);
            if (seenSections.count(folded)) {
                inSection = false;      // duplicate [section]: FIRST WINS
            } else {
                seenSections.insert(folded);
                cur = folded;
                inSection = true;
            }
            continue;
        }

        if (!inSection) continue;                          // pre-section key: dropped

        std::string::size_type eq = start;
        while (eq < lineEnd && content[eq] != '=') ++eq;
        if (eq >= lineEnd) continue;                       // no '=' : not a key line

        std::string key = content.substr(b, eq - b);
        std::string::size_type kb = 0, ke = key.size();
        while (kb < ke && static_cast<unsigned char>(key[kb]) <= ' ') ++kb;
        while (ke > kb && static_cast<unsigned char>(key[ke - 1]) <= ' ') --ke;
        key = FoldAscii(key.substr(kb, ke - kb));

        const std::pair<std::string, std::string> id(cur, key);
        if (out->count(id)) continue;                      // duplicate key: FIRST WINS

        ValueSpan span;
        span.begin = eq + 1;
        span.end   = lineEnd;
        (*out)[id] = span;
    }
}

} // namespace

RecipeWriteResult RecipeDocApplyEdits(const vclcompat::AnsiString& path,
                                      const std::vector<RecipeFieldEdit>& edits,
                                      RecipeWriteMode mode) {
    RecipeWriteResult r;
    r.ok = false;
    r.changed = r.identical = r.notFound = 0;

    const std::string p(path.c_str());

    std::string content;
    if (!ReadWholeFile(p.c_str(), &content)) {
        r.error = "cannot read " + p;
        return r;
    }

    std::map<std::pair<std::string, std::string>, ValueSpan> spans;
    IndexValueSpans(content, &spans);

    // Collect the replacements, keyed by span start so they can be spliced in
    // file order. A caller that sends the same field twice is a caller bug, but
    // it must not corrupt the file: the map collapses it to one replacement and
    // the counts stay honest.
    std::map<std::string::size_type, std::pair<ValueSpan, std::string> > repl;
    for (std::size_t i = 0; i < edits.size(); ++i) {
        const std::pair<std::string, std::string> id(FoldAscii(edits[i].section),
                                                     FoldAscii(edits[i].key));
        const std::map<std::pair<std::string, std::string>, ValueSpan>::const_iterator
            it = spans.find(id);
        if (it == spans.end()) {
            ++r.notFound;       // never appended -- see the header
            continue;
        }
        const ValueSpan& s = it->second;
        const std::string had = content.substr(s.begin, s.end - s.begin);
        if (had == edits[i].rawValue) { ++r.identical; continue; }
        if (repl.count(s.begin)) continue;   // same field twice: keep the first
        repl[s.begin] = std::make_pair(s, edits[i].rawValue);
        ++r.changed;
    }

    if (mode == kRecipeWriteDryRun) {
        r.ok = true;
        return r;
    }
    if (r.changed == 0) {
        // Nothing to do, and doing nothing is better than rewriting a file
        // identically: the mtime is what the gate watches.
        r.ok = true;
        return r;
    }

    // Splice. Every byte outside a replaced span is copied through untouched.
    std::string out;
    out.reserve(content.size() + 64);
    std::string::size_type cursor = 0;
    for (std::map<std::string::size_type,
                  std::pair<ValueSpan, std::string> >::const_iterator it = repl.begin();
         it != repl.end(); ++it) {
        const ValueSpan& s = it->second.first;
        out.append(content, cursor, s.begin - cursor);
        out.append(it->second.second);
        cursor = s.end;
    }
    out.append(content, cursor, content.size() - cursor);

    // Backup FIRST. If this fails nothing else is attempted -- the point of a
    // backup is to exist before the risk, not after it. Naming follows the
    // repo's own convention (.gitignore carries `*.bak_*` for exactly this).
    char stamp[32];
    const std::time_t now = std::time(0);
    std::tm* lt = std::localtime(&now);
    if (lt) {
        std::sprintf(stamp, "%04d%02d%02d_%02d%02d%02d",
                     lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                     lt->tm_hour, lt->tm_min, lt->tm_sec);
    } else {
        std::sprintf(stamp, "unknown_time");
    }
    const std::string backup = p + ".bak_" + stamp + "_webwrite";
    if (!WriteWholeFile(backup.c_str(), content)) {
        r.error = "backup failed: " + backup;
        return r;
    }
    r.backupPath = backup;

    // Temp file, closed, then atomic replace. These four steps are the web
    // author's own specification for the C++ producer
    // (JSON/Runtime-bridge-contract.json delivery.writerSteps), and they are what
    // stops a 100 ms poller reading a half-written document.
    const std::string tmp = p + ".tmp_webwrite";
    if (!WriteWholeFile(tmp.c_str(), out)) {
        r.error = "temp write failed: " + tmp;
        std::remove(tmp.c_str());
        return r;
    }
#if defined(_WIN32)
    if (!::MoveFileExA(tmp.c_str(), p.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        r.error = "atomic replace failed";
        std::remove(tmp.c_str());
        return r;
    }
#else
    // std::rename over an existing file is UB on some platforms and fails on
    // Windows, which is why the branch above exists at all.
    if (std::rename(tmp.c_str(), p.c_str()) != 0) {
        r.error = "rename failed";
        std::remove(tmp.c_str());
        return r;
    }
#endif

    // SetMD5ByFolder is NOT called. golden TfContact::SaveSetupFile
    // (cContact.cpp:14313 in V912) does not call it either -- measured 20260911
    // by reading the whole function -- so the folder checksum goes stale on a
    // local edit exactly as it does today. Adding it here would make the web
    // write behave DIFFERENTLY from the machine's own Save button, and matching
    // that button is the point. The consequence is real but pre-existing: the
    // next server download of this work file fails CompareMD5ByFolder
    // (uLotInfo.cpp:4645, the tree's only verify call) with WAR16118 when
    // IniConfig.bN20_CheckMD5 is on.
    r.ok = true;
    return r;
}

} // namespace ht9045
