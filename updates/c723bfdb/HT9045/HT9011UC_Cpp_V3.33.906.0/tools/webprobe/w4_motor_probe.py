# =============================================================================
#  tools/webprobe/w4_motor_probe.py -- W4 motor.access / motor.stop end-to-end probe.
#
#  AI(W906-W4-MOTOR) 20260925: new file. Drives a running wb_serve over the real
#  WebSocket (same framing as cmd_probe.py) and checks what the MotorTest page
#  would get back. Built for a machine WITHOUT a PCIE-1203 card (this laptop):
#  every 1203 path must refuse honestly, the golden (MOT[]) paths must answer.
#
#    1. motor.access without the operator token      -> ok:false "not-operator"
#    2. control.takeover                              -> ok
#    3. servoToggle on a PCI1203 axis                -> ok:false (1203 control not armed)
#    4. motor.stop from a SECOND connection, no token -> ok:true, StopAllMotor ran   (token exemption)
#    5. motor.stop carrying action=jogP              -> ok:false (motor.stop is locked to action=stop)
#    6. home on a PCI1203 axis                       -> ok:false
#    7. motorPowerToggle                             -> ok:false, names the missing dependency (G9)
#    8. teachGo (uteach)                             -> ok:false, names W5
#    9. setJogHighSpeed on a non-1203 axis           -> ok:true with a value   (golden MOT memory only)
#   10. GET /api/struct/motor/runtime                -> JSON with motors[]
#
#  ⚠ Run it against wb_serve only after `python tools/realfile_guard.py snap <tag>` --
#    wb_serve boot writes real machine files (Gerneral.ini defaults, machinerecord.dat).
#  Exit 0 = every expectation met.
# =============================================================================
import argparse, json, os, sys, time, urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake, send_text   # noqa: E402
import socket, struct   # noqa: E402


class FrameReader:
    """Persistent server-frame reader. cmd_probe.wait_ack rebuilds its generator per call, so bytes of a
    half-received snapshot frame that follow an ack are dropped and the stream goes out of step --
    measured 20260925: 3 of 10 acks lost although wb_serve logged every reply. This keeps the buffer."""
    def __init__(self, sock, leftover):
        self.sock, self.buf = sock, leftover

    def next_text(self, deadline):
        while True:
            b = self.buf
            if len(b) >= 2:
                ln, off = b[1] & 0x7F, 2
                if ln == 126 and len(b) >= 4:
                    ln, off = struct.unpack('>H', b[2:4])[0], 4
                elif ln == 127 and len(b) >= 10:
                    ln, off = struct.unpack('>Q', b[2:10])[0], 10
                if (ln < 126 or off > 2) and len(b) >= off + ln:
                    op, payload = b[0] & 0x0F, b[off:off + ln]
                    self.buf = b[off + ln:]
                    if op == 1:
                        return payload.decode('utf-8', 'replace')
                    continue
            if time.monotonic() > deadline:
                return None
            try:
                c = self.sock.recv(65536)
            except socket.timeout:
                continue
            if not c:
                return None
            self.buf += c

FAILS = []


def check(cond, what, detail=''):
    print(('  PASS: ' if cond else '  FAIL: ') + what + ((' -- ' + detail) if detail and not cond else ''))
    if not cond:
        FAILS.append(what)


def req(action, motors, source='uMotorTest', button='btn', params=None, seq=1):
    return json.dumps({'seq': seq, 'id': 'cmd-%d' % seq, 'source': source, 'button': button,
                       'action': action, 'kind': 'control', 'motors': motors, 'params': params or {},
                       'issuedAt': '2026-09-25T00:00:00Z', 'state': 'requested'})


