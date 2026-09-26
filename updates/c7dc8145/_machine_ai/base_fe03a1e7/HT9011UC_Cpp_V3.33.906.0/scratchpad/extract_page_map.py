# -*- coding: utf-8 -*-
"""extract_page_map.py -- 頁面 id -> 配方(區段,鍵) 對照表的機械抽取器。

AI(W906-FW-EXTRACT) 20260914: 重建。原版是另一台的 scratchpad 未追蹤檔。

兩種抽取法都實作：
  一跳  el*->Add(widget, &member, TYPE, "Section", "Key", ...)   <- 含型別，優先
  兩跳  member = ReadIniData(dir, "Section", "Key", def)
        widget->Text = ...member...

設計上「從頁面 id 反查 golden 全樹」，所以不需要事先知道哪個 .cpp 對哪個頁面。

用法:
  py extract_page_map.py <page.html> [page2.html ...]
  py extract_page_map.py --calibrate <Setup.Contact.html>
"""
import os, re, sys, glob, json, collections

GOLDEN = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
RECIPES = r'D:\HT9045\IniData\Data'

# --- golden 索引（整棵樹只掃一次） -----------------------------------------
RE_ADD = re.compile(
    r'\bel\w*->Add\(\s*'
    r'([A-Za-z_]\w*)\s*,\s*'          # 1 widget id
    r'&?([^,]+?)\s*,\s*'              # 2 member
    r'(EC\w+)\s*,\s*'                 # 3 type
    r'"([^"]*)"\s*,\s*'               # 4 section
    r'"([^"]*)"',                     # 5 key
    re.S)
Q = r'(?:AnsiString\s*\(\s*)?"([^"]*)"\s*\)?'
RE_READINI = re.compile(
    r'([A-Za-z_][\w\[\].>-]*)\s*=\s*(?:Check)?(?:And)?ReadIniData\s*\(\s*[^,]+,\s*' + Q + r'\s*,\s*' + Q, re.S)
# 直接一行：widget->Text = ReadIniData(dir,"Sec","Key",...)
RE_DIRECT = re.compile(
    r'([A-Za-z_]\w*)\s*->\s*(?:Text|Caption)\s*=\s*(?:Check)?(?:And)?ReadIniData\s*\(\s*[^,]+,\s*'
    + Q + r'\s*,\s*' + Q, re.S)
RE_ASSIGN_TEXT = re.compile(
    r'\b([A-Za-z_]\w*)\s*->\s*Text\s*=([^;]{0,200});', re.S)

SKIP_MEMBERS = {
    'Text','Caption','AnsiString','FloatToStrF','FloatToStr','IntToStr','StrToInt',
    'StrToFloat','ffFixed','ffGeneral','Items','CommaText','Value','Format','sprintf',
    'ReadIniData','CheckAndReadIniData','WriteIniData','szDir','FileName','true','false',
}

def read(p):
    for enc in ('utf-8', 'cp950', 'latin-1'):
        try:
            return open(p, encoding=enc, errors='strict').read()
        except (UnicodeDecodeError, LookupError):
            continue
    return open(p, encoding='latin-1', errors='replace').read()

def index_golden():
    onehop, member2key, widget2members, direct = {}, {}, collections.defaultdict(set), {}
    files = []
    for dp, dn, fn in os.walk(GOLDEN):
        if '.svn' in dp.split(os.sep):
            continue
        files += [os.path.join(dp, f) for f in fn if f.lower().endswith(('.cpp', '.h'))]
    for f in files:
        try:
            s = read(f)
        except OSError:
            continue
        base = os.path.basename(f)
        for m in RE_ADD.finditer(s):
            w, mem, ty, sec, key = (g.strip() for g in m.groups())
            onehop.setdefault(w, (sec, key, ty, base))
        for m in RE_DIRECT.finditer(s):
            w, sec, key = (g.strip() for g in m.groups())
            direct.setdefault(w, (sec, key, base))
        for m in RE_READINI.finditer(s):
            mem, sec, key = (g.strip() for g in m.groups())
            if '->' in mem:          # widget->Text= 由 RE_DIRECT 處理，不是成員
                continue
            short = mem.split('.')[-1]
            if short in SKIP_MEMBERS:
                continue
            member2key.setdefault(short, (sec, key, base))
        for m in RE_ASSIGN_TEXT.finditer(s):
            w, rhs = m.group(1), m.group(2)
            for tok in re.findall(r'[A-Za-z_]\w*', rhs):
                if tok in SKIP_MEMBERS or len(tok) < 4:
                    continue
                widget2members[w].add(tok)
    return onehop, member2key, widget2members, direct

