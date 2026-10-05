# -*- coding: utf-8 -*-
r"""
Generate HT9011UC iosetview.dfm Alias annotation document (self-contained HTML).
Parses the DFM, computes absolute pixel positions of TMyLedLane / TBtnPanelLane
components, and overlays annotation boxes on the 1:1 screenshots in D:\HT9045\IMG\IO.
"""
import re, os, base64, json, sys, shutil

def _portal_docs(sub, old):
    """St02 20261005 (Steven: documents go to the RD5 portal): <portal repo>\\public\\Docs\\<sub>.
    Portal repo = env RD5_PORTAL_REPO, else D:\\RD5-Portal (St01 / St02), else D:\\HT9045-Index (laptop);
    none of them there -> the old D:\\docs location, with a warning."""
    for root in (os.environ.get("RD5_PORTAL_REPO"), r"D:\RD5-Portal", r"D:\HT9045-Index"):
        if root and os.path.isdir(os.path.join(root, "public", "Docs")):
            return os.path.join(root, "public", "Docs", sub)
    print("WARNING: RD5 portal repo not found (set RD5_PORTAL_REPO) -> old location " + old, file=sys.stderr)
    return old

DFM   = r"D:\HT9045\HT9011UC_Code_V3.33.908.0_20260702\iosetview.dfm"
IMGDIR= r"D:\HT9045\IMG\IO"
OUT   = _portal_docs(r"manual\HT9011UC_IOSetView_Alias_Map.html", r"D:\docs\manual\HT9011UC_IOSetView_Alias_Map.html")
CSV_SRC = r"D:\HT9045\system\IO_Table.csv"        # machine master copy
CSV_DST = os.path.join(os.path.dirname(OUT), "IO_Table.csv")          # copied next to HTML (if absent)

TARGET_TYPES = ("TMyLedLane", "TBtnPanelLane")
# TTL page uses plain TMyLed / TBtnPanel (with Alias) - collected only under tsTTL
TTL_TYPES = ("TMyLed", "TBtnPanel")

# components excluded from the document (deleted on request)
EXCLUDE_COMPS = {"btnSwEpArm2"}

# region-capture pages: screenshot covers only one container (1:1), not the whole
# window. page_key -> anchor container name; marker pos = comp_abs - anchor_abs
REGION_PAGES = {
    "grpManual":         "grpManual",   # Stack1 EMPTY & COLOR TRAY group (898x413)
    "tsTrayArm":         "gbTrayArm",   # Vacuum-Others Tray Arm sub-tabs (298x340)
    "tsRTArm":           "gbTrayArm",
    "tsUnderArm":        "gbTrayArm",
    "tsStack1_Cassette": "pgcStack1",   # whole PageControl capture (906x797)
    "tsStack2_Cassette": "pgcStack2",
}

# ---- calibration constants (image pixel space) -----------------------------
BORDER_X = 8      # window left border in screenshot
TITLE_Y  = 31     # title bar + top border height

# per-page fine-tune offset (dx, dy) in image pixels, applied to ALL markers
# of that page. e.g. "tsIndex": (3, -2) => all tsIndex boxes shift right 3, up 2
PAGE_ADJ = {
    "tsStack1_Above": (-6, -6),
    "tsStack2_Above": (-6, -6),
    "tsStack3_Above": (-6, -6),
    "tsStack1_Under": (-6, -6),
    "tsStack2_Under": (-6, -6),
    "tsStack3_Under": (-6, -6),
    "tsFix":          (-6,  6),
    "tsInArmVacuum":  (-6, 10),
    "tsIndexVacuum":  (-4, 17),
    "tsOutArmVacuum": (-6, 15),
    "tsOtherVacuum":  (-8, -9),
    "tsShuttle":      (-3, -13),
    "tsKeyPad":       (-2, -10),
    "tsSystem":       (-15, -20),
    "tsIndex":        (-3, -13),
    "tsATC":          (-5, -13),
    "tsTTL":          (0, 11),
    "grpManual":      (-2, -22),
    "tsStack1_Cassette": (-2, -15),
    "tsStack2_Cassette": (-2, -15),
    "tsTrayArm":      (-2, -15),
    "tsRTArm":        (-2, -15),
    "tsUnderArm":     (-2, -15),
    "tsAGV":          (-4, -23),
    "tsMagazine":     (-2, -2),
}

