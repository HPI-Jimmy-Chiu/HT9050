# -*- coding: utf-8 -*-
"""start_sites_census.py -- 盤點「哪些程式路徑能啟動機台」，並判斷每一處死活。

AI(W906-DUET-REF) 20260921。

## 為什麼要有這支

`WebStart.h` 的核心論證是「override TfMain::Start() 會一次武裝 N 條路徑」，
所以那個 N 與那張清單是承重的。它被人工維護過兩次，兩次都錯：

  * 20260915 的盤點說「約 19 個呼叫點，其中只有 2 個在 #if 0 裡」，
    並把 SECSGEM/uHGemHT9045.cpp:4722 列為活的第一個例子。
    實際上它從 20260809（commit 0b95c18）起就在 `#if 0 // GATE G01` 內。
    會看漏的原因很具體：**GATE G01 橫跨 212 行**（4521-4733）。
    往回目視找「最近的 #if」只會看到 4182 那個已經 #endif 掉的，看不到 4521。
  * 同一份盤點列出的每一個行號都已漂移。
  * 20260921 我自己重量時**又漏了 11 個**：只 grep `fMain->Start(`，
    結果只命中那些「碰巧在行尾帶了 `// golden fMain->Start("...")` 註解」的
    `W7C1_FMAIN_START(...)` 呼叫（DoCleanOutFinishCheck 那 6 個），
    而沒有那行註解的 DoOneCycleFinishCheck 5 個與 DoART_AfterCleanOut 6 個
    **完全沒出現在結果裡**。正確數字是 32 不是 21。

⇒ 記憶 same-trap-thrice-fix-the-tool：第三次就改機制。這支就是那個機制。
   （而它第一次跑就抓到第三次的錯，所以這個機制是划算的。）

## 它做什麼

1. 用 git grep 找出所有候選呼叫點（不是 ripgrep 目測，不是遞迴 grep ——
   見記憶 use-git-grep-for-whole-tree-search 與 recursive-grep-returns-false-empty）。
2. 對每個候選**逐行累積 `#if 0` 深度**，深度 > 0 才算被閘住。
   這是全檔線性掃描，不是往回找最近的 #if，所以 212 行的 gate 不會漏。
3. 印出分組清單與三個數字：總數／活的／被閘住的。

## 它不做什麼（誠實的邊界）

* **它不是前處理器。** 只認 `#if 0` 這一種死分支；`#if SOME_MACRO` 一律當活的。
  要判斷巨集展開後的死活，用編譯器（記憶 verify-with-compiler-not-scanner）。
  對這個題目夠用，因為這棵樹的刻意閘門一律寫成 `#if 0`。
* **它不跟蹤別名。** `W7C1_FMAIN_START(s)` 這種巨集是硬編在 PATTERNS 裡的；
  日後有人新增別的包裝巨集，要手動加進來，否則會漏。
* 註解會被 strip_comment() 去掉後才比對，所以「只出現在行尾註解裡」不會被誤計，
  純註解行也不會。但**不處理跨行的 /* */ 區塊註解**。
* **它只數呼叫點，不解讀語意。** 特別是第二份報告的 `SoftStart=true`：那是一個
  **共用的加速斜坡旗標**，不是每一處都代表「從閒置啟動生產」。20260921 逐一讀過的分類：
    SECSGEM 4 處  = S2F42 host command 遠端啟動        ← 真的是遠端啟動
    WebStart.cpp 3 處 = 我們自己翻譯的 StartFromWeb 落點  ← 就是那個落點本身
    csystem 2 處  = AccelateTask 斜坡（慢速 10 秒 → 換生產速度）← 不是冷啟動
    forms/fMain.cpp 1 處 = 馬達上電後的 HOME 序列（下一行就是 iHome=1）← 不是冷啟動
  要引用數字時請連這個分類一起引，否則會把 10 講成「10 條啟動路徑」。

## 用法

    python tools/start_sites_census.py              # 人看的表
    python tools/start_sites_census.py --check 32 29 3   # CI：總數/活/閘 不符就 exit 1

第二種形式是給 gate 用的：把今天的數字釘住，任何人新增或移除啟動路徑時
這支會紅燈，逼他回來更新 WebStart.h 的論證與 docs/DUET3D_REFERENCE_ANALYSIS.md §9.1.1。
"""
import io
import os
import re
import subprocess
import sys

