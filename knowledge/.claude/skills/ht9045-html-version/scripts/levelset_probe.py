# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 驗收 /api/system/levelset 與 system.levels.put 的讀寫語意。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""levelset_probe.py -- 二進位投影通路的驗收探針。

AI(W906-FW-LEVELSET) 20260916。

為什麼要自己重新解碼一次檔案：
「寫入成功」的證據不能只看 ack 加重讀。20260916 早上踩過一次 —— 重讀跟寫入
若指向同一個假目標（當時是 --dry 的暫存副本），永遠一致。所以這支探針每一項
都拿**磁碟上實體檔的 SHA256 與自己獨立解出來的 int32**當證據，不信 ack。

探針也必須先證明自己有能力發現問題，否則會假 PASS：
  * T2 把 API 回的 values 跟自己解碼的結果逐格比對（API 若回假資料會抓到）；
  * T5/T6/T7 是三種非法輸入，期望「拒寫且檔案不動」——若伺服器其實寫了，
    SHA256 會變，測試失敗；
  * T9 檢查 --allow-system-write 閘門本身（沒帶旗標時 apply 必須被拒）。

用法：
    py -3 levelset_probe.py            # 需要 wb_serve 已在 8045 且帶
                                       # --allow-cmd --allow-system-write
    py -3 levelset_probe.py --gate     # 針對「沒帶 --allow-system-write」的
                                       # 伺服器，只跑 T9
