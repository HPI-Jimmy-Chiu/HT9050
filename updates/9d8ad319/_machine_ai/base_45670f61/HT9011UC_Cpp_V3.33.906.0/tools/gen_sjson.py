# -*- coding: utf-8 -*-
# Steven 20260923
# ----------------------------------------------------------------------
# AI(W906-JSONBRIDGE-S2) 新檔。從 cprod.h / LastSet.h 的結構宣告產生
# JsonBridge/gen/sjson_<STRUCT>.gen.cpp 的 FieldDesc 表。
# 規格：.claude/skills/ht9045-json-bridge/SKILL.md 4.1
# ----------------------------------------------------------------------

"""gen_sjson.py -- 產生結構的欄位表。

只做一件事：把結構宣告裡的**欄位名字、型別、維度**列出來，產生一段 C++。
位移一律由 offsetof 算、元素大小一律由 sizeof 算 —— 產生器不碰任何數字。

  ⚠⚠ 大括號配對前**一定要先剝掉註解**。

  20260923 踩過而且踩得很深：cprod.h:2156 是

      bool bAlarm4EnableIntervalYield;   //…Interval Total Yield Difference {

  行尾註解裡有一個 `{`。從 `} SYSTEM_TEST_IF;` 往回配對而沒有先剝註解的話，
  會咬住它，於是「結構」只剩尾巴：SYSTEM_TEST_IF 被量成 351 個成員，
  實際是 788。漏掉的 437 個不會有任何徵兆 —— 產生器照樣跑完、C++ 照樣編過、
  測試照樣綠，只是那些欄位永遠不會出現在 JSON 裡。

  同樣的道理適用於字串字面值裡的大括號，所以剝註解的同時也要認得字串。

分兩種輸入：
  * 一般設定結構（cprod.h 的 SYSTEM_*）—— 欄位名有意義，逐欄出。
  * 二進位 blob（LastSet.h 的 LAST_GENERAL_SET）—— 一樣逐欄出，只是它的
    持久化是整塊 sizeof 寫檔，那是 Binding 的事不是這裡的事。
"""

import io
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.dirname(HERE)
GENDIR = os.path.join(PORT, 'JsonBridge', 'gen')

# 要產生的結構：(標頭, 結構名)
# (標頭, 結構名, 要掃 ini 對照的實例名)。實例名給 None 表示不掃
# （二進位 blob 沒有 ini 鍵可對）。
TARGETS = [
    ('cprod.h', 'SYSTEM_DEVICE_FORM'),
    # AI(W906-JSONBRIDGE-S3) 20260923（使用者指示）：LevelSet 是二進位檔
    #   （system\levelset.dat，256 個小端 int32，golden cSecurity.cpp:1474
    #   GetLevelSet / :1511 SetLevelSet 整塊 ReadData/WriteData），
    #   「用陣列的方式處理就可以」。
    #   它的結構正好只有一個成員 int AccessLevel[256]，所以這裡不需要任何
    #   特例 —— 一維 int 陣列這條路本來就走得通。
    ('cprod.h', 'LAST_LEVEL_SET'),
    # AI(W906-JSONBRIDGE-S4) 20260923: 收益最大的一個 —— JerryYang 20260922
    #   剛翻好的 TfSetup::ReadFile() 灌的就是 TestIF_File。
    #   ⚠ 788 個成員，不是 351。351 是「反向配對咬到 cprod.h:2156 註解裡的 `{`」
    #     之後量到的尾巴。本產生器的 strip_comments 就是為了這件事存在的。
    ('cprod.h', 'SYSTEM_TEST_IF'),
    # AI(W906-JSONBRIDGE-S5) 20260923: 溫控與工作檔。
    ('cprod.h', 'SYSTEM_TEMPERATURE'),
    ('cprod.h', 'SYSTEM_TRAY_FORM'),
    ('cprod.h', 'SYSTEM_TEST_MODE'),
]

# C++ 型別 -> FieldType
TYPEMAP = {
    'int': 'kFieldInt',
    'double': 'kFieldDouble',
    'bool': 'kFieldBool',
    'AnsiString': 'kFieldAnsiString',
    'char': 'kFieldCharBuf',
}


