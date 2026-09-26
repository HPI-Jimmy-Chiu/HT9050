# -*- coding: utf-8 -*-
# ===========================================================================
#  tools/nb2_assist/r32_switch_to_906.py  --  NB2 輔助 session 20260925
#
#  使用者 20260925 13:3x 裁決 R32＝A：「照 §15 全部改回 906」。這支把 README R32「② 改法」機械化：
#    1. 三支產生器＋4 支 editlist 設定檔的 golden 路徑 → 906（可用 HT9045_GOLDEN_ROOT 覆寫，與 dfm2rc be1a88de 同一個變數）
#    2. blocks／replace 的 (方法, 起, 迄) 換成 906 行號（用 golden906_switch_plan 的 find_block，比對段落文字）
#    3. 906 沒有對應段落的 block → 刪（連同正上方同縮排的原因註解）
#    4. formbridge overrides 的 old 字串在 906 方法本體找不到 → 刪
#    5. DeviceForm_File.py 的 methods 拿掉 906 沒有的方法（CalcDeviceForce）
#    ＋ tuple 正上方註解裡寫的「:起-迄」若正好是 912 那組數字，一併換成 906 的
#  用 ast 找 tuple（不是字串搜尋），偏移量是 UTF-8 位元組；編輯從檔尾往前套。
#
#  用法：
#    python tools/nb2_assist/r32_switch_to_906.py --root <樹根的複本> [--g912 <V912 原版 6943d134>]   # 就地改那份複本
#    python tools/nb2_assist/r32_switch_to_906.py --diff OUT.patch [--g912 ...]                        # 只產 git apply 用的 patch，不動任何檔
#  ⚠ --root 只給複本：它會改檔。對 repo 本身請用 --diff，由新電腦審過再 git apply。
# ===========================================================================
import argparse
import ast
import difflib
import io
import os
import re
import shutil
import sys

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import golden906_switch_plan as P  # noqa: E402

REF912 = r'D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
HANDWRITTEN = ''   # main() 填：FileRW 裡手寫（非產生器輸出）的 .cpp/.h 全文
LOC912 = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
G906 = r'D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618'
NEWEXPR = "(os.environ.get('HT9045_GOLDEN_ROOT') or r'%s')" % G906
PATHFILES = ['tools/gen_editlist.py', 'tools/gen_formbridge.py', 'tools/gen_sjson.py',
             'tools/editlist/Offset_File.py', 'tools/editlist/TTLCfg.py', 'tools/editlist/Temperature.py',
             'tools/editlist/TestIF_File_SetUp.py']


def const_int(n):
    return isinstance(n, ast.Constant) and isinstance(n.value, int) and not isinstance(n.value, bool)


def const_str(n):
    return isinstance(n, ast.Constant) and isinstance(n.value, str)


class Buf:
    """UTF-8 位元組緩衝，以 (lineno, col_offset) 定位。"""
    def __init__(self, text):
        self.b = text.encode('utf-8')
        self.starts = [0]
        for i, ch in enumerate(self.b):
            if ch == 0x0A:
                self.starts.append(i + 1)
        self.edits = []   # (start, end, replacement bytes)

    def off(self, lineno, col):
        return self.starts[lineno - 1] + col

    def line_span(self, lineno):
        s = self.starts[lineno - 1]
        e = self.starts[lineno] if lineno < len(self.starts) else len(self.b)
        return s, e

    def add(self, s, e, rep):
        self.edits.append((s, e, rep.encode('utf-8') if isinstance(rep, str) else rep))

    def result(self):
        out = self.b
        for s, e, rep in sorted(self.edits, key=lambda x: -x[0]):
            out = out[:s] + rep + out[e:]
        return out.decode('utf-8')


