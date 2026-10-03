import json, sys, collections

FLAG = '-fexcess-precision=fast'
for path in sys.argv[1:]:
    db = json.load(open(path, encoding='utf-8'))
    n = len(db)
    files = [e['file'] for e in db]
    uniq_files = len(set(f.lower() for f in files))
    outs = [e.get('output', '') for e in db]
    uniq_outs = len(set(o.lower() for o in outs))
    kinds = collections.Counter()
    with_flag = 0
    flag_count_hist = collections.Counter()
    for e in db:
        cmd = e['command']
        exe = cmd.split()[0].lower()
        kinds[exe] += 1
        c = cmd.split().count(FLAG)
        flag_count_hist[c] += 1
        if c:
            with_flag += 1
    print(path)
    print('  entries', n, 'unique files', uniq_files, 'unique outputs', uniq_outs)
    print('  compilers', dict(kinds))
    print('  flag occurrences per command', dict(flag_count_hist))
    ex = db[0]['command']
    print('  example:', ex[:400])
    # show a C entry if any
    for e in db:
        if e['command'].split()[0].lower().endswith('gcc.exe'):
            print('  C example:', e['command'][:300])
            break
