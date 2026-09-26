#!/usr/bin/env python3
# AI(W906-CSVONLY) 20260926: MDB Updater (SQLiteUpdater.exe) -> V906, CSV version (cMyDB plan P2).
#
# Reads the static alarm-code catalog out of the BCB6 MDB Updater source
#   D:\MDB Updater\MDB_Updater_Rev902.0_20260410\CreatDatabase.cpp   (Big5)
# and emits
#   AlarmCodeCatalog.cpp            (generated C++ tables, UTF-8, CRLF)
#   ship/Error/AlarmCodeList.txt    (what SQLiteUpdater writes to D:\HT9045\Error\AlarmCodeList.txt; ASCII, CRLF)
#
# Faithfulness rules (golden = MDB Updater Main.cpp / CreatDatabase.cpp):
#   * order = CreateTableAlarmList(): "WAR000000" first, then CreateTableAlarmList_001 .. _031 in call order,
#     each unit's DoInsertAlarmCode calls in source order.
#   * Unit 24 is not literal: CreateTableAlarmList_024_Motor() builds MotorList (164 names) x AlarmList (9 texts)
#     and inserts sprintf("WAR24%03d%d", i, j) / sprintf("%s -- %s", motor, alarm).  The generator keeps the two
#     lists and the C++ replays the same double loop, so the table stays reviewable against the source.
#   * duplicates: golden DoInsertAlarmCode keeps the FIRST occurrence and logs "@@Error!!" +
#     "Alarm Code: <code> is duplicate!!".  The generator reports them; the C++ reproduces the same behaviour.
#   * sqlite (INSERT OR REPLACE INTO AlarmList / CreateTable / CreateView / DoSaveEventLog) is retired
#     (user ruling 20260926) and not carried over.
#   * TraceDB (one-time Tray.DB/Plate.DB -> TrayForm.csv/PlateForm.csv BDE migration) is DEFERRED
#     (user 20260926: 不重要, 往後排); V906 already reads the two CSV files directly.
#
# Validated 20260926 against a real machine file (D:\HT9045\Error\AlarmCodeList.txt on STEVEN-NB3, written by
# the 2023 Rev780 updater): run with --src <Rev780 CreatDatabase.cpp> --check <that file> -> 2528 of 2530 catalog
# lines identical and in order; the 2 differences are message-text edits inside the WAR1532x range of that build.
#
# Usage:  python tools/gen_alarmcode_catalog.py [--src <CreatDatabase.cpp>] [--check <AlarmCodeList.txt>]
#   --check compares the generated list with a real machine AlarmCodeList.txt and prints the result.
import argparse, hashlib, io, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
TREE = os.path.dirname(HERE)
DEFAULT_SRC = r'D:\MDB Updater\MDB_Updater_Rev902.0_20260410\CreatDatabase.cpp'
CALL = re.compile(r'^\s*DoInsertAlarmCode\(\s*"([A-Z]{3}\d+)"\s*,\s*"((?:[^"\\]|\\.)*)"\s*\)\s*;')
ADD = re.compile(r'^\s*(MotorList|AlarmList)->Add\(\s*"((?:[^"\\]|\\.)*)"\s*\)\s*;')
FUNC = re.compile(r'^void\s+(\w+)\(\)')


def parse(src_text):
    lines = src_text.splitlines()
    bodies, cur = {}, None
    for n, l in enumerate(lines, 1):
        m = FUNC.match(l)
        if m:
            cur = m.group(1)
            bodies[cur] = []
            continue
        if cur is not None:
            bodies[cur].append((n, l))
    top = bodies['CreateTableAlarmList']
    order, head = [], []
    for n, l in top:
        m = re.match(r'^\s*(CreateTableAlarmList_\w+)\(\)\s*;', l)
        if m:
            order.append(m.group(1))
        c = CALL.match(l)
        if c:
            head.append((c.group(1), c.group(2), n))
    units = []
    motors, alarms = [], []
    for fn in order:
        entries = []
        for n, l in bodies[fn]:
            if l.lstrip().startswith('//'):
                continue
            c = CALL.match(l)
            if c:
                entries.append((c.group(1), c.group(2), n))
                continue
            a = ADD.match(l)
            if a and fn.endswith('_024_Motor'):
                (motors if a.group(1) == 'MotorList' else alarms).append(a.group(2))
            elif 'DoInsertAlarmCode(' in l and not fn.endswith('_024_Motor'):
                raise SystemExit('unparsed DoInsertAlarmCode at line %d: %s' % (n, l.strip()))
        units.append((fn, entries))
    return head, units, motors, alarms


def replay(head, units, motors, alarms):
    """Exactly the golden insertion sequence (before dedup)."""
    seq = [(c, m) for c, m, _ in head]
    for fn, entries in units:
        seq += [(c, m) for c, m, _ in entries]
        if fn.endswith('_024_Motor'):
            for i, mot in enumerate(motors):
                for j, al in enumerate(alarms):
                    seq.append(('WAR24%03d%d' % (i, j), '%s -- %s' % (mot, al)))
    return seq


def dedup(seq):
    seen, out, dups = set(), [], []
    for c, m in seq:
        if c in seen:
            dups.append(c)
        else:
            seen.add(c)
            out.append((c, m))
    return out, dups


def cstr(s):
    return '"' + s + '"'   # source literals are already valid C (no escapes present; checked)


