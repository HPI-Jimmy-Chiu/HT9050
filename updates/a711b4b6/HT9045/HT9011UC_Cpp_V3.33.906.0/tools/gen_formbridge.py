# -*- coding: utf-8 -*-
# Steven 20260924
# ----------------------------------------------------------------------
# S12 第二型新檔。直接讀 golden BCB 原檔（cp950），把表單的 DoIniDataToForm()、
# SaveSetupFile() 與它們呼叫到的事件處理器，機械改寫成 JSON bridge：
#     widget->Prop = 右式;   ->  J.SetProp("widget", 右式);
#     widget->Prop（讀）     ->  J.GetProp("widget")
# 控制流程、右式、WriteIniData 原樣保留。輸出 JsonBridge/gen/bridge_<Class>.gen.cpp。
# 規格：.claude/skills/ht9045-json-bridge/references/decisions.md 二之三、phases.md S12
# ----------------------------------------------------------------------
"""gen_formbridge.py -- 由 golden 原檔產生表單 JSON bridge。

使用者 20260924：「DoIniDataToForm 不需要翻譯吧？直接使用 BCB 的原檔，設計成
JSON bridge 就好了」。所以本產生器不讀移植樹的表單，只讀 golden：
  * widget 名單與型別 ＝ golden 的 <form>.h（例：cTesterIF.h 240 個）
  * 函式本體       ＝ golden 的 <form>.cpp

改寫規則（只動程式碼，註解原樣保留並轉成 UTF-8）：
  1. W->Items->Strings[i]  -> J.Item("W", i)       W->Items->Count -> J.ItemCount("W")
     W->Items->Add(x)      -> J.ItemsAdd("W", x)   W->Clear()／W->Items->Clear() -> J.ItemsClear("W")
  2. W->Prop = 右式;       -> J.SetProp("W", 右式);（右式裡的讀值先照 3 改寫）
  3. W->Prop               -> J.GetProp("W")
  4. 自己的方法 m(this…)    -> B_m(J, …)（只限 METHODS 裡列的；TObject* Sender 參數拿掉）
  5. 表單成員 fShow／bflag  -> J.M("fShow")
  改寫完若還有 `widget->` 殘留，產生器**中止**並列出行號 —— 由人在 OVERRIDES 裡決定，不猜。

  ⚠ 語句切分以「行」為單位（審查更正 20260924：原本寫「跨行時整條用括號配對收齊」，實作沒有這樣做）。
    一條賦值的右式跨行時，改寫出來的程式碼編譯不過 —— 由編譯器擋下，再用 overrides 處理。

  AI(W906-FRW-S157) 20260927 [W906]：FORM['events'] ＝ WS form.event 的事件表（Steven ★ Q40＝A，RULINGS_20260926 S157）——
    [(控制項, 'change'|'click', golden 處理器)]，處理器照 methods 同樣轉換；另產生 golden DFM 的點得到判斷鏈（dfm_chain）。

用法：python tools/gen_formbridge.py      （在 HT9011UC_Cpp_V3.33.906.0 底下跑）
"""
import io
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
# 使用者 20260924：「在 WriteFile 資料夾下用不同的 cpp，方便以後查詢」「最好是一個結構一個 cpp」
# → 同日改名 FileRW（「把相同結構的 Read Write 整合到同一個 cpp 裡面」「WriteFile 改成 FileRW」）
# 「對應到 BCB 不同的 form，可以分 function」。
WRITEFILE = os.path.join(ROOT, 'FileRW')
GOLDEN = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'

PROPS = {'Text': 'Text', 'ItemIndex': 'ItemIndex', 'Checked': 'Checked', 'Caption': 'Caption',
         'Visible': 'Visible', 'Enabled': 'Enabled', 'TabVisible': 'TabVisible',
         'ActivePageIndex': 'ActivePageIndex', 'Down': 'Down'}

