# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_smartdiag_probe.py -- Data.SmartDiagnostic.html（golden V912 TfSmartDiagnostic，
#  SmartDiagnostic.cpp／SmartDiagnostic.dfm）接 C++ smartdiag.op 的 e2e probe。
#
#  Steven 團隊 20260925 (Data.SmartDiagnostic).
#
#  用真的瀏覽器（headless Edge ＋ DevTools）開頁面，驗：
#    O  開窗：state.created；sgCylinder 9×11 的表頭／名稱欄／Sensor QTY 欄＝golden FormShow（:290-344）
#    F  state＝檔案：自己讀 D:\HT9045\system 的四個檔，照 golden 的讀法算一次再比
#         SmartDiagnosticRecord.txt      沒有 → Summary 2 列（dfm RowCount=2）、表頭＝InitialVariable :94-99；
#                                        有 → 列數＝行數、每格＝golden SplitStrByDotSpaceOnly（, \t \r : 分隔，連續分隔併掉）
#         SmartDiagnosticRecordReset.txt 同上（CyliderManagement）
#         SmartDiagnosticPara.ini        [Setup] LimitCount／LimitCheckTime（沒有＝0，TIniFile ReadInteger 預設）
#         CylinderPreAlm.ini             沒有 → sgCylinder 第 3/4/6/7/8 欄空白（golden ReadCylinderData :869-872 直接 return）
#       顏色＝golden InitialStringGridColor（第 0 列 0x00917B51、其他 0x00DFD9CC）
#    D  畫面＝state：三張表的格子文字、下拉項目、兩個參數框
#    C  寫入（--no-write 跳過）：
#         create 取消段 → needConfirm（"Sure To Create New Report?"）、state 不變；確認段 → Summary 296 列（以 Type_HT9050 跑時 322 列，加 --type9050），
#                有名字的列 ON/OFF="0"、下拉＝這些名字、Management 同名＋"0"
#         reset  → 照 golden 迴圈（第 i 列比「第 i 個下拉項」，超過項目數時比 ""）自己算出被重設的列，比對；
#                  選到的那一列重畫成 clWindow；ItemIndex 回 -1
#         cell   → sgCylinder[3,1] 填 2500；[4,1] 取消＝golden 把 atoi(原字) 寫回
#         save   取消段 → 三個檔 SHA256 不變；確認段（LimitCount=123、LimitCheckTime=45）→
#                SmartDiagnosticRecord.txt／Reset.txt＝每列每格後面接 ","（CRLF），Para.ini 兩個鍵，edits 重讀＝123／45
#
#  ⚠ C 段會寫 D:\HT9045\system\SmartDiagnosticRecord.txt、SmartDiagnosticRecordReset.txt、SmartDiagnosticPara.ini
#    （restore 範圍）。請在整合者的備份→還原流程裡跑，不要單獨對真檔跑。
#  ⚠ 還有一個副作用要知道：SmartDiagnosticRecord.txt 存在之後，同一個 wb_serve 行程裡每一次氣缸動作
#    （mycylin.cpp → GetCyliderOn/OffCount，CosFunction.bCylinderOnOffTimeLog 開著時）都會照 golden 重寫這個檔。
#  ⚠ 登入的帳號要有 [1] Config 與 [24] Start Condition 的權限；不夠時 golden 會跳 WAR1676，而這支探針開的是
#    單獨一頁（沒有 background 的對話框宿主），伺服器會停在那個警報等人回答。
#
#  用法（wb_serve 剛啟動、這一頁還沒被開過；否則加 --not-fresh）：
#      python tools\webprobe\data_smartdiag_probe.py --port 8046 --user S12TEST --password S12PW [--no-write] [--not-fresh]
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
from s12c_config_probe import wait_js                           # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
SYS = r'D:\HT9045\system'
F_REC = os.path.join(SYS, 'SmartDiagnosticRecord.txt')
F_RST = os.path.join(SYS, 'SmartDiagnosticRecordReset.txt')
F_PARA = os.path.join(SYS, 'SmartDiagnosticPara.ini')
F_PRE = os.path.join(SYS, 'CylinderPreAlm.ini')

