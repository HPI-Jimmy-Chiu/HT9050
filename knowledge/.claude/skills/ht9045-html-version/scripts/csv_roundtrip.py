# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 新檔。對一個 csv 系統檔做「真寫入 -> 還原」往返，逐位元組比對。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""csv_roundtrip.py -- 證明 system.file.put 對 csv 的真寫入是位元組保真的。

AI(W906-FW-SYSWIRE) 20260916。

dryRun 只能證明「伺服器認得這些格子」；CsvApplyEdits 真寫入時會重組整行
（SplitCsv -> 改一格 -> 用逗號接回 -> 補回原本的行尾），這一步有沒有動到
別的位元組，只有真的寫一次再比對才知道。

流程（伺服器要帶 --allow-system-write）：
  1. 記下檔案 SHA256
  2. 挑一格（預設最後一列、最後一欄 —— 就是 20260916 修掉的那個 '\\n' 邊界）
     寫入一個不同的值，確認 ack changed=1，且檔案「只有那一格」變了
  3. 寫回原值，確認檔案 SHA256 與步驟 1 完全相同
  4. 清掉 CsvApplyEdits 留在 system\\ 的 .bak_ 備份（測試產生的，不是機台的）

用法: py csv_roundtrip.py motTable [rowKey] [column]
"""
import hashlib, io, json, os, sys, glob, urllib.request

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
PORT = int(os.environ.get('WB_PORT', '8045'))          # Steven 20260916（審查 D）：埠可指定
BASE = 'http://127.0.0.1:%d/api/system/' % PORT

_wsp = {}
with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ws_probe.py'),
          encoding='utf-8') as _fh:
    exec(compile(_fh.read().replace('\nmain()', '\n# main() -- disabled'), 'ws_probe.py', 'exec'), _wsp)
_wsp['PORT'] = PORT
connect, send, recv = _wsp['connect'], _wsp['send'], _wsp['recv']


def sha(p):
    h = hashlib.sha256()
    h.update(open(p, 'rb').read())
    return h.hexdigest()


def rows_roundtrip(f):
    """Steven 20260916：整列新增 -> 刪除 的真寫入往返（system.csv.rows）。

    期望：新增後檔案剛好多一行（在檔尾、用檔案自己的行尾）、其餘位元組不變；
    刪除後 SHA256 與一開始完全相同；測試留下的 .bak_ 清掉。
    """
    d = json.load(urllib.request.urlopen(BASE + f))
    path, cols, kc = d['path'], d['columns'], d['keyColumn']
    # Steven 20260916（審查 R6）：motTable 伺服器端限 Motorname 為 M##，測試鍵改用空著的 M98。
    key = 'M98' if kc == 'Motorname' else 'WBPROBE_TMP'
    assert not any(r.get(kc) == key for r in d['rows']), '測試鍵 %r 已存在，換一個' % key
    print('目標 %s  路徵 %s  識別欄 %s  測試鍵 %r' % (f, path, kc, key))
    before_bytes = open(path, 'rb').read(); before = sha(path)
    baks_before = set(glob.glob(path + '.bak_*'))
    s = connect(); send(s, {'type': 'cmd', 'id': 1, 'cmd': 'control.acquire'}); recv(s, 1)

    def rows(body, i):
        send(s, {'type': 'cmd', 'id': i, 'cmd': 'system.csv.rows', 'tag': f,
                 'value': json.dumps(dict(body, dryRun=False))})
        return recv(s, i, timeout=30) or {}

    rc = 0
    a = rows({'add': [{kc: key, cols[-1]: 'probe'}]}, 20)
    mid = open(path, 'rb').read()
    eol = b'\r\n' if b'\r\n' in before_bytes else b'\n'
    expect_line = (','.join(key if c == kc else ('probe' if c == cols[-1] else '') for c in cols)).encode() + eol
    ok1 = a.get('ok') and a.get('added') == 1 and mid == before_bytes + expect_line
    rc |= 0 if ok1 else 1
    print('新增 ack: ok=%s added=%s notFound=%s' % (a.get('ok'), a.get('added'), a.get('notFound')))
    print('步驟 A  檔案 == 原檔 + 剛好一行  %s' % ('PASS' if ok1 else 'FAIL'))
    if not ok1:
        print('   期望尾行:', expect_line); print('   實際尾端:', mid[len(before_bytes) - 20:][:120])

    a = rows({'delete': [key]}, 21)
    after = sha(path)
    ok2 = a.get('ok') and a.get('deleted') == 1 and after == before
    rc |= 0 if ok2 else 1
    print('刪除 ack: ok=%s deleted=%s notFound=%s' % (a.get('ok'), a.get('deleted'), a.get('notFound')))
    print('步驟 B  SHA256 %s  %s' % ('相同' if after == before else '不同！', 'PASS' if ok2 else 'FAIL'))

    new_baks = sorted(set(glob.glob(path + '.bak_*')) - baks_before)
    for b in new_baks:
        os.remove(b)
    print('步驟 C  清掉測試產生的備份 %d 個' % len(new_baks))
    sys.exit(rc)


def main():
    if len(sys.argv) > 2 and sys.argv[2] == '--rows':      # Steven 20260916
        rows_roundtrip(sys.argv[1])
        return
    f = sys.argv[1] if len(sys.argv) > 1 else 'motTable'
    d = json.load(urllib.request.urlopen(BASE + f))
    path, cols, kc, rows = d['path'], d['columns'], d['keyColumn'], d['rows']
    key = sys.argv[2] if len(sys.argv) > 2 else rows[-1][kc]
    col = sys.argv[3] if len(sys.argv) > 3 else cols[-1]
    row = [r for r in rows if r[kc] == key][0]
    orig = str(row[col])
    new = '7' if orig != '7' else '8'
    print('目標 %s  路徑 %s' % (f, path))
    print('格子 [%s=%s] %s   原值=%r  測試值=%r' % (kc, key, col, orig, new))

    before_bytes = open(path, 'rb').read()
    before = sha(path)
    baks_before = set(glob.glob(path + '.bak_*'))

    s = connect()
    send(s, {'type': 'cmd', 'id': 1, 'cmd': 'control.acquire'}); recv(s, 1)

    def put(v, i):
        send(s, {'type': 'cmd', 'id': i, 'cmd': 'system.file.put', 'tag': f,
                 'value': json.dumps({'rows': {key: {col: v}}, 'dryRun': False})})
        return recv(s, i, timeout=30) or {}

    rc = 0
    a = put(new, 10)
    mid_bytes = open(path, 'rb').read()
    print('寫入 ack: ok=%s changed=%s identical=%s notFound=%s'
          % (a.get('ok'), a.get('changed'), a.get('identical'), a.get('notFound')))
    # 檔案差異必須「只有那一格」：其他每一行位元組相同
    bl, ml = before_bytes.split(b'\n'), mid_bytes.split(b'\n')
    diff_lines = [i for i in range(max(len(bl), len(ml)))
                  if (bl[i] if i < len(bl) else None) != (ml[i] if i < len(ml) else None)]
    # Steven 20260916（審查 D）：不只「一行變動」，那一行還要「只有目標欄不同」。
    cell_ok = False
    if len(diff_lines) == 1:
        bcells = bl[diff_lines[0]].rstrip(b'\r').split(b',')
        mcells = ml[diff_lines[0]].rstrip(b'\r').split(b',')
        ci = cols.index(col)
        cell_ok = (len(bcells) == len(mcells) and
                   all((bcells[i] == mcells[i]) == (i != ci) for i in range(len(bcells))))
    ok1 = a.get('ok') and a.get('changed') == 1 and len(diff_lines) == 1 and cell_ok
    rc |= 0 if ok1 else 1
    print('步驟 2  變動的行數=%d（必須 1）、該行只有目標欄不同=%s  %s'
          % (len(diff_lines), cell_ok, 'PASS' if ok1 else 'FAIL'))
    if diff_lines:
        i = diff_lines[0]
        print('   before:', bl[i][:120]); print('   after :', ml[i][:120])

    a = put(orig, 11)
    after = sha(path)
    ok2 = a.get('ok') and a.get('changed') == 1 and after == before
    rc |= 0 if ok2 else 1
    print('步驟 3  還原 ack changed=%s；SHA256 %s  %s'
          % (a.get('changed'), '相同' if after == before else '不同！', 'PASS' if ok2 else 'FAIL'))
    print('   before=%s' % before[:24]); print('   after =%s' % after[:24])

    # 4. 清掉測試產生的備份
    new_baks = sorted(set(glob.glob(path + '.bak_*')) - baks_before)
    for b in new_baks:
        os.remove(b)
    print('步驟 4  清掉測試產生的備份 %d 個：%s' % (len(new_baks), ', '.join(os.path.basename(b) for b in new_baks)))
    sys.exit(rc)


main()
