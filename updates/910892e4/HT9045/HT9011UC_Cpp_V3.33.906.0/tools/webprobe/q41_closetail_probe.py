# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/q41_closetail_probe.py -- Q41 第 3 項「關窗尾段」第一批（LU-2、SP-6、BS-2、CT-4）＋ R87（HotPlate form.save 運轉中擋）。
#
#  AI(W906-FRW-S158) 20260927 [W906]  NOT in golden。方案 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_CLOSETAIL_PLAN_20260927.md；
#  裁決 RULINGS_20260926 S107-1（存檔後就跑）、decisions-pending R84～R87（Steven 沒反對就照 St01 建議 A）。
#  C++：FileRW/MainClick.cpp 檔尾（W906_Main_sbLdUldClickTail／sbSpeedClickTail／sbBinClickTail／sbContactClickOpen）、
#  各頁入口 FileRW/Ld_UldDelayTime.cpp、ArmSpeed_File.cpp、BinSelect.cpp、DeviceForm_File.cpp；R87 在 JsonBridge/FormJson.cpp FormSave。
#  **寫好、還沒跑過**（Steven01 這台不跑 wb_serve，.claude/skills/ht9045-html-json/references/wbserve-conventions.md §6）。
#  AI(W906-FRW-S158) 20260927 [W906] 第二批（一樣寫好、沒跑過）：TF-5 UserDefForm_File（W906_Main_sbTrayFormClickTail）、
#    TA-6 TrayForm＝Tray Assignment 頁（W906_Main_sbTrayAssignClickTail）、YM-5 TestIF_File_YieldMonitoring（W906_Main_sbYieldClickTail）
#    併進 S2／R1 的 PAGES；HP-3 HotPlate（A 形狀 form.save，W906_Main_sbPlateFormClickTail）＝S3（--hotplate）；CC-E15 Configuration
#    （editlist.get／save IniConfig，W906_Main_sbConfigurationClickHead／Tail）＝S4（--config），運轉中另外併進 R1。
#
#  兩個觀察點：
#    trace   ack.session.trace（filerw::ELMark）：尾段一進來記 "<函式名>"；運轉中不跑再多記 "<函式名>:running"
#            （原因在 ack.session.todo，"<函式名> not run (R86): …"）
#    主控台  wb_serve 的 stdout（--console <檔>：啟動 wb_serve 時 > 導到那個檔；wb_serve.cpp:3617 setvbuf _IONBF，一行一行即時落地）：
#            跑了 "close-tail <函式名> -> ran: …"；沒跑 "close-tail <函式名> -> not run: …"。沒給 --console 就只看 trace。
#
#  驗什麼（不用瀏覽器，直接走 WS；機台狀態看 auth.mode 的 systemStart）：
#    S  機台停著：
#       S1 editlist.get DeviceForm_File（Contact 開頁＝golden V912 main.cpp:28314 Show() 之後的 :28315-28316，R84）→ ok、
#          trace 有 W906_Main_sbContactClickOpen、沒有 ":running"；主控台 "-> ran"。
#          ⚠ 會跑 golden FormShow＋DoStructUnitConvert＋SetWorkParameter（以讀為主，見下面「會寫的檔」）；--no-open 跳過。
#       S2（--save 才做，會真的寫配方檔）Ld_UldDelayTime／ArmSpeed_File／BinSelect：editlist.get → 原值存回（widgets 由回應的
#          proxies 組：editable 或 mustSend 的替身；值欄位 text／checked／itemIndex／position／dateTime／cells，mustSend 的另帶
#          其餘非狀態欄位）→ ok、saved（或 A02 的 closed）→ trace 有該頁的尾段名、沒有 ":running"；主控台 "-> ran"。
#          BinSelect 不帶 bin（＝golden 存 FormShow 讀進來的值，FileRW/BinSelect.cpp SaveFlow 的 reason）。
#       S3（--hotplate 才做，會真的寫 HotPlate.Data）HotPlate：GET /api/form/Setup.HotPlate.html → widgets 的 text／itemIndex／checked
#          原值 form.save → ok、saved（或 A02 的 closed）→ ack.todo 沒有 "W906_Main_sbPlateFormClickTail not run"；主控台
#          "close-tail W906_Main_sbPlateFormClickTail -> ran"（行尾帶 InitialOK=0／1：0 時 GetHotPlateYHalfPos 在 golden 第一行就 return）。
#          A 形狀沒有 session.trace，只看 todo 與主控台。沒帶 --hotplate 只印 INFO。
#       S4（--config 才做）Configuration：editlist.get IniConfig（主控台 "close-tail W906_Main_sbConfigurationClickHead -> recorded"）
#          → 原值 editlist.save、answers 不答（＝golden「Config data save to define?」NO，config.ini 不寫）→ ok、saved=false，
#          trace 仍有 W906_Main_sbConfigurationClickTail（golden 關窗不論存不存都跑）、沒有 ":running"；主控台 "-> ran"。
#          ⚠ 答 NO 也會寫：尾段的 UpdateMainOperateMode 寫 D:\HT9045\system\lastdata.dat（路徑寫死，--dry 導不到）並切加熱器繼電器、
#            送 ATC7 指令（HT9050 機台要機台端在場，RULINGS_20260927 第 16 條）；fYieldMonitoring->ReadFile 缺鍵補寫 <配方>\Tester.Data。
#    R  機台運轉中（systemStart=true；--expect-running 時不是運轉中算失敗）：
#       R1 editlist.save Ld_UldDelayTime／ArmSpeed_File／BinSelect／UserDefForm_File／TrayForm／TestIF_File_YieldMonitoring／IniConfig
#          （空 widgets）→ 被 RULINGS_20260927 #7 擋（tools/wb_serve.cpp:5303，
#          在尾段之前）；主控台沒有這一頁的 "close-tail" 行。
#       R2 editlist.get DeviceForm_File → 被開窗閘 "running:" 擋（FileRW/_EditPage.cpp GTools 查 SystemStart），主控台沒有 "-> ran"。
#          SoftStart 已起、SystemStart 還沒起的那一瞬間才會走到尾段自己的 R86 檢查（trace ":running"、todo 有原因、主控台
#          "-> not run"）—— 探針抓不到那個窗口；抓到了照樣驗，沒抓到只印 INFO。
#       R3 form.save Setup.HotPlate.html value {"widgets":{}} → error 以 "running:" 開頭、含「不能從網頁存設定」與「R87」
#          （擋在 sourceGap 與空跑之前，什麼檔都沒寫）。
#
#  ⚠ --save：照 Jimmy 0918「備份→驗證→還原」—— 跑之前自己備份 <配方> 資料夾、D:\HT9045\system\Gerneral.ini、teach.ini；
#    探針前後比 SHA256（--recipe-dir 整個資料夾＋Gerneral.ini＋--watch 列的檔），有變就列出來，不自動還原。
#    預期會變：UdUld.Data、ArmCondition.Data（A57_1 存到 machine 時是 sSaveByMachine）、Binasgn*.Data（存檔鈕本來就寫，golden
#    格式正規化）、BackupSetupFile 的備份；SetWorkParameter（BS-2、CT-4 的尾段）缺鍵時補寫 Gerneral.ini [Shuttle] CHECK_RANGE／
#    iInShtZRange（⛔ 20260927 更正：--dry 已取消，現在寫的是真的 D:\HT9045\system\Gerneral.ini）。
#    AI(W906-FRW-S158) 20260927 第二批另外會變：Tray.Data（TrayForm／Tray Assignment 存檔鈕）、Tester.Data（Yield 存檔鈕；--config 的
#    fYieldMonitoring->ReadFile 缺鍵補寫）、HotPlate.Data（--hotplate）、HandlerCondition.Data（--hotplate 的 LoadAutoCleanData 每次寫
#    iIndexArmAutoCleanCnt）、D:\HT9045\system\RunMode.txt 與事件紀錄 MES2107／ChangeLog（Yield 尾段的 SetStartModeData）、
#    D:\HT9045\system\lastdata.dat（SetStartModeData／--config 的 UpdateMainOperateMode；路徑寫死，--dry 導不到）。
#    lastdata.dat、RunMode.txt、config.ini 一律列進 SHA256 前後比。
#  ⚠ 權限：golden A02（IniConfig.bA02DisableSaveParsWhenSwitchToOp 且 Operator）時 Ld_ULd／BinSel／TrayForm／Tray Assignment／Yield／
#    HotPlate 的存檔鈕直接 Close() → 走 closed 那一支（先 FormClose 再尾段，尾段照跑）。要走正常存檔就帶 --user／--password。
#    Speed 沒有 A02 守衛；Configuration 的存檔就是 FormClose。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046 > %TEMP%\wb_console.txt 2>&1   # ⛔ 20260927 更正：--dry 已取消（帶了 wb_serve 直接結束，tools/wb_serve.cpp:3683-3684）；在 Steven01 怎麼跑見 D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\wbserve-sandbox-run.md
#      python tools\webprobe\q41_closetail_probe.py --port 8046 --console %TEMP%\wb_console.txt
#             [--save --recipe-dir D:\HT9045\IniData\Data\<recipe>] [--hotplate] [--config] [--user U --password P]
#             [--no-open] [--expect-running]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import hashlib
import json
import os
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake, send_text, read_frames   # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
GENERAL_INI = r'D:\HT9045\system\Gerneral.ini'
SYSTEM_WATCH = [r'D:\HT9045\system\lastdata.dat', r'D:\HT9045\system\RunMode.txt', r'D:\HT9045\config\config.ini']   # AI(W906-FRW-S158) 20260927 第二批
VALUE_KEYS = ('text', 'checked', 'itemIndex', 'position', 'dateTime', 'cells')
STATE_KEYS = ('visible', 'enabled', 'editable', 'tabVisible')
EMPTY_SAVE = json.dumps({'widgets': {}, 'answers': {}})
PAGES = [   # (editlist tag, 尾段函式名, golden)
    ('Ld_UldDelayTime', 'W906_Main_sbLdUldClickTail', 'V912 main.cpp:28454'),
    ('ArmSpeed_File', 'W906_Main_sbSpeedClickTail', 'V912 main.cpp:28696-28699'),
    ('BinSelect', 'W906_Main_sbBinClickTail', 'V912 main.cpp:28325-28327'),
    # AI(W906-FRW-S158) 20260927 [W906] 第二批（C 路，同一套 S2／R1）
    ('UserDefForm_File', 'W906_Main_sbTrayFormClickTail', 'V912 main.cpp:28411-28413'),
    ('TrayForm', 'W906_Main_sbTrayAssignClickTail', 'V912 main.cpp:28434-28437'),              # ⚠ TrayForm＝Tray Assignment 頁
    ('TestIF_File_YieldMonitoring', 'W906_Main_sbYieldClickTail', 'V912 main.cpp:28510-28512'),
]
CONTACT = ('DeviceForm_File', 'W906_Main_sbContactClickOpen', 'V912 main.cpp:28315-28316')
HOTPLATE = 'Setup.HotPlate.html'
HOTPLATE_TAIL = ('W906_Main_sbPlateFormClickTail', 'V912 main.cpp:28422-28425')                 # AI(W906-FRW-S158) 20260927
CONFIG = ('IniConfig', 'W906_Main_sbConfigurationClickHead', 'W906_Main_sbConfigurationClickTail',
          'V912 main.cpp:28615-28622 / :28626-28657')                                          # AI(W906-FRW-S158) 20260927


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def info(what):
    print('  INFO  ' + what)


