# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/showmymessage_probe.py -- golden ShowMyMessage／MyMessageBox 在網頁上顯示並回答的 e2e
#
#  AI(W906-SMM) 20260925  Steven 20260925 指示：C++ 的 ShowMyMessage 在網頁上看不到，操作員會漏看
#  golden 的警告。這支用真的瀏覽器（headless Edge ＋ DevTools）證明：
#
#    A  background.html 殼層（dialog-bridge.js 畫）：ShowMyMessage（Pause）顯示、文字正確、
#       C++ 在等（echo 的 ack 還沒回、其他指令回 modal-pending）→ 按 pnlPause → C++ 解除等待、信箱 idle
#    B  background.html 殼層：ShowUnloaderTrayMessage 不停機型 —— 右下角小窗、C++ 不等（sys.ping 照常）、
#       按確認 → C++ 收尾（信箱 idle）
#    C  page/main.html 單獨開（ht9045_modal.js 自己畫）：ShowMyMessage(Ok=true) → 按鈕是 OK → 按下 → 解除
#    D  page/Setup.Speed.html 單獨開：ShowMyMessageBox_YES_NO（S2 帶 ';' 副訊息）→ 按 No → C++ 回 2；
#       再一次按 Yes → C++ 回 1；再來一則不停機型
#    E  關掉網頁再開也要跳出：沒開頁面時先發 ShowMyMessage，之後才開頁 → 框出現 → 答完解除
#
#  觸發器用既有的 sys.echoModal（tools/wb_serve.cpp；tag 選 pause／ok／yesno／nonstop），
#  它呼叫的是**真的** ShowMyMessage／ShowMyMessageBox_YES_NO／ShowUnloaderTrayMessage。
#  ⚠ 探針自己握單一操作員權杖（sys.echoModal 要），頁面搶不到權杖時的錯誤訊息是預期中的；
#    回答用的 dialog.response 免權杖（WebBridgeServer.cpp:1446），所以不受影響。
#  ⚠ 每一步都有時限：DevTools 呼叫逾時就換一條連線、JS alert／confirm 由探針自動關掉並記下來；
#    某一步沒答到時探針自己替它送回答，讓 C++ 離開等待，後面的步驟才不會連鎖失敗。
#
#  用法（由 scratchpad/run_showmymessage.sh 起 wb_serve 後呼叫）：
#      python tools/webprobe/showmymessage_probe.py --port 8046 [--dbg 9335]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import shutil
import socket
import struct
import sys
import threading
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake                            # noqa: E402
from s12_form_probe import launch_edge                        # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace', line_buffering=True)
FAILS = []


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)
    return bool(ok)


