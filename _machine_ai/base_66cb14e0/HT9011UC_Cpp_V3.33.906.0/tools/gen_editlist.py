# -*- coding: utf-8 -*-
# Steven 20260924
# ----------------------------------------------------------------------
# S12 C 類（HTEditList）產生器。直接讀 golden BCB 原檔（cp950），把表單的
# HTEditList 註冊碼與讀寫流程改寫成「具名替身」版，輸出 FileRW/<結構>.cpp。
# 規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md 一之二
# ----------------------------------------------------------------------
"""gen_editlist.py -- 由 golden 原檔產生 C 類（HTEditList）讀寫檔。

使用者 20260924：「CPP 端沒有元件，但是元件在 html 端」「元件的部分改用名稱即可」
「elConfig、cbLastSet、elConfig_byRecipe 從未被 new 過 —— 你參考 BCB 的，不要看 CPP」。

改寫規則（只動程式碼，註解原樣保留並轉成 UTF-8）：
  1. golden header 裡宣告的 widget 名稱 W（例 cbA01、edA01、tsN23）→ EL<型別>("W")
     —— 具名替身（FileRW/_EditList.h）。同名永遠同一個物件，`->Tag`／`->Visible` 照舊成立。
     golden header 的型別 vclcompat 沒有的（TTrackBar、TImage…）退回 TControl。
  4. 寫方向另有三條：ShowMyMessage → filerw::ELMessage（進 JSON messages）；
     ShowMyMessageBox_YES_NO → filerw::ELAsk（頁面已確認的題目回 1＝YES，沒答的回 2＝NO 並列出）；
     blocks 列的 golden 行段 → #if 0 ＋ filerw::ELTodo（原文保留、執行時回報待辦）。
  2. 表單自己的方法 m(…) → IC_m(…)（只限 METHODS 裡列的）。
  3. 其餘（IniConfig、CosFunction、elConfig->Add(…)、ReadIniData…）原樣 —— 對到移植樹的全域。
  字串常值與註解不改。改寫後若有 overrides 沒用到或 golden 行號核對不過，產生器中止。

用法：python tools/gen_editlist.py      （在 HT9011UC_Cpp_V3.33.906.0 底下跑）
"""
import io
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
OUT = os.path.join(ROOT, 'FileRW')
GOLDEN = r'D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'

# vclcompat 有的型別（其餘退回 TControl）
KNOWN = {'TEdit', 'TCheckBox', 'TComboBox', 'TRadioGroup', 'TLabel', 'TPanel', 'TGroupBox', 'TTabSheet',
         'TPageControl', 'TSpeedButton', 'TButton', 'TBitBtn', 'TMemo', 'TLabeledEdit', 'TRadioButton',
         'TListBox', 'TDateTimePicker'}
# golden 型別 → 替身的 C++ 型別（vclcompat 沒有、或少了 golden 用到的屬性）：FileRW/_EditList.h
MAPPED = {'TTrackBar': 'filerw::ELTrackBar', 'TUpDown': 'filerw::ELTrackBar', 'TDateTimePicker': 'filerw::ELDateTimePicker',
          'TStringGrid': 'filerw::ELStringGrid'}

# ----------------------------------------------------------------------
# 每個結構的設定：tools/editlist/<struct>.py（檔內 STRUCT = {...}；檔名＝STRUCT['struct']）。
# Steven 20260924：拆檔，讓不同工程師各改各的結構、不互相衝突（同 tools/formbridge/）。
# 順序＝golden 開機 CreateForm 的註冊順序很重要（HTEditList 同鍵第一筆生效），所以用 ORDER 明列；
# 不在 ORDER 裡的新檔排在最後（照檔名）。
# ----------------------------------------------------------------------
EDITLIST_DIR = os.path.join(HERE, 'editlist')
ORDER = ['IniConfig', 'Ld_UldDelayTime', 'UserDefForm_File', 'ArmSpeed_File']


