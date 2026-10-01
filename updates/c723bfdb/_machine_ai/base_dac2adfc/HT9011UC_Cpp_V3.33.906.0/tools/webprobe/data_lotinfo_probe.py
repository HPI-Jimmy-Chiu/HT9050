# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_lotinfo_probe.py -- Data.LotInfo.html（golden V912 uLotInfo）的 e2e probe，
#  外加三個 Data 頁面的「假數字止血」掃描。
#
#  Steven 團隊 20260925（上午：Lot 分頁；下午：其餘分頁 —— 分頁可見度、Lot Info 子分頁、ATC、BarCode、
#  Tester Log、Selection、WS lotinfo.op）。
#
#  用真的瀏覽器（headless Edge ＋ DevTools 協定）開頁面，驗：
#    T  頁面註冊的 tag＝預期清單；C++ 快照裡每個 lot.* tag 都在（WebBridgeTags.cpp 檔尾 W906_StageLotInfoTags／…TabTags）
#    B  開機狀態：第一次 lot.start 之前 lot.* 是 null（golden 開機的 FormShow→SetLotStart(true) 在移植樹是 GATE WC-19）
#    D  畫面＝tag：Lot 分頁 6 格
#    V  分頁可見度：37 個 lot.tab.* 都是 bool；27 個頁籤 display 與 tag 一致；幾個只看 Gerneral.ini 的分頁
#       與 golden 條件逐一對照（REAL_TIME_CCD、BAR_CODE_INSTALL、ESD 恆藏、ATC、Auto Clean Monitor＝CC_SCK、WinWay、ATC 6.1、Bundle）；
#       lot.tab.active 是 27 個之一，而且開頁後選中的就是它
#    A  ATC：palATC／pl_ATC_Online 畫面＝tag；CH 可見度＝golden ShowATCTempPanel（i < iATC_Use_Heat_Count）；CH 溫度格全是 "---"
#    BC BarCode：30 格畫面＝tag；計數是整數、Total＝前四欄加總、Rate 是 %2.2f
#    TL Tester Log：狀態字＝tag、行數是整數
#    S  lot.start（--no-lotstart 可跳過）：lot.* 與 Lot Info 子分頁 11 格＝config.ini [Lot Info]；Start Time 格式；LotStartTime 是今天
#    OP WS lotinfo.op（分派沒接時整段記 FAIL「分派未接」）：
#       testerLog.get 的行數＝tag；selection.get（分頁 golden 看不到時要回 guard tab-hidden）；
#       barcode.clearCount {"confirmed":false} 要回 needConfirm＋archive.written=false；
#       --clear-barcode 才真的清（confirmed:true → executed，計數全 0、tag 跟著變；新建的空目錄
#       D:\HT9045_Log\2DBarCode\YYYY_MM_DD\ 由探針刪掉）；
#       --selection-save 才真的存（分頁看得到時：勾選值原樣存一次，Security_new.def [Network] 的值＝勾選）
#    F  假數字：Data.LotInfo.html（ATC CH 溫度格、Auto Clean Monitor）、Data.TestCategory.html、Data.SortCT.html 的 ART 分頁
#       —— 接了 tag 的元素不掃（實況值剛好等於舊展示值時不能誤報）
#
#  ⚠ S 段會寫 D:\HT9045\config\config.ini 的 [Lot Info]（與 system\ArmByLot*.dat）；selection.get 在缺鍵時會補寫
#    config\Security_new.def [Network]；--selection-save 會寫同一段。三者都在 scratchpad restore.sh 的還原範圍。
#    請用 scratchpad/run_lotinfo.sh 跑（鎖＋restore＋SHA256），不要單獨對真檔跑。
#
#  用法：
#      set W906_PWBOOK_PATH=<scratch book>  &  build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\data_lotinfo_probe.py --port 8046 --user S12TEST --password S12PW
#             [--no-lotstart] [--clear-barcode] [--selection-save]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import re
import shutil
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
CONFIG = r'D:\HT9045\config\config.ini'
SECURITY = r'D:\HT9045\config\Security_new.def'
GENERAL = r'D:\HT9045\system\Gerneral.ini'
LOT = 'LOTPROBE0925'
OPR = 'OPPROBE'

# 頁面格 id ↔ tag（與 web/page/ht9045_lotinfo_wire.js 一致）
BIND = [('lotNo', 'lot.id'), ('deviceName', 'lot.device'), ('runMode', 'lot.runMode'),
        ('operator', 'lot.operator'), ('startTime', 'lot.startTime'), ('loadingCount', 'sort.loading')]
LOT_TAGS = ['lot.id', 'lot.operator', 'lot.runMode', 'lot.startTime', 'lot.device']
# tag ↔ config.ini [Lot Info] 鍵（golden ReadWriteLotInfo uLotInfo.cpp:1446-1532 / SetLotID :1534-1612）
TAG_KEY = [('lot.id', 'Lot No'), ('lot.id', 'Lot ID'), ('lot.operator', 'Operator'),
           ('lot.startTime', 'Start Time'), ('lot.runMode', 'Run Mode'), ('lot.device', 'Device Name')]
