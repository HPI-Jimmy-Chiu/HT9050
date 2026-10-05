# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 新檔。用 Edge 無頭模式真的載入 Status.Security.html，驗證 179 組權限 radio
# 有沒有依照 levelset.dat 的值被選起來、以及索引有沒有對位。
# 當日完整變更紀錄：<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""security_smoke.py -- Status.Security 的 DOM 驗收（不是看原始碼，是量跑過 JS 的 DOM）。

AI(W906-FW-LEVELSET) 20260916。

為什麼一定要量 DOM：
這一頁的 radio 沒有 id，對照表是引擎在執行期從 title 的 [NN] 掃出來的。
「掃出來的表對不對」沒有辦法用讀原始碼證明 —— 只能把頁面真的跑一次，
再把畫面上選中的那顆，跟 /api/system/levelset 回的第 NN 格逐一對。

四項檢查：
  1. 掃到的 TMySecurity 面板數 == 179（cSecurity.cpp:71-260 的 push_back 筆數）
  2. 每個面板的 [NN] 唯一、落在 0..255
  3. 每組選中的那一顆的序位 == API values[NN]      <- 索引對位的真正證據
  4. 刻意用「錯的索引法」(_<n>) 重算一次，必須對不上
     —— 這一項是探針的自我證明：如果連錯的接法都能通過，前三項就沒有鑑別力。

伺服器要先跑著（唯讀即可）。
用法: py -3 security_smoke.py [--port 8045]
"""
import io
import json
import os
import re
import subprocess
import sys
import urllib.request

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

EDGE = r'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'
PORT = 8045
for _i, _a in enumerate(sys.argv):
    if _a == '--port' and _i + 1 < len(sys.argv):
        PORT = int(sys.argv[_i + 1])
BASE = 'http://127.0.0.1:%d' % PORT
PAGE = 'Status.Security.html'
EXPECT_PANELS = 179

PANEL = re.compile(
    r'<div[^>]*title="MySecurity_Panel_(\d+) : TMySecurity（\[(\d+)\]\s*([^）]*)）"[^>]*>', re.S)
RADIO = re.compile(r'<input[^>]*type="radio"[^>]*>')


def dump(url):
    cmd = [EDGE, '--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check',
           '--user-data-dir=' + os.path.join(os.environ.get('TEMP', '.'), 'wb_edge_profile'),
           '--virtual-time-budget=12000', '--dump-dom', url]
    out = subprocess.run(cmd, capture_output=True, timeout=120)
    return out.stdout.decode('utf-8', 'replace')


def panels_of(dom):
    """切出每個面板的片段。面板是平鋪的兄弟 div，所以用「下一個面板的起點」當終點；
    最後一個用 scrollbox 收尾，否則會把頁面剩下的 radio 都吃進來（第一版就是這樣
    讓最後一組看起來有 10 顆 radio）。"""
    starts = [(m.start(), int(m.group(1)), int(m.group(2)), m.group(3).strip())
              for m in PANEL.finditer(dom)]
    out = []
    for i, (pos, seq, idx, cap) in enumerate(starts):
        end = starts[i + 1][0] if i + 1 < len(starts) else len(dom)
        seg = dom[pos:end]
        # 只取這個面板自己那一個 fieldset 裡的 radio
        fs = re.search(r'<fieldset[^>]*>(.*?)</fieldset>', seg, re.S)
        radios = RADIO.findall(fs.group(1)) if fs else []
        sel = [j for j, r in enumerate(radios) if 'checked' in r]
        out.append({'seq': seq, 'idx': idx, 'cap': cap,
                    'n': len(radios), 'sel': sel[0] if len(sel) == 1 else None})
    return out


def main():
    api = json.load(urllib.request.urlopen(BASE + '/api/system/levelset'))
    values = api['values']
    print('levelset: %d 格，path=%s' % (len(values), api['path']))

    dom = dump(BASE + '/page/' + PAGE)
    print('=== %s  (DOM %d bytes)' % (PAGE, len(dom)))
    ps = panels_of(dom)

    fails = []

    def check(name, ok, detail=''):
        print(('  PASS  ' if ok else '  FAIL  ') + name + (('  -- ' + detail) if detail else ''))
        if not ok:
            fails.append(name)

    # 1
    check('面板數 == %d' % EXPECT_PANELS, len(ps) == EXPECT_PANELS, '掃到 %d' % len(ps))

    # 2
    idxs = [p['idx'] for p in ps]
    check('[NN] 唯一且落在 0..255',
          len(set(idxs)) == len(idxs) and all(0 <= i < 256 for i in idxs),
          'unique=%d min=%d max=%d' % (len(set(idxs)), min(idxs), max(idxs)))

    # 3 -- 正確索引法
    good, bad = 0, []
    for p in ps:
        if p['sel'] is None:
            bad.append('[%d] %s 沒有剛好一顆選中' % (p['idx'], p['cap']))
            continue
        want = values[p['idx']]
        if p['sel'] == want:
            good += 1
        else:
            bad.append('[%d] %s 畫面選第 %d 顆，檔案是 %d' % (p['idx'], p['cap'], p['sel'], want))
    check('每組選中的序位 == values[[NN]]（%d/%d）' % (good, len(ps)),
          good == len(ps) and not bad, ' | '.join(bad[:5]))

    # 4 -- 自我證明：用錯的索引法必須對不上
    wrong = 0
    for p in ps:
        if p['sel'] is None:
            continue
        if p['seq'] != p['idx'] and p['sel'] == values[p['seq']]:
            wrong += 1
    shifted = sum(1 for p in ps if p['seq'] != p['idx'])
    check('錯的索引法 (_<n>/Panel_N) 對不上（有 %d 組兩者不同）' % shifted,
          shifted > 0 and wrong < shifted,
          '若用錯索引會有 %d/%d 組剛好巧合吻合' % (wrong, shifted))

    print('')
    if fails:
        print('FAILED %d: %s' % (len(fails), '; '.join(fails)))
        return 1
    print('ALL PASS')
    return 0


if __name__ == '__main__':
    sys.exit(main())