class Ws(object):
    def __init__(self, port):
        self.sock, left = ws_handshake('127.0.0.1', port, '/ht9045', time.monotonic() + 5)
        self.frames = read_frames(self.sock, left, time.monotonic() + 900)
        self.n = 100
        self.sent = {}

    def cmd(self, name, tag=None, value=None):
        key = (name, tag, value)                   # WebCmdGuard：同 cmd＋tag＋value 400 ms 內重複回 busy:
        dt = time.monotonic() - self.sent.get(key, -9)
        if dt < 0.5:
            time.sleep(0.5 - dt)
        self.sent[key] = time.monotonic()
        self.n += 1
        m = {'type': 'cmd', 'id': self.n, 'cmd': name}
        if tag is not None:
            m['tag'] = tag
        if value is not None:
            m['value'] = value
        send_text(self.sock, json.dumps(m))
        for op, p in self.frames:
            if op != 1:
                continue
            try:
                d = json.loads(p.decode('utf-8', 'replace'))
            except ValueError:
                continue
            if d.get('type') == 'ack' and d.get('id') == self.n:
                return d
        return None


class Console(object):
    """wb_serve 主控台（stdout 導到檔）。mark() 記下目前長度，new_text() 回之後新增的字。沒給檔 ⇒ enabled=False。"""

    def __init__(self, path):
        self.path = path
        self.enabled = bool(path)
        self.pos = 0
        if self.enabled and not os.path.isfile(path):
            info('--console %s 不存在：只看 trace' % path)
            self.enabled = False

    def mark(self):
        if self.enabled:
            self.pos = os.path.getsize(self.path)

    def new_text(self, wait=0.3):
        if not self.enabled:
            return ''
        time.sleep(wait)
        with open(self.path, 'rb') as f:
            f.seek(self.pos)
            b = f.read()
        self.pos += len(b)
        return b.decode('utf-8', 'replace')


