# AI(W906-FPFAST) 20261003 -- assemble FP_ORACLE_FINDINGS.md s9 from the draft + fragments and append it
# (the doc is CRLF; every appended line gets CRLF; nothing above the old end changes).
# usage: python fill_doc.py <doc> <draft> KEY=fragfile ...
import sys

CRLF = chr(13) + chr(10)
LF = chr(10)
doc, draft = sys.argv[1], sys.argv[2]
text = open(draft, encoding='utf-8').read()
for kv in sys.argv[3:]:
    k, f = kv.split('=', 1)
    frag = open(f, encoding='utf-8').read().rstrip(LF)
    ph = '@@' + k + '@@'
    assert text.count(ph) == 1, ph
    text = text.replace(ph, frag)
assert '@@' not in text, 'unfilled placeholder'
assert CRLF not in text
old = open(doc, 'rb').read().decode('utf-8')
assert old.endswith(CRLF + CRLF), 'doc does not end with a blank line'
n_old = old.count(CRLF)
add = text.strip(LF).replace(LF, CRLF) + CRLF
new = old + add
assert new.count(LF) == new.count(CRLF), 'bare LF'
assert not any(chr(c) in add for c in (7, 8, 11, 12)), 'control character in the appended text'
open(doc, 'wb').write(new.encode('utf-8'))
print('appended', add.count(CRLF), 'lines; doc', n_old, '->', new.count(CRLF), 'lines')
