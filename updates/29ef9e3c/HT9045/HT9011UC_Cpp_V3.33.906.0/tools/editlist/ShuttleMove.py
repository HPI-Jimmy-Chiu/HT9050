# -*- coding: utf-8 -*-
# tools/editlist/ShuttleMove.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only ShuttleMove
#
# //AI(W906-E031) 20261003 [W906] (St01)：E-031 全面切換第 1 批 —— golden 換成 906 0618（STRUCT['golden']＝'906'，tools/golden_root.py；
#   Jimmy RULINGS_20261003 第 2、4 條）。ShuttleMove.cpp／.h／.dfm 0618 與 V912 逐位元組相同 ⇒ 本檔 ShuttleMove.cpp 的行號兩棵都對、
#   產生的程式不變（只有檔頭路徑變）；其他 golden 檔的引用改成 0618，括號裡是 V912。
# Steven 團隊 20260925：golden TfShuttleMove（ShuttleMove.cpp，2761 行；20260925～E-030 讀 V912）—— HW.ShuttleMove.html 的「讀寫段」。
# 結構名 ShuttleMove（同 StartCondition 的慣例）：這張表單沒有自己的主結構，它讀寫的是別人的欄位 ——
#   Tech.iInSH1Sen7DetectPos／iInSH2Sen7DetectPos／iInSH1BarCodePos／iInSH2BarCodePos／OutSH1ZOneRowDetectPos／
#     OutSH2ZOneRowDetectPos／OutSH1ZDetectPos／OutSH2ZDetectPos／iInSH1SenICDetectPos／iInSH2SenICDetectPos
#     （全是 golden uteach.cpp 的 TECH_PARA → fTeach->SaveFile(true) 寫 system\teach.ini）
#   Prod.iInSH1SenICAddPos／iInSH2SenICAddPos、fCCLink->iInShtLtcPercent
#     （<recipe>\HandlerCondition.Data [Shuttle]；讀＝golden TfShuttleMove::ReadData :2252，寫＝sbUpdateClick :1985-1986）
#   → 用表單名（去掉 Tf）當 tag，不冒用 Teach（Teach 已有自己的 C 路 tag）。
#
# 轉的 golden 方法（讀寫段；移動／送料等機台動作不轉，見 FileRW/ShuttleMove.cpp 檔頭）：
#   建構子 :59、FormShow :67（開頁）、sbUpdateClick :1965（存檔鈕）、ShowShuttleSensorPosition :1998（兩者都呼叫）。
#   ReadData :2252 不轉成本 TU 的方法：它是 golden 從 main.cpp:11300／:25100、cinitial.cpp:11225（V912 main.cpp:11741／:25814、cinitial.cpp:11230）呼叫的
#   fShuttleMove->ReadData()，移植在門面 forms/fShuttleMove.cpp（全樹一份），這裡的兩個呼叫點（:111、:1988）改呼叫它。
#
# Steven 20260926 裁決 S47：「只有latch才會用到，無latch的機台就不用讀寫」→ FormShow :109 與 sbUpdateClick :1976 的
#   HandlerCondition.Data [Shuttle] 讀寫段照 golden 的 `if(In_Shuttle_Auto_Latch==eInSHAutoLtc)`（20260925 版曾依裁決 38 改成一律讀寫，
#   S47 撤回）。開機（main.cpp:11300（V912 :11741））與換配方（:25100（V912 :25814））的 fShuttleMove->ReadData() 也照 golden 同一個條件。
_F = 'TfShuttleMove'

import re

import golden_root as _GR   # AI(W906-E031) 20261003 [W906]：golden 一律經過 tools/golden_root.py（行號＝產生器的行號）

_TREE = _GR.tree_of('ShuttleMove', '906')   # AI(W906-E031)：E-031 全面切換第 1 批 —— 906 0618；STRUCT['golden'] 同一個值
_cpp = _GR.lines(_TREE, 'ShuttleMove.cpp')


def _chk(gl, text):
    """golden 行號核對（產生器另外還會核對 replace 的起行）。"""
    if text not in _cpp[gl - 1]:
        raise SystemExit('ShuttleMove.py: golden ShuttleMove.cpp:%d is not %r (got %r)' % (gl, text, _cpp[gl - 1]))
    return gl


