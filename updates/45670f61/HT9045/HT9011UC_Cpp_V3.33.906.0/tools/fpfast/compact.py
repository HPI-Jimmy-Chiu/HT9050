# AI(W906-FPFAST) 20261003 -- shrink a proof.py results.jsonl for the repo: drop the per-section name
# lists (they are what makes it 30 MB) and the recorded commands, keep everything proof.py's resume and
# summarize.py need.  Output is gzip.  usage: python compact.py <results.jsonl> <out.jsonl.gz>
import gzip, json, sys

KEEP = ('key', 'i', 'file', 'entries', 'outputs', 'rcA', 'rcB', 'stderr_equal', 'stdout_equal',
        'stderrB_mentions_excess', 'tA', 'tB', 'status', 'shaA', 'shaB', 'sizeA', 'sizeB',
        'bytes_identical', 'dis_diff', 'func_diff', 'con_diff', 'section_order_equal', 'objdump_rc',
        'dis_text_lines', 'errA', 'errB')
n = 0
with gzip.open(sys.argv[2], 'wt', encoding='utf-8') as out:
    for line in open(sys.argv[1], encoding='utf-8'):
        line = line.strip()
        if not line:
            continue
        r = json.loads(line)
        c = {k: r[k] for k in KEEP if k in r}
        c['n_dis_sections'] = len(r.get('dis_sections', []))
        c['n_con_sections'] = len(r.get('con_sections', []))
        out.write(json.dumps(c) + chr(10))
        n += 1
print('rows', n)
