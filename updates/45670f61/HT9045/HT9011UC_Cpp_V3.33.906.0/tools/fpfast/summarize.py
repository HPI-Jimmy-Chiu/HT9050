# AI(W906-FPFAST) 20261003 -- roll up proof.py results (one results.jsonl per config).
import gzip, json, sys, collections

for path in sys.argv[1:]:
    opener = gzip.open if path.endswith('.gz') else open
    rows = [json.loads(l) for l in opener(path, 'rt', encoding='utf-8') if l.strip()]
    keys = set()
    dup = 0
    for r in rows:
        if r['key'] in keys:
            dup += 1
        keys.add(r['key'])
    c = collections.Counter()
    files = set()
    entries = 0
    dis_secs = con_secs = dis_lines = 0
    lang = collections.Counter()
    nondiff = []
    for r in rows:
        files.add(r['file'].lower())
        entries += r['entries']
        exe = r['key'].split('|', 1)[1].split(' ', 1)[0].lower()
        lang['C' if exe.endswith('gcc.exe') else 'C++'] += 1
        c['rc_ok'] += (r['rcA'] == 0 and r['rcB'] == 0)
        c['stderr_equal'] += bool(r.get('stderr_equal'))
        c['stdout_equal'] += bool(r.get('stdout_equal'))
        c['stderrB_mentions_excess'] += bool(r.get('stderrB_mentions_excess'))
        if r['status'] == 'FAILED':
            c['failed'] += 1
            continue
        c['bytes_identical'] += bool(r.get('bytes_identical'))
        nds = r['n_dis_sections'] if 'n_dis_sections' in r else len(r.get('dis_sections', []))
        ncs = r['n_con_sections'] if 'n_con_sections' in r else len(r.get('con_sections', []))
        dis_secs += nds
        con_secs += ncs
        dis_lines += r.get('dis_text_lines', 0)
        nod = nds == 0
        if r['status'] == 'IDENTICAL':
            c['identical'] += 1
        elif not r.get('dis_diff') and not r.get('con_diff') and r.get('bytes_identical') and nod:
            c['identical_no_code'] += 1
        else:
            nondiff.append(r)
    print(path)
    print('  result rows %d (duplicate keys %d), unique compile commands %d, compile entries covered %d, source files %d, lang %s'
          % (len(rows), dup, len(keys), entries, len(files), dict(lang)))
    print('  both compiles rc 0: %d   stderr A==B: %d   stdout A==B: %d   new excess-precision diagnostics: %d   failed: %d'
          % (c['rc_ok'], c['stderr_equal'], c['stdout_equal'], c['stderrB_mentions_excess'], c['failed']))
    print('  byte-identical objects: %d   all sections identical: %d   + no-code objects (byte-identical, 0 disassembly sections): %d'
          % (c['bytes_identical'], c['identical'], c['identical_no_code']))
    print('  disassembly sections compared %d (%d lines), objdump -s sections compared %d' % (dis_secs, dis_lines, con_secs))
    print('  remaining non-identical: %d' % len(nondiff))
    for r in nondiff:
        print('    #%d %s dis_diff=%s con_diff=%s bytes_identical=%s' % (r['i'], r['file'], r.get('dis_diff'), r.get('con_diff'), r.get('bytes_identical')))
