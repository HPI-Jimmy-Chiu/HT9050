# -*- coding: utf-8 -*-
# Steven 20260915
# ----------------------------------------------------------------------
# 比對兩版 .dfm 找出只改名沒改程式碼的元件。（保存版）
# 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
# ----------------------------------------------------------------------

"""dfm_rename_diff.py -- 找出 V910 相對 V912 的 .dfm 元件改名。

AI(W906-FW-RENAME) 20260915。

使用者在 V910 樹只改了 .dfm 的元件名稱，程式碼沒動。這會讓「依 widget id 反查
golden .cpp」的抽取器對不上——網頁是照某一棵樹的 dfm 產生的，抽取卻用另一棵。

作法：同名 .dfm 兩棵樹各取 `object <id>: <TClass>` 清單，
  - 只在 V910 有 -> 新名
  - 只在 V912 有 -> 舊名
  再用「同一個父層、同一個 TClass、同一個出現序位」配對出改名候選。

輸出 JSON：{form: {v910_only, v912_only, pairs:[[v912舊, v910新, class]]}}
"""
import os, re, sys, json, io

V910 = r'D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2'
V912 = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
OUT  = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dfm_renames.json')

RE_OBJ = re.compile(r'^(\s*)object\s+([A-Za-z_]\w*)\s*:\s*(\w+)', re.M)


def read(p):
    for enc in ('utf-8', 'cp950', 'latin-1'):
        try:
            return open(p, encoding=enc, errors='strict').read()
        except (UnicodeDecodeError, LookupError):
            continue
    return open(p, encoding='latin-1', errors='replace').read()


def objects(path):
    """回傳 [(indent, id, class, 序位)]，序位是同 (indent,class) 內的出現次序。"""
    out, seen = [], {}
    for m in RE_OBJ.finditer(read(path)):
        indent, wid, cls = len(m.group(1)), m.group(2), m.group(3)
        k = (indent, cls)
        seen[k] = seen.get(k, 0) + 1
        out.append((indent, wid, cls, seen[k]))
    return out


def index(root):
    d = {}
    for dp, dn, fn in os.walk(root):
        if '.svn' in dp.split(os.sep):
            continue
        for f in fn:
            if f.lower().endswith('.dfm'):
                d.setdefault(f.lower(), os.path.join(dp, f))
    return d


def main():
    a, b = index(V910), index(V912)
    common = sorted(set(a) & set(b))
    result, total_pairs = {}, 0
    for f in common:
        oa, ob = objects(a[f]), objects(b[f])
        ia = {w: (ind, cls, seq) for ind, w, cls, seq in oa}
        ib = {w: (ind, cls, seq) for ind, w, cls, seq in ob}
        only_a = sorted(set(ia) - set(ib))     # V910 有、V912 沒有 -> 疑似新名
        only_b = sorted(set(ib) - set(ia))     # V912 有、V910 沒有 -> 疑似舊名
        if not only_a and not only_b:
            continue
        # 用 (indent, class, seq) 當指紋配對
        fa = {}
        for w in only_a:
            fa.setdefault(ia[w], []).append(w)
        pairs = []
        for w in only_b:
            key = ib[w]
            if key in fa and fa[key]:
                pairs.append([w, fa[key].pop(0), key[1]])
        paired_new = {p[1] for p in pairs}
        paired_old = {p[0] for p in pairs}
        result[f] = dict(
            v910_only=[w for w in only_a if w not in paired_new],
            v912_only=[w for w in only_b if w not in paired_old],
            pairs=pairs)
        total_pairs += len(pairs)
    with open(OUT, 'w', encoding='utf-8', newline='') as fh:
        json.dump(result, fh, ensure_ascii=False, indent=1)
    print('比對 %d 個同名 .dfm，%d 個有差異，配對出 %d 組改名候選'
          % (len(common), len(result), total_pairs))
    print('輸出: %s' % OUT)
    for f in sorted(result, key=lambda k: -len(result[k]['pairs']))[:12]:
        r = result[f]
        print('  %-30s 改名 %-3d  只在V910 %-3d  只在V912 %-3d'
              % (f, len(r['pairs']), len(r['v910_only']), len(r['v912_only'])))


main()
