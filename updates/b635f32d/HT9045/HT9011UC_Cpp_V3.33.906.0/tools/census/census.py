#!/usr/bin/env python3
r"""HT9045 V906 translation census -- FUNCTION-LEVEL, re-runnable, version-controlled.

WHY THIS FILE EXISTS IN THE REPO
The previous census (`census.py` / `classify.py` / `remaining.py`, 20260807) was left in a
session scratchpad and vanished with the session, which is why
docs/PT_CAMPAIGN_PLAN.md section 3's percentages had to be blocked from quotation. This is
the replacement and it lives in the tree.

METHOD (per PT_CAMPAIGN_PLAN section 2)
  scope  = golden HT9045.bpr <OBJFILES>. Those entries are all `..\\Obj\\<Name>.obj`
           (a flat output dir), so sources are matched by BASENAME against an index of
           the golden tree, never looking inside .svn.
  unit   = code line (blank- and comment-stripped) and raw line, both reported.
  file   = does a port mirror exist at the same relative path?
  func   = golden top-level function definitions vs port definitions OF THE SAME NAME.
           A golden function with no port counterpart is charged at its full golden span,
           so "the file exists but only 10% is translated" cannot read as complete.
  form   = a same-named .dfm exists beside the golden source.

KNOWN DISTORTIONS, stated because a number without them is misleading
  * extract-calc-core renames. This tree deliberately renames when it extracts a
    computational core (golden `TfBarCode::DoBarcodeCCDInShuttle_1` ->
    `BarCode_DoBarcodeCCDInShuttle_1`). Pure name matching UNDER-counts. An alias rule
    covers `<Prefix>_<GoldenName>` and `<Class>_<GoldenName>`; it is deliberately NOT a
    bare cross-class match, which would mis-pair `HT9045Gem::AddEC` onto a base method.
  * OVERLOADS COLLAPSE, so overload-only gaps are INVISIBLE (found 20260809 during
    PT-W5b). `functions()` keys by name, so golden's two
    `HT9045Gem::S7F6_ProcessProgramData` -- the no-arg one at :5326 and the
    `(AnsiString FileName)` one at :5605 -- became one entry, and the missing-function
    list showed only one. If a port defines one overload and not the other, this census
    reports the function as DONE. Fixing it properly means keying by signature, which
    needs a real parameter-list parse (BCB6 default arguments and multi-line
    declarations make the naive split wrong). Until then: for any file whose remaining
    gap matters, grep golden for repeated definition names before trusting the count.
    MEASURED SCOPE of this blind spot, so nobody has to guess how bad it is: across the
    whole golden tree, 29 .cpp files contain a repeated `Class::method` name, hiding 72
    extra definitions in total. So the "done" side of every percentage here is
    optimistic by at most those 72 functions -- material for a per-file decision, not
    enough to move the headline figures.
  * ABBREVIATED BODIES COUNT AS DONE -- the largest distortion found so far, and it goes the
    OPTIMISTIC way (found 20260810 during PT-W5f's audit). This census asks "does a port
    function of this name exist and is it not wholly gated". It never compares SIZE. The
    port's established practice is to preserve golden's body VERBATIM inside `#if 0` and
    supply a small LIVE no-op beside it with a gate note, e.g. csystem.cpp DoTrayFeedProcess:
    golden's 1,235 lines sit gated at :6249, and the live body at :7487 is 6 lines of
    explanation. This census scores that function as fully translated.
    The extreme case is csystem.cpp's own MainProc: golden 2,390 lines, live port body 26.
    DoAllProcess is the same shape -- golden 1,141 lines against a 239-line port body whose
    entire early-return ladder (golden :9165-10047, 883 lines) is a TWELVE-LINE SUMMARY
    inside one `#if 0`, containing text like `/* ... ~300 lines ... */`.
    MEASURED with tools/census/body_size_scan.py: of 1,356 comparable port bodies, 74 are
    under 50% of their golden span. Restricted to golden files whose braces balance (the
    others have unreliable spans -- see the brace-walk distortion below), 59 remain: 52
    NON-FORM functions accounting for 14,905 golden lines, plus 7 form functions at 981.
    EFFECT ON THE HEADLINE, stated because every percentage in this file inherits it:
        non-form as reported here      done 267,261 / 336,509 = 79.4%
        non-form adjusted for this     done 252,356 / 336,509 = 75.0%
    So the non-form figure is optimistic by about 4.4 points. That is a FLOOR on the
    overstatement, not a ceiling: the scan only catches bodies under half size, so partial
    translations between 50% and 100% are still counted as complete.
    NOT FIXED HERE ON PURPOSE. Making span a pass/fail criterion would be wrong -- port
    bodies are routinely LONGER than golden because of banners and gate notes, and a shorter
    body can be legitimate when a block was correctly relocated. Use body_size_scan.py as a
    RANKING of where to look, and quote the adjusted figure alongside the raw one.
  * ONE-LINE DEFINITIONS ARE INVISIBLE (found 20260809 during PT-W5c). `DEFN` ends with
    `\([^;]*$` -- '(' and no ';' on the line -- so a body written on a single line is not
    recognised as a definition at all:
        bool CheckSafeDoorIsClosed() { return true; }        <-- not seen
        bool InSHT1InLF() { return false; }                   <-- not seen
    MEASURED SCOPE: the port tree holds 766 such definitions; 110 of them are the port
    body for a same-named golden function IN THE SAME FILE (Motor/mymotor.cpp 47,
    ainarm9045.cpp 35, aoutarm9045.cpp 11, mytray.cpp 9, ...), each charged at full
    golden span.
    DO NOT "fix" this without thinking: a one-line port body is nearly always a DEGRADED
    STUB (mymotor.cpp's 47 are the Gali_* offline stubs), so reporting them as NOT
    translated is arguably the correct answer for wave planning, and this census
    deliberately keeps that behaviour.
    WHERE IT IS FLATLY WRONG is a pre-integration COLLISION SCAN: the symbol IS defined
    and WILL collide. Measured cost on 20260809: a scan for csystem.cpp's 122-function
    wave-1 scope using this regex found 24 existing definitions; the same scan with
    one-line bodies included found 46 across 11 files, covering 44 of the 122. The 22
    missed ones would each have surfaced as an unexpected `multiple definition` at link.
    Any collision scan MUST use its own definition matcher, not this one.
  * CROSS-FILE HOMES are counted as MISSING (found 20260809 during PT-W5c). Matching is
    per RELATIVE PATH, but this port deliberately relocates bodies out of a golden file:
    golden csystem.cpp's 21-strong HasIC predicate family lives in the port's
    csystem_predicates.cpp, `XPitchIsStand` in csystem_shims.cpp, golden
    BarCode/BarCode_Sh1.cpp's scan bodies in BarCode/BarCode_Shuttle1_Scan.cpp. Each is
    charged at its full golden span, so remaining work is OVERSTATED.
    MEASURED CEILING, tree-wide: of 5,823 golden functions with no live same-path port
    body (382,620 golden lines), 268 have a body elsewhere in the port under a name that
    is defined in EXACTLY ONE golden file -- 23,871 golden lines, 6.2% of the gap; the
    non-form share is 10,537 lines. A further 564 hits (49,368 lines) were EXCLUDED as
    unprovable: names like `FormShow` / `ReadFile` / `SaveSetupFile` / `DoIniDataToForm`
    exist on dozens of distinct VCL form classes, so a port hit does not identify the
    same function -- which is exactly why the alias rule above refuses bare cross-class
    matching. Treat 23,871 as a CEILING, not a correction: a same-named body elsewhere
    may be a DEGRADED STUB rather than a translation. Measured on csystem.cpp's 24 hits:
    21 faithful (csystem_predicates.cpp), 1 faithful (csystem_shims.cpp XPitchIsStand),
    1 faithful-but-duplicating-a-golden-defect (aoutarm.cpp SendDataToASE), and 1 a
    `static` no-op stub (MyLaneIo.cpp:49) -- so ~4% of that file's hits were NOT done.
  * THE BRACE WALK OVERSHOOTS ON UNBALANCED FILES, inflating spans (found 20260809).
    `functions()` finds a definition's end by naive `{`/`}` counting, which is wrong when
    a file's braces do not balance -- `#if`/`#else` arms with asymmetric braces, or a
    brace inside a string/mojibake byte. When balance is never reached the walk runs to
    the 4000-line cap or to the next accidental balance point, and the span is fiction:
    golden Command.cpp (whole-file balance +1) reports `TfMain::PERSITETemperatureStrings`
    at :945 as spanning 4,002 lines.
    MEASURED SCOPE: 18 of the 289 in-scope golden files are brace-unbalanced by this
    naive count (main.cpp +5, cSocket.cpp -7, ainarm9045.cpp -2, cContact.cpp +1,
    Command.cpp +1, and 13 more at +/-1), together holding 130,159 golden lines. Spans
    inside those 18 files may be overstated; the other 271 are unaffected. NOTE for wave
    planning: csystem.cpp balances, so its figures are sound -- cContact.cpp does NOT,
    so verify its per-function spans by hand before sizing a wave on them.
  * `#if 0` in a port file. Golden code kept inside `#if 0` is NOT translated behaviour.
    Port functions whose body is entirely gated are reported separately as GATED rather
    than counted as translated.
  * form/non-form by .dfm is mechanically right and semantically wrong for a class that
    spans files (TfMain across main.cpp + Command.cpp). Reported, not hidden.

USAGE
    python tools/census/census.py                 # summary
    python tools/census/census.py --detail        # + per-file remaining table
    python tools/census/census.py --json out.json
"""
import io, os, re, sys, json, argparse