# 呼叫點的樣式。新增包裝巨集時加在這裡。
# 每一項是 (git grep 用的固定字串, 確認用的 regex, 說明)。
# 兩段式的理由：git grep -F 快且不受 shell 轉義影響，regex 再對「去掉註解後的碼」
# 做精確確認，避免「只出現在行尾註解裡」被誤計。
PATTERNS = [
    ("fMain->Start(", r"fMain\s*->\s*Start\s*\(", "直接呼叫"),
    ("W7C1_FMAIN_START(", r"W7C1_FMAIN_START\s*\(", "經 W7C1_FMAIN_START 巨集"),
]

# 不經 Start() 的第二條繞道：直接寫啟動旗標。
# ⚠ 必須容許空白：這棵樹裡 `SoftStart=true` 與 `SoftStart = true` 兩種寫法都有，
#   只 grep 前者會漏掉 WebStart.cpp 的三處。
SOFTSTART = [("SoftStart", r"\bSoftStart\s*=\s*true\b", "直接寫啟動旗標")]

# 第三份：翻譯落點 StartFromWeb() 自己的呼叫者。
#
# ⚠⚠ 這一份**刻意不套用 EXCLUDE_PREFIXES**，理由是承重的：
#   S3 武裝（把 fMain 指向 TfMainWeb、讓 start.run 呼叫 StartFromWeb）
#   就住在 `tools/wb_serve.cpp` 裡。把 tools/ 排除掉，這支就永遠看不到
#   「這棵樹已經武裝了」—— 而那正是 20260918 到 20260921 之間發生的事：
#   WebStart.cpp 有 13 句註解寫著「StartFromWeb 全樹零個呼叫者（S3 尚未武裝）」，
#   在武裝之後**整整三天沒有人改**。那些句子不是描述、是**授權**
#   （「行為 delta = 0」＝「你可以放心解這個閘」）。
#
# 上面兩份數的是「有幾條路能啟動機台」，所以排除非生產碼是對的。
# 這一份數的是「這棵樹現在是不是武裝的」，所以不能排除。
ARMED = [("StartFromWeb", r"->\s*StartFromWeb\s*\(|\.\s*StartFromWeb\s*\(",
          "呼叫翻譯落點 StartFromWeb()")]

# 樹上宣稱「還沒武裝」的句子。武裝之後這些就變成假的授權。
# 已經就地標註「作廢」的那些不算 —— 它們是刻意保留的史料。
STALE_CLAIM_FILES = ["WebStart.cpp"]
STALE_CLAIM_RE = re.compile(r"零個呼叫者|尚未武裝")
VOID_RE = re.compile(r"作廢")

# 不列入生產碼計數的路徑前綴。
EXCLUDE_PREFIXES = ("tests/", "tools/")

IF0_RE = re.compile(r"^\s*#\s*if\s+0\b")
IF_RE = re.compile(r"^\s*#\s*if")
ELSE_RE = re.compile(r"^\s*#\s*el(se|if)")
ENDIF_RE = re.compile(r"^\s*#\s*endif")
# 巨集的「定義」那一行不是呼叫點。
DEFINE_RE = re.compile(r"^\s*#\s*define")


