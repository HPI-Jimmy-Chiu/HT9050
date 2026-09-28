"""Reference action flow from a golden StateRecord Task_ListWithTime.csv (TfMain::SaveTaskList format, golden main.cpp:6402-6751):
each task row is 'Alias, t0, v0, t1, v1, ...' NEWEST first; empty time = slot never written.
Output JSON: {task: {"ran": bool, "n": transitions recorded, "first": t, "last": t, "seq": [values oldest->newest, consecutive repeats
collapsed], "cycle": shortest repeating unit of seq's tail if any}}.  Usage: python flow_ref.py <Task_ListWithTime.csv> <out.json>

v2 (20260927, flow_cmp v2): parse_pairs() keeps the timed entries and tells task rows from the variable-dump rows; parse() is
unchanged (v1 callers: rancmp.py and friends)."""
import json, sys

RING = 500                  # entries kept per task: golden cpublic.h:237 `#define MAX_Q_10 500`
TASK_ROW_MIN_FIELDS = 10    # a task row has one field per ring slot (>= MAX_Q_10); the dump rows after the task block
                            # (bPickFromLoader, MOT[...].x, CleanKitTime, MainProcMonitor ...) have at most 9


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


# ------------------------------------------------------------------------------------------------ v2 helpers
def tms(t):
    """'HH:MM:SS.mmm' -> milliseconds of the day; None when it is not a time"""
    try:
        h, m, s = t.split(':')
        return int(round((int(h) * 3600 + int(m) * 60 + float(s)) * 1000))
    except (ValueError, AttributeError):
        return None


def fmt_ms(ms):
    if ms is None:
        return '-'
    d, ms = divmod(int(ms), 86400000)
    h, r = divmod(ms, 3600000)
    m, r = divmod(r, 60000)
    s = '%02d:%02d:%02d.%03d' % (h, m, r // 1000, r % 1000)
    return s + ('(+%dd)' % d if d else '')


def collapse(vals):
    out = []
    for v in vals:
        if not out or out[-1] != v:
            out.append(v)
    return out


def steady_loop(seq, kmax=40, reps=3):
    """The task's steady loop anywhere in seq (cycle() above only looks at the tail, and a reference ring usually ends in a
    disturbed state): the unit u, 1 <= len(u) <= kmax, of the longest stretch of seq that is u repeated at least `reps`
    times; ties -> the shorter unit.  Returned as its lexicographically smallest rotation so two loops compare with ==.
    [] when no unit repeats `reps` times."""
    n, best = len(seq), None                       # best = (stretch length, -k, unit)
    for k in range(1, min(kmax, n // reps) + 1):
        cur = 0
        for j in range(k, n):
            cur = cur + 1 if seq[j] == seq[j - k] else 0
            if cur + k >= reps * k:
                cand = (cur + k, -k)
                if best is None or cand > best[:2]:
                    best = (cur + k, -k, seq[j - k + 1:j + 1])
    if best is None:
        return []
    u = list(best[2])
    return min(u[i:] + u[:i] for i in range(len(u)))


def parse_pairs(path, ring=RING):
    """Task rows only -> {alias: {'pairs': [(ms, 'HH:MM:SS.mmm', value) oldest first], 'n', 'full': n >= ring, 'row': file row,
    'bad': unparsable entries}}.  Rows with fewer than TASK_ROW_MIN_FIELDS fields are the variable dump, not tasks; rows with
    an empty alias are unregistered QueueTaskList slots.  Duplicate aliases get '#2', '#3'.
    The file stores only HH:MM:SS.mmm: when it holds times >= 18:00 AND < 06:00 it is taken to span midnight and times before
    12:00 get +24 h (a run that long is unusual; the report says so when it happens)."""
    rows, raw = {}, []
    for ln, line in enumerate(open(path, 'rb').read().decode('cp950', 'replace').splitlines(), 1):
        f = [x.strip() for x in line.split(',')]
        if len(f) < TASK_ROW_MIN_FIELDS or not f[0]:
            continue
        rest, pairs, bad = f[1:], [], 0
        for i in range(0, len(rest) - 1, 2):
            t, v = rest[i], rest[i + 1]
            if t == '' or v == '':
                continue
            ms = tms(t)
            try:
                vv = int(v)
            except ValueError:
                vv = None
            if ms is None or vv is None:
                bad += 1
                continue
            pairs.append([ms, t, vv])
        pairs.reverse()              # the ring is written newest first (golden main.cpp:6402-6751 / port cStateRecord.cpp:140)
        name, k = f[0], 2
        while name in rows:
            name = '%s#%d' % (f[0], k)
            k += 1
        rows[name] = {'pairs': pairs, 'n': len(pairs), 'full': len(pairs) >= ring, 'row': ln, 'bad': bad}
        raw.extend(pairs)
    allms = [p[0] for p in raw]
    midnight = bool(allms) and max(allms) >= 18 * 3600000 and min(allms) < 6 * 3600000
    if midnight:
        for r in rows.values():
            for p in r['pairs']:
                if p[0] < 12 * 3600000:
                    p[0] += 86400000
            r['pairs'].sort(key=lambda p: p[0])
    for r in rows.values():
        r['pairs'] = [tuple(p) for p in r['pairs']]
    return rows, {'midnight': midnight}


if __name__ == '__main__':
    d = parse(sys.argv[1])
    json.dump(d, open(sys.argv[2], 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    ran = [k for k, v in d.items() if v['ran']]
    print('tasks', len(d), 'ran', len(ran))
    for k in ran:
        v = d[k]
        print('%-36s n=%3d %s..%s  steps=%3d  cycle=%s  head=%s' % (k, v['n'], v['first'], v['last'], len(v['seq']), v['cycle'][:12], v['seq'][:10]))
