# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/status_towerlight_probe.py -- Status.TowerLight.html（golden V912 cTowerLight.cpp TfTowerLight）
#  的 e2e probe。AI(W906-TOWERLIGHT) 20260925（Steven 團隊）。
#
#  用真的瀏覽器（headless Edge ＋ DevTools 協定）開頁面，驗：
#    L  佈局：C++ 回報的 offsetof／sizeof(LAST_GENERAL_SET) ＝ --offsets 檔（MinGW 量的移植樹逐欄 offset）
#    R  讀：頁面拿到的 MessageLight[8][3]／MusicSelect[0..7] ＝ system\lastdata.dat 同位移的值；LED 亮滅＝值
#    C  點燈（golden RGB00Click）：值 0→1→2→0；lastdata.dat／lastdata_backup.dat 前後逐欄比對，只有該格變
#    T  tower.* tag：在 ShowRunLed 目前那一列（runState）改綠燈 → tower.green 跟著變（1＝亮、0＝滅）
#    M  音樂（FormShow→換框→FormClose→WriteLastDataFile）：只有 MusicSelect[i] 變
#    N  重讀：重新開頁，值＝新值
#    G  C++ 端拒絕：led 名稱不合法、index 超出 0～4
#
#  ⚠ 會寫 D:\HT9045\system\lastdata.dat／lastdata_backup.dat（restore 範圍內），以及 WriteLastDataFile 檔尾的
#    config\config.ini 計數鍵。一定要在 restore.sh 包起來的 e2e 裡跑。
#
#  用法：python status_towerlight_probe.py --port 8046 --user S12TEST --password S12PW --offsets <o_p.txt>
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import struct
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
PAGE = 'Status.TowerLight.html'
LD = r'D:\HT9045\system\lastdata.dat'
LDB = r'D:\HT9045\system\lastdata_backup.dat'
COMBOS = ['cbRunning', 'cbJam', 'cbPause', 'cbMessage', 'cbHeating', 'cbHome', 'cbOffLine', 'cbART']


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def load_offsets(path):
    fields = []
    size = None
    for ln in open(path, encoding='utf-8'):
        p = ln.split()
        if p and p[0] == 'SIZEOF':
            size = int(p[1])
        elif len(p) == 3:
            fields.append((int(p[0]), int(p[1]), p[2]))
    return size, fields


def field_of(fields, off):
    hit = [f for f in fields if f[0] <= off < f[0] + f[1]]
    return hit[-1] if hit else None


def diff_fields(fields, a, b):
    """回傳 {欄位名: [變動的位移...]}；位移落在欄位之外記成 (padding)。"""
    # 20260926 S45：舊檔 178896 bytes，第一次寫檔延長到 sizeof=179928。延長出來的尾端＝記憶體裡的
    # iBinBaseRT／bO25_RTBaselined（golden ReadLastDataFile 讀短檔時維持 0），所以短的一邊補 0 再比。
    n = max(len(a), len(b))
    a = a + bytes(n - len(a))
    b = b + bytes(n - len(b))
    out = {}
    for i in range(n):
        if a[i] != b[i]:
            f = field_of(fields, i)
            out.setdefault(f[2] if f else '(padding/tail)', []).append(i)
    return out


def ml_from(buf, off):
    v = struct.unpack_from('<24i', buf, off)
    return [list(v[i * 3:i * 3 + 3]) for i in range(8)]


def ms_from(buf, off):
    return list(struct.unpack_from('<9i', buf, off))


def raw(cdp, value):
    js = ("HT9045Recipe.rawCmd('towerlight.op', {value:%s}).then(function(m){return JSON.stringify({ok:true,res:m});},"
          "function(e){return JSON.stringify({ok:false,err:String(e&&e.message||e)});})") % json.dumps(json.dumps(value))
    return json.loads(cdp.eval(js, timeout=30))


def st(cdp):
    return json.loads(cdp.eval("JSON.stringify(HT9045TowerLight.state())"))


def last(cdp):
    return json.loads(cdp.eval("JSON.stringify(HT9045TowerLight.lastResult())"))


def tag(cdp, name):
    return cdp.eval("window.HT9045Tags ? JSON.stringify(HT9045Tags.get(%s)) : 'null'" % json.dumps(name))


def open_page(cdp, port):
    cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (port, PAGE)})
    time.sleep(0.5)
    return wait_js(cdp, "window.HT9045TowerLight && HT9045TowerLight.loads()>0", 60)


