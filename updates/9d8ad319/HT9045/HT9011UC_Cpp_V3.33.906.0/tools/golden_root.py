# -*- coding: utf-8 -*-
# AI(W906-E031) 20261003 [W906] (St01)：新檔（todo E-031 第一階段）。三支產生器（gen_editlist／gen_formbridge／gen_sjson）
#   與 tools/editlist、tools/formbridge 的設定檔「讀哪一棵 golden」集中在這一支。
#   Jimmy RULINGS_20261003 第 4 條（golden_root 由 St01 負責、照 St01 的排程）、第 2 條（golden＝906 0618，0625 只對照）、
#   第 1 條（V912 是修正或明顯比較好的留 V912，兩邊行號都寫、帳本記一列）；RULINGS_20261002 第 20a 條（溫控照 V912）。
"""golden_root.py -- 產生器讀的 golden 樹（906 0618／V912）。

兩棵樹
  '906'   golden 906 0618。cp950，不在 git（0618 是加密 7z 發的，RULINGS_20261001 第 19 條；放進 git 等於繞過那層保護）。
          位置：環境變數 W906_GOLDEN_ROOT → HT9045_GOLDEN_ROOT（tools/dfm2rc、gen_teach_editlist、gate 範本用的同一個，指 0618）
                → 預設 G906_SHOWN（D:\\HT9045\\backup\\HT9011UC_Code_V3.33.906.0_20260618）。
          釘住：PINS 列的檔（NB2 golden_fingerprint v1 的 text hash，跟 NB2 公布的 fp_0618 一字不差）在第一次用到 906 時全部核對，
                有一個不符就中止 —— 0625、解壓不完整、被改過的樹都讀不進來。產生檔印的是 G906_SHOWN，不是本機路徑，
                所以樹在哪台機器、放哪個資料夾，產出都一樣（E-030 報告 §7 的「(a) 只用環境變數、兩台產出不一樣」由 PINS 補上）。
  'v912'  V912（HT9011UC_Code_V3.33.912.0_20260908_Jimmy，在 git）。預設 V912_SHOWN；環境變數 W906_V912_ROOT 可改（產生檔照印
          V912_SHOWN）。不釘：Jimmy 會改這棵（例 main c2f6c75a 的 main.cpp +30 行），它的漂移由 git 管。

哪個結構讀哪一棵（tree_of）
  1. 設定檔 STRUCT／FORM 的 'golden'：'906' 或 'v912'；
  2. 沒寫 → KEEP_V912 有列 → 'v912'；
  3. 其他 → DEFAULT_TREE。第一階段＝'v912'：沒寫 'golden' 的結構產出一字不變；全面切換時改成 '906'。
  KEEP_V912 的結構寫 'golden': '906' → 中止（裁決的例外不能被單一設定檔推翻；要改先改這張表、寫理由）。

單一方法留 V912（method_trees）
  設定檔 'golden_methods' = {方法: 'v912'}：整支方法照 V912（第 1 條保留，例 V912 才有的方法）；KEEP_V912_METHODS 列的方法一律 V912。
  改讀另一棵的方法，產生檔的 golden 標籤多一個樹名（`// golden V912 HandlerSys.cpp:1519 …`），它的 replace／blocks 列寫那一棵的行號，
  它用到、結構那棵 header 沒宣告的元件從那一棵的 header 補。只差幾行的保留不要整支換樹：用 replace 列（取代碼＝V912 寫法、三段註解，
  .claude/skills/ht9045-html-json/references/route-c-golden-bridge.md 的 E-032／E-031 兩條）。

設定檔自己讀 golden（_expect／L()／_span…）一律經過這裡，行號才會跟產生器一樣：
    import golden_root as _GR                        # AI(W906-E031)
    _TREE = _GR.tree_of('<struct>', '906')           # 跟 STRUCT['golden'] 同一個值
    _cpp = _GR.lines(_TREE, 'cTrayAssignment.cpp')   # cp950 解碼、\\r\\n→\\n、split('\\n')（產生器的行號）
  選了 906 的設定檔裡若還有字串寫著 V912 樹的資料夾名，產生器中止（check_config_source）——否則設定檔釘的是 V912 的行、
  產生器改的是 0618 的同一個行號，會靜默蓋錯地方。

指令（在 HT9011UC_Cpp_V3.33.906.0 底下跑；python＝Python314 絕對路徑）
  python tools/golden_root.py check                  兩個根目錄、從哪個環境變數來、PINS 逐檔核對
  python tools/golden_root.py map <檔> <V912行>...    V912 行號 → 0618 行號（difflib；只有 equal 段給號碼，落在不同段回 DIFF）
  python tools/golden_root.py blocks <檔>            V912 → 0618 的 difflib 段落（1 起，只列不同的段）
"""
import difflib
import hashlib
import io
import os
import sys
import tokenize

