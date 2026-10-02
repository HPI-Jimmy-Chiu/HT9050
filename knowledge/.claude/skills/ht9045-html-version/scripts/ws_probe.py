# -*- coding: utf-8 -*-
# Steven 20260915
# ----------------------------------------------------------------------
# 手工 WebSocket 客戶端，比對指令前後 SHA256 驗證 dryRun。
# 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
# ----------------------------------------------------------------------

"""ws_probe.py -- 最小 WebSocket 客戶端，驗證 dryRun 是否真的不寫。

AI(W906-FW-DRYRUN) 20260915。

用途：送一筆 recipe.doc.put（dryRun 在 value 裡），比對目標檔案的 SHA256
在指令前後是否相同。相同 = dryRun 生效；不同 = 仍在寫。

沒有第三方相依，手工做 RFC6455 握手與 frame。
"""
import base64, hashlib, json, os, socket, struct, sys, time

HOST, PORT, PATH = '127.0.0.1', 8045, '/ht9045'


def sha(p):
    h = hashlib.sha256()
    with open(p, 'rb') as f:
        for b in iter(lambda: f.read(65536), b''):
            h.update(b)
    return h.hexdigest()


def connect():
    s = socket.create_connection((HOST, PORT), timeout=10)
    key = base64.b64encode(os.urandom(16)).decode()
    req = ('GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\n'
           'Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n'
           'Sec-WebSocket-Version: 13\r\n\r\n' % (PATH, HOST, PORT, key))
    s.sendall(req.encode())
    buf = b''
    while b'\r\n\r\n' not in buf:
        buf += s.recv(4096)
    if b'101' not in buf.split(b'\r\n')[0]:
        raise RuntimeError('handshake failed: ' + buf.split(b'\r\n')[0].decode())
    return s


def send(s, obj):
    payload = json.dumps(obj).encode('utf-8')
    mask = os.urandom(4)
    masked = bytes(payload[i] ^ mask[i % 4] for i in range(len(payload)))
    n = len(payload)
    hdr = b'\x81'
    if n < 126:
        hdr += struct.pack('!B', 0x80 | n)
    elif n < 65536:
        hdr += struct.pack('!BH', 0x80 | 126, n)
    else:
        hdr += struct.pack('!BQ', 0x80 | 127, n)
    s.sendall(hdr + mask + masked)


def recv(s, want_id, timeout=10):
    """讀到 id 相符的 ack 為止；其餘訊框（tag 推播）忽略。"""
    s.settimeout(timeout)
    end = time.time() + timeout
    buf = b''
    while time.time() < end:
        try:
            chunk = s.recv(65536)
        except socket.timeout:
            break
        if not chunk:
            break
        buf += chunk
        while True:
            if len(buf) < 2:
                break
            b1, b2 = buf[0], buf[1]
            ln = b2 & 0x7F
            off = 2
            if ln == 126:
                if len(buf) < 4: break
                ln = struct.unpack('!H', buf[2:4])[0]; off = 4
            elif ln == 127:
                if len(buf) < 10: break
                ln = struct.unpack('!Q', buf[2:10])[0]; off = 10
            if len(buf) < off + ln:
                break
            data = buf[off:off + ln]
            buf = buf[off + ln:]
            if (b1 & 0x0F) != 1:
                continue
            try:
                m = json.loads(data.decode('utf-8'))
            except Exception:
                continue
            if m.get('id') == want_id:
                return m
    return None


def main():
    target = sys.argv[1] if len(sys.argv) > 1 else None
    doc = sys.argv[2] if len(sys.argv) > 2 else 'contact'
    sec = sys.argv[3] if len(sys.argv) > 3 else 'Test Arm1'
    key = sys.argv[4] if len(sys.argv) > 4 else 'Pick Up'
    val = sys.argv[5] if len(sys.argv) > 5 else '99.99'

    before = sha(target) if target else None
    s = connect()
    print('handshake OK')
    send(s, {'type': 'cmd', 'id': 1, 'cmd': 'control.acquire'})
    print('acquire ack:', recv(s, 1))

    if os.environ.get('SYS'):
        # system.file.put：tag=檔名，value={"sections":{...},"dryRun":bool}
        payload = {'sections': {sec: {key: {'raw': val}}},
                   'dryRun': not os.environ.get('REAL_WRITE')}
        send(s, {'type': 'cmd', 'id': 2, 'cmd': 'system.file.put',
                 'tag': doc, 'value': json.dumps(payload)})
    elif os.environ.get('OLD_SHAPE'):
        # 修正前的寫法：dryRun 是 value 的兄弟欄位，不在 value 裡面
        payload = {'sections': {sec: {key: {'raw': val}}}}
        send(s, {'type': 'cmd', 'id': 2, 'cmd': 'recipe.doc.put',
                 'tag': doc, 'value': json.dumps(payload), 'dryRun': True})
    else:
        payload = {'sections': {sec: {key: {'raw': val}}},
                   'dryRun': not os.environ.get('REAL_WRITE')}
        send(s, {'type': 'cmd', 'id': 2, 'cmd': 'recipe.doc.put',
                 'tag': doc, 'value': json.dumps(payload)})
    ack = recv(s, 2)
    print('put(dryRun) ack:', ack)

    send(s, {'type': 'cmd', 'id': 3, 'cmd': 'control.release'})
    recv(s, 3, 5)
    s.close()

    if target:
        after = sha(target)
        print('sha before:', before)
        print('sha after :', after)
        print('VERDICT:', 'UNCHANGED (dryRun works)' if before == after
              else '*** FILE CHANGED -- dryRun DID NOT take effect ***')


main()
