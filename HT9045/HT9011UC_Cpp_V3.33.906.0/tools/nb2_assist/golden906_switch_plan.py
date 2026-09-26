# -*- coding: utf-8 -*-
# ===========================================================================
#  tools/nb2_assist/golden906_switch_plan.py  --  NB2 輔助 session 20260925
#
#  問題：RULINGS_20260925 §15「翻譯對照一律 906」。be1a88de 只把 dfm2rc 改回 906；
#        S12 的三支產生器（gen_editlist／gen_formbridge／gen_sjson）與 4 支 editlist 設定檔的 golden 仍指 V912。
#        把它們改回 906 時：
#          (a) 哪些被轉的 golden 方法 906 與 912 **內容不同**（＝移植樹現在帶著 V912 專屬行為）？
#          (b) 設定檔裡寫死的 V912 行號（blocks／replace：方法, 起行, 迄行, …）在 906 是第幾行？
#              找不到的段落＝只存在 V912 的程式碼（改回 906 後那條 block 直接拿掉）。
#          (c) header 的 widget 名單差幾個？gen_sjson 的 ini 鍵對照差幾個？
#
#  做法：用**產生器自己的**函式（body_of／method_body／widgets_of／dfm_items，exec 產生器原始碼、截在
#        `STRUCTS = load_structs()`／`FORMS = load_forms()` 之前）讀兩棵 golden，確保量的是產生器實際會抓的那段。
#        設定檔照原樣 exec，只把 golden 路徑字串換成本機的 V912（D:\HT9045_ref 不在 NB2）。
#        方法內容比「去註解、去空白的程式碼行」multiset（第一級，字面）。
#
#  自我檢查：以 V912 跑時每條 block 的起行都必須落在該方法內（formbridge 另驗 must 字串）——
#        不成立代表本機 V912 與產生器用的 D:\HT9045_ref 那份不同，結果不可信，工具會明講。
#
#  ⚠ V912 要用**公司原版**（git 6943d134「匯入 … 原封不動」），不是 D:\HT9045 裡那份：後者有我們的維護修改
#    （例 uYieldMonitoring.cpp 的 acbcf268／17833c3a），行號會移動，自我檢查 11 條不過（NB2 20260925 實測）。
#    預設順序：D:\HT9045_ref（產生器用的那份）→ 沒有就用 D:\HT9045 那份並警告。要原版：
#      git archive -o v912.tar 6943d134 HT9011UC_Code_V3.33.912.0_20260908_Jimmy ＋ tar -x，再 --g912 指過去。
#
#  用法：python tools/nb2_assist/golden906_switch_plan.py [--g906 DIR] [--g912 DIR] [--json OUT]
# ===========================================================================
import argparse
import json
import os
import re
import sys
from collections import Counter

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
HERE = os.path.dirname(os.path.abspath(__file__))
TREE = os.path.abspath(os.path.join(HERE, '..', '..'))
TOOLS = os.path.join(TREE, 'tools')
REF912 = r'D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
DEF906 = r'D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618'
DEF912 = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'


def gen_namespace(fn, load_line, stop):
    """exec 產生器原始碼：把載入設定那行換成空清單（設定檔另外載），截在第一個模組層級副作用（寫檔）之前。"""
    src = open(os.path.join(TOOLS, fn), encoding='utf-8').read()
    src = src.replace(load_line, load_line.split('=')[0] + '= []', 1)
    i = src.rindex(stop) if stop else len(src)
    ns = {'__file__': os.path.join(TOOLS, fn), '__name__': 'nb2_' + fn[:-3]}
    src = re.sub(r'^sys[.]stdout = io[.]TextIOWrapper[(].*$', 'pass', src[:i], flags=re.M)
    exec(compile(src, fn, 'exec'), ns)
    return ns


def load_cfgs(sub, var, g912):
    out = []
    d = os.path.join(TOOLS, sub)
    for fn in sorted(os.listdir(d)):
        if not fn.endswith('.py') or fn.startswith('_'):
            continue
        src = open(os.path.join(d, fn), encoding='utf-8').read().replace(REF912, g912)
        ns = {'__file__': os.path.join(d, fn), '__name__': 'cfg'}
        try:
            exec(compile(src, fn, 'exec'), ns)
        except Exception as e:
            out.append((fn, None, 'load failed: %s' % e))
            continue
        out.append((fn, ns.get(var), None))
    return out