# ---- ShowShuttleSensorPosition（:1998-2092）的四個 TTMyTray：vclcompat 沒有 TTMyTray（產生器會退回 TControl，沒有 XItem／
#      SetCellNumber）→ 本 TU 的 SM_TTMyTray（只存格子數字，FileRW/ShuttleMove.cpp 的 extraJson 送給頁面畫）。
#      逐行機械改寫：mtX->XItem／mtX->SetCellNumber → SM_mtX.；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()（aHotPlateSubstrate.h 與
#      HTEditList.h 衝突，經 FileRW/_KitSuck.cpp 轉接）；btnTStep → 具名替身；ShowMyMessage → filerw::ELMessage。
#      mtInSHBarCodePos 的 fBarCode->GetMovePos（golden BarCode.cpp，移植樹 TfBarCode 沒有這支）→ 不填格子、記 bPortGap（頁面顯示缺口）。
_TRAYS = ['mtedInSHSen7DetectPos', 'mtOutSHDetectPos', 'mtedInSHSenICDetectPos', 'mtInSHBarCodePos']


def _tray_line(raw):
    code = raw.split('//')[0].rstrip()
    t = code
    if 'fBarCode->GetMovePos' in t:
        m = re.match(r'^(\s*)mtInSHBarCodePos->SetCellNumber\(.*\);$', t)
        assert m, raw
        return m.group(1) + 'SM_mtInSHBarCodePos.bPortGap=true;'
    t = re.sub(r'\b(%s)->(XItem|SetCellNumber)\b' % '|'.join(_TRAYS), r'SM_\1.\2', t)
    t = t.replace('InArmSuck.iShtCol', 'FileRW_InArmSuckShtCol()')
    t = re.sub(r'\bbtnTStep->', 'EL<TButton>("%s", "btnTStep")->' % _F, t)
    t = re.sub(r'\bShowMyMessage\s*\(', 'filerw::ELMessage(', t)
    return t


_SSP_A = _chk(2001, 'mtedInSHSen7DetectPos->XItem=InArmSuck.iShtCol;')
_SSP_B = _chk(2091, '}')
_SSP = []
for _gl in range(_SSP_A, _SSP_B + 1):
    _raw = _cpp[_gl - 1]
    _code = _raw.split('//')[0]
    if re.search(r'\b(%s)->|InArmSuck\.|fBarCode->' % '|'.join(_TRAYS), _code):
        _SSP.append(('ShowShuttleSensorPosition', _gl, _gl,
                     '自動：TTMyTray 格子 → SM_TTMyTray（extraJson 送頁面畫）；InArmSuck.iShtCol → FileRW_InArmSuckShtCol()；'
                     'fBarCode->GetMovePos 未移植 → bPortGap',
                     _tray_line(_raw).strip()))

