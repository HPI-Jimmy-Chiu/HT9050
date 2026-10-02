# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 量出每頁接線程度，寫入 screenshot_meta.js 的 PAGE_WIRE_STATUS（本次補算 sysEnums）。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""產生 PAGE_WIRE_STATUS 並寫進 screenshot_meta.js。

AI(W906-FW-PAGESTATUS) 20260915。

回答一個問題：每一頁 HTML 現在跟 wb_serve 接到什麼程度。
資料全部從實體檔案量出來，不是人工維護的清單：

  fields/optional  -> 配方（/api/recipe/<doc> + recipe.doc.put）
  sysFields        -> 機台設定檔（/api/system/<name> + system.file.put）
  kb               -> QWERTY 小鍵盤欄位數
  pending          -> 抽取有誤、刻意不接的欄位

分級：
  data  已接資料（可讀寫）
  kb    只有小鍵盤，資料未接
  none  兩者皆無
"""
import io
import json
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

PAGE = 'D:/HT9045/web/page'
META = 'D:/HT9045/page/screenshot_meta.js'


def count_block(text, name):
    m = re.search(r'\b' + name + r':\s*\{', text)
    if not m:
        return 0
    i, depth = m.end(), 1
    while i < len(text) and depth:
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
        i += 1
    return len(re.findall(r'^\s*[A-Za-z_][A-Za-z0-9_]*\s*:', text[m.end():i - 1], re.M))


# 每頁 -> 它的接線 js
wire_of = {}
for fn in os.listdir(PAGE):
    if not fn.endswith('.html'):
        continue
    html = open(os.path.join(PAGE, fn), encoding='utf-8', errors='replace').read()
    js = re.findall(r'src="(ht9045_[A-Za-z0-9_]+\.js)"', html)
    # Steven 20260918: ht9045_wire_livesettings.js 是共用函式庫（跟 engine、recipe_client 同性質），
    # 不是「一頁一檔」的接線資料。不排除掉的話，掛了它的頁面會被當成接線檔是它，
    # 該頁的 sysFields / kb 就整個歸零 —— 實測會讓系統檔 947->560、鍵盤 1466->941。
    js = [x for x in js if x not in ('ht9045_wire_engine.js', 'ht9045_recipe_client.js',
                                     'ht9045_wire_livesettings.js')]
    wire_of[fn] = js[0] if js else ''

rows = []
for fn in sorted(wire_of):
    js = wire_of[fn]
    f = o = s = p = k = tg = 0
    grid = lvl = ''                                                        # Steven 20260916：每頁重設，不然會漏到下一頁
    if js and os.path.isfile(os.path.join(PAGE, js)):
        t = open(os.path.join(PAGE, js), encoding='utf-8', errors='replace').read()
        f, o, s, p, k = (count_block(t, n) for n in
                         ('fields', 'optional', 'sysFields', 'pending', 'kb'))
        # Steven 20260916
        # sysEnums（radio group / checkbox / combo）也是機台設定檔欄位，
        # 只是控制項不是文字框。漏算它，HW.HandlerSys 會顯示 33 而不是 214。
        s += count_block(t, 'sysEnums')
        # Steven 20260916
        # sysGrid（表格 = 檔案）：沒有逐格 id，所以 sysFields/sysEnums 都是 0，
        # 但整張 csv 都可讀寫。用 grid 欄記下檔名，level 判成 data。
        gm = re.search(r"sysGrid:\s*\{[^}]*?file:\s*'([^']+)'", t, re.S)
        grid = gm.group(1) if gm else ''
        # Steven 20260916 (W906-FW-LEVELSET)
        # sysLevels（i32 投影 = 檔案）：Status.Security 的 179 組權限 radio 沒有 id，
        # 對照表是執行期從 DOM 的 [NN] 掃出來的，所以 sysFields/sysEnums 都是 0。
        # 不算它的話，那一頁會顯示成「完全未接」——明明整份 levelset.dat 都可讀寫。
        lm = re.search(r"sysLevels:\s*\{[^}]*?file:\s*'([^']+)'[^}]*?expect:\s*(\d+)", t, re.S)
        if lm:
            lvl = lm.group(1)
            s += int(lm.group(2))
        # Steven 20260916 (W906-FW-TAGSUB)
        # tags（執行期唯讀顯示）另計，不混進「系統檔欄位」——那一欄講的是可讀寫的檔案欄位。
        # 不能用 count_block：它的鍵名正則是 [A-Za-z_]\w*，而 tag 名帶點
        # （temp.sv）一定要加引號，於是一個都數不到，main.html 會被判成 'kb'。
        tm = re.search(r'\btags:\s*\{(.*?)\n  \}', t, re.S)
        tg = len(re.findall(r"^\s*'[^']+'\s*:", tm.group(1), re.M)) if tm else 0
        if js in ('ht9045_contact_wire.js', 'ht9045_hotplate_wire.js'):
            # 這兩支是引擎出現前的專屬接線，欄位不是用 fields:{} 區塊寫的
            f = len(re.findall(r'^\s*(?:ed|edt|cb)[A-Za-z0-9_]+\s*:', t, re.M))
    html = open(os.path.join(PAGE, fn), encoding='utf-8', errors='replace').read()
    inputs = len(re.findall(r'<input', html))
    # Steven 20260916：只接 tag 的頁面算 'tag' ——它有活的資料在顯示，
    # 但按存檔不會寫回任何檔案，跟 'data' 不是同一回事，也不該算成 'none'。
    level = ('data' if (f + o + s or grid) else
             'tag' if tg else ('kb' if k else 'none'))
    rows.append({
        # Steven 20260916：表格與 i32 投影的標籤要分開 —— levelset 不是表格，
        # 標成「[表格 levelset]」會讓人以為那一頁有一張 csv。
        'page': fn,
        'wire': (js + ('  [表格 ' + grid + ']' if grid else '')
                    + ('  [權限表 ' + lvl + ']' if lvl else '')) if js else js,
        'level': level, 'grid': grid, 'levels': lvl,
        'recipe': f + o, 'sys': s, 'kb': k, 'pending': p, 'inputs': inputs, 'tags': tg,
    })

rows.sort(key=lambda r: (-(r['recipe'] + r['sys']), -r['kb'], r['page']))

src = open(META, encoding='utf-8').read()
block = ('\n\n// AI(W906-FW-PAGESTATUS) 20260915: 每頁與 wb_serve 的接線程度。\n'
         '// 由 scratchpad/gen_page_status.py 從實體接線檔量出來，不要手改。\n'
         'PAGE_WIRE_STATUS = [\n'
         + '\n'.join(' ' + json.dumps(r, ensure_ascii=False) + ',' for r in rows).rstrip(',')
         + '\n];\n')
src = re.sub(r'\n*// AI\(W906-FW-PAGESTATUS\).*?\nPAGE_WIRE_STATUS = \[.*?\n\];\n', '\n', src, flags=re.S)
open(META, 'w', encoding='utf-8', newline='').write(src.rstrip('\n') + block)

d = sum(1 for r in rows if r['level'] == 'data')
kb = sum(1 for r in rows if r['level'] == 'kb')
no = sum(1 for r in rows if r['level'] == 'none')
print('%-34s %-6s %7s %5s %5s %8s' % ('page', 'level', 'recipe', 'sys', 'kb', 'inputs'))
for r in rows:
    print('%-34s %-6s %7d %5d %5d %8d'
          % (r['page'], r['level'], r['recipe'], r['sys'], r['kb'], r['inputs']))
print()
print('共 %d 頁：已接資料 %d、只有鍵盤 %d、完全未接 %d' % (len(rows), d, kb, no))
print('配方欄位合計 %d、系統檔欄位合計 %d、鍵盤欄位合計 %d'
      % (sum(r['recipe'] for r in rows), sum(r['sys'] for r in rows), sum(r['kb'] for r in rows)))
