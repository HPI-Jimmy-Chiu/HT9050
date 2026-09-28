"""INBOX 49 / RULINGS_20260927 第 3、5 條 -- step 3 of the action-flow comparison: drive the SIM wb_serve the way the operator of
the reference run did (boot -> control -> [lot start] -> START, answer the alarms like an operator), take Task_ListWithTime.csv
snapshots through act.main.stateRecord {"taskListOnly":true} (FLOWDIAG), then put every real file back (0918 rule: backup ->
verify -> restore; the 'opmode' sysguard snapshot for system/config/IniData, a private copy for MDB/CFG/SECS/setup.inf, and
'delete what the run created' for the log dirs).

usage: python flow_run.py <tag> <seconds> <checkpoints e.g. 60,180,400> [lotId operatorId]
env FLOW_START_MODE="Initial Start": send main.runStartMode (golden cbRunStartModeChange) before lot.start/START
output: scratchpad/flow/run_<tag>/  stdout.txt events.jsonl timeline.jsonl tasklist_<sec>.csv restore.txt
Refuses to start while a gate (ctest / cc1plus / ninja) is running: the ctest suite reads the live D:\\HT9045\\system.

v2 (20260927, flowcmp v2; research report staterecord-flow-gap.md B5/B8):
env FLOW_SCRIPT=<json>  operator script: a list of steps, or {"steps": [...], "snapshots": [...], "answers": [...]}
  (no "steps" = the default sequence above, plus the snapshots/answers).  control.acquire always goes first.
  step keys: cmd, value, tag (the strings $LOT / $OP become argv lotId / operatorId), ack_timeout (s, default 20),
    wait {tag: <streamed tag>, task: <Task_ListWithTime alias>, until: value or [values], timeout: s (default 60),
          poll: s (task polling through act.main.stateRecord, default 5)} -- met when the value is seen after the step's
          command was sent (the wait's start for a step without cmd) or is the current one; a timeout is logged and the
          script goes on.  Only the tags WebBridgeTags.cpp publishes can be waited on by tag (e.g. pump.task.testHead,
          pump.guard.allMotorHome); fHome->iHomeStep is NOT streamed -- wait on it as task "HomeStep" (QueueTaskList[267],
          port cStateRecord.cpp:1981 = golden main.cpp:10012),
    wait_even_if_failed (default false: a cmd whose ack failed skips its wait), sleep: s, snapshot: <name>, note.
  A failed or missing ack is logged and the script CONTINUES: e.g. main.home (wb_serve.cpp W906_MainHomeCommand, 816ce9b2)
  acks ok=false with outcome "noop" in a ship build (golden BtnHomeClick does nothing without SOFT_SIMULTE), and START then
  homes by itself.  script_result.json records each ack's 'outcome' (homeArmed / noop / disabled / ...).
  snapshots: [{name, tag|task, until, poll?}] -- anchor snapshots: tasklist_<name>.csv the first time the value is seen
    (tag: any value that came through the stream since the run started; task: polled).
  Per-step results (sent time, ack, wait) -> script_result.json; flow_cmp v2 takes its anchor floor from it.
env FLOW_ANSWERS=<json>  scripted dialog answers, a list or {"answers": [...]} of {id, match: regex, answer, channel:
  query|mbox|any, times?} (also accepted inside FLOW_SCRIPT).  The first rule whose regex matches the dialog text wins:
  query text = code | kind | text of the WS 'query' frame; mbox text = arguments.s1/s2/s3 + display texts of
  Message-dialog-request.json.  An answer the dialog does not offer (query: not in 'options'; mailbox: YES/NO needs both
  panels, ALARMRESET its panel, anything else pnlPause with that caption) falls back to the default policy below.
events.jsonl entries also carry 'wall' (HH:MM:SS.mmm, the clock the task list uses).
"""
import base64, hashlib, json, os, queue, re, shutil, socket, struct, subprocess, sys, threading, time
_TREE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))   # <repo>/HT9011UC_Cpp_V3.33.906.0
_REPO = os.path.dirname(_TREE)
_OUT = os.environ.get('FLOW_OUT') or os.path.join(os.environ.get('TEMP', _REPO), 'ht9045_flowcmp')   # outputs never go into the tree
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from flow_ref import parse_pairs, tms   # v2: task polling (script waits, anchor snapshots)

