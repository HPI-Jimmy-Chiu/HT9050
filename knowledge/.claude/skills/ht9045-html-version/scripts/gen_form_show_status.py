# -*- coding: utf-8 -*-
# Steven 20260919
# ----------------------------------------------------------------------
# 新檔。盤點 HT9045.cpp 的 Application->CreateForm 清單裡，哪些表單建立後
# 從未被 Show() / ShowModal() 顯示過，寫入 screenshot_meta.js 的
# FORM_SHOW_STATUS。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260919_Steven.md
# ----------------------------------------------------------------------

"""gen_form_show_status.py -- 產生 FORM_SHOW_STATUS 並寫進 screenshot_meta.js。

回答一個問題：HT9045.cpp 開機時一口氣 Application->CreateForm 出來的那一批
表單，有幾個從頭到尾沒有任何地方叫它顯示。

三邊的資料都是量出來的，不是人工維護的清單：
  建立了什麼    -> HT9045.cpp 的 Application->CreateForm(__classid(T*), &var)
  顯示了什麼    -> 全樹 *.cpp/*.h 的 var->Show( 與 var->ShowModal(
  替代顯示路徑  -> var->Visible = 、var->BringToFront(、dfm 的 Visible = True

判定（shownBy）：
  main     Application->MainForm，由 Application->Run 自動顯示（只有第一個）
  modal    有 var->ShowModal()
  show     有 var->Show()
  indirect 沒有人直接叫它，但它自己有個方法裡寫了裸的 Show()/ShowModal()，
           而那個方法被單元外面拿到（var->Method，或 Btn->OnClick=var->Handler）
  visible  以上都沒有，但有 var->Visible=true 或 dfm 的 Visible=True
  none     全都沒有 —— 建立了但從來不顯示

⚠ indirect 這一級是必要的，不是湊數。實測兩個都靠它才顯示得出來：
    ATCInterfaceForm  main.cpp 叫 ShowInterface(1)，方法內部才 Show()
    HGem              SECSGEM.cpp 把 GemSBSetup->OnClick 指到 HGem->GemSBSetupClick，
                      那個 handler 內部 ShowModal()
  只看 var->Show( 會把這兩個誤判成「從來不顯示」。

⚠ 「none」不等於「沒有用」。TDataModule 後裔（kind=datamodule）本來就沒有
  視窗，CreateForm 只是替它做生命週期與 Owner 管理；這一類在表裡另外標
  kind=datamodule，要跟真的「有視窗卻不顯示」的分開看。

⚠ 掃描前一定要先剝掉註解與字串內容。直接 grep 會把 `//fXxx->Show();` 這種
  註解掉的呼叫算成有顯示，而 golden 樹裡這種註解不少；也會被字串裡的 /*
  帶著一路吃掉幾千行程式碼，讓結論整個反過來（實測 main.cpp 會少掉
  fBinSel->ShowModal() 那一段）。strip() 就是為了這兩個坑。

用法: py -3 gen_form_show_status.py [BASE]
"""
import collections
import glob
import io
import json
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

BASE = sys.argv[1] if len(sys.argv) > 1 else \
    r'D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2'
METAS = [r'D:\HT9045\page\screenshot_meta.js',
         r'D:\HT9045\web\page\screenshot_meta.js']

BS = chr(92)


def strip(t):
    """剝掉註解與字串內容，保留行數。字串裡的 /* 不會把它帶走。"""
    out = []
    i, n = 0, len(t)
    while i < n:
        c = t[i]
        if c == '/' and i + 1 < n:
            d = t[i + 1]
            if d == '/':
                j = t.find('\n', i)
                i = n if j < 0 else j
                continue
            if d == '*':
                j = t.find('*/', i + 2)
                seg = t[i:(n if j < 0 else j + 2)]
                out.append('\n' * seg.count('\n'))
                i = n if j < 0 else j + 2
                continue
        if c == '"' or c == "'":
            q, j = c, i + 1
            while j < n:
                if t[j] == BS:
                    j += 2
                    continue
                if t[j] == q:
                    j += 1
                    break
                if t[j] == '\n':
                    break
                j += 1
            out.append(' ')
            i = j
            continue
        out.append(c)
        i += 1
    return ''.join(out)


