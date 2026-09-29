// =============================================================================
//  test_jam_w59.cpp -- ctest JamW59: ★W59-1 text core of WebSecurityJamW59.h (the editable long alarm description).
//
//  AI(W906-SEC-W59) 20260929 (St02-E).  Pure functions only (W906_JAMW59_CORE_ONLY): no file is read or written, no
//  fSecurity, no machine path -- so no sandbox is needed.  Byte literals were produced with Python's cp950 / cp949 /
//  cp1252 codecs (not typed by hand).
//    1. LangOf = golden ChangeJamMessage :987-1008 (code page, RTF charset, font)
//    2. DisplayText of plain files: UTF-8 kept, else a DBCS system code page (GetACP, passed in), else the language's --
//       Big5 in the Korea and English folders too (St02-E2 W1 / W2: this machine's files), CP949 / cp1252 on other PCs
//    3. DisplayText of an RTF like TRichEdit writes (font table charsets, \deff, \'xx, \uN + \ucN fallback, \par,
//       destinations skipped)
//    4. EncodeRtf: pure ASCII, golden's font per language, round trip through DisplayText for all four languages
//    5. characters the code page lacks -> \uN? (Hangul in a Big5 file), still round-trips
//    6. empty text -> no lines (golden SaveJamLevel :1200 does not save), only line breaks -> saved (Count != 0);
//       Normalize (CR, trailing line breaks)
// =============================================================================
#define W906_JAMW59_CORE_ONLY 1
#include "WebSecurityJamW59.h"

#include <cstdio>
#include <string>
#include <vector>

static int g_fail = 0, g_pass = 0;
static void Check(bool ok, const char* what)
{
    if (ok) { ++g_pass; std::printf("  ok   %s\n", what); }
    else    { ++g_fail; std::printf("  FAIL %s\n", what); }
}
static std::string Join(const std::vector<std::string>& v)
{
    std::string s;
    for (size_t i = 0; i < v.size(); ++i) { if (i) s += "\n"; s += v[i]; }   // = WebSecurityJam.cpp :212 (Lines joined by \n)
    return s;
}
static bool AllAscii(const std::vector<std::string>& v)
{
    for (size_t i = 0; i < v.size(); ++i)
        for (size_t j = 0; j < v[i].size(); ++j)
            if ((unsigned char)v[i][j] >= 0x80) return false;
    return true;
}

