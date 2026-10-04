# -*- coding: utf-8 -*-
"""tools/webprobe/c12_census_md.py -- AI(W906-ST02-C12) 20261003 (St02-E): writes the ST02-C12 census report (Markdown) from the
rows of c12_button_census.py.  Called by c12_button_census.py --out-md; no other use."""
import collections, subprocess

WHO = {
    'A': '網頁那一側（Steven 這邊，St01／St02 認領）：C++ 已有處理器，網頁沒送',
    'B': 'C++ 沒翻（照 golden 補；筆電或 St02 先在 §1 認領）',
    'B(web-sends-unknown-cmd)': '網頁送了 C++ 不認得的命令（對一下命令名；誰的頁誰改）',
    'C': '照 golden 藏（golden 在這台本來就看不到）——網頁那一側',
    'C0': '不用做：golden 這顆本來就沒有 OnClick／處理器是空的',
    'D': '列給 Jimmy：處理器本體會動馬達／寫輸出／啟動，不派',
    'E': '機台端正在做（WORKLOG_MACHINE §4 提到）——先問機台，避免重工',
}
ORDER = ['dead/D', 'dead/B', 'dead/A', 'dead/C', 'dead/C0', 'dead/?', 'B(web-sends-unknown-cmd)', 'OK-cmd', 'OK-http', 'OK-ui',
         's:unbound/D', 's:unbound/B', 's:unbound/A', 's:unbound/C', 's:unbound/C0', 's:unbound/?', 's:B(web-sends-unknown-cmd)',
         's:referenced', 's:bound-data', 's:bound-ui', 's:bound-cmd', 's:C', 's:C0', 'greyed(runtime-list)', 'greyed(static)',
         'greyed(probe)', 'hidden(static)', 'hidden(probe)', 'missing(probe)', 'dev-page']


def short(c):
    return c.replace('+E?', '').split('/')[-1].replace('s:', '')


def who(cat):
    b = short(cat)
    out = WHO.get(b, '')
    if '+E?' in cat:
        out = (out + '；' if out else '') + WHO['E']
    return out


def head_commit(repo):
    try:
        return subprocess.run(['git', '-C', repo, 'log', '-1', '--format=%h %ci'], capture_output=True, text=True).stdout.strip()
    except Exception:
        return '?'


def esc(s):
    return str(s).replace('|', '／').replace('\n', ' ')