# ----------------------------------------------------------------------
# 每個表單的設定：tools/formbridge/<Class>.py（一個 BCB 表單一個檔，檔內 FORM = {...}）。
# Steven 20260924：拆檔，讓不同工程師各改各的表單設定、不互相衝突。欄位說明見 tools/formbridge/README.md。
# ----------------------------------------------------------------------
FORMS_DIR = os.path.join(HERE, 'formbridge')


def load_forms():
    # --only 時別人的設定檔壞了（作業中）不擋自己：跳過並警告；目標表單自己的檔壞了才中止（工程師回報 20260924）
    only = sys.argv[sys.argv.index('--only') + 1] if '--only' in sys.argv else None
    forms = []
    for fn in sorted(os.listdir(FORMS_DIR)):
        if not fn.endswith('.py') or fn.startswith('_'):
            continue
        ns = {}
        try:
            exec(compile(open(os.path.join(FORMS_DIR, fn), encoding='utf-8').read(), fn, 'exec'), ns)
        except Exception as e:
            if only and fn != only + '.py':
                print('⚠ 跳過 %s（--only %s；這個設定檔目前載入失敗：%s）' % (fn, only, e))
                continue
            raise
        f = ns['FORM']
        if f['class'] + '.py' != fn:
            raise SystemExit('%s: FORM class %s does not match the file name' % (fn, f['class']))
        forms.append(f)
    return forms


FORMS = load_forms()

def cp950(path):
    return open(path, 'rb').read().decode('cp950', errors='replace')


def split_comment(line):
    """回傳 (code, comment)。comment 含 //。字串裡的 // 不算。"""
    i, n, q = 0, len(line), None
    while i < n:
        c = line[i]
        if q:
            if c == '\\':
                i += 2; continue
            if c == q:
                q = None
        elif c in '"\'':
            q = c
        elif line.startswith('//', i):
            return line[:i], line[i:]
        i += 1
    return line, ''


def widgets_of(h_text, cls):
    m = re.search(r'\bclass\s+(?:PACKAGE\s+)?' + cls + r'\b[^;{]*\{', h_text)
    body = h_text[m.end():]
    out = {}
    for mm in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', body, re.M):
        out[mm.group(2)] = mm.group(1)
    return out


def method_body(cpp, cls, name):
    # 宣告行尾常帶註解，`{` 在下一行：`void __fastcall TFTestIF::InitcbDIOType(bool bAlarm)   //Steven …`
    m = re.search(r'^[^\n]*\b' + cls + r'::' + name + r'\s*\(([^)]*)\)\s*(?://[^\n]*\s*)*\{', cpp, re.M)
    if not m:
        raise SystemExit('golden %s::%s not found' % (cls, name))
    i, d = m.end(), 1
    q = None
    while d and i < len(cpp):
        c = cpp[i]
        if q:
            if c == '\\':
                i += 1
            elif c == q:
                q = None
        elif cpp.startswith('//', i):
            j = cpp.find('\n', i); i = j; continue
        elif c in '"\'':
            q = c
        elif c == '{':
            d += 1
        elif c == '}':
            d -= 1
        i += 1
    # 本體第 0 行 ＝ `{` 所在那一行（宣告行尾有註解時 `{` 在下一行）
    brace_line = cpp.count('\n', 0, m.end()) + 1
    return m.group(1), cpp[m.end():i - 1], brace_line


