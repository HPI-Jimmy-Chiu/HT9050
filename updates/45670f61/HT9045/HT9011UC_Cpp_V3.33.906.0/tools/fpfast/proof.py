# AI(W906-FPFAST) 20261003 -- proof that -fexcess-precision=fast does not change
# what the oracle (MinGW.org g++ 6.3.0) emits.  For every unique compile command
# of a compile_commands.json (the build's own command = WITH the flag), compile it
# twice into a scratch dir -- WITH (as is) and WITHOUT (flag token removed) -- and
# compare: return code, stderr, object bytes, `objdump -d -r --no-show-raw-insn`
# section by section (and function by function when a section differs), and
# `objdump -s` (all section contents: .text .rdata .data ...) section by section.
#
# Runs itself at IDLE priority on the affinity mask given (default 0x3F); every
# child (g++ -> cc1plus / as) inherits both.  No backslash literal in this file:
# chr(92) where one is needed.
import argparse, ctypes, hashlib, json, os, re, subprocess, sys, threading, time
from concurrent.futures import ThreadPoolExecutor

IDLE_PRIORITY_CLASS = 0x00000040
CREATE_NO_WINDOW = 0x08000000
FLAG = '-fexcess-precision=fast'
NL = chr(10)
CR = chr(13)
BS = chr(92)
FUNC_RE = re.compile('^([0-9a-f]+) <(.*)>:$')


def set_low(mask):
    k = ctypes.windll.kernel32
    k.GetCurrentProcess.restype = ctypes.c_void_p
    k.SetPriorityClass.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    k.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    h = k.GetCurrentProcess()
    return bool(k.SetPriorityClass(h, IDLE_PRIORITY_CLASS)), bool(k.SetProcessAffinityMask(h, mask))


def sha(path):
    hh = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            hh.update(chunk)
    return hh.hexdigest()


def split_sections(text, marker):
    secs = {}
    order = []
    cur = None
    buf = []
    for line in text.split(NL):
        line = line.rstrip(CR)
        if line.startswith(marker):
            if cur is not None:
                secs[cur] = buf
            cur = line[len(marker):].rstrip(':').strip()
            order.append(cur)
            buf = []
        elif cur is not None:
            buf.append(line)
    if cur is not None:
        secs[cur] = buf
    return order, secs


def split_functions(lines):
    funcs = {}
    cur = '<prologue>'
    buf = []
    for line in lines:
        m = FUNC_RE.match(line)
        if m:
            funcs.setdefault(cur, []).append(buf)
            cur = m.group(2)
            buf = [line]
        else:
            buf.append(line)
    funcs.setdefault(cur, []).append(buf)
    return funcs


def run(cmd, cwd, env):
    t0 = time.time()
    p = subprocess.run(cmd, cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                       creationflags=IDLE_PRIORITY_CLASS | CREATE_NO_WINDOW)
    return p.returncode, p.stdout.decode('utf-8', 'replace'), p.stderr.decode('utf-8', 'replace'), time.time() - t0


