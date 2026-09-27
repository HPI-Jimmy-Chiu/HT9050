# -*- coding: utf-8 -*-
# tools/formbridge/TfSpeed.py -- gen_formbridge.py 的表單設定（一個 BCB 表單一個檔）。
# 欄位說明見 tools/formbridge/README.md。改完跑：python tools/gen_formbridge.py --only TfSpeed
#
# Steven 團隊 20260924（golden = HT9011UC_Code_V3.33.912.0_20260908_Jimmy cSpeed.cpp / cSpeed.h / cSpeed.dfm）
#
# ⚠ 存檔這一輪**不開**（沒有 save／saveFlow → /api/form 回 saveable:false，form.save 回 405）。理由（都是量過的）：
#   1. golden 存檔鈕 spbSaveClick（:1433）**沒有呼叫** SaveSetupFile（:2250）：它自己內嵌一份 WriteIniData，
#      比 SaveSetupFile 多寫約 30 鍵（HP Vacuum、Precisor、Die Clean、Cylinder Delay、Index Cycle Time 監控、
#      E50、Shake Shuttle、iInArmToShtReleaseMode、bYPitchNotUseSearchLastMode…）＋ ReadWriteFile(false)。
#      SaveSetupFile 只由 cBuilder.cpp:527/557（另存配方）與 csystem.cpp:23567 呼叫。
#   2. form.save 的空跑只跑 `save`，頁面也只送 `save` 讀到的欄位（saveReads）。拿 SaveSetupFile 當 save、
#      spbSaveClick 當 saveFlow，spbSaveClick 會把那約 30 個「頁面沒送」的欄位當空字串／false 寫回配方 ——
#      靜默毀掉 ArmCondition.Data。拿 spbSaveClick 當 save 也不行：它有副作用（ReadFile、SECS、備份、
#      dmTrayMotor->StartSetSpeed…）且自己算路徑，空跑會寫到真檔。
#   3. golden SaveSetupFile 是一個參數 (szDir)，產生器的 Save 包裝固定呼叫 (J, szDir, S)，編不過。
#   → 需要產生器／FormSave 的共通規則（見報告），這裡不自己繞。SaveSetupFile／spbSaveClick 仍轉出來、
#     過語法檢查，規則落地後只要補 save／saveFlow 兩個欄位。
import os
import re

_GOLDEN = r'D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'


def _cp950(name):
    return open(os.path.join(_GOLDEN, name), 'rb').read().decode('cp950', errors='replace')


# ----------------------------------------------------------------------------
# TUpDown／TTrackBar 的 Position（產生器的 PROPS 沒有 Position，FormState 也沒有）。
# golden 的用法固定兩種：`udX->Position = 值;` 接著 `edX->Text = udX->Position;`。
# VCL／Win32 語意：UDM_SETPOS 會夾在 [Min, Max]，讀 Position 回夾過的值 —— 所以 edit 顯示的是夾過的值。
# 這裡把 Position 存在表單成員 J.M("udX.Position")，Min／Max 取 golden cSpeed.dfm（沒寫＝VCL 預設
# TUpDown 0..100、TTrackBar 0..10），FormShow 裡 `->Max = …` 改寫 J.M("udX.Max")。
# TTrackBar 的 Position 改變會觸發 OnChange（tbAllSpeedChange…），照呼叫；Max 變小會把 Position 夾下來並觸發
# OnChange（VCL TTrackBar.SetParams）。頁面的 tbAllSpeed 滑桿拿不到 Position（FormState 沒這個屬性）→ 報告提規則。
# 覆寫由下面的程式從 golden 原文逐行產生（不手抄），每一條的原文＝golden 那一行（去掉行尾註解）。
# ----------------------------------------------------------------------------
_DFM = _cp950('cSpeed.dfm')
_CPP = _cp950('cSpeed.cpp')
_LINES = _CPP.split('\n')          # _LINES[n-1] ＝ golden 第 n 行
_TB_ONCHANGE = {'tbAllSpeed': 'tbAllSpeedChange', 'tbAccSpeed': 'tbAccSpeedChange',
                'tbEPControl': 'tbEPControlChange'}


def _dfm_range(w):
    m = re.search(r'object %s: (TUpDown|TTrackBar)\b(.*?)\n\s*end\b' % w, _DFM, re.S)
    if not m:
        raise SystemExit('TfSpeed.py: %s not found in golden cSpeed.dfm' % w)
    body = m.group(2)
    lo = re.search(r'\n\s*Min = (-?\d+)', body)
    hi = re.search(r'\n\s*Max = (-?\d+)', body)
    dmax = 100 if m.group(1) == 'TUpDown' else 10
    return int(lo.group(1)) if lo else 0, int(hi.group(1)) if hi else dmax


