// ===========================================================================
//  vclcompat/TStringList.cpp -- BCB6 TStringList implementation (used subset).
// ===========================================================================
#include "vclcompat/TStringList.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
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
//  CommaText / DelimitedText -- the BCB6 VCL algorithms, ported line by line.
//
//  AI(W906-COMMATEXT) 20261003: this used to be a home-grown "CSV-ish" reader/
//  writer that differed from BCB6 in six ways (INBOX 152).  The authority now is
//  the VCL source shipped with BCB6 (D:\ProgramFiles\Borland\CBuilder6\Source\vcl):
//      Classes.pas   TStrings.GetCommaText     :4067-4085
//                    TStrings.GetDelimitedText :4087-4114
//                    TStrings.SetCommaText     :4304-4309
//                    TStrings.SetDelimitedText :4375-4434
//      SysUtils.pas  AnsiQuotedStr             :3861-3898
//                    AnsiExtractQuotedStr      :3900-3941
//  and the behaviour is pinned by a REAL BCB6 run: tests/oracle/commatext_bcb6.cpp
//  (bcc32 5.6.4 + rtl.lib) -> tests/oracle/commatext_bcb6_expected.txt, compared by
//  ctest VclCommaTextBcb6.  What changed against the old code, all per that source:
//    1. an unquoted item ends at ANY char <= ' ' (not only at the delimiter):
//       "07 Tester I/F" is THREE items; blanks after an item are skipped, then one
//       optional delimiter, then blanks again;
//    2. a delimiter adds a trailing empty item only when it is the very LAST char:
//       "a,b," -> {a,b,""} but "a, " -> {a};  "a,,b" -> {a,"",b};
//    3. text right after a closing quote starts the next item: "\"ab\"cd,e" -> {ab,cd,e};
//    4. AnsiExtractQuotedStr's quirks: an unterminated quote drops the LAST char
//       ("\"abc" -> "ab"), "\"" / "\"a" -> "";
//    5. the writer quotes an item only if its scan meets #0..' ', QuoteChar or
//       Delimiter before the NUL, so an EMPTY item among several is written bare
//       ("a,,b", was "a,\"\",b"); one empty item alone is QuoteChar+QuoteChar;
//    6. SetCommaText assigns Delimiter := ',' and QuoteChar := '"' for good.
//
//  CharNext.  The RTL steps with Win32 CharNext (an MBCS walk in the ANSI code page)
//  and finds the quote with AnsiStrScan.  Here both step BYTES on purpose: the port's
//  runtime strings are UTF-8, where every byte of a multi-byte character is >= 0x80 --
//  never a blank, a NUL, ',' or '"' (or any other ASCII delimiter) -- so a byte walk over
//  UTF-8 finds exactly the item boundaries CharNext finds over the Big5 original.
//  Calling the real ::CharNextA would be WRONG on UTF-8 under ACP 950: it takes any byte
//  0x81..0xFE for a Big5 lead byte and swallows the next byte, even a ','.  (Only raw
//  Big5 bytes with a delimiter in 0x40..0x7E, e.g. '|', would differ -- oracle case V01.)
//
//  PChar semantics are kept: every walk runs over value.c_str(), so an embedded NUL
//  ends the parse exactly like the Pascal `while P^ <> #0`.
// ---------------------------------------------------------------------------
namespace {

// Win32 CharNext, byte-stepping (see above).  At the NUL it does not move -- CharNextA's
// documented behaviour, which SetDelimitedText's `CharNext(P1)^ = #0` relies on.
inline const char* CtCharNext(const char* p) { return *p ? p + 1 : p; }

// Pascal `P^ in [#1..' ']`
inline bool CtIsBlank1(char c) { return c != '\0' && static_cast<unsigned char>(c) <= ' '; }

// SysUtils.StrScan / AnsiStrScan (sysutils.pas:5661 / :11230) for the quote char: the
// first Quote before the NUL, or nil.  Quote = #0 would make the RTL step past the
// terminator (undefined); here it simply finds nothing.
inline const char* CtScan(const char* s, char quote) { return quote ? std::strchr(s, quote) : nullptr; }

// SysUtils.StrEnd
inline const char* CtStrEnd(const char* s) { return s + std::strlen(s); }

// System.Move(Src^, Dest^, Count) into a string sized by SetLength: Count <= 0 moves
// nothing (the RTL's SAR/JS exit); bytes that would land past Length(Result) are dropped.
// The RTL writes them anyway -- over the string's NUL terminator, or, when Length is 0,
// through PChar('') = System's read-only @@zeroByte, i.e. EAccessViolation (oracle R01/R02).
inline void CtMove(std::string& dst, size_t at, const char* src, std::ptrdiff_t count)
{
    for (std::ptrdiff_t k = 0; k < count; ++k)
        if (at + static_cast<size_t>(k) < dst.size()) dst[at + static_cast<size_t>(k)] = src[k];
}

// SysUtils.AnsiExtractQuotedStr(var Src: PChar; Quote: Char): string  (sysutils.pas:3900-3941)
std::string CtExtractQuoted(const char*& Src, char Quote)
{
    std::string Result;                                         // Result := '';
    if (Src == nullptr || *Src != Quote) return Result;         // if (Src = nil) or (Src^ <> Quote) then Exit;
    ++Src;                                                      // Inc(Src);
    int DropCount = 1;                                          // DropCount := 1;
    const char* P = Src;                                        // P := Src;
    Src = CtScan(Src, Quote);                                   // Src := AnsiStrScan(Src, Quote);
    while (Src != nullptr) {                                    // while Src <> nil do   // count adjacent pairs
        ++Src;                                                  //   Inc(Src);
        if (*Src != Quote) break;                               //   if Src^ <> Quote then Break;
        ++Src;                                                  //   Inc(Src);
        ++DropCount;                                            //   Inc(DropCount);
        Src = CtScan(Src, Quote);                               //   Src := AnsiStrScan(Src, Quote);
    }
    if (Src == nullptr) Src = CtStrEnd(P);                      // if Src = nil then Src := StrEnd(P);
    if ((Src - P) <= 1) return Result;                          // if ((Src - P) <= 1) then Exit;
    if (DropCount == 1) {
        Result.assign(P, static_cast<size_t>(Src - P - 1));     // SetString(Result, P, Src - P - 1)
    } else {
        // SetLength(Result, Src - P - DropCount).  With no closing quote the last char is
        // lost: the moves below copy one byte more than this length (see CtMove).
        const std::ptrdiff_t len = (Src - P) - DropCount;
        Result.assign(static_cast<size_t>(len > 0 ? len : 0), '\0');
        size_t Dest = 0;                                        // Dest := PChar(Result);
        Src = CtScan(P, Quote);                                 // Src := AnsiStrScan(P, Quote);
        while (Src != nullptr) {                                // while Src <> nil do
            ++Src;                                              //   Inc(Src);
            if (*Src != Quote) break;                           //   if Src^ <> Quote then Break;
            CtMove(Result, Dest, P, Src - P);                   //   Move(P^, Dest^, Src - P);
            Dest += static_cast<size_t>(Src - P);               //   Inc(Dest, Src - P);
            ++Src;                                              //   Inc(Src);
            P = Src;                                            //   P := Src;
            Src = CtScan(Src, Quote);                           //   Src := AnsiStrScan(Src, Quote);
        }
        if (Src == nullptr) Src = CtStrEnd(P);                  // if Src = nil then Src := StrEnd(P);
        CtMove(Result, Dest, P, Src - P - 1);                   // Move(P^, Dest^, Src - P - 1);
    }
    return Result;
}

// SysUtils.AnsiQuotedStr(const S: string; Quote: Char): string  (sysutils.pas:3861-3898).
// The quote count and the copy stop at the first NUL (PChar scans); with no quote before
// it the whole S, NULs included, is wrapped.  With quotes AND a NUL the RTL leaves the
// tail past the NUL uninitialised (SetLength without a copy); it is zero here.
std::string CtQuoted(const std::string& S, char Quote)
{
    int AddCount = 0;                                           // AddCount := 0;
    const char* P = CtScan(S.c_str(), Quote);                   // P := AnsiStrScan(PChar(S), Quote);
    while (P != nullptr) {                                      // while P <> nil do
        ++P;                                                    //   Inc(P);
        ++AddCount;                                             //   Inc(AddCount);
        P = CtScan(P, Quote);                                   //   P := AnsiStrScan(P, Quote);
    }
    if (AddCount == 0)                                          // if AddCount = 0 then
        return std::string(1, Quote) + S + std::string(1, Quote); //   Result := Quote + S + Quote; Exit;
    std::string Result(S.size() + static_cast<size_t>(AddCount) + 2, '\0');   // SetLength(Result, Length(S) + AddCount + 2);
    size_t Dest = 0;                                            // Dest := Pointer(Result);
    Result[Dest++] = Quote;                                     // Dest^ := Quote; Inc(Dest);
    const char* Src = S.c_str();                                // Src := Pointer(S);
    P = CtScan(Src, Quote);                                     // P := AnsiStrScan(Src, Quote);
    do {                                                        // repeat
        ++P;                                                    //   Inc(P);
        CtMove(Result, Dest, Src, P - Src);                     //   Move(Src^, Dest^, P - Src);
        Dest += static_cast<size_t>(P - Src);                   //   Inc(Dest, P - Src);
        Result[Dest++] = Quote;                                 //   Dest^ := Quote; Inc(Dest);
        Src = P;                                                //   Src := P;
        P = CtScan(Src, Quote);                                 //   P := AnsiStrScan(Src, Quote);
    } while (P != nullptr);                                     // until P = nil;
    P = CtStrEnd(Src);                                          // P := StrEnd(Src);
    CtMove(Result, Dest, Src, P - Src);                         // Move(Src^, Dest^, P - Src);
    Dest += static_cast<size_t>(P - Src);                       // Inc(Dest, P - Src);
    Result[Dest] = Quote;                                       // Dest^ := Quote;
    return Result;
}

// TStrings.GetDelimitedText (classes.pas:4087-4114)
std::string CtGetDelimited(const std::vector<AnsiString>& items, char Delimiter, char QuoteChar)
{
    const size_t Count = items.size();                          // Count := GetCount;
    if (Count == 1 && items[0].str().empty())                   // if (Count = 1) and (Get(0) = '') then
        return std::string(2, QuoteChar);                       //   Result := QuoteChar + QuoteChar
    std::string Result;                                         // Result := '';
    for (size_t I = 0; I < Count; ++I) {                        // for I := 0 to Count - 1 do
        std::string S = items[I].str();                         //   S := Get(I);
        const char* P = S.c_str();                              //   P := PChar(S);
        while (!(static_cast<unsigned char>(*P) <= ' ' || *P == QuoteChar || *P == Delimiter))
            P = CtCharNext(P);                                  //   while not (P^ in [#0..' ', QuoteChar, Delimiter]) do P := CharNext(P);
        if (*P != '\0') S = CtQuoted(S, QuoteChar);             //   if (P^ <> #0) then S := AnsiQuotedStr(S, QuoteChar);
        Result += S;                                            //   Result := Result + S + Delimiter;
        Result += Delimiter;
    }
    if (!Result.empty()) Result.erase(Result.size() - 1);       // System.Delete(Result, Length(Result), 1);
    return Result;
}

// TStrings.SetDelimitedText (classes.pas:4375-4434), minus the Clear / Add / BeginUpdate
// calls, which the caller makes (see TStringList::SetDelimitedText).
void CtSetDelimited(const std::string& Value, char Delimiter, char QuoteChar, std::vector<std::string>& out)
{
    const char* P = Value.c_str();                              // P := PChar(Value);
    while (CtIsBlank1(*P)) P = CtCharNext(P);                   // while P^ in [#1..' '] do P := CharNext(P);
    while (*P != '\0') {                                        // while P^ <> #0 do
        std::string S;
        if (*P == QuoteChar) {                                  //   if P^ = QuoteChar then
            S = CtExtractQuoted(P, QuoteChar);                  //     S := AnsiExtractQuotedStr(P, QuoteChar)
        } else {                                                //   else
            const char* P1 = P;                                 //     P1 := P;
            while (static_cast<unsigned char>(*P) > ' ' && *P != Delimiter)
                P = CtCharNext(P);                              //     while (P^ > ' ') and (P^ <> Delimiter) do P := CharNext(P);
            S.assign(P1, static_cast<size_t>(P - P1));          //     SetString(S, P1, P - P1);
        }
        out.push_back(S);                                       //   Add(S);
        while (CtIsBlank1(*P)) P = CtCharNext(P);               //   while P^ in [#1..' '] do P := CharNext(P);
        if (*P == Delimiter) {                                  //   if P^ = Delimiter then
            const char* P1 = P;                                 //     P1 := P;
            if (*CtCharNext(P1) == '\0')                        //     if CharNext(P1)^ = #0 then
                out.push_back(std::string());                   //       Add('');
            do {                                                //     repeat
                P = CtCharNext(P);                              //       P := CharNext(P);
            } while (CtIsBlank1(*P));                           //     until not (P^ in [#1..' ']);
        }
    }
}

} // namespace