def code_lines(body, split):
    out = []
    in_block = False
    for raw in body.split('\n'):
        s = raw.rstrip('\r')
        if in_block:
            if '*/' in s:
                s = s[s.index('*/') + 2:]
                in_block = False
            else:
                continue
        # 去 /* … */（單行）與 //
        s = re.sub(r'/\*.*?\*/', ' ', s)
        if '/*' in s:
            s = s[:s.index('/*')]
            in_block = True
        s = ''.join(t for c, t in split(s) if c) if split else s.split('//')[0]
        s = ' '.join(s.split())
        if s and s not in ('{', '}', ';'):
            out.append(s)
    return out


def norm(l):
    return ' '.join(l.replace('\r', '').split('//')[0].split())


def find_block(src_lines, s, e, tgt_lines, lo, hi):
    """912 的 s..e 段（1-based）在 906 的 [lo,hi]（1-based）裡的位置。回傳 (起, 迄) 或 None。"""
    seg = [norm(x) for x in src_lines[s - 1:e]]
    seg_nz = [x for x in seg if x]
    if not seg_nz:
        return None
    first = seg_nz[0]
    for a in range(max(0, lo - 1), min(len(tgt_lines), hi)):
        if norm(tgt_lines[a]) != first:
            continue
        # 逐行吃非空行
        k, b = 0, a
        while b < len(tgt_lines) and k < len(seg_nz):
            t = norm(tgt_lines[b])
            if t:
                if t != seg_nz[k]:
                    break
                k += 1
            b += 1
        if k == len(seg_nz):
            return a + 1, b
    return None


_ALIGN = {}


def map_block(L2, L9, sp2, sp9, s, e):
    """912 的 s..e 在 906 的哪幾行：把 912 方法本體與 906 方法本體（去註解、去空白後）逐行對齊（difflib），
    s 與 e 都落在「相同」區段才算數。回傳 (a, b) 或 None（＝這段是 912 專屬／改過的）。
    ⚠ 20260925 更正：舊版只用 find_block（取第一個文字相符處），重複段落會全部對到第一個
    （例 TrayForm ReadFile 912 :341/:350/:374 三段同文，舊表全對到 906 :303）。"""
    import difflib
    key = (id(L2), id(L9), sp2, sp9)
    if key not in _ALIGN:
        a = [norm(x) for x in L2[sp2[0] - 1:sp2[1]]]
        b = [norm(x) for x in L9[sp9[0] - 1:sp9[1]]]
        m = {}
        for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
            if tag == 'equal':
                for k in range(i2 - i1):
                    m[sp2[0] + i1 + k] = sp9[0] + j1 + k
        _ALIGN[key] = m
    m = _ALIGN[key]
    if s in m and e in m and (m[e] - m[s]) == (e - s):
        return m[s], m[e]
    # 段落裡夾著 V912 多出／改過的行（例 BinSelect 建構子 912 :974-1078 只多一行 bAskCountPassword=false;）：
    # 取段內第一條與最後一條已對齊的行當 906 的起迄，標 MAP_RESIZED。整段一條都對不到才算 912 專屬。
    inner = [k for k in range(s, e + 1) if k in m]
    if len(inner) >= 2 or (inner and e == s):
        a0, b0 = m[inner[0]], m[inner[-1]]
        if b0 >= a0:
            MAP_RESIZED.add((s, e, (a0, b0)))
            return a0, b0
    # 退路：段落落在 difflib 的「改過」區段（常見於 V912 在旁邊多插了幾行）。用 s 前面最近一條已對齊的行推出
    # 預期位置，在 906 方法內找同文字段落、取最近的一處（±30 行內）。呼叫端可用 MAP_APPROX 知道是推算的。
    prev = [k for k in m if k < s]
    if not prev:
        return None
    p = max(prev)
    expect = m[p] + (s - p)
    seg = [x for x in (norm(y) for y in L2[s - 1:e]) if x]
    best = None
    for a0 in range(sp9[0], sp9[1] + 1):
        hit = find_block(L2, s, e, L9, a0, a0)
        if hit and (best is None or abs(hit[0] - expect) < abs(best[0] - expect)):
            best = hit
    if best and abs(best[0] - expect) <= 30 and seg:
        MAP_APPROX.add((s, e, best))
        return best
    return None


