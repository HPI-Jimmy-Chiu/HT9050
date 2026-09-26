# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/s12c_page_probe.py -- C 路（golden 表單橋，HTEditList 形狀）任一頁的 e2e probe。
#
#  Steven 20260924.  規格：.claude/skills/ht9045-html-json/references/route-c-golden-bridge.md。
#  s12c_config_probe.py 是 Config.Configuration.html 專用版；這支給第二個起的 C 路結構（FileRW/_EditPage.h）。
#
#  用真的瀏覽器（headless Edge ＋ DevTools 協定）開頁面：
#    R  讀：引擎走 WS editlist.get（golden FormShow），清單值＝畫面值；mustSend 全在頁面上；
#          editable=false 的替身全停用、editable=true 的都可操作
#    W  寫（--write 才做）：原值存一次（只允許 golden 格式正規化）→ 再存一次原值位元組不變（G1）；
#       再改 --edit 一個欄位存一次 → 檔案只差那一行
#    L  存檔後重讀，畫面是新值
#
#  ⚠ --write 會真的改 --file。跑之前自己備份，跑完比 SHA256 還原。golden A02：要 --user/--password。
#  用法：
#      python tools\webprobe\s12c_page_probe.py --port 8046 --page Setup.Ld_ULd.html --struct Ld_UldDelayTime
#             [--write --file D:\HT9045\IniData\Data\<recipe>\UdUld.Data --edit edtLD_TrayArrivalDely=0.3
#              --user S12TEST --password S12PW]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login      # noqa: E402
from s12c_config_probe import wait_js, page_save            # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def ini_map(raw):
    out, sec = {}, ''
    for ln in raw.decode('cp950', 'replace').splitlines():
        t = ln.strip()
        if t.startswith('[') and t.endswith(']'):
            sec = t[1:-1]
        elif '=' in t:
            k, v = t.split('=', 1)
            out[(sec.strip(), k.strip())] = v
    return out


def golden_keys(gen_inc):
    """審查第 8 輪 M-6：golden 會寫的 (區段, 鍵) —— 從產生的 .gen.inc 機械抽：HTEditList 的 ->Add(替身, &變數, 型別,
    "區段", "鍵", …) 與 WriteIniData(路徑, "區段", "鍵", …)。第一次存檔「新增的鍵」必須在這裡面，才算 golden 補鍵。
    另收 golden 執行期組的鍵名（asKey.printf("bSpecBinBySiteCompareEnable%02d_FT", i)）：只取 printf／sprintf 的格式字串，
    轉成正則（%d→數字、%s→任意），以 ('*', 正則) 放進鍵集 —— 區段不限（區段多半也是變數）。"""
    import re
    t = open(gen_inc, encoding='utf-8').read()
    keys = set(re.findall(r'->Add\([^;]*?,\s*EC\w+\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"', t))
    keys |= set(re.findall(r'WriteIniData\w*\(\s*\w+\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"', t))
    # Steven 20260925（BinSelect）：golden 另一種寫法 FormSysTools->WriteIniData(區段, 鍵, 值)（先 OpenIniFile，沒有路徑參數）。
    # 區段是字面值就收 (區段, 鍵)；區段是運算式（GroupStr＝"Category%d"、s6TrayName[i]）就收 ('*', ^鍵$)。
    fst = r'FormSysTools->WriteIniData\w*\(\s*'
    keys |= set(re.findall(fst + r'"([^"]*)"\s*,\s*"([^"]*)"', t))
    keys |= set(('*', '^' + re.escape(k) + '$') for k in re.findall(fst + r'[A-Za-z_][^,"]*?\s*,\s*"([^"]*)"', t))
    conv ={'d': r'\d+', 'u': r'\d+', 'i': r'\d+', 's': r'.*', 'f': r'[-0-9.]+'}
    for fmt in re.findall(r'\bs?printf\(\s*"([^"]*%[^"]*)"', t):
        parts = re.split(r'(%[-0-9.]*[duisf])', fmt)
        rx = ''.join(conv[x[-1]] if (x.startswith('%') and len(x) > 1 and x[-1] in conv) else re.escape(x) for x in parts)
        lit = ''.join(x for x in parts if not (x.startswith('%') and len(x) > 1))
        if len(lit.strip()) >= 4:   # 只有 %s／%d 之類的格式（"%s"）什麼鍵都吻合，不收
            keys.add(('*', '^' + rx + '$'))
    return {(a.strip(), b.strip()) if a != "*" else (a, b) for a, b in keys}   # INI 寫入會去掉鍵名前後空白（golden 有 "…Auto Site Off "）