CYL_HDR = ['Cylinder Name', 'Sensor QTY', 'On Time(sec)', 'Upper limit(On)', 'Lower limit(On)', 'Off Time(sec)',
           'Upper limit(Off)', 'Lower limit(Off)', 'Waring %']                                         # golden :301-309
CYL_NAMES = ['LoaderEdgePush', 'LoaderTrayY_Fixer', 'Empty_Fix', 'Color_Fix', 'Auto1EdgePush', 'Auto1Side_Fixer',
             'Auto2EdgePush', 'Auto2Side_Fixer', 'Auto3EdgePush', 'Auto3Side_Fixer']                    # golden :335-344
CYL_QTY = ['1', '2', '2', '2', '1', '2', '1', '2', '1', '2']                                            # golden :322-331
SUM_HDR = ['Item', 'Cylider_Name', 'ON_Count', 'OFF_Count', 'StartTime', 'EndTime']                     # golden :94-99
MNG_HDR = ['CyliderName', 'ResetCount']                                                                 # golden :103-104
C_HDR, C_DATA = '#517b91', '#ccd9df'                                                                    # 0x00917B51 / 0x00DFD9CC
MAX_CYL = 321                                                                                           # mycylin.h:42 MaxCylinderItem = the C++ loop bound (golden_reset_row = forms/fSmartDiagnostic.cpp reset loop)   AI(W906-F9050-BD) 20261004: was 295
ROWS_SHOWN = 295                                                                                        # W906_CylinderRowsShown() (mycylin.cpp): Create's rows -- 295 unless wb_serve runs as Type_HT9050 (then MaxCylinderItem: --type9050)
ROWS = ROWS_SHOWN                                                                                       # set by main() from --type9050


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def sha(p):
    return hashlib.sha256(open(p, 'rb').read()).hexdigest() if os.path.exists(p) else None


def split_golden(line):
    """golden SmartDiagnostic.cpp:386-431 SplitStrByDotSpaceOnly 的連續呼叫：分隔字元 , \\t \\r :，前導分隔略過。"""
    return [t for t in re.split(r'[,\t\r:]+', line) if t != '']


def grid_from_file(path, cols):
    """golden LoadSDSummaryData／LoadSDResetSummaryData：RowCount＝行數（VCL 下限 1），PasteStringGridAsTabFormat。"""
    lines = open(path, 'rb').read().decode('cp950', errors='replace').splitlines()
    rows = max(1, len(lines))
    g = [[''] * cols for _ in range(rows)]
    for y, ln in enumerate(lines[:rows]):
        for x, tok in enumerate(split_golden(ln)[:cols]):
            g[y][x] = tok[:255]
    return g


def ini_int(path, sec, key):
    if not os.path.exists(path):
        return 0
    cur = None
    for ln in open(path, 'rb').read().decode('cp950', errors='replace').splitlines():
        s = ln.strip()
        if s.startswith('[') and s.endswith(']'):
            cur = s[1:-1].strip().lower()
        elif cur == sec.lower() and '=' in s:
            k, v = s.split('=', 1)
            if k.strip().lower() == key.lower():
                try:
                    return int(v.strip())
                except ValueError:
                    return 0
    return 0


def op(cdp, expr, timeout=60):
    return json.loads(cdp.eval("(%s).then(function(r){return JSON.stringify(r);})" % expr, timeout=timeout))


def state(cdp):
    return json.loads(cdp.eval("JSON.stringify(HT9045SmartDiag.state())"))


