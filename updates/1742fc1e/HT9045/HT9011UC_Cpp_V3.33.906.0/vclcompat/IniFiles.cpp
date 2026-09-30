// ===========================================================================
//  vclcompat/IniFiles.cpp  -- implementation of the BCB6 TIniFile / TMemIniFile
//  shims.  Semantics mirror BCB6 exactly; see IniFiles.h for the contract.
// ===========================================================================
#include "vclcompat/IniFiles.h"
#include "vclcompat/SysUtils.h"   // FileExists

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>

namespace vclcompat {

// ===========================================================================
//  Local parsing helpers -- BCB6-faithful, locale-independent.
// ===========================================================================
namespace {

// BCB6 StrToIntDef-like: decimal, or Pascal '$' / C '0x' hex prefix; a leading
// sign is tolerated; the WHOLE trimmed string must parse or we return def.
// (TIniFile::ReadInteger == StrToIntDef(ReadString(...), Default) in BCB6.)
int parseIntDef(const std::string& raw, int def) {
    // trim (chars <= ' ', matching AnsiString::Trim)
    size_t b = 0, e = raw.size();
    while (b < e && static_cast<unsigned char>(raw[b]) <= ' ') ++b;
    while (e > b && static_cast<unsigned char>(raw[e - 1]) <= ' ') --e;
    if (b >= e) return def;
    std::string s = raw.substr(b, e - b);

    const char* p = s.c_str();
    char* end = 0;
    long v;
    if (s.size() >= 1 && s[0] == '$') {                 // Pascal hex literal
        v = std::strtol(p + 1, &end, 16);
        if (end == p + 1 || *end != '\0') return def;
    } else if (s.size() >= 2 && s[0] == '0' &&
               (s[1] == 'x' || s[1] == 'X')) {          // C hex literal
        v = std::strtol(p, &end, 16);
        if (end == p || *end != '\0') return def;
    } else {                                            // decimal
        v = std::strtol(p, &end, 10);
        if (end == p || *end != '\0') return def;
    }
    return static_cast<int>(v);
}

// Locale-independent decimal parse ('.' separator).  Whole trimmed string must
// parse or we return def.  (TIniFile::ReadFloat semantics.)
double parseFloatDef(const std::string& raw, double def) {
    size_t b = 0, e = raw.size();
    while (b < e && static_cast<unsigned char>(raw[b]) <= ' ') ++b;
    while (e > b && static_cast<unsigned char>(raw[e - 1]) <= ' ') --e;
    if (b >= e) return def;
    std::string s = raw.substr(b, e - b);
    const char* p = s.c_str();
    char* end = 0;
    double v = std::strtod(p, &end);
    if (end == p || *end != '\0') return def;
    return v;
}

} // anonymous namespace

// ===========================================================================
//  TIniStore
// ===========================================================================

// ASCII case-insensitive byte compare; bytes >= 0x80 (Big5 lead/trail) compared
// verbatim so multibyte section/value names round-trip without corruption.
bool TIniStore::iequal(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        unsigned char ca = static_cast<unsigned char>(a[i]);
        unsigned char cb = static_cast<unsigned char>(b[i]);
        if (ca < 0x80) ca = static_cast<unsigned char>(std::toupper(ca));
        if (cb < 0x80) cb = static_cast<unsigned char>(std::toupper(cb));
        if (ca != cb) return false;
    }
    return true;
}

TIniStore::Section* TIniStore::findSection(const AnsiString& name) {
    for (size_t i = 0; i < sections_.size(); ++i)
        if (iequal(sections_[i].name, name.str())) return &sections_[i];
    return 0;
}

const TIniStore::Section* TIniStore::findSection(const AnsiString& name) const {
    for (size_t i = 0; i < sections_.size(); ++i)
        if (iequal(sections_[i].name, name.str())) return &sections_[i];
    return 0;
}

bool TIniStore::SectionExists(const AnsiString& section) const {
    return findSection(section) != 0;
}

bool TIniStore::ValueExists(const AnsiString& section, const AnsiString& ident) const {
    const Section* s = findSection(section);
    if (!s) return false;
    for (size_t i = 0; i < s->items.size(); ++i)
        if (iequal(s->items[i].key, ident.str())) return true;
    return false;
}

AnsiString TIniStore::ReadRaw(const AnsiString& section, const AnsiString& ident,
                              bool& found) const {
    found = false;
    const Section* s = findSection(section);
    if (!s) return AnsiString();
    for (size_t i = 0; i < s->items.size(); ++i) {
        if (iequal(s->items[i].key, ident.str())) {
            found = true;
            return AnsiString(s->items[i].val);
        }
    }
    return AnsiString();
}

void TIniStore::WriteRaw(const AnsiString& section, const AnsiString& ident,
                         const AnsiString& value) {
    Section* s = findSection(section);
    if (!s) {
        Section ns;
        ns.name = section.str();
        sections_.push_back(ns);
        s = &sections_.back();
    }
    for (size_t i = 0; i < s->items.size(); ++i) {
        if (iequal(s->items[i].key, ident.str())) {
            s->items[i].val = value.str();
            return;
        }
    }
    KeyVal kv;
    kv.key = ident.str();
    kv.val = value.str();
    s->items.push_back(kv);
}

void TIniStore::FillSectionKeys(const AnsiString& section, TStrings* dest) const {
    if (!dest) return;
    dest->Clear();
    const Section* s = findSection(section);
    if (!s) return;
    for (size_t i = 0; i < s->items.size(); ++i)
        dest->Add(AnsiString(s->items[i].key));
}

void TIniStore::FillSectionNames(TStrings* dest) const {
    if (!dest) return;
    dest->Clear();
    for (size_t i = 0; i < sections_.size(); ++i)
        dest->Add(AnsiString(sections_[i].name));
}

