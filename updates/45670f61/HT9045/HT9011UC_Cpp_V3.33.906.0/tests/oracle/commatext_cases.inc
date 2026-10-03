// ===========================================================================
//  tests/oracle/commatext_cases.inc
//  AI(W906-COMMATEXT) 20261003: ONE case table + driver, compiled TWICE:
//
//    * by BCB6 bcc32 5.6.4 into tests/oracle/commatext_bcb6.cpp (the ORACLE: a real
//      Classes.TStringList from rtl.lib), whose output was pinned as
//      tests/oracle/commatext_bcb6_expected.txt;
//    * by MinGW g++ (C++17) into tests/test_vcl_commatext_bcb6.cpp (ctest
//      VclCommaTextBcb6), which runs the SAME code against vclcompat::TStringList
//      and compares its output with the pinned file line by line.
//
//  So the table, the driver and the output format cannot drift between the two:
//  only the TStringList underneath differs.  The includer must provide, in scope:
//  TStringList, TObject, AnsiString, and <stdio.h> (sprintf).
//
//  Constraints (this file must stay BCB6-compilable): C++98 only -- no auto, no
//  nullptr, no range-for, no lambdas, no std::string; pure ASCII source; every
//  non-ASCII byte and every NUL is a THREE-digit octal escape (a 3-digit octal
//  escape can never swallow the next character, unlike \x).
//
//  Output format (one case = two lines; everything after the id is escaped by
//  CtEsc: bytes 0x20..0x7E except [ ] \ verbatim, everything else \xHH):
//    <id> set n=<Count> [item0][item1]...      list state after the setter
//    <id> get [<text>]                         CommaText / DelimitedText read back
//  SET cases print "set" then "get" (set -> get round trip), GET cases print
//  "get" then "set" (get -> set round trip, parsed into a FRESH list).
// ===========================================================================

enum { CT_COMMA = 0, CT_DELIM = 1 };

// A literal plus its exact byte count (the literal may contain NULs).
#define CT_S(s) s, (int)(sizeof(s) - 1)

// SET case: list->CommaText = in   (CT_COMMA)
//           list->Delimiter = delim; list->QuoteChar = quote; list->DelimitedText = in   (CT_DELIM)
struct CtSetCase {
    const char* id;
    int         kind;
    char        delim;
    char        quote;
    const char* in;
    int         len;
    const char* note;   // human-readable; not printed
};

// GET case: items are each TERMINATED by \377 (0xFF never occurs inside an item here),
// so "" = empty list, "\377" = one empty item, "a\377\377" = {"a", ""}.
struct CtGetCase {
    const char* id;
    int         kind;
    char        delim;
    char        quote;
    const char* items;
    int         len;
    const char* note;
};

