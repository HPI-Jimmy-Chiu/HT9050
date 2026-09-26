# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/s12c_offset_probe.py -- C 路 Setup.OffSet.html（golden TfOffSet，FileRW/Offset_File.cpp）專用 e2e probe。
#
#  Steven 團隊 20260925.  s12c_page_probe.py 的通用檢查是 _EditPage 形狀（lists／mustSend／proxies），
#  Offset 的回應是分組的 offsets 整包（選取式編輯器），所以另寫這支；s12c_page_probe.py 不動。
#
#  用真的瀏覽器（headless Edge ＋ DevTools）開頁面（web/page/ht9045_offset_wire.js 負責選組與存檔）：
#    R  讀：editlist.get 回 offsets；部位按鈕顯示／可按＝各組 buttonVisible／clickable（refused 不顯示）；
#          開頁沒有選取（golden iNowOffsetSel=-1）；逐組點按鈕 → 編輯框的值／visible／可改＝該組 widgets；
#          標題（palOffsetParts／pnlIndexOffset）＝該組 display；切組會記住修改、改回原值就不算改過
#    W  寫（--write）：offsetDir 下每個檔都快照，每一步比對：
#          stander  Loader（offsets["0"]）：原值存（只允許同段數值相等的 golden 格式正規化）→ 再存位元組不變 →
#                   edArmX +0.1 → 只有 [Loader] Hand X 一行變 → 重讀畫面是新值 → 改回原值 → 位元組回到改前
#          special  Test Arm1（offsets["9:2"]）：同上，改 IndexArmOffSet2 → 只有 [Test Arm1] Place 一行變
#          ack：只送出的組出現在 groups、saved=true、tail.ran=true
#
#  ⚠ --write 會真的改 offsetDir（本機 D:\HT9045\IniData\DefineOffset）。請經 scratchpad/run_offset.sh 跑（restore＋SHA256）。
#  用法：python tools\webprobe\s12c_offset_probe.py --port 8046 --user S12TEST --password S12PW [--write]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import shutil
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login      # noqa: E402
from s12c_config_probe import wait_js                       # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
PAGE = 'Setup.OffSet.html'


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def J(cdp, expr, timeout=30):
    v = cdp.eval(expr, timeout)
    return json.loads(v) if isinstance(v, str) else v


def wait_idle(cdp, secs=90):
    """頁面兩個產生的接線檔各 register 一次 → 開頁會有兩次 editlist.get；等全部回來、onLoaded 跑完。"""
    ok = wait_js(cdp, "window.HT9045Offset && HT9045Offset.state().loads>0 && HT9045Offset.state().pendingGets===0 ? 1 : null", secs)
    time.sleep(1.5)
    ok = ok and wait_js(cdp, "HT9045Offset.state().pendingGets===0 ? 1 : null", secs)
    return bool(ok)


def snap(d):
    out = {}
    for n in sorted(os.listdir(d)):
        p = os.path.join(d, n)
        if os.path.isfile(p):
            out[n] = open(p, 'rb').read()
    return out


def line_diff(a, b):
    """同長度檔案的逐行差異 [(區段, 舊行, 新行)]；長度不同回 None。"""
    x = a.decode('cp950', 'replace').splitlines()
    y = b.decode('cp950', 'replace').splitlines()
    if len(x) != len(y):
        return None
    out, sec = [], ''
    for p, q in zip(x, y):
        t = q.strip()
        if t.startswith('[') and t.endswith(']'):
            sec = t[1:-1]
        if p != q:
            out.append((sec, p, q))
    return out


def num_equal(p, q):
    if '=' not in p or '=' not in q:
        return False
    kp, vp = p.split('=', 1)
    kq, vq = q.split('=', 1)
    if kp != kq:
        return False
    try:
        return abs(float(vp) - float(vq)) < 1e-9
    except ValueError:
        return False


def dir_diff(before, after):
    """{檔名: 逐行差異 或 'size'／'added'／'removed'}，只列有變的檔。"""
    out = {}
    for n in set(before) | set(after):
        if n not in before:
            out[n] = 'added'
        elif n not in after:
            out[n] = 'removed'
        elif before[n] != after[n]:
            d = line_diff(before[n], after[n])
            out[n] = d if d is not None else 'line count changed (%d -> %d bytes)' % (len(before[n]), len(after[n]))
    return out