# Lot Info 子分頁（tsChipAdv）另外 12 格：(tag, 頁面 id, config.ini [Lot Info] 鍵；Machine ID 沒有鍵)
CHIPADV = [('lot.endTime', 'lbledtEndTime', 'End Time'), ('lot.testerOsVer', 'lbledtTesterOsVer', 'Tester OS Ver'),
           ('lot.customer', 'lbledtCustomer', 'Customer'), ('lot.testProg', 'lbledtTestProg', 'Test Program'),
           ('lot.testerId', 'lbledtTesterID', 'Tester ID'), ('lot.subLotNo', 'lbledtSubLotNo', 'Sub Lot No'),
           ('lot.testCode', 'lbledtTestCode', 'Test Code'), ('lot.machineId', 'lbledtMachineID', None),
           ('lot.testBinNo', 'lbledtTestBinNo', 'Test Bin No'), ('lot.modeCode', 'lbledtModeCode', 'Mode Code'),
           ('lot.stage', 'edtStage', 'Stage'), ('lot.step', 'edtStep', 'Step')]
TABS = ['tsDeviceInfo', 'tsLotID', 'tsFTP', 'tsRTCFullViewImg', 'tsATC', 'ts_OCRInterface', 'ts_SocketInterface',
        'tsSelection', 'tsBarCode', 'ts_AutoCleanMonitor', 'ts_AutoRetestMonitor', 'tsOCRBarCode', 'tsESDMonitor',
        'tsASEMARMS', 'tsASECLEventLog', 'tsChamberBoost', 'ATC_WinWay', 'tsRFMD', 'ts_FTPAutomation', 'tsYieldMonitior',
        'tsTesterLog', 'ts_ATC6_1', 'tsBundle', 'tsSetupFileCheck', 'tsAMR', 'tsKYEC_AMR', 'tsOtherTool']
SUBTABS = ['tsMurata', 'tsSigurd_CX', 'tsSPIL_SZ', 'tsOEE', 'ts2DSort', 'tsChipAdv', 'tsVTest', 'tsPATSetUp', 'tsSigurd', 'tsTPW']
ATC_FIXED = ['lot.atc.caption', 'lot.atc.workTemp', 'lot.atc.online', 'lot.atc.online.color', 'lot.atc.power',
             'lot.atc.chiller.visible', 'lot.atc.chillerLabel.visible', 'lot.atc.atc70.visible', 'lot.atc.atc70Label.visible',
             'lot.atc.chillerSV.visible', 'lot.atc.chillerSVValue.visible', 'lot.atc.recipeFile.visible', 'lot.atc.dewPoint.visible',
             'lot.atc.use4.visible', 'lot.atc.use8.visible', 'lot.atc.use32.visible']
BC_FIXED = ['lot.barcode.checkByLot', 'lot.barcode.checkByLot.color', 'lot.barcode.changeFile.visible', 'lot.barcode.display.visible']
TL_TAGS = ['lot.testerLog.status', 'lot.testerLog.status.color', 'lot.testerLog.count', 'lot.testerLog.tail']


def server_tags():
    """C++ 送出的 lot.* tag（檔尾那兩個函式），不含上午的 5 個。"""
    t = ['lot.tab.' + n for n in TABS + SUBTABS] + ['lot.tab.active', 'lot.tab.tsChipAdv.caption']
    t += [c[0] for c in CHIPADV] + ATC_FIXED
    for i in range(1, 33):
        t += ['lot.atc.ch%d.visible' % i, 'lot.atc.ch%d.ref.visible' % i]
    t += ['lot.barcode.r%d.c%d' % (r, c) for r in range(1, 7) for c in range(1, 6)] + BC_FIXED + TL_TAGS
    return t


def page_tags():
    """頁面接線檔註冊的 tag（子分頁 tsMurata… 只收不綁，不在這裡）。"""
    t = [tg for _, tg in BIND] + ['lot.tab.active', 'lot.tab.tsChipAdv', 'lot.tab.tsChipAdv.caption']
    t += ['lot.tab.' + n for n in TABS] + [c[0] for c in CHIPADV] + ATC_FIXED
    for i in range(1, 33):
        t += ['lot.atc.ch%d.visible' % i, 'lot.atc.ch%d.ref.visible' % i]
    t += ['lot.barcode.r%d.c%d' % (r, c) for r in range(1, 7) for c in range(1, 6)] + BC_FIXED + TL_TAGS
    return t