// ---------------------------------------------------------------------------
//  Disk I/O.  INI grammar (BCB6 / Win32 profile):
//    * '[name]' on its own line opens a section (trailing ']' may be absent;
//      we take up to the last ']' or end of line, then trim).
//    * 'key=value' adds a key under the current section. The FIRST '=' splits;
//      key is trimmed of surrounding blanks; value keeps everything after '='
//      VERBATIM (no trim) -- BCB6 preserves trailing/internal value spaces, and
//      this family round-trips '%0.4f' / Big5 text that must not be mangled.
//    * Keys before any [section] are dropped (no global section in this family).
//    * ';' / '#' comment lines and blank lines are skipped. (BCB6 only treats
//      ';' as a comment; we also skip '#'-lines defensively. The real config
//      files in this project use neither inside the keys this family reads.)
//    * Line endings: CR, LF, or CRLF all accepted (real files are CRLF).
// ---------------------------------------------------------------------------
void TIniStore::LoadFromFile(const AnsiString& path) {
    sections_.clear();

    std::FILE* fp = std::fopen(path.c_str(), "rb");
    if (!fp) return;   // missing file => empty store (reads then fall to Default)

    std::string content;
    char buf[8192];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), fp)) > 0)
        content.append(buf, n);
    std::fclose(fp);

    Section* cur = 0;
    size_t i = 0, len = content.size();
    while (i < len) {
        // extract one raw line (without the EOL bytes)
        size_t start = i;
        while (i < len && content[i] != '\n' && content[i] != '\r') ++i;
        std::string line = content.substr(start, i - start);
        // consume EOL (handle CRLF as one)
        if (i < len && content[i] == '\r') { ++i; if (i < len && content[i] == '\n') ++i; }
        else if (i < len && content[i] == '\n') { ++i; }

        // left-trim for structural detection (value bytes handled separately)
        size_t b = 0;
        while (b < line.size() && static_cast<unsigned char>(line[b]) <= ' ') ++b;
        if (b >= line.size()) continue;                 // blank
        char c0 = line[b];
        if (c0 == ';' || c0 == '#') continue;           // comment

        if (c0 == '[') {
            size_t close = line.rfind(']');
            std::string name = (close != std::string::npos && close > b)
                               ? line.substr(b + 1, close - b - 1)
                               : line.substr(b + 1);
            // trim section name
            size_t sb = 0, se = name.size();
            while (sb < se && static_cast<unsigned char>(name[sb]) <= ' ') ++sb;
            while (se > sb && static_cast<unsigned char>(name[se - 1]) <= ' ') --se;
            Section ns;
            ns.name = name.substr(sb, se - sb);
            sections_.push_back(ns);
            cur = &sections_.back();
            continue;
        }

        // key=value (first '=' splits)
        size_t eq = line.find('=', b);
        if (eq == std::string::npos) continue;           // not a key line; skip
        if (!cur) continue;                              // key before any section

        std::string key = line.substr(b, eq - b);
        // trim key
        size_t kb = 0, ke = key.size();
        while (kb < ke && static_cast<unsigned char>(key[kb]) <= ' ') ++kb;
        while (ke > kb && static_cast<unsigned char>(key[ke - 1]) <= ' ') --ke;
        std::string k = key.substr(kb, ke - kb);
        if (k.empty()) continue;

        std::string v = line.substr(eq + 1);             // value: verbatim
        KeyVal kv; kv.key = k; kv.val = v;
        cur->items.push_back(kv);
    }
}

void TIniStore::SaveToFile(const AnsiString& path, bool blankLineAfterSection) const {
    std::FILE* fp = std::fopen(path.c_str(), "wb");
    if (!fp) return;
    for (size_t s = 0; s < sections_.size(); ++s) {
        std::fputc('[', fp);
        std::fwrite(sections_[s].name.data(), 1, sections_[s].name.size(), fp);
        std::fputs("]\r\n", fp);                          // CRLF (Win32 convention)
        for (size_t k = 0; k < sections_[s].items.size(); ++k) {
            const KeyVal& kv = sections_[s].items[k];
            std::fwrite(kv.key.data(), 1, kv.key.size(), fp);
            std::fputc('=', fp);
            std::fwrite(kv.val.data(), 1, kv.val.size(), fp);
            std::fputs("\r\n", fp);
        }   if (blankLineAfterSection) std::fputs("\r\n", fp);   // AI(W906-T4-INIFMT) 20260924: BCB6 TMemIniFile::GetStrings emits an empty line after EVERY section (incl. the last)
    }
    std::fclose(fp);
}
AnsiString W906_DiskIniReadRaw(const AnsiString&, const AnsiString&, const AnsiString&, bool&); AnsiString W906_Win32ProfileDefault(const AnsiString&); bool W906_DiskIniValueExists(const AnsiString&, const AnsiString&, const AnsiString&); bool W906_DiskIniSectionExists(const AnsiString&, const AnsiString&); void W906_DiskIniReadSection(const AnsiString&, const AnsiString&, TStrings*); void W906_DiskIniReadSections(const AnsiString&, TStrings*);   // AI(W906-A5-INIREAD) 20260924: write-through TIniFile reads the CURRENT file on every call like BCB6 TIniFile over Win32 GetPrivateProfileStringA -- defined at EOF
// ===========================================================================
//  TIniFile
// ===========================================================================
TIniFile::TIniFile(const AnsiString& fileName)
    : FileName(fileName), writeThrough_(true) {
    // Win32 TIniFile binds lazily, and so does this one: NOTHING is loaded here.   // AI(W906-A5-INIREAD) 20260924: was an eager store_.LoadFromFile(fileName) -- the image every read used to answer from
    // Every Read*/ValueExists/SectionExists/ReadSection(s) re-reads the file (IniFiles.cpp EOF); a missing file => defaults.
    // (store_ is read only by the TMemIniFile variant; WriteString still mirrors into it, which is harmless.)   //AI(W906-BOOTSPEED-2) 20260929: no longer true -- harmless for results but O(keys) per write; the write-through TIniFile now leaves store_ empty (WriteString)
}

// Tag ctor: TMemIniFile path -- eager full load, but NO write-through.
TIniFile::TIniFile(const AnsiString& fileName, MemTag)
    : FileName(fileName), writeThrough_(false) {
    store_.LoadFromFile(fileName);
}

TIniFile::~TIniFile() {
    // Base TIniFile is write-through; nothing buffered to flush here. (The Mem
    // subclass overrides destruction order via its own dtor -> UpdateFile.)
}

void TIniFile::flush() {
    store_.SaveToFile(FileName, !writeThrough_);   // AI(W906-T4-INIFMT) 20260924: Mem variant = BCB6 TMemIniFile layout, so a golden-written file round-trips byte-identical; write-through TIniFile unchanged
}

void TIniFile::UpdateFile() {
    // TIniFile: Win32 writes are immediate -> commit / no-op.  For the Mem
    // variant (writeThrough_==false) this is the ONLY disk flush.
    if (!writeThrough_)
        flush();
}

bool TIniFile::SectionExists(const AnsiString& section) {
    return writeThrough_ ? W906_DiskIniSectionExists(FileName, section) : store_.SectionExists(section);   // AI(W906-A5-INIREAD) 20260924: write-through = BCB6 TCustomIniFile.SectionExists over the CURRENT file (ReadSection(...).Count > 0 -> a key-less section does not exist)
}

bool TIniFile::ValueExists(const AnsiString& section, const AnsiString& ident) {
    return writeThrough_ ? W906_DiskIniValueExists(FileName, section, ident) : store_.ValueExists(section, ident);   // AI(W906-A5-INIREAD) 20260924: write-through = BCB6 TCustomIniFile.ValueExists over the CURRENT file (ReadSection + case-insensitive IndexOf(Ident))
}

