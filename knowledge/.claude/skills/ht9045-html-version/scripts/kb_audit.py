# -*- coding: utf-8 -*-
# Steven 20260915
# ----------------------------------------------------------------------
# 產生 docs/KEYBOARD_AUDIT.md；孤兒頁刪除後重跑，A 1461/B 1233/C 53。
# 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
# ----------------------------------------------------------------------

"""kb_audit.py -- 小鍵盤覆蓋率稽核，產出紀錄檔。

AI(W906-FW-KB) 20260915。

機台上沒有實體鍵盤，小鍵盤是唯一輸入途徑。這支腳本掃出「哪些輸入框沒有小鍵盤」
與「哪些有小鍵盤但沒有 golden 依據（通用 QWERTY、無範圍夾限）」。

分三級：
  A 有 golden 依據          旗標與夾限抽自 ShowQwertyKey + .dfm OnMouseDown
  B 通用 QWERTY             引擎有掛鍵盤，但沒有 golden 旗標，也沒有範圍夾限
  C 完全沒有小鍵盤          該頁沒有接引擎，只能用實體鍵盤 -> 機台上打不了字
"""
import os, re, json, io, sys, glob

PAGE = r'D:\HT9045\web\page'
OUT  = r'D:\HT9045\docs\KEYBOARD_AUDIT.md'

RE_INPUT = re.compile(r'<\s*input([^>]*)>', re.I | re.S)
RE_ID    = re.compile(r'\bid\s*=\s*["\']([^"\']+)["\']', re.I)
RE_TYPE  = re.compile(r'\btype\s*=\s*["\']([^"\']+)["\']', re.I)
RE_DIS   = re.compile(r'\b(disabled|readonly)\b', re.I)


def read(p):
    return open(p, encoding='utf-8', errors='replace').read()


def kb_ids(datafile):
    """從接線資料檔取出有 golden 依據的 widget id。"""
    if not os.path.isfile(datafile):
        return set()
    s = read(datafile)
    m = re.search(r'kb:\s*\{(.*?)\n  \}', s, re.S) or re.search(r'KB\s*=\s*\{(.*?)\n  \};', s, re.S)
    if not m:
        return set()
    return set(re.findall(r'^\s*(\w+):\s*\[', m.group(1), re.M))


