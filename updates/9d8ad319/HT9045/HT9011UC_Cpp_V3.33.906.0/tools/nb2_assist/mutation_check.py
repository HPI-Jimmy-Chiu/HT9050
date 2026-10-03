#!/usr/bin/env python3
# mutation_check.py -- "red check" helper: break the code under test on purpose, one small change at a time, and see
# whether the ctest notices.  NB2-1, user 1003 12:0x "第4項的突變檢查工具也做" (README R183 item 4: colleague MRs'
# new ctest pins come with "which line was broken on purpose -> which CHECK failed").
#
#   python mutation_check.py --file HT9011UC_Cpp_V3.33.906.0/WebMotorAccess.cpp --lines 3880-3900 \
#          --build-dir D:/HT9045/_wt_main/Obj/V906/build --target test_web_motor_access --ctest WebMotorAccess [--max 12]
#
# Mutations (one per run, only in the code part of a line -- not in // comments or string literals):
#   == <-> !=    <  -> >=    >  -> <=    <= -> >    >= -> <    && <-> ||    true <-> false    !x -> x (drop a '!')
#   integer literal n -> n+1    whole statement line -> commented out ("DEL")
# For each mutant: write the mutated bytes, `cmake --build <dir> --target <target>`, `ctest -R <regex>`, then put the
# ORIGINAL bytes back and check their sha1.  A build error counts as "build-broke" (not a useful mutant, not a pass).
#
# Safety: refuses a file with uncommitted changes (git diff), refuses the F5 dir (...\Obj\V906\build_dbg) unless
# --allow-f5, keeps a copy of the original in %TEMP% and prints how to restore it if the run is killed.  Run it in your
# own worktree / build dir, never one another session is building in, and run NOTHING else (no ctest, no build) in that
# build dir meanwhile -- NB2-1's first trial got five false "build-broke" rows from a concurrent `ctest -N`.  NB2-1 20261003: nor any OTHER build dir of the same worktree -- the mutant sits in the shared source, so e.g. a ship build running meanwhile compiles it (two false FAILs in ship, gone after the restore + rebuild).
# The ctest regex must match at least one test (a regex matching nothing is "green" and proves nothing -- refused).
# Output = a Markdown table you can paste into the MR description.
import argparse, hashlib, os, re, shutil, subprocess, sys, tempfile, time

OPS = [
    (re.compile(r'=='), '!='), (re.compile(r'!='), '=='),
    (re.compile(r'(?<![<>=!])<=(?!=)'), '>'), (re.compile(r'(?<![<>=!])>=(?!=)'), '<'),
    (re.compile(r'(?<![<>\-=])<(?![<=])'), '>='), (re.compile(r'(?<![<>\-=])>(?![>=])'), '<='),
    (re.compile(r'&&'), '||'), (re.compile(r'\|\|'), '&&'),
    (re.compile(r'\btrue\b'), 'false'), (re.compile(r'\bfalse\b'), 'true'),
    (re.compile(r'!(?=[A-Za-z_(])'), ''),
]
INTLIT = re.compile(r'(?<![\w.])(\d+)(?![\w.])')


def code_spans(line):
    """(start, end) spans of the line that are code: stops at //, skips string / char literals."""
    spans, i, n, start = [], 0, len(line), 0
    while i < n:
        c = line[i]
        if c == '/' and i + 1 < n and line[i + 1] == '/':
            break
        if c in '"\'':
            spans.append((start, i))
            j = i + 1
            while j < n and line[j] != c:
                j += 2 if line[j] == chr(92) else 1
            i = j + 1
            start = i
            continue
        i += 1
    spans.append((start, min(i, n)))
    return [s for s in spans if s[1] > s[0]]


def mutants_for(line):
    out = []
    for a, b in code_spans(line):
        seg = line[a:b]
        for rx, rep in OPS:
            for m in rx.finditer(seg):
                out.append((a + m.start(), a + m.end(), rep, '%s -> %s' % (m.group(0), rep or '(drop)')))
        for m in INTLIT.finditer(seg):
            if 'include' in line or '#' in line.lstrip()[:1]:
                continue
            out.append((a + m.start(), a + m.end(), str(int(m.group(1)) + 1), '%s -> %d' % (m.group(1), int(m.group(1)) + 1)))
    code = line.split('//')[0].strip()
    if code.endswith(';') and not code.startswith(('return', 'case', 'break', '}', '{')) and '(' in code:
        out.append((None, None, None, 'DEL (statement commented out)'))
    return out