AnsiString TIniFile::ReadString(const AnsiString& section, const AnsiString& ident,
                                const AnsiString& def) {
    bool found = false;
    AnsiString v = writeThrough_ ? W906_DiskIniReadRaw(FileName, section, ident, found) : store_.ReadRaw(section, ident, found);   // AI(W906-A5-INIREAD) 20260924: write-through reads the CURRENT file like GetPrivateProfileStringA (BCB6 2048-byte buffer)
    return found ? v : (writeThrough_ ? W906_Win32ProfileDefault(def) : def);   // AI(W906-A5-INIREAD) 20260924: Win32 hands back the default minus its trailing SPACES, cut to 2047 bytes
}

int TIniFile::ReadInteger(const AnsiString& section, const AnsiString& ident, int def) {
    bool found = false;
    AnsiString v = writeThrough_ ? W906_DiskIniReadRaw(FileName, section, ident, found) : store_.ReadRaw(section, ident, found);   // AI(W906-A5-INIREAD) 20260924: write-through reads the CURRENT file like GetPrivateProfileStringA (BCB6 2048-byte buffer)
    if (!found) return def;
    return parseIntDef(v.str(), def);
}

double TIniFile::ReadFloat(const AnsiString& section, const AnsiString& ident, double def) {
    bool found = false;
    AnsiString v = writeThrough_ ? W906_DiskIniReadRaw(FileName, section, ident, found) : store_.ReadRaw(section, ident, found);   // AI(W906-A5-INIREAD) 20260924: write-through reads the CURRENT file like GetPrivateProfileStringA (BCB6 2048-byte buffer)
    if (!found) return def;
    return parseFloatDef(v.str(), def);
}

bool TIniFile::ReadBool(const AnsiString& section, const AnsiString& ident, bool def) {
    bool found = false;
    AnsiString v = writeThrough_ ? W906_DiskIniReadRaw(FileName, section, ident, found) : store_.ReadRaw(section, ident, found);   // AI(W906-A5-INIREAD) 20260924: write-through reads the CURRENT file like GetPrivateProfileStringA (BCB6 2048-byte buffer)
    if (!found) return def;
    // BCB6 ReadBool == (ReadInteger(...) != 0).  Unparseable -> treat as the
    // integer default derived from `def` so the result equals `def`.
    int iv = parseIntDef(v.str(), def ? 1 : 0);
    return iv != 0;
}

TDateTime TIniFile::ReadDateTime(const AnsiString& section, const AnsiString& ident,
                                 const TDateTime& def) {
    bool found = false;
    AnsiString v = writeThrough_ ? W906_DiskIniReadRaw(FileName, section, ident, found) : store_.ReadRaw(section, ident, found);   // AI(W906-A5-INIREAD) 20260924: write-through reads the CURRENT file like GetPrivateProfileStringA (BCB6 2048-byte buffer)
    if (!found) return def;
    if (v.Trim().IsEmpty()) return def;
    return StrToDateTime(v);
}

void TIniFile::WriteString(const AnsiString& section, const AnsiString& ident,
                           const AnsiString& value) {
    if (!writeThrough_) store_.WriteRaw(section, ident, value);   //AI(W906-BOOTSPEED-2) 20260929: the in-memory mirror is only READ by the TMemIniFile variant (every write-through read/ValueExists/ReadSection goes to disk, flush() runs only when !writeThrough_) -- mirroring here grew a never-read copy of every key and made each write O(keys) (boot sample: TIniStore::iequal < WriteRaw ~10 of 46 s in W906_SecurityJamBoot, whose one INIFile collects ~23,000 keys of JAM0000.dat)
    if (writeThrough_) { void W906_Win32WriteProfileString(const AnsiString&, const AnsiString&, const AnsiString&, const AnsiString&); W906_Win32WriteProfileString(FileName, section, ident, value); }   // AI(W906-T4-INIFMT2) 20260924: in-place edit of the CURRENT disk file like Win32 WritePrivateProfileStringA (was flush() = rebuild the whole file from the image loaded at construction) -- see EOF
}

void TIniFile::WriteInteger(const AnsiString& section, const AnsiString& ident, int value) {
    WriteString(section, ident, AnsiString(value));     // AnsiString(int) -> decimal text
}

void TIniFile::WriteBool(const AnsiString& section, const AnsiString& ident, bool value) {
    WriteString(section, ident, AnsiString(value ? 1 : 0));  // '0'/'1'
}

void TIniFile::WriteDateTime(const AnsiString& section, const AnsiString& ident,
                             const TDateTime& value) {
    WriteString(section, ident, DateTimeToStr(value));
}

void TIniFile::ReadSection(const AnsiString& section, TStrings* dest) {
    if (writeThrough_) W906_DiskIniReadSection(FileName, section, dest); else store_.FillSectionKeys(section, dest);   // AI(W906-A5-INIREAD) 20260924: write-through = BCB6 TIniFile.ReadSection (16384-byte GetPrivateProfileStringA key list) over the CURRENT file
}

void TIniFile::ReadSections(TStrings* dest) {
    if (writeThrough_) W906_DiskIniReadSections(FileName, dest); else store_.FillSectionNames(dest);   // AI(W906-A5-INIREAD) 20260924: write-through = BCB6 TIniFile.ReadSections (16384-byte section list) over the CURRENT file
}

// ===========================================================================
//  TMemIniFile
// ===========================================================================
TMemIniFile::TMemIniFile(const AnsiString& fileName)
    : TIniFile(fileName, TIniFile::kMem) {   // eager full load, no write-through
}

TMemIniFile::~TMemIniFile() {
    // Safety-net flush: the BCB6 idiom always calls UpdateFile() before delete
    // (common.cpp CloseIniFileMem 362-363), but flush here too so an in-memory
    // mutation is never silently lost if a caller forgets.  Idempotent.
    flush();
}

} // namespace vclcompat

