# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/status_countersel_probe.py -- Status.CounterSel.html（golden V912 cCounterSel.cpp TfCounterSel）的 e2e probe。
#  AI(W906-CRT-CounterSel) 20260926（Steven 團隊）：新檔。
#
#  為什麼不用通用的 s12c_page_probe.py：它的 --edit 是 `document.getElementById(id).value = v`，只改得動文字框；
#  這一頁 18 個元件全是 TCheckBox／TRadioButton／TRadioGroup（<label> 包 <input>），改不到值，「改一筆只差一行」必定失敗。
#  R／G1 的規則照 s12c_page_probe.py（共用 ini_map／semantic_diff）。
#
#  用真的瀏覽器（headless Edge ＋ DevTools 協定）開頁面，驗：
#    R  讀：editlist.get tag=IniConfig_CounterSel；mustSend 12 個都在頁面上；替身值＝畫面值；
#          畫面值＝ --file 的 [Visible]（有那個鍵才比；沒有＝golden 用 LastSet.* 預設值，開頁會補寫）；
#          六組 On／Off 單選鈕同名互斥、剛好勾一個；gbScanner（DFM Enabled=False）底下停用、其餘可操作
#    W  （--write 才做）原值存一次：只允許 [Visible] 的 golden 補鍵／同值；再存一次位元組不變（G1）
#       改 --edit 一個元件（真的點畫面上的 input）→ 檔案只差 [Visible] 那一個鍵；重讀畫面是新值；再點回來存 → 位元組＝G1 那一版
#    D  cbDefaultValue 勾選存檔：ack.todo 有 :91-103（FormPos.def 不寫）、重讀後勾選被清掉（golden :90）、FormPos.def 沒變、
#       --file 位元組不變
#    E  Exit 鈕：點 spbExit（頁面不在 iframe 裡，不會關）→ 走存檔（lastSave 換新、saved=true）、--file 位元組不變
#
#  ⚠ --write 會真的改 --file（預設 AuthPath＝D:\HT9045\config\config.ini，量產共用檔）。跑之前自己備份、跑完比 SHA256；
#    本探針最後會把改過的那一格點回原值再存（檔案回到 G1 那一版＝原檔＋golden 補鍵），不會直接覆寫檔案。
#    golden FormClose 沒有 A02／權限守衛 → --user／--password 可省（帶了就先 auth.login）。
#  用法：
#      python tools\webprobe\status_countersel_probe.py --port 8046
#             [--write --file D:\HT9045\config\config.ini --edit cbUPH [--allow "<正則：[Visible] 以外允許變的 區段/鍵>"]]
#  --edit：cbUPH／cbIndexTime／cbCycleTime／cbContactHeight／rbLoadingCount／rbContactCount／rbTestCategory／rbTemperature／
#          rbBinAssign／rgTestCategory（rbScanner 在 DFM 停用的 gbScanner 裡，改不到）
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import hashlib
import json
import os
import re
import shutil
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js, page_save                # noqa: E402
from s12c_page_probe import ini_map, semantic_diff             # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
PAGE = 'Status.CounterSel.html'
STRUCT = 'IniConfig_CounterSel'
# golden cCounterSel.cpp FormShow :32-48／FormClose :59-69 ↔ golden cprod.cpp:2721 ProcessLastSetIni_Visible 的 [Visible] 鍵
CB = {'cbUPH': 'bShowUPH', 'cbIndexTime': 'bShowIndexTime', 'cbCycleTime': 'bShowTimeInfo', 'cbContactHeight': 'bShowContactHeight'}
RB = {'rbLoadingCount': 'bShowLoaderCT', 'rbContactCount': 'bShowContactCT', 'rbTestCategory': 'bShowTestCate',
      'rbScanner': 'bShowScanCate', 'rbTemperature': 'bShowTemper', 'rbBinAssign': 'bShowBinCT'}
RG = {'rgTestCategory': 'iShowCateByArm'}
VIS_KEYS = set(CB.values()) | set(RB.values()) | set(RG.values())

