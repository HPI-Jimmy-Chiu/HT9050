// ===========================================================================
//  tests/test_w7_f2_sckart_state.cpp
//  W7-F2 -- SckArt state: merge proof + golden-divergence characterization.
//
//  ATTRIBUTION (added AI(W906-W7-F2fix2) 20260731 -- until now the file carried
//  only fix-round tags, so it read as though the fix rounds had authored PARTs A
//  and B).  Everything below landed in a single git commit (f61e25e), so this
//  split is taken from the AI() tags, not from history:
//    * PARTs A and B, and the file itself -- wave W7-F2, AI(W906-W7-F2) 20260729:
//      the same wave that made SckArtRemainderState derive from SckArtState, and
//      the tag on this target's own block in tests/CMakeLists.txt.
//    * PART C -- AI(W906-W7-F2fix) 20260729 (review round 2).
//    * PART C parser hardening, label softening and the claim corrections marked
//      below -- AI(W906-W7-F2fix2) 20260731 (review round 3).
//    * PART C parser round 2 (string-literal-aware comment stripping, clang-format
//      anchor tolerance, hex/suffix literals, untruncated diagnostics) and the
//      disclosure corrections that go with them -- AI(W906-W7-F2fix3) 20260801
//      (review round 4).  No assertion was added or removed: still 46 calls.
//
//  Two jobs, deliberately in one target because they are two halves of the same
//  analysis (docs/W7_UI_ARCHITECTURE_PLAN.md SS3-C4 / SS6-F2 / SS9-R9):
//
//  PART A -- MERGE PROOF.  W7-F2 made SckArtRemainderState derive from
//    SckArtState instead of re-declaring the 9 golden TfSCKART fields the two
//    had in common.  The merge is only behaviour-neutral if the base ctor
//    produces exactly the values the derived ctor used to produce, so every one
//    of those 9 is asserted here on BOTH types.  If a later wave changes either
//    ctor, this fails.
//
//  PART B -- GOLDEN-DIVERGENCE CHARACTERIZATION.  Diffing the ported SckArt
//    copies field by field against golden turned up two places where the ported
//    tree does NOT match golden.  Plan SS6-F2 says such a find is REPORTED, not
//    silently reconciled, so W7-F2 changed no values -- it pinned the current
//    (diverging) values here instead.  Each assertion below states the golden
//    value in its message.  THESE ARE NOT "CORRECT" EXPECTATIONS: a future wave
//    that fixes the divergence MUST flip the constant here in the same commit.
//    Failing to notice is exactly what this file exists to prevent.
//
//    D1  iLOTSTATUS_* status codes.  Golden's TfSCKART ctor assigns the whole
//        family explicitly -- golden Automation/SCK_ART.cpp:43-49:
//            iLOTSTATUS_NONE=0  _W=1  _T=2  _L=3  _R=4  _F=5  _A=6
//        csystem.cpp's two seams zero-initialise the four they carry
//        (W7C1's iLOTSTATUS_L, W7C2's iLOTSTATUS_W/_R/_A).
//
//    D2  iTesterType.  Golden Automation/SCK_ART.cpp:42 sets 0, then :111-114
//        `else { iTesterType=1; }` -- so golden constructs 1 for every
//        CUSTOMER_CODE except CC_SCK.  All ported copies hardcode 0.
//
//  PART C -- THE SEAM CONSTANTS THEMSELVES (AI(W906-W7-F2fix) 20260729, added by
//    the review track).  PART B alone was NOT a pin on D1/D2, even though this
//    file's own header and two csystem.cpp comment blocks said it was.  PART B
//    builds its own local SckArtState and feeds SckArt_SetLotStatus a literal 0
//    or 3 -- a true statement about that function, but one that stays green no
//    matter what csystem.cpp's seams construct.  The SIX initialisers that
//    actually carry D1 and D2 had NO observer at all, so a later wave could have
//    "fixed" or further broken a real behavioural divergence with the whole
//    suite green.  PART C closes that: it reads csystem.cpp's own source text
//    and asserts each of the six.  AI(W906-W7-F2fix2) 20260731: PART C's parser
//    was hardened after a mutation run found four ways to change a seam value and
//    keep it green; see PART C's own block for exactly which perturbations redden
//    it, which reflows deliberately do not, and what a text scanner still cannot
//    see (that block also withdraws the claim that no runtime observation exists
//    -- a hard one does, it is just not cheap).
//
//  ALSO PINNED: SckArt_SetLotStatus's full 0..7 mapping, which IS a faithful
//  translation of golden :639-667 and is the thing D1 corrupts by feeding it the
//  wrong input.  Keeping both in one file makes the interaction visible.
//
//  HOW MANY CHECKS THIS FILE REALLY CONTRIBUTES (AI(W906-W7-F2fix2) 20260731 --
//  the summary line at the bottom prints CHECK CALLS, and a reviewer read it as
//  independent coverage).  Re-counted by hand this round: 46 calls (40 CHECK +
//  6 PIN_SEAM), of which
//    * 2 are strictly redundant: B9 re-runs SckArt_SetLotStatus(3) and B10
//      re-runs SckArt_SetLotStatus(0), both already asserted by B3 and B7 (B3/B7
//      also assert the combo out-param, so B9/B10 are strict subsets).  They are
//      kept because they state the COST of D1 next to the divergence, not for
//      coverage;
//    * 9 assert on the DERIVED type a base initialiser its base-type twin already
//      asserts -- A2/A4/A6/A8/A10/A12/A14/A16 and B12.  That duplication IS the
//      merge proof (it is what would catch a derived ctor re-declaring one of the
//      9 fields), but it constrains no additional value.
//  So: 46 calls, 35 independent facts.  Do not quote 46 as coverage.
//
//  Toolchain: MinGW g++, C++14. Non-zero exit on any failure. No window, no
//  globals mutated beyond the local state objects.  PART C reads ONE file
//  (csystem.cpp) read-only; nothing is written anywhere.
// ===========================================================================
#include "Automation/SCK_ART.h"
#include "Automation/SCK_ART_Remainder.h"

#include <cstdio>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

static int g_fail  = 0;
static int g_total = 0;

static void check(bool ok, const char* what, const char* file, int line)
{
    ++g_total;
    if (!ok) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n", file, line, what);
    }
}
#define CHECK(cond, what) check((cond), (what), __FILE__, __LINE__)

