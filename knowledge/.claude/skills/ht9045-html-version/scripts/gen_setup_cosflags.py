# -*- coding: utf-8 -*-
r"""gen_setup_cosflags.py -- 從 golden CosFunction.cpp 抽出 Set Up 畫面要用的客戶旗標

//Steven 20260921
當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260921_Steven.md

為什麼需要這支
--------------
Setup.SetUp.html 的 ScrollBar1（Site Mode 選擇）行為，golden 是
cSetUp.cpp::TfSetup::ScrollBar1Change。它的每一條分支都在問
`CosFunction.bXXX` / `IniConfig.bXXX`。

這兩組旗標**不在任何 ini 檔裡**：它們是 CosFunction.cpp 依 CUSTOMER_CODE
在開機時寫死的（InitialCosFunction() 給預設 -> FUNC_CC_<客戶>() 覆寫 ->
FUNC_CC_Common() 再覆寫一次）。web 端能從 Gerneral.ini 讀到
[System] CUSTOMER_CODE，所以只要有一張「客戶碼 -> 旗標」的表，
瀏覽器就能算出和機台一樣的答案。

這支就是產那張表。輸出 client\ht9045_setup_cosflags.js。

抽取法與已知界線
----------------
* 只認「整行就是一個賦值」的形式：
      CosFunction.bEnable6Site   =true;   // 註解
  行首是 `//` 的整行註解跳過（golden 很多旗標是被 mark 掉的，那些不算數）。
* `if(...)` 之後才賦值的那幾行**不採用**，會列進 CONDITIONAL 報告。
  理由：條件本身又牽到別的執行期狀態（fSCKART->iTesterType 之類），
  在瀏覽器這一側無法求值；寧可留在預設值也不要猜。
* `#ifdef` 內的賦值**照收**。golden 出貨版的 define 組合看不出來，
  這是刻意選的偏向：寧可多開一個模式讓人看得到，也不要少開讓人以為機台壞了。
* FUNC_CC_xxx 之間的互相呼叫（目前只有 FUNC_CC_JCET）會展開一層。

重跑：
  <python314> D:\HT9045\HT9011UC_Cpp_V3.33.906.0\scratchpad\gen_setup_cosflags.py
"""
import os
import re
import sys

GOLDEN = r'D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2'
COS = os.path.join(GOLDEN, 'CosFunction.cpp')
MT = os.path.join(GOLDEN, 'MachineType.h')
OUT = r'D:\HT9045\client\ht9045_setup_cosflags.js'

# ScrollBar1Change / CompChange / chkOffCenterkitClick / btnLUpToRDownNClick
# 這四支用到的旗標，逐行從 golden 抄出來的清單。
WANT = [
    # --- CosFunction ---------------------------------------------------------
    'CosFunction.bDisableOpenAllSiteWhenChangeShtMod',
    'CosFunction.bEnable2x1Site',
    'CosFunction.bEnable12Site',
    'CosFunction.bEnable6Site',
    'CosFunction.bCanUse2x2NNMode',
    'CosFunction.bCanUse2x3NNMode',
    'CosFunction.bCanUse2x4NNMode',
    'CosFunction.bUse32ChanelSiteMap',
    'CosFunction.b2x4SupportCenterPitch',
    'CosFunction.bEnableOctal_12Kit',
    'CosFunction.bEnableOctal_16Kit',
    'CosFunction.bCanUseBias',
    'CosFunction.bCanUse2x2Bias',
    'CosFunction.bEnable_1x3Kit',
    'CosFunction.bEnableDual_1x4Kit',
    'CosFunction.bInOutArmUseBackRowSuck',
    'CosFunction.bRotateUseHT7000HPKit',
    'CosFunction.bNonCenterModeCanUseShtOffset',
    'CosFunction.bEnable12SiteUse16SLK',
    'CosFunction.b32SiteYOffsetMode',
    # --- IniConfig（這幾個也是客戶碼決定的，不是 config.ini）-------------------
    'IniConfig.bDualSiteSupply4CH',
    'IniConfig.bSPILFunction',
    'IniConfig.bVTESTFunction',
    'IniConfig.bIndexArm2SupplyLight',
    'IniConfig.bDisableSelectSearchLast',
    'IniConfig.bKoreaFunction',
]
WANTSET = set(WANT)

ASSIGN = re.compile(
    r'^\s*((?:CosFunction|IniConfig)\.[A-Za-z_][A-Za-z0-9_]*)\s*=\s*(true|false)\s*;')
