# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_observer_probe.py -- Data.Observer.html（golden V912 cObserver.cpp TfObserver）
#  的 e2e probe。
#
#  Steven 團隊 20260925.
#
#  用真的瀏覽器（headless Edge ＋ DevTools 協定）開頁面，驗：
#    F  頁面原始碼沒有舊的假資料（模擬器的 0017 days／HT9046AT-136、寫死的 JAM0217、tcGrid 形狀腳本、前端 Filter）
#    O  開頁送出 observer.get open；Operating Information／Version 的 15 格（14 格＋APHeadLabel18）
#       畫面＝回應（有 noSource 的顯示 "---" 且 title 帶原因）；pnlDayJamRate 可見性＝回應
#    V  回應的值＝移植樹來源重算：ConvertMSecToTime／ConvertSecondToSPC／ProcessRunInfo 公式
#       從回應 sources（LastSet 欄位）算；而 sources 的 LastSet 欄位＝ system\lastdata.dat 在
#       C++ 回報的位移上讀出來的值（golden ReadLastDataFile cprod.cpp:1703 的讀法）。
#       labVersion＝asHandlerVersion(+"."+SVNRevision)，或 C++ 判定沒有來源（"---"）
#    E  engine 接的 5 格（labModel／labSerialNo／labMachineID／labDeviceName）＝回應 captions
#    T  golden Timer1：每秒一次 act=timer，ticks 會增加、畫面＝最新回應；權杖不被長期佔住
#       （另一條連線拿得到 control.acquire）
#    C  Tester Category：點頁籤 → act=tab 1 → 16 個 TTMyTray 畫面＝回應；數字模式的值＝
#       直接從 system\Arm0.dat／Arm1.dat 照 golden TEST_CATEGORY::UpdataCount（cSocket.cpp:1258）＋
#       WriteCategoryData（golden :3275）算；切 Row-B、Socket Real Number、Head % 會重取
#    S  System Message：點頁籤 → act=tab 3 → strngrdEventLog 畫面＝回應；回應＝用 golden V912
#       SplitEventLogCsvLine（:3825）解析 EventLogTxt\年\月\最後一個 CSV 的結果；JAM only＋Query、
#       點 lstEventLog 第一個檔、換月份都會重取且值對得上檔案
#    W  整段期間 lastdata*.dat／Arm*.dat／Gerneral.ini／Security_new.def／EventLogTxt 的 SHA256 不變
#       AI(W906-Q57-TRIAGE) 20260930：快照在登入「之後」拍（登入照 golden 寫 MES2140／MES2144，不是這一頁寫的）；EventLogTxt 只往後多了
#       golden 定時記錄（PERIODIC_UNITS）不算。V 的 SystemAccSecond 改成「記憶體只許比檔案多、不超過檔案年紀」（0b38b6b5 之後每拍累加）；
#       O 的開頁 15 格改成跟「最近一次畫上去的回應」比（timer 每秒改 labPowerOnTime 等）。見各處 AI(W906-Q57-TRIAGE)。
#
#  只讀，不寫任何檔。
#  用法：
#      set W906_PWBOOK_PATH=<scratch book>  &  build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\data_observer_probe.py --port 8046 [--user S12TEST --password S12PW]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import glob
import hashlib
import json
import os
import re
import shutil
import struct
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402
from cmd_probe import ws_handshake, send_text, read_frames     # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
PAGE = 'Data.Observer.html'
SYSDIR = r'D:\HT9045\system'
CAPS = ['labPowerOnTime', 'labRunningTime', 'labProductTime', 'labLoadingCount', 'labMUBA', 'labMTBA', 'labMTBF',
        'pnlDayJamRate', 'labVersion', 'labFactory', 'pnlGPIBVersion', 'pnlESDVersion', 'pnlATCVersion',
        'pnlTTLRS232Version', 'APHeadLabel18']
MAX_ROW, MAX_COL = 4, 8        # MachineType.h MAX_SOCKET_ROW／MAX_SOCKET_COL
stStartTime, stPauseTime, stPowerOn, stProductTime, stJamTime = 0, 1, 2, 3, 4   # MachineType.h:673 eSystemTime


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


# ---- C 語意的整數運算（golden 是 32-bit long／int） ------------------------------------------------
def cdiv(a, b):
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b >= 0) else -q


def cmod(a, b):
    return a - b * cdiv(a, b)


def wrap32(x):
    x &= 0xFFFFFFFF
    return x - (1 << 32) if x & 0x80000000 else x


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def msec_to_time(s):          # cpublic.cpp:193 ConvertMSecToTime
    ms = cmod(s, 1000)
    secs = cmod(cdiv(s, 1000), 60)
    mins = cdiv(cdiv(s, 1000), 60)
    hours = cdiv(mins, 60)
    mins = cmod(mins, 60)
    days = cdiv(hours, 24)
    hours = cmod(hours, 24)
    return '%04d days %02d:%02d:%02d.%03d' % (days, hours, mins, secs, ms)


def sec_to_spc(s):            # cpublic.cpp:165 ConvertSecondToSPC
    secs = cmod(s, 60)
    mins = cdiv(s, 60)
    hours = cdiv(mins, 60)
    mins = cmod(mins, 60)
    hours = cmod(hours, 24)
    return '%02d:%02d:%02d' % (hours, mins, secs)


def pct(n, d):                # MachineType.h:1713 ChangeToPercentage
    return '0.00%' if d == 0 else '%0.2f%%' % ((float(n) / float(d)) * 100.0)


def pct_close(a, b):
    try:
        return abs(float(a.rstrip('%')) - float(b.rstrip('%'))) <= 0.0100001
    except Exception:
        return a == b


def dec(bs):
    """C++ W906Obs_ToUtf8 的同一個政策：合法 UTF-8 原樣、否則 CP950、否則 U+FFFD。"""
    try:
        return bs.decode('utf-8')
    except UnicodeDecodeError:
        pass
    try:
        return bs.decode('cp950')
    except UnicodeDecodeError:
        return ''.join(chr(c) if c < 0x80 else '\ufffd' for c in bs)


def sha_files(paths):
    out = {}
    for p in paths:
        if os.path.isfile(p):
            out[p] = hashlib.sha256(open(p, 'rb').read()).hexdigest()
    return out


