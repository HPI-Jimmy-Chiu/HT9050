# -*- coding: utf-8 -*-
"""timer_census.py -- 盤點 golden 全部表單上的 TTimer，給 Timer 排程表計畫書（docs/TIMER_TABLE_PLAN.md）用。

AI(W906-TIMER-TABLE) 20261001。

## 為什麼要有這支

使用者 20261001：「這份計畫書工作量大，甚至包含所有的 timer 你都要考慮到」。
Timer 散在 100 多張表單上，人工數一定漏（記憶 call-site-census-needs-a-tool-not-a-grep）。
這支把「golden 有哪些 TTimer、它們什麼時候開、本體多大、移植樹提到沒有」一次量出來，
計畫書的清單由它產生，日後重跑就能對帳。

## 它量什麼（每一支 TTimer 一列）

1. 表單檔（.dfm）裡的 `object X: TTimer` 區塊：Enabled（沒寫＝True）、Interval（沒寫＝1000）、OnTimer。
2. 表單是不是開機就建（golden HT9045.cpp 的 `Application->CreateForm`）。
   開機就建＋Enabled=True ⇒ 這支 Timer 從程式啟動就在跑，不管畫面有沒有顯示。
3. 本體（`void __fastcall TfX::Handler(TObject *Sender)`）的行數、開頭有沒有看 fShow／bShow。
4. 全 golden 對 `X->Enabled=` / `X->Interval=` 的寫入，以及寫在哪個函式裡（FormShow／FormClose／其他）。
5. 重入保護旗標（`static bool b=false; if(b) return; b=true; ... b=false;`）與
   「設了 true 之後 return 卻沒放回 false」的出口（main.cpp:2928 那一類，V912 RogerYang 20260823 修過一個）。
6. 移植樹：`TfX::Handler` 這個完整名字在活的程式碼（不在 `#if 0` 裡）出現幾次；
   只用名字（不帶類別）在幾個檔被提到（含註解，只當線索）。

## 分類（計畫書 §3 用）

  A  開機時是關的，由別的程式打開（多半是自己表單的 FormShow）    ⇒ 開關照翻就對了
  B  一直開著，本體開頭看 fShow／bShow，畫面沒開就 return           ⇒ fShow 接頁面表就對了
  C  一直開著，本體不看畫面 —— 程式活著就跑                           ⇒ 不可以改成「網頁開了才跑」
  D  開機時是關的，也找不到任何打開它的程式                          ⇒ 死 Timer（翻不翻要看理由）
  ?  表單不是開機就建（用到才 new），Enabled=True                    ⇒ 建好之後就跑，要看誰建它

## 它不做什麼（誠實的邊界）

* **不是編譯器。** 本體範圍用「函式標頭在第 0 欄、第一個第 0 欄的 `}` 結束」判斷 —— golden 的寫法一律如此。
* fShow 判斷只看本體前 15 個非空白行；寫在更後面的不算（會被分成 C，翻譯的人要再看一眼）。
* 「沒放回 false 的 return」只看 return 前 3 行有沒有 `旗標=false`；巢狀在其他區塊裡放回的會誤報 —— 是線索，不是結論。
* 移植樹那兩欄只按名字找。移植樹常把 golden 的一段翻成 `W906_XxxTick()` 這種新名字（例：Timer2 的測試秒數），
  那種情況這支看不到 —— 第 6 項只能說「找到了」，不能說「沒翻」。

用法：
    python tools/timer_census.py                         # 印摘要，並寫 docs/TIMER_CENSUS.md 與 .tsv
    python tools/timer_census.py --golden <golden 根目錄>
    python tools/timer_census.py --no-write              # 只印
"""
import argparse
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.dirname(HERE)                         # HT9011UC_Cpp_V3.33.906.0
GOLDEN_NAME = "HT9011UC_Code_V3.33.906.0_20260618"


def find_golden():
    # worktree 裡沒有 golden（它不進 git），往上找，最後退回主 checkout。
    d = PORT
    for _ in range(6):
        cand = os.path.join(os.path.dirname(d), GOLDEN_NAME)
        if os.path.isdir(cand):
            return cand
        d = os.path.dirname(d)
    cand = os.path.join("D:" + os.sep, "HT9045", GOLDEN_NAME)
    return cand if os.path.isdir(cand) else None