def read(path):
    return io.open(path, encoding='cp950', errors='replace').read()


def main():
    main_cpp = os.path.join(BASE, 'HT9045.cpp')
    if not os.path.isfile(main_cpp):
        print('找不到 ' + main_cpp)
        return 1

    txt = strip(read(main_cpp))
    created = [(m.group(1), m.group(2)) for m in re.finditer(
        r'Application->CreateForm\(__classid\((\w+)\)\s*,\s*&(\w+)\s*\)', txt)]
    if not created:
        print('HT9045.cpp 裡找不到 CreateForm')
        return 1
    order = {v: i for i, (_c, v) in enumerate(created)}
    vars_ = set(order)

    srcs = []
    for ext in ('*.cpp', '*.h'):
        srcs += glob.glob(os.path.join(BASE, '**', ext), recursive=True)

    member = re.compile(r'(\w+)\s*->\s*(ShowModal|Show|Visible|BringToFront)\b')
    anyref = re.compile(r'(\w+)\s*->\s*(\w+)')
    # 裸的 Show()/ShowModal()：前面沒有 -> 也沒有 .，就是 this->
    selfshow = re.compile(r'(?<![>.\w])(ShowModal|Show)\s*\(')
    # 方法定義 TCls::Name(
    methdef = re.compile(r'\b(T\w+)::(\w+)\s*\(')
    hits = collections.defaultdict(lambda: collections.defaultdict(list))
    refs = collections.Counter()
    # cls -> {方法名: 'file:line'}，該方法內部會讓自己顯示
    selfmeth = collections.defaultdict(dict)
    # var -> 外面用到的成員名
    usedmem = collections.defaultdict(set)

    texts = {}
    for f in srcs:
        try:
            t = strip(read(f))
        except Exception:
            continue
        texts[f] = t
        rel = os.path.relpath(f, BASE).replace(os.sep, '/')
        for m in member.finditer(t):
            v, mem = m.group(1), m.group(2)
            if v in vars_:
                hits[v][mem].append('%s:%d' % (rel, t.count('\n', 0, m.start()) + 1))
        for m in anyref.finditer(t):
            if m.group(1) in vars_:
                refs[m.group(1)] += 1
                usedmem[m.group(1)].add(m.group(2))

    # 把每個裸 Show()/ShowModal() 歸給它所在的那個 TCls::Method
    for f, t in texts.items():
        defs = [(m.start(), m.group(1), m.group(2)) for m in methdef.finditer(t)]
        if not defs:
            continue
        rel = os.path.relpath(f, BASE).replace(os.sep, '/')
        for m in selfshow.finditer(t):
            owner = None
            for pos, cls, name in defs:
                if pos < m.start():
                    owner = (cls, name)
                else:
                    break
            if owner:
                selfmeth[owner[0]].setdefault(
                    owner[1], '%s:%d' % (rel, t.count('\n', 0, m.start()) + 1))

    # class -> 基底類別 / 宣告單元
    base_of, unit_of = {}, {}
    decl = re.compile(r'class\s+(\w+)\s*:\s*public\s+(\w+)')
    for f in srcs:
        if not f.lower().endswith('.h'):
            continue
        try:
            t = strip(read(f))
        except Exception:
            continue
        for m in decl.finditer(t):
            if m.group(1) not in base_of:
                base_of[m.group(1)] = m.group(2)
                unit_of[m.group(1)] = os.path.splitext(os.path.basename(f))[0]

    # dfm 裡 Visible = True 的表單，建立當下就自己出現，不需要有人叫 Show()
    dfm_of, dfm_visible = {}, set()
    for d in glob.glob(os.path.join(BASE, '**', '*.dfm'), recursive=True):
        try:
            lines = io.open(d, encoding='cp950', errors='replace').read().split('\n')
        except Exception:
            continue
        m = re.match(r'object (\w+): (\w+)', lines[0].strip())
        if not m:
            continue
        dfm_of[m.group(2)] = os.path.basename(d)
        for ln in lines[1:80]:
            s = ln.strip()
            if s.startswith('object ') or s.startswith('inherited '):
                break   # 進到子控制項了，後面的 Visible 不是表單自己的
            if re.match(r'Visible\s*=\s*True$', s):
                dfm_visible.add(m.group(2))
                break

    rows = []
    for cls, var in created:
        h = hits.get(var, {})
        base = base_of.get(cls, '')
        kind = 'datamodule' if base.endswith('TDataModule') else 'form'
        # 自家方法裡有裸 Show()，而那個方法被單元外面拿到 -> 間接顯示得出來
        via = sorted(n for n in selfmeth.get(cls, {}) if n in usedmem.get(var, ()))
        if order[var] == 0:
            by = 'main'
        elif h.get('ShowModal'):
            by = 'modal'
        elif h.get('Show'):
            by = 'show'
        elif via:
            by = 'indirect'
        elif h.get('Visible') or cls in dfm_visible:
            by = 'visible'
        else:
            by = 'none'
        sites = h.get('ShowModal') or h.get('Show') or \
            ['%s() @ %s' % (n, selfmeth[cls][n]) for n in via] or h.get('Visible') or []
        rows.append({
            'cls': cls, 'var': var, 'unit': unit_of.get(cls, ''),
            'dfm': dfm_of.get(cls, ''), 'base': base, 'kind': kind,
            'shownBy': by, 'sites': sites[:4],
            'via': via,
            'dfmVisible': cls in dfm_visible,
            'btf': len(h.get('BringToFront', [])),
            'refs': refs.get(var, 0),
        })

    block = ('\n\n// AI(W906-FW-FORMSHOW) 20260919: HT9045.cpp 的 CreateForm 清單，'
             '哪些建立後從未顯示。\n'
             '// 由 scratchpad/gen_form_show_status.py 量出來（掃描前先剝掉註解與字串，\n'
             '// 否則被註解掉的 Show() 會把結論反過來），不要手改。\n'
             'FORM_SHOW_STATUS = [\n'
             + '\n'.join(' ' + json.dumps(r, ensure_ascii=False) + ',' for r in rows).rstrip(',')
             + '\n];\n')
    for meta in METAS:
        if not os.path.isfile(meta):
            print('略過（不存在）：' + meta)
            continue
        src = io.open(meta, encoding='utf-8').read()
        src = re.sub(r'\n*// AI\(W906-FW-FORMSHOW\).*?\nFORM_SHOW_STATUS = \[.*?\n\];\n',
                     '\n', src, flags=re.S)
        io.open(meta, 'w', encoding='utf-8', newline='').write(src.rstrip('\n') + block)
        print('已寫入 ' + meta)

    cnt = collections.Counter(r['shownBy'] for r in rows)
    nonef = [r for r in rows if r['shownBy'] == 'none']
    print('')
    print('來源樹：' + BASE)
    print('CreateForm 共 %d 個：主表單 %d／ShowModal %d／Show %d／間接 %d／'
          '只靠 Visible %d／從未顯示 %d'
          % (len(rows), cnt['main'], cnt['modal'], cnt['show'], cnt['indirect'],
             cnt['visible'], cnt['none']))
    print('從未顯示的 %d 個裡，TDataModule（本來就沒有視窗）%d 個、真的有視窗 %d 個'
          % (len(nonef), sum(1 for r in nonef if r['kind'] == 'datamodule'),
             sum(1 for r in nonef if r['kind'] == 'form')))
    print('')
    print('%-24s %-22s %-18s %-12s %-11s %s'
          % ('class', '變數', '單元', '基底', 'kind', '參考次數'))
    for r in nonef:
        print('%-24s %-22s %-18s %-12s %-11s %d'
              % (r['cls'], r['var'], r['unit'], r['base'], r['kind'], r['refs']))
    return 0


if __name__ == '__main__':
    sys.exit(main())
