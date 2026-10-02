# -*- coding: utf-8 -*-
"""
honprec-blue-expandable-template / template.py
多層次可展開鴻勁藍 HTML 報告 - 基礎生成腳本
用法：
  1. 複製本檔至 D:\AI_TempFile\your_report.py
  2. 替換 ─── Data ─── 區塊中的資料
  3. python your_report.py
  4. 在 OUT_PATH 位置取得 HTML
"""
import os

# ─── 路徑設定 ─────────────────────────────────────────────────────────────────
LOGO_PATH = r'D:\AI_TempFile\_logo_b64_arch.txt'
OUT_PATH  = u'D:\\00_ReleaseNote\\\u5ee0\u5167\\0000_HonPrec\\other\\2026\\report.html'

# Logo Base64（若檔案不存在則使用空白）
logo_b64 = ''
if os.path.exists(LOGO_PATH):
    logo_b64 = open(LOGO_PATH, 'r').read().strip()

# ─── Data（替換這裡）──────────────────────────────────────────────────────────
# 報告基本資訊
REPORT_TITLE    = u'報告標題'
REPORT_SUBTITLE = u'副標題描述'
REPORT_DATE     = u'2026-04-14'
REPORT_AUTHOR   = u'Steven'
REPORT_VERSION  = u'v1.0'

# 統計卡片資料：(數字, 說明)
STATS = [
    ('4', u'Agents'),
    ('8', u'Skills'),
    ('2', u'Projects'),
    ('1', u'Version'),
]

# ─── Skill Data（替換為你的技能清單）────────────────────────────────────────
# 格式：(編號, skill_id, 類別代碼, 說明, 參考檔案清單)
# 類別代碼可用：flow / hw / config / comm / qa / merge / mgmt / core / art
SAMPLE_SKILLS = [
    ('1', 'skill-name-1', 'flow', u'技能說明 1', u'ref1.md · ref2.md'),
    ('2', 'skill-name-2', 'config', u'技能說明 2', u'ref3.md'),
    ('3', 'skill-name-3', 'comm', u'技能說明 3', u'ref4.md · ref5.xlsx'),
]

# 其他群組可依需求新增
# GROUP2_SKILLS = [...]

# ─── 版本記錄 ─────────────────────────────────────────────────────────────────
VERSION_ROWS = [
    (u'2026-04-14', u'初始版本建立'),
]

# ─── Helpers ──────────────────────────────────────────────────────────────────
CAT_LABELS = {
    'flow'  : 'Flow',
    'hw'    : 'HW',
    'config': 'Config',
    'comm'  : 'Comm',
    'qa'    : 'QA',
    'merge' : 'Merge',
    'mgmt'  : 'Mgmt',
    'core'  : 'Core',
    'art'   : 'ART',
}

def ej(s):
    """Escape for JS double-quoted string."""
    return s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n').replace("'", "\\'")

def eh(s):
    """Simple HTML escape."""
    return s.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;').replace('"', '&quot;')

def make_js_data(*skills_lists):
    rows = []
    for lst in skills_lists:
        for (num, sid, cat, desc, refs) in lst:
            cat_label = CAT_LABELS.get(cat, cat)
            rows.append('  "%s":{cat:"%s",catLabel:"%s",desc:"%s",refs:"%s"}' % (
                ej(sid), cat, cat_label, ej(desc), ej(refs)
            ))
    return 'const SKILL_DATA = {\n' + ',\n'.join(rows) + '\n};'

def chip_btn(sid, cat):
    cat_label = CAT_LABELS.get(cat, cat)
    return ('<button class="chip %s" data-skill="%s" onclick="chipClick(this,\'%s\')" title="點擊展開知識庫">'
            '<span class="cat-tag">%s</span> %s</button>' % (cat, ej(sid), ej(sid), cat_label, eh(sid)))

def tbl_rows(rows, alt_start=0):
    out = []
    for i, r in enumerate(rows):
        cls = ' class="alt"' if (i + alt_start) % 2 == 1 else ''
        cells = ''.join('<td>%s</td>' % c for c in r)
        out.append('<tr%s>%s</tr>' % (cls, cells))
    return '\n'.join(out)

def tbl_header(*cols):
    cells = ''.join('<th>%s</th>' % c for c in cols)
    return '<tr>%s</tr>' % cells

