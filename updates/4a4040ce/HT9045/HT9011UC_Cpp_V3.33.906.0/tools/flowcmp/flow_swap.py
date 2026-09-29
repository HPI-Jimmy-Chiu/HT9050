"""RULINGS_20260927 第 6 條（第 30 題 A，真實路徑）: run the flow comparison with the REFERENCE machine's settings in the REAL
D:\\HT9045 locations.  Swap in: system, config (whole dirs, originals renamed aside), setup.inf (original copied aside),
IniData\\Data\\FT005054 (added; the current IniData stays).  Run flow_run.py in swapped mode.  Swap back and prove with the
'opmode' sysguard manifest that D:\\HT9045 is byte-identical to before.  A marker file names the holds so flow_unswap.py can
recover after a crash.
usage: python flow_swap.py <tag> <seconds> <checkpoints> [lot op]
env FLOW_PREP_INITIALSTART=1 (v2 20260927, report staterecord-flow-gap.md B1 option B): after the swap-in and BEFORE the run,
  set byte 0 (MachRec.bInitialStart) of the SWAPPED-IN system\\machinerecord.dat from 1 to 0, so the port's boot takes golden
  cinitial.cpp:8059-8064 instead of the "Need Load last machine record?" prompt (:8141-8173) -- see prep_initial_start().
  The real machinerecord.dat is never touched (it sits in system.__flowhold_<tag>); the patched copy is deleted by the
  swap-back like the rest of the swapped-in system dir, and the opmode manifest check proves it.
  All other env (FLOW_SCRIPT, FLOW_ANSWERS, FLOW_START_MODE, FLOW_EXE, FLOW_OUT) passes through to flow_run.py."""
import hashlib, json, os, shutil, subprocess, sys, time
_TREE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))   # <repo>/HT9011UC_Cpp_V3.33.906.0
_REPO = os.path.dirname(_TREE)
_OUT = os.environ.get('FLOW_OUT') or os.path.join(os.environ.get('TEMP', _REPO), 'ht9045_flowcmp')   # outputs never go into the tree
BS = chr(92)
ROOT = 'D:' + BS + 'HT9045'
REF = os.environ.get('FLOW_SWAP_SRC') or os.path.join(ROOT, 'Staterecord', '2025-12-11 17_47_57', 'HT9045')   # AI(W906-FLOW-T1) 20260929: FLOW_SWAP_SRC=<record>\HT9045 (RULINGS_20260929 #8: the golden 906.2 SIM record 2026-09-29 10_58_52)
# AI(W906-FLOW-T1) 20260929: FLOW_SWAP_RECIPE_REPLACE=1 -- the laptop already has IniData\Data\<RECIPE> (the aligned copy) but the
#   record's copy differs (golden saved 8 of its files during that run); hold the laptop's folder at the D:\HT9045 top level (not
#   inside IniData\Data, where golden/wb_serve would list it as another recipe) and copy the record's in; unswap() puts it back.
RECIPE_REPLACE = os.environ.get('FLOW_SWAP_RECIPE_REPLACE') == '1'
RECIPE = 'FT005054'
SNAP = os.path.join(ROOT, 'backup', 'gate_sysguard', os.environ.get('FLOW_SNAP', 'ht9050m_0928'))   # AI(W906-FLOW-2) 20260928: baseline = HT9050 machine config (RULINGS_20260928 #4)
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = _OUT
MARK = os.path.join(ROOT, 'backup', 'flowswap_ACTIVE.json')


def md5(p):
    with open(p, 'rb') as f:
        return hashlib.md5(f.read()).hexdigest()


def manifest_diff():
    man = json.load(open(os.path.join(SNAP, '_manifest.json'), encoding='utf-8'))
    cur = {}
    for d in ('system', 'config', 'IniData'):
        for dp, dn, fn in os.walk(os.path.join(ROOT, d)):
            for f in fn:
                p = os.path.join(dp, f)
                cur[os.path.relpath(p, ROOT).replace(BS, '/')] = md5(p)
    return [r for r in set(man) | set(cur) if man.get(r) != cur.get(r)]


def busy():
    o = subprocess.run(['tasklist'], stdout=subprocess.PIPE).stdout.decode('cp950', 'replace').lower()
    return [x for x in ('ctest.exe', 'cc1plus.exe', 'ninja.exe', 'wb_serve.exe') if x in o]