// ===========================================================================
//  PART C support -- read csystem.cpp and parse a ctor initialiser out of it.
//  AI(W906-W7-F2fix) 20260729.
//
//  AI(W906-W7-F2fix2) 20260731 -- HARDENED.  An independent mutation run applied
//  26 perturbations to csystem.cpp against the 20260729 version of this parser.
//  It reddened on all six direct golden flips and on several structural edits,
//  but it stayed GREEN at 46/46 on four perturbations that had genuinely changed
//  the constructed value.  All four are closed below; what is still NOT closed is
//  disclosed in PART C's own block (search "STILL NOT CLOSED"), not papered over.
//    H1  ctor-BODY assignment (`W7C1_TfSCKARTSeam():iTesterType(0),...{ iTesterType=1; }`).
//        The scanned region stopped at the first `{`, so the body was invisible --
//        and body assignment is GOLDEN's OWN SHAPE (golden Automation/SCK_ART.cpp:38-55
//        assigns iTesterType/iLOTSTATUS_* in the ctor body, there is no mem-initialiser
//        list at all), i.e. exactly the edit a future reconciling wave would write.
//        CLOSED: the ctor body is now extracted by brace matching and an assignment
//        to the pinned field there fails the pin.
//    H2  comment-anchor collision.  `<Ctor>():` occurs TWICE per seam -- once in the
//        explanatory comment above the ctor, once in the ctor -- and comment stripping
//        ran AFTER the substring was cut, so it could not see the `//` that opened the
//        comment line.  The parser therefore read the COMMENT, and the documented,
//        invited edit (change the ctor to golden AND update the adjacent comment)
//        stayed green.  CLOSED: comments are stripped FILE-WIDE first, and the search
//        is anchored on the `struct` declaration before the ctor is looked for.
//    H3  realistic rename (struct + ctor renamed together, still compiling) stayed
//        green while the pin's prose still named a type that no longer existed.
//        CLOSED by the struct-declaration anchor: the name is now required to exist.
//    H4  `iTesterType(0+1)` passed -- the value parser read the first digit run and
//        stopped.  CLOSED: the whole parenthesised expression is parsed and must be
//        nothing but an integer literal.
//  Also fixed, a false-alarm the same run measured: a newline between `(` and the
//  literal used to go red on a value-preserving reflow.  Whitespace after `(` now
//  includes newlines and carriage returns.
//
//  AI(W906-W7-F2fix3) 20260801 -- HARDENED AGAIN.  A second independent prover ran 31
//  mutations against the 20260731 parser, confirmed H1-H4 stay closed and that all six
//  seam constants are individually pinned, and found one more silent hole plus three
//  false alarms.  All four are closed below; every one was reproduced and re-measured
//  here before and after the fix (see each item).
//    H5  H1 RE-OPENED BY ONE CHARACTER -- the comment stripper was not string-literal
//        aware, so a `//` inside a STRING LITERAL on the same line as a ctor-body
//        assignment deleted the rest of that line, assignment included, before
//        w7f2_bodyAssignsField could see it:
//            ...iOutputJamCnt(0){ const char* kSrv = "http://ART"; (void)kSrv; iTesterType = 1; }
//        passed 46/46, while the same body with "http:/ART" (ONE slash, same constructed
//        value 1) failed 1/46.  That is the H1 class -- ctor-body assignment, golden's own
//        shape -- back again.  CLOSED: w7f2_stripComments is now a state machine that
//        skips `"..."` and `'...'` (backslash escapes included).
//    F1  The anchor was the hard-coded literal `<Ctor>():`, so BOTH standard
//        clang-format constructor-initializer styles reddened the pin although neither
//        changes a value: `Ctor() : ...` (space before colon, 2/46) and `Ctor()\n  : ...`
//        (BreakConstructorInitializers, 2/46).  "Somebody runs a formatter over
//        csystem.cpp" is the likeliest accidental trigger there is.  CLOSED: whitespace
//        and newlines between `()` and `:` are skipped.
//    F2  `iTesterType(0x0)` and `iTesterType(0u)` reddened, both value-preserving.
//        CLOSED by ACCEPTING them: hex, octal and the u/U/l/L suffixes are now evaluated
//        with C++'s own base rules (so `010` is 8, not 10).  Accepting is the right call
//        rather than disclosing, because the parser still refuses everything that is not
//        a single literal -- `0x1` and `3L` still redden on VALUE, measured -- so the
//        pin's reach is unchanged while a whole class of false alarms disappears.
//    F3  The failure diagnostic was built into a `char buf[512]`; C5's and C6's labels
//        are ~470 chars, so every C5/C6 value failure was truncated mid-word and the
//        "expected N" the maintainer needs was cut off.  CLOSED: std::ostringstream.
//  One thing the prover reported that turned out NOT to be a hole: the disclosed item
//  "`/* */` comments are not stripped" was harmless for the H1 shape (`/* set */
//  iTesterType = 1;` reddens correctly, re-measured).  It is stripped now anyway --
//  once quotes are tracked, an apostrophe inside an unstripped block comment would open
//  a bogus char literal -- so that disclosure is retired rather than reworded.
// ===========================================================================

//  Locate the ported tree root from THIS file's own absolute path.  Verified,
//  not assumed: with the "MinGW Makefiles" generator CMake invokes g++ with the
//  absolute source path (checked in the generated
//  tests/CMakeFiles/test_w7_f2_sckart_state.dir/build.make of a fresh
//  configure), so __FILE__ is `<root>\tests\test_w7_f2_sckart_state.cpp`.
//  Deriving the path this way instead of from a target_compile_definitions
//  keeps the fix inside this file -- tests/CMakeLists.txt is owned by another
//  track this round and must not be touched.
static std::string w7f2_portedRoot()
{
    std::string self(__FILE__);
    std::string::size_type a = self.find_last_of("/\\");          // strip filename
    if (a == std::string::npos) return std::string();
    std::string dir = self.substr(0, a);
    std::string::size_type b = dir.find_last_of("/\\");           // strip "tests"
    if (b == std::string::npos) return std::string();
    return dir.substr(0, b);
}