def strip_comment(text):
    """去掉行尾 // 註解，但不動字串字面裡的 //。

    為什麼需要：`W7C1_FMAIN_START("x");  // golden fMain->Start("...")` 這種行
    同時命中兩個樣式；而 `// fMain->Start()` 這種純註解行完全不該計。
    只看行首是不是 // 不夠 —— 會把「碼 + 註解」當成碼（正確）卻也可能把
    「碼裡沒有、只有註解裡有」的行算進去（錯誤）。

    限制（誠實寫出來）：不處理跨行的 /* */ 區塊註解。這棵樹的呼叫點都是單行，
    夠用；要更嚴格請用編譯器，見記憶 verify-with-compiler-not-scanner。
    """
    out = []
    in_str = False
    in_chr = False
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if in_str:
            if c == "\\":
                i += 2
                continue
            if c == '"':
                in_str = False
        elif in_chr:
            if c == "\\":
                i += 2
                continue
            if c == "'":
                in_chr = False
        else:
            if c == '"':
                in_str = True
            elif c == "'":
                in_chr = True
            elif c == "/" and i + 1 < n and text[i + 1] == "/":
                break
            elif c == "/" and i + 1 < n and text[i + 1] == "*":
                break
        out.append(c)
        i += 1
    return "".join(out)


def git_grep(pattern, root):
    """回傳 [(path, lineno, text)]。用 git grep，不用遞迴 grep。"""
    try:
        out = subprocess.run(
            ["git", "grep", "-n", "-F", pattern, "--", "*.cpp", "*.h"],
            cwd=root, capture_output=True, text=True, encoding="utf-8",
            errors="replace",
        )
    except OSError as e:
        sys.stderr.write("git grep failed: %s\n" % e)
        return []
    hits = []
    for line in out.stdout.split("\n"):
        if not line.strip():
            continue
        parts = line.split(":", 2)
        if len(parts) < 3:
            continue
        path, lineno, text = parts[0], parts[1], parts[2]
        if not lineno.isdigit():
            continue
        hits.append((path.replace("\\", "/"), int(lineno), text))
    return hits


_depth_cache = {}


def if0_depths(root, path):
    """回傳 {行號: #if 0 深度}。整檔線性掃描，不往回找。"""
    if path in _depth_cache:
        return _depth_cache[path]
    full = os.path.join(root, path)
    try:
        lines = io.open(full, encoding="utf-8", errors="replace").read().split("\n")
    except IOError:
        _depth_cache[path] = {}
        return {}
    depths = {}
    stack = []      # 每層一個 bool：True = 這層是死的 #if 0 分支
    dead = 0
    for i, ln in enumerate(lines, 1):
        s = ln.strip()
        if IF0_RE.match(s):
            stack.append(True)
            dead += 1
        elif IF_RE.match(s):
            stack.append(False)
        elif ELSE_RE.match(s) and stack:
            if stack[-1]:
                stack[-1] = False
                dead -= 1
        elif ENDIF_RE.match(s) and stack:
            if stack.pop():
                dead -= 1
        depths[i] = dead
    _depth_cache[path] = depths
    return depths


def collect(root, patterns):
    seen = set()
    rows = []
    for grep_pat, confirm_re, kind in patterns:
        rx = re.compile(confirm_re)
        for path, lineno, text in git_grep(grep_pat, root):
            if (path, lineno) in seen:
                continue
            if DEFINE_RE.match(text.strip()):
                continue
            code = strip_comment(text)          # ← 關鍵：只看碼，不看註解
            if not rx.search(code):
                continue
            seen.add((path, lineno))
            depth = if0_depths(root, path).get(lineno, 0)
            rows.append({
                "path": path,
                "line": lineno,
                "kind": kind,
                "gated": depth > 0,
                "text": code.strip()[:90],
                "production": not path.startswith(EXCLUDE_PREFIXES),
            })
    rows.sort(key=lambda r: (r["path"], r["line"]))
    return rows


def report(rows, title):
    print("=" * 78)
    print(title)
    print("=" * 78)
    cur = None
    for r in rows:
        if r["path"] != cur:
            cur = r["path"]
            print("\n  %s" % cur)
        mark = "GATED " if r["gated"] else "LIVE  "
        extra = "" if r["production"] else "  (非生產碼，不計)"
        print("    :%-6d %s %s%s" % (r["line"], mark, r["text"], extra))
    prod = [r for r in rows if r["production"]]
    live = [r for r in prod if not r["gated"]]
    gated = [r for r in prod if r["gated"]]
    print("\n  生產碼 %d 個：活 %d、閘 %d（另有非生產碼 %d 個不計）"
          % (len(prod), len(live), len(gated), len(rows) - len(prod)))
    return len(prod), len(live), len(gated)