GOLD = os.environ.get('HT9045_GOLDEN', 'D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618')
PORT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BS = chr(92)

# A top-level definition: starts at column 0, has a parameter list, is not a call or a
# declaration ending in ';'.
DEFN = re.compile(
    r'^(?!\s)(?:[A-Za-z_][\w:*&<>,\s]*?\s+)??'      # optional return type
    r'(?:([A-Za-z_]\w*)::)?'                         # optional Class::
    r'(~?[A-Za-z_]\w*)\s*'                           # name
    r'\([^;]*$')                                     # '(' and no ';' on this line
KEYWORDS = {'if', 'for', 'while', 'switch', 'return', 'else', 'do', 'catch', 'sizeof',
            'ZeroMemory', 'memset', 'memcpy', 'delete', 'new', 'case', 'try', 'throw',
            'and', 'or', 'not', 'defined', 'assert'}


def defn_probe(line):
    r"""Reduce a source line to just its would-be signature, for DEFN matching.

    DEFN ends in `\([^;]*$` so that a DECLARATION (which ends in ';') is not mistaken for a
    definition. Two things defeated that, both found 20260811 and both causing real definitions to
    be counted as MISSING:
      1. a trailing COMMENT containing ';' --
             bool bUseAxxGPicker()   // golden :8304 (REAL-defined here; stub removed)
      2. a ONE-LINE BODY, where the ';' is inside the braces --
             int CheckShuttleSensor_9045_2x5(int, bool, bool) { return 0; }
         This one hid 35 already-translated functions from a single wave's target list, and only
         surfaced as 35 redefinition errors after the stitch.
    So: drop the comment, then drop anything from the first '{' onwards. A real declaration still
    keeps its ';' and is still correctly rejected.
    """
    s = line.split('//')[0]
    if '{' in s:
        s = s.split('{')[0]
    return s.rstrip()


