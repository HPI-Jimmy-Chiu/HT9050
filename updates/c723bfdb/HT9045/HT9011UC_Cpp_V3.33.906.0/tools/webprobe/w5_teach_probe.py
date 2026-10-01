# =============================================================================
#  tools/webprobe/w5_teach_probe.py -- W5 Teach page (C route) end-to-end probe.
#
#  AI(W906-W5-TEACH) 20260925: new file. Drives a running wb_serve over the real
#  WebSocket and checks the Teach page's load / save against the REAL teach.ini:
#
#    1. editlist.save Teach before any editlist.get      -> refused ("reload page")
#    2. editlist.get Teach                               -> struct Teach, elTeach 203 entries, TECH_* lists
#    3. page values == teach.ini values                  (TECH_PARA sample + elTeach sample)
#    4. save one TECH_PARA value v -> v+1 (answer YES)   -> saved, teach.ini [group] key == v+1
#    5. editlist.get again                               -> the widget shows v+1 (mirrored from the C++ variable)
#    6. save v+2 answering NO                            -> saved:false, file still v+1, page back to v+1
#    7. save the original v back                         -> file == v
#    8. system.file.put tag=teach (B route)              -> 409 owned by C route
#   W5-b（教導頁運動鈕；這台沒卡、沒歸零，驗整條路到 golden 的拒絕點）：
#    9. teachGo GoButton030（MInArmZA）                  -> "motor need home"（GoButton140 的檢查）
#   10. teachSet SetButton030 start                      -> "motor need home"（SetButton140 的檢查，不動伺服）
#   11. 怪按鈕 SetBtnPADView_Z（TECH_PARA 綁 SetButton020Click）-> 拒絕「怪按鈕」、指向 docs/W5_PROGRESS.md §6（W5-b）
#   11b. SetButton060（golden 建構子 :271 覆寫成 SetButton140Click）-> 進到 SetButton140 的檢查（「need home」），不是怪按鈕
#   11c. SetButton202（golden :474 已註解掉的死登錄，.dfm Visible=False）-> 拒絕「golden 按不到」
#   11f. SetInSmartSetupButton（從來沒登錄，.dfm Tag 0）    -> 拒絕「未登錄」，說出 golden 會讀 TechTwoPara[0]
#   11g. GoButton068（沒歸零）                           -> "need home"，拒絕尾巴帶 active=MTestZ2（golden ActiveMotorIndex，覆核第二輪 R-W5B-1）
#   11d. SetButtonInSh1LtcSenZ1（MotorTrayXClick）      -> ok、selectOnly、selectMotor
#   11e. teachSet query／結束（沒有進行中的）           -> ok、teachActive:false、沒有 positions
#   12. setTeachFromCurrent                              -> ui（頁面自己寫 EditPtr）
#  AI(W906-W5-b) 20260925: W5-b 覆核 —— [11] 原本拿 SetButton060 當怪按鈕，那是產生器漏套 golden 建構子覆寫造成的錯誤預期（W5B-1）。
#
#  ⚠ Run only after `python tools/realfile_guard.py snap <tag>` -- step 4/7 write the real
#    system\teach.ini (and wb_serve boot writes Gerneral.ini / machinerecord.dat).
#  Exit 0 = every expectation met.
# =============================================================================
import argparse, json, os, random, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from w4_motor_probe import Conn, check, FAILS, req   # noqa: E402

TEACH_INI = r'D:\HT9045\system\teach.ini'
Q_SAVE = 'Sure to Save? (確定要存檔?)'


def read_ini(path):
    """Win32 profile semantics: section / key case-insensitive, first occurrence wins."""
    d, sec = {}, None
    for ln in open(path, 'rb').read().decode('cp950', 'replace').splitlines():
        s = ln.strip()
        m = re.match(r'^\[(.*)\]$', s)
        if m:
            sec = m.group(1).strip().lower()
            d.setdefault(sec, {})
            continue
        if sec is None or '=' not in s or s.startswith(';'):
            continue
        k, v = s.split('=', 1)
        d[sec].setdefault(k.strip().lower(), v.strip())
    return d


def ini_get(d, sec, key):
    return d.get(sec.lower(), {}).get(key.lower())


