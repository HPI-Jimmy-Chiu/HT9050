"""flow_cmp v2 -- compare a port Task_ListWithTime.csv with the reference StateRecord (FT005054) inside a data-driven window
(INBOX 49; RULINGS_20260927 3/5/6; research report staterecord-flow-gap.md sections 1, 3, 4).

usage: python flow_cmp.py <port.csv> [ref.csv] [--window S1] [--windows windows.json] [--allowlist allowlist.json]
                          [--no-allowlist] [--after HH:MM:SS.mmm|none] [--json out.json] [--txt out.txt] [--force]
  The text report goes to stdout (and --txt); the JSON to --json (default %TEMP%/ht9045_flowcmp/cmp/<dir>__<csv>.<window>.json).
  exit 0 = PASS, 1 = FAIL (differences), 2 = refused / bad input.

What is compared (window W of windows.json; reference = its ring restricted to W; port = everything after the port anchor):
 * every task that MOVED in the reference window (more than one distinct step): the reference sequence -- raw, or after
   allow-list rules -- must be a PREFIX of the port sequence (SIM keeps going where the real machine stopped on the alarm).
   Verdict EQUAL (raw) / EQUAL-mod-AL [ids] / DIFF(i) (first index that differs) / INCOMPLETE (the port ring wrapped after
   the anchor -- take an earlier snapshot, e.g. flow_run's anchor snapshots).
 * full rings (reference ring full, or a cycle-only rule): identical history = EQUAL, else the steady loop only (same unit up
   to rotation) = EQUAL-mod-AL; a full reference ring without a cycle-only rule is a DIFF (nothing is excused silently).
 * tasks that did not move in the reference window: DIFF when they move in the port between the start anchor and the
   production anchor (unexpected mover) or sit at another value.
 * row parity (rows registered / with data in one file only) and initial values (first recorded entry, unwrapped rings only).
The allow-list is refused as a whole when any entry lacks golden_cite.  Times are HH:MM:SS.mmm of the machine clock; the
port anchor is taken after the command named by the window's start_anchor.after_cmd when script_result.json (flow_run v2)
sits next to the port csv, or after --after."""
import argparse, hashlib, itertools, json, os, re, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from flow_ref import RING, collapse, fmt_ms, parse_pairs, steady_loop, tms

BS = chr(92)
REF = os.path.join('D:' + BS, 'HT9045', 'Staterecord', '2025-12-11 17_47_57', 'Task_ListWithTime.csv')
_OUT = os.environ.get('FLOW_OUT') or os.path.join(os.environ.get('TEMP', HERE), 'ht9045_flowcmp')   # never into the tree
KINDS = ('skip-steps', 'replace', 'stays', 'cycle-only', 'exclude-row', 'info')
SEQ_KINDS = ('skip-steps', 'replace', 'stays')
OKV = ('EQUAL', 'EQUAL-mod-AL')


class Refused(Exception):
    pass


def _ints(x):
    return isinstance(x, list) and all(isinstance(v, int) and not isinstance(v, bool) for v in x)


def _aliases(t):
    if isinstance(t, str):
        return [t]
    if isinstance(t, list) and all(isinstance(x, str) for x in t):
        return t
    return None


