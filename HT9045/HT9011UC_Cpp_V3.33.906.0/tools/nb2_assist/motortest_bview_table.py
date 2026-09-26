#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""NB2 輔助工具 #24（20260925）：馬達測試頁 164 支馬達的 bView 對照表。

用途
  golden `TfMotorTest::TfMotorTest`（uMotorTest.cpp）用 164 筆
  `MotorTestClass.push_back(new TMotorTestClass(MotNo[, bView]))` 決定哪幾支馬達出現在畫面上，
  網頁的馬達清單（機台端 MT-E1 的 W906_MotorTestVisibility）要照它。這支工具把 golden 抽成表，
  並做三個檢查：
    (1) 筆數 == TOTAL_MOTOR，而且 enum 值剛好涵蓋 0..TOTAL_MOTOR-1、沒有重複
    (2) push 的順序 j 是否等於馬達 enum 值
        —— golden DoLoopMove（uMotorTest.cpp:412）用 MotorTestClass[ActiveIndex]（ActiveIndex＝馬達編號）
           去索引這個「依 push 順序」的 vector；只要有一筆 j != 值，Loop Move 就讀到別支馬達的 pos1/pos2
    (3) 可選：對照移植樹 forms/fMotorTest.cpp（或任何含同樣 push_back 寫法的檔）的順序與運算式

只讀；不寫任何檔（--tsv 才輸出到指定路徑）。golden 是 Big5，用 cp950 strict 解碼（失敗就停，不猜）。

用法
  python motortest_bview_table.py                      # golden 表＋檢查
  python motortest_bview_table.py --port forms/fMotorTest.cpp
  python motortest_bview_table.py --port <機台版 fMotorTest.cpp> --port-label machine
  python motortest_bview_table.py --tsv out.tsv        # 輸出 TSV（j, 行, 名稱, 值, bView）
  環境變數 HT9045_GOLDEN_ROOT 可改 golden 位置。
結束碼：0＝全部檢查通過；1＝有差異或檢查失敗；2＝輸入讀不到。
"""
import argparse
import os
import re
import sys

GOLDEN = os.environ.get('HT9045_GOLDEN_ROOT') or r'D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618'
PUSH = re.compile(r'MotorTestClass\s*\.\s*push_back\s*\(\s*new\s+TMotorTestClass\s*\(')
CONST = re.compile(r'^\s*const\s+int\s+(M\w+)\s*=\s*(\d+)\s*;')


def read_text(path, big5):
    b = open(path, 'rb').read()
    t = b.decode('cp950', errors='strict') if big5 else b.decode('utf-8', errors='strict')
    return [l.rstrip('\r') for l in t.split('\n')]


def strip_comments(lines):
    """去掉 // 與 /* */ 註解（不處理字串裡的 //，這兩個檔的 push_back 行沒有字串）。"""
    out, in_block = [], False
    for l in lines:
        s, i = '', 0
        while i < len(l):
            if in_block:
                k = l.find('*/', i)
                if k < 0:
                    i = len(l)
                else:
                    in_block, i = False, k + 2
                continue
            if l.startswith('//', i):
                break
            if l.startswith('/*', i):
                in_block, i = True, i + 2
                continue
            s += l[i]
            i += 1
        out.append(s)
    return out


def args_of(s, start):
    """s[start] 緊接在 TMotorTestClass( 之後；回傳 (參數字串列, 結束位置)。"""
    depth, cur, parts, i = 1, '', [], start
    while i < len(s):
        c = s[i]
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                parts.append(cur.strip())
                return parts, i
        if c == ',' and depth == 1:
            parts.append(cur.strip())
            cur = ''
        else:
            cur += c
        i += 1
    return None, i


def norm(e):
    return re.sub(r'\s+', '', e)


