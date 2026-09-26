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
# 用法：python tools/gen_teach_editlist.py      （在 HT9011UC_Cpp_V3.33.906.0 底下跑）
# ----------------------------------------------------------------------
import io
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
GOLDEN = r'D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618'
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
lines.append('//  golden：%s\\uteach.cpp（cp950 → UTF-8）' % GOLDEN)
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
open(OUT, 'wb').write(('\r\n'.join(lines) + '\r\n').encode('utf-8'))
print('wrote %s: %d elTeach adds, %d key->widget pairs, suck %d/%d/%d' % (OUT, n_add, len(pairs), len(te_in), len(te_out), len(te_sort)))