# ------------------------------------------------------------------------------------------------ allow-list
def load_allowlist(path):
    d = json.load(open(path, encoding='utf-8'))
    errs, entries, ids = [], [], set()
    for n, e in enumerate(d.get('entries', [])):
        eid = e.get('id') or ('entry#%d' % n)
        if eid in ids:
            errs.append('%s: duplicate id' % eid)
        ids.add(eid)
        cite = e.get('golden_cite')
        if not isinstance(cite, str) or not cite.strip():
            errs.append('%s: no golden_cite -- every allow-list entry must cite golden 906 file:line' % eid)
            continue
        rule = e.get('rule') or {}
        k = rule.get('kind')
        if k not in KINDS:
            errs.append('%s: rule.kind %r is not one of %s' % (eid, k, ', '.join(KINDS)))
            continue
        tasks = _aliases(e.get('task'))
        if tasks is None:
            errs.append('%s: task must be an alias or a list of aliases' % eid)
            continue
        if k != 'info' and not tasks:
            errs.append('%s: a %s rule needs at least one task' % (eid, k))
            continue
        if k == 'skip-steps':
            if not _ints(rule.get('steps')) or not rule['steps']:
                errs.append('%s: skip-steps needs steps: [int, ...]' % eid)
            b = rule.get('between')
            if b is not None and not (_ints(b) and len(b) == 2):
                errs.append('%s: between must be [a, b]' % eid)
        elif k == 'replace':
            pairs = rule.get('pairs')
            if not (isinstance(pairs, list) and pairs and
                    all(isinstance(p, dict) and _ints(p.get('from')) and p['from'] and _ints(p.get('to')) for p in pairs)):
                errs.append('%s: replace needs pairs: [{from: [int, ...], to: [int, ...]}]' % eid)
        elif k == 'stays':
            v = rule.get('value')
            if not isinstance(v, int) or isinstance(v, bool):
                errs.append('%s: stays needs value: int' % eid)
        st = e.get('status', 'unverified')
        if st not in ('verified', 'unverified'):
            errs.append('%s: status must be verified or unverified' % eid)
        entries.append({'id': eid, 'tasks': tasks, 'rule': rule, 'kind': k, 'cite': cite.strip(), 'status': st,
                        'note': e.get('verify_note', '')})
    pending = [{'id': p.get('id', '?'), 'tasks': _aliases(p.get('task')) or [], 'why': p.get('why_not_active', '')}
               for p in d.get('pending', [])]
    if errs:
        raise Refused('allow-list %s REFUSED (%d problem(s)):\n  ' % (path, len(errs)) + '\n  '.join(errs))
    return entries, pending


def skip_steps(seq, rule):
    steps = set(rule['steps'])
    b = rule.get('between')
    if not b:
        return collapse([v for v in seq if v not in steps])
    a, z = b
    out, i, n = [], 0, len(seq)
    while i < n:
        out.append(seq[i])
        if seq[i] == a:
            j = i + 1
            while j < n and seq[j] in steps:
                j += 1
            if j > i + 1 and j < n and seq[j] == z:     # only a complete run a, <steps...>, z
                i = j
                continue
        i += 1
    return collapse(out)


def replace_runs(seq, rule):
    out = list(seq)
    for p in rule['pairs']:
        f, t = p['from'], p['to']
        res, i = [], 0
        while i < len(out):
            if out[i:i + len(f)] == f:
                res.extend(t)
                i += len(f)
            else:
                res.append(out[i])
                i += 1
        out = res
    return collapse(out)


def apply_rules(seq, rules):
    for r in rules:                                   # allow-list file order
        if r['kind'] == 'skip-steps':
            seq = skip_steps(seq, r['rule'])
        elif r['kind'] == 'replace':
            seq = replace_runs(seq, r['rule'])
        elif r['kind'] == 'stays':
            seq = [r['rule']['value']]
    return seq


def plen(e, p):
    n = 0
    while n < len(e) and n < len(p) and e[n] == p[n]:
        n += 1
    return n


def best_match(eraw, p, rules):
    """(verdict, rules used, expected sequence, agreeing prefix length).  Raw first; then every combination of the task's
    sequence rules, fewest first; a 'stays' rule needs an exact match.  On no match: the combination that agrees longest."""
    if plen(eraw, p) == len(eraw):
        return 'EQUAL', [], eraw, len(eraw)
    best = ([], eraw, plen(eraw, p))
    for r in range(1, len(rules) + 1):
        for combo in itertools.combinations(rules, r):
            e = apply_rules(eraw, combo)
            exact = any(c['kind'] == 'stays' for c in combo)
            n = plen(e, p)
            if (e == p) if exact else (n == len(e)):
                return 'EQUAL-mod-AL', list(combo), e, n
            if n > best[2]:
                best = (list(combo), e, n)
    return 'DIFF', best[0], best[1], best[2]


def rot_equal(u, w):
    return bool(u) and len(u) == len(w) and any(w == u[i:] + u[:i] for i in range(len(u)))