def load_structs():
    only = sys.argv[sys.argv.index('--only') + 1] if '--only' in sys.argv else None
    fns = [f for f in os.listdir(EDITLIST_DIR) if f.endswith('.py') and not f.startswith('_')]
    fns.sort(key=lambda f: (ORDER.index(f[:-3]) if f[:-3] in ORDER else len(ORDER), f))
    out = []
    for fn in fns:
        ns = {}
        try:
            exec(compile(open(os.path.join(EDITLIST_DIR, fn), encoding='utf-8').read(), fn, 'exec'), ns)
        except Exception as e:
            if only and fn != only + '.py':
                print('⚠ 跳過 %s（--only %s；這個設定檔目前載入失敗：%s）' % (fn, only, e))
                continue
            raise
        st = ns['STRUCT']
        if st['struct'] + '.py' != fn:
            if only and fn != only + '.py':   # 審查第 9 輪 M-4：別人的檔不擋自己的 --only
                print('⚠ 跳過 %s（STRUCT struct %s 與檔名不符）' % (fn, st['struct']))
                continue
            raise SystemExit('%s: STRUCT struct %s does not match the file name' % (fn, st['struct']))
        out.append(st)
    return out


STRUCTS = load_structs()


def cp950(path):
    return open(path, 'rb').read().decode('cp950', errors='replace')


def split_code(line):
    """把一行切成 [(是程式碼?, 文字)]：字串常值與 // 註解不改寫。"""
    out, i, n, buf = [], 0, len(line), ''
    while i < n:
        c = line[i]
        if line.startswith('//', i):
            if buf: out.append((True, buf)); buf = ''
            out.append((False, line[i:])); return out
        if c in '"\'':
            if buf: out.append((True, buf)); buf = ''
            j = i + 1
            while j < n and line[j] != c:
                j += 2 if line[j] == '\\' else 1
            out.append((False, line[i:j + 1])); i = j + 1; continue
        buf += c; i += 1
    if buf: out.append((True, buf))
    return out


def widgets_of(h, cls):
    m = re.search(r'\bclass\s+(?:PACKAGE\s+)?' + cls + r'\b[^;{]*\{', h)
    body = h[m.end():]
    out = {}
    for mm in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', body, re.M):
        t = mm.group(1)
        out[mm.group(2)] = MAPPED.get(t) or (t if t in KNOWN else 'TControl')
    return out


def dfm_string(tok):
    """BCB6 文字 DFM 的字串：'abc'、'it''s'、#12345 串接。#n 有 >255 的是 Unicode 碼位，否則是 cp950 位元組。"""
    parts, i, codes = [], 0, []
    out = ''
    raw = []          # (kind, value)
    while i < len(tok):
        c = tok[i]
        if c == "'":
            j, buf = i + 1, ''
            while j < len(tok):
                if tok[j] == "'" and j + 1 < len(tok) and tok[j + 1] == "'":
                    buf += "'"; j += 2; continue
                if tok[j] == "'":
                    break
                buf += tok[j]; j += 1
            raw.append(('s', buf)); i = j + 1
        elif c == '#':
            m = re.match(r'#(\d+)', tok[i:])
            raw.append(('c', int(m.group(1)))); i += len(m.group(0))
        else:
            i += 1
    # 審查 20260924 #4：BCB6 文字 DFM 的 #nnn 是 Unicode 碼位（這份 DFM 有 #176、#65306，沒有原始高位元組）
    uni = True
    b = bytearray()
    for k, v in raw:
        if k == 's':
            if uni:
                out += v
            else:
                b += v.encode('cp950', errors='replace')
        else:
            if uni:
                out += chr(v)
            else:
                b.append(v)
    return out if uni else b.decode('cp950', errors='replace')


def dfm_items(dfm_text):
    """{元件名: [Items.Strings…]}（TComboBox／TRadioGroup／TListBox 的設計期清單）"""
    out, stack = {}, []
    lines = dfm_text.replace('\r\n', '\n').split('\n')
    i = 0
    while i < len(lines):
        ln = lines[i].strip()
        m = re.match(r'(?:object|inherited|inline)\s+(\w+)\s*:\s*(\w+)', ln)
        if m:
            stack.append(m.group(1))
        elif ln == 'end':
            if stack: stack.pop()
        elif ln.startswith('Items.Strings = (') and stack:
            items, cur = [], ln[len('Items.Strings = ('):]
            while True:
                t = cur.strip()
                done = t.endswith(')')
                if done: t = t[:-1].rstrip()
                if t:
                    # 一個項目可能以 + 續行
                    while t.endswith('+'):
                        i += 1
                        t = t[:-1].rstrip() + lines[i].strip()
                        if t.endswith(')'):
                            done = True; t = t[:-1].rstrip()
                    items.append(dfm_string(t))
                if done: break
                i += 1
                cur = lines[i]
            out[stack[-1]] = items
        i += 1
    return out