// UTF-8:  \344\270\255\346\226\207 = zhong wen ("Chinese"), \346\270\254\350\251\246 = ce shi ("test"),
//         \351\240\205\347\233\256 = xiang mu ("item"), \345\274\225\350\231\237 = yin hao ("quote"),
//         \345\205\247\345\256\271 = nei rong ("content"), \345\205\247 = nei alone (3 bytes)
// Big5:   the same words in cp950: \244\244\244\345, \264\372\270\325, \266\265\245\330, \244\336\270\271,
//         \244\272\256\145 (its second trail byte is ASCII 'e'); \263\134 = xu (trail byte 0x5C = backslash);
//         \244\174 = a Big5 char whose trail byte is 0x7C = '|'.
//
// Why every UTF-8 run here is an EVEN number of bytes: the oracle's SetDelimitedText /
// GetDelimitedText step with Win32 CharNext, which follows the ANSI code page (950 on the oracle
// box), i.e. it walks ANY byte 0x81..0xFE as a Big5 lead byte plus one trail byte.  BCB6 itself
// only ever sees Big5, where that walk is right; on UTF-8 it is not.  With even runs of 3-byte
// characters (2 chars = 6 bytes) the cp950 walk re-aligns on every ASCII byte, so these cases pin
// the delimiter / quote / blank rules, not an artefact of feeding UTF-8 to a cp950 RTL.  The port
// steps bytes, which on UTF-8 gives the item boundaries BCB6 gives on the Big5 original (compare
// S36 with S57).  (AnsiStrScan, used for the QUOTE char, follows SysLocale.FarEast instead, which
// SysUtils only sets for a non-western THREAD locale -- the oracle header records it; case V02.)
static const CtSetCase kCtSet[] = {
    { "S01", CT_COMMA, ',', '"', CT_S(""),                          "empty string -> empty list" },
    { "S02", CT_COMMA, ',', '"', CT_S("a"),                         "single token" },
    { "S03", CT_COMMA, ',', '"', CT_S("a,b,c"),                     "plain" },
    { "S04", CT_COMMA, ',', '"', CT_S("a,b,"),                      "delimiter as the LAST char adds one empty item" },
    { "S05", CT_COMMA, ',', '"', CT_S("a, "),                       "delimiter followed by a blank: no empty item" },
    { "S06", CT_COMMA, ',', '"', CT_S("a,,b"),                      "empty middle item" },
    { "S07", CT_COMMA, ',', '"', CT_S(",a"),                        "leading delimiter" },
    { "S08", CT_COMMA, ',', '"', CT_S(","),                         "lone delimiter" },
    { "S09", CT_COMMA, ',', '"', CT_S(",,"),                        "two delimiters" },
    { "S10", CT_COMMA, ',', '"', CT_S(", "),                        "delimiter + blank" },
    { "S11", CT_COMMA, ',', '"', CT_S("   a"),                      "leading blanks skipped" },
    { "S12", CT_COMMA, ',', '"', CT_S("   "),                       "only blanks" },
    { "S13", CT_COMMA, ',', '"', CT_S("07 Tester I/F"),             "a blank ends an unquoted token" },
    { "S14", CT_COMMA, ',', '"', CT_S("a b,c"),                     "" },
    { "S15", CT_COMMA, ',', '"', CT_S("a , b"),                     "blanks around the delimiter" },
    { "S16", CT_COMMA, ',', '"', CT_S("a ,,b"),                     "" },
    { "S17", CT_COMMA, ',', '"', CT_S("\"ab\"cd,e"),                "text after a closing quote starts the next item" },
    { "S18", CT_COMMA, ',', '"', CT_S("\"a b\",c"),                 "quoted blank" },
    { "S19", CT_COMMA, ',', '"', CT_S("\"x,y\",z"),                 "quoted delimiter" },
    { "S20", CT_COMMA, ',', '"', CT_S("\"a\"\"b\""),                "doubled quote" },
    { "S21", CT_COMMA, ',', '"', CT_S("\"\""),                      "empty quoted item" },
    { "S22", CT_COMMA, ',', '"', CT_S("\"\","),                     "empty quoted + trailing delimiter" },
    { "S23", CT_COMMA, ',', '"', CT_S("\"abc"),                     "unterminated quote" },
    { "S24", CT_COMMA, ',', '"', CT_S("\"a"),                       "unterminated, one char" },
    { "S25", CT_COMMA, ',', '"', CT_S("\""),                        "lone quote" },
    { "S26", CT_COMMA, ',', '"', CT_S("\"a\"\""),                   "unterminated, ends in a doubled quote" },
    { "S28", CT_COMMA, ',', '"', CT_S("\"\"\"\""),                  "four quotes = one quote char" },
    { "S29", CT_COMMA, ',', '"', CT_S("\"\"\"\"\""),                "five quotes (unterminated)" },
    { "S30", CT_COMMA, ',', '"', CT_S("a\"b"),                      "quote inside an unquoted token" },
    { "S31", CT_COMMA, ',', '"', CT_S("a\011b"),                    "tab" },
    { "S32", CT_COMMA, ',', '"', CT_S("a\015\012b"),                "CRLF" },
    { "S33", CT_COMMA, ',', '"', CT_S("a\015b,c"),                  "CR" },
    { "S34", CT_COMMA, ',', '"', CT_S("\011a,\011b\011"),           "tabs around" },
    { "S35", CT_COMMA, ',', '"', CT_S("a\001b"),                    "control char" },
    { "S36", CT_COMMA, ',', '"', CT_S("\344\270\255\346\226\207,\346\270\254\350\251\246 \351\240\205\347\233\256,\"\345\274\225\350\231\237 \345\205\247\345\256\271\""),
                                                                    "UTF-8 Chinese, one with a blank, one quoted" },
    { "S37", CT_COMMA, ',', '"', CT_S("2025-08-13,10:20:30.123,07 Tester I/F,JAM0701"),
                                                                    "St02's event-log example" },
    { "S38", CT_COMMA, ',', '"', CT_S("2023-09-01, 11:33:52.793, Process,MES2107,\"\011\",\"\011\",\"\011\",\"Change Start Mode\",\"Continuous Start\""),
                                                                    "real EventLogTxt line (fixtures/ela)" },
    { "S39", CT_COMMA, ',', '"', CT_S("2023-09-01, 11:33:54.131, Process,\"\011\",\"\011\",\"\011\",\"\011\",\"Auto clean : reset data because DevicePices was change\","),
                                                                    "real EventLogTxt line ending in a delimiter" },
    { "S40", CT_COMMA, ',', '"', CT_S("a,b\000c,d"),                "embedded NUL ends the parse" },
    { "S41", CT_COMMA, ',', '"', CT_S("a,\000b"),                   "NUL right after a delimiter = end of string" },
    { "S42", CT_COMMA, ',', '"', CT_S("  \"q\"  ,  x  "),           "blanks around quoted and plain" },
    { "S43", CT_COMMA, ',', '"', CT_S("\"a\" \"b\""),               "two quoted items separated by a blank only" },
    { "S44", CT_COMMA, ',', '"', CT_S("\"a\",\"\""),                "" },
    { "S45", CT_COMMA, ',', '"', CT_S("\"\",\"\""),                 "" },
    { "S46", CT_COMMA, ',', '"', CT_S("x\"y\"z"),                   "" },
    { "S47", CT_COMMA, ',', '"', CT_S("\244\244,\263\134"),         "Big5 bytes (trail byte 0x5C) with ','" },
    { "S48", CT_COMMA, ',', '"', CT_S("Schedule name, Start time, Input tray"),
                                                                    "Production_Log header" },
    { "S49", CT_COMMA, ',', '"', CT_S("1,2.5,-3,1e3,0x10"),         "numbers" },
    { "S50", CT_COMMA, ',', '"', CT_S("\"a\"b\"c\""),               "" },
    { "S51", CT_COMMA, ',', '"', CT_S("\"a\"\"\""),                 "terminated, ends in a doubled quote" },
    { "S52", CT_COMMA, ',', '"', CT_S("\"a,b"),                     "unterminated with a delimiter inside" },
    { "S53", CT_COMMA, ',', '"', CT_S("\" a \""),                   "quoted blanks kept" },
    { "S54", CT_COMMA, ',', '"', CT_S("a\177b,\240c"),              "DEL (0x7F) and 0xA0 are > ' ' (unsigned)" },
    { "S55", CT_COMMA, ',', '"', CT_S("\"a\"\"b\"\"c\",\"\"\"x\"\"\""), "two doubled pairs; quoted quote-x-quote" },
    { "S56", CT_COMMA, ',', '"', CT_S("\"ab\"\"cd"),                "unterminated with a doubled pair in the middle" },
    { "S57", CT_COMMA, ',', '"', CT_S("\244\244\244\345,\264\372\270\325 \266\265\245\330,\"\244\336\270\271 \244\272\256\145\""),
                                                                    "S36 in Big5 (cp950) bytes" },

    { "D01", CT_DELIM, ';',  '"',  CT_S("a;b c;\"d;e\""),            "';' delimiter" },
    { "D02", CT_DELIM, ';',  '"',  CT_S("a,b;c"),                    "',' is ordinary with ';'" },
    { "D03", CT_DELIM, '|',  '"',  CT_S("a|b||c|"),                  "'|' delimiter, trailing" },
    { "D04", CT_DELIM, ' ',  '"',  CT_S("a b  c "),                  "blank delimiter" },
    { "D05", CT_DELIM, '\t', '"',  CT_S("a\011b\011\011c\011"),      "tab delimiter: tabs are blanks too" },
    { "D06", CT_DELIM, '\t', '"',  CT_S("a\011\"b c\"\011d"),        "" },
    { "D07", CT_DELIM, ',',  '\'', CT_S("'a,b','c''d',e"),           "QuoteChar '" },
    { "D08", CT_DELIM, ',',  '\'', CT_S("\"a,b\""),                  "'\"' is ordinary with QuoteChar '" },
    { "D09", CT_DELIM, '\0', '"',  CT_S("a,b"),                      "NUL delimiter" },
    { "D10", CT_DELIM, ',',  '"',  CT_S("a b,c"),                    "DelimitedText with the defaults" },
    { "D11", CT_DELIM, '=',  '"',  CT_S("k=v w"),                    "" },
    { "D12", CT_DELIM, ':',  '"',  CT_S("10:20:30.123"),             "" },
    { "D13", CT_DELIM, ';',  '"',  CT_S("a;b;"),                     "trailing ';'" },
    { "D14", CT_DELIM, ';',  '"',  CT_S("a; "),                      "trailing ';' + blank" },

    // MBCS cases with their own port-side expectation (kPortOnly in tests/test_vcl_commatext_bcb6.cpp):
    // the oracle walks cp950 lead/trail bytes, which the port deliberately does not imitate (its
    // strings are UTF-8).  V01: CharNext (ACP 950) hides the Big5 trail byte 0x7C from the '|'
    // delimiter.  V02: an odd UTF-8 run right before the closing quote -- AnsiStrScan would take
    // that quote for a trail byte when SysLocale.FarEast is set, and FarEast depends on the oracle
    // box's thread locale, so the oracle line is not a stable reference.
    { "V01", CT_DELIM, '|',  '"',  CT_S("x\244\174y|z"),             "Big5 trail byte 0x7C under '|': CharNext skips it" },
    { "V02", CT_COMMA, ',',  '"',  CT_S("\"\345\205\247\",x"),       "UTF-8 odd run before a closing quote" }
};

