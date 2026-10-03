# AI(W906-FPFAST) 20261003 -- is proof.py's "WITHOUT" command really the command the tree compiled
# before the change?  Compare compile_commands.json of a configure of HEAD's CMakeLists.txt (no flag
# line) with the one of the changed tree, entry by entry (keyed by output object path):
#   head_cmd == changed_cmd with the one " -fexcess-precision=fast" token removed.
import json, sys

FLAG = ' -fexcess-precision=fast'
head = json.load(open(sys.argv[1], encoding='utf-8'))
chg = json.load(open(sys.argv[2], encoding='utf-8'))
hd_root, ch_root = sys.argv[3].lower(), sys.argv[4].lower()   # build dir paths, to normalise


def keyed(db, root):
    out = {}
    for e in db:
        k = e['output'].lower().replace(root, '@BUILD@')
        out[k] = e['command'].replace(e['directory'], '@BUILD@').replace(e['directory'].replace('/', chr(92)), '@BUILD@')
    return out


H = keyed(head, hd_root)
C = keyed(chg, ch_root)
both = sorted(set(H) & set(C))
same = 0
diff = []
for k in both:
    if H[k] == C[k].replace(FLAG, '', 1) and H[k].count('-fexcess-precision') == 0:
        same += 1
    else:
        diff.append(k)
print('head entries', len(H), 'changed entries', len(C), 'common', len(both), 'only-head', len(set(H) - set(C)), 'only-changed', len(set(C) - set(H)))
print('common entries where WITHOUT == HEAD exactly:', same, ' mismatches:', len(diff))
for k in diff[:10]:
    print('  MISMATCH', k)
    print('    head:', H[k][:400])
    print('    chg :', C[k][:400])
for k in sorted(set(C) - set(H)):
    print('  only in changed tree:', k)
