# -*- coding: utf-8 -*-
"""dfm → HTML 產生器 v2（絕對座標版）
- 依 dfm Left/Top/Width/Height 絕對定位還原畫面
- TALed/TMyLed/TMyLedLane/TBtnPanel/TBtnPanelLane/TTMyTray 使用 hwidgets.js 模板
- TImage 底圖依 IMG_MAP（cpp 執行期 LoadFromFile 邏輯）轉 PNG
- 自動重寫 IDE.ComponentMap.html 的 offset/speed/io 區段
"""
import re, html, os, json, hashlib

BASE = r'D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2'
OUT  = r'D:\HT9045\page'

# Steven 20260919
# ----------------------------------------------------------------------
# 1. 新增 JOB_BASE：單一 job 可指定別棵 golden 樹。
# 2. 新增 3 個 job：Setup.ContactForce / HW.VacuumUnit / Setup.AGV。
# 3. OUT 可用環境變數 HT9045_DFM_OUT 覆寫（只想取幾頁時用，不必動整棵 page\）。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260919_Steven.md
# ----------------------------------------------------------------------
OUT = os.environ.get('HT9045_DFM_OUT') or OUT

# 個別 job 的來源樹覆寫。預設全部走 BASE(V910)，這裡只列需要走別棵的。
#
# ⚠ AGV.dfm 一定要走 V912：V910 那份的 E84 燈號還是 IDE 自動命名（Label33、
#   Label128…），V912 改成了 lblE84_1_In0 / lblE84_1_Out4 這種帶語意的名字。
#   原則 1 要求「元件名稱必須與 .dfm 完全一致」，用 V910 轉出來的 id 會是一批
#   Label 數字，之後接線對不上，也沒辦法從 id 看出是哪一支 E84 訊號。
#   兩棵樹的 ContactForce.dfm / VacuumUnit.dfm 是 byte-identical，不需要覆寫。
V912 = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
JOB_BASE = {
    r'Automation\AGV.dfm': V912,
}
BMP  = r'D:\HT9045\IMG\BMP'
GRAPHIC = r'D:\HT9045\IMG\Graphic'

# TImage 底圖對照（依 cpp 執行期 LoadFromFile；HT9011UC：AUTO_EMPTY_COLOR>=3 / InArm D 型吸嘴座）
IMG_MAP = {
    ('cOffSet.dfm', 'Image1'): os.path.join(BMP, 'InOutArmOffset_6.bmp'),
    ('cOffSet.dfm', 'Image3'): os.path.join(BMP, 'InOutArmOffset_6.bmp'),
    ('cOffSet.dfm', 'Image2'): os.path.join(BMP, 'SuckBaseD.bmp'),
    ('iosetview.dfm', 'imgOther'): os.path.join(BMP, 'IP Setting.bmp'),
    # Cleaning：Image1/Image3 皆載 HoatPlate1（FormShow）
    (r'AutoClean\uCleaning.dfm', 'Image1'): os.path.join(BMP, 'HoatPlate1.bmp'),
    (r'AutoClean\uCleaning.dfm', 'Image3'): os.path.join(BMP, 'HoatPlate1.bmp'),
    # Contact：imgIndex=Contact / imgSLK=Contact2
    ('cContact.dfm', 'imgIndex'): os.path.join(BMP, 'Contact.bmp'),
    ('cContact.dfm', 'imgSLK'):   os.path.join(BMP, 'Contact2.bmp'),
    # Loader/Unloader Condition 示意圖
    ('cLd_ULd.dfm', 'Image1'): os.path.join(BMP, 'LoaderCondition.bmp'),
    ('cLd_ULd.dfm', 'Image2'): os.path.join(BMP, 'UnloaderCondition.bmp'),
    # Tray Form：三分頁各一張 TrayForm2 / TrayForm3；Device/Tray 方向圖
    ('cTrayForm.dfm', 'Image1'): os.path.join(BMP, 'TrayForm2.bmp'),
    ('cTrayForm.dfm', 'Image2'): os.path.join(BMP, 'TrayForm2.bmp'),
    ('cTrayForm.dfm', 'Image3'): os.path.join(BMP, 'TrayForm2.bmp'),
    ('cTrayForm.dfm', 'Image4'): os.path.join(BMP, 'TrayForm3.bmp'),
    ('cTrayForm.dfm', 'Image5'): os.path.join(BMP, 'TrayForm3.bmp'),
    ('cTrayForm.dfm', 'Image6'): os.path.join(BMP, 'TrayForm3.bmp'),
    ('cTrayForm.dfm', 'imgDevice0'):   os.path.join(BMP, 'Device0.bmp'),
    ('cTrayForm.dfm', 'imgDevice90'):  os.path.join(BMP, 'Device90.bmp'),
    ('cTrayForm.dfm', 'imgDevice180'): os.path.join(BMP, 'Device180.bmp'),
    ('cTrayForm.dfm', 'imgDevice270'): os.path.join(BMP, 'Device270.bmp'),
    ('cTrayForm.dfm', 'imgTray0'):        os.path.join(BMP, 'Tray0.bmp'),
    ('cTrayForm.dfm', 'imgTray180'):      os.path.join(BMP, 'Tray180.bmp'),
    ('cTrayForm.dfm', 'imgTray0_Flip'):   os.path.join(BMP, 'Tray0_Flip.bmp'),
    ('cTrayForm.dfm', 'imgTray180_Flip'): os.path.join(BMP, 'Tray180_Flip.bmp'),
    # Hot Plate：小示意 HoatPlate2 ×2、大示意 HoatPlate1
    ('cHotPlate.dfm', 'Image1'): os.path.join(BMP, 'HoatPlate2.bmp'),
    ('cHotPlate.dfm', 'Image2'): os.path.join(BMP, 'HoatPlate2.bmp'),
    ('cHotPlate.dfm', 'Image3'): os.path.join(BMP, 'HoatPlate1.bmp'),
    # Set Up：機種相依（實機 8-Site → 8siteCenterX）
    ('cSetUp.dfm', 'Image1'): os.path.join(BMP, '8siteCenterX.bmp'),
    # Temperature Setting：tmode{iPoint}.bmp（預設單點 → tmode1）
    ('uTemp_Set.dfm', 'Image1'): os.path.join(BMP, 'tmode1.bmp'),
    # QA Mode / Config I37：type{方向}.bmp（預設方向 0）
    ('QAMode.dfm', 'Image1'): os.path.join(BMP, 'type0.bmp'),
    ('cConfiguration.dfm', 'imgI37_3'): os.path.join(BMP, 'type0.bmp'),
    # Tray Assignment：13 個托盤方向圖＋2 個測試方向圖，cpp 皆載 type0.bmp（預設方向）
    **{('cTrayAssignment.dfm', nm): os.path.join(BMP, 'type0.bmp') for nm in (
        'imgLoader', 'imgAuto1', 'imgAuto2', 'imgAuto3', 'imgAuto4', 'imgAuto5', 'imgAuto6',
        'imgFix1', 'imgFix2', 'imgFix3', 'imgFix4', 'imgFix5', 'imgFix6', 'ImgNormalTest', 'ImgReTest')},
    # Teach（uteach.dfm）：cpp FormShow 依 GrapicPath+*.BMP LoadFromFile（非-16Picker 預設機種）
    ('uteach.dfm', 'Image8'): os.path.join(GRAPHIC, 'Index.bmp'),      # tsIndex
    ('uteach.dfm', 'Image2'): os.path.join(GRAPHIC, '40pitch.bmp'),    # tsXPitch40
    ('uteach.dfm', 'Image4'): os.path.join(GRAPHIC, '120pitch.bmp'),   # tsXPitch120
    ('uteach.dfm', 'Image3'): os.path.join(GRAPHIC, 'pitchY25.bmp'),   # tsYPitch15
    ('uteach.dfm', 'Image6'): os.path.join(GRAPHIC, 'pitchY60.bmp'),   # tsYPitch60
    ('uteach.dfm', 'Image1'): os.path.join(GRAPHIC, 'Shuttle.bmp'),    # tsShuttlePos
    ('uteach.dfm', 'Image5'): os.path.join(GRAPHIC, 'Kit.bmp'),        # tsKit
    ('uteach.dfm', 'Image7'): os.path.join(GRAPHIC, 'Tray.bmp'),       # tsTrayArm
    # Image10 (tsHinge) 為 dfm 內嵌 Picture.Data，由 embed_png 自動解圖
}

# 僅 HTML 使用的語意 ID：保留 BCB6 DFM/C++ 原名稱，避免影響 VCL 參考。
HTML_ID_OVERRIDES = {
    ('cConfiguration.dfm', 'Panel2'): 'pnlTempCommClient',
    ('cConfiguration.dfm', 'GroupBox2'): 'gbA01_Competence',
    ('cConfiguration.dfm', 'GroupBox4'): 'gbC20_DewPoint',
    **{('cConfiguration.dfm', 'GroupBox' + str(source)): 'gbE31_Hot_' + target for source, target in {
        8: 'Auto1', 9: 'Auto2', 10: 'Auto3', 11: 'Fix3', 12: 'Fix2', 13: 'Fix1',
        14: 'Auto4', 15: 'Auto5', 18: 'Auto6', 19: 'Fix4', 20: 'Fix5', 21: 'Fix6'
    }.items()},
    **{('cConfiguration.dfm', 'GroupBox' + str(source)): 'gbE31_Cold_' + target for source, target in {
        29: 'Auto1', 30: 'Auto2', 31: 'Auto3', 32: 'Fix3', 33: 'Fix2', 34: 'Fix1',
        35: 'Auto4', 36: 'Auto5', 37: 'Auto6', 38: 'Fix4', 39: 'Fix5', 40: 'Fix6'
    }.items()},
    **{('cConfiguration.dfm', 'GroupBox' + str(source)): 'gbE32_Hot_' + target for source, target in {
        22: 'InSh1', 23: 'InSh2', 24: 'OutSh1', 25: 'OutSh2'
    }.items()},
    **{('cConfiguration.dfm', 'GroupBox' + str(source)): 'gbE32_Cold_' + target for source, target in {
        41: 'InSh1', 42: 'InSh2', 43: 'OutSh1', 44: 'OutSh2'
    }.items()},
    ('cConfiguration.dfm', 'Panel3'): 'pnlN14_20',
    ('cConfiguration.dfm', 'Panel4'): 'pnlN14_21',
    ('cConfiguration.dfm', 'Panel5'): 'pnlN14_23',
    ('cConfiguration.dfm', 'Panel6'): 'pnlN14_24',
    ('cConfiguration.dfm', 'GroupBox5'): 'gbN22_3_LogUpload',
    ('cConfiguration.dfm', 'Panel7'): 'pnlN22_3_LogUpload',
    ('cConfiguration.dfm', 'GroupBox1'): 'gbN25_2_TempLogFtp',
    ('cConfiguration.dfm', 'GroupBox3'): 'gbN35_1_LogUpload',
    ('cConfiguration.dfm', 'Panel1'): 'pnlN35_1_LogUpload',
    ('cConfiguration.dfm', 'GroupBox6'): 'gbN40_HandlerFtpBackup',
    ('cConfiguration.dfm', 'GroupBox7'): 'gbN41_HandlerDataBackup',
}

def html_id(dfm, source_name):
    return HTML_ID_OVERRIDES.get((dfm, source_name), source_name)

# ---------- DFM 解析 ----------
def decode_dfm_str(s):
    out, i = [], 0
    while i < len(s):
        c = s[i]
        if c == "'":
            j = i + 1; buf = []
            while j < len(s):
                if s[j] == "'":
                    if j + 1 < len(s) and s[j+1] == "'":
                        buf.append("'"); j += 2; continue
                    break
                buf.append(s[j]); j += 1
            out.append(''.join(buf)); i = j + 1
        elif c == '#':
            m = re.match(r'#(\d+)', s[i:])
            out.append(chr(int(m.group(1)))); i += len(m.group(0))
        else:
            i += 1
    return ''.join(out)

class Node:
    def __init__(self, name, typ):
        self.name, self.typ = name, typ
        self.props, self.kids = {}, []
    def s(self, k, d=''):
        v = self.props.get(k)
        if v is None: return d
        if v and (v[0] == "'" or v[0] == '#'): return decode_dfm_str(v)
        return v
    def i(self, k, d=0):
        try: return int(self.props.get(k, d))
        except (TypeError, ValueError): return d

def parse_dfm(path):
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        return parse_dfm_lines(f.readlines())

def parse_dfm_block(path, objname):
    """從大型 dfm 切出單一 object 區塊再解析（避開整檔解析時 stack 提前清空導致 root 被覆寫）"""
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        lines = f.readlines()
    pat = re.compile(r'^(\s*)(?:object|inherited)\s+' + re.escape(objname) + r'\s*:')
    for i, ln in enumerate(lines):
        m = pat.match(ln)
        if not m:
            continue
        ind = len(m.group(1))
        j = i + 1
        while j < len(lines):
            t = lines[j]
            if t.strip() == 'end' and len(t) - len(t.lstrip()) == ind:
                break
            j += 1
        return parse_dfm_lines(lines[i:j + 1])
    return None

def parse_dfm_lines(lines):
    root = None; stack = []
    i = 0
    while i < len(lines):
        s = lines[i].strip()
        m = re.match(r'(?:object|inherited)\s+(\w+)\s*:\s*(\w+)', s)
        if m:
            n = Node(m.group(1), m.group(2))
            if stack: stack[-1].kids.append(n)
            else: root = n
            stack.append(n)
        elif s in ('end', 'end>'):
            if stack: stack.pop()
        elif '=' in s and stack:
            k, v = s.split('=', 1)
            k, v = k.strip(), v.strip()
            if v == '{':                       # 二進位資料（Picture.Data / Glyph.Data）
                hexbuf = [] if k in ('Picture.Data', 'Glyph.Data') else None
                while i < len(lines) and not lines[i].strip().endswith('}'):
                    i += 1
                    if hexbuf is not None: hexbuf.append(lines[i].strip().rstrip('}'))
                if hexbuf is not None:
                    stack[-1].props[k] = ''.join(hexbuf)
            elif v == '(' or k == 'Items.Strings':
                items = []
                i += 1                         # 項目從下一行開始，不可把本行解成空字串
                while i < len(lines):
                    t = lines[i].strip()
                    if t.endswith(')'):
                        t = t[:-1].strip()
                        if t: items.append(decode_dfm_str(t))
                        break
                    if t and t != '(':
                        items.append(decode_dfm_str(t))
                    i += 1
                stack[-1].props[k.replace('Items.Strings', 'Items')] = items
            else:
                if v == '' and i + 1 < len(lines) and lines[i + 1].strip().startswith("'"):
                    i += 1; v = lines[i].strip()               # Caption = ↵ '...'（值從下一行開始）
                while v.endswith('+') and i + 1 < len(lines):   # 長字串續行
                    i += 1; nxt = lines[i].strip(); v = v[:-1].strip()
                    if v.endswith("'") and nxt.startswith("'"):
                        v = v[:-1] + nxt[1:]                    # 合併相鄰字串段，去掉邊界引號
                    else:
                        v = v + nxt
                stack[-1].props[k] = v
        i += 1
    return root