def dfm_parents(dfm_text, types=None, props=None):
    """{元件名: 父元件名}（DFM 的 object … end 巢狀）。types／props 有給就順便收 {名: 型別}、
    {名: {'Enabled': 'False', 'ReadOnly': 'True', …}}（元件自己那一層的屬性，不含子元件的）。"""
    out, stack = {}, []
    for ln in dfm_text.replace('\r\n', '\n').split('\n'):
        t = ln.strip()
        m = re.match(r'(?:object|inherited|inline)\s+(\w+)\s*:\s*(\w+)', t)
        if m:
            if stack:
                out[m.group(1)] = stack[-1]
            stack.append(m.group(1))
            if types is not None:
                types[m.group(1)] = m.group(2)
        elif t == 'end' and stack:
            stack.pop()
        elif props is not None and stack:
            pm = re.match(r'(Enabled|ReadOnly|Visible)\s*=\s*(True|False)$', t) or \
                 re.match(r'(Min|Max|Position|Associate|OnChange|Checked|ItemIndex)\s*=\s*(-?\w+)$', t)   # TTrackBar／TUpDown／設計期值（Steven 20260924）
            if pm:
                props.setdefault(stack[-1], {})[pm.group(1)] = pm.group(2)
            else:
                tm = re.match(r"Text\s*=\s*((?:'[^']*'|#\d+)+)$", t)   # 設計期 Text（單行；續行 + 的不收）
                if tm:
                    props.setdefault(stack[-1], {})['Text'] = dfm_string(tm.group(1))
    return out


def cstr(v):
    return '"' + v.replace('\\', '\\\\').replace('"', '\\"') + '"'


def body_of(cpp, cls, name):
    # 建構子的初始化串列（`: TForm(Owner)`）也允許（Steven 20260924：TfLd_ULd 的註冊在建構子裡）
    m = re.search(r'^[^\n]*\b' + cls + r'::' + name + r'\s*\(([^)]*)\)\s*(?:(?://[^\n]*\s*)|(?::\s*[^{;/]*))*\{',
                  cpp, re.M)
    if not m:
        raise SystemExit('golden %s::%s not found' % (cls, name))
    i, d, q = m.end(), 1, None
    while d and i < len(cpp):
        c = cpp[i]
        if q:
            if c == '\\': i += 1
            elif c == q: q = None
        elif cpp.startswith('//', i):
            i = cpp.find('\n', i); continue
        elif c in '"\'': q = c
        elif c == '{': d += 1
        elif c == '}': d -= 1
        i += 1
    return m.group(1), cpp[m.end():i - 1], cpp.count('\n', 0, m.end()) + 1, cpp.count('\n', 0, m.start()) + 1