# 三個頁面改前寫死的展示值（回歸守衛：再出現就是有人把假數字加回來）。
# Steven 20260925 下午：拿掉 'ATC On Line' —— 那是 golden pl_ATC_Online 的真字樣（dfm :2372／FormShow :670-675），現在由 tag 送。
FAKE = {
    'Data.LotInfo.html': [r'85\.0', r'85\.7', r'85\.6', r'Soak Time 120', r'Q7HK21A03', r'85 / 200', r'（200）', r'Clean Pad\s*OK'],
    'Data.TestCategory.html': [],          # 值格另外逐格驗
    'Data.SortCT.html': [r'\b312\b', r'\b298\b', r'92\.28%', r'6\.71%', r'1\.01%', r'\b275\b'],
}


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def read_ini(path, section):
    """ini 的一段；同一鍵出現多次取第一個（與 GetPrivateProfileString／TIniFile 一致）。"""
    try:
        txt = open(path, 'rb').read().decode('cp950', errors='replace')
    except OSError:
        return {}
    sec, vals = None, {}
    for line in txt.splitlines():
        s = line.strip()
        if s.startswith('[') and s.endswith(']'):
            sec = s[1:-1]
            continue
        if sec == section and '=' in s:
            k, v = s.split('=', 1)
            vals.setdefault(k.strip(), v.strip())
    return vals


def read_lot_info():
    return read_ini(CONFIG, 'Lot Info')


def general(key, section=None, default=None):
    """Gerneral.ini 的鍵（不分段找第一個；section 給了就只找那一段）。"""
    try:
        txt = open(GENERAL, 'rb').read().decode('cp950', errors='replace')
    except OSError:
        return default
    sec = None
    for line in txt.splitlines():
        s = line.strip()
        if s.startswith('[') and s.endswith(']'):
            sec = s[1:-1]
            continue
        if '=' in s and (section is None or sec == section):
            k, v = s.split('=', 1)
            if k.strip() == key:
                return v.strip()
    return default


def gint(key, section=None, default=0):
    try:
        return int(general(key, section, str(default)))
    except (TypeError, ValueError):
        return default


def tags(cdp, names):
    js = "JSON.stringify(%s.map(function(t){return [t, HT9045Tags.has(t), HT9045Tags.get(t)];}))" % json.dumps(names)
    return {t: (h, v) for t, h, v in json.loads(cdp.eval(js))}


def texts(cdp, ids):
    js = "JSON.stringify(%s.map(function(id){var e=document.getElementById(id); return [id, e?e.textContent:null];}))" % json.dumps(ids)
    return dict(json.loads(cdp.eval(js)))


def fmt(v):
    """引擎 tagFmt：null→'---'；數字依小數位（sort.loading 是 0 位）；其他 String(v)。"""
    if v is None:
        return '---'
    if isinstance(v, bool):
        return 'true' if v else 'false'
    if isinstance(v, (int, float)):
        return str(int(round(v)))
    return str(v)


def verify_dom(cdp, label):
    t = tags(cdp, [tg for _, tg in BIND])
    d = texts(cdp, [b for b, _ in BIND])
    bad = ['%s=%r（tag %s=%r）' % (el, d.get(el), tg, t[tg][1]) for el, tg in BIND if d.get(el) != fmt(t[tg][1])]
    check(not bad, 'D  %s：Lot 分頁 6 格＝tag（不符：%s）' % (label, bad))
    return t


def open_page(cdp, port, page, ready):
    cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (port, page)})
    time.sleep(0.5)
    return wait_js(cdp, ready, 60)


def page_text_js(sel):
    # sel 範圍內所有文字節點（含隱藏分頁；排除 script/style 與接了 tag 的元素），合成一個字串。
    return ("(function(){var skip={};try{var m=HT9045Page.cfg.tags;Object.keys(m).forEach(function(k){skip[m[k][0]]=1;});}catch(e){}"
            "function bound(n){for(var p=n.parentNode;p&&p.nodeType===1;p=p.parentNode){if(p.id&&skip[p.id])return true;}return false;}"
            "var r=[];Array.prototype.forEach.call(document.querySelectorAll(%s),function(root){"
            "var w=document.createTreeWalker(root, NodeFilter.SHOW_TEXT, null), n;"
            "while((n=w.nextNode())){var p=n.parentNode&&n.parentNode.nodeName; if(p==='SCRIPT'||p==='STYLE')continue;"
            "if(bound(n))continue; r.push(n.nodeValue);}}); return r.join(' ');})()") % json.dumps(sel)


def scan_fake(cdp, page, sel):
    txt = cdp.eval(page_text_js(sel)) or ''
    hits = [p for p in FAKE[page] if re.search(p, txt)]
    check(bool(txt.strip()) and not hits, 'F  %s（%s）文字掃不到舊展示值（命中：%s）' % (page, sel, hits))


def lotinfo_op(cdp, payload, timeout=30):
    js = ("HT9045Recipe.rawCmd('control.acquire').catch(function(){}).then(function(){"
          "return HT9045Recipe.rawCmd('lotinfo.op', {value: %s});}).then(function(m){"
          "if(m&&typeof m.value==='string'){try{return JSON.stringify(JSON.parse(m.value));}catch(e){}}return JSON.stringify(m);},"
          # 守衛擋下時伺服器回 ok=false、訊息就是 C++ 的 JSON（同 WebSortCT／ht9045_sortct_wire.js parseErr）
          "function(e){var t=String(e&&e.message||e);try{var j=JSON.parse(t);if(j&&typeof j==='object')return JSON.stringify(j);}catch(x){}"
          "return JSON.stringify({executed:false,guard:'transport',detail:t});})") % json.dumps(json.dumps(payload))
    try:
        return json.loads(cdp.eval(js, timeout=timeout))
    except Exception as e:                      # noqa: BLE001
        return {'executed': False, 'guard': 'probe', 'detail': str(e)}