def main(argv):
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    rows = collect(root, PATTERNS)
    total, live, gated = report(rows, "能啟動機台的呼叫點（fMain->Start 家族）")

    soft = collect(root, SOFTSTART)
    print()
    report(soft, "第二條繞道：不經 Start() 直接寫 SoftStart=true")

    # --- 第三份：武裝狀態 vs 樹上的宣稱 -------------------------------------
    armed = collect(root, ARMED)
    live_armed = [r for r in armed if not r["gated"]]
    print()
    report(armed, "★ 武裝狀態：誰在呼叫 StartFromWeb()（**刻意不排除 tools/**）")

    # 宣稱「還沒武裝」而且**下一行沒有就地作廢標註**的句子。
    # 留著原句是刻意的（史料），但它必須被標成作廢，否則它還在發放許可。
    claims = []
    for rel in STALE_CLAIM_FILES:
        p = os.path.join(root, rel)
        if not os.path.isfile(p):
            continue
        with io.open(p, encoding="utf-8") as fh:
            src = fh.readlines()
        for n, line in enumerate(src):
            if not STALE_CLAIM_RE.search(line):
                continue
            nxt = src[n + 1] if n + 1 < len(src) else ""
            if VOID_RE.search(line) or VOID_RE.search(nxt):
                continue                      # 已經標過作廢，是史料不是授權
            claims.append((rel, n + 1, line.strip()[:88]))

    print()
    print("=" * 78)
    print("★ 一致性斷言：武裝狀態 vs 樹上的宣稱")
    print("=" * 78)
    print("  活的 StartFromWeb 呼叫點            ：%d" % len(live_armed))
    print("  未標作廢的「零個呼叫者／尚未武裝」  ：%d" % len(claims))

    if live_armed and claims:
        sys.stderr.write(
            "\nFAIL 樹是武裝的，但有 %d 句註解還在說它不是，而且沒有標作廢。\n"
            "  那些句子不是描述，是**授權** ——「行為 delta = 0」等於\n"
            "  「你可以放心解這個閘，反正沒人呼叫」。武裝之後那個授權不成立。\n"
            "  修法：在該句下一行加一句帶「作廢」兩字的就地標註，\n"
            "        並在 WebStart.cpp 檔頭 §S3 留下權威說明。\n" % len(claims))
        for rel, n, text in claims:
            sys.stderr.write("    %s:%d  %s\n" % (rel, n, text))
        return 1
    if live_armed:
        print("  PASS 武裝，且沒有未標作廢的相反宣稱殘留。")
    else:
        print("  （尚未武裝）")

    if "--check" in argv:
        i = argv.index("--check")
        try:
            want = [int(x) for x in argv[i + 1:i + 4]]
        except (ValueError, IndexError):
            sys.stderr.write("--check 需要三個數字：總數 活 閘\n")
            return 2
        got = [total, live, gated]
        if got != want:
            sys.stderr.write(
                "\nFAIL 啟動路徑數量改變：期望 總數/活/閘 = %s，實際 %s\n"
                "  啟動路徑是 WebStart.h 論證的承重數字。改變它表示有人新增或移除了\n"
                "  一條能啟動機台的路徑。請回去更新：\n"
                "    HT9011UC_Cpp_V3.33.906.0/WebStart.h（那段盤點）\n"
                "    HT9011UC_Cpp_V3.33.906.0/docs/DUET3D_REFERENCE_ANALYSIS.md §9.1.1\n"
                "    D:\\HT9045\\CLAUDE.md（專有名詞表的 StartFromWeb 那一列）\n"
                % (want, got))
            return 1
        print("\nPASS 啟動路徑數量與釘住的基準一致：總數/活/閘 = %s" % got)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