def read(path, golden):
    enc = 'cp950' if golden else 'utf-8'
    return io.open(path, encoding=enc, errors='replace').read()


def code_lines(text):
    raw = len(text.splitlines())
    t = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    n = sum(1 for l in t.splitlines() if re.sub(r'//.*', '', l).strip())
    return raw, n


def gate_depth_map(lines):
    """line index (0-based) -> True if inside any `#if 0`."""
    out, stack = [], []
    for l in lines:
        t = l.strip()
        if re.match(r'^#\s*if\b|^#\s*ifdef\b|^#\s*ifndef\b', t):
            zero = bool(re.match(r'^#\s*if\s+0\b', t))
            # BUG FIXED 20260810: this was `stack.append(zero or (stack and stack[-1]))`.
            # When `stack` is EMPTY, `stack and stack[-1]` short-circuits to `stack` ITSELF,
            # so appending it made the list SELF-REFERENTIAL. From then on `stack[-1]` was a
            # non-empty list -- always truthy -- so EVERY line inside any #ifdef / #ifndef /
            # #if <expr> block was reported as GATED. Effect: functions inside such blocks
            # counted as NOT translated, understating completion; and the collision scans
            # built on this helper skipped genuinely LIVE definitions (it hid 3 real stubs at
            # PT-W5f's integration, which the linker then reported as duplicates).
            # Non-nested `#if 0` blocks were accidentally correct, which is why this survived.
            stack.append(bool(zero) or bool(stack[-1] if stack else False))
        elif t.startswith('#endif') and stack:
            stack.pop()
        out.append(bool(stack[-1]) if stack else False)
    return out


