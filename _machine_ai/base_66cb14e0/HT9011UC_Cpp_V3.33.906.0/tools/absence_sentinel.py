# -*- coding: utf-8 -*-
r"""absence_sentinel.py -- 重驗樹上每一條「這個符號全樹沒有」的宣稱。

AI(W906-NL-ABSENCE) 20260916: night-loop skill 線 D 的第四個哨兵
（「absence-claim 重驗」）一直沒有實作，所以它從來沒跑過。這支就是它。

為什麼需要它
------------
gate 的理由常常是「X 全樹沒有 port」。那種宣稱**會過期** —— 後來的翻譯波次
把 X 落地了，但沒有人回頭改 gate。20260916 手工抽查最強的 9 條，**2 條過期**
（`FormBarcodeReader`、`ArmXCanSuck4IC_1032`，後者的定義就在同一個檔案
1,300 行上面）。22% 的過期率，而樹上還有約 400 條同型宣稱。

判準：用連結器的帳，不是 grep
-----------------------------
grep 分不出「註解裡提到」「`#if 0` 裡」「真的編出來」。20260916 第一版腳本
用「行首是不是 //」排除註解，把 `#if 0 // GATE G22` 底下的碼算成活的，
差點誤報一個 NULL 解參考。
所以這裡問的是 `nm --defined-only` 對 `build/*.a`：**編譯器實際產出了什麼**。

用法
----
    python tools/absence_sentinel.py [--build <dir>] [--verbose]
    python tools/absence_sentinel.py --selftest      # 比對器自己的固定樣本

退出碼：0 = 沒有過期的宣稱；1 = 有（逐條印出來）；2 = 環境不對（沒有 build）。

⚠ 它回報的是**需要人看一眼**的候選，不是判決。同名子字串會命中
（例如宣稱講的是 `X::ShowModal`，而 class X 本身存在）。每一條都要開檔確認。
⚠ archive 的新舊很重要：build/ 若是舊的，這支就是在問一個過期的問題。
   它會把 archive 的 mtime 印出來，自己判斷。
"""
import argparse
import glob
import os
import re
import subprocess
import sys
import time

TREE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NM_CANDIDATES = [r"C:\MinGW\bin\nm.exe", "nm"]

# 宣稱句型。每一條都要能抓出「被宣稱不存在的識別字」。
CLAIM_PATTERNS = [
    # `X -- STILL ABSENT` / `X: STILL ABSENT`
    re.compile(r"\b([A-Za-z_]\w{3,})\b[^\n]{0,40}STILL[- ]ABSENT"),
    # `no port ... X` 之類：X 在句尾括號或反引號裡
    re.compile(r"NO port[^\n]{0,60}?`([A-Za-z_]\w{3,})`", re.I),
    # 全樹沒有 X / 全樹沒有定義 X
    re.compile(r"全樹沒有[^\n]{0,20}?`?([A-Za-z_]\w{3,})`?"),
    # `grep -n "X" ... -> 0 hits`
    re.compile(r"grep[^\n]{0,80}?[\"'`]([A-Za-z_]\w{3,})[\"'`][^\n]{0,60}?0 hits"),
    # `X` has NO port object
    re.compile(r"\b([A-Za-z_]\w{3,})\b has NO port", re.I),
]