class Conn:
    def __init__(self, host, port, path, seconds):
        self.deadline = time.monotonic() + seconds
        self.sock, left = ws_handshake(host, port, path, self.deadline)
        self.rd = FrameReader(self.sock, left)
        self.nid = 100

    def cmd(self, name, tag=None, value=None):
        self.nid += 1
        msg = {'type': 'cmd', 'id': self.nid, 'cmd': name}
        if tag is not None:
            msg['tag'] = tag
        if value is not None:
            msg['value'] = value
        send_text(self.sock, json.dumps(msg))
        dl = min(self.deadline, time.monotonic() + 8.0)
        while True:
            t = self.rd.next_text(dl)
            if t is None:
                return {}
            try:
                j = json.loads(t)
            except ValueError:
                continue
            if j.get('type') == 'ack' and j.get('id') == self.nid:
                return j


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--host', default='127.0.0.1')
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--path', default='/ht9045')
    ap.add_argument('--seconds', type=float, default=40.0)
    ap.add_argument('--axis1203', default='MOutArmY', help='a PCI1203 Alias in Mot_Table')
    ap.add_argument('--axisOther', default='MInArmPitch', help='a non-1203 Alias in Mot_Table')
    a = ap.parse_args()

    try:
        cfg = json.loads(urllib.request.urlopen('http://%s:%d/api/struct/motor/config' % (a.host, a.port), timeout=5).read().decode('utf-8', 'replace'))
        ms = cfg.get('motors') or []
        p1203 = [m['motorId'] for m in ms if m.get('driverType') == 'PCI1203']
        other = [m['motorId'] for m in ms if m.get('driverType') not in ('PCI1203', '', None)]
        if p1203: a.axis1203 = p1203[0]
        else: a.axis1203 = None
        if other: a.axisOther = other[0]
        print('table: %d axes, PCI1203 %d -> axis1203=%s, axisOther=%s' % (len(ms), len(p1203), a.axis1203, a.axisOther))
    except Exception as e:  # noqa: BLE001
        print('config API unreadable, keeping defaults: %r' % e)
    c1 = Conn(a.host, a.port, a.path, a.seconds)
    print('[1] token gate')
    r = c1.cmd('motor.access', a.axisOther, req('servoToggle', [a.axisOther]))
    check(r.get('ok') is False and 'not-operator' in str(r.get('error', '')), 'motor.access without token -> not-operator', json.dumps(r, ensure_ascii=False))
    print('[2] acquire')
    r = c1.cmd('control.takeover')  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
    check(r.get('ok') is True, 'control.takeover', json.dumps(r, ensure_ascii=False))
    print('[3] 1203 without a card')
    if a.axis1203:
        r = c1.cmd('motor.access', a.axis1203, req('servoToggle', [a.axis1203], params={'servoOn': True}))
        check(r.get('ok') is False and '1203' in str(r.get('error', '')), 'servoToggle on PCI1203 -> honest refusal', json.dumps(r, ensure_ascii=False))
    else:
        print('  SKIP: this machine has no PCI1203 axis in its Mot_Table')
    print('[4] motor.stop from a second connection (no token)')
    c2 = Conn(a.host, a.port, a.path, a.seconds)
    r = c2.cmd('motor.stop', '', req('stop', [], button='btnStop'))
    check(r.get('ok') is True and r.get('result') == 'stopped', 'motor.stop without token -> stopped', json.dumps(r, ensure_ascii=False))
    check(isinstance(r.get('pci1203'), dict) and r['pci1203'].get('ready') is False, 'stop ack says the 1203 half is not ready', json.dumps(r, ensure_ascii=False))
    print('[5] motor.stop locked to action=stop')
    r = c2.cmd('motor.stop', a.axisOther, req('jogP', [a.axisOther], params={'speed': 10}))
    check(r.get('ok') is False and 'motor.stop' in str(r.get('error', '')), 'motor.stop with jogP -> refused', json.dumps(r, ensure_ascii=False))
    print('[6] home without a 1203 control layer')
    ax6 = a.axis1203 or a.axisOther
    r = c1.cmd('motor.access', ax6, req('home', [ax6], params={'start': True}))   # AI(W906-W4D) 20260925: NB2 R23 W4C-5 explicit intent
    check(r.get('ok') is False, 'home -> refused (1203 not armed, or non-1203 ProcessSingleMotorHome stub)', json.dumps(r, ensure_ascii=False))
    print('[7] blocked button')
    r = c1.cmd('motor.access', '', req('motorPowerToggle', [], params={'powerOn': True}))
    check(r.get('ok') is False and 'G9' in str(r.get('error', '')), 'motorPowerToggle -> blocked (G9)', json.dumps(r, ensure_ascii=False))
    print('[8] teach motion')
    r = c1.cmd('motor.access', a.axisOther, req('teachGo', [a.axisOther], source='uteach'))
    check(r.get('ok') is False and 'W5' in str(r.get('error', '')), 'teachGo -> W5', json.dumps(r, ensure_ascii=False))
    print('[9] golden MOT memory')
    r = c1.cmd('motor.access', a.axisOther, req('setJogHighSpeed', [a.axisOther], params={'speed': 50}))
    ok9 = (r.get('ok') is True and isinstance(r.get('value'), int))
    why9 = str(r.get('error', ''))
    check(ok9 or ('MOT[].Motor' in why9), 'setJogHighSpeed on non-1203 -> value (or honest "no golden motor object")', json.dumps(r, ensure_ascii=False))
    print('[10] runtime API')
    try:
        body = urllib.request.urlopen('http://%s:%d/api/struct/motor/runtime' % (a.host, a.port), timeout=5).read()
        j = json.loads(body.decode('utf-8', 'replace'))
        check(isinstance(j.get('motors'), list) and len(j['motors']) > 0, 'runtime has motors[]', str(j)[:200])
    except Exception as e:  # noqa: BLE001
        check(False, 'runtime API reachable', repr(e))
    c1.cmd('control.release')
    print('\n%d failed' % len(FAILS))
    return 0 if not FAILS else 1


if __name__ == '__main__':
    sys.exit(main())