def read_big5(path):
    with open(path, "rb") as f:
        raw = f.read()
    return raw.decode("cp950", errors="replace")


def walk(root, exts):
    for dp, dns, fns in os.walk(root):
        dns[:] = [d for d in dns if d.lower() != ".svn"]
        for fn in fns:
            if os.path.splitext(fn)[1].lower() in exts:
                yield os.path.join(dp, fn)


OBJ_RE = re.compile(r"^(\s*)(object|inherited|inline)\s+(\w+)\s*:\s*(\w+)")
PROP_RE = re.compile(r"^\s*(\w+)\s*=\s*(.+?)\s*$")


def parse_dfm(path):
    """回傳 (表單變數, 表單類別, [timer dict])。文字格式才解；二進位格式回 None。"""
    txt = read_big5(path)
    if not txt.lstrip().startswith(("object", "inherited", "inline")):
        return None
    lines = txt.splitlines()
    m = OBJ_RE.match(lines[0])
    if not m:
        return None
    form_var, form_cls = m.group(3), m.group(4)
    timers = []
    i = 0
    while i < len(lines):
        mo = OBJ_RE.match(lines[i])
        if mo and mo.group(4) == "TTimer":
            indent = mo.group(1)
            t = {"name": mo.group(3), "dfm_line": i + 1, "Enabled": "True", "Interval": "1000", "OnTimer": ""}
            j = i + 1
            while j < len(lines) and not re.match(r"^" + re.escape(indent) + r"end\s*$", lines[j]):
                mp = PROP_RE.match(lines[j])
                if mp and mp.group(1) in ("Enabled", "Interval", "OnTimer"):
                    t[mp.group(1)] = mp.group(2)
                j += 1
            timers.append(t)
            i = j
        i += 1
    return form_var, form_cls, timers


FUNC_HDR = re.compile(r"^[A-Za-z_].*?\b(\w+)::(~?\w+)\s*\(")
FREE_HDR = re.compile(r"^(?!(?:if|for|while|switch|return|else|do|case|typedef|extern)\b)[A-Za-z_][\w\s\*&<>,]*?\b(\w+)\s*\(")


def header(line):
    """回傳 (類別, 名字)；自由函式的類別是 ""。不是函式標頭回 None。"""
    if line.rstrip().endswith(";"):
        return None
    m = FUNC_HDR.match(line)
    if m:
        return m.group(1), m.group(2)
    m = FREE_HDR.match(line)
    if m:
        return "", m.group(1)
    return None


def func_spans(lines):
    """[(start, end, class, name)]，1-based 行號；標頭在第 0 欄、第一個第 0 欄的 } 結束。"""
    spans = []
    i = 0
    n = len(lines)
    while i < n:
        h = header(lines[i])
        if h:
            j = i + 1
            while j < n and not lines[j].startswith("{"):
                if lines[j].startswith("}") or header(lines[j]):
                    break
                j += 1
            if j < n and lines[j].startswith("{") and j - i <= 6:
                k = j + 1
                while k < n and not lines[k].startswith("}"):
                    k += 1
                spans.append((i + 1, k + 1, h[0], h[1]))
                i = k + 1
                continue
        i += 1
    return spans


def enclosing(spans, line):
    for s, e, c, n in spans:
        if s <= line <= e:
            return ("%s::%s" % (c, n)) if c else n
    return "(file scope)"


def strip_comment(s):
    k = s.find("//")
    return s if k < 0 else s[:k]


FSHOW_RE = re.compile(r"!\s*(fShow|bShow|Visible|Showing)\b|\b(fShow|bShow|Visible|Showing)\s*==\s*false")


