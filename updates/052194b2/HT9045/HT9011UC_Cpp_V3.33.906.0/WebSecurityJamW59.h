// =============================================================================
//  WebSecurityJamW59.h -- ★W59-1 on the Handler's Jam Code page (Status.Security, WS security.jam): the long alarm
//  description (golden RichEditJamCode) can be edited from the web, one file per language, for WebSecurityJam.cpp.
//
//  AI(W906-SEC-W59) 20260929 (St02-E).  Steven 0929 W59-1 = 可以改、用多國語言欄位（Error\<語言>\<碼>.dat，要處理
//  Big5／韓文／RTF）.  Golden 906_0625_Steven cSecurity.cpp (D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven):
//    ChangeJamMessage :976-1030  cbJamLang 0 English / 1 Chinese / 2 Korean / 3 Singapore -> the file
//                                D:\HT9045\Error\<English|Chinese|Korea|Singapore>\<code>.dat, Lines->LoadFromFile;
//                                font: Courier New / 微軟正黑體 / Batang + HANGEUL_CHARSET / 微軟正黑體 (ANSI_CHARSET
//                                otherwise, :987)
//    SaveJamLevel :1183-1201     same file; `if(RichEditJamCode->Lines->Count!=0) RichEditJamCode->Lines->SaveToFile`
//                                -- overwrite, no backup; an empty box is not saved (the file stays)
//    cSecurity.dfm :797-811      RichEditJamCode: TRichEdit, no ReadOnly, no PlainText => the operator edits it and
//                                TRichEdit saves RTF (a plain file becomes RTF on its first save)
//  So, as golden: an edited description is written as RTF, overwriting, only when the box is not empty.  The RTF is
//  pure ASCII: text in the language's code page as \'xx (English 1252, Chinese / Singapore 950, Korea 949), a
//  character the code page lacks as \uN? -- RichEdit (golden's reader) and DisplayText (ours) read both.
//  Not changed: a record the operator did not edit is written back as loaded (WebSecurityJam.cpp's existing path).
//
//  Machine files at 20260929 (STEVEN-NB3 D:\HT9045\Error; St02-E2 review ST02_REVIEW_W59_20260929.md): English 938 .dat,
//  390 of them already golden RTF, 83 plain ones with Big5 text; Chinese 600; Korea 360, whose 319 non-ASCII files are
//  ALL Big5 (249 byte-identical to Chinese, none CP949-only); Singapore 377, Big5.  The PC's ANSI code page is 950.
//  So plain text is read as golden's RichEdit / ANSI reads it: UTF-8 if valid, else the system code page (GetACP)
//  when it decodes strictly, else the language's code page.  RTF brings its own \ansicpg / \fcharset.
//
//  WebSecurityJam.cpp (St01's file -- the lines are claim requests, ledger "W59") uses:
//    jamw59::DisplayText(msg, lang)        for the "message" key (lang = cbJamLang->ItemIndex, as golden :983 sets
//                                          JamLang, which is private): the text as the operator sees it, UTF-8 (RTF decoded
//                                          with its code page; plain text: UTF-8, else GetACP strict, else the
//                                          language's).  Before, JsonWriter's Big5 / cp1252 guess garbled RTF \'xx.
//    jamw59::Apply(k, it)                  through jamw45::Apply (our WebSecurityJamW45.h, no claim): values
//                                          .RichEditJamCode = the edited text; unchanged text leaves the loaded lines.
//  The permission is the page's JamTabAllowed guard (select / save refuse without it), as for every Jam widget.
//  W906_JAMW59_CORE_ONLY (ctest JamW59): only the pure text functions below, no fSecurity / cJSON.
// =============================================================================
#pragma once

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include <windows.h>