def plan_for(cfg_kind, cfg, el, fb, g906, g912):
    """回傳 {(meth,s,e): ('renum',(a,b)) | ('delete',None)} 與要刪的 override (meth, old) 集合、906 沒有的方法。"""
    cp = el['cp950']
    cpp9 = cp(os.path.join(g906, cfg['cpp'])); cpp2 = cp(os.path.join(g912, cfg['cpp']))
    L9 = cpp9.replace('\r\n', '\n').split('\n'); L2 = cpp2.replace('\r\n', '\n').split('\n')

    def body(cpp, m):
        if cfg_kind == 'editlist':
            params, b, line0, sig = el['body_of'](cpp, cfg['class'], m)
        else:
            params, b, line0 = fb['method_body'](cpp, cfg['class'], m)
        return b, line0

    spans9, spans2, missing906 = {}, {}, []
    bodies9 = {}
    for m in cfg['methods']:
        try:
            b2, l2 = body(cpp2, m)
            spans2[m] = (l2, l2 + b2.count('\n'))
        except BaseException:
            pass
        try:
            b9, l9 = body(cpp9, m)
            spans9[m] = (l9, l9 + b9.count('\n')); bodies9[m] = b9
        except BaseException:
            missing906.append(m)
    blocks = list(cfg.get('blocks', [])) + (list(cfg.get('replace', [])) if cfg_kind == 'editlist' else [])
    bp = {}
    for blk in blocks:
        meth, s, e = blk[0], blk[1], blk[2]
        sp, sp2 = spans9.get(meth), spans2.get(meth)
        hit = P.map_block(L2, L9, sp2, sp, s, e) if (sp and sp2) else None
        bp[(meth, s, e)] = ('renum', hit) if hit else ('delete', None)
    ov_del = set()
    if cfg_kind == 'formbridge':
        for meth, old, new in cfg.get('overrides', []):
            if old not in bodies9.get(meth, ''):
                ov_del.add((meth, old))
    return bp, ov_del, missing906


# 函式裡寫死的 golden 行號（不是 block tuple，AST 規則抓不到）。每條：(設定檔, golden 檔, [(912 起, 迄, 906 起)], [(舊字串, 新字串)])。
# 套用前逐行驗證：912 的 [起..迄] 與 906 的 [新起..] 內容（strip 後）完全相同，否則中止。
SPECIAL = [
    ('tools/editlist/Temperature.py', 'cTesterIF.cpp', [(566, 570, 565), (713, 779, 690)], [
        ('a, b = 713, 779', 'a, b = 690, 756'),
        ("in _TIF[568] and 'Tester.Data' in _TIF[569]", "in _TIF[567] and 'Tester.Data' in _TIF[568]"),
        ('list(range(566, 571))', 'list(range(565, 570))'),
    ]),
]


# formbridge「整段覆寫」在 906 對不到、要照 906 原文重寫的：{(設定檔, 方法, 912 起, 912 迄): (906 起, 906 迄, 新 tuple 原始碼)}。
# 套用前驗證 906 起行含 must 字串（產生器自己也會驗）。
REWRITE = {
    ('tools/formbridge/TfSetup.py', 'ScrollBar1Change', 1071, 1075): (1065, 1068,
        "('ScrollBar1Change', 1065, 1068, 'cb16DirectHeater->Visible=(USE', "
        "'J.SetVisible(\"cb16DirectHeater\", (USE_16_HEATER==eht32HeaterEJ1N || USE_16_HEATER==eht32HeaterKT4H || "
        "(ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16)));   "
        "//Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 "
        "//Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關   "
        "/* golden 906 cSetUp.cpp:1065-1068（§15 R32：沒有 V912 RogerYang 20260812 補的 DTME08） */')"),
    ('tools/formbridge/TfSetup.py', 'ScrollBar1Change', 1083, 1085): (1076, 1077,
        "('ScrollBar1Change', 1076, 1077, 'cb12Site10DirectHeater->Visibl', "
        "'J.SetVisible(\"cb12Site10DirectHeater\", (USE_16_HEATER==eht32HeaterEJ1N || USE_16_HEATER==eht32HeaterKT4H));   "
        "//Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器   "
        "/* golden 906 cSetUp.cpp:1076-1077（§15 R32：同上） */')"),
}


def apply_special(rel, src, g906, g912, cp):
    for f, gfile, ranges, reps in SPECIAL:
        if f != rel:
            continue
        g9 = cp(os.path.join(g906, gfile)).replace('\r\n', '\n').split('\n')
        g2 = cp(os.path.join(g912, gfile)).replace('\r\n', '\n').split('\n')
        for s, e, n in ranges:
            if [x.strip() for x in g2[s - 1:e]] != [x.strip() for x in g9[n - 1:n - 1 + e - s + 1]]:
                raise SystemExit('SPECIAL %s: %s 912 :%d-%d != 906 :%d-%d' % (rel, gfile, s, e, n, n + e - s))
        for old, new in reps:
            if src.count(old) != 1:
                raise SystemExit('SPECIAL %s: %r occurs %d times (expected 1)' % (rel, old, src.count(old)))
            src = src.replace(old, new)
    return src


