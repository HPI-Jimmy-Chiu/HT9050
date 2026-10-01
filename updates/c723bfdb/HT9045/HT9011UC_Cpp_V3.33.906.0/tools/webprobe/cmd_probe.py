# =============================================================================
#  tools/webprobe/cmd_probe.py -- FW-W1 command-channel e2e probe.
#
#  AI(W906-FW-W1) 20260819: new file. Exercises the browser->ws->CommandQueue->
#  tick-drain->CompleteCommand->ack round trip that wb_serve --allow-cmd wires
#  (docs/WEBBRIDGE_WRITEPATH_DESIGN.md section 6, wave FW-W1).
#
#  Modes:
#    default            : against `wb_serve --allow-cmd` --
#                           sys.ping        -> ack ok:true
#                           no.such.cmd     -> ack ok:false ("unknown cmd...")
#    --expect-readonly  : against plain read-only wb_serve --
#                           sys.ping        -> ack ok:false ("read-only")
#
#  Exit 0 = every expectation met; nonzero = which stage failed (see prints).
#  Client frames are MASKED per RFC6455 (server drops unmasked client frames).
# =============================================================================
import argparse, base64, hashlib, json, os, socket, struct, time

WS_GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'


def ws_handshake(host, port, path, deadline):
    key = base64.b64encode(os.urandom(16)).decode()
    expect = base64.b64encode(hashlib.sha1((key + WS_GUID).encode()).digest()).decode()
    sock = socket.create_connection((host, port), timeout=5.0)
    sock.sendall(('GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\n'
                  'Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n'
                  'Sec-WebSocket-Version: 13\r\n\r\n'
                  % (path, host, port, key)).encode())
    sock.settimeout(0.5)
    buf = b''
    while b'\r\n\r\n' not in buf:
        if time.monotonic() > deadline:
            raise RuntimeError('handshake: no response before deadline')
        try:
            c = sock.recv(4096)
        except socket.timeout:
            continue
        if not c:
            raise RuntimeError('handshake: connection closed')
        buf += c
    head, rest = buf.split(b'\r\n\r\n', 1)
    if expect.encode() not in head:
        raise RuntimeError('handshake: Sec-WebSocket-Accept mismatch')
    return sock, rest


def send_text(sock, payload):
    data = payload.encode('utf-8')
    mask = os.urandom(4)
    header = bytearray([0x81])                      # FIN + text
    n = len(data)
    if n < 126:
        header.append(0x80 | n)                     # MASK bit + len
    elif n < 65536:
        header.append(0x80 | 126)
        header += struct.pack('>H', n)
    else:
        header.append(0x80 | 127)
        header += struct.pack('>Q', n)
    header += mask
    masked = bytes(b ^ mask[i % 4] for i, b in enumerate(data))
    sock.sendall(bytes(header) + masked)


def read_frames(sock, leftover, deadline):
    """Generator of (opcode, payload) server frames (server frames unmasked)."""
    buf = leftover
    while True:
        while True:
            if len(buf) >= 2:
                b0, b1 = buf[0], buf[1]
                ln = b1 & 0x7F
                off = 2
                if ln == 126:
                    if len(buf) < 4: pass
                    else:
                        ln = struct.unpack('>H', buf[2:4])[0]; off = 4
                elif ln == 127:
                    if len(buf) < 10: pass
                    else:
                        ln = struct.unpack('>Q', buf[2:10])[0]; off = 10
                if ln < 126 or off > 2:
                    if len(buf) >= off + ln:
                        payload = buf[off:off + ln]
                        buf = buf[off + ln:]
                        yield (b0 & 0x0F, payload)
                        continue
            if time.monotonic() > deadline:
                return
            try:
                c = sock.recv(65536)
            except socket.timeout:
                continue
            if not c:
                return
            buf += c