# ---------- 顏色 / 字型 ----------
CLR = {'clBtnFace':'#ece9d8','clWindow':'#ffffff','clWhite':'#ffffff','clBlack':'#000000',
 'clRed':'#ff0000','clLime':'#00ff00','clGreen':'#008000','clBlue':'#0000ff','clYellow':'#ffff00',
 'clSilver':'#c0c0c0','clGray':'#808080','clGrayText':'#808080','clMaroon':'#800000','clNavy':'#000080',
 'clOlive':'#808000','clPurple':'#800080','clTeal':'#008080','clAqua':'#00ffff','clFuchsia':'#ff00ff',
 'clSkyBlue':'#87ceeb','clMoneyGreen':'#c0dcc0','clCream':'#fffbf0','clMedGray':'#a0a0a4',
 'clWindowText':'#000000','clBtnText':'#000000','clInfoBk':'#ffffe1','clHighlight':'#316ac5',
 'clMenu':'#ece9d8','clInactiveCaption':'#7a96df','clActiveCaption':'#0054e3','clBtnShadow':'#808080',
 'clBtnHighlight':'#ffffff','cl3DDkShadow':'#404040','clInfoText':'#000000','clHotLight':'#0066cc',
 'clScrollBar':'#d4d0c8','clAppWorkSpace':'#808080','clDefault':''}
def color(v, d=''):
    if not v: return d
    v = v.strip()
    # dfm 全系統表單主題色（#A6B7C2）→ 統一成 CSS 主題 token，免得分頁底色不一
    if v == '12761254' or v == 'clBtnFace': return 'var(--form-bg,#ece9d8)'
    if v in CLR: return CLR[v] or d
    try:
        n = int(v)
        if n < 0: return d
        return '#%02x%02x%02x' % (n & 0xFF, (n >> 8) & 0xFF, (n >> 16) & 0xFF)
    except ValueError:
        return d

# TPanel/TScrollBox 底色主題化映射（僅容器底色；字色/LED/TShape 不走這裡）：
# dfm 各頁硬編碼的磚紅/紫/teal 標題 panel 統一成 --dfm-hdr、大面積天藍底統一成 --form-bg、
# 淡黃提示欄→--note-bg、綠色狀態→--green；IoSetView 專屬色（#3E3A39/#DCDDDD/#FFF33B/#9FA0A0）
# 為已驗收的刻意設計不映射；clGray 為值顯示格（中性色）保留。
PANEL_BG_MAP = {
    # 強調標題 panel（白字深底）
    '9534289':  'var(--dfm-hdr,#517b91)',   # #517B91 主流（9 頁）
    '6055843':  'var(--dfm-hdr,#517b91)',   # #A3675C cObserver 磚紅
    '13606558': 'var(--dfm-hdr,#517b91)',   # #9E9ECF SCK_ART 紫
    'clTeal':   'var(--dfm-hdr,#517b91)',
    '7163209':  'var(--dfm-hdr,#517b91)',   # #494D6D cStartCondition
    # 大面積淡藍/灰藍底 → 表單底
    'clSkyBlue': 'var(--form-bg,#ece9d8)',
    '14072713': 'var(--form-bg,#ece9d8)',   # #89BBD6 cObserver/cStartCondition
    '14464888': 'var(--form-bg,#ece9d8)',   # #78B7DC uTemp_Set
    '13487501': 'var(--form-bg,#ece9d8)',   # #8DCDCD cObserver
    '11444360': 'var(--form-bg,#ece9d8)',   # #88A0AE MyCCLinkSensor
    '13550775': 'var(--form-bg,#ece9d8)',   # #B7C4CE cSetUp
    '14670284': 'var(--form-bg,#ece9d8)',   # #CCD9DF uCleaning 等
    # 淡黃提示欄
    '13761532': 'var(--note-bg,#fcfbd1)',   # #FCFBD1 cObserver 日期欄
    'clInfoBk': 'var(--note-bg,#fcfbd1)',
    'clMoneyGreen': 'var(--note-bg,#fcfbd1)',
    # 綠色狀態塊
    '5812224': 'var(--green,#00d000)',      # #00B058 SCK_ART
    'clLime':  'var(--green,#00d000)',
    # 白 panel → 輸入底（dark 主題才協調）
    'clWhite': 'var(--input-bg,#fff)',
}
def panel_bg(v, d=''):
    if v and v.strip() in PANEL_BG_MAP: return PANEL_BG_MAP[v.strip()]
    return color(v, d)
def fontpx(n):
    h = n.i('Font.Height', 0)
    return abs(h) - 2 if h else 11   # VCL Font.Height(-13)≈11px 顯示
def panel_font_css(n, font_color):
    name = n.s('Font.Name', '').strip()
    style = n.s('Font.Style', '')
    css = [f'color:{font_color}', f'font-size:{fontpx(n)}px']
    if name:
        css.append(f'font-family:"{esc(name)}",sans-serif')
    css.append('font-weight:700' if 'fsBold' in style else 'font-weight:400')
    if 'fsItalic' in style: css.append('font-style:italic')
    if 'fsUnderline' in style: css.append('text-decoration:underline')
    return ';'.join(css) + ';'

# ---------- HTML 生成 ----------
def esc(s): return html.escape(str(s), quote=True)
def cap(n, default=''):
    c = n.s('Caption', n.s('EditLabel.Caption', default))
    return c.replace('&&', '\x01').replace('&', '').replace('\x01', '&')

SKIP_TYPES = {'TTimer','TOpenDialog','TSaveDialog','TSavePictureDialog','TPopupMenu','TImageList','TUpDown','TXPManifest','TComm','TClientSocket','TServerSocket','TMediaPlayer'}
# VCL 子元件座標原點就是控制項左上角（GroupBox 不另加 caption 偏移）

# ---------- LED + Label 整合（模擬 TLabeledALed / TMyLabeledLed / TMyLabeledLedLane） ----------
LED_TYPES = ('TALed', 'TMyLed', 'TMyLedLane')
LED_VTYPE = {'TALed': 'TLabeledALed', 'TMyLed': 'TMyLabeledLed', 'TMyLedLane': 'TMyLabeledLedLane'}
LABELED_LED_JOBS = {'iosetview.dfm', r'EJ1N\OmronEJ1N.dfm'}   # LED+Label 整合（Omron grpStatus Module 1~8）
LED_PAIR = {}     # id(led_node) -> (label_node, side)
LABEL_SKIP = set()  # id(label_node)：已被整合的 Label 不再單獨輸出

# 容器別方向規則（依實機語意，2026-08-27 使用者回饋）：(容器名, 允許方向, 左右最大間隙)
# 由上而下先匹配先贏；方向=None 用預設演算法；方向=空集合 不整合（TTL 保持分拆）
LED_SIDE_RULES = [
    ('gbSocketSensor', {'lpTop'}, 14),    # Index：Socket Sensor 文字在上方
    ('grpWinWayGroup', {'lpRight'}, 14),  # ATC：WinWay 1~4 文字在右側
    ('grpStatus',      {'lpRight'}, 14),  # Omron：Module 1~8 文字在右側（直排會誤抱下列 Label）
    ('tsSystem', {'lpRight'}, 14),
    ('tsIndex',  {'lpRight'}, 14),
    ('tsAOI',    {'lpRight'}, 14),
    ('tsAGV',    {'lpLeft'},  65),        # E84：文字在燈號左側（GO label 間隙達 60）
    ('tsTTL',    set(),       0),         # TTL 不使用 LabeledLed
]

def pair_led_labels(root):
    """同容器內尋找緊鄰 LED 的 TLabel，距離最近優先一對一配對；
    LED_SIDE_RULES 依頁籤/群組限制方向，避免 LED 直向密排時錯抱鄰列 Label（Ion Fan 案例）"""
    def rule_of(anc):
        for name, sides, gap in LED_SIDE_RULES:
            if name in anc: return sides, gap
        return None, 14
    def walk(n, anc):
        anc = anc + [n.name]
        sides, maxgap = rule_of(anc)
        leds = [k for k in n.kids if k.typ in LED_TYPES and k.props.get('Visible') != 'False']
        lbls = [k for k in n.kids if k.typ == 'TLabel' and k.props.get('Visible') != 'False'
                and cap(k) and not k.kids]
        cands = []
        if sides is None or sides:
            for led in leds:
                L1, T1, W1, H1 = led.i('Left'), led.i('Top'), led.i('Width'), led.i('Height')
                for lb in lbls:
                    L2, T2, W2, H2 = lb.i('Left'), lb.i('Top'), lb.i('Width'), lb.i('Height')
                    vov = not (T2 + H2 < T1 - 2 or T2 > T1 + H1 + 2)      # 垂直重疊（左右配對用）
                    hov = not (L2 + W2 < L1 - 10 or L2 > L1 + W1 + 10)    # 水平重疊（上下配對用）
                    gr, gl = L2 - (L1 + W1), L1 - (L2 + W2)
                    gt, gb = T1 - (T2 + H2), T2 - (T1 + H1)
                    side = dist = None
                    if vov and -2 <= gr <= maxgap: side, dist = 'lpRight', max(gr, 0)
                    elif vov and -2 <= gl <= maxgap: side, dist = 'lpLeft', max(gl, 0)
                    elif hov and -4 <= gt <= 10: side, dist = 'lpTop', max(gt, 0) + .5
                    elif hov and -4 <= gb <= 10: side, dist = 'lpBottom', max(gb, 0) + .5
                    if side and (sides is None or side in sides):
                        cands.append((dist, led, lb, side))
        cands.sort(key=lambda c: c[0])
        used = set()
        for dist, led, lb, side in cands:
            if id(led) in used or id(lb) in used: continue
            used.add(id(led)); used.add(id(lb))
            LED_PAIR[id(led)] = (lb, side); LABEL_SKIP.add(id(lb))
        for k in n.kids: walk(k, anc)
    walk(root, [])

# 容器內 LED 縮小置中：dfm 行距 26px、LED 24px 視覺過緊（使用者 2026-08-27 回饋）→ 縮至 N px 留行距
LED_RESIZE = {'pnlSystemPower': 18}
def resize_leds(root):
    def walk(n):
        sz = LED_RESIZE.get(n.name)
        if sz:
            for k in n.kids:
                if k.typ in LED_TYPES:
                    d = (k.i('Width') - sz) // 2
                    k.props['Left'] = str(k.i('Left') + d)
                    k.props['Top'] = str(k.i('Top') + d)
                    k.props['Width'] = str(sz)
                    k.props['Height'] = str(sz)
        for k in n.kids: walk(k)
    walk(root)

# 列距重排：gbSocketSensor 列距 29px < 內容 31px（Label19+LED14 重疊 2px）→ 每列加距、群組增高
# {容器名: (基準Top, 原列距, 每列加距, 群組增高)}
ROW_REPITCH = {'gbSocketSensor': (15, 29, 4, 16)}
def repitch_rows(root):
    def walk(n):
        r = ROW_REPITCH.get(n.name)
        if r:
            base, pitch, add, grow = r
            for k in n.kids:
                row = (k.i('Top') - base) // pitch
                if row > 0: k.props['Top'] = str(k.i('Top') + row * add)
            n.props['Height'] = str(n.i('Height') + grow)
        for k in n.kids: walk(k)
    walk(root)

# 直行群組盒重排：頁籤直欄 Panel 內群組盒 Top 相貼（0 間隙）→ 依 Top 排序後加垂直間隙、欄增高
# {頁籤名: 盒間隙px}；只處理頁籤「直接子 Panel」的第一層堆疊，巢狀 Panel 內部不動
RESPACE = {'tsIndex': 6}
def respace_stacks(root):
    def walk(n, parent):
        if n.typ == 'TPanel' and parent in RESPACE:
            gap = RESPACE[parent]
            stack = sorted([k for k in n.kids if k.typ in ('TGroupBox', 'TPanel')], key=lambda k: k.i('Top'))
            if len(stack) >= 2:
                bottom = stack[0].i('Top') + stack[0].i('Height')
                for k in stack[1:]:
                    nt = bottom + gap
                    k.props['Top'] = str(nt)
                    bottom = nt + k.i('Height')
                n.props['Height'] = str(max(n.i('Height'), bottom + gap))
        for k in n.kids: walk(k, n.name)
    walk(root, '')

# 頁籤抽出：把容器內指定元件抽成同 PageControl 的新 TTabSheet（插在 anchor 頁籤之後）
# {dfm: [(來源TabSheet名, 元件名, 新TabSheet名, 新Caption, anchor頁籤名)]}；元件重定位到新頁 (0,0)
# grpManual：Above Conveyor 底部 alBottom 413px，使頁面過高 → 獨立頁籤放 Cassette 旁（2026-08-27 使用者要求）
TAB_EXTRACT = {
    'iosetview.dfm': [('tsStack1_Above', 'grpManual', 'tsStack1_Manual', 'Manual Track', 'tsStack1_Cassette')],
}
def extract_tabs(root, jobs):
    def find(n, name):
        if n.name == name: return n
        for k in n.kids:
            r = find(k, name)
            if r: return r
    def parent_of(n, child):
        for k in n.kids:
            if k is child: return n
            r = parent_of(k, child)
            if r: return r
    for src_name, comp, ts_name, ts_cap, anchor_name in jobs:
        src = find(root, src_name)
        node = find(src, comp)
        parent_of(src, node).kids.remove(node)
        node.props.pop('Align', None)
        node.props['Left'] = '0'; node.props['Top'] = '0'
        ts = Node(ts_name, 'TTabSheet')
        ts.props['Caption'] = f"'{ts_cap}'"
        ts.kids.append(node)
        pc = parent_of(root, src)
        anchor = find(pc, anchor_name) or src
        pc.kids.insert(pc.kids.index(anchor) + 1, ts)

def gbx_inset(n):
    """GroupBox 外框內縮量（--gbi）：VCL 邊框畫在元件邊界上，固定內縮會在
    子元件貼邊的盒子穿越元件 → 依每盒子元件淨空計算：有空間才縮、貼邊就不縮"""
    W, H = n.i('Width'), n.i('Height')
    minL = minT = clr_r = clr_b = 9999
    for k in n.kids:
        if k.typ in SKIP_TYPES: continue
        if k.name in HIDE_ALWAYS or (k.props.get('Visible') == 'False' and not SHOW_HIDDEN): continue
        L, T = POS_OVERRIDE.get(k.name, (k.i('Left'), k.i('Top')))
        minL, minT = min(minL, L), min(minT, T)
        if k.props.get('Align') == 'alClient':
            clr_r = clr_b = 0
            continue
        clr_r = min(clr_r, W - (L + k.i('Width')))
        clr_b = min(clr_b, H - (T + k.i('Height')))
    if clr_b == 9999: return '2px 3px 3px 2px'   # 無子元件：維持預設
    t = max(1, min(2, minT - 1)); l = max(1, min(2, minL - 1))
    r = max(1, min(3, clr_r - 1)); b = max(1, min(3, clr_b - 1))
    return f'{t}px {r}px {b}px {l}px'

