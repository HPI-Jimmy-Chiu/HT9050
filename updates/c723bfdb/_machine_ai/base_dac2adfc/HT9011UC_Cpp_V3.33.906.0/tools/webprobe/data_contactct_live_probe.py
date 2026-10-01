# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_contactct_live_probe.py -- Data.ContactCT.html 即時更新（S115）的 e2e probe。
#
#  AI(W906-FRW-S115) 20260927 [W906] Steven 團隊。RULINGS_20260926 S115；S124＝B（唯讀 *.get 免權杖，
#  St02 9d790ff2：WebBridge/WebBridgeServer.cpp:1448）。
#
#  golden 對照：TfContactCT 沒有 Timer；別人叫 fContactCT->sgYield->Refresh() 重畫 —— 每測完一次
#  （ProcessCount 結尾 atester_ProcessCount.cpp:1999）、各種清除之後（cSortCT.cpp:694／:866、main.cpp:35349…）。
#  TfMain::Timer1Timer（main.cpp:3222-3233，30 ms）寫 btClearCount->Enabled = !SystemStart。
#  網頁照 S124「可能一秒鐘就更新一次」每 1000 ms 送 contactct.get {"yieldType":<正在看的那一項>}。
#
#  驗（開開頁之前先用一條從來不拿權杖的 WS 連線）：
#    R  沒拿權杖的連線：contactct.get（不帶 value＝FormShow）與 {"yieldType":3} 都成功（不是 not-operator）；
#       回應帶 enabled.btClearCount（布林），且＝ !guard.systemStart（該 tag 有值時）
#    T  開頁後 4.2 秒內 ticks 增加 ≥3、loads 不動（tick 不算開頁／點選）、沒有 tickError；畫面＝最新回應
#    K  另一條連線拿著權杖不放的 3.2 秒內，頁面照樣每秒更新（ticks +≥2、沒有 tokenFallback）；
#       頁面本身不佔權杖（另一條連線拿得到 control.acquire）
#    S  點 History（0）之後，tick 送的是 0：3.2 秒後 data.itemIndex 還是 0、選中的 radio 還是 0
#       （--serve-log 有給：wb_serve 主控台的 "contactct.get yieldType=0 -> ok" 那段期間增加 ≥2 行）
#    N  按 Count Clear 顯示「尚未接」，過兩拍之後狀態列還是那一句（tick 不蓋掉）；按鈕 disabled＝回應 enabled
#    H  把頁面放進同源 iframe：iframe display:none 的 3.2 秒內 ticks 最多 +1（送出中那一條），
#       顯示回來之後 2.2 秒內 ticks +≥1
#    V  （--values）最新一次 tick 的數值＝golden 公式從 system\Arm*.dat 算（沿用 data_contactct_probe 的 verify_values；
#       只在機台沒有在跑時有意義：記憶體裡的計數每測完一次就變，.dat 要等 WriteCTInfo 才寫）
#    W  整段期間 system\Arm*.dat SHA256 不變（只讀）
#
#  只讀，不寫任何檔。本探針 20260927 寫成時沒有執行過（交件規則：不 build、不跑 wb_serve）。
#  用法：
#      set W906_PWBOOK_PATH=<scratch book>  &  build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046 > serve.log
#      python tools\webprobe\data_contactct_live_probe.py --port 8046 [--user S12TEST --password S12PW]
#             [--serve-log serve.log] [--values]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import re
import shutil
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402
from cmd_probe import ws_handshake, send_text, read_frames     # noqa: E402
import data_contactct_probe as base                             # noqa: E402  compare_dom／verify_values／read_arm／sha_arm

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = base.FAILS            # 同一張失敗表（verify_values／compare_dom 會寫進 base.FAILS）
check = base.check
PAGE = 'Data.ContactCT.html'


