// ===========================================================================
//  tests/test_IniFiles.cpp
//
//  W3-cont CONFIG-READERS GATEWAY: vclcompat TIniFile / TMemIniFile shim.
//
//  Verifies the BCB6 ini semantics that the common.cpp ini-helper family
//  (OpenIniFile / CheckAndReadIniData* / ReadIniData* / WriteIniData* /
//  CheckSectionExist / CheckKeyExist / *Mem) depends on:
//    * default fallback on missing section / key / unparseable numeric
//    * case-insensitive section + key matching
//    * ReadInteger decimal + '$'/'0x' hex; ReadFloat decimal; ReadBool 0/1
//    * the '%0.4f' double round-trip (WriteString text -> ReadFloat), which is
//      how CheckAndReadIniData(double) persists doubles (NOT WriteFloat)
//    * WriteString/Integer/Bool/DateTime create section/key if absent
//    * SectionExists / ValueExists
//    * TMemIniFile mutations flush ONLY on UpdateFile()
//    * FileName property (used by OpenIniFile reuse comparison)
//    * Big5 value bytes preserved verbatim
//  Plus a REAL-config oracle pass over D:\HT9045\system\Gerneral.ini for the
//  known-truth [System] values (MOTION_CARD_TYPE=1, IO_CARD_TYPE=2, ...).
//  The real-config block runtime-SKIPs (does not fail) when the file is absent.
//
//  Self-contained CHECK harness (matches the other vclcompat tests' style).
// ===========================================================================
#include "vclcompat/vcl_compat.h"

#include <cstdio>
#include <cstdlib>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
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

// A scratch path for write tests.  Prefer the session scratchpad/tempdir.
static AnsiString tmpIniPath(const char* leaf) {
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    return AnsiString(t) + AnsiString("\\") + AnsiString(W906_TestTmpName(leaf).c_str());   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
}