def main():
    rows = []
    for html in sorted(glob.glob(os.path.join(PAGE, '*.html'))):
        s = read(html)
        base = os.path.basename(html)
        # 這一頁有沒有掛引擎或手寫接線
        wired = ('ht9045_wire_engine.js' in s) or ('_wire.js' in s)
        has_qwerty = 'qwerty.js' in s
        # 資料檔
        # 注意：要排除 wire_engine —— 它是行為，不是資料，沒有 kb 區塊。
        cand = re.findall(r'src="(ht9045_(?:wire_[a-z0-9]+|contact_wire|hotplate_wire)\.js)"', s)
        cand = [c for c in cand if c != 'ht9045_wire_engine.js']
        data = cand[0] if cand else None
        golden = kb_ids(os.path.join(PAGE, data)) if data else set()

        texts, a, b, c = [], [], [], []
        for m in RE_INPUT.finditer(s):
            attrs = m.group(1)
            t = (RE_TYPE.search(attrs).group(1).lower() if RE_TYPE.search(attrs) else 'text')
            if t not in ('text', ''):
                continue
            idm = RE_ID.search(attrs)
            wid = idm.group(1) if idm else '(無 id)'
            texts.append(wid)
            if not (wired and has_qwerty):
                c.append(wid)
            elif wid in golden:
                a.append(wid)
            else:
                b.append(wid)
        if not texts:
            continue
        rows.append(dict(page=base, total=len(texts), A=a, B=b, C=c,
                         wired=wired, qwerty=has_qwerty, data=data))

    rows.sort(key=lambda r: (-len(r['C']), -len(r['B'])))
    L = []
    L.append('# 小鍵盤覆蓋率稽核')
    L.append('')
    L.append('> **//Steven 20260915** — 本檔由產生器輸出，標記寫在 kb_audit.py 的樣板裡，重跑後仍在。')
    L.append('> 當日完整變更紀錄：`D:' + chr(92) + 'HT9045' + chr(92) + 'CHANGES_20260915_Steven.md`')
    L.append('')
    L.append('> 由 `HT9011UC_Cpp_V3.33.906.0/scratchpad/kb_audit.py` 產生，'
             '**不要手改**。重跑即更新。')
    L.append('')
    L.append('**機台上沒有實體鍵盤**，小鍵盤是唯一輸入途徑。所以 C 級欄位在機台上'
             '是「看得到但打不了字」，必須處理。')
    L.append('')
    L.append('## 輸入途徑規格（20260915 定案）')
    L.append('')
    L.append('1. **HTML 畫面本身不可以使用實體鍵盤** —— 所有文字框設 `readonly`，'
             '單擊叫出小鍵盤（與 golden 的 `OnMouseDown` 一致）。')
    L.append('2. **只有小鍵盤出現時**，才可以用「對應到小鍵盤按鍵」的實體鍵：'
             '可見字元、Backspace(`⌫`)、Delete、Enter、Escape(`Abort`)、Space。'
             '小鍵盤上沒有的鍵一律不接受；小鍵盤沒開時實體鍵完全無效。')
    L.append('')
    L.append('實作在 `web/page/ht9045_wire_engine.js` 的 `physicalKeys()`。'
             '⚠ 沒有做在 `qwerty.js` 是因為那是網頁作者的檔案，`sync_web.py --apply` '
             '會整檔覆蓋。長久解法是網頁作者把它收進 `qwerty.js`。')
    L.append('')
    L.append('| 級別 | 意義 | 機台上可用？ |')
    L.append('|---|---|---|')
    L.append('| **A** | 有 golden 依據：旗標與範圍夾限抽自 `ShowQwertyKey` ＋ `.dfm` 的 `OnMouseDown` | ✅ 完全可用 |')
    L.append('| **B** | 通用 QWERTY：引擎有掛鍵盤，但沒有 golden 旗標，**也沒有範圍夾限** | ⚠ 可輸入，但少一道防呆 |')
    L.append('| **C** | 完全沒有小鍵盤 | ❌ **機台上打不了字** |')
    L.append('')
    ta = sum(len(r['A']) for r in rows)
    tb = sum(len(r['B']) for r in rows)
    tc = sum(len(r['C']) for r in rows)
    L.append('## 總計')
    L.append('')
    L.append('| | 數量 |')
    L.append('|---|---|')
    L.append('| A 有 golden 依據 | **%d** |' % ta)
    L.append('| B 通用 QWERTY | **%d** |' % tb)
    L.append('| C 沒有小鍵盤 | **%d** |' % tc)
    L.append('| 合計文字輸入框 | %d |' % (ta + tb + tc))
    L.append('')
    L.append('## 逐頁')
    L.append('')
    L.append('| 頁面 | 文字框 | A | B | C | 接線檔 |')
    L.append('|---|---|---|---|---|---|')
    for r in rows:
        L.append('| `%s` | %d | %d | %d | %s | %s |'
                 % (r['page'], r['total'], len(r['A']), len(r['B']),
                    ('**%d**' % len(r['C'])) if r['C'] else '0',
                    ('`%s`' % r['data']) if r['data'] else '—'))
    L.append('')
    if tc:
        L.append('## C 級明細 —— 機台上打不了字，要優先處理')
        L.append('')
        for r in rows:
            if not r['C']:
                continue
            why = []
            if not r['wired']:
                why.append('沒有接引擎')
            if not r['qwerty']:
                why.append('沒有載入 qwerty.js')
            L.append('### `%s`（%d 個）' % (r['page'], len(r['C'])))
            L.append('')
            L.append('原因：%s' % ('、'.join(why) or '未知'))
            L.append('')
            L.append('```')
            L.append(' '.join(r['C'][:60]) + (' ...' if len(r['C']) > 60 else ''))
            L.append('```')
            L.append('')
    L.append('## B 級明細 —— 可輸入但無範圍夾限')
    L.append('')
    L.append('這些欄位在 golden 裡沒有對應的 `ShowQwertyKey` 呼叫（或該呼叫的 min/max 是 C++ '
             '執行期變數，JS 取不到值）。引擎給它們通用 QWERTY，**不夾限**——')
    L.append('猜一個夾限比不夾限更危險，所以刻意留空。')
    L.append('')
    for r in rows:
        if not r['B']:
            continue
        L.append('- **`%s`**（%d 個）：`%s`%s'
                 % (r['page'], len(r['B']), '` `'.join(r['B'][:25]),
                    ' …' if len(r['B']) > 25 else ''))
    L.append('')
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    open(OUT, 'w', encoding='utf-8', newline='').write('\n'.join(L))
    print('A(有 golden)=%d  B(通用 QWERTY)=%d  C(無鍵盤)=%d' % (ta, tb, tc))
    print('輸出: %s' % OUT)


main()