def page_save(cdp, key, edit=None):
    """點 key 的部位按鈕（必要時先切頁），可選改一格，按 spbSave；等存檔＋重讀完成，回 ack。"""
    js = "(function(){window.confirm=function(){return true;};" \
         "var g=HT9045Page.golden().page.offsets[%s];" % json.dumps(key)
    js += "HT9045Offset.showPage(/:2$/.test(%s)?'tsIndexOffset':'tsInOutArmOffset');" % json.dumps(key)
    js += "document.getElementById(g.button).click();"
    if edit:
        js += "var e=document.getElementById(%s); e.value=%s; e.dispatchEvent(new Event('change',{bubbles:true}));" % (
            json.dumps(edit[0]), json.dumps(edit[1]))
    js += "window.__ofs=null; window.__ofsSend=JSON.stringify(HT9045Offset.collect().widgets);" \
          "document.getElementById('spbSave').click(); return true;})()"
    cdp.eval(js)
    # save() 送出 → ack → HT9045Page.load() → editlist.get → onLoaded（loads+1）
    t0 = time.monotonic()
    ack = None
    while time.monotonic() - t0 < 120:
        ack = J(cdp, "JSON.stringify(HT9045Offset.lastAck())")
        if ack is not None:
            break
        time.sleep(0.5)
    wait_idle(cdp)
    sent = J(cdp, "window.__ofsSend")
    return ack or {}, sent