def _is_call(n, names):
    return isinstance(n, ast.Call) and isinstance(n.func, ast.Name) and n.func.id in names


def edit_config(src, bp, ov_del, drop_methods, methods, ns, kind, rel='', line906=None, absent906=None):
    """ns：同一支設定檔以 912 路徑 exec 出來的命名空間 —— 用來求出 L(...)／_at(...)／RANGE(...) 這類呼叫在 912 的值，
    才知道一個 tuple 對應 bp 的哪一條。"""
    tree = ast.parse(src)
    buf = Buf(src)
    lines = src.split('\n')
    seen = set()
    rep = {'renum': 0, 'delete': 0, 'ov_delete': 0, 'comment_fix': 0, 'method_drop': 0, 'auto': 0, 'member_drop': 0}
    manual = []

    def ev(n):
        if const_int(n):
            return n.value
        # 只對「參數全是字面值的函式呼叫」求值（L('m','text')、_at(26,'x')、RANGE(...)）。
        # ⚠ 不可以對名稱求值：迴圈變數（TrayForm 的 _ln）在 exec 完只剩最後一個值，會把整個迴圈誤算成一條。
        if not (isinstance(n, ast.Call) and isinstance(n.func, ast.Name)
                and all(isinstance(x, ast.Constant) for x in n.args) and not n.keywords):
            return None
        try:
            v = eval(compile(ast.fix_missing_locations(ast.Expression(body=n)), '<cfg>', 'eval'), ns)
        except BaseException:
            return None
        return v if isinstance(v, int) and not isinstance(v, bool) else None

    def delete_node(node):
        s = buf.off(node.lineno, node.col_offset)
        e = buf.off(node.end_lineno, node.end_col_offset)
        j = e
        while j < len(buf.b) and buf.b[j:j + 1] in (b' ', b'\t'):
            j += 1
        if buf.b[j:j + 1] == b',':
            e = j + 1
        ls, le = buf.line_span(node.lineno)
        before = buf.b[ls:s]
        ee, eend = buf.line_span(node.end_lineno)
        after = buf.b[e:eend]
        if before.strip() == b'' and after.strip() in (b'', b'\r'):
            s = ls
            e = eend
            ind = before
            k = node.lineno - 1
            while k >= 1:
                t = lines[k - 1].rstrip('\r')
                if (t.strip().startswith('#') and t[:len(t) - len(t.lstrip())].encode('utf-8') == ind
                        and not re.match(r'\s*#\s*[-=]{3,}', t)):     # 段落標題（# ---- X ----）不是這條的原因註解，留著
                    s = buf.line_span(k)[0]
                    k -= 1
                else:
                    break
        buf.add(s, e, b'')

    def replace_int(n, val):
        """n 是字面整數，或 _at(字面整數, ...)：換掉那個整數。其他寫法（L()、RANGE()…按內容定位）不動、回 False。"""
        if const_int(n):
            buf.add(buf.off(n.lineno, n.col_offset), buf.off(n.end_lineno, n.end_col_offset), str(val))
            return True
        if _is_call(n, {'_at'}) and n.args and const_int(n.args[0]):
            t = n.args[0]
            buf.add(buf.off(t.lineno, t.col_offset), buf.off(t.end_lineno, t.end_col_offset), str(val))
            return True
        return False

    def fix_strings(node, s912, e912, a, b):
        for elt in node.elts[3:]:
            if const_str(elt):
                seg = src.encode('utf-8')[buf.off(elt.lineno, elt.col_offset):buf.off(elt.end_lineno, elt.end_col_offset)].decode('utf-8')
                new = seg.replace(':%d-%d' % (s912, e912), ':%d-%d' % (a, b))
                new = re.sub(r':%d(?!\d)' % s912, ':%d' % a, new)
                if new != seg:
                    buf.add(buf.off(elt.lineno, elt.col_offset), buf.off(elt.end_lineno, elt.end_col_offset), new)
                    rep['comment_fix'] += 1

    def fix_comments_above(node, s912, e912, a, b):
        k = node.lineno - 1
        while k >= 1:
            t = lines[k - 1]
            if not t.strip().startswith('#'):
                break
            new = t.replace(':%d-%d' % (s912, e912), ':%d-%d' % (a, b))
            if new != t:
                ls, le = buf.line_span(k)
                buf.add(ls, ls + len(t.encode('utf-8')), new)
                rep['comment_fix'] += 1
            k -= 1

    for node in ast.walk(tree):
        if not isinstance(node, ast.Tuple) or len(node.elts) < 3 or not const_str(node.elts[0]):
            continue
        meth = node.elts[0].value
        if meth not in methods:
            continue
        if len(node.elts) >= 4:
            v1, v2 = ev(node.elts[1]), ev(node.elts[2])
            if v1 is None or v2 is None:
                continue
            key = (meth, v1, v2)
            if key not in bp or key in seen:
                continue
            seen.add(key)
            act, hit = bp[key]
            rw = REWRITE.get((rel,) + key)
            if rw and kind == 'formbridge':
                must = ast.literal_eval(rw[2])[3]
                if line906 is None or must not in line906(rw[0]):
                    raise SystemExit('REWRITE %s %r: golden 906 :%d does not contain %r' % (rel, key, rw[0], must))
                buf.add(buf.off(node.lineno, node.col_offset), buf.off(node.end_lineno, node.end_col_offset), rw[2])
                rep['rewrite'] = rep.get('rewrite', 0) + 1
                continue
            if act == 'delete' and kind == 'formbridge':
                manual.append((key, None, '整段覆寫的 912 原文在 906 對不到（906 那段寫法不同）→ 要照 906 原文重寫或拿掉'))
                continue
            if act == 'delete':
                delete_node(node); rep['delete'] += 1
                continue
            if hit == (v1, v2):
                continue
            resized = (hit[1] - hit[0]) != (v2 - v1) or any(x[0] == v1 and x[1] == v2 for x in P.MAP_APPROX)
            if kind == 'formbridge' and resized:
                manual.append((key, hit, '整段覆寫的內容是照 912 寫的，906 段落長度或位置不同 → 要照 906 原文重寫'))
                continue
            d1 = replace_int(node.elts[1], hit[0])
            d2 = replace_int(node.elts[2], hit[1])
            if not d1 and not d2:
                rep['auto'] += 1           # 按內容定位（L／RANGE／_find），換 golden 就會自己算對
                continue
            if d1 != d2:
                manual.append((key, hit, '起迄只有一邊是字面數字'))
            rep['renum'] += 1
        elif len(node.elts) == 3 and const_str(node.elts[1]) and (meth, node.elts[1].value) in ov_del:
            delete_node(node); rep['ov_delete'] += 1
    # 字典的鍵、range() 的參數：只換「還沒對到 tuple、而且是單行 renum」的那些 912 行號（例 TrayForm 的 _PTR、ShowCompnet 迴圈）
    rest = {k[1]: bp[k][1][0] for k in bp if k not in seen and k[1] == k[2] and bp[k][0] == 'renum' and bp[k][1][0] == bp[k][1][1]}
    used = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Dict):
            for kn in node.keys:
                if kn is not None and const_int(kn) and kn.value in rest:
                    if rest[kn.value] != kn.value:
                        replace_int(kn, rest[kn.value]); rep['renum'] += 1
                    used.add(kn.value)
        elif _is_call(node, {'range'}) and len(node.args) == 2 and all(const_int(x) for x in node.args):
            lo, hi = node.args[0].value, node.args[1].value
            if lo in rest and (hi - 1) in rest and rest[hi - 1] - rest[lo] == hi - 1 - lo:
                if rest[lo] != lo:
                    replace_int(node.args[0], rest[lo]); replace_int(node.args[1], rest[hi - 1] + 1); rep['renum'] += hi - lo
                used.update(range(lo, hi))
    for k in list(bp):
        if k not in seen and k[1] in used:
            seen.add(k)
    for node in ast.walk(tree):
        if isinstance(node, ast.List):
            for elt in node.elts:
                if const_str(elt) and elt.value in drop_methods:
                    # 清單中間的一個元素：連同前面的空白一起拿掉，免得留下行尾空白（git apply 會警告）
                    s = buf.off(elt.lineno, elt.col_offset)
                    e = buf.off(elt.end_lineno, elt.end_col_offset)
                    if buf.b[e:e + 1] == b',':
                        e += 1
                    while s > 0 and buf.b[s - 1:s] == b' ':
                        s -= 1
                    buf.add(s, e, b'')
                    rep['method_drop'] += 1
    for node in ast.walk(tree):
        if isinstance(node, ast.Dict):
            for kn, vn in zip(node.keys, node.values):
                if kn is not None and const_str(kn) and kn.value in drop_methods:
                    s = buf.off(kn.lineno, kn.col_offset)
                    e = buf.off(vn.end_lineno, vn.end_col_offset)
                    if buf.b[e:e + 1] == b',':
                        e += 1
                    while buf.b[e:e + 1] == b' ':
                        e += 1
                    buf.add(s, e, b'')
                    rep['method_drop'] += 1
                if kn is not None and const_str(kn) and kn.value == 'members' and absent906 is not None:
                    # 值可能是 [...] + [...] 的串接（TestIF_File_YieldMonitoring.py）⇒ 運算式裡每個 list 常數都看
                    for lst in [x for x in ast.walk(vn) if isinstance(x, ast.List)]:
                        for elt in lst.elts:
                            if const_str(elt) and absent906(elt.value):
                                delete_node(elt)
                                rep['member_drop'] = rep.get('member_drop', 0) + 1
    unmatched = [k for k in bp if k not in seen]
    return buf.result(), rep, unmatched, manual



