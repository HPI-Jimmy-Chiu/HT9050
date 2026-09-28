"""INBOX 49 / RULINGS_20260927 第 3、5 條 -- step 3 of the action-flow comparison: drive the SIM wb_serve the way the operator of
the reference run did (boot -> control -> [lot start] -> START, answer the alarms like an operator), take Task_ListWithTime.csv
snapshots through act.main.stateRecord {"taskListOnly":true} (FLOWDIAG), then put every real file back (0918 rule: backup ->
verify -> restore; the 'opmode' sysguard snapshot for system/config/IniData, a private copy for MDB/CFG/SECS/setup.inf, and
'delete what the run created' for the log dirs).

usage: python flow_run.py <tag> <seconds> <checkpoints e.g. 60,180,400> [lotId operatorId]
env FLOW_START_MODE="Initial Start": send main.runStartMode (golden cbRunStartModeChange) before lot.start/START
output: scratchpad/flow/run_<tag>/  stdout.txt events.jsonl timeline.jsonl tasklist_<sec>.csv restore.txt
Refuses to start while a gate (ctest / cc1plus / ninja) is running: the ctest suite reads the live D:\\HT9045\\system.
"""
import base64, hashlib, json, os, queue, shutil, socket, struct, subprocess, sys, threading, time
_TREE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))   # <repo>/HT9011UC_Cpp_V3.33.906.0
_REPO = os.path.dirname(_TREE)
_OUT = os.environ.get('FLOW_OUT') or os.path.join(os.environ.get('TEMP', _REPO), 'ht9045_flowcmp')   # outputs never go into the tree

BS = chr(92)
ROOT = 'D:' + BS + 'HT9045'
EXE = os.environ.get('FLOW_EXE') or os.path.join(_REPO, 'Obj', 'V906', 'build_sim', 'wb_serve.exe')   # the SIM (SOFT_SIMULTE) build
SNAP = os.path.join(ROOT, 'backup', 'gate_sysguard', 'opmode')
DIRS = ['system', 'config', 'IniData']
COPYDIRS = ['MDB', 'CFG', 'SECS']
COPYFILES = ['setup.inf', 'CurrentSetupData.txt']
EXTRA = [os.path.join(ROOT, 'Error'), 'D:' + BS + 'HT9045_Log', 'D:' + BS + 'GPIBLOG', 'D:' + BS + 'HT9045_StateRecord',
         'D:' + BS + 'AutoCleanLogs', 'D:' + BS + 'HandlerLog', 'D:' + BS + 'RS232Log', 'D:' + BS + 'SECS_GEM_LOGS',
         'D:' + BS + 'UnloaderInfo']
GPIBF = ['D:' + BS + 'GPIB9045' + BS + 'system' + BS + 'general.ini', 'D:' + BS + 'GPIB9045' + BS + 'system' + BS + 'GpibString.dat']
PORT = 18123
WS_GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'
HERE = _OUT


def md5(p):
    with open(p, 'rb') as f:
        return hashlib.md5(f.read()).hexdigest()


def walk(base, dirs):
    for d in dirs:
        for dp, dn, fn in os.walk(os.path.join(base, d)):
            for f in fn:
                p = os.path.join(dp, f)
                yield os.path.relpath(p, base).replace(BS, '/'), p


def listing(d):
    out = set()
    if os.path.isdir(d):
        for dp, dn, fn in os.walk(d):
            for f in fn:
                out.add(os.path.join(dp, f))
    return out


def busy_gate():
    o = subprocess.run(['tasklist'], stdout=subprocess.PIPE).stdout.decode('cp950', 'replace').lower()
    return [x for x in ('ctest.exe', 'cc1plus.exe', 'ninja.exe', 'wb_serve.exe') if x in o]


def other_wb_serve(mypid):
    """PIDs of wb_serve.exe processes that are not ours (e.g. the user pressed F5 in VS Code mid-run).
    busy_gate() only guards the start; a second wb_serve during a flow_swap run would read/write the swapped-in
    real-machine system/config and its writes would be overwritten by the restore (user 20260927 18:1x)."""
    import csv
    o = subprocess.run(['tasklist', '/FI', 'IMAGENAME eq wb_serve.exe', '/FO', 'CSV', '/NH'],
                       stdout=subprocess.PIPE).stdout.decode('cp950', 'replace')
    pids = []
    for row in csv.reader(o.splitlines()):
        if len(row) > 1 and row[0].lower() == 'wb_serve.exe' and row[1].isdigit() and int(row[1]) != mypid:
            pids.append(int(row[1]))
    return pids


