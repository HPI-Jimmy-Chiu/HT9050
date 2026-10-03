// =============================================================================
//  test_fpfast.cpp -- AI(W906-FPFAST) 20261003
//  ctest: FPFAST_Pin        argv[1] = port root (read only)
//
//  RULINGS_20261003 #7 (= NIGHT_REPORT s0 #77 A): the whole tree is compiled with
//  -fexcess-precision=fast (CMakeLists.txt:7, GNU C / C++ only) so that the machine's
//  WinLibs g++ 16.2 i686 evaluates `double == <decimal literal>` the way BCB6 and the
//  oracle g++ 6.3.0 do.  Without it, GCC >= 13 in strict -std=c++14 (CMAKE_CXX_EXTENSIONS
//  OFF) defaults to -fexcess-precision=standard on x87: the literal is evaluated in long
//  double and `d == 5.6` is false for a double d that holds 5.6 (docs/FP_ORACLE_FINDINGS.md
//  s1 / s3 / s9).  This test keeps anybody from dropping the line:
//
//   [R] RUNTIME, the BCB6 answers (bcc32 -Od, measured 20261003, FP_ORACLE_FINDINGS.md s9)
//       for the shapes the tree has: a double read from text / memory compared with 40.2,
//       5.6 or 26.67 (cContact / cinitial / fContact / adam6024 / Adam6024Pressure /
//       fHotPlate), the strict range `v<0.8 || v>5.2` at 5.2 (Adam6024Pressure_St02.cpp
//       :822), a computed value stored to a double then compared (adam6024 KYEC
//       `d = 28/10.0; d == 2.8`).  WinLibs g++ 16.2 WITHOUT the flag answers them the other
//       way (red).  The oracle g++ 6.3.0 is fast-only for C++ (`standard` is "sorry,
//       unimplemented"), so there [R] stays green with or without the line -- hence:
//   [S] SOURCE: <root>/CMakeLists.txt still carries ONE live
//       add_compile_options(... -fexcess-precision=fast ...) for C and for CXX, GNU only,
//       before the first target; no live -fexcess-precision=standard in either CMakeLists.
//   [X] x87 only: 80-bit intermediates BCB6 relies on are not changed by the flag
//       ((1e16+1)-1e16 == 1, (1e308*10)/10 == 1e308, 56 != 5.6*10 -- bcc32 measured the same);
//       red if someone "fixes" FP by moving to SSE math.
//
//  Every value goes through a volatile or a noinline function so the optimiser cannot fold
//  the comparison (a Release -O3 build must give the same answers).  No file is written.
//  CONTROLS (20261003, FP_ORACLE_FINDINGS.md s9.4): g++ 6.3.0 without the flag -> [R] green,
//  CMakeLists.txt copy without the line -> [S] red; WinLibs 16.2 without the flag -> [R] red.
// =============================================================================
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__GNUC__)
#define FPFAST_NOINLINE __attribute__((noinline))
#else
#define FPFAST_NOINLINE
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                                   \
    do {                                                                                   \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                          \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; }    \
    } while (0)

// cprod.h's DeviceForm_File shape: a global struct whose double fields come from the recipe text.
struct FpfastDeviceForm { int iPad; double dKitDiameter; double dDieForceKitDiameter; };
FpfastDeviceForm g_fpfastDeviceForm;

// ReadIniData(double) -> vclcompat ReadFloat -> std::strtod (vclcompat/IniFiles.cpp); recipes are %0.4f.
FPFAST_NOINLINE static double ReadFloatText(const char* s) { return std::strtod(s, nullptr); }
FPFAST_NOINLINE static double ViaParam(double x) { return x; }

static void PartR_Runtime()
{
    std::printf("PART R -- double vs decimal literal, the BCB6 answers\n");
    g_fpfastDeviceForm.dKitDiameter = ReadFloatText("40.2000");
    CHECK(g_fpfastDeviceForm.dKitDiameter == 40.2,
          "R1 DeviceForm.dKitDiameter (\"40.2000\") == 40.2 is TRUE (cContact / cinitial / fContact:1671 shape)");
    g_fpfastDeviceForm.dKitDiameter = ReadFloatText("5.6000");
    double fDiameter = g_fpfastDeviceForm.dKitDiameter;
    CHECK(fDiameter == 5.6, "R2 fDiameter = DeviceForm.dKitDiameter (\"5.6000\"); fDiameter == 5.6 is TRUE (adam6024 shape)");
    g_fpfastDeviceForm.dDieForceKitDiameter = ReadFloatText("40.2000");
    fDiameter = g_fpfastDeviceForm.dDieForceKitDiameter;
    CHECK(fDiameter == 40.2, "R3 fDiameter (\"40.2000\") == 40.2 is TRUE (Adam6024Pressure_St02.cpp:535 golden shape)");
    volatile double vd56 = 5.6;
    CHECK(vd56 == 5.6, "R4 volatile double vd = 5.6; vd == 5.6 is TRUE");
    volatile double vd402 = 40.2;
    CHECK(vd402 == 40.2, "R5 volatile double vd = 40.2; vd == 40.2 is TRUE");
    CHECK(ViaParam(40.2) == 40.2, "R6 ViaParam(40.2) == 40.2 is TRUE (FP_ORACLE_FINDINGS.md s3 via_param)");
    const double xpitch = ReadFloatText("26.67");
    CHECK(xpitch == 26.67, "R7 XPitch (\"26.67\") == 26.67 is TRUE (fHotPlate.cpp:234 shape)");
    const double v52 = ReadFloatText("5.2");
    CHECK(!(v52 < 0.8 || v52 > 5.2), "R8 v = 5.2: (v<0.8 || v>5.2) is FALSE (Adam6024Pressure_St02.cpp:822 golden range)");
    const double v08 = ReadFloatText("0.8");
    CHECK(!(v08 < 0.8 || v08 > 5.2), "R9 v = 0.8: (v<0.8 || v>5.2) is FALSE");
    volatile int i28 = 28;
    volatile double d28 = i28 / 10.0;                     // stored to a 64-bit double (BCB6 -Od -r- does that for every local)
    CHECK(d28 == 2.8, "R10 d = 28/10.0 stored; d == 2.8 is TRUE (adam6024 KYEC shape)");
}

