// ===========================================================================
//  vclcompat/TStringList.cpp -- BCB6 TStringList implementation (used subset).
// ===========================================================================
#include "vclcompat/TStringList.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace vclcompat {

// ---------------------------------------------------------------------------
//  element access (0-based)
// ---------------------------------------------------------------------------
int TStringList::Add(const AnsiString& s) {
    items_.push_back(s);
    objects_.push_back(0);
    syncCount();
    return static_cast<int>(items_.size()) - 1;
}

int TStringList::AddObject(const AnsiString& s, TObject* obj) {
    items_.push_back(s);
    objects_.push_back(obj);
    syncCount();
    return static_cast<int>(items_.size()) - 1;
}

void TStringList::Insert(int index, const AnsiString& s) {
    if (index < 0) index = 0;
    if (index > static_cast<int>(items_.size())) index = static_cast<int>(items_.size());
    items_.insert(items_.begin() + index, s);
    objects_.insert(objects_.begin() + index, static_cast<TObject*>(0));
    syncCount();
}

void TStringList::Delete(int index) {
    if (index < 0 || index >= static_cast<int>(items_.size())) return;
    items_.erase(items_.begin() + index);
    objects_.erase(objects_.begin() + index);
    syncCount();
}

void TStringList::Clear() {
    items_.clear();
    objects_.clear();
    syncCount();
}

int TStringList::IndexOf(const AnsiString& s) const {
    for (size_t i = 0; i < items_.size(); ++i)
        if (items_[i] == s) return static_cast<int>(i);   // BCB6 IndexOf: case-sensitive
    return -1;
}

AnsiString TStringList::First() const {
    return items_.empty() ? AnsiString() : items_.front();
}

void TStringList::Sort() {
    // BCB6 default Sort uses AnsiCompareText-ish ordering; the source's Sort
    // comparators rely on AnsiString operator<, so we sort by that (byte order).
    // objects_ is reset to parallel the sorted strings (objects rarely combined
    // with Sort in this codebase). To keep object association we sort pairs.
    std::vector<size_t> idx(items_.size());
    for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
    std::stable_sort(idx.begin(), idx.end(),
        [this](size_t a, size_t b) { return items_[a] < items_[b]; });
    std::vector<AnsiString> ni; ni.reserve(items_.size());
    std::vector<TObject*>   no; no.reserve(objects_.size());
    for (size_t k = 0; k < idx.size(); ++k) { ni.push_back(items_[idx[k]]); no.push_back(objects_[idx[k]]); }
    items_.swap(ni);
    objects_.swap(no);
}

AnsiString TStringList::GetString(int i) const {
    if (i < 0 || i >= static_cast<int>(items_.size())) return AnsiString();
    return items_[i];
}

void TStringList::SetString(int i, const AnsiString& s) {
    if (i < 0 || i >= static_cast<int>(items_.size())) return;
    items_[i] = s;
}

TObject* TStringList::GetObject(int i) const {
    if (i < 0 || i >= static_cast<int>(objects_.size())) return 0;
    return objects_[i];
}

void TStringList::SetObject(int i, TObject* o) {
    if (i < 0 || i >= static_cast<int>(objects_.size())) return;
    objects_[i] = o;
}

// ---------------------------------------------------------------------------
//  Text.  BCB6 TStrings::GetTextStr appends sLineBreak (CRLF on Windows) after
//  EVERY string, INCLUDING the last one.
//
//  AI(W906-PT-W1-integrate) 20260807: this used to join with a bare '\n' and emit
//  no terminator after the final line, with the note "Good enough for the
//  read/parse uses here".  It stopped being good enough the moment a WRITER used
//  it: Public/MyStringList.cpp feeds `MyList->Text` straight into a raw
//  ::WriteFile (golden MyStringList.cpp:305 -> :322, port :562 -> :579), so the
//  missing terminator made the FIRST line of each flush concatenate onto the LAST
//  line of the previous one -- one fused record per append seam, i.e. once every
//  MaxLineCount lines, in every TMyStringList log (EventLog, production CSVs,
//  slEventLog, sl2DMappingLog, slGroundManLog, tsSoftwareExeTime).  Measured, not
//  argued: tests/test_ptw1_mystringlist.cpp asserted golden's disk bytes and 8 of
//  its assertions failed on exactly this.
//  SetText is unaffected -- it does not create a trailing empty item for a final
//  line break (see the final `if` in SetText below), so Text round-trips.
// ---------------------------------------------------------------------------
AnsiString TStringList::GetText() const {
    std::string out;
    for (size_t i = 0; i < items_.size(); ++i) {
        out += items_[i].str();
        out += "\r\n";
    }
    return AnsiString(out);
}

