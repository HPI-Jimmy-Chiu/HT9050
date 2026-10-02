# -*- coding: utf-8 -*-
r"""檢查 HT9050 的 IO 表，並把它對回 HT9045 的程式碼常數。

用法（一定要用 .venv 的 python）：

    & d:\HT9045\.venv\Scripts\python.exe check_io_table.py
    & d:\HT9045\.venv\Scripts\python.exe check_io_table.py --csv D:\HT9045\system\IO_Table_9050.csv

檢查項目
    1. 表頭 14 欄是否齊全（程式端是用名字比對，名字錯了會靜默走位置 fallback）
    2. 欄位數不符的資料列
    3. 重複的 Alias
    4. IOType / ISABase / ModuleType / Enable 分佈
    5. 與 HT9045 IO_Table.csv 的差集
    6. 哪些 Alias 在 cmydef.cpp 找不到對應的 const int（= 表走在程式前面）

退出碼：0 = 只有資訊；1 = 有結構性問題（欄位數／表頭）。
Alias 對不上 cmydef 不算失敗，只是報數——HT9050 現階段本來就有一批待新增的常數。
"""
import argparse
import collections
import csv
import io
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SKILL = os.path.dirname(HERE)

DEFAULT_CSV = os.path.join(SKILL, "docs", "IO_Table_9050.csv")
DEFAULT_REF = r"D:\HT9045\system\IO_Table.csv"
DEFAULT_CMYDEF = r"D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cmydef.cpp"

EXPECTED = ["IOType", "Alias", "Lane", "ModuleType", "IP", "Port", "Bit", "InType",
            "ISABase", "Enable", "OnAlarmTime", "OffAlarmTime", "OnDelayTime", "OffDelayTime"]

# MachineType.h enum eIOType
ISABASE = {"0": "eMotionNet", "1": "eISABase", "2": "ePCI1735U", "3": "ePCI1203", "4": "ePLCbase"}


def load(path):
    with io.open(path, encoding="utf-8", errors="replace", newline="") as fh:
        return list(csv.reader(fh))