def make_section_header(layer_num, title, subtitle=''):
    sub = ('<div class="sec-sub">%s</div>' % subtitle) if subtitle else ''
    return '''<div class="section-header">
  <div class="layer-badge">%s</div>
  <div class="sec-titles"><div class="sec-title">%s</div>%s</div>
</div>''' % (layer_num, title, sub)

def make_dom(dom_id, title, subtitle, skills, info_html=''):
    """Domain block with chip grid (for flat skill lists)."""
    chips = '\n    '.join(chip_btn(s[1], s[2]) for s in skills)
    return '''<div class="dom" id="dom-%(id)s">
  <div class="dom-hd">
    <div class="dom-title">%(title)s</div>
    <div class="dom-meta">%(subtitle)s</div>
  </div>
  %(info)s
  <div class="chip-grid" id="cg-%(id)s">
    %(chips)s
  </div>
  <div class="skill-panel" id="sp-%(id)s" style="display:none">
    <div class="sp-inner"></div>
  </div>
</div>''' % {'id': dom_id, 'title': title, 'subtitle': subtitle, 'info': info_html, 'chips': chips}

def make_project_card(cid, badge, name, workspace, output, version, sub_skills_list):
    """LAYER 2 agent card: expandable header + chip grid inside body."""
    chips = '\n      '.join(chip_btn(s[1], s[2]) for s in sub_skills_list)
    return '''<div class="tool-card" id="tc2-%(cid)s">
  <div class="tool-hd" onclick="toggleCard('%(cid)s')">
    <span class="tool-badge">%(badge)s</span>
    <div class="tool-info">
      <div class="tool-name">%(name)s</div>
      <div class="tool-meta"><code>%(ws)s</code> &rarr; <strong>%(out)s</strong>&nbsp;&nbsp;<span class="ver-tag">%(ver)s</span></div>
    </div>
    <button class="toggle-btn" id="tb-%(cid)s">&#xFF0B;</button>
  </div>
  <div class="tool-body" id="body-%(cid)s" style="display:none">
    <div class="dom-info chip-hint" style="margin-bottom:10px">&#128161; 點擊 Chip 展開 References 知識庫</div>
    <div class="chip-grid" id="cg2-%(cid)s" style="padding:0 0 8px">
      %(chips)s
    </div>
    <div class="skill-panel" id="sp2-%(cid)s" style="display:none">
      <div class="sp-inner"></div>
    </div>
  </div>
</div>''' % {'cid': cid, 'badge': badge, 'name': name, 'ws': workspace,
             'out': output, 'ver': version, 'chips': chips}

def tool_agent_card(name, badge_text, workspace, output, version, desc, detail_html):
    """LAYER 3 detail card: expandable with tables/code inside body."""
    cid = name.lower().replace(' ', '-').replace('_', '-')
    return '''<div class="tool-card" id="tc-%(cid)s">
  <div class="tool-hd" onclick="toggleCard('%(cid)s')">
    <span class="tool-badge">%(badge)s</span>
    <div class="tool-info">
      <div class="tool-name">%(name)s</div>
      <div class="tool-meta"><code>%(ws)s</code> &rarr; <strong>%(out)s</strong>&nbsp;&nbsp;<span class="ver-tag">%(ver)s</span></div>
    </div>
    <button class="toggle-btn" id="tb-%(cid)s">&#xFF0B;</button>
  </div>
  <div class="tool-body" id="body-%(cid)s" style="display:none">
    <div class="tool-desc">%(desc)s</div>
    %(detail)s
  </div>
</div>''' % {'cid': cid, 'badge': badge_text, 'name': name, 'ws': workspace,
             'out': output, 'ver': version, 'desc': desc, 'detail': detail_html}

def make_global_skill_card(cid, badge, title, ws, detail_html):
    """LAYER 4 global skill card: expandable."""
    return '''<div class="tool-card" id="tc4-%(cid)s">
  <div class="tool-hd" onclick="toggleCard('%(cid)s')">
    <span class="tool-badge">%(badge)s</span>
    <div class="tool-info">
      <div class="tool-name">%(title)s</div>
      <div class="tool-meta"><code>%(ws)s</code></div>
    </div>
    <button class="toggle-btn" id="tb-%(cid)s">&#xFF0B;</button>
  </div>
  <div class="tool-body" id="body-%(cid)s" style="display:none">
    %(detail)s
  </div>
</div>''' % {'cid': cid, 'badge': badge, 'title': title, 'ws': ws, 'detail': detail_html}

