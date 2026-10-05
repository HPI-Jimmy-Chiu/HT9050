#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
update_md_from_excel.py — 根據 Diff Report 自動更新 .md 參照檔

用途：讀取 secs_sync_audit.py 產生的 JSON，自動更新 .md 檔案
      並在檔案頭部追加版本紀錄，同時更新 Layer2-Changelog.md

使用方式：
    python update_md_from_excel.py --json audit_data.json --refs-dir ./references [--dry-run]

版本：V1.00 (2026-04-16)
"""

import argparse
import json
import re
import sys
from datetime import datetime
from pathlib import Path


def update_version_table(content, version, date, author, summary):
    """在 .md 檔的版本控制表中新增一行"""
    # 尋找版本控制表的最後一行（| Vx.xx | ... |）
    pattern = r"(\| V\d+\.\d+ \|[^\n]+\n)"
    matches = list(re.finditer(pattern, content))
    if matches:
        last_match = matches[-1]
        # 計算新版本號
        last_ver = re.search(r"V(\d+)\.(\d+)", last_match.group(0))
        if last_ver:
            major = int(last_ver.group(1))
            minor = int(last_ver.group(2)) + 1
            new_ver = f"V{major}.{minor:02d}"
        else:
            new_ver = version

        new_row = f"| {new_ver} | {date} | {author} | {summary} |\n"
        insert_pos = last_match.end()
        content = content[:insert_pos] + new_row + content[insert_pos:]
    return content


def update_count_in_header(content, old_count, new_count, label):
    """更新 .md 頭部的統計數字"""
    if old_count != new_count:
        content = content.replace(str(old_count), str(new_count), 1)
    return content


def append_changelog(changelog_path, date, version, author, changes, source_excel):
    """在 Layer2-Changelog.md 追加新條目"""
    entry_lines = [
        f"\n### {date} {version} — {author}\n",
        f"\n**來源 Excel**：`{source_excel}`\n",
        f"\n**變更內容**：\n",
    ]

    for change in changes:
        label = change["label"]
        added = change.get("added", {})
        removed = change.get("removed", {})
        modified = change.get("modified", {})

        if added or removed or modified:
            entry_lines.append(f"\n- **{label}**：")
            if added:
                entry_lines.append(f"+{len(added)} 新增")
            if removed:
                entry_lines.append(f", -{len(removed)} 移除")
            if modified:
                entry_lines.append(f", ~{len(modified)} 修改")
            entry_lines.append("\n")

            if added:
                for k, v in sorted(added.items(), key=lambda x: int(x[0]) if x[0].isdigit() else 0):
                    name = v.get("name", str(v))
                    entry_lines.append(f"  - + {k}: {name}\n")

    entry_lines.append(f"\n**審核者**：（待填）\n")
    entry_lines.append(f"\n---\n")

    if changelog_path.exists():
        existing = changelog_path.read_text(encoding="utf-8")
        # 找到「## 更新歷史」之後插入
        marker = "## 更新歷史"
        if marker in existing:
            idx = existing.index(marker) + len(marker)
            # 跳過該行的換行
            while idx < len(existing) and existing[idx] in ("\n", "\r"):
                idx += 1
            updated = existing[:idx] + "\n" + "".join(entry_lines) + existing[idx:]
        else:
            updated = existing + "\n" + "".join(entry_lines)
        changelog_path.write_text(updated, encoding="utf-8")
    else:
        print(f"WARNING: Changelog 不存在: {changelog_path}")


def main():
    parser = argparse.ArgumentParser(
        description="根據 Diff Report 自動更新 .md 參照檔",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--json", required=True, help="secs_sync_audit.py 產生的 JSON 檔")
    parser.add_argument("--refs-dir", default="./references", help="references/ 資料夾路徑")
    parser.add_argument("--author", default="AI", help="更新者名稱")
    parser.add_argument("--dry-run", action="store_true", help="只顯示會做什麼，不實際修改")

    args = parser.parse_args()

    json_path = Path(args.json)
    refs_dir = Path(args.refs_dir)

    if not json_path.exists():
        print(f"ERROR: 找不到 JSON 檔: {json_path}")
        sys.exit(1)

    data = json.loads(json_path.read_text(encoding="utf-8"))
    date = datetime.now().strftime("%Y-%m-%d")
    new_excel = Path(data["new_file"]).name

    print(f"{'[DRY RUN] ' if args.dry_run else ''}Processing audit from {data['audit_date']}")
    print(f"Source: {new_excel}")
    print()

    # 對應 diff label → .md 檔案
    label_to_file = {
        "SVID (狀態變量)": "SVID-ECID-Reference.md",
        "ECID (設備常數)": "SVID-ECID-Reference.md",
        "RCMD (遠端指令)": "RCMD-Reference.md",
        "Alarm (警報代碼)": "Alarm-Reference.md",
    }

    changes_summary = []

    for diff in data["diffs"]:
        label = diff["label"]
        added_count = len(diff.get("added", {}))
        removed_count = len(diff.get("removed", {}))
        modified_count = len(diff.get("modified", {}))

        if added_count == 0 and removed_count == 0 and modified_count == 0:
            continue

        target_file = label_to_file.get(label)
        if not target_file:
            continue

        md_path = refs_dir / target_file
        summary = f"+{added_count}" if added_count else ""
        if removed_count:
            summary += f", -{removed_count}"
        if modified_count:
            summary += f", ~{modified_count}"

        print(f"  {label}: {summary}")
        print(f"    → {md_path.name}")

        changes_summary.append(diff)

        if not args.dry_run and md_path.exists():
            content = md_path.read_text(encoding="utf-8")
            content = update_version_table(
                content,
                version="",
                date=date,
                author=args.author,
                summary=f"{label} {summary} (from {new_excel})",
            )
            md_path.write_text(content, encoding="utf-8")
            print(f"    ✅ 版本表已更新")

    # 更新 Changelog
    changelog_path = refs_dir / "Layer2-Changelog.md"
    if changes_summary:
        print(f"\n  Changelog → {changelog_path.name}")
        if not args.dry_run:
            if changelog_path.exists():
                append_changelog(changelog_path, date, "V_NEW", args.author, changes_summary, new_excel)
                print(f"    ✅ Changelog 已更新")
            else:
                print(f"    ⚠️ Changelog 不存在")

    if args.dry_run:
        print(f"\n[DRY RUN] 以上為預覽，未實際修改檔案")
    else:
        print(f"\n✅ 完成！請手動確認修改後 git commit")
        print(f'   建議 commit message: "chore: sync SVID/ECID/RCMD from {new_excel}"')


if __name__ == "__main__":
    main()