STATE_JS = r"""(function(){
  function inp(id){var e=document.getElementById(id); return e?(e.tagName==='INPUT'?e:e.querySelector('input')):null;}
  var o={cb:{},rb:{},rg:{},px:HT9045Page.golden().page.proxies};
  ['cbUPH','cbIndexTime','cbCycleTime','cbContactHeight','cbDefaultValue'].forEach(function(id){var i=inp(id); o.cb[id]=i?{c:i.checked,d:i.disabled}:null;});
  ['rbLoadingCount','rbContactCount','rbTestCategory','rbScanner','rbTemperature','rbBinAssign'].forEach(function(p){
    var a=inp(p+'_On'), b=inp(p+'_Off');
    o.rb[p]=(a&&b)?{on:a.checked,off:b.checked,name:[a.name,b.name],d:[a.disabled,b.disabled]}:null;});
  var rs=document.querySelectorAll('#rgTestCategory input[type="radio"]'), k=-1;
  for(var i=0;i<rs.length;i++) if(rs[i].checked) k=i;
  o.rg.rgTestCategory={idx:k,n:rs.length,d:rs.length?rs[0].disabled:null};
  return JSON.stringify(o);})()"""


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def sha(b):
    return hashlib.sha256(b).hexdigest()[:16]


def truthy(v):
    return str(v).strip().lower() in ('1', 'true', '-1')


def state(cdp):
    return json.loads(cdp.eval(STATE_JS))


def click_edit(cdp, wid):
    """真的點畫面上的 input（VCL 操作員動作）。回傳點完之後這個元件的新值（bool 或 ItemIndex）。"""
    if wid in CB:
        js = "(function(){var i=document.querySelector('#%s input'); i.click(); return JSON.stringify(i.checked);})()" % wid
    elif wid in RB:
        js = ("(function(){var a=document.querySelector('#%s_On input'), b=document.querySelector('#%s_Off input');"
              " (a.checked?b:a).click(); return JSON.stringify(a.checked);})()" % (wid, wid))
    elif wid in RG:
        js = ("(function(){var r=document.querySelectorAll('#%s input[type=\"radio\"]'), k=-1;"
              " for(var i=0;i<r.length;i++) if(r[i].checked) k=i; r[k===1?0:1].click(); return JSON.stringify(k===1?0:1);})()" % wid)
    else:
        raise SystemExit('--edit %s：不認得（見檔頭）' % wid)
    return json.loads(cdp.eval(js))


def key_of(wid):
    return CB.get(wid) or RB.get(wid) or RG.get(wid)


