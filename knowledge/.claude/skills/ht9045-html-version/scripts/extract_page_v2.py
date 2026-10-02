# -*- coding: utf-8 -*-
# Steven 20260915
# ----------------------------------------------------------------------
# 表單範圍鎖定的抽取器（v1 不鎖範圍會把值寫進錯的檔還不報錯）。
# 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
# ----------------------------------------------------------------------

"""extract_page_v2.py -- 頁面 -> (配方對照表 + 小鍵盤旗標) 的機械抽取器。

AI(W906-FW-EXTRACT2) 20260914。

v1 (extract_page_map.py) 的致命缺陷：全樹依 widget id 反查，沒有限定來源檔。
Setup.HotPlate 與 Setup.TrayForm 都有 XCT1/XST1/... ，v1 把 TrayForm 的註冊
安到 HotPlate 頭上，照抽會把 HotPlate 的值寫進 Tray.Data。

v2 的作法：
  1. 用 .dfm 的 `object <id>:` 與頁面 id 的重疊度，鎖定「這一頁屬於哪個表單」
  2. 只從那個表單的 .cpp 抽，來源是 WriteIniData 那一側 —— 它把
     (區段, 鍵) 與 widget 寫在同一行，是最不會出錯的來源
  3. 小鍵盤：.cpp 的 ShowQwertyKey(Sender, FLAGS, dp, checkRange, min, max)
     配上 .dfm 的 OnMouseDown = <handler>，得到每個 widget 的旗標與範圍
  4. 掃全部配方，算每個鍵的涵蓋率（分母只算「有那份文件」的配方）

用法: py extract_page_v2.py <page.html> [...]
"""
import os, re, sys, glob, json, collections

GOLDEN  = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
RECIPES = r'D:\HT9045\IniData\Data'

def read(p):
    for enc in ('utf-8', 'cp950', 'latin-1'):
        try: return open(p, encoding=enc, errors='strict').read()
        except (UnicodeDecodeError, LookupError): continue
    return open(p, encoding='latin-1', errors='replace').read()

RE_OBJ   = re.compile(r'^\s*object\s+([A-Za-z_]\w*)\s*:\s*(\w+)', re.M)
RE_ONMD  = re.compile(r'^\s*On(MouseDown|Click|Enter|DblClick)\s*=\s*(\w+)', re.M)
Q        = r'(?:AnsiString\s*\(\s*)?"([^"]*)"\s*\)?'
RE_WRITE = re.compile(r'WriteIniData\s*\(\s*[^,]+,\s*' + Q + r'\s*,\s*' + Q + r'\s*,([^;]*);', re.S)
RE_READ  = re.compile(r'([A-Za-z_]\w*)\s*->\s*Text\s*=\s*(?:Check)?(?:And)?ReadIniData\s*\(\s*[^,]+,\s*' + Q + r'\s*,\s*' + Q, re.S)
RE_FUNC  = re.compile(r'void\s+__fastcall\s+\w+::(\w+)\s*\(')
RE_SQK   = re.compile(r'ShowQwertyKey\s*\(\s*[^,]+,\s*N_(\w+)((?:\s*,\s*[^,()]+)*)\s*\)')
RE_ADD   = re.compile(
    r'(?<![A-Za-z0-9_])el\w*->Add\(\s*([A-Za-z_]\w*)\s*,\s*&?([^,]+?)\s*,\s*(EC\w+)\s*,\s*'
    + Q + r'\s*,\s*' + Q, re.S)
RE_MEMREAD = re.compile(r'([A-Za-z_][\w\[\].]*)\s*=\s*(?:Check)?(?:And)?ReadIniData\s*\(\s*[^,]+,\s*' + Q + r'\s*,\s*' + Q, re.S)
RE_MEMWRITE = re.compile(r'WriteIniData\s*\(\s*[^,]+,\s*' + Q + r'\s*,\s*' + Q + r'\s*,\s*([A-Za-z_][\w\[\].]*)\s*\)', re.S)