def parse_pushes(lines, big5_label):
    """回傳 [(行號, 名稱, bView運算式, 前置處理器條件)]；bView 省略時為 'true'（uMotorTest.h:34 預設值）。
    `#if 0` 區段內的不算（移植樹常把 golden 原文放在 #if 0 裡）。"""
    code = strip_comments(lines)
    res, pp = [], []
    skip_to = 0
    for n, s in enumerate(code, 1):
        if n <= skip_to:
            continue
        t = s.strip()
        m = re.match(r'#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)', t)
        if m:
            kw, rest = m.group(1), m.group(2).strip()
            if kw in ('if', 'ifdef', 'ifndef'):
                pp.append(kw + ' ' + rest)
            elif kw in ('elif', 'else') and pp:
                pp[-1] = pp[-1] + ' | ' + kw + ' ' + rest
            elif kw == 'endif' and pp:
                pp.pop()
            continue
        if any(p.startswith('if 0') and '| else' not in p for p in pp):
            continue
        mm = PUSH.search(s)
        if mm:
            # 跨行的 push_back：把後面的行接上，直到括號配平（golden MLoaderY 那筆寫了 3 行）
            joined, last = s, n
            a, _ = args_of(joined, mm.end())
            while a is None and last < len(code) and last - n < 20:
                joined += ' ' + code[last]
                last += 1
                a, _ = args_of(joined, mm.end())
            if not a:
                print('WARN %s:%d 解析不了: %s' % (big5_label, n, s.strip()[:120]), file=sys.stderr)
                continue
            skip_to = last
            if PUSH.search(joined, mm.end()):
                print('WARN %s:%d 同一段有兩個 push_back，只取第一個' % (big5_label, n), file=sys.stderr)
            name = a[0]
            bview = a[1] if len(a) > 1 else 'true'
            res.append((n, name, bview, ' && '.join(pp)))
    return res