def err(ack):
    return (ack or {}).get('error') or ''


def short(ack):
    return json.dumps(ack, ensure_ascii=False)[:300]


def trace(ack):
    return ((ack or {}).get('session') or {}).get('trace') or []


def todo(ack):
    return ((ack or {}).get('session') or {}).get('todo') or []


def sha(path):
    return hashlib.sha256(open(path, 'rb').read()).hexdigest() if os.path.isfile(path) else None


def watch_list(a):
    out = [GENERAL_INI] + SYSTEM_WATCH + list(a.watch)
    if a.recipe_dir and os.path.isdir(a.recipe_dir):
        out += [os.path.join(a.recipe_dir, f) for f in sorted(os.listdir(a.recipe_dir))
                if os.path.isfile(os.path.join(a.recipe_dir, f))]
    return out


def roundtrip_widgets(g):
    """editlist.get 回應 → 原值存回的 widgets：editable 的替身帶值欄位；mustSend 的替身（不論能不能改）帶全部非狀態欄位
    （PageSave 少一個 mustSend 就 400；不可改的值 PageSave 自己丟進 ack.ignored）。"""
    px = g.get('proxies') or {}
    must = set(g.get('mustSend') or [])
    out = {}
    for name, p in px.items():
        if not isinstance(p, dict):
            continue
        if name in must:
            v = dict((k, x) for k, x in p.items() if k not in STATE_KEYS)
        elif p.get('editable'):
            v = dict((k, p[k]) for k in VALUE_KEYS if k in p)
        else:
            continue
        if v:
            out[name] = v
    missing = [m for m in must if m not in out]
    return out, missing