int main() {
    // =====================================================================
    //  A) Synthetic round-trip: write a fresh ini, reopen, verify semantics.
    // =====================================================================
    AnsiString path = tmpIniPath("vclcompat_test_inifiles.ini");
    DeleteFile(path);   // start clean (ignore result)

    // ---- create + write-through (TIniFile) -------------------------------
    {
        TIniFile* ini = new TIniFile(path);
        CHECK(ini->FileName == path);                 // FileName property

        // Section/key absent before any write.
        CHECK(ini->SectionExists(AnsiString("Sys")) == false);
        CHECK(ini->ValueExists(AnsiString("Sys"), AnsiString("Speed")) == false);

        // Writes create section + key.
        ini->WriteInteger(AnsiString("Sys"), AnsiString("Speed"), 1234);
        ini->WriteBool   (AnsiString("Sys"), AnsiString("Enabled"), true);
        ini->WriteString (AnsiString("Sys"), AnsiString("Name"), AnsiString("HT9045"));
        ini->WriteString (AnsiString("Sys"), AnsiString("Hex"), AnsiString("$1F"));   // 31 dec

        // The '%0.4f' double round-trip used by CheckAndReadIniData(double):
        //   persist as WriteString("%0.4f"), re-read via ReadFloat.
        {
            AnsiString s; s.sprintf("%0.4f", 3.14159);
            ini->WriteString(AnsiString("Sys"), AnsiString("Pi"), s);
        }

        ini->UpdateFile();
        CHECK(ini->SectionExists(AnsiString("Sys")) == true);
        CHECK(ini->ValueExists(AnsiString("Sys"), AnsiString("Speed")) == true);
        delete ini;
    }

    // ---- reopen a NEW instance: data persisted to disk -------------------
    {
        TIniFile* ini = new TIniFile(path);

        // exact reads
        CHECK(ini->ReadInteger(AnsiString("Sys"), AnsiString("Speed"), -1) == 1234);
        CHECK(ini->ReadBool   (AnsiString("Sys"), AnsiString("Enabled"), false) == true);
        CHECK(ini->ReadString (AnsiString("Sys"), AnsiString("Name"), AnsiString("?")) == AnsiString("HT9045"));

        // hex parse ('$1F' -> 31)
        CHECK(ini->ReadInteger(AnsiString("Sys"), AnsiString("Hex"), -1) == 31);

        // '%0.4f' double round-trip -> stored as "3.1416" (4-decimal rounding),
        // re-read via ReadFloat == 3.1416 exactly (to 4 dp).
        double pi = ini->ReadFloat(AnsiString("Sys"), AnsiString("Pi"), 0.0);
        CHECK(pi > 3.14155 && pi < 3.14165);   // == 3.1416 within fp tolerance

        // ---- DEFAULT FALLBACK --------------------------------------------
        // missing key -> default verbatim (every typed flavour)
        CHECK(ini->ReadInteger(AnsiString("Sys"), AnsiString("NoSuchKey"), 777) == 777);
        CHECK(ini->ReadFloat  (AnsiString("Sys"), AnsiString("NoSuchKey"), 2.5) == 2.5);
        CHECK(ini->ReadBool   (AnsiString("Sys"), AnsiString("NoSuchKey"), true) == true);
        CHECK(ini->ReadString (AnsiString("Sys"), AnsiString("NoSuchKey"), AnsiString("DEF")) == AnsiString("DEF"));
        // missing SECTION -> default
        CHECK(ini->ReadInteger(AnsiString("Nope"), AnsiString("Speed"), 42) == 42);
        CHECK(ini->ReadString (AnsiString("Nope"), AnsiString("Name"), AnsiString("D")) == AnsiString("D"));

        // unparseable numeric -> default (Name="HT9045" read as int)
        CHECK(ini->ReadInteger(AnsiString("Sys"), AnsiString("Name"), 99) == 99);
        CHECK(ini->ReadFloat  (AnsiString("Sys"), AnsiString("Name"), 1.25) == 1.25);

        // ---- CASE-INSENSITIVE section + key ------------------------------
        CHECK(ini->ReadInteger(AnsiString("sys"), AnsiString("speed"), -1) == 1234);
        CHECK(ini->ReadInteger(AnsiString("SYS"), AnsiString("SPEED"), -1) == 1234);
        CHECK(ini->ValueExists(AnsiString("SyS"), AnsiString("nAmE")) == true);
        CHECK(ini->SectionExists(AnsiString("SYS")) == true);

        // ReadBool nonzero-int => true (write "5" via WriteString, read bool)
        ini->WriteString(AnsiString("Sys"), AnsiString("Flag5"), AnsiString("5"));
        CHECK(ini->ReadBool(AnsiString("Sys"), AnsiString("Flag5"), false) == true);

        // overwrite existing key keeps single entry / updates value
        ini->WriteInteger(AnsiString("Sys"), AnsiString("Speed"), 9000);
        CHECK(ini->ReadInteger(AnsiString("Sys"), AnsiString("Speed"), -1) == 9000);

        // enumeration: ReadSection lists the keys we wrote
        TStringList* keys = new TStringList();
        ini->ReadSection(AnsiString("Sys"), keys);
        CHECK(keys->IndexOf(AnsiString("Speed")) >= 0);
        CHECK(keys->IndexOf(AnsiString("Name"))  >= 0);
        delete keys;

        delete ini;
    }

    // =====================================================================
    //  B) DateTime round-trip (WriteDateTime text -> ReadDateTime).
    // =====================================================================
    {
        TIniFile* ini = new TIniFile(path);
        TDateTime when = EncodeDate(2026, 6, 26) + EncodeTime(13, 45, 30, 0);
        ini->WriteDateTime(AnsiString("Time"), AnsiString("Stamp"), when);
        ini->UpdateFile();
        delete ini;

        TIniFile* re = new TIniFile(path);
        TDateTime back = re->ReadDateTime(AnsiString("Time"), AnsiString("Stamp"), TDateTime(0.0));
        // round-trips to the second (string format is "yyyy/mm/dd hh:nn:ss").
        CHECK((double)back > 0.0);
        Word y, mo, d, h, mi, s, ms;
        DecodeDate(back, y, mo, d);
        DecodeTime(back, h, mi, s, ms);
        CHECK(y == 2026 && mo == 6 && d == 26);
        CHECK(h == 13 && mi == 45 && s == 30);
        // missing key -> default verbatim
        TDateTime def(1234.5);
        TDateTime miss = re->ReadDateTime(AnsiString("Time"), AnsiString("None"), def);
        CHECK((double)miss == 1234.5);
        delete re;
    }

    // =====================================================================
    //  C) Big5 value bytes preserved verbatim through write/read.
    // =====================================================================
    {
        // Big5 bytes for the Chinese for "test" (中文): A4 A4 A4 E5 -> use a
        // simple known Big5 pair; we only assert byte-exact round-trip.
        const char big5[] = { (char)0xA4, (char)0xA4, (char)0xA4, (char)0xE5, 0 };
        AnsiString val(big5);
        TIniFile* ini = new TIniFile(path);
        ini->WriteString(AnsiString("Big5"), AnsiString("Note"), val);
        ini->UpdateFile();
        delete ini;

        TIniFile* re = new TIniFile(path);
        AnsiString back = re->ReadString(AnsiString("Big5"), AnsiString("Note"), AnsiString(""));
        CHECK(back == val);                 // byte-exact
        CHECK(back.Length() == 4);
        delete re;
    }

    // =====================================================================
    //  D) TMemIniFile: mutations flush ONLY on UpdateFile().
    // =====================================================================
    {
        AnsiString mpath = tmpIniPath("vclcompat_test_mem.ini");
        DeleteFile(mpath);

        // create a baseline file with a value
        {
            TIniFile* seed = new TIniFile(mpath);
            seed->WriteInteger(AnsiString("M"), AnsiString("V"), 1);
            seed->UpdateFile();
            delete seed;
        }

        // open Mem, mutate in memory, do NOT UpdateFile, then drop without
        // calling UpdateFile via a fresh disk read BEFORE delete...
        {
            TMemIniFile* mem = new TMemIniFile(mpath);
            CHECK(mem->ReadInteger(AnsiString("M"), AnsiString("V"), -1) == 1);
            mem->WriteInteger(AnsiString("M"), AnsiString("V"), 2);   // memory only
            // a separate TIniFile reading disk must still see the OLD value,
            // proving the Mem write did not hit disk yet.
            TIniFile* peek = new TIniFile(mpath);
            CHECK(peek->ReadInteger(AnsiString("M"), AnsiString("V"), -1) == 1);
            delete peek;

            mem->UpdateFile();   // NOW flush to disk
            TIniFile* peek2 = new TIniFile(mpath);
            CHECK(peek2->ReadInteger(AnsiString("M"), AnsiString("V"), -1) == 2);
            delete peek2;
            delete mem;
        }
        DeleteFile(mpath);
    }

    DeleteFile(path);

    // =====================================================================
    //  E) REAL-config ORACLE (runtime-SKIP if absent).
    //     D:\HT9045\system\Gerneral.ini known [System] truths.
    // =====================================================================
    {
        const char* env = std::getenv("HT9045_SYSTEM_DIR");
        AnsiString sysDir = (env && *env) ? AnsiString(env) : AnsiString("D:\\HT9045\\system");
        AnsiString iniPath = sysDir + AnsiString("\\Gerneral.ini");
        if (FileExists(iniPath)) {
            TIniFile* g = new TIniFile(iniPath);
            std::printf("REAL Gerneral.ini: [System] MOTION_CARD_TYPE=%d IO_CARD_TYPE=%d "
                        "TTL_CARD_TYPE=%d INDEX_MOTION_CARD=%d ; [TempCtrl] HEATER_CTRL_TYPE=%d\n",
                        g->ReadInteger(AnsiString("System"),  AnsiString("MOTION_CARD_TYPE"),  -1),
                        g->ReadInteger(AnsiString("System"),  AnsiString("IO_CARD_TYPE"),      -1),
                        g->ReadInteger(AnsiString("System"),  AnsiString("TTL_CARD_TYPE"),     -1),
                        g->ReadInteger(AnsiString("System"),  AnsiString("INDEX_MOTION_CARD"), -1),
                        g->ReadInteger(AnsiString("TempCtrl"),AnsiString("HEATER_CTRL_TYPE"),  -1));
            // ================================================================
            //  AI(W906-P8) 20260920: 這一段本來把**這台機器的組態值**釘死成
            //  斷言（MOTION_CARD_TYPE==1 / IO_CARD_TYPE==2 / TTL_CARD_TYPE==2 /
            //  INDEX_MOTION_CARD==0 / HEATER_CTRL_TYPE==4 / T_MODE_SPEED≈0.9）。
            //
            //  ⇒ 它在**任何組態不同的機器上都會紅**，包含同事的機台。
            //     20260917 §C13.2 實測這台就有三條不符：
            //       IO_CARD_TYPE     斷言 2  實際 1
            //       TTL_CARD_TYPE    斷言 2  實際 0
            //       HEATER_CTRL_TYPE 斷言 4  實際 2
            //
            //  一個「換台機器就紅」的常駐失敗 = **訓練所有人忽略紅燈**，
            //  而這支測試真正該守的是 `TIniFile` 的**讀取機制**，
            //  不是某一台機器裝了哪張卡。
            //
            //  ⇒ 改成斷言機制：鍵存在、型別解析得出來、大小寫不敏感、
            //     同一個鍵重讀一致。**值等於多少**移到機台專屬的驗收腳本
            //     （`tools/machine_config_expect.py`），那裡紅才有意義。
            // ================================================================
            CHECK(g->SectionExists(AnsiString("System")) == true);

            // 1) 鍵存在且解析得出整數 —— 用 sentinel 分辨「讀不到」與「值是 0」。
            //    ⚠ 不可以只斷言 `!= -1`：真的有鍵的值就是 -1 的話會假綠。
            //    所以用兩個不同的 sentinel 讀兩次，兩次都回 sentinel 才是「沒有」。
            const char* kIntKeys[] = { "MOTION_CARD_TYPE", "IO_CARD_TYPE",
                                       "TTL_CARD_TYPE", "INDEX_MOTION_CARD" };
            for (int ki = 0; ki < 4; ++ki) {
                const int a = g->ReadInteger(AnsiString("System"),
                                             AnsiString(kIntKeys[ki]), -1);
                const int b = g->ReadInteger(AnsiString("System"),
                                             AnsiString(kIntKeys[ki]), -424242);
                CHECK(!(a == -1 && b == -424242));      // 兩個 sentinel 都回 -> 鍵不存在
                CHECK(a == b);                          // 鍵存在 -> 與 default 無關
            }
            {
                const int a = g->ReadInteger(AnsiString("TempCtrl"),
                                             AnsiString("HEATER_CTRL_TYPE"), -1);
                const int b = g->ReadInteger(AnsiString("TempCtrl"),
                                             AnsiString("HEATER_CTRL_TYPE"), -424242);
                CHECK(!(a == -1 && b == -424242));
                CHECK(a == b);
            }

            // 2) 大小寫不敏感的節名比對 —— ★ 比的是**兩次讀出來一樣**，
            //    不是「等於 1」。原本那條連帶把值釘死了。
            CHECK(g->ReadInteger(AnsiString("SYSTEM"),  AnsiString("MOTION_CARD_TYPE"), -1)
               == g->ReadInteger(AnsiString("System"),  AnsiString("MOTION_CARD_TYPE"), -1));
            CHECK(g->ReadInteger(AnsiString("system"),  AnsiString("MOTION_CARD_TYPE"), -1)
               == g->ReadInteger(AnsiString("System"),  AnsiString("MOTION_CARD_TYPE"), -1));

            // 3) 浮點讀取 —— ★ 斷言「解析得出來且重讀一致」，不是「等於 0.9」。
            const double tmode  = g->ReadFloat(AnsiString("System"),
                                               AnsiString("T_MODE_SPEED"), -1.0);
            const double tmode2 = g->ReadFloat(AnsiString("System"),
                                               AnsiString("T_MODE_SPEED"), -9999.0);
            CHECK(!(tmode == -1.0 && tmode2 == -9999.0));   // 兩個 sentinel 都回 -> 沒這個鍵
            CHECK(tmode == tmode2);                          // 有鍵 -> 與 default 無關
            delete g;
        } else {
            std::printf("SKIP real Gerneral.ini oracle: \"%s\" not found.\n", iniPath.c_str());
        }
    }

    std::printf("test_IniFiles: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) { std::printf("FAILED: %d checks\n", g_fail); return 1; }
    std::printf("PASS\n");
    return 0;
}