BS = chr(92)
ROOT = 'D:' + BS + 'HT9045'
EXE = os.environ.get('FLOW_EXE') or os.path.join(_REPO, 'Obj', 'V906', 'build_sim', 'wb_serve.exe')   # the SIM (SOFT_SIMULTE) build
# AI(W906-FLOW-2) 20260928: the laptop's baseline is now the HT9050 machine configuration (RULINGS_20260928 #4; sysguard
#   snapshot 'ht9050m_0928'); 'opmode' is the pre-0928 laptop config and restoring it would undo the 9050 install.
SNAP = os.path.join(ROOT, 'backup', 'gate_sysguard', os.environ.get('FLOW_SNAP', 'ht9050m_0928'))
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


def wall():
    """local clock as HH:MM:SS.mmm -- the format Task_ListWithTime.csv uses (flow_cmp v2 anchors on it)"""
    t = time.time()
    return time.strftime('%H:%M:%S', time.localtime(t)) + '.%03d' % int((t % 1) * 1000)


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


# ---------------------------------------------------------------- v2: scripted answers / operator script
def load_answers(items, src):
    rules = []
    for n, r in enumerate(items or []):
        if not isinstance(r, dict) or not r.get('match') or not r.get('answer'):
            raise SystemExit('%s: answer rule #%d needs match and answer: %r' % (src, n, r))
        ch = r.get('channel', 'any')
        if ch not in ('query', 'mbox', 'any'):
            raise SystemExit('%s: answer rule #%d: channel must be query, mbox or any' % (src, n))
        rules.append({'id': r.get('id') or ('%s#%d' % (os.path.basename(src), n)), 'rx': re.compile(r['match']),
                      'answer': str(r['answer']).upper(), 'channel': ch, 'times': r.get('times'), 'used': 0})
    return rules


STEP_KEYS = {'cmd', 'value', 'tag', 'ack_timeout', 'wait', 'wait_even_if_failed', 'sleep', 'snapshot', 'note'}
WAIT_KEYS = {'tag', 'task', 'until', 'timeout', 'poll', 'note'}


def _check_wait(w, where):
    if not isinstance(w, dict) or not (set(w) <= WAIT_KEYS) or 'until' not in w or not ('tag' in w or 'task' in w):
        raise SystemExit('%s: wait needs tag and/or task plus until (keys %s): %r' % (where, sorted(WAIT_KEYS), w))


def load_script(path):
    """FLOW_SCRIPT json -> {'steps': list or None, 'snapshots': [...], 'answers': [...]}; validated before anything runs"""
    d = json.load(open(path, encoding='utf-8'))
    if isinstance(d, list):
        d = {'steps': d}
    steps = d.get('steps')
    for i, s in enumerate(steps or []):
        where = '%s step %d' % (path, i)
        if not isinstance(s, dict) or not (set(s) <= STEP_KEYS) or not ({'cmd', 'sleep', 'snapshot', 'wait'} & set(s)):
            raise SystemExit('%s: unknown or empty step (keys %s): %r' % (where, sorted(STEP_KEYS), s))
        if 'wait' in s:
            _check_wait(s['wait'], where)
        if 'snapshot' in s and (not isinstance(s['snapshot'], str) or s['snapshot'].isdigit()):
            raise SystemExit('%s: snapshot name must be a non-numeric string (numbers are the seconds checkpoints)' % where)
    names = set()
    for i, a in enumerate(d.get('snapshots') or []):
        where = '%s snapshot %d' % (path, i)
        if not isinstance(a, dict) or not isinstance(a.get('name'), str) or a['name'].isdigit() or a['name'] in names:
            raise SystemExit('%s: needs a unique non-numeric name: %r' % (where, a))
        names.add(a['name'])
        _check_wait(dict((k, v) for k, v in a.items() if k != 'name'), where)
    return {'steps': steps, 'snapshots': d.get('snapshots') or [], 'answers': load_answers(d.get('answers'), path)}


def _want(w):
    u = w['until']
    return u if isinstance(u, list) else [u]


def rule_for(rules, channel, textv):
    for r in rules:
        if r['channel'] in ('any', channel) and (r['times'] is None or r['used'] < r['times']) and r['rx'].search(textv):
            return r
    return None