# ---------------------------------------------------------------------------
# 第二階段：註解與字串裡的 golden 行號（審查 A #1/#2/#4、B #3）。
#   只換「在兩棵樹逐行對齊時是同一行」的引用；V912 專屬的行（對不到）原樣留著 —— 那是在描述 V912 的程式碼。
#   明寫檔名（X.cpp:N）照檔名換；只寫「:N」的，視為這支設定檔自己的 golden cpp。
#   前面 16 字內有「移植樹／port／FileRW／906」的不換（那是移植樹或已經是 906 的行號）。
# ---------------------------------------------------------------------------
_CITE_FILE = re.compile(r'(?P<file>[A-Za-z_]\w*\.(?:cpp|h|hpp|dfm))\s*:\s*(?P<a>\d{1,6})(?:\s*[-～~]\s*:?(?P<b>\d{1,6}))?')
_CITE_BARE = re.compile(r'(?:(?<=[\s（(、／＋,，=])|^)[:：](?P<a>\d{2,6})(?:\s*[-～~]\s*:?(?P<b>\d{2,6}))?')
_SKIP_BEFORE = ('移植樹', '移植', 'port', 'Port', 'FileRW', '906', 'wb_serve', 'WebBridge')


class GoldenMaps:
    def __init__(self, g906, g912, cp):
        self.g906, self.g912, self.cp, self.cache = g906, g912, cp, {}

    def get(self, fname):
        if fname in self.cache:
            return self.cache[fname]
        m = None
        p9, p2 = os.path.join(self.g906, fname), os.path.join(self.g912, fname)
        if os.path.isfile(p9) and os.path.isfile(p2):
            a = [P.norm(x) for x in self.cp(p2).replace('\r\n', '\n').split('\n')]
            b = [P.norm(x) for x in self.cp(p9).replace('\r\n', '\n').split('\n')]
            m = {}
            for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
                if tag == 'equal':
                    for k in range(i2 - i1):
                        m[i1 + k + 1] = j1 + k + 1
        self.cache[fname] = m
        return m