def entries(page, name):
    return ((page.get('lists') or {}).get(name) or {}).get('entries') or []


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--host', default='127.0.0.1')
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--path', default='/ht9045')
    ap.add_argument('--seconds', type=float, default=60.0)
    a = ap.parse_args()

    c = Conn(a.host, a.port, a.path, a.seconds)
    r = c.cmd('control.takeover')  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
    check(r.get('ok') is True, 'control.takeover', json.dumps(r, ensure_ascii=False))

    print('[1] save before any page open')
    r = c.cmd('editlist.save', 'Teach', json.dumps({'widgets': {}, 'answers': {}}))
    check(r.get('ok') is False and 'reload page' in str(r.get('error', '')), 'save before get -> refused', json.dumps(r, ensure_ascii=False)[:300])

    print('[2] editlist.get Teach')
    pg = c.cmd('editlist.get', 'Teach')
    check(pg.get('ok') is True and pg.get('struct') == 'Teach', 'editlist.get Teach ok', json.dumps(pg, ensure_ascii=False)[:300])
    tp, tw, ts, gen, el = (entries(pg, n) for n in ('techPara', 'techTwoPara', 'techSuckPara', 'general', 'elTeach'))
    print('  techPara %d  techTwoPara %d  techSuckPara %d  general %d  elTeach %d' % (len(tp), len(tw), len(ts), len(gen), len(el)))
    check(len(el) == 203, 'elTeach has the 203 golden InitialTeachEditList entries', str(len(el)))
    check(len(tp) > 100 and len(ts) >= 32 and len(gen) == 4, 'TECH_PARA / SUCKPARA / general lists present')

    print('[3] page == teach.ini')
    ini = read_ini(TEACH_INI)
    shuttle = re.compile(r'^MIn(Shutte|Shuttle)[12]$')
    cand = [e for e in tp if e.get('group') and not shuttle.match(e['group']) and ini_get(ini, e['group'], e['key']) is not None]
    check(len(cand) > 50, 'enough TECH_PARA entries with a key in teach.ini', str(len(cand)))
    bad = [(e['id'], e['group'], e['key'], e['text'], ini_get(ini, e['group'], e['key']))
           for e in cand if str(int(ini_get(ini, e['group'], e['key']) or 0)) != e['text']]
    check(not bad, 'every TECH_PARA page value == teach.ini (%d compared)' % len(cand), repr(bad[:5]))
    elc = [e for e in el if e.get('group') and ini_get(ini, e['group'], e['key']) is not None and 'text' in e]

    def same(page_text, file_text):
        # HTEditList 的 double 筆畫面照登錄的小數位數顯示（"0.000"），檔案存的是 "0.000000" —— 比數值（20260925 實測）
        if page_text == file_text:
            return True
        try:
            return abs(float(page_text) - float(file_text)) < 1e-9 or \
                ('.' in page_text and abs(float(page_text) - float(file_text)) <= 0.5 * 10 ** -len(page_text.split('.')[1]))
        except ValueError:
            return False
    elbad = [(e.get('id'), e['group'], e['key'], e['text'], ini_get(ini, e['group'], e['key']))
             for e in elc if not same(e['text'], ini_get(ini, e['group'], e['key']))]
    check(len(elc) > 0 and not elbad, 'every elTeach page value == teach.ini (%d compared)' % len(elc), repr(elbad[:5]))

    print('[4] save v -> v+1 (YES)')
    random.seed(20260925)
    x = random.choice(cand)
    v = int(x['text'])
    print('  widget %s  [%s] %s  = %d' % (x['id'], x['group'], x['key'], v))
    r = c.cmd('editlist.save', 'Teach', json.dumps({'widgets': {x['id']: {'text': str(v + 1)}}, 'answers': {Q_SAVE: 1}}))
    sess = r.get('session') or {}
    check(r.get('ok') is True and r.get('saved') is True, 'save YES -> saved', json.dumps(r, ensure_ascii=False)[:400])
    print('  asked=%s  todo=%s' % ([q.get('en') for q in sess.get('asked') or []], sess.get('todo')))
    after = ini_get(read_ini(TEACH_INI), x['group'], x['key'])
    check(after == str(v + 1), 'teach.ini now holds v+1', '%r' % after)
    # golden btnSaveClick :2280 寫 atoi(InSHZDownRange->Text)；FormShow 只在 In_Shuttle_Auto_Latch==eInSHAutoLtc 時填它（:1961），
    # 否則是 .dfm 的空字串 ⇒ 寫 0（golden 同；docs/W5_PROGRESS.md §4-5）
    gtxt = [e['text'] for e in gen if e['id'] == 'InSHZDownRange'][0]
    gv = ini_get(read_ini(r'D:\HT9045\system\Gerneral.ini'), 'Shuttle', 'iInShtZRange')
    check(gv == str(int(gtxt or 0)), 'Gerneral.ini [Shuttle] iInShtZRange == atoi(InSHZDownRange text %r) (golden)' % gtxt, repr(gv))

    print('[5] reopen shows the C++ variable')
    pg2 = c.cmd('editlist.get', 'Teach')
    y = [e for e in entries(pg2, 'techPara') if e['id'] == x['id']]
    check(y and y[0]['text'] == str(v + 1), 'reopened page shows v+1', repr(y[:1]))

    print('[6] save v+2 answering NO')
    r = c.cmd('editlist.save', 'Teach', json.dumps({'widgets': {x['id']: {'text': str(v + 2)}}, 'answers': {Q_SAVE: 2}}))
    # golden asks only while fAllMotorHome==false; if the process has homed, YES/NO is not asked and it saves
    asked = [q.get('en') for q in ((r.get('session') or {}).get('asked') or [])]
    if Q_SAVE in asked:
        check(r.get('ok') is True and r.get('saved') is False, 'answer NO -> saved:false', json.dumps(r, ensure_ascii=False)[:300])
        check(ini_get(read_ini(TEACH_INI), x['group'], x['key']) == str(v + 1), 'teach.ini still v+1 after NO')
        pg3 = c.cmd('editlist.get', 'Teach')
        y = [e for e in entries(pg3, 'techPara') if e['id'] == x['id']]
        check(y and y[0]['text'] == str(v + 1), 'page back to v+1 after NO', repr(y[:1]))
    else:
        print('  SKIP: golden did not ask (fAllMotorHome true)')

    print('[7] restore v')
    c.cmd('editlist.get', 'Teach')
    r = c.cmd('editlist.save', 'Teach', json.dumps({'widgets': {x['id']: {'text': str(v)}}, 'answers': {Q_SAVE: 1}}))
    check(r.get('ok') is True and r.get('saved') is True, 'restore save', json.dumps(r, ensure_ascii=False)[:300])
    check(ini_get(read_ini(TEACH_INI), x['group'], x['key']) == str(v), 'teach.ini back to v')

    print('[8] B route write to teach.ini refused')
    r = c.cmd('system.file.put', 'teach', json.dumps({'sections': {}}))
    check(r.get('ok') is False and 'C route' in str(r.get('error', '')), 'system.file.put teach -> owned by C route', json.dumps(r, ensure_ascii=False)[:300])

    # ---- W5-b（AI(W906-W5-TEACH) 20260925）：教導頁運動鈕走 golden 按鈕表＋golden 互鎖。這台沒卡、沒歸零 ⇒ 驗的是整條路
    #   （WS → 解析 btn／targets → golden 按鈕表 → 核對馬達 → CheckCanMove／IsCanQuickJogMove → 照實拒絕在「需要歸零」或「怪按鈕」）
    # AI(W906-W5-b) 20260925: W5B-4 —— 目標值改成 fields（{畫面欄位 id: 值}），取 C 路載入的值（editlist.get）
    zsafe = [e for e in tp if e['id'] == 'setEditInZSafeHeight']
    zval = int(zsafe[0]['text']) if zsafe and re.match(r'^-?\d+$', zsafe[0].get('text', '')) else 0
    print('[9] teachGo GoButton030 (MInArmZA, not homed) -> golden "motor need home"')
    r = c.cmd('motor.access', 'MInArmZA', req('teachGo', ['MInArmZA'], source='uteach', button='GoButton*',
                                                params={'btn': 'GoButton030', 'fields': {'setEditInZSafeHeight': zval}, 'speed': 1}))
    check(r.get('ok') is False and 'need home' in str(r.get('error', '')), 'teachGo reaches GoButton140 checks (golden registry + Tag + interlocks)', json.dumps(r, ensure_ascii=False)[:300])
    check('[editPtr=' not in str(r.get('error', '')), 'GoButton140 "need home" leaves EditPtr alone (golden sets it after the HomeFlag check)', json.dumps(r, ensure_ascii=False)[:300])
    print('[10] teachSet SetButton030 start -> golden "motor need home" (no servo change)')
    r = c.cmd('motor.access', 'MInArmZA', req('teachSet', ['MInArmZA'], source='uteach', button='SetButton*',
                                                params={'btn': 'SetButton030', 'start': True}))
    check(r.get('ok') is False and 'need home' in str(r.get('error', '')), 'teachSet reaches SetButton140 checks', json.dumps(r, ensure_ascii=False)[:300])
    # AI(W906-W5-b) 20260925: 覆核第二輪 R-W5B-1 —— 尾巴是 " [editPtr=…,active=…]"（golden EditPtr 與 ActiveMotorIndex 一起帶）
    err = str(r.get('error', ''))
    check('editPtr=setEditInZSafeHeight' in err and 'active=' in err, 'SetButton140 refusal carries golden EditPtr and ActiveMotorIndex (set before the HomeFlag check, W5B-13 / R-W5B-1)', json.dumps(r, ensure_ascii=False)[:300])
    print('[11] quirk button SetBtnPADView_Z (TECH_PARA bound to SetButton020Click) -> option C (ruling 2C): own row = SetButton140 on MOutArmZE -> need home here')
    r = c.cmd('motor.access', 'MOutArmZE', req('teachSet', ['MOutArmZE'], source='uteach', button='SetButton*',
                                                 params={'btn': 'SetBtnPADView_Z', 'start': True}))
    err = str(r.get('error', ''))
    check(r.get('ok') is False and 'need home' in err and '怪按鈕' not in err,   # AI(W906-W5-b) 20260925: 裁決 2C（RULINGS_20260925 第 9 條）後怪按鈕走自己那一列
          'quirk button runs its own row (SetButton140 checks), not refused as a quirk', json.dumps(r, ensure_ascii=False)[:400])
    print('[11b] SetButton060 (golden ctor :271 SetButton140Click) -> SetButton140 checks, not a quirk')
    r = c.cmd('motor.access', 'MInShuttle1', req('teachSet', ['MInShuttle1'], source='uteach', button='SetButton*',
                                                   params={'btn': 'SetButton060', 'start': True}))
    err = str(r.get('error', ''))
    check(r.get('ok') is False and 'need home' in err and '怪按鈕' not in err, 'SetButton060 is a normal SetButton140 (W5B-1)', json.dumps(r, ensure_ascii=False)[:300])
    print('[11c] SetButton202 (golden :474 commented out, .dfm Visible=False) -> golden cannot press it')
    r = c.cmd('motor.access', 'MInShuttle1', req('teachSet', ['MInShuttle1'], source='uteach', button='SetButton*',
                                                   params={'btn': 'SetButton202', 'start': True}))
    check(r.get('ok') is False and '按不到' in str(r.get('error', '')), 'dead registration refused as unreachable in golden (W5B-8 / R-W5B-2)', json.dumps(r, ensure_ascii=False)[:300])
    print('[11f] SetInSmartSetupButton (never registered, .dfm Tag 0) -> unregistered quirk')
    r = c.cmd('motor.access', 'MInArmX', req('teachSet', ['MInArmX'], source='uteach', button='SetButton*',
                                               params={'btn': 'SetInSmartSetupButton', 'start': True}))
    err = str(r.get('error', ''))
    check(r.get('ok') is False and '未登錄' in err and 'TechTwoPara[0]' in err, 'unregistered teach button refused, says golden reads TechTwoPara[0] (R-W5B-3)', json.dumps(r, ensure_ascii=False)[:400])
    print('[11g] GoButton068 (not homed) -> need home, refusal carries active=MTestZ2')
    r = c.cmd('motor.access', 'MInArmX', req('teachGo', ['MInArmX'], source='uteach', button='GoButton*',
                                               params={'btn': 'GoButton068', 'fields': {}}))
    err = str(r.get('error', ''))
    check(r.get('ok') is False and 'active=MTestZ2' in err, 'GoButton068 -> golden ActiveMotorIndex = MTestZ2 (last registration), page must select it (R-W5B-1)', json.dumps(r, ensure_ascii=False)[:400])
    print('[11d] SetButtonInSh1LtcSenZ1 (MotorTrayXClick) -> select motor only')
    r = c.cmd('motor.access', 'MInSh1LtcSenZ1', req('teachSet', ['MInSh1LtcSenZ1'], source='uteach', button='SetButton*',
                                                      params={'btn': 'SetButtonInSh1LtcSenZ1', 'start': True}))
    check(r.get('ok') is True and r.get('result') == 'selectOnly' and r.get('selectMotor') == 'MInSh1LtcSenZ1',
          'MotorTrayXClick Set button: ok, selectOnly, no motion', json.dumps(r, ensure_ascii=False)[:300])
    print('[11e] teachSet query / end with nothing active')
    r = c.cmd('motor.access', 'MInArmZA', req('teachSet', ['MInArmZA'], source='uteach', button='SetButton*', params={'query': True}))
    check(r.get('ok') is True and r.get('teachActive') is False and 'positions' not in r, 'query: no hand-teach in progress (teachActive false)', json.dumps(r, ensure_ascii=False)[:300])
    r = c.cmd('motor.access', 'MInArmZA', req('teachSet', ['MInArmZA'], source='uteach', button='SetButton*', params={'btn': 'SetButton030', 'start': False, 'accept': True}))
    check(r.get('ok') is True and r.get('result') == 'teachIdle' and r.get('teachActive') is False and 'positions' not in r,
          'end with nothing active: idle, no positions written', json.dumps(r, ensure_ascii=False)[:300])
    print('[12] setTeachFromCurrent -> ui only')
    r = c.cmd('motor.access', 'MInArmZA', req('setTeachFromCurrent', ['MInArmZA'], source='uteach', button='btnSetTo'))
    check(r.get('ok') is True and r.get('layer') == 'ui', 'SetTo is ui-only (page writes EditPtr)', json.dumps(r, ensure_ascii=False)[:300])

    print('\n%d failed' % len(FAILS))
    return 1 if FAILS else 0


if __name__ == '__main__':
    sys.exit(main())