def convert(st):
    P = st.get('prefix', 'IC')          # 產生的 static 函式／表的前綴（IniConfig＝IC；一個結構一個）
    st['_sig'] = {}
    cpp = cp950(os.path.join(GOLDEN, st['cpp']))
    h = cp950(os.path.join(GOLDEN, st['h']))
    W = widgets_of(h, st['class'])
    names = sorted(W, key=len, reverse=True)
    wre = re.compile(r'(?<![\w.>:])(' + '|'.join(map(re.escape, names)) + r')\b(?!\s*::)')
    mre = re.compile(r'(?<![\w.>:])(' + '|'.join(st['methods']) + r')\s*\(')
    out, used = [], set()
    blocks = {}
    for b in st.get('blocks', []):
        blocks.setdefault(b[0], []).append(b)
    for b in st.get('replace', []):
        blocks.setdefault(b[0], []).append(b)
    hit_blocks = set()
    for meth in st['methods']:
        params, body, line0, sig = body_of(cpp, st['class'], meth)
        out.append('// golden %s:%d  %s::%s(%s)' % (st['cpp'], sig, st['class'], meth, ' '.join(params.split())))
        st['_sig'][meth] = st.get('params', {}).get(meth, params.strip())
        out.append('static %s %s_%s(%s)' % (st.get('rettype', {}).get(meth, 'void'), P, meth, st['_sig'][meth]))
        out.append('{')
        if meth in st.get('save_methods', []):
            out.append('    filerw::ELMark("%s");   // FileRW：存檔流程 trace（ack.trace）' % meth)
        for n, raw in enumerate(body.split('\n')):
            gl = line0 + n
            for b in blocks.get(meth, []):
                if gl == b[1]:
                    hit_blocks.add(b)
                    if len(b) > 4:      # replace：不是待辦，是等價取代
                        out.append('    %s   // golden %s:%d-%d 取代：%s' % (b[4], st['cpp'], b[1], b[2], b[3]))
                    elif meth in st.get('save_methods', []):
                        out.append('    filerw::ELTodo("golden %s:%d-%d %s");' % (st['cpp'], b[1], b[2], b[3]))
                    else:               # 顯示端（FormShow）：不進存檔 ack 的 todo，只留原文
                        pass
                    out.append('#if 0 // GATE (S12-C save) golden %s:%d-%d -- %s' % (st['cpp'], b[1], b[2], b[3]))
            parts = []
            for is_code, txt in split_code(raw.rstrip('\r')):
                if is_code:
                    txt = mre.sub(lambda m: P + '_' + m.group(1) + '(', txt)
                    def w(m):
                        used.add(m.group(1))
                        return 'EL<%s>("%s", "%s")' % (W[m.group(1)], st['class'], m.group(1))
                    txt = wre.sub(w, txt)
                    txt = re.sub(r'\bShowMyMessageBox_YES_NO\s*\(', 'filerw::ELAsk(', txt)
                    txt = re.sub(r'\bShowMyMessage\s*\(', 'filerw::ELMessage(', txt)
                    # golden 權限：ChangeCompomentEnabled(容器, bEnable, bMustEnable) —— 替身沒有父子樹，
                    # 只設容器替身的 Enabled（golden 同一套規則），頁面依 DOM 把容器底下全部停用（proxies.enabled）
                    txt = re.sub(r'\bChangeCompomentEnabled\s*\(', 'filerw::ELChangeCompomentEnabled(', txt)
                    # golden 密碼框（MyMessageBox->DoPassword_MBox()）：伺服器端不能假裝驗過 —— 一律回 false（＝golden 輸錯），記 todo
                    txt = re.sub(r'\bMyMessageBox->DoPassword_MBox\s*\(\s*\)', 'filerw::ELPasswordRefused("DoPassword_MBox")', txt)
                parts.append(txt)
            line = ''.join(parts).rstrip()
            # 純 UI 外觀（字色）：HTML 端自己管，C++ 替身沒有 Font；原行保留成註解
            # 純 UI 外觀（字色／底色／分頁切換／圖片）：HTML 端自己管。整行換成空敘述「;」＋原文註解 ——
            # 不能只註解掉：它若是無大括號 if 的本體，下一行會變成 if 本體。
            if not line.lstrip().startswith('//') and (
               re.match(r'\s*[^;]*->(Font->Color|Color|ActivePage)\s*=[^;=][^;]*;\s*(//.*)?$', line) or
               re.match(r'\s*[^;]*->Picture->[^;]*;\s*(//.*)?$', line)):
                ind = line[:len(line) - len(line.lstrip())]
                line = ind + ';   // UI-only（HTML 端處理）：' + line.strip()
            out.append(line)
            for b in blocks.get(meth, []):
                if gl == b[2]:
                    out.append('#endif // GATE (S12-C save)')
        out.append('}')
        out.append('')
    # 存檔流程讀的替身（排除 #if 0 GATE 段；`EL<>(…)->屬性[…] = …`（不是 ==）算寫）
    reads, cur, gated = set(), None, False
    elre = re.compile(r'EL<[\w:]+>\("\w+", "(\w+)"\)->\w+((?:\[[^\]]*\])*)\s*(==|=)?')
    for line in out:
        m = re.match(r'static \w+ ' + P + r'_(\w+)\(', line)
        if m:
            cur = m.group(1)
        if line.startswith('#if 0 // GATE (S12-C save)'):
            gated = True
        elif line.startswith('#endif // GATE (S12-C save)'):
            gated = False
        if gated or cur not in st.get('save_methods', []) or line.lstrip().startswith('//'):
            continue
        for mm in elre.finditer(line):
            if mm.group(3) != '=':
                reads.add(mm.group(1))
    st['_save_reads'] = sorted(reads)
    miss = [b for bl in blocks.values() for b in bl if b not in hit_blocks]
    if miss:
        raise SystemExit('blocks not hit: %r' % miss)
    return out, W, used