def ctor_span(lines):
    a = next((i for i, l in enumerate(lines) if re.search(r'TfMotorTest::TfMotorTest\s*\(', l)), None)
    if a is None:
        return None
    b = next((i for i in range(a, len(lines)) if re.search(r'for\s*\(\s*int\s+i\s*=\s*0\s*;\s*i\s*<\s*TOTAL_MOTOR', lines[i])), len(lines))
    return a, b


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', help='要對照的移植檔（UTF-8），預設不對照')
    ap.add_argument('--port-label', default='port')
    ap.add_argument('--port-big5', action='store_true', help='移植檔是 Big5（例如 V912）')
    ap.add_argument('--tsv', help='輸出 golden 表到這個 TSV')
    ap.add_argument('--quiet', action='store_true', help='不印 164 行的表')
    a = ap.parse_args()
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    sys.stderr.reconfigure(encoding='utf-8', errors='replace')

    try:
        gl = read_text(os.path.join(GOLDEN, 'uMotorTest.cpp'), True)
        cl = read_text(os.path.join(GOLDEN, 'cmydef.cpp'), True)
        dl = read_text(os.path.join(GOLDEN, 'cmydef.h'), True)
    except (OSError, UnicodeDecodeError) as e:
        print('golden 讀不到或解碼失敗：%s' % e)
        return 2

    total = next((int(m.group(1)) for l in dl for m in [re.match(r'\s*#define\s+TOTAL_MOTOR\s+(\d+)', l)] if m), None)
    vals = {}
    for l in strip_comments(cl):
        m = CONST.match(l)
        if m:
            vals[m.group(1)] = int(m.group(2))

    span = ctor_span(gl)
    if span is None:
        print('golden 找不到 TfMotorTest::TfMotorTest')
        return 2
    g = [(n + span[0], nm, bv, pp) for (n, nm, bv, pp) in parse_pushes(gl[span[0]:span[1]], 'golden')]

    bad = 0
    print('golden：%s\\uMotorTest.cpp  建構子 :%d-%d  push_back %d 筆；TOTAL_MOTOR=%s（cmydef.h）；cmydef.cpp 馬達常數 %d 個'
          % (GOLDEN, span[0] + 1, span[1], len(g), total, len(vals)))
    rows = []
    for j, (n, nm, bv, pp) in enumerate(g):
        v = vals.get(nm)
        rows.append((j, n, nm, v, bv, pp))
    if not a.quiet:
        print('j\t行\t名稱\t值\tj==值\tbView\t前置處理器')
        for j, n, nm, v, bv, pp in rows:
            print('%d\t%d\t%s\t%s\t%s\t%s\t%s' % (j, n, nm, v, 'Y' if v == j else 'N', bv, pp))

    # 檢查 (1)
    unk = [r for r in rows if r[3] is None]
    seen = {}
    for r in rows:
        if r[3] is not None:
            seen.setdefault(r[3], []).append(r)
    dup = {k: v for k, v in seen.items() if len(v) > 1}
    miss = [k for k in range(total or 0) if k not in seen]
    print('\n[1] 筆數 %d／TOTAL_MOTOR %s：%s' % (len(rows), total, '相符' if len(rows) == total else '不符'))
    print('    名稱查不到值 %d；值重複 %d；0..%d 缺 %d' % (len(unk), len(dup), (total or 0) - 1, len(miss)))
    for r in unk:
        print('    查不到：golden:%d %s' % (r[1], r[2]))
    for k, v in dup.items():
        print('    重複值 %d：%s' % (k, ', '.join('%s(:%d)' % (x[2], x[1]) for x in v)))
    if miss:
        print('    缺的值：%s' % miss)
    bad += (len(rows) != total) + len(unk) + len(dup) + len(miss)

    # 檢查 (2)
    off = [r for r in rows if r[3] is not None and r[3] != r[0]]
    print('\n[2] push 順序 j == enum 值：%d／%d 筆相同；不同 %d 筆' % (len(rows) - len(off), len(rows), len(off)))
    for r in off[:40]:
        print('    j=%d golden:%d %s=%d ⇒ MotorTestClass[%d] 其實是 %s' % (r[0], r[1], r[2], r[3], r[3],
              rows[r[3]][2] if r[3] < len(rows) else '(越界)'))
    if len(off) > 40:
        print('    …（還有 %d 筆）' % (len(off) - 40))
    bad += len(off)

    const_false = [r for r in rows if norm(r[4]) == 'false']
    const_true = [r for r in rows if norm(r[4]) == 'true']
    print('\n    bView：常數 true %d、常數 false %d、依組態 %d' % (len(const_true), len(const_false),
          len(rows) - len(const_true) - len(const_false)))

    if a.tsv:
        with open(a.tsv, 'w', encoding='utf-8', newline='\n') as f:
            f.write('j\tgolden_line\tname\tvalue\tbView\tpp\n')
            for j, n, nm, v, bv, pp in rows:
                f.write('%d\t%d\t%s\t%s\t%s\t%s\n' % (j, n, nm, v, bv, pp))
        print('    TSV 已寫：%s' % a.tsv)

    # 檢查 (3)
    if a.port:
        try:
            pl = read_text(a.port, a.port_big5)
        except (OSError, UnicodeDecodeError) as e:
            print('移植檔讀不到：%s' % e)
            return 2
        sp = ctor_span(pl) or (0, len(pl))
        p = [(n + sp[0], nm, bv, pp) for (n, nm, bv, pp) in parse_pushes(pl[sp[0]:sp[1]], a.port_label)]
        print('\n[3] %s：%s  建構子 :%d-%d  push_back %d 筆（註解與 #if 0 內不算）' % (a.port_label, a.port, sp[0] + 1, sp[1], len(p)))
        diff = 0
        for j in range(max(len(g), len(p))):
            gg = g[j] if j < len(g) else None
            pq = p[j] if j < len(p) else None
            if gg and pq and gg[1] == pq[1] and norm(gg[2]) == norm(pq[2]):
                continue
            diff += 1
            if diff <= 40:
                print('    j=%d golden:%s %s (%s)  vs  %s:%s %s (%s)' % (
                    j, gg[0] if gg else '-', gg[1] if gg else '-', gg[2] if gg else '-',
                    a.port_label, pq[0] if pq else '-', pq[1] if pq else '-', pq[2] if pq else '-'))
        gn = [x[1] for x in g]
        pn = [x[1] for x in p]
        print('    逐筆不同 %d 筆；golden 有而 %s 沒有：%s；%s 有而 golden 沒有：%s' % (
            diff, a.port_label, sorted(set(gn) - set(pn)) or '無', a.port_label, sorted(set(pn) - set(gn)) or '無'))
        bad += diff
    print('\n結果：%s' % ('全部檢查通過' if bad == 0 else '有 %d 項差異／失敗' % bad))
    return 0 if bad == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
