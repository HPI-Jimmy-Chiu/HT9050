#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""NB2 輔助工具 #26（20260926）：替身「接到哪個物件」對 golden 逐點核對。

移植樹常用一個替身（巨集或 static 函式）代替 golden 的 `<物件>.<成員>`，例如
`W64bT2_BNeedCheckGet(i,j)` 代替 golden 的 `BTestSuck.bNeedCheck[i][j]`。替身一旦從 no-op 改成真的讀寫某個物件，
每個呼叫點就必須在 golden 也是那個物件；golden 若是別的物件（例如 FTestSuck），就是讀寫到另一邊。

做法：找出移植檔裡每個替身呼叫點（跳過定義行），讀它行尾註解標的 `golden :N`，看 golden 同名檔第 N 行
`<物件>.<成員>` 的物件是不是預期的那一個。另外列出 golden 這個成員的所有物件分布，方便看漏了哪裡。

只讀。用法：
  python stub_receiver_verify.py [--rev origin/main] <檔> <替身正規式> <成員> <預期物件>
例：
  python stub_receiver_verify.py --rev origin/main aTester_Rear.cpp "W64bT2_BNeedCheck(Get|Set)\\s*\\(" bNeedCheck BTestSuck
  python stub_receiver_verify.py aTester_Front.cpp "W64B_NEEDCHECK_(GET|SET)\\s*\\(" bNeedCheck FTestSuck
結束碼：0＝每個呼叫點都對；1＝有呼叫點在 golden 是別的物件（或沒有 golden 行號可核對）。
"""
import argparse, os, re, subprocess, sys

GOLDEN = os.environ.get('HT9045_GOLDEN_ROOT') or r'D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618'
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def main():
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    ap = argparse.ArgumentParser()
    ap.add_argument('--rev')
    ap.add_argument('file')
    ap.add_argument('stub')
    ap.add_argument('member')
    ap.add_argument('expect')
    a = ap.parse_args()
    if a.rev:
        b = subprocess.run(['git', '-C', ROOT, 'show', '%s:HT9011UC_Cpp_V3.33.906.0/%s' % (a.rev, a.file)], capture_output=True, check=True).stdout
    else:
        b = open(os.path.join(ROOT, a.file), 'rb').read()
    P = b.decode('utf-8', 'strict').replace('\r\n', '\n').split('\n')
    G = open(os.path.join(GOLDEN, os.path.basename(a.file)), 'rb').read().decode('cp950', 'replace').replace('\r\n', '\n').split('\n')
    mem = re.compile(r'(\w+)\s*\.\s*' + re.escape(a.member) + r'\b')
    dist = {}
    for i, l in enumerate(G):
        m = mem.search(l.split('//')[0])
        if m:
            dist.setdefault(m.group(1), []).append(i + 1)
    print('golden %s 的 .%s 物件分布：' % (os.path.basename(a.file), a.member))
    for k, v in sorted(dist.items(), key=lambda kv: -len(kv[1])):
        print('   %-12s %3d  %s' % (k, len(v), v[:12]))
    stub = re.compile(a.stub)
    bad = n = 0
    for i, l in enumerate(P):
        code = l.split('//')[0]
        if not stub.search(code) or re.match(r'\s*(static\b|#\s*define\b)', l):
            continue
        n += 1
        c = re.search(r'golden\s*:?\s*(\d+)', l)
        if not c:
            bad += 1
            print('   ？ :%d 沒有 golden 行號可核對：%s' % (i + 1, l.strip()[:110]))
            continue
        gn = int(c.group(1))
        m = mem.search(G[gn - 1].split('//')[0]) if gn <= len(G) else None
        rec = m.group(1) if m else None
        if rec != a.expect:
            bad += 1
            print('   ✗ :%d 標 golden :%d，golden 是 %s（替身接的是 %s）：%s' % (i + 1, gn, rec, a.expect, G[gn - 1].strip()[:90]))
    print('\n替身呼叫點 %d 個；物件和 golden 不同或無法核對 %d 個' % (n, bad))
    return 0 if bad == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