def emit(st):
    lines, W, used = convert(st)
    P = st.get('prefix', 'IC')
    L = ['// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。',
         '// ---------------------------------------------------------------------------',
         '//  結構 %s 的讀寫檔（HTEditList：%s → %s）。' % (st['struct'], '、'.join(st['lists']), '、'.join(st['files'])),
         '//  來源：golden %s\\%s（cp950 → UTF-8）；widget 名稱與型別取自 golden %s。' % (GOLDEN, st['cpp'], st['h']),
         '//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。',
         '//  golden 程式用到的替身 %d 個（header 宣告 %d 個）。' % (len(used), len(W)),
         '//  索引：FileRW/README.md',
         '// ---------------------------------------------------------------------------',
         '#include "FileRW/_EditList.h"']
    L += ['#include "%s"' % i for i in st['includes']]
    L += [''] + st.get('decls', [])
    if st.get('members'):
        L += ['', '// golden %s 的非元件成員（本 TU 只有這一個表單，所以是 static）' % st['h']] + [(m if m.startswith('#') else 'static ' + m) for m in st['members']]
    if st.get('globals'):
        g = cp950(os.path.join(GOLDEN, st['cpp'])).replace('\r\n', '\n').split('\n')
        L += ['', '// golden %s 檔案層級全域（本表單私用，照抄成 static）' % st['cpp']]
        for name in st['globals']:
            hit = [(i, l) for i, l in enumerate(g) if re.match(r'^[A-Za-z]\w*\s+' + name + r'\s*(=[^;]*)?;', l)]
            if len(hit) != 1:
                raise SystemExit('global %s: %d hits' % (name, len(hit)))
            i, l = hit[0]
            L.append('static ' + l.rstrip() + '   // golden %s:%d' % (st['cpp'], i + 1))
    L += ['', 'using filerw::EL;', '', '// 前置宣告']
    for m in st['methods']:
        L.append('static %s %s_%s(%s);' % (st.get('rettype', {}).get(m, 'void'), P, m, st['_sig'][m]))
    L.append('')
    body_lines = lines   # 方法本體最後才放（表要先宣告）
    if st.get('adopt'):
        # 移植樹表單物件已有的同名同型別元件 → 直接當替身（ELKeep），golden 轉出來的程式與移植樹其他程式共用同一個物件
        ad = st['adopt']
        ph = open(os.path.join(ROOT, ad['header']), encoding='utf-8').read()
        port = {m.group(1): m.group(2) for m in re.finditer(r'\*\s*(\w+)\s*=\s*new\s+vclcompat::(T\w+)\s*\(\s*\)', ph)}
        hit = sorted(n for n in port if n in W and W[n] == port[n])
        skip = sorted(n for n in port if n in W and W[n] != port[n])
        L += ['// 移植樹 %s（%s）已有的同名同型別元件 %d 個：替身就是那個物件（開機時最先呼叫）' % (ad['object'], ad['header'], len(hit)),
              'static void %s_AdoptPortWidgets()' % P, '{']
        for n in hit:
            L.append('    filerw::ELKeep("%s", "%s", %s->%s);' % (st['class'], n, ad['object'], n))
        L += ['}', '']
        print('  adopt %s：%d 個共用（型別不同略過：%s）' % (ad['object'], len(hit), skip))
    # golden DFM 的設計期 Items：HTEditList::Add 以 Items->Count-1 當 ItemIndex 上限（CheckRange），
    # 替身沒有 Items 就會把檔案讀到的值夾成 0（實測 coI20 2→0）。開機時照 DFM 補上。
    dfm = dfm_items(cp950(os.path.join(GOLDEN, st['cpp'].replace('.cpp', '.dfm'))))
    L += ['// golden %s 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）' % st['cpp'].replace('.cpp', '.dfm'),
          'static void %s_DfmItems()' % P, '{']
    n_items = 0
    for name in sorted(dfm):
        if name not in W or W[name] not in ('TComboBox', 'TRadioGroup', 'TListBox'):
            continue
        L.append('    { TStringList* s = EL<%s>("%s", "%s")->Items;' % (W[name], st['class'], name))
        for v in dfm[name]:
            L.append('      s->Add(%s);' % cstr(v))
        L.append('    }')
        n_items += 1
    L += ['}', '']
    st['_dfm_items'] = n_items
    # 審查 H-A：權限。golden ChangeCompomentEnabled(容器,…) 停用整個容器；替身要知道自己的祖先才能判斷
    # 「可不可以改」。父子關係照 golden DFM；用到的替身的每一層祖先都預先建好（型別照 header）。
    dtypes, dprops = {}, {}
    par = dfm_parents(cp950(os.path.join(GOLDEN, st['cpp'].replace('.cpp', '.dfm'))), dtypes, dprops)
    # header 沒宣告、只在 DFM 裡的容器（例 tsD_60）也要進祖先鏈（審查 #3）：型別取 DFM 的
    WT = dict(W)
    for n, t in dtypes.items():
        if n not in WT and n != st['class'][1:] and t != st['class']:
            WT[n] = MAPPED.get(t) or (t if t in KNOWN else 'TControl')
    names_used = set(re.findall(r'EL<[\w:]+>\("%s", "(\w+)"\)' % st['class'], '\n'.join(lines)))
    anc = set()
    for n in names_used:
        p = par.get(n)
        while p and p in WT:
            anc.add(p)
            p = par.get(p)
    pairs = sorted((n, par[n]) for n in names_used | anc if n in par and par[n] in WT)
    L += ['// golden %s 的父子關係（替身 → 容器），權限判斷用' % st['cpp'].replace('.cpp', '.dfm'),
          'static const char* const k%s_ParentOf[][2] = {' % P]
    for a, b in pairs:
        L.append('    {"%s", "%s"},' % (a, b))
    L += ['};', '', '// 上面用到的容器替身先建好（型別照 golden header）', 'static void %s_CreateContainerProxies()' % P, '{']
    for n in sorted(anc):
        L.append('    EL<%s>("%s", "%s");' % (WT[n], st['class'], n))
    L += ['    filerw::ELSetParents("%s", k%s_ParentOf, (int)(sizeof(k%s_ParentOf) / sizeof(k%s_ParentOf[0])));' % (st['class'], P, P, P),
          '}', '']
    # 審查 #2：golden DFM 設計期 Enabled=False／ReadOnly=True／Visible=False（建構時就生效）
    allp = names_used | anc
    L += ['// golden %s 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）' % st['cpp'].replace('.cpp', '.dfm'),
          'static void %s_DfmState()' % P, '{']
    nst = 0
    for n in sorted(allp):
        pr = dprops.get(n, {})
        if pr.get('Enabled') == 'False':
            L.append('    EL<%s>("%s", "%s")->Enabled = false;' % (WT[n], st['class'], n)); nst += 1
        if pr.get('Visible') == 'False':
            L.append('    EL<%s>("%s", "%s")->Visible = false;' % (WT[n], st['class'], n)); nst += 1
        if pr.get('ReadOnly') == 'True':
            L.append('    filerw::ELSetReadOnly("%s", "%s");' % (st['class'], n)); nst += 1
    # 設計期的值（VCL 建構時從 DFM 載入）：TEdit／TComboBox 的 Text、TCheckBox／TRadioButton 的 Checked、
    # TComboBox／TRadioGroup 的 ItemIndex（Items 已由 DfmItems 先放好）。golden 有些值只靠它（例 cSpeed.cpp:765
    # iIndexSpeed=atoi(edtTrySpeed->Text) 讀的是 DFM 的 '900000'）。Steven 20260924。
    nval = 0
    for n in sorted(allp):
        t, pr = dtypes.get(n), dprops.get(n, {})
        if 'Text' in pr and t in ('TEdit', 'TLabeledEdit', 'TComboBox', 'TMemo', 'TMaskEdit'):
            L.append('    EL<%s>("%s", "%s")->Text = %s;' % (WT[n], st['class'], n, cstr(pr['Text']))); nval += 1
        if pr.get('Checked') == 'True' and t in ('TCheckBox', 'TRadioButton'):
            L.append('    EL<%s>("%s", "%s")->Checked = true;' % (WT[n], st['class'], n)); nval += 1
        if 'ItemIndex' in pr and t in ('TComboBox', 'TRadioGroup', 'TListBox'):
            L.append('    EL<%s>("%s", "%s")->ItemIndex = %s;' % (WT[n], st['class'], n, pr['ItemIndex'])); nval += 1
    print('  DFM 設計期值（Text／Checked／ItemIndex）：%d 筆' % nval)
    # TTrackBar／TUpDown 的設計期 Min／Max／Position（VCL 預設：TTrackBar 0/10/0、TUpDown 0/100/0）、
    # TUpDown 的 Associate、OnChange 事件（golden 設 ->Position 會觸發它）。DfmInit 不夾、不觸發。
    ntb = 0
    for n in sorted(allp):
        t = dtypes.get(n)
        if t not in ('TTrackBar', 'TUpDown'):
            continue
        pr = dprops.get(n, {})
        mx = pr.get('Max', '10' if t == 'TTrackBar' else '100')
        L.append('    EL<filerw::ELTrackBar>("%s", "%s")->DfmInit(%s, %s, %s);' %
                 (st['class'], n, pr.get('Min', '0'), mx, pr.get('Position', '0')))
        a = pr.get('Associate')
        if a and a in WT:
            L.append('    EL<filerw::ELTrackBar>("%s", "%s")->Associate = EL<%s>("%s", "%s");' % (st['class'], n, WT[a], st['class'], a))
        ev = pr.get('OnChange')
        if ev and ev in st['methods']:
            L.append('    EL<filerw::ELTrackBar>("%s", "%s")->OnChange = &%s_%s;' % (st['class'], n, P, ev))
        elif ev:
            L.append('    // golden DFM %s.OnChange = %s：這個方法沒有轉（不在 methods）' % (n, ev))
        ntb += 1
    L += ['}', '']
    print('  TTrackBar／TUpDown 設計期狀態：%d 個' % ntb)
    # 審查 #2：golden FormShow :4619-4623 ChangeCompomentEnabled(頁, true, true) 只動容器與這五種
    # （cAuthority.cpp:470-507）；TEdit／TComboBox／TDateTimePicker／TTrackBar 保留原本的 Enabled
    ENABLE_T = {'TPanel', 'TPageControl', 'TTabSheet', 'TTabControl', 'TForm', 'THeader', 'TPage', 'TGroupBox',
                'TScrollBox', 'TRadioGroup', 'TLabel', 'TSpeedButton', 'TButton', 'TBitBtn', 'TCheckBox', 'TRadioButton'}
    gt = {}
    gt.update({n: t for n, t in dtypes.items()})
    # 審查 M1（第 6 輪）：golden 只跑 pcConfig->Pages[i]（i 走遍 PageCount），且跳過 tsSearchFunction ——
    # 範圍是「pcConfig 的頁與頁底下」，不含 pcConfig 本身、頁外的元件、tsSearchFunction 整棵
    root, skip = st.get('enable_all_root'), set(st.get('enable_all_skip', ()))

    def in_scope(n):
        if not root:
            return True
        p = n
        for _ in range(64):
            if p in skip:
                return False
            if par.get(p) == root:
                return True
            p = par.get(p)
            if not p:
                return False
        return False
    en = sorted(n for n in allp if gt.get(n) in ENABLE_T and in_scope(n))
    L += ['// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）',
          'static const char* const k%s_EnableAll[] = {' % P]
    for i in range(0, len(en), 6):
        L.append('    ' + ' '.join('"%s",' % x for x in en[i:i + 6]))
    L += ['};', '']
    print('  DFM 設計期狀態：%d 筆；ChangeCompomentEnabled 會重開的：%d 個' % (nst, len(en)))
    print('  DFM 父子：%d 筆，容器替身 %d 個' % (len(pairs), len(anc)))
    if st.get('save_methods'):
        L += ['// 存檔流程（%s）讀的替身 %d 個 —— FileRW/%s.cpp 用它決定頁面必須送哪些值' %
              ('、'.join(st['save_methods']), len(st['_save_reads']), st['struct']),
              'static const char* const k%s_SaveReads[] = {' % P]
        for i in range(0, len(st['_save_reads']), 6):
            L.append('    ' + ' '.join('"%s",' % n for n in st['_save_reads'][i:i + 6]))
        L += ['};', '']
        # 審查 20260924 C1：這些替身只在存檔流程裡第一次 EL<>，頁面值套用時還不存在 → 值被丟掉、
        # 存檔用預設值。開機就照 golden header 的型別建好。
        L += ['// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）',
              'static void %s_CreateSaveProxies()' % P, '{']
        for n in st['_save_reads']:
            L.append('    EL<%s>("%s", "%s");' % (W[n], st['class'], n))
        L += ['}', '']
    L += body_lines
    path = os.path.join(OUT, st['struct'] + '.gen.inc')
    open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(L) + '\n')
    print('  DFM Items：%d 個元件' % st.get('_dfm_items', 0))
    print('✔ %-10s %d 個方法、用到 %d 個具名替身 → %s' % (st['struct'], len(st['methods']), len(used),
                                                    os.path.relpath(path, ROOT)))