class JsonWs(object):
    """一條 WebSocket（JSON 文字訊框）：背景執行緒收訊框、回 pong；依 id 等回應。
    wb_serve 與 DevTools 都用它（兩邊都是 {"id":N,...} 對 {"id":N,...}）。"""

    def __init__(self, host, port, path, is_reply):
        self.sock, left = ws_handshake(host, port, path, time.monotonic() + 10)
        self.sock.settimeout(0.5)
        self.buf = left
        self.is_reply = is_reply
        self.replies = {}
        self.events = []
        self.on_event = None
        self.cv = threading.Condition()
        self.nid = 100
        self.alive = True
        self.lock = threading.Lock()
        threading.Thread(target=self._reader, daemon=True).start()

    def _send(self, opcode, data):
        mask = os.urandom(4)
        header = bytearray([0x80 | opcode])
        n = len(data)
        if n < 126:
            header.append(0x80 | n)
        elif n < 65536:
            header.append(0x80 | 126)
            header += struct.pack('>H', n)
        else:
            header.append(0x80 | 127)
            header += struct.pack('>Q', n)
        header += mask
        with self.lock:
            self.sock.sendall(bytes(header) + bytes(b ^ mask[i % 4] for i, b in enumerate(data)))

    def _reader(self):
        buf = self.buf
        while self.alive:
            while len(buf) >= 2:
                b0, b1 = buf[0], buf[1]
                ln, off = b1 & 0x7F, 2
                if ln == 126:
                    if len(buf) < 4:
                        break
                    ln, off = struct.unpack('>H', buf[2:4])[0], 4
                elif ln == 127:
                    if len(buf) < 10:
                        break
                    ln, off = struct.unpack('>Q', buf[2:10])[0], 10
                if len(buf) < off + ln:
                    break
                payload, buf = buf[off:off + ln], buf[off + ln:]
                op = b0 & 0x0F
                if op == 9:
                    try:
                        self._send(0xA, payload)
                    except OSError:
                        pass
                elif op == 1:
                    try:
                        m = json.loads(payload.decode('utf-8', 'replace'))
                    except ValueError:
                        continue
                    if self.is_reply(m):
                        with self.cv:
                            self.replies[m.get('id')] = m
                            self.cv.notify_all()
                    else:
                        self.events.append(m)
                        if self.on_event:
                            try:
                                self.on_event(m)
                            except Exception:                  # noqa: BLE001
                                pass
            try:
                c = self.sock.recv(1 << 20)
            except socket.timeout:
                continue
            except OSError:
                return
            if not c:
                return
            buf += c

    def send(self, obj):
        self.nid += 1
        obj = dict(obj)
        obj['id'] = self.nid
        self._send(1, json.dumps(obj, ensure_ascii=False).encode('utf-8'))
        return self.nid

    def wait(self, i, secs):
        end = time.monotonic() + secs
        with self.cv:
            while i not in self.replies:
                left = end - time.monotonic()
                if left <= 0:
                    return None
                self.cv.wait(left)
            return self.replies[i]

    def close(self):
        self.alive = False
        try:
            self.sock.close()
        except OSError:
            pass


# ---- wb_serve ----------------------------------------------------------------
class Serve(object):
    def __init__(self, port):
        self.ws = JsonWs('127.0.0.1', port, '/ht9045', lambda m: m.get('type') == 'ack')

    def send(self, cmd, **kw):
        m = {'type': 'cmd', 'cmd': cmd}
        m.update(kw)
        return self.ws.send(m)

    def ack(self, i, secs):
        return self.ws.wait(i, secs)

    def cmd(self, cmd, secs=15, **kw):
        return self.ack(self.send(cmd, **kw), secs)

    def modal_frames(self):
        return sum(1 for e in self.ws.events if e.get('type') == 'modal')


# ---- DevTools ----------------------------------------------------------------
PRELUDE = r"""(function(){ try {
  if (window.__probeLog) return; var L = window.__probeLog = [];
  function s(a){ try { return typeof a === 'string' ? a : JSON.stringify(a); } catch (e) { return String(a); } }
  ['error','warn'].forEach(function(k){ var o = console[k]; console[k] = function(){ try { L.push(k + ': ' + [].map.call(arguments, s).join(' ')); } catch (e) {} return o.apply(console, arguments); }; });
  window.alert = function(m){ L.push('alert: ' + m); };
  window.addEventListener('error', function(e){ L.push('onerror: ' + e.message + ' @' + (e.filename || '') + ':' + (e.lineno || '')); });
  window.addEventListener('unhandledrejection', function(e){ L.push('unhandledrejection: ' + s(e.reason && e.reason.message || e.reason)); });
} catch (e) {} })();"""