def unswap(m, say):
    for d in ('system', 'config'):
        hold = m['holds'][d]
        cur = os.path.join(ROOT, d)
        if os.path.isdir(hold):
            if os.path.isdir(cur):
                shutil.rmtree(cur)
            os.rename(hold, cur)
            say('  restored dir', d)
    hs = m['holds']['setup.inf']
    if os.path.isfile(hs):
        shutil.copy2(hs, os.path.join(ROOT, 'setup.inf'))
        os.remove(hs)
        say('  restored setup.inf')
    rd = os.path.join(ROOT, 'IniData', 'Data', RECIPE)
    if m.get('recipe_added') and os.path.isdir(rd):
        shutil.rmtree(rd)
        say('  removed added recipe', RECIPE)
    hr = m['holds'].get('recipe')                     # AI(W906-FLOW-T1) 20260929: FLOW_SWAP_RECIPE_REPLACE
    if hr and os.path.isdir(hr):
        if os.path.isdir(rd):
            shutil.rmtree(rd)
        os.rename(hr, rd)
        say('  restored recipe', RECIPE, 'from its hold')


MACHREC = 'machinerecord.dat'   # the bSpare=false file: golden cinitial.cpp:8033-8036, port cinitial.cpp:9667-9670


def _hex(b):
    return ' '.join('%02x' % c for c in b)


def prep_initial_start(path, ref_path, say):
    """FLOW_PREP_INITIALSTART=1: MachRec.bInitialStart := false in the swapped-in copy.
    Why byte 0 (checked 20260927 in both trees):
      * struct MachineRecord's FIRST member is `bool bInitialStart;` (golden cinitial.cpp:7632-7634, port cinitial.cpp:9377-9379),
        so it sits at offset 0 whatever the padding after it; bool is one byte (0 = false, 1 = true).
      * LoadMachineRecord reads the file raw from offset 0 over the whole struct: ReadData(<file>, (char*)&MachRec.bInitialStart,
        sizeof(struct MachineRecord)) (golden cinitial.cpp:8035-8036, port cinitial.cpp:9669-9670); ReadData is CreateFile +
        ReadFile into that pointer, no header (golden cprod.cpp:1340-1358, port cprod.cpp:1438-1456).
      * `if(MachRec.bInitialStart==false && bSpare==false) { SaveMachineRecord(); bFTPDownloadSetupFile=false; return; }`
        (golden cinitial.cpp:8059-8064, port cinitial.cpp:9711-9736) -- the boot then skips the YES/NO prompt of :8141-8173.
    Differences from the operator's NO/YES answer (report B1): the NO path also runs the SPIL bNoitceFixTray scan (:8154-8164)
    and the VTEST OEE clear (:8166-8170); this path does neither.
    Refuses unless `path` is byte-identical to the reference file (= it is the fresh swapped-in copy) and byte 0 is 0 or 1."""
    a, r = open(path, 'rb').read(), open(ref_path, 'rb').read()
    if os.path.samefile(path, ref_path):
        raise RuntimeError('prep: %s IS the reference file -- refused' % path)
    if a != r:
        raise RuntimeError('prep: %s is not a fresh copy of %s -- refused' % (path, ref_path))
    if a[0] not in (0, 1):
        raise RuntimeError('prep: byte 0 of %s is 0x%02x, not a bool -- layout unexpected, refused' % (path, a[0]))
    if a[0] == 1:
        with open(path, 'r+b') as f:
            f.seek(0)
            f.write(b'\x00')
    b = open(path, 'rb').read()
    ok = b[0] == 0 and b[1:] == a[1:] and len(b) == len(a)
    info = {'file': path, 'size': len(b), 'before16': _hex(a[:16]), 'after16': _hex(b[:16]), 'changed': a[0] != b[0], 'ok': ok}
    say('prep FLOW_PREP_INITIALSTART: %s byte0 %02x -> %02x (%s)' % (path, a[0], b[0], 'set' if info['changed'] else 'already 0'))
    say('  first 16 bytes before: %s' % info['before16'])
    say('  first 16 bytes after : %s' % info['after16'])
    if not ok:
        raise RuntimeError('prep: verification failed (only byte 0 may change): %r' % info)
    return info


