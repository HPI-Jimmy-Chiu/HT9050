# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/s12_form_probe.py -- JSON 橋接層 S12 的 e2e probe（AI(W906-Q4-S126) 20260927 起只剩第二型：/api/form/ 列的頁）。
#
#  Steven 20260924.  規格：.claude/skills/ht9045-json-bridge/references/phases.md S12、
#  decisions.md 二之二（「DoIniDataToForm() 就等於是 C++ 發送 JSON 給 HTML」）。
#
#  用真的瀏覽器（headless Edge ＋ DevTools 協定）開頁面，驗三件事：
#    R  讀：/api/form/<Page> 的每個 widget，畫面上的值與它一致
#       （文字欄位依 C++ 的小數位數比；值相同時畫面必須是檔案原字串）
#    W  寫（--write 才做）：改一個欄位 → 引擎 save() → 檔案只差那一行
#    L  存檔後重讀：ack 帶 reload、/api/form 回新值（沒有這一步頁面會看到舊值）
#
#  ⚠ --write 會真的改配方檔。跑之前自己備份配方夾，跑完比對 SHA256 再還原。
#  ⚠ golden A02：AccessLevel 0 不能存（審查 M3 第 6 輪）→ --write 要帶 --user/--password（測試用密碼簿，
#    wb_serve 以 W906_PWBOOK_PATH 啟動），存檔前先登入。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\s12_form_probe.py --port 8046 [--page Setup.HotPlate.html]
#             [--write XST1=13.360 --user S12TEST --password S12PW]
#
#  Exit 0 = 全部通過；非 0 = 失敗項目數（看輸出）。
# =============================================================================
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake, send_text, read_frames   # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')

EDGE = [r'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe',
        r'C:\Program Files\Microsoft\Edge\Application\msedge.exe']
FAILS = []


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def ws_login(port, user, pw):
    sock, left = ws_handshake('127.0.0.1', port, '/ht9045', time.monotonic() + 5)
    frames = read_frames(sock, left, time.monotonic() + 30)

    def cmd(i, name, **kw):
        m = {'type': 'cmd', 'id': i, 'cmd': name}
        m.update(kw)
        send_text(sock, json.dumps(m))
        for op, p in frames:
            if op != 1:
                continue
            try:
                d = json.loads(p.decode('utf-8'))
            except Exception:
                continue
            if d.get('type') == 'ack' and d.get('id') == i:
                return d
    cmd(1, 'control.acquire')
    r = cmd(2, 'auth.login', tag=user, value=pw)
    if r and not r.get('ok') and 'already logged in' in (r.get('error') or ''):
        # 模擬組態開機就是 HonPrec／btLogin=Logout；golden btLoginClick：Caption=Logout 時按下是登出（WebLogin.cpp）
        cmd(4, 'auth.logout')
        time.sleep(0.5)                            # WebCmdGuard: a second auth.login within 400 ms of the first one is refused as busy (Q50 20260928)
        r = cmd(5, 'auth.login', tag=user, value=pw)
    cmd(3, 'control.release')                      # 讓頁面自己拿權杖
    return r and r.get('ok')


def http_json(url):
    return json.loads(urllib.request.urlopen(url, timeout=10).read().decode('utf-8'))


class Cdp(object):
    """最小的 DevTools 客戶端：只用 Page.navigate 與 Runtime.evaluate。"""

    def __init__(self, ws_url):
        hostport, path = ws_url[len('ws://'):].split('/', 1)
        host, port = hostport.split(':')
        self.sock, left = ws_handshake(host, int(port), '/' + path, time.monotonic() + 10)
        self.frames = read_frames(self.sock, left, time.monotonic() + 3600)
        self.nid = 0

    def call(self, method, params=None, timeout=20):
        self.nid += 1
        send_text(self.sock, json.dumps({'id': self.nid, 'method': method, 'params': params or {}}))
        end = time.monotonic() + timeout
        for op, payload in self.frames:
            if op != 1:
                continue
            m = json.loads(payload.decode('utf-8'))
            if m.get('id') == self.nid:
                return m
            if time.monotonic() > end:
                break
        raise RuntimeError('CDP %s: no reply' % method)

    def eval(self, expr, timeout=20):
        r = self.call('Runtime.evaluate', {'expression': expr, 'awaitPromise': True,
                                            'returnByValue': True}, timeout)
        res = r.get('result', {})
        if 'exceptionDetails' in res:
            raise RuntimeError('JS: %s' % json.dumps(res['exceptionDetails'])[:400])
        return res.get('result', {}).get('value')


