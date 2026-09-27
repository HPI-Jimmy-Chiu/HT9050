"""Recovery after a crashed flow_swap.py: read D:\\HT9045\\backup\\flowswap_ACTIVE.json and put system/config/setup.inf back,
remove the added recipe, then compare with the opmode manifest.  usage: python flow_unswap.py"""
import json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import flow_swap as fs

if not os.path.exists(fs.MARK):
    print('no active swap marker; nothing to do')
    sys.exit(0)
m = json.load(open(fs.MARK, encoding='utf-8'))
print('undoing swap', m['tag'], m['t'])
fs.unswap(m, print)
d = fs.manifest_diff()
print('re-check vs opmode:', 'CLEAN' if not d else 'DIFF %r' % d[:8])
if not d:
    os.remove(fs.MARK)