def key_known(k, gkeys):
    import re
    if k in gkeys:
        return True
    return any(sec == '*' and re.match(rx, k[1]) for sec, rx in gkeys)


def semantic_diff(a, b, gkeys=None, allow=None):
    import re
    x, y = ini_map(a), ini_map(b)
    same, bad, added, allowed = 0, [], [], []
    for k, v in y.items():
        if allow and re.search(allow, '%s/%s' % k) and (k not in x or x[k] != v):
            allowed.append('%s/%s' % k)
            continue
        if k not in x:
            if gkeys is not None and not key_known(k, gkeys):
                bad.append(('%s/%s' % k, '(added, not a golden key)', v))
            else:
                added.append('%s/%s' % k)
        elif x[k] != v:
            try:
                if abs(float(x[k]) - float(v)) < 1e-9:
                    same += 1
                    continue
            except ValueError:
                pass
            bad.append(('%s/%s' % k, x[k], v))
    bad += [('%s/%s' % k, x[k], '(removed)') for k in x if k not in y]
    # 同一區段重複的鍵（ini_map 看不到）：逐行計數
    for raw, tag in ((a, 'before'), (b, 'after')):
        seen, sec = {}, ''
        for ln in raw.decode('cp950', 'replace').splitlines():
            t = ln.strip()
            if t.startswith('[') and t.endswith(']'):
                sec = t[1:-1]
            elif '=' in t:
                kk = (sec, t.split('=', 1)[0])
                seen[kk] = seen.get(kk, 0) + 1
        dup = ['%s/%s' % kk for kk, n in seen.items() if n > 1]
        if dup and tag == 'after':
            bad.append(('duplicate keys after save', '', dup[:6]))
    return {'same': same, 'bad': bad, 'added': added, 'allowed': allowed}


# -----------------------------------------------------------------------------
#  Steven 20260925：--struct BinSelect 專用（Setup.BinSel.html，golden TfBinSel）。
#  bin 格子的值不在替身上，而在回應的 bin.tags[tag].panel —— 通用檢查看不到，這裡另外比：
#    R  頁面作用中那一組 = 回應的 activeTag；頁面工作副本 panel = 回應 panel（沒動過）；
#       表頭 bin 數 = iTestBinCount；托盤／Not in use 列的指派格 = BackT6PosTrayRow；Cons. Fail 列的 V = bConFail；
#       editable=false 的組，格子表是唯讀
#    W  --edit bincell=<列 y>:<bin x|auto>：真的在頁面上對那一格送 mousedown（golden mtBinSelectMouseDown），
#       存檔 → 檔案只差「那一格對應的鍵」（y=3 Cons. Fail：[Bin Func …]/BinConsFail 與 [Category<x>]/Cons.Fail）→
#       再存一次位元組不變 → 重讀後頁面是新值
# -----------------------------------------------------------------------------
BIN_ROW_FIELD = {3: 'bConFail'}                    # 目前寫入測試支援的列（golden eConsFail）
BIN_ROW_KEYS = {3: (r'^Bin Func', 'BinConsFail', 'Category%d', 'Cons.Fail')}