// Run LAST and one by one: BCB6 raises EAccessViolation inside AnsiExtractQuotedStr here
// (an unterminated quoted field whose content is exactly two quote chars: SetLength(Result, 0),
// then Move writes one byte through PChar('') = System's read-only @@zeroByte).
static const CtSetCase kCtRisky[] = {
    { "R01", CT_COMMA, ',', '"', CT_S("\"\"\""),   "three quotes" },
    { "R02", CT_COMMA, ',', '"', CT_S("a,\"\"\""), "an item, then three quotes" }
};

static const CtGetCase kCtGet[] = {
    { "G01", CT_COMMA, ',', '"', CT_S(""),                          "empty list" },
    { "G02", CT_COMMA, ',', '"', CT_S("\377"),                      "one empty item" },
    { "G03", CT_COMMA, ',', '"', CT_S("\377\377"),                  "two empty items" },
    { "G04", CT_COMMA, ',', '"', CT_S("a\377\377b\377"),            "empty middle item is NOT quoted" },
    { "G05", CT_COMMA, ',', '"', CT_S("a b\377c\377"),              "" },
    { "G06", CT_COMMA, ',', '"', CT_S("x,y\377z\377"),              "" },
    { "G07", CT_COMMA, ',', '"', CT_S("a\"b\377"),                  "" },
    { "G08", CT_COMMA, ',', '"', CT_S("a\377b\377\377"),            "" },
    { "G09", CT_COMMA, ',', '"', CT_S("\377a\377"),                 "" },
    { "G10", CT_COMMA, ',', '"', CT_S(" lead\377trail \377"),       "" },
    { "G11", CT_COMMA, ',', '"', CT_S("tab\011here\377"),           "" },
    { "G12", CT_COMMA, ',', '"', CT_S("cr\015lf\012\377"),          "" },
    { "G13", CT_COMMA, ',', '"', CT_S("\344\270\255\346\226\207\377\346\270\254\350\251\246 \351\240\205\347\233\256\377"),
                                                                    "UTF-8 Chinese" },
    { "G14", CT_COMMA, ',', '"', CT_S("07 Tester I/F\377JAM0701\377"), "" },
    { "G15", CT_COMMA, ',', '"', CT_S("a\000b c\377"),              "NUL before the blank: scan stops, not quoted" },
    { "G16", CT_COMMA, ',', '"', CT_S("a b\000c\"d\377"),           "quoted; the quote after the NUL is not doubled" },
    { "G17", CT_COMMA, ',', '"', CT_S("\"\377"),                    "item = one quote char" },
    { "G18", CT_COMMA, ',', '"', CT_S("\377\377\377"),              "three empty items" },
    { "G19", CT_COMMA, ',', '"', CT_S("\"\"\377"),                  "item = two quote chars" },
    { "G20", CT_COMMA, ',', '"', CT_S("x\377"),                     "" },
    { "G21", CT_DELIM, ';', '"',  CT_S("a;b\377c,d\377e f\377"),    "';' delimiter" },
    { "G22", CT_DELIM, ',', '\'', CT_S("it's\377a b\377"),          "QuoteChar '" },
    { "G23", CT_DELIM, '|', '"',  CT_S("\377a\377\377"),            "" },
    { "G24", CT_COMMA, ',', '"', CT_S("a\"b c\377"),                "" },
    { "G25", CT_DELIM, ';', '"',  CT_S("\377"),                     "one empty item, ';'" },
    { "G26", CT_DELIM, ';', '\'', CT_S("\377"),                     "one empty item, QuoteChar '" },
    { "G27", CT_COMMA, ',', '"', CT_S("\001\377"),                  "control char" },
    { "G28", CT_COMMA, ',', '"', CT_S("\177\377\240\377"),          "0x7F and 0xA0 are not quoted" },
    { "G29", CT_COMMA, ',', '"', CT_S("2025-08-13\37710:20:30.123\37707 Tester I/F\377JAM0701\377"), "St02's example" },
    { "G30", CT_COMMA, ',', '"', CT_S("a\377\377\377b\377\377"),    "" }
};