# ------------------------------------------------------------------------------------------------ windows / anchors
def seq_from(pairs, start, end=None):
    """value at `start` (last entry at or before it) + every entry in (start, end], repeats collapsed"""
    prev, vals = None, []
    for ms, t, v in pairs:
        if ms <= start:
            prev = v
            continue
        if end is not None and ms > end:
            break
        vals.append(v)
    return collapse(([prev] if prev is not None else []) + vals)


def start_anchor(rows, spec, after):
    t, leave = spec['task'], spec['leave']
    r = rows.get(t)
    if not r or not r['pairs']:
        return None, 'no entries for %s in this file' % t
    p = r['pairs']
    for i in range(1, len(p)):
        if p[i][2] != leave and p[i - 1][2] == leave and (after is None or p[i][0] >= after):
            st = p[i - 1][0] if after is None else max(p[i - 1][0], after - 1)
            return {'task': t, 'leave': leave, 'to': p[i][2], 'at': p[i][0], 'start': st,
                    'how': '%s %d->%d' % (t, leave, p[i][2])}, ''
    for i in range(len(p)):
        if p[i][2] != leave and (after is None or p[i][0] >= after):
            return {'task': t, 'leave': leave, 'to': p[i][2], 'at': p[i][0], 'start': p[i][0] - 1,
                    'how': 'FALLBACK: %s first at %d with no recorded %d before it' % (t, p[i][2], leave)}, ''
    return None, '%s never leaves %d%s' % (t, leave, (' after %s' % fmt_ms(after)) if after is not None else '')


def reach_anchor(rows, spec, lo, hi=None):
    r = rows.get(spec['task'])
    for ms, t, v in (r['pairs'] if r else []):
        if ms > lo and (hi is None or ms <= hi) and v == spec['reach']:
            return ms
    return None


def after_from_script(port_csv, cmdname):
    """flow_run v2 writes script_result.json into the run dir; the sent time of the first `cmdname` step is the anchor floor"""
    p = os.path.join(os.path.dirname(os.path.abspath(port_csv)), 'script_result.json')
    if not cmdname or not os.path.isfile(p):
        return None, ''
    try:
        d = json.load(open(p, encoding='utf-8'))
    except (OSError, ValueError):
        return None, ''
    for s in d.get('steps', []):
        if s.get('cmd') == cmdname and s.get('sent_wall'):
            return tms(s['sent_wall']), 'script_result.json: %s sent at %s (ack ok=%s)' % (cmdname, s['sent_wall'], s.get('ack_ok'))
    return None, 'script_result.json has no %s step' % cmdname