def ack_outcome(a):
    """the handler's 'outcome' field, if it has one: on success WebBridgeServer splices the handler's JSON object into the ack
    (AckJson, WebBridge/WebBridgeServer.cpp:1264-1304); on failure the same object arrives as the 'error' string.
    main.home: homeArmed (SIM) / noop (ship build, golden BtnHomeClick does nothing) / disabled / teachModal / ... (wb_serve.cpp
    W906_MainHomeCommand)."""
    if not isinstance(a, dict):
        return None
    if a.get('outcome') is not None:
        return a.get('outcome')
    e = a.get('error')
    if isinstance(e, str) and e.startswith('{'):
        try:
            return (json.loads(e) or {}).get('outcome')
        except ValueError:
            return None
    return None


def mbox_scripted(answered, rules):
    """(requestId, answer, rule id, text) when a scripted rule matches the pending ShowMyMessage request and the box offers
    that answer; requestId None otherwise, so the default policy (mbox_choose) decides."""
    if not rules:
        return None, None, None, ''
    try:
        j = json.load(open(MBOX, encoding='utf-8'))
    except (OSError, ValueError):
        return None, None, None, ''
    qid = j.get('requestId') or ''
    if j.get('state') != 'pending' or not qid or qid in answered:
        return None, None, None, ''
    ar, dp = j.get('arguments') or {}, j.get('display') or {}
    txt = ' | '.join(x for x in (ar.get('s1'), ar.get('s2'), ar.get('s3'), dp.get('primaryText'), dp.get('secondaryText'),
                                 dp.get('subText')) if x)
    r = rule_for(rules, 'mbox', txt)
    if r is None:
        return None, None, None, txt
    p = dp.get('panels') or {}
    vis = lambda k: bool((p.get(k) or {}).get('visible'))
    a = r['answer']
    # the box only takes what it shows (wb_serve MbWait answers anything else "not an offered option" and keeps waiting):
    # YES/NO = both panels, ALARMRESET = pnlAlarmReset, any other answer = pnlPause with that caption (mbox_choose sends the caption)
    offered = ((vis('pnlYes') and vis('pnlNo')) if a in ('YES', 'NO') else vis('pnlAlarmReset') if a == 'ALARMRESET'
               else vis('pnlPause') and ((p.get('pnlPause') or {}).get('caption') or 'Pause').upper() == a)
    if not offered:
        return None, None, 'refused:' + r['id'], txt
    r['used'] += 1
    return qid, a, r['id'], txt