def objdump(objdump_exe, args, path, env):
    p = subprocess.run([objdump_exe] + args + [path], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                       env=env, creationflags=IDLE_PRIORITY_CLASS | CREATE_NO_WINDOW)
    return p.returncode, p.stdout.decode('utf-8', 'replace'), p.stderr.decode('utf-8', 'replace')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--db', required=True)
    ap.add_argument('--tag', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--jobs', type=int, default=6)
    ap.add_argument('--mask', type=lambda s: int(s, 0), default=0x3F)
    ap.add_argument('--limit', type=int, default=0)
    ap.add_argument('--only', default='')
    ap.add_argument('--mingw', default='C:' + BS + 'MinGW' + BS + 'bin')
    ap.add_argument('--keep', action='store_true', help='keep identical objects too')
    ap.add_argument('--b-extra', default='', help='NEGATIVE CONTROL ONLY: extra flag(s) appended to the WITH command')
    ap.add_argument('--extra-both', default='', help='extra flag(s) put right after the compiler on BOTH commands (e.g. -O3 -DNDEBUG)')
    a = ap.parse_args()

    pri, aff = set_low(a.mask)
    env = dict(os.environ)
    env['PATH'] = a.mingw + ';' + env.get('PATH', '')
    objdump_exe = os.path.join(a.mingw, 'objdump.exe')

    db = json.load(open(a.db, encoding='utf-8'))
    skipped = []
    groups = {}
    for e in db:
        cmd = e['command']
        exe = cmd.split(' ')[0].lower()
        if not (exe.endswith('g++.exe') or exe.endswith('gcc.exe')):
            skipped.append({'file': e['file'], 'exe': exe, 'has_flag': FLAG in cmd})
            continue
        if cmd.count(' -o ') != 1:
            raise SystemExit('cannot locate -o in: ' + cmd[:300])
        _i = cmd.index(' -o ') + 4
        o = ' -o ' + cmd[_i:cmd.index(' ', _i)] + ' '
        e['_o'] = o
        if cmd.split(' ').count(FLAG) != 1:
            raise SystemExit('flag not exactly once in: ' + cmd[:300])
        key = e['directory'].lower() + '|' + cmd.replace(o, ' -o @OUT@ ')
        groups.setdefault(key, []).append(e)
    keys = sorted(groups)
    if a.only:
        subs = [x.strip().lower() for x in a.only.split(',') if x.strip()]
        keys = [k for k in keys if any(x in k.lower() for x in subs)]
    if a.limit:
        keys = keys[:a.limit]

    os.makedirs(os.path.join(a.out, 'A'), exist_ok=True)
    os.makedirs(os.path.join(a.out, 'B'), exist_ok=True)
    jl = os.path.join(a.out, 'results.jsonl')
    done = set()
    if os.path.exists(jl):
        good = []
        for line in open(jl, encoding='utf-8'):
            line = line.strip()
            if line:
                try:
                    r0 = json.loads(line)
                except ValueError:
                    continue            # a line cut by a kill: that key is simply redone
                done.add(r0['key'])
                good.append(line)
        with open(jl, 'w', encoding='utf-8') as f:
            f.write(''.join(x + NL for x in good))
    lock = threading.Lock()
    stats = {'n': 0, 'byte_identical': 0, 'all_sections_identical': 0, 'different': 0, 'failed': 0}
    t_start = time.time()
    todo = [k for k in keys if k not in done]
    print('[proof %s] priority_idle=%s affinity_set=%s mask=0x%X entries=%d unique_commands=%d skipped_non_gcc=%d todo=%d already_done=%d'
          % (a.tag, pri, aff, a.mask, sum(len(groups[k]) for k in keys), len(keys), len(skipped), len(todo), len(keys) - len(todo)), flush=True)
    for s in skipped:
        print('[proof %s] skipped (not a GNU compile): %s %s flag_present=%s' % (a.tag, s['exe'], s['file'], s['has_flag']), flush=True)

    def one(k):
        i = keys.index(k)
        es = groups[k]
        e0 = es[0]
        cwd = e0['directory']
        objA = os.path.join(a.out, 'A', '%04d.o' % i)
        objB = os.path.join(a.out, 'B', '%04d.o' % i)
        o = e0['_o']
        cmdB = e0['command'].replace(o, ' -o "' + objB + '" ')
        cmdA = e0['command'].replace(o, ' -o "' + objA + '" ').replace(' ' + FLAG, '', 1)
        if a.b_extra:
            cmdB = cmdB.replace(' ' + FLAG, ' ' + FLAG + ' ' + a.b_extra, 1)
        if a.extra_both:
            sp = cmdA.index(' ')
            cmdA = cmdA[:sp] + ' ' + a.extra_both + cmdA[sp:]
            sp = cmdB.index(' ')
            cmdB = cmdB[:sp] + ' ' + a.extra_both + cmdB[sp:]
        assert FLAG not in cmdA.split(' ') and cmdB.split(' ').count(FLAG) == 1
        for f in (objA, objB):
            if os.path.exists(f):
                os.remove(f)
        rcA, outA, errA, tA = run(cmdA, cwd, env)
        rcB, outB, errB, tB = run(cmdB, cwd, env)
        r = {'key': k, 'i': i, 'file': e0['file'], 'entries': len(es),
             'outputs': [x['output'] for x in es], 'rcA': rcA, 'rcB': rcB,
             'stderr_equal': errA == errB, 'stdout_equal': outA == outB,
             'stderrB_mentions_excess': ('excess' in errB.lower()) and ('excess' not in errA.lower()),
             'tA': round(tA, 2), 'tB': round(tB, 2), 'cmdA': cmdA, 'cmdB': cmdB}
        if rcA != 0 or rcB != 0 or not os.path.exists(objA) or not os.path.exists(objB):
            r['status'] = 'FAILED'
            r['errA'] = errA[-2000:]
            r['errB'] = errB[-2000:]
        else:
            hA, hB = sha(objA), sha(objB)
            r['shaA'], r['shaB'] = hA, hB
            r['sizeA'], r['sizeB'] = os.path.getsize(objA), os.path.getsize(objB)
            r['bytes_identical'] = (hA == hB)
            # objdump -d -r --no-show-raw-insn, section by section
            dA = objdump(objdump_exe, ['-d', '-r', '--no-show-raw-insn'], objA, env)
            dB = objdump(objdump_exe, ['-d', '-r', '--no-show-raw-insn'], objB, env)
            ordA, secA = split_sections(dA[1], 'Disassembly of section ')
            ordB, secB = split_sections(dB[1], 'Disassembly of section ')
            dis_diff = []
            func_diff = []
            for s in sorted(set(secA) | set(secB)):
                if secA.get(s) != secB.get(s):
                    dis_diff.append(s)
                    fA = split_functions(secA.get(s, []))
                    fB = split_functions(secB.get(s, []))
                    for fn in sorted(set(fA) | set(fB)):
                        if fA.get(fn) != fB.get(fn):
                            func_diff.append(s + ':' + fn)
            # objdump -s: every section's contents (.text .rdata .data .bss-less ...)
            cA = objdump(objdump_exe, ['-s'], objA, env)
            cB = objdump(objdump_exe, ['-s'], objB, env)
            cordA, csecA = split_sections(cA[1], 'Contents of section ')
            cordB, csecB = split_sections(cB[1], 'Contents of section ')
            con_diff = [s for s in sorted(set(csecA) | set(csecB)) if csecA.get(s) != csecB.get(s)]
            r['objdump_rc'] = [dA[0], dB[0], cA[0], cB[0]]
            r['dis_sections'] = sorted(secA)
            r['con_sections'] = sorted(csecA)
            r['dis_text_lines'] = sum(len(v) for v in secA.values())
            r['dis_diff'] = dis_diff
            r['func_diff'] = func_diff[:50]
            r['con_diff'] = con_diff
            r['section_order_equal'] = (ordA == ordB and cordA == cordB)
            same = (not dis_diff and not con_diff and ordA == ordB and cordA == cordB
                    and all(x == 0 for x in r['objdump_rc']) and len(secA) > 0)
            r['status'] = 'IDENTICAL' if same else 'DIFFERENT'
            if same and not a.keep:
                os.remove(objA)
                os.remove(objB)
        with lock:
            stats['n'] += 1
            if r['status'] == 'FAILED':
                stats['failed'] += 1
            elif r['status'] == 'DIFFERENT':
                stats['different'] += 1
            else:
                stats['all_sections_identical'] += 1
            if r.get('bytes_identical'):
                stats['byte_identical'] += 1
            with open(jl, 'a', encoding='utf-8') as f:
                f.write(json.dumps(r) + NL)
            if r['status'] != 'IDENTICAL':
                print('[proof %s] %s %s %s' % (a.tag, r['status'], r['file'], (r.get('dis_diff'), r.get('con_diff'), r.get('func_diff'), r.get('errB', '')[-300:])), flush=True)
            if stats['n'] % 25 == 0 or stats['n'] == len(todo):
                el = time.time() - t_start
                eta = el / stats['n'] * (len(todo) - stats['n'])
                print('[proof %s] %d/%d identical=%d byte_identical=%d different=%d failed=%d elapsed=%.0fs eta=%.0fs'
                      % (a.tag, stats['n'], len(todo), stats['all_sections_identical'], stats['byte_identical'],
                         stats['different'], stats['failed'], el, eta), flush=True)
        return r

    with ThreadPoolExecutor(max_workers=a.jobs) as ex:
        list(ex.map(one, todo))
    print('[proof %s] DONE %s' % (a.tag, json.dumps(stats)), flush=True)


if __name__ == '__main__':
    main()
