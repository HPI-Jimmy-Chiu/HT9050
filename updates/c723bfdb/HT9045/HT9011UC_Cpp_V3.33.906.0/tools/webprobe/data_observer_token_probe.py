# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_observer_token_probe.py -- Data.Observer.html 權杖分流（S124）的 e2e probe。
#
#  AI(W906-Q2-S124) 20260927 [W906] Steven 團隊。RULINGS_20260926 S124＝B（唯讀的查詢免權杖；
#  St02 9d790ff2：WebBridge/WebBridgeServer.cpp:1448 把 observer.get 列進名稱級免權杖）。
#  頁面 web/page/ht9045_observer_wire.js：
#    唯讀 act（READ_ACTS＝WebCmdGuard.cpp:112-113 kObserverActs：open／timer／tab／rowNo／form／year／month／file／
#      filter／query／ccKinds／ccKindsForm／ccHistory／ccHistoryForm）直接 observer.get、不拿權杖；
#      舊 wb_serve 回 not-operator 才退回 acquire → get → release。
#    會改記憶體的四個 act（yieldSite／yieldMax／yieldMin／yieldClear）每次 acquire → get → release
#      （St02 在它的分支把這四個改成伺服器端要權杖；改動進來前後頁面都能用）。
#  比照 Data.ContactCT（c35f375e，data_contactct_live_probe.py）：一次只送一條、背景分頁／外框縮小不送、
#  回應沒變不動畫面。
#
#  驗（頁面狀態在 window.__observer；sent＝頁面送出的最後 40 條指令 {cmd, act}）：
#    F  伺服器送出的 ht9045_observer_wire.js 是 S124 版（有 AI(W906-Q2-S124) 與 READ_ACTS）
#    R  一條從來不拿權杖的 WS 連線：observer.get（不帶 value＝open）與 {"act":"timer"} 都成功；
#       另一條連線拿著權杖不放時再送一次 timer，仍成功（免權杖跟誰拿著無關）
#    G  伺服器對「會改記憶體的 act」要不要權杖（St02 的改動）：沒拿權杖的連線送
#       observer.get {"act":"yieldMax","text":"x"}（"x" 過不了 golden N_INTEGER 鍵盤檢查 ⇒ 本體就算跑到也在改任何東西之前丟例外，
#       沒有副作用）。回的錯是鍵盤檢查那句＝伺服器還沒擋（NOTE；--expect-gate 時 FAIL）；別的拒絕（例 not-operator）＝已擋（PASS）
#    O  開頁：sent 裡 observer.get(open) 前面沒有 control.acquire；tokenFallback＝0
#    T  4.2 秒內 ticks +≥3；每一條 observer.get(timer) 前後都沒有 control.acquire／release；沒有 tickError
#    P  頁面不佔權杖：開著時另一條連線 10 次 control.acquire（每 0.25 秒一次、拿到就還）全部成功
#    K  另一條連線拿著權杖不放的 3.2 秒內：頁面照樣每秒更新（ticks +≥2、沒有 tickError、tokenFallback＝0）；
#       點 Tester Category 頁籤（act=tab 1）照樣成功（loads +1、沒有 error、sent 裡沒有 control.acquire）
#    M  同樣權杖在別人手上：頁面送 yieldMax "x" ⇒ 只送出 control.acquire、沒有送 observer.get(yieldMax)，error 講權杖；
#       權杖還給伺服器之後再送一次 ⇒ sent 依序 control.acquire → observer.get(yieldMax) → control.release，
#       之後另一條連線拿得到權杖（頁面有還）
#    X  （--mutate，會改記憶體：切換 ChartYield 一條線再切回來）Yield 頁點 mtRowA 第 1 個 site 格兩次：
#       每次都是 acquire → get → release；兩次之後那條線的 active＝原本（Series->Active 還原）
#    U  回應沒變就不動畫面：3.2 秒內 renders 沒增加、same +≥2 時，15 個 caption 格＋狀態列沒有任何 DOM 變動
#       （renders 有增加＝機台在跑、值在變：NOTE 略過）
#    S  按 btnBackupLogYear 的「尚未接」提示，兩拍之後還在（timer 不改寫狀態列）
#    H  頁面放進同源 iframe：iframe display:none 的 3.2 秒內 ticks 最多 +1（送出中那一條）；顯示回來 2.2 秒內 +≥1
#    W  整段期間 system\lastdata*.dat／Arm*.dat／Gerneral.ini SHA256 不變（只讀；--mutate 也只改記憶體）
#
#  本探針 20260927 寫成時沒有執行過（交件規則：不 build、不跑 wb_serve）。
#  用法：
#      set W906_PWBOOK_PATH=<scratch book>  &  build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\data_observer_token_probe.py --port 8046 [--user S12TEST --password S12PW]
#             [--expect-gate] [--mutate]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import glob
import json
import os
import shutil
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402
from data_contactct_live_probe import RawWs, unwrap             # noqa: E402  沒拿權杖／拿著權杖不放的旁路連線
import data_observer_probe as obs                               # noqa: E402  sha_files

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
PAGE = 'Data.Observer.html'
SYSDIR = r'D:\HT9045\system'
CAPS = obs.CAPS
READ_ACTS = ['open', 'timer', 'tab', 'rowNo', 'form', 'year', 'month', 'file', 'filter', 'query',
             'ccKinds', 'ccKindsForm', 'ccHistory', 'ccHistoryForm']
