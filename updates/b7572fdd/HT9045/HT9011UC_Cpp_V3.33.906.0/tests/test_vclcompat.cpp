// ===========================================================================
//  tests/test_vclcompat.cpp
//  Assertion harness for the TRICKY BCB6 semantics the compat layer must
//  reproduce. Non-zero exit on any failure. No external test framework.
//
//  Build:  linked against the `vclcompat` static lib (see tests/CMakeLists.txt).
// ===========================================================================
#include "vclcompat/vcl_compat.h"   // exercises the umbrella header too
#include "vclcompat/Controls.h"     // AI(W906-D024) 20261001: TRadioGroup::Items

#include <cstdio>
#include <string>

static int g_fail = 0;
static int g_total = 0;

static void check(bool cond, const char* expr, const char* file, int line) {
    ++g_total;
    if (!cond) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n", file, line, expr);
    }
}
#define CHECK(cond) check((cond), #cond, __FILE__, __LINE__)

// Compare an AnsiString to an expected literal, printing both on mismatch.
static void checkEq(const AnsiString& got, const char* expect,
                    const char* what, const char* file, int line) {
    ++g_total;
    if (std::string(got.c_str()) != std::string(expect)) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s : got \"%s\" expected \"%s\"\n",
                    file, line, what, got.c_str(), expect);
    }
}
#define CHECK_EQ(got, expect) checkEq((got), (expect), #got, __FILE__, __LINE__)