def expect_ran(ack, txt, con, fn, what):
    tr = trace(ack)
    check(fn in tr and (fn + ':running') not in tr, '%s trace 有 %s、沒有 ":running"：%s' % (what, fn, tr[-8:]))
    if con.enabled:
        check(('close-tail %s -> ran' % fn) in txt, '%s 主控台有 "close-tail %s -> ran"' % (what, fn))


def run_stopped(ws, a, con):
    print('== S  機台停著')
    struct, fn, golden = CONTACT
    if a.no_open:
        info('S1 --no-open：不送 editlist.get %s' % struct)
    else:
        con.mark()
        g = ws.cmd('editlist.get', struct)
        txt = con.new_text()
        check(bool(g and g.get('ok')), 'S1 editlist.get %s（golden %s，R84 開頁跑）：%s' % (struct, golden, err(g)[:200]))
        if g and g.get('ok'):
            expect_ran(g, txt, con, fn, 'S1')
    if not a.save:
        info('S2 沒帶 --save：%s 的存檔尾段不驗（會寫配方檔）' % '／'.join(p[0] for p in PAGES))
    else:
        for struct, fn, golden in PAGES:
            g = ws.cmd('editlist.get', struct)
            if not (g and g.get('ok')):
                check(False, 'S2 %s editlist.get：%s' % (struct, err(g)[:200]))
                continue
            w, missing = roundtrip_widgets(g)
            check(not missing, 'S2 %s mustSend 都在 proxies 裡（缺：%s）' % (struct, missing[:10]))
            con.mark()
            s = ws.cmd('editlist.save', struct, json.dumps({'widgets': w, 'answers': {}}))
            txt = con.new_text()
            tr = trace(s)
            ok = bool(s and s.get('ok'))
            saved, closed = bool((s or {}).get('saved')), 'closed' in tr
            check(ok and (saved or closed), 'S2 %s 原值存回 → ok、%s：%s' % (
                struct, 'closed（A02）' if closed else 'saved', err(s)[:200] if not ok else 'saved=%s' % saved))
            if ok and (saved or closed):
                expect_ran(s, txt, con, fn, 'S2 %s（golden %s）' % (struct, golden))
            if todo(s):
                info('S2 %s session.todo：%s' % (struct, json.dumps(todo(s), ensure_ascii=False)[:400]))
    run_hotplate(a, ws, con)
    run_config(a, ws, con)


