"""Reference action flow from a golden StateRecord Task_ListWithTime.csv (TfMain::SaveTaskList format, golden main.cpp:6402-6751):
each task row is 'Alias, t0, v0, t1, v1, ...' NEWEST first; empty time = slot never written.
Output JSON: {task: {"ran": bool, "n": transitions recorded, "first": t, "last": t, "seq": [values oldest->newest, consecutive repeats
collapsed], "cycle": shortest repeating unit of seq's tail if any}}.  Usage: python flow_ref.py <Task_ListWithTime.csv> <out.json>"""
import json, sys


def parse(path):
    out = {}
    for line in open(path, 'rb').read().decode('cp950', 'replace').splitlines():
        f = [x.strip() for x in line.split(',')]
        if not f or not f[0]:
            continue
        name, rest = f[0], f[1:]
        pairs = []
        for i in range(0, len(rest) - 1, 2):
            t, v = rest[i], rest[i + 1]
            if t == '' or v == '':
                continue
            try:
                vv = int(v)
            except ValueError:
                pairs = None
                break
            pairs.append((t, vv))
        if pairs is None:            # not a task row (the variable dump rows after the task block)
            continue
        if name in out:              # duplicate alias rows (e.g. LoadNewColorTrayToCarTask appears twice): keep both, suffixed
            name = name + '#2'
        pairs.reverse()              # oldest first
        seq = []
        for t, v in pairs:
            if not seq or seq[-1] != v:
                seq.append(v)
        out[name] = {'ran': bool(pairs), 'n': len(pairs), 'first': pairs[0][0] if pairs else '', 'last': pairs[-1][0] if pairs else '',
                     'seq': seq, 'cycle': cycle(seq)}
    return out


def cycle(seq):
    """shortest unit u such that the last 3*len(u) (or all, if shorter) values are u repeated -- the task's steady loop"""
    n = len(seq)
    for k in range(1, min(40, n // 2) + 1):
        tail = seq[-min(n, 3 * k):]
        u = seq[-k:]
        if all(tail[i] == u[(i - len(tail)) % k] for i in range(len(tail))) and len(tail) >= 2 * k:
            return u
    return []


if __name__ == '__main__':
    d = parse(sys.argv[1])
    json.dump(d, open(sys.argv[2], 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    ran = [k for k, v in d.items() if v['ran']]
    print('tasks', len(d), 'ran', len(ran))
    for k in ran:
        v = d[k]
        print('%-36s n=%3d %s..%s  steps=%3d  cycle=%s  head=%s' % (k, v['n'], v['first'], v['last'], len(v['seq']), v['cycle'][:12], v['seq'][:10]))
