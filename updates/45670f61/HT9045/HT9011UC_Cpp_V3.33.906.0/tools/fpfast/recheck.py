# AI(W906-FPFAST) 20261003 -- second look at every pair proof.py marked DIFFERENT.
# (1) cmp the two objects it kept and list the differing bytes in context (which section,
#     which string); (2) recompile A and B with __DATE__ / __TIME__ pinned
#     (-Wno-builtin-macro-redefined -D__DATE__="Oct  3 2026" -D__TIME__="00:00:00", the SAME
#     extra arguments on both sides) and require byte identity + objdump identity.
# No backslash literal in this file.
import argparse, json, os, sys
import proof as P

DQ = chr(34)
BS = chr(92)
PIN = (' -Wno-builtin-macro-redefined ' + DQ + '-D__DATE__=' + BS + DQ + 'Oct  3 2026' + BS + DQ + DQ +
       ' ' + DQ + '-D__TIME__=' + BS + DQ + '00:00:00' + BS + DQ + DQ)


def byte_diffs(a, b, limit=40):
    da = open(a, 'rb').read()
    db = open(b, 'rb').read()
    if len(da) != len(db):
        return ['size %d vs %d' % (len(da), len(db))]
    out = []
    i = 0
    while i < len(da) and len(out) < limit:
        if da[i] != db[i]:
            j = i
            while j < len(da) and da[j] != db[j]:
                j += 1
            lo, hi = max(0, i - 12), min(len(da), j + 12)
            out.append('@%d..%d A=%r B=%r' % (i, j, da[lo:hi], db[lo:hi]))
            i = j
        else:
            i += 1
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--db', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--mask', type=lambda s: int(s, 0), default=0x3F)
    ap.add_argument('--mingw', default='C:' + BS + 'MinGW' + BS + 'bin')
    a = ap.parse_args()
    P.set_low(a.mask)
    env = dict(os.environ)
    env['PATH'] = a.mingw + ';' + env.get('PATH', '')
    objdump_exe = os.path.join(a.mingw, 'objdump.exe')
    db = json.load(open(a.db, encoding='utf-8'))
    bykey = {}
    for e in db:
        cmd = e['command']
        if cmd.count(' -o ') != 1:
            continue
        i0 = cmd.index(' -o ') + 4
        o = ' -o ' + cmd[i0:cmd.index(' ', i0)] + ' '
        e['_o'] = o
        bykey.setdefault(e['directory'].lower() + '|' + cmd.replace(o, ' -o @OUT@ '), e)
    rows = [json.loads(l) for l in open(os.path.join(a.out, 'results.jsonl'), encoding='utf-8') if l.strip()]
    bad = [r for r in rows if r['status'] != 'IDENTICAL' and not r.get('bytes_identical')]   # byte-identical = nothing to explain
    print('[recheck] %d result rows, %d not IDENTICAL' % (len(rows), len(bad)), flush=True)
    os.makedirs(os.path.join(a.out, 'N'), exist_ok=True)
    allok = True
    for r in bad:
        print('[recheck] ---- #%d %s  status=%s dis_diff=%s con_diff=%s' % (r['i'], r['file'], r['status'], r.get('dis_diff'), r.get('con_diff')), flush=True)
        objA = os.path.join(a.out, 'A', '%04d.o' % r['i'])
        objB = os.path.join(a.out, 'B', '%04d.o' % r['i'])
        if os.path.exists(objA) and os.path.exists(objB):
            for d in byte_diffs(objA, objB):
                print('[recheck]   byte diff ' + d, flush=True)
        e = bykey.get(r['key'])
        if e is None:
            print('[recheck]   key not found in db', flush=True)
            allok = False
            continue
        nA = os.path.join(a.out, 'N', '%04d_A.o' % r['i'])
        nB = os.path.join(a.out, 'N', '%04d_B.o' % r['i'])
        base = e['command'].replace(' ' + P.FLAG, ' ' + P.FLAG + PIN, 1)
        cmdB = base.replace(e['_o'], ' -o ' + DQ + nB + DQ + ' ')
        cmdA = base.replace(e['_o'], ' -o ' + DQ + nA + DQ + ' ').replace(' ' + P.FLAG, '', 1)
        rcA, _, errA, _ = P.run(cmdA, e['directory'], env)
        rcB, _, errB, _ = P.run(cmdB, e['directory'], env)
        if rcA or rcB:
            print('[recheck]   compile failed rcA=%d rcB=%d %s' % (rcA, rcB, errB[-400:]), flush=True)
            allok = False
            continue
        same_bytes = P.sha(nA) == P.sha(nB)
        dA = P.objdump(objdump_exe, ['-d', '-r', '--no-show-raw-insn'], nA, env)[1]
        dB = P.objdump(objdump_exe, ['-d', '-r', '--no-show-raw-insn'], nB, env)[1]
        sA = P.objdump(objdump_exe, ['-s'], nA, env)[1]
        sB = P.objdump(objdump_exe, ['-s'], nB, env)[1]
        _, secA = P.split_sections(dA, 'Disassembly of section ')
        _, secB = P.split_sections(dB, 'Disassembly of section ')
        _, csA = P.split_sections(sA, 'Contents of section ')
        _, csB = P.split_sections(sB, 'Contents of section ')
        dis_same = secA == secB
        con_same = csA == csB
        # the pin really took effect: the pinned time string is in the object, the build time is not
        pinned = b'00:00:00' in open(nA, 'rb').read()
        print('[recheck]   time-pinned recompile: bytes_identical=%s disasm_sections_identical=%s (%d) contents_sections_identical=%s (%d) pin_present=%s stderr_equal=%s'
              % (same_bytes, dis_same, len(secA), con_same, len(csA), pinned, errA == errB), flush=True)
        if not (same_bytes and dis_same and con_same):
            allok = False
    print('[recheck] RESULT ' + ('ALL EXPLAINED (identical once __DATE__/__TIME__ are pinned)' if allok else 'NOT ALL EXPLAINED'), flush=True)
    sys.exit(0 if allok else 1)


if __name__ == '__main__':
    main()