# ─── CSS ──────────────────────────────────────────────────────────────────────
CSS = """
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'Microsoft JhengHei','\\5FAE\\8EDF\\6B63\\9ED1\\9AD4',Arial,sans-serif;font-size:13px;background:#f4f6fa;color:#1a1a2a;line-height:1.6}
a{color:#005a9e}

/* Header */
.header{background:linear-gradient(135deg,#003e7e 0%,#005a9e 60%,#0078d4 100%);color:#fff;padding:20px 32px;display:flex;align-items:center;gap:20px;box-shadow:0 3px 10px rgba(0,62,126,.4)}
.header img{height:52px;border-radius:4px;background:#fff;padding:4px}
.header-text h1{font-size:20px;font-weight:700;letter-spacing:.5px}
.header-text p{font-size:12px;opacity:.85;margin-top:3px}
.header-meta{margin-left:auto;text-align:right;font-size:11px;opacity:.8;line-height:1.8}

/* Meta block */
.meta-block{background:#fff;border-left:5px solid #003e7e;margin:18px 24px 0;padding:14px 20px;border-radius:0 6px 6px 0;box-shadow:0 1px 4px rgba(0,0,0,.08)}
.meta-block table{width:100%;border-collapse:collapse}
.meta-block td{padding:3px 12px;font-size:12px;vertical-align:top}
.meta-block td:first-child{font-weight:600;color:#003e7e;width:110px;white-space:nowrap}

/* Stats bar */
.stats-bar{display:flex;gap:12px;padding:16px 24px;flex-wrap:wrap}
.stat-card{background:#fff;border:1px solid #d0dff5;border-radius:8px;padding:12px 20px;flex:1;min-width:110px;text-align:center;box-shadow:0 1px 4px rgba(0,62,126,.07);transition:transform .15s}
.stat-card:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(0,62,126,.14)}
.stat-num{font-size:28px;font-weight:700;color:#003e7e;line-height:1}
.stat-label{font-size:11px;color:#667;margin-top:4px}

/* Section header */
.sec-wrap{margin:0 24px 22px}
.section-header{display:flex;align-items:center;gap:14px;margin-bottom:12px;border-bottom:2px solid #003e7e;padding-bottom:8px}
.layer-badge{background:#003e7e;color:#fff;border-radius:50%;width:36px;height:36px;display:flex;align-items:center;justify-content:center;font-size:14px;font-weight:700;flex-shrink:0}
.sec-title{font-size:16px;font-weight:700;color:#003e7e}
.sec-sub{font-size:12px;color:#666;margin-top:2px}

/* Domain blocks */
.dom{background:#fff;border:1px solid #d0dff5;border-radius:8px;margin-bottom:14px;overflow:hidden;box-shadow:0 1px 4px rgba(0,62,126,.06)}
.dom-hd{background:linear-gradient(90deg,#eef3fb,#f7f9fd);padding:10px 16px;border-bottom:1px solid #d0dff5}
.dom-title{font-size:14px;font-weight:700;color:#003e7e}
.dom-meta{font-size:11px;color:#666;margin-top:2px}
.dom-info{padding:8px 16px;background:#fffbf0;border-bottom:1px solid #f0e8c8;font-size:12px;color:#555}

/* Chip grid */
.chip-grid{padding:12px 16px;display:flex;flex-wrap:wrap;gap:7px}

/* Chips */
.chip{display:inline-flex;align-items:center;gap:5px;border:none;border-radius:16px;padding:5px 12px;font-size:12px;font-family:inherit;cursor:pointer;transition:all .15s;font-weight:500;white-space:nowrap}
.chip .cat-tag{background:rgba(0,0,0,.12);border-radius:10px;padding:1px 7px;font-size:10px;font-weight:700;letter-spacing:.3px}
.chip:hover{transform:translateY(-1px);box-shadow:0 3px 8px rgba(0,0,0,.15)}
.chip.active{box-shadow:0 3px 10px rgba(0,0,0,.25);transform:translateY(-1px)}

.chip.flow  {background:#dbeafe;color:#1d4ed8}.chip.flow .cat-tag{background:#1d4ed8;color:#fff}
.chip.flow.active{background:#1d4ed8;color:#fff}
.chip.hw    {background:#ffedd5;color:#9a3412}.chip.hw .cat-tag{background:#ea580c;color:#fff}
.chip.hw.active{background:#ea580c;color:#fff}
.chip.config{background:#dcfce7;color:#166534}.chip.config .cat-tag{background:#16a34a;color:#fff}
.chip.config.active{background:#16a34a;color:#fff}
.chip.comm  {background:#f3e8ff;color:#6b21a8}.chip.comm .cat-tag{background:#9333ea;color:#fff}
.chip.comm.active{background:#9333ea;color:#fff}
.chip.qa    {background:#fee2e2;color:#991b1b}.chip.qa .cat-tag{background:#dc2626;color:#fff}
.chip.qa.active{background:#dc2626;color:#fff}
.chip.merge {background:#fef9c3;color:#854d0e}.chip.merge .cat-tag{background:#d97706;color:#fff}
.chip.merge.active{background:#d97706;color:#fff}
.chip.mgmt  {background:#e0e7ff;color:#3730a3}.chip.mgmt .cat-tag{background:#4338ca;color:#fff}
.chip.mgmt.active{background:#4338ca;color:#fff}
.chip.core  {background:#ccfbf1;color:#134e4a}.chip.core .cat-tag{background:#0d9488;color:#fff}
.chip.core.active{background:#0d9488;color:#fff}
.chip.art   {background:#fef3c7;color:#78350f}.chip.art .cat-tag{background:#f59e0b;color:#fff}
.chip.art.active{background:#f59e0b;color:#fff}

/* Skill detail panel */
.skill-panel{margin:0 16px 14px;border-radius:6px;overflow:hidden;border:1px solid #c7d9f0}
.sp-head{background:#003e7e;color:#fff;padding:10px 14px;display:flex;align-items:center;gap:10px;flex-wrap:wrap}
.sp-head .cat-badge{background:rgba(255,255,255,.2);border:1px solid rgba(255,255,255,.4);border-radius:12px;padding:2px 10px;font-size:11px;font-weight:700}
.sp-head strong{font-size:13px;flex:1}
.sp-desc{font-size:11px;opacity:.85;width:100%;margin-top:2px}
.sp-close{background:rgba(255,255,255,.2);border:none;color:#fff;border-radius:4px;padding:2px 8px;cursor:pointer;font-size:12px;margin-left:auto}
.sp-close:hover{background:rgba(255,255,255,.35)}
.sp-refs-label{background:#eef3fb;padding:6px 14px;font-size:11px;font-weight:600;color:#003e7e;border-bottom:1px solid #d0dff5}
.sp-refs{padding:10px 14px;display:flex;flex-wrap:wrap;gap:5px;background:#fff}
.ref-pill{background:#f0f4fb;border:1px solid #c7d9f0;border-radius:4px;padding:3px 9px;font-size:11px;font-family:'Consolas','Courier New',monospace;color:#1a3a6e;cursor:default}
.ref-pill:hover{background:#dbeafe}

/* Table styles */
table.std{width:100%;border-collapse:collapse;font-size:12px}
table.std th{background:#003e7e;color:#fff;padding:7px 12px;text-align:left;font-weight:600}
table.std td{padding:6px 12px;border-bottom:1px solid #e9eef5;vertical-align:top}
table.std tr:hover td{background:#dce8f8}
table.std tr.alt td{background:#f0f4fb}
table.std tr.alt:hover td{background:#dce8f8}
code.path{background:#f0f4fb;border:1px solid #d0dff5;border-radius:3px;padding:1px 6px;font-size:11px;color:#1a3a6e}

/* Tool agent cards */
.tool-card{border:1px solid #d0dff5;border-radius:8px;margin-bottom:12px;overflow:hidden;background:#fff;box-shadow:0 1px 4px rgba(0,62,126,.06)}
.tool-hd{background:linear-gradient(90deg,#eef3fb,#f7f9fd);padding:12px 16px;display:flex;align-items:center;gap:12px;cursor:pointer;transition:background .15s}
.tool-hd:hover{background:linear-gradient(90deg,#dbe8f8,#eef3fb)}
.tool-badge{background:#003e7e;color:#fff;border-radius:50%;width:40px;height:40px;display:flex;align-items:center;justify-content:center;font-size:15px;font-weight:700;flex-shrink:0}
.tool-info{flex:1}
.tool-name{font-size:14px;font-weight:700;color:#003e7e}
.tool-meta{font-size:11px;color:#666;margin-top:2px}
.ver-tag{background:#e0eeff;border:1px solid #b3cff0;border-radius:10px;padding:1px 8px;font-size:10px;color:#003e7e}
.toggle-btn{background:#003e7e;color:#fff;border:none;border-radius:50%;width:28px;height:28px;font-size:16px;cursor:pointer;line-height:1;transition:transform .2s}
.tool-body{padding:12px 16px 14px;border-top:1px solid #d0dff5}
.tool-desc{font-size:12px;color:#444;margin-bottom:10px;padding:8px 12px;background:#fffbf0;border-left:3px solid #e8c840;border-radius:0 4px 4px 0}
.sub-section{margin-top:10px}
.sub-section h4{font-size:12px;font-weight:700;color:#003e7e;margin-bottom:6px;border-bottom:1px dashed #d0dff5;padding-bottom:4px}
.coupling-box{background:#1a1a2e;color:#7dd3fc;font-family:'Consolas','Courier New',monospace;font-size:11px;padding:12px 16px;border-radius:6px;margin:6px 0;white-space:pre;overflow-x:auto}

/* Sync visual */
.sync-card{background:#fffbf0;border:1px solid #e8c840;border-radius:8px;padding:14px 18px;margin-bottom:12px}
.sync-card h3{font-size:13px;font-weight:700;color:#92400e;margin-bottom:10px}
.sync-visual{display:flex;flex-direction:column;gap:12px}
.sync-nodes{display:flex;align-items:center;gap:0;flex-wrap:wrap;justify-content:center}
.sync-node{background:#fff;border:2px solid #003e7e;border-radius:10px;padding:10px 16px;min-width:130px;text-align:center;box-shadow:0 2px 6px rgba(0,62,126,.12)}
.sync-node-badge{display:inline-block;background:#003e7e;color:#fff;border-radius:50%;width:30px;height:30px;line-height:30px;font-size:12px;font-weight:700;margin-bottom:4px}
.sync-node-label{font-size:12px;font-weight:700;color:#003e7e}
.sync-node-sub{font-size:10px;color:#666;margin-top:3px;font-family:'Consolas',monospace;line-height:1.4}
.nd-ht{border-color:#1d4ed8}.nd-ht .sync-node-badge{background:#1d4ed8}
.nd-gp{border-color:#0d9488}.nd-gp .sync-node-badge{background:#0d9488}
.nd-rs{border-color:#9333ea}.nd-rs .sync-node-badge{background:#9333ea}
.sync-arrows{display:flex;flex-direction:column;align-items:center;justify-content:center;padding:0 6px;color:#003e7e;font-size:18px;font-weight:700;line-height:1;gap:2px}
.sync-hub{display:flex;justify-content:center}
.sync-hub-box{background:#f0f4fb;border:2px dashed #003e7e;border-radius:10px;padding:10px 20px;text-align:center;width:100%}
.sync-hub-title{font-size:12px;font-weight:700;color:#003e7e;margin-bottom:8px}
.sync-hub-items{display:flex;gap:8px;justify-content:center;flex-wrap:wrap}
.sync-item{display:inline-block;border-radius:8px;padding:6px 14px;font-size:11px;font-weight:700;text-align:center;line-height:1.4}
.sync-item small{font-weight:400;font-size:10px}
.si-cc  {background:#dbeafe;color:#1d4ed8;border:1px solid #93c5fd}
.si-msg {background:#dcfce7;color:#166534;border:1px solid #86efac}
.si-mode{background:#f3e8ff;color:#6b21a8;border:1px solid #d8b4fe}

/* SVN flow */
.svn-flow{display:flex;align-items:center;gap:12px;flex-wrap:wrap}
.svn-sources{display:flex;flex-direction:column;gap:5px;min-width:160px}
.svn-src-label{font-size:10px;font-weight:700;color:#666;text-transform:uppercase;letter-spacing:.5px;margin-bottom:2px}
.svn-src-box{display:flex;align-items:center;gap:6px;background:#fff;border:1px solid #d0dff5;border-radius:6px;padding:4px 10px;font-size:11px;font-family:'Consolas',monospace}
.svn-dot{display:inline-block;width:8px;height:8px;border-radius:50%;flex-shrink:0}
.sd-ht{background:#1d4ed8}.sd-gp{background:#0d9488}.sd-es{background:#dc2626}
.sd-el{background:#d97706}.sd-rs{background:#9333ea}
.svn-arrow{font-size:24px;color:#003e7e;font-weight:700;padding:0 4px}
.svn-right{display:flex;flex-direction:column;align-items:center;gap:6px;flex:1;min-width:180px}
.svn-commit-box{background:#003e7e;color:#fff;border-radius:8px;padding:10px 20px;text-align:center;width:100%}
.svn-commit-title{font-size:13px;font-weight:700}
.svn-commit-sub{font-size:10px;opacity:.8;margin-top:2px}
.svn-arrow-down{font-size:20px;color:#003e7e}
.svn-report-box{background:#fff;border:2px solid #003e7e;border-radius:8px;padding:10px 16px;text-align:center;width:100%}
.svn-report-title{font-size:12px;font-weight:700;color:#003e7e;margin-bottom:7px}
.svn-report-tags{display:flex;gap:5px;flex-wrap:wrap;justify-content:center}
.svn-tag{background:#eef3fb;border:1px solid #c7d9f0;border-radius:4px;padding:2px 8px;font-size:10px;color:#003e7e;font-weight:600}

/* Dispatch / version tables */
.dispatch-wrap table.std th{background:#005a9e}
.dispatch-wrap tr td:first-child{font-weight:500;color:#003e7e}
.ver-wrap tr.alt td{background:#f0f4fb}
.ver-wrap tr:last-child td{font-weight:600;color:#003e7e;background:#e8f0fc}

/* Footer */
.footer{text-align:center;padding:18px;font-size:11px;color:#999;border-top:1px solid #e0e8f0;margin-top:10px}
.chip-hint{font-size:10px;color:#999;padding:4px 16px;margin-top:-4px}
"""