def read_checks(cdp):
    info = J(cdp, "JSON.stringify((function(){var p=HT9045Page.golden().page; return {struct:p.struct, dir:p.offsetDir,"
                  " hot:p.temperatureHot, keys:Object.keys(p.offsets||{}), st:HT9045Offset.state()};})())")
    check(info['struct'] == 'Offset_File', 'R  struct = Offset_File（實際 %s）' % info['struct'])
    ns = len([k for k in info['keys'] if not k.endswith(':2')])
    nsp = len([k for k in info['keys'] if k.endswith(':2')])
    check(ns == 53 and nsp == 11, 'R  offsets：stander %d 組（應 53）、special %d 組（應 11）' % (ns, nsp))
    print('     offsetDir=%s  temperatureHot=%s  loads=%s' % (info['dir'], info['hot'], info['st']['loads']))
    st = info['st']
    check(st['cur']['stander'] is None and st['cur']['special'] is None,
          'R  開頁沒有選取（golden 建構子 iNowOffsetSel=-1／iSpecialOffSetSel=-1）：%s' % st['cur'])
    # 部位按鈕
    btn = J(cdp, r"""JSON.stringify((function(){var p=HT9045Page.golden().page.offsets, bad=[], n=0, nv=0;
      Object.keys(p).forEach(function(k){var g=p[k], b=document.getElementById(g.button); if(!b){bad.push(g.button+':no element'); return;}
        n++; var cs=getComputedStyle(b), shown=cs.visibility!=='hidden' && cs.display!=='none';
        var want=g.buttonVisible && !g.refused; if(want) nv++;
        if(shown!==want) bad.push(k+' '+g.button+' shown='+shown+' want='+want);
        if(b.disabled!==!(g.clickable && !g.refused)) bad.push(k+' '+g.button+' disabled='+b.disabled+' clickable='+g.clickable);});
      return {n:n, nv:nv, bad:bad};})())""")
    check(not btn['bad'], 'R  部位按鈕 %d 顆：顯示／可按與各組 buttonVisible／clickable／refused 一致（顯示 %d 顆；不一致：%s）' %
          (btn['n'], btn['nv'], btn['bad'][:8]))
    empty = J(cdp, "JSON.stringify(['edArmX','EditPickA','IndexArmOffSet2','edtTrayArmX'].map(function(id){var e=document.getElementById(id);"
                   " return id+':'+JSON.stringify(e.value)+':'+(e.disabled?'dis':'ENA');}))")
    check(all(s.endswith(':"":dis') for s in empty), 'R  沒有選取時編輯框清空停用：%s' % empty)
    # 逐組點按鈕，畫面＝該組 widgets／display
    res = J(cdp, r"""JSON.stringify((function(){var p=HT9045Page.golden().page.offsets, bad=[], n=0, nw=0, skipped=[];
      Object.keys(p).forEach(function(k){var g=p[k];
        if(!(g.buttonVisible && g.clickable && !g.refused)){skipped.push(k); return;}
        HT9045Offset.showPage(/:2$/.test(k)?'tsIndexOffset':'tsInOutArmOffset');
        document.getElementById(g.button).click(); n++;
        var st=HT9045Offset.state().cur, curk=/:2$/.test(k)?st.special:st.stander;
        if(curk!==k){bad.push(k+': not selected ('+curk+')'); return;}
        Object.keys(g.widgets).forEach(function(id){var w=g.widgets[id], e=document.getElementById(id); nw++;
          if(!e){bad.push(k+' '+id+': no element'); return;}
          if(w.text!==undefined && e.value!==w.text) bad.push(k+' '+id+' value '+e.value+'!='+w.text);
          var vis=e.style.visibility!=='hidden'; if(vis!==w.visible) bad.push(k+' '+id+' visible '+vis+'!='+w.visible);
          if(e.disabled!==!w.editable) bad.push(k+' '+id+' disabled '+e.disabled+' editable '+w.editable);});
        var cap=/:2$/.test(k)?'pnlIndexOffset':'palOffsetParts', dc=(g.display[cap]||{}).caption;
        var shown=(document.querySelector('#'+cap+' > .pnlCap')||{}).textContent;
        if(dc && shown!==dc) bad.push(k+' '+cap+' caption '+shown+'!='+dc);
        Object.keys(g.display).forEach(function(nm){var e=document.getElementById(nm); if(!e) return;
          var vis=e.style.visibility!=='hidden' && e.style.display!=='none'; if(vis!==g.display[nm].visible) bad.push(k+' '+nm+' display.visible '+vis);});
      });
      return {n:n, nw:nw, bad:bad, skipped:skipped.length};})())""", 120)
    check(res['n'] > 0 and not res['bad'],
          'R  逐組點選 %d 組（略過不可選 %d 組）、比 %d 個欄位：值／visible／可改／標題＝該組回應（不一致：%s）' %
          (res['n'], res['skipped'], res['nw'], res['bad'][:8]))
    # 切組記住修改；改回原值就不算改過
    mem = J(cdp, r"""JSON.stringify((function(){var p=HT9045Page.golden().page.offsets;
      HT9045Offset.showPage('tsInOutArmOffset');
      document.getElementById(p['0'].button).click();
      var e=document.getElementById('edArmX'), orig=e.value, v=(parseFloat(orig)+1).toFixed(2);
      e.value=v; e.dispatchEvent(new Event('change',{bubbles:true}));
      document.getElementById(p['1'].button).click();
      var inOther=document.getElementById('edArmX').value;
      document.getElementById(p['0'].button).click();
      var back=document.getElementById('edArmX').value;
      var c1=HT9045Offset.collect();
      document.getElementById('edArmX').value=orig;
      document.getElementById(p['1'].button).click();
      var c2=HT9045Offset.collect();
      document.getElementById(p['0'].button).click();
      return {orig:orig, v:v, inOther:inOther, want1:p['1'].widgets.edArmX.text, back:back,
              send1:Object.keys(c1.widgets.offsets), why1:c1.why, send2:Object.keys(c2.widgets.offsets), why2:c2.why};})())""")
    check(mem['inOther'] == mem['want1'] and mem['back'] == mem['v'],
          'R  切組記住修改：Loader edArmX %s→%s，切到 Hot Plate1 顯示 %s（該組值 %s），切回 Loader 仍是 %s' %
          (mem['orig'], mem['v'], mem['inOther'], mem['want1'], mem['back']))
    check(mem['send1'] == ['0'] and 'changed' in mem['why1'].get('0', ''),
          'R  改過的組才送：改 Loader 後目前在 Loader → 送 %s（%s）' % (mem['send1'], mem['why1']))
    check(mem['send2'] == ['1'] and 'golden' in mem['why2'].get('1', ''),
          'R  改回原值就不算改過：只剩 golden spbSaveClick 目前選取 Hot Plate1 → 送 %s（%s）' % (mem['send2'], mem['why2']))
    return info