# --- 配方鍵存在性索引 -------------------------------------------------------
def index_recipes():
    per = {}
    global KEY2FILES, FILE2COUNT
    KEY2FILES = collections.defaultdict(collections.Counter)
    FILE2COUNT = collections.Counter()
    for d in sorted(glob.glob(os.path.join(RECIPES, '*'))):
        if not os.path.isdir(d):
            continue
        keys = set()
        for f in glob.glob(os.path.join(d, '*.Data')) + glob.glob(os.path.join(d, '*.ini')):
            fb = os.path.basename(f).lower()
            FILE2COUNT[fb] += 1
            sec = ''
            try:
                for line in read(f).splitlines():
                    line = line.strip()
                    if line.startswith('[') and line.endswith(']'):
                        sec = line[1:-1].strip()
                    elif '=' in line and not line.startswith((';', '#')):
                        kk = (sec.lower(), line.split('=', 1)[0].strip().lower())
                        keys.add(kk); KEY2FILES[kk][fb] += 1
            except OSError:
                pass
        per[os.path.basename(d)] = keys
    return per

# --- 頁面 id ---------------------------------------------------------------
RE_TAG = re.compile(r'<\s*(\w+)([^>]*?\bid\s*=\s*["\']([^"\']+)["\'][^>]*)>', re.I | re.S)
def page_ids(path):
    s = read(path)
    out = {}
    for m in RE_TAG.finditer(s):
        tag, attrs, pid = m.group(1).lower(), m.group(2), m.group(3)
        ty = 'text' if tag == 'input' else tag
        t = re.search(r'\btype\s*=\s*["\']([^"\']+)["\']', attrs, re.I)
        if tag == 'input':
            ty = (t.group(1).lower() if t else 'text')
        out.setdefault(pid, ty)
    return out

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    sys.stderr.write('indexing golden ...\n')
    onehop, member2key, w2m, direct = index_golden()
    sys.stderr.write('  one-hop Add entries: %d\n' % len(onehop))
    sys.stderr.write('indexing recipes ...\n')
    recipes = index_recipes()
    nrec = len(recipes)
    sys.stderr.write('  recipe folders: %d\n' % nrec)

    rows = []
    for page in args:
        ids = page_ids(page)
        hits = {}
        for pid, ty in ids.items():
            if pid in onehop:
                sec, key, cty, src = onehop[pid]
                hits[pid] = dict(section=sec, key=key, ctype=cty, html=ty, via='1hop', src=src)
                continue
            if pid in direct:
                sec, key, src = direct[pid]
                hits[pid] = dict(section=sec, key=key, ctype='', html=ty, via='direct', src=src)
                continue
            for mem in sorted(w2m.get(pid, ()), key=lambda t: (-len(t), t)):
                if mem in member2key:
                    sec, key, src = member2key[mem]
                    hits[pid] = dict(section=sec, key=key, ctype='', html=ty, via='2hop', src=src)
                    break
        # 鍵存在性
        allrec = partial = none_ = 0
        for pid, h in hits.items():
            probe = (h['section'].lower(), h['key'].lower())
            n = sum(1 for k in recipes.values() if probe in k)
            fc = KEY2FILES.get(probe)
            doc = fc.most_common(1)[0][0] if fc else None
            denom = FILE2COUNT.get(doc, nrec) if doc else nrec
            h['inrecipes'] = n; h['doc'] = doc; h['denom'] = denom
            if n >= denom and n > 0:   allrec += 1
            elif n == 0:    none_ += 1
            else:           partial += 1
        text_fields = sum(1 for h in hits.values() if h['html'] == 'text')
        rows.append(dict(page=os.path.basename(page), ids=len(ids), matched=len(hits),
                         text=text_fields, allrec=allrec, partial=partial, none=none_,
                         detail=hits))
    print(json.dumps(dict(recipe_count=nrec, rows=rows), ensure_ascii=False, indent=1))

main()