# ─── JavaScript ───────────────────────────────────────────────────────────────
JS = """
/* Unified chip click: works inside .dom, .tool-body, or anywhere */
function chipClick(btn, skillId) {
    var container = btn.parentElement;           /* .chip-grid */
    if (container) container = container.parentElement;  /* actual container */
    if (!container) return;
    var panel = container.querySelector('.skill-panel');
    if (!panel) return;
    var inner = panel.querySelector('.sp-inner');
    if (!inner) return;

    var allChips = container.querySelectorAll('.chip');
    var isActive = btn.classList.contains('active');
    allChips.forEach(function(c){ c.classList.remove('active'); });

    if (isActive) { panel.style.display = 'none'; return; }

    btn.classList.add('active');
    var data = SKILL_DATA[skillId];
    if (!data) { panel.style.display = 'none'; return; }

    var refParts = data.refs.split(' \\u00b7 ');
    var refHtml = refParts.map(function(r){
        return '<span class="ref-pill">' + r.trim() + '</span>';
    }).join('');

    inner.innerHTML =
        '<div class="sp-head">'
        + '<span class="cat-badge">' + data.catLabel + '</span>'
        + '<strong>' + skillId + '</strong>'
        + '<button class="sp-close" onclick="closeSkillPanel(this)">&#10005; \\u95dc\\u9589</button>'
        + '<div class="sp-desc">' + data.desc + '</div>'
        + '</div>'
        + '<div class="sp-refs-label">&#128196; References \\u77e5\\u8b58\\u5eab</div>'
        + '<div class="sp-refs">' + refHtml + '</div>';

    panel.style.display = 'block';
    panel.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
}

/* Close skill panel */
function closeSkillPanel(btn) {
    var panel = btn.closest('.skill-panel');
    if (!panel) return;
    panel.style.display = 'none';
    var container = panel.parentElement;
    if (container) {
        container.querySelectorAll('.chip.active').forEach(function(c){
            c.classList.remove('active');
        });
    }
}

/* Card toggle: handles body- and body2- prefixes */
function toggleCard(cid) {
    var body = document.getElementById('body-' + cid)
            || document.getElementById('body2-' + cid);
    var tbtn = document.getElementById('tb-' + cid)
            || document.getElementById('tb2-' + cid);
    if (!body) return;
    var isOpen = body.style.display !== 'none';
    body.style.display = isOpen ? 'none' : 'block';
    if (tbtn) tbtn.textContent = isOpen ? '\\uff0b' : '\\uff0d';
}

function toggleCard2(cid) { toggleCard(cid); }
"""