// TStrings.GetCommaText (classes.pas:4067-4085): Delimiter := ','; QuoteChar := '"';
// GetDelimitedText; then FDelimiter / FQuoteChar / FDefined restored -- no net change.
AnsiString TStringList::GetCommaText() const { return AnsiString(CtGetDelimited(items_, ',', '"')); }

// TStrings.SetCommaText (classes.pas:4304-4309) -- the two assignments stay in effect
// after the call (a later DelimitedText uses ',' and '"').
void TStringList::SetCommaText(const AnsiString& v) {
    Delimiter = ',';
    QuoteChar = '"';
    SetDelimitedText(v);
}

AnsiString TStringList::GetDelimitedText() const { return AnsiString(CtGetDelimited(items_, Delimiter, QuoteChar)); }

// TStrings.SetDelimitedText: BeginUpdate; Clear; ... Add(S) per item ...; EndUpdate.
// vclcompat models no OnChange / Sorted / Duplicates, so Clear + Add reduce to "replace
// the items, every object nil".  Add is the virtual call, as in VCL.  Clear is qualified
// (non-virtual) on purpose: TRadioGroupItems (Controls.h) overrides Clear() to imitate
// VCL's ItemsChange clamp, which VCL runs at EndUpdate against the FINAL Count --
// dispatching to that override here, before the Adds, would clamp ItemIndex against an
// empty list.
void TStringList::SetDelimitedText(const AnsiString& v) {
    std::vector<std::string> parsed;
    CtSetDelimited(v.str(), Delimiter, QuoteChar, parsed);
    TStringList::Clear();
    for (size_t i = 0; i < parsed.size(); ++i)
        Add(AnsiString(parsed[i]));
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
