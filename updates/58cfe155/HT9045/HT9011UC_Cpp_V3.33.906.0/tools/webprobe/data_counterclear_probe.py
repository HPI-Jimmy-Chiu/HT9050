# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_counterclear_probe.py -- Data.CounterClear.html（golden V912 cCounterClear.cpp
#  TfCounterClear）的 e2e probe。
#
#  Steven 團隊 20260925.
#
#  用真的瀏覽器（headless Edge ＋ DevTools 協定）開頁面，驗：
#    A  權限：畫面 8 個清除框的 enable ＝ config\Security_new.def [Counter Clear]（golden FormShow :92-114）
#    S  Select All：golden cbSelectAllMouseUp（:50-77）只勾有授權的框、Caption 變 UnSelect All；
#       取消任一框 → golden cbAlarmDataMouseUp（:79-88）把 Select All 取消、Caption 回 Select All
#    G  C++ 端閘：停用的框 counterclear.click／counterclear.exe 都被拒（not-enabled）
#    D  Execute dryRun：回傳的呼叫順序＝golden spbExeClick（:425-453），一對 Clarn_Data(10)；計數不變
#       （AI(W906-Q57-TRIAGE) 20260930：Time Data 除外 —— 0b38b6b5 之後照 golden UpdateRecordScreen 每拍累加，只許變大，見 counters_same）
#    X  Execute 真的清（--write 才做）：只清一類，對應計數歸零、其他類不動
#
#  ⚠ --write 會寫 D:\HT9045\system\lastdata.dat（在 restore 範圍）以及
#    D:\HT9045_Log\QtyData\YYYYMM\<PC>_<date>.csv（Clarn_Data 的 QtyLog，**不在** restore 範圍）。
#    所以預設不做；要跑請先自己備份 D:\HT9045_Log\QtyData。
#
#  用法：
#      set W906_PWBOOK_PATH=<scratch book>  &  build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\data_counterclear_probe.py --port 8046 --user S12TEST --password S12PW [--write]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import shutil
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
PAGE = 'Data.CounterClear.html'
SECURITY = r'D:\HT9045\config\Security_new.def'
# golden cAuthority.cpp funCounterClr[] 的順序 ↔ 畫面框（golden FormShow :94-101）
AUTH_KEYS = ['Alarm Data', 'Tester Category', 'Scanner Category', 'Loading Count',
             'Contact Count(Kind)', 'Contact Count(Total)', 'Sorting Count', 'Time Data']
BOXES = ['cbAlarmData', 'cbTestCategory', 'cbScanner', 'cbLoadingCount', 'cbContactCountCurr',
         'cbContactCountHis', 'cbSortingCount', 'cbTimeData']


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def read_auth():
    """Security_new.def 的 [Counter Clear]；沒寫的鍵 golden CheckAndReadIniData 預設 1。"""
    txt = open(SECURITY, 'rb').read().decode('cp950', errors='replace')
    sec, vals = None, {}
    for line in txt.splitlines():
        s = line.strip()
        if s.startswith('[') and s.endswith(']'):
            sec = s[1:-1]
            continue
        if sec == 'Counter Clear' and '=' in s:
            k, v = s.split('=', 1)
            vals[k.strip()] = v.strip()
    return [vals.get(k, '1') not in ('0', '') for k in AUTH_KEYS]


def dom(cdp):
    return json.loads(cdp.eval(
        "(function(){var o={};['cbAlarmData','cbTestCategory','cbScanner','cbLoadingCount','cbContactCountCurr',"
        "'cbContactCountHis','cbSortingCount','cbTimeData','cbSelectAll'].forEach(function(id){"
        "var l=document.getElementById(id), c=l.querySelector('input');"
        "o[id]={checked:c.checked, disabled:c.disabled, caption:l.textContent.trim()};}); return JSON.stringify(o);})()"))


def click(cdp, bid):
    cdp.eval("document.getElementById(%s).querySelector('input').click()" % json.dumps(bid))
    time.sleep(0.2)
    wait_js(cdp, "!HT9045CounterClear.busy()", 15)


# AI(W906-Q57-TRIAGE) 20260930: Q57 r4a／r4g「D dryRun 後計數不變」FAIL＝這支探針（267425bc，20260925）寫在 0b38b6b5（S113，20260926）之前。
#   counters.timeData.SystemAccSecond01＝LastSet.SystemAccSecond[0..1][0..7] 的總和（JsonBridge/ChanAction.cpp:562-566），現在 wb_serve 每拍照
#   golden TfMain::UpdateRecordScreen（V912 main.cpp:8584-8687，Timer1 :3284；移植 FileRW/MainRecord.cpp:132-225）把經過的毫秒數 P 加進去：
#   PowerOn 一定加；Start＋Home／Contact／Product 其一（運轉中）、Pause（停著、機內有 IC）、Jam（告警框開著）、SystemNG 看狀態 ⇒ 每一組一拍
#   最多加 5 項、[0]、[1] 兩組 ⇒ 總和最多長 10×經過時間。開頁到 dryRun 之間它本來就會變大。其他類只在生產／清除時變，照舊要一模一樣。
#   Time Data 改成：沒有被清掉、只許變大，而且不超過 10×經過時間＋5 秒；某一格 32-bit long 環繞時總和少 2^32 ⇒ 差值以 2^32 取餘。
TIMEDATA_KEY = 'timeData'