MAP_APPROX = set()
MAP_RESIZED = set()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--g906', default=DEF906)
    ap.add_argument('--g912', default=REF912 if os.path.isdir(REF912) else DEF912)
    ap.add_argument('--json')
    a = ap.parse_args()
    el = gen_namespace('gen_editlist.py', 'STRUCTS = load_structs()', '\nONLY = sys.argv')
    fb = gen_namespace('gen_formbridge.py', 'FORMS = load_forms()', '\nmain()')
    cp = el['cp950']
    rows, selfcheck = [], []

    def one(kind, fn, cfg, getbody, blocks, bfmt):
        cpp9, cpp2 = (cp(os.path.join(g, cfg['cpp'])) for g in (a.g906, a.g912))
        h9, h2 = (cp(os.path.join(g, cfg['h'])) for g in (a.g906, a.g912))
        L9 = cpp9.replace('\r\n', '\n').split('\n')
        L2 = cpp2.replace('\r\n', '\n').split('\n')
        w9 = set(el['widgets_of'](h9, cfg['class']))
        w2 = set(el['widgets_of'](h2, cfg['class']))
        r = {'kind': kind, 'cfg': fn, 'cpp': cfg['cpp'], 'class': cfg['class'], 'methods': [], 'blocks': [],
             'widgets_only912': sorted(w2 - w9), 'widgets_only906': sorted(w9 - w2)}
        spans9, spans2 = {}, {}
        for m in cfg['methods']:
            try:
                b2, l2 = getbody(cpp2, cfg['class'], m)
            except BaseException as e:
                r['methods'].append({'m': m, 'err912': str(e)[:120]})
                continue
            try:
                b9, l9 = getbody(cpp9, cfg['class'], m)
            except BaseException as e:
                r['methods'].append({'m': m, 'l912': l2, 'err906': 'not in 906: %s' % str(e)[:80]})
                continue
            spans2[m] = (l2, l2 + b2.count('\n'))
            spans9[m] = (l9, l9 + b9.count('\n'))
            c9, c2 = Counter(code_lines(b9, el['split_code'])), Counter(code_lines(b2, el['split_code']))
            o9, o2 = list((c9 - c2).elements()), list((c2 - c9).elements())
            r['methods'].append({'m': m, 'l906': l9, 'l912': l2, 'only906': o9, 'only912': o2})
        for blk in blocks:
            meth, s, e = bfmt(blk)
            ent = {'m': meth, 's912': s, 'e912': e, 'reason': str(blk[3])[:90]}
            sp2 = spans2.get(meth)
            if not sp2 or not (sp2[0] <= s <= sp2[1]):
                selfcheck.append('%s %s block %s:%d 不在 V912 %s 的方法範圍 %s' % (kind, fn, cfg['cpp'], s, meth, sp2))
            if kind == 'formbridge' and blk[3] not in L2[s - 1]:
                selfcheck.append('%s %s block %s:%d 起行不含 must 字串 %r' % (kind, fn, cfg['cpp'], s, blk[3][:40]))
            sp9 = spans9.get(meth)
            hit = map_block(L2, L9, sp2, sp9, s, e) if (sp9 and sp2) else None
            if not hit:
                hit2 = find_block(L2, s, e, L9, 1, len(L9))
                ent['r906'] = ('elsewhere', hit2) if hit2 else None
            else:
                ent['r906'] = hit
            r['blocks'].append(ent)
        # formbridge overrides (meth, old, new)：old 要在方法本體找得到，否則產生器會報「override 沒用到」
        r['ov_miss906'] = []
        if kind == 'formbridge':
            for meth, old, new in cfg.get('overrides', []):
                try:
                    b9, _ = getbody(cpp9, cfg['class'], meth)
                except BaseException:
                    b9 = ''
                if old not in b9:
                    r['ov_miss906'].append((meth, old[:80]))
        rows.append(r)

    def el_body(cpp, cls, m):
        params, body, line0, sig = el['body_of'](cpp, cls, m)
        return body, line0

    def fb_body(cpp, cls, m):
        params, body, line0 = fb['method_body'](cpp, cls, m)
        return body, line0

    for fn, st, err in load_cfgs('editlist', 'STRUCT', a.g912):
        if err or not st:
            rows.append({'kind': 'editlist', 'cfg': fn, 'err': err})
            continue
        one('editlist', fn, st, el_body, list(st.get('blocks', [])) + list(st.get('replace', [])), lambda b: (b[0], b[1], b[2]))
    for fn, fo, err in load_cfgs('formbridge', 'FORM', a.g912):
        if err or not fo:
            rows.append({'kind': 'formbridge', 'cfg': fn, 'err': err})
            continue
        one('formbridge', fn, fo, fb_body, list(fo.get('blocks', [])), lambda b: (b[0], b[1], b[2]))

    # gen_sjson ini 鍵對照
    sj = gen_namespace('gen_sjson.py', '@@none@@', "\nif __name__ == '__main__':")
    sjrows = []
    for struct, inst in sorted(sj.get('INI_INSTANCE', {}).items()):
        if not inst:
            continue
        res = {}
        for tag, g in (('906', a.g906), ('912', a.g912)):
            sj['GOLDEN'] = g
            mp = sj['scan_ini_map']((inst,))
            res[tag] = {(f, x[1], x[2]) for f, v in mp.items() for x in v}
        sjrows.append({'struct': struct, 'inst': inst, 'n906': len(res['906']), 'n912': len(res['912']),
                       'only912': sorted(res['912'] - res['906']), 'only906': sorted(res['906'] - res['912'])})

    # ---------------- 報告 ----------------
    P = print
    P('# golden 改回 906 的換算表（S12 產生器）\n')
    P('golden 906＝`%s`；V912＝`%s`（產生器寫的是 `%s`）。\n' % (a.g906, a.g912, REF912))
    if selfcheck:
        P('## ⚠ 自我檢查失敗 %d 條（本機 V912 與產生器用的那份可能不同；下列結果要打折）\n' % len(selfcheck))
        for s in selfcheck:
            P('* ' + s)
        P()
    else:
        P('自我檢查：所有 block 的 V912 起行都落在該方法內、formbridge 的 must 字串都對得上 ⇒ 本機 V912 與產生器那份一致。\n')
    P('## 1. 每個設定：被轉的方法 906↔912 是否相同、block 行號換算\n')
    P('| 設定 | golden | 方法（同/異/缺） | 內容不同的方法 | block（可換算/他處/906沒有） | widget 只在 912 |')
    P('|---|---|---|---|---|---|')
    for r in rows:
        if r.get('err'):
            P('| `%s/%s` | — | 載入失敗：%s | | | |' % (r['kind'], r['cfg'], r['err']))
            continue
        same = [m for m in r['methods'] if 'only906' in m and not m['only906'] and not m['only912']]
        diff = [m for m in r['methods'] if 'only906' in m and (m['only906'] or m['only912'])]
        miss = [m for m in r['methods'] if 'err906' in m or 'err912' in m]
        bok = [b for b in r['blocks'] if isinstance(b['r906'], tuple) and b['r906'][0] != 'elsewhere']
        bel = [b for b in r['blocks'] if isinstance(b['r906'], tuple) and b['r906'][0] == 'elsewhere']
        bno = [b for b in r['blocks'] if b['r906'] is None]
        P('| `%s/%s` | `%s` | %d/%d/%d | %s | %d/%d/%d | %s |' % (
            r['kind'], r['cfg'], r['cpp'], len(same), len(diff), len(miss),
            '、'.join('%s(+%d/−%d)' % (m['m'], len(m['only912']), len(m['only906'])) for m in diff) or '—',
            len(bok), len(bel), len(bno), len(r['widgets_only912'])))
    P('\n（「+a/−b」＝V912 多 a 行、少 b 行程式碼。）\n')
    ms = [m for r in rows for m in r.get('methods', [])]
    same = sum(1 for m in ms if 'only906' in m and not m['only906'] and not m['only912'])
    diff = [m for m in ms if 'only906' in m and (m['only906'] or m['only912'])]
    bl = [b for r in rows for b in r.get('blocks', [])]
    P('**合計**：被轉的方法 %d 個＝906 與 912 相同 %d、不同 %d（V912 多 %d 行、少 %d 行）、只有一邊有 %d；'
      'block／replace %d 條＝可換算到 906 %d、在方法外 %d、906 沒有 %d。formbridge overrides 在 906 找不到的 %d 條。\n' % (
          len(ms), same, len(diff), sum(len(m['only912']) for m in diff), sum(len(m['only906']) for m in diff),
          len(ms) - same - len(diff), len(bl),
          sum(1 for b in bl if isinstance(b['r906'], tuple) and b['r906'][0] != 'elsewhere'),
          sum(1 for b in bl if isinstance(b['r906'], tuple) and b['r906'][0] == 'elsewhere'),
          sum(1 for b in bl if b['r906'] is None), sum(len(r.get('ov_miss906', [])) for r in rows)))
    for r in rows:
        for meth, old in r.get('ov_miss906', []):
            P('* override 在 906 找不到：`%s` %s `%s`' % (r['cfg'], meth, old))
    P('## 2. 內容不同的方法：V912 專屬行（改回 906 後會消失）與 906 專屬行（會出現）\n')
    for r in rows:
        for m in r.get('methods', []):
            if m.get('only906') or m.get('only912'):
                P('### `%s` %s::%s（906 :%d ↔ 912 :%d）\n' % (r['cpp'], r['class'], m['m'], m['l906'], m['l912']))
                for s in m['only912'][:12]:
                    P('* 912＋ `%s`' % s[:170])
                if len(m['only912']) > 12:
                    P('* …912＋ 另 %d 行' % (len(m['only912']) - 12))
                for s in m['only906'][:12]:
                    P('* 906＋ `%s`' % s[:170])
                if len(m['only906']) > 12:
                    P('* …906＋ 另 %d 行' % (len(m['only906']) - 12))
                P()
            if m.get('err906') or m.get('err912'):
                P('* ⚠ `%s` %s::%s：%s\n' % (r['cpp'], r['class'], m['m'], m.get('err906') or m.get('err912')))
    P('## 3. block／replace 行號換算（改設定檔用）\n')
    P('| 設定 | 方法 | V912 起-迄 | 906 起-迄 | 原因（節錄） |\n|---|---|---|---|---|')
    for r in rows:
        for b in r.get('blocks', []):
            if b['r906'] is None:
                t = '**906 沒有**（V912 專屬段；改回 906 後拿掉這條）'
            elif b['r906'][0] == 'elsewhere':
                t = '在方法外 %s（要人工看）' % (b['r906'][1],)
            else:
                t = '%d-%d' % b['r906']
            P('| `%s` | %s | %d-%d | %s | %s |' % (r['cfg'], b['m'], b['s912'], b['e912'], t, b['reason'].replace('|', '/')))
    P('\n## 4. widget 只在一邊的\n')
    for r in rows:
        if r.get('widgets_only912') or r.get('widgets_only906'):
            P('* `%s`（%s）：只在 912 %s；只在 906 %s' % (r['cfg'], r['class'], r['widgets_only912'] or '—', r['widgets_only906'] or '—'))
    P('\n## 5. gen_sjson 的 ini 鍵對照（欄位, 區段, 鍵）\n')
    P('| 結構 | 實例 | 906 | 912 | 只在 912 | 只在 906 |\n|---|---|---|---|---|---|')
    for s in sjrows:
        P('| %s | %s | %d | %d | %s | %s |' % (s['struct'], s['inst'], s['n906'], s['n912'],
                                            '；'.join('%s[%s]%s' % x for x in s['only912'][:8]) or '—',
                                            '；'.join('%s[%s]%s' % x for x in s['only906'][:8]) or '—'))
    if a.json:
        json.dump({'rows': rows, 'sjson': [dict(s, only912=[list(x) for x in s['only912']], only906=[list(x) for x in s['only906']]) for s in sjrows],
                   'selfcheck': selfcheck}, open(a.json, 'w', encoding='utf-8'), ensure_ascii=False, indent=1, default=str)


if __name__ == '__main__':
    main()