class RawWs(object):
    """一條自己的 WS 連線（不經過頁面）：用來證明「沒拿權杖也查得到」與「拿著權杖不放」。"""

    def __init__(self, port):
        self.sock, left = ws_handshake('127.0.0.1', port, '/ht9045', time.monotonic() + 5)
        self.frames = read_frames(self.sock, left, time.monotonic() + 900)
        self.nid = 0
        self.snap = None

    def cmd(self, name, timeout=15, **kw):
        self.nid += 1
        m = {'type': 'cmd', 'id': self.nid, 'cmd': name}
        m.update(kw)
        send_text(self.sock, json.dumps(m))
        end = time.monotonic() + timeout
        for op, p in self.frames:
            if op != 1:
                continue
            try:
                d = json.loads(p.decode('utf-8', 'replace'))
            except ValueError:
                continue
            if d.get('type') == 'snapshot' and self.snap is None:
                self.snap = dict(d.get('data', {}))
            if d.get('type') == 'ack' and d.get('id') == self.nid:
                return d
            if time.monotonic() > end:
                break
        return None

    def close(self):
        try:
            self.sock.close()
        except Exception:
            pass


def unwrap(ack):
    if ack and isinstance(ack.get('value'), str):
        try:
            j = json.loads(ack['value'])
            if isinstance(j, dict):
                return j
        except ValueError:
            pass
    return ack


def tag(snap, name):
    v = (snap or {}).get(name)
    if isinstance(v, dict) and 'v' in v:
        v = v['v']
    return v


def state(cdp, where='window'):
    return json.loads(cdp.eval("JSON.stringify((function(){var s=%s.__contactct||{};return {loads:s.loads,ticks:s.ticks,"
                               "same:s.same,renders:s.renders,skipped:s.skipped,tokenFallback:s.tokenFallback,"
                               "error:s.error,tickError:s.tickError,itemIndex:(s.data&&s.data.rgYieldType)?"
                               "s.data.rgYieldType.itemIndex:null};})())" % where) or '{}')


