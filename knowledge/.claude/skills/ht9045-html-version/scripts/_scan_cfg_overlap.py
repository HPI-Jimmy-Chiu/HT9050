# -*- coding: utf-8 -*-
"""掃描 cConfiguration.dfm：Visible=False 元件與同容器可見元件的幾何重疊"""
import io, re, sys
sys.path.insert(0, r'D:\AI_TempFile')

p = r'D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2\cConfiguration.dfm'
lines = io.open(p, encoding='cp950', errors='replace').read().split('\n')

# 簡易解析：object 樹
class N:
    def __init__(s, name, typ, parent):
        s.name, s.typ, s.parent = name, typ, parent
        s.props, s.kids = {}, []

root = None
cur = None
stack = []
i = 0
while i < len(lines):
    ln = lines[i].strip()
    m = re.match(r'(?:object|inline)\s+(\w+):\s*(\w+)', ln)
    if m:
        n = N(m.group(1), m.group(2), cur)
        if cur: cur.kids.append(n)
        else: root = n
        stack.append(n); cur = n
    elif ln == 'end' or ln == 'end>':
        if stack: stack.pop()
        cur = stack[-1] if stack else None
    else:
        m2 = re.match(r'(\w[\w.]*)\s*=\s*(.*)', ln)
        if m2 and cur:
            cur.props[m2.group(1)] = m2.group(2)
    i += 1

def geo(n):
    try:
        L = int(n.props.get('Left', '0'))
        T = int(n.props.get('Top', '0'))
        W = int(n.props.get('Width', '0'))
        H = int(n.props.get('Height', '0'))
        return L, T, W, H
    except ValueError:
        return 0, 0, 0, 0

def overlap(a, b):
    ax, ay, aw, ah = geo(a); bx, by, bw, bh = geo(b)
    ox = max(0, min(ax+aw, bx+bw) - max(ax, bx))
    oy = max(0, min(ay+ah, by+bh) - max(ay, by))
    return ox*oy

pairs = []
def walk(n):
    kids = [k for k in n.kids]
    hid = [k for k in kids if k.props.get('Visible') == 'False']
    for h in hid:
        hx, hy, hw, hh = geo(h)
        area_h = hw*hh or 1
        for o in kids:
            if o is h: continue
            a = overlap(h, o)
            if a > 0.3 * min(area_h, (geo(o)[2]*geo(o)[3]) or 1):
                ovis = 'HID' if o.props.get('Visible') == 'False' else 'vis'
                pairs.append((n.name, h.name, h.typ, o.name, o.typ, ovis, round(a/area_h, 2)))
    for k in kids: walk(k)
walk(root)
print('overlap pairs:', len(pairs))
for t in pairs:
    print(' | '.join(map(str, t)))