# per-page numbered-range fine-tune: page -> list of (no_from, no_to, dx, dy)
# applies to markers whose table number (#) falls in [no_from, no_to]
PAGE_NO_ADJ = {
    "tsInArmVacuum": [
        (1, 73,  0,   4),   # #1~#73
        (74, 75, -1,  5),   # #74~#75
        (76, 88, -2, -19),  # #76~#88
        (89, 97, -4, -42),  # #89~#97 (prev -3,-39, this time -1,-3)
    ],
    "tsOtherVacuum": [
        # (old entries removed 2026-07-06: all 14 adjusted items were RT Arm
        #  sub-tab components, now separated into their own region pages)
    ],
    "tsSystem": [
        (1, 19, 11, 28),                   # top EMG / safe-door row (runtime reflow)
        (21, 21, 2, 18), (24, 24, 2, 18),  # right status column (x=675)
        (29, 29, 2, 18), (32, 32, 2, 18),
        (37, 37, 2, 18), (41, 41, 2, 18),
        (46, 46, 2, 18), (51, 51, 2, 18),
        (54, 54, 2, 18), (59, 59, 2, 18),
        (62, 62, 2, 18), (65, 65, 2, 18),
        (78, 78, 1, -4), (87, 87, 1, -4), (92, 92, 1, -4),   # Tower Light
        (96, 96, 1, 0), (101, 101, 1, 0),                     # Music
        (107, 108, 1, 0),
        (111, 112, 1, -2),                                    # Motor Power
        (113, 116, 11, 28),                                   # bottom EMG row
        (57, 58, 1, -1),                                      # Index Ion Fan / Fan Clean
        (60, 61, 4, 21), (66, 67, 4, 21),                     # center button column
        (74, 75, 4, 21), (81, 82, 4, 21),
        (93, 93, 2, -1),                                      # Smoke Detect
        (104, 105, 1, -2),                                    # Tester Dry Air
        (109, 110, 0, -2),                                    # All SafeDoor / All EMG
        (94, 95, 1, -2), (97, 98, 1, -2),                     # Ground Man LEDs
        (36, 36, 0, -3), (40, 40, 0, -3), (44, 44, 0, -3),    # OCR column
        (49, 50, 0, -3), (55, 56, 0, -3),
        (20, 20, 8, 25), (22, 22, 8, 25), (35, 35, 8, 25),    # left safe-door strip
        (63, 63, 8, 25), (68, 68, 8, 25), (80, 80, 8, 25),
        (85, 85, 8, 25), (99, 99, 8, 25), (102, 102, 8, 25),
        (106, 106, 8, 25),
        (23, 23, 8, 25), (45, 45, 8, 25), (64, 64, 8, 25),    # right safe-door strip
        (71, 71, 8, 25), (79, 79, 8, 25), (86, 86, 8, 25),
        (100, 100, 8, 25), (103, 103, 8, 25),
    ],
    "tsATC": [
        (1, 12, 0, 4),   # top Temperature Over Detect LEDs (already aligned, offset page adj)
        (3, 4, 1, -2), (7, 8, 1, -2),
        (1, 2, 0, -2), (5, 6, 0, -2), (9, 12, 0, -2),
    ],
    "tsShuttle": [
        (3, 4, -3, -22),
    ],
    "tsIndex": [
        (76, 77, 0, 20), (82, 85, 0, 20), (88, 89, 0, 20),
        (93, 94, 0, 20), (100, 100, 0, 20), (105, 106, 0, 20),
        (111, 112, 0, 20),
        (78, 79, 0, -20), (86, 87, 0, -20), (90, 91, 0, -20),  # docking LEDs
        (81, 81, 0, 20),                                       # C_CoolingValve_On LED
        (99, 99, 0, 20),                                       # align with #100
    ],
    "tsStack1_Above": [
        (5, 5, -2, -13), (7, 7, -2, -13), (12, 16, -2, -13),
        (23, 24, -2, -13),
        # (22): -10 then +10 => net 0
    ],
}

# per-component fine-tune: component name -> (dx, dy[, w, h])
# overrides position of a single marker; optional w/h replaces box size
COMP_ADJ = {
    # Color Sensor group (Alias: SnMCUSensor1~4)
    "myldln1": (0, -10),
    "myldln2": (0, -10),
    "myldln3": (0, -10),
    "myldln4": (0, -10),
    # Loader Func group (LoadCar RFID)
    "btnC_LoadCarRFIDRotArmU":    (0, -10),
    "btnC_LoadCarRFIDRotArmD":    (0, -10),
    "ledC_LoadCarRFIDRotArmD_Off":(0, -10),
    "ledC_LoadCarRFIDRotArmD_On": (0, -10),
    "ledSnLoadCarRFIDSW":         (0, -10),
}
# per-PageControl client-area adjust (dx, dy) added to a TabSheet's origin
PGC_ADJ = {
    "PC_IOSET":     (4, 51),   # 2 rows of flat-button tabs (empirically calibrated)
    "pgcStack1":    (4, 25),
    "pgcStack2":    (4, 25),
    "pgcStack3":    (4, 25),
    "pgcVacuum":    (4, 25),
    "pgcTrayArm":   (4, 25),
    "PageControl1": (4, 25),
    "PageControl2": (4, 25),
}
DEFAULT_PGC_ADJ = (4, 25)