# ------------------------------------------------------------------------------------------------ compare
def compare(port_path, ref_path, win, wid, entries, pending, after, after_how, ring):
    ref, rinfo = parse_pairs(ref_path, ring)
    port, pinfo = parse_pairs(port_path, ring)
    rs, rend = tms(win['ref_start']), tms(win['ref_end'])
    if rs is None or rend is None or rend <= rs:
        raise Refused('window %s: bad ref_start/ref_end %r %r' % (wid, win.get('ref_start'), win.get('ref_end')))
    if after is not None and pinfo['midnight'] and after < 12 * 3600000:   # same shift parse_pairs gave the port's entries
        after += 86400000
    anc, why = start_anchor(port, win['start_anchor'], after)
    if anc is None:
        raise Refused('port start anchor not found: %s' % why)
    ps = anc['start']
    prod = reach_anchor(port, win['prod_anchor'], ps) if win.get('prod_anchor') else None
    rprod = reach_anchor(ref, win['prod_anchor'], rs, rend) if win.get('prod_anchor') else None

    by_task = {}
    for e in entries:
        for t in e['tasks']:
            by_task.setdefault(t, []).append(e)
    pend_by = {}
    for p in pending:
        for t in p['tasks']:
            pend_by.setdefault(t, []).append(p['id'])
    excluded = sorted(t for t, es in by_task.items() if any(e['kind'] == 'exclude-row' for e in es))
    exs = set(excluded)

    def rules_of(t, kinds):
        return [e for e in by_task.get(t, []) if e['kind'] in kinds]

    def pseq(t, end=None):
        return seq_from(port[t]['pairs'], ps, end) if t in port else None

    tasks, full, movers, stat_diff = [], [], [], []
    stat_ok = 0
    for t in ref:                                      # reference file order
        if t in exs:
            continue
        rr = ref[t]
        eraw = seq_from(rr['pairs'], rs, rend)
        cyc = rules_of(t, ('cycle-only',))
        pinfo_t = port.get(t)
        note = []
        if pend_by.get(t):
            note.append('pending allow-list (NOT applied): %s' % ', '.join(pend_by[t]))
        if rr['full'] or cyc:
            efull = seq_from(rr['pairs'], rs)
            rec = {'task': t, 'kind': 'full-ring', 'ref_n': rr['n'], 'ref_earliest': fmt_ms(rr['pairs'][0][0]) if rr['pairs'] else '-',
                   'ref_window': eraw, 'ref_cycle': steady_loop(efull), 'al': [], 'note': note}
            if pinfo_t is None:
                rec.update(verdict='DIFF', first_diff=0, port_cycle=[], port_head=None)
                rec['note'].append('no row in the port file')
            else:
                p = pseq(t)
                rec['port_cycle'] = steady_loop(p)
                rec['port_head'] = p[:40]
                rec['port_n'] = pinfo_t['n']
                if p == efull:
                    rec.update(verdict='EQUAL', first_diff=None)
                elif rot_equal(rec['ref_cycle'], rec['port_cycle']) and cyc:
                    rec.update(verdict='EQUAL-mod-AL', first_diff=None, al=[cyc[0]['id']])
                else:
                    rec.update(verdict='DIFF', first_diff=None)
                    if not cyc:
                        rec['note'].append('reference ring is full but there is no cycle-only allow-list entry for it')
                    elif not rec['port_cycle']:
                        rec['note'].append('no steady loop in the port sequence (steady_loop: a unit repeated 3 times)')
                    else:
                        rec['note'].append('steady loops differ')
            full.append(rec)
        elif len(eraw) > 1:
            rec = {'task': t, 'kind': 'moved', 'ref_seq': eraw, 'al': [], 'note': note}
            seqr = rules_of(t, SEQ_KINDS)
            rec['al_available'] = [e['id'] for e in seqr]
            if pinfo_t is None:
                rec.update(verdict='DIFF', first_diff=0, expected=eraw, port_seq=None)
                rec['note'].append('no row in the port file')
            else:
                p = pseq(t)
                v, used, e, n = best_match(eraw, p, seqr)
                if v == 'DIFF' and pinfo_t['full'] and pinfo_t['pairs'] and pinfo_t['pairs'][0][0] > ps:
                    v = 'INCOMPLETE'
                    rec['note'].append('port ring (%d) wrapped after the anchor: earliest kept %s > window start %s'
                                       % (pinfo_t['n'], fmt_ms(pinfo_t['pairs'][0][0]), fmt_ms(ps)))
                rec.update(verdict=v, al=[u['id'] for u in used], expected=e, port_seq=p, match_len=n,
                           first_diff=None if v in OKV else n, port_more=max(0, len(p) - len(e)))
            tasks.append(rec)
        else:
            if pinfo_t is None:
                continue                                  # row parity reports it
            pp = pseq(t, prod)
            if len(pp) > 1:
                movers.append({'task': t, 'ref_seq': eraw, 'port_seq': pp[:40], 'note': 'moved in the port before the production anchor, not in the reference window'})
            elif eraw and pp and eraw[0] != pp[0]:
                stat_diff.append({'task': t, 'ref_value': eraw[0], 'port_value': pp[0]})
            else:
                stat_ok += 1
    for t in port:                                    # moved in the port, no row in the reference at all
        if t in ref or t in exs:
            continue
        pp = pseq(t, prod)
        if len(pp) > 1:
            movers.append({'task': t, 'ref_seq': None, 'port_seq': pp[:40], 'note': 'no row in the reference'})

    R, P = set(ref) - exs, set(port) - exs
    rows = {'ref_rows': len(ref), 'port_rows': len(port), 'excluded': excluded,
            'only_ref': sorted(R - P), 'only_port': sorted(P - R),
            'data_only_ref': sorted(t for t in R & P if ref[t]['n'] and not port[t]['n']),
            'data_only_port': sorted(t for t in R & P if port[t]['n'] and not ref[t]['n'])}
    init, wrapped = [], []
    for t in sorted(R & P):
        if not (ref[t]['n'] and port[t]['n']):
            continue
        if ref[t]['full'] or port[t]['full']:
            wrapped.append(t)
            continue
        a, b = ref[t]['pairs'][0][2], port[t]['pairs'][0][2]
        if a != b:
            init.append({'task': t, 'ref_first': a, 'port_first': b, 'port_first_at': port[t]['pairs'][0][1]})

    cnt = {}
    for r in tasks + full:
        cnt[r['verdict']] = cnt.get(r['verdict'], 0) + 1
    fail = (any(r['verdict'] not in OKV for r in tasks + full) or movers or stat_diff or rows['only_ref'] or rows['only_port']
            or rows['data_only_ref'] or rows['data_only_port'] or init)
    return {
        'tool': 'flow_cmp v2', 'generated': time.strftime('%Y-%m-%d %H:%M:%S'),
        'port': os.path.abspath(port_path), 'ref': os.path.abspath(ref_path),
        'port_md5': hashlib.md5(open(port_path, 'rb').read()).hexdigest(), 'ref_md5': hashlib.md5(open(ref_path, 'rb').read()).hexdigest(),
        'window': dict(win, id=wid), 'ring': ring, 'midnight': {'ref': rinfo['midnight'], 'port': pinfo['midnight']},
        'anchors': {'after': fmt_ms(after) if after is not None else None, 'after_how': after_how,
                    'port_start': dict(anc, at=fmt_ms(anc['at']), start=fmt_ms(anc['start'])),
                    'port_prod': fmt_ms(prod) if prod is not None else None,
                    'ref_start': win['ref_start'], 'ref_end': win['ref_end'], 'ref_prod': fmt_ms(rprod) if rprod is not None else None},
        'allowlist': {'entries': [{'id': e['id'], 'kind': e['kind'], 'tasks': e['tasks'], 'status': e['status'], 'cite': e['cite']} for e in entries],
                      'pending': pending},
        'summary': {'result': 'FAIL' if fail else 'PASS', 'verdicts': cnt, 'moved_tasks': len(tasks), 'full_ring_tasks': len(full),
                    'unexpected_movers': len(movers), 'stationary_value_diffs': len(stat_diff), 'stationary_ok': stat_ok,
                    'rows_only_ref': len(rows['only_ref']), 'rows_only_port': len(rows['only_port']),
                    'data_only_ref': len(rows['data_only_ref']), 'data_only_port': len(rows['data_only_port']),
                    'initial_value_diffs': len(init)},
        'tasks': tasks, 'full_ring': full, 'unexpected_movers': movers, 'stationary_diffs': stat_diff,
        'rows': rows, 'initial_values': {'diffs': init, 'skipped_wrapped': wrapped},
    }