namespace jamw59 {

// golden ChangeJamMessage :987-1008
struct Lang
{
    unsigned    cp;        // the code page of the .dat bytes
    int         charset;   // RTF \fcharset (0 ANSI, 136 CHINESEBIG5, 129 HANGEUL)
    const char* fontUtf8;  // golden's font for that language
};
inline Lang LangOf(int jamLang)
{
    if (jamLang == 2) return Lang{ 949, 129, "Batang" };                                               // Korean :998-1003
    if (jamLang == 1 || jamLang == 3)                                                                  // Chinese / Singapore
        return Lang{ 950, 136, "\xE5\xBE\xAE\xE8\xBB\x9F\xE6\xAD\xA3\xE9\xBB\x91\xE9\xAB\x94" };      // 微軟正黑體
    return Lang{ 1252, 0, "Courier New" };                                                             // English :988-992
}

inline std::wstring ToWide(const std::string& s, unsigned cp)
{
    if (s.empty()) return std::wstring();
    const int n = ::MultiByteToWideChar(cp, 0, s.data(), (int)s.size(), 0, 0);
    if (n <= 0) return std::wstring(s.begin(), s.end());
    std::wstring w((size_t)n, L'\0');
    ::MultiByteToWideChar(cp, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}
inline std::string ToUtf8(const std::wstring& w)
{
    if (w.empty()) return std::string();
    const int n = ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), 0, 0, 0, 0);
    if (n <= 0) return std::string();
    std::string s((size_t)n, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &s[0], n, 0, 0);
    return s;
}
inline bool IsUtf8(const std::string& s)
{
    return s.empty() || ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), 0, 0) > 0;
}

// RTF \fcharset -> code page (the ones a Handler machine can have; anything else = the language's)
inline unsigned CpOfCharset(int cs, unsigned dflt)
{
    switch (cs) {
    case 0:   return 1252;
    case 128: return 932;
    case 129: return 949;
    case 134: return 936;
    case 136: return 950;
    default:  return dflt;
    }
}

// RTF -> text (enough for what TRichEdit writes: groups, \fonttbl charsets, \'xx, \uN with \ucN fallback, \par / \line /
// \tab, the escaped \ { }, and destinations skipped).
inline std::wstring RtfToWide(const std::string& r, unsigned dflt)
{
    std::wstring out;
    std::string  pend;                     // bytes of the current run, decoded with cp when the run ends
    unsigned     cp = dflt, docCp = dflt;
    std::map<int, unsigned> fontCp;
    int depth = 0, skip = -1, fontTbl = -1, uc = 1, fallback = 0, defFont = -1, curDef = -1;
    auto flush = [&]() { if (!pend.empty()) { out += ToWide(pend, cp); pend.clear(); } };
    auto text  = [&](char c) { if (fallback > 0) { --fallback; return; } if (skip < 0) pend += c; };
    size_t i = 0;
    const size_t n = r.size();
    while (i < n) {
        const char ch = r[i];
        if (ch == '{') { ++depth; ++i; continue; }
        if (ch == '}') {
            if (skip == depth) skip = -1;
            if (fontTbl == depth) {                                        // font table done: the body starts in \deff's font
                fontTbl = -1;
                std::map<int, unsigned>::const_iterator it = fontCp.find(defFont);
                if (it != fontCp.end()) { flush(); cp = it->second; }
            }
            --depth; ++i; continue;
        }
        if (ch == '\r' || ch == '\n') { ++i; continue; }
        if (ch != '\\') { text(ch); ++i; continue; }
        if (i + 1 >= n) break;
        const char c1 = r[i + 1];
        if (c1 == '\'' && i + 3 < n) {                                     // \'xx
            const std::string hx = r.substr(i + 2, 2);
            char* e = 0;
            const long v = std::strtol(hx.c_str(), &e, 16);
            if (e && *e == '\0') text((char)v);
            i += 4;
            continue;
        }
        if (!((c1 >= 'a' && c1 <= 'z') || (c1 >= 'A' && c1 <= 'Z'))) {    // control symbol
            if (c1 == '*') { if (skip < 0) skip = depth; }
            else if (c1 == '\\' || c1 == '{' || c1 == '}') text(c1);
            else if (c1 == '~') text(' ');
            i += 2;
            continue;
        }
        size_t j = i + 1;
        std::string word;
        while (j < n && ((r[j] >= 'a' && r[j] <= 'z') || (r[j] >= 'A' && r[j] <= 'Z'))) word += r[j++];
        bool hasNum = false;
        long num = 0;
        if (j < n && (r[j] == '-' || (r[j] >= '0' && r[j] <= '9'))) {
            size_t k = j + 1;
            while (k < n && r[k] >= '0' && r[k] <= '9') ++k;
            num = std::strtol(r.substr(j, k - j).c_str(), 0, 10);
            hasNum = true;
            j = k;
        }
        if (j < n && r[j] == ' ') ++j;                                     // the delimiter space belongs to the word
        i = j;
        if (word == "fonttbl") { fontTbl = depth; if (skip < 0) skip = depth; continue; }
        if (fontTbl >= 0) {                                                // inside the font table: map \fN -> \fcharsetM
            if (word == "f" && hasNum) curDef = (int)num;
            else if (word == "fcharset" && hasNum && curDef >= 0) fontCp[curDef] = CpOfCharset((int)num, docCp);
            continue;
        }
        if (word == "colortbl" || word == "stylesheet" || word == "info" || word == "pict" || word == "header" ||
            word == "footer" || word == "object" || word == "listtable" || word == "listoverridetable") {
            if (skip < 0) skip = depth;
            continue;
        }
        if (skip >= 0) continue;
        if (word == "ansicpg" && hasNum) { flush(); docCp = cp = (unsigned)num; continue; }
        if (word == "deff" && hasNum) { defFont = (int)num; continue; }
        if (word == "f" && hasNum) {
            flush();
            std::map<int, unsigned>::const_iterator it = fontCp.find((int)num);
            cp = it != fontCp.end() ? it->second : docCp;
            continue;
        }
        if (word == "uc" && hasNum) { uc = (int)num; continue; }
        if (word == "u" && hasNum) { flush(); out += (wchar_t)(num < 0 ? num + 65536 : num); fallback = uc; continue; }
        if (word == "par" || word == "line") { flush(); out += L'\n'; continue; }
        if (word == "tab") { flush(); out += L'\t'; continue; }
    }
    flush();
    return out;
}