def dfm_chain(dfm_text, name):
    """AI(W906-FRW-S157) 20260927 [W906]：WS form.event —— golden DFM 裡 name 自己＋每一層容器（由內而外，不含表單本身），
    各帶設計期 Enabled／Visible（DFM 只記與預設 True 不同的值）。RunEvent 跑完 display 後照這條鏈判斷 golden 使用者點不點得到。"""
    par, props, stack, root = {}, {}, [], None
    for ln in dfm_text.replace('\r\n', '\n').split('\n'):
        t = ln.strip()
        m = re.match(r'(?:object|inherited|inline)\s+(\w+)\s*:\s*(\w+)', t)
        if m:
            if stack:
                par[m.group(1)] = next((x for x in reversed(stack) if x), None)
            else:
                root = m.group(1)
            stack.append(m.group(1))
        elif t == 'item':               # TCollection 的項目（Panels = < item … end >）也用 end 收尾
            stack.append(None)
        elif t == 'end' and stack:
            stack.pop()
        elif stack and stack[-1]:
            pm = re.match(r'(Enabled|Visible)\s*=\s*(True|False)$', t)
            if pm:
                props.setdefault(stack[-1], {})[pm.group(1)] = pm.group(2)
    if name not in par:
        raise SystemExit('golden DFM has no component %s' % name)
    chain, n = [], name
    while n and n != root:
        pr = props.get(n, {})
        chain.append((n, pr.get('Enabled') != 'False', pr.get('Visible') != 'False'))
        n = par.get(n)
    return chain


def rewrite_generic(code, form):
    """Steven 20260924：各表單共通、不必每個寫 override 的 golden 寫法。"""
    # golden ShowMyMessage(en[, zh]) 在伺服器端不彈窗，改進 JSON messages
    code = re.sub(r'\bShowMyMessage\s*\(', 'J.Message(', code)
    # 表單自己的 Close()：標記 closed，由頁面決定
    code = re.sub(r'^(\s*)Close\s*\(\s*\)\s*;', r'\1J.M("closed")=1;   /* golden: Close() */', code)
    # 表單本身的視窗屬性（位置／標題），HTML 不用
    code = re.sub(r'^(\s*)((?:Top|Left|Width|Height|Caption)\s*=[^;]*;)', r'\1/* golden（視窗屬性，HTML 不用）: \2 */', code)
    # 高級審查員第二輪（低）：fMain->BackupSetupFile() 在移植樹是空函式（forms/fMain.cpp:458），照叫但照實標 Todo
    code = re.sub(r'\bfMain->BackupSetupFile\s*\(\s*\)\s*;',
                  'fMain->BackupSetupFile(); J.Todo("fMain->BackupSetupFile() is a no-op in the port (forms/fMain.cpp:458) -- no auto backup");',
                  code)
    # golden 呼叫移植樹已翻好的方法（例：讀檔器 ReadFile）→ 接到移植樹實例
    for name, repl in form.get('port_calls', {}).items():
        code = re.sub(r'(?<![\w>.:])%s\s*\(' % name, repl + '(', code)
    return code


def rewrite_reads(code, W, methods, members):
    # 1. Items
    code = re.sub(r'\b(%s)->Items->Strings\[([^\]]+)\]' % W, r'J.Item("\1", \2)', code)
    code = re.sub(r'\b(%s)->Items->Count\b' % W, r'J.ItemCount("\1")', code)
    code = re.sub(r'\b(%s)->Items->Add\(' % W, r'J.ItemsAdd("\1", ', code)
    code = re.sub(r'\b(%s)->(?:Items->)?Clear\(\)' % W, r'J.ItemsClear("\1")', code)
    # 3. 讀值
    code = re.sub(r'\b(%s)->(%s)\b' % (W, '|'.join(PROPS)), lambda m: 'J.Get%s("%s")' % (m.group(2), m.group(1)), code)
    # 4. 自己的方法
    for meth in methods:
        code = re.sub(r'(?<![\w>.:])%s\s*\(\s*\)' % meth, 'B_%s(J)' % meth, code)      # m()  （無參數）
        code = re.sub(r'\b%s\s*\(\s*this\s*\)' % meth, 'B_%s(J)' % meth, code)
        code = re.sub(r'\b%s\s*\(\s*(?!this\b)' % meth, 'B_%s(J, ' % meth, code)
    # 5. 成員
    for mb in members:
        code = re.sub(r'(?<![\w."])%s\b' % mb, 'J.M("%s")' % mb, code)
    return code