# ------------------------------------------------------------------------------------------------ text
def _s(seq, n=40):
    if seq is None:
        return '(no row)'
    s = ' '.join(str(v) for v in seq[:n])
    return s + (' ... (+%d)' % (len(seq) - n) if len(seq) > n else '')


def _around(seq, i, w=3):
    if seq is None:
        return '(no row)'
    lo = max(0, i - w)
    return ('... ' if lo else '') + ' '.join(('[%s]' % v) if k == i else str(v) for k, v in enumerate(seq[lo:i + w + 1], lo)) + \
        (' ...' if i + w + 1 < len(seq) else '')


def text(res):
    a, w, s = res['anchors'], res['window'], res['summary']
    status = dict((e['id'], e['status']) for e in res['allowlist']['entries'])
    alid = lambda ids: ', '.join(i + ('?' if status.get(i) == 'unverified' else '') for i in ids)
    L = ['flow_cmp v2  window %s: %s' % (w['id'], w.get('desc', '')),
         'ref : %s  (%s .. %s, md5 %s)' % (res['ref'], a['ref_start'], a['ref_end'], res['ref_md5'][:8]),
         'port: %s  (md5 %s)' % (res['port'], res['port_md5'][:8]),
         'anchor: port window starts %s (%s at %s)%s' % (a['port_start']['start'], a['port_start']['how'], a['port_start']['at'],
                                                          ('; only after %s [%s]' % (a['after'], a['after_how'])) if a['after'] else ''),
         '        production anchor: port %s, reference %s' % (a['port_prod'] or 'NOT FOUND (unexpected-mover check covers the whole port run)', a['ref_prod'] or '-'),
         'allow-list: %d entries (%s)%s' % (len(res['allowlist']['entries']), alid([e['id'] for e in res['allowlist']['entries']]) or 'none',
                                           ('; pending, NOT applied: ' + ', '.join('%s (%s)' % (p['id'], p['why']) for p in res['allowlist']['pending'])) if res['allowlist']['pending'] else ''),
         '            (? = unverified entry)']
    if res['midnight']['ref'] or res['midnight']['port']:
        L.append('NOTE: a file spans midnight (ref=%s port=%s): times before 12:00 were taken as the next day' % (res['midnight']['ref'], res['midnight']['port']))
    L.append('')
    L.append('RESULT: %s   %s; unexpected movers %d; stationary value diffs %d; rows only-ref %d only-port %d; data only-ref %d only-port %d; initial-value diffs %d'
             % (s['result'], ', '.join('%s %d' % kv for kv in sorted(s['verdicts'].items())) or 'no tasks', s['unexpected_movers'],
                s['stationary_value_diffs'], s['rows_only_ref'], s['rows_only_port'], s['data_only_ref'], s['data_only_port'], s['initial_value_diffs']))
    L.append('')
    L.append('== tasks that moved in the reference window (%d) -- reference must be a prefix of the port ==' % len(res['tasks']))
    for r in res['tasks']:
        v = r['verdict'] if r['verdict'] in OKV else '%s(%s)' % (r['verdict'], r['first_diff'])
        L.append('[%-13s] %-36s %s%s' % (v, r['task'], ('AL ' + alid(r['al'])) if r['al'] else '',
                                         ('  (rules available: %s)' % alid(r['al_available'])) if r['al_available'] and r['verdict'] not in OKV else ''))
        if r['verdict'] in OKV:
            L.append('      ref : %s' % _s(r['ref_seq']))
            if r['al']:
                L.append('      exp : %s' % _s(r['expected']))
            if r.get('port_more'):
                L.append('      port continues with %d more step(s): %s' % (r['port_more'], _s(r['port_seq'][len(r['expected']):], 12)))
        else:
            i = r['first_diff'] or 0
            L.append('      ref : %s' % _s(r['ref_seq']))
            if r['expected'] != r['ref_seq']:
                L.append('      exp : %s' % _s(r['expected']))
            L.append('      port: %s' % _s(r['port_seq']))
            L.append('      at %d: expected %s | port %s' % (i, _around(r['expected'], i), _around(r['port_seq'], i)))
        for n in r['note']:
            L.append('      note: %s' % n)
    L.append('')
    L.append('== full rings, steady loop only (%d) ==' % len(res['full_ring']))
    for r in res['full_ring']:
        L.append('[%-13s] %-36s %s ref n=%d earliest %s  ref loop %s | port loop %s' % (
            r['verdict'], r['task'], ('AL ' + alid(r['al'])) if r['al'] else '', r['ref_n'], r['ref_earliest'], r['ref_cycle'], r.get('port_cycle')))
        for n in r['note']:
            L.append('      note: %s' % n)
    L.append('')
    L.append('== unexpected movers: moved in the port before the production anchor, not in the reference window (%d) ==' % len(res['unexpected_movers']))
    for r in res['unexpected_movers']:
        L.append('  %-36s ref %s | port %s  (%s)' % (r['task'], _s(r['ref_seq'], 6), _s(r['port_seq'], 16), r['note']))
    L.append('')
    L.append('== stationary tasks at another value (%d; %d agree) ==' % (len(res['stationary_diffs']), s['stationary_ok']))
    for r in res['stationary_diffs']:
        L.append('  %-36s ref %s | port %s' % (r['task'], r['ref_value'], r['port_value']))
    rw = res['rows']
    L.append('')
    L.append('== row parity (ref %d rows, port %d rows; excluded %d: %s) ==' % (rw['ref_rows'], rw['port_rows'], len(rw['excluded']), ', '.join(rw['excluded'])))
    L.append('  only in the reference (%d): %s' % (len(rw['only_ref']), ', '.join(rw['only_ref'])))
    L.append('  only in the port      (%d): %s' % (len(rw['only_port']), ', '.join(rw['only_port'])))
    L.append('  data only in the reference (%d): %s' % (len(rw['data_only_ref']), ', '.join(rw['data_only_ref'])))
    L.append('  data only in the port      (%d): %s' % (len(rw['data_only_port']), ', '.join(rw['data_only_port'])))
    iv = res['initial_values']
    L.append('')
    L.append('== initial values: first recorded entry (%d differ; %d skipped, ring full) ==' % (len(iv['diffs']), len(iv['skipped_wrapped'])))
    for r in iv['diffs']:
        L.append('  %-36s ref %s | port %s (port first at %s)' % (r['task'], r['ref_first'], r['port_first'], r['port_first_at']))
    return '\n'.join(L) + '\n'