def renumber_citations(text, default_cpp, maps, stats):
    import tokenize
    lines = text.split('\n')
    starts = [0]
    for l in lines:
        starts.append(starts[-1] + len(l.encode('utf-8')) + 1)
    edits = []   # (byte start, byte end, new)
    try:
        toks = list(tokenize.generate_tokens(io.StringIO(text).readline))
    except (tokenize.TokenError, SyntaxError):
        return text

    def boff(row, col):
        return starts[row - 1] + len(lines[row - 1][:col].encode('utf-8'))

    for t in toks:
        if t.type not in (tokenize.COMMENT, tokenize.STRING):
            continue
        s_tok = t.string
        (r0, c0) = t.start
        # 把 token 內的字元位置換成 (row, col)
        pos_rc = []
        r, c = r0, c0
        for ch in s_tok:
            pos_rc.append((r, c))
            if ch == '\n':
                r, c = r + 1, 0
            else:
                c += 1
        for rx, has_file in ((_CITE_FILE, True), (_CITE_BARE, False)):
            for m in rx.finditer(s_tok):
                before = s_tok[max(0, m.start() - 16):m.start()]
                if any(w in before for w in _SKIP_BEFORE):
                    stats['skip'] += 1
                    continue
                fname = m.group('file') if has_file else default_cpp
                mp = maps.get(fname)
                if not mp:
                    continue
                a0 = int(m.group('a')); b0 = int(m.group('b')) if m.group('b') else None
                if a0 not in mp:
                    stats['unmapped'] += 1
                    continue
                na = mp[a0]
                nb = None
                if b0 is not None:
                    if b0 in mp:
                        nb = mp[b0]
                    else:
                        inner = [k for k in range(a0, b0 + 1) if k in mp]
                        nb = mp[inner[-1]] if inner else na
                if na == a0 and (b0 is None or nb == b0):
                    continue
                for grp, new in (('a', na), ('b', nb)):
                    if new is None or m.group(grp) is None or int(m.group(grp)) == new:
                        continue
                    i0, i1 = m.start(grp), m.end(grp)
                    rr, cc = pos_rc[i0]
                    re_, ce = pos_rc[i1 - 1]
                    edits.append((boff(rr, cc), boff(re_, ce) + len(s_tok[i1 - 1].encode('utf-8')), str(new)))
                stats['fixed'] += 1
    b = text.encode('utf-8')
    seen = set()
    for s0, e0, new in sorted(edits, key=lambda x: -x[0]):
        if (s0, e0) in seen:
            continue
        seen.add((s0, e0))
        b = b[:s0] + new.encode('utf-8') + b[e0:]
    return b.decode('utf-8')