"""
import base64
import hashlib
import io
import json
import os
import socket
import struct
import sys
import time
import urllib.request

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

HOST, PORT, PATH = '127.0.0.1', 8045, '/ht9045'
# 機台上常常已經有一支交付包的 wb_serve 佔著 8045；測試用的伺服器換埠時
# 用 --port 指過來，不要去砍別人正在跑的那支。
for _i, _a in enumerate(sys.argv):
    if _a == '--port' and _i + 1 < len(sys.argv):
        PORT = int(sys.argv[_i + 1])
TARGET = r'D:\HT9045\system\levelset.dat'
COUNT = 256

_fails = []


def check(name, ok, detail=''):
    print(('  PASS  ' if ok else '  FAIL  ') + name + (('  -- ' + detail) if detail else ''))
    if not ok:
        _fails.append(name)
    return ok


# --------------------------------------------------------------------------
#  獨立的檔案觀測：不經過伺服器
# --------------------------------------------------------------------------
def file_sha():
    with open(TARGET, 'rb') as f:
        return hashlib.sha256(f.read()).hexdigest()


def file_values():
    """自己解一次小端 int32，作為 API 回傳值的對照組。"""
    with open(TARGET, 'rb') as f:
        raw = f.read()
    if len(raw) != COUNT * 4:
        raise RuntimeError('unexpected size %d' % len(raw))
    return list(struct.unpack('<%di' % COUNT, raw))


# --------------------------------------------------------------------------
#  HTTP
# --------------------------------------------------------------------------
def get_json(url):
    with urllib.request.urlopen('http://%s:%d%s' % (HOST, PORT, url), timeout=10) as r:
        return json.loads(r.read().decode('utf-8'))


# --------------------------------------------------------------------------
#  WebSocket（手工 RFC6455，沿用 ws_probe.py 的做法，不引第三方相依）
# --------------------------------------------------------------------------
class Ws(object):
    def __init__(self):
        self.s = socket.create_connection((HOST, PORT), timeout=10)
        key = base64.b64encode(os.urandom(16)).decode()
        req = ('GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\n'
               'Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n'
               'Sec-WebSocket-Version: 13\r\n\r\n' % (PATH, HOST, PORT, key))
        self.s.sendall(req.encode())
        buf = b''
        while b'\r\n\r\n' not in buf:
            buf += self.s.recv(4096)
        if b'101' not in buf.split(b'\r\n')[0]:
            raise RuntimeError('handshake failed')
        # 握手回應與第一個訊框常在同一個 TCP 區段，殘留位元組要留著
        self.buf = buf.split(b'\r\n\r\n', 1)[1]
        self.nid = 1

    def send(self, obj):
        payload = json.dumps(obj).encode('utf-8')
        mask = os.urandom(4)
        n = len(payload)
        hdr = b'\x81'
        if n < 126:
            hdr += bytes([0x80 | n])
        elif n < 65536:
            hdr += bytes([0x80 | 126]) + struct.pack('>H', n)
        else:
            hdr += bytes([0x80 | 127]) + struct.pack('>Q', n)
        masked = bytes(payload[i] ^ mask[i % 4] for i in range(n))
        self.s.sendall(hdr + mask + masked)

    def _frame(self, timeout=10):
        end = time.time() + timeout
        while True:
            if len(self.buf) >= 2:
                b0, b1 = self.buf[0], self.buf[1]
                ln = b1 & 0x7F
                off = 2
                if ln == 126:
                    if len(self.buf) < 4:
                        ln = None
                    else:
                        ln = struct.unpack('>H', self.buf[2:4])[0]
                        off = 4
                elif ln == 127:
                    if len(self.buf) < 10:
                        ln = None
                    else:
                        ln = struct.unpack('>Q', self.buf[2:10])[0]
                        off = 10
                if ln is not None and len(self.buf) >= off + ln:
                    data = self.buf[off:off + ln]
                    self.buf = self.buf[off + ln:]
                    if (b0 & 0x0F) == 1:
                        return data.decode('utf-8', 'replace')
                    continue
            if time.time() > end:
                raise RuntimeError('ws read timeout')
            self.s.settimeout(max(0.2, end - time.time()))
            try:
                chunk = self.s.recv(65536)
            except socket.timeout:
                continue
            if not chunk:
                raise RuntimeError('ws closed')
            self.buf += chunk

    def cmd(self, name, tag=None, value=None, timeout=15):
        i = self.nid
        self.nid += 1
        m = {'type': 'cmd', 'id': i, 'cmd': name}
        if tag is not None:
            m['tag'] = tag
        if value is not None:
            m['value'] = value
        self.send(m)
        end = time.time() + timeout
        while time.time() < end:
            msg = json.loads(self._frame(timeout=max(1, end - time.time())))
            if msg.get('type') == 'ack' and msg.get('id') == i:
                return msg
        raise RuntimeError('no ack for ' + name)


def levels_put(ws, values, dry):
    return ws.cmd('system.levels.put', tag='levelset',
                  value=json.dumps({'values': values, 'dryRun': dry}))


def refused_by_us(ack):
    """拒寫必須是「我們的驗證」拒的，不是被單一操作員 token 順手擋掉的。

    第一版探針沒有這層檢查，結果 T5/T6/T7 全部因為 not-operator 而 ok=false，
    被判成 PASS —— 探針對「伺服器其實根本沒跑到驗證」完全無感。這就是假 PASS。
    """
    return (ack.get('ok') is False
            and (ack.get('changed') or 0) == 0
            and 'not-operator' not in str(ack.get('error', ''))
            and 'read-only' not in str(ack.get('error', '')))


# --------------------------------------------------------------------------
def main():
    gate_only = '--gate' in sys.argv

    print('levelset probe -- target %s' % TARGET)
    base_sha = file_sha()
    base_vals = file_values()
    print('  baseline sha256 %s' % base_sha[:16])

    ws = Ws()
    # 單一操作員 token：不先拿，每一筆寫入都會被 not-operator 擋掉，而那會讓
    # 「拒寫」類的測試全部假 PASS（第一版就是這樣）。
    a = ws.cmd('control.acquire')
    if not check('T0 取得 control token', a.get('ok') is True, json.dumps(a)):
        return report()

    if gate_only:
        # T9: 沒有 --allow-system-write 時，dryRun 要過、apply 要被擋，且檔案不動。
        #
        # Steven 20260916：payload 刻意寫**與現值相同**的值。
        # 第一版寫 (現值+1)%4，結果有一次誤把 --gate 指到帶了 --allow-system-write
        # 的伺服器，那筆 apply 真的生效、把 AccessLevel[0] 從 2 改成 3 ——
        # 測試本身變成了一次真寫入。探針的 SHA256 檢查抓到了，但探針不該先製造
        # 它要偵測的損害。寫相同值的話：閘門關著 -> ok:false（要 --allow-system-write）、
        # 閘門開著 -> ok:true 且 identical=1、changed=0，兩種都分得出來，
        # 而且**任何一種情況下磁碟都不會變**。
        same = base_vals[0]
        a = levels_put(ws, {'0': same}, True)
        check('T9a dryRun 在沒有 --allow-system-write 時仍可執行', a.get('ok') is True,
              str(a.get('error', '')))
        b = levels_put(ws, {'0': same}, False)
        gate_closed = (b.get('ok') is False and 'allow-system-write' in str(b.get('error', '')))
        check('T9b apply 在沒有 --allow-system-write 時被拒', gate_closed,
              str(b.get('error', '')) or
              '這台伺服器帶了 --allow-system-write —— --gate 要指向沒帶那個旗標的伺服器')
        check('T9c 檔案未被動過', file_sha() == base_sha)
        return report()

    # ---- T1 清單列出 levelset -------------------------------------------
    idx = get_json('/api/system/')
    row = [f for f in idx.get('files', []) if f.get('name') == 'levelset']
    if check('T1 /api/system/ 清單含 levelset', len(row) == 1):
        r = row[0]
        check('T1b kind=i32 / available / count=256',
              r.get('kind') == 'i32' and r.get('available') is True and r.get('count') == COUNT,
              json.dumps({k: r.get(k) for k in ('kind', 'available', 'count', 'min', 'max')}))

    # ---- T2 讀出來的值要跟自己解碼的一致 ---------------------------------
    doc = get_json('/api/system/levelset')
    vals = doc.get('values')
    check('T2 values 長度 256', isinstance(vals, list) and len(vals) == COUNT,
          'got %r' % (len(vals) if isinstance(vals, list) else type(vals)))
    check('T2b values 與獨立解碼逐格相同', vals == base_vals,
          'first diff at %s' % next((i for i, (a, b) in enumerate(zip(vals or [], base_vals))
                                     if a != b), 'none'))
    check('T2c path 指向實體檔', str(doc.get('path', '')).lower() == TARGET.lower(),
          str(doc.get('path')))

    # 挑一個值域內、且與現值不同的索引來測
    probe_idx = 40
    old = base_vals[probe_idx]
    new = 1 if old != 1 else 2

    # ---- T3 dryRun 不寫 ---------------------------------------------------
    a = levels_put(ws, {str(probe_idx): new}, True)
    check('T3 dryRun ack ok 且 changed=1', a.get('ok') is True and a.get('changed') == 1,
          json.dumps(a))
    check('T3b dryRun 後檔案 SHA256 未變', file_sha() == base_sha)

    # ---- T4 真寫入 --------------------------------------------------------
    a = levels_put(ws, {str(probe_idx): new}, False)
    check('T4 apply ack ok 且 changed=1', a.get('ok') is True and a.get('changed') == 1,
          json.dumps(a))
    after_sha = file_sha()
    check('T4b 檔案 SHA256 有變', after_sha != base_sha)
    bak = a.get('backup', '')
    check('T4c ack 回的 backup 實際存在', bool(bak) and os.path.isfile(bak), str(bak))
    now_vals = file_values()
    check('T4d 只有目標索引變了，其餘 255 格原封不動',
          now_vals[probe_idx] == new and
          all(now_vals[i] == base_vals[i] for i in range(COUNT) if i != probe_idx),
          'idx %d: %d -> %d' % (probe_idx, old, now_vals[probe_idx]))
    check('T4e 重讀 API 反映新值', get_json('/api/system/levelset')['values'][probe_idx] == new)

    # ---- T5/T6/T7 非法輸入一律拒寫且不動檔案 ------------------------------
    for name, payload in (('T5 值域外 (9)', {str(probe_idx): 9}),
                          ('T6 索引越界 (999)', {'999': 1}),
                          ('T7 非數字索引 ("3x")', {'3x': 1})):
        a = levels_put(ws, payload, False)
        check(name + ' 被我們的驗證拒掉 (ok=false, changed=0)',
              refused_by_us(a), json.dumps(a))
        check(name + ' 檔案未被動過', file_sha() == after_sha)

    # ---- T8 還原 ----------------------------------------------------------
    a = levels_put(ws, {str(probe_idx): old}, False)
    check('T8 還原 ack ok', a.get('ok') is True, json.dumps(a))
    check('T8b 還原後 SHA256 回到基準', file_sha() == base_sha,
          '%s vs %s' % (file_sha()[:16], base_sha[:16]))

    return report()


def report():
    print('')
    if _fails:
        print('FAILED %d: %s' % (len(_fails), '; '.join(_fails)))
        return 1
    print('ALL PASS')
    return 0


if __name__ == '__main__':
    sys.exit(main())
