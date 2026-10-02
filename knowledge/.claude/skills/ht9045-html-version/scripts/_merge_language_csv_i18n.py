# Language.csv（D:\HT9045\system\Language.csv，cp950）併入 page/i18n.js 的 HTI18N.dfm 字典
# 不產生獨立 JSON（依使用者指示 2026-09-09）；改完務必用瀏覽器 http 驗證（file:// 會吃快取）。
import csv, json, re

SRC = r"D:\HT9045\system\Language.csv"
DST = r"D:\HT9045\page\i18n.js"

with open(SRC, encoding="cp950", errors="replace", newline="") as f:
    rows = list(csv.reader(f))
header = rows[0]
assert header == ["From", "Name", "EString", "CString", "EHint", "CHint"], header

entries = {}
order = []
for r in rows[1:]:
    if not any(c.strip() for c in r):
        continue
    d = dict(zip(header, r + [""] * (len(header) - len(r))))
    key = "%s.%s" % (d["From"], d["Name"])
    if key in entries:
        continue
    item = {}
    if d["EString"].strip():
        item["en"] = d["EString"].strip()
    if d["CString"].strip():
        item["zh"] = d["CString"].strip()
    if d["EHint"].strip():
        item["enHint"] = d["EHint"].strip()
    if d["CHint"].strip():
        item["zhHint"] = d["CHint"].strip()
    if item:
        entries[key] = item
        order.append(key)

lines = ["  dfm: {"]
for i, key in enumerate(order):
    v = entries[key]
    parts = []
    for f in ("en", "zh", "enHint", "zhHint"):
        if f in v:
            parts.append("%s:%s" % (f, json.dumps(v[f], ensure_ascii=False)))
    lines.append("    %s: {%s}%s" % (json.dumps(key, ensure_ascii=False), ", ".join(parts), "," if i < len(order) - 1 else ""))
lines.append("  },")
dfm_block = "\n".join(lines)

content = open(DST, encoding="utf-8").read()
marker_start = "  terms: {"
idx = content.index(marker_start)
new_content = content[:idx] + dfm_block + "\n\n" + content[idx:]

# t() 方法後補一個 td()（依 Form.Component 查 dfm 字典）
new_content = new_content.replace(
    "  t: function (key, lang) {\n    var e = this.terms[key];\n    if (!e || lang === 'en' || !e[lang]) return key;\n    return e[lang];\n  }\n};",
    "  t: function (key, lang) {\n    var e = this.terms[key];\n    if (!e || lang === 'en' || !e[lang]) return key;\n    return e[lang];\n  },\n\n"
    "  /* Language.csv 併入（2026-09-09）：td(form, name, lang) 查 dfm 字典，查無回傳 null（呼叫端自行 fallback 原始 Caption） */\n"
    "  td: function (form, name, lang) {\n    var e = this.dfm[form + '.' + name];\n    if (!e) return null;\n    if (lang === 'en') return e.en || null;\n    return e[lang] || e.en || null;\n  }\n};"
)

with open(DST, "w", encoding="utf-8") as f:
    f.write(new_content)
print("merged", len(order), "dfm entries into i18n.js")