def main(argv=None):
    ap = argparse.ArgumentParser(description='flow_cmp v2 (see the module docstring)')
    ap.add_argument('port')
    ap.add_argument('ref', nargs='?', default=None)
    ap.add_argument('--windows', default=os.path.join(HERE, 'windows.json'))
    ap.add_argument('--window', default=None)
    ap.add_argument('--allowlist', default=os.path.join(HERE, 'allowlist.json'))
    ap.add_argument('--no-allowlist', action='store_true')
    ap.add_argument('--after', default=None, help='HH:MM:SS.mmm, or none (default: from script_result.json when present)')
    ap.add_argument('--json', default=None)
    ap.add_argument('--txt', default=None)
    ap.add_argument('--force', action='store_true', help='allow a window with enabled=false')
    a = ap.parse_args(argv)
    try:
        W = json.load(open(a.windows, encoding='utf-8'))
        wid = a.window or W.get('default')
        win = (W.get('windows') or {}).get(wid)
        if not win:
            raise Refused('no window %r in %s' % (wid, a.windows))
        if not win.get('enabled', True) and not a.force:
            raise Refused('window %s is disabled: %s (use --force)' % (wid, win.get('why_disabled', '')))
        ref = a.ref or W.get('reference') or REF
        ring = int(W.get('ring') or RING)
        entries, pending = ([], []) if a.no_allowlist else load_allowlist(a.allowlist)
        if a.after and a.after.lower() == 'none':
            after, how = None, '--after none'
        elif a.after:
            after, how = tms(a.after), '--after'
            if after is None:
                raise Refused('--after %r is not HH:MM:SS.mmm' % a.after)
        else:
            after, how = after_from_script(a.port, win['start_anchor'].get('after_cmd'))
        res = compare(a.port, ref, win, wid, entries, pending, after, how, ring)
    except Refused as e:
        sys.stderr.write('flow_cmp: %s\n' % e)
        return 2
    base = re.sub(r'[^A-Za-z0-9_.-]+', '_', os.path.basename(os.path.dirname(os.path.abspath(a.port))) + '__' +
                  os.path.splitext(os.path.basename(a.port))[0])
    jp = a.json or os.path.join(_OUT, 'cmp', '%s.%s.json' % (base, wid))
    tp = a.txt or os.path.splitext(jp)[0] + '.txt'
    for p in (jp, tp):
        os.makedirs(os.path.dirname(os.path.abspath(p)), exist_ok=True)
    t = text(res)
    json.dump(res, open(jp, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    open(tp, 'w', encoding='utf-8').write(t)
    sys.stdout.write(t)
    sys.stdout.write('\njson: %s\ntext: %s\n' % (jp, tp))
    return 0 if res['summary']['result'] == 'PASS' else 1


if __name__ == '__main__':
    sys.exit(main())