# Steven 20260916
# AI(W906-FW-SYSWIRE) 20260916：補齊 golden 的 ini 呼叫家族。
# v2 原本只認 `WriteIniData(` 與 `ReadIniData(`，而 golden 實際上有五種：
#     WriteIniData(file, sec, key, val)            2583 處
#     (Check)(And)ReadIniData(file, sec, key, def) 2443 處
#     WriteIniDataGeneral(sec, key, val)            381 處 ← 無檔名參數，固定 Gerneral.ini
#     CheckAndReadIniDataGeneral(sec, key, def)     730 處 ← 同上
#     WriteIniDataNoLog(file, sec, key, val)         38 處
# 漏掉 General 家族的後果是可量測的：HandlerSys.cpp 有 506 個 ini 呼叫，
# 舊版只抽到 3 個欄位，因為它整頁都是 WriteIniDataGeneral。
# 注意 `WriteIniData\s*\(` 不會誤命中 `WriteIniDataGeneral(`／`WriteIniDataNoLog(`
# （後面緊接的是字母不是括號），所以新舊規則不會重複計算。
RE_WRITE_G  = re.compile(r'WriteIniDataGeneral\s*\(\s*' + Q + r'\s*,\s*' + Q + r'\s*,([^;]*);', re.S)
RE_READ_G   = re.compile(r'([A-Za-z_]\w*)\s*->\s*Text\s*=\s*(?:Check)?(?:And)?ReadIniDataGeneral'
                         r'\s*\(\s*' + Q + r'\s*,\s*' + Q, re.S)
RE_WRITE_NL = re.compile(r'WriteIniDataNoLog\s*\(\s*([^,]+),\s*' + Q + r'\s*,\s*' + Q + r'\s*,([^;]*);', re.S)

# 帶檔名參數的寫入：把第 1 個參數也抓出來。
# 這不是可有可無的 —— uteach.cpp 同一個表單就寫三個目標（asTeachPath 11 處、
# asGeneralPath 4 處、區域變數 szDir 1 處）。若假設「一頁對一個檔」，那 4 個
# Gerneral.ini 的欄位會被當成 teach.ini 的鍵寫出去，而且兩邊都有同名區段時
# 不會報錯 —— 正是 20260915 記取的「寫錯檔不報錯」那一類失敗。
RE_WRITE_F  = re.compile(r'WriteIniData\s*\(\s*([^,]+),\s*' + Q + r'\s*,\s*' + Q + r'\s*,([^;]*);', re.S)

# 非文字控制項。golden 用 ->Checked（TCheckBox）與 ->ItemIndex（TRadioGroup/TComboBox）。
# 這些**不放進 row['fields']**，另存 row['props'] —— fields 的語意是「文字欄位對照表」，
# 混進去會讓既有 17 個接線檔的 fields/optional/pending 分類全部變動。
RE_PROP     = re.compile(r'\b([A-Za-z_]\w*)\s*->\s*(Checked|ItemIndex)\b')

# el<名字>->Add(...) 的清單物件各自綁一個檔，不能假設同一個 cpp 只有一個。
RE_ELFILE   = re.compile(r'\b(el[A-Za-z0-9_]*)\s*->\s*(?:Read|Write)EditText(?:From|To)File\s*\(\s*([^;]*?)\)', re.S)
RE_ELADD    = re.compile(r'(?<![A-Za-z0-9_])(el\w*)->Add\(')

# 檔名運算式 -> wb_serve /api/system/<name>。只列已查證過的；認不得就回 None，
# 讓 gen_wire 略過而不是猜一個。
SYSFILE_ALIAS = {'asteachpath': 'teach', 'asgeneralpath': 'gerneral',
                 'asconfigpath': 'config'}


def resolve_sysfile(expr, localmap):
    """把 WriteIniData 的第一個參數解析成系統檔代號，認不出來回 None。"""
    if not expr:
        return None
    t = expr.strip().strip('()').split('.')[-1].strip()
    t = re.sub(r'^[&*]+', '', t)
    low = t.lower()
    if low in SYSFILE_ALIAS:
        return SYSFILE_ALIAS[low]
    if low in localmap:                      # 區域變數（szDir = asTeachPath）
        return localmap[low]
    return None