def code_only(lines):
    """AI(W906-CENSUS-BRACE) 20260927: each line with // comments, /* */ blocks (multi-line) and "..." / '...' literals
    removed, so functions() counts only the braces the compiler sees.  Before this, one brace inside a comment or a
    string made the balance walk run on and swallow every function after it (port Command.cpp:3026-3734: GetCZSiteMap,
    GetUPH, GetAll, GetSiteState ... all reported MISSING although translated)."""
    out, in_block = [], False
    for s in lines:
        res, k, n = [], 0, len(s)
        while k < n:
            if in_block:
                e = s.find('*/', k)
                if e < 0:
                    k = n
                    break
                in_block = False
                k = e + 2
                continue
            c = s[k]
            if c == '/' and k + 1 < n and s[k + 1] == '/':
                break
            if c == '/' and k + 1 < n and s[k + 1] == '*':
                in_block = True
                k += 2
                continue
            if c == '"' or c == "'":
                q = c
                k += 1
                while k < n and s[k] != q:
                    k += 2 if s[k] == '\\' else 1
                k += 1
                continue
            res.append(c)
            k += 1
        out.append(''.join(res))
    return out


def functions(text):
    """name -> (start, end, span, gated) using brace balance."""
    lines = text.splitlines()
    gated = gate_depth_map(lines)
    code = code_only(lines)   # AI(W906-CENSUS-BRACE) 20260927: braces are counted on this, not on the raw line
    out, i, n = {}, 0, len(lines)
    while i < n:
        l = lines[i]
        if not l or l[0] in ' \t/#}':
            i += 1
            continue
        # AI(W906-CENSUS-BRACE) 20260927: match the signature on the comment-stripped line too -- a whole function inside
        #   /* ... */ (golden ATC/ATCInterface.cpp:1277-1286 `}*/`) is not compiled, so it is not a function; matching the raw
        #   line made its brace walk run on and swallow the NEXT function (the destructor at :1288).
        m = DEFN.match(defn_probe(code[i])) if code[i].strip() else None
        if not m:
            i += 1
            continue
        name = m.group(2)
        if name in KEYWORDS or (m.group(1) or '') in KEYWORDS:
            i += 1
            continue
        # walk to the opening brace, then to balance
        depth, j, seen = 0, i, False
        while j < n:
            depth += code[j].count('{') - code[j].count('}')
            if '{' in code[j]:
                seen = True
            if seen and depth <= 0:
                break
            j += 1
            if j - i > 4000:
                break
        if not seen:
            i += 1
            continue
        key = (m.group(1) + '::' + name) if m.group(1) else name
        # BUG FIXED 20260810: this was a bare `out.setdefault(...)`, i.e. FIRST OCCURRENCE
        # WINS. That is wrong for this tree, which deliberately keeps a `#if 0` golden-verbatim
        # copy BESIDE a small live body -- and the gated copy usually comes FIRST
        # (csystem.cpp MainProc: gated golden at :595, live body at :2999). First-wins recorded
        # the function as GATED and therefore charged it as MISSING, so ADDING golden's text
        # made completion appear to DROP: PT-W6a moved non-form from 79.4% to 78.4% purely
        # through this, while the compiler's own preprocessed output was byte-identical.
        # A LIVE occurrence must beat a GATED one; among equals, keep the larger span.
        prev = out.get(key)
        if prev is None or (prev[3] and not gated[i]) or \
           (prev[3] == gated[i] and (j - i + 1) > prev[2]):
            out[key] = (i + 1, j + 1, j - i + 1, gated[i])
        i = j + 1
    return out