V912_SHOWN = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
G906_SHOWN = r'D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618'
SHOWN = {'906': G906_SHOWN, 'v912': V912_SHOWN}
TAG = {'906': '906', 'v912': 'V912'}          # 產生檔的 golden 標籤用（只有方法改讀另一棵時才印）
ENV_906 = ('W906_GOLDEN_ROOT', 'HT9045_GOLDEN_ROOT')
ENV_V912 = 'W906_V912_ROOT'
V912_DIRNAME = 'HT9011UC_Code_V3.33.912.0_20260908_Jimmy'

# 第一階段（E-031）＝'v912'：只有寫了 'golden': '906' 的結構換樹。全面切換＝改成 '906'（見 route-c-golden-bridge.md E-031 的順序表）。
DEFAULT_TREE = 'v912'

# 一律留 V912 的結構（產生器的結構名：editlist 的 STRUCT['struct']、formbridge 的 FORM['struct']、gen_sjson 的結構名）→ 理由。
KEEP_V912 = {
    'Temperature': 'RULINGS_20261002 #20a temperature (golden uTemp_Set.cpp; E-027)',
    'SYSTEM_TEMPERATURE': 'RULINGS_20261002 #20a temperature (gen_sjson ini map of the Temperature struct)',
    'HSys': 'RULINGS_20261002 #20a HandlerSys Heater tab (E-029: ctor :70-112, LoaderSystemSet :262-278, SaveSystemSet :779-794, '
            'rgHeaterTypeClick :1519 V912-only) -- whole struct stays V912 until the full switch splits it (KEEP_V912_METHODS)',
    'ACTForm': 'RULINGS_20261002 #20a temperature (golden AutoTemperature.cpp, auto K-temp calibration; same file in 0618 and V912)',
    'Winway': 'RULINGS_20261002 #20a temperature (golden ATC/WinWaySetting.cpp, ATC WinWay set temperature; same file in 0618 and V912)',
}

# 結構換到 906 之後仍然一律留 V912 的方法 → 理由（HSys 現在整個在 KEEP_V912，這一列是全面切換時拆開用的）。
KEEP_V912_METHODS = {
    'HSys': {'rgHeaterTypeClick': 'RULINGS_20261002 #20a heater (E-029; V912-only method)'},
}

# 906 0618 的釘子：NB2 golden_fingerprint v1 的 FILE text hash（cp950 解碼、\r\n／\r→\n、每行去尾端空白，sha1 前 16 碼）。
# 值與 NB2 公布的 fp_0618_nb2.tsv（D:\AI_TempFile\st01e-e032\）逐檔一樣；前 8 個 0618／0625／V912 三棵都不同（認得出拿錯樹），
# Magazine.cpp 是 E-031 試行結構讀的檔。
PINS = {
    'main.cpp': '9c532c033fe887e2',
    'ckernel.cpp': 'cc13274e443bd1ca',
    'cConfiguration.cpp': '903d1bbd9d6874fc',
    'cSetUp.cpp': '1851a72a9681b114',
    'cTrayAssignment.cpp': '3916f16d9c71a7e0',
    'cContact.cpp': 'b7d1277d2bee1b1c',
    'cmydef.h': '563317891555f2d9',
    'Config.h': '4809f5bbe971c512',
    'Magazine.cpp': 'fec6a9513c2639f3',
}

TREES = ('906', 'v912')
_PINNED = {}      # 已核對過的 906 根目錄
_TEXT = {}        # (樹, 檔) → 解碼後全文


def _check_tree(tree):
    if tree not in TREES:
        raise SystemExit("golden_root: unknown tree %r (use '906' or 'v912')" % (tree,))