def not_wired(r):
    return bool(r) and (r.get('guard') == 'unknown-action' or re.search(r'unknown cmd|unknown command', str(r.get('detail', '')), re.I))


def section_tabs(cdp):
    names = ['lot.tab.' + n for n in TABS + SUBTABS] + ['lot.tab.active', 'lot.tab.tsChipAdv.caption']
    t = tags(cdp, names)
    notbool = [n for n in names[:-2] if not isinstance(t[n][1], bool)]
    check(not notbool, 'V  37 個 lot.tab.* 都是 bool（不是：%s）' % notbool)
    vis = {n: t['lot.tab.' + n][1] for n in TABS + SUBTABS}
    print('  INFO  golden 可見的分頁：%s' % ', '.join(n for n in TABS if vis.get(n)))
    print('  INFO  lot.tab.active=%r  tsChipAdv.caption=%r' % (t['lot.tab.active'][1], t['lot.tab.tsChipAdv.caption'][1]))
    disp = json.loads(cdp.eval("JSON.stringify(%s.map(function(n){var e=document.getElementById('tab_'+n);"
                               "return [n, e?e.style.display:null, e?e.classList.contains('act'):null];}))" % json.dumps(TABS)))
    bad = [n for n, d, _ in disp if (d == 'none') != (vis.get(n) is False)]
    check(not bad, 'V  27 個頁籤的 display 與 lot.tab.* 一致（false 才藏；不符：%s）' % bad)
    act = [n for n, _, a in disp if a]
    tact = t['lot.tab.active'][1]
    check(tact in TABS, 'V  lot.tab.active 是 27 個分頁之一：%r' % tact)
    check(act == [tact] or (vis.get(tact) is False), 'V  開頁後選中的分頁＝lot.tab.active（選中：%s）' % act)

    # 只看 Gerneral.ini 就能算的 golden 條件（V912 uLotInfo.cpp 行號見 forms/fLotInfo.cpp 檔尾 W906_RefreshTabVisible）
    rtc = gint('REAL_TIME_CCD') != 0
    bc = gint('BAR_CODE_INSTALL') in (2, 3, 4)
    atcsys = gint('USE_ATC_MODE', 'ATC')
    tri = gint('Tri_Temp_Machine') == 1
    atc = (not tri) and atcsys in (4, 6)                  # SetATCFormVisible :10119-10166（eATCHonPrecType=4、eNewATCSystem=6）
    cc = gint('CUSTOMER_CODE')
    exp = {'tsRTCFullViewImg': rtc,                        # FormShow :549
           'tsBarCode': bc,                                # :571／:590
           'tsESDMonitor': False,                          # :574
           'tsATC': atc,
           'ts_AutoCleanMonitor': cc == 947,               # :463-470 CC_SCK=947
           'ATC_WinWay': atcsys == 7,                      # Timer2Timer :7152-7155
           'tsBundle': gint('USE_COVER_TRAYID') != 0}      # :333
    if tri:
        exp['ts_ATC6_1'] = True                            # :331 第一項
    bad = ['%s tag=%r golden=%r' % (k, vis.get(k), v) for k, v in exp.items() if vis.get(k) != v]
    check(not bad, 'V  只看 Gerneral.ini 的分頁可見度＝golden 條件（CUSTOMER_CODE=%d REAL_TIME_CCD=%s BAR_CODE_INSTALL=%d '
                   'USE_ATC_MODE=%d；不符：%s）' % (cc, rtc, gint('BAR_CODE_INSTALL'), atcsys, bad))
    return vis