// UTF-8 / code-page literals (Python: str.encode)
static const char* kZhUtf8   = "\xe9\x8c\xaf\xe8\xaa\xa4";                                  // 錯誤
static const char* kZhBig5   = "\xbf\xf9\xbb\x7e";
static const char* kZh2Utf8  = "\xe8\xab\x8b\xe6\xaa\xa2\xe6\x9f\xa5\xe9\xa6\xac\xe9\x81\x94"; // 請檢查馬達
static const char* kKoUtf8   = "\xec\x98\xa4\xeb\xa5\x98";                                  // 오류
static const char* kKoCp949  = "\xbf\xc0\xb7\xf9";
static const char* kKo2Utf8  = "\xeb\xaa\xa8\xed\x84\xb0\x20\xed\x99\x95\xec\x9d\xb8";         // 모터 확인
static const char* kDegUtf8  = "\xc2\xb0" "C";                                              // °C
static const char* kDeg1252  = "\xb0" "C";

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("=== JamW59 (WebSecurityJamW59.h text core, ★W59-1) ===\n");

    std::printf("\n-- 1. LangOf\n");
    Check(jamw59::LangOf(0).cp == 1252 && jamw59::LangOf(0).charset == 0 && std::string(jamw59::LangOf(0).fontUtf8) == "Courier New",
          "1. English: cp1252, ANSI_CHARSET, Courier New (golden :988-992)");
    Check(jamw59::LangOf(1).cp == 950 && jamw59::LangOf(1).charset == 136 && jamw59::LangOf(3).cp == 950,
          "1. Chinese / Singapore: Big5 (950, CHINESEBIG5 136)");
    Check(jamw59::LangOf(2).cp == 949 && jamw59::LangOf(2).charset == 129 && std::string(jamw59::LangOf(2).fontUtf8) == "Batang",
          "1. Korea: 949, HANGEUL_CHARSET 129, Batang (golden :998-1003)");

    std::printf("\n-- 2. plain files\n");
    // the ACP is passed explicitly, so the result does not depend on the PC running the ctest (STEVEN-NB3: 950)
    Check(jamw59::DisplayText(kZhBig5, 1, 950) == kZhUtf8, "2. Chinese Big5 -> UTF-8 (ACP 950)");
    Check(jamw59::DisplayText(kZhBig5, 3, 950) == kZhUtf8, "2. Singapore Big5 -> UTF-8");
    Check(jamw59::DisplayText(kZhBig5, 2, 950) == kZhUtf8,
          "2. Korea folder holding Big5 (all 319 non-ASCII Korea files on STEVEN-NB3) -> the Chinese text, not Hangul (E2 W1)");
    Check(jamw59::DisplayText(kZhBig5, 0, 950) == kZhUtf8, "2. English folder holding Big5 (83 plain files, e.g. WAR0121) (E2 W2)");
    Check(jamw59::DisplayText(kKoCp949, 2, 949) == kKoUtf8, "2. a real CP949 Korea file on a Korean PC (ACP 949)");
    Check(jamw59::DisplayText(kKoCp949, 2, 1252) == kKoUtf8, "2. ... and on a single-byte-ACP PC (1252): the language's 949");
    Check(jamw59::DisplayText(kDeg1252, 0, 1252) == kDegUtf8, "2. English cp1252 degree sign on a 1252 PC");
    Check(jamw59::DisplayText("Check the motor.\nThen reset.", 0, 950) == "Check the motor.\nThen reset.", "2. English ASCII unchanged");
    Check(jamw59::DisplayText(kZhUtf8, 1, 950) == kZhUtf8, "2. a file already in UTF-8 is kept");

    std::printf("\n-- 3. RTF as TRichEdit writes it\n");
    {
        const std::string rtf =
            "{\\rtf1\\ansi\\ansicpg950\\deff0\\deftab480{\\fonttbl{\\f0\\fswiss\\fcharset136 \\'b7\\'4c\\'b3\\'6e\\'a5\\'bf\\'b6\\'c2\\'c5\\'e9;}"
            "{\\f1\\fnil\\fcharset0 Courier New;}}\n"
            "{\\colortbl\\red0\\green0\\blue0;}\n"
            "{\\*\\generator Riched20 10.0.19041;}\\viewkind4\\uc1\n"
            "\\pard\\plain\\f0\\fs16\\cf0 \\'bf\\'f9\\'bb\\'7e\\par\n"
            "\\plain\\f1\\fs16 Check\\tab A \\{1\\}\\par\n"
            "}";
        const std::string want = std::string(kZhUtf8) + "\nCheck\tA {1}\n";
        Check(jamw59::DisplayText(rtf, 1) == want, "3. font table charsets, \\'xx Big5, \\tab, escaped braces, \\*generator skipped");
    }
    {
        const std::string rtf = "{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl{\\f0\\fnil\\fcharset129 Batang;}}\\uc1\\pard\\f0 \\'bf\\'c0\\'b7\\'f9\\par}";
        Check(jamw59::DisplayText(rtf, 2) == std::string(kKoUtf8) + "\n", "3. \\deff0 font charset 129 decodes the body as CP949");
    }
    {
        const std::string rtf = "{\\rtf1\\ansi\\ansicpg950\\uc1\\pard \\u-14812?\\u-18088?\\par}";   // 오 U+C624, 류 U+B958
        Check(jamw59::DisplayText(rtf, 1) == std::string(kKoUtf8) + "\n", "3. \\uN with its one-character fallback (\\uc1)");
    }

    std::printf("\n-- 4. EncodeRtf round trips\n");
    const std::string texts[4] = {
        std::string("Check the motor \\ sensor {X1}.\n\tThen press RESET.\n") + kDegUtf8,
        std::string(kZhUtf8) + "\n" + kZh2Utf8,
        std::string(kKoUtf8) + "\n" + kKo2Utf8,
        std::string(kZh2Utf8) + " (SG)",
    };
    const char* heads[4] = { "{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl{\\f0\\fnil\\fcharset0 Courier New;}}",
                             "{\\rtf1\\ansi\\ansicpg950\\deff0{\\fonttbl{\\f0\\fnil\\fcharset136 \\'b7\\'4c\\'b3\\'6e\\'a5\\'bf\\'b6\\'c2\\'c5\\'e9;}}",
                             "{\\rtf1\\ansi\\ansicpg949\\deff0{\\fonttbl{\\f0\\fnil\\fcharset129 Batang;}}",
                             "{\\rtf1\\ansi\\ansicpg950\\deff0{\\fonttbl{\\f0\\fnil\\fcharset136 \\'b7\\'4c\\'b3\\'6e\\'a5\\'bf\\'b6\\'c2\\'c5\\'e9;}}" };
    for (int lang = 0; lang < 4; ++lang) {
        const std::vector<std::string> lines = jamw59::EncodeRtf(texts[lang], lang);
        char what[160];
        std::snprintf(what, sizeof(what), "4. language %d: golden's font / code page header, pure ASCII, round trip", lang);
        Check(!lines.empty() && lines[0] == heads[lang] && lines.back() == "}" && AllAscii(lines) &&
                  jamw59::Normalize(jamw59::DisplayText(Join(lines), lang)) == jamw59::Normalize(texts[lang]),
              what);
    }
    {
        const std::vector<std::string> lines = jamw59::EncodeRtf(std::string(kZhUtf8), 1);
        Check(lines.size() == 3 && lines[1] == "\\viewkind4\\uc1\\pard\\f0\\fs16 \\'bf\\'f9\\'bb\\'7e\\par",
              "4. the body line: \\'xx Big5 bytes, \\par (8 pt = golden's Font->Height -11)");
    }

    std::printf("\n-- 5. characters the code page lacks\n");
    {
        const std::vector<std::string> lines = jamw59::EncodeRtf(std::string(kKoUtf8), 1);   // Hangul in a Big5 file
        Check(lines.size() == 3 && lines[1].find("\\u-14812?") != std::string::npos && AllAscii(lines) &&
                  jamw59::Normalize(jamw59::DisplayText(Join(lines), 1)) == kKoUtf8,
              "5. Hangul in the Chinese file -> \\uN? and back");
    }

    std::printf("\n-- 6. empty text, Normalize\n");
    Check(jamw59::EncodeRtf("", 1).empty(), "6. empty box -> no lines (golden :1200 Count==0: nothing saved, the file stays)");
    {
        const std::vector<std::string> lines = jamw59::EncodeRtf("\r\n\n", 0);   // two line breaks = two lines in golden's box
        Check(lines.size() == 4 && lines[1] == "\\viewkind4\\uc1\\pard\\f0\\fs16 \\par" && lines[2] == "\\par" && lines[3] == "}",
              "6. only line breaks -> saved as empty lines (golden :1200 saves when Count != 0; E2 W6)");
    }
    Check(jamw59::Normalize("a\r\nb\r\n\r\n") == "a\nb" && jamw59::Normalize("a") == "a", "6. Normalize drops CR and trailing line breaks");

    std::printf("\n=== JamW59: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