# ---- DFM parsing ------------------------------------------------------------
class Node:
    __slots__ = ("name","dtype","props","children","parent")
    def __init__(self, name, dtype, parent):
        self.name=name; self.dtype=dtype; self.parent=parent
        self.props={}; self.children=[]

obj_re  = re.compile(r"^\s*(?:object|inherited)\s+(\w+)\s*:\s*(\w+)")
prop_re = re.compile(r"^\s*([\w.]+)\s*=\s*(.+?)\s*$")

def parse_dfm(path):
    root=None; stack=[]
    in_paren=False; in_bin=False
    with open(path, "r", encoding="cp950", errors="replace") as f:
        for raw in f:
            line=raw.rstrip("\n")
            s=line.strip()
            if in_bin:
                if s.endswith("}"): in_bin=False
                continue
            if in_paren:
                if s.endswith(")"): in_paren=False
                continue
            m=obj_re.match(line)
            if m:
                node=Node(m.group(1), m.group(2), stack[-1] if stack else None)
                if stack: stack[-1].children.append(node)
                else: root=node
                stack.append(node)
                continue
            if s=="item":
                node=Node("<item>","<item>", stack[-1] if stack else None)
                if stack: stack[-1].children.append(node)
                stack.append(node)
                continue
            if s=="end" or s=="end>":
                if stack: stack.pop()
                continue
            pm=prop_re.match(line)
            if pm and stack:
                k,v=pm.group(1),pm.group(2)
                if v=="(" or v=="<":
                    if v=="(": in_paren=True
                    # '<' collections are followed by item/end lines handled above
                    continue
                if v=="{":
                    in_bin=True
                    continue
                stack[-1].props[k]=v
    return root

def prop_int(n,k,default=0):
    v=n.props.get(k)
    if v is None: return default
    try: return int(v)
    except ValueError: return default

def prop_str(n,k):
    v=n.props.get(k,"")
    if v.startswith("'") and v.endswith("'"):
        v=v[1:-1].replace("''","'")
    return v

def font_height(n):
    """effective font height (negative VCL value) with inheritance"""
    cur=n
    while cur is not None:
        if "Font.Height" in cur.props:
            try: return abs(int(cur.props["Font.Height"]))
            except ValueError: pass
        cur=cur.parent
    return 11

def client_offset(n):
    """origin of children inside this container, relative to container Left/Top"""
    t=n.dtype
    if t=="TTabSheet":
        return (0,0)  # handled by PageControl adjust
    if t=="TGroupBox":
        fh=font_height(n)
        return (2, int(fh*1.2)+3)
    if t=="TPanel":
        bw=prop_int(n,"BorderWidth",0)
        bo=n.props.get("BevelOuter","bvRaised")
        bi=n.props.get("BevelInner","bvNone")
        bev=(0 if bo=="bvNone" else 1)+(0 if bi=="bvNone" else 1)
        return (bw+bev, bw+bev)
    if t=="TScrollBox":
        return (2,2)
    return (0,0)

def abs_pos(n):
    """absolute (x,y) of component top-left in FORM CLIENT coords"""
    x=prop_int(n,"Left"); y=prop_int(n,"Top")
    cur=n.parent
    while cur is not None:
        if cur.parent is None:
            break  # form itself: Left/Top is screen position, not client offset
        if cur.dtype=="TTabSheet":
            pgc=cur.parent
            adj=PGC_ADJ.get(pgc.name, DEFAULT_PGC_ADJ) if pgc is not None else DEFAULT_PGC_ADJ
            x+=adj[0]; y+=adj[1]
            # add PageControl own Left/Top then continue above it
            if pgc is not None:
                x+=prop_int(pgc,"Left"); y+=prop_int(pgc,"Top")
                cur=pgc.parent
                continue
        else:
            dx,dy=client_offset(cur)
            x+=dx+prop_int(cur,"Left"); y+=dy+prop_int(cur,"Top")
        cur=cur.parent
    return x,y

# ---- collect components ------------------------------------------------------
IMAGES={fn[:-4] for fn in os.listdir(IMGDIR) if fn.lower().endswith(".png")}

def tab_chain(n):
    """list of TTabSheet ancestor names, outermost first"""
    chain=[]
    cur=n.parent
    while cur is not None:
        if cur.dtype=="TTabSheet": chain.append(cur.name)
        cur=cur.parent
    return list(reversed(chain))

def tab_captions(n):
    caps=[]
    cur=n.parent
    while cur is not None:
        if cur.dtype=="TTabSheet": caps.append(prop_str(cur,"Caption"))
        cur=cur.parent
    return list(reversed(caps))

root=parse_dfm(DFM)
components=[]
def _ancestors(n):
    names=set(); cur=n.parent
    while cur is not None:
        names.add(cur.name); cur=cur.parent
    return names

