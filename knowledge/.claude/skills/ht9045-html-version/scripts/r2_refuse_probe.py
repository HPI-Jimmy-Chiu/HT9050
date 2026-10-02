# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 新檔。驗證審查 R2：apply 階段有 notFound 時，ack 必須 ok:false、changed=0，且檔案不動。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""r2_refuse_probe.py -- 「一好一壞」混合送 apply，證明 all-or-nothing 且 ack 不假裝成功。

需要帶 --allow-system-write 的伺服器（WB_PORT 指定埠，預設 8045）。
每個目標：先記 SHA256 → 送 {好鍵改值, 壞鍵} dryRun:false → 期望 ok:false、
error 含 'refused'、changed/added=0 → SHA256 不變。
"""
import hashlib, io, json, os, sys, urllib.request

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
PORT = int(os.environ.get('WB_PORT', '8045'))
BASE = 'http://127.0.0.1:%d/api/system/' % PORT
_w = {}
with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ws_probe.py'), encoding='utf-8') as fh:
    exec(compile(fh.read().replace('\nmain()', '\n#'), 'ws_probe.py', 'exec'), _w)
_w['PORT'] = PORT


def sha(p):
    return hashlib.sha256(open(p, 'rb').read()).hexdigest()


def main():
    s = _w['connect'](); _w['send'](s, {'type': 'cmd', 'id': 1, 'cmd': 'control.acquire'}); _w['recv'](s, 1)
    rc = 0
    cases = []
    # ini：gerneral 一好一壞
    g = json.load(urllib.request.urlopen(BASE + 'gerneral'))
    cur = g['sections']['System']['AUTO_EMPTY_COLOR']['raw']
    cases.append(('gerneral ini', 'system.file.put', 'gerneral', g['path'],
                  {'sections': {'System': {'AUTO_EMPTY_COLOR': {'raw': '9' if cur != '9' else '8'},
                                           '__NO_SUCH_KEY__': {'raw': '1'}}}}, 'changed'))
    # csv 格子：motTable 一好一壞
    m = json.load(urllib.request.urlopen(BASE + 'motTable'))
    k = m['rows'][0][m['keyColumn']]
    cases.append(('motTable cells', 'system.file.put', 'motTable', m['path'],
                  {'rows': {k: {'Range': '777'}, 'NO_SUCH_ROW': {'Range': '1'}}}, 'changed'))
    # csv 整列：一好一壞
    cases.append(('motTable rows', 'system.csv.rows', 'motTable', m['path'],
                  {'add': [{'Motorname': 'M98'}], 'delete': ['NO_SUCH_KEY_X']}, 'added'))
    i = 10
    for name, cmd, tag, path, body, cntkey in cases:
        before = sha(path)
        i += 1
        _w['send'](s, {'type': 'cmd', 'id': i, 'cmd': cmd, 'tag': tag, 'value': json.dumps(dict(body, dryRun=False))})
        a = _w['recv'](s, i, timeout=20) or {}
        after = sha(path)
        ok = (a.get('ok') is False) and ('refused' in str(a.get('error', ''))) and after == before
        rc |= 0 if ok else 1
        print('%-16s ok=%-5s error=%r  %s=%s  檔案不變=%s  %s'
              % (name, a.get('ok'), a.get('error'), cntkey, a.get(cntkey), after == before, 'PASS' if ok else 'FAIL'))
    sys.exit(rc)


main()