static bool w7f2_readFile(const std::string &path, std::string &out)
{
    std::ifstream f(path.c_str(), std::ios::in | std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return !out.empty();
}

static bool w7f2_isIdentChar(char ch)
{
    return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

//  Strip comments so that nothing below can read one by accident.
//  AI(W906-W7-F2fix2) 20260731: applied ONCE to the WHOLE file before anything is
//  searched for (hole H2), not to an already-extracted substring -- a substring that
//  starts inside a comment line no longer carries the `//` that opened it, so stripping
//  afterwards is blind to it.
//  AI(W906-W7-F2fix3) 20260801 -- REWRITTEN AS A STATE MACHINE, string-literal aware
//  (hole H5).  The line-comment-only version treated `//` inside a string literal as a
//  comment start, which silently deleted a ctor-body assignment on the same line and so
//  re-opened H1; see this section's header for the two one-character-apart mutations that
//  proved it.  `"..."` and `'...'` are skipped, backslash escapes included, and `/* */`
//  is stripped too (necessary once quotes are tracked: an apostrophe inside an unstripped
//  block comment would otherwise open a bogus char literal and swallow real code).
//  Comment bodies are dropped but their newlines are kept, so `//` detection on following
//  lines is unaffected.
//  Not handled, and disclosed rather than papered over: C++11 raw string literals
//  (`R"(...)"`) -- none exist in this tree (BCB6-era source).  See PART C's residual list
//  for the two shapes that were actually tried.
static std::string w7f2_stripComments(const std::string &s)
{
    enum State { CODE, LINE, BLOCK, DQ, SQ };
    State st = CODE;
    std::string r;
    r.reserve(s.size());
    for (std::string::size_type i = 0; i < s.size(); ++i) {
        const char ch = s[i];
        const char nx = (i + 1 < s.size()) ? s[i + 1] : '\0';
        if (st == LINE) {
            if (ch == '\n') { st = CODE; r += '\n'; }
        } else if (st == BLOCK) {
            if (ch == '\n')                  r += '\n';
            else if (ch == '*' && nx == '/') { st = CODE; ++i; }
        } else if (st == DQ || st == SQ) {
            r += ch;
            if (ch == '\\' && i + 1 < s.size())                    { r += nx; ++i; }
            else if ((st == DQ && ch == '"') || (st == SQ && ch == '\'')) st = CODE;
        } else {                                                   // CODE
            if      (ch == '/' && nx == '/') { st = LINE;  ++i; }
            else if (ch == '/' && nx == '*') { st = BLOCK; ++i; }
            else if (ch == '"')              { st = DQ; r += ch; }
            //  A `'` straight after a DIGIT is a C++14 digit separator (`1'000`), not the
            //  start of a char literal -- do not open SQ on it.  Deliberately narrower than
            //  "after any identifier character": that wider test would also have refused to
            //  open on the prefixed char literals `L'x'` / `u8'a'`, which are real literals.
            else if (ch == '\'' &&
                     !(i > 0 && std::isdigit(static_cast<unsigned char>(s[i - 1])) != 0))
                                             { st = SQ; r += ch; }
            else                             { r += ch; }
        }
    }
    return r;
}

//  Whole-identifier occurrence of `name` at `pos`? (so `iTesterType` does not match
//  inside `core.iTesterTypeBackup`, and `iLOTSTATUS_R` not inside `iLOTSTATUS_RX`).
static bool w7f2_isIdentAt(const std::string &s, std::string::size_type pos, const std::string &name)
{
    if (pos > 0 && w7f2_isIdentChar(s[pos - 1])) return false;
    const std::string::size_type end = pos + name.size();
    if (end < s.size() && w7f2_isIdentChar(s[end])) return false;
    return true;
}

static std::string::size_type w7f2_skipSpace(const std::string &s, std::string::size_type p)
{
    //  Newlines/CRs included on purpose: a value-preserving reflow that puts the
    //  literal on the next line must NOT redden (measured false alarm, 20260731).
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\n' || s[p] == '\r')) ++p;
    return p;
}

//  The WHOLE parenthesised initialiser must be an optionally-signed integer literal
//  and nothing else (hole H4: `0+1` used to parse as 0).  `kGolden`, `0+1`, `1-1` and
//  `iLOTSTATUS_L` all fail here, which is the intended outcome -- the pin's promise is
//  only meaningful for a literal it can actually evaluate.
//  AI(W906-W7-F2fix3) 20260801 (false alarm F2): hex `0x0`, octal `010` and the u/U/l/L
//  suffixes are ACCEPTED now and evaluated with C++'s own base rules.  They used to
//  redden although all of them are value-preserving.  Evaluating rather than merely
//  tolerating them matters: `010` is EIGHT in C++, and a base-10 reading would have been
//  a wrong value, not just a loud one.  `0x1` / `3L` still redden -- on value, measured.
static bool w7f2_literalFromExpr(const std::string &expr, long &out)
{
    std::string t;
    for (std::string::size_type i = 0; i < expr.size(); ++i)
        if (expr[i] != ' ' && expr[i] != '\t' && expr[i] != '\n' && expr[i] != '\r') t += expr[i];
    if (t.empty()) return false;
    std::string::size_type i = 0;
    bool neg = false;
    if (t[0] == '+' || t[0] == '-') { neg = (t[0] == '-'); i = 1; }

    //  Trailing integer-suffix: at most 3 of u/U/l/L in any order (`0u`, `3L`, `1uLL`).
    std::string::size_type end = t.size();
    int nSuffix = 0;
    while (end > i && nSuffix < 3) {
        const char c = t[end - 1];
        if (c == 'u' || c == 'U' || c == 'l' || c == 'L') { --end; ++nSuffix; }
        else break;
    }
    if (end <= i) return false;

    int base = 10;
    if (end - i > 2 && t[i] == '0' && (t[i + 1] == 'x' || t[i + 1] == 'X')) { base = 16; i += 2; }
    else if (end - i > 1 && t[i] == '0')                                    { base = 8; }

    long v = 0;
    for (; i < end; ++i) {
        const char c = t[i];
        int d;
        if      (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = 10 + (c - 'a');
        else if (c >= 'A' && c <= 'F') d = 10 + (c - 'A');
        else return false;
        if (d >= base) return false;
        v = v * base + d;
    }
    out = neg ? -v : v;
    return true;
}

//  Extract the brace-matched ctor body starting at the `{` at `open`.
static bool w7f2_braceBody(const std::string &src, std::string::size_type open, std::string &out)
{
    int depth = 0;
    for (std::string::size_type i = open; i < src.size(); ++i) {
        if (src[i] == '{') ++depth;
        else if (src[i] == '}') {
            --depth;
            if (depth == 0) { out = src.substr(open + 1, i - (open + 1)); return true; }
        }
    }
    return false;
}

//  Hole H1.  Does the ctor BODY assign `field`, i.e. is the mem-initialiser value no
//  longer the constructed value?  Assignment in the ctor body is GOLDEN's own shape,
//  so this is the single most likely way for a future wave to change one of these six
//  values.  Deliberately conservative: `=` (not `==`), the compound assignments, and
//  `++`/`--`/`<<=`/`>>=` all count.  A read (`x=field;`, `field==1`) does not.
static bool w7f2_bodyAssignsField(const std::string &body, const std::string &field)
{
    std::string::size_type f = body.find(field);
    while (f != std::string::npos) {
        if (w7f2_isIdentAt(body, f, field)) {
            const std::string::size_type p = w7f2_skipSpace(body, f + field.size());
            const char c0 = (p     < body.size()) ? body[p]     : '\0';
            const char c1 = (p + 1 < body.size()) ? body[p + 1] : '\0';
            const char c2 = (p + 2 < body.size()) ? body[p + 2] : '\0';
            if (c0 == '=' && c1 != '=') return true;                                   // field = ...
            if (c1 == '=' && (c0 == '+' || c0 == '-' || c0 == '*' || c0 == '/' ||
                              c0 == '%' || c0 == '&' || c0 == '|' || c0 == '^'))
                return true;                                                            // field += ... etc
            if ((c0 == '+' && c1 == '+') || (c0 == '-' && c1 == '-')) return true;      // field++ / field--
            if ((c0 == '<' && c1 == '<' && c2 == '=') ||
                (c0 == '>' && c1 == '>' && c2 == '=')) return true;                     // field <<= / >>=
        }
        f = body.find(field, f + 1);
    }
    return false;
}

//  Return the mem-initialiser value for `field` in `ctorName`'s init-list, having
//  first checked that the ctor body does not then re-assign it.
//  `src` MUST already be comment-stripped (w7f2_stripComments, file-wide).
//  Anchored BY SYMBOL NAME, never by line number (plan SS5 / KNOWLEDGE Gotcha 10:
//  ported line numbers rot immediately).  Fails loudly -- and therefore turns the
//  test red -- if the struct, the ctor, the init-list, the field, a plain integer
//  literal, or a body free of assignments to `field` cannot be found, so PART C can
//  never silently pass.
static bool w7f2_ctorInitValue(const std::string &src, const std::string &ctorName,
                               const std::string &field, long &out, std::string &why)
{
    //  Anchor 1: the STRUCT DECLARATION (hole H2/H3).  Required as a whole identifier
    //  so `struct W7C1_TfSCKARTSeamV2` does not satisfy an anchor on W7C1_TfSCKARTSeam.
    const std::string decl = "struct " + ctorName;
    std::string::size_type s = src.find(decl);
    while (s != std::string::npos &&
           s + decl.size() < src.size() && w7f2_isIdentChar(src[s + decl.size()]))
        s = src.find(decl, s + 1);
    if (s == std::string::npos) {
        why = "`" + decl + "` is not declared in csystem.cpp (renamed? removed?)";
        return false;
    }

    //  Anchor 2: the ctor's init-list, AFTER the struct declaration.
    //  AI(W906-W7-F2fix3) 20260801 (false alarm F1): this used to be the hard-coded
    //  literal `<Ctor>():` with no tolerance at all, so BOTH standard clang-format
    //  constructor-initializer styles -- `Ctor() : ...` and `Ctor()\n    : ...`
    //  (BreakConstructorInitializers) -- reddened the pin without changing any value.
    //  Whitespace and newlines between `()` and `:` are skipped now.  A `::` is still
    //  rejected: that is a qualified name, not a mem-initialiser list.
    const std::string head = ctorName + "()";
    std::string::size_type c = std::string::npos;
    for (std::string::size_type h = src.find(head, s); h != std::string::npos;
         h = src.find(head, h + 1)) {
        if (!w7f2_isIdentAt(src, h, ctorName)) continue;
        const std::string::size_type p = w7f2_skipSpace(src, h + head.size());
        if (p < src.size() && src[p] == ':' && !(p + 1 < src.size() && src[p + 1] == ':')) {
            c = h;
            break;
        }
    }
    if (c == std::string::npos) {
        why = "no `" + ctorName + "() : ...` mem-initialiser list after the `" + decl +
              "` declaration";
        return false;
    }
    std::string::size_type open = src.find('{', c);
    if (open == std::string::npos) {
        why = "no ctor body `{` after `" + ctorName + "() :`";
        return false;
    }
    const std::string region = src.substr(c, open - c);

    std::string::size_type f = region.find(field);
    while (f != std::string::npos) {
        std::string::size_type p = w7f2_skipSpace(region, f + field.size());
        if (w7f2_isIdentAt(region, f, field) && p < region.size() && region[p] == '(') {
            int depth = 1;
            std::string::size_type q = p + 1;
            for (; q < region.size(); ++q) {
                if (region[q] == '(') ++depth;
                else if (region[q] == ')') { --depth; if (depth == 0) break; }
            }
            if (depth != 0) {
                why = ctorName + "'s `" + field + "(` initialiser has no closing `)` before the ctor body";
                return false;
            }
            const std::string expr = region.substr(p + 1, q - (p + 1));
            long v = 0;
            if (!w7f2_literalFromExpr(expr, v)) {
                why = ctorName + "'s `" + field + "(" + expr + ")` initialiser is not a plain integer literal";
                return false;
            }
            std::string body;
            if (!w7f2_braceBody(src, open, body)) {
                why = ctorName + "'s ctor body has no matching `}`";
                return false;
            }
            if (w7f2_bodyAssignsField(body, field)) {
                why = ctorName + "'s ctor BODY assigns `" + field + "` after the init-list, so the "
                      "init-list value (" + (v == 0 ? std::string("0") : std::string("non-zero")) +
                      ") is no longer the constructed value -- that is golden's own shape "
                      "(golden Automation/SCK_ART.cpp:38-55 assigns these in the body), so if this is "
                      "a deliberate reconciliation, move the pin to the assigned value";
                return false;
            }
            out = v;
            return true;
        }
        f = region.find(field, f + 1);
    }
    why = "`" + field + "` is not in " + ctorName + "'s mem-initialiser list";
    return false;
}

//  One PART C pin: assert the seam initialiser still holds `expectPorted`.
static void w7f2_pinSeamInit(const std::string &src, const char *ctorName,
                             const char *field, long expectPorted,
                             const char *label, const char *file, int line)
{
    long v = -12345;
    std::string why;
    if (!w7f2_ctorInitValue(src, ctorName, field, v, why)) {
        std::string msg = std::string(label) + "  COULD NOT BE CHECKED: " + why +
                          ".  This pin exists because nothing else observes that initialiser -- "
                          "re-anchor it, do not delete it.";
        check(false, msg.c_str(), file, line);
        return;
    }
    if (v != expectPorted) {
        //  AI(W906-W7-F2fix3) 20260801 (false alarm F3): this was a `char buf[512]` +
        //  snprintf.  C5's and C6's labels are ~470 characters, so every C5/C6 VALUE
        //  failure was truncated mid-word and the `expected N` -- the one number the
        //  maintainer actually needs -- was cut off.  Built with ostringstream now, so
        //  the message cannot be clipped however long a label grows.
        std::ostringstream m;
        m << label << "  -- source says " << ctorName << "::" << field << "(" << v
          << "), this pin expected " << expectPorted;
        const std::string msg = m.str();
        check(false, msg.c_str(), file, line);
        return;
    }
    check(true, label, file, line);
}
#define PIN_SEAM(src, ctor, field, expect, label) \
    w7f2_pinSeamInit((src), (ctor), (field), (expect), (label), __FILE__, __LINE__)

int main()
{
    // =====================================================================
    //  PART A -- merge proof: the 9 formerly-duplicated fields construct
    //  identically on the base and on the derived type.
    //  (Field list and golden line numbers: golden Automation/SCK_ART.h:236
    //  sLOTSTATUS, :251 iCurrentStatus, :249 iTesterType, :245 iInputCount,
    //  :248 iFTRTCount, :250 iManualRejectCnt, :285 iNeedRT, :261 dCurrYield,
    //  :263 iCurrent93KARTStep.)
    // =====================================================================
    SckArtState          base;
    SckArtRemainderState rem;

    CHECK(base.sLOTSTATUS         == AnsiString("NONE"), "A1  SckArtState.sLOTSTATUS ctor value is \"NONE\" (golden SCK_ART.cpp:51)");
    CHECK(rem.sLOTSTATUS          == AnsiString("NONE"), "A2  SckArtRemainderState inherits the same \"NONE\" (merge must not change it)");
    CHECK(base.iCurrentStatus     == 0,                  "A3  SckArtState.iCurrentStatus ctor value is 0 (== golden iLOTSTATUS_NONE)");
    CHECK(rem.iCurrentStatus      == 0,                  "A4  SckArtRemainderState.iCurrentStatus matches the base");
    CHECK(base.iInputCount        == 0,                  "A5  SckArtState.iInputCount ctor value is 0");
    CHECK(rem.iInputCount         == 0,                  "A6  SckArtRemainderState.iInputCount matches the base");
    CHECK(base.iFTRTCount         == 0,                  "A7  SckArtState.iFTRTCount ctor value is 0");
    CHECK(rem.iFTRTCount          == 0,                  "A8  SckArtRemainderState.iFTRTCount matches the base");
    CHECK(base.iManualRejectCnt   == 0,                  "A9  SckArtState.iManualRejectCnt ctor value is 0");
    CHECK(rem.iManualRejectCnt    == 0,                  "A10 SckArtRemainderState.iManualRejectCnt matches the base");
    CHECK(base.iNeedRT            == 0,                  "A11 SckArtState.iNeedRT ctor value is 0");
    CHECK(rem.iNeedRT             == 0,                  "A12 SckArtRemainderState.iNeedRT matches the base");
    CHECK(base.dCurrYield         == 0.0,                "A13 SckArtState.dCurrYield ctor value is 0.0");
    CHECK(rem.dCurrYield          == 0.0,                "A14 SckArtRemainderState.dCurrYield matches the base");
    CHECK(base.iCurrent93KARTStep == 0,                  "A15 SckArtState.iCurrent93KARTStep ctor value is 0");
    CHECK(rem.iCurrent93KARTStep  == 0,                  "A16 SckArtRemainderState.iCurrent93KARTStep matches the base");

    // The merge is only useful if the sibling free functions can now be called
    // on a remainder state through the base reference. Prove the binding works
    // and that it writes through to the derived object (no slicing).
    SckArt_SetLotStatus(rem, 4);
    CHECK(rem.sLOTSTATUS    == AnsiString("LOTSTATUS_R"), "A17 SckArt_SetLotStatus() binds a SckArtRemainderState& to SckArtState& and writes through");
    CHECK(rem.iCurrentStatus == 4,                        "A18 ...and the derived object sees iCurrentStatus, i.e. nothing was sliced");

    // Derived-only fields still construct as before the merge.
    SckArtRemainderState rem2;
    CHECK(rem2.iCurrentFlexARTStep == 0,   "A19 derived-only iCurrentFlexARTStep still 0 (golden SCK_ART.h:264)");
    CHECK(rem2.iLotCount           == 0,   "A20 derived-only iLotCount still 0 (golden SCK_ART.h:246)");
    CHECK(rem2.iInputJamCnt        == 0,   "A21 derived-only iInputJamCnt still 0 (golden SCK_ART.cpp:54)");
    CHECK(rem2.iOutputJamCnt       == 0,   "A22 derived-only iOutputJamCnt still 0 (golden SCK_ART.cpp:55)");
    CHECK(rem2.sSetupFilePath      == AnsiString(""), "A23 derived-only sSetupFilePath still \"\" (golden SCK_ART.cpp:40)");
    CHECK(rem2.sLotStartTime       == AnsiString(""), "A24 derived-only sLotStartTime still \"\" (golden SCK_ART.cpp:41)");

    // The 3 base fields the remainder never used are inherited and inert.
    CHECK(rem2.bLdCntExdInputCnt  == false, "A25 inherited-but-unused bLdCntExdInputCnt is false (golden SCK_ART.h:350)");
    CHECK(rem2.bBackUpInArmMode   == false, "A26 inherited-but-unused bBackUpInArmMode is false (golden SCK_ART.h:279)");
    CHECK(rem2.bBackUpOutArmMode  == false, "A27 inherited-but-unused bBackUpOutArmMode is false (golden SCK_ART.h:280)");

    // =====================================================================
    //  PART B(i) -- the faithful half: SckArt_SetLotStatus's mapping.
    //  golden Automation/SCK_ART.cpp:639-667. This is CORRECT and must stay.
    // =====================================================================
    {
        SckArtState st;
        int combo = -99;

        SckArt_SetLotStatus(st, 1, &combo);
        CHECK(st.sLOTSTATUS == AnsiString("LOTSTATUS_W") && st.iCurrentStatus == 1 && combo == 1,
              "B1  SetLotStatus(1) -> \"LOTSTATUS_W\" (golden :643-645)");
        SckArt_SetLotStatus(st, 2, &combo);
        CHECK(st.sLOTSTATUS == AnsiString("LOTSTATUS_T") && st.iCurrentStatus == 2 && combo == 2,
              "B2  SetLotStatus(2) -> \"LOTSTATUS_T\" (golden :646-648)");
        SckArt_SetLotStatus(st, 3, &combo);
        CHECK(st.sLOTSTATUS == AnsiString("LOTSTATUS_L") && st.iCurrentStatus == 3 && combo == 3,
              "B3  SetLotStatus(3) -> \"LOTSTATUS_L\" (golden :649-651)");
        SckArt_SetLotStatus(st, 4, &combo);
        CHECK(st.sLOTSTATUS == AnsiString("LOTSTATUS_R") && st.iCurrentStatus == 4 && combo == 4,
              "B4  SetLotStatus(4) -> \"LOTSTATUS_R\" (golden :652-654)");
        SckArt_SetLotStatus(st, 5, &combo);
        CHECK(st.sLOTSTATUS == AnsiString("LOTSTATUS_F") && st.iCurrentStatus == 5 && combo == 5,
              "B5  SetLotStatus(5) -> \"LOTSTATUS_F\" (golden :655-657)");
        SckArt_SetLotStatus(st, 6, &combo);
        CHECK(st.sLOTSTATUS == AnsiString("LOTSTATUS_A") && st.iCurrentStatus == 6 && combo == 6,
              "B6  SetLotStatus(6) -> \"LOTSTATUS_A\" (golden :658-660)");
        SckArt_SetLotStatus(st, 0, &combo);
        CHECK(st.sLOTSTATUS == AnsiString("NONE") && st.iCurrentStatus == 0 && combo == 0,
              "B7  SetLotStatus(0) -> default: \"NONE\" (golden :661-662); 0 is golden's iLOTSTATUS_NONE");
        CHECK(SckArt_GetLotStatus(st) == AnsiString("NONE"),
              "B8  GetLotStatus() returns sLOTSTATUS verbatim (golden :669-672)");
    }

    // =====================================================================
    //  PART B(ii) -- the OBSERVABLE COST of divergence D1.
    //  csystem.cpp's DoCleanOutFinishCheck calls SetLotStatus(iLOTSTATUS_L).
    //  Golden's iLOTSTATUS_L is 3 (golden SCK_ART.cpp:46); W7C1_TfSCKARTSeam
    //  constructs it 0. Reproduce both feeds and pin what each produces, so the
    //  cost of the divergence is stated as an executable fact, not prose.
    //
    //  AI(W906-W7-F2fix) 20260729: B9/B10 used to be labelled "DIVERGENCE D1
    //  PINNED".  They are NOT a pin on the divergence -- they feed literals to
    //  SckArt_SetLotStatus and would stay green if the seam's iLOTSTATUS_L became
    //  3, or 7, or anything else.  The seam initialiser itself is pinned by PART C
    //  (checks C2/C4/C5/C6).  Labels corrected; the assertions themselves are
    //  sound and unchanged.
    // =====================================================================
    {
        SckArtState asGolden;
        SckArt_SetLotStatus(asGolden, 3);              // golden iLOTSTATUS_L == 3
        CHECK(asGolden.sLOTSTATUS == AnsiString("LOTSTATUS_L") && asGolden.iCurrentStatus == 3,
              "B9  reference: with golden's iLOTSTATUS_L(3) the lot status becomes \"LOTSTATUS_L\"");

        SckArtState asPorted;
        SckArt_SetLotStatus(asPorted, 0);              // W7C1_TfSCKARTSeam's iLOTSTATUS_L == 0
        CHECK(asPorted.sLOTSTATUS == AnsiString("NONE") && asPorted.iCurrentStatus == 0,
              "B10 COST OF DIVERGENCE D1 (not the pin -- see PART C): feeding SckArt_SetLotStatus "
              "the 0 that csystem.cpp's W7C1_TfSCKARTSeam constructs for iLOTSTATUS_L, instead of "
              "golden's 3 (golden SCK_ART.cpp:46), records \"NONE\" instead of \"LOTSTATUS_L\". "
              "W7C2_TfSCKARTSeam has the same defect for iLOTSTATUS_W(golden 1, SCK_ART.cpp:44) / "
              "iLOTSTATUS_R(golden 4, :47) / iLOTSTATUS_A(golden 6, :49). REPORTED not fixed (plan "
              "SS6-F2). This assertion is a property of SckArt_SetLotStatus and stays TRUE even "
              "after the seams are fixed -- it is PART C, not this, that must be updated then.");
    }

    // =====================================================================
    //  PART B(iii) -- DIVERGENCE D2 in the two STATE STRUCTS, pinned.
    //  golden SCK_ART.cpp:42 sets iTesterType=0, then :111-114 sets it to 1 for
    //  every CUSTOMER_CODE except CC_SCK. Both ported state structs hardcode 0.
    //  These two ARE genuine pins: they read the constructed field of the real
    //  types.  The csystem.cpp seams' own iTesterType shadows are separate
    //  storage and are pinned by PART C (C2/C6) instead.
    // =====================================================================
    {
        SckArtState          st;
        SckArtRemainderState rs;
        CHECK(st.iTesterType == 0,
              "B11 DIVERGENCE D2 PINNED (SckArtState): iTesterType constructs 0 = Flex. Golden "
              "constructs 1 = 93K for every CUSTOMER_CODE except CC_SCK (golden SCK_ART.cpp:42 "
              "then :111-114 `else { iTesterType=1; }`). REPORTED not fixed (plan SS6-F2). WHEN A "
              "FUTURE WAVE RE-DERIVES THE CUSTOMER BRANCH, UPDATE THIS ASSERTION.");
        CHECK(rs.iTesterType == 0,
              "B12 DIVERGENCE D2 PINNED (SckArtRemainderState): same field, now inherited from "
              "SckArtState after the W7-F2 merge, so the divergence is single-sourced -- one fix "
              "site instead of two.");
    }

    // =====================================================================
    //  PART C -- THE SIX csystem.cpp SEAM INITIALISERS THAT CARRY D1 AND D2.   [RESOLVED W-214 20261010 (St02-E): D1 fixed -- the seam fields are no longer read and hold golden values (C1-C4), C9 pins the call sites, C10 the golden constants on fSCKART; D2 reconciled by W1b, C7 / C8]
    //  AI(W906-W7-F2fix) 20260729 -- review-track HIGH-1.
    //
    //  WHY A SOURCE-TEXT PIN AND NOT A RUNTIME ONE.  `struct W7C1_TfSCKARTSeam`
    //  and `struct W7C2_TfSCKARTSeam`, and their single instances
    //  W7C1_fSCKART_ext / W7C2_fSCKART_ext, are declared `static` at file scope
    //  INSIDE csystem.cpp and reached only through the file-local W7C1_SCKART /
    //  W7C2_SCKART macros.  They have no external linkage and appear in no
    //  header, so no test translation unit can name them -- checked, not assumed.
    //  The six divergent values are therefore not directly observable, and the
    //  one place they are written down is the source text of the two ctor
    //  initialiser lists.  That is what PART C reads.
    //
    //  AI(W906-W7-F2fix2) 20260731 -- CORRECTION.  This block used to say flatly
    //  "there is therefore no runtime observation available".  That overstates it.
    //  What is true is that no test TU can NAME the seams.  An INDIRECT runtime
    //  observation is not impossible: DoCleanOutFinishCheck() is an ordinary free
    //  function (declared in csystem.h, already driven by
    //  tests/test_w7_c1_cleanout_finish.cpp) and its ART gate reads
    //  `W7C1_SCKART->iTesterType==0` at runtime, so a test that forced
    //  USE_AUTO_RETEST/IniConfig.bA10_AutoReTest/CosFunction.bUseSCKART on and
    //  drove the clean-out state machine as far as that gate could tell 0 from 1
    //  by its side effects.  It is not CHEAP: the gate sits behind ~20 sequential
    //  machine-state guards earlier in that function, each of which returns early,
    //  and the effects that distinguish the two paths run through
    //  CheckLoadingCount and the WAR0119/WAR0120/WAR0121 ShowErrorMessage arms.
    //  NOT ATTEMPTED in this round -- recorded as a possible future behavioural
    //  test, not as an impossibility.
    //
    //  WHAT THIS IS AND IS NOT.  It is a text-level regression guard on six
    //  specific initialisers, anchored by symbol name.  It is NOT a behavioural
    //  observation, and it does NOT prove the seams behave like golden -- they
    //  demonstrably do not, which is the whole point.  The residual exposure
    //  (nothing offline exercises the ART branches these constants gate) is
    //  recorded in docs/W7-UI-SKIPPED.md section "## W7-F2-fix (2026-07-29)",
    //  per plan SS12.6.
    //
    //  WHY IT IS LOAD-BEARING, AND EXACTLY HOW FAR IT REACHES.  Measured by
    //  mutating csystem.cpp, running this exe, and restoring the bytes -- on 20260729
    //  (26 perturbations, parser as first written), on 20260731 (hardened parser), and
    //  again on 20260801 (31 prover mutations, then a 30-case re-measure of the parser
    //  as it now stands).  It goes RED on: each of the six values flipped to its golden
    //  constant; a wrong non-golden value; a non-literal initialiser (`0+1` included);
    //  the field deleted from the list; the whole ctor deleted; conversion to an in-class
    //  initialiser; the struct and/or the ctor renamed; the ctor body assigning the
    //  pinned field, INCLUDING when the same line carries a string literal containing
    //  `//` (that one was a silent hole until 20260801); a value-changing hex/suffixed
    //  literal (`0x1`, `3L`); and csystem.cpp unreadable.  It stays GREEN on
    //  value-preserving reflows: extra spaces, a leading comma, a comment inside the
    //  list, a newline between `(` and the literal, BOTH clang-format
    //  constructor-initializer styles (`Ctor() :` and `Ctor()` + newline + `: `), and
    //  hex/octal/suffixed spellings of the same value (`0x0`, `0u`).
    //
    //  STILL NOT CLOSED, and it is a text scanner so some of this cannot be:
    //    (a) a ctor body that changes the field WITHOUT naming it -- a helper call,
    //        `memset(this,...)`, or a delegated ctor -- is invisible here;
    //    (b) so is a write to the single instance from ANYWHERE ELSE in csystem.cpp
    //        (e.g. `W7C1_fSCKART_ext.iTesterType=1;` in some init path).  Nothing
    //        like that exists today: the only writes to any seam iTesterType in the
    //        tree are W7C2_TfSCKARTSeam::CheckNeedRT's `core.iTesterType=iTesterType;`
    //        (which writes the embedded core, not the shadow) -- grepped again
    //        20260801 -- but this pin would not notice one being added;
    //    (c) brace-init (`iTesterType{0}`) reddens even though it preserves the
    //        value.  Left as a conservative false alarm rather than a silent hole;
    //    (d) C++11 raw string literals are not understood by w7f2_stripComments.  None
    //        exist in this tree.  Two shapes WERE tried on 20260801 -- `R"(http://x)"` and
    //        `R"(a"b)"`, each in a ctor body that also assigns the pinned field -- and both
    //        reddened, i.e. the mis-parse fails loudly rather than passing.  That is a
    //        measurement on two shapes, not a proof for every raw string.
    //  RETIRED FROM THIS LIST on 20260801: "`/* */` comments are not stripped".  It was
    //  disclosed as a hole and it was not one for the shape that matters (`/* set */
    //  iTesterType = 1;` in the body reddens correctly -- re-measured), and block
    //  comments are stripped now regardless.
    //
    //  WHEN A FUTURE WAVE ACTUALLY FIXES THE SEAMS, the honest edit is to change
    //  the expected value here from the ported 0 to the golden constant in the
    //  SAME change, not to delete the check.
    // =====================================================================
    {
        const std::string root = w7f2_portedRoot();
        const std::string path = root + "/csystem.cpp";
        std::string raw;
        const bool got = w7f2_readFile(path, raw);
        CHECK(got, "C0  csystem.cpp is readable from this test (path derived from __FILE__); "
                   "without it the six pins below cannot run and MUST NOT be treated as passing");

        if (got) {
            //  File-wide comment strip BEFORE any searching -- hole H2.
            const std::string src = w7f2_stripComments(raw);

            //  AI(W906-W7-F2fix2) 20260731 -- LABELS SOFTENED.  These six used to
            //  say the ctor "constructs" the value.  A text pin cannot see that:
            //  what it checks is that the mem-initialiser still READS `field(0)`
            //  and that the ctor body does not then assign the same field.  That
            //  is strictly less than "constructs" (see STILL NOT CLOSED (a)/(b)
            //  above), so the labels now claim only what is checked.

            // --- D1 RESOLVED (AI(W906-W214) 20261010 (St02-E), laptop 06:2x): the seam iLOTSTATUS_* fields are no longer read; they hold golden values (laptop 07:1x) ---
            PIN_SEAM(src, "W7C1_TfSCKARTSeam", "iLOTSTATUS_L", 3, "C1  RESOLVED W-214 (laptop 07:1x): W7C1_TfSCKARTSeam::iLOTSTATUS_L is no longer read (see C9) and holds golden 3, Automation/SCK_ART.cpp:46");
            PIN_SEAM(src, "W7C2_TfSCKARTSeam", "iLOTSTATUS_W", 1, "C2  RESOLVED W-214 (laptop 07:1x): W7C2_TfSCKARTSeam::iLOTSTATUS_W is no longer read (see C9) and holds golden 1, Automation/SCK_ART.cpp:44");
            PIN_SEAM(src, "W7C2_TfSCKARTSeam", "iLOTSTATUS_R", 4, "C3  RESOLVED W-214 (laptop 07:1x): W7C2_TfSCKARTSeam::iLOTSTATUS_R is no longer read (see C9) and holds golden 4, Automation/SCK_ART.cpp:47");
            PIN_SEAM(src, "W7C2_TfSCKARTSeam", "iLOTSTATUS_A", 6, "C4  RESOLVED W-214 (laptop 07:1x): W7C2_TfSCKARTSeam::iLOTSTATUS_A is no longer read (see C9) and holds golden 6, Automation/SCK_ART.cpp:49");
            {   const char* site[] = { "W7C1_SCKART->SetLotStatus(fSCKART->iLOTSTATUS_L)", "fSCKART->iCurrentStatus!=fSCKART->iLOTSTATUS_A",
                                       "fSCKART->iCurrentStatus==fSCKART->iLOTSTATUS_R", "W7C2_SCKART->SetLotStatus(fSCKART->iLOTSTATUS_W)",
                                       "SetLotStatus(int iStatus){ fSCKART->SetLotStatus(iStatus);" };   // 2 + 2 + 2 + 2 call sites, 2 seam setters
                const char* shadow[] = { "W7C1_SCKART->iLOTSTATUS_", "W7C2_SCKART->iLOTSTATUS_", "W7C2_SCKART->iCurrentStatus", "W7C1_SCKART->iWaitGPIBLotR", "W7C2_SCKART->iWaitGPIBLotR", "W7C2_SCKART->AccessFile" };   // last two: W-216
                int nSite[5] = {0, 0, 0, 0, 0}, nShadow = 0;  for (int k = 0; k < 5; ++k) for (std::size_t at = src.find(site[k]); at != std::string::npos; at = src.find(site[k], at + 1)) ++nSite[k];
                for (int k = 0; k < 6; ++k) for (std::size_t at = src.find(shadow[k]); at != std::string::npos; at = src.find(shadow[k], at + 1)) ++nShadow;
                CHECK(nSite[0] == 2 && nSite[1] == 2 && nSite[2] == 2 && nSite[3] == 2 && nSite[4] == 2 && nShadow == 0, "C9  D1 RESOLVED IN SOURCE TEXT (W-214): the 8 lot-status call sites (:3200 :3251 / :5784 :5869 / :5827 :5912 / :5829 :5914) read fSCKART->, both seam SetLotStatus forward to fSCKART, and no seam iLOTSTATUS_* / W7C2 iCurrentStatus / W7C1 + W7C2 iWaitGPIBLotR / W7C2 AccessFile use is left (W-216)"); }
            {   std::string fs;  const bool gotFs = w7f2_readFile(root + "/forms/fSCKART.cpp", fs);  const std::string fc = gotFs ? w7f2_stripComments(fs) : std::string();
                const char* k[] = { "iLOTSTATUS_W   = 1;", "iLOTSTATUS_T = 2;", "iLOTSTATUS_L = 3;", "iLOTSTATUS_R = 4;", "iLOTSTATUS_F = 5;", "iLOTSTATUS_A = 6;" };
                int nK = 0;  for (int i = 0; i < 6; ++i) if (fc.find(k[i]) != std::string::npos) ++nK;
                CHECK(gotFs && nK == 6, "C10 the ONE lot state's constants are golden: TfSCKART's ctor (forms/fSCKART.cpp) sets W 1, T 2, L 3, R 4, F 5, A 6 "
                      "(golden Automation/SCK_ART.cpp:44-49) -- the values every csystem.cpp call site now reads through fSCKART->"); }

            // --- D2: the two seam-local iTesterType shadows -----------------
            //   These are NOT the SckArtState field B11 pins -- each seam owns a
            //   separate top-level `iTesterType`, and it is the seam's copy (not
            //   the embedded `core`'s) that every csystem.cpp call site reads.
            PIN_SEAM(src, "W7C1_TfSCKARTSeam", "iTesterType", 0,
                     "C5  D2 PINNED IN SOURCE TEXT (the divergence it carries IS behavioural): "
                     "W7C1_TfSCKARTSeam's mem-initialiser list still reads iTesterType(0), body does "
                     "not assign it; golden Automation/SCK_ART.cpp:42 then :113 leaves 1 for every "
                     "CUSTOMER_CODE except CC_SCK. With 0 the `W7C1_SCKART->iTesterType==0` GATE in "
                     "DoCleanOutFinishCheck is TRUE, so the port ENTERS the whole Flex-ART block that "
                     "golden SKIPS -- see Automation/SCK_ART.cpp's SckArtState ctor note");
            PIN_SEAM(src, "W7C2_TfSCKARTSeam", "iTesterType", 0,
                     "C6  D2 PINNED IN SOURCE TEXT (the divergence it carries IS behavioural): "
                     "W7C2_TfSCKARTSeam's mem-initialiser list still reads iTesterType(0), body does "
                     "not assign it; golden leaves 1 except for CC_SCK, which inverts all five "
                     "`W7C2_SCKART->iTesterType==0/==1` reads in DoART_AfterCleanOut");

            // --- D2 RECONCILED (AI(W906-W1b) 20260927, Steven W1 = (b), the laptop's W1b "yes, inside W1") -----
            //   C5 / C6 still hold (the two shadows still construct 0), but nothing reads them any more: the six
            //   brand reads (DoCleanOutFinishCheck's ART gate and the five in DoART_AfterCleanOut) and
            //   W7C2_TfSCKARTSeam::CheckNeedRT's copy into its core read fSCKART->iTesterType, the one live brand
            //   (forms/fSCKART.cpp AccessFile, the recipe's [AutoRetest] iTesterType, missing = 1).  So the
            //   golden 1 reaches every gate and D2 no longer changes behaviour.  C7 / C8 pin that in source text.
            {
                std::size_t nShadow = 0, nLive = 0;
                const char* shadows[] = { "W7C1_SCKART->iTesterType", "W7C2_SCKART->iTesterType",
                                          "core.iTesterType=iTesterType" };
                for (int k = 0; k < 3; ++k)
                    for (std::size_t at = src.find(shadows[k]); at != std::string::npos; at = src.find(shadows[k], at + 1))
                        ++nShadow;
                for (std::size_t at = src.find("fSCKART->iTesterType"); at != std::string::npos;
                     at = src.find("fSCKART->iTesterType", at + 1))
                    ++nLive;
                CHECK(nShadow == 0,
                      "C7  D2 RECONCILED IN SOURCE TEXT: csystem.cpp (comments stripped) has no read of "
                      "W7C1_SCKART->iTesterType / W7C2_SCKART->iTesterType and no `core.iTesterType=iTesterType` "
                      "-- every brand read goes to fSCKART->iTesterType (W1b)");
                CHECK(nLive >= 9 && src.find("core.iTesterType=fSCKART->iTesterType") != std::string::npos,
                      "C8  D2 RECONCILED IN SOURCE TEXT: the six former shadow reads, CheckNeedRT's core copy and the "
                      "two older direct reads (:753, :1576) all read fSCKART->iTesterType (>= 9 sites)");
            }
        }
    }

    // =====================================================================
    //  Summary
    // =====================================================================
    if (g_fail == 0) {
        std::printf("PASS  %d/%d checks passed\n", g_total, g_total);
        return 0;
    }
    std::printf("FAIL  %d/%d checks failed\n", g_fail, g_total);
    return 1;
}
