# -*- coding: utf-8 -*-
# Read-only: predict which ContactInfo.ini keys the V912 TfContactForce ctor (+ReadFile) would seed at boot on this machine.
# Assumes CUSTOMER_CODE=791 (not ASE_SG / KYEC), EP_Install=3 (!=5), INSTALL_DOUBLE_EP=1 (not MULTI), bUseDynamicKitDiameter=true.
import re

P = r'D:\HT9045\system\ContactInfo.ini'
sec, cur = {}, None
for raw in open(P, 'rb').read().decode('cp950', errors='replace').splitlines():
    s = raw.strip()
    m = re.match(r'^\[(.*)\]$', s)
    if m:
        cur = m.group(1).lower()
        sec.setdefault(cur, {})
        continue
    if cur is not None and '=' in s:
        k, v = s.split('=', 1)
        sec[cur][k.strip().lower()] = v


def has(g, k):
    return k.lower() in sec.get(g.lower(), {})


missing = []


def need(g, k, dflt):
    if not has(g, k):
        missing.append((g, k, dflt))
        sec.setdefault(g.lower(), {})[k.lower()] = dflt


# ctor
need('SLK Type', 'Type', '30,40,60,56')
need('SLK Type', 'Visible', '1,1,1,0')
types = [t for t in sec['slk type']['type'].split(',')]
need('SLK Type', 'DieForceType', '20,30,40,50')
need('SLK Type', 'DieForceVisible', '1,1,1,0')
dtypes = sec['slk type']['dieforcetype'].split(',')
# ReadFile SLKClass
for t in types:
    if t and float(t) > 15.0:
        g = 'Diameter_%0.3fmm' % float(t)
        for k in ('LoadRate', 'LoadRate_NS', 'HotOffset', 'ContactOffset', 'ContactOffset_NS'):
            need(g, k, '1.0000' if k.startswith('LoadRate') else '0.0000')
for t in dtypes:
    if t and float(t) > 15.0:
        g = 'DieForceDiameter_%0.3fmm' % float(t)
        need(g, 'LoadRate', '1.0000')
        need(g, 'ContactOffset', '1.0000')
# ReadFile SLKIndClass (non-MULTI): i*16+j, Ind list follows [SLK Type] in V912
ind_types = [t for t in types if t and float(t) > 15.0]
for t in ind_types:
    for j in range(16):
        g = 'Diameter_%0.3fmm_%d' % (float(t), j)
        need(g, 'LoadRate', '1.0000')
        need(g, 'ContactOffset', '0.0000')

secs = sorted(set(g for g, _, _ in missing), key=lambda x: (x.split('_')[1], int(x.rsplit('_', 1)[1]) if x.rsplit('_', 1)[1].isdigit() else -1))
print('missing keys: %d in %d sections' % (len(missing), len(secs)))
diam = {}
for g in secs:
    d = g.split('_')[1]
    diam[d] = diam.get(d, 0) + 1
print('sections per diameter:', diam)
print('other (non-Ind) missing:', [(g, k) for g, k, _ in missing if not re.search(r'mm_\d+$', g)])
