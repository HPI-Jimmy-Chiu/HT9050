#!/usr/bin/env python3
# AI(W906-S1) 20261007 (Ifor01; W-124): objcompare -- did a source change (header split, include moves, ...) change any code?
#   Header-slimming plan (.claude/skills/hpi-build/references/cpp/references/header-slimming-and-motor-io-isolation-20261006.md
#   s A5, "verify nothing changed" step 5: "the tree has no objcompare tool yet").
#
# usage: python tools/objcompare.py <before_build_dir> <after_build_dir> [--jobs=N] [--show=N]
#   Build the same source tree twice with the same toolchain and flags (before / after the change), into two build dirs
#   whose names have the SAME LENGTH (e.g. objcmp_before_sim / objcmp_after__sim): a few TUs carry the build dir in a -D
#   (cbootlog BOOT_LOG_DIR ...); with equal lengths that string is swapped before comparing and nothing shifts.
#   Every *.obj / *.o of the before dir is paired with the same relative path in the after dir and compared on
#     1. the list of sections (name + flags) and every section's raw bytes, the before dir path swapped for the after dir path;
#     2. every section's relocations as (offset, target symbol NAME, type) -- a call that now goes to another function shows
#        here even when the bytes are the same;
#     3. the symbol table as a sorted list of (name, value, section name, type, storage class, aux records) -- the size of
#        .bss and of COMDAT sections lives only in the section symbol's aux record (int a[100] -> a[101] changes nothing else).
#   .debug* sections (and the symbols / relocations in them) are skipped: line numbers move whenever a header changes, so
#   debug info always differs.  This is plan step 2 (strip-debug, then compare code, nm and data) done byte-for-byte,
#   which is stricter than comparing disassembly.
#   The objects are read directly (PE/COFF, i386 and x86-64, normal and /bigobj): MinGW binutils 2.28 needs 10-20 s per
#   object for `objdump -s` / `-r` on objects with a few hundred COMDAT sections, i.e. hours for a whole tree.
#   Known noise, reported but not counted as a difference: __DATE__ / __TIME__ strings (cObserver.cpp, WebBridgeTags.cpp,
#   TempCtrl/TriTemp.cpp in this tree) -- a section whose only differing bytes are inside date / time strings.
#   Exit 0 = no difference (noise aside), 1 = differences, 2 = usage / tool problem.
import concurrent.futures, os, re, struct, sys

RE_DATE = re.compile(rb'(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [ 0-3]\d \d{4}|\d\d:\d\d:\d\d')
BIGOBJ_CLSID = bytes.fromhex('c7a1bad1eebaa94baf20faf66aa4dcb8')
SCN_LNK_NRELOC_OVFL = 0x01000000

