# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# HW.teach / HW.HandlerSys 改接機台設定檔；新增 sysEnums（radio/checkbox/combo）。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""gen_wire.py -- 從 extract_page_v2.py 的輸出產生每頁的接線資料檔。

AI(W906-FW-GEN) 20260914。

產出 D:\\HT9045\\web\\page\\ht9045_wire_<slug>.js，內容只有資料，
（AI(W906-P5B) 20260923：原本產到 D:\\HT9045\\client\\，P5 裁決甲把 web\\page 定為唯一權威、client\\ 退出版控，
  所以產出落點改到 web\\page —— 否則下次重跑會把接線寫回一個已退場的目錄。）
行為由 ht9045_wire_engine.js 提供。

規則：
  fields  只放 safe（該鍵在所有擁有該文件的配方裡都存在）的欄位。
          不安全的鍵一旦送出就會讓 preview 回 notFound，依規則 2 整頁拒寫。
  pending 放不安全的，附上 n/denom，讓人知道為什麼沒接。
  kb      golden ShowQwertyKey 的旗標與夾限，沒有依據的欄位引擎會給全 QWERTY。
"""
import json, os, re, sys, io

HERE    = os.path.dirname(os.path.abspath(__file__))
CLIENT  = r'D:\HT9045\web\page'   # AI(W906-P5B) 20260923：P5 裁決甲，原本是 D:\HT9045\client（名稱沿用，指向改掉）
WEBPAGE = r'D:\HT9045\web\page'

# 伺服器認得的文件名（GET /api/recipe/ 回的）
DOCS = ['armCondition', 'binasgn', 'binasgnOff-Line', 'binasgnOff', 'binasgnOff_ART',
        'binasgn_ART', 'contact', 'handlerCondition', 'hotPlate', 'rotate',
        'temperature', 'tester', 'testMode', 'tray', 'udUld', 'configByRecipe']
DOCMAP = {d.lower(): d for d in DOCS}


def api_doc(fname):
    """'armcondition.data' -> 'armCondition'，對不上就回 None。"""
    if not fname:
        return None
    stem = re.sub(r'\.(data|ini)$', '', fname.lower())
    return DOCMAP.get(stem)


def js_str(s):
    return "'" + str(s).replace('\\', '\\\\').replace("'", "\\'") + "'"


# 這些頁不是配方頁（讀 config.ini 等機台層設定），只給小鍵盤，不產生配方對照。
# AI(W906-FW-SYSFILE) 20260915：這些頁綁的是機台設定檔（/api/system/），不是配方。
# 抽出的 (區段,鍵) 會逐一比對該檔實際內容，命中的才接；沒命中的不猜。
SYS_PAGES = {'Config.Configuration.html': 'config',
             # dio 的路徑是算出來的（依 config 開關＋Tester.Data 的 TypeName），
             # 所以鍵索引不能從固定路徑讀，要問 API。
             'Config.DIOInterFaceCFG.html': 'dio',
             # Steven 20260916
             # 這兩頁改走機台設定檔。值是 None 代表「不要用整頁一個檔的假設，
             # 改用抽取器逐欄位給的 sysfile」—— uteach.dfm 這一個表單就同時寫
             # teach.ini（140 個）與 Gerneral.ini（2 個），假設一頁一個檔會把
             # 那 2 個鍵寫到 teach.ini 去，而且兩邊都有同名區段時不會報錯。
             'HW.teach.html': None,
             'HW.HandlerSys.html': None}
SYS_PATH = {'config':   r'D:\HT9045\config\config.ini',
            'gerneral': r'D:\HT9045\system\Gerneral.ini',
            'teach':    r'D:\HT9045\system\teach.ini'}


def sys_keys_via_api(name):
    """動態路徑的檔（dio）只能問伺服器要目前生效的內容。"""
    import json as _j, urllib.request as _u
    try:
        d = _j.load(_u.urlopen('http://127.0.0.1:8045/api/system/' + name))
    except Exception as e:
        print('  WARN: /api/system/%s 讀不到（%s）' % (name, e))
        return {}
    idx = {}
    for sec, kv in (d.get('sections') or {}).items():
        for k in kv:
            idx[(sec.lower(), k.lower())] = (sec, k)
    return idx


def sys_keys(path):
    idx, sec, txt = {}, '', None
    for enc in ('utf-8', 'cp950', 'latin-1'):
        try:
            txt = open(path, encoding=enc, errors='strict').read(); break
        except (UnicodeDecodeError, LookupError):
            continue
    if txt is None:
        return idx
    for line in txt.splitlines():
        t = line.strip()
        if t.startswith('[') and t.endswith(']'):
            sec = t[1:-1].strip()
        elif '=' in t and not t.startswith((';', '#')):
            k = t.split('=', 1)[0].strip()
            idx[(sec.lower(), k.lower())] = (sec, k)
    return idx


KB_ONLY = {'Config.Configuration.html',
           # Steven 20260916
           # ⚠ 更正 20260915 的註解。原文寫「HW.teach 在 FW 戰役政策裡標為
           #   永不自動接線」—— 查證後那句話不存在於任何政策檔，是本產生器
           #   自己在 20260914 寫下、隔天又被當成理由引用的循環依據。
           #   fw-wave-loop SKILL.md 第 3 條的原文是「唯讀是硬邊界：write path
           #   是安全關鍵，**夜間永不做**」，限制的是無人值守的自動波次，
           #   不是這一頁本身。使用者 20260916 明確要求接線，該條不適用。
           #
           # HW.teach 與 HW.HandlerSys 已移到 SYS_PAGES 接機台設定檔。
           # 這兩頁留在這裡：它們是 StringGrid 表格頁（motTable / ioTable 兩個
           # csv），widget 與 (區段,鍵) 沒有一對一關係，要走 sysRows 而不是
           # sysFields；HW.MotorTest 還有 66 個文字框根本沒有 id，接不了。
           'HW.MotorTest.html', 'HW.IoSetView.html'}


# Steven 20260916（使用者要求移除浮動的 "Save to recipe" / "Reload" 注入鈕）
# 每頁自己的存檔鈕 / 重讀鈕 id。引擎不再猜，也不再自己生一顆浮動鈕。
#
#   '<頁面>': (saveBtn, reloadBtn, '<golden 依據>')
#
# ⚠ 只收查證過的。引擎的舊猜測清單（spbSave/btSave/btnSave/btOK/btApply）仍在，
#   所以已經用那些 id 的頁面不用列進來；這張表補的是「有存檔鈕但 id 不在清單裡」
#   的 16 頁。查不到依據的就不列 —— 那一頁會在狀態列明說「唯讀」，
#   比隨便綁一顆看起來像存檔的鈕安全。
#
# ⚠ btnOk 不是存檔鈕。fQAMode.cpp:66 的 btnOkClick 只有 Close()；
#   uYieldMonitoring.cpp 的 btnOkClick 只有 ReadFile()+Close()。
#   真正寫檔的是 btnApply（DoFormToData + WriteIniData）。綁錯那顆會變成
#   「按了沒反應」，而且看起來像接線壞掉。
BTN_PAGES = {
    'HW.HandlerSys.html':         ('SaveBtn', 'LoadBtn',
                                   'golden HandlerSys.cpp:1121 SaveBtnClick -> SaveSystemSet()；:1127 LoadBtnClick -> LoaderSystemSet()'),
    'Data.StartCondition.html':   ('sbSave', '',
                                   'golden cStartCondition.cpp:708 sbSaveClick'),
    'HW.OmronEJ1N.html':          ('btSaveData', '',
                                   'golden EJ1N/OmronEJ1N.cpp:2051 btSaveDataClick'),
    'Setup.Cleaning.html':        ('sbCleanSave', '',
                                   'golden AutoClean/uCleaning.cpp:1778 sbCleanSaveClick'),
    'Setup.QAMode.html':          ('btnApply', '',
                                   'forms/fQAMode.cpp:177 btnApplyClick -> DoFormToData + WriteIniData（btnOk 只有 Close）'),
    'Setup.YieldMonitoring.html': ('btnApply', '',
                                   'golden uYieldMonitoring.cpp btnApplyClick（btnOk 只有 ReadFile+Close）'),
    'Setup.SetUp.html':           ('sbUpdate', '',
                                   'golden cSetUp.cpp sbUpdateClick'),
}

# Steven 20260916 (W906-FW-TAGSUB)
# 執行期 tag -> 畫面元素。唯讀顯示，不進 save()。
#
#   '<頁面>': { '<tag>': ['<元素 id>', '<形狀>', <小數位或 None>, '<依據>'] }
#
# ⚠ 這張表只收「有依據」的對應，不收看起來合理的猜測。
#   一個接錯的 tag 會讓畫面上一個看起來正常的數字其實來自別的東西，
#   那比空著更危險。目前 wb_serve 送 117 個 tag，這裡只有 4 個 ——
#   不是漏掉，是其餘 113 個在現行 HTML 上沒有可驗證的目標元素。
#   完整盤點見 ScreenShots.html 的「執行期 tag」段。
#
# 為什麼舊的 docs/web-fw-legacy/js/model/tagmap.js 不能用：
#   它的 join key 是 .dfm 葉節點名，對的是那一版 legacy dashboard；
#   實測 43 筆裡只有 9 筆的葉名還存在於現行頁面，而且多半是導覽鈕不是資料。
TAG_PAGES = {
    'main.html': {
        # Temperature.fWorkTemperBase -> edWorkTemperBase：名字就是 golden 的欄位名，
        # main.html:143 那個 span 本來就標著 edWorkTemperBase。
        'temp.sv':        ['edWorkTemperBase', 'text', 1,
                           'WebBridgeTags.cpp:611 Temperature.fWorkTemperBase'],
        'temp.soak':      ['edSoakTime', 'text', 0,
                           'WebBridgeTags.cpp:612 Temperature.fSoakTime'],
        # fMain->cbSetupFileName 的唯讀鏡像；tagmap.js 自己也記了這一條
        # 「Panel2.edSetupFileName mirrors it as read-only text」。
        'recipe.current': ['edSetupFileName', 'text', None,
                           'WebBridgeTags.cpp:449 fMain->cbSetupFileName->Text'],
        # Steven 20260916：clock.text **刻意不接**。
        # WebBridgeTags.cpp:654 把它 gate 在 `pumping` 上 —— 只有 wb_publish --pump
        # 那個模擬模式才有值，wb_serve 之下恆為 null。那段註解自己講了理由：
        # 「a publisher that is not driving the spine has a static screen by
        #  definition, so a ticking clock on it would be the misleading part.」
        # 它是模擬用的時鐘，不是 handler 的狀態列時鐘。
        #
        # main.html:425 有自己的 setInterval 牆上時鐘（瀏覽器時間，每秒更新）。
        # 那**不是**機台資料，所以不算「Simulator 路徑 vs wb_serve 路徑」的衝突，
        # 保留它是對的。我一度把它誤認成「patch 訊框在跳」的證據——實測
        # wb_serve 在 6 秒內送 0 幀 patch，那個時鐘全部是本地 JS 在跑。
    },
}

# Steven 20260916
# 表格頁：整張 csv 就是檔案，走引擎的 sysGrid 模式（不是 sysRows 的逐格對照）。
# host / save / reload 是頁面上 golden 元件對應的 id；kb 規則照抄 golden 對表頭的
# AnsiPos 子字串比對（uMotorTest.cpp:2229-2243、iosetview.cpp:3254-3262）。
GRID_PAGES = {
    'HW.MotorTest.html': dict(
        file='motTable', host='strngrdMotorData', save='sbUpdate', reload='sbtReload',
        add='btnAddMotor', delete='btnDeleteMotor',      # Steven 20260916：新增/刪除列走 system.csv.rows
        # 審查 E1：Motorname 是鍵欄，新增列時要能打 'M'，且必須是 M%02d 才對得到 golden 的馬達 enum。
        keyPattern=r'^M\d{2}$',
        readOnly=[],
        kb=[['Motorname', 'NO_SYMBOL|NO_SPACE'], ['Alias', 'NO_SYMBOL|NO_SPACE'], ['CardModel', 'NO_SYMBOL|NO_SPACE'],
            ['GearRatio', 'DOUBLE'], ['Acc', 'DOUBLE'], ['Dec', 'DOUBLE'], ['*', 'INTEGER']],
        filters=[]),
    'HW.IoSetView.html': dict(
        file='ioTable', host='strngrdIoTable', save='sbUpdate', reload='sbtReload',
        add='btnAddIO', delete='btnDeleteIO',             # Steven 20260916
        readOnly=[],
        # golden：表頭含 Type / Alias / Note -> 文字鍵盤；其餘 -> 整數。
        # 注意 'Type' 用子字串比對會同時命中 IOType / ModuleType / InType，
        # golden 就是這樣（AnsiPos），照抄不「修正」。
        kb=[['Type', 'NO_SYMBOL|NO_SPACE'], ['Alias', 'NO_SYMBOL|NO_SPACE'],
            ['Note', 'NO_SYMBOL|NO_SPACE'], ['*', 'INTEGER']],
        filters=[dict(el='edtSearchIO', kind='search'),
                 dict(el='cbbType', kind='equals', col='IOType', all='All'),
                 dict(el='cbbLane', kind='equals', col='Lane', all='All')]),
}


def js_lit(v):
    """Python 值 -> JS 字面值（只處理這裡會用到的型別）。"""
    import json as _j
    return _j.dumps(v, ensure_ascii=False)


def emit(row):
    page = row['page']                       # Setup.Speed.html
    slug = re.sub(r'[^a-z0-9]', '', page.replace('.html', '').lower())
    fields, optional, pending, skipped = {}, {}, {}, []

    sysfields, sysenums, sysabsent = {}, {}, {}
    if page in SYS_PAGES:
        # Steven 20260916
        # 索引現在是「每個用到的系統檔各一份」，而不是整頁共用一份。
        cache = {}

        def index_of(f):
            if f not in cache:
                cache[f] = sys_keys_via_api(f) if f not in SYS_PATH else sys_keys(SYS_PATH[f])
            return cache[f]

        pagefile = SYS_PAGES[page]

        def target(h):
            """這一筆寫到哪個系統檔。頁層設定優先，否則用抽取器逐筆解析的結果。"""
            return pagefile or h.get('sysfile')

        for wid, h in sorted(row['fields'].items()):
            if h.get('html') != 'text':
                continue                      # 文字框走 sysFields
            f = target(h)
            if not f:
                continue                      # 解析不出目標檔就不接，不猜
            probe = (h['section'].lower(), h['key'].lower())
            idx = index_of(f)
            if probe in idx:
                sysfields[wid] = (f, idx[probe][0], idx[probe][1])
            else:
                # Steven 20260916
                # 契約在、實體檔沒有這個鍵。teach.ini 的 ...Ae/Af/Ag/Ah 是 site
                # 變體，這台開發機的機型沒有那些 site。這與「抽取抽錯」不同，
                # 分開記成 sysAbsent，上機台後鍵存在就會自動生效。
                sysabsent[wid] = (f, h['section'], h['key'])

        # Steven 20260916：同一 widget 綁多個區段的（teach 的 setEditTestZSafePos）列進 pending
        for wid, a in sorted(row.get('ambiguous', {}).items()):
            pending[wid] = ('system:' + a['sysfile'], '/'.join(b[0] for b in a['binds']),
                            a['binds'][0][1], a['why'] + '；golden 同一個值寫兩個區段，一個 id 對不了兩個鍵')

        # Steven 20260916：非文字控制項 -> sysEnums
        KIND = {'fieldset': 'index', 'select': 'index', 'label': 'bool'}
        for wid, h in sorted(row.get('props', {}).items()):
            kind = KIND.get(h.get('html'))
            if not kind:
                continue
            f = target(h)
            if not f:
                continue
            probe = (h['section'].lower(), h['key'].lower())
            idx = index_of(f)
            if probe in idx:
                sysenums[wid] = (f, idx[probe][0], idx[probe][1], kind)
            else:
                sysabsent[wid] = (f, h['section'], h['key'])
        items = []
    else:
        items = [] if page in KB_ONLY else sorted(row['fields'].items())
    for wid, h in items:
        doc = api_doc(h.get('doc'))
        if h.get('safe') and doc:
            fields[wid] = (doc, h['section'], h['key'])
        elif doc and h.get('inrecipes', 0) > 0:
            # partial：這個鍵確實存在於部分配方。良性的 notFound，引擎會在存檔時
            # 只送「目前配方真的有」的鍵。抽取錯誤（inrecipes==0）不會走到這裡。
            optional[wid] = (doc, h['section'], h['key'],
                             '%s/%s 個配方有' % (h.get('inrecipes'), h.get('denom')))
        else:
            why = ('%s 只有 %s/%s 個配方有' % (h.get('doc') or '(不在任何配方)',
                                              h.get('inrecipes', 0), h.get('denom', 0))
                   if h.get('doc') else '這個鍵在任何配方檔裡都不存在（抽取可能有誤）')
            if h.get('doc') and not doc:
                why = '文件 %s 不在伺服器認得的清單裡' % h['doc']
            pending[wid] = (doc or (h.get('doc') or '?'), h['section'], h['key'], why)

    kb = {}
    dropped = 0
    for wid, k in sorted(row.get('keyboards', {}).items()):
        dp = k['dp'] if k['dp'] not in (None, '') else 0
        mn = k['min'] if k['min'] not in (None, '') else 0
        mx = k['max'] if k['max'] not in (None, '') else 0
        cr = bool(k['checkRange'])
        # golden myQwertyKeyBoard.cpp:252 遇到 min<0 或 max<=0 會把整組範圍丟掉，
        # 改成 0~65535。HW.teach 的行程值是負的（-500~-7000），照抄那條規則會讓
        # 任何負值被夾成 0 —— 操作員反而打不進正確的教導值。
        # 所以這類一律關掉夾限，不照抄也不自己發明一個。
        try:
            if cr and (float(mn) < 0 or float(mx) <= 0):
                cr, mn, mx, dropped = False, 0, 0, dropped + 1
        except (TypeError, ValueError):
            cr, mn, mx = False, 0, 0
        kb[wid] = (k['flags'], dp, cr, mn, mx)

    L = []
    L.append('/* ht9045_wire_%s.js -- %s 的接線資料（行為在 ht9045_wire_engine.js）' % (slug, page))
    L.append(' * ---------------------------------------------------------------------------')
    L.append(' * //Steven 20260916  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）')
    L.append(' * 當日完整變更紀錄：D:' + chr(92) + 'docs' + chr(92) + 'ChangeLog' + chr(92)
             + 'CHANGES_20260916_Steven.md')
    L.append(' * ---------------------------------------------------------------------------')
    L.append(' * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。')
    L.append(' * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。')
    L.append(' *')
    L.append(' * 來源表單    %s' % (row.get('form') or '?'))
    L.append(' * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog')
    L.append(' *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，')
    L.append(' *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。')
    L.append(' *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），')
    L.append(' *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。')
    L.append(' * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)')
    L.append(' *             配上 .dfm 的 OnMouseDown = <handler>。')
    L.append(' *')
    if page in GRID_PAGES:
        # Steven 20260916
        g = GRID_PAGES[page]
        L.append(' * 表格頁：golden 是 TStringGrid，整個 %s 載進格子、雙擊改、Save 整張寫回。' % g['file'])
        L.append(' *   這裡走引擎的 sysGrid 模式（表格 = 檔案），不是逐格 id 對照；')
        L.append(' *   Save 只送有改過的格子，鍵欄唯讀，legacy 的 JSON 快照 grid 被藏起來。')
    elif page in KB_ONLY:
        # Steven 20260916
        # 原本這三行是寫死的 Config.Configuration 數字（1011/1008），被複製到
        # 每一個 KB_ONLY 頁的檔頭，造成 HW.HandlerSys 的檔頭寫著別頁的統計。
        # 改成用量的：抽到幾個就寫幾個。
        L.append(' * ⚠ 這一頁不是配方頁：它讀的是機台層設定檔，不是作用中配方。')
        L.append(' *   抽取到 %d 個欄位，其中 %d 個在任何配方檔裡都不存在。'
                 % (len(row['fields']),
                    sum(1 for h in row['fields'].values() if not h.get('doc'))))
        L.append(' *   所以本檔只提供小鍵盤，不提供讀寫。')
    if sysfields or sysenums:
        # Steven 20260916：一頁可能橫跨多個系統檔，所以列出實際用到的那幾個。
        used = sorted(set([v[0] for v in sysfields.values()] +
                          [v[0] for v in sysenums.values()]))
        L.append(' * 機台設定檔（/api/system/）%s' % '、'.join(used))
        L.append(' *   文字欄位     %d 個（逐鍵比對過實檔確認存在）' % len(sysfields))
        L.append(' *   非文字控制項 %d 個（radio group / checkbox / combo）' % len(sysenums))
        if sysabsent:
            L.append(' *   契約有、本機實檔沒有 %d 個（見檔尾 SYS ABSENT）' % len(sysabsent))
    L.append(' * 必接 %d 個（該鍵在所有擁有該文件的配方裡都存在）' % len(fields))
    L.append(' * 選用 %d 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）' % len(optional))
    L.append(' * 未接 %d 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）' % len(pending))
    L.append(' * 小鍵盤 %d 個欄位有 golden 依據' % len(kb))
    if dropped:
        L.append(' * ⚠ 其中 %d 個的 golden 範圍是負值或 max<=0。golden 的' % dropped)
        L.append(' *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，')
        L.append(' *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，')
        L.append(' *   欄位仍可輸入，只是少一道範圍檢查。')
    L.append(' */')
    L.append('HT9045Wire.register({')
    L.append('  page: %s,' % js_str(page))
    L.append('  slug: %s,' % js_str(slug))
    L.append('  fields: {')
    for wid, (d, s, k) in fields.items():
        L.append('    %-28s [%s, %s, %s],' % (wid + ':', js_str(d), js_str(s), js_str(k)))
    L.append('  },')
    if sysfields:
        L.append('  sysFields: {')
        for wid, (f, sec, k) in sysfields.items():
            L.append('    %-30s [%s, %s, %s],' % (wid + ':', js_str(f), js_str(sec), js_str(k)))
        L.append('  },')
    # Steven 20260916
    if sysenums:
        L.append('  sysEnums: {')
        for wid, (f, sec, k, kind) in sysenums.items():
            L.append('    %-30s [%s, %s, %s, %s],'
                     % (wid + ':', js_str(f), js_str(sec), js_str(k), js_str(kind)))
        L.append('  },')
    L.append('  optional: {')
    for wid, (d, sec, k, cov) in optional.items():
        L.append('    %-28s [%s, %s, %s],   // %s' % (wid + ':', js_str(d), js_str(sec), js_str(k), cov))
    L.append('  },')
    # Steven 20260916：頁面自己的存檔/重讀鈕（引擎不再注入浮動鈕）
    if page in BTN_PAGES:
        sb, rb, why = BTN_PAGES[page]
        L.append('  // 存檔鈕依據：%s' % why)
        L.append('  saveBtn: %s,' % js_str(sb))
        if rb:
            L.append('  reloadBtn: %s,' % js_str(rb))
    # Steven 20260916：執行期 tag 顯示（唯讀，不進 save）
    if page in TAG_PAGES:
        L.append('  // 執行期 tag（唯讀顯示）：wb_serve 每 500ms 送 snapshot/patch。')
        L.append('  // null 一律顯示成 "---" —— 那是「不可知」，不是 0。')
        L.append('  tags: {')
        for tag, spec in TAG_PAGES[page].items():
            dp = spec[2] if len(spec) > 2 else None
            L.append('    %-24s [%s, %s%s],   // %s'
                     % (js_str(tag) + ':', js_str(spec[0]), js_str(spec[1]),
                        '' if dp is None else ', %d' % dp, spec[3] if len(spec) > 3 else ''))
        L.append('  },')
    L.append('  pending: {')
    for wid, (d, s, k, why) in pending.items():
        L.append('    %-28s [%s, %s, %s, %s],' % (wid + ':', js_str(d), js_str(s), js_str(k), js_str(why)))
    L.append('  },')
    L.append('  kb: {')
    for wid, (f, dp, cr, mn, mx) in kb.items():
        L.append('    %-28s [%s, %s, %s, %s, %s],' % (wid + ':', js_str(f), dp,
                                                      'true' if cr else 'false', mn, mx))
    # Steven 20260916：表格頁
    if page in GRID_PAGES:
        g = GRID_PAGES[page]
        L.append('  },')
        L.append('  // 表格模式：/api/system/%s 整張畫進 #%s，雙擊格子改、Save 只送改過的格子。' % (g['file'], g['host']))
        L.append('  sysGrid: {')
        L.append('    file: %s, host: %s, save: %s, reload: %s,' % (js_str(g['file']), js_str(g['host']),
                                                                  js_str(g['save']), js_str(g['reload'])))
        L.append('    add: %s, del: %s,' % (js_str(g.get('add', '')), js_str(g.get('delete', ''))))
        if g.get('keyPattern'):
            L.append('    keyPattern: %s,' % js_str(g['keyPattern']))
        L.append('    readOnly: %s,' % js_lit(g['readOnly']))
        L.append('    kb: %s,' % js_lit(g['kb']))
        L.append('    filters: %s' % js_lit(g['filters']))
        L.append('  }')
    else:
        L.append('  }')
    L.append('});')
    L.append('')
    if pending:
        L.append('/* ---------------------------------------------------------------------------')
        L.append(' * 沒接的 %d 個，以及為什麼' % len(pending))
        L.append(' * ---------------------------------------------------------------------------')
        L.append(' * 這些鍵在「全部 216 個配方」裡一次都沒出現過 —— 那不是配方相依，')
        L.append(' * 是抽取器抽錯了（例如 golden 那裡是前綴+後綴串接組成的鍵，正則只')
        L.append(' * 抓到字面前綴）。接下去會讓 preview 回 notFound 並被規則 2 擋下，')
        L.append(' * 更糟的情況是誤指到另一個真實存在的鍵，把值寫錯地方。')
        L.append(' * 「部分配方才有」的鍵不在這裡 —— 它們在上面的 optional。')
        L.append(' *')
        for wid, (d, s, k, why) in pending.items():
            L.append(' *   %-24s [%s] %s' % (wid, s, k))
            L.append(' *       %s' % why)
        L.append(' * --------------------------------------------------------------------------- */')
        L.append('')
    # Steven 20260916
    if sysabsent:
        L.append('/* --- SYS ABSENT ------------------------------------------------------------')
        L.append(' * 對照表沒問題（來源是 golden 同一行的 WriteIniData*），但這台機器的')
        L.append(' * 實體設定檔裡目前沒有這個鍵 —— 多半是機型沒有那些 site/軸。')
        L.append(' * 與 PENDING 的差別：PENDING 是抽取抽錯，接了會寫錯地方；')
        L.append(' * 這裡只是本機缺鍵，上機台後鍵存在就該接上，重跑產生器即可。')
        L.append(' *')
        for wid, (f, sec, k) in sorted(sysabsent.items()):
            L.append(' *   %-26s %-10s [%s] %s' % (wid, f, sec, k))
        L.append(' * --------------------------------------------------------------------------- */')
        L.append('')
    return (slug, '\n'.join(L), len(fields) + len(sysfields) + len(sysenums),
            len(optional), len(pending), len(kb))


def main():
    src = os.path.join(HERE, os.environ.get('GEN_IN', 'v2all.json'))
    rows = json.load(open(src, encoding='utf-8'))['rows']
    made = []
    for row in rows:
        nf = sum(1 for h in row['fields'].values() if h.get('safe'))
        # optional（部分配方才有）也算「有東西可接」—— Ld_ULd 就是全部落在這一類。
        no = sum(1 for h in row['fields'].values()
                 if not h.get('safe') and h.get('doc') and h.get('inrecipes', 0) > 0
                 and api_doc(h.get('doc')))
        nk = len(row.get('keyboards', {}))
        if row['page'] in SYS_PAGES:
            nf = 1                      # 交給 emit() 決定，不在這裡跳過
        if nf == 0 and no == 0 and nk == 0:
            continue                      # 沒東西可接也沒鍵盤，跳過
        slug, text, a, o, b, c = emit(row)
        out = os.path.join(CLIENT, 'ht9045_wire_%s.js' % slug)
        open(out, 'w', encoding='utf-8', newline='').write(text)
        made.append((row['page'], slug, a, o, b, c))
    made.sort(key=lambda t: t[2] + t[3])
    print('%-26s %-16s %6s %8s %8s %6s' % ('page', 'slug', 'must', 'optional', 'pending', 'kb'))
    for p, s, a, o, b, c in made:
        print('%-26s %-16s %6d %8d %8d %6d' % (p, s, a, o, b, c))


main()