MUT_ACTS = ['yieldSite', 'yieldMax', 'yieldMin', 'yieldClear']
KEYPAD_ERR = 'N_INTEGER keypad'           # cObserver.cpp W906Obs_YieldAxisEdit 的例外字（本體跑到了＝伺服器沒擋）


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def watched():
    ps = glob.glob(os.path.join(SYSDIR, 'lastdata*.dat')) + glob.glob(os.path.join(SYSDIR, 'Arm*.dat'))
    ps.append(os.path.join(SYSDIR, 'Gerneral.ini'))
    return sorted(ps)


def state(cdp, where='window'):
    return json.loads(cdp.eval(
        "JSON.stringify((function(){var s=%s.__observer||{};return {loads:s.loads,ticks:s.ticks,same:s.same,renders:s.renders,"
        "skipped:s.skipped,stale:s.stale,tokenFallback:s.tokenFallback,tokenOps:s.tokenOps,error:s.error,tickError:s.tickError,"
        "sentN:(s.sent||[]).length,hasData:!!s.data};})())" % where) or '{}')


def mark(cdp):
    """在 sent 環上做記號：之後 sent_since() 只回記號之後送出的指令。"""
    cdp.eval("(function(){var s=window.__observer; s.__mark=(s.__mark||0)+1; s.sent.push({cmd:'__mark',act:String(s.__mark)}); return s.__mark;})()")


def sent_since(cdp):
    arr = json.loads(cdp.eval("JSON.stringify(window.__observer.sent)") or '[]')
    idx = max([i for i, e in enumerate(arr) if e.get('cmd') == '__mark'] or [-1])
    return [e for e in arr[idx + 1:] if e.get('cmd') != '__mark']


def fmt(seq):
    return ', '.join(e['cmd'] + ('(%s)' % e['act'] if e.get('act') else '') for e in seq) or '（沒有）'


