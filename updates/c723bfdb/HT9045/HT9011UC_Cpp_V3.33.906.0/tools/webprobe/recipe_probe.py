# =============================================================================
#  tools/webprobe/recipe_probe.py -- FW-C1 recipe read/write e2e probe.
#
#  AI(W906-FW-C1WIRE) 20260911: new file. Exercises the chain a BROWSER uses to
#  read and edit a recipe document, against a running `wb_serve --allow-cmd`:
#
#      GET  /api/recipe/            -> the active recipe and its documents
#      GET  /api/recipe/<doc>       -> {path, available, sections{...{value,type,raw}}}
#      ws   control.takeover         -> the single-operator token
#      ws   recipe.doc.put (dryRun) -> ack ok, and the file must NOT move
#      ws   recipe.doc.put (apply)  -> ack ok, and the value must come back changed
#      GET  /api/recipe/<doc>       -> the new value, read back over HTTP
#
#  Hand-rolled client, no libraries, same posture as ws_probe.py and
#  cmd_probe.py: if this passes, a browser gets the same bytes.
#
#  ---------------------------------------------------------------------------
#  RUN IT AGAINST --dry. wb_serve --dry scratch-copies the recipe folder and
#  redirects DataPath (wb_serve.cpp:429), so the write lands on the copy and the
#  plant's own recipe is untouched. That protection works only because the route
#  resolves paths through golden's GetRecipePath() instead of building them --
#  a probe pointed at a --real server WILL edit production data.
#
#  The probe therefore REFUSES to run unless the server reports a scratch path,
#  unless --i-mean-it is passed. A safety rail that can be forgotten is not one.
#  ---------------------------------------------------------------------------
#
#  Usage:  python recipe_probe.py [--host 127.0.0.1] [--port 8045]
#                                 [--doc contact] [--i-mean-it]
#  Exit:   0 = every expectation met
#          2 = cannot reach the server
#          3 = an HTTP expectation failed
#          4 = the WebSocket handshake failed
#          5 = a command was rejected or acked false
#          6 = the document did not change when it should have, changed when it
#              should not have, or the Origin gate let the wrong thing through
#          7 = refused: the server is not on a scratch path
#
#  WARNING: never run while ctest is running (WB_TcpLink port clash).
# =============================================================================
import argparse
import base64
import hashlib
import json
import os
import socket
import struct
import sys
import time

WS_GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'


# ---------------------------------------------------------------- HTTP ------
def http_get(host, port, target, timeout=10.0):
    """One request per connection -- the server closes after each (its own
    documented contract), so a fresh socket per GET is correct, not wasteful."""
    s = socket.create_connection((host, port), timeout=timeout)
    try:
        req = ('GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n'
               % (target, host, port))
        s.sendall(req.encode('ascii'))
        buf = b''
        while True:
            chunk = s.recv(65536)
            if not chunk:
                break
            buf += chunk
    finally:
        s.close()
    head, _, body = buf.partition(b'\r\n\r\n')
    first = head.split(b'\r\n', 1)[0].decode('latin-1')
    status = int(first.split()[1]) if len(first.split()) > 1 else 0
    ctype = ''
    for line in head.split(b'\r\n')[1:]:
        if line.lower().startswith(b'content-type:'):
            ctype = line.split(b':', 1)[1].strip().decode('latin-1')
    return status, ctype, body


# ------------------------------------------------------------ WebSocket -----
def ws_handshake(host, port, path, timeout=10.0, origin='self'):
    """origin='self' sends this server's own Origin; None sends NO Origin
    header at all (what a non-browser client looks like); any other string is
    sent verbatim."""
    key = base64.b64encode(os.urandom(16)).decode()
    expect = base64.b64encode(hashlib.sha1((key + WS_GUID).encode()).digest()).decode()
    s = socket.create_connection((host, port), timeout=timeout)
    if origin == 'self':
        origin = 'http://%s:%d' % (host, port)
    ohdr = '' if origin is None else ('Origin: %s\r\n' % origin)
    req = ('GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\n'
           'Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n'
           'Sec-WebSocket-Version: 13\r\n%s\r\n'
           % (path, host, port, key, ohdr))
    s.sendall(req.encode('ascii'))
    buf = b''
    deadline = time.time() + timeout
    while b'\r\n\r\n' not in buf and time.time() < deadline:
        chunk = s.recv(4096)
        if not chunk:
            break
        buf += chunk
    head, _, rest = buf.partition(b'\r\n\r\n')
    txt = head.decode('latin-1')
    first = txt.split('\r\n')[0]
    if '101' not in first:
        s.close()
        return None, b'', first
    if expect.lower() not in txt.lower():
        s.close()
        return None, b'', 'accept key mismatch'
    return s, rest, ''


