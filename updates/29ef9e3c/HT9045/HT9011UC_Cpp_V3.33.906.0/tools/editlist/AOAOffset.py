# -*- coding: utf-8 -*-
# tools/editlist/AOAOffset.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only AOAOffset
#
# //AI(W906-FRW-AOA) 20260926: 新檔（Steven 團隊）。Steven 20260926「先以大量把讀寫檔進行移植為首要工作」；盤點出處
#   cmydef_io_audit.md 第四節 P6（iAOA_* 38 個：讀 ✅、寫缺）。20260926～E-030 golden 照 V912（RULINGS 第 37 條：Steven＝畫面讀寫）。
#   golden TfMain（main.cpp 0618 34972 行／V912 36170 行、main.h、main.dfm，cp950）裡 AOA offset 那一小塊 —— Main.AOAInfo.html 的讀寫。
# //AI(W906-E031) 20261003 [W906] (St01)：E-031 全面切換第 1 批 —— golden 換成 906 0618（STRUCT['golden']＝'906'，tools/golden_root.py；
#   Jimmy RULINGS_20261003 第 2、4 條）。本檔的列全部照內容找行（L()／_span，沒有寫死的數字列）⇒ 換樹不必重排；轉的 OffsetSaveClick、
#   FormShow 的兩段、main.h 的 39 個元件宣告、main.dfm 38 個 Ed_* 與 pnlAOAForHT9011 的屬性兩棵逐字相同 ⇒ 產生的程式不變（只有標籤／行號）。
#   下面的行號改成 0618、括號 V912（照內容在兩棵找過；舊寫的 V912 號碼是 c2f6c75a 之前的或少 2）。main.h 0618 少 2 個元件宣告（不是本檔用的）。
#   頁面補件：web/page/ht9045_aoaoffset_c.js。入口：FileRW/AOAOffset.cpp（開機、PageDesc）。
#
# 結構名 AOAOffset：沒有 cprod.h 的主結構，讀寫的是 cmydef.h 的 38 個 int 全域 iAOA_*（golden cmydef.cpp:5344-5382，V912 :5366-5404），
#   檔案是 D:\HT9045\system\Gerneral.ini [System] AOA_InArm_*／AOA_OutArm_*（量產共用檔；擁有者 HSys，見 FileRW/AOAOffset.cpp 檔頭）。
#   tag 不能用 TfMain／Main（太大、將來主畫面別的讀寫段也會用 TfMain 的替身），用資料的名字。prefix 'AOA'（20260926 查過沒人用）。
#
# golden 的四個位置（20260926 重查的是 V912；E-031 改成 0618、括號 V912）：
#   讀檔  SYSTEM_MODULAR::ReadGeneralIni database.cpp:1447-1494（V912 :1451-1498；CheckAndReadIniDataGeneral，Auto4-6／Fix4-6 只在 AUTO_EMPTY_COLOR>=3）
#         → 移植樹 database.cpp:1576-1623（逐字，live：LoadMachineConfig → HSys.ReadGeneralIni；HSys 存檔後也重讀）。不在本檔。
#   填元件 TfMain::FormShow main.cpp:11240-11285（V912 :11681-11726）（`if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF_File.bEnableAutoAlignment==true)`
#         iAOA_* → fMain->Ed_*Offset_X/Y->Text）＋ :11297（V912 :11738）pnlAOAForHT9011->Visible=(AUTO_EMPTY_COLOR>=3)。
#         ⚠ 這是主視窗的 FormShow＝開機只跑一次；golden 切到 AOA Info 頁籤（pgMotionView，main.dfm 無 OnChange）不跑任何程式、
#         換配方（ChangeSetUpFile → DoReadLastData）也不重填。→ 本檔把這兩段做成 AOA_FormShowAOA()（members，開機呼叫一次）。
#   存檔  TfMain::OffsetSaveClick main.cpp:33822-33915（V912 :34988-35081；主畫面 AOA Info 頁 Panel14 的 OffsetSave 鈕）：38 個 Ed_* → iAOA_*（atoi）
#         → WriteIniDataGeneral ×26（AUTO_EMPTY_COLOR>=3 時 ×38）。沒有權限檢查、沒有確認框、沒有 return、寫完不重讀。→ 本檔 method。
#   用值  AutoAlignment\AutoAlignment.cpp:4664-5325 讀的是 fMain->Ed_*Offset_*->Text（元件字串），不是 iAOA_*。移植樹沒有這段
#         （CCD 對位流程，Jimmy）；將來移植時要讀本檔的具名替身 EL<TEdit>("TfMain","Ed_*Offset_*")。
#
# golden 事實（照翻，不修；見 FileRW/AOAOffset.cpp 檔頭「golden 看起來不對的地方」）：
#   * main.dfm 38 個 Ed_*Offset_X/Y 全部 Visible = False（Panel14 內 26 個 object 行 :16763-17013、pnlAOAForHT9011 內 12 個 :17157-17256；V912 :16795-17045／:17189-17288），golden 全樹沒有任何地方
#     把它們設成可見（20260926 grep `Offset_[XY]->` 只有 ->Text）→ 操作員在 golden 畫面上看不到、也改不到；OffsetSave 鈕看得到。
#     具名替身照 DFM 設 Visible=false ⇒ ELEditable=false ⇒ 頁面送的值一律進 ack.ignored，存檔寫的是開機填的值（＝golden）。
#   * FormShow 的條件不成立（這台 MACHINE_HAS_AUTO_ALIGNMENT_CCD=0）時元件留在 DFM 的 '0' → 按 OffsetSave 會把 [System] AOA_* 全寫成 0。
#
# 為什麼不轉 FormShow 整支：TfMain::FormShow 是 :9141-11365（2,225 行；V912 :9568-11849），產生器會把裡面用到的幾百個主畫面元件全部建成替身
#   （names_used 連 #if 0 內的也算）。只取 :11240-11285 與 :11297 兩段（V912 :11681-11726／:11738），逐字機械改寫（_proxy），行號與原文由 L() 核對。
# 為什麼要逐行 replace：golden 寫 fMain->Ed_*（經全域指標取自己的元件），產生器只改寫沒有前綴的元件名（同
#   tools/editlist/TestIF_File_AutoAlignment.py 的 fAutoAlignment->cbAOA_UseFix* 做法）→ 只把 `fMain->` 前綴換成具名替身，其餘逐字。
import re