def walk(n):
    if n.name not in EXCLUDE_COMPS:
        tabs=tab_chain(n)
        hit = n.dtype in TARGET_TYPES or (n.dtype in TTL_TYPES and "tsTTL" in tabs)
        if hit:
            x,y=abs_pos(n)
            components.append({
                "name":n.name, "type":n.dtype,
                "alias":prop_str(n,"Alias"),
                "caption":prop_str(n,"Caption"),
                "x":x, "y":y,
                "w":prop_int(n,"Width",20), "h":prop_int(n,"Height",14),
                "tabs":tabs, "tabcaps":tab_captions(n),
                "anc":_ancestors(n),
            })
    for c in n.children: walk(c)
walk(root)

# anchor nodes for region pages
_anchor_nodes={}
def _find_anchor(n):
    if n.name in set(REGION_PAGES.values()):
        _anchor_nodes[n.name]=n
    for c in n.children: _find_anchor(c)
_find_anchor(root)
ANCHOR_ABS={name: abs_pos(node) for name,node in _anchor_nodes.items()}

# ---- group by page -----------------------------------------------------------
# page key = deepest tabsheet that has an image; pgcTrayArm sub-tabs and
# grpManual are standalone region pages (own screenshots)
PAGE_ORDER=["tsStack1_Above","grpManual","tsStack1_Under","tsStack1_Cassette",
            "tsStack2_Above","tsStack2_Under","tsStack2_Cassette",
            "tsStack3_Above","tsStack3_Under",
            "tsFix","tsInArmVacuum","tsIndexVacuum","tsOutArmVacuum","tsOtherVacuum",
            "tsTrayArm","tsRTArm","tsUnderArm",
            "tsShuttle","tsKeyPad","tsSystem","tsIndex","tsATC",
            "tsMagazine","tsAOI","tsAGV","tsTTL","tsConnection","tsSafe"]
PAGE_TITLE={
 "tsStack1_Above":"Stack 1 - Above Conveyor","tsStack1_Under":"Stack 1 - Under Conveyor",
 "tsStack1_Cassette":"Stack 1 - Cassette",
 "grpManual":"Stack 1 - Empty && Color Tray (Manual)",
 "tsStack2_Above":"Stack 2 - Above Conveyor","tsStack2_Under":"Stack 2 - Under Conveyor",
 "tsStack2_Cassette":"Stack 2 - Cassette",
 "tsStack3_Above":"Stack 3 - Above Conveyor","tsStack3_Under":"Stack 3 - Under Conveyor",
 "tsFix":"Fix Tray","tsInArmVacuum":"Vacuum - In Arm","tsIndexVacuum":"Vacuum - Index",
 "tsOutArmVacuum":"Vacuum - Out Arm","tsOtherVacuum":"Vacuum - Others",
 "tsTrayArm":"Vacuum - Others / Tray Arm","tsRTArm":"Vacuum - Others / RT Arm",
 "tsUnderArm":"Vacuum - Others / Under Arm",
 "tsShuttle":"Shuttle","tsKeyPad":"Key Pad","tsSystem":"System","tsIndex":"Index",
 "tsATC":"ATC","tsMagazine":"Options - Magazine","tsAOI":"Options - AOI",
 "tsAGV":"Options - AGV","tsTTL":"TTL","tsConnection":"Others","tsSafe":"Safe PLC - Safe",
}

pages={}
for c in components:
    tabs=c["tabs"]
    key=None; visible=True; subnote=""
    if "grpManual" in c["anc"]:
        key="grpManual"
    else:
        # find the deepest tab that is a known page
        for t in reversed(tabs):
            if t in PAGE_ORDER:
                key=t; break
        if key is None:
            key=tabs[-1] if tabs else "?"
    c["visible"]=visible
    c["subnote"]=subnote
    pages.setdefault(key,[]).append(c)

# ---- emit HTML ---------------------------------------------------------------
import struct
def img_info(name):
    """return (base64, width, height) or None"""
    p=os.path.join(IMGDIR,name+".png")
    if not os.path.exists(p): return None
    data=open(p,"rb").read()
    w,h=struct.unpack(">II", data[16:24])   # PNG IHDR
    return base64.b64encode(data).decode(), w, h

