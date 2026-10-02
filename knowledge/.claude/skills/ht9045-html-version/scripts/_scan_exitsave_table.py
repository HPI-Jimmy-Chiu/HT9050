# -*- coding: utf-8 -*-
"""掃描 30 dfm 全部符合 Exit/Save 統一規則的元件 → 輸出 markdown 表格"""
import io, os
src = io.open(r'D:\AI_TempFile\_gen_dfm_abs.py', encoding='utf-8').read()
ns = {}
exec(src[:src.find('def render')], ns)
exec(src[src.find('JOBS = ['):src.find('# Config 頁大量元件')], ns)
parse_dfm, cap, JOBS, BASE = ns['parse_dfm'], ns['cap'], ns['JOBS'], ns['BASE']
glyph_png, fontpx = ns['glyph_png'], ns['fontpx']
EXIT_CAPS, SAVE_CAPS = ns['EXIT_CAPS'], ns['SAVE_CAPS']

rows = []
def walk(n, page, title):
    c = cap(n).strip()
    uni = 'exit' if c in EXIT_CAPS else ('save' if c in SAVE_CAPS else None)
    if uni:
        if n.typ in ('TSpeedButton', 'TButton', 'TBitBtn'):
            g = glyph_png(n) if n.typ in ('TSpeedButton', 'TBitBtn') else None
            rows.append((page, title, n.name, n.typ, c, uni,
                         n.i('Left'), n.i('Top'), n.i('Width'), n.i('Height'),
                         g[0] if g else '—', fontpx(n)))
        elif n.typ == 'TPanel' and not n.kids and n.props.get('OnClick'):
            rows.append((page, title, n.name, 'TPanel', c, uni,
                         n.i('Left'), n.i('Top'), n.i('Width'), n.i('Height'),
                         '—', fontpx(n)))
    for k in n.kids:
        walk(k, page, title)

for dfm, out_, title in JOBS:
    walk(parse_dfm(os.path.join(BASE, dfm)), out_, title)

out = io.open(r'D:\AI_TempFile\_exitsave_table.md', 'w', encoding='utf-8')
out.write('| 頁面 | 元件 | 原型別 | Caption | 統一為 | 位置 L,T (W×H) | 原 glyph | 原 fs |\n')
out.write('|---|---|---|---|---|---|---|---|\n')
for p, t, name, typ, c, uni, L, T, W, H, g, fs in rows:
    out.write(f'| {p} | {name} | {typ} | {c} | {uni} | {L},{T} ({W}×{H}) | {g} | {fs} |\n')
out.close()
print(f'{len(rows)} rows -> _exitsave_table.md')
from collections import Counter
print(Counter(r[5] for r in rows))
print(Counter(r[3] for r in rows))