import golden_root as _GR   # AI(W906-E031) 20261003 [W906]：golden 一律經過 tools/golden_root.py（行號＝產生器的行號）

_TREE = _GR.tree_of('AOAOffset', '906')   # AI(W906-E031)：E-031 全面切換第 1 批 —— 906 0618；STRUCT['golden'] 同一個值
_cpp = _GR.lines(_TREE, 'main.cpp')
_h = _GR.text(_TREE, 'main.h')   # AI(W906-E031)：header 同一棵（原本 cp950 解碼、\r\n 原樣 —— GR.text 一樣）
_F = 'TfMain'

METHODS = ['OffsetSaveClick']

# golden main.h 的元件型別（只收本檔用到的：Ed_*Offset_X/Y 與 pnlAOAForHT9011）
_TYPES = {m.group(2): m.group(1) for m in re.finditer(r'^\s*(T\w+)\s*\*\s*(Ed_\w+Offset_[XY]|pnlAOAForHT9011)\s*;', _h, re.M)}
if len(_TYPES) != 39 or any(t != 'TEdit' for n, t in _TYPES.items() if n != 'pnlAOAForHT9011') or _TYPES.get('pnlAOAForHT9011') != 'TPanel':
    raise SystemExit('AOAOffset.py: golden main.h Ed_*Offset_X/Y / pnlAOAForHT9011 declarations changed: %r' % sorted(_TYPES.items()))


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfMain::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('AOAOffset.py: span %s' % meth)


SPANS = {m: _span(m) for m in METHODS + ['FormShow']}


def L(meth, text, nth=1):
    """golden 行號（1 起）：meth 本體裡第 nth 個含 text 的行。行號漂了就中止（不蓋錯地方）。"""
    a, b = SPANS[meth]
    k = 0
    for gl in range(a, b + 1):
        if text in _cpp[gl - 1]:
            k += 1
            if k == nth:
                return gl
    raise SystemExit('AOAOffset.py: L(%s, %r) not found' % (meth, text))


_SELF = re.compile(r'\bfMain->(Ed_\w+Offset_[XY])\b')
_BARE = re.compile(r'(?<![\w.>:])(pnlAOAForHT9011)\b')


def _proxy(code, qualified):
    """golden fMain->Ed_*（與 TfMain 方法內的裸名 pnlAOAForHT9011）→ 具名替身；字串／註解以外逐字。"""
    el = 'filerw::EL' if qualified else 'EL'
    code = _SELF.sub(lambda m: '%s<%s>("%s", "%s")' % (el, _TYPES[m.group(1)], _F, m.group(1)), code)
    return _BARE.sub(lambda m: '%s<%s>("%s", "%s")' % (el, _TYPES[m.group(1)], _F, m.group(1)), code)


