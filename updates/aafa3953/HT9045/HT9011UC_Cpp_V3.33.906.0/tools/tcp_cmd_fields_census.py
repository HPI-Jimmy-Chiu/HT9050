# -*- coding: utf-8 -*-
"""tcp_cmd_fields_census.py -- how many comma-terminated fields each TCP command (7016) reads.

AI(W906-W10) 20260927 (St02-E).  W10 R2: the framer (TesterComm/Tcp/TcpCmdFramer.cpp) ends a command at the comma
its id needs -- exactly where golden's TCPCommandServerClientRead stops reading sData[0..3].  That table is
load-bearing (a wrong count joins two commands or splits one), so it is derived from the code, not typed by hand:

  * every branch `sData[0]=="HTxx" && sData[1]=="NNN"` of TfMain::TCPCommandServerClientRead in Command.cpp,
  * the highest sData[k] that branch's code mentions (comments and /* */ blocks removed; `#if 0` blocks KEPT --
    golden's parser reads the field whether or not the port has gated the line that uses it),
  * fields = k + 1 (head and id included): 2 / 3 / 4.

Usage:
    python tools/tcp_cmd_fields_census.py                    # print the table
    python tools/tcp_cmd_fields_census.py --check            # ctest TesterComm_TcpCmdFieldsCensus: the framer's
                                                             #   table must equal the one derived here (exit 1)
    python tools/tcp_cmd_fields_census.py --golden <Command.cpp> [--golden-enc cp950]
                                                             # the same census on a golden tree (read-only)
"""
import io
import os
import re
import sys

HEAD_RE = re.compile(r'sData\[0\]\s*==\s*"(HT[A-Z]+)"\s*&&\s*sData\[1\]\s*==\s*"(\d+)"')
USE_RE = re.compile(r'sData\[(\d)\]')
FUNC_RE = re.compile(r'^\s*void\s+(__fastcall\s+)?TfMain::TCPCommandServerClientRead\s*\(')


def strip_line_comment(s):
    out, in_str, i = [], False, 0
    while i < len(s):
        c = s[i]
        if in_str:
            if c == '\\':
                out.append(s[i:i + 2])
                i += 2
                continue
            if c == '"':
                in_str = False
        else:
            if c == '"':
                in_str = True
            elif s.startswith('//', i):
                break
        out.append(c)
        i += 1
    return ''.join(out)


def census(lines):
    start = None
    for i, l in enumerate(lines):
        if FUNC_RE.match(l):
            start = i
            break
    if start is None:
        raise SystemExit('TCPCommandServerClientRead not found')
    fields, order, cur, in_block = {}, [], None, False
    for i in range(start + 2, len(lines)):
        raw = lines[i]
        if raw.startswith('}'):
            break                                   # the function's closing brace (column 0)
        s = raw
        if in_block:
            if '*/' not in s:
                continue
            s = s.split('*/', 1)[1]
            in_block = False
        while '/*' in strip_line_comment(s):
            a, rest = s.split('/*', 1)
            if '*/' in rest:
                s = a + rest.split('*/', 1)[1]
            else:
                s = a
                in_block = True
                break
        code = strip_line_comment(s)
        m = HEAD_RE.search(code)
        if m:
            cur = (m.group(1), m.group(2))
            if cur not in fields:
                fields[cur] = 2
                order.append(cur)
        if cur:
            for k in USE_RE.findall(code[m.end():] if m else code):
                fields[cur] = max(fields[cur], int(k) + 1)
    return fields, order


def framer_table(path):
    text = io.open(path, encoding='utf-8').read()
    a = text.index('W10-FIELDS-TABLE-BEGIN')
    b = text.index('W10-FIELDS-TABLE-END')
    body = text[a:b]
    def ids(name):
        m = re.search(name + r'\[\]\s*=\s*\{([^}]*)\}', body)
        return set(re.findall(r'"(\d+)"', m.group(1))) if m else set()
    return ids('kThreeFields'), ids('kFourFields')


def main(argv):
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    src, enc = os.path.join(root, 'Command.cpp'), 'utf-8'
    if '--golden' in argv:
        src = argv[argv.index('--golden') + 1]
        enc = argv[argv.index('--golden-enc') + 1] if '--golden-enc' in argv else 'cp950'
    lines = io.open(src, 'rb').read().decode(enc, 'replace').replace('\r\n', '\n').split('\n')
    fields, order = census(lines)
    by = {}
    for k in order:
        by.setdefault(fields[k], []).append('%s,%s' % k)
    print('%s: %d commands' % (src, len(order)))
    for n in sorted(by):
        print('  %d fields (%d): %s' % (n, len(by[n]), ' '.join(by[n])))
    if '--check' not in argv:
        return 0
    three, four = framer_table(os.path.join(root, 'TesterComm', 'Tcp', 'TcpCmdFramer.cpp'))
    want3 = set(i for (h, i), n in fields.items() if n == 3 and h == 'HTSET')
    want4 = set(i for (h, i), n in fields.items() if n == 4 and h == 'HTSET')
    # HTSET,720 reads sData[3] only when it is there: the framer lists it with 3 fields + an optional 4th
    if '720' in want4:
        want4.discard('720')
        want3.add('720')
    other = sorted('%s,%s' % k for k, n in fields.items() if n > 2 and k[0] != 'HTSET')
    ok = (three == want3 and four == want4 and not other)
    if not ok:
        sys.stderr.write('FAIL framer table != Command.cpp\n  3 fields: framer %s / code %s\n  4 fields: framer %s / '
                         'code %s\n  non-HTSET ids with > 2 fields (the framer assumes 2): %s\n'
                         % (sorted(three), sorted(want3), sorted(four), sorted(want4), other))
        return 1
    print('PASS the framer table (TesterComm/Tcp/TcpCmdFramer.cpp) matches Command.cpp: %d three-field, %d four-field'
          % (len(three), len(four)))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