def watched_files(ev_root):
    ps = glob.glob(os.path.join(SYSDIR, 'lastdata*.dat')) + glob.glob(os.path.join(SYSDIR, 'Arm*.dat'))
    ps += [os.path.join(SYSDIR, 'Gerneral.ini'), r'D:\HT9045\config\Security_new.def']
    for dp, _, fs in os.walk(ev_root):
        ps += [os.path.join(dp, f) for f in fs]
    return sorted(ps)


# ---- golden ReadLastDataFile（cprod.cpp:1703-1790）的檔案選擇 --------------------------------------
def lastdata_bytes():
    main = os.path.join(SYSDIR, 'lastdata.dat')
    back = os.path.join(SYSDIR, 'lastdata_backup.dat')
    back2 = os.path.join(SYSDIR, 'lastdata_backup2.dat')
    s1 = os.path.getsize(main) if os.path.exists(main) else 0
    s2 = os.path.getsize(back) if os.path.exists(back) else 0
    # AI(W906-Q57-TRIAGE) 20260930: 多回第三個值＝讀到的那個檔的路徑（V 要看它的修改時間，見 verify_values）
    if s1 == 0 and s2 == 0 and os.path.exists(back2):
        return open(back2, 'rb').read(), 'lastdata_backup2.dat', back2
    if os.path.exists(main):
        data = open(main, 'rb').read()
        if os.path.exists(back) and open(back, 'rb').read() != data:
            return open(back, 'rb').read(), 'lastdata_backup.dat（與 lastdata.dat 不同，golden :1754 改讀備份）', back
        return data, 'lastdata.dat', main
    if os.path.exists(back):
        return open(back, 'rb').read(), 'lastdata_backup.dat', back
    return b'', '（沒有 lastdata）', None


def i32(data, off):
    if off + 4 > len(data):
        return 0
    return struct.unpack_from('<i', data, off)[0]