# 20260916 第一次跑完的分類結果。22 個候選逐條開檔看過，只有 2 個是真的過期。
# 把分類寫回來，讓下一次跑的人只看沒看過的。
#
# ⚠ 這不是「忽略清單」—— 每一條都寫了為什麼不算，可以複查。
KNOWN_NOT_EXPIRED = {
    "Del_Tree":
        "宣稱 scoped 到 TempCtrl（`grep TempCtrl/*.cpp -> 0 hits`），"
        "而 csystem.cpp 自己就是那唯一的定義。宣稱成立。",
    "bInArmToLoaderUsage":
        "那個 `_bInArmToLoaderUsage` 就是 ainarm9045.cpp 自己的定義；"
        "absence proof 證的是「沒有第二份」，不是「沒有」。",
    "Barcode_Reader":
        "cSetUp.cpp:1116 的註解**自己已經註記過期**"
        "（「20260824 的量測，當時為真、現在為假」）。",
    "ShowModal":
        "宣稱講的是 `fProductionInfo->ShowModal`；命中的是 TfPassword 等別的類別。"
        "20260916 實測 forms/fProductionInfo.h 仍然沒有 ShowModal。",
    "TfFTPClient":
        "命中的是 AutoRetest.cpp:325 的 file-local `W906ART_TfFTPClientSeam`，"
        "不是 TfFTPClient 的 port。",
    "ReadFile": "名字太泛，命中的是 TLotSummary::ReadFile 等無關符號。",
    "fProductionInfo":
        "cObserver.cpp:6094 的宣稱講的是 `fProductionInfo->ShowModal()`，不是那個物件。"
        "20260916 實測 `git grep ShowModal -- forms/fProductionInfo.h` 零命中，宣稱成立。",
    "bRunATC":
        "宣稱講的是「沒有會被編譯的 `bRunATC = true`」（20260916 用 g++ -E 定案），"
        "不是「沒有這個變數」。",
    "bFirstTime": "函式內 static，名字太泛。",
    "DoFTRTClick": "命中的是匿名 namespace 裡的 W5FA_TfMainExt seam，不是 golden 那支。",
    "SetOpenBin": "命中的是 W7T1_TfMainTorqueSeam 的 seam，不是 golden 那支。",
    "Send_Command_TTL": "命中的是 TfMain 的成員函式；宣稱講的是自由函式版本。",
    "fCounterClear": "命中的是 forms 的 static-init 符號，與宣稱的物件不同。",
    "FormBarcodeReader": "20260916 已確認過期並在 ebf7854 更正（保留以免重複回報）。",
    "ArmXCanSuck4IC_1032": "20260916 已確認過期並在 8312c1c 更正。",
}

# 註解區塊裡出現這些字，代表寫的人已經知道它過期了 -> 不用再報一次。
#
# ⚠ 20260916 第二次修：原本只靠散文字樣（「已經過期」「EXPIRED」…），結果我
# 自己註記完 10 幾處之後哨兵還是紅 —— 因為我在不同處寫了「只過期了一半」、
# 小寫 expired、單獨的「過期」。**靠散文比對不會收斂。**
# 正解是認一個**標籤**：每一處退役註記都帶 。
# 之後要退役新的過期宣稱，就用同一個標籤，哨兵才會自動閉嘴。
EXPIRY_MARKERS = ("AI(W906-NL-ABSENCE)",
                  "現在為假", "已經過期", "已過期", "HAS EXPIRED", "EXPIRED",
                  "expired", "當時為真")

# 這些字太常見，抓到也沒意義
STOPWORDS = {
    "this", "that", "there", "these", "those", "with", "from", "have", "has",
    "port", "tree", "gate", "golden", "grep", "hits", "none", "null", "true",
    "false", "class", "struct", "void", "bool", "const", "form", "file",
    "line", "lines", "code", "call", "calls", "which", "where", "when",
    "ABSENT", "absent", "STILL", "still", "OWNER", "DELTA", "WHY",
}


def symbol_matches(ident, sym):
    """這個 mangled 符號，是不是**就是**這個識別字？

    AI(W906-NL-ABSENCE) 20260916 第二版：第一版用 `ident in sym` 子字串比對，
    結果 `fShow` 命中 `TfShowBinSet`、`fTeach` 命中 `TfTeachPageControl`、
    `ReadFile` 命中 `TLotSummary::ReadFile` —— 22 個候選裡 20 個是這種假陽性。

    Itanium ABI 的名字是**長度前綴**的：`MoveInArm2XYToClean` 一定以
    `19MoveInArm2XYToClean` 的形式出現（19 == len）。拿這個當判準，
    `TfShowBinSet` 裡的 `fShow` 就不會再命中，因為它前面的數字是 12 而不是 5。
    C 連結的全域則是 `_name` 精確相等。
    """
    # ⚠ 20260916 第三版。第二版規定「長度前綴後面不可以接英數」——**那是錯的**，
    # 而且它造出假陰性：mangled 函式名後面接的是參數編碼（`v` = void），本來就是
    # 英數，所以 `__Z19MoveInArm2XYToCleanv` 被判成不匹配，兩條真的過期的宣稱
    # 直接從清單消失。假陽性會被人判掉，假陰性沒有人會來找。
    #
    # 正解是看**前綴側**：名字前面那串數字要剛好等於 len(ident)。
    # `TfShowBinSet` 裡的 `fShow` 前面是 `T`（非數字）-> 不匹配，正是要的效果。
    n = len(ident)
    start = 0
    while True:
        i = sym.find(ident, start)
        if i < 0:
            break
        start = i + 1
        j = i
        while j > 0 and sym[j - 1].isdigit():
            j -= 1
        if j < i and sym[j:i] == str(n):
            return True
    # C 連結：MinGW 會加一個前導底線
    return sym == "_" + ident or sym == ident