def dom(cdp):
    js = ("(function(){function t(id){var e=document.querySelector('#'+id+' table');if(!e)return null;"
          "return Array.prototype.map.call(e.rows,function(r){return Array.prototype.map.call(r.cells,function(c){return c.textContent;});});}"
          "var cb=document.getElementById('cob_SmartDiagnostic_CyliderName');"
          "return JSON.stringify({summary:t('sg_SmartDiagnostic_Summary'),management:t('sg_SmartDiagnostic_CyliderManagement'),"
          "cylinder:t('sgCylinder'),combo:cb?Array.prototype.map.call(cb.options,function(o){return o.textContent;}):null,"
          "comboIndex:cb?cb.selectedIndex:null,"
          "ed1:(document.getElementById('ed_SmartDiagnostic_LimitCountValue')||{}).value,"
          "ed2:(document.getElementById('ed_SmartDiagnostic_LimitCheckTime')||{}).value,"
          "status:(document.getElementById('sdStatus')||{}).textContent||''});})()")
    return json.loads(cdp.eval(js))


def verify_dom(cdp, s, label):
    d = dom(cdp)
    for k in ('summary', 'management', 'cylinder'):
        check(d[k] == s[k]['cells'], 'D  %s：%s 表格文字＝state（%d×%d）' % (label, k, s[k]['rowCount'], s[k]['colCount']))
    check(d['combo'] == s['combo']['items'] and d['comboIndex'] == s['combo']['itemIndex'],
          'D  %s：下拉 %d 項、選 %r ＝ state' % (label, len(d['combo'] or []), d['comboIndex']))
    check(d['ed1'] == s['edits']['ed_SmartDiagnostic_LimitCountValue'] and d['ed2'] == s['edits']['ed_SmartDiagnostic_LimitCheckTime'],
          'D  %s：參數框 %r／%r ＝ state' % (label, d['ed1'], d['ed2']))


def verify_files(s, label, fresh):
    ex = {f['key']: f for f in s['files']}
    for k, p in (('record', F_REC), ('reset', F_RST), ('para', F_PARA), ('preAlm', F_PRE)):
        check(ex[k]['exists'] == os.path.exists(p), 'F  %s：state.files.%s.exists=%r ＝ 磁碟 %r' % (label, k, ex[k]['exists'], os.path.exists(p)))
    lc, lt = ini_int(F_PARA, 'Setup', 'LimitCount'), ini_int(F_PARA, 'Setup', 'LimitCheckTime')
    check(s['edits']['ed_SmartDiagnostic_LimitCountValue'] == str(lc) and s['edits']['ed_SmartDiagnostic_LimitCheckTime'] == str(lt),
          'F  %s：參數框＝SmartDiagnosticPara.ini [Setup] LimitCount=%d LimitCheckTime=%d（golden LoadSmartDiagnosticParameter :483-491）' % (label, lc, lt))
    if not fresh:
        return
    # 只在「本行程第一次建立表單」時比 Summary／Management 和檔案（之後 golden 不再重讀，記憶體可能已經被操作改過）
    if os.path.exists(F_REC):
        g = grid_from_file(F_REC, 6)
        check(s['summary']['cells'] == g, 'F  %s：Summary＝SmartDiagnosticRecord.txt（%d 列，golden 的分隔規則）' % (label, len(g)))
        check(s['startRecord'] is True and s['timerEnabled'] is True, 'F  %s：紀錄檔存在 → bStartRecord／Timer 開（:520、:24）' % label)
    else:
        check(s['summary']['rowCount'] == 2 and s['summary']['cells'][0] == SUM_HDR,
              'F  %s：沒有紀錄檔 → Summary 2 列（dfm RowCount=2）、表頭 %r' % (label, s['summary']['cells'][0]))
        check(s['startRecord'] is False and s['timerEnabled'] is False, 'F  %s：沒有紀錄檔 → bStartRecord=false、Timer 關（:499、:24）' % label)
        cols = s['summary']['colors']
        check(all(c == C_HDR for c in cols[0]) and all(c == C_DATA for c in cols[1]),
              'F  %s：Summary 顏色＝InitialStringGridColor（第 0 列 %s、其他 %s）：%r' % (label, C_HDR, C_DATA, cols[:2]))
    if os.path.exists(F_RST):
        g = grid_from_file(F_RST, 2)
        check(s['management']['cells'] == g, 'F  %s：Management＝SmartDiagnosticRecordReset.txt（%d 列）' % (label, len(g)))
    else:
        check(s['management']['rowCount'] == 2 and s['management']['cells'][0] == MNG_HDR,
              'F  %s：沒有 Reset 檔 → Management 2 列、表頭 %r' % (label, s['management']['cells'][0]))