def write_step(cdp, d, key, label, section, keyname, wid, before, delta=0.1):
    """原值存 → 再存不變 → 改一格只差一行 → 重讀新值 → 改回位元組回到改前。"""
    ack0, sent0 = page_save(cdp, key)
    s1 = snap(d)
    print('     [%s] 原值存檔 送出組 %s；ack groups=%s tail.ran=%s' % (label, list((sent0 or {}).get('offsets', {})),
                                                         {k: v.get('saved') for k, v in (ack0.get('groups') or {}).items()},
                                                         (ack0.get('tail') or {}).get('ran')))
    check(list((sent0 or {}).get('offsets', {})) == [key], 'W  [%s] 沒改也送 golden 目前選取那一組：%s' % (label, list((sent0 or {}).get('offsets', {}))))
    g0 = (ack0.get('groups') or {}).get(key) or {}
    check(g0.get('saved') is True and not g0.get('unknown') and (ack0.get('tail') or {}).get('ran') is True,
          'W  [%s] 原值存檔 saved（applied %d、ignored %s、unknown %s、refused %s、error %s）' %
          (label, len(g0.get('applied') or []), g0.get('ignored'), g0.get('unknown'), g0.get('refused'), g0.get('error')))
    dd = dir_diff(before, s1)
    norm, other = [], []
    for fn, lines in dd.items():
        if not isinstance(lines, list):
            other.append((fn, lines))
            continue
        for sec, p, q in lines:
            (norm if (sec == section and num_equal(p, q)) else other).append((fn, sec, p, q))
    check(not other, 'W  [%s] 原值存檔：只有 [%s] 同鍵數值相等的 golden 格式正規化 %d 行 %s（其他差異：%s）' %
          (label, section, len(norm), norm[:6], other[:6]))
    ack1, _ = page_save(cdp, key)
    s2 = snap(d)
    check((((ack1.get('groups') or {}).get(key)) or {}).get('saved') is True and s2 == s1,
          'W  [%s] G1：golden 寫過之後再存原值，offsetDir 全部檔案位元組不變（%s）' % (label, dir_diff(s1, s2) or 'same'))
    old = J(cdp, "JSON.stringify(HT9045Page.golden().page.offsets[%s].widgets[%s].text)" % (json.dumps(key), json.dumps(wid)))
    new = '%.2f' % (float(old) + delta)
    ack2, sent2 = page_save(cdp, key, (wid, new))
    s3 = snap(d)
    g2 = (ack2.get('groups') or {}).get(key) or {}
    check(g2.get('saved') is True and wid in (g2.get('applied') or []),
          'W  [%s] %s %s→%s 存檔 saved，applied 含 %s（ignored %s）' % (label, wid, old, new, wid, g2.get('ignored')))
    dd = dir_diff(s2, s3)
    flat = [(fn, sec, p, q) for fn, lines in dd.items() for (sec, p, q) in (lines if isinstance(lines, list) else [('?', lines, '')])]
    ok = (len(flat) == 1 and flat[0][1] == section and flat[0][2].split('=', 1)[0] == keyname and
          abs(float(flat[0][3].split('=', 1)[1]) - float(new)) < 1e-9)
    check(ok, 'W  [%s] 只有 [%s] %s 一行變：%s' % (label, section, keyname, flat[:6]))
    back = J(cdp, "JSON.stringify({grp:HT9045Page.golden().page.offsets[%s].widgets[%s].text, cur:HT9045Offset.state().cur,"
                  " dom:document.getElementById(%s).value})" % (json.dumps(key), json.dumps(wid), json.dumps(wid)))
    check(back['grp'] is not None and abs(float(back['grp']) - float(new)) < 1e-9 and back['dom'] == back['grp'],
          'L  [%s] 存檔後重讀：回應 %s=%s、畫面（仍選 %s）=%s' % (label, wid, back['grp'], back['cur'], back['dom']))
    ack3, _ = page_save(cdp, key, (wid, old))
    s4 = snap(d)
    check((((ack3.get('groups') or {}).get(key)) or {}).get('saved') is True and s4 == s2,
          'W  [%s] 改回原值 %s 再存：offsetDir 位元組回到改前（%s）' % (label, old, dir_diff(s2, s4) or 'same'))
    return s4


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9335)
    ap.add_argument('--user', default='')
    ap.add_argument('--password', default='')
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--page', default=PAGE)          # run_page.sh 形式的參數相容（不用）
    ap.add_argument('--struct', default='Offset_File')
    a = ap.parse_args()
    if a.write and not (a.user and a.password):
        check(False, 'W  --write 需要 --user／--password（golden A02：Operator 不能存）')
        return
    if a.user:
        check(bool(ws_login(a.port, a.user, a.password)), 'auth.login %s' % a.user)
    edge, prof, dws = launch_edge(a.dbg)
    try:
        cdp = Cdp(dws)
        cdp.call('Page.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (a.port, PAGE)})
        check(wait_idle(cdp), 'R  頁面走 C 路讀到 editlist.get Offset_File 並完成顯示')
        if FAILS:
            return
        info = read_checks(cdp)
        if not a.write:
            return
        d = info['dir']
        base = snap(d)
        print('     offsetDir 檔案：%s' % {k: len(v) for k, v in base.items()})
        s = write_step(cdp, d, '0', 'stander Loader', 'Loader', 'Hand X', 'edArmX', base)
        write_step(cdp, d, '9:2', 'special Test Arm1', 'Test Arm1', 'Place', 'IndexArmOffSet2', s)
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)


if __name__ == '__main__':
    main()
    print('FAIL %d' % len(FAILS) if FAILS else 'ALL PASS')
    sys.exit(len(FAILS))