def find_nm():
    for c in NM_CANDIDATES:
        try:
            subprocess.run([c, "--version"], capture_output=True, check=True)
            return c
        except Exception:
            continue
    return None


def load_defined(nm, build):
    """-> {symbol_text: [archive, ...]}  （原樣，不 demangle；用子字串比對）"""
    out = {}
    archives = sorted(glob.glob(os.path.join(build, "*.a")))
    if not archives:
        return out, archives
    for a in archives:
        r = subprocess.run([nm, "--defined-only", a], capture_output=True)
        # memory: nm 輸出是 CRLF，用 splitlines()
        for line in r.stdout.decode("utf-8", "replace").splitlines():
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) >= 3:
                out.setdefault(parts[-1], set()).add(os.path.basename(a))
    return out, archives


WINDOW = 20   # 註記通常就在宣稱的上下幾行；20 行夠寬又不會跨到別的 gate


def collect_claims():
    """-> [(path, lineno, ident, raw, annotated)]

    AI(W906-NL-ABSENCE) 20260916 第二版：`annotated` 是「這條宣稱**前後 ±20 行**
    裡有沒有人已經註記它過期」。第一版只看宣稱那一行，所以我把 6 處註記完之後
    哨兵**照樣全紅** —— 那正是我同一天寫進 night-loop skill 的失敗模式：
    **一個永遠紅的哨兵等於沒有哨兵，而且更糟，因為它訓練人忽略紅燈。**
    """
    r = subprocess.run(["git", "-C", TREE, "ls-files", "--", "*.cpp", "*.h"],
                       capture_output=True)
    files = [f for f in r.stdout.decode("utf-8", "replace").splitlines()
             if f and not f.startswith("docs/")]
    claims = []
    for rel in files:
        p = os.path.join(TREE, rel.replace("/", os.sep))
        try:
            with open(p, "r", encoding="utf-8", errors="replace") as fh:
                lines = fh.read().splitlines()
        except OSError:
            continue
        for n, line in enumerate(lines, 1):
            if "ABSENT" not in line and "全樹沒有" not in line \
               and "0 hits" not in line and "NO port" not in line \
               and "has NO port" not in line:
                continue
            for rx in CLAIM_PATTERNS:
                for m in rx.finditer(line):
                    ident = m.group(1)
                    if ident in STOPWORDS or len(ident) < 4:
                        continue
                    lo = max(0, n - 1 - WINDOW)
                    hi = min(len(lines), n + WINDOW)
                    win = "\n".join(lines[lo:hi])
                    #AI(W906-NL-ABSENCE) 20260917: 大小寫不敏感。20260916 的清單
                    # 有 EXPIRED 與 expired，**就是沒有句首大寫的 Expired**，於是
                    # uHGemHT9045.cpp:5357 / :5380 兩處明明寫了「Expired --」「Expired.」
                    # 卻被判成沒標註，整個 fShowBinSelect 因此報 NEEDS REVIEW。
                    # 一個對正確標註喊紅的哨兵會訓練人忽略紅燈——那正是這個檔
                    # 自己在 :96 寫下的失敗模式。
                    win_l = win.lower()
                    ann = any(mk.lower() in win_l for mk in EXPIRY_MARKERS)
                    claims.append((rel, n, ident, line.strip()[:120], ann))
    return claims