html=[]
html.append("""<!DOCTYPE html>
<html lang="zh-Hant"><head><meta charset="utf-8">
<title>HT9011UC IO check and verify - Alias Map (V3.33.908.0)</title>
<style>
 body{font-family:"Segoe UI",Arial,"Microsoft JhengHei",sans-serif;background:#f4f6f8;margin:0;padding:0 0 60px 0;}
 header{background:#123c69;color:#fff;padding:18px 32px;}
 header h1{margin:0;font-size:22px;} header p{margin:6px 0 0 0;font-size:13px;color:#cfe0f2;}
 nav{position:sticky;top:0;background:#fff;border-bottom:1px solid #d5dbe2;padding:8px 24px;z-index:50;line-height:2;}
 nav a{margin-right:10px;font-size:12.5px;color:#175fa9;text-decoration:none;white-space:nowrap;}
 nav a:hover{text-decoration:underline;}
 section{margin:28px 24px;background:#fff;border:1px solid #d5dbe2;border-radius:6px;padding:18px 22px;}
 h2{margin:0 0 4px 0;font-size:18px;color:#123c69;}
 .meta{font-size:12px;color:#667;margin-bottom:12px;}
 .wrap{position:relative;border:1px solid #bbb;}
 .wrap img{display:block;}
 .mk{position:absolute;border:1.5px solid rgba(220,30,30,.9);background:rgba(255,60,60,.12);box-sizing:border-box;cursor:pointer;}
 .mk .no{position:absolute;left:-2px;top:-13px;background:#d32;color:#fff;font-size:9px;line-height:11px;padding:0 3px;border-radius:2px;font-weight:bold;}
 .mk:hover,.mk.hl{border-color:#00b0ff;background:rgba(0,176,255,.25);z-index:20;}
 .mk:hover .no,.mk.hl .no{background:#0077cc;}
 .mk .tip{display:none;position:absolute;left:0;top:100%;margin-top:2px;background:#222;color:#fff;font-size:11px;padding:3px 7px;border-radius:3px;white-space:nowrap;z-index:30;}
 .mk:hover .tip{display:block;}
 .ctl{margin:8px 0;font-size:12.5px;color:#345;}
 table{border-collapse:collapse;font-size:12px;margin-top:12px;width:938px;}
 th,td{border:1px solid #c8d0d8;padding:3px 8px;text-align:left;}
 th{background:#e8eef4;color:#123c69;position:sticky;}
 tr:nth-child(even){background:#f7fafc;}
 tr.hl{background:#d2ecff !important;}
 tr.hidden-comp td{color:#999;}
 td.alias{font-family:Consolas,monospace;color:#0a5;font-weight:600;}
 td.cname{font-family:Consolas,monospace;}
 .badge{display:inline-block;font-size:10px;padding:1px 6px;border-radius:8px;color:#fff;}
 .b-led{background:#2a7;} .b-btn{background:#159;}
 .note{font-size:11px;color:#b50;}
 #csvbar{background:#fdf6e3;border-bottom:1px solid #d9cfa8;padding:8px 24px;font-size:12.5px;color:#543;}
 #csvbar button{font-size:12px;margin:0 4px;padding:2px 10px;cursor:pointer;}
 td.iocode{font-family:Consolas,monospace;font-weight:700;color:#146;white-space:nowrap;}
 td.iocode .dis{color:#aaa;text-decoration:line-through;}
 td.iocode .miss{color:#c33;font-weight:400;font-size:11px;}
 td.iocode .nc{color:#b80;font-weight:400;}
 td.iotime{font-size:11px;color:#557;white-space:nowrap;}
 .csvstat{margin-left:10px;color:#579;}
 #iostats{margin-top:8px;}
 #iostats table{border-collapse:collapse;font-size:12px;width:auto;margin:0;}
 #iostats th,#iostats td{border:1px solid #cbbd90;padding:2px 12px;text-align:right;background:#fffdf5;}
 #iostats th{background:#f3e9c8;color:#543;text-align:center;}
 #iostats td:first-child{text-align:left;font-weight:600;}
 #iostats tr.sum td{background:#f3e9c8;font-weight:700;}
</style></head><body>
<header>
 <h1>HT9011UC「IO check and verify」畫面元件 Alias 對照表</h1>
 <p>來源：HT9011UC_Code_V3.33.908.0_20260702 / iosetview.dfm ｜ 元件類型：TMyLedLane / TBtnPanelLane（TTL 頁含 TMyLed / TBtnPanel）｜ 更新日期：2026-07-06</p>
</header>
<nav>""")

for key in PAGE_ORDER:
    if key in pages:
        html.append('<a href="#%s">%s</a>' % (key, PAGE_TITLE.get(key,key)))
html.append("</nav>")
html.append("""<div id="csvbar">&#128202; IO_Table.csv：<b id="csvsrc">載入中...</b>
 <input type="file" id="csvfile" accept=".csv" style="display:none" onchange="loadCsvFile(this.files[0])">
 <button onclick="document.getElementById('csvfile').click()">&#128194; 載入 IO_Table.csv</button>
 <button onclick="clearCsv()">還原內建資料</button>
 <span style="color:#997">（也可直接將 CSV 拖放到頁面；載入後會記住，重新整理仍有效）</span>
 <div id="iostats"></div></div>""")