def run(cmd, cwd=None, timeout=1800):
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, timeout=timeout)
    return r.returncode, (r.stdout + r.stderr).decode('utf-8', 'replace')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--file', required=True, help='source file, repo-relative or absolute')
    ap.add_argument('--lines', required=True, help='A-B (1-based, inclusive)')
    ap.add_argument('--build-dir', required=True)
    ap.add_argument('--target', required=True, help='CMake target that compiles the file (the test exe)')
    ap.add_argument('--ctest', required=True, help='ctest -R regex')
    ap.add_argument('--max', type=int, default=12)
    ap.add_argument('--allow-f5', action='store_true')
    a = ap.parse_args()

    bd = os.path.abspath(a.build_dir)
    if bd.lower().rstrip('\\/').endswith(os.path.join('obj', 'v906', 'build_dbg')) and not a.allow_f5:
        sys.exit('refused: %s is the F5 build dir' % bd)
    path = os.path.abspath(a.file)
    top = run(['git', 'rev-parse', '--show-toplevel'], cwd=os.path.dirname(path))[1].strip()
    rc, _ = run(['git', 'diff', '--quiet', '--', path], cwd=top)
    if rc != 0:
        sys.exit('refused: %s has uncommitted changes -- commit or set them aside first' % path)
    lo, hi = [int(x) for x in a.lines.split('-')]
    orig = open(path, 'rb').read()
    orig_sha = hashlib.sha1(orig).hexdigest()
    keep = os.path.join(tempfile.gettempdir(), 'mutation_check_%s_%s' % (os.path.basename(path), orig_sha[:8]))
    shutil.copyfile(path, keep)
    print('original saved to %s (sha1 %s); if this run is killed, copy it back over %s' % (keep, orig_sha[:12], path),
          file=sys.stderr)
    eol = b'\r\n' if b'\r\n' in orig else b'\n'
    lines = orig.split(eol)
    cand = []
    for ln in range(lo, hi + 1):
        text = lines[ln - 1].decode('utf-8', 'surrogateescape')
        for s, e, rep, what in mutants_for(text):
            cand.append((ln, s, e, rep, what))
    if len(cand) > a.max:
        total = len(cand)
        step = total / float(a.max)
        cand = [cand[int(i * step)] for i in range(a.max)]
        print('NOTE: %d candidate mutants in %s:%s, sampled %d evenly (narrow --lines or raise --max to try them all)'
              % (total, os.path.basename(path), a.lines, a.max), file=sys.stderr)

    rc, out = run(['cmake', '--build', bd, '--target', a.target])
    if rc != 0:
        sys.exit('baseline build failed:\n' + out[-2000:])
    rc, out = run(['ctest', '-R', a.ctest, '--output-on-failure'], cwd=bd)
    if rc != 0:
        sys.exit('baseline ctest is not green -- fix that first:\n' + out[-2000:])
    m = re.search(r'tests passed, \d+ tests failed out of (\d+)', out)
    if not m or int(m.group(1)) == 0:                       # a regex that matches no test is "green" -- and proves nothing
        sys.exit('baseline ran no test for -R %r -- check the name with `ctest -N -R ...`' % a.ctest)
    print('baseline: %s test(s) green for -R %s' % (m.group(1), a.ctest), file=sys.stderr)

    rows = []
    try:
        for ln, s, e, rep, what in cand:
            text = lines[ln - 1].decode('utf-8', 'surrogateescape')
            if s is None:
                ind = len(text) - len(text.lstrip())
                new = text[:ind] + '//MUTANT ' + text[ind:]
            else:
                new = text[:s] + rep + text[e:]
            mut = list(lines)
            mut[ln - 1] = new.encode('utf-8', 'surrogateescape')
            open(path, 'wb').write(eol.join(mut))
            t0 = time.time()
            rc, out = run(['cmake', '--build', bd, '--target', a.target])
            if rc != 0:
                err = next((l.strip() for l in out.splitlines() if 'error' in l.lower()), out.strip().splitlines()[-1] if out.strip() else '')
                rows.append((ln, what, 'build-broke', err[:140]))
            else:
                rc, out = run(['ctest', '-R', a.ctest, '--output-on-failure'], cwd=bd)
                if rc != 0:
                    fl = next((l.strip() for l in out.splitlines() if 'FAIL' in l and ('CHECK' in l or '[' in l)), '')
                    rows.append((ln, what, 'CAUGHT', fl[:140]))
                else:
                    rows.append((ln, what, 'survived', ''))
            print('  %s:%d %-28s %s (%.0fs)' % (os.path.basename(path), ln, what, rows[-1][2], time.time() - t0), file=sys.stderr)
    finally:
        open(path, 'wb').write(orig)
        back = hashlib.sha1(open(path, 'rb').read()).hexdigest()
        print('restored %s: %s' % (path, 'OK' if back == orig_sha else 'MISMATCH -- copy %s back!' % keep), file=sys.stderr)
        if back == orig_sha:
            os.remove(keep)
        run(['cmake', '--build', bd, '--target', a.target])      # leave the build dir matching the real source

    caught = sum(1 for r in rows if r[2] == 'CAUGHT')
    useful = sum(1 for r in rows if r[2] != 'build-broke')
    print('### Red check: `%s:%s` vs `ctest -R %s`' % (os.path.relpath(path, top).replace(os.sep, '/'), a.lines, a.ctest))
    print('')
    print('%d of %d mutants caught (%d did not build).' % (caught, useful, len(rows) - useful))
    print('')
    print('| line | change | result | first failing check |')
    print('|---|---|---|---|')
    for ln, what, res, fl in rows:
        print('| %d | `%s` | %s | %s |' % (ln, what.replace('|', '\\|'), res, fl.replace('|', '/')))
    if useful and caught < useful:
        print('')
        print('"survived" = the tests stayed green with that line broken: either add a pin for it or say why it does not matter.')


if __name__ == '__main__':
    main()