def env_root(tree):
    """(路徑, 來源)：來源是環境變數名，或 None（預設路徑）。不核對。"""
    _check_tree(tree)
    if tree == '906':
        for k in ENV_906:
            v = os.environ.get(k)
            if v:
                return v, k
        return G906_SHOWN, None
    v = os.environ.get(ENV_V912)
    return (v, ENV_V912) if v else (V912_SHOWN, None)


def text_hash(raw):
    """NB2 golden_fingerprint v1 的 text hash（D:\\AI_TempFile\\st01e-e032\\golden_fingerprint.py norm_text＋sha）。"""
    t = raw.decode('cp950', errors='replace')
    t = '\n'.join(l.rstrip() for l in t.replace('\r\n', '\n').replace('\r', '\n').split('\n'))
    return hashlib.sha1(t.encode('utf-8', 'surrogatepass')).hexdigest()[:16]


def verify_pins(path):
    """[(檔, 期望, 實際或 'missing')]：不符的才列。"""
    bad = []
    for rel, want in sorted(PINS.items()):
        p = os.path.join(path, rel)
        if not os.path.isfile(p):
            bad.append((rel, want, 'missing'))
            continue
        got = text_hash(open(p, 'rb').read())
        if got != want:
            bad.append((rel, want, got))
    return bad


def root(tree):
    """這一棵在本機的路徑。906 第一次用到時核對 PINS（不符就中止）。"""
    path, src = env_root(tree)
    if not os.path.isdir(path):
        raise SystemExit('golden_root: %s tree not found at %s%s' % (
            TAG[tree], path, (' (from %s)' % src) if src else
            (' -- set %s to the golden 906 0618 tree' % ENV_906[0] if tree == '906' else ' -- set %s' % ENV_V912)))
    if tree == '906' and path not in _PINNED:
        bad = verify_pins(path)
        if bad:
            raise SystemExit('golden_root: %s%s is not golden 906 0618 -- pinned files differ:\n%s' % (
                path, (' (from %s)' % src) if src else '',
                '\n'.join('  %s: expected %s, got %s' % b for b in bad)))
        _PINNED[path] = True
    return path


def shown(tree):
    """產生檔印的路徑（固定字串，不是本機路徑）。"""
    _check_tree(tree)
    return SHOWN[tree]


def path(tree, rel):
    return os.path.join(root(tree), rel)


def text(tree, rel):
    """golden 檔全文：cp950 解碼（errors='replace'），\\r\\n 原樣 —— 跟產生器的 cp950() 一樣，行號＝count('\\n')+1。"""
    key = (tree, rel)
    if key not in _TEXT:
        _TEXT[key] = open(path(tree, rel), 'rb').read().decode('cp950', errors='replace')
    return _TEXT[key]


def lines(tree, rel):
    """設定檔用的行陣列（\\r\\n→\\n、split('\\n')；第 n 行＝[n-1]）。"""
    return text(tree, rel).replace('\r\n', '\n').split('\n')


def tree_of(struct, asked=None):
    """結構讀哪一棵：'golden' 有寫照寫（KEEP_V912 不准寫 906）；沒寫 → KEEP_V912 → 'v912'，否則 DEFAULT_TREE。"""
    if asked is not None:
        _check_tree(asked)
    keep = KEEP_V912.get(struct)
    if keep and asked == '906':
        raise SystemExit("golden_root: %s stays on V912 (%s) -- remove its 'golden': '906' "
                         "(or change KEEP_V912 in tools/golden_root.py with a ruling)" % (struct, keep))
    if asked:
        return asked
    return 'v912' if keep else DEFAULT_TREE


def method_trees(struct, tree, asked_methods, methods):
    """{方法: 樹}：golden_methods 有寫照寫；KEEP_V912_METHODS 一律 v912；其餘＝結構的樹。"""
    asked_methods = asked_methods or {}
    for m, t in asked_methods.items():
        if m not in methods:
            raise SystemExit("golden_root: %s golden_methods: %s is not in methods" % (struct, m))
        _check_tree(t)
    keep = KEEP_V912_METHODS.get(struct, {})
    out = {}
    for m in methods:
        t = asked_methods.get(m)
        if m in keep:
            if t == '906':
                raise SystemExit("golden_root: %s::%s stays on V912 (%s)" % (struct, m, keep[m]))
            t = 'v912'
        out[m] = t or tree
    return out