# 比對器的固定樣本。前三組是**實際咬過的**：
#   1-2  第二版的假陰性（函式的參數編碼 `v` 被當成「後面接了英數」）
#   3-4  第一版的假陽性（子字串：fShow 命中 TfShowBinSet、fTeach 命中 TfTeachPageControl）
MATCH_CASES = [
    ("MoveInArm2XYToClean", "__Z19MoveInArm2XYToCleanv", True),
    ("MoveInArm2XYToDecayTeach", "__Z24MoveInArm2XYToDecayTeachv", True),
    ("fShow", ".eh_frame$_ZN12TfShowBinSet5CloseEv", False),
    ("fTeach", ".eh_frame$_ZN18TfTeachPageControlC1Ev", False),
    ("fTemp_Set", "_fTemp_Set", True),
    ("ArmXCanSuck4IC_1032", "__Z19ArmXCanSuck4IC_1032v", True),
    ("ReadFile", "__ZN11TLotSummary8ReadFileEv", True),
    ("Del_Tree", "__Z8Del_TreeN9vclcompat10AnsiStringE", True),
    ("Suck", "__Z19ArmXCanSuck4IC_1032v", False),      # 子字串但不是成分
    ("Clean", "__Z19MoveInArm2XYToCleanv", False),      # 同上
]


def selftest():
    bad = 0
    print("selftest -- the matcher against cases that actually bit us\n")
    for ident, sym, want in MATCH_CASES:
        got = symbol_matches(ident, sym)
        hit = (got == want)
        if not hit:
            bad += 1
        print("  %-4s %-26s %-42s want=%-5s got=%s"
              % ("PASS" if hit else "FAIL", ident, sym[:42], want, got))
    print()
    print("selftest OK: the matcher accepts real components and rejects "
          "substrings" if not bad else "SELFTEST FAILED on %d case(s)" % bad)
    return 1 if bad else 0


def main(argv):
    if "--selftest" in argv:
        return selftest()
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", default=os.path.join(TREE, "build"))
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args(argv[1:])

    nm = find_nm()
    if not nm:
        print("absence_sentinel: no nm on PATH -- cannot ask the linker")
        return 2

    defined, archives = load_defined(nm, a.build)
    if not archives:
        print("absence_sentinel: no archives under %s -- build first" % a.build)
        return 2
    newest = max(os.path.getmtime(x) for x in archives)
    print("archives: %d under %s (newest %s)"
          % (len(archives), a.build,
             time.strftime("%Y-%m-%d %H:%M", time.localtime(newest))))
    print("defined symbols: %d" % len(defined))

    claims = collect_claims()
    idents = {}
    for rel, n, ident, raw, ann in claims:
        idents.setdefault(ident, []).append((rel, n, raw, ann))
    print("absence claims parsed: %d over %d distinct identifiers"
          % (len(claims), len(idents)))
    print()

    expired, suppressed = [], []
    for ident in sorted(idents):
        hits = [k for k in defined if symbol_matches(ident, k)]
        if not hits:
            continue
        if ident in KNOWN_NOT_EXPIRED:
            suppressed.append((ident, KNOWN_NOT_EXPIRED[ident]))
            continue
        # 每一處宣稱旁邊（±20 行）都已經有人註記過期 -> 債已經記下了，不用再報
        sites = idents[ident]
        if sites and all(ann for _, _, _, ann in sites):
            suppressed.append(
                (ident, "all %d claim site(s) annotated as expired nearby" % len(sites)))
            continue
        expired.append((ident, hits, idents[ident]))

    if suppressed:
        print("suppressed %d identifier(s) triaged on 20260916 "
              "(reasons in KNOWN_NOT_EXPIRED):" % len(suppressed))
        for ident, why in suppressed:
            print("   %-24s %s" % (ident, why[:70]))
        print()

    if not expired:
        print("ALL ABSENCE CLAIMS STILL HOLD")
        return 0

    print("=" * 74)
    print("NEEDS REVIEW: %d identifier(s) claimed absent but present in the "
          "linker's table" % len(expired))
    print("(length-prefixed exact match -- still open each site and confirm)")
    print("=" * 74)
    for ident, hits, sites in expired:
        # sites: [(rel, lineno, raw, annotated)]
        print()
        print("%-28s %d defined symbol(s), e.g. %s"
              % (ident, len(hits), sorted(hits)[0][:60]))
        for rel, n, raw, ann in sites[:4]:
            print("    claimed at %s:%d" % (rel, n))
            if a.verbose:
                print("        %s" % raw)
        if len(sites) > 4:
            print("    ... and %d more site(s)" % (len(sites) - 4))
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