# ---- golden TArm::ReadFile（cSocket.cpp:537-735）：Pass／Fail／BinCT／IFErr ----------------------
def read_arm(name, bincount):
    main = os.path.join(SYSDIR, name + '.dat')
    back = os.path.join(SYSDIR, name + '_backup.dat')
    empty = {(i, j): (0, 0, [0] * bincount, 0) for i in range(MAX_ROW) for j in range(MAX_COL)}
    if not os.path.exists(main):
        return empty
    data = open(main, 'rb').read()
    if os.path.exists(back) and open(back, 'rb').read() != data:
        data = open(back, 'rb').read()
    size = len(data)
    if size == 2432:
        per, nb, ifx = 19, 15, 18
    elif size == 12800:
        per, nb, ifx = 100, bincount, bincount + 3
    elif size == 13312:
        per, nb, ifx = 104, bincount, bincount + 3
    else:
        per, nb, ifx = size // (MAX_ROW * MAX_COL * 4), bincount, bincount + 3
    v = struct.unpack('<%dI' % (len(data) // 4), data[:len(data) // 4 * 4])
    out = {}
    for i in range(MAX_ROW):
        for j in range(MAX_COL):
            b = (i * MAX_COL + j) * per
            g = lambda k: v[b + k] if b + k < len(v) else 0
            bins = [g(3 + k) if k < nb else 0 for k in range(bincount)]
            out[(i, j)] = (g(0), g(1), bins, g(ifx))
    return out


def update_count(arms, inp):
    """golden TEST_CATEGORY::UpdataCount(true)（cSocket.cpp:1258），None_NN 分支。"""
    nb = inp['iTestBinCount']
    head = [[[0] * MAX_COL for _ in range(MAX_ROW)] for _ in range(2)]
    passh = [[[0] * MAX_COL for _ in range(MAX_ROW)] for _ in range(2)]
    cat = [[[[0] * (nb + 1) for _ in range(MAX_COL)] for _ in range(MAX_ROW)] for _ in range(2)]
    sock = [[0] * MAX_COL for _ in range(MAX_ROW)]
    tcat = [0] * (nb + 1)
    tsock = tpass = 0
    for r in range(inp['shtRow']):
        for c in range(inp['shtCol']):
            if inp['siteMap'][r][c] > 0:
                for a in (0, 1):
                    p, f, bins, ife = arms[a][(r, c)]
                    cat[a][r][c][nb] = ife
                    tcat[nb] += ife
                    for k in range(nb):
                        cat[a][r][c][k] = bins[k]
                        tcat[k] += bins[k]
                    sock[r][c] += p + f
                    head[a][r][c] += p + f
                    tsock += p + f
                    passh[a][r][c] += p
                    tpass += p
    return dict(head=head, passh=passh, cat=cat, sock=sock, tcat=tcat, tsock=tsock, tpass=tpass)


def expect_category(uc, inp, row, mode):
    """golden WriteCategoryData（:3275-3548）非 32-site、數字模式：回傳 {tray: {(x,y): text}}（只放這次寫到的格）。"""
    nb = inp['iTestBinCount']
    e = {k: {} for k in ('mtRowName', 'mtChName', 'mtTotal', 'mtCategoryTotal', 'mtHeadTotal', 'mtPassHead',
                         'mtSockTotal', 'mtPassSocket', 'mtIfError', 'mtCategoryNo')}
    e['mtRowName'][(0, 0)] = 'RowA' if row == 0 else 'RowB'
    for i in range(8):                                       # mtChName->XItem（ctor 8；non-32 FormShow 不改）
        m = inp['siteMap'][row][i]
        e['mtChName'][(i, 0)] = ('CH%d' % m) if m > 0 else '----'
    e['mtTotal'][(0, 0)] = ' Total '
    e['mtTotal'][(0, 2)] = ' Total '
    e['mtTotal'][(0, 1)] = str(uc['tsock'])
    e['mtTotal'][(0, 3)] = str(uc['tpass'])
    e['mtTotal'][(0, 4)] = str(uc['tcat'][nb])
    for k in range(nb):
        e['mtCategoryTotal'][(0, k)] = str(uc['tcat'][k])
    for c in range(inp['maxIndexCol']):
        for a in (0, 1):
            e['mtHeadTotal'][(c * 2 + a, 0)] = str(uc['head'][a][row][c])
            e['mtPassHead'][(c * 2 + a, 0)] = str(uc['passh'][a][row][c])
            e['mtSockTotal'][(c + a, 0)] = str(uc['head'][0][row][c] + uc['head'][1][row][c])
            e['mtPassSocket'][(c + a, 0)] = str(uc['passh'][0][row][c] + uc['passh'][1][row][c])
        e['mtIfError'][(c, 0)] = str(uc['cat'][0][row][c][nb] + uc['cat'][1][row][c][nb])
        if mode == 'rbSocketNumber':
            for k in range(nb):
                e['mtCategoryNo'][(c, k)] = str(uc['cat'][0][row][c][k] + uc['cat'][1][row][c][k])
        else:
            for a in (0, 1):
                for k in range(nb):
                    e['mtCategoryNo'][(c * 2 + a, k)] = str(uc['cat'][a][row][c][k])
    return e


# ---- golden V912 SplitEventLogCsvLine（:3812-3846） --------------------------------------------------
def trim(bs):
    b, e = 0, len(bs)
    while b < e and bs[b] <= 0x20:
        b += 1
    while e > b and bs[e - 1] <= 0x20:
        e -= 1
    return bs[b:e]


def csv_field(bs):
    bs = trim(bs)
    if len(bs) >= 1 and bs[0:1] == b'"':
        bs = bs[1:]
        if len(bs) >= 1 and bs[-1:] == b'"':
            bs = bs[:-1]
        bs = bs.replace(b'""', b'"')
    return bs


def split_csv(line):
    out, start, inq = [], 0, False
    for i in range(len(line)):
        ch = line[i:i + 1]
        if ch == b'"':
            inq = not inq
        elif ch == b',' and not inq:
            out.append(csv_field(line[start:i]))
            start = i + 1
    out.append(csv_field(line[start:]))
    return out


# AI(W906-Q57-TRIAGE) 20260930: golden 自己定時寫進 EventLogTxt 的記錄（不是頁面操作）：UnitName 欄＝cMyDB.cpp MyDBITimeData（"TimeData"）、
#   MyDBITotalLoader（"TimeDataTotalLoader"）、MyDBIUPH（"UPH"），寫法都是 slEventLog->AddTextWithDateTime(SL->CommaText)＋SaveEventLog()
#   （SPIL 格式是 AddTextWithLineNo，UnitName 在第 1 欄）。06f8ef0a（St02-E cMyDB P4）之後這些是 golden 本體、會真的落地。
PERIODIC_UNITS = (b'TimeData', b'TimeDataTotalLoader', b'UPH')


def eventlog_periodic_only(old, new):
    """(ok, 多出來的列)：old 是 new 的開頭（只往後加），而且多出來的每一列都是 PERIODIC_UNITS 的記錄。"""
    if old is None or new is None or len(new) < len(old) or new[:len(old)] != old:
        return False, []
    extra = [ln for ln in new[len(old):].splitlines() if ln.strip()]
    for ln in extra:
        fs = split_csv(ln)
        if not ((len(fs) > 2 and fs[2] in PERIODIC_UNITS) or (fs and fs[0] in PERIODIC_UNITS)):
            return False, extra
    return bool(extra), extra


def load_lines(path):
    return open(path, 'rb').read().splitlines()      # TStringList::LoadFromFile：CR、LF、CRLF 都斷行


def expect_eventlog(path, filt, cols, header_before):
    """golden GetEventLogText（:3857-4027）非 SPIL：回傳期望的 cells（rows×cols，文字）。"""
    if not path or not os.path.isfile(path):
        rows = [[''] * cols for _ in range(2)]
        rows[0] = header_before
        rows[1][1] = 'No Record!!'
        return rows
    lines = load_lines(path)
    if len(lines) > 10000:
        rows = [header_before[:], [''] * cols]
        rows[1][1] = 'Event log over 10000 rows!!'
        return rows
    grid = []
    if filt == 'All Data':
        for ln in lines:
            fs = [dec(x) for x in split_csv(ln)]
            grid.append([(fs[c] if c < len(fs) else '') for c in range(cols)])
        return grid
    m = []
    for ln in lines[1:]:
        fs = split_csv(ln)
        if filt in ('JAM only', 'WAR only', 'MES only'):
            if len(fs) > 3 and fs[3].startswith(filt[:3].encode()):
                m.append(fs)
        else:
            if len(fs) > 2 and filt.encode('cp950') in fs[2]:
                m.append(fs)
    grid = [header_before[:]]
    for fs in m:
        fs = [dec(x) for x in fs]
        grid.append([(fs[c] if c < len(fs) else '') for c in range(cols)])
    grid.append([''] * cols)
    if not m:
        grid[1][1] = 'No Record!!'
    return grid


def csv_list(root, year, month):
    d = os.path.join(root, year, month)
    out = []
    for dp, dns, fs in os.walk(d):
        dns.sort()
        for f in sorted(fs):
            if f.lower().endswith('.csv'):
                out.append(os.path.join(dp, f))
    return out


# ---- DOM 讀取 ------------------------------------------------------------------------------------
JS_CAPS = ("(function(){var o={};%s.forEach(function(id){var e=document.getElementById(id);if(!e){o[id]=null;return;}"
           "var c=e.querySelector(':scope > .pnlCap');o[id]={text:(c?c.textContent:e.textContent),title:e.title,"
           "display:getComputedStyle(e).display,obs:e.getAttribute('data-obs')};});return JSON.stringify(o);})()")


def dom_caps(cdp, ids):
    return json.loads(cdp.eval(JS_CAPS % json.dumps(ids)))


# AI(W906-Q57-TRIAGE) 20260930: 畫面＋頁面最近一次畫上去的回應（window.__observer.last）＋最近一次 full 回應（.data）在「同一次 eval」
#   裡一起讀。頁面 ht9045_observer_wire.js sendNow 是 st.last=d 之後同步 render(d)（:171-173），JS 單執行緒 ⇒ 同一次 eval 讀到的畫面
#   一定就是 last 畫的；分兩次 eval 讀，中間可能插進一拍 timer（見 verify_caps 的註解）。
JS_CAPS_SNAP = ("(function(){var o={};%s.forEach(function(id){var e=document.getElementById(id);if(!e){o[id]=null;return;}"
                "var c=e.querySelector(':scope > .pnlCap');o[id]={text:(c?c.textContent:e.textContent),title:e.title,"
                "display:getComputedStyle(e).display,obs:e.getAttribute('data-obs')};});var s=window.__observer||{};"
                "return JSON.stringify({dom:o,last:s.last||null,data:s.data||null});})()")


def caps_snapshot(cdp, ids):
    return json.loads(cdp.eval(JS_CAPS_SNAP % json.dumps(ids)))


def dom_trays(cdp):
    return json.loads(cdp.eval(
        "(function(){var o={};document.querySelectorAll('#tcTrays .tcTray').forEach(function(t){var x=+t.getAttribute('data-x'),"
        "y=+t.getAttribute('data-y'),cells=[];for(var j=0;j<y;j++)cells.push([]);t.querySelectorAll('.tcCell').forEach(function(c){"
        "cells[+c.getAttribute('data-cy')][+c.getAttribute('data-cx')]=c.textContent;});o[t.id]={x:x,y:y,cells:cells};});"
        "var rg=[].map.call(document.querySelectorAll('#rgRowNo label'),function(l){return {text:l.textContent,checked:l.querySelector('input').checked};});"
        "var rb={};document.querySelectorAll('#tcTrays input[name=tcForm]').forEach(function(i){rb[i.id]={checked:i.checked,disabled:i.disabled};});"
        "return JSON.stringify({trays:o,rg:rg,rb:rb});})()"))


def dom_eventlog(cdp):
    return json.loads(cdp.eval(
        "(function(){var g=[];document.querySelectorAll('#evGrid tr').forEach(function(tr){var r=[];"
        "tr.querySelectorAll('th,td').forEach(function(c){r.push(c.textContent);});g.push(r);});"
        "var l=[].map.call(document.querySelectorAll('#lstEventLog .obsLi'),function(d){return {text:d.textContent,sel:d.classList.contains('sel')};});"
        "function s(id){var e=document.getElementById(id);return e&&e.selectedIndex>=0?{i:e.selectedIndex,t:e.options[e.selectedIndex].text,n:e.options.length}:null;}"
        "return JSON.stringify({grid:g,list:l,year:s('cbbEventLogYear'),month:s('cbbMonth'),filter:s('cbbFilter')});})()"))


def st(cdp):
    return json.loads(cdp.eval("JSON.stringify({loads:(window.__observer||{}).loads,ticks:(window.__observer||{}).ticks,"
                               "error:(window.__observer||{}).error,last:(window.__observer||{}).last})"))


def full(cdp):
    return json.loads(cdp.eval("JSON.stringify((window.__observer||{}).data)"))


def act_and_wait(cdp, js, label, pred="true"):
    loads = st(cdp)['loads']
    cdp.eval(js)
    ok = wait_js(cdp, "window.__observer && window.__observer.loads>%d && window.__observer.data && (%s) ? 1 : 0" % (loads, pred), 30)
    check(bool(ok), '%s -> 重取 observer.get（loads %d -> %s）' % (label, loads, st(cdp)['loads']))
    return full(cdp) if ok else None


# ---- 各區驗證 ------------------------------------------------------------------------------------
def verify_caps(cdp, d, label):
    # AI(W906-Q57-TRIAGE) 20260930: Q57 r4a FAIL「labPowerOnTime 畫面 '…19.714' != 回應 '…15.803'、labMTBF 同」＝比錯回應。0b38b6b5（S113）
    #   之後 PowerOn 照 golden 每拍累加（main.cpp:8584-8687），頁面 golden Timer1（1000 ms）的 act=timer 回應會把 labPowerOnTime／labMTBF…
    #   改成新值；開頁之後等了 1.5 秒，畫面已經是第 3、4 拍的值，而 .data 仍是開頁那一包（full 回應才換）。
    #   改成：同一次 eval 取畫面與頁面最近一次畫上去的回應（caps_snapshot），每一格跟「最近一次帶了這一格的回應」比；
    #   timer 不帶的格子仍跟開頁那一包比。開頁那一包自己的值另由 V 用 golden 公式驗。
    snap = caps_snapshot(cdp, CAPS)
    dom = snap.get('dom') or {}
    d = snap.get('data') or d
    last = snap.get('last') or d
    bad = []
    for id_ in CAPS:
        e = dom.get(id_)
        if e is None:
            bad.append(id_ + ' 不在頁面')
            continue
        lc, ln = (last.get('captions') or {}), (last.get('noSource') or {})
        caps_src, ns_src = (lc, ln) if (id_ in lc or id_ in ln) else (d['captions'], d['noSource'])
        if id_ in ns_src:
            if e['text'] != '---' or ns_src[id_][:20] not in e['title']:
                bad.append('%s 應為 --- 且 title 帶原因（畫面 %r）' % (id_, e['text']))
        elif e['text'] != caps_src.get(id_):
            bad.append('%s 畫面 %r != 回應（%s）%r' % (id_, e['text'], last.get('act') if caps_src is lc else 'open', caps_src.get(id_)))
    check(not bad, '%s：15 格畫面＝回應（noSource：%s）%s' % (label, ','.join(sorted(d['noSource'])) or '無', ('' if not bad else '；' + '; '.join(bad[:5]))))
    vis = d['visible']['pnlDayJamRate']
    check((dom['pnlDayJamRate']['display'] != 'none') == vis, '%s：pnlDayJamRate 可見性＝回應 %s（golden FormShow :644-648 bVTESTFunction）' % (label, vis))
    return dom


def verify_values(d):
    s, cap = d['sources'], d['captions']
    sas = s['SystemAccSecond0']
    exp = {
        'labPowerOnTime': msec_to_time(sas[stPowerOn]),
        'labRunningTime': msec_to_time(sas[stStartTime]),
        'labProductTime': msec_to_time(sas[stProductTime]),
        'labLoadingCount': str(s['SendCT'][0] if s['customerCode'] == s['ccAmkorKorea'] else s['SendCT'][1]),
        'labMTBF': sec_to_spc(cdiv(sas[stPowerOn], 1000)),
    }
    jam, send1 = s['iJamCount1'], s['SendCT'][1]
    dsec = cdiv(wrap32(sas[stPauseTime] + sas[stProductTime] + sas[stJamTime]), 1000)
    if jam == 0:
        exp['labMTBA'] = '0 / %s' % sec_to_spc(dsec)
        exp['labMUBA'] = '0 / %d unit' % send1
    else:
        exp['labMTBA'] = ('1 / %s' % sec_to_spc(int(f32(float(dsec) / float(jam))))) if dsec != 0 else '%d / 0' % jam
        if s['userLanguage'] == s['eulSingapore']:
            exp['labMUBA'] = '%d / %d unit' % (jam, send1)
        else:
            exp['labMUBA'] = ('1 / %d unit' % cdiv(send1, jam)) if send1 != 0 else '%d / 0 unit' % jam
    if s['vtest']:
        dj, ds = s['iDayJamCount'], s['iDaySendCT']
        exp['pnlDayJamRate'] = '0 / 1 unit' if dj == 0 else (('1 / %d unit' % cdiv(ds, dj)) if ds != 0 else '%d / 0 unit' % dj)
    bad = ['%s %r != 公式 %r' % (k, cap.get(k), v) for k, v in exp.items() if cap.get(k) != v]
    check(not bad, 'V：%d 格＝golden 公式（ConvertMSecToTime／ConvertSecondToSPC／ProcessRunInfo）從 LastSet 重算%s'
          % (len(exp), '' if not bad else '：' + '; '.join(bad)))
    print('  手算  SystemAccSecond[0]=%s SendCT=%s iJamCount[1]=%d -> PowerOn %s／MTBA %s／MUBA %s'
          % (sas, s['SendCT'], jam, exp['labPowerOnTime'], exp['labMTBA'], exp['labMUBA']))
    # labVersion
    if 'labVersion' in d['noSource']:
        need_svn = not s['hiSilicon']
        check(s['asHandlerVersion'] == '' or (need_svn and s['SVNRevision'] == ''),
              'V：labVersion 判定沒有來源的條件成立（asHandlerVersion=%r SVNRevision=%r）' % (s['asHandlerVersion'], s['SVNRevision']))
    else:
        want = s['asHandlerVersion'] + ('' if s['hiSilicon'] else '.' + s['SVNRevision'])
        check(cap['labVersion'] == want, 'V：labVersion＝asHandlerVersion(+"."+SVNRevision)（%r）' % want)
    check(cap['labModel'] == s['sMachineType'] and cap['labMachineID'] == s['SocketHandlerID'],
          'V：labModel／labMachineID＝IniConfig.sMachineType／SocketHandlerID（%r／%r）' % (s['sMachineType'], s['SocketHandlerID']))
    # sources ↔ lastdata.dat
    data, which, path = lastdata_bytes()
    lay = d['lastdata']
    file_sas = [i32(data, lay['SystemAccSecond'] + 4 * k) for k in range(8)]
    file_send = [i32(data, lay['SendCT']), i32(data, lay['SendCT'] + 4)]
    file_jam1 = i32(data, lay['iJamCount'] + 4)
    print('  info  %s %d bytes（sizeof(LAST_GENERAL_SET)=%d）；檔內 SystemAccSecond[0]=%s SendCT[0..1]=%s iJamCount[1]=%d'
          % (which, len(data), lay['size'], file_sas, file_send, file_jam1))
    check(lay['longSize'] == 4 and lay['intSize'] == 4, 'V：這個 build 的 long／int 是 4 bytes（lastdata.dat 版面與 BCB6 相同）')
    # AI(W906-Q57-TRIAGE) 20260930: Q57 r4a FAIL（只差 SystemAccSecond[0][stPowerOn]：記憶體 1511175803、檔 1511127480，多 48 秒）＝
    #   這條寫在 0b38b6b5（S113）之前。現在 wb_serve 每拍照 golden TfMain::UpdateRecordScreen（V912 main.cpp:8584-8687，Timer1 :3284）
    #   把距上一拍的毫秒數加進 LastSet.SystemAccSecond[0..3][*]（PowerOn 一定加；Pause／Jam／Start…看狀態），lastdata.dat 只在 golden
    #   WriteLastDataFile 被呼叫時才寫 ⇒ 記憶體可以比檔案「多」，但最多多「檔案上次寫到現在」那麼久，而且不會比檔案少。
    #   SendCT／iJamCount 只在生產中變 ⇒ 照舊要一模一樣。32-bit 累加照 C 的 long 環繞算差值（wrap32）。
    age_ms = int((time.time() - os.path.getmtime(path)) * 1000) if path and os.path.exists(path) else -1
    grow = [wrap32(m - f) for m, f in zip(sas, file_sas)]
    slack = 5000   # 500 ms 一拍、檔案時間解析度、開頁回應到這裡的時間
    sas_ok = len(sas) == len(file_sas) and age_ms >= 0 and all(0 <= g <= age_ms + slack for g in grow)
    print('  info  記憶體 SystemAccSecond[0] 比 %s 多 %s ms（檔案 %d ms 前寫的；golden UpdateRecordScreen 每拍累加）' % (which, grow, age_ms))
    check(sas_ok and file_send == s['SendCT'] and file_jam1 == jam,
          'V：回應 sources 的 LastSet 欄位＝%s 在 C++ 回報位移讀出的值（SystemAccSecond 只許比檔案多、不超過檔案的年紀）' % which)


def verify_trays(cdp, d, label):
    c = d['category']
    g = dom_trays(cdp)
    bad = []
    for t in c['trays']:
        dt = g['trays'].get(t['name'])
        if not dt:
            bad.append(t['name'] + ' 不在畫面')
            continue
        if dt['x'] != t['x'] or dt['y'] != t['y'] or dt['cells'] != t['cells']:
            bad.append('%s（%dx%d）不同' % (t['name'], t['x'], t['y']))
    check(not bad, '%s：16 個 TTMyTray 畫面＝回應%s' % (label, '' if not bad else '：' + '; '.join(bad[:5])))
    rg = c['rgRowNo']
    check([x['text'] for x in g['rg']] == rg['items'] and [k for k, x in enumerate(g['rg']) if x['checked']] == [rg['itemIndex']],
          '%s：rgRowNo 選項／選取＝回應（%s，%d）' % (label, rg['items'], rg['itemIndex']))
    rb_ok = all(g['rb'].get(k) == {'checked': v['checked'], 'disabled': not v['enabled']} for k, v in c['radios'].items())
    check(rb_ok, '%s：Display Form 四顆 radio checked／enabled＝回應' % label)


def tray_map(d):
    return {t['name']: t for t in d['category']['trays']}


def verify_category_values(d, arms, label, mode):
    inp = d['category']['inputs']
    if inp['twoArm32Site'] or inp['nnMode'] != 0:
        print('  NOTE  twoArm32Site／NN 模式：本探針只驗一般配置（UpdataCount None_NN、WriteCategoryData 非 32-site）')
        return
    uc = update_count(arms, inp)
    row = d['category']['rgRowNo']['itemIndex']
    exp = expect_category(uc, inp, row, mode)
    tm = tray_map(d)
    bad, n = [], 0
    for name, cells in exp.items():
        t = tm[name]
        for (x, y), v in cells.items():
            if x >= t['x'] or y >= t['y']:
                continue                                       # golden SetCellNumber 寫在 XItem／YItem 外：畫不出來
            n += 1
            got = t['cells'][y][x]
            if got != v:
                bad.append('%s(%d,%d) %r!=%r' % (name, x, y, got, v))
    check(not bad, '%s：%d 格＝golden UpdataCount＋WriteCategoryData 從 Arm0.dat／Arm1.dat 算%s'
          % (label, n, '' if not bad else '：' + '; '.join(bad[:6])))
    print('  手算  %s：iTotalSocket=%d iPassSocket=%d iTotalCategory[0..3]=%s siteMap[%d]=%s'
          % (label, uc['tsock'], uc['tpass'], uc['tcat'][:4], row, inp['siteMap'][row]))
    fixed = {'mtNo': [['No.']], 'mtTotalName': [['Total']],
             'mtCategorySum': [['Head Total'], ['Socket Total'], ['Pass Head'], ['Pass Socket'], ['I/F Error']]}
    for name, cells in fixed.items():
        check(tm[name]['cells'] == cells, '%s：%s＝golden FormShow :521-528 的字（%s）' % (label, name, tm[name]['cells']))
    names = [r[0] for r in tm['myCategoryName']['cells']]
    check(names == ['  Category%02d' % i for i in range(inp['iTestBinCount'])],
          '%s：myCategoryName＝golden FormShow :537-541（%d 列＝iTestBinCount）' % (label, len(names)))


def verify_percent(d, arms, label):
    inp = d['category']['inputs']
    if inp['twoArm32Site'] or inp['nnMode'] != 0:
        return
    uc = update_count(arms, inp)
    row = d['category']['rgRowNo']['itemIndex']
    tm = tray_map(d)
    bad = []
    if not pct_close(tm['mtTotal']['cells'][1][0], pct(uc['tsock'], uc['tsock'])):
        bad.append('mtTotal(0,1)')
    for c in range(inp['maxIndexCol']):
        for a in (0, 1):
            x = c * 2 + a
            if x < tm['mtHeadTotal']['x'] and not pct_close(tm['mtHeadTotal']['cells'][0][x], pct(uc['head'][a][row][c], uc['tsock'])):
                bad.append('mtHeadTotal(%d)' % x)
            if x < tm['mtPassHead']['x'] and not pct_close(tm['mtPassHead']['cells'][0][x], pct(uc['passh'][a][row][c], uc['head'][a][row][c])):
                bad.append('mtPassHead(%d)' % x)
    check(not bad, '%s：百分比格＝golden ChangeToPercentage（±0.01，msvcrt 進位）%s' % (label, '' if not bad else '：' + ','.join(bad[:6])))


def verify_eventlog(cdp, d, label, header_before=None, filt=None):
    e = d['eventLog']
    g = dom_eventlog(cdp)
    check(g['grid'] == e['cells'], '%s：strngrdEventLog 畫面（%d×%d）＝回應' % (label, e['rows'], e['cols']))
    check([x['text'] for x in g['list']] == e['files'] and [k for k, x in enumerate(g['list']) if x['sel']] == ([e['fileIndex']] if e['fileIndex'] >= 0 else []),
          '%s：lstEventLog 畫面＝回應（%d 檔，選 %d）' % (label, len(e['files']), e['fileIndex']))
    check(g['month'] and g['month']['i'] == e['monthIndex'] and g['filter'] and g['filter']['i'] == e['filterIndex']
          and g['year'] and g['year']['t'] == e['yearText'],
          '%s：年／月／Filter 下拉＝回應（%s／%s／%s）' % (label, e['yearText'], e['monthText'], e['filterText']))
    want_files = csv_list(e['root'], e['yearText'], e['monthText'])
    check(sorted(e['files']) == sorted(want_files), '%s：lstEventLog＝%s\\%s\\%s 下的 *.CSV（%d 檔）' % (label, e['root'], e['yearText'], e['monthText'], len(want_files)))
    hb = header_before if header_before is not None else [''] * e['cols']
    f = filt if filt is not None else (e['filterText'] if e['filterIndex'] != 0 else 'All Data')
    want = expect_eventlog(e['file'], f, e['cols'], hb)
    diff = [r for r in range(max(len(want), len(e['cells']))) if (want[r] if r < len(want) else None) != (e['cells'][r] if r < len(e['cells']) else None)]
    check(e['rows'] == len(want) and not diff,
          '%s：回應格子＝golden V912 SplitEventLogCsvLine 解析 %s（Filter=%s，%d 列；不同列：%s）'
          % (label, os.path.basename(e['file']) or '（無檔）', f, len(want), diff[:5]))
    if diff:
        r = diff[0]
        print('  diff  列 %d：回應 %r' % (r, e['cells'][r] if r < len(e['cells']) else None))
        print('        期望 %r' % (want[r] if r < len(want) else None))
    return e


def token_free(port):
    """另一條連線拿得到權杖＝頁面沒有長期佔住（每次 acquire→get→release）。"""
    sock, left = ws_handshake('127.0.0.1', port, '/ht9045', time.monotonic() + 5)
    frames = read_frames(sock, left, time.monotonic() + 20)
    got = False
    for i in range(1, 11):
        send_text(sock, json.dumps({'type': 'cmd', 'id': i, 'cmd': 'control.acquire'}))
        for op, p in frames:
            if op != 1:
                continue
            try:
                m = json.loads(p.decode('utf-8'))
            except Exception:
                continue
            if m.get('type') == 'ack' and m.get('id') == i:
                got = bool(m.get('ok'))
                break
        if got:
            send_text(sock, json.dumps({'type': 'cmd', 'id': 99, 'cmd': 'control.release'}))
            time.sleep(0.2)
            break
        time.sleep(0.3)
    try:
        sock.close()
    except Exception:
        pass
    return got


SHOT_DIR = None


def shot(cdp, name):
    """--shot <dir>：把目前畫面存成 PNG（人工目視用，不影響判定）。"""
    if not SHOT_DIR:
        return
    import base64
    try:
        r = cdp.call('Page.captureScreenshot', {'format': 'png'}, 30)
        data = (r.get('result') or {}).get('data')
        if data:
            p = os.path.join(SHOT_DIR, 'observer_%s.png' % name)
            open(p, 'wb').write(base64.b64decode(data))
            print('  shot  %s' % p)
    except Exception as e:
        print('  shot  %s 失敗：%s' % (name, e))


def main():
    global SHOT_DIR
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9338)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--shot', help='截圖存放目錄（可省略）')
    a = ap.parse_args()
    SHOT_DIR = a.shot

    ev_root = r'D:\HT9045_Log\EventLogTxt'

    if a.user:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)
    # AI(W906-Q57-TRIAGE) 20260930: 快照移到登入之後。Q57 r4a W FAIL（EventLogTxt_20260929.csv 變了）＝登入本身照 golden 寫 event log：
    #   ws_login 先登出（golden V912 main.cpp:28104 btLoginClick → NewRecordProcess("MES2140", "Operator login")）再登入
    #   （密碼本模式 :15223／:15296 NewRecordProcess("MES2144", "USER login")）。06f8ef0a（St02-E cMyDB P4）之後 NewRecordProcess 是
    #   golden 本體（cMyDB.cpp:1545 → MyDBIProcessNew → slEventLog），不再是空替身。W 要守的是「這一頁唯讀」，登入不是這一頁做的。
    watch = watched_files(ev_root)
    sha0 = sha_files(watch)
    ev0 = {p: open(p, 'rb').read() for p in watch if p.startswith(ev_root) and os.path.isfile(p)}   # AI(W906-Q57-TRIAGE) 20260930: W 的 golden 定時記錄比對用（eventlog_periodic_only）

    src = urllib.request.urlopen('http://127.0.0.1:%d/page/%s' % (a.port, PAGE), timeout=10).read().decode('utf-8')
    live = re.sub(r'<!--.*?-->', '', src, flags=re.S)           # 頁面註解裡引述過這些假值（說明為什麼移除），不算
    fake = [s for s in ('0017 days', 'HT9046AT-136', 'JAM0217', 'WAR0701', 'name="tcArm"', "getElementById('tcGrid')",
                        'tr.dataset.cat', '<th>Category</th>') if s in live]
    check(not fake, 'F：頁面原始碼沒有假資料／假行為（找到：%s）' % fake)
    check('ht9045_observer_wire.js' in src and 'ht9045_recipe_client.js' in src, 'F：頁面載入 recipe_client＋observer_wire')

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Runtime.enable')
        if SHOT_DIR:
            cdp.call('Emulation.setDeviceMetricsOverride', {'width': 1000, 'height': 1000, 'deviceScaleFactor': 1, 'mobile': False})
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (a.port, PAGE)})
        ok = wait_js(cdp, "window.__observer && (window.__observer.data || window.__observer.error) ? 1 : 0", 60)
        check(bool(ok), 'O：開頁送出 observer.get open 並收到回應')
        s0 = st(cdp)
        d = full(cdp)
        if not d:
            check(False, 'O：observer.get 成功（%s）' % s0.get('error'))
            return len(FAILS)
        check(d['act'] == 'open' and d['bShow'] and d['full'], 'O：回應 act=open、bShow=true、full=true')
        print('  info  noSource=%s' % sorted(d['noSource']))
        for k in sorted(d['noSource']):
            print('        %s：%s' % (k, d['noSource'][k][:110]))
        time.sleep(1.5)                                          # 等 engine 的 sysText／tag 也寫完
        verify_caps(cdp, full(cdp), '開頁')
        shot(cdp, 'counter')
        verify_values(d)

        # E：engine 接的格子＝回應
        dom = dom_caps(cdp, ['labModel', 'labSerialNo', 'labMachineID', 'labDeviceName', 'labReleaseDate'])
        cap = d['captions']
        bad = [k for k in ('labModel', 'labSerialNo', 'labMachineID', 'labDeviceName') if dom[k]['text'] != cap[k]]
        check(not bad, 'E：engine 接的 labModel／labSerialNo／labMachineID／labDeviceName＝observer.get captions%s'
              % ('' if not bad else '：' + '; '.join('%s 畫面 %r 回應 %r' % (k, dom[k]['text'], cap[k]) for k in bad)))
        print('  info  labReleaseDate：畫面（tag build.stamp）%r；golden RunInfo.SoftwareDate（observer.get）%r'
              % (dom['labReleaseDate']['text'], cap['labReleaseDate']))

        # T：Timer1
        t0 = st(cdp)['ticks']
        time.sleep(4.2)
        t1 = st(cdp)
        check(t1['ticks'] - t0 >= 3, 'T：golden Timer1 每 1000 ms 一拍（4.2 秒內 ticks %d -> %d）' % (t0, t1['ticks']))
        last = t1['last'] or {}
        check(last.get('act') == 'timer' and last.get('full') is False, 'T：timer 回應只帶 captions（act=timer、full=false）')
        if last.get('captions'):
            # AI(W906-Q57-TRIAGE) 20260930: 同 verify_caps —— 畫面與 last 在同一次 eval 取（分兩次取時中間插進一拍，labPowerOnTime 會對不上）
            snapt = caps_snapshot(cdp, CAPS)
            dom, last = snapt.get('dom') or {}, snapt.get('last') or last
            badt = [k for k in CAPS if k not in (last.get('noSource') or {}) and dom[k]['text'] != (last.get('captions') or {}).get(k)]
            check(not badt, 'T：畫面＝最新 timer 回應（%s）' % (badt or '全部相同'))
        check(token_free(a.port), 'T：另一條連線拿得到 control 權杖（頁面每次 acquire→observer.get→release，不長期佔住）')

        # C：Tester Category
        arms = None
        d = act_and_wait(cdp, "document.querySelector('#pgcObserv > .pcTabs > .tab[data-t=\"1\"]').click(),1",
                         'C：點 Tester Category 頁籤', "window.__observer.data.category.loaded")
        if d:
            nb = d['category']['inputs']['iTestBinCount']
            arms = [read_arm('Arm0', nb), read_arm('Arm1', nb)]
            print('  info  inputs=%s' % {k: v for k, v in d['category']['inputs'].items() if k != 'siteMap'})
            time.sleep(0.3)
            verify_trays(cdp, d, 'C 開頁籤')
            shot(cdp, 'category')
            verify_category_values(d, arms, 'C Row-A／Head Real Number', 'rbHeadNumber')
            rg = d['category']['rgRowNo']
            if len(rg['items']) > 1:
                d2 = act_and_wait(cdp, "document.querySelectorAll('#rgRowNo input')[1].click(),1", 'C：點 rgRowNo 第 2 項',
                                  "window.__observer.data.category.rgRowNo.itemIndex==1")
                if d2:
                    time.sleep(0.3)
                    verify_trays(cdp, d2, 'C Row-B')
                    verify_category_values(d2, arms, 'C Row-B／Head Real Number', 'rbHeadNumber')
            if d['category']['radios']['rbSocketNumber']['enabled']:
                d3 = act_and_wait(cdp, "document.getElementById('rbSocketNumber').click(),1", 'C：點 Socket Real Number',
                                  "window.__observer.data.category.radios.rbSocketNumber.checked")
                if d3:
                    time.sleep(0.3)
                    verify_trays(cdp, d3, 'C Socket Real Number')
                    check(tray_map(d3)['mtCategoryNo']['x'] == 8, 'C：Socket 模式 mtCategoryNo->XItem=8（golden :3350-3353）')
                    verify_category_values(d3, arms, 'C Socket Real Number', 'rbSocketNumber')
            d4 = act_and_wait(cdp, "document.getElementById('rbHeadPercent').click(),1", 'C：點 Head %',
                              "window.__observer.data.category.radios.rbHeadPercent.checked")
            if d4:
                time.sleep(0.3)
                verify_trays(cdp, d4, 'C Head %')
                verify_percent(d4, arms, 'C Head %')
            act_and_wait(cdp, "document.getElementById('rbHeadNumber').click(),1", 'C：點回 Head Real Number',
                         "window.__observer.data.category.radios.rbHeadNumber.checked")

        # S：System Message
        d = act_and_wait(cdp, "document.querySelector('#pgcObserv > .pcTabs > .tab[data-t=\"3\"]').click(),1",
                         'S：點 System Message 頁籤', "window.__observer.data.eventLog.loaded")
        if d:
            time.sleep(0.3)
            e = verify_eventlog(cdp, d, 'S 開頁籤')
            shot(cdp, 'eventlog')
            header = e['cells'][0][:] if e['rows'] > 0 else [''] * e['cols']
            if e['files']:
                check(e['fileIndex'] == len(e['files']) - 1, 'S：預設選最後一個檔（golden cbbMonthChange :3790）')
            fi = e['filters'].index('JAM only') if 'JAM only' in e['filters'] else -1
            if fi >= 0:
                d5 = act_and_wait(cdp, "(function(){var s=document.getElementById('cbbFilter');s.selectedIndex=%d;"
                                       "s.dispatchEvent(new Event('change'));return 1;})()" % fi,
                                  'S：cbbFilter 選 JAM only', "window.__observer.data.eventLog.filterIndex==%d" % fi)
                if d5:
                    check(d5['eventLog']['cells'] == e['cells'], 'S：只改 Filter 不重讀（golden cbbFilter 沒有 OnChange）')
                    d6 = act_and_wait(cdp, "document.getElementById('btnQueryEventLogTxt').click(),1", 'S：按 Query')
                    if d6:
                        time.sleep(0.3)
                        verify_eventlog(cdp, d6, 'S JAM only', header, 'JAM only')
            if e['files']:
                d7 = act_and_wait(cdp, "document.querySelectorAll('#lstEventLog .obsLi')[0].click(),1", 'S：點 lstEventLog 第 1 個檔',
                                  "window.__observer.data.eventLog.fileIndex==0")
                if d7:
                    time.sleep(0.3)
                    check(d7['eventLog']['filterIndex'] == 0, 'S：點檔案把 Filter 回 All Data（golden lstEventLogClick :3760）')
                    verify_eventlog(cdp, d7, 'S 第 1 個檔', header, 'All Data')
            mi = e['monthIndex']
            other = [i for i in range(len(e['months'])) if i != mi and
                     os.path.isdir(os.path.join(ev_root, e['yearText'], e['months'][i]))]
            if other:
                m2 = other[-1]
                d8 = act_and_wait(cdp, "(function(){var s=document.getElementById('cbbMonth');s.selectedIndex=%d;"
                                       "s.dispatchEvent(new Event('change'));return 1;})()" % m2,
                                  'S：cbbMonth 換成 %s' % e['months'][m2], "window.__observer.data.eventLog.monthIndex==%d" % m2)
                if d8:
                    time.sleep(0.3)
                    verify_eventlog(cdp, d8, 'S 月份 %s' % e['months'][m2], d8['eventLog']['cells'][0] if d8['eventLog']['rows'] else None, 'All Data')
                act_and_wait(cdp, "(function(){var s=document.getElementById('cbbMonth');s.selectedIndex=%d;"
                                  "s.dispatchEvent(new Event('change'));return 1;})()" % mi, 'S：cbbMonth 換回 %s' % e['months'][mi])
            else:
                print('  NOTE  %s 年只有一個月份資料夾，跳過換月份' % e['yearText'])
        check(not st(cdp).get('error'), 'O：整段沒有 observer.get 錯誤（%s）' % st(cdp).get('error'))
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)

    sha1 = sha_files(watch)
    changed = [p for p in sha0 if sha0[p] != sha1.get(p)]
    # AI(W906-Q57-TRIAGE) 20260930: EventLogTxt 只多了 golden 定時記錄（整點 TimeData 等，不是這一頁寫的）的不算這一頁寫檔，列出來給人看
    periodic = []
    for p in list(changed):
        if p in ev0:
            ok_p, extra = eventlog_periodic_only(ev0[p], open(p, 'rb').read() if os.path.isfile(p) else None)
            if ok_p:
                changed.remove(p)
                periodic.append('%s +%d 列' % (os.path.basename(p), len(extra)))
    if periodic:
        print('  info  EventLogTxt 只多了 golden 定時記錄（%s：cMyDB.cpp MyDBITimeData／MyDBITotalLoader／MyDBIUPH）：%s'
              % ('／'.join(u.decode() for u in PERIODIC_UNITS), '; '.join(periodic)))
    check(not changed, 'W：探針期間 %d 個檔（lastdata*.dat／Arm*.dat／Gerneral.ini／Security_new.def／EventLogTxt）SHA256 不變%s'
          % (len(sha0), '' if not changed else '：' + '; '.join(changed[:5])))
    print('\n%s：%d 項失敗' % ('PASS' if not FAILS else 'FAIL', len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