def emit_cpp(path, src_path, src_sha, head, units, motors, alarms):
    L = []
    L.append('// ===========================================================================')
    L.append('//  AlarmCodeCatalog.cpp -- GENERATED by tools/gen_alarmcode_catalog.py. DO NOT EDIT BY HAND.')
    L.append('//  AI(W906-CSVONLY) 20260926: static alarm-code catalog of the BCB6 MDB Updater (SQLiteUpdater.exe).')
    L.append('//  source : %s' % src_path)
    L.append('//  sha256 : %s' % src_sha)
    L.append('//  Order and duplicate handling follow golden CreateTableAlarmList() / DoInsertAlarmCode();')
    L.append('//  see AlarmCodeCatalog.h and the generator header.')
    L.append('// ===========================================================================')
    L.append('#include "AlarmCodeCatalog.h"')
    L.append('')
    L.append('namespace {')
    L.append('')
    L.append('struct Entry { const char* code; const char* msg; };')
    L.append('')
    L.append('const Entry kHead[] = {')
    for c, m, n in head:
        L.append('    { %s, %s },   // CreatDatabase.cpp:%d' % (cstr(c), cstr(m), n))
    L.append('};')
    for fn, entries in units:
        L.append('')
        L.append('// %s' % fn)
        if entries:
            L.append('const Entry k_%s[] = {' % fn)
            for c, m, n in entries:
                L.append('    { %s, %s },   // :%d' % (cstr(c), cstr(m), n))
            L.append('};')
    L.append('')
    L.append('// CreateTableAlarmList_024_Motor(): MotorList x AlarmList, code WAR24%03d%d, message "%s -- %s"')
    L.append('const char* const kMotorNames[] = {')
    for mname in motors:
        L.append('    %s,' % cstr(mname))
    L.append('};')
    L.append('const char* const kMotorAlarms[] = {')
    for a in alarms:
        L.append('    %s,' % cstr(a))
    L.append('};')
    L.append('')
    L.append('template <std::size_t N>')
    L.append('void Emit(const Entry (&t)[N], AlarmCodeSink sink, void* ctx) {')
    L.append('    for (std::size_t i = 0; i < N; ++i) sink(t[i].code, t[i].msg, ctx);')
    L.append('}')
    L.append('')
    L.append('}  // namespace')
    L.append('')
    L.append('void AlarmCodeCatalog_Replay(AlarmCodeSink sink, void* ctx)')
    L.append('{')
    L.append('    Emit(kHead, sink, ctx);')
    for fn, entries in units:
        if entries:
            L.append('    Emit(k_%s, sink, ctx);' % fn)
        if fn.endswith('_024_Motor'):
            L.append('    for (std::size_t i = 0; i < sizeof(kMotorNames) / sizeof(kMotorNames[0]); ++i)')
            L.append('    {')
            L.append('        for (std::size_t j = 0; j < sizeof(kMotorAlarms) / sizeof(kMotorAlarms[0]); ++j)')
            L.append('        {')
            L.append('            char code[16];')
            L.append('            std::snprintf(code, sizeof(code), "WAR24%03u%u", (unsigned)i, (unsigned)j);')
            L.append('            std::string msg = std::string(kMotorNames[i]) + " -- " + kMotorAlarms[j];')
            L.append('            sink(code, msg.c_str(), ctx);')
            L.append('        }')
            L.append('    }')
    L.append('}')
    L.append('')
    L.append('std::size_t AlarmCodeCatalog_RawCount()')
    L.append('{')
    raw = len(head) + sum(len(e) for _, e in units) + len(motors) * len(alarms)
    L.append('    return %d;   // insertions before dedup (golden sequence length)' % raw)
    L.append('}')
    L.append('')
    io.open(path, 'w', encoding='utf-8', newline='').write('\r\n'.join(L))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default=DEFAULT_SRC)
    ap.add_argument('--check', default=None)
    ap.add_argument('--no-write', action='store_true')
    a = ap.parse_args()
    raw_bytes = open(a.src, 'rb').read()
    sha = hashlib.sha256(raw_bytes).hexdigest()
    text = raw_bytes.decode('cp950')
    head, units, motors, alarms = parse(text)
    seq = replay(head, units, motors, alarms)
    out, dups = dedup(seq)
    print('units %d, literal %d (+head %d), motors %d x alarms %d, raw %d, unique %d, duplicates %d'
          % (len(units), sum(len(e) for _, e in units), len(head), len(motors), len(alarms), len(seq), len(out), len(dups)))
    if dups:
        print('duplicates (first kept, as golden):', ', '.join(dups))
    if not a.no_write:
        emit_cpp(os.path.join(TREE, 'AlarmCodeCatalog.cpp'), a.src, sha, head, units, motors, alarms)
        ship = os.path.join(TREE, 'ship', 'Error')
        os.makedirs(ship, exist_ok=True)
        data = ''.join('%s=%s\r\n' % (c, m) for c, m in out)
        open(os.path.join(ship, 'AlarmCodeList.txt'), 'wb').write(data.encode('ascii'))
        print('wrote AlarmCodeCatalog.cpp and ship/Error/AlarmCodeList.txt')
    if a.check:
        real = open(a.check, 'rb').read().decode('ascii').split('\r\n')
        if real and real[-1] == '':
            real.pop()
        gen = ['%s=%s' % (c, m) for c, m in out]
        k = 0
        while k < min(len(gen), len(real)) and gen[k] == real[k]:
            k += 1
        rset, gset = set(real), set(gen)
        print('check vs %s: real %d lines, generated %d; common prefix %d; generated lines missing from real %d; real lines not generated %d'
              % (a.check, len(real), len(gen), k, len(gset - rset), len(rset - gset)))
        if k < len(gen):
            print('  first difference at line %d:' % (k + 1))
            print('    generated: %s' % (gen[k] if k < len(gen) else '<eof>'))
            print('    real     : %s' % (real[k] if k < len(real) else '<eof>'))


if __name__ == '__main__':
    main()