class _EdgeProc(subprocess.Popen):
    """Steven 20260925：Edge 會自己再開 renderer／GPU／utility 子程序，Popen.kill() 只殺主程序 → 每跑一次探針留下一串
    無頭 Edge（20260925 清掉 836 個，最舊的是前一天）。kill／terminate 改成 taskkill /T 整棵樹；atexit 也會補殺（探針中途例外）。"""
    prof = ''

    def _tree_kill(self):
        # 主程序已結束時 /T 找不到子程序，所以不看 poll()，一律整棵樹砍（重複呼叫無害）
        subprocess.run(['taskkill', '/PID', str(self.pid), '/T', '/F'],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        # 20260925 實測：--headless=new 的啟動程序可能先退出，瀏覽器／renderer 不在這棵樹下 → 再依 profile 目錄收一次
        if self.prof:
            leaf = os.path.basename(self.prof.rstrip('\\/'))
            ps = ("Get-CimInstance Win32_Process -Filter \"Name='msedge.exe'\" | "
                  "Where-Object { $_.CommandLine -like '*%s*' } | "
                  "ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }" % leaf)
            subprocess.run(['powershell', '-NoProfile', '-Command', ps],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def kill(self):
        self._tree_kill()

    def terminate(self):
        self._tree_kill()


def launch_edge(dbg_port):
    import atexit
    exe = next((p for p in EDGE if os.path.exists(p)), None)
    if not exe:
        raise SystemExit('找不到 Microsoft Edge')
    prof = tempfile.mkdtemp(prefix='s12_edge_')
    p = _EdgeProc([exe, '--headless=new', '--disable-gpu', '--no-first-run',
                   '--remote-debugging-port=%d' % dbg_port, '--user-data-dir=' + prof,
                   'about:blank'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    p.prof = prof
    atexit.register(p._tree_kill)
    for _ in range(60):
        try:
            tabs = http_json('http://127.0.0.1:%d/json/list' % dbg_port)
            page = [t for t in tabs if t.get('type') == 'page']
            if page:
                return p, prof, page[0]['webSocketDebuggerUrl']
        except Exception:
            pass
        time.sleep(0.25)
    p.kill()
    raise SystemExit('Edge 的 DevTools 埠沒有起來')


def sameShown(file_text, cpp_text):
    a, b = str(file_text).strip(), str(cpp_text).strip()
    if a == b:
        return True
    try:
        fa, fb = float(a), float(b)
    except ValueError:
        return False
    d = len(b.split('.', 1)[1]) if '.' in b else 0
    return ('%.*f' % (d, fa)) == ('%.*f' % (d, fb))


# 在頁面裡讀出 widget 的現況。形狀與 /api/form 的 widgets 相同，方便逐項比。
READ_DOM = r"""
(async function () {
  for (var i = 0; i < 100; i++) {                       // 等引擎讀完檔＋覆蓋完
    if (window.HT9045Page && HT9045Page.form && HT9045Page.form()) break;
    await new Promise(function (r) { setTimeout(r, 100); });
  }
  await new Promise(function (r) { setTimeout(r, 800); }); // 舊 loader 可能較晚完成，它會再覆蓋一次
  var f = window.HT9045Page && HT9045Page.form && HT9045Page.form();
  if (!f) return {error: 'HT9045Page.form() 沒有值：引擎沒跑到 formOverlay'};
  var out = {};
  Object.keys(f.widgets || {}).forEach(function (id) {
    var el = document.getElementById(id), w = f.widgets[id], o = {};
    if (!el) { out[id] = {missing: true}; return; }
    if (w.text !== undefined && 'value' in el) o.text = el.value;
    if (w.checked !== undefined) {
      var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]');
      o.checked = c ? c.checked : null;
    }
    if (w.itemIndex !== undefined) {
      if (el.tagName === 'SELECT') o.itemIndex = el.selectedIndex;
      else { var rs = el.querySelectorAll('input[type="radio"]'); o.itemIndex = -1;
             for (var k = 0; k < rs.length; k++) if (rs[k].checked) o.itemIndex = k; }
    }
    o.src = el.getAttribute('data-src');
    out[id] = o;
  });
  return {form: f, dom: out};
})()
"""


def verify_read(cdp, base, page, label):
    print('[%s] %s' % (label, page))
    cdp.call('Page.navigate', {'url': base + '/page/' + page})
    r = cdp.eval(READ_DOM, timeout=30)
    if not r or 'error' in r:
        check(False, '頁面讀值：%s' % (r or {}).get('error', 'no result'))
        return None
    api = r['form']
    check(api.get('available') is True, 'C++ 表單 %s 在（%s；assigned %s / %s）'
          % (api.get('form'), api.get('kind'), api.get('assignedCount'), api.get('widgetCount')))
    # Steven 20260924 (S12 第二型)：讀檔端缺口 ⇒ 引擎不可拿結構初值蓋畫面，一個 data-src=cpp 都不該有。
    if api.get('kind') == 'golden-bridge' and api.get('sourceGap'):
        n_src = sum(1 for d in r['dom'].values() if d.get('src') == 'cpp')
        check(n_src == 0, 'sourceGap 存在 ⇒ 畫面沒有被 C++ 初值覆蓋（data-src=cpp %d 個）' % n_src)
        check(api.get('saveable') is False, 'sourceGap 存在 ⇒ saveable=false')
        return r
    bad = []
    for wid, w in api.get('widgets', {}).items():
        d = r['dom'].get(wid, {})
        if d.get('missing'):
            continue                                    # 頁面沒有這個 id：引擎另外回報，不在這裡判
        if 'text' in w and 'text' in d and not sameShown(d['text'], w['text']):
            bad.append('%s text 畫面=%r C++=%r' % (wid, d['text'], w['text']))
        if 'checked' in w and d.get('checked') is not None and d['checked'] != w['checked']:
            bad.append('%s checked 畫面=%r C++=%r' % (wid, d['checked'], w['checked']))
        # VCL 的 ItemIndex=-1＋Text＝清單外文字；頁面用「補一個該文字的選項並選取」表達，只比文字
        text_sel = w.get('itemIndex', 0) < 0 and 'text' in w
        if not text_sel and 'itemIndex' in w and 'itemIndex' in d and d['itemIndex'] != w['itemIndex'] and d['itemIndex'] != -1:
            bad.append('%s itemIndex 畫面=%r C++=%r' % (wid, d['itemIndex'], w['itemIndex']))
    check(not bad, '畫面值與 C++ DoIniDataToForm 一致（%d 個 widget）%s'
          % (len(api.get('widgets', {})), ('：' + '; '.join(bad[:8])) if bad else ''))
    n_src = sum(1 for d in r['dom'].values() if d.get('src') == 'cpp')
    check(n_src > 0, '有 widget 標上 data-src=cpp（%d 個）' % n_src)
    return r


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9333)
    ap.add_argument('--page', action='append')
    ap.add_argument('--write', help='<widgetId>=<新值>，只對第一個 --page 做')
    ap.add_argument('--user')
    ap.add_argument('--password')
    a = ap.parse_args()
    base = 'http://127.0.0.1:%d' % a.port
    pages = a.page or [f['page'] for f in http_json(base + '/api/form/')['forms']]
    print('伺服器 %s；頁面 %s' % (base, ', '.join(pages)))

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')
        for p in pages:
            verify_read(cdp, base, p, 'R')

        if a.write:
            if not (a.user and a.password):
                check(False, '[W] --write 需要 --user/--password（golden A02：Operator 不能存）')
                return len(FAILS)
            check(bool(ws_login(a.port, a.user, a.password)), '[W] 登入 %s（存檔前；golden A02）' % a.user)
            page = pages[0]
            wid, val = a.write.split('=', 1)
            print('[W] %s：%s = %s，走引擎的 save()' % (page, wid, val))
            before = http_json(base + '/api/form/' + page)['widgets'].get(wid, {}).get('text')
            js = r"""
(async function () {
  window.confirm = function () { return true; };      // 存檔確認框：probe 代按「確定」
  var el = document.getElementById(%s); el.value = %s;
  el.dispatchEvent(new Event('input', {bubbles: true}));
  el.dispatchEvent(new Event('change', {bubbles: true}));
  await HT9045Page.save();
  await new Promise(function (r) { setTimeout(r, 1500); });
  return {now: el.value};
})()
""" % (json.dumps(wid), json.dumps(val))
            r = cdp.eval(js, timeout=40)
            after = http_json(base + '/api/form/' + page)['widgets'].get(wid, {}).get('text')
            check(before is not None and sameShown(after, val),
                  '[L] 存檔後重讀：/api/form %s 從 %r 變成 %r（要 %r）' % (wid, before, after, val))
            check(r and sameShown(r.get('now'), val), '[L] 存檔後畫面 %s = %r' % (wid, (r or {}).get('now')))
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)
    print('s12_form_probe: %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