def drop_dead_private(orig, text, stats):
    """模組層級的 _名稱 = 字串常數：原本有人用、刪完 block 之後沒人用了 ⇒ 一起刪（審查 A：HSys _HEATER_TODO_*）。"""
    def refs(src):
        t = ast.parse(src)
        c = {}
        for n in ast.walk(t):
            if isinstance(n, ast.Name) and isinstance(n.ctx, ast.Load):
                c[n.id] = c.get(n.id, 0) + 1
        return t, c
    t0, c0 = refs(orig)
    t1, c1 = refs(text)
    buf = Buf(text)
    lines = text.split('\n')
    for node in t1.body:
        if (isinstance(node, ast.Assign) and len(node.targets) == 1 and isinstance(node.targets[0], ast.Name)
                and node.targets[0].id.startswith('_') and c0.get(node.targets[0].id, 0) > 0
                and c1.get(node.targets[0].id, 0) == 0):
            try:
                v = ast.literal_eval(node.value)
            except Exception:
                continue
            if not isinstance(v, str):
                continue
            s0 = buf.line_span(node.lineno)[0]
            e0 = buf.line_span(node.end_lineno)[1]
            buf.add(s0, e0, b'')
            stats['dead_const'] += 1
    return buf.result()


def add_guard(text, rel, stats):
    """golden 路徑那一行後面加一行：找不到 golden 就明講（審查 A #3：gen_sjson 會靜默產空表）。"""
    out = []
    for l in text.split('\n'):
        out.append(l)
        m = re.match(r'^(?P<ind>\s*)(?P<name>[A-Za-z_]\w*)\s*=\s*\(os\.environ\.get\(\'HT9045_GOLDEN_ROOT\'\)', l)
        if m:
            out.append("%sif not os.path.isfile(os.path.join(%s, 'cSetUp.cpp')): raise SystemExit('%s: golden 906 not found at %%s -- set HT9045_GOLDEN_ROOT to the golden tree' %% %s)   "
                       "# AI(W906-GOLDEN906) 20260925 (NB2 R39): 找不到 golden 就中止，不要靜默產空表"
                       % (m.group('ind'), m.group('name'), rel.split('/')[-1], m.group('name')))
            stats['guard'] += 1
    return '\n'.join(out)