# ---- OffsetSaveClick（golden :33822，V912 :34988）：38 行 `iAOA_* =atoi(fMain->Ed_*->Text.c_str());` 逐行換成具名替身
_RD_A = L('OffsetSaveClick', 'iAOA_InArm_Loader_X     =atoi(fMain->Ed_LoaderOffset_X->Text.c_str());')
_RD_B = L('OffsetSaveClick', 'iAOA_OutArm_Shuttle2_Y  =atoi(fMain->Ed_OutSH2Offset_Y->Text.c_str());')
REPLACE = []
for _gl in range(_RD_A, _RD_B + 1):
    _code = _cpp[_gl - 1].split('//')[0].strip()
    if not _code:
        continue
    if not _SELF.search(_code):
        raise SystemExit('AOAOffset.py: golden main.cpp:%d in OffsetSaveClick read block has no fMain->Ed_*: %r' % (_gl, _code))
    REPLACE.append(('OffsetSaveClick', _gl, _gl,
                    'golden 寫法帶 fMain-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字',
                    _proxy(_code, False)))
if len(REPLACE) != 38:
    raise SystemExit('AOAOffset.py: OffsetSaveClick read block has %d widget lines, expected 38' % len(REPLACE))

# ---- 填元件：golden TfMain::FormShow :11240-11285（iAOA_* → Ed_*）＋ :11297（pnlAOAForHT9011->Visible）（V912 :11681-11726／:11738）→ 本 TU 的 AOA_FormShowAOA()。
#   members 在 `using filerw::EL;` 之前輸出 → 用 filerw::EL；members 會在第一行前面加 `static `。
#   FormShow 裡 `if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&` 有三處（:9258 HiSilicon／:9304 ASE_KaohSiung 版號、:11240 本段；V912 :9687／:9737／:11681）→ 從第一個
#   fMain->Ed_LoaderOffset_X 往回三行定位，再逐行核對。
_FS_A = L('FormShow', 'fMain->Ed_LoaderOffset_X->Text  =iAOA_InArm_Loader_X;') - 3
if ('if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&' not in _cpp[_FS_A - 1] or 'TestIF_File.bEnableAutoAlignment==true)' not in _cpp[_FS_A]
        or _cpp[_FS_A + 1].strip() != '{'):
    raise SystemExit('AOAOffset.py: golden main.cpp:%d is not the AOA offset fill block' % _FS_A)
_FS_B = _FS_A
while _cpp[_FS_B - 1].rstrip() != '    }':
    _FS_B += 1
_PNL = L('FormShow', 'pnlAOAForHT9011->Visible=(AUTO_EMPTY_COLOR>=3);')
_n_fill = sum(1 for gl in range(_FS_A, _FS_B + 1) if _SELF.search(_cpp[gl - 1]))
if _n_fill != 38 or not (_FS_B < _PNL < _FS_B + 20):
    raise SystemExit('AOAOffset.py: fill block %d..%d has %d widget lines (expected 38), pnlAOAForHT9011 at %d' % (_FS_A, _FS_B, _n_fill, _PNL))


def _formshow_aoa():
    body = ['void AOA_FormShowAOA()   // golden TfMain::FormShow main.cpp:%d-%d ＋ :%d（逐字；fMain->Ed_* 與 pnlAOAForHT9011 → 具名替身）'
            % (_FS_A, _FS_B, _PNL), '{']
    body += [_proxy(x.rstrip(), True) for x in _cpp[_FS_A - 1:_FS_B]]
    body += ['    // golden :%d-%d（RENESAS_Server 瑞薩 FT-CT，與 AOA 無關）不在這裡' % (_FS_B + 1, _PNL - 1),
             _proxy(_cpp[_PNL - 1].rstrip(), True) + '   // golden main.cpp:%d' % _PNL,
             '}']
    return '\n'.join(body)


STRUCT = {
    'struct': 'AOAOffset',
    'golden': _TREE,   # AI(W906-E031) 20261003 [W906]：906 0618（tools/golden_root.py）
    'prefix': 'AOA',
    'class': _F,
    'cpp': 'main.cpp',
    'h': 'main.h',
    'files': ['D:\\HT9045\\system\\Gerneral.ini [System] AOA_InArm_*／AOA_OutArm_* _X/_Y（寫：OffsetSaveClick，WriteIniDataGeneral '
              '26 鍵，AUTO_EMPTY_COLOR>=3 時 38 鍵；讀：移植樹 database.cpp:1576 ReadGeneralIni，不在本檔）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['OffsetSaveClick'],
    'params': {'OffsetSaveClick': ''},
    'members': [
        _formshow_aoa(),
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cmydef.h', 'cprod.h', 'common.h'],
    # cmydef.h：iAOA_* 38 個、MACHINE_HAS_AUTO_ALIGNMENT_CCD、AUTO_EMPTY_COLOR；cprod.h：TestIF_File（bEnableAutoAlignment）；
    # common.h：WriteIniDataGeneral（INIFileGeneral：LoadMachineConfig 開的那一個，golden SYSTEM_MODULAR 建構子 database.cpp:49（V912 :50）開、
    #           程式結束才關 —— 兩邊都是開機開一次、一直開著）
    'decls': [],
    'overrides': [],
}
