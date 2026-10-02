"""Rebuild tests/CMakeLists.txt for a merge: theirs (main) + the branch's pure insertions vs the merge base.

usage: python cmake_rebase_union.py <repo_dir> <base_rev> <ours_rev> <theirs_rev>
Writes the result into <repo_dir>/HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt (CRLF if the working file was CRLF).
Rules:
  * every ours hunk must be a pure insertion (-a,0 +b,n); anything else -> abort (resolve by hand) -- except:
    - main changed exactly the same base line(s) too (the START census pin): main's line is kept and the script says so
    - only ours changed those lines (same line count, main untouched): ours' version is kept (plain 3-way result)
  * an ours insertion after base line a goes after the theirs line that base line a became; if theirs also
    inserted right after a, ours goes AFTER theirs (the laptop's "append after main's tests" rule)
  * if a theirs hunk modifies/deletes base line a (or the line right after), abort
  * an ours block that main already has verbatim (the same change reached main another way, e.g. a cherry-pick) is skipped
  * prints if( / endif( / add_test( counts and checks result == theirs + ours - base - skipped, the add_test name set,
    and no duplicate names
Then: git add the file, run the START census (tools/start_sites_census.py) and write what it measures, `cmake .`, build, ctest.
Origin: ST01-E 20261001 (q59 a5f33032, d026 0f72c710); a plain keep-both union lost one if( on each branch (ST01-M's dry run).
"""
import os, re, subprocess, sys

repo, base, ours, theirs = sys.argv[1:5]
F = 'HT9011UC_Cpp_V3.33.906.0/tests/CMakeLists.txt'

def show(rev):
    out = subprocess.run(['git', '-C', repo, 'show', '%s:%s' % (rev, F)], capture_output=True, check=True).stdout
    return out.decode('utf-8').replace('\r\n', '\n').split('\n')

