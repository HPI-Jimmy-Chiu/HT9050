"""bs_comment_census.py -- AI(W906-BSCOMMENT) 20261010 (laptop): find `//` comments that end with a backslash.

Why: GCC splices a backslash-newline BEFORE it strips comments (C++ translation phase 2), so in
    x = 2;   // ... \\<newline>
    x = 3;
the second line becomes part of the comment and never runs.  BCB6 bcc32 5.6.4 (the golden compiler) does NOT do that --
measured 20261010 21:4x on the laptop with a probe carrying the exact golden 913 bytes (uLotInfo.cpp:3679 / :3777:
Big5 + two ASCII backslashes + CRLF; :3630 Big5 trail byte 0x5C; backslash + space):
    bcc32 5.6.4 -> a=3 b=3 c=3  (every next line ran)        g++ 6.3 -Wall -> a=2 b=2 c=2 + "multi-line comment"
So a golden line that follows such a comment RUNS on the production machine and silently VANISHES in this port.
The faithful translation keeps the line alive (drop the trailing backslash from the comment); it is NOT a golden defect.

GCC's own -Wcomment already reports every site, but in a build log of thousands of lines nobody reads it, so this census
is a ctest: it counts the sites whose swallowed line holds code (CODE) and fails when that number changes.
Lines that only swallow a blank line or another comment (golden ASCII-art boxes `//  \\`, paths in notes) are listed
with --list but not counted.

usage: python tools/bs_comment_census.py [--root <port tree>] [--list] [--check-code N] [--selftest]
exit:  0 ok / 1 --check-code mismatch or selftest failure / 2 usage"""
import os
import sys

BS = 92
EXT = ('.cpp', '.h', '.hpp', '.inc', '.c')
SKIP_DIRS = {'.git', '.svn', 'build', 'node_modules', '__pycache__'}


def physical_lines(data):
    """Split like the C preprocessor: CRLF, CR and LF all end a physical line."""
    out, cur, i, n = [], bytearray(), 0, len(data)
    while i < n:
        c = data[i]
        if c == 13:
            out.append(bytes(cur))
            cur = bytearray()
            i += 2 if i + 1 < n and data[i + 1] == 10 else 1
            continue
        if c == 10:
            out.append(bytes(cur))
            cur = bytearray()
            i += 1
            continue
        cur.append(c)
        i += 1
    out.append(bytes(cur))
    return out


def ends_with_backslash(line):
    # GCC also splices backslash + blanks + newline (with a "backslash and newline separated by space" warning)
    return line.rstrip(b' \t').endswith(bytes([BS]))


def scan_bytes(data):
    """Return [(line, col, kind, comment_line, [swallowed lines])]; kind = CODE / harmless."""
    L = physical_lines(data)
    hits = []
    in_block = False
    k = 0
    while k < len(L):
        s = L[k]
        j, n = 0, len(s)
        comment_at = -1
        while j < n:
            if in_block:
                e = s.find(b'*/', j)
                if e < 0:
                    j = n
                    break
                in_block = False
                j = e + 2
                continue
            c = s[j]
            if c in (34, 39):                      # "..." or '...' (this tree has no raw strings)
                q = c
                j += 1
                while j < n and s[j] != q:
                    j += 2 if s[j] == BS else 1
                j += 1
                continue
            if s.startswith(b'/*', j):
                in_block = True
                j += 2
                continue
            if s.startswith(b'//', j):
                comment_at = j
                break
            j += 1
        if comment_at >= 0 and ends_with_backslash(s):
            swallowed = []
            m = k
            while m + 1 < len(L) and ends_with_backslash(L[m]):
                swallowed.append(L[m + 1])
                m += 1
            code = any(x.strip() and not x.strip().startswith(b'//') for x in swallowed)
            hits.append((k + 1, comment_at + 1, 'CODE' if code else 'harmless', s, swallowed))
            k = m + 1
            continue
        k += 1
    return hits