static int CtNumSet()   { return (int)(sizeof(kCtSet)   / sizeof(kCtSet[0])); }
static int CtNumRisky() { return (int)(sizeof(kCtRisky) / sizeof(kCtRisky[0])); }
static int CtNumGet()   { return (int)(sizeof(kCtGet)   / sizeof(kCtGet[0])); }
static int CtNumBehav() { return 6; }

static void CtEsc(AnsiString& out, const char* p, int n)
{
    static const char hx[] = "0123456789ABCDEF";
    for (int k = 0; k < n; ++k) {
        unsigned char c = (unsigned char)p[k];
        if (c >= 0x20 && c < 0x7F && c != '[' && c != ']' && c != '\\') {
            char b[2]; b[0] = (char)c; b[1] = 0;
            out += b;
        } else {
            char b[5]; b[0] = '\\'; b[1] = 'x'; b[2] = hx[c >> 4]; b[3] = hx[c & 15]; b[4] = 0;
            out += b;
        }
    }
}

static void CtEscStr(AnsiString& out, const AnsiString& s) { CtEsc(out, s.c_str(), s.Length()); }

static void CtInt(AnsiString& out, int v)
{
    char b[32];
    sprintf(b, "%d", v);
    out += b;
}

// " n=<Count> [item0][item1]..."
static void CtDump(AnsiString& out, TStringList* sl)
{
    out += " n=";
    CtInt(out, sl->Count);
    for (int k = 0; k < sl->Count; ++k) {
        AnsiString s = sl->Strings[k];
        out += " [";
        CtEscStr(out, s);
        out += "]";
    }
}