def binsel_read_checks(cdp):
    s = json.loads(cdp.eval(
        "(function(){var g=HT9045Page.golden().page, b=g&&g.bin; if(!b||!window.HT9045BinSel) return JSON.stringify(null);"
        " var st=HT9045BinSel.state(); if(!st) return JSON.stringify({noState:true});"
        " var tg=b.tags.filter(function(t){return t.tag===st.tag;})[0]||{};"
        " var P=st.panel, bad=[], asg=0, want=0, cons=[];"
        " for(var x=0;x<st.N;x++){ var r=P.BackT6PosTrayRow[x]; if(r>=st.NOTUSE){want++;"
        "   var td=document.querySelector('#binBody tr[data-y=\"'+r+'\"] td.bc[data-b=\"'+x+'\"]');"
        "   if(!td||!td.classList.contains('asg')) bad.push('bin'+x+'@row'+r); }"
        "   var c=document.querySelector('#binBody tr[data-y=\"3\"] td.bc[data-b=\"'+x+'\"]');"
        "   if(c && (c.textContent==='V')!==!!P.bConFail[x]) cons.push(x); }"
        " asg=document.querySelectorAll('#binBody td.bc.asg').length;"
        " return JSON.stringify({activeTag:b.activeTag, tag:st.tag, name:tg.name, file:tg.file, editable:tg.editable,"
        "  panelEnabled:tg.panelEnabled, N:b.iTestBinCount, T:b.eTrayCount, pageN:st.N,"
        "  hdr:document.querySelectorAll('#headRow th.bhdr').length, trayRows:document.querySelectorAll('#binBody tr[data-tray]').length,"
        "  same:JSON.stringify(P)===JSON.stringify(tg.panel), bad:bad, asg:asg, want:want, cons:cons,"
        "  ro:document.getElementById('binTable').classList.contains('ro'), guessed:st.guessed,"
        "  tags:b.tags.map(function(t){return t.name+(t.loaded?'':'(未讀)')+(t.editable?'':'(不可改)');})});})()"))
    check(bool(s) and not s.get('noState'), 'R  BinSelect：回應有 bin，頁面建好格子（HT9045BinSel.state）')
    if not s or s.get('noState'):
        return None
    print('     bin：作用中 %s（tag %s，%s），iTestBinCount=%s，eTrayCount=%s，editable=%s，panelEnabled=%s' %
          (s['name'], s['tag'], s['file'], s['N'], s['T'], s['editable'], s['panelEnabled']))
    print('     7 組：%s' % ', '.join(s['tags']))
    print('     規則推定（回應沒有 bin.rules）：%s' % (s['guessed'] or '無'))
    check(s['tag'] == s['activeTag'], 'R  BinSelect：頁面作用中那一組 = 回應 activeTag（%s vs %s）' % (s['tag'], s['activeTag']))
    check(s['same'], 'R  BinSelect：頁面 panel 工作副本 = 回應 bin.tags[%s].panel（開頁未編輯）' % s['tag'])
    check(s['hdr'] == s['N'] == s['pageN'], 'R  BinSelect：表頭 bin 數 %s = iTestBinCount %s' % (s['hdr'], s['N']))
    check(s['trayRows'] == s['T'] + 1, 'R  BinSelect：托盤列 %s = eTrayCount %s + Not in use' % (s['trayRows'], s['T']))
    check(not s['bad'] and s['asg'] == s['want'],
          'R  BinSelect：指派格 %d 個 = BackT6PosTrayRow（應 %d；不符：%s）' % (s['asg'], s['want'], s['bad'][:8]))
    check(not s['cons'], 'R  BinSelect：Cons. Fail 列的 V = panel.bConFail（不符的 bin：%s）' % s['cons'][:8])
    check(s['ro'] == (not s['editable']), 'R  BinSelect：格子表唯讀 = !editable（ro=%s editable=%s）' % (s['ro'], s['editable']))
    if s['editable']:
        # 頁面操作自測（不存檔）：拖拉指派（SetBinTray）、托盤 Failed／Error 欄（mtTrayNameMouseDown），最後切 cbTestMode
        # 同一組（golden cbTestModeChange 的 ReadFile）丟掉修改，確認回到檔案值 —— 後面的寫入測試從乾淨狀態開始
        t = json.loads(cdp.eval(r"""(function(){
          var S=function(){return HT9045BinSel.state();}, s=S(), P=s.panel, out={};
          function down(sel){var e=document.querySelector(sel); if(!e) return false; e.dispatchEvent(new MouseEvent('mousedown',{bubbles:true}));
            document.dispatchEvent(new MouseEvent('mouseup',{bubbles:true})); return true;}
          var x=-1; for(var i=0;i<s.N;i++) if(P.BackT6PosTrayRow[i]>=s.SETTING){x=i;break;}
          if(x>=0){ down('#binBody tr[data-y="'+s.NOTUSE+'"] td.bc[data-b="'+x+'"]'); var q=S().panel;
            out.drag={x:x, row:q.BackT6PosTrayRow[x], con:q.bConFail[x], asg:!!document.querySelector('#binBody tr[data-y="'+s.NOTUSE+'"] td.bc.asg[data-b="'+x+'"]')}; }
          var tt=-1; for(var k=0;k<s.T;k++) if(!P.bT6Link[k] && P.iErrorT6!==k && s.rules.trayCanUse[k]){tt=k;break;}
          if(tt>=0){ var f0=S().panel.iT6IsFail[tt]; down('#binBody tr[data-tray="'+tt+'"] td.stat[data-col="1"]');
            out.pass={t:tt, before:f0, after:S().panel.iT6IsFail[tt], text:document.querySelector('#binBody tr[data-tray="'+tt+'"] td.stat[data-col="1"]').textContent};
            down('#binBody tr[data-tray="'+tt+'"] td.stat[data-col="2"]');
            out.err={t:tt, iErrorT6:S().panel.iErrorT6, fail:S().panel.iT6IsFail[tt]}; }
          var sel=document.getElementById('cbTestMode'); sel.dispatchEvent(new Event('change',{bubbles:true}));
          out.reset=JSON.stringify(S().panel)===JSON.stringify(HT9045BinSel.filePanel());
          return JSON.stringify(out);})()"""))
        d = t.get('drag')
        check(bool(d) and d['row'] == cdp.eval('HT9045BinSel.state().NOTUSE') and d['con'] is False and d['asg'],
              'R  BinSelect 自測：把 bin 拖到 Not in use（SetBinTray：該 bin 改列、Cons.Fail 清掉、畫面指派格跟著動）%s' % d)
        p = t.get('pass')
        check(bool(p) and p['before'] != p['after'] and p['text'] == ('Failed' if p['after'] > 0 else 'Pass'),
              'R  BinSelect 自測：托盤 Failed 欄點一下 Pass/Fail 切換 %s' % p)
        e = t.get('err')
        check(bool(e) and e['iErrorT6'] == e['t'] and e['fail'] == 1, 'R  BinSelect 自測：托盤 Error 欄（iErrorT6 移到該盤、該盤設 Fail）%s' % e)
        check(t.get('reset') is True, 'R  BinSelect 自測：切 cbTestMode（cbTestModeChange 的 ReadFile）丟掉未存修改、回到檔案值')
    return s