FUNCDEF = re.compile(r'^void\s+(FUNC_CC_[A-Za-z0-9_]+)\s*\(\s*\)')
CALL = re.compile(r'^\s*(FUNC_CC_[A-Za-z0-9_]+)\s*\(\s*\)\s*;')
CASE = re.compile(r'^\s*case\s+(CC_[A-Za-z0-9_]+)\s*:\s*(FUNC_CC_[A-Za-z0-9_]+)\s*\(\s*\)\s*;')
DEFINE = re.compile(r'^#define\s+(CC_[A-Za-z0-9_]+)\s+(\d+)')


def read_big5(path):
    with open(path, 'rb') as f:
        return f.read().decode('big5', 'replace').splitlines()


def split_bodies(lines):
    """FUNC_CC_xxx -> 該函式的行（含 InitialCosFunction）。以大括號計數切段。"""
    bodies, i, n = {}, 0, len(lines)
    while i < n:
        m = FUNCDEF.match(lines[i]) or re.match(r'^void\s+(InitialCosFunction)\s*\(\s*\)', lines[i])
        if not m:
            i += 1
            continue
        name = m.group(1)
        # 只收「最外層大括號之間」的行：不含開頭的 { 也不含結尾的 }，
        # 這樣 flags_of() 的 depth 才是從 0 起算（第一版把開頭的 { 也收進去，
        # 整個函式都變成 depth==1，結果一個旗標都抽不到）。
        j = i
        while j < n and '{' not in lines[j]:
            j += 1
        if j >= n:
            i += 1
            continue
        depth, body, k = 0, [], j
        while k < n:
            depth += lines[k].count('{') - lines[k].count('}')
            if depth <= 0:
                break
            if k > j:
                body.append(lines[k])
            k += 1
        bodies[name] = body
        i = k + 1
    return bodies


def flags_of(body, conditional_log, where):
    """回傳 {flag: bool}。只採用「函式最外層、且不在 if/else 底下」的賦值。

    判準有兩層，缺一不可：
      (1) 大括號深度 == 0 —— if/else/for 的 { } 區塊一律排除。
          #ifdef 沒有大括號，所以會留下來（刻意的，見檔頭說明）。
      (2) one-shot 旗標 —— 沒帶大括號的 `if(...)` 只管下一個敘述行，
          用完就清掉。第一版把「不是我們要的旗標」那一行直接 continue，
          沒清掉旗標，結果條件往下沾到不相干的賦值（FUNC_CC_HONPREC_QC
          的 bAutoRetestGPIBmode if/else 就把後面 5 個旗標全誤判成條件式）。
    """
    out = {}
    depth = 0
    one_shot = False        # 上一行是沒帶大括號的 if/else
    for ln in body:
        s = ln.strip()
        if not s or s.startswith('//') or s.startswith('#'):
            continue
        opens, closes = ln.count('{'), ln.count('}')
        at_top = (depth == 0)

        if at_top:
            m = ASSIGN.match(ln)
            if m and m.group(1) in WANTSET:
                flag, val = m.group(1), (m.group(2) == 'true')
                if one_shot:
                    conditional_log.append((where, flag, val, s))
                else:
                    out[flag] = val
            if m or not re.match(r'^(if|else|\}?\s*else|while|for|switch)\b', s):
                # 任何一個「真正的敘述」都吃掉 one-shot；不吃的話條件會往下沾。
                one_shot = False
            if re.match(r'^(if|else|\}?\s*else|while|for|switch)\b', s) and opens == 0:
                one_shot = True

        depth += opens - closes
        if depth < 0:
            depth = 0
    return out