static void CtLine(AnsiString& out, const char* id, const char* what)
{
    out += id;
    out += " ";
    out += what;
}

static AnsiString CtGetText(TStringList* sl, int kind)
{
    if (kind == CT_COMMA) { AnsiString t = sl->CommaText; return t; }
    AnsiString t = sl->DelimitedText;
    return t;
}

static void CtSetText(TStringList* sl, int kind, const AnsiString& v)
{
    if (kind == CT_COMMA) sl->CommaText = v;
    else                  sl->DelimitedText = v;
}

static void CtApplyKind(TStringList* sl, int kind, char delim, char quote)
{
    if (kind == CT_DELIM) { sl->Delimiter = delim; sl->QuoteChar = quote; }
}

static void CtRunSetCase(const CtSetCase& c, TStringList* sl, AnsiString& out)
{
    CtApplyKind(sl, c.kind, c.delim, c.quote);
    CtSetText(sl, c.kind, AnsiString(c.in, (unsigned int)c.len));
    CtLine(out, c.id, "set");
    CtDump(out, sl);
    out += "\n";
    AnsiString back = CtGetText(sl, c.kind);
    CtLine(out, c.id, "get [");
    CtEscStr(out, back);
    out += "]\n";
}

static void CtRunGetCase(const CtGetCase& c, TStringList* sl, AnsiString& out)
{
    CtApplyKind(sl, c.kind, c.delim, c.quote);
    int start = 0;
    for (int k = 0; k < c.len; ++k) {
        if ((unsigned char)c.items[k] == 0xFF) {
            sl->Add(AnsiString(c.items + start, (unsigned int)(k - start)));
            start = k + 1;
        }
    }
    AnsiString t = CtGetText(sl, c.kind);
    CtLine(out, c.id, "get [");
    CtEscStr(out, t);
    out += "]\n";
    TStringList* sl2 = new TStringList;
    CtApplyKind(sl2, c.kind, c.delim, c.quote);
    CtSetText(sl2, c.kind, t);
    CtLine(out, c.id, "set");
    CtDump(out, sl2);
    out += "\n";
    delete sl2;
}