def counters_same(before, after, elapsed_s):
    """回傳 (ok, 說明)：timeData 以外全部相同；timeData 每個值只許照 golden UpdateRecordScreen 的速度長大。"""
    bad = [k for k in sorted(set(before) | set(after)) if k != TIMEDATA_KEY and before.get(k) != after.get(k)]
    tb, ta = before.get(TIMEDATA_KEY) or {}, after.get(TIMEDATA_KEY) or {}
    limit = int(10 * elapsed_s * 1000) + 5000
    grow = {}
    for k in sorted(set(tb) | set(ta)):
        if not isinstance(tb.get(k), int) or not isinstance(ta.get(k), int):
            bad.append('%s.%s' % (TIMEDATA_KEY, k))
            continue
        g = (ta[k] - tb[k]) % (1 << 32)
        grow[k] = g
        if g > limit:
            bad.append('%s.%s（%d → %d）' % (TIMEDATA_KEY, k, tb[k], ta[k]))
    return not bad, '不同：%s；timeData 長了 %s ms（上限 %d）' % (bad, grow, limit)


def raw(cdp, name, extra):
    """直接送 WS（繞過頁面的 disabled），回 {ok, res|err}。"""
    js = ("HT9045Recipe.rawCmd(%s, %s).then(function(m){return JSON.stringify({ok:true,res:m});},"
          "function(e){return JSON.stringify({ok:false,err:String(e&&e.message||e)});})") % (
        json.dumps(name), json.dumps(extra))
    return json.loads(cdp.eval(js, timeout=30))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9335)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--write', action='store_true', help='真的清一類（會寫 restore 範圍外的 QtyLog，見檔頭）')
    ap.add_argument('--family', default='cbTimeData', help='--write 時清哪一框（預設 cbTimeData）')
    a = ap.parse_args()

    auth = read_auth()
    print('Security_new.def [Counter Clear]：%s' % ', '.join('%s=%d' % (b, v) for b, v in zip(BOXES, auth)))
    if a.user and a.password:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')
        t_open = time.monotonic()   # AI(W906-Q57-TRIAGE) 20260930: before（開頁 counterclear.get 的 counters）最早就在這之後取 ⇒ counters_same 的經過時間從這裡算
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (a.port, PAGE)})
        ok = wait_js(cdp, "window.HT9045CounterClear && HT9045CounterClear.loads()>0", 60)
        check(bool(ok), '開頁 counterclear.get 成功（lastError=%r）' % (
            cdp.eval("window.HT9045CounterClear ? HT9045CounterClear.lastError() : 'no wire'")))
        if not ok:
            return len(FAILS)
        st = json.loads(cdp.eval("JSON.stringify(HT9045CounterClear.state())"))

        # ---- A 權限 --------------------------------------------------------
        check(st['auth'] == auth, 'A  C++ authCounterClr[0..7] %s ＝ Security_new.def %s' % (st['auth'], auth))
        d = dom(cdp)
        bad = [b for b, v in zip(BOXES, auth) if d[b]['disabled'] == v or st['widgets'][b]['enabled'] != v]
        check(not bad, 'A  8 個清除框 enable 照 golden FormShow（不符：%s）' % bad)
        bad = [b for b, v in zip(BOXES, auth) if not v and d[b]['checked']]
        check(not bad, 'A  未授權的框未勾（golden FormShow :102-109；違反：%s）' % bad)
        check(not d['cbSelectAll']['disabled'], 'A  cbSelectAll 可按（dfm 預設 Enabled）')
        caps = {b: d[b]['caption'] for b in BOXES}
        check(caps['cbAlarmData'] == 'Alarm and Lot Data' and caps['cbTimeData'] == 'Time Data',
              'A  Caption 取自 C++（dfm）：%s' % caps)
        before = st['counters']

        # ---- S Select All --------------------------------------------------
        if d['cbSelectAll']['checked']:
            click(cdp, 'cbSelectAll')       # 先回到未全選
        click(cdp, 'cbSelectAll')
        d = dom(cdp)
        bad = [b for b, v in zip(BOXES, auth) if d[b]['checked'] != v]
        check(not bad and d['cbSelectAll']['caption'] == 'UnSelect All',
              'S  Select All：有授權的全勾、未授權不動，Caption=%r（不符：%s）' % (d['cbSelectAll']['caption'], bad))
        first = next((b for b, v in zip(BOXES, auth) if v), None)
        click(cdp, first)
        d = dom(cdp)
        check(not d[first]['checked'] and not d['cbSelectAll']['checked'] and d['cbSelectAll']['caption'] == 'Select All',
              'S  取消 %s → Select All 取消、Caption=%r（golden cbAlarmDataMouseUp）' % (first, d['cbSelectAll']['caption']))
        click(cdp, 'cbSelectAll')           # 全選
        click(cdp, 'cbSelectAll')           # 全不選
        d = dom(cdp)
        bad = [b for b in BOXES if d[b]['checked']]
        check(not bad and d['cbSelectAll']['caption'] == 'Select All', 'S  UnSelect All → 全部取消（殘留：%s）' % bad)
        srv = json.loads(cdp.eval("JSON.stringify(HT9045CounterClear.state().widgets)"))
        check(all(srv[b]['checked'] == d[b]['checked'] for b in BOXES), 'S  畫面勾選＝C++ fCounterClear 的 Checked')

        # ---- G C++ 端閘 ----------------------------------------------------
        denied = next((b for b, v in zip(BOXES, auth) if not v), None)
        if denied:
            r = raw(cdp, 'counterclear.click', {'tag': denied, 'value': json.dumps({'checked': True})})
            check(not r['ok'] and 'not-enabled' in r.get('err', ''), 'G  click 停用框 %s 被拒：%s' % (denied, r))
            r = raw(cdp, 'counterclear.exe', {'value': json.dumps({'checked': {denied: True}, 'dryRun': True})})
            check(not r['ok'] and 'not-enabled' in r.get('err', ''), 'G  exe 帶停用框 %s 被拒：%s' % (denied, r))
        else:
            print('  SKIP  G：Security_new.def 沒有未授權的框')

        # ---- D dryRun ------------------------------------------------------
        pick = [b for b in ('cbLoadingCount', 'cbTimeData') if auth[BOXES.index(b)]]
        for b in pick:
            click(cdp, b)
        r = json.loads(cdp.eval("HT9045CounterClear.execute({dryRun:true}).then(function(r){return JSON.stringify(r);},"
                                "function(e){return JSON.stringify({err:String(e.message||e)});})", timeout=30))
        exp = ['MyDBIProductionData("Clear Count executed")', 'Clarn_Data(10, "Manual clear count")']
        if 'cbLoadingCount' in pick:
            exp += ['ClearCount(ctLoadingCounts)', 'ClearCount(ctIndexCount)']
        if 'cbTimeData' in pick:
            exp += ['ClearCount(ctTimeData)']
        exp += ['Clarn_Data(10, "Manual clear count done")', 'MyDBIProcess("Process", "Counter Clear has been executed!!")']
        check(r.get('dryRun') is True and r.get('executed') is False and r.get('would') == exp,
              'D  dryRun 呼叫順序＝golden spbExeClick：%s' % r.get('would', r))
        same, why = counters_same(before, r.get('counters') or {}, time.monotonic() - t_open)   # AI(W906-Q57-TRIAGE) 20260930: 見 counters_same
        check(same, 'D  dryRun 後計數不變（Time Data 照 golden UpdateRecordScreen 每拍累加，只許變大；%s）' % why)
        for b in pick:
            click(cdp, b)                   # 還原勾選

        # ---- X 真的清 -------------------------------------------------------
        if not a.write:
            print('  SKIP  X：沒帶 --write（真的清會寫 D:\\HT9045_Log\\QtyData，restore 範圍外）')
        else:
            fam = a.family
            famkey = {'cbLoadingCount': 'loadingCount', 'cbContactCountCurr': 'contactCurr',
                      'cbContactCountHis': 'contactHis', 'cbSortingCount': 'sortingCount',
                      'cbTimeData': 'timeData', 'cbTestCategory': 'testerCategory'}.get(fam)
            if not auth[BOXES.index(fam)] or not famkey:
                check(False, 'X  %s 沒有授權或沒有對應計數' % fam)
            else:
                click(cdp, fam)
                r = json.loads(cdp.eval("HT9045CounterClear.execute().then(function(r){return JSON.stringify(r);},"
                                        "function(e){return JSON.stringify({err:String(e.message||e)});})", timeout=60))
                check(r.get('executed') is True, 'X  執行 %s：%s' % (fam, r.get('err', 'executed')))
                after = r.get('counters') or {}

                def zero(v):
                    return all(zero(x) for x in v.values()) if isinstance(v, dict) else \
                        all(zero(x) for x in v) if isinstance(v, list) else v == 0
                check(zero(after.get(famkey, {'x': 1})), 'X  %s 歸零：%s（前 %s）' % (famkey, after.get(famkey), before.get(famkey)))
                # AI(W906-Q57-TRIAGE) 20260930: 清的不是 timeData 時，timeData 一樣照 golden 每拍累加（counters_same）；清掉的那一類不比
                rest_b = {k: v for k, v in before.items() if k != famkey}
                rest_a = {k: v for k, v in after.items() if k != famkey}
                same, why = counters_same(rest_b, rest_a, time.monotonic() - t_open)
                check(same, 'X  其他類不動（%s）' % why)
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)
    print('data_counterclear_probe: %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
