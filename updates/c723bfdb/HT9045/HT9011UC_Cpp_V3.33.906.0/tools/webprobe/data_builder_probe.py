# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_builder_probe.py -- Data.Builder.html（golden V912 TfBuilder，cBuilder.cpp／cBuilder.dfm）
#  接 C++ builder.op 的 e2e probe。
#
#  Steven 團隊 20260925 (Data.Builder).
#
#  用真的瀏覽器（headless Edge ＋ DevTools）開頁面，驗：
#    O  開窗：三個清單＝自己列 D:\HT9045\IniData\Data 下「不是隱藏」的資料夾，順序＝FindFirstFile 的順序
#       （golden InitCompData :350-394）；labDir＝"Path:"+Directory（ShowDirBoxPath :267-272）；
#       DirectoryListBox1 的子資料夾＝自己列（排除隱藏／系統，照 VCL ReadDirectoryNames）
#    X  change：選來源 → edNewFileName 可用（:35-41）；打 "ab/c:d" → golden OnKeyPress 擋掉 / : → "abcd"（:495-500）；
#       清空 → Create 停用（:43-49）
#    C  寫入（--no-write 跳過）：
#       create  取消段 → 沒建；壞名字（".."、"a\\b"、"a/b"）→ 移植樹守衛／golden 鍵盤濾字擋下、沒建；
#               確認段 → IniData\Data\<新> 與 IniData\Offset\<新> 存在，Data 下每個檔（含子目錄）的相對路徑／SHA256＝來源；
#               清單尾端多一項（golden 只 Add，不重列）；同名再建 → 詢問段帶 nameExists
#       export  瀏覽到暫存資料夾 → 取消段沒寫；確認段 → <暫存>\<新> 的檔＝來源、infoBoxes＝["Export data finish!!"]；
#               瀏覽到 IniData\Data 再匯出 → 移植樹守衛 export-into-recipes（golden 在那裡會先刪掉要匯出的配方）
#       import  暫存資料夾裡放一個子資料夾 → 取消段沒寫；確認段 → IniData\Data 下出現它（CopySourTarget :274-335）
#       delete  使用中的配方 → golden :211-215「File in use, cannot delete」、資料夾還在；
#               新建的 → 取消段還在（golden :223-229 照樣重列清單）；確認段 → Data 與 Offset 兩個都不在了（資源回收筒）
#       close   → fShow=false
#
#  ⚠ C 段會在 D:\HT9045\IniData\Data、\Offset 建立／刪除 W906PRB*、W906IMP* 資料夾（刪除進資源回收筒），
#    請在整合者「先備份整個 IniData 再還原」的流程裡跑。export／import 只用 %TEMP% 下的暫存資料夾，跑完刪掉。
#  ⚠ 登入的帳號要有 [1] Config 與 [23] Config - Builder 的權限；不夠時 golden 會跳 WAR1676，而這支探針開的是
#    單獨一頁（沒有 background 的對話框宿主），伺服器會停在那個警報等人回答。
#
#  用法：
#      python tools\webprobe\data_builder_probe.py --port 8046 --user S12TEST --password S12PW [--no-write]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import hashlib
import json
import os
import shutil
import stat
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
DATA = 'D:\\HT9045\\IniData\\Data'
OFFSET = 'D:\\HT9045\\IniData\\Offset'


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def attrs(p):
    try:
        return os.stat(p).st_file_attributes
    except OSError:
        return 0


def golden_list(root):
    """golden InitCompData：FindFirstFile(<DataPath>*)，跳過隱藏與 . ..，只收資料夾（os.listdir 用同一個 API，順序相同）。"""
    out = []
    for n in os.listdir(root):
        p = os.path.join(root, n)
        if os.path.isdir(p) and not (attrs(p) & stat.FILE_ATTRIBUTE_HIDDEN):
            out.append(n)
    return out


def vcl_children(d):
    """VCL TDirectoryListBox.ReadDirectoryNames：FindFirst(faDirectory) 濾掉隱藏／系統，Sorted（不分大小寫）。"""
    try:
        names = os.listdir(d)
    except OSError:
        return []
    out = [n for n in names if os.path.isdir(os.path.join(d, n)) and
           not (attrs(os.path.join(d, n)) & (stat.FILE_ATTRIBUTE_HIDDEN | stat.FILE_ATTRIBUTE_SYSTEM))]
    return sorted(out, key=lambda s: s.upper())