STRUCT = {
    'struct': 'ShuttleMove',
    'golden': _TREE,   # AI(W906-E031) 20261003 [W906]：906 0618（tools/golden_root.py）
    'prefix': 'SM',
    'class': _F,
    'cpp': 'ShuttleMove.cpp',
    'h': 'ShuttleMove.h',
    'files': ['system\\teach.ini（fTeach->SaveFile(true)：TECH_PARA 十個 Shuttle 教導點）',
              '<recipe>\\HandlerCondition.Data [Shuttle] InSH1SenICAddPos／InSH2SenICAddPos（寫）＋InShtLtcPercent（只讀）'],
    'lists': [],
    'methods': ['TfShuttleMove', 'FormShow', 'sbUpdateClick', 'ShowShuttleSensorPosition'],
    'save_methods': ['sbUpdateClick'],
    'params': {'TfShuttleMove': '', 'FormShow': '', 'sbUpdateClick': ''},
    'members': [
        'bool fShow=false;   // golden ShuttleMove.h:96 —— 本 TU 自己的。不寫門面 fShuttleMove->fShow：網頁關頁沒有回報（golden FormClose :2135 '
        '才清），寫了就永遠是 true，會讓移植樹讀它的地方（forms/fShuttleMove.h 的說明）改走「操作員開著這張表單」的分支',
        'int iMoveToTarget=-1;      // golden ShuttleMove.h:91（建構子 :64 設 -1）',
        'DWORD LastClickTime=0;     // golden ShuttleMove.h:92',
        'int LastClickX=-1;         // golden ShuttleMove.h:93',
        'int LastClickY=-1;         // golden ShuttleMove.h:93',
        '#define bShuttleRetry (fShuttleMove->bShuttleRetry)   // golden ShuttleMove.h:110：門面的那一個（golden ckernel.cpp:142 讀；FormShow :87 清成 false＝它的常態值）',
        'SM_TTMyTray SM_mtedInSHSen7DetectPos;    // golden ShuttleMove.h:49 TTMyTray（見 decls）',
        'SM_TTMyTray SM_mtInSHBarCodePos;         // golden ShuttleMove.h:50',
        'SM_TTMyTray SM_mtOutSHDetectPos;         // golden ShuttleMove.h:51',
        'SM_TTMyTray SM_mtedInSHSenICDetectPos;   // golden ShuttleMove.h:68',
    ],
    'replace': [
        ('FormShow', _chk(70, 'Left=100;'), _chk(71, 'Top =20;'), 'Left／Top：視窗位置（HTML 不用）', ';'),
        ('FormShow', _chk(111, 'ReadData();'), 111,
         'ReadData()：golden 表單方法 → 門面 fShuttleMove->ReadData()（forms/fShuttleMove.cpp，golden :2252-2264 逐句；main.cpp／cinitial.cpp 呼叫的同一支）',
         'fShuttleMove->ReadData();'),
        ('sbUpdateClick', _chk(1988, 'ReadData();'), 1988,
         'ReadData()：同 FormShow :111 → 門面 fShuttleMove->ReadData()（寫完重讀，Prod.iInSH?SenICAddPos 變新值）',
         'fShuttleMove->ReadData();'),
        ('sbUpdateClick', _chk(1991, 'fTeach->SaveFile(true);'), 1991,
         'fTeach->SaveFile(true)：golden uteach.cpp:4924（V912 :4939；bSaveByTeach=true：TECH_PARA 的 SetEdit->Text=*Parameter 後寫 teach.ini）→ '
         'FileRW/Teach.cpp 的 FileRW_Teach_SaveFile(true)（同一份 IC_SaveFile；tech.dat 照裁決 14 不寫、回報 todo）',
         'FileRW_Teach_SaveFile(true);'),
    ] + _SSP,
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fShuttleMove.h', 'Motor/mymotor.h'],
    'decls': [
        'void SetTechDataToProd();              // cinitial.h:183（golden cinitial.h:32；cinitial.h 其餘宣告本 TU 不用）',
        'void InitShuttleThreadParameter();     // cinitial.h:143（golden sbUpdateClick :1992「Teach做完要重新Init一次」）',
        'int  GetSHCHKPos(int iSite, int iCenterBase);   // cinitial.h:34（ShowShuttleSensorPosition 的格子數字）',
        'void FileRW_Teach_SaveFile(bool bSaveByTeach);  // FileRW/Teach.cpp：golden TfTeach::SaveFile（uteach.cpp:4924，V912 :4939）',
        'int  FileRW_InArmSuckShtCol();           // FileRW/_KitSuck.cpp：golden InArmSuck.iShtCol（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 衝突）',
        '// golden BarCode.h:83-91 enum eMulti2DType 的三個值（ShowShuttleSensorPosition :2060-2062 用；移植樹只在 BarCode/BarCode_Shuttle2_CCDScan.h:197，那支 header 帶進 TfBarCode 狀態，本 TU 不 include）',
        'static const int e1x2In1CCD=0, e2x2In1CCD=3, e2x2In2CCD=4;',
        '// golden TTMyTray（HTray.h）的 C 路替身：只存 XItem 與 SetCellNumber 的數字（x＝欄、y＝列；DFM 四個格子 YItem 2 或 4、XItem ≤ 16）',
        'struct SM_TTMyTray {',
        '    int  XItem=0;',
        '    int  Cell[16][4]={{0}};',
        '    bool Has[16][4]={{false}};',
        '    bool bPortGap=false;              // golden 填格子的來源沒有移植（mtInSHBarCodePos：fBarCode->GetMovePos）',
        '    void SetCellNumber(int x, int y, int v) { if(x>=0 && x<16 && y>=0 && y<4) { Cell[x][y]=v; Has[x][y]=true; } }',
        '    void Reset() { XItem=0; bPortGap=false; for(int x=0; x<16; x++) for(int y=0; y<4; y++) { Cell[x][y]=0; Has[x][y]=false; } }',
        '};',
    ],
    'overrides': [],
}