def verify_open(s):
    c = s['cylinder']
    check(c['rowCount'] == 11 and c['colCount'] == 9, 'O  sgCylinder 11×9（golden FormShow :291-292）：%d×%d' % (c['rowCount'], c['colCount']))
    check(c['cells'][0] == CYL_HDR, 'O  sgCylinder 表頭＝golden :301-309')
    check([c['cells'][r][0] for r in range(1, 11)] == CYL_NAMES, 'O  sgCylinder 名稱欄＝golden :335-344（ReadCylinderData 之後改回顯示名）')
    check([c['cells'][r][1] for r in range(1, 11)] == CYL_QTY, 'O  sgCylinder Sensor QTY＝golden :322-331')
    if not os.path.exists(F_PRE):
        blank = all(c['cells'][r][k] == '' for r in range(1, 11) for k in (3, 4, 6, 7, 8))
        check(blank, 'O  沒有 CylinderPreAlm.ini → 上下限／Waring 欄空白（golden ReadCylinderData :869-872 直接 return）')
    else:
        print('  INFO  CylinderPreAlm.ini 存在：上下限欄 %r' % [c['cells'][r][3:9] for r in range(1, 4)])
    check(all(x == C_DATA for row in c['colors'] for x in row), 'O  sgCylinder 顏色全是 %s（InitialCylinderSGColor :227-246）' % C_DATA)
    check(bool(re.match(r'^\d{4}年-\d{2}月-\d{2}日-\d{2}時$', s['timeCaption'])), 'O  時間面板＝golden :287 格式：%r' % s['timeCaption'])
    v = s['visible']
    check(v['pn_SmartDiagnostic_Parameter'] == v['pn_SmartDiagnostic_CyliderManagement'] and
          s['activePage'] == (0 if v['pn_SmartDiagnostic_Parameter'] else 1),
          'O  C24 分支一致（:346-356）：Parameter/Management 顯示=%r、ActivePageIndex=%r' % (v['pn_SmartDiagnostic_Parameter'], s['activePage']))