def check_g912(repo, g912, cp):
    """強自我檢查：repo 裡已產生的 FileRW 檔記著「// golden X.cpp:N  Cls::Meth(」，那是產生器當時用的 V912 的方法行號。
    g912 必須每一個都對得上（±1），否則行號換算的基準就錯了。
    ⚠ 20260925 實測：產生器用的不是公司原版 6943d134，而是含我們 V912 維護修改（d37f50ff cContact +2、
    uTemp_Set +27、acbcf268 uYieldMonitoring）的那一版；拿原版當基準，DeviceForm 的 UpdateContactRelative 會換錯行、編譯失敗。"""
    from collections import defaultdict
    want = defaultdict(set)
    rx = re.compile(r'^// golden (\S+?\.cpp):(\d+)\s+(\w+)::(~?\w+)\(')
    fw = os.path.join(repo, 'FileRW')
    for f in os.listdir(fw):
        if f.endswith('.gen.inc') or f in ('TestIF_File.cpp', 'HotPlateForm_File.cpp'):
            for l in open(os.path.join(fw, f), encoding='utf-8', errors='replace'):
                m = rx.match(l)
                if m:
                    want[m.group(1)].add((m.group(3), m.group(4), int(m.group(2))))
    bad, tot = [], 0
    for gf, items in want.items():
        txt = cp(os.path.join(g912, gf)).replace('\r\n', '\n').split('\n')
        defs = defaultdict(list)
        for i, t in enumerate(txt):
            m = re.match(r'^[^/\n]*?\b(\w+)::(~?\w+)\s*\(', t)
            if m:
                defs[(m.group(1), m.group(2))].append(i + 1)
        for cls, meth, ln in items:
            tot += 1
            if not any(abs(c - ln) <= 1 for c in defs.get((cls, meth), [])):
                bad.append('%s %s::%s 產出記 :%d，g912 是 %s' % (gf, cls, meth, ln, defs.get((cls, meth), [])[:3]))
    return tot, bad


