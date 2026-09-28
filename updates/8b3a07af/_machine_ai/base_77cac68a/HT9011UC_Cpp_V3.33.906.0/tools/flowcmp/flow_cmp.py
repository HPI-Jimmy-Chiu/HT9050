"""Compare a port Task_ListWithTime.csv with the reference (golden StateRecord FT005054) task by task.
usage: python flow_cmp.py <port.csv> [ref.csv] > report.md
For every task that MOVED in the reference (more than one distinct step), print the reference's compressed step sequence head
and the port's, the first index where they differ, and the steps only one side reached.  Tasks that moved only in the port are
listed too.  Times are ignored (different windows); repeats are collapsed (flow_ref.parse)."""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from flow_ref import parse

REF = os.path.join('D:' + chr(92), 'HT9045', 'Staterecord', '2025-12-11 17_47_57', 'Task_ListWithTime.csv')


def moved(v):
    return v['ran'] and len(v['seq']) > 1


def main():
    port = parse(sys.argv[1])
    ref = parse(sys.argv[2] if len(sys.argv) > 2 else REF)
    rm = sorted(k for k, v in ref.items() if moved(v))
    pm = sorted(k for k, v in port.items() if moved(v))
    print('# flow compare\n\nref moved %d tasks, port moved %d; both %d\n' % (len(rm), len(pm), len(set(rm) & set(pm))))
    print('| task | ref steps (head) | port steps (head) | first diff | only ref | only port |')
    print('|---|---|---|---|---|---|')
    for k in rm:
        rs = ref[k]['seq']
        ps = port.get(k, {}).get('seq', []) if k in port else None
        if ps is None:
            print('| %s | %s | (no row) | - | - | - |' % (k, rs[:12]))
            continue
        d = next((i for i in range(min(len(rs), len(ps))) if rs[i] != ps[i]), min(len(rs), len(ps)))
        onlyr = sorted(set(rs) - set(ps))
        onlyp = sorted(set(ps) - set(rs))
        print('| %s | %s | %s | %d | %s | %s |' % (k, rs[:12], ps[:12], d, onlyr[:12], onlyp[:12]))
    extra = [k for k in pm if k not in rm]
    if extra:
        print('\n## moved only in the port\n')
        for k in extra:
            print('* %s: %s' % (k, port[k]['seq'][:16]))


if __name__ == '__main__':
    main()