static void CtDelimQuote(AnsiString& out, TStringList* sl)
{
    char d = sl->Delimiter;
    char q = sl->QuoteChar;
    out += " Delimiter=[";
    CtEsc(out, &d, 1);
    out += "] QuoteChar=[";
    CtEsc(out, &q, 1);
    out += "]";
}

static const char* CtBehavId(int i)
{
    static const char* ids[] = { "B01", "B02", "B03", "B04", "B05", "B06" };
    return ids[i];
}

// Behaviour around the parse/build itself (classes.pas SetCommaText / GetCommaText /
// SetDelimitedText's Clear + Add).
static void CtRunBehav(int i, TStringList* sl, AnsiString& out)
{
    const char* id = CtBehavId(i);
    switch (i) {
    case 0: // SetCommaText assigns Delimiter := ',' and QuoteChar := '"' for good (classes.pas:4306-4307)
        sl->Delimiter = ';';
        sl->QuoteChar = '\'';
        sl->CommaText = "a;b,'c d'";
        CtLine(out, id, "set");
        CtDump(out, sl);
        CtDelimQuote(out, sl);
        out += "\n";
        break;
    case 1: { // GetCommaText restores Delimiter / QuoteChar (classes.pas:4073-4083)
        sl->Delimiter = ';';
        sl->QuoteChar = '\'';
        sl->Add("a b");
        sl->Add("c");
        AnsiString t = sl->CommaText;
        CtLine(out, id, "get [");
        CtEscStr(out, t);
        out += "]";
        CtDelimQuote(out, sl);
        AnsiString d = sl->DelimitedText;
        out += " delimited=[";
        CtEscStr(out, d);
        out += "]\n";
        break; }
    case 2: { // Clear + Add: objects are dropped (classes.pas:4382 Clear, :4405 Add)
        sl->AddObject("x", (TObject*)sl);
        sl->AddObject("y", (TObject*)sl);
        sl->CommaText = "a,b";
        TObject* o0 = sl->Objects[0];
        TObject* o1 = sl->Objects[1];
        CtLine(out, id, "set");
        CtDump(out, sl);
        out += o0 ? " obj0=set" : " obj0=nil";
        out += o1 ? " obj1=set" : " obj1=nil";
        out += "\n";
        break; }
    case 3: // previous items are cleared
        sl->Add("old1");
        sl->Add("old2");
        sl->CommaText = "n";
        CtLine(out, id, "set");
        CtDump(out, sl);
        out += "\n";
        break;
    case 4: // after a CommaText set, DelimitedText parses with ',' (the side effect of case 0)
        sl->Delimiter = ';';
        sl->CommaText = "x";
        sl->DelimitedText = "a;b,c";
        CtLine(out, id, "set");
        CtDump(out, sl);
        CtDelimQuote(out, sl);
        out += "\n";
        break;
    case 5: { // CommaText set into a list whose DelimitedText setter is then used with a new delimiter
        sl->CommaText = "p q";
        sl->Delimiter = ' ';
        AnsiString d = sl->DelimitedText;
        CtLine(out, id, "get [");
        CtEscStr(out, d);
        out += "]";
        CtDump(out, sl);
        out += "\n";
        break; }
    }
}