def load_ns(path, g912):
    src = open(path, encoding='utf-8').read().replace(REF912, g912)
    ns = {'__file__': path, '__name__': 'cfg'}
    exec(compile(src, os.path.basename(path), 'exec'), ns)
    return ns


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', help='port-tree COPY to edit in place (HT9011UC_Cpp_V3.33.906.0 level)')
    ap.add_argument('--diff', help='write a git-apply patch against this repo instead (touches nothing)')
    ap.add_argument('--g906', default=G906)
    ap.add_argument('--g912', default=REF912 if os.path.isdir(REF912) else LOC912)
    a = ap.parse_args()
    if not a.root and not a.diff:
        sys.exit('need --root <copy> or --diff <out.patch>')
    repo = os.path.abspath(os.path.join(HERE, '..', '..'))
    root = os.path.abspath(a.root) if a.root else repo
    el = P.gen_namespace('gen_editlist.py', 'STRUCTS = load_structs()', '\nONLY = sys.argv')
    fb = P.gen_namespace('gen_formbridge.py', 'FORMS = load_forms()', '\nmain()')
    tot, bad = check_g912(repo, a.g912, el['cp950'])
    print('g912 強自我檢查：%d 個方法，對不上 %d 個' % (tot, len(bad)))
    if bad:
        for b in bad[:10]:
            print('   ' + b)
        sys.exit('g912 不是產生器當時用的那一版 V912。用 git archive acbcf268（或 acbcf268..4309bbd2 任一顆）解出來，再用 --g912 指過去。')
    new_text, report = {}, []
    global HANDWRITTEN
    _gen = {'TestIF_File.cpp', 'HotPlateForm_File.cpp', '_registry.cpp'}
    _fw = os.path.join(repo, 'FileRW')
    HANDWRITTEN = '\n'.join(open(os.path.join(_fw, f), encoding='utf-8', errors='replace').read()
                            for f in os.listdir(_fw)
                            if f.endswith(('.cpp', '.h')) and not f.endswith('.gen.inc') and f not in _gen)
    gmaps = GoldenMaps(a.g906, a.g912, el['cp950'])
    cstats = {'fixed': 0, 'unmapped': 0, 'skip': 0, 'dead_const': 0, 'guard': 0}
    for kind, sub, var in (('editlist', 'tools/editlist', 'STRUCT'), ('formbridge', 'tools/formbridge', 'FORM')):
        for fn, cfg, err in P.load_cfgs(sub.split('/')[1], var, a.g912):
            rel = sub + '/' + fn
            if err or not cfg:
                report.append('SKIP %s: %s' % (rel, err)); continue
            bp, ov_del, miss906 = plan_for(kind, cfg, el, fb, a.g906, a.g912)
            src = open(os.path.join(root, rel.replace('/', os.sep)), encoding='utf-8', newline='').read()
            ns = load_ns(os.path.join(repo, rel.replace('/', os.sep)), a.g912)
            _L9 = el['cp950'](os.path.join(a.g906, cfg['cpp'])).replace('\r\n', '\n').split('\n')
            _T9 = el['cp950'](os.path.join(a.g906, cfg['cpp'])) + el['cp950'](os.path.join(a.g906, cfg['h']))
            _T2 = el['cp950'](os.path.join(a.g912, cfg['cpp'])) + el['cp950'](os.path.join(a.g912, cfg['h']))

            def _absent906(decl, _T9=_T9, _T2=_T2):
                code = decl.split('//')[0].strip()
                if not code or code.startswith('#'):
                    return False
                # 只取「被宣告的名稱」：去掉型別前綴，逗號分開，每段取第一個識別字（不取 =false 的 false）
                body = re.sub(r'^(?:(?:unsigned|signed|const|static|volatile)\s+)*[A-Za-z_]\w*(?:\s*<[^>]*>)?[\s*&]+', '', code.rstrip(';').strip())
                names = [mm.group(1) for mm in (re.match(r'\s*\**\s*([A-Za-z_]\w*)', part) for part in body.split(',')) if mm]
                if re.match(r'^\w+$', code):
                    names = [code]
                # 手寫的 FileRW 入口檔還在用的不刪（例 FileRW/BinSelect.cpp:420 bCountSetPassed —— 那段本身是 V912 rf360，
                #   要不要一起拿掉是手寫碼的事，列在報告裡）
                if any(re.search(r'\b%s\b' % re.escape(n), HANDWRITTEN) for n in names):
                    return False
                return bool(names) and all(not re.search(r'\b%s\b' % re.escape(n), _T9) and re.search(r'\b%s\b' % re.escape(n), _T2)
                                          for n in names)
            out, rep, unmatched, manual = edit_config(src, bp, ov_del, set(miss906), set(cfg['methods']), ns, kind,
                                                      rel, lambda n, _L9=_L9: _L9[n - 1], _absent906)
            out = renumber_citations(out, cfg['cpp'], gmaps, cstats)
            out = drop_dead_private(src, out, cstats)
            out = apply_special(rel, out, a.g906, a.g912, el['cp950'])
            new_text[rel] = out
            report.append('%-45s renum=%d auto=%d delete=%d ov_delete=%d rewrite=%d method_drop=%d member_drop=%d%s%s%s' % (
                rel, rep['renum'], rep['auto'], rep['delete'], rep['ov_delete'], rep.get('rewrite', 0), rep['method_drop'], rep['member_drop'],
                ('\n      UNMATCHED（沒找到對應的 tuple，要人工）=%r' % unmatched) if unmatched else '',
                ''.join('\n      MANUAL %r -> 906 %r：%s' % m for m in manual),
                ('\n      906 沒有的方法=%r（已從 methods 拿掉）' % miss906) if miss906 else ''))
    for rel in PATHFILES:
        src = new_text.get(rel) or open(os.path.join(root, rel.replace('/', os.sep)), encoding='utf-8', newline='').read()
        n = 0
        for old in (REF912, LOC912):
            lit = "r'%s'" % old
            n += src.count(lit)
            src = src.replace(lit, NEWEXPR)
        src = add_guard(src, rel, cstats)      # 引用行號只換設定檔（上面的迴圈做過）；產生器本身的說明註解不動
        new_text[rel] = src
        report.append('%-45s golden path replaced x%d' % (rel, n))
    changed = {k: v for k, v in new_text.items()
               if v != open(os.path.join(root, k.replace('/', os.sep)), encoding='utf-8', newline='').read()}
    if a.root:
        for rel, txt in changed.items():
            open(os.path.join(root, rel.replace('/', os.sep)), 'w', encoding='utf-8', newline='').write(txt)
    if a.diff:
        chunks = []
        for rel in sorted(changed):
            old = open(os.path.join(repo, rel.replace('/', os.sep)), encoding='utf-8', newline='').read()
            path = 'HT9011UC_Cpp_V3.33.906.0/' + rel
            d = difflib.unified_diff(old.splitlines(True), changed[rel].splitlines(True), 'a/' + path, 'b/' + path)
            chunks.append(''.join(d))
        open(a.diff, 'w', encoding='utf-8', newline='').write(''.join(chunks))
    print('\n'.join(report))
    print('citations renumbered=%(fixed)d unmapped(V912-only, kept)=%(unmapped)d skipped(port/906)=%(skip)d '
          'dead private constants dropped=%(dead_const)d golden guards added=%(guard)d' % cstats)
    print('files changed: %d' % len(changed))


if __name__ == '__main__':
    main()