# ---------------------------------------------------------------- WebSocket client (masked client frames, RFC 6455)
class WS:
    def __init__(self, port):
        key = base64.b64encode(os.urandom(16)).decode()
        self.s = socket.create_connection(('127.0.0.1', port), timeout=5.0)
        self.s.sendall(('GET /ht9045 HTTP/1.1' + chr(13) + chr(10) + 'Host: 127.0.0.1:%d' % port + chr(13) + chr(10) +
                        'Upgrade: websocket' + chr(13) + chr(10) + 'Connection: Upgrade' + chr(13) + chr(10) +
                        'Sec-WebSocket-Key: ' + key + chr(13) + chr(10) + 'Sec-WebSocket-Version: 13' + chr(13) + chr(10) +
                        chr(13) + chr(10)).encode())
        buf = b''
        end = (chr(13) + chr(10) + chr(13) + chr(10)).encode()
        while end not in buf:
            c = self.s.recv(4096)
            if not c:
                raise RuntimeError('handshake closed')
            buf += c
        head, self.buf = buf.split(end, 1)
        if b' 101 ' not in head.split(b'\n')[0]:
            raise RuntimeError('handshake refused: %r' % head[:120])
        self.s.settimeout(0.5)
        self.lock = threading.Lock()
        self.q = queue.Queue()
        self.alive = True
        self.t = threading.Thread(target=self._reader, daemon=True)
        self.t.start()

    def _send(self, op, data):
        mask = os.urandom(4)
        h = bytearray([0x80 | op])
        n = len(data)
        if n < 126:
            h.append(0x80 | n)
        elif n < 65536:
            h.append(0x80 | 126); h += struct.pack('>H', n)
        else:
            h.append(0x80 | 127); h += struct.pack('>Q', n)
        h += mask
        with self.lock:
            self.s.sendall(bytes(h) + bytes(b ^ mask[i % 4] for i, b in enumerate(data)))

    def send_json(self, obj):
        self._send(1, json.dumps(obj, ensure_ascii=False).encode('utf-8'))

    def _reader(self):
        buf, frag = self.buf, b''
        while self.alive:
            while len(buf) >= 2:
                b0, b1 = buf[0], buf[1]
                ln, off = b1 & 0x7F, 2
                if ln == 126:
                    if len(buf) < 4: break
                    ln = struct.unpack('>H', buf[2:4])[0]; off = 4
                elif ln == 127:
                    if len(buf) < 10: break
                    ln = struct.unpack('>Q', buf[2:10])[0]; off = 10
                if len(buf) < off + ln:
                    break
                payload, buf = buf[off:off + ln], buf[off + ln:]
                op, fin = b0 & 0x0F, b0 & 0x80
                if op == 9:
                    try: self._send(10, payload)
                    except OSError: pass
                    continue
                if op == 8:
                    self.alive = False
                    break
                if op in (1, 0):
                    frag += payload
                    if fin:
                        try:
                            self.q.put(json.loads(frag.decode('utf-8', 'replace')))
                        except ValueError:
                            pass
                        frag = b''
            try:
                c = self.s.recv(262144)
            except socket.timeout:
                continue
            except OSError:
                break
            if not c:
                break
            buf += c
        self.alive = False


# ---------------------------------------------------------------- operator policy for alarms / YES-NO
PRIO = ['HOME', 'RETRY', 'SKIP', 'CLEAN_OUT', 'TRAY_FEED', 'TRAY_END', 'RESET', 'ONECYCLE', 'TRAIN']


MBOX = os.path.join(ROOT, 'web', 'JSON', 'runtime', 'Message-dialog-request.json')   # wb_serve MbPost (ShowMyMessage / YES_NO)


def mbox_choose(answered):
    """The 'modal' WS frame is only a trigger; the request (requestId + buttons) is in the mailbox file.
    Operator policy: YES/NO box -> NO (conservative, same as choose()); otherwise press the one button it shows."""
    try:
        j = json.load(open(MBOX, encoding='utf-8'))
    except (OSError, ValueError):
        return None, None
    qid = j.get('requestId') or ''
    if j.get('state') != 'pending' or not qid or qid in answered:
        return None, None
    p = (j.get('display') or {}).get('panels') or {}
    vis = lambda k: bool((p.get(k) or {}).get('visible'))
    if vis('pnlYes') and vis('pnlNo'):
        return qid, 'NO'
    if vis('pnlPause'):
        return qid, ((p.get('pnlPause') or {}).get('caption') or 'Pause').upper()
    if vis('pnlAlarmReset'):
        return qid, 'ALARMRESET'
    return None, None