def cmydef_names(path):
    """cmydef.cpp 的 `const int <Name> = <n>;` 常數名集合。"""
    if not os.path.isfile(path):
        return None
    pat = re.compile(r"^\s*const\s+int\s+([A-Za-z_]\w*)\s*=")
    names = set()
    with io.open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = pat.match(line)
            if m:
                names.add(m.group(1))
    return names


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", default=DEFAULT_CSV)
    ap.add_argument("--ref", default=DEFAULT_REF, help="拿來比的 HT9045 IO 表")
    ap.add_argument("--cmydef", default=DEFAULT_CMYDEF)
    args = ap.parse_args()

    if not os.path.isfile(args.csv):
        print("找不到 %s" % args.csv, file=sys.stderr)
        return 1

    rows = load(args.csv)
    if not rows:
        print("空檔", file=sys.stderr)
        return 1

    fail = False
    header = [c.strip() for c in rows[0]]
    body = rows[1:]

    print("== 檔案 ==")
    print("  %s" % args.csv)
    print("  表頭 %d 欄、資料 %d 列" % (len(header), len(body)))

    print("\n== 1. 表頭 ==")
    missing = [c for c in EXPECTED if c not in header]
    if missing:
        print("  [FAIL] 缺欄位: %s" % ", ".join(missing))
        print("         程式端 TIOTABLENO::SetIOTableNo 用名字比對，缺名字會退回位置 fallback，")
        print("         欄位一旦挪動就會靜默讀錯欄。")
        fail = True
    else:
        print("  [OK] 14 欄齊全")
    extra = [c for c in header if c and c not in EXPECTED]
    if extra:
        print("  額外欄位: %s" % ", ".join(extra))

    print("\n== 2. 欄位數 ==")
    bad = [(i + 2, len(r)) for i, r in enumerate(body) if len(r) != len(header)]
    if bad:
        print("  [FAIL] %d 列欄位數不符 (應為 %d)" % (len(bad), len(header)))
        for ln, n in bad[:10]:
            print("         line %d: %d 欄" % (ln, n))
        fail = True
    else:
        print("  [OK] 全部 %d 欄" % len(header))

    idx = {c: header.index(c) for c in EXPECTED if c in header}

    def col(r, name):
        j = idx.get(name)
        return r[j].strip() if j is not None and j < len(r) else ""

    print("\n== 3. 重複 Alias ==")
    cnt = collections.Counter(col(r, "Alias") for r in body if col(r, "Alias"))
    dup = [(a, n) for a, n in cnt.items() if n > 1]
    if dup:
        print("  %d 個 Alias 重複（同名多列在這張表是常態：Cylinder 與其 _On/_Off 分列，" % len(dup))
        print("  但完全相同的 IOType+Alias 組合就是重複定義，下面只列後者）")
        pair = collections.Counter((col(r, "IOType"), col(r, "Alias")) for r in body if col(r, "Alias"))
        real = [(t, a, n) for (t, a), n in pair.items() if n > 1]
        if real:
            for t, a, n in sorted(real)[:20]:
                print("    [WARN] %s / %s  ×%d" % (t, a, n))
        else:
            print("    [OK] 沒有 IOType+Alias 完全重複的列")
    else:
        print("  [OK] 無重複")

    print("\n== 4. 分佈 ==")
    for name in ("IOType", "ModuleType", "Enable"):
        c = collections.Counter(col(r, name) for r in body)
        print("  %-11s %s" % (name, dict(sorted(c.items(), key=lambda kv: -kv[1]))))
    c = collections.Counter(col(r, "ISABase") for r in body)
    print("  ISABase     " + ", ".join(
        "%s(%s)=%d" % (k, ISABASE.get(k, "?"), v) for k, v in sorted(c.items(), key=lambda kv: -kv[1])))

    print("\n== 5. 與 HT9045 IO_Table.csv 的差集 ==")
    if os.path.isfile(args.ref):
        ref = load(args.ref)
        rh = [x.strip() for x in ref[0]]
        ja = rh.index("Alias") if "Alias" in rh else 1
        na = {r[ja].strip() for r in ref[1:] if len(r) > ja and r[ja].strip()}
        nb = {col(r, "Alias") for r in body if col(r, "Alias")}
        print("  9050 %d 個 Alias / 9045 %d 個" % (len(nb), len(na)))
        print("  只在 9050: %d" % len(nb - na))
        print("  只在 9045: %d  %s" % (len(na - nb), sorted(na - nb)[:10]))
    else:
        print("  [SKIP] 找不到 %s" % args.ref)

    print("\n== 6. cmydef.cpp 沒有對應常數的 Alias ==")
    names = cmydef_names(args.cmydef)
    if names is None:
        print("  [SKIP] 找不到 %s" % args.cmydef)
    else:
        def orphans(table_rows, colf):
            """回傳這張表裡對不到 cmydef 常數的 base name。

            排除兩類雜訊，否則連 HT9045 自己的表都會噴幾百筆：
              * Sucker*  -- 吸嘴名是 cinitial.cpp:326 用 sprintf("FTestSuck%c%c") 組出來的，
                            本來就不會有 const int，也不會有字串字面值。
              * _On/_Off -- 氣缸到位點是 cinitial.cpp:4871-4872 從 CylinderName 加尾綴衍生的，
                            只要 base 在就算數。
            """
            out = set()
            for r in table_rows:
                t = colf(r, "IOType")
                a = colf(r, "Alias")
                if not a or a.startswith("#") or t.startswith("Sucker"):
                    continue
                b = a
                for suf in ("_On", "_Off"):
                    if b.endswith(suf):
                        b = b[:-len(suf)]
                        break
                if b not in names:
                    out.add(b)
            return out

        mine = orphans(body, col)
        print("  cmydef.cpp 常數 %d 個" % len(names))
        print("  本表對不上的 base name: %d 個" % len(mine))

        baseset = None
        if os.path.isfile(args.ref) and os.path.abspath(args.ref) != os.path.abspath(args.csv):
            ref2 = load(args.ref)
            rh2 = [x.strip() for x in ref2[0]]
            ridx = {c: rh2.index(c) for c in EXPECTED if c in rh2}

            def rcol(r, name):
                j = ridx.get(name)
                return r[j].strip() if j is not None and j < len(r) else ""

            baseset = orphans(ref2[1:], rcol)
            print("  HT9045 基準表對不上的: %d 個（既有雜訊，不是 9050 造成的）" % len(baseset))

        if baseset is not None:
            delta = sorted(mine - baseset)
            print("")
            print("  ** 9050 專屬、程式端還沒有常數的點位: %d 個 **" % len(delta))
            print("  （這些才是真正要補進 cmydef.cpp 的；補之前先看 ht9045-array-audit）")
            for a in delta:
                print("    %s" % a)
        else:
            for a in sorted(mine)[:60]:
                print("    %s" % a)
            if len(mine) > 60:
                print("    ... 還有 %d 個" % (len(mine) - 60))

    print("\n== 結論 ==")
    print("  %s" % ("有結構性問題，見上面 [FAIL]" if fail else "結構檢查通過"))
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