def tree_hash(root):
    res = {}
    for base, _, files in os.walk(root):
        for f in files:
            p = os.path.join(base, f)
            res[os.path.relpath(p, root).upper()] = hashlib.sha256(open(p, 'rb').read()).hexdigest()
    return res


def op(cdp, expr, timeout=120):
    return json.loads(cdp.eval("(%s).then(function(r){return JSON.stringify(r);})" % expr, timeout=timeout))


def state(cdp):
    return json.loads(cdp.eval("JSON.stringify(HT9045Builder.state())"))


def dom(cdp):
    js = ("(function(){function o(id){var e=document.getElementById(id);return e?Array.prototype.map.call(e.options,function(x){return x.textContent;}):null;}"
          "function d(id){var e=document.getElementById(id);return e?!!e.disabled:null;}"
          "return JSON.stringify({src:o('cbSourceFile'),del:o('cbDeleteFile'),"
          "chk:Array.prototype.map.call(document.querySelectorAll('#CheckListBox1 input[type=checkbox]'),function(x){return x.dataset.name;}),"
          "dirs:Array.prototype.map.call(document.querySelectorAll('#DirectoryListBox1 .bdDir'),function(x){return x.dataset.path;}),"
          "labDir:(document.getElementById('labDir')||{}).textContent,ed:(document.getElementById('edNewFileName')||{}).value,"
          "edDis:d('edNewFileName'),crDis:d('btCreateSetupFile'),dlDis:d('btDeleteSetupFile'),"
          "status:(document.getElementById('bdStatus')||{}).textContent||''});})()")
    return json.loads(cdp.eval(js))


def verify_lists(cdp, s, label, exact=True):
    want = golden_list(s['dataPath'].rstrip('\\'))
    for k in ('cbSourceFile', 'cbDeleteFile', 'CheckListBox1'):
        got = s[k]['items']
        check(got == want if exact else sorted(got) == sorted(want),
              'O  %s：%s %d 項＝IniData\\Data 的資料夾%s（golden InitCompData）' % (label, k, len(got), '（同順序）' if exact else '（集合）'))
    d = dom(cdp)
    check(d['src'] == s['cbSourceFile']['items'] and d['del'] == s['cbDeleteFile']['items'] and d['chk'] == s['CheckListBox1']['items'],
          'D  %s：畫面上三個清單＝state' % label)