int main() {
    // ===================================================================
    //  AnsiString : 1-BASED indexing
    // ===================================================================
    {
        AnsiString s("Hello");
        // operator[](1) is the FIRST char
        CHECK(s[1] == 'H');
        CHECK(s[5] == 'o');
        // l-value through operator[]
        s[1] = 'J';
        CHECK_EQ(s, "Jello");
    }

    // ---- Pos : 1-based, 0 on miss -------------------------------------
    {
        AnsiString s("ABCDEF");
        CHECK(s.Pos("A") == 1);          // first char -> 1
        CHECK(s.Pos("C") == 3);
        CHECK(s.Pos("CD") == 3);
        CHECK(s.Pos("XYZ") == 0);        // miss -> 0
        CHECK(s.Pos('E') == 5);
        AnsiString tag("@SET_TEMPOK");
        CHECK(tag.Pos("@SET_TEMPOK") == 1);   // real-source idiom: ==1
        CHECK(AnsiString("foo bar").Pos("bar") == 5);
        CHECK(AnsiString("no match here").Pos("zzz") == 0);
        // AnsiPos mirrors Pos for single-byte
        CHECK(AnsiString("ABCDEF").AnsiPos("DEF") == 4);
    }

    // ---- SubString : 1-based start; SubString(6,..) skips first 5 ------
    {
        AnsiString s("HelloWorld");
        CHECK_EQ(s.SubString(1, 5), "Hello");
        CHECK_EQ(s.SubString(6, 5), "World");    // start 6 skips the first 5
        CHECK_EQ(s.SubString(6, 100), "World");  // len clamps to available
        CHECK_EQ(s.SubString(1, 1), "H");
        // BCB6 quirk: start 0 behaves like start 1
        CHECK_EQ(s.SubString(0, 3), "Hel");
        // start past end -> empty
        CHECK_EQ(s.SubString(50, 3), "");
    }

    // ---- Delete / Insert : 1-based, in place --------------------------
    {
        AnsiString s("ABCDEF");
        s.Delete(2, 3);                  // remove "BCD" (1-based index 2, count 3)
        CHECK_EQ(s, "AEF");

        AnsiString s2("ACE");
        s2.Insert(AnsiString("B"), 2);   // insert before position 2 -> "ABCE"
        CHECK_EQ(s2, "ABCE");
        s2.Insert(AnsiString("X"), 1);   // at front
        CHECK_EQ(s2, "XABCE");
        s2.Insert(AnsiString("Z"), 100); // past end -> append
        CHECK_EQ(s2, "XABCEZ");

        // Delete out-of-range is a no-op (BCB6 tolerant)
        AnsiString s3("AB");
        s3.Delete(5, 2);
        CHECK_EQ(s3, "AB");
    }

    // ---- LastDelimiter : 1-based ---------------------------------------
    {
        AnsiString path("dir.sub\\file.txt");
        CHECK(path.LastDelimiter(".") == 13);     // last '.'
        CHECK(path.LastDelimiter("\\") == 8);     // the backslash
        CHECK(AnsiString("nofind").LastDelimiter(".") == 0);
        CHECK(AnsiString("a.b.c").LastDelimiter('.') == 4);
    }

    // ===================================================================
    //  AnsiString : number ctors & operator+ chains
    // ===================================================================
    {
        CHECK_EQ(AnsiString(42), "42");
        CHECK_EQ(AnsiString(-7), "-7");
        CHECK_EQ(AnsiString(0), "0");
        // double formatting (FloatToStr-style: trailing zeros stripped)
        CHECK_EQ(AnsiString(3.0), "3");
        CHECK_EQ(AnsiString(1.5), "1.5");
        CHECK_EQ(AnsiString(141.3), "141.3");
        CHECK_EQ(AnsiString(0.25), "0.25");

        // operator+ chains with implicit int/double/char* promotion
        int x = 5; double y = 2.5;
        AnsiString r = AnsiString("X=") + x + ", Y=" + y;
        CHECK_EQ(r, "X=5, Y=2.5");
        AnsiString r2 = "[" + AnsiString(10) + "]";
        CHECK_EQ(r2, "[10]");
        // char concatenation
        AnsiString r3 = AnsiString("AB") + 'C';
        CHECK_EQ(r3, "ABC");
    }

    // ---- comparisons ---------------------------------------------------
    {
        AnsiString a("foo");
        CHECK(a == "foo");
        CHECK("foo" == a);
        CHECK(a != "bar");
        CHECK(AnsiString("abc") < AnsiString("abd"));
        CHECK(AnsiString("abd") > AnsiString("abc"));
    }

    // ===================================================================
    //  AnsiString : UpperCase / LowerCase / Trim (return NEW string)
    // ===================================================================
    {
        AnsiString s("  Hello World  ");
        CHECK_EQ(s.Trim(), "Hello World");
        CHECK_EQ(s.UpperCase().Trim(), "HELLO WORLD");   // original unchanged
        CHECK_EQ(s, "  Hello World  ");                  // Trim did not mutate
        CHECK_EQ(AnsiString("MixedCase").LowerCase(), "mixedcase");
        CHECK_EQ(AnsiString("\t pad\n").TrimLeft(), "pad\n");
    }

    // ===================================================================
    //  AnsiString : sprintf (in place, returns *this) + %s with AnsiString
    // ===================================================================
    {
        AnsiString s;
        s.sprintf("%04d-%02d-%02d", 2026, 6, 25);
        CHECK_EQ(s, "2026-06-25");

        // BCB6 idiom: pass a RAW AnsiString to %s (no .c_str())
        AnsiString part("U17");
        AnsiString msg;
        msg.sprintf("need remove device of %s", part);
        CHECK_EQ(msg, "need remove device of U17");

        // printf alias
        AnsiString p; p.printf("CH%d", 7);
        CHECK_EQ(p, "CH7");

        // cat_printf appends
        AnsiString buf("LotNo:");
        buf.cat_printf("%s\n", AnsiString("ABC123"));
        CHECK_EQ(buf, "LotNo:ABC123\n");

        // ToInt / ToIntDef / ToDouble
        CHECK(AnsiString("123").ToInt() == 123);
        CHECK(AnsiString("  -45 ").ToInt() == -45);
        CHECK(AnsiString("xyz").ToIntDef(99) == 99);
        CHECK(AnsiString("7").ToIntDef(0) == 7);
        CHECK(AnsiString("2.5").ToDouble() == 2.5);
        CHECK(AnsiString("abc").IsEmpty() == false);
        CHECK(AnsiString("").IsEmpty() == true);
        CHECK(AnsiString("hello").Length() == 5);
    }

    // ===================================================================
    //  SysUtils free functions
    // ===================================================================
    {
        CHECK_EQ(IntToStr(2026), "2026");
        CHECK(StrToInt(AnsiString("88")) == 88);
        CHECK(StrToIntDef(AnsiString("bad"), -1) == -1);
        CHECK(StrToFloatDef(AnsiString("nope"), 3.5) == 3.5);

        CHECK_EQ(FloatToStr(141.3), "141.3");
        CHECK_EQ(FloatToStr(3.0), "3");

        // FormatFloat picture strings (real-source patterns)
        CHECK_EQ(FormatFloat(AnsiString("0.00"), 3.14159), "3.14");
        CHECK_EQ(FormatFloat(AnsiString("0.0"), 2.0), "2.0");
        CHECK_EQ(FormatFloat(AnsiString("0"), 7.6), "8");          // rounds, no decimals
        CHECK_EQ(FormatFloat(AnsiString("0.000"), 1.5), "1.500");
        CHECK_EQ(FormatFloat(AnsiString("00.0"), 3.4), "03.4");    // mandatory int zero pad
        CHECK_EQ(FormatFloat(AnsiString("#0.00"), 0.5), "0.50");
        CHECK_EQ(FormatFloat(AnsiString("0.00%"), 0.1234), "12.34%");

        // FloatToStrF(ffFixed)
        CHECK_EQ(FloatToStrF(12.3456, ffFixed, 15, 2), "12.35");

        // Format (ARRAYOFCONST reduces to bare args)
        CHECK_EQ(Format(AnsiString("A%d"), ARRAYOFCONST((4 + 1))), "A5");
        CHECK_EQ(Format(AnsiString("%.1f"), ARRAYOFCONST((25.0))), "25.0");

        // path helpers
        CHECK_EQ(ExtractFileName(AnsiString("C:\\a\\b\\file.txt")), "file.txt");
        CHECK_EQ(ExtractFilePath(AnsiString("C:\\a\\b\\file.txt")), "C:\\a\\b\\");
        CHECK_EQ(ExtractFileExt(AnsiString("file.txt")), ".txt");
        CHECK_EQ(ChangeFileExt(AnsiString("image.bmp"), AnsiString(".dump")), "image.dump");
        CHECK_EQ(IncludeTrailingBackslash(AnsiString("C:\\dir")), "C:\\dir\\");
        CHECK_EQ(IncludeTrailingBackslash(AnsiString("C:\\dir\\")), "C:\\dir\\");
    }

    // ===================================================================
    //  TStringList : Add / Strings[] / Count / CommaText / IndexOf / Sort
    // ===================================================================
    {
        TStringList* sl = new TStringList();
        CHECK(sl->Add("alpha") == 0);     // returns new index (0-based)
        CHECK(sl->Add("beta") == 1);
        CHECK(sl->Add("gamma") == 2);
        CHECK(sl->Count == 3);
        // Strings[i] is 0-based
        CHECK_EQ(AnsiString(sl->Strings[0]), "alpha");
        CHECK_EQ(AnsiString(sl->Strings[2]), "gamma");
        // write through Strings[]
        sl->Strings[1] = "BETA";
        CHECK_EQ(AnsiString(sl->Strings[1]), "BETA");
        // IndexOf : -1 if absent
        CHECK(sl->IndexOf("alpha") == 0);
        CHECK(sl->IndexOf("missing") == -1);
        // First
        CHECK_EQ(sl->First(), "alpha");
        // Delete (0-based)
        sl->Delete(0);
        CHECK(sl->Count == 2);
        CHECK_EQ(AnsiString(sl->Strings[0]), "BETA");
        delete sl;
    }

    // ---- CommaText round-trip + quoting --------------------------------
    {
        TStringList* sl = new TStringList();
        sl->CommaText = "one,two,three";
        CHECK(sl->Count == 3);
        CHECK_EQ(AnsiString(sl->Strings[0]), "one");
        CHECK_EQ(AnsiString(sl->Strings[2]), "three");

        // a field containing a space gets quoted on the way out
        TStringList* sl2 = new TStringList();
        sl2->Add("a b");
        sl2->Add("c");
        CHECK_EQ(AnsiString(sl2->CommaText), "\"a b\",c");

        // quoted field with embedded comma parses as ONE element
        TStringList* sl3 = new TStringList();
        sl3->CommaText = "\"x,y\",z";
        CHECK(sl3->Count == 2);
        CHECK_EQ(AnsiString(sl3->Strings[0]), "x,y");
        CHECK_EQ(AnsiString(sl3->Strings[1]), "z");

        delete sl; delete sl2; delete sl3;
    }

    // ---- Sort (relies on AnsiString operator<) -------------------------
    {
        TStringList* sl = new TStringList();
        sl->Add("banana");
        sl->Add("apple");
        sl->Add("cherry");
        sl->Sort();
        CHECK_EQ(AnsiString(sl->Strings[0]), "apple");
        CHECK_EQ(AnsiString(sl->Strings[1]), "banana");
        CHECK_EQ(AnsiString(sl->Strings[2]), "cherry");
        delete sl;
    }

    // ---- Text get/set ---------------------------------------------------
    {
        TStringList* sl = new TStringList();
        sl->Text = "line1\r\nline2\r\nline3";
        CHECK(sl->Count == 3);
        CHECK_EQ(AnsiString(sl->Strings[1]), "line2");
        delete sl;
    }

    // ===================================================================
    //  TDateTime : FormatDateTime token handling
    // ===================================================================
    {
        // 2026-06-25 13:07:09.123  -> build a known serial via Encode*
        TDateTime dt = EncodeDate(2026, 6, 25) + EncodeTime(13, 7, 9, 123);

        // month vs minute disambiguation:
        //   yyyy-mm-dd  -> mm is MONTH
        CHECK_EQ(FormatDateTime(AnsiString("yyyy-mm-dd"), dt), "2026-06-25");
        //   HH:MM:ss    -> MM is MINUTE (follows hour token)
        CHECK_EQ(FormatDateTime(AnsiString("HH:MM:ss"), dt), "13:07:09");
        //   nn is always minute
        CHECK_EQ(FormatDateTime(AnsiString("hh:nn:ss"), dt), "13:07:09");
        //   milliseconds
        CHECK_EQ(FormatDateTime(AnsiString("hh:nn:ss.zzz"), dt), "13:07:09.123");
        //   combined date+time (real-source format)
        CHECK_EQ(FormatDateTime(AnsiString("yyyy-mm-dd HH:MM:ss"), dt),
                 "2026-06-25 13:07:09");
        //   AM/PM 12-hour clock
        CHECK_EQ(FormatDateTime(AnsiString("hh:nn AM/PM"), dt), "01:07 PM");

        // round-trip via Decode
        Word Y, Mo, D, h, mi, s, ms;
        DecodeDate(dt, Y, Mo, D);
        DecodeTime(dt, h, mi, s, ms);
        CHECK(Y == 2026 && Mo == 6 && D == 25);
        CHECK(h == 13 && mi == 7 && s == 9 && ms == 123);

        // elapsed math: (later - earlier) as double days
        TDateTime t0 = EncodeDate(2026, 1, 1);
        TDateTime t1 = EncodeDate(2026, 1, 2);
        double elapsedDays = (double)(t1 - t0);
        CHECK(elapsedDays == 1.0);
    }

    // ===================================================================
    //  HexStrToInt : BCB6 project-local hex parser (EJ1N/TextProcess.cpp)
    //  Net behaviour: ensure "0x" prefix then hex-parse, -1 on failure.
    // ===================================================================
    {
        // bare hex digits (the IO_Table Port / ISABase column form)
        CHECK(HexStrToInt(AnsiString("FF"))   == 255);
        CHECK(HexStrToInt(AnsiString("10"))   == 16);     // hex 10 == 16
        CHECK(HexStrToInt(AnsiString("0"))    == 0);
        CHECK(HexStrToInt(AnsiString("280"))  == 0x280);  // 640
        CHECK(HexStrToInt(AnsiString("a"))    == 10);     // lower-case ok
        // explicit "0x" / "$" prefixes both accepted
        CHECK(HexStrToInt(AnsiString("0xFF")) == 255);
        CHECK(HexStrToInt(AnsiString("$1F"))  == 31);
        // whitespace tolerated
        CHECK(HexStrToInt(AnsiString("  20  ")) == 0x20);
        // unparseable -> -1 ; empty -> -1
        CHECK(HexStrToInt(AnsiString("GZ"))   == -1);
        CHECK(HexStrToInt(AnsiString(""))     == -1);
    }

    // ===================================================================
    //  FindFirst / FindNext / FindClose + TSearchRec (BCB6 semantics)
    //  Build a temp dir tree, enumerate it, assert the BCB6 contract:
    //    FindFirst==0 on match, FindNext==0 while more, sr.Name/sr.Attr,
    //    faAnyFile matches dirs+files, attr==0 filters out directories.
    // ===================================================================
    {
        // constant values must match BCB6 SysUtils.hpp
        CHECK(faDirectory == 0x10);
        CHECK(faAnyFile   == 0x3F);
        CHECK(faReadOnly  == 0x01);
        CHECK(faArchive   == 0x20);

        AnsiString base("vclcompat_findtest_dir");
        RemoveDir(base + "\\sub");                 // best-effort pre-clean
        DeleteFile(base + "\\a.txt");
        DeleteFile(base + "\\b.dat");
        RemoveDir(base);
        CHECK(ForceDirectories(base));
        CHECK(CreateDir(base + "\\sub"));
        // two files
        { FILE* f = std::fopen((base + "\\a.txt").c_str(), "wb");
          if (f) { std::fputs("hi", f); std::fclose(f); } }
        { FILE* f = std::fopen((base + "\\b.dat").c_str(), "wb");
          if (f) { std::fputs("yo", f); std::fclose(f); } }

        // faAnyFile : enumerate everything (".", "..", sub, a.txt, b.dat)
        {
            int files = 0, dirs = 0, dots = 0;
            TSearchRec sr;
            int r = FindFirst(base + "\\*.*", faAnyFile, sr);
            CHECK(r == 0);                                  // BCB6: 0 == match
            if (r == 0) {
                do {
                    if (sr.Name == "." || sr.Name == "..") { ++dots; continue; }
                    if ((sr.Attr & faDirectory) != 0) ++dirs;
                    else                              ++files;
                } while (FindNext(sr) == 0);                // BCB6: loop while 0
                FindClose(sr);
            }
            CHECK(files == 2);                              // a.txt + b.dat
            CHECK(dirs  == 1);                              // sub
            CHECK(dots  == 2);                              // . and ..
        }

        // attr == 0 : directories (and . / ..) are filtered OUT; files only.
        {
            int files = 0, dirs = 0;
            TSearchRec sr;
            if (FindFirst(base + "\\*.*", 0, sr) == 0) {
                do {
                    if ((sr.Attr & faDirectory) != 0) ++dirs;
                    else                              ++files;
                } while (FindNext(sr) == 0);
                FindClose(sr);
            }
            CHECK(files == 2);
            CHECK(dirs  == 0);                              // sub + . + .. filtered
        }

        // no-match path : FindFirst returns non-zero (BCB6).
        {
            TSearchRec sr;
            CHECK(FindFirst(base + "\\nope_*.xyz", faAnyFile, sr) != 0);
            FindClose(sr);   // safe even on failed FindFirst
        }

        // cleanup
        DeleteFile(base + "\\a.txt");
        DeleteFile(base + "\\b.dat");
        RemoveDir(base + "\\sub");
        CHECK(RemoveDir(base));                             // dir now empty
    }

    // ===================================================================
    //  AI(W906-D024) 20261001: TRadioGroup::Items->Clear() leaves ItemIndex = -1 (VCL ItemsChange clamp; D-024)
    // ===================================================================
    {
        TRadioGroup rg;
        rg.Items->Add("Arm1"); rg.Items->Add("Arm2"); rg.Items->Add("Arm3");
        rg.ItemIndex = 2;
        rg.Items->Clear();
        CHECK(rg.ItemIndex == -1);                          // golden cContact.cpp:907 / :925 fall back on it
        CHECK(rg.Items->Count == 0);
        rg.Items->Add("A");
        CHECK(rg.ItemIndex == -1);                          // Add does not pick one
        rg.ItemIndex = 0;
        TStringList* base = rg.Items;
        base->Clear();                                      // through the TStringList* the port code holds
        CHECK(rg.ItemIndex == -1);
        rg.Items->Add("x"); rg.Items->Add("y"); rg.ItemIndex = 1;
        rg.Items->Text = "p\r\nq\r\nr";                  // batch setter: no Clear(), index still in range
        CHECK(rg.ItemIndex == 1 && rg.Items->Count == 3);
        TRadioGroup empty;                                   // default stays 0 (separate question)
        CHECK(empty.ItemIndex == 0);
        empty.Items->Clear();
        CHECK(empty.ItemIndex == -1);
    }

    // ===================================================================
    //  summary
    // ===================================================================
    std::printf("\nvclcompat harness: %d/%d checks passed\n",
                g_total - g_fail, g_total);
    if (g_fail != 0) {
        std::printf("RESULT: FAILED (%d failures)\n", g_fail);
        return 1;
    }
    std::printf("RESULT: OK\n");
    return 0;
}