def local_file_vars(ct):
    """抓 `szDir = asTeachPath;` 與 `szDir.sprintf("%s", asTeachPath);` 這類轉手。"""
    out = {}
    for m in re.finditer(r'\b([A-Za-z_]\w*)\s*=\s*([A-Za-z_]\w*)\s*;', ct):
        tgt = SYSFILE_ALIAS.get(m.group(2).lower())
        if tgt:
            out[m.group(1).lower()] = tgt
    for m in re.finditer(r'\b([A-Za-z_]\w*)\s*\.\s*sprintf\s*\(\s*"[^"]*"\s*,\s*([A-Za-z_]\w*)\s*\)', ct):
        tgt = SYSFILE_ALIAS.get(m.group(2).lower())
        if tgt:
            out[m.group(1).lower()] = tgt
    # Steven 20260916
    # uteach 把路徑拆成兩半再餵給清單物件：
    #     AnsiString FileName = ExtractFileName(asTeachPath);
    #     AnsiString FilePath = ExtractFilePath(asTeachPath);
    #     elTeach->ReadEditTextFromFile(FilePath, FileName);
    # 不追這一層，140 個 HTEditList 欄位就查不出目標檔。
    for m in re.finditer(r'\b([A-Za-z_]\w*)\s*=\s*Extract(?:FileName|FilePath|FileDir)'
                         r'\s*\(\s*([A-Za-z_]\w*)\s*\)', ct):
        tgt = SYSFILE_ALIAS.get(m.group(2).lower())
        if tgt:
            out[m.group(1).lower()] = tgt
    return out
RE_WTEXT   = re.compile(r'([A-Za-z_]\w*)\s*->\s*Text\s*=([^;]{0,240});', re.S)
SKIP_TOK = {'Text','Caption','AnsiString','FloatToStrF','FloatToStr','IntToStr','StrToInt',
            'StrToFloat','ffFixed','ffGeneral','Items','CommaText','Value','Format',
            'ReadIniData','CheckAndReadIniData','WriteIniData','szDir','szDir2','FileName',
            'true','false','FormatFloat','Trim','Now','Date'}
RE_TAG   = re.compile(r'<\s*(\w+)([^>]*?\bid\s*=\s*["\']([^"\']+)["\'][^>]*)>', re.I | re.S)

def page_ids(path):
    out = {}
    for m in RE_TAG.finditer(read(path)):
        tag, attrs, pid = m.group(1).lower(), m.group(2), m.group(3)
        ty = tag
        if tag == 'input':
            t = re.search(r'\btype\s*=\s*["\']([^"\']+)["\']', attrs, re.I)
            ty = (t.group(1).lower() if t else 'text')
        out.setdefault(pid, ty)
    return out

def dfm_objects(path):
    s = read(path)
    objs, handlers, cur = {}, {}, None
    for line in s.splitlines():
        m = RE_OBJ.match(line)
        if m:
            cur = m.group(1); objs[cur] = m.group(2); continue
        h = RE_ONMD.match(line)
        if h and cur:
            handlers.setdefault(cur, {})[h.group(1)] = h.group(2)
    return objs, handlers

def pick_form(ids, dfms):
    best, score = None, 0
    for d, (objs, _h) in dfms.items():
        n = len(set(ids) & set(objs))
        if n > score: best, score = d, n
    return best, score

def sqk_by_handler(cpp_text):
    """handler 名 -> (flags, dp, checkRange, min, max)"""
    out, cur = {}, None
    for line in cpp_text.splitlines():
        f = RE_FUNC.search(line)
        if f: cur = f.group(1)
        q = RE_SQK.search(line)
        if q and cur:
            rest = [a.strip() for a in q.group(2).split(',') if a.strip()]
            dp = rest[0] if len(rest) > 0 else None
            cr = rest[1] if len(rest) > 1 else 'false'
            mn = rest[2] if len(rest) > 2 else None
            mx = rest[3] if len(rest) > 3 else None
            num = re.compile(r'^-?\d+(\.\d+)?$')
            # min/max 若是 C++ 變數（InputLimit.dContactHigh 之類），JS 這邊拿不到值，
            # 所以關掉夾限而不是猜一個數字 —— 猜錯的夾限比沒有夾限更危險。
            if not (mn and mx and num.match(mn) and num.match(mx)):
                cr, mn, mx = 'false', None, None
            if dp is not None and not num.match(dp):
                dp = None
            out.setdefault(cur, (q.group(1), dp, cr, mn, mx))
    return out