// the "message" the page shows (UTF-8)
inline bool DecodesStrictly(const std::string& s, unsigned cp)
{
    return !s.empty() && ::MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), 0, 0) > 0;
}

// acp = the PC's ANSI code page (a parameter so the ctest does not depend on the machine it runs on)
inline std::string DisplayText(const std::string& raw, int jamLang, unsigned acp = ::GetACP())
{
    const Lang L = LangOf(jamLang);
    if (raw.compare(0, 5, "{\\rtf") == 0) return ToUtf8(RtfToWide(raw, L.cp));
    if (IsUtf8(raw)) return raw;                                           // ASCII, or a file someone saved as UTF-8
    // golden's ANSI read (950 here: the Korea / English folders' Big5 too).  Only a DBCS system code page: a single-byte
    // one (1252) "decodes" every byte, so on such a PC a real CP949 Korea file goes to the language's code page instead.
    const bool dbcsAcp = acp == 932 || acp == 936 || acp == 949 || acp == 950;
    if (dbcsAcp && DecodesStrictly(raw, acp)) return ToUtf8(ToWide(raw, acp));
    if (DecodesStrictly(raw, L.cp)) return ToUtf8(ToWide(raw, L.cp));
    return ToUtf8(ToWide(raw, acp));
}

// the page's text normalised for "did the operator change it": no CR, no trailing line breaks
inline std::string Normalize(const std::string& s)
{
    std::string t;
    t.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) if (s[i] != '\r') t += s[i];
    while (!t.empty() && t[t.size() - 1] == '\n') t.erase(t.size() - 1);
    return t;
}