def section_atc(cdp, vis):
    t = tags(cdp, ATC_FIXED + ['lot.atc.ch%d.visible' % i for i in range(1, 33)] + ['lot.atc.ch%d.ref.visible' % i for i in range(1, 33)])
    missing = [n for n in t if not t[n][0]]
    check(not missing, 'A  ATC 的 %d 個 tag 都在線上快照（缺：%s）' % (len(t), missing))
    print('  INFO  ATC：caption=%r workTemp=%r online=%r power=%r' % (t['lot.atc.caption'][1], t['lot.atc.workTemp'][1],
                                                                     t['lot.atc.online'][1], t['lot.atc.power'][1]))
    d = texts(cdp, ['palATC', 'palATCWorkingTemp', 'pl_ATC_Online'])
    bad = [(el, d[el], t[tg][1]) for el, tg in (('palATC', 'lot.atc.caption'), ('palATCWorkingTemp', 'lot.atc.workTemp'),
                                                ('pl_ATC_Online', 'lot.atc.online')) if d[el] != fmt(t[tg][1])]
    check(not bad, 'A  palATC／palATCWorkingTemp／pl_ATC_Online 畫面＝tag（不符：%s）' % bad)
    check(t['lot.atc.online'][1] in ('ATC On Line', 'ATC Off Line'), 'A  pl_ATC_Online 是 golden 的兩種字樣之一：%r' % t['lot.atc.online'][1])
    atcsys = gint('USE_ATC_MODE', 'ATC')
    heat = 4 if atcsys == 4 else gint('ATC_SYSTEM_USEHEAT', 'ATC', 4)      # database.cpp:811-814
    chv = [t['lot.atc.ch%d.visible' % i][1] for i in range(1, 33)]
    check(chv == [i < heat for i in range(32)], 'A  CH1..CH32 可見度＝ShowATCTempPanel（i<%d）：%s' % (heat, ''.join('1' if v else '0' for v in chv)))
    rows = json.loads(cdp.eval("JSON.stringify((function(){var r=[];for(var i=1;i<=32;i++){var id='row_ATC'+(i<10?'0':'')+i;"
                               "var e=document.getElementById(id);r.push(e?e.style.display!=='none':null);}return r;})())"))
    check(rows == chv, 'A  CH 列的 display＝tag')
    vals = json.loads(cdp.eval("JSON.stringify(Array.prototype.map.call(document.querySelectorAll('[id^=pl_ATCTempHead],[id^=pl_ATCRefHead],"
                               "#pl_ATCChillerSV,#pl_DewPoint,#lblATC_Now_RecipeFile'),function(e){return e.textContent.trim();}))"))
    check(len(vals) == 67 and all(v == '---' for v in vals), 'F  ATC 沒有來源的 67 格（CH 溫度 32＋參考 32＋Chiller SV／露點／配方檔名）全是 "---"（%d 格）' % len(vals))


def section_barcode(cdp):
    names = ['lot.barcode.r%d.c%d' % (r, c) for r in range(1, 7) for c in range(1, 6)]
    t = tags(cdp, names + BC_FIXED)
    missing = [n for n in t if not t[n][0]]
    check(not missing, 'BC BarCode 的 %d 個 tag 都在（缺：%s）' % (len(t), missing))
    g = {(r, c): t['lot.barcode.r%d.c%d' % (r, c)][1] for r in range(1, 7) for c in range(1, 6)}
    print('  INFO  sgBarcode：%s' % ' | '.join(','.join(str(g[(r, c)]) for c in range(1, 6)) for r in range(1, 7)))
    d = texts(cdp, ['sgBarcode_r%d_c%d' % (r, c) for r in range(1, 7) for c in range(1, 6)])
    bad = [(r, c) for r in range(1, 7) for c in range(1, 6) if d['sgBarcode_r%d_c%d' % (r, c)] != fmt(g[(r, c)])]
    check(not bad, 'BC 30 格畫面＝tag（不符：%s）' % bad)
    try:
        ints = {(r, c): int(g[(r, c)]) for r in (1, 2, 3, 5, 6) for c in range(1, 6)}
        sums = [r for r in (1, 2, 3, 5, 6) if ints[(r, 5)] != sum(ints[(r, c)] for c in range(1, 5))]
        check(not sums, 'BC Load/Pass/Fail/Retry/Duplicate 是整數且 Total＝1_A..2_B 加總（golden BarCode.cpp:5909-5914；不符列：%s）' % sums)
        rates = [g[(4, c)] for c in range(1, 6)]
        check(all(re.match(r'^\d+\.\d{2}$', str(x)) for x in rates), 'BC Rate(%%) 是 %%2.2f：%s' % rates)
        load, pas = ints[(1, 5)], ints[(2, 5)]
        exp = '%2.2f' % (pas * 100.0 / load if load else 0.0)
        check(g[(4, 5)] == exp, 'BC Total Rate＝Pass*100/Load（golden BarCode.cpp:5899-5912）：%r vs %r' % (g[(4, 5)], exp))
    except (TypeError, ValueError) as e:
        check(False, 'BC 計數格不是整數：%s' % e)
    return g


def section_testerlog(cdp):
    t = tags(cdp, TL_TAGS)
    missing = [n for n in t if not t[n][0]]
    check(not missing, 'TL Tester Log 的 4 個 tag 都在（缺：%s）' % missing)
    st = t['lot.testerLog.status'][1]
    check(st in ('OFF-LINE', 'ON-LINE', 'ERROR'), 'TL labTCPIPStatus 是 golden 的三種字樣之一（dfm 初值 OFF-LINE）：%r' % st)
    d = texts(cdp, ['labTCPIPStatus', 'mmTesterLogCount'])
    check(d['labTCPIPStatus'] == fmt(st), 'TL 狀態畫面＝tag：%r' % d['labTCPIPStatus'])
    check(isinstance(t['lot.testerLog.count'][1], int) and t['lot.testerLog.count'][1] >= 0,
          'TL 行數是整數：%r（dfm Lines.Strings=(\'\') 起算 1）' % t['lot.testerLog.count'][1])
    return t