ONLY = sys.argv[sys.argv.index('--only') + 1] if '--only' in sys.argv else None
if ONLY and ONLY not in [st['struct'] for st in STRUCTS]:
    raise SystemExit('--only %s: no such struct in tools/editlist/' % ONLY)
for st in STRUCTS:
    if ONLY and st['struct'] != ONLY:
        continue
    emit(st)
if ONLY:
    sys.exit(0)   # --only：不動 _editlist_sources.cmake（整合時不帶 --only 再跑一次）

# 審查第 8 輪 M-2：wb_serve 的 FileRW 原始檔改用「正面清單」（不 GLOB）—— 共用層＋每個結構的入口 cpp
# （入口是手寫的 FileRW/<struct>.cpp，#include 產生的 <struct>.gen.inc）。gen_formbridge.py 另寫 _formbridge_sources.cmake。
_C = ['# 產生檔 -- tools/gen_editlist.py。C 形狀（HTEditList／具名替身）的原始檔（相對於 HT9011UC_Cpp_V3.33.906.0），',
      '# CMakeLists.txt 的 wb_serve include 它。新增結構：在 STRUCTS 加一筆、寫 FileRW/<struct>.cpp、重跑本產生器。',
      'set(W906_EDITLIST_SRC',
      '    FileRW/_EditList.cpp',
      '    FileRW/_EditPage.cpp',
      '    FileRW/_KitSuck.cpp']