def main():
    tag = sys.argv[1]
    b = busy()
    assert not b, 'refused: running now: %r' % b
    assert not os.path.exists(MARK), 'a previous swap was not undone -- run flow_unswap.py first'
    d0 = manifest_diff()
    assert not d0, 'D:\\HT9045 is not at the opmode baseline: %r' % d0[:5]
    assert os.path.isdir(os.path.join(REF, 'system')) and os.path.isdir(os.path.join(REF, 'config'))
    prep = os.environ.get('FLOW_PREP_INITIALSTART') == '1'
    if prep:                                          # v2: refuse before anything is swapped
        assert not os.environ.get('W906_MACHINERECORD_DIR'), \
            'FLOW_PREP_INITIALSTART: W906_MACHINERECORD_DIR is set, so wb_serve would read machinerecord.dat from there ' \
            '(port cinitial.cpp W906_MachineRecordRedirect), not from the swapped-in system -- unset it first'
        rb0 = open(os.path.join(REF, 'system', MACHREC), 'rb').read(1)
        assert rb0 in (b'\x00', b'\x01'), 'FLOW_PREP_INITIALSTART: reference %s byte 0 is %r, not a bool' % (MACHREC, rb0)
    log = []

    def say(*a):
        s = ' '.join(str(x) for x in a)
        print(s); log.append(s)

    m = {'tag': tag, 'holds': {'system': os.path.join(ROOT, 'system.__flowhold_' + tag),
                               'config': os.path.join(ROOT, 'config.__flowhold_' + tag),
                               'setup.inf': os.path.join(ROOT, 'setup.inf.__flowhold_' + tag)},
         'recipe_added': not os.path.isdir(os.path.join(ROOT, 'IniData', 'Data', RECIPE)), 't': time.strftime('%Y-%m-%d %H:%M:%S')}
    if RECIPE_REPLACE and not m['recipe_added']:     # AI(W906-FLOW-T1) 20260929
        m['holds']['recipe'] = os.path.join(ROOT, 'IniData_' + RECIPE + '.__flowhold_' + tag)
    for h in m['holds'].values():
        assert not os.path.exists(h), h
    json.dump(m, open(MARK, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    rc = None
    try:
        # ---- swap in (real paths)
        shutil.copy2(os.path.join(ROOT, 'setup.inf'), m['holds']['setup.inf'])
        for d in ('system', 'config'):
            os.rename(os.path.join(ROOT, d), m['holds'][d])
            shutil.copytree(os.path.join(REF, d), os.path.join(ROOT, d))
        shutil.copy2(os.path.join(REF, 'setup.inf'), os.path.join(ROOT, 'setup.inf'))
        if m['recipe_added']:
            shutil.copytree(os.path.join(REF, 'IniData', 'Data', RECIPE), os.path.join(ROOT, 'IniData', 'Data', RECIPE))
        elif m['holds'].get('recipe'):                # AI(W906-FLOW-T1) 20260929: replace the laptop's copy with the record's
            os.rename(os.path.join(ROOT, 'IniData', 'Data', RECIPE), m['holds']['recipe'])
            shutil.copytree(os.path.join(REF, 'IniData', 'Data', RECIPE), os.path.join(ROOT, 'IniData', 'Data', RECIPE))
            say('replaced recipe %s with the record copy (laptop copy held at %s)' % (RECIPE, m['holds']['recipe']))
        say('swapped in: system, config, setup.inf (%s), IniData\\Data\\%s' % (open(os.path.join(ROOT, 'setup.inf'), 'rb').read()[:40], RECIPE))
        if prep:                                      # v2: start-of-run data prep, swapped-in copy only (the real file is in the hold)
            assert os.path.isdir(m['holds']['system']), 'the real system dir is not in its hold -- prep refused'
            m['prep'] = prep_initial_start(os.path.join(ROOT, 'system', MACHREC), os.path.join(REF, 'system', MACHREC), say)
            json.dump(m, open(MARK, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
        # ---- run
        env = dict(os.environ, FLOW_SWAPPED='1', FLOW_SWAP_RECIPE=RECIPE, PYTHONIOENCODING='utf-8:replace')
        args = [sys.executable, os.path.join(HERE, 'flow_run.py')] + sys.argv[1:]
        rc = subprocess.run(args, env=env).returncode
        say('flow_run rc', rc)
    finally:
        if m.get('prep'):                             # what the run left in the swapped-in copy (SaveMachineRecord rewrites it)
            try:
                say('prep: swapped-in %s after the run: size %d, first 16 bytes %s (discarded by the swap-back)' % (
                    MACHREC, os.path.getsize(m['prep']['file']), _hex(open(m['prep']['file'], 'rb').read(16))))
            except OSError as e:
                say('prep: could not read the swapped-in %s after the run: %s' % (MACHREC, e))
        unswap(m, say)
        d1 = manifest_diff()
        say('re-check vs opmode after swap back:', 'CLEAN' if not d1 else 'DIFF %r' % d1[:8])
        if not d1:
            os.remove(MARK)
        os.makedirs(os.path.join(OUT, 'run_' + tag), exist_ok=True)   # v2: flow_run may have refused before creating it
        open(os.path.join(OUT, 'run_' + tag, 'swap.txt'), 'w', encoding='utf-8').write('\n'.join(log) + '\n')
    return rc


if __name__ == '__main__':
    sys.exit(main() or 0)