def choose(frame, seen):
    opts = frame.get('options') or []
    code = frame.get('code') or ''
    key = code or (frame.get('text') or '')[:60]
    seen[key] = seen.get(key, 0) + 1
    if not opts:
        return None
    if 'YES' in opts and 'NO' in opts:
        return 'NO'                      # conservative: a YES may clear data or start something; review events.jsonl
    ranked = [o for o in PRIO if o in opts] + [o for o in opts if o not in PRIO]
    k = min(seen[key] - 1, 4) // 2      # the same alarm again and again: move down the list every 2 repeats
    return ranked[min(k, len(ranked) - 1)]


def main():
    tag, total, cps = sys.argv[1], int(sys.argv[2]), [int(x) for x in sys.argv[3].split(',') if x]
    lot = sys.argv[4] if len(sys.argv) > 4 else ''
    opr = sys.argv[5] if len(sys.argv) > 5 else ''
    b = busy_gate()
    assert not b, 'refused: running now: %r' % b
    out = os.path.join(HERE, 'run_' + tag)
    os.makedirs(out, exist_ok=False)
    rep = open(os.path.join(out, 'restore.txt'), 'w', encoding='utf-8')

    def say(*a):
        s = ' '.join(str(x) for x in a)
        print(s); rep.write(s + '\n'); rep.flush()

    # ---- baseline + private copies
    # FLOW_SWAPPED=1 (flow_swap.py): system/config/setup.inf hold the reference machine's files and IniData has the reference
    # recipe added; flow_swap.py swaps them back.  Here only IniData (minus the added recipe) is compared / restored.
    swapped = os.environ.get('FLOW_SWAPPED') == '1'
    skip_recipe = 'IniData/Data/' + os.environ.get('FLOW_SWAP_RECIPE', '\0') + '/'
    man_all = json.load(open(os.path.join(SNAP, '_manifest.json'), encoding='utf-8'))
    in_scope = (lambda r: r.startswith('IniData/') and not r.startswith(skip_recipe)) if swapped else (lambda r: True)
    man = dict((r, h) for r, h in man_all.items() if in_scope(r))
    pre = dict((r, md5(p)) for r, p in walk(ROOT, DIRS) if in_scope(r))
    diff0 = [r for r in man if pre.get(r) != man[r]] + [r for r in pre if r not in man]
    assert not diff0, 'not at the opmode baseline before the run: %r' % diff0[:5]
    if swapped:
        say('SWAPPED mode: comparing / restoring IniData only (minus %s); system/config/setup.inf are flow_swap.py\'s' % skip_recipe)
    precopy = os.path.join(out, '_pre')
    for d in COPYDIRS:
        if os.path.isdir(os.path.join(ROOT, d)):
            shutil.copytree(os.path.join(ROOT, d), os.path.join(precopy, d))
    for f in COPYFILES:
        if os.path.isfile(os.path.join(ROOT, f)):
            os.makedirs(precopy, exist_ok=True)
            shutil.copy2(os.path.join(ROOT, f), os.path.join(precopy, f))
    cpre = dict((r, md5(p)) for r, p in walk(ROOT, [d for d in COPYDIRS if os.path.isdir(os.path.join(ROOT, d))]))
    fpre = dict((f, md5(os.path.join(ROOT, f))) for f in COPYFILES if os.path.isfile(os.path.join(ROOT, f)) and not (swapped and f == 'setup.inf'))
    gpib0 = dict((p, open(p, 'rb').read() if os.path.exists(p) else None) for p in GPIBF)
    existed = dict((d, os.path.isdir(d)) for d in EXTRA)
    before = dict((d, listing(d)) for d in EXTRA)
    dirs_before = dict((d, set(dp for dp, dn, fn in os.walk(d))) for d in EXTRA)
    dtop0, rtop0 = set(os.listdir('D:' + BS)), set(os.listdir(ROOT))

    # ---- run
    env = dict(os.environ, W906_NO_BROWSER_WAKE='1')
    fo = open(os.path.join(out, 'stdout.txt'), 'wb')
    t0 = time.time()
    proc = subprocess.Popen([EXE, '--seconds', str(total), '--port', str(PORT)], stdout=fo, stderr=subprocess.STDOUT,
                            env=env, cwd=os.path.dirname(EXE))
    ev = open(os.path.join(out, 'events.jsonl'), 'w', encoding='utf-8')
    tl = open(os.path.join(out, 'timeline.jsonl'), 'w', encoding='utf-8')
    ws, nid, acks, seen, latest = None, [100], {}, {}, {}

    def rel():
        return round(time.time() - t0, 2)

    def log(kind, obj):
        ev.write(json.dumps({'t': rel(), 'kind': kind, 'obj': obj}, ensure_ascii=False) + '\n'); ev.flush()

    def cmd(name, value=None, tagv=None):
        nid[0] += 1
        m = {'type': 'cmd', 'id': nid[0], 'cmd': name}
        if value is not None: m['value'] = value
        if tagv is not None: m['tag'] = tagv
        log('send', m)
        ws.send_json(m)
        return nid[0]

    mbans = set()   # MyMessageBox requestIds already answered
    aborted = []    # non-empty = a foreign wb_serve showed up mid-run (other_wb_serve); exit code 3

    def pump(until):
        while time.time() < until:
            try:
                j = ws.q.get(timeout=0.2)
            except queue.Empty:
                if not ws.alive or proc.poll() is not None:
                    return
                continue
            ty = j.get('type')
            if ty == 'ack':
                acks[j.get('id')] = j; log('ack', j)
            elif ty == 'query':
                a = choose(j, seen)
                log('query', j)
                if a is not None:
                    nid[0] += 1
                    m = {'type': 'cmd', 'id': nid[0], 'cmd': 'modal.answer', 'tag': str(j.get('qid', '')), 'value': a}
                    log('answer', m); ws.send_json(m)
            elif ty == 'modal':
                log('modal', j)
                q, a = mbox_choose(mbans)
                if q:
                    mbans.add(q); nid[0] += 1
                    m = {'type': 'cmd', 'id': nid[0], 'cmd': 'dialog.response', 'tag': q, 'value': a}
                    log('answer', m); ws.send_json(m)
            d = j.get('data')
            if isinstance(d, dict):
                for k, v in d.items():
                    if k.startswith(('pump.task.', 'guard.', 'machine.')):
                        latest[k] = v

    def wait_ack(i, sec):
        end = time.time() + sec
        while time.time() < end and i not in acks:
            pump(min(end, time.time() + 0.5))
        return acks.get(i)

    try:
        for k in range(60):
            try:
                ws = WS(PORT); break
            except OSError:
                time.sleep(1)
        assert ws is not None, 'no WebSocket'
        say('ws up at', rel())
        say('acquire', json.dumps(wait_ack(cmd('control.acquire'), 10), ensure_ascii=False)[:200])
        smode = os.environ.get('FLOW_START_MODE', '')   # RSMODE: the reference operator picked "Initial Start" before HOME/START
        if smode:
            say('main.runStartMode', json.dumps(wait_ack(cmd('main.runStartMode', smode), 20), ensure_ascii=False)[:300])
        if lot:
            say('lot.start', json.dumps(wait_ack(cmd('lot.start', opr, lot), 20), ensure_ascii=False)[:300])
        say('start.run', json.dumps(wait_ack(cmd('start.run', 'web'), 60), ensure_ascii=False)[:300])
        nexttl, cpi, nextintr = time.time(), 0, time.time()
        while time.time() - t0 < total - 5 and proc.poll() is None and ws.alive:
            pump(time.time() + 1.0)
            if time.time() >= nextintr:
                intr = other_wb_serve(proc.pid)
                nextintr = time.time() + 5
                if intr:
                    aborted.append(intr)
                    say('ABORT: another wb_serve.exe appeared mid-run (pid %r) -- stopping; results of this run are not valid' % intr)
                    proc.kill()
                    break
            if time.time() >= nexttl:
                tl.write(json.dumps({'t': rel(), 'tags': latest}, ensure_ascii=False) + '\n'); tl.flush(); nexttl = time.time() + 2
            if cpi < len(cps) and rel() >= cps[cpi]:
                a = wait_ack(cmd('act.main.stateRecord', json.dumps({'taskListOnly': True})), 30)
                say('stateRecord @%d' % cps[cpi], json.dumps(a, ensure_ascii=False)[:300])
                fold = (a or {}).get('folder') or ''
                src = os.path.join(fold, 'Task_ListWithTime.csv')
                if fold and os.path.isfile(src):
                    shutil.copy2(src, os.path.join(out, 'tasklist_%d.csv' % cps[cpi]))
                    dv = os.path.join(fold, 'DecisionVariables.csv')
                    if os.path.isfile(dv): shutil.copy2(dv, os.path.join(out, 'decision_%d.csv' % cps[cpi]))
                else:
                    say('  no Task_ListWithTime.csv at', src)
                cpi += 1
    finally:
        if ws is not None:
            ws.alive = False
        try:
            proc.wait(timeout=max(10, total - (time.time() - t0) + 30))
        except subprocess.TimeoutExpired:
            proc.kill(); proc.wait()
        fo.close(); ev.close(); tl.close()
        say('wb_serve rc', proc.returncode, 'elapsed %.1f s' % (time.time() - t0), 'queries', sum(seen.values()), dict(seen))

        # ---- restore
        keep = os.path.join(out, '_written')
        os.makedirs(keep, exist_ok=True)
        post = dict((r, md5(p)) for r, p in walk(ROOT, DIRS) if in_scope(r))
        changed = [r for r in man if r in post and post[r] != man[r]]
        gone = [r for r in man if r not in post]
        new = [r for r in post if r not in man]
        say('system/config/IniData: changed %d gone %d new %d' % (len(changed), len(gone), len(new)))
        for r in changed + new:
            shutil.copy2(os.path.join(ROOT, r), os.path.join(keep, r.replace('/', '__')))
        for r in changed + gone:
            dst = os.path.join(ROOT, r)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copy2(os.path.join(SNAP, r), dst)
            say('   restored', r, 'ok' if md5(dst) == man[r] else 'FAILED')
        for r in new:
            os.remove(os.path.join(ROOT, r)); say('   removed new', r)
        cpost = dict((r, md5(p)) for r, p in walk(ROOT, [d for d in COPYDIRS if os.path.isdir(os.path.join(ROOT, d))]))
        for r in sorted(set(cpre) | set(cpost)):
            if cpre.get(r) == cpost.get(r):
                continue
            dst = os.path.join(ROOT, r)
            if r in cpre:
                shutil.copy2(dst, os.path.join(keep, r.replace('/', '__'))) if r in cpost else None
                shutil.copy2(os.path.join(precopy, r), dst)
                say('   restored', r, 'ok' if md5(dst) == cpre[r] else 'FAILED')
            else:
                shutil.copy2(dst, os.path.join(keep, r.replace('/', '__'))); os.remove(dst); say('   removed new', r)
        for f, h in fpre.items():
            dst = os.path.join(ROOT, f)
            if not os.path.isfile(dst) or md5(dst) != h:
                shutil.copy2(os.path.join(precopy, f), dst); say('   restored', f, 'ok' if md5(dst) == h else 'FAILED')
        for p, data in gpib0.items():
            cur = open(p, 'rb').read() if os.path.exists(p) else None
            if cur != data:
                if data is None:
                    os.remove(p)
                else:
                    open(p, 'wb').write(data)
                say('   GPIB restored', p)
        for d in EXTRA:
            added = sorted(listing(d) - before[d])
            for p in added:
                dstk = os.path.join(keep, 'extra__' + os.path.relpath(p, 'D:' + BS).replace(BS, '__'))
                shutil.copy2(p, dstk); os.remove(p)
            if added:
                say('%s: %d new file(s) moved to _written' % (d, len(added)))
            if os.path.isdir(d):
                for nd in sorted((dp for dp, dn, fn in os.walk(d) if dp not in dirs_before[d]), key=len, reverse=True):
                    try:
                        os.rmdir(nd)
                    except OSError:
                        say('   could not remove dir', nd)
            if not existed[d] and os.path.isdir(d):
                try:
                    os.rmdir(d); say('   removed dir created by the run:', d)
                except OSError:
                    say('   dir created by the run is not empty:', d)
        say('new at D:\\ top level:', sorted(set(os.listdir('D:' + BS)) - dtop0))
        say('new at D:\\HT9045 top level:', sorted(set(os.listdir(ROOT)) - rtop0))
        post2 = dict((r, md5(p)) for r, p in walk(ROOT, DIRS) if in_scope(r))
        say('re-check vs opmode:', 'CLEAN' if post2 == man else 'DIFF %r' % [r for r in set(post2) | set(man) if post2.get(r) != man.get(r)][:5])
        if aborted:
            say('RUN ABORTED (foreign wb_serve %r): do not compare these task lists' % aborted)
        rep.close()
    return 3 if aborted else 0


if __name__ == '__main__':
    sys.exit(main())