def hunks(a, b):
    out = subprocess.run(['git', '-C', repo, 'diff', '-U0', '--no-color', a, b, '--', F],
                         capture_output=True, check=True).stdout.decode('utf-8')
    hs = []
    for m in re.finditer(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@', out, re.M):
        a0 = int(m.group(1)); al = int(m.group(2)) if m.group(2) is not None else 1
        b0 = int(m.group(3)); bl = int(m.group(4)) if m.group(4) is not None else 1
        hs.append((a0, al, b0, bl))
    return hs

B, O, T = show(base), show(ours), show(theirs)
oh, th = hunks(base, ours), hunks(base, theirs)
ins = []
both = []
mods = []                                   # ours-only in-place changes (theirs never touched those base lines)
for a0, al, b0, bl in oh:
    if al != 0:
        # both sides changed the very same base lines (e.g. the START census --check pin): keep main's, report it
        if al == bl and any(ta == a0 and tal == al for ta, tal, tb, tbl in th):
            both.append((a0, al, O[b0 - 1:b0 - 1 + bl]))
            continue
        # only ours changed these lines (same line count): a plain 3-way result = ours' version
        if al == bl and not any(ta <= a0 + al - 1 and a0 <= ta + max(tal, 1) - 1 for ta, tal, tb, tbl in th if tal):
            mods.append((a0, al, O[b0 - 1:b0 - 1 + bl]))
            continue
        sys.exit('ABORT: ours hunk -%d,%d +%d,%d is not a pure insertion' % (a0, al, b0, bl))
    ins.append((a0, O[b0 - 1:b0 - 1 + bl]))     # insert after base line a0 (1-based; 0 = top)
for a0, al, lines in both:
    print('BOTH CHANGED base line(s) %d..%d: kept MAIN\'s version; ours was: %s  -> fix by hand (census: re-measure)'
          % (a0, a0 + al - 1, ' | '.join(s.strip()[:120] for s in lines)))

def theirs_pos(a):
    """index in T after which to insert (count of T lines before the insertion point)."""
    off = 0
    same = 0                              # theirs lines inserted right after base line a (ours goes after them)
    for ta, tal, tb, tbl in th:
        if tal == 0:                      # theirs pure insertion after base line ta
            if ta < a: off += tbl
            elif ta == a: same += tbl
        else:                             # theirs modifies base lines ta..ta+tal-1
            if ta <= a + 1 and a <= ta + tal - 1:
                sys.exit('ABORT: theirs changes base line(s) %d..%d next to ours insertion after %d' % (ta, ta + tal - 1, a))
            if ta + tal - 1 < a: off += tbl - tal
    return a + off, same

skipped = []
R = list(T)
for a0, al, lines in mods:                  # same line count, so later insertion positions do not move
    pa, same = theirs_pos(a0 - 1)
    idx = pa + same                         # 0-based index of base line a0 in theirs
    assert R[idx:idx + al] == B[a0 - 1:a0 - 1 + al], ('ours-only change: theirs differs at base', a0)
    R[idx:idx + al] = lines
    print('applied ours-only change at base line(s) %d..%d (main %d)' % (a0, a0 + al - 1, idx + 1))
TJ = '\n' + '\n'.join(T) + '\n'
T_NAMES = set(re.findall(r'add_test\s*\(\s*NAME\s+([A-Za-z0-9_]+)', '\n'.join(T)))
def split_chunks(lines):
    """split an inserted block at '# --- ' test banners (each test block in tests/CMakeLists.txt starts with one)."""
    cuts = [k for k, s in enumerate(lines) if s.startswith('# --- ') and k > 0]
    out, last = [], 0
    for k in cuts:
        out.append(lines[last:k]); last = k
    out.append(lines[last:])
    return out
for a, lines in sorted(ins, key=lambda x: -x[0]):   # bottom-up so earlier positions stay valid
    if lines and ('\n' + '\n'.join(lines) + '\n') in TJ:
        print('skipped %d lines after base %d: already in main verbatim (same change reached main another way)' % (len(lines), a))
        skipped.extend(lines)
        continue
    # sub-blocks whose add_test names ALL exist in main already (e.g. a cherry-pick in different text): drop them
    kept = []
    for ch in split_chunks(lines):
        names = re.findall(r'add_test\s*\(\s*NAME\s+([A-Za-z0-9_]+)', '\n'.join(ch))
        if names and all(n in T_NAMES for n in names):
            print('dropped a %d-line sub-block after base %d: its tests %s already exist in main' % (len(ch), a, names))
            skipped.extend(ch)
        else:
            kept.extend(ch)
    lines = kept
    if not lines:
        continue
    pa, same = theirs_pos(a)
    if a > 0:
        assert T[pa - 1] == B[a - 1], ('anchor mismatch', a, repr(B[a - 1][:60]), repr(T[pa - 1][:60]))
    p = pa + same
    if same and lines and lines[0].strip() != '' and T[p - 1].strip() != '':
        lines = [''] + lines                            # keep a blank line between main's block and ours
    R[p:p] = lines
    print('inserted %d lines after base %d (main %d%s)' % (len(lines), a, pa, ', after main\'s %d' % same if same else ''))

def counts(L):
    s = '\n'.join(L)
    return (len(re.findall(r'^\s*if\s*\(', s, re.M)), len(re.findall(r'^\s*endif\s*\(', s, re.M)),
            len(re.findall(r'add_test\s*\(\s*NAME\b', s)))
cb, co, ct, cr = counts(B), counts(O), counts(T), counts(R)
cs = counts(skipped)
exp = tuple(ct[i] + co[i] - cb[i] - cs[i] for i in range(3))
print('base', cb, 'ours', co, 'theirs', ct, '-> expected', exp, 'result', cr)
if cr != exp or cr[0] != cr[1]:
    sys.exit('ABORT: counts differ')
names = lambda L: sorted(re.findall(r'add_test\s*\(\s*NAME\s+([A-Za-z0-9_]+)', '\n'.join(L)))
nb, no, nt, nr = set(names(B)), set(names(O)), set(names(T)), names(R)
want = sorted((nt | (no - nb)) - (nb - no))
dups = sorted(set(n for n in nr if nr.count(n) > 1))
if dups:
    sys.exit('ABORT: duplicate add_test names after the union (the same test reached both sides in different text, '
             'e.g. a cherry-pick): %s -- drop our copy of those blocks by hand' % dups)
if nr != want:
    sys.exit('ABORT: add_test name set differs: missing %s extra %s' % (sorted(set(want) - set(nr)), sorted(set(nr) - set(want))))
if len(nr) != len(set(nr)):
    sys.exit('ABORT: duplicate add_test names')
p = os.path.join(repo, F.replace('/', os.sep))
crlf = b'\r\n' in open(p, 'rb').read()
txt = '\n'.join(R)
open(p, 'wb').write((txt.replace('\n', '\r\n') if crlf else txt).encode('utf-8'))
print('written', p, 'lines', len(R), 'crlf', crlf, 'add_tests', len(nr))
