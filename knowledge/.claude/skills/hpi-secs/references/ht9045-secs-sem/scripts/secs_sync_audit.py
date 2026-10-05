#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
secs_sync_audit.py — HT9045 SECS/GEM Excel 同步比對工具

用途：比對新舊 Excel 規格表，產生 Diff Report
      檢查 SVID/ECID/RCMD/CEID 的新增、修改、衝突

使用方式：
    python secs_sync_audit.py --new NEW.xlsx --old OLD.xlsx [--output report.txt]

輸出：
    1. 終端 Diff Report
    2. 文字報告檔（可選）
    3. JSON 結構化資料（可選，供 update_md_from_excel.py 使用）

版本：V1.00 (2026-04-16)
"""

import argparse
import json
import sys
from datetime import datetime
from pathlib import Path

try:
    import openpyxl
except ImportError:
    print("ERROR: 需要 openpyxl 套件。請執行: pip install openpyxl")
    sys.exit(1)


def read_sv_ec_sheet(wb, sheet_name="SV & EC"):
    """讀取 SV & EC 工作表，回傳 {id: {name, type, ...}} 字典"""
    svid_map = {}
    ecid_map = {}

    if sheet_name not in wb.sheetnames:
        print(f"WARNING: 找不到工作表 '{sheet_name}'")
        return svid_map, ecid_map

    ws = wb[sheet_name]
    headers = [cell.value for cell in ws[1]]

    # 找欄位索引
    id_col = None
    name_col = None
    type_col = None
    ht9045_col = None

    for i, h in enumerate(headers):
        if h is None:
            continue
        hl = str(h).strip().lower()
        if hl in ("id", "vid", "svid", "ecid"):
            id_col = i
        elif hl in ("name", "variable name", "sv name", "ec name"):
            name_col = i
        elif hl in ("type", "sv/ec", "category"):
            type_col = i
        elif "ht9045" in hl or "ht-9045" in hl:
            ht9045_col = i

    if id_col is None:
        # 嘗試用前幾欄
        id_col = 0
        name_col = name_col or 1

    for row in ws.iter_rows(min_row=2, values_only=False):
        cells = [cell.value for cell in row]
        vid = cells[id_col] if id_col < len(cells) else None
        if vid is None:
            continue
        try:
            vid = int(vid)
        except (ValueError, TypeError):
            continue

        name = str(cells[name_col]).strip() if name_col and name_col < len(cells) and cells[name_col] else ""
        vtype = str(cells[type_col]).strip() if type_col and type_col < len(cells) and cells[type_col] else ""
        ht9045_flag = str(cells[ht9045_col]).strip() if ht9045_col and ht9045_col < len(cells) and cells[ht9045_col] else ""

        entry = {"id": vid, "name": name, "type": vtype, "ht9045": ht9045_flag}

        if vtype.upper().startswith("SV") or vtype.upper().startswith("S"):
            svid_map[vid] = entry
        elif vtype.upper().startswith("EC") or vtype.upper().startswith("E"):
            ecid_map[vid] = entry
        else:
            # 無法分辨時兩邊都記
            svid_map[vid] = entry
            ecid_map[vid] = entry

    return svid_map, ecid_map


def read_rcmd_sheet(wb, sheet_name="Remote Command"):
    """讀取 Remote Command 工作表"""
    rcmd_map = {}

    if sheet_name not in wb.sheetnames:
        # 嘗試其他名稱
        for name in wb.sheetnames:
            if "remote" in name.lower() or "rcmd" in name.lower():
                sheet_name = name
                break
        else:
            print(f"WARNING: 找不到 Remote Command 工作表")
            return rcmd_map

    ws = wb[sheet_name]
    headers = [cell.value for cell in ws[1]]

    name_col = 0
    for i, h in enumerate(headers):
        if h and ("name" in str(h).lower() or "rcmd" in str(h).lower() or "command" in str(h).lower()):
            name_col = i
            break

    for row in ws.iter_rows(min_row=2, values_only=True):
        name = row[name_col] if name_col < len(row) else None
        if name and str(name).strip():
            rcmd_map[str(name).strip()] = {"name": str(name).strip()}

    return rcmd_map


def read_alarm_sheet(wb, sheet_name="AlarmCodeList_HT9045"):
    """讀取 AlarmCodeList 工作表"""
    alarm_map = {}

    if sheet_name not in wb.sheetnames:
        for name in wb.sheetnames:
            if "alarm" in name.lower():
                sheet_name = name
                break
        else:
            return alarm_map

    ws = wb[sheet_name]
    for row in ws.iter_rows(min_row=2, values_only=True):
        if row[0] is not None:
            try:
                alarm_map[int(row[0])] = {"code": int(row[0]), "text": str(row[1]) if len(row) > 1 and row[1] else ""}
            except (ValueError, TypeError):
                pass

    return alarm_map


def compare_maps(old_map, new_map, label):
    """比對兩個字典，回傳差異"""
    added = {k: v for k, v in new_map.items() if k not in old_map}
    removed = {k: v for k, v in old_map.items() if k not in new_map}
    modified = {}

    for k in old_map:
        if k in new_map and old_map[k] != new_map[k]:
            modified[k] = {"old": old_map[k], "new": new_map[k]}

    return {
        "label": label,
        "old_count": len(old_map),
        "new_count": len(new_map),
        "added": added,
        "removed": removed,
        "modified": modified,
    }


def check_id_conflicts(svid_map, ecid_map):
    """檢查 SVID 和 ECID 之間的 ID 衝突"""
    conflicts = []
    common = set(svid_map.keys()) & set(ecid_map.keys())
    for vid in sorted(common):
        conflicts.append({
            "id": vid,
            "sv_name": svid_map[vid].get("name", ""),
            "ec_name": ecid_map[vid].get("name", ""),
        })
    return conflicts


def format_report(diffs, conflicts, old_file, new_file):
    """格式化 Diff Report"""
    lines = []
    lines.append("=" * 60)
    lines.append("SECS/GEM Sync Audit Report")
    lines.append(f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    lines.append(f"Old: {old_file}")
    lines.append(f"New: {new_file}")
    lines.append("=" * 60)
    lines.append("")

    total_added = 0
    total_removed = 0
    total_modified = 0

    for diff in diffs:
        label = diff["label"]
        added = diff["added"]
        removed = diff["removed"]
        modified = diff["modified"]

        total_added += len(added)
        total_removed += len(removed)
        total_modified += len(modified)

        lines.append(f"【{label}】 {diff['old_count']} → {diff['new_count']}")
        lines.append("-" * 40)

        if added:
            lines.append(f"  新增 ({len(added)} 筆):")
            for k, v in sorted(added.items(), key=lambda x: x[0] if isinstance(x[0], int) else 0):
                name = v.get("name", v.get("text", str(v)))
                lines.append(f"    + {k}: {name}")

        if removed:
            lines.append(f"  移除 ({len(removed)} 筆):")
            for k, v in sorted(removed.items(), key=lambda x: x[0] if isinstance(x[0], int) else 0):
                name = v.get("name", v.get("text", str(v)))
                lines.append(f"    - {k}: {name}")

        if modified:
            lines.append(f"  修改 ({len(modified)} 筆):")
            for k, v in sorted(modified.items(), key=lambda x: x[0] if isinstance(x[0], int) else 0):
                lines.append(f"    ~ {k}: {v['old']} → {v['new']}")

        if not added and not removed and not modified:
            lines.append("  ✅ 無變更")

        lines.append("")

    # 衝突檢查
    lines.append("【ID 衝突檢查（SVID vs ECID）】")
    lines.append("-" * 40)
    if conflicts:
        lines.append(f"  ⚠️ 發現 {len(conflicts)} 個 ID 衝突:")
        for c in conflicts:
            lines.append(f"    ID {c['id']}: SV=\"{c['sv_name']}\" vs EC=\"{c['ec_name']}\"")
    else:
        lines.append("  ✅ 無 ID 衝突")

    lines.append("")
    lines.append("=" * 60)
    lines.append("【Summary】")
    lines.append(f"  新增: {total_added} 筆")
    lines.append(f"  移除: {total_removed} 筆")
    lines.append(f"  修改: {total_modified} 筆")
    lines.append(f"  衝突: {len(conflicts)} 筆")

    if total_removed == 0 and len(conflicts) == 0:
        lines.append("")
        lines.append("  ✅ Safe to merge")
    elif len(conflicts) > 0:
        lines.append("")
        lines.append("  ⚠️ 有 ID 衝突，請先解決再合併")
    else:
        lines.append("")
        lines.append("  ⚠️ 有移除項目，請確認是否預期")

    lines.append("=" * 60)
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="HT9045 SECS/GEM Excel 同步比對工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
範例：
  python secs_sync_audit.py --new SECS_20260416.xlsx --old SECS_20260401.xlsx
  python secs_sync_audit.py --new NEW.xlsx --old OLD.xlsx --output report.txt --json data.json
        """,
    )
    parser.add_argument("--new", required=True, help="新版 Excel 檔案路徑")
    parser.add_argument("--old", required=True, help="舊版 Excel 檔案路徑")
    parser.add_argument("--output", "-o", help="輸出報告檔案路徑（可選）")
    parser.add_argument("--json", "-j", help="輸出 JSON 結構化資料（可選，供 update_md_from_excel.py 使用）")

    args = parser.parse_args()

    old_path = Path(args.old)
    new_path = Path(args.new)

    if not old_path.exists():
        print(f"ERROR: 找不到舊版檔案: {old_path}")
        sys.exit(1)
    if not new_path.exists():
        print(f"ERROR: 找不到新版檔案: {new_path}")
        sys.exit(1)

    print(f"Loading old: {old_path.name} ...")
    old_wb = openpyxl.load_workbook(str(old_path), read_only=True, data_only=True)

    print(f"Loading new: {new_path.name} ...")
    new_wb = openpyxl.load_workbook(str(new_path), read_only=True, data_only=True)

    # 讀取各工作表
    old_sv, old_ec = read_sv_ec_sheet(old_wb)
    new_sv, new_ec = read_sv_ec_sheet(new_wb)

    old_rcmd = read_rcmd_sheet(old_wb)
    new_rcmd = read_rcmd_sheet(new_wb)

    old_alarm = read_alarm_sheet(old_wb)
    new_alarm = read_alarm_sheet(new_wb)

    old_wb.close()
    new_wb.close()

    # 比對
    diffs = [
        compare_maps(old_sv, new_sv, "SVID (狀態變量)"),
        compare_maps(old_ec, new_ec, "ECID (設備常數)"),
        compare_maps(old_rcmd, new_rcmd, "RCMD (遠端指令)"),
        compare_maps(old_alarm, new_alarm, "Alarm (警報代碼)"),
    ]

    # 衝突檢查（新版的 SVID vs ECID）
    conflicts = check_id_conflicts(new_sv, new_ec)

    # 生成報告
    report = format_report(diffs, conflicts, old_path.name, new_path.name)
    print()
    print(report)

    # 輸出報告檔
    if args.output:
        output_path = Path(args.output)
        output_path.write_text(report, encoding="utf-8")
        print(f"\n報告已存檔: {output_path}")

    # 輸出 JSON
    if args.json:
        json_path = Path(args.json)
        json_data = {
            "audit_date": datetime.now().isoformat(),
            "old_file": str(old_path),
            "new_file": str(new_path),
            "diffs": [],
            "conflicts": conflicts,
        }
        for diff in diffs:
            # 確保 key 為字串（JSON 需要）
            json_diff = {
                "label": diff["label"],
                "old_count": diff["old_count"],
                "new_count": diff["new_count"],
                "added": {str(k): v for k, v in diff["added"].items()},
                "removed": {str(k): v for k, v in diff["removed"].items()},
                "modified": {str(k): v for k, v in diff["modified"].items()},
            }
            json_data["diffs"].append(json_diff)

        json_path.write_text(json.dumps(json_data, ensure_ascii=False, indent=2), encoding="utf-8")
        print(f"JSON 資料已存檔: {json_path}")


if __name__ == "__main__":
    main()
