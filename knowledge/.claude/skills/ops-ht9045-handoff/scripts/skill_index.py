import os, re, sys

ROOTS = [("HT9045", "D:/HT9045/.claude/skills"), ("user", "D:/.github/skills")]
OUT = sys.argv[1]


def parse(path):
    s = open(path, "rb").read().decode("utf-8", "replace").replace("\r\n", "\n")
    n_lines = s.count("\n") + 1
    fm = ""
    if s.startswith("---\n"):
        end = s.find("\n---", 4)
        fm = s[4:end] if end > 0 else ""
    desc, grab = [], False
    for line in fm.split("\n"):
        if re.match(r"^description:\s*", line):
            grab = True
            rest = re.sub(r"^description:\s*[>|]?-?\s*", "", line)
            if rest:
                desc.append(rest)
            continue
        if grab:
            if re.match(r"^[A-Za-z_][\w-]*:", line):
                break
            desc.append(line.strip())
    d = " ".join(x for x in desc if x).strip().strip('"').strip("'")
    d = re.sub(r"\s+", " ", d)
    return d, n_lines


def split_desc(d):
    cut = len(d)
    for key in ("Use when", "Use When", "USE WHEN", "觸發關鍵字", "關鍵字", "Triggers", "Trigger", "觸發："):
        k = d.find(key)
        if 0 < k < cut:
            cut = k
    summary = d[:cut].strip(" 。.;；")
    if len(summary) > 80:
        summary = summary[:78] + "…"
    kw = ""
    m = re.search(r"(?:觸發關鍵字|關鍵字|Triggers?|keywords?)\s*[:：]\s*(.*)", d, re.I)
    if m:
        items = [x.strip() for x in re.split(r"[,，、]", m.group(1)) if x.strip()]
        kw = ", ".join(items[:8])
        if len(kw) > 90:
            kw = kw[:88] + "…"
    return summary, kw


rows = []
for tag, root in ROOTS:
    if not os.path.isdir(root):
        continue
    for name in sorted(os.listdir(root)):
        sk = os.path.join(root, name, "SKILL.md")
        if not os.path.isfile(sk):
            continue
        d, n = parse(sk)
        summary, kw = split_desc(d)
        refdir = os.path.join(root, name, "references")
        refs = sorted(f for f in os.listdir(refdir)) if os.path.isdir(refdir) else []
        rs = ", ".join(r.replace(".md", "") for r in refs[:6]) + (f" +{len(refs)-6}" if len(refs) > 6 else "")
        rows.append((tag, name, n, summary.replace("|", "/"), kw.replace("|", "/"), rs.replace("|", "/")))

with open(OUT, "w", encoding="utf-8") as f:
    f.write("<!-- AUTO TABLE START -->\n")
    for tag in ("HT9045", "user"):
        sub = [r for r in rows if r[0] == tag]
        f.write(f"\n### {tag}（{len(sub)}）\n\n| skill | 行 | 做什麼 | 關鍵字 | references |\n|---|--:|---|---|---|\n")
        for _, name, n, summ, kw, rs in sub:
            f.write(f"| {name} | {n} | {summ} | {kw} | {rs} |\n")
    f.write("<!-- AUTO TABLE END -->\n")
print(len(rows))