def section_ops(cdp, a, vis, bc0):
    r = lotinfo_op(cdp, {'op': 'testerLog.get'})
    if not_wired(r):
        check(False, 'OP lotinfo.op 分派未接（tools/wb_serve.cpp；WebLotInfo.h 有範例）：%s' % json.dumps(r, ensure_ascii=False)[:200])
        return
    tc = tags(cdp, ['lot.testerLog.count'])['lot.testerLog.count'][1]
    check(r.get('executed') is True and r.get('count') == tc, 'OP testerLog.get 行數＝tag（%r vs %r）' % (r.get('count'), tc))

    r = lotinfo_op(cdp, {'op': 'selection.get'})
    if vis.get('tsSelection') is False:
        check(r.get('executed') is False and r.get('guard') == 'tab-hidden',
              'OP selection.get：golden 看不到 Selection 分頁 → guard tab-hidden（%s）' % json.dumps(r, ensure_ascii=False)[:160])
    else:
        boxes = r.get('boxes') or {}
        check(r.get('executed') is True and len(boxes) == 20, 'OP selection.get 回 20 個勾選框（%d）' % len(boxes))
        sec = read_ini(SECURITY, 'Network')
        bad = [n for n, b in boxes.items() if b.get('readKey') and b.get('visible') and
               sec.get(b['readKey'].rstrip(), sec.get(b['readKey'])) not in (None, '1' if b['checked'] else '0')]
        check(not bad, 'OP 勾選值＝Security_new.def [Network]（不符：%s）' % bad)
        if a.selection_save:
            vals = {n: b['checked'] for n, b in boxes.items()}
            r2 = lotinfo_op(cdp, {'op': 'selection.save', 'values': vals})
            check(r2.get('executed') is True, 'OP selection.save 原值存一次：%s' % json.dumps(r2, ensure_ascii=False)[:160])
            sec2 = read_ini(SECURITY, 'Network')
            bad = [n for n, b in boxes.items() if b.get('saveKey') and b.get('visible') and b.get('enabled') and
                   n not in ('chkART', 'chkStopYield', 'chkConsecutiveFailure') and
                   sec2.get(b['saveKey']) != ('1' if b['checked'] else '0')]
            check(not bad, 'OP 存檔後 Security_new.def [Network]＝勾選（golden btnSaveClick 的鍵；不符：%s）' % bad)
        else:
            print('  SKIP  selection.save：沒有 --selection-save')

    r = lotinfo_op(cdp, {'op': 'barcode.clearCount', 'confirmed': False})
    arc = r.get('archive') or {}
    check(r.get('needConfirm') is True and r.get('executed') is False and arc.get('written') is False,
          'OP barcode.clearCount 第一段：needConfirm、沒執行、archive.written=false（SGDToXLS 未移植）：%s' % json.dumps(r, ensure_ascii=False)[:200])
    if not a.clear_barcode:
        print('  SKIP  barcode.clearCount 第二段：沒有 --clear-barcode')
        return
    folder = str(arc.get('path', '')).rsplit('\\', 1)[0] + '\\' if arc.get('path') else ''
    existed = bool(folder) and os.path.isdir(folder)
    r = lotinfo_op(cdp, {'op': 'barcode.clearCount', 'confirmed': True})
    after = r.get('after') or {}
    zeros = all(str(v) in ('0', '0.00') for row in after.values() for v in row.values()) and len(after) == 6
    check(r.get('executed') is True and zeros, 'OP barcode.clearCount 第二段：executed、清完 30 格全是 0／0.00：%s' % json.dumps(r, ensure_ascii=False)[:200])
    ok = wait_js(cdp, "HT9045Tags.get('lot.barcode.r1.c5')==='0' && HT9045Tags.get('lot.barcode.r4.c5')==='0.00'", 15)
    check(bool(ok), 'OP 15 秒內 tag 跟著歸零（lot.barcode.r1.c5=%r）' % cdp.eval("HT9045Tags.get('lot.barcode.r1.c5')"))
    if folder and not existed and os.path.isdir(folder):
        try:
            os.rmdir(folder)                     # golden :10174 MyForceDirectories 建的；.xls 沒寫，目錄是空的
            print('  INFO  已刪除探針造成的空目錄 %s' % folder)
        except OSError as e:
            print('  WARN  %s 刪不掉（%s）—— 請手動確認' % (folder, e))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9337)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--no-lotstart', action='store_true', help='不送 lot.start（不寫 config.ini）')
    ap.add_argument('--clear-barcode', action='store_true', help='真的清 BarCode 計數（lotinfo.op confirmed:true）')
    ap.add_argument('--selection-save', action='store_true', help='Selection 分頁看得到時，原值存一次 Security_new.def')
    a = ap.parse_args()

    cfg0 = read_lot_info()
    print('config.ini [Lot Info]（開機前）：%s' % ', '.join('%s=%r' % (k, cfg0.get(k))
                                                          for k in ('Lot ID', 'Lot No', 'Operator', 'Start Time',
                                                                    'Run Mode', 'Device Name', 'LotStartTime')))
    if a.user and a.password:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')

        # ---- T / B / D：開機狀態 ------------------------------------------
        ok = open_page(cdp, a.port, 'Data.LotInfo.html',
                       "window.HT9045Tags && HT9045Tags.status().connected && HT9045Tags.has('lot.id') && "
                       "HT9045Tags.has('lot.tab.tsLotID') && window.HT9045Page && window.HT9045LotInfo")
        check(bool(ok), 'T  開頁、tag 串流已連上且收到 lot.id／lot.tab.tsLotID')
        if not ok:
            return len(FAILS)
        time.sleep(1.0)                          # 讓第一個快照的 tag 全部套到畫面
        cfgTags = sorted(json.loads(cdp.eval("JSON.stringify(Object.keys(HT9045Page.cfg.tags))")))
        exp = sorted(set(page_tags()))
        check(cfgTags == exp, 'T  頁面接線檔註冊的 tag＝預期 %d 個（多：%s；少：%s）' % (
            len(exp), sorted(set(cfgTags) - set(exp))[:8], sorted(set(exp) - set(cfgTags))[:8]))
        st = server_tags()
        t_all = tags(cdp, st)
        missing = [n for n in st if not t_all[n][0]]
        check(not missing, 'T  C++ 送的 %d 個 lot.* tag（其餘分頁）都在線上快照（缺：%s）' % (len(st), missing[:10]))
        t0 = tags(cdp, LOT_TAGS + ['sort.loading'])
        missing = [tg for tg in LOT_TAGS if not t0[tg][0]]
        check(not missing, 'T  五個 lot.* 都在線上快照（缺：%s）' % missing)
        print('  INFO  開機 tag：%s' % ', '.join('%s=%r' % (tg, t0[tg][1]) for tg in LOT_TAGS + ['sort.loading']))
        if all(t0[tg][1] is None for tg in LOT_TAGS):
            check(True, 'B  開機 lot.* 全是 null（這個行程還沒有 SetLotID；golden FormShow 的開機讀取是 GATE WC-19）')
            tc = tags(cdp, [c[0] for c in CHIPADV])
            check(all(tc[c[0]][1] is None for c in CHIPADV), 'B  Lot Info 子分頁 12 格開機也是 null（同一個 liveness）')
            print('  INFO  golden 開機會把 config.ini 讀進表單並顯示：Lot No=%r Operator=%r Start Time=%r '
                  '—— 已知落差，見 WebBridgeTags.cpp 檔尾' % (cfg0.get('Lot No'), cfg0.get('Operator'), cfg0.get('Start Time')))
        else:
            bad = ['%s≠[%s]%r' % (tg, k, cfg0.get(k)) for tg, k in TAG_KEY if t0[tg][1] != cfg0.get(k, '')]
            check(not bad, 'B  開機 lot.* 已有值，且＝config.ini [Lot Info]（不符：%s）' % bad)
        check(t0['sort.loading'][1] is None or isinstance(t0['sort.loading'][1], int),
              'B  sort.loading 是整數或 null：%r' % (t0['sort.loading'][1],))
        verify_dom(cdp, '開機')

        # ---- V / A / BC / TL：其餘分頁 --------------------------------------
        vis = section_tabs(cdp)
        section_atc(cdp, vis)
        bc0 = section_barcode(cdp)
        section_testerlog(cdp)

        # ---- S：lot.start 之後 tag 更新 ------------------------------------
        if a.no_lotstart:
            print('  SKIP  S：--no-lotstart')
        else:
            r = json.loads(cdp.eval(
                "HT9045Recipe.lotStart(%s, %s).then(function(m){return JSON.stringify({ok:true,res:m});},"
                "function(e){return JSON.stringify({ok:false,err:String(e&&e.message||e)});})" % (json.dumps(LOT), json.dumps(OPR)),
                timeout=60))
            res = r.get('res') or {}
            if isinstance(res.get('detail'), dict):
                for k, v in res['detail'].items():
                    res.setdefault(k, v)
            check(r.get('ok') and res.get('ok') is True and res.get('lotStart') is True,
                  'S  lot.start ack ok、RunInfo.bLotStart=true：%s' % json.dumps(r, ensure_ascii=False)[:240])
            upd = wait_js(cdp, "HT9045Tags.get('lot.id')===%s && HT9045Tags.get('lot.operator')===%s" % (
                json.dumps(LOT), json.dumps(OPR)), 15)
            check(bool(upd), 'S  15 秒內 tag 更新：lot.id=%r lot.operator=%r' % (
                cdp.eval("HT9045Tags.get('lot.id')"), cdp.eval("HT9045Tags.get('lot.operator')")))
            time.sleep(1.0)
            t1 = verify_dom(cdp, 'lot.start 之後')
            v = {tg: t1[tg][1] for tg in t1}
            check(res.get('lotId') == v.get('lot.id') and res.get('operatorId') == v.get('lot.operator'),
                  'S  tag＝C++ 元件值（ack lotId=%r operatorId=%r；tag %r／%r）' % (
                      res.get('lotId'), res.get('operatorId'), v.get('lot.id'), v.get('lot.operator')))
            cfg1 = read_lot_info()
            bad = ['%s=%r≠[%s]%r' % (tg, v.get(tg), k, cfg1.get(k)) for tg, k in TAG_KEY if v.get(tg) != cfg1.get(k, '')]
            check(not bad, 'S  tag＝config.ini [Lot Info]（Lot No／Lot ID／Operator／Start Time／Run Mode／Device Name；不符：%s）' % bad)
            st2 = v.get('lot.startTime') or ''
            today = time.strftime('%Y%m%d')
            check(bool(re.match(r'^\d{8}_\d{6}$', st2)) and st2[:8] == today,
                  'S  lot.startTime＝golden 格式 yyyymmdd_hhnnss 且是今天（%r；uLotInfo.cpp:1630-1634）' % st2)
            check(cfg1.get('LotStartTime', '').startswith(time.strftime('%Y-%m-%d')),
                  'S  config.ini LotStartTime 是今天（掛鐘不是哨兵 9999）：%r' % cfg1.get('LotStartTime'))
            # Lot Info 子分頁 12 格（golden ReadWriteLotInfo(false) :1497-1528 把元件寫進 config.ini；同一份文字）
            tc = tags(cdp, [c[0] for c in CHIPADV])
            dc = texts(cdp, [c[1] for c in CHIPADV])
            bad = ['%s=%r≠[%s]%r' % (tg, tc[tg][1], k, cfg1.get(k)) for tg, _, k in CHIPADV if k and tc[tg][1] != cfg1.get(k, '')]
            check(not bad, 'S  Lot Info 子分頁 11 格 tag＝config.ini [Lot Info]（不符：%s）' % bad)
            bad = [el for tg, el, _ in CHIPADV if dc[el] != fmt(tc[tg][1])]
            check(not bad, 'S  Lot Info 子分頁 12 格畫面＝tag（不符：%s）' % bad)
            print('  INFO  lot.machineId=%r（golden 只在 ReadWriteLotInfo(true) :1486 填 IniConfig.SocketHandlerID；'
                  '移植樹開機讀取是 GATE WC-19，所以 lot.start 之後仍是空字串 —— 已知落差）' % tc['lot.machineId'][1])
            print('  INFO  lot.start 之後 tag：%s' % ', '.join('%s=%r' % (tg, v.get(tg)) for tg in LOT_TAGS))

        # ---- OP：WS lotinfo.op ---------------------------------------------
        section_ops(cdp, a, vis, bc0)

        # ---- F：假數字 -----------------------------------------------------
        vals = json.loads(cdp.eval("JSON.stringify(Array.prototype.map.call(document.querySelectorAll('[data-pane=acm] .v'),"
                                   "function(e){return e.textContent.trim();}))"))
        check(len(vals) == 2 and all(x in ('---', '--- / ---') for x in vals),
              'F  Auto Clean Monitor（客戶專屬 CC_SCK）2 格是 "---"：%s' % vals)
        scan_fake(cdp, 'Data.LotInfo.html', '[data-pane=atc], [data-pane=barcode], [data-pane=acm]')

        ok = open_page(cdp, a.port, 'Data.TestCategory.html', "document.readyState==='complete'")
        cells = json.loads(cdp.eval("JSON.stringify(Array.prototype.map.call(document.querySelectorAll('table.cat td'),"
                                    "function(e){return [e.textContent.trim(), e.className];}))"))
        # Steven 20260925：TestCategory 頁已改接 tcat.* tag（golden sgArm1DrawCell 的結果），逐格驗證改由 data_testcategory_probe.py；
        # 這裡只留「沒有止血前的綠底 td.g」與掃不到舊展示值
        bad = [c for c in cells if 'g' in c[1].split()]
        check(bool(ok) and not bad, 'F  Data.TestCategory.html 開頁、沒有止血前的綠底 td.g（不符：%s）' % bad)
        scan_fake(cdp, 'Data.TestCategory.html', 'table.cat')

        ok = open_page(cdp, a.port, 'Data.SortCT.html',
                       "document.readyState==='complete' && window.HT9045Page && window.HT9045Tags && HT9045Tags.status().connected")
        # Steven 20260925：SortCT 頁已改接真資料（ART 分頁與 33 站，sort.* tag），逐格驗證改由 data_sortct_probe.py 負責；
        # 這裡只留「掃不到舊的展示數字」。
        check(bool(ok), 'F  Data.SortCT.html 開頁（tag 連線）')
        scan_fake(cdp, 'Data.SortCT.html', '[data-pane=art]')
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)
    print('data_lotinfo_probe: %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