def send_text(sock, payload):
    data = payload.encode('utf-8')
    hdr = bytearray([0x81])
    n = len(data)
    if n < 126:
        hdr.append(0x80 | n)
    elif n < 65536:
        hdr.append(0x80 | 126)
        hdr += struct.pack('>H', n)
    else:
        hdr.append(0x80 | 127)
        hdr += struct.pack('>Q', n)
    mask = os.urandom(4)
    hdr += mask
    sock.sendall(bytes(hdr) + bytes(bytearray(b ^ mask[i % 4]
                                             for i, b in enumerate(data))))


def read_frames(sock, leftover, deadline):
    """Yield complete text payloads; returns (payloads, leftover)."""
    out = []
    buf = leftover
    while time.time() < deadline:
        while True:
            if len(buf) < 2:
                break
            b1, b2 = buf[0], buf[1]
            ln = b2 & 0x7f
            off = 2
            if ln == 126:
                if len(buf) < 4:
                    break
                ln = struct.unpack('>H', buf[2:4])[0]
                off = 4
            elif ln == 127:
                if len(buf) < 10:
                    break
                ln = struct.unpack('>Q', buf[2:10])[0]
                off = 10
            if len(buf) < off + ln:
                break
            payload = buf[off:off + ln]
            buf = buf[off + ln:]
            if (b1 & 0x0f) == 0x1:
                out.append(payload.decode('utf-8', 'replace'))
        if out:
            return out, buf
        try:
            sock.settimeout(max(0.2, deadline - time.time()))
            chunk = sock.recv(65536)
        except socket.timeout:
            continue
        if not chunk:
            break
        buf += chunk
    return out, buf


def send_cmd(sock, leftover, cid, cmd, tag=None, value=None, timeout=20.0):
    msg = {'type': 'cmd', 'id': cid, 'cmd': cmd}
    if tag is not None:
        msg['tag'] = tag
    if value is not None:
        msg['value'] = value
    send_text(sock, json.dumps(msg))
    deadline = time.time() + timeout
    while time.time() < deadline:
        frames, leftover = read_frames(sock, leftover, deadline)
        for f in frames:
            try:
                m = json.loads(f)
            except Exception:
                continue
            if m.get('type') == 'ack' and m.get('id') == cid:
                return m, leftover
    return None, leftover


