# -*- coding: utf-8 -*-
# AI(W906-W5-TEACH) 20260925
# ----------------------------------------------------------------------
# 教導頁 C 路（Steven S12 golden 表單橋）的產生器 —— 輸出 FileRW/Teach.gen.inc。
# 規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md 一之二（具名替身）、§四
#   「Teach | teach.ini | D（TECH_* SaveToFile）＋C（elTeach）… → Teach.cpp」。
#
# 為什麼不是直接加進 tools/gen_editlist.py：那支的讀方向靠「表單方法 → IC_ 方法」整段轉，
#   而教導頁的主資料是 TECH_PARA 物件（forms/fTeachRegistry.cpp，TEACH-W1 已由 gen_teach_registry.py 產生、
#   ReadFile 已活），不是 HTEditList。這支只補兩件 gen_editlist.py 的模式涵蓋不到的：
#   1. golden TfTeach::InitialTeachEditList（uteach.cpp:3131-3358，203 筆 elTeach->Add，全部無條件）
#      —— 第一個引數（元件）改寫成具名替身 filerw::EL<TEdit>("TfTeach","名稱")，其餘逐字不動；
#   2. TECH_PARA／TECH_TWOPARA 的「Key → 畫面元件名稱」對照 —— 從 fTeachRegistry.cpp 的
#      `/*元件*/0` 註解取（移植樹的 SetEdit 是 NULL，元件名只剩那個註解），TECH_SUCKPARA 取 golden :285-292 的陣列。
#
# golden 一律讀 V906 的 golden 樹（與 fTeachRegistry.cpp 同源），cp950 → UTF-8。
# 用法：python tools/gen_teach_editlist.py           （在 HT9011UC_Cpp_V3.33.906.0 底下跑；寫兩個 .gen.inc）
#       python tools/gen_teach_editlist.py --check   （不寫檔：重跑產生器、與 golden 交叉比對，
#                                                     和已 commit 的兩個 .gen.inc 逐字比（換行不算）；不一致 exit 1 —— ctest TeachButtonsGen）
# AI(W906-W5-b) 20260925: 第 4 段（WebTeachButtons.gen.inc）改寫 —— 見第 4 段的說明。
# ----------------------------------------------------------------------
import io
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
CHECK = '--check' in sys.argv[1:]
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
GOLDEN_SHOWN = r'D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618';  GOLDEN = os.environ.get('HT9045_GOLDEN_ROOT') or GOLDEN_SHOWN   # AI(W906-J4) 20260930: Jerry J-4 -- same variable as tools/dfm2rc/dfm_parse.py _golden_root_candidates(); the generated files keep printing GOLDEN_SHOWN (a different path must not make TeachButtonsGen --check red)
OUT = os.path.join(ROOT, 'FileRW', 'Teach.gen.inc')
REG = os.path.join(ROOT, 'forms', 'fTeachRegistry.cpp')