def page_send(cdp, v):
    """window.HT9045Observer.send(v)，等它的 promise（回應或 null）。回 __observer 的 error。"""
    cdp.eval("window.HT9045Observer.send(%s).then(function(){return 1;},function(){return 1;})" % json.dumps(v), 30)
    return state(cdp).get('error')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9339)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--expect-gate', action='store_true', help='G：St02 的「四個改記憶體的 act 要權杖」已經在這顆 wb_serve 裡，沒擋就 FAIL')
    ap.add_argument('--mutate', action='store_true', help='X：真的切換 ChartYield 一條線再切回來（只改記憶體）')
    a = ap.parse_args()

    sha0 = obs.sha_files(watched())
    if a.user:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    src = urllib.request.urlopen('http://127.0.0.1:%d/page/ht9045_observer_wire.js' % a.port, timeout=10).read().decode('utf-8')
    check('AI(W906-Q2-S124)' in src and 'READ_ACTS' in src, 'F：伺服器送出的 ht9045_observer_wire.js 是 S124 版（唯讀 act 免權杖）')

    # ---- R：沒拿權杖的連線 --------------------------------------------------------------------------
    rw = RawWs(a.port)
    hold = None
    try:
        r1 = rw.cmd('observer.get')
        check(bool(r1 and r1.get('ok')), 'R：沒拿權杖的連線 observer.get（不帶 value＝open）成功（%s）'
              % (None if not r1 else (r1.get('error') or 'ok')))
        r2 = rw.cmd('observer.get', value=json.dumps({'act': 'timer'}))
        check(bool(r2 and r2.get('ok')), 'R：沒拿權杖的連線 observer.get {"act":"timer"} 成功（%s）'
              % (None if not r2 else (r2.get('error') or 'ok')))
        hold = RawWs(a.port)
        h = hold.cmd('control.acquire')  # AI(W906-SCREEN-TOKEN) 20261001: stays control.acquire on purpose -- R checks the page does NOT hold the token (a standalone Data.Observer.html, no frame hub); takeover would always succeed
        got = bool(h and h.get('ok'))
        check(got, 'R：另一條連線拿得到權杖（%s）' % (None if not h else (h.get('error') or 'ok')))
        r3 = rw.cmd('observer.get', value=json.dumps({'act': 'timer'}))
        check(bool(r3 and r3.get('ok')), 'R：權杖在別的連線手上時，沒拿權杖的連線 timer 仍成功（%s）'
              % (None if not r3 else (r3.get('error') or 'ok')))

        # ---- G：伺服器對改記憶體的 act 要不要權杖（St02） ------------------------------------------------
        g = rw.cmd('observer.get', value=json.dumps({'act': 'yieldMax', 'text': 'x'}))
        gerr = '' if not g else (g.get('error') or '')
        if g and g.get('ok'):
            check(False, 'G：yieldMax "x" 竟然成功（本體應該拒絕 "x"）：%s' % str(unwrap(g))[:120])
        elif KEYPAD_ERR in gerr:
            if a.expect_gate:
                check(False, 'G：沒拿權杖的連線送 yieldMax，伺服器沒擋、跑到本體（%s）—— St02 的權杖改動不在這顆 wb_serve' % gerr[:90])
            else:
                print('  NOTE  G：沒拿權杖的 yieldMax 跑到本體（%s）＝St02 的權杖改動還沒進這顆 wb_serve；頁面照樣拿權杖送，不受影響' % gerr[:90])
        elif gerr.startswith('busy:'):
            print('  NOTE  G：伺服器回 busy:（%s），這一項不判' % gerr[:90])
        else:
            check(bool(gerr), 'G：沒拿權杖的連線送 yieldMax 被伺服器擋下（%s）' % (gerr[:120] or '沒有回應'))
        if got:
            hold.cmd('control.release')
    finally:
        rw.close()
        if hold:
            hold.close()

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Runtime.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (a.port, PAGE)})
        ok = wait_js(cdp, "window.__observer && (window.__observer.data || window.__observer.error) ? 1 : 0", 60)
        check(bool(ok), 'O：開頁送出 observer.get open 並收到回應')
        s0 = state(cdp)
        if not s0.get('hasData'):
            check(False, 'O：observer.get open 成功（%s）' % s0.get('error'))
            return len(FAILS)
        seq0 = json.loads(cdp.eval("JSON.stringify(window.__observer.sent)") or '[]')
        first_get = next((i for i, e in enumerate(seq0) if e['cmd'] == 'observer.get'), -1)
        check(first_get >= 0 and all(e['cmd'] != 'control.acquire' for e in seq0[:first_get + 1]) and s0['tokenFallback'] == 0,
              'O：開頁直接 observer.get(open)、前面沒有 control.acquire、tokenFallback=0（%s）' % fmt(seq0[:6]))
        tbl = json.loads(cdp.eval("JSON.stringify({r:%s.map(function(a){return HT9045Observer.isReadAct(a);}),"
                                  "m:%s.map(function(a){return HT9045Observer.isReadAct(a);})})"
                                  % (json.dumps(READ_ACTS), json.dumps(MUT_ACTS))) or '{}')
        check(all(tbl.get('r') or [False]) and not any(tbl.get('m') or [True]),
              'O：頁面 READ_ACTS＝WebCmdGuard.cpp:112-113 的 14 個讀取型 act；四個 Yield 操作不在裡面（%s）' % tbl)

        # ---- T：每秒一拍、不拿權杖 ----------------------------------------------------------------------
        mark(cdp)
        t0 = state(cdp)
        time.sleep(4.2)
        t1 = state(cdp)
        seq = sent_since(cdp)
        check(t1['ticks'] - t0['ticks'] >= 3, 'T：4.2 秒內 ticks %d -> %d（golden Timer1 每 1000 ms）' % (t0['ticks'], t1['ticks']))
        check(seq and all(e['cmd'] == 'observer.get' and e['act'] == 'timer' for e in seq),
              'T：這段時間只送 observer.get(timer)，沒有 control.acquire／release（%s）' % fmt(seq[:6]))
        check(not t1.get('tickError'), 'T：沒有 tickError（%s）' % t1.get('tickError'))
        print('  info  ticks=%s same=%s renders=%s skipped=%s stale=%s' % (t1['ticks'], t1['same'], t1['renders'], t1['skipped'], t1['stale']))

        # ---- P：頁面不佔權杖 ----------------------------------------------------------------------------
        pw = RawWs(a.port)
        try:
            okn = 0
            for i in range(10):
                r = pw.cmd('control.acquire')  # AI(W906-SCREEN-TOKEN) 20261001: stays control.acquire on purpose -- P checks the page does not hold the token
                if r and r.get('ok'):
                    okn += 1
                    pw.cmd('control.release')
                time.sleep(0.25)
            check(okn == 10, 'P：頁面開著時另一條連線 10 次 control.acquire 成功 %d 次（頁面不佔權杖）' % okn)
        finally:
            pw.close()

        # ---- K：權杖在別人手上：唯讀照樣動；M：改記憶體的 act 拿不到權杖就不送 --------------------------------
        hold = RawWs(a.port)
        try:
            h = hold.cmd('control.takeover')  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
            got = bool(h and h.get('ok'))
            check(got, 'K：另一條連線拿到權杖並且不放（%s）' % (None if not h else (h.get('error') or 'ok')))
            k0 = state(cdp)
            time.sleep(3.2)
            k1 = state(cdp)
            check(k1['ticks'] - k0['ticks'] >= 2 and not k1.get('tickError') and k1['tokenFallback'] == 0,
                  'K：權杖在別的連線手上的 3.2 秒內頁面照樣更新（ticks %d -> %d，tickError=%s，tokenFallback=%s）'
                  % (k0['ticks'], k1['ticks'], k1.get('tickError'), k1['tokenFallback']))
            mark(cdp)
            loads = k1['loads']
            cdp.eval("(function(){var t=document.querySelector('#pgcObserv > .pcTabs > .tab[data-t=\"1\"]'); if(t) t.click(); return 1;})()")
            ok = wait_js(cdp, "window.__observer.loads>%d ? 1 : 0" % loads, 30)
            k2 = state(cdp)
            seq = sent_since(cdp)
            check(bool(ok) and not k2.get('error') and any(e['cmd'] == 'observer.get' and e['act'] == 'tab' for e in seq)
                  and all(e['cmd'] != 'control.acquire' for e in seq),
                  'K：權杖在別人手上時點 Tester Category 頁籤照樣成功、沒有拿權杖（loads %d -> %s，error=%s；%s）'
                  % (loads, k2['loads'], k2.get('error'), fmt(seq[:6])))

            mark(cdp)
            err = page_send(cdp, {'act': 'yieldMax', 'text': 'x'})
            seq = sent_since(cdp)
            check(any(e['cmd'] == 'control.acquire' for e in seq) and not any(e.get('act') == 'yieldMax' for e in seq),
                  'M：權杖在別人手上時 yieldMax 只送 control.acquire、沒有送 observer.get(yieldMax)（%s）' % fmt(seq[:6]))
            check('權杖' in (err or ''), 'M：error 講權杖（%s）' % (err or '')[:80])
            if got:
                hold.cmd('control.release')
        finally:
            hold.close()
        time.sleep(0.5)                                     # 頁面防連點冷卻（yHold 不經過；WebCmdGuard 400 ms）
        mark(cdp)
        err = page_send(cdp, {'act': 'yieldMax', 'text': 'x'})
        seq = [e for e in sent_since(cdp) if not (e['cmd'] == 'observer.get' and e['act'] == 'timer')]
        want = ['control.acquire', 'observer.get(yieldMax)', 'control.release']
        check([e['cmd'] + ('(%s)' % e['act'] if e.get('act') else '') for e in seq][:3] == want,
              'M：權杖空著時 yieldMax 依序 acquire → observer.get → release（%s）' % fmt(seq[:6]))
        check(KEYPAD_ERR in (err or '') or (err or '').startswith('busy:'),
              'M：yieldMax "x" 由 C++ 本體拒絕（golden N_INTEGER 鍵盤檢查；沒改任何東西）（%s）' % (err or '')[:90])
        chk = RawWs(a.port)
        try:
            r = chk.cmd('control.acquire')  # AI(W906-SCREEN-TOKEN) 20261001: stays control.acquire on purpose -- M checks the page released the token
            check(bool(r and r.get('ok')), 'M：之後另一條連線拿得到權杖（頁面有 release）（%s）' % (None if not r else (r.get('error') or 'ok')))
            if r and r.get('ok'):
                chk.cmd('control.release')
        finally:
            chk.close()

        # ---- X：（--mutate）真的切換一條線再切回來 ---------------------------------------------------------
        if a.mutate:
            loads = state(cdp)['loads']
            cdp.eval("(function(){var t=document.querySelector('#pgcObserv > .pcTabs > .tab[data-t=\"4\"]'); if(t) t.click(); return 1;})()")
            wait_js(cdp, "window.__observer.loads>%d ? 1 : 0" % loads, 30)
            def actives():
                d = json.loads(cdp.eval("JSON.stringify(window.__observer.last)") or '{}')
                return [bool(s and s.get('active')) for s in ((((d.get('yield') or {}).get('chart') or {}).get('series')) or [])]
            a0 = actives()
            seen = []
            for n in (1, 2):
                time.sleep(0.5)                             # WebCmdGuard 400 ms：同一條 yieldSite 太快會回 busy:
                mark(cdp)
                err = page_send(cdp, {'act': 'yieldSite', 'arg': 0, 'text': '0,1'})
                seq = [e for e in sent_since(cdp) if not (e['cmd'] == 'observer.get' and e['act'] == 'timer')]
                check([e['cmd'] for e in seq][:3] == ['control.acquire', 'observer.get', 'control.release'] and not err,
                      'X：第 %d 次 yieldSite（mtRowA 格 0,1）acquire → get → release、成功（%s；error=%s）' % (n, fmt(seq[:6]), err))
                seen.append(actives())
            check(len(a0) == 32 and seen[0] != a0, 'X：第一次點之後有一條線的 active 變了（golden mtRowAMouseUp 切換；%d 條）' % len(a0))
            check(seen[1] == a0, 'X：點兩次之後 32 條線的 active 全部還原')

        # ---- U：回應沒變就不動畫面 ------------------------------------------------------------------------
        ids = json.dumps(CAPS + ['obsStatus'])
        cdp.eval("(function(){window.__obsMut=0; var ids=%s; ids.forEach(function(id){var e=document.getElementById(id); if(!e) return;"
                 " new MutationObserver(function(l){window.__obsMut+=l.length;}).observe(e,{subtree:true,childList:true,characterData:true,attributes:true});});"
                 " return 1;})()" % ids)
        u0 = state(cdp)
        time.sleep(3.2)
        u1 = state(cdp)
        mut = cdp.eval("window.__obsMut")
        if u1['renders'] == u0['renders'] and u1['same'] - u0['same'] >= 2:
            check(mut == 0, 'U：3.2 秒內 %d 拍回應都沒變，15 格＋狀態列 DOM 變動 %s 次（應為 0）' % (u1['same'] - u0['same'], mut))
        else:
            print('  NOTE  U：這段時間 renders %d -> %d、same %d -> %d（機台在跑、值在變），不判 DOM 不動'
                  % (u0['renders'], u1['renders'], u0['same'], u1['same']))

        # ---- S：「尚未接」提示不被 timer 蓋掉 ------------------------------------------------------------
        cdp.eval("(function(){var b=document.getElementById('btnBackupLogYear'); if(b) b.click(); return 1;})()")
        time.sleep(2.2)
        stt = cdp.eval("(document.getElementById('obsStatus')||{}).textContent")
        check('尚未接' in (stt or ''), 'S：兩拍之後狀態列還是「尚未接」（%s）' % (stt or '')[:40])

        # ---- H：iframe 被藏起來就不拍 --------------------------------------------------------------------
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/ht9xxx.css' % a.port})
        time.sleep(1.0)
        cdp.eval("(function(){var f=document.createElement('iframe');f.id='obf';f.style.width='1000px';f.style.height='700px';"
                 "f.src='/page/%s';document.body.appendChild(f);return 1;})()" % PAGE)
        W = "document.getElementById('obf').contentWindow"
        ok = wait_js(cdp, "(function(){try{var s=%s.__observer;return s&&s.data?1:0;}catch(e){return 0;}})()" % W, 60)
        check(bool(ok), 'H：iframe 裡的頁面開起來了')
        if ok:
            time.sleep(2.2)
            h0 = state(cdp, W)
            check(h0['ticks'] >= 1, 'H：iframe 顯示中有在拍（ticks=%s）' % h0['ticks'])
            cdp.eval("document.getElementById('obf').style.display='none'")
            time.sleep(0.3)
            h1 = state(cdp, W)
            time.sleep(3.2)
            h2 = state(cdp, W)
            check(h2['ticks'] - h1['ticks'] <= 1, 'H：iframe display:none 的 3.2 秒內不拍（ticks %d -> %d）' % (h1['ticks'], h2['ticks']))
            cdp.eval("document.getElementById('obf').style.display=''")
            time.sleep(2.2)
            h3 = state(cdp, W)
            check(h3['ticks'] - h2['ticks'] >= 1, 'H：顯示回來之後恢復（ticks %d -> %d）' % (h2['ticks'], h3['ticks']))
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)

    sha1 = obs.sha_files(watched())
    check(sha0 == sha1, 'W：探針期間 system\\lastdata*.dat／Arm*.dat／Gerneral.ini SHA256 全部不變（%d 檔）' % len(sha0))
    print('\n%s：%d 項失敗' % ('PASS' if not FAILS else 'FAIL', len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