static void PartX_X87()
{
#if FLT_EVAL_METHOD == 2
    std::printf("PART X -- x87 80-bit intermediates (FLT_EVAL_METHOD == 2): must not change with the flag\n");
    volatile double a = 1e16, b = 1.0;
    CHECK(((a + b) - a) == 1.0, "X1 (1e16 + 1) - 1e16 == 1 (64-bit mantissa intermediate; SSE would give 0)");
    volatile double big = 1e308;
    CHECK(((big * 10.0) / 10.0) == big, "X2 (1e308 * 10) / 10 == 1e308 (x87 exponent range; SSE would give inf)");
    volatile double dDia = 56.0, f56 = 5.6;
    CHECK(!(dDia == f56 * 10), "X3 56 == 5.6 * 10 is FALSE (80-bit product, adam6024 :450 shape; SSE would say TRUE)");
#else
    std::printf("PART X -- skipped: FLT_EVAL_METHOD = %d (not x87)\n", (int)FLT_EVAL_METHOD);
#endif
}

static bool ReadLines(const std::string& path, std::vector<std::string>& out)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        out.push_back(line);
    }
    return true;
}

// The live (non-comment) part of a CMake line: text before the first '#'.
static std::string Live(const std::string& s)
{
    const std::string::size_type h = s.find('#');
    return h == std::string::npos ? s : s.substr(0, h);
}

static bool Has(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }

static void PartS_Source(const std::string& root)
{
    std::printf("PART S -- CMakeLists.txt still carries the flag (root: %s)\n", root.c_str());
    std::vector<std::string> top, tst;
    const bool okTop = ReadLines(root + "/CMakeLists.txt", top);
    const bool okTst = ReadLines(root + "/tests/CMakeLists.txt", tst);
    CHECK(okTop, "S0 <root>/CMakeLists.txt readable");
    CHECK(okTst, "S0 <root>/tests/CMakeLists.txt readable");
    if (!okTop) return;

    int nFlagLines = 0, flagLine = -1, firstTarget = -1;
    for (int i = 0; i < (int)top.size(); ++i) {
        const std::string live = Live(top[i]);
        if (Has(live, "-fexcess-precision=fast")) { ++nFlagLines; if (flagLine < 0) flagLine = i; }
        if (firstTarget < 0 && (Has(live, "add_library(") || Has(live, "add_executable(") || Has(live, "add_subdirectory(")))
            firstTarget = i;
    }
    CHECK(nFlagLines == 1, "S1 exactly one live line with -fexcess-precision=fast in CMakeLists.txt");
    if (flagLine < 0) return;
    std::printf("       (CMakeLists.txt:%d)\n", flagLine + 1);
    const std::string fl = Live(top[flagLine]);
    std::string::size_type p = fl.find_first_not_of(" \t");
    CHECK(p != std::string::npos && fl.compare(p, 20, "add_compile_options(") == 0,
          "S2 it is a directory-wide add_compile_options( (every target inherits it)");
    const bool cxx = Has(fl, "$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU>>:-fexcess-precision=fast>") ||
                     Has(fl, "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fexcess-precision=fast>");
    const bool c   = Has(fl, "$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:GNU>>:-fexcess-precision=fast>") ||
                     Has(fl, "$<$<COMPILE_LANG_AND_ID:C,GNU>:-fexcess-precision=fast>");
    CHECK(cxx, "S3 ... for CXX, GNU only (MSVC, the second oracle, never sees it)");
    CHECK(c,   "S4 ... for C, GNU only");
    CHECK(firstTarget > flagLine, "S5 ... before the first add_library / add_executable / add_subdirectory");
    bool noStd = true;
    for (const std::string& s : top) if (Has(Live(s), "-fexcess-precision=standard")) noStd = false;
    for (const std::string& s : tst) if (Has(Live(s), "-fexcess-precision=standard")) noStd = false;
    CHECK(noStd, "S6 no live -fexcess-precision=standard in CMakeLists.txt / tests/CMakeLists.txt");
}

int main(int argc, char** argv)
{
#if defined(__VERSION__)
    std::printf("compiler: %s   FLT_EVAL_METHOD = %d\n", __VERSION__, (int)FLT_EVAL_METHOD);
#endif
    PartR_Runtime();
    PartX_X87();
    if (argc > 1) PartS_Source(argv[1]);
    else { std::printf("  FAIL: no argv[1] (port root) -- PART S not run\n"); ++g_fail; }
    std::printf("FPFAST_Pin: %d PASS / %d FAIL\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
