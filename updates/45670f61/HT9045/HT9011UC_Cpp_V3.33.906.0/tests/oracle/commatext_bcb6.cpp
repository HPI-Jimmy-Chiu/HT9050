// ===========================================================================
//  tests/oracle/commatext_bcb6.cpp -- BCB6 ORACLE for TStrings.CommaText / DelimitedText
//  AI(W906-COMMATEXT) 20261003
//
//  NOT part of the CMake build.  Compiled with BCB6 bcc32 5.6.4 against the real VCL
//  (rtl.lib: Classes.TStringList, SysUtils.AnsiExtractQuotedStr / AnsiQuotedStr); see
//  tests/oracle/README_commatext.md for the exact command.  Its output file is pinned as
//  tests/oracle/commatext_bcb6_expected.txt, which ctest VclCommaTextBcb6 compares the
//  port (vclcompat::TStringList) against.  Keep the exe OUT of the tree.
//
//  usage: see main() below.
// ===========================================================================
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "commatext_cases.inc"

static void Commit(AnsiString& out, const AnsiString& piece) { out += piece; }

// One case = a fresh list; an exception is reported as "<id> EXC <class>" plus the list state
// the exception left behind (SetDelimitedText's try/finally EndUpdate keeps what was added).
static void RunSet(const CtSetCase& c, AnsiString& out)
{
    TStringList* sl = new TStringList;
    AnsiString piece;
    try {
        CtRunSetCase(c, sl, piece);
        Commit(out, piece);
    } catch (EAccessViolation&) {
        AnsiString e; CtLine(e, c.id, "EXC EAccessViolation"); CtDump(e, sl); e += "\n"; Commit(out, e);
    } catch (Exception& ex) {
        AnsiString e; CtLine(e, c.id, "EXC Exception ["); CtEscStr(e, ex.Message); e += "]"; CtDump(e, sl); e += "\n"; Commit(out, e);
    } catch (...) {
        AnsiString e; CtLine(e, c.id, "EXC unknown"); CtDump(e, sl); e += "\n"; Commit(out, e);
    }
    delete sl;
}

static void RunGet(const CtGetCase& c, AnsiString& out)
{
    TStringList* sl = new TStringList;
    AnsiString piece;
    try {
        CtRunGetCase(c, sl, piece);
        Commit(out, piece);
    } catch (...) {
        AnsiString e; CtLine(e, c.id, "EXC"); e += "\n"; Commit(out, e);
    }
    delete sl;
}

static void RunBehav(int i, AnsiString& out)
{
    TStringList* sl = new TStringList;
    AnsiString piece;
    try {
        CtRunBehav(i, sl, piece);
        Commit(out, piece);
    } catch (...) {
        AnsiString e; CtLine(e, CtBehavId(i), "EXC"); e += "\n"; Commit(out, e);
    }
    delete sl;
}

// Each finished case is appended to the file at once, so a crash still leaves the earlier ones.
static void Flush(const char* outPath, AnsiString& out)
{
    FILE* f = fopen(outPath, "ab");
    if (f) { fwrite(out.c_str(), 1, out.Length(), f); fclose(f); }
    printf("%s", out.c_str());
    fflush(stdout);
    out = "";
}

// usage: commatext_bcb6.exe <out.txt>            the table (S/D/V, G, B cases)
//        commatext_bcb6.exe <out.txt> <Rnn>      ONE risky case (R01 / R02), appended to <out.txt>
#pragma argsused
int main(int argc, char* argv[])
{
    const char* outPath = (argc > 1) ? argv[1] : "commatext_bcb6_out.txt";
    AnsiString out;
    int i;
    if (argc > 2) {
        for (i = 0; i < CtNumRisky(); ++i) {
            if (strcmp(argv[2], kCtRisky[i].id) == 0) {
                RunSet(kCtRisky[i], out);
                Flush(outPath, out);
                // Post-check in the same process: the RTL still parses "" to an empty list.
                TStringList* sl = new TStringList;
                sl->CommaText = "";
                AnsiString e; CtLine(e, kCtRisky[i].id, "post set \"\" ->"); CtDump(e, sl); e += "\n"; out += e;
                delete sl;
                Flush(outPath, out);
                return 0;
            }
        }
        printf("unknown case %s\n", argv[2]);
        return 2;
    }

    FILE* f0 = fopen(outPath, "wb");
    if (!f0) { printf("cannot write %s\n", outPath); return 2; }
    fclose(f0);
    // CharNext (SetDelimitedText / GetDelimitedText) follows the ANSI code page; AnsiStrScan
    // (AnsiExtractQuotedStr / AnsiQuotedStr) follows SysLocale.FarEast, which SysUtils.InitSysLocale
    // sets only when the THREAD locale is not a western language -- so record both.
    char hdr[256];
    sprintf(hdr, "# commatext oracle: __BORLANDC__=0x%X, GetACP()=%u, SysLocale.FarEast=%d\n",
            (unsigned)__BORLANDC__, (unsigned)GetACP(), (int)SysLocale.FarEast);
    out += hdr;
    out += "# source: tests/oracle/commatext_cases.inc; lines starting with # are not compared\n";
    Flush(outPath, out);

    for (i = 0; i < CtNumSet(); ++i)   { RunSet(kCtSet[i], out);   Flush(outPath, out); }
    for (i = 0; i < CtNumGet(); ++i)   { RunGet(kCtGet[i], out);   Flush(outPath, out); }
    for (i = 0; i < CtNumBehav(); ++i) { RunBehav(i, out);         Flush(outPath, out); }
    return 0;
}