def selftest():
    b = bytes([BS])
    cases = [
        ('golden 913 :3679 shape (Big5 + two backslashes + CRLF)',
         b'a = 2;  //add HotPlate.Data\xa4\xd6' + b + b + b'\r\na = 3;\r\n', ['CODE']),
        ('Big5 trail byte 0x5C at the end (golden :3630 gai)', b'b = 2; //\xbb\x5c\r\nb = 3;\r\n', ['CODE']),
        ('backslash + space', b'c = 2; // x ' + b + b' \nc = 3;\n', ['CODE']),
        ('ASCII-art box swallows only a comment', b'//  ' + b + b + b'\n// <---\nint x;\n', ['harmless']),
        ('swallows a blank line', b'// path D:' + b + b'HT9045' + b + b'\n\nint x;\n', ['harmless']),
        ('chain: two comment lines then code', b'// a ' + b + b'\n// b ' + b + b'\nint x;\n', ['CODE']),
        ('// inside a string is not a comment', b's = "http://x";  y = 1;\nz = 2;\n', []),
        ('macro continuation outside a comment is not counted', b'#define M(a) ' + b + b'\n  (a)\n', []),
        ('// inside a block comment', b'/* // ' + b + b' */ int x;\nint y;\n', []),
        ('block comment spanning lines, then a real one',
         b'/* line one\n line two */ int x; // tail ' + b + b'\nint y;\n', ['CODE']),
        ('escaped quote inside a string', b'p = "a' + b + b'"//";  // ok\nq = 1;\n', []),
    ]
    bad = 0
    for name, data, want in cases:
        got = [h[2] for h in scan_bytes(data)]
        ok = got == want
        bad += 0 if ok else 1
        print('%s  %s  (want %s, got %s)' % ('PASS' if ok else 'FAIL', name, want, got))
    print('selftest: %d / %d passed' % (len(cases) - bad, len(cases)))
    return 0 if bad == 0 else 1


def main(argv):
    if '--selftest' in argv:
        return selftest()
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    if '--root' in argv:
        root = argv[argv.index('--root') + 1]
    want = None
    if '--check-code' in argv:
        try:
            want = int(argv[argv.index('--check-code') + 1])
        except (IndexError, ValueError):
            print(__doc__)
            return 2
    files = []
    for dp, dn, fn in os.walk(root):
        dn[:] = sorted(d for d in dn if d not in SKIP_DIRS and not d.startswith('build_') and not d.startswith('Obj'))
        for f in sorted(fn):
            if f.lower().endswith(EXT):
                files.append(os.path.join(dp, f))
    rows = []
    for fp in files:
        try:
            data = open(fp, 'rb').read()
        except OSError:
            continue
        for ln, col, kind, s, sw in scan_bytes(data):
            rows.append((os.path.relpath(fp, root).replace(os.sep, '/'), ln, col, kind, s, sw))
    code = [r for r in rows if r[3] == 'CODE']
    for r in rows:
        if r[3] == 'CODE' or '--list' in argv:
            print('%-8s %s:%d:%d' % (r[3], r[0], r[1], r[2]))
            print('         comment : %s' % r[4].decode('utf-8', 'replace').strip()[-110:])
            for x in r[5]:
                print('         swallowed: %s' % x.decode('utf-8', 'replace').strip()[:120])
    print('bs_comment_census: %d files, %d // comments end with a backslash, %d swallow code, %d swallow only blank / comment lines'
          % (len(files), len(rows), len(code), len(rows) - len(code)))
    if want is not None and len(code) != want:
        print('CHECK FAILED: %d code-swallowing site(s), expected %d.  A new one means a golden line that runs under BCB6 '
              'vanishes under GCC -- drop the trailing backslash from that comment (same line).  If you removed one, '
              'lower the number in tests/CMakeLists.txt (BsCommentSplice).' % (len(code), want))
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