def write_md(rows, path, golden):
    import c12_button_census as C
    probed = any(r.get('probe') for r in rows)
    vis = [r for r in rows if r['cat'] not in ('dev-page', 'hidden(static)', 'hidden(probe)', 'missing(probe)')
           and not r['cat'].startswith('greyed')]
    cnt = collections.Counter(r['cat'] for r in rows)
    L = []
    L.append('# ST02-C12 按鈕普查：「看得到、按了沒反應」（%s）' % ('靜態＋點擊實測' if probed else '靜態一半；點擊實測欄 UNVERIFIED'))
    L.append('')
    L.append('> **golden 基準：`%s`**（RULINGS_20261003 第 2 條：0618 為準，906_0625_Steven 只對照；對照結果見文末）。' % golden)
    L.append('> 量測樹：main `%s`。只量、只分類，**沒有改任何程式**（TO_STEVEN.md §3 ST02-C12）。' % head_commit(C.REPO))
    L.append('> 產生：`HT9011UC_Cpp_V3.33.906.0/tools/webprobe/c12_button_census.py`（靜態）＋ `c12_click_probe.py`（無頭 Edge＋假伺服器，'
             '**%s**）。每顆的完整欄位在同名 `.tsv`。' % ('已合併實測結果' if probed else '還沒跑：請 St01 代跑，見 §5'))
    L.append('')
    L.append('## 0. 白話摘要')
    L.append('')
    L.append('- 範圍：外框 `web/background.html` WINDOWS 表開得到的頁＋Alert 覆蓋頁，共 %d 頁、%d 顆按鈕（golden 的 TButton／TBitBtn／TSpeedButton，'
             '加上網頁自己的 `<button>`）。' % (len(set(r['page'] for r in rows)), len(rows)))
    L.append('- 看得到又沒有變灰的：**%d 顆**（其餘：靜態藏起來 %d、變灰 %d、開發用頁 %d）。' % (
        len(vis), cnt['hidden(static)'] + cnt['hidden(probe)'], sum(v for k, v in cnt.items() if k.startswith('greyed')), cnt['dev-page']))
    if probed:
        dead = [r for r in vis if r['cat'].startswith('dead/')]
        L.append('- 實測按了**什麼都沒發生**（沒送 WS、沒有 POST、沒有視窗動作、畫面沒變）：**%d 顆**，依 golden／移植樹分成 A～E（§1）。' % len(dead))
    else:
        L.append('- **靜態只能確定兩件事**：①golden 那一側（有沒有處理器、處理器在哪一行、會不會動機台、golden 在這台藏不藏）；'
                 '②移植樹 C++ 有沒有同名的處理器／form.event 表列。「按下去送了什麼」靜態只能找到「這顆 id 被哪支 script 用到」，'
                 '命令多半是引擎依資料表組出來的，所以每一顆的分類前面都加 `s:`（暫定），要等 §5 的點擊實測合併才算數。')
        sun = [r for r in vis if r['cat'].startswith('s:unbound/')]
        L.append('- 靜態就看得出「**網頁沒有任何一支 script 提到這顆 id**」的：**%d 顆**（最可能是真的沒反應，§1 的 s:unbound 列）。' % len(sun))
    L.append('')
    L.append('## 1. 分類與建議誰做')
    L.append('')
    L.append('| 分類 | 意思 | 顆數 | 建議誰做 |')
    L.append('|---|---|---:|---|')
    MEAN = {
        'dead/': '實測沒反應，',
        's:unbound/': '靜態：沒有 script 提到這顆 id，',
        's:referenced': '靜態：id 有被 script 用到，按了做什麼要實測',
        's:bound-data': '靜態：id 在資料表裡（例 teach-access），命令由引擎組，要實測',
        's:bound-ui': '靜態：附近只有視窗動作（openWin／close／postMessage），應該是純畫面動作',
        's:bound-cmd': '靜態：附近找得到命令字串、C++ 認得（實測確認）',
        'OK-cmd': '實測送了 C++ 認得的命令',
        'OK-http': '實測送了 POST／PUT',
        'OK-ui': '實測只有畫面／視窗動作（沒送 C++）',
        'greyed(runtime-list)': '網頁執行時變灰並寫原因（例 TEACH_UNWIRED_B38）',
        'greyed(static)': '網頁 disabled',
        'greyed(probe)': '實測變灰',
        'hidden(static)': '網頁 display:none／visibility:hidden（不算「看得到」）',
        'hidden(probe)': '實測看不到（含 C++／FormShow 藏的）',
        'missing(probe)': '實測頁面上沒有這個 id',
        'dev-page': 'IDE.*／ScreenShots 開發用頁，不算',
    }
    TAIL = {'A': 'C++ 有處理器（form.event 表列或同名函式）', 'B': 'C++ 沒有處理器', 'C': 'golden dfm Visible=False、程式也不打開',
            'C0': 'golden 沒有 OnClick 或處理器是空的', 'D': 'golden 處理器本體會動馬達／寫輸出', '?': '不是 golden 元件、也沒有處理器線索'}
    for k in sorted(cnt, key=lambda x: (ORDER.index(x.replace('+E?', '')) if x.replace('+E?', '') in ORDER else 99, x)):
        m = ''
        for pre, txt in MEAN.items():
            if k.replace('+E?', '').startswith(pre) and (pre.endswith('/') or k.replace('+E?', '') == pre):
                m = txt
                if pre.endswith('/'):
                    m += TAIL.get(short(k), '')
                break
        if not m:
            m = TAIL.get(short(k), '')
        if '+E?' in k:
            m += '；WORKLOG_MACHINE §4 有提到（id 或頁）'
        L.append('| `%s` | %s | %d | %s |' % (k, m, cnt[k], who(k) if (k.startswith('dead/') or k.startswith('s:unbound/') or 'unknown' in k) else ''))
    L.append('')
    L.append('分類規則（`c12_button_census.py` classify／dead_cat）：C0 > C > D > A > B；D 只看處理器**本體**（呼叫下去的函式不追），'
             '讀 `MOT[i]` 位置不算 D，`MOT[i].Gali_MotMove(...)`、`ServoOnOff(...)`、`SW[..].On()`、`SetOutput*`、`ADAM_Write*`、`fMain->Start()` 算。'
             'E 只是「§4 文字裡出現這個 id 或頁名」，要人看。')
    L.append('')
    L.append('## 2. 每頁一列')
    L.append('')
    L.append('| 頁 | golden dfm | 全部 | 藏 | 灰 | 看得到 | 主要分類（顆數） |')
    L.append('|---|---|---:|---:|---:|---:|---|')
    byp = collections.OrderedDict()
    for r in rows:
        byp.setdefault(r['page'], []).append(r)
    for p, rs in byp.items():
        c2 = collections.Counter(r['cat'] for r in rs)
        h = sum(v for k, v in c2.items() if k.startswith('hidden') or k == 'missing(probe)')
        g = sum(v for k, v in c2.items() if k.startswith('greyed'))
        v = len([r for r in rs if r in vis])
        dfm = next((r['g_dfm'].split(':')[0] for r in rs if r['g_dfm']), '')
        top = ', '.join('%s %d' % (k, n) for k, n in c2.most_common() if not k.startswith(('hidden', 'greyed', 'dev')))[:160]
        L.append('| %s | %s | %d | %d | %d | %d | %s |' % (p, dfm or '—', len(rs), h, g, v, esc(top)))
    L.append('')
    L.append('## 3. 每顆一列（看得到又沒有變灰的 %d 顆；藏起來／變灰的只在 .tsv）' % len(vis))
    L.append('')
    L.append('欄位：頁、id、caption、分類、送出的命令（%s）、C++ 認不認得、golden 處理器（golden 906 檔名:行）、會動機台、建議誰做。' % (
        '實測' if probed else '靜態猜測，UNVERIFIED'))
    L.append('')
    L.append('| 頁 | id | caption | 分類 | 送出（%s） | C++ | golden 處理器 | 動機台 | 建議誰做 |' % ('實測' if probed else '靜態'))
    L.append('|---|---|---|---|---|---|---|---|---|')
    key = lambda r: (ORDER.index(r['cat'].replace('+E?', '')) if r['cat'].replace('+E?', '') in ORDER else 99, r['page'], r['id'])
    noid = collections.OrderedDict()
    for r in vis:                                           # web-only buttons without an id (e.g. Security's 180 key buttons): one line per page
        if r['id'] == '(no id)':
            noid.setdefault(r['page'], []).append(r)
    for p, rs in noid.items():
        L.append('| %s | （沒有 id ×%d） | %s … | `%s` | 探針點不到（沒有 id） | — | 網頁自己的按鈕，不是 golden 元件 | | 要人看 |' % (
            p, len(rs), esc(rs[0]['caption'][:20]), rs[0]['cat']))
    for r in sorted((r for r in vis if r['id'] != '(no id)'), key=key):
        h = r['g_handler'] or {}
        sent = r.get('sent_probe') if r.get('probe') else (' '.join(r['cmds'] + ['ui:' + u for u in r['ui']]) or ('refs %d' % len(r['refs']) if r['refs'] else '—'))
        cpp = ' '.join(r['cpp']) or ('form.event 表列' if r['fe_row'] else '') or (' '.join(r['port_handler'][:2]) if r['port_handler'] else '—')
        gh = ('%s %s (%s 句)' % (r['g_onclick'], h.get('loc', ''), h.get('stmts', ''))) if r['g_onclick'] else ('沒有 OnClick' if r['g_dfm'] else '—')
        L.append('| %s | %s | %s | `%s` | %s | %s | %s | %s | %s |' % (
            r['page'], esc(r['id']), esc(r['caption'][:24]), r['cat'], esc(sent)[:90], esc(cpp)[:60], esc(gh)[:90],
            esc(' '.join(h.get('danger', []))) if h else '', who(r['cat']) if (r['cat'].startswith(('dead/', 's:unbound/')) or 'unknown' in r['cat']) else ''))
    L.append('')
    L.append('## 4. 抽 10 顆給筆電複驗')
    L.append('')
    pool = [r for r in sorted(vis, key=key) if r['cat'].startswith(('dead/', 's:unbound/', 'OK-', 's:bound'))]
    pick, seen = [], set()
    for r in pool:                                          # one per (category, page), deterministic
        k = (short(r['cat']), r['page'])
        if k in seen:
            continue
        seen.add(k)
        pick.append(r)
        if len(pick) == 10:
            break
    L.append('| # | 頁 | id | 分類 | 怎麼複驗 |')
    L.append('|---|---|---|---|---|')
    for i, r in enumerate(pick, 1):
        how = ('開這一頁按 `%s`，看 wb_serve oplog 有沒有收到命令；golden %s' % (r['id'], (r['g_handler'] or {}).get('loc', '—')))
        L.append('| %d | %s | %s | `%s` | %s |' % (i, r['page'], esc(r['id']), r['cat'], esc(how)))
    L.append('')
    L.append('## 5. 點擊實測（請 St01 代跑；STEVEN-NB3 只編譯、不執行）')
    L.append('')
    L.append('```')
    L.append('cd <repo>/HT9011UC_Cpp_V3.33.906.0/tools/webprobe')
    L.append('python c12_button_census.py --out-tsv c12_static.tsv')
    L.append('python c12_click_probe.py --census c12_static.tsv --out c12_click.tsv')
    L.append('python c12_button_census.py --merge c12_click.tsv --out-tsv <repo>/docs/handoff/ST02_BUTTON_CENSUS_20261002.tsv '
             '--out-md <repo>/docs/handoff/ST02_BUTTON_CENSUS_20261002.md')
    L.append('```')
    L.append('')
    L.append('- 探針只連自己的假伺服器：頁面腳本跑之前先把**所有** WebSocket 網址（有頁寫死 `ws://127.0.0.1:9045/...`）和 fetch／XHR 網址改到假伺服器的埠；'
             '假伺服器每個命令都回 ok:false、`/api/*` 回 404。碰不到 wb_serve、碰不到機台、不讀寫機台檔案。需要 Edge（不是 ctest）。')
    L.append('- 每頁單獨開（不在外框裡）；有分頁的先點那一頁的頁籤；confirm／prompt 一律回「確定」，才看得到確認之後送的命令。')
    L.append('- 已知限制：①C++ 資料到了才綁的鈕（例 Offset 部位鈕）在假伺服器下看起來沒反應，會落在 dead/；②FormShow bridge（Teach／IO）'
             '藏的元件在假伺服器下不會藏；③不在外框裡，`HT9045Link` 的 hub 路徑沒走到。這三種在 dead/ 裡要人看。')
    L.append('')
    with open(path, 'wb') as f:
        f.write(('\n'.join(L) + '\n').encode('utf-8'))