def run_hotplate(a, ws, con):
    """AI(W906-FRW-S158) 20260927 [W906] S3：HP-3（A 形狀 form.save）。"""
    fn, golden = HOTPLATE_TAIL
    if not a.hotplate:
        info('S3 沒帶 --hotplate：不送 form.save %s（會真的寫 HotPlate.Data）；R87 只在運轉中驗' % HOTPLATE)
        return
    try:
        form = json.loads(urllib.request.urlopen('http://127.0.0.1:%d/api/form/%s' % (a.port, HOTPLATE), timeout=10)
                          .read().decode('utf-8', 'replace'))
    except Exception as e:   # noqa: BLE001 —— 探針：連不上就記失敗
        check(False, 'S3 GET /api/form/%s：%s' % (HOTPLATE, e))
        return
    w = {}
    for wid, v in (form.get('widgets') or {}).items():
        if isinstance(v, dict):
            x = dict((k, v[k]) for k in ('text', 'itemIndex', 'checked') if k in v)   # FormState::FromJson 只收這三種
            if x:
                w[wid] = x
    con.mark()
    r = ws.cmd('form.save', HOTPLATE, json.dumps({'widgets': w}))
    txt = con.new_text()
    ok = bool(r and r.get('ok'))
    saved, closed = bool((r or {}).get('saved')), bool((r or {}).get('closed'))
    check(ok and (saved or closed), 'S3 form.save %s 原值存回 → ok、%s：%s' % (
        HOTPLATE, 'closed（A02）' if closed else 'saved', err(r)[:200] if not ok else 'saved=%s' % saved))
    td = (r or {}).get('todo') or []
    if ok and (saved or closed):
        check(not any(t.startswith(fn + ' not run') for t in td), 'S3 ack.todo 沒有 "%s not run"（golden %s）' % (fn, golden))
        if con.enabled:
            check(('close-tail %s -> ran' % fn) in txt, 'S3 主控台有 "close-tail %s -> ran"' % fn)
            if 'InitialOK=0' in txt:
                info('S3 InitialOK=0：GetHotPlateYHalfPos 在 golden 第一行 return（wb_serve 的 PumpInit 沒 ARMED）')
    if td:
        info('S3 ack.todo：%s' % json.dumps(td, ensure_ascii=False)[:400])


def run_config(a, ws, con):
    """AI(W906-FRW-S158) 20260927 [W906] S4：CC-E15（editlist.get／save IniConfig；答 NO，config.ini 不寫，尾段照跑）。"""
    struct, head, fn, golden = CONFIG
    if not a.config:
        info('S4 沒帶 --config：不驗 Configuration 尾段（答 NO 也會寫 lastdata.dat、切加熱器繼電器、送 ATC7，見檔頭）')
        return
    con.mark()
    g = ws.cmd('editlist.get', struct)
    txt = con.new_text()
    if not (g and g.get('ok')):
        check(False, 'S4 editlist.get %s：%s' % (struct, err(g)[:200]))
        return
    if con.enabled:
        check(('close-tail %s -> recorded' % head) in txt, 'S4 主控台有 "close-tail %s -> recorded"（開窗前記舊值）' % head)
    w, missing = roundtrip_widgets(g)
    check(not missing, 'S4 %s mustSend 都在 proxies 裡（缺：%s）' % (struct, missing[:10]))
    con.mark()
    s = ws.cmd('editlist.save', struct, json.dumps({'widgets': w, 'answers': {}}))
    txt = con.new_text()
    ok = bool(s and s.get('ok'))
    check(ok, 'S4 %s 原值、不答「Config data save to define?」（＝NO）→ ok：%s' % (struct, err(s)[:200]))
    if ok:
        check(not (s or {}).get('saved'), 'S4 saved=false（答 NO 不寫 config.ini）')
        expect_ran(s, txt, con, fn, 'S4 %s（golden %s，不看有沒有存成）' % (struct, golden))
    if todo(s):
        info('S4 session.todo：%s' % json.dumps(todo(s), ensure_ascii=False)[:400])