_INTEG = [l.strip() for l in open(os.path.join(EDITLIST_DIR, '_integrated.txt'), encoding='utf-8')
          if l.strip() and not l.startswith('#')]
_C += ['    FileRW/%s.cpp' % st['struct'] for st in STRUCTS if st['struct'] in _INTEG]   # 只收已整合的（_integrated.txt）
# AI(W906-W5-TEACH) 20260925：_integrated.txt 裡不在 STRUCTS 的名字 = 有自己產生器的手寫入口（例 Teach：FileRW/Teach.cpp＋tools/gen_teach_editlist.py）。
#   合併 v906/steven-cbridge-review6 時 CMakeLists 的手寫清單換成本產生檔，Teach.cpp 因此掉出 wb_serve（wb_serve.cpp 仍呼叫 FileRW_Teach_*，連結會失敗）。
_ST_NAMES = set(st['struct'] for st in STRUCTS)
_C += ['    FileRW/%s.cpp' % n for n in _INTEG if n not in _ST_NAMES and os.path.exists(os.path.join(OUT, n + '.cpp'))]
_C += [')', '']
open(os.path.join(OUT, '_editlist_sources.cmake'), 'w', encoding='utf-8', newline='\n').write('\n'.join(_C))
for st in STRUCTS:
    if st['struct'] not in _INTEG:
        print('  （%s 尚未整合：不進 _editlist_sources.cmake）' % st['struct'])
        continue
    if not os.path.exists(os.path.join(OUT, st['struct'] + '.cpp')):
        print('⚠ FileRW/%s.cpp（入口）還沒寫 —— _editlist_sources.cmake 已列它，建置會失敗' % st['struct'])