def read_coff(d):
    """object file bytes -> (sections [((name, characteristics, size), raw bytes, [(offset, symbol name, type)])],
                             symbols [(name, value, section, type, class, aux)], has .debug sections,
                             file offsets of the section checksums)"""
    sig1, sig2 = struct.unpack_from('<HH', d, 0)
    if sig1 == 0 and sig2 == 0xFFFF and d[12:28] == BIGOBJ_CLSID:
        nsec, symptr, nsym = struct.unpack_from('<III', d, 44)
        sechdr, symsize, secfmt = 56, 20, '<i'
    else:
        nsec, = struct.unpack_from('<H', d, 2)
        symptr, nsym, optsize = struct.unpack_from('<IIH', d, 8)
        sechdr, symsize, secfmt = 20 + optsize, 18, '<h'
    strtab = symptr + nsym * symsize

    def name_at(raw8, long_prefix):
        if long_prefix and raw8[:1] == b'/':
            off = int(raw8[1:].split(b'\0')[0] or b'0')
            return d[strtab + off:d.index(b'\0', strtab + off)].decode('latin-1')
        if not long_prefix and raw8[:4] == b'\0\0\0\0':
            off, = struct.unpack_from('<I', raw8, 4)
            return d[strtab + off:d.index(b'\0', strtab + off)].decode('latin-1')
        return raw8.split(b'\0')[0].decode('latin-1')

    secnames = []
    for i in range(nsec):
        h = d[sechdr + 40 * i:sechdr + 40 * (i + 1)]
        secnames.append(name_at(h[:8], True))
    syms, symname, cksum = [], {}, []
    i = 0
    while i < nsym:
        o = symptr + i * symsize
        name = name_at(d[o:o + 8], False)
        value, = struct.unpack_from('<I', d, o + 8)
        sec, = struct.unpack_from(secfmt, d, o + 12)
        so = o + 12 + (4 if symsize == 20 else 2)
        typ, cls, naux = struct.unpack_from('<HBB', d, so)
        symname[i] = name
        secn = secnames[sec - 1] if 0 < sec <= nsec else str(sec)
        if not secn.startswith('.debug'):
            # aux records count: MinGW puts the size of .bss / of each COMDAT section only in the section symbol's aux record.
            # Its CheckSum (bytes 8-11) is of the section bytes, which are compared on their own after the path swap.
            aux = d[o + symsize:o + symsize * (1 + naux)]
            if cls == 3 and naux and name == secn:
                aux = aux[:8] + b'\0\0\0\0' + aux[12:]
                cksum.append(o + symsize + 8)
            syms.append((name, value, secn, typ, cls, aux))
        i += 1 + naux
    sections = []
    for i in range(nsec):
        h = d[sechdr + 40 * i:sechdr + 40 * (i + 1)]
        vsize, vaddr, rawsize, rawptr, relptr, lnptr, nrel, nln, chars = struct.unpack_from('<IIIIIIHHI', h, 8)
        name = secnames[i]
        if name.startswith('.debug'):
            continue
        raw = d[rawptr:rawptr + rawsize] if rawptr else b''   # .bss: no bytes, the size is in the section list below
        first = 0
        if chars & SCN_LNK_NRELOC_OVFL and nrel == 0xFFFF:
            nrel, = struct.unpack_from('<I', d, relptr)   # the real count sits in the first entry
            first = 1
        rel = []
        for k in range(first, nrel):
            va, si, rt = struct.unpack_from('<IIH', d, relptr + 10 * k)
            rel.append((va, symname.get(si, '#%d' % si), rt))
        rel.sort()
        sections.append(((name, chars, rawsize), raw, rel))
    syms.sort()
    return sections, syms, any(n.startswith('.debug') for n in secnames), cksum

def date_noise(a, b):
    """True when a and b have the same length and every differing byte sits inside a date / time string"""
    if len(a) != len(b):
        return False
    bad = [i for i in range(len(a)) if a[i] != b[i]]
    spans = [(m.start(), m.end()) for buf in (a, b) for m in RE_DATE.finditer(buf)]
    return all(any(s <= i < e for s, e in spans) for i in bad)