def rewrite_assign(code, W, methods, members):
    """W->Prop = 右式;  ->  J.SetProp("W", 右式);   右式先改寫讀值。一行可有多條。"""
    out, pos = [], 0
    pat = re.compile(r'\b(%s)->(%s)\s*=(?!=)' % (W, '|'.join(PROPS)))
    while True:
        m = pat.search(code, pos)
        if not m:
            out.append(rewrite_reads(code[pos:], W, methods, members)); break
        out.append(rewrite_reads(code[pos:m.start()], W, methods, members))
        j, d = m.end(), 0
        while j < len(code) and not (code[j] == ';' and d == 0):
            d += {'(': 1, ')': -1, '[': 1, ']': -1}.get(code[j], 0); j += 1
        rhs = rewrite_reads(code[m.end():j].strip(), W, methods, members)
        prop = m.group(2)
        if prop in ('Text', 'Caption'):
            rhs = 'AnsiString(%s)' % rhs
        out.append('J.Set%s("%s", %s);' % (prop, m.group(1), rhs))
        pos = j + 1
    return ''.join(out)


def convert(form):
    cpp = cp950(os.path.join(GOLDEN, form['cpp']))
    h = cp950(os.path.join(GOLDEN, form['h']))
    widgets = widgets_of(h, form['class'])
    W = '|'.join(sorted(widgets, key=len, reverse=True))
    ov = {}
    for meth, old, new in form['overrides']:
        ov.setdefault(meth, []).append([old, new, 0])
    blk = {}
    for meth, a, b, must, new in form.get('blocks', []):
        blk.setdefault(meth, []).append([a, b, must, new, 0])
    funcs, fails = [], []
    for meth in form['methods']:
        params, body, line0 = method_body(cpp, form['class'], meth)
        ps = [p.strip() for p in params.split(',') if p.strip() and not re.match(r'TObject\s*\*', p.strip())]
        sig = 'static void B_%s(FormState& J%s)' % (meth, ''.join(', ' + p for p in ps))
        lines = ['// golden %s:%d  %s::%s(%s)' % (form['cpp'], line0, form['class'], meth, params.strip()),
                 sig, '{']
        if 'B_' + meth == form.get('save'):
            # 高級審查員第二輪（中）：golden 存檔鈕有提早 return、不寫檔的路徑（權限不足、兩個 HotPlate 都沒勾…）。
            # 進到存檔函式才算「有寫」，ack 的 saved 據此，頁面不可一律顯示「已寫入」。
            lines.append('    J.M("saved")=1;   // bridge：golden 存檔函式有被呼叫（form.save 的 ack.saved）')
        skip_to = 0
        for k, raw in enumerate(body.split('\n')):
            ln = line0 + k
            if ln <= skip_to:
                continue
            b = next((x for x in blk.get(meth, []) if x[0] == ln), None)
            if b:
                if b[2] not in raw:
                    fails.append('block %s:%d does not start with %r (golden moved?)' % (form['cpp'], ln, b[2]))
                b[4] += 1
                lines.append('    // ---- golden %s:%d-%d 整段覆寫（見 gen_formbridge.py FORMS.blocks）----'
                             % (form['cpp'], b[0], b[1]))
                lines.append('    ' + b[3])
                skip_to = b[1]
                continue
            code, com = split_comment(raw.rstrip('\r'))
            hit = None
            for o in ov.get(meth, []):
                if o[0] in code:
                    hit = o; break
            if hit:
                hit[2] += 1
                ind = re.match(r'\s*', code).group(0)
                new = code.replace(hit[0], hit[1].replace('\n', '\n' + ind[:-4] if len(ind) >= 4 else '\n'))
            else:
                new = rewrite_assign(rewrite_generic(code, form), W, form['methods'], form['members'])
            left = re.findall(r'\b(%s)->' % W, new)
            if left:
                fails.append('%s:%d  %s' % (form['cpp'], ln, raw.strip()))
            lines.append((new + com).rstrip())
        lines.append('}')
        funcs.append((meth, sig, lines))
    unused = [(m, o[0]) for m, lst in ov.items() for o in lst if o[2] == 0]
    unused += [(m, 'block %d-%d' % (x[0], x[1])) for m, lst in blk.items() for x in lst if x[4] == 0]
    if unused:
        fails += ['override not used: %s: %s' % u for u in unused]
    if fails:
        print('✘ %s：%d 處無法機械改寫，請加 OVERRIDES：' % (form['class'], len(fails)))
        for f in fails:
            print('   ' + f)
        raise SystemExit(1)
    return funcs, widgets