def index_golden():
    idx = {}
    for root, dirs, files in os.walk(GOLD):
        dirs[:] = [d for d in dirs if d.lower() != '.svn']
        for f in files:
            if f.lower().endswith(('.cpp', '.c')):
                rel = os.path.relpath(os.path.join(root, f), GOLD).replace(BS, '/')
                idx.setdefault(f.lower(), []).append(rel)
    return idx


def port_live_names(cache={}):
    r"""Every name with a LIVE free-function definition ANYWHERE in the port tree.

    WHY (found 20260811, PT-W7e): the per-file model below asks only whether the port file of the
    SAME relative path defines a golden function. A body an earlier wave parked in a DIFFERENT port
    file therefore counts as MISSING. For ainarm2.cpp that was 55 of 82 functions, 37 of them in
    aHotPlateSubstrate.cpp. So the headline "missing" figure overstates how much is left.
    This does not replace the per-file number -- both are reported, because they answer different
    questions: per-file is "is this file mirrored", tree-wide is "is this golden code translated".
    """
    if cache:
        return cache["n"]
    names = set()
    for dp, dn, fn in os.walk(PORT):
        dn[:] = [d for d in dn if d.lower() not in (".svn", ".git", "tests")   # AI(W906-CENSUS-BRACE) 20260927: tests/ stand-ins are not the product (tests/test_ga1_cprod.cpp:159 is a 1-line TfSmartDiagnostic::GetCyliderOnCount)
                 and not d.startswith("build") and not d.startswith("_")]
        for f in fn:
            if not f.endswith(".cpp"):
                continue
            try:
                blob = io.open(os.path.join(dp, f), "rb").read()
            except Exception:
                continue
            sep = chr(13) + chr(10) if blob.count((chr(13) + chr(10)).encode()) else chr(10)
            LL = blob.decode("utf-8", "replace").split(sep)
            g = gate_depth_map(LL)
            CL = code_only(LL)   # AI(W906-CENSUS-BRACE) 20260927: a definition inside /* */ is not live
            for j, l in enumerate(LL):
                if (j < len(g) and g[j]) or not l or l[0] in " " + chr(9) + "/#}" or not CL[j].strip():
                    continue
                m = DEFN.match(defn_probe(CL[j]))
                if m and m.group(2) not in KEYWORDS and (m.group(1) or "") not in KEYWORDS:
                    # free functions by bare name (unchanged); members ALSO by Class::name (AI(W906-CENSUS-BRACE) 20260927:
                    #   golden form methods are usually translated into forms/fXxx.cpp, e.g. TfHome::InitialHomeClass)
                    names.add(m.group(2) if not m.group(1) else m.group(1) + "::" + m.group(2))
    cache["n"] = names
    return names


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--detail', action='store_true')
    ap.add_argument('--json')
    a = ap.parse_args()

    idx = index_golden()
    bpr = io.open(os.path.join(GOLD, 'HT9045.bpr'), encoding='latin-1').read()
    objs = re.search(r'<OBJFILES\s+value="(.*?)"/>', bpr, re.S).group(1).split()

    rows, unresolved = [], []
    for o in objs:
        o = o.strip()
        if not o.lower().endswith('.obj'):
            continue
        base = os.path.basename(o.replace(BS, '/'))[:-4]
        cands = idx.get((base + '.cpp').lower()) or idx.get((base + '.c').lower())
        if not cands:
            unresolved.append(o)
            continue
        src = sorted(cands, key=lambda p: (p.count('/'), len(p)))[0]
        gtext = read(os.path.join(GOLD, src), True)
        graw, gcode = code_lines(gtext)
        gfun = functions(gtext)
        form = os.path.exists(os.path.splitext(os.path.join(GOLD, src))[0] + '.dfm')

        pfile = os.path.join(PORT, src)
        if os.path.exists(pfile):
            ptext = read(pfile, False)
            pfun = functions(ptext)
            # alias set: bare names, Class::name, and Prefix_name extractions
            pnames = set()
            for k, v in pfun.items():
                bare = k.split('::')[-1]
                pnames.add(k)
                pnames.add(bare)
                if not v[3]:
                    pnames.add('LIVE:' + bare)
            extracted = {k.split('_', 1)[1] for k in pnames if '_' in k}
            missing, gated_ct, done_ct = [], 0, 0
            for k, (s, e, span, _g) in gfun.items():
                bare = k.split('::')[-1]
                hit = (k in pnames) or (bare in pnames) or (bare in extracted)
                if not hit:
                    missing.append((k, span))
                elif ('LIVE:' + bare) not in pnames and bare in pnames:
                    gated_ct += 1
                    missing.append((k, span))     # gated == not translated behaviour
                else:
                    done_ct += 1
            live_tree = port_live_names()
            parked = [(k, s) for k, s in missing if k in live_tree or k.split("::")[-1] in live_tree]   # AI(W906-CENSUS-BRACE) 20260927: qualified members too
            parked_lines = sum(s for _, s in parked)
            miss_lines = sum(s for _, s in missing)
            rows.append(dict(src=src, form=form, mirrored=True, graw=graw, gcode=gcode,
                             gfuncs=len(gfun), missing_funcs=len(missing),
                             gated_funcs=gated_ct, missing_lines=miss_lines,
                             parked_lines=parked_lines))
        else:
            rows.append(dict(src=src, form=form, mirrored=False, graw=graw, gcode=gcode,
                             gfuncs=len(gfun), missing_funcs=len(gfun),
                             gated_funcs=0, missing_lines=gcode, parked_lines=0))

    def tot(rs, k):
        return sum(r[k] for r in rs)

    print('scope: %d OBJFILES -> %d sources resolved, %d unresolved'
          % (len(objs), len(rows), len(unresolved)))
    if unresolved:
        print('  UNRESOLVED:', unresolved)
    print()
    print('%-10s %5s %9s %9s %9s %7s' % ('group', 'files', 'goldCode', 'missing', 'done', 'done%'))
    for label, rs in (('non-form', [r for r in rows if not r['form']]),
                      ('form', [r for r in rows if r['form']]),
                      ('ALL', rows)):
        gc, ml = tot(rs, 'gcode'), tot(rs, 'missing_lines')
        print('%-10s %5d %9d %9d %9d %6.1f%%'
              % (label, len(rs), gc, ml, gc - ml, 100.0 * (gc - ml) / gc if gc else 0))
    print()
    nomirror = [r for r in rows if not r['mirrored']]
    print('files with NO port mirror: %d non-form / %d form'
          % (len([r for r in nomirror if not r['form']]),
             len([r for r in nomirror if r['form']])))
    partial = sorted([r for r in rows if r['mirrored'] and r['missing_lines'] > 0],
                     key=lambda r: -r['missing_lines'])
    print("")
    print("SAME TREE, crediting bodies that ARE translated but parked in ANOTHER port file")
    print("(the line above uses the per-file model, which counts those as missing; see")
    print(" port_live_names() for why both numbers are reported):")
    print("%-10s %9s %9s %9s %7s" % ("group", "goldCode", "missing", "done", "done%"))
    for label, rs in (("non-form", [r for r in rows if not r["form"]]),
                      ("form", [r for r in rows if r["form"]]),
                      ("ALL", rows)):
        gc = tot(rs, "gcode")
        ml = tot(rs, "missing_lines") - tot(rs, "parked_lines")
        print("%-10s %9d %9d %9d %6.1f%%"
              % (label, gc, ml, gc - ml, 100.0 * (gc - ml) / gc if gc else 0.0))
    print('mirrored but INCOMPLETE: %d files, %d golden code lines still missing'
          % (len(partial), sum(r['missing_lines'] for r in partial)))
    if a.detail:
        print()
        print('%-42s %5s %6s %6s %8s %s' % ('file', 'funcs', 'miss', 'gated', 'missLine', 'form'))
        for r in partial[:60]:
            print('%-42s %5d %6d %6d %8d %s'
                  % (r['src'], r['gfuncs'], r['missing_funcs'], r['gated_funcs'],
                     r['missing_lines'], 'FORM' if r['form'] else ''))
    if a.json:
        io.open(a.json, 'w', encoding='utf-8').write(json.dumps(rows, indent=1))
        print('\nwrote', a.json)
    return 0


sys.exit(main())