# ----------------------------------------------------------------- main -----
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--host', default='127.0.0.1')
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--path', default='/ht9045')
    ap.add_argument('--doc', default='contact',
                    help='preferred document; falls back to the first'
                         ' one the index offers that has a float field')
    ap.add_argument('--i-mean-it', action='store_true',
                    help='allow running against a non-scratch (production) path')
    a = ap.parse_args()

    print('=' * 74)
    print('recipe_probe: %s:%d  doc=%s' % (a.host, a.port, a.doc))
    print('=' * 74)

    # --- 1. the index -------------------------------------------------------
    try:
        st, ct, body = http_get(a.host, a.port, '/api/recipe/')
    except Exception as exc:
        print('FAIL cannot reach the server: %r' % (exc,))
        return 2
    print('GET /api/recipe/            HTTP %d  %s  %dB' % (st, ct, len(body)))
    if st != 200 or 'json' not in ct:
        print('FAIL index: expected 200 application/json')
        return 3
    index = json.loads(body.decode('utf-8'))
    print('    recipe   : %r' % index.get('recipe'))
    print('    path     : %r' % index.get('path'))
    print('    documents: %d  %s' % (len(index.get('documents') or []),
                                     (index.get('documents') or [])[:6]))

    scratch = 'temp' in (index.get('path') or '').lower()
    if not scratch and not a.i_mean_it:
        print()
        print('REFUSED: the server reports %r, which is not a scratch path.'
              % index.get('path'))
        print('         This probe WRITES. Start wb_serve with --dry, or pass')
        print('         --i-mean-it if you really intend to edit production data.')
        return 7
    print('    scratch  : %s' % ('yes (--dry active)' if scratch
                                 else 'NO -- writing to production, --i-mean-it given'))

    docs = index.get('documents') or []
    if not docs:
        print()
        print('EMPTY INDEX -- the active recipe directory has no .Data/.ini files.')
        print('  The route resolves GetRecipePath() = DataPath + GetLastOpenFN(),')
        print('  and GetLastOpenFN() reads LastDataPath = D:%sHT9045%sSetUp.inf'
              % (chr(92), chr(92)))
        print('  (common.cpp:109) LIVE, on every request.')
        print('  Under --dry, wb_serve scratch-copies the recipe folder ONCE at')
        print('  boot using the name it read then. If setup.inf now names a')
        print('  different recipe, the route points at a scratch subfolder that')
        print('  was never created -- exactly this symptom.')
        print('  Check: setup.inf vs CurrentSetupData.txt\'s "Setup File Name".')
        print('  They are allowed to differ and on at least one machine they do.')
        return 3
    if a.doc and a.doc not in docs:
        print('index does not list %r; falling back to the first document it does'
              % a.doc)
        a.doc = None
    if not a.doc:
        # Index-driven: pick the first document that actually has a float field,
        # because a number is the case the browser edits.
        for cand in docs:
            st2, ct2, b2 = http_get(a.host, a.port, '/api/recipe/' + cand)
            if st2 != 200:
                continue
            try:
                d2 = json.loads(b2.decode('utf-8'))
            except Exception:
                continue
            if any(f.get('type') == 'float'
                   for sec in (d2.get('sections') or {}).values()
                   for f in sec.values()):
                a.doc = cand
                break
        if not a.doc:
            print('FAIL none of the %d documents has an editable float field' % len(docs))
            return 3
        print('    chose    : %r' % a.doc)

    # --- 2. the document ----------------------------------------------------
    st, ct, body = http_get(a.host, a.port, '/api/recipe/' + a.doc)
    print('GET /api/recipe/%-12s HTTP %d  %s  %dB' % (a.doc, st, ct, len(body)))
    if st != 200 or 'json' not in ct:
        print('FAIL document: expected 200 application/json')
        return 3
    doc = json.loads(body.decode('utf-8'))
    for k in ('path', 'available', 'sections'):
        if k not in doc:
            print('FAIL document missing %r' % k)
            return 3
    if set(doc) != {'path', 'available', 'sections'}:
        print('FAIL document keys are %s, the contract says exactly three'
              % sorted(doc))
        return 3

    # pick the first float field -- a number is the case the browser edits
    target = None
    for sname, sec in doc['sections'].items():
        for kname, fld in sec.items():
            if fld.get('type') == 'float':
                target = (sname, kname, fld['raw'])
                break
        if target:
            break
    if not target:
        print('FAIL no float field to edit in %r' % a.doc)
        return 3
    sname, kname, oldraw = target
    print('    editing  : [%s] %s = %r' % (sname, kname, oldraw))

    # a value that is unmistakably ours and still a valid float
    newraw = '1.25' if oldraw.strip() != '1.25' else '2.50'

    # --- 3. the command channel --------------------------------------------
    sock, leftover, err = ws_handshake(a.host, a.port, a.path)
    if not sock:
        print('FAIL websocket handshake: %s' % err)
        return 4
    print('WS  handshake               101, accept key verified')

    try:
        ack, leftover = send_cmd(sock, leftover, 1, 'control.takeover')  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
        if not ack or not ack.get('ok'):
            print('FAIL control.takeover: %r' % ack)
            return 5
        print('WS  control.takeover         ok')

        payload = json.dumps({'dryRun': True,
                              'sections': {sname: {kname: {'raw': newraw}}}})
        ack, leftover = send_cmd(sock, leftover, 2, 'recipe.doc.put',
                                 tag=a.doc, value=payload)
        if not ack or not ack.get('ok'):
            print('FAIL recipe.doc.put (dry): %r' % ack)
            return 5
        print('WS  recipe.doc.put dryRun   ok')

        # the dry run must not have changed anything
        st, ct, body = http_get(a.host, a.port, '/api/recipe/' + a.doc)
        mid = json.loads(body.decode('utf-8'))
        if mid['sections'][sname][kname]['raw'] != oldraw:
            print('FAIL the DRY RUN changed the file: %r -> %r'
                  % (oldraw, mid['sections'][sname][kname]['raw']))
            return 6
        print('    after dry run           unchanged (%r)' % oldraw)

        payload = json.dumps({'sections': {sname: {kname: {'raw': newraw}}}})
        ack, leftover = send_cmd(sock, leftover, 3, 'recipe.doc.put',
                                 tag=a.doc, value=payload)
        if not ack or not ack.get('ok'):
            print('FAIL recipe.doc.put (apply): %r' % ack)
            return 5
        print('WS  recipe.doc.put apply    ok')
    finally:
        try:
            sock.close()
        except Exception:
            pass

    # --- 4. read it back over HTTP -----------------------------------------
    st, ct, body = http_get(a.host, a.port, '/api/recipe/' + a.doc)
    after = json.loads(body.decode('utf-8'))
    got = after['sections'][sname][kname]['raw']
    print('GET /api/recipe/%-12s [%s] %s = %r' % (a.doc, sname, kname, got))
    if got != newraw:
        print('FAIL the applied write did not come back: wanted %r got %r'
              % (newraw, got))
        return 6

    # every other field must be untouched -- the write is surgical
    moved = []
    for sn, sec in after['sections'].items():
        for kn, fld in sec.items():
            if (sn, kn) == (sname, kname):
                continue
            was = doc['sections'].get(sn, {}).get(kn, {}).get('raw')
            if was != fld['raw']:
                moved.append((sn, kn, was, fld['raw']))
    print('    collateral changes      %d' % len(moved))
    for m in moved[:10]:
        print('        %s' % (m,))
    if moved:
        print('FAIL the write touched fields it was not asked to')
        return 6

    # --- 5. the Origin gate, both directions -------------------------------
    #
    # Loopback binding keeps other machines out; it does nothing about another
    # PAGE in the same browser, which can open a WebSocket to 127.0.0.1 and be
    # allowed to. Origin is the header a page cannot forge, so this is the only
    # thing that tells our page from someone else's -- and a gate that is never
    # shown bad input has not been tested.
    print()
    bad, okabsent = None, None
    sock, _lo, err = ws_handshake(a.host, a.port, a.path,
                                  origin='http://evil.example.com')
    if sock:
        sock.close()
        print('FAIL a foreign Origin was ACCEPTED -- the gate is not closed')
        return 6
    bad = err
    print('WS  Origin: evil.example   REJECTED  (%s)' % bad)
    if '403' not in bad:
        print('FAIL expected 403, got %r' % bad)
        return 6

    sock, _lo, err = ws_handshake(a.host, a.port, a.path, origin=None)
    if not sock:
        print('FAIL an ABSENT Origin was rejected (%s) -- that breaks every raw'
              ' TCP client in tools/webprobe and tests/' % err)
        return 6
    sock.close()
    okabsent = True
    print('WS  Origin: (absent)        accepted  -- non-browser clients keep working')

    sock, _lo, err = ws_handshake(a.host, a.port, a.path, origin='null')
    if sock:
        sock.close()
        print('FAIL Origin: null was accepted -- that IS a browser (file://) and'
              ' it is not our page')
        return 6
    print('WS  Origin: null            REJECTED  (%s)' % err)

    print()
    print('PROBE GREEN: read, dry run, apply, and read-back all behaved, no')
    print('             other field moved, and the Origin gate refused a foreign')
    print('             page while still admitting a non-browser client.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