def main():
    pages = [a for a in sys.argv[1:] if not a.startswith('--')]
    dfms = {}
    for dp, dn, fn in os.walk(GOLDEN):
        if '.svn' in dp.split(os.sep): continue
        for f in fn:
            if f.lower().endswith('.dfm'):
                p = os.path.join(dp, f)
                try: dfms[p] = dfm_objects(p)
                except OSError: pass
    sys.stderr.write('dfm forms indexed: %d\n' % len(dfms))

    # 配方索引
    per, filecount = {}, collections.Counter()
    for d in sorted(glob.glob(os.path.join(RECIPES, '*'))):
        if not os.path.isdir(d): continue
        ks = {}
        for f in glob.glob(os.path.join(d, '*.Data')) + glob.glob(os.path.join(d, '*.ini')):
            fb = os.path.basename(f).lower(); filecount[fb] += 1; sec = ''
            for line in read(f).splitlines():
                line = line.strip()
                if line.startswith('[') and line.endswith(']'): sec = line[1:-1].strip()
                elif '=' in line and not line.startswith((';', '#')):
                    ks.setdefault((sec.lower(), line.split('=',1)[0].strip().lower()), fb)
        per[os.path.basename(d)] = ks
    sys.stderr.write('recipes indexed: %d\n' % len(per))

    rows = []
    for page in pages:
        ids = page_ids(page)
        dfm, score = pick_form(ids, dfms)
        row = dict(page=os.path.basename(page), ids=len(ids), form=None, overlap=score,
                   fields={}, props={}, keyboards={}, notes=[])   # Steven 20260916 props
        if not dfm:
            row['notes'].append('找不到對應表單'); rows.append(row); continue
        row['form'] = os.path.basename(dfm)
        objs, handlers = dfms[dfm]
        cpp = os.path.splitext(dfm)[0] + '.cpp'
        if not os.path.isfile(cpp):
            row['notes'].append('找不到 %s' % os.path.basename(cpp)); rows.append(row); continue
        ct = read(cpp)

        # Steven 20260916
        # 檔名運算式的區域轉手表，先建好給下面每一條寫入規則用。
        localmap = local_file_vars(ct)

        # --- 對照表：WriteIniData 那一側（widget 在同一行） ---
        for m in RE_WRITE_F.finditer(ct):
            fexpr, sec, key, rhs = (m.group(1), m.group(2).strip(),
                                    m.group(3).strip(), m.group(4))
            sysfile = resolve_sysfile(fexpr, localmap)          # Steven 20260916
            w = re.search(r'\b([A-Za-z_]\w*)\s*->\s*Text\b', rhs)
            if w:
                wid = w.group(1)
                if wid in ids:
                    row['fields'][wid] = dict(section=sec, key=key, via='WriteIniData',
                                              sysfile=sysfile, html=ids[wid])
            # Steven 20260916：非文字控制項另存 props，不動 fields 的語意。
            for pm in RE_PROP.finditer(rhs):
                wid, prop = pm.group(1), pm.group(2)
                if wid in ids and wid not in row['props']:
                    row['props'][wid] = dict(section=sec, key=key, prop=prop,
                                             via='WriteIniData', sysfile=sysfile,
                                             html=ids[wid])

        # Steven 20260916
        # --- WriteIniDataNoLog：與上面同形，只差沒有寫操作紀錄 ---
        for m in RE_WRITE_NL.finditer(ct):
            fexpr, sec, key, rhs = (m.group(1), m.group(2).strip(),
                                    m.group(3).strip(), m.group(4))
            sysfile = resolve_sysfile(fexpr, localmap)
            w = re.search(r'\b([A-Za-z_]\w*)\s*->\s*Text\b', rhs)
            if w and w.group(1) in ids and w.group(1) not in row['fields']:
                row['fields'][w.group(1)] = dict(section=sec, key=key, via='WriteIniDataNoLog',
                                                 sysfile=sysfile, html=ids[w.group(1)])
            for pm in RE_PROP.finditer(rhs):
                wid, prop = pm.group(1), pm.group(2)
                if wid in ids and wid not in row['props']:
                    row['props'][wid] = dict(section=sec, key=key, prop=prop,
                                             via='WriteIniDataNoLog', sysfile=sysfile,
                                             html=ids[wid])

        # Steven 20260916
        # --- General 家族：沒有檔名參數，目標一律是 Gerneral.ini ---
        for m in RE_WRITE_G.finditer(ct):
            sec, key, rhs = m.group(1).strip(), m.group(2).strip(), m.group(3)
            w = re.search(r'\b([A-Za-z_]\w*)\s*->\s*Text\b', rhs)
            if w and w.group(1) in ids and w.group(1) not in row['fields']:
                row['fields'][w.group(1)] = dict(section=sec, key=key,
                                                 via='WriteIniDataGeneral',
                                                 sysfile='gerneral', html=ids[w.group(1)])
            for pm in RE_PROP.finditer(rhs):
                wid, prop = pm.group(1), pm.group(2)
                if wid in ids and wid not in row['props']:
                    row['props'][wid] = dict(section=sec, key=key, prop=prop,
                                             via='WriteIniDataGeneral', sysfile='gerneral',
                                             html=ids[wid])
        for m in RE_READ_G.finditer(ct):
            wid, sec, key = (g.strip() for g in m.groups())
            if wid in ids and wid not in row['fields']:
                row['fields'][wid] = dict(section=sec, key=key, via='ReadIniDataGeneral',
                                          sysfile='gerneral', html=ids[wid])

        # --- 補：讀取端的直接一行 ---
        for m in RE_READ.finditer(ct):
            wid, sec, key = (g.strip() for g in m.groups())
            if wid in ids and wid not in row['fields']:
                row['fields'][wid] = dict(section=sec, key=key, via='ReadIniData',
                                          sysfile=None, html=ids[wid])

        # --- 補：HTEditList 一跳法（限定本表單的 cpp，含 C++ 型別） ---
        # Steven 20260916：每個 el<名字> 各自綁一個檔，先查出來再套到它的 Add()。
        elfiles = {}
        for m in RE_ELFILE.finditer(ct):
            tgt = None
            for tok in re.findall(r'[A-Za-z_]\w*', m.group(2)):
                tgt = tgt or resolve_sysfile(tok, localmap)
            if tgt:
                elfiles[m.group(1)] = tgt
        for m in RE_ADD.finditer(ct):
            wid, mem, cty, sec, key = (g.strip() for g in m.groups())
            if wid in ids and wid not in row['fields']:
                # RE_ADD 的比對本身就是從 el<名字>->Add( 開頭，直接取即可。
                owner = re.match(r'(el\w*)', m.group(0))
                row['fields'][wid] = dict(section=sec, key=key, via='HTEditList',
                                          ctype=cty, html=ids[wid],
                                          sysfile=elfiles.get(owner.group(1)) if owner else None)

        # Steven 20260916
        # --- 補：teach 的 TECH_PARA 綁定（uteach.cpp 專用） ---
        # golden 把 teach.ini 的讀寫包在 TECH_PARA / TECH_TWOPARA 物件裡：
        #     new TECH_PARA(&Tech.iInArmSafeZ1, MInArmZA, setEditInZSafeHeight, "setEditInZSafeHeight", ...)
        #     new TECH_TWOPARA(&p1, &p2, MInArmX, MInArmY, setEditPreciserX, setEditPreciserY, "k1", "k2", ...)
        # 寫入是 WriteIniData(asTeachPath, MOT[MotorSelect].Alias, Key, SetEdit->Text)：
        # 區段 = 該馬達在 Mot_Table.csv 的 Alias，鍵 = 建構式裡的字面值（實測 387 筆全部 == widget 名）。
        # 這裡把 (widget, 馬達 enum, 鍵) 抽出來當對照表，區段先寫 enum 名 —— 在這台機器上
        # Alias 與 enum 名相同（M00 MInArmX ...），gen_wire 會再對 live teach.ini 逐鍵驗證，
        # 對不上就落到 sysAbsent，不會寫錯地方。
        # 同一個 widget 綁到兩個馬達（setEditTestZSafePos -> MTestZ1 與 MTestZ2）無法用
        # 「一個 id 一個鍵」表達，記到 row['ambiguous']，不接。
        RE_TP  = re.compile(r'new\s+TECH_PARA\s*\(\s*&?[^,]+?\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*"([^"]*)"', re.S)
        RE_TTP = re.compile(r'new\s+TECH_TWOPARA\s*\(\s*&?[^,]+?\s*,\s*&?[^,]+?\s*,\s*(\w+)\s*,\s*(\w+)\s*,'
                            r'\s*(\w+)\s*,\s*(\w+)\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"', re.S)
        tp = {}
        for m in RE_TP.finditer(ct):
            tp.setdefault(m.group(2), set()).add((m.group(1), m.group(3)))
        for m in RE_TTP.finditer(ct):
            tp.setdefault(m.group(3), set()).add((m.group(1), m.group(5)))
            tp.setdefault(m.group(4), set()).add((m.group(2), m.group(6)))
        # Steven 20260916（審查 F6）：TechSuckPara 的 2×8 陣列其實有靜態 widget：
        #   teInArm[2][8]  = {{Z1A,Z1C,Z1E,Z1G,Z1I,Z1K,Z1M,Z1O},{Z1B,Z1D,...,Z1P}}  區段 "InArmZSub"
        #   teOutArm[2][8] = 同款 Z2*                                              區段 "OutArmZSub"
        #   teSortArm[2]   = {editsetEditZ3A, editsetEditZ3B}                       區段 "SortArmZSub"
        # 鍵 = "Picker" + IndexSuckName[i][j]（cmydef.cpp:14，值是 " Aa".." Ah" / " Ba".." Bh"，帶前置空白）。
        # 區段是 TechSuckPara.Group 的靜態字串（uteach.cpp:372-376），不是馬達 Alias。
        suck_rows = ['ACEGIKMO', 'BDFHJLNP']
        suck_names = [' Aa', ' Ab', ' Ac', ' Ad', ' Ae', ' Af', ' Ag', ' Ah',
                      ' Ba', ' Bb', ' Bc', ' Bd', ' Be', ' Bf', ' Bg', ' Bh']
        for arm, sec in (('1', 'InArmZSub'), ('2', 'OutArmZSub')):
            for i in range(2):
                for j in range(8):
                    wid = 'setEditZ%s%s' % (arm, suck_rows[i][j])
                    tp.setdefault(wid, set()).add((sec, 'Picker' + suck_names[i * 8 + j]))
        tp.setdefault('editsetEditZ3A', set()).add(('SortArmZSub', 'Picker Aa'))
        tp.setdefault('editsetEditZ3B', set()).add(('SortArmZSub', 'Picker Ab'))

        row['ambiguous'] = {}
        for wid, binds in tp.items():
            if wid not in ids or wid in row['fields']:
                continue
            if len(binds) > 1:
                row['ambiguous'][wid] = dict(sysfile='teach', binds=sorted(binds),
                                             why='同一個 widget 綁到 %d 個馬達區段' % len(binds))
                continue
            mo, key = next(iter(binds))
            row['fields'][wid] = dict(section=mo, key=key, via='TECH_PARA',
                                      sysfile='teach', html=ids[wid])

        # --- 補：兩跳法（限定本表單的 cpp，所以沒有跨頁撞名風險） ---
        mem2key = {}
        for m in RE_MEMREAD.finditer(ct):
            mem, sec, key = (g.strip() for g in m.groups())
            if '->' in mem:
                continue
            short = mem.split('.')[-1]
            if short in SKIP_TOK or len(short) < 4:
                continue
            mem2key.setdefault(short, (sec, key))
        for m in RE_MEMWRITE.finditer(ct):
            sec, key, mem = (g.strip() for g in m.groups())
            short = mem.split('.')[-1]
            if short in SKIP_TOK or len(short) < 4:
                continue
            mem2key.setdefault(short, (sec, key))
        w2m = {}
        for m in RE_WTEXT.finditer(ct):
            w, rhs = m.group(1), m.group(2)
            toks = [t for t in re.findall(r'[A-Za-z_][\w\[\]]*', rhs)
                    if t not in SKIP_TOK and len(t) >= 4]
            if toks:
                w2m.setdefault(w, toks)
        for wid, toks in w2m.items():
            if wid not in ids or wid in row['fields']:
                continue
            for t in sorted(set(toks), key=lambda x: (-len(x), x)):
                if t in mem2key:
                    sec, key = mem2key[t]
                    row['fields'][wid] = dict(section=sec, key=key, via='2hop',
                                              ctype='', html=ids[wid],
                                              sysfile=None)   # Steven 20260916
                    break

        # --- 小鍵盤 ---
        sqk = sqk_by_handler(ct)
        for wid, hs in handlers.items():
            if wid not in ids: continue
            for ev, hn in hs.items():
                if hn in sqk:
                    fl, dp_, cr, mn, mx = sqk[hn]
                    row['keyboards'][wid] = dict(event=ev.lower(), flags=fl, dp=dp_,
                                                 checkRange=(cr == 'true'), min=mn, max=mx,
                                                 handler=hn)
                    break

        # --- 涵蓋率 ---
        for wid, h in row['fields'].items():
            probe = (h['section'].lower(), h['key'].lower())
            docs = collections.Counter(ks[probe] for ks in per.values() if probe in ks)
            doc = docs.most_common(1)[0][0] if docs else None
            n = sum(docs.values()); denom = filecount.get(doc, len(per)) if doc else len(per)
            h.update(doc=doc, inrecipes=n, denom=denom, safe=(doc is not None and n == denom))
        rows.append(row)
    out = os.environ.get('V2_OUT', 'v2_out.json')
    with open(out, 'w', encoding='utf-8', newline='') as fh:
        json.dump(dict(rows=rows), fh, ensure_ascii=False, indent=1)
    sys.stderr.write('written: ' + out + chr(10))

main()