def strip_comments(src):
    """砍掉 // 與 /* */，同時認得字串與字元字面值。保留行數。

    這個函式是本檔的正確性核心，見上方 docstring 的 ⚠⚠。
    """
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if c == '"' or c == "'":
            q = c
            out.append(c)
            i += 1
            while i < n:
                if src[i] == '\\' and i + 1 < n:
                    out.append(src[i:i + 2])
                    i += 2
                    continue
                out.append(src[i])
                if src[i] == q:
                    i += 1
                    break
                i += 1
            continue
        if c == '/' and i + 1 < n and src[i + 1] == '/':
            while i < n and src[i] != '\n':
                i += 1
            continue
        if c == '/' and i + 1 < n and src[i + 1] == '*':
            i += 2
            while i + 1 < n and not (src[i] == '*' and src[i + 1] == '/'):
                if src[i] == '\n':
                    out.append('\n')
                i += 1
            i += 2
            continue
        out.append(c)
        i += 1
    return ''.join(out)


def struct_body(src_stripped, name):
    """回傳 (body, startLine, endLine)。src 必須是**已剝註解**的。"""
    m = re.search(r'\}\s*' + re.escape(name) + r'\s*;', src_stripped)
    if not m:
        return None
    close = src_stripped.index('}', m.start())
    depth, i = 0, close
    while i >= 0:
        if src_stripped[i] == '}':
            depth += 1
        elif src_stripped[i] == '{':
            depth -= 1
            if depth == 0:
                return (src_stripped[i + 1:close],
                        src_stripped.count('\n', 0, i) + 1,
                        src_stripped.count('\n', 0, close) + 1)
        i -= 1
    return None


DECL = re.compile(r'^\s*((?:unsigned\s+|signed\s+|const\s+)*[A-Za-z_][A-Za-z_0-9]*)\s+(.+)$')
NAMEDIM = re.compile(r'^([A-Za-z_][A-Za-z_0-9]*)((?:\s*\[[^\]]*\])*)\s*$')
DIM = re.compile(r'\[\s*([^\]]+?)\s*\]')


def parse_fields(body):
    """回傳 [(type, name, dims[])]；dims 是原始字串（可能是常數名）。"""
    # 巢狀結構的內層先整段拿掉（頂層成員才算）
    flat, depth = [], 0
    for ch in body:
        if ch == '{':
            depth += 1
            continue
        if ch == '}':
            depth -= 1
            continue
        if depth == 0:
            flat.append(ch)
    out = []
    for decl in ''.join(flat).split(';'):
        decl = ' '.join(decl.split())
        if not decl:
            continue
        # ⚠ 巢狀型別宣告（enum / struct / union / class）不是欄位。
        #   實測 SYSTEM_TEMPERATURE 裡有 `enum BoostFunction { … };`，
        #   把它當欄位會產生 `offsetof(S, BoostFunction)` 這種編不過的東西。
        #   剝掉巢狀大括號之後留下的是 `enum BoostFunction`，所以在這裡擋。
        if re.match(r'^(enum|struct|union|class|typedef)\b', decl):
            continue
        m = DECL.match(decl)
        if not m:
            out.append((None, decl[:60], []))      # 看不懂的，照樣記
            continue
        ctype = m.group(1).strip()
        for one in m.group(2).split(','):
            one = one.strip()
            if not one:
                continue
            mm = NAMEDIM.match(one)
            if not mm:
                out.append((None, one[:60], []))
                continue
            dims = DIM.findall(mm.group(2) or '')
            out.append((ctype, mm.group(1), dims))
    return out


# ---------------------------------------------------------------------------
#  AI(W906-JSONBRIDGE-S6) 20260923：ini 鍵對照表。
#
#  讀方向不需要它（讀的是結構）。寫方向需要 [欄位 -> 檔, 區段, 鍵, 格式]，
#  而 golden 自己就把這張表寫在程式碼裡：
#
#      TestIF_File.dSiteXPitch = ReadIniData(szDir, "Configuration", "X Pitch", 0.1);
#      //     ^ 欄位                          ^ 路徑變數  ^ 區段        ^ 鍵     ^ 預設
#
#  ⚠⚠ **一個欄位可能有多個來源。** 實測 cContact.cpp：`szDir` 是配方
#     Contact.Data、`szDir2` 是寫死的 system\\Contact.ini，依
#     CosFunction.bContactHeightSaveToContactIni 分岔；同一個
#     DeviceForm_File.IndexArmPick[0] 在兩條路上都被讀。
#     所以這張表**不是**一對一，產生器不能挑一條當答案。多來源的一律標
#     multi=true，由 Binding 那一層（手寫）決定，或在 S6 的 Clamp 裡處理。
#
#  ⚠ 區段有時是變數（`S` = GetLastOpenFN()，也就是配方名當區段名）。
#    那種一律記成 <expr>，不猜。
# ---------------------------------------------------------------------------
GOLDEN = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'