void TStringList::SetText(const AnsiString& v) {
    items_.clear();
    objects_.clear();
    const std::string& s = v.str();
    std::string line;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\n') {
            items_.push_back(AnsiString(line));
            objects_.push_back(0);
            line.clear();
        } else if (c == '\r') {
            // swallow CR; a following LF closes the line (CRLF). A lone CR also
            // closes a line (old-mac), matching BCB6 line splitting tolerance.
            if (i + 1 < s.size() && s[i + 1] == '\n') continue;
            items_.push_back(AnsiString(line));
            objects_.push_back(0);
            line.clear();
        } else {
            line += c;
        }
    }
    if (!line.empty() || (!s.empty() && (s[s.size()-1] == '\n' || s[s.size()-1] == '\r'))) {
        if (!line.empty()) { items_.push_back(AnsiString(line)); objects_.push_back(0); }
    }
    syncCount();
}

const char* TStringList::GetTextStr() {
    textCache_ = GetText().str();
    return textCache_.c_str();
}

// ---------------------------------------------------------------------------
//  CommaText / DelimitedText  (Delphi quoting rules)
//   * fields separated by Delimiter (',' for CommaText)
//   * a field is quoted with QuoteChar ('"') if it contains the delimiter,
//     a quote char, or whitespace / control chars
//   * embedded quote chars are doubled
// ---------------------------------------------------------------------------
static AnsiString buildDelimited(const std::vector<AnsiString>& items,
                                 char delim, char quote) {
    std::string out;
    for (size_t i = 0; i < items.size(); ++i) {
        const std::string& f = items[i].str();
        bool needQuote = f.empty();
        for (size_t k = 0; k < f.size() && !needQuote; ++k) {
            unsigned char ch = static_cast<unsigned char>(f[k]);
            if (ch <= ' ' || f[k] == delim || f[k] == quote) needQuote = true;
        }
        if (needQuote) {
            out += quote;
            for (size_t k = 0; k < f.size(); ++k) {
                if (f[k] == quote) out += quote;  // double embedded quotes
                out += f[k];
            }
            out += quote;
        } else {
            out += f;
        }
        if (i + 1 < items.size()) out += delim;
    }
    return AnsiString(out);
}

static void parseDelimited(const std::string& s, char delim, char quote,
                           std::vector<AnsiString>& out) {
    out.clear();
    size_t i = 0, n = s.size();
    // Delphi: leading spaces before a field are skipped; parsing is tolerant.
    while (i <= n) {
        // skip leading blanks (<= ' ') before a token
        while (i < n && static_cast<unsigned char>(s[i]) <= ' ' && s[i] != delim) ++i;
        std::string field;
        if (i < n && s[i] == quote) {
            ++i; // opening quote
            while (i < n) {
                if (s[i] == quote) {
                    if (i + 1 < n && s[i + 1] == quote) { field += quote; i += 2; }
                    else { ++i; break; }   // closing quote
                } else {
                    field += s[i]; ++i;
                }
            }
        } else {
            while (i < n && s[i] != delim) { field += s[i]; ++i; }
            // trim trailing blanks on unquoted field
            while (!field.empty() && static_cast<unsigned char>(field[field.size()-1]) <= ' ')
                field.erase(field.size() - 1);
        }
        out.push_back(AnsiString(field));
        // On a delimiter, step past it and continue. The outer loop guard is
        // `i <= n`, so a TRAILING delimiter (e.g. "a,b,") re-enters the loop
        // once more with i==n and emits the single trailing empty field -- this
        // matches BCB6 CommaText, which turns "a,b," into 3 strings {a,b,""}.
        // (Do NOT push the trailing empty here as well, or it would be counted
        //  twice and every CSV row ending in ',' would gain a phantom column.)
        if (i < n && s[i] == delim) { ++i; continue; }
        break;
    }
    // BCB6: an empty source yields an empty list.
    if (s.empty()) out.clear();
}

