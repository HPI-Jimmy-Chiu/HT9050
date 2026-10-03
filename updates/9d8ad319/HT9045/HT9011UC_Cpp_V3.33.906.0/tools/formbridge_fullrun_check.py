# -*- coding: utf-8 -*-
# AI(W906-E031) 20261003 [W906] (St01)：新檔（todo E-031 第二階段）。ctest E031_FormBridgeFullRun 的本體。
"""formbridge_fullrun_check.py -- gen_formbridge.py 全量重產＝已 commit 的登錄表。

為什麼：FileRW/_registry.cpp 與 FileRW/_formbridge_sources.cmake 是 gen_formbridge.py 全量（不帶 --only）的產生檔，裡面另有兩個手寫 bridge
（TfTeach、Tfiosetview）的列。20261002 這兩列是手改產生檔加的，E-031 第一階段量到全量一跑就被刪——build 不會紅、wb_serve 靜默少兩個 bridge。
現在由 tools/formbridge/_hand_kept.py（keep-list）供列；這支釘住「全量重產的兩檔＝已 commit 的兩檔」，誰把 keep-list 或產生器的輸出弄掉一列就紅。

做法（不 build、不讀寫任何機台檔、秒級）：
  1. 全量跑 tools/gen_formbridge.py --out <TEMP 資料夾>（tempfile.mkdtemp；在移植樹裡面就拒跑），比 _registry.cpp、_formbridge_sources.cmake
     跟移植樹 FileRW/ 的已 commit 版（換行不算：工作樹是 core.autocrlf 的 CRLF，產生器寫 LF），不同就印 unified diff、FAIL。
  2. 反向檢查（RULINGS_20261003 第 15 條，內建）：把 gen_formbridge.py／golden_root.py／formbridge/*.py 複製到另一個 TEMP 資料夾，
     那份的 _hand_kept.py 換成 HAND_KEPT = []，同樣全量跑、同樣比——這一次**必須**不同，而且少的正是 keep-list 的列；
     比對沒抓到（toothless）就 FAIL。
  3. 前後各算一次移植樹 FileRW/ 每個檔的 SHA256，有任何一個變了＝測試寫進了移植樹 ⇒ FAIL。
  V912 golden：沒設 W906_V912_ROOT 時用這份 checkout 自己的 HT9011UC_Code_V3.33.912.0_20260908_Jimmy（在 git；兩檔的內容不看 golden，
  但產生器要讀得過）。有表單選了 golden '906' 時要有 0618 樹（W906_GOLDEN_ROOT／HT9045_GOLDEN_ROOT，tools/golden_root.py）。

用法（在 HT9011UC_Cpp_V3.33.906.0 底下；python＝Python314 絕對路徑）：
  python tools/formbridge_fullrun_check.py
"""
import difflib
import hashlib
import io
import os
import shutil
import subprocess
import sys
import tempfile

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
FILERW = os.path.join(ROOT, 'FileRW')
COMPARED = ('_registry.cpp', '_formbridge_sources.cmake')
V912_DIRNAME = 'HT9011UC_Code_V3.33.912.0_20260908_Jimmy'


def norm(path):
    return open(path, 'rb').read().replace(b'\r\n', b'\n').decode('utf-8')


def tree_hashes():
    out = {}
    for fn in sorted(os.listdir(FILERW)):
        p = os.path.join(FILERW, fn)
        if os.path.isfile(p):
            out[fn] = hashlib.sha256(open(p, 'rb').read()).hexdigest()
    return out


def inside_tree(path):
    a, b = os.path.normcase(os.path.abspath(path)), os.path.normcase(ROOT)
    return a == b or a.startswith(b + os.sep)