class Page(object):
    def __init__(self, target_ws):
        self.url = target_ws
        self.dialogs = []
        self._open()

    def _open(self):
        hostport, path = self.url[len('ws://'):].split('/', 1)
        host, port = hostport.split(':')
        self.ws = JsonWs(host, int(port), '/' + path, lambda m: 'id' in m and 'method' not in m)
        self.ws.on_event = self._event
        self.call('Page.enable', reopen=False)
        self.call('Runtime.enable', reopen=False)
        self.call('Page.addScriptToEvaluateOnNewDocument', {'source': PRELUDE}, reopen=False)

    def _event(self, m):
        if m.get('method') == 'Page.javascriptDialogOpening':
            p = m.get('params') or {}
            self.dialogs.append('%s: %s' % (p.get('type'), p.get('message')))
            self.ws.send({'method': 'Page.handleJavaScriptDialog', 'params': {'accept': True}})

    def call(self, method, params=None, secs=15, reopen=True):
        i = self.ws.send({'method': method, 'params': params or {}})
        r = self.ws.wait(i, secs)
        if r is None:
            print('  ⚠ DevTools %s 沒有在 %ds 內回應%s' % (method, secs, ' —— 換一條連線' if reopen else ''))
            if reopen:
                self.ws.close()
                self._open()
        return r

    def ev(self, expr, secs=10):
        r = self.call('Runtime.evaluate', {'expression': expr, 'awaitPromise': True, 'returnByValue': True}, secs)
        if not r:
            return None
        res = r.get('result', {})
        if 'exceptionDetails' in res:
            return None
        return res.get('result', {}).get('value')

    def goto(self, url):
        self.call('Page.navigate', {'url': url}, 20)


def http_json(url):
    return json.loads(urllib.request.urlopen(url + ('&' if '?' in url else '?') + '_=%d' % int(time.time() * 1000),
                                             timeout=10).read().decode('utf-8'))


def mailbox(base):
    try:
        return http_json(base + '/JSON/Message-dialog-request.json')
    except Exception as e:                                         # noqa: BLE001
        return {'state': 'unreadable', 'error': str(e)}


def wait(fn, secs, step=0.3):
    end = time.monotonic() + secs
    while time.monotonic() < end:
        try:
            v = fn()
        except Exception:                                          # noqa: BLE001
            v = None
        if v:
            return v
        time.sleep(step)
    return None


def mb_state(base, st):
    m = mailbox(base)
    return m if m.get('state') == st else None


def echo(srv, variant, s1, s2):
    # AI(W906-SCREEN-TOKEN) 20261001: background.html's hub takes the token when it connects and keeps it (RULINGS_20261001,
    #   "the newest screen wins"), so the probe takes it back right before each trigger (control.* is answered on the
    #   server's socket thread, also while a box is pending). sys.echoModal is not token-exempt.
    srv.cmd('control.takeover')
    return srv.send('sys.echoModal', tag=variant, value=json.dumps({'s1': s1, 's2': s2}, ensure_ascii=False))


# ---- 框的 DOM：兩種宿主各一組 --------------------------------------------------
DOCS = {
    'bridge':   "document.querySelector('#dialogBridge iframe[data-kind=\"message\"]')",
    'bridgeNS': "document.querySelector('#dialogNonStop iframe[data-kind=\"messageNonStop\"]')",
    'own':      "document.querySelector('#htMsgHost iframe')",
    'ownNS':    "document.querySelector('#htMsgNS iframe')",
}


def box_state(pg, which):
    return pg.ev(r"""(function(){
  try {
    var f = %s; var d = f && f.contentDocument; if (!d) return null;
    function t(id){ var el=d.getElementById(id); if(!el) return null; var c=el.querySelector('.pnlCap'); return (c?c.textContent:el.textContent); }
    function v(id){ var el=d.getElementById(id); return !!(el && d.defaultView.getComputedStyle(el).display!=='none'); }
    return {main:t('lblMainMsg'), zh:t('lblChineseMsg'), sub:t('lblSubMsg'), pause:t('pnlPause'), yes:t('pnlYes'), no:t('pnlNo'),
            pauseV:v('pnlPause'), yesV:v('pnlYes'), noV:v('pnlNo'), subV:v('lblSubMsg'), s1:t('nsS1'), s2:t('nsS2')};
  } catch (e) { return {err:String(e)}; }
})()""" % DOCS[which])


def click(pg, which, elid):
    return pg.ev("(function(){ var f=%s; var d=f&&f.contentDocument; var el=d&&d.getElementById(%s); if(!el) return false; el.click(); return true; })()"
                 % (DOCS[which], json.dumps(elid)))