def run_running(ws, a, con):
    print('== R  機台運轉中')
    for struct, fn, golden in PAGES + [(CONFIG[0], CONFIG[2], CONFIG[3])]:   # AI(W906-FRW-S158) 20260927：＋IniConfig
        con.mark()
        s = ws.cmd('editlist.save', struct, EMPTY_SAVE)
        txt = con.new_text()
        check(bool(s) and not s.get('ok') and '不能從網頁存設定' in err(s),
              'R1 %s 運轉中存檔先被 RULINGS_20260927 #7 擋：%s' % (struct, err(s)[:200]))
        if con.enabled:
            check(('close-tail %s' % fn) not in txt, 'R1 %s 主控台沒有 "close-tail %s"（尾段沒跑）' % (struct, fn))
    struct, fn, golden = CONTACT
    con.mark()
    g = ws.cmd('editlist.get', struct)
    txt = con.new_text()
    if g and not g.get('ok') and err(g).startswith('running:'):
        check(True, 'R2 %s 運轉中開頁被開窗閘 running: 擋：%s' % (struct, err(g)[:160]))
        if con.enabled:
            check(('close-tail %s -> ran' % fn) not in txt, 'R2 主控台沒有 "close-tail %s -> ran"' % fn)
    elif g and g.get('ok') and (fn + ':running') in trace(g):
        check(True, 'R2 %s 開頁過了開窗閘（SoftStart、SystemStart 還沒起），尾段自己的 R86 擋下：trace %s' % (struct, trace(g)[-6:]))
        check(any(t.startswith(fn + ' not run (R86)') for t in todo(g)), 'R2 session.todo 有 "%s not run (R86)"' % fn)
        if con.enabled:
            check(('close-tail %s -> not run' % fn) in txt and ('close-tail %s -> ran' % fn) not in txt,
                  'R2 主控台 "close-tail %s -> not run"、沒有 "-> ran"' % fn)
    else:
        check(False, 'R2 %s 運轉中開頁：預期開窗閘 running: 或尾段 R86 擋，實際 %s' % (struct, short(g)))
    r = ws.cmd('form.save', HOTPLATE, json.dumps({'widgets': {}}))
    e = err(r)
    check(bool(r) and not r.get('ok') and e.startswith('running:') and '不能從網頁存設定' in e and 'R87' in e,
          'R3 form.save %s 運轉中被擋（R87，running:）：%s' % (HOTPLATE, e[:220]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--console', help='wb_serve stdout 導到的檔（看 "close-tail …" 那一行）')
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--save', action='store_true', help='S2：Ld_UldDelayTime／ArmSpeed_File／BinSelect 原值存回（會寫配方檔）')
    ap.add_argument('--recipe-dir', help='SHA256 前後比的配方資料夾（--save 時建議給）')
    ap.add_argument('--watch', action='append', default=[], help='另外要比 SHA256 的檔（可重複）')
    ap.add_argument('--hotplate', action='store_true', help='S3：HotPlate form.save 原值存回（會寫 HotPlate.Data、HandlerCondition.Data）')
    ap.add_argument('--config', action='store_true', help='S4：Configuration 原值、答 NO（config.ini 不寫；尾段寫 lastdata.dat、切加熱器繼電器、送 ATC7）')
    ap.add_argument('--no-open', action='store_true', help='S1 不送 editlist.get DeviceForm_File（不跑 golden FormShow／尾段）')
    ap.add_argument('--expect-running', action='store_true')
    a = ap.parse_args()

    watched = watch_list(a)
    before = dict((p, sha(p)) for p in watched)
    con = Console(a.console)
    ws = Ws(a.port)
    r = ws.cmd('control.acquire')
    check(bool(r and r.get('ok')), 'control.acquire（editlist.save／form.save 要權杖）')
    if a.user:
        r = ws.cmd('auth.login', a.user, a.password or '')
        if r and not r.get('ok') and 'already logged in' in err(r):
            ws.cmd('auth.logout')
            time.sleep(0.5)   # WebCmdGuard: a second auth.login within 400 ms of the first one is refused as busy (Q50 20260928)
            r = ws.cmd('auth.login', a.user, a.password or '')
        check(bool(r and r.get('ok')), 'auth.login %s' % a.user)
    st = ws.cmd('auth.mode') or {}
    print('auth.mode：%s' % short(st))
    if st.get('systemStart'):
        run_running(ws, a, con)
    else:
        check(not a.expect_running, '--expect-running 但機台沒在運轉（auth.mode systemStart=false）')
        run_stopped(ws, a, con)
    ws.cmd('control.release')

    after = dict((p, sha(p)) for p in watched)
    moved = [p for p in watched if before[p] != after[p]]
    print('== 檔案（SHA256 前後）：%d 個有變%s' % (len(moved), '' if a.save or a.hotplate or a.config or not moved else
                                               '（沒帶 --save：只可能是 golden FormShow／SetWorkParameter 的補鍵，見檔頭）'))
    for p in moved:
        print('     ' + p)
    print('FAILS: %d' % len(FAILS))
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