AnsiString TStringList::GetCommaText() const { return buildDelimited(items_, ',', '"'); }

void TStringList::SetCommaText(const AnsiString& v) {
    objects_.clear();
    std::vector<AnsiString> parsed;
    parseDelimited(v.str(), ',', '"', parsed);
    items_.swap(parsed);
    objects_.assign(items_.size(), static_cast<TObject*>(0));
    syncCount();
}

AnsiString TStringList::GetDelimitedText() const { return buildDelimited(items_, Delimiter, QuoteChar); }

void TStringList::SetDelimitedText(const AnsiString& v) {
    objects_.clear();
    std::vector<AnsiString> parsed;
    parseDelimited(v.str(), Delimiter, QuoteChar, parsed);
    items_.swap(parsed);
    objects_.assign(items_.size(), static_cast<TObject*>(0));
    syncCount();
}

// ---------------------------------------------------------------------------
//  File I/O
// ---------------------------------------------------------------------------
void TStringList::LoadFromFile(const AnsiString& path) {
    std::ifstream in(path.c_str(), std::ios::binary);
    items_.clear();
    objects_.clear();
    if (!in) { syncCount(); return; }
    std::ostringstream ss;
    ss << in.rdbuf();
    SetText(AnsiString(ss.str()));
}

void TStringList::SaveToFile(const AnsiString& path) const {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out) return;
    for (size_t i = 0; i < items_.size(); ++i)
        out << items_[i].str() << "\r\n";   // BCB6 writes CRLF per line
}

void TStringList::Assign(const TStringList* src) {
    if (!src || src == this) return;
    items_   = src->items_;
    objects_ = src->objects_;
    syncCount();
}

// ===========================================================================
//  Proxy implementations
// ===========================================================================
StringsProxy::operator AnsiString() const { return owner_->GetString(idx_); }
StringsProxy& StringsProxy::operator=(const AnsiString& v) { owner_->SetString(idx_, v); return *this; }
StringsProxy& StringsProxy::operator=(const char* v)       { owner_->SetString(idx_, AnsiString(v)); return *this; }

ObjectsProxy::operator TObject*() const { return owner_->GetObject(idx_); }
ObjectsProxy& ObjectsProxy::operator=(TObject* v) { owner_->SetObject(idx_, v); return *this; }

TextProxy::operator AnsiString() const { return owner_->GetText(); }
TextProxy& TextProxy::operator=(const AnsiString& v) { owner_->SetText(v); return *this; }
TextProxy& TextProxy::operator=(const char* v)       { owner_->SetText(AnsiString(v)); return *this; }

CommaTextProxy::operator AnsiString() const { return owner_->GetCommaText(); }
CommaTextProxy& CommaTextProxy::operator=(const AnsiString& v) { owner_->SetCommaText(v); return *this; }
CommaTextProxy& CommaTextProxy::operator=(const char* v)       { owner_->SetCommaText(AnsiString(v)); return *this; }

DelimitedTextProxy::operator AnsiString() const { return owner_->GetDelimitedText(); }
DelimitedTextProxy& DelimitedTextProxy::operator=(const AnsiString& v) { owner_->SetDelimitedText(v); return *this; }
DelimitedTextProxy& DelimitedTextProxy::operator=(const char* v)       { owner_->SetDelimitedText(AnsiString(v)); return *this; }

} // namespace vclcompat
