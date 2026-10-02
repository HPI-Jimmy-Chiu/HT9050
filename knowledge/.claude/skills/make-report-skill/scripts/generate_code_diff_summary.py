#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
generate_code_diff_summary.py
mtime 模式程式碼差異摘要產生工具

用法：
  python generate_code_diff_summary.py --project HT9045 --range daily
  python generate_code_diff_summary.py --project GPIB9045 --range weekly
  python generate_code_diff_summary.py --project HT9045 --date 2026-04-10
"""

import getpass
import os
import sys
import json
import re
import argparse
from datetime import datetime, timedelta

# === 專案設定 ===
PROJECT_ROOTS = {
    "HT9045": r"D:\HT9045",
    "GPIB9045": r"D:\GPIB9045",
}

# 掃描哪些副檔名
SCAN_EXTS = {".cpp", ".h"}

# 模組分類規則（HT9045）
MODULE_RULES_HT9045 = [
    ("inarm",    re.compile(r"aInArm", re.IGNORECASE)),
    ("outarm",   re.compile(r"aOutArm", re.IGNORECASE)),
    ("shuttle",  re.compile(r"aShuttle", re.IGNORECASE)),
    ("index",    re.compile(r"(aIndex|aTester)", re.IGNORECASE)),
    ("catchtray",re.compile(r"aCatchTray", re.IGNORECASE)),
    ("motor",    re.compile(r"(Motor|cMotor)", re.IGNORECASE)),
    ("io",       re.compile(r"(cIO|myio)", re.IGNORECASE)),
    ("secs",     re.compile(r"(HT9045Gem|uHGem)", re.IGNORECASE)),
    ("general",  re.compile(r"(CosFunction|HandlerSys)", re.IGNORECASE)),
]

# 模組分類規則（GPIB9045）
MODULE_RULES_GPIB9045 = [
    ("main",    re.compile(r"^Main\.cpp$", re.IGNORECASE)),
    ("rs232",   re.compile(r"RS232", re.IGNORECASE)),
    ("msgdef",  re.compile(r"(MessageDef|cmydef)", re.IGNORECASE)),
    ("art",     re.compile(r"(ART|DummyArt)", re.IGNORECASE)),
    ("dut",     re.compile(r"MyDutPanel", re.IGNORECASE)),
]

# 高風險關鍵字
HIGH_RISK_PATTERN = re.compile(
    r"(Motor|Sucker|Destroy|Alarm|JAM|Interlock|Safety)", re.IGNORECASE
)

# 函式定義 regex
FUNC_PATTERN = re.compile(
    r"^(void|bool|int|AnsiString|double|float|BOOL|char\s*\*?)\s+([A-Za-z_]\w*)\s*\(",
    re.MULTILINE
)

SNAPSHOT_FILENAME = ".last_snapshot_{project}.json"


def get_snapshot_path(script_dir, project):
    name = SNAPSHOT_FILENAME.format(project=project)
    return os.path.join(script_dir, name)


def load_snapshot(path):
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    return {}


def save_snapshot(path, data):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


def scan_files(root):
    """掃描 root 下所有 .cpp/.h，回傳 {rel_path: mtime_str}"""
    result = {}
    for dirpath, _, filenames in os.walk(root):
        for fn in filenames:
            ext = os.path.splitext(fn)[1].lower()
            if ext in SCAN_EXTS:
                full = os.path.join(dirpath, fn)
                try:
                    mtime = os.path.getmtime(full)
                    rel = os.path.relpath(full, root)
                    result[rel] = str(mtime)
                except OSError:
                    pass
    return result


def classify_module(filename, project):
    rules = MODULE_RULES_HT9045 if project == "HT9045" else MODULE_RULES_GPIB9045
    basename = os.path.basename(filename)
    for mod, pat in rules:
        if pat.search(basename):
            return mod
    return "other"


def parse_functions(filepath):
    """從檔案內容解析函式名稱清單"""
    try:
        with open(filepath, "r", encoding="mbcs", errors="ignore") as f:
            content = f.read()
        matches = FUNC_PATTERN.findall(content)
        return [m[1] for m in matches]
    except Exception:
        return []


def assess_risk(filename, funcs):
    name_risk = bool(HIGH_RISK_PATTERN.search(filename))
    func_risk = any(HIGH_RISK_PATTERN.search(fn) for fn in funcs)
    return "⚠ High" if (name_risk or func_risk) else "ℹ Normal"


def generate_summary(project, changed_files, root, date_str):
    lines = []
    lines.append(f"## Code Diff Summary — {date_str} — {project}")
    lines.append("")
    lines.append(f"異動檔案數：**{len(changed_files)}**")
    lines.append("")

    if not changed_files:
        lines.append("_本期間無檔案異動。_")
        return "\n".join(lines)

    # 表格
    lines.append("| 模組 | 檔案 | 風險等級 | 影響函式 |")
    lines.append("|------|------|----------|----------|")

    for rel_path, status in sorted(changed_files.items(), key=lambda x: x[0]):
        basename = os.path.basename(rel_path)
        full_path = os.path.join(root, rel_path)
        module = classify_module(basename, project)
        funcs = parse_functions(full_path) if os.path.exists(full_path) else []
        risk = assess_risk(basename, funcs)
        func_str = ", ".join(funcs[:8]) if funcs else "—"
        if len(funcs) > 8:
            func_str += f" ... (+{len(funcs)-8})"
        lines.append(f"| {module} | `{basename}` | {risk} | {func_str} |")

    lines.append("")
    lines.append(f"> 掃描路徑：`{root}`")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description="mtime 模式程式碼差異摘要產生工具")
    parser.add_argument("--project", required=True, choices=["HT9045", "GPIB9045"],
                        help="專案名稱")
    parser.add_argument("--range", choices=["daily", "weekly"], default="daily",
                        help="掃描範圍（daily=1天, weekly=7天）")
    parser.add_argument("--date", default=None,
                        help="指定日期 YYYY-MM-DD（預設今天）")
    args = parser.parse_args()

    project = args.project
    root = PROJECT_ROOTS.get(project)
    if not root or not os.path.isdir(root):
        print(f"ERROR: 找不到專案目錄 {root}", file=sys.stderr)
        sys.exit(1)

    if args.date:
        base_date = datetime.strptime(args.date, "%Y-%m-%d")
    else:
        base_date = datetime.now().replace(hour=0, minute=0, second=0, microsecond=0)

    if args.range == "weekly":
        cutoff = base_date - timedelta(days=7)
    else:
        cutoff = base_date - timedelta(days=1)

    cutoff_ts = cutoff.timestamp()
    date_str = base_date.strftime("%Y-%m-%d")

    script_dir = os.path.dirname(os.path.abspath(__file__))
    snap_path = get_snapshot_path(script_dir, project)
    old_snap = load_snapshot(snap_path)

    # 掃描當前狀態
    current = scan_files(root)

    # 找出異動檔案（mtime > cutoff 或與快照不同）
    changed = {}
    for rel, mtime_str in current.items():
        mtime = float(mtime_str)
        if mtime > cutoff_ts:
            old_mtime = old_snap.get(rel)
            status = "new" if old_mtime is None else "modified"
            changed[rel] = status

    # 存快照
    save_snapshot(snap_path, current)

    summary = generate_summary(project, changed, root, date_str)
    print(summary)

    # 同時輸出到 ops daily 目錄
    if args.range == "daily":
        # 20260929 Steven：報告寫進 RD5 入口網站 repo（<repo>\public\Docs\Daily\<EnglishName>\，檔名 YYYYMMDD_*.*）
        portal = os.environ.get("RD5_PORTAL_REPO", r"D:\RD5-Portal")
        name = os.environ.get("REPORT_ENGLISH_NAME") or getpass.getuser()
        ops_dir = os.path.join(portal, "public", "Docs", "Daily", name)
        ops_file = os.path.join(ops_dir, f"{date_str.replace('-', '')}_code_diff.md")

        if os.path.isdir(ops_dir):
            with open(ops_file, "w", encoding="utf-8") as f:
                f.write(summary)
            print(f"\n已輸出至：{ops_file}", file=sys.stderr)


if __name__ == "__main__":
    main()