cpp = open(os.path.join(GOLDEN, 'uteach.cpp'), 'rb').read().decode('cp950', 'replace').replace('\r\n', '\n').split('\n')
hdr = open(os.path.join(GOLDEN, 'uteach.h'), 'rb').read().decode('cp950', 'replace')
decl = {m.group(2): m.group(1) for m in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', hdr, re.M)}

# ---- 1. InitialTeachEditList（golden :3131-3358）
start = next(i for i, l in enumerate(cpp) if l.startswith('void TfTeach::InitialTeachEditList()'))
end = next(i for i in range(start + 1, len(cpp)) if cpp[i].startswith('}'))
assert start + 1 == 3131 and end + 1 == 3358, ('golden line check failed', start + 1, end + 1)
body = cpp[start + 2:end]                      # 去掉宣告與 {，保留到 } 前
out_body = []
n_add = 0
for ln, l in enumerate(body, start + 3):
    m = re.match(r'^(\s*)elTeach->Add\((\w+)(\s*,.*)$', l)
    if m:
        w = m.group(2)
        t = decl.get(w)
        assert t == 'TEdit', ('non-TEdit widget', w, t)
        l = '%selTeach->Add(filerw::EL<TEdit>("TfTeach", "%s")%s' % (m.group(1), w, m.group(3))
        n_add += 1
    out_body.append(l.rstrip() + ('' if not l.strip() else ''))
assert n_add == 203, n_add

# ---- 2. TECH_PARA／TECH_TWOPARA 元件對照（fTeachRegistry.cpp 的註解）
reg = open(REG, 'rb').read().decode('utf-8')
# 元件引數兩種寫法：`/*元件*/0`（活的列）或裸識別字（在 #if 0 裡的列，例 setEditLoadPortBufferZ）—— 都收，多的對照不影響執行
tp = [(a or b, k) for a, b, k in re.findall(r'TechPara\.push_back\(new TECH_PARA\([^;]*?(?:/\*(\w+)\*/0|,\s*(\w+))\s*,\s*"([^"]+)"', reg)]
tw = re.findall(r'TechTwoPara\.push_back\(new TECH_TWOPARA\([^;]*?/\*(\w+)\*/0\s*,\s*/\*(\w+)\*/0\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"', reg)
n_tp_reg = reg.count('TechPara.push_back(new TECH_PARA(')
n_tw_reg = reg.count('TechTwoPara.push_back(new TECH_TWOPARA(')
assert len(tp) == n_tp_reg, ('TECH_PARA parse', len(tp), n_tp_reg)
assert len(tw) == n_tw_reg, ('TECH_TWOPARA parse', len(tw), n_tw_reg)
pairs = {}
for w, k in tp:
    pairs.setdefault(k, w)
for w1, w2, k1, k2 in tw:
    pairs.setdefault(k1, w1)
    pairs.setdefault(k2, w2)

# ---- 3. TECH_SUCKPARA（golden :285-292）
def arr(name):
    i = next(i for i, l in enumerate(cpp) if re.search(r'TEdit\s*\*\s*%s\s*\[' % name, l))
    strip = lambda s: re.sub(r'//.*$', '', s)          # 行尾註解（例 teSortArm 的 //RogerYang ...）不是元件名
    txt = strip(cpp[i])
    j = i
    while '};' not in txt:
        j += 1
        txt += strip(cpp[j])
    return re.findall(r'\b(\w+)\b', txt.split('=', 1)[1]), i + 1
te_in, ln_in = arr('teInArm')
te_out, ln_out = arr('teOutArm')
te_sort, ln_sort = arr('teSortArm')
assert len(te_in) == 16 and len(te_out) == 16 and len(te_sort) == 2, (len(te_in), len(te_out), len(te_sort))

lines = []
lines.append('// ===========================================================================')
lines.append('//  FileRW/Teach.gen.inc  --  產生的，不要手改（tools/gen_teach_editlist.py）。')
lines.append('//  golden：%s\\uteach.cpp（cp950 → UTF-8）' % GOLDEN_SHOWN)
lines.append('//  1. IC_InitialTeachEditList = golden TfTeach::InitialTeachEditList（:3131-3358），203 筆 elTeach->Add，')
lines.append('//     元件引數改寫成具名替身 filerw::EL<TEdit>("TfTeach", 名稱)，其餘逐字。')
lines.append('//  2. kTeachKeyWidget：TECH_PARA %d 筆＋TECH_TWOPARA %d 筆的 Key → 畫面元件名稱（fTeachRegistry.cpp 的註解）。' % (len(tp), len(tw)))
lines.append('//  3. kTeInArm／kTeOutArm／kTeSortArm：TECH_SUCKPARA 的元件（golden :%d／:%d／:%d）。' % (ln_in, ln_out, ln_sort))
lines.append('// ===========================================================================')
lines.append('static void IC_InitialTeachEditList()   // golden uteach.cpp:3131')
lines.append('{')
lines.extend(out_body)
lines.append('}')
lines.append('')
lines.append('static const char* const kTeachKeyWidget[][2] = {   // {Key, 元件名稱}')
for k in sorted(pairs):
    lines.append('    { "%s", "%s" },' % (k, pairs[k]))
lines.append('};')
lines.append('static const int kTeachKeyWidgetCount = (int)(sizeof(kTeachKeyWidget) / sizeof(kTeachKeyWidget[0]));')
lines.append('')
lines.append('static const char* const kTeInArm[2][8]  = {{ %s },' % ', '.join('"%s"' % x for x in te_in[:8]))
lines.append('                                            { %s }};' % ', '.join('"%s"' % x for x in te_in[8:]))
lines.append('static const char* const kTeOutArm[2][8] = {{ %s },' % ', '.join('"%s"' % x for x in te_out[:8]))
lines.append('                                            { %s }};' % ', '.join('"%s"' % x for x in te_out[8:]))
lines.append('static const char* const kTeSortArm[2]   = { %s };' % ', '.join('"%s"' % x for x in te_sort))
teach_bytes = ('\r\n'.join(lines) + '\r\n').encode('utf-8')

# ---- 4. W5-b：教導頁 Set／Go 按鈕 → golden 登錄表的每一列（X-macro 表，WebTeachButtons.gen.inc）
#   AI(W906-W5-b) 20260925: 改寫（W5-b 覆核 W5B-1／W5B-2／W5B-8）。原本的第 4 段有三個錯：
#     (1) 只讀 .dfm 的 OnClick，沒套 golden 建構子的執行期覆寫（uteach.cpp:271-274 `SetButton060..063->OnClick=SetButton140Click;`），
#         於是 SetButton060-063 被標成 SetButton064Click，C++ 當怪按鈕拒絕 —— golden 其實可以正常手動教導入料飛梭 1／2 的左右點；
#     (2) 沒剝 `//` 行註解，把 golden 已註解掉的 Set／GoButton202、203 四列死登錄產生出來；
#     (3) 一顆按鈕攤成「每個條件分支一列」，C++ 端拿「頁面送的馬達對得上任何一列」放行 —— 不是 golden 的 Tag 語意。
#   現在的做法：
#     * 移植樹 forms/fTeachRegistry.cpp 與 golden uteach.cpp 建構子**各自**用同一個 C 詞法器剝註解、同一個遞迴下降
#       解析 if／else，得到兩份「登錄列序列」（每一個 push_back 一列，帶它外層的 if 條件），兩份必須逐列相同；
#       另用正則法（不解析控制流，只數 push_back 與按鈕）再數一次，兩種方法的數字必須一致（交叉比對）。
#     * 每一列帶 C++ 條件式（W5B_COND）—— 執行期由 WebMotorAccessLive.cpp 在這台機台上求值，再與 fTeach->TechPara／
#       TechTwoPara（實際登錄結果，比 Parameter 指標＋馬達＋Key）逐列核對；對不上就整個教導頁運動鈕拒絕（fail-closed）。
#     * 處理函式 = .dfm 的 OnClick，再套 golden 建構子的 OnClick 覆寫（全檔的覆寫必須都在建構子裡，否則停）。
#     * golden FormShow（uteach.cpp:1518-1521）把四顆 SetButtonIn*LtcSen* 的 Tag 改成軸控鈕的 Tag → W5B_TAGOVR 列。
#   「按鈕 → 哪一列」（golden Tag 語意：TechPara 迴圈先、TechTwoPara 迴圈後，同一顆按鈕最後登錄的列勝出）不在這裡算，
#   在 C++（WebMotorAccess.cpp MotorAccessResolveTeachButton）用這台機台的實際登錄表算 —— 列的索引依機種條件而變。
AI_BTN_OUT = os.path.join(ROOT, 'WebTeachButtons.gen.inc')
EXPECT_REGS = 312          # golden uteach.cpp 建構子剝註解後的 TechPara＋TechTwoPara push_back 數（20260925 量）
EXPECT_BTNS = 624          # 其中非 NULL 的 Set／Go 按鈕數（W5-b 覆核 indep.py 另法量得同值）
EXPECT_OVR = {'SetButton060': 'SetButton140Click', 'SetButton061': 'SetButton140Click',
              'SetButton062': 'SetButton140Click', 'SetButton063': 'SetButton140Click'}   # golden uteach.cpp:271-274


def c_strip(src):
    """剝 // 與 /* */ 註解（字串、字元常數照留；換行數不變，行號對得回原檔）。"""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if c == '"' or c == "'":
            j = i + 1
            while j < n and src[j] != c:
                if src[j] == chr(92):
                    j += 1
                j += 1
            out.append(src[i:j + 1]); i = j + 1; continue
        if src.startswith('//', i):
            j = src.find(chr(10), i); j = n if j < 0 else j
            i = j; continue
        if src.startswith('/*', i):
            j = src.find('*/', i + 2); j = n if j < 0 else j + 2
            out.append(' ' + chr(10) * src[i:j].count(chr(10))); i = j; continue
        out.append(c); i += 1
    return ''.join(out)


TOKRE = re.compile(r'"(?:[^"' + chr(92) * 2 + r']|' + chr(92) * 2 + r'.)*"|' +
                   r"'(?:[^'" + chr(92) * 2 + r"]|" + chr(92) * 2 + r".)*'|" +
                   r'[A-Za-z_]\w*|\d+\w*|->|==|!=|<=|>=|&&|\|\||::|\S')


def paren(toks, i):
    assert toks[i] == '(', (i, toks[i:i + 5])
    d, j = 0, i
    while True:
        if toks[j] == '(':
            d += 1
        elif toks[j] == ')':
            d -= 1
            if d == 0:
                break
        j += 1
    return j + 1, ''.join(toks[i + 1:j])


def parse_block(toks, i, conds, out, end='}'):
    while i < len(toks) and toks[i] != end:
        i = parse_stmt(toks, i, conds, out)
    return i


def parse_stmt(toks, i, conds, out):
    """一個敘述：{…}、if(…)…[else …]、for／while(…)…、或到 ; 為止的簡單敘述（記錄它與外層條件）。"""
    t = toks[i]
    if t == '{':
        return parse_block(toks, i + 1, conds, out) + 1
    if t == 'if':
        j, c = paren(toks, i + 1)
        chain = [c]
        i = parse_stmt(toks, j, conds + [c], out)
        while i < len(toks) and toks[i] == 'else':
            negs = ['!(%s)' % x for x in chain]
            if toks[i + 1] == 'if':
                j, c2 = paren(toks, i + 2)
                i = parse_stmt(toks, j, conds + negs + [c2], out)
                chain.append(c2)
            else:
                i = parse_stmt(toks, i + 1, conds + negs, out)
                break
        return i
    if t in ('for', 'while'):
        j, c = paren(toks, i + 1)
        return parse_stmt(toks, j, conds + ['<loop %s>' % c], out)
    j, d = i, 0
    while not (toks[j] == ';' and d == 0):
        if toks[j] in '([':
            d += 1
        elif toks[j] in ')]':
            d -= 1
        j += 1
    out.append((conds, toks[i:j]))
    return j + 1


def split_top(toks):
    args, cur, d = [], [], 0
    for t in toks:
        if t in '([':
            d += 1
        elif t in ')]':
            d -= 1
        if t == ',' and d == 0:
            args.append(''.join(cur)); cur = []
        else:
            cur.append(t)
    args.append(''.join(cur))
    return args


def reg_rows(stmts, widget):
    rows = []
    for conds, st in stmts:
        s = ''.join(st)
        m = re.match(r'^(TechPara|TechTwoPara)\.push_back\(newTECH_(PARA|TWOPARA)\((.*)\)\)$', s)
        if not m:
            assert 'push_back(newTECH_PARA' not in s and 'push_back(newTECH_TWOPARA' not in s, ('unparsed push_back', s)
            continue
        assert (m.group(1) == 'TechPara') == (m.group(2) == 'PARA'), s
        assert not any(c.startswith('<loop') for c in conds), ('push_back inside a loop', s)
        k = st.index('TECH_' + m.group(2))
        a = split_top(st[k + 2:-2])
        if m.group(2) == 'PARA':
            assert len(a) in (6, 7), a
            r = dict(owner='P', p=[a[0], None], mot=[a[1], None], edit=[widget(a[2]), None], key=[a[3], None],
                     btn=[widget(a[4]), widget(a[5])])
        else:
            assert len(a) in (10, 11), a
            r = dict(owner='T', p=[a[0], a[1]], mot=[a[2], a[3]], edit=[widget(a[4]), widget(a[5])], key=[a[6], a[7]],
                     btn=[widget(a[8]), widget(a[9])])
        # AI(W906-W5-b) 20260925: 覆核 R-W5B-2 —— 最後一個引數 Visible（預設 true）；TECH_PARA／TECH_TWOPARA 建構子
        #   （golden uteach.cpp:61-74／:159-178）拿它設 SetEdit／funButton／btGo->Visible。原本丟掉了，C++ 照樣執行 golden 按不到的按鈕。
        va = a[6] if m.group(2) == 'PARA' and len(a) == 7 else a[10] if m.group(2) == 'TWOPARA' and len(a) == 11 else 'true'
        assert va in ('true', 'false'), ('Visible argument is not a literal', va, s)
        r['vis'] = (va == 'true')
        for p in r['p']:
            assert p is None or re.match(r'^&(Tech|Teach)\.\w+(\[\d+\])*$', p), ('parameter expression', p)
        for mo in r['mot']:
            assert mo is None or re.match(r'^[A-Za-z_]\w*$', mo), ('motor expression', mo)
        for ky in r['key']:
            assert ky is None or re.match(r'^"\w+"$', ky), ('key literal', ky)
        r['cond'] = list(conds)
        rows.append(r)
    return rows


def w_port(a):                     # 移植樹：T1 變換把元件引數寫成 /*名稱*/0（下面先換成 W906W__名稱 才剝註解）
    m = re.match(r'^W906W__(\w+)$', a)
    if m:
        return m.group(1)
    assert a in ('0', 'NULL'), ('port widget arg', a)
    return None


def w_gold(a):
    if a in ('NULL', '0'):
        return None
    assert re.match(r'^\w+$', a), ('golden widget arg', a)
    return a


# (a) 移植樹 forms/fTeachRegistry.cpp 的 BuildTechRegistry()
reg_src = re.sub(r'/\*(\w+)\*/0' + r'\b', r'W906W__\1', reg.replace(chr(13) + chr(10), chr(10)))
reg_body = c_strip(reg_src[reg_src.index('void TfTeach::BuildTechRegistry()'):])
rt = TOKRE.findall(reg_body)
st_port = []
parse_block(rt, rt.index('{') + 1, [], st_port)
port_rows = reg_rows(st_port, w_port)

# (b) golden uteach.cpp 的 TfTeach::TfTeach 建構子（整支，含 :271-274 的 OnClick 覆寫）
gsrc = open(os.path.join(GOLDEN, 'uteach.cpp'), 'rb').read().decode('cp950', 'replace').replace(chr(13) + chr(10), chr(10))
gcode = c_strip(gsrc)
gct = TOKRE.findall(gcode[gcode.index('TfTeach::TfTeach(TComponent'):])
st_gold = []
parse_block(gct, gct.index('{') + 1, [], st_gold)
gold_rows = reg_rows(st_gold, w_gold)


def rkey(r):
    return (r['owner'], tuple(r['p']), tuple(r['mot']), tuple(r['edit']), tuple(r['key']), tuple(r['btn']), tuple(r['cond']), r['vis'])


assert len(port_rows) == len(gold_rows), ('registry row count port vs golden', len(port_rows), len(gold_rows))
bad = [i for i, (x, y) in enumerate(zip(port_rows, gold_rows)) if rkey(x) != rkey(y)]
assert not bad, ('registry rows differ port vs golden at', bad[:5], rkey(port_rows[bad[0]]), rkey(gold_rows[bad[0]]))
n_btn = sum(1 for r in port_rows for b in r['btn'] if b)
assert len(port_rows) == EXPECT_REGS and n_btn == EXPECT_BTNS, ('exact counts', len(port_rows), n_btn)

# (c) 交叉比對：正則法（不解析控制流）再數一次 golden 建構子 —— 數量與按鈕多重集合都要一致
g_ctor = gcode[gcode.index('TfTeach::TfTeach(TComponent'):]
g_ctor = g_ctor[:g_ctor.index('TechTwoItem=TechTwoPara.size();')]
rx_p = re.findall(r'TechPara\.push_back\(\s*new\s+TECH_PARA\((.*?)\)\s*\)\s*;', g_ctor, re.S)
rx_t = re.findall(r'TechTwoPara\.push_back\(\s*new\s+TECH_TWOPARA\((.*?)\)\s*\)\s*;', g_ctor, re.S)
rx_btn = sorted([b.strip() for a in rx_p for b in a.split(',')[4:6] if b.strip() not in ('NULL', '0')] +
                [b.strip() for a in rx_t for b in a.split(',')[8:10] if b.strip() not in ('NULL', '0')])
assert len(rx_p) + len(rx_t) == EXPECT_REGS, ('regex count', len(rx_p), len(rx_t))
assert rx_btn == sorted(b for r in gold_rows for b in r['btn'] if b), 'regex button multiset != parsed button multiset'

# (d) golden 建構子的 OnClick 覆寫；全檔（剝註解後）的 ->OnClick= 必須都在建構子裡
ovr = {}
for conds, st in st_gold:
    m = re.match(r'^(\w+)->OnClick=(\w+)$', ''.join(st))
    if m:
        assert not conds, ('conditional OnClick override', m.group(0), conds)
        ovr[m.group(1)] = m.group(2)
assert ovr == EXPECT_OVR, ('ctor OnClick overrides', ovr)
all_ovr = dict(re.findall(r'(\w+)\s*->\s*OnClick\s*=\s*(\w+)\s*;', gcode))
assert all_ovr == ovr, ('OnClick assignment outside the ctor', all_ovr)

# (e) .dfm 的 OnClick（堆疊式解析：面板裡的按鈕是巢狀 object）
dfm = open(os.path.join(GOLDEN, 'uteach.dfm'), 'rb').read().decode('cp950', 'replace').replace(chr(13) + chr(10), chr(10))
dfm_obj, stack = {}, []
dfm_par, dfm_prop = {}, {}             # AI(W906-W5-b) 20260925: 覆核 R-W5B-2／3 —— 父物件與 Visible／TabVisible／Enabled／Tag
for ln in dfm.split(chr(10)):
    t = ln.strip()
    m = re.match(r'^(object|inherited|inline) (\w+): (T\w+)', t)
    if m:
        assert m.group(2) not in dfm_obj, ('duplicate dfm object', m.group(2))
        dfm_par[m.group(2)] = stack[-1] if stack else None
        dfm_prop[m.group(2)] = {}
        stack.append(m.group(2)); dfm_obj[m.group(2)] = [m.group(3), '']; continue
    if t == 'end' and stack:
        stack.pop(); continue
    m = re.match(r'^OnClick = (\w+)$', t)
    if m and stack:
        assert dfm_obj[stack[-1]][1] == '', ('two OnClick', stack[-1])
        dfm_obj[stack[-1]][1] = m.group(1)
    m = re.match(r'^(Visible|TabVisible|Enabled|Tag) = (\S+)$', t)
    if m and stack:
        dfm_prop[stack[-1]][m.group(1)] = m.group(2)
assert not stack, stack
handler = {}
for r in port_rows:
    for b in r['btn']:
        if b:
            assert b in dfm_obj and dfm_obj[b][1], ('button without dfm OnClick', b)
            handler[b] = ovr.get(b, dfm_obj[b][1])

# (f) golden FormShow 的 Tag 覆寫（uteach.cpp:1518-1521 `SetButtonInSh1LtcSenZ1->Tag=MotorInSh1LtcZ1->Tag;`）
fs = gcode[gcode.index('void __fastcall TfTeach::FormShow('):]
fs = fs[:fs.index(chr(10) + '}')]
tag_ovr = re.findall(r'(\w+)->Tag\s*=\s*(\w+)->Tag\s*;', fs)
axle = {}
for conds, st in st_gold:
    m = re.match(r'^TechMotorAxle\.push_back\(newTECH_MotorAxle\((\w+),(\w+)(?:,[^)]*)?\)\)$', ''.join(st))
    if m:
        axle.setdefault(m.group(2), (m.group(1), conds))
tagovr_rows = []
for b, src in tag_ovr:
    assert handler.get(b) == 'MotorTrayXClick', ('Tag override on a button whose handler is not MotorTrayXClick', b, handler.get(b))
    assert src in axle and not axle[src][1], ('Tag source is not an unconditional TechMotorAxle button', src)
    tagovr_rows.append((b, src, axle[src][0]))
assert sorted(b for b, h in handler.items() if h == 'MotorTrayXClick') == sorted(b for b, _, _ in tagovr_rows), 'MotorTrayXClick buttons != FormShow Tag overrides'

# (f2) AI(W906-W5-b) 20260925: 覆核 R-W5B-2／R-W5B-3 —— golden 按得到嗎、沒登錄的教導鈕讀哪一列。
#   R-W5B-2：TECH_PARA 建構子的 Visible 引數（上面 r['vis']）＋ .dfm 的 Visible／TabVisible／Enabled ＋ golden 程式裡每一個
#     `X->Visible=`／`->TabVisible=`／`->Enabled=`（建構子與 FormShow 帶 if／for 條件，其他函式一律當「執行期會變」）。
#     只判「靜態確定按不到」：X 的起始值（.dfm）是 false 或者建構子／FormShow 有無條件的 `=false`，而且全檔沒有任何一個
#     可能設成 true 的賦值。依組態才決定的（FormShow 裡帶條件的 Visible，約 600 行）**不模擬** —— 那些當成「按得到」，
#     W5_PROGRESS.md §6 列為待決。
#   R-W5B-3：.dfm 上處理函式是教導處理函式、卻（依機種）沒有登錄的按鈕 —— golden FormShow 不設它的 Tag，Tag＝.dfm 的值（預設 0），
#     處理函式照樣讀 TechPara[Tag]／TechTwoPara[Tag]。W5B_BTN 列出每一顆，C++ 判成「未登錄的怪按鈕」並說出 golden 會讀哪一列。
TEACH_HANDLERS = ('SetButton140Click', 'GoButton140Click', 'SetButton020Click', 'GoButton020Click', 'SetButton064Click', 'MotorTrayXClick')


def body_range(code, sig):
    s = code.index(sig)
    b = code.index('{', s)
    d, j = 0, b
    while True:
        if code[j] == '{':
            d += 1
        elif code[j] == '}':
            d -= 1
            if d == 0:
                return s, j + 1
        j += 1


ctor_rng = body_range(gcode, 'TfTeach::TfTeach(TComponent')
fs_rng = body_range(gcode, 'void __fastcall TfTeach::FormShow(')
fst = TOKRE.findall(gcode[fs_rng[0]:fs_rng[1]])
st_fs = []
parse_block(fst, fst.index('{') + 1, [], st_fs)
VIS_RE = re.compile(r'(\w+)\s*->\s*(Visible|TabVisible|Enabled)\s*=(?!=)\s*([^;]*);')


def vval(e):
    e = e.strip()
    return True if e == 'true' else False if e == 'false' else None     # None＝不是字面值（執行期才知道）


assign = {}                               # (物件, 屬性) → [(值 True/False/None, 有條件, 在建構子或 FormShow, golden 行號)]
unknown_tgt = set()
for fn_rng, stl in ((ctor_rng, st_gold), (fs_rng, st_fs)):
    hits = [m for m in VIS_RE.finditer(gcode, fn_rng[0], fn_rng[1])]
    parsed = [(c, ''.join(t)) for c, t in stl if re.match(r'^\w+->(Visible|TabVisible|Enabled)=', ''.join(t))]
    assert len(hits) == len(parsed), ('visibility assignment count regex vs parser', len(hits), len(parsed))
    for m, (c, s) in zip(hits, parsed):
        assert s.startswith(m.group(1) + '->' + m.group(2) + '='), (s, m.group(0))
        ln = gcode.count(chr(10), 0, m.start()) + 1
        loop = [x for x in c if x.startswith('<loop') and 'ControlCount' in x]
        if loop:                          # FormShow :1785-1803 `for(...X->ControlCount...) Control->Visible=false;`
            assert m.group(1) == 'Control' and m.group(2) == 'Visible' and vval(m.group(3)) is False, ('unexpected Controls loop body', s)
            cont = re.search(r'(\w+)->ControlCount', loop[0]).group(1)
            for ch, pa in dfm_par.items():
                if pa == cont and dfm_obj[ch][0] in ('TSpeedButton', 'TEdit'):
                    assign.setdefault((ch, 'Visible'), []).append((False, True, True, ln))
            continue
        if m.group(1) not in dfm_obj:
            unknown_tgt.add(m.group(1)); continue
        assign.setdefault((m.group(1), m.group(2)), []).append((vval(m.group(3)), bool(c), True, ln))
for m in VIS_RE.finditer(gcode):          # 其他函式（事件處理等）：一律當「執行期可能變成任何值」
    if ctor_rng[0] <= m.start() < ctor_rng[1] or fs_rng[0] <= m.start() < fs_rng[1]:
        continue
    if m.group(1) not in dfm_obj:
        unknown_tgt.add(m.group(1)); continue
    assign.setdefault((m.group(1), m.group(2)), []).append((None if vval(m.group(3)) is not False else False, True, False,
                                                            gcode.count(chr(10), 0, m.start()) + 1))
# 不是 .dfm 物件的賦值目標只能是 TECH_* 建構子的成員（registration 的 Visible 由 r['vis'] 處理）與迴圈變數／區域指標
assert unknown_tgt <= {'funButton', 'btGo', 'SelButton', 'SetEdit', 'Control', 'Ptr', 'EditPtr'}, ('unexpected visibility target', sorted(unknown_tgt))
# 陣列成員的 ->Visible／->Enabled 只能是教導欄位（TEdit），不會是按鈕：TECH_TWOPARA 建構子的 SetEdit[0／1]（golden :172-173）
#   與 FormShow 的 TechSuckPara[..].SetEdit[..][..]->Enabled=false（:1570-1571、:1588）；另外兩處是 pitch 歸零的區域 TButton 陣列
#   `TButton *btn[4]={btn_InPX_13Home,...}`（:5611、:5648、:5772 —— 不是教導鈕）
for m in re.finditer(r'\]\s*->\s*(Visible|TabVisible|Enabled)\s*=(?!=)', gcode):
    lhs = gcode[gcode.rfind(chr(10), 0, m.start()) + 1:m.start()].strip()
    ok_edit = (lhs.startswith('SetEdit') or lhs.startswith('TechSuckPara')) and 'SetEdit' in lhs
    ok_pitch = lhs.startswith('btn[') and 'TButton*btn[4]={btn_InPX_13Home' in gcode[:m.start()].replace(' ', '')[-3000:]
    assert ok_edit or ok_pitch, ('array ->Visible/Enabled on something that is not a teach edit', lhs, gcode.count(chr(10), 0, m.start()) + 1)


def dfm_bool(name, prop):
    v = dfm_prop.get(name, {}).get(prop)
    return True if v is None else (v == 'True')


def static_hidden(name, prop):
    """'' ＝不是靜態確定的 false；否則回理由。"""
    a = assign.get((name, prop), [])
    if any(v is not False for v, _, _, _ in a):
        return ''
    if not dfm_bool(name, prop):
        return '%s.%s=False（.dfm），golden 沒有任何地方設回 true' % (name, prop)
    unc = [ln for v, c, init, ln in a if v is False and not c and init]
    if unc:
        return '%s->%s=false（golden uteach.cpp:%d 無條件），沒有任何地方設回 true' % (name, prop, unc[0])
    return ''


def ancestor_hidden(b):
    n = dfm_par.get(b)
    while n and n != 'fTeach':
        cls = dfm_obj[n][0]
        props = ['Enabled', 'Visible']
        if cls == 'TTabSheet' and dfm_par.get(n) and dfm_obj[dfm_par[n]][0] == 'TPageControl':
            props = ['Enabled', 'TabVisible']              # 分頁的 Visible 由 PageControl 管；看 TabVisible
        for p in props:
            why = static_hidden(n, p)
            if why:
                return why
        n = dfm_par.get(n)
    return ''


def self_other(b):
    """按鈕自己在 registration 之外的 Visible 賦值：'' 沒有；'hide' 無條件 false 且沒有可能設 true；'hideCond' 只有帶條件的 false；'dyn' 可能設 true。"""
    a = assign.get((b, 'Visible'), [])
    if not a:
        return ''
    if any(v is not False for v, _, _, _ in a):
        return 'dyn'
    return 'hide' if any(not c and init for v, c, init, _ in a) else 'hideCond'


registered_btns = set(b for r in port_rows for b in r['btn'] if b)
# golden TfTeach::InitialFormOncetime（uteach.cpp:5998-6014）在 USE_Scanner_AOI_Inspection==TopBottomInstall 時另外 push_back
#   7 列 TECH_TWOPARA＋2 列 TECH_PARA（AOI 上下檢查，參數在 FrmAOI->ttbInsp）；移植樹沒有 FrmAOI（fTeach.h [DEP]）⇒ 這 18 顆按鈕在移植樹一律沒登錄
ifo = gcode[body_range(gcode, 'void TfTeach::InitialFormOncetime(')[0]:body_range(gcode, 'void TfTeach::InitialFormOncetime(')[1]]
late_btns = set()
for m in re.finditer(r'push_back\(\s*new\s+TECH_(PARA|TWOPARA)\((.*?)\)\s*\)\s*;', ifo, re.S):
    a = [x.strip() for x in split_top(TOKRE.findall(m.group(2)))]
    late_btns.update(a[4:6] if m.group(1) == 'PARA' else a[8:10])
assert len(late_btns) == 18 and not (late_btns & registered_btns), ('InitialFormOncetime buttons', len(late_btns))
btn_info = []
for b, (cls, oc) in sorted(dfm_obj.items()):
    if cls != 'TSpeedButton':
        continue
    h = ovr.get(b, oc)
    if h not in TEACH_HANDLERS[:5] and b not in registered_btns:          # MotorTrayXClick 的軸控鈕走 TechMotorAxle（頁面自己選馬達），不列
        continue
    tag = dfm_prop.get(b, {}).get('Tag', '0')
    assert re.match(r'^-?\d+$', tag), (b, tag)
    en = static_hidden(b, 'Enabled')
    btn_info.append((b, h, int(tag), 1 if dfm_bool(b, 'Visible') else 0, self_other(b), en or ancestor_hidden(b), 1 if b in late_btns else 0))
assert registered_btns <= set(x[0] for x in btn_info), ('registered button missing from dfm', sorted(registered_btns - set(x[0] for x in btn_info))[:5])
assert late_btns <= set(x[0] for x in btn_info), ('InitialFormOncetime button without a teach handler', sorted(late_btns - set(x[0] for x in btn_info)))
n_unreg = sum(1 for x in btn_info if x[0] not in registered_btns and x[1] in TEACH_HANDLERS[:5])

# (g) 輸出
atoms = set()
for r in port_rows:
    for c in r['cond']:
        assert not c.startswith('!') or (c.startswith('!(') and c.endswith(')')), c
        atoms.add(c[2:-1] if c.startswith('!(') else c)
assert not any(a.startswith('!') or ';' in a for a in atoms), atoms


def cexpr(conds):
    return '&&'.join('(%s)' % c if not c.startswith('!(') else c for c in conds) if conds else 'true'


def catoms(conds):
    return ';'.join(('!' + c[2:-1]) if c.startswith('!(') else c for c in conds)


def q(s):
    return '"%s"' % (s or '')


def mot(s):
    return 'W5B_MOT(%s)' % s if s else 'W5B_NOMOT'


def ptr(s):
    return 'W5B_PTR(%s)' % s[1:] if s else 'W5B_NOPTR'


bl = ['// ===========================================================================',
      '//  WebTeachButtons.gen.inc  --  產生的，不要手改（tools/gen_teach_editlist.py 第 4 段）。AI(W906-W5-b) 20260925',
      '//  golden：%s\\uteach.cpp 的 TfTeach 建構子登錄表（與 forms/fTeachRegistry.cpp 逐列相同，產生器驗過）' % GOLDEN_SHOWN,
      '//  %d 列登錄（TechPara %d＋TechTwoPara %d），Set／Go 按鈕 %d 顆；處理函式 = uteach.dfm 的 OnClick 再套建構子覆寫 %d 顆（:271-274）。'
      % (len(port_rows), sum(1 for r in port_rows if r['owner'] == 'P'), sum(1 for r in port_rows if r['owner'] == 'T'), n_btn, len(ovr)),
      '//  W5B_ROW(owner, live, atoms, p0, p1, mot0, mot1, key0, key1, edit0, edit1, setBtn, setHandler, goBtn, goHandler, vis)',
      '//    owner  P＝TECH_PARA（TechPara.push_back）、T＝TECH_TWOPARA（TechTwoPara.push_back）',
      '//    live   W5B_COND(外層 if／else 條件的 C++ 式) —— 執行期在這台機台上求值（WebMotorAccessLive.cpp）',
      '//    atoms  同一個條件拆成原子（; 分隔，! 開頭＝else 支），給單元測試用字串求值',
      '//    p0/p1  W5B_PTR(Tech.x)＝golden Parameter；mot0/1 W5B_MOT(列舉名)；key＝ini 鍵；edit＝畫面欄位（golden SetEdit）',
      '//    vis    建構子最後一個引數 Visible（1＝預設 true；0＝golden 傳 false，建構子把兩顆按鈕與欄位設成看不見）。列的順序＝golden 建構子的執行順序。',
      '//  W5B_TAGOVR(btn, axleButton, mot)：golden FormShow（:1518-1521）把 btn 的 Tag 改成軸控鈕 axleButton 的 Tag；',
      '//    btn 的處理函式是 MotorTrayXClick（讀 TechMotorAxle[Tag]，只選馬達 mot，不動）。',
      '//  W5B_BTN(btn, handler, dfmTag, dfmVisible, selfOther, hiddenWhy, lateReg)：.dfm 上每一顆「處理函式是教導處理函式或有登錄」的按鈕（AI(W906-W5-b) 覆核 R-W5B-2／3）',
      '//    dfmTag     .dfm 的 Tag（沒寫＝0）—— 沒登錄的按鈕 golden 不設 Tag，處理函式照這個值讀清單',
      '//    dfmVisible .dfm 的 Visible（沒寫＝1）；selfOther 按鈕自己在登錄以外的 Visible 賦值："" 沒有、"hide" 無條件 false、"hideCond" 只有帶條件的 false、"dyn" 可能設 true',
      '//    hiddenWhy  非空＝靜態確定按不到（按鈕 Enabled=false，或某個父物件看不見／停用，golden 沒有任何地方改回來）的理由',
      '//    lateReg    1＝golden 只在 InitialFormOncetime（:5998-6014，TopBottomInstall）登錄它；移植樹沒有 FrmAOI，一律沒登錄',
      '//  include 之前要定義 W5B_ROW／W5B_TAGOVR／W5B_BTN／W5B_COND／W5B_PTR／W5B_NOPTR／W5B_MOT／W5B_NOMOT；檔尾全部 #undef。',
      '// ===========================================================================']
for r in port_rows:
    s0, s1 = r['btn']
    bl.append("W5B_ROW('%s', W5B_COND(%s), %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %d)" % (
        r['owner'], cexpr(r['cond']), q(catoms(r['cond'])), ptr(r['p'][0]), ptr(r['p'][1]), mot(r['mot'][0]), mot(r['mot'][1]),
        r['key'][0], r['key'][1] or '""', q(r['edit'][0]), q(r['edit'][1]),
        q(s0), q(handler.get(s0) if s0 else ''), q(s1), q(handler.get(s1) if s1 else ''), 1 if r['vis'] else 0))
for b, src, m in tagovr_rows:
    bl.append('W5B_TAGOVR("%s", "%s", W5B_MOT(%s))' % (b, src, m))
for b, h, tag, dv, so, hw, lr in btn_info:
    assert '"' not in hw and chr(92) not in hw, hw
    bl.append('W5B_BTN("%s", "%s", %d, %d, "%s", "%s", %d)' % (b, h, tag, dv, so, hw, lr))
bl += ['#undef W5B_ROW', '#undef W5B_TAGOVR', '#undef W5B_BTN', '#undef W5B_COND', '#undef W5B_PTR', '#undef W5B_NOPTR', '#undef W5B_MOT', '#undef W5B_NOMOT']
btn_bytes = ('\r\n'.join(bl) + '\r\n').encode('utf-8')

summary = ('%d registrations (P %d / T %d), %d buttons, %d ctor OnClick overrides, %d FormShow Tag overrides, %d condition atoms, '
           '%d teach-handler buttons in the dfm (%d never registered in any config, %d statically unreachable, %d registered Visible=false)') % (
    len(port_rows), sum(1 for r in port_rows if r['owner'] == 'P'), sum(1 for r in port_rows if r['owner'] == 'T'),
    n_btn, len(ovr), len(tagovr_rows), len(atoms), len(btn_info), n_unreg, sum(1 for x in btn_info if x[5]),
    len(set(b for r in port_rows if not r['vis'] for b in r['btn'] if b)))
if '--report' in sys.argv[1:]:
    for b, h, tag, dv, so, hw, lr in btn_info:
        regs = [(i, r['owner'], r['vis'], catoms(r['cond'])) for i, r in enumerate(port_rows) if b in r['btn']]
        print('BTN %-26s %-18s tag=%-5d dfmVis=%d self=%-8s late=%d reg=%s hidden=%s' % (b, h, tag, dv, so or '-', lr, regs if regs else 'NONE', hw or '-'))

if CHECK:
    stale = []
    for path, data in ((OUT, teach_bytes), (AI_BTN_OUT, btn_bytes)):
        cur = open(path, 'rb').read() if os.path.exists(path) else b''
        # 換行不算差異：本 repo core.autocrlf=true（索引 LF、工作樹 CRLF），別的 clone 可能是 LF
        if cur.replace(b'\r\n', b'\n') != data.replace(b'\r\n', b'\n'):
            stale.append(os.path.relpath(path, ROOT))
    if stale:
        print('STALE: %s differ(s) from a fresh run of tools/gen_teach_editlist.py -- rerun it and commit the result' % ', '.join(stale))
        sys.exit(1)
    print('OK: Teach.gen.inc and WebTeachButtons.gen.inc match a fresh run; golden cross-check passed (%s)' % summary)
    sys.exit(0)

open(OUT, 'wb').write(teach_bytes)
print('wrote %s: %d elTeach adds, %d key->widget pairs, suck %d/%d/%d' % (OUT, n_add, len(pairs), len(te_in), len(te_out), len(te_sort)))
open(AI_BTN_OUT, 'wb').write(btn_bytes)
print('wrote %s: %s' % (AI_BTN_OUT, summary))