def pos(n, extra=''):
    """絕對定位 style（含 Align 處理）"""
    al = n.props.get('Align', '')
    L, T = n.i('Left'), n.i('Top')
    W, H = n.i('Width'), n.i('Height')
    if n.name in POS_OVERRIDE:
        L, T = POS_OVERRIDE[n.name]
    # alClient：保留設計期原點（已含 alLeft/alTop 兄弟佔位），右/下貼齊父容器伸縮
    # → 跟著 pcPane 內縮縮放才不會溢出出現捲軸；
    # 但需預留 alRight/alBottom 兄弟的空間（acr/acb 由 mark_alclient 預算），
    # 否則 dfm 中排在後面的 alClient 面板會蓋住它們（DIO Panel1 案例）
    if al == 'alClient':
        R, B = getattr(n, 'acr', 0), getattr(n, 'acb', 0)
        st = f'position:absolute;left:{L}px;top:{T}px;right:{R}px;bottom:{B}px;'
    else:
        st = f'position:absolute;left:{L}px;top:{T}px;width:{W}px;height:{H}px;'
    if n.name in HIDE_ALWAYS or (n.props.get('Visible') == 'False' and not SHOW_HIDDEN):
        st += 'display:none;'
    if n.props.get('Enabled') == 'False': st += 'opacity:.45;'
    return st + extra

def mark_alclient(n):
    """預算 alClient 節點需預留給 alRight/alBottom 兄弟的右/下空間"""
    def vis(k):
        return k.name not in HIDE_ALWAYS and not (k.props.get('Visible') == 'False' and not SHOW_HIDDEN)
    r = sum(k.i('Width') for k in n.kids if k.props.get('Align') == 'alRight' and vis(k))
    b = sum(k.i('Height') for k in n.kids if k.props.get('Align') == 'alBottom' and vis(k))
    for k in n.kids:
        if k.props.get('Align') == 'alClient':
            k.acr, k.acb = r, b
        mark_alclient(k)

def led_html(n, ttl, at=None):
    style = n.s('LEDStyle', 'LEDSmall')
    on = ' on' if n.props.get('Value') == 'True' else ''
    tc = color(n.props.get('TrueColor'), '#00ff00')
    fc = color(n.props.get('FalseColor'), '#c0c0c0')
    extra = f'--led-on:{tc};--led-off:{fc};border-radius:{"50%" if "Sq" not in style and "Vert" not in style and "Horiz" not in style else "2px"};'
    if at is not None:  # 整合元件內：使用包裝層相對座標
        st = f'position:absolute;left:{at[0]}px;top:{at[1]}px;width:{n.i("Width")}px;height:{n.i("Height")}px;' + extra
    else:
        st = pos(n, extra)
    return f'<span class="aled {esc(style)}{on}" id="{esc(n.name)}" style="{st}" {ttl}></span>'

def btnpanel_html(n, ttl):
    tc = color(n.props.get('TrueColor'), '#00c000')
    fc = color(n.props.get('FalseColor'), '#ece9d8')
    tf = color(n.props.get('TrueFontColor'), '#ffffff')
    ff = color(n.props.get('FalseFontColor'), '#000000')
    cls = 'btnpanel'
    if n.props.get('Style') == 'tsFlatButtons': cls += ' flat'
    if n.props.get('Down') == 'True': cls += ' down'
    extra = (f'--bp-true:{tc};--bp-false:{fc};--bp-true-font:{tf};--bp-false-font:{ff};'
             f'min-width:0;min-height:0;margin:0;padding:0;font-size:{fontpx(n)}px;')
    return f'<div class="{cls}" id="{esc(n.name)}" style="{pos(n, extra)}" {ttl}>{esc(cap(n))}</div>'

def tray_html(n, ttl, dfm):
    spec = {'name': n.name, 'xitem': n.i('XItem', 2), 'yitem': n.i('YItem', 2),
            'xblockItem': n.i('XBlockItem', 0), 'yblockItem': n.i('YBlockItem', 0),
            'w': n.i('Width'), 'h': n.i('Height'),
            'trayColor': color(n.props.get('Color'), '#8a9a8a'),
            'trayDirect': n.s('TrayDirect', 'csNull')}
    return (f'<div class="traypos" id="{esc(n.name)}" data-tray="{esc(json.dumps(spec))}" '
            f'style="{pos(n)}" {ttl}></div>')

GLYPH_CACHE = {}   # md5 → (png檔名, w, h)；194 顆按鈕共用少數幾種圖，依內容去重
def glyph_png(n):
    """TSpeedButton/TBitBtn Glyph.Data（4 byte 大小(LE)+BMP）→ 取第一格、左下角色轉透明、存 PNG"""
    hx = n.props.get('Glyph.Data')
    if not hx: return None
    key = hashlib.md5(hx.encode()).hexdigest()[:10]
    if key in GLYPH_CACHE: return GLYPH_CACHE[key]
    try:
        data = bytes.fromhex(hx)
        payload = data[4:4 + int.from_bytes(data[:4], 'little')]
        from PIL import Image as _Img
        import io
        im = _Img.open(io.BytesIO(payload)).convert('RGBA')
        ng = max(n.i('NumGlyphs', 1), 1)
        gw = im.width // ng
        im = im.crop((0, 0, gw, im.height))
        tc = im.getpixel((0, im.height - 1))[:3]   # VCL 規則：左下角像素色＝透明色
        px = im.load()
        for y in range(im.height):
            for x in range(im.width):
                if px[x, y][:3] == tc: px[x, y] = (0, 0, 0, 0)
        png = f'dfm_glyph_{key}.png'
        os.makedirs(os.path.join(OUT, 'img'), exist_ok=True)
        im.save(os.path.join(OUT, 'img', png))
        GLYPH_CACHE[key] = (png, im.width, im.height)
        print('glyph:', n.name, '->', png, f'{im.width}x{im.height}')
        return GLYPH_CACHE[key]
    except Exception as e:
        print('glyph FAIL:', n.name, e)
        return None

def bmp_debg(im):
    """BMP 去背：左下角像素色轉透明（同 VCL Transparent 規則）"""
    im = im.convert('RGBA')
    tc = im.getpixel((0, im.height - 1))[:3]
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            if px[x, y][:3] == tc: px[x, y] = (0, 0, 0, 0)
    return im

def embed_png(n, dfm):
    """dfm 內嵌 Picture.Data（1 byte 類別名長度 + 'TBitmap' + BMP 內容）→ 解出並轉存 PNG，回傳檔名"""
    hx = n.props.get('Picture.Data')
    if not hx: return None
    try:
        data = bytes.fromhex(hx)
        # TPicture.Data：1 byte 類別名長度 + 類別名（TBitmap/TJPEGImage）+ 4 byte 大小(LE) + 圖檔內容
        L = data[0]
        payload = data[5 + L: 5 + L + int.from_bytes(data[1 + L:5 + L], 'little')]
        if not payload:
            idx = data.find(b'BM')
            if idx < 0 or idx > 32: return None
            payload = data[idx:]
        from PIL import Image as _Img
        import io
        png = f'dfm_embed_{os.path.splitext(os.path.basename(dfm))[0]}_{n.name}.png'
        os.makedirs(os.path.join(OUT, 'img'), exist_ok=True)
        im = _Img.open(io.BytesIO(payload))
        if payload[:2] == b'BM': im = bmp_debg(im)   # JPEG 照片不去背
        im.save(os.path.join(OUT, 'img', png))
        print('img(embed):', dfm, n.name, '->', png)
        return png
    except Exception as e:
        print('img(embed) FAIL:', dfm, n.name, e)
        return None

def img_html(n, ttl, dfm, imgs_used):
    src = IMG_MAP.get((dfm, n.name))
    png = None
    if src and os.path.exists(src):
        png = 'dfm_' + os.path.splitext(os.path.basename(src))[0].replace(' ', '_') + '.png'
        imgs_used[src] = png
    else:
        png = embed_png(n, dfm)                  # 無外掛對照 → 試解 dfm 內嵌圖
    if png:
        auto = n.props.get('AutoSize') == 'True'
        wh = '' if auto else f'width:{n.i("Width")}px;height:{n.i("Height")}px;'
        return (f'<img src="img/{png}" id="{esc(n.name)}" {ttl} '
                f'style="position:absolute;left:{n.i("Left")}px;top:{n.i("Top")}px;{wh}">')
    if n.props.get('Transparent') == 'True':   # 實機為透明空 TImage（如 Config Image1 雙擊熱區）→ 不畫虛線框
        return f'<div id="{esc(n.name)}" style="{pos(n)}" {ttl}></div>'
    return f'<div class="imgph" id="{esc(n.name)}" style="{pos(n)}" {ttl}>{esc(n.name)}</div>'

# 標題為 Exit/Close 的按鈕型元件 → exitbtn class，由頁尾 JS 統一綁關視窗事件
EXIT_CAPS = {'Exit', 'EXIT', 'Close', 'CLOSE'}
SAVE_CAPS = {'Save', 'SAVE'}
STD_BTN = {}   # 'exit'/'save' → ((png,w,h), fs)；以 cOffSet sbtExit/spbSave 為全局統一標準（主流 18/18 顆）
def exit_cls(n):
    return ' exitbtn' if cap(n).strip() in EXIT_CAPS else ''