def main():
    lines = read_big5(COS)
    bodies = split_bodies(lines)
    cond = []

    defaults = flags_of(bodies.get('InitialCosFunction', []), cond, 'InitialCosFunction')
    for f in WANT:                       # golden 的全域初值：沒出現在 Initial 的一律 false
        defaults.setdefault(f, False)

    common = flags_of(bodies.get('FUNC_CC_Common', []), cond, 'FUNC_CC_Common')

    # 客戶碼 -> 函式名
    code_of = {}
    for ln in read_big5(MT):
        m = DEFINE.match(ln)
        if m:
            code_of[m.group(1)] = int(m.group(2))
    cc2func = {}
    for ln in lines:
        m = CASE.match(ln)
        if m and m.group(1) in code_of:
            cc2func[code_of[m.group(1)]] = (m.group(1), m.group(2))

    table, missing = {}, []
    for code, (ccname, fn) in sorted(cc2func.items()):
        body = bodies.get(fn)
        if body is None:
            missing.append((code, ccname, fn))
            continue
        # 展開一層 FUNC_CC_xxx() 呼叫（目前 golden 只有 FUNC_CC_JCET 一處）
        expanded = []
        for ln in body:
            c = CALL.match(ln)
            if c and c.group(1) in bodies and c.group(1) != 'FUNC_CC_Common':
                expanded.extend(bodies[c.group(1)])
            else:
                expanded.append(ln)
        eff = dict(defaults)
        eff.update(flags_of(expanded, cond, fn))
        eff.update(common)
        diff = {k: v for k, v in eff.items() if v != defaults.get(k, False)}
        table[code] = (ccname, diff)

    eff_default = dict(defaults)
    eff_default.update(common)

    short = lambda f: f.split('.', 1)[1]

    buf = []
    w = buf.append
    w(r'/* ht9045_setup_cosflags.js -- Set Up 畫面的客戶旗標表（這個檔是產生的）')
    w(r' * ---------------------------------------------------------------------------')
    w(r' * //Steven 20260921')
    w(r' * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260921_Steven.md')
    w(r' * ---------------------------------------------------------------------------')
    w(r' * 由 HT9011UC_Cpp_V3.33.906.0\scratchpad\gen_setup_cosflags.py 從 golden')
    w(r' * CosFunction.cpp 機械產生。手改會在下次重跑時被覆蓋 —— 要改請改產生器或 golden。')
    w(r' *')
    w(r' * golden 的順序：InitialCosFunction() 給預設 -> FUNC_CC_<客戶>() 覆寫')
    w(r' *                -> FUNC_CC_Common() 最後再覆寫一次。')
    w(r' * DEFAULTS 已經把 Initial + Common 併好；BY_CODE[客戶碼] 只列「和 DEFAULTS 不同」的。')
    w(r' *')
    w(r' * ⚠ 這張表只涵蓋 Set Up 畫面用得到的 %d 個旗標，不是完整的 CosFunction。' % len(WANT))
    w(r' * ⚠ 條件式賦值（if 底下那些）一律沒收進來 —— 條件牽到執行期狀態，瀏覽器算不出來。')
    w(r' *   共 %d 處，列在檔尾 CONDITIONAL。' % len(cond))
    w(r' */')
    w(r'(function (g) {')
    w(r"  'use strict';")
    w(r'  var DEFAULTS = {')
    for f in WANT:
        w('    %-34s %s,' % (short(f) + ':', 'true' if eff_default.get(f) else 'false'))
    w(r'  };')
    w(r'  /* 客戶碼 -> 覆寫（只列與 DEFAULTS 不同的鍵） */')
    w(r'  var BY_CODE = {')
    for code in sorted(table):
        ccname, diff = table[code]
        if not diff:
            w('    %-6s {},%s// %s' % (str(code) + ':', ' ' * 30, ccname))
            continue
        body = ', '.join('%s: %s' % (short(k), 'true' if v else 'false')
                         for k, v in sorted(diff.items()))
        w('    %-6s {%s},   // %s' % (str(code) + ':', body, ccname))
    w(r'  };')
    w(r'  /* 取某個客戶碼的完整旗標（DEFAULTS 疊上該客戶的覆寫） */')
    w(r'  function forCode(code) {')
    w(r'    var out = {}, k;')
    w(r'    for (k in DEFAULTS) if (DEFAULTS.hasOwnProperty(k)) out[k] = DEFAULTS[k];')
    w(r'    var ov = BY_CODE[code];')
    w(r'    if (ov) for (k in ov) if (ov.hasOwnProperty(k)) out[k] = ov[k];')
    w(r'    return out;')
    w(r'  }')
    w(r'  g.HT9045SetupCos = { DEFAULTS: DEFAULTS, BY_CODE: BY_CODE, forCode: forCode,')
    w(r'                       known: function (c) { return BY_CODE.hasOwnProperty(c); } };')
    w(r'}(this));')
    w('')
    w(r'/* ---------------------------------------------------------------------------')
    w(r' * CONDITIONAL -- golden 裡在 if/else 底下才賦值的，沒有收進上表')
    w(r' * ---------------------------------------------------------------------------')
    for whr, flag, val, src in cond:
        w(' *   %-26s %-46s %s' % (whr, short(flag) + ' = ' + ('true' if val else 'false'), src[:70]))
    if not cond:
        w(r' *   （無）')
    w(r' */')
    if missing:
        buf.append('/* switch 指到但找不到函式定義：%s */'
                   % ', '.join('%s->%s' % (c, f) for _, c, f in missing))

    data = ('\n'.join(buf) + '\n').encode('utf-8')   # 先 encode 成功才開檔，避免半截檔
    with open(OUT, 'wb') as f:
        f.write(data)
    print('wrote %s  (%d customer codes, %d flags, %d conditional skipped)'
          % (OUT, len(table), len(WANT), len(cond)))


if __name__ == '__main__':
    sys.exit(main())