# ─── Build Functions（替換為你的內容）────────────────────────────────────────

def build_header():
    logo_tag = ''
    if logo_b64:
        logo_tag = '<img src="data:image/png;base64,%s" alt="Logo">' % logo_b64
    return '''<div class="header">
  %s
  <div class="header-text">
    <h1>%s</h1>
    <p>%s</p>
  </div>
  <div class="header-meta">版本：%s<br>日期：%s<br>作者：%s</div>
</div>''' % (logo_tag, REPORT_TITLE, REPORT_SUBTITLE, REPORT_VERSION, REPORT_DATE, REPORT_AUTHOR)

def build_stats():
    cards = ''.join(
        '<div class="stat-card"><div class="stat-num">%s</div><div class="stat-label">%s</div></div>' % (n, l)
        for n, l in STATS
    )
    return '<div class="stats-bar">%s</div>' % cards

def build_layer1_sample():
    """示例：LAYER 1 — 簡單資料表"""
    rows = [
        (u'Agent A', u'工作區路徑', u'說明文字 A'),
        (u'Agent B', u'工作區路徑', u'說明文字 B'),
    ]
    return '''<div class="sec-wrap">
%s
<table class="std">
<thead>%s</thead>
<tbody>%s</tbody>
</table>
</div>''' % (
        make_section_header('1', u'LAYER 1 — 範例層次', u'替換為你的層次說明'),
        tbl_header(u'名稱', u'工作區', u'說明'),
        tbl_rows(rows)
    )