def binsel_write(cdp, a, before_bytes):
    import re
    spec = a.edit.split('=', 1)[1]
    y, xs = spec.split(':', 1)
    y = int(y)
    if y not in BIN_ROW_FIELD:
        check(False, 'W  BinSelect：--edit bincell 目前只支援列 %s' % list(BIN_ROW_FIELD))
        return
    f = BIN_ROW_FIELD[y]
    if xs == 'auto':   # Cons. Fail：golden SeteConsFail 只有「指派在 fail 盤」的 bin 才切得動 → 挑第一個
        xs = cdp.eval("(function(){var s=HT9045BinSel.state(), P=s.panel; for(var x=0;x<s.N;x++){var r=P.BackT6PosTrayRow[x];"
                      " if(r>=s.SETTING && P.iT6IsFail[r-s.SETTING]>0) return String(x);} return '-1';})()")
    x = int(xs)
    check(x >= 0, 'W  BinSelect：找到可改的 bin（%s）' % x)
    if x < 0:
        return
    old = cdp.eval("HT9045BinSel.state().panel.%s[%d]" % (f, x))
    cdp.eval("(function(){var td=document.querySelector('#binBody tr[data-y=\"%d\"] td.bc[data-b=\"%d\"]');"
             " td.dispatchEvent(new MouseEvent('mousedown',{bubbles:true})); document.dispatchEvent(new MouseEvent('mouseup',{bubbles:true}));})()" % (y, x))
    new = cdp.eval("HT9045BinSel.state().panel.%s[%d]" % (f, x))
    shown = cdp.eval("document.querySelector('#binBody tr[data-y=\"%d\"] td.bc[data-b=\"%d\"]').textContent" % (y, x))
    check(new != old and (shown == 'V') == bool(new),
          'W  BinSelect：頁面點 bin %d 的 %s 格（golden mtBinSelectMouseDown）%s → %s，畫面「%s」' % (x, f, old, new, shown))
    msg = page_save(cdp)
    print('     存檔（bin %d %s %s→%s）：%s' % (x, f, old, new, msg))
    check(msg.get('saved') is True, 'W  BinSelect：改一格存檔 saved')
    ack = json.loads(cdp.eval("JSON.stringify((HT9045Page.golden().lastSave||{}).bin||null)") or 'null')
    print('     ack.bin：%s' % ack)
    check(bool(ack) and ack.get('applied') is True, 'W  BinSelect：後端套用了頁面送的 panel（ack.bin.applied）')
    after = open(a.file, 'rb').read()
    bx, ax = ini_map(before_bytes), ini_map(after)
    changed = sorted(set(k for k in set(bx) | set(ax) if bx.get(k) != ax.get(k)))
    secrx, key1, catfmt, key2 = BIN_ROW_KEYS[y]
    # 新格式的鍵名字面值就帶 ECID，依組別不同（golden SaveFunctionData："3676 BinConsFail"／"3720 BinConsFail"）
    expect = lambda k: (re.search(secrx, k[0]) and re.search(r'(^|\s)' + re.escape(key1) + '$', k[1])) or \
                       (k[0] == catfmt % x and k[1] == key2)
    extra = [(k, bx.get(k), ax.get(k)) for k in changed if not expect(k)]
    print('     變動的鍵：%s' % [('%s/%s' % k, (bx.get(k) or '')[:40 + 2 * x], (ax.get(k) or '')[:40 + 2 * x]) for k in changed])
    check(changed and not extra, 'W  BinSelect：%s 只變 bin %d 對應的鍵（%d 個；多出的：%s）' %
          (os.path.basename(a.file), x, len(changed), extra[:4]))
    msg2 = page_save(cdp)
    check(msg2.get('saved') is True, 'W  BinSelect：改過之後再存一次 saved')
    after2 = open(a.file, 'rb').read()
    check(after2 == after, 'W  BinSelect：改過之後再存一次，%s 位元組不變（%d → %d bytes）' % (os.path.basename(a.file), len(after), len(after2)))
    back = wait_js(cdp, "(function(){var s=window.HT9045BinSel&&HT9045BinSel.state(); return s && s.panel.%s[%d]===%s ? 'y' : null;})()"
                   % (f, x, json.dumps(new)), 30)
    check(bool(back), 'L  BinSelect：存檔後重讀，頁面 panel.%s[%d] 是新值 %s' % (f, x, new))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9333)
    ap.add_argument('--page', required=True)
    ap.add_argument('--struct', required=True)
    ap.add_argument('--write', action='store_true')
    ap.add_argument('--file', default='')
    ap.add_argument('--edit', default='', help='<id>=<新值>')
    ap.add_argument('--user', default='')
    ap.add_argument('--password', default='')
    ap.add_argument('--gen', default='', help='golden 鍵集來源（預設 FileRW/<struct>.gen.inc）')
    ap.add_argument('--allow', default='', help='正則：已知的 golden 差異鍵（例 "^Event Log/"：golden Now() 補日期）')
    ap.add_argument('--derived', default='', help='正則（比對「鍵=」前的鍵名）：改 --edit 那一格時 golden 連動事件會一起改的鍵（例 Contact 的 edDieForcePerPinGChange 重算 Torque 等）；這些行另外印出來核對，其餘仍只准差一行')
    a = ap.parse_args()
    if a.write and not (a.user and a.password and a.file and a.edit):
        check(False, 'W  --write 需要 --file、--edit、--user、--password（golden A02：Operator 不能存）')
        return
    if a.user:
        check(bool(ws_login(a.port, a.user, a.password)), 'auth.login %s' % a.user)
    edge, prof, dws = launch_edge(a.dbg)
    try:
        cdp = Cdp(dws)
        cdp.call('Page.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (a.port, a.page)})
        g = wait_js(cdp, "window.HT9045Page && HT9045Page.golden && HT9045Page.golden().page ? "
                         "JSON.stringify({struct:HT9045Page.golden().struct, kinds:HT9045Page.golden().kinds, "
                         "mustSend:HT9045Page.golden().page.mustSend}) : null", 90)
        check(bool(g), 'R  頁面走 C 路讀到 editlist.get 的回應')
        if not g:
            return
        g = json.loads(g)
        check(g['struct'] == a.struct, 'R  struct = %s（實際 %s）' % (a.struct, g['struct']))
        kinds = g['kinds']
        print('     值已套上畫面：%d 個元件' % len(kinds))
        miss = [m for m in g['mustSend'] if m not in kinds]
        check(not miss, 'R  mustSend %d 個全部在頁面上且有值（缺：%s）' % (len(g['mustSend']), miss[:10]))
        sample = json.loads(cdp.eval(
            "(function(){var p=HT9045Page.golden().page, n=0, bad=[], noid=[];"
            "Object.keys(p.lists).forEach(function(l){(p.lists[l].entries||[]).forEach(function(e){"
            " if(!e.id){noid.push(e.group+'/'+e.key); return;} var el=document.getElementById(e.id); if(!el){bad.push(e.id+':no element'); return;} n++;"
            " if(e.text!==undefined && el.tagName==='INPUT' && el.type!=='checkbox' && el.value!==e.text && e.itemIndex===undefined) bad.push(e.id+':'+el.value+'!='+e.text);"
            " if(e.checked!==undefined){var c=el.tagName==='INPUT'?el:el.querySelector('input'); if(c&&c.checked!==e.checked) bad.push(e.id+':checked');}"
            " if(e.itemIndex!==undefined && el.tagName==='SELECT' && e.itemIndex>=0 && el.selectedIndex!==e.itemIndex) bad.push(e.id+':idx');"
            "});}); return JSON.stringify({n:n,bad:bad,noid:noid});})()"))
        nlists = cdp.eval("Object.keys(HT9045Page.golden().page.lists||{}).length")
        # Steven 20260925：清單可以登錄了但 0 筆（golden elContact 沒有任何 Add()）——看總筆數，不是看份數
        nentries = cdp.eval("(function(){var l=HT9045Page.golden().page.lists||{}, t=0; Object.keys(l).forEach(function(k){t+=(l[k].entries||[]).length;}); return t;})()")
        check((sample['n'] > 0 or nentries == 0) and not sample['bad'],
              'R  清單值 %d 筆與畫面一致（清單 %s 份、共 %s 筆；不一致：%s）' % (sample['n'], nlists, nentries, sample['bad'][:8]))
        check(not sample['noid'], 'R  每筆清單都有名稱（沒有名稱的：%s）' % sample['noid'][:8])
        perm = json.loads(cdp.eval(
            "(function(){var p=HT9045Page.golden().page.proxies, off=[], on=[], n0=0, n1=0;"
            "Object.keys(p).forEach(function(id){ var el=document.getElementById(id); if(!el) return;"
            " if(p[id].editable===false){ n0++; [el].concat([].slice.call(el.querySelectorAll('input,select,button,textarea'))).forEach(function(x){"
            "  if('disabled' in x && !x.disabled) off.push(id);}); }"
            " else if(p[id].editable===true && ('disabled' in el)){ n1++;"
            "  if(el.disabled && !(el.parentElement && el.parentElement.closest('[aria-disabled=\"true\"]'))) on.push(id);} });"
            " return JSON.stringify({n0:n0,n1:n1,off:off,on:on});})()"))
        check(not perm['off'], 'R  不可改的替身 %d 個全部停用（還能操作的：%s）' % (perm['n0'], perm['off'][:8]))
        check(not perm['on'], 'R  可改的替身 %d 個都可操作（被停用的：%s）' % (perm['n1'], perm['on'][:8]))
        if a.struct == 'BinSelect':      # Steven 20260925：bin 格子另外比（值在回應 bin，不在替身）
            binsel_read_checks(cdp)
        if not a.write:
            return
        before = open(a.file, 'rb').read()
        msg1 = page_save(cdp)
        print('     第一次存檔（原值）：%s' % msg1)
        check(msg1.get('saved') is True, 'W  原值存檔 saved')
        after1 = open(a.file, 'rb').read()
        # 第一次原值存檔可能「正規化」：檔案若不是 golden 寫的（舊版／B 路），golden HTEditList 會用自己的格式重寫
        # （iDecimalPoint 預設 6 → 0.500 變 0.500000）並補上缺的鍵。只允許這兩種差異：同鍵數值相等、或新增的鍵。
        gen = a.gen or os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))),
                                    'FileRW', a.struct + '.gen.inc')
        gkeys = golden_keys(gen) if os.path.exists(gen) else None
        norm = semantic_diff(before, after1, gkeys, a.allow or None)
        check(gkeys is not None, 'W  golden 鍵集：%s（%s 組）' % (gen, len(gkeys) if gkeys else 0))
        check(not norm['bad'], 'W  原值存檔只有正規化差異（數值相等 %d 行、golden 補鍵 %d 個 %s、允許 %s；其他差異：%s）' %
              (norm['same'], len(norm['added']), norm['added'][:6], norm['allowed'], norm['bad'][:6]))
        msg1b = page_save(cdp)
        check(msg1b.get('saved') is True, 'W  再存一次原值 saved')
        after1b = open(a.file, 'rb').read()
        check(after1b == after1, 'W  G1：golden 寫過之後再存原值，%s 位元組不變（%d → %d bytes）' %
              (os.path.basename(a.file), len(after1), len(after1b)))
        if a.struct == 'BinSelect' and a.edit.startswith('bincell='):   # Steven 20260925：從頁面改一格 bin
            binsel_write(cdp, a, after1b)
            return
        wid, val = a.edit.split('=', 1)
        old = cdp.eval("document.getElementById(%s).value" % json.dumps(wid))
        msg2 = page_save(cdp, (wid, val))
        print('     第二次存檔（%s %s→%s）：%s' % (wid, old, val, msg2))
        check(msg2.get('saved') is True, 'W  改一筆存檔 saved')
        after2 = open(a.file, 'rb').read()
        x = after1b.decode('cp950', 'replace').splitlines()
        y = after2.decode('cp950', 'replace').splitlines()
        diff = [(p, q) for p, q in zip(x, y) if p != q]
        if a.derived:   # Steven 20260925：公式頁（golden OnChange 連動重算衍生欄位）
            import re
            der = [d for d in diff if re.search(a.derived, d[1].split('=', 1)[0].strip())]
            main_ = [d for d in diff if d not in der]
            check(len(x) == len(y) and len(main_) == 1, 'W  改一筆後 %s 非連動鍵只差一行（實際：%s）' % (os.path.basename(a.file), main_[:5]))
            print('     golden 連動重算 %d 行（逐行核對公式）：%s' % (len(der), der))
        else:
            check(len(x) == len(y) and len(diff) == 1, 'W  改一筆後 %s 只差一行（實際：%s）' % (os.path.basename(a.file), diff[:5]))
        # golden InitialDataToEdit 用自己的小數位數重新顯示（10.300 → 10.300000）→ 兩邊都是數字時比數值
        back = wait_js(cdp, "(function(){var v=document.getElementById(%s).value, w=%s;"
                            " return (v===w || (!isNaN(parseFloat(v)) && Math.abs(parseFloat(v)-parseFloat(w))<1e-9)) ? 'y:'+v : null;})()"
                       % (json.dumps(wid), json.dumps(val)), 30)
        check(bool(back), 'L  存檔後重讀，畫面是新值（%s）' % (back or cdp.eval("document.getElementById(%s).value" % json.dumps(wid))))
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)


if __name__ == '__main__':
    main()
    print('FAIL %d' % len(FAILS) if FAILS else 'ALL PASS')
    sys.exit(len(FAILS))