def emit_form(form):
    """一個 BCB 表單 → 一段程式（包在 namespace f_<Class> 裡）。回傳 (includes, lines, meta)。"""
    funcs, widgets = convert(form)
    cls = form['class']
    L = ['// ===========================================================================',
         '//  BCB 表單 %s  ——  golden %s（widget 名單取自 golden %s，%d 個）'
         % (cls, form['cpp'], form['h'], len(widgets)),
         '//  頁面 %s' % form['page'],
         '//  寫檔 %s' % '、'.join(form.get('files', [])),
         '// ===========================================================================',
         'namespace f_%s {' % cls, '', '// 前置宣告（golden 的方法之間互相呼叫）']
    L += [sig + ';' for _m, sig, _l in funcs]
    L.append('')
    for _m, _sig, lines in funcs:
        L += lines + ['']
    L += ['static void Display(FormState& J)', '{']
    L += ['    ' + s for s in form['display']]
    L += ['}', '']
    reads = []                              # SaveSetupFile 讀到的 widget：頁面少送一個就整筆拒寫
    for m, _sig, lines in funcs:
        if 'B_' + m == form.get('save'):
            for ln in lines:
                for r in re.findall(r'J\.Get\w+\("(\w+)"\)', ln):
                    if r not in reads:
                        reads.append(r)
    save = flow = 'nullptr'
    if form.get('save'):
        L += ['static void Save(FormState& J, AnsiString szDir, AnsiString S)', '{',
              '    %s(J, szDir, S);' % form['save'], '}', '']
        save = '&f_%s::Save' % cls
    if form.get('saveFlow'):
        L += ['static void SaveFlow(FormState& J)', '{', '    %s(J);' % form['saveFlow'], '}', '']
        flow = '&f_%s::SaveFlow' % cls
    # AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（FORM['events'] = [(控制項, 'change'|'click', golden 處理器), …]，
    #   處理器要在 methods 裡、除了 Sender 沒有別的參數）。chain 見 dfm_chain()。
    events, ev_ref = form.get('events', []), 'nullptr, 0'
    if events:
        cpp_text = cp950(os.path.join(GOLDEN, form['cpp']))
        dfm_text = cp950(os.path.join(GOLDEN, form['cpp'][:-4] + '.dfm'))
        sigs = {m: sig for m, sig, _l in funcs}
        L += ['// AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（tools/formbridge/%s.py 的 events）。' % cls,
              '//   chain＝golden DFM 的控制項自己＋每一層容器與設計期 Enabled／Visible（RunEvent 判斷使用者點不點得到）']
        rows = []
        for i, (ctl, ev, meth) in enumerate(events):
            if ctl not in widgets:
                raise SystemExit('%s: events: %s is not a widget in golden %s' % (cls, ctl, form['h']))
            if ev not in ('change', 'click'):
                raise SystemExit('%s: events: %s event must be change or click, not %r' % (cls, ctl, ev))
            if sigs.get(meth) != 'static void B_%s(FormState& J)' % meth:
                raise SystemExit('%s: events: %s must be in methods and take no parameter besides Sender' % (cls, meth))
            ch = dfm_chain(dfm_text, ctl)
            L.append('static const EventGuard kGuard%d[] = {%s};' % (i, ', '.join(
                '{"%s", %s, %s}' % (n, 'true' if en else 'false', 'true' if vis else 'false') for n, en, vis in ch)))
            m = re.search(r'^[^\n]*\b' + cls + r'::' + meth + r'\s*\(', cpp_text, re.M)
            where = '%s:%d %s::%s' % (form['cpp'].replace('\\', '/'), cpp_text.count('\n', 0, m.start()) + 1, cls, meth)
            rows.append('    {"%s", "%s", "%s", &B_%s, kGuard%d, %d},' % (ctl, ev, where, meth, i, len(ch)))
        L += ['static const EventDesc kEvents[] = {'] + rows + ['};', '']
        ev_ref = 'f_%s::kEvents, (int)(sizeof(f_%s::kEvents) / sizeof(f_%s::kEvents[0]))' % (cls, cls, cls)
    L += ['}  // namespace f_%s' % cls, '',
          'extern const BridgeDesc kBridge_%s = {' % cls,
          '    "%s", "%s", "%s",' % (form['page'], cls, form['cpp']),
          '    &f_%s::Display, %s, %s,' % (cls, save, flow),
          '    // SaveSetupFile 會讀到的 widget（%d 個）：頁面少送任何一個就整筆拒寫' % len(reads),
          '    "%s",' % ','.join(reads),
          '    "%s",' % form.get('sourceGap', '').replace('"', '\\"'),
          '    // AI(W906-FRW-S157) 20260927 [W906]：WS form.event 事件表（%d 個）＋golden header 的元件名' % len(events),
          '    %s,' % ev_ref,
          '    "%s"' % ','.join(sorted(widgets)),
          '};', '']
    meta = {'class': cls, 'page': form['page'], 'golden': form['cpp'], 'files': form.get('files', []),
            'struct': form['struct'], 'also': form.get('also', []), 'methods': [m for m, _s, _l in funcs],
            'reads': len(reads), 'gap': form.get('sourceGap', ''), 'saveFlow': bool(form.get('saveFlow'))}
    print('✔ %-18s %d 個方法 → FileRW/%s.cpp' % (cls, len(funcs), form['struct']))
    return form['includes'], L, meta