def render(n, dfm, imgs_used, bg='var(--panel,#ece9d8)'):
    t, source_name = n.typ, n.name
    name = html_id(dfm, source_name)
    if id(n) in LABEL_SKIP: return ''   # 已整合進 LabeledLed 的 Label
    ttl = f'title="{esc(name)} : {esc(t)}"'
    c = esc(cap(n))
    if t in SKIP_TYPES: return ''
    if t == 'TPageControl':
        tabs, panes = [], []
        sheets = [k for k in n.kids if k.typ == 'TTabSheet']
        actname = DEFAULT_ACTIVE.get(dfm, {}).get(source_name)
        actidx = next((i for i, ts in enumerate(sheets) if ts.name == actname), 0)
        for idx, ts in enumerate(sheets):
            act = ' act' if idx == actidx else ''
            tabs.append(f'<div class="tab{act}" data-t="{idx}" title="{esc(ts.name)} : TTabSheet">{esc(cap(ts, ts.name))}</div>')
            inner = ''.join(render(k, dfm, imgs_used, bg) for k in ts.kids)
            inner += RUNTIME_INJECT.get((dfm, ts.name), '')   # TTabSheet 執行期動態面板（如 PalYield）附加於分頁內
            panes.append(f'<div class="pcPane" data-p="{idx}" title="{esc(ts.name)}" '
                         f'style="display:{"block" if idx==actidx else "none"};">{inner}</div>')
        if (dfm, source_name) in HIDE_TABS:   # 執行期 TabVisible=false：頁籤列不顯示、內容貼頂（layoutPC 見 offsetHeight=0 不重算）
            return (f'<div class="pcWrap" id="{esc(name)}" style="{pos(n)}" title="{esc(name)} : TPageControl（TabVisible=false）">'
                    f'<div class="tabs pcTabs" style="display:none;">{"".join(tabs)}</div>'
                    f'<div class="pcBody" style="top:0;">{"".join(panes)}</div></div>')
        return (f'<div class="pcWrap" id="{esc(name)}" style="{pos(n)}" title="{esc(name)} : TPageControl">'
                f'<div class="tabs pcTabs">{"".join(tabs)}</div><div class="pcBody">{"".join(panes)}</div></div>')
    if t == 'TGroupBox':
        inner = ''.join(render(k, dfm, imgs_used, bg) for k in n.kids)
        leg = f'<legend style="background:{bg};">{c}</legend>' if c else ''
        # VCL TGroupBox：非對齊子元件原點=控制項左上角(0,0)，並裁切於控制項邊界
        return (f'<fieldset class="gbx" id="{esc(name)}" style="{pos(n, f"--gbi:{gbx_inset(n)};")}" {ttl}>{leg}'
                f'<div class="cli" style="position:absolute;inset:0;overflow:hidden;">{inner}</div></fieldset>')
    if t in ('TPanel', 'TScrollBox'):
        # 按鈕型 TPanel（無子元件、Caption 純 Exit/Close/Save、有 OnClick 當按鈕用）→ 統一 TSpeedButton 樣式
        if t == 'TPanel' and not n.kids and n.props.get('OnClick'):
            _rc = cap(n).strip()
            _uni = 'exit' if _rc in EXIT_CAPS else ('save' if _rc in SAVE_CAPS else None)
            if _uni and STD_BTN.get(_uni):
                g, fs = STD_BTN[_uni]
                gimg = f'<img src="img/{g[0]}" alt="" style="flex:none;max-height:88%;">'
                return (f'<button class="btn3d{exit_cls(n)}" id="{esc(name)}" '
                        f'title="{esc(name)} : TSpeedButton（統一 {_uni} 樣式，原 TPanel）" '
                        f'style="{pos(n)}font-size:{fs}px;">{gimg}{c}</button>')
        nbg = panel_bg(n.props.get('Color'), '')
        inj = RUNTIME_INJECT.get((dfm, source_name))   # 執行期動態建立的子元件（dfm 無）
        if inj is not None:
            skip = RUNTIME_INJECT_SKIP.get((dfm, source_name), '*')   # 預設 '*' 全略過（取代設計期殘留）；給集合則只略過該些名稱後附加
            kept = '' if skip == '*' else ''.join(render(k, dfm, imgs_used, nbg or bg) for k in n.kids if k.name not in skip)
            inner = kept + inj
        else:
            inner = ''.join(render(k, dfm, imgs_used, nbg or bg) for k in n.kids)
        extra = (f'background:{nbg};' if nbg else '') + ('overflow:auto;' if t == 'TScrollBox' else '')
        capfc = color(n.props.get('Font.Color'), '#334455')
        # 永遠保留 pnlCap 供 runtime 更新；Caption==元件名時只清空文字，不顯示設計器預設殘留。
        panel_caption = c if (c and c != esc(source_name)) else ''
        lbl = f'<span class="pnlCap" style="{panel_font_css(n, capfc)}">{panel_caption}</span>'
        bev = '' if n.props.get('BevelOuter') == 'bvNone' else 'border:1px outset #ddd;'
        return f'<div class="pnl{exit_cls(n)}" id="{esc(name)}" style="{pos(n, extra + bev)}" {ttl}>{lbl}{inner}</div>'
    if t == 'TBevel':
        return f'<div style="{pos(n)}border:1px inset #ccc;" {ttl}></div>'
    if t == 'TShape':
        bg = color(n.props.get('Brush.Color'), '#fff')
        return f'<div style="{pos(n)}background:{bg};border:1px solid #666;" id="{esc(name)}" {ttl}></div>'
    if t == 'TRadioGroup':
        items = n.props.get('Items', [])
        sel = n.i('ItemIndex', 0)
        rid = f'rg_{name}'
        cols = max(1, n.i('Columns', 1))
        rows = max(1, -(-max(1, len(items)) // cols))   # VCL 多欄排列為先直後橫（column-major）
        opts = ''.join(f'<label class="rgi"><input type="radio" name="{rid}" {"checked" if i==sel else ""}>{esc(it)}</label>'
                       for i, it in enumerate(items)) or '<span class="rgi">--</span>'
        leg = f'<legend style="background:{bg};">{c}</legend>' if c else ''
        return (f'<fieldset class="gbx rg" id="{esc(name)}" style="{pos(n)}" {ttl}>{leg}'
                f'<div class="cli" style="position:absolute;left:4px;top:14px;right:2px;bottom:2px;'
                f'display:grid;grid-template-rows:repeat({rows},1fr);grid-auto-flow:column;align-content:start;">{opts}</div></fieldset>')
    if t == 'TCheckBox':
        chk = 'checked' if n.props.get('Checked') == 'True' else ''
        g = re.search(r' {6,}', c)
        if g:  # 長空白留 Edit 位：尾段右錨03定於 dfm 右緣，免受瀏覽器字寬影響滑進 Edit 底下
            return (f'<label class="ckb" id="{esc(name)}" style="{pos(n)}" {ttl}><input type="checkbox" {chk}>{c[:g.start()]}'
                    f'<span style="position:absolute;right:0;top:50%;transform:translateY(-50%);">{c[g.end():]}</span></label>')
        return f'<label class="ckb" id="{esc(name)}" style="{pos(n)}" {ttl}><input type="checkbox" {chk}>{c}</label>'
    if t == 'TRadioButton':
        chk = 'checked' if n.props.get('Checked') == 'True' else ''
        return f'<label class="ckb" id="{esc(name)}" style="{pos(n)}" {ttl}><input type="radio" {chk}>{c}</label>'
    if t in ('TEdit', 'TLabeledEdit', 'TMaskEdit', 'TCSpinEdit', 'TSpinEdit'):
        v = esc(n.s('Text', n.s('Value', '')))
        lab = f'<span class="elab" style="position:absolute;top:-13px;left:0;">{c}</span>' if (t == 'TLabeledEdit' and c) else ''
        typ = ' type="password"' if n.props.get('PasswordChar') else ''
        fcss = f'font-size:{fontpx(n)}px;' if fontpx(n) > 12 else ''   # 大字 Edit（如 fPassword -32）才覆寫 .ed 預設 11px
        return f'<span style="{pos(n)}">{lab}<input class="ed" id="{esc(name)}"{typ} {ttl} value="{v}" style="width:100%;height:100%;box-sizing:border-box;{fcss}"></span>'
    if t == 'TComboBox':
        items = n.props.get('Items', [])
        v = n.s('Text', '')
        if items:
            sel = v if v in items else items[max(0, n.i('ItemIndex', 0)) if n.i('ItemIndex', -1) >= 0 else 0]
            opts = ''.join(f'<option{" selected" if it == sel else ""}>{esc(it)}</option>' for it in items)
        else:
            opts = f'<option>{esc(v) or esc(name)}</option>'
        return f'<select class="ed" id="{esc(name)}" {ttl} style="{pos(n)}">{opts}</select>'
    if t in ('TSpeedButton', 'TButton', 'TBitBtn'):
        fs = fontpx(n)
        rawcap = cap(n)
        g = glyph_png(n) if t in ('TSpeedButton', 'TBitBtn') else None
        # Caption 純 Exit/Close/Save 者統一為 cOffSet 標準 TSpeedButton 樣式（同 glyph、同字型）
        uni = 'exit' if rawcap.strip() in EXIT_CAPS else ('save' if rawcap.strip() in SAVE_CAPS else None)
        if uni and STD_BTN.get(uni):
            g, fs = STD_BTN[uni]
            ttl = f'title="{esc(name)} : TSpeedButton（統一 {uni} 樣式）"'
        gw = (g[1] + 3) if g else 0
        # 文字超出按鈕寬度時自動縮字（模擬 VCL 窄字型單行顯示，避免左右裁切；扣 glyph 佔寬）
        est = sum(1.0 if ord(ch) > 0x2E7F else 0.55 for ch in rawcap) * fs
        avail = max(n.i('Width') - 10 - gw, 10)
        if est > avail:
            fs = max(8, int(fs * avail / est))
        gimg = f'<img src="img/{g[0]}" alt="" style="flex:none;max-height:88%;">' if g else ''
        col = ';flex-direction:column' if n.s('Layout') == 'blGlyphTop' else ''
        return f'<button class="btn3d{exit_cls(n)}" id="{esc(name)}" {ttl} style="{pos(n)}font-size:{fs}px{col};">{gimg}{c}</button>'
    if t == 'TBtnPanel' or t == 'TBtnPanelLane':
        return btnpanel_html(n, f'title="{esc(name)} : {esc(t)}｜Alias={esc(n.s("Alias"))}"')
    if t in ('TALed', 'TMyLed', 'TMyLedLane'):
        alias = n.s('Alias')
        lttl = f'title="{esc(name)} : {esc(t)}{"｜Alias=" + esc(alias) if alias else ""}"'
        pair = LED_PAIR.get(id(n))
        if pair:  # 整合為虛擬 TLabeledALed / TMyLabeledLed / TMyLabeledLedLane
            lbl, side = pair
            L1, T1, W1, H1 = n.i('Left'), n.i('Top'), n.i('Width'), n.i('Height')
            L2, T2, W2, H2 = lbl.i('Left'), lbl.i('Top'), lbl.i('Width'), lbl.i('Height')
            mL, mT = min(L1, L2), min(T1, T2)
            w, h = max(L1 + W1, L2 + W2) - mL, max(T1 + H1, T2 + H2) - mT
            vtyp = LED_VTYPE[t]
            wttl = (f'title="{esc(name)} : {esc(vtyp)}（{esc(t)}＋{esc(lbl.name)} 整合）｜'
                    f'Caption={esc(cap(lbl))} CaptionPos={side}{"｜Alias=" + esc(alias) if alias else ""}"')
            fc_ = color(lbl.props.get('Font.Color'), 'var(--text,#222)')
            bold = 'font-weight:bold;' if 'fsBold' in lbl.props.get('Font.Style', '') else ''
            capsty = (f'position:absolute;left:{L2 - mL}px;top:{T2 - mT}px;color:{fc_};{bold}'
                      f'font-size:{fontpx(lbl)}px;white-space:nowrap;')
            return (f'<span class="lled {side}" style="position:absolute;left:{mL}px;top:{mT}px;'
                    f'width:{w}px;height:{h}px;" {wttl}>'
                    f'{led_html(n, lttl, at=(L1 - mL, T1 - mT))}'
                    f'<span class="lledCap" style="{capsty}" title="{esc(lbl.name)} : TLabel">{esc(cap(lbl))}</span></span>')
        return led_html(n, lttl)
    if t == 'TTMyTray':
        return tray_html(n, ttl, dfm)
    if t == 'TImage':
        return img_html(n, ttl, dfm, imgs_used)
    if t == 'TTrackBar':
        return (f'<input type="range" class="trk" id="{esc(name)}" {ttl} style="{pos(n)}" '
                f'min="{n.i("Min",0)}" max="{n.i("Max",100)}" value="{n.i("Position",50)}">')
    if t in ('TLabel', 'TStaticText'):
        fc = color(n.props.get('Font.Color'), '#000')
        bold = 'font-weight:bold;' if 'fsBold' in n.props.get('Font.Style', '') else ''
        g = re.search(r' {6,}', c)
        if g and n.props.get('WordWrap') != 'True':
            # 長空白留 Edit 位：AutoSize 的 dfm Width＝VCL 實測文字寬，尾段右緣對齊 dfm 右緣
            return (f'<span class="lb" id="{esc(name)}" {ttl} '
                    f'style="{pos(n)}color:{fc};{bold}white-space:nowrap;font-size:{fontpx(n)}px;">'
                    f'{c[:g.start()]}<span style="position:absolute;right:0;top:0;">{c[g.end():]}</span></span>')
        wrap = '' if n.props.get('WordWrap') == 'True' else 'white-space:nowrap;width:auto;'
        return (f'<span class="lb" id="{esc(name)}" {ttl} '
                f'style="{pos(n)}color:{fc};{bold}{wrap}font-size:{fontpx(n)}px;height:auto;">{c}</span>')
    if t == 'TStringGrid':
        return (f'<div class="sgd" id="{esc(name)}" {ttl} style="{pos(n)}overflow:auto;">'
                f'<table><tr><th colspan="4">{esc(name)}（TStringGrid，執行期填值）</th></tr>'
                f'<tr><td>--</td><td>--</td><td>--</td><td>--</td></tr></table></div>')
    if t in ('TMemo', 'TRichEdit'):
        fc = color(n.props.get('Font.Color'), '')
        fcss = f'color:{fc};font-size:{fontpx(n)}px;' if n.props.get('Font.Height') else ''
        ro = ' readonly' if n.props.get('ReadOnly') == 'True' else ''
        return f'<textarea class="ed" id="{esc(name)}" {ttl} style="{pos(n)}{fcss}resize:none;"{ro}></textarea>'
    if t == 'TCheckListBox':
        its = n.props.get('Items', [])
        rows = ''.join(f'<label class="ckb" style="position:static;"><input type="checkbox">{esc(x)}</label>'
                       for x in (its if isinstance(its, list) else [])) or '&nbsp;'
        return (f'<div class="lbx" id="{esc(name)}" {ttl} style="{pos(n)}background:var(--input-bg,#fff);'
                f'border:1px inset #ccc;overflow:auto;font-size:11px;">{rows}</div>')
    if t == 'TDirectoryListBox':
        return (f'<div class="lbx" id="{esc(name)}" {ttl} style="{pos(n)}background:var(--input-bg,#fff);'
                f'border:1px inset #ccc;overflow:auto;font-size:11px;padding:2px;">'
                f'<div>\U0001f4c2 C:\\</div><div style="padding-left:14px;">\U0001f4c1 HT9045</div></div>')
    if t == 'TDriveComboBox':
        return f'<select class="ed" id="{esc(name)}" {ttl} style="{pos(n)}"><option>\U0001f4bd c:</option></select>'
    if t == 'TScrollBar':
        horiz = n.props.get('Kind', 'sbHorizontal') != 'sbVertical'
        thumb = ('left:35%;top:1px;bottom:1px;width:30%;' if horiz else 'top:35%;left:1px;right:1px;height:30%;')
        return (f'<div id="{esc(name)}" {ttl} style="{pos(n)}background:var(--tab-bg,#d4d0c8);'
                f'border:1px inset #bbb;box-sizing:border-box;">'
                f'<div style="position:absolute;{thumb}background:var(--form-bg,#ece9d8);border:1px outset #ddd;"></div></div>')
    if t == 'TDateTimePicker':
        return f'<input class="ed" id="{esc(name)}" {ttl} value="{esc(n.s("Date", ""))}" style="{pos(n)}">'
    if t == 'TListBox':
        its = n.props.get('Items', [])
        rows = ''.join(f'<div>{esc(x)}</div>' for x in (its if isinstance(its, list) else [])) or '&nbsp;'
        return (f'<div class="lbx" id="{esc(name)}" {ttl} style="{pos(n)}background:var(--input-bg,#fff);'
                f'border:1px inset #ccc;overflow:auto;font-size:11px;">{rows}</div>')
    if t == 'TTreeView':
        return f'<div class="pnl" id="{esc(name)}" style="{pos(n)}border:1px inset #ccc;background:#fff;" {ttl}>🌲 {esc(name)}</div>'
    if n.kids:
        inner = ''.join(render(k, dfm, imgs_used, bg) for k in n.kids)
        return f'<div class="pnl" id="{esc(name)}" style="{pos(n)}" {ttl}>{inner}</div>'
    return f'<span class="lb dim" style="{pos(n)}height:auto;" {ttl}>{esc(name)}</span>'

PAGE_TMPL = '''<!DOCTYPE html>
<html lang="zh-Hant">
<head>
<meta charset="UTF-8">
<title>{title}</title>
<link rel="stylesheet" href="ht9xxx.css">
<link rel="stylesheet" href="theme.css">
<style>
  body.winpage{{font-size:11px;padding:0;margin:0;background:var(--form-bg,#ece9d8);}}
  .form{{position:relative;background:var(--form-bg,#ece9d8);font-family:var(--font-ui,"Microsoft JhengHei","MS Sans Serif",sans-serif);
    user-select:none;-webkit-user-select:none;-ms-user-select:none;}}
  /* VCL：只有編輯框內文字可選取（Label/Panel/按鈕/頁籤 caption 皆不可選） */
  .form input,.form textarea,.form select,.form .mem{{user-select:text;-webkit-user-select:text;-ms-user-select:text;}}
  .pcWrap{{box-sizing:border-box;}}
  /* 分頁列：比照 main 畫面各視窗（ht9xxx.css .tabs/.tab）的經典頁籤，各層級一致 */
  .pcTabs{{display:flex;flex-wrap:wrap;gap:2px;padding:2px 2px 0;position:relative;z-index:2;align-items:end;}}
  .pcTabs .tab{{background:var(--tab-bg,#d4d0c8);border:1px solid var(--pane-border,#888);
    border-radius:4px 4px 0 0;padding:3px 9px;font-size:11px;cursor:pointer;user-select:none;}}
  /* act 頁籤底色=內容面板，並蓋掉交接處框線 → 與面板融為一體（同 main 畫面頁籤） */
  .pcTabs .tab.act{{background:var(--form-bg,#ece9d8);border-bottom-color:var(--form-bg,#ece9d8);font-weight:bold;}}
  /* top:26px 為静態保底（隱藏 iframe 內 JS 算不到高度時仍可見）；layoutPC 顯示後再精確重算 */
  .pcBody{{position:absolute;left:2px;right:2px;top:26px;bottom:2px;border:1px solid var(--pane-border,#999);background:var(--form-bg,#ece9d8);}}
  /* VCL TabSheet 客戶區與頁籤列間有內縮 → 頁籤與 GroupBox caption 拉開距離；
     left 必須為 0：內縮 2px 會讓 alClient 子面板水平溢出 2px 出現整條捲軸（Vacuum/In Arm 案例） */
  .pcPane{{position:absolute;left:0;right:0;top:6px;bottom:0;overflow:hidden;}}
  /* GroupBox 間距：外框交給 ::before 往內縮，相鄰 GroupBox 邊框才有視覺間隔（dfm 座標不動）；
     內縮量由產生器依子元件淨空逐盒計算（--gbi），子元件貼邊時不縮→不會穿越元件 */
  .gbx{{border:none;margin:0;padding:0;box-sizing:border-box;}}
  .gbx::before{{content:'';position:absolute;inset:var(--gbi,2px 3px 3px 2px);border:1px solid var(--gbx-border,#9aa);border-radius:3px;pointer-events:none;}}
  /* legend 絕對定位：不佔內部空間，讓 .cli inset:0 真正對齊控制項左上角（同 VCL） */
  .gbx legend{{position:absolute;top:-2px;left:6px;font-size:11px;color:var(--gbx-legend,#234);padding:0 4px;line-height:12px;}}
  .pnl{{box-sizing:border-box;}}
    .pnlCap{{position:absolute;left:0;right:0;top:2px;text-align:center;white-space:pre;overflow:hidden;}}
  .rgi{{display:block;font-size:11px;white-space:nowrap;}}
  /* flex 垂直置中 + 緊湊行高：dfm Height=15 的 checkbox 文字下緣才不會被裁掉 */
  .ckb{{font-size:11px;white-space:pre;overflow:hidden;display:flex;align-items:center;line-height:13px;}}
  .ckb input{{flex:none;margin:0 4px 0 1px;width:12px;height:12px;}}
  .elab{{font-size:10px;color:#345;white-space:nowrap;}}
  .ed{{font-size:11px;border:1px inset var(--input-border,#aaa);background:var(--input-bg,#fff);color:var(--text,#222);padding:0 3px;box-sizing:border-box;}}
  .lb{{font-size:11px;color:var(--text,#222);overflow:visible;}}
  .lb.dim{{color:var(--text-dim,#889);}}
  .trk{{vertical-align:middle;}}
  .sgd table{{border-collapse:collapse;width:100%;}}
  .sgd th,.sgd td{{border:1px solid #aab;font-size:10px;padding:2px 8px;background:#fff;}}
  .sgd th{{background:#d6dbe3;}}
  .imgph{{border:1px dashed #99a;color:#889;font-size:10px;display:flex;align-items:center;justify-content:center;box-sizing:border-box;}}
  .btn3d{{overflow:hidden;white-space:nowrap;display:inline-flex;align-items:center;justify-content:center;gap:3px;padding:0 2px;}}
  /* LED+Label 整合元件（虛擬 TLabeledALed 系列）：hover 時整組一起框起來 */
  .lled{{box-sizing:border-box;}}
  .lled:hover{{outline:1px dotted var(--gbx-border,#9aa);outline-offset:1px;}}
  .lledCap{{line-height:1.3;}}
</style>
</head>
<body class="winpage">
<div class="form" style="width:{cw}px;height:{ch}px;">
{body}
</div>
<script src="theme.js"></script>
<script src="hwidgets.js"></script>
<script>
// 巢狀 PageControl：只切換同一個 pcWrap 內的分頁
// 隱藏中的 pcWrap offsetHeight=0，須於顯示時重算 pcBody 頂部（重疊 1px 讓 act 頁籤融合面板）
function layoutPC(pc){{
  var tabs=pc.querySelector(':scope > .pcTabs');
  var body=pc.querySelector(':scope > .pcBody');
  if(tabs.offsetHeight>0) body.style.top=(tabs.offsetHeight-1)+'px';
}}
document.querySelectorAll('.pcWrap').forEach(function(pc){{
  var tabs=pc.querySelector(':scope > .pcTabs');
  var body=pc.querySelector(':scope > .pcBody');
  layoutPC(pc);
  // 在隱藏 iframe（background 桌面視窗）內載入時高度為 0，顯示後由 ResizeObserver 重算
  if(window.ResizeObserver) new ResizeObserver(function(){{layoutPC(pc);}}).observe(tabs);
  tabs.querySelectorAll(':scope > .tab').forEach(function(tb){{
    tb.addEventListener('click',function(){{
      tabs.querySelectorAll(':scope > .tab').forEach(function(x){{x.classList.remove('act');}});
      tb.classList.add('act');
      body.querySelectorAll(':scope > .pcPane').forEach(function(p){{
        p.style.display=(p.dataset.p===tb.dataset.t)?'block':'none';
      }});
      pc.querySelectorAll('.pcWrap').forEach(layoutPC);  // 剛顯示的巢狀分頁列此時才有高度
    }});
  }});
}});
// Exit/Close 按鈕（.exitbtn，產生器依 Caption 標記）：通知 background.html 關閉本視窗（同標題列 ✕）
document.querySelectorAll('.exitbtn').forEach(function(b){{
  b.style.cursor='pointer';
  b.addEventListener('click',function(){{
    if(window.parent!==window) window.parent.postMessage({{closeMe:1}},'*');
  }});
}});
// TBtnPanel/TBtnPanelLane：點擊切換 Down（=SetPanelStatus）
document.querySelectorAll('.btnpanel').forEach(function(b){{
  b.addEventListener('click',function(){{b.classList.toggle('down');}});
}});
// TTMyTray：以 hwidgets 模板建格盤
document.querySelectorAll('.traypos').forEach(function(ph){{
  var s=JSON.parse(ph.dataset.tray);
  var cw=Math.max(4,Math.floor((s.w-10)/s.xitem)), chh=Math.max(4,Math.floor((s.h-10)/s.yitem));
  var t=HTWidgets.makeMyTray({{name:s.name+'_t',xitem:s.xitem,yitem:s.yitem,
    xblockItem:s.xblockItem,yblockItem:s.yblockItem,cellW:cw,cellH:chh,
    trayColor:s.trayColor,trayDirect:s.trayDirect}});
  ph.appendChild(t);
}});
</script>
</body>
</html>
'''

# ---------- 執行期動態建立元件注入 ----------
def omron_panel(i, j):
    """單一 TMyOmronPanel（MyOmronPanel.cpp 建構）：GroupBox 206×120＋ImgOmronPanel 顯示＋edSV＋cbEnableAT"""
    # SetPanelPos：iTop[4]={5,129,5,129} iLeft[4]={2,2,212,212}；unit 間 iLeft2(奇)=420 / iTop2(偶)+=248
    iTop = [5, 129, 5, 129][j]
    iLeft = [2, 2, 212, 212][j]
    iLeft2 = 420 if (i % 2) else 0
    iTop2 = (i // 2) * 248
    L, T = iLeft + iLeft2, iTop + iTop2
    cap_ = f'CH{i+1}-{j+1}'
    # ImgOmronPanel 顯示區（4,20 197×95，clGray 背景）；文字座標同 canvas Set* 方法
    disp = (
        '<div style="position:absolute;left:4px;top:20px;width:197px;height:95px;background:#808080;'
        'border:1px solid #6e6e6e;font-family:Arial;overflow:hidden;">'
        '<span style="position:absolute;left:4px;top:1px;font-size:12px;color:#000;">℃</span>'
        '<span style="position:absolute;left:4px;top:20px;font-size:12px;color:#000;">STOP</span>'
        '<span style="position:absolute;left:2px;top:39px;font-size:11px;color:#000;white-space:nowrap;">Input Error</span>'
        '<span style="position:absolute;left:4px;top:58px;font-size:12px;color:#000;">AT</span>'
        '<span style="position:absolute;left:72px;top:-3px;width:125px;text-align:center;'
        'font-size:40px;font-weight:bold;color:#ff0;">0.0</span>'
        '<span style="position:absolute;left:72px;top:46px;width:125px;text-align:center;font-size:12px;color:#000;">Event</span>'
        '</div>'
    )
    cb = '<input type="checkbox" style="position:absolute;left:48px;top:76px;width:15px;height:15px;margin:0;">'
    ed = ('<input class="ed" value="85" style="position:absolute;left:72px;top:85px;width:128px;height:28px;'
          'border:none;background:#fff;color:#000;font:22px Arial;padding:0 4px;box-sizing:border-box;">')
    return (
        f'<div style="position:absolute;left:{L}px;top:{T}px;width:206px;height:120px;'
        f'background:#c3c7cf;border:1px solid #8890a0;box-sizing:border-box;" '
        f'title="myPal[{i}][{j}] : TMyOmronPanel（執行期動態建立，Caption={cap_}）">'
        f'<span style="position:absolute;left:8px;top:-2px;font:bold 12px Arial;color:#000080;'
        f'background:#c3c7cf;padding:0 3px;">{cap_}</span>{disp}{cb}{ed}</div>'
    )

def omron_scrollbox_html():
    """ScrollBox1 內 8 unit × 4 channel = 32 個 TMyOmronPanel（OmronEJ1N.cpp 迴圈建立）"""
    return ''.join(omron_panel(i, j) for i in range(8) for j in range(4))

# fMotorTest 馬達列（uMotorTest.cpp TMotorTestClass 執行期建立，parent pnlMotor）
# 排列：iLPitch=192 iTPitch=28 iMaxRowItem=12 欄優先；每列 cbUsing(L,T)+labName(+16)+edPos1(+50,-3)+edPos2(+117,-3)
# 資料＝fMotorTest.TabSheet6.png 實機截圖（M17/M18 未安裝故跳號）
_MOTORTEST_ROWS = [
    (0, '5171', '43433'), (1, '0', '-86628'), (2, '-100', '-5000'), (3, '0', '-2000'),
    (4, '10', '-2507'), (5, '10', '-1000'), (6, '10', '-1761'), (7, '0', '-2000'),
    (8, '-300', '-2000'), (9, '0', '-2000'), (10, '-300', '-2000'), (11, '2857', '44821'),
    (12, '3557', '44219'), (13, '0', '22903'), (14, '-1000', '-5990'), (15, '0', '-28461'),
    (16, '-1199', '-20616'), (19, '-3036', '-45191'), (20, '-17101', '-79935'), (21, '0', '-4716'),
    (22, '0', '-2400'), (23, '0', '-1600'), (24, '0', '-1600'), (25, '0', '-1600'),
    (26, '0', '-1600'), (27, '0', '-1600'), (28, '0', '-1600'), (29, '0', '-1600'),
    (30, '8773', '137926'), (31, '0', '0'), (32, '0', '-5126'), (33, '-560', '-2219'), (34, '0', '-4816'),
]
def motortest_grid_html():
    iLPitch, iTPitch, iLeft, iTop, iMaxRow = 192, 28, 4, 4, 12
    parts = []
    for idx, (mno, v1, v2) in enumerate(_MOTORTEST_ROWS):
        col, row = idx // iMaxRow, idx % iMaxRow
        L, T = iLeft + iLPitch * col, iTop + iTPitch * row
        cap = f'M{mno:02d}'
        parts.append(
            f'<input type="checkbox" title="cbUsing{mno} : TCheckBox（執行期建立）" '
            f'style="position:absolute;left:{L}px;top:{T}px;width:15px;height:17px;margin:0;">'
            f'<span title="labName{mno} : TLabel（執行期建立）" style="position:absolute;'
            f'left:{L+16}px;top:{T}px;width:31px;height:18px;font:13px Arial;color:#000;background:#a6b8c2;">{cap}</span>'
            f'<input class="ed" value="{v1}" title="edPos1_{mno} : TEdit（執行期建立）" '
            f'style="position:absolute;left:{L+50}px;top:{T-3}px;width:65px;height:24px;box-sizing:border-box;">'
            f'<input class="ed" value="{v2}" title="edPos2_{mno} : TEdit（執行期建立）" '
            f'style="position:absolute;left:{L+117}px;top:{T-3}px;width:65px;height:24px;box-sizing:border-box;">'
        )
    return ''.join(parts)

# fHome Home 監視列（uhome.cpp THomeClass 執行期建立，parent Panel1）
# 排列 iLPitch=270 iTPitch=30 iMaxRowItem=15 欄優先；labName(5,8)+ledHome(170,8 20×20)+edPos(195,8 50×20)
# 資料＝fHome.png 實機截圖（[Mxx] MOT.NumberAlias，M17/M18 未安裝跳號）
_HOME_ROWS = [
    (0, 'MInArmX'), (1, 'MInArmY'), (2, 'MInArmPitch'), (3, 'MInArmZA'), (4, 'MInArmZB'),
    (5, 'MInArmZC'), (6, 'MInArmZD'), (7, 'MInArmZE'), (8, 'MInArmZF'), (9, 'MInArmZG'),
    (10, 'MInArmZH'), (11, 'MInShutte1'), (12, 'MInShutte2'), (13, 'MTestY1'), (14, 'MTestZ1'),
    (15, 'MTestZ2'), (16, 'MTestY2'), (19, 'MOutArmX'), (20, 'MOutArmY'), (21, 'MOutArmPitch'),
    (22, 'MOutArmZA'), (23, 'MOutArmZB'), (24, 'MOutArmZC'), (25, 'MOutArmZD'), (26, 'MOutArmZE'),
    (27, 'MOutArmZF'), (28, 'MOutArmZG'), (29, 'MOutArmZH'), (30, 'MTrayX'), (31, 'MInArmPitchY'),
    (32, 'MInArmPitchX2'), (33, 'MOutArmPitchY'), (34, 'MOutArmPitchX2'),
]
def home_grid_html():
    iLPitch, iTPitch, iMaxRow = 270, 30, 15
    iLabelL, iLabelT, iLedL, iLedT, iEditL, iEditT = 5, 8, 170, 8, 195, 8
    parts = []
    for idx, (mno, alias) in enumerate(_HOME_ROWS):
        col, row = idx // iMaxRow, idx % iMaxRow
        dx, dy = iLPitch * col, iTPitch * row
        parts.append(
            f'<span title="labName{idx:02d} : TLabel（執行期建立）" style="position:absolute;'
            f'left:{iLabelL+dx}px;top:{iLabelT+dy}px;height:20px;font:12pt "MS Sans Serif";color:#000080;white-space:nowrap;">[M{mno:02d}] {alias}</span>'
            f'<span title="ledHome{idx:02d} : TALed（執行期建立，LEDSqLarge）" style="position:absolute;'
            f'left:{iLedL+dx}px;top:{iLedT+dy}px;width:20px;height:20px;background:#00e000;border:1px solid #060;box-sizing:border-box;"></span>'
            f'<input class="ed" value="0" readonly title="edPos{idx:02d} : TEdit（執行期建立，唯讀）" style="position:absolute;'
            f'left:{iEditL+dx}px;top:{iEditT+dy}px;width:50px;height:20px;box-sizing:border-box;">'
        )
    return ''.join(parts)

# ---------- Steven 20260919：宣告式執行期面板（由 page-widgets.js 展開） ----------
# 只宣告「要幾個、叫什麼」，座標與長相留在 hwidgets.js 一份。
# 上面 home_grid_html()/motortest_grid_html() 是舊做法（Python 直接吐 HTML），
# 同一組 dfm 座標會同時存在產生器與 hwidgets.js 兩邊，改一邊另一邊不會跟著動。
def widget_host(maker, items, pitch_x=None, pitch_y=None, note='', pad_top=0):
    spec = {'items': items}
    if pitch_x is not None: spec['pitchX'] = pitch_x
    if pitch_y is not None: spec['pitchY'] = pitch_y
    if pad_top: spec['padTop'] = pad_top
    return ('<div class="htWidgetHost" data-maker="%s" data-spec=\'%s\' title="%s"></div>'
            % (maker, json.dumps(spec, ensure_ascii=False).replace("'", '&#39;'), esc(note)))


# ---- ContactForce：THTSLKClass 家族 ----
# Kit 直徑清單來自 Recipe（CheckAndReadIniData "SLK Type"）；這裡用 golden 的
# 程式內建預設值（ContactForce.cpp:447/579）。Visible=0 的那一個 golden 仍會
# new 出來但 GroupBox->Visible=false，所以 HTML 也不畫。
#   SLK Type         30,40,60,56  Visible 1,1,1,0  -> 30/40/60
#   SLK Type Ind     20,30        Visible 1,0      -> 20
#   DieForceType     20,30,40,50  Visible 1,1,1,0  -> 20/30/40
_SLK_DEFAULTS = [
    ('scrlbxDynamicKit',                'std',       ['30', '40', '60'], 'gbLoadRate_%s',                 102),
    ('scrlbxDynamicKitInd',             'ind',       ['20'],             'gbLoadRateInd_%s',              63),
    ('scrlbxDieForceDynamicKit',        'dieforce',  ['20', '30', '40'], 'gbDieForceLoadRate_%s',         67),
    ('scrlbxDieForceOneByOneDynamicKit', 'dieforce1', ['20', '30', '40'], 'gbDieForceOneByOneLoadRate_%s', 63),
]


def slk_injects():
    out = {}
    for box, variant, dias, namefmt, pitch in _SLK_DEFAULTS:
        items = [{'name': namefmt % d, 'dia': d, 'variant': variant, 'row': i}
                 for i, d in enumerate(dias)]
        out[('ContactForce.dfm', box)] = widget_host(
            'makeContactForceGroup', items, pitch_y=pitch, pad_top=12,
            note='%s：執行期 new 的 Load rate 群組（Align=alTop 由上往下堆），'
                 '直徑清單取自 Recipe [SLK Type]，此處為 golden 內建預設' % box)
    return out


# ---- VacuumUnit：TMyVacuumPanel ----
# VacuumUnit.cpp:38-49：USE_46_SUCKER_DB=0（本機 system\Gerneral.ini）→ HT9045 分支，
# iIndexColMax = TOTAL_VACUUM_UNIT/2 = 4、iInOutColMax = TOTAL_VACUUM_UNIT/2 = 4。
# 名稱取自 cpp 的 sIndexName_16 / sInOutName_8（前後有空白是 golden 原樣，用來置中）。
_VAC_INDEX = [['    Aa    ', '    Ab    ', '    Ac    ', '    Ad    '],
              ['    Ba    ', '    Bb    ', '    Bc    ', '    Bd    ']]
_VAC_INOUT = [['     A    ', '     C    ', '     E    ', '     G    '],
              ['     B    ', '     D    ', '     F    ', '     H    ']]


def vacuum_injects():
    out = {}
    for box, names, pfx in (('scrlbxIndexArm1', _VAC_INDEX, 'myPalArm1'),
                            ('scrlbxIndexArm2', _VAC_INDEX, 'myPalArm2'),
                            ('scrlbxInArm',     _VAC_INOUT, 'myPalInArm'),
                            ('scrlbxOutArm',    _VAC_INOUT, 'myPalOutArm')):
        items = []
        for row in range(2):
            for col in range(4):
                items.append({'name': '%s_%d_%d' % (pfx, col, row), 'caption': names[row][col],
                              'cur': '0.0', 'event': 'Event', 'threshold': '0.0', 'sv': '0',
                              'col': col, 'row': row})
        out[(r'VacuumUnit\VacuumUnit.dfm', box)] = widget_host(
            'makeVacuumPanel', items, pitch_x=81, pitch_y=177, pad_top=10,
            note='%s：執行期 new TMyVacuumPanel（%s），4 欄×2 列；'
                 'USE_46_SUCKER_DB=0 → iIndexColMax=iInOutColMax=4' % (box, pfx))
    return out


RUNTIME_INJECT = {
    (r'EJ1N\OmronEJ1N.dfm', 'ScrollBox1'): omron_scrollbox_html(),
    ('uMotorTest.dfm', 'pnlMotor'): motortest_grid_html(),
    ('uhome.dfm', 'Panel1'): home_grid_html(),
}
RUNTIME_INJECT.update(slk_injects())
RUNTIME_INJECT.update(vacuum_injects())

# fSecurity 權限項（cSecurity.cpp TMySecurity 執行期建立，依 Sender 分入 10 個 ScrollBox 堆疊）
# 每列 Panel 50 高：SpeedButton(7,4 300×42 glyph+caption)＋RadioGroup(310,1 458×42 四等級)
def security_inject():
    src = open(os.path.join(BASE, 'cSecurity.cpp'), encoding='cp950', errors='replace').read()
    items = re.findall(r'new TMySecurity\("([^"]+)"\s*,\s*[^,]+,\s*(sb\w+)\)', src)
    LEVELS = ['Operator', 'Engineer', 'Supervisor', 'HonPrec']
    by_cont = {}
    for security_index, (cap_, cont) in enumerate(items):
        by_cont.setdefault(cont, []).append((security_index, cap_))
    out = {}
    for cont, caps in by_cont.items():
        rows = []
        for row_index, (security_index, cap_) in enumerate(caps):
            radios = ''.join(
                f'<label style="display:inline-flex;align-items:center;gap:3px;font:11px \'MS Sans Serif\';white-space:nowrap;">'
                f'<input type="radio" name="sec_{esc(cont)}_{security_index}" value="{li}"{" checked" if li == 2 else ""}>{lv}</label>'
                for li, lv in enumerate(LEVELS))
            rows.append(
                f'<div class="securityRow" data-security-index="{security_index}" '
                f'style="position:absolute;left:0;top:{row_index*50}px;width:824px;height:50px;background:#c2b8a6;'
                f'box-sizing:border-box;" title="MySecurity_Panel_{security_index} : TMySecurity（{esc(cap_)}）">'
                f'<button class="btn3d" style="position:absolute;left:7px;top:4px;width:300px;height:42px;'
                f'justify-content:flex-start;gap:6px;padding:0 8px;font:12px \'MS Sans Serif\';">'
                f'<span style="flex:none;width:22px;height:22px;border:1px solid #99a;background:#eee;'
                f'display:inline-flex;align-items:center;justify-content:center;font-size:12px;">🔑</span>{esc(cap_)}</button>'
                f'<fieldset style="position:absolute;left:310px;top:1px;width:458px;height:42px;border:1px solid #9aa;'
                f'padding:0;display:flex;align-items:center;justify-content:space-around;">{radios}</fieldset></div>')
        out[('cSecurity.dfm', cont)] = ''.join(rows)
    return out

RUNTIME_INJECT.update(security_inject())

# fYieldMonitoring TMyYieldPanel（uYieldMonitoring.cpp）：ART 分頁 PalYield（alClient）
# iRowHeight=24 iColWidth=40 eBinSetTotal=4(Items/PassYield/OpenShort/Recover)；mtTrayName 2欄(ENABLE/Yield)
def yield_inject():
    RH, CW, ROWS = 24, 40, ['Items', 'PassYield', 'OpenShort', 'Recover']
    BINS = 16   # TEST_MAX_BIN=256（實際依測試設定，手冊取代表值 16）
    th = RH * len(ROWS)
    # mtTrayItem：1 欄 × 4 列（列標籤）
    item_cells = ''.join(
        f'<div style="height:{RH}px;border-bottom:1px solid #999;display:flex;align-items:center;'
        f'padding:0 6px;font:10px Arial;box-sizing:border-box;background:#fff;">{r}</div>' for r in ROWS)
    item = (f'<div title="mtTrayItem : TTMyTray256" style="position:absolute;left:8px;top:8px;width:117px;'
            f'height:{th}px;background:#fff;border:2px solid #444;box-sizing:border-box;">{item_cells}</div>')
    # mtTrayName：2 欄（ENABLE/Yield）× 4 列（首列標題）
    def name_row(i):
        if i == 0:
            return ('<div style="display:flex;height:%dpx;">'
                    '<div style="width:70px;border-right:1px solid #999;border-bottom:1px solid #999;'
                    'display:flex;align-items:center;justify-content:center;font:bold 10px Arial;background:#dfe3e8;">ENABLE</div>'
                    '<div style="flex:1;border-bottom:1px solid #999;display:flex;align-items:center;'
                    'justify-content:center;font:bold 10px Arial;background:#dfe3e8;">Yield</div></div>') % RH
        return ('<div style="display:flex;height:%dpx;">'
                '<div style="width:70px;border-right:1px solid #999;border-bottom:1px solid #999;'
                'display:flex;align-items:center;justify-content:center;"><input type="checkbox"></div>'
                '<div style="flex:1;border-bottom:1px solid #999;display:flex;align-items:center;'
                'justify-content:center;font:10px Arial;">100.0</div></div>') % RH
    name = (f'<div title="mtTrayName : TTMyTray256" style="position:absolute;left:125px;top:8px;width:140px;'
            f'height:{th}px;background:#fff;border:2px solid #444;box-sizing:border-box;">'
            f'{"".join(name_row(i) for i in range(len(ROWS)))}</div>')
    # sbBinSetting ScrollBox → mtBinSelectYield（BINS 欄 × 4 列，首列 bin 號）
    bin_rows = []
    _colmap = ['#fff', '#00c000', '#cc0000', '#ffd700']
    for ri, r in enumerate(ROWS):
        cells = ''
        for b in range(BINS):
            if ri == 0:
                cells += (f'<div style="width:{CW}px;height:{RH}px;border:1px solid #888;box-sizing:border-box;'
                          f'display:flex;align-items:center;justify-content:center;font:10px Arial;background:#d0d8e0;">{b+1}</div>')
            else:
                bg = _colmap[ri]
                cells += (f'<div style="width:{CW}px;height:{RH}px;border:1px solid #888;box-sizing:border-box;'
                          f'background:{bg};"></div>')
        bin_rows.append(f'<div style="display:flex;">{cells}</div>')
    binsel = (f'<div title="mtBinSelectYield : TTMyTray256" style="width:{CW*BINS+10}px;">{"".join(bin_rows)}</div>')
    sb = (f'<div title="sbBinSetting : TScrollBox" style="position:absolute;left:265px;top:8px;width:701px;'
          f'height:{th+18}px;overflow:auto;background:#fff;border:1px solid #99a;box-sizing:border-box;">{binsel}</div>')
    la = (f'<span title="laARTLinit : TLabel" style="position:absolute;left:8px;top:{th+15}px;'
          f'font:20px Arial;color:#000;">Auto Retest limit</span>')
    combo = ('<select class="ed" title="ReTestLimit : TComboBox" style="position:absolute;left:147px;top:%dpx;width:100px;">'
             % (th + 15)) + ''.join(f'<option>{v}</option>' for v in range(1, 11)) + '</select>'
    pal = (f'<div title="PalYieldART : TMyYieldPanel（執行期建立，Align=alClient）" '
           f'style="position:absolute;inset:0;background:#c2b8a6;">{item}{name}{sb}{la}{combo}</div>')
    return {('uYieldMonitoring.dfm', 'tsAutoRetest'): pal}

RUNTIME_INJECT.update(yield_inject())

# fYieldMonitoring 類別警報 checkbox/edit 陣列（uYieldMonitoring.cpp FormCreate 迴圈 i<TEST_MAX_BIN）
# 每列 checkbox(8,30i+10 228寬 "Category N ... %")＋edit(135,30i+4 60寬)；分入 6 個 ScrollBox
def yield_category_inject():
    CATS = 16   # Category 0~15 共 16 個（TEST_MAX_BIN=256 為上限，手冊取代表 16）
    # scrollbox → (checkbox 名前綴, edit 名前綴)
    SB = {
        'scrlbxBinAlarm1_FT': ('cbByBinFailureCat_FT', 'edByBinFailureCat_FT'),
        'scrlbxBinAlarm1_RT': ('cbByBinFailureCat_RT', 'edByBinFailureCat_RT'),
        'scrlbxBinAlarm2_FT': ('cbByBinSiteGapCat_FT', 'edByBinSiteGapCat_FT'),
        'scrlbxBinAlarm2_RT': ('cbByBinSiteGapCat_RT', 'edByBinSiteGapCat_RT'),
        'scrlbxBinAlarm3_FT': ('cbByArmSiteGapCat_FT', 'edByArmSiteGapCat_FT'),
        'scrlbxBinAlarm3_RT': ('cbByArmSiteGapCat_RT', 'edByArmSiteGapCat_RT'),
    }
    out = {}
    for sb, (cbp, edp) in SB.items():
        rows = []
        for i in range(CATS):
            top = 30 * i + 10
            rows.append(
                f'<label title="{cbp}{i:03d} : TCheckBox" style="position:absolute;left:8px;top:{top}px;'
                f'width:228px;height:16px;font:12px \'MS Sans Serif\';display:flex;align-items:center;white-space:nowrap;">'
                f'<input type="checkbox" style="margin:0 4px 0 0;flex:none;">Category {i}'
                f'<span style="position:absolute;right:2px;">%</span></label>'
                f'<input class="ed" title="{edp}{i:03d} : TEdit" style="position:absolute;left:135px;top:{top-6}px;'
                f'width:60px;height:28px;box-sizing:border-box;">')
        out[('uYieldMonitoring.dfm', sb)] = ''.join(rows)
    return out

RUNTIME_INJECT.update(yield_category_inject())
# cObserver tsTestCate：以 HTML table 取代 ~16 個 TTMyTray 拼成的試算表（內容見 _partials/testcate_inner.html）
def observer_testcate_inject():
    fp = os.path.join(OUT, '_partials', 'testcate_inner.html')
    try:
        html = open(fp, encoding='utf-8').read()
    except OSError:
        html = ''
    return {('cObserver.dfm', 'ScrollBox1'): html}
RUNTIME_INJECT.update(observer_testcate_inject())
# 注入時保留的 dfm 子元件（不在此＝'*' 全略過取代）；fHome Panel1 保留欄分隔線 pnlLine1/2/3
RUNTIME_INJECT_SKIP = {
    ('uhome.dfm', 'Panel1'): {'Name', 'ledHome', 'edPos'},   # 只略過設計期範本，保留 pnlLine1/2/3
}

JOBS = [
    ('cOffSet.dfm',   'Setup.OffSet.html',   'Offset（cOffSet.dfm / fOffSet : TfOffSet）'),
    ('cSpeed.dfm',    'Setup.Speed.html',    'Speed（cSpeed.dfm / fSpeed : TfSpeed）'),
    ('iosetview.dfm', 'HW.IoSetView.html', 'IO check and verify（iosetview.dfm / fiosetview : Tfiosetview）'),
    ('cConfiguration.dfm', 'Config.Configuration.html', 'Configure（cConfiguration.dfm / fConfiguration : TfConfiguration）'),
    ('cCounterSel.dfm',    'Status.CounterSel.html',   'Counter Selection（cCounterSel.dfm / fCounterSel : TfCounterSel）'),
    ('cCounterClear.dfm',  'Data.CounterClear.html', 'Counter Clear（cCounterClear.dfm / fCounterClear : TfCounterClear）'),
    ('cBuilder.dfm',       'Data.Builder.html',      'Build Setup（cBuilder.dfm / fBuilder : TfBuilder）'),
    ('DIOInterFaceCFG.dfm','Config.DIOInterFaceCFG.html','DIO Interface Configuration（DIOInterFaceCFG.dfm / fDIOFrom : TfDIOFrom）'),
    ('LtcSensor.dfm',      'Status.LtcSensor.html',     'LtcSensor（LtcSensor.dfm / fLtcSensor : TfLtcSensor）'),
    ('cTowerLight.dfm',    'Status.TowerLight.html',   'TowerLight（cTowerLight.dfm / fTowerLight : TfTowerLight）'),
    (r'EJ1N\OmronEJ1N.dfm','HW.OmronEJ1N.html',     'Omron Thermo Controller（EJ1N/OmronEJ1N.dfm / fOmron : TfOmron）'),
    ('QAMode.dfm',         'Setup.QAMode.html',        'QA Mode（QAMode.dfm / fQAMode : TfQAMode）'),
    (r'BarCode\BarCode.dfm','Setup.BarCode.html',      'Bar Code（BarCode/BarCode.dfm / fBarCode : TfBarCode）'),
    (r'CCLink\MyCCLinkSensor.dfm','HW.MyCCLinkSensor.html','Shuttle Sensor Utility（CCLink/MyCCLinkSensor.dfm / fCCLink : TfCCLink）'),
    (r'AutoClean\uCleaning.dfm','Setup.Cleaning.html','Cleaning（AutoClean/uCleaning.dfm / fCleaning : TfCleaning）'),
    ('cContact.dfm',       'Setup.Contact.html',      'Contact（cContact.dfm / fContact : TfContact）'),
    ('cTesterIF.dfm',      'Setup.TesterIF.html',     'Test IF（cTesterIF.dfm / FTestIF : TFTestIF）'),
    (r'GroundMan\GroundMan.dfm', 'Status.GroundMan.html', 'GroundMan（GroundMan/GroundMan.dfm / fGroundMan : TfGroundMan）'),
    ('cLd_ULd.dfm',        'Setup.Ld_ULd.html',       'Loader / Unloader（cLd_ULd.dfm / fLd_ULd : TfLd_ULd）'),
    ('cSecurity.dfm',      'Status.Security.html',     'Password and Security（cSecurity.dfm / fSecurity : TfSecurity）'),
    ('cTrayForm.dfm',      'Setup.TrayForm.html',     'Tray Form（cTrayForm.dfm / fTrayForm : TfTrayForm）'),
    (r'Automation\SCK_ART.dfm', 'Setup.SCK_ART.html',  'Auto Retest（Automation/SCK_ART.dfm / fSCKART : TfSCKART）'),
    ('uYieldMonitoring.dfm','Setup.YieldMonitoring.html','Yield Monitoring（uYieldMonitoring.dfm / fYieldMonitoring : TfYieldMonitoring）'),
    ('cHotPlate.dfm',      'Setup.HotPlate.html',     'Hot Plate（cHotPlate.dfm / fHotPlate : TfHotPlate）'),
    # Data.Observer.html 已改為手工維護（tsTestCate/StringGrid2,3/mtRowA-D 改 HTML table），不覆寫但仍供 ComponentMap
    ('cObserver.dfm',      'Data.Observer.html',     'Observer（cObserver.dfm / fObserver : TfObserver）'),
    ('cSetUp.dfm',         'Setup.SetUp.html',        'Set Up（cSetUp.dfm / fSetup : TfSetup）'),
    ('SmartDiagnostic.dfm','Data.SmartDiagnostic.html','Smart Diagnostic（SmartDiagnostic.dfm / fSmartDiagnostic : TfSmartDiagnostic）'),
    ('cStartCondition.dfm','Data.StartCondition.html','Start Condition（cStartCondition.dfm / fStartCondition : TfStartCondition）'),
    ('uTemp_Set.dfm',      'Setup.Temp_Set.html',     'Temperature Setting（uTemp_Set.dfm / fTemp_Set : TfTemp_Set）'),
    # Setup.BinSel.html 已改為手工表單重構版（7-tab），不覆寫但仍供 ComponentMap
    ('cBinSel.dfm',        'Setup.BinSel.html',       'Bin Selection（cBinSel.dfm / fBinSel : TfBinSel）'),
    ('uteach.dfm',         'HW.teach.html',        'Teaching（uteach.dfm / fTeach : TfTeach）'),
    ('uMotorTest.dfm',     'HW.MotorTest.html',    'Motor Test（uMotorTest.dfm / fMotorTest : TfMotorTest）'),
    ('uhome.dfm',          'HW.home.html',         'Home Monitor（uhome.dfm / fHome : TfHome）'),
    ('cTrayAssignment.dfm','Setup.TrayAssignment.html','Tray Assignment（cTrayAssignment.dfm / fTrayAssignment : TfTrayAssignment）'),
    ('HandlerSys.dfm',     'HW.HandlerSys.html',    'Handler System（HandlerSys.dfm / HandlerSystem : THandlerSystem）'),
    # 兩個 modal 對話框：由 dialog-bridge.js 依 Alarm/Message-dialog-request.json 以 iframe overlay 開啟
    ('mymessbox.dfm',      'Alert.MyMessageBox.html','Message（mymessbox.dfm / MyMessageBox : TMyMessageBox）'),
    ('note.dfm',           'Alert.Note.html',        'Note（note.dfm / fNote : TfNote）'),
    ('Password.dfm',       'Alert.Password.html',    'Password（Password.dfm / fPassword : TfPassword）'),
    # Steven 20260918：Shuttle Maintain。golden 入口 main.cpp:33658 sbShuttleMaintainClick
    # （main.dfm:11303 的 TSpeedButton，在 tsMotionView>ScrollBox1，不是選單項）。
    ('ShuttleMove.dfm',    'HW.ShuttleMove.html',    'Shuttle Maintain（ShuttleMove.dfm / fShuttleMove : TfShuttleMove）'),
    # Steven 20260919：三個新轉換頁。入口分別是
    #   fContactForce  cContact.cpp:15099  fContactForce->Show()（由 Setup.Contact 頁進）
    #   fVacuumUnit    main.cpp:34663      fVacuumUnit->Show()
    #   fAGV           main.cpp:34883      TfMain::spbAGVClick → fAGV->Show()
    ('ContactForce.dfm',   'Setup.ContactForce.html', 'Contact Force Setting（ContactForce.dfm / fContactForce : TfContactForce）'),
    (r'VacuumUnit\VacuumUnit.dfm', 'HW.VacuumUnit.html', 'Vacuum Unit（VacuumUnit/VacuumUnit.dfm / fVacuumUnit : TfVacuumUnit）'),
    (r'Automation\AGV.dfm', 'Setup.AGV.html',         'AMR Setting（Automation/AGV.dfm / fAGV : TfAGV）'),
]

# ---------- 執行期 FormShow 擺位／可見性覆寫 ----------
# dfm 是設計期狀態；BCB6 FormShow()/Reset() 會在顯示時重排。此表在 render 前直接改節點屬性（值用 dfm 原始寫法）。
# 依 General-config：model HT-9046AT、AUTO_EMPTY_COLOR=1（<3 → fNote Width=980）、USE_OUT_SORT_ARM=0、無 2nd Loader。
def _q(s): return "'" + s.replace("'", "''") + "'"
_HIDE = {'Visible': 'False'}
RUNTIME_PROPS = {
    # TMyMessageBox::FormShow 預設分支（Height=250/Width=480）＋ ShowMyMessage() 預設按鍵狀態
    'mymessbox.dfm': {
        'pnlMain':       {'Left': '8',   'Top': '8',   'Width': '457', 'Height': '153'},
        'lblMainMsg':    {'Left': '8',   'Top': '42',  'Width': '441', 'Font.Height': '-16'},   # Font->Size=12
        'lblChineseMsg': {'Left': '2',   'Top': '108', 'Width': '441', 'Font.Height': '-16'},
        'pnlPause':      {'Left': '171', 'Top': '172', 'Width': '130', 'Height': '33', 'Caption': _q('Pause')},
        'pnlYes':        {'Left': '71',  'Top': '172'},
        'pnlNo':         {'Left': '271', 'Top': '172'},
        'pnlAlarmReset': _HIDE,          # 僅 SECS/GEM Alarm 或 HaltHandler 時顯示
        'lblSubMsg':     _HIDE,          # ShowMyMessage 固定隱藏（YES_NO 帶 ';' 才顯示）
        'moSecsGem':     _HIDE,          # 非 SECS 訊息 Visible=false
        'labStopTime':   _HIDE,          # CosFunction.bShowHandlerStopTime 預設 false
    },
    # TfNote::FormShow（AUTO_EMPTY_COLOR<3 分支）＋ Reset()／ShowErrorUnit()／ShowErrSite() 預設狀態
    'note.dfm': {
        'pnlBottom':        {'Width': '972'},              # Width=980 → client 972（alBottom 寬度隨表單）
        'pnlPicker':        {'Left': '680'},               # alRight：972-292
        'ShowMessageEdit1': {'Width': '672'},              # pnlMsg->Width-8（pnlMsg=972-292=680）
        'reDescription':    {'Width': '672'},              # pnlMsg->Width-8
        'RichEdit1':        {'Width': '664'},              # pnlMsg->Width-16
        'palOutArm':        {'Width': '255'},
        'palHead':          {'Left': '229'},               # Reset()
        'BtnHome':          {'Caption': _q('HOME && RETRY')},   # 非 WAR07352
        'palFix4': _HIDE, 'palFix5': _HIDE, 'palFix6': _HIDE,
        'palAuto4': _HIDE, 'palAuto5': _HIDE, 'palAuto6': _HIDE,
        'palAuto4_Car': _HIDE, 'palAuto5_Car': _HIDE, 'palAuto6_Car': _HIDE,
        'palOutSh3': _HIDE, 'palOutArm2': _HIDE,           # USE_OUT_SORT_ARM==eartUninstall
        'palLoad2': _HIDE, 'palLoad2_Car': _HIDE,          # USE_2nd_LOADER==eartUninstall
        'palOutSh': _HIDE, 'palInSh': _HIDE, 'palTemp': _HIDE, 'palScan': _HIDE,   # Reset()
        'palCCD': _HIDE,                                   # ShowErrorUnit()
        'palShtSensorSOP': _HIDE,                          # FormClose()
        'TMyTray1': _HIDE,                                 # ShowErrSite() 僅 UnitNo 1~5/7 顯示
        'pnlMovie': _HIDE,                                 # PlayMovie() 無 D:\Movie\<Code>.avi 即隱藏
        'btnMoveToFront': _HIDE, 'btnMoveToRear': _HIDE,   # 僅 Index 掉料 Alarm 顯示
        'TrayEdit': _HIDE,                                 # Visible=bInArmSuckErr
        'pnlStopTime': _HIDE,                              # CosFunction.bShowHandlerStopTime
        'palOutShuttleLossIC': _HIDE, 'palSGCheckList': _HIDE,
        'pnlCorrectionCount': _HIDE, 'sbBinEdit': _HIDE,
        'labSecsGemLock': _HIDE,                           # main.cpp 僅 SECS 鎖定時顯示
        'palWrongPW':       {'Left': '160', 'Top': '130'},
        **{n: {'Left': '160', 'Top': '290'} for n in (                          # palRedNotice[12]（AUTO_EMPTY_COLOR<3 → Left=160）
            'palCheckLoader', 'palSGCheckList', 'palCheckSht', 'PanSpecialNote', 'pnlIndex1Error', 'pnlIndex2Error',
            'pnlContact', 'pnlCleanSocket', 'pnlContactOver', 'pnlOutArmDrop', 'pnlPan_TriMachineSpecialNote')},
        **{f'palIonFan{i:02d}': _HIDE for i in range(1, 13)},                    # Reset()
    },
}
RUNTIME_PROPS['note.dfm']['palSGCheckList'] = {'Left': '160', 'Top': '290', 'Visible': 'False'}
# Steven 20260919
# TfVacuumUnit 的建構式（VacuumUnit.cpp:52-99）無條件把四個 GroupBox 依吸嘴數重算，
# FormShow 再把表單設成 690x1020。dfm 的設計期尺寸（四個都 300x300、表單 655x711）
# 不是任何一台機台看得到的樣子 —— 照 dfm 出來，8 個吸嘴面板會擠在 300px 裡。
#   VACUUM_UNIT_WIDTH=81  VACUUM_UNIT_HEIGHT=177（VacuumUnit.h）
#   本機 system\Gerneral.ini 的 USE_46_SUCKER_DB=0 → HT9045 分支，
#   iIndexColMax = iInOutColMax = TOTAL_VACUUM_UNIT/2 = 4
#   Width  = (81+2)*4  = 332
#   Height = (177+12)*2 = 378
RUNTIME_PROPS[r'VacuumUnit\VacuumUnit.dfm'] = {
    'grpIndexArm2': {'Top': '-3',  'Left': '0', 'Width': '332', 'Height': '378'},
    'grpIndexArm1': {'Top': '378', 'Left': '0', 'Width': '332', 'Height': '378'},
    'grpInarm':     {'Top': '-3',  'Left': '0', 'Width': '332', 'Height': '378'},
    'grpOutarm':    {'Top': '-3',  'Left': '0', 'Width': '332', 'Height': '378'},
    # Panel1 是 Align=alBottom。pos() 只處理 alClient 的伸縮，alBottom 仍留在設計期
    # 的 Top=616 —— 平常沒差（client 高＝dfm 高），但這一頁 client 被 FormShow 拉到
    # 989，按鈕列就會卡在畫面中間。這裡直接補算：Top=989-64、Width=682。
    'Panel1':       {'Top': '925', 'Left': '0', 'Width': '682'},
}
# TfPassword::FormShow 預設分支：bTechComUseComboBox=false → edUserName 顯示、cbUserName 隱藏；bFtpPasswordDownload=false；Label3~6 由 login-page.js 依驗證結果/kind 開啟
RUNTIME_PROPS['Password.dfm'] = {
    'cbUserName': _HIDE, 'btnDownload': _HIDE, 'lblPWDownload': _HIDE,
}
# Steven 20260918：TfShuttleMove 執行期覆寫。
# palSh1/2Encoder 的 dfm Caption='0' 是設計期殘留；執行期是兩顆 Shuttle 馬達的
# 編碼器實測值（golden ShuttleMove.cpp:137-138
#   palSh1Encoder->Caption=MOT[MInShuttle1].ReadEncoderPos();）。
# 網頁還沒有這個 tag ⇒ 顯示 '---'（不可知），絕不可留 '0' 假裝是實況。
# rbTemp：dfm Left=-77 停在表單外（VCL 會被父容器裁掉），且 ShuttleMove.cpp
# 全檔 0 次引用（只在 ShuttleMove.h:56 有宣告）⇒ 設計期殘留，隱藏。
RUNTIME_PROPS['ShuttleMove.dfm'] = {
    'palSh1Encoder': {'Caption': _q('---')},
    'palSh2Encoder': {'Caption': _q('---')},
    'rbTemp': _HIDE,
    # FormShow 的機種相依可見性（golden ShuttleMove.cpp:88-148），
    # 依 D:\HT9045\system\Gerneral.ini 實測值套用：
    #   SHUTTLE_SENSOR_TYPE=4 (eSensorCanBus, MachineType.h:770)
    #     -> :89  sbShuttleSensor->Visible=true（dfm 是 False，要反過來）
    #   ENABLE_OUT_SHUTTLEY_LATCH=1
    #     -> :140 sbSensorLatch->Visible=true（dfm 是 False，要反過來）
    #   In_Shuttle_Auto_Latch=0 (非 eInSHAutoLtc, MachineType.h:1581)
    #     -> :146 gbInFiberCheckShtSnLct->Visible=false（dfm 是顯示的）
    #     -> :142 gbScanOutShuttle 標題維持 dfm 的 'Detect Out Shuttle device'
    #   BAR_CODE_INSTALL=3 (非 ebctUninstall) -> :88 gbBarCode 保持顯示（與 dfm 相同，不用寫）
    'sbShuttleSensor': {'Visible': 'True'},
    'sbSensorLatch':   {'Visible': 'True'},
    'gbInFiberCheckShtSnLct': _HIDE,
}
# 執行期 TabVisible=false 的 TPageControl（頁籤列不顯示，內容貼頂）
HIDE_TABS = {('note.dfm', 'pgcNote'), ('Password.dfm', 'PageControl1')}   # TfNote 建構子；TfPassword::FormShow bShowTab==0 只留 tsPassword

def apply_runtime_props(root, dfm):
    tbl = RUNTIME_PROPS.get(dfm)
    if not tbl: return
    def walk(n):
        if n.name in tbl: n.props.update(tbl[n.name])
        for k in n.kids: walk(k)
    walk(root)
# 下列頁面已改為手工維護（dual JSON / offline / 可編輯表格 / motor-access 動作指令）：
# 仍留在 JOBS（ComponentMap 章節靠 JOBS 順序配對 anchor），但不覆寫 HTML 檔
NO_OVERWRITE = {'HW.IoSetView.html', 'HW.teach.html', 'HW.MotorTest.html',
                'Data.Observer.html', 'Setup.BinSel.html', 'HW.HandlerSys.html'}

# 頁面附加：額外 <script> 與 <body> 屬性（設定檔 JSON → 元件綁定；settings-bind.js 依 data-settings-bind 自動執行）
PAGE_EXTRA = {
    'Config.Configuration.html': {'scripts': ['settings.js', 'settings-bind.js'], 'bodyAttr': 'data-settings-bind="config"'},
    'HW.HandlerSys.html':     {'scripts': ['settings.js', 'settings-bind.js', 'handler-search.js', 'handler-customer-search.js', 'handler-track-table.js'], 'bodyAttr': 'data-settings-bind="general"'},
    'Status.Security.html':   {'scripts': ['settings.js', 'security-access.js', 'json-writer.js'], 'bodyAttr': 'data-security-access="1"'},
    # dialog-page.js：接 Alarm/Message-dialog-request（HT_DIALOG_REQUEST）填值、依 kCode 顯示按鍵、回 HT_DIALOG_ACTION
    'Alert.MyMessageBox.html': {'scripts': ['dialog-page.js'], 'bodyAttr': 'data-dialog="message"'},
    'Alert.Note.html':         {'scripts': ['dialog-page.js'], 'bodyAttr': 'data-dialog="alarm"'},
    'Alert.Password.html':     {'scripts': ['login-page.js'], 'bodyAttr': 'data-dialog="auth"'},
    # Steven 20260919：page-widgets.js 把 .htWidgetHost 展開成 hwidgets 元件。
    # 必須排在 hwidgets.js 之後 —— PAGE_EXTRA 的 script 是接在 </body> 前面，
    # 而 hwidgets.js 在 PAGE_TMPL 裡已經更早載入，順序自然正確。
    'Setup.ContactForce.html': {'scripts': ['page-widgets.js']},
    'HW.VacuumUnit.html':      {'scripts': ['page-widgets.js']},
    # Steven 20260921：Set Up 的 Site Mode 行為 —— golden cSetUp.cpp 的
    # ScrollBar1Change / CompChange / chkOffCenterkitClick / btnLUpToRDownNClick。
    # 順序不可顛倒：cosflags（客戶碼旗標表，gen_setup_cosflags.py 產）要先於
    # sitemap（行為）。兩支都只在 DOMContentLoaded 之後才動 DOM，所以和
    # gen_wire.py 插的那幾行誰先誰後都成立。
    'Setup.SetUp.html':        {'scripts': ['ht9045_setup_cosflags.js',
                                            'ht9045_setup_sitemap.js']},
}
def apply_page_extra(out, page):
    ex = PAGE_EXTRA.get(out)
    if not ex:
        return page
    if ex.get('bodyAttr'):
        page = page.replace('<body class="winpage">', '<body class="winpage" ' + ex['bodyAttr'] + '>', 1)
    tags = ''.join('<script src="%s"></script>\n' % j for j in ex.get('scripts', []))
    return page.replace('</body>', tags + '</body>')

# Config 頁大量元件 dfm Visible=False、由執行期程式碼開啟；比照實機截圖全部顯示
SHOW_HIDDEN_JOBS = {'cConfiguration.dfm'}

# 子樹頁：把大型 dfm 內單一容器（如 main.dfm 的 tsActionView）當成獨立頁面輸出
# (dfm, 容器元件名, 輸出檔, 標題, 額外 script)
SUBTREE_JOBS = [
    ('main.dfm', 'tsActionView', 'Main.MotionView.html',
     'Motion View（main.dfm tsActionView）', ['settings.js', 'motionview-sim.js']),
]
# 但下列元件與可見元件互斥疊放（或為開發測試框），保持隱藏（不論 Visible）
HIDE_ALWAYS = {
    'cbA69',      # 與 [A66] 2D Sort 同位置疊放
    'cbOverSetTempMustOpenFan_UltraTempKitSupportAmbient',  # 與 [LA20-4] 99% 疊放
    'edtTemp',    # 開發測試框（Text='edtTemp'）蓋住 palTrayDef
}
# 執行期互斥疊放但手冊要同時呈現 → 改放到空位（Left, Top）
POS_OVERRIDE = {
    'cbA15_1': (9, 268),  # dfm Top=140 與 [A16](142)/lblA16(168) 疊放，移到 [A17] 群組(結束261)與 [A19](321) 之間
    # Vacuum 頁：lbSuckEnabled（Visible=False 的 40px 警示列）佔位使 pgcVacuum 設計期 Top=44；
    # 實機執行期該列隱藏、alClient 重新對齊貼頂 → 比照實機上移
    'pgcVacuum': (4, 4),
    # System 頁同型案例：labIonFanClean（Visible=False 的 40px alTop 警示列）佔位，
    # 實機隱藏後 alTop/alClient 上移 40px 貼頂
    'pnlRearEMG': (4, 4),
    'pnlSystemClient': (4, 34),
}
# 預設顯示分頁（依表單名→頁籤名；未列者取第 1 頁）。dfm ActivePage 是設計器最後存檔狀態不可直接用。
# Config 開啟預設要看到 A[01]：PageControl1→tsConfig（非 tsSoftSimu）、pcConfig→tsA00、pcA00→tsA_00
DEFAULT_ACTIVE = {
    'cConfiguration.dfm': {'PageControl1': 'tsConfig', 'pcConfig': 'tsA00', 'pcA00': 'tsA_00'},
    'uMotorTest.dfm': {'PageControl1': 'TabSheet6'},   # 預設顯示 Motor Test 分頁
    'uteach.dfm':     {'PageControl2': 'tsAxleCtrl'},  # Teaching 預設進 Axle Control（HW.teach.html 為手工頁，已直接改；此處供重生一致）
}

# 表單 client 尺寸修正：dfm 缺 ClientWidth/ClientHeight 時自動扣視窗邊框標題列（W−8 / H−31，XP 風格）；
# 特例可用 CLIENT_OVERRIDE 手指定（依子面板幾何驗算）
CLIENT_OVERRIDE = {r'EJ1N\OmronEJ1N.dfm': (1326, 764),   # Panel7(437)+Panel1(889)、1326；471+Panel9(293)=764
                   'cObserver.dfm': (992, 775),          # FormShow 設 Width=1000（dfm ClientWidth 932 會讓 labDeviceName 944 溢出）
                   r'AutoClean\uCleaning.dfm': (996, 856),  # 加高 9px：grpTrayData(847,alRight)+基底偏移4 → 851，856 容納免捲軸
                   'mymessbox.dfm': (472, 219),          # FormShow 預設分支 Width=480/Height=250（bsSingle 扣邊框）
                   'note.dfm': (972, 761),               # FormShow AUTO_EMPTY_COLOR<3：Width=980；高度沿用 dfm ClientHeight
                   # Steven 20260919：TfVacuumUnit::FormShow 設 Width=690 / Height=1020（扣邊框標題列）
                   r'VacuumUnit\VacuumUnit.dfm': (682, 989)}

imgs_used = {}
sizes = {}
roots = {}
# 預載 Exit/Save 統一標準（cOffSet sbtExit/spbSave：glyph＋Arial -16）
def _find_node(n, nm):
    if n.name == nm: return n
    for k in n.kids:
        f = _find_node(k, nm)
        if f: return f
    return None
_std_root = parse_dfm(os.path.join(BASE, 'cOffSet.dfm'))
for _key, _nm in (('exit', 'sbtExit'), ('save', 'spbSave')):
    _n = _find_node(_std_root, _nm)
    _g = glyph_png(_n) if _n else None
    if _g: STD_BTN[_key] = (_g, fontpx(_n))
print('STD_BTN:', {k: (v[0][0], 'fs=%d' % v[1]) for k, v in STD_BTN.items()})
for dfm, out, title in JOBS:
    SHOW_HIDDEN = dfm in SHOW_HIDDEN_JOBS
    root = parse_dfm(os.path.join(JOB_BASE.get(dfm, BASE), dfm))
    roots[dfm] = root
    apply_runtime_props(root, dfm)
    if dfm in TAB_EXTRACT:
        extract_tabs(root, TAB_EXTRACT[dfm])
    mark_alclient(root)
    LED_PAIR.clear(); LABEL_SKIP.clear()
    if dfm in LABELED_LED_JOBS:
        resize_leds(root)
        repitch_rows(root)
        respace_stacks(root)
        pair_led_labels(root)
    cw = root.i('ClientWidth', root.i('Width', 1008) - 8)
    ch = root.i('ClientHeight', root.i('Height', 731) - 31)
    if dfm in CLIENT_OVERRIDE: cw, ch = CLIENT_OVERRIDE[dfm]
    sizes[out] = (cw, ch)
    body = ''.join(render(k, dfm, imgs_used) for k in root.kids)
    if out in NO_OVERWRITE:
        print(out, 'skipped (手工維護頁，僅取元件樹供 ComponentMap)')
        continue
    with open(os.path.join(OUT, out), 'w', encoding='utf-8') as f:
        f.write(apply_page_extra(out, PAGE_TMPL.format(title=esc(title), body=body, cw=cw, ch=ch)))
    print(out, 'written,', len(body)//1024, 'KB body, form', cw, 'x', ch)

# ---------- 子樹頁（單一 TabSheet → 獨立頁） ----------
def find_node(n, name):
    if n.name == name:
        return n
    for k in n.kids:
        r = find_node(k, name)
        if r:
            return r
    return None

for dfm, node_name, out, title, extra_js in SUBTREE_JOBS:
    SHOW_HIDDEN = False
    node = parse_dfm_block(os.path.join(JOB_BASE.get(dfm, BASE), dfm), node_name)
    if node is None:
        print('subtree miss:', dfm, node_name)
        continue
    mark_alclient(node)
    LED_PAIR.clear(); LABEL_SKIP.clear()
    cw = node.i('Width', 1008); ch = node.i('Height', 731)
    sizes[out] = (cw, ch)
    body = ''.join(render(k, dfm, imgs_used) for k in node.kids)
    page = PAGE_TMPL.format(title=esc(title), body=body, cw=cw, ch=ch)
    if extra_js:
        tags = ''.join('<script src="%s"></script>\n' % j for j in extra_js)
        page = page.replace('</body>', tags + '</body>')
    with open(os.path.join(OUT, out), 'w', encoding='utf-8') as f:
        f.write(page)
    print(out, 'written (subtree),', len(body) // 1024, 'KB body,', cw, 'x', ch)

# 底圖 BMP → PNG（BMP 一律去背：左下角像素色轉透明）
from PIL import Image
os.makedirs(os.path.join(OUT, 'img'), exist_ok=True)
for src, png in imgs_used.items():
    im = Image.open(src)
    if src.lower().endswith('.bmp'): im = bmp_debg(im)
    im.save(os.path.join(OUT, 'img', png))
    print('img:', src, '->', png)

# ---------- ComponentMap 章節 ----------
GENERIC = re.compile(r'^(Label|Panel|GroupBox|Bevel|Image|Shape|StaticText)\d+$')
def collect_names(n, out, dfm):
    name = html_id(dfm, n.name)
    if n.typ not in SKIP_TYPES and (not GENERIC.match(n.name) or name != n.name):
        out.append((name, n.typ))
    for k in n.kids:
        collect_names(k, out, dfm)

def mk_section(anchor, page, dfm, formsig, root):
    rows = []; pc = None; others = []
    for k in root.kids:
        if k.typ == 'TPageControl' and pc is None: pc = k
        else: others.append(k)
    if pc:
        for ts in pc.kids:
            if ts.typ != 'TTabSheet': continue
            names = []
            for k in ts.kids: collect_names(k, names, dfm)
            lst = '／'.join(f'<code>{esc(nm)}</code>' for nm, _ in names) or '（無具名控制項）'
            rows.append(f'  <tr><td><code>{esc(ts.name)}</code><br>{esc(cap(ts, ts.name))}</td>'
                        f'<td style="font-size:11px;">{lst}</td></tr>')
    onames = []
    for k in others: collect_names(k, onames, dfm)
    if onames:
        lst = '／'.join(f'<code>{esc(nm)}</code>' for nm, _ in onames)
        rows.append(f'  <tr><td>其他（分頁外）</td><td style="font-size:11px;">{lst}</td></tr>')
    return (f'<!-- ====================== {page} ====================== -->\n'
            f'<h2 id="{anchor}">{page} <small>對應 {dfm}／{formsig}（元件名稱自 .dfm 自動擷取，略過 Label 等自動命名控制項）</small></h2>\n'
            f'<table>\n  <tr><th style="width:190px;">分頁（TTabSheet）</th><th>控制項名稱</th></tr>\n'
            + '\n'.join(rows) + '\n</table>\n')

sections = []
_ANCHOR_SEQ = [
        ('offset', 'fOffSet : TfOffSet'), ('speed', 'fSpeed : TfSpeed'), ('io', 'fiosetview : Tfiosetview'),
        ('config', 'fConfiguration : TfConfiguration'),
        ('countersel', 'fCounterSel : TfCounterSel'), ('counterclear', 'fCounterClear : TfCounterClear'),
        ('builder', 'fBuilder : TfBuilder'), ('dioform', 'fDIOFrom : TfDIOFrom'),
        ('ltcsensor', 'fLtcSensor : TfLtcSensor'),
        ('towerlight', 'fTowerLight : TfTowerLight'), ('omron', 'fOmron : TfOmron'),
        ('qamode', 'fQAMode : TfQAMode'), ('barcode', 'fBarCode : TfBarCode'),
        ('cclink', 'fCCLink : TfCCLink'), ('cleaning', 'fCleaning : TfCleaning'),
        ('contact', 'fContact : TfContact'),
        ('testerif', 'FTestIF : TFTestIF'), ('groundman', 'fGroundMan : TfGroundMan'),
        ('ldud', 'fLd_ULd : TfLd_ULd'), ('security', 'fSecurity : TfSecurity'),
        ('trayform', 'fTrayForm : TfTrayForm'), ('sckart', 'fSCKART : TfSCKART'),
        ('yieldmon', 'fYieldMonitoring : TfYieldMonitoring'), ('hotplate', 'fHotPlate : TfHotPlate'),
        ('observer', 'fObserver : TfObserver'),
        ('setup', 'fSetup : TfSetup'), ('smartdiag', 'fSmartDiagnostic : TfSmartDiagnostic'),
        ('startcond', 'fStartCondition : TfStartCondition'), ('tempset', 'fTemp_Set : TfTemp_Set'),
        ('binsel', 'fBinSel : TfBinSel'), ('teach', 'fTeach : TfTeach'),
        ('motortest', 'fMotorTest : TfMotorTest'), ('home', 'fHome : TfHome'),
        ('trayassign', 'fTrayAssignment : TfTrayAssignment'),
        ('handlersys', 'HandlerSystem : THandlerSystem'),
        ('mymessbox', 'MyMessageBox : TMyMessageBox'), ('note', 'fNote : TfNote'),
        ('password', 'fPassword : TfPassword'),
        ('shuttlemove', 'fShuttleMove : TfShuttleMove'),
        ('contactforce', 'fContactForce : TfContactForce'),
        ('vacuumunit', 'fVacuumUnit : TfVacuumUnit'),
        ('agv', 'fAGV : TfAGV')]
# 註：ANCHORS 依 dfm 對應（早期用位置 zip，JOBS 增刪時會整段錯位）
ANCHORS = {j[0]: a for j, a in zip(JOBS, _ANCHOR_SEQ)}
for dfm, out, title in JOBS:
    sections.append(mk_section(ANCHORS[dfm][0], out, dfm, ANCHORS[dfm][1], roots[dfm]))
cm_path = os.path.join(OUT, 'IDE.ComponentMap.html')
cm = open(cm_path, encoding='utf-8').read()
start = cm.index('<!-- ====================== Setup.OffSet.html')
end = cm.index('</main>')
cm = cm[:start] + '\n'.join(sections) + '\n' + cm[end:]
open(cm_path, 'w', encoding='utf-8').write(cm)
print('IDE.ComponentMap.html updated')
print('window sizes:', sizes)