def analyse_body(lines, start, end):
    body = lines[start - 1:end]
    code = [strip_comment(x) for x in body]
    nonblank = [(start + idx, c) for idx, c in enumerate(code) if c.strip() and c.strip() not in ("{", "}")]
    fshow = ""
    for ln, c in nonblank[:15]:
        if FSHOW_RE.search(c) and ("return" in c or (ln - start + 1 < len(code) and "return" in code[ln - start + 1])):
            fshow = "%d" % ln
            break
    # 重入旗標：`static bool X=false;` 之中，先被 `if(...X...)` 擋著 return、緊接著（8 行內）被設成 true 的那一個。
    #   只認這個形狀；第一個 static bool 不一定是旗標（THGem::Timer1Timer 的第一個是 bSendDoSeparate）。
    guard = ""
    stuck = []
    for ln, c in nonblank[:25]:
        mg = re.search(r"static\s+bool\s+(\w+)\s*=\s*false", c)
        if not mg:
            continue
        name = mg.group(1)
        set_line = None
        for ln2, c2 in nonblank:
            if ln2 > ln and re.search(r"\b" + name + r"\s*=\s*true\b", c2):
                set_line = ln2
                break
        if set_line is None:
            continue
        if_line = None
        for k, (ln2, c2) in enumerate(nonblank):
            if ln2 <= ln or ln2 >= set_line:
                continue
            if re.search(r"\bif\s*\(.*\b" + name + r"\b", c2):
                window = " ".join(x for _, x in nonblank[k:k + 4])
                if re.search(r"\breturn\b", window):
                    if_line = ln2
                    break
        if if_line is None or set_line - if_line > 8:
            continue
        guard = name
        for ln2, c2 in nonblank:
            if ln2 <= set_line or not re.search(r"\breturn\b", c2):
                continue
            back = code[max(0, ln2 - start - 3):ln2 - start + 1]
            if not any(re.search(r"\b" + name + r"\s*=\s*false", b) for b in back):
                stuck.append(str(ln2))
        break
    return fshow, guard, stuck


_GREP_CACHE = {}


def git_grep_port(word, qualified=False):
    pat = word if qualified else r"\b" + word + r"\b"
    if pat in _GREP_CACHE:                          # Timer1Timer 這種名字 60 張表單共用，同一個 pattern 只跑一次
        return _GREP_CACHE[pat]
    _GREP_CACHE[pat] = _git_grep(pat)
    return _GREP_CACHE[pat]


def _git_grep(pat):
    try:
        out = subprocess.run(["git", "grep", "-n", "-E", pat, "--", "*.cpp", "*.h", "*.inc"],
                             cwd=PORT, capture_output=True, text=True, encoding="utf-8", errors="replace").stdout
    except OSError:
        return []
    hits = []
    for line in out.splitlines():
        path = line.split(":", 1)[0]
        if path.startswith(("docs/", "tests/", "rc_out/", "golden/", "tools/dfm2rc/")):
            continue
        hits.append(line)
    return hits


_IF0_CACHE = {}