READ_CALL = re.compile(
    r'\b([A-Za-z_][A-Za-z_0-9]*)\s*\.\s*([A-Za-z_][A-Za-z_0-9]*)'
    r'((?:\s*\[[^\]]*\])*)\s*=\s*'
    r'(?:Check(?:And)?)?Read(?:Write)?Ini(?:Data|DataMem)?\s*\(\s*'
    r'([A-Za-z_][A-Za-z_0-9]*)\s*,\s*'
    r'(?:"((?:[^"\\]|\\.)*)"|([^,()"]+))\s*,\s*'
    r'(?:"((?:[^"\\]|\\.)*)"|([^,()"]+))\s*,\s*'
    r'([^;]*?)\)\s*;')


def scan_ini_map(instances):
    """掃 golden，回傳 {field: [(pathVar, section, key, default), ...]}。

    instances 是要認的實例名（例如 ('TestIF_File',)）—— 同一個型別的不同實例
    讀的是同一組鍵，但只有 _File 那份是從檔案來的。
    """
    found = {}
    for dp, dn, fs in os.walk(GOLDEN):
        dn[:] = [d for d in dn
                 if d.lower() not in ('build', 'obj', 'backup', '.git')]
        for f in fs:
            if not f.lower().endswith('.cpp'):
                continue
            path = os.path.join(dp, f)
            try:
                raw = io.open(path, 'r', encoding='cp950', errors='replace').read()
            except (IOError, OSError):
                continue
            if not any(i in raw for i in instances):
                continue
            src = strip_comments(raw)        # ⚠ 同樣先剝註解
            for m in READ_CALL.finditer(src):
                if m.group(1) not in instances:
                    continue
                field = m.group(2)
                sec = m.group(5) if m.group(5) is not None else \
                    '<' + ' '.join((m.group(6) or '').split()) + '>'
                key = m.group(7) if m.group(7) is not None else \
                    '<' + ' '.join((m.group(8) or '').split()) + '>'
                rec = (m.group(4), sec, key, ' '.join(m.group(9).split())[:40])
                lst = found.setdefault(field, [])
                if rec not in lst:
                    lst.append(rec)
    return found


BANNER = '''// ===========================================================================
//  sjson_%(S)s.gen.cpp -- GENERATED, DO NOT EDIT BY HAND.
//
//  AI(W906-JSONBRIDGE-S2) 20260923.  NOT in golden.
//
//  產生器：tools/gen_sjson.py
//  來源　：%(H)s:%(A)d-%(B)d（結構 %(S)s，%(N)d 個頂層成員）
//  規格　：.claude/skills/ht9045-json-bridge/SKILL.md 4.1
//
//  重跑：python tools\\gen_sjson.py
//
//  ⚠ offset 與 elemSize 都是 offsetof/sizeof —— **編譯器**算的，不是產生器。
//    產生器只知道名字。
// ===========================================================================
#include "JsonBridge/FieldDesc.h"

#include "vclcompat/vcl_compat.h"
#include "cprod.h"

namespace ht9045 {
namespace sjson {

namespace {

const FieldDesc kFields_%(S)s[] = {
'''

FOOTER = '''};

// AI(W906-JSONBRIDGE-S6): 欄位 -> ini 鍵。寫方向要用；讀方向用不到。
// multi=true 表示 golden 有不只一條路徑讀這個欄位，產生器不替你挑。
const IniKey kIniKeys_%(S)s[] = {
%(INI)s};

}  // namespace

// ⚠ 一定要 extern。C++ 裡命名空間範圍的 `const` 物件預設是**內部連結**，
//   少了 extern 這個定義在別的 TU（Bindings.cpp）看不到，會是
//   「undefined reference to kType_...」。20260923 踩過一次。
extern const TypeDesc kType_%(S)s;
extern const TypeDesc kType_%(S)s = {
    "%(S)s",
    kFields_%(S)s,
    sizeof(kFields_%(S)s) / sizeof(kFields_%(S)s[0]),
    sizeof(%(S)s),
    kIniKeys_%(S)s,
    sizeof(kIniKeys_%(S)s) / sizeof(kIniKeys_%(S)s[0])
};

}  // namespace sjson
}  // namespace ht9045
'''


# 結構 -> 從檔案載入它的那個實例名。用來掃 ini 對照。
# ⚠ 只掃 _File 那份：不帶 _File 的是執行中那份，由 Do*Convert() 從 _File
#   轉出來，不是從檔案讀的。掃錯會把轉換結果誤記成檔案來源。
INI_INSTANCE = {
    'SYSTEM_DEVICE_FORM': 'DeviceForm_File',
    'SYSTEM_TEST_IF':     'TestIF_File',
    'SYSTEM_TEMPERATURE': 'Temperature',
    'SYSTEM_TRAY_FORM':   'TrayForm',
    'SYSTEM_TEST_MODE':   'TestMode',
    # LAST_LEVEL_SET 是二進位 blob（整塊 ReadData/WriteData），沒有 ini 鍵。
}