def run_gen(tools_dir, out_dir, env):
    if inside_tree(out_dir):
        raise SystemExit('FAIL: refusing to write into the source tree: %s' % out_dir)
    r = subprocess.run([sys.executable, os.path.join(tools_dir, 'gen_formbridge.py'), '--out', out_dir],
                       cwd=os.path.dirname(tools_dir), env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    log = r.stdout.decode('utf-8', errors='replace')
    if r.returncode != 0:
        print(log)
        raise SystemExit('FAIL: gen_formbridge.py (%s) exited %d' % (tools_dir, r.returncode))
    return log


def compare(out_dir):
    """{檔名: unified diff 的行}（相同的不列）。"""
    bad = {}
    for fn in COMPARED:
        got = os.path.join(out_dir, fn)
        if not os.path.isfile(got):
            bad[fn] = ['(the full run did not write %s)' % fn]
            continue
        a, b = norm(os.path.join(FILERW, fn)).split('\n'), norm(got).split('\n')
        if a != b:
            bad[fn] = list(difflib.unified_diff(a, b, 'committed FileRW/' + fn, 'full run ' + fn, lineterm=''))
    return bad


def main():
    env = dict(os.environ, PYTHONIOENCODING='utf-8')
    own_v912 = os.path.join(os.path.dirname(ROOT), V912_DIRNAME)
    if not env.get('W906_V912_ROOT') and os.path.isfile(os.path.join(own_v912, 'main.cpp')):
        env['W906_V912_ROOT'] = own_v912
    print('V912 golden: %s' % env.get('W906_V912_ROOT', '(golden_root default)'))
    before = tree_hashes()
    tmp = tempfile.mkdtemp(prefix='w906_e031_fbfull_')
    fails = []
    try:
        # 1. 全量重產＝已 commit 的兩檔
        out = os.path.join(tmp, 'out')
        print(run_gen(HERE, out, env).rstrip())
        bad = compare(out)
        for fn in COMPARED:
            if fn in bad:
                fails.append('full run != committed FileRW/%s' % fn)
                print('\n'.join(bad[fn]))
            else:
                print('ok   full run == committed FileRW/%s' % fn)

        # 2. 反向檢查：keep-list 拿掉的副本必須比對失敗、而且少的正是 keep-list 的列
        mt = os.path.join(tmp, 'mutant', 'tools')
        os.makedirs(os.path.join(mt, 'formbridge'))
        for fn in ('gen_formbridge.py', 'golden_root.py'):
            shutil.copyfile(os.path.join(HERE, fn), os.path.join(mt, fn))
        for fn in os.listdir(os.path.join(HERE, 'formbridge')):
            if fn.endswith('.py'):
                shutil.copyfile(os.path.join(HERE, 'formbridge', fn), os.path.join(mt, 'formbridge', fn))
        ns = {}
        exec(compile(open(os.path.join(HERE, 'formbridge', '_hand_kept.py'), encoding='utf-8').read(), '_hand_kept.py', 'exec'), ns)
        kept = ns['HAND_KEPT']
        open(os.path.join(mt, 'formbridge', '_hand_kept.py'), 'w', encoding='utf-8', newline='\n').write(
            '# formbridge_fullrun_check.py reverse check: keep-list dropped\nHAND_KEPT = []\nCOUNT_NOTE = \'\'\n')
        mout = os.path.join(tmp, 'mutant', 'out')
        run_gen(mt, mout, env)
        mbad = compare(mout)
        if not kept:
            print('note reverse check skipped: _hand_kept.py lists no hand-written bridge')
        else:
            removed = '\n'.join(l for d in mbad.values() for l in d if l.startswith('-') and not l.startswith('---'))
            missing = [k['class'] for k in kept if ('kBridge_%s' % k['class']) not in removed or k['cpp'] not in removed]
            if set(mbad) != set(COMPARED) or missing:
                fails.append('reverse check: dropping the keep-list was not caught (differs in %s; rows not flagged: %s)' % (
                    sorted(mbad), missing))
            else:
                print('ok   reverse check: keep-list dropped -> both files differ, %d hand-kept bridge(s) flagged (%s)' % (
                    len(kept), ', '.join(k['class'] for k in kept)))
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    # 3. 移植樹沒被寫到
    after = tree_hashes()
    touched = sorted(fn for fn in set(before) | set(after) if before.get(fn) != after.get(fn))
    if touched:
        fails.append('the source tree FileRW/ changed during the test: %s' % touched)

    if fails:
        for f in fails:
            print('FAIL ' + f)
        return 1
    print('PASS E031_FormBridgeFullRun')
    return 0


if __name__ == '__main__':
    sys.exit(main())