def build_layer2_sample():
    """示例：LAYER 2 — 可展開 Project 卡片"""
    card = make_project_card(
        'project-a', 'PA', u'Project A',
        u'D:\\Projects\\ProjectA', u'ProjectA.exe', u'3 Sub-Skills',
        SAMPLE_SKILLS
    )
    return '''<div class="sec-wrap">
%s
%s
</div>''' % (
        make_section_header('2', u'LAYER 2 — 專案 Agents', u'點擊卡片展開子技能 Chip'),
        card
    )

def build_version_table():
    return '''<div class="sec-wrap ver-wrap">
%s
<table class="std">
<thead>%s</thead>
<tbody>%s</tbody>
</table>
</div>''' % (
        make_section_header(u'V', u'版本記錄'),
        tbl_header(u'日期', u'變更說明'),
        tbl_rows(VERSION_ROWS)
    )

# ─── Main Build ───────────────────────────────────────────────────────────────

def build():
    js_data = make_js_data(SAMPLE_SKILLS)  # 加入所有技能清單

    sections = '\n'.join([
        build_header(),
        build_stats(),
        build_layer1_sample(),
        build_layer2_sample(),
        build_version_table(),
    ])

    html = u'''<!DOCTYPE html>
<html lang="zh-TW">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>%(title)s</title>
<style>%(css)s</style>
</head>
<body>
%(sections)s
<div class="footer">%(title)s &nbsp;|&nbsp; %(date)s &nbsp;|&nbsp; %(author)s</div>
<script>%(js_data)s</script>
<script>%(js)s</script>
</body>
</html>''' % {
        'title': REPORT_TITLE,
        'css': CSS,
        'sections': sections,
        'date': REPORT_DATE,
        'author': REPORT_AUTHOR,
        'js_data': js_data,
        'js': JS,
    }

    out_dir = os.path.dirname(OUT_PATH)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir)
    with open(OUT_PATH, 'w', encoding='utf-8') as f:
        f.write(html)
    print('Output: %s (%d bytes)' % (OUT_PATH, len(html.encode('utf-8'))))

if __name__ == '__main__':
    build()