def wait_ack(sock, leftover, want_id, deadline):
    for op, payload in read_frames(sock, leftover, deadline):
        if op != 1:
            continue
        try:
            j = json.loads(payload.decode('utf-8', 'replace'))
        except ValueError:
            continue
        if j.get('type') == 'ack' and j.get('id') == want_id:
            return j
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--host', default='127.0.0.1')
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--path', default='/ht9045')
    ap.add_argument('--seconds', type=float, default=10.0)
    ap.add_argument('--expect-readonly', action='store_true')
    # AI(W906-FW-W2) 20260819: auth round trip -- "USER:GOODPW:BADPW". Server
    # must run --allow-cmd with W906_PWBOOK_PATH pointing at a scratch book
    # containing USER's entry. Sequence: bad pw -> ok:false("bad credentials"),
    # good pw -> ok:true, auth.logout -> ok:true.
    ap.add_argument('--auth', default=None)
    # AI(W906-FW-W3) 20260819: two-connection mutual-exclusion scenario:
    # A acquires -> B acquire fails("control-held") -> B ping fails
    # ("not-operator") -> A releases -> B acquires ok.
    ap.add_argument('--control', action='store_true')
    # AI(W906-FW-W4) 20260819: counter.clear round trip -- loadingCount family
    # accepted (default Security_new.def authorization is 1), bogus family
    # refused. The post-clear state observable (SendCT[0]/iIndexCount == 0)
    # is printed by wb_serve itself; the gate script greps the serve log.
    ap.add_argument('--counter', action='store_true')
    # AI(W906-FW-W5a) 20260819: display-only modal path -- sys.echoModal drives
    # the real ShowMyMessage, whose hook broadcasts {"type":"modal",...}; the
    # probe must see BOTH the ack and a modal frame carrying the echoed text.
    ap.add_argument('--modal', action='store_true')
    # AI(W906-FW-W5b) 20260819: ANSWER-carrying path -- sys.echoErrorModal
    # blocks in the wb_serve pump until modal.answer arrives. Sequence proves:
    # query frame broadcast (options decode the K mask), un-offered answer
    # refused, OTHER commands refused "modal-pending" while the query is up
    # (VCL modality equivalent), offered answer accepted, original command
    # acked, and a stray late answer refused "no query pending".
    ap.add_argument('--query', action='store_true')
    # AI(W906-Q30-KZERO) 20260923: kcode==0 回歸探針。`sys.echoErrorModal` 帶
    # value=0 會走 ShowErrorMessage(code, 0, ...)。在 56b00af 之前，
    # ForwardShowErrorMessage 的等待迴圈唯一出口是
    #   if (k != 0 && (k & kcode) != 0)
    # kcode==0 時 (k & 0) 恆為 0 ⇒ **沒有任何出口**，那一支永遠不返回。
    # 而 wb_serve 是單執行緒：它一卡住，PumpTick()->MainProc() 連帶停擺，
    # 其後每個網頁命令都拿不到 ack（症狀是 no ack within 15000ms，是逾時不是拒絕）。
    # 這支探針證明三件事：(1) kcode==0 立刻返回 (2) 迴圈還活著
    # (3) 沒有留下待答查詢（PostQuery 被正確略過，否則瀏覽器會收到一個關不掉的框）。
    ap.add_argument('--query-note', action='store_true')
    args = ap.parse_args()

    if args.query_note:
        deadline = time.monotonic() + args.seconds
        sock, lo = ws_handshake(args.host, args.port, args.path, deadline)
        print('ws handshake ok')

        send_text(sock, json.dumps({'type': 'cmd', 'id': 700, 'cmd': 'control.takeover'}))  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
        a = wait_ack(sock, lo, 700, deadline)
        print('acquire: %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('QUERY-NOTE FAIL: acquire expected ok:true'); return 10

        # --- 回歸點：kcode==0 必須立刻返回 ------------------------------------
        t0 = time.monotonic()
        send_text(sock, json.dumps({'type': 'cmd', 'id': 701, 'cmd': 'sys.echoErrorModal',
                                    'tag': 'WAR0000', 'value': 0}))
        a = wait_ack(sock, b'', 701, deadline)
        dt = time.monotonic() - t0
        print('ack(kcode=0): %s   after %.2fs' % (json.dumps(a), dt))
        if a is None:
            print('QUERY-NOTE FAIL: kcode==0 在 %.1fs 內沒有 ack —— 無限等待回來了'
                  % args.seconds)
            print('  看 wb_serve 的 stdout：只有 `-- NOTE, not blocking` 那行才算修好')
            return 10
        if a.get('ok') is not True:
            print('QUERY-NOTE FAIL: kcode==0 expected ok:true'); return 10
        if dt > 5.0:
            print('QUERY-NOTE FAIL: 有 ack 但花了 %.2fs —— 它不該等任何東西' % dt)
            return 10

        # --- 迴圈還活著嗎：ping 必須是 ok:true，不是 modal-pending -------------
        send_text(sock, json.dumps({'type': 'cmd', 'id': 702, 'cmd': 'sys.ping'}))
        a = wait_ack(sock, b'', 702, deadline)
        print('ping(after note): %s' % json.dumps(a))
        if a is None:
            print('QUERY-NOTE FAIL: ping 沒有 ack —— tick 迴圈已經停了'); return 10
        if a.get('ok') is not True:
            if 'modal-pending' in str(a.get('error', '')):
                print('QUERY-NOTE FAIL: 還有待答查詢 —— PostQuery 沒有被略過,')
                print('  瀏覽器會收到一個 kcode==0、生不出按鈕、關不掉的框')
            else:
                print('QUERY-NOTE FAIL: ping expected ok:true')
            return 10

        # --- 不可以留下待答查詢：這時候的 answer 必須被判成 no query pending ----
        send_text(sock, json.dumps({'type': 'cmd', 'id': 703, 'cmd': 'modal.answer',
                                    'tag': '0', 'value': 'RETRY'}))
        a = wait_ack(sock, b'', 703, deadline)
        print('answer(should be stray): %s' % json.dumps(a))
        if a is None or a.get('ok') is not False or 'no query pending' not in str(a.get('error', '')):
            print('QUERY-NOTE FAIL: kcode==0 之後不該有任何待答查詢'); return 10

        print('QUERY-NOTE PROBE PASS: kcode==0 立刻返回、tick 迴圈仍在轉、沒有殘留待答查詢')
        print('  ⓘ 阻塞路徑（kcode!=0）由 --query 驗；兩個要一起跑才算完整。')
        return 0
    if args.query:
        deadline = time.monotonic() + args.seconds
        sock, lo = ws_handshake(args.host, args.port, args.path, deadline)
        print('ws handshake ok')

        send_text(sock, json.dumps({'type': 'cmd', 'id': 600, 'cmd': 'control.takeover'}))
        a = wait_ack(sock, lo, 600, deadline)
        print('acquire: %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('QUERY FAIL: acquire expected ok:true'); return 10

        send_text(sock, json.dumps({'type': 'cmd', 'id': 601, 'cmd': 'sys.echoErrorModal',
                                    'tag': 'WAR0001', 'value': 3}))
        qid = None
        for op, payload in read_frames(sock, b'', deadline):
            if op != 1:
                continue
            try:
                j = json.loads(payload.decode('utf-8', 'replace'))
            except ValueError:
                continue
            if j.get('type') == 'query':
                print('query frame: %s' % json.dumps(j))
                if j.get('options') != ['RETRY', 'SKIP']:
                    print('QUERY FAIL: kcode 3 should decode to [RETRY, SKIP]'); return 10
                qid = j.get('qid')
                break
        if qid is None:
            print('QUERY FAIL: no query frame before deadline'); return 10

        send_text(sock, json.dumps({'type': 'cmd', 'id': 602, 'cmd': 'modal.answer',
                                    'tag': str(qid), 'value': 'CLEAN_OUT'}))
        a = wait_ack(sock, b'', 602, deadline)
        print('answer(unoffered): %s' % json.dumps(a))
        if a is None or a.get('ok') is not False or 'not an offered option' not in str(a.get('error', '')):
            print('QUERY FAIL: CLEAN_OUT (not in mask 3) expected refusal'); return 10

        send_text(sock, json.dumps({'type': 'cmd', 'id': 603, 'cmd': 'sys.ping'}))
        a = wait_ack(sock, b'', 603, deadline)
        print('ping(during modal): %s' % json.dumps(a))
        if a is None or a.get('ok') is not False or 'modal-pending' not in str(a.get('error', '')):
            print('QUERY FAIL: ping during modal expected modal-pending'); return 10

        send_text(sock, json.dumps({'type': 'cmd', 'id': 604, 'cmd': 'modal.answer',
                                    'tag': str(qid), 'value': 'SKIP'}))
        a = wait_ack(sock, b'', 604, deadline)
        print('answer(SKIP): %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('QUERY FAIL: SKIP expected ok:true'); return 10

        a = wait_ack(sock, b'', 601, deadline)
        print('ack(echoErrorModal): %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('QUERY FAIL: original command expected ok:true after answer'); return 10

        send_text(sock, json.dumps({'type': 'cmd', 'id': 605, 'cmd': 'modal.answer',
                                    'tag': str(qid), 'value': 'SKIP'}))
        a = wait_ack(sock, b'', 605, deadline)
        print('answer(stray): %s' % json.dumps(a))
        if a is None or a.get('ok') is not False or 'no query pending' not in str(a.get('error', '')):
            print('QUERY FAIL: stray late answer expected no-query-pending'); return 10

        print('QUERY PROBE PASS: mask decode, refusal, modality, answer, completion, stray all proven')
        return 0

    if args.modal:
        deadline = time.monotonic() + args.seconds
        sock, lo = ws_handshake(args.host, args.port, args.path, deadline)
        print('ws handshake ok')

        send_text(sock, json.dumps({'type': 'cmd', 'id': 500, 'cmd': 'control.takeover'}))
        a = wait_ack(sock, lo, 500, deadline)
        print('acquire: %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('MODAL FAIL: acquire expected ok:true'); return 9

        text = 'modal probe FW-W5a'
        # text rides `value`: the server's tag-name charset rejects spaces
        send_text(sock, json.dumps({'type': 'cmd', 'id': 501, 'cmd': 'sys.echoModal',
                                    'value': text}))
        got_ack = None
        got_modal = None
        for op, payload in read_frames(sock, b'', deadline):
            if op != 1:
                continue
            try:
                j = json.loads(payload.decode('utf-8', 'replace'))
            except ValueError:
                continue
            if j.get('type') == 'ack' and j.get('id') == 501:
                got_ack = j
                print('ack(echoModal): %s' % json.dumps(j))
            elif j.get('type') == 'modal':
                got_modal = j
                print('modal frame: %s' % json.dumps(j))
            if got_ack is not None and got_modal is not None:
                break
        if got_ack is None or got_ack.get('ok') is not True:
            print('MODAL FAIL: echoModal expected ok:true ack'); return 9
        if got_modal is None or text not in str(got_modal.get('text', '')):
            print('MODAL FAIL: expected a modal frame carrying %r' % text); return 9

        print('MODAL PROBE PASS: ack received and modal frame carried the text')
        return 0

    if args.counter:
        deadline = time.monotonic() + args.seconds
        sock, lo = ws_handshake(args.host, args.port, args.path, deadline)
        print('ws handshake ok')

        send_text(sock, json.dumps({'type': 'cmd', 'id': 400, 'cmd': 'control.takeover'}))
        a = wait_ack(sock, lo, 400, deadline)
        print('acquire: %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('COUNTER FAIL: acquire expected ok:true'); return 8

        send_text(sock, json.dumps({'type': 'cmd', 'id': 401, 'cmd': 'counter.clear',
                                    'tag': 'loadingCount'}))
        a = wait_ack(sock, b'', 401, deadline)
        print('clear(loadingCount): %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('COUNTER FAIL: loadingCount expected ok:true'); return 8

        send_text(sock, json.dumps({'type': 'cmd', 'id': 402, 'cmd': 'counter.clear',
                                    'tag': 'noSuchFamily'}))
        a = wait_ack(sock, b'', 402, deadline)
        print('clear(bogus): %s' % json.dumps(a))
        if a is None or a.get('ok') is not False or 'unknown counter family' not in str(a.get('error', '')):
            print('COUNTER FAIL: bogus family expected unknown-counter-family refusal'); return 8

        print('COUNTER PROBE PASS: loadingCount cleared, bogus family refused')
        return 0

    if args.control:
        deadline = time.monotonic() + args.seconds
        sockA, loA = ws_handshake(args.host, args.port, args.path, deadline)
        sockB, loB = ws_handshake(args.host, args.port, args.path, deadline)
        print('ws handshakes ok (A, B)')

        send_text(sockA, json.dumps({'type': 'cmd', 'id': 301, 'cmd': 'control.takeover'}))
        a = wait_ack(sockA, loA, 301, deadline)
        print('A acquire: %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('CONTROL FAIL: A acquire expected ok:true'); return 7

        send_text(sockB, json.dumps({'type': 'cmd', 'id': 302, 'cmd': 'control.acquire'}))  # AI(W906-SCREEN-TOKEN) 20261001: stays control.acquire on purpose -- this checks that the server REFUSES an acquire while A holds the token (control-held); takeover would always win
        b = wait_ack(sockB, loB, 302, deadline)
        print('B acquire(held): %s' % json.dumps(b))
        if b is None or b.get('ok') is not False or 'control-held' not in str(b.get('error', '')):
            print('CONTROL FAIL: B acquire expected control-held'); return 7

        send_text(sockB, json.dumps({'type': 'cmd', 'id': 303, 'cmd': 'sys.ping'}))
        b = wait_ack(sockB, b'', 303, deadline)
        print('B ping(not-op): %s' % json.dumps(b))
        if b is None or b.get('ok') is not False or 'not-operator' not in str(b.get('error', '')):
            print('CONTROL FAIL: B ping expected not-operator'); return 7

        send_text(sockA, json.dumps({'type': 'cmd', 'id': 304, 'cmd': 'control.release'}))
        a = wait_ack(sockA, b'', 304, deadline)
        print('A release: %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('CONTROL FAIL: A release expected ok:true'); return 7

        send_text(sockB, json.dumps({'type': 'cmd', 'id': 305, 'cmd': 'control.acquire'}))  # AI(W906-SCREEN-TOKEN) 20261001: stays control.acquire on purpose -- this checks that an acquire works once nobody holds the token
        b = wait_ack(sockB, b'', 305, deadline)
        print('B acquire(after release): %s' % json.dumps(b))
        if b is None or b.get('ok') is not True:
            print('CONTROL FAIL: B acquire after release expected ok:true'); return 7

        print('CONTROL PROBE PASS: exclusivity, refusal, release handover all proven')
        return 0

    deadline = time.monotonic() + args.seconds
    try:
        sock, leftover = ws_handshake(args.host, args.port, args.path, deadline)
    except (OSError, RuntimeError) as e:
        print('HANDSHAKE FAIL: %s' % e)
        return 2
    print('ws handshake ok')

    # --- 0. acquire the control token (FW-W3: every non-auth/control cmd
    #        needs it; skipped in --expect-readonly where cmds die earlier) ---
    if not args.expect_readonly:
        send_text(sock, json.dumps({'type': 'cmd', 'id': 100, 'cmd': 'control.takeover'}))
        acq = wait_ack(sock, leftover, 100, deadline)
        leftover = b''
        if acq is None or acq.get('ok') is not True:
            print('CONTROL FAIL: control.takeover expected ok:true, got %s' % json.dumps(acq))
            return 6
        print('ack(control.takeover): %s' % json.dumps(acq))

    # --- 1. sys.ping ---------------------------------------------------------
    send_text(sock, json.dumps({'type': 'cmd', 'id': 101, 'cmd': 'sys.ping'}))
    ack = wait_ack(sock, leftover, 101, deadline)
    leftover = b''
    if ack is None:
        print('CMD FAIL: no ack for sys.ping before deadline')
        return 3
    print('ack(sys.ping): %s' % json.dumps(ack))

    if args.expect_readonly:
        if ack.get('ok') is False and 'read-only' in str(ack.get('error', '')):
            print('READONLY PROBE PASS: cmd correctly refused')
            return 0
        print('READONLY PROBE FAIL: expected ok:false with read-only error')
        return 4

    if ack.get('ok') is not True:
        print('CMD FAIL: sys.ping expected ok:true')
        return 4

    # --- auth mode: bad pw -> fail, good pw -> ok, logout -> ok --------------
    if args.auth:
        user, goodpw, badpw = args.auth.split(':', 2)

        send_text(sock, json.dumps({'type': 'cmd', 'id': 201, 'cmd': 'auth.login',
                                    'tag': user, 'value': badpw}))
        a = wait_ack(sock, b'', 201, deadline)
        print('ack(login bad): %s' % json.dumps(a))
        if a is None or a.get('ok') is not False or 'bad credentials' not in str(a.get('error', '')):
            print('AUTH FAIL: bad password expected ok:false("bad credentials")')
            return 5

        send_text(sock, json.dumps({'type': 'cmd', 'id': 202, 'cmd': 'auth.login',
                                    'tag': user, 'value': goodpw}))
        a = wait_ack(sock, b'', 202, deadline)
        print('ack(login good): %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('AUTH FAIL: good password expected ok:true')
            return 5

        send_text(sock, json.dumps({'type': 'cmd', 'id': 203, 'cmd': 'auth.logout'}))
        a = wait_ack(sock, b'', 203, deadline)
        print('ack(logout): %s' % json.dumps(a))
        if a is None or a.get('ok') is not True:
            print('AUTH FAIL: logout expected ok:true')
            return 5

        print('AUTH PROBE PASS: bad refused, good accepted, logout ok')
        return 0

    # --- 2. unknown command must ack ok:false --------------------------------
    send_text(sock, json.dumps({'type': 'cmd', 'id': 102, 'cmd': 'no.such.cmd'}))
    ack2 = wait_ack(sock, leftover, 102, deadline)
    if ack2 is None:
        print('CMD FAIL: no ack for no.such.cmd before deadline')
        return 3
    print('ack(no.such.cmd): %s' % json.dumps(ack2))
    if ack2.get('ok') is not False:
        print('CMD FAIL: unknown cmd expected ok:false')
        return 4

    print('CMD PROBE PASS: round trip proven (ping acked true, unknown acked false)')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