def check_config_source(src):
    """選了 906 的設定檔不可以自己開 V912：回傳 [(行, 字串)]，是字串常值裡出現 V912 樹資料夾名的地方（# 註解不算）。"""
    hits = []
    try:
        for tok in tokenize.generate_tokens(io.StringIO(src).readline):
            if tok.type == tokenize.STRING and V912_DIRNAME in tok.string:
                hits.append((tok.start[0], tok.string[:90]))
    except (tokenize.TokenError, SyntaxError):
        for n, l in enumerate(src.split('\n'), 1):
            code = l.split('#', 1)[0]
            if V912_DIRNAME in code:
                hits.append((n, l.strip()[:90]))
    return hits


# ---- 行號對照（全面切換時重排數字列用；不在產生器裡跑）----
_OPC = {}


def _norm_lines(tree, rel):
    return [l.rstrip() for l in text(tree, rel).split('\n')]     # rstrip 也去掉 \r；行號同產生器


def opcodes(rel, a='v912', b='906'):
    key = (rel, a, b)
    if key not in _OPC:
        la, lb = _norm_lines(a, rel), _norm_lines(b, rel)
        _OPC[key] = (la, lb, difflib.SequenceMatcher(None, la, lb, autojunk=False).get_opcodes())
    return _OPC[key]


def map_line(rel, n, a='v912', b='906'):
    """a 樹第 n 行 → b 樹的行號（int）；落在不同的段回 None（那一行內容不同，要人看）。"""
    la, lb, ops = opcodes(rel, a, b)
    i = n - 1
    for tag, i1, i2, j1, j2 in ops:
        if i1 <= i < i2:
            return j1 + (i - i1) + 1 if tag == 'equal' else None
    return None


def _main(argv):
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    cmd = argv[0] if argv else 'check'
    if cmd == 'check':
        print('DEFAULT_TREE = %s' % DEFAULT_TREE)
        for t in TREES:
            p, src = env_root(t)
            print('%-4s %s  (%s)%s' % (TAG[t], p, src or 'default', '' if os.path.isdir(p) else '  NOT FOUND'))
        p, src = env_root('906')
        if os.path.isdir(p):
            bad = dict((b[0], b) for b in verify_pins(p))
            for rel in sorted(PINS):
                print('  pin %-20s %s  %s' % (rel, PINS[rel], ('BAD ' + bad[rel][2]) if rel in bad else 'ok'))
            print('906 pins: %s' % ('OK' if not bad else '%d BAD' % len(bad)))
        for k, v in sorted(KEEP_V912.items()):
            print('KEEP_V912 %-20s %s' % (k, v))
        for k, d in sorted(KEEP_V912_METHODS.items()):
            for m, v in sorted(d.items()):
                print('KEEP_V912_METHODS %s::%s  %s' % (k, m, v))
        return 0
    if cmd == 'map' and len(argv) >= 3:
        rel = argv[1]
        for n in argv[2:]:
            r = map_line(rel, int(n))
            if r is None:
                la, lb, ops = opcodes(rel)
                i = int(n) - 1
                blk = next(((tg, i1, i2, j1, j2) for tg, i1, i2, j1, j2 in ops if i1 <= i < i2), None)
                print('V912 %s:%s -> DIFF%s' % (rel, n, (' (%s V912 %d-%d, 0618 %s)' % (
                    blk[0], blk[1] + 1, blk[2], ('%d-%d' % (blk[3] + 1, blk[4])) if blk[4] > blk[3] else
                    'none, goes on at :%d' % (blk[3] + 1))) if blk else ' (out of range)'))
            else:
                print('V912 %s:%s -> 0618 :%d' % (rel, n, r))
        return 0
    if cmd == 'blocks' and len(argv) == 2:
        la, lb, ops = opcodes(argv[1])
        for tg, i1, i2, j1, j2 in ops:
            if tg == 'replace':
                print('replace V912 %d-%d -> 0618 %d-%d' % (i1 + 1, i2, j1 + 1, j2))
            elif tg == 'delete':
                print('delete  V912 %d-%d (V912 only; 0618 goes on at :%d)' % (i1 + 1, i2, j1 + 1))
            elif tg == 'insert':
                print('insert  0618 %d-%d (0618 only; V912 goes on at :%d)' % (j1 + 1, j2, i1 + 1))
        return 0
    sys.stderr.write(__doc__)
    return 2


if __name__ == '__main__':
    sys.exit(_main(sys.argv[1:]))
