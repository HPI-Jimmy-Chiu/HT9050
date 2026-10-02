# -*- coding: utf-8 -*-
"""連上 wb_serve 抓第一個 snapshot 訊框，列出所有 tag 與值。

AI(W906-FW-TAGDUMP) 20260915。

用途：回答「執行期串流現在到底送什麼過來」。不下任何指令，純觀察。
沿用 ws_probe.py 的手工 RFC6455 握手/解框，不引入第三方相依。
"""
import base64
import io
import json
import os
import socket
import struct
import sys
import time

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

HOST, PORT, PATH = '127.0.0.1', 8045, '/ht9045'


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
    # 握手回應與第一個訊框常在同一個 TCP 區段；殘留位元組必須交回去，
    # 否則 snapshot 會被連同 header 一起丟掉（第一版就是這樣什麼都看不到）。
    return s, buf.split(b'\r\n\r\n', 1)[1]


def frames(s, seconds, buf=b''):
    """在 seconds 秒內把收到的文字訊框全部 yield 出來。"""
    s.settimeout(1.0)
    end = time.time() + seconds
    first = True
    while time.time() < end:
        if first:
            # 預載的 buf（握手同一區段送來的 snapshot）要先解，
            # 不能等到第一次 recv 成功——recv 逾時會 continue 而跳過解析。
            first = False
        else:
            try:
                chunk = s.recv(65536)
            except socket.timeout:
                continue
            if not chunk:
                return
            buf += chunk
        while len(buf) >= 2:
            ln, off = buf[1] & 0x7F, 2
            if ln == 126:
                if len(buf) < 4:
                    break
                ln, off = struct.unpack('!H', buf[2:4])[0], 4
            elif ln == 127:
                if len(buf) < 10:
                    break
                ln, off = struct.unpack('!Q', buf[2:10])[0], 10
            if len(buf) < off + ln:
                break
            op, data, buf = buf[0] & 0x0F, buf[off:off + ln], buf[off + ln:]
            if op == 1:
                try:
                    yield json.loads(data.decode('utf-8'))
                except Exception:
                    pass


def main():
    secs = int(sys.argv[1]) if len(sys.argv) > 1 else 6
    s, leftover = connect()
    print('handshake OK；觀察 %d 秒\n' % secs)
    kinds, snap, patches = {}, None, []
    for m in frames(s, secs, leftover):
        t = m.get('type', '?')
        kinds[t] = kinds.get(t, 0) + 1
        if t == 'snapshot' and snap is None:
            snap = m.get('data', {})
        elif t == 'patch':
            patches.append(m.get('data', {}))
    s.close()

    print('收到的訊框型別:', kinds)
    if snap is None:
        print('沒收到 snapshot')
        return
    print('snapshot 內 tag 數: %d\n' % len(snap))

    def val(v):
        return v.get('value') if isinstance(v, dict) else v

    live = {k: v for k, v in snap.items() if val(v) not in (None, '', '---')}
    dead = [k for k in snap if k not in live]
    print('有值 %d / 無值(---) %d\n' % (len(live), len(dead)))

    print('--- 有值的 ---')
    for k in sorted(live):
        print('  %-38s %s' % (k, json.dumps(val(live[k]), ensure_ascii=False)[:60]))

    print('\n--- 無值的（依前綴分組）---')
    grp = {}
    for k in dead:
        grp.setdefault(k.split('.')[0], []).append(k)
    for p in sorted(grp):
        print('  %-14s %2d 個: %s' % (p, len(grp[p]), ' '.join(sorted(grp[p])[:6])))

    if patches:
        ch = set()
        for p in patches:
            ch |= set(p.keys())
        print('\n--- %d 個 patch 訊框，變動過的 tag %d 個 ---' % (len(patches), len(ch)))
        for k in sorted(ch)[:20]:
            print('  ' + k)
    else:
        print('\n--- 觀察期內沒有 patch 訊框 ---')


main()