def main():
    tag, total, cps = sys.argv[1], int(sys.argv[2]), [int(x) for x in sys.argv[3].split(',') if x]
    lot = sys.argv[4] if len(sys.argv) > 4 else ''
    opr = sys.argv[5] if len(sys.argv) > 5 else ''
    # v2: load and validate the operator script / answers BEFORE touching anything
    script = load_script(os.environ['FLOW_SCRIPT']) if os.environ.get('FLOW_SCRIPT') else {'steps': None, 'snapshots': [], 'answers': []}
    answers = list(script['answers'])
    if os.environ.get('FLOW_ANSWERS'):
        da = json.load(open(os.environ['FLOW_ANSWERS'], encoding='utf-8'))
        answers += load_answers(da.get('answers') if isinstance(da, dict) else da, os.environ['FLOW_ANSWERS'])
    sj = json.dumps(script['steps'] or [])
    if ('$LOT' in sj and not lot) or ('$OP' in sj and not opr):
        raise SystemExit('FLOW_SCRIPT uses $LOT / $OP but lotId / operatorId were not given on the command line')
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
    # AI(W906-FLOW-2) 20260928: RULINGS_20260928 #2/#4 -- runs now use the laptop's own HT9050 config (no swap), so the
    #   ruling #41=B prep ("do not read last data": MachRec.bInitialStart byte 0 := 0, port cinitial.cpp:9377-9379 / golden
    #   :7632-7634) is applied to the LIVE system\machinerecord.dat here, after the baseline check; the end-of-run restore
    #   copies the snapshot's file back and the manifest re-check proves it.  In SWAPPED mode flow_swap.py does it instead.
    if not swapped and os.environ.get('FLOW_PREP_INITIALSTART') == '1':
        mr = os.path.join(ROOT, 'system', 'machinerecord.dat')
        b = bytearray(open(mr, 'rb').read())
        assert len(b) > 0 and b[0] in (0, 1), 'machinerecord.dat byte 0 is %r, not a bool' % (b[:1],)
        was = b[0]; b[0] = 0
        open(mr, 'wb').write(bytes(b))
        say('prep FLOW_PREP_INITIALSTART (live file, restored at the end): %s byte0 %02x -> 00' % (mr, was))
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
        ev.write(json.dumps({'t': rel(), 'wall': wall(), 'kind': kind, 'obj': obj}, ensure_ascii=False) + '\n'); ev.flush()

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
    # v2: the tags a script waits / anchors on -> every (time, value) that came through the stream
    watched = set(w['tag'] for w in [s.get('wait') or {} for s in (script['steps'] or [])] + script['snapshots'] if w.get('tag'))
    taghist = dict((k, []) for k in watched)
    arules = dict((r['id'], 0) for r in answers)   # scripted answers actually sent, per rule

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
                r = rule_for(answers, 'query', ' | '.join(str(x) for x in (j.get('code'), j.get('kind'), j.get('text')) if x)) if answers else None
                rid = None
                if r is not None and r['answer'] in (j.get('options') or []):
                    a, rid = r['answer'], r['id']
                    r['used'] += 1; arules[rid] += 1
                elif r is not None:
                    log('answer_refused', {'rule': r['id'], 'answer': r['answer'], 'options': j.get('options')})
                if a is not None:
                    nid[0] += 1
                    m = {'type': 'cmd', 'id': nid[0], 'cmd': 'modal.answer', 'tag': str(j.get('qid', '')), 'value': a}
                    log('answer', dict(m, rule=rid) if rid else m); ws.send_json(m)
            elif ty == 'modal':
                log('modal', j)
                q, a, rid, txt = mbox_scripted(mbans, answers)
                if rid and rid.startswith('refused:'):
                    log('answer_refused', {'rule': rid[8:], 'text': txt})
                    rid = None
                if not q:
                    q, a = mbox_choose(mbans)
                elif rid:
                    arules[rid] += 1
                if q:
                    mbans.add(q); nid[0] += 1
                    m = {'type': 'cmd', 'id': nid[0], 'cmd': 'dialog.response', 'tag': q, 'value': a}
                    log('answer', dict(m, rule=rid) if rid else m); ws.send_json(m)
            d = j.get('data')
            if isinstance(d, dict):
                for k, v in d.items():
                    if k.startswith(('pump.task.', 'guard.', 'machine.')):
                        latest[k] = v
                    if k in taghist:
                        taghist[k].append((time.time(), v))

    def wait_ack(i, sec):
        end = time.time() + sec
        while time.time() < end and i not in acks:
            pump(min(end, time.time() + 0.5))
        return acks.get(i)

    # ---- v2: snapshots (seconds checkpoints, anchors, script steps) and waits
    state = {'nexttl': 0.0, 'nextintr': 0.0, 'cpi': 0, 'armed': False}
    snaps_taken, fired, nextpoll = [], set(), {}

    def state_record():
        a = wait_ack(cmd('act.main.stateRecord', json.dumps({'taskListOnly': True})), 30)
        fold = (a or {}).get('folder') or ''
        src = os.path.join(fold, 'Task_ListWithTime.csv')
        return a, fold, (src if fold and os.path.isfile(src) else None)

    def keep_snap(name, fold, src):
        shutil.copy2(src, os.path.join(out, 'tasklist_%s.csv' % name))
        dv = os.path.join(fold, 'DecisionVariables.csv')
        if os.path.isfile(dv): shutil.copy2(dv, os.path.join(out, 'decision_%s.csv' % name))
        snaps_taken.append({'name': name, 't': rel(), 'wall': wall()})

    def snapshot(name):
        a, fold, src = state_record()
        say('stateRecord @%s' % name, json.dumps(a, ensure_ascii=False)[:300])
        if src:
            keep_snap(name, fold, src)
        else:
            say('  no Task_ListWithTime.csv at', os.path.join(fold, 'Task_ListWithTime.csv'))

    srdir = next((d for d in EXTRA if os.path.basename(d).lower() == 'ht9045_staterecord'), None)

    def task_seen(task, want, since_wall):
        """poll the task list once -> (met, last value, csv, folder).  A poll that does not meet the condition deletes its own
        folder right away (only a *_tasklist folder this run created under D:\\HT9045_StateRecord), so a long wait does not
        leave dozens of snapshots for the restore step to move."""
        a, fold, src = state_record()
        if not src:
            return False, None, None, fold
        rows, pinfo = parse_pairs(src)
        p = (rows.get(task) or {}).get('pairs') or []
        s0 = tms(since_wall)
        if pinfo['midnight'] and s0 < 12 * 3600000:     # parse_pairs moved the after-midnight entries to the next day
            s0 += 86400000
        met = bool(p) and (p[-1][2] in want or any(v in want and ms >= s0 for ms, t, v in p))
        nf = os.path.normpath(fold)
        if (not met and srdir and os.path.dirname(nf).lower() == srdir.lower() and os.path.basename(nf).endswith('_tasklist')
                and nf not in dirs_before.get(srdir, set()) and os.path.isdir(nf)):
            shutil.rmtree(nf, ignore_errors=True)
        return met, (p[-1][2] if p else None), src, fold

    def tag_seen(tagk, want, since):
        h = taghist.get(tagk, [])
        return any(v in want for tt, v in h if tt >= since) or (bool(h) and h[-1][1] in want), (h[-1][1] if h else None)

    def running():
        return not aborted and proc.poll() is None and ws.alive

    def tick():
        """housekeeping between pumps (never from inside pump): foreign wb_serve, timeline, checkpoints, anchor snapshots"""
        if not state['armed']:
            state['nexttl'], state['nextintr'], state['armed'] = time.time(), time.time(), True
        if time.time() >= state['nextintr']:
            intr = other_wb_serve(proc.pid)
            state['nextintr'] = time.time() + 5
            if intr:
                aborted.append(intr)
                say('ABORT: another wb_serve.exe appeared mid-run (pid %r) -- stopping; results of this run are not valid' % intr)
                proc.kill()
                return
        if time.time() >= state['nexttl']:
            tl.write(json.dumps({'t': rel(), 'tags': latest}, ensure_ascii=False) + '\n'); tl.flush(); state['nexttl'] = time.time() + 2
        if state['cpi'] < len(cps) and rel() >= cps[state['cpi']]:
            snapshot(str(cps[state['cpi']]))
            state['cpi'] += 1
        for sp in script['snapshots']:
            nm = sp['name']
            if nm in fired or not running():
                continue
            want = _want(sp)
            if sp.get('tag') and tag_seen(sp['tag'], want, t0)[0]:
                fired.add(nm); say('anchor %s: tag %s reached %s' % (nm, sp['tag'], want)); snapshot(nm)
            elif sp.get('task') and time.time() >= nextpoll.get(nm, 0):
                nextpoll[nm] = time.time() + float(sp.get('poll', 5))
                met, last, src, fold = task_seen(sp['task'], want, time.strftime('%H:%M:%S', time.localtime(t0)) + '.000')
                if met and src:
                    fired.add(nm); say('anchor %s: task %s reached %s' % (nm, sp['task'], want)); keep_snap(nm, fold, src)

    def do_wait(w, since=None, since_wall=None):
        """since / since_wall: when the step's command was SENT (values reached before its ack still count); the wait's own
        start when the step has no command.  The timeout counts from the wait's start."""
        want, tmo, poll = _want(w), float(w.get('timeout', 60)), float(w.get('poll', 5))
        t_start, np_ = time.time(), 0.0
        if since is None:
            since, since_wall = t_start, wall()
        t_end = t_start + tmo
        res = {'until': want, 'met': False, 'by': None, 'waited': 0.0, 'since_wall': since_wall}
        while running():
            if w.get('tag'):
                met, res['last_tag'] = tag_seen(w['tag'], want, since)
                if met:
                    res.update(met=True, by='tag')
                    break
            if w.get('task') and time.time() >= np_:
                np_ = time.time() + poll
                met, res['last_task'], src, fold = task_seen(w['task'], want, since_wall)
                if met:
                    res.update(met=True, by='task')
                    break
            if time.time() >= t_end:
                break
            pump(min(t_end, time.time() + 0.5)); tick()
        res['waited'] = round(time.time() - t_start, 1)
        if not res['met'] and not running():
            res['stopped'] = True
            say('  WAIT ended: wb_serve not running / run aborted')
        elif not res['met']:
            never = w.get('tag') and not taghist.get(w['tag'])
            say('  WAIT TIMEOUT after %.0f s (%s): %s' % (tmo, json.dumps(w, ensure_ascii=False),
                ('tag never published; ' if never else '') + 'last %r' % (res.get('last_task', res.get('last_tag')),)))
        return res

    def subst(v):
        return v.replace('$LOT', lot).replace('$OP', opr) if isinstance(v, str) else v

    results = []

    def dump_results():
        json.dump({'tag': tag, 't0_wall': time.strftime('%H:%M:%S', time.localtime(t0)), 'steps': results, 'snapshots': snaps_taken,
                   'answers_used': arules}, open(os.path.join(out, 'script_result.json'), 'w', encoding='utf-8'), ensure_ascii=False, indent=1)

    def run_steps(steps):
        for i, st in enumerate(steps):
            if not running():
                say('script stopped before step %d: wb_serve not running / run aborted' % i)
                break
            rec = {'i': i, 'cmd': st.get('cmd'), 'note': st.get('note', ''), 'start_wall': wall()}
            ok, sent = True, (None, None)
            if 'sleep' in st:
                end = time.time() + float(st['sleep'])
                while running() and time.time() < end:
                    pump(min(end, time.time() + 0.5)); tick()
            if st.get('cmd'):
                sent = (time.time(), wall())
                rec['sent_wall'], rec['sent_t'] = sent[1], rel()
                a = wait_ack(cmd(st['cmd'], subst(st.get('value')), subst(st.get('tag'))), float(st.get('ack_timeout', 20)))
                ok = bool(a) and bool(a.get('ok'))
                rec['ack_ok'], rec['ack'], rec['outcome'] = ok, a, ack_outcome(a)
                say('step %d %s' % (i, st['cmd']), json.dumps(a, ensure_ascii=False)[:300])
                if not ok:
                    say('  step %d: %s %s -- logged, script continues' % (i, st['cmd'], 'no ack' if a is None else 'ack failed'))
            if st.get('wait') and (ok or st.get('wait_even_if_failed') or not st.get('cmd')):
                rec['wait'] = do_wait(st['wait'], *sent)
            elif st.get('wait'):
                rec['wait'] = {'skipped': 'ack failed'}
            if st.get('snapshot') and running():
                snapshot(st['snapshot'])
                rec['snapshot'] = st['snapshot']
            results.append(rec)
            dump_results()

    try:
        for k in range(60):
            try:
                ws = WS(PORT); break
            except OSError:
                time.sleep(1)
        assert ws is not None, 'no WebSocket'
        say('ws up at', rel())
        say('acquire', json.dumps(wait_ack(cmd('control.acquire'), 10), ensure_ascii=False)[:200])
        if script['steps'] is not None:
            say('FLOW_SCRIPT %s: %d step(s), %d anchor snapshot(s), %d answer rule(s)' % (
                os.environ.get('FLOW_SCRIPT'), len(script['steps']), len(script['snapshots']), len(answers)))
            if os.environ.get('FLOW_START_MODE'):
                say('  FLOW_START_MODE ignored: the script decides the start mode')
            run_steps(script['steps'])
        else:
            if script['snapshots'] or answers:
                say('default sequence with %d anchor snapshot(s), %d answer rule(s)' % (len(script['snapshots']), len(answers)))
            smode = os.environ.get('FLOW_START_MODE', '')   # RSMODE: the reference operator picked "Initial Start" before HOME/START
            if smode:
                say('main.runStartMode', json.dumps(wait_ack(cmd('main.runStartMode', smode), 20), ensure_ascii=False)[:300])
            if lot:
                say('lot.start', json.dumps(wait_ack(cmd('lot.start', opr, lot), 20), ensure_ascii=False)[:300])
            say('start.run', json.dumps(wait_ack(cmd('start.run', 'web'), 60), ensure_ascii=False)[:300])
        while time.time() - t0 < total - 5 and running():
            pump(time.time() + 1.0)
            tick()
    finally:
        if ws is not None:
            ws.alive = False
        try:
            proc.wait(timeout=max(10, total - (time.time() - t0) + 30))
        except subprocess.TimeoutExpired:
            proc.kill(); proc.wait()
        fo.close(); ev.close(); tl.close()
        if script['steps'] is not None or script['snapshots'] or answers:
            dump_results()
            say('script: %d step(s) run, failed acks %d, wait timeouts %d, snapshots %s, scripted answers %s' % (
                len(results), sum(1 for r in results if r.get('cmd') and not r.get('ack_ok')),
                sum(1 for r in results if isinstance(r.get('wait'), dict) and r['wait'].get('met') is False),
                [s['name'] for s in snaps_taken], arules))
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