def main():
    # --only <Class|struct>：只重產那一個結構的 cpp（其餘表單設定不轉、_registry.cpp／README 不動）。
    # 多位工程師同時改不同表單時用；最後由整合的人不帶 --only 跑一次。
    only = sys.argv[sys.argv.index('--only') + 1] if '--only' in sys.argv else None
    os.makedirs(WRITEFILE, exist_ok=True)
    by_struct = {}
    for f in FORMS:
        by_struct.setdefault(f['struct'], []).append(f)
    if only:
        st = next((f['struct'] for f in FORMS if only in (f['class'], f['struct'])), None)
        if not st:
            raise SystemExit('--only %s: no such form class / struct in tools/formbridge/' % only)
        by_struct = {st: by_struct[st]}
    metas = []
    for st, forms in sorted(by_struct.items()):
        incs, body = [], []
        for f in forms:
            inc, lines, meta = emit_form(f)
            for i in inc:
                if i not in incs:
                    incs.append(i)
            body += lines
            metas.append(meta)
        H = ['// 產生檔 -- tools/gen_formbridge.py（Steven 20260924，S12 第二型）。不要手改：改產生器的 FORMS 後重跑。',
             '// ---------------------------------------------------------------------------',
             '//  結構 %s 的寫檔 bridge。一個結構一支 cpp，裡面每個 BCB 表單一組函式（namespace f_<Class>）。' % st,
             '//  來源一律是 golden %s 的原檔（cp950 → UTF-8），不看移植樹的表單。' % GOLDEN,
             '//  規則：widget->Prop = v  ->  J.SetProp("widget", v)；讀值 -> J.GetProp("widget")；',
             '//        控制流程、右式、WriteIniData 原樣保留。',
             '//  索引：FileRW/README.md',
             '// ---------------------------------------------------------------------------',
             '#include "JsonBridge/FormBridge.h"']
        H += ['#include %s' % (i if i.startswith('<') else '"%s"' % i) for i in incs]
        H += ['', 'namespace ht9045 {', 'namespace formbridge {', '']
        T = ['}  // namespace formbridge', '}  // namespace ht9045', '']
        open(os.path.join(WRITEFILE, '%s.cpp' % st), 'w', encoding='utf-8', newline='\n').write('\n'.join(H + body + T))

    if only:
        return
    R = ['// 產生檔 -- tools/gen_formbridge.py（Steven 20260924，S12 第二型）。頁面 → bridge 登錄表。',
         '#include "JsonBridge/FormBridge.h"', '', 'namespace ht9045 {', 'namespace formbridge {', '']
    R += ['extern const BridgeDesc kBridge_%s;' % m['class'] for m in metas]
    R += ['', 'extern const BridgeDesc* const kBridges[] = {'] + ['    &kBridge_%s,' % m['class'] for m in metas]
    R += ['};', 'extern const std::size_t kBridgeCount = %d;' % len(metas), '',
          '}  // namespace formbridge', '}  // namespace ht9045', '']
    open(os.path.join(WRITEFILE, '_registry.cpp'), 'w', encoding='utf-8', newline='\n').write('\n'.join(R))
    # tests/CMakeLists.txt 的 test_formbridge_* 要連進所有 bridge（_registry.cpp 參照每一個 kBridge_*）
    C = ['# 產生檔 -- tools/gen_formbridge.py。golden 表單 bridge 的原始檔（相對於 HT9011UC_Cpp_V3.33.906.0），',
         '# tests/CMakeLists.txt 與 wb_serve（CMakeLists.txt）都 include 它（審查第 8 輪 M-2：正面清單，不 GLOB）。',
         'set(W906_FORMBRIDGE_SRC', '    FileRW/_registry.cpp']
    C += ['    FileRW/%s.cpp' % st for st in sorted(by_struct)]
    C += [')', '']
    open(os.path.join(WRITEFILE, '_formbridge_sources.cmake'), 'w', encoding='utf-8', newline='\n').write('\n'.join(C))

    # README：以結構為主軸的索引（使用者 20260924：「方便以後查詢」）
    D = ['# FileRW —— 讀寫檔 bridge 索引（一個結構一支 cpp，讀與寫在同一支）',
         '',
         '> 產生檔（`tools/gen_formbridge.py`，設定在 `tools/formbridge/<Class>.py`），不要手改。一個結構一支 cpp；每支裡面每個 BCB 表單一組函式',
         '> （`namespace f_<Class>`）。來源一律是 golden BCB 原檔，不看移植樹的表單。',
         '> 規格：`.claude/skills/ht9045-json-bridge/references/phases.md` S12、`decisions.md` 二之三。',
         '',
         '| 結構（檔案） | BCB 表單 | golden | 頁面 | 寫哪些檔 | 也寫到的結構 | 存檔會讀的 widget | 可存檔 | 讀檔端缺口 |',
         '|---|---|---|---|---|---|---|---|---|']
    for m in metas:
        D.append('| `%s.cpp` | `%s` | `%s` | `%s` | %s | %s | %d | %s | %s |' % (
            m['struct'], m['class'], m['golden'], m['page'], '、'.join('`%s`' % x for x in m['files']) or '—',
            '、'.join('`%s`' % x for x in m['also']) or '—', m['reads'],
            '✔' if (m['saveFlow'] and not m['gap']) else '✘', m['gap'] or '—'))
    D += ['', '每個表單轉了哪些 golden 方法：', '']
    for m in metas:
        D.append('- `%s`（`%s.cpp`）：%s' % (m['class'], m['struct'], '、'.join('`%s`' % x for x in m['methods'])))
    D.append('')
    open(os.path.join(WRITEFILE, 'README.md'), 'w', encoding='utf-8', newline='\n').write('\n'.join(D))


main()