// =============================================================================
//  AI(W906-T4-INIFMT2) 20260924: write-through TIniFile == Win32 WritePrivateProfileStringA, byte for byte.
//
//  BCB6 TIniFile::WriteString is a thin call to WritePrivateProfileStringA. That API edits the file IN PLACE and
//  reads what is on disk NOW; the old flush() rebuilt the whole file from the store image taken at construction,
//  which (a) trimmed every key's indentation and dropped comments / duplicate sections, and (b) silently reverted
//  every on-disk change made by another writer since this object was built (docs/RULINGS_20260917.md, "two
//  independent writers"; tools/wb_serve.cpp ZEROARG note).
//
//  The rules below were MEASURED on this machine by calling the real kernel32 WritePrivateProfileStringA through
//  ctypes on scratch files (15,000 random files x writes, 0 mismatches; the only exclusions are tiny LF files that
//  Windows' IsTextUnicode heuristic mis-detects and rewrites as UTF-16 -- not reproduced, never seen on real files):
//    * a trailing whitespace-only line WITHOUT a line terminator is dropped first;
//    * section match: the first line whose trimmed text starts with '[' -- name = up to ']' (or end), trimmed,
//      ASCII case-insensitive; the FIRST matching section wins (duplicates untouched);
//    * key match inside the section: a line containing '=' whose first non-blank char is not ';' -- key = text
//      before the FIRST '=', trimmed, ASCII case-insensitive ('#' is NOT a comment for Win32);
//    * modify: keep everything up to and including that '=', replace the rest of the line with the value verbatim;
//    * add key: after the last key line of the section (or right after the header if none); EXCEPTION: when the
//      section is the last one and the file's last line is blank/whitespace, append at end of file instead;
//    * add section: "[name]\r\nkey=value\r\n" at end of file, no blank line before it;
//    * every line this function creates ends in CRLF, even in an LF file; the file always ends up ending in '\n'.
//  Section/key names are written as passed, trimmed. Case folding is ASCII-only (Big5 trail bytes 0x41-0x5A are
//  not folded here; the keys and section names in this project's files are ASCII).
// =============================================================================
#include <string>
#include <vector>
namespace vclcompat { int W906_IniProbe(const AnsiString& file); bool W906_IniTouchSame(const AnsiString& file); std::string W906_IniApplyFast(const std::string& buf, const std::string& secIn, const std::string& keyIn, const std::string& val);   //AI(W906-BOOTSPEED-2) 20260929: boot-speed helpers, defined at EOF
namespace {
struct W906IniLine { std::string c, t; };
bool W906IniWs(char ch) { return ch == ' ' || ch == '\t'; }
std::string W906IniTrim(const std::string& s)
{
    size_t b = 0, e = s.size();
    while (b < e && W906IniWs(s[b])) ++b;
    while (e > b && W906IniWs(s[e - 1])) --e;
    return s.substr(b, e - b);
}
std::string W906IniLower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = char(s[i] - 'A' + 'a');
    return s;
}
std::vector<W906IniLine> W906IniSplit(const std::string& buf)
{
    std::vector<W906IniLine> out;
    size_t i = 0, n = buf.size();
    while (i < n) {
        size_t j = i;
        while (j < n && buf[j] != '\r' && buf[j] != '\n') ++j;
        if (j >= n) { out.push_back(W906IniLine{buf.substr(i), std::string()}); break; }
        if (buf[j] == '\r' && j + 1 < n && buf[j + 1] == '\n') { out.push_back(W906IniLine{buf.substr(i, j - i), "\r\n"}); i = j + 2; }
        else { out.push_back(W906IniLine{buf.substr(i, j - i), std::string(1, buf[j])}); i = j + 1; }
    }
    return out;
}
bool W906IniSection(const std::string& c, std::string& name)
{
    std::string s = W906IniTrim(c);
    if (s.empty() || s[0] != '[') return false;
    std::string body = s.substr(1);
    size_t k = body.find(']');
    if (k != std::string::npos) body = body.substr(0, k);
    name = W906IniLower(W906IniTrim(body));
    return true;
}
bool W906IniKey(const std::string& c, std::string& key)
{
    size_t b = 0;
    while (b < c.size() && W906IniWs(c[b])) ++b;
    if (b < c.size() && c[b] == ';') return false;
    size_t k = c.find('=');
    if (k == std::string::npos) return false;
    key = W906IniLower(W906IniTrim(c.substr(0, k)));
    return true;
}
std::string W906IniJoin(const std::vector<W906IniLine>& L)
{
    std::string o;
    for (size_t i = 0; i < L.size(); ++i) { o += L[i].c; o += L[i].t; }
    return o;
}
std::string W906IniEnsureNl(const std::string& b) { return (b.empty() || b[b.size() - 1] == '\n') ? b : b + "\r\n"; }
std::string W906IniDropWsTail(const std::string& b)
{
    std::vector<W906IniLine> L = W906IniSplit(b);
    if (!L.empty() && L.back().t.empty() && W906IniTrim(L.back().c).empty())
        return b.substr(0, b.size() - L.back().c.size());
    return b;
}
std::string W906IniApply(std::string buf, const std::string& secIn, const std::string& keyIn, const std::string& val)
{
    buf = W906IniDropWsTail(buf);
    std::vector<W906IniLine> L = W906IniSplit(buf);
    const std::string osec = W906IniTrim(secIn), okey = W906IniTrim(keyIn);
    const std::string lsec = W906IniLower(osec), lkey = W906IniLower(okey);
    std::string nm;
    size_t si = L.size();
    for (size_t i = 0; i < L.size(); ++i)
        if (W906IniSection(L[i].c, nm) && nm == lsec) { si = i; break; }
    if (si == L.size())
        return W906IniEnsureNl(W906IniEnsureNl(buf) + "[" + osec + "]\r\n" + okey + "=" + val + "\r\n");
    size_t ei = L.size();
    for (size_t i = si + 1; i < L.size(); ++i)
        if (W906IniSection(L[i].c, nm)) { ei = i; break; }
    size_t lastKey = si;
    std::string kk;
    for (size_t i = si + 1; i < ei; ++i) {
        if (W906IniKey(L[i].c, kk)) {
            if (kk == lkey) {
                size_t eq = L[i].c.find('=');
                L[i].c = L[i].c.substr(0, eq + 1) + val;
                return W906IniEnsureNl(W906IniJoin(L));
            }
            lastKey = i;
        }
    }
    if (ei == L.size() && W906IniTrim(L.back().c).empty() && lastKey != L.size() - 1)
        return W906IniEnsureNl(W906IniEnsureNl(buf) + okey + "=" + val + "\r\n");
    if (L[lastKey].t.empty()) L[lastKey].t = "\r\n";
    L.insert(L.begin() + (lastKey + 1), W906IniLine{okey + "=" + val, "\r\n"});
    return W906IniEnsureNl(W906IniJoin(L));
}
} // namespace

// Exposed (not static) so the differential test can drive it with the exact bytes it fed to kernel32.
std::string W906_Win32ProfileApply(const std::string& buf, const std::string& section, const std::string& key,
                                   const std::string& value)
{
    return W906_IniApplyFast(buf, section, key, value);   //AI(W906-BOOTSPEED-2) 20260929: was W906IniApply -- same bytes, one pass, no per-line strings (EOF); the old engine stays as W906_Win32ProfileApplyRef for the differential test
}

void W906_Win32WriteProfileString(const AnsiString& file, const AnsiString& section, const AnsiString& ident,
                                  const AnsiString& value)
{
    std::string buf; const int probe = W906_IniProbe(file); bool had = false;   //AI(W906-BOOTSPEED-2) 20260929: 0 = open as before, 1 = the file is not there, 2 = its directory is not there (see EOF)
    if (std::FILE* fp = (probe == 0 ? std::fopen(file.c_str(), "rb") : 0)) { had = true;   //AI(W906-BOOTSPEED-2) 20260929: a file the OS reports missing is not opened -- that open fails the same way (was: fopen on a missing path)
        char tmp[65536];
        size_t n;
        while ((n = std::fread(tmp, 1, sizeof tmp, fp)) > 0) buf.append(tmp, n);
        std::fclose(fp);
    }
    const std::string out = W906_IniApplyFast(buf, section.c_str(), ident.c_str(), value.c_str());   if (had && out == buf && W906_IniTouchSame(file)) return;   //AI(W906-BOOTSPEED-2) 20260929: bytes on disk already == the result -> stamp LastWriteTime (what kernel32 does, measured) instead of truncate + rewrite; W906_IniApplyFast = W906IniApply's bytes in one pass (both at EOF)
    if (std::FILE* fp = (probe == 2 ? 0 : std::fopen(file.c_str(), "wb"))) {   //AI(W906-BOOTSPEED-2) 20260929: no directory -> "wb" cannot create the file either (was: fopen on a missing directory)
        std::fwrite(out.data(), 1, out.size(), fp);
        std::fclose(fp);
    }
}
} // namespace vclcompat