def diag(pg, base, where):
    print('  ── 診斷（%s）' % where)
    print('     信箱：%s' % json.dumps({k: mailbox(base).get(k) for k in ('state', 'requestId', 'function', 'blocking')}, ensure_ascii=False))
    v = pg.ev(r"""(function(){ var o={};
  try{o.mode=window.HT9045Modal&&HT9045Modal.mode(); o.act=window.HT9045Modal&&HT9045Modal.active()&&HT9045Modal.active().requestId;}catch(e){}
  try{var a=HTDialogBridge.active(); o.bridgeActive=a&&{kind:a.kind,dk:a.displayKind,rid:a.request.requestId,sub:a.submitting}; o.q=HTDialogBridge.queues();
      var st=document.querySelector('#dialogBridge .dbWindow[data-kind="message"] .dbStatus'); o.status=st&&st.textContent; o.open=document.getElementById('dialogBridge').className;}catch(e){}
  try{o.hostStatus=[].map.call(document.querySelectorAll('.htmStatus'),function(x){return x.textContent;});}catch(e){}
  o.log=(window.__probeLog||[]).slice(-15);
  try{[].forEach.call(document.querySelectorAll('iframe'),function(f){ try{ var l=f.contentWindow.__probeLog; if(l&&l.length) (o.frames=o.frames||{})[f.getAttribute('data-kind')||f.getAttribute('data-base-src')||f.src]=l.slice(-6);}catch(e){} });}catch(e){}
  return JSON.stringify(o); })()""")
    print('     頁面：%s' % v)
    if pg.dialogs:
        print('     JS 對話框（探針已自動關掉）：%s' % pg.dialogs[-5:])


def recover(srv, base, eid, value):
    """這一步沒答到：探針自己送回答，讓 C++ 離開等待，後面的步驟才不會全部 modal-pending。"""
    if srv.ack(eid, 0.1) is not None:
        return
    for m in (mailbox(base), disk_mailbox(r'D:\HT9045\web')):
        if m.get('state') == 'pending' and m.get('blocking'):
            r = srv.cmd('dialog.response', secs=10, tag=m.get('requestId'), value=value)
            print('  ⚠ 探針代答 %s=%s 讓 C++ 離開等待：%s' % (m.get('requestId'), value, r))
            break
    srv.ack(eid, 10)


def disk_mailbox(web_root):
    try:
        return json.loads(open(os.path.join(web_root, 'JSON', 'runtime', 'Message-dialog-request.json'), 'rb').read().decode('utf-8'))
    except Exception:                                              # noqa: BLE001
        return {}


def unblock(srv, base, web_root, why):
    """C++ 還停在某一則阻塞框上 ⇒ 探針自己送 dialog.response，讓 wb_serve 離開等待（不可以讓它永遠等）。
    信箱先看 HTTP，再看磁碟（HTTP 那一層若有問題，磁碟上的 requestId 才是 C++ 真的在等的那一則）。"""
    for m in (mailbox(base), disk_mailbox(web_root)):
        if m.get('state') == 'pending' and m.get('blocking'):
            value = 'NO' if m.get('function') == 'ShowMyMessageBox_YES_NO' else 'PAUSE'
            try:
                r = srv.cmd('dialog.response', secs=5, tag=m.get('requestId'), value=value)
            except Exception as e:                                 # noqa: BLE001
                r = str(e)
            print('  ⚠ [%s] 探針代答 %s=%s，讓 C++ 離開等待：%s' % (why, m.get('requestId'), value, r))
            return