def golden_reset_row(s):
    """golden :702-717：第一個 i 使 Summary.Cells[1][i+1]==combo.Items[i]（i≥Count 時 Items[i] 是 ""，stdctrls.pas:2251-2259）。"""
    items, cells = s['combo']['items'], s['summary']['cells']
    for i in range(MAX_CYL):
        name = cells[i + 1][1] if i + 1 < len(cells) else ''
        item = items[i] if i < len(items) else ''
        if name == item:
            return i + 1
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9341)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--no-write', action='store_true', help='只驗顯示，不按 Create／Reset／Save')
    ap.add_argument('--not-fresh', action='store_true',
                    help='wb_serve 這個行程之前已經開過這一頁（golden 只在建立表單時讀紀錄檔，之後記憶體可能已改）：不拿 Summary 跟檔案比')
    ap.add_argument('--type9050', action='store_true',   # AI(W906-F9050-BD) 20261004
                    help='wb_serve 以 Type_HT9050 執行：Create 的列數＝MaxCylinderItem（321），不是 295（W906_CylinderRowsShown）')
    a = ap.parse_args()
    global ROWS
    ROWS = MAX_CYL if a.type9050 else ROWS_SHOWN

    h0 = {p: sha(p) for p in (F_REC, F_RST, F_PARA, F_PRE)}
    print('  INFO  開始前：' + '、'.join('%s=%s' % (os.path.basename(p), 'missing' if v is None else v[:12]) for p, v in h0.items()))
    if a.user and a.password:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/Data.SmartDiagnostic.html' % a.port})
        ok = wait_js(cdp, "window.HT9045SmartDiag && !HT9045SmartDiag.busy() && HT9045SmartDiag.last()", 60)
        check(bool(ok), '開頁：HT9045SmartDiag 載入、open 有回應')
        if not ok:
            return len(FAILS)
        r0 = json.loads(cdp.eval("JSON.stringify(HT9045SmartDiag.last())"))
        if 'unknown cmd' in (r0.get('detail') or '') or r0.get('guard') == 'unknown-action':
            print('  SKIP  smartdiag.op 的伺服器分派還沒接（WebSmartDiag.h 有 wb_serve 那一臂的範例）')
            st = dom(cdp)['status']
            check('分派' in st, '分派未接時頁面明講：%r' % st)
            check({p: sha(p) for p in h0} == h0, '分派未接時 system 下四個檔一個位元組都沒變')
            return len(FAILS)
        check(r0.get('executed') is True, 'O  open executed（guard=%r detail=%r）' % (r0.get('guard'), r0.get('detail')))
        if not r0.get('executed'):
            return len(FAILS)
        s = state(cdp)
        check(s['created'] is True, 'O  W906_Create 已跑（golden 開機建立表單的 ctor＋OnCreate）')
        verify_open(s)
        # 本行程第一次開窗才能拿 Summary 跟檔案比（golden 只在建立表單時讀一次）；wb_serve 剛啟動時才成立
        verify_files(s, '開窗', not a.not_fresh)
        verify_dom(cdp, s, '開窗')

        if a.no_write:
            print('  SKIP  C：--no-write')
            return len(FAILS)

        # ---- create ----------------------------------------------------------------
        before = state(cdp)
        r = op(cdp, "HT9045SmartDiag.create({answer:false})")
        check(r.get('cancelled') and r.get('asked'), 'C  create 取消段：走到 golden 的確認框（:597）')
        check(state(cdp)['summary'] == before['summary'], 'C  create 取消後 Summary 不變')
        time.sleep(0.45)   # AI(W906-CMDGUARD) 20260926: 同一個 phase-1（confirmed:false）payload 在 400 ms 內重送會被 wb_serve WebCmdGuard 回 busy（S107-3）
        r = op(cdp, "HT9045SmartDiag.create({answer:true})")
        check(r.get('executed') is True, 'C  create 確認段 executed（guard=%r detail=%r）' % (r.get('guard'), r.get('detail')))
        s = state(cdp)
        sc = s['summary']['cells']
        check(s['summary']['rowCount'] == ROWS + 1 and s['management']['rowCount'] == ROWS + 1,
              'C  create：兩張表 RowCount＝W906_CylinderRowsShown()+1＝%d（:603-604）' % (ROWS + 1))
        check(all(sc[r][0] == str(r) for r in range(1, ROWS + 1)), 'C  create：Summary 第 0 欄＝1..%d（:608）' % ROWS)
        named = [r for r in range(1, ROWS + 1) if sc[r][1] != '']
        print('  INFO  create：%d 個有名字的氣缸（Cylinder[i].CylinderName，wb_serve 的 IO 表初始化給的）' % len(named))
        check(len(named) > 0, 'C  create：至少一個氣缸有名字（0 個代表 wb_serve 的 Cylinder[] 名字沒初始化）')
        tfmt = re.compile(r'^\d{4}-\d{2}-\d{2}-\d{2}$')
        bad = [r for r in named if not (sc[r][2] == '0' and sc[r][3] == '0' and tfmt.match(sc[r][4]) and sc[r][4] == sc[r][5])]
        check(not bad, 'C  create：有名字的列 ON/OFF="0"、Start=End=YYYY-MM-DD-HH（:611-615；不符：%s）' % bad[:5])
        check(s['combo']['items'] == before['combo']['items'] + [sc[r][1] for r in named],
              'C  create：下拉＝原本的項目＋依列序的名字（golden 不先 Clear，:618）')
        mc = s['management']['cells']
        check(all(mc[r][0] == sc[r][1] and mc[r][1] == '0' for r in named), 'C  create：Management 同名＋"0"（:619-620）')
        verify_dom(cdp, s, 'create 後')

        # ---- reset -----------------------------------------------------------------
        items = s['combo']['items']
        idx = len(items) - 1
        exp_row = golden_reset_row(s)
        old_cnt = s['management']['cells'][exp_row][1] if exp_row is not None and exp_row < s['management']['rowCount'] else None
        r = op(cdp, "HT9045SmartDiag.reset(%d,{answer:false})" % idx)
        check(r.get('cancelled') and r.get('asked'), 'C  reset 取消段：走到 golden 的確認框（:698）')
        time.sleep(0.45)   # AI(W906-CMDGUARD) 20260926: 同一個 phase-1（confirmed:false）payload 在 400 ms 內重送會被 wb_serve WebCmdGuard 回 busy（S107-3）
        r = op(cdp, "HT9045SmartDiag.reset(%d,{answer:true})" % idx)
        check(r.get('executed') is True, 'C  reset 確認段 executed（guard=%r detail=%r）' % (r.get('guard'), r.get('detail')))
        s2 = state(cdp)
        print('  INFO  reset：選第 %d 項 %r；golden 迴圈算出來會重設第 %r 列 %r（golden 的怪癖：比的是第 i 個下拉項，不是選到的那個）' % (
            idx, items[idx] if items else None, exp_row, s['summary']['cells'][exp_row][1] if exp_row else None))
        if exp_row is not None:
            rr = s2['summary']['cells'][exp_row]
            check(rr[2] == '0' and rr[3] == '0' and tfmt.match(rr[4]) and rr[4] == rr[5], 'C  reset：第 %d 列 ON/OFF 歸零、時間更新（:706-709）' % exp_row)
            try:
                want = str(int(old_cnt or '0') + 1)
            except ValueError:
                want = '1'
            check(s2['management']['cells'][exp_row][1] == want, 'C  reset：Management 第 %d 列 ResetCount %r → %r（:712-714）' % (exp_row, old_cnt, want))
        win = s2['summary']['colors'][idx + 1] if idx + 1 < len(s2['summary']['colors']) else None
        print('  INFO  reset：選到的第 %d 列顏色 %r（clWindow，GetSysColor(COLOR_WINDOW)）' % (idx + 1, win))
        check(win is not None and len(set(win)) == 1 and win[0] not in (C_DATA, C_HDR),
              'C  reset：選到的那一列 6 格重畫成同一個 clWindow 顏色（:719-722）')
        check(s2['combo']['itemIndex'] == -1, 'C  reset：下拉 ItemIndex 回 -1（:723）')

        # ---- cell ------------------------------------------------------------------
        old41 = s2['cylinder']['cells'][1][4]
        r = op(cdp, "HT9045SmartDiag.cell(3,1,2500)")
        s3 = state(cdp)
        check(r.get('executed') is True and s3['cylinder']['cells'][1][3] == '2500', 'C  cell：sgCylinder[3,1]=2500（:854-865）')
        r = op(cdp, "HT9045SmartDiag.cell(4,1)")
        try:
            want = str(int(re.match(r'^\s*(-?\d+)', old41).group(1))) if re.match(r'^\s*-?\d', old41 or '') else '0'
        except Exception:
            want = '0'
        check(state(cdp)['cylinder']['cells'][1][4] == want, 'C  cell 取消：[4,1] %r → atoi＝%r（golden MyInputBox 把 Result 寫回）' % (old41, want))
        r = op(cdp, "HT9045SmartDiag.cell(0,1,5)")
        check(state(cdp)['cylinder']['cells'][1][0] == CYL_NAMES[0], 'C  cell：第 0 欄不可改（golden :857）')

        # ---- save ------------------------------------------------------------------
        h1 = {p: sha(p) for p in (F_REC, F_RST, F_PARA)}
        edits = {'ed_SmartDiagnostic_LimitCountValue': '123', 'ed_SmartDiagnostic_LimitCheckTime': '45'}
        r = op(cdp, "HT9045SmartDiag.save(%s,{answer:false})" % json.dumps(edits))
        check(r.get('cancelled') and r.get('asked'), 'C  save 取消段：走到 golden 的確認框（:174）')
        check({p: sha(p) for p in h1} == h1, 'C  save 取消後三個檔一個位元組都沒變')
        time.sleep(0.45)   # AI(W906-CMDGUARD) 20260926: 同一個 phase-1（confirmed:false）payload 在 400 ms 內重送會被 wb_serve WebCmdGuard 回 busy（S107-3）
        r = op(cdp, "HT9045SmartDiag.save(%s,{answer:true})" % json.dumps(edits))
        check(r.get('executed') is True, 'C  save 確認段 executed（guard=%r detail=%r）' % (r.get('guard'), r.get('detail')))
        s4 = state(cdp)
        want = b''.join((','.join(row) + ',').encode('cp950', errors='replace') + b'\r\n' for row in s4['summary']['cells'])
        got = open(F_REC, 'rb').read() if os.path.exists(F_REC) else None
        check(got == want, 'C  save：SmartDiagnosticRecord.txt＝Summary 每列每格接 ","（CRLF，:555-565）；%d bytes' % len(got or b''))
        want = b''.join((','.join(row) + ',').encode('cp950', errors='replace') + b'\r\n' for row in s4['management']['cells'])
        got = open(F_RST, 'rb').read() if os.path.exists(F_RST) else None
        check(got == want, 'C  save：SmartDiagnosticRecordReset.txt＝Management（:580-590）')
        check(ini_int(F_PARA, 'Setup', 'LimitCount') == 123 and ini_int(F_PARA, 'Setup', 'LimitCheckTime') == 45,
              'C  save：SmartDiagnosticPara.ini [Setup] LimitCount=123 LimitCheckTime=45（SetVCLToParameter :39-54）')
        check(s4['edits']['ed_SmartDiagnostic_LimitCountValue'] == '123' and s4['limitCountValue'] == 123 and s4['limitCheckTime'] == 45,
              'C  save：存完重讀（LoadSmartDiagnosticParameter :476）edits／iLimit*＝123／45')
        check(sha(F_PRE) == h0[F_PRE], 'C  save：CylinderPreAlm.ini 沒被寫（golden 的 SaveCylinderData 沒有入口，dfm 沒綁 OnMouseDown）')
        verify_files(s4, 'save 後', False)
        verify_dom(cdp, s4, 'save 後')

        # ---- 重新開窗：golden 不重讀 Summary（只在建立表單時讀），FormShow 只重讀 CylinderPreAlm.ini ----
        r = op(cdp, "HT9045SmartDiag.open()")
        s5 = state(cdp)
        check(r.get('executed') is True and s5['summary']['cells'] == s4['summary']['cells'],
              'C  重開：Summary 維持記憶體內容（golden FormShow 不呼叫 LoadSDSummaryData）')
        print('  INFO  本行程的 Timer：timerEnabled=%r（golden 只在建立表單時看紀錄檔在不在，:24）' % s5['timerEnabled'])
        print('  INFO  之後 SmartDiagnosticRecord.txt 存在：本行程每次氣缸動作都會照 golden（:656/:689）重寫它')
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)
    print('data_smartdiag_probe: %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