def live_line(path, lineno):
    """該行在不在 #if 0 裡（線性累積深度，同 start_sites_census.py）。"""
    if path not in _IF0_CACHE:
        depth = 0
        stack = []
        dead = []
        try:
            with open(os.path.join(PORT, path), encoding="utf-8", errors="replace") as f:
                for raw in f:
                    s = raw.strip()
                    if re.match(r"#\s*if\s+0\b", s):
                        stack.append(True)
                        depth += 1
                    elif re.match(r"#\s*if", s):
                        stack.append(False)
                    elif re.match(r"#\s*else", s) and stack and stack[-1]:
                        stack[-1] = False
                        depth -= 1
                    elif re.match(r"#\s*endif", s) and stack:
                        if stack.pop():
                            depth -= 1
                    dead.append(depth > 0)
        except OSError:
            pass
        _IF0_CACHE[path] = dead
    dead = _IF0_CACHE[path]
    return not (0 < lineno <= len(dead) and dead[lineno - 1])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--golden", default=None)
    ap.add_argument("--no-write", action="store_true")
    a = ap.parse_args()
    golden = a.golden or find_golden()
    if not golden or not os.path.isdir(golden):
        print("找不到 golden 樹（%s）；用 --golden 指定" % GOLDEN_NAME)
        return 2

    # 開機就建的表單（golden HT9045.cpp）
    auto = {}
    prj = os.path.join(golden, "HT9045.cpp")
    if os.path.isfile(prj):
        for m in re.finditer(r"CreateForm\(__classid\((\w+)\)\s*,\s*&(\w+)\)", read_big5(prj)):
            auto[m.group(1)] = m.group(2)

    # 全部 golden .cpp 一次讀進來（寫入點要全樹找）
    cpp_files = {}
    for p in walk(golden, {".cpp"}):
        rel = os.path.relpath(p, golden).replace(os.sep, "/")
        lines = read_big5(p).splitlines()
        cpp_files[rel] = (lines, None)

    # 只有同時帶 `->` 與 Enabled／Interval 的行才可能是寫入點；先挑出來，每支 Timer 只掃這些
    prop_lines = []
    for rel, (lines, _) in cpp_files.items():
        for idx, raw in enumerate(lines):
            if "->" in raw and ("Enabled" in raw or "Interval" in raw):
                c = strip_comment(raw)
                if "->" in c:
                    prop_lines.append((rel, idx, c))

    def spans_of(rel):
        lines, sp = cpp_files[rel]
        if sp is None:
            sp = func_spans(lines)
            cpp_files[rel] = (lines, sp)
        return sp

    rows = []
    for dfm in sorted(walk(golden, {".dfm"})):
        rel_dfm = os.path.relpath(dfm, golden).replace(os.sep, "/")
        parsed = parse_dfm(dfm)
        if parsed is None:
            continue
        form_var, form_cls, timers = parsed
        if not timers:
            continue
        own_cpp = rel_dfm[:-4] + ".cpp"
        for t in timers:
            r = dict(t)
            r.update({"dfm": rel_dfm, "form_var": form_var, "form_cls": form_cls,
                      "auto_created": "yes" if form_cls in auto else "no"})
            handler = t["OnTimer"]
            # 本體
            r.update({"body": "", "body_lines": 0, "fshow_line": "", "guard": "", "stuck_returns": ""})
            search = [own_cpp] if own_cpp in cpp_files else []
            search += [k for k in cpp_files if k not in search]
            if handler:
                for rel in search:
                    for s, e, c, n in spans_of(rel):
                        if c == form_cls and n == handler:
                            fs, gd, st = analyse_body(cpp_files[rel][0], s, e)
                            r.update({"body": "%s:%d-%d" % (rel, s, e), "body_lines": e - s + 1,
                                      "fshow_line": fs, "guard": gd, "stuck_returns": ",".join(st)})
                            break
                    if r["body"]:
                        break
            # 對 Enabled／Interval 的寫入
            writes = []
            nm = re.escape(t["name"])
            q_re = re.compile(r"\b" + re.escape(form_var) + r"\s*->\s*" + nm + r"\s*->\s*(Enabled|Interval)\s*=[^=]")
            u_re = re.compile(r"(?<![>\w])" + nm + r"\s*->\s*(Enabled|Interval)\s*=[^=]")
            for rel, idx, c in prop_lines:
                    if t["name"] not in c:
                        continue
                    mq = q_re.search(c)
                    mu = u_re.search(c) if rel == own_cpp else None
                    m = mq or mu
                    if not m:
                        continue
                    if mu and not mq:
                        # 自己表單檔裡沒帶表單變數的寫法，要確定是在自己類別的函式裡
                        fn = enclosing(spans_of(rel), idx + 1)
                        if not fn.startswith(form_cls + "::"):
                            continue
                    fn = enclosing(spans_of(rel), idx + 1)
                    val = c[m.end() - 1:].strip().rstrip(";").strip()
                    writes.append("%s %s=%s @%s:%d" % (fn, m.group(1), val, rel, idx + 1))
            r["writes"] = writes
            # 分類
            en = t["Enabled"].lower() == "true"
            enable_writes = [w for w in writes if " Enabled=" in w and "=false" not in w.split(" Enabled=")[1].split(" @")[0].lower()]
            if r["fshow_line"]:
                cls = "B"
            elif not en:
                cls = "A" if enable_writes else "D"
            elif form_cls not in auto:
                cls = "?"
            else:
                cls = "C"
            r["class"] = cls
            # 移植樹
            qual = git_grep_port(re.escape(form_cls) + r"::" + re.escape(handler), qualified=True) if handler else []
            qname = form_cls + "::" + handler

            def in_code(hit):
                # 註解裡的提及不算（移植樹大量在 // 與 /* */ 裡引用 golden 名字）
                text = hit.split(":", 2)[2] if hit.count(":") >= 2 else ""
                text = re.sub(r"/\*.*?\*/", "", text)
                return qname in strip_comment(text)
            r["port_qualified_live"] = sum(1 for h in qual if in_code(h) and live_line(h.split(":", 2)[0], int(h.split(":", 2)[1])))
            r["port_qualified_total"] = len(qual)
            if handler:
                files = set(h.split(":", 1)[0] for h in git_grep_port(handler))
                r["port_name_files"] = len(files)
            else:
                r["port_name_files"] = 0
            rows.append(r)

    # ---- 摘要 ----
    from collections import Counter
    by_cls = Counter(r["class"] for r in rows)
    forms = sorted(set(r["form_cls"] for r in rows))
    total_lines = sum(r["body_lines"] for r in rows)
    print("golden: %s" % golden)
    print("TTimer 共 %d 支，分布在 %d 張表單；本體合計 %d 行" % (len(rows), len(forms), total_lines))
    print("分類：" + "  ".join("%s=%d" % (k, by_cls[k]) for k in sorted(by_cls)))
    print("移植樹有活的 TfX::Handler：%d 支；名字完全沒出現：%d 支"
          % (sum(1 for r in rows if r["port_qualified_live"] > 0),
             sum(1 for r in rows if r["port_name_files"] == 0)))
    print("有重入保護旗標：%d 支；其中有『沒放回 false 的 return』線索：%d 支"
          % (sum(1 for r in rows if r["guard"]), sum(1 for r in rows if r["stuck_returns"])))

    if a.no_write:
        return 0

    cols = ["form_cls", "form_var", "name", "class", "Enabled", "Interval", "OnTimer", "auto_created",
            "body", "body_lines", "fshow_line", "guard", "stuck_returns",
            "port_qualified_live", "port_qualified_total", "port_name_files", "dfm", "dfm_line"]
    tsv = os.path.join(PORT, "docs", "TIMER_CENSUS.tsv")
    with open(tsv, "w", encoding="utf-8", newline="\n") as f:
        f.write("\t".join(cols + ["writes"]) + "\n")
        for r in rows:
            f.write("\t".join(str(r[c]) for c in cols) + "\t" + " | ".join(r["writes"]) + "\n")

    md = os.path.join(PORT, "docs", "TIMER_CENSUS.md")
    with open(md, "w", encoding="utf-8", newline="\n") as f:
        f.write("# golden TTimer 全表（工具產生，不要手改）\n\n")
        f.write("> 產生：`python tools/timer_census.py`（AI(W906-TIMER-TABLE) 20261001）。golden＝`%s`。\n" % GOLDEN_NAME)
        f.write("> 分類與欄位意思見工具檔頭；計畫書見 `docs/TIMER_TABLE_PLAN.md`。完整欄位（含每一個 Enabled／Interval 寫入點）在同名 `.tsv`。\n\n")
        f.write("**%d 支 TTimer、%d 張表單、本體合計 %d 行**；分類 %s。\n\n"
                % (len(rows), len(forms), total_lines, "、".join("%s＝%d" % (k, by_cls[k]) for k in sorted(by_cls))))
        f.write("| 表單 | Timer | 類 | Enabled | Interval | 開機就建 | 本體 | 行數 | fShow 檢查 | 重入旗標 | 沒放回的 return | 移植樹 TfX::Handler（活／全） | 名字出現的檔數 |\n")
        f.write("|---|---|---|---|---|---|---|---|---|---|---|---|---|\n")
        for r in sorted(rows, key=lambda x: (x["form_cls"] != "TfMain", x["form_cls"], x["name"])):
            f.write("| %s | %s | %s | %s | %s | %s | `%s` | %s | %s | %s | %s | %s／%s | %s |\n" % (
                r["form_cls"], r["name"], r["class"], r["Enabled"], r["Interval"], r["auto_created"],
                r["body"] or "（找不到本體）", r["body_lines"], r["fshow_line"] or "", r["guard"] or "",
                r["stuck_returns"] or "", r["port_qualified_live"], r["port_qualified_total"], r["port_name_files"]))
    print("寫出 %s 與 %s" % (os.path.relpath(md, PORT), os.path.relpath(tsv, PORT)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
