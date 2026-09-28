"""RULINGS_20260927 第 6 條（第 30 題 A，真實路徑）: run the flow comparison with the REFERENCE machine's settings in the REAL
D:\\HT9045 locations.  Swap in: system, config (whole dirs, originals renamed aside), setup.inf (original copied aside),
IniData\\Data\\FT005054 (added; the current IniData stays).  Run flow_run.py in swapped mode.  Swap back and prove with the
'opmode' sysguard manifest that D:\\HT9045 is byte-identical to before.  A marker file names the holds so flow_unswap.py can
recover after a crash.
usage: python flow_swap.py <tag> <seconds> <checkpoints> [lot op]"""
import hashlib, json, os, shutil, subprocess, sys, time
_TREE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))   # <repo>/HT9011UC_Cpp_V3.33.906.0
_REPO = os.path.dirname(_TREE)
_OUT = os.environ.get('FLOW_OUT') or os.path.join(os.environ.get('TEMP', _REPO), 'ht9045_flowcmp')   # outputs never go into the tree
BS = chr(92)
ROOT = 'D:' + BS + 'HT9045'
REF = os.path.join(ROOT, 'Staterecord', '2025-12-11 17_47_57', 'HT9045')
RECIPE = 'FT005054'
SNAP = os.path.join(ROOT, 'backup', 'gate_sysguard', 'opmode')
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


def main():
    tag = sys.argv[1]
    b = busy()
    assert not b, 'refused: running now: %r' % b
    assert not os.path.exists(MARK), 'a previous swap was not undone -- run flow_unswap.py first'
    d0 = manifest_diff()
    assert not d0, 'D:\\HT9045 is not at the opmode baseline: %r' % d0[:5]
    assert os.path.isdir(os.path.join(REF, 'system')) and os.path.isdir(os.path.join(REF, 'config'))
    log = []

    def say(*a):
        s = ' '.join(str(x) for x in a)
        print(s); log.append(s)

    m = {'tag': tag, 'holds': {'system': os.path.join(ROOT, 'system.__flowhold_' + tag),
                               'config': os.path.join(ROOT, 'config.__flowhold_' + tag),
                               'setup.inf': os.path.join(ROOT, 'setup.inf.__flowhold_' + tag)},
         'recipe_added': not os.path.isdir(os.path.join(ROOT, 'IniData', 'Data', RECIPE)), 't': time.strftime('%Y-%m-%d %H:%M:%S')}
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
        say('swapped in: system, config, setup.inf (%s), IniData\\Data\\%s' % (open(os.path.join(ROOT, 'setup.inf'), 'rb').read()[:40], RECIPE))
        # ---- run
        env = dict(os.environ, FLOW_SWAPPED='1', FLOW_SWAP_RECIPE=RECIPE, PYTHONIOENCODING='utf-8:replace')
        args = [sys.executable, os.path.join(HERE, 'flow_run.py')] + sys.argv[1:]
        rc = subprocess.run(args, env=env).returncode
        say('flow_run rc', rc)
    finally:
        unswap(m, say)
        d1 = manifest_diff()
        say('re-check vs opmode after swap back:', 'CLEAN' if not d1 else 'DIFF %r' % d1[:8])
        if not d1:
            os.remove(MARK)
        open(os.path.join(OUT, 'run_' + tag, 'swap.txt'), 'w', encoding='utf-8').write('\n'.join(log) + '\n')
    return rc


if __name__ == '__main__':
    sys.exit(main() or 0)