def _pos_reads(expr):
    return re.sub(r'\b((?:ud|tb)\w+)->Position\b', r'J.M("\1.Position")', expr)


def _method_lines(name):
    m = re.search(r'^[^\n]*\bTfSpeed::' + name + r'\s*\(', _CPP, re.M)
    i = _CPP.index('{', m.end())
    first = _CPP.count('\n', 0, i) + 1          # golden 行號（`{` 那一行）
    d, j = 1, i + 1
    while d:
        c = _CPP[j]
        if c == '{':
            d += 1
        elif c == '}':
            d -= 1
        j += 1
    return [(first + k, ln) for k, ln in enumerate(_CPP[i:j].split('\n'))]


def _strip(line):
    code = line.split('//', 1)[0]
    return code.strip()


def _position_overrides(methods, skip):
    """skip = {方法: [(起, 迄)]}：被 blocks 整段換掉的 golden 行不產生覆寫（否則產生器報 override not used）。"""
    out, seen = [], set()
    for meth in methods:
        for ln, raw in _method_lines(meth):
            if any(a <= ln <= b for a, b in skip.get(meth, [])):
                continue
            code = _strip(raw)
            if not code or (meth, code) in seen:
                continue
            new = None
            m = re.match(r'^((?:ud|tb)\w+)->Position\s*=\s*(.+);$', code)
            if m:
                w, rhs = m.group(1), _pos_reads(m.group(2))
                lo, hi = _dfm_range(w)
                clamp = ('const int mx_=J.M("%s.MaxSet")?J.M("%s.Max"):%d; int v_=(int)(%s); '
                         'if(v_<%d) v_=%d; if(v_>mx_) v_=mx_;' % (w, w, hi, rhs, lo, lo))
                if w in _TB_ONCHANGE:
                    new = ('{ %s if(J.M("%s.Position")!=v_) { J.M("%s.Position")=v_; B_%s(J); } }'
                           '   /* golden: %s.Position=…（夾在 dfm Min..Max，值變了觸發 OnChange）*/'
                           % (clamp, w, w, _TB_ONCHANGE[w], w))
                else:
                    new = ('{ %s J.M("%s.Position")=v_; }   /* golden: %s.Position=…（UDM_SETPOS 夾在 dfm Min..Max）*/'
                           % (clamp, w, w))
            if new is None:
                m = re.match(r'^(\w+)->Text\s*=\s*((?:ud|tb)\w+)->Position;$', code)
                if m:
                    new = 'J.SetText("%s", AnsiString(J.M("%s.Position")));' % (m.group(1), m.group(2))
            if new is None:
                m = re.match(r'^((?:ud|tb)\w+)->Max\s*=\s*(.+);$', code)
                if m:
                    w, rhs = m.group(1), m.group(2)
                    new = 'J.M("%s.Max")=(%s); J.M("%s.MaxSet")=1;' % (w, rhs, w)
                    if w in _TB_ONCHANGE:
                        new = ('{ %s if(J.M("%s.Position")>J.M("%s.Max")) { J.M("%s.Position")=J.M("%s.Max"); B_%s(J); } }'
                               '   /* VCL TTrackBar.SetParams：Max 變小把 Position 夾下來並觸發 OnChange */'
                               % (new, w, w, w, w, _TB_ONCHANGE[w]))
            if new is None:
                # 跨行的 `W->Prop = (a ||` / `b);`：產生器以行為單位，右式跨行會改壞（gen_formbridge.py 檔頭 ⚠）。
                # 整條收齊成一行改寫，續行換成註解。
                m = re.match(r'^(\w+)->(Visible|Enabled|TabVisible|Checked)\s*=\s*(.*)$', code)
                if m and not code.endswith(';'):
                    parts, k = [m.group(3)], ln + 1
                    conts = []
                    while True:
                        c = _strip(_LINES[k - 1])
                        conts.append(c)
                        parts.append(c)
                        if c.endswith(';'):
                            break
                        k += 1
                    rhs = ' '.join(parts)[:-1]
                    new = 'J.Set%s("%s", %s);   /* golden :%d-%d 跨行，收成一行 */' % (m.group(2), m.group(1), rhs, ln, k)
                    for c in conts:
                        out.append((meth, c, '/* （續上一條 golden :%d）*/' % ln))
                        seen.add((meth, c))
            if new is None:
                m = re.match(r'^(tb\w+)->SelEnd\s*=\s*(tb\w+)->Position;$', code)
                if m:
                    new = '/* golden: %s.SelEnd=%s.Position —— 滑桿的選取段，純畫面 */' % (m.group(1), m.group(2))
            if new is not None:
                seen.add((meth, code))
                out.append((meth, code, new))
    return out


_METHODS = ['FormShow', 'ReadFile', 'ReadWriteFile', 'DoIniDataToForm',
            'tbAllSpeedChange', 'tbAccSpeedChange', 'tbEPControlChange',
            'spbSaveClick', 'SaveSetupFile']