def emit(header, struct):
    hpath = os.path.join(PORT, header)
    raw = io.open(hpath, 'r', encoding='utf-8', errors='replace').read()
    stripped = strip_comments(raw)            # ⚠ 先剝，再配對
    got = struct_body(stripped, struct)
    if got is None:
        sys.stderr.write('找不到 struct %s（%s）\n' % (struct, header))
        return None
    body, a, b = got
    fields = parse_fields(body)

    lines = []
    nsupp = 0
    for ctype, name, dims in fields:
        if ctype is None:
            lines.append('    // ⚠ 無法解析的宣告，'
                         '保留為註解以免這個欄位'
                         '静默消失：%s' % name)
            nsupp += 1
            continue
        ft = TYPEMAP.get(ctype, 'kFieldUnsupported')
        if ft == 'kFieldUnsupported':
            nsupp += 1
        # ⚠ 維度一律用 sizeof 算，**不要**把標頭裡的那個常數名抄出來。
        #   實測：SYSTEM_TEMPERATURE 的陣列大小是結構範圍的 enum 常數
        #   （enum BoostFunction { … ebTotal }），抄名字出來會得到
        #   「'ebTotal' was not declared in this scope」。sizeof 相除既不需要
        #   名稱解析，也是編譯器算的 —— 跟 offsetof 同一條原則。
        def dim_expr(level):
            base = '((%s*)0)->%s%s' % (struct, name, '[0]' * level)
            return 'sizeof(%s) / sizeof(%s[0])' % (base, base)

        d0 = dim_expr(0) if len(dims) >= 1 else '0'
        d1 = dim_expr(1) if len(dims) >= 2 else '0'
        if len(dims) > 2:
            ft = 'kFieldUnsupported'          # 三維以上留給後面的期，不猜
            nsupp += 1
        elem = ('sizeof(((%s*)0)->%s%s)'
                % (struct, name, '[0]' * len(dims))) if dims else \
               ('sizeof(((%s*)0)->%s)' % (struct, name))
        lines.append('    { "%s", offsetof(%s, %s), %s, %s, (std::size_t)(%s), (std::size_t)(%s) },'
                     % (name, struct, name, ft, elem, d0, d1))

    if not os.path.isdir(GENDIR):
        os.makedirs(GENDIR)
    out = os.path.join(GENDIR, 'sjson_%s.gen.cpp' % struct)
    # ini 鍵對照（S6 寫方向要用）。讀方向用不到 —— 讀的是結構。
    inst = INI_INSTANCE.get(struct)
    ini = scan_ini_map((inst,)) if inst else {}
    ilines, nmulti, nmapped = [], 0, 0

    def esc(t):
        return t.replace('\\', '\\\\').replace('"', '\\"')

    for ctype, name, _d in fields:
        if ctype is None:
            continue
        recs = ini.get(name)
        if not recs:
            continue
        nmapped += 1
        if len(recs) > 1:
            nmulti += 1
        # ⚠ 多來源的只出第一筆並標 multi。產生器不能挑一條當答案：
        #   哪一條生效是執行期條件（例如 CosFunction.bContactHeightSaveToContactIni）。
        pv, sec, key, dflt = recs[0]
        ilines.append('    { "%s", "%s", "%s", "%s", "%s", %s },'
                      % (name, esc(pv), esc(sec), esc(key), esc(dflt),
                         'true' if len(recs) > 1 else 'false'))
    if not ilines:
        ilines.append('    { 0, 0, 0, 0, 0, false }   '
                      '// 無 ini 對照（二進位 blob，或還沒掃到）')

    subst = {'S': struct, 'H': header, 'A': a, 'B': b, 'N': len(fields),
             'INI': '\n'.join(ilines) + '\n'}
    io.open(out, 'w', encoding='utf-8', newline='\n').write(
        BANNER % subst + '\n'.join(lines) + '\n' + FOOTER % subst)
    print('  %-22s %s:%d-%d  %d 欄（%d 不支援），ini 對照 %d 欄（%d 多來源）'
          % (struct, header, a, b, len(fields), nsupp, nmapped, nmulti))
    return len(fields)


def main():
    print('gen_sjson: 產生欄位表')
    total = 0
    for header, struct in TARGETS:
        n = emit(header, struct)
        if n:
            total += n
    print('合計 %d 個欄位' % total)
    return 0


if __name__ == '__main__':
    sys.exit(main())