def arm_deadline(secs, srv, base, web_root, edge_box):
    """整支探針的時限。到了就先代答任何阻塞框、關掉 Edge，然後以非 0 結束 ——
    run_showmymessage.sh 接著會 kill wb_serve 並 restore，鎖一定會放掉。"""
    def fire():
        print('')
        print('  ⚠⚠ 探針總時限 %ds 到了 —— 代答阻塞框、收掉 Edge、結束' % secs)
        try:
            unblock(srv, base, web_root, 'deadline')
        except Exception:                                          # noqa: BLE001
            pass
        try:
            if edge_box:
                edge_box[0].kill()
        except Exception:                                          # noqa: BLE001
            pass
        sys.stdout.flush()
        os._exit(98)
    t = threading.Timer(secs, fire)
    t.daemon = True
    t.start()
    return t


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9335)
    ap.add_argument('--mailbox-only', action='store_true', help='不開瀏覽器，只驗 C++ → 信箱 → HTTP')
    ap.add_argument('--web-root', default=r'D:\HT9045\web')
    ap.add_argument('--deadline', type=int, default=200, help='整支探針的時限（秒）；wb_serve 以 --seconds 300 起，要留時間給 restore')
    a = ap.parse_args()
    base = 'http://127.0.0.1:%d' % a.port
    print('伺服器 %s' % base)

    mb0 = mailbox(base)
    check(mb0.get('state') == 'idle', '[0] 開始前 Message 信箱是 idle（state=%s）' % mb0.get('state'))

    srv = Serve(a.port)
    r = srv.cmd('control.takeover')   # AI(W906-SCREEN-TOKEN) 20261001: takeover -- an HMI screen may already hold it
    if not check(bool(r and r.get('ok')), '[0] 探針取得單一操作員權杖（sys.echoModal 要）：%s' % (r or {}).get('error')):
        return len(FAILS)
    edge_box = []
    arm_deadline(a.deadline, srv, base, a.web_root, edge_box)

    if a.mailbox_only:
        # 不開瀏覽器：只驗 C++ → 信箱 → HTTP 這一段，再由探針自己回答（排查「網頁看不到」用）
        eid = echo(srv, 'pause', 'SMM-0: mailbox only', '只驗信箱')
        time.sleep(2.0)
        for u in ('/JSON/Message-dialog-request.json', '/JSON/runtime/Message-dialog-request.json'):
            try:
                raw = urllib.request.urlopen(base + u + '?_=%d' % int(time.time() * 1000), timeout=5).read()
                print('  GET %s -> %s' % (u, raw[:160].decode('utf-8', 'replace')))
            except Exception as e:                                 # noqa: BLE001
                print('  GET %s -> %s' % (u, e))
        disk = os.path.join(a.web_root, 'JSON', 'runtime', 'Message-dialog-request.json')
        try:
            print('  disk %s -> %s' % (disk, open(disk, 'rb').read()[:160].decode('utf-8', 'replace')))
        except OSError as e:
            print('  disk %s -> %s' % (disk, e))
        m = mailbox(base)
        check(m.get('state') == 'pending', '[M] HTTP /JSON/Message-dialog-request.json 看得到 pending（state=%s）' % m.get('state'))
        unblock(srv, base, a.web_root, 'mailbox-only')
        check(srv.ack(eid, 10) is not None, '[M] 代答後 C++ 解除等待')
        srv.cmd('control.release', secs=3)
        print('\nshowmymessage_probe (mailbox-only): %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
        return len(FAILS)

    edge, prof, cdp_ws = launch_edge(a.dbg)
    edge_box.append(edge)
    try:
        pg = Page(cdp_ws)

        # ================================================================ A
        print('\n[A] background.html 殼層（dialog-bridge.js）—— ShowMyMessage，按鈕 Pause')
        pg.goto(base + '/background.html?mode=release')
        ready = wait(lambda: pg.ev("(function(){var f=%s; return !!(window.HT9045Modal && HT9045Modal.mode()==='bridge' && window.HTDialogHost"
                                   " && f && f.contentWindow && f.contentWindow.HTDialogPage);})()" % DOCS['bridge']), 90, 0.5)
        if not check(bool(ready), '[A] 殼層載入完成，ht9045_modal.js 判定交給 dialog-bridge（mode=%s）' % pg.ev("window.HT9045Modal && HT9045Modal.mode()")):
            diag(pg, base, 'A 載入')
        s1, s2 = 'SMM-A: Tray is not ready, please check.', '中文訊息 A：Tray 尚未就緒，請檢查。'
        f0 = srv.modal_frames()
        eid = echo(srv, 'pause', s1, s2)
        mb = wait(lambda: mb_state(base, 'pending'), 10)
        check(bool(mb) and mb.get('function') == 'ShowMyMessage' and mb.get('blocking') is True
              and str(mb.get('requestId', '')).startswith('msg-'),
              '[A] C++ 寫了 Message 信箱：pending／ShowMyMessage／blocking／requestId=%s' % (mb or {}).get('requestId'))
        check(wait(lambda: srv.modal_frames() > f0, 5), '[A] WS 也廣播了 modal 訊框（觸發用）')
        st = wait(lambda: (lambda s: s if (s and s.get('main') == s1) else None)(box_state(pg, 'bridge')), 15)
        shown = pg.ev("(function(){var v=document.getElementById('dialogBridge');var w=v&&v.querySelector('.dbWindow[data-kind=\"message\"]');"
                      "return !!(v && v.classList.contains('open') && w && w.classList.contains('show'));})()")
        check(bool(shown), '[A] #dialogBridge 開著、Message 視窗顯示')
        check(bool(st) and st.get('zh') == s2, '[A] 文字：main=%r zh=%r' % ((st or {}).get('main'), (st or {}).get('zh')))
        check(bool(st) and (st.get('pause') or '').strip() == 'Pause' and st.get('pauseV') and not st.get('yesV') and not st.get('noV'),
              '[A] 按鈕：只有 pnlPause＝Pause（golden :835 Ok=false）')
        check(srv.ack(eid, 0.5) is None, '[A] C++ 在等：sys.echoModal 的 ack 還沒回（golden ShowModal）')
        r = srv.cmd('sys.ping', secs=5)
        check(bool(r) and r.get('ok') is False and 'modal-pending' in (r.get('error') or ''),
              '[A] 等待中其他指令回 modal-pending：%s' % r)
        click(pg, 'bridge', 'pnlPause')
        r = srv.ack(eid, 15)
        if not check(bool(r and r.get('ok')), '[A] 按 pnlPause → C++ 解除等待（echo ack=%s）' % r):
            diag(pg, base, 'A 回答')
            recover(srv, base, eid, 'PAUSE')
        check(wait(lambda: pg.ev("!document.getElementById('dialogBridge').classList.contains('open')"), 10), '[A] 框關掉了')
        check(wait(lambda: mb_state(base, 'idle'), 5), '[A] 信箱退役成 idle（F5 不會再彈）')

        # ================================================================ B
        print('\n[B] background.html 殼層 —— ShowUnloaderTrayMessage 不停機型')
        s1, s2 = 'SMM-B: Auto1 is full with trays, Please take it off.', 'Auto1 上的Tray盤已滿,請取下Tray盤'
        eid = echo(srv, 'nonstop', s1, s2)
        r = srv.ack(eid, 10)
        check(bool(r and r.get('ok')) and str((r or {}).get('modeless', '')).startswith('msg-'),
              '[B] C++ 不等：ack 立刻回、非阻塞框 %s' % (r or {}).get('modeless'))
        mb = mailbox(base)
        check(mb.get('state') == 'pending' and mb.get('blocking') is False
              and (mb.get('requestedSideEffects') or {}).get('stopAllMotor') is False,
              '[B] 信箱：pending／blocking=false／stopAllMotor=false（iUnLoaderCount=8 不停機）')
        ns = wait(lambda: (lambda s: s if (s and s.get('s1') == s1) else None)(box_state(pg, 'bridgeNS')), 15)
        if not check(bool(ns) and ns.get('s2') == s2, '[B] 不停機小窗顯示：s1=%r s2=%r' % ((ns or {}).get('s1'), (ns or {}).get('s2'))):
            diag(pg, base, 'B 顯示')
        r = srv.cmd('sys.ping', secs=5)
        check(bool(r and r.get('ok')), '[B] C++ 沒有停在等待：sys.ping 照常 ok')
        click(pg, 'bridgeNS', 'nsOk')
        if not check(wait(lambda: mb_state(base, 'idle'), 10), '[B] 按確認 → C++ 收尾（FormClose），信箱 idle'):
            diag(pg, base, 'B 回答')
        check(wait(lambda: pg.ev("!document.getElementById('dialogNonStop').classList.contains('open')"), 5), '[B] 小窗關掉了')

        # ================================================================ C
        print('\n[C] page/main.html 單獨開（ht9045_modal.js 自己畫）—— ShowMyMessage(Ok=true)')
        pg.goto(base + '/page/main.html?mode=release')
        ready = wait(lambda: pg.ev("(function(){var f=%s; return !!(window.HT9045Modal && HT9045Modal.mode()==='own' && f && f.contentWindow"
                                   " && f.contentWindow.HTDialogPage);})()" % DOCS['own']), 30, 0.5)
        if not check(bool(ready), '[C] main.html：ht9045_modal.js 自己畫（mode=%s）' % pg.ev("window.HT9045Modal && HT9045Modal.mode()")):
            diag(pg, base, 'C 載入')
        s1, s2 = 'SMM-C: Lot end finished, press OK.', '批次結束，請按 OK。'
        eid = echo(srv, 'ok', s1, s2)
        st = wait(lambda: (lambda s: s if (s and s.get('main') == s1) else None)(box_state(pg, 'own')), 15)
        check(pg.ev("document.getElementById('htMsgHost').classList.contains('open')"), '[C] 遮罩開著（golden modal）')
        if not check(bool(st) and st.get('zh') == s2 and (st.get('pause') or '').strip() == 'OK',
                     '[C] 文字與按鈕：main=%r zh=%r pnlPause=%r（golden :833 Ok=true → OK）'
                     % ((st or {}).get('main'), (st or {}).get('zh'), (st or {}).get('pause'))):
            diag(pg, base, 'C 顯示')
        check(srv.ack(eid, 0.5) is None, '[C] C++ 在等')
        click(pg, 'own', 'pnlPause')
        r = srv.ack(eid, 15)
        if not check(bool(r and r.get('ok')), '[C] 按 OK → C++ 解除等待'):
            diag(pg, base, 'C 回答')
            recover(srv, base, eid, 'OK')
        check(wait(lambda: pg.ev("!document.getElementById('htMsgHost').classList.contains('open')"), 5), '[C] 框關掉了')

        # ================================================================ D
        print('\n[D] page/Setup.Speed.html 單獨開 —— ShowMyMessageBox_YES_NO 與不停機型')
        pg.goto(base + '/page/Setup.Speed.html?mode=release')
        ready = wait(lambda: pg.ev("(function(){var f=%s; return !!(window.HT9045Modal && HT9045Modal.mode()==='own' && f && f.contentWindow"
                                   " && f.contentWindow.HTDialogPage);})()" % DOCS['own']), 30, 0.5)
        if not check(bool(ready), '[D] Setup.Speed.html：ht9045_modal.js 自己畫'):
            diag(pg, base, 'D 載入')
        s1, s2 = 'SMM-D: Save the speed setting?', '確定要儲存速度設定？;存檔後立即生效'
        eid = echo(srv, 'yesno', s1, s2)
        st = wait(lambda: (lambda s: s if (s and s.get('main') == s1) else None)(box_state(pg, 'own')), 15)
        check(mailbox(base).get('function') == 'ShowMyMessageBox_YES_NO', '[D] 信箱 function=ShowMyMessageBox_YES_NO')
        check(bool(st) and st.get('yesV') and st.get('noV') and not st.get('pauseV')
              and (st.get('yes') or '').strip() == 'Yes' and (st.get('no') or '').strip() == 'No',
              '[D] 按鈕：pnlYes／pnlNo 顯示、pnlPause 隱藏（golden :1045-1047）：%s' % st)
        check(bool(st) and st.get('zh') == '確定要儲存速度設定？' and st.get('subV') and st.get('sub') == '存檔後立即生效',
              '[D] S2 以 ; 拆成中文訊息＋副訊息（golden :1052-1062）：zh=%r sub=%r' % ((st or {}).get('zh'), (st or {}).get('sub')))
        check(srv.ack(eid, 0.5) is None, '[D] C++ 在等 YES/NO')
        click(pg, 'own', 'pnlNo')
        r = srv.ack(eid, 15)
        if not check(bool(r and r.get('ok')) and (r or {}).get('returned') == 2,
                     '[D] 按 No → ShowMyMessageBox_YES_NO 回 2（golden pnlNo Tag=2）：%s' % r):
            diag(pg, base, 'D No')
            recover(srv, base, eid, 'NO')
        wait(lambda: pg.ev("!document.getElementById('htMsgHost').classList.contains('open')"), 5)
        eid = echo(srv, 'yesno', 'SMM-D2: Initial Start???', '初始化')
        st = wait(lambda: (lambda s: s if (s and s.get('main') == 'SMM-D2: Initial Start???') else None)(box_state(pg, 'own')), 15)
        check(bool(st) and not st.get('subV') and st.get('zh') == '初始化', '[D] 沒有 ; 的 S2 不顯示副訊息（上一則的排版有還原）')
        click(pg, 'own', 'pnlYes')
        r = srv.ack(eid, 15)
        if not check(bool(r and r.get('ok')) and (r or {}).get('returned') == 1, '[D] 按 Yes → 回 1（golden pnlYes Tag=1）：%s' % r):
            diag(pg, base, 'D Yes')
            recover(srv, base, eid, 'YES')
        wait(lambda: pg.ev("!document.getElementById('htMsgHost').classList.contains('open')"), 5)
        s1, s2 = 'SMM-D3: Color Tray is full with trays, Please take it off', 'Color上的Tray盤已滿,請取下Tray盤'
        eid = echo(srv, 'nonstop', s1, s2)
        r = srv.ack(eid, 10)
        check(bool(r and r.get('ok')), '[D] 不停機型：C++ 不等')
        ns = wait(lambda: (lambda s: s if (s and s.get('s1') == s1) else None)(box_state(pg, 'ownNS')), 15)
        if not check(bool(ns) and ns.get('s2') == s2 and not pg.ev("document.getElementById('htMsgHost').classList.contains('open')"),
                     '[D] 右下角小窗顯示、沒有遮罩（不擋畫面）：%s' % ((ns or {}).get('s1'))):
            diag(pg, base, 'D 不停機')
        click(pg, 'ownNS', 'nsOk')
        check(wait(lambda: mb_state(base, 'idle'), 10), '[D] 按確認 → C++ 收尾，信箱 idle')

        # ================================================================ E
        print('\n[E] 關掉網頁再開也要跳出（沒開頁面時先發 ShowMyMessage）')
        pg.goto('about:blank')
        time.sleep(1.0)
        s1, s2 = 'SMM-E: Message raised while no page was open.', '沒開網頁時發出的訊息'
        eid = echo(srv, 'pause', s1, s2)
        check(wait(lambda: mb_state(base, 'pending'), 10), '[E] 信箱 pending')
        time.sleep(1.0)
        pg.goto(base + '/page/Setup.Speed.html?mode=release')
        st = wait(lambda: (lambda s: s if (s and s.get('main') == s1) else None)(box_state(pg, 'own')), 30, 0.5)
        if not check(bool(st) and st.get('zh') == s2, '[E] 開頁後框出現：%s' % ((st or {}).get('main'))):
            diag(pg, base, 'E 顯示')
        check(srv.ack(eid, 0.2) is None, '[E] C++ 一直在等')
        click(pg, 'own', 'pnlPause')
        r = srv.ack(eid, 15)
        if not check(bool(r and r.get('ok')), '[E] 按 Pause → 解除等待'):
            diag(pg, base, 'E 回答')
            recover(srv, base, eid, 'PAUSE')
        check(wait(lambda: mb_state(base, 'idle'), 5), '[E] 信箱 idle')
        if pg.dialogs:
            print('  ⓘ 過程中頁面跳過的 JS 對話框（探針自動關掉）：%s' % pg.dialogs)
    finally:
        unblock(srv, base, a.web_root, 'finally')
        try:
            srv.cmd('control.release', secs=3)
        except Exception:                                          # noqa: BLE001
            pass
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)
    print('\nshowmymessage_probe: %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