total=0
for key in PAGE_ORDER:
    if key not in pages: continue
    comps=sorted(pages[key], key=lambda c:(c["y"],c["x"]))
    for i,c in enumerate(comps,1): c["no"]=i
    total+=len(comps)
    info=img_info(key)
    nled=sum(1 for c in comps if c["type"].startswith("TMyLed"))
    nbtn=len(comps)-nled
    html.append('<section id="%s">' % key)
    html.append('<h2>%s <span style="font-size:12px;color:#888">(%s)</span></h2>' % (PAGE_TITLE.get(key,key), key))
    html.append('<div class="meta">LED：%d 個 ｜ BTN：%d 個 ｜ 合計 %d 個<span class="csvstat" id="cs_%s"></span></div>' % (nled,nbtn,len(comps),key))
    if info:
        b64,iw,ih=info
        html.append('<div class="ctl"><label><input type="checkbox" checked onclick="tgl(this,\'%s\')"> 顯示標註框（滑鼠移到框上可顯示 Alias；點框可跳至表格）</label></div>' % key)
        html.append('<div class="wrap" id="w_%s" style="width:%dpx"><img src="data:image/png;base64,%s" alt="%s" style="width:%dpx;height:%dpx">' % (key,iw,b64,key,iw,ih))
        pdx,pdy=PAGE_ADJ.get(key,(0,0))
        anchor=REGION_PAGES.get(key)
        if anchor:
            ax,ay=ANCHOR_ABS[anchor]
            bx,by=-ax,-ay          # region capture: origin at anchor container
        else:
            bx,by=BORDER_X,TITLE_Y # full-window capture
        for c in comps:
            if not c["visible"]: continue
            x=c["x"]+bx+pdx; y=c["y"]+by+pdy
            w=max(c["w"],10); h=max(c["h"],10)
            adj=COMP_ADJ.get(c["name"])
            if adj:
                x+=adj[0]; y+=adj[1]
                if len(adj)>=4: w,h=adj[2],adj[3]
            for (nf,nt,ndx,ndy) in PAGE_NO_ADJ.get(key,()):
                if nf<=c["no"]<=nt:
                    x+=ndx; y+=ndy
            tip="%s ｜ %s" % (c["alias"] or "(無 Alias)", c["name"])
            html.append('<div class="mk" id="mk_%s_%d" style="left:%dpx;top:%dpx;width:%dpx;height:%dpx" '
                        'onclick="jmp(\'%s\',%d)" onmouseover="hlr(\'%s\',%d,1)" onmouseout="hlr(\'%s\',%d,0)">'
                        '<span class="no">%d</span><span class="tip" data-alias="%s">%s</span></div>'
                        % (key,c["no"],x,y,w,h,
                           key,c["no"],key,c["no"],key,c["no"],c["no"],c["alias"],tip))
        html.append('</div>')
    else:
        html.append('<div class="note">※ 此分頁無對應截圖（%s.png 不存在），僅列出元件清單。</div>' % key)
    html.append('<table><thead><tr><th style="width:36px">#</th><th style="width:210px">元件名稱</th><th style="width:100px">類型</th><th style="width:230px">Alias</th><th style="width:90px">IO 點位</th><th style="width:140px">時序(Output)</th><th>備註</th></tr></thead><tbody>')
    for c in comps:
        badge='<span class="badge b-led">LED</span>' if c["type"].startswith("TMyLed") else '<span class="badge b-btn">BTN</span>'
        note=""
        if c["subnote"]:
            note="子分頁：%s" % c["subnote"]
            if not c["visible"]: note+="（截圖未顯示）"
        html.append('<tr id="tr_%s_%d" class="%s" onmouseover="hlm(\'%s\',%d,1)" onmouseout="hlm(\'%s\',%d,0)">'
                    '<td>%d</td><td class="cname">%s</td><td>%s %s</td><td class="alias">%s</td>'
                    '<td class="iocode" data-alias="%s"></td><td class="iotime"></td>'
                    '<td>%s</td></tr>'
                    % (key,c["no"], "hidden-comp" if not c["visible"] else "",
                       key,c["no"],key,c["no"],
                       c["no"],c["name"],badge,c["type"],c["alias"] or "&mdash;",
                       c["alias"],
                       note))
    html.append('</tbody></table></section>')

# embed default CSV (machine master copy at generation time)
try:
    _csv_text = open(CSV_SRC, "r", encoding="cp950", errors="replace").read()
except IOError:
    _csv_text = ""
html.append('<script>var DEFAULT_CSV=%s;</script>' % json.dumps(_csv_text))