def compare(job):
    rel, pa, pb, swap = job
    da, db = open(pa, 'rb').read(), open(pb, 'rb').read()
    for x, y in swap:
        da = da.replace(x, y)
    if da == db:
        return rel, [], []
    try:
        SA, YA, dbg_a, ck_a = read_coff(da)   # (path already swapped)
        SB, YB, dbg_b, ck_b = read_coff(db)
    except Exception as e:
        return rel, ['tool error: %r' % e], []
    notes, noise = [], []
    if [h for h, _, _ in SA] != [h for h, _, _ in SB]:
        ha, hb = [h for h, _, _ in SA], [h for h, _, _ in SB]
        k = next((i for i in range(min(len(ha), len(hb))) if ha[i] != hb[i]), min(len(ha), len(hb)))
        notes.append('section list differs (%d -> %d sections; first: %s -> %s)' % (
            len(ha), len(hb), ha[k] if k < len(ha) else '-', hb[k] if k < len(hb) else '-'))
    for ((n, _, _), a, ra), (_, b, rb) in zip(SA, SB):
        if a != b:
            if date_noise(a, b):
                noise.append(n)
            else:
                at = next((i for i in range(min(len(a), len(b))) if a[i] != b[i]), min(len(a), len(b)))
                notes.append('section %s bytes differ (%d -> %d bytes, first at +0x%x)' % (n, len(a), len(b), at))
        if ra != rb:
            k = next((i for i in range(min(len(ra), len(rb))) if ra[i] != rb[i]), min(len(ra), len(rb)))
            notes.append('section %s relocations differ (%d -> %d; first: %s -> %s)' % (
                n, len(ra), len(rb), ra[k] if k < len(ra) else '-', rb[k] if k < len(rb) else '-'))
    if YA != YB:
        sa, sb = set(YA), set(YB)
        brief = lambda s: [(n, v, sec, a.hex()[:24]) for n, v, sec, _, _, a in sorted(s)[:2]]
        notes.append('symbols differ (only before %s; only after %s)' % (brief(sa - sb), brief(sb - sa)))
    if not notes and not noise and not dbg_a and not dbg_b:
        # no debug info, so nothing but those section checksums may differ: anything else is in a part this tool does not decode
        fa, fb = bytearray(da), bytearray(db)
        for buf, offs in ((fa, ck_a), (fb, ck_b)):
            for off in offs:
                buf[off:off + 4] = b'\0\0\0\0'
        if fa != fb:
            at = next((i for i in range(min(len(fa), len(fb))) if fa[i] != fb[i]), min(len(fa), len(fb)))
            notes.append('file bytes differ outside the compared parts (%d -> %d bytes, first at +0x%x)' % (len(fa), len(fb), at))
    return rel, notes, noise

def objs(root):
    out = {}
    for dp, dn, fn in os.walk(root):
        for f in fn:
            if f.endswith('.obj') or f.endswith('.o'):
                p = os.path.join(dp, f)
                out[os.path.relpath(p, root).replace('\\', '/').lower()] = p
    return out

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    jobs = int(next((a.split('=')[1] for a in sys.argv if a.startswith('--jobs=')), os.cpu_count() or 4))
    show = int(next((a.split('=')[1] for a in sys.argv if a.startswith('--show=')), 30))
    if len(args) != 2 or not all(os.path.isdir(a) for a in args):
        print('usage: objcompare.py <before_build_dir> <after_build_dir> [--jobs=N] [--show=N]')
        return 2
    da, db = (os.path.abspath(a) for a in args)
    if len(os.path.basename(da)) != len(os.path.basename(db)):
        print('objcompare: the two build dir names must have the same length (%s / %s)' % (os.path.basename(da), os.path.basename(db)))
        return 2
    swap = []
    for x, y in ((da, db), (da.replace('\\', '/'), db.replace('\\', '/'))):
        for f in (lambda s: s, str.lower, lambda s: s[0].lower() + s[1:]):
            if (f(x).encode(), f(y).encode()) not in swap and f(x) != f(y):
                swap.append((f(x).encode(), f(y).encode()))
    A, B = objs(da), objs(db)
    only_a, only_b = sorted(set(A) - set(B)), sorted(set(B) - set(A))
    diffs, noise = [], []
    work = [(r, A[r], B[r], swap) for r in sorted(set(A) & set(B))]
    with concurrent.futures.ProcessPoolExecutor(jobs) as ex:
        for rel, notes, nz in ex.map(compare, work, chunksize=8):
            if notes:
                diffs.append((rel, notes))
            if nz:
                noise.append((rel, nz))
    print('objcompare: %d objects compared; %d differ; %d with date/time noise only; %d only before; %d only after'
          % (len(work), len(diffs), len(noise), len(only_a), len(only_b)))
    for rel, nz in noise[:show]:
        print('  noise  %s  (%s)' % (rel, ', '.join(nz)))
    for rel, notes in diffs[:show]:
        print('  DIFF   %s  : %s' % (rel, '; '.join(notes[:4]) + (' (+%d more)' % (len(notes) - 4) if len(notes) > 4 else '')))
    for rel in only_a[:show]:
        print('  ONLY BEFORE  %s' % rel)
    for rel in only_b[:show]:
        print('  ONLY AFTER   %s' % rel)
    return 0 if not diffs and not only_a and not only_b else 1

if __name__ == '__main__':
    sys.exit(main())
