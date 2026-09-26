#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""NB2 輔助工具 #25（20260926）：解閘區段對 golden 逐行核對。

移植樹把 `#if 0 // TODO … golden <檔>:<行>` 解開時，慣例是把兩行閘換成註解：
    //AI(W906-…) YYYYMMDD: gate LIFTED -- … golden [<檔>]:<行>[-<行>]
    …（原本被閘住的程式碼，現在是活的）…
    //AI(W906-…) YYYYMMDD: (end of lifted gate)
這支工具找出所有這種區段，把區段內每一行活碼（去掉 // 註解、空白正規化）和 golden 906（Big5，cp950 讀）
同名檔從標記的行號起的「非空白、非註解」行逐一比對；另外在 golden ±3 行內找同一句，容許移植時的小幅位移。

只讀。golden 找不到同名檔時，改用標記裡寫的 <檔>。結束碼 0＝全部區段逐行相同。
用法：
  python lifted_gate_verify.py aTester_Front.cpp aTester_Rear.cpp csystem.cpp
  python lifted_gate_verify.py --rev <commit> aTester_Front.cpp …   （讀某個 commit 的版本，不看工作樹）
  環境變數 HT9045_GOLDEN_ROOT 可改 golden 位置。
"""
import os, re, subprocess, sys

GOLDEN = os.environ.get('HT9045_GOLDEN_ROOT') or r'D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618'
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))          # HT9011UC_Cpp_V3.33.906.0
START = re.compile(r'gate LIFTED.*?golden\s+(?:([\w/\\]+\.(?:cpp|h)):)?:?(\d+)(?:-(\d+))?')
END = re.compile(r'\(end of lifted gate\)')


def norm(s):
    s = re.sub(r'//.*', '', s)
    s = re.sub(r'/\*.*?\*/', '', s)
    return re.sub(r'\s+', '', s)


def read_port(rel, rev):
    if rev:
        r = subprocess.run(['git', '-C', ROOT, 'show', '%s:HT9011UC_Cpp_V3.33.906.0/%s' % (rev, rel.replace('\\', '/'))],
                           capture_output=True)
        if r.returncode != 0:
            raise OSError(r.stderr.decode('utf-8', 'replace'))
        b = r.stdout
    else:
        b = open(os.path.join(ROOT, rel), 'rb').read()
    return b.decode('utf-8', 'strict').replace('\r\n', '\n').split('\n')


_gcache = {}
def read_golden(fn):
    if fn not in _gcache:
        p = os.path.join(GOLDEN, fn)
        _gcache[fn] = open(p, 'rb').read().decode('cp950', 'replace').replace('\r\n', '\n').split('\n') if os.path.exists(p) else None
    return _gcache[fn]


def main():
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    args = sys.argv[1:]
    rev = None
    if args[:1] == ['--rev']:
        rev, args = args[1], args[2:]
    total = bad = shifted = 0
    for rel in args:
        L = read_port(rel, rev)
        i = 0
        while i < len(L):
            m = START.search(L[i])
            if not m:
                i += 1
                continue
            gfile = os.path.basename(m.group(1)) if m.group(1) else os.path.basename(rel)
            gstart = int(m.group(2))
            j = i + 1
            body = []
            while j < len(L) and not END.search(L[j]) and not START.search(L[j]):
                if norm(L[j]):
                    body.append((j + 1, norm(L[j]), L[j].strip()))
                j += 1
            gl = read_golden(gfile)
            total += 1
            if gl is None:
                bad += 1
                print('%s:%d  golden 沒有 %s' % (rel, i + 1, gfile))
                i = j
                continue
            # golden 從 gstart 起取同樣數目的非空白、非註解行
            g = []
            k = gstart - 1
            while k < len(gl) and len(g) < len(body):
                if norm(gl[k]):
                    g.append((k + 1, norm(gl[k])))
                k += 1
            ok = [b[1] == (g[n][1] if n < len(g) else None) for n, b in enumerate(body)]
            if all(ok) and body:
                i = j
                continue
            # 容許小幅位移：每一行在 golden gstart±3 行內找同一句
            near = {norm(gl[x]) for x in range(max(0, gstart - 4), min(len(gl), gstart + len(body) + 3))}
            miss = [b for b in body if b[1] not in near]
            if not body:
                bad += 1
                print('%s:%d  區段是空的（標記 golden %s:%d）' % (rel, i + 1, gfile, gstart))
            elif miss:
                bad += 1
                print('%s:%d  ✗ 和 golden %s:%d 不同：' % (rel, i + 1, gfile, gstart))
                for b in miss:
                    print('      移植 :%d  %s' % (b[0], b[2][:140]))
                for x in range(gstart - 1, min(len(gl), gstart + len(body) + 1)):
                    print('      golden :%d  %s' % (x + 1, gl[x].strip()[:140]))
            else:
                shifted += 1
                print('%s:%d  △ 句子都在 golden %s:%d±3，但行序或位置有位移' % (rel, i + 1, gfile, gstart))
            i = j
    print('\n解閘區段 %d 個；逐行相同 %d；句子相同但位移 %d；不同或無法核對 %d' % (total, total - bad - shifted, shifted, bad))
    return 0 if bad == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