def vis_diff(a, b, allow):
    """回傳 ([Visible] 內變了的 (鍵, 舊, 新)), [Visible] 以外變了且不在 --allow 的 (區段/鍵, 舊, 新))"""
    x, y = ini_map(a), ini_map(b)
    vis, other = [], []
    for k in sorted(set(x) | set(y)):
        if x.get(k) == y.get(k):
            continue
        if k[0] == 'Visible':
            vis.append((k[1], x.get(k), y.get(k)))
        elif not (allow and re.search(allow, '%s/%s' % k)):
            other.append(('%s/%s' % k, x.get(k), y.get(k)))
    return vis, other


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9335)
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--file', default=r'D:\HT9045\config\config.ini', help='AuthPath+"config.ini"（W906_AUTH_PATH 有設就跟著改）')
    ap.add_argument('--edit', default='cbUPH')
    ap.add_argument('--allow', default='', help='正則（比對「區段/鍵」）：[Visible] 以外、同一段時間裡別的寫者會動的鍵（例 WriteLastDataFile 檔尾計數鍵）')
    ap.add_argument('--user', default='')
    ap.add_argument('--password', default='')
    a = ap.parse_args()
    if a.user:
        check(bool(ws_login(a.port, a.user, a.password)), 'auth.login %s' % a.user)
    edge, prof, dws = launch_edge(a.dbg)
    try:
        cdp = Cdp(dws)
        cdp.call('Page.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (a.port, PAGE)})
        g = wait_js(cdp, "window.HT9045Page && HT9045Page.golden && HT9045Page.golden().page ? "
                         "JSON.stringify({struct:HT9045Page.golden().struct, kinds:HT9045Page.golden().kinds, "
                         "mustSend:HT9045Page.golden().page.mustSend, todo:(HT9045Page.golden().page.session||{}).todo}) : null", 90)
        check(bool(g), 'R  頁面走 C 路讀到 editlist.get 的回應（引擎 GOLDEN_BRIDGE 要有 %s）' % PAGE)
        if not g:
            return
        g = json.loads(g)
        check(g['struct'] == STRUCT, 'R  struct = %s（實際 %s）' % (STRUCT, g['struct']))
        miss = [m for m in g['mustSend'] if m not in g['kinds']]
        check(len(g['mustSend']) == 12 and not miss, 'R  mustSend %d 個全部在頁面上且有值（缺：%s）' % (len(g['mustSend']), miss))
        s = state(cdp)
        px = s['px']
        bad = []
        for id_ in list(CB) + ['cbDefaultValue']:
            if s['cb'][id_] is None or s['cb'][id_]['c'] != px.get(id_, {}).get('checked'):
                bad.append(id_)
        for p in RB:
            r = s['rb'][p]
            if r is None or r['on'] != px.get(p + '_On', {}).get('checked') or r['off'] != px.get(p + '_Off', {}).get('checked'):
                bad.append(p)
        if s['rg']['rgTestCategory']['idx'] != px.get('rgTestCategory', {}).get('itemIndex'):
            bad.append('rgTestCategory')
        check(not bad, 'R  替身值＝畫面值（不一致：%s）' % bad)
        grp = [p for p in RB if not (s['rb'][p] and s['rb'][p]['name'][0] and s['rb'][p]['name'][0] == s['rb'][p]['name'][1]
                                     and s['rb'][p]['on'] != s['rb'][p]['off'])]
        check(not grp, 'R  六組 On／Off 同名互斥、剛好勾一個（VCL 同 Parent；不符：%s）' % grp)
        dis = s['rb']['rbScanner']['d']
        en = [id_ for id_ in CB if s['cb'][id_]['d']] + [p for p in RB if p != 'rbScanner' and any(s['rb'][p]['d'])] + \
             (['rgTestCategory'] if s['rg']['rgTestCategory']['d'] else []) + (['cbDefaultValue'] if s['cb']['cbDefaultValue']['d'] else [])
        check(all(dis) and not en, 'R  gbScanner（DFM Enabled=False）底下停用 %s、其餘可操作（被停用的：%s）' % (dis, en))
        if os.path.exists(a.file):
            f = ini_map(open(a.file, 'rb').read())
            want = {}
            for id_, k in CB.items():
                want[k] = s['cb'][id_]['c']
            for p, k in RB.items():
                want[k] = s['rb'][p]['on']
            fbad, absent = [], []
            for k, v in want.items():
                if ('Visible', k) not in f:
                    absent.append(k)
                elif truthy(f[('Visible', k)]) != v:
                    fbad.append('%s=%s/畫面%s' % (k, f[('Visible', k)], v))
            if ('Visible', 'iShowCateByArm') in f:
                if f[('Visible', 'iShowCateByArm')].strip() != str(s['rg']['rgTestCategory']['idx']):
                    fbad.append('iShowCateByArm=%s/畫面%s' % (f[('Visible', 'iShowCateByArm')], s['rg']['rgTestCategory']['idx']))
            else:
                absent.append('iShowCateByArm')
            if absent:
                print('     INFO  %s 沒有 [Visible] %s：golden 用 LastSet.* 當預設（開頁 CheckAndReadIniData 會補寫）' % (a.file, absent))
            check(not fbad, 'R  畫面值＝ %s [Visible]（不一致：%s）' % (a.file, fbad))
        print('     開頁 session.todo：%s' % g.get('todo'))
        if not a.write:
            return
        before = open(a.file, 'rb').read()
        # ---- W1：原值存檔（golden FormClose 無條件寫 [Visible] 11 鍵）----
        m1 = page_save(cdp)
        print('     第一次存檔（原值）：%s' % m1)
        check(m1.get('saved') is True, 'W  原值存檔 saved')
        after1 = open(a.file, 'rb').read()
        gkeys = set(('Visible', k) for k in VIS_KEYS)
        norm = semantic_diff(before, after1, gkeys, a.allow or None)
        check(not norm['bad'], 'W  原值存檔只有正規化差異（同值 %d、golden 補鍵 %s、允許 %s；其他：%s）' %
              (norm['same'], norm['added'], norm['allowed'], norm['bad'][:6]))
        # AI(W906-Q57-TRIAGE) 20260930: Q57 r3g FAIL「再存一次原值 saved」＝探針自己連點：這一包跟上一次 editlist.save 一模一樣（同 cmd＋tag＋value），
        #   上一次做完 182 ms 就送（r3g console.txt:183「[cmdguard] busy, not run … arrived 182 ms after」）⇒ 伺服器防連點 WebCmdGuard
        #   （2ae40ffe，400 ms，Steven 20260926「全部按鈕要防連點」）照規則不跑。這支探針（0b1f4204）寫在 WebCmdGuard 之前。等過窗口再存。
        time.sleep(0.6)
        m1b = page_save(cdp)
        check(m1b.get('saved') is True, 'W  再存一次原值 saved（%s）' % m1b)
        after1b = open(a.file, 'rb').read()
        check(after1b == after1, 'W  G1：再存原值 %s 位元組不變（%s → %s）' % (os.path.basename(a.file), sha(after1), sha(after1b)))
        # ---- W2：改一個元件 ----
        k = key_of(a.edit)
        nv = click_edit(cdp, a.edit)
        m2 = page_save(cdp)
        print('     改 %s → %s 存檔：%s' % (a.edit, nv, m2))
        check(m2.get('saved') is True, 'W  改一筆存檔 saved')
        after2 = open(a.file, 'rb').read()
        vis, other = vis_diff(after1b, after2, a.allow)
        want_v = str(nv) if isinstance(nv, int) and not isinstance(nv, bool) else None
        ok = len(vis) == 1 and vis[0][0] == k and (truthy(vis[0][2]) == nv if want_v is None else vis[0][2].strip() == want_v)
        check(ok and not other, 'W  只差 [Visible] %s 一個鍵（[Visible] 變了：%s；其他區段變了：%s）' % (k, vis, other[:6]))
        s2 = state(cdp)
        cur = (s2['cb'][a.edit]['c'] if a.edit in CB else s2['rb'][a.edit]['on'] if a.edit in RB else s2['rg'][a.edit]['idx'])
        check(cur == nv, 'L  存檔後重讀（golden FormShow）畫面是新值（%s＝%s，要 %s）' % (a.edit, cur, nv))
        # ---- 點回原值（不直接覆寫檔案：記憶體 IniConfig 也要回去）----
        click_edit(cdp, a.edit)
        m3 = page_save(cdp)
        after3 = open(a.file, 'rb').read()
        vis3, other3 = vis_diff(after1b, after3, a.allow)
        check(m3.get('saved') is True and not vis3, 'W  點回原值存檔：[Visible] 回到 G1 那一版（仍不同：%s；其他區段：%s）' % (vis3, other3[:6]))
        # ---- D：cbDefaultValue（golden :87-104；FormPos.def 那段擋掉）----
        fp = os.path.join(os.path.dirname(a.file), 'FormPos.def')
        fp0 = open(fp, 'rb').read() if os.path.exists(fp) else None
        cdp.eval("document.querySelector('#cbDefaultValue input').click()")
        cdp.eval("(function(){window.confirm=function(){return true;}; window.__csd=null; HT9045Page.save().then(function(){"
                 "window.__csd=JSON.stringify(HT9045Page.golden().lastSave||{});});})()", timeout=120)
        ackd = json.loads(wait_js(cdp, 'window.__csd', 120) or '{}')
        todo = (ackd.get('session') or {}).get('todo') or []
        after4 = open(a.file, 'rb').read()
        fp1 = open(fp, 'rb').read() if os.path.exists(fp) else None
        s4 = state(cdp)
        check(ackd.get('saved') is True and any(':91-103' in t for t in todo),
              'D  cbDefaultValue 勾選存檔：saved、ack.todo 列出 :91-103（FormPos.def 不寫）（todo=%s）' % todo)
        check(s4['cb']['cbDefaultValue']['c'] is False, 'D  重讀後 cbDefaultValue 被清掉（golden :90）')
        check(fp0 == fp1, 'D  %s 沒有被寫（%s）' % (fp, 'unchanged' if fp0 == fp1 else 'CHANGED'))
        vis4, other4 = vis_diff(after3, after4, a.allow)
        check(not vis4 and not other4, 'D  %s 內容不變（[Visible]：%s；其他：%s）' % (os.path.basename(a.file), vis4, other4[:6]))
        # ---- E：Exit 鈕＝golden FormClose（頁面不在 iframe 裡，不會關）----
        cdp.eval("window.confirm=function(){return true;}; window.__cse0=HT9045Page.golden().lastSave; "
                 "document.getElementById('spbExit').click();")
        e = wait_js(cdp, "(function(){var L=HT9045Page.golden().lastSave; return (L && L!==window.__cse0) ? JSON.stringify(L) : null;})()", 120)
        ack_e = json.loads(e or '{}')
        after5 = open(a.file, 'rb').read()
        vis5, other5 = vis_diff(after4, after5, a.allow)
        check(ack_e.get('saved') is True, 'E  點 Exit 走 golden FormClose 存檔（saved=%s）' % ack_e.get('saved'))
        check(not vis5 and not other5, 'E  Exit 存原值 %s 內容不變（[Visible]：%s；其他：%s）' % (os.path.basename(a.file), vis5, other5[:6]))
        print('     %s：跑前 %s、G1 版 %s、結束 %s%s' % (a.file, sha(before), sha(after1b), sha(after5),
              '（＝跑前）' if after5 == before else '（與跑前不同：golden 補鍵／格式，見 W1）'))
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)


if __name__ == '__main__':
    main()
    print('FAIL %d' % len(FAILS) if FAILS else 'ALL PASS')
    sys.exit(len(FAILS))