html.append("""<script>
function tgl(cb,k){var w=document.getElementById('w_'+k);if(!w)return;
 var ms=w.getElementsByClassName('mk');for(var i=0;i<ms.length;i++)ms[i].style.display=cb.checked?'':'none';}
function hlr(k,n,on){var t=document.getElementById('tr_'+k+'_'+n);if(t)t.classList.toggle('hl',!!on);}
function hlm(k,n,on){var m=document.getElementById('mk_'+k+'_'+n);if(m)m.classList.toggle('hl',!!on);}
function jmp(k,n){var t=document.getElementById('tr_'+k+'_'+n);if(t){t.scrollIntoView({block:'center'});t.classList.add('hl');setTimeout(function(){t.classList.remove('hl');},1600);}}

/* ================= IO_Table.csv integration ================= */
var IN_TYPES ={Sensor:1,Cylinder_On:1,Cylinder_Off:1,Sucker:1};
var OUT_TYPES={Switch:1,Cylinder:1,Sucker_On:1,Sucker_Off:1};
function parseCsv(text){
  var lines=text.split(/\\r?\\n/), map={};
  if(!lines.length)return map;
  var hdr=lines[0].split(','), idx={};
  for(var i=0;i<hdr.length;i++)idx[hdr[i].trim()]=i;
  function g(f,k){var j=idx[k];return (j===undefined||f[j]===undefined)?'':f[j].trim();}
  for(var li=1;li<lines.length;li++){
    if(!lines[li])continue;
    var f=lines[li].split(',');
    var alias=g(f,'Alias'); if(!alias)continue;
    (map[alias]=map[alias]||[]).push({
      t:g(f,'IOType'),lane:g(f,'Lane'),ip:g(f,'IP'),port:g(f,'Port'),bit:g(f,'Bit'),
      en:g(f,'Enable'),onA:g(f,'OnAlarmTime'),offA:g(f,'OffAlarmTime'),
      onD:g(f,'OnDelayTime'),offD:g(f,'OffDelayTime')});
  }
  return map;
}
function ioCode(r){
  var d=IN_TYPES[r.t]?'I':(OUT_TYPES[r.t]?'O':'?');
  if(r.lane===''||r.ip===''||r.port===''||r.bit==='')return null; // not wired
  var ip=parseInt(r.ip,10);
  var ipc=(ip<10)?String(ip):String.fromCharCode(55+ip); // 10=A,11=B,... 32=W
  return d+r.lane+ipc+r.port+r.bit;
}
function timeStr(r){
  if(!OUT_TYPES[r.t])return '';
  if(!r.onA&&!r.offA&&!r.onD&&!r.offD)return '';
  return '\u8b66\u5831 '+(r.onA||'-')+'/'+(r.offA||'-')+' \uff5c \u5ef6\u9072 '+(r.onD||'-')+'/'+(r.offD||'-');
}
function applyCsv(text,label){
  var map=parseCsv(text);
  var stats={};
  var tds=document.querySelectorAll('td.iocode');
  for(var i=0;i<tds.length;i++){
    var td=tds[i], alias=td.getAttribute('data-alias');
    var tt=td.parentNode.querySelector('td.iotime');
    var sec=td.closest('section'); var sk=sec?sec.id:'';
    stats[sk]=stats[sk]||{hit:0,miss:0,nc:0};
    if(!alias){td.innerHTML='';tt.innerHTML='';continue;}
    var rows=map[alias];
    if(!rows){td.innerHTML='<span class="miss">\u7121\u8cc7\u6599</span>';tt.innerHTML='';stats[sk].miss++;continue;}
    var codes=[],times=[],wired=false;
    for(var j=0;j<rows.length;j++){
      var c=ioCode(rows[j]);
      var dis=(rows[j].en==='0');
      if(c){wired=true;codes.push(dis?'<span class="dis">'+c+'</span>':c);}
      else codes.push('<span class="nc">\u672a\u914d\u7f6e</span>');
      var ts=timeStr(rows[j]); if(ts&&times.indexOf(ts)<0)times.push(ts);
    }
    td.innerHTML=codes.join('<br>');
    tt.innerHTML=times.join('<br>');
    if(wired)stats[sk].hit++;else stats[sk].nc++;
  }
  for(var k in stats){
    var el=document.getElementById('cs_'+k);
    if(el)el.textContent='\uff5c CSV\uff1a\u5df2\u914d\u7f6e '+stats[k].hit+' \uff65 \u672a\u914d\u7f6e '+stats[k].nc+' \uff65 \u7121\u8cc7\u6599 '+stats[k].miss;
  }
  /* ---- global statistics panel ---- */
  function catOf(t){
    if(t==='Sensor')return 'Sensor';
    if(t==='Switch')return 'Switch';
    if(t==='Cylinder'||t==='Cylinder_On'||t==='Cylinder_Off')return 'Cylinder';
    if(t==='Sucker'||t==='Sucker_On'||t==='Sucker_Off')return 'Sucker';
    return 'Other';
  }
  var CATS=['Sensor','Switch','Cylinder','Sucker','Other'];
  var tbl={};  // category -> {rows, en, scr}
  for(var ci=0;ci<CATS.length;ci++)tbl[CATS[ci]]={rows:0,en:0,scr:0};
  for(var a in map){
    for(var j=0;j<map[a].length;j++){
      var cat=catOf(map[a][j].t);
      tbl[cat].rows++;
      if(map[a][j].en==='1')tbl[cat].en++;
    }
  }
  // screen-side: distinct aliases used by document components
  var seen={}, scrTotal=0, scrDistinct=0, scrHit=0;
  var tds2=document.querySelectorAll('td.iocode');
  for(var i=0;i<tds2.length;i++){
    var a=tds2[i].getAttribute('data-alias');
    if(!a)continue;
    scrTotal++;
    if(seen[a])continue;
    seen[a]=1; scrDistinct++;
    if(map[a]){scrHit++; tbl[catOf(map[a][0].t)].scr++;}
  }
  var totR=0,totE=0,totS=0;
  var h='<table><tr><th>\u5206\u985e</th><th>IO_Table \u7b46\u6578</th><th>Enable</th><th>\u756b\u9762\u4e2d\uff08\u4e0d\u91cd\u8907 Alias\uff09</th></tr>';
  for(var ci=0;ci<CATS.length;ci++){
    var c=CATS[ci], r=tbl[c];
    if(c==='Other'&&r.rows===0&&r.scr===0)continue;
    totR+=r.rows;totE+=r.en;totS+=r.scr;
    h+='<tr><td>'+c+'</td><td>'+r.rows+'</td><td>'+r.en+'</td><td>'+r.scr+'</td></tr>';
  }
  h+='<tr class="sum"><td>\u5408\u8a08</td><td>'+totR+'</td><td>'+totE+'</td><td>'+totS+'</td></tr></table>';
  h+='<div style="margin-top:4px;color:#765">\u756b\u9762\u5143\u4ef6\u7e3d\u6578\uff08\u542b Alias\uff09\uff1a'+scrTotal+
     ' \uff5c \u4e0d\u91cd\u8907 Alias\uff1a'+scrDistinct+
     ' \uff5c \u5728 IO_Table \u627e\u5230\uff1a'+scrHit+
     ' \uff5c \u627e\u4e0d\u5230\uff1a'+(scrDistinct-scrHit)+'</div>';
  document.getElementById('iostats').innerHTML=h;
  // marker tooltips
  var tips=document.querySelectorAll('.tip[data-alias]');
  for(var i=0;i<tips.length;i++){
    var tp=tips[i], alias=tp.getAttribute('data-alias');
    if(!tp.getAttribute('data-base'))tp.setAttribute('data-base',tp.textContent);
    var base=tp.getAttribute('data-base');
    var rows=alias?map[alias]:null;
    if(rows){
      var cs=[];
      for(var j=0;j<rows.length;j++){var c=ioCode(rows[j]);cs.push(c||'\u672a\u914d\u7f6e');}
      tp.textContent=base+' \uff5c '+cs.join(' / ');
    }else tp.textContent=base;
  }
  document.getElementById('csvsrc').textContent=label;
}
function loadCsvFile(file){
  if(!file)return;
  var rd=new FileReader();
  rd.onload=function(){
    var text=rd.result;
    try{localStorage.setItem('ioTableCsv',text);
        localStorage.setItem('ioTableMeta',file.name+' ('+new Date().toLocaleString()+')');}catch(e){}
    applyCsv(text,'\u5916\u90e8\u6a94\u6848\uff1a'+file.name+' ('+new Date().toLocaleString()+')');
  };
  rd.readAsText(file,'big5');
}
function clearCsv(){
  try{localStorage.removeItem('ioTableCsv');localStorage.removeItem('ioTableMeta');}catch(e){}
  applyCsv(DEFAULT_CSV,'\u5167\u5efa\u8cc7\u6599\uff08\u6587\u4ef6\u751f\u6210\u6642\u5d4c\u5165\uff09');
}
document.addEventListener('dragover',function(e){e.preventDefault();});
document.addEventListener('drop',function(e){
  e.preventDefault();
  if(e.dataTransfer.files.length)loadCsvFile(e.dataTransfer.files[0]);
});
(function(){
  var t=null,m=null;
  try{t=localStorage.getItem('ioTableCsv');m=localStorage.getItem('ioTableMeta');}catch(e){}
  if(t)applyCsv(t,'\u5916\u90e8\u6a94\u6848\uff1a'+(m||''));
  else applyCsv(DEFAULT_CSV,'\u5167\u5efa\u8cc7\u6599\uff08\u6587\u4ef6\u751f\u6210\u6642\u5d4c\u5165\uff09');
})();
</script>
<div style="margin:10px 26px;font-size:12px;color:#667">合計 """+"" ""+"""<span id="tt"></span></div>
</body></html>""")

os.makedirs(os.path.dirname(OUT), exist_ok=True)
# copy CSV next to HTML only if absent (do not clobber user edits)
if os.path.exists(CSV_SRC) and not os.path.exists(CSV_DST):
    shutil.copy(CSV_SRC, CSV_DST)
with open(OUT,"w",encoding="utf-8") as f:
    f.write("\n".join(html).replace('<span id="tt"></span>', str(total)+" 個元件"))
print("pages:", {k:len(v) for k,v in pages.items()})
print("total:", total)
print("written:", OUT, os.path.getsize(OUT), "bytes")