// =============================================================================
//  AI(W906-A5-INIREAD) 20260924: write-through TIniFile READS == BCB6 TIniFile over Win32 GetPrivateProfileStringA.
//
//  User ruling 20260924 A5 ("按照舊版作法"): BCB6 TIniFile::ReadString is a thin call to GetPrivateProfileStringA,
//  which reads the file ON DISK at every call, so an edit made by another writer (the browser) is seen by the very next
//  read. The port used to answer every read from the image loaded when the object was constructed, and INIFileGeneral /
//  INIFile (common.cpp OpenGeneralIniFile / OpenIniFile) live for the whole run -> a browser edit reached C++ only after
//  a restart. Now every write-through read re-reads the file. There is deliberately NO cache: a same-second, same-size
//  edit must be seen, and the cost was measured acceptable by the main loop (~5,400 reads at boot, 0 while idle, ~790
//  from START to PAUSE). TMemIniFile is untouched (read once at construction = golden TMemIniFile semantics).
//
//  Rules MEASURED on this machine against the real kernel32 through ctypes (3,300 random files: 369,600 reads + 29,700
//  enumerations, 0 differences; resident ctest IniFiles_Win32ReadDiff re-checks them from C++):
//    * a line ends at CRLF, LF or a bare CR;
//    * "trim" strips every byte 0x01..0x20 at both ends (0xA0 is NOT blank);
//    * section header: a line whose trimmed text starts with '['; name = text up to the FIRST ']' (or end), trimmed;
//      matching is ASCII case-insensitive byte by byte (a Big5 trail byte 0x41-0x5A folds too -- measured);
//    * only the FIRST matching section is searched; a duplicate [section] later in the file is invisible to reads;
//    * key line: trimmed text contains '=' and does not start with ';' ('#' is NOT a comment); key = text before the
//      FIRST '=', trimmed; a line without '=' is not a key; lines before any header belong to no section;
//    * duplicate key: the first one wins (the key enumeration lists both);
//    * value = text after that '=', trimmed, then ONE pair of matching quotes ("..." or '...') removed when the value is
//      at least 2 bytes long; an inline ';' is NOT a comment;
//    * the section and key being looked up are trimmed first;
//    * not found: the default comes back minus its trailing SPACES (0x20 only; a tab stays);
//    * the result is cut to nSize-1 bytes (BCB6 ReadString passes a 2048-byte buffer -> at most 2047);
//    * enumeration (key == NULL / section == NULL): every name followed by a NUL; unless that list is at most nSize-2
//      bytes it is cut to nSize-2 bytes (the last name may be partial; a double-byte character the cut would split
//      comes back as a NUL) and nSize-2 is returned (small-nSize sweeps: 5,300 files / 26,500 enumerations, 1,500 of
//      those files with Big5 names, 0 differences). The section list has EVERY header, duplicates included; the key
//      list is the first matching section's keys, empty and duplicate names included.
//  BCB6 layer on top (Delphi 6 IniFiles.pas, transliterated FROM MEMORY -- the BCB6 VCL source is not on this laptop):
//    * TIniFile.ReadSection / ReadSections: 16384-byte buffer, names taken with `while P^ <> #0` -> the list stops at
//      the first EMPTY name ("=5" as a section's first key line hides every key of that section);
//    * TCustomIniFile.ValueExists   = ReadSection, then TStringList.IndexOf(Ident) (case-insensitive, Ident NOT trimmed);
//    * TCustomIniFile.SectionExists = ReadSection(...).Count > 0 -> a section with no keys does NOT exist;
//    * ReadInteger / ReadFloat / ReadBool / ReadDateTime parse ReadString(Section, Ident, '') -- the parsers above are
//      unchanged. GetPrivateProfileIntA is NOT on that path; measured, it disagrees ("abc" -> 0, "12abc" -> 12, "$1F" -> 0).
//  Real files checked against the two buffer caps (read-only census of D:\HT9045\system, config, IniData, CFG; 1,562
//  files): biggest key list 14,782 bytes (IniData/Data/*/HandlerCondition.Data [laser]) < 16,384; longest value 1,791
//  bytes < 2,047 -- so on today's files the caps change nothing, but they are what golden would do past them.
// =============================================================================
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif
namespace vclcompat {
namespace {
struct W906RdSpan { const char* p; size_t n; };
// The scanners below use raw pointers and open-coded tests on purpose: the tree builds with no -O flag (empty
// CMAKE_BUILD_TYPE), where every std::string::operator[] / inline helper per byte is a real call -- measured 924 us
// vs a 57 KB file for a full scan before, and every read now pays one scan.
W906RdSpan W906RdTrim(const char* p, size_t n)
{
    const char* e = p + n;
    while (p < e && (unsigned char)(*p - 1) < 0x20u) ++p;              // 0x01..0x20
    while (e > p && (unsigned char)(e[-1] - 1) < 0x20u) --e;
    W906RdSpan s = {p, (size_t)(e - p)};
    return s;
}
bool W906RdSame(const W906RdSpan& a, const W906RdSpan& b)
{
    if (a.n != b.n) return false;
    for (size_t i = 0; i < a.n; ++i) {
        unsigned char x = (unsigned char)a.p[i], y = (unsigned char)b.p[i];
        if (x >= 'A' && x <= 'Z') x = (unsigned char)(x + 32);
        if (y >= 'A' && y <= 'Z') y = (unsigned char)(y + 32);
        if (x != y) return false;
    }
    return true;
}
// Next physical line of buf at *pos (terminator excluded); *pos moves past CRLF, LF or a bare CR.
bool W906RdNextLine(const std::string& buf, size_t* pos, W906RdSpan* line)
{
    const char* const b = buf.data();
    const char* const e = b + buf.size();
    const char* p = b + *pos;
    if (p >= e) return false;
    line->p = p;
    while (p < e && *p != '\r' && *p != '\n') ++p;
    line->n = (size_t)(p - line->p);
    if (p < e && *p == '\r' && p + 1 < e && p[1] == '\n') p += 2;
    else if (p < e) p += 1;
    *pos = (size_t)(p - b);
    return true;
}
bool W906RdHeader(const W906RdSpan& line, W906RdSpan* name)
{
    const W906RdSpan t = W906RdTrim(line.p, line.n);
    if (t.n == 0 || t.p[0] != '[') return false;
    const char* body = t.p + 1;
    size_t bn = t.n - 1;
    if (const void* close = std::memchr(body, ']', bn)) bn = (size_t)((const char*)close - body);
    *name = W906RdTrim(body, bn);
    return true;
}
bool W906RdKeyLine(const W906RdSpan& line, W906RdSpan* key, W906RdSpan* val)
{
    const W906RdSpan t = W906RdTrim(line.p, line.n);
    if (t.n == 0 || t.p[0] == ';') return false;
    const char* eq = (const char*)std::memchr(t.p, '=', t.n);
    if (!eq) return false;
    *key = W906RdTrim(t.p, (size_t)(eq - t.p));
    *val = W906RdTrim(eq + 1, (size_t)(t.p + t.n - eq - 1));
    return true;
}
// Offset just past the header line of the FIRST section named `sec`; npos when there is none.
size_t W906RdFindSection(const std::string& buf, const std::string& sec)
{
    const W906RdSpan want = W906RdTrim(sec.data(), sec.size());
    size_t pos = 0;
    W906RdSpan line, name;
    while (W906RdNextLine(buf, &pos, &line))
        if (W906RdHeader(line, &name) && W906RdSame(name, want)) return pos;
    return std::string::npos;
}
bool W906RdLookup(const std::string& buf, const std::string& sec, const std::string& key, std::string* value)
{
    size_t pos = W906RdFindSection(buf, sec);
    if (pos == std::string::npos) return false;
    const W906RdSpan want = W906RdTrim(key.data(), key.size());
    W906RdSpan line, name, k, v;
    while (W906RdNextLine(buf, &pos, &line)) {
        if (W906RdHeader(line, &name)) break;
        if (W906RdKeyLine(line, &k, &v) && W906RdSame(k, want)) {
            if (v.n >= 2 && v.p[0] == v.p[v.n - 1] && (v.p[0] == '"' || v.p[0] == '\'')) { ++v.p; v.n -= 2; }
            value->assign(v.p, v.n);
            return true;
        }
    }
    return false;
}
std::string W906RdGetString(const std::string& buf, const std::string& sec, const std::string& key,
                            const std::string& def, size_t size, bool* found)
{
    std::string v;
    const bool f = W906RdLookup(buf, sec, key, &v);
    if (!f) {
        v = def;
        while (!v.empty() && v[v.size() - 1] == ' ') v.erase(v.size() - 1);
    }
    if (size == 0) v.clear();
    else if (v.size() > size - 1) v.resize(size - 1);
    if (found) *found = f;
    return v;
}
// The first n bytes kernel32 leaves in the buffer for a NULL key (sec != 0) or NULL section (sec == 0).
std::string W906RdNames(const std::string& buf, const std::string* sec, size_t size)
{
    std::string list;
    size_t pos = 0;
    W906RdSpan line, name, k, v;
    if (sec) {
        pos = W906RdFindSection(buf, *sec);
        if (pos == std::string::npos) return std::string();
        while (W906RdNextLine(buf, &pos, &line)) {
            if (W906RdHeader(line, &name)) break;
            if (W906RdKeyLine(line, &k, &v)) { list.append(k.p, k.n); list += '\0'; }
        }
    } else {
        while (W906RdNextLine(buf, &pos, &line))
            if (W906RdHeader(line, &name)) { list.append(name.p, name.n); list += '\0'; }
    }
    // Measured: the list comes back whole only when list + 2 <= nSize (one byte more than "list + closing NUL" -- keys
    // aaa,bbb,ccc = 12 bytes: nSize 13 already returns 11, nSize 14 returns 12; same for section lists).
    if (list.size() + 2 <= size) return list;
    std::string cut = list.substr(0, size >= 2 ? size - 2 : 0);
#ifdef _WIN32
    // Measured (ACP 950): a double-byte character that the cut would split is not emitted -- its lead byte comes back as
    // NUL (sections A,<Big5 4 bytes> at nSize 5: "A\0\0", not "A\0\xa4"). Values / defaults are cut mid-character as is.
    for (size_t i = 0; i < cut.size(); ) {
        const bool pair = ::IsDBCSLeadByte((BYTE)cut[i]) && i + 1 < list.size() && list[i + 1] != '\0';
        if (pair && i + 1 == cut.size()) { cut[i] = '\0'; break; }
        i += pair ? 2 : 1;
    }
#endif
    return cut;
}
// BCB6 TIniFile.ReadSection(s): `P := Buffer; while P^ <> #0 do begin Strings.Add(P); Inc(P, StrLen(P) + 1); end`.
void W906RdSplitNames(const std::string& raw, std::vector<std::string>* out)
{
    size_t p = 0;
    while (p < raw.size() && raw[p] != '\0') {
        size_t e = raw.find('\0', p);
        if (e == std::string::npos) e = raw.size();
        out->push_back(raw.substr(p, e - p));
        p = e + 1;
    }
}
// The file as it is on disk NOW. Opened with FILE_SHARE_DELETE on Windows so that the browser's atomic replace
// (WebBridgeRecipeDoc.cpp MoveFileExA over the target) cannot fail because a read happens to hold the file open.
std::string W906RdSlurp(const AnsiString& file)
{
    std::string buf; if (W906_IniProbe(file) != 0) return buf;   //AI(W906-BOOTSPEED-2) 20260929: the OS reports the file (or its directory) missing -> the open below would fail the same way; skip its ~100-170 us under the on-access scanner (see EOF)
#ifdef _WIN32
    HANDLE h = ::CreateFileA(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return buf;
    LARGE_INTEGER sz;
    DWORD got = 0;
    if (::GetFileSizeEx(h, &sz) && sz.QuadPart > 0 && sz.QuadPart < 0x40000000) {   // one sized read (the common case)
        buf.resize((size_t)sz.QuadPart);
        if (!::ReadFile(h, &buf[0], (DWORD)buf.size(), &got, NULL)) got = 0;
        buf.resize(got);
    }
    char tmp[4096];                                    // whatever else is there (file grew / size unknown)
    while (::ReadFile(h, tmp, (DWORD)sizeof tmp, &got, NULL) && got > 0) buf.append(tmp, got);
    ::CloseHandle(h);
#else
    if (std::FILE* fp = std::fopen(file.c_str(), "rb")) {
        char tmp[65536];
        size_t n;
        while ((n = std::fread(tmp, 1, sizeof tmp, fp)) > 0) buf.append(tmp, n);
        std::fclose(fp);
    }
#endif
    return buf;
}
const size_t kW906BcbReadStringBuf = 2048;    // BCB6 TIniFile.ReadString: Buffer: array[0..2047] of Char
const size_t kW906BcbReadSectionBuf = 16384;  // BCB6 TIniFile.ReadSection / ReadSections: BufSize = 16384
void W906RdBcbSection(const AnsiString& file, const AnsiString& section, std::vector<std::string>* names)
{
    const std::string sec(section.c_str());
    W906RdSplitNames(W906RdNames(W906RdSlurp(file), &sec, kW906BcbReadSectionBuf), names);
}
} // namespace

// ---- pure-buffer primitives, exposed (not static) so IniFiles_Win32ReadDiff can hold them to kernel32 byte for byte ----
std::string W906_Win32ProfileGetString(const std::string& buf, const std::string& section, const std::string& key,
                                       const std::string& def, size_t size)
{
    return W906RdGetString(buf, section, key, def, size, 0);
}
std::string W906_Win32ProfileGetNames(const std::string& buf, const std::string* section, size_t size)
{
    return W906RdNames(buf, section, size);
}

// ---- what the write-through TIniFile calls (declared above TIniFile's constructor) ----
AnsiString W906_DiskIniReadRaw(const AnsiString& file, const AnsiString& section, const AnsiString& ident, bool& found)
{
    return AnsiString(W906RdGetString(W906RdSlurp(file), section.c_str(), ident.c_str(), std::string(),
                                      kW906BcbReadStringBuf, &found));
}
AnsiString W906_Win32ProfileDefault(const AnsiString& def)
{
    return AnsiString(W906RdGetString(std::string(), std::string(), std::string(), def.c_str(), kW906BcbReadStringBuf, 0));
}
void W906_DiskIniReadSection(const AnsiString& file, const AnsiString& section, TStrings* dest)
{
    if (!dest) return;
    dest->Clear();
    std::vector<std::string> names;
    W906RdBcbSection(file, section, &names);
    for (size_t i = 0; i < names.size(); ++i) dest->Add(AnsiString(names[i]));
}
void W906_DiskIniReadSections(const AnsiString& file, TStrings* dest)
{
    if (!dest) return;
    dest->Clear();
    std::vector<std::string> names;
    W906RdSplitNames(W906RdNames(W906RdSlurp(file), 0, kW906BcbReadSectionBuf), &names);
    for (size_t i = 0; i < names.size(); ++i) dest->Add(AnsiString(names[i]));
}
bool W906_DiskIniValueExists(const AnsiString& file, const AnsiString& section, const AnsiString& ident)
{
    std::vector<std::string> names;
    W906RdBcbSection(file, section, &names);
    const std::string id(ident.c_str());
    const W906RdSpan want = {id.data(), id.size()};                 // NOT trimmed: TStringList.IndexOf compares as given
    for (size_t i = 0; i < names.size(); ++i) {
        const W906RdSpan have = {names[i].data(), names[i].size()};
        if (W906RdSame(have, want)) return true;
    }
    return false;
}
bool W906_DiskIniSectionExists(const AnsiString& file, const AnsiString& section)
{
    std::vector<std::string> names;
    W906RdBcbSection(file, section, &names);
    return !names.empty();
}
} // namespace vclcompat

// =============================================================================
//  AI(W906-BOOTSPEED-2) 20260929: two boot-speed helpers for the write-through TIniFile above.
//
//  Measured on this machine (on-access virus scanner running; boot samples bootsample_20260929_203751_16644 and
//  _204608_12284, micro benchmarks on scratch copies):
//    * opening a path that does not exist costs 96-168 us (CreateFileA read, fopen "rb", fopen "wb"); asking
//      GetFileAttributesA about the same path costs ~6 us. W906_SecurityJamBoot (cSecurity.cpp) makes ~65 such
//      failed opens per alarm code x 3,278 codes on every boot while D:\HT9045\Error\English\ does not exist
//      (JAM0000.dat can then never be created, so golden's "build it once" loop runs every time) = most of the
//      32-46 s that stretch took.
//    * re-writing an ini with IDENTICAL bytes (fopen "wb" = truncate + write) costs 4-27 ms, because the scanner
//      re-scans a file after every handle that had write access; WriteLastDataFile (cprod.cpp) makes ~290 of them on
//      config.ini per call and runs 4 times at boot. kernel32 WritePrivateProfileStringA with an unchanged value
//      leaves the bytes alone but DOES move LastWriteTime (measured), so the equivalent end state is "same bytes,
//      LastWriteTime = now": stamping the time through a FILE_WRITE_ATTRIBUTES handle costs ~0.2 ms.
//  Neither helper changes a byte of any file, the value any read returns, or the order of the writes.
// =============================================================================
namespace vclcompat {
// 0 = open as before; 1 = the file does not exist (its directory does); 2 = the directory does not exist.
// Only the two "not found" answers short-circuit. Every other outcome (exists, access denied, bad name, ...) goes
// on to the original open, which then succeeds or fails exactly as it did before.
int W906_IniProbe(const AnsiString& file)
{
#ifdef _WIN32
    if (::GetFileAttributesA(file.c_str()) != INVALID_FILE_ATTRIBUTES) return 0;
    const DWORD e = ::GetLastError();
    if (e == ERROR_FILE_NOT_FOUND) return 1;
    if (e == ERROR_PATH_NOT_FOUND) return 2;
#else
    (void)file;
#endif
    return 0;
}
// Called only when the file already holds exactly the bytes W906_Win32WriteProfileString would write. Sets
// LastWriteTime to now -- what the rewrite (and kernel32) leave behind -- without writing the data again.
// false -> the caller rewrites as before. Declines whenever the rewrite could end differently: any attribute other
// than ARCHIVE (fopen "wb" = CREATE_ALWAYS + FILE_ATTRIBUTE_NORMAL fails on read-only / hidden / system files and
// resets the others), or ARCHIVE not set (a rewrite sets it).
bool W906_IniTouchSame(const AnsiString& file)
{
#ifdef _WIN32
    if (::GetFileAttributesA(file.c_str()) != FILE_ATTRIBUTE_ARCHIVE) return false;
    HANDLE h = ::CreateFileA(file.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    FILETIME now;
    ::GetSystemTimeAsFileTime(&now);
    const BOOL ok = ::SetFileTime(h, NULL, NULL, &now);
    ::CloseHandle(h);
    return ok != FALSE;
#else
    (void)file;
    return false;
#endif
}
} // namespace vclcompat

// =============================================================================
//  AI(W906-BOOTSPEED-2) 20260929: W906_IniApplyFast -- the same function as W906IniApply (AI(W906-T4-INIFMT2), the
//  measured kernel32 WritePrivateProfileStringA rules above), without the per-line std::string / std::vector work.
//  W906IniApply splits the whole file into a vector of line strings TWICE (W906IniDropWsTail + W906IniSplit), then
//  trims / lower-cases a copy of every line it looks at and joins everything back: 4.0-4.4 ms of CPU per write on the
//  33 KB config.ini at -O0 (more under the debugger's heap), and WriteLastDataFile alone makes ~290 writes per call,
//  4 calls per boot. This version walks the buffer once with offsets and builds only the output string.
//  Line by line it answers the same questions with the same definitions:
//    * lines end at CRLF, a bare CR or a bare LF (W906IniSplit); the last one may have no terminator;
//    * a trailing whitespace-only (' ' / '\t') line WITHOUT a terminator is dropped first (W906IniDropWsTail);
//    * header = trimmed text starts with '['; name = up to the first ']' of the trimmed text (or its end), trimmed,
//      compared A-Z-case-insensitively (W906IniSection + W906IniLower); the first matching header wins;
//    * key line = the first non-blank char is not ';' and the text has '='; key = text before the FIRST '=', trimmed,
//      same case folding (W906IniKey); the first matching key inside the section wins;
//    * the three results (replace the value / append at end of file / insert after the last key line) are built
//      exactly as W906IniApply builds them, including every W906IniEnsureNl.
//  Verified 20260929: old-vs-new on 400,000 random buffers + ~5,000 writes into copies of real machine files (scratch
//  differential, 0 differences) and the resident ctest IniFiles_Win32Diff (3,027 cases, TIniFile::WriteString ->
//  W906_Win32WriteProfileString -> this function, vs the real kernel32: 0 mismatches). CPU per write on the 33 KB
//  config.ini: 3.9 ms -> 0.09 ms (16.7 ms -> 0.15 ms under gdb's debug heap).
// =============================================================================
namespace vclcompat {
namespace {
inline bool W906FsWs(char c) { return c == ' ' || c == '\t'; }                 // == W906IniWs
struct W906FsLine { size_t s, n, e; };                                         // text [s, s+n); the line incl. its terminator ends at e
bool W906FsNext(const char* p, size_t len, size_t* pos, W906FsLine* ln)       // == one step of W906IniSplit
{
    const size_t i = *pos;
    if (i >= len) return false;
    size_t j = i;
    while (j < len && p[j] != '\r' && p[j] != '\n') ++j;
    ln->s = i;
    ln->n = j - i;
    if (j >= len) ln->e = len;
    else if (p[j] == '\r' && j + 1 < len && p[j + 1] == '\n') ln->e = j + 2;
    else ln->e = j + 1;
    *pos = ln->e;
    return true;
}
void W906FsTrim(const char* p, size_t* b, size_t* e)                          // == W906IniTrim on [b, e)
{
    while (*b < *e && W906FsWs(p[*b])) ++*b;
    while (*e > *b && W906FsWs(p[*e - 1])) --*e;
}
bool W906FsSame(const char* p, size_t b, size_t e, const std::string& lower) // W906IniLower(p[b, e)) == lower
{
    if (e - b != lower.size()) return false;
    for (size_t i = 0; i < lower.size(); ++i) {
        char c = p[b + i];
        if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
        if (c != lower[i]) return false;
    }
    return true;
}
bool W906FsHeader(const char* p, const W906FsLine& ln, size_t* nb, size_t* ne) // == W906IniSection (name span, not folded)
{
    size_t b = ln.s, e = ln.s + ln.n;
    W906FsTrim(p, &b, &e);
    if (b >= e || p[b] != '[') return false;
    size_t r = b + 1;
    while (r < e && p[r] != ']') ++r;
    size_t x = b + 1, y = r;
    W906FsTrim(p, &x, &y);
    *nb = x;
    *ne = y;
    return true;
}
bool W906FsKey(const char* p, const W906FsLine& ln, size_t* kb, size_t* ke, size_t* eq) // == W906IniKey (key span + first '=')
{
    const size_t e = ln.s + ln.n;
    size_t b = ln.s;
    while (b < e && W906FsWs(p[b])) ++b;
    if (b < e && p[b] == ';') return false;
    size_t k = ln.s;
    while (k < e && p[k] != '=') ++k;
    if (k >= e) return false;
    size_t x = ln.s, y = k;
    W906FsTrim(p, &x, &y);
    *kb = x;
    *ke = y;
    *eq = k;
    return true;
}
} // namespace

std::string W906_IniApplyFast(const std::string& bufIn, const std::string& secIn, const std::string& keyIn, const std::string& val)
{
    const char* const p = bufIn.data();
    size_t len = bufIn.size();
    if (len > 0 && p[len - 1] != '\r' && p[len - 1] != '\n') {                // W906IniDropWsTail
        size_t st = len;
        while (st > 0 && p[st - 1] != '\r' && p[st - 1] != '\n') --st;
        size_t i = st;
        while (i < len && W906FsWs(p[i])) ++i;
        if (i == len) len = st;
    }
    const std::string osec = W906IniTrim(secIn), okey = W906IniTrim(keyIn);
    const std::string lsec = W906IniLower(osec), lkey = W906IniLower(okey);
    size_t pos = 0, nb = 0, ne = 0, kb = 0, ke = 0, eq = 0;
    W906FsLine ln = {0, 0, 0}, sec = {0, 0, 0};
    bool found = false;
    while (W906FsNext(p, len, &pos, &ln))
        if (W906FsHeader(p, ln, &nb, &ne) && W906FsSame(p, nb, ne, lsec)) { sec = ln; found = true; break; }
    if (!found)
        return W906IniEnsureNl(W906IniEnsureNl(std::string(p, len)) + "[" + osec + "]\r\n" + okey + "=" + val + "\r\n");
    W906FsLine lastKey = sec, last = sec;
    bool nextHeader = false;
    while (W906FsNext(p, len, &pos, &ln)) {
        if (W906FsHeader(p, ln, &nb, &ne)) { nextHeader = true; break; }
        last = ln;
        if (W906FsKey(p, ln, &kb, &ke, &eq)) {
            if (W906FsSame(p, kb, ke, lkey)) {                                  // modify: keep up to and including '=', then the value
                std::string out;
                out.reserve(len + val.size() + 2);
                out.append(p, eq + 1);
                out += val;
                out.append(p + ln.s + ln.n, len - (ln.s + ln.n));
                return W906IniEnsureNl(out);
            }
            lastKey = ln;
        }
    }
    if (!nextHeader) {                                                          // section is the last one: blank last line -> append at EOF
        size_t b = last.s, e = last.s + last.n;
        W906FsTrim(p, &b, &e);
        if (b == e && lastKey.s != last.s)
            return W906IniEnsureNl(W906IniEnsureNl(std::string(p, len)) + okey + "=" + val + "\r\n");
    }
    std::string out;                                                            // insert after the last key line (or the header)
    out.reserve(len + okey.size() + val.size() + 5);
    out.append(p, lastKey.e);
    if (lastKey.e == lastKey.s + lastKey.n) out += "\r\n";
    out += okey;
    out += '=';
    out += val;
    out += "\r\n";
    out.append(p + lastKey.e, len - lastKey.e);
    return W906IniEnsureNl(out);
}
// The engine before AI(W906-BOOTSPEED-2), kept byte-for-byte as the reference the differential test compares with.
std::string W906_Win32ProfileApplyRef(const std::string& buf, const std::string& section, const std::string& key,
                                      const std::string& value)
{
    return W906IniApply(buf, section, key, value);
}
} // namespace vclcompat