inline void AppendEscaped(std::string& o, const std::wstring& w, unsigned cp)
{
    char hex[8];
    for (size_t i = 0; i < w.size(); ++i) {
        const wchar_t c = w[i];
        if (c == L'\\' || c == L'{' || c == L'}') { o += '\\'; o += (char)c; continue; }
        if (c == L'\t') { o += "\\tab "; continue; }
        if (c < 0x80) { o += (char)c; continue; }
        char b[8];
        BOOL usedDefault = FALSE;
        const int k = ::WideCharToMultiByte(cp, WC_NO_BEST_FIT_CHARS, &c, 1, b, (int)sizeof(b), 0, &usedDefault);
        if (k > 0 && !usedDefault) {
            for (int m = 0; m < k; ++m) { std::snprintf(hex, sizeof(hex), "\\'%02x", (unsigned)(unsigned char)b[m]); o += hex; }
        } else {
            std::snprintf(hex, sizeof(hex), "%d", (int)(short)c);
            o += "\\u"; o += hex; o += '?';
        }
    }
}

// the edited text (UTF-8) as the RTF lines golden's TRichEdit would save.  Golden :1200 saves when Lines->Count != 0, so
// only a truly empty box gives no lines (not saved); a box of only line breaks is saved, as golden does (St02-E2 W6).
inline std::vector<std::string> EncodeRtf(const std::string& utf8, int jamLang)
{
    std::vector<std::string> lines;
    std::string t;
    for (size_t i = 0; i < utf8.size(); ++i) if (utf8[i] != '\r') t += utf8[i];
    if (t.empty()) return lines;
    if (t[t.size() - 1] == '\n') t.erase(t.size() - 1);                  // the box's last line break = the final \par below
    const Lang L = LangOf(jamLang);
    char num[32];
    std::string head = "{\\rtf1\\ansi\\ansicpg";
    std::snprintf(num, sizeof(num), "%u", L.cp); head += num;
    head += "\\deff0{\\fonttbl{\\f0\\fnil\\fcharset";
    std::snprintf(num, sizeof(num), "%d", L.charset); head += num;
    head += ' ';
    AppendEscaped(head, ToWide(L.fontUtf8, CP_UTF8), L.cp);
    head += ";}}";
    lines.push_back(head);
    const std::wstring w = ToWide(t, CP_UTF8);
    std::string cur = "\\viewkind4\\uc1\\pard\\f0\\fs16 ";
    size_t a = 0;
    while (true) {
        const size_t b = w.find(L'\n', a);
        AppendEscaped(cur, w.substr(a, b == std::wstring::npos ? std::wstring::npos : b - a), L.cp);
        cur += "\\par";
        lines.push_back(cur);
        cur.clear();
        if (b == std::wstring::npos) break;
        a = b + 1;
    }
    lines.push_back("}");
    return lines;
}

}  // namespace jamw59

#ifndef W906_JAMW59_CORE_ONLY
#include "Public/cJSON.h"
#include "forms/fSecurity.h"

namespace jamw59 {

// values.RichEditJamCode (the operator's text for the "from" record, already re-read from its file): true = taken.
// Unchanged -> the loaded lines stay (they are written back as before); changed -> the RTF lines; empty -> no lines, so
// golden SaveJamLevel :1200 does not save (the file keeps its text); only line breaks -> saved, as golden (Count != 0).
inline bool Apply(const std::string& name, const cJSON* it)
{
    if (name != "RichEditJamCode" || it == 0 || !cJSON_IsString(it) || it->valuestring == 0) return false;
    TfSecurity* f = fSecurity;
    if (f == 0 || f->RichEditJamCode == 0 || f->RichEditJamCode->Lines == 0) return false;
    std::string cur;
    for (int i = 0; i < f->RichEditJamCode->Lines->Count; ++i) {
        if (i) cur += "\n";
        cur += f->RichEditJamCode->Lines->Strings[i].c_str();
    }
    const std::string edited = it->valuestring;
    const int lang = f->cbJamLang->ItemIndex < 0 ? 0 : f->cbJamLang->ItemIndex;   // = golden :983 JamLang=cbJamLang->ItemIndex (JamLang is private)
    if (Normalize(DisplayText(cur, lang)) == Normalize(edited)) return true;
    const std::vector<std::string> lines = EncodeRtf(edited, lang);
    f->RichEditJamCode->Lines->Clear();
    for (size_t i = 0; i < lines.size(); ++i) f->RichEditJamCode->Lines->Add(AnsiString(lines[i].c_str()));
    return true;
}

}  // namespace jamw59
#endif