def click_led(cdp, led):
    cdp.eval("document.getElementById(%s).click()" % json.dumps(led))
    time.sleep(0.2)
    wait_js(cdp, "!HT9045TowerLight.busy()", 15)
    return last(cdp)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9341)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--offsets', required=True, help='MinGW 量的 LAST_GENERAL_SET 逐欄 offset（SIZEOF 行＋"off size name"）')
    a = ap.parse_args()

    size, fields = load_offsets(a.offsets)
    fmap = {f[2]: f for f in fields}
    if a.user and a.password:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')
        ok = open_page(cdp, a.port)
        check(bool(ok), '開頁 towerlight.op get 成功（lastError=%r）' % (
            cdp.eval("window.HT9045TowerLight ? HT9045TowerLight.lastError() : 'no wire'")))
        if not ok:
            return len(FAILS)
        s = st(cdp)
        ld = s['lastdata']

        # ---- L 佈局 ---------------------------------------------------------
        check(ld['sizeof'] == size, 'L  sizeof(LAST_GENERAL_SET) C++=%d ＝ offsets 檔 %d' % (ld['sizeof'], size))
        check(ld['offsetMessageLight'] == fmap['MessageLight'][0] and ld['sizeMessageLight'] == fmap['MessageLight'][1],
              'L  MessageLight offset/size C++=%d/%d ＝ %s' % (ld['offsetMessageLight'], ld['sizeMessageLight'], fmap['MessageLight'][:2]))
        check(ld['offsetMusicSelect'] == fmap['MusicSelect'][0] and ld['sizeMusicSelect'] == fmap['MusicSelect'][1],
              'L  MusicSelect offset/size C++=%d/%d ＝ %s' % (ld['offsetMusicSelect'], ld['sizeMusicSelect'], fmap['MusicSelect'][:2]))
        f0 = open(LD, 'rb').read()
        fb0 = open(LDB, 'rb').read()
        check(len(f0) in (size, 178896), 'L  lastdata.dat 檔長 %d（sizeof %d；178896＝S45 之前寫的舊檔，golden 讀短檔只填前段）' % (len(f0), size))

        # ---- R 讀 -----------------------------------------------------------
        mlf = ml_from(f0, ld['offsetMessageLight'])
        msf = ms_from(f0, ld['offsetMusicSelect'])
        print('  lastdata.dat MessageLight=%s MusicSelect=%s' % (mlf, msf))
        print('  C++          MessageLight=%s MusicSelect=%s runState=%s accessLevel=%s iOfflineRun=%s' % (
            s['messageLight'], s['musicSelect'], s['runState'], s['accessLevel'], s['iOfflineRun']))
        check(s['messageLight'] == mlf, 'R  MessageLight[8][3] ＝ lastdata.dat')
        clamp = [min(4, max(0, v)) for v in msf[:8]]
        check(s['musicSelect'] == clamp, 'R  MusicSelect[0..7] ＝ lastdata.dat（FormShow 鉗制 0～4 後：%s）' % clamp)
        dom = json.loads(cdp.eval(
            "(function(){var o={};for(var i=0;i<8;i++)for(var j=0;j<3;j++){var id='RGB'+i+j;o[id]=document.getElementById(id).classList.contains('on');}"
            "['cbRunning','cbJam','cbPause','cbMessage','cbHeating','cbHome','cbOffLine','cbART'].forEach(function(c){o[c]=document.getElementById(c).selectedIndex;});"
            "return JSON.stringify(o);})()"))
        bad = ['RGB%d%d' % (i, j) for i in range(8) for j in range(3)
               if mlf[i][j] in (0, 1) and dom['RGB%d%d' % (i, j)] != (mlf[i][j] == 1)]
        check(not bad, 'R  畫面 LED 亮滅＝值（0 滅／1 亮；2 閃不驗）不符：%s' % bad)
        bad = [c for k, c in enumerate(COMBOS) if dom[c] != clamp[k]]
        check(not bad, 'R  畫面 8 個下拉框＝MusicSelect（不符：%s）' % bad)

        # ---- C 點燈＋T tower.* ---------------------------------------------
        rs = s['runState']
        row = rs if 0 <= rs <= min(7, s['iOfflineRun']) else 2
        led = 'RGB%d0' % row
        print('  點 %s（ShowRunLed runState=%d 那一列的綠燈）；tower.green 目前 %s' % (led, rs, tag(cdp, 'tower.green')))
        prev = open(LD, 'rb').read()
        prevb = open(LDB, 'rb').read()
        v0 = s['messageLight'][row][0]
        r = click_led(cdp, led)
        check(r and r['before'] == v0 and r['after'] == (v0 + 1) % 3 and r['written'],
              'C  %s：%d → %s（golden 0→1→2→0，written=%s）' % (led, v0, r and r['after'], r and r['written']))
        now = open(LD, 'rb').read()
        nowb = open(LDB, 'rb').read()
        d = diff_fields(fields, prev, now)
        print('  lastdata.dat 變動欄位：%s' % {k: len(v) for k, v in d.items()})
        cell_off = ld['offsetMessageLight'] + (row * 3 + 0) * 4
        only_cell = list(d.keys()) == ['MessageLight'] and all(cell_off <= x < cell_off + 4 for x in d['MessageLight'])
        check(only_cell, 'C  lastdata.dat 只有 MessageLight[%d][0]（offset %d..%d）變' % (row, cell_off, cell_off + 3))
        check(len(now) == size and len(nowb) == size, 'C  寫檔後 lastdata.dat／backup 檔長 %d／%d ＝ sizeof %d（舊檔 %d）' % (len(now), len(nowb), size, len(prev)))
        check(now[178896:] == bytes(size - 178896), 'C  延長出來的尾端（iBinBaseRT／bO25_RTBaselined）全 0')
        check(ml_from(now, ld['offsetMessageLight'])[row][0] == r['after'], 'C  檔內 MessageLight[%d][0] ＝ %d' % (row, r['after']))
        db = diff_fields(fields, prevb, nowb)
        check(list(db.keys()) == ['MessageLight'] and nowb == now,
              'C  lastdata_backup.dat 同步（變動 %s；與 lastdata.dat 相同=%s）' % ({k: len(v) for k, v in db.items()}, nowb == now))

        # 轉到 1（亮）看 tower.green=true，再轉到 0（滅）看 false
        def spin_to(target):
            for _ in range(3):
                if st(cdp)['messageLight'][row][0] == target:
                    return True
                click_led(cdp, led)
            return st(cdp)['messageLight'][row][0] == target

        def wait_tag(want, secs=8):
            t0 = time.monotonic()
            while time.monotonic() - t0 < secs:
                if tag(cdp, 'tower.green') == want:
                    return True
                time.sleep(0.3)
            return False
        rs_now = st(cdp)['runState']
        if spin_to(1):
            ok1 = wait_tag('true')
            check(ok1, 'T  MessageLight[%d][0]=1 → tower.green=true（現在 %s，runState=%s）' % (row, tag(cdp, 'tower.green'), rs_now))
        if spin_to(0):
            ok0 = wait_tag('false')
            check(ok0, 'T  MessageLight[%d][0]=0 → tower.green=false（現在 %s）' % (row, tag(cdp, 'tower.green')))
        # 回到原值（restore.sh 之外多一層保險）
        spin_to(v0)
        check(ml_from(open(LD, 'rb').read(), ld['offsetMessageLight'])[row][0] == v0, 'C  轉回原值 %d' % v0)

        # ---- M 音樂 ---------------------------------------------------------
        ci = 1                                                    # cbJam ↔ MusicSelect[1]
        m0 = st(cdp)['musicSelect'][ci]
        want = (m0 + 1) % 5
        prev = open(LD, 'rb').read()
        cdp.eval("(function(){var e=document.getElementById('cbJam');e.selectedIndex=%d;e.dispatchEvent(new Event('change'));})()" % want)
        time.sleep(0.2)
        wait_js(cdp, "!HT9045TowerLight.busy()", 15)
        r = last(cdp)
        check(r and r['before'] == m0 and r['after'] == want and r['written'],
              'M  cbJam：%d → %s（written=%s）' % (m0, r and r['after'], r and r['written']))
        now = open(LD, 'rb').read()
        d = diff_fields(fields, prev, now)
        print('  lastdata.dat 變動欄位：%s' % {k: len(v) for k, v in d.items()})
        m_off = ld['offsetMusicSelect'] + ci * 4
        check(list(d.keys()) == ['MusicSelect'] and all(m_off <= x < m_off + 4 for x in d['MusicSelect']),
              'M  lastdata.dat 只有 MusicSelect[%d]（offset %d..%d）變' % (ci, m_off, m_off + 3))

        # ---- N 重讀 ---------------------------------------------------------
        ok = open_page(cdp, a.port)
        s2 = st(cdp) if ok else {}
        check(bool(ok) and s2.get('musicSelect', [None] * 8)[ci] == want and
              cdp.eval("document.getElementById('cbJam').selectedIndex") == want,
              'N  重新開頁：MusicSelect[%d]=%s、cbJam 顯示 %s（應為 %d）' % (
                  ci, s2.get('musicSelect'), cdp.eval("document.getElementById('cbJam').selectedIndex"), want))
        check(s2.get('messageLight') == ml_from(open(LD, 'rb').read(), ld['offsetMessageLight']),
              'N  重新開頁：MessageLight ＝ lastdata.dat')

        # ---- G C++ 端拒絕 ---------------------------------------------------
        before = open(LD, 'rb').read()
        g1 = raw(cdp, {'op': 'click', 'led': 'RGB99'})
        g2 = raw(cdp, {'op': 'music', 'combo': 'cbJam', 'index': 7})
        g3 = raw(cdp, {'op': 'music', 'combo': 'cbNope', 'index': 1})
        check(not g1['ok'] and 'bad-payload' in g1['err'], 'G  led=RGB99 被拒（%s）' % g1.get('err', '')[:80])
        check(not g2['ok'] and 'bad-payload' in g2['err'], 'G  index=7 被拒（%s）' % g2.get('err', '')[:80])
        check(not g3['ok'] and 'bad-payload' in g3['err'], 'G  combo=cbNope 被拒（%s）' % g3.get('err', '')[:80])
        check(open(LD, 'rb').read() == before, 'G  被拒的請求沒有寫檔')
    finally:
        edge.kill()
    print('FAILS=%d' % len(FAILS))
    for f in FAILS:
        print('  - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