FORM = {
    'class': 'TfSpeed',
    'cpp': 'cSpeed.cpp',
    'h': 'cSpeed.h',
    'page': 'Setup.Speed.html',
    'struct': 'ArmSpeed_File',
    'files': ['ArmCondition.Data'],
    'also': ['SHSpeed_File', 'MGSpeed_File', 'TestIF_File（ArmCondition.Data 裡的 Index Cycle Time／Shake／Tray Z 速度等鍵）'],
    # golden 開表單＝建構子（:31-39）＋ FormShow（:41，內含 ReadFile＋DoIniDataToForm）。
    # ReadFile（:340）／ReadWriteFile（:784）轉 golden 版而不是接移植樹的 fSpeed->ReadFile：
    # 結構的讀法與移植樹 cSpeed.cpp:426 逐行相同（20260924 diff -w 量過，只差 GATE S5/S6），
    # 但 golden ReadFile 自己也設 widget（tbAllSpeed／tbAccSpeed／tbEPControl 的 Position、edAllSpeed、
    # edAllAccSpeed、edEPControl、gbEPControl、Tray Z 速度 edit），接移植樹會讓那些值只留在移植樹的 facade、不進 J。
    # tb*Change 是 TTrackBar 的 OnChange（Position 改變時 VCL 觸發）。
    # spbSaveClick／SaveSetupFile 轉出來但這輪不接（見檔頭）。
    'methods': _METHODS,
    'members': ['fShow'],
    'display': [
        '// golden 建構子 cSpeed.cpp:35-37（LastFileName="" 與 :38 Hint 不影響畫面）',
        'J.SetEnabled("tbAllSpeed", false);',
        'J.SetEnabled("spbSpeedAdd", false);',
        'J.SetEnabled("spbSpeedDec", false);',
        'B_FormShow(J);',
    ],
    # 'save' / 'saveFlow'：這輪不設（見檔頭 1-3）
    'sourceGap': '',
    'includes': ['cprod.h', 'Config.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h', 'LastSet.h',
                 'vclcompat/SysUtils.h', 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 'forms/fMain.h', 'forms/fSecurity.h', 'csystem.h', 'cAuthority.h', 'aHotPlateSubstrate.h',
                 # IniRecordMonitoringIndexCycleTime（spbSaveClick :1529，本體 cObserver.cpp:3129）
                 'forms/fObserver.h'],
    'blocks': [
        # golden ReadWriteFile :797-879 —— Loader/Empty/Color/Auto Tray 的 Y 步進速度，全部經 dmTrayMotor
        # （golden Motor/TrayStepMotor，TDataModule）。移植樹沒有 dmTrayMotor（cSpeed.cpp:888 GATE S5）。
        ('ReadWriteFile', 797, 879, 'if(LoaderUnload_StepMotor)',
         'if(LoaderUnload_StepMotor)                                                  //Steven 20200529 : Loader入Tray改步進\n'
         '    {\n'
         '        J.Todo("golden ReadWriteFile :797-879 tray Y step-motor speeds (dmTrayMotor->iStepMotorSpeed[], edtLoaderSpeed1..edtAuto6Speed1, edtLoaderSpeed2) not ported (no dmTrayMotor in the port, cSpeed.cpp GATE S5) -- not shown, not saved");\n'
         '    }'),
        # golden spbSaveClick :1739-1761 Recipe Parameter Default 比對（fRPDefault／SearchRecipeParameter／
        # fCleaning／FTestIF／fYieldMonitoring->SearchRecipeParameter）—— 移植樹都沒有（forms/fSpeed.h GATE S4）。
        ('spbSaveClick', 1739, 1761, 'if(CosFunction.bRecipeParameterDefault)',
         'if(CosFunction.bRecipeParameterDefault)                                     //Isaac 20170527 (Steven) defalut值比較功能\n'
         '    {\n'
         '        J.Todo("golden spbSaveClick :1739-1761 Recipe Parameter Default compare (fRPDefault / SearchRecipeParameter) not ported");\n'
         '    }'),
    ],
    'overrides': _position_overrides(_METHODS, {'ReadWriteFile': [(797, 879)]}) + [
        # ---- FormShow ----
        # 表單成員 LastFileName（AnsiString，J.M 只放 int）：golden 在 FormShow／ReadFile 設成 GetLastOpenFN()，
        # 之後只拿來組路徑與標題 —— 直接呼叫 GetLastOpenFN()（同一次操作中配方不會換）。
        ('FormShow', 'LastFileName=GetLastOpenFN();', '/* golden: LastFileName=GetLastOpenFN(); —— 表單成員，用到的地方直接呼叫 GetLastOpenFN() */'),
        ('FormShow', 'S.sprintf("Speed Condition  \'\'%s\'\'  ",LastFileName);', 'S.sprintf("Speed Condition  \'\'%s\'\'  ",GetLastOpenFN());'),
        # :49 ActivePage 不在 PROPS；tsAllSpeed 是 PageControl1 的第 0 頁（golden cSpeed.dfm:37）
        ('FormShow', 'PageControl1->ActivePage=tsAllSpeed;', 'J.SetActivePageIndex("PageControl1", 0);   // golden: PageControl1.ActivePage=tsAllSpeed（dfm 第 0 頁）'),
        # :174 golden 在 widget 名與 -> 之間多一個空白，產生器的樣式對不到
        ('FormShow', 'gbOutArmWaitTime ->Enabled    =fSecurity->Insufficient(142, false);',
         'J.SetEnabled("gbOutArmWaitTime", fSecurity->Insufficient(142, false));'),
        # :208 焦點，純畫面
        ('FormShow', 'rbTemp->SetFocus();', '/* golden: rbTemp.SetFocus() —— 焦點，HTML 不用 */'),
        # ---- ReadFile ----
        ('ReadFile', 'LastFileName=GetLastOpenFN();', '/* golden: LastFileName=GetLastOpenFN(); —— 見 FormShow */'),
        ('ReadFile', 'szDir.sprintf("%s%s", DataPath, LastFileName);', 'szDir.sprintf("%s%s", DataPath, GetLastOpenFN());   // golden: LastFileName'),
        # :765-766 測 UPH 用的全域，來源是 grpIndexUPHTryRun 裡的 edit（只有 DEBUG_INDEX_UPH 才看得到）。
        # bridge 的 J 沒有它們 → 會讀到 "" 把全域蓋成 0。照 golden dfm 的 Text（cSpeed.dfm edtTrySpeed '900000'、
        # edtTryAcc '5500000'）—— 非 DEBUG 版 golden 這兩個 edit 永遠是 dfm 值。
        ('ReadFile', 'iIndexSpeed=atoi(edtTrySpeed->Text.c_str());',
         'iIndexSpeed=atoi("900000");   // golden: atoi(edtTrySpeed.Text.c_str())，dfm Text=\'900000\'（edit 只在 DEBUG_INDEX_UPH 可見）'),
        ('ReadFile', 'iIndexAcc=atoi(edtTryAcc->Text.c_str());',
         'iIndexAcc=atoi("5500000");    // golden: atoi(edtTryAcc.Text.c_str())，dfm Text=\'5500000\''),
        # ---- spbSaveClick（這輪不接，仍要能編）----
        ('spbSaveClick', 'szDir.sprintf("%s%s", DataPath, LastFileName);', 'szDir.sprintf("%s%s", DataPath, GetLastOpenFN());   // golden: LastFileName'),
        ('spbSaveClick', 'if(MyMessageBox->Visible==true)',
         'J.Todo("golden spbSaveClick :1523-1526 MyMessageBox->Close() before the cycle-time alarm reset -- not done by the bridge");\n'
         '        if(false)'),
        ('spbSaveClick', 'MyMessageBox->Close();', '/* golden: MyMessageBox->Close(); （見上一行 Todo）*/'),
        ('spbSaveClick', 'fShowMessage->sgdSpeedView->Refresh();',
         '/* golden: fShowMessage->sgdSpeedView->Refresh(); —— 主畫面速度表重畫，web 另外刷新 */'),
        ('spbSaveClick', 'dmTrayMotor->StartSetSpeed();',
         'J.Todo("golden spbSaveClick :1784 dmTrayMotor->StartSetSpeed() not ported (no dmTrayMotor, cSpeed.cpp GATE S5) -- tray step-motor speeds are NOT pushed to the drivers");'),
    ],
}


# ----------------------------------------------------------------------------
# 自我檢查：產生器的 override 是「片段包含」比對、每行取第一個命中的 —— 片段太短會搶到別行。
# 每一行 golden 程式碼最多只能被一條 override 命中，否則中止（不讓產生器默默蓋錯行）。
# ----------------------------------------------------------------------------
def _check_overrides(form):
    bad = []
    blk = {}
    for meth, a, b, _must, _new in form.get('blocks', []):
        blk.setdefault(meth, []).append((a, b))
    for meth in form['methods']:
        frags = [o for (m, o, _n) in form['overrides'] if m == meth]
        for ln, raw in _method_lines(meth):
            if any(a <= ln <= b for a, b in blk.get(meth, [])):
                continue
            code = raw.split('//', 1)[0]
            hits = [f for f in frags if f in code]
            if len(hits) > 1:
                bad.append('%s :%d matched by %d overrides: %r' % (meth, ln, len(hits), hits))
    if bad:
        raise SystemExit('TfSpeed.py override self-check failed:\n  ' + '\n  '.join(bad))


_check_overrides(FORM)