def verify_dir(cdp, s, label):
    check(s['labDir'] == 'Path:' + s['directory'], 'O  %s：labDir=%r ＝ "Path:"+Directory（:267-272）' % (label, s['labDir']))
    tree = s['dirTree']
    cur = [n for n in tree if n['kind'] == 'current']
    check(len(cur) == 1 and cur[0]['path'].rstrip('\\').upper() == s['directory'].rstrip('\\').upper(),
          'O  %s：DirectoryListBox1 反白列＝Directory %r' % (label, s['directory']))
    kids = [n['name'] for n in tree if n['kind'] == 'closed']
    check(kids == vcl_children(s['directory']), 'O  %s：子資料夾 %d 個＝自己列（VCL ReadDirectoryNames，濾隱藏／系統、排序）' % (label, len(kids)))
    d = dom(cdp)
    check(d['labDir'] == s['labDir'] and d['dirs'] == [n['path'] for n in tree], 'D  %s：labDir 與資料夾樹＝state' % label)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9342)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--no-write', action='store_true', help='只驗顯示與 change，不建／刪／匯入／匯出')
    a = ap.parse_args()

    before_data = golden_list(DATA)
    if a.user and a.password:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    edge, prof, ws = launch_edge(a.dbg)
    tmps = []
    made = []
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/Data.Builder.html' % a.port})
        ok = wait_js(cdp, "window.HT9045Builder && !HT9045Builder.busy() && HT9045Builder.last()", 60)
        check(bool(ok), '開頁：HT9045Builder 載入、open 有回應')
        if not ok:
            return len(FAILS)
        r0 = json.loads(cdp.eval("JSON.stringify(HT9045Builder.last())"))
        if 'unknown cmd' in (r0.get('detail') or '') or r0.get('guard') == 'unknown-action':
            print('  SKIP  builder.op 的伺服器分派還沒接（WebBuilder.h 有 wb_serve 那一臂的範例）')
            st = dom(cdp)['status']
            check('分派' in st, '分派未接時頁面明講：%r' % st)
            check(golden_list(DATA) == before_data, '分派未接時 IniData\\Data 沒有任何變化')
            return len(FAILS)
        check(r0.get('executed') is True, 'O  open executed（guard=%r detail=%r）' % (r0.get('guard'), r0.get('detail')))
        if not r0.get('executed'):
            return len(FAILS)
        s = state(cdp)
        check(s['fShow'] is True, 'O  FormShow 跑過（fShow=true，:32）')
        check(s['dataPath'].rstrip('\\').upper() == DATA.upper() and s['offsetPath'].rstrip('\\').upper() == OFFSET.upper(),
              'O  DataPath／OffsetPath＝golden common.cpp:33-34（%r／%r）' % (s['dataPath'], s['offsetPath']))
        verify_lists(cdp, s, '開窗')
        verify_dir(cdp, s, '開窗')
        letters = [d['letter'] for d in s['drives']]
        check('c' in letters and 'd' in letters, 'O  DriveComboBox1 有 c: d:（TDriveComboBox.BuildList）：%r' % letters)
        cur = s['currentRecipe']
        print('  INFO  使用中的配方（fMain->cbSetupFileName->Text＝GetLastOpenFN）：%r；配方 %d 個' % (cur, len(s['cbSourceFile']['items'])))

        # ---- X：change -----------------------------------------------------------
        items = s['cbSourceFile']['items']
        src = cur if cur in items else (items[0] if items else '')
        r = op(cdp, "HT9045Builder.change('cbSourceFile',%s)" % json.dumps(src))
        s = state(cdp)
        check(r.get('executed') and s['cbSourceFile']['text'] == src and s['edNewFileName']['enabled'] is True,
              'X  選來源 %r → edNewFileName 可用（cbSourceFileChange :35-41）' % src)
        r = op(cdp, "HT9045Builder.change('edNewFileName','ab/c:d')")
        s = state(cdp)
        check(r.get('keyFiltered') is True and s['edNewFileName']['text'] == 'abcd' and s['btCreateSetupFile']['enabled'] is True,
              'X  打 "ab/c:d" → golden OnKeyPress 擋掉 / : → %r，Create 可用（:43-49）' % s['edNewFileName']['text'])
        check(dom(cdp)['ed'] == 'abcd', 'D  輸入框改成濾過的字 %r' % dom(cdp)['ed'])
        r = op(cdp, "HT9045Builder.change('edNewFileName','')")
        s = state(cdp)
        check(s['btCreateSetupFile']['enabled'] is False and dom(cdp)['crDis'] is True, 'X  清空 → Create 停用（畫面也停用）')
        r = op(cdp, "HT9045Builder.change('cbSourceFile','__no_such_recipe__')")
        check(r.get('executed') is False and r.get('guard') == 'bad-payload', 'X  選清單外的來源 → 拒絕（csDropDownList）')

        if a.no_write:
            print('  SKIP  C：--no-write')
            return len(FAILS)
        if not src:
            print('  SKIP  C：IniData\\Data 下沒有任何配方可當來源')
            return len(FAILS)

        ts = time.strftime('%H%M%S')
        name = 'W906PRB' + ts
        nd, no = os.path.join(DATA, name), os.path.join(OFFSET, name)

        # ---- create ----------------------------------------------------------------
        r = op(cdp, "HT9045Builder.create(%s,%s,{answer:false})" % (json.dumps(src), json.dumps(name)))
        check(r.get('cancelled') and r.get('asked') and 'Do you create' in ' '.join((r.get('phase1') or {}).get('prompt') or []),
              'C  create 取消段：golden 的題目（:94-95）%r' % (r.get('phase1') or {}).get('prompt'))
        check(not os.path.exists(nd) and not os.path.exists(no), 'C  create 取消後沒有建資料夾')
        for bad in ('..', 'a\\b', 'a/b', 'CON'):
            r = op(cdp, "HT9045Builder.create(%s,%s,{answer:true})" % (json.dumps(src), json.dumps(bad)))
            check(r.get('executed') is not True and r.get('guard') in ('bad-name', 'button-disabled'),
                  'C  create 壞名字 %r → 擋下（guard=%r portGuard=%r）' % (bad, r.get('guard'), r.get('portGuard')))
        check(golden_list(DATA) == before_data, 'C  壞名字之後 IniData\\Data 沒變')
        r = op(cdp, "HT9045Builder.create(%s,%s,{answer:true})" % (json.dumps(src), json.dumps(name)))
        check(r.get('executed') is True, 'C  create 確認段 executed（guard=%r detail=%r）' % (r.get('guard'), r.get('detail')))
        made.append(name)
        check(os.path.isdir(nd) and os.path.isdir(no), 'C  create：Data\\%s 與 Offset\\%s 都在（MyForceDirectories :106）' % (name, name))
        ops = r.get('shellOps') or []
        print('  INFO  create shellOps：%s' % [(o['op'], o['from'], o['to'], o['ret']) for o in ops])
        check(len(ops) == 2 and all(o['op'] == 'copy' for o in ops) and ops[0]['ret'] == 0,
              'C  create：兩次 FO_COPY（Data、Offset），Data 那次 ret=0')
        check(tree_hash(os.path.join(DATA, src)) == tree_hash(nd), 'C  create：Data\\%s 的每個檔（含子目錄）＝來源 %s' % (name, src))
        s = state(cdp)
        check(s['cbSourceFile']['items'][-1] == name and s['cbDeleteFile']['items'][-1] == name and s['CheckListBox1']['items'][-1] == name,
              'C  create：三個清單尾端多了 %s（golden :148-150 只 Add）' % name)
        check(s['edNewFileName']['text'] == '' and s['btCreateSetupFile']['enabled'] is False and s['asBackupCreate'] == name,
              'C  create：edNewFileName 清空 → Create 停用（TEdit OnChange）、asBackupCreate=%r（:151-152）' % s['asBackupCreate'])
        r = op(cdp, "HT9045Builder.create(%s,%s,{answer:false})" % (json.dumps(src), json.dumps(name)))
        check((r.get('phase1') or {}).get('nameExists') is True, 'C  同名再建：詢問段帶 nameExists（golden 不擋，會複製進去）；已取消')

        # ---- export ----------------------------------------------------------------
        tmp = tempfile.mkdtemp(prefix='w906_bexp_')
        tmps.append(tmp)
        r = op(cdp, "HT9045Builder.dir(%s)" % json.dumps(tmp))
        s = state(cdp)
        check(r.get('executed') and s['directory'].upper() == os.path.normpath(tmp).upper(), 'C  dir → %r' % s['directory'])
        verify_dir(cdp, s, 'dir 暫存')
        r = op(cdp, "HT9045Builder.dir(%s)" % json.dumps(tmp + '\\__nope__'))
        check(r.get('executed') is False and r.get('guard') == 'dir-missing', 'C  dir 到不存在的資料夾 → 擋下')
        r = op(cdp, "HT9045Builder.exportTo([%s],{answer:false})" % json.dumps(name))
        check(r.get('cancelled') and not os.path.exists(os.path.join(tmp, name)), 'C  export 取消段：沒寫')
        time.sleep(0.45)   # AI(W906-CMDGUARD) 20260926: 同一個 phase-1（confirmed:false）payload 在 400 ms 內重送會被 wb_serve WebCmdGuard 回 busy（S107-3）
        r = op(cdp, "HT9045Builder.exportTo([%s],{answer:true})" % json.dumps(name))
        check(r.get('executed') is True and r.get('infoBoxes') == ['Export data finish!!'],
              'C  export 確認段 executed、golden 的完成訊息（:472）%r' % r.get('infoBoxes'))
        check(tree_hash(os.path.join(tmp, name)) == tree_hash(nd), 'C  export：%s\\%s 的檔＝Data\\%s' % (tmp, name, name))
        r = op(cdp, "HT9045Builder.dir(%s)" % json.dumps(DATA))
        r = op(cdp, "HT9045Builder.exportTo([%s],{answer:true})" % json.dumps(name))
        check(r.get('executed') is not True and r.get('guard') == 'export-into-recipes' and os.path.isdir(nd),
              'C  export 到 IniData\\Data → 移植樹守衛擋下（guard=%r），Data\\%s 還在' % (r.get('guard'), name))

        # ---- import ----------------------------------------------------------------
        tmp2 = tempfile.mkdtemp(prefix='w906_bimp_')
        tmps.append(tmp2)
        imp = 'W906IMP' + ts
        os.makedirs(os.path.join(tmp2, imp))
        open(os.path.join(tmp2, imp, 'probe.txt'), 'w').write('w906 builder import probe ' + ts)
        op(cdp, "HT9045Builder.dir(%s)" % json.dumps(tmp2))
        r = op(cdp, "HT9045Builder.importDir({answer:false})")
        check(r.get('cancelled') and not os.path.exists(os.path.join(DATA, imp)), 'C  import 取消段：沒寫')
        time.sleep(0.45)   # AI(W906-CMDGUARD) 20260926: 同一個 phase-1（confirmed:false）payload 在 400 ms 內重送會被 wb_serve WebCmdGuard 回 busy（S107-3）
        r = op(cdp, "HT9045Builder.importDir({answer:true})")
        check(r.get('executed') is True, 'C  import 確認段 executed（guard=%r detail=%r）' % (r.get('guard'), r.get('detail')))
        single = os.path.exists(os.path.join(DATA, os.path.basename(tmp2)))
        landed = os.path.join(DATA, os.path.basename(tmp2), imp) if single else os.path.join(DATA, imp)
        made.append(os.path.basename(tmp2) if single else imp)
        print('  INFO  import：CosFunction.bBuilderImportSingleFolder=%r（由落點判斷）→ %s' % (single, landed))
        check(os.path.isfile(os.path.join(landed, 'probe.txt')), 'C  import：%s\\probe.txt 在（CopySourTarget :274-335）' % landed)
        s = state(cdp)
        check(made[-1] in s['cbSourceFile']['items'], 'C  import 後清單重列（InitCompData :334）含 %s' % made[-1])

        # ---- delete ----------------------------------------------------------------
        if cur in s['cbDeleteFile']['items']:
            r = op(cdp, "HT9045Builder.del(%s,{answer:true})" % json.dumps(cur))
            msgs = ' '.join(m['s1'] for m in (r.get('messages') or []))
            check(r.get('executed') is not True and 'File in use' in msgs and os.path.isdir(os.path.join(DATA, cur)),
                  'C  刪使用中的配方 %r → golden :211-215 擋下（%r），資料夾還在' % (cur, msgs))
        else:
            print('  INFO  使用中的配方 %r 不在清單裡，golden 的使用中守衛驗不到' % cur)
        r = op(cdp, "HT9045Builder.del(%s,{answer:false})" % json.dumps(name))
        check(r.get('cancelled') and os.path.isdir(nd), 'C  delete 取消段：Data\\%s 還在' % name)
        s = state(cdp)
        check(s['cbDeleteFile']['text'] == '', 'C  delete 取消後 golden :228-229 照樣重列、清掉選擇')
        time.sleep(0.45)   # AI(W906-CMDGUARD) 20260926: 同一個 phase-1（confirmed:false）payload 在 400 ms 內重送會被 wb_serve WebCmdGuard 回 busy（S107-3）
        r = op(cdp, "HT9045Builder.del(%s,{answer:true})" % json.dumps(name))
        check(r.get('executed') is True, 'C  delete 確認段 executed（guard=%r detail=%r）' % (r.get('guard'), r.get('detail')))
        check(not os.path.exists(nd) and not os.path.exists(no), 'C  delete：Data\\%s 與 Offset\\%s 都不在了' % (name, name))
        dops = [o for o in (r.get('shellOps') or []) if o['op'] == 'delete']
        check(len(dops) == 2 and all(o['ret'] == 0 for o in dops) and all(o['flags'] & 0x40 for o in dops),
              'C  delete：兩次 FO_DELETE、ret=0、FOF_ALLOWUNDO（資源回收筒）')
        if not os.path.exists(nd):
            made.remove(name)
        s = state(cdp)
        verify_lists(cdp, s, 'delete 後')
        # 收尾：import 進來的那個也刪掉（同一條 golden 路）
        for n in list(made):
            if n in s['cbDeleteFile']['items'] and n != cur:
                r = op(cdp, "HT9045Builder.del(%s,{answer:true})" % json.dumps(n))
                if r.get('executed') and not os.path.exists(os.path.join(DATA, n)):
                    made.remove(n)
        check(not made, 'C  收尾：探針建的資料夾都刪掉了（剩：%r）' % made)

        r = json.loads(cdp.eval("HT9045Recipe.rawCmd('control.takeover').catch(function(){}).then(function(){return HT9045Recipe.rawCmd('builder.op',{value:JSON.stringify({act:'close'})});})"  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
                                ".then(function(m){return JSON.stringify(m&&typeof m.value==='string'?JSON.parse(m.value):m);},function(e){return JSON.stringify({error:String(e)});})"))
        check((r.get('state') or {}).get('fShow') is False, 'C  close → fShow=false（FormClose :502-505）')
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)
        for t in tmps:
            shutil.rmtree(t, ignore_errors=True)
    print('data_builder_probe: %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