def serve_log_count(path, yt):
    if not path or not os.path.exists(path):
        return None
    pat = re.compile(r'^contactct\.get yieldType=%d -> ok' % yt)
    with open(path, 'rb') as f:
        txt = f.read().decode('utf-8', 'replace')
    return sum(1 for line in txt.splitlines() if pat.match(line))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9338)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--serve-log', help='wb_serve 主控台輸出的檔（有給才驗 S 的伺服器端行數）')
    ap.add_argument('--values', action='store_true', help='V：最新 tick 的數值對 system\\Arm*.dat（機台沒在跑時才用）')
    a = ap.parse_args()

    sha0 = base.sha_arm()
    if a.user:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    src = urllib.request.urlopen('http://127.0.0.1:%d/page/ht9045_contactct_wire.js' % a.port, timeout=10).read().decode('utf-8')
    check('TICK_MS = 1000' in src and 'AI(W906-FRW-S115)' in src, 'F：伺服器送出的 ht9045_contactct_wire.js 是 S115 版（每 1000 ms）')

    # ---- R：沒拿權杖的連線 ------------------------------------------------------------------------
    rw = RawWs(a.port)
    try:
        r1 = rw.cmd('contactct.get')
        check(bool(r1 and r1.get('ok')), 'R：沒拿權杖的連線 contactct.get（不帶 value）成功（%s）'
              % (None if not r1 else (r1.get('error') or 'ok')))
        r2 = rw.cmd('contactct.get', value=json.dumps({'yieldType': 3}))
        check(bool(r2 and r2.get('ok')), 'R：沒拿權杖的連線 contactct.get {"yieldType":3} 成功（%s）'
              % (None if not r2 else (r2.get('error') or 'ok')))
        d2 = unwrap(r2) if r2 and r2.get('ok') else {}
        en = (d2.get('enabled') or {}).get('btClearCount')
        check(isinstance(en, bool), 'R：回應帶 enabled.btClearCount（布林；golden main.cpp:3222-3233）＝%r' % en)
        ss = tag(rw.snap, 'guard.systemStart')
        if isinstance(ss, bool) and isinstance(en, bool):
            check(en == (not ss), 'R：enabled.btClearCount（%r）＝ !guard.systemStart（%r）' % (en, ss))
        else:
            print('  NOTE  guard.systemStart 沒有值（%r；pump 沒有 ARMED 時是 null），R 的 !SystemStart 比對略過' % ss)
    finally:
        rw.close()

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Runtime.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (a.port, PAGE)})
        ok = wait_js(cdp, "window.__contactct && window.__contactct.loads>=1 && (window.__contactct.data||window.__contactct.error) ? 1 : 0", 60)
        check(bool(ok), '開頁送出 contactct.get 並收到回應')
        s0 = state(cdp)
        if s0.get('error') or s0.get('itemIndex') is None:
            check(False, '開頁 contactct.get 成功（%s）' % s0.get('error'))
            return len(FAILS)
        check(s0['itemIndex'] == 3, '開頁 itemIndex＝golden FormShow :73 的 3（%s）' % s0['itemIndex'])

        # ---- T：每秒一拍 ------------------------------------------------------------------------------
        time.sleep(4.2)
        s1 = state(cdp)
        check(s1['ticks'] - s0['ticks'] >= 3, 'T：4.2 秒內 ticks %d -> %d（每 1000 ms 一拍）' % (s0['ticks'], s1['ticks']))
        check(s1['loads'] == s0['loads'], 'T：tick 不算 loads（%d -> %d）' % (s0['loads'], s1['loads']))
        check(not s1.get('tickError'), 'T：沒有 tickError（%s）' % s1.get('tickError'))
        print('  info  ticks=%d same=%d renders=%d skipped=%d' % (s1['ticks'], s1['same'], s1['renders'], s1['skipped']))
        d = json.loads(cdp.eval("JSON.stringify(window.__contactct.data)"))
        base.compare_dom(cdp, d, 'T 最新回應')

        # ---- K：權杖在別人手上照樣更新；頁面自己不佔權杖 ----------------------------------------------
        hold = RawWs(a.port)
        try:
            h = hold.cmd('control.acquire')
            got = bool(h and h.get('ok'))
            check(got, 'K：頁面開著時另一條連線拿得到 control 權杖（頁面不佔權杖）（%s）'
                  % (None if not h else (h.get('error') or 'ok')))
            k0 = state(cdp)
            time.sleep(3.2)
            k1 = state(cdp)
            check(k1['ticks'] - k0['ticks'] >= 2 and not k1.get('tickError'),
                  'K：權杖在別的連線手上的 3.2 秒內頁面照樣更新（ticks %d -> %d，tickError=%s）'
                  % (k0['ticks'], k1['ticks'], k1.get('tickError')))
            check(k1['tokenFallback'] == 0, 'K：頁面沒有走舊伺服器的 acquire→get→release（tokenFallback=%s）' % k1['tokenFallback'])
            if got:
                hold.cmd('control.release')
        finally:
            hold.close()

        # ---- S：點 History（0），tick 送的是 0 ---------------------------------------------------------
        before = state(cdp)
        n0 = serve_log_count(a.serve_log, 0)
        cdp.eval("(function(){var r=document.querySelectorAll('#rgYieldType input')[0]; r.click(); return 1;})()")
        ok = wait_js(cdp, "window.__contactct.loads>%d && window.__contactct.data.rgYieldType.itemIndex==0 ? 1 : 0" % before['loads'], 30)
        check(bool(ok), 'S：點 History -> 重取（loads %d -> %s）' % (before['loads'], state(cdp)['loads']))
        t0 = state(cdp)['ticks']
        time.sleep(3.2)
        s2 = state(cdp)
        chk = cdp.eval("[].map.call(document.querySelectorAll('#rgYieldType input'),function(i){return i.checked;}).indexOf(true)")
        check(s2['ticks'] - t0 >= 2 and s2['itemIndex'] == 0 and chk == 0,
              'S：之後的 tick 維持 History（ticks %d -> %d，itemIndex=%s，選中=%s）' % (t0, s2['ticks'], s2['itemIndex'], chk))
        n1 = serve_log_count(a.serve_log, 0)
        if n0 is not None:
            check(n1 - n0 >= 3, 'S：wb_serve 主控台 "contactct.get yieldType=0 -> ok" %d -> %d（點一下＋每拍一行）' % (n0, n1))
        d = json.loads(cdp.eval("JSON.stringify(window.__contactct.data)"))
        base.compare_dom(cdp, d, 'S History tick')
        # 回到 Kind(%)，讓後面的 V 跟開頁同一個模式
        loads = s2['loads']
        cdp.eval("(function(){var r=document.querySelectorAll('#rgYieldType input')[3]; r.click(); return 1;})()")
        wait_js(cdp, "window.__contactct.loads>%d && window.__contactct.data.rgYieldType.itemIndex==3 ? 1 : 0" % loads, 30)

        # ---- N：Count Clear 的提示不被 tick 蓋掉；disabled＝回應 ----------------------------------------
        d = json.loads(cdp.eval("JSON.stringify(window.__contactct.data)"))
        en = (d.get('enabled') or {}).get('btClearCount')
        dis = cdp.eval("document.getElementById('btClearCount').disabled")
        check(isinstance(en, bool) and dis == (not en), 'N：btClearCount.disabled（%r）＝ !enabled（%r）' % (dis, en))
        if not dis:
            cdp.eval("document.getElementById('btClearCount').click()")
            time.sleep(2.2)
            stt = cdp.eval("document.getElementById('ctStatus').textContent")
            check('尚未接' in (stt or ''), 'N：兩拍之後狀態列還是「尚未接」（%s）' % (stt or '')[:40])
        else:
            print('  NOTE  運轉中（btClearCount disabled＝golden），N 的「尚未接」略過')

        # ---- V：數值（選配） ----------------------------------------------------------------------------
        if a.values:
            arms = [base.read_arm('Arm0'), base.read_arm('Arm1')]
            his = [base.read_arm('ArmHis0'), base.read_arm('ArmHis1')]
            wait_js(cdp, "window.__contactct.ticks>%d ? 1 : 0" % state(cdp)['ticks'], 5)
            d = json.loads(cdp.eval("JSON.stringify(window.__contactct.data)"))
            base.verify_values(d, arms, his, 'V 最新 tick Kind(%)')

        # ---- H：iframe 被藏起來就不拍 ------------------------------------------------------------------
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/ht9xxx.css' % a.port})
        time.sleep(1.0)
        cdp.eval("(function(){var f=document.createElement('iframe');f.id='ctf';f.style.width='400px';f.style.height='460px';"
                 "f.src='/page/%s';document.body.appendChild(f);return 1;})()" % PAGE)
        W = "document.getElementById('ctf').contentWindow"
        ok = wait_js(cdp, "(function(){try{var s=%s.__contactct;return s&&s.data&&s.loads>=1?1:0;}catch(e){return 0;}})()" % W, 60)
        check(bool(ok), 'H：iframe 裡的頁面開起來了')
        if ok:
            time.sleep(2.2)
            h0 = state(cdp, W)
            check(h0['ticks'] >= 1, 'H：iframe 顯示中有在拍（ticks=%s）' % h0['ticks'])
            cdp.eval("document.getElementById('ctf').style.display='none'")
            time.sleep(0.3)
            h1 = state(cdp, W)
            time.sleep(3.2)
            h2 = state(cdp, W)
            check(h2['ticks'] - h1['ticks'] <= 1, 'H：iframe display:none 的 3.2 秒內不拍（ticks %d -> %d）' % (h1['ticks'], h2['ticks']))
            cdp.eval("document.getElementById('ctf').style.display=''")
            time.sleep(2.2)
            h3 = state(cdp, W)
            check(h3['ticks'] - h2['ticks'] >= 1, 'H：顯示回來之後恢復（ticks %d -> %d）' % (h2['ticks'], h3['ticks']))
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)

    sha1 = base.sha_arm()
    check(sha0 == sha1, 'W：探針期間 system\\Arm*.dat SHA256 全部不變（%d 檔）' % len(sha0))
    print('\n%s：%d 項失敗' % ('PASS' if not FAILS else 'FAIL', len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
